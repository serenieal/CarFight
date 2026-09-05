// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-04
// Description: CF-FQ-041 Runtime Equipment Apply + CF-FQ-047 Standard Mount active identity handoff 구현
// Scope: transient VehicleFittingData 후보, Fitting/Mass checkpoint, 장비 의존 Runtime refresh, 실패 복구와 final readback을 소유합니다.
// Changelog:
// - v1.2.0: CF-FQ-047 mid-review correction. 기존 active weapon이 candidate에서 사라졌는데 USER target이 weapon-bearing이 아니면 Fitting Prepare 전에 ValidationFailed로 명시 거부합니다.
// - v1.1.0: CF-FQ-047 P0 correction. 기존 Weapon active Mount가 candidate weapon-bearing Snapshot에 없고 USER target Mount가 유효한 무기 Mount이면 candidate active identity를 exact target으로 전환합니다.
// - v1.0.0: Runtime Test Catalog authorization, 단일 Mount 교체, Snapshot validation, Fitting+Mass 적용·복구와 readback을 최초 구현.
// Migration:
// - Inventory Ledger/FieldFit Inventory transaction을 위조하지 않습니다. 기존 Fitting Runtime authority와 Chaos Mass authority만 재사용합니다.
// - 현재 VehicleFittingData가 같은 VehicleData를 가리키면 선택/탄약/방어를 복사하고, 아니면 VehicleData 기본값에서 transient 후보를 구성합니다.
// - 후보 또는 복구 Refresh는 Applied Snapshot의 InitialSortieAmmoLoads 기준으로 Ammo Runtime을 재구성하며 현재 전투 중 잔탄 상태를 checkpoint하지 않습니다.

#include "CFRuntimeEquipApply.h"
#include "CFRuntimeEquipApplyPolicy.h"

