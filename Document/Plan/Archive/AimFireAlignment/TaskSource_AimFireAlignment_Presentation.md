# Codex Task Source — CF-FQ-022B Aim Fire Alignment Presentation

- Task ID: `CF-FQ-022B-AIM-FIRE-PRESENTATION`
- Version: `1.0.0`
- Date: `2026-07-13`
- Priority: `P0`
- Depends On: `CF-FQ-022A-AIM-FIRE-CORE`

## Goal

Core Task가 추가한 `TurretAligning`, `WeaponNotAligned`, `MuzzleBlocked`, `FCFVehicleWeaponAimSolution`을 기존 Reticle과 VehicleDebug에 연결하고 관련 문서를 Build Complete / PIE Pending 상태로 갱신한다.

## In Scope

1. Reticle의 모든 enum switch에 `TurretAligning`을 추가한다.
2. 기존 이미지/TextBlock을 사용해 정렬 중 상태의 한국어 문구와 색상을 표시한다.
3. WeaponNotAligned가 빨간 FireRejected로 주 Reticle을 덮지 않는 Core 정책을 UI에서 유지한다.
4. MuzzleBlocked는 Blocked/AimBlocked 주황 표시를 사용한다.
5. VehicleDebug Aim 또는 Weapon 섹션에 Weapon Aim Solution 필드를 표시한다.
6. 신규 RejectReason과 ReticleState의 Debug 표시 문자열을 추가한다.
7. 관련 Plan/SSOT/Systems 문서의 Version / Changelog / Migration을 갱신한다.
8. CF-FQ-022는 Active, CF-TC-019는 PARTIAL로 기록하고 PIE PASS는 기록하지 않는다.
9. 최종 BuildEditor.bat를 실행한다.

## Out of Scope

```text
- Core Camera/Aim/Pawn 계산 변경
- 발사 방향/검증 로직 재설계
- WBP_AimReticle 구조 또는 .uasset 수정
- 신규 TextBlock/Image 바인딩
- Projectile 터널링 수정
- 실제 Damage/HP
- 탄도 Solver/선행 조준/락온
- 네트워크/복제
- VFX/SFX/애니메이션
```

## Constraints

1. Core Task가 성공적으로 적용된 작업 트리에서만 실행한다.
2. 기존 WBP Optional 바인딩 이름과 빈 BP Graph를 변경하지 않는다.
3. 신규 WBP 요소는 만들지 않는다.
4. `TurretAligningReticleColor`는 EditDefaultsOnly/BlueprintReadWrite이며 기본값은 `FLinearColor(1.0f, 0.72f, 0.15f, 1.0f)`다.
5. 표시 문구는 다음으로 고정한다.

```text
State = 조준: 터렛 정렬 중
Hint  = 총구가 조준점을 추적 중
```

6. `WeaponNotAligned`는 LastFireResult/Debug에는 보이지만 일반 FireRejected 텍스트와 빨간색으로 주 Reticle을 덮지 않는다.
7. `MuzzleBlocked`는 `AimBlocked` FireFeedback/Blocked Reticle을 재사용한다.
8. 기존 FireSuccess → Cooldown → 종료 우선순위와 OutOfArc 전용 경고 정책을 유지한다.
9. VehicleDebug 필드 ID는 기존 snake_case 스타일, 화면 표시명은 한국어를 사용한다.
10. 지정된 C++ 파일 외 코드/자산을 수정하지 않는다.
11. 문서 수정은 아래 고유 파일명의 현재 CarFight 문서만 대상으로 한다.
12. 기존 dirty worktree를 정리하거나 되돌리지 않는다.
13. commit / push 금지.
14. 빌드 성공만 기록하고 PIE PASS를 주장하지 않는다.
15. 추가 C++ 파일이 필요하면 범위를 넓히지 말고 중단 후 보고한다.

## Target Files

