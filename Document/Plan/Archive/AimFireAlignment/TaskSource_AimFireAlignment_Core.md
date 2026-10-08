# Codex Task Source — CF-FQ-022A Aim Fire Alignment Core

- Task ID: `CF-FQ-022A-AIM-FIRE-CORE`
- Version: `1.0.0`
- Date: `2026-07-13`
- Priority: `P0`
- Depends On: none
- Followed By: `CF-FQ-022B-AIM-FIRE-PRESENTATION`

## Goal

Camera Reticle 목표점을 단일 월드 목표로 사용하고, Muzzle 기준 요구 발사 방향·터렛 추적·HitScan/Projectile 방향·발사 거부 판정을 하나의 Weapon Aim Solution으로 통합한다.

## In Scope

1. Camera Aim Trace를 `CFCollisionChannels::WeaponHit`으로 변경한다.
2. Camera Trace Hit은 목표 표면 선택 결과이며 그 자체를 AimBlocked로 처리하지 않는다.
3. `bAimTraceHasBlockingHit`, `TurretAligning`, `WeaponNotAligned`, `MuzzleBlocked`, `FCFVehicleWeaponAimSolution`을 추가한다.
4. AimComp가 Weapon Aim Solution을 저장하고 LocalAimState의 CanFire/Blocked/ReticleState에 반영한다.
5. Pawn이 Muzzle Socket에서 Reticle 목표점으로 향하는 DesiredLaunchDirection을 계산한다.
6. 터렛이 Muzzle 위치에서 목표점으로 향하는 방향을 추적한다.
7. Muzzle X축과 DesiredLaunchDirection의 정렬 오차를 계산한다.
8. 기존 TurretMountData의 StabilizationToleranceDeg, AimSettleTimeSeconds와 TurretState.bTurretSettled을 재사용한다.
9. Muzzle→Target WeaponHit Trace로 총구 앞 장애물을 검출한다.
10. BuildFireCommand, ValidateFireCommand, HitScan, Projectile이 같은 AimOrigin/AimDirection/Target을 사용한다.
11. WeaponNotAligned는 일반 빨간 FireRejected로 덮지 않고 MuzzleBlocked는 기존 AimBlocked 피드백을 사용한다.
12. 관련 코드 파일의 Version / Changelog / Migration을 갱신한다.
13. BuildEditor.bat를 실행한다.

## Out of Scope

```text
- Reticle 색상/한국어 문구/switch 구현
- VehicleDebug Panel 표시 구현
- 문서 상태 갱신
- WBP 또는 .uasset 수정
- Projectile 터널링/Sweep/Sub-stepping
- 탄도 Solver/선행 조준/락온/Aim Assist
- 실제 Damage/HP
- 네트워크/복제
- StaticMesh/맵/Collision 자산 수정
```

## Constraints

1. 기존 `LocalAimTargetLocation`과 FireRequest 필드는 삭제/리네이밍하지 않는다.
2. 새 정렬 허용 오차 DataAsset 필드를 만들지 않는다.
3. Muzzle이 없거나 목표/방향이 무효면 fallback 방향으로 발사하지 말고 안전하게 거부한다.
4. 목표 표면 도달 허용 오차는 10cm다. Muzzle Trace HitPoint가 목표점에서 10cm 이내면 차단이 아니다.
5. OutOfArc, TurretAligning, Blocked 의미를 분리한다.
6. Camera 단계에서는 `bAimBlocked=false`; 실제 차단은 Muzzle obstruction으로 결정한다.
7. `WeaponNotAligned`는 LastFireResult에는 기록하되 BuildFireFeedbackViewData에서 FireRejected 빨간 오버라이드를 만들지 않는다.
8. `MuzzleBlocked`는 기존 AimBlocked FireFeedback으로 매핑한다.
9. `OutOfWeaponArc`는 기존 OutOfArcWarning 정책을 유지한다.
10. 모든 신규 함수/변수/UPROPERTY/UFUNCTION 위에 1줄 요약 주석을 작성한다.
11. 지정된 파일 외 코드/자산을 수정하지 않는다.
12. 광범위한 기존 dirty worktree를 정리하거나 되돌리지 않는다.
13. commit / push 금지.
14. 빌드 성공만 기록하고 PIE PASS를 주장하지 않는다.
15. 추가 C++ 파일이 반드시 필요하면 작업을 중단하고 이유를 보고한다.

## Target Files

