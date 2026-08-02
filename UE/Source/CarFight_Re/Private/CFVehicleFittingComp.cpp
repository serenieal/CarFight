// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-02
// Description: CF-FQ-034 FIT-P0-05 초기 출격 피팅·질량 상태 구현
// Scope: Legacy·Snapshot Prepare, Initial Mass Target·검증, Weapon·Defense 원자 Commit, 실패 Rollback과 Applied Snapshot 수명을 구현합니다.
// Changelog:
// - v1.1.0: Initial Mass Prepare, Legacy fallback, 물리 생성 전 기록, 실제 질량 검증, 같은 질량 Verify Only와 다른 질량 재적용 거부를 추가.
// - v1.0.0: UCFVehicleFittingComp 상태 머신과 실제 차량 Adapter를 최초 구현.
// Migration:
// - 초기 Invalid Snapshot은 아직 Snapshot 질량이 구성되지 않은 수명에서만 Legacy 입력으로 fallback한다.
// - 이미 구성된 Snapshot 질량과 다른 Target은 Physics State Hot Recreate 없이 거부한다.
// - Commit 실패 시 직전 Applied 입력 또는 VehicleData Legacy 입력으로 Weapon·Defense를 복원하지만 물리 질량을 직접 재작성하지 않는다.
// - SetMassOverrideInKg, Physics State 재생성과 Ammo 적용은 수행하지 않는다.

#include "CFVehicleFittingComp.h"

#include "CFVehicleData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"

namespace
{
	/** 실제 차량 Weapon·Defense 컴포넌트를 피팅 원자 적용 계약에 연결하는 Adapter입니다. */
	class FCFVehicleFittingRuntimeApplyAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.0.0] 실제 적용 대상과 하위 Runtime 컴포넌트를 캐시합니다.
		FCFVehicleFittingRuntimeApplyAdapter(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData, UCFVehicleWeaponComp* InVehicleWeaponComp, UCFVehicleDefenseComp* InVehicleDefenseComp)
			: OwnerVehiclePawn(InOwnerVehiclePawn)
			, VehicleData(InVehicleData)
			, VehicleWeaponComp(InVehicleWeaponComp)
			, VehicleDefenseComp(InVehicleDefenseComp)
		{
		}

		// [v1.0.0] Legacy 또는 Snapshot 장비 입력으로 Weapon Runtime을 초기화합니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			bOutWeaponRuntimeReady = false;
			if (!OwnerVehiclePawn || !VehicleData || !VehicleWeaponComp)
			{
				return false;
			}

			if (WeaponInput.UsesLegacyVehicleConfiguration())
			{
				bOutWeaponRuntimeReady = VehicleWeaponComp->InitializeWeaponRuntimeForActiveProfile(OwnerVehiclePawn, VehicleData, WeaponInput.ActiveMountProfileId);
				return true;
			}

			bOutWeaponRuntimeReady = VehicleWeaponComp->InitializeWeaponRuntimeFromFitting(OwnerVehiclePawn, VehicleData, WeaponInput.ActiveMountProfileId, WeaponInput.EquipmentPresetData);
			return WeaponInput.bHasResolvedMount ? bOutWeaponRuntimeReady : true;
		}

		// [v1.0.0] Legacy 또는 Snapshot 방어 입력으로 Defense Runtime을 초기화합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			bOutDefenseRuntimeReady = false;
			if (!VehicleData || !VehicleDefenseComp)
			{
				return false;
			}

			if (DefenseInput.UsesLegacyVehicleConfiguration())
			{
				bOutDefenseRuntimeReady = VehicleDefenseComp->InitializeFromVehicleData(VehicleData);
				return true;
			}

			bOutDefenseRuntimeReady = VehicleDefenseComp->InitializeFromDefenseData(DefenseInput.DefenseData);
			if (!DefenseInput.DefenseData)
			{
				return VehicleDefenseComp->GetActiveDefenseData() == nullptr && !VehicleDefenseComp->IsDefenseInitialized();
			}

			return bOutDefenseRuntimeReady && VehicleDefenseComp->GetActiveDefenseData() == DefenseInput.DefenseData;
		}

	private:
		// [v1.0.0] Weapon Runtime Owner 차량 Pawn입니다.
		TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

		// [v1.0.0] Legacy와 Snapshot 입력의 기준 VehicleData입니다.
		TObjectPtr<UCFVehicleData> VehicleData = nullptr;

		// [v1.0.0] 장비 입력을 적용할 Weapon 컴포넌트입니다.
		TObjectPtr<UCFVehicleWeaponComp> VehicleWeaponComp = nullptr;

		// [v1.0.0] 방어 입력을 적용할 Defense 컴포넌트입니다.
		TObjectPtr<UCFVehicleDefenseComp> VehicleDefenseComp = nullptr;
	};
}

