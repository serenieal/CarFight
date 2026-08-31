# VehicleData

- 문서 버전: v2.2.0
- 최근 갱신일: 2026-09-01
- 문서 상태: Current Implementation
- 적용 범위: `UCFVehicleData`, `UCFVDAValidator`, `ACFVehiclePawn::ApplyVehicleDataConfig()`와 현재 대표 VehicleData 기준

---

## 1. 문서 목적

이 문서는 CarFight에서 `VehicleData`가 **현재 실제로 어떤 데이터를 소유하고, VehiclePawn 런타임에 어떻게 적용되며, 어떤 검증 계약으로 보호되는지** 기록한다.

미래 튜닝 계획이나 사용자 주행감 목표를 기록하는 문서가 아니다.
실제 수치 조정 계획은 `Document/Plan/VehicleDataTuning/VehicleDataTuningPlan.md`가 소유한다.

---

## 2. 현재 핵심 구조

현재 VehicleData 흐름은 다음과 같다.

```text
UCFVehicleData
→ ACFVehiclePawn.VehicleData
→ ApplyVehicleDataConfig()
→ Movement / Reference / WheelPhysics / WheelVisual / TurretVisual / DriveState
→ VehicleRuntime
```

주요 소스:

```text
UE/Source/CarFight_Re/Public/CFVehicleData.h
UE/Source/CarFight_Re/Private/CFVehicleData.cpp
UE/Source/CarFight_Re/Public/CFVDAValidator.h
UE/Source/CarFight_Re/Private/CFVDAValidator.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

---

## 3. UCFVehicleData가 현재 소유하는 영역

`UCFVehicleData`는 단순 주행 튜닝 DataAsset이 아니라 차량 하나의 루트 구성 데이터다.

```text
VehicleVisualConfig
VehicleLayoutConfig
HardpointSlots
MountProfiles
BaseVehicleMassKg
MaximumGrossMassKg
VehicleMovementConfig
WheelVisualConfig
VehicleReferenceConfig
VehicleDurabilityConfig
DefaultDefenseData
DefaultDestroyedFxData
DestroyedFxSocketName
DriveStateConfig
```

각 책임은 다음과 같다.

### VehicleVisualConfig

차체·4개 휠 StaticMesh 참조를 제공한다.

### VehicleLayoutConfig

차량 크기와 중심, 휠베이스·트랙 같은 레이아웃 기준을 제공한다.

### HardpointSlots / MountProfiles

피팅과 전투 장비가 사용하는 차량 위치 슬롯과 안정 장착 규칙을 제공한다.

```text
HardpointSlot.LocationSlotId
MountProfile.MountProfileId
MountProfile.LocationSlotRef
```

`LocationSlotRef`는 실제 `HardpointSlots.LocationSlotId`를 참조해야 한다.

### BaseVehicleMassKg / MaximumGrossMassKg

CF-FQ-034 Fitting이 사용하는 차량 기준 질량과 최대 허용 총중량이다.

```text
0 / 0
= 레거시 미설정 허용

한쪽만 설정
= 불완전 설정

둘 다 > 0
= MaximumGrossMassKg >= BaseVehicleMassKg 필요
```

### VehicleMovementConfig

엔진, 차체 공력, 디퍼렌셜, 조향, Wheel Runtime 상세값과 ThrottleInputScale을 제공한다.

RPM 관련 Current 계약은 다음처럼 분리한다.

```text
EngineIdleRPM
= Chaos EngineSetup.EngineIdleRPM에 적용되는 실제 아이들 RPM

EngineMaxRPM
= Chaos EngineSetup.MaxRPM에 적용되는 실제 물리 엔진 최대 RPM
= HUD Redline 시작값이 아님

