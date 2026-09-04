// Copyright (c) CarFight. All Rights Reserved.
// File: CFDACurrentTypeTests.cpp
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02B current concrete type semantics focused Automation입니다.
// Changelog:
// - v1.1.0: 미래 concrete DA 확장 허용, exact SourceName, source symbol compile guard, batch atomicity, duplicate namespace policy를 검증.
// - v1.0.2: VehicleData 공개 read-only VDA validator를 CustomContract expected matrix에 반영.
// - v1.0.1: Identity Required/N/A와 resolver kind 분리 계약을 current 27종 matrix에 반영.
// - v1.0.0: current 27 concrete descriptor matrix, policy completeness, idempotent registration과 metadata-only Coverage integration을 검증.
// Migration:
// - Asset load, Stable ID resolve, Validation 실행, Duplicate ID 분석, Reference/Referencer, UI mutation을 수행하지 않습니다.

#include "DataManagement/CFDAAuditService.h"
#include "DataManagement/CFDATypeRegistry.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFAmmoData.h"
#include "CFCombatFxData.h"
#include "CFDamageData.h"
#include "CFEquipmentPresetData.h"
#include "CFInventoryItemData.h"
#include "CFProjectileData.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFTargetSelectData.h"
#include "CFTurretMountData.h"
#include "CFVDAValidator.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"

namespace CFDACurrentTypeTestsPrivate
{
	// current Source audit에서 고정한 descriptor 핵심 정책 기대값입니다.
	struct FExpectedDescriptor
	{
		// Unreal canonical native class path입니다.
		const TCHAR* ClassPath;

		// 사용자 Overview Domain입니다.
		ECFDADomain Domain;

		// stable identity의 Required/Optional/N/A 정책입니다.
		ECFDAIdentityPolicy IdentityPolicy;

		// stable identity의 actual Source resolver 종류입니다.
		ECFDAIdentityResolverKind IdentityResolverKind;

		// actual Source에서 확인한 exact identity field/function 이름입니다.
		const TCHAR* IdentitySourceName;

		// loaded health lane 선언 정책입니다.
		ECFDAValidationPolicy ValidationPolicy;

		// actual Source에서 확인한 exact validation entry 이름입니다.
		const TCHAR* ValidationSourceName;

		// stable ID duplicate 비교 namespace derivation 정책입니다.
		ECFDADuplicateNamespacePolicy DuplicateNamespacePolicy;
	};

