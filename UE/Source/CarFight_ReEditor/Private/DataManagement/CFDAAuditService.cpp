// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAuditService.cpp
// Version: v1.3.0
// Date: 2026-09-04
// Description: CF-FQ-045 metadata-only Inventory Core + 사용자-facing Registry 상태 메시지 구현입니다.
// Changelog:
// - v1.3.0: Registry 준비/조회 실패 메시지를 한글 우선 사용자 표현으로 교정.
// - v1.2.0: DAM-P0-02C loaded result freshness용 InventoryGeneration stamp와 next-generation pure helper를 추가.
// - v1.1.0: FCFDATypeRegistry를 Refresh/BuildInventory에 연결하는 metadata-only overload를 추가.
// - v1.0.1: project-local persisted unknown DataAsset 후보 discovery를 보강하고 Scope root를 path-segment 경계로 판정하도록 교정.
// - v1.0.0: native module class scan, recursive Asset Registry metadata scan, deterministic dedupe, zero-instance/abstract/Coverage/Scope 상태 계산을 구현.
// Migration:
// - Stable ID resolve, Validation, Duplicate ID, Reference/Referencer, Manager UI, Nomad Tab은 의도적으로 구현하지 않습니다.

#include "DataManagement/CFDAAuditService.h"
#include "DataManagement/CFDATypeRegistry.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/DataAsset.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectIterator.h"

namespace CFDAAuditServicePrivate
{
	// DAM-P0-01 canonical native module package 이름입니다.
	const TSet<FString> CanonicalModulePackages =
	{
		TEXT("/Script/CarFight_Re"),
		TEXT("/Script/CarFight_ReEditor")
	};

	// class path 문자열에서 표시 가능한 technical class name을 추출합니다.
	FString ExtractTechnicalClassName(const FString& ClassPath)
	{
		// class path의 마지막 package/class 구분점 위치입니다.
		int32 LastDotIndex = INDEX_NONE;
		if (ClassPath.FindLastChar(TEXT('.'), LastDotIndex) && LastDotIndex + 1 < ClassPath.Len())
		{
			return ClassPath.Mid(LastDotIndex + 1);
		}

		return ClassPath;
	}

	// Package path가 exact root 또는 그 하위 path인지 segment 경계로 판정합니다.
	bool IsPackagePathWithin(const FString& PackagePath, const TCHAR* RootPath)
	{
		// 비교할 canonical root 문자열입니다.
		const FString RootPathString(RootPath);
		if (PackagePath.Equals(RootPathString, ESearchCase::CaseSensitive))
		{
			return true;
		}

		// prefix 오인을 막기 위해 root 뒤에 slash를 붙인 child prefix입니다.
		const FString ChildPrefix = RootPathString + TEXT("/");
		return PackagePath.StartsWith(ChildPrefix, ESearchCase::CaseSensitive);
	}

	// native CarFight UDataAsset descendant와 Asset Registry filter에 사용할 UClass 포인터를 함께 수집합니다.
	void DiscoverNativeTypes(
		TArray<FCFDANativeTypeMetadata>& OutNativeTypes,
		TArray<UClass*>& OutNativeClasses)
	{
		OutNativeTypes.Reset();
		OutNativeClasses.Reset();

		// 같은 native class가 iterator 경로에서 중복 투영되는 것을 막는 canonical class path 집합입니다.
		TSet<FString> SeenClassPaths;

		// 현재 CarFight Editor process에 등록된 native UClass를 순회합니다.
		for (TObjectIterator<UClass> ClassIterator; ClassIterator; ++ClassIterator)
		{
			// 현재 검사할 native class 후보입니다.
			UClass* CandidateClass = *ClassIterator;
			if (!CandidateClass
				|| CandidateClass == UDataAsset::StaticClass()
				|| !CandidateClass->HasAnyClassFlags(CLASS_Native)
				|| !CandidateClass->IsChildOf(UDataAsset::StaticClass()))
			{
				continue;
			}

			// class owner module의 /Script package 이름입니다.
			const FString ModulePackageName = CandidateClass->GetOutermost()->GetName();
			if (!CanonicalModulePackages.Contains(ModulePackageName))
			{
				continue;
			}

			// Unreal canonical native class path 문자열입니다.
			const FString ClassPath = CandidateClass->GetClassPathName().ToString();
			if (SeenClassPaths.Contains(ClassPath))
			{
				continue;
			}

			SeenClassPaths.Add(ClassPath);
			OutNativeClasses.Add(CandidateClass);

			// pure analyzer에 넘길 native type metadata입니다.
			FCFDANativeTypeMetadata TypeMetadata;
			TypeMetadata.ClassPath = ClassPath;
			TypeMetadata.TechnicalClassName = CandidateClass->GetName();
			TypeMetadata.bAbstract = CandidateClass->HasAnyClassFlags(CLASS_Abstract);
			OutNativeTypes.Add(MoveTemp(TypeMetadata));
		}
	}

