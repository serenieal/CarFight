// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBuilderTests.cpp
// Version: v1.0.0
// Date: 2026-08-27
// Description: CF-FQ-040 VB-P0-05 Builder companion / private 4-Profile write lane direct focused Automation입니다.
// Scope: R2 companion creation, baseline preservation, stale/owner/path guards, R1 Profile commit, persistent receipt, stale Evidence/Profile guards, Save0를 직접 검증합니다.
// Changelog:
// - v1.0.0: VB-P0-05 핵심 write facade를 실제 호출하는 direct Automation 2건을 추가.
// Migration:
// - 모든 fixture는 in-memory /Temp package만 사용하며 SavePackage를 호출하지 않습니다.
// - 테스트가 만든 package/object는 Automation process lifetime에만 존재합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleBuilderTestsPrivate
{
	/** VB-P0-05 direct write tests가 공유하는 managed Target/Recipe/private Profile fixture입니다. */
	struct FBuilderFixture
	{
		// Target VehicleData를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe/Profile/Evidence fixture를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* RecipePackage = nullptr;

		// Resolver/Validator가 읽을 managed Target VehicleData입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// Builder companion/profile owner가 될 managed Recipe입니다.
		UCFVehicleRecipeData* Recipe = nullptr;

		// USER-authored wheel socket facts를 제공할 transient Chassis입니다.
		UStaticMesh* ChassisMesh = nullptr;

		// Builder-private VehicleBase Profile fixture입니다.
		UCFVehicleBaseProfile* VehicleBase = nullptr;

		// Builder-private Drivetrain Profile fixture입니다.
		UCFDrivetrainProfile* Drivetrain = nullptr;

		// Builder-private Handling Profile fixture입니다.
		UCFHandlingProfile* Handling = nullptr;

		// Builder-private Performance Profile fixture입니다.
		UCFPerformanceProfile* Performance = nullptr;

		// Profile proposal provenance에 사용할 Reference Evidence fixture입니다.
		UCFVehicleRefEvidence* Evidence = nullptr;
	};

	// Automation object를 격리할 unique /Temp package를 만듭니다.
	UPackage* CreateTestPackage(const TCHAR* Prefix)
	{
		// 다른 Automation run과 충돌하지 않는 unique long package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// Builder companion destination으로 사용할 아직 존재하지 않는 unique /Temp asset identity를 만듭니다.
	FCFBuilderAssetIdentity MakeAssetIdentity(const TCHAR* Prefix, const TCHAR* AssetName)
	{
		// Production contract가 요구하는 /Game long package identity를 사용하되 Automation process에서 절대 Save하지 않는 unique package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Game/Test/CarFightDataAuthoring/Automation/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		// Request에 반환할 exact package/object identity입니다.
		FCFBuilderAssetIdentity Identity;
		Identity.PackageName = PackageName;
		Identity.AssetName = FName(AssetName);
		return Identity;
	}

	// Transient Chassis에 USER-authored wheel socket을 추가합니다.
	void AddSocket(UStaticMesh& ChassisMesh, const FName SocketName, const FVector& RelativeLocation)
	{
		// Chassis가 소유하는 transient socket입니다.
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(&ChassisMesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = RelativeLocation;
		Socket->RelativeRotation = FRotator::ZeroRotator;
		Socket->RelativeScale = FVector::OneVector;
		ChassisMesh.AddSocket(Socket);
	}

	// Validator/Resolver를 통과하는 managed Existing Definition + Recipe baseline을 구성합니다.
	bool BuildManagedFixture(FBuilderFixture& OutFixture, FString& OutError)
	{
		OutFixture = FBuilderFixture();
		OutFixture.TargetPackage = CreateTestPackage(TEXT("CFVBTarget"));
		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFVBRecipe"));
		if (!OutFixture.TargetPackage || !OutFixture.RecipePackage)
		{
			OutError = TEXT("VB-P0-05 /Temp fixture package를 만들 수 없습니다.");
			return false;
		}

		// Managed import source가 될 target VehicleData입니다.
		OutFixture.TargetVehicleData = NewObject<UCFVehicleData>(OutFixture.TargetPackage, TEXT("DA_VB_Target"), RF_Transactional);
		// Managed authoring owner가 될 Recipe입니다.
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_VB_Recipe"), RF_Transactional);
		// USER wheel socket authority를 흉내 낼 Chassis입니다.
		OutFixture.ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		if (!OutFixture.TargetVehicleData || !OutFixture.Recipe || !OutFixture.ChassisMesh)
		{
			OutError = TEXT("VB-P0-05 Target/Recipe/Chassis fixture를 만들 수 없습니다.");
			return false;
		}

		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_FL"), FVector(110.0, -62.0, 28.0));
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_FR"), FVector(110.0, 62.0, 28.0));
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_RL"), FVector(-108.0, -62.0, 28.0));
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_RR"), FVector(-108.0, 62.0, 28.0));

		// Non-zero bounds와 stable Engine path를 제공하는 wheel mesh입니다.
		UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		// Required wheel class reference에 사용할 ChaosVehicleWheel class입니다.
		UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
		if (!WheelMesh || !WheelClass)
		{
			OutError = TEXT("VB-P0-05 Engine Cube/ChaosVehicleWheel fixture를 load할 수 없습니다.");
			return false;
		}

		OutFixture.TargetVehicleData->VehicleVisualConfig.ChassisMesh = OutFixture.ChassisMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFL = WheelMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFR = WheelMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRL = WheelMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRR = WheelMesh;
		OutFixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass = WheelClass;
		OutFixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass = WheelClass;
		OutFixture.TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides = true;
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation = FVector(110.0, -62.0, 28.0);
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeLocation = FVector(110.0, 62.0, 28.0);
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeLocation = FVector(-108.0, -62.0, 28.0);
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeLocation = FVector(-108.0, 62.0, 28.0);
		OutFixture.TargetVehicleData->BaseVehicleMassKg = 1540.0f;
		OutFixture.TargetVehicleData->MaximumGrossMassKg = 2280.0f;

		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;
		// Lossless Existing Definition import source snapshot입니다.
		FCFVehicleDefinitionSnapshot DefinitionSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*OutFixture.TargetVehicleData, DefinitionSnapshot, OutError))
		{
			return false;
		}
		// Managed ownership baseline을 만드는 import summary입니다.
		FCFVehicleImportResult ImportResult;
		if (!FCFVehicleImportService::ImportDefinitionSnapshot(DefinitionSnapshot, *OutFixture.Recipe, ImportResult, OutError))
		{
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Recipe가 소유하는 Builder-private 4 Profile 중 요청한 domain을 /Temp fixture로 binding합니다.
	bool AttachPrivateProfiles(FBuilderFixture& InOutFixture, const bool bIncludePerformance, FString& OutError)
	{
		// VehicleBase private Profile입니다.
		InOutFixture.VehicleBase = NewObject<UCFVehicleBaseProfile>(InOutFixture.RecipePackage, TEXT("DA_VB_Base"), RF_Transactional);
		// Drivetrain private Profile입니다.
		InOutFixture.Drivetrain = NewObject<UCFDrivetrainProfile>(InOutFixture.RecipePackage, TEXT("DA_VB_Drive"), RF_Transactional);
		// Handling private Profile입니다.
		InOutFixture.Handling = NewObject<UCFHandlingProfile>(InOutFixture.RecipePackage, TEXT("DA_VB_Handle"), RF_Transactional);
		// Optional Performance private Profile입니다.
		InOutFixture.Performance = bIncludePerformance
			? NewObject<UCFPerformanceProfile>(InOutFixture.RecipePackage, TEXT("DA_VB_Perf"), RF_Transactional)
			: nullptr;
		if (!InOutFixture.VehicleBase || !InOutFixture.Drivetrain || !InOutFixture.Handling || (bIncludePerformance && !InOutFixture.Performance))
		{
			OutError = TEXT("VB-P0-05 private Profile fixture를 만들 수 없습니다.");
			return false;
		}

		InOutFixture.VehicleBase->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		InOutFixture.Drivetrain->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		InOutFixture.Handling->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		if (InOutFixture.Performance)
		{
			InOutFixture.Performance->Meta.OwnerRecipeId = InOutFixture.Recipe->RecipeId;
		}

		InOutFixture.VehicleBase->Data.BaseVehicleMassKg = InOutFixture.TargetVehicleData->BaseVehicleMassKg;
		InOutFixture.VehicleBase->Data.MaximumGrossMassKg = InOutFixture.TargetVehicleData->MaximumGrossMassKg;
		InOutFixture.VehicleBase->Data.MaxHealth = InOutFixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth;
		InOutFixture.VehicleBase->Data.ChassisWidth = InOutFixture.TargetVehicleData->VehicleMovementConfig.ChassisWidth;
		InOutFixture.VehicleBase->Data.ChassisHeight = InOutFixture.TargetVehicleData->VehicleMovementConfig.ChassisHeight;
		InOutFixture.VehicleBase->Data.ExpectedWheelCount = 4;
		InOutFixture.VehicleBase->Data.FrontWheelCountForSteering = 2;
		InOutFixture.VehicleBase->Data.WheelMeshScaleClampMin = 0.25f;
		InOutFixture.VehicleBase->Data.WheelMeshScaleClampMax = 4.0f;
		InOutFixture.Drivetrain->Data.FrontWheelClass = InOutFixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass;
		InOutFixture.Drivetrain->Data.RearWheelClass = InOutFixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass;

		InOutFixture.Recipe->ProfileBindings.VehicleBaseProfile = InOutFixture.VehicleBase;
		InOutFixture.Recipe->ProfileBindings.DrivetrainProfile = InOutFixture.Drivetrain;
		InOutFixture.Recipe->ProfileBindings.HandlingProfile = InOutFixture.Handling;
		if (InOutFixture.Performance)
		{
			InOutFixture.Recipe->ProfileBindings.PerformanceProfile = InOutFixture.Performance;
		}
		OutError.Reset();
		return true;
	}

	// Profile commit provenance에 사용할 canonical FACT Evidence를 fixture Recipe/Target에 binding합니다.
	bool AttachEvidence(FBuilderFixture& InOutFixture, FString& OutError)
	{
		// Recipe package가 소유하는 test Evidence입니다.
		InOutFixture.Evidence = NewObject<UCFVehicleRefEvidence>(InOutFixture.RecipePackage, TEXT("DA_VB_Evidence"), RF_Transactional);
		if (!InOutFixture.Evidence)
		{
			OutError = TEXT("VB-P0-05 Evidence fixture를 만들 수 없습니다.");
			return false;
		}

		InOutFixture.Evidence->TargetRecipeId = InOutFixture.Recipe->RecipeId;
		InOutFixture.Evidence->TargetRecipePath = FSoftObjectPath(InOutFixture.Recipe);
		InOutFixture.Evidence->TargetDefinitionPath = FSoftObjectPath(InOutFixture.TargetVehicleData);
		// Profile commit이 실제 소비할 canonical FACT claim입니다.
		FCFRefClaim& FactClaim = InOutFixture.Evidence->Claims.AddDefaulted_GetRef();
		FactClaim.ClaimId = TEXT("Claim_VB_Mass");
		FactClaim.FactKey = TEXT("CurbMassKg");
		FactClaim.ValueKind = ECFRefValueKind::Number;
		FactClaim.NumberValue = 1540.0;
		FactClaim.UnitId = TEXT("kg");
		FactClaim.Provenance = ECFRefProvenance::FACT;
		FactClaim.ResolutionState = ECFRefClaimResolution::Canonical;
		return InOutFixture.Evidence->RefreshEvidenceFingerprint(OutError);
	}

	// Current fixture private Profiles/Evidence에서 direct R1 commit request를 만듭니다.
	FCFBuilderProfileCommitRequest BuildProfileCommitRequest(FBuilderFixture& Fixture)
	{
		// Direct Profile commit request입니다.
		FCFBuilderProfileCommitRequest Request;
		Request.Recipe = Fixture.Recipe;
		Request.ExpectedOwnerRecipeId = Fixture.Recipe->RecipeId;
		Request.Payload.VehicleBaseProfilePath = FSoftObjectPath(Fixture.VehicleBase);
		Request.Payload.DrivetrainProfilePath = FSoftObjectPath(Fixture.Drivetrain);
		Request.Payload.HandlingProfilePath = FSoftObjectPath(Fixture.Handling);
		Request.Payload.PerformanceProfilePath = FSoftObjectPath(Fixture.Performance);
		Request.Payload.VehicleBaseData = Fixture.VehicleBase->Data;
		Request.Payload.DrivetrainData = Fixture.Drivetrain->Data;
		Request.Payload.HandlingData = Fixture.Handling->Data;
		Request.Payload.PerformanceData = Fixture.Performance->Data;
		Request.EvidenceBinding.EvidencePath = FSoftObjectPath(Fixture.Evidence);
		Request.EvidenceBinding.ExpectedEvidenceId = Fixture.Evidence->EvidenceId;
		Request.EvidenceBinding.ExpectedEvidenceFingerprint = Fixture.Evidence->EvidenceFingerprint;
		Request.EvidenceBinding.ConsumedClaimIds = {TEXT("Claim_VB_Mass")};
		Request.UpstreamBuilderProposalHash = TEXT("VB-P0-05-Direct-Automation");
		Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
		return Request;
	}

	// Preview가 요구하는 exact approval/current fingerprint를 commit request에 채웁니다.
	void BindCommitApproval(
		FCFBuilderProfileCommitRequest& InOutRequest,
		const FCFBuilderProfileCommitPreview& Preview,
		const FString& ClientOperationId)
	{
		InOutRequest.ExpectedCurrentFingerprints = Preview.CurrentFingerprints;
		InOutRequest.CallContext.ClientOperationId = ClientOperationId;
		InOutRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
		InOutRequest.CallContext.ApprovalScopeHash = Preview.Proposal.ProposalHash;
		InOutRequest.CallContext.ExpectedRecipeFingerprint = Preview.Proposal.ExpectedRecipeFingerprint;
		InOutRequest.CallContext.ExpectedTargetDefinitionHash = Preview.Proposal.ExpectedTargetDefinitionHash;
		InOutRequest.CallContext.ExpectedResolverContractRevision = Preview.Proposal.ResolverContractRevision;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderCompanionFlowTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderCompanionFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderProfileCommitTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Missing companion subset creation, stale approval, path collision, foreign owner와 baseline preservation을 직접 검증합니다.
bool FCFVehicleBuilderCompanionFlowTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderTestsPrivate;

	// Direct companion test fixture입니다.
	FBuilderFixture Fixture;
	// Fixture build diagnostic입니다.
	FString Error;
	if (!BuildManagedFixture(Fixture, Error) || !AttachPrivateProfiles(Fixture, false, Error))
	{
		AddError(Error);
		return false;
	}

	// Companion 보완 전 Target full hash입니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, TargetBefore, Error))
	{
		AddError(Error);
		return false;
	}

	// Missing Performance seed와 new Evidence destination을 포함한 CompleteExisting request입니다.
	FCFBuilderCompanionRequest Request;
	Request.Recipe = Fixture.Recipe;
	Request.Mode = ECFBuilderCompanionMode::CompleteExisting;
	Request.bHasInitialProfilePayload = true;
	Request.InitialProfilePayload.VehicleBaseData = Fixture.VehicleBase->Data;
	Request.InitialProfilePayload.DrivetrainData = Fixture.Drivetrain->Data;
	Request.InitialProfilePayload.HandlingData = Fixture.Handling->Data;
	Request.InitialProfilePayload.PerformanceData = FCFPerformanceProfileData();
	Request.EvidenceAsset = MakeAssetIdentity(TEXT("CFVBEvidence"), TEXT("DA_VB_Evidence_New"));
	Request.PerformanceAsset = MakeAssetIdentity(TEXT("CFVBPerformance"), TEXT("DA_VB_Performance_New"));
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;

	// Existing package collision을 강제로 만드는 mutation0 request입니다.
	FCFBuilderCompanionRequest CollisionRequest = Request;
	CollisionRequest.EvidenceAsset.PackageName = Fixture.TargetPackage->GetName();
	CollisionRequest.EvidenceAsset.AssetName = TEXT("DA_Collision");
	// Collision preview result입니다.
	FCFBuilderCompanionPreview CollisionPreview;
	TestFalse(TEXT("VB-P0-05 companion destination collision is rejected"), FCFVehicleAuthoringService::PreviewBuilderCompanions(CollisionRequest, CollisionPreview));
	TestTrue(TEXT("VB-P0-05 collision preview performs no mutation"), !CollisionPreview.Operation.Mutation.bCreatedAssets && !CollisionPreview.Operation.Mutation.bRecipeChanged);

	// 정상 baseline-preserving companion preview입니다.
	FCFBuilderCompanionPreview Preview;
	if (!TestTrue(TEXT("VB-P0-05 companion preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderCompanions(Request, Preview)))
	{
		AddError(Preview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("VB-P0-05 CompleteExisting prospective hash preserves baseline"), Preview.ProspectiveResolvedDefinitionHash, Preview.CurrentResolvedDefinitionHash);
	TestFalse(TEXT("VB-P0-05 companion preview does not save"), Preview.Operation.Mutation.bSavePerformed);

	Request.CallContext.ClientOperationId = TEXT("VB-P0-05-Companion-Stale");
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	Request.CallContext.ApprovalScopeHash = Preview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = Preview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = Preview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = Preview.Proposal.ResolverContractRevision;

	// Preview 뒤 transaction 밖 Target drift를 만들어 stale approval을 검증합니다.
	const float MaxHealthBeforeDrift = Fixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth;
	Fixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth = MaxHealthBeforeDrift + 1.0f;
	// Stale companion terminal result입니다.
	FCFBuilderCompanionResult StaleResult;
	TestFalse(TEXT("VB-P0-05 stale Target blocks companion commit"), FCFVehicleAuthoringService::CreateBuilderCompanions(Request, StaleResult));
	TestTrue(TEXT("VB-P0-05 stale commit creates no Performance binding"), Fixture.Recipe->ProfileBindings.PerformanceProfile.IsNull());
	Fixture.TargetVehicleData->VehicleDurabilityConfig.MaxHealth = MaxHealthBeforeDrift;

	// Restored current state에서 새 exact preview를 만듭니다.
	FCFBuilderCompanionPreview FreshPreview;
	if (!TestTrue(TEXT("VB-P0-05 fresh companion preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderCompanions(Request, FreshPreview)))
	{
		AddError(FreshPreview.Operation.Message);
		return false;
	}
	Request.CallContext.ClientOperationId = TEXT("VB-P0-05-Companion-Commit");
	Request.CallContext.ApprovalScopeHash = FreshPreview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = FreshPreview.Proposal.ResolverContractRevision;

	// Missing Evidence + Performance를 실제 R2 lane으로 생성하는 terminal result입니다.
	FCFBuilderCompanionResult CommitResult;
	if (!TestTrue(TEXT("VB-P0-05 companion commit succeeds"), FCFVehicleAuthoringService::CreateBuilderCompanions(Request, CommitResult)))
	{
		AddError(CommitResult.Operation.Message);
		return false;
	}
	TestNotNull(TEXT("VB-P0-05 Evidence companion created"), CommitResult.CreatedEvidence.Get());
	TestNotNull(TEXT("VB-P0-05 Performance companion created"), CommitResult.CreatedPerformance.Get());
	TestTrue(TEXT("VB-P0-05 created Performance is recipe-bound"), Fixture.Recipe->ProfileBindings.PerformanceProfile.Get() == CommitResult.CreatedPerformance.Get());
	TestEqual(TEXT("VB-P0-05 created Performance owner is Recipe"), CommitResult.CreatedPerformance->Meta.OwnerRecipeId, Fixture.Recipe->RecipeId);
	TestFalse(TEXT("VB-P0-05 companion commit does not save"), CommitResult.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("VB-P0-05 companion commit does not mutate Target"), CommitResult.Operation.Mutation.bTargetChanged);

	// Companion 생성 뒤에도 Target full Definition이 그대로인지 확인합니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, TargetAfter, Error))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("VB-P0-05 companion creation preserves Target Definition"), TargetAfter.DefinitionHash, TargetBefore.DefinitionHash);

	// Existing companion owner를 foreign Recipe로 바꿔 shared/foreign mutation guard를 검증합니다.
	const FGuid PerformanceOwnerBefore = CommitResult.CreatedPerformance->Meta.OwnerRecipeId;
	CommitResult.CreatedPerformance->Meta.OwnerRecipeId = FGuid::NewGuid();
	FCFBuilderCompanionRequest ForeignOwnerRequest = Request;
	ForeignOwnerRequest.ExistingEvidencePath = FSoftObjectPath(CommitResult.CreatedEvidence.Get());
	// Foreign owner preview result입니다.
	FCFBuilderCompanionPreview ForeignOwnerPreview;
	TestFalse(TEXT("VB-P0-05 foreign-owner companion is rejected"), FCFVehicleAuthoringService::PreviewBuilderCompanions(ForeignOwnerRequest, ForeignOwnerPreview));
	CommitResult.CreatedPerformance->Meta.OwnerRecipeId = PerformanceOwnerBefore;
	return true;
}

// Evidence-bound private 4-Profile commit, receipt, stale Profile/Evidence와 idempotent old approval을 직접 검증합니다.
bool FCFVehicleBuilderProfileCommitTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderTestsPrivate;

	// Direct private Profile commit fixture입니다.
	FBuilderFixture Fixture;
	// Fixture/evidence build diagnostic입니다.
	FString Error;
	if (!BuildManagedFixture(Fixture, Error)
		|| !AttachPrivateProfiles(Fixture, true, Error)
		|| !AttachEvidence(Fixture, Error))
	{
		AddError(Error);
		return false;
	}

	// Commit 전 semantic Recipe fingerprint입니다.
	FCFVehicleRecipeSnapshot RecipeBefore;
	// Commit 전 Target full Definition hash입니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Fixture.Recipe, RecipeBefore, Error)
		|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, TargetBefore, Error))
	{
		AddError(Error);
		return false;
	}

	// Current private Profile/Evidence에서 만든 direct R1 request입니다.
	FCFBuilderProfileCommitRequest Request = BuildProfileCommitRequest(Fixture);
	Request.Payload.VehicleBaseData.ChassisWidth = Fixture.VehicleBase->Data.ChassisWidth + 1.0f;
	// Fresh direct Profile commit preview입니다.
	FCFBuilderProfileCommitPreview Preview;
	if (!TestTrue(TEXT("VB-P0-05 Profile commit preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderProfiles(Request, Preview)))
	{
		AddError(Preview.Operation.Message);
		return false;
	}
	BindCommitApproval(Request, Preview, TEXT("VB-P0-05-Profile-Commit"));

	// 성공 commit 전 Profile revision입니다.
	const int32 BaseRevisionBefore = Fixture.VehicleBase->Meta.AuthoringRevision;
	// Direct R1 terminal result입니다.
	FCFAuthoringOpResult CommitResult;
	if (!TestTrue(TEXT("VB-P0-05 Profile commit succeeds"), FCFVehicleAuthoringService::CommitBuilderProfiles(Request, Preview, CommitResult)))
	{
		AddError(CommitResult.Message);
		return false;
	}
	TestTrue(TEXT("VB-P0-05 Profile payload changed"), CommitResult.Mutation.bProfileChanged);
	TestTrue(TEXT("VB-P0-05 receipt metadata changed Recipe"), CommitResult.Mutation.bRecipeChanged);
	TestFalse(TEXT("VB-P0-05 Profile commit does not mutate Target"), CommitResult.Mutation.bTargetChanged);
	TestFalse(TEXT("VB-P0-05 Profile commit does not save"), CommitResult.Mutation.bSavePerformed);
	TestEqual(TEXT("VB-P0-05 Base revision increments once"), Fixture.VehicleBase->Meta.AuthoringRevision, BaseRevisionBefore + 1);
	TestTrue(TEXT("VB-P0-05 persistent Builder receipt is valid"), Fixture.Recipe->BuilderCommitReceipt.IsValid());
	TestEqual(TEXT("VB-P0-05 receipt ProposalHash matches accepted preview"), Fixture.Recipe->BuilderCommitReceipt.ProposalHash, Preview.Proposal.ProposalHash);
	TestEqual(TEXT("VB-P0-05 receipt Evidence fingerprint matches"), Fixture.Recipe->BuilderCommitReceipt.EvidenceFingerprint, Fixture.Evidence->EvidenceFingerprint);

	// Receipt metadata는 Recipe semantic fingerprint를 바꾸지 않아야 합니다.
	FCFVehicleRecipeSnapshot RecipeAfter;
	// Profile commit 뒤 Target은 mutation0여야 합니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Fixture.Recipe, RecipeAfter, Error)
		|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, TargetAfter, Error))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("VB-P0-05 receipt is excluded from semantic Recipe fingerprint"), RecipeAfter.RecipeFingerprint, RecipeBefore.RecipeFingerprint);
	TestEqual(TEXT("VB-P0-05 Profile commit preserves Target Definition"), TargetAfter.DefinitionHash, TargetBefore.DefinitionHash);

	// 같은 old approval을 재호출해도 새 mutation 없이 idempotent NoChange로 끝나야 합니다.
	const int32 BaseRevisionAfterCommit = Fixture.VehicleBase->Meta.AuthoringRevision;
	// Old approval replay result입니다.
	FCFAuthoringOpResult ReplayResult;
	TestTrue(TEXT("VB-P0-05 old approval replay is terminal NoChange"), FCFVehicleAuthoringService::CommitBuilderProfiles(Request, Preview, ReplayResult));
	TestEqual(TEXT("VB-P0-05 old approval replay status is NoChange"), ReplayResult.Status, ECFAuthoringOpStatus::NoChange);
	TestEqual(TEXT("VB-P0-05 old approval replay does not increment revision"), Fixture.VehicleBase->Meta.AuthoringRevision, BaseRevisionAfterCommit);

	// 새 prospective payload를 preview한 뒤 다른 Profile raw drift를 삽입해 stale fingerprint를 검증합니다.
	FCFBuilderProfileCommitRequest StaleProfileRequest = BuildProfileCommitRequest(Fixture);
	StaleProfileRequest.Payload.VehicleBaseData.ChassisWidth = Fixture.VehicleBase->Data.ChassisWidth + 2.0f;
	// Stale test용 approved preview입니다.
	FCFBuilderProfileCommitPreview StaleProfilePreview;
	if (!TestTrue(TEXT("VB-P0-05 stale Profile setup preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderProfiles(StaleProfileRequest, StaleProfilePreview)))
	{
		AddError(StaleProfilePreview.Operation.Message);
		return false;
	}
	BindCommitApproval(StaleProfileRequest, StaleProfilePreview, TEXT("VB-P0-05-Stale-Profile"));
	// Transaction 밖 raw Profile drift 전 값입니다.
	const float BrakeTorqueBeforeDrift = Fixture.Handling->Data.FrontWheelMaxBrakeTorque;
	Fixture.Handling->Data.FrontWheelMaxBrakeTorque = BrakeTorqueBeforeDrift + 1.0f;
	// Stale Profile commit result입니다.
	FCFAuthoringOpResult StaleProfileResult;
	TestFalse(TEXT("VB-P0-05 stale current Profile blocks commit"), FCFVehicleAuthoringService::CommitBuilderProfiles(StaleProfileRequest, StaleProfilePreview, StaleProfileResult));
	Fixture.Handling->Data.FrontWheelMaxBrakeTorque = BrakeTorqueBeforeDrift;

	// Evidence fingerprint를 preview 이후 바꿔 stale Evidence를 검증합니다.
	FCFBuilderProfileCommitRequest StaleEvidenceRequest = BuildProfileCommitRequest(Fixture);
	StaleEvidenceRequest.Payload.VehicleBaseData.ChassisWidth = Fixture.VehicleBase->Data.ChassisWidth + 3.0f;
	// Evidence stale test용 preview입니다.
	FCFBuilderProfileCommitPreview StaleEvidencePreview;
	if (!TestTrue(TEXT("VB-P0-05 stale Evidence setup preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderProfiles(StaleEvidenceRequest, StaleEvidencePreview)))
	{
		AddError(StaleEvidencePreview.Operation.Message);
		return false;
	}
	BindCommitApproval(StaleEvidenceRequest, StaleEvidencePreview, TEXT("VB-P0-05-Stale-Evidence"));
	// Evidence semantic claim의 preview 전 값입니다.
	const double FactValueBeforeDrift = Fixture.Evidence->Claims[0].NumberValue;
	Fixture.Evidence->Claims[0].NumberValue = FactValueBeforeDrift + 1.0;
	if (!Fixture.Evidence->RefreshEvidenceFingerprint(Error))
	{
		AddError(Error);
		return false;
	}
	// Stale Evidence commit result입니다.
	FCFAuthoringOpResult StaleEvidenceResult;
	TestFalse(TEXT("VB-P0-05 stale Evidence blocks commit"), FCFVehicleAuthoringService::CommitBuilderProfiles(StaleEvidenceRequest, StaleEvidencePreview, StaleEvidenceResult));
	Fixture.Evidence->Claims[0].NumberValue = FactValueBeforeDrift;
	if (!Fixture.Evidence->RefreshEvidenceFingerprint(Error))
	{
		AddError(Error);
		return false;
	}

	// One private Profile의 owner를 foreign으로 바꿔 shared/legacy fail-closed guard를 검증합니다.
	const FGuid PerformanceOwnerBefore = Fixture.Performance->Meta.OwnerRecipeId;
	Fixture.Performance->Meta.OwnerRecipeId.Invalidate();
	// Foreign/shared owner preview result입니다.
	FCFBuilderProfileCommitPreview ForeignOwnerPreview;
	TestFalse(TEXT("VB-P0-05 shared/foreign Profile owner is rejected"), FCFVehicleAuthoringService::PreviewBuilderProfiles(BuildProfileCommitRequest(Fixture), ForeignOwnerPreview));
	Fixture.Performance->Meta.OwnerRecipeId = PerformanceOwnerBefore;
	return true;
}

#endif
