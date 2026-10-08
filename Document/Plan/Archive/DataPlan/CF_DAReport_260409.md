# CarFight 구현 기준 DataAsset 보고서 v1.0

- 작성일: 2026-04-09
- 대상 프로젝트: `UE/CarFight_Re`
- 작성 기준: 현재 프로젝트에 실제 구현된 `UCFVehicleData` 중심 구조와 이를 소비하는 C++ 파이프라인
- 검토 범위:
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleData.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Content/CarFight/Vehicles/*`
- 주의:
  - 본 보고서는 **현재 구현된 DataAsset 중심**으로 작성했다.
  - UE 에셋 상세 덤프(`DA_PoliceCar` 내부 필드)는 세션 타임아웃으로 직접 확정하지 못했다.
  - 따라서 에셋 내부 값은 추정하지 않고, **C++ 소비 구조와 파일 배치로 검증 가능한 내용만 기록**한다.

---

## 1. 핵심 결론

현재 프로젝트의 DataAsset 기준 구현은 **`UCFVehicleData`를 루트 차량 DataAsset으로 삼고, `ACFVehiclePawn`이 이를 읽어 차량 시각/이동/참조/휠 시각/DriveState 설정에 순차 적용하는 구조**로 정리돼 있다.

즉, 지금 프로젝트에서 실제 운영 기준으로 볼 수 있는 DataAsset 구조는 아래와 같다.

```text
UCFVehicleData
├── VehicleVisualConfig
├── VehicleMovementConfig
├── WheelVisualConfig
├── VehicleReferenceConfig
└── DriveStateConfig
```

이 구조는 문서상 목표였던 "차종 데이터는 DataAsset이 소유하고, 해석과 적용은 C++가 소유한다"는 방향을 **부분 설계 수준이 아니라 실제 적용 코드 수준으로 구현한 상태**다.

다만, 현재 구조는 완성형 최종 구조라기보다 **운영 가능한 1차 구현본**에 가깝다. `Layout`, `ValidationMeta`, 세분화된 `WheelVisual` 보정 데이터는 아직 충분히 루트 DataAsset로 올라오지 않았고, 일부 차종별 값은 여전히 `CFWheelSyncComp` 또는 런타임 컴포넌트 쪽에 남아 있다.

---

## 2. 현재 프로젝트에서 확인된 DataAsset 자산 배치

`UE/Content/CarFight/Vehicles` 기준으로 현재 확인된 차량 데이터 계층은 다음과 같다.

### 2.1 구현 기준 자산
- `UE/Content/CarFight/Vehicles/Data/Cars/DA_PoliceCar.uasset`

### 2.2 현재 공존 중인 레거시 자산
- `UE/Content/CarFight/Vehicles/PDA_VehicleArchetype.uasset`
- `UE/Content/CarFight/Vehicles/S_WheelConfig.uasset`

### 2.3 관련 런타임/참조 자산
- `UE/Content/CarFight/Vehicles/BP_CFVehiclePawn.uasset`
- `UE/Content/CarFight/Vehicles/BP_Wheel_Front.uasset`
- `UE/Content/CarFight/Vehicles/BP_Wheel_Rear.uasset`

### 2.4 시각 자산 배치
- `UE/Content/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body.uasset`
- `UE/Content/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_FL.uasset`
- `UE/Content/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_FR.uasset`
- `UE/Content/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_RL.uasset`
- `UE/Content/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_RR.uasset`

이 배치를 기준으로 보면, 현재 프로젝트는 **차량 시각 자산과 차종 DataAsset 자산이 분리된 상태**이며, `DA_PoliceCar`가 그 중간의 루트 데이터 자산 역할을 맡도록 설계돼 있다.

---

## 3. `UCFVehicleData`가 현재 실제로 소유하는 데이터

`CFVehicleData.h` 기준으로 `UCFVehicleData`는 `UPrimaryDataAsset` 기반이며, 현재 아래 5개 상위 설정 구조를 가진다.

### 3.1 `VehicleVisualConfig`
차량 시각 자산 참조를 소유한다.

포함 항목:
- `ChassisMesh`
- `WheelMeshFL`
- `WheelMeshFR`
- `WheelMeshRL`
- `WheelMeshRR`

역할:
- 차체와 4개 바퀴의 Static Mesh 자산 참조를 보관한다.
- `CFVehiclePawn`의 시각 컴포넌트에 직접 적용되는 입력값이다.

### 3.2 `VehicleMovementConfig`
현재 구현 기준에서 가장 많은 실제 차종 수치를 소유한다.

포함 항목 그룹:
- 전/후륜 브레이크/핸드브레이크
- 전/후륜 반지름/폭
- 전/후륜 마찰, 코너링 강성, 하중 비율
- 전/후륜 스프링, 프리로드, 서스펜션 Raise/Drop
- 전/후륜 엔진 영향 여부
- SweepShape
- 차체 높이, Drag, Downforce
- 중심질량 오버라이드
- 엔진 토크/RPM/Idle/BrakeEffect/RevUp/RevDown
- DifferentialType, FrontRearSplit
- SteeringType, SteeringAngleRatio
- `bLegacyWheelFrictionPosition`
- 전/후륜 `AdditionalOffset`
- `MovementProfileName`
- `bUseMovementOverrides`

역할:
- 차종별 VehicleMovement 기본값을 DataAsset에 저장한다.
- Wheel Class 기본값과 런타임 Wheel setter를 동시에 구동하는 핵심 입력 데이터다.

### 3.3 `WheelVisualConfig`
현재는 최소 구성만 구현돼 있다.

포함 항목:
- `bUseWheelVisualOverrides`
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`

역할:
- 휠 시각 동기화 단계에서 필요한 최소 메타 데이터를 전달한다.
- 아직 조향 부호, 스핀 축, 서스펜션 보정 등은 포함하지 않는다.

### 3.4 `VehicleReferenceConfig`
차량이 사용할 Wheel Class 참조를 소유한다.

포함 항목:
- `FrontWheelClass`
- `RearWheelClass`

역할:
- 전륜/후륜 Wheel Class를 VehicleMovement의 `WheelSetups`에 적용하는 기준 참조다.
- 동시에 Wheel Class CDO에 DataAsset 수치를 임시 반영하는 경로의 진입점이다.

### 3.5 `DriveStateConfig`
차량별 주행 상태 판정 임계값 구성을 소유한다.

역할:
- `VehicleDriveComp`에 전달되는 DriveState 판정 설정이다.
- 현재 프로젝트에서 DataAsset이 단순 물리값뿐 아니라 상태 판정 설정까지 포함해 확장되고 있음을 보여준다.

---

## 4. `UCFVehicleData`를 실제로 소비하는 C++ 경로

현재 구현의 핵심은 **DataAsset이 단순 저장소가 아니라, `ACFVehiclePawn`에서 실제로 소비되고 적용된다는 점**이다.

`CFVehiclePawn.h` 기준으로 Pawn은 아래처럼 `VehicleData`를 직접 소유한다.

- `TObjectPtr<UCFVehicleData> VehicleData`

즉, 현재 기준 구현은 이미 `UCFVehicleData`를 **공식 타입으로 고정**했다.

### 4.1 적용 진입점
`ACFVehiclePawn::ApplyVehicleDataConfig()`

이 함수는 현재 아래 순서로 DataAsset 내용을 적용한다.

1. `ApplyVehicleVisualConfig()`
2. `ApplyVehicleLayoutConfig()`
3. `ApplyVehicleMovementConfig()`
4. `ApplyVehicleReferenceConfig()`
5. `ApplyVehicleWheelPhysicsConfig()`
6. `ApplyVehicleWheelVisualConfig()`
7. `VehicleDriveComp->ApplyDriveStateConfig()`

이 순서는 곧 **현재 프로젝트의 DataAsset 소비 파이프라인**이라고 볼 수 있다.

---

## 5. 설정별 실제 적용 상태

### 5.1 Visual 적용: 구현됨
`ApplyVehicleVisualConfig()`는 다음 이름의 컴포넌트를 찾아 DataAsset의 시각 자산을 넣는다.

대상 컴포넌트 이름:
- `SM_Chassis`
- `Wheel_Mesh_FL`
- `Wheel_Mesh_FR`
- `Wheel_Mesh_RL`
- `Wheel_Mesh_RR`

적용 방식:
- `VehicleVisualConfig`의 메쉬를 각 컴포넌트에 `SetStaticMesh()`로 반영
- 비어 있는 FR/RL/RR은 FL 메쉬 재사용 가능

판정:
- **실구현 완료**
- 시각 계층은 이미 DataAsset 중심 구조로 동작 중이다.

### 5.2 Layout 적용: 형식만 존재
`ApplyVehicleLayoutConfig()`는 현재 실질 계산 없이 아래 요약만 남긴다.

- `VehicleLayout: ManualAnchorLayout=Required`

판정:
- **미구현에 가까운 최소 스텁 상태**
- 문서상 Layout 축은 존재하지만, 현재 루트 DataAsset 구조나 실제 적용 로직으로 완성되진 않았다.

### 5.3 VehicleMovement 본체 적용: 구현됨
`ApplyVehicleMovementConfig()`는 `VehicleMovementComp` 본체 필드에 직접 값을 넣는다.

실제 반영되는 대표 항목:
- `ChassisHeight`
- `DragCoefficient`
- `DownforceCoefficient`
- `bEnableCenterOfMassOverride`
- `CenterOfMassOverride`
- `bLegacyWheelFrictionPosition`
- 전/후륜 `AdditionalOffset`
- `EngineSetup.*`
- `DifferentialSetup.*`
- `SteeringSetup.*`

판정:
- **실구현 완료**
- 현재 DataAsset 기반 이동 수치는 단순 보관이 아니라 실제 Chaos Vehicle 설정에 주입된다.

### 5.4 Wheel Class 참조 적용: 구현됨
`ApplyVehicleReferenceConfig()`는 다음을 수행한다.

- `FrontWheelClass` / `RearWheelClass`를 WheelSetup에 할당
- 조건 충족 시 Wheel Class CDO를 가져옴
- DataAsset의 `VehicleMovementConfig` 값을 Wheel Class 기본값에 임시 반영
- `RecreatePhysicsState()` 호출
- 이후 Wheel Class CDO 원상복구

의미:
- 현재 프로젝트는 Wheel Class를 단순 참조만 하지 않고,
- **DataAsset 수치를 Wheel Class 초기값과 실제 인스턴스 재생성 경로에 연결**하고 있다.

판정:
- **실구현 완료**
- 현재 구조에서 DataAsset 중심 운영의 핵심 증거 중 하나다.

### 5.5 런타임 휠 물리 setter 적용: 구현됨
`ApplyVehicleWheelPhysicsConfig()`는 런타임 Wheel 인스턴스 기준으로 setter를 호출한다.

전륜 적용 예:
- 최대 조향각
- 최대 브레이크 토크
- 반지름
- 마찰 배수
- 엔진 영향 여부
- 서스펜션 파라미터

후륜 적용 예:
- 최대 브레이크 토크
- 핸드브레이크 토크
- 반지름
- 마찰 배수
- 엔진 영향 여부
- 서스펜션 파라미터

판정:
- **실구현 완료**
- 현재 프로젝트는 VehicleMovement DataAsset 값을 CDO 초기값 수준에서 끝내지 않고, 런타임 휠 인스턴스까지 적용한다.

### 5.6 WheelVisual 적용: 최소 구현
`ApplyVehicleWheelVisualConfig()`는 현재 다음 2개만 `WheelSyncComp`에 전달한다.

- `ExpectedWheelCount`
- `FrontWheelCountForSteering`

판정:
- **부분 구현**
- WheelVisual 계층은 존재하지만, 현재는 메타 수준 최소 데이터만 올라와 있다.

### 5.7 DriveState 적용: 구현됨
`ApplyVehicleDataConfig()` 마지막 단계에서:

- `VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig)`

를 호출한다.

판정:
- **실구현 완료**
- DataAsset이 물리값뿐 아니라 주행 상태 판정값까지 공급한다.

---

## 6. 레거시 데이터와의 관계

현재 프로젝트는 신규 루트 DataAsset 구조만 단독으로 존재하는 상태가 아니라, **레거시 데이터와 공존 중인 전환기 구조**다.

### 6.1 공존 자산
- `PDA_VehicleArchetype`
- `S_WheelConfig`
- `DA_PoliceCar`

### 6.2 코드 차원의 레거시 흡수
`UCFVehicleData::PostLoad()`는 과거 실험용 VehicleMovement 세트를 감지하면, 현재 프로젝트 기준 기본값으로 자동 보정한다.

의미:
- 레거시 자산/실험값이 남아 있어도 신규 루트 DataAsset 운영 기준으로 최대한 흡수하려는 장치다.

판정:
- **브리지 구현 존재**
- 완전 정리 전 단계지만, 최소한의 마이그레이션 안전장치는 이미 구현됐다.

---

## 7. 현재 DataAsset 구조의 장점

### 7.1 차종별 수치의 공식 입력점이 생겼다
현재는 `VehicleMovementComp` Details만 직접 만지는 구조가 아니라, `UCFVehicleData`를 통해 차종 수치가 공급된다.

### 7.2 C++가 해석과 적용을 소유한다
DataAsset은 값을 저장하고, 실제 해석과 적용은 `CFVehiclePawn`이 담당한다.

이는 현재 프로젝트가 목표로 하는:
- C++ 중심 규칙 해석
- Thin BP 유지
- DataAsset 기반 차종 분리

방향과 일치한다.

### 7.3 차종 확장 가능성이 생겼다
현재 구조는 경찰차 1대 기준으로 보이더라도, 구조적으로는 같은 `UCFVehicleData` 인스턴스를 늘려 다른 차종을 확장할 수 있다.

### 7.4 레거시 흡수 경로가 있다
`PostLoad()` 자동 보정은 임시 실험값과 현재 기준값 사이를 이어주는 최소한의 안전장치다.

---

## 8. 현재 DataAsset 구조의 한계

### 8.1 `Layout`이 사실상 아직 없다
문서상 중요했던 Layout 축은 현재 구현상 `ManualAnchorLayout=Required` 수준의 선언만 존재한다.

### 8.2 `WheelVisualConfig`가 매우 얇다
현재는 휠 개수와 조향 전륜 개수 정도만 올라와 있고,
- 조향 부호
- 스핀 부호
- 서스펜션 부호
- 이름 매핑
- 차종별 시각 보정
등은 아직 루트 DataAsset로 충분히 이동하지 않았다.

### 8.3 일부 차종 데이터가 여전히 `CFWheelSyncComp` 쪽에 남아 있을 가능성이 높다
현재 구조상 `WheelSyncComp`는 여전히 여러 시각 보정/디버그 값을 들고 있으며, 이 중 일부는 장기적으로 DataAsset 후보가 될 수 있다.

### 8.4 에셋 인스턴스 필드 검증은 추가 확인이 필요하다
이번 세션에서는 `DA_PoliceCar` 내부 필드를 직접 덤프하지 못했다.

따라서 다음은 이번 보고서에서 확정하지 않았다.
- `DA_PoliceCar`에 실제 어떤 값이 채워져 있는가
- `BP_CFVehiclePawn`가 실제로 어느 `VehicleData`를 참조 중인가
- Wheel BP와 DataAsset 값이 현 시점에 정확히 일치하는가

---

## 9. 현재 구현 기준 최종 판정

현재 프로젝트의 DataAsset 구현 상태는 아래처럼 판정한다.

### 9.1 완료
- `UCFVehicleData` 루트 타입 정의
- `VehicleData`의 공식 타입 고정
- Visual/Movement/Reference/DriveState 소비 파이프라인
- VehicleMovement 본체 적용
- Wheel Class 참조 및 임시 튜닝 적용
- 런타임 휠 setter 적용
- 레거시 실험값 자동 보정

### 9.2 부분 완료
- WheelVisualConfig
- 레거시 → 신규 구조 이행
- 다차종 확장 기반

### 9.3 미완성
- Layout 실구현
- ValidationMeta 계층
- 충분히 세분화된 WheelVisual 차종 데이터 계층
- 에셋 인스턴스 기준의 완전 검증

---

## 10. 실무 판단

현재 프로젝트는 이미 **"DataAsset을 도입했다" 수준을 넘어서, `UCFVehicleData`를 실제 차량 구성의 운영 입력으로 쓰는 단계**에 도달해 있다.

즉, 보고서 기준으로 현재 DataAsset 구조는 다음처럼 정의할 수 있다.

> **CarFight의 현재 구현 기준 차량 데이터 구조는 `UCFVehicleData` 루트 DataAsset을 중심으로 하며, `ACFVehiclePawn`이 이를 읽어 시각 자산, VehicleMovement 본체, Wheel Class 참조, 런타임 휠 물리, DriveState 설정을 C++에서 순차 적용하는 구조다.**

다만 이 구조는 아직 최종 완성형은 아니다. 현재 단계는 **루트 DataAsset 기반 운영 구조는 성립했지만, Layout / Validation / 확장 WheelVisual 계층이 추가되어야 하는 1차 완료 상태**로 보는 것이 가장 정확하다.

---

## 11. 다음 권장 작업

1. `DA_PoliceCar` 내부 필드 실측 덤프 재시도
2. `BP_CFVehiclePawn`의 실제 `VehicleData` 연결 검증
3. `WheelVisualConfig`에 차종별 보정 데이터 추가 여부 결정
4. `Layout` 계층을 실제 구조체/적용 코드로 승격
5. ValidationMeta 축 도입 여부 검토
