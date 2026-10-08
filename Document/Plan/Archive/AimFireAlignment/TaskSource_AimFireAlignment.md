# Codex Task Source — CF-FQ-022 Aim / Turret / Muzzle Alignment

- Task ID: `CF-FQ-022-AIM-FIRE-ALIGNMENT`
- Version: `1.0.0`
- Date: `2026-07-13`
- Priority: `P0`
- Status: `Ready for Codex`
- Source Design: `Document/Plan/AimFireAlignment/ImplementationDesign.md`


## Goal

화면 Reticle이 선택한 월드 목표점을 단일 조준 기준으로 사용하고, 터렛 추적 방향과 Muzzle 기준 실제 HitScan/Projectile 발사 방향을 일치시킨다.

완료 후 동작은 다음과 같아야 한다.

```text
Camera WeaponHit Trace
→ DesiredAimTargetLocation
→ MuzzleWorldLocation에서 목표점으로 DesiredLaunchDirection 계산
→ 터렛이 해당 방향을 추적
→ CurrentMuzzleDirection과의 정렬 오차 계산
→ 정렬 완료 및 Muzzle 경로 정상일 때만 발사 승인
→ HitScan과 Projectile 모두 동일 FireRequest.AimDirection 사용
```


## In Scope

1. Camera Aim Trace를 `CFCollisionChannels::WeaponHit`으로 통일한다.
2. Camera Trace의 Blocking Hit은 목표점 선택 결과로 기록하고, 그 자체를 발사 차단으로 해석하지 않는다.
3. 현재 Aim Trace가 표면에 적중했는지 별도 Debug 상태로 구분한다.
4. `FCFVehicleWeaponAimSolution` 공용 구조체를 추가한다.
5. Pawn이 Muzzle Socket과 Reticle 목표점으로 Weapon Aim Solution을 계산한다.
6. 터렛 추적 방향을 Muzzle 위치에서 목표점으로 향하는 방향으로 통일한다.
7. Muzzle 현재 X축과 요구 발사 방향의 각도 오차를 계산한다.
8. 기존 `UCFTurretMountData::StabilizationToleranceDeg`와 `AimSettleTimeSeconds`, `FCFVehicleTurretState::bTurretSettled`을 정렬 판정에 재사용한다.
9. Muzzle에서 목표점까지 `WeaponHit` Trace를 수행해 가까운 장애물을 검출한다.
10. `BuildFireCommand()`가 최종 `AimOrigin=MuzzleWorldLocation`, `AimDirection=DesiredLaunchDirection`, `PredictedAimTargetLocation=DesiredAimTargetLocation`을 사용한다.
11. `ValidateFireCommand()`가 OutOfArc, WeaponNotAligned, MuzzleBlocked를 구분해 거부한다.
12. HitScan과 Projectile Actor가 같은 FireRequest 방향을 사용한다.
13. `TurretAligning` Reticle 상태와 한국어 표시/색상을 추가한다.
14. VehicleDebug Aim/Weapon 섹션에 Weapon Aim Solution 핵심 값을 표시한다.
15. 코드와 관련 문서의 Version / Changelog / Migration을 갱신한다.
16. Unreal Editor 타깃 빌드를 실행한다.


## Out of Scope

```text
- Projectile 고속 터널링 수정
- Sweep / Sub-stepping / CCD 작업
- 중력 Projectile 탄도 Solver
- 이동 표적 선행 조준
- 자동 락온 / Aim Assist
- 다중 터렛
- 서버 권한 발사 / 복제
- 실제 Damage / HP 감소
- WBP_AimReticle 구조 또는 .uasset 수정
- 별도 Weapon Reticle 이미지 추가
- VFX / SFX / 애니메이션 제작
- 차량 StaticMesh / 맵 / Collision 자산 수정
```


## Constraints

1. UE 최신버전과 현재 `CarFight_Re` 구조를 기준으로 한다.
2. 기존 `LocalAimTargetLocation`은 BP/직렬화 호환을 위해 삭제하거나 이름을 변경하지 않는다. Weapon Aim Solution에서는 같은 값을 `DesiredAimTargetLocation` 의미로 복사해 사용한다.
3. 새 정렬 허용 오차 DataAsset 필드를 만들지 않는다. 현재 `UCFTurretMountData::StabilizationToleranceDeg`를 사용한다.
4. 정렬 완료는 다음 조건을 모두 만족해야 한다.

