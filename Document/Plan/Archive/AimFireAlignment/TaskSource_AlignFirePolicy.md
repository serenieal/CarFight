# Codex Task Source — CF-FQ-022 Align Fire Policy

- Task ID: `CF-FQ-022-ALIGN-FIRE-POLICY`
- Version: `1.0.0`
- Date: `2026-07-13`
- Priority: `P0 Follow-up`
- Status: `Ready for Codex`

## Goal

터렛별로 정렬 중 발사 허용 여부를 `UCFTurretMountData` DataAsset에서 선택할 수 있게 한다.

`bAllowFireWhileAligning=true`인 터렛은 회전 또는 총구 정렬 중에도 발사할 수 있어야 한다. 단, 탄환이 Reticle 목표 방향으로 꺾여 나가면 안 되므로 정렬 중 실제 발사 방향은 현재 Muzzle Socket X축 방향이어야 한다.

`bAllowFireWhileAligning=false`인 터렛은 현재처럼 정렬 완료 전 발사를 거부한다.

## Repository First

구현 전에 반드시 현재 저장소 코드와 SSOT를 다시 읽는다. 이전 대화, 추정, 오래된 문서보다 현재 Repository와 SSOT를 우선한다.

우선 확인할 코드:

```text
UE/Source/CarFight_Re/Public/CFTurretMountData.h
UE/Source/CarFight_Re/Private/CFTurretMountData.cpp
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
```

우선 확인할 문서:

```text
Document/ProjectSSOT/03_FeatureQueue.md
Document/ProjectSSOT/05_TestChecklist.md
Document/Plan/AimFireAlignment/ImplementationDesign.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/FireFeedback.md
Document/Systems/UI/AimReticle.md
Document/Plan/VehiclePlatformPlan/CF_CombatDataOwnership.md
```

## Confirmed Current Behavior

현재 `ACFVehiclePawn::ValidateFireCommand()`는 아래 상태를 각각 무조건 거부한다.

```text
WeaponAimSolution.bTurretAligning
WeaponAimSolution.bWeaponNotAligned
```

현재 `BuildWeaponAimSolution()`은 Reticle 목표점으로 향하는 방향을 계산한 뒤 `FinalFireOrigin.WorldFireDirection`과 `WeaponAimSolution.AimDirection`에 사용한다.

따라서 정렬 거부만 단순히 제거하면 아직 다른 방향을 보고 있는 총구에서 탄환이 Reticle 목표 방향으로 꺾여 나가는 문제가 생길 수 있다.

## In Scope

1. `UCFTurretMountData`에 `bAllowFireWhileAligning` 필드를 추가한다.
2. 필드 기본값은 `true`로 한다.
3. Category는 터렛 발사 정책을 명확히 나타내는 경로를 사용한다.
4. 한국어 `DisplayName`과 구체적인 `ToolTip`을 작성한다.
5. `FCFVehicleWeaponAimSolution`이 아래 의미를 구분해 보관하도록 현재 구조에 맞는 최소 필드를 추가한다.
   - 정렬 중 발사 허용 정책
   - Reticle 목표점으로 향하는 요구 방향
   - 현재 Muzzle Socket X축 방향
   - 실제 HitScan/Projectile이 사용할 최종 발사 방향
6. 권장 필드 의미는 다음과 같다.

```text
bAllowFireWhileAligning
DesiredAimDirection
CurrentMuzzleDirection
AimDirection = 실제 최종 발사 방향
```

7. 정렬 상태는 다음 의미로 계산한다.

```text
bWeaponIsAligning = bTurretAligning || bWeaponNotAligned
```

8. 최종 발사 방향은 다음 정책을 따른다.

```text
정렬 미완료 + 허용 true  -> CurrentMuzzleDirection
정렬 완료                -> DesiredAimDirection
정렬 미완료 + 허용 false -> 방향은 계산하되 검증에서 발사 거부
```

9. `BuildFireCommand`, HitScan, Projectile은 동일한 최종 `FireRequest.AimDirection`을 사용한다.
10. `ValidateFireCommand()`는 `bAllowFireWhileAligning=false`일 때만 `TurretAligning`과 `WeaponNotAligned`를 발사 거부 사유로 사용한다.
11. `MuzzleBlocked`는 옵션과 관계없이 계속 발사를 거부한다.
12. 정렬 중 실제 발사 방향이 현재 Muzzle 방향이면 Muzzle obstruction 검사도 실제 최종 발사 경로를 기준으로 수행한다.
13. 실제 경로 검사 거리는 기존 Reticle 목표 거리 또는 현재 무기 사거리 정책과 충돌하지 않는 최소 변경으로 결정한다.
14. `UCFVehicleAimComp`의 `bLocalCanFire`는 아래 조건에서 정렬 중에도 true가 될 수 있어야 한다.

