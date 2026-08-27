// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthTests.cpp
// Version: v1.12.0
// Date: 2026-08-27
// Description: DAUTH Foundation + CF-FQ-040 VB-P0-07 persistent provenance receipt / post-state guarded Undo hardening Automation입니다.
// Changelog:
// - v1.12.0: Final Review provenance를 실제 receipt-only Builder Profile commit으로 준비하고 Claim subset tamper를 차단하며, transaction 밖 post-Apply Target drift가 guarded Undo에서 StateChanged로 차단되는지 검증.
// - v1.11.0: Final Review가 existing Validation/Drift/Gameplay/Diff와 consumed Evidence provenance를 집계하고 explicit R3 Apply → diff0 → exact transaction guarded Undo → diff restore를 수행하는지 transient-only로 검증.
// - v1.10.0: NewVehicle USER Socket authority, optional gameplay defaults, Fitting mass와 CompleteExisting stored Hardpoint baseline preservation을 transient-only로 검증.
// - v1.9.0: CFVRN-1 Unicode NFC 계약을 조합형/분해형 canonical-equivalent 문자열의 동일 Evidence fingerprint로 검증.
// - v1.8.0: Vehicle Reference Evidence의 array order/diagnostic exclusion과 semantic claim-change fingerprint 변화를 transient-only Automation으로 추가.
// - v1.7.0: 설계 검수 교정으로 TransmissionRatios를 atomic descriptor 1개로 검증하고 Registry/Reflection 기대치를 127로 정정.
// - v1.6.0: Registry/Reflection 128 coverage, ChassisWidth/Transmission descriptor dependency, TArray<float> ratio codec round-trip와 OwnerRecipeId snapshot separation을 검증.
// - v1.5.0: UI-P0-06 explicit RedlineStartRPM schema 호환을 위해 Current Registry/Reflection coverage를 118로 검증하고 PerformanceProfileDirect descriptor/dependency를 확인.
// - v1.4.0: DAUTH-P0-08D Chassis socket / Wheel bounds / asset fingerprint Snapshot Reader 검증 추가.
// - v1.3.0: DAUTH-P0-08C RequiredDependencies, Recipe/Profile/Definition/Project Default Snapshot과 deterministic fingerprint 검증 추가.
// - v1.2.0: Codec trailing text 차단과 UTF-8 deterministic hash 안정성 검증을 추가하고 formatting을 교정.
// - v1.1.0: Recipe duplicate가 원본 RecipeId를 재사용하지 않는 identity 검증을 추가.
// - v1.0.0: Editor-only asset contract, 당시 117 coverage, stable path, float codec round-trip 검증을 추가.
// Migration:
// - Runtime/Content Asset mutation 없이 CDO와 transient value만 검사합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "DataAuthoring/CFVehicleAssetReader.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"
#include "CFVehicleData.h"
#include "Editor.h"
#include "Editor/Transactor.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "ScopedTransaction.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringEditorOnlyTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Foundation.EditorOnlyAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringRegistryTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.Foundation.Registry127",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringFieldPathTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Foundation.StableFieldPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringCodecTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Foundation.FieldCodec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringDependencyTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Snapshot.RequiredDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleRecipeSnapshotTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Snapshot.RecipeFingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleProfileSnapshotTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Snapshot.ProfileSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleDefinitionSnapshotTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Snapshot.DefinitionAndProjectDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAssetSnapshotTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Snapshot.AssetReader",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleRefEvidenceTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.Evidence.DeterministicFingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderGameplayGuideTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderFinalReviewTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_07.FinalReviewApplyUndo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Recipe와 5개 Profile CDO가 explicit Editor-only object로 판정되는지 검증합니다.
bool FCFVehicleAuthoringEditorOnlyTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Recipe IsEditorOnly"), GetDefault<UCFVehicleRecipeData>()->IsEditorOnly());
	TestTrue(TEXT("VehicleBase Profile IsEditorOnly"), GetDefault<UCFVehicleBaseProfile>()->IsEditorOnly());
	TestTrue(TEXT("Drivetrain Profile IsEditorOnly"), GetDefault<UCFDrivetrainProfile>()->IsEditorOnly());
	TestTrue(TEXT("Handling Profile IsEditorOnly"), GetDefault<UCFHandlingProfile>()->IsEditorOnly());
	TestTrue(TEXT("Performance Profile IsEditorOnly"), GetDefault<UCFPerformanceProfile>()->IsEditorOnly());
		TestTrue(TEXT("DriveState Profile IsEditorOnly"), GetDefault<UCFDriveStateProfile>()->IsEditorOnly());
	TestTrue(TEXT("Recipe IsEditorOnlyObject"), IsEditorOnlyObject(GetDefault<UCFVehicleRecipeData>(), true));

	// Duplicate identity 검증 원본으로 사용할 transient Recipe입니다.
	UCFVehicleRecipeData* SourceRecipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient source Recipe exists"), SourceRecipe))
	{
		return false;
	}

	// 원본과 별도 identity를 받아야 하는 transient duplicate Recipe입니다.
	UCFVehicleRecipeData* DuplicateRecipe = DuplicateObject<UCFVehicleRecipeData>(SourceRecipe, GetTransientPackage());
	if (!TestNotNull(TEXT("Transient duplicate Recipe exists"), DuplicateRecipe))
	{
		return false;
	}

	TestTrue(TEXT("Source RecipeId is valid"), SourceRecipe->RecipeId.IsValid());
	TestTrue(TEXT("Duplicate RecipeId is valid"), DuplicateRecipe->RecipeId.IsValid());
	TestNotEqual(TEXT("Duplicate receives a new RecipeId"), DuplicateRecipe->RecipeId, SourceRecipe->RecipeId);
	return true;
}

// Current Registry 127개와 Current UCFVehicleData Reflection leaf 127개의 양방향 coverage 및 Builder 추가 descriptor를 검증합니다.
bool FCFVehicleAuthoringRegistryTest::RunTest(const FString& Parameters)
{
	// Coverage가 발견한 상세 오류 목록입니다.
	TArray<FString> CoverageErrors;
	// Registry와 Current source의 양방향 coverage 결과입니다.
	const bool bCoverageValid = FCFVehicleFieldRegistry::ValidateCoverage(CoverageErrors);

		TestEqual(TEXT("Current Registry descriptor count"), FCFVehicleFieldRegistry::GetDescriptors().Num(), FCFVehicleFieldRegistry::ExpectedLeafPatternCount);
	TestEqual(TEXT("Current reflection leaf count"), FCFVehicleFieldRegistry::DiscoverDefinitionLeafPatterns().Num(), FCFVehicleFieldRegistry::ExpectedLeafPatternCount);

	// [v1.5.0] explicit RedlineStartRPM의 current Registry descriptor입니다.
	const FCFVehicleFieldDescriptor* RedlineDescriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([](const FCFVehicleFieldDescriptor& Descriptor)
	{
		return Descriptor.GetCanonicalPattern() == TEXT("VehicleMovementConfig.RedlineStartRPM");
	});
	if (TestNotNull(TEXT("RedlineStartRPM Registry descriptor exists"), RedlineDescriptor))
	{
		TestEqual(TEXT("RedlineStartRPM owner domain Performance"), RedlineDescriptor->PrimaryProfileDomain, ECFVehicleProfileDomain::Performance);
		TestEqual(TEXT("RedlineStartRPM resolve rule PerformanceProfileDirect"), RedlineDescriptor->ResolveRule, ECFVehicleResolveRule::PerformanceProfileDirect);
		TestTrue(TEXT("RedlineStartRPM depends on Profile.Performance"), RedlineDescriptor->RequiredDependencies.Contains(FName(TEXT("Profile.Performance"))));
	}

	// [v1.6.0] Reference wheel fallback + AssetDerived 우선 정책을 가진 전륜 반지름 descriptor입니다.
	const FCFVehicleFieldDescriptor* FrontRadiusDescriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([](const FCFVehicleFieldDescriptor& Descriptor)
	{
		return Descriptor.GetCanonicalPattern() == TEXT("VehicleMovementConfig.FrontWheelRadius");
	});
	if (TestNotNull(TEXT("FrontWheelRadius Registry descriptor exists"), FrontRadiusDescriptor))
	{
		TestEqual(TEXT("FrontWheelRadius owner domain VehicleBase"), FrontRadiusDescriptor->PrimaryProfileDomain, ECFVehicleProfileDomain::VehicleBase);
		TestEqual(TEXT("FrontWheelRadius resolve rule BaseProfileMeasurementPolicy"), FrontRadiusDescriptor->ResolveRule, ECFVehicleResolveRule::BaseProfileMeasurementPolicy);
		TestTrue(TEXT("FrontWheelRadius depends on Profile.VehicleBase"), FrontRadiusDescriptor->RequiredDependencies.Contains(FName(TEXT("Profile.VehicleBase"))));
		TestTrue(TEXT("FrontWheelRadius depends on Asset.WheelBounds"), FrontRadiusDescriptor->RequiredDependencies.Contains(FName(TEXT("Asset.WheelBounds"))));
	}

	// [v1.7.0] 전진/후진 배열을 한 provenance로 묶는 atomic typed ratio-set descriptor입니다.
	const FCFVehicleFieldDescriptor* TransmissionRatiosDescriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([](const FCFVehicleFieldDescriptor& Descriptor)
	{
		return Descriptor.GetCanonicalPattern() == TEXT("VehicleMovementConfig.TransmissionRatios");
	});
	if (TestNotNull(TEXT("TransmissionRatios atomic Registry descriptor exists"), TransmissionRatiosDescriptor))
	{
		TestEqual(TEXT("TransmissionRatios owner domain Drivetrain"), TransmissionRatiosDescriptor->PrimaryProfileDomain, ECFVehicleProfileDomain::Drivetrain);
		TestTrue(TEXT("TransmissionRatios depends on Profile.Drivetrain"), TransmissionRatiosDescriptor->RequiredDependencies.Contains(FName(TEXT("Profile.Drivetrain"))));
	}
	TestFalse(TEXT("ForwardGearRatios is not an independent Registry leaf"), FCFVehicleFieldRegistry::GetDescriptors().ContainsByPredicate([](const FCFVehicleFieldDescriptor& Descriptor)
	{
		return Descriptor.GetCanonicalPattern() == TEXT("VehicleMovementConfig.TransmissionRatios.ForwardGearRatios");
	}));
	TestFalse(TEXT("ReverseGearRatios is not an independent Registry leaf"), FCFVehicleFieldRegistry::GetDescriptors().ContainsByPredicate([](const FCFVehicleFieldDescriptor& Descriptor)
	{
		return Descriptor.GetCanonicalPattern() == TEXT("VehicleMovementConfig.TransmissionRatios.ReverseGearRatios");
	}));

	for (const FString& CoverageError : CoverageErrors)
	{
		AddError(CoverageError);
	}
	TestTrue(TEXT("Registry bidirectional coverage"), bCoverageValid);
	return true;
}

// Scalar와 Stable-ID array path가 canonical identity를 올바르게 만드는지 검증합니다.
bool FCFVehicleAuthoringFieldPathTest::RunTest(const FString& Parameters)
{
	// Scalar VehicleMovement field path입니다.
	FCFVehicleFieldPath ScalarPath;
	ScalarPath.PropertyChain = {TEXT("VehicleMovementConfig"), TEXT("EngineMaxTorque")};
	TestEqual(TEXT("Scalar canonical path"), ScalarPath.ToCanonicalString(), FString(TEXT("VehicleMovementConfig.EngineMaxTorque")));

	// 실제 Hardpoint stable selector를 가진 array field path입니다.
	FCFVehicleFieldPath ArrayPath;
	ArrayPath.CollectionPropertyName = TEXT("HardpointSlots");
	ArrayPath.SelectorKeyPropertyName = TEXT("LocationSlotId");
	ArrayPath.SelectorKeyValue = TEXT("Top_01");
	ArrayPath.PropertyChain = {TEXT("LocalLocation")};
	TestEqual(TEXT("Array canonical path"), ArrayPath.ToCanonicalString(), FString(TEXT("HardpointSlots[LocationSlotId=Top_01].LocalLocation")));
	return true;
}

