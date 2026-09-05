// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFRuntimeEquipApplyPolicy.h
// Version: 1.1.0
// Date: 2026-09-04
// Description: Runtime Equipment Apply의 candidate active weapon Mount 선택 및 fail-closed 전환 순수 정책입니다.
// Scope: 현재 active weapon Mount 보존, USER target Mount handoff와 비무장 target 전환 거부를 Snapshot truth만으로 결정합니다.
// Changelog:
// - v1.1.0: active weapon이 candidate에서 사라졌는데 USER target이 weapon-bearing이 아니면 downstream 암묵 동작에 의존하지 않고 명시적으로 Reject하는 resolution 계약을 추가.
// - v1.0.0: current active weapon Mount 보존과 USER target weapon Mount handoff 정책을 최초 추가.
// Migration:
// - Private 정책 전용 변경이라 Asset migration은 없습니다. Caller는 Prepared/Commit 전에 resolution CanApply()를 확인해야 합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFFittingTypes.h"

namespace CFRuntimeEquipApplyPolicy
{
	// Candidate active Mount 선택 결과를 설명하는 결정론적 정책 상태입니다.
	enum class ECFCandidateActiveMountDecision : uint8
	{
		PreserveCurrent,
		HandoffToTarget,
		NoActiveWeapon,
		RejectNonWeaponTarget
	};

	// Candidate active Mount 선택과 Apply 허용 여부를 함께 운반합니다.
	struct FCFCandidateActiveMountResolution
	{
		// Fitting Runtime Prepare에 전달할 active Mount identity입니다.
		FName RequestedActiveMountProfileId = NAME_None;

		// 이 identity가 선택된 이유 또는 fail-closed 거부 이유입니다.
		ECFCandidateActiveMountDecision Decision = ECFCandidateActiveMountDecision::RejectNonWeaponTarget;

		// Candidate Fitting Prepare까지 진행해도 되는지 반환합니다.
		bool CanApply() const
		{
			return Decision != ECFCandidateActiveMountDecision::RejectNonWeaponTarget;
		}
	};

	// Snapshot에 exact weapon-bearing Mount가 존재하는지 확인합니다.
	inline bool HasWeaponBearingMount(
		const FCFVehicleFittingSnapshot& FittingSnapshot,
		const FName MountProfileId)
	{
		if (MountProfileId.IsNone())
		{
			return false;
		}

		return FittingSnapshot.ResolvedMounts.ContainsByPredicate(
			[MountProfileId](const FCFResolvedFittingMount& ResolvedMount)
			{
				return ResolvedMount.MountProfileId == MountProfileId
					&& ResolvedMount.WeaponData.Get() != nullptr;
			});
	}

	// 기존 active weapon Mount가 candidate에 남아 있으면 보존하고,
	// 사라졌다면 USER가 명시 적용한 target weapon Mount로만 좁게 전환합니다.
	// 기존 active weapon이 있었는데 target이 비무장이면 Prepare 전에 명시적으로 거부합니다.
	inline FCFCandidateActiveMountResolution ResolveCandidateActiveMount(
		const FCFVehicleFittingSnapshot& CandidateSnapshot,
		const FName CurrentActiveMountProfileId,
		const FName TargetMountProfileId)
	{
		if (HasWeaponBearingMount(CandidateSnapshot, CurrentActiveMountProfileId))
		{
			return {CurrentActiveMountProfileId, ECFCandidateActiveMountDecision::PreserveCurrent};
		}

		if (HasWeaponBearingMount(CandidateSnapshot, TargetMountProfileId))
		{
			return {TargetMountProfileId, ECFCandidateActiveMountDecision::HandoffToTarget};
		}

		if (CurrentActiveMountProfileId.IsNone())
		{
			return {NAME_None, ECFCandidateActiveMountDecision::NoActiveWeapon};
		}

		return {CurrentActiveMountProfileId, ECFCandidateActiveMountDecision::RejectNonWeaponTarget};
	}
}
