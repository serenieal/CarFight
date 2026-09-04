// Copyright (c) CarFight. All Rights Reserved.
// File: CFDARegistryTests.cpp
// Version: v1.0.1
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02A Typed Registry Foundation focused Automation입니다.
// Changelog:
// - v1.0.1: Registered/Unregistered semantic policy-authority guard를 검증.
// - v1.0.0: descriptor 등록/idempotency/conflict, deterministic enumeration, Unregistered fallback, Coverage bridge, current inventory fallback을 검증.
// Migration:
// - synthetic descriptor/DTO와 metadata-only Asset Registry scan만 사용하며 Stable ID/Validation/Reference/UI를 실행하지 않습니다.

#include "DataManagement/CFDAAuditService.h"
#include "DataManagement/CFDATypeRegistry.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

namespace CFDARegistryTestsPrivate
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDARegistryFoundationTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02A.RegistryFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDARegistryCurrentFallbackTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02A.CurrentUnregisteredFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Typed Registry의 등록/lookup/fallback/Coverage bridge가 deterministic한지 검증합니다.
bool FCFDARegistryFoundationTest::RunTest(const FString& Parameters)
{
	// 등록 coverage를 가질 synthetic vehicle type path입니다.
	const FString AlphaClassPath = TEXT("/Script/CarFight_Re.CFAlphaData");

	// second descriptor deterministic order를 확인할 synthetic combat type path입니다.
	const FString BetaClassPath = TEXT("/Script/CarFight_Re.CFBetaData");

	// Registry에 등록하지 않을 future type path입니다.
	const FString FutureClassPath = TEXT("/Script/CarFight_Re.CFFutureData");

	// DAM-P0-02A synthetic Typed Registry입니다.
	FCFDATypeRegistry TypeRegistry;

	// Alpha에 등록할 semantic descriptor입니다.
	FCFDASemanticDescriptor AlphaDescriptor;
	AlphaDescriptor.ClassPath = AlphaClassPath;
	AlphaDescriptor.Domain = ECFDADomain::Vehicle;
	AlphaDescriptor.TypeDisplayName = TEXT("알파 차량 데이터");
	AlphaDescriptor.RoleDescription = TEXT("Synthetic 차량 데이터 역할 설명입니다.");

	// Beta에 등록할 semantic descriptor입니다.
	FCFDASemanticDescriptor BetaDescriptor;
	BetaDescriptor.ClassPath = BetaClassPath;
	BetaDescriptor.Domain = ECFDADomain::Combat;
	BetaDescriptor.TypeDisplayName = TEXT("베타 전투 데이터");
	BetaDescriptor.RoleDescription = TEXT("Synthetic 전투 데이터 역할 설명입니다.");

	// registration 오류 진단 문자열입니다.
	FString RegistrationError;
	TestTrue(
		TEXT("Beta descriptor registers"),
		TypeRegistry.RegisterDescriptor(BetaDescriptor, &RegistrationError));
	TestTrue(TEXT("Beta register has no error"), RegistrationError.IsEmpty());

	TestTrue(
		TEXT("Alpha descriptor registers"),
		TypeRegistry.RegisterDescriptor(AlphaDescriptor, &RegistrationError));
	TestTrue(TEXT("Alpha register has no error"), RegistrationError.IsEmpty());
	TestEqual(TEXT("Registry count after two descriptors"), TypeRegistry.Num(), 2);

	TestTrue(
		TEXT("Exact descriptor re-register is idempotent"),
		TypeRegistry.RegisterDescriptor(AlphaDescriptor, &RegistrationError));
	TestTrue(TEXT("Idempotent register has no error"), RegistrationError.IsEmpty());
	TestEqual(TEXT("Registry count remains two"), TypeRegistry.Num(), 2);

	// same class path에 다른 Domain을 넣는 conflicting descriptor입니다.
	FCFDASemanticDescriptor ConflictingAlphaDescriptor = AlphaDescriptor;
	ConflictingAlphaDescriptor.Domain = ECFDADomain::Targeting;
	TestFalse(
		TEXT("Conflicting descriptor fails closed"),
		TypeRegistry.RegisterDescriptor(ConflictingAlphaDescriptor, &RegistrationError));
	TestTrue(TEXT("Conflict reports error"), !RegistrationError.IsEmpty());
	TestEqual(TEXT("Conflict does not change registry count"), TypeRegistry.Num(), 2);

	// exact 등록된 Alpha descriptor lookup입니다.
	const FCFDASemanticDescriptor* FoundAlphaDescriptor =
		TypeRegistry.FindDescriptor(AlphaClassPath);
	TestNotNull(TEXT("Alpha descriptor lookup"), FoundAlphaDescriptor);
	if (FoundAlphaDescriptor)
	{
		TestEqual(TEXT("Alpha domain preserved"), FoundAlphaDescriptor->Domain, ECFDADomain::Vehicle);
		TestEqual(TEXT("Alpha display name preserved"), FoundAlphaDescriptor->TypeDisplayName, AlphaDescriptor.TypeDisplayName);
	}

	// class path 기준 deterministic descriptor enumeration입니다.
	const TArray<FCFDASemanticDescriptor> OrderedDescriptors = TypeRegistry.GetDescriptors();
	TestEqual(TEXT("Descriptor enumeration count"), OrderedDescriptors.Num(), 2);
	if (OrderedDescriptors.Num() == 2)
	{
		TestEqual(TEXT("Descriptor order Alpha first"), OrderedDescriptors[0].ClassPath, AlphaClassPath);
		TestEqual(TEXT("Descriptor order Beta second"), OrderedDescriptors[1].ClassPath, BetaClassPath);
	}

	// registered Alpha semantic lookup 결과입니다.
	const FCFDAResolvedSemantic RegisteredSemantic =
		TypeRegistry.ResolveSemantic(AlphaClassPath, TEXT("CFAlphaData"));
	TestEqual(TEXT("Registered semantic coverage"), RegisteredSemantic.CoverageState, ECFDACoverageState::Registered);
	TestTrue(TEXT("Registered semantic policy is authoritative"), RegisteredSemantic.HasAuthoritativePolicy());
	TestEqual(TEXT("Registered semantic domain"), RegisteredSemantic.Descriptor.Domain, ECFDADomain::Vehicle);
	TestEqual(TEXT("Registered semantic display"), RegisteredSemantic.Descriptor.TypeDisplayName, AlphaDescriptor.TypeDisplayName);

	// 등록되지 않은 Future type의 안전한 semantic fallback입니다.
	const FCFDAResolvedSemantic FutureFallback =
		TypeRegistry.ResolveSemantic(FutureClassPath, TEXT("CFFutureData"));
	TestEqual(TEXT("Future fallback unregistered"), FutureFallback.CoverageState, ECFDACoverageState::Unregistered);
	TestFalse(TEXT("Future fallback policy is not authoritative"), FutureFallback.HasAuthoritativePolicy());
	TestEqual(TEXT("Future fallback domain"), FutureFallback.Descriptor.Domain, ECFDADomain::Unclassified);
	TestEqual(TEXT("Future fallback technical display"), FutureFallback.Descriptor.TypeDisplayName, FString(TEXT("CFFutureData")));
	TestTrue(TEXT("Future fallback role is readable"), !FutureFallback.Descriptor.RoleDescription.IsEmpty());

	// descriptor coverage bridge를 검증할 synthetic native type rows입니다.
	TArray<FCFDANativeTypeMetadata> NativeTypes;
	NativeTypes.Add({AlphaClassPath, TEXT("CFAlphaData"), false});
	NativeTypes.Add({FutureClassPath, TEXT("CFFutureData"), false});

	// persisted coverage bridge를 검증할 synthetic asset rows입니다.
	TArray<FCFDAPersistedAssetMetadata> PersistedAssets;
	PersistedAssets.Add({
		TEXT("DA_Alpha"),
		TEXT("/Game/CarFight/Data/DA_Alpha.DA_Alpha"),
		TEXT("/Game/CarFight/Data"),
		AlphaClassPath});
	PersistedAssets.Add({
		TEXT("DA_Future"),
		TEXT("/Game/CarFight/Data/DA_Future.DA_Future"),
		TEXT("/Game/CarFight/Data"),
		FutureClassPath});

	// Registry overload를 통해 만든 metadata-only synthetic inventory입니다.
	const FCFDAInventoryResult InventoryResult = FCFDAAuditService::BuildInventory(
		ECFDARegistryState::Ready,
		FString(),
		NativeTypes,
		PersistedAssets,
		TypeRegistry);

	// registered Alpha inventory type입니다.
	const FCFDATypeRecord* AlphaType =
		CFDARegistryTestsPrivate::FindTypeRecord(InventoryResult, AlphaClassPath);
	TestNotNull(TEXT("Alpha inventory type visible"), AlphaType);
	if (AlphaType)
	{
		TestEqual(TEXT("Alpha inventory coverage registered"), AlphaType->CoverageState, ECFDACoverageState::Registered);
	}

	// unregistered Future inventory type입니다.
	const FCFDATypeRecord* FutureType =
		CFDARegistryTestsPrivate::FindTypeRecord(InventoryResult, FutureClassPath);
	TestNotNull(TEXT("Future inventory type visible"), FutureType);
	if (FutureType)
	{
		TestEqual(TEXT("Future inventory coverage unregistered"), FutureType->CoverageState, ECFDACoverageState::Unregistered);
	}

	TestEqual(TEXT("Alpha asset coverage registered"), InventoryResult.AssetRecords[0].CoverageState, ECFDACoverageState::Registered);
	TestEqual(TEXT("Future asset coverage unregistered"), InventoryResult.AssetRecords[1].CoverageState, ECFDACoverageState::Unregistered);
	return true;
}

