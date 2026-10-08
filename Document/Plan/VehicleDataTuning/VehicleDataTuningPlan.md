# CF-FQ-015 차량 데이터 튜닝 패스 Plan

- 문서 버전: v0.3.0
- 최근 갱신일: 2026-09-11
- 문서 상태: Done / Historical + Retained Path / Rebaseline Complete
- Feature: `CF-FQ-015 차량 데이터 튜닝 패스`
- 현재 단계: `VD-P0-00~03 Historical Technical PASS / VD-P0-04 Superseded / Not Executed`
- USER PIE: VD-P0-04를 수행해 PASS한 기록 없음 / 후속 VehicleBuilder Performance Tuning Protocol로 책임 이관

---

## 2026-09-11 Final Rebaseline Closure

`CF-FQ-015 Vehicle Data Tuning`은 원래 남겨둔 `VD-P0-04 USER Tuning`을 그대로 재개하지 않고 현재 차량 제작·Authoring·Driving Test 체계에 맞춰 Rebaseline한 뒤 **Done / Historical + Retained Path**로 종료한다.

```text
VD-P0-00 Foundation Audit
= Historical Technical PASS

VD-P0-01 Validator Contract
= Historical Technical PASS

VD-P0-02 Representative Compare
= Historical Technical PASS

VD-P0-03 Runtime Apply Contract
= Historical Technical PASS

VD-P0-04 USER Tuning
= Superseded / Not Executed
= USER PASS 아님
```

기존 기술 evidence는 보존한다.

```text
Historical Official UE 5.8 Editor Build
= 0cffed2f02674b6692d4f8af811d9036 / PASS / Exit 0

Historical CarFight.VehicleData
= 59091b9559db46859c3a521be72396bf / 3/3 PASS

VehicleData Content Asset mutation
= 0
```

당시 남겨둔 Sedan/SUV Raw USER Tuning은 후속 `CF-FQ-038 Data Authoring`, `CF-FQ-040 Vehicle Builder`와 Wagon Driving Benchmark / persistent USER Driving Acceptance가 더 완전한 workflow를 제공하게 되어 그대로 실행할 필요가 없어졌다. 따라서 미수행 USER Gate를 PASS로 꾸며 닫지 않고 **책임 자체가 successor Current System으로 대체된 것으로 종료**한다.

Current owner:

```text
Performance Tuning workflow
→ main_game Document/Systems/Vehicles/VehicleBuilder.md v1.6.0

VehicleData foundation
→ main_game Document/Systems/Vehicles/VehicleData.md v2.3.0
```

CF-FQ-015에서 유지 가치가 있는 네 계약은 VehicleBuilder Current System으로 승격됐다.

```text
1. Controlled Axis Tuning
   - 원인 추적을 위해 한 번에 한 행동 축부터 조정

2. Technical Benchmark + USER Feel Pair
   - 기술 수치와 사람의 주행감은 서로 대체하지 않음

3. Vehicle Character / Reference Baseline
   - 차량의 의도·비교 기준을 먼저 정의하고 Raw 차이를 좋음/나쁨으로 자동 판정하지 않음

4. Measurement Gap / Benchmark Extension Ownership
   - 현재 계측할 수 없는 축은 임의 PASS하지 않고 Measurement Gap으로 기록
```

현재 일반 구현 완료로 확대하지 않는 Measurement Gap 후보:

```text
100→0 제동 시간 / 거리
Yaw Rate / 횡가속
Slip Angle
Suspension stroke / settling
Airborne false-positive 정량 관측
```

이 closure는 문서/책임 Rebaseline이다. 새 Source/Asset/Build/Automation/PIE mutation은 0이며, 완료된 VD-P0-00~03을 재실행하지 않는다.

Historical placement:

```text
G0 = PASS — 기존 기술 기반 + successor tuning/acceptance 경로 확인
G1 = PASS — VehicleBuilder v1.6.0 + VehicleData v2.3.0 Current 승격
G2 = PASS — ActiveWork / ProjectSSOT / Plan Index stale current route 제거
G3 = PASS — VehicleDataTuningPlan.md retained path 보존
G4 = PASS — semantic Historical
G5 = Deferred — physical move optional
```