	// CarFight native 계열 또는 project-local /Game DataAsset의 persisted FAssetData metadata를 수집합니다.
	bool DiscoverPersistedAssets(
		IAssetRegistry& AssetRegistry,
		const TArray<UClass*>& NativeClasses,
		TArray<FCFDAPersistedAssetMetadata>& OutPersistedAssets)
	{
		OutPersistedAssets.Reset();

		// CarFight native class identity와 Asset Registry가 아는 derived class identity를 모은 집합입니다.
		TSet<FTopLevelAssetPath> RelevantClassPaths;

		// derived class graph query의 root가 되는 CarFight native class path 목록입니다.
		TArray<FTopLevelAssetPath> NativeClassPaths;
		NativeClassPaths.Reserve(NativeClasses.Num());

		// native class identity를 exact relevant set과 derived roots에 모두 추가합니다.
		for (UClass* NativeClass : NativeClasses)
		{
			if (!NativeClass)
			{
				continue;
			}

			// 현재 native DataAsset class의 canonical class path입니다.
			const FTopLevelAssetPath NativeClassPath = NativeClass->GetClassPathName();
			RelevantClassPaths.Add(NativeClassPath);
			NativeClassPaths.AddUnique(NativeClassPath);
		}

		// derived class graph에서 제외할 class가 없는 P0-01 집합입니다.
		const TSet<FTopLevelAssetPath> ExcludedClassPaths;

		// Blueprint/generated descendant를 포함한 CarFight native 계열 derived class path입니다.
		TSet<FTopLevelAssetPath> DerivedClassPaths;
		AssetRegistry.GetDerivedClassNames(NativeClassPaths, ExcludedClassPaths, DerivedClassPaths);
		RelevantClassPaths.Append(DerivedClassPaths);

		// 모든 persisted UDataAsset descendant를 metadata-only로 읽는 Asset Registry filter입니다.
		FARFilter AssetFilter;
		AssetFilter.bRecursiveClasses = true;
		AssetFilter.ClassPaths.Add(UDataAsset::StaticClass()->GetClassPathName());

		// metadata-only Asset Registry query 결과입니다.
		TArray<FAssetData> AssetDataResults;
		if (!AssetRegistry.GetAssets(AssetFilter, AssetDataResults))
		{
			return false;
		}

		OutPersistedAssets.Reserve(AssetDataResults.Num());

		// FAssetData만 DTO로 투영하며 GetAsset()은 호출하지 않습니다.
		for (const FAssetData& AssetData : AssetDataResults)
		{
			// 현재 Asset의 package path 문자열입니다.
			const FString PackagePath = AssetData.PackagePath.ToString();

			// /Game 아래의 project-local 후보인지 나타냅니다.
			const bool bProjectLocalCandidate = IsPackagePathWithin(PackagePath, TEXT("/Game"));

			// CarFight native class 또는 그 derived class identity인지 나타냅니다.
			const bool bKnownCarFightClass = RelevantClassPaths.Contains(AssetData.AssetClassPath);
			if (!bProjectLocalCandidate && !bKnownCarFightClass)
			{
				continue;
			}

			// persisted Asset 한 건의 metadata-only DTO입니다.
			FCFDAPersistedAssetMetadata AssetMetadata;
			AssetMetadata.AssetName = AssetData.AssetName.ToString();
			AssetMetadata.ObjectPath = AssetData.GetSoftObjectPath().ToString();
			AssetMetadata.PackagePath = PackagePath;
			AssetMetadata.ClassPath = AssetData.AssetClassPath.ToString();
			OutPersistedAssets.Add(MoveTemp(AssetMetadata));
		}

		return true;
	}
}

// caller의 current generation을 stamp하면서 Typed Registry metadata-only inventory를 새로 만듭니다.
FCFDAInventoryResult FCFDAAuditService::Refresh(
	const FCFDATypeRegistry& TypeRegistry,
	const uint64 InventoryGeneration)
{
	// 기존 metadata-only Refresh 결과입니다.
	FCFDAInventoryResult Result = Refresh(TypeRegistry);
	Result.InventoryGeneration = InventoryGeneration;
	return Result;
}