// [v1.0.0] Legacy 또는 Snapshot Runtime 입력 계약이 유효한지 반환합니다.
bool FCFFittingSortieRuntimeInput::IsValid() const
{
	if (!VehicleData)
	{
		return false;
	}

	if (bUseLegacyVehicleConfiguration)
	{
		return WeaponInput.bUseLegacyVehicleConfiguration && DefenseInput.bUseLegacyVehicleConfiguration;
	}

	return FittingSnapshot.IsValid()
		&& FittingSnapshot.VehicleData == VehicleData
		&& !WeaponInput.bUseLegacyVehicleConfiguration
		&& !DefenseInput.bUseLegacyVehicleConfiguration;
}

// [v1.0.0] Tick이 필요 없는 출격 피팅 컴포넌트 기본값을 초기화합니다.
UCFVehicleFittingComp::UCFVehicleFittingComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// [v1.0.0] EndPlay에서 Prepared·Applied 상태를 정리합니다.
void UCFVehicleFittingComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetFittingRuntimeState();
	Super::EndPlay(EndPlayReason);
}

// [v1.0.0] VehicleFittingData를 검증해 아직 적용하지 않은 Runtime 입력을 준비합니다.
bool UCFVehicleFittingComp::PrepareSortieFitting(const UCFVehicleFittingData* InVehicleFittingData, UCFVehicleData* InVehicleData, const FName RequestedActiveMountProfileId)
{
	ClearPreparedRuntimeInput();

	if (!InVehicleData)
	{
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = TEXT("FittingRuntime: PrepareFailed, VehicleData=Missing");
		return false;
	}

	if (!InVehicleFittingData)
	{
		PreparedRuntimeInput = BuildLegacyRuntimeInput(InVehicleData, RequestedActiveMountProfileId);
		bHasPreparedRuntimeInput = PreparedRuntimeInput.IsValid();
		RuntimeApplyState = bHasPreparedRuntimeInput ? ECFFittingRuntimeApplyState::PreparedLegacy : ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = bHasPreparedRuntimeInput ? TEXT("FittingRuntime: PreparedLegacy") : TEXT("FittingRuntime: PrepareLegacyFailed");
		return bHasPreparedRuntimeInput;
	}

	// [v1.0.0] Pawn과 Runtime을 변경하지 않고 생성한 후보 Snapshot입니다.
	const FCFVehicleFittingSnapshot CandidateSnapshot = InVehicleFittingData->BuildFittingSnapshot();
	if (!CandidateSnapshot.IsValid())
	{
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = FString::Printf(TEXT("FittingRuntime: PrepareFailed, Fitting=%s, Issues=%d"), *InVehicleFittingData->FittingId.ToString(), CandidateSnapshot.ValidationIssues.Num());
		return false;
	}

	if (CandidateSnapshot.VehicleData != InVehicleData)
	{
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = TEXT("FittingRuntime: PrepareFailed, VehicleDataMismatch");
		return false;
	}

	// [v1.0.0] Snapshot 입력 변환 실패 요약입니다.
	FString RuntimeInputFailureSummary;
	if (!BuildSnapshotRuntimeInput(CandidateSnapshot, RequestedActiveMountProfileId, PreparedRuntimeInput, RuntimeInputFailureSummary))
	{
		ClearPreparedRuntimeInput();
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = RuntimeInputFailureSummary;
		return false;
	}

	bHasPreparedRuntimeInput = true;
	RuntimeApplyState = ECFFittingRuntimeApplyState::PreparedSnapshot;
	LastFittingRuntimeSummary = FString::Printf(TEXT("FittingRuntime: PreparedSnapshot, Fitting=%s"), *CandidateSnapshot.FittingId.ToString());
	return true;
}

