// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBuilderTests.cpp
// Version: v1.16.0
// Date: 2026-09-02
// Description: CF-FQ-040 Builder write lane + ESH-02/03 vehicle-specific Engine Curve/Transmission provenance focused Automation입니다.
// Scope: R2 companion, Evidence Refresh, R1 Profile commit/receipt, Transmission contract와 Engine Curve FACT/DERIVED/GAME_BIAS review/hash 및 post-DefinitionApply persistent Target을 직접 검증합니다.
// Changelog:
// - v1.16.0: actual Wagon persisted expectation을 ESH-03 USER-approved ChangeUpRPM 5500 Target DefinitionApply 뒤 상태로 갱신. Transmission receipt e1be..., Target hash 0e5b..., Target/Drivetrain 5500/2000을 exact 검증.
// - v1.15.0: actual Wagon mutation0 PhysicsDraft preview가 Engine Curve hash뿐 아니라 current TransmissionProposalHash도 Saved JSON/log에 노출해 ESH-03 fixed-common proposal exact identity를 재사용 가능하게 함.
// - v1.14.0: actual Wagon persisted test를 pre-Apply Target=false invariant에서 post-DefinitionApply Target=true + exact 6-point Curve + AppliedState hash/revision readback으로 전환.
// - v1.13.0: ESH-02 Baseline compatibility, GAME_BIAS/DERIVED Engine Curve provenance, consumed Claim binding, deterministic hash, invalid curve 및 persistent receipt commit/resume 회귀를 추가.
// - v1.12.0: Wagon 비종속 generic Transmission contract에 Blocked Resolver의 WheelRadius fallback을 ShiftSpeed authority로 소비하지 않는 fail-closed 회귀를 추가.
// - v1.11.0: actual Wagon regression이 current Resolver authority 상태에 따라 ShiftSpeed 제공/차단을 검증하도록 보강해 WSA reacceptance 전후 모두 재사용 가능하게 함.
// - v1.10.0: actual Wagon transmission diagnostic의 WSA radius authority 추적을 위해 current ResolveResult 전체를 Saved mutation0 diagnostic JSON으로 기록.
// - v1.9.0: actual Wagon Saved PhysicsDraft v2를 prospective transient Evidence에 binding해 VehicleSpecificRequired TransmissionReview, consumed Claim, 8단 fixed-shift diagnostic/blocker0을 Product Asset mutation0으로 dry-run 검증.
// - v1.8.0: actual Wagon persisted Recipe/Evidence + canonical Saved ResearchDraft를 mutation0 Existing Evidence Refresh preview로 dry-run해 schema/binding/semantic integrity와 prospective EvidenceFingerprint를 산출하는 actual regression을 추가.
// - v1.7.0: Existing Reference Evidence complete replacement R1의 preview/explicit approval/identity preservation/stale replay/NoChange/GAME_BIAS reject/Save0 회귀를 BuilderEvidenceRefresh로 추가.
// - v1.6.0: generic Transmission fixture를 실제 research FactKey/value에 binding하고 semantic/value mismatch, DERIVED input stale, GAME_BIAS gear-count origin, empty/zero ratio/final payload fail-closed 회귀를 추가.
// - v1.5.0: Wagon 비종속 synthetic fixture로 Legacy/VehicleSpecific policy, FACT/DERIVED/GAME_BIAS provenance, missing/stale provenance, gear count/ratio order/shift RPM blocker와 deterministic Transmission hash를 검증하는 BuilderTransmissionContract 회귀를 추가.
// - v1.4.0: VehicleSpecificRequired Recipe가 bUseTransmissionConfig=false compatibility fallback을 Step 5 Profile Preview에서 fail-closed하고 LegacyCompatible fixture는 기존 commit을 계속 허용하는 회귀를 추가.
// - v1.3.0: actual Wagon에서 드러난 managed Recipe + private Profile 0/4 NewVehicle bootstrap을 complete Wagon-like payload로 직접 Preview하고 BlueprintGeneratedClass/native Class canonical roundtrip을 모두 회귀 검증.
// - v1.2.0: SocketScaleFromChassis에서 unrelated Profile change는 허용하고 Reference wheel geometry 5필드 mutation은 Preview에서 fail-closed 검증.
// - v1.1.0: VB-P0-09 Step 1 initial Research payload를 Companion flow에 추가하고 empty/GAME_BIAS reject 및 preview→commit Evidence fingerprint exact binding을 검증.
// - v1.0.0: VB-P0-05 핵심 write facade를 실제 호출하는 direct Automation 2건을 추가.
// Migration:
// - 모든 fixture는 in-memory /Temp package만 사용하며 SavePackage를 호출하지 않습니다.
// - 테스트가 만든 package/object는 Automation process lifetime에만 존재합니다.

