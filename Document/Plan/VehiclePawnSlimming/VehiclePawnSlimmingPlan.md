# CF-FQ-048 Vehicle Pawn Slimming Plan

- 문서 버전: v0.7.0
- 작성일: 2026-09-06
- 최근 갱신일: 2026-09-15
- 문서 상태: Completed / Historical + Retained Path
- Feature ID: `CF-FQ-048`
- 우선순위: `P2`
- 대표 Plan: `Document/Plan/VehiclePawnSlimming/VehiclePawnSlimmingPlan.md`
- Current next gate: `None` — **VPS-P0 COMPLETE / USER SMOKE PASS / CURRENT SYSTEM PROMOTED / RUNTIMEREAD WAIVED-DEFERRED DUE TO UE MCP UNAVAILABLE**
- 설계 상태: Initial Design Audit Correction + Re-review **PASS / P0 0 / blocking P1 0**; VPS-P0-00 Freeze Review Correction + Re-review **PASS / P0 0 / blocking P1 0**; VPS-P0-01 Correction + Re-review **PASS / P0 0 / blocking P1 0**; VPS-P0-02 Authority Correction + Final Review **PASS / P0 0 / blocking P1 0 / P2 0**; VPS-P0-03 Runtime Extraction + Final Review **PASS / P0 0 / blocking P1 0 / P2 0**; VPS-P0-04 Debug/Header Cleanup + Final Review **PASS / P0 0 / blocking P1 0 / P2 0**; VPS-P0-05 Contract Correction + Re-review **PASS / P0 0 / blocking P1 0 / P2 0 / CONTRACT READY**

---

## 1. 목적

`ACFVehiclePawn`이 차량 Actor 수명주기와 조립점 역할을 넘어 Runtime 초기화, Fire 실행, Vehicle Visual, Turret Visual, Wheel Visual, Debug 집계, Editor authoring 및 다수 cross-component coordination까지 누적한 상태를 단계적으로 축소한다.

이번 Feature의 목표는 단순한 줄 수 감소나 Component 수 증가가 아니다.

```text
ACFVehiclePawn
= Actor/Pawn lifecycle
+ Default Subobject / BP-SCS 조립점
+ Vehicle identity / ICFTargetSelectable
+ Enhanced Input binding
+ 기존 Public API 호환 Facade
+ P0에서 동결된 기존 observable/serialized state authority
```

도메인 동작은 명확한 책임에 따라 기존 Component 또는 신규 내부 Coordinator에 위임한다.

최종 성공 기준은 새로운 차량 Runtime/Visual/Fire 기능을 추가할 때 `CFVehiclePawn.cpp`를 기본 변경 지점으로 사용하지 않아도 되는 구조다.

---

## 2. 현재 문제와 기준선

2026-09-05~06 read-only audit 기준:

- `CFVehiclePawn.h`: 약 1,955 lines
- `CFVehiclePawn.cpp`: 약 6,946 lines
- 합계: 약 8,901 lines
- `ACFVehiclePawn` class declaration은 header 약 line 999에서 시작하며 앞부분에 다수 `FCFVehicleDebug*` DTO가 누적돼 있다.
- 기존 코드에는 이미 Drive, WheelSync, Camera, Aim, Weapon, Fitting, Ammo, Launcher, Health, Defense, CombatFx, TargetSelect, Sensor 등 의미 있는 Domain Component가 존재한다.
- 따라서 문제는 "모든 상태가 Pawn에 있다"가 아니라, **Pawn이 Domain Component 위의 orchestration / visual / fire / debug / editor 책임까지 계속 누적하는 것**이다.

현재 주요 직접 책임:

```text
Lifecycle / Input
VehicleData Runtime Apply
Movement / Wheel Physics
Fitting / Ammo / Launcher / Sensor Runtime bootstrap
Fire Command / Validation / HitScan / Projectile execution
Launcher scheduled-shot callback
Wheel / Turret / Owner Visual
Runtime Ready / Summary
Debug DTO / Aggregation / Text
WITH_EDITOR authoring helpers
```

---

## 3. 최상위 리팩토링 원칙

### 3.1 Strangler Refactoring

기존 Pawn API를 제거한 뒤 호출자를 한꺼번에 바꾸지 않는다.

```text
기존 Pawn API
→ 신규 내부 Component에 위임
→ 기존 Automation / Runtime Apply / Builder / Launcher seam 보존
→ 단계별 검증
→ 마지막에 Pawn 내부 구현 정리
```

### 3.2 Behavior Extraction / Contract State Freeze

P0에서는 **동작 위치는 이동할 수 있지만 외부에서 관찰되거나 Blueprint/Asset 직렬화 계약에 참여하는 기존 상태 Authority는 이동하지 않는다.**

P0에서 Pawn에 고정하는 대표 상태:

```text
VehicleData
VehicleFittingData
bVehicleCoreRuntimeReady
bVehicleCombatRuntimeReady
bVehicleRuntimeReady
LastVehicleRuntimeSummary
NextFireRequestId
LastFireRequest
LastFireResult
LastFireFeedbackStartTimeSeconds
Damage Debug state
Blueprint Editable / Visible UPROPERTY
기존 Public API가 노출하는 observable state
```

신규 Component에 동일 상태를 Mirror로 복제하지 않는다.

반대로 외부 계약이 없는 순수 private implementation cache는 해당 책임 Component로 이동할 수 있다. 예: Wheel visual authored base transform cache. 단 기존 Pawn private test seam은 wrapper로 보존할 수 있다.

### 3.3 Gameplay Semantics Freeze

이번 P0은 구조 리팩토링이다.

- 주행감 변경 금지
- 조향감 변경 금지
- 조준감 변경 금지
- Weapon/Ammo/Launcher 의미 변경 금지
- VehicleData/Fitting 의미 변경 금지
- Runtime Apply 성공/복구 의미 변경 금지
- 주요 Runtime summary / failure semantics의 임의 변경 금지

동작이 달라지면 기능 개선이 아니라 회귀로 취급한다.

### 3.4 Component Explosion 금지

P0 신규 내부 Component는 원칙적으로 다음 3개까지만 허용한다.

```text
UCFVehicleVisualComp
UCFVehicleFireComp
UCFVehicleRuntimeComp
```

새 `VehicleInputComp`, `VehicleCombatComp`, `VehicleDebugComp`는 P0 범위가 아니다.

---

## 4. 목표 구조

```text
ACFVehiclePawn
│
├─ AWheeledVehiclePawn Lifecycle
├─ Default Subobject / BP-SCS composition root
├─ Enhanced Input binding
├─ ICFTargetSelectable
├─ Contract State Authority
├─ Compatibility Facade
│
├── UCFVehicleVisualComp
│    └─ Vehicle / Wheel / Turret / Owner presentation behavior
│
├── UCFVehicleFireComp
│    └─ single accepted-fire execution coordinator
│
├── UCFVehicleRuntimeComp
│    └─ whole-vehicle runtime bootstrap/reconfigure coordinator
│
└── Existing Domain Components
     ├─ Drive
     ├─ WheelSync
     ├─ Camera
     ├─ Aim
     ├─ Weapon
     ├─ Fitting
     ├─ Ammo
     ├─ Launcher
     ├─ Health
     ├─ Defense
     ├─ CombatFx
     ├─ TargetSelect
     └─ Sensor
```

신규 세 Component는 사용자 조립용 기능 Component가 아니라 Pawn 내부 Coordinator다. 기본적으로 `BlueprintSpawnableComponent`로 공개하지 않고 Pawn constructor의 `CreateDefaultSubobject`에서 하나씩만 생성한다.

---

## 5. Lifecycle Freeze

다음 Override의 ownership과 호출 순서는 P0 동안 Pawn에 고정한다.

```text
OnConstruction
PreRegisterAllComponents
BeginPlay
EndPlay
Tick
SetupPlayerInputComponent
```

신규 Coordinator는 독립 BeginPlay/Tick ownership을 가져가지 않는다.

기본 정책:

```text
PrimaryComponentTick.bCanEverTick = false
```

### 5.1 PreRegister 질량 계약

현재 중요한 순서:

```text
ACFVehiclePawn::PreRegisterAllComponents
→ PrepareInitialSortieRuntimeMass
→ Super::PreRegisterAllComponents
```

물리 Component 등록 전에 초기 Fitting Snapshot 질량을 준비해야 하므로 이 순서를 변경하지 않는다.

최종 위임 형태:

```text
Pawn::PreRegisterAllComponents
→ RuntimeComp::PrepareInitialMassBeforePhysics
→ Super::PreRegisterAllComponents
```

### 5.2 Tick ordering

현재 의미 순서를 보존한다.

```text
Super Tick
→ Runtime ready guard
→ Steering update
→ Wheel visual update
→ Owner visual stabilization
→ Owner body stabilization
→ Turret aim visual update
→ Debug display
```

VisualComp는 독립 Tick을 갖지 않고 Pawn이 현재 순서대로 호출한다.

---

## 6. UCFVehicleVisualComp 경계

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleVisualComp.h
UE/Source/CarFight_Re/Private/CFVehicleVisualComp.cpp
```

책임:

```text
Chassis mesh presentation
Wheel mesh / scale / orientation
Wheel authored-transform private cache
Wheel/layout presentation
Turret mesh/pivot presentation
Owner visual stabilization
Owner body visual stabilization
```

P0에서 포함하지 않는 것:

```text
WeaponHit collision
Projectile collision
TargetSelect collision
Damage / hit surface policy
Weapon validation
```

`ConfigureVehicleVisualHitCollision()`은 이름과 달리 gameplay collision 계약을 포함하므로 P0에서 VisualComp로 이동하지 않는다.

### 6.1 BP/SCS fresh resolve

`SM_Body`, `Wheel_Anchor_*`, `Wheel_Mesh_*`는 장기 cached pointer로 간주하지 않는다.

필요 시 current BP/SCS hierarchy에서 fresh resolve하며 `OnConstruction`에서 visual authored cache를 무효화한 뒤 fresh apply한다.

기존 C++ default subobject인 Turret/OwnerVisual component identity와 이름은 유지한다.

### 6.2 Shared turret state

다음 상태는 Visual 전용이 아니라 Fire/Debug도 소비하므로 P0에서 Pawn storage를 유지한다.

```text
LastTurretMountData
bLastTurretMountDataAssigned
bLastTurretVisualAttached
LastTurretVisualSummary
```

VisualComp와 FireComp가 서로 직접 의존하도록 만들지 않는다.

---

## 7. UCFVehicleFireComp 경계

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleFireComp.h
UE/Source/CarFight_Re/Private/CFVehicleFireComp.cpp
```

책임:

```text
Fire Command build
Fire validation
Muzzle resolution
Weapon Aim Solution glue
HitScan execution
Projectile execution
Accepted-fire execution
Scheduled Launcher shot execution
```

기존 Domain Authority는 유지한다.

```text
Weapon state / cooldown / heat / charge → VehicleWeaponComp
Ammo / reload / reservation → VehicleAmmoComp
Ripple / Salvo scheduler → LauncherComp
Aim state → VehicleAimComp
Projectile reuse → ProjectilePoolComp
FX → CombatFxComp
```

### 7.1 Launcher compatibility seam

기존 Launcher 계약은 유지한다.

```text
LauncherComp
→ Pawn::ExecuteScheduledLauncherShot(...)
→ FireComp::ExecuteScheduledLauncherShot(...)
```

`LauncherComp`가 새 FireComp를 직접 알 필요가 없다.

### 7.2 Fire observable state

`LastFireRequest`, `LastFireResult`, `LastFireFeedbackStartTimeSeconds`, Damage Debug state 등은 P0에서 Pawn Authority를 유지한다.

FireComp는 계산/실행 결과를 반환하고 Pawn이 기존 observable state에 commit한다.

`BuildFireFeedbackViewData()`는 UI projection 성격이므로 FireComp P0 책임이 아니다.

---

## 8. UCFVehicleRuntimeComp 경계

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleRuntimeComp.h
UE/Source/CarFight_Re/Private/CFVehicleRuntimeComp.cpp
```

책임:

```text
VehicleData runtime apply orchestration
Movement setup
Wheel physics setup
Fitting prepare/commit coordination
Initial mass prepare/verify behavior
Ammo initialization
Launcher initialization
Health / Defense / Sensor / TargetSelect runtime coordination
Core / Combat readiness calculation
Fitting-dependent runtime refresh behavior
Runtime result/summary material 생성
```

기존 외부 API는 보존한다.

```text
Pawn::InitializeVehicleRuntime()
→ RuntimeComp

Pawn::RefreshFittingDependentRuntime()
→ RuntimeComp
```

RuntimeComp는 새로운 Ready mirror state를 소유하지 않는다. 최종 결과는 Pawn의 기존 Runtime Ready/summary storage에 commit한다.

`ApplyVehicleDataConfig()` 등 기존 Automation이 접근하는 Pawn private seam도 P0에서 wrapper로 유지한다.

---

## 9. Input / Editor / Debug 범위

### 9.1 Input

P0에서 새 Input Component를 만들지 않는다.

다음은 Pawn에 유지한다.

```text
Enhanced Input assets
SetupPlayerInputComponent
Mapping Context registration
얇은 Handle* input entry
현재 VehicleMove / steering ownership semantics
```

Input/Steering ownership 재설계는 P0 완료 뒤 별도 판단한다.

### 9.2 Editor authoring

현재 `WITH_EDITOR`의 `CaptureWheelLayoutFromBodySockets()` 및 `ApplyVehicleLayoutFromDataInEditor()`는 장기적으로 runtime Pawn에서 제거 후보지만 P0에서 즉시 삭제하지 않는다.

CallInEditor / Blueprint / Vehicle Builder 대체 경로 사용처 감사 후 별도 migration으로 다룬다.

### 9.3 Debug

`CFVehicleDebugTypes.h`로 reflected debug DTO를 분리하는 것은 P0 후반에 수행한다.

Debug aggregation을 먼저 분리해 Pawn private 전체를 읽는 God Reader를 만들지 않는다. Visual/Fire/Runtime extraction 뒤 read boundary가 생긴 다음 정리한다.

---

## 10. Test / Compatibility Freeze

다음 기존 seam은 P0에서 유지한다.

```text
Pawn::InitializeVehicleRuntime
Pawn::RefreshFittingDependentRuntime
Pawn::ExecuteAcceptedFireCommand
Pawn::ExecuteScheduledLauncherShot
Pawn::ApplyVehicleDataConfig
Pawn::ApplyVehicleWheelVisualConfig
Pawn::Capture/InvalidateWheelVisualAuthoredBaseTransformsIfNeeded
```

기존 friend automation과 Launcher callback을 새 Component API로 일괄 migration하지 않는다.

Runtime Apply, Runtime Equipment Apply, Builder Step 8, Fitting/Ammo/HUD/Wheel tests가 기존 Pawn facade를 계속 사용할 수 있어야 한다.

---

## 11. 단계

### VPS-P0-00 Contract / State / Lifecycle Freeze — PASS

2026-09-06 current `CFVehiclePawn.h/.cpp`, `CFRuntimeVehicleApply.cpp`, `CFRuntimeEquipApply.cpp`, `CFLauncherComp.cpp`를 기준으로 구현 전 호환 계약을 동결했다.

```text
검증 성격: read-only Source contract audit + Plan 문서화
CF-FQ-048 Source mutation: 0
CF-FQ-048 Asset mutation: 0
Official Build: Not Required — document-only gate
P0 blocker: 0
blocking P1: 0
결론: PASS
```

#### 11.0.1 Freeze 원칙

- P0에서 **Behavior는 추출할 수 있으나 기존 observable/serialized State Authority는 Pawn에서 임의 이동하지 않는다.**
- 신규 `UCFVehicleVisualComp`, `UCFVehicleFireComp`, `UCFVehicleRuntimeComp`에 아래 Pawn 상태의 mirror를 만들지 않는다.
- 기존 Domain Component가 이미 소유하는 상태는 그대로 해당 Component가 Source of Truth다. 신규 Coordinator는 호출·조정만 하며 복제하지 않는다.
- Pawn의 lifecycle override ownership과 아래 호출 순서는 유지한다.
- BP/SCS Component는 필요 시 Pawn에서 **fresh resolve**하며 신규 Component에 장기 `USceneComponent*`/`UStaticMeshComponent*` 캐시를 만들지 않는다.
- `ConfigureVehicleVisualHitCollision()`은 이름에 `Visual`이 있어도 WeaponHit/피격 의미에 관여하므로 순수 Visual responsibility로 이동하지 않는다.
- 기존 Pawn Public API, Runtime Apply seam, Launcher callback, Automation friend/private seam은 compatibility facade로 보존한다.
- 이 Gate에서는 신규 Component 구현을 시작하지 않는다.

#### 11.0.2 Public UPROPERTY / Blueprint 노출 Freeze

아래 이름, 타입 역할, Edit/Visible 범주와 Blueprint ReadOnly/ReadWrite 의미를 P0 호환 계약으로 유지한다.

| 노출 | 동결 대상 |
|---|---|
| `EditAnywhere, BlueprintReadOnly` | `VehicleData`, `VehicleFittingData` |
| `EditDefaultsOnly, BlueprintReadOnly` | `DefaultInputMappingContext`, `InputAction_VehicleMove`, `InputAction_Throttle`, `InputAction_Steering`, `InputAction_Brake`, `InputAction_Handbrake`, `InputAction_Look`, `InputAction_Fire`, `InputAction_SelectTarget`, `InputAction_ClearTarget`, `InputAction_SelectWeapon`, `InputAction_RadarZoom`, `InputAction_StartActiveScan`, `InputAction_StopActiveScan`, `FireSuccessFeedbackDurationSeconds`, `FireRejectedFeedbackDurationSeconds`, `AimReticleWidgetClass`, `TargetSelectWidgetClass` |
| `EditAnywhere, BlueprintReadWrite` — Input/Move | `InputDeviceMode`, `InputDeviceAnalogThreshold`, `InputMappingPriority`, `bAutoRegisterInputMappingContext`, `VehicleMoveInputConfig`, `InputOwnershipHoldTimeSec`, `SteeringDirectionMinMagnitude`, `SteeringLockToLockTimeSec`, `bSmoothLegacySteeringInput`, `bEnableSpeedSteeringLimit`, `SpeedSteeringLimitStartSpeedKmh`, `SpeedSteeringLimitFullSpeedKmh`, `SpeedSteeringLimitMinScale`, `SteeringReturnMinSpeedKmh`, `SteeringReturnMaxSpeedKmh`, `SteeringReturnMinRate`, `SteeringReturnMaxRate` |
| `VisibleInstanceOnly, BlueprintReadOnly` — Input/Move | `LastMoveDirectionIntent`, `LastVehicleMoveInputResult`, `CurrentInputOwnership`, `TargetSteeringInput`, `LegacyTargetSteeringInput`, `CurrentSteeringInput`, `LastSteeringTurnRate`, `LastSteeringReturnRate`, `bSteeringReturningToCenter`, `LastVehicleMoveInputTimeSec`, `LastLegacyAxisInputTimeSec` |
| `VisibleAnywhere, BlueprintReadOnly` — Component | `VehicleDriveComp`, `WheelSyncComp`, `VehicleCameraComp`, `VehicleAimComp`, `VehicleWeaponComp`, `VehicleFittingComp`, `VehicleAmmoComp`, `LauncherComp`, `ProjectilePoolComp`, `VehicleHealthComp`, `VehicleDefenseComp`, `CombatFxComp`, `TargetPointComp`, `TargetSelectComp`, `VehicleSensorComp`, `TurretMountRootComp`, `TurretBaseMeshComp`, `TurretYawPivotComp`, `TurretYawMeshComp`, `TurretPitchPivotComp`, `TurretPitchMeshComp`, `OwnerVisualRootComp` |
| `EditAnywhere, BlueprintReadWrite` — Runtime | `bAutoInitializeOnBeginPlay`, `bEnableWheelVisualTick` |
| `VisibleInstanceOnly, BlueprintReadOnly` — Runtime | `bVehicleCoreRuntimeReady`, `bVehicleCombatRuntimeReady`, `bVehicleRuntimeReady`, `LastVehicleRuntimeSummary` |
| `VisibleInstanceOnly, BlueprintReadOnly` — Fire/Damage | `NextFireRequestId`, `LastFireRequest`, `LastFireResult`, `LastFireFeedbackStartTimeSeconds`, `bHasLastDamageHitContext`, `LastDamageHitContext`, `LastDamageHitContextSummary`, `bHasLastDamageApplyResult`, `LastDamageApplyResult`, `LastDamageApplyResultSummary` |
| `EditAnywhere, BlueprintReadWrite` — Aim/UI/Debug | `bDrawLocalAimTraceDebug`, `LocalAimTraceDebugDuration`, `bShowAimReticle`, `AimReticleZOrder`, `bShowTargetSelectHud`, `TargetSelectHudZOrder`, `bEnableDriveStateOnScreenDebug`, `bEnableVehicleDebugOnScreenMessage`, `DriveStateDebugDisplayMode`, `bShowDriveStateTransitionSummary`, `bShowVehicleDebugHud`, `bShowVehicleDebugPanel`, `bShowVehicleDebugEvents`, `DriveStateDebugMessageDuration` |
| `EditAnywhere, BlueprintReadWrite` — Owner Visual | `bEnableOwnerVisualStabilization`, `OwnerVisualStabilizationInterpSpeed`, `OwnerVisualStabilizationMaxLagDeg`, `bOwnerVisualStabilizeYaw`, `bOwnerVisualStabilizePitchRoll`, `bHideOwnerPhysicsMeshWhenStabilized`, `bEnableOwnerBodyVisualStabilization`, `OwnerBodyVisualInterpSpeed`, `OwnerBodyVisualMaxLagDeg`, `bOwnerBodyVisualStabilizeYaw`, `bOwnerBodyVisualStabilizePitchRoll` |

특히 다음 Legacy 직렬화 슬롯은 현재 Pawn runtime이 직접 소비하지 않더라도 이름/노출을 유지한다.

```text
AimReticleWidgetClass
AimReticleZOrder
TargetSelectWidgetClass
TargetSelectHudZOrder
```

#### 11.0.3 Public UFUNCTION / C++ facade Freeze

다음 Blueprint 호출 표면은 이름, 호출 가능성, 반환 의미를 유지한다.

```text
BlueprintPure
- GetVehicleDriveComp
- GetWheelSyncComp
- GetVehicleCameraComp
- GetVehicleAimComp
- GetVehicleWeaponComp
- GetVehicleFittingComp
- GetVehicleAmmoComp
- GetLauncherComp
- GetProjectilePoolComp
- GetVehicleHealthComp
- GetVehicleDefenseComp
- GetVehicleBodyMeshComponent
- GetTargetPointComp
- GetTargetSelectComp
- GetVehicleSensorComp
- GetLastTurretVisualSummary
- ShouldShowAimReticle
- ShouldShowTargetSelectHud
- GetVehicleSpeed
- GetDriveState
- GetDriveStateSnapshot
- GetVehicleDebugSnapshot
- GetVehicleDebugOverview
- GetVehicleDebugDrive
- GetVehicleDebugInput
- GetVehicleDebugCamera
- GetVehicleDebugAim
- GetVehicleDebugTarget
- GetVehicleDebugWeapon
- GetVehicleDebugRuntime
- ShouldShowVehicleDebugUi
- ShouldShowVehicleDebugHud
- ShouldShowVehicleDebugPanel

