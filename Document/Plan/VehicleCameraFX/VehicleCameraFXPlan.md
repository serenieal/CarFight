# CF-FQ-057 Vehicle Camera Driving FX — 주행 카메라 연출 고도화 계획

- 문서 버전: v0.3.0
- 작성일: 2026-09-27 (Asia/Seoul)
- 최근 갱신일: 2026-10-02 (Asia/Seoul)
- 상태: Historical + Retained Path / VCFX-P0-04 CURRENT SYSTEM PROMOTION + CLOSURE COMPLETE / LATERAL USER PASS / OVERSPEED USER PASS / REAR KICK FUNCTION CONFIRMED + TUNING DEFERRED / BRAKE KICK FUNCTION CONFIRMED + TUNING DEFERRED / CAMERA POP-COLLISION USER PASS / AIM-TARGETING INTERFERENCE USER PASS / COMFORT REVIEW DEFERRED / MODE ATTENUATION WIRING DEBT
- Feature: `CF-FQ-057`
- UDS Work: `wrk_74f4a7c1e03b4cb7b0a8134cb89d2f16`
- Project: `CarFight`, `project_id=carfight`, `repository_role=main_game`, branch `SwitchSourceEngine`
- 공식 엔진: Unreal Engine 5.8 Source Build
- Lifecycle(상태·현재 Phase·정확한 Next) 권한: `Document/UDS/records/work/wrk_74f4a7c1e03b4cb7b0a8134cb89d2f16/`
- 현재 구현 권한: 실제 Source / 저장 Asset / `Document/Systems/Vehicles/VehicleCamera.md`

---

## 1. 목적

CarFight의 차량 주행에서 실제 차량 물리 속도나 가속도를 인위적으로 바꾸지 않고, **Camera Presentation(카메라 표현) 계층이 속도·가속·제동·횡가속을 더 풍부하게 전달하도록 개선**한다.

이번 Feature는 기존 차량 카메라를 새로 만드는 작업이 아니다. 현재 `UCFVehicleCameraComp`와 `UCFVehicleCameraData`가 이미 제공하는 자유 조준, SpringArm(스프링 암), 속도 기반 FOV(시야각), 충돌 보조, Pitch/Roll 분리, Yaw 완충을 기준선으로 보존하면서 주행감 연출을 확장한다.

### 1.1 차량별 정규화 우선 원칙

**Camera FX의 입력 강도는 모든 차량에 동일한 절대 속도·가속도 숫자를 공통 적용하지 않는다.** 차량마다 최고속도, 가속 성능, 제동 성능, 코너링 한계와 의도된 주행감이 다르므로, P0 기본 계약은 각 차량의 자기 성능 범위를 기준으로 한 Normalized Ratio(정규화 비율, 0.0~1.0 / 0~100%)를 사용한다.

예시:

```text
Vehicle A 최고속도 = 120 km/h, 현재속도 = 90 km/h  → SpeedRatio = 75%
Vehicle B 최고속도 = 240 km/h, 현재속도 = 180 km/h → SpeedRatio = 75%
```

두 차량은 절대속도는 다르지만 각각 자기 성능 범위의 75%를 사용 중이므로 Camera FX의 기본 속도 구간은 동일한 75%로 해석한다. 이후 차량별 CameraData의 응답 Curve(커브)와 강도 배율로 서로 다른 성격을 줄 수 있다.

기본 입력 계약은 다음 방향을 따른다.

| 입력 | 금지되는 기본안 | P0 기본안 |
| --- | --- | --- |
| Speed | 모든 차량에 `120 km/h` 같은 공통 임계값 | `PlanarSpeed / ReferenceMaxSpeed` 정규화 비율 |
| Acceleration | 모든 차량에 동일한 `m/s²` Kick 임계값 | `ForwardAcceleration / ReferenceMaxSpeed` 기반 정규화 변화율 |
| Braking | 모든 차량에 동일한 감속 임계값 | `-ForwardAcceleration / ReferenceMaxSpeed` 기반 정규화 변화율 |
| Lateral Motion | 모든 차량에 동일한 횡가속/Roll 임계값 또는 Steering 입력 직접 사용 | `LateralAcceleration / ReferenceMaxSpeed` signed 기본 요구량 × 실제 차체 Motion intensity × 자유시점 signed ViewAlignment |

단, FOV Degree(시야각 도), Camera Offset(카메라 위치), Roll Degree(롤 각도)처럼 **최종 화면에 적용되는 출력값에는 안전을 위한 절대 Clamp(상한/하한)가 필요**하다. 즉 `입력 판단 = 차량별 비율 중심`, `최종 화면 출력 = 차량별 튜닝 + 절대 안전 상한`으로 분리한다.

VCFX-P0-00 Fresh Rebaseline 결과, 현재 `UCFVehicleData`에는 엔진 Torque/RPM, Gear Ratio, Brake Torque, Steering/Wheel/Drag 등 차량별 물리 원인값은 존재하지만 런타임에서 직접 소비할 단일 최고속도 기준값은 없다. Vehicle Builder Benchmark는 실제 `PeakSpeedKmh`, 0→100, 제동, Turning 결과를 측정하지만 이는 측정/검수 evidence이며 현재 packaged runtime authority가 아니다.

따라서 P0 기준은 **차량별 `ReferenceMaxSpeed` 성격의 authored performance reference exact1을 VehicleData 계층에 두고, Camera FX의 공통 정규화 분모로 재사용하는 방향**이다. 이 값은 물리 Speed Limit가 아니며, Accepted Vehicle Builder Benchmark와 차량 의도 성능을 근거로 작성하는 Presentation/Performance reference다. 정확한 C++ 심볼명·구조체·노출 위치는 VCFX-P0-01에서 Freeze한다.

Camera FX 전용으로 모든 차량에 같은 글로벌 성능 상수를 새로 만드는 것은 금지하고, invalid/zero reference에서 `120 km/h` 같은 silent fallback을 사용하지 않는다. 해당 경우 Driving FX를 fail-safe로 비활성화하고 진단 가능한 상태로 남기는 계약을 P0-01에서 확정한다.

| 연출 계층 | 입력 신호 | 표현 목적 | P0 기본 범위 |
| --- | --- | --- | --- |
| Speed FOV | 차량 속도 | 고속 체감 강화 | 기존 구현 재기준화·곡선/범위 개선 검토 |
| Acceleration Kick | 종방향 가속도 | 급가속 시 질량감 | 제한된 후퇴 반응 |
| Brake Kick | 종방향 감속도 | 급제동 시 관성감 | 제한된 전진 반응 |
| Lateral Roll | signed 횡가속 + 실제 차체 Roll/RollRate/YawRate + 현재 자유시점 정렬 | 차체가 실제로 격하게 반응할 때만 코너링·슬라이드 질량감 전달, 측면 조준 가독성 보존 | 화면 방향에 맞춘 제한된 Roll 반응 |
| High-Speed Micro Motion | 속도·노면/차량 상태 | 고속 긴장감 | 매우 약한 진동, 조준 방해 금지 |
| Aim/Combat Attenuation | 카메라/전투 상태 | 전투 가독성 보존 | 연출 강도 자동 감쇠 |
| Special-State FX | 향후 Boost 등 명시 상태 | 특수 주행 강조 | P0 필수 아님 |

Motion Blur(모션 블러), Vignette(비네트), Chromatic Aberration(색수차) 같은 Post Process(후처리)는 P0 핵심 구현에 자동 포함하지 않는다. 시인성·조준·멀미 위험을 USER 검수한 뒤 필요할 때만 후속 옵션으로 판단한다.

---

## 2. 2026-09-27 Fresh 읽기 전용 기준선

### 2.1 현재 이미 존재하는 기능

- `UCFVehicleCameraComp`가 Tick에서 Aim → Camera Transform → Aim Trace 순으로 카메라를 갱신한다.
- `FCFVehicleCameraTuningConfig`에는 이미 `bUseSpeedBasedFOV`, `bUseSpeedBasedArmLength`, `SpeedForMaxBonusKmh`, `MaxSpeedFOVBonus`, `MaxSpeedArmLengthBonus`, `FOVInterpSpeed`, `ArmLengthInterpSpeed`가 존재한다.
- Source 구조체 기본값은 `BaseFOV=87`, `SpeedForMaxBonusKmh=120`, `MaxSpeedFOVBonus=6`, `MaxSpeedArmLengthBonus=45`이다. 이는 **C++ 기본값**이며 실제 저장 Product DataAsset 값과 동일하다고 아직 판정하지 않는다.
- `UCFVehicleCameraComp`는 `CurrentFOV`를 보간하고 `FollowCamera->SetFieldOfView(CurrentFOV)`로 실제 카메라에 적용한다.
- 차량 Root Pitch/Roll 직접 전달을 줄이는 `bIsolateCameraFromVehiclePitchRoll`과 Yaw 완충이 이미 있다.
- 현재 Source 검색에서 일반적인 `CameraLag` 기반 구현은 확인되지 않았다. 필요하더라도 기존 SpringArm/Camera ownership과 충돌하지 않는지 먼저 검수한다.

따라서 **“속도에 따라 FOV를 처음 추가한다”는 계획은 사용하지 않는다.** 이번 Feature는 기존 속도 기반 카메라를 재기준화하고 추가 주행 연출을 안전하게 합성하는 작업이다.

### 2.2 Gameplay(게임플레이) 소비 경계

Fresh Source 기준 `CFTargetCandidateSearch.cpp::BuildRuntimeSearchView`는 Target Search View를 구성할 때 현재 Presentation Camera를 다음 네 곳에서 직접 소비한다.

- `ViewOrigin` = `GetCurrentAimTraceStartLocation()` → 현재 `FollowCamera` 위치 우선
- `ViewDirection` = `GetCurrentAimDirection()` → 현재 `FollowCamera` Forward 우선
- `ViewUpDirection` = 현재 `FollowCamera` Up
- `VerticalFOVDeg` = `CameraRuntimeState.CurrentFOV` 또는 실제 `FollowCamera->FieldOfView`

또한 `CalculateNormalizedScreenDistance`는 Direction/Up/FOV를 모두 사용해 후보의 화면 거리와 안정화 순서를 계산한다. 따라서 Driving Presentation용 FOV뿐 아니라 향후 Camera Roll 또는 Pitch/Yaw Shake까지 그대로 `FollowCamera`에 합성하면 Target Selection 우선순위와 Aim Trace 방향에 부수효과가 생길 수 있다.

VCFX-P0-00에서 이 결합은 **Gameplay View와 Presentation View를 명시적으로 분리해야 하는 설계 경계**로 확정했다. 신규 Driving FX가 Targeting/Aim 의미를 암묵적으로 변경하는 것은 허용하지 않는다.

### 2.3 VCFX-P0-00 Fresh Rebaseline 결과 — 2026-09-28

#### A. 현재 Speed 입력 의미

- `UCFVehicleCameraComp::GetVehicleSpeedKmh()` → `ACFVehiclePawn::GetVehicleSpeed()` → `UCFVehicleDriveComp::GetCurrentSpeedKmh()` 경로다.
- 현재 `CurrentSpeedKmh`는 Actor Velocity의 `Size()`를 사용하므로 Z축 수직 속도까지 포함한 3D 속도다.
- Driving FX의 Speed Ratio는 점프/낙하가 고속 주행으로 해석되지 않도록 **월드 Planar(XY) 속도**를 기본 신호로 사용한다. Airborne은 기존 Camera Mode 의미로 별도 처리한다.

#### B. 차량별 Performance Reference 권위

- `FCFVehicleMovementConfig`는 Engine/Transmission/Brake/Steering/Wheel/Drag 등 물리 원인값을 차량별로 소유한다.
- 단일 authored `MaxSpeedKmh`/`TopSpeedKmh` runtime field는 Fresh Source에서 확인되지 않았다.
- Vehicle Builder Benchmark는 실제 `PeakSpeedKmh`, `Accel0To100Sec`, `Brake100ToIdleSec/Distance`, `TurningRadiusM`, `TurningAverageSpeedKmh` 등을 측정한다.
- Benchmark 결과를 runtime Camera가 직접 읽는 현재 계약은 없다.
- 따라서 Benchmark를 **reference 작성/검수 evidence**로 사용하고, Runtime은 VehicleData의 authored performance reference를 읽는 방향으로 분리한다.

#### C. 단일 ReferenceMaxSpeed 기반 정규화

P0-00에서는 가속/제동/횡가속 기준값을 차량마다 별도로 4세트 관리하지 않는다. 차량당 하나의 `ReferenceMaxSpeed` 성격 값을 공통 분모로 사용한다.

```text
ReferenceSpeedCmPerSec = ReferenceMaxSpeedKmh / 0.036
PlanarVelocity = WorldVelocity with Z=0
SpeedRatio = |PlanarVelocity| / ReferenceSpeedCmPerSec

PlanarAcceleration = (CurrentPlanarVelocity - PreviousPlanarVelocity) / DeltaTime
ForwardRatePerSec = Dot(PlanarAcceleration, VehicleForwardXY) / ReferenceSpeedCmPerSec
LateralRatePerSec = Dot(PlanarAcceleration, VehicleRightXY) / ReferenceSpeedCmPerSec

AccelerationRate = max(ForwardRatePerSec, 0)
BrakingRate = max(-ForwardRatePerSec, 0)
LateralRate = signed LateralRatePerSec
```

`ForwardRatePerSec=0.20`은 “현재 차량 기준 최고속도의 약 20%에 해당하는 속도를 1초에 얻는 변화율”이라는 의미다. 즉 모든 차량에 같은 `m/s²`를 강제하지 않으면서 같은 Percentage(백분율) 언어로 가속·제동·횡운동을 표현할 수 있다.

이 값들은 물리 상태를 바꾸지 않는 read-only Presentation 입력이다. Spawn/Teleport/큰 DeltaTime/분모 0/비정상 Vector는 필터링하고, 실제 FX 입력 범위와 Clamp는 P0-01에서 Freeze한다.

#### D. Gameplay View / Presentation View 분리 계약

P0 기본 방향은 다음과 같다.

```text
Gameplay View
- Driving FX 이전의 안정된 Aim Direction
- Presentation Roll/Shake를 제외한 안정된 Up
- Base + Camera Mode + Aim Profile 의미의 Gameplay FOV

Presentation View
- Speed FOV/Arm
- Acceleration/Braking Kick
- Lateral Roll
- 후속 High-Speed Micro Motion
- SpringArm Collision과 Collision View Assist를 거친 실제 화면
```

TargetSelect와 Aim의 **각도/FOV 의미**는 Gameplay View를 사용한다. 반면 Trace/LOS의 시작 위치는 실제 SpringArm 충돌로 카메라가 벽 앞으로 압축될 수 있으므로 **collision-resolved 실제 Camera Origin을 유지**하는 방향이 P0 기본안이다. 즉 화면의 실제 기하학적 위치 변화는 반영하되, Driving FX의 Roll/FOV/Shake가 조준 방향·타겟 우선순위를 변경하지 않도록 분리한다.

정확한 Runtime View 구조체/API와 어떤 기존 getter를 유지·교정할지는 P0-01에서 Freeze한다.

#### E. Presentation Style 소유권

- 현재 `UCFVehicleCameraData`가 Camera Presentation tuning의 기존 owner다.
- 현재 VehicleData → VehicleCameraData per-vehicle override 관계는 Fresh Source에서 확인되지 않았다.
- P0 기본 방향은 **VehicleData가 선택적인 Camera Profile/Data override를 가질 수 있게 하고**, Camera component의 기존 `VehicleCameraData`는 fallback으로 유지하는 것이다.
- 여러 차량이 Heavy/Sport 같은 동일 CameraData를 공유할 수도 있고, 특정 차량만 고유 Profile을 사용할 수도 있다.
- 정확한 property/API와 migration은 P0-01에서 Freeze한다.

