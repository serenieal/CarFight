# CarFight 최소 차량 DataAsset 골격안 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 기준 구현: `BP_CFVehiclePawn` / `CFVehiclePawn`
- 문서 목적: MCP에서 `VehicleMovementComp` 및 휠 Details 수치를 완전히 읽기 전까지, 지금 당장 구현해도 되는 최소 차량 DataAsset 골격을 정의한다.
- 관련 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleMovementInventory.md`

> 상태 주의 (2026-04-01):
> - 이 문서는 초기 골격 설계 기록이다.
> - 본문에 남아 있는 `FCFVehicleWheelLayout / WheelLayout / AutoFit` 언급은 현재 구현 기준이 아니다.
> - 현재 코드 기준은 레거시 레이아웃 구조 삭제 완료이며, 실제 운영 기준은 `수동 Wheel_Anchor 배치 + WheelRadius`다.

---

## 핵심 결론

지금 단계에서는 차량 데이터 시스템을 완성형으로 만들 필요가 없다.

대신 아래 목표만 달성하면 충분하다.

1. `UCFVehicleData`를 **공식 루트 차량 DataAsset 후보**로 고정한다.
2. `CFVehiclePawn`가 장기적으로 이 DataAsset을 읽는 구조로 방향을 고정한다.
3. 세부 튜닝 숫자 없이도 **차종 자산 참조 / 기본 레이아웃 / 최소한의 Movement 슬롯 / 최소한의 WheelVisual 슬롯**을 담을 수 있게 만든다.
4. 나중에 MCP에서 실제 수치를 읽을 수 있게 되면, 기존 구조를 깨지 않고 하위 항목만 확장할 수 있게 만든다.

즉, 지금 필요한 것은 **완성형 데이터 시스템**이 아니라 **확장 가능한 최소 골격**이다.

---

## 지금 단계의 설계 원칙

### 1. 루트는 하나만 둔다

현재 단계에서는 차량 1대당 루트 DataAsset 1개만 두는 것이 맞다.

예시:

- `DA_Car_Police01`
- 나중에 차종이 늘면 `DA_Car_Sports01`, `DA_Car_Offroad01` 식으로 추가

중요한 점은, 지금은 프리셋 DataAsset을 여러 개 만들지 않는 것이다.

- MovementPreset
- WheelVisualPreset
- TirePreset
- CameraPreset

이런 것들은 **지금 단계에서는 보류**한다.

### 2. 상위 카테고리만 먼저 고정한다

세부 숫자를 아직 모르더라도, 상위 카테고리 이름은 지금 먼저 고정해도 된다.

권장 상위 카테고리:

- `Visual`
- `Layout`
- `Movement`
- `WheelVisual`
- `References`

이렇게 잡아두면 나중에 값이 늘어나도 위치가 흔들리지 않는다.

### 3. 숫자가 확정되지 않은 곳은 슬롯만 만든다

예를 들어 지금은 아래 값을 정확히 못 본다.

- 엔진 토크
- 기어비
- 브레이크 수치
- 휠 반경 / 폭
- 서스펜션 수치

그렇다면 지금은 이 숫자를 억지로 넣지 말고, **나중에 채울 구조체 슬롯만 준비**하는 것이 맞다.

### 4. C++가 해석 책임을 가진다

지금 단계에서도 이 원칙은 바뀌지 않는다.

- DataAsset은 저장소
- C++는 해석 및 적용
- BP는 표현/조립

즉, 지금 골격안은 단순 저장 구조만 먼저 만들고, C++가 나중에 이 구조를 읽는 쪽으로 확장 가능한 형태여야 한다.

---

## 최소 루트 구조 제안

현재 단계 권장 구조는 아래와 같다.

```text
UCFVehicleData
├── Visual
├── Layout
├── Movement
├── WheelVisual
└── References
```

이때 중요한 점은, 모든 카테고리를 세부 구현까지 완성할 필요는 없다는 것이다.

지금은 각 카테고리가 아래 정도 역할만 가지면 충분하다.

### Visual
- 차체 메쉬 참조
- 휠 메쉬 참조 FL / FR / RL / RR

### Layout
- 기본 휠 레이아웃 구조
- 현재 있는 `FCFVehicleWheelLayout`를 활용하거나 확장

### Movement
- 나중에 VehicleMovement 수치를 넣을 자리
- 지금은 최소 슬롯만 정의

### WheelVisual
- 나중에 WheelSync 차종 데이터를 넣을 자리
- 지금은 최소 슬롯만 정의

### References
- 전륜 / 후륜 Wheel Class 참조
- 필요 시 보조 자산 참조

---

## 지금 바로 넣어도 되는 필드

아래는 세부 수치가 없어도 지금 넣어도 무리가 적은 필드다.

### Visual 계층
- `ChassisMesh`
- `WheelMeshFL`
- `WheelMeshFR`
- `WheelMeshRL`
- `WheelMeshRR`

이 값들은 실제 자산 참조라서 지금도 비교적 확실하다.

### Layout 계층
- `WheelLayout`
  - 현재 `FCFVehicleWheelLayout` 사용 또는 최소 확장

이 값도 현재 구조에서 자연스럽게 이어질 수 있다.

### References 계층
- `FrontWheelClass`
- `RearWheelClass`

현재 프로젝트에 `BP_Wheel_Front`, `BP_Wheel_Rear`가 실제 존재하므로, 이 참조 슬롯은 지금 만들어도 된다.

### Movement 계층
지금은 실제 수치를 다 넣지 말고 아래 정도만 준비한다.

- `bUseMovementOverrides`
- `MovementProfileName` 또는 식별용 이름
- 비어 있는 최소 구조체 슬롯

즉, 숫자가 아니라 **나중에 수용할 자리**만 만든다.

### WheelVisual 계층
지금은 아래 정도만 준비한다.

- `ExpectedWheelCount`
- `FrontWheelCountForSteering`
- `bUseWheelVisualOverrides`

그리고 세부 보정값은 나중에 확장한다.

---

## 지금은 넣지 않는 것이 좋은 필드

아래는 지금 억지로 넣지 않는 것이 좋다.

### Movement 세부 숫자
- Max Torque
- Max RPM
- Idle RPM
- Gear Ratio 배열
- Brake Torque
- Handbrake Torque
- Steering Curve 계열
- Differential 세부 설정
- 휠별 Suspension 수치
- 타이어 마찰 수치

이유는 아직 실제 기준값을 안정적으로 읽지 못하기 때문이다.

### WheelVisual 세부 보정 숫자
- SteeringYawScaleDeg
- SteeringVisualSign
- SuspensionVisualSign
- WheelSpinVisualSign
- 차종별 상세 오프셋

이 값들도 지금은 자리만 생각하고 숫자 확정은 나중으로 미루는 것이 좋다.

### Debug 관련 필드
아래 계열은 절대 루트 차량 DataAsset에 넣지 않는다.

- helper compare 토글
- verbose log
- 강제 스핀 각도
- 강제 서스펜션 오프셋
- 단일 축 테스트 토글

이 값들은 계속 `CFWheelSyncComp`의 Debug 영역에 남겨야 한다.

---

## 최소 구조 예시

아래는 개념 예시다.

```text
UCFVehicleData
├── Visual
│   ├── ChassisMesh
│   ├── WheelMeshFL
│   ├── WheelMeshFR
│   ├── WheelMeshRL
│   └── WheelMeshRR
├── Layout
│   └── WheelLayout
├── Movement
│   ├── bUseMovementOverrides
│   └── ReservedMovementConfig
├── WheelVisual
│   ├── ExpectedWheelCount
│   ├── FrontWheelCountForSteering
│   └── bUseWheelVisualOverrides
└── References
    ├── FrontWheelClass
    └── RearWheelClass