BlueprintCallable
- RequestReloadCurrentWeapon
- RequestSelectWeaponIndex
- RequestStartActiveScan
- RequestStopActiveScan
- ConfirmCurrentTargetCandidate
- ClearSelectedTargetManually
- BuildFireFeedbackViewData
- RegisterDefaultInputMappingContext
- InitializeVehicleRuntime
- PrepareWheelSync
- UpdateVehicleWheelVisuals
- SetVehicleThrottleInput
- SetVehicleSteeringInput
- SetVehicleBrakeInput
- SetVehicleHandbrakeInput
- ClearGameplayInputForPause

WITH_EDITOR / CallInEditor + BlueprintCallable
- CaptureWheelLayoutFromBodySockets
- ApplyVehicleLayoutFromDataInEditor
```

다음은 Blueprint UFUNCTION은 아니지만 P0 compatibility C++ facade/seam으로 유지한다.

```text
RefreshFittingDependentRuntime
RecordProjectileDamageHitContextFromPool
IsTargetSelectable_Implementation
GetTargetDisplayInfo_Implementation
GetTargetSelectionLocation_Implementation
GetTargetTrackState_Implementation
```

`InitializeVehicleRuntime()`의 반환값은 **Core Ready** 의미를 유지한다. `RefreshFittingDependentRuntime()`의 반환값은 **Combat Ready** 의미를 유지한다. 이름이 비슷하다는 이유로 두 반환 의미를 통합하지 않는다.

#### 11.0.4 Pawn Contract State Authority Freeze

P0에서 Pawn에 남겨야 하는 상태는 다음과 같다.

```text
A. 직렬화/Blueprint observable state
- 11.0.2에 열거한 Pawn UPROPERTY 전부

B. Runtime readiness / summary authority
- bVehicleCoreRuntimeReady
- bVehicleCombatRuntimeReady
- bVehicleRuntimeReady
- LastVehicleRuntimeSummary

C. Fire observable authority
- NextFireRequestId
- LastFireRequest
- LastFireResult
- LastFireFeedbackStartTimeSeconds
- FireSuccessFeedbackDurationSeconds
- FireRejectedFeedbackDurationSeconds

D. Turret shared/debug authority
- LastTurretMountData
- bLastTurretMountDataAssigned
- LastTurretMountId
- LastTurretMountSummary
- bLastTurretVisualAttached
- LastTurretVisualSummary
- LastTurretBaseMeshName
- LastTurretYawMeshName
- LastTurretPitchMeshName
```

반대로 아래처럼 외부에서 관측되지 않고 Visual behavior 내부 구현에만 쓰이는 private cache/runtime state는 **Contract State Authority가 아니다.** VPS-P0-01에서 Visual behavior와 함께 정확히 한 번 이동할 수 있다.

```text
WheelVisualAuthoredBaseTransforms
bHasCapturedWheelVisualAuthoredBaseTransforms
bOwnerVisualStabilizationReady
SmoothedOwnerVisualRotation
bHasSmoothedOwnerVisualRotation
OwnerVisualStabilizedComponents
bOwnerVisualPhysicsMeshHidden
bOwnerBodyVisualStabilizationReady
SmoothedOwnerBodyVisualRotation
bHasSmoothedOwnerBodyVisualRotation
OriginalOwnerBodyVisualRelativeRotation
bHasOriginalOwnerBodyVisualRelativeRotation
```

이 private state는 Pawn과 신규 Visual Component 양쪽에 동시에 mirror하지 않는다. 이동한다면 lifecycle ordering, Construction invalidation, fresh BP/SCS resolve와 기존 Pawn private test wrapper의 observable 결과를 그대로 보존해야 한다.

기존 Domain Source of Truth는 이동하지 않는다.

```text
VehicleFittingComp = Prepared/Applied Fitting Snapshot
VehicleAmmoComp = loaded/reserve/reload/launcher reservation ammo state
VehicleWeaponComp = active weapon/muzzle/heat/charge/cooldown/turret gameplay state
LauncherComp = sequence scheduling/volley state
VehicleDriveComp = drive state
WheelSyncComp = wheel sync runtime
VehicleAimComp = aim/fire validation visual state
VehicleHealthComp = integrity/destroyed state
VehicleDefenseComp = shield/armor damage distribution state
TargetSelectComp = candidate/selected target state
VehicleSensorComp = sensor contact/knowledge state
```

신규 Coordinator가 위 값을 복제하거나 별도 authoritative bool/summary를 만들면 P0 위반이다.

#### 11.0.5 Default Subobject 이름 / identity Freeze

아래 `CreateDefaultSubobject` identity는 Blueprint serialization/SCS compatibility를 위해 정확히 유지한다.

| Pawn property | Default Subobject name |
|---|---|
| `VehicleDriveComp` | `VehicleDriveComp` |
| `WheelSyncComp` | `WheelSyncComp` |
| `VehicleCameraComp` | `VehicleCameraComp` |
| `VehicleAimComp` | `VehicleAimComp` |
| `VehicleWeaponComp` | `VehicleWeaponComp` |
| `VehicleFittingComp` | `VehicleFittingComp` |
| `VehicleAmmoComp` | `VehicleAmmoComp` |
| `LauncherComp` | `LauncherComp` |
| `ProjectilePoolComp` | `ProjectilePoolComp` |
| `VehicleHealthComp` | `VehicleHealthComp` |
| `VehicleDefenseComp` | `VehicleDefenseComp` |
| `CombatFxComp` | `CombatFxComp` |
| `TargetSelectComp` | `TargetSelectComp` |
| `VehicleSensorComp` | `VehicleSensorComp` |
| `TargetPointComp` | `TargetPoint` |
| `OwnerVisualRootComp` | `OwnerVisualRoot` |
| `TurretMountRootComp` | `Turret_MountRoot` |
| `TurretBaseMeshComp` | `Turret_BaseMesh` |
| `TurretYawPivotComp` | `Turret_YawPivot` |
| `TurretYawMeshComp` | `Turret_YawMesh` |
| `TurretPitchPivotComp` | `Turret_PitchPivot` |
| `TurretPitchMeshComp` | `Turret_PitchMesh` |

`TargetPointComp`의 property 이름과 실제 Default Subobject 이름 `TargetPoint`이 다른 것은 current contract이며 임의 정규화/rename하지 않는다.

#### 11.0.6 BP/SCS component name lookup Freeze

현재 Pawn은 이름 기반 BP/SCS 컴포넌트를 호출 시점에 actor에서 다시 열거해 찾는다.

```text
SM_Body
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR
Wheel_Mesh_FL
Wheel_Mesh_FR
Wheel_Mesh_RL
Wheel_Mesh_RR
```

동결 규칙:

1. `GetVehicleBodyMeshComponent()`는 `GetComponents<UStaticMeshComponent>` 후 exact `SM_Body` 이름을 찾는 semantics를 유지한다.
2. Wheel/Layout Visual helper도 현재 Actor의 Scene/StaticMesh Component를 이름으로 fresh resolve한다.
3. Construction/SCS 재실행 후 stale pointer가 남을 수 있는 신규 장기 component cache를 만들지 않는다.
4. `WheelVisualAuthoredBaseTransforms`처럼 **값(Transform)** 을 보관하는 순수 private cache는 pointer cache와 구분한다. VPS-P0-01에서 Visual behavior와 함께 이동할 수 있지만 Pawn/VisualComp 이중 mirror는 금지하고 `OnConstruction` invalidation + fresh resolve semantics를 유지한다.

#### 11.0.7 Pawn Lifecycle Ownership / 호출 순서 Freeze

`ACFVehiclePawn`이 다음 override를 계속 소유한다.

**OnConstruction**

```text
1. Super::OnConstruction
2. InvalidateWheelVisualAuthoredBaseTransforms
3. ApplyVehicleVisualConfig
4. ApplyVehicleWheelVisualConfig
5. ApplyVehicleLayoutConfig
6. ApplyVehicleTurretVisualConfig
```

**PreRegisterAllComponents**

```text
1. [WITH_EDITOR] PreRegister.BeforePrepare probe
2. bCanPrepareInitialSortieMass 계산
   = !RF_ClassDefaultObject
   && bAutoInitializeOnBeginPlay
   && GetWorld() != nullptr
   && GetWorld()->IsGameWorld()
3. 조건 충족 시 PrepareInitialSortieRuntimeMass
4. [WITH_EDITOR] AfterPrepareBeforeSuper probe
5. Super::PreRegisterAllComponents
6. [WITH_EDITOR] AfterSuper probe
```

초기 질량은 **조건부 Prepare가 Super보다 먼저** 실행되는 ordering이 핵심 계약이다. CDO, Editor Preview, `bAutoInitializeOnBeginPlay == false`인 수동 초기화 Pawn에서는 이 PreRegister mass prepare를 실행하지 않는다.

`PrepareInitialSortieRuntimeMass()`는 Prepared Fitting이 실제 초기 질량 적용을 요구할 때 Chaos Vehicle Movement `Mass`를 물리 등록 전에 적용하고 기록한다. Movement 미해결 또는 mass record 실패 시 이전 Mass 복원/Legacy fallback semantics를 유지한다.

**BeginPlay**

```text
1. Super::BeginPlay
2. [WITH_EDITOR] probe
3. ApplyVehicleSinglePlayerBaseline
4. bCanRunLocalPresentation 계산
5. Local + bAutoRegisterInputMappingContext이면 RegisterDefaultInputMappingContext
6. bAutoInitializeOnBeginPlay이면 InitializeVehicleRuntime
7. [WITH_EDITOR] post-init / delayed probes
8. Pawn은 Reticle/TargetSelect Widget을 직접 생성하지 않음 — UCFUISubsystem ownership 유지
```

**EndPlay**

```text
1. [WITH_EDITOR] EndPlay.BeforeReset probe
2. VehicleFittingComp->ResetFittingRuntimeState
3. VehicleAmmoComp->ResetAmmoRuntime
4. [WITH_EDITOR] EndPlay.AfterResetBeforeSuper probe
5. Super::EndPlay(EndPlayReason)
```

`Super::EndPlay`가 reset 뒤 마지막에 호출되는 current ordering을 유지한다.

**Tick**

```text
1. Super::Tick
2. !bVehicleRuntimeReady이면 DisplayDriveStateOnScreenDebug 후 return
3. NonDedicated + Local이면 UpdateVehicleMoveSteeringInput
4. NonDedicated + bEnableWheelVisualTick이면 UpdateVehicleWheelVisuals
5. UpdateOwnerVisualStabilization
6. UpdateOwnerBodyVisualStabilization
7. UpdateVehicleTurretAimVisuals
8. DisplayDriveStateOnScreenDebug
```

`bVehicleRuntimeReady`는 아래와 같이 CoreReady 별칭이므로 Tick guard 의미도 CoreReady 기준이다.

**SetupPlayerInputComponent**

```text
1. Super::SetupPlayerInputComponent
2. Dedicated 또는 !Local이면 return
3. bAutoRegisterInputMappingContext이면 RegisterDefaultInputMappingContext
4. EnhancedInputComponent cast 실패 시 return
5. VehicleMove Triggered/Completed
6. Throttle Triggered/Completed
7. Steering Triggered/Completed
8. Brake Triggered/Completed
9. Look Triggered/Completed
10. Handbrake Started/Completed
11. optional Fire Started
12. optional SelectTarget Started
13. optional ClearTarget Started
14. optional SelectWeapon Started
15. optional RadarZoom Started
16. optional StartActiveScan Started
17. optional StopActiveScan Started
```

P0 extraction은 helper body를 위임할 수 있으나 override owner와 위 순서/guard를 변경하지 않는다.

#### 11.0.8 `InitializeVehicleRuntime` Freeze

현재 초기화 의미와 주요 ordering을 다음처럼 동결한다.

```text
1. Core/Combat/compat RuntimeReady false + InitializeStarted summary
2. Ammo reset
3. ApplyVehicleVisualConfig
4. ApplyVehicleDataConfig
5. PrepareOwnerVisualStabilization
6. ApplyVehicleLayoutConfig
7. ApplyVehicleTurretVisualConfig
8. Drive cache
9. PrepareWheelSync
10. Aim initialize
11. Health initialize
12. Prepared Fitting reuse 또는 PrepareInitialSortieFitting
13. VerifyInitialSortieRuntimeMass
14. 같은 Prepared Snapshot을 Weapon + Defense에 Commit
15. Weapon readback
16. finite Ammo Runtime 구성
17. Fitting 적용 후 Turret Visual 재적용
18. Launcher initialize
19. Defense / CombatFx / Sensor runtime 준비
20. CoreReady 계산
21. CombatReady 계산
22. bVehicleRuntimeReady = bVehicleCoreRuntimeReady
23. LastVehicleRuntimeSummary 최종 기록
24. return bVehicleRuntimeReady
```

핵심 의미:

- Runtime `VehicleData` 교체 시 새 Chassis가 Layout/WheelSync보다 먼저 적용된다.
- PreRegister에서 준비한 Fitting 입력이 있으면 같은 Prepared Snapshot을 재사용한다.
- Initial Mass 검증을 통과해야 Fitting Weapon/Defense Commit을 허용한다.
- `bVehicleRuntimeReady`는 **CoreReady 호환 별칭**이며 CombatReady가 아니다.
- 함수 반환값도 `bVehicleRuntimeReady`, 즉 **CoreReady**다.
- Sensor 초기화 결과는 현재 Core/CombatReady 식에 직접 포함되지 않는다.

#### 11.0.9 `RefreshFittingDependentRuntime` Freeze

Hot Equipment Apply 이후 전체 Vehicle Runtime을 재초기화하지 않고 Fitting 의존 상태만 다시 구성하는 seam이다.

```text
1. VehicleFittingComp + HasAppliedRuntimeInput 요구
2. 없으면 CombatReady=false,
   Summary="FittingDependentRuntime: Failed, AppliedFittingRuntimeMissing",
   return false
3. Ammo reset/rebuild
4. ApplyVehicleTurretVisualConfig
5. Launcher reinitialize
6. Aim/Weapon/TargetSelect readback
7. CombatReady = CoreReady && Aim && Weapon && Ammo && Launcher && TargetSelect
8. bVehicleRuntimeReady = bVehicleCoreRuntimeReady
9. FittingDependentRuntime summary 기록
10. return bVehicleCombatRuntimeReady
```

실패 2번 경로에서 CoreReady/compat RuntimeReady를 임의로 false로 덮지 않는 current semantics도 유지한다. 즉 이 함수의 반환은 **CombatReady**, `bVehicleRuntimeReady`의 의미는 계속 **CoreReady**다.

#### 11.0.10 Runtime Apply / Runtime Equip Apply seam Freeze

**Runtime Vehicle Apply (`CFRuntimeVehicleApply.cpp`)**

```text
Candidate VehicleData transient duplicate
→ Pawn.VehicleFittingData = nullptr
→ Pawn.VehicleData = transient candidate
→ Pawn.InitializeVehicleRuntime()
→ RuntimeReady + transient identity + Fitting null readback
```

실패 시 기존 `VehicleData`/`VehicleFittingData` pointer identity를 복원하고 `InitializeVehicleRuntime()`을 다시 호출해 recovery를 검증한다. P0 extraction은 이 caller가 Pawn public field/facade를 우회하게 만들지 않는다.

**Runtime Equip Apply (`CFRuntimeEquipApply.cpp`)**

```text
기존 Applied Fitting / Weapon / Defense / Mass checkpoint
→ transient Fitting Snapshot prepare/commit
→ 필요 시 Chaos Mass hot apply
→ Pawn.VehicleFittingData = transient fitting
→ Pawn.RefreshFittingDependentRuntime()
→ GetVehicleDebugRuntime + bVehicleCombatRuntimeReady readback
```

실패 recovery도 기존 Fitting/Mass/source를 복구한 뒤 `RefreshFittingDependentRuntime()`을 다시 호출한다. 신규 Runtime Coordinator가 별도 state authority를 만들어 이 recovery/readback seam을 우회하면 안 된다.

#### 11.0.11 Launcher → Pawn scheduled-shot callback Freeze

`UCFLauncherComp`는 예약된 후속 발사에서 다음 exact private callback을 사용한다.

```text
OwnerVehiclePawn->ExecuteScheduledLauncherShot(
    VolleyId,
    SequenceShotIndex,
    CommandTargetLocation,
    GuidanceTargetActorSnapshot)
