// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchApplyTests.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08M B3 Batch Definition Apply Foundation Automation입니다.
// Changelog:
// - v1.0.0: Global preflight mutation0, deterministic order, partial failure, target-only ApplyService lane와 ineligible classification 검증 추가.
// Migration:
// - /Temp Recipe/Target과 transient fixture만 사용하며 Content Asset/file save를 수행하지 않습니다.
// - Target mutation은 production과 동일하게 FCFBatchApplyService -> FCFVehicleApplyService 경로만 사용합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFBatchApplyTestAccess.h"
#include "CFVehicleData.h"
#include "DataAuthoring/CFBatchApply.h"
#include "DataAuthoring/CFBatchImport.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#include <type_traits>


namespace CFBatchApplyTestsPrivate
{
		/** B3 Automation 한 Vehicle에 필요한 in-memory Recipe/Target truth입니다. */
	struct FApplyFixture
	{
		// Target VehicleData를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* RecipePackage = nullptr;

		// B3 ApplyService가 변경할 Runtime canonical Target fixture입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// B3 fresh read/resolve의 persistent Authoring Recipe fixture입니다.
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

	// Existing Definition import와 Definition Validator를 통과하는 Target baseline을 구성합니다.
	bool ConfigureValidTarget(FApplyFixture& InOutFixture, FString& OutError)
	{
		InOutFixture.TargetPackage = CreateTestPackage(TEXT("CFBatchApplyTarget"));
		if (!InOutFixture.TargetPackage)
		{
			OutError = TEXT("B3 Target /Temp package를 만들 수 없습니다.");
			return false;
		}

		// Existing ApplyService가 사용할 exact Target object입니다.
		InOutFixture.TargetVehicleData = NewObject<UCFVehicleData>(InOutFixture.TargetPackage, TEXT("DA_BatchApplyTarget_Test"), RF_Transactional);
		if (!InOutFixture.TargetVehicleData)
		{
			OutError = TEXT("B3 Target VehicleData를 만들 수 없습니다.");
			return false;
		}

		// Layout/Hardpoint asset source로 사용할 transient StaticMesh입니다.
		InOutFixture.ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		if (!InOutFixture.ChassisMesh)
		{
			OutError = TEXT("B3 transient Chassis mesh를 만들 수 없습니다.");
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
			OutError = TEXT("B3 Engine Cube wheel fixture를 로드할 수 없습니다.");
			return false;
		}
		// Existing Validator가 요구하는 ChaosVehicleWheel class입니다.
		UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
		if (!WheelClass)
		{
			OutError = TEXT("B3 ChaosVehicleWheel class를 로드할 수 없습니다.");
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
		OutError.Reset();
		return true;
	}

	// Current Definition을 Existing Definition import한 Recipe fixture를 생성합니다.
	bool BuildImportedFixture(FApplyFixture& OutFixture, FString& OutError)
	{
		OutFixture = FApplyFixture();
		if (!ConfigureValidTarget(OutFixture, OutError))
		{
			return false;
		}

		// Existing Definition exact source snapshot입니다.
		FCFVehicleDefinitionSnapshot CurrentDefinition;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*OutFixture.TargetVehicleData, CurrentDefinition, OutError))
		{
			return false;
		}
		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFBatchApplyRecipe"));
		if (!OutFixture.RecipePackage)
		{
			OutError = TEXT("B3 Recipe /Temp package를 만들 수 없습니다.");
			return false;
		}

		// B3 plan이 fresh read할 persistent Recipe입니다.
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_BatchApplyRecipe_Test"), RF_Transactional);
		if (!OutFixture.Recipe)
		{
			OutError = TEXT("B3 Recipe를 만들 수 없습니다.");
			return false;
		}
		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;

		// P0-08G shared Import Core로 current Definition을 lossless source baseline으로 가져옵니다.
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

	// Hardpoint ownership을 release하고 actual non-empty Apply diff를 구성합니다.
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
		// Target에 새로 생성할 semantic Hardpoint입니다.
		FCFHardpointIntent& NewHardpoint = Recipe.HardpointIntents.AddDefaulted_GetRef();
		NewHardpoint.LocationSlotId = TEXT("Front_New");
		NewHardpoint.LocationCategory = TEXT("Front");
		NewHardpoint.SocketName = TEXT("HP_Front_New");