---

## 1. 목적

CF-FQ-015의 목표는 기준 차량의 Movement/Wheel/DriveState 값을 무작정 조정하는 것이 아니다.
먼저 `UCFVehicleData`에 저장된 값이 실제 런타임에 어떻게 적용되고, 기준 차량과 비교 차량 사이의 차이가 어떤 데이터에서 발생하는지 추적 가능하게 만든다.

```text
VehicleData 저장값
→ Validator 계약
→ 기준/비교 Snapshot
→ Runtime 적용 계약
→ USER 주행감 검증
→ 필요한 값만 튜닝
```

실제 주행감이 확인되지 않은 상태에서 Engine, Steering, Wheel, DriveState 수치를 임의로 변경하지 않는다.

---

## 2. 보호 조건

- `CF-FQ-026 TargetSelect / TS-P0-08`의 기존 Technical PASS와 USER 재검증 체크포인트를 그대로 보존한다.
- 사용자가 PIE를 직접 볼 수 없는 동안 새 USER PASS를 추정하지 않는다.
- `DA_TestSedan`, `DA_TestSUV`와 기타 VehicleData 자산은 이번 Foundation 단계에서 읽기 전용 기준 자산이다.
- VehicleData Asset을 자동 수정·저장하지 않는다.
- 기존 미커밋 Content Asset과 `WBP_TargetSelect.uasset`를 건드리지 않는다.
- 실제 차량 감각을 바꾸는 수치 변경은 `VD-P0-04 USER Tuning` 전에는 수행하지 않는다.
- 현재 `BaseVehicleMassKg=0 / MaximumGrossMassKg=0`은 Fitting 미설정 레거시 허용 상태이며 오류로 승격하지 않는다.

---

## 3. 현재 실제 구현 기준

### 3.1 VehicleData

현재 `UCFVehicleData`는 다음 주요 영역을 소유한다.

```text
VehicleVisualConfig
VehicleLayoutConfig
HardpointSlots
MountProfiles
BaseVehicleMassKg / MaximumGrossMassKg
VehicleMovementConfig
WheelVisualConfig
VehicleReferenceConfig
VehicleDurabilityConfig
DefaultDefenseData
DefaultDestroyedFxData
DriveStateConfig
```

`ACFVehiclePawn::ApplyVehicleDataConfig()`는 현재 다음 경로를 적용한다.

```text
ApplyVehicleMovementConfig
ApplyVehicleReferenceConfig
ApplyVehicleWheelPhysicsConfig
ApplyVehicleWheelVisualConfig
ApplyVehicleTurretVisualConfig
VehicleDriveComp.ApplyDriveStateConfig
```

### 3.2 bUseMovementOverrides 현재 의미

현재 런타임에서 `bUseMovementOverrides`는 전체 VehicleMovementConfig의 단일 on/off 스위치가 아니다.

```text
ApplyVehicleMovementConfig
- Engine / Drag / Downforce / COM / Differential / Steering 본체 값은 VehicleData가 있으면 적용

ApplyVehicleWheelPhysicsConfig
- Wheel Class / AdditionalOffset는 적용
- 세부 Wheel Runtime Tuning은 bUseMovementOverrides=true일 때 적용

SetVehicleThrottleInput
- ThrottleInputScale은 bUseMovementOverrides=true일 때 사용
- false이면 1.0 fallback
```

따라서 기존 Validator의 “VehicleMovement 덮어쓰기가 꺼져 있습니다”라는 단일 문구는 현재 실제 의미를 충분히 설명하지 못한다. VD-P0-01에서 이를 정확한 안내 계약으로 교정한다.

---

## 4. 대표 VehicleData Baseline — 2026-08-15 read-only evidence

### DA_TestSedan

ObjectPath:
`/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan.DA_TestSedan`

확인된 대표 값:

```text
bUseMovementOverrides = true
ThrottleInputScale = 0.600
FrontWheelMaxSteerAngle = 37.0 deg
Front/Rear WheelRadius = 35 cm
DriveState bUseDriveStateOverrides = true
Layout bUseLayoutOverrides = true
WheelVisual bUseWheelVisualOverrides = true
WheelVisual bAutoScaleWheelMeshToRadius = true
BaseVehicleMassKg = 0
MaximumGrossMassKg = 0
HardpointSlots = Front_01, Top_01
MountProfile = RoofTurret_MediumOrLarge → Top_01
```