```

Pawn callback semantics:

```text
BuildFireCommandForTarget(CommandTargetLocation, true)
→ ValidateFireCommandInternal(..., bIgnoreWeaponCooldown=true)
→ ExecuteAcceptedFireCommand(..., GuidanceTargetActorSnapshot, bAllowProjectileFallback=false)
→ ApplyFireResultInternal(..., bRecordCooldown=false)
→ executed && accepted 반환
```

첫 발사 순간의 Command Target 위치와 Guidance Target Actor Snapshot을 유지하며 후속 발사에서 현재 TargetSelect를 다시 조회해 목표 Actor를 바꾸지 않는다. `VolleyId`/`SequenceShotIndex`가 현재 Pawn body에서 직접 소비되지 않더라도 callback signature를 유지한다.

#### 11.0.12 Automation friend/private seam Freeze

다음 friend 선언은 프로덕션 Public API를 늘리지 않고 current private seam을 검증하기 위한 계약이므로 P0에서 유지한다.

```text
friend class UCFLauncherComp;
friend class FCFAmmoFireTransactionTest;
friend class FCFHUDP006HeatResourceTest;
friend class FCFVDATuningRuntimeApplyTest;
friend class FCFWheelSizeRuntimeVisualTest;
friend class FCFWheelSizeRightFallbackOrientationSpinTest;
```

특히 아래 private seam을 단순 추출 편의를 위해 public으로 승격하거나 테스트를 신규 Component 내부 API로 일괄 migration하지 않는다.

```text
ExecuteAcceptedFireCommand
ExecuteScheduledLauncherShot
ApplyVehicleDataConfig
ApplyVehicleWheelVisualConfig
ApplyVehicleLayoutConfig
CaptureWheelVisualAuthoredBaseTransformsIfNeeded
InvalidateWheelVisualAuthoredBaseTransforms
```

`ApplyVehicleLayoutConfig()`은 `FCFWheelSizeRuntimeVisualTest`가 직접 사용하는 existing private Automation seam이다. VPS-P0-01에서 내부 Wheel Anchor layout behavior를 `UCFVehicleVisualComp`로 위임할 수는 있지만, `ACFVehiclePawn::ApplyVehicleLayoutConfig()` private compatibility wrapper 자체는 유지한다. 이 wrapper는 기존처럼 BP/SCS `Wheel_Anchor_*`를 fresh resolve하는 의미와 `LastVehicleRuntimeSummary` Pawn Authority를 보존해야 하며, Visual Component에 별도 summary mirror를 만들지 않는다.

#### 11.0.13 Fire observable state / semantics Freeze

Pawn의 observable Fire 결과는 다음 순서를 유지한다.

```text
입력 시 LastFireRequest 생성/갱신
→ validation / ammo transaction / 실제 execute
→ ApplyFireResultInternal
   - LastFireRequest = FireCommand
   - LastFireResult = FireResult
   - LastFireFeedbackStartTimeSeconds 갱신
   - Accepted shot마다 Heat 1회 누적
   - Accepted shot마다 Charge 1회 소비
   - 호출 정책에 따라 Cooldown 기록
   - Accepted shot마다 Muzzle sequence advance
   - Fire FX 재생
   - AimComp fire-validation/visual state 갱신
```

`ExecuteAcceptedFireCommand()`은 실행 단계 실패 시 `InOutFireResult.bAccepted`와 `RejectReason`을 변경할 수 있다. 이 최종 결과가 Pawn observable state로 commit되는 의미를 유지한다.

Ripple/Salvo 첫 입력은 첫 발사 순간 Guidance Actor Snapshot을 캡처하고, finite ammo sequence는 첫 발 전에 전체 유효 발수를 예약할 수 있다. 이미 Launcher sequence가 활성 상태이면 current Ammo Action Lock 여부에 따라 `WeaponActionLocked` 또는 `WeaponCooldown` reject를 기록한다.

Scheduled shot도 같은 `LastFireRequest`/`LastFireResult`/feedback/Aim observable surface를 갱신하되 입력 단위 cooldown을 다시 기록하지 않는다.

#### 11.0.14 Runtime Ready / Runtime Summary observable Freeze

```text
bVehicleCoreRuntimeReady  = 차량 기본 주행/물리/내구도/방어/Fitting core readiness
bVehicleCombatRuntimeReady = Core + Aim + Weapon + Ammo + Launcher + TargetSelect readiness
bVehicleRuntimeReady       = bVehicleCoreRuntimeReady의 legacy compatibility alias
```

`GetVehicleDebugSnapshot()`은 다음 observable alias를 유지한다.

```text
DebugSnapshot.bRuntimeReady                    <- bVehicleRuntimeReady
DebugSnapshot.Overview.bRuntimeReady           <- bVehicleRuntimeReady
DebugSnapshot.Runtime.bRuntimeReady            <- bVehicleRuntimeReady
DebugSnapshot.Runtime.RuntimeSummary           <- LastVehicleRuntimeSummary
DebugSnapshot.Runtime.LastInitAttemptSummary   <- LastVehicleRuntimeSummary
DebugSnapshot.Runtime.LastValidationSummary    <- LastVehicleRuntimeSummary
```

따라서 `LastVehicleRuntimeSummary`를 신규 Runtime Component가 별도로 mirror하고 Pawn과 비동기화시키면 안 된다.

주요 summary/failure semantics도 유지한다.

```text
Constructor: Constructed
Initialize 시작: VehicleRuntime: InitializeStarted
Refresh hard fail: FittingDependentRuntime: Failed, AppliedFittingRuntimeMissing
Initialize 최종: VehicleRuntime: Data=..., Fitting=..., Mass=..., Drive=..., WheelSync=...,
                 Aim=..., Weapon=..., Ammo=..., Launcher=..., Health=..., Defense=...,
                 TargetSelect=..., OwnerVisual=..., CoreReady=..., CombatReady=... | ...
Refresh 최종: FittingDependentRuntime: Fitting=..., Aim=..., Weapon=..., Ammo=...,
              Launcher=..., TargetSelect=..., CoreReady=..., CombatReady=... | ...
Ammo 상태 구분: Missing / Failed / Ready / InfiniteCompatibility
Defense 상태 구분: Missing / Ready / LegacyFallback
```

문자열 포맷의 공백까지 ABI처럼 고정한다는 뜻은 아니지만, **상태 분류·실패 원인·Core/Combat 구분·readback 의미를 바꾸지 않는다.** 변경이 필요하면 별도 semantic migration으로 다룬다.

#### 11.0.15 Turret shared state Freeze

Pawn Debug/Blueprint가 함께 읽는 다음 shared turret state는 P0에서 Pawn authority로 유지한다.

```text
LastTurretMountData
bLastTurretMountDataAssigned
LastTurretMountId
LastTurretMountSummary
bLastTurretVisualAttached
LastTurretVisualSummary
LastTurretBaseMeshName
LastTurretYawMeshName
LastTurretPitchMeshName
```

`GetLastTurretVisualSummary()`와 `GetVehicleDebugSnapshot().Weapon`이 이 상태를 관측하므로 Visual Component 추출 시 별도 mirror state를 만들지 않는다. 우선 Pawn-owned state를 갱신하는 behavior delegation 형태로 시작한다.

#### 11.0.16 Gameplay Collision Freeze

`ConfigureVehicleVisualHitCollision()`은 단순 표시 옵션이 아니다.

```text
VehicleMesh: WeaponHit gameplay 채널 무시
SM_Body: 시각 차체이면서 현재 WeaponHit 피격 surface 역할
```

따라서 VPS-P0-01에서 이 함수 전체를 `UCFVehicleVisualComp` 소유로 옮기는 것은 금지한다. Visual mesh assignment와 gameplay hit collision semantics를 분리해서 다룬다.

#### 11.0.17 VPS-P0-00 설계검수

Current Source와 위 Manifest를 다시 대조한 결과:

```text
P0-1 Public/Blueprint 계약 누락                  PASS
P0-2 Contract State Authority 중복/이동 허용       PASS
P0-3 Lifecycle override/order 누락                PASS
P0-4 Initial Mass PreRegister ordering 누락       PASS
P0-5 Runtime Apply/Equip recovery seam 누락       PASS
P0-6 Launcher target snapshot callback 누락       PASS
P0-7 Automation private seam 공개 API 확대        PASS
P0-8 RuntimeReady Core/Combat 의미 혼합            PASS
P0-9 Fire observable/summary semantics 누락        PASS
P0-10 Turret shared observable state 누락          PASS
P0-11 BP/SCS stale pointer cache 허용              PASS
P0-12 Gameplay collision Visual 오분류             PASS
P0-13 ApplyVehicleLayoutConfig Automation seam 누락    PASS
```

검수에서 확인된 주의점은 blocker가 아니라 **반드시 보존해야 할 의도적 비대칭**이다.

1. `TargetPointComp` property의 Default Subobject identity는 `TargetPoint`다.
2. `InitializeVehicleRuntime()`은 CoreReady를 반환하지만 `RefreshFittingDependentRuntime()`은 CombatReady를 반환한다.
3. `bVehicleRuntimeReady`는 CombatReady가 아니라 CoreReady compatibility alias다.
4. `PreRegisterAllComponents()`의 initial mass prepare는 조건부이며 `Super` 이전이다.
5. `EndPlay()`은 Fitting/Ammo reset 뒤 `Super::EndPlay()`을 호출한다.
6. `RefreshFittingDependentRuntime()`의 AppliedFitting missing 실패는 CombatReady를 내리지만 CoreReady/compat ready를 임의 초기화하지 않는다.
7. BP/SCS visual component는 on-demand resolve가 current contract다.
8. `ConfigureVehicleVisualHitCollision()`은 Visual 이름을 갖지만 gameplay hit semantics다.
9. 순수 private Visual cache/runtime state는 Contract State Authority와 구분하며, behavior와 함께 단일 소유권으로 이동할 수 있다. Pawn/VisualComp mirror는 금지한다.
10. `ApplyVehicleLayoutConfig()`은 `FCFWheelSizeRuntimeVisualTest`의 existing private Automation seam이므로 Pawn wrapper를 유지하고, `LastVehicleRuntimeSummary`는 계속 Pawn Authority로 둔다.

최종 판정:

```text
VPS-P0-00: PASS
P0 blockers: 0
blocking P1: 0
Source/Asset mutation: 0
Next exact gate: VPS-P0-01 Visual Behavior Extraction
```

`VPS-P0-01`은 이 Freeze Manifest를 구현 제약으로 사용하며, 별도 구현 착수 전 fresh Git/source diff를 다시 확인한다.

### VPS-P0-01 Visual Behavior Extraction

`UCFVehicleVisualComp` 추가 및 Pawn wrapper 보존.

구현 완료 범위:

- `UCFVehicleVisualComp`를 `VehicleVisualComp` exact Default Subobject identity의 Pawn 내부 coordinator로 추가했다.
- Chassis/Wheel/Layout/Turret/Owner Visual 행동을 Component로 위임하되 기존 Pawn wrapper와 lifecycle 호출 순서는 유지했다.
- Wheel authored base transform, Owner Visual smoothing/ready 등 순수 private Visual value cache는 VisualComp 단일 소유권으로 이동하고 Pawn mirror를 제거했다.
- `ApplyVehicleLayoutConfig()` Pawn private Automation compatibility wrapper, `LastVehicleRuntimeSummary` Pawn Authority, Wheel Anchor Location+Rotation-only 적용을 유지했다.
- Turret shared observable state는 Pawn Authority를 유지하고 VisualComp가 기존 Pawn field를 갱신한다.
- `ConfigureVehicleVisualHitCollision()`과 `SM_Body` gameplay hit collision semantics는 Pawn에 유지했다.
- Product Asset/Blueprint 저장·재저장은 수행하지 않았다.

중간검수에서 blocking P1 1건을 확인했다.

```text
P1-1 UCFVehicleVisualComp::OwnerVisualStabilizedComponents가
      fresh-resolve 대상 BP/SCS SceneComponent 포인터를 장기 보관
```

교정:

- `OwnerVisualStabilizedComponents` 장기 pointer bookkeeping cache 자체를 제거했다.
- `Reset()` / `AddUnique()` 사용을 제거했다.
- 교정 후 `OwnerVisualStabilizedComponents` 저장소 참조는 0건이다.
- Owner Visual 대상은 계속 이름 기반 fresh resolve 후 즉시 사용한다.
- `CFVehicleVisualComp.h/.cpp`는 v1.0.1로 교정 이력을 기록했다.

검증 결과:

- Official UE 5.8 `CarFight_ReEditor Win64 Development` Build **PASS**
  - `CFVehicleVisualComp.cpp`, `CFVehiclePawn.cpp` 개별 compile + `UnrealEditor-CarFight_Re.dll` link PASS
- Wheel/Construction affected:
  - `Registry132` PASS
  - `SchemaUtilityLegacy` PASS
  - `SocketScaleDerived` PASS
  - `ObjectReferenceRoundTrip` PASS
  - `DeferredApplyGuard` PASS
  - `RuntimeVisualFallback` PASS
  - `RightFallbackOrientationSpin` PASS
  - BuilderShell / GameplayGuidance / BuilderProfileCommit PASS
- `RunWheelSizeTests.ps1`의 마지막 bundled `CarFight.VehicleData.VD_P0_01.ValidatorContract`가 한 차례 no-terminal `Condition failed`로 wrapper를 FAIL시켰으나, 이 테스트는 Transient `UCFVehicleData` + `UCFVDAValidator`만 검증하며 P0-01 Visual source를 실행하지 않는다.
- 위 ValidatorContract를 MCP log suppression + exact single-test로 격리 재실행해 **1/1 PASS / failure 0 / missing 0 / unexpected 0 / duplicate 0**을 확인했다. 따라서 P0-01 회귀가 아닌 동일 프로세스 실행 오염으로 분리한다.
- Aim affected `CarFight.UI.UI_P0_09.ViewModeDirectionFoundation` exact single-test **1/1 PASS / failure 0 / missing 0 / unexpected 0 / duplicate 0**
- current test source에는 `ApplyVehicleTurretVisualConfig`, `LastTurretVisualSummary`, `bLastTurretVisualAttached`를 직접 검증하는 기존 Turret Visual Automation이 없다. 이번 Gate에서는 source equivalence review로 Pawn-owned Turret state, attachment flow, aim direction fallback과 wrapper 유지 여부를 재검수했다.
- Product Asset mutation/save **0**

교정 후 재검수:

```text
VPS-P0-01: Correction + Re-review PASS
P0 blockers: 0
blocking P1: 0
P2: 0
Official UE 5.8 Build: PASS
Direct Wheel/Construction affected: PASS
Aim affected exact: PASS
Turret direct Automation: current existing test 없음 / source equivalence review PASS
Product Asset mutation/save: 0
Next exact gate: VPS-P0-02 Fire Behavior Extraction — Not Started
```

### VPS-P0-02 Fire Behavior Extraction

`UCFVehicleFireComp`를 추가하고 기존 Pawn Fire facade / observable state Authority / Launcher callback / Automation seam을 보존했다.

구현 완료 범위:

- `UCFVehicleFireComp`를 `VehicleFireComp` exact Default Subobject identity의 Pawn 내부 coordinator로 추가했다.
- Fire Command 계산, validation, Muzzle/Aim glue, HitScan/Projectile 실행, Ammo transaction coordination, Launcher 후속 발사 실행과 Fire domain side effect orchestration을 FireComp로 위임했다.
- 기존 Pawn private/public compatibility wrapper와 `UCFLauncherComp -> ACFVehiclePawn::ExecuteScheduledLauncherShot()` callback을 유지했다.
- `NextFireRequestId`, `LastFireRequest`, `LastFireResult`, `LastFireFeedbackStartTimeSeconds`와 Damage Debug observable은 Pawn Authority를 유지했다.
- Weapon cooldown/Heat/Charge/Muzzle state는 `VehicleWeaponComp`, Ammo/Reload/Reservation은 `VehicleAmmoComp`, scheduler는 `LauncherComp`, Projectile pool은 `ProjectilePoolComp`, 피해 계산/내구도는 기존 Defense/Health 계층, FX는 `CombatFxComp`, Aim state는 `VehicleAimComp`의 기존 Domain Authority를 유지했다.
- Ripple/Salvo 첫 입력의 Command Target 위치와 Guidance Target Actor Snapshot을 보존하고 scheduled shot에서 current TargetSelect를 다시 조회하지 않는 계약을 유지했다.
- 신규 FireComp에는 BlueprintCallable/BlueprintPure API를 추가하지 않았고 Product Asset/Blueprint 저장·재저장은 수행하지 않았다.

중간검수에서 blocking P1 1건을 확인했다.

```text
P1-1 초기 FireComp가 NextFireRequestId를 직접 증가시키고
     입력 순간 LastFireRequest를 직접 갱신해
     P0 Freeze의 Pawn observable Authority를 침범