// FProperty float value의 canonical export/import와 type mismatch 차단을 검증합니다.
bool FCFVehicleAuthoringCodecTest::RunTest(const FString& Parameters)
{
	// Movement struct에서 EngineMaxTorque reflection property를 찾습니다.
	FFloatProperty* TorqueProperty = FindFProperty<FFloatProperty>(FCFVehicleMovementConfig::StaticStruct(), GET_MEMBER_NAME_CHECKED(FCFVehicleMovementConfig, EngineMaxTorque));
	if (!TestNotNull(TEXT("EngineMaxTorque property exists"), TorqueProperty))
	{
		return false;
	}

	// Export source로 사용할 current-compatible movement value copy입니다.
	FCFVehicleMovementConfig SourceMovement;
	// Import destination으로 사용할 별도 movement value copy입니다.
	FCFVehicleMovementConfig DestinationMovement;
	DestinationMovement.EngineMaxTorque = 0.0f;

	// Generic codec export 결과입니다.
	FCFVehicleFieldValue ExportedValue;
	// Codec 실패 사유를 받을 문자열입니다.
	FString CodecError;
	const bool bExported = FCFVehicleFieldCodec::ExportValue(*TorqueProperty, &SourceMovement.EngineMaxTorque, ExportedValue, CodecError);
	TestTrue(TEXT("Float export succeeds"), bExported);
	TestTrue(TEXT("Type signature is populated"), !ExportedValue.PropertyTypeSignature.IsEmpty());
	TestTrue(TEXT("Canonical value is populated"), !ExportedValue.CanonicalValueText.IsEmpty());

	// Exported generic value를 별도 float storage에 복원한 결과입니다.
	const bool bImported = FCFVehicleFieldCodec::ImportValue(*TorqueProperty, &DestinationMovement.EngineMaxTorque, nullptr, ExportedValue, CodecError);
	TestTrue(TEXT("Float import succeeds"), bImported);
	TestEqual(TEXT("Float round trip"), DestinationMovement.EngineMaxTorque, SourceMovement.EngineMaxTorque);

	// Type mismatch 차단을 확인하기 위해 signature만 훼손한 value입니다.
	FCFVehicleFieldValue WrongTypeValue = ExportedValue;
	WrongTypeValue.PropertyTypeSignature = TEXT("BoolProperty");
		const bool bWrongTypeImported = FCFVehicleFieldCodec::ImportValue(*TorqueProperty, &DestinationMovement.EngineMaxTorque, nullptr, WrongTypeValue, CodecError);
	TestFalse(TEXT("Type mismatch is blocked"), bWrongTypeImported);

	// 정상 canonical float 뒤에 임의 trailing token을 붙인 invalid value입니다.
	FCFVehicleFieldValue TrailingTextValue = ExportedValue;
	TrailingTextValue.CanonicalValueText += TEXT(" trailing");
	const bool bTrailingTextImported = FCFVehicleFieldCodec::ImportValue(*TorqueProperty, &DestinationMovement.EngineMaxTorque, nullptr, TrailingTextValue, CodecError);
	TestFalse(TEXT("Trailing canonical text is blocked"), bTrailingTextImported);

	// 비ASCII canonical payload도 동일 입력에서는 동일 UTF-8 hash를 가져야 합니다.
	FCFVehicleFieldValue UnicodeValue;
	UnicodeValue.PropertyTypeSignature = TEXT("StringProperty");
	UnicodeValue.CanonicalValueText = TEXT("차량_테스트");
	// 동일 payload의 첫 deterministic hash입니다.
	const FString FirstUnicodeHash = FCFVehicleFieldCodec::HashValue(UnicodeValue);
	// 동일 payload를 다시 계산한 두 번째 deterministic hash입니다.
	const FString SecondUnicodeHash = FCFVehicleFieldCodec::HashValue(UnicodeValue);
	TestEqual(TEXT("UTF-8 hash is stable"), FirstUnicodeHash, SecondUnicodeHash);
		TestEqual(TEXT("MD5 hex length"), FirstUnicodeHash.Len(), 32);
	return true;
}

// Current Registry descriptor의 RequiredDependencies가 비어 있지 않고 대표 field별 exact dependency를 가지는지 검증합니다.
bool FCFVehicleAuthoringDependencyTest::RunTest(const FString& Parameters)
{
		// Current Registry descriptor의 immutable cache입니다.
	const TArray<FCFVehicleFieldDescriptor>& Descriptors = FCFVehicleFieldRegistry::GetDescriptors();
	for (const FCFVehicleFieldDescriptor& Descriptor : Descriptors)
	{
		TestTrue(
			FString::Printf(TEXT("RequiredDependencies populated: %s"), *Descriptor.GetCanonicalPattern()),
			!Descriptor.RequiredDependencies.IsEmpty());

		// Descriptor 하나에서 duplicate dependency key를 검출하기 위한 집합입니다.
		TSet<FName> UniqueDependencies;
		for (const FName Dependency : Descriptor.RequiredDependencies)
		{
			TestFalse(
				FString::Printf(TEXT("Dependency key is unique: %s / %s"), *Descriptor.GetCanonicalPattern(), *Dependency.ToString()),
				UniqueDependencies.Contains(Dependency));
			TestFalse(TEXT("Dependency key is not None"), Dependency.IsNone());
			UniqueDependencies.Add(Dependency);
		}
	}

	// Canonical path로 대표 descriptor를 찾아 expected dependency key를 검사하는 helper입니다.
	auto TestDescriptorDependencies = [this, &Descriptors](const TCHAR* CanonicalPath, std::initializer_list<const TCHAR*> ExpectedDependencies)
	{
		// 검사할 canonical path와 일치하는 descriptor입니다.
		const FCFVehicleFieldDescriptor* FoundDescriptor = Descriptors.FindByPredicate([CanonicalPath](const FCFVehicleFieldDescriptor& Descriptor)
		{
			return Descriptor.GetCanonicalPattern() == CanonicalPath;
		});
		if (!TestNotNull(FString::Printf(TEXT("Dependency descriptor exists: %s"), CanonicalPath), FoundDescriptor))
		{
			return;
		}

		for (const TCHAR* ExpectedDependency : ExpectedDependencies)
		{
			TestTrue(
				FString::Printf(TEXT("Dependency %s -> %s"), CanonicalPath, ExpectedDependency),
				FoundDescriptor->RequiredDependencies.Contains(FName(ExpectedDependency)));
		}
	};

	TestDescriptorDependencies(TEXT("VehicleVisualConfig.ChassisMesh"), {TEXT("Recipe.AssetIntent")});
	TestDescriptorDependencies(TEXT("VehicleLayoutConfig.bUseLayoutOverrides"), {TEXT("Resolved.LayoutState")});
	TestDescriptorDependencies(TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeLocation"), {TEXT("Asset.ChassisSockets"), TEXT("Recipe.AssetIntent")});
	TestDescriptorDependencies(TEXT("MountProfiles[MountProfileId=*].LocationSlotRef"), {TEXT("Recipe.MountIntents"), TEXT("Recipe.HardpointIntents")});
	TestDescriptorDependencies(TEXT("MountProfiles[MountProfileId=*].TurretYawMesh"), {TEXT("Recipe.ImportState.LegacySerialized")});
		TestDescriptorDependencies(TEXT("VehicleMovementConfig.EngineMaxTorque"), {TEXT("Profile.Performance"), TEXT("Recipe.DrivingFeelIntent"), TEXT("Resolver.OptionalBaseMassContext")});
	TestDescriptorDependencies(TEXT("VehicleMovementConfig.RedlineStartRPM"), {TEXT("Profile.Performance")});
	TestDescriptorDependencies(TEXT("VehicleMovementConfig.FrontWheelSpringRate"), {TEXT("Profile.Handling"), TEXT("Recipe.DrivingFeelIntent"), TEXT("Resolver.OptionalBaseMassContext")});
	TestDescriptorDependencies(TEXT("VehicleMovementConfig.CenterOfMassOverride"), {TEXT("Project.CompatibilityDefaults"), TEXT("Recipe.AdvancedOverrides"), TEXT("Recipe.ImportState.LegacyPins")});
	TestDescriptorDependencies(TEXT("VehicleMovementConfig.FrontWheelRadius"), {TEXT("Asset.WheelBounds")});
	TestDescriptorDependencies(TEXT("DefaultDefenseData"), {TEXT("Profile.VehicleBase"), TEXT("Recipe.DefaultDataIntent")});
	TestDescriptorDependencies(TEXT("DriveStateConfig.IdleEnterSpeedThresholdKmh"), {TEXT("Recipe.DriveStateMode"), TEXT("Profile.DriveState")});
	return true;
}

// Recipe Snapshot이 live UObject 재조회 없이 value copy되고 AppliedState/Revision이 fingerprint를 오염시키지 않는지 검증합니다.
bool FCFVehicleRecipeSnapshotTest::RunTest(const FString& Parameters)
{
	// Snapshot source로 사용할 transient Editor-only Recipe입니다.
	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient Recipe exists"), Recipe))
	{
		return false;
	}

	Recipe->VehicleArchetypeId = TEXT("SnapshotTestVehicle");
	Recipe->DrivingFeelIntent.AccelerationFeel = 0.75f;
	Recipe->MassIntent.BaseMassMode = ECFAuthoringInputMode::ExplicitValue;
	Recipe->MassIntent.ExplicitBaseMassKg = 1570.0f;
	Recipe->AppliedState.AppliedDefinitionHash = TEXT("Applied_A");
	Recipe->AuthoringRevision = 10;

	// 첫 canonical Recipe Snapshot입니다.
	FCFVehicleRecipeSnapshot FirstSnapshot;
	// Snapshot Builder 실패 이유를 받는 문자열입니다.
	FString SnapshotError;
	const bool bFirstBuilt = FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, FirstSnapshot, SnapshotError);
	TestTrue(TEXT("First Recipe Snapshot builds"), bFirstBuilt);
	TestTrue(TEXT("Recipe fingerprint populated"), !FirstSnapshot.RecipeFingerprint.IsEmpty());
	TestEqual(TEXT("Recipe fingerprint length"), FirstSnapshot.RecipeFingerprint.Len(), 32);
	TestEqual(TEXT("Recipe intent copied"), FirstSnapshot.DrivingFeelIntent.AccelerationFeel, 0.75f);
	TestEqual(TEXT("Recipe mass copied"), FirstSnapshot.MassIntent.ExplicitBaseMassKg, 1570.0f);

	// AppliedState와 diagnostic revision만 바꾼 뒤 fingerprint exclusion을 확인합니다.
	Recipe->AppliedState.AppliedDefinitionHash = TEXT("Applied_B");
	Recipe->AuthoringRevision = 11;
	// Non-semantic state만 바뀐 두 번째 Recipe Snapshot입니다.
	FCFVehicleRecipeSnapshot NonSemanticSnapshot;
	const bool bNonSemanticBuilt = FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, NonSemanticSnapshot, SnapshotError);
	TestTrue(TEXT("Non-semantic Recipe Snapshot builds"), bNonSemanticBuilt);
	TestEqual(TEXT("AppliedState/revision do not change Recipe fingerprint"), NonSemanticSnapshot.RecipeFingerprint, FirstSnapshot.RecipeFingerprint);
	TestEqual(TEXT("AppliedState still copied into Snapshot"), NonSemanticSnapshot.AppliedState.AppliedDefinitionHash, FString(TEXT("Applied_B")));

	// 실제 resolver semantic intent를 바꿔 fingerprint가 달라지는지 확인합니다.
	Recipe->DrivingFeelIntent.AccelerationFeel = 0.80f;
	// Semantic intent가 바뀐 세 번째 Recipe Snapshot입니다.
	FCFVehicleRecipeSnapshot SemanticSnapshot;
	const bool bSemanticBuilt = FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, SemanticSnapshot, SnapshotError);
	TestTrue(TEXT("Semantic Recipe Snapshot builds"), bSemanticBuilt);
	TestTrue(TEXT("Semantic Recipe change updates fingerprint"), SemanticSnapshot.RecipeFingerprint != FirstSnapshot.RecipeFingerprint);
	return true;
}

