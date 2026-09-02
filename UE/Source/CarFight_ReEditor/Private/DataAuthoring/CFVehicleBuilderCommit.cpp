// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBuilderCommit.cpp
// Version: v1.6.0
// Date: 2026-09-01
// Description: CF-FQ-040 Builder private Profile commit + ESH-02 Engine Curve provenance/hash guard 구현입니다.
// Scope: fresh Evidence + Recipe binding + OwnerRecipeId + 4 current fingerprints + prospective Resolver/Target state를 exact approval scope에 묶습니다.
// Changelog:
// - v1.6.0: vehicle-specific Performance Engine Curve review를 fresh Evidence/consumed Claim에 검증하고 complete Curve payload와 deterministic EngineCurveProposalHash로 persistent receipt에 binding.
// - v1.5.0: VehicleSpecificRequired Recipe의 Transmission Core field-level review를 fresh Evidence에 검증하고 complete Drivetrain payload와 deterministic TransmissionProposalHash로 receipt에 binding. Fixed-shift blocker diagnostic을 Step 5 preview에서 fail-closed.
// - v1.4.0: SocketScaleFromChassis에서 VehicleBase Reference wheel geometry 5필드의 Builder mutation을 baseline-preserve fail-closed로 차단.
// - v1.3.0: BuilderCommitReceipt에 consumed canonical Claim ID 목록 자체를 lexical order로 보존해 Editor restart 뒤 Final Review provenance resume를 지원.
// - v1.2.0: accepted proposal/Evidence/Claim set/4 Profile fingerprint를 Recipe BuilderCommitReceipt에 persistent binding하고, receipt-only migration과 PostEditChange/final fingerprint readback/rollback을 추가.
// - v1.1.0: 설계 검수 교정으로 current EvidenceId/fingerprint/consumed canonical claims를 fresh 검증하고 complete prospective 4 Profile을 Shared Resolver/Definition validation에 통과시킨 결과를 proposal hash에 binding. Current gameplay에 manual shift가 없으므로 Builder Transmission은 automatic gears + auto reverse를 요구.
// - v1.0.0: PreviewBuilderProfiles / CommitBuilderProfiles 최초 구현. Shared/legacy Profile mutation과 raw VehicleData write를 fail-closed로 차단.
// Migration:
// - 이 파일은 기존 Shared Profile B2 편집 경로를 대체하지 않습니다. Meta.OwnerRecipeId가 exact RecipeId이고 Recipe가 실제 binding한 VehicleBase/Drivetrain/Handling/Performance 4 Profile에만 사용합니다.
// - Commit은 complete typed Data 4개와 non-semantic BuilderCommitReceipt를 한 FScopedTransaction으로 갱신합니다. VehicleData/OwnerRecipeId는 수정하지 않으며 Save/automatic retry를 수행하지 않습니다.
// - Receipt는 Recipe semantic fingerprint에서 제외되며 current Profile/Evidence가 달라지면 Final Review에서 stale로 판정됩니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "CFBuilderEngineUtil.h"
#include "CFBuilderTransUtil.h"
#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/SecureHash.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleBuilderCommitPrivate
{
	// Builder-private atomic Profile operation의 stable operation identity입니다.
	const FName CommitOperationName(TEXT("CommitBuilderProfiles"));

	// Common typed result를 Builder-private R1 operation 기본 상태로 초기화합니다.
	void InitializeResult(FCFAuthoringOpResult& OutResult, const FString& ClientOperationId = FString())
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = CommitOperationName;
		OutResult.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.ClientOperationId = ClientOperationId;
		OutResult.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		OutResult.Mutation.bSavePerformed = false;
		OutResult.Mutation.bAutomaticRetryPerformed = false;
	}

	// Result를 mutation 없는 Blocked 상태로 종료합니다.
	bool Block(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
		return false;
	}

	// Result를 stale/concurrent Conflict 상태로 종료합니다.
	bool Conflict(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Conflict;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
		return false;
	}

	// Result를 성공 상태로 종료합니다.
	void Succeed(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Proposal hash canonical payload에 delimiter-safe token을 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Canonical UTF-8 payload를 existing Authoring facade와 같은 lowercase MD5 digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR payload를 platform-independent UTF-8 byte sequence로 변환한 값입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// UTF-8 bytes를 누적할 MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// 최종 128-bit MD5 결과 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Digest를 lowercase hexadecimal로 저장할 결과입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// 한 nibble을 lowercase hex로 변환하는 stable 문자표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Soft object path를 exact expected Profile class로 resolve/load합니다.
	template <typename TProfile>
	TProfile* LoadProfile(const FSoftObjectPath& ProfilePath, FString& OutError)
	{
		if (!ProfilePath.IsValid())
		{
			OutError = TEXT("Builder-private Profile path가 비어 있습니다.");
			return nullptr;
		}

		// 이미 로드된 Profile 또는 path에서 load한 UObject입니다.
		UObject* LoadedObject = ProfilePath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = ProfilePath.TryLoad();
		}
		// Exact expected Profile type으로 확인한 결과입니다.
		TProfile* Profile = Cast<TProfile>(LoadedObject);
		if (!Profile)
		{
			OutError = FString::Printf(TEXT("Builder-private Profile type/path가 올바르지 않습니다: %s"), *ProfilePath.ToString());
			return nullptr;
		}

		OutError.Reset();
		return Profile;
	}

	// Builder request가 가리키는 네 exact Profile UObject를 한 묶음으로 보관합니다.
	struct FLoadedProfiles
	{
		// VehicleBase private Profile입니다.
		UCFVehicleBaseProfile* VehicleBase = nullptr;

		// Drivetrain private Profile입니다.
		UCFDrivetrainProfile* Drivetrain = nullptr;

		// Handling private Profile입니다.
		UCFHandlingProfile* Handling = nullptr;

		// Performance private Profile입니다.
		UCFPerformanceProfile* Performance = nullptr;
	};

	// Recipe binding과 request path가 exact 일치하는 네 private Profile을 load합니다.
	bool LoadAndValidateBoundProfiles(
		const FCFBuilderProfileCommitRequest& Request,
		FLoadedProfiles& OutProfiles,
		FCFAuthoringOpResult& OutOperation)
	{
		if (!Request.Recipe)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Builder-private Profile commit에는 persistent Recipe가 필요합니다."));
		}
		if (!Request.Recipe->RecipeId.IsValid()
			|| !Request.ExpectedOwnerRecipeId.IsValid()
			|| Request.Recipe->RecipeId != Request.ExpectedOwnerRecipeId)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("ExpectedOwnerRecipeId가 current RecipeId와 정확히 일치해야 합니다."));
		}

		// Recipe가 현재 binding한 VehicleBase Profile path입니다.
		const FSoftObjectPath BoundVehicleBasePath = Request.Recipe->ProfileBindings.VehicleBaseProfile.ToSoftObjectPath();
		// Recipe가 현재 binding한 Drivetrain Profile path입니다.
		const FSoftObjectPath BoundDrivetrainPath = Request.Recipe->ProfileBindings.DrivetrainProfile.ToSoftObjectPath();
		// Recipe가 현재 binding한 Handling Profile path입니다.
		const FSoftObjectPath BoundHandlingPath = Request.Recipe->ProfileBindings.HandlingProfile.ToSoftObjectPath();
		// Recipe가 현재 binding한 Performance Profile path입니다.
		const FSoftObjectPath BoundPerformancePath = Request.Recipe->ProfileBindings.PerformanceProfile.ToSoftObjectPath();

		if (BoundVehicleBasePath != Request.Payload.VehicleBaseProfilePath
			|| BoundDrivetrainPath != Request.Payload.DrivetrainProfilePath
			|| BoundHandlingPath != Request.Payload.HandlingProfilePath
			|| BoundPerformancePath != Request.Payload.PerformanceProfilePath)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::StateChanged, TEXT("Request의 4 Profile path가 current Recipe binding과 정확히 일치하지 않습니다."));
		}

		// Profile load/type failure diagnostic입니다.
		FString LoadError;
		OutProfiles.VehicleBase = LoadProfile<UCFVehicleBaseProfile>(Request.Payload.VehicleBaseProfilePath, LoadError);
		if (!OutProfiles.VehicleBase)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::ProfileNotFound, LoadError);
		}
		OutProfiles.Drivetrain = LoadProfile<UCFDrivetrainProfile>(Request.Payload.DrivetrainProfilePath, LoadError);
		if (!OutProfiles.Drivetrain)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::ProfileNotFound, LoadError);
		}
		OutProfiles.Handling = LoadProfile<UCFHandlingProfile>(Request.Payload.HandlingProfilePath, LoadError);
		if (!OutProfiles.Handling)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::ProfileNotFound, LoadError);
		}
		OutProfiles.Performance = LoadProfile<UCFPerformanceProfile>(Request.Payload.PerformanceProfilePath, LoadError);
		if (!OutProfiles.Performance)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::ProfileNotFound, LoadError);
		}

		if (OutProfiles.VehicleBase->Meta.OwnerRecipeId != Request.ExpectedOwnerRecipeId
			|| OutProfiles.Drivetrain->Meta.OwnerRecipeId != Request.ExpectedOwnerRecipeId
			|| OutProfiles.Handling->Meta.OwnerRecipeId != Request.ExpectedOwnerRecipeId
			|| OutProfiles.Performance->Meta.OwnerRecipeId != Request.ExpectedOwnerRecipeId)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::UnsupportedOperation, TEXT("4 Profile 중 하나 이상이 current Recipe가 소유한 Builder-private Profile이 아닙니다. shared/legacy Profile은 이 commit 경로로 수정할 수 없습니다."));
		}

		return true;
	}

	// Evidence path를 exact UCFVehicleRefEvidence로 resolve/load합니다.
	UCFVehicleRefEvidence* LoadEvidence(const FSoftObjectPath& EvidencePath, FString& OutError)
	{
		if (!EvidencePath.IsValid())
		{
			OutError = TEXT("Reference Evidence path가 비어 있습니다.");
			return nullptr;
		}

		// 이미 로드된 Evidence 또는 path에서 load한 UObject입니다.
		UObject* LoadedObject = EvidencePath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = EvidencePath.TryLoad();
		}
		// Exact Reference Evidence type으로 확인한 결과입니다.
		UCFVehicleRefEvidence* Evidence = Cast<UCFVehicleRefEvidence>(LoadedObject);
		if (!Evidence)
		{
			OutError = FString::Printf(TEXT("Reference Evidence type/path가 올바르지 않습니다: %s"), *EvidencePath.ToString());
			return nullptr;
		}

		OutError.Reset();
		return Evidence;
	}

	// Evidence binding과 current Evidence semantic state/consumed claims를 fresh fail-closed 검증합니다.
	bool ValidateCurrentEvidence(
		const FCFBuilderProfileCommitRequest& Request,
		FString& OutFreshEvidenceFingerprint,
		FCFAuthoringOpResult& OutOperation)
	{
		// Evidence load/fingerprint diagnostic입니다.
		FString EvidenceError;
		UCFVehicleRefEvidence* Evidence = LoadEvidence(Request.EvidenceBinding.EvidencePath, EvidenceError);
		if (!Evidence)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::InvalidSemanticInput, EvidenceError);
		}
		if (!Request.EvidenceBinding.ExpectedEvidenceId.IsValid()
			|| Evidence->EvidenceId != Request.EvidenceBinding.ExpectedEvidenceId)
		{
			return Conflict(OutOperation, ECFAuthoringErrorCode::StateChanged, TEXT("Reference Evidence identity가 preview 입력과 일치하지 않습니다."));
		}
		if (Evidence->TargetRecipeId != Request.Recipe->RecipeId
			|| Evidence->TargetRecipePath != FSoftObjectPath(Request.Recipe)
			|| Evidence->TargetDefinitionPath != Request.Recipe->TargetVehicleData.ToSoftObjectPath())
		{
			return Block(OutOperation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("Reference Evidence의 Recipe/Definition binding이 current Builder vehicle과 일치하지 않습니다."));
		}
		if (!Evidence->BuildEvidenceFingerprint(OutFreshEvidenceFingerprint, EvidenceError))
		{
			return Block(OutOperation, ECFAuthoringErrorCode::InternalError, EvidenceError);
		}
		if (Request.EvidenceBinding.ExpectedEvidenceFingerprint.IsEmpty()
			|| Request.EvidenceBinding.ExpectedEvidenceFingerprint != OutFreshEvidenceFingerprint
			|| Evidence->EvidenceFingerprint != OutFreshEvidenceFingerprint)
		{
			return Conflict(OutOperation, ECFAuthoringErrorCode::StateChanged, TEXT("Reference Evidence fingerprint가 preview 입력 또는 generated cached fingerprint와 일치하지 않습니다. 새 Research/Preview가 필요합니다."));
		}

		// 중복 consumed claim을 막을 stable ID set입니다.
		TSet<FName> ConsumedClaimSet;
		for (const FName ClaimId : Request.EvidenceBinding.ConsumedClaimIds)
		{
			if (ClaimId.IsNone() || ConsumedClaimSet.Contains(ClaimId))
			{
				return Block(OutOperation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("ConsumedClaimIds에는 None 또는 중복 Claim ID를 사용할 수 없습니다."));
			}
			ConsumedClaimSet.Add(ClaimId);

			// 요청한 Claim ID의 current Evidence record입니다.
			const FCFRefClaim* Claim = Evidence->Claims.FindByPredicate([ClaimId](const FCFRefClaim& Candidate)
			{
				return Candidate.ClaimId == ClaimId;
			});
			if (!Claim || Claim->ResolutionState != ECFRefClaimResolution::Canonical)
			{
				return Block(OutOperation, ECFAuthoringErrorCode::ValidationBlocked, FString::Printf(TEXT("Consumed Claim은 current Evidence에서 Canonical이어야 합니다: %s"), *ClaimId.ToString()));
			}
		}

		for (const FCFRefConflict& EvidenceConflict : Evidence->Conflicts)
		{
			if (EvidenceConflict.Severity == ECFRefConflictSeverity::Block)
			{
				return Block(OutOperation, ECFAuthoringErrorCode::ValidationBlocked, FString::Printf(TEXT("Reference Evidence에 unresolved/blocking conflict가 남아 있습니다: %s"), *EvidenceConflict.ConflictId.ToString()));
			}
		}
		for (const FCFRefUnknownFact& UnknownFact : Evidence->UnknownFacts)
		{
			if (UnknownFact.BlockingUse == ECFRefUnknownBlockingUse::ProposalBlock)
			{
				return Block(OutOperation, ECFAuthoringErrorCode::ValidationBlocked, FString::Printf(TEXT("Reference Evidence의 required fact가 Unknown 상태입니다: %s"), *UnknownFact.FactKey.ToString()));
			}
		}

		return true;
	}

	// UE 5.8 Transmission ratio 배열이 positive finite magnitude만 포함하는지 검사합니다.
	bool ArePositiveFiniteRatios(const TArray<float>& Ratios)
	{
		if (Ratios.IsEmpty())
		{
			return false;
		}
		for (const float Ratio : Ratios)
		{
			if (!FMath::IsFinite(Ratio) || Ratio <= 0.0f)
			{
				return false;
			}
		}
		return true;
	}

	// Shift RPM 값이 Chaos 내부 uint32 의미와 같은 비음수 정수값인지 검사합니다.
	bool IsNonNegativeIntegerRpm(const float RpmValue)
	{
		return FMath::IsFinite(RpmValue)
			&& RpmValue >= 0.0f
			&& FMath::IsNearlyEqual(RpmValue, FMath::RoundToFloat(RpmValue));
	}

	// SocketScaleFromChassis에서 Builder Profile proposal이 Wheel Size shadow authority를 만들지 않는지 검사합니다.
	bool ValidateSocketScaleWheelAuthority(
		const FCFBuilderProfileCommitRequest& Request,
		const FLoadedProfiles& CurrentProfiles,
		FString& OutError)
	{
		if (!Request.Recipe || Request.Recipe->WheelVisualIntent.Mode != ECFWheelVisualIntentMode::SocketScaleFromChassis)
		{
			return true;
		}
		if (!CurrentProfiles.VehicleBase)
		{
			OutError = TEXT("Socket Scale Wheel authority 검증에 current VehicleBase Profile이 필요합니다.");
			return false;
		}

		const FCFVehicleBaseProfileData& Current = CurrentProfiles.VehicleBase->Data;
		const FCFVehicleBaseProfileData& Proposed = Request.Payload.VehicleBaseData;
		if (Current.bUseReferenceWheelGeometry)
		{
			OutError = TEXT("SocketScaleFromChassis에서는 current VehicleBase Profile의 bUseReferenceWheelGeometry가 false여야 합니다. Reference wheel geometry는 sanity reference로만 유지하세요.");
			return false;
		}

		const bool bProtectedFieldsPreserved = Proposed.bUseReferenceWheelGeometry == Current.bUseReferenceWheelGeometry
			&& FMath::IsNearlyEqual(Proposed.FrontWheelRadius, Current.FrontWheelRadius, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Proposed.RearWheelRadius, Current.RearWheelRadius, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Proposed.FrontWheelWidth, Current.FrontWheelWidth, KINDA_SMALL_NUMBER)
			&& FMath::IsNearlyEqual(Proposed.RearWheelWidth, Current.RearWheelWidth, KINDA_SMALL_NUMBER);
		if (!bProtectedFieldsPreserved)
		{
			OutError = TEXT("SocketScaleFromChassis에서는 AI/Builder Profile proposal이 bUseReferenceWheelGeometry 또는 Front/Rear WheelRadius/Width를 변경할 수 없습니다. 타이어 크기는 USER Wheel Socket Scale + Wheel Mesh Bounds가 소유합니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Complete prospective Builder payload가 Profile/Transmission 최소 물리 계약을 만족하는지 검사합니다.
	bool ValidateProspectivePayload(const FCFBuilderPrivateProfilePayload& Payload, FString& OutError)
	{
		if (!FMath::IsFinite(Payload.VehicleBaseData.ChassisWidth) || Payload.VehicleBaseData.ChassisWidth <= 0.0f)
		{
			OutError = TEXT("VehicleBase.ChassisWidth는 0보다 큰 finite 값이어야 합니다.");
			return false;
		}
		if (Payload.VehicleBaseData.bUseReferenceWheelGeometry
			&& (!FMath::IsFinite(Payload.VehicleBaseData.FrontWheelRadius) || Payload.VehicleBaseData.FrontWheelRadius <= 0.0f
				|| !FMath::IsFinite(Payload.VehicleBaseData.RearWheelRadius) || Payload.VehicleBaseData.RearWheelRadius <= 0.0f
				|| !FMath::IsFinite(Payload.VehicleBaseData.FrontWheelWidth) || Payload.VehicleBaseData.FrontWheelWidth <= 0.0f
				|| !FMath::IsFinite(Payload.VehicleBaseData.RearWheelWidth) || Payload.VehicleBaseData.RearWheelWidth <= 0.0f))
		{
			OutError = TEXT("Reference wheel geometry를 사용할 때 전/후륜 반지름·폭은 모두 0보다 큰 finite 값이어야 합니다.");
			return false;
		}

		// Complete Drivetrain Transmission payload입니다.
		const FCFDrivetrainProfileData& DrivetrainData = Payload.DrivetrainData;
		if (DrivetrainData.bUseTransmissionConfig
			&& (!ArePositiveFiniteRatios(DrivetrainData.TransmissionRatios.ForwardGearRatios)
				|| !ArePositiveFiniteRatios(DrivetrainData.TransmissionRatios.ReverseGearRatios)
				|| !FMath::IsFinite(DrivetrainData.FinalRatio) || DrivetrainData.FinalRatio <= 0.0f
				|| !IsNonNegativeIntegerRpm(DrivetrainData.ChangeUpRPM)
				|| !IsNonNegativeIntegerRpm(DrivetrainData.ChangeDownRPM)
				|| !FMath::IsFinite(DrivetrainData.GearChangeTime) || DrivetrainData.GearChangeTime < 0.0f
				|| !FMath::IsFinite(DrivetrainData.TransmissionEfficiency) || DrivetrainData.TransmissionEfficiency < 0.0f || DrivetrainData.TransmissionEfficiency > 1.0f
				|| DrivetrainData.ChangeDownRPM > DrivetrainData.ChangeUpRPM
				|| !DrivetrainData.bUseAutomaticGears
				|| !DrivetrainData.bUseAutoReverse))
		{
			OutError = TEXT("Drivetrain Transmission payload가 유효하지 않습니다. 기어비는 양수 finite magnitude, Shift RPM은 비음수 정수여야 하며 현재 CarFight 조작계에는 manual shift가 없으므로 Builder Transmission은 Automatic Gears + Auto Reverse를 사용해야 합니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// 네 Profile UObject의 typed Data fingerprint와 OwnerRecipeId snapshot을 한 번에 생성합니다.
	bool BuildProfileFingerprints(
		const FLoadedProfiles& Profiles,
		FCFBuilderProfileFingerprints& OutFingerprints,
		FString& OutError)
	{
		// SnapshotBuilder가 생성할 5-domain value snapshot입니다.
		FCFVehicleProfileSnapshotSet Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			Profiles.VehicleBase,
			Profiles.Drivetrain,
			Profiles.Handling,
			Profiles.Performance,
			nullptr,
			Snapshot,
			OutError))
		{
			return false;
		}

		OutFingerprints.VehicleBaseFingerprint = Snapshot.BaseSource.ProfileFingerprint;
		OutFingerprints.DrivetrainFingerprint = Snapshot.DrivetrainSource.ProfileFingerprint;
		OutFingerprints.HandlingFingerprint = Snapshot.HandlingSource.ProfileFingerprint;
		OutFingerprints.PerformanceFingerprint = Snapshot.PerformanceSource.ProfileFingerprint;
		return true;
	}

	// Complete prospective 4 Profile을 transient Recipe에 binding해 fingerprint와 Shared Resolver/Definition validation을 mutation0로 계산합니다.
	bool BuildProspectiveState(
		const FCFBuilderProfileCommitRequest& Request,
		const FLoadedProfiles& CurrentProfiles,
		FCFBuilderProfileFingerprints& OutFingerprints,
		FCFVehicleResolveResult& OutResolveResult,
		FString& OutError)
	{
		// VehicleBase source를 value-copy한 transient duplicate입니다.
		UCFVehicleBaseProfile* ProspectiveVehicleBase = DuplicateObject<UCFVehicleBaseProfile>(CurrentProfiles.VehicleBase, GetTransientPackage());
		// Drivetrain source를 value-copy한 transient duplicate입니다.
		UCFDrivetrainProfile* ProspectiveDrivetrain = DuplicateObject<UCFDrivetrainProfile>(CurrentProfiles.Drivetrain, GetTransientPackage());
		// Handling source를 value-copy한 transient duplicate입니다.
		UCFHandlingProfile* ProspectiveHandling = DuplicateObject<UCFHandlingProfile>(CurrentProfiles.Handling, GetTransientPackage());
		// Performance source를 value-copy한 transient duplicate입니다.
		UCFPerformanceProfile* ProspectivePerformance = DuplicateObject<UCFPerformanceProfile>(CurrentProfiles.Performance, GetTransientPackage());
		if (!ProspectiveVehicleBase || !ProspectiveDrivetrain || !ProspectiveHandling || !ProspectivePerformance)
		{
			OutError = TEXT("Prospective Builder-private Profile transient duplicate를 만들 수 없습니다.");
			return false;
		}

		ProspectiveVehicleBase->Data = Request.Payload.VehicleBaseData;
		ProspectiveDrivetrain->Data = Request.Payload.DrivetrainData;
		ProspectiveHandling->Data = Request.Payload.HandlingData;
		ProspectivePerformance->Data = Request.Payload.PerformanceData;

		// Prospective transient duplicates를 fingerprint할 묶음입니다.
		FLoadedProfiles ProspectiveProfiles;
		ProspectiveProfiles.VehicleBase = ProspectiveVehicleBase;
		ProspectiveProfiles.Drivetrain = ProspectiveDrivetrain;
		ProspectiveProfiles.Handling = ProspectiveHandling;
		ProspectiveProfiles.Performance = ProspectivePerformance;
		if (!BuildProfileFingerprints(ProspectiveProfiles, OutFingerprints, OutError))
		{
			return false;
		}

		// Current Recipe/Target/ProjectDefault/Asset/DriveState truth를 deterministic baseline으로 가져올 read request입니다.
		FCFVehicleAuthoringReadRequest CurrentReadRequest;
		CurrentReadRequest.Recipe = Request.Recipe;
		CurrentReadRequest.TargetVehicleData = Request.Recipe->TargetVehicleData.LoadSynchronous();
		CurrentReadRequest.CallerKind = Request.CallContext.CallerKind;
		if (!CurrentReadRequest.TargetVehicleData)
		{
			OutError = TEXT("Prospective Resolver가 사용할 Recipe-bound Target VehicleData를 load할 수 없습니다.");
			return false;
		}

		// Current stable snapshot facts를 얻는 shared facade result입니다. ResolveStatus 자체는 prospective payload가 교정할 수 있으므로 여기서 Success를 강제하지 않습니다.
		FCFVehicleResolveReadResult CurrentReadResult;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(CurrentReadRequest, CurrentReadResult))
		{
			OutError = CurrentReadResult.Operation.Message.IsEmpty()
				? TEXT("Prospective Builder payload의 current snapshot baseline을 만들 수 없습니다.")
				: CurrentReadResult.Operation.Message;
			return false;
		}

		// Transient duplicate의 randomized UObject path 대신 actual persistent Profile identity를 사용할 prospective 4-domain snapshot입니다.
		FCFVehicleProfileSnapshotSet ProspectiveFourProfiles;
		if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			ProspectiveVehicleBase,
			ProspectiveDrivetrain,
			ProspectiveHandling,
			ProspectivePerformance,
			nullptr,
			ProspectiveFourProfiles,
			OutError))
		{
			return false;
		}
		ProspectiveFourProfiles.BaseSource.SourceObjectPath = Request.Payload.VehicleBaseProfilePath;
		ProspectiveFourProfiles.DrivetrainSource.SourceObjectPath = Request.Payload.DrivetrainProfilePath;
		ProspectiveFourProfiles.HandlingSource.SourceObjectPath = Request.Payload.HandlingProfilePath;
		ProspectiveFourProfiles.PerformanceSource.SourceObjectPath = Request.Payload.PerformanceProfilePath;

		// Current Recipe semantic identity와 non-profile snapshots는 그대로 두고 private 4 Profile payload/source만 prospective 값으로 치환한 pure Resolver request입니다.
		FCFVehicleResolveRequest ProspectiveResolveRequest = CurrentReadResult.ResolveRequest;
		ProspectiveResolveRequest.Profiles.BaseSource = ProspectiveFourProfiles.BaseSource;
		ProspectiveResolveRequest.Profiles.BaseData = ProspectiveFourProfiles.BaseData;
		ProspectiveResolveRequest.Profiles.DrivetrainSource = ProspectiveFourProfiles.DrivetrainSource;
		ProspectiveResolveRequest.Profiles.DrivetrainData = ProspectiveFourProfiles.DrivetrainData;
		ProspectiveResolveRequest.Profiles.HandlingSource = ProspectiveFourProfiles.HandlingSource;
		ProspectiveResolveRequest.Profiles.HandlingData = ProspectiveFourProfiles.HandlingData;
		ProspectiveResolveRequest.Profiles.PerformanceSource = ProspectiveFourProfiles.PerformanceSource;
		ProspectiveResolveRequest.Profiles.PerformanceData = ProspectiveFourProfiles.PerformanceData;
		ProspectiveResolveRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;

		if (!FCFVehicleResolver::Resolve(ProspectiveResolveRequest, OutResolveResult))
		{
			OutError = TEXT("Prospective Builder payload Shared Pure Resolver가 internal Error로 실패했습니다.");
			return false;
		}
		if (OutResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
		{
			OutError = TEXT("Complete prospective Builder 4 Profile이 Shared Resolver/Definition validation을 통과하지 못했습니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// 네 domain fingerprint 묶음이 exact 동일한지 반환합니다.
	bool FingerprintsEqual(const FCFBuilderProfileFingerprints& A, const FCFBuilderProfileFingerprints& B)
	{
		return A.VehicleBaseFingerprint == B.VehicleBaseFingerprint
			&& A.DrivetrainFingerprint == B.DrivetrainFingerprint
			&& A.HandlingFingerprint == B.HandlingFingerprint
			&& A.PerformanceFingerprint == B.PerformanceFingerprint;
	}

	// Current Recipe/Target snapshots와 exact deterministic fingerprints를 생성합니다.
	bool BuildCurrentOwnerState(
		const FCFBuilderProfileCommitRequest& Request,
		FString& OutRecipeFingerprint,
		FString& OutTargetDefinitionHash,
		FSoftObjectPath& OutTargetPath,
		FCFAuthoringOpResult& OutOperation)
	{
		// Current persistent Recipe semantic snapshot입니다.
		FCFVehicleRecipeSnapshot RecipeSnapshot;
		// Snapshot build diagnostic입니다.
		FString SnapshotError;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.Recipe, RecipeSnapshot, SnapshotError))
		{
			return Block(OutOperation, ECFAuthoringErrorCode::InternalError, SnapshotError);
		}
		OutRecipeFingerprint = RecipeSnapshot.RecipeFingerprint;

		OutTargetPath = Request.Recipe->TargetVehicleData.ToSoftObjectPath();
		if (!OutTargetPath.IsValid())
		{
			return Block(OutOperation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Recipe에 Target VehicleData binding이 없습니다."));
		}

		// 이미 load된 Target 또는 binding path에서 load한 UObject입니다.
		UObject* TargetObject = OutTargetPath.ResolveObject();
		if (!TargetObject)
		{
			TargetObject = OutTargetPath.TryLoad();
		}
		// Recipe binding이 가리키는 exact VehicleData입니다.
		UCFVehicleData* TargetVehicleData = Cast<UCFVehicleData>(TargetObject);
		if (!TargetVehicleData)
		{
			return Block(OutOperation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Recipe Target VehicleData를 resolve할 수 없습니다."));
		}

		// Current Target의 full Definition snapshot입니다.
		FCFVehicleDefinitionSnapshot DefinitionSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, DefinitionSnapshot, SnapshotError))
		{
			return Block(OutOperation, ECFAuthoringErrorCode::InternalError, SnapshotError);
		}
		OutTargetDefinitionHash = DefinitionSnapshot.DefinitionHash;
		return true;
	}

	// Consumed Claim ID를 physical array order와 무관한 canonical string으로 만듭니다.
	FString BuildConsumedClaimIdString(const TArray<FName>& ConsumedClaimIds)
	{
		// deterministic lexical order를 만들 Claim ID 복사본입니다.
		TArray<FName> SortedClaimIds = ConsumedClaimIds;
		SortedClaimIds.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});

		// delimiter-safe Claim ID payload입니다.
		FString ClaimPayload;
		for (const FName ClaimId : SortedClaimIds)
		{
			AppendToken(ClaimPayload, TEXT("ClaimId"), ClaimId.ToString());
		}
		return ClaimPayload;
	}

	// Consumed Claim ID를 deterministic lexical order로 정렬한 persistent resume 목록을 만듭니다.
	TArray<FName> BuildCanonicalConsumedClaimIds(const TArray<FName>& ConsumedClaimIds)
	{
		// Caller order와 무관하게 receipt에 저장할 canonical 복사본입니다.
		TArray<FName> SortedClaimIds = ConsumedClaimIds;
		SortedClaimIds.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});
		return SortedClaimIds;
	}

	// Consumed Claim ID set의 order-independent deterministic hash를 만듭니다.
	FString BuildConsumedClaimIdHash(const TArray<FName>& ConsumedClaimIds)
	{
		return HashUtf8Payload(BuildConsumedClaimIdString(ConsumedClaimIds));
	}

	// Current Recipe receipt가 fresh proposal/profile/evidence를 exact 증명하는지 확인합니다.
	bool ReceiptMatchesFreshProposal(
		const UCFVehicleRecipeData& Recipe,
		const FCFBuilderProfileCommitRequest& Request,
		const FCFBuilderProfileCommitPreview& Preview)
	{
		// Current persistent Builder provenance receipt입니다.
		const FCFVehicleBuilderCommitReceipt& Receipt = Recipe.BuilderCommitReceipt;
		return Receipt.IsValid()
			&& Receipt.EvidencePath == Request.EvidenceBinding.EvidencePath
			&& Receipt.EvidenceId == Request.EvidenceBinding.ExpectedEvidenceId
			&& Receipt.EvidenceFingerprint == Preview.EvidenceFingerprint
			&& Receipt.ConsumedClaimIds == BuildCanonicalConsumedClaimIds(Request.EvidenceBinding.ConsumedClaimIds)
			&& Receipt.ConsumedClaimIdsHash == BuildConsumedClaimIdHash(Request.EvidenceBinding.ConsumedClaimIds)
			&& Receipt.VehicleBaseFingerprint == Preview.ProspectiveFingerprints.VehicleBaseFingerprint
			&& Receipt.DrivetrainFingerprint == Preview.ProspectiveFingerprints.DrivetrainFingerprint
			&& Receipt.HandlingFingerprint == Preview.ProspectiveFingerprints.HandlingFingerprint
			&& Receipt.PerformanceFingerprint == Preview.ProspectiveFingerprints.PerformanceFingerprint
			&& Receipt.ProspectiveResolvedDefinitionHash == Preview.ProspectiveResolvedDefinitionHash
			&& Receipt.TransmissionPolicy == Request.Recipe->BuilderTransmissionPolicy
			&& Receipt.TransmissionProposalHash == Preview.TransmissionProposalHash
			&& (Preview.TransmissionProposalHash.IsEmpty()
				|| CFBuilderTransUtil::BuildTransmissionProposalHash(Receipt.TransmissionReview, Request.Payload.DrivetrainData) == Preview.TransmissionProposalHash)
			&& Receipt.EngineCurveProposalHash == Preview.EngineCurveProposalHash
			&& (Preview.EngineCurveProposalHash.IsEmpty()
				|| CFBuilderEngineUtil::BuildEngineCurveProposalHash(Receipt.EngineCurveReview, Request.Payload.PerformanceData) == Preview.EngineCurveProposalHash)
			&& Receipt.ResolverContractRevision == Preview.Proposal.ResolverContractRevision;
	}

	// Fresh proposal 결과를 persistent non-semantic Builder receipt로 materialize합니다.
	FCFVehicleBuilderCommitReceipt BuildReceipt(
		const FCFBuilderProfileCommitRequest& Request,
		const FCFBuilderProfileCommitPreview& Preview)
	{
		// Commit 후 Recipe에 저장할 exact provenance receipt입니다.
		FCFVehicleBuilderCommitReceipt Receipt;
		Receipt.ProposalHash = Preview.Proposal.ProposalHash;
		Receipt.EvidencePath = Request.EvidenceBinding.EvidencePath;
		Receipt.EvidenceId = Request.EvidenceBinding.ExpectedEvidenceId;
		Receipt.EvidenceFingerprint = Preview.EvidenceFingerprint;
		Receipt.ConsumedClaimIds = BuildCanonicalConsumedClaimIds(Request.EvidenceBinding.ConsumedClaimIds);
		Receipt.ConsumedClaimIdsHash = BuildConsumedClaimIdHash(Request.EvidenceBinding.ConsumedClaimIds);
		Receipt.VehicleBaseFingerprint = Preview.ProspectiveFingerprints.VehicleBaseFingerprint;
		Receipt.DrivetrainFingerprint = Preview.ProspectiveFingerprints.DrivetrainFingerprint;
		Receipt.HandlingFingerprint = Preview.ProspectiveFingerprints.HandlingFingerprint;
		Receipt.PerformanceFingerprint = Preview.ProspectiveFingerprints.PerformanceFingerprint;
		Receipt.ProspectiveResolvedDefinitionHash = Preview.ProspectiveResolvedDefinitionHash;
		Receipt.TransmissionPolicy = Request.Recipe->BuilderTransmissionPolicy;
		Receipt.TransmissionProposalHash = Preview.TransmissionProposalHash;
		Receipt.TransmissionReview = Request.TransmissionReview;
		Receipt.EngineCurveProposalHash = Preview.EngineCurveProposalHash;
		Receipt.EngineCurveReview = Request.EngineCurveReview;
		Receipt.ResolverContractRevision = Preview.Proposal.ResolverContractRevision;
		return Receipt;
	}

	// Evidence, owner, paths, current/prospective fingerprints와 prospective Resolver result를 하나의 approval hash로 결합합니다.
	FString BuildProposalHash(
		const FCFBuilderProfileCommitRequest& Request,
		const FCFBuilderProfileFingerprints& CurrentFingerprints,
		const FCFBuilderProfileFingerprints& ProspectiveFingerprints,
		const FString& FreshEvidenceFingerprint,
		const FString& ProspectiveSourceSignature,
		const FString& ProspectiveResolvedDefinitionHash,
		const FString& RecipeFingerprint,
		const FSoftObjectPath& TargetPath,
		const FString& TargetDefinitionHash)
	{
		// Localized text/UObject address를 제외한 canonical approval payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), CommitOperationName.ToString());
		AppendToken(Payload, TEXT("UpstreamBuilderProposalHash"), Request.UpstreamBuilderProposalHash);
		AppendToken(Payload, TEXT("EvidencePath"), Request.EvidenceBinding.EvidencePath.ToString());
		AppendToken(Payload, TEXT("EvidenceId"), Request.EvidenceBinding.ExpectedEvidenceId.ToString(EGuidFormats::DigitsWithHyphensLower));
		AppendToken(Payload, TEXT("EvidenceFingerprint"), FreshEvidenceFingerprint);
		AppendToken(Payload, TEXT("ConsumedClaimIds"), BuildConsumedClaimIdString(Request.EvidenceBinding.ConsumedClaimIds));
		AppendToken(Payload, TEXT("RecipeId"), Request.Recipe->RecipeId.ToString());
		AppendToken(Payload, TEXT("RecipePath"), FSoftObjectPath(Request.Recipe).ToString());
		AppendToken(Payload, TEXT("RecipeFingerprint"), RecipeFingerprint);
		AppendToken(Payload, TEXT("TargetPath"), TargetPath.ToString());
		AppendToken(Payload, TEXT("TargetDefinitionHash"), TargetDefinitionHash);
		AppendToken(Payload, TEXT("OwnerRecipeId"), Request.ExpectedOwnerRecipeId.ToString());
		AppendToken(Payload, TEXT("VehicleBasePath"), Request.Payload.VehicleBaseProfilePath.ToString());
		AppendToken(Payload, TEXT("VehicleBaseCurrent"), CurrentFingerprints.VehicleBaseFingerprint);
		AppendToken(Payload, TEXT("VehicleBaseProspective"), ProspectiveFingerprints.VehicleBaseFingerprint);
		AppendToken(Payload, TEXT("DrivetrainPath"), Request.Payload.DrivetrainProfilePath.ToString());
		AppendToken(Payload, TEXT("DrivetrainCurrent"), CurrentFingerprints.DrivetrainFingerprint);
		AppendToken(Payload, TEXT("DrivetrainProspective"), ProspectiveFingerprints.DrivetrainFingerprint);
		AppendToken(Payload, TEXT("HandlingPath"), Request.Payload.HandlingProfilePath.ToString());
		AppendToken(Payload, TEXT("HandlingCurrent"), CurrentFingerprints.HandlingFingerprint);
		AppendToken(Payload, TEXT("HandlingProspective"), ProspectiveFingerprints.HandlingFingerprint);
		AppendToken(Payload, TEXT("PerformancePath"), Request.Payload.PerformanceProfilePath.ToString());
		AppendToken(Payload, TEXT("PerformanceCurrent"), CurrentFingerprints.PerformanceFingerprint);
		AppendToken(Payload, TEXT("PerformanceProspective"), ProspectiveFingerprints.PerformanceFingerprint);
		AppendToken(Payload, TEXT("ProspectiveSourceSignature"), ProspectiveSourceSignature);
		AppendToken(Payload, TEXT("ProspectiveResolvedDefinitionHash"), ProspectiveResolvedDefinitionHash);
		AppendToken(Payload, TEXT("TransmissionPolicy"), FString::FromInt(static_cast<uint8>(Request.Recipe->BuilderTransmissionPolicy)));
		AppendToken(Payload, TEXT("TransmissionProposalHash"), CFBuilderTransUtil::BuildTransmissionProposalHash(Request.TransmissionReview, Request.Payload.DrivetrainData));
		AppendToken(Payload, TEXT("EngineCurveProposalHash"), CFBuilderEngineUtil::BuildEngineCurveProposalHash(Request.EngineCurveReview, Request.Payload.PerformanceData));
		AppendToken(Payload, TEXT("ResolverRevision"), FString::FromInt(FCFVehicleResolver::CurrentResolverContractRevision));
		return HashUtf8Payload(Payload);
	}
}