#### F. Persisted Asset evidence

Fresh AssetDump managed dataset:

`adset_v1_5acb83e891e1f4b9ef2d733afbb71be4.bdd1b15768494d24cc66139c`

- `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`는 `CFVehicleCameraData`로 확인했다.
- Sedan/SUV/Defense SUV의 persisted Movement 값은 Throttle/Steering/Wheel/Mass 등에서 실제 차이를 보인다.
- 현재 `VehicleCamera.md`가 적은 과거 `/Data/Camera/DA_Cam_Default` 경로와 Fresh persisted 경로가 다르므로 Systems 문서에 stale path가 있다.
- `BP_CFVehiclePawn` fresh 단일 AssetDump는 preparation failure가 1회 발생했다. 반복 복구하지 않았으며 P0 설계는 해당 BP binding을 전제로 하지 않으므로 비차단 evidence limitation으로 유지한다.

### 2.4 VCFX-P0-00 Design Review 판정

```text
P0: 0
blocking P1: 0
P2: 1 non-blocking — VehicleCamera Systems 문서의 persisted DA path/일부 현재 Source 의미 stale
Evidence limitation: BP_CFVehiclePawn fresh binding dump NOT CONFIRMED / non-blocking
Verdict: TECHNICAL DESIGN PASS
Implementation: NOT STARTED
```

P0-00은 종료하며 exact next는 `VCFX-P0-01 Camera FX Contract + Data Freeze`다.

---

## 3. 소유권 및 구현 방향

### 3.1 C++ 책임

C++은 측정·합성·안전 제한을 담당한다.

- 차량 속도와 필요한 종방향/횡방향 운동 신호를 안정적으로 추출한다.
- Raw 절대값을 바로 FX 강도로 쓰지 않고, 차량별 authoritative 성능 기준에 대해 0.0~1.0 중심의 정규화 입력으로 변환한다.
- 정규화 입력은 필요 시 0~100%로 디버그/튜닝 UI에 표현하되 내부 계산은 단위가 명확한 Ratio로 유지한다.
- DeltaTime(델타 타임), 순간 노이즈, Teleport(텔레포트), Spawn(스폰), 초기화에 의한 잘못된 가속도 Spike(스파이크)를 방지한다.
- 여러 Camera FX가 동시에 적용될 때 하나의 최종 Camera Presentation 결과로 합성한다.
- FOV, 위치 Offset(오프셋), Roll, Shake 강도에 Clamp(제한)와 보간을 적용한다.
- Camera Mode(카메라 모드), Aim/Combat 상태에 따른 감쇠를 계산한다.
- 기존 Aim Direction(조준 방향), Aim Trace, SpringArm Collision, Target Selection 의미를 보호한다.
- Dedicated Server(전용 서버)의 기존 Camera Runtime Skip 계약을 보존한다.

### 3.2 DataAsset / Blueprint(블루프린트) 책임

튜닝은 가능한 한 기존 `UCFVehicleCameraData` 계열에서 관리한다.

- Normalized Speed Ratio(정규화 속도 비율) → FOV Curve(커브)
- Normalized Speed Ratio → Arm/Distance 보정
- Normalized Acceleration Ratio(정규화 가속 비율) → Rear Offset/Kick
- Normalized Braking Ratio(정규화 제동 비율) → Forward Offset/Kick
- Normalized Lateral Motion Ratio(정규화 횡운동 비율) → Roll
- High-Speed Motion 강도
- Aim/Combat 감쇠 배율
- 효과별 최대값과 보간 속도

**새 DataAsset 또는 새 Camera FX Component를 먼저 만든다는 결론은 내리지 않는다.** VCFX-P0-00에서 현재 `FCFVehicleCameraTuningConfig` 확장이 충분한지, 별도 하위 Config가 필요한지 검수한 뒤 최소 구조를 선택한다.

---

## 4. Pre-Implementation 핵심 검수 항목

### P1-1 — Presentation FOV와 Gameplay Target Search FOV 결합

현재 `FollowCamera->FieldOfView`가 화면 표현과 Target Search 양쪽에 사용된다.

다음 중 어떤 계약을 사용할지 확정한다.

1. 실제 보이는 FOV가 넓어지는 만큼 Target Search도 넓어지는 것이 의도된 Gameplay 규칙인지,
2. Target Search는 별도의 Gameplay View FOV를 유지하고 Presentation FOV만 시각적으로 넓혀야 하는지.

사용자 승인 없이 Target 획득 범위를 Camera Polish(카메라 폴리싱)의 부수효과로 변경하는 것은 금지한다.

### P1-2 — 운동 신호의 권위·차량별 정규화·필터링

Camera FX용 신호는 가능한 한 실제 차량 움직임에서 계산하되, **FX 강도 계산에는 차량별 성능 범위에 대한 정규화 비율을 기본 입력으로 사용한다.**

- 속도: 현재 월드 Planar(XY) 속도 → 차량별 `ReferenceMaxSpeed` 대비 `SpeedRatio`
- 가속: Planar 속도 변화의 진행축 성분 → `ReferenceMaxSpeed` 대비 초당 정규화 변화율 `AccelerationRate`
- 제동: Planar 속도 변화의 진행축 음수 성분 → `ReferenceMaxSpeed` 대비 초당 정규화 변화율 `BrakingRate`
- 코너링: Planar 속도 변화의 횡축 성분 → `ReferenceMaxSpeed` 대비 signed 초당 정규화 변화율 `LateralRate`
- Raw Steering Input(원시 조향 입력)만으로 Roll을 결정하는 방식은 기본안으로 사용하지 않는다. 미끄러짐·공중 상태·충돌 시 실제 차량 운동과 어긋날 수 있기 때문이다.
- 전 차량 공통 `SpeedForMaxBonusKmh=120` 같은 절대 임계값을 신규 FX 계약의 기준으로 고정하지 않는다.

정규화 분모는 P0-00 결과에 따라 VehicleData 계층의 차량별 authored `ReferenceMaxSpeed` 성격 값 exact1로 수렴한다. Vehicle Builder Benchmark의 accepted Peak Speed는 이를 작성·검수하는 evidence로 사용하되 runtime Camera가 Benchmark 결과를 직접 의존하지 않는다. 정확한 필드명/validation/migration은 P0-01에서 Freeze한다.

초기화/충돌/Teleport로 생기는 순간 Spike는 정규화 전에 필터링하고, 분모가 0이거나 비정상인 기준값은 fail-safe 처리해 NaN/Infinity가 Camera FX로 전달되지 않게 한다.

### P1-3 — 기존 Stabilization 및 SpringArm Collision 합성 순서

현재 카메라는 차량 Pitch/Roll 분리, Yaw Damping, SpringArm Collision, Collision View Assist를 이미 가진다.

신규 Kick/Roll/Shake가 이 기능 앞뒤 어디에서 합성되는지 명확히 한다. 다음 문제를 방지해야 한다.

- 벽 충돌 시 Camera Pop
- SpringArm 실제 거리와 내부 상태 불일치
- 조준 방향 흔들림
- 차량 차체 Roll과 Camera Roll의 이중 적용
- Collision View Assist와 Speed FOV의 과도한 중첩

### P1-4 — Combat/Aim 가독성과 USER Comfort

- Aim/Combat에서는 Roll/Shake/Kick을 감쇠할 수 있어야 한다.
- FOV 변화도 Reticle/Targeting 의미와 함께 검수한다.
- 화면 흔들림 강도는 명시적 상한을 가진다.
- 필요하면 Camera Motion Intensity(카메라 모션 강도)를 줄이거나 끄는 설정을 후속 검토한다.

---

## 5. 단계별 Gate(단계)

### VCFX-P0-00 — Fresh Rebaseline + Pre-Implementation Review

정확한 첫 단계다.

- canonical Checkout과 main_game/plan_repo dirty를 보호한 상태에서 Camera 관련 Source, DataAsset, Blueprint, Systems를 좁은 범위로 다시 읽는다.
- 실제 저장 `VehicleCameraData` 값과 차량별 Override 여부를 확인한다.
- [PASS] 차량별 성능 원인값은 VehicleData, 측정 evidence는 Vehicle Builder Benchmark가 소유함을 확인했다.
- [PASS] 런타임 단일 최고속도 reference 부재를 확인하고 VehicleData authored `ReferenceMaxSpeed` exact1 방향을 확정했다.
- [PASS] 전 차량 공통 절대 임계값이 남아 있는 현재 speed-based FOV/Arm 계약을 식별하고 Normalized Ratio migration을 다음 Gate에서 Freeze하기로 했다.
- `UpdateCameraTransform`의 속도/FOV/Arm/Collision 합성 순서를 정확히 추적한다.
- [PASS] TargetSelect가 실제 Camera Origin/Direction/Up/FOV를 소비함을 추적하고 Gameplay View / Presentation View 분리 방향을 확정했다.
- [PASS] 월드 Planar Velocity 차분을 사용해 종방향/횡방향 변화율을 계산하는 방향을 확정했다.
- [PASS] Design Review 결과 `P0 0 / blocking P1 0 / P2 1 non-blocking / TECHNICAL DESIGN PASS`.
- P0-00에서 구현은 시작하지 않았다.

### VCFX-P0-01 — Camera FX Contract + Data Freeze

P0-00의 정규화·Gameplay/Presentation 분리 원칙을 구현 가능한 exact contract로 동결한다. 이 단계에서는 Source/Product Asset을 수정하지 않는다.

#### A. C++ / DataAsset 구조 Freeze

새 Camera Component나 새 Camera DataAsset class는 만들지 않는다. 기존 owner를 최소 확장한다.

##### `UCFVehicleData` — 차량별 성능 기준과 Camera Profile 선택권

`UE/Source/CarFight_Re/Public/CFVehicleData.h`에 다음 exact2 top-level property를 추가한다.

1. `float ReferenceMaxSpeedKmh = 0.0f;`
   - Category: `CarFight|Vehicle Data|Performance`
   - ClampMin: `0.0`, Units: `km/h`
   - DisplayName: `기준 최고속도 km/h (ReferenceMaxSpeedKmh)`
   - ToolTip: `Camera Driving FX 등 차량 성능 비율 계산에 사용하는 차량별 기준 최고속도입니다. 물리 최고속도 제한값이 아니며 0은 미설정입니다. Vehicle Builder의 동일 saved Target/DefinitionHash에 결합된 accepted PeakSpeedKmh를 우선 근거로 작성합니다.`
   - 의미: Physics Speed Limit가 아니라 authored performance reference다.

2. `TObjectPtr<UCFVehicleCameraData> CameraPresentationDataOverride = nullptr;`
   - Category: `CarFight|Vehicle Data|Camera`
   - DisplayName: `카메라 표현 데이터 덮어쓰기 (CameraPresentationDataOverride)`
   - ToolTip: `이 차량의 카메라 표현 튜닝만 덮어쓸 VehicleCameraData입니다. 이 참조에서는 CameraTuningConfig만 사용하며 DefaultAimProfile은 변경하지 않습니다. 비어 있으면 VehicleCameraComp의 기존 VehicleCameraData 표현 튜닝을 사용합니다.`
   - `UCFVehicleCameraData`는 forward declaration으로 참조하며 VehicleData가 Camera 계산 책임을 소유하지 않는다.
   - **Gameplay 보호:** 이 per-vehicle override는 `CameraTuningConfig`만 선택한다. `DefaultAimProfile`의 Yaw/Pitch 제한·Aim gameplay semantics는 절대 override하지 않는다.

현재 VehicleData Authoring은 Registry/Recipe/Resolver/Snapshot 경로를 사용하므로 P0-02에서 위 exact2를 **현재 Vehicle Builder typed authoring field surface에 같이 온보딩**한다. Product VehicleData를 raw 직접 편집하는 우회 경로는 사용하지 않는다.

##### `FCFVehicleDrivingFXConfig` — 기존 CameraData 내부 신규 하위 Config exact1

`UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`에 `USTRUCT(BlueprintType) FCFVehicleDrivingFXConfig`를 추가하고, 기존 `FCFVehicleCameraTuningConfig`에 다음 property exact1을 추가한다.

`FCFVehicleDrivingFXConfig DrivingFXConfig;`

별도 DataAsset을 만들지 않고 기존 `UCFVehicleCameraData`가 Presentation tuning authority를 계속 소유한다.

`FCFVehicleDrivingFXConfig`의 Freeze 필드는 다음과 같다.

| 필드 | 기본값 | 역할 |
| --- | ---: | --- |
| `bUseNormalizedDrivingFX` | `false` | 기존 Product의 backward compatibility gate. False이면 현재 legacy speed FOV/Arm 경로 유지 |
| `SpeedFOVOffsetCurve` | 아래 Seed | `SpeedRatio 0..1` → FOV 추가값 deg |
| `SpeedArmOffsetCurve` | 아래 Seed | `SpeedRatio 0..1` → Arm 추가값 cm |
| `AccelerationArmKickCurve` | 아래 Seed | `AccelerationRate 0..1` → 뒤쪽 Arm Kick cm |
| `BrakingArmKickCurve` | 아래 Seed | `BrakingRate 0..1` → 앞쪽 Arm Kick cm |
| `LateralRollCurve` | 아래 Seed | signed `LateralRate -1..1` → Camera Roll deg |
| `MotionRateInterpSpeed` | `10.0` | Accel/Brake/Lateral normalized signal 보간 속도 |
| `LateralBodyMotionInterpSpeed` | `6.0` | 실제 차체 Motion intensity 0..1 보간 속도 |
| `LateralBodyRollAngleForFullIntensityDeg` | `7.5` | 절대 차체 Roll 각도가 이 값일 때 Roll-angle intensity=1 |
| `LateralBodyRollRateForFullIntensityDegPerSec` | `45.0` | 절대 local Roll 각속도가 이 값일 때 Roll-rate intensity=1 |
| `LateralBodyYawRateForFullIntensityDegPerSec` | `90.0` | 절대 local Yaw 각속도가 이 값일 때 Yaw-rate intensity=1 |
| `MaxSpeedFOVOffsetDeg` | `12.0` | Speed FOV offset 절대 안전 상한 |
| `MaxSpeedArmOffsetCm` | `120.0` | Speed Arm offset 절대 안전 상한 |
| `MaxAccelerationArmKickCm` | `35.0` | 가속 Rear Kick 절대 상한 |
| `MaxBrakingArmKickCm` | `35.0` | 제동 Forward Kick 절대 상한 |
| `MaxLateralRollDeg` | `3.0` | Roll 절대 상한 `±3°` |
| `MinPresentationFOVDeg` | `60.0` | 최종 화면 FOV 하한 |
| `MaxPresentationFOVDeg` | `120.0` | 최종 화면 FOV 상한 |
| `MaxPresentationArmLengthCm` | `900.0` | 최종 Arm 상한. 하한은 기존 `MinArmLength` 재사용 |
| `CombatSpeedFXScale` | `0.50` | Combat에서 Speed FOV/Arm 배율 |
| `CombatMotionFXScale` | `0.25` | Combat에서 Kick/Roll 배율 |
| `AimSpeedFXScale` | `0.50` | 명시적 Aim modifier에서 Speed FX 배율 |
| `AimMotionFXScale` | `0.20` | 명시적 Aim modifier에서 Kick/Roll 배율 |
| `AirborneSpeedFXScale` | `0.25` | Airborne에서 Speed FX 배율 |
| `AirborneMotionFXScale` | `0.00` | Airborne에서 Accel/Brake/Roll 비활성 |
| `ReverseSpeedFXScale` | `0.50` | Reverse에서 Speed FX 배율 |
| `ReverseMotionFXScale` | `0.35` | Reverse에서 Kick/Roll 배율 |