```

교정:

- Request ID와 ClientFireTimeSeconds 할당을 Pawn `BuildFireCommand*()` wrapper로 복귀시켰다.
- 입력 순간 `LastFireRequest = BuildFireCommand()` commit을 Pawn `HandleFireStarted()`로 복귀시켰다.
- FireComp는 Pawn이 전달한 Request ID/시간/FireRequest를 계산·검증·실행에만 사용하도록 교정했다.
- scheduled shot도 Pawn `BuildFireCommandForTarget()`을 통해 새 Request ID/시간을 할당받고 첫 발사 순간 target snapshot을 그대로 사용한다.
- 교정 후 `NextFireRequestId++`, `LastFireRequest`, `LastFireResult`, `LastFireFeedbackStartTimeSeconds`의 실제 write Authority는 Pawn에만 남는 것을 source search로 재확인했다.
- `CFVehicleFireComp.h/.cpp`는 v1.0.1, `CFVehiclePawn.h/.cpp`는 v2.169.1로 교정 이력을 기록했다.

검증 결과:

- Official UE 5.8 `CarFight_ReEditor Win64 Development` Build **PASS**
  - Build job `caf81d1766ce41c99b4afb2e6be577ef`
  - UHT + `CFVehicleFireComp.cpp` / `CFVehiclePawn.cpp` compile + `UnrealEditor-CarFight_Re.dll` link PASS
- exact affected Automation **6/6 PASS**
  - `CarFight.Ammo.AMMO_P0_03.FireTransaction` — PASS (`d984ef8dd72242e08fa92102b396a897`)
  - `CarFight.Launcher.LM_P0_03B.SchedulerContract` — PASS (`661d66993e5a472abb0d4fdbcc45401e`)
  - `CarFight.Ammo.AMMO_P0_04.LauncherLock` — PASS (`8ac2978a52714018a2f0756e780c4060`)
  - `CarFight.UI.UI_P0_06.HeatRuntimeResourceContract` — PASS (`03373c36d4d7440e857a8f38f3fc71ef`)
  - `CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract` — PASS (`0c730a17fb384f5a88c5c6ba6662aa76`)
  - `CarFight.Projectile.LM_P0_06.SourceIsolation` — PASS (`edd5ab86d7024ed5839d345c1ecb38db`)
- 각 exact 실행은 `SUCCESS_COUNT=1 / FAILURE_COUNT=0 / MISSING_COUNT=0 / UNEXPECTED_COUNT=0 / DUPLICATE_TERMINAL_COUNT=0`이다.
- FireTransaction은 실제 Pawn `ExecuteAcceptedFireCommand()`의 HitScan Miss 소비, Projectile 실행 실패 Rollback, Direct fallback 소비와 NoAmmo 경로를 검증한다.
- Projectile Launch/SourceIsolation은 Launch Context 전달, Pool/Source collision isolation과 downstream HitContext 계약을 affected regression으로 검증했다.
- current test source에는 `CombatFxComp::PlayFireFx/PlayImpactFx` 또는 `BuildFireFeedbackViewData`/`LastFireFeedbackStartTimeSeconds`를 직접 호출하는 기존 Automation이 없다. 이번 Gate에서는 source equivalence review로 Pawn observable commit 뒤 기존 CombatFx/Aim side effect 순서와 FireFeedback source가 유지되는지 재검수했다.
- Product Asset mutation/save **0**

최종검수:

```text
VPS-P0-02: Authority Correction + Final Review PASS
P0 blockers: 0
blocking P1: 0
P2: 0
Official UE 5.8 Build: PASS
Exact affected Automation: 6/6 PASS
CombatFx / FireFeedback direct Automation: current existing test 없음 / source equivalence review PASS
Product Asset mutation/save: 0
Next exact gate: VPS-P0-03 Runtime Behavior Extraction — Not Started
```

### VPS-P0-03 Runtime Behavior Extraction

`UCFVehicleRuntimeComp`를 추가하되 기존 Pawn Runtime facade, lifecycle owner와 State Authority를 유지한다.

착수 전 계약검수에서 P0 구현 결함은 없었지만, 이 단계의 로컬 구현 경계가 한 문장으로만 표현되어 아래 서로 다른 책임을 RuntimeComp가 임의로 합쳐 소유할 위험을 blocking P1 1건으로 판정했다.

```text
P1-1 P0-03 로컬 extraction contract가 과도하게 넓음
     - PreRegister initial-mass prepare-before-Super seam
     - Initialize / Refresh의 서로 다른 반환·ready semantics
     - ApplyVehicleDataConfig Automation private seam
     - P0-01 Visual wrapper / 기존 Domain Component Authority 경계
     - Runtime Vehicle/Equipment Apply rollback/readback facade
     의 구현 보존 방법이 P0-03 항목 자체에 명시되지 않음
```

교정 후 P0-03 구현 계약:

1. 신규 내부 기본 서브오브젝트 identity는 정확히 `VehicleRuntimeComp`로 사용한다. P0에서는 BlueprintSpawnableComponent, 신규 BlueprintCallable/BlueprintPure getter 또는 별도 public Runtime API를 만들지 않는다.
2. `ACFVehiclePawn`이 `PreRegisterAllComponents`, `BeginPlay`, `EndPlay`, `Tick` 등 Actor/Pawn lifecycle override를 계속 소유한다. RuntimeComp가 Actor lifecycle owner가 되지 않는다.
3. 다음 Pawn compatibility wrapper는 유지한다.
   - `InitializeVehicleRuntime()` — Public/BlueprintCallable, 반환은 CoreReady
   - `RefreshFittingDependentRuntime()` — C++ facade, 반환은 CombatReady
   - `PrepareInitialSortieRuntimeMass()` — PreRegister의 prepare-before-Super 보호 wrapper
   - `VerifyInitialSortieRuntimeMass()` — Physics 등록 이후 Initial Mass 검증 wrapper
   - `ApplyVehicleDataConfig()` — `FCFVDATuningRuntimeApplyTest`가 직접 사용하는 private Automation wrapper
   - `PrepareWheelSync()` 및 P0-01 Visual 관련 기존 Pawn facade
4. RuntimeComp는 `VehicleData`, `VehicleFittingData`, `bVehicleCoreRuntimeReady`, `bVehicleCombatRuntimeReady`, `bVehicleRuntimeReady`, `LastVehicleRuntimeSummary`의 복제 storage를 만들지 않는다. 계산/조정 결과는 기존 Pawn storage에만 반영한다.
5. `bVehicleRuntimeReady = bVehicleCoreRuntimeReady` legacy alias를 유지한다. Initialize는 CoreReady를 반환하고 Refresh는 CombatReady를 반환한다. Refresh의 Applied Fitting missing 실패는 CombatReady만 false로 만들며 CoreReady/compat를 임의 초기화하지 않는다.
6. PreRegister initial-mass 의미를 그대로 유지한다.
   - `!RF_ClassDefaultObject && bAutoInitializeOnBeginPlay && GameWorld`일 때만 prepare
   - `PrepareInitialSortieRuntimeMass()`가 `Super::PreRegisterAllComponents()`보다 먼저 실행
   - Prepared Snapshot이 요구하는 Mass를 Physics State 생성 전 Movement Mass에 기록
   - `VerifyInitialSortieRuntimeMass()`는 Physics 등록 이후, 같은 Prepared Snapshot을 Weapon/Defense에 Commit하기 전에 실행
7. `ApplyVehicleDataConfig()`의 현재 ordering을 유지한다.

```text
Movement
→ Reference
→ Wheel Physics
→ Wheel Visual
→ Turret Visual
→ DriveState config
```

Movement/Reference/WheelPhysics의 큰 implementation body는 RuntimeComp로 이동할 수 있지만 Pawn wrapper를 통한 기존 호출 의미를 보존한다. Wheel/Turret/Owner Visual 행동은 P0-01 VisualComp 책임이며 RuntimeComp는 기존 Pawn Visual wrapper를 통해서만 해당 단계를 호출하고 Visual state/cache를 다시 소유하지 않는다.
8. RuntimeComp는 기존 Domain Component의 authority를 복제하지 않는다.
   - Fitting prepared/applied snapshot = `VehicleFittingComp`
   - Drive state = `VehicleDriveComp`
   - Wheel runtime = `WheelSyncComp`
   - Aim = `VehicleAimComp`
   - Weapon = `VehicleWeaponComp`
   - Ammo = `VehicleAmmoComp`
   - Launcher sequence = `LauncherComp`
   - Health/Defense = `VehicleHealthComp` / `VehicleDefenseComp`
   - Target/Sensor = `TargetSelectComp` / `VehicleSensorComp`
9. `InitializeVehicleRuntime()`의 §11.0.8 24-step order를 그대로 보존한다. 특히 VehicleData apply가 Layout/WheelSync보다 먼저이고, PreRegister prepared fitting reuse → Initial Mass verify → 같은 Snapshot Weapon/Defense commit 순서를 바꾸지 않는다.
10. `RefreshFittingDependentRuntime()`의 §11.0.9 10-step order와 "전체 Vehicle Runtime 재초기화 금지" 의미를 유지한다. Ammo/TurretVisual/Launcher와 readiness readback만 기존 Fitting-dependent 범위에서 재구성한다.
11. `CFRuntimeVehicleApply`와 `CFRuntimeEquipApply`는 계속 Pawn의 public fields/facade를 사용한다. RuntimeComp를 직접 호출하도록 caller를 migration하지 않는다.
    - Vehicle Apply 실패: 이전 VehicleData/Fitting exact pointer 복원 → Pawn `InitializeVehicleRuntime()` recovery
    - Equipment Apply 실패: 이전 Fitting/Mass/source 복원 → Pawn `RefreshFittingDependentRuntime()` recovery
    - ApplyFailed / RecoveryFailed와 final readback 의미를 변경하지 않는다.
12. RuntimeComp가 Pawn private behavior에 접근해야 하면 `friend class UCFVehicleRuntimeComp` 같은 내부 seam만 추가한다. 기존 테스트를 RuntimeComp API로 옮기거나 기존 private seam을 public으로 승격하지 않는다.
13. Product Asset/Blueprint 저장·재저장은 P0-03에서 요구하지 않는다.

구현 후 최소 exact validation matrix:

- Official UE 5.8 Editor Build
- `CarFight.VehicleData.VD_P0_03.RuntimeApplyContract` — `ApplyVehicleDataConfig` private seam/Movement apply
- `CarFight.VehicleData.VD_P0_04.RuntimeReinitializeVisualContract` — Builder Step 8형 same-Pawn VehicleData reinitialize + Visual ordering
- `CarFight.Fitting.FIT_P0_05.InitialMass` — Fitting initial-mass state policy
- `CarFight.Fitting.FIT_P0_05.DefensePIEPipeline` — actual BP Pawn PreRegister Mass → Physics → Initialize → Snapshot Commit
- `CarFight.RuntimeApply.RTA_P0_02.VehicleApplySuccess`
- `CarFight.RuntimeApply.RTA_P0_02.ApplyFailedRecovery`
- `CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess`
- `CarFight.RuntimeApply.RTA_P0_03.ApplyFailedRecovery`
- `CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback` — WheelSync/Layout affected

위 목록은 P0-03 source diff에 직접 영향이 있는 exact test를 기본으로 하며, 새 failure 근거 없이 broad historical suite를 자동 추가하지 않는다.

착수 전 교정 재검수:

```text
VPS-P0-03 Pre-start Contract Review: Correction + Re-review PASS
P0 blockers: 0
blocking P1: 0
P2: 0
Source implementation mutation: 0
Asset mutation/save: 0
Official Build: Not Required — contract/document-only gate
Next exact gate: VPS-P0-03 Runtime Behavior Extraction — Implementation Not Started
```

구현 + 최종검수 결과:

- 신규 내부 coordinator `UCFVehicleRuntimeComp`를 `VehicleRuntimeComp` exact default subobject identity로 추가했다.
- `InitializeVehicleRuntime`, `RefreshFittingDependentRuntime`, Initial Sortie Mass prepare/verify, `ApplyVehicleDataConfig` 실행 본문을 RuntimeComp로 추출했다.
- Pawn은 `PreRegisterAllComponents` / `BeginPlay` lifecycle, Public/BP/Automation facade, Vehicle/Fitting data와 Core/Combat/compat readiness, `LastVehicleRuntimeSummary` observable Authority를 계속 소유한다.
- RuntimeComp는 독립 Tick/BeginPlay, Blueprint callable/pure API와 중복 Runtime state storage를 만들지 않는다.
- Initialize 반환=`CoreReady`, Refresh 반환=`CombatReady`, compat `bVehicleRuntimeReady=CoreReady` 비대칭과 PreRegister prepare-before-Super / BeginPlay verify→same Snapshot Commit 의미를 보존했다.
- Runtime Vehicle/Equipment Apply caller와 rollback/readback facade는 이동시키지 않았고 기존 Pawn wrapper를 그대로 재사용한다.
- 최종 source diff review에서 lifecycle override mutation 0, Public/BP signature migration 0, Domain Authority mirror 0을 확인했다.

Fresh validation:

- Official UE 5.8 Editor Build: **PASS**
  - canonical entry: `Tools/BuildEditor.bat --non-interactive`
  - process job: `bf809fd5646e45439553eb1a680d0dac`
  - exit code: `0`
  - `CFVehiclePawn.cpp` compile, `UnrealEditor-CarFight_Re.dll` link, `Result: Succeeded` 확인
- P0-03 affected Automation exact9 same-process: **9/9 PASS**
  - process job: `f084579f1f3e4603928e8e5e3bf24554`
  - engine exit code `0`
  - success `9`, failure `0`, missing `0`, unexpected `0`, duplicate terminal `0`
  - VehicleData runtime apply/reinitialize, Initial Mass, actual BP Pawn Defense pipeline, Runtime Vehicle/Equipment Apply success+recovery, Wheel Visual fallback 전부 PASS
- 첫 direct PowerShell 배열 전달 시도는 parameter binding 단계에서 Automation 시작 전에 종료되어 제품/테스트 evidence에서 제외했다. 최종 exact9 result가 validation authority다.
- Product Asset mutation/save: `0`

최종 판정:

```text
VPS-P0-03 Runtime Behavior Extraction: Implementation + Final Review PASS
P0 blockers: 0
blocking P1: 0
P2: 0
Next exact gate: VPS-P0-04 Debug / Header Cleanup — Not Started
```

### VPS-P0-04 Debug / Header Cleanup

목표:

- `CFVehiclePawn.h`가 직접 소유하던 VehicleDebug reflected type 선언을 전용 Public type header로 분리한다.
- VehicleDebug를 소비하는 Public header가 Debug type 사용만을 이유로 `CFVehiclePawn.h` 전체에 의존하지 않도록 include 경계를 정리한다.
- 기존 Pawn Debug Blueprint facade, RuntimeApply readback, reflected type identity와 실제 debug aggregation 동작은 변경하지 않는다.

#### Pre-Implementation Contract Review — HOLD

Fresh source/consumer audit 결과:

```text
VPS-P0-04 Pre-Implementation Contract Review
P0 blockers: 0
blocking P1: 3
P2: 0
Verdict: HOLD
Source implementation mutation: 0
Product Asset mutation/save: 0
Official Build: Not Run — contract/document-only gate
P0-03 Build/exact9/final-review baseline: preserved
Next exact gate: VPS-P0-04 Contract Correction + Re-review
```

**P1-1 — DebugTypes extraction dependency boundary가 불완전하다.**

현재 `FCFVehicleDebugOverview`, `FCFVehicleDebugInput` 등 Debug reflected type은 `CFVehiclePawn.h` 안에서 같은 header가 소유하는 입력 reflected type에 의존한다.

보호 대상 입력 type:

- `ECFVehicleInputDeviceMode`
- `ECFVehicleMoveDirectionIntent`
- `ECFVehicleMoveZone`
- `ECFVehicleInputOwnership`
- `FCFVehicleMoveInputConfig`
- `FCFVehicleMoveInputResult`

`CFVehicleDebugTypes.h`가 다시 `CFVehiclePawn.h`를 include하면 Pawn → DebugTypes → Pawn 순환 의존이 생긴다. 반대로 일부 Debug type만 분리하면 header owner 분리의 목적을 달성하지 못하고 UHT/reflection 의존이 불명확해진다.

교정 계약:

- 신규 Public `CFVehicleInputTypes.h`를 input reflected type의 단일 declaration owner로 사용한다.
- 위 기존 enum/struct의 **이름, enum 값, UENUM/USTRUCT 노출, UPROPERTY 구성, 기본값 의미를 변경하지 않고 선언 위치만 이동**한다.
- `CFVehicleDebugTypes.h`는 `CFVehicleInputTypes.h`와 필요한 Domain type header를 직접 include한다.
- 이것은 header/type owner 정리이며 새 `VehicleInputComp`, 입력 상태 이동, 입력 behavior 변경을 뜻하지 않는다.
- `CFVehiclePawn.h`는 두 type header를 include하고 기존 함수/변수 계약을 그대로 유지한다.

**P1-2 — Public consumer include 전환 계약이 없다.**

현재 다음 Public header가 Debug type 때문에 `CFVehiclePawn.h` 전체를 직접 include한다.

- `Public/UI/CFVehicleDebugHudWidget.h`
- `Public/UI/CFVehicleDebugPanelWidget.h`
- `Public/CarFightVehicleUtils.h`

교정 계약:

- Debug HUD/Panel Public header는 `CFVehicleDebugTypes.h`를 직접 include하고 `ACFVehiclePawn`은 가능한 범위에서 forward declaration한다.
- `CarFightVehicleUtils.h`는 Pawn 전체 대신 `CFVehicleDebugTypes.h`를 직접 include하고 기존 Drive type dependency만 별도 유지한다.
- Public header에서 Pawn include를 제거한 뒤 실제 Pawn 메서드를 호출하는 `CFVehicleDebugHudWidget.cpp` / `CFVehicleDebugPanelWidget.cpp` 등 Private translation unit은 `CFVehiclePawn.h`를 **명시적으로** include한다.
- include hygiene는 compile dependency 정리일 뿐 UObject ownership, Widget behavior, RuntimeApply UI behavior를 바꾸지 않는다.

**P1-3 — Reflection / Pawn facade / regression freeze가 부족하다.**

다음 BlueprintPure/Public facade는 기존 위치와 signature를 그대로 유지해야 한다.

- `GetVehicleDebugSnapshot()`
- `GetVehicleDebugOverview()`
- `GetVehicleDebugDrive()`
- `GetVehicleDebugInput()`
- `GetVehicleDebugCamera()`
- `GetVehicleDebugAim()`
- `GetVehicleDebugTarget()`
- `GetVehicleDebugWeapon()`
- `GetVehicleDebugRuntime()`
- `ShouldShowVehicleDebugUi()`
- `ShouldShowVehicleDebugHud()`
- `ShouldShowVehicleDebugPanel()`

추가 freeze:

- `ECFVehicleDebugDisplayMode`와 `FCFVehicleDebugOverview/Drive/Input/Runtime/Camera/Aim/Target/Weapon/Snapshot`의 reflected 이름, 필드명, 필드 타입, Blueprint 노출 의미를 유지한다.
- `BuildVehicleDebugTextSingleLine`, `BuildVehicleDebugTextMultiLine`, `BuildVehicleDebugSummary`, `DisplayDriveStateOnScreenDebug`의 behavior owner는 Pawn에 유지한다.
- `CFRuntimeVehicleApply` / `CFRuntimeEquipApply`가 사용하는 `GetVehicleDebugRuntime()` readback seam을 유지한다.
- P0-04에서 새 Debug coordinator/component를 만들거나 Component public read API를 확장하지 않는다. 이미 존재하는 Component getter/read 경로를 aggregation 내부에서 읽는 현재 behavior만 보존한다.
- Product Blueprint/SCS/Asset migration 또는 resave는 필요하지 않아야 하며 Product Asset mutation/save는 `0`을 유지한다.

Implementation 이후 필수 validation 계약:

- Official UE 5.8 Editor Build — UHT/reflection/header dependency 검증을 위해 **mandatory**.
- P0-03 accepted affected exact9를 그대로 재실행한다.
- VehicleDebug public consumer와 RuntimeApply UI의 직접 소비 경계를 추가해 아래 exact3도 같은 affected run에 포함한다.
  - `CarFight.RuntimeApply.RTA_P0_04.PanelNavigation`
  - `CarFight.RuntimeApply.RTA_P0_04.ExplicitVehicleApply`
  - `CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply`
- 따라서 P0-04 implementation acceptance의 현재 최소 affected set은 **exact12**다.
- Header/type declaration 이동만 수행하는 동안 USER visual smoke는 이 Gate의 필수 조건이 아니다. 전체 USER smoke는 기존 계획대로 VPS-P0-05 Final Integration에서 수행한다.

현재 판정:

```text
VPS-P0-04 Implementation: NOT STARTED
Reason: blocking P1 3 contract gaps
P0-03 accepted evidence: PRESERVED
Next exact gate: VPS-P0-04 Contract Correction + Re-review
```

#### Contract Correction

P1-1~3을 다음 구현 계약으로 교정한다.

**A. `CFVehicleInputTypes.h` declaration owner 계약**

- 신규 Public reflected header `CFVehicleInputTypes.h`를 생성한다.
- 이 header는 `CoreMinimal.h`를 직접 include하고 `CFVehicleInputTypes.generated.h`를 generated include 위치 규칙에 맞게 둔다.
- 다음 reflected declaration의 **단일 declaration owner**가 된다.
  - `ECFVehicleInputDeviceMode`
  - `ECFVehicleMoveDirectionIntent`
  - `ECFVehicleMoveZone`
  - `ECFVehicleInputOwnership`
  - `FCFVehicleMoveInputConfig`
  - `FCFVehicleMoveInputResult`
- C++ 이름, enum 값, `UENUM/USTRUCT(BlueprintType)`, `UPROPERTY` 이름·타입·Category·meta·ToolTip과 기본값 의미는 변경하지 않고 선언 위치만 이동한다.
- `CFVehiclePawn.h`를 include하지 않는다.
- 새 Component, 새 Runtime state, 입력 해석 behavior 또는 입력 Authority를 추가·이동하지 않는다.

**B. `CFVehicleDebugTypes.h` declaration owner / direct dependency 계약**

- 신규 Public reflected header `CFVehicleDebugTypes.h`를 생성한다.
- Debug reflected declaration이 값으로 직접 사용하는 타입 owner만 직접 include한다.
  - `CoreMinimal.h`
  - `CFVehicleInputTypes.h`
  - `CFVehicleDriveComp.h`
  - `CFVehicleAimTypes.h`
  - `CFVehicleCameraTypes.h`
  - `CFDamageTypes.h`
  - `CFVehicleWeaponTypes.h`
  - `CFTargetSelectTypes.h`
  - `CFVehicleDebugTypes.generated.h`
- `CFVehiclePawn.h`를 include하지 않는다. Pawn 또는 다른 transitive include에 의존해 reflected value type을 얻지 않는다.
- `FCFVehicleDebugWeapon`이 포인터로만 보유하는 `UCFEquipmentPresetData`, `UCFTurretMountData`, `UCFWeaponData`, `UCFProjectileData`, `UCFDamageData`는 우선 forward declaration으로 유지한다. UHT/compile이 complete type을 요구하면 Pawn include를 되살리지 않고 해당 타입의 최소 direct owning header만 추가한다.
- 다음 declaration의 단일 owner가 된다.
  - `ECFVehicleDebugDisplayMode`
  - `FCFVehicleDebugOverview`
  - `FCFVehicleDebugDrive`
  - `FCFVehicleDebugInput`
  - `FCFVehicleDebugRuntime`
  - `FCFVehicleDebugCamera`
  - `FCFVehicleDebugAim`
  - `FCFVehicleDebugTarget`
  - `FCFVehicleDebugWeapon`
  - `FCFVehicleDebugSnapshot`
- reflected 이름, 필드 이름·타입·순서 의미, Blueprint 노출, Category/meta/ToolTip과 기본값 의미는 변경하지 않는다.
- reflected type/property rename이 아니므로 P0-04에서는 새 `CoreRedirects`를 만들지 않는다.
- Debug coordinator/component, Debug state owner, 새 public read API를 추가하지 않는다.

**C. `CFVehiclePawn.h` 소비 계약**

- `CFVehiclePawn.h`는 `CFVehicleInputTypes.h`와 `CFVehicleDebugTypes.h`를 직접 include한다.
- 이동된 input/debug declaration의 중복 정의만 Pawn header에서 제거한다.
- Pawn class 내부의 기존 UPROPERTY/UFUNCTION, C++ 함수 signature, Blueprint facade, lifecycle, observable/serialized state Authority와 Debug aggregation behavior는 그대로 유지한다.
- P0-04를 이유로 Pawn의 다른 Domain include를 공격적으로 정리하지 않는다. 직접 필요성이 컴파일로 증명되는 include는 유지하며 scope를 header/type owner 정리로 제한한다.

**D. Public consumer include 계약**

- `Public/UI/CFVehicleDebugHudWidget.h`와 `Public/UI/CFVehicleDebugPanelWidget.h`는 `CFVehicleDebugTypes.h`를 직접 include하고 `ACFVehiclePawn`은 forward declaration한다.
- 두 Widget `.cpp`는 Pawn 함수를 실제 호출하므로 `CFVehiclePawn.h`를 명시적으로 include한다.
- `Public/CarFightVehicleUtils.h`는 기존 `CFVehicleDriveComp.h` 직접 dependency를 유지하고 `CFVehiclePawn.h` 대신 `CFVehicleDebugTypes.h`를 직접 include한다.
- include cleanup은 compile dependency 정리만 수행하며 Widget ownership, Viewport lifecycle, RuntimeApply UI behavior와 UObject 수명을 변경하지 않는다.

**E. Pawn Debug facade / RuntimeApply readback freeze**

- v0.5.1에서 열거한 `GetVehicleDebug*()`와 `ShouldShowVehicleDebug*()` Public/Blueprint signature를 모두 그대로 유지한다.
- `BuildVehicleDebugTextSingleLine`, `BuildVehicleDebugTextMultiLine`, `BuildVehicleDebugSummary`, `DisplayDriveStateOnScreenDebug` behavior owner는 Pawn에 유지한다.
- `CFRuntimeVehicleApply`와 `CFRuntimeEquipApply`가 성공/실패/복구 뒤 `GetVehicleDebugRuntime()`의 `bRuntimeReady`와 `RuntimeSummary`를 읽는 현재 readback seam을 그대로 유지한다.
- Product Blueprint/SCS graph, serialized property name, Asset migration/resave는 `0`이어야 한다.

**F. Implementation acceptance — affected exact12**

Implementation 뒤에는 다음을 하나의 최소 affected contract로 검증한다.

```text
Official UE 5.8 Editor Build
- UHT / reflection declaration relocation
- include dependency
- compile / link