RedlineStartRPM
= HUD Tachometer의 차량별 실제 레드라인 시작 RPM
= 현재 Chaos 물리 설정에는 적용하지 않는 authored/presentation 계약
= 0이면 명시적 미설정
= EngineMaxRPM, EngineIdleRPM, 변속 설정에서 자동 추정 금지
= 명시값은 EngineIdleRPM < RedlineStartRPM < EngineMaxRPM
```

`ChangeUpRPM`이라는 별도 CarFight VehicleData 필드는 현재 Source에 존재하지 않으며 Redline source로 가정하지 않는다.

### WheelVisualConfig

Wheel Visual의 적용 모드와 WheelSync가 소비할 시각 기준을 제공한다.

현재 정상 신규 차량 제작의 Wheel Size Authority는 다음과 같다.

```text
canonical shared Wheel StaticMesh Bounds
+
USER-authored Wheel_Anchor Socket RelativeScale
→ Wheel_Mesh visual scale
→ Front/Rear Wheel Radius/Width derived physics size
```

`bUseWheelSocketScale=true`일 때 USER-authored Socket Scale이 차량별 타이어 크기의 authority다. 이 모드에서는 `bAutoScaleWheelMeshToRadius`를 사용하지 않으며 물리 Radius/Width와 시각 크기는 같은 Socket Scale source에서 파생한다.

FL-only shared Wheel Mesh fallback을 허용한다. FR/RL/RR Mesh가 비어 있으면 FL Mesh를 재사용하되, FR/RR처럼 Right slot에서 **null fallback으로 FL을 재사용한 경우에만** runtime이 Right orientation compensation과 spin handedness를 적용한다. explicit FR/RR Mesh는 FL과 같은 pointer여도 자동 반전하지 않는다.

Legacy `bAutoScaleWheelMeshToRadius` / `bAutoCenterWheelMeshBoundsToOrigin` 경로는 기존 Asset 호환을 위해 유지한다. VehicleData hot-reinit에서는 이전 차량이 변경한 Wheel_Mesh Location/Rotation/Scale이 다음 차량에 남지 않도록 BP-authored base full relative transform을 복원한 뒤 현재 VehicleData의 Wheel Visual 설정을 적용한다.

### VehicleReferenceConfig

Front/Rear Wheel Class를 제공한다.

### VehicleDurabilityConfig / DefaultDefenseData

차량 내구도 기본값과 방어 데이터 참조를 제공한다.

### DefaultDestroyedFxData / DestroyedFxSocketName

최초 파괴 Niagara 연출의 차량별 기본값과 우선 소켓을 제공한다.

### DriveStateConfig

Idle / Reversing / Airborne 판정 임계값, 히스테리시스와 상태 Hold 정책을 제공한다.

---

## 4. 실제 런타임 적용 순서

`ACFVehiclePawn::ApplyVehicleDataConfig()`는 현재 다음 순서로 VehicleData를 해석한다.

```text
ApplyVehicleMovementConfig()
ApplyVehicleReferenceConfig()
ApplyVehicleWheelPhysicsConfig()
ApplyVehicleWheelVisualConfig()
ApplyVehicleTurretVisualConfig()
VehicleDriveComp->ApplyDriveStateConfig(...)
```

즉 VehicleData는 저장용 메타데이터가 아니라 실제 VehicleRuntime 구성 입력이다.

단 `VehicleMovementConfig.RedlineStartRPM`은 현재 예외적으로 **물리 Runtime을 직접 바꾸지 않는 명시적 차량 데이터 계약**이다. `EngineMaxRPM`은 기존처럼 Chaos `EngineSetup.MaxRPM`에 적용되지만 `RedlineStartRPM`은 HUD Provider/Presenter가 읽는 authored source이며, Production SpeedGauge visual binding 전까지 차량 주행 결과에는 영향을 주지 않는다.

---

## 5. bUseMovementOverrides의 정확한 현재 의미

`VehicleMovementConfig.bUseMovementOverrides`는 **VehicleMovementConfig 전체를 끄는 스위치가 아니다.**

현재 실제 계약:

```text
bUseMovementOverrides와 무관하게 VehicleData가 있으면 적용
- EngineSetup
- DragCoefficient
- DownforceCoefficient
- CenterOfMassOverride
- DifferentialSetup
- SteeringSetup
- Wheel Class / AdditionalOffset 경로

bUseMovementOverrides=true일 때 차량별 상세값 사용
- Wheel Class CDO Runtime 상세 튜닝
- ThrottleInputScale