모든 Scale은 `0..1` Clamp 계약이다. `Destroyed` 또는 `Spectate`이면 Speed/Motion scale 모두 C++에서 `0`으로 강제한다.

##### Curve Seed Freeze

C++ 신규 구조체 기본 Seed는 다음이다. 이 값은 첫 구현/USER Review 출발점이며 P0-03 USER Feel 결과에 따라 조정 가능하지만, P0-02 구현 중 임의 변경하지 않는다.

```text
SpeedFOVOffsetCurve
(0.00, 0.0), (0.25, 0.0), (0.50, 1.0), (0.75, 3.0), (1.00, 6.0)

SpeedArmOffsetCurve
(0.00, 0.0), (0.25, 0.0), (0.50, 10.0), (0.75, 25.0), (1.00, 45.0)

AccelerationArmKickCurve
(0.00, 0.0), (0.05, 0.0), (0.15, 12.0), (0.30, 20.0), (1.00, 25.0)

BrakingArmKickCurve
(0.00, 0.0), (0.05, 0.0), (0.15, 10.0), (0.30, 18.0), (1.00, 22.0)

LateralRollCurve
(-1.00, -2.5), (-0.25, -1.5), (-0.10, -0.75), (-0.03, 0.0),
(0.00, 0.0),
(0.03, 0.0), (0.10, 0.75), (0.25, 1.5), (1.00, 2.5)
```

`LateralRate > 0`은 차량 Right(XY) 방향의 횡가속 성분이다. Curve가 signed input/output을 직접 소유하므로 좌우 기본 반응 방향을 별도 hard-coded 부호로 숨기지 않는다.

##### P0-03 USER Feel Contract Correction — Body Motion + Free-Look Alignment

USER review에서 저속 완만한 회전 시 실제 차체는 크게 흔들리지 않는데 Camera Roll만 빠르게 반응해 오프로드 주행처럼 느껴지는 문제가 확인됐다. 또한 CarFight는 일반 주행 게임과 달리 조준을 위해 시점을 차량 전방에서 측면까지 자유롭게 돌릴 수 있으므로 차량 기준 Lateral Roll을 화면에 그대로 적용하면 자유 조준 방향과 Presentation 방향이 불일치할 수 있다.

따라서 `LateralRate`의 signed 방향/기본 요구량 계약은 유지하되 최종 Presentation Roll 강도와 화면 방향은 다음 modulation을 추가한다.

```text
BodyRollDeg      = NormalizeAxis(VehicleMesh.WorldRotation.Roll)
BodyAngularWorld = VehicleMesh.GetPhysicsAngularVelocityInDegrees()
BodyAngularLocal = VehicleMeshTransform.InverseTransformVectorNoScale(BodyAngularWorld)
BodyRollRate     = BodyAngularLocal.X
BodyYawRate      = BodyAngularLocal.Z

RollAngleIntensity = Clamp(Abs(BodyRollDeg)  / 7.5, 0, 1)
RollRateIntensity  = Clamp(Abs(BodyRollRate) / 45.0, 0, 1)
YawRateIntensity   = Clamp(Abs(BodyYawRate)  / 90.0, 0, 1)

BodyMotionIntensityRaw = Max(RollAngleIntensity, RollRateIntensity, YawRateIntensity)
BodyMotionIntensity    = FInterpTo(previous, BodyMotionIntensityRaw, DeltaTime, 6.0)

VehicleForwardXY = horizontal direction of BaseVehicleAimRotation
ViewForwardXY    = horizontal direction of WorldAimRotation after free-look Aim
ViewAlignment    = Clamp(Dot(Normalize(VehicleForwardXY), Normalize(ViewForwardXY)), -1, 1)

BaseLateralRollDeg = Clamp(LateralRollCurve(LateralRate), -MaxLateralRollDeg, +MaxLateralRollDeg)
FinalLateralRollDeg = BaseLateralRollDeg
                    * BodyMotionIntensity
                    * ViewAlignment
                    * ResolvedMotionFXScale
```

의미는 다음과 같다.

- 차체가 평온하면 `BodyMotionIntensity≈0`이므로 횡가속만으로 카메라가 크게 기울지 않는다.
- 실제 Roll/Yaw motion이 격해질수록 `BodyMotionIntensity→1`로 접근해 authored Roll headroom을 사용할 수 있다.
- 차량 정면 시점은 `ViewAlignment≈+1`로 정상 적용한다.
- 차량 측면 ±90° 자유시점은 `ViewAlignment≈0`이므로 Lateral Roll을 억제해 조준 가독성을 보존한다.
- 후면 시점이 향후 허용되면 `ViewAlignment<0`이므로 화면 기준 Roll 방향을 자동 반전한다.
- 차체 Roll 자체를 Camera Rotation으로 복사하지 않는다. `bIsolateCameraFromVehiclePitchRoll=true` 안정화 철학을 유지하고 차체 motion은 강도 신호로만 사용한다.
- `bAimPresentation`은 자유시점 각도로 추론하지 않는다. 명시적 Aim/Combat producer가 공급될 때 사용할 기존 별도 상태 감쇠 계약으로 유지한다.
- Vehicle Mesh/각속도/View 입력이 invalid이면 해당 Lateral Presentation modulation을 exact0으로 fail-safe한다.

#### B. Camera Presentation Resolution Freeze

`UCFVehicleCameraComp`에 내부 `ResolveCameraPresentationData()` 책임을 추가한다. 이 resolver는 **Presentation tuning 전용**이며 `DefaultAimProfile`을 resolve하지 않는다.

```text
Presentation Tuning Resolution
1. VehiclePawn->VehicleData->CameraPresentationDataOverride가 유효
   → 해당 asset의 CameraTuningConfig만 사용
2. 아니면 Component.VehicleCameraData가 유효
   → 기존 Component CameraTuningConfig 사용
3. 둘 다 없음
   → C++ FCFVehicleCameraTuningConfig 기본값 사용

Aim Profile Resolution — 기존 Gameplay 의미 보존
1. bUseAimProfileOverride == true
   → 현재 explicit AimProfileOverride
2. 아니면 Component.VehicleCameraData가 유효
   → Component.VehicleCameraData->DefaultAimProfile
3. 아니면
   → C++ FCFVehicleCameraAimProfile 기본값
```

Runtime Content Apply로 `VehiclePawn->VehicleData`가 교체될 수 있으므로 Presentation override를 BeginPlay에 영구 캐시하지 않는다. 반대로 `BuildResolvedAimProfile()`은 **per-vehicle Presentation override를 보지 않으며**, 현재 Component CameraData + explicit AimProfileOverride의 기존 Gameplay owner 관계를 유지한다.

이 분리로 Heavy/Sport 등 차량별 Camera Presentation을 바꿔도 Yaw/Pitch 조준 제한, rear-view 허용 등 Aim Gameplay가 부수적으로 달라지지 않는다.

#### C. Normalized Motion Sampling / Math Freeze

물리 상태는 read-only로 읽으며 Camera FX 때문에 Vehicle Physics 값을 수정하지 않는다.

```text
ReferenceSpeedCmPerSec = ReferenceMaxSpeedKmh / 0.036
CurrentPlanarVelocity = FVector(WorldVelocity.X, WorldVelocity.Y, 0)
CurrentPlanarSpeed = |CurrentPlanarVelocity|
SpeedRatio = Clamp(CurrentPlanarSpeed / ReferenceSpeedCmPerSec, 0, 1)

VehicleForwardXY = Normalize(FVector(Forward.X, Forward.Y, 0))
VehicleRightXY   = Normalize(FVector(Right.X, Right.Y, 0))

CurrentForwardSpeedAbs  = Abs(Dot(CurrentPlanarVelocity, VehicleForwardXY))
PreviousForwardSpeedAbs = cached value computed at the previous valid sample with that frame's ForwardXY
LongitudinalSpeedRate   = (CurrentForwardSpeedAbs - PreviousForwardSpeedAbs) / DeltaTime

PlanarAcceleration = (CurrentPlanarVelocity - PreviousPlanarVelocity) / DeltaTime
LateralAcceleration = Dot(PlanarAcceleration, VehicleRightXY)

AccelerationRateRaw = Max( LongitudinalSpeedRate / ReferenceSpeedCmPerSec, 0)
BrakingRateRaw      = Max(-LongitudinalSpeedRate / ReferenceSpeedCmPerSec, 0)
LateralRateRaw      = LateralAcceleration / ReferenceSpeedCmPerSec

AccelerationRate = Clamp(AccelerationRateRaw, 0, 1)
BrakingRate      = Clamp(BrakingRateRaw, 0, 1)
LateralRate      = Clamp(LateralRateRaw, -1, 1)
```

전진/후진 모두 `Abs(ForwardSpeed)` 변화량으로 가속/제동을 분류하므로 후진 가속을 제동으로 잘못 해석하지 않는다. `PreviousForwardSpeedAbs`는 이전 Velocity를 **현재 축으로 다시 투영하지 않고 이전 valid frame에서 계산한 scalar를 캐시**한다. 따라서 차량이 빠르게 Yaw 회전할 때 축 변화만으로 가짜 Accel/Brake가 생기는 위험을 줄인다. Lateral은 실제 XY velocity 변화에서 계산하고 Steering Input 자체를 FX 기준으로 쓰지 않는다.

##### Sampling Safety / Reset Freeze

다음은 Artistic Data가 아니라 C++ safety contract다.

```text
MinValidDeltaTimeSec = 0.0001
MaxValidDeltaTimeSec = 0.10
MinTeleportResetDistanceCm = 2000
TeleportDistanceMultiplier = 3.0
MaxNormalizedMotionRate = 1.0
```

Safety failure를 두 종류로 분리한다.

**Reference/Input invalid — 전체 normalized Driving FX exact0**

- `ReferenceMaxSpeedKmh`가 non-finite 또는 `<= 0`
- current Velocity/Location/Forward/Right가 non-finite 또는 XY axis 정규화 불가

이 경우 `SpeedRatio / AccelerationRate / BrakingRate / LateralRate = 0`이며 legacy global speed fallback을 사용하지 않는다.

**Motion history invalid/reset — Speed는 허용, Kick/Roll만 exact0**

- 최초 sample
- `DeltaTime < MinValidDeltaTimeSec` 또는 `> MaxValidDeltaTimeSec`
- Teleport reset 조건 충족

이 경우 current reference/velocity가 valid하면 `SpeedRatio`는 현재 frame에서 계산해 Speed FOV/Arm을 유지할 수 있다. 다만 motion history를 현재 sample로 재초기화하고 `AccelerationRate / BrakingRate / LateralRate = 0`으로 하여 Kick/Roll spike를 차단한다.

모든 경로에서 NaN/Infinity를 Curve/Camera Transform으로 전달하지 않는다.

Teleport reset은 다음 조건이다.

```text
AllowedPlanarDistance = Max(
    MinTeleportResetDistanceCm,
    ReferenceSpeedCmPerSec * DeltaTime * TeleportDistanceMultiplier)

ActualPlanarLocationDelta > AllowedPlanarDistance
→ motion history reset / Motion FX = 0 for that frame
```

`SpeedRatio`는 valid reference와 current planar velocity가 있으면 별도로 계산할 수 있으나, reset frame의 Kick/Roll은 반드시 0이다. Accel/Brake/Lateral target은 먼저 `[-1,1]` safety clamp 후 `MotionRateInterpSpeed`로 보간한다.

#### D. Gameplay View / Presentation View Runtime Freeze

`CFVehicleCameraTypes.h`에 `FCFVehicleCameraGameplayView`를 신규 추가한다.

필드 exact5:

```text
bool bValid
FVector ViewOrigin
FVector ViewDirection
FVector ViewUpDirection
float VerticalFOVDeg
```

의미는 다음과 같다.

- `ViewOrigin`: SpringArm Collision이 해결된 **실제 현재 Camera 위치**. Driving Arm/Kick으로 실제 화면 위치가 움직였다면 그 위치를 반영한다.
- `ViewDirection`: **기존 Camera Transform 계산이 끝난 뒤 신규 Driving Presentation Roll을 적용하기 직전 FollowCamera의 실제 pre-Driving rotation**에서 얻은 Forward. 이 방식으로 기존 BP/Camera 상대 회전이 있더라도 legacy 방향을 보존한다.
- `ViewUpDirection`: 같은 pre-Driving Camera rotation에서 얻은 Up. 신규 Lateral Roll만 제외하며 기존 baseline relative rotation은 보존한다.
- `VerticalFOVDeg`: Base + 기존 Camera Mode + AimProfile + Collision View Assist까지 반영하되 **신규 Driving Speed FOV는 제외한 보간된 Gameplay FOV**.

`UCFVehicleCameraComp`는 normalized Driving FX가 활성일 때 `CurrentGameplayFOV`를 Presentation `CurrentFOV`와 별도로 보간하고 `GetGameplayView()`를 BlueprintPure로 제공한다. `bUseNormalizedDrivingFX=false` legacy path에서는 `CurrentGameplayFOV = CurrentFOV`로 동일하게 유지해 기존 Target Search FOV 의미를 exact 보존한다.

기존 API 호환 계약:

- `GetCurrentAimDirection()` 시그니처는 유지하되 normalized Driving path에서는 `GameplayView.ViewDirection`을 반환한다. legacy path에서도 GameplayView를 **pre-Driving actual Camera rotation**에서 캡처하므로 현재 FollowCamera Forward와 동일 의미를 유지한다.
- `GetCurrentAimTraceStartLocation()`은 실제 collision-resolved Camera origin을 계속 반환한다.
- `CameraRuntimeState.CurrentFOV`는 기존 호환을 위해 **실제 Presentation FOV** 의미를 유지한다.
- RuntimeState에는 `CurrentGameplayFOV`, normalized input/output 진단 필드를 additive로 추가한다.

Consumer 계약:

```text
Aim Trace
- Origin    = GameplayView.ViewOrigin (실제 Camera 위치)
- Direction = GameplayView.ViewDirection

TargetSelect
- ViewOrigin       = GameplayView.ViewOrigin
- ViewDirection    = GameplayView.ViewDirection
- ViewUpDirection  = GameplayView.ViewUpDirection
- VerticalFOVDeg   = GameplayView.VerticalFOVDeg

HUD / VehicleVisual existing GetCurrentAimDirection consumers
- 안정된 Gameplay direction을 받는다.
```

따라서 신규 Speed FOV, Kick, Roll이 Target Search 각도/FOV 우선순위나 Weapon Aim 방향을 암묵적으로 변경하지 않는다. 실제 Camera 위치 이동과 SpringArm Collision에 따른 Trace origin 변화만 기하학적으로 유지한다.

#### E. Mode / Aim Attenuation Freeze

현재 `FCFVehicleCameraModeFlags`는 Combat/Reverse/Airborne/Destroyed/Spectate modifier를 이미 소유하지만 현재 Product Source에서 명시적 Aim-active authority는 확인되지 않았다.

따라서 `FCFVehicleCameraModeFlags`에 다음 additive flag를 추가한다.

`bool bAimPresentation = false;`

DisplayName: `조준 카메라 연출 감쇠 (bAimPresentation)`
ToolTip: `사용자가 명시적인 조준/정밀 시점에 들어갔을 때 Driving Camera FX를 감쇠할 외부 modifier입니다. AimProfileOverride 존재 여부에서 자동 추정하지 않습니다.`

현재 authoritative producer가 없으므로 기본값 False를 유지한다. `AimProfileOverride`, Reticle 상태, Weapon 장착 여부로 이 값을 추정하는 것은 금지한다.

Speed와 Motion 감쇠는 별도로 계산하며, 여러 modifier가 동시에 True면 **활성 값 중 가장 작은 scale**을 사용한다. 곱셈으로 과도하게 중복 감쇠하지 않는다.

