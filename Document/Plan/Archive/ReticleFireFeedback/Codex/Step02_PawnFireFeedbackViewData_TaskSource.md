# Codex TaskSource Step02 Pawn FireFeedback ViewData

Version: 0.1.1
Date: 2026-07-09
Task ID: ReticleFireFeedback-Step02-PawnFireFeedbackViewData
Status: Ready for plan.build_codex_task

## Goal

Add Pawn-side FireFeedback ViewData generation for CarFight Reticle / FireFeedback implementation.

The Pawn must expose a Blueprint-callable function that builds `FCFVehicleFireFeedbackViewData` from existing local fire result, weapon cooldown, and local aim state.

## In Scope

- Modify `CFVehiclePawn.h`.
- Modify `CFVehiclePawn.cpp`.
- Include `CFVehicleFireFeedbackTypes.h` in `CFVehiclePawn.h`.
- Add `LastFireFeedbackStartTimeSeconds` to `ACFVehiclePawn`.
- Add `FireSuccessFeedbackDurationSeconds` to `ACFVehiclePawn`.
- Add `FireRejectedFeedbackDurationSeconds` to `ACFVehiclePawn`.
- Add `BuildFireFeedbackViewData() const` to `ACFVehiclePawn`.
- Update `ApplyFireResult()` so accepted and rejected fire results refresh `LastFireFeedbackStartTimeSeconds`.
- Implement `ACFVehiclePawn::BuildFireFeedbackViewData() const`.
- Use existing `GetActiveWeaponCooldownSeconds()` for total cooldown.
- Use existing `GetRemainingCooldownSeconds(float CurrentTimeSeconds)` for remaining cooldown.
- Use existing `VehicleAimComp->GetLocalAimState()` to determine `bShowOutOfArcWarning`.

## Out of Scope

- Do not modify the Step 1 FireFeedback types file unless a compile error proves it is invalid.
- Do not modify Aim Reticle widget files.
- Do not modify Vehicle Weapon component files.
- Do not modify any WBP or Blueprint asset.
- Do not modify any document file.
- Do not add a new cooldown Getter to `UCFVehicleWeaponComp`.
- Do not create a FireFeedback component class.
- Do not add server, RPC, replication, or multiplayer code.

## Constraints

- This task is Step 2 only.
- The only write targets are the two files listed in Target Files.
- All new UPROPERTY and UFUNCTION declarations must have one-line Korean summary comments above them.
- `BuildFireFeedbackViewData()` must be `const`.
- `BuildFireFeedbackViewData()` must not mutate game state.
- `FirePending` must not mean server wait.
- `FireRejected` must not mean server rejection.
- `OutOfArcWarning` must be a helper warning and must not be treated as a hard fire block.
- Do not add any `.cpp` file for FireFeedback types.
- Do not add any new class.

## Target Files

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

## Existing Code Facts

- `ACFVehiclePawn` already has `LastFireRequest`.
- `ACFVehiclePawn` already has `LastFireResult`.
- `ACFVehiclePawn` already has `ApplyFireResult(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult)`.
- `ACFVehiclePawn` already has `GetVehicleAimComp()` and `GetVehicleWeaponComp()`.
- `UCFVehicleWeaponComp` already has `GetActiveWeaponCooldownSeconds() const`.
- `UCFVehicleWeaponComp` already has `GetRemainingCooldownSeconds(float CurrentTimeSeconds) const`.
- `UCFVehicleWeaponComp` already has `IsActiveWeaponOnCooldown(float CurrentTimeSeconds) const`.

## Required Header Changes

In `CFVehiclePawn.h`, add this include in the include area:

```cpp
#include "CFVehicleFireFeedbackTypes.h"
```

Add these properties to `ACFVehiclePawn`, near existing fire / aim debug fields such as `LastFireRequest` and `LastFireResult`:

```cpp
// [v0.1.1] 마지막 로컬 발사 피드백이 시작된 월드 시간입니다.
UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|FireFeedback", meta=(DisplayName="마지막 FireFeedback 시작 시간 (LastFireFeedbackStartTimeSeconds)", ToolTip="Reticle / FireFeedback UI가 최근 발사 성공 또는 실패 피드백 표시 시간을 계산할 때 사용하는 월드 시간입니다."))
double LastFireFeedbackStartTimeSeconds = -1.0;

// [v0.1.1] 발사 성공 피드백을 화면에 유지할 시간입니다.
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|FireFeedback", meta=(ClampMin="0.0", DisplayName="발사 성공 피드백 유지 시간 (FireSuccessFeedbackDurationSeconds)", ToolTip="발사 성공 피드백을 Reticle UI에 짧게 표시할 시간입니다."))
float FireSuccessFeedbackDurationSeconds = 0.12f;

// [v0.1.1] 발사 실패 피드백을 화면에 유지할 시간입니다.
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|FireFeedback", meta=(ClampMin="0.0", DisplayName="발사 실패 피드백 유지 시간 (FireRejectedFeedbackDurationSeconds)", ToolTip="발사 실패 또는 조건 미충족 피드백을 Reticle UI에 표시할 시간입니다."))
float FireRejectedFeedbackDurationSeconds = 0.35f;
```

Add this function declaration to a public UI/debug-accessible function area:

```cpp
// [v0.1.1] Reticle / FireFeedback UI가 읽을 현재 로컬 발사 피드백 표시 데이터를 만듭니다.
UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|FireFeedback", meta=(DisplayName="FireFeedback 표시 데이터 만들기 (BuildFireFeedbackViewData)", ToolTip="마지막 로컬 발사 결과, 무기 쿨다운, 로컬 조준 상태를 Reticle / FireFeedback UI 표시 데이터로 변환합니다."))
FCFVehicleFireFeedbackViewData BuildFireFeedbackViewData() const;
```

