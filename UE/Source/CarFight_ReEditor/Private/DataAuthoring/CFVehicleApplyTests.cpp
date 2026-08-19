// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleApplyTests.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08H Apply Transaction Foundation Automation입니다.
// Changelog:
// - v1.0.0: dependency-safe success, five frozen TOCTOU preconditions + reviewed diff freshness, post-mutation atomic rollback/no-auto-save 검증 추가.
// Migration:
// - 테스트는 in-memory /Temp package와 transient StaticMesh만 사용하며 Content Asset을 생성하거나 저장하지 않습니다.
// - Target mutation은 검증 대상인 FCFVehicleApplyService를 통해서만 수행합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleApplyService.h"
#include "DataAuthoring/CFVehicleApplyTestAccess.h"
#include "DataAuthoring/CFVehicleAssetReader.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleApplyTestsPrivate
{
	/** Apply Automation 한 건에 필요한 in-memory Target/Recipe/Preview 상태입니다. */
	struct FApplyFixture
	{
		// Target VehicleData를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* RecipePackage = nullptr;

		// ApplyService만 persistent-style mutation할 test Target입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// AppliedState가 갱신될 Editor-only test Recipe입니다.
		UCFVehicleRecipeData* Recipe = nullptr;

		// Target visual/layout validator가 사용할 transient Chassis mesh입니다.
		UStaticMesh* ChassisMesh = nullptr;

		// 사용자가 검토한 Preview의 Current Target Snapshot입니다.
		FCFVehicleDefinitionSnapshot PreviewTargetSnapshot;

		// 사용자가 검토한 immutable Resolver request입니다.
		FCFVehicleResolveRequest ResolveRequest;

		// 사용자가 승인한 Resolver result입니다.
		FCFVehicleResolveResult ApprovedResolveResult;

		// 실제 ApplyService 호출에 사용할 Frozen TOCTOU request입니다.
		FCFVehicleApplyRequest ApplyRequest;
	};

	// Automation object를 격리할 unique /Temp package를 만듭니다.
	UPackage* CreateTestPackage(const TCHAR* Prefix)
	{
		// 다른 Automation run과 충돌하지 않는 unique long package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// Transient Chassis에 validator/asset reader가 실제로 읽을 socket을 추가합니다.
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

	// Validator를 통과하는 Current Target과 old Hardpoint baseline을 구성합니다.
	bool ConfigureValidTarget(FApplyFixture& InOutFixture, FString& OutError)
	{
		InOutFixture.TargetPackage = CreateTestPackage(TEXT("CFDAApplyTarget"));
		if (!InOutFixture.TargetPackage)
		{
			OutError = TEXT("Apply Target /Temp package를 만들 수 없습니다.");
			return false;
		}

		// ApplyService mutation 대상 test VehicleData입니다.
		InOutFixture.TargetVehicleData = NewObject<UCFVehicleData>(InOutFixture.TargetPackage, TEXT("DA_ApplyTarget_Test"), RF_Transactional);
		if (!InOutFixture.TargetVehicleData)
		{
			OutError = TEXT("Apply Target VehicleData를 만들 수 없습니다.");
			return false;
		}

		// Layout/Hardpoint socket source로 사용할 transient Chassis mesh입니다.
		InOutFixture.ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		if (!InOutFixture.ChassisMesh)
		{
			OutError = TEXT("Apply transient Chassis StaticMesh를 만들 수 없습니다.");
			return false;
		}

		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_FL"), FVector(110.0, -62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_FR"), FVector(110.0, 62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_RL"), FVector(-108.0, -62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("Wheel_Anchor_RR"), FVector(-108.0, 62.0, 28.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("HP_Top_Old"), FVector(0.0, 0.0, 82.0));
		AddSocket(*InOutFixture.ChassisMesh, TEXT("HP_Front_New"), FVector(128.0, 0.0, 44.0));

		// Non-zero bounds와 persistent Engine path를 제공하는 read-only Wheel mesh입니다.
		UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!WheelMesh)
		{
			OutError = TEXT("Apply fixture용 Engine Cube wheel mesh를 로드할 수 없습니다.");
			return false;
		}

		// Validator의 required Wheel Class reference에 사용할 loaded ChaosVehicles class입니다.
		UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
		if (!WheelClass)
		{
			OutError = TEXT("ChaosVehicleWheel class를 로드할 수 없습니다.");
			return false;
		}

		// Current target visual references입니다.
		InOutFixture.TargetVehicleData->VehicleVisualConfig.ChassisMesh = InOutFixture.ChassisMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFL = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFR = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRL = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRR = WheelMesh;
		InOutFixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass = WheelClass;
		InOutFixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass = WheelClass;

		// Asset-derived layout과 exact same values를 가진 valid Current layout입니다.
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

		// Fitting validator가 legacy-unset special case 대신 explicit valid mass pair를 검사하게 합니다.
		InOutFixture.TargetVehicleData->BaseVehicleMassKg = 1540.0f;
		InOutFixture.TargetVehicleData->MaximumGrossMassKg = 2280.0f;

		InOutFixture.TargetVehicleData->HardpointSlots.Reset();
		// Current Target에만 존재하고 Apply에서 제거될 old Hardpoint입니다.
		FCFVehicleHardpointSlot& OldHardpoint = InOutFixture.TargetVehicleData->HardpointSlots.AddDefaulted_GetRef();
		OldHardpoint.LocationSlotId = TEXT("Top_Old");
		OldHardpoint.LocationCategory = TEXT("Top");
		OldHardpoint.SocketName = TEXT("HP_Top_Old");
		OldHardpoint.LocalLocation = FVector(0.0, 0.0, 82.0);
		OldHardpoint.LocalRotation = FRotator::ZeroRotator;

		// Current Target에는 Mount가 없어 new Mount add dependency를 명확히 검증할 수 있습니다.
		InOutFixture.TargetVehicleData->MountProfiles.Reset();
		OutError.Reset();
		return true;
	}

	// Initial Import baseline에서 Hardpoints만 이미 승인된 Adoption 상태처럼 release합니다.
	void ReleaseHardpointLegacyPins(UCFVehicleRecipeData& Recipe)
	{
		Recipe.ImportState.LegacyPinnedFields.RemoveAll([](const FCFVehicleFieldOverride& Override)
		{
			return Override.FieldPath.CollectionPropertyName == TEXT("HardpointSlots");
		});
		Recipe.ImportState.AdoptedGroups.Add(ECFVehicleAdoptGroup::Hardpoints);
		Recipe.ImportState.ManageState = Recipe.ImportState.LegacyPinnedFields.IsEmpty()
			? ECFVehicleManageState::Managed
			: ECFVehicleManageState::PartiallyManaged;
	}

	// Imported Recipe를 old Hardpoint → new Hardpoint + new Mount desired state로 편집합니다.
	void ConfigureDesiredRecipe(UCFVehicleRecipeData& Recipe)
	{
		ReleaseHardpointLegacyPins(Recipe);
		Recipe.HardpointIntents.Reset();
		// Desired Definition에 추가할 new Hardpoint semantic intent입니다.
		FCFHardpointIntent& NewHardpoint = Recipe.HardpointIntents.AddDefaulted_GetRef();
		NewHardpoint.LocationSlotId = TEXT("Front_New");
		NewHardpoint.LocationCategory = TEXT("Front");
		NewHardpoint.SocketName = TEXT("HP_Front_New");

		Recipe.MountIntents.Reset();
		// New Hardpoint를 참조하므로 Hardpoint add보다 뒤에 생성되어야 하는 new Mount입니다.
		FCFMountIntent& NewMount = Recipe.MountIntents.AddDefaulted_GetRef();
		NewMount.MountProfileId = TEXT("M_Front_New");
		NewMount.LocationSlotRef = TEXT("Front_New");
		NewMount.MountType = ECFVehicleMountType::Turret;
		NewMount.SizeLimit = ECFVehicleWeaponSize::Medium;
		NewMount.DefaultEquipmentPresetData = nullptr;
		NewMount.bExposedModule = true;

		// Recipe semantic edit의 diagnostic revision입니다.
		++Recipe.AuthoringRevision;
	}

	// Persistent Recipe/Target truth와 current Asset facts로 reviewed Resolve Preview를 만듭니다.
	bool BuildReviewedPreview(FApplyFixture& InOutFixture, FString& OutError)
	{
		InOutFixture.ResolveRequest = FCFVehicleResolveRequest();
		InOutFixture.ResolveRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*InOutFixture.Recipe, InOutFixture.ResolveRequest.Recipe, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(InOutFixture.ResolveRequest.ProjectDefaults, OutError)
			|| !FCFVehicleAssetReader::BuildAssetSnapshot(InOutFixture.ResolveRequest.Recipe, InOutFixture.ResolveRequest.Assets, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*InOutFixture.TargetVehicleData, InOutFixture.PreviewTargetSnapshot, OutError))
		{
			return false;
		}

		InOutFixture.ResolveRequest.bHasCurrentDefinition = true;
		InOutFixture.ResolveRequest.CurrentDefinition = InOutFixture.PreviewTargetSnapshot;
		if (!FCFVehicleResolver::Resolve(InOutFixture.ResolveRequest, InOutFixture.ApprovedResolveResult))
		{
			OutError = TEXT("Apply fixture reviewed Resolver가 internal Error로 실패했습니다.");
			return false;
		}
		if (InOutFixture.ApprovedResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
		{
			OutError = FString::Printf(
				TEXT("Apply fixture reviewed Resolver가 Success가 아닙니다. RecipeIssues=%d ResolverIssues=%d DefinitionIssues=%d"),
				InOutFixture.ApprovedResolveResult.RecipeValidation.Num(),
				InOutFixture.ApprovedResolveResult.ResolverValidation.Num(),
				InOutFixture.ApprovedResolveResult.DefinitionValidation.Num());
			return false;
		}

		InOutFixture.ApplyRequest = FCFVehicleApplyRequest();
		InOutFixture.ApplyRequest.Recipe = InOutFixture.Recipe;
		InOutFixture.ApplyRequest.TargetVehicleData = InOutFixture.TargetVehicleData;
		InOutFixture.ApplyRequest.ResolveRequest = InOutFixture.ResolveRequest;
		InOutFixture.ApplyRequest.ApprovedResolveResult = InOutFixture.ApprovedResolveResult;
		InOutFixture.ApplyRequest.ExpectedRecipeFingerprint = InOutFixture.ResolveRequest.Recipe.RecipeFingerprint;
		InOutFixture.ApplyRequest.ExpectedSourceSignature = InOutFixture.ApprovedResolveResult.SourceSignature;
		InOutFixture.ApplyRequest.ExpectedTargetDefinitionHash = InOutFixture.PreviewTargetSnapshot.DefinitionHash;
		InOutFixture.ApplyRequest.ExpectedResolvedDefinitionHash = InOutFixture.ApprovedResolveResult.ResolvedDefinitionHash;
		InOutFixture.ApplyRequest.ExpectedResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		OutError.Reset();
		return true;
	}

	// Valid Current Target → Existing Definition Import → desired Recipe edit → reviewed Preview fixture를 완성합니다.
	bool BuildApplyFixture(FApplyFixture& OutFixture, FString& OutError)
	{
		OutFixture = FApplyFixture();
		if (!ConfigureValidTarget(OutFixture, OutError))
		{
			return false;
		}

		// Initial Import source가 될 exact Current Target Snapshot입니다.
		FCFVehicleDefinitionSnapshot InitialDefinition;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*OutFixture.TargetVehicleData, InitialDefinition, OutError))
		{
			return false;
		}

		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFDAApplyRecipe"));
		if (!OutFixture.RecipePackage)
		{
			OutError = TEXT("Apply Recipe /Temp package를 만들 수 없습니다.");
			return false;
		}

		// AppliedState를 함께 transaction할 test Recipe입니다.
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_ApplyRecipe_Test"), RF_Transactional);
		if (!OutFixture.Recipe)
		{
			OutError = TEXT("Apply Recipe를 만들 수 없습니다.");
			return false;
		}
		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;

		// Current Definition 전체를 lossless Legacy baseline으로 가져오는 P0-08G 결과입니다.
		FCFVehicleImportResult ImportResult;
		if (!FCFVehicleImportService::ImportDefinitionSnapshot(InitialDefinition, *OutFixture.Recipe, ImportResult, OutError))
		{
			return false;
		}

		ConfigureDesiredRecipe(*OutFixture.Recipe);
		return BuildReviewedPreview(OutFixture, OutError);
	}

	// Exact stable selector Hardpoint가 Target에 존재하는지 검사합니다.
	const FCFVehicleHardpointSlot* FindHardpoint(const UCFVehicleData& Target, const FName LocationSlotId)
	{
		return Target.HardpointSlots.FindByPredicate([LocationSlotId](const FCFVehicleHardpointSlot& Hardpoint)
		{
			return Hardpoint.LocationSlotId == LocationSlotId;
		});
	}

	// Exact stable selector Mount가 Target에 존재하는지 검사합니다.
	const FCFVehicleMountProfile* FindMount(const UCFVehicleData& Target, const FName MountProfileId)
	{
		return Target.MountProfiles.FindByPredicate([MountProfileId](const FCFVehicleMountProfile& Mount)
		{
			return Mount.MountProfileId == MountProfileId;
		});
	}

	// Current Target full Definition hash를 fresh Snapshot으로 반환합니다.
	FString BuildTargetHash(const UCFVehicleData& Target, FString& OutError)
	{
		// Hash authority로 사용할 fresh full Definition Snapshot입니다.
		FCFVehicleDefinitionSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.DefinitionHash;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleApplySuccessTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Apply.SuccessAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleApplyPreconditionTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Apply.Preconditions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleApplyRollbackTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Apply.Rollback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Reviewed Diff를 dependency-safe 순서로 Target에 적용하고 AppliedState/no-auto-save 계약을 검증합니다.
bool FCFVehicleApplySuccessTest::RunTest(const FString& Parameters)
{
	// Apply success fixture입니다.
	CFVehicleApplyTestsPrivate::FApplyFixture Fixture;
	// Fixture build 또는 Apply 실패 사유입니다.
	FString Error;
	if (!TestTrue(TEXT("Apply success fixture builds"), CFVehicleApplyTestsPrivate::BuildApplyFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	TestNotNull(TEXT("Old Hardpoint exists before Apply"), CFVehicleApplyTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Top_Old")));
	TestNull(TEXT("New Hardpoint is absent before Apply"), CFVehicleApplyTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Front_New")));
	TestNull(TEXT("New Mount is absent before Apply"), CFVehicleApplyTestsPrivate::FindMount(*Fixture.TargetVehicleData, TEXT("M_Front_New")));
	TestTrue(TEXT("Reviewed Preview contains actual field diff"), Fixture.ApprovedResolveResult.FieldDiff.Num() > 0);

	// Apply 자체가 Dirty를 만든다는 것을 분리해서 보기 위해 setup의 dirty state를 지웁니다.
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Central ApplyService 결과입니다.
	FCFVehicleApplyResult ApplyResult;
	if (!TestTrue(TEXT("Central ApplyService succeeds"), FCFVehicleApplyService::Apply(Fixture.ApplyRequest, ApplyResult)))
	{
		AddError(ApplyResult.Message);
		return false;
	}

	TestEqual(TEXT("Apply status is Success"), ApplyResult.Status, ECFVehicleApplyStatus::Success);
	TestEqual(TEXT("Apply failure code is None"), ApplyResult.FailureCode, ECFVehicleApplyFailureCode::None);
	TestTrue(TEXT("Target mutation is committed"), ApplyResult.bTargetMutationCommitted);
	TestTrue(TEXT("Recipe AppliedState is committed"), ApplyResult.bRecipeAppliedStateUpdated);
	TestEqual(TEXT("Applied projection hash equals reviewed resolved hash"), ApplyResult.AppliedDefinitionHash, Fixture.ApplyRequest.ExpectedResolvedDefinitionHash);

	// R14에는 removal row가 없으므로 actual plan이 current-only old selector 제거를 하나 이상 보완해야 합니다.
	TestTrue(TEXT("Apply plan includes synthesized current-only removal"), ApplyResult.AppliedDiffOperationCount > Fixture.ApprovedResolveResult.FieldDiff.Num());
	TestNull(TEXT("Old Hardpoint is removed"), CFVehicleApplyTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Top_Old")));
	// Dependency-safe order가 성공했다면 new Mount가 참조할 Hardpoint가 먼저 존재해야 합니다.
	const FCFVehicleHardpointSlot* NewHardpoint = CFVehicleApplyTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Front_New"));
	if (TestNotNull(TEXT("New Hardpoint is added"), NewHardpoint))
	{
		TestEqual(TEXT("New Hardpoint socket-derived location is applied"), NewHardpoint->LocalLocation, FVector(128.0, 0.0, 44.0));
	}
	// New Mount result입니다.
	const FCFVehicleMountProfile* NewMount = CFVehicleApplyTestsPrivate::FindMount(*Fixture.TargetVehicleData, TEXT("M_Front_New"));
	if (TestNotNull(TEXT("New Mount is added"), NewMount))
	{
		TestEqual(TEXT("New Mount references newly added Hardpoint"), NewMount->LocationSlotRef, FName(TEXT("Front_New")));
	}

	TestEqual(TEXT("AppliedState recipe fingerprint"), Fixture.Recipe->AppliedState.AppliedRecipeFingerprint, Fixture.ApplyRequest.ExpectedRecipeFingerprint);
	TestEqual(TEXT("AppliedState source signature"), Fixture.Recipe->AppliedState.AppliedSourceSignature, Fixture.ApplyRequest.ExpectedSourceSignature);
	TestEqual(TEXT("AppliedState definition hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, Fixture.ApplyRequest.ExpectedResolvedDefinitionHash);
	TestEqual(TEXT("AppliedState resolver revision"), Fixture.Recipe->AppliedState.ResolverContractRevision, Fixture.ApplyRequest.ExpectedResolverContractRevision);
	TestEqual(TEXT("AppliedState trace count covers all resolved fields"), Fixture.Recipe->AppliedState.FieldTraces.Num(), Fixture.ApprovedResolveResult.SortedResolvedFields.Num());

	// Apply는 Save가 아니라 Package Dirty까지이므로 둘 다 dirty 상태로 남아 있어야 합니다.
	TestTrue(TEXT("Target package remains dirty for explicit user save"), Fixture.TargetPackage->IsDirty());
	TestTrue(TEXT("Recipe package remains dirty for explicit user save"), Fixture.RecipePackage->IsDirty());
	return true;
}

// Frozen Recipe/Source/Target/Resolved/ResolverRevision precondition과 reviewed Diff freshness가 mutation 전에 차단되는지 검증합니다.
bool FCFVehicleApplyPreconditionTest::RunTest(const FString& Parameters)
{
	// TOCTOU precondition fixture입니다.
	CFVehicleApplyTestsPrivate::FApplyFixture Fixture;
	// Fixture build 또는 snapshot 실패 사유입니다.
	FString Error;
	if (!TestTrue(TEXT("Apply precondition fixture builds"), CFVehicleApplyTestsPrivate::BuildApplyFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// 모든 rejected request가 보존해야 할 exact Target baseline hash입니다.
	const FString BaselineTargetHash = CFVehicleApplyTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (!TestFalse(TEXT("Baseline target hash is not empty"), BaselineTargetHash.IsEmpty()))
	{
		AddError(Error);
		return false;
	}
	// Apply 전 persistent AppliedState baseline입니다.
	const FString BaselineAppliedHash = Fixture.Recipe->AppliedState.AppliedDefinitionHash;

	// Rejected request 하나가 PreviewOutOfDate 계열로 mutation 0인지 공통 검증합니다.
	auto VerifyRejectedRequest = [this, &Fixture, &BaselineTargetHash, &BaselineAppliedHash, &Error](
		const TCHAR* Label,
		const FCFVehicleApplyRequest& RejectedRequest,
		const ECFVehicleApplyFailureCode ExpectedFailureCode) -> bool
	{
		// Rejected Apply 결과입니다.
		FCFVehicleApplyResult RejectedResult;
		const bool bApplied = FCFVehicleApplyService::Apply(RejectedRequest, RejectedResult);
		TestFalse(FString::Printf(TEXT("%s is rejected"), Label), bApplied);
		TestEqual(FString::Printf(TEXT("%s failure code"), Label), RejectedResult.FailureCode, ExpectedFailureCode);
		TestFalse(FString::Printf(TEXT("%s never commits target mutation"), Label), RejectedResult.bTargetMutationCommitted);

		// Rejected call 뒤 Target hash입니다.
		const FString CurrentTargetHash = CFVehicleApplyTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
		TestEqual(FString::Printf(TEXT("%s keeps Target hash"), Label), CurrentTargetHash, BaselineTargetHash);
		TestEqual(FString::Printf(TEXT("%s keeps Recipe AppliedState"), Label), Fixture.Recipe->AppliedState.AppliedDefinitionHash, BaselineAppliedHash);
		return !bApplied && CurrentTargetHash == BaselineTargetHash;
	};

	// Stale Recipe semantic fingerprint request입니다.
	FCFVehicleApplyRequest RecipeStaleRequest = Fixture.ApplyRequest;
	RecipeStaleRequest.ExpectedRecipeFingerprint = TEXT("stale-recipe-fingerprint");
	VerifyRejectedRequest(TEXT("Recipe fingerprint precondition"), RecipeStaleRequest, ECFVehicleApplyFailureCode::PreviewOutOfDate);

	// Stale effective source signature request입니다.
	FCFVehicleApplyRequest SourceStaleRequest = Fixture.ApplyRequest;
	SourceStaleRequest.ExpectedSourceSignature = TEXT("stale-source-signature");
	VerifyRejectedRequest(TEXT("Source signature precondition"), SourceStaleRequest, ECFVehicleApplyFailureCode::PreviewOutOfDate);

	// Stale Current Target hash request입니다.
	FCFVehicleApplyRequest TargetStaleRequest = Fixture.ApplyRequest;
	TargetStaleRequest.ExpectedTargetDefinitionHash = TEXT("stale-target-definition-hash");
	VerifyRejectedRequest(TEXT("Target hash precondition"), TargetStaleRequest, ECFVehicleApplyFailureCode::PreviewOutOfDate);

	// Stale expected resolved Definition hash request입니다.
	FCFVehicleApplyRequest ResolvedStaleRequest = Fixture.ApplyRequest;
	ResolvedStaleRequest.ExpectedResolvedDefinitionHash = TEXT("stale-resolved-definition-hash");
	VerifyRejectedRequest(TEXT("Resolved hash precondition"), ResolvedStaleRequest, ECFVehicleApplyFailureCode::PreviewOutOfDate);

	// Stale Resolver semantic contract revision request입니다.
	FCFVehicleApplyRequest RevisionStaleRequest = Fixture.ApplyRequest;
	RevisionStaleRequest.ExpectedResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision + 1;
	VerifyRejectedRequest(TEXT("Resolver revision precondition"), RevisionStaleRequest, ECFVehicleApplyFailureCode::PreviewOutOfDate);

	if (Fixture.ApplyRequest.ApprovedResolveResult.FieldDiff.IsEmpty())
	{
		AddError(TEXT("Reviewed diff mismatch probe에 필요한 FieldDiff가 없습니다."));
		return false;
	}
	// Approved result의 reviewed Diff row 하나만 제거한 tampered request입니다.
	FCFVehicleApplyRequest DiffTamperedRequest = Fixture.ApplyRequest;
	DiffTamperedRequest.ApprovedResolveResult.FieldDiff.RemoveAt(DiffTamperedRequest.ApprovedResolveResult.FieldDiff.Num() - 1);
	VerifyRejectedRequest(TEXT("Reviewed diff freshness"), DiffTamperedRequest, ECFVehicleApplyFailureCode::ReviewedDiffMismatch);
	return true;
}

// Actual Target A8 mutation 뒤 강제 실패에서 Target/AppliedState/dirty state가 자체 rollback되는지 검증합니다.
bool FCFVehicleApplyRollbackTest::RunTest(const FString& Parameters)
{
	// Post-mutation rollback fixture입니다.
	CFVehicleApplyTestsPrivate::FApplyFixture Fixture;
	// Fixture build 또는 rollback snapshot 실패 사유입니다.
	FString Error;
	if (!TestTrue(TEXT("Apply rollback fixture builds"), CFVehicleApplyTestsPrivate::BuildApplyFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Recipe fingerprint에는 포함되지 않는 pre-existing AppliedState sentinel입니다.
	Fixture.Recipe->AppliedState.AppliedRecipeRevision = 123;
	Fixture.Recipe->AppliedState.AppliedRecipeFingerprint = TEXT("sentinel-recipe");
	Fixture.Recipe->AppliedState.AppliedSourceSignature = TEXT("sentinel-source");
	Fixture.Recipe->AppliedState.AppliedDefinitionHash = TEXT("sentinel-definition");
	Fixture.Recipe->AppliedState.ResolverContractRevision = 77;
	// Rollback 뒤 정확히 복원돼야 할 AppliedState copy입니다.
	const FCFVehicleAppliedState AppliedStateBeforeFailure = Fixture.Recipe->AppliedState;
	// Rollback 뒤 정확히 복원돼야 할 Target full hash입니다.
	const FString TargetHashBeforeFailure = CFVehicleApplyTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	if (!TestFalse(TEXT("Rollback baseline target hash is not empty"), TargetHashBeforeFailure.IsEmpty()))
	{
		AddError(Error);
		return false;
	}

	// Setup mutation과 rollback mutation을 구분하기 위한 pre-apply package dirty state입니다.
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// A8 Target mutation 직후 의도적 failure를 주입한 private Automation 결과입니다.
	FCFVehicleApplyResult RollbackResult;
	TestFalse(TEXT("Injected post-mutation Apply fails"), FCFVehicleApplyTestAccess::ApplyWithPostMutationFailure(Fixture.ApplyRequest, RollbackResult));
	TestEqual(TEXT("Injected failure reports TargetApplyFailed"), RollbackResult.FailureCode, ECFVehicleApplyFailureCode::TargetApplyFailed);
	TestTrue(TEXT("Service verifies rollback"), RollbackResult.bRollbackVerified);
	TestFalse(TEXT("Failed transaction never commits target mutation"), RollbackResult.bTargetMutationCommitted);
	TestFalse(TEXT("Failed transaction never commits AppliedState"), RollbackResult.bRecipeAppliedStateUpdated);

	// Rollback 뒤 fresh Target full hash입니다.
	const FString TargetHashAfterFailure = CFVehicleApplyTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	TestEqual(TEXT("Rollback restores exact Target Definition hash"), TargetHashAfterFailure, TargetHashBeforeFailure);
	TestNotNull(TEXT("Rollback restores old Hardpoint"), CFVehicleApplyTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Top_Old")));
	TestNull(TEXT("Rollback removes partially added new Hardpoint"), CFVehicleApplyTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Front_New")));
	TestNull(TEXT("Rollback removes partially added new Mount"), CFVehicleApplyTestsPrivate::FindMount(*Fixture.TargetVehicleData, TEXT("M_Front_New")));

	TestEqual(TEXT("Rollback restores AppliedRecipeRevision"), Fixture.Recipe->AppliedState.AppliedRecipeRevision, AppliedStateBeforeFailure.AppliedRecipeRevision);
	TestEqual(TEXT("Rollback restores AppliedRecipeFingerprint"), Fixture.Recipe->AppliedState.AppliedRecipeFingerprint, AppliedStateBeforeFailure.AppliedRecipeFingerprint);
	TestEqual(TEXT("Rollback restores AppliedSourceSignature"), Fixture.Recipe->AppliedState.AppliedSourceSignature, AppliedStateBeforeFailure.AppliedSourceSignature);
	TestEqual(TEXT("Rollback restores AppliedDefinitionHash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, AppliedStateBeforeFailure.AppliedDefinitionHash);
	TestEqual(TEXT("Rollback restores ResolverContractRevision"), Fixture.Recipe->AppliedState.ResolverContractRevision, AppliedStateBeforeFailure.ResolverContractRevision);
	TestFalse(TEXT("Rollback restores Target package clean flag"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Rollback restores Recipe package clean flag"), Fixture.RecipePackage->IsDirty());
	return true;
}

#endif