#include "Algo/Reverse.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFBuilderEngineUtil.h"
#include "DataAuthoring/CFBuilderTransUtil.h"
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

	// Companion new Evidence에 사용할 최소 canonical FACT Research payload를 구성합니다.
	FCFVehicleRefEvidencePayload BuildInitialEvidencePayload()
	{
		// Initial Evidence에 넣을 complete research payload입니다.
		FCFVehicleRefEvidencePayload Payload;

		// Exact Primary Reference identity입니다.
		FCFRefVehicleIdentity& ReferenceVehicle = Payload.ReferenceVehicles.AddDefaulted_GetRef();
		ReferenceVehicle.ReferenceVehicleId = TEXT("REF-VB-AUTO");
		ReferenceVehicle.Role = ECFRefVehicleRole::Primary;
		ReferenceVehicle.Manufacturer = TEXT("CarFightAutomation");
		ReferenceVehicle.Model = TEXT("BaselineVehicle");
		ReferenceVehicle.ModelYearStart = 2026;
		ReferenceVehicle.ModelYearEnd = 2026;
		ReferenceVehicle.ModelYearQualifier = ECFRefModelYearQualifier::Exact;
		ReferenceVehicle.MarketRegion = TEXT("TEST");
		ReferenceVehicle.IdentityConfidence = 0.95f;

		// FACT를 뒷받침하는 exact test citation입니다.
		FCFRefSourceCitation& Source = Payload.Sources.AddDefaulted_GetRef();
		Source.SourceId = TEXT("SRC-VB-AUTO");
		Source.Tier = ECFRefSourceTier::TierA;
		Source.SourceKind = TEXT("AutomationFixture");
		Source.Publisher = TEXT("CarFight");
		Source.DocumentTitle = TEXT("VB Companion Test Evidence");
		Source.CanonicalUrl = TEXT("https://example.invalid/carfight/vb-companion");
		Source.ReferenceVehicleIds = {ReferenceVehicle.ReferenceVehicleId};
		Source.OriginGroupId = TEXT("ORG-VB-AUTO");
		Source.OriginIndependence = ECFRefOriginIndependence::IndependentOrigin;

		// Baseline mass를 표현하는 canonical FACT claim입니다.
		FCFRefClaim& FactClaim = Payload.Claims.AddDefaulted_GetRef();
		FactClaim.ClaimId = TEXT("CLAIM-VB-MASS");
		FactClaim.ReferenceVehicleId = ReferenceVehicle.ReferenceVehicleId;
		FactClaim.FactKey = TEXT("CurbMassKg");
		FactClaim.ValueKind = ECFRefValueKind::Number;
		FactClaim.NumberValue = 1540.0;
		FactClaim.UnitId = TEXT("kg");
		FactClaim.SourceValueText = TEXT("1540 kg");
		FactClaim.Provenance = ECFRefProvenance::FACT;
		FactClaim.CitationIds = {Source.SourceId};
		FactClaim.ConfidenceScore = 0.95f;
		FactClaim.ResolutionState = ECFRefClaimResolution::Canonical;

		Payload.ResearchNotes = TEXT("Automation-only normalized Research payload.");
		return Payload;
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

	// 차종 이름이나 Wagon Asset에 의존하지 않는 4단 automatic Transmission fixture payload를 구성합니다.
	FCFDrivetrainProfileData BuildGenericTransmissionPayload()
	{
		// Builder-wide contract를 검증할 synthetic Drivetrain payload입니다.
		FCFDrivetrainProfileData Data;
		Data.bUseTransmissionConfig = true;
		Data.bUseAutomaticGears = true;
		Data.bUseAutoReverse = true;
		Data.TransmissionRatios.ForwardGearRatios = {3.20f, 2.05f, 1.38f, 1.00f};
		Data.TransmissionRatios.ReverseGearRatios = {3.10f};
		Data.FinalRatio = 3.55f;
		Data.ChangeUpRPM = 4200.0f;
		Data.ChangeDownRPM = 1800.0f;
		Data.GearChangeTime = 0.35f;
		Data.TransmissionEfficiency = 0.90f;
		return Data;
	}

	// Ordered ratio set을 Evidence CanonicalText grammar로 직렬화합니다.
	FString BuildRatioClaimText(const TArray<float>& Ratios)
	{
		// Comma-separated canonical ratio 문자열입니다.
		FString Result;
		for (const float Ratio : Ratios)
		{
			if (!Result.IsEmpty())
			{
				Result += TEXT(",");
			}
			Result += LexToString(Ratio);
		}
		return Result;
	}

	// 특정 Transmission semantic의 Evidence Claim 값을 current payload와 동일하게 채웁니다.
	void SetClaimValueForSemantic(
		FCFRefClaim& Claim,
		const FName SemanticKey,
		const FCFDrivetrainProfileData& Payload)
	{
		if (SemanticKey == CFBuilderTransUtil::AutomaticGearsKey)
		{
			Claim.ValueKind = ECFRefValueKind::Boolean;
			Claim.BooleanValue = Payload.bUseAutomaticGears;
			return;
		}
		if (SemanticKey == CFBuilderTransUtil::ForwardRatiosKey)
		{
			Claim.ValueKind = ECFRefValueKind::CanonicalText;
			Claim.TextValue = BuildRatioClaimText(Payload.TransmissionRatios.ForwardGearRatios);
			return;
		}
		if (SemanticKey == CFBuilderTransUtil::ReverseRatiosKey)
		{
			Claim.ValueKind = Payload.TransmissionRatios.ReverseGearRatios.Num() == 1
				? ECFRefValueKind::Number
				: ECFRefValueKind::CanonicalText;
			if (Payload.TransmissionRatios.ReverseGearRatios.Num() == 1)
			{
				Claim.NumberValue = Payload.TransmissionRatios.ReverseGearRatios[0];
			}
			else
			{
				Claim.TextValue = BuildRatioClaimText(Payload.TransmissionRatios.ReverseGearRatios);
			}
			return;
		}
		Claim.ValueKind = ECFRefValueKind::Number;
		Claim.NumberValue = SemanticKey == CFBuilderTransUtil::FinalRatioKey
			? Payload.FinalRatio
			: (SemanticKey == CFBuilderTransUtil::ChangeUpRpmKey ? Payload.ChangeUpRPM : Payload.ChangeDownRPM);
		Claim.UnitId = SemanticKey == CFBuilderTransUtil::ChangeUpRpmKey || SemanticKey == CFBuilderTransUtil::ChangeDownRpmKey
			? FName(TEXT("rpm"))
			: NAME_None;
	}

	// Transmission output semantic에 대응하는 synthetic canonical Claim을 추가합니다.
	FCFRefClaim& AddTransmissionOutputClaim(
		UCFVehicleRefEvidence& Evidence,
		const FName ClaimId,
		const FName SemanticKey,
		const ECFRefProvenance Provenance,
		const FCFDrivetrainProfileData& Payload)
	{
		// 하나의 semantic-bound canonical Transmission Claim입니다.
		FCFRefClaim& Claim = Evidence.Claims.AddDefaulted_GetRef();
		Claim.ClaimId = ClaimId;
		Claim.FactKey = CFBuilderTransUtil::GetOutputFactKeyForSemantic(SemanticKey);
		Claim.Provenance = Provenance;
		Claim.ResolutionState = ECFRefClaimResolution::Canonical;
		SetClaimValueForSemantic(Claim, SemanticKey, Payload);
		return Claim;
	}

	// DERIVED output 또는 automatic control GAME_BIAS가 소비할 canonical FACT input을 추가합니다.
	FCFRefClaim& AddTransmissionInputFact(
		UCFVehicleRefEvidence& Evidence,
		const FName ClaimId,
		const FName FactKey,
		const FName SemanticKey,
		const FCFDrivetrainProfileData& Payload)
	{
		// 실제 proposal input 역할의 canonical FACT Claim입니다.
		FCFRefClaim& Claim = Evidence.Claims.AddDefaulted_GetRef();
		Claim.ClaimId = ClaimId;
		Claim.FactKey = FactKey;
		Claim.Provenance = ECFRefProvenance::FACT;
		Claim.ResolutionState = ECFRefClaimResolution::Canonical;
		if (FactKey == CFBuilderTransUtil::TransmissionTypeFactKey)
		{
			Claim.ValueKind = ECFRefValueKind::CanonicalText;
			Claim.TextValue = TEXT("Automatic");
		}
		else if (FactKey == CFBuilderTransUtil::GearCountFactKey)
		{
			Claim.ValueKind = ECFRefValueKind::Integer;
			Claim.IntegerValue = Payload.TransmissionRatios.ForwardGearRatios.Num();
		}
		else
		{
			SetClaimValueForSemantic(Claim, SemanticKey, Payload);
		}
		return Claim;
	}

	// GAME_BIAS가 exact 실차 값 부재를 보존할 synthetic Unknown Fact를 추가합니다.
	FCFRefUnknownFact& AddTransmissionUnknown(
		UCFVehicleRefEvidence& Evidence,
		const FName UnknownFactId,
		const FName FactKey)
	{
		// 숫자로 대체하지 않는 semantic-bound Unknown Fact입니다.
		FCFRefUnknownFact& UnknownFact = Evidence.UnknownFacts.AddDefaulted_GetRef();
		UnknownFact.UnknownFactId = UnknownFactId;
		UnknownFact.FactKey = FactKey;
		UnknownFact.Reason = ECFRefUnknownReason::NotPublished;
		return UnknownFact;
	}

	// 7개 Transmission Core semantic을 요청 provenance 중심의 complete synthetic review로 구성합니다.
	FCFBuilderTransmissionReview BuildGenericTransmissionReview(
		UCFVehicleRefEvidence& Evidence,
		const ECFBuilderTransmissionDisposition RequestedDisposition,
		TArray<FName>& OutConsumedClaimIds)
	{
		// Builder-wide synthetic payload authority입니다.
		const FCFDrivetrainProfileData Payload = BuildGenericTransmissionPayload();
		// Builder-wide synthetic review입니다.
		FCFBuilderTransmissionReview Review;
		Review.SchemaRevision = 1;
		Review.ExpectedForwardGearCount = Payload.TransmissionRatios.ForwardGearRatios.Num();
		OutConsumedClaimIds.Reset();

		// Vehicle-specific completion이 요구하는 exact Core semantic 목록입니다.
		const FName CoreKeys[] =
		{
			CFBuilderTransUtil::AutomaticGearsKey,
			CFBuilderTransUtil::AutoReverseKey,
			CFBuilderTransUtil::ForwardRatiosKey,
			CFBuilderTransUtil::ReverseRatiosKey,
			CFBuilderTransUtil::FinalRatioKey,
			CFBuilderTransUtil::ChangeUpRpmKey,
			CFBuilderTransUtil::ChangeDownRpmKey
		};

		// Core semantic을 deterministic index로 순회합니다.
		for (int32 CoreIndex = 0; CoreIndex < UE_ARRAY_COUNT(CoreKeys); ++CoreIndex)
		{
			// 현재 field의 stable semantic key입니다.
			const FName SemanticKey = CoreKeys[CoreIndex];
			// AutoReverse는 실차 direct/derived fact가 아니라 CarFight control policy이므로 항상 GAME_BIAS로 검증합니다.
			const ECFBuilderTransmissionDisposition EffectiveDisposition = SemanticKey == CFBuilderTransUtil::AutoReverseKey
				? ECFBuilderTransmissionDisposition::GameBias
				: RequestedDisposition;
			// 한 Core field의 reviewed provenance입니다.
			FCFBuilderTransmissionComponentReview& Component = Review.Components.AddDefaulted_GetRef();
			Component.SemanticKey = SemanticKey;
			Component.Disposition = EffectiveDisposition;

			if (EffectiveDisposition == ECFBuilderTransmissionDisposition::EvidenceDirect)
			{
				// Direct FACT output Claim ID입니다.
				const FName ClaimId(*FString::Printf(TEXT("TRANS_DIRECT_%02d"), CoreIndex));
				AddTransmissionOutputClaim(Evidence, ClaimId, SemanticKey, ECFRefProvenance::FACT, Payload);
				Component.EvidenceClaimIds.Add(ClaimId);
				OutConsumedClaimIds.Add(ClaimId);
			}
			else if (EffectiveDisposition == ECFBuilderTransmissionDisposition::EvidenceDerived)
			{
				// DERIVED output Claim ID입니다.
				const FName DerivedClaimId(*FString::Printf(TEXT("TRANS_DERIVED_%02d"), CoreIndex));
				// DERIVED calculation input FACT Claim ID입니다.
				const FName InputClaimId(*FString::Printf(TEXT("TRANS_INPUT_%02d"), CoreIndex));
				// Automatic boolean은 TransmissionType에서 파생하고 나머지는 same-semantic normalized FACT input을 사용합니다.
				const FName InputFactKey = SemanticKey == CFBuilderTransUtil::AutomaticGearsKey
					? CFBuilderTransUtil::TransmissionTypeFactKey
					: CFBuilderTransUtil::GetOutputFactKeyForSemantic(SemanticKey);
				AddTransmissionInputFact(Evidence, InputClaimId, InputFactKey, SemanticKey, Payload);
				FCFRefClaim& DerivedClaim = AddTransmissionOutputClaim(Evidence, DerivedClaimId, SemanticKey, ECFRefProvenance::DERIVED, Payload);
				DerivedClaim.InputClaimIds.Add(InputClaimId);
				DerivedClaim.MethodId = TEXT("SyntheticDerivedTransmission");
				DerivedClaim.MethodRevision = 1;
				Component.EvidenceClaimIds.Add(DerivedClaimId);
				Component.MethodId = DerivedClaim.MethodId;
				Component.MethodRevision = DerivedClaim.MethodRevision;
				OutConsumedClaimIds.Add(InputClaimId);
				OutConsumedClaimIds.Add(DerivedClaimId);
			}
			else if (EffectiveDisposition == ECFBuilderTransmissionDisposition::GameBias)
			{
				Component.MethodId = SemanticKey == CFBuilderTransUtil::AutomaticGearsKey || SemanticKey == CFBuilderTransUtil::AutoReverseKey
					? FName(TEXT("CarFightAutomaticControlApproximation"))
					: FName(TEXT("FixedShiftTransmissionBias"));
				Component.MethodRevision = 1;
				Component.MethodParameters.Add(TEXT("SyntheticFixture"), TEXT("1"));
				if (SemanticKey == CFBuilderTransUtil::AutomaticGearsKey || SemanticKey == CFBuilderTransUtil::AutoReverseKey)
				{
					// Control policy가 소비하는 TransmissionType FACT ID입니다.
					const FName TransmissionTypeClaimId(*FString::Printf(TEXT("TRANS_TYPE_%02d"), CoreIndex));
					AddTransmissionInputFact(Evidence, TransmissionTypeClaimId, CFBuilderTransUtil::TransmissionTypeFactKey, SemanticKey, Payload);
					Component.EvidenceClaimIds.Add(TransmissionTypeClaimId);
					OutConsumedClaimIds.Add(TransmissionTypeClaimId);
				}
				else
				{
					// Exact real-world semantic이 Unknown임을 보존하는 ID입니다.
					const FName UnknownFactId(*FString::Printf(TEXT("TRANS_UNKNOWN_%02d"), CoreIndex));
					AddTransmissionUnknown(Evidence, UnknownFactId, CFBuilderTransUtil::GetOutputFactKeyForSemantic(SemanticKey));
					Component.UnknownFactIds.Add(UnknownFactId);
					if (SemanticKey == CFBuilderTransUtil::ForwardRatiosKey)
					{
						// GAME_BIAS ratio-set의 단수 origin을 별도로 보존하는 Unknown ID입니다.
						const FName GearCountUnknownId(TEXT("TRANS_UNKNOWN_GEAR_COUNT"));
						AddTransmissionUnknown(Evidence, GearCountUnknownId, CFBuilderTransUtil::GearCountFactKey);
						Component.UnknownFactIds.Add(GearCountUnknownId);
					}
				}
			}
		}
		return Review;
	}

	// Engine Curve provenance fixture에 canonical numeric/range FACT를 추가합니다.
	FCFRefClaim& AddEngineFact(
		UCFVehicleRefEvidence& Evidence,
		const FName ClaimId,
		const FName FactKey,
		const double ValueA,
		const double ValueB = 0.0)
	{
		FCFRefClaim& Claim = Evidence.Claims.AddDefaulted_GetRef();
		Claim.ClaimId = ClaimId;
		Claim.FactKey = FactKey;
		Claim.Provenance = ECFRefProvenance::FACT;
		Claim.ResolutionState = ECFRefClaimResolution::Canonical;
		if (FactKey == CFBuilderEngineUtil::TorqueBandFactKey || FactKey == CFBuilderEngineUtil::PowerBandFactKey)
		{
			Claim.ValueKind = ECFRefValueKind::Range;
			Claim.RangeMinValue = ValueA;
			Claim.RangeMaxValue = ValueB;
			Claim.UnitId = TEXT("rpm");
		}
		else
		{
			Claim.ValueKind = ECFRefValueKind::Number;
			Claim.NumberValue = ValueA;
			Claim.UnitId = FactKey == CFBuilderEngineUtil::MaxTorqueFactKey ? FName(TEXT("Nm")) : FName(TEXT("kW"));
		}
		return Claim;
	}

	// ESH-02 generic sparse-anchor vehicle-specific Engine Curve payload를 구성합니다.
	FCFPerformanceProfileData BuildGenericEngineCurvePayload()
	{
		FCFPerformanceProfileData Data;
		Data.EngineMaxTorqueByFeel = {280.0f, 350.0f, 430.0f};
		Data.EngineMaxRPMByFeel = {5500.0f, 6500.0f, 7500.0f};
		Data.ThrottleInputScaleByFeel = {0.8f, 1.0f, 1.1f};
		Data.EngineIdleRPM = 900.0f;
		Data.bUseEngineTorqueCurve = true;

		auto AddPoint = [&Data](const float Rpm, const float Multiplier)
		{
			FCFVehicleEngineTorquePoint& Point = Data.EngineTorqueCurve.Points.AddDefaulted_GetRef();
			Point.EngineRPM = Rpm;
			Point.TorqueMultiplier = Multiplier;
		};
		AddPoint(900.0f, 0.60f);
		AddPoint(1800.0f, 1.00f);
		AddPoint(4800.0f, 1.00f);
		AddPoint(5550.0f, 0.90f);
		AddPoint(6500.0f, 0.65f);
		return Data;
	}

	// Sparse FACT anchors + explicit method를 가진 generic GAME_BIAS Engine Curve review입니다.
	FCFBuilderEngineCurveReview BuildGenericEngineCurveReview(const TArray<FName>& EngineClaimIds)
	{
		FCFBuilderEngineCurveReview Review;
		Review.SchemaRevision = 1;
		Review.Disposition = ECFBuilderEngineCurveDisposition::GameBias;
		Review.EvidenceClaimIds = EngineClaimIds;
		Review.MethodId = TEXT("SparseAnchorEngineCurveBias");
		Review.MethodRevision = 1;
		Review.MethodParameters.Add(TEXT("Interpolation"), TEXT("Linear"));
		Review.MethodParameters.Add(TEXT("Intent"), TEXT("Sparse FACT anchors plus explicit low/tail GAME_BIAS"));
		return Review;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderTransmissionContractTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderTransmissionContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderEngineCurveTest,
	"CarFight.DataAuthoring.CF_FQ_040.ESH_02.BuilderEngineCurveContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderWagonEngineTest,
	"CarFight.DataAuthoring.CF_FQ_040.ESH_02.WagonEngineCurveDraft",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderWagonEnginePersistedTest,
	"CarFight.DataAuthoring.CF_FQ_040.ESH_02.WagonEngineCurvePersisted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderEvidenceRefreshTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderEvidenceRefresh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderWagonDraftTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.WagonTransmissionDraft",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderAllProfileBootstrapTest,
	"CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderCompanionBootstrapAllProfiles",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Managed Recipe의 private Profile 0/4에서 Content Blueprint WheelClass가 effective winner가 되는 bootstrap 경로를 일반화해 검증합니다.
bool FCFVehicleBuilderAllProfileBootstrapTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderTestsPrivate;

	FBuilderFixture Fixture;
	FString Error;
	if (!BuildManagedFixture(Fixture, Error))
	{
		AddError(Error);
		return false;
	}

	// actual Wagon처럼 legacy pinned baseline 없이 Recipe/Profile/Asset source가 effective winner가 되도록 import pin을 제거합니다.
	Fixture.Recipe->ImportState.LegacyPinnedFields.Reset();
	Fixture.Recipe->ImportState.LegacySerializedFields.Reset();
	Fixture.Recipe->ImportState.ImportedDefinitionHash.Reset();

	// actual Wagon과 동일하게 USER Chassis Socket Scale 경로를 사용하되 Profile은 0/4 상태를 유지합니다.
	Fixture.Recipe->WheelVisualIntent.Mode = ECFWheelVisualIntentMode::SocketScaleFromChassis;
	TestTrue(TEXT("0/4 bootstrap starts without VehicleBase"), Fixture.Recipe->ProfileBindings.VehicleBaseProfile.IsNull());
	TestTrue(TEXT("0/4 bootstrap starts without Drivetrain"), Fixture.Recipe->ProfileBindings.DrivetrainProfile.IsNull());
	TestTrue(TEXT("0/4 bootstrap starts without Handling"), Fixture.Recipe->ProfileBindings.HandlingProfile.IsNull());
	TestTrue(TEXT("0/4 bootstrap starts without Performance"), Fixture.Recipe->ProfileBindings.PerformanceProfile.IsNull());

	FCFBuilderCompanionRequest Request;
	Request.Recipe = Fixture.Recipe;
	Request.Mode = ECFBuilderCompanionMode::NewVehicle;
	Request.bHasInitialEvidencePayload = true;
	Request.NewEvidenceId = FGuid::NewGuid();
	Request.InitialEvidencePayload = BuildInitialEvidencePayload();
	Request.bHasInitialProfilePayload = true;
	Request.EvidenceAsset = MakeAssetIdentity(TEXT("CFVBAllEvidence"), TEXT("DA_VB_All_Evidence"));
	Request.VehicleBaseAsset = MakeAssetIdentity(TEXT("CFVBAllBase"), TEXT("DA_VB_All_Base"));
	Request.DrivetrainAsset = MakeAssetIdentity(TEXT("CFVBAllDrive"), TEXT("DA_VB_All_Drive"));
	Request.HandlingAsset = MakeAssetIdentity(TEXT("CFVBAllHandling"), TEXT("DA_VB_All_Handling"));
	Request.PerformanceAsset = MakeAssetIdentity(TEXT("CFVBAllPerformance"), TEXT("DA_VB_All_Performance"));
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;

	FCFVehicleBaseProfileData& Base = Request.InitialProfilePayload.VehicleBaseData;
	Base.BaseVehicleMassKg = 1886.0f;
	Base.MaximumGrossMassKg = 2350.0f;
	Base.MaxHealth = 100.0f;
	Base.ChassisWidth = 185.0f;
	Base.ChassisHeight = 149.0f;
	Base.bUseReferenceWheelGeometry = false;
	Base.FrontWheelRadius = 30.0f;
	Base.RearWheelRadius = 30.0f;
	Base.FrontWheelWidth = 6.0f;
	Base.RearWheelWidth = 6.0f;
	Base.ExpectedWheelCount = 4;
	Base.FrontWheelCountForSteering = 2;
	Base.bAutoScaleWheelMeshToRadius = false;
	Base.WheelMeshRadiusMeasureMode = ECFWheelMeshRadiusMeasureMode::AutoMaxXZ;
	Base.bAutoCenterWheelMeshBoundsToOrigin = true;
	Base.WheelMeshScaleClampMin = 0.25f;
	Base.WheelMeshScaleClampMax = 4.0f;

	FCFDrivetrainProfileData& Drive = Request.InitialProfilePayload.DrivetrainData;
	Drive.DifferentialType = EVehicleDifferential::AllWheelDrive;
	Drive.FrontRearSplit = 0.5f;
	Drive.bUseTransmissionConfig = false;
	Drive.bUseAutomaticGears = true;
	Drive.bUseAutoReverse = true;
	Drive.FinalRatio = 3.08f;
	Drive.ChangeUpRPM = 4500.0f;
	Drive.ChangeDownRPM = 2000.0f;
	Drive.GearChangeTime = 0.4f;
	Drive.TransmissionEfficiency = 0.9f;
	Drive.bFrontWheelAffectedByEngine = true;
	Drive.bRearWheelAffectedByEngine = true;

	// actual Wagon ResearchDraft가 사용하는 Blueprint Generated Wheel Class 두 개를 exact path로 로드합니다.
	UClass* FrontWheelBlueprintClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/CarFight/Vehicles/Blueprints/BP_Wheel_Front.BP_Wheel_Front_C"));
	UClass* RearWheelBlueprintClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/CarFight/Vehicles/Blueprints/BP_Wheel_Rear.BP_Wheel_Rear_C"));
	if (!FrontWheelBlueprintClass || !RearWheelBlueprintClass)
	{
		AddError(TEXT("actual Wagon Blueprint Wheel Class를 load할 수 없습니다."));
		return false;
	}
	Drive.FrontWheelClass = FrontWheelBlueprintClass;
	Drive.RearWheelClass = RearWheelBlueprintClass;

	// USTRUCT aggregate initialization에 의존하지 않고 exact 세 값을 명시적으로 채웁니다.
	const auto SetFeelResponse = [](FCFFeelResponse& Response, const float Low, const float Neutral, const float High)
	{
		Response.LowValue = Low;
		Response.NeutralValue = Neutral;
		Response.HighValue = High;
	};

	FCFHandlingProfileData& Handling = Request.InitialProfilePayload.HandlingData;
	SetFeelResponse(Handling.FrontWheelMaxSteerAngleByFeel, 28.0f, 35.0f, 42.0f);
	SetFeelResponse(Handling.SteeringAngleRatioByFeel, 0.55f, 0.70f, 0.85f);
	SetFeelResponse(Handling.FrontWheelFrictionByFeel, 1.6f, 2.0f, 2.4f);
	SetFeelResponse(Handling.RearWheelFrictionByFeel, 1.6f, 2.0f, 2.4f);
	SetFeelResponse(Handling.FrontCorneringByFeel, 800.0f, 1000.0f, 1200.0f);
	SetFeelResponse(Handling.RearCorneringByFeel, 800.0f, 1000.0f, 1200.0f);
	SetFeelResponse(Handling.FrontSpringRateByFeel, 200.0f, 250.0f, 300.0f);
	SetFeelResponse(Handling.RearSpringRateByFeel, 200.0f, 250.0f, 300.0f);
	SetFeelResponse(Handling.FrontSpringPreloadByFeel, 40.0f, 50.0f, 60.0f);
	SetFeelResponse(Handling.RearSpringPreloadByFeel, 40.0f, 50.0f, 60.0f);
	Handling.FrontWheelMaxBrakeTorque = 1500.0f;
	Handling.RearWheelMaxBrakeTorque = 1500.0f;
	Handling.RearWheelMaxHandBrakeTorque = 3000.0f;
	Handling.FrontWheelLoadRatio = 0.5f;
	Handling.RearWheelLoadRatio = 0.5f;
	Handling.FrontWheelSuspensionMaxRaise = 8.0f;
	Handling.RearWheelSuspensionMaxRaise = 10.0f;
	Handling.FrontWheelSuspensionMaxDrop = 10.0f;
	Handling.RearWheelSuspensionMaxDrop = 10.0f;
	Handling.FrontWheelSweepShape = ESweepShape::Raycast;
	Handling.RearWheelSweepShape = ESweepShape::Raycast;
	Handling.SteeringType = ESteeringType::AngleRatio;

	FCFPerformanceProfileData& Performance = Request.InitialProfilePayload.PerformanceData;
	SetFeelResponse(Performance.EngineMaxTorqueByFeel, 280.0f, 350.0f, 450.0f);
	SetFeelResponse(Performance.EngineMaxRPMByFeel, 5500.0f, 6500.0f, 7500.0f);
	SetFeelResponse(Performance.ThrottleInputScaleByFeel, 0.8f, 1.0f, 1.0f);
	Performance.EngineIdleRPM = 900.0f;
	Performance.RedlineStartRPM = 0.0f;
	Performance.EngineBrakeEffect = 0.2f;
	Performance.EngineRevUpMOI = 5.0f;
	Performance.EngineRevDownRate = 600.0f;
	Performance.DragCoefficient = 0.3f;
	Performance.DownforceCoefficient = 0.3f;

	FCFBuilderCompanionPreview Preview;
	if (!TestTrue(TEXT("0/4 Wagon-like companion prospective preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderCompanions(Request, Preview)))
	{
		AddError(Preview.Operation.Message);
		return false;
	}
	TestFalse(TEXT("0/4 bootstrap preview performs no Asset creation"), Preview.Operation.Mutation.bCreatedAssets);
	TestFalse(TEXT("0/4 bootstrap preview performs no Target mutation"), Preview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("0/4 bootstrap preview performs no Save"), Preview.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("0/4 bootstrap prospective hash is present"), Preview.ProspectiveResolvedDefinitionHash.IsEmpty());

	// 같은 0/4 effective-winner 조건에서 /Script native WheelClass도 별도 canonical branch로 roundtrip되는지 검증합니다.
	Request.InitialProfilePayload.DrivetrainData.FrontWheelClass = Fixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass;
	Request.InitialProfilePayload.DrivetrainData.RearWheelClass = Fixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass;
	FCFBuilderCompanionPreview NativeClassPreview;
	if (!TestTrue(TEXT("0/4 native WheelClass companion prospective preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderCompanions(Request, NativeClassPreview)))
	{
		AddError(NativeClassPreview.Operation.Message);
		return false;
	}
	TestFalse(TEXT("native WheelClass preview performs no Asset creation"), NativeClassPreview.Operation.Mutation.bCreatedAssets);
	TestFalse(TEXT("native WheelClass preview performs no Target mutation"), NativeClassPreview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("native WheelClass preview performs no Save"), NativeClassPreview.Operation.Mutation.bSavePerformed);
	TestFalse(TEXT("native WheelClass prospective hash is present"), NativeClassPreview.ProspectiveResolvedDefinitionHash.IsEmpty());
	return true;
}

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
	Request.bHasInitialEvidencePayload = true;
	Request.NewEvidenceId = FGuid::NewGuid();
	Request.InitialEvidencePayload = BuildInitialEvidencePayload();
	Request.bHasInitialProfilePayload = true;
	Request.InitialProfilePayload.VehicleBaseData = Fixture.VehicleBase->Data;
	Request.InitialProfilePayload.DrivetrainData = Fixture.Drivetrain->Data;
	Request.InitialProfilePayload.HandlingData = Fixture.Handling->Data;
	Request.InitialProfilePayload.PerformanceData = FCFPerformanceProfileData();
	Request.EvidenceAsset = MakeAssetIdentity(TEXT("CFVBEvidence"), TEXT("DA_VB_Evidence_New"));
	Request.PerformanceAsset = MakeAssetIdentity(TEXT("CFVBPerformance"), TEXT("DA_VB_Performance_New"));
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;

	// Initial Research payload 없이 empty Evidence를 만들려는 mutation0 request입니다.
	FCFBuilderCompanionRequest EmptyEvidenceRequest = Request;
	EmptyEvidenceRequest.bHasInitialEvidencePayload = false;
	// Empty Evidence preview result입니다.
	FCFBuilderCompanionPreview EmptyEvidencePreview;
	TestFalse(TEXT("VB-P0-09 empty initial Evidence payload is rejected"), FCFVehicleAuthoringService::PreviewBuilderCompanions(EmptyEvidenceRequest, EmptyEvidencePreview));
	TestFalse(TEXT("VB-P0-09 empty Evidence rejection performs no mutation"), EmptyEvidencePreview.Operation.Mutation.bCreatedAssets);

	// Initial Research 단계에서 GAME_BIAS를 넣은 mutation0 request입니다.
	FCFBuilderCompanionRequest GameBiasRequest = Request;
	GameBiasRequest.InitialEvidencePayload.Claims[0].Provenance = ECFRefProvenance::GAME_BIAS;
	// GAME_BIAS Research preview result입니다.
	FCFBuilderCompanionPreview GameBiasPreview;
	TestFalse(TEXT("VB-P0-09 initial Research GAME_BIAS is rejected"), FCFVehicleAuthoringService::PreviewBuilderCompanions(GameBiasRequest, GameBiasPreview));
	TestFalse(TEXT("VB-P0-09 GAME_BIAS rejection performs no mutation"), GameBiasPreview.Operation.Mutation.bCreatedAssets);

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
	TestFalse(TEXT("VB-P0-09 prospective Evidence fingerprint is not empty"), Preview.ProspectiveEvidenceFingerprint.IsEmpty());
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
	TestEqual(TEXT("VB-P0-09 committed Evidence keeps approved fixed EvidenceId"), CommitResult.CreatedEvidence->EvidenceId, Request.NewEvidenceId);
	TestEqual(TEXT("VB-P0-09 committed Evidence fingerprint matches approved preview"), CommitResult.CreatedEvidence->EvidenceFingerprint, FreshPreview.ProspectiveEvidenceFingerprint);
	TestEqual(TEXT("VB-P0-09 committed Evidence keeps initial canonical claim"), CommitResult.CreatedEvidence->Claims.Num(), 1);
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
	ForeignOwnerRequest.bHasInitialEvidencePayload = false;
	ForeignOwnerRequest.NewEvidenceId.Invalidate();
	// Foreign owner preview result입니다.
	FCFBuilderCompanionPreview ForeignOwnerPreview;
	TestFalse(TEXT("VB-P0-05 foreign-owner companion is rejected"), FCFVehicleAuthoringService::PreviewBuilderCompanions(ForeignOwnerRequest, ForeignOwnerPreview));
	CommitResult.CreatedPerformance->Meta.OwnerRecipeId = PerformanceOwnerBefore;
	return true;
}

// Actual Wagon persisted truth와 Saved ResearchDraft를 Existing Evidence Refresh mutation0 preview로 검증하고 prospective fingerprint를 Saved diagnostic JSON으로 남깁니다.
bool FCFVehicleBuilderWagonDraftTest::RunTest(const FString& Parameters)
{
	// Actual Wagon managed Recipe exact object path입니다.
	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");
	// Actual Wagon persistent Reference Evidence exact object path입니다.
	const TCHAR* WagonEvidencePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_Ref_Wagon.DA_Ref_Wagon");

	// Disk-saved actual Wagon Recipe입니다.
	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipePath);
	// Disk-saved actual Wagon Reference Evidence입니다.
	UCFVehicleRefEvidence* Evidence = LoadObject<UCFVehicleRefEvidence>(nullptr, WagonEvidencePath);
	if (!TestNotNull(TEXT("Actual Wagon Recipe loads"), Recipe)
		|| !TestNotNull(TEXT("Actual Wagon Evidence loads"), Evidence))
	{
		return false;
	}

	// Recipe가 소유하는 disk-saved Target VehicleData입니다.
	UCFVehicleData* Target = Recipe->TargetVehicleData.LoadSynchronous();
	if (!TestNotNull(TEXT("Actual Wagon Target loads"), Target))
	{
		return false;
	}

	// Canonical AI ResearchDraft path입니다.
	const FString ResearchDraftPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("ResearchDraft.json")));
	// UTF-8 ResearchDraft JSON 원문입니다.
	FString ResearchDraftJson;
	if (!TestTrue(TEXT("Actual Wagon ResearchDraft exists"), FFileHelper::LoadFileToString(ResearchDraftJson, *ResearchDraftPath)))
	{
		return false;
	}

	// Typed ResearchDraft schema로 deserialize한 actual proposal입니다.
	FCFBuilderResearchDraft ResearchDraft;
	if (!TestTrue(
		TEXT("Actual Wagon ResearchDraft deserializes"),
		FJsonObjectConverter::JsonObjectStringToUStruct(ResearchDraftJson, &ResearchDraft, 0, 0)))
	{
		return false;
	}
	TestEqual(TEXT("Actual Wagon ResearchDraft schema is supported"), ResearchDraft.SchemaRevision, 1);
	TestEqual(TEXT("Actual Wagon ResearchDraft RecipeId matches"), ResearchDraft.RecipeId, Recipe->RecipeId);
	TestEqual(TEXT("Actual Wagon ResearchDraft Target matches"), ResearchDraft.TargetDefinitionPath, FSoftObjectPath(Target));

	// Preview 전 actual Evidence fingerprint입니다.
	const FString EvidenceFingerprintBefore = Evidence->EvidenceFingerprint;
	// Preview 전 Recipe diagnostic revision입니다.
	const int32 RecipeRevisionBefore = Recipe->AuthoringRevision;
	// Preview 전 Evidence diagnostic revision입니다.
	const int32 EvidenceRevisionBefore = Evidence->AuthoringRevision;
	// Preview 전 Target full Definition hash입니다.
	FString SnapshotError;
	// Actual Target baseline snapshot입니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	if (!TestTrue(
		TEXT("Actual Wagon target snapshot builds before dry-run"),
		FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}

	// Existing Evidence refresh mutation0 request입니다.
	FCFBuilderEvidenceRefreshRequest Request;
	Request.Recipe = Recipe;
	Request.EvidencePath = FSoftObjectPath(Evidence);
	Request.ExpectedCurrentEvidenceFingerprint = Evidence->EvidenceFingerprint;
	Request.EvidencePayload = ResearchDraft.EvidencePayload;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	Request.CallContext.ClientOperationId = TEXT("VB-P0-09-Wagon-Evidence-DryRun");

	// Actual Wagon current→prospective Evidence mutation0 preview입니다.
	FCFBuilderEvidenceRefreshPreview Preview;
	if (!TestTrue(
		TEXT("Actual Wagon Evidence Refresh dry-run preview succeeds"),
		FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(Request, Preview)))
	{
		AddError(Preview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("Actual Wagon preview is R1"), Preview.Proposal.RiskClass, ECFAuthoringRiskClass::R1_AuthoringRecordWrite);
	TestEqual(TEXT("Actual Wagon preview requires AuthoringWrite"), Preview.Proposal.RequiredApprovalClass, ECFAuthoringApprovalClass::AuthoringWrite);
	TestFalse(TEXT("Actual Wagon prospective fingerprint is non-empty"), Preview.ProspectiveEvidenceFingerprint.IsEmpty());
	TestNotEqual(TEXT("Actual Wagon refreshed Research changes Evidence fingerprint"), Preview.ProspectiveEvidenceFingerprint, Preview.CurrentEvidenceFingerprint);
	TestFalse(TEXT("Actual Wagon dry-run performs no Evidence mutation"), Preview.Operation.Mutation.bEvidenceChanged);
	TestFalse(TEXT("Actual Wagon dry-run performs no Recipe mutation"), Preview.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Actual Wagon dry-run performs no Target mutation"), Preview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Actual Wagon dry-run performs no Save"), Preview.Operation.Mutation.bSavePerformed);
	TestEqual(TEXT("Actual Wagon persistent Evidence fingerprint remains unchanged"), Evidence->EvidenceFingerprint, EvidenceFingerprintBefore);
	TestEqual(TEXT("Actual Wagon Recipe revision remains unchanged"), Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("Actual Wagon Evidence revision remains unchanged"), Evidence->AuthoringRevision, EvidenceRevisionBefore);

	// Dry-run 뒤 actual Target snapshot입니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!TestTrue(
		TEXT("Actual Wagon target snapshot builds after dry-run"),
		FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}
	TestEqual(TEXT("Actual Wagon target hash remains unchanged"), TargetAfter.DefinitionHash, TargetBefore.DefinitionHash);

	// Canonical AI PhysicsDraft v2 path입니다.
	const FString PhysicsDraftPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("PhysicsDraft.json")));
	// UTF-8 PhysicsDraft JSON 원문입니다.
	FString PhysicsDraftJson;
	if (!TestTrue(TEXT("Actual Wagon PhysicsDraft v2 exists"), FFileHelper::LoadFileToString(PhysicsDraftJson, *PhysicsDraftPath)))
	{
		return false;
	}

	// Typed PhysicsDraft v2 proposal입니다.
	FCFBuilderPhysicsDraft PhysicsDraft;
	if (!TestTrue(
		TEXT("Actual Wagon PhysicsDraft v2 deserializes"),
		FJsonObjectConverter::JsonObjectStringToUStruct(PhysicsDraftJson, &PhysicsDraft, 0, 0)))
	{
		return false;
	}
	TestEqual(TEXT("Actual Wagon PhysicsDraft schema is v2"), PhysicsDraft.SchemaRevision, 2);
	TestEqual(TEXT("Actual Wagon PhysicsDraft RecipeId matches"), PhysicsDraft.RecipeId, Recipe->RecipeId);
	TestEqual(TEXT("Actual Wagon PhysicsDraft Target matches"), PhysicsDraft.TargetDefinitionPath, FSoftObjectPath(Target));
	TestEqual(TEXT("Actual Wagon PhysicsDraft EvidenceId matches persistent identity"), PhysicsDraft.EvidenceId, Evidence->EvidenceId);
	TestEqual(TEXT("Actual Wagon PhysicsDraft binds prospective Evidence fingerprint"), PhysicsDraft.ExpectedEvidenceFingerprint, Preview.ProspectiveEvidenceFingerprint);
	TestFalse(TEXT("Actual Wagon PhysicsDraft correlation hash is present"), PhysicsDraft.ProposalCorrelationHash.IsEmpty());
	TestEqual(TEXT("Actual Wagon PhysicsDraft expects eight forward gears"), PhysicsDraft.TransmissionReview.ExpectedForwardGearCount, 8);
	TestEqual(TEXT("Actual Wagon PhysicsDraft contains nine transmission review components"), PhysicsDraft.TransmissionReview.Components.Num(), 9);
	TestTrue(TEXT("Actual Wagon PhysicsDraft enables vehicle-specific transmission"), PhysicsDraft.ProfilePayload.DrivetrainData.bUseTransmissionConfig);
	TestEqual(TEXT("Actual Wagon PhysicsDraft has eight forward ratios"), PhysicsDraft.ProfilePayload.DrivetrainData.TransmissionRatios.ForwardGearRatios.Num(), 8);
	TestEqual(TEXT("Actual Wagon PhysicsDraft has one reverse ratio"), PhysicsDraft.ProfilePayload.DrivetrainData.TransmissionRatios.ReverseGearRatios.Num(), 1);

	// Existing Wagon identity/binding을 보존하면서 새 Research payload만 가진 transient prospective Evidence입니다.
	UCFVehicleRefEvidence* ProspectiveEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Actual Wagon transient prospective Evidence exists"), ProspectiveEvidence))
	{
		return false;
	}
	ProspectiveEvidence->EvidenceId = Evidence->EvidenceId;
	ProspectiveEvidence->SchemaRevision = Evidence->SchemaRevision;
	ProspectiveEvidence->NormalizationPolicyRevision = Evidence->NormalizationPolicyRevision;
	ProspectiveEvidence->TargetRecipeId = Evidence->TargetRecipeId;
	ProspectiveEvidence->TargetRecipePath = Evidence->TargetRecipePath;
	ProspectiveEvidence->TargetDefinitionPath = Evidence->TargetDefinitionPath;
	// Transient clone에는 preview와 같은 complete Research replacement payload만 적용합니다.
	FString ProspectiveEvidenceError;
	if (!TestTrue(
		TEXT("Actual Wagon transient prospective Evidence payload applies"),
		ProspectiveEvidence->ApplyInitialResearchPayload(ResearchDraft.EvidencePayload, ProspectiveEvidenceError)))
	{
		AddError(ProspectiveEvidenceError);
		return false;
	}
	TestEqual(TEXT("Actual Wagon transient prospective fingerprint matches R1 preview"), ProspectiveEvidence->EvidenceFingerprint, Preview.ProspectiveEvidenceFingerprint);

	// 실제 Step 5 facade가 요구하는 모든 consumed Claim이 prospective Evidence에서 Canonical인지 검증합니다.
	TSet<FName> SeenConsumedClaimIds;
	for (const FName ClaimId : PhysicsDraft.ConsumedClaimIds)
	{
		if (!TestFalse(TEXT("Actual Wagon PhysicsDraft consumed Claim is non-None"), ClaimId.IsNone())
			|| !TestFalse(TEXT("Actual Wagon PhysicsDraft consumed Claim is unique"), SeenConsumedClaimIds.Contains(ClaimId)))
		{
			return false;
		}
		SeenConsumedClaimIds.Add(ClaimId);
		// Prospective Evidence에서 exact Claim을 찾습니다.
		const FCFRefClaim* Claim = ProspectiveEvidence->Claims.FindByPredicate([ClaimId](const FCFRefClaim& Candidate)
		{
			return Candidate.ClaimId == ClaimId;
		});
		if (!TestNotNull(*FString::Printf(TEXT("Actual Wagon consumed Claim exists: %s"), *ClaimId.ToString()), Claim)
			|| !TestEqual(*FString::Printf(TEXT("Actual Wagon consumed Claim is Canonical: %s"), *ClaimId.ToString()), Claim->ResolutionState, ECFRefClaimResolution::Canonical))
		{
			return false;
		}
	}

	// Future one-time policy migration 후 적용될 exact VehicleSpecificRequired Transmission contract를 current Product Asset mutation 없이 검증합니다.
	FString TransmissionReviewError;
	if (!TestTrue(
		TEXT("Actual Wagon PhysicsDraft v2 passes VehicleSpecificRequired TransmissionReview"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			PhysicsDraft.TransmissionReview,
			PhysicsDraft.ProfilePayload.DrivetrainData,
			*ProspectiveEvidence,
			PhysicsDraft.ConsumedClaimIds,
			TransmissionReviewError)))
	{
		AddError(TransmissionReviewError);
		return false;
	}

	// Prospective Drivetrain payload + review를 binding하는 deterministic Transmission proposal hash입니다.
	const FString TransmissionProposalHash = CFBuilderTransUtil::BuildTransmissionProposalHash(
		PhysicsDraft.TransmissionReview,
		PhysicsDraft.ProfilePayload.DrivetrainData);
	TestFalse(TEXT("Actual Wagon Transmission proposal hash is non-empty"), TransmissionProposalHash.IsEmpty());

	// Current WSA radius / EngineMaxRPM authority를 read-only로 얻는 actual Wagon resolve request입니다.
	FCFVehicleAuthoringReadRequest ResolveRequest;
	ResolveRequest.Recipe = Recipe;
	ResolveRequest.TargetVehicleData = Target;
	ResolveRequest.CallerKind = ECFAuthoringCallerKind::Automation;
	// Current persisted Wagon resolve 결과입니다. Proposed transmission diagnostic은 여기서 Wheel Radius와 EngineMaxRPM만 소비합니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!TestTrue(TEXT("Actual Wagon current resolve for transmission diagnostic succeeds"), FCFVehicleAuthoringService::ResolveVehiclePreview(ResolveRequest, ResolveRead)))
	{
		AddError(ResolveRead.Operation.Message);
		return false;
	}

	// Radius authority를 추적할 complete current Resolver evidence JSON입니다.
	FString ResolveResultJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString(ResolveRead.ResolveResult, ResolveResultJson))
	{
		AddError(TEXT("Actual Wagon ResolveResult를 JSON으로 serialize할 수 없습니다."));
		return false;
	}
	// Product Asset mutation 없이 Saved에만 남기는 current Resolver trace 경로입니다.
	const FString ResolveResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("WagonResolvePreview.json")));
	if (!FFileHelper::SaveStringToFile(
		ResolveResultJson,
		*ResolveResultPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		AddError(FString::Printf(TEXT("Wagon Resolve Preview result를 쓸 수 없습니다: %s"), *ResolveResultPath));
		return false;
	}

	// Future VehicleSpecificRequired proposal을 current WSA-owned radius에 대입한 no-slip fixed-shift diagnostic입니다.
	FCFBuilderTransmissionDiagnostic TransmissionDiagnostic;
	CFBuilderTransUtil::BuildTransmissionDiagnostic(
		ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
		PhysicsDraft.TransmissionReview,
		TransmissionProposalHash,
		PhysicsDraft.ProfilePayload.DrivetrainData,
		PhysicsDraft.ProfilePayload.PerformanceData,
		ResolveRead.ResolveResult,
		TransmissionDiagnostic);
	TestTrue(TEXT("Actual Wagon transmission diagnostic evaluated"), TransmissionDiagnostic.bEvaluated);
	TestTrue(TEXT("Actual Wagon transmission diagnostic is vehicle-specific"), TransmissionDiagnostic.bVehicleSpecificRequired);
	TestEqual(TEXT("Actual Wagon transmission diagnostic has eight gear rows"), TransmissionDiagnostic.Gears.Num(), 8);
	// Current Resolver authority가 정상일 때만 WSA radius 기반 speed diagnostic을 제공해야 합니다.
	const bool bResolveAuthorityAvailable = ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success;
	if (bResolveAuthorityAvailable)
	{
		TestEqual(TEXT("Actual Wagon successful Resolver yields zero transmission authority blockers"), TransmissionDiagnostic.Blockers.Num(), 0);
	}
	else
	{
		TestTrue(TEXT("Actual Wagon blocked Resolver yields WheelRadius authority blocker"), TransmissionDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
		{
			return Blocker.Contains(TEXT("Transmission.WheelRadiusAuthorityUnavailable"));
		}));
	}
	if (TransmissionDiagnostic.Gears.Num() == 8)
	{
		TestEqual(TEXT("Actual Wagon first-gear ShiftSpeed availability follows Resolver authority"), TransmissionDiagnostic.Gears[0].bShiftSpeedAvailable, bResolveAuthorityAvailable);
		if (bResolveAuthorityAvailable)
		{
			TestTrue(TEXT("Actual Wagon first-gear vehicle-specific ChangeUp speed is below 80kmh"), TransmissionDiagnostic.Gears[0].ShiftSpeedMaxKmh < 80.0f);
		}
		TestTrue(TEXT("Actual Wagon 1->2 post-shift RPM remains available independent of WSA radius"), TransmissionDiagnostic.Gears[0].bPostShiftAvailable);
		TestTrue(TEXT("Actual Wagon 1->2 post-shift RPM remains above ChangeDownRPM"), TransmissionDiagnostic.Gears[0].PostShiftRPM > PhysicsDraft.ProfilePayload.DrivetrainData.ChangeDownRPM);
	}

	// Human/AI follow-up가 exact prospective fingerprint를 안정적으로 읽을 Saved diagnostic result path입니다.
	const FString ResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("WagonEvidencePreview.json")));
	// Result parent directory입니다.
	const FString ResultDirectory = FPaths::GetPath(ResultPath);
	if (!IFileManager::Get().MakeDirectory(*ResultDirectory, true))
	{
		AddError(FString::Printf(TEXT("Wagon Evidence Preview result directory를 만들 수 없습니다: %s"), *ResultDirectory));
		return false;
	}

	// Actual dry-run 결과를 Product Asset이 아닌 Saved diagnostic JSON으로 기록합니다.
	const FString ResultJson = FString::Printf(
		TEXT("{\n")
		TEXT("  \"CurrentEvidenceFingerprint\": \"%s\",\n")
		TEXT("  \"ProspectiveEvidenceFingerprint\": \"%s\",\n")
		TEXT("  \"CurrentClaimCount\": %d,\n")
		TEXT("  \"ProspectiveClaimCount\": %d,\n")
		TEXT("  \"CurrentUnknownFactCount\": %d,\n")
		TEXT("  \"ProspectiveUnknownFactCount\": %d,\n")
		TEXT("  \"BuilderTransmissionPolicy\": \"%s\",\n")
		TEXT("  \"ProductAssetMutation\": false,\n")
		TEXT("  \"SavePerformed\": false\n")
		TEXT("}\n"),
		*Preview.CurrentEvidenceFingerprint,
		*Preview.ProspectiveEvidenceFingerprint,
		Preview.CurrentClaimCount,
		Preview.ProspectiveClaimCount,
		Preview.CurrentUnknownFactCount,
		Preview.ProspectiveUnknownFactCount,
		Recipe->BuilderTransmissionPolicy == ECFBuilderTransmissionPolicy::VehicleSpecificRequired
			? TEXT("VehicleSpecificRequired")
			: TEXT("LegacyCompatible"));
	if (!FFileHelper::SaveStringToFile(
		ResultJson,
		*ResultPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		AddError(FString::Printf(TEXT("Wagon Evidence Preview result를 쓸 수 없습니다: %s"), *ResultPath));
		return false;
	}

	// Human/AI가 Builder UI 진입 전 actual Transmission proposal hash와 gear diagnostics를 읽을 Saved JSON입니다.
	const FString PhysicsResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CarFight"),
		TEXT("VehicleBuilder"),
		TEXT("WagonPhysicsPreview.json")));
	// USTRUCT diagnostic의 machine-readable JSON입니다.
	FString TransmissionDiagnosticJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString(TransmissionDiagnostic, TransmissionDiagnosticJson))
	{
		AddError(TEXT("Actual Wagon TransmissionDiagnostic을 JSON으로 serialize할 수 없습니다."));
		return false;
	}
	// Product Asset이 아닌 Saved dry-run diagnostic result입니다.
	const FString PhysicsResultJson = FString::Printf(
		TEXT("{\n")
		TEXT("  \"ExpectedEvidenceFingerprint\": \"%s\",\n")
		TEXT("  \"ProposalCorrelationHash\": \"%s\",\n")
		TEXT("  \"TransmissionProposalHash\": \"%s\",\n")
		TEXT("  \"Diagnostic\": %s,\n")
		TEXT("  \"ProductAssetMutation\": false,\n")
		TEXT("  \"SavePerformed\": false\n")
		TEXT("}\n"),
		*PhysicsDraft.ExpectedEvidenceFingerprint,
		*PhysicsDraft.ProposalCorrelationHash,
		*TransmissionProposalHash,
		*TransmissionDiagnosticJson);
	if (!FFileHelper::SaveStringToFile(
		PhysicsResultJson,
		*PhysicsResultPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		AddError(FString::Printf(TEXT("Wagon Physics Preview result를 쓸 수 없습니다: %s"), *PhysicsResultPath));
		return false;
	}

	AddInfo(FString::Printf(
		TEXT("WAGON_EVIDENCE_PROSPECTIVE_FINGERPRINT=%s"),
		*Preview.ProspectiveEvidenceFingerprint));
	AddInfo(FString::Printf(
		TEXT("WAGON_TRANSMISSION_PROPOSAL_HASH=%s"),
		*TransmissionProposalHash));
	return true;
}