```

여기서 `ReservedMovementConfig`는 지금 당장 완성하는 구조체가 아니라, **나중에 실제 수치 이관 시 채울 영역**으로 두는 개념이다.

---

## 권장 구현 수준

지금 단계에서 구현 강도는 아래 정도가 적당하다.

### 지금 해도 되는 수준
- `UCFVehicleData`에 상위 카테고리 추가
- Visual / Layout / References는 실제 필드 추가
- Movement / WheelVisual은 최소 슬롯만 추가
- `CFVehiclePawn`에 이 DataAsset을 받을 방향 유지

### 아직 하지 않는 수준
- `CFVehiclePawn`가 Movement 세부 수치를 전부 파싱해 적용하는 코드
- `VehicleMovementComp` 전 필드 자동 이관 로직
- Wheel BP와 DataAsset 사이의 완전한 동기화 시스템
- 프리셋 DA 계층 분리

즉, 지금은 **데이터 저장 구조의 골격만 구현**하고, **실제 세부 적용기는 다음 단계로 미룬다**.

---

## 나중에 어떻게 확장할 수 있는가

MCP가 디테일 수치를 읽을 수 있게 되면 아래 순서로 확장하면 된다.

### 1단계
`Movement` 하위 구조체 구체화

예:
- Engine
- Steering
- Brake
- Transmission
- Differential
- WheelPhysics

### 2단계
`WheelVisual` 하위 구조체 구체화

예:
- SteeringVisual
- SuspensionVisual
- SpinVisual
- NameMapping 또는 규약 정보

### 3단계
중복이 보이면 그때 프리셋 분리

예:
- 여러 차량이 같은 구동 세팅을 공유한다
- 여러 차량이 같은 휠 시각 보정을 공유한다

이때만 프리셋 DA를 빼는 것이 맞다.

즉, 현재 골격은 **프리셋 분리 이전 단계의 안정된 기본형**으로 이해하면 된다.

---

## 현재 단계 권장 결론

지금 당장 구현해도 되는 최소 골격안은 아래로 정리할 수 있다.

1. `UCFVehicleData`를 루트 차량 DA로 사용한다.
2. `Visual`, `Layout`, `Movement`, `WheelVisual`, `References` 상위 계층만 먼저 고정한다.
3. `Visual`, `Layout`, `References`는 실제 필드를 먼저 넣는다.
4. `Movement`, `WheelVisual`은 세부 숫자 대신 확장 슬롯만 만든다.
5. `CFVehiclePawn`가 장기적으로 이 DataAsset을 공식 입력으로 받는 방향만 유지한다.
6. 세부 숫자 이관과 C++ 적용 확장은 MCP 수치 가시화 완료 후 재개한다.

이 방식이면 지금 프로젝트 규모에 비해 과하지 않으면서도, 나중에 구조를 다시 갈아엎지 않고 이어서 확장할 수 있다.

---

## 다음 권장 작업

1. `UCFVehicleData` 최소 골격 필드 확정
2. `CFVehiclePawn`의 `VehicleData` 슬롯 공식화 방향 정리
3. 경찰차 1대에 대해 Visual / Layout / References만 먼저 연결
4. MCP 수치 가시화 완료 후 Movement / WheelVisual 세부 확장 재개