```text
- 유효한 Muzzle Socket
- 유효한 DesiredAimTargetLocation
- 유효한 DesiredLaunchDirection
- Local Aim이 무기 조준각 내부
- TurretState.bTurretSettled == true
- AimAlignmentErrorDegrees <= StabilizationToleranceDeg
- bMuzzleLineBlocked == false
```

5. Muzzle obstruction 판정은 `WeaponHit` Line Trace를 사용하고 Owner Pawn을 Ignore한다.
6. Camera 목표 표면 자체는 Muzzle obstruction으로 취급하지 않는다. Muzzle Trace HitPoint가 DesiredAimTargetLocation에서 `10cm` 이내이면 목표 표면 도달로 본다.
7. 목표점보다 앞에서 Blocking Hit이 발생하면 `bMuzzleLineBlocked=true`다.
8. Muzzle Socket 또는 Aim Solution이 유효하지 않으면 발사를 안전하게 거부한다. 기존 하드포인트 방향으로 조용히 발사하지 않는다.
9. `OutOfArc`와 `TurretAligning`을 합치지 않는다.

```text
OutOfArc       = 기계적 조준 범위 밖
TurretAligning = 범위 안이지만 현재 총구가 목표 방향에 아직 정렬되지 않음
Blocked        = Muzzle 경로가 목표점보다 앞에서 막힘
```

10. `WeaponNotAligned` 거부는 주 Reticle의 `TurretAligning` 상태를 유지해야 하며 일반 `FireRejected` 빨간색으로 덮어쓰지 않는다.
11. `MuzzleBlocked` 거부는 기존 `AimBlocked` FireFeedback과 `Blocked` 주황 상태를 재사용한다.
12. `OutOfWeaponArc`는 기존 전용 OutOfArc 경고 정책을 유지한다.
13. `TurretAligning` 기본 색상은 WBP에서 조정 가능한 `FLinearColor(1.0f, 0.72f, 0.15f, 1.0f)`로 추가한다.
14. 기존 Optional WBP 바인딩 이름과 빈 BP Graph는 변경하지 않는다.
15. 함수/변수/UPROPERTY/UFUNCTION 선언 및 정의 위에는 프로젝트 규칙의 1줄 요약 주석을 추가한다.
16. 클래스/파일명은 32자 제한을 준수한다.
17. 기존 광범위한 dirty worktree를 존중한다. 지정된 파일 외 변경을 정리하거나 되돌리지 않는다.
18. commit / push를 수행하지 않는다.
19. 빌드 성공만 기록하고 PIE PASS를 주장하지 않는다.
20. 추가 파일이 반드시 필요해지면 임의로 범위를 넓히지 말고 작업을 중단해 이유와 필요한 파일을 보고한다.


## Target Files

### Required C++ modifications