```text
ResolvedSpeedFXScale  = Min(active Speed scales, default 1.0)
ResolvedMotionFXScale = Min(active Motion scales, default 1.0)

Destroyed 또는 Spectate
→ 둘 다 0
```

Airborne에서는 Motion scale 0이므로 점프/착지의 velocity spike가 Accel/Brake/Roll 연출로 중복 표현되지 않는다. 기존 `AirborneFOVOffset`은 Gameplay Camera Mode 의미로 그대로 유지한다.

#### F. Final Composition Order Freeze

한 Tick의 순서를 다음으로 고정한다.

```text
1. Resolve VehicleData / resolved VehicleCameraData
2. Resolve AimProfile / Camera Mode flags
3. Sample planar vehicle motion + validate/reset history
4. Build Base Gameplay arm/FOV
   = Base + 기존 Mode + AimProfile
5. Evaluate normalized Driving FX curves
6. Resolve Speed/Motion attenuation
7. Build Presentation arm
   = Gameplay arm
   + SpeedArmOffset
   + AccelerationRearKick
   - BrakingForwardKick
8. Clamp Presentation arm
   = [기존 MinArmLength, MaxPresentationArmLengthCm]
9. 기존 SpringArm Collision / Collision View Assist 계산
10. Build Gameplay desired FOV
    = Base + 기존 Mode + AimProfile + CollisionFOVAssist
11. Build Presentation desired FOV
    = Gameplay desired FOV + SpeedFOVOffset
12. Clamp Presentation FOV
    = [MinPresentationFOVDeg, MaxPresentationFOVDeg]
13. GameplayFOV와 PresentationFOV를 각각 FOVInterpSpeed로 보간
14. 기존 Yaw Damping + Clamp Aim으로 자유시점이 포함된 `WorldAimRotation` 계산
15. Vehicle Mesh 실제 Roll/RollRate/YawRate로 `BodyMotionIntensity` 0..1 계산/보간
16. 차량 Yaw-only 전방과 `WorldAimRotation` 수평 전방 dot으로 signed `ViewAlignment` -1..1 계산
17. `BaseLateralRoll × BodyMotionIntensity × ViewAlignment × ResolvedMotionFXScale`로 최종 Lateral Presentation Roll 계산
18. SpringArm을 기존 Gameplay rotation + Presentation arm으로 해결
19. FollowCamera의 신규 Driving Roll 적용 전 actual rotation에서 Gameplay Direction/Up을 캡처
20. collision-resolved Camera origin + Gameplay FOV를 GameplayView에 기록
21. FollowCamera에 Presentation FOV 적용
22. FollowCamera의 초기 Relative Rotation을 기준으로 최종 Lateral Roll만 additive 적용
23. Gameplay View 기반 Aim Trace 수행
```

P0 Minimum에서는 Presentation Pitch/Yaw Shake를 추가하지 않는다. High-Speed Micro Motion은 P0-02 핵심 4효과가 안정된 뒤 후속 범위로 남긴다.

Collision View Assist는 기존 Geometry/Visibility behavior로 보존한다. normalized path의 Gameplay FOV에는 기존 Collision FOV Assist를 포함한다. Presentation Arm/Kick이 벽 근처 실제 Camera origin과 compression에 영향을 주면 그 실제 geometry 변화는 허용된 예외이며, Driving Speed FOV와 Roll 자체만 Gameplay Direction/Up/FOV에서 제거한다.

FollowCamera Roll은 초기화 시 `BaseFollowCameraRelativeRotation`을 한 번 저장하고 매 frame `Base + PresentationRoll`로 재구성한다. 누적 `AddLocalRotation`을 사용하지 않아 drift를 방지하며 FX disable 시 baseline rotation으로 복귀한다.

#### G. Curve Evaluation / Output Clamp Freeze

- Curve input은 위 normalized 값만 사용한다.
- Curve output이 non-finite이면 해당 효과만 0 처리한다.
- Speed FOV output: `±MaxSpeedFOVOffsetDeg` clamp 후 Speed scale 적용.
- Speed Arm output: `±MaxSpeedArmOffsetCm` clamp 후 Speed scale 적용.
- Accel/Brake curve output: 각각 `0..Max*KickCm` clamp 후 Motion scale 적용.
- Lateral Roll curve output: 먼저 `±MaxLateralRollDeg`로 기본 요구량을 clamp한 뒤 `BodyMotionIntensity(0..1) × signed ViewAlignment(-1..1) × Motion scale(0..1)`을 적용하고 최종값을 다시 `±MaxLateralRollDeg`로 안전 clamp한다.
- 최종 FOV/Arm은 개별 effect clamp 후 다시 절대 Presentation clamp를 적용한다.

#### H. Legacy / Product Migration Freeze

기존 fields:

```text
bUseSpeedBasedFOV
bUseSpeedBasedArmLength
SpeedForMaxBonusKmh
MaxSpeedFOVBonus
MaxSpeedArmLengthBonus
```

은 serialized backward compatibility를 위해 P0-02에서 삭제/rename하지 않는다.

- `DrivingFXConfig.bUseNormalizedDrivingFX == false`
  → **현재 legacy path exact 동작 유지**.
- `true`
  → legacy `SpeedForMaxBonusKmh`와 legacy speed bonus 계산을 사용하지 않고 normalized path만 사용.
- normalized path에서 `ReferenceMaxSpeedKmh <= 0` 또는 invalid
  → Driving FX exact0 / legacy global speed fallback 금지.

`DA_Cam_Default` migration은 다음 순서를 지킨다.

```text
M1. Source/Automation에 additive contract 먼저 구현. 신규 bUseNormalizedDrivingFX 기본 False.
M2. 현재 DA_Cam_Default를 resolve하는 in-scope production VehicleData 각각의 ReferenceMaxSpeedKmh를 확보.
    - 동일 saved Target path + DefinitionHash에 결합된 latest accepted Vehicle Builder PeakSpeedKmh 우선.
    - evidence가 없으면 benchmark를 실행/승인해 근거를 만든 뒤 작성.
    - 숫자를 Engine RPM/기어비에서 추정하지 않음.
M3. ReferenceMaxSpeedKmh와 CameraPresentationDataOverride를 Vehicle Builder typed Authoring 경로로 Preview/Review/Apply.
M4. 모든 in-scope VehicleData의 valid reference를 fresh persisted evidence로 확인한 뒤에만 DA_Cam_Default normalized path를 활성화.
M5. DA_Cam_Default의 현재 serialized legacy MaxSpeedFOVBonus/MaxSpeedArmLengthBonus를 읽어 normalized Speed curve endpoint를 보존.
M6. Fresh AssetDump + 공식 UE 5.8 Build + Focused Automation + PIE Technical Validation.
```

기존 legacy endpoint를 normalized Curve에 옮길 때 shape는 다음 비율을 사용한다.

```text
Speed FOV curve output multiplier by LegacyMaxSpeedFOVBonus:
0.00→0.0, 0.25→0.0, 0.50→0.1667, 0.75→0.5, 1.00→1.0

Speed Arm curve output multiplier by LegacyMaxSpeedArmLengthBonus:
0.00→0.0, 0.25→0.0, 0.50→0.2222, 0.75→0.5556, 1.00→1.0
```

따라서 현재 persisted endpoint가 Source default와 다르더라도 migration이 그 값을 덮어쓰지 않는다. `SpeedForMaxBonusKmh=120`은 per-vehicle reference로 대체되므로 normalized data로 복사하지 않는다.

한 차량이라도 in-scope인데 valid reference가 없으면 shared `DA_Cam_Default`의 normalized 활성화를 HOLD한다. 누락 차량을 조용히 zero-FX로 바꾼 채 shared asset을 승격하지 않는다.

#### I. C++ / DataAsset / Blueprint Responsibility Freeze

**C++**
- Profile resolution, planar sampling, normalized math, reset/fail-safe, interpolation, clamp, composition, Gameplay View, Target/Aim 보호를 소유한다.
- Dedicated Server 기존 skip 계약을 유지한다.
- Camera FX 때문에 Physics/Replication 값을 쓰지 않는다.

**VehicleData / Vehicle Builder**
- `ReferenceMaxSpeedKmh`, `CameraPresentationDataOverride`의 authored value와 Preview/Review/Apply를 소유한다.
- Benchmark는 reference evidence이며 runtime Camera dependency가 아니다.

**VehicleCameraData**
- Driving curve, 강도, clamp, mode attenuation과 Presentation style을 소유한다.

**Blueprint**
- 기존 Camera component reference와 fallback CameraData assignment를 유지한다.
- Tick 기반 수학/정규화/Curve 합성을 추가하지 않는다.
- 외부 시스템이 실제 명시적 Aim presentation 상태를 갖게 될 때 `CameraModeFlags.bAimPresentation`을 전달할 수 있다.

#### J. P0-01 Contract Correction 기록

v0.2.0 candidate를 실제 Source 소비 경계와 역검수하면서 blocking P1 후보 exact4를 발견했고, v0.2.1에서 모두 계약 교정했다.

1. `P1-C1`: per-vehicle `VehicleCameraDataOverride`가 `DefaultAimProfile`까지 교체하면 Presentation 선택이 Yaw/Pitch Aim Gameplay를 변경할 수 있음.
   - Correction: field를 `CameraPresentationDataOverride`로 좁히고 `CameraTuningConfig`만 소비. Aim Profile owner는 기존 Component CameraData + explicit AimProfileOverride로 보존.
2. `P1-C2`: 이전 Velocity를 현재 ForwardXY로 다시 투영하면 Yaw 변화만으로 가짜 Longitudinal accel/brake가 생길 수 있음.
   - Correction: `PreviousForwardSpeedAbs` scalar를 이전 valid sample에서 계산·캐시.
3. `P1-C3`: invalid reference와 단순 first-frame/Teleport reset의 SpeedRatio 처리 의미가 혼재함.
   - Correction: reference/input invalid는 전체 FX exact0, motion-history reset은 SpeedRatio 허용 + Kick/Roll exact0으로 분리.
4. `P1-C4`: Gameplay direction을 새 WorldAimRotation에서 재구성하면 기존 BP Camera relative rotation이 존재할 때 legacy direction을 바꿀 수 있음.
   - Correction: 신규 Driving Roll 적용 직전 실제 FollowCamera rotation에서 Direction/Up을 캡처해 legacy 의미를 보존.

교정 후 남은 비차단 항목:

- `P2-1`: `VehicleCamera.md`의 persisted Camera DataAsset 경로 등 일부 Current 문서가 Fresh Asset 경로와 stale하다. 구현 후 Systems promotion에서 교정한다.
- `P2-2`: 현재 Product Source에는 `bAimPresentation`을 공급할 authoritative Aim-active state가 없다. `AimProfileOverride` 등에서 추정하지 않고 default False를 유지한다. 실제 Aim mode가 생길 때 명시적으로 연결한다.
- `P2-3`: `CFVehicleData.h` 및 Vehicle Authoring 관련 파일은 다른 accepted work의 기존 dirty가 존재한다. P0-02 착수 시 fresh diff를 읽고 exact additive merge만 수행해야 하며 기존 dirty를 정리/되돌리지 않는다.

P0 또는 blocking P1이 발견되면 P0-02 구현은 HOLD한다.

#### K. VCFX-P0-01 Design Re-review — Final

v0.2.1 교정본을 P0-00 baseline과 현재 Source 소비 경계에 다시 대조했다.

| Review 축 | 판정 | 근거 |
| --- | --- | --- |
| Data authority / 성능 기준 | PASS | VehicleData `ReferenceMaxSpeedKmh` exact1, accepted Benchmark는 authoring evidence, global performance fallback 금지 |
| C++ / DataAsset / BP 책임 | PASS | 계산·안전은 C++, style은 VehicleCameraData, authored selection은 VehicleData/Builder, BP Tick 계산 없음 |
| Camera Presentation / Aim Gameplay 분리 | PASS | `CameraPresentationDataOverride`는 `CameraTuningConfig`만 소비하고 DefaultAimProfile owner를 변경하지 않음 |
| Normalized math / Reverse | PASS | Planar speed + cached previous forward-speed magnitude + signed lateral acceleration로 전후진 오분류/axis reprojection 위험 교정 |
| Spawn / Hitch / Teleport / invalid input | PASS | reference/input invalid와 motion-history reset 의미 분리, NaN/Infinity fail-safe, Kick/Roll reset 계약 명시 |
| Gameplay View / Presentation View | PASS | actual collision-resolved origin 유지, pre-Driving actual Camera rotation의 Direction/Up + GameplayFOV 사용 |
| Legacy backward compatibility | PASS | `bUseNormalizedDrivingFX=false`에서 legacy speed FOV/Arm과 CurrentFOV 의미 유지, 기존 serialized field 삭제/rename 0 |
| Mode attenuation | PASS with P2 evidence note | Combat/Aim/Airborne/Reverse scale 계약은 명시됐으나 실제 Product producer는 P0-02에서 fresh 증거 필요 |
| Product migration atomicity | PASS | 모든 in-scope VehicleData valid reference persisted 전 shared DA_Cam_Default normalized activation HOLD |
| Physics / Replication / Dedicated Server | PASS | movement read-only, replicated physics mutation 0, 기존 Dedicated Server camera skip 유지 |
| Dirty protection | PASS | P0-01 Source/Product Asset mutation 0. P0-02에서 existing dirty fresh merge 필수 |

최종 남은 비차단 항목은 exact3이다.

1. `P2-1` VehicleCamera Systems 문서 stale path/일부 Source 의미 — 구현 후 Current Systems 동기화에서 교정.
2. `P2-2` C++ Source에서 `SetCameraModeFlags()` Product caller가 확인되지 않고 명시적 Aim-active authority도 없음. 이전 BP fresh dump preparation failure 때문에 Blueprint producer도 미확정. P0-02에서는 producer evidence를 확정하기 전 Combat/Airborne/Aim 실제 Runtime attenuation PASS를 주장하지 않는다.
3. `P2-3` `CFVehicleData.h`와 Vehicle Authoring 관련 기존 dirty가 존재. P0-02는 fresh diff 후 additive exact merge만 허용.

```text
VCFX-P0-01 FINAL DESIGN RE-REVIEW
P0: 0
blocking P1: 0
P2: 3 non-blocking
Candidate blocking P1: 4 found / 4 corrected
Verdict: CONTRACT FROZEN / TECHNICAL DESIGN PASS
Source implementation: NOT STARTED
Product Asset mutation: 0
Build / Automation / PIE: NOT RUN — design-only gate
Exact next: VCFX-P0-02 Minimum Implementation + Technical Validation
```

이 판정으로 구현 Gate가 열리지만, P0-01 세션에서는 구현을 시작하지 않는다.

### VCFX-P0-02 — Minimum Implementation + Technical Validation

우선 다음 4개를 핵심 구현 대상으로 한다.

1. Normalized Speed Ratio → 기존 FOV/Arm 개선
2. Normalized Acceleration Ratio → Rear Kick
3. Normalized Braking Ratio → Forward Kick
4. Normalized Lateral Motion Ratio → Camera Roll

Road/High-Speed Shake는 위 4개가 안정된 뒤 작은 범위로 추가한다.

검증 범위:

- 서로 다른 최고속도/가속 성능을 가진 차량에서 동일한 성능 사용 비율(예: 25%/50%/75%/100%)이 올바르게 정규화되는지
- 같은 75% SpeedRatio라도 차량별 CameraData Curve/강도 튜닝에 따라 의도한 개성이 유지되는지
- 초기화/정지/저속/고속에서 FOV와 Offset이 유한하고 제한 범위 내인지
- 급가속→정속→급제동에서 Pop 없이 복귀하는지
- 좌/우 코너링 Roll 대칭성
- Aim/Combat 감쇠
- SpringArm Collision 상호작용
- Target Search FOV가 P0-01 계약과 일치하는지
- Dedicated Server 기존 Skip 계약
- 공식 UE 5.8 Source Editor Build + Focused Automation