	// current concrete 27종의 exact descriptor policy matrix를 반환합니다.
	TArray<FExpectedDescriptor> BuildExpectedDescriptors()
	{
		// P0-02B actual Source audit 기준 current concrete descriptor matrix입니다.
		return
		{
			{TEXT("/Script/CarFight_Re.CFAmmoData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("AmmoId"), ECFDAValidationPolicy::CustomContract, TEXT("IsAmmoDataValid"), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFCombatFxData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("CombatFxId"), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFDamageData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("DamageId"), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFEquipmentPresetData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("EquipmentId"), ECFDAValidationPolicy::CustomContract, TEXT("HasCompleteEquipmentData"), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFEquipmentItemData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::PrimaryAssetId, TEXT("ItemDefinitionId"), ECFDAValidationPolicy::NativeDataValidation, TEXT("IsDataValid"), ECFDADuplicateNamespacePolicy::PrimaryAssetType},
			{TEXT("/Script/CarFight_Re.CFDefenseItemData"), ECFDADomain::Vehicle, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::PrimaryAssetId, TEXT("ItemDefinitionId"), ECFDAValidationPolicy::NativeDataValidation, TEXT("IsDataValid"), ECFDADuplicateNamespacePolicy::PrimaryAssetType},
			{TEXT("/Script/CarFight_Re.CFProjectileData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("ProjectileId"), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFRuntimeTestCatalogData"), ECFDADomain::Authoring, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::CustomContract, TEXT("ValidateRuntimeTestCatalog"), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_Re.CFTargetSelectData"), ECFDADomain::Targeting, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::CustomContract, TEXT("IsTargetSelectConfigValid"), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_Re.CFTurretMountData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("TurretMountId"), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFVehicleCameraData"), ECFDADomain::Vehicle, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_Re.CFVehicleData"), ECFDADomain::Vehicle, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::PrimaryAssetId, TEXT("GetPrimaryAssetId"), ECFDAValidationPolicy::CustomContract, TEXT("UCFVDAValidator::ValidateVehicleData"), ECFDADuplicateNamespacePolicy::PrimaryAssetType},
			{TEXT("/Script/CarFight_Re.CFVehicleDefenseData"), ECFDADomain::Vehicle, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("DefenseId"), ECFDAValidationPolicy::NativeDataValidation, TEXT("IsDataValid"), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFVehicleFittingData"), ECFDADomain::Vehicle, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("FittingId"), ECFDAValidationPolicy::NativeDataValidation, TEXT("IsDataValid"), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFVehicleSensorData"), ECFDADomain::Targeting, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::NativeDataValidation, TEXT("IsDataValid"), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_Re.CFWeaponData"), ECFDADomain::Combat, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("WeaponId"), ECFDAValidationPolicy::NativeDataValidation, TEXT("IsDataValid"), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFHUDLayoutData"), ECFDADomain::UI, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitFName, TEXT("ProfileId"), ECFDAValidationPolicy::CustomContract, TEXT("ValidateLayoutData"), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_Re.CFHUDVisualData"), ECFDADomain::UI, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_Re.CFUIDensityData"), ECFDADomain::UI, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::CustomContract, TEXT("ValidateDensityData"), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_Re.CFUIStyleData"), ECFDADomain::UI, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::CustomContract, TEXT("ValidateStyleData"), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_ReEditor.CFDriveStateProfile"), ECFDADomain::Authoring, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_ReEditor.CFDrivetrainProfile"), ECFDADomain::Authoring, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_ReEditor.CFHandlingProfile"), ECFDADomain::Authoring, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_ReEditor.CFPerformanceProfile"), ECFDADomain::Authoring, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_ReEditor.CFVehicleBaseProfile"), ECFDADomain::Authoring, ECFDAIdentityPolicy::NotApplicable, ECFDAIdentityResolverKind::None, TEXT(""), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::NotApplicable},
			{TEXT("/Script/CarFight_ReEditor.CFVehicleRecipeData"), ECFDADomain::Authoring, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitGuid, TEXT("RecipeId"), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::ExactClassPath},
			{TEXT("/Script/CarFight_ReEditor.CFVehicleRefEvidence"), ECFDADomain::Authoring, ECFDAIdentityPolicy::Required, ECFDAIdentityResolverKind::ExplicitGuid, TEXT("EvidenceId"), ECFDAValidationPolicy::None, TEXT(""), ECFDADuplicateNamespacePolicy::ExactClassPath}
		};
	}

	// actual Source symbol rename이 descriptor 문자열 테스트와 함께 compile-time에 드러나도록 audited symbol을 직접 참조합니다.
	void VerifyAuditedSourceSymbolsCompile()
	{
		// Explicit identity UPROPERTY symbol들입니다.
		(void)GET_MEMBER_NAME_CHECKED(UCFAmmoData, AmmoId);
		(void)GET_MEMBER_NAME_CHECKED(UCFCombatFxData, CombatFxId);
		(void)GET_MEMBER_NAME_CHECKED(UCFDamageData, DamageId);
		(void)GET_MEMBER_NAME_CHECKED(UCFEquipmentPresetData, EquipmentId);
		(void)GET_MEMBER_NAME_CHECKED(UCFInventoryItemData, ItemDefinitionId);
		(void)GET_MEMBER_NAME_CHECKED(UCFProjectileData, ProjectileId);
		(void)GET_MEMBER_NAME_CHECKED(UCFTurretMountData, TurretMountId);
		(void)GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseData, DefenseId);
		(void)GET_MEMBER_NAME_CHECKED(UCFVehicleFittingData, FittingId);
		(void)GET_MEMBER_NAME_CHECKED(UCFWeaponData, WeaponId);
		(void)GET_MEMBER_NAME_CHECKED(UCFHUDLayoutData, ProfileId);
		(void)GET_MEMBER_NAME_CHECKED(UCFVehicleRecipeData, RecipeId);
		(void)GET_MEMBER_NAME_CHECKED(UCFVehicleRefEvidence, EvidenceId);

		// Custom validation entry symbol들입니다.
		(void)&UCFAmmoData::IsAmmoDataValid;
		(void)&UCFEquipmentPresetData::HasCompleteEquipmentData;
		(void)&UCFRuntimeTestCatalogData::ValidateRuntimeTestCatalog;
		(void)&UCFTargetSelectData::IsTargetSelectConfigValid;
		(void)&UCFVDAValidator::ValidateVehicleData;
		(void)&UCFHUDLayoutData::ValidateLayoutData;
		(void)&UCFUIDensityData::ValidateDensityData;
		(void)&UCFUIStyleData::ValidateStyleData;

		// Native Data Validation entry symbol입니다.
		(void)&UCFInventoryItemData::IsDataValid;
		(void)&UCFVehicleDefenseData::IsDataValid;
		(void)&UCFVehicleFittingData::IsDataValid;
		(void)&UCFVehicleSensorData::IsDataValid;
		(void)&UCFWeaponData::IsDataValid;

		// PrimaryAssetId resolver entry가 존재함을 compile-time에 확인합니다.
		(void)&UCFInventoryItemData::GetPrimaryAssetId;
		(void)&UCFVehicleData::GetPrimaryAssetId;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDACurrentDescriptorMatrixTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02B.CurrentDescriptorMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDACurrentCoverageIntegrationTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_02B.CurrentCoverageIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// current 27 concrete descriptor의 exact semantic/policy/source, batch atomicity와 namespace derivation을 검증합니다.
