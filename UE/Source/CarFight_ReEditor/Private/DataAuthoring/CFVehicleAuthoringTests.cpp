// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringTests.cpp
// Version: v1.1.0
// Date: 2026-08-17
// Description: DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation Automation입니다.
// Changelog:
// - v1.1.0: Frozen RecipeFingerprint에 포함되지 않는 VehicleArchetype typed write의 실제 commit과 fingerprint-covered concurrent stale block을 분리 검증.
// - v1.0.0: facade/core parity, R0 mutation0, prospective preview mutation0, R1 stale/approval/dedupe, R3 shared Apply lane/no-auto-save/no-auto-retry 검증 추가.
// Migration:
// - 테스트는 /Temp package와 transient fixture만 사용하고 Content Asset을 저장하지 않습니다.
// - Target write 검증은 production과 동일하게 FCFVehicleAuthoringService -> FCFVehicleApplyService 경로만 사용합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleAssetReader.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleAuthoringTestsPrivate
{
	/** Common Authoring facade Automation 한 건에 필요한 in-memory truth입니다. */
	struct FAuthoringFixture
	{
		// Target VehicleData를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* RecipePackage = nullptr;

		// R0 read와 R3 Apply의 canonical Target fixture입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// R0/R1/R3가 공통으로 사용하는 Editor-only Recipe fixture입니다.
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

	// Existing Definition import와 Definition Validator를 모두 통과하는 Target baseline을 구성합니다.
	bool ConfigureValidTarget(FAuthoringFixture& InOutFixture, FString& OutError)
	{
		InOutFixture.TargetPackage = CreateTestPackage(TEXT("CFDAAuthTarget"));
		if (!InOutFixture.TargetPackage)
		{
			OutError = TEXT("Authoring Target /Temp package를 만들 수 없습니다.");
			return false;
		}

		// Shared Apply lane이 사용할 target object입니다.
		InOutFixture.TargetVehicleData = NewObject<UCFVehicleData>(InOutFixture.TargetPackage, TEXT("DA_AuthoringTarget_Test"), RF_Transactional);
		if (!InOutFixture.TargetVehicleData)
		{
			OutError = TEXT("Authoring Target VehicleData를 만들 수 없습니다.");
			return false;
		}

		// Layout/Hardpoint source로 사용할 transient StaticMesh입니다.
		InOutFixture.ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		if (!InOutFixture.ChassisMesh)
		{
			OutError = TEXT("Authoring transient Chassis mesh를 만들 수 없습니다.");
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
			OutError = TEXT("Engine Cube wheel fixture를 로드할 수 없습니다.");
			return false;
		}

		// Existing Validator의 required Wheel Class reference입니다.
		UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
		if (!WheelClass)
		{
			OutError = TEXT("ChaosVehicleWheel class를 로드할 수 없습니다.");
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
		OutError.Reset();
		return true;
	}

	// Current Definition을 Existing Definition import한 Recipe fixture를 생성합니다.
	bool BuildImportedFixture(FAuthoringFixture& OutFixture, FString& OutError)
	{
		OutFixture = FAuthoringFixture();
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

		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFDAAuthRecipe"));
		if (!OutFixture.RecipePackage)
		{
			OutError = TEXT("Authoring Recipe /Temp package를 만들 수 없습니다.");
			return false;
		}

		// Common Authoring facade가 읽고 transaction할 Recipe fixture입니다.
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_AuthoringRecipe_Test"), RF_Transactional);
		if (!OutFixture.Recipe)
		{
			OutError = TEXT("Authoring Recipe를 만들 수 없습니다.");
			return false;
		}
		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;

		// P0-08G shared Import Core로 current Definition을 lossless baseline으로 가져옵니다.
		FCFVehicleImportResult ImportResult;
		if (!FCFVehicleImportService::ImportDefinitionSnapshot(CurrentDefinition, *OutFixture.Recipe, ImportResult, OutError))
		{
			return false;
		}
		OutError.Reset();
		return true;
	}

	// P0-08H와 같은 actual array apply difference를 만들도록 Hardpoint ownership을 release하고 new desired state를 구성합니다.
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
		// New Hardpoint를 참조해 dependency-safe add order를 검증할 semantic Mount입니다.
		FCFMountIntent& NewMount = Recipe.MountIntents.AddDefaulted_GetRef();
		NewMount.MountProfileId = TEXT("M_Front_New");
		NewMount.LocationSlotRef = TEXT("Front_New");
		NewMount.MountType = ECFVehicleMountType::Turret;
		NewMount.SizeLimit = ECFVehicleWeaponSize::Medium;
		NewMount.DefaultEquipmentPresetData = nullptr;
		NewMount.bExposedModule = true;
		++Recipe.AuthoringRevision;
	}

	// Facade와 비교할 direct shared Core resolver request/result를 구성합니다.
	bool BuildDirectResolve(
		const FAuthoringFixture& Fixture,
		FCFVehicleResolveRequest& OutRequest,
		FCFVehicleResolveResult& OutResult,
		FString& OutError)
	{
		OutRequest = FCFVehicleResolveRequest();
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Fixture.Recipe, OutRequest.Recipe, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, nullptr, nullptr, nullptr, nullptr, OutRequest.Profiles, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(OutRequest.ProjectDefaults, OutError)
			|| !FCFVehicleAssetReader::BuildAssetSnapshot(OutRequest.Recipe, OutRequest.Assets, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, OutRequest.CurrentDefinition, OutError))
		{
			return false;
		}
		OutRequest.bHasCurrentDefinition = true;
		OutRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		if (!FCFVehicleResolver::Resolve(OutRequest, OutResult))
		{
			OutError = TEXT("Direct shared Core Resolver가 internal Error로 실패했습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Current Target full Definition hash를 fresh Snapshot으로 반환합니다.
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

	// Persistent Recipe semantic fingerprint를 fresh Snapshot으로 반환합니다.
	FString BuildRecipeFingerprint(const UCFVehicleRecipeData& Recipe, FString& OutError)
	{
		// Fingerprint authority가 될 fresh Recipe Snapshot입니다.
		FCFVehicleRecipeSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.RecipeFingerprint;
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

	// Standard R0 facade request를 fixture에서 만듭니다.
	FCFVehicleAuthoringReadRequest MakeReadRequest(const FAuthoringFixture& Fixture)
	{
		// Fixture Recipe/Target을 exact identity로 지정한 R0 request입니다.
		FCFVehicleAuthoringReadRequest Request;
		Request.Recipe = Fixture.Recipe;
		Request.TargetVehicleData = Fixture.TargetVehicleData;
		Request.CallerKind = ECFAuthoringCallerKind::Automation;
		return Request;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringParityTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Authoring.FacadeCoreParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringPreviewTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Authoring.PreviewMutationZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringWriteTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Authoring.TypedWriteGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAuthoringApplyTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Authoring.ApplySharedLane",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Common facade가 Snapshot/Resolver Core를 복제하지 않고 동일 result를 반환하며 R0 mutation이 0인지 검증합니다.
bool FCFVehicleAuthoringParityTest::RunTest(const FString& Parameters)
{
	// Facade/Core parity fixture입니다.
	CFVehicleAuthoringTestsPrivate::FAuthoringFixture Fixture;
	// Fixture/direct resolve failure diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Authoring parity fixture builds"), CFVehicleAuthoringTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Direct Snapshot/Resolver Core request입니다.
	FCFVehicleResolveRequest DirectRequest;
	// Direct Pure Resolver result입니다.
	FCFVehicleResolveResult DirectResult;
	if (!TestTrue(TEXT("Direct shared Core Resolve builds"), CFVehicleAuthoringTestsPrivate::BuildDirectResolve(Fixture, DirectRequest, DirectResult, Error)))
	{
		AddError(Error);
		return false;
	}

	// Same persistent truth를 facade로 읽는 request입니다.
	const FCFVehicleAuthoringReadRequest ReadRequest = CFVehicleAuthoringTestsPrivate::MakeReadRequest(Fixture);
	// Facade shared Resolver result입니다.
	FCFVehicleResolveReadResult FacadeResolve;
	if (!TestTrue(TEXT("Facade ResolveVehiclePreview succeeds"), FCFVehicleAuthoringService::ResolveVehiclePreview(ReadRequest, FacadeResolve)))
	{
		AddError(FacadeResolve.Operation.Message);
		return false;
	}

	TestEqual(TEXT("Facade Recipe fingerprint equals direct Core"), FacadeResolve.ResolveRequest.Recipe.RecipeFingerprint, DirectRequest.Recipe.RecipeFingerprint);
	TestEqual(TEXT("Facade Target hash equals direct Core"), FacadeResolve.ResolveRequest.CurrentDefinition.DefinitionHash, DirectRequest.CurrentDefinition.DefinitionHash);
	TestEqual(TEXT("Facade SourceSignature equals direct Core"), FacadeResolve.ResolveResult.SourceSignature, DirectResult.SourceSignature);
	TestEqual(TEXT("Facade ResolvedDefinitionHash equals direct Core"), FacadeResolve.ResolveResult.ResolvedDefinitionHash, DirectResult.ResolvedDefinitionHash);
	TestEqual(TEXT("Facade Resolver status equals direct Core"), FacadeResolve.ResolveResult.ResolveStatus, DirectResult.ResolveStatus);
	TestEqual(TEXT("Facade Diff row count equals direct Core"), FacadeResolve.ResolveResult.FieldDiff.Num(), DirectResult.FieldDiff.Num());
	TestEqual(TEXT("Facade DiffHash equals direct Core rows"), FacadeResolve.Operation.CurrentDiffHash, FCFVehicleAuthoringService::BuildDiffHash(DirectResult.FieldDiff));
	TestEqual(TEXT("Facade SourceTrace count equals direct Core"), FacadeResolve.ResolveResult.PreviewSourceTrace.Num(), DirectResult.PreviewSourceTrace.Num());

	// Dedicated Diff read facade result입니다.
	FCFVehicleDiffReadResult DiffRead;
	TestTrue(TEXT("ReadPendingDiff succeeds"), FCFVehicleAuthoringService::ReadPendingDiff(ReadRequest, DiffRead));
	TestEqual(TEXT("ReadPendingDiff hash equals Resolve preview"), DiffRead.DiffHash, FacadeResolve.Operation.CurrentDiffHash);
	TestEqual(TEXT("ReadPendingDiff row count equals Resolver"), DiffRead.FieldDiff.Num(), FacadeResolve.ResolveResult.FieldDiff.Num());

	// Dedicated full SourceTrace read facade result입니다.
	FCFVehicleTraceReadResult TraceRead;
	TestTrue(TEXT("ReadSourceTrace succeeds"), FCFVehicleAuthoringService::ReadSourceTrace(ReadRequest, TArray<FCFVehicleFieldPath>(), TraceRead));
	TestEqual(TEXT("ReadSourceTrace count equals Resolver"), TraceRead.SourceTrace.Num(), FacadeResolve.ResolveResult.PreviewSourceTrace.Num());

	// Dedicated Validation read facade result입니다.
	FCFVehicleValidationReadResult ValidationRead;
	TestTrue(TEXT("ReadValidation succeeds"), FCFVehicleAuthoringService::ReadValidation(ReadRequest, ValidationRead));
	TestEqual(TEXT("Recipe validation count parity"), ValidationRead.RecipeValidation.Num(), FacadeResolve.ResolveResult.RecipeValidation.Num());
	TestEqual(TEXT("Resolver validation count parity"), ValidationRead.ResolverValidation.Num(), FacadeResolve.ResolveResult.ResolverValidation.Num());
	TestEqual(TEXT("Definition validation count parity"), ValidationRead.DefinitionValidation.Num(), FacadeResolve.ResolveResult.DefinitionValidation.Num());

	// Dedicated External Drift read facade result입니다.
	FCFVehicleDriftReadResult DriftRead;
	TestTrue(TEXT("ReviewExternalDrift succeeds"), FCFVehicleAuthoringService::ReviewExternalDrift(ReadRequest, DriftRead));
	TestEqual(TEXT("External drift flag parity"), DriftRead.StaleReport.bHasExternalDrift, FacadeResolve.ResolveResult.StaleReport.bHasExternalDrift);
	TestEqual(TEXT("Effective stale flag parity"), DriftRead.StaleReport.bHasEffectiveStale, FacadeResolve.ResolveResult.StaleReport.bHasEffectiveStale);

	TestFalse(TEXT("R0 facade never dirties Target package"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("R0 facade never dirties Recipe package"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("R0 mutation footprint target false"), FacadeResolve.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("R0 mutation footprint recipe false"), FacadeResolve.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("R0 save false"), FacadeResolve.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("R0 automatic retry false"), FacadeResolve.Operation.Mutation.bAutomaticRetryPerformed);
	return true;
}

// PreviewRecipeChange가 typed prospective state를 계산하되 persistent Recipe/Target/package를 수정하지 않는지 검증합니다.
bool FCFVehicleAuthoringPreviewTest::RunTest(const FString& Parameters)
{
	// Prospective preview fixture입니다.
	CFVehicleAuthoringTestsPrivate::FAuthoringFixture Fixture;
	// Fixture/hash failure diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Authoring preview fixture builds"), CFVehicleAuthoringTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Preview 전 persistent Recipe fingerprint입니다.
	const FString RecipeFingerprintBefore = CFVehicleAuthoringTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Preview 전 persistent Target full Definition hash입니다.
	const FString TargetHashBefore = CFVehicleAuthoringTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Preview 전 Recipe diagnostic revision입니다.
	const int32 RevisionBefore = Fixture.Recipe->AuthoringRevision;
	// Preview 전 omitted axes가 보존되어야 할 value입니다.
	const float SteeringBefore = Fixture.Recipe->DrivingFeelIntent.SteeringAgility;
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// AccelerationFeel 한 축만 바꾸는 typed semantic prospective request입니다.
	FCFVehicleRecipeChangeRequest PreviewRequest;
	PreviewRequest.Recipe = Fixture.Recipe;
	PreviewRequest.TargetVehicleData = Fixture.TargetVehicleData;
	PreviewRequest.CallContext.CallerKind = ECFAuthoringCallerKind::AI;
	PreviewRequest.Change.Operation = ECFVehicleSemanticOp::SetDrivingFeel;
	PreviewRequest.Change.DrivingFeelPatch.bSetAccelerationFeel = true;
	PreviewRequest.Change.DrivingFeelPatch.AccelerationFeel = 0.73f;

	// Prospective semantic preview result입니다.
	FCFVehicleRecipePreviewResult PreviewResult;
	if (!TestTrue(TEXT("PreviewRecipeChange succeeds"), FCFVehicleAuthoringService::PreviewRecipeChange(PreviewRequest, PreviewResult)))
	{
		AddError(PreviewResult.Operation.Message);
		return false;
	}

	TestEqual(TEXT("Preview operation itself is R0"), PreviewResult.Operation.RiskClass, ECFAuthoringRiskClass::R0_ReadOnly);
	TestEqual(TEXT("Proposal is R1 AuthoringRecordWrite"), PreviewResult.Proposal.RiskClass, ECFAuthoringRiskClass::R1_AuthoringRecordWrite);
	TestEqual(TEXT("Proposal requires exact AuthoringWrite"), PreviewResult.Proposal.RequiredApprovalClass, ECFAuthoringApprovalClass::AuthoringWrite);
	TestFalse(TEXT("Recipe semantic proposal never mutates Target"), PreviewResult.Proposal.bTargetMutation);
	TestFalse(TEXT("Proposal never saves"), PreviewResult.Proposal.bSavePerformed);
	TestFalse(TEXT("Proposal hash is non-empty"), PreviewResult.Proposal.ProposalHash.IsEmpty());
	TestTrue(TEXT("Prospective Recipe fingerprint changes"), PreviewResult.Proposal.ProspectiveRecipeFingerprint != RecipeFingerprintBefore);

	TestEqual(TEXT("Persistent AccelerationFeel remains unchanged"), Fixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.5f);
	TestEqual(TEXT("Omitted Steering axis remains unchanged"), Fixture.Recipe->DrivingFeelIntent.SteeringAgility, SteeringBefore);
	TestEqual(TEXT("Persistent AuthoringRevision remains unchanged"), Fixture.Recipe->AuthoringRevision, RevisionBefore);
	TestEqual(TEXT("Persistent Recipe fingerprint remains unchanged"), CFVehicleAuthoringTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Persistent Target hash remains unchanged"), CFVehicleAuthoringTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestFalse(TEXT("Preview leaves Recipe package clean"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Preview leaves Target package clean"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Preview result mutation recipe false"), PreviewResult.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Preview result mutation target false"), PreviewResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Preview result automatic retry false"), PreviewResult.Operation.Mutation.bAutomaticRetryPerformed);
	return true;
}

// R1 exact approval, fresh fingerprint, no higher approval reuse, stale block, dedupe와 Recipe-only/no-save commit을 검증합니다.
bool FCFVehicleAuthoringWriteTest::RunTest(const FString& Parameters)
{
	// Fresh R1 write fixture입니다.
	CFVehicleAuthoringTestsPrivate::FAuthoringFixture Fixture;
	// Fixture/hash diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Authoring write fixture builds"), CFVehicleAuthoringTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// R1 prospective request입니다.
	FCFVehicleRecipeChangeRequest PreviewRequest;
	PreviewRequest.Recipe = Fixture.Recipe;
	PreviewRequest.TargetVehicleData = Fixture.TargetVehicleData;
	PreviewRequest.CallContext.CallerKind = ECFAuthoringCallerKind::AI;
	PreviewRequest.Change.Operation = ECFVehicleSemanticOp::SetDrivingFeel;
	PreviewRequest.Change.DrivingFeelPatch.bSetAccelerationFeel = true;
	PreviewRequest.Change.DrivingFeelPatch.AccelerationFeel = 0.67f;

	// R1 exact prospective approval source입니다.
	FCFVehicleRecipePreviewResult PreviewResult;
	if (!TestTrue(TEXT("R1 preview succeeds"), FCFVehicleAuthoringService::PreviewRecipeChange(PreviewRequest, PreviewResult)))
	{
		AddError(PreviewResult.Operation.Message);
		return false;
	}

	// DefinitionApply를 R1에 재사용하려는 invalid higher-approval request입니다.
	FCFVehicleRecipeChangeRequest WrongApprovalRequest = PreviewRequest;
	WrongApprovalRequest.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	WrongApprovalRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	WrongApprovalRequest.CallContext.ApprovalScopeHash = PreviewResult.Proposal.ProposalHash;
	WrongApprovalRequest.CallContext.ExpectedRecipeFingerprint = PreviewResult.Proposal.ExpectedRecipeFingerprint;
	WrongApprovalRequest.CallContext.ExpectedTargetDefinitionHash = PreviewResult.Proposal.ExpectedTargetDefinitionHash;
	WrongApprovalRequest.CallContext.ExpectedResolverContractRevision = PreviewResult.Proposal.ResolverContractRevision;
	// Higher approval reuse rejection result입니다.
	FCFAuthoringOpResult WrongApprovalResult;
	TestFalse(TEXT("R1 rejects higher DefinitionApply approval reuse"), FCFVehicleAuthoringService::CommitRecipeChange(WrongApprovalRequest, WrongApprovalResult));
	TestEqual(TEXT("Wrong approval reports ApprovalRequired"), WrongApprovalResult.ErrorCode, ECFAuthoringErrorCode::ApprovalRequired);

	// Approved R1 commit 전 Target baseline hash입니다.
	const FString TargetHashBeforeCommit = CFVehicleAuthoringTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Approved R1 commit 전 diagnostic revision입니다.
	const int32 RevisionBeforeCommit = Fixture.Recipe->AuthoringRevision;
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Exact AuthoringWrite approval + Preview state를 가진 commit request입니다.
	FCFVehicleRecipeChangeRequest CommitRequest = PreviewRequest;
	CommitRequest.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	CommitRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	CommitRequest.CallContext.ApprovalScopeHash = PreviewResult.Proposal.ProposalHash;
	CommitRequest.CallContext.ExpectedRecipeFingerprint = PreviewResult.Proposal.ExpectedRecipeFingerprint;
	CommitRequest.CallContext.ExpectedTargetDefinitionHash = PreviewResult.Proposal.ExpectedTargetDefinitionHash;
	CommitRequest.CallContext.ExpectedResolverContractRevision = PreviewResult.Proposal.ResolverContractRevision;
	// Successful R1 commit result입니다.
	FCFAuthoringOpResult CommitResult;
	if (!TestTrue(TEXT("Fresh approved R1 commit succeeds"), FCFVehicleAuthoringService::CommitRecipeChange(CommitRequest, CommitResult)))
	{
		AddError(CommitResult.Message);
		return false;
	}

	TestEqual(TEXT("R1 commit status Succeeded"), CommitResult.Status, ECFAuthoringOpStatus::Succeeded);
	TestTrue(TEXT("R1 commit mutates Recipe"), CommitResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("R1 commit never mutates Target"), CommitResult.Mutation.bTargetChanged);
	TestFalse(TEXT("R1 commit never mutates Profile"), CommitResult.Mutation.bProfileChanged);
	TestFalse(TEXT("R1 commit never saves"), CommitResult.Mutation.bSavePerformed);
	TestFalse(TEXT("R1 commit never auto-retries"), CommitResult.Mutation.bAutomaticRetryPerformed);
	TestEqual(TEXT("R1 desired feel is committed"), Fixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.67f);
	TestEqual(TEXT("R1 increments AuthoringRevision once"), Fixture.Recipe->AuthoringRevision, RevisionBeforeCommit + 1);
	TestEqual(TEXT("R1 committed fingerprint equals proposal"), CommitResult.CurrentRecipeFingerprint, PreviewResult.Proposal.ProspectiveRecipeFingerprint);
	TestEqual(TEXT("R1 leaves Target hash unchanged"), CFVehicleAuthoringTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBeforeCommit);
	TestTrue(TEXT("R1 marks Recipe package dirty"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("R1 does not dirty Target package"), Fixture.TargetPackage->IsDirty());

	// Same ClientOperationId + same exact request의 dedupe replay 결과입니다.
	FCFAuthoringOpResult DedupeResult;
	TestTrue(TEXT("Same R1 operation id replays terminal result"), FCFVehicleAuthoringService::CommitRecipeChange(CommitRequest, DedupeResult));
	TestTrue(TEXT("R1 duplicate reports dedupe replay"), DedupeResult.bResultReplayedFromDedupe);
	TestFalse(TEXT("R1 duplicate performs no second Recipe mutation"), DedupeResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("R1 duplicate performs no Target mutation"), DedupeResult.Mutation.bTargetChanged);
	TestFalse(TEXT("R1 duplicate is not automatic retry"), DedupeResult.Mutation.bAutomaticRetryPerformed);
		TestEqual(TEXT("R1 duplicate does not increment revision twice"), Fixture.Recipe->AuthoringRevision, RevisionBeforeCommit + 1);

	// Frozen RecipeFingerprint에는 포함되지 않지만 Section 25 R1 catalog에는 포함되는 Archetype semantic operation preview입니다.
	FCFVehicleRecipeChangeRequest ArchetypePreviewRequest;
	ArchetypePreviewRequest.Recipe = Fixture.Recipe;
	ArchetypePreviewRequest.TargetVehicleData = Fixture.TargetVehicleData;
	ArchetypePreviewRequest.CallContext.CallerKind = ECFAuthoringCallerKind::AI;
	ArchetypePreviewRequest.Change.Operation = ECFVehicleSemanticOp::SetVehicleArchetype;
	ArchetypePreviewRequest.Change.VehicleArchetypeId = TEXT("Archetype_Typed_Write");
	// Archetype exact proposal입니다.
	FCFVehicleRecipePreviewResult ArchetypePreview;
	if (!TestTrue(TEXT("Archetype typed preview succeeds"), FCFVehicleAuthoringService::PreviewRecipeChange(ArchetypePreviewRequest, ArchetypePreview)))
	{
		AddError(ArchetypePreview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("Archetype is outside Frozen resolver RecipeFingerprint set"), ArchetypePreview.Proposal.ProspectiveRecipeFingerprint, ArchetypePreview.Proposal.ExpectedRecipeFingerprint);

	// Exact AuthoringWrite approval을 가진 Archetype commit request입니다.
	FCFVehicleRecipeChangeRequest ArchetypeCommitRequest = ArchetypePreviewRequest;
	ArchetypeCommitRequest.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	ArchetypeCommitRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	ArchetypeCommitRequest.CallContext.ApprovalScopeHash = ArchetypePreview.Proposal.ProposalHash;
	ArchetypeCommitRequest.CallContext.ExpectedRecipeFingerprint = ArchetypePreview.Proposal.ExpectedRecipeFingerprint;
	ArchetypeCommitRequest.CallContext.ExpectedTargetDefinitionHash = ArchetypePreview.Proposal.ExpectedTargetDefinitionHash;
	ArchetypeCommitRequest.CallContext.ExpectedResolverContractRevision = ArchetypePreview.Proposal.ResolverContractRevision;
	// Archetype typed write result입니다.
	FCFAuthoringOpResult ArchetypeCommitResult;
	if (!TestTrue(TEXT("Archetype typed desired state commits despite unchanged resolver fingerprint"), FCFVehicleAuthoringService::CommitRecipeChange(ArchetypeCommitRequest, ArchetypeCommitResult)))
	{
		AddError(ArchetypeCommitResult.Message);
		return false;
	}
	TestEqual(TEXT("Archetype write status Succeeded"), ArchetypeCommitResult.Status, ECFAuthoringOpStatus::Succeeded);
	TestEqual(TEXT("Archetype desired value committed"), Fixture.Recipe->VehicleArchetypeId, FName(TEXT("Archetype_Typed_Write")));
	TestTrue(TEXT("Archetype write mutates Recipe"), ArchetypeCommitResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Archetype write never mutates Target"), ArchetypeCommitResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Archetype write never saves"), ArchetypeCommitResult.Mutation.bSavePerformed);

	// 별도 stale-write fixture입니다.
	CFVehicleAuthoringTestsPrivate::FAuthoringFixture StaleFixture;
	if (!TestTrue(TEXT("Stale R1 fixture builds"), CFVehicleAuthoringTestsPrivate::BuildImportedFixture(StaleFixture, Error)))
	{
		AddError(Error);
		return false;
	}
		// Stale 검증용 fingerprint-covered prospective request입니다.
	FCFVehicleRecipeChangeRequest StalePreviewRequest;
	StalePreviewRequest.Recipe = StaleFixture.Recipe;
	StalePreviewRequest.TargetVehicleData = StaleFixture.TargetVehicleData;
	StalePreviewRequest.CallContext.CallerKind = ECFAuthoringCallerKind::AI;
	StalePreviewRequest.Change.Operation = ECFVehicleSemanticOp::SetDrivingFeel;
	StalePreviewRequest.Change.DrivingFeelPatch.bSetAccelerationFeel = true;
	StalePreviewRequest.Change.DrivingFeelPatch.AccelerationFeel = 0.81f;
	// Stale approval source입니다.
	FCFVehicleRecipePreviewResult StalePreview;
	if (!TestTrue(TEXT("Stale source preview succeeds"), FCFVehicleAuthoringService::PreviewRecipeChange(StalePreviewRequest, StalePreview)))
	{
		AddError(StalePreview.Operation.Message);
		return false;
	}

		// Preview와 commit 사이에 다른 fingerprint-covered Feel axis가 외부에서 바뀐 concurrent semantic mutation을 시뮬레이션합니다.
	StaleFixture.Recipe->DrivingFeelIntent.GripFeel = 0.22f;
	++StaleFixture.Recipe->AuthoringRevision;
	// Commit 직전 external state fingerprint입니다.
	const FString ExternalFingerprint = CFVehicleAuthoringTestsPrivate::BuildRecipeFingerprint(*StaleFixture.Recipe, Error);
	// Commit 직전 diagnostic revision입니다.
	const int32 ExternalRevision = StaleFixture.Recipe->AuthoringRevision;
	// Commit 직전 Target hash입니다.
	const FString StaleTargetHash = CFVehicleAuthoringTestsPrivate::BuildTargetHash(*StaleFixture.TargetVehicleData, Error);

	// Old approval/precondition을 그대로 가진 stale commit request입니다.
	FCFVehicleRecipeChangeRequest StaleCommitRequest = StalePreviewRequest;
	StaleCommitRequest.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	StaleCommitRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	StaleCommitRequest.CallContext.ApprovalScopeHash = StalePreview.Proposal.ProposalHash;
	StaleCommitRequest.CallContext.ExpectedRecipeFingerprint = StalePreview.Proposal.ExpectedRecipeFingerprint;
	StaleCommitRequest.CallContext.ExpectedTargetDefinitionHash = StalePreview.Proposal.ExpectedTargetDefinitionHash;
	StaleCommitRequest.CallContext.ExpectedResolverContractRevision = StalePreview.Proposal.ResolverContractRevision;
	// Stale block typed result입니다.
	FCFAuthoringOpResult StaleResult;
	TestFalse(TEXT("Stale R1 commit is blocked"), FCFVehicleAuthoringService::CommitRecipeChange(StaleCommitRequest, StaleResult));
	TestEqual(TEXT("Stale R1 reports RecipeFingerprintMismatch"), StaleResult.ErrorCode, ECFAuthoringErrorCode::RecipeFingerprintMismatch);
	TestEqual(TEXT("Stale block preserves external Recipe fingerprint"), CFVehicleAuthoringTestsPrivate::BuildRecipeFingerprint(*StaleFixture.Recipe, Error), ExternalFingerprint);
	TestEqual(TEXT("Stale block preserves external revision"), StaleFixture.Recipe->AuthoringRevision, ExternalRevision);
	TestEqual(TEXT("Stale block preserves Target hash"), CFVehicleAuthoringTestsPrivate::BuildTargetHash(*StaleFixture.TargetVehicleData, Error), StaleTargetHash);
	TestFalse(TEXT("Stale block never auto-retries"), StaleResult.Mutation.bAutomaticRetryPerformed);
	TestFalse(TEXT("Stale block never saves"), StaleResult.Mutation.bSavePerformed);
	return true;
}

// R3 facade가 FCFVehicleApplyService shared lane으로 actual Target을 적용하고 duplicate request를 재실행하지 않는지 검증합니다.
bool FCFVehicleAuthoringApplyTest::RunTest(const FString& Parameters)
{
	// Actual R3 shared Apply fixture입니다.
	CFVehicleAuthoringTestsPrivate::FAuthoringFixture Fixture;
	// Fixture/preview/hash diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Authoring R3 fixture builds"), CFVehicleAuthoringTestsPrivate::BuildImportedFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	CFVehicleAuthoringTestsPrivate::ConfigureApplyDifference(*Fixture.Recipe);

	// Shared facade가 만든 current approved resolver request/result입니다.
	const FCFVehicleAuthoringReadRequest ReadRequest = CFVehicleAuthoringTestsPrivate::MakeReadRequest(Fixture);
	// R3 reviewed resolve source입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!TestTrue(TEXT("R3 facade preview succeeds"), FCFVehicleAuthoringService::ResolveVehiclePreview(ReadRequest, ResolveRead)))
	{
		AddError(ResolveRead.Operation.Message);
		return false;
	}
	if (!TestEqual(TEXT("R3 reviewed resolve is Success"), ResolveRead.ResolveResult.ResolveStatus, ECFVehicleResolveStatus::Success))
	{
		return false;
	}
	TestTrue(TEXT("R3 reviewed diff is non-empty"), ResolveRead.ResolveResult.FieldDiff.Num() > 0);

	// P0-08H shared ApplyRequest를 facade R3에 그대로 전달할 reviewed payload입니다.
	FCFVehicleApplyRequest ApplyRequest;
	ApplyRequest.Recipe = Fixture.Recipe;
	ApplyRequest.TargetVehicleData = Fixture.TargetVehicleData;
	ApplyRequest.ResolveRequest = ResolveRead.ResolveRequest;
	ApplyRequest.ApprovedResolveResult = ResolveRead.ResolveResult;
	ApplyRequest.ExpectedRecipeFingerprint = ResolveRead.ResolveRequest.Recipe.RecipeFingerprint;
	ApplyRequest.ExpectedSourceSignature = ResolveRead.ResolveResult.SourceSignature;
	ApplyRequest.ExpectedTargetDefinitionHash = ResolveRead.ResolveRequest.CurrentDefinition.DefinitionHash;
	ApplyRequest.ExpectedResolvedDefinitionHash = ResolveRead.ResolveResult.ResolvedDefinitionHash;
	ApplyRequest.ExpectedResolverContractRevision = ResolveRead.ResolveResult.ResolverContractRevision;

	// Exact R3 approval proposal입니다.
	FCFAuthoringProposal ApplyProposal;
	// R3 approval proposal helper result입니다.
	FCFAuthoringOpResult ProposalResult;
	if (!TestTrue(TEXT("BuildApplyApprovalProposal succeeds"), FCFVehicleAuthoringService::BuildApplyApprovalProposal(ApplyRequest, ApplyProposal, ProposalResult)))
	{
		AddError(ProposalResult.Message);
		return false;
	}
	TestEqual(TEXT("Apply proposal Risk is R3"), ApplyProposal.RiskClass, ECFAuthoringRiskClass::R3_DefinitionApply);
	TestEqual(TEXT("Apply proposal requires DefinitionApply"), ApplyProposal.RequiredApprovalClass, ECFAuthoringApprovalClass::DefinitionApply);
	TestEqual(TEXT("Apply proposal DiffHash matches facade hash"), ApplyProposal.DiffHash, FCFVehicleAuthoringService::BuildDiffHash(ResolveRead.ResolveResult.FieldDiff));
	TestFalse(TEXT("Apply proposal never performs Save"), ApplyProposal.bSavePerformed);

	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);

	// Approved R3 facade request입니다.
	FCFVehicleApplyOpRequest ApplyOpRequest;
	ApplyOpRequest.ApplyRequest = ApplyRequest;
	ApplyOpRequest.ExpectedDiffHash = ApplyProposal.DiffHash;
	ApplyOpRequest.CallContext.ClientOperationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	ApplyOpRequest.CallContext.CallerKind = ECFAuthoringCallerKind::AI;
	ApplyOpRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	ApplyOpRequest.CallContext.ApprovalScopeHash = ApplyProposal.ProposalHash;
	ApplyOpRequest.CallContext.ExpectedRecipeFingerprint = ApplyProposal.ExpectedRecipeFingerprint;
	ApplyOpRequest.CallContext.ExpectedTargetDefinitionHash = ApplyProposal.ExpectedTargetDefinitionHash;
	ApplyOpRequest.CallContext.ExpectedResolverContractRevision = ApplyProposal.ResolverContractRevision;

	// Actual shared Apply lane typed result입니다.
	FCFAuthoringOpResult ApplyResult;
	if (!TestTrue(TEXT("ApplyResolvedVehicle succeeds through shared lane"), FCFVehicleAuthoringService::ApplyResolvedVehicle(ApplyOpRequest, ApplyResult)))
	{
		AddError(ApplyResult.Message);
		return false;
	}
	TestEqual(TEXT("R3 facade status Succeeded"), ApplyResult.Status, ECFAuthoringOpStatus::Succeeded);
	TestTrue(TEXT("R3 reports Target mutation"), ApplyResult.Mutation.bTargetChanged);
	TestTrue(TEXT("R3 reports AppliedState Recipe mutation"), ApplyResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("R3 never mutates Profile"), ApplyResult.Mutation.bProfileChanged);
	TestFalse(TEXT("R3 never saves"), ApplyResult.Mutation.bSavePerformed);
	TestFalse(TEXT("R3 never automatic retries"), ApplyResult.Mutation.bAutomaticRetryPerformed);
	TestTrue(TEXT("R3 applied operation count is nonzero"), ApplyResult.AppliedFieldCount > 0);
	TestNull(TEXT("Shared Apply removes old Hardpoint"), CFVehicleAuthoringTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Top_Old")));
	TestNotNull(TEXT("Shared Apply adds new Hardpoint"), CFVehicleAuthoringTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Front_New")));
	TestNotNull(TEXT("Shared Apply adds dependent Mount"), CFVehicleAuthoringTestsPrivate::FindMount(*Fixture.TargetVehicleData, TEXT("M_Front_New")));
	TestEqual(TEXT("AppliedState definition hash equals approved resolved hash"), Fixture.Recipe->AppliedState.AppliedDefinitionHash, ApplyRequest.ExpectedResolvedDefinitionHash);
	TestTrue(TEXT("R3 marks Target package dirty"), Fixture.TargetPackage->IsDirty());
	TestTrue(TEXT("R3 marks Recipe package dirty"), Fixture.RecipePackage->IsDirty());

	// Same exact ClientOperationId의 duplicate R3 request는 underlying ApplyService를 두 번째 호출하지 않아야 합니다.
	FCFAuthoringOpResult DedupeResult;
	TestTrue(TEXT("Duplicate R3 request replays terminal result"), FCFVehicleAuthoringService::ApplyResolvedVehicle(ApplyOpRequest, DedupeResult));
	TestTrue(TEXT("Duplicate R3 result marks dedupe replay"), DedupeResult.bResultReplayedFromDedupe);
	TestFalse(TEXT("Duplicate R3 performs no second Target mutation"), DedupeResult.Mutation.bTargetChanged);
	TestFalse(TEXT("Duplicate R3 performs no second Recipe mutation"), DedupeResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("Duplicate R3 is not automatic retry"), DedupeResult.Mutation.bAutomaticRetryPerformed);
	TestFalse(TEXT("Duplicate R3 performs no Save"), DedupeResult.Mutation.bSavePerformed);
	TestNull(TEXT("Duplicate R3 keeps old Hardpoint absent"), CFVehicleAuthoringTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Top_Old")));
	TestNotNull(TEXT("Duplicate R3 keeps new Hardpoint"), CFVehicleAuthoringTestsPrivate::FindHardpoint(*Fixture.TargetVehicleData, TEXT("Front_New")));
	return true;
}

#endif