		Recipe.MountIntents.Reset();
		// New Hardpoint를 참조하는 semantic Mount입니다.
		FCFMountIntent& NewMount = Recipe.MountIntents.AddDefaulted_GetRef();
		NewMount.MountProfileId = TEXT("M_Front_New");
		NewMount.LocationSlotRef = TEXT("Front_New");
		NewMount.MountType = ECFVehicleMountType::Turret;
		NewMount.SizeLimit = ECFVehicleWeaponSize::Medium;
		NewMount.DefaultEquipmentPresetData = nullptr;
		NewMount.bExposedModule = true;
		++Recipe.AuthoringRevision;
	}

	// Current Target full Definition hash를 shared SnapshotBuilder로 읽습니다.
	FString BuildTargetHash(const UCFVehicleData& Target, FString& OutError)
	{
		// Current immutable full Definition snapshot입니다.
		FCFVehicleDefinitionSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.DefinitionHash;
	}

	// Recipe 집합을 fresh B3 plan과 exact B3 approval로 연속 생성합니다.
	bool BuildPlanAndApproval(
		const TArray<UCFVehicleRecipeData*>& Recipes,
		FCFBatchDefinitionApplyPlan& OutPlan,
		FCFBatchDefinitionApplyApproval& OutApproval,
		FString& OutError)
	{
		// Fresh B3 R3 evidence collection request입니다.
		FCFBatchDefinitionApplyPlanRequest PlanRequest;
		PlanRequest.Recipes = Recipes;
		PlanRequest.CallerKind = ECFAuthoringCallerKind::Automation;
		if (!FCFBatchApplyService::BuildDefinitionApplyPlan(PlanRequest, OutPlan, OutError))
		{
			return false;
		}
		return FCFBatchApplyService::BuildDefinitionApplyApproval(OutPlan, OutApproval, OutError);
	}

	// Exact reviewed plan/approval을 explicit B3 Apply request로 실행합니다.
	bool ApplyApprovedPlan(
		const FCFBatchDefinitionApplyPlan& Plan,
		const FCFBatchDefinitionApplyApproval& Approval,
		FCFBatchDefinitionApplyResult& OutResult)
	{
		// Explicit B3 Definition Apply request입니다.
		FCFBatchDefinitionApplyRequest ApplyRequest;
		ApplyRequest.Plan = &Plan;
		ApplyRequest.Approval = &Approval;
		ApplyRequest.bDefinitionApplyApproved = true;
		return FCFBatchApplyService::ApplyBatch(ApplyRequest, OutResult);
	}

	// DAUTH-P0-08M additional fixture helper anchor.

}

static_assert(
	!std::is_same_v<FCFBatchDefinitionApplyApproval, FCFBatchCommitApproval>,
	"B1/B2 source commit approval type must not be reusable as B3 Definition Apply approval.");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchApplyPreflightTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchApply.PreflightAllZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchApplyOrderTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchApply.DeterministicOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchApplyPartialTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchApply.PartialFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchApplyEligibilityTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchApply.EligibilityApproval",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Approval 이후 한 eligible Target이 stale이면 first mutation 전에 whole B3를 mutation0으로 중단하는지 검증합니다.