bool FCFDACurrentDescriptorMatrixTest::RunTest(const FString& Parameters)
{
	CFDACurrentTypeTestsPrivate::VerifyAuditedSourceSymbolsCompile();

	// current concrete descriptor를 등록할 fresh Registry입니다.
	FCFDATypeRegistry TypeRegistry;

	// current descriptor registration 오류입니다.
	FString RegistrationError;
	TestTrue(
		TEXT("Current descriptors register"),
		TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError));
	TestTrue(TEXT("Current registration has no error"), RegistrationError.IsEmpty());

	// actual Source audit 기준 expected descriptor matrix입니다.
	const TArray<CFDACurrentTypeTestsPrivate::FExpectedDescriptor> ExpectedDescriptors =
		CFDACurrentTypeTestsPrivate::BuildExpectedDescriptors();
	TestEqual(TEXT("Current descriptor matrix count"), ExpectedDescriptors.Num(), 27);
	TestEqual(TEXT("Registry contains exact current descriptors"), TypeRegistry.Num(), ExpectedDescriptors.Num());

	// current descriptor 27종의 exact semantic/policy/source를 하나씩 확인합니다.
	for (const CFDACurrentTypeTestsPrivate::FExpectedDescriptor& ExpectedDescriptor : ExpectedDescriptors)
	{
		// expected class path에 등록된 current semantic descriptor입니다.
		const FCFDASemanticDescriptor* Descriptor = TypeRegistry.FindDescriptor(ExpectedDescriptor.ClassPath);
		TestNotNull(FString::Printf(TEXT("Descriptor exists: %s"), ExpectedDescriptor.ClassPath), Descriptor);
		if (!Descriptor)
		{
			continue;
		}

		TestEqual(FString::Printf(TEXT("Domain: %s"), ExpectedDescriptor.ClassPath), Descriptor->Domain, ExpectedDescriptor.Domain);
		TestTrue(FString::Printf(TEXT("Type display name exists: %s"), ExpectedDescriptor.ClassPath), !Descriptor->TypeDisplayName.IsEmpty());
		TestTrue(FString::Printf(TEXT("User purpose exists: %s"), ExpectedDescriptor.ClassPath), !Descriptor->UserPurposeDescription.IsEmpty());
		TestTrue(FString::Printf(TEXT("User usage exists: %s"), ExpectedDescriptor.ClassPath), !Descriptor->UserUsageDescription.IsEmpty());
		TestTrue(FString::Printf(TEXT("Role description exists: %s"), ExpectedDescriptor.ClassPath), !Descriptor->RoleDescription.IsEmpty());
		TestEqual(FString::Printf(TEXT("Identity policy: %s"), ExpectedDescriptor.ClassPath), Descriptor->IdentityPolicy, ExpectedDescriptor.IdentityPolicy);
		TestEqual(FString::Printf(TEXT("Identity resolver: %s"), ExpectedDescriptor.ClassPath), Descriptor->IdentityResolverKind, ExpectedDescriptor.IdentityResolverKind);
		TestEqual(FString::Printf(TEXT("Identity source exact: %s"), ExpectedDescriptor.ClassPath), Descriptor->IdentitySourceName, FString(ExpectedDescriptor.IdentitySourceName));
		TestEqual(FString::Printf(TEXT("Validation policy: %s"), ExpectedDescriptor.ClassPath), Descriptor->ValidationPolicy, ExpectedDescriptor.ValidationPolicy);
		TestEqual(FString::Printf(TEXT("Validation source exact: %s"), ExpectedDescriptor.ClassPath), Descriptor->ValidationSourceName, FString(ExpectedDescriptor.ValidationSourceName));
		TestEqual(
			FString::Printf(TEXT("Duplicate namespace policy: %s"), ExpectedDescriptor.ClassPath),
			FCFDATypeRegistry::ResolveDuplicateNamespacePolicy(*Descriptor),
			ExpectedDescriptor.DuplicateNamespacePolicy);
	}

	// 같은 current descriptor set을 재등록한 뒤에도 count가 늘지 않는지 확인합니다.
	TestTrue(
		TEXT("Current descriptor registration is idempotent"),
		TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError));
	TestTrue(TEXT("Idempotent current registration has no error"), RegistrationError.IsEmpty());
	TestEqual(TEXT("Idempotent current descriptor count"), TypeRegistry.Num(), 27);

	// abstract framework base는 current concrete registry에 등록하지 않는 계약입니다.
	TestNull(
		TEXT("Abstract InventoryItem base is not semantic concrete mapping"),
		TypeRegistry.FindDescriptor(TEXT("/Script/CarFight_Re.CFInventoryItemData")));

	// batch 중간 conflict를 만들기 위해 current VehicleData class path를 다른 semantic으로 선점한 Registry입니다.
	FCFDATypeRegistry ConflictRegistry;

	// current VehicleData와 exact class path만 같고 semantic은 다른 유효 descriptor입니다.
	FCFDASemanticDescriptor ConflictingVehicleDescriptor;
	ConflictingVehicleDescriptor.ClassPath = TEXT("/Script/CarFight_Re.CFVehicleData");
	ConflictingVehicleDescriptor.Domain = ECFDADomain::Combat;
	ConflictingVehicleDescriptor.TypeDisplayName = TEXT("충돌 차량 데이터");
	ConflictingVehicleDescriptor.RoleDescription = TEXT("Batch atomicity 검증용 synthetic conflicting descriptor입니다.");
	TestTrue(
		TEXT("Preexisting conflict descriptor registers"),
		ConflictRegistry.RegisterDescriptor(ConflictingVehicleDescriptor, &RegistrationError));
	TestEqual(TEXT("Conflict registry starts with one descriptor"), ConflictRegistry.Num(), 1);

	// current batch가 VehicleData에서 실패해도 앞선 descriptor들이 원본 Registry에 남지 않아야 합니다.
	TestFalse(
		TEXT("Current descriptor batch conflict fails"),
		ConflictRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError));
	TestTrue(TEXT("Batch conflict reports error"), !RegistrationError.IsEmpty());
	TestEqual(TEXT("Failed current batch preserves original registry count"), ConflictRegistry.Num(), 1);

	// 실패 후에도 선점한 descriptor가 exact 그대로 보존되는지 확인합니다.
	const FCFDASemanticDescriptor* PreservedConflict =
		ConflictRegistry.FindDescriptor(TEXT("/Script/CarFight_Re.CFVehicleData"));
	TestNotNull(TEXT("Preexisting conflict survives failed batch"), PreservedConflict);
	if (PreservedConflict)
	{
		TestEqual(TEXT("Failed batch preserves preexisting conflict domain"), PreservedConflict->Domain, ECFDADomain::Combat);
		TestEqual(TEXT("Failed batch preserves preexisting conflict display"), PreservedConflict->TypeDisplayName, FString(TEXT("충돌 차량 데이터")));
	}

	// batch 전에 없던 앞선 current descriptor가 실패한 원본 Registry에 누출되지 않았는지 확인합니다.
	TestNull(
		TEXT("Failed batch does not leak earlier Ammo descriptor"),
		ConflictRegistry.FindDescriptor(TEXT("/Script/CarFight_Re.CFAmmoData")));
	return true;
}