// Builder-private 4 Profile complete payload를 fresh Evidence/current owner/fingerprint/Recipe/Target/prospective Resolver에 binding해 mutation0 preview합니다.
bool FCFVehicleAuthoringService::PreviewBuilderProfiles(
	const FCFBuilderProfileCommitRequest& Request,
	FCFBuilderProfileCommitPreview& OutPreview)
{
	using namespace CFVehicleBuilderCommitPrivate;

	OutPreview = FCFBuilderProfileCommitPreview();
	InitializeResult(OutPreview.Operation, Request.CallContext.ClientOperationId);

	// Request path에서 load하고 owner/binding을 확인한 current private Profile 묶음입니다.
	FLoadedProfiles CurrentProfiles;
	if (!LoadAndValidateBoundProfiles(Request, CurrentProfiles, OutPreview.Operation))
	{
		return false;
	}

	// Current EvidenceId/fingerprint/claim resolution을 직접 다시 읽어 확인한 fresh fingerprint입니다.
	if (!ValidateCurrentEvidence(Request, OutPreview.EvidenceFingerprint, OutPreview.Operation))
	{
		return false;
	}

	// Field-level Transmission provenance를 검증할 current Evidence입니다.
	FString TransmissionEvidenceError;
	// Request exact path에서 다시 읽은 current Evidence object입니다.
	UCFVehicleRefEvidence* TransmissionEvidence = LoadEvidence(Request.EvidenceBinding.EvidencePath, TransmissionEvidenceError);
	if (!TransmissionEvidence)
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, TransmissionEvidenceError);
	}
	if (!CFBuilderTransUtil::ValidateTransmissionReview(
		Request.Recipe->BuilderTransmissionPolicy,
		Request.TransmissionReview,
		Request.Payload.DrivetrainData,
		*TransmissionEvidence,
		Request.EvidenceBinding.ConsumedClaimIds,
		TransmissionEvidenceError))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, TransmissionEvidenceError);
	}

	// ESH-02 Performance Engine Curve review를 같은 fresh Evidence/consumed Claim authority에 binding해 검증합니다.
	FString EngineCurveValidationError;
	if (!CFBuilderEngineUtil::ValidateEngineCurveReview(
		Request.EngineCurveReview,
		Request.Payload.PerformanceData,
		*TransmissionEvidence,
		Request.EvidenceBinding.ConsumedClaimIds,
		EngineCurveValidationError))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, EngineCurveValidationError);
	}

	// Complete prospective typed payload의 fail-closed validation diagnostic입니다.
	FString ValidationError;
	if (!ValidateProspectivePayload(Request.Payload, ValidationError))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, ValidationError);
	}
	if (!ValidateSocketScaleWheelAuthority(Request, CurrentProfiles, ValidationError))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, ValidationError);
	}

	// Current persistent Recipe fingerprint입니다.
	FString RecipeFingerprint;
	// Current full Target Definition hash입니다.
	FString TargetDefinitionHash;
	// Current Recipe-bound Target identity입니다.
	FSoftObjectPath TargetPath;
	if (!BuildCurrentOwnerState(Request, RecipeFingerprint, TargetDefinitionHash, TargetPath, OutPreview.Operation))
	{
		return false;
	}

	// Current 네 private Profile data fingerprints입니다.
	FString FingerprintError;
	if (!BuildProfileFingerprints(CurrentProfiles, OutPreview.CurrentFingerprints, FingerprintError))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::InternalError, FingerprintError);
	}
	// Complete prospective 4 Profile을 transient Recipe에 binding해 Shared Resolver/Definition validation까지 실행한 결과입니다.
	FCFVehicleResolveResult ProspectiveResolveResult;
	if (!BuildProspectiveState(Request, CurrentProfiles, OutPreview.ProspectiveFingerprints, ProspectiveResolveResult, FingerprintError))
	{
		return Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, FingerprintError);
	}
	OutPreview.ProspectiveSourceSignature = ProspectiveResolveResult.SourceSignature;
	OutPreview.ProspectiveResolvedDefinitionHash = ProspectiveResolveResult.ResolvedDefinitionHash;
	OutPreview.TransmissionProposalHash = CFBuilderTransUtil::BuildTransmissionProposalHash(Request.TransmissionReview, Request.Payload.DrivetrainData);
	OutPreview.EngineCurveProposalHash = CFBuilderEngineUtil::BuildEngineCurveProposalHash(Request.EngineCurveReview, Request.Payload.PerformanceData);
	CFBuilderEngineUtil::BuildEngineCurveDiagnostic(
		Request.EngineCurveReview,
		OutPreview.EngineCurveProposalHash,
		Request.Payload.PerformanceData,
		OutPreview.EngineCurveDiagnostic);
	if (!OutPreview.EngineCurveDiagnostic.Blockers.IsEmpty())
	{
		return Block(
			OutPreview.Operation,
			ECFAuthoringErrorCode::ValidationBlocked,
			FString::Join(OutPreview.EngineCurveDiagnostic.Blockers, TEXT(" | ")));
	}

	CFBuilderTransUtil::BuildTransmissionDiagnostic(
		Request.Recipe->BuilderTransmissionPolicy,
		Request.TransmissionReview,
		OutPreview.TransmissionProposalHash,
		Request.Payload.DrivetrainData,
		Request.Payload.PerformanceData,
		ProspectiveResolveResult,
		OutPreview.TransmissionDiagnostic);
	if (Request.Recipe->BuilderTransmissionPolicy == ECFBuilderTransmissionPolicy::VehicleSpecificRequired
		&& !OutPreview.TransmissionDiagnostic.Blockers.IsEmpty())
	{
		return Block(
			OutPreview.Operation,
			ECFAuthoringErrorCode::ValidationBlocked,
			FString::Join(OutPreview.TransmissionDiagnostic.Blockers, TEXT(" | ")));
	}

	OutPreview.Operation.CurrentRecipeFingerprint = RecipeFingerprint;
	OutPreview.Operation.CurrentTargetDefinitionHash = TargetDefinitionHash;
	OutPreview.Operation.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;

	OutPreview.Proposal.OperationName = CommitOperationName;
	OutPreview.Proposal.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
	OutPreview.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	OutPreview.Proposal.ExpectedRecipeFingerprint = RecipeFingerprint;
	OutPreview.Proposal.ExpectedTargetDefinitionHash = TargetDefinitionHash;
	OutPreview.Proposal.ProspectiveRecipeFingerprint = RecipeFingerprint;
	OutPreview.Proposal.ProspectiveSourceSignature = OutPreview.ProspectiveSourceSignature;
	OutPreview.Proposal.ProspectiveResolvedDefinitionHash = OutPreview.ProspectiveResolvedDefinitionHash;
	OutPreview.Proposal.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutPreview.Proposal.bTargetMutation = false;
	OutPreview.Proposal.bSavePerformed = false;
	OutPreview.Proposal.ProposalHash = BuildProposalHash(
		Request,
		OutPreview.CurrentFingerprints,
		OutPreview.ProspectiveFingerprints,
		OutPreview.EvidenceFingerprint,
		OutPreview.ProspectiveSourceSignature,
		OutPreview.ProspectiveResolvedDefinitionHash,
		RecipeFingerprint,
		TargetPath,
		TargetDefinitionHash);

	if (FingerprintsEqual(OutPreview.CurrentFingerprints, OutPreview.ProspectiveFingerprints)
		&& ReceiptMatchesFreshProposal(*Request.Recipe, Request, OutPreview))
	{
		OutPreview.Operation.Status = ECFAuthoringOpStatus::NoChange;
		OutPreview.Operation.ErrorCode = ECFAuthoringErrorCode::None;
		OutPreview.Operation.Message = TEXT("Builder-private 4 Profile payload와 persistent provenance receipt가 current accepted proposal과 동일합니다.");
		return true;
	}

	Succeed(
		OutPreview.Operation,
		FingerprintsEqual(OutPreview.CurrentFingerprints, OutPreview.ProspectiveFingerprints)
			? TEXT("Profile payload는 동일하지만 persistent Builder provenance receipt를 생성/갱신해야 합니다. persistent mutation은 아직 수행하지 않았습니다.")
			: TEXT("Builder-private 4 Profile complete payload preview가 생성되었습니다. persistent mutation은 수행하지 않았습니다."));
	return true;
}