#include "CFEquipmentPresetData.h"
#include "CFFieldFitCoordinator.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	// [v1.0.0] 질량 readback 비교에서 소수 오차만 허용하는 Runtime Apply 허용 오차입니다.
	constexpr float RuntimeMassToleranceKg = 0.01f;

	// [v1.0.0] mutation 동안 이전 Fitting UObject와 Fitting Runtime/Mass 상태를 보존하는 operation checkpoint입니다.
	struct FCFRuntimeEquipCheckpoint
	{
		// 적용 전 Pawn.VehicleFittingData UObject를 operation 종료까지 강하게 보존합니다.
		TStrongObjectPtr<UCFVehicleFittingData> PreviousVehicleFittingData;

		// 적용 전 FittingComp의 Weapon·Defense·Sensor Applied Runtime 상태입니다.
		FCFFittingRuntimeCheckpoint PreviousFittingRuntimeCheckpoint;

		// 적용 전 Chaos VehicleMovement에 설정된 차량 질량입니다.
		float PreviousConfiguredMassKg = 0.0f;
	};

	// [v1.0.0] 현재 Pawn의 Chaos VehicleMovement 설정 질량을 안전하게 읽습니다.
	bool TryGetConfiguredMassKg(const ACFVehiclePawn* VehiclePawn, float& OutConfiguredMassKg)
	{
		OutConfiguredMassKg = 0.0f;
		if (!IsValid(VehiclePawn))
		{
			return false;
		}

		// 현재 Pawn의 inherited VehicleMovement를 실제 Chaos 질량 SSOT로 해석한 컴포넌트입니다.
		const UChaosWheeledVehicleMovementComponent* VehicleMovementComponent =
			Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent());
		if (!IsValid(VehicleMovementComponent)
			|| !FMath::IsFinite(VehicleMovementComponent->Mass)
			|| VehicleMovementComponent->Mass <= 0.0f)
		{
			return false;
		}

		OutConfiguredMassKg = VehicleMovementComponent->Mass;
		return true;
	}

	// [v1.0.0] 현재 Applied Fitting Snapshot의 대상 Mount에서 실제 적용 장비를 찾아 반환합니다.
	UCFEquipmentPresetData* FindAppliedEquipmentForMount(
		const UCFVehicleFittingComp* VehicleFittingComp,
		const FName TargetMountProfileId)
	{
		if (!IsValid(VehicleFittingComp)
			|| TargetMountProfileId.IsNone()
			|| !VehicleFittingComp->HasAppliedFittingSnapshot())
		{
			return nullptr;
		}

		// 현재 FittingComp가 authoritative readback으로 보유한 Applied Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehicleFittingComp->GetAppliedFittingSnapshot();
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == TargetMountProfileId)
			{
				return ResolvedMount.EquipmentPresetData;
			}
		}

		return nullptr;
	}

	// [v1.0.0] Pawn/Fitting/Mass/대상 Mount 장비의 최종 Runtime 상태를 결과에 다시 읽습니다.
	void CaptureEquipmentRuntimeReadback(
		ACFVehiclePawn* VehiclePawn,
		const FName TargetMountProfileId,
		const UCFVehicleFittingData* ExpectedTransientFittingData,
		FCFRuntimeEquipApplyResult& InOutResult)
	{
		if (!IsValid(VehiclePawn))
		{
			InOutResult.CurrentVehicleFittingDataPath = TEXT("None");
			InOutResult.CurrentEquipmentPath = TEXT("None");
			InOutResult.CurrentConfiguredMassKg = 0.0f;
			InOutResult.bRuntimeReady = false;
			InOutResult.bAppliedTransientFittingActive = false;
			InOutResult.FittingRuntimeSummary = TEXT("FittingRuntime: PawnInvalid");
			InOutResult.RuntimeSummary = TEXT("VehicleRuntimeReadback: PawnInvalid");
			return;
		}

		// 현재 Pawn이 source pointer로 보유한 VehicleFittingData입니다.
		UCFVehicleFittingData* CurrentVehicleFittingData = VehiclePawn->VehicleFittingData.Get();

		// 현재 Applied Snapshot의 대상 Mount에 실제 연결된 EquipmentPresetData입니다.
		UCFEquipmentPresetData* CurrentEquipmentPresetData =
			FindAppliedEquipmentForMount(VehiclePawn->GetVehicleFittingComp(), TargetMountProfileId);

		// 공개 VehicleDebug 계약에서 현재 코어 Runtime과 마지막 runtime summary를 다시 읽은 값입니다.
		const FCFVehicleDebugRuntime RuntimeReadback = VehiclePawn->GetVehicleDebugRuntime();

		// 현재 VehicleMovement의 설정 질량을 operation 결과에 남길 readback입니다.
		float CurrentConfiguredMassKg = 0.0f;
		TryGetConfiguredMassKg(VehiclePawn, CurrentConfiguredMassKg);

		InOutResult.CurrentVehicleFittingDataPath = GetPathNameSafe(CurrentVehicleFittingData);
		InOutResult.CurrentEquipmentPath = GetPathNameSafe(CurrentEquipmentPresetData);
		InOutResult.CurrentConfiguredMassKg = CurrentConfiguredMassKg;
		InOutResult.bRuntimeReady = RuntimeReadback.bRuntimeReady && VehiclePawn->bVehicleCombatRuntimeReady;
		InOutResult.RuntimeSummary = RuntimeReadback.RuntimeSummary;

		// 현재 FittingComp의 마지막 Prepare/Commit/Restore 결과 요약입니다.
		const UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
		InOutResult.FittingRuntimeSummary = IsValid(VehicleFittingComp)
			? VehicleFittingComp->GetLastFittingRuntimeSummary()
			: TEXT("FittingRuntime: ComponentMissing");

		InOutResult.bAppliedTransientFittingActive = ExpectedTransientFittingData
			&& CurrentVehicleFittingData == ExpectedTransientFittingData
			&& CurrentVehicleFittingData->HasAnyFlags(RF_Transient)
			&& CurrentVehicleFittingData->GetOutermost() == GetTransientPackage();
	}

	// [v1.0.0] 대상 Mount의 현재 선택을 교체하면서 나머지 Fitting 선택을 그대로 보존합니다.
	void ReplaceTargetMountSelection(
		UCFVehicleFittingData* CandidateFittingData,
		const FName TargetMountProfileId,
		UCFEquipmentPresetData* CandidateEquipmentPresetData)
	{
		if (!IsValid(CandidateFittingData))
		{
			return;
		}

		for (FCFVehicleMountSelection& MountSelection : CandidateFittingData->MountSelections)
		{
			if (MountSelection.MountProfileId == TargetMountProfileId)
			{
				MountSelection.EquipmentPresetData = CandidateEquipmentPresetData;
				MountSelection.bEnabled = true;
				return;
			}
		}

		// 기존 선택에 없던 MountProfile을 명시적 활성 장착으로 추가하는 새 선택입니다.
		FCFVehicleMountSelection NewMountSelection;
		NewMountSelection.MountProfileId = TargetMountProfileId;
		NewMountSelection.EquipmentPresetData = CandidateEquipmentPresetData;
		NewMountSelection.bEnabled = true;
		CandidateFittingData->MountSelections.Add(NewMountSelection);
	}

	// [v1.0.0] 현재 VehicleData/Fitting을 바탕으로 저장하지 않는 단일 Mount 교체 후보를 만들고 Snapshot까지 검증합니다.
	bool BuildTransientEquipmentCandidate(
		ACFVehiclePawn* VehiclePawn,
		const FName TargetMountProfileId,
		UCFEquipmentPresetData* CandidateEquipmentPresetData,
		UCFVehicleFittingData*& OutTransientFittingData,
		FCFVehicleFittingSnapshot& OutCandidateFittingSnapshot,
		FString& OutFailureReason)
	{
		OutTransientFittingData = nullptr;
		OutCandidateFittingSnapshot = FCFVehicleFittingSnapshot();
		OutFailureReason.Reset();

		if (!IsValid(VehiclePawn)
			|| !IsValid(VehiclePawn->VehicleData)
			|| TargetMountProfileId.IsNone()
			|| !IsValid(CandidateEquipmentPresetData))
		{
			OutFailureReason = TEXT("장비 Runtime 후보를 구성할 Pawn/VehicleData/MountProfile/EquipmentPresetData 입력이 유효하지 않습니다.");
			return false;
		}

		// 현재 Pawn source pointer의 FittingData가 동일 VehicleData에 속하면 모든 기존 선택을 보존할 원본입니다.
		UCFVehicleFittingData* CurrentVehicleFittingData = VehiclePawn->VehicleFittingData.Get();

		// 저장 Asset을 직접 변경하지 않기 위한 unique transient Fitting UObject 이름입니다.
		const FName TransientFittingDataName = MakeUniqueObjectName(
			GetTransientPackage(),
			UCFVehicleFittingData::StaticClass(),
			TEXT("RTA_EquipFitting"));

		if (IsValid(CurrentVehicleFittingData)
			&& CurrentVehicleFittingData->VehicleData == VehiclePawn->VehicleData)
		{
			OutTransientFittingData = DuplicateObject<UCFVehicleFittingData>(
				CurrentVehicleFittingData,
				GetTransientPackage(),
				TransientFittingDataName);
		}
		else
		{
			OutTransientFittingData = NewObject<UCFVehicleFittingData>(
				GetTransientPackage(),
				UCFVehicleFittingData::StaticClass(),
				TransientFittingDataName,
				RF_Transient);
			if (IsValid(OutTransientFittingData))
			{
				OutTransientFittingData->VehicleData = VehiclePawn->VehicleData.Get();
				OutTransientFittingData->MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;
			}
		}

		if (!IsValid(OutTransientFittingData))
		{
			OutFailureReason = TEXT("장비 Runtime용 transient VehicleFittingData 생성에 실패했습니다.");
			return false;
		}

		OutTransientFittingData->ClearFlags(RF_Public | RF_Standalone);
		OutTransientFittingData->SetFlags(RF_Transient);
		OutTransientFittingData->FittingId = TEXT("RTA_RuntimeEquip");
		OutTransientFittingData->DisplayName = FText::FromString(TEXT("RTA Runtime Equipment Candidate"));
		OutTransientFittingData->VehicleData = VehiclePawn->VehicleData.Get();

		ReplaceTargetMountSelection(
			OutTransientFittingData,
			TargetMountProfileId,
			CandidateEquipmentPresetData);

		OutCandidateFittingSnapshot = OutTransientFittingData->BuildFittingSnapshot();
		if (!OutCandidateFittingSnapshot.IsValid())
		{
			FString IssueSummary = TEXT("<none>");
			if (!OutCandidateFittingSnapshot.ValidationIssues.IsEmpty())
			{
				const FCFFittingValidationIssue& FirstIssue = OutCandidateFittingSnapshot.ValidationIssues[0];
				IssueSummary = FString::Printf(
					TEXT("Code=%d Mount=%s Message=%s"),
					static_cast<int32>(FirstIssue.IssueCode),
					*FirstIssue.MountProfileId.ToString(),
					*FirstIssue.Message.ToString());
			}
			OutFailureReason = FString::Printf(
				TEXT("장비 Runtime 후보 Fitting Snapshot 검증에 실패했습니다. Issues=%d FirstIssue=[%s]"),
				OutCandidateFittingSnapshot.ValidationIssues.Num(),
				*IssueSummary);
			return false;
		}

		if (OutCandidateFittingSnapshot.VehicleData != VehiclePawn->VehicleData.Get())
		{
			OutFailureReason = TEXT("장비 Runtime 후보 Snapshot의 VehicleData가 현재 Pawn VehicleData와 일치하지 않습니다.");
			return false;
		}

		// Snapshot이 대상 Mount를 요청한 exact EquipmentPresetData로 해석했는지 여부입니다.
		bool bTargetEquipmentResolved = false;
		for (const FCFResolvedFittingMount& ResolvedMount : OutCandidateFittingSnapshot.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == TargetMountProfileId)
			{
				bTargetEquipmentResolved = ResolvedMount.EquipmentPresetData == CandidateEquipmentPresetData;
				break;
			}
		}

		if (!bTargetEquipmentResolved)
		{
			OutFailureReason = FString::Printf(
				TEXT("장비 Runtime 후보가 대상 MountProfile에 요청 장비를 해석하지 못했습니다. Mount=%s"),
				*TargetMountProfileId.ToString());
			return false;
		}

		if (!FMath::IsFinite(OutCandidateFittingSnapshot.TotalVehicleMassKg)
			|| OutCandidateFittingSnapshot.TotalVehicleMassKg <= 0.0f)
		{
			OutFailureReason = TEXT("장비 Runtime 후보 Snapshot의 TotalVehicleMassKg가 유효하지 않습니다.");
			return false;
		}

		return true;
	}

	// [v1.0.0] 성공한 후보 mutation 뒤 Fitting/Mass/source pointer와 장비 의존 Runtime을 직전 checkpoint로 복구합니다.
	bool RecoverPreviousEquipmentRuntime(
		ACFVehiclePawn* VehiclePawn,
		const FCFRuntimeEquipCheckpoint& RuntimeCheckpoint,
		FString& OutRecoverySummary)
	{
		OutRecoverySummary.Reset();
		if (!IsValid(VehiclePawn))
		{
			OutRecoverySummary = TEXT("RecoveryFailed: VehiclePawnInvalid");
			return false;
		}

		// 후보 Fitting Commit을 직전 Weapon·Defense·Sensor Applied Runtime으로 복원할 컴포넌트입니다.
		UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();

		// 기존 Fitting vehicle-facing Adapter가 요구하는 Weapon Runtime 컴포넌트입니다.
		UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();

		// 기존 Fitting vehicle-facing Adapter가 요구하는 Defense Runtime 컴포넌트입니다.
		UCFVehicleDefenseComp* VehicleDefenseComp = VehiclePawn->GetVehicleDefenseComp();
		if (!IsValid(VehicleFittingComp) || !IsValid(VehicleWeaponComp) || !IsValid(VehicleDefenseComp))
		{
			OutRecoverySummary = TEXT("RecoveryFailed: FittingOrCombatComponentMissing");
			return false;
		}

		// 직전 Fitting Applied Runtime과 Applied Snapshot 상태를 기존 authority로 복원한 결과입니다.
		const bool bFittingRestoreSucceeded = VehicleFittingComp->RestoreAppliedRuntimeCheckpointToVehicle(
			VehiclePawn,
			VehicleWeaponComp,
			VehicleDefenseComp,
			RuntimeCheckpoint.PreviousFittingRuntimeCheckpoint);

		// 직전 source-level VehicleFittingData 포인터를 후보 transient pointer 대신 복원합니다.
		VehiclePawn->VehicleFittingData = RuntimeCheckpoint.PreviousVehicleFittingData.Get();

		// Fitting 적용 실패 과정에서 질량이 실제로 변경됐는지 판단할 현재 설정 질량입니다.
		float CurrentConfiguredMassKg = 0.0f;
		const bool bCurrentMassReadable = TryGetConfiguredMassKg(VehiclePawn, CurrentConfiguredMassKg);

		// 이미 이전 질량이면 불필요한 Physics State 재생성을 생략한 질량 복구 성공 상태입니다.
		bool bMassRestoreSucceeded = bCurrentMassReadable
			&& FMath::IsNearlyEqual(
				CurrentConfiguredMassKg,
				RuntimeCheckpoint.PreviousConfiguredMassKg,
				RuntimeMassToleranceKg);

		// 후보 질량이 실제 남아 있을 때만 기존 Chaos mass reapply authority로 이전 질량을 복원합니다.
		FString MassRestoreFailureSummary;
		if (!bMassRestoreSucceeded)
		{
			// 실제 CarFight Chaos VehicleMovement의 질량을 보상 복구할 Adapter입니다.
			FCFChaosVehicleMassRuntime MassRuntime(VehiclePawn);
			bMassRestoreSucceeded = MassRuntime.ReapplyVehicleMassKg(
				RuntimeCheckpoint.PreviousConfiguredMassKg,
				MassRestoreFailureSummary);
		}

		// 복구된 Fitting Snapshot 기준 Ammo·TurretVisual·Launcher를 다시 연결한 결과입니다.
		const bool bDependentRuntimeRefreshed = bFittingRestoreSucceeded
			&& bMassRestoreSucceeded
			&& VehiclePawn->RefreshFittingDependentRuntime();

		// source pointer까지 정확히 직전 FittingData identity로 돌아왔는지 여부입니다.
		const bool bSourcePointerRestored =
			VehiclePawn->VehicleFittingData.Get() == RuntimeCheckpoint.PreviousVehicleFittingData.Get();

		// 복구 뒤 Chaos 설정 질량의 최종 readback입니다.
		float RecoveredConfiguredMassKg = 0.0f;
		const bool bRecoveredMassMatches = TryGetConfiguredMassKg(VehiclePawn, RecoveredConfiguredMassKg)
			&& FMath::IsNearlyEqual(
				RecoveredConfiguredMassKg,
				RuntimeCheckpoint.PreviousConfiguredMassKg,
				RuntimeMassToleranceKg);

		const bool bRecoverySucceeded = bFittingRestoreSucceeded
			&& bMassRestoreSucceeded
			&& bDependentRuntimeRefreshed
			&& bSourcePointerRestored
			&& bRecoveredMassMatches;

		OutRecoverySummary = FString::Printf(
			TEXT("EquipmentRecovery: Fitting=%s, Mass=%s, Refresh=%s, SourcePointer=%s, FinalMass=%s%s%s"),
			bFittingRestoreSucceeded ? TEXT("Restored") : TEXT("Failed"),
			bMassRestoreSucceeded ? TEXT("Restored") : TEXT("Failed"),
			bDependentRuntimeRefreshed ? TEXT("Ready") : TEXT("Failed"),
			bSourcePointerRestored ? TEXT("Restored") : TEXT("Mismatch"),
			bRecoveredMassMatches ? TEXT("Restored") : TEXT("Mismatch"),
			MassRestoreFailureSummary.IsEmpty() ? TEXT("") : TEXT(", MassReason="),
			MassRestoreFailureSummary.IsEmpty() ? TEXT("") : *MassRestoreFailureSummary);
		return bRecoverySucceeded;
	}

	// [v1.0.0] 이미 authorization된 장비의 실제 same-Pawn Runtime apply/recovery를 수행합니다.
	FCFRuntimeEquipApplyResult ApplyValidatedEquipmentRuntime(
		ACFVehiclePawn* VehiclePawn,
		const FName TargetMountProfileId,
		UCFEquipmentPresetData* CandidateEquipmentPresetData)
	{
		// 호출자에게 반환할 bounded 장비 Runtime operation 결과입니다.
		FCFRuntimeEquipApplyResult Result;
		Result.RequestedMountProfileId = TargetMountProfileId;
		Result.RequestedEquipmentPath = GetPathNameSafe(CandidateEquipmentPresetData);

		if (!IsValid(VehiclePawn))
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = TEXT("Equipment Runtime Apply 대상 CFVehiclePawn이 유효하지 않습니다.");
			CaptureEquipmentRuntimeReadback(nullptr, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		if (!IsValid(VehiclePawn->VehicleData)
			|| TargetMountProfileId.IsNone()
			|| !IsValid(CandidateEquipmentPresetData))
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = TEXT("Equipment Runtime Apply의 VehicleData, MountProfileId 또는 EquipmentPresetData가 유효하지 않습니다.");
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// 기존 Fitting authority의 Prepare/Commit/Restore를 수행할 Pawn component입니다.
		UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();

		// Fitting Commit의 Weapon participant입니다.
		UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();

		// Fitting Commit의 Defense participant입니다.
		UCFVehicleDefenseComp* VehicleDefenseComp = VehiclePawn->GetVehicleDefenseComp();
		if (!IsValid(VehicleFittingComp)
			|| !IsValid(VehicleWeaponComp)
			|| !IsValid(VehicleDefenseComp)
			|| !VehicleFittingComp->HasAppliedRuntimeInput())
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = TEXT("Equipment Runtime Apply 전에 유효한 기존 Fitting Runtime과 Weapon/Defense 컴포넌트가 필요합니다.");
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// mutation 직전 Pawn source FittingData를 GC로부터 보존하는 strong reference입니다.
		TStrongObjectPtr<UCFVehicleFittingData> PreviousVehicleFittingData(VehiclePawn->VehicleFittingData.Get());

		// mutation 직전 FittingComp가 보유한 Applied Runtime checkpoint입니다.
		const FCFFittingRuntimeCheckpoint PreviousFittingRuntimeCheckpoint =
			VehicleFittingComp->CaptureAppliedRuntimeCheckpoint();
		if (!PreviousFittingRuntimeCheckpoint.IsValid())
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = TEXT("Equipment Runtime Apply 직전 Fitting Runtime checkpoint를 캡처할 수 없습니다.");
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// mutation 직전 Chaos VehicleMovement에 실제 설정된 질량입니다.
		float PreviousConfiguredMassKg = 0.0f;
		if (!TryGetConfiguredMassKg(VehiclePawn, PreviousConfiguredMassKg))
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = TEXT("Equipment Runtime Apply 직전 Chaos VehicleMovement 설정 질량을 읽을 수 없습니다.");
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// Fitting source, Applied Runtime과 질량을 하나로 묶은 보상 복구 checkpoint입니다.
		FCFRuntimeEquipCheckpoint RuntimeCheckpoint{
			MoveTemp(PreviousVehicleFittingData),
			PreviousFittingRuntimeCheckpoint,
			PreviousConfiguredMassKg
		};

		Result.PreviousVehicleFittingDataPath =
			GetPathNameSafe(RuntimeCheckpoint.PreviousVehicleFittingData.Get());
		Result.PreviousConfiguredMassKg = RuntimeCheckpoint.PreviousConfiguredMassKg;

		// 저장하지 않는 후보 VehicleFittingData입니다.
		UCFVehicleFittingData* TransientFittingData = nullptr;

		// 후보 FittingData에서 완전 검증된 적용용 Snapshot입니다.
		FCFVehicleFittingSnapshot CandidateFittingSnapshot;

		// Snapshot 후보 구성 또는 계약 검증 실패 이유입니다.
		FString CandidateValidationFailureReason;
		if (!BuildTransientEquipmentCandidate(
			VehiclePawn,
			TargetMountProfileId,
			CandidateEquipmentPresetData,
			TransientFittingData,
			CandidateFittingSnapshot,
			CandidateValidationFailureReason))
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = CandidateValidationFailureReason;
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// mutation 시작 전 후보 transient UObject를 operation 종료까지 보존합니다.
		TStrongObjectPtr<UCFVehicleFittingData> StrongTransientFittingData(TransientFittingData);

		Result.CandidateTotalMassKg = CandidateFittingSnapshot.TotalVehicleMassKg;
		Result.bMassReapplyRequired = !FMath::IsNearlyEqual(
			RuntimeCheckpoint.PreviousConfiguredMassKg,
			CandidateFittingSnapshot.TotalVehicleMassKg,
			RuntimeMassToleranceKg);

		// 현재 무기 선택 보존, USER target handoff 또는 비무장 target fail-closed를 결정한 순수 정책 결과입니다.
		const CFRuntimeEquipApplyPolicy::FCFCandidateActiveMountResolution ActiveMountResolution =
			CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount(
				CandidateFittingSnapshot,
				VehicleWeaponComp->GetActiveMountProfileId(),
				TargetMountProfileId);
		if (!ActiveMountResolution.CanApply())
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
			Result.Message = FString::Printf(
				TEXT("Equipment Runtime Apply가 활성 무기 장착점을 잃는 비무장 전환을 거부했습니다. CurrentActive=%s Target=%s"),
				*VehicleWeaponComp->GetActiveMountProfileId().ToString(),
				*TargetMountProfileId.ToString());
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// 후보 Fitting Runtime Prepare에 전달할 명시적으로 검증된 active Mount identity입니다.
		const FName RequestedActiveMountProfileId = ActiveMountResolution.RequestedActiveMountProfileId;

		// 후보 Snapshot을 기존 Fitting Runtime authority의 Prepared 상태로 변환한 결과입니다.
		const bool bFittingPrepared = VehicleFittingComp->PrepareSortieFittingSnapshot(
			CandidateFittingSnapshot,
			RequestedActiveMountProfileId);
		if (!bFittingPrepared)
		{
			Result.Status = ECFRuntimeEquipApplyStatus::ApplyFailed;
			Result.Message = FString::Printf(
				TEXT("Equipment Runtime Apply 후보 Fitting Prepare에 실패했습니다. %s"),
				*VehicleFittingComp->GetLastFittingRuntimeSummary());
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// Weapon·Defense·Sensor를 기존 Fitting vehicle-facing Adapter로 원자 Commit한 결과입니다.
		const bool bFittingCommitted = VehicleFittingComp->CommitPreparedSortieFittingToVehicle(
			VehiclePawn,
			VehicleWeaponComp,
			VehicleDefenseComp);
		if (!bFittingCommitted)
		{
			Result.bRecoveryAttempted = true;

			// FittingComp 내부 Commit rollback이 직전 Runtime을 실제 복구했는지 여부입니다.
			const bool bInternalFittingRecoverySucceeded =
				VehicleFittingComp->WasLastCommitFailureRecovered();

			// 내부 복구가 성공했으면 이전 Applied Fitting 기준 장비 의존 Runtime도 다시 연결합니다.
			const bool bDependentRuntimeRecovered =
				bInternalFittingRecoverySucceeded && VehiclePawn->RefreshFittingDependentRuntime();

			Result.bRecoverySucceeded =
				bInternalFittingRecoverySucceeded && bDependentRuntimeRecovered;
			Result.Status = Result.bRecoverySucceeded
				? ECFRuntimeEquipApplyStatus::ApplyFailed
				: ECFRuntimeEquipApplyStatus::RecoveryFailed;
			Result.Message = FString::Printf(
				TEXT("Equipment Runtime Fitting Commit 실패. InternalRecovery=%s, Refresh=%s | %s"),
				bInternalFittingRecoverySucceeded ? TEXT("Succeeded") : TEXT("Failed"),
				bDependentRuntimeRecovered ? TEXT("Succeeded") : TEXT("Failed"),
				*VehicleFittingComp->GetLastFittingRuntimeSummary());
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// 후보 총질량이 달라졌을 때 실제 Chaos VehicleMovement 질량을 hot reapply한 결과입니다.
		bool bMassApplied = true;

		// 후보 질량 적용 실패의 bounded 진단 문자열입니다.
		FString MassApplyFailureSummary;
		if (Result.bMassReapplyRequired)
		{
			// 실제 CarFight Chaos Physics State 보존 규칙을 사용하는 질량 적용 Adapter입니다.
			FCFChaosVehicleMassRuntime MassRuntime(VehiclePawn);
			bMassApplied = MassRuntime.ReapplyVehicleMassKg(
				CandidateFittingSnapshot.TotalVehicleMassKg,
				MassApplyFailureSummary);
		}

		if (!bMassApplied)
		{
			Result.bRecoveryAttempted = true;

			// 이미 성공한 Fitting Commit과 혹시 변경됐을 수 있는 Mass를 모두 직전 상태로 보상 복구한 결과입니다.
			FString RecoverySummary;
			Result.bRecoverySucceeded = RecoverPreviousEquipmentRuntime(
				VehiclePawn,
				RuntimeCheckpoint,
				RecoverySummary);
			Result.Status = Result.bRecoverySucceeded
				? ECFRuntimeEquipApplyStatus::ApplyFailed
				: ECFRuntimeEquipApplyStatus::RecoveryFailed;
			Result.Message = FString::Printf(
				TEXT("Equipment Runtime Mass Apply 실패. %s | %s"),
				*MassApplyFailureSummary,
				*RecoverySummary);
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		// Fitting+Mass가 성공한 뒤에만 Pawn source pointer를 저장 Asset 대신 transient 후보로 전환합니다.
		VehiclePawn->VehicleFittingData = StrongTransientFittingData.Get();

		// 후보 Applied Snapshot 기준 Ammo·TurretVisual·Launcher와 최종 CombatReady를 다시 구성한 결과입니다.
		const bool bDependentRuntimeRefreshed = VehiclePawn->RefreshFittingDependentRuntime();
		if (!bDependentRuntimeRefreshed)
		{
			Result.bRecoveryAttempted = true;

			// post-Fitting refresh 실패도 같은 Fitting/Mass/source checkpoint로 원자 보상합니다.
			FString RecoverySummary;
			Result.bRecoverySucceeded = RecoverPreviousEquipmentRuntime(
				VehiclePawn,
				RuntimeCheckpoint,
				RecoverySummary);
			Result.Status = Result.bRecoverySucceeded
				? ECFRuntimeEquipApplyStatus::ApplyFailed
				: ECFRuntimeEquipApplyStatus::RecoveryFailed;
			Result.Message = FString::Printf(
				TEXT("Equipment Runtime 장비 의존 Runtime refresh 실패. %s"),
				*RecoverySummary);
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		CaptureEquipmentRuntimeReadback(
			VehiclePawn,
			TargetMountProfileId,
			StrongTransientFittingData.Get(),
			Result);

		// final Applied Snapshot이 요청한 exact EquipmentPresetData를 가리키는지 여부입니다.
		const bool bEquipmentReadbackMatches =
			FindAppliedEquipmentForMount(VehicleFittingComp, TargetMountProfileId)
			== CandidateEquipmentPresetData;

		// final Chaos 설정 질량이 후보 Snapshot 총질량과 일치하는지 여부입니다.
		const bool bMassReadbackMatches = FMath::IsNearlyEqual(
			Result.CurrentConfiguredMassKg,
			CandidateFittingSnapshot.TotalVehicleMassKg,
			RuntimeMassToleranceKg);

		// source pointer, Fitting readback, 질량과 CombatReady가 모두 후보 적용 완료를 증명하는지 여부입니다.
		const bool bFinalReadbackMatches = Result.bRuntimeReady
			&& Result.bAppliedTransientFittingActive
			&& bEquipmentReadbackMatches
			&& bMassReadbackMatches;

		if (!bFinalReadbackMatches)
		{
			Result.bRecoveryAttempted = true;

			// 최종 readback mismatch도 부분 성공으로 숨기지 않고 직전 Runtime으로 복구합니다.
			FString RecoverySummary;
			Result.bRecoverySucceeded = RecoverPreviousEquipmentRuntime(
				VehiclePawn,
				RuntimeCheckpoint,
				RecoverySummary);
			Result.Status = Result.bRecoverySucceeded
				? ECFRuntimeEquipApplyStatus::ApplyFailed
				: ECFRuntimeEquipApplyStatus::RecoveryFailed;
			Result.Message = FString::Printf(
				TEXT("Equipment Runtime final readback 검증 실패. Equipment=%s, Mass=%s, Runtime=%s, Transient=%s | %s"),
				bEquipmentReadbackMatches ? TEXT("Match") : TEXT("Mismatch"),
				bMassReadbackMatches ? TEXT("Match") : TEXT("Mismatch"),
				Result.bRuntimeReady ? TEXT("Ready") : TEXT("NotReady"),
				Result.bAppliedTransientFittingActive ? TEXT("True") : TEXT("False"),
				*RecoverySummary);
			CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
			return Result;
		}

		Result.Status = ECFRuntimeEquipApplyStatus::Succeeded;
		Result.Message = FString::Printf(
			TEXT("Equipment Runtime Apply 성공: Mount=%s, Equipment=%s, TotalMass=%.2fkg"),
			*TargetMountProfileId.ToString(),
			*GetPathNameSafe(CandidateEquipmentPresetData),
			CandidateFittingSnapshot.TotalVehicleMassKg);
		return Result;
	}
}

// [v1.0.0] Candidate가 현재 Runtime Test Catalog에 exact UObject identity로 등록된 EquipmentPresetData인지 fail-closed로 검증합니다.
bool FCFRuntimeEquipApplyService::ValidateCatalogEquipmentCandidate(
	const UCFRuntimeTestCatalogData* RuntimeCatalog,
	const UCFEquipmentPresetData* CandidateEquipmentPresetData,
	FString& OutFailureReason)
{
	OutFailureReason.Reset();

	if (!IsValid(RuntimeCatalog))
	{
		OutFailureReason = TEXT("Runtime Test Catalog가 유효하지 않습니다.");
		return false;
	}

	// Catalog 전체 hard-reference 계약의 상세 오류 목록입니다.
	TArray<FText> CatalogValidationErrors;
	if (!RuntimeCatalog->ValidateRuntimeTestCatalog(CatalogValidationErrors))
	{
		OutFailureReason = FString::Printf(
			TEXT("Runtime Test Catalog 계약이 유효하지 않습니다. Issues=%d"),
			CatalogValidationErrors.Num());
		return false;
	}

	if (!IsValid(CandidateEquipmentPresetData))
	{
		OutFailureReason = TEXT("선택한 EquipmentPresetData가 유효하지 않습니다.");
		return false;
	}

	// exact UObject identity로 Candidate가 명시 등록 장비 목록에 포함됐는지 확인합니다.
	const bool bCandidateRegistered = RuntimeCatalog->AllowedEquipmentPresetData.ContainsByPredicate(
		[CandidateEquipmentPresetData](const TObjectPtr<UCFEquipmentPresetData>& RegisteredEquipmentPresetData)
		{
			return RegisteredEquipmentPresetData.Get() == CandidateEquipmentPresetData;
		});

	if (!bCandidateRegistered)
	{
		OutFailureReason = FString::Printf(
			TEXT("선택한 EquipmentPresetData가 Runtime Test Catalog 허용 목록에 없습니다: %s"),
			*GetPathNameSafe(CandidateEquipmentPresetData));
		return false;
	}

	return true;
}

// [v1.0.0] Catalog authorization을 통과한 EquipmentPresetData만 단일 Mount Runtime operation으로 전달합니다.
FCFRuntimeEquipApplyResult FCFRuntimeEquipApplyService::ApplyCatalogEquipment(
	ACFVehiclePawn* VehiclePawn,
	const UCFRuntimeTestCatalogData* RuntimeCatalog,
	const FName TargetMountProfileId,
	UCFEquipmentPresetData* CandidateEquipmentPresetData)
{
	// Catalog authorization 실패 이유를 보존할 문자열입니다.
	FString ValidationFailureReason;
	if (!ValidateCatalogEquipmentCandidate(
		RuntimeCatalog,
		CandidateEquipmentPresetData,
		ValidationFailureReason))
	{
		// mutation 없이 반환할 catalog 검증 실패 결과입니다.
		FCFRuntimeEquipApplyResult Result;
		Result.Status = ECFRuntimeEquipApplyStatus::ValidationFailed;
		Result.RequestedMountProfileId = TargetMountProfileId;
		Result.RequestedEquipmentPath = GetPathNameSafe(CandidateEquipmentPresetData);
		Result.Message = ValidationFailureReason;
		CaptureEquipmentRuntimeReadback(VehiclePawn, TargetMountProfileId, nullptr, Result);
		return Result;
	}

	return ApplyValidatedEquipmentRuntime(
		VehiclePawn,
		TargetMountProfileId,
		CandidateEquipmentPresetData);
}

// [v1.0.0] 호출자가 source authorization을 완료한 EquipmentPresetData를 transient Fitting + 기존 Runtime authority로 적용합니다.
FCFRuntimeEquipApplyResult FCFRuntimeEquipApplyService::ApplyEquipmentRuntime(
	ACFVehiclePawn* VehiclePawn,
	const FName TargetMountProfileId,
	UCFEquipmentPresetData* CandidateEquipmentPresetData)
{
	return ApplyValidatedEquipmentRuntime(
		VehiclePawn,
		TargetMountProfileId,
		CandidateEquipmentPresetData);
}