// 5개 typed Profile이 source metadata와 payload value copy로 분리되고 display metadata가 payload fingerprint를 바꾸지 않는지 검증합니다.
bool FCFVehicleProfileSnapshotTest::RunTest(const FString& Parameters)
{
	// Vehicle Base transient Profile입니다.
	UCFVehicleBaseProfile* BaseProfile = NewObject<UCFVehicleBaseProfile>(GetTransientPackage());
	// Drivetrain transient Profile입니다.
	UCFDrivetrainProfile* DrivetrainProfile = NewObject<UCFDrivetrainProfile>(GetTransientPackage());
	// Handling transient Profile입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(GetTransientPackage());
	// Performance transient Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = NewObject<UCFPerformanceProfile>(GetTransientPackage());
	// DriveState transient Profile입니다.
	UCFDriveStateProfile* DriveStateProfile = NewObject<UCFDriveStateProfile>(GetTransientPackage());
	if (!TestNotNull(TEXT("Base Profile exists"), BaseProfile)
		|| !TestNotNull(TEXT("Drivetrain Profile exists"), DrivetrainProfile)
		|| !TestNotNull(TEXT("Handling Profile exists"), HandlingProfile)
		|| !TestNotNull(TEXT("Performance Profile exists"), PerformanceProfile)
		|| !TestNotNull(TEXT("DriveState Profile exists"), DriveStateProfile))
	{
		return false;
	}

	BaseProfile->Meta.DisplayName = FText::FromString(TEXT("표시 이름 A"));
	BaseProfile->Meta.Description = TEXT("fingerprint 제외 설명");
	BaseProfile->Meta.AuthoringRevision = 3;
	BaseProfile->Data.BaseVehicleMassKg = 1570.0f;
	DrivetrainProfile->Data.FrontRearSplit = 0.4f;
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = 1500.0f;
	PerformanceProfile->Data.EngineIdleRPM = 900.0f;
	DriveStateProfile->Data.IdleEnterSpeedThresholdKmh = 0.75f;

	// 5개 Profile의 첫 immutable Snapshot Set입니다.
	FCFVehicleProfileSnapshotSet FirstSnapshot;
	// Profile Snapshot Builder 실패 이유를 받을 문자열입니다.
	FString SnapshotError;
	const bool bFirstBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
		BaseProfile,
		DrivetrainProfile,
		HandlingProfile,
		PerformanceProfile,
		DriveStateProfile,
		FirstSnapshot,
		SnapshotError);
	TestTrue(TEXT("Five Profile Snapshot Set builds"), bFirstBuilt);
	TestTrue(TEXT("Base source present"), FirstSnapshot.BaseSource.IsPresent());
	TestTrue(TEXT("Drivetrain source present"), FirstSnapshot.DrivetrainSource.IsPresent());
	TestTrue(TEXT("Handling source present"), FirstSnapshot.HandlingSource.IsPresent());
	TestTrue(TEXT("Performance source present"), FirstSnapshot.PerformanceSource.IsPresent());
	TestTrue(TEXT("DriveState source present"), FirstSnapshot.DriveStateSource.IsPresent());
	TestEqual(TEXT("Base payload copied"), FirstSnapshot.BaseData.BaseVehicleMassKg, 1570.0f);
	TestEqual(TEXT("Base source revision copied"), FirstSnapshot.BaseSource.AuthoringRevision, 3);

	// 표시 metadata만 수정한 뒤 Profile payload fingerprint 불변성을 확인합니다.
	BaseProfile->Meta.DisplayName = FText::FromString(TEXT("표시 이름 B"));
	BaseProfile->Meta.Description = TEXT("바뀐 설명");
	BaseProfile->Meta.AuthoringRevision = 4;
	// Metadata-only 변경 뒤 두 번째 Snapshot Set입니다.
	FCFVehicleProfileSnapshotSet MetadataSnapshot;
	const bool bMetadataBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
		BaseProfile,
		DrivetrainProfile,
		HandlingProfile,
		PerformanceProfile,
		DriveStateProfile,
		MetadataSnapshot,
		SnapshotError);
	TestTrue(TEXT("Metadata Profile Snapshot Set builds"), bMetadataBuilt);
	TestEqual(TEXT("Display metadata does not change payload fingerprint"), MetadataSnapshot.BaseSource.ProfileFingerprint, FirstSnapshot.BaseSource.ProfileFingerprint);
	TestEqual(TEXT("Diagnostic revision still updates"), MetadataSnapshot.BaseSource.AuthoringRevision, 4);

	// Resolver가 실제 소비하는 typed data를 수정해 fingerprint 변화 여부를 확인합니다.
	BaseProfile->Data.BaseVehicleMassKg = 1600.0f;
	// Payload 변경 뒤 세 번째 Snapshot Set입니다.
	FCFVehicleProfileSnapshotSet PayloadSnapshot;
	const bool bPayloadBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
		BaseProfile,
		DrivetrainProfile,
		HandlingProfile,
		PerformanceProfile,
		DriveStateProfile,
		PayloadSnapshot,
		SnapshotError);
	TestTrue(TEXT("Payload Profile Snapshot Set builds"), bPayloadBuilt);
	TestTrue(TEXT("Typed payload change updates fingerprint"), PayloadSnapshot.BaseSource.ProfileFingerprint != FirstSnapshot.BaseSource.ProfileFingerprint);
	return true;
}

// Current Definition exact Stable-ID expansion과 current Registry-pattern Project Compatibility Default Snapshot/hash determinism을 검증합니다.
bool FCFVehicleDefinitionSnapshotTest::RunTest(const FString& Parameters)
{
	// Content Asset을 건드리지 않는 transient UCFVehicleData source입니다.
	UCFVehicleData* Definition = NewObject<UCFVehicleData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient VehicleData exists"), Definition))
	{
		return false;
	}

	// Stable selector 정렬을 검증하기 위해 의도적으로 역순으로 넣는 첫 Hardpoint입니다.
	FCFVehicleHardpointSlot& Top02 = Definition->HardpointSlots.AddDefaulted_GetRef();
	Top02.LocationSlotId = TEXT("Top_02");
	Top02.LocalLocation = FVector(20.0, 0.0, 0.0);
	// 두 번째 Hardpoint는 canonical path상 Top_02보다 먼저 정렬되어야 합니다.
	FCFVehicleHardpointSlot& Top01 = Definition->HardpointSlots.AddDefaulted_GetRef();
	Top01.LocationSlotId = TEXT("Top_01");
	Top01.LocalLocation = FVector(10.0, 0.0, 0.0);

	// 실제 Definition의 Registry-expanded exact Snapshot입니다.
	FCFVehicleDefinitionSnapshot FirstSnapshot;
	// Definition Snapshot Builder 실패 이유를 받을 문자열입니다.
	FString SnapshotError;
	const bool bFirstBuilt = FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Definition, FirstSnapshot, SnapshotError);
	TestTrue(TEXT("Definition Snapshot builds"), bFirstBuilt);
		// Current Registry pattern 중 Hardpoint 5 + Mount 16을 빼고 Hardpoint 2개 x 5 leaf를 실제 element로 확장한 개수입니다.
	const int32 ExpectedExactFieldCount = FCFVehicleFieldRegistry::ExpectedLeafPatternCount - 5 - 16 + (2 * 5);
	TestEqual(TEXT("Registry-expanded exact field count"), FirstSnapshot.SortedFields.Num(), ExpectedExactFieldCount);
	TestEqual(TEXT("Definition hash length"), FirstSnapshot.DefinitionHash.Len(), 32);

	// Exact Stable-ID path 존재 여부와 전체 canonical sorting을 확인합니다.
	bool bFoundTop01Location = false;
	for (int32 FieldIndex = 0; FieldIndex < FirstSnapshot.SortedFields.Num(); ++FieldIndex)
	{
		// 현재 sorted field의 canonical exact path입니다.
		const FString CurrentPath = FirstSnapshot.SortedFields[FieldIndex].FieldPath.ToCanonicalString(true);
		bFoundTop01Location |= CurrentPath == TEXT("HardpointSlots[LocationSlotId=Top_01].LocalLocation");
		if (FieldIndex > 0)
		{
			// 바로 앞 field의 canonical exact path입니다.
			const FString PreviousPath = FirstSnapshot.SortedFields[FieldIndex - 1].FieldPath.ToCanonicalString(true);
			TestTrue(TEXT("Definition fields are canonical sorted"), PreviousPath < CurrentPath);
		}
	}
	TestTrue(TEXT("Stable selector expands into exact field path"), bFoundTop01Location);

	// Array physical order만 바꿔도 Stable-ID field set/hash가 흔들리지 않아야 합니다.
	Definition->HardpointSlots.Swap(0, 1);
	// Reordered Definition Snapshot입니다.
	FCFVehicleDefinitionSnapshot ReorderedSnapshot;
	const bool bReorderedBuilt = FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Definition, ReorderedSnapshot, SnapshotError);
	TestTrue(TEXT("Reordered Definition Snapshot builds"), bReorderedBuilt);
	TestEqual(TEXT("Stable-ID reorder keeps Definition hash"), ReorderedSnapshot.DefinitionHash, FirstSnapshot.DefinitionHash);

	// Top_01의 실제 leaf value를 바꾸면 exact Definition hash가 달라져야 합니다.
	for (FCFVehicleHardpointSlot& Hardpoint : Definition->HardpointSlots)
	{
		if (Hardpoint.LocationSlotId == TEXT("Top_01"))
		{
			Hardpoint.LocalLocation.X += 1.0;
		}
	}
	// Value 변경 뒤 Definition Snapshot입니다.
	FCFVehicleDefinitionSnapshot ChangedSnapshot;
	const bool bChangedBuilt = FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Definition, ChangedSnapshot, SnapshotError);
	TestTrue(TEXT("Changed Definition Snapshot builds"), bChangedBuilt);
	TestTrue(TEXT("Exact field value change updates Definition hash"), ChangedSnapshot.DefinitionHash != FirstSnapshot.DefinitionHash);

	// Project Compatibility Default builder가 CDO array를 변경하지 않는지 비교할 원래 개수입니다.
	const UCFVehicleData* VehicleDataCDO = GetDefault<UCFVehicleData>();
	const int32 OriginalDefaultHardpointCount = VehicleDataCDO->HardpointSlots.Num();
	const int32 OriginalDefaultMountCount = VehicleDataCDO->MountProfiles.Num();
		// 첫 current Registry-pattern Project Compatibility Default Snapshot입니다.
	FCFVehicleDefinitionSnapshot FirstDefaultSnapshot;
	const bool bFirstDefaultBuilt = FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(FirstDefaultSnapshot, SnapshotError);
	TestTrue(TEXT("Project Compatibility Default Snapshot builds"), bFirstDefaultBuilt);
		TestEqual(TEXT("Project Default has exactly current Registry patterns"), FirstDefaultSnapshot.SortedFields.Num(), FCFVehicleFieldRegistry::ExpectedLeafPatternCount);
	TestEqual(TEXT("Project Default hash length"), FirstDefaultSnapshot.DefinitionHash.Len(), 32);
	TestEqual(TEXT("Project Default builder does not mutate CDO Hardpoints"), VehicleDataCDO->HardpointSlots.Num(), OriginalDefaultHardpointCount);
	TestEqual(TEXT("Project Default builder does not mutate CDO Mounts"), VehicleDataCDO->MountProfiles.Num(), OriginalDefaultMountCount);

		// 같은 C++ defaults에서 다시 만든 두 번째 current Registry-pattern Snapshot입니다.
	FCFVehicleDefinitionSnapshot SecondDefaultSnapshot;
	const bool bSecondDefaultBuilt = FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(SecondDefaultSnapshot, SnapshotError);
	TestTrue(TEXT("Repeated Project Default Snapshot builds"), bSecondDefaultBuilt);
	TestEqual(TEXT("Project Default hash is deterministic"), SecondDefaultSnapshot.DefinitionHash, FirstDefaultSnapshot.DefinitionHash);

	// Empty CDO collection의 element C++ defaults도 wildcard Registry field로 포함되었는지 확인합니다.
	const bool bHasWildcardHardpoint = FirstDefaultSnapshot.SortedFields.ContainsByPredicate([](const FCFVehicleFieldEntry& Entry)
	{
		return Entry.FieldPath.ToCanonicalString(true) == TEXT("HardpointSlots[LocationSlotId=*].LocalLocation");
	});
		TestTrue(TEXT("Project Default contains wildcard Hardpoint C++ default"), bHasWildcardHardpoint);
	return true;
}