// [v1.1.0] 초기 출격에서 PreRegister와 BeginPlay가 공유할 Runtime 입력과 질량 Target을 준비합니다.
bool UCFVehicleFittingComp::PrepareInitialSortieFitting(const UCFVehicleFittingData* InVehicleFittingData, UCFVehicleData* InVehicleData, const FName RequestedActiveMountProfileId)
{
	ClearPreparedInitialMassInput();

	// [v1.1.0] 현재 Pawn 물리 수명에 이미 Snapshot Mass가 구성됐는지 여부입니다.
	const bool bHasExistingConfiguredMass = bHasConfiguredInitialMass;
	if (!InVehicleFittingData)
	{
		if (bHasExistingConfiguredMass)
		{
			ClearPreparedRuntimeInput();
			InitialMassState = ECFInitialMassState::ReapplyRejected;
			LastInitialMassSummary = FString::Printf(TEXT("InitialMass: ReapplyRejected, Existing=%.3f, Requested=Legacy"), ConfiguredInitialMassKg);
			LastFittingRuntimeSummary = TEXT("FittingRuntime: PrepareFailed, RuntimeMassChangeToLegacyRejected");
			return false;
		}

		// [v1.1.0] FittingData가 없는 출격은 기존 VehicleData와 Chaos 질량을 그대로 사용합니다.
		const bool bLegacyPrepared = PrepareSortieFitting(nullptr, InVehicleData, RequestedActiveMountProfileId);
		if (!bLegacyPrepared)
		{
			InitialMassState = ECFInitialMassState::VerificationFailed;
			LastInitialMassSummary = TEXT("InitialMass: LegacyPrepareFailed");
			return false;
		}

		bPreparedInitialMassUsesLegacy = true;
		InitialMassState = ECFInitialMassState::LegacyPreserved;
		LastInitialMassSummary = TEXT("InitialMass: LegacyPreserved, Reason=FittingDataMissing");
		return true;
	}

	if (!PrepareSortieFitting(InVehicleFittingData, InVehicleData, RequestedActiveMountProfileId))
	{
		if (bHasExistingConfiguredMass)
		{
			ClearPreparedRuntimeInput();
			InitialMassState = ECFInitialMassState::ReapplyRejected;
			LastInitialMassSummary = FString::Printf(TEXT("InitialMass: ReapplyRejected, Existing=%.3f, RequestedSnapshot=Invalid"), ConfiguredInitialMassKg);
			LastFittingRuntimeSummary = TEXT("FittingRuntime: PrepareFailed, InvalidSnapshotAfterMassConfigured");
			return false;
		}

		return FallbackPreparedInitialMassToLegacy(InVehicleData, RequestedActiveMountProfileId, TEXT("InvalidSnapshot"));
	}

	// [v1.1.0] Prepared Runtime 입력에서 읽은 결정론적 Snapshot Target Mass입니다.
	const float CandidateTargetMassKg = PreparedRuntimeInput.FittingSnapshot.TotalVehicleMassKg;
	if (!FMath::IsFinite(CandidateTargetMassKg) || CandidateTargetMassKg <= 0.0f)
	{
		if (bHasExistingConfiguredMass)
		{
			ClearPreparedRuntimeInput();
			InitialMassState = ECFInitialMassState::ReapplyRejected;
			LastInitialMassSummary = TEXT("InitialMass: ReapplyRejected, RequestedTarget=Invalid");
			LastFittingRuntimeSummary = TEXT("FittingRuntime: PrepareFailed, InvalidMassAfterMassConfigured");
			return false;
		}

		return FallbackPreparedInitialMassToLegacy(InVehicleData, RequestedActiveMountProfileId, TEXT("InvalidTargetMass"));
	}

	bPreparedInitialMassUsesLegacy = false;
	bHasPreparedInitialMass = true;
	PreparedInitialMassKg = CandidateTargetMassKg;

	if (bHasExistingConfiguredMass)
	{
		// [v1.1.0] 이미 구성된 질량과 새 Snapshot Target을 비교할 허용 오차입니다.
		const float ReinitializeToleranceKg = CalculateInitialMassToleranceKg(CandidateTargetMassKg);
		if (!FMath::IsNearlyEqual(ConfiguredInitialMassKg, CandidateTargetMassKg, ReinitializeToleranceKg))
		{
			ClearPreparedRuntimeInput();
			ClearPreparedInitialMassInput();
			InitialMassState = ECFInitialMassState::ReapplyRejected;
			LastInitialMassSummary = FString::Printf(TEXT("InitialMass: ReapplyRejected, Existing=%.3f, Requested=%.3f, Tolerance=%.3f"), ConfiguredInitialMassKg, CandidateTargetMassKg, ReinitializeToleranceKg);
			LastFittingRuntimeSummary = TEXT("FittingRuntime: PrepareFailed, RuntimeMassReapplyUnsupported");
			return false;
		}

		bPreparedInitialMassNeedsApply = false;
		bPreparedInitialMassVerifyOnly = true;
		InitialMassState = ECFInitialMassState::PreparedSnapshot;
		LastInitialMassSummary = FString::Printf(TEXT("InitialMass: PreparedVerifyOnly, Target=%.3f"), CandidateTargetMassKg);
		return true;
	}

	bPreparedInitialMassNeedsApply = true;
	bPreparedInitialMassVerifyOnly = false;
	InitialMassState = ECFInitialMassState::PreparedSnapshot;
	LastInitialMassSummary = FString::Printf(TEXT("InitialMass: PreparedForPrePhysicsApply, Target=%.3f"), CandidateTargetMassKg);
	return true;
}