// empty Registry에서도 current native/persisted inventory가 사라지지 않고 Unregistered fallback으로 남는지 검증합니다.
bool FCFDARegistryCurrentFallbackTest::RunTest(const FString& Parameters)
{
	// current Editor process의 Asset Registry module입니다.
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// current Editor process의 Asset Registry interface입니다.
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// integration assertion 전에 Registry completion prerequisite를 충족합니다.
	AssetRegistry.SearchAllAssets(true);
	TestFalse(TEXT("Asset Registry completion prerequisite"), AssetRegistry.IsLoadingAssets());

	// DAM-P0-02B semantic mapping 전의 intentionally empty Registry입니다.
	FCFDATypeRegistry EmptyRegistry;

	// empty Registry coverage를 적용한 current metadata-only inventory입니다.
	const FCFDAInventoryResult Result = FCFDAAuditService::Refresh(EmptyRegistry);
	TestEqual(TEXT("Current fallback registry ready"), Result.RegistryState, ECFDARegistryState::Ready);
	TestTrue(TEXT("Current fallback inventory complete"), Result.IsInventoryComplete());
	TestTrue(TEXT("Current native type baseline retained"), Result.TypeRecords.Num() >= 28);
	TestTrue(TEXT("Current persisted asset baseline retained"), Result.AssetRecords.Num() >= 53);

	// current canonical native type 수입니다.
	int32 CanonicalNativeTypeCount = 0;

	// current canonical native types가 모두 Unregistered로 보이는지 집계합니다.
	int32 CanonicalUnregisteredTypeCount = 0;

	// current type row를 순회하며 fallback semantic도 함께 검증합니다.
	for (const FCFDATypeRecord& TypeRecord : Result.TypeRecords)
	{
		if (!TypeRecord.bCanonicalNative)
		{
			continue;
		}

		++CanonicalNativeTypeCount;
		if (TypeRecord.CoverageState == ECFDACoverageState::Unregistered)
		{
			++CanonicalUnregisteredTypeCount;
		}

		// 현재 Type row의 Unregistered semantic fallback입니다.
		const FCFDAResolvedSemantic FallbackSemantic =
			EmptyRegistry.ResolveSemantic(TypeRecord.ClassPath, TypeRecord.TechnicalClassName);
		TestEqual(
			FString::Printf(TEXT("Fallback coverage: %s"), *TypeRecord.ClassPath),
			FallbackSemantic.CoverageState,
			ECFDACoverageState::Unregistered);
		TestEqual(
			FString::Printf(TEXT("Fallback domain: %s"), *TypeRecord.ClassPath),
			FallbackSemantic.Descriptor.Domain,
			ECFDADomain::Unclassified);
		TestTrue(
			FString::Printf(TEXT("Fallback display exists: %s"), *TypeRecord.ClassPath),
			!FallbackSemantic.Descriptor.TypeDisplayName.IsEmpty());
	}

	TestTrue(TEXT("Current canonical native baseline >= 28"), CanonicalNativeTypeCount >= 28);
	TestEqual(
		TEXT("All current canonical native types remain Unregistered before P0-02B mapping"),
		CanonicalUnregisteredTypeCount,
		CanonicalNativeTypeCount);
	return true;
}

#endif