P0-03 preserved affected exact9
1. CarFight.VehicleData.VD_P0_03.RuntimeApplyContract
2. CarFight.VehicleData.VD_P0_04.RuntimeReinitializeVisualContract
3. CarFight.Fitting.FIT_P0_05.InitialMass
4. CarFight.Fitting.FIT_P0_05.DefensePIEPipeline
5. CarFight.RuntimeApply.RTA_P0_02.VehicleApplySuccess
6. CarFight.RuntimeApply.RTA_P0_02.ApplyFailedRecovery
7. CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess
8. CarFight.RuntimeApply.RTA_P0_03.ApplyFailedRecovery
9. CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback

P0-04 direct consumer exact3
10. CarFight.RuntimeApply.RTA_P0_04.PanelNavigation
11. CarFight.RuntimeApply.RTA_P0_04.ExplicitVehicleApply
12. CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply
```

- exact12는 가능하면 기존 exact-list same-process runner로 실행한다.
- acceptance는 success `12`, failure `0`, missing `0`, unexpected `0`, duplicate terminal `0`을 요구한다.
- Build PASS 전에는 reflected declaration relocation을 PASS로 보지 않는다.
- exact12 PASS 뒤에도 source review에서 InputTypes/DebugTypes→Pawn include `0`, Public Debug consumer의 불필요한 Pawn include `0`, duplicate reflected declaration `0`, Pawn facade signature 변화 `0`, RuntimeApply `GetVehicleDebugRuntime()` readback 유지 여부를 확인한다.
- USER visual smoke는 P0-04 필수 Gate가 아니며 VPS-P0-05 Final Integration에서 수행한다.

#### Contract Correction + Re-review — PASS

Fresh current source/consumer/test 재대조 결과:

- Input reflected declaration과 Debug reflected declaration의 owner를 위 A/B로 분리하면 Pawn→DebugTypes→Pawn 순환 dependency 없이 완결된다.
- Debug reflected value field가 요구하는 실제 direct owner를 `CFVehicleDriveComp.h`, `CFVehicleAimTypes.h`, `CFVehicleCameraTypes.h`, `CFDamageTypes.h`, `CFVehicleWeaponTypes.h`, `CFTargetSelectTypes.h`로 확인했다.
- HUD/Panel Public header는 DebugTypes + Pawn forward declaration으로 충분하고, Pawn 메서드 호출은 Private `.cpp`의 explicit Pawn include로 닫을 수 있음을 확인했다.
- `CarFightVehicleUtils.h`는 Drive type과 Debug Snapshot만 필요하므로 Pawn 전체 include가 계약상 필요하지 않음을 확인했다.
- `CFRuntimeVehicleApply` / `CFRuntimeEquipApply`의 현재 `GetVehicleDebugRuntime()` readback 사용을 재확인했다.
- RuntimeApply UI exact3의 실제 Automation 이름이 current source에 존재함을 확인해 exact12 목록을 추측 없이 고정했다.
- 새 Input/Debug Component, 새 state Authority, Public API 확대, Blueprint/SCS/Asset migration은 필요하지 않다.

```text
VPS-P0-04 Contract Correction + Re-review
P0 blockers: 0
blocking P1: 0
P2: 0
Verdict: PASS / CONTRACT READY
Source implementation mutation: 0
Product Asset mutation/save: 0
Official Build: Not Run — contract/document-only gate
Automation: Not Run — contract/document-only gate
P0-03 accepted Build/exact9/final-review evidence: PRESERVED
Next exact gate: VPS-P0-04 Debug / Header Cleanup — Implementation Not Started
```

#### Implementation + Fresh Validation + Final Review — PASS

구현 결과:

- 신규 Public `CFVehicleInputTypes.h v1.0.0`을 생성하고 기존 Pawn header의 input reflected declaration 4 enum + 2 struct를 이름/값/UPROPERTY/meta/default 의미 변경 없이 이동했다.
- 신규 Public `CFVehicleDebugTypes.h v1.0.0`을 생성하고 Debug display enum + Debug struct 9개를 단일 declaration owner로 이동했다.
- `CFVehicleDebugTypes.h`는 `CFVehicleInputTypes.h`, Drive/Aim/Camera/Damage/Weapon/TargetSelect type owner를 직접 include하고 `CFVehiclePawn.h` 의존은 0으로 유지했다. DataAsset object pointer type은 forward declaration으로 유지했다.
- `CFVehiclePawn.h`는 v2.171.0으로 전진하고 두 전용 type header를 직접 include하며 이동된 declaration의 중복 정의를 제거했다. Pawn class의 UPROPERTY/UFUNCTION, Debug facade, lifecycle과 observable state Authority는 변경하지 않았다.
- `CFVehicleDebugHudWidget.h v1.1.0` / `CFVehicleDebugPanelWidget.h v1.35.0`은 `CFVehicleDebugTypes.h`를 직접 include하고 `ACFVehiclePawn`을 forward declaration한다. 실제 Pawn facade 호출은 각 Private `.cpp`에서 `CFVehiclePawn.h`를 explicit include한다.
- `CarFightVehicleUtils.h v1.4.0`은 기존 Drive type direct dependency를 유지하면서 Pawn 전체 include 대신 `CFVehicleDebugTypes.h`를 직접 소비한다.
- 새 Input/Debug Component, Debug state Authority, Public read API, CoreRedirect, Product Blueprint/SCS mutation과 Asset resave는 추가하지 않았다.

Fresh Official Build:

- UE 5.8 Source Build / `CarFight_ReEditor Win64 Development` **PASS**.
- build job: `241ef0b05aa9455c94ca562fc93d81cc`.
- canonical build log에서 `D:\UnrealEngine_Source`, UHT file-change invalidation, UHT/compile/link와 terminal `Result: Succeeded`를 확인했다.
- build status 조회 lane 한도 도달 뒤에도 중복 build를 시작하지 않았고, 같은 existing build job의 managed log를 read-only observer로 관찰해 terminal success를 회수했다. 이 observer는 Build producer가 아니며 종료 후 quarantine했다.
- 기존 `Tripo3DUEBridge` Interchange dependency warning은 이번 변경과 무관한 non-blocking 기존 경고다.

Fresh affected Automation exact12:

- exact-list same-process process job: `7f9806d89c98491ca4ee89cdec95ae2b`.
- `ENGINE_EXIT_CODE=0`.
- success `12`, failure `0`, missing `0`, unexpected `0`, duplicate terminal `0`.
- P0-03 preserved exact9 + RuntimeApply UI direct consumer exact3를 모두 PASS했다.

```text
PASS CarFight.VehicleData.VD_P0_03.RuntimeApplyContract
PASS CarFight.VehicleData.VD_P0_04.RuntimeReinitializeVisualContract
PASS CarFight.Fitting.FIT_P0_05.InitialMass
PASS CarFight.Fitting.FIT_P0_05.DefensePIEPipeline
PASS CarFight.RuntimeApply.RTA_P0_02.VehicleApplySuccess
PASS CarFight.RuntimeApply.RTA_P0_02.ApplyFailedRecovery
PASS CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess
PASS CarFight.RuntimeApply.RTA_P0_03.ApplyFailedRecovery
PASS CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback
PASS CarFight.RuntimeApply.RTA_P0_04.PanelNavigation
PASS CarFight.RuntimeApply.RTA_P0_04.ExplicitVehicleApply
PASS CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply
```

Final source re-review:

- Input reflected declaration 6개는 `CFVehicleInputTypes.h` exact single owner, duplicate declaration `0`.
- Debug reflected declaration 10개는 `CFVehicleDebugTypes.h` exact single owner, duplicate declaration `0`.
- `UE/Source/CarFight_Re/Public/**/*.h`의 direct `#include "CFVehiclePawn.h"`는 fresh search 기준 `0`이다.
- Pawn Public/Blueprint Debug facade는 `GetVehicleDebug*` exact9 + `ShouldShowVehicleDebug*` exact3 signature 그대로 유지된다.
- `CFRuntimeVehicleApply`와 `CFRuntimeEquipApply`는 계속 `GetVehicleDebugRuntime()`의 `bRuntimeReady` / `RuntimeSummary` readback seam을 사용한다.
- `BuildVehicleDebugTextSingleLine`, `BuildVehicleDebugTextMultiLine`, `BuildVehicleDebugSummary`, `DisplayDriveStateOnScreenDebug` behavior owner는 계속 Pawn이다.
- Product Asset mutation/save `0`, USER visual smoke는 계약대로 P0-04 필수 Gate가 아니며 P0-05 Final Integration에서 수행한다.
- 작업 전용 header-split / build-observer / exact12 helper는 공용 도구로 승격하지 않고 모두 quarantine해 worktree residue `0`이다.

최종 판정:

```text
VPS-P0-04 Debug / Header Cleanup: Implementation + Fresh Validation + Final Review PASS
P0 blockers: 0
blocking P1: 0
P2: 0
Official UE 5.8 Build: PASS
Affected Automation exact12: 12/12 PASS
Product Asset mutation/save: 0
Next exact gate: VPS-P0-05 Pawn Facade Cleanup / Final Integration — Not Started
```

### VPS-P0-05 Pawn Facade Cleanup / Final Integration

남은 Pawn 책임을 최종 재분류한다.

목표:

```text
Lifecycle
Composition root
Input entry
Target identity
Compatibility facade
Contract state
```

최종 affected union regression과 짧은 USER smoke를 수행한다.

#### Pre-Implementation Contract Review — HOLD

P0-00~04 accepted source/evidence와 current `CFVehiclePawn.h/.cpp`, Visual/Fire/Runtime coordinator 경계를 fresh 재대조한 결과 **새 P0 구현 결함은 발견하지 않았지만 P0-05 자체의 마감 계약이 아직 구현 착수를 허용할 만큼 구체적이지 않은 blocking P1 3건**을 확인했다.

```text
VPS-P0-05 Pre-Implementation Contract Review
P0 blockers: 0
blocking P1: 3
P2: 0
Verdict: HOLD
Source implementation mutation: 0
Product Asset mutation/save: 0
Official Build: Not Run — contract/document-only gate
Automation: Not Run — contract/document-only gate
P0-00~04 accepted evidence: PRESERVED
Next exact gate: VPS-P0-05 Contract Correction + Re-review
```

**P1-1 — 최종 Pawn responsibility/facade 분류와 "삭제하지 말아야 할 얇은 wrapper" 계약이 불완전하다.**

Fresh current source audit에서 P0-01~04 이후 Pawn에 남은 큰 책임은 대부분 초기 Design Audit이 의도한 최종 Pawn 역할과 일치한다.

현재 Pawn에 **남아야 하는 최종 책임**:

- **Lifecycle owner** — `OnConstruction`, `PreRegisterAllComponents`, `BeginPlay`, `EndPlay`, `Tick`, `SetupPlayerInputComponent`.
- **Composition root** — Default Subobject identity, BP/SCS component 연결, component 간 조립 진입점.
- **Input entry** — Enhanced Input binding과 `Handle*` 입력 진입, `RequestReloadCurrentWeapon`, `RequestSelectWeaponIndex`, `RequestStartActiveScan`, `RequestStopActiveScan` 같은 Pawn-level gameplay command facade. P0에서는 새 `VehicleInputComp`를 만들지 않는다.
- **Target identity** — `ICFTargetSelectable` 구현, `TargetPointComp`, 차량의 Target display/location/track identity와 선택/해제 command facade.
- **Compatibility facade** — 이미 추출된 Visual/Fire/Runtime behavior에 대한 기존 Public/BP/Automation/private seam wrapper. 본문이 한 줄 delegation이라는 이유만으로 삭제하지 않는다.
- **Contract state Authority** — Vehicle/Fitting data pointer, Core/Combat/compat Ready, Runtime summary, Fire/Turret/Debug 등 P0-00 Freeze가 Pawn observable/serialized/shared state로 고정한 상태.

추가 이동 금지 경계:

- `BuildVehicleDebugTextSingleLine`, `BuildVehicleDebugTextMultiLine`, `BuildVehicleDebugSummary`, `GetVehicleDebug*`, `ShouldShowVehicleDebug*`는 P0-04에서 Pawn facade/aggregation owner로 명시적으로 보존됐다. P0-05에서 새 Debug coordinator를 만들지 않는다.
- `ConfigureVehicleVisualHitCollision()`은 이름에 Visual이 포함돼도 `WeaponHit`/`Projectile`/`TargetSelect` gameplay collision contract를 소유하므로 VisualComp로 옮기지 않는다.
- `CaptureWheelLayoutFromBodySockets()` / `ApplyVehicleLayoutFromDataInEditor()` 등 `WITH_EDITOR` authoring seam은 P0의 Editor authoring 제거 범위가 아니므로 line-count 정리를 이유로 이동/삭제하지 않는다.
- `ResolveVehicleMoveInput`, steering/input ownership과 Tick의 neutral-return 처리는 P0에서 Input behavior 재설계 대상이 아니다.

