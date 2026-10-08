# Codex Task Source — CF-FQ-022 Reticle Recovery Hotfix

- Task ID: `CF-FQ-022-HOTFIX-RETICLE-RECOVERY`
- Version: `1.0.0`
- Date: `2026-07-13`
- Priority: `P0 Blocking Bug`
- Status: `Ready for Codex`

## Goal

터렛/총구 정렬 중 `LocalReticleState`가 `Hidden`으로 떨어지고, Reticle 위젯 Root가 `Collapsed`되어 `NativeTick()` 복구가 끊기는 문제를 수정한다.

완료 후에는 정렬 중 Reticle이 amber `TurretAligning` 상태로 계속 표시되고, 어떠한 임시 `Hidden` 상태가 발생하더라도 위젯이 Tick을 유지해 다시 표시 상태로 복구되어야 한다.

## Confirmed Root Cause

```text
WeaponAimSolution 유효
→ bTurretAligning 또는 bWeaponNotAligned
→ bLocalCanFire = false
→ bAimBlocked = false
→ bWithinWeaponArc = true
→ BuildLocalReticleState() 마지막 fallback이 Hidden 반환
→ UCFAimReticleWidget::UpdateReticleVisibility()가 Root를 Collapsed 처리
→ NativeTick 갱신 중단 가능
→ 정렬 완료 후 Ready로 복구하지 못함
```

현재 `ECFVehicleReticleState`에는 `TurretAligning` 값이 없고, FireFeedback의 DisplayKey로만 정렬 중 문구를 표시한다.

## In Scope

1. `ECFVehicleReticleState::TurretAligning`을 추가한다.
2. `UCFVehicleAimComp`가 유효한 Weapon Aim Solution의 정렬 중 상태를 `Hidden`이 아니라 `TurretAligning`으로 반환하게 한다.
3. Reticle 상태 우선순위를 명확히 한다.
4. `UCFAimReticleWidget`의 상태 기반 숨김에서 Root `Collapsed` 사용을 제거한다.
5. 실제 Root Visibility는 기존 `ACFVehiclePawn::RefreshAimReticleWidget()`이 계속 담당하게 한다.
6. 위젯 내부의 논리적 `Hidden`은 RenderOpacity 또는 동등한 Tick 유지 방식으로 표현한다.
7. `TurretAligning`의 상태 텍스트, Hint, 색상 switch를 추가한다.
8. 기존 FireFeedback DisplayKey 기반 `TurretAligning` 표시와 충돌하지 않게 유지한다.
9. 관련 코드 파일의 Version / Changelog / Migration을 갱신한다.
10. 관련 문서를 Hotfix Build Complete / PIE Pending 상태로 갱신한다.
11. `Tools\BuildEditor.bat` 빌드를 실행한다.

## Out of Scope

```text
- Aim Solution 계산식 변경
- Muzzle obstruction 계산 변경
- FireRequest 방향/검증 순서 변경
- Projectile 터널링 수정
- Damage/HP 구현
- WBP_AimReticle 구조 변경
- 신규 BindWidget 추가
- .uasset, 맵, StaticMesh 수정
- 네트워크/복제
- Git commit/push
```

## Constraints

1. 기존 `WeaponNotAligned`, `MuzzleBlocked`, `OutOfWeaponArc`, FireSuccess, Cooldown 처리 의미를 변경하지 않는다.
2. `WeaponNotAligned`는 일반 빨간 FireRejected로 주 Reticle을 덮지 않는다.
3. `MuzzleBlocked`는 기존 Blocked/AimBlocked 주황 상태를 유지한다.
4. `OutOfArcWarning` 전용 텍스트 정책을 유지한다.
5. `TurretAligningReticleColor` 기존 프로퍼티를 재사용한다. 새 색상 프로퍼티를 만들지 않는다.
6. `Hidden` 상태 자체는 삭제하지 않는다. Runtime 미준비, 비로컬 Pawn 등 실제 숨김 용도로 유지한다.
7. 위젯 자체의 상태 갱신 함수는 Root를 `Collapsed` 또는 `Hidden`으로 만들지 않는다.
8. `ACFVehiclePawn::RefreshAimReticleWidget()`의 `ShouldShowAimReticle()` 기반 Root Visibility 정책은 변경하지 않는다.
9. WBP와 `.uasset`을 수정하지 않는다.
10. 지정된 코드 파일 외 C++를 수정하지 않는다.
11. 기존 dirty worktree를 정리하거나 되돌리지 않는다.
12. PIE PASS를 주장하지 않는다.
13. commit/push를 수행하지 않는다.

## Target Files

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

## Required Document Updates

```text
Document/Plan/AimFireAlignment/ImplementationDesign.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/ProjectSSOT/05_TestChecklist.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/UI/AimReticle.md
```

## Required Code Changes