```text
UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

### Required document modifications

```text
ImplementationDesign.md
03_FeatureQueue.md
05_TestChecklist.md
VehicleAim.md
WeaponFire.md
AimReticle.md
```

문서 수정은 위 고유 파일명의 현재 CarFight 문서만 대상으로 한다.

### Read-only references

```text
CFCollisionChannels
CFTurretMountData
CFVehicleWeaponComp
CFVehicleWeaponTypes
CFVehicleFireFeedbackTypes
FireFeedback 시스템 문서
VehicleCoreDecisions 결정 로그
```


## Required Type Changes

### `FCFVehicleCameraRuntimeState`

기존 필드를 삭제하지 말고 다음 상태를 추가한다.

```cpp
bool bAimTraceHasBlockingHit = false;
```

의미:

```text
true  = Camera WeaponHit Trace가 표면을 찾아 DesiredAimTargetLocation을 ImpactPoint로 설정함
false = 최대 Trace 거리 끝점을 DesiredAimTargetLocation으로 사용함
```

`bAimBlocked`는 Camera Trace가 목표 표면에 맞았다는 이유만으로 true가 되면 안 된다. Camera 단계에서 발사 차단을 결정하지 않는다.

### `ECFVehicleReticleState`

```cpp
TurretAligning
```

### `ECFVehicleFireRejectReason`

```cpp
WeaponNotAligned
MuzzleBlocked
```

### `FCFVehicleWeaponAimSolution`

`CFVehicleAimTypes.h`에 BlueprintType 구조체로 추가한다.

필수 필드:

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

모든 필드는 안전한 기본값과 직관적인 DisplayName / ToolTip을 가진다.


## Camera Aim Requirements

`UCFVehicleCameraComp::UpdateAimTrace()`:

```text
TraceStart     = FollowCamera 위치 또는 기존 fallback
TraceDirection = GetCurrentAimDirection()
TraceEnd       = TraceStart + Direction * AimTraceLength
TraceChannel   = CFCollisionChannels::WeaponHit
Ignored Actor  = OwnerActor
```

결과:

```text
bAimTraceHasBlockingHit = Trace 결과
AimHitLocation          = ImpactPoint 또는 TraceEnd
AimTraceDistance        = Hit Distance 또는 AimTraceLength
bAimBlocked             = false
bWeaponCanFireAtCurrentAim = 유효한 방향과 목표점이 있으면 true
```

Debug Line/Sphere는 유지한다. Hit 여부 색상은 목표 획득 Debug일 뿐 발사 차단 색상이라는 주석을 쓰지 않는다.


## Aim Component Requirements

`UCFVehicleAimComp::RefreshLocalAimState()`는 Camera 목표점을 로컬 조준 입력으로 유지한다.

필수 변경:

```text
- LocalAimTargetLocation = CameraRuntimeState.AimHitLocation 유지
- LocalAimDirection은 기존 BP 호환을 위해 유지
- Camera blocking hit 자체를 bLocalAimBlocked로 복사하지 않음
- bLocalAimBlocked는 Pawn이 적용하는 Weapon Aim Solution의 Muzzle obstruction 의미로 갱신
- bLocalCanFire는 Weapon Aim Solution의 bCanFire를 반영
- LocalReticleState는 Hidden / OutOfArc / TurretAligning / Blocked / Ready를 구분
```

공개 API 후보를 다음 의미로 추가한다.

```cpp
void ApplyWeaponAimSolution(const FCFVehicleWeaponAimSolution& InWeaponAimSolution);
FCFVehicleWeaponAimSolution GetWeaponAimSolution() const;
```

AimComp가 `WeaponAimSolution`을 보관하고 Reticle/Debug의 단일 읽기 지점이 되게 한다.

Reticle 우선순위:

```text
1. Runtime 미준비 또는 목표 무효 = Hidden
2. Muzzle 경로 막힘 = Blocked
3. 무기 조준각 밖 = OutOfArc
4. 총구 미정렬 = TurretAligning
5. 발사 가능 = Ready
6. 그 외 = Hidden
```


## Pawn Aim Solution Requirements

`ACFVehiclePawn`에 다음 의미의 함수와 캐시를 추가한다.

```cpp
void RefreshWeaponAimSolution();
bool ResolveMuzzleWorldTransform(FTransform& OutMuzzleWorldTransform) const;
bool TraceMuzzleLineToAimTarget(
    const FVector& MuzzleWorldLocation,
    const FVector& DesiredAimTargetLocation,
    FHitResult& OutHitResult) const;
FCFVehicleWeaponAimSolution LastWeaponAimSolution;
```

실제 함수명은 위 이름을 우선 사용한다.

### Tick ordering

```text
UpdateVehicleTurretAimVisuals(DeltaSeconds)
→ RefreshWeaponAimSolution()
→ Debug/UI가 최신 상태를 읽음
```

`BuildFireCommand()` 시작 시에도 `RefreshWeaponAimSolution()`을 한 번 호출해 입력 이벤트 시점의 최신 Muzzle Transform을 사용한다.

### Desired direction

```cpp
DesiredLaunchDirection =
    (DesiredAimTargetLocation - MuzzleWorldLocation).GetSafeNormal();
```

### Current direction

```cpp
CurrentMuzzleDirection =
    MuzzleSocketWorldTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