// Typed Registry descriptor coverage를 적용해 metadata-only inventory를 새로 만듭니다.
FCFDAInventoryResult FCFDAAuditService::Refresh(const FCFDATypeRegistry& TypeRegistry)
{
	return Refresh(TypeRegistry.BuildRefreshOptions());
}

// 현재 loaded native class universe와 Asset Registry metadata로 inventory를 새로 만듭니다.
FCFDAInventoryResult FCFDAAuditService::Refresh(const FCFDARefreshOptions& Options)
{
	// native canonical DataAsset metadata입니다.
	TArray<FCFDANativeTypeMetadata> NativeTypes;

	// Asset Registry recursive class filter를 구성할 native UClass 목록입니다.
	TArray<UClass*> NativeClasses;
	CFDAAuditServicePrivate::DiscoverNativeTypes(NativeTypes, NativeClasses);

	// AssetRegistry module을 명시적으로 resolve한 결과입니다.
	FAssetRegistryModule* AssetRegistryModule = FModuleManager::LoadModulePtr<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if (!AssetRegistryModule)
	{
		return BuildInventory(
			ECFDARegistryState::Failed,
			TEXT("에셋 레지스트리 기능을 불러오지 못했습니다."),
			NativeTypes,
			TArray<FCFDAPersistedAssetMetadata>(),
			Options);
	}

	// 현재 Editor process의 Asset Registry interface입니다.
	IAssetRegistry& AssetRegistry = AssetRegistryModule->Get();

	// Registry gathering 여부를 complete inventory 판정과 분리해 보존합니다.
	const bool bRegistryGathering = AssetRegistry.IsLoadingAssets();

	// Refresh 시작 시점의 Registry readiness 상태입니다.
	const ECFDARegistryState RegistryState =
		bRegistryGathering ? ECFDARegistryState::Gathering : ECFDARegistryState::Ready;

	// Gathering 결과를 0건 complete inventory로 오해하지 않게 하는 진단 메시지입니다.
	const FString RegistryMessage =
		bRegistryGathering ? TEXT("에셋 레지스트리가 아직 정보를 수집 중입니다.") : FString();

	// persisted Asset metadata-only DTO입니다.
	TArray<FCFDAPersistedAssetMetadata> PersistedAssets;
	if (!CFDAAuditServicePrivate::DiscoverPersistedAssets(AssetRegistry, NativeClasses, PersistedAssets))
	{
		return BuildInventory(
			ECFDARegistryState::Failed,
			TEXT("에셋 레지스트리에서 저장된 에셋 정보를 조회하지 못했습니다."),
			NativeTypes,
			TArray<FCFDAPersistedAssetMetadata>(),
			Options);
	}

	return BuildInventory(RegistryState, RegistryMessage, NativeTypes, PersistedAssets, Options);
}

// synthetic DTO에 Typed Registry descriptor coverage를 적용해 pure inventory를 만듭니다.
FCFDAInventoryResult FCFDAAuditService::BuildInventory(
	ECFDARegistryState RegistryState,
	const FString& RegistryMessage,
	const TArray<FCFDANativeTypeMetadata>& NativeTypes,
	const TArray<FCFDAPersistedAssetMetadata>& PersistedAssets,
	const FCFDATypeRegistry& TypeRegistry)
{
	return BuildInventory(
		RegistryState,
		RegistryMessage,
		NativeTypes,
		PersistedAssets,
		TypeRegistry.BuildRefreshOptions());
}