// current audited 27종은 Registered로 보장하되 미래 concrete DA는 Unregistered 상태로 추가되어도 실패하지 않는지 검증합니다.
bool FCFDACurrentCoverageIntegrationTest::RunTest(const FString& Parameters)
{
	// current Editor process의 Asset Registry module입니다.
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// current Editor process의 Asset Registry interface입니다.
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// current inventory integration assertion 전에 Registry completion prerequisite를 충족합니다.
	AssetRegistry.SearchAllAssets(true);
	TestFalse(TEXT("Asset Registry completion prerequisite"), AssetRegistry.IsLoadingAssets());

	// current semantic/policy descriptors를 소유할 Registry입니다.
	FCFDATypeRegistry TypeRegistry;

	// current descriptor registration 오류입니다.
	FString RegistrationError;
	if (!TestTrue(
		TEXT("Current descriptors register for integration"),
		TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError)))
	{
		AddError(RegistrationError);
		return false;
	}

	// current semantic Registry coverage를 적용한 metadata-only inventory입니다.
	const FCFDAInventoryResult Result = FCFDAAuditService::Refresh(TypeRegistry);
	TestEqual(TEXT("Current semantic inventory registry ready"), Result.RegistryState, ECFDARegistryState::Ready);
	TestTrue(TEXT("Current semantic inventory complete"), Result.IsInventoryComplete());
	TestTrue(TEXT("Current native type baseline retained"), Result.TypeRecords.Num() >= 28);
	TestTrue(TEXT("Current persisted asset baseline retained"), Result.AssetRecords.Num() >= 53);

	// current audited descriptor class path 집합입니다.
	TSet<FString> ExpectedCurrentClassPaths;
	for (const CFDACurrentTypeTestsPrivate::FExpectedDescriptor& ExpectedDescriptor :
		CFDACurrentTypeTestsPrivate::BuildExpectedDescriptors())
	{
		ExpectedCurrentClassPaths.Add(ExpectedDescriptor.ClassPath);
	}

	// actual inventory에서 발견된 current audited descriptor 수입니다.
	int32 FoundExpectedCurrentTypeCount = 0;

	// current baseline abstract framework base가 발견됐는지 나타냅니다.
	bool bFoundInventoryItemAbstractBase = false;

	// canonical native type 전체를 순회하되 future type의 Unregistered 추가를 허용합니다.
	for (const FCFDATypeRecord& TypeRecord : Result.TypeRecords)
	{
		if (!TypeRecord.bCanonicalNative)
		{
			continue;
		}

		if (ExpectedCurrentClassPaths.Contains(TypeRecord.ClassPath))
		{
			++FoundExpectedCurrentTypeCount;
			TestFalse(
				FString::Printf(TEXT("Audited current type remains concrete: %s"), *TypeRecord.ClassPath),
				TypeRecord.bAbstract);
			TestEqual(
				FString::Printf(TEXT("Audited current type remains Registered: %s"), *TypeRecord.ClassPath),
				TypeRecord.CoverageState,
				ECFDACoverageState::Registered);
			TestNotNull(
				FString::Printf(TEXT("Audited current semantic exists: %s"), *TypeRecord.ClassPath),
				TypeRegistry.FindDescriptor(TypeRecord.ClassPath));
			continue;
		}

		if (TypeRecord.ClassPath == TEXT("/Script/CarFight_Re.CFInventoryItemData"))
		{
			bFoundInventoryItemAbstractBase = true;
			TestTrue(TEXT("InventoryItem framework base remains abstract"), TypeRecord.bAbstract);
			TestEqual(
				TEXT("InventoryItem framework base remains Unregistered"),
				TypeRecord.CoverageState,
				ECFDACoverageState::Unregistered);
			continue;
		}

		// 미래 native 타입은 Generic Discovery에 보이는 것 자체가 정상이며 descriptor 등록 전 Unregistered를 허용합니다.
		TestTrue(
			FString::Printf(TEXT("Future canonical type remains visible with legal coverage: %s"), *TypeRecord.ClassPath),
			TypeRecord.CoverageState == ECFDACoverageState::Registered
				|| TypeRecord.CoverageState == ECFDACoverageState::Unregistered);
	}

	TestEqual(TEXT("All audited current concrete types remain discoverable"), FoundExpectedCurrentTypeCount, 27);
	TestTrue(TEXT("Current abstract InventoryItem base remains discoverable"), bFoundInventoryItemAbstractBase);
	return true;
}

#endif
