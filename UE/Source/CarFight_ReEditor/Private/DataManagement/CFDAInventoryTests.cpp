// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAInventoryTests.cpp
// Version: v1.0.1
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-01 Inventory Core focused Automation입니다.
// Changelog:
// - v1.0.1: Scope root의 exact/child 경계와 false-prefix 방지 시나리오를 focused merge test에 추가.
// - v1.0.0: deterministic dedupe/merge, abstract zero-instance, Scope/Unclassified, Coverage, Gathering, current native/asset integration을 검증.
// Migration:
// - synthetic DTO seam과 metadata-only Asset Registry scan만 사용하며 Product Asset을 생성·수정·저장하지 않습니다.

#include "DataManagement/CFDAAuditService.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Algo/Reverse.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

namespace CFDAInventoryTestsPrivate
{
	// class path로 Type record를 찾습니다.
	const FCFDATypeRecord* FindTypeRecord(
		const FCFDAInventoryResult& Result,
		const FString& ClassPath)
	{
		// requested class path와 일치하는 Type record입니다.
		for (const FCFDATypeRecord& TypeRecord : Result.TypeRecords)
		{
			if (TypeRecord.ClassPath == ClassPath)
			{
				return &TypeRecord;
			}
		}

		return nullptr;
	}

	// object path로 Asset record를 찾습니다.
	const FCFDAAssetRecord* FindAssetRecord(
		const FCFDAInventoryResult& Result,
		const FString& ObjectPath)
	{
		// requested object path와 일치하는 Asset record입니다.
		for (const FCFDAAssetRecord& AssetRecord : Result.AssetRecords)
		{
			if (AssetRecord.ObjectPath == ObjectPath)
			{
				return &AssetRecord;
			}
		}

		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAInventoryMergeTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_01.MergeScopeCoverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAInventoryGatheringTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_01.RegistryGathering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAInventoryIntegrationTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_01.CurrentInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// synthetic DTO 순서를 뒤집어도 merge/dedupe/Scope/Coverage 결과가 deterministic한지 검증합니다.
bool FCFDAInventoryMergeTest::RunTest(const FString& Parameters)
{
	// registered descriptor가 있다고 가정할 synthetic canonical type path입니다.
	const FString AlphaClassPath = TEXT("/Script/CarFight_Re.CFAlphaData");

	// zero-instance abstract framework type path입니다.
	const FString FrameworkClassPath = TEXT("/Script/CarFight_Re.CFFrameworkData");

	// persisted scan에서만 발견될 synthetic future type path입니다.
	const FString FutureClassPath = TEXT("/Script/CarFight_Re.CFFutureData");

	// native class discovery synthetic rows입니다.
	TArray<FCFDANativeTypeMetadata> NativeTypes;
	NativeTypes.Add({AlphaClassPath, TEXT("CFAlphaData"), false});
	NativeTypes.Add({FrameworkClassPath, TEXT("CFFrameworkData"), true});
	NativeTypes.Add({AlphaClassPath, TEXT("ZZ_ConflictingDuplicate"), true});

	// persisted Asset Registry synthetic rows이며 첫 행을 intentional duplicate로 추가합니다.
	TArray<FCFDAPersistedAssetMetadata> PersistedAssets;
	PersistedAssets.Add({TEXT("DA_Alpha"), TEXT("/Game/CarFight/Data/DA_Alpha.DA_Alpha"), TEXT("/Game/CarFight/Data"), AlphaClassPath});
	PersistedAssets.Add({TEXT("DA_Alpha"), TEXT("/Game/CarFight/Data/DA_Alpha.DA_Alpha"), TEXT("/Game/CarFight/Data"), AlphaClassPath});
	PersistedAssets.Add({TEXT("DA_TestFuture"), TEXT("/Game/Test/DA_TestFuture.DA_TestFuture"), TEXT("/Game/Test"), FutureClassPath});
	PersistedAssets.Add({TEXT("DA_Unclassified"), TEXT("/External/CarFight/DA_Unclassified.DA_Unclassified"), TEXT("/External/CarFight"), AlphaClassPath});

	// Alpha type만 descriptor coverage가 있다고 가정하는 Refresh 옵션입니다.
	FCFDARefreshOptions Options;
	Options.RegisteredDescriptorClassPaths.Add(AlphaClassPath);

	// 원본 순서에서 만든 first deterministic inventory입니다.
	const FCFDAInventoryResult FirstResult = FCFDAAuditService::BuildInventory(
		ECFDARegistryState::Ready,
		FString(),
		NativeTypes,
		PersistedAssets,
		Options);

	Algo::Reverse(NativeTypes);
	Algo::Reverse(PersistedAssets);

	// 역순 입력에서 만든 second deterministic inventory입니다.
	const FCFDAInventoryResult SecondResult = FCFDAAuditService::BuildInventory(
		ECFDARegistryState::Ready,
		FString(),
		NativeTypes,
		PersistedAssets,
		Options);

	TestEqual(TEXT("Type deterministic count"), FirstResult.TypeRecords.Num(), 3);
	TestEqual(TEXT("Asset exact dedupe count"), FirstResult.AssetRecords.Num(), 3);
	TestEqual(TEXT("Reverse input type count"), SecondResult.TypeRecords.Num(), FirstResult.TypeRecords.Num());
	TestEqual(TEXT("Reverse input asset count"), SecondResult.AssetRecords.Num(), FirstResult.AssetRecords.Num());

	// 등록된 concrete Alpha type record입니다.
	const FCFDATypeRecord* AlphaType =
		CFDAInventoryTestsPrivate::FindTypeRecord(FirstResult, AlphaClassPath);
	TestNotNull(TEXT("Alpha type visible"), AlphaType);
	if (AlphaType)
	{
		TestTrue(TEXT("Alpha canonical native"), AlphaType->bCanonicalNative);
		TestFalse(TEXT("Alpha concrete"), AlphaType->bAbstract);
		TestEqual(TEXT("Alpha asset count after dedupe"), AlphaType->AssetCount, 2);
		TestEqual(TEXT("Alpha descriptor coverage"), AlphaType->CoverageState, ECFDACoverageState::Registered);
		TestEqual(TEXT("Alpha instance state"), AlphaType->TypeInstanceState, ECFDATypeInstanceState::HasAssets);
	}

	// zero-instance abstract framework type record입니다.
	const FCFDATypeRecord* FrameworkType =
		CFDAInventoryTestsPrivate::FindTypeRecord(FirstResult, FrameworkClassPath);
	TestNotNull(TEXT("Framework type visible with zero instance"), FrameworkType);
	if (FrameworkType)
	{
		TestTrue(TEXT("Framework abstract"), FrameworkType->bAbstract);
		TestEqual(TEXT("Framework zero assets"), FrameworkType->AssetCount, 0);
		TestEqual(TEXT("Framework no-instance state"), FrameworkType->TypeInstanceState, ECFDATypeInstanceState::NoAssetInstance);
		TestEqual(TEXT("Framework unregistered coverage"), FrameworkType->CoverageState, ECFDACoverageState::Unregistered);
	}

	// persisted scan에서만 발견된 future candidate Type record입니다.
	const FCFDATypeRecord* FutureType =
		CFDAInventoryTestsPrivate::FindTypeRecord(FirstResult, FutureClassPath);
	TestNotNull(TEXT("Future persisted-only type visible"), FutureType);
	if (FutureType)
	{
		TestFalse(TEXT("Future type is not canonical native"), FutureType->bCanonicalNative);
		TestEqual(TEXT("Future type coverage"), FutureType->CoverageState, ECFDACoverageState::Unregistered);
		TestEqual(TEXT("Future type has asset"), FutureType->TypeInstanceState, ECFDATypeInstanceState::HasAssets);
	}

	// Test Scope로 분류되어야 하는 synthetic future asset입니다.
	const FCFDAAssetRecord* TestAsset =
		CFDAInventoryTestsPrivate::FindAssetRecord(FirstResult, TEXT("/Game/Test/DA_TestFuture.DA_TestFuture"));
	TestNotNull(TEXT("Test asset visible"), TestAsset);
	if (TestAsset)
	{
		TestEqual(TEXT("Test asset scope"), TestAsset->Scope, ECFDAScope::Test);
		TestEqual(TEXT("Test asset coverage"), TestAsset->CoverageState, ECFDACoverageState::Unregistered);
		TestEqual(TEXT("Test asset health lazy"), TestAsset->HealthState, ECFDAHealthState::NotValidated);
		TestEqual(TEXT("Test asset stable id lazy"), TestAsset->StableIdState, ECFDAStableIdState::NotResolved);
		TestTrue(TEXT("Test asset stable id empty"), TestAsset->StableId.IsEmpty());
	}

	// 표준 path 밖이라 Unclassified여야 하는 synthetic asset입니다.
	const FCFDAAssetRecord* UnclassifiedAsset =
		CFDAInventoryTestsPrivate::FindAssetRecord(FirstResult, TEXT("/External/CarFight/DA_Unclassified.DA_Unclassified"));
	TestNotNull(TEXT("Unclassified asset visible"), UnclassifiedAsset);
	if (UnclassifiedAsset)
	{
		TestEqual(TEXT("Unclassified scope"), UnclassifiedAsset->Scope, ECFDAScope::Unclassified);
	}

	// Scope root exact/child와 prefix false-positive 방지를 직접 검증합니다.
	TestEqual(TEXT("Scope exact Test root"), FCFDAAuditService::ClassifyScope(TEXT("/Game/Test")), ECFDAScope::Test);
	TestEqual(TEXT("Scope Test child"), FCFDAAuditService::ClassifyScope(TEXT("/Game/Test/Foo")), ECFDAScope::Test);
	TestEqual(TEXT("Scope CarFight Tests child"), FCFDAAuditService::ClassifyScope(TEXT("/Game/CarFight/Tests/Foo")), ECFDAScope::Test);
	TestEqual(TEXT("Scope Legacy child"), FCFDAAuditService::ClassifyScope(TEXT("/Game/CarFight/_Legacy/Foo")), ECFDAScope::Legacy);
	TestEqual(TEXT("Scope Debug child"), FCFDAAuditService::ClassifyScope(TEXT("/Game/CarFight/Debug/Foo")), ECFDAScope::Debug);
	TestEqual(TEXT("Scope Authoring child"), FCFDAAuditService::ClassifyScope(TEXT("/Game/CarFight/Data/Authoring/Foo")), ECFDAScope::Authoring);
	TestEqual(TEXT("Scope Runtime child"), FCFDAAuditService::ClassifyScope(TEXT("/Game/CarFight/Data/Foo")), ECFDAScope::RuntimeContent);
	TestEqual(TEXT("Scope other Game unclassified"), FCFDAAuditService::ClassifyScope(TEXT("/Game/Other/Foo")), ECFDAScope::Unclassified);
	TestEqual(TEXT("Scope Test false prefix"), FCFDAAuditService::ClassifyScope(TEXT("/Game/Testament/Foo")), ECFDAScope::Unclassified);
	TestEqual(TEXT("Scope Debug false prefix remains runtime"), FCFDAAuditService::ClassifyScope(TEXT("/Game/CarFight/DebugTools/Foo")), ECFDAScope::RuntimeContent);

	// 두 결과의 deterministic Type order와 identity를 비교합니다.
	for (int32 Index = 0; Index < FirstResult.TypeRecords.Num(); ++Index)
	{
		TestEqual(
			FString::Printf(TEXT("Deterministic type[%d]"), Index),
			FirstResult.TypeRecords[Index].ClassPath,
			SecondResult.TypeRecords[Index].ClassPath);
	}

	// 두 결과의 deterministic Asset order와 identity를 비교합니다.
	for (int32 Index = 0; Index < FirstResult.AssetRecords.Num(); ++Index)
	{
		TestEqual(
			FString::Printf(TEXT("Deterministic asset[%d]"), Index),
			FirstResult.AssetRecords[Index].ObjectPath,
			SecondResult.AssetRecords[Index].ObjectPath);
	}

	return true;
}

// Registry Gathering 상태에서는 zero rows를 complete inventory로 오해하지 않는지 검증합니다.
bool FCFDAInventoryGatheringTest::RunTest(const FString& Parameters)
{
	// Gathering 중에도 visibility를 유지해야 하는 zero-instance native type입니다.
	TArray<FCFDANativeTypeMetadata> NativeTypes;
	NativeTypes.Add({TEXT("/Script/CarFight_Re.CFZeroData"), TEXT("CFZeroData"), false});

	// descriptor coverage가 없는 기본 옵션입니다.
	const FCFDARefreshOptions Options;

	// Asset Registry Gathering을 synthetic하게 투영한 inventory입니다.
	const FCFDAInventoryResult Result = FCFDAAuditService::BuildInventory(
		ECFDARegistryState::Gathering,
		TEXT("Registry Scanning"),
		NativeTypes,
		TArray<FCFDAPersistedAssetMetadata>(),
		Options);

	TestEqual(TEXT("Gathering state retained"), Result.RegistryState, ECFDARegistryState::Gathering);
	TestFalse(TEXT("Gathering inventory is incomplete"), Result.IsInventoryComplete());
	TestEqual(TEXT("Zero-instance type remains visible"), Result.TypeRecords.Num(), 1);
	TestEqual(TEXT("No false persisted assets"), Result.AssetRecords.Num(), 0);
	TestEqual(TEXT("Zero-instance state retained"), Result.TypeRecords[0].TypeInstanceState, ECFDATypeInstanceState::NoAssetInstance);
	return true;
}

// current Project의 native CarFight DA universe와 persisted metadata inventory baseline을 integration으로 검증합니다.
bool FCFDAInventoryIntegrationTest::RunTest(const FString& Parameters)
{
	// current Editor process의 Asset Registry module입니다.
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// current Editor process의 Asset Registry interface입니다.
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Automation inventory assertion 전에 Registry completion prerequisite를 synchronous scan으로 충족합니다.
	AssetRegistry.SearchAllAssets(true);
	TestFalse(TEXT("Asset Registry completion prerequisite"), AssetRegistry.IsLoadingAssets());

	// descriptor registry는 DAM-P0-02 owner이므로 P0-01 integration에서는 비어 있는 옵션을 사용합니다.
	const FCFDARefreshOptions Options;

	// current Project metadata-only inventory입니다.
	const FCFDAInventoryResult Result = FCFDAAuditService::Refresh(Options);
	TestEqual(TEXT("Current inventory registry ready"), Result.RegistryState, ECFDARegistryState::Ready);
	TestTrue(TEXT("Current inventory complete"), Result.IsInventoryComplete());

	// current native CarFight type baseline을 세기 위한 counter입니다.
	int32 CanonicalNativeTypeCount = 0;

	// current native abstract baseline을 세기 위한 counter입니다.
	int32 CanonicalAbstractTypeCount = 0;

	// abstract UCFInventoryItemData class path를 확인하기 위한 flag입니다.
	bool bFoundInventoryItemAbstract = false;

	// current canonical native type records를 집계합니다.
	for (const FCFDATypeRecord& TypeRecord : Result.TypeRecords)
	{
		if (!TypeRecord.bCanonicalNative)
		{
			continue;
		}

		++CanonicalNativeTypeCount;
		if (TypeRecord.bAbstract)
		{
			++CanonicalAbstractTypeCount;
		}

		if (TypeRecord.ClassPath == TEXT("/Script/CarFight_Re.CFInventoryItemData"))
		{
			bFoundInventoryItemAbstract = TypeRecord.bAbstract;
		}
	}

	TestTrue(TEXT("Canonical native DA baseline >= 28"), CanonicalNativeTypeCount >= 28);
	TestTrue(TEXT("Canonical abstract baseline >= 1"), CanonicalAbstractTypeCount >= 1);
	TestTrue(TEXT("CFInventoryItemData abstract visible"), bFoundInventoryItemAbstract);
	TestTrue(TEXT("Persisted DA baseline >= 53"), Result.AssetRecords.Num() >= 53);

	// current persisted asset object path dedupe를 검증하는 set입니다.
	TSet<FString> UniqueObjectPaths;

	// 모든 persisted asset의 exact class type visibility와 dedupe를 확인합니다.
	for (const FCFDAAssetRecord& AssetRecord : Result.AssetRecords)
	{
		TestFalse(
			FString::Printf(TEXT("No duplicate object path: %s"), *AssetRecord.ObjectPath),
			UniqueObjectPaths.Contains(AssetRecord.ObjectPath));
		UniqueObjectPaths.Add(AssetRecord.ObjectPath);

		// asset class를 소유하는 visible Type record입니다.
		const FCFDATypeRecord* OwningType =
			CFDAInventoryTestsPrivate::FindTypeRecord(Result, AssetRecord.ClassPath);
		TestNotNull(
			FString::Printf(TEXT("Asset class visible: %s"), *AssetRecord.ClassPath),
			OwningType);
	}

	return true;
}

#endif