// [v1.1.0] 물리 생성 전 Movement Mass 기록이 필요한 Snapshot 준비 상태인지 반환합니다.
bool UCFVehicleFittingComp::ShouldApplyPreparedInitialMass() const
{
	return bHasPreparedInitialMass
		&& !bPreparedInitialMassUsesLegacy
		&& bPreparedInitialMassNeedsApply
		&& !bPreparedInitialMassVerifyOnly;
}

// [v1.1.0] Target Mass 검증에 사용할 최소 1kg 또는 1% 허용 오차를 계산합니다.
float UCFVehicleFittingComp::CalculateInitialMassToleranceKg(const float TargetMassKg)
{
	if (!FMath::IsFinite(TargetMassKg) || TargetMassKg <= 0.0f)
	{
		return 1.0f;
	}

	return FMath::Max(1.0f, TargetMassKg * 0.01f);
}

// [v1.1.0] Pawn이 물리 생성 전에 기록한 이전·설정 Movement Mass를 상태에 등록합니다.
bool UCFVehicleFittingComp::RecordInitialMassBeforePhysics(const float PreviousMassKg, const float ConfiguredMassKg)
{
	if (bPreparedInitialMassUsesLegacy)
	{
		InitialMassState = ECFInitialMassState::LegacyPreserved;
		LastInitialMassSummary = TEXT("InitialMass: LegacyPreserved, PrePhysicsWrite=Skipped");
		return true;
	}

	if (!bHasPreparedInitialMass || !FMath::IsFinite(PreparedInitialMassKg) || PreparedInitialMassKg <= 0.0f)
	{
		InitialMassState = ECFInitialMassState::VerificationFailed;
		LastInitialMassSummary = TEXT("InitialMass: RecordFailed, PreparedTargetInvalid");
		return false;
	}

	// [v1.1.0] 설정값이 Snapshot Target과 같은지 검사할 허용 오차입니다.
	const float ConfiguredToleranceKg = CalculateInitialMassToleranceKg(PreparedInitialMassKg);
	if (!FMath::IsFinite(ConfiguredMassKg)
		|| ConfiguredMassKg <= 0.0f
		|| !FMath::IsNearlyEqual(ConfiguredMassKg, PreparedInitialMassKg, ConfiguredToleranceKg))
	{
		InitialMassState = ECFInitialMassState::VerificationFailed;
		LastInitialMassSummary = FString::Printf(TEXT("InitialMass: RecordFailed, Target=%.3f, Configured=%.3f, Tolerance=%.3f"), PreparedInitialMassKg, ConfiguredMassKg, ConfiguredToleranceKg);
		return false;
	}

	if (!bPreparedInitialMassVerifyOnly && (!FMath::IsFinite(PreviousMassKg) || PreviousMassKg <= 0.0f))
	{
		InitialMassState = ECFInitialMassState::VerificationFailed;
		LastInitialMassSummary = FString::Printf(TEXT("InitialMass: RecordFailed, PreviousMass=%.3f"), PreviousMassKg);
		return false;
	}

	if (!bHasConfiguredInitialMass)
	{
		PreviousMovementMassKg = PreviousMassKg;
	}

	bHasConfiguredInitialMass = true;
	ConfiguredInitialMassKg = PreparedInitialMassKg;
	bInitialMassVerified = false;
	bPreparedInitialMassNeedsApply = false;
	bPreparedInitialMassVerifyOnly = true;
	InitialMassState = ECFInitialMassState::ConfiguredBeforePhysics;
	LastInitialMassSummary = FString::Printf(TEXT("InitialMass: ConfiguredBeforePhysics, Previous=%.3f, Target=%.3f"), PreviousMovementMassKg, ConfiguredInitialMassKg);
	return true;
}