```text
Aim Solution 유효
bAllowFireWhileAligning=true
MuzzleBlocked=false
유효한 최종 AimDirection 존재
```

15. `LocalReticleState`는 정렬 중 계속 `TurretAligning`을 유지한다. 발사 가능 여부와 시각적 정렬 상태를 분리한다.
16. `UCFTurretMountData::BuildTurretMountSummary()`에 정렬 중 발사 허용 정책을 표시한다.
17. 관련 코드와 문서의 Version / Changelog / Migration을 갱신한다.
18. `Tools\BuildEditor.bat`를 실행한다.

## Out of Scope

```text
- Reticle Recovery Hotfix 제거 또는 되돌리기
- TurretAligning 상태 삭제
- WBP_AimReticle 구조 또는 바인딩 변경
- 신규 BindWidget 추가
- Reticle 이미지 또는 애니메이션 추가
- Projectile 연속 충돌 / 터널링 수정
- Damage / HP 구현
- Ballistic Solver
- 네트워크 / 복제
- 신규 DataAsset 인스턴스 생성
- 기존 DataAsset 인스턴스 저장
- .uasset / .umap / StaticMesh 수정
- Git add / commit / push
```

## Constraints

1. 파일과 클래스 이름은 32자를 넘지 않는다.
2. 변수와 함수 선언/정의 위에 버전이 포함된 한 줄 요약 주석을 작성한다.
3. Blueprint 노출 프로퍼티는 한국어 `DisplayName`과 구체적인 `ToolTip`을 작성한다.
4. C++과 BP 책임을 분리한다. 계산과 발사 판정은 C++에 둔다.
5. 기존 `MuzzleBlocked`, `FireSuccess`, `Cooldown`, `WeaponNotAligned`, `OutOfArcWarning` 의미를 회귀시키지 않는다.
6. `WeaponNotAligned`가 일반 빨간 `FireRejected`로 주 Reticle을 덮지 않는 기존 UI 정책을 유지한다.
7. 정렬 중 발사 허용 상태에서도 Reticle은 amber `TurretAligning` 상태를 유지한다.
8. `bAllowFireWhileAligning` 기본값은 `true`다.
9. 활성 `TurretMountData`가 없을 때 새 정책이 Missing Data 검증을 우회해서는 안 된다.
10. 기존 dirty worktree를 정리, restore, reset, revert하지 않는다.
11. 이전부터 존재하던 `WBP_AimReticle.uasset`과 다른 `.uasset` 변경을 이번 작업 변경으로 오인하지 않는다.
12. 코드 작업 전후 pathspec diff로 이번 변경 범위를 분리한다.
13. PIE PASS는 사용자가 직접 확인하기 전까지 기록하지 않는다.
14. Git commit과 push를 수행하지 않는다.

## Target Files

실제 저장소를 확인한 뒤 아래 파일 중 필요한 최소 파일만 수정한다.

```text
UE/Source/CarFight_Re/Public/CFTurretMountData.h
UE/Source/CarFight_Re/Private/CFTurretMountData.cpp
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

시그니처 또는 public API 변경이 실제로 필요할 때만 아래 헤더를 추가 수정한다.

```text
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
```

아래 파일은 기본적으로 검토 전용이다.

```text
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

## Required Document Updates

```text
Document/Plan/AimFireAlignment/ImplementationDesign.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/ProjectSSOT/05_TestChecklist.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/FireFeedback.md
```

현재 구현 설명이 실제로 변하는 경우에만 아래 문서를 갱신한다.

```text
Document/Systems/UI/AimReticle.md
Document/Plan/VehiclePlatformPlan/CF_CombatDataOwnership.md
```

## Required Code Behavior

### DataAsset

권장 선언 의미:

```cpp
bool bAllowFireWhileAligning = true;
```

ToolTip에는 아래 의미를 명시한다.

```text
True이면 터렛 회전 또는 총구 정렬 중에도 현재 Muzzle 방향으로 발사한다.
False이면 터렛과 총구 정렬이 완료될 때까지 발사를 거부한다.
MuzzleBlocked는 이 옵션과 관계없이 계속 발사를 차단한다.
```

### Aim Solution

아래 방향을 혼동하지 않는다.

```text
DesiredAimDirection
= Muzzle 위치에서 Reticle 목표점으로 향하는 요구 방향

CurrentMuzzleDirection
= 현재 Muzzle Socket X축 방향

AimDirection
= 실제 HitScan / Projectile이 사용할 최종 방향
```

### Fire Validation

```text
MuzzleBlocked -> 항상 거부

bAllowFireWhileAligning=false
  + bTurretAligning=true   -> TurretAligning 거부
  + bWeaponNotAligned=true -> WeaponNotAligned 거부

bAllowFireWhileAligning=true
  + 정렬 중                -> 발사 허용
```

### Reticle