// Reference Evidence semantic fingerprint가 array order/diagnostic 변화에는 안정적이고 canonical claim 변화에는 민감한지 검증합니다.
bool FCFVehicleRefEvidenceTest::RunTest(const FString& Parameters)
{
	// Content Asset을 만들지 않는 transient Reference Evidence source입니다.
	UCFVehicleRefEvidence* Evidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient Reference Evidence exists"), Evidence))
	{
		return false;
	}

	// 첫 primary Reference Vehicle identity입니다.
	FCFRefVehicleIdentity& PrimaryVehicle = Evidence->ReferenceVehicles.AddDefaulted_GetRef();
	PrimaryVehicle.ReferenceVehicleId = TEXT("REF_A");
	PrimaryVehicle.Role = ECFRefVehicleRole::Primary;
	PrimaryVehicle.Manufacturer = TEXT("Kia");
	PrimaryVehicle.Model = TEXT("Morning");
	PrimaryVehicle.ModelYearStart = 2027;
	PrimaryVehicle.ModelYearEnd = 2027;
	PrimaryVehicle.ModelYearQualifier = ECFRefModelYearQualifier::Exact;
	PrimaryVehicle.MarketRegion = TEXT("KR");
	PrimaryVehicle.IdentityConfidence = 0.95f;

	// 첫 official source citation입니다.
	FCFRefSourceCitation& SourceA = Evidence->Sources.AddDefaulted_GetRef();
	SourceA.SourceId = TEXT("SRC_A");
	SourceA.Tier = ECFRefSourceTier::TierA;
	SourceA.SourceKind = TEXT("ManufacturerSpecification");
	SourceA.Publisher = TEXT("Kia");
	SourceA.DocumentTitle = TEXT("The 2027 Morning Specification");
	SourceA.CanonicalUrl = TEXT("HTTPS://www.kia.com/kr/vehicles/morning/specification?utm_source=test#spec");
	SourceA.ReferenceVehicleIds = {TEXT("REF_A")};
	SourceA.OriginGroupId = TEXT("KIA_OFFICIAL");
	SourceA.OriginIndependence = ECFRefOriginIndependence::SameOrigin;
	SourceA.AccessedAtUtc = FDateTime(2026, 8, 26, 10, 0, 0);

	// 두 번째 corroboration source citation입니다.
	FCFRefSourceCitation& SourceB = Evidence->Sources.AddDefaulted_GetRef();
	SourceB.SourceId = TEXT("SRC_B");
	SourceB.Tier = ECFRefSourceTier::TierC;
	SourceB.SourceKind = TEXT("VehicleDatabase");
	SourceB.Publisher = TEXT("Carnoon");
	SourceB.CanonicalUrl = TEXT("https://www.carnoon.co.kr/newcar/vehicle/example");
	SourceB.ReferenceVehicleIds = {TEXT("REF_A")};
	SourceB.OriginIndependence = ECFRefOriginIndependence::OriginUnknown;

	// Canonical curb-mass FACT claim입니다.
	FCFRefClaim& MassClaim = Evidence->Claims.AddDefaulted_GetRef();
	MassClaim.ClaimId = TEXT("CLM_MASS");
	MassClaim.ReferenceVehicleId = TEXT("REF_A");
	MassClaim.FactKey = TEXT("Mass.Curb");
	MassClaim.ValueKind = ECFRefValueKind::Number;
	MassClaim.NumberValue = 975.0;
	MassClaim.UnitId = TEXT("kg");
	MassClaim.SourceValueText = TEXT("975 kg");
	MassClaim.Provenance = ECFRefProvenance::FACT;
	MassClaim.CitationIds = {TEXT("SRC_B"), TEXT("SRC_A")};
	MassClaim.ConfidenceScore = 0.95f;
	MassClaim.ResolutionState = ECFRefClaimResolution::Canonical;

	// Canonical wheelbase FACT claim입니다.
	FCFRefClaim& WheelbaseClaim = Evidence->Claims.AddDefaulted_GetRef();
	WheelbaseClaim.ClaimId = TEXT("CLM_WB");
	WheelbaseClaim.ReferenceVehicleId = TEXT("REF_A");
	WheelbaseClaim.FactKey = TEXT("Dimensions.Wheelbase");
	WheelbaseClaim.ValueKind = ECFRefValueKind::Number;
	WheelbaseClaim.NumberValue = 2400.0;
	WheelbaseClaim.UnitId = TEXT("mm");
	WheelbaseClaim.Provenance = ECFRefProvenance::FACT;
	WheelbaseClaim.CitationIds = {TEXT("SRC_A")};
	WheelbaseClaim.ConfidenceScore = 0.99f;
	WheelbaseClaim.ResolutionState = ECFRefClaimResolution::Canonical;

	// 첫 semantic fingerprint입니다.
	FString FirstFingerprint;
	// Evidence fingerprint build diagnostic입니다.
	FString FingerprintError;
	TestTrue(TEXT("First Evidence fingerprint builds"), Evidence->BuildEvidenceFingerprint(FirstFingerprint, FingerprintError));
	TestEqual(TEXT("Evidence SHA-256 hex length"), FirstFingerprint.Len(), 64);

	// Physical array order와 diagnostic-only 값을 변경합니다.
	Evidence->Sources.Swap(0, 1);
	Evidence->Claims.Swap(0, 1);
	Evidence->Sources[0].AccessedAtUtc = FDateTime(2030, 1, 1, 0, 0, 0);
	Evidence->Sources[0].SourceScopeNote = TEXT("fingerprint 제외 diagnostic");
	Evidence->ResearchNotes = TEXT("사람용 메모 변경");
	Evidence->AuthoringRevision = 99;
	// Reorder/diagnostic-only 변경 뒤 fingerprint입니다.
	FString ReorderedFingerprint;
	TestTrue(TEXT("Reordered Evidence fingerprint builds"), Evidence->BuildEvidenceFingerprint(ReorderedFingerprint, FingerprintError));
	TestEqual(TEXT("Array order and diagnostic state do not change Evidence fingerprint"), ReorderedFingerprint, FirstFingerprint);

	// Unicode NFC composed form으로 의미상 동일한 제조사 문자열을 설정합니다.
	Evidence->ReferenceVehicles[0].Manufacturer = TEXT("\u00E9");
	// NFC composed 문자열 상태의 Evidence fingerprint입니다.
	FString ComposedNfcFingerprint;
	TestTrue(TEXT("Composed NFC Evidence fingerprint builds"), Evidence->BuildEvidenceFingerprint(ComposedNfcFingerprint, FingerprintError));

	// 같은 문자를 e + combining acute accent 분해형으로 설정합니다.
	Evidence->ReferenceVehicles[0].Manufacturer = TEXT("e\u0301");
	// Unicode canonical-equivalent decomposed 문자열 상태의 Evidence fingerprint입니다.
	FString DecomposedNfcFingerprint;
	TestTrue(TEXT("Decomposed NFC Evidence fingerprint builds"), Evidence->BuildEvidenceFingerprint(DecomposedNfcFingerprint, FingerprintError));
	TestEqual(TEXT("Unicode NFC canonical equivalents keep Evidence fingerprint"), DecomposedNfcFingerprint, ComposedNfcFingerprint);

	// 뒤 semantic mutation 검증이 최초 baseline과 동일한 identity에서 시작하도록 원래 제조사를 복원합니다.
	Evidence->ReferenceVehicles[0].Manufacturer = TEXT("Kia");

	// ClaimId로 찾아 실제 semantic curb mass만 변경합니다.
	FCFRefClaim* ChangedMassClaim = Evidence->Claims.FindByPredicate([](const FCFRefClaim& Claim)
	{
		return Claim.ClaimId == TEXT("CLM_MASS");
	});
	if (!TestNotNull(TEXT("Mass claim remains addressable by stable ClaimId"), ChangedMassClaim))
	{
		return false;
	}
	ChangedMassClaim->NumberValue = 1010.0;
	// Semantic fact 변경 뒤 fingerprint입니다.
	FString ChangedFingerprint;
	TestTrue(TEXT("Changed Evidence fingerprint builds"), Evidence->BuildEvidenceFingerprint(ChangedFingerprint, FingerprintError));
	TestTrue(TEXT("Semantic claim change updates Evidence fingerprint"), ChangedFingerprint != FirstFingerprint);
	return true;
}

// Chassis socket facts와 Wheel local bounds가 UObject 없는 deterministic Asset Snapshot으로 고정되는지 검증합니다.
bool FCFVehicleAssetSnapshotTest::RunTest(const FString& Parameters)
{
	// Socket extraction source로 사용할 transient Chassis StaticMesh입니다.
	UStaticMesh* TestChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), TEXT("DAUTH_P0_08D_Chassis"));
	if (!TestNotNull(TEXT("Transient Chassis StaticMesh exists"), TestChassisMesh))
	{
		return false;
	}

	// Transient Chassis에 deterministic socket을 추가하고 pointer를 반환하는 helper입니다.
	auto AddTestSocket = [TestChassisMesh](const FName SocketName, const FVector& RelativeLocation, const FRotator& RelativeRotation) -> UStaticMeshSocket*
	{
		// Chassis가 소유하는 transient StaticMesh socket입니다.
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(TestChassisMesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = RelativeLocation;
		Socket->RelativeRotation = RelativeRotation;
		Socket->RelativeScale = FVector::OneVector;
		TestChassisMesh->AddSocket(Socket);
		return Socket;
	};

	// Fingerprint 변화 검증에서 수정할 FL socket입니다.
	UStaticMeshSocket* WheelSocketFL = AddTestSocket(TEXT("Wheel_Anchor_FL"), FVector(100.0, -50.0, 25.0), FRotator(0.0, 1.0, 0.0));
	AddTestSocket(TEXT("Wheel_Anchor_FR"), FVector(100.0, 50.0, 25.0), FRotator(0.0, -1.0, 0.0));
	AddTestSocket(TEXT("Wheel_Anchor_RL"), FVector(-100.0, -50.0, 25.0), FRotator::ZeroRotator);
	AddTestSocket(TEXT("Wheel_Anchor_RR"), FVector(-100.0, 50.0, 25.0), FRotator::ZeroRotator);
	AddTestSocket(TEXT("HP_Top_01"), FVector(0.0, 0.0, 80.0), FRotator(0.0, 90.0, 0.0));

	// 실제 non-zero local bounds를 제공하는 Engine built-in StaticMesh입니다.
	UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!TestNotNull(TEXT("Engine Cube Wheel test mesh exists"), WheelMesh))
	{
		return false;
	}

	// Asset Reader 입력으로 사용할 immutable Recipe Snapshot입니다.
	FCFVehicleRecipeSnapshot RecipeSnapshot;
	RecipeSnapshot.AssetIntent.ChassisMesh = TestChassisMesh;
	RecipeSnapshot.AssetIntent.WheelMeshFL = WheelMesh;
	RecipeSnapshot.AssetIntent.WheelMeshFR = WheelMesh;
	RecipeSnapshot.AssetIntent.WheelMeshRL = WheelMesh;
	RecipeSnapshot.AssetIntent.WheelMeshRR = WheelMesh;
	// Existing Layout Capture와 같은 default fallback을 검증하기 위해 FL binding만 None으로 둡니다.
	RecipeSnapshot.AssetIntent.BodyWheelSocketFL = NAME_None;

	// 실제 존재하는 선택 Hardpoint socket intent입니다.
	FCFHardpointIntent& FoundHardpoint = RecipeSnapshot.HardpointIntents.AddDefaulted_GetRef();
	FoundHardpoint.LocationSlotId = TEXT("Top_01");
	FoundHardpoint.SocketName = TEXT("HP_Top_01");
	// SocketName은 지정됐지만 Chassis에는 존재하지 않는 선택 Hardpoint intent입니다.
	FCFHardpointIntent& MissingHardpoint = RecipeSnapshot.HardpointIntents.AddDefaulted_GetRef();
	MissingHardpoint.LocationSlotId = TEXT("Top_02");
	MissingHardpoint.SocketName = TEXT("HP_Top_02_Missing");

	// 첫 immutable Asset Snapshot입니다.
	FCFVehicleAssetSnapshot FirstSnapshot;
	// Asset Reader 실패 사유를 받을 문자열입니다.
	FString AssetReadError;
	const bool bFirstBuilt = FCFVehicleAssetReader::BuildAssetSnapshot(RecipeSnapshot, FirstSnapshot, AssetReadError);
	TestTrue(TEXT("Asset Snapshot builds with optional missing Hardpoint socket"), bFirstBuilt);
	TestTrue(TEXT("Chassis asset loaded"), FirstSnapshot.bChassisLoaded);
	TestEqual(TEXT("Requested socket facts are deduped and complete"), FirstSnapshot.ChassisSockets.Num(), 6);
	TestEqual(TEXT("Chassis layout fingerprint length"), FirstSnapshot.ChassisLayoutFingerprint.Len(), 32);

	// None FL binding이 기존 CaptureLayout fallback인 Wheel_Anchor_FL fact로 해석됐는지 확인합니다.
	const FCFVehicleSocketSnapshot* FLSocketFact = FirstSnapshot.FindChassisSocket(TEXT("Wheel_Anchor_FL"));
	if (TestNotNull(TEXT("Default FL socket fact exists"), FLSocketFact))
	{
		TestTrue(TEXT("Default FL socket is found"), FLSocketFact->bFound);
		TestEqual(TEXT("Default FL socket location copied"), FLSocketFact->RelativeLocation, FVector(100.0, -50.0, 25.0));
	}

	// 선택 Hardpoint missing은 Snapshot build 실패가 아니라 explicit fact로 남아야 합니다.
	const FCFVehicleSocketSnapshot* MissingHardpointFact = FirstSnapshot.FindChassisSocket(TEXT("HP_Top_02_Missing"));
	if (TestNotNull(TEXT("Missing Hardpoint socket fact exists"), MissingHardpointFact))
	{
		TestFalse(TEXT("Missing Hardpoint socket remains not found"), MissingHardpointFact->bFound);
	}

	// Wheel local bounds와 fingerprint가 실제 StaticMesh local bounds를 반영하는지 확인합니다.
	const FBoxSphereBounds ExpectedWheelBounds = WheelMesh->GetBounds();
	TestTrue(TEXT("FL Wheel asset loaded"), FirstSnapshot.WheelFL.bAssetLoaded);
	TestEqual(TEXT("FL Wheel bounds origin copied"), FirstSnapshot.WheelFL.BoundsOrigin, ExpectedWheelBounds.Origin);
	TestEqual(TEXT("FL Wheel bounds extent copied"), FirstSnapshot.WheelFL.BoundsExtent, ExpectedWheelBounds.BoxExtent);
	TestEqual(TEXT("Wheel measurement fingerprint length"), FirstSnapshot.WheelFL.MeasureFingerprint.Len(), 32);
	TestEqual(TEXT("Same wheel asset/facts produce same slot fingerprint"), FirstSnapshot.WheelFR.MeasureFingerprint, FirstSnapshot.WheelFL.MeasureFingerprint);

	// 동일 source를 다시 읽은 두 번째 Snapshot입니다.
	FCFVehicleAssetSnapshot RepeatedSnapshot;
	const bool bRepeatedBuilt = FCFVehicleAssetReader::BuildAssetSnapshot(RecipeSnapshot, RepeatedSnapshot, AssetReadError);
	TestTrue(TEXT("Repeated Asset Snapshot builds"), bRepeatedBuilt);
	TestEqual(TEXT("Chassis fingerprint is deterministic"), RepeatedSnapshot.ChassisLayoutFingerprint, FirstSnapshot.ChassisLayoutFingerprint);
	TestEqual(TEXT("Wheel fingerprint is deterministic"), RepeatedSnapshot.WheelFL.MeasureFingerprint, FirstSnapshot.WheelFL.MeasureFingerprint);

	// Resolver-relevant Chassis socket transform만 변경합니다.
	WheelSocketFL->RelativeLocation.X += 1.0;
	// Socket transform 변경 뒤 Asset Snapshot입니다.
	FCFVehicleAssetSnapshot SocketChangedSnapshot;
	const bool bSocketChangedBuilt = FCFVehicleAssetReader::BuildAssetSnapshot(RecipeSnapshot, SocketChangedSnapshot, AssetReadError);
	TestTrue(TEXT("Socket-changed Asset Snapshot builds"), bSocketChangedBuilt);
	TestTrue(TEXT("Socket transform change updates Chassis fingerprint"), SocketChangedSnapshot.ChassisLayoutFingerprint != FirstSnapshot.ChassisLayoutFingerprint);
	TestEqual(TEXT("Chassis-only change does not update Wheel fingerprint"), SocketChangedSnapshot.WheelFL.MeasureFingerprint, FirstSnapshot.WheelFL.MeasureFingerprint);

	// Wheel binding name만 존재하지 않는 socket으로 바꿔도 reader는 mutation 없이 missing fact를 반환합니다.
	RecipeSnapshot.AssetIntent.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR_Missing");
	// Missing required Wheel binding을 가진 Asset Snapshot입니다.
	FCFVehicleAssetSnapshot MissingWheelSocketSnapshot;
	const bool bMissingWheelBuilt = FCFVehicleAssetReader::BuildAssetSnapshot(RecipeSnapshot, MissingWheelSocketSnapshot, AssetReadError);
	TestTrue(TEXT("Missing Wheel socket is represented for later Resolver validation"), bMissingWheelBuilt);
	const FCFVehicleSocketSnapshot* MissingWheelFact = MissingWheelSocketSnapshot.FindChassisSocket(TEXT("Wheel_Anchor_FR_Missing"));
	if (TestNotNull(TEXT("Missing Wheel socket fact exists"), MissingWheelFact))
	{
		TestFalse(TEXT("Missing Wheel socket fact is not found"), MissingWheelFact->bFound);
	}
	return true;
}