bUseMovementOverrides=false
- 세부 Wheel Runtime Tuning 미적용
- ThrottleInputScale = 1.0 fallback
- Engine/Drag/Differential/Steering 본체 적용은 유지
```

2026-08-15 `VD-P0-03 RuntimeApplyContract`가 실제 `ACFVehiclePawn::ApplyVehicleDataConfig()`를 호출해 이 계약을 자동 검증했다.

따라서 UI/Validator/후속 문서에서 이 플래그를 “Movement 전체 적용 사용”으로 설명하면 안 된다.

---

## 6. WheelVisual 자동 스케일 계약

`WheelVisualConfig.bUseWheelVisualOverrides=true`이고 `bAutoScaleWheelMeshToRadius=true`이면 WheelMesh의 로컬 바운드를 기준으로 `FrontWheelRadius / RearWheelRadius`에 맞는 표시 스케일을 계산한다.

현재 Validator 안전 계약:

```text
WheelMeshScaleClampMin > 0
WheelMeshScaleClampMax > 0
WheelMeshScaleClampMin <= WheelMeshScaleClampMax
FrontWheelRadius > 0
RearWheelRadius > 0
```

런타임 내부 clamp fallback이 잘못된 데이터를 조용히 숨기는 용도로 사용되지 않도록 입력 단계에서 검증한다.

---

## 7. VehicleData Validator

현재 공식 검증기는 기존 `UCFVDAValidator` 하나를 사용한다.
병렬 Validator를 만들지 않는다.

현재 주요 검증 범위:

```text
필수 자산 참조
Wheel Socket
Layout / WheelAnchor
HardpointSlots
Fitting Mass
MountProfiles
Movement
WheelVisual
DriveState
기준 VehicleData 비교
```

### Fitting Mass

```text
Base=0 && MaximumGross=0
→ Info / 레거시 미설정 허용

한쪽만 0
→ Error

음수 / 비유한 값
→ Error

MaximumGross < Base
→ Error
```

### MountProfiles

```text
MountProfiles 비어 있음
→ Info / 장비 없는 차량 허용

MountProfileId 없음
→ Error

MountProfileId 중복
→ Error

LocationSlotRef 없음
→ Error

LocationSlotRef가 HardpointSlots에 없음
→ Error
```

### Movement

`bUseMovementOverrides=false`는 Error가 아니라 현재 fallback 의미를 설명하는 Info다.
ThrottleInputScale은 `bUseMovementOverrides=true`일 때만 0 이하를 Error로 본다.

`RedlineStartRPM` Validator 계약:

```text
RedlineStartRPM = 0
→ 유효한 미설정 / 기존 VehicleData 호환

RedlineStartRPM < 0
→ Error

RedlineStartRPM > 0
→ EngineIdleRPM보다 커야 함
→ EngineMaxRPM보다 작아야 함

자동 보정 / EngineMaxRPM fallback
→ 금지
```

현재 Data Authoring compatibility surface는 `RedlineStartRPM` additive leaf를 포함한 **118 Registry leaf**다. P0-08 당시 original 117 Registry 검증 증거는 Historical scope로 유지한다.

---

## 8. Representative Compare

`UCFVDAValidator::CompareVehicleData()`는 차량의 좋고 나쁨을 자동 판정하지 않는다.
기준 차량과 대상 차량의 데이터 차이를 추적 가능한 FieldPath로 보고한다.

현재 비교 범위의 주요 항목:

```text
MovementProfileName
bUseMovementOverrides
EngineMaxTorque / EngineMaxRPM
ThrottleInputScale
FrontWheelMaxSteerAngle
Front/Rear WheelRadius
Front/Rear WheelWidth
Friction
SpringRate
CenterOfMass override
WheelVisual auto-scale
WheelVisual clamp min/max
BaseVehicleMassKg
MaximumGrossMassKg
DriveState override / 주요 threshold
```

실제 수치 변경은 별도 USER 튜닝 결정이다.

---

## 9. 현재 대표 read-only baseline

2026-08-15 AssetDump로 다시 확인한 현재 비교 기준은 다음 두 자산이다.

```text
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan.DA_TestSedan
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV.DA_TestSUV
```

### DA_TestSedan

확인된 대표 값:

```text
bUseMovementOverrides = true
ThrottleInputScale = 0.600
FrontWheelMaxSteerAngle = 37.0 deg
Front/Rear WheelRadius = 35 cm
DriveState override = true
Layout override = true
WheelVisual override = true
Wheel auto-scale = true
Wheel scale clamp = 0.25 .. 4.0
BaseVehicleMassKg = 0
MaximumGrossMassKg = 0
Hardpoints = Front_01, Top_01
MountProfile = RoofTurret_MediumOrLarge → Top_01
```

### DA_TestSUV

확인된 대표 값:

```text
bUseMovementOverrides = true
ThrottleInputScale = 0.504
FrontWheelMaxSteerAngle = 33.48 deg
Front/Rear WheelRadius = 40 cm
DriveState override = true
Layout override = true
WheelVisual override = true
Wheel auto-scale = false
Wheel scale clamp = 0.25 .. 4.0
BaseVehicleMassKg = 0
MaximumGrossMassKg = 0
Hardpoints = Front_01, Top_01, Top_02
MountProfile = RoofTurret_MediumOrLarge → Top_01
```

AssetDump 결과가 노출하지 않은 WheelWidth 등은 추정하지 않는다.

`DA_PoliceCar`는 현재 대표 VehicleData 기준이 아니다. 과거 문서의 DA_PoliceCar 값은 Historical 참고로만 본다.

---

## 10. 자동 검증 상태

CF-FQ-015 원격 기술 체크포인트:

```text
Official UE 5.8 Editor Build
0cffed2f02674b6692d4f8af811d9036
PASS / Exit 0

