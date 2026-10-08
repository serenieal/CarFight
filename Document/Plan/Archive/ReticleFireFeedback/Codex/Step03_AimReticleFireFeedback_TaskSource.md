# Codex TaskSource Step03 AimReticle FireFeedback Integration

Version: 0.1.0
Date: 2026-07-10
Task ID: ReticleFireFeedback-Step03-AimReticleFireFeedback
Status: Ready for plan.build_codex_task

## Goal

Extend `UCFAimReticleWidget` so it reads `ACFVehiclePawn::BuildFireFeedbackViewData()` and combines FireFeedback state with the existing Aim Reticle state.

The widget must keep the existing VehicleAimComp-driven Reticle flow, then apply FireFeedback as a short UI overlay / state override according to the policy below.

## In Scope

- Modify `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`.
- Modify `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`.
- Include `CFVehicleFireFeedbackTypes.h` in `CFAimReticleWidget.h`.
- Add Optional TextBlock bindings for FireFeedback display text.
- Add a cached `FCFVehicleFireFeedbackViewData` member.
- Add helper functions for applying FireFeedback view data.
- Add helper functions for FireFeedback display text and hint text.
- Add a helper function that resolves final `ECFVehicleReticleState` from base Reticle state plus FireFeedback view data.
- Update `RefreshFromPawn()` so it reads `VehiclePawnRef->BuildFireFeedbackViewData()` after reading the base Aim state.
- Update `RefreshTextBlocks()` so it also updates FireFeedback TextBlocks and cooldown text.

## Out of Scope

- Do not modify `ACFVehiclePawn` files.
- Do not modify `UCFVehicleWeaponComp` files.
- Do not modify `CFVehicleFireFeedbackTypes.h`.
- Do not modify `CFVehicleAimTypes.h`.
- Do not modify any WBP or Blueprint asset.
- Do not add animation events in this step.
- Do not create a FireFeedback component class.
- Do not add server, RPC, replication, or multiplayer code.
- Do not modify document files.

## Constraints

- This task is Step 3 only.
- The only write targets are the two files listed in Target Files.
- All new UPROPERTY / helper declarations must have one-line Korean summary comments above them.
- New TextBlock fields must use `BindWidgetOptional` only.
- Missing WBP widgets must not crash the C++ widget.
- Keep existing `Text_ReticleState`, `Text_CanFire`, and `Text_ReticleHint` behavior.
- `FireSuccess` must not override base Reticle state.
- `OutOfArcWarning` must not override base Reticle state.
- `Cooldown`, `NoWeapon`, `AimBlocked`, `FireRejected`, and `FirePending` may override base Reticle state when `bFeedbackActive` and `bOverrideReticleState` are true.
- `OutOfArcWarning` must remain a helper warning, not a hard fire block.
- `FirePending` must not mean server wait.
- `FireRejected` must not mean server rejection.

## Target Files

- `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`

## Existing Code Facts

- `UCFAimReticleWidget` currently has optional TextBlocks: `Text_ReticleState`, `Text_CanFire`, `Text_ReticleHint`.
- `UCFAimReticleWidget::RefreshFromPawn()` currently reads `VehiclePawnRef->GetVehicleAimComp()`.
- `RefreshFromPawn()` currently reads `VehicleAimComp->GetLocalAimState()` and sets `bCachedCanFire`.
- `RefreshFromPawn()` currently calls `ApplyReticleState(VehicleAimComp->GetReticleState())`.
- `RefreshTextBlocks()` currently updates `Text_ReticleState`, `Text_CanFire`, and `Text_ReticleHint`.
- Step 2 added `ACFVehiclePawn::BuildFireFeedbackViewData() const`.
- Step 1 added `FCFVehicleFireFeedbackViewData` and `ECFVehicleFireFeedbackState`.

## Required Header Changes

In `CFAimReticleWidget.h`, add this include:

```cpp
#include "CFVehicleFireFeedbackTypes.h"
```

Add these optional TextBlocks near the existing TextBlock fields:

