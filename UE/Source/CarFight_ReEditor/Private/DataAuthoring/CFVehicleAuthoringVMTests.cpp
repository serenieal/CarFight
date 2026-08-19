// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringVMTests.cpp
// Version: v1.7.0
// Date: 2026-08-19
// Description: DAUTH-P0-09~12 Vehicle Authoring Workspace ViewModel/Parity/UX 보호 Automation입니다.
// Changelog:
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
	FCFVehicleP11DriftRecoveryTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.DriftRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleP11MeshCreateTest,
	"CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Editor.h"
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
	TestFalse(TEXT("Workspace Apply never auto-saves"), ApplyResult.Mutation.bSavePerformed);
	TestFalse(TEXT("Workspace Apply never automatic retries"), ApplyResult.Mutation.bAutomaticRetryPerformed);
	TestFalse(TEXT("Prepared approval is consumed after Apply"), ViewModel.HasPreparedApply());

	TestNotNull(TEXT("Editor transaction system is available"), GEditor);
	if (!GEditor)
	{
		return false;
	}
	// Existing ApplyService FScopedTransaction을 되돌리는 Unreal standard editor Undo 결과입니다.
	const bool bUndoSucceeded = GEditor->UndoTransaction();
	TestTrue(TEXT("Unreal standard Undo succeeds for Apply transaction"), bUndoSucceeded);
	TestEqual(TEXT("Undo restores full Target Definition hash"), CFVehicleAuthoringVMTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Undo restores AppliedState recipe revision"), Fixture.Recipe->AppliedState.AppliedRecipeRevision, AppliedRevisionBefore);
	TestEqual(TEXT("Undo restores AppliedState Recipe fingerprint"), Fixture.Recipe->AppliedState.AppliedRecipeFingerprint, AppliedRecipeFingerprintBefore);
	TestEqual(TEXT("Undo restores AppliedState Source signature"), Fixture.Recipe->AppliedState.AppliedSourceSignature, AppliedSourceSignatureBefore);
	TestEqual(TEXT("Undo restores AppliedState Definition hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, AppliedDefinitionHashBefore);
	TestEqual(TEXT("Undo restores AppliedState resolver revision"), Fixture.Recipe->AppliedState.ResolverContractRevision, AppliedResolverRevisionBefore);
	TestEqual(TEXT("Undo restores AppliedState trace count"), Fixture.Recipe->AppliedState.FieldTraces.Num(), AppliedTraceCountBefore);

	// Undo 이후 current truth를 다시 읽는 UI refresh diagnostic입니다.
	FString RefreshError;
	TestTrue(TEXT("Workspace refreshes fresh truth after Undo"), ViewModel.RefreshPreview(RefreshError));
	TestTrue(TEXT("Undo returns pending Authoring Changes"), ViewModel.GetPendingDiffCount() > 0);
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