// VB-P0-06 Gameplay Setup이 optional defaults와 USER Socket authority를 지키고 Existing stored Hardpoint를 baseline-safe하게 보존하는지 검증합니다.
bool FCFVehicleBuilderGameplayGuideTest::RunTest(const FString& Parameters)
{
	// R0 guidance의 current Target으로 사용할 transient VehicleData입니다.
	UCFVehicleData* TargetVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	// R0 guidance의 authoritative semantic owner로 사용할 transient Recipe입니다.
	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage());
	// Builder-private VehicleBase source를 대표할 transient Profile입니다.
	UCFVehicleBaseProfile* BaseProfile = NewObject<UCFVehicleBaseProfile>(GetTransientPackage());
	// Builder-private Drivetrain source를 대표할 transient Profile입니다.
	UCFDrivetrainProfile* DrivetrainProfile = NewObject<UCFDrivetrainProfile>(GetTransientPackage());
	// Builder-private Handling source를 대표할 transient Profile입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(GetTransientPackage());
	// Builder-private Performance source를 대표할 transient Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = NewObject<UCFPerformanceProfile>(GetTransientPackage());
	// USER가 직접 Socket을 배치한 상황을 만들 transient Chassis StaticMesh입니다.
	UStaticMesh* ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage());
	if (!TestNotNull(TEXT("VB-P0-06 target exists"), TargetVehicleData)
		|| !TestNotNull(TEXT("VB-P0-06 recipe exists"), Recipe)
		|| !TestNotNull(TEXT("VB-P0-06 base profile exists"), BaseProfile)
		|| !TestNotNull(TEXT("VB-P0-06 drivetrain profile exists"), DrivetrainProfile)
		|| !TestNotNull(TEXT("VB-P0-06 handling profile exists"), HandlingProfile)
		|| !TestNotNull(TEXT("VB-P0-06 performance profile exists"), PerformanceProfile)
		|| !TestNotNull(TEXT("VB-P0-06 chassis exists"), ChassisMesh))
	{
		return false;
	}

	// Resolver/Validator가 사용할 실제 non-zero bounds의 Engine wheel mesh입니다.
	UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	// Resolver/Validator required reference를 만족할 existing Chaos wheel class입니다.
	UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
	if (!TestNotNull(TEXT("VB-P0-06 wheel mesh exists"), WheelMesh)
		|| !TestNotNull(TEXT("VB-P0-06 wheel class exists"), WheelClass))
	{
		return false;
	}

	// USER가 직접 배치한 fixture Socket을 Chassis에 추가하는 helper입니다.
	auto AddFixtureSocket = [ChassisMesh](const FName SocketName, const FVector& RelativeLocation)
	{
		// Chassis가 소유하는 transient StaticMesh Socket입니다.
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(ChassisMesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = RelativeLocation;
		Socket->RelativeRotation = FRotator::ZeroRotator;
		Socket->RelativeScale = FVector::OneVector;
		ChassisMesh->AddSocket(Socket);
	};
	AddFixtureSocket(TEXT("Wheel_Anchor_FL"), FVector(110.0, -62.0, 28.0));
	AddFixtureSocket(TEXT("Wheel_Anchor_FR"), FVector(110.0, 62.0, 28.0));
	AddFixtureSocket(TEXT("Wheel_Anchor_RL"), FVector(-108.0, -62.0, 28.0));
	AddFixtureSocket(TEXT("Wheel_Anchor_RR"), FVector(-108.0, 62.0, 28.0));
	AddFixtureSocket(TEXT("HP_Top_01"), FVector(0.0, 0.0, 80.0));

	TargetVehicleData->VehicleVisualConfig.ChassisMesh = ChassisMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshFL = WheelMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshFR = WheelMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshRL = WheelMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshRR = WheelMesh;
	TargetVehicleData->VehicleReferenceConfig.FrontWheelClass = WheelClass;
	TargetVehicleData->VehicleReferenceConfig.RearWheelClass = WheelClass;
	TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides = true;
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation = FVector(110.0, -62.0, 28.0);
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeLocation = FVector(110.0, 62.0, 28.0);
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeLocation = FVector(-108.0, -62.0, 28.0);
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeLocation = FVector(-108.0, 62.0, 28.0);
	TargetVehicleData->BaseVehicleMassKg = 1540.0f;
	TargetVehicleData->MaximumGrossMassKg = 2280.0f;

	// Existing Vehicle Completion baseline에 미리 존재하는 stored Top hardpoint입니다.
	FCFVehicleHardpointSlot& ExistingHardpoint = TargetVehicleData->HardpointSlots.AddDefaulted_GetRef();
	ExistingHardpoint.LocationSlotId = TEXT("Top_01");
	ExistingHardpoint.LocationCategory = TEXT("Top");
	ExistingHardpoint.SocketName = TEXT("HP_Top_01");
	ExistingHardpoint.LocalLocation = FVector(0.0, 0.0, 80.0);
	ExistingHardpoint.LocalRotation = FRotator::ZeroRotator;
	// Existing baseline에 존재하는 Top mount profile입니다.
	FCFVehicleMountProfile& ExistingMount = TargetVehicleData->MountProfiles.AddDefaulted_GetRef();
	ExistingMount.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	ExistingMount.LocationSlotRef = TEXT("Top_01");
	ExistingMount.MountType = ECFVehicleMountType::Turret;
	ExistingMount.SizeLimit = ECFVehicleWeaponSize::Large;
	ExistingMount.DefaultEquipmentPresetData = nullptr;

	Recipe->TargetVehicleData = TargetVehicleData;
	// Lossless Existing Definition import에 사용할 current target snapshot입니다.
	FCFVehicleDefinitionSnapshot ImportedDefinition;
	// Import/snapshot 실패 이유입니다.
	FString ImportError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, ImportedDefinition, ImportError))
	{
		AddError(ImportError);
		return false;
	}
	// Existing Import 결과 진단입니다.
	FCFVehicleImportResult ImportResult;
	if (!FCFVehicleImportService::ImportDefinitionSnapshot(ImportedDefinition, *Recipe, ImportResult, ImportError))
	{
		AddError(ImportError);
		return false;
	}

	Recipe->ProfileBindings.VehicleBaseProfile = BaseProfile;
	Recipe->ProfileBindings.DrivetrainProfile = DrivetrainProfile;
	Recipe->ProfileBindings.HandlingProfile = HandlingProfile;
	Recipe->ProfileBindings.PerformanceProfile = PerformanceProfile;
	Recipe->AssetIntent.ChassisMesh = ChassisMesh;
	Recipe->DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;
	Recipe->WheelVisualIntent.Mode = ECFWheelVisualIntentMode::UseProfilePolicy;
	Recipe->DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::UseProfile;
	Recipe->DefaultDataIntent.DefenseMode = ECFAssetIntentMode::UseProfile;
	Recipe->DefaultDataIntent.DestroyedFxMode = ECFAssetIntentMode::UseProfile;
	Recipe->MassIntent.BaseMassMode = ECFAuthoringInputMode::UseProfile;
	Recipe->MassIntent.GrossMassMode = ECFAuthoringInputMode::UseProfile;

	BaseProfile->Meta.OwnerRecipeId = Recipe->RecipeId;
	BaseProfile->Data.BaseVehicleMassKg = 1540.0f;
	BaseProfile->Data.MaximumGrossMassKg = 2280.0f;
	BaseProfile->Data.MaxHealth = TargetVehicleData->VehicleDurabilityConfig.MaxHealth;
	BaseProfile->Data.ChassisWidth = TargetVehicleData->VehicleMovementConfig.ChassisWidth;
	BaseProfile->Data.ChassisHeight = TargetVehicleData->VehicleMovementConfig.ChassisHeight;
	BaseProfile->Data.ExpectedWheelCount = 4;
	BaseProfile->Data.FrontWheelCountForSteering = 2;
	BaseProfile->Data.bAutoScaleWheelMeshToRadius = false;
	BaseProfile->Data.WheelMeshScaleClampMin = 0.25f;
	BaseProfile->Data.WheelMeshScaleClampMax = 4.0f;
	DrivetrainProfile->Meta.OwnerRecipeId = Recipe->RecipeId;
	HandlingProfile->Meta.OwnerRecipeId = Recipe->RecipeId;
	PerformanceProfile->Meta.OwnerRecipeId = Recipe->RecipeId;

	Recipe->HardpointIntents.Reset();
	// Builder semantic owner에 추가할 Top hardpoint intent입니다.
	FCFHardpointIntent& HardpointIntent = Recipe->HardpointIntents.AddDefaulted_GetRef();
	HardpointIntent.LocationSlotId = TEXT("Top_01");
	HardpointIntent.LocationCategory = TEXT("Top");
	HardpointIntent.SocketName = TEXT("HP_Top_01");

	Recipe->MountIntents.Reset();
	// Top_01을 참조하지만 기본 EquipmentPreset은 강제하지 않는 Mount intent입니다.
	FCFMountIntent& MountIntent = Recipe->MountIntents.AddDefaulted_GetRef();
	MountIntent.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	MountIntent.LocationSlotRef = TEXT("Top_01");
	MountIntent.MountType = ECFVehicleMountType::Turret;
	MountIntent.SizeLimit = ECFVehicleWeaponSize::Large;

	// NewVehicle guidance 요청입니다.
	FCFBuilderGameplayGuidanceRequest NewVehicleRequest;
	NewVehicleRequest.ReadRequest.Recipe = Recipe;
	NewVehicleRequest.ReadRequest.TargetVehicleData = TargetVehicleData;
	NewVehicleRequest.ReadRequest.CallerKind = ECFAuthoringCallerKind::Automation;
	NewVehicleRequest.Mode = ECFBuilderCompanionMode::NewVehicle;
	// 정상 수동 Socket을 가진 첫 guidance 결과입니다.
	FCFBuilderGameplayGuidanceResult ReadyResult;
	const bool bReadyRead = FCFVehicleAuthoringService::ReadBuilderGameplayGuidance(NewVehicleRequest, ReadyResult);
	if (!TestTrue(TEXT("VB-P0-06 ready guidance read succeeds"), bReadyRead))
	{
		AddError(ReadyResult.Operation.Message);
		return false;
	}

	// 영역별 결과를 enum identity로 찾는 helper입니다.
	auto FindItem = [](const FCFBuilderGameplayGuidanceResult& Result, const ECFBuilderGameplayArea Area) -> const FCFBuilderGameplayGuidanceItem*
	{
		return Result.Items.FindByPredicate([Area](const FCFBuilderGameplayGuidanceItem& Item)
		{
			return Item.Area == Area;
		});
	};

	TestEqual(TEXT("VB-P0-06 fixed gameplay area count"), ReadyResult.Items.Num(), 8);
	TestTrue(TEXT("VB-P0-06 ready step can complete"), ReadyResult.bCanCompleteGameplayStep);
	TestEqual(TEXT("VB-P0-06 ready review count"), ReadyResult.NeedsReviewCount, 0);
	TestEqual(TEXT("VB-P0-06 ready blocked count"), ReadyResult.BlockedCount, 0);
	TestFalse(TEXT("VB-P0-06 guidance does not mutate Recipe"), ReadyResult.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("VB-P0-06 guidance does not mutate Target"), ReadyResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("VB-P0-06 guidance does not mutate Profile"), ReadyResult.Operation.Mutation.bProfileChanged);
	TestFalse(TEXT("VB-P0-06 guidance does not save"), ReadyResult.Operation.Mutation.bSavePerformed);

	// Defense optional 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* DefenseItem = FindItem(ReadyResult, ECFBuilderGameplayArea::Defense);
	// Destroyed FX optional 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* DestroyedFxItem = FindItem(ReadyResult, ECFBuilderGameplayArea::DestroyedFx);
	// Hardpoint complete 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* HardpointItem = FindItem(ReadyResult, ECFBuilderGameplayArea::Hardpoints);
	// Mount complete 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* MountItem = FindItem(ReadyResult, ECFBuilderGameplayArea::MountProfiles);
	// DriveState complete 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* DriveStateItem = FindItem(ReadyResult, ECFBuilderGameplayArea::DriveState);
	// Fitting mass complete 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* FittingItem = FindItem(ReadyResult, ECFBuilderGameplayArea::FittingMass);
	if (TestNotNull(TEXT("VB-P0-06 defense item exists"), DefenseItem))
	{
		TestEqual(TEXT("VB-P0-06 missing Defense is optional"), DefenseItem->State, ECFBuilderGuidanceState::Optional);
	}
	if (TestNotNull(TEXT("VB-P0-06 destroyed fx item exists"), DestroyedFxItem))
	{
		TestEqual(TEXT("VB-P0-06 missing DestroyedFx is optional"), DestroyedFxItem->State, ECFBuilderGuidanceState::Optional);
	}
	if (TestNotNull(TEXT("VB-P0-06 hardpoint item exists"), HardpointItem))
	{
		TestEqual(TEXT("VB-P0-06 found USER hardpoint socket is complete"), HardpointItem->State, ECFBuilderGuidanceState::Complete);
	}
	if (TestNotNull(TEXT("VB-P0-06 mount item exists"), MountItem))
	{
		TestEqual(TEXT("VB-P0-06 mount without default equipment is complete"), MountItem->State, ECFBuilderGuidanceState::Complete);
	}
	if (TestNotNull(TEXT("VB-P0-06 drive state item exists"), DriveStateItem))
	{
		TestEqual(TEXT("VB-P0-06 ProjectDefault drive state is complete"), DriveStateItem->State, ECFBuilderGuidanceState::Complete);
	}
	if (TestNotNull(TEXT("VB-P0-06 fitting item exists"), FittingItem))
	{
		TestEqual(TEXT("VB-P0-06 complete fitting mass is complete"), FittingItem->State, ECFBuilderGuidanceState::Complete);
	}

	// USER가 아직 만들지 않은 missing hardpoint Socket 이름으로 semantic binding만 바꿉니다.
	Recipe->HardpointIntents[0].SocketName = TEXT("HP_Top_01_Missing");
	// Missing USER Socket을 읽는 NewVehicle guidance 결과입니다.
	FCFBuilderGameplayGuidanceResult MissingSocketResult;
	const bool bMissingSocketRead = FCFVehicleAuthoringService::ReadBuilderGameplayGuidance(NewVehicleRequest, MissingSocketResult);
	if (!TestTrue(TEXT("VB-P0-06 missing socket guidance read succeeds"), bMissingSocketRead))
	{
		AddError(MissingSocketResult.Operation.Message);
		return false;
	}
	// Missing USER Socket의 Hardpoint 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* MissingHardpointItem = FindItem(MissingSocketResult, ECFBuilderGameplayArea::Hardpoints);
	if (TestNotNull(TEXT("VB-P0-06 missing hardpoint item exists"), MissingHardpointItem))
	{
		TestEqual(TEXT("VB-P0-06 missing USER socket requires review"), MissingHardpointItem->State, ECFBuilderGuidanceState::NeedsReview);
	}
	TestFalse(TEXT("VB-P0-06 missing USER socket blocks step completion"), MissingSocketResult.bCanCompleteGameplayStep);
	TestTrue(TEXT("VB-P0-06 missing USER socket has manual guidance"), MissingSocketResult.SocketGuidance.ContainsByPredicate([](const FCFBuilderManualSocketGuidance& Guidance)
	{
		return Guidance.Area == ECFBuilderGameplayArea::Hardpoints
			&& Guidance.SocketName == FName(TEXT("HP_Top_01_Missing"))
			&& !Guidance.bFoundOnChassis
			&& Guidance.Instruction.Contains(TEXT("자동배치하지 않습니다"));
	}));

	// Existing baseline stored transform은 유지하고 Recipe Socket binding만 없는 상태를 만듭니다.
	Recipe->HardpointIntents[0].SocketName = NAME_None;
	// Existing Vehicle Completion mode로 같은 current truth를 다시 평가합니다.
	FCFBuilderGameplayGuidanceRequest ExistingRequest = NewVehicleRequest;
	ExistingRequest.Mode = ECFBuilderCompanionMode::CompleteExisting;
	// Existing stored transform baseline-preservation guidance 결과입니다.
	FCFBuilderGameplayGuidanceResult ExistingResult;
	const bool bExistingRead = FCFVehicleAuthoringService::ReadBuilderGameplayGuidance(ExistingRequest, ExistingResult);
	if (!TestTrue(TEXT("VB-P0-06 existing completion guidance read succeeds"), bExistingRead))
	{
		AddError(ExistingResult.Operation.Message);
		return false;
	}
	// Existing mode Hardpoint 상태 row입니다.
	const FCFBuilderGameplayGuidanceItem* ExistingHardpointItem = FindItem(ExistingResult, ECFBuilderGameplayArea::Hardpoints);
	if (TestNotNull(TEXT("VB-P0-06 existing hardpoint item exists"), ExistingHardpointItem))
	{
		TestEqual(TEXT("VB-P0-06 existing stored transform avoids forced socket migration"), ExistingHardpointItem->State, ECFBuilderGuidanceState::Complete);
	}
	TestTrue(TEXT("VB-P0-06 existing stored transform is explicitly accepted"), ExistingResult.SocketGuidance.ContainsByPredicate([](const FCFBuilderManualSocketGuidance& Guidance)
	{
		return Guidance.Area == ECFBuilderGameplayArea::Hardpoints
			&& Guidance.SemanticId == FName(TEXT("Top_01"))
			&& Guidance.bExistingStoredTransformAccepted;
	}));
	TestFalse(TEXT("VB-P0-06 existing guidance still does not mutate Target"), ExistingResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("VB-P0-06 existing guidance still does not save"), ExistingResult.Operation.Mutation.bSavePerformed);
	return true;
}