### VCFX-P0-03 — USER Driving / Combat Feel Review

대표 주행에서 USER가 다음을 확인한다.

- 저속에서 카메라가 불필요하게 움직이지 않는가
- 고속에서 속도 체감이 분명히 좋아졌는가
- 급가속/급제동에서 차량 질량감이 느껴지는가
- 코너링 Roll이 과장되거나 멀미를 유발하지 않는가
- 조준 중 화면이 흔들려 전투를 방해하지 않는가
- 벽 근처 SpringArm Collision에서 Camera Pop이 없는가
- 장시간 플레이 피로감이 과하지 않은가

RuntimeRead 등으로 기술 상태를 확인할 수 있어도 **“멋있다 / 속도감이 좋다 / 멀미가 없다 / 조준감이 좋다”는 USER Acceptance를 AI가 대신하지 않는다.**

### VCFX-P0-04 — Current System Promotion + Closure

- 최종 구현을 `Document/Systems/Vehicles/VehicleCamera.md`에 Current 계약으로 승격한다.
- FeatureQueue의 완료 owner를 동기화한다.
- UDS Work를 immutable successor로 closure한다.
- 대표 Plan을 Historical로 전환한다.
- 사용자/타 Work dirty를 건드리지 않는다.

---

## 6. 수용 기준

- 차량 Physics(물리)의 속도·가속·조향 값을 Camera 연출을 위해 변경하지 않는다.
- 차량마다 성능 한계가 다른데도 모든 차량에 동일한 절대 속도/가속/제동 임계값을 Camera FX 기준으로 강제하지 않는다.
- Camera FX 입력 판단은 차량별 authoritative 성능 기준 대비 Normalized Ratio(0.0~1.0 / 0~100%)를 우선 사용한다.
- 최종 FOV/Offset/Roll 등 화면 출력에는 차량별 튜닝과 별개로 안전한 절대 Clamp를 유지한다.
- 기존 Aim Direction / Aim Trace / Reticle / TargetSelect 동작을 부수적으로 변경하지 않는다.
- Presentation FOV가 Gameplay 의미를 바꾸는 경우 반드시 명시 계약과 검증이 있어야 한다.
- 모든 Camera FX는 보간과 상한을 갖고 Spawn/Teleport/충돌로 무한값·대형 Spike를 만들지 않는다.
- 기존 SpringArm Collision, Collision View Assist, Pitch/Roll Isolation, Yaw Damping을 임의 폐기하지 않는다.
- 효과별 강도를 DataAsset/Blueprint에서 튜닝할 수 있도록 하되 계산 책임을 Blueprint Tick에 분산하지 않는다.
- Camera Shake/후처리는 조준성과 USER Comfort를 해치지 않는 범위에서만 승인한다.
- 구현 전 VCFX-P0-00/01 설계 Gate를 통과한다.

---

## 7. 범위 보호

이번 Feature에서 기본적으로 건드리지 않는다.

- 차량 엔진/타이어/서스펜션 물리 재설계
- Weapon Aim, Ballistics, Target Selection 규칙 재설계
- HUD 전체 Visual Rework
- Boost/Nitro 시스템 신규 구현
- VR 전용 카메라 시스템
- Cinematic Camera 시스템
- 전역 Post Process 재설계

필요성이 발견되면 기존 소유 Feature에 handoff하거나 별도 후속 Feature로 분리한다.

---

## 8. 현재 상태

```text
Formal Promotion: COMPLETE
Feature: CF-FQ-057
VCFX-P0-00: TECHNICAL DESIGN PASS
VCFX-P0-01: CONTRACT FROZEN / TECHNICAL DESIGN PASS
Current Phase: VCFX-P0-04 Current System Promotion + Closure — COMPLETE
VCFX-P0-02: TECHNICAL PASS
Implementation: SOURCE MINIMUM IMPLEMENTATION PASS
Typed Vehicle Builder authoring: PASS — ReferenceMaxSpeedKmh + CameraPresentationDataOverride exact2 / Registry 137 / Resolver revision 7
Runtime source: PASS — FCFVehicleDrivingFXConfig, normalized planar motion/fail-safe, Speed FOV/Arm, Accel/Brake Kick, Gameplay View split, legacy-disabled branch + P0-03 actual Vehicle Mesh body-motion/free-look Lateral Presentation modulation
TargetSelect consumer split: PASS — Gameplay View preferred for Origin/Direction/Up/FOV
Reference benchmark: PASS — exact3 fixed-60Hz / top_speed_stable=true
  - DA_TestSedan = 97.830399 km/h / TargetHash 6149f7f557970105eae7417525e20377 / RunId 2bedfc6c-9a16-48a3-a70a-e52fa54d2868
  - DA_TestSUV = 92.759979 km/h / TargetHash 463b4f2f5053ef7a6c995912eb0a640f / RunId f2c5ceee-097b-4dc8-ad5d-f944909663bd
  - DA_VehicleDefense_TestSUV = 89.953880 km/h / TargetHash afd7d9a89aa43034bdf676193b56f675 / RunId 96188aaf-994c-48b4-bc4f-c43e2049ef60
Reference migration: PASS — exact3 typed Recipe commit + dirty-preserving selective Target exact1 migration
  - TestSUV pre-existing EQ_PFP_RocketReview drift preserved; full DefinitionApply was intentionally not used
Shared DA_Cam_Default normalized activation: PASS — Preview -> reviewed exact-one-flag Apply -> separate disk Verify / bUseNormalizedDrivingFX=true
BP binding: PASS — BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData hard-references /Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default
CameraModeFlags/Aim/Airborne attenuation: IMPLEMENTED / CURRENT PRODUCT PRODUCER UNWIRED — non-blocking wiring debt. Fresh C++ search found `SetCameraModeFlags()` declaration/definition only and no Product caller. Fresh AssetDump scanned all 6 indexed `/Game/CarFight` Blueprints and found no `SetCameraModeFlags`/`CameraModeFlags`/`bAimPresentation`/`bAirborne`/camera `bCombat` graph symbol. Therefore actual gameplay attenuation USER feel cannot be meaningfully tested in the current Product path.
Official UE 5.8 Build: PASS — job `58a7fd0fe6b04ea0834438c8cf61658a`; Admin terminal polling budget이 소진된 뒤 read-only recovery process `c6324d1cd0844d7c8efe07890cc86840`가 official UBT log에서 `Running=0 / CameraCompCompile=1 / CameraTestCompile=1 / Result:Succeeded / Failed=0`을 확인
Focused Automation: exact8 / 8 PASS — process `5588bff93fdc4694a7485eb9890c02f8`; 기존 exact7 회귀 + `VCFX_P0_03.LateralPresentationMath` PASS
Normalized motion direct validation: PASS — runtime과 Automation이 private `CFVehicleCameraMath.h` 계산 authority를 공유하며 차량별 25/50/75/100% SpeedRatio, 서로 다른 ReferenceSpeed의 동일 50% phase, Acceleration/Braking/Lateral signed rate, Clamp, zero/negative/NaN/Infinity exact0 fail-safe를 직접 검증
Lateral Presentation correction validation: PASS — calm body intensity 0, actual Roll/RollRate/YawRate normalized intensity, strongest-signal max, front/side/rear signed ViewAlignment `+1/0/-1`, diagonal continuous scale, invalid exact0, Body×View×Motion scale multiplication을 직접 검증
Product evidence:
  - benchmark summary = UE/Saved/CarFight/VCFXP002BenchSummary.json
  - reference selective Apply process = bcbfdd00fa4e4c758e5105931b218831
  - Camera activation process = 5cda072c442242f6877157cd5e0664fd / terminal VCFX_CAM_ACTIVATION=PASS
  - Camera AssetDump = adset_v1_1359b125c5ea74f8f5eb151e4d1dcb8d.3badfb69844be1ee7e008aec
  - CFVehicleData exact3 AssetDump = adset_v1_9d16ff0177681371138c203d43019413.4d00b5e31d236084cc22ed14
  - persisted references = Sedan 97.8303986 / TestSUV 92.7599792 / DefenseSUV 89.9538803 km/h
  - P0-03 current CameraData = AssetDump `adset_v1_4ea5ab37f8ccf66bb0004a8ff7b616d7.a447ee08dd738529a32da051`; normalized=true / MaxLateralRollDeg=2.5 / BodyMotionInterp=6 / RollAngleFull=7.5deg / RollRateFull=45deg/s / YawRateFull=90deg/s / existing Lateral·Accel·Brake curves unchanged
GoPyMCP managed PIE / RuntimeRead: DEFERRED / NON-BLOCKING by USER operating decision
Wagon USER-review activation: PASS — current Wagon TargetHash `0e38269d76e9907d1569facbe0ee10f4`에 fresh fixed-60Hz benchmark를 재결합해 `PeakSpeedKmh=89.175705`, `top_speed_stable=true`, RunId `513827e0-01c9-4529-a5a9-a6c15dfa0606`를 확보하고 `DA_Recipe_Wagon` + `DA_Vehicle_Wagon`에 `ReferenceMaxSpeedKmh=89.175705` + `CameraPresentationDataOverride=DA_Cam_Default` exact2 selective migration을 적용했다. Apply process `060797a8b98e4021a0c71d8f504086fc`는 Preview/Apply/fresh disk Verify PASS, post-apply AssetDump `adset_v1_798a56aa52a87f86beba97426c54fef0.2a2f7eb1f411847d9dffae55`도 exact2 persisted 값을 확인했다. Recipe `AppliedState`는 기존 revision/hash를 그대로 보존했다.
USER Review scope: Wagon only — 현재 프로젝트에서 USER가 직접 검수할 정상 차량 기준을 Wagon으로 고정한다. 다른 차량의 feel 결과를 P0-03 USER acceptance 근거로 사용하지 않는다.
USER Acceptance: PARTIAL ACCEPTANCE COMPLETE WITH EXPLICIT DEFERRED / UNAVAILABLE ITEMS
USER direct review:
  - 저속 직진 안정성: 변화 과장 없음 확인
  - 고속 Speed FOV 변화: USER가 직접 확인했고 현재 `볼만한 듯`으로 잠정 수용
  - 저속 Lateral Roll: 기존 `횡가속→Roll + clamp` 모델은 `차체는 잔잔한데 카메라만 휙휙 꺾여 오프로드처럼 느껴짐`으로 REJECT됐으나 actual body-motion + free-look Contract Correction 후 Wagon USER 재검수에서 `크게 이상한 것 못 느낌 / 일단 넘어가도 됨`으로 수용. Lateral Roll USER PASS.
  - 고속 Speed FOV/Arm 체감: 기준속도 100% 이후에도 변화 지속 확인 / USER PASS
  - Acceleration Rear Kick 방향·강도: Wagon에서 작동 확인 / 체감은 둔하지만 현재 추가 튜닝 보류 / 고성능 차량 추가 후 필요 시 재평가
  - Braking Forward Kick 방향·강도: Wagon 급정지에서 전방 Kick 작동 및 정지 후 baseline 복귀 확인 / 급정지 충격 체감은 약함 / 현재 추가 튜닝 보류
  - Lateral Roll 좌우 방향·강도
  - 벽 근처 Camera Pop/충돌 체감: 일반 접근/이탈의 즉시 Arm 압축 + 부드러운 복귀를 확인했고, Collision Chatter 교정 후 동일 hit/clear 미세 경계에서 반복 회전 재검수 결과 USER가 `지터링 사라졌어`로 확인. Camera Pop/Collision USER PASS.
  - 조준/타겟팅 방해 여부: Target Lock/조준 상태에서 가속·고속주행·좌우 선회·급브레이크·장애물 근처 Camera Compression 조건을 포함해 USER가 `별 문제 없어`로 확인. Presentation Camera FX가 Aim/Target Lock을 끌거나 틀거나 비정상 해제시키는 문제 없음 / USER PASS
  - 멀미/피로감: USER 결정으로 Deferred / PASS 미주장
  - 실제 gameplay에서 사용 가능한 Combat/Aim/Airborne 상태 감쇠 체감: 현재 Product producer/wiring 없음 / USER 검수 불가 / non-blocking wiring debt
Next:
  1. `Acceleration Rear Kick`은 Wagon에서 작동 자체를 USER가 확인했다. 현재 둔한 체감은 Wagon의 느린 가속 특성 영향 가능성이 있으므로 추가 튜닝하지 않고 보류한다.
  2. Rear Kick은 향후 고성능 차량이 추가된 뒤에도 부족하거나 과도한 문제가 재현될 때만 다시 연다. 현재 `AccelerationRearKickScale=1.25`와 Curve shape는 유지한다.
  3. Overspeed 고속 지속가속은 100 km/h 이후에도 FOV/Arm 변화가 계속되는 것을 USER가 직접 확인해 PASS했다. 재검수 대상으로 되돌리지 않는다.
  4. `Braking Forward Kick`은 Wagon 급정지에서 작동과 baseline 복귀를 USER가 확인했지만 급정지 충격 체감은 약했다. 현재 Wagon의 제동 성능과 Arm-only Presentation 특성이 함께 영향을 줄 수 있으므로 추가 튜닝하지 않고 보류한다.
  5. 향후 제동력이 강한 차량에서도 급정지 체감이 부족하면 BrakingArmKickCurve/별도 scale 또는 필요 시 다른 Presentation 축을 재평가한다.
  6. `Camera Pop/Collision`은 USER PASS다. 일반 접근/이탈의 즉시 Arm 압축 + 부드러운 복귀를 확인했고, 이전에 재현됐던 hit/clear 미세 경계의 빠른 압축↔복귀 Chatter/Jitter는 교정 후 동일 조건 재검수에서 사라졌음을 USER가 확인했다.
  7. root cause와 교정 계약은 그대로 유지한다. SpringArm `SolvedArmLength`를 Presentation `CurrentArmLength`에 역주입하던 feedback loop를 제거했고, 별도 Collision Recovery Arm + full-path `ECC_Camera` Sphere Sweep + `0.10s` Clear hold + `4cm` release probe padding + `InterpSpeed=6` 복귀를 사용한다. SpringArm 자체 collision test는 최종 안전망으로 유지한다.
  8. Camera collision 관련 Source/DataAsset 추가 변경은 하지 않는다. Technical exact8 PASS와 USER 경계 재검수 PASS를 현재 기준선으로 유지한다.
  9. `조준/타겟팅 방해 여부`는 USER PASS다. Target Lock/조준 상태의 가속·고속주행·좌우 선회·급브레이크·장애물 근처 Camera Compression 조건에서 Presentation Camera FX 때문에 Aim/Target Lock이 끌리거나 틀어지거나 비정상 해제되는 문제를 USER가 발견하지 않았다.
  10. `멀미/피로감` USER 검수는 사용자의 `멀미는 일단 넘어가` 결정에 따라 **Deferred**다. PASS로 확대하지 않으며 향후 Camera FX 튜닝 또는 차량 성능군 확대 시 다시 확인할 수 있다.
  11. Combat/Aim/Airborne 감쇠 producer/wiring 기술 확인 결과, C++ Product caller는 0건이고 `/Game/CarFight`의 현재 인덱스 Blueprint 6개 전체에서도 관련 graph symbol이 0건이다. 따라서 감쇠 계산 자체는 구현돼 있으나 실제 Product 상태를 주입하는 경로는 현재 없다. USER feel 검수 대상에서 제외하고 non-blocking wiring debt로 남긴다.
  12. `VCFX-P0-04 Current System Promotion + Closure`를 완료했다. Current implementation owner는 `Document/Systems/Vehicles/VehicleCamera.md v1.3.0`이다. 이 Work에는 더 이상 current next action이 없다. 향후 재개 trigger는 (a) 고성능/강제동 차량에서 Rear/Brake tuning 재평가 필요, (b) USER가 Comfort review를 다시 열기로 결정, (c) Combat/Aim/Airborne authoritative producer가 구현되어 attenuation wiring/feel 검증이 가능해지는 경우뿐이며, 그때는 새 lifecycle로 연다.
```