// Fresh preview와 AuthoringWrite approval을 재검사한 뒤 Recipe에 binding된 private 4 Profile complete payload를 한 transaction으로 commit합니다.
bool FCFVehicleAuthoringService::CommitBuilderProfiles(
	const FCFBuilderProfileCommitRequest& Request,
	const FCFBuilderProfileCommitPreview& ApprovedPreview,
	FCFAuthoringOpResult& OutResult)
{
	using namespace CFVehicleBuilderCommitPrivate;

	InitializeResult(OutResult, Request.CallContext.ClientOperationId);

	// Commit 직전 current owner/fingerprint/Recipe/Target state를 다시 계산한 fresh preview입니다.
	FCFBuilderProfileCommitPreview FreshPreview;
	if (!PreviewBuilderProfiles(Request, FreshPreview))
	{
		OutResult = FreshPreview.Operation;
		return false;
	}
	if (FreshPreview.Operation.Status == ECFAuthoringOpStatus::NoChange)
	{
		OutResult = FreshPreview.Operation;
		return true;
	}

	if (ApprovedPreview.Proposal.ProposalHash.IsEmpty()
		|| ApprovedPreview.Proposal.ProposalHash != FreshPreview.Proposal.ProposalHash)
	{
		return Conflict(OutResult, ECFAuthoringErrorCode::PreviewOutOfDate, TEXT("승인한 Builder Profile preview가 current fresh preview와 다릅니다. 새 Preview가 필요합니다."));
	}
	if (Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::AuthoringWrite)
	{
		return Block(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Builder-private Profile commit에는 AuthoringWrite approval이 필요합니다."));
	}
	if (Request.CallContext.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		return Block(OutResult, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("ApprovalScopeHash가 fresh Builder Profile proposal과 일치하지 않습니다."));
	}
	if (Request.CallContext.ExpectedRecipeFingerprint != FreshPreview.Proposal.ExpectedRecipeFingerprint)
	{
		return Conflict(OutResult, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("Recipe fingerprint가 preview 이후 변경되었습니다."));
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		return Conflict(OutResult, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Target Definition hash가 preview 이후 변경되었습니다."));
	}
	if (Request.CallContext.ExpectedResolverContractRevision != FreshPreview.Proposal.ResolverContractRevision
		|| Request.CallContext.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		return Conflict(OutResult, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Resolver contract revision이 preview와 일치하지 않습니다."));
	}
	if (!FingerprintsEqual(Request.ExpectedCurrentFingerprints, FreshPreview.CurrentFingerprints))
	{
		return Conflict(OutResult, ECFAuthoringErrorCode::StateChanged, TEXT("Builder-private Profile fingerprint가 preview 이후 변경되었습니다. shared/raw mutation을 덮어쓰지 않습니다."));
	}

	// Fresh commit 시점에 다시 load/owner/binding 검증한 exact private Profile 묶음입니다.
	FLoadedProfiles CurrentProfiles;
	if (!LoadAndValidateBoundProfiles(Request, CurrentProfiles, OutResult))
	{
		return false;
	}

	// 실제 Profile payload가 바뀌는 commit인지 receipt-only migration인지 구분합니다.
	const bool bProfilePayloadChanged = !FingerprintsEqual(FreshPreview.CurrentFingerprints, FreshPreview.ProspectiveFingerprints);

	// Rollback에 사용할 VehicleBase payload backup입니다.
	const FCFVehicleBaseProfileData VehicleBaseBackup = CurrentProfiles.VehicleBase->Data;
	// Rollback에 사용할 Drivetrain payload backup입니다.
	const FCFDrivetrainProfileData DrivetrainBackup = CurrentProfiles.Drivetrain->Data;
	// Rollback에 사용할 Handling payload backup입니다.
	const FCFHandlingProfileData HandlingBackup = CurrentProfiles.Handling->Data;
	// Rollback에 사용할 Performance payload backup입니다.
	const FCFPerformanceProfileData PerformanceBackup = CurrentProfiles.Performance->Data;
	// Rollback에 사용할 VehicleBase revision backup입니다.
	const int32 VehicleBaseRevisionBackup = CurrentProfiles.VehicleBase->Meta.AuthoringRevision;
	// Rollback에 사용할 Drivetrain revision backup입니다.
	const int32 DrivetrainRevisionBackup = CurrentProfiles.Drivetrain->Meta.AuthoringRevision;
	// Rollback에 사용할 Handling revision backup입니다.
	const int32 HandlingRevisionBackup = CurrentProfiles.Handling->Meta.AuthoringRevision;
	// Rollback에 사용할 Performance revision backup입니다.
	const int32 PerformanceRevisionBackup = CurrentProfiles.Performance->Meta.AuthoringRevision;
	// Rollback에 사용할 Recipe receipt backup입니다.
	const FCFVehicleBuilderCommitReceipt ReceiptBackup = Request.Recipe->BuilderCommitReceipt;
	// Rollback에 사용할 Recipe revision backup입니다.
	const int32 RecipeRevisionBackup = Request.Recipe->AuthoringRevision;

	// Commit 전에 각 package의 dirty 상태를 보존하는 helper입니다.
	auto IsPackageDirty = [](const UObject* Object)
	{
		// Object가 속한 package입니다.
		const UPackage* Package = Object ? Object->GetOutermost() : nullptr;
		return Package && Package->IsDirty();
	};
	// Rollback에서 한 object package의 원래 dirty flag를 복원하는 helper입니다.
	auto RestorePackageDirty = [](UObject* Object, const bool bWasDirty)
	{
		// Dirty flag를 복원할 object package입니다.
		UPackage* Package = Object ? Object->GetOutermost() : nullptr;
		if (Package)
		{
			Package->SetDirtyFlag(bWasDirty);
		}
	};
	// Commit 전 VehicleBase package dirty 상태입니다.
	const bool bVehicleBaseDirtyBefore = IsPackageDirty(CurrentProfiles.VehicleBase);
	// Commit 전 Drivetrain package dirty 상태입니다.
	const bool bDrivetrainDirtyBefore = IsPackageDirty(CurrentProfiles.Drivetrain);
	// Commit 전 Handling package dirty 상태입니다.
	const bool bHandlingDirtyBefore = IsPackageDirty(CurrentProfiles.Handling);
	// Commit 전 Performance package dirty 상태입니다.
	const bool bPerformanceDirtyBefore = IsPackageDirty(CurrentProfiles.Performance);
	// Commit 전 Recipe package dirty 상태입니다.
	const bool bRecipeDirtyBefore = IsPackageDirty(Request.Recipe);

	// Profile payload와 receipt를 하나의 Undo/Redo unit으로 묶는 transaction입니다.
	FScopedTransaction Transaction(NSLOCTEXT("CarFight", "CommitBuilderProfiles", "Guided Vehicle Builder - Commit Private Profiles"));
	if (bProfilePayloadChanged)
	{
		CurrentProfiles.VehicleBase->Modify();
		CurrentProfiles.Drivetrain->Modify();
		CurrentProfiles.Handling->Modify();
		CurrentProfiles.Performance->Modify();
	}
	Request.Recipe->Modify();

	if (bProfilePayloadChanged)
	{
		CurrentProfiles.VehicleBase->Data = Request.Payload.VehicleBaseData;
		CurrentProfiles.Drivetrain->Data = Request.Payload.DrivetrainData;
		CurrentProfiles.Handling->Data = Request.Payload.HandlingData;
		CurrentProfiles.Performance->Data = Request.Payload.PerformanceData;
		++CurrentProfiles.VehicleBase->Meta.AuthoringRevision;
		++CurrentProfiles.Drivetrain->Meta.AuthoringRevision;
		++CurrentProfiles.Handling->Meta.AuthoringRevision;
		++CurrentProfiles.Performance->Meta.AuthoringRevision;
	}
	Request.Recipe->BuilderCommitReceipt = BuildReceipt(Request, FreshPreview);
	++Request.Recipe->AuthoringRevision;

	// Rollback 시 persistent payload/revision/receipt/dirty 상태를 모두 복구하는 helper입니다.
	auto RestoreCommitState = [&]()
	{
		if (bProfilePayloadChanged)
		{
			CurrentProfiles.VehicleBase->Data = VehicleBaseBackup;
			CurrentProfiles.Drivetrain->Data = DrivetrainBackup;
			CurrentProfiles.Handling->Data = HandlingBackup;
			CurrentProfiles.Performance->Data = PerformanceBackup;
			CurrentProfiles.VehicleBase->Meta.AuthoringRevision = VehicleBaseRevisionBackup;
			CurrentProfiles.Drivetrain->Meta.AuthoringRevision = DrivetrainRevisionBackup;
			CurrentProfiles.Handling->Meta.AuthoringRevision = HandlingRevisionBackup;
			CurrentProfiles.Performance->Meta.AuthoringRevision = PerformanceRevisionBackup;
			CurrentProfiles.VehicleBase->PostEditChange();
			CurrentProfiles.Drivetrain->PostEditChange();
			CurrentProfiles.Handling->PostEditChange();
			CurrentProfiles.Performance->PostEditChange();
		}
		Request.Recipe->BuilderCommitReceipt = ReceiptBackup;
		Request.Recipe->AuthoringRevision = RecipeRevisionBackup;
		Request.Recipe->PostEditChange();
		RestorePackageDirty(CurrentProfiles.VehicleBase, bVehicleBaseDirtyBefore);
		RestorePackageDirty(CurrentProfiles.Drivetrain, bDrivetrainDirtyBefore);
		RestorePackageDirty(CurrentProfiles.Handling, bHandlingDirtyBefore);
		RestorePackageDirty(CurrentProfiles.Performance, bPerformanceDirtyBefore);
		RestorePackageDirty(Request.Recipe, bRecipeDirtyBefore);
	};

	// Persistent assignment 직후 exact profile fingerprint readback입니다.
	FCFBuilderProfileFingerprints AssignedFingerprints;
	// Assignment readback diagnostic입니다.
	FString ReadbackError;
	if (!BuildProfileFingerprints(CurrentProfiles, AssignedFingerprints, ReadbackError)
		|| !FingerprintsEqual(AssignedFingerprints, FreshPreview.ProspectiveFingerprints)
		|| !ReceiptMatchesFreshProposal(*Request.Recipe, Request, FreshPreview))
	{
		RestoreCommitState();
		Transaction.Cancel();
		return Block(OutResult, ECFAuthoringErrorCode::InternalError, ReadbackError.IsEmpty() ? TEXT("Builder Profile/receipt persistent assignment readback이 approved prospective state와 일치하지 않아 rollback했습니다.") : ReadbackError);
	}

	// Editor details/cache notification은 semantic assignment 검증 뒤 발행합니다.
	if (bProfilePayloadChanged)
	{
		CurrentProfiles.VehicleBase->PostEditChange();
		CurrentProfiles.Drivetrain->PostEditChange();
		CurrentProfiles.Handling->PostEditChange();
		CurrentProfiles.Performance->PostEditChange();
	}
	Request.Recipe->PostEditChange();

	// PostEditChange 뒤에도 semantic payload가 그대로인지 검증할 final fingerprints입니다.
	FCFBuilderProfileFingerprints FinalFingerprints;
	if (!BuildProfileFingerprints(CurrentProfiles, FinalFingerprints, ReadbackError)
		|| !FingerprintsEqual(FinalFingerprints, FreshPreview.ProspectiveFingerprints)
		|| !ReceiptMatchesFreshProposal(*Request.Recipe, Request, FreshPreview))
	{
		RestoreCommitState();
		Transaction.Cancel();
		return Block(OutResult, ECFAuthoringErrorCode::InternalError, ReadbackError.IsEmpty() ? TEXT("PostEditChange 뒤 Builder Profile/receipt semantic readback이 달라져 전체 rollback했습니다.") : ReadbackError);
	}

	// 모든 final readback 성공 뒤에만 package dirty를 남깁니다.
	bool bAnyPackageDirty = Request.Recipe->MarkPackageDirty();
	if (bProfilePayloadChanged)
	{
		bAnyPackageDirty |= CurrentProfiles.VehicleBase->MarkPackageDirty();
		bAnyPackageDirty |= CurrentProfiles.Drivetrain->MarkPackageDirty();
		bAnyPackageDirty |= CurrentProfiles.Handling->MarkPackageDirty();
		bAnyPackageDirty |= CurrentProfiles.Performance->MarkPackageDirty();
	}

	OutResult = FreshPreview.Operation;
	OutResult.OperationName = CommitOperationName;
	OutResult.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
	OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
	OutResult.Mutation.bProfileChanged = bProfilePayloadChanged;
	OutResult.Mutation.bPackageDirty = bAnyPackageDirty || IsPackageDirty(Request.Recipe);
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bTargetChanged = false;
	OutResult.Mutation.bCreatedAssets = false;
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;
	OutResult.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	Succeed(
		OutResult,
		bProfilePayloadChanged
			? TEXT("Builder-private 4 Profile complete payload와 persistent provenance receipt를 한 transaction으로 commit했습니다. VehicleData/OwnerRecipeId는 수정하지 않았고 Save는 수행하지 않았습니다.")
			: TEXT("Profile payload는 유지하고 persistent Builder provenance receipt만 한 transaction으로 기록했습니다. VehicleData/OwnerRecipeId는 수정하지 않았고 Save는 수행하지 않았습니다."));
	return true;
}