// VB-P0-07 Final Review가 existing Core evidence를 집계하고 explicit Apply/guarded Undo를 새 writer 없이 왕복하는지 검증합니다.
bool FCFVehicleBuilderFinalReviewTest::RunTest(const FString& Parameters)
{
	// Final Review의 current Target으로 사용할 transient VehicleData입니다.
	UCFVehicleData* TargetVehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("VB_P0_07_Target"));
	// Final Review의 authoritative semantic owner로 사용할 transient Recipe입니다.
	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), TEXT("VB_P0_07_Recipe"));
	// Builder-private VehicleBase source를 대표할 transient Profile입니다.
	UCFVehicleBaseProfile* BaseProfile = NewObject<UCFVehicleBaseProfile>(GetTransientPackage(), TEXT("VB_P0_07_Base"));
	// Builder-private Drivetrain source를 대표할 transient Profile입니다.
	UCFDrivetrainProfile* DrivetrainProfile = NewObject<UCFDrivetrainProfile>(GetTransientPackage(), TEXT("VB_P0_07_Drivetrain"));
	// Builder-private Handling source를 대표할 transient Profile입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(GetTransientPackage(), TEXT("VB_P0_07_Handling"));
	// Builder-private Performance source를 대표할 transient Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = NewObject<UCFPerformanceProfile>(GetTransientPackage(), TEXT("VB_P0_07_Performance"));
	// USER가 Wheel/Hardpoint Socket을 직접 배치한 상황을 만들 transient Chassis입니다.
	UStaticMesh* ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), TEXT("VB_P0_07_Chassis"));
	// Final Review provenance owner로 사용할 transient Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage(), TEXT("VB_P0_07_Evidence"));
	if (!TestNotNull(TEXT("VB-P0-07 target exists"), TargetVehicleData)
		|| !TestNotNull(TEXT("VB-P0-07 recipe exists"), Recipe)
		|| !TestNotNull(TEXT("VB-P0-07 base profile exists"), BaseProfile)
		|| !TestNotNull(TEXT("VB-P0-07 drivetrain profile exists"), DrivetrainProfile)
		|| !TestNotNull(TEXT("VB-P0-07 handling profile exists"), HandlingProfile)
		|| !TestNotNull(TEXT("VB-P0-07 performance profile exists"), PerformanceProfile)
		|| !TestNotNull(TEXT("VB-P0-07 chassis exists"), ChassisMesh)
		|| !TestNotNull(TEXT("VB-P0-07 evidence exists"), Evidence))
	{
		return false;
	}

	// ApplyService의 FScopedTransaction이 Target 변경을 Unreal Undo stack에 기록할 수 있게 하는 fixture flag입니다.
	TargetVehicleData->SetFlags(RF_Transactional);
	// ApplyService의 AppliedState 변경을 같은 Unreal transaction에 기록할 수 있게 하는 fixture flag입니다.
	Recipe->SetFlags(RF_Transactional);

	// Resolver/Validator가 사용할 실제 non-zero bounds의 Engine wheel mesh입니다.
	UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	// Resolver/Validator required reference를 만족할 existing Chaos wheel class입니다.
	UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
	if (!TestNotNull(TEXT("VB-P0-07 wheel mesh exists"), WheelMesh)
		|| !TestNotNull(TEXT("VB-P0-07 wheel class exists"), WheelClass))
	{
		return false;
	}

	// USER가 직접 배치한 fixture Socket을 Chassis에 추가하는 helper입니다.
	auto AddFixtureSocket = [ChassisMesh](const FName SocketName, const FVector& RelativeLocation)
	{
		// Chassis가 소유하는 transient StaticMesh Socket입니다.
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(ChassisMesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = RelativeLocation;
		Socket->RelativeRotation = FRotator::ZeroRotator;
		Socket->RelativeScale = FVector::OneVector;
		ChassisMesh->AddSocket(Socket);
	};
	AddFixtureSocket(TEXT("Wheel_Anchor_FL"), FVector(110.0, -62.0, 28.0));
	AddFixtureSocket(TEXT("Wheel_Anchor_FR"), FVector(110.0, 62.0, 28.0));
	AddFixtureSocket(TEXT("Wheel_Anchor_RL"), FVector(-108.0, -62.0, 28.0));
	AddFixtureSocket(TEXT("Wheel_Anchor_RR"), FVector(-108.0, 62.0, 28.0));
	AddFixtureSocket(TEXT("HP_Top_01"), FVector(0.0, 0.0, 80.0));

	TargetVehicleData->VehicleVisualConfig.ChassisMesh = ChassisMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshFL = WheelMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshFR = WheelMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshRL = WheelMesh;
	TargetVehicleData->VehicleVisualConfig.WheelMeshRR = WheelMesh;
	TargetVehicleData->VehicleReferenceConfig.FrontWheelClass = WheelClass;
	TargetVehicleData->VehicleReferenceConfig.RearWheelClass = WheelClass;
	TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides = true;
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");
	TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation = FVector(110.0, -62.0, 28.0);
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeLocation = FVector(110.0, 62.0, 28.0);
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeLocation = FVector(-108.0, -62.0, 28.0);
	TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeLocation = FVector(-108.0, 62.0, 28.0);
	TargetVehicleData->BaseVehicleMassKg = 1540.0f;
	TargetVehicleData->MaximumGrossMassKg = 2280.0f;

	// Existing Vehicle Completion baseline의 USER-authored Top hardpoint입니다.
	FCFVehicleHardpointSlot& ExistingHardpoint = TargetVehicleData->HardpointSlots.AddDefaulted_GetRef();
	ExistingHardpoint.LocationSlotId = TEXT("Top_01");
	ExistingHardpoint.LocationCategory = TEXT("Top");
	ExistingHardpoint.SocketName = TEXT("HP_Top_01");
	ExistingHardpoint.LocalLocation = FVector(0.0, 0.0, 80.0);
	ExistingHardpoint.LocalRotation = FRotator::ZeroRotator;
	// Existing baseline의 Top mount입니다.
	FCFVehicleMountProfile& ExistingMount = TargetVehicleData->MountProfiles.AddDefaulted_GetRef();
	ExistingMount.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	ExistingMount.LocationSlotRef = TEXT("Top_01");
	ExistingMount.MountType = ECFVehicleMountType::Turret;
	ExistingMount.SizeLimit = ECFVehicleWeaponSize::Large;
	ExistingMount.DefaultEquipmentPresetData = nullptr;

	Recipe->TargetVehicleData = TargetVehicleData;
	// Existing Definition import source snapshot입니다.
	FCFVehicleDefinitionSnapshot ImportedDefinition;
	// Import/snapshot diagnostic입니다.
	FString ImportError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, ImportedDefinition, ImportError))
	{
		AddError(ImportError);
		return false;
	}
	// Existing Definition import summary입니다.
	FCFVehicleImportResult ImportResult;
	if (!FCFVehicleImportService::ImportDefinitionSnapshot(ImportedDefinition, *Recipe, ImportResult, ImportError))
	{
		AddError(ImportError);
		return false;
	}

	Recipe->ProfileBindings.VehicleBaseProfile = BaseProfile;
	Recipe->ProfileBindings.DrivetrainProfile = DrivetrainProfile;
	Recipe->ProfileBindings.HandlingProfile = HandlingProfile;
	Recipe->ProfileBindings.PerformanceProfile = PerformanceProfile;
	Recipe->AssetIntent.ChassisMesh = ChassisMesh;
	Recipe->DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;
	Recipe->WheelVisualIntent.Mode = ECFWheelVisualIntentMode::UseProfilePolicy;
	Recipe->DefaultDataIntent.DefenseMode = ECFAssetIntentMode::UseProfile;
	Recipe->DefaultDataIntent.DestroyedFxMode = ECFAssetIntentMode::UseProfile;
	Recipe->MassIntent.BaseMassMode = ECFAuthoringInputMode::UseProfile;
	Recipe->MassIntent.GrossMassMode = ECFAuthoringInputMode::UseProfile;

	BaseProfile->Meta.OwnerRecipeId = Recipe->RecipeId;
	BaseProfile->Data.BaseVehicleMassKg = 1540.0f;
	BaseProfile->Data.MaximumGrossMassKg = 2280.0f;
	BaseProfile->Data.MaxHealth = TargetVehicleData->VehicleDurabilityConfig.MaxHealth;
	BaseProfile->Data.ChassisWidth = TargetVehicleData->VehicleMovementConfig.ChassisWidth;
	BaseProfile->Data.ChassisHeight = TargetVehicleData->VehicleMovementConfig.ChassisHeight;
	BaseProfile->Data.ExpectedWheelCount = 4;
	BaseProfile->Data.FrontWheelCountForSteering = 2;
	BaseProfile->Data.bAutoScaleWheelMeshToRadius = false;
	BaseProfile->Data.WheelMeshScaleClampMin = 0.25f;
	BaseProfile->Data.WheelMeshScaleClampMax = 4.0f;
	DrivetrainProfile->Meta.OwnerRecipeId = Recipe->RecipeId;
	HandlingProfile->Meta.OwnerRecipeId = Recipe->RecipeId;
	PerformanceProfile->Meta.OwnerRecipeId = Recipe->RecipeId;

	Recipe->HardpointIntents.Reset();
	// Existing USER Socket을 그대로 참조하는 managed Top hardpoint intent입니다.
	FCFHardpointIntent& HardpointIntent = Recipe->HardpointIntents.AddDefaulted_GetRef();
	HardpointIntent.LocationSlotId = TEXT("Top_01");
	HardpointIntent.LocationCategory = TEXT("Top");
	HardpointIntent.SocketName = TEXT("HP_Top_01");
	Recipe->MountIntents.Reset();
	// Existing Top_01을 참조하는 managed Mount intent입니다.
	FCFMountIntent& MountIntent = Recipe->MountIntents.AddDefaulted_GetRef();
	MountIntent.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	MountIntent.LocationSlotRef = TEXT("Top_01");
	MountIntent.MountType = ECFVehicleMountType::Turret;
	MountIntent.SizeLimit = ECFVehicleWeaponSize::Large;

	// Final Review Apply를 실제로 요구하게 만들 MaxHealth legacy pin만 release합니다.
	Recipe->ImportState.LegacyPinnedFields.RemoveAll([](const FCFVehicleFieldOverride& Override)
	{
		return Override.FieldPath.ToCanonicalString(true) == TEXT("VehicleDurabilityConfig.MaxHealth");
	});
	Recipe->DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::ExplicitValue;
	Recipe->DurabilityIntent.ExplicitMaxHealth = TargetVehicleData->VehicleDurabilityConfig.MaxHealth + 25.0f;

	Evidence->TargetRecipeId = Recipe->RecipeId;
	Evidence->TargetRecipePath = FSoftObjectPath(Recipe);
	Evidence->TargetDefinitionPath = FSoftObjectPath(TargetVehicleData);
	// Final Review에서 FACT provenance를 검증할 canonical consumed Claim입니다.
	FCFRefClaim& FactClaim = Evidence->Claims.AddDefaulted_GetRef();
	FactClaim.ClaimId = TEXT("Claim_Fact_Mass");
	FactClaim.FactKey = TEXT("CurbMassKg");
	FactClaim.ValueKind = ECFRefValueKind::Number;
	FactClaim.NumberValue = 1540.0;
	FactClaim.UnitId = TEXT("kg");
	FactClaim.Provenance = ECFRefProvenance::FACT;
	FactClaim.ResolutionState = ECFRefClaimResolution::Canonical;
	// Final Review에서 DERIVED provenance를 검증할 canonical consumed Claim입니다.
	FCFRefClaim& DerivedClaim = Evidence->Claims.AddDefaulted_GetRef();
	DerivedClaim.ClaimId = TEXT("Claim_Derived_Ratio");
	DerivedClaim.FactKey = TEXT("PowerToMassRatio");
	DerivedClaim.ValueKind = ECFRefValueKind::Number;
	DerivedClaim.NumberValue = 1.0;
	DerivedClaim.Provenance = ECFRefProvenance::DERIVED;
	DerivedClaim.InputClaimIds = {FactClaim.ClaimId};
	DerivedClaim.MethodId = TEXT("AutomationDerived");
	DerivedClaim.MethodRevision = 1;
	DerivedClaim.ResolutionState = ECFRefClaimResolution::Canonical;
	// Final Review에서 GAME_BIAS provenance를 검증할 canonical consumed Claim입니다.
	FCFRefClaim& BiasClaim = Evidence->Claims.AddDefaulted_GetRef();
	BiasClaim.ClaimId = TEXT("Claim_Bias_Health");
	BiasClaim.FactKey = TEXT("GameplayDurabilityBias");
	BiasClaim.ValueKind = ECFRefValueKind::Number;
	BiasClaim.NumberValue = 25.0;
	BiasClaim.Provenance = ECFRefProvenance::GAME_BIAS;
	BiasClaim.InputClaimIds = {FactClaim.ClaimId};
	BiasClaim.MethodId = TEXT("AutomationBias");
	BiasClaim.MethodRevision = 1;
	BiasClaim.ResolutionState = ECFRefClaimResolution::Canonical;
	// Current semantic Evidence fingerprint 생성 diagnostic입니다.
	FString EvidenceError;
	if (!TestTrue(TEXT("VB-P0-07 Evidence fingerprint refresh succeeds"), Evidence->RefreshEvidenceFingerprint(EvidenceError)))
	{
		AddError(EvidenceError);
		return false;
	}

	// Final Review provenance authority를 실제 Builder Profile commit lane으로 생성할 receipt request입니다.
	FCFBuilderProfileCommitRequest ReceiptRequest;
	ReceiptRequest.Recipe = Recipe;
	ReceiptRequest.ExpectedOwnerRecipeId = Recipe->RecipeId;
	ReceiptRequest.Payload.VehicleBaseProfilePath = FSoftObjectPath(BaseProfile);
	ReceiptRequest.Payload.DrivetrainProfilePath = FSoftObjectPath(DrivetrainProfile);
	ReceiptRequest.Payload.HandlingProfilePath = FSoftObjectPath(HandlingProfile);
	ReceiptRequest.Payload.PerformanceProfilePath = FSoftObjectPath(PerformanceProfile);
	ReceiptRequest.Payload.VehicleBaseData = BaseProfile->Data;
	ReceiptRequest.Payload.DrivetrainData = DrivetrainProfile->Data;
	ReceiptRequest.Payload.HandlingData = HandlingProfile->Data;
	ReceiptRequest.Payload.PerformanceData = PerformanceProfile->Data;
	ReceiptRequest.EvidenceBinding.EvidencePath = FSoftObjectPath(Evidence);
	ReceiptRequest.EvidenceBinding.ExpectedEvidenceId = Evidence->EvidenceId;
	ReceiptRequest.EvidenceBinding.ExpectedEvidenceFingerprint = Evidence->EvidenceFingerprint;
	ReceiptRequest.EvidenceBinding.ConsumedClaimIds = {FactClaim.ClaimId, DerivedClaim.ClaimId, BiasClaim.ClaimId};
	ReceiptRequest.UpstreamBuilderProposalHash = TEXT("VB-P0-07-Automation-Upstream");
	ReceiptRequest.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;

	// Same-payload receipt-only commit을 승인할 fresh Builder Profile preview입니다.
	FCFBuilderProfileCommitPreview ReceiptPreview;
	if (!TestTrue(TEXT("VB-P0-07 receipt-only Builder Profile preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderProfiles(ReceiptRequest, ReceiptPreview)))
	{
		AddError(ReceiptPreview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("VB-P0-07 missing receipt requires R1 preview even when payload matches"), ReceiptPreview.Operation.Status, ECFAuthoringOpStatus::Succeeded);
	ReceiptRequest.ExpectedCurrentFingerprints = ReceiptPreview.CurrentFingerprints;
	ReceiptRequest.CallContext.ClientOperationId = TEXT("VB-P0-07-Receipt-Automation");
	ReceiptRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	ReceiptRequest.CallContext.ApprovalScopeHash = ReceiptPreview.Proposal.ProposalHash;
	ReceiptRequest.CallContext.ExpectedRecipeFingerprint = ReceiptPreview.Proposal.ExpectedRecipeFingerprint;
	ReceiptRequest.CallContext.ExpectedTargetDefinitionHash = ReceiptPreview.Proposal.ExpectedTargetDefinitionHash;
	ReceiptRequest.CallContext.ExpectedResolverContractRevision = ReceiptPreview.Proposal.ResolverContractRevision;
	// Receipt-only commit terminal result입니다.
	FCFAuthoringOpResult ReceiptCommitResult;
	if (!TestTrue(TEXT("VB-P0-07 receipt-only Builder Profile commit succeeds"), FCFVehicleAuthoringService::CommitBuilderProfiles(ReceiptRequest, ReceiptPreview, ReceiptCommitResult)))
	{
		AddError(ReceiptCommitResult.Message);
		return false;
	}
	TestTrue(TEXT("VB-P0-07 persistent Builder receipt is valid"), Recipe->BuilderCommitReceipt.IsValid());
	TestFalse(TEXT("VB-P0-07 receipt-only commit does not mutate Profile payload"), ReceiptCommitResult.Mutation.bProfileChanged);
	TestTrue(TEXT("VB-P0-07 receipt-only commit records Recipe provenance metadata"), ReceiptCommitResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("VB-P0-07 receipt-only commit does not save"), ReceiptCommitResult.Mutation.bSavePerformed);

	// Existing Vehicle Completion Final Review request입니다.
	FCFBuilderFinalReviewRequest ReviewRequest;
	ReviewRequest.GameplayRequest.ReadRequest.Recipe = Recipe;
	ReviewRequest.GameplayRequest.ReadRequest.TargetVehicleData = TargetVehicleData;
	ReviewRequest.GameplayRequest.ReadRequest.CallerKind = ECFAuthoringCallerKind::Automation;
	ReviewRequest.GameplayRequest.Mode = ECFBuilderCompanionMode::CompleteExisting;
	ReviewRequest.bHasEvidenceBinding = true;
	ReviewRequest.EvidenceBinding.EvidencePath = FSoftObjectPath(Evidence);
	ReviewRequest.EvidenceBinding.ExpectedEvidenceId = Evidence->EvidenceId;
	ReviewRequest.EvidenceBinding.ExpectedEvidenceFingerprint = Evidence->EvidenceFingerprint;
	ReviewRequest.EvidenceBinding.ConsumedClaimIds = {FactClaim.ClaimId, DerivedClaim.ClaimId, BiasClaim.ClaimId};

	// Initial mutation0 Final Review입니다.
	FCFBuilderFinalReviewResult ReviewResult;
	if (!TestTrue(TEXT("VB-P0-07 Final Review read succeeds"), FCFVehicleAuthoringService::ReadBuilderFinalReview(ReviewRequest, ReviewResult)))
	{
		AddError(ReviewResult.Operation.Message);
		return false;
	}
	TestTrue(TEXT("VB-P0-07 provenance is available"), ReviewResult.Provenance.bAvailable);
	TestEqual(TEXT("VB-P0-07 consumed claim count"), ReviewResult.Provenance.ConsumedClaimCount, 3);
	TestEqual(TEXT("VB-P0-07 FACT claim count"), ReviewResult.Provenance.FactClaimCount, 1);
	TestEqual(TEXT("VB-P0-07 DERIVED claim count"), ReviewResult.Provenance.DerivedClaimCount, 1);
	TestEqual(TEXT("VB-P0-07 GAME_BIAS claim count"), ReviewResult.Provenance.GameBiasClaimCount, 1);
	TestTrue(TEXT("VB-P0-07 target diff exists"), ReviewResult.FieldDiff.Num() > 0);
	TestTrue(TEXT("VB-P0-07 Apply is required"), ReviewResult.bApplyRequired);
	TestEqual(TEXT("VB-P0-07 blocker count is zero"), ReviewResult.BlockingIssueCount, 0);
	TestTrue(TEXT("VB-P0-07 can build explicit Apply"), ReviewResult.bCanApply);
	TestFalse(TEXT("VB-P0-07 cannot complete before Apply"), ReviewResult.bCanCompleteFinalReview);
	TestFalse(TEXT("VB-P0-07 Apply proposal hash is populated"), ReviewResult.ApplyProposal.ProposalHash.IsEmpty());
	TestFalse(TEXT("VB-P0-07 review does not mutate Target"), ReviewResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("VB-P0-07 review does not save"), ReviewResult.Operation.Mutation.bSavePerformed);

	// Evidence binding 없이 provenance를 추측하지 않는 blocker 검증 request입니다.
	FCFBuilderFinalReviewRequest MissingEvidenceRequest = ReviewRequest;
	MissingEvidenceRequest.bHasEvidenceBinding = false;
	MissingEvidenceRequest.EvidenceBinding = FCFBuilderEvidenceBinding();
	// Missing provenance Final Review 결과입니다.
	FCFBuilderFinalReviewResult MissingEvidenceResult;
	TestTrue(TEXT("VB-P0-07 missing Evidence review still returns diagnostic"), FCFVehicleAuthoringService::ReadBuilderFinalReview(MissingEvidenceRequest, MissingEvidenceResult));
	TestFalse(TEXT("VB-P0-07 missing Evidence cannot Apply"), MissingEvidenceResult.bCanApply);
	TestTrue(TEXT("VB-P0-07 missing Evidence adds blocker"), MissingEvidenceResult.BlockingIssueCount > 0);
	TestFalse(TEXT("VB-P0-07 missing Evidence does not invent provenance"), MissingEvidenceResult.Provenance.bAvailable);

	// 같은 Evidence의 다른 canonical Claim subset을 caller가 바꿔치기한 request입니다.
	FCFBuilderFinalReviewRequest TamperedClaimRequest = ReviewRequest;
	TamperedClaimRequest.EvidenceBinding.ConsumedClaimIds = {FactClaim.ClaimId};
	// Persistent receipt와 Claim set이 다른 Final Review 결과입니다.
	FCFBuilderFinalReviewResult TamperedClaimResult;
	TestTrue(TEXT("VB-P0-07 tampered canonical Claim subset returns diagnostic"), FCFVehicleAuthoringService::ReadBuilderFinalReview(TamperedClaimRequest, TamperedClaimResult));
	TestFalse(TEXT("VB-P0-07 tampered canonical Claim subset cannot reuse provenance"), TamperedClaimResult.Provenance.bAvailable);
	TestTrue(TEXT("VB-P0-07 tampered canonical Claim subset adds blocker"), TamperedClaimResult.BlockingIssueCount > 0);
	TestFalse(TEXT("VB-P0-07 tampered canonical Claim subset cannot Apply"), TamperedClaimResult.bCanApply);

	// USER가 Final Review exact proposal을 명시 승인한 Apply request입니다.
	FCFBuilderFinalApplyRequest ApplyRequest;
	ApplyRequest.ReviewRequest = ReviewRequest;
	ApplyRequest.CallContext.ClientOperationId = TEXT("VB-P0-07-Apply-Automation");
	ApplyRequest.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	ApplyRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	ApplyRequest.CallContext.ApprovalScopeHash = ReviewResult.ApplyProposal.ProposalHash;
	// Explicit Apply terminal result입니다.
	FCFBuilderFinalApplyResult ApplyResult;
	if (!TestTrue(TEXT("VB-P0-07 explicit Apply succeeds"), FCFVehicleAuthoringService::ApplyBuilderFinalReview(ApplyRequest, ApplyResult)))
	{
		AddError(ApplyResult.Operation.Message);
		return false;
	}
	TestEqual(TEXT("VB-P0-07 Apply status succeeded"), ApplyResult.Operation.Status, ECFAuthoringOpStatus::Succeeded);
	TestTrue(TEXT("VB-P0-07 Apply mutates Target through shared lane"), ApplyResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("VB-P0-07 Apply does not save"), ApplyResult.Operation.Mutation.bSavePerformed);
	TestTrue(TEXT("VB-P0-07 guarded Undo token is available"), ApplyResult.bUndoAvailable);
	TestTrue(TEXT("VB-P0-07 Undo transaction id is valid"), ApplyResult.UndoToken.TransactionId.IsValid());
	TestFalse(TEXT("VB-P0-07 Undo scope is populated"), ApplyResult.UndoToken.UndoScopeHash.IsEmpty());

	// Apply 뒤 diff0 Final Review입니다.
	FCFBuilderFinalReviewResult AppliedReview;
	if (!TestTrue(TEXT("VB-P0-07 post-Apply Final Review succeeds"), FCFVehicleAuthoringService::ReadBuilderFinalReview(ReviewRequest, AppliedReview)))
	{
		AddError(AppliedReview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("VB-P0-07 post-Apply target diff is zero"), AppliedReview.FieldDiff.Num(), 0);
	TestFalse(TEXT("VB-P0-07 post-Apply no Apply required"), AppliedReview.bApplyRequired);
	TestTrue(TEXT("VB-P0-07 post-Apply Final Review can complete"), AppliedReview.bCanCompleteFinalReview);

	// Builder가 방금 만든 exact transaction에 대한 explicit guarded Undo request입니다.
	FCFBuilderUndoRequest UndoRequest;
	UndoRequest.UndoToken = ApplyResult.UndoToken;
	UndoRequest.CallContext.ClientOperationId = TEXT("VB-P0-07-Undo-Automation");
	UndoRequest.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	UndoRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	UndoRequest.CallContext.ApprovalScopeHash = ApplyResult.UndoToken.UndoScopeHash;

	// Undo stack을 거치지 않은 raw Target drift를 흉내 낼 post-Apply 값입니다.
	const float AppliedMaxHealthBeforeRawDrift = TargetVehicleData->VehicleDurabilityConfig.MaxHealth;
	TargetVehicleData->VehicleDurabilityConfig.MaxHealth = AppliedMaxHealthBeforeRawDrift + 1.0f;
	// Raw drift 상태에서 guarded Undo가 반드시 거부되는 결과입니다.
	FCFAuthoringOpResult RawDriftUndoResult;
	TestFalse(TEXT("VB-P0-07 guarded Undo refuses nontransactional Target drift"), FCFVehicleAuthoringService::UndoBuilderFinalApply(UndoRequest, RawDriftUndoResult));
	TestEqual(TEXT("VB-P0-07 raw drift refusal is StateChanged"), RawDriftUndoResult.ErrorCode, ECFAuthoringErrorCode::StateChanged);
	TargetVehicleData->VehicleDurabilityConfig.MaxHealth = AppliedMaxHealthBeforeRawDrift;

	// Builder Apply 뒤에 끼워 다른 top transaction을 잘못 Undo하지 않는지 검증할 unrelated transactional Recipe입니다.
	UCFVehicleRecipeData* InterveningRecipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), TEXT("VB_P0_07_Intervening"), RF_Transient | RF_Transactional);
	if (!TestNotNull(TEXT("VB-P0-07 intervening transaction fixture exists"), InterveningRecipe))
	{
		return false;
	}
	// Intervening transaction 전 revision입니다.
	const int32 InterveningRevisionBefore = InterveningRecipe->AuthoringRevision;
	{
		// Builder 소유가 아닌 unrelated Editor transaction입니다.
		FScopedTransaction InterveningTransaction(NSLOCTEXT("CarFightDataAuthoringTests", "VBP007InterveningTransaction", "VB-P0-07 Intervening Transaction"));
		InterveningRecipe->Modify();
		++InterveningRecipe->AuthoringRevision;
	}
	// Intervening transaction 후 revision입니다.
	const int32 InterveningRevisionAfter = InterveningRecipe->AuthoringRevision;
	TestNotEqual(TEXT("VB-P0-07 intervening transaction changes its own object"), InterveningRevisionAfter, InterveningRevisionBefore);
	// Wrong-top guarded Undo가 반드시 거부되는 결과입니다.
	FCFAuthoringOpResult WrongTopUndoResult;
	TestFalse(TEXT("VB-P0-07 guarded Undo refuses unrelated top transaction"), FCFVehicleAuthoringService::UndoBuilderFinalApply(UndoRequest, WrongTopUndoResult));
	TestEqual(TEXT("VB-P0-07 wrong-top refusal is StateChanged"), WrongTopUndoResult.ErrorCode, ECFAuthoringErrorCode::StateChanged);
	TestEqual(TEXT("VB-P0-07 wrong-top refusal preserves intervening object"), InterveningRecipe->AuthoringRevision, InterveningRevisionAfter);
	TestEqual(TEXT("VB-P0-07 wrong-top refusal preserves applied Target"), TargetVehicleData->VehicleDurabilityConfig.MaxHealth, Recipe->DurabilityIntent.ExplicitMaxHealth);

	if (!TestNotNull(TEXT("VB-P0-07 Editor exists for transaction recovery"), GEditor) || !GEditor->Trans)
	{
		return false;
	}
	// 테스트가 unrelated top transaction만 표준 Undo해 Builder Apply를 다시 stack top으로 복원합니다.
	const bool bInterveningUndoSucceeded = GEditor->UndoTransaction();
	TestTrue(TEXT("VB-P0-07 standard Undo removes only intervening transaction"), bInterveningUndoSucceeded);
	TestEqual(TEXT("VB-P0-07 intervening transaction is restored"), InterveningRecipe->AuthoringRevision, InterveningRevisionBefore);

	// Exact Builder transaction만 되돌리는 Guarded Undo terminal result입니다.
	FCFAuthoringOpResult UndoResult;
	if (!TestTrue(TEXT("VB-P0-07 guarded Undo succeeds"), FCFVehicleAuthoringService::UndoBuilderFinalApply(UndoRequest, UndoResult)))
	{
		AddError(UndoResult.Message);
		return false;
	}
	TestFalse(TEXT("VB-P0-07 Undo does not save"), UndoResult.Mutation.bSavePerformed);

	// Undo 뒤 pre-Apply diff가 다시 나타나는 fresh Final Review입니다.
	FCFBuilderFinalReviewResult RestoredReview;
	if (!TestTrue(TEXT("VB-P0-07 post-Undo Final Review succeeds"), FCFVehicleAuthoringService::ReadBuilderFinalReview(ReviewRequest, RestoredReview)))
	{
		AddError(RestoredReview.Operation.Message);
		return false;
	}
	TestTrue(TEXT("VB-P0-07 post-Undo diff is restored"), RestoredReview.FieldDiff.Num() > 0);
	TestTrue(TEXT("VB-P0-07 post-Undo Apply is available again"), RestoredReview.bCanApply);
	TestFalse(TEXT("VB-P0-07 post-Undo review does not save"), RestoredReview.Operation.Mutation.bSavePerformed);
	return true;
}

#endif
