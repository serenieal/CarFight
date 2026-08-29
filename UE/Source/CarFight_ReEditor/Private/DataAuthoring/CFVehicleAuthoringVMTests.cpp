// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringVMTests.cpp
// Version: v1.21.0
// Date: 2026-08-28
// Description: DAUTH-P0-09~12 Vehicle Authoring Workspace + WSA Builder integration 보호 Automation입니다.
// Changelog:
// - v1.21.0: actual Wagon E2E 회귀로 Recipe-only revision 6이 있는 Managed NewVehicle의 private Profile 0/4 bootstrap selection/refresh 허용을 BuilderShell에 추가.
// - v1.20.0: WSA-P0-04 BuilderShell에 Socket mode canonical Wheel, Socket Scale/axle validation, Scale-only Layout stale 회귀를 추가.
// - v1.19.0: E2E에서 발견된 Step 2 Wheel Mesh 지정 UX의 typed Recipe-only commit, FL missing Blocked, FL restore Complete, VehicleData hash 불변 회귀검증을 추가.
// - v1.18.0: USER 피드백 기반 Builder Step 3 소켓 준비 UX의 stable title, required 4-role exact name, current found-state, custom binding projection 회귀검증을 추가.
// - v1.17.0: VB-P0-09 Step 8 saved-state gate, exact TargetHash+RunId benchmark binding, active PIE 전 USER PASS 차단, exact resume token/stale invalidation focused regression을 추가.
// - v1.16.0: VB-P0-09 Step 7 Guided VM의 fresh Final Review→explicit DefinitionApply→Complete→exact guarded Undo→Ready focused integration regression을 추가.
// - v1.15.0: VB-P0-09 Step 6 existing ReadBuilderGameplayGuidance R0 Shell projection의 8영역 Complete/mutation0/fresh VM resume와 Step 5 drift prerequisite lock 회귀검증을 추가.
// - v1.14.0: VB-P0-09 Step 5 PhysicsDraft→private 4 Profile typed Preview/Commit→persistent receipt resume→Profile drift Stale 통합 회귀검증을 추가.
// - v1.13.0: VB-P0-09 Step 1 ResearchDraft→Companion Preview/Commit→Reference review token→fresh VM resume→Evidence fingerprint drift Stale 통합 회귀검증을 추가.
// - v1.12.0: BuilderShell 검증을 array index/exact-8 결합에서 stable StepId lookup으로 전환해 Step 추가·재배치 회귀 내성을 확보.
// - v1.11.0: CF-FQ-040 VB-P0-09 BuilderShell에 VB-P0-03 fresh AssetSnapshot 기반 Mesh/Socket/Layout baseline, socket-transform stale, duplicate-role blocker 회귀검증 추가.
// - v1.10.0: UA-06 USER UX remediation의 Registry 기반 Shared Profile field selector가 사용하는 current numeric authored value read-only helper를 B2 commit 전/후 exact 값으로 검증.
// - v1.9.0: UA-07 readiness에서 reviewed Profile Bind→explicit Unbind가 Recipe binding만 원래 없음 상태로 복구하고 Target/Profile/Save를 변경하지 않는 focused regression 추가.
// - v1.8.0: UA-06 readiness regression으로 Workspace transaction 뒤 unrelated Editor transaction이 끼면 custom Undo를 차단하고, intervening Undo 후 exact Workspace transaction만 되돌리는지 검증.
// - v1.7.0: UA-03 regression으로 Handling을 참조하는 invalid `/Engine/Transient` scratch Recipe가 살아 있어도 B2 affected inventory에서 제외되고 persistent Recipe preview/commit이 성공하는지 검증.
// - v1.6.0: P0-12 UA-03 회귀 보호로 Shared Profile fixture를 raw ProfileBindings 대입하지 않고 actual reviewed BindVehicleProfile Recipe-only 경로로 연결해 B2 impact까지 검증.
// - v1.5.0: P0-12 UA-02 회귀 보호로 existing chassis와 다른 sibling vehicle directory의 unused chassis mesh도 후보로 발견되는지 검증 추가.
// - v1.4.0: P0-12 UA-01 회귀 보호로 실제 WheelMesh 참조가 Mesh-only 차체 후보에 섞이지 않는지 검증 추가.
// - v1.3.0: P0-11 Frozen 24.90~24.94 Adoption/Profile Impact/Drift 3-way/Mesh Candidate creation focused closure 검증 추가.
// - v1.2.0: P0-11 representative Authoring E2E와 applied VehicleData의 Inventory/Fitting consumer non-regression 검증 추가.
// - v1.1.0: P0-10 Reference Compare, typed Layout/Driving Feel+Undo, Measurement/Adoption, legacy Wizard managed-target guard parity 검증 추가.
// - v1.0.0: ViewModel↔facade parity, preview mutation0, Initial Import mutation boundary, Apply/standard Undo, stale approval fail-closed 검증 추가.
// Migration:
// - 테스트 fixture만 직접 UCFVehicleData를 구성하며 production UI/ViewModel은 FCFVehicleAuthoringService facade만 사용합니다.
// - Content Asset/file save를 수행하지 않으며 Initial Import asset은 테스트 종료 시 Asset Registry에서 제거합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "CFInventoryFitAdapter.h"
#include "CFVehicleFittingData.h"
#include "CFVDAWizardTestAccess.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP10ReferenceTest,
	"CarFight.DataAuthoring.DAUTH_P0_10.Workspace.ReferenceCompare",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP10SemanticUndoTest,
	"CarFight.DataAuthoring.DAUTH_P0_10.Workspace.LayoutDrivingUndo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP10AdoptionTest,
	"CarFight.DataAuthoring.DAUTH_P0_10.Workspace.AdoptionMeasurement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP10LegacyGuardTest,
	"CarFight.DataAuthoring.DAUTH_P0_10.Workspace.LegacyManagedGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11AuthoringE2ETest,
	"CarFight.DataAuthoring.DAUTH_P0_11.Workspace.AuthoringE2E",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11ConsumerTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.Workspace.ConsumerRegression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11AdoptionTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.AdoptionCompleteness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11ProfileImpactTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.SharedProfileImpact",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP12ProfileUnbindTest,
	"CarFight.DataAuthoring.DAUTH_P0_12.Workspace.ProfileUnbind",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11DriftRecoveryTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.DriftRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11MeshCreateTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Editor.h"