---

## 9. Changelog

### v0.3.0 - 2026-10-02

- `VCFX-P0-04 Current System Promotion + Closure`를 완료하고 representative Plan을 `Historical + Retained Path`로 전환했다. 물리 이동(G5)은 기존 dirty/reference 보호를 위해 Deferred다.
- Current implementation owner는 `Document/Systems/Vehicles/VehicleCamera.md v1.3.0`이다.
- USER PASS 범위는 Lateral Roll, Overspeed Speed FOV/Arm, Camera Pop/Collision, Aim/Targeting Interference다.
- Acceleration Rear Kick과 Braking Forward Kick은 기능 동작을 확인했으나 추가 feel tuning은 각각 고성능/강제동 차량 이후로 Deferred다. 멀미/피로감은 USER 결정으로 Deferred이며 PASS로 확대하지 않는다.
- Combat/Aim/Airborne attenuation은 계산/API 구현을 보존하되 현재 Product producer가 없는 non-blocking wiring debt로 닫는다. 실제 producer가 생길 때 새 lifecycle에서 wiring/feel을 검증한다.
- 기존 official UE 5.8 Build 성공과 focused Automation exact8/8 PASS, Camera collision USER revalidation, Aim/Targeting USER PASS evidence를 closure baseline으로 보존한다. 이번 closure에서 Source/DataAsset mutation은 없다.

### v0.2.21 - 2026-10-02

- Combat/Aim/Airborne Camera Mode attenuation의 실제 producer/wiring을 fresh audit했다.
- C++ 전체 검색에서 `SetCameraModeFlags()`는 `CFVehicleCameraComp`의 선언/정의만 존재하고 Product caller는 0건이었다.
- AssetDump current index의 `/Game/CarFight` Blueprint는 exact6이며, 6개 전체 `bp_search_index`에서 `SetCameraModeFlags`, `CameraModeFlags`, `bAimPresentation`, `bAirborne`, camera `bCombat` 관련 graph symbol이 0건이었다.
- 따라서 Mode attenuation 계산 로직은 구현돼 있으나 현재 Product gameplay에서는 producer가 없어 USER feel 검수가 불가능하다. 이 항목을 non-blocking wiring debt로 분류한다.
- P0-03에서 즉시 실행 가능한 USER 검수는 더 없다. Comfort는 Deferred, Accel/Brake 추가 feel tuning은 향후 성능 차량으로 Deferred다. 현재 상태 수용 시 exact next lifecycle 단계는 `VCFX-P0-04 Current System Promotion + Closure`다.

### v0.2.20 - 2026-10-02

- USER가 `멀미는 일단 넘어가`로 결정해 멀미/피로감 항목을 PASS가 아닌 Deferred로 기록했다.
- 기존 Lateral/Overspeed/Camera Pop-Collision/Aim-Targeting USER PASS와 Rear/Brake 기능 확인 상태는 유지한다.
- Source/DataAsset mutation은 없다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. exact next는 Combat/Aim/Airborne 감쇠 상태의 실제 producer/wiring 기술 확인이다.

### v0.2.19 - 2026-10-02

- Camera Pop/Collision USER PASS 이후 `조준/타겟팅 방해 여부` USER 검수를 진행했다.
- Target Lock/조준 상태에서 가속, 고속주행, 좌우 선회, 급브레이크, 장애물 근처 Camera Compression 조건을 포함해 확인했고 USER가 `별 문제 없어`로 판정했다.
- Presentation Camera FX가 Aim/Target Lock을 끌거나 틀거나 비정상 해제시키는 문제는 재현되지 않아 Aim/Targeting Interference 항목을 USER PASS로 확정한다.
- Source/DataAsset mutation은 없으며 이전 Build + focused exact8 PASS evidence를 그대로 유지한다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. exact next USER 항목은 `멀미/피로감`이다.

### v0.2.18 - 2026-10-02

- Collision Chatter 교정본을 이전 문제와 동일한 hit/clear 미세 경계 조건에서 USER가 재검수했고 `지터링 사라졌어`로 판정했다.
- 일반 장애물 접촉 시 즉시 Arm 압축, 장애물 이탈 시 부드러운 복귀, 경계에서의 안정 유지까지 확인되어 `Camera Pop/Collision` 항목을 USER PASS로 복구했다.
- v0.2.16~17의 root cause, collision recovery 구조, USER 재빌드 성공, focused Automation exact8/8 PASS evidence는 그대로 보존한다. 이번 USER PASS로 Source/DataAsset 추가 mutation이나 Build/Automation 재실행은 하지 않는다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. exact next USER 항목은 `조준/타겟팅 방해 여부`다.

### v0.2.17 - 2026-10-02

- Camera Collision Chatter 교정 후 USER가 Editor 종료 상태에서 공식 UE 5.8 재빌드를 직접 수행했고 Build 성공을 보고했다. 따라서 이전 `LNK1104`는 코드 오류가 아닌 runtime-build interference였음이 확인됐다.
- 기존 `Tools/RunVCFXP002Tests.ps1`을 새 binary 기준으로 재실행했다. process `ebf0a8c24cd244719cdd2b27eb2759e2`, execution mode `exact_list_same_process`, `SUCCESS_COUNT=8`, `FAILURE_COUNT=0`, `MISSING_COUNT=0`, `UNEXPECTED_COUNT=0`, `DUPLICATE_TERMINAL_COUNT=0`, terminal marker `VCFX_P0_02_FOCUSED_AUTOMATION=PASS`로 Technical PASS했다.
- `FrozenDefaults`는 새 `CollisionReleaseHoldTimeSec=0.10`, `CollisionReleaseProbePaddingCm=4.0`, `CollisionRecoveryInterpSpeed=6.0` seed를 기존 exact8 범위 안에서 검증한다.
- Camera Collision Chatter Correction은 Technical PASS로 전환한다. 다만 시각적/체감 품질은 USER 판단이므로 `Camera Pop` USER PASS는 아직 복구하지 않는다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. exact next는 동일 hit/clear 경계에서 카메라를 천천히 반복 회전해 빠른 압축↔복귀 chatter가 사라졌는지 USER 재검수하는 것이다.

### v0.2.16 - 2026-10-02

- v0.2.15에서 일반 장애물 접근/이탈만 보고 기록했던 Camera Pop USER PASS를 취소했다. USER가 카메라를 hit/clear 미세 경계에 두고 회전할 때 카메라가 빠르게 안쪽으로 압축됐다가 다시 복귀하는 동작을 반복하는 Collision Chatter/Jitter를 재현했다.
- fresh Source Audit에서 SpringArm collision으로 줄어든 `SolvedArmLength`를 `CurrentArmLength`에 역주입하고, 다음 frame부터 Presentation 목표 길이로 다시 확장하는 feedback loop를 root cause로 확인했다. 짧아진 target이 obstacle을 self-clear한 뒤 재확장하면서 다시 충돌하는 반복이 가능했다.
- `CurrentArmLength`는 Presentation 목표 상태로 유지하고 collision solved distance를 더 이상 역주입하지 않는다. 별도 `CollisionRecoveryArmLength`, `CollisionClearElapsedSec`, `bCollisionRecoveryActive` 상태를 추가했다.
- `ResolveCollisionStableArmLength()`은 전체 Presentation 목표 경로를 `ECC_Camera` Sphere Sweep으로 검사한다. 충돌 진입은 즉시 압축하고, blocking 중 outward expansion을 금지하며, 해제 시 `0.10s` 연속 Clear + `4cm` 추가 Probe hysteresis 이후 `InterpSpeed=6`으로 복귀한다. SpringArm built-in collision은 안전망으로 그대로 둔다.
- `FCFVehicleCameraTuningConfig`에 `CollisionReleaseHoldTimeSec=0.10`, `CollisionReleaseProbePaddingCm=4.0`, `CollisionRecoveryInterpSpeed=6.0` tuning을 추가했고 기존 exact8 focused set의 FrozenDefaults가 새 seed를 검증하도록 확장했다. 테스트 개수는 늘리지 않았다.
- 최초 공식 UE 5.8 Build job `4327e342b8b74c87acddaa0ed79ee6b4`는 `CFVehicleCameraComp.cpp`와 `CFVehicleCameraFXTests.cpp`를 포함한 compile 단계까지 PASS했으나, 실행 중인 `UnrealEditor.exe`가 `UnrealEditor-CarFight_Re.dll` / `UnrealEditor-CarFight_ReEditor.dll`을 점유해 최종 link가 `LNK1104`로 차단됐다. 이후 USER가 Editor 종료 후 같은 수정본을 직접 재빌드했고 Build 성공을 보고했다. 새 binary 기준 focused Automation process `ebf0a8c24cd244719cdd2b27eb2759e2`는 exact8/8 PASS, `FAILURE_COUNT=0`, `MISSING_COUNT=0`, `UNEXPECTED_COUNT=0`, `DUPLICATE_TERMINAL_COUNT=0`을 확인했다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. exact next는 동일 hit/clear 경계 USER 재검수이며 그 전에는 조준/타겟팅 검수로 진행하지 않는다.

### v0.2.15 - 2026-10-02

- Wagon 주변 장애물 접근/이탈 USER 검수에서 Camera Pop/충돌 체감을 확인했다.
- 장애물 접촉 시 CameraBoom이 빠르게 안쪽으로 압축되는 반응은 장애물 관통을 막기 위한 정상 collision response로 수용했다.
- 장애물에서 빠져나올 때 카메라가 부드럽게 원래 거리로 복귀하는 것을 확인해 `Camera Pop` 항목을 USER PASS로 기록했다.
- Source/DataAsset/Collision 설정은 변경하지 않았다. Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다.
- 다음 USER 항목은 `조준/타겟팅 방해 여부`다.

### v0.2.14 - 2026-10-02

- Wagon 급정지 USER 검수에서 Braking Forward Kick의 작동과 정지 후 baseline 복귀를 확인했다.
- 다만 `급정지한다`는 체감은 약했다. 현재 구현은 실제 longitudinal deceleration을 normalized 입력으로 사용하고 `BrakingArmKickCurve`를 통해 Arm을 줄이는 방식이므로 Wagon의 약한 제동 성능이 입력 강도를 낮출 수 있다.
- 동시에 현재 Brake Presentation은 Arm 축소 중심이며 Curve endpoint도 보수적이므로 차량 성능 외에 Presentation 설계 자체도 체감을 완화할 수 있다.
- 현재는 Source/DataAsset를 수정하지 않고 Brake tuning을 보류한다. 향후 제동력이 강한 차량에서도 동일 현상이 재현될 때만 tuning 또는 Presentation 축을 다시 연다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. 다음 USER 항목은 `Camera Pop`이다.

### v0.2.13 - 2026-10-02

- Wagon USER 재검수에서 Acceleration Rear Kick의 작동 자체는 확인했다. 다만 Wagon의 가속이 느려 뒤로 당겨지는 체감은 둔하게 느껴졌다.
- 현재 단계에서는 수치나 Curve를 추가 조정하지 않기로 USER 결정했다. Wagon 단일 차량의 느린 가속 특성만으로 전체 Rear Kick tuning을 확대하지 않는다.
- `AccelerationRearKickScale=1.25`와 현재 AccelerationArmKickCurve shape를 그대로 유지한다.
- 향후 고성능 차량이 추가된 뒤에도 Rear Kick이 부족하거나 과도한 문제가 재현될 때만 tuning을 다시 연다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. exact next USER 항목은 `Braking Forward Kick`이다.

### v0.2.12 - 2026-10-02

- Wagon USER 재검수에서 `100 km/h를 넘어가도 Speed FOV/Arm 변화가 계속되는 것`을 직접 확인했다.
- Overspeed 고속 지속가속 Presentation 항목을 USER PASS로 기록한다. 이는 전체 `VCFX-P0-03 USER Acceptance`가 아니라 Overspeed 항목에 한정된 수용이다.
- 기존 0~100% Speed Curve, `ReferenceMaxSpeedKmh`, 차량 물리, Brake/Lateral, Gameplay View/TargetSelect 계약에는 추가 변경이 없다.
- exact next USER 항목은 `Acceleration Rear Kick` 초반 급가속 방향·강도 검수다. Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다.

### v0.2.11 - 2026-09-30

- Wagon USER 검수에서 초반 급가속 Rear Kick이 약하고, `ReferenceMaxSpeedKmh≈89.18 km/h`를 넘어 실제 속도가 계속 증가해도 기존 SpeedRatio가 1.0에서 포화되어 약 100 km/h 이후 고속 가속감이 줄어드는 문제가 확인됐다.
- 원인은 `CFVehicleCameraMath::NormalizeSpeedRatio`가 기본적으로 `0..1` clamp되고 Speed FOV/Arm Curve가 1.0에서 끝나는 구조였다. `ReferenceMaxSpeedKmh`는 benchmark 기반 performance reference이므로 카메라 체감 때문에 값을 임의로 올리지 않았다.
- 기존 0~100% Speed FOV/Arm Curve를 그대로 보존하면서 `MaxSpeedPresentationRatio=1.5`를 추가했다. Runtime은 SpeedRatio를 CameraData가 허용한 ratio까지 계산하되, 기존 Curve에는 `min(SpeedRatio,1.0)`만 입력하고 `OverspeedPhase=(SpeedRatio-1)/(MaxRatio-1)`를 별도 계산한다.
- 현재 overspeed Product seed는 `MaxOverspeedFOVBonusDeg=4`, `MaxOverspeedArmBonusCm=30`이다. Wagon reference 기준 약 89 km/h에서 기존 Curve 100%를 유지하고 약 134 km/h(150%)까지 FOV/Arm headroom이 선형으로 추가된다.
- 초반 Rear Kick은 기존 `AccelerationArmKickCurve` shape를 보존하고 `AccelerationRearKickScale=1.25`만 추가했다. Braking/Lateral Curve, 차량 물리, ReferenceMaxSpeed, Gameplay View 계약은 변경하지 않았다.
- 첫 official Build job `fa3d1d7d01e64cb4b4e8fb16b3237823`은 `CFVehicleCameraFXTests.cpp` tail이 잘려 `#if/#endif` 짝이 깨진 테스트 소스 오류 `C1070`로 실패했다. Product/Runtime 링크 문제는 아니었고, CameraComp compile까지는 진입했다. 테스트 tail을 복구한 corrected Build job `c17a0f712ea741e3bcbb4e6568cf71f0`은 `CFVehicleCameraFXTests.cpp` compile, CarFight_Re/ReEditor DLL link, UBT `Result:Succeeded`로 PASS했다.
- focused Automation process `4c661b1277fa477aa9ac27d74ffdbb27` exact8/8 PASS. `FrozenDefaults`와 `NormalizedMotionMath`가 default 0..1 parity, explicit 1.5 overspeed ratio clamp, 0/0.5/1 overspeed phase, invalid exact0, 새 overspeed/rear-kick seed를 검증한다.
- Product tune process `a2d5c9d54d8a4c21b0a40cdd1a3f50e3`의 Python은 `VCFX_SPEED_ACCEL_TUNE_PASS`를 출력하고 Asset 저장까지 성공했지만 wrapper가 과거 `VCFX_LATERAL_TUNE_PASS` marker를 기대해 process 자체는 실패로 표기됐다. blind retry하지 않고 log evidence를 확인한 뒤 wrapper를 v1.2.1로 교정했다.
- fresh persisted AssetDump `adset_v1_6317bedee790f503c072f871aa0b1e7f.67430b6d68b9cefb0e867c06`에서 `normalized=true`, `MaxSpeedPresentationRatio=1.5`, `MaxOverspeedFOVBonusDeg=4`, `MaxOverspeedArmBonusCm=30`, `AccelerationRearKickScale=1.25`, `MaxLateralRollDeg=2.5`, `MotionRateInterpSpeed=10`을 확인했다. 기존 Speed/Accel/Brake/Lateral Curve는 모두 보존됐다.
- lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다. 다음은 Wagon에서 초반 급가속 Rear Kick과 100 km/h 이후 고속 지속가속감을 직접 재검수한다.