## Required CPP Changes

In `ACFVehiclePawn::ApplyFireResult(...)`, after assigning `LastFireRequest` and `LastFireResult`, record the feedback start time:

```cpp
// [v0.1.1] 마지막 로컬 FireFeedback 표시 시작 시간을 기록합니다.
LastFireFeedbackStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;
```

Implement `ACFVehiclePawn::BuildFireFeedbackViewData() const` near other fire-related functions.

Implementation requirements:

- Initialize `FCFVehicleFireFeedbackViewData ViewData;`.
- Set `ViewData.LastRejectReason = LastFireResult.RejectReason;`.
- Get current world time using `GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0`.
- Compute feedback age from `LastFireFeedbackStartTimeSeconds`.
- If there is no valid feedback start time, treat feedback age as very large.
- Read total cooldown from `VehicleWeaponComp->GetActiveWeaponCooldownSeconds()`.
- Read remaining cooldown from `VehicleWeaponComp->GetRemainingCooldownSeconds(static_cast<float>(CurrentTimeSeconds))`.
- Set `TotalCooldownSeconds`, `RemainingCooldownSeconds`, `bShowCooldown`, and `CooldownRatio`.
- If `VehicleAimComp` exists, set `bShowOutOfArcWarning = !VehicleAimComp->GetLocalAimState().bLocalWithinWeaponArc`.
- If cooldown is active, return `Cooldown` feedback with `bFeedbackActive = true` and `bOverrideReticleState = true`.
- If `LastFireResult.bAccepted` is true, return `FireSuccess` while inside `FireSuccessFeedbackDurationSeconds`; success must not override base Reticle state.
- If reject reason is `NoWeapon`, return `NoWeapon` while inside `FireRejectedFeedbackDurationSeconds`; it should override base Reticle state.
- If reject reason is `WeaponCooldown`, return `Cooldown`; it should override base Reticle state.
- If reject reason is `AimBlocked`, return `AimBlocked` while inside `FireRejectedFeedbackDurationSeconds`; it should override base Reticle state.
- If reject reason is `OutOfWeaponArc`, return `OutOfArcWarning`, set `bShowOutOfArcWarning = true`, and do not override base Reticle state.
- If reject reason is `None`, return `None` with inactive feedback unless success branch already handled it.
- For all other reject reasons, return `FireRejected` while inside `FireRejectedFeedbackDurationSeconds`; it should override base Reticle state.
- Set `FeedbackDisplayKey` to `FireSuccess`, `Cooldown`, `NoWeapon`, `AimBlocked`, `OutOfArcWarning`, `FireRejected`, or `NAME_None`.

## Acceptance Criteria

- `CFVehiclePawn.h` includes `CFVehicleFireFeedbackTypes.h`.
- `ACFVehiclePawn` has `LastFireFeedbackStartTimeSeconds`.
- `ACFVehiclePawn` has `FireSuccessFeedbackDurationSeconds`.
- `ACFVehiclePawn` has `FireRejectedFeedbackDurationSeconds`.
- `ACFVehiclePawn` declares `BuildFireFeedbackViewData() const` as a Blueprint-callable UFUNCTION.
- `CFVehiclePawn.cpp` implements `ACFVehiclePawn::BuildFireFeedbackViewData() const`.
- `ApplyFireResult()` updates `LastFireFeedbackStartTimeSeconds` for accepted and rejected results.
- The implementation uses `GetActiveWeaponCooldownSeconds()` for total cooldown.
- The implementation uses `GetRemainingCooldownSeconds(float CurrentTimeSeconds)` for remaining cooldown.
- The implementation does not add any new `UCFVehicleWeaponComp` Getter.
- `OutOfArcWarning` is helper warning only and does not override base Reticle state.
- `FireSuccess` does not override base Reticle state.
- `Cooldown`, `NoWeapon`, `AimBlocked`, and generic `FireRejected` can override base Reticle state.
- No UI widget file is modified.
- No Blueprint / WBP asset is modified.
- No server, RPC, replication, or multiplayer code is added.

## Verification

- Inspect git diff and confirm only `CFVehiclePawn.h` and `CFVehiclePawn.cpp` are changed by Step 2.
- Confirm Step 1 FireFeedback types file is used but not unnecessarily modified.
- Confirm there is no change to Aim Reticle widget files.
- Confirm there is no change to Vehicle Weapon component files.
- Confirm `GetActiveWeaponRemainingCooldownSeconds()` is not introduced anywhere.
- Confirm `GetRemainingCooldownSeconds(static_cast<float>(CurrentTimeSeconds))` is used.
- If a C++ build is available, run the normal project build or Unreal C++ compile path.
- If build cannot be run, report that build was not run and provide static verification instead.

## Reporting Format

```text
작업 완료 보고

수정 파일:
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp

추가한 항목:
- LastFireFeedbackStartTimeSeconds
- FireSuccessFeedbackDurationSeconds
- FireRejectedFeedbackDurationSeconds
- BuildFireFeedbackViewData()
- ApplyFireResult() 피드백 시작 시간 기록

수정하지 않은 파일:
- CFVehicleFireFeedbackTypes.h, unless compile fix was required
- CFAimReticleWidget.h / .cpp
- CFVehicleWeaponComp.h / .cpp
- WBP assets
- Documents

검증:
- git diff:
- cooldown getter usage:
- OutOfArcWarning override policy:
- build:

특이사항:
- 없음, or describe issue
```