// synthetic DTO를 deterministic하게 merge/dedupe하여 Automation 가능한 pure inventory를 만듭니다.
FCFDAInventoryResult FCFDAAuditService::BuildInventory(
	ECFDARegistryState RegistryState,
	const FString& RegistryMessage,
	const TArray<FCFDANativeTypeMetadata>& NativeTypes,
	const TArray<FCFDAPersistedAssetMetadata>& PersistedAssets,
	const FCFDARefreshOptions& Options)
{
	// 최종 metadata-only inventory 결과입니다.
	FCFDAInventoryResult Result;
	Result.RegistryState = RegistryState;
	Result.RegistryMessage = RegistryMessage;

	// 입력 순서와 무관한 type dedupe를 위해 정렬한 native metadata입니다.
	TArray<FCFDANativeTypeMetadata> SortedNativeTypes = NativeTypes;
	SortedNativeTypes.Sort([](const FCFDANativeTypeMetadata& Left, const FCFDANativeTypeMetadata& Right)
	{
		// 같은 class path에서 conflict가 있어도 첫 선택이 deterministic하도록 secondary key까지 비교합니다.
		const int32 ClassCompare = Left.ClassPath.Compare(Right.ClassPath, ESearchCase::CaseSensitive);
		if (ClassCompare != 0)
		{
			return ClassCompare < 0;
		}

		// 같은 class path의 technical name tie-break입니다.
		const int32 NameCompare = Left.TechnicalClassName.Compare(Right.TechnicalClassName, ESearchCase::CaseSensitive);
		if (NameCompare != 0)
		{
			return NameCompare < 0;
		}

		return static_cast<uint8>(Left.bAbstract) < static_cast<uint8>(Right.bAbstract);
	});

	// class path별 exact Type record를 소유하는 merge map입니다.
	TMap<FString, FCFDATypeRecord> TypeRecordsByClassPath;

	// canonical native type을 class path 단위로 1회만 투영합니다.
	for (const FCFDANativeTypeMetadata& NativeType : SortedNativeTypes)
	{
		if (NativeType.ClassPath.IsEmpty() || TypeRecordsByClassPath.Contains(NativeType.ClassPath))
		{
			continue;
		}

		// native canonical Type record입니다.
		FCFDATypeRecord TypeRecord;
		TypeRecord.ClassPath = NativeType.ClassPath;
		TypeRecord.TechnicalClassName = NativeType.TechnicalClassName.IsEmpty()
			? CFDAAuditServicePrivate::ExtractTechnicalClassName(NativeType.ClassPath)
			: NativeType.TechnicalClassName;
		TypeRecord.bCanonicalNative = true;
		TypeRecord.bAbstract = NativeType.bAbstract;
		TypeRecord.CoverageState = Options.RegisteredDescriptorClassPaths.Contains(NativeType.ClassPath)
			? ECFDACoverageState::Registered
			: ECFDACoverageState::Unregistered;
		TypeRecordsByClassPath.Add(TypeRecord.ClassPath, MoveTemp(TypeRecord));
	}

	// 입력 순서와 무관한 Asset dedupe를 위해 deterministic key 순으로 정렬한 metadata입니다.
	TArray<FCFDAPersistedAssetMetadata> SortedPersistedAssets = PersistedAssets;
	SortedPersistedAssets.Sort([](const FCFDAPersistedAssetMetadata& Left, const FCFDAPersistedAssetMetadata& Right)
	{
		// ObjectPath가 asset identity의 primary deterministic key입니다.
		const int32 ObjectCompare = Left.ObjectPath.Compare(Right.ObjectPath, ESearchCase::CaseSensitive);
		if (ObjectCompare != 0)
		{
			return ObjectCompare < 0;
		}

		// duplicate ObjectPath conflict 시 class path를 secondary key로 사용합니다.
		const int32 ClassCompare = Left.ClassPath.Compare(Right.ClassPath, ESearchCase::CaseSensitive);
		if (ClassCompare != 0)
		{
			return ClassCompare < 0;
		}

		// duplicate ObjectPath conflict 시 package path를 tertiary key로 사용합니다.
		const int32 PackageCompare = Left.PackagePath.Compare(Right.PackagePath, ESearchCase::CaseSensitive);
		if (PackageCompare != 0)
		{
			return PackageCompare < 0;
		}

		return Left.AssetName.Compare(Right.AssetName, ESearchCase::CaseSensitive) < 0;
	});

	// 같은 persisted object path를 exact 1건으로 유지하는 dedupe 집합입니다.
	TSet<FString> SeenObjectPaths;

	// metadata-only persisted Asset을 dedupe하고 Type inventory와 merge합니다.
	for (const FCFDAPersistedAssetMetadata& PersistedAsset : SortedPersistedAssets)
	{
		if (PersistedAsset.ObjectPath.IsEmpty() || SeenObjectPaths.Contains(PersistedAsset.ObjectPath))
		{
			continue;
		}

		SeenObjectPaths.Add(PersistedAsset.ObjectPath);

		// canonical native set 밖에서 Asset scan으로 발견된 class를 숨기지 않기 위한 candidate Type record입니다.
		if (!PersistedAsset.ClassPath.IsEmpty() && !TypeRecordsByClassPath.Contains(PersistedAsset.ClassPath))
		{
			// persisted unknown candidate의 Generic Type record입니다.
			FCFDATypeRecord CandidateTypeRecord;
			CandidateTypeRecord.ClassPath = PersistedAsset.ClassPath;
			CandidateTypeRecord.TechnicalClassName =
				CFDAAuditServicePrivate::ExtractTechnicalClassName(PersistedAsset.ClassPath);
			CandidateTypeRecord.bCanonicalNative = false;
			CandidateTypeRecord.bAbstract = false;
			CandidateTypeRecord.CoverageState = Options.RegisteredDescriptorClassPaths.Contains(PersistedAsset.ClassPath)
				? ECFDACoverageState::Registered
				: ECFDACoverageState::Unregistered;
			TypeRecordsByClassPath.Add(CandidateTypeRecord.ClassPath, MoveTemp(CandidateTypeRecord));
		}

		// Generic persisted Asset record입니다.
		FCFDAAssetRecord AssetRecord;
		AssetRecord.AssetName = PersistedAsset.AssetName;
		AssetRecord.ObjectPath = PersistedAsset.ObjectPath;
		AssetRecord.PackagePath = PersistedAsset.PackagePath;
		AssetRecord.ClassPath = PersistedAsset.ClassPath;
		AssetRecord.Scope = ClassifyScope(PersistedAsset.PackagePath);
		AssetRecord.CoverageState = Options.RegisteredDescriptorClassPaths.Contains(PersistedAsset.ClassPath)
			? ECFDACoverageState::Registered
			: ECFDACoverageState::Unregistered;
		AssetRecord.HealthState = ECFDAHealthState::NotValidated;
		AssetRecord.StableIdState = ECFDAStableIdState::NotResolved;
		AssetRecord.StableId.Reset();
		Result.AssetRecords.Add(MoveTemp(AssetRecord));

		// 이 exact class path의 Type record입니다.
		FCFDATypeRecord* OwningTypeRecord = TypeRecordsByClassPath.Find(PersistedAsset.ClassPath);
		if (OwningTypeRecord)
		{
			++OwningTypeRecord->AssetCount;
		}
	}

	// map iteration order를 제거하기 위해 final Type records를 배열로 복사합니다.
	TypeRecordsByClassPath.GenerateValueArray(Result.TypeRecords);

	// asset count를 TypeInstanceState로 정규화합니다.
	for (FCFDATypeRecord& TypeRecord : Result.TypeRecords)
	{
		TypeRecord.TypeInstanceState = TypeRecord.AssetCount > 0
			? ECFDATypeInstanceState::HasAssets
			: ECFDATypeInstanceState::NoAssetInstance;
	}

	// Type records의 final deterministic order입니다.
	Result.TypeRecords.Sort([](const FCFDATypeRecord& Left, const FCFDATypeRecord& Right)
	{
		return Left.ClassPath.Compare(Right.ClassPath, ESearchCase::CaseSensitive) < 0;
	});

	// Asset records의 final deterministic order입니다.
	Result.AssetRecords.Sort([](const FCFDAAssetRecord& Left, const FCFDAAssetRecord& Right)
	{
		return Left.ObjectPath.Compare(Right.ObjectPath, ESearchCase::CaseSensitive) < 0;
	});

	return Result;
}