```

### Alignment error

```cpp
ClampedDot = Clamp(Dot(CurrentMuzzleDirection, DesiredLaunchDirection), -1, 1)
AimAlignmentErrorDegrees = RadiansToDegrees(Acos(ClampedDot))
```

### Alignment tolerance

```text
Active tolerance = Max(LastTurretMountData->StabilizationToleranceDeg, 0)
Fallback tolerance = 0.5 degrees only when TurretMountData is unavailable
```

TurretMountData 또는 Muzzle이 없는 상태는 `bHasValidMuzzle=false`, `bCanFire=false`이며 발사를 거부한다.

### Muzzle obstruction

```text
Trace = MuzzleWorldLocation → DesiredAimTargetLocation
Channel = WeaponHit
Ignore = this Pawn
```

판정:

```text
No Hit                                = not blocked
HitPoint within 10cm of target point  = target surface reached, not blocked
HitPoint more than 10cm before target = blocked
```

`MuzzleBlockingHitLocation`은 blocked인 경우에만 HitPoint를 저장하고, 아니면 ZeroVector로 초기화한다.

### Turret tracking

`ResolveTurretAimWorldDirection()`은 유효한 Muzzle Transform과 목표점이 있으면 다음 방향을 최우선 사용한다.

```cpp
(DesiredAimTargetLocation - MuzzleWorldLocation).GetSafeNormal()
```

Muzzle을 구할 수 없을 때만 기존 TurretYawPivot / LocalAimDirection fallback을 유지한다.


## Fire Command and Validation Requirements

### `BuildFireCommand()`

Weapon Aim Solution이 유효하면 다음 값을 사용한다.

```text
AimOrigin                   = MuzzleWorldLocation
AimDirection                = DesiredLaunchDirection
PredictedAimTargetLocation  = DesiredAimTargetLocation
WeaponGroupId               = 기존 MountProfileId
```

`TryBuildMuzzleFireOrigin()`은 Muzzle 위치 해결에 사용할 수 있으나 최종 방향을 Muzzle Socket X축으로 덮어쓰면 안 된다.

`FCFVehicleFireOrigin.WorldFireDirection`과 WeaponComp의 기록도 `DesiredLaunchDirection`으로 맞춘다.

### `ValidateFireCommand()`

기존 소유자/런타임/무기/쿨다운 검증을 유지하면서 다음 순서를 추가한다.

```text
1. bWithinWeaponArc == false      → OutOfWeaponArc
2. bMuzzleLineBlocked == true     → MuzzleBlocked
3. bWeaponAlignedToAim == false   → WeaponNotAligned
4. 나머지 기존 검증
5. 승인
```

Invalid Muzzle/Aim Solution은 `InvalidAimOrigin` 또는 `InvalidAimDirection`으로 안전하게 거부한다.

Projectile 경로에서도 `ValidationAimTargetLocation`은 `PredictedAimTargetLocation`을 유지한다. 최대 사거리 끝점으로 바꾸지 않는다.

HitScan과 Projectile 스폰은 기존처럼 `FireCommand.AimOrigin`과 `FireCommand.AimDirection`을 사용한다.


## FireFeedback / Reticle Requirements

### `WeaponNotAligned`

```text
- LastFireResult에는 거부 사유 기록
- BuildFireFeedbackViewData에서 일반 FireRejected 빨간 피드백으로 덮어쓰지 않음
- 기본 Reticle 상태 TurretAligning 유지
```

### `MuzzleBlocked`

```text
- 기존 AimBlocked FireFeedback 상태 재사용
- 최종 Reticle = Blocked
- 주황색 유지
```

### `OutOfWeaponArc`

기존 전용 OutOfArcWarning과 주 Reticle 비덮어쓰기 정책을 유지한다.

### Reticle display

```text
State: 조준: 터렛 정렬 중
Hint : 총구가 조준점을 추적 중
Color: TurretAligningReticleColor
```

새 WBP TextBlock이나 Image는 추가하지 않는다.


## VehicleDebug Requirements

Aim 또는 Weapon 섹션에 최소 다음 값을 표시한다.

```text
DesiredAimTargetLocation
MuzzleWorldLocation
DesiredLaunchDirection
CurrentMuzzleDirection
AimAlignmentErrorDegrees
bWithinWeaponArc
bTurretSettled
bWeaponAlignedToAim
bMuzzleLineBlocked
MuzzleBlockingHitLocation
bCanFire
```

기존 필드 ID 스타일을 유지하고 화면 표시명은 한국어로 작성한다.

`WeaponNotAligned`, `MuzzleBlocked`, `TurretAligning` enum 표시 문자열도 누락 없이 추가한다.


## Documentation Requirements

### `ImplementationDesign.md`

```text
- Version 증가
- 구현 결과와 실제 수정 파일 기록
- Status를 Build Complete / PIE Pending 의미로 변경
- Migration과 Changelog 추가
```

### `03_FeatureQueue.md`

```text
- CF-FQ-022: Ready → Active
- Build 완료와 PIE 대기 상태 기록
- CF-FQ-023과 CF-FQ-018 순서는 유지
```

### `05_TestChecklist.md`

```text
- CF-TC-019: TODO → PARTIAL
- Build 통과만 기록
- PIE 항목은 TODO 유지
```

### Systems 문서

현재 구현을 정확히 반영하되 PIE를 통과했다고 쓰지 않는다.


## Verification

### Static checks

```text
- Camera Aim Trace에 ECC_Visibility가 남아 있지 않음
- 최종 FireRequest.AimDirection을 Muzzle Socket X축으로 덮어쓰지 않음
- WeaponNotAligned / MuzzleBlocked / TurretAligning의 모든 switch 처리 존재
- WBP 바인딩 이름 변경 없음
- 지정 파일 외 변경 없음
```

### Build

Repository root에서 다음을 실행한다.

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

빌드 실패 시 실패 로그의 첫 실제 C++ 오류를 수정하고 재실행한다.
Live Coding 상태를 전제로 성공 처리하지 않는다.

### Post-build report

다음을 보고한다.

```text
- 변경 파일 목록
- 핵심 구현 요약
- BuildEditor.bat 결과
- 남은 PIE 검증 목록
- 추가 변경이 필요한 WBP/BP가 없다는 확인
- commit / push 미수행 확인
```


## Acceptance Criteria

1. Camera Aim Trace가 `WeaponHit`을 사용한다.
2. Camera가 표면에 맞아도 그 사실만으로 AimBlocked가 되지 않는다.
3. `FCFVehicleWeaponAimSolution`이 AimComp에서 최신 상태로 제공된다.
4. 터렛 목표 방향이 Muzzle에서 Reticle 목표점으로 향한다.
5. FireRequest 방향이 `DesiredLaunchDirection`과 같다.
6. HitScan과 Projectile이 같은 방향으로 발사된다.
7. 정렬 중 발사 입력은 `WeaponNotAligned`로 거부된다.
8. Muzzle 앞 장애물은 `MuzzleBlocked`로 거부된다.
9. 기계적 범위 밖은 `OutOfWeaponArc`로 구분된다.
10. Reticle이 `TurretAligning`, `Blocked`, `OutOfArc`, `Ready`를 구분한다.
11. `WeaponNotAligned`가 주 Reticle을 빨간 FireRejected로 덮지 않는다.
12. VehicleDebug에서 목표점, 두 방향, 정렬 오차, 차단 상태를 확인할 수 있다.
13. 기존 FireSuccess → Cooldown → 종료 흐름이 유지된다.
14. 기존 OutOfArc 전용 경고 정책이 유지된다.
15. 기존 WBP 자산 수정 없이 컴파일된다.
16. `BuildEditor.bat`가 성공한다.
17. 문서에는 PIE Pending으로 기록한다.
18. commit / push를 수행하지 않는다.


## Manual PIE Checklist — User Verification Required

Codex는 아래 항목을 PASS 처리하지 않는다.

```text
1. 5m / 20m / 100m 목표에서 Reticle과 탄착 일치
2. 카메라 좌우 오프셋 상태에서 가까운 목표 일치
3. 터렛 회전 중 TurretAligning 표시
4. 정렬 완료 시 Ready 전환
5. 정렬 중 Fire 입력이 발사되지 않음
6. 낮은 벽이 총구 앞에 있을 때 Blocked / MuzzleBlocked
7. OutOfArc와 TurretAligning 구분
8. 차량 주행/선회 중 목표 추적
9. FireSuccess → Cooldown → 종료 회귀 없음
10. HitScan과 Projectile 모드 모두 동일 목표 방향
```


## Migration

```text
- 기존 LocalAimTargetLocation과 FireRequest 필드명은 유지한다.
- ECFVehicleReticleState에 TurretAligning 값이 추가되므로 모든 switch를 갱신한다.
- ECFVehicleFireRejectReason에 WeaponNotAligned / MuzzleBlocked 값이 추가되므로 모든 표시/피드백 switch를 갱신한다.
- WBP_AimReticle에는 신규 바인딩이 필요 없다.
- TurretMountData는 기존 StabilizationToleranceDeg / AimSettleTimeSeconds 값을 그대로 사용한다.
- 기존 DataAsset과 BP의 수동 값 마이그레이션은 필요 없다.
- 코드 변경 후 관련 BP를 열었을 때 enum 변경에 따른 컴파일 경고가 있으면 사용자 PIE 전 BP Compile/Save가 필요하다고 보고한다.
```