### DA_TestSUV

ObjectPath:
`/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV.DA_TestSUV`

확인된 대표 값:

```text
bUseMovementOverrides = true
ThrottleInputScale = 0.504
FrontWheelMaxSteerAngle = 33.48 deg
Front/Rear WheelRadius = 40 cm
DriveState bUseDriveStateOverrides = true
Layout bUseLayoutOverrides = true
WheelVisual bUseWheelVisualOverrides = true
WheelVisual bAutoScaleWheelMeshToRadius = false
BaseVehicleMassKg = 0
MaximumGrossMassKg = 0
HardpointSlots = Front_01, Top_01, Top_02
MountProfile = RoofTurret_MediumOrLarge → Top_01
```

두 대표 자산 모두 현재 `bUseMovementOverrides=true`이므로 기존 flag 의미의 모호성이 현재 대표 차량 런타임을 즉시 깨고 있다고 판정하지 않는다.

---

## 5. 작업 단계

| Task | 이름 | 상태 | 종료 기준 |
| --- | --- | --- | --- |
| `VD-P0-00` | Foundation Audit | Done | VehicleData/Validator/runtime apply/대표 Sedan·SUV 저장값 확인 |
| `VD-P0-01` | Validator Contract | Technical Done | 현재 VehicleData 계약 보강 + `ValidatorContract` PASS |
| `VD-P0-02` | Representative Compare | Technical Done | 기존 CompareVehicleData 확장 + `RepresentativeCompare` PASS / Asset 변경 0 |
| `VD-P0-03` | Runtime Apply Contract | Technical Done | 실제 `ACFVehiclePawn::ApplyVehicleDataConfig()` 경로 `RuntimeApplyContract` PASS |
| `VD-P0-04` | USER Tuning | Superseded / Not Executed | 후속 VehicleBuilder Performance Tuning Protocol로 책임 이관 / USER PASS로 확대 금지 |

---

## 6. VD-P0-01 — Validator Contract

기존 `UCFVDAValidator`를 폐기하거나 새 Validator를 중복 생성하지 않는다.
현재 구현을 확장해 CF-FQ-015의 정식 데이터 검증 기반으로 사용한다.

### 6.1 보강 범위

#### Movement 안내 의미 교정

`bUseMovementOverrides=false` 자체를 “Movement 전체 미적용”처럼 설명하지 않는다.

```text
false
= Wheel Runtime 상세 Override와 ThrottleInputScale이 fallback 경로를 사용
!= Engine/Drag/Steering VehicleData 값 전체 미적용
```

false 상태는 Error가 아니라 Info/Warning 안내로 유지한다.

#### Fitting Mass

안전 계약:

```text
Base=0 && MaximumGross=0
→ 미설정 / 레거시 허용 / 오류 아님

한쪽만 0
→ Error / Fitting 질량 계약 불완전

Base>0 && MaximumGross>0 && MaximumGross<Base
→ Error
```

#### MountProfile ↔ Hardpoint

검증:

```text
MountProfileId 없음
중복 MountProfileId
LocationSlotRef 없음
LocationSlotRef가 실제 HardpointSlots에 없음
```

기존 Fitting/Vehicle 런타임이 참조하는 안정 ID 계약을 Validator에서도 조기에 확인한다.

#### Wheel auto-scale

`bAutoScaleWheelMeshToRadius=true`일 때 다음을 검증한다.

```text
WheelMeshScaleClampMin > 0
WheelMeshScaleClampMax > 0
Min <= Max
사용할 Front/Rear WheelRadius > 0
```

런타임의 0.01 clamp fallback에 의존해 잘못된 데이터를 숨기지 않는다.

### 6.2 Automation

새 테스트 필터:

`CarFight.VehicleData.VD_P0_01.ValidatorContract`

검증 항목:

```text
Missing target report
Movement flag 안내 의미
Legacy mass 0/0 허용
Partial mass 설정 문제
MaximumGross < Base 오류
MountProfile missing/duplicate/unknown LocationSlotRef
Valid MountProfile + Hardpoint PASS
Wheel auto-scale Min/Max 오류
Representative-like valid transient VehicleData 기본 계약
```

테스트는 Transient DataAsset만 사용하고 Content Asset을 저장하지 않는다.

---

## 7. VD-P0-02 — Representative Compare

`DA_TestSedan`을 P0 기준, `DA_TestSUV`를 비교 대상으로 사용한다.

비교는 실제 데이터 차이를 보여주는 도구이며 좋음/나쁨을 자동 판정하지 않는다.

최소 비교 항목:

```text
MovementProfileName
ThrottleInputScale
EngineMaxTorque / MaxRPM
FrontWheelMaxSteerAngle
Wheel Radius / Width
Friction / Spring
DriveState 주요 threshold
Wheel auto-scale 상태
Base / MaximumGross mass 설정 상태
```

PIE 없이 가능한 단계에서는 값 변경 없이 비교 결과와 추적 가능성만 닫는다.

---

## 8. VD-P0-03 — Runtime Apply Contract

목표는 “차가 재밌다”를 자동화하는 것이 아니라 DataAsset 값이 올바른 런타임 필드로 전달되는지 검증하는 것이다.

후보 범위:

```text
Engine torque / RPM
Drag / Downforce
Differential split
Steering type / ratio
Wheel class / offsets
Wheel runtime tuning flag
ThrottleInputScale 경로
DriveState config handoff
WheelVisual expected/front count
```

가능한 항목만 Transient World/Pawn으로 검증하고 실제 주행감은 VD-P0-04에 남긴다.

---

## 9. VD-P0-04 — USER Tuning — Historical / Superseded

아래 절차는 2026-08-15 당시 계획한 Historical USER Tuning 순서다. 실제로 USER PASS까지 수행한 기록이 아니며 2026-09-11 Rebaseline에서 successor VehicleBuilder protocol로 대체되어 더 이상 Current next gate가 아니다.

당시에는 사용자가 PIE를 직접 볼 수 있을 때만 수행하도록 계획했다.

권장 순서:

```text
DA_TestSedan 기준 주행
→ 출발/0~30km/h
→ 제동
→ 저속 조향
→ 중속 조향
→ 고속 안정성
→ 작은 턱/서스펜션
→ DA_TestSUV 동일 항목 비교
→ 문제 축 하나씩만 수정
```

여러 값을 동시에 조정하지 않는다.

---

## 10. 현재 검증 결과와 다음 작업

```text
Official UE 5.8 Editor Build
= 0cffed2f02674b6692d4f8af811d9036 / PASS / Exit 0

CarFight.VehicleData
= 59091b9559db46859c3a521be72396bf / 3/3 PASS
- VD-P0-01 ValidatorContract
- VD-P0-02 RepresentativeCompare
- VD-P0-03 RuntimeApplyContract

VehicleData Content Asset 변경
= 0
```

VD-P0-03은 Transient `ACFVehiclePawn`에 실제 `ApplyVehicleDataConfig()`를 호출해 다음을 확인했다.

```text
bUseMovementOverrides=false
→ Engine MaxTorque / MaxRPM 적용
→ Drag / Downforce 적용
→ Differential type / FrontRearSplit 적용
→ Steering type / AngleRatio 적용

DriveState override=true
→ 히스테리시스·홀드·Idle/Reverse/Airborne/Input threshold 전달

DriveState override=false
→ 기존 DriveComp 값 유지
```

전역 Wheel Class CDO를 변경할 수 있는 상세 Wheel Runtime setter는 VD-P0-03에서 의도적으로 제외했다. 해당 값은 Validator 계약으로 보호하며 실제 차량 주행 확인 없이 전역 CDO 상태를 테스트용으로 변형하지 않는다.

Historical note: 2026-08-15 당시에는 원격 기술 Gate가 모두 끝나고 다음 실제 Gate가 `VD-P0-04 USER Tuning`이었다. 2026-09-11 Rebaseline 이후 이 next gate는 폐기됐으며 CF-FQ-015에 현재 남은 실행 Gate는 없다.