CarFight.VehicleData
59091b9559db46859c3a521be72396bf
3/3 PASS

- CarFight.VehicleData.VD_P0_01.ValidatorContract
- CarFight.VehicleData.VD_P0_02.RepresentativeCompare
- CarFight.VehicleData.VD_P0_03.RuntimeApplyContract
```

`VD-P0-03`은 Transient `ACFVehiclePawn`과 `UCFVehicleData`를 사용해 실제 `ApplyVehicleDataConfig()` 경로를 실행했다.

검증 내용:

```text
bUseMovementOverrides=false에서도
- Engine MaxTorque / MaxRPM 적용
- Drag / Downforce 적용
- Differential type / split 적용
- Steering type / ratio 적용

DriveState override=true
- DriveState 설정 전달

DriveState override=false
- 기존 DriveComp 값 유지
```

테스트는 Content Asset을 수정·저장하지 않았다.

Wheel Class CDO의 전역 상세 Runtime setter는 테스트 격리 위험 때문에 이 회귀에서 의도적으로 변경하지 않았다.

---

## 11. 현재 미완료 범위

CF-FQ-015의 원격 기술 계약은 완료됐지만 **실제 차량 주행감 튜닝은 완료되지 않았다.**

남은 `VD-P0-04 USER Tuning`:

```text
DA_TestSedan 실제 출발·가속
제동
저속 조향
중속 조향
고속 안정성
작은 턱 / 서스펜션 체감
DA_TestSUV 동일 항목 비교
필요한 축만 한 번에 하나씩 수정
```

사용자가 PIE를 직접 확인하기 전에는 Engine/Wheel/DriveState 값을 자동 조정하지 않는다.

---

## 12. 책임과 비책임

VehicleData 책임:

```text
차량 구성 데이터 소유
안정 ID와 참조 제공
런타임 적용 입력 제공
Validator 입력 계약 제공
```

VehicleData 비책임:

```text
실제 차량 입력 처리
DriveState 계산 자체
WheelSync 계산 자체
전투 피해 계산
UI 표시
USER 주행감 자동 판정
```

---

## 13. 갱신 조건

다음이 변경되면 이 문서를 갱신한다.

```text
CFVehicleData 구조
ApplyVehicleDataConfig 소비 경로
bUseMovementOverrides 의미
WheelVisual auto-scale 정책
Fitting mass 계약
Hardpoint / MountProfile 계약
CFVDAValidator 검증 범위
대표 VehicleData baseline
USER 튜닝 결과
```

---

## 14. Changelog

### v2.2.0 - 2026-09-01

- CF-FQ-040 Wheel Size Authority P0 USER PASS 결과를 Current 계약으로 승격했다.
- `bUseWheelSocketScale=true`에서 USER-authored Wheel Socket Scale을 시각/물리 Wheel Size의 단일 authority로 정의하고 canonical shared Wheel Bounds와 함께 Radius/Width를 파생하는 계약을 추가했다.
- FL-only shared Mesh null fallback의 Right slot orientation/spin 보정과 explicit Right Mesh 예외 계약을 반영했다.
- runtime hot-reinit 시 authored Wheel_Mesh full relative transform(Location/Rotation/Scale)을 복원해 Legacy AutoScale/AutoCenter, SocketScale, Manual mode 사이의 stale transform 잔류를 금지하는 현재 동작을 기록했다.

### v2.1.0 - 2026-08-18

- `FCFVehicleMovementConfig`에 `RedlineStartRPM` explicit authored field가 additive 추가된 Current 구현을 반영했다. `EngineMaxRPM`은 Chaos `EngineSetup.MaxRPM` 물리 상한, `RedlineStartRPM`은 HUD Tachometer 레드라인 시작 source로 책임을 분리했다.
- `RedlineStartRPM=0`을 backward-compatible 미설정으로 정의했다. 값이 명시되면 `EngineIdleRPM < RedlineStartRPM < EngineMaxRPM`을 `UCFVDAValidator v1.5.0`이 강제하고 자동 보정·fallback은 하지 않는다.
- Current HUD Provider는 Current Engine RPM을 Chaos Runtime에서, Redline/Maximum을 VehicleData에서 읽는다. Presenter mapping은 Redline→0.85, EngineMaxRPM→1.0이며 Production SpeedGauge Asset binding은 아직 Pending이다.
- additive VehicleData leaf에 맞춰 Current Data Authoring Registry compatibility surface는 118로 확장됐다. P0-08의 original 117 Registry PASS는 Historical evidence로 보존한다.
- Official Build `745dba430bcc47c2925c841a0c5a6686` PASS와 신규 RPM source/presentation Automation 각 1/1 PASS를 확보했다. 대표 VehicleData Asset의 실제 Redline 값은 이번 slice에서 작성하지 않았다.

### v2.0.0 - 2026-08-15

- 2026-04~07의 DA_PoliceCar 중심 구형 설명을 현재 `DA_TestSedan / DA_TestSUV` baseline으로 전면 교정했다.
- Layout, Hardpoint, MountProfile, Fitting Mass, Defense/Fx/Durability까지 확장된 현재 UCFVehicleData 범위를 반영했다.
- `bUseMovementOverrides`의 실제 런타임 의미를 Engine/Drag/Steering 본체와 Wheel Runtime/Throttle fallback으로 구분했다.
- `UCFVDAValidator v1.3.0+`의 Fitting Mass, MountProfile, Movement flag, Wheel auto-scale 계약과 Representative Compare 범위를 기록했다.
- CF-FQ-015 VD-P0-00~03 Technical PASS와 Build·Automation 증거를 반영했다.
- 실제 VehicleData 튜닝값은 변경하지 않았으며 VD-P0-04 USER Tuning은 Pending으로 명시했다.

### v1.1.0 - 2026-07-08

- WheelVisualConfig WheelRadius 기반 WheelMesh 자동 스케일 옵션을 기록했다.

### v1.0.0 - 2026-04-22

- VehicleData 문서 최초 작성.

---

## 15. Migration

- 기존 VehicleData는 `RedlineStartRPM=0` 기본값으로 기존 주행 물리를 그대로 유지한다. HUD에서 임의 Redline을 만들지 않는다.
- 실제 Redline을 authoring할 때는 대표 차량의 Idle/Max와 설계 의도를 확인한 명시값만 사용하며 `EngineMaxRPM * 0.85` 같은 자동 산식을 사용하지 않는다.
- P0-08 당시 117-field 문서/테스트 결과는 당시 schema의 Historical evidence다. Current Source 판단은 additive `RedlineStartRPM`을 포함한 118-field Registry를 우선한다.
- `DA_PoliceCar`를 현재 대표 기준으로 사용하는 과거 설명은 Historical로 본다.
- 현재 P0 대표 비교 경로는 `DA_TestSedan / DA_TestSUV`다.
- `bUseMovementOverrides=false`를 VehicleMovement 전체 비활성으로 해석하지 않는다.
- CF-FQ-015 Technical PASS를 USER 주행감 PASS로 승격하지 않는다.