```cpp
// [v1.3.0] 현재 발사 피드백 상태를 표시할 선택적 텍스트 위젯 참조입니다.
UPROPERTY(meta=(BindWidgetOptional))
TObjectPtr<UTextBlock> Text_FireFeedbackState = nullptr;

// [v1.3.0] 현재 발사 피드백 보조 설명을 표시할 선택적 텍스트 위젯 참조입니다.
UPROPERTY(meta=(BindWidgetOptional))
TObjectPtr<UTextBlock> Text_FireFeedbackHint = nullptr;

// [v1.3.0] 현재 남은 쿨다운 시간을 표시할 선택적 텍스트 위젯 참조입니다.
UPROPERTY(meta=(BindWidgetOptional))
TObjectPtr<UTextBlock> Text_Cooldown = nullptr;
```

Add this cache member:

```cpp
// [v1.3.0] 마지막으로 Pawn에서 읽은 FireFeedback 표시 데이터입니다.
UPROPERTY(BlueprintReadOnly, Category="CarFight|Aim|Reticle|FireFeedback", meta=(DisplayName="FireFeedback 표시 데이터 캐시 (CachedFireFeedbackViewData)", ToolTip="현재 Reticle UI가 표시 중인 로컬 발사 피드백 표시 데이터입니다."))
FCFVehicleFireFeedbackViewData CachedFireFeedbackViewData;
```

Add these private helper declarations:

```cpp
// [v1.3.0] FireFeedback 표시 데이터를 캐시에 저장하고 선택적 TextBlock에 반영합니다.
void ApplyFireFeedbackViewData(const FCFVehicleFireFeedbackViewData& InViewData);

// [v1.3.0] FireFeedback 상태를 UI 표시용 한국어 텍스트로 변환합니다.
FText GetFireFeedbackStateDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const;

// [v1.3.0] FireFeedback 상태를 UI 보조 설명 텍스트로 변환합니다.
FText GetFireFeedbackHintDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const;

// [v1.3.0] 기본 Reticle 상태와 FireFeedback 표시 데이터를 합쳐 최종 Reticle 상태를 반환합니다.
ECFVehicleReticleState ResolveReticleStateFromFireFeedback(const FCFVehicleFireFeedbackViewData& InViewData, ECFVehicleReticleState BaseReticleState) const;
```

## Required CPP Changes

Update `RefreshFromPawn()` flow:

- If `VehiclePawnRef` is invalid, reset `CachedFireFeedbackViewData` to default and hide as before.
- If `VehicleAimComp` is invalid, reset `CachedFireFeedbackViewData` to default and hide as before.
- Read `BaseReticleState = VehicleAimComp->GetReticleState()`.
- Read `CachedFireFeedbackViewData = VehiclePawnRef->BuildFireFeedbackViewData()` through `ApplyFireFeedbackViewData()`.
- Resolve final Reticle state using `ResolveReticleStateFromFireFeedback(CachedFireFeedbackViewData, BaseReticleState)`.
- Call `ApplyReticleState(FinalReticleState)`.
- Keep `UpdateReticleVisibility()` behavior.

Implement `ResolveReticleStateFromFireFeedback()` with this policy:

- If `!InViewData.bFeedbackActive`, return `BaseReticleState`.
- If `!InViewData.bOverrideReticleState`, return `BaseReticleState`.
- `FirePending` -> `ECFVehicleReticleState::FirePending`.
- `FireRejected` -> `ECFVehicleReticleState::FireRejected`.
- `Cooldown` -> `ECFVehicleReticleState::Cooldown`.
- `NoWeapon` -> `ECFVehicleReticleState::NoWeapon`.
- `AimBlocked` -> `ECFVehicleReticleState::Blocked`.
- `FireSuccess`, `OutOfArcWarning`, `None`, and default -> `BaseReticleState`.

Implement `ApplyFireFeedbackViewData()`:

- Store `CachedFireFeedbackViewData = InViewData`.
- Call `RefreshTextBlocks()`.

Update `RefreshTextBlocks()`:

- Keep existing updates for `Text_ReticleState`, `Text_CanFire`, and `Text_ReticleHint`.
- If `Text_FireFeedbackState` exists, set it to `GetFireFeedbackStateDisplayText(CachedFireFeedbackViewData.FeedbackState)` when `bFeedbackActive` is true, otherwise empty text.
- If `Text_FireFeedbackHint` exists, set it to `GetFireFeedbackHintDisplayText(CachedFireFeedbackViewData.FeedbackState)` when `bFeedbackActive` is true, otherwise empty text.
- If `Text_Cooldown` exists, show formatted remaining cooldown text only when `CachedFireFeedbackViewData.bShowCooldown` is true. Otherwise set empty text.
- Suggested cooldown text format: `FText::FromString(FString::Printf(TEXT("%.2f초"), CachedFireFeedbackViewData.RemainingCooldownSeconds))`.