교정 요구:

- P0-05는 **추가 extraction을 필수 성과로 요구하지 않는다**.
- source call-graph와 Freeze Manifest상 실제 잔여 misplaced behavior가 증명되지 않으면 **Product source mutation 0도 정상적인 최종 구현 결과**로 허용한다.
- 기존 wrapper/facade 삭제는 `private-only + external/BP/Automation/RuntimeApply/lifecycle consumer 0 + Freeze seam 아님 + existing coordinator가 behavior owner`가 모두 증명된 경우만 허용한다.
- 줄 수 감소는 Acceptance가 아니며, "마지막 단계니까 더 줄인다"는 이유로 API/상태/authoring seam을 제거하지 않는다.

**P1-2 — 최종 affected union regression이 exact-name/dedupe 기준으로 동결되지 않았다.**

현재 Plan은 각 단계의 accepted affected evidence를 보존하지만 P0-05에는 이를 합친 **최종 union의 정확한 테스트 이름·중복 제거 규칙·합격 count**가 없다. 이 상태에서 구현 뒤 broad suite를 임의 추가하거나 반대로 특정 단계 회귀를 누락할 위험이 있다.

교정 요구:

- P0-01 Visual accepted affected set, P0-02 Fire accepted exact6, P0-03 Runtime accepted exact9, P0-04 Header/RuntimeApply accepted exact12를 **실제 current Automation registration exact name으로 다시 resolve**한다.
- 같은 exact test는 한 번만 실행하며, 특히 P0-03 exact9가 P0-04 exact12에 이미 포함되는 중복을 제거한다.
- historical no-terminal contamination, 이미 isolated PASS로 분리된 runner artifact, 직접 대응 Automation이 없어 source-equivalence review로 닫은 Turret/CombatFx 항목은 새로운 실패 근거 없이 union count에 가짜 항목으로 만들지 않는다.
- contract correction에서 최종 exact union 목록과 expected success count를 문서에 먼저 고정한 뒤에만 P0-05 final regression을 실행한다.
- P0-05가 Product C++를 실제 수정하면 fresh Official UE 5.8 Build를 mandatory로 실행한다. source mutation이 0이면 latest source-changing gate인 P0-04 Official Build PASS를 final source-build baseline으로 보존할 수 있으며 문서 마감만을 이유로 동일 binary를 다시 build하지 않는다.

**P1-3 — AI technical runtime validation과 USER smoke, 그리고 Feature Done 승격 조건의 경계가 불완전하다.**

현재 Acceptance는 `spawn / drive / steer / weapon select / fire / turret / launcher / target / scan / runtime apply`를 한 줄 USER smoke로 묶고 있어, RuntimeRead로 닫을 기술 사실과 실제 사람이 판단해야 할 조작감·시각·UX를 구분하지 못한다.

교정 요구:

- **AI Technical Validation**: Accepted `GoPyMCP.RuntimeRead`가 exact fact를 관측할 수 있는 경우 spawn/current Pawn/component binding, RuntimeReady/readback, weapon/target/scan/runtime-apply 상태 같은 기술 prerequisite를 AI가 먼저 직접 확인한다. 관측 편의를 위한 새 Product getter/log/debug surface는 추가하지 않는다.
- **USER Smoke**: 사람 판단이 필요한 `spawn이 정상적으로 보이는가`, `drive/steer 조작감에 구조 리팩터링 회귀가 없는가`, `weapon select/fire/turret/launcher 반응과 시각이 자연스러운가`, `target/scan/runtime apply UX가 기존과 같은가`만 짧은 representative PIE 절차로 고정한다.
- USER smoke는 CF-FQ-039 Production UI Visual Rework의 미완성 Visual 품질 Acceptance를 대신하거나 간섭하지 않는다. P0-05는 **기능 회귀 유무**만 확인하고 현재 CF-FQ-039 SourceArt/Production Asset dirty를 수정·저장하지 않는다.
- contract correction에서 대표 Product Pawn/PIE 진입점, 최소 조작 순서, PASS/FAIL 기록 형식을 current project evidence로 확정한다.
- Technical union PASS만으로 CF-FQ-048을 Done으로 확대하지 않는다. 최종 USER smoke가 아직 수행되지 않으면 `Technical Ready / USER Smoke Pending` 상태로 보존하고, USER PASS 후 Current System promotion + FeatureQueue Done + ActiveWork current-route cleanup을 수행한다.

현재 source audit 결론:

```text
추가 대형 Component extraction 필요 근거: 0
새 VehicleInputComp 필요: 0
새 Debug coordinator 필요: 0
Target identity migration 필요: 0
Editor authoring removal 필요: 0
Gameplay hit collision migration 필요: 0
P0-05 Product source mutation requirement: NOT ASSUMED
```

따라서 이번 Gate에서는 Product source를 수정하지 않는다. 위 세 계약을 교정해 **P0/P1 0을 재검수하기 전에는 facade 삭제·이동, final union 실행, USER smoke 실행을 시작하지 않는다.**

#### Contract Correction + Re-review — PASS / CONTRACT READY

P1-1~3을 current source, current Automation registration, current Project config와 CarFight RuntimeRead 정책 기준으로 교정하고 재검수했다.

##### A. Final Pawn responsibility / facade deletion freeze

P0-05 이후에도 `ACFVehiclePawn`은 아래 책임의 **최종 owner**다.

```text
Lifecycle owner
- OnConstruction
- PreRegisterAllComponents
- BeginPlay
- EndPlay
- Tick
- SetupPlayerInputComponent

Composition root
- Default Subobject identity
- BP/SCS component composition
- component 간 조립 진입점

Input entry
- Enhanced Input mapping/binding
- Handle* input entry
- RequestReloadCurrentWeapon
- RequestSelectWeaponIndex
- RequestStartActiveScan
- RequestStopActiveScan

Target identity
- ICFTargetSelectable 구현
- TargetPointComp 조립
- Target display/location/track identity
- target confirm/clear command facade

Compatibility facade
- 기존 Public/BP/Automation/private Visual/Fire/Runtime seam
- RuntimeApply가 소비하는 Pawn readback seam

Contract state Authority
- Vehicle/Fitting data pointer
- Core/Combat/compat Runtime Ready
- LastVehicleRuntimeSummary
- Fire/Turret/Debug 등 P0-00 Freeze의 observable/serialized/shared state
```

다음 항목은 **명시적 유지 대상**이며 P0-05 cleanup이라는 이유만으로 이동·삭제하지 않는다.

- `GetVehicleDebug*`, `ShouldShowVehicleDebug*`, `BuildVehicleDebugTextSingleLine`, `BuildVehicleDebugTextMultiLine`, `BuildVehicleDebugSummary`, `DisplayDriveStateOnScreenDebug`.
- `ConfigureVehicleVisualHitCollision()`의 `WeaponHit` / `Projectile` / `TargetSelect` gameplay collision contract.
- `CaptureWheelLayoutFromBodySockets()` / `ApplyVehicleLayoutFromDataInEditor()` 등 `WITH_EDITOR` authoring seam.
- `ResolveVehicleMoveInput`, steering/input ownership, Tick neutral-return 등 VehicleMove input behavior.
- `InitializeVehicleRuntime`, `RefreshFittingDependentRuntime`, `ApplyVehicleDataConfig`, Initial Sortie Mass prepare/verify 등 기존 facade가 Runtime coordinator에 위임하는 seam.
- Visual/Fire coordinator에 위임하는 기존 Pawn private compatibility wrapper 중 Automation/Freeze/lifecycle 호출 경계로 남아 있는 seam.

Facade 삭제는 아래 **5조건 AND**를 모두 만족할 때만 허용한다.

```text
1. private-only
2. Public / Blueprint / Automation / RuntimeApply / lifecycle consumer = 0
3. P0-00 Freeze seam이 아님
4. observable / serialized / shared state Authority를 소유하지 않음
5. existing coordinator가 동일 behavior를 이미 완전히 소유하고 caller가 direct coordinator call로 안전하게 대체 가능함
```

하나라도 증명되지 않으면 삭제하지 않는다. 한 줄 delegation, 이름 중복, 파일 길이 또는 line count는 삭제 근거가 아니다.

Fresh current source audit에서는 위 5조건을 모두 충족한다고 확정된 **추가 facade 삭제 대상이 0건**이다. 따라서 P0-05 Final Integration은 Product C++ source mutation `0`으로 종료될 수 있으며, 이것을 cleanup 미완료로 간주하지 않는다. 향후 별도 Feature에서 책임 경계가 바뀌면 그 Feature가 새 migration 계약을 소유한다.

##### B. P0-01~04 accepted Automation final affected union — exact28 freeze

Current Automation registration과 existing runner를 fresh resolve한 결과를 기준으로 final union을 아래 **unique exact28**로 동결한다.

P0-01 Visual/Aim accepted affected set은 unrelated `CarFight.VehicleData.VD_P0_01.ValidatorContract`를 제외한 exact11이다. 해당 Validator는 P0-01 당시 bundled runner에서 no-terminal contamination이 발생했지만 P0-01 Visual source를 실행하지 않는 별도 `UCFVDAValidator` 테스트였고, exact isolated 1/1 PASS로 이미 분리되었다. Final VPS affected union에는 포함하지 않는다.

```text
P0-01 Visual / Aim — exact11
1. CarFight.DataAuthoring.CF_FQ_040.WSA_P0_01.Foundation.Registry132
2. CarFight.Vehicle.WheelSize.WSA_P0_01.SchemaUtilityLegacy
3. CarFight.DataAuthoring.CF_FQ_040.WSA_P0_02.Resolver.SocketScaleDerived
4. CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Resolver.ObjectReferenceRoundTrip
5. CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Migration.DeferredApplyGuard
6. CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback
7. CarFight.Vehicle.WheelSize.WSA_P0_07.RightFallbackOrientationSpin
8. CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell
9. CarFight.DataAuthoring.CF_FQ_040.VB_P0_06.GameplayGuidance
10. CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit
11. CarFight.UI.UI_P0_09.ViewModeDirectionFoundation

P0-02 Fire — exact6
12. CarFight.Ammo.AMMO_P0_03.FireTransaction
13. CarFight.Launcher.LM_P0_03B.SchedulerContract
14. CarFight.Ammo.AMMO_P0_04.LauncherLock
15. CarFight.UI.UI_P0_06.HeatRuntimeResourceContract
16. CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract
17. CarFight.Projectile.LM_P0_06.SourceIsolation

P0-03 Runtime + P0-04 Header/RuntimeApply union — exact11 additional
18. CarFight.VehicleData.VD_P0_03.RuntimeApplyContract
19. CarFight.VehicleData.VD_P0_04.RuntimeReinitializeVisualContract
20. CarFight.Fitting.FIT_P0_05.InitialMass
21. CarFight.Fitting.FIT_P0_05.DefensePIEPipeline
22. CarFight.RuntimeApply.RTA_P0_02.VehicleApplySuccess
23. CarFight.RuntimeApply.RTA_P0_02.ApplyFailedRecovery
24. CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess
25. CarFight.RuntimeApply.RTA_P0_03.ApplyFailedRecovery
26. CarFight.RuntimeApply.RTA_P0_04.PanelNavigation
27. CarFight.RuntimeApply.RTA_P0_04.ExplicitVehicleApply
28. CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply
```

Deduplication 근거:

- P0-03 accepted exact9는 P0-04 exact12에 **완전히 포함**되므로 별도 추가 count `0`이다.
- `CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback`은 P0-01 Visual exact11과 P0-04 exact12 양쪽에 존재하므로 최종 union에서 **1회만 실행**한다.
- 따라서 `11 + 6 + 12 - 1 = unique exact28`이다.
- Turret Visual과 CombatFx/FireFeedback은 해당 단계에서 direct Automation이 없어 source-equivalence final review로 닫은 항목이며, P0-05에서 가짜 exact test를 만들어 count를 부풀리지 않는다.

Final regression acceptance는 다음을 요구한다.

```text
expected exact tests: 28
success: 28
failure: 0
missing: 0
unexpected: 0
duplicate terminal: 0
```

가능하면 기존 exact-list same-process 경로를 사용한다. broad suite를 final authority로 대체하지 않는다.

Build 조건:

- P0-05 Product C++ source mutation이 `0`이면 latest source-changing gate인 P0-04 Official UE 5.8 Build job `241ef0b05aa9455c94ca562fc93d81cc` PASS를 final source-build baseline으로 보존한다. 문서 마감만을 이유로 동일 binary를 다시 build하지 않는다.
- 위 A의 5조건을 만족해 실제 Product C++ facade cleanup을 수행하게 되면 fresh Official UE 5.8 Build가 mandatory이며 그 뒤 exact28을 실행한다.

##### C. AI RuntimeRead Technical Validation contract

대표 Product 진입점은 current Project config를 따른다.

```text
Map:
/Game/Maps/TestMap.TestMap

Configured default vehicle Pawn:
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C
```

별도 테스트 Pawn parent 교체, Product Asset save 또는 Wagon-specific UAT를 만들지 않는다.

Final Integration의 AI runtime technical validation은 Accepted `GoPyMCP.RuntimeRead` 범위에서 **read-only**로 수행한다. Editor/PIE lifecycle은 별도 control plane이며, 관측 편의를 위한 Product Debug HUD / Print / UE_LOG / 신규 getter / Consumer 전용 MCP operation을 추가하지 않는다.

**T0 — USER smoke 전 initial technical prerequisite**

RuntimeRead가 current Ready lifetime에서 지원하는 범위만 사용해 다음을 확인한다.

1. current PIE World가 representative `TestMap`인지 확인한다.
2. LocalPlayer의 controlled Pawn이 current `BP_CFVehiclePawn_C` / native `ACFVehiclePawn` lineage인지 확인한다.
3. Pawn의 expected component composition이 유지되는지 확인한다. 최소한 Visual/Fire/Runtime coordinator와 Drive/Wheel/Aim/Health/Fitting/Ammo/Weapon/Launcher/Defense/TargetSelect/Sensor 계층이 현재 fitting에 맞게 존재·bind되어야 한다.
4. Runtime readiness/readback에서 coordinator-missing, facade-missing 또는 초기화 순서 붕괴를 나타내는 상태가 없어야 한다.
5. `bVehicleRuntimeReady` compatibility 의미가 Core Ready와 어긋나지 않아야 하며, armed representative fitting에서는 Combat-ready prerequisite가 정상 구성되어야 한다. 장착 상태 때문에 Combat Ready가 의도적으로 false인 경우에는 `RuntimeSummary`가 그 의도 원인을 설명해야 하며 구조 리팩터링 누락으로 오판하지 않는다.
6. current selected weapon/launcher/target-select/sensor runtime object/state가 representative fitting에서 유효한지 확인한다.
7. RuntimeApply/VehicleDebug readback에 사용하는 Runtime Ready/Summary 관측값이 Pawn contract와 모순되지 않는지 확인한다.

**T1 — USER가 representative action을 수행한 뒤 same-lifetime technical readback**

RuntimeRead가 지원하는 경우 USER action 직후 다음 상태를 read-only로 확인한다.

- weapon selection 뒤 selected weapon runtime identity/state.
- fire/launcher action 뒤 fatal/stuck sequence 또는 missing runtime 상태가 남지 않았는지.
- target select/clear 뒤 selected target state transition.
- active scan start/stop 뒤 sensor scan state transition.
- representative transient Runtime Apply 뒤 Runtime Ready/Summary와 current runtime state가 failure/recovery 계약과 모순되지 않는지.

RuntimeRead가 특정 fact를 지원하지 않거나 current runtime/tooling이 unavailable이면 임의 getter/function 호출이나 blind retry로 우회하지 않는다. 해당 항목은 `Technical Runtime Validation Pending — Observation Gap`으로 기록하며 USER의 육안 확인을 기술 evidence 대체재로 사용하지 않는다. 동일 invariant가 이미 exact28 Automation으로 직접 증명되는 경우에는 그 Automation evidence를 명시적으로 연결할 수 있지만, 관측하지 않은 runtime fact를 PASS로 추정하지 않는다.

이번 Contract Correction 세션에서는 RuntimeRead 실행 자체를 하지 않는다. current UE read-only toolset 조회가 upstream `502`로 응답한 것은 **이번 계약 문서화 시점의 도구 가용성 관측**일 뿐, future Final Integration에서 RuntimeRead가 영구적으로 불가능하다는 계약으로 승격하지 않는다.

##### D. Representative USER smoke contract

USER smoke는 **기능 회귀와 체감 회귀**만 판단한다. CF-FQ-039 Production UI Visual Rework의 pixel/디자인 품질 Acceptance, 차량/무기 밸런스 튜닝, Wagon 전용 UAT는 범위 밖이다.

진입:

```text
Editor / Game Default Map: TestMap
Playable Pawn: current configured BP_CFVehiclePawn
Mode: representative local PIE
Product Asset Save / Save All: 금지
```

최소 조작 순서:

1. **Spawn/Visual functional check** — PIE 시작 후 차량이 정상 spawn되고 차체/휠/활성 터렛 등 핵심 차량 표현이 사라지거나 명백히 깨지지 않았는지 확인한다.
2. **Drive/Steer** — 전진/후진, 좌/우 조향을 짧게 수행하고 입력이 먹지 않거나 steering neutral-return이 고착되는 등의 리팩터링 회귀가 없는지 확인한다.
3. **Weapon Select / Fire / Turret / Launcher** — 현재 fitting이 허용하는 weapon 선택을 수행하고 발사한다. 터렛 조준 반응, 발사 입력, projectile/hitscan/launcher 반응이 기존처럼 자연스럽고 죽은 입력/고착 sequence가 없는지 확인한다.
4. **Target / Scan** — 기존 입력으로 target select/clear와 active scan start/stop을 수행해 조작 흐름이 끊기지 않는지 확인한다.
5. **Runtime Apply UX** — 기존 Runtime Apply UI에서 현재 존재하는 대표 Vehicle/Equipment 대상으로 **transient apply만** 수행하고 panel navigation, apply interaction, readback 표시가 stale/무반응 상태가 아닌지 확인한다. Product Asset을 저장하지 않는다.

USER PASS 기준:

- crash/hang 없음.
- 차량 입력이 죽거나 계속 고착되는 회귀 없음.
- weapon select/fire/turret/launcher가 구조 리팩터링으로 무반응이 되지 않음.
- target/scan UX가 끊기거나 명백히 stale되지 않음.
- Runtime Apply UX가 기존 정상 경로에서 동작하고 리팩터링 때문에 facade/readback이 끊기지 않음.
- HUD pixel-perfect 품질이나 CF-FQ-039의 미완성 Production Visual을 VPS 실패로 판정하지 않음.

USER receipt 기록 형식:

```text
Date / KST:
Map: /Game/Maps/TestMap
Pawn: BP_CFVehiclePawn
Spawn/Visual Functional: PASS|FAIL
Drive/Steer Feel: PASS|FAIL
Weapon/Fire/Turret/Launcher: PASS|FAIL|N/A(reason)
Target/Scan UX: PASS|FAIL|N/A(reason)
RuntimeApply UX: PASS|FAIL|N/A(reason)
Overall USER Smoke: PASS|FAIL
Notes: exact symptom only
```

##### E. Final integration state / Done promotion contract