```text
UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

## Required Types

### Camera runtime

```cpp
bool bAimTraceHasBlockingHit = false;
```

Camera trace 결과:

```text
TraceChannel = WeaponHit
AimHitLocation = ImpactPoint 또는 TraceEnd
bAimTraceHasBlockingHit = 실제 Hit 여부
bAimBlocked = false
bWeaponCanFireAtCurrentAim = 유효한 방향/목표 여부
```

### Reticle and reject enum

```cpp
ECFVehicleReticleState::TurretAligning
ECFVehicleFireRejectReason::WeaponNotAligned
ECFVehicleFireRejectReason::MuzzleBlocked
```

### Weapon Aim Solution

`FCFVehicleAimTypes.h`에 BlueprintType 구조체를 추가한다.

```cpp
FVector DesiredAimTargetLocation;
FVector MuzzleWorldLocation;
FVector DesiredLaunchDirection;
FVector CurrentMuzzleDirection;
FVector MuzzleBlockingHitLocation;
float AimAlignmentErrorDegrees;
bool bHasValidAimTarget;
bool bHasValidMuzzle;
bool bWithinWeaponArc;
bool bTurretSettled;
bool bWeaponAlignedToAim;
bool bMuzzleLineBlocked;
bool bCanFire;
```

모든 필드에 안전한 기본값과 한국어 DisplayName/ToolTip을 제공한다.

## Aim Component Requirements

다음 의미의 API를 추가한다.

```cpp
void ApplyWeaponAimSolution(const FCFVehicleWeaponAimSolution& InWeaponAimSolution);
FCFVehicleWeaponAimSolution GetWeaponAimSolution() const;
```

AimComp가 최신 Solution을 저장하고 LocalAimState를 다음 우선순위로 갱신한다.

```text
1. Runtime/목표 무효 = Hidden
2. Muzzle line blocked = Blocked
3. Weapon arc 밖 = OutOfArc
4. 총구 미정렬 = TurretAligning
5. bCanFire = Ready
6. 그 외 = Hidden
```

Camera blocking hit 자체를 `bLocalAimBlocked`로 복사하지 않는다.

## Pawn Requirements

다음 의미의 함수/캐시를 추가한다.

```cpp
void RefreshWeaponAimSolution();
bool ResolveMuzzleWorldTransform(FTransform& OutMuzzleWorldTransform) const;
bool TraceMuzzleLineToAimTarget(const FVector& MuzzleWorldLocation, const FVector& DesiredAimTargetLocation, FHitResult& OutHitResult) const;
FCFVehicleWeaponAimSolution LastWeaponAimSolution;
```

Tick 순서:

```text
UpdateVehicleTurretAimVisuals
→ RefreshWeaponAimSolution
→ Debug/UI가 최신 AimComp Solution을 읽음
```

`BuildFireCommand()` 시작에서도 RefreshWeaponAimSolution을 호출한다.

계산식:

```cpp
DesiredLaunchDirection = (DesiredAimTargetLocation - MuzzleWorldLocation).GetSafeNormal();
CurrentMuzzleDirection = MuzzleSocketTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
AimAlignmentErrorDegrees = RadiansToDegrees(Acos(Clamp(Dot(Current, Desired), -1, 1)));
```

정렬 완료 조건:

```text
- Valid target/muzzle/directions
- bWithinWeaponArc
- TurretState.bTurretSettled
- AlignmentError <= Max(StabilizationToleranceDeg, 0)
- Muzzle line not blocked
```

TurretMountData가 없을 때 fallback tolerance는 0.5도지만 `bHasValidMuzzle=false` 또는 필수 장착 데이터 누락이면 발사는 거부한다.

Muzzle trace:

```text
Start = MuzzleWorldLocation
End = DesiredAimTargetLocation
Channel = WeaponHit
Ignore = this Pawn
Hit within 10cm of target = target reached, not blocked
Earlier Hit = blocked
```

`ResolveTurretAimWorldDirection()`은 유효한 Muzzle과 목표가 있으면 Muzzle→Target 방향을 최우선 사용한다.

## Fire Requirements

Weapon Aim Solution이 유효하면:

```text
AimOrigin = MuzzleWorldLocation
AimDirection = DesiredLaunchDirection
PredictedAimTargetLocation = DesiredAimTargetLocation
```

`TryBuildMuzzleFireOrigin()`은 Muzzle 위치를 해결할 수 있지만 최종 방향을 Socket X축으로 덮어쓰면 안 된다. WeaponComp에 기록하는 WorldFireDirection도 DesiredLaunchDirection이어야 한다.

Validate 순서에 다음을 추가한다.

```text
Outside arc → OutOfWeaponArc
Muzzle blocked → MuzzleBlocked
Not aligned → WeaponNotAligned
Invalid solution → InvalidAimOrigin 또는 InvalidAimDirection
```

Projectile 승인 결과의 ValidationAimTargetLocation은 PredictedAimTargetLocation을 유지한다. 최대 사거리 끝점으로 교체하지 않는다.

## Verification

Static:

```text
- Camera Aim Trace에 ECC_Visibility가 남아 있지 않음
- FireRequest.AimDirection을 Muzzle Socket X축으로 최종 덮어쓰지 않음
- 신규 enum의 Core switch/feedback 처리 존재
- Target Files 외 코드/자산 변경 없음
```

Build:

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

실패하면 첫 실제 C++ 오류를 수정하고 재실행한다. Live Coding 성공으로 대체하지 않는다.

## Acceptance Criteria

1. Camera Aim Trace가 WeaponHit을 사용한다.
2. Camera 표면 Hit만으로 AimBlocked가 되지 않는다.
3. AimComp가 최신 Weapon Aim Solution을 제공한다.
4. 터렛 요구 방향이 Muzzle→Reticle Target이다.
5. FireRequest가 Muzzle 위치와 DesiredLaunchDirection을 사용한다.
6. HitScan/Projectile 방향이 동일하다.
7. Outside arc / Muzzle blocked / Not aligned 거부 사유가 구분된다.
8. WeaponNotAligned가 일반 FireRejected 빨간 오버라이드를 만들지 않는다.
9. 기존 FireSuccess/Cooldown/OutOfArcWarning 동작을 손상시키지 않는다.
10. BuildEditor.bat가 성공한다.
11. commit/push를 수행하지 않는다.
12. PIE는 Pending으로 보고한다.

## Migration

```text
- LocalAimTargetLocation과 FireRequest 기존 필드 유지
- 신규 enum 값으로 인해 Presentation Task에서 모든 UI/Debug switch를 추가 갱신해야 함
- 기존 DataAsset 값 변경 없음
- WBP 변경 없음
- Core Task 완료 후 반드시 CF-FQ-022B를 이어서 실행
```
