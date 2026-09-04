// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAReferenceService.cpp
// Version: v1.2.0
// Date: 2026-09-04
// Description: CF-FQ-045 DAM-P0-04E 참조 관계 사용자 메시지 한글 우선 교정 구현입니다.
// Changelog:
// - v1.2.0: 참조 대상/사용처 조회의 성공·실패 메시지를 사용자 관점의 한글 표현으로 교정.
// - v1.1.0: GetDependencies/GetReferencers false를 successful-empty로 오해하지 않도록 query evidence를 fail-closed 처리.
// - v1.0.0: package dependency/referencer를 Package category로 read-only 조회.
// Migration:
// - UObject load, Save, mutation 없음.

#include "DataManagement/CFDAReferenceService.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"

namespace CFDAReferenceServicePrivate
{
	const FCFDAAssetRecord* FindAssetRecord(
		const FCFDAInventoryResult& Inventory,
		const FString& ObjectPath)
	{
		for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
		{
			if (AssetRecord.ObjectPath == ObjectPath)
			{
				return &AssetRecord;
			}
		}
		return nullptr;
	}

	void SortUniquePackageNames(
		const TArray<FName>& PackageNames,
		TArray<FString>& OutPackageNames)
	{
		TSet<FString> UniqueNames;
		for (const FName PackageName : PackageNames)
		{
			if (!PackageName.IsNone())
			{
				UniqueNames.Add(PackageName.ToString());
			}
		}

		OutPackageNames = UniqueNames.Array();
		OutPackageNames.Sort([](const FString& Left, const FString& Right)
		{
			return Left.Compare(Right, ESearchCase::CaseSensitive) < 0;
		});
	}
}

void FCFDAReferenceService::FinalizeQueryEvidence(
	const bool bReferenceQuerySucceeded,
	const bool bReferencerQuerySucceeded,
	FCFDAReferenceResult& InOutResult)
{
	InOutResult.bReferenceEvidenceAvailable = bReferenceQuerySucceeded;
	InOutResult.bReferencerEvidenceAvailable = bReferencerQuerySucceeded;

	if (!bReferenceQuerySucceeded || !bReferencerQuerySucceeded)
	{
		InOutResult.QueryState = ECFDAReferenceQueryState::QueryFailed;
		InOutResult.ReferencerState = ECFDAReferencerState::QueryFailed;

		if (!bReferenceQuerySucceeded && !bReferencerQuerySucceeded)
		{
			InOutResult.Message = TEXT("에셋 레지스트리에서 참조 대상과 사용처 정보를 모두 확인하지 못했습니다. 결과가 0건이라는 뜻은 아닙니다.");
		}
		else if (!bReferenceQuerySucceeded)
		{
			InOutResult.Message = TEXT("에셋 레지스트리에서 이 에셋이 참조하는 대상 정보를 확인하지 못했습니다. 현재 목록을 완전한 결과로 간주하지 않습니다.");
		}
		else
		{
			InOutResult.Message = TEXT("에셋 레지스트리에서 이 에셋을 참조하는 사용처 정보를 확인하지 못했습니다. 사용처가 없다는 뜻은 아닙니다.");
		}
		return;
	}

	InOutResult.QueryState = ECFDAReferenceQueryState::Succeeded;
	InOutResult.ReferencerState = InOutResult.Referencers.IsEmpty()
		? ECFDAReferencerState::NoKnownReferencer
		: ECFDAReferencerState::Referenced;
	InOutResult.Message = FString::Printf(
		TEXT("참조 대상 %d개 / 사용처 %d개를 저장된 에셋 레지스트리 정보에서 조회했습니다."),
		InOutResult.References.Num(),
		InOutResult.Referencers.Num());
}

FCFDAReferenceResult FCFDAReferenceService::Query(
	const FCFDAInventoryResult& Inventory,
	const FString& ObjectPath)
{
	FCFDAReferenceResult Result;
	Result.InventoryGeneration = Inventory.InventoryGeneration;
	Result.ObjectPath = ObjectPath;

	if (Inventory.InventoryGeneration == 0 || !Inventory.IsInventoryComplete())
	{
		Result.QueryState = ECFDAReferenceQueryState::QueryFailed;
		Result.ReferencerState = ECFDAReferencerState::QueryFailed;
		Result.Message = TEXT("참조 관계 조회를 실행하려면 준비가 완료된 데이터 목록이 필요합니다.");
		return Result;
	}

	const FCFDAAssetRecord* AssetRecord =
		CFDAReferenceServicePrivate::FindAssetRecord(Inventory, ObjectPath);
	if (!AssetRecord)
	{
		Result.QueryState = ECFDAReferenceQueryState::QueryFailed;
		Result.ReferencerState = ECFDAReferencerState::QueryFailed;
		Result.Message = TEXT("요청한 오브젝트 경로가 현재 데이터 목록에 없습니다.");
		return Result;
	}

	const FString PackageNameString = FPackageName::ObjectPathToPackageName(ObjectPath);
	if (PackageNameString.IsEmpty())
	{
		Result.QueryState = ECFDAReferenceQueryState::QueryFailed;
		Result.ReferencerState = ECFDAReferencerState::QueryFailed;
		Result.Message = TEXT("오브젝트 경로에서 패키지 이름을 확인하지 못했습니다.");
		return Result;
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if (AssetRegistry.IsLoadingAssets())
	{
		Result.QueryState = ECFDAReferenceQueryState::QueryFailed;
		Result.ReferencerState = ECFDAReferencerState::QueryFailed;
		Result.Message = TEXT("에셋 레지스트리가 정보를 수집 중이라 참조 관계 조회를 완료하지 못했습니다.");
		return Result;
	}

	const FName PackageName(*PackageNameString);
	const UE::AssetRegistry::FDependencyQuery DependencyQuery;
	TArray<FName> ReferencePackages;
	TArray<FName> ReferencerPackages;

	// UE 5.8에서 false는 '0건'이 아니라 CachedDependsNode를 찾지 못한 query failure이므로 반드시 보존합니다.
	const bool bReferenceQuerySucceeded = AssetRegistry.GetDependencies(
		PackageName,
		ReferencePackages,
		UE::AssetRegistry::EDependencyCategory::Package,
		DependencyQuery);
	const bool bReferencerQuerySucceeded = AssetRegistry.GetReferencers(
		PackageName,
		ReferencerPackages,
		UE::AssetRegistry::EDependencyCategory::Package,
		DependencyQuery);

	CFDAReferenceServicePrivate::SortUniquePackageNames(
		ReferencePackages,
		Result.References);
	CFDAReferenceServicePrivate::SortUniquePackageNames(
		ReferencerPackages,
		Result.Referencers);

	FinalizeQueryEvidence(
		bReferenceQuerySucceeded,
		bReferencerQuerySucceeded,
		Result);
	return Result;
}