### 1. Reticle enum

`CFVehicleAimTypes.h`의 `ECFVehicleReticleState`에 다음 값을 추가한다.

```cpp
TurretAligning UMETA(DisplayName="TurretAligning")
```

기존 enum 값은 삭제하거나 이름을 변경하지 않는다.

### 2. AimComp state resolution

`RefreshLocalAimState()`에서 다음 의미의 값을 계산한다.

```cpp
const bool bWeaponIsAligning = bHasWeaponAimSolution
    && (WeaponAimSolution.bTurretAligning || WeaponAimSolution.bWeaponNotAligned);
```

`BuildLocalReticleState()`가 정렬 상태를 받을 수 있도록 시그니처를 확장한다.

권장 우선순위:

```text
1. Runtime 미준비          → Hidden
2. Muzzle blocked          → Blocked
3. 무기 조준각 밖          → OutOfArc
4. 터렛/총구 정렬 중       → TurretAligning
5. 발사 가능               → Ready
6. 그 외 비정상 fallback   → Hidden
```

`bWeaponIsAligning=true`인 정상 정렬 대기 상태에서 `Hidden`을 반환하면 안 된다.

### 3. Widget tick recovery

`UCFAimReticleWidget::UpdateReticleVisibility()`는 CachedReticleState에 따라 Root Visibility를 `Collapsed`로 바꾸지 않는다.

권장 구현:

```cpp
SetRenderOpacity(CachedReticleState == ECFVehicleReticleState::Hidden ? 0.0f : 1.0f);
```

Root Visibility는 Pawn이 설정한 `HitTestInvisible/Collapsed` 값을 덮어쓰지 않는다.

이렇게 해서 논리적 Hidden 상태에서도 `NativeTick()`과 `RefreshFromPawn()`이 계속 실행되고, 상태가 Ready/TurretAligning으로 바뀌면 RenderOpacity가 1로 복구되어야 한다.

### 4. TurretAligning UI switches

다음 함수에 `TurretAligning` 처리를 추가한다.

```text
GetReticleStateDisplayText()
GetReticleHintDisplayText()
GetReticleStateColor()
```

문구:

```text
State = 조준: 터렛 정렬 중
Hint  = 총구가 조준점을 추적 중
Color = TurretAligningReticleColor
```

기존 FireFeedback DisplayKey 기반 정렬 중 문구는 유지한다.

## Acceptance Criteria

1. 정렬 중 `LocalReticleState`가 `TurretAligning`이다.
2. 정렬 중 Reticle이 사라지지 않는다.
3. 정렬 완료 후 `Ready`로 전환된다.
4. 논리적 `Hidden`이 발생해도 위젯 Tick이 중단되지 않고 이후 표시 상태로 복구된다.
5. 위젯 내부 상태 함수가 Root Visibility를 `Collapsed`로 설정하지 않는다.
6. Pawn의 `ShouldShowAimReticle()` 정책은 유지된다.
7. `WeaponNotAligned`가 빨간 FireRejected로 덮어쓰지 않는다.
8. `MuzzleBlocked`, FireSuccess, Cooldown, OutOfArcWarning 회귀가 없다.
9. WBP와 `.uasset` 변경이 없다.
10. `BuildEditor.bat`가 성공한다.
11. 문서는 Hotfix Build Complete / PIE Pending으로 기록한다.
12. commit/push를 수행하지 않는다.

## Verification

### Static review

```text
- ECFVehicleReticleState에 TurretAligning 존재
- BuildLocalReticleState 호출과 선언/정의가 새 정렬 인자를 일치시킴
- 정상 정렬 대기 경로가 Hidden으로 떨어지지 않음
- CFAimReticleWidget.cpp에서 CachedReticleState 기반 SetVisibility(Collapsed) 제거
- TurretAligning 텍스트/힌트/색상 switch 존재
- 지정 C++ 파일 외 코드 변경 없음
- .uasset 변경 없음
```

### Build

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

빌드 실패 시 첫 실제 C++ 오류를 수정하고 재실행한다.

### Manual PIE — User verification required

```text
1. PIE 시작 직후 Reticle 표시
2. 조준 방향을 크게 변경하면 amber TurretAligning 표시
3. 정렬 완료 시 Ready 복귀
4. 정렬 중 발사 입력 후 Reticle이 사라지지 않음
5. FireSuccess → Cooldown → 현재 Aim 상태 복귀
6. 총구 막힘에서 Blocked 표시
7. OutOfArc 전용 경고 유지
8. PIE 종료 후 재실행 시 Reticle 정상 표시
```

## Documentation State

```text
CF-FQ-022 = Active 유지
CF-TC-019 = PARTIAL 유지
Hotfix 코드/빌드 완료만 기록
Reticle Recovery PIE = Pending
```

## Unresolved

- 없음.