```text
정렬 중 + 발사 허용 여부와 관계없이 LocalReticleState=TurretAligning
정렬 완료 후 Ready
FireSuccess / Cooldown 종료 후 현재 Aim 상태로 복귀
```

## Acceptance Criteria

1. `UCFTurretMountData`에 `bAllowFireWhileAligning`이 존재하고 기본값이 true다.
2. 프로퍼티에 한국어 DisplayName과 동작을 오해하지 않는 ToolTip이 있다.
3. `BuildTurretMountSummary()`에서 현재 정책을 확인할 수 있다.
4. Aim Solution이 요구 방향, 현재 Muzzle 방향, 최종 발사 방향을 구분한다.
5. 정렬 중 발사 허용 true이면 발사 명령이 승인된다.
6. 정렬 중 허용된 탄환은 현재 Muzzle Socket X축 방향으로 진행한다.
7. 정렬 완료 후 탄환은 Reticle 목표점 방향으로 진행한다.
8. 정렬 중 발사 허용 false이면 TurretAligning 또는 WeaponNotAligned로 거부된다.
9. MuzzleBlocked는 true/false 양쪽 정책에서 계속 발사를 거부한다.
10. 정렬 중 Reticle은 사라지지 않고 amber TurretAligning을 유지한다.
11. 허용 true일 때 `bLocalCanFire`가 정렬 중 true가 될 수 있다.
12. FireSuccess, Cooldown, OutOfArcWarning, WeaponNotAligned UI 정책에 회귀가 없다.
13. HitScan과 Projectile이 같은 최종 FireRequest.AimDirection을 사용한다.
14. WBP와 `.uasset` 변경이 없다.
15. `Tools\BuildEditor.bat`가 성공한다.
16. 문서는 Build Complete / PIE Pending으로 기록한다.
17. CF-FQ-022는 Active를 유지한다.
18. CF-TC-019는 PARTIAL을 유지한다.
19. Git commit/push를 수행하지 않는다.

## Verification

### Static Review

```text
- CFTurretMountData에 bAllowFireWhileAligning 기본값 true 존재
- DisplayName / ToolTip / 버전 주석 존재
- BuildTurretMountSummary 정책 표시 존재
- Weapon Aim Solution에 정책과 방향 구분 존재
- BuildWeaponAimSolution 최종 AimDirection 선택 조건 확인
- 정렬 중 허용 발사의 obstruction trace가 실제 최종 경로를 검사하는지 확인
- ValidateFireCommand가 옵션 false일 때만 정렬 상태를 거부하는지 확인
- AimComp bLocalCanFire가 옵션 true를 반영하는지 확인
- BuildFireCommand / HitScan / Projectile 최종 방향 공유 확인
- TurretAligning Reticle 우선순위 유지 확인
- 지정 범위 밖 코드와 .uasset 변경 없음 확인
```

### Build

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

빌드 실패 시 첫 실제 C++ 오류부터 수정하고 다시 실행한다.

### Manual PIE — User Verification Required

`bAllowFireWhileAligning=true`:

```text
1. 터렛을 크게 회전시킨다.
2. Reticle이 amber TurretAligning으로 유지되는지 확인한다.
3. 정렬 중 발사 입력이 승인되는지 확인한다.
4. 탄환이 현재 총구 방향으로 나가는지 확인한다.
5. 정렬 완료 후 탄환이 Reticle 목표점으로 수렴하는지 확인한다.
6. 총구 앞 장애물이 있으면 발사가 거부되는지 확인한다.
7. FireSuccess -> Cooldown -> TurretAligning 또는 Ready 복귀를 확인한다.
```

`bAllowFireWhileAligning=false`:

```text
1. 터렛을 크게 회전시킨다.
2. 정렬 중 발사가 거부되는지 확인한다.
3. Reticle이 사라지지 않는지 확인한다.
4. 정렬 완료 후 Ready로 전환되는지 확인한다.
5. 정렬 완료 후 발사가 승인되는지 확인한다.
```

## Documentation State

```text
CF-FQ-022 = Active 유지
CF-TC-019 = PARTIAL 유지
Align Fire Policy Code/Build = 완료 후 기록
Align Fire Policy PIE = Pending
```

## Failure Conditions

```text
- 정렬 중 허용 발사가 Reticle 방향으로 꺾여 나가면 실패다.
- true인데 TurretAligning 또는 WeaponNotAligned로 계속 거부되면 실패다.
- false인데 정렬 중 발사가 승인되면 실패다.
- MuzzleBlocked를 우회하면 실패다.
- Reticle Recovery Hotfix가 회귀하면 실패다.
- .uasset 또는 WBP를 수정하면 실패다.
- 사용자 확인 없이 PIE PASS를 기록하면 실패다.
- commit/push를 수행하면 실패다.
```

## Unresolved

- 없음.