// [v1.1.0] BeginPlay에서 Movement 설정값과 VehicleMesh 실제 질량·Physics 상태를 검증합니다.
bool UCFVehicleFittingComp::VerifyInitialMassAfterPhysics(const float ConfiguredMovementMassKg, const float ActualVehicleMeshMassKg, const bool bHasPhysicsState, const bool bSimulatesPhysics, const bool bHasPhysicsAsset)
{
	if (bPreparedInitialMassUsesLegacy)
	{
		bInitialMassVerified = true;
		InitialMassState = ECFInitialMassState::LegacyPreserved;
		LastInitialMassSummary = FString::Printf(TEXT("InitialMass: LegacyVerified, Configured=%.3f, Actual=%.3f"), ConfiguredMovementMassKg, ActualVehicleMeshMassKg);
		return true;
	}

	if (!bHasPreparedInitialMass || !bHasConfiguredInitialMass)
	{
		bInitialMassVerified = false;
		InitialMassState = ECFInitialMassState::VerificationFailed;
		LastInitialMassSummary = TEXT("InitialMass: VerifyFailed, PreparedOrConfiguredTargetMissing");
		return false;
	}

	// [v1.1.0] Configured·Actual 값을 Target과 비교할 공통 허용 오차입니다.
	const float MassToleranceKg = CalculateInitialMassToleranceKg(PreparedInitialMassKg);
	// [v1.1.0] Movement 설정값이 Target 허용 오차 안인지 여부입니다.
	const bool bConfiguredMassMatches = FMath::IsFinite(ConfiguredMovementMassKg)
		&& ConfiguredMovementMassKg > 0.0f
		&& FMath::IsNearlyEqual(ConfiguredMovementMassKg, PreparedInitialMassKg, MassToleranceKg);
	// [v1.1.0] VehicleMesh 실제 질량이 Target 허용 오차 안인지 여부입니다.
	const bool bActualMassMatches = FMath::IsFinite(ActualVehicleMeshMassKg)
		&& ActualVehicleMeshMassKg > 0.0f
		&& FMath::IsNearlyEqual(ActualVehicleMeshMassKg, PreparedInitialMassKg, MassToleranceKg);
	// [v1.1.0] 실제 Chaos Body 검증에 필요한 모든 물리 상태가 유효한지 여부입니다.
	const bool bPhysicsContractValid = bHasPhysicsState && bSimulatesPhysics && bHasPhysicsAsset;

	bInitialMassVerified = bConfiguredMassMatches && bActualMassMatches && bPhysicsContractValid;
	InitialMassState = bInitialMassVerified ? ECFInitialMassState::Verified : ECFInitialMassState::VerificationFailed;
	LastInitialMassSummary = FString::Printf(
		TEXT("InitialMass: Verify=%s, Target=%.3f, Configured=%.3f, Actual=%.3f, Tolerance=%.3f, PhysicsState=%s, Simulate=%s, PhysicsAsset=%s"),
		bInitialMassVerified ? TEXT("Passed") : TEXT("Failed"),
		PreparedInitialMassKg,
		ConfiguredMovementMassKg,
		ActualVehicleMeshMassKg,
		MassToleranceKg,
		bHasPhysicsState ? TEXT("Yes") : TEXT("No"),
		bSimulatesPhysics ? TEXT("Yes") : TEXT("No"),
		bHasPhysicsAsset ? TEXT("Yes") : TEXT("No"));
	return bInitialMassVerified;
}

// [v1.1.0] 물리 생성 전 Snapshot Mass 적용 실패를 Legacy Runtime 입력으로 되돌립니다.
bool UCFVehicleFittingComp::FallbackPreparedInitialMassToLegacy(UCFVehicleData* InVehicleData, const FName RequestedActiveMountProfileId, const FString& FailureReason)
{
	if (bHasConfiguredInitialMass)
	{
		ClearPreparedRuntimeInput();
		ClearPreparedInitialMassInput();
		InitialMassState = ECFInitialMassState::ReapplyRejected;
		LastInitialMassSummary = FString::Printf(TEXT("InitialMass: LegacyFallbackRejected, Existing=%.3f, Reason=%s"), ConfiguredInitialMassKg, *FailureReason);
		LastFittingRuntimeSummary = TEXT("FittingRuntime: LegacyFallbackRejectedAfterMassConfigured");
		return false;
	}

	ClearPreparedRuntimeInput();
	ClearPreparedInitialMassInput();
	PreparedRuntimeInput = BuildLegacyRuntimeInput(InVehicleData, RequestedActiveMountProfileId);
	bHasPreparedRuntimeInput = PreparedRuntimeInput.IsValid();
	RuntimeApplyState = bHasPreparedRuntimeInput ? ECFFittingRuntimeApplyState::PreparedLegacy : ECFFittingRuntimeApplyState::ApplyFailed;
	bPreparedInitialMassUsesLegacy = true;
	InitialMassState = bHasPreparedRuntimeInput ? ECFInitialMassState::LegacyPreserved : ECFInitialMassState::VerificationFailed;
	LastInitialMassSummary = FString::Printf(TEXT("InitialMass: LegacyFallback=%s, Reason=%s"), bHasPreparedRuntimeInput ? TEXT("Prepared") : TEXT("Failed"), *FailureReason);
	LastFittingRuntimeSummary = FString::Printf(TEXT("FittingRuntime: %s, InitialMassFallbackReason=%s"), bHasPreparedRuntimeInput ? TEXT("PreparedLegacyFallback") : TEXT("PrepareLegacyFallbackFailed"), *FailureReason);
	return bHasPreparedRuntimeInput;
}