bool FCFBatchApplyPreflightTest::RunTest(const FString& Parameters)
{
	// First eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture FirstFixture;
	// Second eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture SecondFixture;
	// Fixture/plan diagnostics입니다.
	FString Error;
	if (!TestTrue(TEXT("Preflight first fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("Preflight second fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(SecondFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*FirstFixture.Recipe);
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*SecondFixture.Recipe);

	// Reviewed fresh B3 plan입니다.
	FCFBatchDefinitionApplyPlan Plan;
	// Exact B3 approval입니다.
	FCFBatchDefinitionApplyApproval Approval;
	TArray<UCFVehicleRecipeData*> Recipes = {FirstFixture.Recipe, SecondFixture.Recipe};
	if (!TestTrue(TEXT("Preflight plan/approval builds"), CFBatchApplyTestsPrivate::BuildPlanAndApproval(Recipes, Plan, Approval, Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Preflight plan has two eligible Vehicles"), Plan.EligibleVehicleCount, 2);

	// Fresh first Target hash before stale injection입니다.
	const FString FirstHashBefore = CFBatchApplyTestsPrivate::BuildTargetHash(*FirstFixture.TargetVehicleData, Error);
	// Approval 이후 second Target에 발생한 external edit입니다.
	SecondFixture.TargetVehicleData->DestroyedFxSocketName = TEXT("B3_GlobalPreflight_ExternalEdit");
	// B3가 보존해야 하는 externally edited second Target hash입니다.
	const FString SecondExternalHash = CFBatchApplyTestsPrivate::BuildTargetHash(*SecondFixture.TargetVehicleData, Error);

	// Global-preflight blocked B3 result입니다.
	FCFBatchDefinitionApplyResult ApplyResult;
	TestFalse(TEXT("Stale eligible item blocks whole B3"), CFBatchApplyTestsPrivate::ApplyApprovedPlan(Plan, Approval, ApplyResult));
	TestEqual(TEXT("Preflight terminal status is Blocked"), ApplyResult.Status, ECFBatchApplyStatus::Blocked);
	TestEqual(TEXT("Preflight stable error taxonomy"), ApplyResult.ErrorCode, ECFBatchApplyErrorCode::GlobalPreflightFailed);
	TestEqual(TEXT("Preflight ApplyService call count remains zero"), ApplyResult.ApplyServiceCallCount, 0);
	TestEqual(TEXT("Preflight Applied count remains zero"), ApplyResult.AppliedVehicleCount, 0);
	TestEqual(TEXT("First Target remains mutation zero"), CFBatchApplyTestsPrivate::BuildTargetHash(*FirstFixture.TargetVehicleData, Error), FirstHashBefore);
	TestEqual(TEXT("External second Target edit is preserved"), CFBatchApplyTestsPrivate::BuildTargetHash(*SecondFixture.TargetVehicleData, Error), SecondExternalHash);
	TestFalse(TEXT("Preflight failure never performs global rollback"), ApplyResult.bGlobalRollbackPerformed);
	TestFalse(TEXT("Preflight failure never retries"), ApplyResult.bAutomaticRetryPerformed);
	TestFalse(TEXT("Preflight failure never saves"), ApplyResult.bSavePerformed);
	return true;
}

// Reversed Recipe input에서도 canonical Target path order/hash가 고정되고 각 Target이 ApplyService exact once로 성공하는지 검증합니다.
bool FCFBatchApplyOrderTest::RunTest(const FString& Parameters)
{
	// First eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture FirstFixture;
	// Second eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture SecondFixture;
	// Fixture/plan diagnostics입니다.
	FString Error;
	if (!TestTrue(TEXT("Order first fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("Order second fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(SecondFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*FirstFixture.Recipe);
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*SecondFixture.Recipe);

	// Reverse-input plan입니다.
	FCFBatchDefinitionApplyPlan ReversePlan;
	// Reverse-input exact approval입니다.
	FCFBatchDefinitionApplyApproval ReverseApproval;
	TArray<UCFVehicleRecipeData*> ReverseRecipes = {SecondFixture.Recipe, FirstFixture.Recipe};
	if (!TestTrue(TEXT("Reverse plan/approval builds"), CFBatchApplyTestsPrivate::BuildPlanAndApproval(ReverseRecipes, ReversePlan, ReverseApproval, Error)))
	{
		AddError(Error);
		return false;
	}

	// Forward-input plan입니다.
	FCFBatchDefinitionApplyPlan ForwardPlan;
	// Forward-input exact approval입니다.
	FCFBatchDefinitionApplyApproval ForwardApproval;
	TArray<UCFVehicleRecipeData*> ForwardRecipes = {FirstFixture.Recipe, SecondFixture.Recipe};
	TestTrue(TEXT("Forward plan/approval builds"), CFBatchApplyTestsPrivate::BuildPlanAndApproval(ForwardRecipes, ForwardPlan, ForwardApproval, Error));
	TestEqual(TEXT("Input order does not change BatchApplyPlanHash"), ReversePlan.BatchApplyPlanHash, ForwardPlan.BatchApplyPlanHash);
	TestEqual(TEXT("Input order does not change ordered target set"), ReversePlan.OrderedEligibleTargetPaths, ForwardPlan.OrderedEligibleTargetPaths);
	TestTrue(TEXT("Ordered target set is ascending"), ReversePlan.OrderedEligibleTargetPaths.Num() == 2
		&& ReversePlan.OrderedEligibleTargetPaths[0] < ReversePlan.OrderedEligibleTargetPaths[1]);

	// First Target baseline hash입니다.
	const FString FirstHashBefore = CFBatchApplyTestsPrivate::BuildTargetHash(*FirstFixture.TargetVehicleData, Error);
	// Second Target baseline hash입니다.
	const FString SecondHashBefore = CFBatchApplyTestsPrivate::BuildTargetHash(*SecondFixture.TargetVehicleData, Error);
	// Successful B3 terminal result입니다.
	FCFBatchDefinitionApplyResult ApplyResult;
	TestTrue(TEXT("Canonical B3 sequence succeeds"), CFBatchApplyTestsPrivate::ApplyApprovedPlan(ReversePlan, ReverseApproval, ApplyResult));
	TestEqual(TEXT("Canonical B3 terminal status succeeds"), ApplyResult.Status, ECFBatchApplyStatus::Succeeded);
	TestEqual(TEXT("Both Vehicles applied"), ApplyResult.AppliedVehicleCount, 2);
	TestEqual(TEXT("No Vehicle failed"), ApplyResult.FailedVehicleCount, 0);
	TestEqual(TEXT("No eligible Vehicle remains NotStarted"), ApplyResult.NotStartedVehicleCount, 0);
	TestEqual(TEXT("ApplyService exact total call count"), ApplyResult.ApplyServiceCallCount, 2);
	for (const FCFBatchVehicleApplyResult& VehicleResult : ApplyResult.VehicleResults)
	{
		if (VehicleResult.State == ECFBatchVehicleApplyState::Applied)
		{
			TestEqual(TEXT("Each Applied target calls ApplyService exactly once"), VehicleResult.ApplyServiceCallCount, 1);
			TestTrue(TEXT("Each Applied target reports Target mutation"), VehicleResult.bTargetChanged);
		}
	}
	TestNotEqual(TEXT("First Target changes through shared ApplyService"), CFBatchApplyTestsPrivate::BuildTargetHash(*FirstFixture.TargetVehicleData, Error), FirstHashBefore);
	TestNotEqual(TEXT("Second Target changes through shared ApplyService"), CFBatchApplyTestsPrivate::BuildTargetHash(*SecondFixture.TargetVehicleData, Error), SecondHashBefore);
	TestTrue(TEXT("First Target package dirty"), FirstFixture.TargetPackage->IsDirty());
	TestTrue(TEXT("Second Target package dirty"), SecondFixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Batch never performs global rollback"), ApplyResult.bGlobalRollbackPerformed);
	TestFalse(TEXT("Batch never automatic retries"), ApplyResult.bAutomaticRetryPerformed);
	TestFalse(TEXT("Batch never saves"), ApplyResult.bSavePerformed);
	return true;
}

// 첫 Vehicle 성공 뒤 두 번째 ApplyService 호출 직전 stale을 주입해 Applied/Failed/NotStarted partial result를 검증합니다.
bool FCFBatchApplyPartialTest::RunTest(const FString& Parameters)
{
	// First eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture FirstFixture;
	// Second eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture SecondFixture;
	// Third eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture ThirdFixture;
	// Fixture/plan diagnostics입니다.
	FString Error;
	if (!TestTrue(TEXT("Partial first fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("Partial second fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(SecondFixture, Error))
		|| !TestTrue(TEXT("Partial third fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(ThirdFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*FirstFixture.Recipe);
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*SecondFixture.Recipe);
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*ThirdFixture.Recipe);

	// Reviewed fresh B3 plan입니다.
	FCFBatchDefinitionApplyPlan Plan;
	// Exact B3 approval입니다.
	FCFBatchDefinitionApplyApproval Approval;
	TArray<UCFVehicleRecipeData*> Recipes = {ThirdFixture.Recipe, FirstFixture.Recipe, SecondFixture.Recipe};
	if (!TestTrue(TEXT("Partial plan/approval builds"), CFBatchApplyTestsPrivate::BuildPlanAndApproval(Recipes, Plan, Approval, Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Partial plan has three eligible Vehicles"), Plan.EligibleVehicleCount, 3);

	// Canonical first Target path입니다.
	const FString FirstTargetPath = Plan.OrderedEligibleTargetPaths[0];
	// Canonical second Target path입니다.
	const FString SecondTargetPath = Plan.OrderedEligibleTargetPaths[1];
	// Canonical third Target path입니다.
	const FString ThirdTargetPath = Plan.OrderedEligibleTargetPaths[2];
	// Third target pre-run hash입니다.
	UCFVehicleData* ThirdTarget = nullptr;
	for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
	{
		if (Item.TargetPath == ThirdTargetPath)
		{
			ThirdTarget = Item.TargetVehicleData;
			break;
		}
	}
	TestNotNull(TEXT("Partial third target resolves"), ThirdTarget);
	// Stop-On-First-Failure 뒤 보존되어야 할 third Target baseline입니다.
	const FString ThirdHashBefore = ThirdTarget ? CFBatchApplyTestsPrivate::BuildTargetHash(*ThirdTarget, Error) : FString();

	// Explicit B3 request입니다.
	FCFBatchDefinitionApplyRequest ApplyRequest;
	ApplyRequest.Plan = &Plan;
	ApplyRequest.Approval = &Approval;
	ApplyRequest.bDefinitionApplyApproved = true;
	// Sequential partial-failure terminal result입니다.
	FCFBatchDefinitionApplyResult ApplyResult;
	const bool bApplySucceeded = FCFBatchApplyTestAccess::ApplyWithPreApplyHook(
		ApplyRequest,
		ApplyResult,
		[](const int32 ExecutionIndex, UCFVehicleData* TargetVehicleData)
		{
			if (ExecutionIndex == 1 && TargetVehicleData)
			{
				TargetVehicleData->DestroyedFxSocketName = TEXT("B3_SecondApply_StaleInjection");
			}
		});
	TestFalse(TEXT("Second ApplyService stale produces partial failure"), bApplySucceeded);
	TestEqual(TEXT("Partial terminal status"), ApplyResult.Status, ECFBatchApplyStatus::PartialFailure);
	TestEqual(TEXT("Partial stable error taxonomy"), ApplyResult.ErrorCode, ECFBatchApplyErrorCode::VehicleApplyFailed);
	TestEqual(TEXT("Exactly one Vehicle applied before failure"), ApplyResult.AppliedVehicleCount, 1);
	TestEqual(TEXT("Exactly one Vehicle failed"), ApplyResult.FailedVehicleCount, 1);
	TestEqual(TEXT("Exactly one Vehicle remains NotStarted"), ApplyResult.NotStartedVehicleCount, 1);
	TestEqual(TEXT("Only first and failed second call ApplyService"), ApplyResult.ApplyServiceCallCount, 2);

	// Canonical first result row입니다.
	const FCFBatchVehicleApplyResult* FirstResult = ApplyResult.VehicleResults.FindByPredicate([&FirstTargetPath](const FCFBatchVehicleApplyResult& Item)
	{
		return Item.TargetPath == FirstTargetPath;
	});
	// Canonical second result row입니다.
	const FCFBatchVehicleApplyResult* SecondResult = ApplyResult.VehicleResults.FindByPredicate([&SecondTargetPath](const FCFBatchVehicleApplyResult& Item)
	{
		return Item.TargetPath == SecondTargetPath;
	});
	// Canonical third result row입니다.
	const FCFBatchVehicleApplyResult* ThirdResult = ApplyResult.VehicleResults.FindByPredicate([&ThirdTargetPath](const FCFBatchVehicleApplyResult& Item)
	{
		return Item.TargetPath == ThirdTargetPath;
	});
	TestNotNull(TEXT("Partial first result exists"), FirstResult);
	TestNotNull(TEXT("Partial second result exists"), SecondResult);
	TestNotNull(TEXT("Partial third result exists"), ThirdResult);
	if (FirstResult && SecondResult && ThirdResult)
	{
		TestEqual(TEXT("First target remains Applied"), FirstResult->State, ECFBatchVehicleApplyState::Applied);
		TestEqual(TEXT("First target calls ApplyService once"), FirstResult->ApplyServiceCallCount, 1);
		TestTrue(TEXT("First target mutation remains committed"), FirstResult->bTargetChanged);
		TestEqual(TEXT("Second target is Failed"), SecondResult->State, ECFBatchVehicleApplyState::Failed);
		TestEqual(TEXT("Second target calls ApplyService once"), SecondResult->ApplyServiceCallCount, 1);
		TestEqual(TEXT("Second stale is caught by ApplyService precondition"), SecondResult->ApplyFailureCode, ECFVehicleApplyFailureCode::PreviewOutOfDate);
		TestEqual(TEXT("Third target is NotStarted"), ThirdResult->State, ECFBatchVehicleApplyState::NotStarted);
		TestEqual(TEXT("Third target never calls ApplyService"), ThirdResult->ApplyServiceCallCount, 0);
	}
	if (ThirdTarget)
	{
		TestEqual(TEXT("NotStarted third Target is unchanged"), CFBatchApplyTestsPrivate::BuildTargetHash(*ThirdTarget, Error), ThirdHashBefore);
	}
	TestFalse(TEXT("Partial failure does not global rollback first success"), ApplyResult.bGlobalRollbackPerformed);
	TestFalse(TEXT("Partial failure does not automatic retry"), ApplyResult.bAutomaticRetryPerformed);
	TestFalse(TEXT("Partial failure does not save"), ApplyResult.bSavePerformed);
	return true;
}

// NoChange/ExternalDrift/ResolveBlocked를 ineligible로 분리하고 exact B3 approval tamper가 mutation0으로 막히는지 검증합니다.
bool FCFBatchApplyEligibilityTest::RunTest(const FString& Parameters)
{
	// Eligible fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture EligibleFixture;
	// NoChange fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture NoChangeFixture;
	// External Drift fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture DriftFixture;
	// Resolve-blocked fixture입니다.
	CFBatchApplyTestsPrivate::FApplyFixture BlockedFixture;
	// Fixture/plan diagnostics입니다.
	FString Error;
	if (!TestTrue(TEXT("Eligibility eligible fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(EligibleFixture, Error))
		|| !TestTrue(TEXT("Eligibility no-change fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(NoChangeFixture, Error))
		|| !TestTrue(TEXT("Eligibility drift fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(DriftFixture, Error))
		|| !TestTrue(TEXT("Eligibility blocked fixture builds"), CFBatchApplyTestsPrivate::BuildImportedFixture(BlockedFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*EligibleFixture.Recipe);
	CFBatchApplyTestsPrivate::ConfigureApplyDifference(*DriftFixture.Recipe);

	// Drift fixture를 먼저 정상 Apply해 AppliedState/Target baseline을 확립합니다.
	FCFBatchDefinitionApplyPlan DriftSeedPlan;
	// Drift seed exact approval입니다.
	FCFBatchDefinitionApplyApproval DriftSeedApproval;
	TArray<UCFVehicleRecipeData*> DriftSeedRecipes = {DriftFixture.Recipe};
	if (!TestTrue(TEXT("Drift seed plan/approval builds"), CFBatchApplyTestsPrivate::BuildPlanAndApproval(DriftSeedRecipes, DriftSeedPlan, DriftSeedApproval, Error)))
	{
		AddError(Error);
		return false;
	}
	// Drift seed successful apply result입니다.
	FCFBatchDefinitionApplyResult DriftSeedResult;
	if (!TestTrue(TEXT("Drift seed Apply succeeds"), CFBatchApplyTestsPrivate::ApplyApprovedPlan(DriftSeedPlan, DriftSeedApproval, DriftSeedResult)))
	{
		return false;
	}
	// AppliedState baseline 이후의 manual Target external edit입니다.
	DriftFixture.TargetVehicleData->DestroyedFxSocketName = TEXT("B3_Eligibility_ExternalDrift");

	// Managed 상태에서 required Profiles를 의도적으로 제거해 Resolver Blocked fixture를 만듭니다.
	BlockedFixture.Recipe->ImportState.ManageState = ECFVehicleManageState::Managed;
	BlockedFixture.Recipe->ProfileBindings = FCFVehicleProfileBindings();
	++BlockedFixture.Recipe->AuthoringRevision;

	// Mixed eligibility fresh plan request입니다.
	FCFBatchDefinitionApplyPlanRequest PlanRequest;
	PlanRequest.Recipes = {BlockedFixture.Recipe, DriftFixture.Recipe, NoChangeFixture.Recipe, EligibleFixture.Recipe};
	PlanRequest.CallerKind = ECFAuthoringCallerKind::Automation;
	// Mixed eligibility plan입니다.
	FCFBatchDefinitionApplyPlan Plan;
	if (!TestTrue(TEXT("Mixed eligibility plan builds"), FCFBatchApplyService::BuildDefinitionApplyPlan(PlanRequest, Plan, Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Only one mixed item is eligible"), Plan.EligibleVehicleCount, 1);
	TestEqual(TEXT("Three mixed items are ineligible"), Plan.IneligibleVehicleCount, 3);

	// Eligibility별 observed count입니다.
	int32 NoChangeCount = 0;
	// External Drift observed count입니다.
	int32 DriftCount = 0;
	// Resolve Blocked observed count입니다.
	int32 ResolveBlockedCount = 0;
	for (const FCFBatchDefinitionApplyItem& Item : Plan.Items)
	{
		NoChangeCount += Item.Eligibility == ECFBatchApplyEligibility::NoChange ? 1 : 0;
		DriftCount += Item.Eligibility == ECFBatchApplyEligibility::ExternalDrift ? 1 : 0;
		ResolveBlockedCount += Item.Eligibility == ECFBatchApplyEligibility::ResolveBlocked ? 1 : 0;
	}
	TestEqual(TEXT("NoChange is ineligible"), NoChangeCount, 1);
	TestEqual(TEXT("External Drift is ineligible"), DriftCount, 1);
	TestEqual(TEXT("Resolve Blocked is ineligible"), ResolveBlockedCount, 1);

	// Mixed plan exact B3 approval입니다.
	FCFBatchDefinitionApplyApproval Approval;
	if (!TestTrue(TEXT("Mixed eligibility B3 approval builds"), FCFBatchApplyService::BuildDefinitionApplyApproval(Plan, Approval, Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("B3 approval binds only eligible target"), Approval.OrderedTargetPaths.Num(), 1);
	TestFalse(TEXT("B3 approval auto-save stays false"), Approval.bAutoSave);

	// Approval tamper 전에 eligible Target hash를 보존합니다.
	const FString EligibleHashBefore = CFBatchApplyTestsPrivate::BuildTargetHash(*EligibleFixture.TargetVehicleData, Error);
	// Per-target R3 evidence를 tamper한 invalid B3 approval입니다.
	FCFBatchDefinitionApplyApproval TamperedApproval = Approval;
	TamperedApproval.PerTargetEvidence[0].R3EvidenceHash = TEXT("tampered-r3-evidence");
	// Tampered approval request입니다.
	FCFBatchDefinitionApplyRequest ApplyRequest;
	ApplyRequest.Plan = &Plan;
	ApplyRequest.Approval = &TamperedApproval;
	ApplyRequest.bDefinitionApplyApproved = true;
	// Tampered approval terminal result입니다.
	FCFBatchDefinitionApplyResult ApplyResult;
	TestFalse(TEXT("Tampered B3 approval blocks before mutation"), FCFBatchApplyService::ApplyBatch(ApplyRequest, ApplyResult));
	TestEqual(TEXT("Tampered R3 evidence stable taxonomy"), ApplyResult.ErrorCode, ECFBatchApplyErrorCode::PerTargetEvidenceMismatch);
	TestEqual(TEXT("Tampered approval calls ApplyService zero times"), ApplyResult.ApplyServiceCallCount, 0);
	TestEqual(TEXT("Tampered approval keeps eligible Target unchanged"), CFBatchApplyTestsPrivate::BuildTargetHash(*EligibleFixture.TargetVehicleData, Error), EligibleHashBefore);
	return true;
}



#endif
