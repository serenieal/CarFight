// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleImportTests.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08G Existing Definition Import / Legacy Pin / Adoption Foundation Automation입니다.
// Changelog:
// - v1.0.0: lossless import, hidden Mount serialized partition, Legacy Pin precedence, group/field preview, stale guard와 Recipe-only commit 검증 추가.
// Migration:
// - 테스트는 메모리 UPackage와 transient UCFVehicleData만 사용하며 Content Asset을 생성/저장하지 않습니다.
// - Movement raw 값에서 Driving Feel/Profile을 역산하는 경로를 사용하지 않습니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleImportTestsPrivate
{
	// Test source Definition과 Recipe semantic candidate에서 공유할 Engine read-only Cube mesh를 가져옵니다.
	UStaticMesh* LoadTestCube()
	{
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	// Test-only UCFVehicleData에 Existing Definition import가 보존해야 할 대표 scalar/stable-array/legacy values를 구성합니다.
	bool ConfigureSourceVehicle(UCFVehicleData& SourceVehicle, FString& OutError)
	{
		// Read-only Engine Cube mesh입니다.
		UStaticMesh* CubeMesh = LoadTestCube();
		if (!CubeMesh)
		{
			OutError = TEXT("Engine BasicShapes Cube를 로드할 수 없습니다.");
			return false;
		}

		SourceVehicle.VehicleVisualConfig.ChassisMesh = CubeMesh;
		SourceVehicle.VehicleVisualConfig.WheelMeshFL = CubeMesh;
		SourceVehicle.VehicleVisualConfig.WheelMeshFR = CubeMesh;
		SourceVehicle.VehicleVisualConfig.WheelMeshRL = CubeMesh;
		SourceVehicle.VehicleVisualConfig.WheelMeshRR = CubeMesh;

		SourceVehicle.VehicleLayoutConfig.BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");
		SourceVehicle.VehicleLayoutConfig.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");
		SourceVehicle.VehicleLayoutConfig.BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");
		SourceVehicle.VehicleLayoutConfig.BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
		SourceVehicle.BaseVehicleMassKg = 1635.0f;
		SourceVehicle.MaximumGrossMassKg = 2480.0f;
		SourceVehicle.VehicleDurabilityConfig.MaxHealth = 315.0f;
		SourceVehicle.DriveStateConfig.bUseDriveStateOverrides = true;
		SourceVehicle.DestroyedFxSocketName = TEXT("FX_Imported_Destroyed");

		SourceVehicle.HardpointSlots.Reset();
		// Existing Definition의 stable Hardpoint fixture입니다.
		FCFVehicleHardpointSlot& Hardpoint = SourceVehicle.HardpointSlots.AddDefaulted_GetRef();
		Hardpoint.LocationSlotId = TEXT("Top_02");
		Hardpoint.LocationCategory = TEXT("Top");
		Hardpoint.SocketName = TEXT("HP_Top_02");
		Hardpoint.LocalLocation = FVector(15.0, 0.0, 82.0);
		Hardpoint.LocalRotation = FRotator(0.0, 12.0, 0.0);

		SourceVehicle.MountProfiles.Reset();
		// Existing Definition의 active + hidden serialized Mount fixture입니다.
		FCFVehicleMountProfile& Mount = SourceVehicle.MountProfiles.AddDefaulted_GetRef();
		Mount.MountProfileId = TEXT("M_Z");
		Mount.LocationSlotRef = TEXT("Top_02");
		Mount.MountType = ECFVehicleMountType::Turret;
		Mount.SizeLimit = ECFVehicleWeaponSize::Medium;
		Mount.DefaultEquipmentPresetData = nullptr;
		Mount.bExposedModule = false;
		Mount.YawTurnRateDegPerSec = 77.0f;
		Mount.PitchTurnRateDegPerSec = 51.0f;
		Mount.StabilizationToleranceDeg = 1.75f;
		Mount.AimSettleTimeSeconds = 0.42f;
		Mount.MountWeightKg = 444.0f;

		OutError.Reset();
		return true;
	}

	// Test source UCFVehicleData를 Frozen Registry exact Definition Snapshot으로 캡처합니다.
	bool BuildSourceDefinition(
		UCFVehicleData& SourceVehicle,
		FCFVehicleDefinitionSnapshot& OutDefinition,
		FString& OutError)
	{
		if (!ConfigureSourceVehicle(SourceVehicle, OutError))
		{
			return false;
		}
		return FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(SourceVehicle, OutDefinition, OutError);
	}

	// Recipe-only transaction dirty 상태를 격리할 in-memory test package를 만듭니다.
	UPackage* CreateRecipeTestPackage()
	{
		// 다른 Automation run과 충돌하지 않을 unique package 이름입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/CFDAImport_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// Existing Definition import용 Recipe를 in-memory package에 생성합니다.
	UCFVehicleRecipeData* CreateRecipe(UPackage*& OutPackage)
	{
		OutPackage = CreateRecipeTestPackage();
		if (!OutPackage)
		{
			return nullptr;
		}

		// Package에만 존재하고 저장하지 않는 test Recipe입니다.
		UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(OutPackage, TEXT("DA_ImportRecipe_Test"), RF_Transactional);
		if (Recipe)
		{
			Recipe->DrivingFeelIntent.AccelerationFeel = 0.17f;
			Recipe->DrivingFeelIntent.SteeringAgility = 0.29f;
			Recipe->DrivingFeelIntent.GripFeel = 0.63f;
			Recipe->DrivingFeelIntent.SuspensionFirmness = 0.81f;
		}
		return Recipe;
	}

	// Resolver value-only Asset Snapshot에 deterministic chassis/wheel/socket facts를 구성합니다.
	bool BuildAssetSnapshot(FCFVehicleAssetSnapshot& OutAssets, FString& OutError)
	{
		OutAssets = FCFVehicleAssetSnapshot();
		// Read-only Engine Cube mesh입니다.
		UStaticMesh* CubeMesh = LoadTestCube();
		if (!CubeMesh)
		{
			OutError = TEXT("Asset Snapshot fixture용 Cube를 로드할 수 없습니다.");
			return false;
		}

		// Cube object path를 Resolver value snapshot에만 보존합니다.
		const FSoftObjectPath CubePath(CubeMesh);
		OutAssets.ChassisObjectPath = CubePath;
		OutAssets.bChassisLoaded = true;
		OutAssets.ChassisLayoutFingerprint = TEXT("import-chassis-v1");

		auto AddSocket = [&OutAssets](const FName SocketName, const FVector& Location)
		{
			// Resolver가 소비할 immutable-style socket fact입니다.
			FCFVehicleSocketSnapshot& Socket = OutAssets.ChassisSockets.AddDefaulted_GetRef();
			Socket.SocketName = SocketName;
			Socket.bFound = true;
			Socket.RelativeLocation = Location;
			Socket.RelativeRotation = FRotator::ZeroRotator;
			Socket.RelativeScale = FVector::OneVector;
		};
		AddSocket(TEXT("HP_Top_02"), FVector(15.0, 0.0, 82.0));
		AddSocket(TEXT("Wheel_Anchor_FL"), FVector(100.0, -60.0, 25.0));
		AddSocket(TEXT("Wheel_Anchor_FR"), FVector(100.0, 60.0, 25.0));
		AddSocket(TEXT("Wheel_Anchor_RL"), FVector(-100.0, -60.0, 25.0));
		AddSocket(TEXT("Wheel_Anchor_RR"), FVector(-100.0, 60.0, 25.0));
		OutAssets.ChassisSockets.Sort([](const FCFVehicleSocketSnapshot& Left, const FCFVehicleSocketSnapshot& Right)
		{
			return Left.SocketName.LexicalLess(Right.SocketName);
		});

		// Cube local bounds를 네 Wheel measurement fact에 공통 적용합니다.
		const FBoxSphereBounds CubeBounds = CubeMesh->GetBounds();
		// 네 wheel snapshot storage입니다.
		FCFVehicleWheelAssetSnapshot* WheelSnapshots[] = {&OutAssets.WheelFL, &OutAssets.WheelFR, &OutAssets.WheelRL, &OutAssets.WheelRR};
		// 각 wheel의 deterministic measurement fingerprint입니다.
		const TCHAR* WheelFingerprints[] = {TEXT("import-wheel-fl-v1"), TEXT("import-wheel-fr-v1"), TEXT("import-wheel-rl-v1"), TEXT("import-wheel-rr-v1")};
		for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(WheelSnapshots); ++WheelIndex)
		{
			WheelSnapshots[WheelIndex]->ObjectPath = CubePath;
			WheelSnapshots[WheelIndex]->bAssetLoaded = true;
			WheelSnapshots[WheelIndex]->BoundsOrigin = FVector(CubeBounds.Origin);
			WheelSnapshots[WheelIndex]->BoundsExtent = FVector(CubeBounds.BoxExtent);
			WheelSnapshots[WheelIndex]->MeasureFingerprint = WheelFingerprints[WheelIndex];
		}

		OutError.Reset();
		return true;
	}

	// Imported Recipe Snapshot + Current Definition + Project Defaults + Asset facts로 Adoption Resolver request를 구성합니다.
	bool BuildImportedRequest(
		const UCFVehicleRecipeData& Recipe,
		const FCFVehicleDefinitionSnapshot& CurrentDefinition,
		FCFVehicleResolveRequest& OutRequest,
		FString& OutError)
	{
		OutRequest = FCFVehicleResolveRequest();
		OutRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, OutRequest.Recipe, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(OutRequest.ProjectDefaults, OutError)
			|| !BuildAssetSnapshot(OutRequest.Assets, OutError))
		{
			return false;
		}

		OutRequest.bHasCurrentDefinition = true;
		OutRequest.CurrentDefinition = CurrentDefinition;
		OutError.Reset();
		return true;
	}

	// Registry wildcard descriptor가 exact path를 소유하는지 검사합니다.
	bool DoesPathMatchDescriptor(
		const FCFVehicleFieldPath& ExactPath,
		const FCFVehicleFieldDescriptor& Descriptor)
	{
		const FCFVehicleFieldPath& Pattern = Descriptor.StablePathPattern;
		return ExactPath.CollectionPropertyName == Pattern.CollectionPropertyName
			&& ExactPath.SelectorKeyPropertyName == Pattern.SelectorKeyPropertyName
			&& ExactPath.PropertyChain == Pattern.PropertyChain
			&& (Pattern.CollectionPropertyName.IsNone() ? ExactPath.SelectorKeyValue.IsNone() : !ExactPath.SelectorKeyValue.IsNone());
	}

	// Exact field path를 소유하는 Frozen Registry descriptor를 찾습니다.
	const FCFVehicleFieldDescriptor* FindDescriptor(const FCFVehicleFieldPath& ExactPath)
	{
		return FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([&ExactPath](const FCFVehicleFieldDescriptor& Descriptor)
		{
			return DoesPathMatchDescriptor(ExactPath, Descriptor);
		});
	}

	// Legacy override 배열에서 canonical exact path row를 찾습니다.
	const FCFVehicleFieldOverride* FindOverride(
		const TArray<FCFVehicleFieldOverride>& Overrides,
		const TCHAR* CanonicalPath)
	{
		return Overrides.FindByPredicate([CanonicalPath](const FCFVehicleFieldOverride& Override)
		{
			return Override.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// Resolve result에서 canonical exact field Source Trace를 찾습니다.
	const FCFVehicleSourceTrace* FindTrace(
		const FCFVehicleResolveResult& ResolveResult,
		const TCHAR* CanonicalPath)
	{
		return ResolveResult.PreviewSourceTrace.FindByPredicate([CanonicalPath](const FCFVehicleSourceTrace& Trace)
		{
			return Trace.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// Source Trace의 effective layer type을 fail-safe로 반환합니다.
	ECFVehicleSourceType GetEffectiveSourceType(const FCFVehicleSourceTrace* Trace)
	{
		if (!Trace || !Trace->Layers.IsValidIndex(Trace->EffectiveLayerIndex))
		{
			return ECFVehicleSourceType::ProjectCompatibilityDefault;
		}
		return Trace->Layers[Trace->EffectiveLayerIndex].SourceType;
	}

	// Exact canonical path가 preview removal path 집합에 포함되는지 검사합니다.
	bool ContainsPath(
		const TArray<FCFVehicleFieldPath>& Paths,
		const TCHAR* CanonicalPath)
	{
		return Paths.ContainsByPredicate([CanonicalPath](const FCFVehicleFieldPath& Path)
		{
			return Path.ToCanonicalString(true) == CanonicalPath;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleExistingImportTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Import.LosslessPins",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAdoptionPreviewTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Import.AdoptionPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAdoptionCommitTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Import.AdoptionCommit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Current Definition 전체를 Legacy baseline으로 lossless 보존하고 hidden Mount 10 leaf를 분리하는지 검증합니다.
bool FCFVehicleExistingImportTest::RunTest(const FString& Parameters)
{
	// Content package와 무관한 transient source Definition object입니다.
	UCFVehicleData* SourceVehicle = NewObject<UCFVehicleData>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Transient source VehicleData exists"), SourceVehicle))
	{
		return false;
	}

	// Import source exact Definition Snapshot입니다.
	FCFVehicleDefinitionSnapshot SourceDefinition;
	// Fixture/Snapshot 실패 이유입니다.
	FString Error;
	if (!TestTrue(TEXT("Source Definition Snapshot builds"), CFVehicleImportTestsPrivate::BuildSourceDefinition(*SourceVehicle, SourceDefinition, Error)))
	{
		AddError(Error);
		return false;
	}

	// Recipe-only transaction을 담을 메모리 package입니다.
	UPackage* RecipePackage = nullptr;
	// Existing Definition import 대상 test Recipe입니다.
	UCFVehicleRecipeData* Recipe = CFVehicleImportTestsPrivate::CreateRecipe(RecipePackage);
	if (!TestNotNull(TEXT("In-memory Recipe exists"), Recipe))
	{
		return false;
	}

	// Import가 raw Movement에서 역산하면 안 되는 preexisting semantic feel 값입니다.
	const FCFVehicleFeelIntent FeelBeforeImport = Recipe->DrivingFeelIntent;
	// Existing Definition → Recipe migration state import 결과입니다.
	FCFVehicleImportResult ImportResult;
	if (!TestTrue(TEXT("Existing Definition import succeeds"), FCFVehicleImportService::ImportDefinitionSnapshot(SourceDefinition, *Recipe, ImportResult, Error)))
	{
		AddError(Error);
		return false;
	}

	TestEqual(TEXT("Imported Definition hash is preserved"), ImportResult.ImportedDefinitionHash, SourceDefinition.DefinitionHash);
	TestEqual(TEXT("Import state starts LegacyImported"), Recipe->ImportState.ManageState, ECFVehicleManageState::LegacyImported);
	TestEqual(TEXT("Normal + serialized baselines cover every exact Definition field"), ImportResult.LegacyPinnedFieldCount + ImportResult.LegacySerializedFieldCount, SourceDefinition.SortedFields.Num());
	TestEqual(TEXT("One Mount contributes exact hidden serialized ten leaves"), ImportResult.LegacySerializedFieldCount, 10);
	TestTrue(TEXT("Normal Legacy Pins exist"), ImportResult.LegacyPinnedFieldCount > 0);

	for (const FCFVehicleFieldOverride& SerializedField : Recipe->ImportState.LegacySerializedFields)
	{
		// Hidden serialized row를 소유하는 descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = CFVehicleImportTestsPrivate::FindDescriptor(SerializedField.FieldPath);
		TestTrue(TEXT("Every LegacySerialized field maps to a hidden serialized descriptor"), Descriptor && Descriptor->bLegacySerialized);
	}
	for (const FCFVehicleFieldOverride& PinnedField : Recipe->ImportState.LegacyPinnedFields)
	{
		// Normal Legacy Pin row를 소유하는 descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = CFVehicleImportTestsPrivate::FindDescriptor(PinnedField.FieldPath);
		TestTrue(TEXT("Normal Legacy Pin never contains hidden serialized descriptor"), Descriptor && !Descriptor->bLegacySerialized);
	}

	TestEqual(TEXT("Base mass copied as explicit semantic candidate"), Recipe->MassIntent.ExplicitBaseMassKg, 1635.0f);
	TestEqual(TEXT("Gross mass copied as explicit semantic candidate"), Recipe->MassIntent.ExplicitGrossMassKg, 2480.0f);
	TestEqual(TEXT("Durability copied as explicit semantic candidate"), Recipe->DurabilityIntent.ExplicitMaxHealth, 315.0f);
	TestEqual(TEXT("DriveState override mode copied semantically"), Recipe->DriveStateMode, ECFVehicleDriveStateMode::VehicleSpecific);
	TestEqual(TEXT("Hardpoint semantic candidate count"), Recipe->HardpointIntents.Num(), 1);
	TestEqual(TEXT("Hardpoint stable identity copied"), Recipe->HardpointIntents[0].LocationSlotId, FName(TEXT("Top_02")));
	TestEqual(TEXT("Mount semantic candidate count"), Recipe->MountIntents.Num(), 1);
	TestEqual(TEXT("Mount stable identity copied"), Recipe->MountIntents[0].MountProfileId, FName(TEXT("M_Z")));
	TestEqual(TEXT("Mount active LocationSlotRef copied"), Recipe->MountIntents[0].LocationSlotRef, FName(TEXT("Top_02")));
	TestTrue(TEXT("Chassis hard reference became same-asset soft semantic reference"), Recipe->AssetIntent.ChassisMesh.ToSoftObjectPath().ToString().Contains(TEXT("/Engine/BasicShapes/Cube.Cube")));

	// Distinctive hidden legacy value가 normal semantic MountIntent가 아니라 LegacySerializedFields에 보존됐는지 확인합니다.
	const FCFVehicleFieldOverride* HiddenYawRate = CFVehicleImportTestsPrivate::FindOverride(
		Recipe->ImportState.LegacySerializedFields,
		TEXT("MountProfiles[MountProfileId=M_Z].YawTurnRateDegPerSec"));
	if (TestNotNull(TEXT("Hidden Mount Yaw rate is preserved only as LegacySerialized baseline"), HiddenYawRate))
	{
		TestEqual(TEXT("Hidden Yaw rate value is lossless"), FCString::Atof(*HiddenYawRate->OverrideValue.CanonicalValueText), 77.0f);
	}

	TestEqual(TEXT("Acceleration Feel is not inferred from raw Movement"), Recipe->DrivingFeelIntent.AccelerationFeel, FeelBeforeImport.AccelerationFeel);
	TestEqual(TEXT("Steering Feel is not inferred from raw Movement"), Recipe->DrivingFeelIntent.SteeringAgility, FeelBeforeImport.SteeringAgility);
	TestEqual(TEXT("Grip Feel is not inferred from raw Movement"), Recipe->DrivingFeelIntent.GripFeel, FeelBeforeImport.GripFeel);
	TestEqual(TEXT("Suspension Feel is not inferred from raw Movement"), Recipe->DrivingFeelIntent.SuspensionFirmness, FeelBeforeImport.SuspensionFirmness);
	TestTrue(TEXT("Vehicle Base Profile is not auto-bound"), Recipe->ProfileBindings.VehicleBaseProfile.IsNull());
	TestTrue(TEXT("Handling Profile is not auto-bound"), Recipe->ProfileBindings.HandlingProfile.IsNull());
	TestTrue(TEXT("Performance Profile is not auto-bound"), Recipe->ProfileBindings.PerformanceProfile.IsNull());

	// Imported Recipe와 original source Definition을 함께 넣은 Resolver request입니다.
	FCFVehicleResolveRequest ResolveRequest;
	if (!TestTrue(TEXT("Imported Resolver request builds"), CFVehicleImportTestsPrivate::BuildImportedRequest(*Recipe, SourceDefinition, ResolveRequest, Error)))
	{
		AddError(Error);
		return false;
	}

	// Full Legacy Pin baseline이 effective ownership을 유지하는 Resolver result입니다.
	FCFVehicleResolveResult ResolveResult;
	TestTrue(TEXT("Imported Legacy baseline Resolve has no internal error"), FCFVehicleResolver::Resolve(ResolveRequest, ResolveResult));
	TestEqual(TEXT("Legacy baseline reconstructs exact imported Definition hash"), ResolveResult.ResolvedDefinitionHash, SourceDefinition.DefinitionHash);

	// Representative normal Pin Source Trace입니다.
	const FCFVehicleSourceTrace* MassTrace = CFVehicleImportTestsPrivate::FindTrace(ResolveResult, TEXT("BaseVehicleMassKg"));
	TestEqual(TEXT("Imported mass stays Legacy Pin effective"), CFVehicleImportTestsPrivate::GetEffectiveSourceType(MassTrace), ECFVehicleSourceType::LegacyImportedPinnedBaseline);
	// Representative hidden serialized Source Trace입니다.
	const FCFVehicleSourceTrace* HiddenTrace = CFVehicleImportTestsPrivate::FindTrace(ResolveResult, TEXT("MountProfiles[MountProfileId=M_Z].MountWeightKg"));
	TestEqual(TEXT("Hidden Mount field stays LegacySerialized passthrough effective"), CFVehicleImportTestsPrivate::GetEffectiveSourceType(HiddenTrace), ECFVehicleSourceType::LegacySerializedPassthrough);

	// Import가 Target UCFVehicleData를 수정하지 않았음을 다시 캡처할 post-import Snapshot입니다.
	FCFVehicleDefinitionSnapshot SourceDefinitionAfterImport;
	TestTrue(TEXT("Target source re-snapshot succeeds"), FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*SourceVehicle, SourceDefinitionAfterImport, Error));
	TestEqual(TEXT("Target UCFVehicleData hash is unchanged by Recipe import"), SourceDefinitionAfterImport.DefinitionHash, SourceDefinition.DefinitionHash);
	return true;
}

// Group/field Adoption Preview가 persistent Recipe를 수정하지 않고 선택 Legacy Pin만 가상 제외하는지 검증합니다.
bool FCFVehicleAdoptionPreviewTest::RunTest(const FString& Parameters)
{
	// Content package와 무관한 transient source Definition object입니다.
	UCFVehicleData* SourceVehicle = NewObject<UCFVehicleData>(GetTransientPackage(), NAME_None, RF_Transient);
	// Import source exact Definition Snapshot입니다.
	FCFVehicleDefinitionSnapshot SourceDefinition;
	// Fixture/Import/Preview 실패 이유입니다.
	FString Error;
	if (!SourceVehicle || !CFVehicleImportTestsPrivate::BuildSourceDefinition(*SourceVehicle, SourceDefinition, Error))
	{
		AddError(Error);
		return false;
	}

	// Recipe-only transaction을 담을 메모리 package입니다.
	UPackage* RecipePackage = nullptr;
	// Existing Definition import 대상 test Recipe입니다.
	UCFVehicleRecipeData* Recipe = CFVehicleImportTestsPrivate::CreateRecipe(RecipePackage);
	// Initial import 결과입니다.
	FCFVehicleImportResult ImportResult;
	if (!Recipe || !FCFVehicleImportService::ImportDefinitionSnapshot(SourceDefinition, *Recipe, ImportResult, Error))
	{
		AddError(Error);
		return false;
	}

	// Preview 전 persistent Recipe Snapshot입니다.
	FCFVehicleRecipeSnapshot BeforePreviewSnapshot;
	TestTrue(TEXT("Before-preview Recipe Snapshot builds"), FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, BeforePreviewSnapshot, Error));
	// Preview 전 persistent normal Legacy Pin 수입니다.
	const int32 PinCountBeforePreview = Recipe->ImportState.LegacyPinnedFields.Num();
	// Preview 전 Recipe revision입니다.
	const int32 RevisionBeforePreview = Recipe->AuthoringRevision;

	// Current Legacy baseline Resolver request입니다.
	FCFVehicleResolveRequest CurrentRequest;
	if (!TestTrue(TEXT("Adoption preview request builds"), CFVehicleImportTestsPrivate::BuildImportedRequest(*Recipe, SourceDefinition, CurrentRequest, Error)))
	{
		AddError(Error);
		return false;
	}

	// MassDurability group Pin만 가상 제거한 preview입니다.
	FCFVehicleAdoptionPreview GroupPreview;
	if (!TestTrue(TEXT("MassDurability group preview builds"), FCFVehicleImportService::PreviewGroupAdoption(CurrentRequest, ECFVehicleAdoptGroup::MassDurability, GroupPreview, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("MassDurability group has a prospective non-Legacy source"), GroupPreview.bCanCommit);
	TestTrue(TEXT("Group preview removes BaseVehicleMassKg virtually"), CFVehicleImportTestsPrivate::ContainsPath(GroupPreview.LegacyPinPathsToRemove, TEXT("BaseVehicleMassKg")));
	TestTrue(TEXT("Group preview removes MaximumGrossMassKg virtually"), CFVehicleImportTestsPrivate::ContainsPath(GroupPreview.LegacyPinPathsToRemove, TEXT("MaximumGrossMassKg")));
	TestTrue(TEXT("Group preview removes MaxHealth virtually"), CFVehicleImportTestsPrivate::ContainsPath(GroupPreview.LegacyPinPathsToRemove, TEXT("VehicleDurabilityConfig.MaxHealth")));

	// Current and prospective Base mass traces입니다.
	const FCFVehicleSourceTrace* CurrentMassTrace = CFVehicleImportTestsPrivate::FindTrace(GroupPreview.CurrentResolveResult, TEXT("BaseVehicleMassKg"));
	// Prospective Preview에서 새 semantic source가 effective한 trace입니다.
	const FCFVehicleSourceTrace* ProspectiveMassTrace = CFVehicleImportTestsPrivate::FindTrace(GroupPreview.ProspectiveResolveResult, TEXT("BaseVehicleMassKg"));
	TestEqual(TEXT("Current group preview retains Legacy Pin source"), CFVehicleImportTestsPrivate::GetEffectiveSourceType(CurrentMassTrace), ECFVehicleSourceType::LegacyImportedPinnedBaseline);
	TestTrue(TEXT("Prospective group preview releases mass from Legacy ownership"), CFVehicleImportTestsPrivate::GetEffectiveSourceType(ProspectiveMassTrace) != ECFVehicleSourceType::LegacyImportedPinnedBaseline);

	// Preview가 persistent Recipe를 변경하지 않았음을 확인할 post-preview Snapshot입니다.
	FCFVehicleRecipeSnapshot AfterPreviewSnapshot;
	TestTrue(TEXT("After-preview Recipe Snapshot builds"), FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, AfterPreviewSnapshot, Error));
	TestEqual(TEXT("Group preview does not change persistent Recipe fingerprint"), AfterPreviewSnapshot.RecipeFingerprint, BeforePreviewSnapshot.RecipeFingerprint);
	TestEqual(TEXT("Group preview does not change persistent pin count"), Recipe->ImportState.LegacyPinnedFields.Num(), PinCountBeforePreview);
	TestEqual(TEXT("Group preview does not change persistent revision"), Recipe->AuthoringRevision, RevisionBeforePreview);

	// Identity field의 field-level Adoption 금지를 검증할 exact path입니다.
	FCFVehicleFieldPath IdentityPath;
	for (const FCFVehicleFieldOverride& Pin : Recipe->ImportState.LegacyPinnedFields)
	{
		if (Pin.FieldPath.ToCanonicalString(true) == TEXT("HardpointSlots[LocationSlotId=Top_02].LocationSlotId"))
		{
			IdentityPath = Pin.FieldPath;
			break;
		}
	}
	// Identity field 금지 결과입니다.
	FCFVehicleAdoptionPreview RejectedIdentityPreview;
	TestFalse(TEXT("Identity field-level Adoption is rejected"), FCFVehicleImportService::PreviewFieldAdoption(CurrentRequest, IdentityPath, RejectedIdentityPreview, Error));

	// Hidden LegacySerialized field의 field-level Adoption 금지를 검증할 exact path입니다.
	FCFVehicleFieldPath HiddenPath;
	if (const FCFVehicleFieldOverride* HiddenPin = CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacySerializedFields, TEXT("MountProfiles[MountProfileId=M_Z].MountWeightKg")))
	{
		HiddenPath = HiddenPin->FieldPath;
	}
	// Hidden serialized field 금지 결과입니다.
	FCFVehicleAdoptionPreview RejectedHiddenPreview;
	TestFalse(TEXT("Hidden LegacySerialized field-level Adoption is rejected"), FCFVehicleImportService::PreviewFieldAdoption(CurrentRequest, HiddenPath, RejectedHiddenPreview, Error));

	// Normal advanced field-level Adoption 대상 exact path입니다.
	FCFVehicleFieldPath DestroyedSocketPath;
	if (const FCFVehicleFieldOverride* DestroyedSocketPin = CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("DestroyedFxSocketName")))
	{
		DestroyedSocketPath = DestroyedSocketPin->FieldPath;
	}
	// Exact field 하나만 virtual release한 preview입니다.
	FCFVehicleAdoptionPreview FieldPreview;
	if (!TestTrue(TEXT("DestroyedFxSocketName field preview builds"), FCFVehicleImportService::PreviewFieldAdoption(CurrentRequest, DestroyedSocketPath, FieldPreview, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Field preview can commit when direct semantic candidate exists"), FieldPreview.bCanCommit);
	TestEqual(TEXT("Field preview removes exactly one direct pin"), FieldPreview.LegacyPinPathsToRemove.Num(), 1);
	TestTrue(TEXT("Field preview removal is DestroyedFxSocketName"), CFVehicleImportTestsPrivate::ContainsPath(FieldPreview.LegacyPinPathsToRemove, TEXT("DestroyedFxSocketName")));
	return true;
}

// Approved Adoption이 선택 Pin/AdoptedGroups/revision만 Recipe에 반영하고 stale preview와 Target mutation을 차단하는지 검증합니다.
bool FCFVehicleAdoptionCommitTest::RunTest(const FString& Parameters)
{
	// Content package와 무관한 transient source Definition object입니다.
	UCFVehicleData* SourceVehicle = NewObject<UCFVehicleData>(GetTransientPackage(), NAME_None, RF_Transient);
	// Import source exact Definition Snapshot입니다.
	FCFVehicleDefinitionSnapshot SourceDefinition;
	// Fixture/Import/Preview/Commit 실패 이유입니다.
	FString Error;
	if (!SourceVehicle || !CFVehicleImportTestsPrivate::BuildSourceDefinition(*SourceVehicle, SourceDefinition, Error))
	{
		AddError(Error);
		return false;
	}

	// Recipe-only transaction을 담을 메모리 package입니다.
	UPackage* RecipePackage = nullptr;
	// Existing Definition import 대상 test Recipe입니다.
	UCFVehicleRecipeData* Recipe = CFVehicleImportTestsPrivate::CreateRecipe(RecipePackage);
	// Initial import 결과입니다.
	FCFVehicleImportResult ImportResult;
	if (!Recipe || !FCFVehicleImportService::ImportDefinitionSnapshot(SourceDefinition, *Recipe, ImportResult, Error))
	{
		AddError(Error);
		return false;
	}

	// Group adoption 직전 Resolver request입니다.
	FCFVehicleResolveRequest GroupRequest;
	if (!CFVehicleImportTestsPrivate::BuildImportedRequest(*Recipe, SourceDefinition, GroupRequest, Error))
	{
		AddError(Error);
		return false;
	}
	// 승인할 MassDurability group preview입니다.
	FCFVehicleAdoptionPreview GroupPreview;
	if (!FCFVehicleImportService::PreviewGroupAdoption(GroupRequest, ECFVehicleAdoptGroup::MassDurability, GroupPreview, Error) || !GroupPreview.bCanCommit)
	{
		AddError(GroupPreview.BlockReason.IsEmpty() ? Error : GroupPreview.BlockReason);
		return false;
	}

	// Group commit 직전 Recipe revision입니다.
	const int32 RevisionBeforeGroupCommit = Recipe->AuthoringRevision;
	TestTrue(TEXT("Approved group Adoption commit succeeds"), FCFVehicleImportService::CommitAdoption(*Recipe, GroupPreview, Error));
	TestEqual(TEXT("Group Adoption increments Recipe revision once"), Recipe->AuthoringRevision, RevisionBeforeGroupCommit + 1);
	TestTrue(TEXT("Group Adoption records AdoptedGroups"), Recipe->ImportState.AdoptedGroups.Contains(ECFVehicleAdoptGroup::MassDurability));
	TestEqual(TEXT("Remaining legacy pins keep Recipe PartiallyManaged"), Recipe->ImportState.ManageState, ECFVehicleManageState::PartiallyManaged);
	TestNull(TEXT("Committed BaseVehicleMassKg pin is removed"), CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("BaseVehicleMassKg")));
	TestNull(TEXT("Committed MaximumGrossMassKg pin is removed"), CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("MaximumGrossMassKg")));
	TestNull(TEXT("Committed MaxHealth pin is removed"), CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("VehicleDurabilityConfig.MaxHealth")));
	TestEqual(TEXT("Hidden LegacySerialized fields remain untouched by normal group commit"), Recipe->ImportState.LegacySerializedFields.Num(), 10);

	// Group commit 뒤 semantic Recipe fingerprint가 preview prospective fingerprint와 일치하는 Snapshot입니다.
	FCFVehicleRecipeSnapshot AfterGroupCommitSnapshot;
	TestTrue(TEXT("Post-group-commit Recipe Snapshot builds"), FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Recipe, AfterGroupCommitSnapshot, Error));
	TestEqual(TEXT("Group commit matches approved prospective Recipe fingerprint"), AfterGroupCommitSnapshot.RecipeFingerprint, GroupPreview.ProspectiveRecipeFingerprint);

	// Field adoption용 fresh current request입니다.
	FCFVehicleResolveRequest FieldRequest;
	if (!CFVehicleImportTestsPrivate::BuildImportedRequest(*Recipe, SourceDefinition, FieldRequest, Error))
	{
		AddError(Error);
		return false;
	}
	// DestroyedFxSocketName exact pin path입니다.
	FCFVehicleFieldPath DestroyedSocketPath;
	if (const FCFVehicleFieldOverride* DestroyedSocketPin = CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("DestroyedFxSocketName")))
	{
		DestroyedSocketPath = DestroyedSocketPin->FieldPath;
	}
	// Stale guard 검증에 사용할 first field preview입니다.
	FCFVehicleAdoptionPreview StaleFieldPreview;
	if (!FCFVehicleImportService::PreviewFieldAdoption(FieldRequest, DestroyedSocketPath, StaleFieldPreview, Error) || !StaleFieldPreview.bCanCommit)
	{
		AddError(StaleFieldPreview.BlockReason.IsEmpty() ? Error : StaleFieldPreview.BlockReason);
		return false;
	}

	// Preview 뒤 semantic payload를 바꿔 stale preview를 의도적으로 만듭니다.
	Recipe->DefaultDataIntent.DestroyedFxSocketName = TEXT("FX_Changed_After_Preview");
	// Stale commit 전 revision입니다.
	const int32 RevisionBeforeStaleCommit = Recipe->AuthoringRevision;
	TestFalse(TEXT("Stale field Adoption preview is rejected"), FCFVehicleImportService::CommitAdoption(*Recipe, StaleFieldPreview, Error));
	TestEqual(TEXT("Rejected stale commit does not change revision"), Recipe->AuthoringRevision, RevisionBeforeStaleCommit);
	TestNotNull(TEXT("Rejected stale commit does not remove pin"), CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("DestroyedFxSocketName")));

	// Original semantic value를 복원해 fresh preview를 다시 만듭니다.
	Recipe->DefaultDataIntent.DestroyedFxSocketName = TEXT("FX_Imported_Destroyed");
	// Fresh field preview를 만들 request입니다.
	FCFVehicleResolveRequest FreshFieldRequest;
	TestTrue(TEXT("Fresh field request rebuilds"), CFVehicleImportTestsPrivate::BuildImportedRequest(*Recipe, SourceDefinition, FreshFieldRequest, Error));
	// Fresh exact field preview입니다.
	FCFVehicleAdoptionPreview FreshFieldPreview;
	if (!TestTrue(TEXT("Fresh field preview rebuilds"), FCFVehicleImportService::PreviewFieldAdoption(FreshFieldRequest, DestroyedSocketPath, FreshFieldPreview, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Fresh field preview is committable"), FreshFieldPreview.bCanCommit);

	// Fresh field commit 직전 revision입니다.
	const int32 RevisionBeforeFieldCommit = Recipe->AuthoringRevision;
	TestTrue(TEXT("Approved field Adoption commit succeeds"), FCFVehicleImportService::CommitAdoption(*Recipe, FreshFieldPreview, Error));
	TestEqual(TEXT("Field Adoption increments Recipe revision once"), Recipe->AuthoringRevision, RevisionBeforeFieldCommit + 1);
	TestNull(TEXT("Field Adoption removes only exact DestroyedFxSocketName pin"), CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("DestroyedFxSocketName")));
	TestFalse(TEXT("Field Adoption does not mark whole DefenseFx group adopted"), Recipe->ImportState.AdoptedGroups.Contains(ECFVehicleAdoptGroup::DefenseFx));
	TestTrue(TEXT("Unrelated DefenseFx DefaultDefenseData pin remains"), CFVehicleImportTestsPrivate::FindOverride(Recipe->ImportState.LegacyPinnedFields, TEXT("DefaultDefenseData")) != nullptr);

	// Recipe-only commit 후 Target UCFVehicleData가 그대로인지 확인할 fresh source Snapshot입니다.
	FCFVehicleDefinitionSnapshot SourceDefinitionAfterCommits;
	TestTrue(TEXT("Target source re-snapshot after commits succeeds"), FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*SourceVehicle, SourceDefinitionAfterCommits, Error));
	TestEqual(TEXT("Adoption commits never mutate Target UCFVehicleData"), SourceDefinitionAfterCommits.DefinitionHash, SourceDefinition.DefinitionHash);
	return true;
}

#endif