**Technical closure 조건**:

```text
- final source/facade review P0 = 0 / blocking P1 = 0
- source mutation 0이면 P0-04 accepted Build baseline 유지
  또는 source mutation >0이면 fresh Official Build PASS
- final affected union exact28 = 28/28 PASS
- RuntimeRead AI Technical Validation은 사용 가능할 때 required observable 항목 PASS를 요구한다.
  단 현재처럼 UE MCP 자체가 unavailable하고 Product failure evidence가 0이며 source final audit + Official Build + final exact28이 모두 PASS인 경우, RuntimeRead는 `Waived / Deferred Observation`으로 분리하고 USER smoke를 closure gate로 사용한다.
- Product Asset mutation/save = 0
- unrelated dirty 보존
```

위 조건이 충족됐지만 USER smoke가 아직 없으면 상태는:

```text
CF-FQ-048 = Ready
VPS-P0-05 = Technical Ready / USER Smoke Pending
Done promotion = 금지
```

USER smoke까지 PASS한 경우에만 다음 promotion을 수행한다.

1. 대표 Plan을 VPS-P0 Complete로 마감하고 Build/Automation/RuntimeRead/USER receipt를 evidence로 보존한다.
2. 현재 구현 계약을 `Document/Systems/Vehicles/VehicleRuntime.md`에 승격하고, 실제로 변경된 Current contract가 있을 때만 관련 WeaponFire/Visual Current 문서를 최소 범위 갱신한다.
3. `Document/ProjectSSOT/03_FeatureQueue.md`의 `CF-FQ-048`을 `Done`으로 전진한다.
4. `Document/ActiveWork.md`에서 CF-FQ-048의 Ready/current-route pointer를 완료 상태에 맞게 정리한다.
5. `Document/Plan/README.md` projection을 완료 lifecycle에 맞게 갱신하고 대표 Plan의 historical/reference 경로를 보존한다.
6. CF-FQ-039의 Active 상태, SourceArt/Production UI dirty와 독립 Acceptance는 변경하지 않는다.

Technical FAIL이면 USER smoke 결과가 좋아도 Done으로 승격하지 않는다. USER FAIL이면 Technical PASS를 무효화하지 않되 `Technical Ready / USER Smoke Failed`로 남기고 exact symptom을 새 correction 대상과 연결한다. RuntimeRead/tooling unavailable만으로 USER에게 기술 값을 대신 읽게 하지 않는다.

##### Final Integration technical validation result — 2026-09-15