// Existing Reference Evidence complete replacement의 reviewed R1 contract를 차종/실제 Product Asset 없이 검증합니다.
bool FCFVehicleBuilderEvidenceRefreshTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderTestsPrivate;

	// Managed Recipe/Target ownership fixture입니다.
	FBuilderFixture Fixture;
	// Fixture/semantic validation diagnostic입니다.
	FString Error;
	if (!BuildManagedFixture(Fixture, Error))
	{
		AddError(Error);
		return false;
	}

	// Current existing Reference Evidence fixture입니다.
	Fixture.Evidence = NewObject<UCFVehicleRefEvidence>(Fixture.RecipePackage, TEXT("DA_VB_RefreshEvidence"), RF_Transactional);
	if (!TestNotNull(TEXT("Evidence Refresh fixture exists"), Fixture.Evidence))
	{
		return false;
	}
	Fixture.Evidence->TargetRecipeId = Fixture.Recipe->RecipeId;
	Fixture.Evidence->TargetRecipePath = FSoftObjectPath(Fixture.Recipe);
	Fixture.Evidence->TargetDefinitionPath = FSoftObjectPath(Fixture.TargetVehicleData);

	// Current persisted-like research payload입니다.
	const FCFVehicleRefEvidencePayload InitialPayload = BuildInitialEvidencePayload();
	if (!TestTrue(TEXT("Evidence Refresh initial payload applies"), Fixture.Evidence->ApplyInitialResearchPayload(InitialPayload, Error)))
	{
		AddError(Error);
		return false;
	}

	// Refresh 전 exact Evidence identity입니다.
	const FGuid EvidenceIdBefore = Fixture.Evidence->EvidenceId;
	// Refresh 전 exact Recipe binding입니다.
	const FGuid TargetRecipeIdBefore = Fixture.Evidence->TargetRecipeId;
	// Refresh 전 exact Recipe path binding입니다.
	const FSoftObjectPath TargetRecipePathBefore = Fixture.Evidence->TargetRecipePath;
	// Refresh 전 exact Target path binding입니다.
	const FSoftObjectPath TargetDefinitionPathBefore = Fixture.Evidence->TargetDefinitionPath;
	// Refresh 전 semantic fingerprint입니다.
	const FString EvidenceFingerprintBefore = Fixture.Evidence->EvidenceFingerprint;
	// Refresh 전 diagnostic revision입니다.
	const int32 AuthoringRevisionBefore = Fixture.Evidence->AuthoringRevision;

	// Refresh가 Recipe/Target semantic state를 바꾸지 않는지 비교할 baseline snapshot입니다.
	FCFVehicleRecipeSnapshot RecipeBefore;
	// Refresh가 Target Definition을 바꾸지 않는지 비교할 baseline snapshot입니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Fixture.Recipe, RecipeBefore, Error)
		|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, TargetBefore, Error))
	{
		AddError(Error);
		return false;
	}

	// 기존 payload 전체를 보존하면서 새 canonical FACT 하나를 추가한 complete replacement입니다.
	FCFVehicleRefEvidencePayload ReplacementPayload = InitialPayload;
	// Later research에서 새로 확보한 generic engine power FACT입니다.
	FCFRefClaim& PowerClaim = ReplacementPayload.Claims.AddDefaulted_GetRef();
	PowerClaim.ClaimId = TEXT("CLAIM-VB-POWER");
	PowerClaim.ReferenceVehicleId = ReplacementPayload.ReferenceVehicles[0].ReferenceVehicleId;
	PowerClaim.FactKey = TEXT("Engine.MaxPower");
	PowerClaim.ValueKind = ECFRefValueKind::Number;
	PowerClaim.NumberValue = 180.0;
	PowerClaim.UnitId = TEXT("kW");
	PowerClaim.SourceValueText = TEXT("180 kW");
	PowerClaim.Provenance = ECFRefProvenance::FACT;
	PowerClaim.CitationIds = {ReplacementPayload.Sources[0].SourceId};
	PowerClaim.ConfidenceScore = 0.95f;
	PowerClaim.ResolutionState = ECFRefClaimResolution::Canonical;
	ReplacementPayload.ResearchNotes = TEXT("Automation Evidence Refresh replacement payload.");

	// Existing Evidence refresh mutation0 request입니다.
	FCFBuilderEvidenceRefreshRequest Request;
	Request.Recipe = Fixture.Recipe;
	Request.EvidencePath = FSoftObjectPath(Fixture.Evidence);
	Request.ExpectedCurrentEvidenceFingerprint = EvidenceFingerprintBefore;
	Request.EvidencePayload = ReplacementPayload;
	Request.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	Request.CallContext.ClientOperationId = TEXT("VB-P0-05-EvidenceRefresh-Preview");

	// USER approval 전에 볼 exact mutation0 preview입니다.
	FCFBuilderEvidenceRefreshPreview Preview;
	if (!TestTrue(TEXT("Builder Evidence Refresh preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(Request, Preview)))
	{
		AddError(Preview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("Evidence Refresh preview sees one current claim"), Preview.CurrentClaimCount, 1);
	TestEqual(TEXT("Evidence Refresh preview sees two prospective claims"), Preview.ProspectiveClaimCount, 2);
	TestNotEqual(TEXT("Evidence Refresh prospective fingerprint changes"), Preview.ProspectiveEvidenceFingerprint, EvidenceFingerprintBefore);
	TestFalse(TEXT("Evidence Refresh preview performs no Evidence mutation"), Preview.Operation.Mutation.bEvidenceChanged);
	TestFalse(TEXT("Evidence Refresh preview performs no Recipe mutation"), Preview.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Evidence Refresh preview performs no Target mutation"), Preview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Evidence Refresh preview performs no Save"), Preview.Operation.Mutation.bSavePerformed);
	TestEqual(TEXT("Evidence Refresh preview leaves persistent fingerprint unchanged"), Fixture.Evidence->EvidenceFingerprint, EvidenceFingerprintBefore);

	// Explicit AuthoringWrite approval 없이 terminal commit할 수 없습니다.
	FCFBuilderEvidenceRefreshResult UnapprovedResult;
	TestFalse(TEXT("Evidence Refresh commit requires explicit approval"), FCFVehicleAuthoringService::CommitBuilderEvidenceRefresh(Request, Preview, UnapprovedResult));
	TestEqual(TEXT("Unapproved Evidence Refresh is ApprovalRequired"), UnapprovedResult.Operation.ErrorCode, ECFAuthoringErrorCode::ApprovalRequired);
	TestEqual(TEXT("Unapproved Evidence Refresh leaves fingerprint unchanged"), Fixture.Evidence->EvidenceFingerprint, EvidenceFingerprintBefore);

	// Preview가 요구하는 exact AuthoringWrite approval/current authority를 request에 binding합니다.
	Request.CallContext.ClientOperationId = TEXT("VB-P0-05-EvidenceRefresh-Commit");
	Request.CallContext.ApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	Request.CallContext.ApprovalScopeHash = Preview.Proposal.ProposalHash;
	Request.CallContext.ExpectedRecipeFingerprint = Preview.Proposal.ExpectedRecipeFingerprint;
	Request.CallContext.ExpectedTargetDefinitionHash = Preview.Proposal.ExpectedTargetDefinitionHash;
	Request.CallContext.ExpectedResolverContractRevision = Preview.Proposal.ResolverContractRevision;

	// Approved complete replacement terminal result입니다.
	FCFBuilderEvidenceRefreshResult CommitResult;
	if (!TestTrue(TEXT("Approved Evidence Refresh commit succeeds"), FCFVehicleAuthoringService::CommitBuilderEvidenceRefresh(Request, Preview, CommitResult)))
	{
		AddError(CommitResult.Operation.Message);
		return false;
	}
	TestTrue(TEXT("Evidence Refresh marks Evidence mutation"), CommitResult.Operation.Mutation.bEvidenceChanged);
	TestFalse(TEXT("Evidence Refresh does not mutate Recipe"), CommitResult.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("Evidence Refresh does not mutate Target"), CommitResult.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("Evidence Refresh does not mutate Profiles"), CommitResult.Operation.Mutation.bProfileChanged);
	TestFalse(TEXT("Evidence Refresh does not create Assets"), CommitResult.Operation.Mutation.bCreatedAssets);
	TestFalse(TEXT("Evidence Refresh does not Save"), CommitResult.Operation.Mutation.bSavePerformed);
	TestEqual(TEXT("Evidence Refresh preserves EvidenceId"), Fixture.Evidence->EvidenceId, EvidenceIdBefore);
	TestEqual(TEXT("Evidence Refresh preserves TargetRecipeId"), Fixture.Evidence->TargetRecipeId, TargetRecipeIdBefore);
	TestEqual(TEXT("Evidence Refresh preserves Recipe path binding"), Fixture.Evidence->TargetRecipePath, TargetRecipePathBefore);
	TestEqual(TEXT("Evidence Refresh preserves Target path binding"), Fixture.Evidence->TargetDefinitionPath, TargetDefinitionPathBefore);
	TestEqual(TEXT("Evidence Refresh commits prospective fingerprint"), Fixture.Evidence->EvidenceFingerprint, Preview.ProspectiveEvidenceFingerprint);
	TestEqual(TEXT("Evidence Refresh terminal fingerprint matches persistent state"), CommitResult.EvidenceFingerprint, Fixture.Evidence->EvidenceFingerprint);
	TestEqual(TEXT("Evidence Refresh increments diagnostic revision once"), Fixture.Evidence->AuthoringRevision, AuthoringRevisionBefore + 1);
	TestEqual(TEXT("Evidence Refresh commits complete replacement claims"), Fixture.Evidence->Claims.Num(), 2);

	// Recipe/Target semantic snapshots must remain byte-semantic equivalent after Evidence-only write.
	FCFVehicleRecipeSnapshot RecipeAfter;
	// Evidence-only write 뒤 Target snapshot입니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Fixture.Recipe, RecipeAfter, Error)
		|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Fixture.TargetVehicleData, TargetAfter, Error))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Evidence Refresh preserves Recipe fingerprint"), RecipeAfter.RecipeFingerprint, RecipeBefore.RecipeFingerprint);
	TestEqual(TEXT("Evidence Refresh preserves Target Definition"), TargetAfter.DefinitionHash, TargetBefore.DefinitionHash);

	// Old fingerprint를 baseline으로 재사용하면 stale replay가 fail-closed해야 합니다.
	FCFBuilderEvidenceRefreshPreview StalePreview;
	TestFalse(TEXT("Evidence Refresh stale old fingerprint is rejected"), FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(Request, StalePreview));
	TestEqual(TEXT("Stale Evidence Refresh reports StateChanged"), StalePreview.Operation.ErrorCode, ECFAuthoringErrorCode::StateChanged);

	// Fresh fingerprint + same complete replacement은 idempotent NoChange여야 합니다.
	FCFBuilderEvidenceRefreshRequest SameRequest = Request;
	SameRequest.ExpectedCurrentEvidenceFingerprint = Fixture.Evidence->EvidenceFingerprint;
	SameRequest.CallContext = FCFAuthoringCallContext();
	SameRequest.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	SameRequest.CallContext.ClientOperationId = TEXT("VB-P0-05-EvidenceRefresh-NoChange");
	FCFBuilderEvidenceRefreshPreview SamePreview;
	TestTrue(TEXT("Same Evidence Refresh preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(SameRequest, SamePreview));
	TestEqual(TEXT("Same Evidence Refresh is NoChange"), SamePreview.Operation.Status, ECFAuthoringOpStatus::NoChange);

	// Persistent Research normalization은 GAME_BIAS Claim을 계속 거부해야 합니다.
	FCFBuilderEvidenceRefreshRequest GameBiasRequest = SameRequest;
	GameBiasRequest.EvidencePayload.Claims[0].Provenance = ECFRefProvenance::GAME_BIAS;
	GameBiasRequest.CallContext.ClientOperationId = TEXT("VB-P0-05-EvidenceRefresh-GameBiasReject");
	FCFBuilderEvidenceRefreshPreview GameBiasPreview;
	TestFalse(TEXT("Evidence Refresh rejects GAME_BIAS research claim"), FCFVehicleAuthoringService::PreviewBuilderEvidenceRefresh(GameBiasRequest, GameBiasPreview));
	TestEqual(TEXT("GAME_BIAS Evidence Refresh is ValidationBlocked"), GameBiasPreview.Operation.ErrorCode, ECFAuthoringErrorCode::ValidationBlocked);

	return true;
}

// 차종/실제 Asset과 무관하게 ESH-02 Engine Curve provenance/hash/receipt 계약 전체를 검증합니다.
bool FCFVehicleBuilderEngineCurveTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderTestsPrivate;
	(void)Parameters;

	// 기존 차량의 Curve opt-out + BaselineInherited compatibility를 검증할 최소 Evidence입니다.
	UCFVehicleRefEvidence* BaselineEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("ESH-02 baseline Evidence fixture exists"), BaselineEvidence))
	{
		return false;
	}
	FCFPerformanceProfileData BaselinePayload;
	FCFBuilderEngineCurveReview BaselineReview;
	FString Error;
	TestTrue(
		TEXT("ESH-02 legacy/baseline curve opt-out remains valid"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(BaselineReview, BaselinePayload, *BaselineEvidence, {}, Error));
	TestTrue(TEXT("Baseline Engine Curve hash stays empty"), CFBuilderEngineUtil::BuildEngineCurveProposalHash(BaselineReview, BaselinePayload).IsEmpty());
	FCFBuilderEngineCurveDiagnostic BaselineDiagnostic;
	CFBuilderEngineUtil::BuildEngineCurveDiagnostic(BaselineReview, FString(), BaselinePayload, BaselineDiagnostic);
	TestTrue(TEXT("Baseline Engine Curve emits fidelity partial warning"), BaselineDiagnostic.Warnings.ContainsByPredicate([](const FString& Warning)
	{
		return Warning.Contains(TEXT("Performance.EngineCurveFidelityPartial"));
	}));
	TestEqual(TEXT("Baseline Engine Curve does not block legacy vehicle"), BaselineDiagnostic.Blockers.Num(), 0);

	// Full Preview/Commit/receipt contract까지 검증할 managed generic fixture입니다.
	FBuilderFixture Fixture;
	if (!BuildManagedFixture(Fixture, Error)
		|| !AttachPrivateProfiles(Fixture, true, Error)
		|| !AttachEvidence(Fixture, Error))
	{
		AddError(Error);
		return false;
	}

	// Sparse Engine Curve proposal의 real-data-like FACT anchors입니다.
	const FName TorqueClaimId(TEXT("ENGINE_MAX_TORQUE"));
	const FName TorqueBandClaimId(TEXT("ENGINE_TORQUE_BAND"));
	const FName PowerClaimId(TEXT("ENGINE_MAX_POWER"));
	const FName PowerBandClaimId(TEXT("ENGINE_POWER_BAND"));
	AddEngineFact(*Fixture.Evidence, TorqueClaimId, CFBuilderEngineUtil::MaxTorqueFactKey, 350.0);
	AddEngineFact(*Fixture.Evidence, TorqueBandClaimId, CFBuilderEngineUtil::TorqueBandFactKey, 1800.0, 4800.0);
	AddEngineFact(*Fixture.Evidence, PowerClaimId, CFBuilderEngineUtil::MaxPowerFactKey, 184.0);
	AddEngineFact(*Fixture.Evidence, PowerBandClaimId, CFBuilderEngineUtil::PowerBandFactKey, 5400.0, 5700.0);
	if (!Fixture.Evidence->RefreshEvidenceFingerprint(Error))
	{
		AddError(Error);
		return false;
	}

	// GAME_BIAS curve가 실제 소비하는 canonical Engine FACT 목록입니다.
	const TArray<FName> EngineClaimIds = {TorqueClaimId, TorqueBandClaimId, PowerClaimId, PowerBandClaimId};
	// Curve value owner인 complete Performance payload입니다.
	const FCFPerformanceProfileData EnginePayload = BuildGenericEngineCurvePayload();
	// Sparse FACT + explicit low/tail approximation review입니다.
	const FCFBuilderEngineCurveReview GameBiasReview = BuildGenericEngineCurveReview(EngineClaimIds);
	// Profile proposal 전체 consumed Claim set입니다.
	TArray<FName> ConsumedClaimIds = {TEXT("Claim_VB_Mass")};
	ConsumedClaimIds.Append(EngineClaimIds);

	TestTrue(
		TEXT("ESH-02 GAME_BIAS sparse-anchor Engine Curve validates"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(GameBiasReview, EnginePayload, *Fixture.Evidence, ConsumedClaimIds, Error));
	const FString GameBiasHash = CFBuilderEngineUtil::BuildEngineCurveProposalHash(GameBiasReview, EnginePayload);
	TestFalse(TEXT("ESH-02 vehicle-specific Engine Curve hash is populated"), GameBiasHash.IsEmpty());

	FCFBuilderEngineCurveDiagnostic GameBiasDiagnostic;
	CFBuilderEngineUtil::BuildEngineCurveDiagnostic(GameBiasReview, GameBiasHash, EnginePayload, GameBiasDiagnostic);
	TestTrue(TEXT("ESH-02 GAME_BIAS diagnostic is evaluated"), GameBiasDiagnostic.bEvaluated);
	TestTrue(TEXT("ESH-02 GAME_BIAS diagnostic sees vehicle-specific curve"), GameBiasDiagnostic.bVehicleSpecificCurveEnabled);
	TestEqual(TEXT("ESH-02 GAME_BIAS diagnostic has no blocker"), GameBiasDiagnostic.Blockers.Num(), 0);
	TestTrue(TEXT("ESH-02 GAME_BIAS diagnostic exposes USER review warning"), GameBiasDiagnostic.Warnings.ContainsByPredicate([](const FString& Warning)
	{
		return Warning.Contains(TEXT("Performance.EngineCurveGameBias"));
	}));

	// Claim/map physical order를 바꿔도 proposal hash는 동일해야 합니다.
	FCFBuilderEngineCurveReview ReorderedReview = GameBiasReview;
	Algo::Reverse(ReorderedReview.EvidenceClaimIds);
	ReorderedReview.MethodParameters.Reset();
	ReorderedReview.MethodParameters.Add(TEXT("Intent"), TEXT("Sparse FACT anchors plus explicit low/tail GAME_BIAS"));
	ReorderedReview.MethodParameters.Add(TEXT("Interpolation"), TEXT("Linear"));
	TestEqual(
		TEXT("ESH-02 Engine Curve hash ignores Claim/map physical order"),
		CFBuilderEngineUtil::BuildEngineCurveProposalHash(ReorderedReview, EnginePayload),
		GameBiasHash);

	// Curve point 하나의 semantic value가 달라지면 hash도 반드시 달라져야 합니다.
	FCFPerformanceProfileData ChangedCurvePayload = EnginePayload;
	ChangedCurvePayload.EngineTorqueCurve.Points.Last().TorqueMultiplier = 0.64f;
	TestNotEqual(
		TEXT("ESH-02 Engine Curve payload change changes proposal hash"),
		CFBuilderEngineUtil::BuildEngineCurveProposalHash(GameBiasReview, ChangedCurvePayload),
		GameBiasHash);

	// Enabled vehicle-specific curve를 BaselineInherited로 위장할 수 없습니다.
	FCFBuilderEngineCurveReview InvalidBaselineReview;
	TestFalse(
		TEXT("ESH-02 enabled curve rejects BaselineInherited review"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(InvalidBaselineReview, EnginePayload, *Fixture.Evidence, ConsumedClaimIds, Error));
	TestTrue(TEXT("Enabled baseline mismatch reports fidelity blocker"), Error.Contains(TEXT("Performance.EngineCurveFidelityPartial")));

	// Evidence에 존재해도 consumed set에서 빠진 Claim은 provenance로 사용할 수 없습니다.
	TArray<FName> MissingConsumedClaims = ConsumedClaimIds;
	MissingConsumedClaims.Remove(PowerBandClaimId);
	TestFalse(
		TEXT("ESH-02 unconsumed Engine Claim is blocked"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(GameBiasReview, EnginePayload, *Fixture.Evidence, MissingConsumedClaims, Error));
	TestTrue(TEXT("Unconsumed Engine Claim reports provenance blocker"), Error.Contains(TEXT("Performance.EngineCurveProvenanceUnbound")));

	// Engine Curve와 무관한 mass Claim을 Engine anchor로 사용할 수 없습니다.
	FCFBuilderEngineCurveReview WrongSemanticReview = GameBiasReview;
	WrongSemanticReview.EvidenceClaimIds = {TEXT("Claim_VB_Mass")};
	TestFalse(
		TEXT("ESH-02 unrelated FACT cannot support Engine Curve"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(WrongSemanticReview, EnginePayload, *Fixture.Evidence, ConsumedClaimIds, Error));
	TestTrue(TEXT("Unrelated Engine input reports semantic mismatch"), Error.Contains(TEXT("Performance.EngineCurveSemanticMismatch")));

	// DERIVED는 torque/power magnitude와 RPM/band anchor를 모두 가져야 합니다.
	FCFBuilderEngineCurveReview DerivedReview = GameBiasReview;
	DerivedReview.Disposition = ECFBuilderEngineCurveDisposition::EvidenceDerived;
	DerivedReview.MethodId = TEXT("SparseAnchorEngineCurveDerived");
	DerivedReview.MethodParameters.Reset();
	DerivedReview.MethodParameters.Add(TEXT("Interpolation"), TEXT("Linear"));
	TestTrue(
		TEXT("ESH-02 fully anchored deterministic DERIVED curve validates"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(DerivedReview, EnginePayload, *Fixture.Evidence, ConsumedClaimIds, Error));

	FCFBuilderEngineCurveReview UnderAnchoredDerivedReview = DerivedReview;
	UnderAnchoredDerivedReview.EvidenceClaimIds.Remove(PowerBandClaimId);
	TestFalse(
		TEXT("ESH-02 under-anchored DERIVED curve is blocked"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(UnderAnchoredDerivedReview, EnginePayload, *Fixture.Evidence, ConsumedClaimIds, Error));
	TestTrue(TEXT("Under-anchored DERIVED reports dedicated blocker"), Error.Contains(TEXT("Performance.EngineCurveAnchorInsufficient")));

	// Runtime curve 자체가 malformed면 provenance가 좋아도 통과할 수 없습니다.
	FCFPerformanceProfileData InvalidCurvePayload = EnginePayload;
	InvalidCurvePayload.EngineTorqueCurve.Points[2].EngineRPM = InvalidCurvePayload.EngineTorqueCurve.Points[1].EngineRPM;
	TestFalse(
		TEXT("ESH-02 duplicate-RPM Engine Curve is blocked"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(GameBiasReview, InvalidCurvePayload, *Fixture.Evidence, ConsumedClaimIds, Error));
	TestTrue(TEXT("Malformed Engine Curve reports invalid blocker"), Error.Contains(TEXT("Performance.EngineCurveInvalid")));

	// 실제 Builder Profile Preview/Commit lane이 Engine review/hash를 persistent receipt까지 전달하는지 검증합니다.
	FCFBuilderProfileCommitRequest CommitRequest = BuildProfileCommitRequest(Fixture);
	CommitRequest.Payload.PerformanceData = EnginePayload;
	CommitRequest.EvidenceBinding.ConsumedClaimIds = ConsumedClaimIds;
	CommitRequest.EngineCurveReview = GameBiasReview;
	CommitRequest.UpstreamBuilderProposalHash = TEXT("ESH-02-EngineCurve-Automation");

	FCFBuilderProfileCommitPreview CommitPreview;
	if (!TestTrue(TEXT("ESH-02 Engine Curve Profile preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderProfiles(CommitRequest, CommitPreview)))
	{
		AddError(CommitPreview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("ESH-02 preview Engine Curve hash matches utility"), CommitPreview.EngineCurveProposalHash, GameBiasHash);
	TestEqual(TEXT("ESH-02 preview Engine Curve blockers zero"), CommitPreview.EngineCurveDiagnostic.Blockers.Num(), 0);

	BindCommitApproval(CommitRequest, CommitPreview, TEXT("ESH-02-EngineCurve-Commit"));
	FCFAuthoringOpResult CommitResult;
	if (!TestTrue(TEXT("ESH-02 Engine Curve Profile commit succeeds"), FCFVehicleAuthoringService::CommitBuilderProfiles(CommitRequest, CommitPreview, CommitResult)))
	{
		AddError(CommitResult.Message);
		return false;
	}
	TestEqual(TEXT("ESH-02 receipt stores Engine Curve hash"), Fixture.Recipe->BuilderCommitReceipt.EngineCurveProposalHash, GameBiasHash);
	TestEqual(TEXT("ESH-02 receipt stores Engine Curve method"), Fixture.Recipe->BuilderCommitReceipt.EngineCurveReview.MethodId, GameBiasReview.MethodId);
	TestTrue(TEXT("ESH-02 Performance Profile committed curve opt-in"), Fixture.Performance->Data.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-02 Performance Profile committed curve point count"), Fixture.Performance->Data.EngineTorqueCurve.Points.Num(), EnginePayload.EngineTorqueCurve.Points.Num());

	// Editor restart resume와 동등하게 current payload + persistent receipt로 fresh NoChange를 재검증합니다.
	FCFBuilderProfileCommitRequest ResumeRequest = BuildProfileCommitRequest(Fixture);
	ResumeRequest.EvidenceBinding.ConsumedClaimIds = Fixture.Recipe->BuilderCommitReceipt.ConsumedClaimIds;
	ResumeRequest.EngineCurveReview = Fixture.Recipe->BuilderCommitReceipt.EngineCurveReview;
	ResumeRequest.UpstreamBuilderProposalHash = TEXT("ESH-02-EngineCurve-Resume");
	FCFBuilderProfileCommitPreview ResumePreview;
	TestTrue(TEXT("ESH-02 persistent Engine Curve receipt resumes"), FCFVehicleAuthoringService::PreviewBuilderProfiles(ResumeRequest, ResumePreview));
	TestEqual(TEXT("ESH-02 persistent receipt resume is NoChange"), ResumePreview.Operation.Status, ECFAuthoringOpStatus::NoChange);
	TestEqual(TEXT("ESH-02 resumed hash remains exact"), ResumePreview.EngineCurveProposalHash, Fixture.Recipe->BuilderCommitReceipt.EngineCurveProposalHash);

	// Transaction 밖 raw Curve drift는 old receipt와 동일한 NoChange로 숨지 않고 fresh receipt/update 필요 상태가 되어야 합니다.
	const float TailMultiplierBeforeDrift = Fixture.Performance->Data.EngineTorqueCurve.Points.Last().TorqueMultiplier;
	Fixture.Performance->Data.EngineTorqueCurve.Points.Last().TorqueMultiplier = TailMultiplierBeforeDrift - 0.05f;
	FCFBuilderProfileCommitRequest DriftedResumeRequest = BuildProfileCommitRequest(Fixture);
	DriftedResumeRequest.EvidenceBinding.ConsumedClaimIds = Fixture.Recipe->BuilderCommitReceipt.ConsumedClaimIds;
	DriftedResumeRequest.EngineCurveReview = Fixture.Recipe->BuilderCommitReceipt.EngineCurveReview;
	DriftedResumeRequest.UpstreamBuilderProposalHash = TEXT("ESH-02-EngineCurve-DriftedResume");
	FCFBuilderProfileCommitPreview DriftedResumePreview;
	TestTrue(TEXT("ESH-02 raw Curve drift produces fresh preview"), FCFVehicleAuthoringService::PreviewBuilderProfiles(DriftedResumeRequest, DriftedResumePreview));
	TestNotEqual(TEXT("ESH-02 raw Curve drift cannot reuse old receipt hash"), DriftedResumePreview.EngineCurveProposalHash, Fixture.Recipe->BuilderCommitReceipt.EngineCurveProposalHash);
	TestNotEqual(TEXT("ESH-02 raw Curve drift is not NoChange"), DriftedResumePreview.Operation.Status, ECFAuthoringOpStatus::NoChange);
	Fixture.Performance->Data.EngineTorqueCurve.Points.Last().TorqueMultiplier = TailMultiplierBeforeDrift;

	return true;
}

// Actual Wagon PhysicsDraft v3 Engine Curve proposal을 current persistent Recipe/Evidence/private Profile에 mutation0 dry-run 검증합니다.
bool FCFVehicleBuilderWagonEngineTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");
	const TCHAR* WagonEvidencePath = TEXT("/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_Ref_Wagon.DA_Ref_Wagon");
	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipePath);
	UCFVehicleRefEvidence* Evidence = LoadObject<UCFVehicleRefEvidence>(nullptr, WagonEvidencePath);
	if (!TestNotNull(TEXT("ESH-02 actual Wagon Recipe loads"), Recipe)
		|| !TestNotNull(TEXT("ESH-02 actual Wagon Evidence loads"), Evidence))
	{
		return false;
	}

	UCFVehicleData* Target = Recipe->TargetVehicleData.LoadSynchronous();
	UCFPerformanceProfile* PerformanceProfile = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!TestNotNull(TEXT("ESH-02 actual Wagon Target loads"), Target)
		|| !TestNotNull(TEXT("ESH-02 actual Wagon Performance Profile loads"), PerformanceProfile))
	{
		return false;
	}

	// Product mutation0를 증명할 pre-preview persistent truth입니다.
	const FString EvidenceFingerprintBefore = Evidence->EvidenceFingerprint;
	const int32 RecipeRevisionBefore = Recipe->AuthoringRevision;
	const int32 PerformanceRevisionBefore = PerformanceProfile->Meta.AuthoringRevision;
	const bool bEngineCurveEnabledBefore = PerformanceProfile->Data.bUseEngineTorqueCurve;
	FCFVehicleDefinitionSnapshot TargetBefore;
	FString SnapshotError;
	if (!TestTrue(TEXT("ESH-02 actual Wagon target snapshot builds"), FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}

	const FString PhysicsDraftPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(), TEXT("CarFight"), TEXT("VehicleBuilder"), TEXT("PhysicsDraft.json")));
	FString PhysicsDraftJson;
	if (!TestTrue(TEXT("ESH-02 actual Wagon PhysicsDraft exists"), FFileHelper::LoadFileToString(PhysicsDraftJson, *PhysicsDraftPath)))
	{
		return false;
	}

	FCFBuilderPhysicsDraft PhysicsDraft;
	if (!TestTrue(TEXT("ESH-02 actual Wagon PhysicsDraft v3 deserializes"), FJsonObjectConverter::JsonObjectStringToUStruct(PhysicsDraftJson, &PhysicsDraft, 0, 0)))
	{
		return false;
	}
	TestEqual(TEXT("ESH-02 actual Wagon PhysicsDraft schema is v3"), PhysicsDraft.SchemaRevision, 3);
	TestEqual(TEXT("ESH-02 actual Wagon PhysicsDraft Recipe binding matches"), PhysicsDraft.RecipeId, Recipe->RecipeId);
	TestEqual(TEXT("ESH-02 actual Wagon PhysicsDraft Target binding matches"), PhysicsDraft.TargetDefinitionPath, FSoftObjectPath(Target));
	TestEqual(TEXT("ESH-02 actual Wagon PhysicsDraft Evidence identity matches"), PhysicsDraft.EvidenceId, Evidence->EvidenceId);
	TestEqual(TEXT("ESH-02 actual Wagon PhysicsDraft Evidence fingerprint matches current"), PhysicsDraft.ExpectedEvidenceFingerprint, Evidence->EvidenceFingerprint);
	TestTrue(TEXT("ESH-02 actual Wagon proposal enables vehicle-specific Engine Curve"), PhysicsDraft.ProfilePayload.PerformanceData.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-02 actual Wagon Engine Curve has six points"), PhysicsDraft.ProfilePayload.PerformanceData.EngineTorqueCurve.Points.Num(), 6);
	TestEqual(TEXT("ESH-02 actual Wagon Engine Curve review is GAME_BIAS"), PhysicsDraft.EngineCurveReview.Disposition, ECFBuilderEngineCurveDisposition::GameBias);
	TestEqual(TEXT("ESH-02 actual Wagon Engine Curve method is stable"), PhysicsDraft.EngineCurveReview.MethodId, FName(TEXT("SparseAnchorEngineCurveBias")));

	// Draft가 소비한다고 선언한 모든 Claim이 current persistent Evidence에서 canonical인지 확인합니다.
	for (const FName ClaimId : PhysicsDraft.ConsumedClaimIds)
	{
		const FCFRefClaim* Claim = Evidence->Claims.FindByPredicate([ClaimId](const FCFRefClaim& Candidate)
		{
			return Candidate.ClaimId == ClaimId;
		});
		if (!TestNotNull(*FString::Printf(TEXT("ESH-02 actual consumed Claim exists: %s"), *ClaimId.ToString()), Claim))
		{
			return false;
		}
		TestEqual(*FString::Printf(TEXT("ESH-02 actual consumed Claim canonical: %s"), *ClaimId.ToString()), Claim->ResolutionState, ECFRefClaimResolution::Canonical);
	}

	FString EngineCurveValidationError;
	if (!TestTrue(
		TEXT("ESH-02 actual Wagon Engine Curve review validates against current Evidence"),
		CFBuilderEngineUtil::ValidateEngineCurveReview(
			PhysicsDraft.EngineCurveReview,
			PhysicsDraft.ProfilePayload.PerformanceData,
			*Evidence,
			PhysicsDraft.ConsumedClaimIds,
			EngineCurveValidationError)))
	{
		AddError(EngineCurveValidationError);
		return false;
	}

	const FString EngineCurveProposalHash = CFBuilderEngineUtil::BuildEngineCurveProposalHash(
		PhysicsDraft.EngineCurveReview,
		PhysicsDraft.ProfilePayload.PerformanceData);
	TestFalse(TEXT("ESH-02 actual Wagon Engine Curve proposal hash is non-empty"), EngineCurveProposalHash.IsEmpty());

	FCFBuilderEngineCurveDiagnostic EngineDiagnostic;
	CFBuilderEngineUtil::BuildEngineCurveDiagnostic(
		PhysicsDraft.EngineCurveReview,
		EngineCurveProposalHash,
		PhysicsDraft.ProfilePayload.PerformanceData,
		EngineDiagnostic);
	TestEqual(TEXT("ESH-02 actual Wagon Engine Curve diagnostic blockers zero"), EngineDiagnostic.Blockers.Num(), 0);
	TestTrue(TEXT("ESH-02 actual Wagon Engine Curve exposes GAME_BIAS USER warning"), EngineDiagnostic.Warnings.ContainsByPredicate([](const FString& Warning)
	{
		return Warning.Contains(TEXT("Performance.EngineCurveGameBias"));
	}));

	// 250PS published power plateau가 5400/5700 RPM derived anchor에서 물리 관계 P=T*omega를 만족하는지 직접 확인합니다.
	const double PublishedPowerKw = 250.0 * 0.73549875;
	for (const int32 PointIndex : {3, 4})
	{
		const FCFVehicleEngineTorquePoint& Point = PhysicsDraft.ProfilePayload.PerformanceData.EngineTorqueCurve.Points[PointIndex];
		const double TorqueNm = static_cast<double>(PhysicsDraft.ProfilePayload.PerformanceData.EngineMaxTorqueByFeel.NeutralValue) * Point.TorqueMultiplier;
		const double DerivedPowerKw = TorqueNm * static_cast<double>(Point.EngineRPM) / 9549.296596425384;
		TestTrue(
			*FString::Printf(TEXT("ESH-02 actual power anchor %.0fRPM matches 250PS"), Point.EngineRPM),
			FMath::IsNearlyEqual(DerivedPowerKw, PublishedPowerKw, 0.05));
	}

	// Existing Step 5 facade에 실제 Draft를 넣어 current persistent authority 기준 mutation0 preview를 검증합니다.
	FCFBuilderProfileCommitRequest PreviewRequest;
	PreviewRequest.Recipe = Recipe;
	PreviewRequest.ExpectedOwnerRecipeId = Recipe->RecipeId;
	PreviewRequest.Payload = PhysicsDraft.ProfilePayload;
	PreviewRequest.EvidenceBinding.EvidencePath = FSoftObjectPath(Evidence);
	PreviewRequest.EvidenceBinding.ExpectedEvidenceId = Evidence->EvidenceId;
	PreviewRequest.EvidenceBinding.ExpectedEvidenceFingerprint = Evidence->EvidenceFingerprint;
	PreviewRequest.EvidenceBinding.ConsumedClaimIds = PhysicsDraft.ConsumedClaimIds;
	PreviewRequest.TransmissionReview = PhysicsDraft.TransmissionReview;
	PreviewRequest.EngineCurveReview = PhysicsDraft.EngineCurveReview;
	PreviewRequest.UpstreamBuilderProposalHash = PhysicsDraft.ProposalCorrelationHash;
	PreviewRequest.CallContext.CallerKind = ECFAuthoringCallerKind::Automation;
	PreviewRequest.CallContext.ClientOperationId = TEXT("ESH-02-Wagon-EngineCurve-DryRun");

	FCFBuilderProfileCommitPreview Preview;
	if (!TestTrue(TEXT("ESH-02 actual Wagon Step 5 Engine Curve preview succeeds"), FCFVehicleAuthoringService::PreviewBuilderProfiles(PreviewRequest, Preview)))
	{
		AddError(Preview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("ESH-02 actual Wagon preview Engine Curve hash matches direct calculation"), Preview.EngineCurveProposalHash, EngineCurveProposalHash);
	TestEqual(TEXT("ESH-02 actual Wagon preview Engine Curve blockers zero"), Preview.EngineCurveDiagnostic.Blockers.Num(), 0);
	TestFalse(TEXT("ESH-02 actual Wagon preview does not mutate Profiles"), Preview.Operation.Mutation.bProfileChanged);
	TestFalse(TEXT("ESH-02 actual Wagon preview does not mutate Recipe"), Preview.Operation.Mutation.bRecipeChanged);
	TestFalse(TEXT("ESH-02 actual Wagon preview does not mutate Target"), Preview.Operation.Mutation.bTargetChanged);
	TestFalse(TEXT("ESH-02 actual Wagon preview does not Save"), Preview.Operation.Mutation.bSavePerformed);

	// Preview 뒤 persistent truth가 exact 유지되는지 재확인합니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!TestTrue(TEXT("ESH-02 actual Wagon post-preview target snapshot builds"), FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}
	TestEqual(TEXT("ESH-02 actual Wagon Target hash unchanged"), TargetAfter.DefinitionHash, TargetBefore.DefinitionHash);
	TestEqual(TEXT("ESH-02 actual Wagon Evidence fingerprint unchanged"), Evidence->EvidenceFingerprint, EvidenceFingerprintBefore);
	TestEqual(TEXT("ESH-02 actual Wagon Recipe revision unchanged"), Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("ESH-02 actual Wagon Performance revision unchanged"), PerformanceProfile->Meta.AuthoringRevision, PerformanceRevisionBefore);
	TestEqual(TEXT("ESH-02 actual Wagon persistent Engine Curve opt-in unchanged"), PerformanceProfile->Data.bUseEngineTorqueCurve, bEngineCurveEnabledBefore);

	// USER review에 표시할 bounded dry-run diagnostic을 Saved에 남깁니다.
	FString EngineDiagnosticJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Preview.EngineCurveDiagnostic, EngineDiagnosticJson))
	{
		AddError(TEXT("ESH-02 actual Wagon Engine diagnostic JSON serialize failed."));
		return false;
	}
	const FString ResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(), TEXT("CarFight"), TEXT("VehicleBuilder"), TEXT("WagonEnginePreview.json")));
	const FString ResultJson = FString::Printf(
		TEXT("{\n  \"EngineCurveProposalHash\": \"%s\",\n  \"TransmissionProposalHash\": \"%s\",\n  \"Diagnostic\": %s,\n  \"ProductAssetMutation\": false,\n  \"SavePerformed\": false\n}\n"),
		*Preview.EngineCurveProposalHash,
		*Preview.TransmissionProposalHash,
		*EngineDiagnosticJson);
	if (!FFileHelper::SaveStringToFile(ResultJson, *ResultPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		AddError(FString::Printf(TEXT("ESH-02 WagonEnginePreview.json write failed: %s"), *ResultPath));
		return false;
	}

	AddInfo(FString::Printf(TEXT("WAGON_ENGINE_CURVE_PROPOSAL_HASH=%s"), *Preview.EngineCurveProposalHash));
	AddInfo(FString::Printf(TEXT("WAGON_TRANSMISSION_PROPOSAL_HASH=%s"), *Preview.TransmissionProposalHash));
	return true;
}

// 별도 fresh Editor process에서 actual Wagon ESH-02 persistent Profile/receipt와 post-DefinitionApply Target Curve를 직접 readback합니다.
bool FCFVehicleBuilderWagonEnginePersistedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");
	const TCHAR* ExpectedEngineCurveProposalHash = TEXT("773221966499c6295f2a652a21b270a5");
	const TCHAR* ExpectedTransmissionProposalHash = TEXT("e1be2562d77fb1f5a993d6e7f5d17962");

	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipePath);
	if (!TestNotNull(TEXT("ESH-02 persisted Wagon Recipe loads"), Recipe))
	{
		return false;
	}

	UCFVehicleData* Target = Recipe->TargetVehicleData.LoadSynchronous();
	UCFPerformanceProfile* Performance = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	UCFVehicleBaseProfile* Base = Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	UCFDrivetrainProfile* Drivetrain = Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	UCFHandlingProfile* Handling = Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	if (!TestNotNull(TEXT("ESH-02 persisted Wagon Target loads"), Target)
		|| !TestNotNull(TEXT("ESH-02 persisted Wagon Performance loads"), Performance)
		|| !TestNotNull(TEXT("ESH-02 persisted Wagon Base loads"), Base)
		|| !TestNotNull(TEXT("ESH-02 persisted Wagon Drivetrain loads"), Drivetrain)
		|| !TestNotNull(TEXT("ESH-02 persisted Wagon Handling loads"), Handling))
	{
		return false;
	}

	// Fresh disk readback에서 approved Engine Curve receipt가 exact 유지돼야 합니다.
	TestTrue(TEXT("ESH-02 persisted Builder receipt valid"), Recipe->BuilderCommitReceipt.IsValid());
	TestEqual(TEXT("ESH-02 persisted Engine Curve hash exact"), Recipe->BuilderCommitReceipt.EngineCurveProposalHash, FString(ExpectedEngineCurveProposalHash));
	TestEqual(TEXT("ESH-02 persisted Transmission hash preserved"), Recipe->BuilderCommitReceipt.TransmissionProposalHash, FString(ExpectedTransmissionProposalHash));
	TestEqual(TEXT("ESH-02 persisted Resolver revision is 5"), Recipe->BuilderCommitReceipt.ResolverContractRevision, 5);
	TestEqual(TEXT("ESH-02 persisted Engine Curve disposition GAME_BIAS"), Recipe->BuilderCommitReceipt.EngineCurveReview.Disposition, ECFBuilderEngineCurveDisposition::GameBias);
	TestEqual(TEXT("ESH-02 persisted Engine Curve method exact"), Recipe->BuilderCommitReceipt.EngineCurveReview.MethodId, FName(TEXT("SparseAnchorEngineCurveBias")));

	// USER-approved Engine Curve의 expected RPM 배열입니다.
	const float ExpectedRpm[6] = {900.0f, 1800.0f, 4800.0f, 5400.0f, 5700.0f, 6500.0f};
	// USER-approved Engine Curve의 expected normalized torque multiplier 배열입니다.
	const float ExpectedMultiplier[6] = {0.60f, 1.00f, 1.00f, 0.929033824f, 0.880137307f, 0.65f};

	// Performance Profile은 approved 6-point vehicle-specific Curve를 persistent 소유해야 합니다.
	TestTrue(TEXT("ESH-02 persisted Performance enables Engine Curve"), Performance->Data.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-02 persisted Performance curve point count"), Performance->Data.EngineTorqueCurve.Points.Num(), 6);
	if (Performance->Data.EngineTorqueCurve.Points.Num() == 6)
	{
		for (int32 PointIndex = 0; PointIndex < 6; ++PointIndex)
		{
			// 현재 Performance Profile의 한 Engine Curve point입니다.
			const FCFVehicleEngineTorquePoint& Point = Performance->Data.EngineTorqueCurve.Points[PointIndex];
			TestTrue(
				*FString::Printf(TEXT("ESH-02 persisted Performance Curve point %d RPM exact"), PointIndex),
				FMath::IsNearlyEqual(Point.EngineRPM, ExpectedRpm[PointIndex], 0.0001f));
			TestTrue(
				*FString::Printf(TEXT("ESH-02 persisted Performance Curve point %d multiplier exact"), PointIndex),
				FMath::IsNearlyEqual(Point.TorqueMultiplier, ExpectedMultiplier[PointIndex], 0.0001f));
		}
	}

	// USER DefinitionApply 뒤 Target은 approved 8AT/Engine Curve와 ESH-03 fixed-common 5500/2000을 persistent 소유해야 합니다.
	TestEqual(TEXT("ESH-02/03 persisted Target still 8 forward gears"), Target->VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.Num(), 8);
	TestTrue(TEXT("ESH-02/03 persisted Target FinalRatio still 3.20"), FMath::IsNearlyEqual(Target->VehicleMovementConfig.FinalRatio, 3.20f, 0.0001f));
	TestEqual(TEXT("ESH-03 persisted Target ChangeUpRPM exact"), Target->VehicleMovementConfig.ChangeUpRPM, 5500.0f);
	TestEqual(TEXT("ESH-03 persisted Target ChangeDownRPM preserved"), Target->VehicleMovementConfig.ChangeDownRPM, 2000.0f);
	TestEqual(TEXT("ESH-03 persisted Drivetrain ChangeUpRPM exact"), Drivetrain->Data.ChangeUpRPM, 5500.0f);
	TestEqual(TEXT("ESH-03 persisted Drivetrain ChangeDownRPM preserved"), Drivetrain->Data.ChangeDownRPM, 2000.0f);
	TestTrue(TEXT("ESH-02 persisted Target Engine Curve applied"), Target->VehicleMovementConfig.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-02 persisted Target curve point count"), Target->VehicleMovementConfig.EngineTorqueCurve.Points.Num(), 6);
	if (Target->VehicleMovementConfig.EngineTorqueCurve.Points.Num() == 6)
	{
		for (int32 PointIndex = 0; PointIndex < 6; ++PointIndex)
		{
			// 현재 persistent Target의 한 Engine Curve point입니다.
			const FCFVehicleEngineTorquePoint& Point = Target->VehicleMovementConfig.EngineTorqueCurve.Points[PointIndex];
			TestTrue(
				*FString::Printf(TEXT("ESH-02 persisted Target Curve point %d RPM exact"), PointIndex),
				FMath::IsNearlyEqual(Point.EngineRPM, ExpectedRpm[PointIndex], 0.0001f));
			TestTrue(
				*FString::Printf(TEXT("ESH-02 persisted Target Curve point %d multiplier exact"), PointIndex),
				FMath::IsNearlyEqual(Point.TorqueMultiplier, ExpectedMultiplier[PointIndex], 0.0001f));
		}
	}
	TestEqual(
		TEXT("ESH-02 persisted Recipe AppliedState target hash exact"),
		Recipe->AppliedState.AppliedDefinitionHash,
		FString(TEXT("0e5b48e8dcd39deba441da9237218be6")));
	TestEqual(TEXT("ESH-02 persisted Recipe AppliedState resolver revision is 5"), Recipe->AppliedState.ResolverContractRevision, 5);

	// 4-Profile atomic transaction에서 unchanged domains는 semantic fingerprint가 receipt와 fresh 일치해야 합니다.
	FCFVehicleProfileSnapshotSet ProfileSnapshots;
	FString SnapshotError;
	if (!TestTrue(
		TEXT("ESH-02 persisted 4-Profile snapshot builds"),
		FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(Base, Drivetrain, Handling, Performance, nullptr, ProfileSnapshots, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}
	TestEqual(TEXT("ESH-02 persisted Base fingerprint matches receipt"), ProfileSnapshots.BaseSource.ProfileFingerprint, Recipe->BuilderCommitReceipt.VehicleBaseFingerprint);
	TestEqual(TEXT("ESH-02 persisted Drivetrain fingerprint matches receipt"), ProfileSnapshots.DrivetrainSource.ProfileFingerprint, Recipe->BuilderCommitReceipt.DrivetrainFingerprint);
	TestEqual(TEXT("ESH-02 persisted Handling fingerprint matches receipt"), ProfileSnapshots.HandlingSource.ProfileFingerprint, Recipe->BuilderCommitReceipt.HandlingFingerprint);
	TestEqual(TEXT("ESH-02 persisted Performance fingerprint matches receipt"), ProfileSnapshots.PerformanceSource.ProfileFingerprint, Recipe->BuilderCommitReceipt.PerformanceFingerprint);

	AddInfo(FString::Printf(
		TEXT("WAGON_ENGINE_PERSISTED engine_hash=%s recipe_revision=%d performance_revision=%d target_curve=%s"),
		*Recipe->BuilderCommitReceipt.EngineCurveProposalHash,
		Recipe->AuthoringRevision,
		Performance->Meta.AuthoringRevision,
		Target->VehicleMovementConfig.bUseEngineTorqueCurve ? TEXT("true") : TEXT("false")));
	return true;
}

// 차종/실제 Asset과 무관하게 Guided Vehicle Builder의 Transmission authoring 계약 전체를 검증합니다.
bool FCFVehicleBuilderTransmissionContractTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderTestsPrivate;

	// Wagon이나 특정 실차 이름을 사용하지 않는 synthetic Evidence fixture입니다.
	UCFVehicleRefEvidence* Evidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Builder Transmission generic Evidence fixture exists"), Evidence))
	{
		return false;
	}

	// Legacy vehicle compatibility는 vehicle-specific review 없이도 기존 fallback을 허용해야 합니다.
	// Legacy compatibility behavior를 검증할 기본 Drivetrain payload입니다.
	FCFDrivetrainProfileData LegacyPayload;
	// Legacy와 missing-review negative case가 공유하는 empty review입니다.
	FCFBuilderTransmissionReview EmptyReview;
	// 각 contract validation의 machine-readable blocker를 확인할 diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("LegacyCompatible preserves transmission compatibility fallback"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::LegacyCompatible,
			EmptyReview,
			LegacyPayload,
			*Evidence,
			{},
			Error));

	// 신규 Guided vehicle은 compatibility fallback만으로 완료될 수 없습니다.
	TestFalse(
		TEXT("VehicleSpecificRequired rejects disabled transmission config"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			EmptyReview,
			LegacyPayload,
			*Evidence,
			{},
			Error));
	TestTrue(TEXT("Disabled config reports vehicle-specific blocker"), Error.Contains(TEXT("Transmission.VehicleSpecificRequired")));

	// 실차로 직접 증명 가능한 Core는 FACT, CarFight-only AutoReverse는 explicit GAME_BIAS로 binding된 generic vehicle이 정상 통과해야 합니다.
	// FACT/DERIVED/GAME_BIAS positive case가 공유할 complete generic Transmission payload입니다.
	FCFDrivetrainProfileData DirectPayload = BuildGenericTransmissionPayload();
	// FACT review가 실제 소비한 canonical Claim ID 목록입니다.
	TArray<FName> DirectConsumedClaims;
	// FACT-backed fields + explicit AutoReverse control GAME_BIAS field-level review입니다.
	FCFBuilderTransmissionReview DirectReview = BuildGenericTransmissionReview(
		*Evidence,
		ECFBuilderTransmissionDisposition::EvidenceDirect,
		DirectConsumedClaims);
	TestTrue(
		TEXT("FACT-backed generic transmission with explicit control policy passes vehicle-specific contract"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			DirectPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));

	// 아무 FACT나 다른 field 근거로 재사용할 수 없도록 exact research FactKey binding을 검증합니다.
	// FinalRatio component가 참조하는 exact direct Claim ID입니다.
	const FName DirectFinalRatioClaimId = DirectReview.Components[4].EvidenceClaimIds[0];
	// Semantic mismatch를 만들 direct FinalRatio Claim입니다.
	FCFRefClaim* DirectFinalRatioClaim = Evidence->Claims.FindByPredicate([DirectFinalRatioClaimId](const FCFRefClaim& Claim)
	{
		return Claim.ClaimId == DirectFinalRatioClaimId;
	});
	if (!TestNotNull(TEXT("Direct FinalRatio claim exists"), DirectFinalRatioClaim))
	{
		return false;
	}
	// 복원할 원래 FinalDrive FactKey입니다.
	const FName OriginalFinalRatioFactKey = DirectFinalRatioClaim->FactKey;
	DirectFinalRatioClaim->FactKey = TEXT("Mass.Curb");
	TestFalse(
		TEXT("Unrelated FACT cannot prove FinalRatio"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			DirectPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Unrelated FACT reports semantic mismatch"), Error.Contains(TEXT("Transmission.ProvenanceSemanticMismatch")));
	DirectFinalRatioClaim->FactKey = OriginalFinalRatioFactKey;

	// Correct FactKey라도 Evidence 값과 proposal 값이 다르면 FACT로 위장한 tuning을 차단합니다.
	// 복원할 원래 FinalDrive ratio Evidence 값입니다.
	const double OriginalFinalRatioValue = DirectFinalRatioClaim->NumberValue;
	DirectFinalRatioClaim->NumberValue += 0.25;
	TestFalse(
		TEXT("Mismatched FACT value cannot prove FinalRatio payload"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			DirectPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Mismatched FACT value reports value mismatch"), Error.Contains(TEXT("Transmission.ProvenanceValueMismatch")));
	DirectFinalRatioClaim->NumberValue = OriginalFinalRatioValue;

	// deterministic hash는 review component 입력 순서와 무관해야 합니다.
	// Complete FACT proposal의 deterministic reference hash입니다.
	const FString DirectHash = CFBuilderTransUtil::BuildTransmissionProposalHash(DirectReview, DirectPayload);
	// Component array 순서만 반대로 바꾼 동등 review입니다.
	FCFBuilderTransmissionReview ReorderedReview = DirectReview;
	Algo::Reverse(ReorderedReview.Components);
	TestFalse(TEXT("Transmission proposal hash is populated"), DirectHash.IsEmpty());
	TestEqual(
		TEXT("Transmission proposal hash ignores review component array order"),
		CFBuilderTransUtil::BuildTransmissionProposalHash(ReorderedReview, DirectPayload),
		DirectHash);

	// payload의 실제 ratio가 바뀌면 같은 provenance metadata여도 proposal hash는 달라져야 합니다.
	// Hash payload binding을 검증하기 위해 1단 ratio만 바꾼 payload입니다.
	FCFDrivetrainProfileData ChangedPayload = DirectPayload;
	ChangedPayload.TransmissionRatios.ForwardGearRatios[0] += 0.10f;
	TestNotEqual(
		TEXT("Transmission proposal hash binds complete drivetrain payload"),
		CFBuilderTransUtil::BuildTransmissionProposalHash(DirectReview, ChangedPayload),
		DirectHash);

	// DERIVED Core도 canonical DERIVED Claim에 binding되면 차량별 proposal로 정상 통과해야 합니다.
	// DERIVED positive case 전용 synthetic Evidence입니다.
	UCFVehicleRefEvidence* DerivedEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage(), NAME_None, RF_Transient);
	// DERIVED review가 소비한 canonical Claim ID 목록입니다.
	TArray<FName> DerivedConsumedClaims;
	// DERIVED-backed field-level review입니다.
	FCFBuilderTransmissionReview DerivedReview = BuildGenericTransmissionReview(
		*DerivedEvidence,
		ECFBuilderTransmissionDisposition::EvidenceDerived,
		DerivedConsumedClaims);
	TestTrue(
		TEXT("DERIVED-backed generic transmission passes vehicle-specific contract"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DerivedReview,
			DirectPayload,
			*DerivedEvidence,
			DerivedConsumedClaims,
			Error));

	// DERIVED output claim만 소비하고 계산 input FACT를 누락하면 재현 가능한 provenance가 아니므로 차단해야 합니다.
	// AutomaticGears DERIVED output Claim ID입니다.
	const FName AutomaticDerivedClaimId = DerivedReview.Components[0].EvidenceClaimIds[0];
	// AutomaticGears DERIVED output Claim입니다.
	const FCFRefClaim* AutomaticDerivedClaim = DerivedEvidence->Claims.FindByPredicate([AutomaticDerivedClaimId](const FCFRefClaim& Claim)
	{
		return Claim.ClaimId == AutomaticDerivedClaimId;
	});
	if (!TestNotNull(TEXT("Automatic DERIVED output claim exists"), AutomaticDerivedClaim)
		|| !TestTrue(TEXT("Automatic DERIVED output has input fact"), AutomaticDerivedClaim->InputClaimIds.Num() == 1))
	{
		return false;
	}
	// DERIVED calculation input 하나를 current consumed set에서 제거한 목록입니다.
	TArray<FName> MissingDerivedInputClaims = DerivedConsumedClaims;
	MissingDerivedInputClaims.Remove(AutomaticDerivedClaim->InputClaimIds[0]);
	TestFalse(
		TEXT("DERIVED output without consumed FACT input is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DerivedReview,
			DirectPayload,
			*DerivedEvidence,
			MissingDerivedInputClaims,
			Error));
	TestTrue(TEXT("Missing DERIVED input reports provenance blocker"), Error.Contains(TEXT("Transmission.ProvenanceUnbound")));

	// 실차 exact 값이 Unknown이어도 explicit GAME_BIAS method가 Unknown provenance를 보존하면 정상 통과해야 합니다.
	// GAME_BIAS positive case 전용 synthetic Evidence입니다.
	UCFVehicleRefEvidence* GameBiasEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage(), NAME_None, RF_Transient);
	// Ratio/shift는 Unknown을 보존하고 automatic-control fields는 TransmissionType FACT를 소비하는 Claim 목록입니다.
	TArray<FName> GameBiasConsumedClaims;
	// explicit method + Unknown Fact를 가진 GAME_BIAS review입니다.
	FCFBuilderTransmissionReview GameBiasReview = BuildGenericTransmissionReview(
		*GameBiasEvidence,
		ECFBuilderTransmissionDisposition::GameBias,
		GameBiasConsumedClaims);
	TestTrue(
		TEXT("GAME_BIAS generic transmission with explicit method and Unknown facts passes"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			GameBiasReview,
			DirectPayload,
			*GameBiasEvidence,
			GameBiasConsumedClaims,
			Error));

	// GAME_BIAS ratio-set은 ratio Unknown만으로 임의 단수를 만들 수 없고 GearCount FACT/Unknown origin도 필요합니다.
	// GearCount origin만 제거한 ForwardRatios GAME_BIAS review입니다.
	FCFBuilderTransmissionReview MissingGearCountOriginReview = GameBiasReview;
	MissingGearCountOriginReview.Components[2].UnknownFactIds.Remove(FName(TEXT("TRANS_UNKNOWN_GEAR_COUNT")));
	TestFalse(
		TEXT("GAME_BIAS forward ratios without gear-count origin are blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			MissingGearCountOriginReview,
			DirectPayload,
			*GameBiasEvidence,
			GameBiasConsumedClaims,
			Error));
	TestTrue(TEXT("Missing GAME_BIAS gear-count origin reports semantic blocker"), Error.Contains(TEXT("Transmission.ProvenanceSemanticMismatch")));

	// Core 하나라도 BaselineInherited이면 신규 Guided completion을 차단해야 합니다.
	// Core 하나를 BaselineInherited로 강등한 invalid review입니다.
	FCFBuilderTransmissionReview BaselineReview = DirectReview;
	BaselineReview.Components[0].Disposition = ECFBuilderTransmissionDisposition::BaselineInherited;
	BaselineReview.Components[0].EvidenceClaimIds.Reset();
	TestFalse(
		TEXT("Vehicle-specific Core BaselineInherited is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			BaselineReview,
			DirectPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Baseline core reports missing proposal blocker"), Error.Contains(TEXT("Transmission.CoreProposalMissing")));

	// Claim이 Evidence에는 있어도 current consumed set에서 빠지면 stale/unbound provenance로 차단해야 합니다.
	// 첫 FACT Claim을 current consumed set에서 제거한 stale/unbound 목록입니다.
	TArray<FName> MissingConsumedClaim = DirectConsumedClaims;
	MissingConsumedClaim.RemoveAt(0);
	TestFalse(
		TEXT("Unconsumed field claim is blocked as unbound provenance"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			DirectPayload,
			*Evidence,
			MissingConsumedClaim,
			Error));
	TestTrue(TEXT("Unconsumed claim reports provenance blocker"), Error.Contains(TEXT("Transmission.ProvenanceUnbound")));

	// Evidence gear count와 proposed ratio count가 다르면 차량 종류와 무관하게 fail-closed해야 합니다.
	// Evidence expected gear count만 5단으로 바꾼 contradiction review입니다.
	FCFBuilderTransmissionReview WrongCountReview = DirectReview;
	WrongCountReview.ExpectedForwardGearCount = 5;
	TestFalse(
		TEXT("Forward gear count contradiction is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			WrongCountReview,
			DirectPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Gear count contradiction reports dedicated blocker"), Error.Contains(TEXT("Transmission.GearCountConflict")));

	// reviewed gear count 자체가 비어 있는 vehicle-specific proposal도 완료로 인정하지 않습니다.
	// ExpectedForwardGearCount가 0인 incomplete review입니다.
	FCFBuilderTransmissionReview MissingCountReview = DirectReview;
	MissingCountReview.ExpectedForwardGearCount = 0;
	TestFalse(
		TEXT("Missing reviewed forward gear count is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			MissingCountReview,
			DirectPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Missing gear count reports dedicated blocker"), Error.Contains(TEXT("Transmission.GearCountConflict")));

	// 빈 ratio array, zero/negative ratio와 zero FinalRatio는 complete Transmission payload가 아닙니다.
	// Forward ratio array가 비어 있는 invalid payload입니다.
	FCFDrivetrainProfileData EmptyForwardRatiosPayload = DirectPayload;
	EmptyForwardRatiosPayload.TransmissionRatios.ForwardGearRatios.Reset();
	TestFalse(
		TEXT("Empty forward ratio set is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			EmptyForwardRatiosPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Empty ratio set reports invalid payload blocker"), Error.Contains(TEXT("Transmission.InvalidRatioPayload")));

	// FinalRatio가 0인 invalid payload입니다.
	FCFDrivetrainProfileData ZeroFinalRatioPayload = DirectPayload;
	ZeroFinalRatioPayload.FinalRatio = 0.0f;
	TestFalse(
		TEXT("Zero FinalRatio is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			ZeroFinalRatioPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Zero FinalRatio reports invalid payload blocker"), Error.Contains(TEXT("Transmission.InvalidRatioPayload")));

	// 전진 기어비가 상위 단수로 갈수록 감소하지 않으면 structural blocker입니다.
	// 2→3단에서 ratio가 증가하도록 만든 invalid structural payload입니다.
	FCFDrivetrainProfileData WrongOrderPayload = DirectPayload;
	WrongOrderPayload.TransmissionRatios.ForwardGearRatios = {3.20f, 2.05f, 2.20f, 1.00f};
	TestFalse(
		TEXT("Invalid forward ratio order is blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			WrongOrderPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Ratio order reports dedicated blocker"), Error.Contains(TEXT("Transmission.RatioOrderInvalid")));

	// Downshift threshold가 Upshift 이상이면 fixed-shift contract 자체가 모순입니다.
	// ChangeDownRPM이 ChangeUpRPM과 같아지도록 만든 invalid fixed-shift payload입니다.
	FCFDrivetrainProfileData InvalidShiftPayload = DirectPayload;
	InvalidShiftPayload.ChangeDownRPM = InvalidShiftPayload.ChangeUpRPM;
	TestFalse(
		TEXT("Contradictory fixed shift RPM thresholds are blocked"),
		CFBuilderTransUtil::ValidateTransmissionReview(
			ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
			DirectReview,
			InvalidShiftPayload,
			*Evidence,
			DirectConsumedClaims,
			Error));
	TestTrue(TEXT("Invalid shift thresholds report dedicated blocker"), Error.Contains(TEXT("Transmission.InvalidShiftRPM")));

	// Resolver가 Blocked이면 compatibility/default resolved Wheel Radius가 존재하더라도 vehicle-specific ShiftSpeed authority로 사용하면 안 됩니다.
	FCFVehicleResolveResult BlockedResolveResult;
	BlockedResolveResult.ResolveStatus = ECFVehicleResolveStatus::Blocked;
	// Generic blocked Resolver에서도 gear-ratio/post-shift 계산은 유지하고 WSA-dependent speed만 fail-closed해야 합니다.
	FCFBuilderTransmissionDiagnostic BlockedDiagnostic;
	CFBuilderTransUtil::BuildTransmissionDiagnostic(
		ECFBuilderTransmissionPolicy::VehicleSpecificRequired,
		DirectReview,
		DirectHash,
		DirectPayload,
		FCFPerformanceProfileData(),
		BlockedResolveResult,
		BlockedDiagnostic);
	TestTrue(TEXT("Blocked Resolver transmission diagnostic is evaluated"), BlockedDiagnostic.bEvaluated);
	TestTrue(TEXT("Blocked Resolver diagnostic keeps vehicle-specific requirement"), BlockedDiagnostic.bVehicleSpecificRequired);
	TestTrue(TEXT("Blocked Resolver reports WheelRadius authority blocker"), BlockedDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
	{
		return Blocker.Contains(TEXT("Transmission.WheelRadiusAuthorityUnavailable"));
	}));
	TestEqual(
		TEXT("Blocked Resolver still produces generic gear rows"),
		BlockedDiagnostic.Gears.Num(),
		DirectPayload.TransmissionRatios.ForwardGearRatios.Num());
	if (!BlockedDiagnostic.Gears.IsEmpty())
	{
		TestFalse(TEXT("Blocked Resolver never exposes ShiftSpeed"), BlockedDiagnostic.Gears[0].bShiftSpeedAvailable);
		TestTrue(TEXT("Blocked Resolver keeps ratio-only PostShift RPM diagnostic"), BlockedDiagnostic.Gears[0].bPostShiftAvailable);
	}

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

	// Guided 신규 차량에서 Compatibility Default가 조용히 vehicle-specific 완료로 승계되지 않는지 검증합니다.
	Fixture.Recipe->BuilderTransmissionPolicy = ECFBuilderTransmissionPolicy::VehicleSpecificRequired;
	// bUseTransmissionConfig=false인 current compatibility-fallback payload request입니다.
	FCFBuilderProfileCommitRequest MissingTransmissionRequest = BuildProfileCommitRequest(Fixture);
	// Vehicle-specific gate가 차단해야 하는 mutation0 preview입니다.
	FCFBuilderProfileCommitPreview MissingTransmissionPreview;
	TestFalse(
		TEXT("VehicleSpecificRequired blocks bUseTransmissionConfig=false compatibility fallback"),
		FCFVehicleAuthoringService::PreviewBuilderProfiles(MissingTransmissionRequest, MissingTransmissionPreview));
	TestEqual(
		TEXT("Vehicle-specific compatibility fallback is ValidationBlocked"),
		MissingTransmissionPreview.Operation.ErrorCode,
		ECFAuthoringErrorCode::ValidationBlocked);

	// 아래 기존 VB-P0-05 regression은 Legacy compatibility baseline을 그대로 검증합니다.
	Fixture.Recipe->BuilderTransmissionPolicy = ECFBuilderTransmissionPolicy::LegacyCompatible;

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

	// WSA Socket mode에서 unrelated Builder Profile field는 계속 proposal 가능해야 합니다.
	Fixture.Recipe->WheelVisualIntent.Mode = ECFWheelVisualIntentMode::SocketScaleFromChassis;
	++Fixture.Recipe->AuthoringRevision;
	FCFBuilderProfileCommitRequest SocketSafeRequest = BuildProfileCommitRequest(Fixture);
	SocketSafeRequest.Payload.VehicleBaseData.ChassisWidth = Fixture.VehicleBase->Data.ChassisWidth + 0.5f;
	FCFBuilderProfileCommitPreview SocketSafePreview;
	if (!TestTrue(TEXT("WSA Socket mode allows unrelated VehicleBase proposal"), FCFVehicleAuthoringService::PreviewBuilderProfiles(SocketSafeRequest, SocketSafePreview)))
	{
		AddError(SocketSafePreview.Operation.Message);
		return false;
	}

	// AI/Builder가 Socket-owned FrontWheelRadius를 shadow value로 바꾸려 하면 Preview 단계에서 fail-closed해야 합니다.
	FCFBuilderProfileCommitRequest SocketWheelMutationRequest = BuildProfileCommitRequest(Fixture);
	SocketWheelMutationRequest.Payload.VehicleBaseData.FrontWheelRadius += 1.0f;
	FCFBuilderProfileCommitPreview SocketWheelMutationPreview;
	TestFalse(TEXT("WSA Socket mode blocks FrontWheelRadius profile mutation"), FCFVehicleAuthoringService::PreviewBuilderProfiles(SocketWheelMutationRequest, SocketWheelMutationPreview));
	TestEqual(TEXT("WSA protected wheel mutation is ValidationBlocked"), SocketWheelMutationPreview.Operation.ErrorCode, ECFAuthoringErrorCode::ValidationBlocked);

	// bUseReferenceWheelGeometry 자체를 켜서 Profile을 다시 Wheel Size authority로 만들려는 proposal도 차단합니다.
	FCFBuilderProfileCommitRequest SocketReferenceEnableRequest = BuildProfileCommitRequest(Fixture);
	SocketReferenceEnableRequest.Payload.VehicleBaseData.bUseReferenceWheelGeometry = true;
	FCFBuilderProfileCommitPreview SocketReferenceEnablePreview;
	TestFalse(TEXT("WSA Socket mode blocks bUseReferenceWheelGeometry enable"), FCFVehicleAuthoringService::PreviewBuilderProfiles(SocketReferenceEnableRequest, SocketReferenceEnablePreview));

	Fixture.Recipe->WheelVisualIntent.Mode = ECFWheelVisualIntentMode::UseProfilePolicy;
	++Fixture.Recipe->AuthoringRevision;

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
