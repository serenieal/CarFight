// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthTests.cpp
// Version: v1.5.0
// Date: 2026-08-18
// Description: DAUTH-P0-08 Foundation의 current schema/Never-Cook/Stable Field/Registry/Codec/Immutable Snapshot/Asset Reader Automation입니다.
// Changelog:
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
#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"
#include "CFVehicleData.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringEditorOnlyTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Foundation.EditorOnlyAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringRegistryTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Foundation.Registry117",
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

// Current Registry 118개와 Current UCFVehicleData Reflection leaf 118개의 양방향 coverage 및 Redline direct authoring descriptor를 검증합니다.
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

#endif