---

## 11. Changelog

### v0.3.0 - 2026-09-11

- `CF-FQ-015 Vehicle Data Tuning`을 Rebaseline Complete / Done / Historical + Retained Path로 전환했다.
- VD-P0-00~03 Historical Technical PASS와 Build `0cffed...`, `CarFight.VehicleData` 3/3 evidence는 보존하고 재실행하지 않는다.
- VD-P0-04 Sedan/SUV Raw USER Tuning은 실제 USER PASS가 아니라 `Superseded / Not Executed`로 종료했다. 후속 Data Authoring / VehicleBuilder가 Authoring·Benchmark·USER Acceptance를 소유한다.
- Controlled Axis Tuning, Technical Benchmark + USER Feel Pair, Vehicle Character / Reference Baseline, Measurement Gap / Benchmark Extension Ownership 네 계약을 main_game `VehicleBuilder.md v1.6.0`으로 승격했다. VehicleData foundation은 `VehicleData.md v2.3.0`이 유지한다.
- 제동·Yaw Rate·횡가속·Slip Angle·Suspension 계측은 구현 완료로 확대하지 않고 Measurement Gap 후보로 남겼다. Source/Asset/Build/Automation/PIE mutation은 0이다.

### v0.2.0 - 2026-08-15

- VD-P0-01 Validator Contract를 Technical Done으로 전환했다. 기존 UCFVDAValidator에 피팅 질량, MountProfile↔Hardpoint, Movement flag 의미와 Wheel auto-scale 정합성 검증을 추가했다.
- VD-P0-02 Representative Compare를 Technical Done으로 전환했다. 기존 CompareVehicleData에 Movement override, Wheel width, WheelVisual auto-scale/clamp와 Fitting mass 비교를 추가했다.
- VD-P0-03 Runtime Apply Contract를 Technical Done으로 전환했다. 테스트 전용 friend 경계만 추가하고 실제 ACFVehiclePawn::ApplyVehicleDataConfig 경로에서 Movement 본체와 DriveState 전달을 검증했다.
- 공식 Build `0cffed2f02674b6692d4f8af811d9036` PASS, `CarFight.VehicleData` `59091b9559db46859c3a521be72396bf` 3/3 PASS를 기록했다.
- DA_TestSedan/DA_TestSUV와 기타 VehicleData Content Asset 값은 변경하지 않았다. 실제 주행감 튜닝은 VD-P0-04 USER 단계로 보존한다.

### v0.1.0 - 2026-08-15

- CF-FQ-015를 원격 기술 작업용 Active Plan으로 최초 등록했다.
- 기존 UCFVehicleData, UCFVDAValidator, ACFVehiclePawn 적용 경로와 DA_TestSedan/DA_TestSUV 저장값을 fresh 감사했다.
- 기존 Validator를 폐기하지 않고 현재 VehicleData 계약으로 승격하는 VD-P0-01을 정의했다.
- 실제 VehicleData 값 변경과 USER 주행감 튜닝을 VD-P0-04로 분리해 현재 원격 단계에서 금지했다.

---

## 12. Migration

- VD-P0-00~03은 Historical Technical PASS다. 이 완료를 실제 주행감 USER PASS로 해석하지 않는다.
- VD-P0-04는 USER PASS가 아니라 Superseded / Not Executed다. 새 차량 성능 튜닝은 main_game `VehicleBuilder.md v1.6.0` Performance Tuning Protocol을 사용한다.
- `bUseMovementOverrides=false`의 현재 계약은 Engine/Drag/Differential/Steering 본체 미적용이 아니라 세부 Wheel Runtime Tuning과 ThrottleInputScale fallback이다.
- 기존 `CFVDAValidator`와 VehicleData 필드명을 유지한다. 새 병렬 Validator를 만들지 않는다.
- 기존 Sedan/SUV Asset은 VD-P0-01~03에서 read-only reference로 취급한다.
- VehicleData Systems 문서의 오래된 기준값은 실제 코드·AssetDump보다 우선하지 않는다.
- TS-P0-08 USER PIE 체크포인트는 취소되지 않으며 사용자 시각 확인 가능 시 그대로 재개한다.