// session-local owner가 Refresh 직전에 사용할 다음 generation 값을 계산합니다.
uint64 FCFDAAuditService::NextInventoryGeneration(const uint64 CurrentInventoryGeneration)
{
	return CurrentInventoryGeneration == MAX_uint64
		? 1
		: CurrentInventoryGeneration + 1;
}

// Package path를 DAM v0.1.1 Scope 계약으로 분류합니다.
ECFDAScope FCFDAAuditService::ClassifyScope(const FString& PackagePath)
{
	if (CFDAAuditServicePrivate::IsPackagePathWithin(PackagePath, TEXT("/Game/CarFight/Tests"))
		|| CFDAAuditServicePrivate::IsPackagePathWithin(PackagePath, TEXT("/Game/Test")))
	{
		return ECFDAScope::Test;
	}

	if (CFDAAuditServicePrivate::IsPackagePathWithin(PackagePath, TEXT("/Game/CarFight/_Legacy")))
	{
		return ECFDAScope::Legacy;
	}

	if (CFDAAuditServicePrivate::IsPackagePathWithin(PackagePath, TEXT("/Game/CarFight/Debug")))
	{
		return ECFDAScope::Debug;
	}

	if (CFDAAuditServicePrivate::IsPackagePathWithin(PackagePath, TEXT("/Game/CarFight/Data/Authoring")))
	{
		return ECFDAScope::Authoring;
	}

	if (CFDAAuditServicePrivate::IsPackagePathWithin(PackagePath, TEXT("/Game/CarFight")))
	{
		return ECFDAScope::RuntimeContent;
	}

	return ECFDAScope::Unclassified;
}