```text
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

## Required Document Files

```text
ImplementationDesign.md
03_FeatureQueue.md
05_TestChecklist.md
VehicleAim.md
WeaponFire.md
AimReticle.md
```

## Reticle Requirements

### Header

다음 스타일 프로퍼티를 추가한다.

```cpp
FLinearColor TurretAligningReticleColor = FLinearColor(1.0f, 0.72f, 0.15f, 1.0f);
```

프로젝트 규칙에 맞는 1줄 요약 주석, DisplayName, ToolTip을 작성한다.

### State text and hint

`GetReticleStateDisplayText()`와 `GetReticleHintDisplayText()`:

```text
TurretAligning State = 조준: 터렛 정렬 중
TurretAligning Hint  = 총구가 조준점을 추적 중
```

### Color

`GetReticleStateColor()`에서 TurretAligning은 `TurretAligningReticleColor`를 반환한다.

### Final state resolution

Core AimComp의 BaseReticleState가 TurretAligning이면 활성 FireSuccess/Cooldown 등 기존 우선순위를 제외하고 그대로 유지한다.

```text
FireSuccess active  = 기존 녹색 우선
Cooldown active     = 기존 파란색 우선
MuzzleBlocked       = Blocked 주황
WeaponNotAligned    = Base TurretAligning 유지
OutOfWeaponArc      = 기존 전용 warning 유지
그 외 reject        = 기존 FireRejected
```

모든 `ECFVehicleReticleState` switch에서 TurretAligning 누락이 없어야 한다.

## VehicleDebug Requirements

Core가 제공하는 `FCFVehicleWeaponAimSolution`을 Aim 또는 Weapon 섹션에 표시한다.

필수 표시값:

```text
DesiredAimTargetLocation
MuzzleWorldLocation
DesiredLaunchDirection
CurrentMuzzleDirection
AimAlignmentErrorDegrees
bHasValidAimTarget
bHasValidMuzzle
bWithinWeaponArc
bTurretSettled
bWeaponAlignedToAim
bMuzzleLineBlocked
MuzzleBlockingHitLocation
bCanFire
```

권장 필드 ID:

```text
aim_desired_target
aim_muzzle_location
aim_desired_launch_direction
aim_current_muzzle_direction
aim_alignment_error_degrees
aim_has_valid_target
aim_has_valid_muzzle
aim_within_weapon_arc
aim_turret_settled
aim_weapon_aligned
aim_muzzle_line_blocked
aim_muzzle_blocking_hit
aim_can_fire
```

기존 섹션 구조를 유지하고 별도 탭을 만들지 않는다.

신규 enum 표시 문자열:

```text
TurretAligning    = 터렛 정렬 중
WeaponNotAligned = 무기 미정렬
MuzzleBlocked    = 총구 경로 막힘
```

## Documentation Requirements

### ImplementationDesign.md

```text
- Version 증가
- Status = Build Complete / PIE Pending 의미
- 실제 수정 파일과 구현 결과 기록
- Migration / Changelog 추가
- Manual PIE Checklist 유지
```

### 03_FeatureQueue.md

```text
- CF-FQ-022 Ready → Active
- Core/Presentation Build 완료, PIE Pending 기록
- CF-FQ-023과 CF-FQ-018 순서 유지
```

### 05_TestChecklist.md

```text
- CF-TC-019 TODO → PARTIAL
- Static/Build 확인만 완료로 기록
- 5m/20m/100m, 정렬 중/완료, Muzzle blocked, 이동 중 테스트는 TODO 유지
```

### VehicleAim.md

현재 Weapon Aim Solution, 목표점 SSOT, 정렬/차단 상태를 구현된 현재 기준으로 기록하되 PIE 확인 전 완전 PASS로 쓰지 않는다.

### WeaponFire.md

FireRequest의 AimOrigin/AimDirection/PredictedAimTargetLocation 의미와 신규 RejectReason을 기록한다.

### AimReticle.md

TurretAligning 문구/색상/우선순위와 신규 WBP 바인딩이 필요 없다는 점을 기록한다.

## Verification

Static:

```text
- ECFVehicleReticleState switch에 TurretAligning 누락 없음
- RejectReason 표시 switch에 WeaponNotAligned/MuzzleBlocked 누락 없음
- WBP BindWidget 이름 변화 없음
- .uasset 변경 없음
- CF-FQ-022=Active, CF-TC-019=PARTIAL
- 문서에 PIE PASS 문구 없음
- Target C++ Files 외 코드 변경 없음
```

Build:

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

빌드 실패 시 첫 실제 C++ 오류를 수정하고 재실행한다. Live Coding으로 대체하지 않는다.

## Acceptance Criteria

1. TurretAligning 상태 문구와 Hint가 한국어로 표시된다.
2. TurretAligning 기본 색상이 편집 가능한 amber 값이다.
3. WeaponNotAligned가 빨간 FireRejected로 주 Reticle을 덮지 않는다.
4. MuzzleBlocked는 Blocked/AimBlocked 주황 표시다.
5. OutOfArc 전용 warning 정책이 유지된다.
6. FireSuccess/Cooldown 우선순위가 유지된다.
7. VehicleDebug에서 Weapon Aim Solution 핵심 필드를 확인할 수 있다.
8. 모든 신규 enum 문자열이 Debug/UI에서 누락되지 않는다.
9. 신규 WBP 요소와 .uasset 변경이 없다.
10. 관련 문서 Version/Changelog/Migration이 갱신된다.
11. CF-FQ-022는 Active, CF-TC-019는 PARTIAL이다.
12. BuildEditor.bat가 성공한다.
13. commit/push를 수행하지 않는다.
14. PIE는 Pending으로 보고한다.

## Manual PIE Checklist — User Verification Required

```text
1. 5m / 20m / 100m에서 Reticle과 탄착 일치
2. 가까운 목표에서 카메라-Muzzle 시차 보정
3. 터렛 회전 중 TurretAligning 표시
4. 정렬 완료 시 Ready 전환
5. 정렬 중 Fire가 실제 발사되지 않음
6. 총구 앞 낮은 벽에서 MuzzleBlocked/Blocked
7. OutOfArc와 TurretAligning 구분
8. 주행/선회 중 목표 추적
9. FireSuccess → Cooldown → 종료 회귀 없음
10. HitScan/Projectile 모드 방향 일치
```

## Migration

```text
- WBP_AimReticle은 기존 Optional 이미지/TextBlock만 사용
- 신규 BindWidget 없음
- enum 추가로 BP 컴파일 경고가 있으면 사용자 PIE 전 BP Compile/Save 필요성을 보고
- 기존 색상 프로퍼티와 저장값 유지
- 문서만 Build Complete / PIE Pending 상태로 전환
```