#include "Editor/Transactor.h"
#include "JsonObjectConverter.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ScopedTransaction.h"
#include "Engine/StaticMesh.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Engine/StaticMeshSocket.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleAuthoringVMTestsPrivate
{
		/** P0-09 Workspace Automation 한 건에 필요한 in-memory Recipe/Target truth입니다. */
	struct FWorkspaceFixture
	{
		// Target VehicleData를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* RecipePackage = nullptr;

		// ViewModel/Apply의 Runtime canonical Target fixture입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// Managed Workspace의 Editor-only Recipe fixture입니다.
		UCFVehicleRecipeData* Recipe = nullptr;

		// AssetReader/Validator가 읽는 transient Chassis mesh입니다.
		UStaticMesh* ChassisMesh = nullptr;
	};

	// Automation object를 격리할 unique /Temp package를 만듭니다.
	UPackage* CreateTestPackage(const TCHAR* Prefix)
	{
		// 다른 Automation run과 충돌하지 않는 unique package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// Transient Chassis에 deterministic socket을 추가합니다.
	void AddSocket(UStaticMesh& ChassisMesh, const FName SocketName, const FVector& RelativeLocation)
	{
		// Chassis가 소유하는 transient StaticMesh socket입니다.
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(&ChassisMesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = RelativeLocation;
		Socket->RelativeRotation = FRotator::ZeroRotator;
		Socket->RelativeScale = FVector::OneVector;
		ChassisMesh.AddSocket(Socket);
	}

	// Existing Validator/AssetReader/Resolver를 모두 통과하는 deterministic Target baseline을 구성합니다.
	bool ConfigureValidTarget(FWorkspaceFixture& InOutFixture, FString& OutError)
	{
		InOutFixture.TargetPackage = CreateTestPackage(TEXT("CFDAWorkspaceTarget"));
		if (!InOutFixture.TargetPackage)
		{
			OutError = TEXT("Workspace Target /Temp package를 만들 수 없습니다.");
			return false;
		}

		// ViewModel/Apply가 사용할 canonical Target object입니다.
		InOutFixture.TargetVehicleData = NewObject<UCFVehicleData>(
			InOutFixture.TargetPackage,
			TEXT("DA_WorkspaceTarget_Test"),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!InOutFixture.TargetVehicleData)
		{
			OutError = TEXT("Workspace Target VehicleData를 만들 수 없습니다.");
			return false;
		}

		// Layout/Hardpoint source로 사용할 transient StaticMesh입니다.
		InOutFixture.ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		if (!InOutFixture.ChassisMesh)
		{
			OutError = TEXT("Workspace transient Chassis mesh를 만들 수 없습니다.");
			return false;
		}
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_FL"), FVector(110.0, -62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_FR"), FVector(110.0, 62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_RL"), FVector(-108.0, -62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_RR"), FVector(-108.0, 62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("HP_Top_Old"), FVector(0.0, 0.0, 82.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("HP_Front_New"), FVector(128.0, 0.0, 44.0));

		// Valid non-zero bounds를 제공하는 Engine read-only wheel mesh입니다.
		UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!WheelMesh)
		{
			OutError = TEXT("Workspace Engine Cube wheel fixture를 로드할 수 없습니다.");
			return false;
		}
		// Existing Validator의 required Wheel Class reference입니다.
		UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
		if (!WheelClass)
		{
			OutError = TEXT("Workspace ChaosVehicleWheel class를 로드할 수 없습니다.");
			return false;
		}

		InOutFixture.TargetVehicleData->VehicleVisualConfig.ChassisMesh = InOutFixture.ChassisMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFL = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFR = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRL = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRR = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass = WheelClass;
		InOutFixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass = WheelClass;
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides = true;
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation = FVector(110.0, -62.0, 28.0);
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeLocation = FVector(110.0, 62.0, 28.0);
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeLocation = FVector(-108.0, -62.0, 28.0);
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeLocation = FVector(-108.0, 62.0, 28.0);
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeRotation = FRotator::ZeroRotator;
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeRotation = FRotator::ZeroRotator;
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeRotation = FRotator::ZeroRotator;
		InOutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeRotation = FRotator::ZeroRotator;
		InOutFixture.TargetVehicleData->BaseVehicleMassKg = 1540.0f;
		InOutFixture.TargetVehicleData->MaximumGrossMassKg = 2280.0f;
		InOutFixture.TargetVehicleData->HardpointSlots.Reset();
		// Existing Definition에만 존재하는 old stable Hardpoint입니다.
		FCFVehicleHardpointSlot& OldHardpoint = InOutFixture.TargetVehicleData->HardpointSlots.AddDefaulted_GetRef();
		OldHardpoint.LocationSlotId = TEXT("Top_Old");
		OldHardpoint.LocationCategory = TEXT("Top");
		OldHardpoint.SocketName = TEXT("HP_Top_Old");
		OldHardpoint.LocalLocation = FVector(0.0, 0.0, 82.0);
		OldHardpoint.LocalRotation = FRotator::ZeroRotator;
		InOutFixture.TargetVehicleData->MountProfiles.Reset();
		InOutFixture.TargetPackage->SetDirtyFlag(false);
		OutError.Reset();
		return true;
	}

	// Current Definition을 shared Existing Import Core로 가져온 managed Recipe fixture를 생성합니다.
	bool BuildImportedFixture(FWorkspaceFixture& OutFixture, FString& OutError)
	{
		OutFixture = FWorkspaceFixture();
		if (!ConfigureValidTarget(OutFixture, OutError))
		{
			return false;
		}

		// Existing Definition exact snapshot입니다.
		FCFVehicleDefinitionSnapshot CurrentDefinition;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*OutFixture.TargetVehicleData, CurrentDefinition, OutError))
		{
			return false;
		}
		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFDAWorkspaceRecipe"));
		if (!OutFixture.RecipePackage)
		{
			OutError = TEXT("Workspace Recipe /Temp package를 만들 수 없습니다.");
			return false;
		}
		// ViewModel이 ResolveObject로 선택할 Editor-only Recipe fixture입니다.
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(
			OutFixture.RecipePackage,
			TEXT("DA_WorkspaceRecipe_Test"),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!OutFixture.Recipe)
		{
			OutError = TEXT("Workspace Recipe를 만들 수 없습니다.");
			return false;
		}
		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;

		// P0-08G shared Import Core 결과입니다.
		FCFVehicleImportResult ImportResult;
		if (!FCFVehicleImportService::ImportDefinitionSnapshot(CurrentDefinition, *OutFixture.Recipe, ImportResult, OutError))
		{
			return false;
		}
		OutFixture.TargetPackage->SetDirtyFlag(false);
		OutFixture.RecipePackage->SetDirtyFlag(false);
		OutError.Reset();
		return true;
	}

	// P0-08H와 동일한 structural Definition Apply difference를 Recipe desired state에 구성합니다.
	void ConfigureApplyDifference(UCFVehicleRecipeData& Recipe)
	{
		Recipe.ImportState.LegacyPinnedFields.RemoveAll([](const FCFVehicleFieldOverride& Override)
		{
			return Override.FieldPath.CollectionPropertyName == TEXT("HardpointSlots");
		});
		Recipe.ImportState.AdoptedGroups.Add(ECFVehicleAdoptGroup::Hardpoints);
		Recipe.ImportState.ManageState = Recipe.ImportState.LegacyPinnedFields.IsEmpty()
			? ECFVehicleManageState::Managed
			: ECFVehicleManageState::PartiallyManaged;
		Recipe.HardpointIntents.Reset();
		// Apply target에 새로 생성할 semantic Hardpoint입니다.
		FCFHardpointIntent& NewHardpoint = Recipe.HardpointIntents.AddDefaulted_GetRef();
		NewHardpoint.LocationSlotId = TEXT("Front_New");
		NewHardpoint.LocationCategory = TEXT("Front");
		NewHardpoint.SocketName = TEXT("HP_Front_New");
		Recipe.MountIntents.Reset();
		// New Hardpoint를 참조해 dependency-safe order를 요구하는 semantic Mount입니다.
		FCFMountIntent& NewMount = Recipe.MountIntents.AddDefaulted_GetRef();
		NewMount.MountProfileId = TEXT("M_Front_New");
		NewMount.LocationSlotRef = TEXT("Front_New");
		NewMount.MountType = ECFVehicleMountType::Turret;
		NewMount.SizeLimit = ECFVehicleWeaponSize::Medium;
		NewMount.DefaultEquipmentPresetData = nullptr;
		NewMount.bExposedModule = true;
		++Recipe.AuthoringRevision;
	}

	// Fixture를 ViewModel selection row로 표현합니다.
	FCFVehicleListEntry BuildListEntry(const FWorkspaceFixture& Fixture, const bool bIncludeRecipe)
	{
		// ViewModel single-Vehicle selection row입니다.
		FCFVehicleListEntry Entry;
		Entry.DefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
		if (bIncludeRecipe && Fixture.Recipe)
		{
			Entry.RecipePath = FSoftObjectPath(Fixture.Recipe);
			Entry.RecipeId = Fixture.Recipe->RecipeId;
			Entry.ManageState = Fixture.Recipe->ImportState.ManageState;
			Entry.AdvancedOverrideCount = Fixture.Recipe->AdvancedOverrides.Num();
		}
		return Entry;
	}

	// Current Target full Definition hash를 shared SnapshotBuilder로 반환합니다.
	FString BuildTargetHash(const UCFVehicleData& Target, FString& OutError)
	{
		// Hash authority가 될 exact full Definition Snapshot입니다.
		FCFVehicleDefinitionSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.DefinitionHash;
	}

	// Current Recipe semantic fingerprint를 shared SnapshotBuilder로 반환합니다.
	FString BuildRecipeFingerprint(const UCFVehicleRecipeData& Recipe, FString& OutError)
	{
		// Fingerprint authority가 될 exact Recipe Snapshot입니다.
		FCFVehicleRecipeSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.RecipeFingerprint;
	}

	// Workspace parity 비교용 direct facade read set을 구성합니다.
	bool BuildFacadeReadSet(
		FWorkspaceFixture& Fixture,
		FCFVehicleResolveReadResult& OutResolve,
		FCFVehicleDiffReadResult& OutDiff,
		FCFVehicleTraceReadResult& OutTrace,
		FCFVehicleValidationReadResult& OutValidation,
		FString& OutError)
	{
		// ViewModel과 같은 Recipe/Target facade request입니다.
		FCFVehicleAuthoringReadRequest Request;
		Request.Recipe = Fixture.Recipe;
		Request.TargetVehicleData = Fixture.TargetVehicleData;
		Request.CallerKind = ECFAuthoringCallerKind::Automation;
		// 전체 Trace projection을 뜻하는 empty filter입니다.
		const TArray<FCFVehicleFieldPath> AllTraceFields;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(Request, OutResolve)
			|| !FCFVehicleAuthoringService::ReadPendingDiff(Request, OutDiff)
			|| !FCFVehicleAuthoringService::ReadSourceTrace(Request, AllTraceFields, OutTrace)
			|| !FCFVehicleAuthoringService::ReadValidation(Request, OutValidation))
		{
			OutError = !OutResolve.Operation.Message.IsEmpty()
				? OutResolve.Operation.Message
				: (!OutDiff.Operation.Message.IsEmpty() ? OutDiff.Operation.Message : OutValidation.Operation.Message);
			return false;
		}
		OutError.Reset();
		return true;
	}

		// Created Initial Import Recipe를 Asset Registry에서 제거하고 garbage 대상으로 돌려 test side effect를 정리합니다.
	void CleanupImportedRecipe(UCFVehicleRecipeData* Recipe)
	{
		if (!Recipe)
		{
			return;
		}
		FAssetRegistryModule::AssetDeleted(Recipe);
		Recipe->ClearFlags(RF_Public | RF_Standalone);
		Recipe->MarkAsGarbage();
	}

	// Automation에서 Asset Registry에 등록한 임시 asset을 제거하고 garbage 대상으로 돌립니다.
	void CleanupRegisteredAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return;
		}
		FAssetRegistryModule::AssetDeleted(Asset);
		Asset->ClearFlags(RF_Public | RF_Standalone);
		Asset->MarkAsGarbage();
	}

	// Step 1 integration fixture에 Builder-private exact OwnerRecipeId Profile 4종을 연결합니다.
	bool AttachBuilderPrivateProfiles(FWorkspaceFixture& InOutFixture, FString& OutError)
	{
		if (!InOutFixture.Recipe || !InOutFixture.RecipePackage || !InOutFixture.TargetVehicleData)
		{
			OutError = TEXT("Builder Step 1 private Profile fixture prerequisite가 없습니다.");
			return false;
		}

		// VehicleBase private Profile fixture입니다.
		UCFVehicleBaseProfile* VehicleBase = NewObject<UCFVehicleBaseProfile>(
			InOutFixture.RecipePackage, TEXT("DA_BuilderStep1_Base"), RF_Transactional);
		// Drivetrain private Profile fixture입니다.
		UCFDrivetrainProfile* Drivetrain = NewObject<UCFDrivetrainProfile>(
			InOutFixture.RecipePackage, TEXT("DA_BuilderStep1_Drive"), RF_Transactional);
		// Handling private Profile fixture입니다.
		UCFHandlingProfile* Handling = NewObject<UCFHandlingProfile>(
			InOutFixture.RecipePackage, TEXT("DA_BuilderStep1_Handle"), RF_Transactional);
		// Performance private Profile fixture입니다.
		UCFPerformanceProfile* Performance = NewObject<UCFPerformanceProfile>(
			InOutFixture.RecipePackage, TEXT("DA_BuilderStep1_Perf"), RF_Transactional);
		if (!VehicleBase || !Drivetrain || !Handling || !Performance)
		{
			OutError = TEXT("Builder Step 1 private Profile fixture를 만들 수 없습니다.");
			return false;
		}

		VehicleBase->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		Drivetrain->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		Handling->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		Performance->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;

		// Existing imported baseline과 호환되는 VehicleBase seed입니다.
		VehicleBase->Data.BaseVehicleMassKg = InOutFixture.TargetVehicleData->BaseVehicleMassKg;
		VehicleBase->Data.MaximumGrossMassKg = InOutFixture.TargetVehicleData->MaximumGrossMassKg;
		VehicleBase->Data.MaxHealth = InOutFixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth;
		VehicleBase->Data.ChassisWidth = InOutFixture.TargetVehicleData->VehicleMovementConfig.ChassisWidth;
		VehicleBase->Data.ChassisHeight = InOutFixture.TargetVehicleData->VehicleMovementConfig.ChassisHeight;
		VehicleBase->Data.ExpectedWheelCount = 4;
		VehicleBase->Data.FrontWheelCountForSteering = 2;
		VehicleBase->Data.WheelMeshScaleClampMin = 0.25f;
		VehicleBase->Data.WheelMeshScaleClampMax = 4.0f;
		// Existing required wheel class references와 동일한 Drivetrain seed입니다.
		Drivetrain->Data.FrontWheelClass = InOutFixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass;
		Drivetrain->Data.RearWheelClass = InOutFixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass;

		InOutFixture.Recipe->ProfileBindings.VehicleBaseProfile = VehicleBase;
		InOutFixture.Recipe->ProfileBindings.DrivetrainProfile = Drivetrain;
		InOutFixture.Recipe->ProfileBindings.HandlingProfile = Handling;
		InOutFixture.Recipe->ProfileBindings.PerformanceProfile = Performance;
		OutError.Reset();
		return true;
	}

	// Step 1 integration Automation에 사용할 최소 valid canonical FACT Reference Research payload를 만듭니다.
	FCFVehicleRefEvidencePayload BuildBuilderStep1EvidencePayload()
	{
		// AI Research Draft에 넣을 normalized Evidence payload입니다.
		FCFVehicleRefEvidencePayload Payload;

		// Exact Primary Reference identity입니다.
		FCFRefVehicleIdentity& Reference = Payload.ReferenceVehicles.AddDefaulted_GetRef();
		Reference.ReferenceVehicleId = TEXT("REF-BUILDER-STEP1");
		Reference.Role = ECFRefVehicleRole::Primary;
		Reference.Manufacturer = TEXT("CarFight");
		Reference.Model = TEXT("Automation Reference");
		Reference.ModelYearStart = 2026;
		Reference.ModelYearEnd = 2026;
		Reference.ModelYearQualifier = ECFRefModelYearQualifier::Exact;
		Reference.Trim = TEXT("Integration");
		Reference.Powertrain = TEXT("Test");
		Reference.Transmission = TEXT("Test");
		Reference.MarketRegion = TEXT("TEST");
		Reference.IdentityConfidence = 0.95f;

		// Canonical FACT citation입니다.
		FCFRefSourceCitation& Source = Payload.Sources.AddDefaulted_GetRef();
		Source.SourceId = TEXT("SRC-BUILDER-STEP1");
		Source.Tier = ECFRefSourceTier::TierA;
		Source.SourceKind = TEXT("AutomationFixture");
		Source.Publisher = TEXT("CarFight");
		Source.DocumentTitle = TEXT("Builder Step 1 Integration Evidence");
		Source.CanonicalUrl = TEXT("https://example.invalid/carfight/builder-step1");
		Source.ReferenceVehicleIds = {Reference.ReferenceVehicleId};
		Source.OriginGroupId = TEXT("ORG-BUILDER-STEP1");
		Source.OriginIndependence = ECFRefOriginIndependence::IndependentOrigin;

		// Canonical curb-mass FACT claim입니다.
		FCFRefClaim& Claim = Payload.Claims.AddDefaulted_GetRef();
		Claim.ClaimId = TEXT("CLAIM-BUILDER-STEP1-MASS");
		Claim.ReferenceVehicleId = Reference.ReferenceVehicleId;
		Claim.FactKey = TEXT("CurbMassKg");
		Claim.ValueKind = ECFRefValueKind::Number;
		Claim.NumberValue = 1540.0;
		Claim.UnitId = TEXT("kg");
		Claim.SourceValueText = TEXT("1540 kg");
		Claim.Provenance = ECFRefProvenance::FACT;
		Claim.CitationIds = {Source.SourceId};
		Claim.ConfidenceScore = 0.95f;
		Claim.ResolutionState = ECFRefClaimResolution::Canonical;

		Payload.ResearchNotes = TEXT("VB-P0-09 Step 1 integration Automation only.");
		return Payload;
	}

	/** USER의 실제 ResearchDraft.json을 보존하면서 Automation용 JSON을 잠시 배치하는 RAII guard입니다. */
	struct FBuilderResearchDraftFileGuard
	{
		// Builder VM이 읽는 canonical transient Draft path입니다.
		FString DraftPath;

		// Test 시작 전에 파일이 존재했는지 여부입니다.
		bool bHadExistingFile = false;

		// Test 시작 전 원문입니다.
		FString PreviousContents;

		// Canonical Draft path와 기존 파일 상태를 캡처합니다.
		explicit FBuilderResearchDraftFileGuard(const FString& InDraftPath)
			: DraftPath(InDraftPath)
		{
			bHadExistingFile = FFileHelper::LoadFileToString(PreviousContents, *DraftPath);
		}

		// Automation 종료 시 기존 Draft를 exact 복원하거나 test-only 파일을 제거합니다.
		~FBuilderResearchDraftFileGuard()
		{
			if (bHadExistingFile)
			{
				FFileHelper::SaveStringToFile(
					PreviousContents,
					*DraftPath,
					FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
			}
			else
			{
				IFileManager::Get().Delete(*DraftPath, false, true, true);
			}
		}

		// Typed Research Draft를 UTF-8 JSON으로 canonical Saved path에 씁니다.
		bool WriteDraft(const FCFBuilderResearchDraft& Draft, FString& OutError) const
		{
			// Draft parent directory입니다.
			const FString ParentDirectory = FPaths::GetPath(DraftPath);
			if (!IFileManager::Get().MakeDirectory(*ParentDirectory, true))
			{
				OutError = FString::Printf(TEXT("Research Draft test directory를 만들 수 없습니다: %s"), *ParentDirectory);
				return false;
			}

			// USTRUCT를 serialize한 JSON 문자열입니다.
			FString JsonText;
			if (!FJsonObjectConverter::UStructToJsonObjectString(Draft, JsonText))
			{
				OutError = TEXT("FCFBuilderResearchDraft를 JSON으로 serialize할 수 없습니다.");
				return false;
			}
			if (!FFileHelper::SaveStringToFile(
				JsonText,
				*DraftPath,
				FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				OutError = FString::Printf(TEXT("Automation Research Draft를 쓸 수 없습니다: %s"), *DraftPath);
				return false;
			}
			OutError.Reset();
			return true;
		}
	};



	/** USER의 실제 PhysicsDraft.json을 보존하면서 Automation용 Step 5 JSON을 잠시 배치하는 RAII guard입니다. */
	struct FBuilderPhysicsDraftFileGuard
	{
		// Builder VM이 읽는 canonical transient Physics Draft path입니다.
		FString DraftPath;

		// Test 시작 전에 파일이 존재했는지 여부입니다.
		bool bHadExistingFile = false;

		// Test 시작 전 원문입니다.
		FString PreviousContents;

		// Canonical Physics Draft path와 기존 파일 상태를 캡처합니다.
		explicit FBuilderPhysicsDraftFileGuard(const FString& InDraftPath)
			: DraftPath(InDraftPath)
		{
			bHadExistingFile = FFileHelper::LoadFileToString(PreviousContents, *DraftPath);
		}

		// Automation 종료 시 기존 Physics Draft를 exact 복원하거나 test-only 파일을 제거합니다.
		~FBuilderPhysicsDraftFileGuard()
		{
			if (bHadExistingFile)
			{
				FFileHelper::SaveStringToFile(
					PreviousContents,
					*DraftPath,
					FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
			}
			else
			{
				IFileManager::Get().Delete(*DraftPath, false, true, true);
			}
		}

		// Typed Physics Draft를 UTF-8 JSON으로 canonical Saved path에 씁니다.
		bool WriteDraft(const FCFBuilderPhysicsDraft& Draft, FString& OutError) const
		{
			// Draft parent directory입니다.
			const FString ParentDirectory = FPaths::GetPath(DraftPath);
			if (!IFileManager::Get().MakeDirectory(*ParentDirectory, true))
			{
				OutError = FString::Printf(TEXT("Physics Draft test directory를 만들 수 없습니다: %s"), *ParentDirectory);
				return false;
			}

			// USTRUCT를 serialize한 JSON 문자열입니다.
			FString JsonText;
			if (!FJsonObjectConverter::UStructToJsonObjectString(Draft, JsonText))
			{
				OutError = TEXT("FCFBuilderPhysicsDraft를 JSON으로 serialize할 수 없습니다.");
				return false;
			}
			if (!FFileHelper::SaveStringToFile(
				JsonText,
				*DraftPath,
				FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				OutError = FString::Printf(TEXT("Automation Physics Draft를 쓸 수 없습니다: %s"), *DraftPath);
				return false;
			}
			OutError.Reset();
			return true;
		}
	};

	/** Random fixture Recipe의 local Reference review token section을 Automation 종료 시 제거하는 RAII guard입니다. */
	struct FBuilderReferenceTokenGuard
	{
		// 정리할 Recipe-specific EditorPerProject config section입니다.
		FString Section;

		// Random fixture RecipeId의 local section을 고정합니다.
		explicit FBuilderReferenceTokenGuard(const FGuid& RecipeId)
			: Section(FString::Printf(
				TEXT("CarFight.VehicleBuilder.ReferenceReview.%s"),
				*RecipeId.ToString(EGuidFormats::Digits)))
		{
		}

		// Test-only local token을 제거합니다.
		~FBuilderReferenceTokenGuard()
		{
			if (GConfig)
			{
				GConfig->EmptySection(*Section, GEditorPerProjectIni);
				GConfig->Flush(false, GEditorPerProjectIni);
			}
		}
	};


	/** USER의 실제 VehicleBuilderBenchmarkResult.json을 보존하면서 Step 8 synthetic result를 잠시 배치하는 RAII guard입니다. */
	struct FBuilderBenchmarkResultFileGuard
	{
		// Builder VM이 읽는 canonical benchmark result path입니다.
		FString ResultPath;

		// Test 시작 전에 result file이 존재했는지 여부입니다.
		bool bHadExistingFile = false;

		// Test 시작 전 result JSON 원문입니다.
		FString PreviousContents;

		// Canonical result path와 기존 파일 상태를 캡처합니다.
		explicit FBuilderBenchmarkResultFileGuard(const FString& InResultPath)
			: ResultPath(InResultPath)
		{
			bHadExistingFile = FFileHelper::LoadFileToString(PreviousContents, *ResultPath);
		}

		// Automation 종료 시 기존 benchmark result를 exact 복원하거나 test-only 파일을 제거합니다.
		~FBuilderBenchmarkResultFileGuard()
		{
			if (bHadExistingFile)
			{
				FFileHelper::SaveStringToFile(
					PreviousContents,
					*ResultPath,
					FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
			}
			else
			{
				IFileManager::Get().Delete(*ResultPath, false, true, true);
			}
		}

		// Step 8 parser가 소비할 synthetic current/stale benchmark envelope를 UTF-8 JSON으로 기록합니다.
		bool WriteResult(
			const FString& VehicleDataPath,
			const FString& TargetDefinitionHash,
			const FString& RunId,
			const bool bReferenceThresholdAsserted,
			const bool bUserDrivingFeelAsserted,
			FString& OutError) const
		{
			// Result parent directory입니다.
			const FString ParentDirectory = FPaths::GetPath(ResultPath);
			if (!IFileManager::Get().MakeDirectory(*ParentDirectory, true))
			{
				OutError = FString::Printf(TEXT("Step 8 benchmark result test directory를 만들 수 없습니다: %s"), *ParentDirectory);
				return false;
			}

			// JSON boolean literal입니다.
			const TCHAR* ReferenceThresholdText = bReferenceThresholdAsserted ? TEXT("true") : TEXT("false");
			// JSON boolean literal입니다.
			const TCHAR* UserDrivingFeelText = bUserDrivingFeelAsserted ? TEXT("true") : TEXT("false");
			// Existing VB-P0-08 metric schema와 Guided additive provenance를 모두 포함한 synthetic JSON입니다.
			const FString JsonText = FString::Printf(
				TEXT(
					"{\n"
					"  \"schema_version\": \"carfight_vehicle_builder_benchmark_v1\",\n"
					"  \"status\": \"success\",\n"
					"  \"run_id\": \"%s\",\n"
					"  \"expected_target_definition_hash\": \"%s\",\n"
					"  \"completed_utc\": \"2026-08-27T12:00:00.0000000Z\",\n"
					"  \"test_filter\": \"CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TechnicalDrivingBenchmark\",\n"
					"  \"engine_exit_code\": 0,\n"
					"  \"success_marker_count\": 1,\n"
					"  \"failure_marker_count\": 0,\n"
					"  \"metric_count\": 1,\n"
					"  \"reference_threshold_asserted\": %s,\n"
					"  \"user_driving_feel_asserted\": %s,\n"
					"  \"metric\": {\n"
					"    \"label\": \"Step8Synthetic\",\n"
					"    \"vehicle_data_path\": \"%s\",\n"
					"    \"fitting_data_path\": \"\",\n"
					"    \"configured_mass_kg\": 1570.0,\n"
					"    \"actual_mass_kg\": 1570.0,\n"
					"    \"acceleration_0_to_50_sec\": 3.716667,\n"
					"    \"acceleration_0_to_100_sec\": -1.0,\n"
					"    \"peak_speed_kmh\": 89.953712,\n"
					"    \"top_speed_stable\": false,\n"
					"    \"peak_engine_rpm\": 5100.0,\n"
					"    \"peak_speed_gear\": 4,\n"
					"    \"braking_100_available\": false,\n"
					"    \"braking_start_kmh\": 0.0,\n"
					"    \"braking_100_to_idle_sec\": -1.0,\n"
					"    \"braking_100_to_idle_distance_m\": -1.0,\n"
					"    \"steady_yaw_deg\": 17.858885,\n"
					"    \"effective_turning_radius_m\": 8.799048,\n"
					"    \"turning_average_speed_kmh\": 29.5\n"
					"  },\n"
					"  \"log_path\": \"Step8Synthetic\"\n"
					"}\n"),
				*RunId,
				*TargetDefinitionHash,
				ReferenceThresholdText,
				UserDrivingFeelText,
				*VehicleDataPath);

			if (!FFileHelper::SaveStringToFile(
				JsonText,
				*ResultPath,
				FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				OutError = FString::Printf(TEXT("Step 8 synthetic benchmark result를 쓸 수 없습니다: %s"), *ResultPath);
				return false;
			}

			OutError.Reset();
			return true;
		}
	};

	/** Random fixture Recipe의 USER Driving acceptance config section을 test 종료 시 제거하는 RAII guard입니다. */
	struct FBuilderDrivingTokenGuard
	{
		// 정리할 Recipe-specific EditorPerProject config section입니다.
		FString Section;

		// Random fixture RecipeId의 Driving acceptance section을 고정합니다.
		explicit FBuilderDrivingTokenGuard(const FGuid& RecipeId)
			: Section(FString::Printf(
				TEXT("CarFight.VehicleBuilder.DrivingAcceptance.%s"),
				*RecipeId.ToString(EGuidFormats::Digits)))
		{
		}

		// Test-only local Driving token을 제거합니다.
		~FBuilderDrivingTokenGuard()
		{
			if (GConfig)
			{
				GConfig->EmptySection(*Section, GEditorPerProjectIni);
				GConfig->Flush(false, GEditorPerProjectIni);
			}
		}

		// Production AcceptCurrentUserDriving을 우회하려는 경로가 아니라 restart-resume binding만 검증할 test fixture token을 기록합니다.
		bool WriteResumeToken(
			const FGuid& RecipeId,
			const FString& TargetDefinitionHash,
			const FString& BenchmarkRunId,
			FString& OutError) const
		{
			if (!GConfig || !RecipeId.IsValid() || TargetDefinitionHash.IsEmpty() || BenchmarkRunId.IsEmpty())
			{
				OutError = TEXT("Step 8 Driving resume token test prerequisite가 없습니다.");
				return false;
			}

			// Stable RecipeId 문자열입니다.
			const FString RecipeIdText = RecipeId.ToString(EGuidFormats::Digits);
			GConfig->SetString(*Section, TEXT("RecipeId"), *RecipeIdText, GEditorPerProjectIni);
			GConfig->SetString(*Section, TEXT("TargetDefinitionHash"), *TargetDefinitionHash, GEditorPerProjectIni);
			GConfig->SetString(*Section, TEXT("BenchmarkRunId"), *BenchmarkRunId, GEditorPerProjectIni);
			GConfig->Flush(false, GEditorPerProjectIni);
			OutError.Reset();
			return true;
		}
	};

	// Step 8 focused test가 이전 Step test를 replay하지 않고 final-applied prerequisite만 최소 구성합니다.
	bool PrepareBuilderStep8AppliedFixture(
		FWorkspaceFixture& Fixture,
		FCFVehicleBuilderVM& BuilderViewModel,
		UCFVehicleRefEvidence*& OutEvidence,
		FString& OutError)
	{
		OutEvidence = nullptr;

		// Final Review에 실제 Target diff가 생기도록 imported MaxHealth legacy pin만 release합니다.
		Fixture.Recipe->ImportState.LegacyPinnedFields.RemoveAll([](const FCFVehicleFieldOverride& Override)
		{
			return Override.FieldPath.ToCanonicalString(true) == TEXT("VehicleDurabilityConfig.MaxHealth");
		});
		Fixture.Recipe->DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::ExplicitValue;
		Fixture.Recipe->DurabilityIntent.ExplicitMaxHealth = Fixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth + 25.0f;
		++Fixture.Recipe->AuthoringRevision;

		// Guided Builder selection row입니다.
		const FCFVehicleListEntry Entry = BuildListEntry(Fixture, true);
		if (!BuilderViewModel.SelectVehicle(Entry, OutError))
		{
			return false;
		}

		// USER의 실제 ResearchDraft.json을 보존하는 test-only file guard입니다.
		FBuilderResearchDraftFileGuard ResearchDraftGuard(BuilderViewModel.GetResearchDraftPath());
		// Minimal valid Reference Evidence prerequisite draft입니다.
		FCFBuilderResearchDraft ResearchDraft;
		ResearchDraft.SchemaRevision = 1;
		ResearchDraft.RecipeId = Fixture.Recipe->RecipeId;
		ResearchDraft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
		ResearchDraft.EvidencePayload = BuildBuilderStep1EvidencePayload();
		ResearchDraft.ResearchLabel = TEXT("VB-P0-09 Step 8 prerequisite");
		if (!ResearchDraftGuard.WriteDraft(ResearchDraft, OutError)
			|| !BuilderViewModel.LoadResearchDraft(OutError))
		{
			return false;
		}

		// Missing Evidence만 만드는 prerequisite Companion preview입니다.
		FCFBuilderCompanionPreview CompanionPreview;
		if (!BuilderViewModel.PrepareResearchCompanions(CompanionPreview, OutError))
		{
			return false;
		}

		// Persistent Evidence prerequisite 결과입니다.
		FCFBuilderCompanionResult CompanionResult;
		if (!BuilderViewModel.ExecutePreparedResearchCompanions(CompanionResult, OutError)
			|| !CompanionResult.CreatedEvidence.Get())
		{
			return false;
		}
		OutEvidence = CompanionResult.CreatedEvidence.Get();
		if (!BuilderViewModel.AcceptCurrentReferenceSet(OutError))
		{
			return false;
		}

		// Current Builder-private VehicleBase Profile입니다.
		UCFVehicleBaseProfile* VehicleBaseProfile = Fixture.Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
		// Current Builder-private Drivetrain Profile입니다.
		UCFDrivetrainProfile* DrivetrainProfile = Fixture.Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
		// Current Builder-private Handling Profile입니다.
		UCFHandlingProfile* HandlingProfile = Fixture.Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
		// Current Builder-private Performance Profile입니다.
		UCFPerformanceProfile* PerformanceProfile = Fixture.Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
		if (!VehicleBaseProfile || !DrivetrainProfile || !HandlingProfile || !PerformanceProfile)
		{
			OutError = TEXT("Step 8 private Profile prerequisite를 load할 수 없습니다.");
			return false;
		}

		// USER의 실제 PhysicsDraft.json을 보존하는 test-only file guard입니다.
		FBuilderPhysicsDraftFileGuard PhysicsDraftGuard(BuilderViewModel.GetPhysicsProposalDraftPath());
		// Persistent Builder receipt를 생성할 typed prerequisite draft입니다.
		FCFBuilderPhysicsDraft PhysicsDraft;
		PhysicsDraft.SchemaRevision = 1;
		PhysicsDraft.RecipeId = Fixture.Recipe->RecipeId;
		PhysicsDraft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
		PhysicsDraft.EvidenceId = OutEvidence->EvidenceId;
		PhysicsDraft.ExpectedEvidenceFingerprint = OutEvidence->EvidenceFingerprint;
		PhysicsDraft.ProfilePayload.VehicleBaseData = VehicleBaseProfile->Data;
		PhysicsDraft.ProfilePayload.DrivetrainData = DrivetrainProfile->Data;
		PhysicsDraft.ProfilePayload.HandlingData = HandlingProfile->Data;
		PhysicsDraft.ProfilePayload.PerformanceData = PerformanceProfile->Data;
		PhysicsDraft.ConsumedClaimIds = {TEXT("CLAIM-BUILDER-STEP1-MASS")};
		PhysicsDraft.ProposalCorrelationHash = TEXT("3333333333333333333333333333333333333333333333333333333333333333");
		PhysicsDraft.ProposalLabel = TEXT("Step 8 prerequisite");
		PhysicsDraft.UserFacingSummary = TEXT("Step 8 focused prerequisite receipt");
		if (!PhysicsDraftGuard.WriteDraft(PhysicsDraft, OutError)
			|| !BuilderViewModel.LoadPhysicsProposalDraft(OutError))
		{
			return false;
		}

		// Existing private Profile typed preview입니다.
		FCFBuilderProfileCommitPreview PhysicsPreview;
		if (!BuilderViewModel.PreparePhysicsProposal(PhysicsPreview, OutError))
		{
			return false;
		}

		// Persistent receipt prerequisite commit입니다.
		FCFAuthoringOpResult PhysicsCommitResult;
		if (!BuilderViewModel.ExecutePreparedPhysicsProposal(PhysicsCommitResult, OutError))
		{
			return false;
		}

		// Final Review exact Apply proposal입니다.
		FCFBuilderFinalReviewResult FinalReview;
		if (!BuilderViewModel.PrepareFinalReviewApply(FinalReview, OutError))
		{
			return false;
		}

		// Final-applied Step 8 prerequisite terminal result입니다.
		FCFBuilderFinalApplyResult FinalApplyResult;
		if (!BuilderViewModel.ExecutePreparedFinalReviewApply(FinalApplyResult, OutError))
		{
			return false;
		}

		OutError.Reset();
		return true;
	}

	// External Drift review에서 exact canonical Stable Field Path row를 찾습니다.
	const FCFVehicleDriftReviewRow* FindDriftRow(
		const FCFVehicleDriftReviewResult& Review,
		const FString& CanonicalPath)
	{
		return Review.Rows.FindByPredicate([&CanonicalPath](const FCFVehicleDriftReviewRow& Row)
		{
			return Row.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleWorkspaceParityTest,
	"CarFight.DataAuthoring.DAUTH_P0_09.Workspace.ViewModelCoreParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleWorkspacePreviewTest,
	"CarFight.DataAuthoring.DAUTH_P0_09.Workspace.PreviewMutationZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleWorkspaceImportTest,
	"CarFight.DataAuthoring.DAUTH_P0_09.Workspace.InitialImport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleWorkspaceUndoTest,
	"CarFight.DataAuthoring.DAUTH_P0_09.Workspace.ApplyUndo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleWorkspaceStaleTest,
	"CarFight.DataAuthoring.DAUTH_P0_09.Workspace.StaleApprovalBlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleWorkspaceTabTest,
	"CarFight.DataAuthoring.DAUTH_P0_09.Workspace.TabRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderShellTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderStep1FlowTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep1Reference",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderStep5PhysicsTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep5Physics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderStep7ReviewTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderStep8DrivingTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


// ViewModel의 Resolve/Diff/Trace/Validation presentation state가 같은 facade direct read 결과와 exact parity인지 검증합니다.
bool FCFVehicleWorkspaceParityTest::RunTest(const FString& Parameters)
{
	// Managed Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/read diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Workspace parity fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*Fixture.Recipe);
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// 실제 Slate가 사용하는 transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Workspace ViewModel selects managed Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// 같은 Recipe/Target을 직접 facade로 읽은 Resolve result입니다.
	FCFVehicleResolveReadResult DirectResolve;
	// 같은 Recipe/Target을 직접 facade로 읽은 Diff result입니다.
	FCFVehicleDiffReadResult DirectDiff;
	// 같은 Recipe/Target을 직접 facade로 읽은 Trace result입니다.
	FCFVehicleTraceReadResult DirectTrace;
	// 같은 Recipe/Target을 직접 facade로 읽은 Validation result입니다.
	FCFVehicleValidationReadResult DirectValidation;
	if (!TestTrue(
		TEXT("Direct facade read set succeeds"),
		CFVehicleAuthoringVMTestsPrivate::BuildFacadeReadSet(Fixture, DirectResolve, DirectDiff, DirectTrace, DirectValidation, Error)))
	{
		AddError(Error);
		return false;
	}

	TestTrue(TEXT("ViewModel preview is Fresh"), ViewModel.IsPreviewFresh());
	TestEqual(TEXT("Resolve SourceSignature parity"), ViewModel.GetResolveResult().ResolveResult.SourceSignature, DirectResolve.ResolveResult.SourceSignature);
	TestEqual(TEXT("Resolve ResolvedDefinitionHash parity"), ViewModel.GetResolveResult().ResolveResult.ResolvedDefinitionHash, DirectResolve.ResolveResult.ResolvedDefinitionHash);
	TestEqual(TEXT("Resolve Current Definition hash parity"), ViewModel.GetResolveResult().ResolveRequest.CurrentDefinition.DefinitionHash, DirectResolve.ResolveRequest.CurrentDefinition.DefinitionHash);
	TestEqual(TEXT("Diff hash parity"), ViewModel.GetDiffResult().DiffHash, DirectDiff.DiffHash);
	TestEqual(TEXT("Diff count parity"), ViewModel.GetPendingDiffCount(), DirectDiff.FieldDiff.Num());
	TestEqual(TEXT("Trace count parity"), ViewModel.GetTraceResult().SourceTrace.Num(), DirectTrace.SourceTrace.Num());
	TestEqual(TEXT("Validation warning parity"), ViewModel.GetValidationResult().Operation.ValidationSummary.WarningCount, DirectValidation.Operation.ValidationSummary.WarningCount);
	TestEqual(TEXT("Validation blocked parity"), ViewModel.GetValidationResult().Operation.ValidationSummary.BlockedCount, DirectValidation.Operation.ValidationSummary.BlockedCount);
	TestEqual(TEXT("Validation error parity"), ViewModel.GetValidationResult().Operation.ValidationSummary.ErrorCount, DirectValidation.Operation.ValidationSummary.ErrorCount);
	TestEqual(TEXT("Validation ApplyBlocking parity"), ViewModel.GetValidationResult().Operation.ValidationSummary.bApplyBlocking, DirectValidation.Operation.ValidationSummary.bApplyBlocking);

	// Direct facade result만으로 계산한 Frozen normal Apply condition입니다.
	const bool bDirectCanApply = DirectResolve.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success
		&& !DirectDiff.FieldDiff.IsEmpty()
		&& !DirectValidation.Operation.ValidationSummary.bApplyBlocking
		&& !DirectResolve.ResolveResult.StaleReport.bHasExternalDrift;
	TestEqual(TEXT("ViewModel CanApply is derived from facade truth only"), ViewModel.CanApply(), bDirectCanApply);
	TestFalse(TEXT("Read-only ViewModel parity does not dirty Target package"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Read-only ViewModel parity does not dirty Recipe package"), Fixture.RecipePackage->IsDirty());
	return true;
}

// Typed Recipe prospective preview가 Recipe/Target/package를 전혀 mutation하지 않는지 검증합니다.
bool FCFVehicleWorkspacePreviewTest::RunTest(const FString& Parameters)
{
	// Managed Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Workspace preview fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Preview 전 Target full Definition hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Preview 전 Recipe semantic fingerprint입니다.
	const FString RecipeFingerprintBefore = CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Preview 전 diagnostic Authoring revision입니다.
	const int32 RevisionBefore = Fixture.Recipe->AuthoringRevision;
	// Preview 전 actual Vehicle Archetype property입니다.
	const FName ArchetypeBefore = Fixture.Recipe->VehicleArchetypeId;

	// 실제 Slate가 사용하는 transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Workspace preview ViewModel selects Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Persistent Recipe와 다른 prospective Archetype intent입니다.
	const FName ProspectiveArchetype(TEXT("P0_09_PreviewArchetype"));
	// Facade prospective preview result입니다.
	FCFVehicleRecipePreviewResult PreviewResult;
	TestTrue(TEXT("Prospective Recipe preview succeeds"), ViewModel.PreviewArchetypeIntent(ProspectiveArchetype, PreviewResult));
	TestFalse(TEXT("Prospective preview proposal does not target-mutate"), PreviewResult.Proposal.bTargetMutation);
	TestFalse(TEXT("Prospective preview operation does not mark Target changed"), PreviewResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Prospective preview operation does not mark Recipe changed"), PreviewResult.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Prospective preview does not save"), PreviewResult.Operation.Mutation.bSavePerformed);
	TestEqual(TEXT("Target hash unchanged after prospective preview"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Recipe fingerprint unchanged after prospective preview"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Recipe AuthoringRevision unchanged after prospective preview"), Fixture.Recipe->AuthoringRevision, RevisionBefore);
	TestEqual(TEXT("Recipe VehicleArchetype property unchanged after prospective preview"), Fixture.Recipe->VehicleArchetypeId, ArchetypeBefore);
	TestFalse(TEXT("Prospective preview does not dirty Target package"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Prospective preview does not dirty Recipe package"), Fixture.RecipePackage->IsDirty());
	return true;
}

// Reviewed Initial Import가 preview mutation0을 지키고 commit 시 새 Recipe만 만들며 Target은 그대로 유지하는지 검증합니다.
bool FCFVehicleWorkspaceImportTest::RunTest(const FString& Parameters)
{
	// Unmanaged Target-only Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Workspace unmanaged target builds"), CFVehicleAuthoringVMTestsPrivate::ConfigureValidTarget(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Import 전 Target full Definition hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	Fixture.TargetPackage->SetDirtyFlag(false);

	// 실제 Slate가 사용하는 transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Recipe 없는 unmanaged selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, false);
	if (!TestTrue(TEXT("Workspace selects unmanaged Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	TestFalse(TEXT("Unmanaged selection has no Recipe"), ViewModel.HasRecipe());

	// Test-only persistent package folder입니다. file save는 수행하지 않습니다.
	const FString RecipeFolder(TEXT("/Game/CarFight/Tests/DataAuthoringP09"));
	// 다른 Automation run과 충돌하지 않는 reviewed Recipe asset name입니다.
	const FName RecipeName(*FString::Printf(TEXT("DA_P09Recipe_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
	// Mutation0 reviewed Initial Import proposal입니다.
	FCFVehicleInitialImportPreviewResult ImportPreview;
	if (!TestTrue(TEXT("Initial Import preview succeeds"), ViewModel.BuildInitialImportPreview(RecipeFolder, RecipeName, ImportPreview)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	TestEqual(TEXT("Initial Import preview risk is R2"), ImportPreview.Proposal.RiskClass, ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite);
	TestEqual(TEXT("Initial Import requires OwnershipWrite approval"), ImportPreview.Proposal.RequiredApprovalClass, ECFAuthoringApprovalClass::OwnershipWrite);
	TestFalse(TEXT("Initial Import preview never target-mutates"), ImportPreview.Proposal.bTargetMutation);
	TestFalse(TEXT("Initial Import preview never saves"), ImportPreview.Proposal.bSavePerformed);
	TestTrue(TEXT("Initial Import preview has Legacy Pins"), ImportPreview.ImportSummary.LegacyPinnedFieldCount > 0);
		TestEqual(TEXT("Fixture has no MountProfile hidden serialized payload"), ImportPreview.ImportSummary.LegacySerializedFieldCount, 0);
	TestEqual(TEXT("Target hash unchanged after Initial Import preview"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestFalse(TEXT("Initial Import preview does not dirty Target package"), Fixture.TargetPackage->IsDirty());

	// Exact reviewed R2 Initial Import commit result입니다.
	FCFVehicleInitialImportResult ImportResult;
	if (!TestTrue(TEXT("Reviewed Initial Import commit succeeds"), ViewModel.CommitInitialImport(RecipeFolder, RecipeName, ImportResult)))
	{
		AddError(ImportResult.Operation.Message);
		return false;
	}
	TestNotNull(TEXT("Initial Import creates Recipe only"), ImportResult.Recipe.Get());
	TestTrue(TEXT("ViewModel now owns managed Recipe selection"), ViewModel.HasRecipe());
	TestTrue(TEXT("Initial Import reports Recipe mutation"), ImportResult.Operation.Mutation.bRecipeChanged);
	TestTrue(TEXT("Initial Import reports created asset"), ImportResult.Operation.Mutation.bCreatedAssets);
	TestFalse(TEXT("Initial Import reports Target mutation zero"), ImportResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Initial Import never auto-saves"), ImportResult.Operation.Mutation.bSavePerformed);
		TestFalse(TEXT("Initial Import never automatic retries"), ImportResult.Operation.Mutation.bAutomaticRetryPerformed);
	TestEqual(TEXT("Committed Legacy Pin count matches reviewed proposal"), ImportResult.ImportSummary.LegacyPinnedFieldCount, ImportPreview.ImportSummary.LegacyPinnedFieldCount);
	TestEqual(TEXT("Committed hidden serialized count matches reviewed proposal"), ImportResult.ImportSummary.LegacySerializedFieldCount, ImportPreview.ImportSummary.LegacySerializedFieldCount);
	TestEqual(TEXT("Committed semantic candidate count matches reviewed proposal"), ImportResult.ImportSummary.SemanticCandidateFieldCount, ImportPreview.ImportSummary.SemanticCandidateFieldCount);
	TestEqual(TEXT("Target hash unchanged after Initial Import commit"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestFalse(TEXT("Target package remains clean after Initial Import"), Fixture.TargetPackage->IsDirty());
	if (ImportResult.Recipe)
	{
		TestTrue(TEXT("New Recipe package is dirty pending explicit save"), ImportResult.Recipe->GetOutermost()->IsDirty());
		TestEqual(TEXT("New Recipe binds exact Target"), ImportResult.Recipe->TargetVehicleData.Get(), Fixture.TargetVehicleData);
	}

	CFVehicleAuthoringVMTestsPrivate::CleanupImportedRecipe(ImportResult.Recipe.Get());
	return true;
}

// ViewModel Apply가 기존 ApplyService transaction을 사용하고 Unreal standard Undo가 Target/AppliedState를 원상 복원하는지 검증합니다.
bool FCFVehicleWorkspaceUndoTest::RunTest(const FString& Parameters)
{
	// Managed Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Workspace ApplyUndo fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*Fixture.Recipe);
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Apply 전 full Target Definition hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Apply 전 AppliedState recipe revision입니다.
	const int32 AppliedRevisionBefore = Fixture.Recipe->AppliedState.AppliedRecipeRevision;
	// Apply 전 AppliedState Recipe fingerprint입니다.
	const FString AppliedRecipeFingerprintBefore = Fixture.Recipe->AppliedState.AppliedRecipeFingerprint;
	// Apply 전 AppliedState Source signature입니다.
	const FString AppliedSourceSignatureBefore = Fixture.Recipe->AppliedState.AppliedSourceSignature;
	// Apply 전 AppliedState Definition hash입니다.
	const FString AppliedDefinitionHashBefore = Fixture.Recipe->AppliedState.AppliedDefinitionHash;
	// Apply 전 AppliedState resolver revision입니다.
	const int32 AppliedResolverRevisionBefore = Fixture.Recipe->AppliedState.ResolverContractRevision;
	// Apply 전 field trace count입니다.
	const int32 AppliedTraceCountBefore = Fixture.Recipe->AppliedState.FieldTraces.Num();

	// 실제 Slate가 사용하는 transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Workspace ApplyUndo ViewModel selects Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Workspace Apply is enabled from fresh facade truth"), ViewModel.CanApply());

	// Exact R3 preparation result입니다.
	FCFAuthoringOpResult PrepareResult;
	if (!TestTrue(TEXT("Workspace prepares exact R3 Apply approval"), ViewModel.PrepareApply(PrepareResult)))
	{
		AddError(PrepareResult.Message);
		return false;
	}
	TestTrue(TEXT("Workspace holds prepared Apply approval"), ViewModel.HasPreparedApply());

	// Shared facade/ApplyService terminal result입니다.
	FCFAuthoringOpResult ApplyResult;
	if (!TestTrue(TEXT("Workspace Apply succeeds through shared facade lane"), ViewModel.ExecutePreparedApply(ApplyResult)))
	{
		AddError(ApplyResult.Message);
		return false;
	}
	// Successful Apply Target hash입니다.
	const FString TargetHashAfterApply = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	TestNotEqual(TEXT("Successful Apply changes Target Definition"), TargetHashAfterApply, TargetHashBefore);
	TestTrue(TEXT("Successful Apply records AppliedState"), !Fixture.Recipe->AppliedState.AppliedDefinitionHash.IsEmpty());
		TestTrue(TEXT("Workspace Apply reports Target mutation"), ApplyResult.Mutation.bTargetChanged);
	TestTrue(TEXT("Workspace Apply reports Recipe AppliedState mutation"), ApplyResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Workspace Apply never auto-saves"), ApplyResult.Mutation.bSavePerformed);
	TestFalse(TEXT("Workspace Apply never automatic retries"), ApplyResult.Mutation.bAutomaticRetryPerformed);
	TestFalse(TEXT("Prepared approval is consumed after Apply"), ViewModel.HasPreparedApply());

	TestNotNull(TEXT("Editor transaction system is available"), GEditor);
		if (!GEditor || !GEditor->Trans)
	{
		return false;
	}
		// Apply 직후 UE가 보고하는 next Undo transaction identity입니다. Generic IsValid는 Automation에서 false일 수 있어 ownership authority로 사용하지 않습니다.
	const FTransactionContext ApplyUndoContext = GEditor->Trans->GetUndoContext(false);
	TestTrue(TEXT("Apply Undo context has a valid TransactionId"), ApplyUndoContext.TransactionId.IsValid());
	TestEqual(TEXT("Apply Undo context is CarFight Definition Apply"), ApplyUndoContext.Context, FString(TEXT("CarFight.VehicleAuthoring.DefinitionApply")));
	TestTrue(TEXT("Apply Undo context PrimaryObject is current Target"), ApplyUndoContext.PrimaryObject == Fixture.TargetVehicleData);
	TestTrue(TEXT("Workspace exposes exact top-bound Undo after Apply"), ViewModel.CanUndoLastWorkspaceAction());

	// Workspace Apply 뒤에 끼워 넣을 unrelated transactional UObject입니다.
	UCFVehicleRecipeData* InterveningRecipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional);
	TestNotNull(TEXT("Intervening transaction fixture exists"), InterveningRecipe);
	if (!InterveningRecipe)
	{
		return false;
	}
	// Intervening transaction 전 revision입니다.
	const int32 InterveningRevisionBefore = InterveningRecipe->AuthoringRevision;
	{
		// Workspace 소유가 아닌 별도 Editor transaction입니다.
		FScopedTransaction InterveningTransaction(NSLOCTEXT("CarFightDataAuthoringTests", "InterveningEditorTransaction", "DAUTH Test Intervening Editor Transaction"));
		InterveningRecipe->Modify();
		++InterveningRecipe->AuthoringRevision;
	}
	// Intervening transaction 후 revision입니다.
	const int32 InterveningRevisionAfter = InterveningRecipe->AuthoringRevision;
	TestNotEqual(TEXT("Intervening Editor transaction changes its own object"), InterveningRevisionAfter, InterveningRevisionBefore);
	TestFalse(TEXT("Workspace custom Undo disables when another Editor transaction is on top"), ViewModel.CanUndoLastWorkspaceAction());
	// Wrong-top Undo 시도 diagnostic입니다.
	FString WrongTopUndoError;
	TestFalse(TEXT("Workspace refuses to undo unrelated top transaction"), ViewModel.UndoLastWorkspaceAction(WrongTopUndoError));
	TestEqual(TEXT("Refused Workspace Undo leaves intervening transaction untouched"), InterveningRecipe->AuthoringRevision, InterveningRevisionAfter);
	TestEqual(TEXT("Refused Workspace Undo leaves applied Target untouched"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashAfterApply);

	// 테스트가 직접 unrelated top transaction만 표준 Undo해 Workspace transaction을 다시 top으로 복원합니다.
	const bool bInterveningUndoSucceeded = GEditor->UndoTransaction();
	TestTrue(TEXT("Standard Undo removes only intervening Editor transaction"), bInterveningUndoSucceeded);
	TestEqual(TEXT("Intervening transaction Undo restores its own object"), InterveningRecipe->AuthoringRevision, InterveningRevisionBefore);
	TestTrue(TEXT("Workspace custom Undo re-enables when exact tracked transaction returns to top"), ViewModel.CanUndoLastWorkspaceAction());

	// Exact tracked Workspace Apply transaction만 되돌리는 ViewModel standard Undo 결과입니다.
	FString WorkspaceUndoError;
	const bool bUndoSucceeded = ViewModel.UndoLastWorkspaceAction(WorkspaceUndoError);
	TestTrue(TEXT("Workspace exact TransactionId Undo succeeds for Apply transaction"), bUndoSucceeded);
	TestEqual(TEXT("Undo restores full Target Definition hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Undo restores AppliedState recipe revision"), Fixture.Recipe->AppliedState.AppliedRecipeRevision, AppliedRevisionBefore);
	TestEqual(TEXT("Undo restores AppliedState Recipe fingerprint"), Fixture.Recipe->AppliedState.AppliedRecipeFingerprint, AppliedRecipeFingerprintBefore);
	TestEqual(TEXT("Undo restores AppliedState Source signature"), Fixture.Recipe->AppliedState.AppliedSourceSignature, AppliedSourceSignatureBefore);
	TestEqual(TEXT("Undo restores AppliedState Definition hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, AppliedDefinitionHashBefore);
	TestEqual(TEXT("Undo restores AppliedState resolver revision"), Fixture.Recipe->AppliedState.ResolverContractRevision, AppliedResolverRevisionBefore);
	TestEqual(TEXT("Undo restores AppliedState trace count"), Fixture.Recipe->AppliedState.FieldTraces.Num(), AppliedTraceCountBefore);

		TestTrue(TEXT("Workspace Undo leaves a fresh preview"), ViewModel.IsPreviewFresh());
	TestTrue(TEXT("Undo returns pending Authoring Changes"), ViewModel.GetPendingDiffCount() > 0);
	TestFalse(TEXT("Consumed Workspace transaction is no longer custom-undoable"), ViewModel.CanUndoLastWorkspaceAction());
	return true;
}

// Fresh Editor module에서 새 Vehicle Authoring tab과 기존 Wizard tab이 함께 등록되고 새 Workspace가 실제 생성되는지 검증합니다.
bool FCFVehicleWorkspaceTabTest::RunTest(const FString& Parameters)
{
	// P0-09 신규 Vehicle Authoring Nomad Tab identity입니다.
	const FName VehicleAuthoringTabName(TEXT("CarFight.VehicleAuthoring"));
	// P0-10 parity 전까지 반드시 보존할 legacy Wizard tab identity입니다.
	const FName VehicleWizardTabName(TEXT("CarFight.VehicleDAWizard"));
	TestTrue(TEXT("Vehicle Authoring Nomad Tab spawner is registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleAuthoringTabName));
	TestTrue(TEXT("Legacy Vehicle DA Wizard spawner remains registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleWizardTabName));

	// 실제 module spawner를 통해 생성된 P0-09 Workspace tab입니다.
	TSharedPtr<SDockTab> VehicleAuthoringTab = FGlobalTabmanager::Get()->TryInvokeTab(VehicleAuthoringTabName);
	TestTrue(TEXT("Vehicle Authoring Workspace tab is actually created"), VehicleAuthoringTab.IsValid());
	if (VehicleAuthoringTab.IsValid())
	{
		TestTrue(TEXT("Vehicle Authoring Workspace has valid Slate content"), VehicleAuthoringTab->GetContent() != SNullWidget::NullWidget);
		VehicleAuthoringTab->RequestCloseTab();
	}
	return true;
}

// Guided Vehicle Builder의 별도 tab registration과 stable StepId 기반 current baseline contract를 검증합니다.
bool FCFVehicleBuilderShellTest::RunTest(const FString& Parameters)
{
	// 신규 Guided Builder Nomad Tab identity입니다.
	const FName VehicleBuilderTabName(TEXT("CarFight.VehicleBuilder"));
	// 계속 보존해야 하는 Advanced Workspace tab identity입니다.
	const FName VehicleAuthoringTabName(TEXT("CarFight.VehicleAuthoring"));
	// 계속 hidden fallback으로 보존해야 하는 legacy Wizard tab identity입니다.
	const FName VehicleWizardTabName(TEXT("CarFight.VehicleDAWizard"));
	TestTrue(TEXT("Guided Vehicle Builder tab spawner is registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleBuilderTabName));
	TestTrue(TEXT("Advanced Vehicle Authoring tab remains registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleAuthoringTabName));
	TestTrue(TEXT("Legacy Vehicle DA Wizard spawner remains registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleWizardTabName));

	// 실제 Guided Shell이 사용하는 transient ViewModel입니다.
	FCFVehicleBuilderVM BuilderViewModel;
	// Asset Registry read diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Guided Builder target browser fresh read succeeds"), BuilderViewModel.RefreshVehicles(Error)))
	{
		AddError(Error);
		return false;
	}

	const TArray<FCFVehicleBuilderStepView>& Steps = BuilderViewModel.GetStepViews();
	TestTrue(TEXT("Guided Builder contains current baseline Step definitions"), Steps.Num() >= 8);
	// Stable semantic identity로 찾은 current Reference Step입니다.
	const FCFVehicleBuilderStepView* IdentityStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	// Stable semantic identity로 찾은 current Mesh Step입니다.
	const FCFVehicleBuilderStepView* MeshPrepStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	TestNotNull(TEXT("IdentityReference Step exists by stable StepId"), IdentityStep);
	TestNotNull(TEXT("MeshPrep Step exists by stable StepId"), MeshPrepStep);
	// Stable semantic identity로 찾은 current Socket 준비 Step입니다.
	const FCFVehicleBuilderStepView* SocketGuideStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	TestNotNull(TEXT("SocketGuide Step exists by stable StepId"), SocketGuideStep);
	if (SocketGuideStep)
	{
		TestEqual(TEXT("SocketGuide USER-facing title is socket preparation"), SocketGuideStep->Title.ToString(), FString(TEXT("소켓 준비 / Naming")));
	}
	TestNotNull(TEXT("LayoutCapture Step exists by stable StepId"), BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture));
	TestNotNull(TEXT("PhysicsProposal Step exists by stable StepId"), BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::PhysicsProposal));
	TestNotNull(TEXT("GameplaySetup Step exists by stable StepId"), BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::GameplaySetup));
	TestNotNull(TEXT("FinalReview Step exists by stable StepId"), BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::FinalReview));
	TestNotNull(TEXT("DrivingTest Step exists by stable StepId"), BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::DrivingTest));
	if (IdentityStep && MeshPrepStep)
	{
		TestEqual(TEXT("No-selection IdentityReference is Ready"), IdentityStep->State, ECFVehicleBuilderStepState::Ready);
		TestEqual(TEXT("No-selection MeshPrep remains Locked"), MeshPrepStep->State, ECFVehicleBuilderStepState::Locked);
	}

	// 실제 module spawner를 통해 Guided Builder Slate tab을 생성합니다.
	TSharedPtr<SDockTab> VehicleBuilderTab = FGlobalTabmanager::Get()->TryInvokeTab(VehicleBuilderTabName);
	TestTrue(TEXT("Guided Vehicle Builder tab is actually created"), VehicleBuilderTab.IsValid());
	if (VehicleBuilderTab.IsValid())
	{
		TestTrue(TEXT("Guided Vehicle Builder has valid Slate content"), VehicleBuilderTab->GetContent() != SNullWidget::NullWidget);
		VehicleBuilderTab->RequestCloseTab();
	}

	// actual Wagon처럼 core 생성 뒤 WSA/Mesh Recipe-only revision만 진행된 NewVehicle bootstrap fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture NewVehicleBootstrapFixture;
	if (!TestTrue(TEXT("Builder NewVehicle bootstrap fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(NewVehicleBootstrapFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Existing-import fixture의 lifecycle hash를 제거해 CreateVehicleRecords 직후와 같은 NewVehicle lifecycle을 재현합니다.
	NewVehicleBootstrapFixture.Recipe->ImportState.ImportedDefinitionHash.Reset();
	NewVehicleBootstrapFixture.Recipe->AppliedState.AppliedDefinitionHash.Reset();
	NewVehicleBootstrapFixture.Recipe->ImportState.ManageState = ECFVehicleManageState::Managed;
	// WSA/Mesh 같은 Target 미적용 Recipe-only authoring이 여러 번 진행된 actual Wagon revision입니다.
	NewVehicleBootstrapFixture.Recipe->AuthoringRevision = 6;
	// actual Wagon처럼 Builder-private Profile이 하나도 없는지 먼저 고정합니다.
	TestTrue(TEXT("Builder NewVehicle bootstrap starts with VehicleBase Profile missing"), NewVehicleBootstrapFixture.Recipe->ProfileBindings.VehicleBaseProfile.IsNull());
	TestTrue(TEXT("Builder NewVehicle bootstrap starts with Drivetrain Profile missing"), NewVehicleBootstrapFixture.Recipe->ProfileBindings.DrivetrainProfile.IsNull());
	TestTrue(TEXT("Builder NewVehicle bootstrap starts with Handling Profile missing"), NewVehicleBootstrapFixture.Recipe->ProfileBindings.HandlingProfile.IsNull());
	TestTrue(TEXT("Builder NewVehicle bootstrap starts with Performance Profile missing"), NewVehicleBootstrapFixture.Recipe->ProfileBindings.PerformanceProfile.IsNull());
	// 실제 Guided Builder가 Wagon형 NewVehicle bootstrap 상태를 평가할 transient ViewModel입니다.
	FCFVehicleBuilderVM NewVehicleBootstrapViewModel;
	// Managed NewVehicle core selection row입니다.
	const FCFVehicleListEntry NewVehicleBootstrapEntry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(NewVehicleBootstrapFixture, true);
	if (!TestTrue(TEXT("Builder accepts Managed NewVehicle with Recipe-only revision before private Profile bootstrap"), NewVehicleBootstrapViewModel.SelectVehicle(NewVehicleBootstrapEntry, Error)))
	{
		AddError(Error);
		return false;
	}
	if (!TestTrue(TEXT("Builder refresh preserves NewVehicle private Profile bootstrap state"), NewVehicleBootstrapViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	// Profile bootstrap 전에는 Reference 단계가 정상 workflow 진입점으로 남아야 합니다.
	const FCFVehicleBuilderStepView* NewVehicleReferenceStep = NewVehicleBootstrapViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	if (TestNotNull(TEXT("Builder NewVehicle bootstrap Reference Step exists"), NewVehicleReferenceStep))
	{
		TestEqual(TEXT("Builder NewVehicle bootstrap Reference Step remains Ready"), NewVehicleReferenceStep->State, ECFVehicleBuilderStepState::Ready);
	}

	// VB-P0-03 read-only evaluator의 정상 managed Vehicle fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture BuilderFixture;
	if (!TestTrue(TEXT("Builder P0-03 managed fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(BuilderFixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// 실제 Guided Builder가 managed Vehicle을 평가할 transient ViewModel입니다.
	FCFVehicleBuilderVM ManagedBuilderViewModel;
	// Existing managed Recipe/Target selection row입니다.
	const FCFVehicleListEntry ManagedEntry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(BuilderFixture, true);
	if (!TestTrue(TEXT("Builder P0-03 managed Vehicle selects"), ManagedBuilderViewModel.SelectVehicle(ManagedEntry, Error)))
	{
		AddError(Error);
		return false;
	}

	// Baseline current AssetSnapshot에서 stable StepId로 찾은 evaluator projection입니다.
	const FCFVehicleBuilderStepView* BaselineMeshStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	// Baseline Socket evaluator projection입니다.
	const FCFVehicleBuilderStepView* BaselineSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	// Baseline Layout evaluator projection입니다.
	const FCFVehicleBuilderStepView* BaselineLayoutStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture);
	if (TestNotNull(TEXT("Builder P0-03 baseline MeshPrep exists"), BaselineMeshStep)
		&& TestNotNull(TEXT("Builder P0-03 baseline SocketGuide exists"), BaselineSocketStep)
		&& TestNotNull(TEXT("Builder P0-03 baseline LayoutCapture exists"), BaselineLayoutStep))
	{
		TestEqual(TEXT("Builder P0-03 MeshPrep baseline is Complete"), BaselineMeshStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 SocketGuide baseline is Complete"), BaselineSocketStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 LayoutCapture baseline is Complete"), BaselineLayoutStep->State, ECFVehicleBuilderStepState::Complete);
	}

	// Step 2 Recipe-only Mesh commit 전 Target VehicleData full hash입니다.
	const FString TargetHashBeforeMeshPreparationCommit = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*BuilderFixture.TargetVehicleData, Error);
	// Current baseline AssetIntent에서 required FL만 비운 Step 2 blocked 시나리오입니다.
	FCFVehicleAssetIntent MissingWheelFLIntent = BuilderFixture.Recipe->AssetIntent;
	MissingWheelFLIntent.WheelMeshFL = TSoftObjectPtr<UStaticMesh>();
	// Step 2 typed Recipe-only commit result입니다.
	FCFAuthoringOpResult MissingWheelFLCommitResult;
	if (!TestTrue(TEXT("Builder Step 2 typed commit accepts explicit missing FL Recipe state"), ManagedBuilderViewModel.CommitMeshPreparation(MissingWheelFLIntent, MissingWheelFLCommitResult, Error)))
	{
		AddError(Error);
		return false;
	}
	// Required FL missing 상태의 Step 2 evaluator projection입니다.
	const FCFVehicleBuilderStepView* MissingWheelFLStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	if (TestNotNull(TEXT("Builder Step 2 exists after missing FL commit"), MissingWheelFLStep))
	{
		TestEqual(TEXT("Builder Step 2 blocks when required WheelMeshFL is missing"), MissingWheelFLStep->State, ECFVehicleBuilderStepState::Blocked);
	}
	TestEqual(TEXT("Builder Step 2 Recipe-only commit leaves Target VehicleData hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*BuilderFixture.TargetVehicleData, Error), TargetHashBeforeMeshPreparationCommit);

	// Current Recipe에서 FL을 known valid Wheel Mesh로 복원하는 Guided Step 2 시나리오입니다.
	FCFVehicleAssetIntent RestoredWheelFLIntent = BuilderFixture.Recipe->AssetIntent;
	RestoredWheelFLIntent.WheelMeshFL = BuilderFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFL;
	// Step 2 restore typed Recipe-only commit result입니다.
	FCFAuthoringOpResult RestoredWheelFLCommitResult;
	if (!TestTrue(TEXT("Builder Step 2 typed commit restores required FL Wheel Mesh"), ManagedBuilderViewModel.CommitMeshPreparation(RestoredWheelFLIntent, RestoredWheelFLCommitResult, Error)))
	{
		AddError(Error);
		return false;
	}
	// Required FL 복원 뒤 Step 2 evaluator projection입니다.
	const FCFVehicleBuilderStepView* RestoredWheelFLStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	if (TestNotNull(TEXT("Builder Step 2 exists after FL restore"), RestoredWheelFLStep))
	{
		TestEqual(TEXT("Builder Step 2 returns Complete after valid FL Wheel Mesh restore"), RestoredWheelFLStep->State, ECFVehicleBuilderStepState::Complete);
	}
	TestEqual(TEXT("Builder Step 2 FL restore still leaves Target VehicleData hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*BuilderFixture.TargetVehicleData, Error), TargetHashBeforeMeshPreparationCommit);

	// Step 3 UI가 표시할 current effective Wheel Socket exact 이름 4종입니다.
	const TArray<FName> BaselineRequiredSocketNames = ManagedBuilderViewModel.GetRequiredWheelSocketNames();
	TestEqual(TEXT("Builder Step 3 exposes exactly four required Wheel Socket roles"), BaselineRequiredSocketNames.Num(), 4);
	if (BaselineRequiredSocketNames.Num() == 4)
	{
		TestEqual(TEXT("Builder Step 3 FL exact default Socket name"), BaselineRequiredSocketNames[0], FName(TEXT("Wheel_Anchor_FL")));
		TestEqual(TEXT("Builder Step 3 FR exact default Socket name"), BaselineRequiredSocketNames[1], FName(TEXT("Wheel_Anchor_FR")));
		TestEqual(TEXT("Builder Step 3 RL exact default Socket name"), BaselineRequiredSocketNames[2], FName(TEXT("Wheel_Anchor_RL")));
		TestEqual(TEXT("Builder Step 3 RR exact default Socket name"), BaselineRequiredSocketNames[3], FName(TEXT("Wheel_Anchor_RR")));
		for (const FName SocketName : BaselineRequiredSocketNames)
		{
			TestTrue(FString::Printf(TEXT("Builder Step 3 fresh Chassis contains required Socket %s"), *SocketName.ToString()), ManagedBuilderViewModel.IsCurrentChassisSocketFound(SocketName));
		}
	}

	// VB-P0-03 B 시나리오처럼 optional FR/RL/RR Wheel reference를 비워 current backend FL-reuse warning semantics를 검증합니다.
	BuilderFixture.Recipe->AssetIntent.WheelMeshFR = TSoftObjectPtr<UStaticMesh>();
	BuilderFixture.Recipe->AssetIntent.WheelMeshRL = TSoftObjectPtr<UStaticMesh>();
	BuilderFixture.Recipe->AssetIntent.WheelMeshRR = TSoftObjectPtr<UStaticMesh>();
	// Fresh Recipe snapshot을 요구하는 semantic revision입니다.
	++BuilderFixture.Recipe->AuthoringRevision;
	if (!TestTrue(TEXT("Builder P0-03 refresh with optional Wheel refs missing succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	// Optional Wheel 3종을 비운 뒤 stable StepId evaluator projection입니다.
	const FCFVehicleBuilderStepView* OptionalMeshStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	// Optional Wheel 상태의 Socket projection입니다.
	const FCFVehicleBuilderStepView* OptionalSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	// Optional Wheel 상태의 Layout projection입니다.
	const FCFVehicleBuilderStepView* OptionalLayoutStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture);
	if (OptionalMeshStep && OptionalSocketStep && OptionalLayoutStep)
	{
		TestEqual(TEXT("Builder P0-03 MeshPrep allows missing optional FR/RL/RR"), OptionalMeshStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 SocketGuide remains Complete with FL-only Wheel asset"), OptionalSocketStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 LayoutCapture remains Complete with FL-only Wheel asset"), OptionalLayoutStep->State, ECFVehicleBuilderStepState::Complete);
	}

	// WSA-P0-04 Socket mode 검증 전 원래 WheelVisual/FL binding과 Existing Import pin 상태를 보존합니다.
	const ECFWheelVisualIntentMode OriginalWheelVisualMode = BuilderFixture.Recipe->WheelVisualIntent.Mode;
	const TSoftObjectPtr<UStaticMesh> OriginalWheelMeshFL = BuilderFixture.Recipe->AssetIntent.WheelMeshFL;
	const TArray<FCFVehicleFieldOverride> OriginalLegacyPins = BuilderFixture.Recipe->ImportState.LegacyPinnedFields;
	const ECFVehicleManageState OriginalManageState = BuilderFixture.Recipe->ImportState.ManageState;
	// 이 fixture는 Existing Import에서 만들어졌으므로 새 WSA semantic field의 old false pin만 제거해 신규 Builder normal ownership을 재현합니다.
	BuilderFixture.Recipe->ImportState.LegacyPinnedFields.RemoveAll([](const FCFVehicleFieldOverride& Override)
	{
		return Override.FieldPath.ToCanonicalString(true) == TEXT("WheelVisualConfig.bUseWheelSocketScale");
	});
	BuilderFixture.Recipe->ImportState.ManageState = BuilderFixture.Recipe->ImportState.LegacyPinnedFields.IsEmpty()
		? ECFVehicleManageState::Managed
		: ECFVehicleManageState::PartiallyManaged;
	BuilderFixture.Recipe->WheelVisualIntent.Mode = ECFWheelVisualIntentMode::SocketScaleFromChassis;
	++BuilderFixture.Recipe->AuthoringRevision;
	if (!TestTrue(TEXT("WSA Builder refresh with non-canonical Cube succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	const FCFVehicleBuilderStepView* NonCanonicalMeshStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	if (TestNotNull(TEXT("WSA non-canonical Step 2 exists"), NonCanonicalMeshStep))
	{
		TestEqual(TEXT("WSA Socket mode blocks non-canonical Wheel Mesh"), NonCanonicalMeshStep->State, ECFVehicleBuilderStepState::Blocked);
	}

	// 실제 canonical shared Wheel을 FL 하나만 지정해 runtime fallback을 쓰는 신규 정상 경로입니다.
	UStaticMesh* CanonicalWheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL"));
	if (!TestNotNull(TEXT("WSA canonical shared Wheel loads"), CanonicalWheelMesh))
	{
		return false;
	}
	BuilderFixture.Recipe->AssetIntent.WheelMeshFL = CanonicalWheelMesh;
	BuilderFixture.Recipe->AssetIntent.WheelMeshFR = TSoftObjectPtr<UStaticMesh>();
	BuilderFixture.Recipe->AssetIntent.WheelMeshRL = TSoftObjectPtr<UStaticMesh>();
	BuilderFixture.Recipe->AssetIntent.WheelMeshRR = TSoftObjectPtr<UStaticMesh>();
	++BuilderFixture.Recipe->AuthoringRevision;
	if (!TestTrue(TEXT("WSA Builder refresh with canonical FL fallback succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	const FCFVehicleBuilderStepView* CanonicalMeshStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	const FCFVehicleBuilderStepView* CanonicalSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	const FCFVehicleBuilderStepView* CanonicalLayoutStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture);
	if (CanonicalMeshStep && CanonicalSocketStep && CanonicalLayoutStep)
	{
		TestEqual(TEXT("WSA canonical FL-only MeshPrep Complete"), CanonicalMeshStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("WSA OneVector Socket scales keep SocketGuide Complete"), CanonicalSocketStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("WSA persisted OneVector scales keep Layout current"), CanonicalLayoutStep->State, ECFVehicleBuilderStepState::Complete);
	}

	UStaticMeshSocket* WsaSocketFL = BuilderFixture.ChassisMesh ? BuilderFixture.ChassisMesh->FindSocket(TEXT("Wheel_Anchor_FL")) : nullptr;
	UStaticMeshSocket* WsaSocketFR = BuilderFixture.ChassisMesh ? BuilderFixture.ChassisMesh->FindSocket(TEXT("Wheel_Anchor_FR")) : nullptr;
	if (!TestNotNull(TEXT("WSA FL socket exists"), WsaSocketFL) || !TestNotNull(TEXT("WSA FR socket exists"), WsaSocketFR))
	{
		return false;
	}
	WsaSocketFL->RelativeScale = FVector(0.72, 1.12, 0.72);
	WsaSocketFR->RelativeScale = FVector(0.72, 1.12, 0.72);
	if (!TestTrue(TEXT("WSA refresh after valid front axle scale change succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	const FCFVehicleBuilderStepView* ScaleChangedSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	const FCFVehicleBuilderStepView* ScaleChangedLayoutStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture);
	if (ScaleChangedSocketStep && ScaleChangedLayoutStep)
	{
		TestEqual(TEXT("WSA valid equal axle Socket Scale stays Complete"), ScaleChangedSocketStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("WSA scale-only authored change makes Layout stale"), ScaleChangedLayoutStep->State, ECFVehicleBuilderStepState::Stale);
	}

	WsaSocketFL->RelativeScale = FVector(0.72, 1.12, 0.70);
	// Invalid USER Scale은 Resolver/Workspace Preview 자체를 Block하므로 RefreshCurrentState=false가 정상이다. Builder Step state는 실패 경로에서도 rebuild된다.
	TestFalse(TEXT("WSA X/Z mismatch blocks top-level refresh fail-closed"), ManagedBuilderViewModel.RefreshCurrentState(Error));
	const FCFVehicleBuilderStepView* InvalidScaleSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	if (TestNotNull(TEXT("WSA invalid scale SocketGuide exists"), InvalidScaleSocketStep))
	{
		TestEqual(TEXT("WSA X/Z mismatch blocks SocketGuide"), InvalidScaleSocketStep->State, ECFVehicleBuilderStepState::Blocked);
	}

	// 기존 P0-03 후속 회귀가 같은 baseline에서 계속 실행되도록 WSA-only fixture 변경을 원복합니다.
	WsaSocketFL->RelativeScale = FVector::OneVector;
	WsaSocketFR->RelativeScale = FVector::OneVector;
	BuilderFixture.Recipe->WheelVisualIntent.Mode = OriginalWheelVisualMode;
	BuilderFixture.Recipe->AssetIntent.WheelMeshFL = OriginalWheelMeshFL;
	BuilderFixture.Recipe->ImportState.LegacyPinnedFields = OriginalLegacyPins;
	BuilderFixture.Recipe->ImportState.ManageState = OriginalManageState;
	++BuilderFixture.Recipe->AuthoringRevision;
	if (!TestTrue(TEXT("WSA fixture restore succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}

	// Current Chassis StaticMesh에서 fresh AssetSnapshot fingerprint를 바꿀 FL Wheel socket입니다.
	UStaticMeshSocket* WheelSocketFL = BuilderFixture.ChassisMesh ? BuilderFixture.ChassisMesh->FindSocket(TEXT("Wheel_Anchor_FL")) : nullptr;
	if (!TestNotNull(TEXT("Builder P0-03 FL Wheel socket fixture exists"), WheelSocketFL))
	{
		return false;
	}
	// Capture 이후 Chassis socket transform이 바뀐 H 시나리오를 재현합니다.
	WheelSocketFL->RelativeLocation.X += 1.0f;
	if (!TestTrue(TEXT("Builder P0-03 refresh after socket transform change succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	// Socket transform change 뒤 stable StepId evaluator projection입니다.
	const FCFVehicleBuilderStepView* SocketChangedMeshStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	// Socket-only change 뒤 SocketGuide projection입니다.
	const FCFVehicleBuilderStepView* SocketChangedSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	// Socket-only change 뒤 LayoutCapture projection입니다.
	const FCFVehicleBuilderStepView* SocketChangedLayoutStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture);
	if (SocketChangedMeshStep && SocketChangedSocketStep && SocketChangedLayoutStep)
	{
		TestEqual(TEXT("Builder P0-03 MeshPrep stays Complete after socket-only change"), SocketChangedMeshStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 SocketGuide stays Complete after socket-only change"), SocketChangedSocketStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 LayoutCapture becomes Stale after socket transform change"), SocketChangedLayoutStep->State, ECFVehicleBuilderStepState::Stale);
	}

	// FL/FR role이 같은 effective socket name을 공유하는 D 시나리오를 transient Recipe fixture에 구성합니다.
	BuilderFixture.Recipe->AssetIntent.BodyWheelSocketFR = BuilderFixture.Recipe->AssetIntent.BodyWheelSocketFL;
	// Fresh Recipe fingerprint/resolve를 요구하는 semantic revision입니다.
	++BuilderFixture.Recipe->AuthoringRevision;
	if (!TestTrue(TEXT("Builder P0-03 refresh after duplicate Wheel role succeeds"), ManagedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	// Duplicate role edit 뒤 Step 3 UI helper가 current custom binding을 그대로 반영하는지 확인합니다.
	const TArray<FName> DuplicateRequiredSocketNames = ManagedBuilderViewModel.GetRequiredWheelSocketNames();
	if (DuplicateRequiredSocketNames.Num() == 4)
	{
		TestEqual(TEXT("Builder Step 3 helper reflects custom duplicate FR binding"), DuplicateRequiredSocketNames[1], DuplicateRequiredSocketNames[0]);
	}

	// Duplicate role binding 뒤 stable StepId evaluator projection입니다.
	const FCFVehicleBuilderStepView* DuplicateMeshStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::MeshPrep);
	// Duplicate role 상태의 SocketGuide projection입니다.
	const FCFVehicleBuilderStepView* DuplicateSocketStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::SocketGuide);
	// Duplicate role 상태의 LayoutCapture projection입니다.
	const FCFVehicleBuilderStepView* DuplicateLayoutStep = ManagedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::LayoutCapture);
	if (DuplicateMeshStep && DuplicateSocketStep && DuplicateLayoutStep)
	{
		TestEqual(TEXT("Builder P0-03 MeshPrep stays Complete after socket-name-only edit"), DuplicateMeshStep->State, ECFVehicleBuilderStepState::Complete);
		TestEqual(TEXT("Builder P0-03 SocketGuide blocks duplicate Wheel role binding"), DuplicateSocketStep->State, ECFVehicleBuilderStepState::Blocked);
		TestEqual(TEXT("Builder P0-03 LayoutCapture blocks when SocketGuide prerequisite fails"), DuplicateLayoutStep->State, ECFVehicleBuilderStepState::Blocked);
	}
	return true;
}

// Guided Builder Step 1의 ResearchDraft→Companion→USER review→resume→fingerprint stale 전체 USER flow를 검증합니다.
bool FCFVehicleBuilderStep1FlowTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleAuthoringVMTestsPrivate;

	// Existing managed Vehicle truth를 제공하는 integration fixture입니다.
	FWorkspaceFixture Fixture;
	// Fixture/VM/fingerprint diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Builder Step 1 managed fixture builds"), BuildImportedFixture(Fixture, Error))
		|| !TestTrue(TEXT("Builder Step 1 private Profile fixture attaches"), AttachBuilderPrivateProfiles(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Random fixture Recipe 전용 local review token cleanup guard입니다.
	FBuilderReferenceTokenGuard TokenGuard(Fixture.Recipe->RecipeId);
	// Builder selection row입니다.
	const FCFVehicleListEntry Entry = BuildListEntry(Fixture, true);
	// 실제 Guided Shell이 사용하는 Step 1 ViewModel입니다.
	FCFVehicleBuilderVM BuilderViewModel;
	if (!TestTrue(TEXT("Builder Step 1 managed Vehicle selects"), BuilderViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// Evidence가 아직 없을 때의 Step 1 projection입니다.
	const FCFVehicleBuilderStepView* InitialReferenceStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	if (!TestNotNull(TEXT("Builder Step 1 IdentityReference exists"), InitialReferenceStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 1 without Evidence is Ready, not Complete"), InitialReferenceStep->State, ECFVehicleBuilderStepState::Ready);
	TestTrue(TEXT("Builder Step 1 starts without persistent Evidence"), BuilderViewModel.GetCurrentReferenceEvidence() == nullptr);

	// USER의 실제 ResearchDraft.json을 보존하는 file guard입니다.
	FBuilderResearchDraftFileGuard DraftFileGuard(BuilderViewModel.GetResearchDraftPath());
	// AI→Builder transient handoff fixture입니다.
	FCFBuilderResearchDraft Draft;
	Draft.SchemaRevision = 1;
	Draft.RecipeId = Fixture.Recipe->RecipeId;
	Draft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
	Draft.EvidencePayload = BuildBuilderStep1EvidencePayload();
	Draft.ResearchLabel = TEXT("VB-P0-09 Step 1 Integration Draft");
	if (!TestTrue(TEXT("Builder Step 1 typed Research Draft JSON writes"), DraftFileGuard.WriteDraft(Draft, Error)))
	{
		AddError(Error);
		return false;
	}

	if (!TestTrue(TEXT("Builder Step 1 Research Draft exact binding loads"), BuilderViewModel.LoadResearchDraft(Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Builder Step 1 holds loaded transient Research Draft"), BuilderViewModel.HasLoadedResearchDraft());
	TestTrue(TEXT("Builder Step 1 Draft load does not create Evidence"), BuilderViewModel.GetCurrentReferenceEvidence() == nullptr);

	// Exact R2 Companion mutation0 proposal입니다.
	FCFBuilderCompanionPreview CompanionPreview;
	if (!TestTrue(TEXT("Builder Step 1 Companion mutation0 preview succeeds"), BuilderViewModel.PrepareResearchCompanions(CompanionPreview, Error)))
	{
		AddError(Error);
		return false;
	}
	TestFalse(TEXT("Builder Step 1 Companion preview does not save"), CompanionPreview.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("Builder Step 1 prospective Evidence fingerprint is non-empty"), CompanionPreview.ProspectiveEvidenceFingerprint.IsEmpty());

	// Preview 당시 Target full Definition hash입니다.
	const FString TargetHashBeforeCompanion = BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (TargetHashBeforeCompanion.IsEmpty())
	{
		AddError(Error);
		return false;
	}

	// USER explicit OwnershipWrite approval을 VM이 one-shot으로 binding한 terminal result입니다.
	FCFBuilderCompanionResult CompanionResult;
	if (!TestTrue(TEXT("Builder Step 1 approved Companion commit succeeds"), BuilderViewModel.ExecutePreparedResearchCompanions(CompanionResult, Error)))
	{
		AddError(Error);
		return false;
	}
	TestFalse(TEXT("Builder Step 1 Companion commit never auto-saves"), CompanionResult.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("Builder Step 1 Companion commit does not mutate Target"), CompanionResult.Operation.Mutation.bTargetChanged);
	TestEqual(TEXT("Builder Step 1 Companion preserves Target Definition"), BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBeforeCompanion);
	TestNotNull(TEXT("Builder Step 1 persistent Evidence is discoverable after commit"), BuilderViewModel.GetCurrentReferenceEvidence());

	// Companion commit만으로 Reference USER review가 자동 승인되지 않아야 합니다.
	const FCFVehicleBuilderStepView* PreAcceptReferenceStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	if (!TestNotNull(TEXT("Builder Step 1 pre-accept projection exists"), PreAcceptReferenceStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 1 requires explicit USER Reference review after Companion commit"), PreAcceptReferenceStep->State, ECFVehicleBuilderStepState::Ready);

	if (!TestTrue(TEXT("Builder Step 1 explicit Reference Set review token succeeds"), BuilderViewModel.AcceptCurrentReferenceSet(Error)))
	{
		AddError(Error);
		return false;
	}
	// USER review 직후 current fingerprint와 token이 일치한 projection입니다.
	const FCFVehicleBuilderStepView* AcceptedReferenceStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	if (!TestNotNull(TEXT("Builder Step 1 accepted projection exists"), AcceptedReferenceStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 1 becomes Complete only after exact USER fingerprint review"), AcceptedReferenceStep->State, ECFVehicleBuilderStepState::Complete);

	// Editor restart와 같은 새 transient VM입니다. Mutation approval은 복원하지 않고 local review token만 복원해야 합니다.
	FCFVehicleBuilderVM ResumedBuilderViewModel;
	if (!TestTrue(TEXT("Builder Step 1 fresh VM reselects same managed Vehicle"), ResumedBuilderViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	// Fresh VM이 persistent Evidence + local token에서 다시 계산한 projection입니다.
	const FCFVehicleBuilderStepView* ResumedReferenceStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	if (!TestNotNull(TEXT("Builder Step 1 resumed projection exists"), ResumedReferenceStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 1 local Reference review token resumes exact current Evidence"), ResumedReferenceStep->State, ECFVehicleBuilderStepState::Complete);

	// Stale test를 위해 current persistent Evidence를 보존합니다.
	UCFVehicleRefEvidence* Evidence = ResumedBuilderViewModel.GetCurrentReferenceEvidence();
	if (!TestNotNull(TEXT("Builder Step 1 resumed Evidence exists"), Evidence) || Evidence->Claims.IsEmpty())
	{
		return false;
	}
	// Drift 전 canonical FACT 값입니다.
	const double OriginalClaimValue = Evidence->Claims[0].NumberValue;
	// Drift 전 USER-approved Evidence fingerprint입니다.
	const FString OriginalEvidenceFingerprint = Evidence->EvidenceFingerprint;
	Evidence->Claims[0].NumberValue = OriginalClaimValue + 1.0;
	if (!Evidence->RefreshEvidenceFingerprint(Error))
	{
		AddError(Error);
		return false;
	}
	TestNotEqual(TEXT("Builder Step 1 semantic Evidence drift changes fingerprint"), Evidence->EvidenceFingerprint, OriginalEvidenceFingerprint);

	if (!TestTrue(TEXT("Builder Step 1 refresh after Evidence drift succeeds"), ResumedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	// Old local review token과 new current Evidence fingerprint가 달라진 projection입니다.
	const FCFVehicleBuilderStepView* StaleReferenceStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	if (!TestNotNull(TEXT("Builder Step 1 stale projection exists"), StaleReferenceStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 1 becomes Stale when Evidence fingerprint changes"), StaleReferenceStep->State, ECFVehicleBuilderStepState::Stale);

	// Test process 내 후속 검사에 drifted Evidence를 남기지 않도록 original semantic state를 복원합니다.
	Evidence->Claims[0].NumberValue = OriginalClaimValue;
	if (!Evidence->RefreshEvidenceFingerprint(Error))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Builder Step 1 Evidence restore returns original fingerprint"), Evidence->EvidenceFingerprint, OriginalEvidenceFingerprint);

	// Asset Registry에 등록된 unsaved test Evidence를 후속 Automation에서 보이지 않도록 제거합니다.
	CleanupRegisteredAsset(Evidence);
	return true;
}


// Guided Builder Step 5의 PhysicsDraft→private 4 Profile typed Preview/Commit→receipt resume→Profile drift Stale 전체 flow를 검증합니다.
bool FCFVehicleBuilderStep5PhysicsTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleAuthoringVMTestsPrivate;

	// Step 1~5를 함께 통과할 managed Vehicle integration fixture입니다.
	FWorkspaceFixture Fixture;
	// Fixture/Draft/Profile diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Builder Step 5 managed fixture builds"), BuildImportedFixture(Fixture, Error))
		|| !TestTrue(TEXT("Builder Step 5 private Profile fixture attaches"), AttachBuilderPrivateProfiles(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Random fixture Recipe 전용 local Reference review token cleanup guard입니다.
	FBuilderReferenceTokenGuard TokenGuard(Fixture.Recipe->RecipeId);
	// Guided Builder selection row입니다.
	const FCFVehicleListEntry Entry = BuildListEntry(Fixture, true);
	// 실제 Guided Shell이 사용하는 ViewModel입니다.
	FCFVehicleBuilderVM BuilderViewModel;
	if (!TestTrue(TEXT("Builder Step 5 managed Vehicle selects"), BuilderViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// USER의 실제 ResearchDraft.json을 보존하는 Step 1 file guard입니다.
	FBuilderResearchDraftFileGuard ResearchDraftGuard(BuilderViewModel.GetResearchDraftPath());
	// Step 5 prerequisite Reference Evidence를 만드는 AI Research Draft fixture입니다.
	FCFBuilderResearchDraft ResearchDraft;
	ResearchDraft.SchemaRevision = 1;
	ResearchDraft.RecipeId = Fixture.Recipe->RecipeId;
	ResearchDraft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
	ResearchDraft.EvidencePayload = BuildBuilderStep1EvidencePayload();
	ResearchDraft.ResearchLabel = TEXT("VB-P0-09 Step 5 Reference Fixture");
	if (!TestTrue(TEXT("Builder Step 5 prerequisite Research Draft writes"), ResearchDraftGuard.WriteDraft(ResearchDraft, Error))
		|| !TestTrue(TEXT("Builder Step 5 prerequisite Research Draft loads"), BuilderViewModel.LoadResearchDraft(Error)))
	{
		AddError(Error);
		return false;
	}

	// Existing private 4 Profile은 보존하고 Missing Evidence만 만드는 Step 1 mutation0 preview입니다.
	FCFBuilderCompanionPreview CompanionPreview;
	if (!TestTrue(TEXT("Builder Step 5 prerequisite Companion preview succeeds"), BuilderViewModel.PrepareResearchCompanions(CompanionPreview, Error)))
	{
		AddError(Error);
		return false;
	}
	// USER-approved Step 1 Companion commit fixture 결과입니다.
	FCFBuilderCompanionResult CompanionResult;
	if (!TestTrue(TEXT("Builder Step 5 prerequisite Companion commit succeeds"), BuilderViewModel.ExecutePreparedResearchCompanions(CompanionResult, Error)))
	{
		AddError(Error);
		return false;
	}
	if (!TestNotNull(TEXT("Builder Step 5 prerequisite Evidence creates"), CompanionResult.CreatedEvidence.Get()))
	{
		return false;
	}
	if (!TestTrue(TEXT("Builder Step 5 prerequisite Reference Set accepts"), BuilderViewModel.AcceptCurrentReferenceSet(Error)))
	{
		AddError(Error);
		return false;
	}

	// Step 1~4가 current baseline에서 모두 Complete인지 확인합니다.
	const ECFVehicleBuilderStepId PrerequisiteStepIds[] =
	{
		ECFVehicleBuilderStepId::IdentityReference,
		ECFVehicleBuilderStepId::MeshPrep,
		ECFVehicleBuilderStepId::SocketGuide,
		ECFVehicleBuilderStepId::LayoutCapture
	};
	for (const ECFVehicleBuilderStepId StepId : PrerequisiteStepIds)
	{
		// Stable StepId 기반 prerequisite projection입니다.
		const FCFVehicleBuilderStepView* Step = BuilderViewModel.FindStepView(StepId);
		if (!TestNotNull(TEXT("Builder Step 5 prerequisite Step exists"), Step))
		{
			return false;
		}
		TestEqual(TEXT("Builder Step 5 prerequisite is Complete"), Step->State, ECFVehicleBuilderStepState::Complete);
	}

	// Step 5 commit 전 Target full Definition hash입니다.
	const FString TargetHashBeforePhysics = BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (TargetHashBeforePhysics.IsEmpty())
	{
		AddError(Error);
		return false;
	}

	// Current Recipe exact private VehicleBase Profile입니다.
	UCFVehicleBaseProfile* VehicleBaseProfile = Fixture.Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	// Current Recipe exact private Drivetrain Profile입니다.
	UCFDrivetrainProfile* DrivetrainProfile = Fixture.Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	// Current Recipe exact private Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = Fixture.Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	// Current Recipe exact private Performance Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = Fixture.Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!TestNotNull(TEXT("Builder Step 5 VehicleBase private Profile exists"), VehicleBaseProfile)
		|| !TestNotNull(TEXT("Builder Step 5 Drivetrain private Profile exists"), DrivetrainProfile)
		|| !TestNotNull(TEXT("Builder Step 5 Handling private Profile exists"), HandlingProfile)
		|| !TestNotNull(TEXT("Builder Step 5 Performance private Profile exists"), PerformanceProfile))
	{
		return false;
	}

	// USER의 실제 PhysicsDraft.json을 보존하는 Step 5 file guard입니다.
	FBuilderPhysicsDraftFileGuard PhysicsDraftGuard(BuilderViewModel.GetPhysicsProposalDraftPath());
	// AI가 current accepted Evidence에서 작성한 complete typed Physics Proposal fixture입니다.
	FCFBuilderPhysicsDraft PhysicsDraft;
	PhysicsDraft.SchemaRevision = 1;
	PhysicsDraft.RecipeId = Fixture.Recipe->RecipeId;
	PhysicsDraft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
	PhysicsDraft.EvidenceId = CompanionResult.CreatedEvidence->EvidenceId;
	PhysicsDraft.ExpectedEvidenceFingerprint = CompanionResult.CreatedEvidence->EvidenceFingerprint;
	PhysicsDraft.ProfilePayload.VehicleBaseData = VehicleBaseProfile->Data;
	PhysicsDraft.ProfilePayload.DrivetrainData = DrivetrainProfile->Data;
	PhysicsDraft.ProfilePayload.HandlingData = HandlingProfile->Data;
	PhysicsDraft.ProfilePayload.PerformanceData = PerformanceProfile->Data;
	// 실제 Physics Proposal 변화가 존재하도록 VehicleBase의 typed chassis width만 안전하게 변경합니다.
	PhysicsDraft.ProfilePayload.VehicleBaseData.ChassisWidth = VehicleBaseProfile->Data.ChassisWidth + 1.0f;
	PhysicsDraft.ConsumedClaimIds = {TEXT("CLAIM-BUILDER-STEP1-MASS")};
	PhysicsDraft.ProposalCorrelationHash = TEXT("1111111111111111111111111111111111111111111111111111111111111111");
	PhysicsDraft.ProposalLabel = TEXT("Step 5 Automation Physics Proposal");
	PhysicsDraft.UserFacingSummary = TEXT("Reference 근거를 유지하면서 차체 물리 폭의 typed proposal 변화를 검증합니다.");

	if (!TestTrue(TEXT("Builder Step 5 typed Physics Draft writes"), PhysicsDraftGuard.WriteDraft(PhysicsDraft, Error))
		|| !TestTrue(TEXT("Builder Step 5 Physics Draft exact binding loads"), BuilderViewModel.LoadPhysicsProposalDraft(Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Builder Step 5 holds loaded Physics Draft"), BuilderViewModel.HasLoadedPhysicsProposalDraft());

	// Loaded Draft 상태의 Step 5 projection입니다.
	const FCFVehicleBuilderStepView* LoadedPhysicsStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	if (!TestNotNull(TEXT("Builder Step 5 loaded projection exists"), LoadedPhysicsStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 5 loaded proposal is Ready before explicit commit"), LoadedPhysicsStep->State, ECFVehicleBuilderStepState::Ready);

	// USER dialog 전에 existing typed facade가 만든 exact mutation0 preview입니다.
	FCFBuilderProfileCommitPreview PhysicsPreview;
	if (!TestTrue(TEXT("Builder Step 5 typed mutation0 preview succeeds"), BuilderViewModel.PreparePhysicsProposal(PhysicsPreview, Error)))
	{
		AddError(Error);
		return false;
	}
	TestFalse(TEXT("Builder Step 5 preview never saves"), PhysicsPreview.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("Builder Step 5 preview never mutates Target"), PhysicsPreview.Operation.Mutation.bTargetChanged);
	TestNotEqual(
		TEXT("Builder Step 5 VehicleBase proposal changes typed fingerprint"),
		PhysicsPreview.CurrentFingerprints.VehicleBaseFingerprint,
		PhysicsPreview.ProspectiveFingerprints.VehicleBaseFingerprint);

	// USER explicit AuthoringWrite approval을 VM이 one-shot binding한 terminal result입니다.
	FCFAuthoringOpResult PhysicsCommitResult;
	if (!TestTrue(TEXT("Builder Step 5 approved private Profile commit succeeds"), BuilderViewModel.ExecutePreparedPhysicsProposal(PhysicsCommitResult, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Builder Step 5 commit changes Builder-private Profile payload"), PhysicsCommitResult.Mutation.bProfileChanged);
	TestTrue(TEXT("Builder Step 5 commit records Recipe receipt metadata"), PhysicsCommitResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Builder Step 5 commit does not mutate Target VehicleData"), PhysicsCommitResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Builder Step 5 commit does not auto-save"), PhysicsCommitResult.Mutation.bSavePerformed);
	TestEqual(TEXT("Builder Step 5 commit preserves Target full Definition"), BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBeforePhysics);
	TestTrue(TEXT("Builder Step 5 persistent receipt is valid"), Fixture.Recipe->BuilderCommitReceipt.IsValid());
	TestEqual(TEXT("Builder Step 5 receipt Evidence fingerprint is exact"), Fixture.Recipe->BuilderCommitReceipt.EvidenceFingerprint, CompanionResult.CreatedEvidence->EvidenceFingerprint);
	TestFalse(TEXT("Builder Step 5 consumed transient Physics Draft is cleared"), BuilderViewModel.HasLoadedPhysicsProposalDraft());

	// Commit 직후 persistent receipt/current private Profile truth에서 다시 파생한 projection입니다.
	const FCFVehicleBuilderStepView* CommittedPhysicsStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	if (!TestNotNull(TEXT("Builder Step 5 committed projection exists"), CommittedPhysicsStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 5 becomes Complete after exact typed commit receipt"), CommittedPhysicsStep->State, ECFVehicleBuilderStepState::Complete);

	// Step 5 Complete 직후 existing ReadBuilderGameplayGuidance R0에서 fresh derive한 Step 6 projection입니다.
	const FCFVehicleBuilderStepView* CommittedGameplayStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::GameplaySetup);
	if (!TestNotNull(TEXT("Builder Step 6 committed-time projection exists"), CommittedGameplayStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 6 becomes Complete from current Gameplay Guidance"), CommittedGameplayStep->State, ECFVehicleBuilderStepState::Complete);
	TestTrue(TEXT("Builder Step 6 stores fresh Gameplay Guidance projection"), BuilderViewModel.HasGameplayGuidanceResult());

	// Step 6이 그대로 노출하는 existing R0 Gameplay Guidance 결과입니다.
	const FCFBuilderGameplayGuidanceResult& CommittedGuidance = BuilderViewModel.GetGameplayGuidanceResult();
	TestEqual(TEXT("Builder Step 6 exposes fixed 8 gameplay areas"), CommittedGuidance.Items.Num(), 8);
	TestEqual(TEXT("Builder Step 6 ready guidance has no blocked area"), CommittedGuidance.BlockedCount, 0);
	TestEqual(TEXT("Builder Step 6 ready guidance has no USER review area"), CommittedGuidance.NeedsReviewCount, 0);
	TestTrue(TEXT("Builder Step 6 ready guidance can complete"), CommittedGuidance.bCanCompleteGameplayStep);
	TestFalse(TEXT("Builder Step 6 R0 does not mutate Recipe"), CommittedGuidance.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Builder Step 6 R0 does not mutate Target"), CommittedGuidance.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Builder Step 6 R0 does not mutate Profile"), CommittedGuidance.Operation.Mutation.bProfileChanged);
	TestFalse(TEXT("Builder Step 6 R0 never saves"), CommittedGuidance.Operation.Mutation.bSavePerformed);

	// Editor restart와 같은 fresh transient VM입니다. old mutation approval 없이 persistent receipt와 local Reference token만 재사용합니다.
	FCFVehicleBuilderVM ResumedBuilderViewModel;
	if (!TestTrue(TEXT("Builder Step 5 fresh VM reselects same Vehicle"), ResumedBuilderViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	// Fresh VM에서 persistent receipt가 current Evidence/Profile/Resolver와 일치하는 Step 5 projection입니다.
	const FCFVehicleBuilderStepView* ResumedPhysicsStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	if (!TestNotNull(TEXT("Builder Step 5 resumed projection exists"), ResumedPhysicsStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 5 resumes Complete from persistent current truth"), ResumedPhysicsStep->State, ECFVehicleBuilderStepState::Complete);
	TestFalse(TEXT("Builder Step 5 fresh VM restores no transient Physics Draft"), ResumedBuilderViewModel.HasLoadedPhysicsProposalDraft());

	// Fresh VM도 별도 persistent Step 6 flag 없이 current R0 Gameplay Guidance에서 다시 계산한 projection입니다.
	const FCFVehicleBuilderStepView* ResumedGameplayStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::GameplaySetup);
	if (!TestNotNull(TEXT("Builder Step 6 resumed projection exists"), ResumedGameplayStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 6 resumes Complete from fresh R0 truth"), ResumedGameplayStep->State, ECFVehicleBuilderStepState::Complete);
	TestTrue(TEXT("Builder Step 6 fresh VM re-reads Gameplay Guidance"), ResumedBuilderViewModel.HasGameplayGuidanceResult());
	TestEqual(TEXT("Builder Step 6 fresh VM still exposes 8 areas"), ResumedBuilderViewModel.GetGameplayGuidanceResult().Items.Num(), 8);

	// Persistent receipt 이후 transaction 밖 private Profile drift 전 값입니다.
	const float BrakeTorqueBeforeDrift = HandlingProfile->Data.FrontWheelMaxBrakeTorque;
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = BrakeTorqueBeforeDrift + 1.0f;
	if (!TestTrue(TEXT("Builder Step 5 refresh after private Profile drift succeeds"), ResumedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		return false;
	}
	// Receipt fingerprint와 current private Profile fingerprint가 달라진 projection입니다.
	const FCFVehicleBuilderStepView* StalePhysicsStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	if (!TestNotNull(TEXT("Builder Step 5 stale projection exists"), StalePhysicsStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 5 becomes Stale after private Profile drift"), StalePhysicsStep->State, ECFVehicleBuilderStepState::Stale);

	// Step 5 provenance가 Stale이면 Step 6은 old guidance를 Complete로 유지하지 않고 prerequisite에서 잠겨야 합니다.
	const FCFVehicleBuilderStepView* LockedGameplayStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::GameplaySetup);
	if (!TestNotNull(TEXT("Builder Step 6 stale-prerequisite projection exists"), LockedGameplayStep))
	{
		return false;
	}
	TestEqual(TEXT("Builder Step 6 locks when Step 5 becomes Stale"), LockedGameplayStep->State, ECFVehicleBuilderStepState::Locked);
	TestFalse(TEXT("Builder Step 6 drops old guidance when prerequisite becomes stale"), ResumedBuilderViewModel.HasGameplayGuidanceResult());

	// 후속 Automation에 raw drift를 남기지 않도록 원래 Profile payload를 복원합니다.
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = BrakeTorqueBeforeDrift;

	// Asset Registry에 등록된 unsaved test Evidence를 후속 Automation에서 보이지 않도록 제거합니다.
	CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
	return true;
}


// Guided Builder Step 7의 fresh Final Review → explicit DefinitionApply → exact guarded Undo Shell VM flow를 검증합니다.
bool FCFVehicleBuilderStep7ReviewTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleAuthoringVMTestsPrivate;

	// Step 7 prerequisite를 최소 current truth로 구성할 managed Vehicle fixture입니다.
	FWorkspaceFixture Fixture;
	// Fixture/Builder operation diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Builder Step 7 managed fixture builds"), BuildImportedFixture(Fixture, Error))
		|| !TestTrue(TEXT("Builder Step 7 private Profile fixture attaches"), AttachBuilderPrivateProfiles(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Step 7이 실제 DefinitionApply를 요구하도록 imported MaxHealth legacy pin만 prerequisite 단계에서 release합니다.
	Fixture.Recipe->ImportState.LegacyPinnedFields.RemoveAll([](const FCFVehicleFieldOverride& Override)
	{
		return Override.FieldPath.ToCanonicalString(true) == TEXT("VehicleDurabilityConfig.MaxHealth");
	});
	// Final Review가 materialize할 current semantic durability desired value입니다.
	Fixture.Recipe->DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::ExplicitValue;
	Fixture.Recipe->DurabilityIntent.ExplicitMaxHealth = Fixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth + 25.0f;
	++Fixture.Recipe->AuthoringRevision;

	// Random fixture Recipe 전용 local Reference review token cleanup guard입니다.
	FBuilderReferenceTokenGuard TokenGuard(Fixture.Recipe->RecipeId);
	// Guided Builder selection row입니다.
	const FCFVehicleListEntry Entry = BuildListEntry(Fixture, true);
	// 실제 Guided Shell이 사용하는 transient ViewModel입니다.
	FCFVehicleBuilderVM BuilderViewModel;
	if (!TestTrue(TEXT("Builder Step 7 managed Vehicle selects"), BuilderViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// USER의 실제 ResearchDraft.json을 보존하는 prerequisite file guard입니다.
	FBuilderResearchDraftFileGuard ResearchDraftGuard(BuilderViewModel.GetResearchDraftPath());
	// Step 7 provenance prerequisite Evidence를 만드는 typed Research Draft입니다.
	FCFBuilderResearchDraft ResearchDraft;
	ResearchDraft.SchemaRevision = 1;
	ResearchDraft.RecipeId = Fixture.Recipe->RecipeId;
	ResearchDraft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
	ResearchDraft.EvidencePayload = BuildBuilderStep1EvidencePayload();
	ResearchDraft.ResearchLabel = TEXT("VB-P0-09 Step 7 Reference Fixture");
	if (!ResearchDraftGuard.WriteDraft(ResearchDraft, Error)
		|| !BuilderViewModel.LoadResearchDraft(Error))
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Reference prerequisite failed: %s"), *Error));
		return false;
	}

	// Existing private Profile 4종을 보존하고 missing Evidence만 만드는 prerequisite preview입니다.
	FCFBuilderCompanionPreview CompanionPreview;
	if (!BuilderViewModel.PrepareResearchCompanions(CompanionPreview, Error))
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Companion preview prerequisite failed: %s"), *Error));
		return false;
	}

	// Provenance owner Evidence를 만드는 prerequisite terminal result입니다.
	FCFBuilderCompanionResult CompanionResult;
	if (!BuilderViewModel.ExecutePreparedResearchCompanions(CompanionResult, Error)
		|| !CompanionResult.CreatedEvidence.Get())
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Companion prerequisite failed: %s"), *Error));
		return false;
	}
	if (!BuilderViewModel.AcceptCurrentReferenceSet(Error))
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Reference review prerequisite failed: %s"), *Error));
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}

	// Current Recipe exact private VehicleBase Profile입니다.
	UCFVehicleBaseProfile* VehicleBaseProfile = Fixture.Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	// Current Recipe exact private Drivetrain Profile입니다.
	UCFDrivetrainProfile* DrivetrainProfile = Fixture.Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	// Current Recipe exact private Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = Fixture.Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	// Current Recipe exact private Performance Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = Fixture.Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!VehicleBaseProfile || !DrivetrainProfile || !HandlingProfile || !PerformanceProfile)
	{
		AddError(TEXT("Builder Step 7 private Profile prerequisite를 읽을 수 없습니다."));
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}

	// USER의 실제 PhysicsDraft.json을 보존하는 prerequisite file guard입니다.
	FBuilderPhysicsDraftFileGuard PhysicsDraftGuard(BuilderViewModel.GetPhysicsProposalDraftPath());
	// Target Definition에 실제 Diff를 만들 complete typed Physics Proposal입니다.
	FCFBuilderPhysicsDraft PhysicsDraft;
	PhysicsDraft.SchemaRevision = 1;
	PhysicsDraft.RecipeId = Fixture.Recipe->RecipeId;
	PhysicsDraft.TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);
	PhysicsDraft.EvidenceId = CompanionResult.CreatedEvidence->EvidenceId;
	PhysicsDraft.ExpectedEvidenceFingerprint = CompanionResult.CreatedEvidence->EvidenceFingerprint;
	PhysicsDraft.ProfilePayload.VehicleBaseData = VehicleBaseProfile->Data;
	PhysicsDraft.ProfilePayload.DrivetrainData = DrivetrainProfile->Data;
	PhysicsDraft.ProfilePayload.HandlingData = HandlingProfile->Data;
	PhysicsDraft.ProfilePayload.PerformanceData = PerformanceProfile->Data;
	// Step 7이 실제 DefinitionApply할 단일 safe typed difference입니다.
	PhysicsDraft.ProfilePayload.VehicleBaseData.ChassisWidth = VehicleBaseProfile->Data.ChassisWidth + 1.0f;
	PhysicsDraft.ConsumedClaimIds = {TEXT("CLAIM-BUILDER-STEP1-MASS")};
	PhysicsDraft.ProposalCorrelationHash = TEXT("2222222222222222222222222222222222222222222222222222222222222222");
	PhysicsDraft.ProposalLabel = TEXT("Step 7 Automation Physics Prerequisite");
	PhysicsDraft.UserFacingSummary = TEXT("Step 7 DefinitionApply/Undo 검증용 typed chassis width difference입니다.");
	if (!PhysicsDraftGuard.WriteDraft(PhysicsDraft, Error)
		|| !BuilderViewModel.LoadPhysicsProposalDraft(Error))
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Physics prerequisite load failed: %s"), *Error));
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}

	// Existing typed facade의 prerequisite mutation0 Profile proposal입니다.
	FCFBuilderProfileCommitPreview PhysicsPreview;
	if (!BuilderViewModel.PreparePhysicsProposal(PhysicsPreview, Error))
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Physics prerequisite preview failed: %s"), *Error));
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}

	// Step 7 provenance receipt를 만드는 prerequisite private Profile commit입니다.
	FCFAuthoringOpResult PhysicsCommitResult;
	if (!BuilderViewModel.ExecutePreparedPhysicsProposal(PhysicsCommitResult, Error))
	{
		AddError(FString::Printf(TEXT("Builder Step 7 Physics prerequisite commit failed: %s"), *Error));
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}

	// Step 7 Apply 전 exact Target Definition hash입니다.
	const FString PreApplyTargetHash = BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (PreApplyTargetHash.IsEmpty())
	{
		AddError(Error);
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}

	// Step 7의 current fresh projection입니다.
	const FCFVehicleBuilderStepView* InitialFinalReviewStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::FinalReview);
	if (!TestNotNull(TEXT("Builder Step 7 projection exists"), InitialFinalReviewStep))
	{
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}
	TestEqual(TEXT("Builder Step 7 is Ready when reviewed Target Diff exists"), InitialFinalReviewStep->State, ECFVehicleBuilderStepState::Ready);
	TestTrue(TEXT("Builder Step 7 has fresh Final Review result"), BuilderViewModel.HasFinalReviewResult());

	// Existing ReadBuilderFinalReview R0 projection입니다.
	const FCFBuilderFinalReviewResult& InitialReview = BuilderViewModel.GetFinalReviewResult();
	TestEqual(TEXT("Builder Step 7 Final Review blocker count is zero"), InitialReview.BlockingIssueCount, 0);
	TestFalse(TEXT("Builder Step 7 Final Review has no external drift"), InitialReview.bHasExternalDrift);
	TestTrue(TEXT("Builder Step 7 Final Review requires Apply"), InitialReview.bApplyRequired);
	TestTrue(TEXT("Builder Step 7 Final Review can Apply"), InitialReview.bCanApply);
	TestFalse(TEXT("Builder Step 7 Final Review is not complete before Apply"), InitialReview.bCanCompleteFinalReview);
	TestTrue(TEXT("Builder Step 7 Final Review exposes at least one exact field diff"), InitialReview.FieldDiff.Num() > 0);
	TestTrue(TEXT("Builder Step 7 provenance is available"), InitialReview.Provenance.bAvailable);
	TestFalse(TEXT("Builder Step 7 R0 never mutates Target"), InitialReview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Builder Step 7 R0 never mutates Recipe"), InitialReview.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Builder Step 7 R0 never saves"), InitialReview.Operation.Mutation.bSavePerformed);

	// USER dialog 직전 exact mutation0 DefinitionApply proposal입니다.
	FCFBuilderFinalReviewResult PreparedReview;
	if (!TestTrue(TEXT("Builder Step 7 prepares fresh DefinitionApply review"), BuilderViewModel.PrepareFinalReviewApply(PreparedReview, Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}
	TestTrue(TEXT("Builder Step 7 prepared review keeps exact proposal hash"), !PreparedReview.ApplyProposal.ProposalHash.IsEmpty());
	TestFalse(TEXT("Builder Step 7 prepared review still does not mutate Target"), PreparedReview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Builder Step 7 prepared review never saves"), PreparedReview.Operation.Mutation.bSavePerformed);

	// Explicit USER DefinitionApply를 모사하는 VM terminal result입니다.
	FCFBuilderFinalApplyResult ApplyResult;
	if (!TestTrue(TEXT("Builder Step 7 explicit DefinitionApply succeeds"), BuilderViewModel.ExecutePreparedFinalReviewApply(ApplyResult, Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}
	TestTrue(TEXT("Builder Step 7 Apply mutates Target through existing R3 lane"), ApplyResult.Operation.Mutation.bTargetChanged);
	TestTrue(TEXT("Builder Step 7 Apply updates Recipe AppliedState"), ApplyResult.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Builder Step 7 Apply does not mutate private Profiles"), ApplyResult.Operation.Mutation.bProfileChanged);
	TestFalse(TEXT("Builder Step 7 Apply never auto-saves"), ApplyResult.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("Builder Step 7 Apply never automatic-retries"), ApplyResult.Operation.Mutation.bAutomaticRetryPerformed);
	TestTrue(TEXT("Builder Step 7 Apply exposes guarded Undo"), ApplyResult.bUndoAvailable);
	TestTrue(TEXT("Builder Step 7 VM stores guarded Undo token"), BuilderViewModel.HasFinalReviewUndoToken());

	// Apply 뒤 fresh current truth의 Step 7 projection입니다.
	const FCFVehicleBuilderStepView* AppliedFinalReviewStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::FinalReview);
	if (!TestNotNull(TEXT("Builder Step 7 applied projection exists"), AppliedFinalReviewStep))
	{
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}
	TestEqual(TEXT("Builder Step 7 becomes Complete after exact Apply"), AppliedFinalReviewStep->State, ECFVehicleBuilderStepState::Complete);
	TestTrue(TEXT("Builder Step 7 post-Apply Final Review exists"), BuilderViewModel.HasFinalReviewResult());
	TestTrue(TEXT("Builder Step 7 post-Apply review can complete"), BuilderViewModel.GetFinalReviewResult().bCanCompleteFinalReview);
	TestFalse(TEXT("Builder Step 7 post-Apply review requires no Apply"), BuilderViewModel.GetFinalReviewResult().bApplyRequired);
	TestEqual(TEXT("Builder Step 7 post-Apply exact diff count is zero"), BuilderViewModel.GetFinalReviewResult().FieldDiff.Num(), 0);
	TestNotEqual(TEXT("Builder Step 7 Apply changes full Target hash"), BuildTargetHash(*Fixture.TargetVehicleData, Error), PreApplyTargetHash);

	// Explicit USER guarded Undo를 모사하는 exact token terminal result입니다.
	FCFAuthoringOpResult UndoResult;
	if (!TestTrue(TEXT("Builder Step 7 exact guarded Undo succeeds"), BuilderViewModel.ExecuteFinalReviewUndo(UndoResult, Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}
	TestTrue(TEXT("Builder Step 7 guarded Undo reports Target change"), UndoResult.Mutation.bTargetChanged);
	TestTrue(TEXT("Builder Step 7 guarded Undo reports Recipe AppliedState change"), UndoResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Builder Step 7 guarded Undo never saves"), UndoResult.Mutation.bSavePerformed);
	TestFalse(TEXT("Builder Step 7 guarded Undo token is consumed"), BuilderViewModel.HasFinalReviewUndoToken());
	TestEqual(TEXT("Builder Step 7 guarded Undo restores exact pre-Apply Target hash"), BuildTargetHash(*Fixture.TargetVehicleData, Error), PreApplyTargetHash);

	// Undo 뒤 current Target에는 다시 reviewed Diff가 있으므로 Step 7은 Ready로 돌아가야 합니다.
	const FCFVehicleBuilderStepView* RestoredFinalReviewStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::FinalReview);
	if (!TestNotNull(TEXT("Builder Step 7 post-Undo projection exists"), RestoredFinalReviewStep))
	{
		CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
		return false;
	}
	TestEqual(TEXT("Builder Step 7 returns Ready after exact guarded Undo"), RestoredFinalReviewStep->State, ECFVehicleBuilderStepState::Ready);
	TestTrue(TEXT("Builder Step 7 post-Undo review can Apply again only after new review"), BuilderViewModel.GetFinalReviewResult().bCanApply);
	TestTrue(TEXT("Builder Step 7 post-Undo diff is restored"), BuilderViewModel.GetFinalReviewResult().FieldDiff.Num() > 0);

	// Asset Registry에 등록된 unsaved test Evidence를 후속 Automation에서 보이지 않도록 제거합니다.
	CleanupRegisteredAsset(CompanionResult.CreatedEvidence.Get());
	return true;
}


// Guided Builder Step 8의 saved-state gate / current benchmark binding / USER Driving exact resume contract를 검증합니다.
bool FCFVehicleBuilderStep8DrivingTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleAuthoringVMTestsPrivate;

	// Step 8 final-applied prerequisite를 구성할 managed Vehicle fixture입니다.
	FWorkspaceFixture Fixture;
	// Fixture/Step 8 diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Builder Step 8 managed fixture builds"), BuildImportedFixture(Fixture, Error))
		|| !TestTrue(TEXT("Builder Step 8 private Profile fixture attaches"), AttachBuilderPrivateProfiles(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Random Recipe의 Step 1 USER Reference token을 test lifetime 동안만 보존/정리합니다.
	FBuilderReferenceTokenGuard ReferenceTokenGuard(Fixture.Recipe->RecipeId);
	// Random Recipe의 Step 8 USER Driving token을 test lifetime 동안만 보존/정리합니다.
	FBuilderDrivingTokenGuard DrivingTokenGuard(Fixture.Recipe->RecipeId);
	// USER의 실제 benchmark result JSON을 exact 복원할 file guard입니다.
	FCFVehicleBuilderVM BuilderViewModel;
	FBuilderBenchmarkResultFileGuard BenchmarkFileGuard(BuilderViewModel.GetDrivingBenchmarkResultPath());

	// Step 1~7 test filter를 replay하지 않고 final-applied prerequisite만 최소 구성해 주는 Evidence입니다.
	UCFVehicleRefEvidence* Evidence = nullptr;
	if (!TestTrue(
		TEXT("Builder Step 8 final-applied prerequisite builds"),
		PrepareBuilderStep8AppliedFixture(Fixture, BuilderViewModel, Evidence, Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(Evidence);
		return false;
	}

	// Step 8 prerequisite가 실제 Final Review Complete까지 도달했는지 확인하는 current projection입니다.
	const FCFVehicleBuilderStepView* FinalReviewStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::FinalReview);
	if (!TestNotNull(TEXT("Builder Step 8 Final Review prerequisite exists"), FinalReviewStep))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestEqual(TEXT("Builder Step 8 prerequisite Final Review is Complete"), FinalReviewStep->State, ECFVehicleBuilderStepState::Complete);

	// Step 7 Apply 직후 auto Save가 없기 때문에 Dirty saved-state gate에 머무는 Step 8 projection입니다.
	const FCFVehicleBuilderStepView* DirtyDrivingStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::DrivingTest);
	if (!TestNotNull(TEXT("Builder Step 8 dirty-state projection exists"), DirtyDrivingStep))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestEqual(TEXT("Builder Step 8 waits in Ready while applied Target is unsaved"), DirtyDrivingStep->State, ECFVehicleBuilderStepState::Ready);
	TestTrue(TEXT("Builder Step 8 Recipe is dirty after explicit Apply"), Fixture.RecipePackage->IsDirty());
	TestTrue(TEXT("Builder Step 8 Target is dirty after explicit Apply"), Fixture.TargetPackage->IsDirty());

	// Dirty Target에서는 external fresh PIE benchmark launch를 절대 준비하지 않는 launch fields입니다.
	FString Executable;
	// Dirty Target launch argument입니다.
	FString Arguments;
	// Dirty Target launch working directory입니다.
	FString WorkingDirectory;
	// Dirty Target launch run identity입니다.
	FString DirtyRunId;
	TestFalse(
		TEXT("Builder Step 8 refuses benchmark launch before explicit save"),
		BuilderViewModel.PrepareDrivingBenchmarkLaunch(Executable, Arguments, WorkingDirectory, DirtyRunId, Error));
	TestTrue(TEXT("Builder Step 8 dirty launch guard does not auto-save Recipe"), Fixture.RecipePackage->IsDirty());
	TestTrue(TEXT("Builder Step 8 dirty launch guard does not auto-save Target"), Fixture.TargetPackage->IsDirty());

	// USER가 실제 Save했다고 가정한 test-only package dirty clear입니다. Disk save 자체는 수행하지 않습니다.
	Fixture.RecipePackage->SetDirtyFlag(false);
	Fixture.TargetPackage->SetDirtyFlag(false);

	// Applied current Target exact DefinitionHash입니다.
	const FString CurrentTargetHash = BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (!TestFalse(TEXT("Builder Step 8 current Target hash is populated"), CurrentTargetHash.IsEmpty()))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestEqual(
		TEXT("Builder Step 8 Recipe AppliedState matches current Target hash"),
		Fixture.Recipe->AppliedState.AppliedDefinitionHash,
		CurrentTargetHash);

	// Current synthetic benchmark run identity입니다.
	const FString CurrentRunId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	// Current Target exact object path입니다.
	const FString CurrentTargetPath = FSoftObjectPath(Fixture.TargetVehicleData).ToString();
	if (!TestTrue(
		TEXT("Builder Step 8 current benchmark result writes"),
		BenchmarkFileGuard.WriteResult(CurrentTargetPath, CurrentTargetHash, CurrentRunId, false, false, Error))
		|| !TestTrue(TEXT("Builder Step 8 refreshes current benchmark binding"), BuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(Evidence);
		return false;
	}

	// Exact current benchmark가 binding된 Step 8 projection입니다.
	const FCFVehicleBuilderStepView* BenchmarkedDrivingStep = BuilderViewModel.FindStepView(ECFVehicleBuilderStepId::DrivingTest);
	if (!TestNotNull(TEXT("Builder Step 8 benchmarked projection exists"), BenchmarkedDrivingStep))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestEqual(TEXT("Builder Step 8 remains Ready until USER Driving PASS"), BenchmarkedDrivingStep->State, ECFVehicleBuilderStepState::Ready);
	TestTrue(TEXT("Builder Step 8 accepts exact current technical benchmark"), BuilderViewModel.HasDrivingBenchmarkResult());
	TestEqual(TEXT("Builder Step 8 binds exact benchmark RunId"), BuilderViewModel.GetDrivingBenchmarkResult().RunId, CurrentRunId);
	TestEqual(TEXT("Builder Step 8 binds exact Target DefinitionHash"), BuilderViewModel.GetDrivingBenchmarkResult().ExpectedTargetDefinitionHash, CurrentTargetHash);
	TestEqual(
		TEXT("Builder Step 8 treats 0-100 unavailable as observed metric, not technical failure"),
		BuilderViewModel.GetDrivingBenchmarkResult().Metric.Acceleration0To100Seconds,
		-1.0);
	TestFalse(TEXT("Builder Step 8 runner never asserts Reference threshold"), BuilderViewModel.GetDrivingBenchmarkResult().bReferenceThresholdAsserted);
	TestFalse(TEXT("Builder Step 8 runner never asserts USER driving feel"), BuilderViewModel.GetDrivingBenchmarkResult().bUserDrivingFeelAsserted);

	// Active PIE에 선택 차량을 실제 적용하지 않은 상태에서는 USER PASS를 프로그램적으로 위조할 수 없어야 합니다.
	TestFalse(TEXT("Builder Step 8 USER PASS is blocked before active PIE test-drive preparation"), BuilderViewModel.AcceptCurrentUserDriving(Error));
	TestFalse(TEXT("Builder Step 8 has no USER acceptance before actual drive"), BuilderViewModel.HasCurrentUserDrivingAcceptance());

	// Transient fixture package는 disk에 저장되어 있지 않으므로 launch preparation이 package existence gate에서도 fail-closed해야 합니다.
	FString UnsavedPackageRunId;
	TestFalse(
		TEXT("Builder Step 8 refuses external benchmark for non-persistent test package"),
		BuilderViewModel.PrepareDrivingBenchmarkLaunch(Executable, Arguments, WorkingDirectory, UnsavedPackageRunId, Error));
	TestFalse(TEXT("Builder Step 8 non-persistent launch guard does not dirty Recipe"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Builder Step 8 non-persistent launch guard does not dirty Target"), Fixture.TargetPackage->IsDirty());

	// Production AcceptCurrentUserDriving이 실제 PIE를 요구하는 것은 위에서 검증했으므로, 여기서는 restart resume binding만 test fixture token으로 검증합니다.
	if (!TestTrue(
		TEXT("Builder Step 8 test-only exact resume token writes"),
		DrivingTokenGuard.WriteResumeToken(Fixture.Recipe->RecipeId, CurrentTargetHash, CurrentRunId, Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(Evidence);
		return false;
	}

	// Editor restart와 같은 fresh VM입니다. Product mutation approval은 복원하지 않고 exact local USER acceptance만 current truth와 비교합니다.
	FCFVehicleBuilderVM ResumedBuilderViewModel;
	// Same managed selection row입니다.
	const FCFVehicleListEntry Entry = BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Builder Step 8 fresh VM reselects same Vehicle"), ResumedBuilderViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(Evidence);
		return false;
	}

	// Exact Target hash + RunId + RecipeId가 모두 일치해 resume된 Step 8 projection입니다.
	const FCFVehicleBuilderStepView* ResumedDrivingStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::DrivingTest);
	if (!TestNotNull(TEXT("Builder Step 8 resumed projection exists"), ResumedDrivingStep))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestTrue(TEXT("Builder Step 8 exact local USER acceptance resumes"), ResumedBuilderViewModel.HasCurrentUserDrivingAcceptance());
	TestEqual(TEXT("Builder Step 8 becomes Complete only with exact benchmark-bound USER token"), ResumedDrivingStep->State, ECFVehicleBuilderStepState::Complete);

	// 같은 Target이더라도 새 benchmark RunId가 생기면 이전 USER 주행 PASS를 자동 재사용하지 않아야 합니다.
	const FString NewRunId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	if (!TestTrue(
		TEXT("Builder Step 8 new benchmark run result writes"),
		BenchmarkFileGuard.WriteResult(CurrentTargetPath, CurrentTargetHash, NewRunId, false, false, Error))
		|| !TestTrue(TEXT("Builder Step 8 refreshes new benchmark run"), ResumedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	const FCFVehicleBuilderStepView* NewRunDrivingStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::DrivingTest);
	if (!TestNotNull(TEXT("Builder Step 8 new-run projection exists"), NewRunDrivingStep))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestTrue(TEXT("Builder Step 8 new run remains technically current"), ResumedBuilderViewModel.HasDrivingBenchmarkResult());
	TestFalse(TEXT("Builder Step 8 old USER token does not cross benchmark RunId"), ResumedBuilderViewModel.HasCurrentUserDrivingAcceptance());
	TestEqual(TEXT("Builder Step 8 returns Ready for USER Driving after new run"), NewRunDrivingStep->State, ECFVehicleBuilderStepState::Ready);

	// Target DefinitionHash가 달라진 stale benchmark result입니다.
	const FString StaleTargetHash = TEXT("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
	if (!TestTrue(
		TEXT("Builder Step 8 stale Target benchmark result writes"),
		BenchmarkFileGuard.WriteResult(CurrentTargetPath, StaleTargetHash, NewRunId, false, false, Error))
		|| !TestTrue(TEXT("Builder Step 8 refreshes stale benchmark diagnostic"), ResumedBuilderViewModel.RefreshCurrentState(Error)))
	{
		AddError(Error);
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	const FCFVehicleBuilderStepView* StaleDrivingStep = ResumedBuilderViewModel.FindStepView(ECFVehicleBuilderStepId::DrivingTest);
	if (!TestNotNull(TEXT("Builder Step 8 stale projection exists"), StaleDrivingStep))
	{
		CleanupRegisteredAsset(Evidence);
		return false;
	}
	TestFalse(TEXT("Builder Step 8 rejects benchmark bound to another Target hash"), ResumedBuilderViewModel.HasDrivingBenchmarkResult());
	TestEqual(TEXT("Builder Step 8 marks stale benchmark for rerun"), StaleDrivingStep->State, ECFVehicleBuilderStepState::Stale);

	// Asset Registry에 등록된 unsaved test Evidence를 후속 Automation에서 보이지 않도록 제거합니다.
	CleanupRegisteredAsset(Evidence);
	return true;
}

// Prepared R3 approval 뒤 Recipe state가 바뀌면 silent re-preview/retry 없이 Target mutation0으로 차단되는지 검증합니다.
bool FCFVehicleWorkspaceStaleTest::RunTest(const FString& Parameters)
{
	// Managed Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Workspace stale fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*Fixture.Recipe);
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Apply 전 Target Definition hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// 실제 Slate가 사용하는 transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Workspace stale ViewModel selects Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// Reviewed exact R3 preparation result입니다.
	FCFAuthoringOpResult PrepareResult;
	if (!TestTrue(TEXT("Workspace stale test prepares Apply approval"), ViewModel.PrepareApply(PrepareResult)))
	{
		AddError(PrepareResult.Message);
		return false;
	}

	// Approval 뒤 발생한 concurrent Recipe semantic edit입니다. Automation fixture setup이며 production UI writer가 아닙니다.
	Fixture.Recipe->HardpointIntents[0].SocketName = TEXT("HP_Top_Old");
	++Fixture.Recipe->AuthoringRevision;

	// Stale exact approval execution terminal result입니다.
	FCFAuthoringOpResult ApplyResult;
	TestFalse(TEXT("Stale prepared Apply is blocked"), ViewModel.ExecutePreparedApply(ApplyResult));
	TestTrue(TEXT("Stale Apply does not report success"), ApplyResult.Status != ECFAuthoringOpStatus::Succeeded);
	TestFalse(TEXT("Stale Apply target mutation remains zero"), ApplyResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Stale Apply automatic retry remains zero"), ApplyResult.Mutation.bAutomaticRetryPerformed);
	TestFalse(TEXT("Stale Apply never saves"), ApplyResult.Mutation.bSavePerformed);
	TestEqual(TEXT("Stale Apply leaves Target Definition unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("ViewModel marks stale prepared preview OutOfDate"), ViewModel.GetPreviewView(), ECFWorkspacePreviewView::OutOfDate);
	TestFalse(TEXT("Stale prepared approval is consumed, not retried"), ViewModel.HasPreparedApply());
	return true;
}




// Reference Vehicle Compare가 117-field projection을 read-only로 사용하고 Reference 차이를 정확히 노출하는지 검증합니다.
bool FCFVehicleP10ReferenceTest::RunTest(const FString& Parameters)
{
	// Current managed Vehicle fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture CurrentFixture;
	// Reference raw Definition fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture ReferenceFixture;
	// Fixture/compare diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-10 current fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(CurrentFixture, Error))
		|| !TestTrue(TEXT("P0-10 reference fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(ReferenceFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Reference raw Definition에만 존재하는 deterministic difference입니다.
	ReferenceFixture.TargetVehicleData->BaseVehicleMassKg = 1660.0f;
	CurrentFixture.TargetPackage->SetDirtyFlag(false);
	CurrentFixture.RecipePackage->SetDirtyFlag(false);
	ReferenceFixture.TargetPackage->SetDirtyFlag(false);
	ReferenceFixture.RecipePackage->SetDirtyFlag(false);

	// 실제 Workspace transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Current managed selection row입니다.
	const FCFVehicleListEntry CurrentEntry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(CurrentFixture, true);
	if (!TestTrue(TEXT("P0-10 current managed Vehicle selects"), ViewModel.SelectVehicle(CurrentEntry, Error)))
	{
		AddError(Error);
		return false;
	}
	// Reference는 RecipePath를 의도적으로 생략해 Current Definition mode로 비교합니다.
	const FCFVehicleListEntry ReferenceEntry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(ReferenceFixture, false);
	if (!TestTrue(TEXT("P0-10 Reference Vehicle selects"), ViewModel.SelectReferenceVehicle(ReferenceEntry, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Reference Compare returns Stable Field rows"), ViewModel.GetReferenceCompareResult().Rows.Num() > 0);
	TestTrue(TEXT("Reference mass difference is detected"), ViewModel.GetReferenceCompareResult().DifferentCount > 0);
	TestTrue(TEXT("Different-only Reference Compare refresh succeeds"), ViewModel.RefreshReferenceCompare(true, Error));
	TestEqual(TEXT("Different-only result contains only different rows"), ViewModel.GetReferenceCompareResult().Rows.Num(), ViewModel.GetReferenceCompareResult().DifferentCount);
	TestFalse(TEXT("Reference Compare does not dirty current Target"), CurrentFixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Reference Compare does not dirty current Recipe"), CurrentFixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Reference Compare does not dirty reference Target"), ReferenceFixture.TargetPackage->IsDirty());
	return true;
}

// Typed Assets/Layout와 Frozen 4축 Driving Feel 변경이 Recipe-only이며 Workspace standard Undo로 복원되는지 검증합니다.
bool FCFVehicleP10SemanticUndoTest::RunTest(const FString& Parameters)
{
	// Managed Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-10 semantic fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Recipe-only semantic changes 전 Target full hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Original AssetIntent입니다.
	const FCFVehicleAssetIntent OriginalAssetIntent = Fixture.Recipe->AssetIntent;
	// Original Driving Feel semantic axes입니다.
	const FCFVehicleFeelIntent OriginalFeel = Fixture.Recipe->DrivingFeelIntent;

	// 실제 Workspace transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("P0-10 semantic ViewModel selects Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// Same required sockets를 가진 alternate transient chassis입니다.
	UStaticMesh* AlternateChassis = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
	TestNotNull(TEXT("Alternate layout chassis created"), AlternateChassis);
	if (!AlternateChassis)
	{
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::AddSocket(*AlternateChassis, TEXT("Wheel_Anchor_FL"), FVector(112.0, -62.0, 28.0));
	CFVehicleAuthoringVMTestsPrivate::AddSocket(*AlternateChassis, TEXT("Wheel_Anchor_FR"), FVector(112.0, 62.0, 28.0));
	CFVehicleAuthoringVMTestsPrivate::AddSocket(*AlternateChassis, TEXT("Wheel_Anchor_RL"), FVector(-106.0, -62.0, 28.0));
	CFVehicleAuthoringVMTestsPrivate::AddSocket(*AlternateChassis, TEXT("Wheel_Anchor_RR"), FVector(-106.0, 62.0, 28.0));
	CFVehicleAuthoringVMTestsPrivate::AddSocket(*AlternateChassis, TEXT("HP_Top_Old"), FVector(0.0, 0.0, 82.0));
	CFVehicleAuthoringVMTestsPrivate::AddSocket(*AlternateChassis, TEXT("HP_Front_New"), FVector(128.0, 0.0, 44.0));
	// Alternate chassis intent입니다.
	FCFVehicleAssetIntent AlternateIntent = OriginalAssetIntent;
	AlternateIntent.ChassisMesh = AlternateChassis;
	// Typed Layout semantic commit result입니다.
	FCFAuthoringOpResult AssetCommit;
	TestTrue(TEXT("Typed Asset/Layout intent commit succeeds"), ViewModel.CommitAssetIntent(AlternateIntent, AssetCommit));
	TestTrue(TEXT("Asset/Layout commit changes Recipe"), AssetCommit.Mutation.bRecipeChanged);
	TestEqual(TEXT("Asset/Layout commit leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestTrue(TEXT("Workspace exposes standard Undo after Layout commit"), ViewModel.CanUndoLastWorkspaceAction());
	TestTrue(TEXT("Workspace standard Undo restores Layout Recipe"), ViewModel.UndoLastWorkspaceAction(Error));
	TestEqual(TEXT("Undo restores original Chassis intent"), Fixture.Recipe->AssetIntent.ChassisMesh.ToSoftObjectPath(), OriginalAssetIntent.ChassisMesh.ToSoftObjectPath());
	TestEqual(TEXT("Undo still leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);

	// Frozen Sports preset typed semantic commit result입니다.
	FCFAuthoringOpResult FeelCommit;
	TestTrue(TEXT("Sports 4-axis preset commit succeeds"), ViewModel.CommitDrivingFeelPreset(TEXT("Sports"), FeelCommit));
	TestEqual(TEXT("Sports Acceleration exact"), Fixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.85f);
	TestEqual(TEXT("Sports Steering exact"), Fixture.Recipe->DrivingFeelIntent.SteeringAgility, 0.78f);
	TestEqual(TEXT("Sports Grip exact"), Fixture.Recipe->DrivingFeelIntent.GripFeel, 0.82f);
	TestEqual(TEXT("Sports Suspension exact"), Fixture.Recipe->DrivingFeelIntent.SuspensionFirmness, 0.78f);
	TestEqual(TEXT("Driving Feel commit leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestTrue(TEXT("Workspace standard Undo restores Driving Feel"), ViewModel.UndoLastWorkspaceAction(Error));
	TestEqual(TEXT("Undo restores original Acceleration"), Fixture.Recipe->DrivingFeelIntent.AccelerationFeel, OriginalFeel.AccelerationFeel);
	TestEqual(TEXT("Undo restores original Steering"), Fixture.Recipe->DrivingFeelIntent.SteeringAgility, OriginalFeel.SteeringAgility);
	TestEqual(TEXT("Undo restores original Grip"), Fixture.Recipe->DrivingFeelIntent.GripFeel, OriginalFeel.GripFeel);
	TestEqual(TEXT("Undo restores original Suspension"), Fixture.Recipe->DrivingFeelIntent.SuspensionFirmness, OriginalFeel.SuspensionFirmness);
	return true;
}



// Wheel Measurement와 Legacy Adoption이 review 전 mutation0, commit 시 Recipe-only, Undo 가능 경계를 지키는지 검증합니다.
bool FCFVehicleP10AdoptionTest::RunTest(const FString& Parameters)
{
	// Managed Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-10 adoption fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Measurement/Adoption 전 Target Definition hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Measurement/Adoption 전 Recipe fingerprint입니다.
	const FString RecipeFingerprintBefore = CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Adoption 전 exact legacy pin count입니다.
	const int32 LegacyPinCountBefore = Fixture.Recipe->ImportState.LegacyPinnedFields.Num();
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// 실제 Workspace transient ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("P0-10 adoption ViewModel selects Vehicle"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Wheel measurement proposals exist"), ViewModel.GetMeasurementResult().Proposals.Num() > 0);
	if (ViewModel.GetMeasurementResult().Proposals.IsEmpty())
	{
		return false;
	}
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// 첫 exact current Resolver measurement proposal입니다.
	const FCFVehicleMeasurementProposal MeasurementProposal = ViewModel.GetMeasurementResult().Proposals[0];
	// Mutation0 reviewed measurement prospective result입니다.
	FCFVehicleMeasurementPreviewResult MeasurementPreview;
	TestTrue(
		TEXT("Measurement decision preview succeeds"),
		ViewModel.PrepareMeasurementDecision(MeasurementProposal, ECFVehicleMeasureDecision::AcceptMeasuredValue, MeasurementPreview));
	TestFalse(TEXT("Measurement preview does not dirty Recipe"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Measurement preview does not dirty Target"), Fixture.TargetPackage->IsDirty());
	TestEqual(TEXT("Measurement preview leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);

	// Reviewed R2 Recipe-only measurement commit result입니다.
	FCFAuthoringOpResult MeasurementCommit;
	if (!TestTrue(TEXT("Measurement decision commit succeeds"), ViewModel.ExecutePreparedMeasurement(MeasurementCommit)))
	{
		AddError(MeasurementCommit.Message);
		return false;
	}
	TestTrue(TEXT("Measurement commit reports Recipe changed"), MeasurementCommit.Mutation.bRecipeChanged);
	TestFalse(TEXT("Measurement commit reports Target unchanged"), MeasurementCommit.Mutation.bTargetChanged);
	TestFalse(TEXT("Measurement commit never auto-saves"), MeasurementCommit.Mutation.bSavePerformed);
	TestFalse(TEXT("Measurement commit never automatic retries"), MeasurementCommit.Mutation.bAutomaticRetryPerformed);
	TestEqual(TEXT("Measurement commit leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestNotEqual(TEXT("Measurement commit changes Recipe fingerprint"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestTrue(TEXT("Measurement commit enables Workspace Undo"), ViewModel.CanUndoLastWorkspaceAction());
	TestTrue(TEXT("Measurement Workspace Undo succeeds"), ViewModel.UndoLastWorkspaceAction(Error));
	TestEqual(TEXT("Measurement Undo restores Recipe fingerprint"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Measurement Undo leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);

	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	// Mutation0 Hardpoints ownership transition preview입니다.
	FCFVehicleAdoptionPreviewResult AdoptionPreview;
	if (!TestTrue(TEXT("Hardpoints Adoption preview succeeds"), ViewModel.PrepareGroupAdoption(ECFVehicleAdoptGroup::Hardpoints, AdoptionPreview)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	TestTrue(TEXT("Hardpoints Adoption preview has pins to remove"), AdoptionPreview.Preview.LegacyPinPathsToRemove.Num() > 0);
	TestEqual(TEXT("Adoption preview does not change persistent pin count"), Fixture.Recipe->ImportState.LegacyPinnedFields.Num(), LegacyPinCountBefore);
	TestFalse(TEXT("Adoption preview does not dirty Recipe"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Adoption preview does not dirty Target"), Fixture.TargetPackage->IsDirty());

	// Reviewed R2 Existing Import Core Recipe-only adoption result입니다.
	FCFAuthoringOpResult AdoptionCommit;
	if (!TestTrue(TEXT("Hardpoints Adoption commit succeeds"), ViewModel.ExecutePreparedAdoption(AdoptionCommit)))
	{
		AddError(AdoptionCommit.Message);
		return false;
	}
	TestTrue(TEXT("Adoption commit reports Recipe changed"), AdoptionCommit.Mutation.bRecipeChanged);
	TestFalse(TEXT("Adoption commit reports Target unchanged"), AdoptionCommit.Mutation.bTargetChanged);
	TestFalse(TEXT("Adoption commit never auto-saves"), AdoptionCommit.Mutation.bSavePerformed);
	TestFalse(TEXT("Adoption commit never automatic retries"), AdoptionCommit.Mutation.bAutomaticRetryPerformed);
	TestTrue(TEXT("Adoption commit removes Legacy pins"), Fixture.Recipe->ImportState.LegacyPinnedFields.Num() < LegacyPinCountBefore);
	TestEqual(TEXT("Adoption commit leaves Target hash unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestTrue(TEXT("Adoption commit enables Workspace Undo"), ViewModel.CanUndoLastWorkspaceAction());
	TestTrue(TEXT("Adoption Workspace Undo succeeds"), ViewModel.UndoLastWorkspaceAction(Error));
	TestEqual(TEXT("Adoption Undo restores Legacy pin count"), Fixture.Recipe->ImportState.LegacyPinnedFields.Num(), LegacyPinCountBefore);
	return true;
}

// Legacy Wizard가 managed Target에서는 변경 동작을 끄고 unmanaged Target에서는 기존 경로를 보존하며 두 탭이 공존하는지 검증합니다.
bool FCFVehicleP10LegacyGuardTest::RunTest(const FString& Parameters)
{
	// Managed Target + Recipe fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture ManagedFixture;
	// Unmanaged Target-only fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture UnmanagedFixture;
	// Fixture diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-10 managed guard fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(ManagedFixture, Error))
		|| !TestTrue(TEXT("P0-10 unmanaged guard fixture builds"), CFVehicleAuthoringVMTestsPrivate::ConfigureValidTarget(UnmanagedFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	ManagedFixture.TargetPackage->SetDirtyFlag(false);
	ManagedFixture.RecipePackage->SetDirtyFlag(false);
	UnmanagedFixture.TargetPackage->SetDirtyFlag(false);

	// Production legacy Wizard의 managed-target guard를 읽을 test instance입니다.
	SCFVDAWizardTab ManagedWizard;
	FCFVDAWizardTestAccess::SetTarget(ManagedWizard, ManagedFixture.TargetVehicleData);
	TestTrue(TEXT("Legacy Wizard detects managed Authoring Recipe"), FCFVDAWizardTestAccess::HasManagedRecipe(ManagedWizard));
	TestFalse(TEXT("Managed legacy Layout action is disabled"), FCFVDAWizardTestAccess::CanCaptureLayout(ManagedWizard));
	TestFalse(TEXT("Managed legacy Quick Tune action is disabled"), FCFVDAWizardTestAccess::CanApplyDrivingFeel(ManagedWizard));
	TestFalse(TEXT("Managed legacy memory Revert action is disabled"), FCFVDAWizardTestAccess::CanRevertDrivingFeel(ManagedWizard));

	// Recipe가 없는 Existing Definition용 legacy Wizard instance입니다.
	SCFVDAWizardTab UnmanagedWizard;
	FCFVDAWizardTestAccess::SetTarget(UnmanagedWizard, UnmanagedFixture.TargetVehicleData);
	TestFalse(TEXT("Legacy Wizard leaves unmanaged Target unmanaged"), FCFVDAWizardTestAccess::HasManagedRecipe(UnmanagedWizard));
	TestTrue(TEXT("Unmanaged legacy Layout action remains available"), FCFVDAWizardTestAccess::CanCaptureLayout(UnmanagedWizard));
	TestTrue(TEXT("Unmanaged legacy Quick Tune action remains available"), FCFVDAWizardTestAccess::CanApplyDrivingFeel(UnmanagedWizard));

	// P0-10에서도 신규/기존 Nomad Tab registration이 동시에 유지되는지 확인합니다.
	const FName VehicleAuthoringTabName(TEXT("CarFight.VehicleAuthoring"));
	const FName VehicleWizardTabName(TEXT("CarFight.VehicleDAWizard"));
	TestTrue(TEXT("P0-10 Vehicle Authoring tab remains registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleAuthoringTabName));
	TestTrue(TEXT("P0-10 legacy Vehicle DA Wizard remains registered"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleWizardTabName));
	TestFalse(TEXT("Managed guard read does not dirty Target"), ManagedFixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Managed guard read does not dirty Recipe"), ManagedFixture.RecipePackage->IsDirty());
		TestFalse(TEXT("Unmanaged guard read does not dirty Target"), UnmanagedFixture.TargetPackage->IsDirty());
	return true;
}

// 대표 managed fixture에서 Recipe edit, fresh preview, trace/diff/validation, Apply/Undo, raw drift fail-closed를 연속 검증합니다.
bool FCFVehicleP11AuthoringE2ETest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// P0-11 대표 managed Vehicle/Recipe fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture와 fresh-read diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-11 representative fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// 기존 ApplyService가 이미 검증한 structural difference를 대표 pending authoring state로 구성합니다.
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*Fixture.Recipe);
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	// Authoring 적용 전 canonical Target hash입니다.
	const FString TargetHashBeforeApply = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (!TestFalse(TEXT("Pre-apply Target hash exists"), TargetHashBeforeApply.IsEmpty()))
	{
		AddError(Error);
		return false;
	}

	// 실제 Workspace ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed representative selection입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Representative managed Vehicle selects"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Initial representative preview is fresh"), ViewModel.GetPreviewView() == ECFWorkspacePreviewView::Fresh);
	TestTrue(TEXT("Initial representative pending diff exists"), ViewModel.GetDiffResult().FieldDiff.Num() > 0);
	TestTrue(TEXT("Initial representative source trace exists"), ViewModel.GetTraceResult().SourceTrace.Num() > 0);
	TestTrue(TEXT("Initial representative validation allows Apply"), ViewModel.CanApply());

		// Recipe edit 전에 준비한 old R3 approval의 typed operation result입니다.
	FCFAuthoringOpResult OldPrepareResult;
	if (!TestTrue(TEXT("Old Apply approval can be prepared"), ViewModel.PrepareApply(OldPrepareResult)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	TestTrue(TEXT("Old Apply approval is stored"), ViewModel.HasPreparedApply());

	// Frozen typed Recipe semantic edit 결과입니다.
	FCFAuthoringOpResult RecipeEditResult;
	if (!TestTrue(TEXT("Typed Recipe edit succeeds"), ViewModel.CommitDrivingFeelPreset(TEXT("Sports"), RecipeEditResult)))
	{
		AddError(RecipeEditResult.Message);
		return false;
	}
	TestTrue(TEXT("Typed Recipe edit is Recipe mutation"), RecipeEditResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Typed Recipe edit is not Target mutation"), RecipeEditResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Typed Recipe edit invalidates old Apply approval"), ViewModel.HasPreparedApply());
	TestEqual(TEXT("Typed Recipe edit leaves Target unchanged"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBeforeApply);
	TestTrue(TEXT("Recipe edit automatically returns a fresh preview"), ViewModel.GetPreviewView() == ECFWorkspacePreviewView::Fresh);
	TestTrue(TEXT("Fresh post-edit Diff remains reviewable"), ViewModel.GetDiffResult().FieldDiff.Num() > 0);
	TestTrue(TEXT("Fresh post-edit Source Trace remains populated"), ViewModel.GetTraceResult().SourceTrace.Num() > 0);
	TestTrue(TEXT("Fresh post-edit Validation allows normal Apply"), ViewModel.CanApply());

		// Fresh reviewed R3 approval의 typed operation result입니다.
	FCFAuthoringOpResult FreshPrepareResult;
	if (!TestTrue(TEXT("Fresh Apply approval can be prepared"), ViewModel.PrepareApply(FreshPrepareResult)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
		TestNotEqual(TEXT("Fresh approval binds changed Recipe fingerprint"), FreshPrepareResult.CurrentRecipeFingerprint, OldPrepareResult.CurrentRecipeFingerprint);


	// Shared ApplyService lane terminal result입니다.
	FCFAuthoringOpResult FirstApplyResult;
	if (!TestTrue(TEXT("Representative explicit Apply succeeds"), ViewModel.ExecutePreparedApply(FirstApplyResult)))
	{
		AddError(FirstApplyResult.Message);
		return false;
	}
	TestTrue(TEXT("Apply reports Target mutation"), FirstApplyResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Apply never auto-saves"), FirstApplyResult.Mutation.bSavePerformed);
	TestFalse(TEXT("Apply never automatically retries"), FirstApplyResult.Mutation.bAutomaticRetryPerformed);
	// First Apply 후 exact current Target hash입니다.
	const FString FirstAppliedTargetHash = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
		TestNotEqual(TEXT("Apply changes full Target hash"), FirstAppliedTargetHash, TargetHashBeforeApply);
	TestFalse(TEXT("Apply returns resolved projection hash"), FirstApplyResult.CurrentResolvedDefinitionHash.IsEmpty());
	TestEqual(TEXT("AppliedState binds reviewed resolved projection hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, FirstApplyResult.CurrentResolvedDefinitionHash);
	TestEqual(TEXT("Post-Apply pending diff is empty"), ViewModel.GetDiffResult().FieldDiff.Num(), 0);
	TestTrue(TEXT("Workspace owns last Apply transaction for Undo"), ViewModel.CanUndoLastWorkspaceAction());

	// P0-11 coherent Workspace Undo입니다.
	if (!TestTrue(TEXT("Workspace Apply Undo succeeds"), ViewModel.UndoLastWorkspaceAction(Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Undo restores pre-Apply Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBeforeApply);
	TestTrue(TEXT("Undo restores pending authoring diff"), ViewModel.GetDiffResult().FieldDiff.Num() > 0);
	TestTrue(TEXT("Undo leaves fresh preview"), ViewModel.GetPreviewView() == ECFWorkspacePreviewView::Fresh);
	TestTrue(TEXT("Undo-restored state can be applied again"), ViewModel.CanApply());

		// Raw drift 검증을 위해 같은 fresh reviewed state를 다시 적용합니다.
	FCFAuthoringOpResult ReapplyPrepareResult;
	if (!TestTrue(TEXT("Reapply approval can be prepared"), ViewModel.PrepareApply(ReapplyPrepareResult)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	FCFAuthoringOpResult ReapplyResult;
	if (!TestTrue(TEXT("Reapply succeeds"), ViewModel.ExecutePreparedApply(ReapplyResult)))
	{
		AddError(ReapplyResult.Message);
		return false;
	}
		// Raw edit 전 Recipe fingerprint는 silent absorption 여부를 판정하는 기준입니다.
	const FString RecipeFingerprintBeforeRawDrift = CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Raw edit 전 AppliedState hash는 Last Applied resolver projection 기준입니다.
	const FString AppliedHashBeforeRawDrift = Fixture.Recipe->AppliedState.AppliedDefinitionHash;
	// Raw edit 전 full Current Definition hash입니다.
	const FString FullTargetHashBeforeRawDrift = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Advanced Escape Hatch와 동일하게 Authoring facade 밖에서 발생한 raw Definition change를 재현합니다.
	Fixture.TargetVehicleData->BaseVehicleMassKg += 7.0f;
	Fixture.TargetVehicleData->MarkPackageDirty();
	// Raw edit 직후 canonical current Definition hash입니다.
		const FString RawDriftTargetHash = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	TestNotEqual(TEXT("Raw edit changes current full Target hash"), RawDriftTargetHash, FullTargetHashBeforeRawDrift);

	if (!TestTrue(TEXT("Raw edit refresh succeeds as reviewed drift state"), ViewModel.RefreshPreview(Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Raw edit becomes External Drift"), ViewModel.GetResolveResult().ResolveResult.StaleReport.bHasExternalDrift);
	TestTrue(TEXT("Workspace sync state is External Drift"), ViewModel.GetSyncView() == ECFWorkspaceSyncView::ExternalDrift);
	TestFalse(TEXT("External Drift disables normal Apply"), ViewModel.CanApply());
		// Drift 상태에서 normal Apply 준비를 시도한 typed blocked result입니다.
	FCFAuthoringOpResult DriftPrepareResult;
	TestFalse(TEXT("External Drift blocks normal Apply preparation"), ViewModel.PrepareApply(DriftPrepareResult));
	TestEqual(TEXT("Drift review never silently mutates Recipe fingerprint"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBeforeRawDrift);
	TestEqual(TEXT("Drift review preserves Last Applied hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, AppliedHashBeforeRawDrift);
	TestEqual(TEXT("Drift review preserves raw Target state"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), RawDriftTargetHash);
	return true;
}

// Authoring Apply 결과 VehicleData를 기존 Fitting과 Inventory consumer가 별도 Authoring 경로 없이 그대로 소비하는지 검증합니다.
bool FCFVehicleP11ConsumerTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// P0-11 consumer regression에 사용할 managed fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/apply diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-11 consumer fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*Fixture.Recipe);
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Authoring Apply를 수행할 production Workspace ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed consumer fixture selection입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Consumer fixture selects"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
		// Exact reviewed Apply 준비의 typed operation result입니다.
	FCFAuthoringOpResult PrepareResult;
	if (!TestTrue(TEXT("Consumer fixture Apply prepares"), ViewModel.PrepareApply(PrepareResult)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	// Production shared Apply lane result입니다.
	FCFAuthoringOpResult ApplyResult;
	if (!TestTrue(TEXT("Consumer fixture Authoring Apply succeeds"), ViewModel.ExecutePreparedApply(ApplyResult)))
	{
		AddError(ApplyResult.Message);
		return false;
	}
	TestEqual(TEXT("Applied Vehicle has one semantic Hardpoint"), Fixture.TargetVehicleData->HardpointSlots.Num(), 1);
	TestEqual(TEXT("Applied Vehicle Hardpoint identity"), Fixture.TargetVehicleData->HardpointSlots[0].LocationSlotId, FName(TEXT("Front_New")));
	TestEqual(TEXT("Applied Vehicle has one semantic MountProfile"), Fixture.TargetVehicleData->MountProfiles.Num(), 1);
		TestEqual(TEXT("Applied Vehicle Mount identity"), Fixture.TargetVehicleData->MountProfiles[0].MountProfileId, FName(TEXT("M_Front_New")));
	// Consumer read 시작 직전 full Current Definition hash입니다.
	const FString TargetHashBeforeConsumerReads = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);

	// 기존 Fitting consumer가 동일한 applied VehicleData pointer를 읽는 transient fitting입니다.
	UCFVehicleFittingData* FittingData = NewObject<UCFVehicleFittingData>(GetTransientPackage());
	TestNotNull(TEXT("Transient Fitting consumer created"), FittingData);
	if (!FittingData)
	{
		return false;
	}
	FittingData->FittingId = TEXT("DAUTH_P0_11_FittingConsumer");
	FittingData->VehicleData = Fixture.TargetVehicleData;
	FittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsEmpty;
	FittingData->DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;
	// Existing Fitting consumer의 pure VehicleData snapshot입니다.
	const FCFVehicleFittingSnapshot FittingSnapshot = FittingData->BuildFittingSnapshot();
	TestTrue(TEXT("Existing Fitting consumer accepts Authoring-applied VehicleData"), FittingSnapshot.IsValid());
	TestTrue(TEXT("Fitting consumer keeps exact applied VehicleData pointer"), FittingSnapshot.VehicleData == Fixture.TargetVehicleData);
	TestEqual(TEXT("Fitting consumer reads applied BaseVehicleMassKg"), FittingSnapshot.BaseVehicleMassKg, Fixture.TargetVehicleData->BaseVehicleMassKg);
	TestEqual(TEXT("Fitting consumer reads applied MountProfile count"), FittingSnapshot.ResolvedMounts.Num(), 1);
	if (FittingSnapshot.ResolvedMounts.Num() == 1)
	{
		TestEqual(TEXT("Fitting consumer resolves applied MountProfileId"), FittingSnapshot.ResolvedMounts[0].MountProfileId, FName(TEXT("M_Front_New")));
		TestEqual(TEXT("Fitting consumer resolves applied Hardpoint LocationSlotId"), FittingSnapshot.ResolvedMounts[0].LocationSlotId, FName(TEXT("Front_New")));
	}

		// Inventory Fit Adapter가 요구하는 valid current vehicle owner identity입니다. Runtime helper export를 늘리지 않도록 value를 test에서 직접 구성합니다.
	FCFInventoryOwnerId InventoryOwnerId;
	InventoryOwnerId.Value = FGuid::NewGuid();
	// Item selection이 없는 consumer regression에서는 empty-but-valid Container 집합을 사용합니다.
	TArray<FCFInventoryContainerState> EmptyContainers;
	// Item selection이 없으므로 Definition 목록도 비어 있습니다.
	TArray<UCFInventoryItemData*> EmptyDefinitions;
	// Active reservation이 없는 default ledger입니다.
	FCFInventoryTransferLedger EmptyLedger;
	// Existing Inventory→Fitting adapter request입니다.
	FCFInventoryFitRequest InventoryFitRequest;
	InventoryFitRequest.FittingId = TEXT("DAUTH_P0_11_InventoryConsumer");
	InventoryFitRequest.VehicleData = Fixture.TargetVehicleData;
	InventoryFitRequest.MissingMountSelectionPolicy = ECFMissingMountPolicy::TreatAsEmpty;
		InventoryFitRequest.DefenseSelection.SelectionMode = ECFDefenseSelectionMode::ExplicitNone;
	InventoryFitRequest.AccessContext.CurrentVehicleOwnerId = InventoryOwnerId;
	// Existing Inventory adapter result from the Authoring-applied canonical VehicleData입니다.
	const FCFInventoryFitResult InventoryFitResult = FCFInventoryFitAdapter::BuildFittingBinding(
		EmptyContainers,
		EmptyDefinitions,
		EmptyLedger,
		InventoryFitRequest);
		TestEqual(TEXT("Inventory consumer status remains Success"), InventoryFitResult.AdapterStatus, ECFInventoryFitStatus::Success);
	TestTrue(TEXT("Inventory Fit Adapter produces valid fitting snapshot"), InventoryFitResult.FittingSnapshot.IsValid());
	TestTrue(TEXT("Inventory consumer fitting snapshot keeps applied VehicleData"), InventoryFitResult.FittingSnapshot.VehicleData == Fixture.TargetVehicleData);
		TestEqual(TEXT("Inventory consumer reads applied MountProfile"), InventoryFitResult.FittingSnapshot.ResolvedMounts.Num(), 1);
	TestEqual(TEXT("Fitting/Inventory consumer reads do not mutate full Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBeforeConsumerReads);
	TestEqual(TEXT("AppliedState remains resolver projection hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, ApplyResult.CurrentResolvedDefinitionHash);
	TestFalse(TEXT("Consumer reads never auto-save"), ApplyResult.Mutation.bSavePerformed);
	return true;
}


// Frozen 24.90 normal Workspace에서 Handling/Performance ownership Adoption이 기존 Import Core를 통해 실제 도달 가능한지 검증합니다.
bool FCFVehicleP11AdoptionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Handling/Performance Legacy Pin을 가진 imported managed fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture와 adoption diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-11 adoption completeness fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Adoption 전 canonical Target hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Normal Workspace가 사용하는 actual ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Managed selection row입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("P0-11 adoption completeness Vehicle selects"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Handling group mutation0 ownership preview입니다.
	FCFVehicleAdoptionPreviewResult HandlingPreview;
	if (!TestTrue(TEXT("Handling Adoption preview succeeds"), ViewModel.PrepareGroupAdoption(ECFVehicleAdoptGroup::Handling, HandlingPreview)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	TestTrue(TEXT("Handling Adoption has reviewed Legacy pins"), HandlingPreview.Preview.LegacyPinPathsToRemove.Num() > 0);
	TestFalse(TEXT("Handling preview does not dirty Recipe"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Handling preview does not dirty Target"), Fixture.TargetPackage->IsDirty());
	// Reviewed Handling ownership commit입니다.
	FCFAuthoringOpResult HandlingCommit;
	if (!TestTrue(TEXT("Handling Adoption commit succeeds"), ViewModel.ExecutePreparedAdoption(HandlingCommit)))
	{
		AddError(HandlingCommit.Message);
		return false;
	}
	TestTrue(TEXT("Handling Adoption reports Recipe mutation"), HandlingCommit.Mutation.bRecipeChanged);
	TestFalse(TEXT("Handling Adoption never mutates Target"), HandlingCommit.Mutation.bTargetChanged);
	TestFalse(TEXT("Handling Adoption never auto-saves"), HandlingCommit.Mutation.bSavePerformed);
	TestTrue(TEXT("Handling group becomes adopted"), Fixture.Recipe->ImportState.AdoptedGroups.Contains(ECFVehicleAdoptGroup::Handling));
	TestEqual(TEXT("Handling Adoption preserves Target"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);

	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	// Performance group mutation0 ownership preview입니다.
	FCFVehicleAdoptionPreviewResult PerformancePreview;
	if (!TestTrue(TEXT("Performance Adoption preview succeeds"), ViewModel.PrepareGroupAdoption(ECFVehicleAdoptGroup::Performance, PerformancePreview)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
	TestTrue(TEXT("Performance Adoption has reviewed Legacy pins"), PerformancePreview.Preview.LegacyPinPathsToRemove.Num() > 0);
	TestFalse(TEXT("Performance preview does not dirty Recipe"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Performance preview does not dirty Target"), Fixture.TargetPackage->IsDirty());
	// Reviewed Performance ownership commit입니다.
	FCFAuthoringOpResult PerformanceCommit;
	if (!TestTrue(TEXT("Performance Adoption commit succeeds"), ViewModel.ExecutePreparedAdoption(PerformanceCommit)))
	{
		AddError(PerformanceCommit.Message);
		return false;
	}
	TestTrue(TEXT("Performance Adoption reports Recipe mutation"), PerformanceCommit.Mutation.bRecipeChanged);
	TestFalse(TEXT("Performance Adoption never mutates Target"), PerformanceCommit.Mutation.bTargetChanged);
	TestFalse(TEXT("Performance Adoption never auto-saves"), PerformanceCommit.Mutation.bSavePerformed);
		TestTrue(TEXT("Performance group becomes adopted"), Fixture.Recipe->ImportState.AdoptedGroups.Contains(ECFVehicleAdoptGroup::Performance));
	TestEqual(TEXT("Performance Adoption preserves Target"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	return true;
}

// UA-07 검증용 temporary Profile binding을 explicit R1 unbind로 원래 없음 상태까지 baseline-safe 복구하는지 검증합니다.
bool FCFVehicleP12ProfileUnbindTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Bind/Unbind를 수행할 managed Vehicle Workspace fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture와 Target hash 계산 diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-12 Profile Unbind fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Temporary Handling Profile을 소유하는 저장하지 않는 /Temp package입니다.
	UPackage* ProfilePackage = CFVehicleAuthoringVMTestsPrivate::CreateTestPackage(TEXT("CFP12UnbindHandling"));
	// Bind 후 Unbind할 existing Profile fixture입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(ProfilePackage, TEXT("DA_P12UnbindHandling_Test"), RF_Transactional);
	if (!TestNotNull(TEXT("P0-12 temporary Handling Profile creates"), HandlingProfile))
	{
		return false;
	}
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = Fixture.TargetVehicleData->VehicleMovementConfig.FrontWheelMaxBrakeTorque;
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	ProfilePackage->SetDirtyFlag(false);

	// Recipe-only Bind/Unbind 전 canonical Target hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// 실제 Workspace facade route를 사용하는 ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// 아직 Handling Profile이 연결되지 않은 managed selection입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("P0-12 Profile Unbind vehicle selects"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}
	TestFalse(TEXT("Handling baseline starts unbound"), ViewModel.GetBoundProfilePath(ECFVehicleProfileDomain::Handling).IsValid());

	// Existing reviewed BindVehicleProfile R1 transaction 결과입니다.
	FCFAuthoringOpResult BindResult;
	if (!TestTrue(TEXT("Temporary Handling binding succeeds"), ViewModel.CommitProfileBinding(
		ECFVehicleProfileDomain::Handling,
		FSoftObjectPath(HandlingProfile),
		BindResult)))
	{
		AddError(BindResult.Message);
		return false;
	}
	TestTrue(TEXT("Temporary binding changes Recipe"), BindResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Temporary binding does not change Target"), BindResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Temporary binding does not change Profile"), BindResult.Mutation.bProfileChanged);
	TestFalse(TEXT("Temporary binding does not save"), BindResult.Mutation.bSavePerformed);
	TestEqual(TEXT("Handling binding becomes exact temporary Profile"), ViewModel.GetBoundProfilePath(ECFVehicleProfileDomain::Handling), FSoftObjectPath(HandlingProfile));
	TestEqual(TEXT("Temporary binding preserves Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestFalse(TEXT("Temporary binding leaves Profile package clean"), ProfilePackage->IsDirty());

	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	ProfilePackage->SetDirtyFlag(false);

	// 새 explicit reviewed UnbindVehicleProfile R1 transaction 결과입니다.
	FCFAuthoringOpResult UnbindResult;
	if (!TestTrue(TEXT("Explicit Profile unbind succeeds"), ViewModel.CommitProfileUnbinding(
		ECFVehicleProfileDomain::Handling,
		UnbindResult)))
	{
		AddError(UnbindResult.Message);
		return false;
	}
	TestTrue(TEXT("Profile unbind changes Recipe"), UnbindResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Profile unbind does not change Target"), UnbindResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Profile unbind does not change Profile"), UnbindResult.Mutation.bProfileChanged);
	TestFalse(TEXT("Profile unbind does not save"), UnbindResult.Mutation.bSavePerformed);
	TestFalse(TEXT("Handling binding returns to exact none baseline"), ViewModel.GetBoundProfilePath(ECFVehicleProfileDomain::Handling).IsValid());
	TestFalse(TEXT("Persistent Recipe Handling binding is null"), Fixture.Recipe->ProfileBindings.HandlingProfile.ToSoftObjectPath().IsValid());
	TestEqual(TEXT("Profile unbind preserves Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestFalse(TEXT("Profile unbind leaves Target package clean"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Profile unbind leaves Profile package clean"), ProfilePackage->IsDirty());
	return true;
}

// Frozen 24.91 Shared Profile single-Vehicle route가 기존 B2 transaction과 affected Vehicle impact preview를 그대로 재사용하는지 검증합니다.
bool FCFVehicleP11ProfileImpactTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Shared Profile을 참조할 managed affected Vehicle fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture Fixture;
	// Fixture/Profile preview diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-11 Shared Profile fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Shared Handling Profile을 소유하는 unsaved /Temp package입니다.
	UPackage* ProfilePackage = CFVehicleAuthoringVMTestsPrivate::CreateTestPackage(TEXT("CFP11SharedHandling"));
	// B2 typed numeric edit 대상 Shared Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(ProfilePackage, TEXT("DA_P11Handling_Test"), RF_Transactional);
	if (!TestNotNull(TEXT("P0-11 Shared Handling Profile creates"), HandlingProfile))
	{
		return false;
	}
		HandlingProfile->Data.FrontWheelMaxBrakeTorque = 1500.0f;
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	ProfilePackage->SetDirtyFlag(false);

	// Profile binding commit 전 canonical Target hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);

	// Normal Workspace Shared Profile route의 ViewModel입니다.
	FCFVehicleAuthoringVM ViewModel;
	// Shared Profile을 아직 binding하지 않은 managed Vehicle selection입니다.
	const FCFVehicleListEntry Entry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(Fixture, true);
	if (!TestTrue(TEXT("Shared Profile affected Vehicle selects before binding"), ViewModel.SelectVehicle(Entry, Error)))
	{
		AddError(Error);
		return false;
	}

	// UA-03 Production Workspace와 같은 reviewed BindVehicleProfile Recipe-only transaction 결과입니다.
	FCFAuthoringOpResult BindingResult;
	if (!TestTrue(TEXT("Shared Profile reviewed binding succeeds"), ViewModel.CommitProfileBinding(
		ECFVehicleProfileDomain::Handling,
		FSoftObjectPath(HandlingProfile),
		BindingResult)))
	{
		AddError(BindingResult.Message);
		return false;
	}
	TestTrue(TEXT("Profile binding reports Recipe mutation"), BindingResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Profile binding never target-mutates"), BindingResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Profile binding never profile-mutates"), BindingResult.Mutation.bProfileChanged);
	TestFalse(TEXT("Profile binding never auto-saves"), BindingResult.Mutation.bSavePerformed);
	TestEqual(TEXT("ViewModel exposes exact current Handling binding"), ViewModel.GetBoundProfilePath(ECFVehicleProfileDomain::Handling), FSoftObjectPath(HandlingProfile));
	TestEqual(TEXT("Profile binding preserves Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
			TestFalse(TEXT("Profile binding does not dirty Shared Profile"), ProfilePackage->IsDirty());

	// 실제 Authoring inventory에 포함되면 안 되는 rollback/prospective scratch Recipe를 재현합니다.
	TStrongObjectPtr<UCFVehicleRecipeData> TransientScratchRecipe(
		DuplicateObject<UCFVehicleRecipeData>(Fixture.Recipe, GetTransientPackage(), NAME_None));
	if (!TestTrue(TEXT("Transient scratch Recipe creates"), TransientScratchRecipe.IsValid()))
	{
		return false;
	}
	TransientScratchRecipe->SetFlags(RF_Transient);
	// Scratch만 Vehicle-specific DriveState required blocker를 가지게 해 잘못 수집될 경우 B2가 즉시 실패하도록 합니다.
	TransientScratchRecipe->DriveStateMode = ECFVehicleDriveStateMode::VehicleSpecific;
	TransientScratchRecipe->ProfileBindings.DriveStateProfile.Reset();
	TestTrue(TEXT("Transient scratch still references Handling Profile"),
		TransientScratchRecipe->ProfileBindings.HandlingProfile.ToSoftObjectPath() == FSoftObjectPath(HandlingProfile));
	TestTrue(TEXT("Regression scratch lives under /Engine/Transient"),
		FSoftObjectPath(TransientScratchRecipe.Get()).ToString().StartsWith(TEXT("/Engine/Transient")));

	// B2 commit 전 dependent Recipe fingerprint입니다.
	const FString RecipeFingerprintBefore = CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// B2 commit 전 dependent Recipe revision입니다.
	const int32 RecipeRevisionBefore = Fixture.Recipe->AuthoringRevision;
	// B2 commit 전 Profile revision입니다.
	const int32 ProfileRevisionBefore = HandlingProfile->Meta.AuthoringRevision;
		Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	ProfilePackage->SetDirtyFlag(false);

	// Registry selector가 B2 commit 전에 읽어야 하는 current authored numeric canonical value입니다.
	FString CurrentProfileNumericValue;
	// Current authored value read-only helper diagnostic입니다.
	FString CurrentProfileNumericValueError;
	if (!TestTrue(TEXT("Shared Profile current numeric read succeeds before B2 commit"), ViewModel.ReadBoundProfileNumericValue(
		ECFVehicleProfileDomain::Handling,
		TEXT("Profile.Handling.FrontWheelMaxBrakeTorque"),
		CurrentProfileNumericValue,
		CurrentProfileNumericValueError)))
	{
		AddError(CurrentProfileNumericValueError);
		return false;
	}
	TestTrue(TEXT("Shared Profile current numeric read matches 1500 before B2 commit"),
		FMath::IsNearlyEqual(FCString::Atof(*CurrentProfileNumericValue), 1500.0f));

	// Existing Batch Registry의 exact Handling numeric ColumnId를 사용하는 mutation0 B2 preview입니다.
	FCFProfileNumericEditPreview Preview;
	if (!TestTrue(TEXT("Shared Profile B2 preview succeeds"), ViewModel.PrepareProfileNumericEdit(
		ECFVehicleProfileDomain::Handling,
		TEXT("Profile.Handling.FrontWheelMaxBrakeTorque"),
		TEXT("2000"),
		Preview)))
	{
		AddError(ViewModel.GetLastMessage());
		return false;
	}
		TestEqual(TEXT("Shared Profile preview leaves persistent numeric unchanged"), HandlingProfile->Data.FrontWheelMaxBrakeTorque, 1500.0f);
	TestTrue(TEXT("Shared Profile preview reports affected Vehicle"), Preview.AffectedVehicleCount >= 1);
	TestTrue(TEXT("Shared Profile preview exposes navigation rows"), Preview.AffectedVehicles.Num() >= 1);
	// Transient rollback/prospective scratch는 affected navigation identity로 절대 노출되면 안 됩니다.
	const bool bContainsTransientScratch = Preview.AffectedVehicles.ContainsByPredicate([](const FCFProfileImpactVehicle& Impact)
	{
		return Impact.RecipePath.StartsWith(TEXT("/Engine/Transient"));
	});
	TestFalse(TEXT("Shared Profile preview excludes transient scratch Recipe"), bContainsTransientScratch);
	// 다른 live Automation fixture가 함께 존재해도 exact current Target 포함 여부만 검증합니다.
	const FString ExpectedAffectedTargetPath = FSoftObjectPath(Fixture.TargetVehicleData).ToString();
	// Affected impact navigation rows 안에 current fixture Target이 존재하는지 확인합니다.
	const bool bContainsExpectedAffectedVehicle = Preview.AffectedVehicles.ContainsByPredicate([&ExpectedAffectedTargetPath](const FCFProfileImpactVehicle& Impact)
	{
		return Impact.TargetPath == ExpectedAffectedTargetPath;
	});
	TestTrue(TEXT("Affected Vehicle navigation contains exact Target"), bContainsExpectedAffectedVehicle);
	TestEqual(TEXT("Shared Profile preview preserves Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Shared Profile preview preserves Recipe fingerprint"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Shared Profile preview preserves Recipe revision"), Fixture.Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestFalse(TEXT("Shared Profile preview does not dirty Profile"), ProfilePackage->IsDirty());
	TestFalse(TEXT("Shared Profile preview does not dirty Recipe"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Shared Profile preview does not dirty Target"), Fixture.TargetPackage->IsDirty());

	// Existing B2 source commit terminal result입니다.
	FCFProfileNumericEditResult CommitResult;
	if (!TestTrue(TEXT("Shared Profile B2 commit succeeds"), ViewModel.ExecutePreparedProfileEdit(CommitResult)))
	{
		AddError(CommitResult.Operation.Message);
		return false;
	}
		TestEqual(TEXT("Shared Profile numeric value committed through B2"), HandlingProfile->Data.FrontWheelMaxBrakeTorque, 2000.0f);
	CurrentProfileNumericValue.Reset();
	CurrentProfileNumericValueError.Reset();
	if (!TestTrue(TEXT("Shared Profile current numeric read succeeds after B2 commit"), ViewModel.ReadBoundProfileNumericValue(
		ECFVehicleProfileDomain::Handling,
		TEXT("Profile.Handling.FrontWheelMaxBrakeTorque"),
		CurrentProfileNumericValue,
		CurrentProfileNumericValueError)))
	{
		AddError(CurrentProfileNumericValueError);
		return false;
	}
	TestTrue(TEXT("Shared Profile current numeric read matches 2000 after B2 commit"),
		FMath::IsNearlyEqual(FCString::Atof(*CurrentProfileNumericValue), 2000.0f));
	TestEqual(TEXT("Shared Profile revision increments once"), HandlingProfile->Meta.AuthoringRevision, ProfileRevisionBefore + 1);
	TestEqual(TEXT("Dependent Recipe revision is unchanged by B2"), Fixture.Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("Dependent Recipe fingerprint remains same binding identity"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestTrue(TEXT("Shared Profile package becomes dirty pending explicit save"), ProfilePackage->IsDirty());
	TestFalse(TEXT("Shared Profile B2 never target-mutates"), CommitResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Shared Profile B2 never auto-saves"), CommitResult.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("Shared Profile B2 never automatically retries"), CommitResult.Operation.Mutation.bAutomaticRetryPerformed);
	TestEqual(TEXT("Shared Profile B2 preserves Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	return true;
}

// Frozen 24.92 External Drift가 exact 3-way value review와 Keep/Preserve/Advanced explicit decision을 모두 제공하는지 검증합니다.
bool FCFVehicleP11DriftRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Keep/Preserve decision fixture입니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture PreserveFixture;
	// Drift setup diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("P0-11 Preserve Drift fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(PreserveFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*PreserveFixture.Recipe);
	PreserveFixture.TargetPackage->SetDirtyFlag(false);
	PreserveFixture.RecipePackage->SetDirtyFlag(false);

	// Keep/Preserve flow를 실행할 normal Workspace ViewModel입니다.
	FCFVehicleAuthoringVM PreserveViewModel;
	// Preserve fixture managed selection입니다.
	const FCFVehicleListEntry PreserveEntry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(PreserveFixture, true);
	if (!TestTrue(TEXT("P0-11 Preserve Drift Vehicle selects"), PreserveViewModel.SelectVehicle(PreserveEntry, Error)))
	{
		AddError(Error);
		return false;
	}
	// Last Applied exact values를 생성할 reviewed Apply preparation입니다.
	FCFAuthoringOpResult InitialApplyPrepare;
	if (!TestTrue(TEXT("Drift baseline Apply prepares"), PreserveViewModel.PrepareApply(InitialApplyPrepare)))
	{
		AddError(PreserveViewModel.GetLastMessage());
		return false;
	}
	// Last Applied exact values를 기록하는 baseline Apply result입니다.
	FCFAuthoringOpResult InitialApplyResult;
	if (!TestTrue(TEXT("Drift baseline Apply succeeds"), PreserveViewModel.ExecutePreparedApply(InitialApplyResult)))
	{
		AddError(InitialApplyResult.Message);
		return false;
	}

	// Advanced Override 허용 field에 facade 밖 raw edit를 만들어 External Drift를 재현합니다.
	PreserveFixture.TargetVehicleData->VehicleMovementConfig.EngineMaxTorque += 25.0f;
	PreserveFixture.TargetVehicleData->MarkPackageDirty();
	// Recovery가 보존해야 할 raw Target hash입니다.
	const FString RawTargetHash = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*PreserveFixture.TargetVehicleData, Error);
	if (!TestTrue(TEXT("Raw torque drift refresh succeeds"), PreserveViewModel.RefreshPreview(Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Raw torque edit is External Drift"), PreserveViewModel.HasExternalDrift());
	TestFalse(TEXT("Unreviewed External Drift still blocks Apply"), PreserveViewModel.CanApply());

	// Fresh exact 3-way drift review입니다.
	if (!TestTrue(TEXT("External Drift 3-way review builds"), PreserveViewModel.RefreshDriftReview(Error)))
	{
		AddError(Error);
		return false;
	}
	// EngineMaxTorque exact 3-way row입니다.
	const FCFVehicleDriftReviewRow* TorqueRow = CFVehicleAuthoringVMTestsPrivate::FindDriftRow(
		PreserveViewModel.GetDriftReview(),
		TEXT("VehicleMovementConfig.EngineMaxTorque"));
	if (!TestNotNull(TEXT("3-way review contains EngineMaxTorque"), TorqueRow))
	{
		return false;
	}
		TestTrue(TEXT("New Apply trace carries exact Last Applied value"), TorqueRow->bHasLastAppliedValue);
	TestTrue(TEXT("EngineMaxTorque supports Advanced Override"), TorqueRow->bAdvancedOverrideAllowed);
	TestNotEqual(TEXT("Current Raw differs from Last Applied after drift"), TorqueRow->CurrentRawValue.CanonicalValueText, TorqueRow->LastAppliedValue.CanonicalValueText);
	// 이후 ViewModel review refresh에도 안전하게 사용할 exact Stable Field Path value copy입니다.
	const FCFVehicleFieldPath TorqueFieldPath = TorqueRow->FieldPath;

	// Empty subset means current External Drift whole reviewed group for Keep Authoring입니다.
	TArray<FCFVehicleFieldPath> WholeDriftGroup;
	// Mutation0 Keep Authoring prospective review입니다.
	FCFVehicleDriftDecisionPreview KeepPreview;
	if (!TestTrue(TEXT("Keep Authoring preview succeeds"), PreserveViewModel.PrepareDriftDecision(
		WholeDriftGroup,
		ECFVehicleDriftDecision::KeepAuthoring,
		FString(),
		KeepPreview)))
	{
		AddError(PreserveViewModel.GetLastMessage());
		return false;
	}
	// Keep decision 전 Recipe fingerprint입니다.
	const FString RecipeFingerprintBeforeKeep = CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*PreserveFixture.Recipe, Error);
	// Mutation0 reviewed Keep token terminal result입니다.
	FCFAuthoringOpResult KeepResult;
	if (!TestTrue(TEXT("Keep Authoring review token commits"), PreserveViewModel.ExecutePreparedDriftDecision(KeepResult)))
	{
		AddError(KeepResult.Message);
		return false;
	}
	TestTrue(TEXT("Keep Authoring exact evidence token is active"), PreserveViewModel.HasAcceptedDriftKeep());
	TestTrue(TEXT("Reviewed Keep token opens normal Apply gate"), PreserveViewModel.CanApply());
	TestEqual(TEXT("Keep Authoring never mutates Recipe"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*PreserveFixture.Recipe, Error), RecipeFingerprintBeforeKeep);
	TestEqual(TEXT("Keep Authoring never mutates raw Target"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*PreserveFixture.TargetVehicleData, Error), RawTargetHash);
	TestFalse(TEXT("Keep Authoring reports no Recipe mutation"), KeepResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Keep Authoring reports no Target mutation"), KeepResult.Mutation.bTargetChanged);

	// Fresh refresh가 old Keep token을 폐기하는 expected lifecycle입니다.
	if (!TestTrue(TEXT("Fresh refresh after Keep succeeds"), PreserveViewModel.RefreshPreview(Error)))
	{
		AddError(Error);
		return false;
	}
	TestFalse(TEXT("Refresh invalidates Keep token"), PreserveViewModel.HasAcceptedDriftKeep());
	TestFalse(TEXT("External Drift blocks Apply again after token invalidation"), PreserveViewModel.CanApply());

	// Exact torque field만 Preserve Raw As Legacy Pin할 reviewed subset입니다.
		TArray<FCFVehicleFieldPath> TorqueOnlyPaths;
	TorqueOnlyPaths.Add(TorqueFieldPath);
	// Preserve Raw prospective ownership review입니다.
	FCFVehicleDriftDecisionPreview PreservePreview;
	if (!TestTrue(TEXT("Preserve Raw preview succeeds"), PreserveViewModel.PrepareDriftDecision(
		TorqueOnlyPaths,
		ECFVehicleDriftDecision::PreserveRawAsLegacyPin,
		FString(),
		PreservePreview)))
	{
		AddError(PreserveViewModel.GetLastMessage());
		return false;
	}
	// Preserve commit 전 Recipe fingerprint입니다.
	const FString RecipeFingerprintBeforePreserve = CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*PreserveFixture.Recipe, Error);
	// Reviewed Preserve Raw terminal result입니다.
	FCFAuthoringOpResult PreserveResult;
	if (!TestTrue(TEXT("Preserve Raw commit succeeds"), PreserveViewModel.ExecutePreparedDriftDecision(PreserveResult)))
	{
		AddError(PreserveResult.Message);
		return false;
	}
	TestTrue(TEXT("Preserve Raw reports Recipe ownership mutation"), PreserveResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Preserve Raw never mutates Target"), PreserveResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Preserve Raw never auto-saves"), PreserveResult.Mutation.bSavePerformed);
	TestNotEqual(TEXT("Preserve Raw changes Recipe ownership fingerprint"), CFVehicleAuthoringVMTestsPrivate::BuildRecipeFingerprint(*PreserveFixture.Recipe, Error), RecipeFingerprintBeforePreserve);
	TestEqual(TEXT("Preserve Raw keeps exact raw Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*PreserveFixture.TargetVehicleData, Error), RawTargetHash);

	// Advanced Override decision은 독립 fixture에서 검증해 Preserve ownership state와 섞이지 않게 합니다.
	CFVehicleAuthoringVMTestsPrivate::FWorkspaceFixture AdvancedFixture;
	if (!TestTrue(TEXT("P0-11 Advanced Drift fixture builds"), CFVehicleAuthoringVMTestsPrivate::BuildImportedFixture(AdvancedFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringVMTestsPrivate::ConfigureApplyDifference(*AdvancedFixture.Recipe);
	// Advanced fixture normal Workspace ViewModel입니다.
	FCFVehicleAuthoringVM AdvancedViewModel;
	// Advanced fixture managed selection입니다.
	const FCFVehicleListEntry AdvancedEntry = CFVehicleAuthoringVMTestsPrivate::BuildListEntry(AdvancedFixture, true);
	if (!TestTrue(TEXT("P0-11 Advanced Drift Vehicle selects"), AdvancedViewModel.SelectVehicle(AdvancedEntry, Error)))
	{
		AddError(Error);
		return false;
	}
	// Advanced fixture Last Applied baseline preparation입니다.
	FCFAuthoringOpResult AdvancedApplyPrepare;
	if (!TestTrue(TEXT("Advanced Drift baseline Apply prepares"), AdvancedViewModel.PrepareApply(AdvancedApplyPrepare)))
	{
		AddError(AdvancedViewModel.GetLastMessage());
		return false;
	}
	// Advanced fixture Last Applied baseline result입니다.
	FCFAuthoringOpResult AdvancedApplyResult;
	if (!TestTrue(TEXT("Advanced Drift baseline Apply succeeds"), AdvancedViewModel.ExecutePreparedApply(AdvancedApplyResult)))
	{
		AddError(AdvancedApplyResult.Message);
		return false;
	}
	AdvancedFixture.TargetVehicleData->VehicleMovementConfig.EngineMaxTorque += 35.0f;
	AdvancedFixture.TargetVehicleData->MarkPackageDirty();
	// Advanced recovery가 보존해야 하는 raw Target hash입니다.
	const FString AdvancedRawTargetHash = CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*AdvancedFixture.TargetVehicleData, Error);
	if (!TestTrue(TEXT("Advanced raw drift refresh succeeds"), AdvancedViewModel.RefreshPreview(Error)))
	{
		AddError(Error);
		return false;
	}
	if (!TestTrue(TEXT("Advanced 3-way review builds"), AdvancedViewModel.RefreshDriftReview(Error)))
	{
		AddError(Error);
		return false;
	}
	// Advanced exact EngineMaxTorque row입니다.
	const FCFVehicleDriftReviewRow* AdvancedTorqueRow = CFVehicleAuthoringVMTestsPrivate::FindDriftRow(
		AdvancedViewModel.GetDriftReview(),
		TEXT("VehicleMovementConfig.EngineMaxTorque"));
	if (!TestNotNull(TEXT("Advanced review contains EngineMaxTorque"), AdvancedTorqueRow))
	{
		return false;
	}
	// Advanced exact reviewed subset입니다.
	TArray<FCFVehicleFieldPath> AdvancedPaths;
	AdvancedPaths.Add(AdvancedTorqueRow->FieldPath);
	// Explicit reason-bound Advanced Override prospective review입니다.
	FCFVehicleDriftDecisionPreview AdvancedPreview;
	if (!TestTrue(TEXT("Promote Raw to Advanced Override preview succeeds"), AdvancedViewModel.PrepareDriftDecision(
		AdvancedPaths,
		ECFVehicleDriftDecision::PromoteRawToAdvancedOverride,
		TEXT("P0-11 Automation reviewed external tuning"),
		AdvancedPreview)))
	{
		AddError(AdvancedViewModel.GetLastMessage());
		return false;
	}
	// Advanced Override terminal result입니다.
	FCFAuthoringOpResult AdvancedResult;
	if (!TestTrue(TEXT("Promote Raw to Advanced Override commit succeeds"), AdvancedViewModel.ExecutePreparedDriftDecision(AdvancedResult)))
	{
		AddError(AdvancedResult.Message);
		return false;
	}
	TestTrue(TEXT("Advanced Override reports Recipe mutation"), AdvancedResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Advanced Override never mutates Target"), AdvancedResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Advanced Override never auto-saves"), AdvancedResult.Mutation.bSavePerformed);
	TestEqual(TEXT("Advanced Override preserves raw Target hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*AdvancedFixture.TargetVehicleData, Error), AdvancedRawTargetHash);
	// EngineMaxTorque exact Advanced Override 존재 여부입니다.
	const bool bHasEngineTorqueOverride = AdvancedFixture.Recipe->AdvancedOverrides.ContainsByPredicate([](const FCFVehicleFieldOverride& Override)
	{
		return Override.FieldPath.ToCanonicalString(true) == TEXT("VehicleMovementConfig.EngineMaxTorque");
	});
	TestTrue(TEXT("Advanced Override stores exact EngineMaxTorque field"), bHasEngineTorqueOverride);
	return true;
}

// Frozen 24.94 Mesh-only Candidate discovery와 reviewed Definition+Recipe two-record creation/no-inference 경계를 검증합니다.
bool FCFVehicleP11MeshCreateTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Test asset identity를 격리하는 unique token입니다.
	const FString UniqueToken = FGuid::NewGuid().ToString(EGuidFormats::Digits);
		// Mesh candidate inventory용 bounded common collection root입니다.
	const FString MeshCollectionRoot = TEXT("/Game/CarFight/Tests/DataAuthoringP11/Mesh_") + UniqueToken;
	// Existing chassis가 속한 vehicle subdirectory입니다.
	const FString UsedVehicleFolder = MeshCollectionRoot + TEXT("/UsedVehicle");
	// Candidate chassis가 속한 별도 sibling vehicle subdirectory입니다.
	const FString CandidateVehicleFolder = MeshCollectionRoot + TEXT("/CandidateVehicle");
	// Existing used mesh package name입니다.
	const FString UsedMeshPackageName = UsedVehicleFolder + TEXT("/SM_Used");
	// Candidate mesh package name입니다.
	const FString CandidateMeshPackageName = CandidateVehicleFolder + TEXT("/SM_Candidate");
	// Existing Definition이 실제 wheel visual로 사용하는 used vehicle folder mesh package name입니다.
	const FString WheelMeshPackageName = UsedVehicleFolder + TEXT("/SM_KnownWheel");
	// Existing unmanaged Definition package name입니다.
	const FString ExistingDefinitionPackageName = TEXT("/Game/CarFight/Tests/DataAuthoringP11/DA_Existing_") + UniqueToken;

	// Used StaticMesh package입니다.
	UPackage* UsedMeshPackage = CreatePackage(*UsedMeshPackageName);
	// Used StaticMesh source asset입니다.
	UStaticMesh* UsedMesh = NewObject<UStaticMesh>(UsedMeshPackage, TEXT("SM_Used"), RF_Public | RF_Standalone | RF_Transactional);
	// Mesh-only Candidate package입니다.
	UPackage* CandidateMeshPackage = CreatePackage(*CandidateMeshPackageName);
		// 아직 Vehicle Definition에 연결되지 않은 candidate StaticMesh입니다.
	UStaticMesh* CandidateMesh = NewObject<UStaticMesh>(CandidateMeshPackage, TEXT("SM_Candidate"), RF_Public | RF_Standalone | RF_Transactional);
	// Known wheel StaticMesh package입니다.
	UPackage* WheelMeshPackage = CreatePackage(*WheelMeshPackageName);
	// Existing Definition이 WheelMeshFL로 참조하는 sibling StaticMesh입니다.
	UStaticMesh* KnownWheelMesh = NewObject<UStaticMesh>(WheelMeshPackage, TEXT("SM_KnownWheel"), RF_Public | RF_Standalone | RF_Transactional);
	// Existing unmanaged Definition package입니다.
	UPackage* ExistingDefinitionPackage = CreatePackage(*ExistingDefinitionPackageName);
		// Used mesh를 참조해 common collection root를 establish하는 unmanaged Definition입니다.
	UCFVehicleData* ExistingDefinition = NewObject<UCFVehicleData>(ExistingDefinitionPackage, *FString(TEXT("DA_Existing_") + UniqueToken), RF_Public | RF_Standalone | RF_Transactional);
		if (!TestNotNull(TEXT("MeshCreate used mesh exists"), UsedMesh)
		|| !TestNotNull(TEXT("MeshCreate candidate mesh exists"), CandidateMesh)
		|| !TestNotNull(TEXT("MeshCreate known wheel mesh exists"), KnownWheelMesh)
		|| !TestNotNull(TEXT("MeshCreate existing Definition exists"), ExistingDefinition))
	{
		return false;
	}
		ExistingDefinition->VehicleVisualConfig.ChassisMesh = UsedMesh;
	ExistingDefinition->VehicleVisualConfig.WheelMeshFL = KnownWheelMesh;
	FAssetRegistryModule::AssetCreated(UsedMesh);
	FAssetRegistryModule::AssetCreated(CandidateMesh);
	FAssetRegistryModule::AssetCreated(KnownWheelMesh);
	FAssetRegistryModule::AssetCreated(ExistingDefinition);
	UsedMeshPackage->SetDirtyFlag(false);
	CandidateMeshPackage->SetDirtyFlag(false);
	WheelMeshPackage->SetDirtyFlag(false);
	ExistingDefinitionPackage->SetDirtyFlag(false);

	// Mesh-only Candidate를 포함하는 normal Browser R0 request입니다.
	FCFVehicleListRequest ListRequest;
	ListRequest.bIncludeRecipeRecords = false;
	ListRequest.bIncludeUnmanagedDefinitions = true;
	ListRequest.bIncludeMeshCandidates = true;
	ListRequest.SearchText = UniqueToken;
	// Common Authoring facade Browser result입니다.
	FCFVehicleListResult ListResult;
	if (!TestTrue(TEXT("Mesh-only Candidate Browser query succeeds"), FCFVehicleAuthoringService::ListVehicles(ListRequest, ListResult)))
	{
		AddError(ListResult.Operation.Message);
				CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(ExistingDefinition);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(KnownWheelMesh);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(CandidateMesh);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(UsedMesh);
		return false;
	}
	// Exact unique candidate object path입니다.
	const FString CandidatePath = FSoftObjectPath(CandidateMesh).ToString();
	// Browser result에서 exact Mesh-only Candidate row를 찾습니다.
	const FCFVehicleListEntry* CandidateEntry = ListResult.Vehicles.FindByPredicate([&CandidatePath](const FCFVehicleListEntry& Entry)
	{
		return Entry.bMeshOnlyCandidate && Entry.ChassisMeshPath.ToString() == CandidatePath;
	});
		TestNotNull(TEXT("Unused StaticMesh in sibling vehicle directory is projected as Mesh-only Candidate"), CandidateEntry);
	// Used mesh가 candidate로 잘못 노출되는지 검사합니다.
	const FString UsedMeshPath = FSoftObjectPath(UsedMesh).ToString();
	const bool bUsedMeshProjectedAsCandidate = ListResult.Vehicles.ContainsByPredicate([&UsedMeshPath](const FCFVehicleListEntry& Entry)
	{
		return Entry.bMeshOnlyCandidate && Entry.ChassisMeshPath.ToString() == UsedMeshPath;
	});
		TestFalse(TEXT("Already-used Chassis mesh is not a Mesh-only Candidate"), bUsedMeshProjectedAsCandidate);
	// Known wheel mesh가 chassis candidate로 잘못 노출되는지 검사합니다.
	const FString KnownWheelMeshPath = FSoftObjectPath(KnownWheelMesh).ToString();
	const bool bKnownWheelProjectedAsCandidate = ListResult.Vehicles.ContainsByPredicate([&KnownWheelMeshPath](const FCFVehicleListEntry& Entry)
	{
		return Entry.bMeshOnlyCandidate && Entry.ChassisMeshPath.ToString() == KnownWheelMeshPath;
	});
	TestFalse(TEXT("WheelMesh referenced by VehicleData is not a Mesh-only Chassis Candidate"), bKnownWheelProjectedAsCandidate);

	// 새 Definition package name입니다.
	const FString NewDefinitionPackageName = TEXT("/Game/CarFight/Tests/DataAuthoringP11/DA_Vehicle_") + UniqueToken;
	// 새 Recipe package name입니다.
	const FString NewRecipePackageName = TEXT("/Game/CarFight/Tests/DataAuthoringP11/DA_Recipe_") + UniqueToken;
	// Explicit two-record creation request입니다.
	FCFVehicleRecordCreateRequest CreateRequest;
	CreateRequest.DefinitionPackageName = NewDefinitionPackageName;
	CreateRequest.DefinitionAssetName = FName(*FString(TEXT("DA_Vehicle_") + UniqueToken));
	CreateRequest.RecipePackageName = NewRecipePackageName;
	CreateRequest.RecipeAssetName = FName(*FString(TEXT("DA_Recipe_") + UniqueToken));
	CreateRequest.ChassisMesh = CandidateMesh;
	CreateRequest.CallerKind = ECFAuthoringCallerKind::Automation;
	// Mutation0 reviewed creation proposal입니다.
	FCFVehicleRecordCreatePreview CreatePreview;
	if (!TestTrue(TEXT("Create Vehicle From Mesh preview succeeds"), FCFVehicleAuthoringService::PreviewVehicleRecords(CreateRequest, CreatePreview)))
	{
		AddError(CreatePreview.Operation.Message);
				CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(ExistingDefinition);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(KnownWheelMesh);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(CandidateMesh);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(UsedMesh);
		return false;
	}
	TestFalse(TEXT("Create Vehicle preview never saves"), CreatePreview.Proposal.bSavePerformed);
	TestTrue(TEXT("Create Vehicle preview requires OwnershipWrite"), CreatePreview.Proposal.RequiredApprovalClass == ECFAuthoringApprovalClass::OwnershipWrite);

	CreateRequest.bOwnershipWriteApproved = true;
	CreateRequest.ApprovalScopeHash = CreatePreview.Proposal.ProposalHash;
	// Exact reviewed two-record creation terminal result입니다.
	FCFVehicleRecordCreateResult CreateResult;
	if (!TestTrue(TEXT("Create Vehicle From Mesh two-record commit succeeds"), FCFVehicleAuthoringService::CreateVehicleRecords(CreateRequest, CreateResult)))
	{
		AddError(CreateResult.Operation.Message);
				CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(ExistingDefinition);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(KnownWheelMesh);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(CandidateMesh);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(UsedMesh);
		return false;
	}
	TestNotNull(TEXT("Create Vehicle creates canonical Definition record"), CreateResult.CreatedDefinition);
	TestNotNull(TEXT("Create Vehicle creates Editor-only Recipe record"), CreateResult.CreatedRecipe);
	if (CreateResult.CreatedDefinition && CreateResult.CreatedRecipe)
	{
		TestEqual(TEXT("Recipe binds exact created Definition"), CreateResult.CreatedRecipe->TargetVehicleData.Get(), CreateResult.CreatedDefinition);
		TestEqual(TEXT("Recipe stores explicit candidate Chassis intent"), CreateResult.CreatedRecipe->AssetIntent.ChassisMesh.Get(), CandidateMesh);
		TestTrue(TEXT("New Recipe starts managed"), CreateResult.CreatedRecipe->ImportState.ManageState == ECFVehicleManageState::Managed);
		TestTrue(TEXT("VehicleBase Profile is not inferred"), CreateResult.CreatedRecipe->ProfileBindings.VehicleBaseProfile.IsNull());
		TestTrue(TEXT("Drivetrain Profile is not inferred"), CreateResult.CreatedRecipe->ProfileBindings.DrivetrainProfile.IsNull());
		TestTrue(TEXT("Handling Profile is not inferred"), CreateResult.CreatedRecipe->ProfileBindings.HandlingProfile.IsNull());
		TestTrue(TEXT("Performance Profile is not inferred"), CreateResult.CreatedRecipe->ProfileBindings.PerformanceProfile.IsNull());
		TestTrue(TEXT("DriveState Profile is not inferred"), CreateResult.CreatedRecipe->ProfileBindings.DriveStateProfile.IsNull());
		TestNull(TEXT("Definition C++ defaults are not silently Apply-mutated with candidate Chassis"), CreateResult.CreatedDefinition->VehicleVisualConfig.ChassisMesh);
		TestTrue(TEXT("Created Definition package is dirty pending explicit save"), CreateResult.CreatedDefinition->GetOutermost()->IsDirty());
		TestTrue(TEXT("Created Recipe package is dirty pending explicit save"), CreateResult.CreatedRecipe->GetOutermost()->IsDirty());
	}
	TestTrue(TEXT("Two-record creation reports created assets"), CreateResult.Operation.Mutation.bCreatedAssets);
	TestFalse(TEXT("Two-record creation never auto-saves"), CreateResult.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("Two-record creation never automatically retries"), CreateResult.Operation.Mutation.bAutomaticRetryPerformed);

	CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(CreateResult.CreatedRecipe);
	CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(CreateResult.CreatedDefinition);
		CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(ExistingDefinition);
	CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(KnownWheelMesh);
	CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(CandidateMesh);
	CFVehicleAuthoringVMTestsPrivate::CleanupRegisteredAsset(UsedMesh);
	return true;
}

#endif