### v0.2.10 - 2026-09-29

- P0-03 Lateral Contract Correction의 Wagon USER 재검수를 완료했다. USER는 교정 후 주행에서 `크게 이상한 것을 못 느끼겠으니 일단 넘어가도 되겠다`고 판정했다.
- 이를 Lateral Roll 항목의 USER PASS로 기록한다. 이는 전체 VCFX-P0-03 USER Acceptance가 아니라 Lateral 항목에 한정된 수용이다.
- body-motion/free-look normalized/gameplay-view 계약과 Product 값은 v0.2.9 Technical PASS 상태 그대로 보존한다. Source/Asset mutation, Build, Automation 재실행은 0이다.
- exact next USER 항목은 `Acceleration Rear Kick` 방향·강도 검수다. Lifecycle phase는 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`다.

### v0.2.9 - 2026-09-29

- P0-03 USER review에서 직진 Speed FOV 변화는 직접 확인돼 현재 체감이 `볼만한 듯`으로 잠정 수용됐다. 반면 저속 좌우 회전은 차량 차체가 크게 흔들리지 않는데 Camera Roll만 빠르게 꺾여 이질적이라는 피드백을 받았다.
- USER가 CarFight의 카메라는 일반 레이싱 카메라와 달리 조준을 위해 시점을 전방~측면으로 자유롭게 돌린다는 점을 명시했고, 단순 `MaxLateralRollDeg` 축소가 아니라 `차가 실제로 격하게 움직일 때 카메라도 반응`하는 모델로 교정하기로 승인했다.
- 기존 `LateralRate`는 signed 방향/기본 요구량 authority로 유지하고, 실제 Vehicle Mesh의 Roll angle / local Roll angular rate / local Yaw angular rate 중 가장 강한 값을 `BodyMotionIntensity 0..1`로 계산해 Lateral Roll 강도만 조절하도록 구현했다. 차체 Roll 자체를 Camera Rotation으로 복사하지 않아 `bIsolateCameraFromVehiclePitchRoll` 안정화 철학을 유지한다.
- 자유조준 대응은 `ViewAlignment = dot(VehicleForwardXY, ViewForwardXY)` signed 값으로 구현했다. 정면 `+1`, 측면 `0`, 후면 `-1`이며 free-look 각도 자체를 `bAimPresentation`으로 추론하지 않는다. explicit Aim/Combat mode attenuation 계약은 별도로 유지한다.
- 최종 Lateral Presentation은 `Clamp(LateralRollCurve(LateralRate)) × BodyMotionIntensity × ViewAlignment × ResolvedMotionFXScale`로 계산한다. Gameplay View는 신규 Roll 적용 전 실제 FollowCamera rotation에서 계속 캡처하므로 Aim/TargetSelect Direction/Up/FOV 계약은 유지된다.
- 신규 CameraData seed는 `LateralBodyMotionInterpSpeed=6`, `LateralBodyRollAngleForFullIntensityDeg=7.5`, `LateralBodyRollRateForFullIntensityDegPerSec=45`, `LateralBodyYawRateForFullIntensityDegPerSec=90`이다. `CFVehicleCameraMath.h v1.1.0`, `CFVehicleCameraData.h / Types.h / Comp.h / Comp.cpp v0.3.0`, `CFVehicleCameraFXTests.cpp v1.2.0`에 반영했다.
- Official UE 5.8 Build job `58a7fd0fe6b04ea0834438c8cf61658a`는 핵심 CameraComp/CameraFXTests non-unity compile까지 확인됐고, Admin 동일-request terminal polling budget 소진 후 read-only recovery process `c6324d1cd0844d7c8efe07890cc86840`가 official UBT log의 `Running=0 / Result:Succeeded / Failed=0`을 확인해 Build PASS를 닫았다.
- focused Automation process `5588bff93fdc4694a7485eb9890c02f8`는 exact8/8 PASS다. 신규 `CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_03.LateralPresentationMath`가 calm body=0, body signal normalization/max, front/side/rear `+1/0/-1`, diagonal continuous alignment, invalid exact0, Body×View×Motion scale을 직접 검증한다.
- 기존 임시 완화값 `MaxLateralRollDeg=1.25`는 새 modulation Technical PASS 후 `2.5deg` headroom으로 복구했다. headless Product tune process `fe8172dcb3a94644abbc442de3d9314e` PASS이며 fresh AssetDump `adset_v1_4ea5ab37f8ccf66bb0004a8ff7b616d7.a447ee08dd738529a32da051`에서 normalized=true, MaxRoll=2.5, body thresholds `6/7.5/45/90`, MotionRateInterpSpeed=10, 기존 Lateral/Accel/Brake curve 유지 상태를 확인했다.
- Lifecycle은 계속 `VCFX-P0-03 USER Driving / Combat Feel Review`이며 USER Acceptance는 미완료다. 다음은 Wagon 전방 완만한 회전 → 전방 더 격한 회전 → 약 90° 측면 자유시점 순으로 새 Lateral feel을 직접 재검수한다. Lifecycle/next gate가 바뀌지 않았으므로 새 UDS successor는 생성하지 않는다.

### v0.2.8 - 2026-09-29

- P0-03 USER 저속 안정성 1차 피드백에서 직진보다 좌우 회전 시 Camera FX가 확실히 보였고, 저속 회전에서도 화면 흔들림이 과해 오프로드 주행처럼 느껴진다는 판정을 받았다.
- frozen normalized/gameplay-view 계약과 Speed/Accel/Brake 계층은 유지하고 `DA_Cam_Default.DrivingFXConfig.MaxLateralRollDeg`만 `3.0 -> 1.25 deg`로 exact-one-field 교정했다. `MotionRateInterpSpeed=10`, Lateral curve seed, Acceleration/Braking curve, normalized flag는 변경하지 않았다.
- headless tune process `b5cb4809261240a894a24d2192c6a329` PASS. fresh persisted AssetDump `adset_v1_8e146e597214d9a99493f69391be0f73.2dadf797df157b5767bf6f08`에서 `MaxLateralRollDeg=1.25`, `MotionRateInterpSpeed=10`, frozen Lateral/Accel/Brake curve 유지, normalized=true를 재확인했다.
- USER Acceptance는 아직 Pending이며 다음 확인은 동일 Wagon 저속 좌우 회전에서 흔들림이 충분히 줄었는지 재검수한다. Lifecycle phase는 VCFX-P0-03 그대로다.

### v0.2.7 - 2026-09-29

- `VCFX-P0-03 USER Driving / Combat Feel Review`의 실제 검수 차량을 USER 지시에 따라 `Wagon` 단일 차량으로 고정했다.
- Wagon의 persisted 상태를 fresh 확인한 결과 `ReferenceMaxSpeedKmh=0`, `CameraPresentationDataOverride=None`이어서 normalized Driving FX가 USER 검수에 유효하게 들어갈 수 없는 상태였다. 공용 Component fallback 바인딩 존재 여부와 무관하게 `ReferenceMaxSpeedKmh=0`은 normalized Driving FX input-valid gate를 닫으므로 Wagon USER Review blocker였다.
- 기존 DrivingAcceptanceReceipt의 old TargetHash `8aad29d04e008fcd2acb2e6470cd87f1`는 current Wagon Target과 달라 그대로 사용하지 않았다. fresh typed Preview에서 current TargetHash `0e38269d76e9907d1569facbe0ee10f4`를 확인한 뒤 동일 hash에 fixed-60Hz technical benchmark를 다시 결합했다. RunId `513827e0-01c9-4529-a5a9-a6c15dfa0606`, `PeakSpeedKmh=89.175705`, `top_speed_stable=true`다.
- `CFVCFXRefCmdlet`을 v1.3.0으로 일반화해 기존 Reference exact1 migration 계약을 보존하면서 optional `CameraPresentationDataOverride` exact2 selective migration을 지원했다. USER가 공식 UE 5.8 Build 성공을 확인했고 새 commandlet은 실제 headless process에서 로드·실행됐다.
- Wagon Recipe/Target에 `ReferenceMaxSpeedKmh=89.175705`와 `CameraPresentationDataOverride=/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default` exact2만 적용했다. baseline drift 0, applied_field_count 2이며 unrelated Target field와 Recipe `AppliedState`를 보존했다. process `060797a8b98e4021a0c71d8f504086fc` terminal `VCFX_WAGON_CORRECTION=PASS`.
- fresh persisted AssetDump `adset_v1_798a56aa52a87f86beba97426c54fef0.2a2f7eb1f411847d9dffae55`에서 Wagon `ReferenceMaxSpeedKmh=89.175705`, `CameraPresentationDataOverride=DA_Cam_Default`를 재확인했다.
- earlier narrow CameraData-only dependency query의 referencer exact0은 외부 Blueprint binding 부재를 독립적으로 증명하지 못하므로 BP binding 부재 근거로 사용하지 않는다. Wagon은 차량별 explicit Presentation override를 갖게 되어 P0-03 USER review에서 fallback binding 해석에 의존하지 않는다.
- lifecycle phase/next는 기존 `VCFX-P0-03 USER Driving / Combat Feel Review` 그대로이므로 새 UDS successor는 생성하지 않는다.

### v0.2.6 - 2026-09-29

- `VCFX-P0-02 TECHNICAL PASS`의 검증 근거를 production normalized motion math 직접 검증까지 강화했다. `CFVehicleCameraMath.h` Private pure helper를 추가하고 `UCFVehicleCameraComp`가 실제 Speed/Acceleration/Braking/Lateral ratio 계산에 같은 helper를 사용하도록 교정해 Runtime과 Automation의 계산 authority를 단일화했다. Public API / Blueprint debug surface는 추가하지 않았고 계산 의미는 v0.2.5 계약에서 변경하지 않았다.
- `CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_02.NormalizedMotionMath`를 focused Automation에 추가했다. 차량별 25/50/75/100% SpeedRatio, 서로 다른 100/200 km/h Reference 차량의 동일 50% phase, positive Acceleration, negative Braking, signed Lateral Rate, overshoot Clamp, zero/negative/NaN/Infinity reference/input exact0 fail-safe를 직접 검증한다.
- 첫 exact7 process `153fc299633044b39f297aaa64332ea0`에서는 production 수식 오류가 아니라 0.75/0.20/±0.40 float division 결과에 대한 test assertion 허용오차가 과도하게 엄격해 신규 test 1개만 FAIL했다. Product helper는 변경하지 않고 test tolerance만 `1e-4`로 명시한 v1.1.1로 교정했다.
- current official UE 5.8 Build `8be0cdf0cd374ebc8616cd50137b989b` PASS, 최종 focused Automation process `6ec4346e4f66433d9eccfd1a46fe156f` exact7/7 PASS를 확인했다. 기존 Registry/Resolver/Sensor/TargetSelect/FrozenDefaults 6개 회귀도 모두 유지됐다.
- `DA_Cam_Default`의 persisted `bUseNormalizedDrivingFX=true`는 read-only Preview process `201ce5f92f3e483294137e7412fdabf6`에서 `CurrentEnabled=1 / ChangeRequired=0`으로 재확인했으며 이번 보강에서 Product Asset mutation은 0이다.
- canonical UDS fresh DAG head는 기존 `ver_9d73b4d68c214c8d81fd8bb4a6e91f20` exact1이며 이미 `VCFX-P0-03 USER Driving / Combat Feel Review`를 가리킨다. Lifecycle/Next가 바뀌지 않았으므로 새 UDS successor를 생성하지 않는다.

### v0.2.5 - 2026-09-29

- `VCFX-P0-02 Minimum Implementation + Technical Validation`을 TECHNICAL PASS로 종료하고 next gate를 `VCFX-P0-03 USER Driving / Combat Feel Review`로 전진시켰다. USER Acceptance는 아직 주장하지 않는다.
- accepted fixed-60Hz Vehicle Builder benchmark exact3를 새로 확보했다. `DA_TestSedan=97.830399`, `DA_TestSUV=92.759979`, `DA_VehicleDefense_TestSUV=89.953880 km/h`이며 세 결과 모두 `top_speed_stable=true`, exact Target path + DefinitionHash + RunId에 결합돼 있다.
- Sedan/TestSUV의 `BaseVehicleMassKg=0` legacy 상태는 Product mass를 수정하지 않고 benchmark 전용 explicit `PreserveLegacyMass` opt-in에서 fresh BeginPlay Chaos configured/actual mass를 보존해 측정했다. 기본 benchmark hard-fail 계약은 유지한다.
- Reference migration은 existing typed `SetDefaultDataIntent` Preview/Commit authority를 사용했다. TestSUV에는 PFP `EQ_PFP_RocketReview` 기존 dirty가 있어 full DefinitionApply가 unrelated 무장을 되돌릴 위험이 확인됐으므로, baseline Resolver drift exact preservation + reviewed `ReferenceMaxSpeedKmh` exact-one-field selective Target migration으로 전환했다. fresh AssetDump에서 RocketReview가 그대로 보존됐음을 확인했다.
- persisted `ReferenceMaxSpeedKmh` exact3는 Sedan `97.8303986`, TestSUV `92.7599792`, DefenseSUV `89.9538803 km/h`로 확인됐다. `CameraPresentationDataOverride=None`은 그대로 유지한다.
- fresh Blueprint AssetDump에서 `BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData`가 `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`를 hard reference함을 확인했다.
- `DA_Cam_Default`는 full CameraTuningConfig + AimProfile pre-state scope hash를 검토한 뒤 `DrivingFXConfig.bUseNormalizedDrivingFX` exact-one-flag만 false→true로 변경했다. package-byte rollback과 별도 disk reload Verify가 PASS했고 activation process `5cda072c442242f6877157cd5e0664fd`는 `VCFX_CAM_ACTIVATION=PASS`로 종료했다.
- current official UE 5.8 Build `556bcd7e5b704253a78dcdd1b11d1ce9` PASS, activation 후 focused Automation process `8ec5c2a8fc5c4d5482895aeed7d2eea0` exact6/6 PASS를 확인했다.
- fresh persisted evidence는 Camera dataset `adset_v1_1359b125c5ea74f8f5eb151e4d1dcb8d.3badfb69844be1ee7e008aec`, Vehicle exact3 dataset `adset_v1_9d16ff0177681371138c203d43019413.4d00b5e31d236084cc22ed14`이다.
- GoPyMCP managed PIE/RuntimeRead는 현재 사용자 운영 결정에 따라 optional deferred evidence로 유지하며 P0-02 Technical PASS나 P0-03 진입을 차단하지 않는다. 시각·주행감·조준감·멀미/피로감은 USER 직접 검수로 판정한다.

### v0.2.4 - 2026-09-28

- 현재 GoPyMCP를 작업 가능한 prerequisite로 취급하지 않는 사용자 운영 결정을 반영했다. prior `Managed UE Bridge interpreter identity mismatch`는 CF-FQ-057의 blocking gate에서 제거하고 optional deferred technical evidence로 재분류했다.
- Speed FOV/Arm, Accel/Brake Kick, Lateral Roll, Camera Pop, 조준 방해와 실제 gameplay에서 사용 가능한 Combat/Aim/Airborne 감쇠 결과처럼 사람이 직접 운전해서 확인할 수 있는 항목은 USER direct review로 이관했다.
- `SetCameraModeFlags()` Product producer 자동증명 부재 자체는 core Camera FX 진행 blocker에서 제외했다. 해당 상태가 실제 gameplay에 존재하면 USER review에서 결과를 확인하고, 상태 자체가 아직 존재하지 않으면 별도 후속 wiring debt로 남긴다.
- 실제 normalized FX 활성 자체를 막는 `ReferenceMaxSpeedKmh=0` exact3와 accepted same-Target/DefinitionHash `PeakSpeedKmh` evidence 부재는 그대로 Product migration blocker로 유지했다. 임의 숫자 추정과 legacy global fallback 금지 계약도 유지한다.
- Source/Build/Automation/AssetDump PASS evidence는 그대로 보존하며 재검증하지 않았다.

### v0.2.3 - 2026-09-28

- `VCFX-P0-02 Minimum Implementation`을 동결 계약 범위에서 additive 구현했다. `ReferenceMaxSpeedKmh` + `CameraPresentationDataOverride` typed Vehicle Builder authoring을 Registry 137 / Resolver revision 7에 온보딩하고, `FCFVehicleDrivingFXConfig`, normalized XY motion/fail-safe, Speed FOV/Arm, Accel/Brake Kick, Lateral Roll, Gameplay View 분리와 legacy-disabled 분기를 구현했다.
- `CFTargetCandidateSearch`는 신규 Driving Presentation Roll/FOV가 Target selection geometry에 섞이지 않도록 `GetGameplayView()`의 Origin/Direction/Up/FOV를 우선 사용한다.
- invalid reference/non-finite input은 Curve의 0-input authored 값과 무관하게 신규 Driving FX 전체 exact0이 되도록 `bDrivingMotionInputValid` gate를 추가했다. first-frame/hitch/teleport history reset은 SpeedRatio를 허용하되 Motion Kick/Roll history만 0으로 초기화한다.
- 공식 UE 5.8 `CarFight_ReEditor Win64 Development` final exact-source Build job `77dc3a851a374ef3a9aca8f4f91f5ab5` PASS. 첫 semantic Build에서는 신규 Registry test의 잘못된 멤버명 `AdoptGroup` 1건만 검출돼 `AdoptionGroup`으로 최소 교정 후 job `2c793e6cd80e4800940ce26c24f68d3b`가 PASS했다. 이후 유지보수 주석만 보강한 뒤 current managed Editor가 DLL을 점유해 중간 job `f20436956aac459f98aeb2f49805d33c`가 link-only file-lock으로 실패했으며, 이번 세션이 시작한 AI-owned Editor를 managed `stop_ai_owned`로 종료한 뒤 현재 exact Source를 재빌드해 최종 PASS했다.
- task-scoped focused Automation `Tools/RunVCFXP002Tests.ps1`로 Camera defaults/Camera typed Resolver/Registry/Sensor precedence/TargetSelect candidate+diagnostics exact6을 한 프로세스에서 실행해 6/6 PASS했다. process job `0d094404223140bc9978cce33b416607`.
- fresh AssetDump `adset_v1_10e5afc7236839148edaef9f408f10b4.48ee093a94f563196d5396a7`에서 `DA_Cam_Default`의 legacy endpoint `MaxSpeedFOVBonus=6`, `MaxSpeedArmLengthBonus=45`, `bUseNormalizedDrivingFX=false`를 확인했다.
- fresh CFVehicleData dataset `adset_v1_f6ea3a9d8ea246b03707781ce5e5a91c.9239b52fa2f0a7d2238a86df`에서 in-scope exact3 `DA_TestSedan`, `DA_TestSUV`, `DA_VehicleDefense_TestSUV` 모두 `ReferenceMaxSpeedKmh=0`, `CameraPresentationDataOverride=None`임을 확인했다. accepted Benchmark의 `PeakSpeedKmh + 동일 Target/DefinitionHash` evidence도 current docs에서 찾지 못했으므로 M4 계약대로 shared `DA_Cam_Default` normalized activation을 HOLD했고 Product asset 값을 임의 추정하지 않았다.
- C++ fresh search에서 `SetCameraModeFlags()` Product caller는 setter 정의 외 exact0이며 `bCombat`, `bAirborne`, `bAimPresentation` producer도 확인되지 않았다. `BP_CFVehiclePawn` fresh AssetDump preparation도 1회 fail-closed했으므로 Combat/Aim/Airborne attenuation runtime PASS를 주장하지 않는다.
- canonical Editor Runtime `runtime_55dc7fe189294f03a4df3bd910ad9122`는 Ready까지 정상 시작됐으나 bounded `carfight.pie.start`가 `Managed UE Bridge interpreter identity mismatch`로 PIE 시작 전에 fail-closed했다. GoPyMCP 자체는 CarFight 세션에서 수정하지 않고 PIE Technical Validation을 MCP-side BLOCKED로 남긴다.
- VCFX-P0-02는 Source/Build/Automation/AssetDump technical evidence까지 전진했으나 Product reference migration과 PIE RuntimeRead가 남아 있으므로 Complete/Acceptance로 승격하지 않는다.

### v0.2.2 - 2026-09-28

- VCFX-P0-01 교정본 Design Re-review를 완료해 `P0 0 / blocking P1 0 / P2 3 non-blocking`으로 `CONTRACT FROZEN / TECHNICAL DESIGN PASS` 판정했다.
- v0.2.0에서 발견한 blocking P1 후보 exact4는 v0.2.1 교정으로 전부 닫혔음을 재확인했다.
- 최종 contract는 VehicleData `ReferenceMaxSpeedKmh` + `CameraPresentationDataOverride`, VehicleCameraData 내부 `FCFVehicleDrivingFXConfig`, `FCFVehicleCameraGameplayView`, normalized math/fail-safe/Clamp, Gameplay-Presentation 소비 경계, mode attenuation, atomic migration 순서를 포함한다.
- C++ Source에서 CameraModeFlags producer와 explicit Aim-active authority는 확인되지 않았고 Blueprint producer도 이전 evidence limitation 때문에 미확정이므로 P2로 유지한다. P0-02에서 runtime producer evidence 전에는 mode attenuation PASS를 주장하지 않는다.
- P0-01에서 Source/Product Asset mutation 0, Build/Automation/PIE Not Run을 유지했다. exact next는 `VCFX-P0-02 Minimum Implementation + Technical Validation`이다.

### v0.2.1 - 2026-09-28

- v0.2.0 Contract Freeze candidate를 실제 Camera/Aim 소비 구조에 역검수해 blocking P1 후보 exact4를 발견하고 모두 문서 단계에서 교정했다.
- per-vehicle Camera override가 `DefaultAimProfile`까지 교체하던 위험을 제거했다. exact field를 `CameraPresentationDataOverride`로 교정하고 `CameraTuningConfig`만 override하며 Aim Gameplay owner는 기존 Component CameraData + explicit AimProfileOverride로 보존한다.
- Longitudinal motion 계산에서 previous velocity를 current axis로 재투영하지 않고 `PreviousForwardSpeedAbs` scalar를 이전 valid frame에서 캐시하도록 교정했다.
- invalid reference와 first-frame/Teleport history reset의 의미를 분리해 전자는 전체 normalized FX exact0, 후자는 SpeedRatio 유지 가능 + Motion FX exact0으로 교정했다.
- Gameplay Direction/Up은 새 rotation을 재구성하지 않고 신규 Driving Roll 적용 직전의 실제 FollowCamera rotation에서 캡처해 기존 BP relative orientation을 보존하도록 교정했다.
- normalized=false legacy path에서는 `CurrentGameplayFOV = CurrentFOV`로 유지해 기존 Target Search FOV 의미를 exact 보존한다.
- Source/Product Asset mutation은 여전히 0이며 다음은 교정본 Design Re-review다.

### v0.2.0 - 2026-09-28

- `VCFX-P0-01 Camera FX Contract + Data Freeze` candidate를 작성했다. 구현은 아직 시작하지 않았다.
- VehicleData exact fields를 `ReferenceMaxSpeedKmh` + optional `VehicleCameraDataOverride`로 Freeze하고, 기존 VehicleCameraData 안에 `FCFVehicleDrivingFXConfig` 하위 구조체 exact1을 추가하는 최소 구조로 수렴했다.
- Speed/Accel/Brake/Lateral normalized math, reverse-safe longitudinal magnitude handling, sample/Teleport fail-safe, Curve Seed와 absolute output clamp를 exact contract로 고정했다.
- `FCFVehicleCameraGameplayView`와 별도 `CurrentGameplayFOV`를 도입해 Aim Trace/TargetSelect/HUD 방향 소비자가 Driving Presentation Roll/FOV에 오염되지 않도록 API 의미를 고정했다.
- Presentation arm은 Speed + Accel - Brake, Presentation FOV는 Gameplay FOV + Speed FOV, Roll은 FollowCamera baseline relative rotation에 additive 적용하는 합성 순서를 Freeze했다.
- Combat/Aim/Airborne/Reverse는 Speed/Motion scale을 분리하고 동시 활성 시 minimum scale을 사용하도록 고정했다. Product에 명시적 Aim-active authority가 없으므로 `bAimPresentation=false` explicit modifier를 추가하고 추론을 금지했다.
- Legacy speed fields는 serialized compatibility용으로 보존하며 `bUseNormalizedDrivingFX=false`에서는 기존 동작을 exact 유지하고, normalized=true에서는 global `SpeedForMaxBonusKmh` fallback을 금지했다.
- Product migration은 source additive implementation → accepted benchmark 기반 per-Vehicle reference authoring → persisted confirmation → shared DA_Cam_Default normalized activation 순으로 동결했다.
- Fresh candidate review issue는 `P0 0 / blocking P1 0 / P2 3 non-blocking`이며, 다음은 Design Re-review다.

### v0.1.2 - 2026-09-28

- `VCFX-P0-00 Fresh Rebaseline + Pre-Implementation Review`를 `P0 0 / blocking P1 0 / P2 1 non-blocking / TECHNICAL DESIGN PASS`로 종료했다.
- 현재 Camera Speed 입력이 3D velocity magnitude이며 global `SpeedForMaxBonusKmh`에 의해 정규화되는 구조를 확인했다. 신규 Driving FX는 Planar(XY) velocity를 사용하도록 설계 경계를 교정했다.
- VehicleData에는 물리 원인값은 충분하지만 runtime authored 단일 최고속도 reference가 없고, Vehicle Builder Benchmark가 `PeakSpeedKmh` 등 실제 성능 evidence를 제공함을 확인했다.
- 차량당 별도 가속/제동/횡가속 기준값을 중복 관리하지 않고 VehicleData authored `ReferenceMaxSpeed` 성격 값 exact1을 공통 분모로 사용해 Speed Ratio와 종/횡 속도 변화율을 백분율 축으로 정규화하는 방향을 확정했다.
- TargetSelect가 실제 Camera Origin/Direction/Up/FOV를 소비함을 확인해 `Gameplay View / Presentation View` 분리를 P0 계약으로 승격했다. 실제 collision-resolved Camera Origin은 유지하되 Driving FX의 FOV/Roll/Shake가 Gameplay 각도·타겟 우선순위를 바꾸지 않게 한다.
- Fresh persisted AssetDump dataset `adset_v1_5acb83e891e1f4b9ef2d733afbb71be4.bdd1b15768494d24cc66139c`로 `DA_Cam_Default` 및 차량별 Movement 차이를 확인했다. VehicleCamera Systems 문서의 과거 Camera DA 경로 stale은 P2 non-blocking으로 남겼다.
- `BP_CFVehiclePawn` fresh dump preparation failure 1회는 반복 복구하지 않았으며 P0 설계를 차단하지 않는 evidence limitation으로 기록했다.
- Source/Product Asset mutation 0, Build/Automation/PIE Not Run. exact next는 `VCFX-P0-01 Camera FX Contract + Data Freeze`다.

### v0.1.1 - 2026-09-28

- USER 요구로 **차량별 정규화 비율 우선 원칙**을 P0 설계 계약으로 추가했다. 서로 다른 차량에 동일한 절대 속도·가속·제동 숫자를 공통 Camera FX 임계값으로 강제하지 않는다.
- Speed/Acceleration/Braking/Lateral Motion은 각 차량의 authoritative 성능 범위 대비 0.0~1.0(0~100%) Normalized Ratio를 기본 입력으로 사용하도록 계획을 교정했다.
- 최종 FOV/Offset/Roll 출력은 Ratio 입력과 분리해 차량별 튜닝 Curve/배율을 거친 뒤 절대 안전 Clamp를 적용하도록 경계를 명시했다.
- 차량별 성능 기준값의 authoritative source는 VCFX-P0-00에서 fresh 확인하며, 기준이 없을 경우 글로벌 상수를 임의 도입하지 않고 설계 blocker로 남긴다.
- Source/Product Asset/Build/Automation/PIE 변경은 수행하지 않았다.

### v0.1.0 - 2026-09-27

- USER 승인으로 `CF-FQ-057 Vehicle Camera Driving FX / 주행 카메라 연출 고도화`를 정식 Plan으로 승격했다.
- 기존 `UCFVehicleCameraComp`에 speed-aware FOV/Arm이 이미 존재함을 Fresh Source 기준으로 반영해 “FOV 신규 추가”가 아닌 “기존 기반 재기준화 + Layered FX 확장”으로 범위를 교정했다.
- `CFTargetCandidateSearch`가 실제 `FollowCamera->FieldOfView`를 소비하는 결합을 P1-1 설계 검수 항목으로 등록했다.
- Acceleration/Brake Kick, Lateral Roll, High-Speed Micro Motion, Aim/Combat Attenuation의 책임 경계와 VCFX-P0-00~04 Gate를 정의했다.
- Source/Product Asset/Build/Automation/PIE 변경은 수행하지 않았다.

## 10. Migration

- 기존 VehicleCamera 구현과 DataAsset을 이 문서 생성만으로 변경하지 않는다.
- 현재 speed-based FOV/Arm과 global `SpeedForMaxBonusKmh`는 serialized legacy compatibility 경로로 유지한다. `DrivingFXConfig.bUseNormalizedDrivingFX=false`에서는 현재 동작을 유지하고 true에서만 신규 normalized path를 사용한다.
- 신규 normalized path에는 `120 km/h` 같은 silent global fallback을 두지 않는다. `ReferenceMaxSpeedKmh`가 invalid하면 해당 Driving FX를 fail-safe exact0으로 평가한다.
- Vehicle Builder Benchmark 결과는 runtime Camera dependency가 아니라 `ReferenceMaxSpeedKmh` 작성·검수 evidence로 취급한다. 동일 saved Target + DefinitionHash에 결합된 accepted `PeakSpeedKmh`를 우선한다.
- Shared `DA_Cam_Default`의 normalized 활성화는 이를 resolve하는 in-scope production VehicleData 전부에 valid reference가 persisted된 뒤에만 허용한다.
- 기존 Product VehicleData는 raw edit하지 않고 현재 Vehicle Builder typed Authoring 경로에 `ReferenceMaxSpeedKmh` + `CameraPresentationDataOverride` exact2 field를 온보딩한 뒤 Preview/Review/Apply한다.
- per-vehicle Presentation CameraData는 `CameraTuningConfig`만 override하며 `DefaultAimProfile`은 override하지 않는다. Presentation migration이 Aim Gameplay를 변경하는 것은 금지한다.
- 기존 `CF-FQ-013 카메라/로컬 조준 고도화`의 완료 상태를 재오픈하지 않는다. CF-FQ-057은 Driving Presentation(주행 표현) 확장만 소유한다.