Product source final re-audit에서 추가 삭제·이동·Facade 제거 조건을 만족하는 seam은 **0건**으로 재확인됐다. 따라서 P0-05 Product source mutation은 0이며 P0-04 accepted Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc`를 final source-build baseline으로 보존한다.

final affected unique exact28은 same-process exact-list mode로 fresh 실행했다.

```text
Process: 6431180d7e7d4b11ac9f8dc51a25b5c2
Engine exit: 0
Success: 28
Failure: 0
Missing: 0
Unexpected: 0
Duplicate terminal: 0
Verdict: PASS
```

검증용 temporary runner residue는 남기지 않았고 기존 공용 runner 변경도 최종 diff 0으로 복원했다. Product Asset mutation/save는 0이다.

Representative Editor는 canonical lifecycle로 Ready까지 올라왔고 current managed Runtime은 `runtime_deba1d9a1b7448d6bb506229b378ca40`로 확인됐다. 그러나 Accepted UE ReadOnly RuntimeRead entry `ue.status`가 exact Runtime selector를 사용한 T0 prerequisite에서 반복해서 `502 Upstream or external service errors`를 반환했다. GoPyMCP core/adapter self-test는 PASS이고 bridge/session/UE environment도 정상이라 현재 evidence는 **CarFight Product failure가 아니라 UE ReadOnly connector observation path failure**로 분류한다.

RuntimeRead T0 기술 사실을 사용자에게 대신 읽게 하거나 관측 편의를 위한 Product getter/log/MCP surface를 추가하지 않는다. 이후 UE MCP가 현재 사용 불가 상태로 확인되어, 이번 VPS-P0-05 closure에서는 RuntimeRead T0를 **필수 완료 게이트에서 해제하고 Deferred Observation으로 전환**한다.

Waiver는 "관측하지 못한 runtime fact를 PASS로 간주"하는 의미가 아니다. 이미 확보된 source final audit, P0-04 Official Build PASS, final exact28 28/28 PASS를 기술 근거로 사용하고, 사람이 직접 확인해야 하는 functional/feel 회귀는 USER smoke로 남긴다. UE MCP가 복구되면 RuntimeRead T0는 별도 non-blocking technical revalidation으로 실행할 수 있다.

```text
Source mutation required: 0
P0-04 Official Build baseline: PRESERVED / PASS
Final affected exact28: PASS 28/28
RuntimeRead T0: WAIVED / DEFERRED OBSERVATION — UE MCP unavailable
Technical Ready: GRANTED
USER Smoke: REQUIRED / PENDING
Done promotion: USER Smoke PASS 전 금지
Exact next: VPS-P0-05 USER Smoke
```

따라서 USER smoke는 지금부터 실행 가능하다. 단 USER 육안/조작 결과를 RuntimeRead 내부 상태값의 대체 관측으로 소급 해석하지 않는다.

##### USER Smoke / Final Closure result — 2026-09-15

사용자는 representative local PIE에서 Vehicle Pawn Slimming 이후의 실제 조작/기능 흐름을 확인한 뒤 **"sliming에 의한 고장은 없는 거 같아"**라고 판정했다. 이 사용자 판정을 VPS-P0-05의 최종 functional/feel regression receipt로 사용한다. 개별 항목을 사용자가 모두 표 형식으로 재기록하지 않았으므로 확인하지 않은 세부 항목을 임의로 PASS로 꾸며 확장하지 않는다.

확인 과정에서 Target Select는 실제 동작했으며, Active Scan은 현재 차량에서 동작하지 않았다. 추가 조사 결과 현재 Product는 scanner-less 차량에 `FallbackSensorConfig`의 0-range를 적용해 Active Scan을 거부하고, Vehicle Builder로 신규 제작한 차량에도 기본 Sensor/Active Scan baseline이 자동 공급되지 않는 상태다. 반면 ProjectSSOT의 전투 기획은 P0부터 `기본 센서 범위`를 요구하고 센서 타입 피팅을 후속 확장으로 두고 있어 **Vehicle Builder / 기본 Sensor baseline의 선행 설계-구현 불일치**로 분리한다.

이 Active Scan 문제는 VPS에서 새로 발생한 회귀라는 근거가 없고 CF-FQ-037 Scanner/current Sensor 계약에 이미 존재하므로, CF-FQ-048 USER Smoke 실패로 계산하지 않는다. 별도 correction lifecycle에서 다뤄야 하며 이번 closure에서 Sensor/Vehicle Builder Source·Asset을 수정하지 않는다.

```text
Date / KST: 2026-09-15
Representative Entry: /Game/Maps/TestMap + BP_CFVehiclePawn
USER Regression Judgment: PASS — no Vehicle Pawn Slimming-induced breakage observed
Target Select: observed working
Active Scan: EXCLUDED FROM VPS REGRESSION ACCEPTANCE — pre-existing Vehicle Builder / base Sensor baseline mismatch
RuntimeRead T0: WAIVED / DEFERRED OBSERVATION — UE MCP unavailable
Overall USER Smoke: PASS
Product Asset save: 0
```

최종 closure:

```text
VPS-P0-00~05: COMPLETE
P0 blockers: 0
blocking P1: 0
P2: 0
Official Build baseline: PASS / 241ef0b05aa9455c94ca562fc93d81cc
Final affected exact28: PASS 28/28 / 6431180d7e7d4b11ac9f8dc51a25b5c2
RuntimeRead: WAIVED / DEFERRED — non-blocking tooling observation gap
USER Smoke: PASS
CF-FQ-048 lifecycle: Done -> Historical + Retained Path
Current System owner: Document/Systems/Vehicles/VehicleRuntime.md v1.6.0
Next exact gate: None
```

##### Re-review result

세 blocking P1은 다음과 같이 closure됐다.

- **P1-1 CLOSED** — final Pawn owner와 유지 seam, facade 삭제 5조건 AND, source mutation 0 허용 계약을 동결했다. 현재 추가 삭제 대상 0건이다.
- **P1-2 CLOSED** — current Automation registration을 fresh resolve해 exact28 unique union, dedupe 근거와 expected terminal count를 고정했다.
- **P1-3 CLOSED** — current TestMap + configured BP_CFVehiclePawn을 representative entry로 고정하고 RuntimeRead T0/T1, USER smoke 5단계, Technical Ready/USER Pending/Done promotion 경계를 분리했다.

```text
VPS-P0-05 Contract Correction + Re-review
P0 blockers: 0
blocking P1: 0
P2: 0
Verdict: PASS / CONTRACT READY
Product source mutation: 0
Product Asset mutation/save: 0
Official Build: Not Run — contract/document-only gate; P0-04 accepted Build preserved
Automation: Not Run — final exact28 contract frozen only
RuntimeRead: Not Run — contract frozen only; current toolset inquiry observed transient upstream 502
USER Smoke: Not Run — contract frozen only
P0-00~04 accepted evidence: PRESERVED
Next exact gate: VPS-P0-05 Final Integration — Technical Validation / USER Smoke
```

이 Gate에서는 final exact28 regression, PIE RuntimeRead validation과 USER smoke를 실행하지 않는다.

---

## 12. P0에서 하지 않는 것

- VehicleData public→private 캡슐화
- 대규모 public getter/setter migration
- 새 VehicleInputComp 추가
- generic VehicleCombatComp 추가
- Scene Component 이름/identity migration
- Blueprint parent 교체
- Blueprint/SCS hierarchy 재작성
- Weapon/Ammo/Launcher gameplay 의미 변경
- Steering feel tuning
- Runtime Apply 기능 확장
- Editor authoring 제거
- Legacy UI UPROPERTY 삭제
- 기존 Product Asset 일괄 저장/재저장

---

## 13. Acceptance

P0 구조 Acceptance:

1. 기존 Public/Blueprint/Runtime Apply/Launcher/Automation 계약이 유지된다.
2. 새 Coordinator의 duplicate state authority가 없다.
3. Lifecycle 호출 순서가 유지된다.
4. BP/SCS component fresh resolve가 보장된다.
5. gameplay collision을 Visual 책임으로 잘못 이동하지 않는다.
6. 기존 Domain Component의 상태를 신규 Component가 복제하지 않는다.
7. 단계별 Official Build와 focused/affected regression이 PASS한다.
8. Final Integration에서 exact28 Technical regression을 닫고, RuntimeRead는 사용 가능한 경우 기술 관측을 수행한다. UE MCP 자체 unavailable + Product failure evidence 0 + source audit/Build/exact28 PASS이면 `Waived / Deferred Observation`을 허용하며, 별도 representative USER smoke에서 구조 리팩터링으로 인한 기능·체감 회귀가 없어야 한다.
9. 신규 기능의 기본 변경 지점이 더 이상 `CFVehiclePawn.cpp` 하나로 집중되지 않는다.

줄 수 감소는 보조 지표일 뿐 Acceptance가 아니다. 현재 약 8,901 combined lines에서 상당한 축소가 예상되지만 특정 line count 달성을 목표로 의미 있는 API/상태를 제거하지 않는다.

---

## 14. 기존 dirty 보호

승격 시점 main_game에는 Wagon/Runtime Test Catalog/Weapon Definition uasset 및 `SourceArt/UI/HUD/VehiclePanel/` 병렬 dirty가 존재한다.

CF-FQ-048 구현은 해당 기존 Asset dirty를 수정·정리·저장·되돌리지 않는다.

Source 작업 시작 전 fresh `git.status`와 `CFVehiclePawn` 관련 diff를 다시 확인한다.

---

## 15. Current checkpoint

```text
Feature: CF-FQ-048 Vehicle Pawn Slimming
Priority: P2
Lifecycle: Done / Historical + Retained Path
Design Audit: Correction + Re-review PASS
VPS-P0-00 Freeze Review: Correction + Re-review PASS
VPS-P0-01 Visual Extraction: Correction + Re-review PASS
VPS-P0-02 Fire Extraction: Authority Correction + Final Review PASS
VPS-P0-03 Runtime Extraction: Implementation + Final Review PASS
VPS-P0-04 Debug / Header Cleanup: Implementation + Fresh Validation + Final Review PASS
VPS-P0-05 Pre-Implementation Contract Review: Historical HOLD / P0 0 / blocking P1 3 / P2 0
VPS-P0-05 Contract Correction + Re-review: PASS / CONTRACT READY
VPS-P0-05 Final Integration Source Re-audit: mutation0 ACCEPTED
VPS-P0-05 Final affected exact28: PASS 28/28 / process `6431180d7e7d4b11ac9f8dc51a25b5c2`
VPS-P0-05 RuntimeRead T0: WAIVED / DEFERRED OBSERVATION — UE MCP unavailable / Product failure evidence 0
Technical Ready: GRANTED
USER Smoke: PASS — 사용자 판정 `sliming에 의한 고장은 없는 거 같아`
Known unrelated issue: Vehicle Builder-created vehicle의 기본 Sensor/Active Scan baseline 누락 — pre-existing design/current mismatch, VPS regression에서 제외
P0 blockers: 0
blocking P1: 0
P2: 0
Source mutation for completed VPS implementation: CFVehiclePawn.h/.cpp + CFVehicleVisualComp.h/.cpp + CFVehicleFireComp.h/.cpp + CFVehicleRuntimeComp.h/.cpp + CFVehicleInputTypes.h + CFVehicleDebugTypes.h + VehicleDebug HUD/Panel h/.cpp + CarFightVehicleUtils.h
P0-05 Product source mutation: 0
Asset mutation for VPS: 0
Product Asset save: 0
P0-04 accepted evidence: Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS + affected exact12 process `7f9806d89c98491ca4ee89cdec95ae2b` 12/12 PASS + final source re-review P0 0 P1 0 P2 0
P0-05 final affected union: unique exact28 PASS / success28 failure0 missing0 unexpected0 duplicate0
Representative runtime entry: `/Game/Maps/TestMap.TestMap` + `/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C`
Final validation split: Source audit + Build baseline + exact28 Technical PASS + USER regression smoke PASS / RuntimeRead는 UE MCP 복구 후 optional non-blocking revalidation
Current System Promotion: Document/Systems/Vehicles/VehicleRuntime.md v1.6.0
Next exact gate: None
```

CF-FQ-048은 Done/Historical로 종료했으며 현재 단일 Active `CF-FQ-039`는 변경하지 않는다. retained Plan의 old next gate는 현재 착수 지시로 재사용하지 않는다.

---

## 16. Changelog

### v0.7.0 - 2026-09-15

- 사용자의 representative functional/feel 확인에서 Vehicle Pawn Slimming으로 인한 고장이 관찰되지 않았다는 최종 판정을 받아 VPS-P0-05 USER Smoke를 PASS로 마감했다. 확인하지 않은 세부 항목을 임의 PASS로 확대하지 않고 사용자 최종 regression judgment를 receipt로 보존한다.
- Target Select는 동작을 확인했다. Active Scan 미동작은 current scanner-less 0-range 계약과 Vehicle Builder 신규 차량의 기본 Sensor baseline 누락에서 재현되는 선행 설계/구현 불일치로 분류했으며, VPS가 새로 만든 회귀 근거가 없어 CF-FQ-048 Acceptance에서 제외했다. 별도 correction lifecycle 전까지 Sensor/Builder Source·Asset mutation은 0이다.
- P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS, final exact28 process `6431180d7e7d4b11ac9f8dc51a25b5c2` 28/28 PASS, Product Asset mutation/save 0을 최종 evidence로 보존한다.
- RuntimeRead T0는 UE MCP unavailable 때문에 `Waived / Deferred Observation`으로 남으며 관측하지 않은 내부 runtime fact를 PASS로 소급하지 않는다. 향후 UE MCP 복구 시 optional non-blocking revalidation으로만 수행한다.
- CF-FQ-048을 `Done / Historical + Retained Path`로 종료하고 Current owner를 `Document/Systems/Vehicles/VehicleRuntime.md v1.6.0`으로 승격한다. 현재 단일 Active `CF-FQ-039`는 변경하지 않는다.

### v0.6.4 - 2026-09-15

- 사용자 결정과 현재 UE MCP unavailable 상태를 반영해 RuntimeRead T0를 VPS-P0-05의 필수 완료 게이트에서 해제하고 `Waived / Deferred Observation`으로 전환했다.
- 이 waiver는 미관측 runtime fact를 PASS로 간주하는 것이 아니다. Product source final audit mutation0, P0-04 Official UE 5.8 Build PASS, final exact28 28/28 PASS를 기술 근거로 Technical Ready를 부여하고, 실제 기능/체감 회귀 확인은 USER smoke를 필수 closure gate로 유지한다.
- 현재 상태를 `Technical Ready / USER Smoke Pending`으로 전진했다. UE MCP가 복구되면 RuntimeRead T0는 별도 non-blocking revalidation으로 수행할 수 있으나 CF-FQ-048 Done promotion의 선행 필수조건은 아니다.
- Product Source/Asset mutation과 save는 0이며 CF-FQ-039 Active와 기존 병렬 dirty를 보호한다. exact next는 `VPS-P0-05 USER Smoke`다.

### v0.6.3 - 2026-09-15

- `VPS-P0-05 Final Integration` final source re-audit에서 추가 Product source 변경 필요성을 0건으로 재확인해 P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS baseline을 그대로 보존했다.
- final unique exact28을 same-process exact-list mode로 fresh 실행해 process `6431180d7e7d4b11ac9f8dc51a25b5c2`, Engine exit 0, success28 / failure0 / missing0 / unexpected0 / duplicate0 PASS를 확보했다.
- canonical Editor lifecycle은 Ready까지 성공했지만 exact managed Runtime `runtime_deba1d9a1b7448d6bb506229b378ca40` 대상 UE ReadOnly `ue.status`가 반복 502를 반환해 RuntimeRead T0 prerequisite를 관측하지 못했다. GoPyMCP core/adapter self-test와 UE environment는 정상이라 Product defect가 아닌 observation-path tooling blocker로 분류한다.
- RuntimeRead 기술 값을 USER에게 대신 확인시키거나 Product getter/log/MCP surface를 추가하지 않는다. 현재 상태는 **Technical Ready Not Yet Granted / USER Smoke Not Started**이며 exact next는 `VPS-P0-05 RuntimeRead T0 Revalidation`이다.
- Product Source mutation 0 / Product Asset mutation·save 0 / temporary runner residue 0을 유지했다. USER PASS 전 Done promotion은 계속 금지한다.

### v0.6.2 - 2026-09-14

- `VPS-P0-05 Contract Correction + Re-review`에서 v0.6.1의 blocking P1 3건을 모두 교정해 **P0 0 / blocking P1 0 / P2 0 / PASS / CONTRACT READY**로 전진했다.
- Lifecycle/Composition root/Input entry/Target identity/Compatibility facade/Contract state를 final Pawn owner로 동결하고, facade 삭제를 `private-only + consumer0 + Freeze seam 아님 + state Authority 아님 + coordinator complete owner`의 5조건 AND로 제한했다. Fresh audit에서 추가 삭제 대상은 0건이며 P0-05 source mutation 0도 정상 completion으로 허용한다.
- P0-01 Visual/Aim current exact11, P0-02 Fire exact6, P0-04 exact12를 current registration에서 fresh resolve했다. P0-03 exact9은 P0-04 exact12에 완전 포함되고 RuntimeVisualFallback 1건이 P0-01/P0-04 중복이므로 final unique affected union을 **exact28**로 동결했다.
- P0-01 bundled `VD_P0_01.ValidatorContract`는 Visual source를 실행하지 않는 unrelated validator이며 historical no-terminal contamination 뒤 isolated 1/1 PASS였으므로 final VPS union에서 제외했다. Turret/CombatFx source-review-only 항목도 가짜 Automation으로 만들지 않는다.
- final exact28 acceptance를 success28 / failure0 / missing0 / unexpected0 / duplicate0으로 고정했다. P0-05 Product source mutation이 0이면 P0-04 Build `241ef0b05aa9455c94ca562fc93d81cc`를 final source-build baseline으로 보존하고, 실제 source cleanup이 생기면 fresh Official Build를 mandatory로 한다.
- representative PIE entry를 current `TestMap` + configured `BP_CFVehiclePawn`으로 고정하고 RuntimeRead T0 initial prerequisite / T1 post-action readback과 USER smoke 5단계를 분리했다. 관측 편의를 위한 Product debug getter/log/MCP 추가와 blind retry는 금지한다.
- USER smoke는 functional/feel regression만 판단하며 CF-FQ-039 Production UI Visual pixel/design Acceptance, Wagon 전용 UAT와 Product Asset Save는 범위 밖이다.
- Technical closure 뒤 USER smoke 전 상태는 `Technical Ready / USER Smoke Pending`이며 Done 승격을 금지한다. Technical + USER PASS 후에만 Systems promotion, FeatureQueue Done, ActiveWork route cleanup과 Plan lifecycle 정리를 수행한다.
- 이번 Gate에서는 Product Source/Asset mutation, Build, final exact28, PIE RuntimeRead, USER smoke 실행을 모두 0으로 유지했다. UE read-only toolset 조회의 transient upstream 502는 current availability 관측으로만 기록하고 영구 capability 부재로 확대하지 않는다.
- exact next는 `VPS-P0-05 Final Integration — Technical Validation / USER Smoke`다.

### v0.6.1 - 2026-09-14

- `VPS-P0-05 Pawn Facade Cleanup / Final Integration` 착수 전 current Pawn/Visual/Fire/Runtime source와 P0-00~04 accepted evidence를 fresh 재대조했다.
- 추가 대형 extraction이 필요한 P0 결함은 없었지만, 남은 Pawn responsibility/facade 삭제 경계, 최종 affected union exact-name/dedupe/count, AI RuntimeRead technical validation ↔ USER smoke ↔ Done promotion 경계가 아직 모호한 blocking P1 3건을 확인해 **HOLD**로 기록했다.
- Lifecycle/Composition root/Input entry/Target identity/Compatibility facade/Contract state는 현재 final Pawn 역할로 분류했다. P0-04 Debug aggregation, gameplay hit collision, WITH_EDITOR authoring seam과 VehicleMove input behavior는 line-count 정리 대상이 아님을 명시했다.
- P0-05는 추가 extraction이나 source line 감소를 필수 성과로 요구하지 않는다. call-graph상 misplaced behavior가 없으면 Product source mutation 0도 정상 결과로 허용한다.
- final regression은 P0-01 Visual + P0-02 Fire exact6 + P0-03 Runtime exact9 + P0-04 exact12의 실제 current Automation exact name을 resolve하고 중복 제거한 union을 contract correction에서 먼저 동결해야 한다.
- final validation은 RuntimeRead로 닫을 기술 사실과 사람의 시각/조작감/UX smoke를 분리하고, USER smoke PASS 전에는 CF-FQ-048을 Done으로 확대하지 않는다. CF-FQ-039 Production UI Visual dirty/Acceptance는 별도 보호한다.
- 이번 Gate는 document/source read-only audit이므로 Product source/Asset mutation, Build, Automation, PIE 실행은 0이며 P0-00~04 accepted evidence를 그대로 보존한다.
- exact next는 `VPS-P0-05 Contract Correction + Re-review`다.

### v0.6.0 - 2026-09-14

- `VPS-P0-04 Debug / Header Cleanup` 구현을 완료해 `CFVehicleInputTypes.h`와 `CFVehicleDebugTypes.h`를 신규 reflected declaration owner로 분리했다. Input 6 declaration과 Debug 10 declaration은 fresh source search에서 각각 exact single owner이며 duplicate 0이다.
- `CFVehiclePawn.h v2.171.0`은 새 type header를 직접 include하고 기존 declaration 중복을 제거했지만 Pawn Public/BP Debug facade, lifecycle, observable state와 Debug aggregation behavior는 유지했다.
- Debug HUD/Panel Public header는 DebugTypes + Pawn forward declaration, Private `.cpp`는 Pawn explicit include로 정리하고 `CarFightVehicleUtils.h`도 Drive + DebugTypes direct dependency로 전환했다. fresh Public header search에서 direct Pawn include는 0이다.
- RuntimeVehicleApply/RuntimeEquipApply의 `GetVehicleDebugRuntime()` readback seam과 `bRuntimeReady`/`RuntimeSummary` 의미를 source re-review로 재확인했다.
- Official UE 5.8 Build job `241ef0b05aa9455c94ca562fc93d81cc`에서 UHT/compile/link 및 terminal `Result: Succeeded`를 확인했다. status lane 한도 이후에도 중복 build 없이 같은 job의 managed log terminal을 read-only로 회수했다.
- affected exact12 process `7f9806d89c98491ca4ee89cdec95ae2b`는 `ENGINE_EXIT_CODE=0`, success 12 / failure·missing·unexpected·duplicate 0으로 PASS했다.
- Product Asset mutation/save 0, 작업 전용 helper residue 0이며 최종 판정은 **P0 0 / blocking P1 0 / P2 0 / PASS**다.
- exact next는 `VPS-P0-05 Pawn Facade Cleanup / Final Integration — Not Started`다.

### v0.5.2 - 2026-09-14

- VPS-P0-04 Pre-Implementation HOLD의 blocking P1 3건을 current source/consumer/test 기준으로 교정하고 재검수했다.
- `CFVehicleInputTypes.h`를 4개 input enum + 2개 input struct의 exact reflected declaration owner로, `CFVehicleDebugTypes.h`를 Debug enum + 9개 Debug struct의 exact reflected declaration owner로 고정했다. 이름·값·필드·Blueprint/metadata/default 의미는 그대로이며 새 Input/Debug Component나 state Authority는 만들지 않는다.
- DebugTypes의 reflected value dependency를 InputTypes/DriveComp/AimTypes/CameraTypes/DamageTypes/WeaponTypes/TargetSelectTypes direct include로 고정하고 Pawn/transitive include 의존을 금지했다. UObject DataAsset pointer는 forward declaration을 우선하고 UHT가 요구할 때만 최소 direct owner header를 추가한다.
- Debug HUD/Panel Public header는 DebugTypes + Pawn forward declaration, 실제 호출 `.cpp`는 Pawn explicit include로 분리하고 VehicleUtils는 DriveComp + DebugTypes만 직접 소비하도록 계약을 확정했다.
- Pawn의 `GetVehicleDebug*` / `ShouldShowVehicleDebug*`, Debug text aggregation owner, RuntimeVehicleApply/RuntimeEquipApply의 `GetVehicleDebugRuntime()` readback seam, Product BP/SCS/serialized contract와 Asset save 0을 동결했다.
- Implementation acceptance를 Official UE 5.8 Build + P0-03 exact9 + RuntimeApply UI exact3 = **affected exact12**로 확정하고 실제 exact3 이름을 current test source에서 재확인했다.
- Contract Correction + Re-review 결과는 **P0 0 / blocking P1 0 / P2 0 / PASS / CONTRACT READY**다. 이번 Gate는 document-only이므로 Source implementation/Asset mutation/Build/Automation 실행은 0이며 P0-03 accepted evidence는 그대로 보존한다.
- exact next는 `VPS-P0-04 Debug / Header Cleanup — Implementation Not Started`다.

### v0.5.1 - 2026-09-14

- `VPS-P0-04 Debug / Header Cleanup` 구현 전 fresh contract/source/consumer audit를 수행했다.
- `CFVehicleDebugTypes.h` 단독 분리 시 Pawn-local input reflected type 의존 때문에 include cycle 또는 부분 owner 분리가 발생할 수 있어 blocking P1으로 판정했다.
- `CFVehicleInputTypes.h`를 기존 input reflected declaration의 독립 owner로 두되 이름/값/노출/behavior를 전혀 바꾸지 않는 교정 계약을 요구했다. 새 Input Component나 state migration은 범위 밖이다.
- Debug HUD/Panel/VehicleUtils Public header의 Pawn transitive include를 DebugTypes + forward declaration 경계로 교정하고 Private translation unit에서 Pawn include를 명시하도록 dependency 계약을 고정했다.
- 기존 `GetVehicleDebug*` / `ShouldShowVehicleDebug*` Blueprint facade, reflected Debug type identity/field, Pawn debug aggregation behavior와 RuntimeApply `GetVehicleDebugRuntime()` readback seam 보존을 필수 계약으로 고정했다.
- 구현 후 검증은 Official UE 5.8 Build + P0-03 accepted exact9 + RuntimeApply UI exact3 = affected exact12를 최소 acceptance로 정의했다.
- 사전검수 결과는 **P0 0 / blocking P1 3 / P2 0 / HOLD**다. Source/Asset mutation은 0이며 P0-03 Build/exact9/final-review PASS baseline은 그대로 보존한다.
- exact next는 `VPS-P0-04 Contract Correction + Re-review`다.

### v0.5.0 - 2026-09-14

- `VPS-P0-03 Runtime Behavior Extraction` 구현과 최종검수를 완료해 `UCFVehicleRuntimeComp`를 내부 coordinator로 추가했다. Initialize/Refresh, Initial Mass prepare/verify, VehicleData apply orchestration을 Pawn에서 위임하되 lifecycle/Public/BP/Automation facade와 observable/serialized state Authority는 Pawn에 유지했다.
- Official UE 5.8 canonical Build를 fresh 실행해 process `bf809fd5646e45439553eb1a680d0dac`, Exit 0 / `Result: Succeeded`를 확보했다.
- 대표 Plan의 affected exact9를 공용 exact-list same-process runner로 실행해 process `f084579f1f3e4603928e8e5e3bf24554`, 9/9 PASS, failure/missing/unexpected/duplicate 0을 확인했다.
- source equivalence/final review에서 lifecycle ordering, Initialize=CoreReady / Refresh=CombatReady, compat RuntimeReady=CoreReady, RuntimeApply recovery facade, Domain Authority mirror 금지와 Product Asset mutation/save 0을 재확인했다.
- 최종 판정은 **P0 0 / blocking P1 0 / P2 0 PASS**이며 exact next를 `VPS-P0-04 Debug / Header Cleanup`으로 전진했다. 현재 단일 Active `CF-FQ-039`는 변경하지 않는다.

### v0.4.1 - 2026-09-06

- `VPS-P0-03 Runtime Behavior Extraction` 착수 전 current Pawn/RuntimeApply/RuntimeEquip/Fitting test source를 P0-00 Freeze와 다시 대조했다.
- 현재 구현의 P0 결함은 없었지만 P0-03 로컬 항목이 RuntimeComp 책임을 과도하게 넓게 읽을 수 있는 blocking P1 1건을 확인했다.
- `VehicleRuntimeComp` exact 내부 identity, Pawn lifecycle ownership, Prepare/Verify Initial Mass wrapper와 prepare-before-Super 경계, Initialize=CoreReady / Refresh=CombatReady 비대칭, `ApplyVehicleDataConfig()` Automation wrapper, Visual/Domain Authority 분리와 Runtime Vehicle/Equipment Apply rollback/readback facade를 P0-03 전용 구현 계약으로 명시했다.
- 실제 `FCFFittingDefensePIEPipelineTest`가 BP Pawn RegisterAllComponents에서 PreRegister Initial Mass를 거쳐 `InitializeVehicleRuntime()`의 Verify→Snapshot Commit까지 검증하는 것을 확인하고, VehicleData Step8형 reinitialize·RTA Vehicle/Equipment recovery·WheelSync를 포함한 최소 exact validation matrix를 고정했다.
- 교정 후 재검수 결과 **P0 0 / blocking P1 0 / P2 0 PASS**다. 이번 Gate는 contract/document-only이므로 Source implementation/Asset mutation 0, Build/Automation 실행 0이며 다음 Gate는 같은 `VPS-P0-03 Runtime Behavior Extraction` 구현 착수다.

### v0.4.0 - 2026-09-06

- `VPS-P0-02 Fire Behavior Extraction`을 구현해 `UCFVehicleFireComp`를 추가하고 Fire Command/validation/Muzzle-Aim/HitScan-Projectile/Launcher 후속 발사와 Fire side effect orchestration을 Pawn에서 위임했다.
- 기존 Pawn Fire facade, observable state, Automation private seam과 Launcher callback을 보존하고 각 Domain Component의 Weapon/Ammo/Launcher/Projectile/Damage/CombatFx/Aim Authority를 복제하지 않았다.
- 중간검수에서 FireComp의 `NextFireRequestId` 증가와 입력 순간 `LastFireRequest` 직접 write를 blocking P1 1건으로 확인했고, Request ID/시간 할당과 observable commit을 Pawn으로 복귀시켜 P0 Freeze Authority를 복원했다.
- 교정본 공식 UE 5.8 Editor Build `caf81d1766ce41c99b4afb2e6be577ef` PASS와 FireTransaction/Launcher Scheduler/LauncherLock/Heat/ProjectileLaunch/Projectile SourceIsolation exact affected Automation 6/6 PASS를 확인했다.
- current Source에는 CombatFx/FireFeedback를 직접 호출하는 기존 Automation이 없어 source equivalence review로 observable commit과 기존 side effect/feedback source 순서 보존을 재검수했다.
- 최종검수 결과 **P0 0 / blocking P1 0 / P2 0 PASS**, Product Asset mutation/save 0이다. 다음 Gate는 `VPS-P0-03 Runtime Behavior Extraction`이며 이번 작업에서는 착수하지 않았다.

### v0.3.0 - 2026-09-06

- `VPS-P0-01 Visual Behavior Extraction`을 구현해 `UCFVehicleVisualComp`를 추가하고 Chassis/Wheel/Layout/Turret/Owner Visual 행동과 순수 private Visual value cache를 Pawn에서 위임했다.
- 기존 Pawn Public/BP facade, lifecycle ordering, `ApplyVehicleLayoutConfig()` Automation private wrapper, Runtime/Turret observable state Authority와 `SM_Body` gameplay hit collision 의미를 보존했다.
- 중간검수에서 BP/SCS SceneComponent 장기 pointer bookkeeping cache `OwnerVisualStabilizedComponents`를 blocking P1 1건으로 확인했고, cache와 모든 사용처를 제거해 fresh-resolve 계약을 복원했다.
- 교정 후 공식 UE 5.8 Editor Build PASS, 직접 Wheel/Construction affected PASS, Aim exact 1/1 PASS를 확인했다.
- Wheel runner의 마지막 unrelated `VD_P0_01.ValidatorContract` 1회 no-terminal 실패는 exact 격리 재실행 1/1 PASS로 오염성 실패임을 분리했다.
- current Source에는 직접 Turret Visual Automation이 없어 source equivalence review로 Pawn Authority/attachment/aim wrapper 계약을 재검수했다.
- 최종 재검수 결과 **P0 0 / blocking P1 0 / P2 0 PASS**, Product Asset mutation/save 0이다. 다음 Gate는 `VPS-P0-02 Fire Behavior Extraction`이며 이번 작업에서는 착수하지 않았다.

### v0.2.1 - 2026-09-06

- 중간검수에서 `FCFWheelSizeRuntimeVisualTest`가 직접 호출하는 `ApplyVehicleLayoutConfig()`가 Automation private seam Freeze 목록에서 누락된 blocking P1 1건을 확인했다.
- `ApplyVehicleLayoutConfig()` Pawn private compatibility wrapper 유지, BP/SCS `Wheel_Anchor_*` fresh resolve 의미 유지, `LastVehicleRuntimeSummary` Pawn Authority 유지, Visual Component summary mirror 금지를 Freeze Manifest에 추가했다.
- 교정 후 재검수 결과 P0 0 / blocking P1 0 PASS를 확인했으며 Source/Asset mutation은 계속 0이다. Exact next gate는 `VPS-P0-01 Visual Behavior Extraction`을 유지한다.

### v0.2.0 - 2026-09-06

- `VPS-P0-00 Contract / State / Lifecycle Freeze`를 current Source 기준으로 완료하고 Freeze Manifest를 추가했다.
- Public UFUNCTION/UPROPERTY와 Blueprint 노출, Pawn Contract State Authority, Default Subobject identity, BP/SCS fresh name resolve 계약을 동결했다.
- `OnConstruction`, `PreRegisterAllComponents`, `BeginPlay`, `EndPlay`, `Tick`, `SetupPlayerInputComponent`의 current owner/호출 순서와 initial mass prepare-before-Super ordering을 동결했다.
- Runtime Vehicle Apply / Runtime Equip Apply recovery seam, `InitializeVehicleRuntime`, `RefreshFittingDependentRuntime`, Launcher scheduled-shot callback과 Automation friend/private seam을 동결했다.
- Fire observable state, Core/Combat/compat RuntimeReady 의미, Runtime summary/failure semantics와 Turret shared state를 동결했다.
- `TargetPointComp`↔`TargetPoint`, Initialize=CoreReady 반환 / Refresh=CombatReady 반환, compat RuntimeReady=CoreReady 등 의도적 비대칭을 명시적으로 보존했다.
- 설계검수 중 순수 private Visual cache를 Contract State Authority로 과도하게 고정한 Manifest 자기모순 1건을 교정했다. 외부 observable/serialized/shared state는 Pawn에 유지하되 Visual 전용 private cache/runtime state는 단일 소유권 이동 가능, mirror 금지로 확정했다.
- 교정 후 재검수 결과 P0 0 / blocking P1 0 PASS, Source/Asset mutation 0을 확인하고 exact next gate를 `VPS-P0-01 Visual Behavior Extraction`으로 전진시켰다. 구현은 아직 시작하지 않았다.

### v0.1.0 - 2026-09-06

- CF-FQ-048 Vehicle Pawn Slimming을 P2 / Ready 정식 Feature로 승격했다.
- read-only Pawn responsibility audit 결과와 initial design을 통합했다.
- 최초 설계검수의 P0 4 / P1 4를 교정하고 재검수 P0 0 / blocking P1 0 PASS를 반영했다.
- Behavior Extraction / Contract State Freeze, Lifecycle Freeze, BP/SCS fresh resolve, gameplay collision ownership 분리, Launcher/Pawn compatibility seam, test seam 보존을 P0 핵심 계약으로 고정했다.
- 실제 Source/Asset 구현은 시작하지 않았으며 exact next gate를 `VPS-P0-00 Contract / State / Lifecycle Freeze`로 고정했다.