// [v1.0.0] 준비된 입력을 원자 적용하고 실패 시 직전 입력으로 복원합니다.
bool UCFVehicleFittingComp::CommitPreparedSortieFitting(ICFFittingRuntimeApplyAdapter& RuntimeApplyAdapter)
{
	if (!bHasPreparedRuntimeInput || !PreparedRuntimeInput.IsValid())
	{
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = TEXT("FittingRuntime: CommitFailed, PreparedInputInvalid");
		return false;
	}

	// [v1.0.0] 부분 적용 실패 시 되돌릴 직전 입력입니다.
	const FCFFittingSortieRuntimeInput PreviousRuntimeInput = bHasAppliedRuntimeInput
		? AppliedRuntimeInput
		: BuildLegacyRuntimeInput(PreparedRuntimeInput.VehicleData, PreparedRuntimeInput.WeaponInput.ActiveMountProfileId);

	// [v1.0.0] 후보 Weapon 입력 적용 결과입니다.
	bool bCandidateWeaponRuntimeReady = false;
	if (!RuntimeApplyAdapter.ApplyWeaponRuntime(PreparedRuntimeInput.WeaponInput, bCandidateWeaponRuntimeReady))
	{
		bool bRollbackWeaponRuntimeReady = false;
		bool bRollbackDefenseRuntimeReady = false;
		const bool bRollbackSucceeded = RestoreRuntimeInput(RuntimeApplyAdapter, PreviousRuntimeInput, bRollbackWeaponRuntimeReady, bRollbackDefenseRuntimeReady);
				ClearPreparedRuntimeInput();
		ClearPreparedInitialMassInput();
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = FString::Printf(TEXT("FittingRuntime: CommitFailedAtWeapon, Rollback=%s"), bRollbackSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
		return false;
	}

	// [v1.0.0] 후보 Defense 입력 적용 결과입니다.
	bool bCandidateDefenseRuntimeReady = false;
	if (!RuntimeApplyAdapter.ApplyDefenseRuntime(PreparedRuntimeInput.DefenseInput, bCandidateDefenseRuntimeReady))
	{
		bool bRollbackWeaponRuntimeReady = false;
		bool bRollbackDefenseRuntimeReady = false;
		const bool bRollbackSucceeded = RestoreRuntimeInput(RuntimeApplyAdapter, PreviousRuntimeInput, bRollbackWeaponRuntimeReady, bRollbackDefenseRuntimeReady);
				ClearPreparedRuntimeInput();
		ClearPreparedInitialMassInput();
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = FString::Printf(TEXT("FittingRuntime: CommitFailedAtDefense, Rollback=%s"), bRollbackSucceeded ? TEXT("Succeeded") : TEXT("Failed"));
		return false;
	}

	AppliedRuntimeInput = PreparedRuntimeInput;
	bHasAppliedRuntimeInput = true;
	bLastWeaponRuntimeReady = bCandidateWeaponRuntimeReady;
	bLastDefenseRuntimeReady = bCandidateDefenseRuntimeReady;

	if (AppliedRuntimeInput.UsesLegacyVehicleConfiguration())
	{
		bHasAppliedFittingSnapshot = false;
		AppliedFittingSnapshot = FCFVehicleFittingSnapshot();
		RuntimeApplyState = ECFFittingRuntimeApplyState::CommittedLegacy;
	}
	else
	{
		bHasAppliedFittingSnapshot = true;
		AppliedFittingSnapshot = AppliedRuntimeInput.FittingSnapshot;
		RuntimeApplyState = ECFFittingRuntimeApplyState::CommittedSnapshot;
	}

		LastFittingRuntimeSummary = FString::Printf(TEXT("FittingRuntime: CommitSucceeded, Mode=%s, Fitting=%s, WeaponReady=%s, DefenseReady=%s"), AppliedRuntimeInput.UsesLegacyVehicleConfiguration() ? TEXT("Legacy") : TEXT("Snapshot"), bHasAppliedFittingSnapshot ? *AppliedFittingSnapshot.FittingId.ToString() : TEXT("None"), bLastWeaponRuntimeReady ? TEXT("Yes") : TEXT("No"), bLastDefenseRuntimeReady ? TEXT("Yes") : TEXT("No"));
	ClearPreparedRuntimeInput();
	ClearPreparedInitialMassInput();
	return true;
}

// [v1.0.0] 실제 차량 컴포넌트에 준비된 출격 피팅을 Commit합니다.
bool UCFVehicleFittingComp::CommitPreparedSortieFittingToVehicle(ACFVehiclePawn* OwnerVehiclePawn, UCFVehicleWeaponComp* VehicleWeaponComp, UCFVehicleDefenseComp* VehicleDefenseComp)
{
	if (!bHasPreparedRuntimeInput || !PreparedRuntimeInput.VehicleData)
	{
		RuntimeApplyState = ECFFittingRuntimeApplyState::ApplyFailed;
		LastFittingRuntimeSummary = TEXT("FittingRuntime: VehicleCommitFailed, PreparedInputMissing");
		return false;
	}

	FCFVehicleFittingRuntimeApplyAdapter RuntimeApplyAdapter(OwnerVehiclePawn, PreparedRuntimeInput.VehicleData, VehicleWeaponComp, VehicleDefenseComp);
	return CommitPreparedSortieFitting(RuntimeApplyAdapter);
}

// [v1.0.0] Prepared 입력만 취소합니다.
void UCFVehicleFittingComp::RollbackPreparedSortieFitting()
{
		const bool bHadPreparedRuntimeInput = bHasPreparedRuntimeInput;
	ClearPreparedRuntimeInput();
	ClearPreparedInitialMassInput();
	RuntimeApplyState = ECFFittingRuntimeApplyState::RolledBack;
	LastFittingRuntimeSummary = bHadPreparedRuntimeInput ? TEXT("FittingRuntime: PreparedInputRolledBack") : TEXT("FittingRuntime: RollbackNoPreparedInput");
}

// [v1.0.0] Prepared·Applied 상태와 Snapshot을 모두 초기화합니다.
void UCFVehicleFittingComp::ResetFittingRuntimeState()
{
		ClearPreparedRuntimeInput();
	ClearPreparedInitialMassInput();
	bHasAppliedRuntimeInput = false;
	AppliedRuntimeInput = FCFFittingSortieRuntimeInput();
	bHasAppliedFittingSnapshot = false;
	AppliedFittingSnapshot = FCFVehicleFittingSnapshot();
		RuntimeApplyState = ECFFittingRuntimeApplyState::Uninitialized;
	InitialMassState = ECFInitialMassState::Uninitialized;
	PreviousMovementMassKg = 0.0f;
	bHasConfiguredInitialMass = false;
	ConfiguredInitialMassKg = 0.0f;
	bInitialMassVerified = false;
	LastInitialMassSummary = TEXT("InitialMass: Reset");
	bLastWeaponRuntimeReady = false;
	bLastDefenseRuntimeReady = false;
	LastFittingRuntimeSummary = TEXT("FittingRuntime: Reset");
}

// [v1.0.0] VehicleFittingData가 없을 때 Legacy Runtime 입력을 생성합니다.
FCFFittingSortieRuntimeInput UCFVehicleFittingComp::BuildLegacyRuntimeInput(UCFVehicleData* InVehicleData, const FName RequestedActiveMountProfileId)
{
	FCFFittingSortieRuntimeInput RuntimeInput;
	RuntimeInput.bUseLegacyVehicleConfiguration = true;
	RuntimeInput.VehicleData = InVehicleData;
	RuntimeInput.WeaponInput.bUseLegacyVehicleConfiguration = true;
	RuntimeInput.WeaponInput.ActiveMountProfileId = RequestedActiveMountProfileId;
	RuntimeInput.DefenseInput.bUseLegacyVehicleConfiguration = true;
	RuntimeInput.DefenseInput.SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;
	RuntimeInput.DefenseInput.DefenseData = InVehicleData ? InVehicleData->DefaultDefenseData : nullptr;
	return RuntimeInput;
}

// [v1.0.0] 유효 Snapshot을 Weapon·Defense 입력으로 변환합니다.
bool UCFVehicleFittingComp::BuildSnapshotRuntimeInput(const FCFVehicleFittingSnapshot& FittingSnapshot, const FName RequestedActiveMountProfileId, FCFFittingSortieRuntimeInput& OutRuntimeInput, FString& OutFailureSummary)
{
	OutRuntimeInput = FCFFittingSortieRuntimeInput();
	OutFailureSummary.Reset();

	if (!FittingSnapshot.IsValid() || !FittingSnapshot.VehicleData)
	{
		OutFailureSummary = TEXT("FittingRuntime: BuildRuntimeInputFailed, SnapshotInvalid");
		return false;
	}

	OutRuntimeInput.bUseLegacyVehicleConfiguration = false;
	OutRuntimeInput.VehicleData = FittingSnapshot.VehicleData;
	OutRuntimeInput.FittingSnapshot = FittingSnapshot;
	OutRuntimeInput.WeaponInput.bUseLegacyVehicleConfiguration = false;
	OutRuntimeInput.DefenseInput.bUseLegacyVehicleConfiguration = false;
	OutRuntimeInput.DefenseInput.SelectionMode = FittingSnapshot.ResolvedDefenseSelectionMode;
	OutRuntimeInput.DefenseInput.DefenseData = FittingSnapshot.ResolvedDefenseData;

	// [v1.0.0] 기존 Weapon 활성 프로파일과 일치하는 Snapshot 장착입니다.
	const FCFResolvedFittingMount* ResolvedActiveMount = nullptr;
	if (!RequestedActiveMountProfileId.IsNone())
	{
		ResolvedActiveMount = FittingSnapshot.ResolvedMounts.FindByPredicate([RequestedActiveMountProfileId](const FCFResolvedFittingMount& ResolvedMount)
		{
			return ResolvedMount.MountProfileId == RequestedActiveMountProfileId;
		});

		if (!ResolvedActiveMount && !FittingSnapshot.ResolvedMounts.IsEmpty())
		{
			OutFailureSummary = FString::Printf(TEXT("FittingRuntime: ActiveMountProfileMissing, Mount=%s"), *RequestedActiveMountProfileId.ToString());
			return false;
		}
	}
	else if (!FittingSnapshot.ResolvedMounts.IsEmpty())
	{
		ResolvedActiveMount = &FittingSnapshot.ResolvedMounts[0];
	}

	if (ResolvedActiveMount)
	{
		OutRuntimeInput.WeaponInput.bHasResolvedMount = true;
		OutRuntimeInput.WeaponInput.ActiveMountProfileId = ResolvedActiveMount->MountProfileId;
		OutRuntimeInput.WeaponInput.EquipmentPresetData = ResolvedActiveMount->EquipmentPresetData;
		OutRuntimeInput.WeaponInput.SelectionSource = ResolvedActiveMount->SelectionSource;
	}

	if (!OutRuntimeInput.IsValid())
	{
		OutFailureSummary = TEXT("FittingRuntime: BuildRuntimeInputFailed, ContractInvalid");
		return false;
	}

	return true;
}

// [v1.0.0] 직전 입력을 Weapon·Defense Adapter에 다시 적용합니다.
bool UCFVehicleFittingComp::RestoreRuntimeInput(ICFFittingRuntimeApplyAdapter& RuntimeApplyAdapter, const FCFFittingSortieRuntimeInput& RuntimeInput, bool& bOutWeaponRuntimeReady, bool& bOutDefenseRuntimeReady)
{
	bOutWeaponRuntimeReady = false;
	bOutDefenseRuntimeReady = false;
	if (!RuntimeInput.IsValid())
	{
		return false;
	}

	const bool bWeaponRestored = RuntimeApplyAdapter.ApplyWeaponRuntime(RuntimeInput.WeaponInput, bOutWeaponRuntimeReady);
	const bool bDefenseRestored = RuntimeApplyAdapter.ApplyDefenseRuntime(RuntimeInput.DefenseInput, bOutDefenseRuntimeReady);
	return bWeaponRestored && bDefenseRestored;
}

// [v1.0.0] Prepared 입력만 기본값으로 정리합니다.
void UCFVehicleFittingComp::ClearPreparedRuntimeInput()
{
	bHasPreparedRuntimeInput = false;
	PreparedRuntimeInput = FCFFittingSortieRuntimeInput();
}

// [v1.1.0] 현재 Prepared Initial Mass Target과 Legacy·Verify Only 플래그를 정리합니다.
void UCFVehicleFittingComp::ClearPreparedInitialMassInput()
{
	bPreparedInitialMassUsesLegacy = true;
	bHasPreparedInitialMass = false;
	bPreparedInitialMassNeedsApply = false;
	bPreparedInitialMassVerifyOnly = false;
	PreparedInitialMassKg = 0.0f;
}