Implement FireFeedback state display text:

- `FireSuccess` -> `발사`
- `FirePending` -> `발사 처리 중`
- `FireRejected` -> `발사 불가`
- `Cooldown` -> `재사용 대기`
- `NoWeapon` -> `무기 없음`
- `AimBlocked` -> `조준 가림`
- `OutOfArcWarning` -> `각도 경고`
- `None` / default -> empty text

Implement FireFeedback hint text:

- `FireSuccess` -> `발사 요청 수락`
- `FirePending` -> `로컬 발사 처리 중`
- `FireRejected` -> `발사 조건 미충족`
- `Cooldown` -> `무기 재사용 대기 중`
- `NoWeapon` -> `사용 가능한 무기 없음`
- `AimBlocked` -> `조준선이 막힘`
- `OutOfArcWarning` -> `조준각 경고`
- `None` / default -> empty text

## Acceptance Criteria

- `CFAimReticleWidget.h` includes `CFVehicleFireFeedbackTypes.h`.
- `Text_FireFeedbackState`, `Text_FireFeedbackHint`, and `Text_Cooldown` are added with `BindWidgetOptional`.
- `CachedFireFeedbackViewData` is added.
- `ApplyFireFeedbackViewData()` is declared and implemented.
- `GetFireFeedbackStateDisplayText()` is declared and implemented.
- `GetFireFeedbackHintDisplayText()` is declared and implemented.
- `ResolveReticleStateFromFireFeedback()` is declared and implemented.
- `RefreshFromPawn()` reads `VehiclePawnRef->BuildFireFeedbackViewData()`.
- `RefreshFromPawn()` applies final Reticle state resolved from base Reticle state and FireFeedback view data.
- `RefreshTextBlocks()` updates FireFeedback and Cooldown optional TextBlocks safely.
- `FireSuccess` does not override base Reticle state.
- `OutOfArcWarning` does not override base Reticle state.
- `Cooldown`, `NoWeapon`, `AimBlocked`, `FireRejected`, and `FirePending` can override base Reticle state only when active and configured to override.
- Existing Reticle TextBlocks still work.
- No WBP / Blueprint asset is modified.
- No Pawn or Weapon component file is modified.
- No server, RPC, replication, or multiplayer code is added.

## Verification

- Inspect git diff and confirm only `CFAimReticleWidget.h` and `CFAimReticleWidget.cpp` are changed by Step 3.
- Confirm no changes to `CFVehiclePawn.h` or `CFVehiclePawn.cpp`.
- Confirm no changes to `CFVehicleWeaponComp.h` or `CFVehicleWeaponComp.cpp`.
- Confirm no changes to WBP or Blueprint assets.
- Confirm all new widget bindings use `BindWidgetOptional`.
- Confirm `BuildFireFeedbackViewData()` is called from `RefreshFromPawn()`.
- Confirm `FireSuccess` and `OutOfArcWarning` do not override base Reticle state.
- If a C++ build is available, run the normal project build or Unreal C++ compile path.
- If build cannot be run, report that build was not run and provide static verification instead.

## Reporting Format

```text
작업 완료 보고

수정 파일:
- UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
- UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp

추가한 항목:
- Text_FireFeedbackState
- Text_FireFeedbackHint
- Text_Cooldown
- CachedFireFeedbackViewData
- ApplyFireFeedbackViewData()
- GetFireFeedbackStateDisplayText()
- GetFireFeedbackHintDisplayText()
- ResolveReticleStateFromFireFeedback()
- RefreshFromPawn() FireFeedback 통합
- RefreshTextBlocks() FireFeedback 표시 확장

수정하지 않은 파일:
- CFVehiclePawn.h / .cpp
- CFVehicleWeaponComp.h / .cpp
- CFVehicleFireFeedbackTypes.h
- WBP assets
- Documents

검증:
- git diff:
- BindWidgetOptional:
- BuildFireFeedbackViewData call:
- FireSuccess / OutOfArcWarning override policy:
- build:

특이사항:
- 없음, or describe issue
```
