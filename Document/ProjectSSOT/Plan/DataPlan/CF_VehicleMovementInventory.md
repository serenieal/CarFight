# CarFight VehicleMovement 데이터 인벤토리 초안 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 기준 구현: `BP_CFVehiclePawn` (`CFVehiclePawn` 기반)
- 문서 목적: `BP_CFVehiclePawn`의 `VehicleMovementComp`에서 관리 중일 가능성이 높은 차량 물리/휠/엔진/조향/브레이크/변속 계열 데이터를 인벤토리하고, 무엇을 DataAsset 대상으로 볼지 정리한다.
- 관련 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`
  - `Document/ProjectSSOT/00_Handover.md`
  - `Document/ProjectSSOT/01_Roadmap.md`

> 상태 주의 (2026-04-01):
> - 이 문서는 VehicleMovement 인벤토리 초안이다.
> - 본문에 남은 `FCFVehicleWheelLayout` 언급은 현재 코드 기준에서 삭제된 옛 설계 축으로 본다.
> - 현재 구현 판단은 `UCFVehicleData`의 실제 남아 있는 구조체와 활성 SSOT를 우선한다.

---

## 핵심 판단

현재 MCP 기반 에셋 덤프로는 `BP_CFVehiclePawn` 안의 `VehicleMovementComp`가 실제로 어떤 수치값을 들고 있는지 숫자 단위로 전부 확정하기 어렵다.

하지만 아래 2가지는 이미 확정할 수 있다.

1. `BP_CFVehiclePawn`는 현재 기준 구현이다.
2. 차량 물리 핵심 조정값의 공식 후보 위치 중 하나가 `VehicleMovementComp`라는 점은 매우 강하다.

따라서 이번 문서는 아래 원칙으로 작성한다.

- **MCP로 확정 가능한 항목**은 확정으로 적는다.
- **UE 에디터 Details를 직접 열어야 숫자가 확정되는 항목**은 "에디터 확인 필요"로 적는다.
- 지금 단계의 목적은 숫자 복사가 아니라 **무엇을 DataAsset 대상으로 봐야 하는지 분류하는 것**이다.

---

## 현재 확인 상태

### MCP로 확인된 사실
- 기준 차량 BP: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 부모 클래스: `CFVehiclePawn`
- 현재 차량 관련 휠 BP 존재:
  - `/Game/CarFight/Vehicles/BP_Wheel_Front`
  - `/Game/CarFight/Vehicles/BP_Wheel_Rear`
- 입력 자산 존재:
  - `/Game/CarFight/Input/IA_Throttle`
  - `/Game/CarFight/Input/IA_Steering`
  - `/Game/CarFight/Input/IA_Brake`
  - `/Game/CarFight/Input/IA_Handbrake`
  - `/Game/CarFight/Input/IMC_Vehicle_Default`
- 레거시 차량 데이터 자산 존재:
  - `/Game/CarFight/Vehicles/PDA_VehicleArchetype`
  - `/Game/CarFight/Data/Cars/DA_PoliceCar`
  - `/Game/CarFight/Vehicles/S_WheelConfig`
- 신규 Native 차량 데이터 타입 존재:
  - `UCFVehicleData`
  - `FCFVehicleWheelLayout`

### MCP만으로는 숫자 확정이 어려운 영역
- `VehicleMovementComp`의 엔진 수치
- `VehicleMovementComp`의 조향 수치
- `VehicleMovementComp`의 브레이크 / 핸드브레이크 수치
- `VehicleMovementComp`의 변속 / 기어비
- `VehicleMovementComp`의 휠별 세부 반경/폭/서스펜션
- `BP_Wheel_Front`, `BP_Wheel_Rear`의 구체적 기본값

즉, 현재 단계에서는 **항목 분류와 이전 우선순위를 먼저 문서화**하는 것이 맞다.

---

## VehicleMovement 인벤토리 표

| 항목 그룹 | 세부 항목 예시 | 현재 위치 | 현재 상태 | DataAsset 이전 후보 | 최종 소유자 권장 | 비고 |
|---|---|---|---|---|---|---|
| Wheel Setup 슬롯 구성 | FL/FR/RL/RR에 어떤 Wheel Class가 연결되는지 | `VehicleMovementComp` | 일부는 기존 문서로 확인됨 | 예 | DataAsset + C++ 적용 | 전/후륜 Wheel Class 참조는 차종 데이터 후보 |
| 전륜 Wheel Class | `BP_Wheel_Front` | `VehicleMovementComp` 참조 | 존재 확인 | 예 | DataAsset 참조 | 차종마다 다른 전륜 세팅 가능 |
| 후륜 Wheel Class | `BP_Wheel_Rear` | `VehicleMovementComp` 참조 | 존재 확인 | 예 | DataAsset 참조 | 차종마다 다른 후륜 세팅 가능 |
| 엔진 기본 세팅 | Max Torque, Max RPM, Idle RPM 등 | `VehicleMovementComp` Details | 에디터 확인 필요 | 매우 높음 | DataAsset + C++ 해석 | 차종별 차이 핵심 |
| 토크/출력 곡선 | Torque Curve 또는 등가 설정 | `VehicleMovementComp` Details | 에디터 확인 필요 | 매우 높음 | DataAsset + C++ 해석 | 스포츠카/경찰차 차별점 핵심 |
| 조향 세팅 | Max Steer Angle, Steering Response | `VehicleMovementComp` Details | 에디터 확인 필요 | 매우 높음 | DataAsset + C++ 해석 | 차체 반응감 핵심 |
| 브레이크 세팅 | Brake Torque, Brake Strength | `VehicleMovementComp` Details | 에디터 확인 필요 | 매우 높음 | DataAsset + C++ 해석 | 차종별 차이 큼 |
| 핸드브레이크 세팅 | Handbrake Torque / Handbrake Behavior | `VehicleMovementComp` Details | 에디터 확인 필요 | 매우 높음 | DataAsset + C++ 해석 | 드리프트 감각과 직접 연결 |
| 변속 세팅 | Gear Ratio, Final Ratio, Shift Timing | `VehicleMovementComp` Details | 에디터 확인 필요 | 높음 | DataAsset + C++ 해석 | 차종별 성격 차이 큼 |
| 디퍼렌셜 / 구동축 | FWD / RWD / AWD 계열 설정 | `VehicleMovementComp` Details | 에디터 확인 필요 | 높음 | DataAsset + C++ 해석 | 차종 정체성과 연결 |
| 휠 반경 | Radius | `BP_Wheel_Front/Rear` 또는 Movement 참조 | 에디터 확인 필요 | 높음 | 부품 프리셋 또는 DataAsset | 휠 BP와 루트 DA 중 어디가 기준인지 정해야 함 |
| 휠 폭 | Width | `BP_Wheel_Front/Rear` 또는 Movement 참조 | 에디터 확인 필요 | 높음 | 부품 프리셋 또는 DataAsset | 차종/부품 공용 여부 판정 필요 |
| 전륜 조향 여부 | 앞바퀴가 조향 대상인지 | `BP_Wheel_Front` | 에디터 확인 필요 | 중간 | 부품 프리셋 또는 C++ 규약 | 일반적으로 앞바퀴 공용 프리셋 가능 |
| 후륜 핸드브레이크 여부 | 뒷바퀴가 핸드브레이크 대상인지 | `BP_Wheel_Rear` | 에디터 확인 필요 | 중간 | 부품 프리셋 또는 C++ 규약 | 공용 후륜 프리셋 후보 |
| 서스펜션 세팅 | Spring/Damper/Travel 계열 | Wheel BP 또는 Movement | 에디터 확인 필요 | 높음 | 부품 프리셋 또는 DataAsset | 차종 차이와 휠 타입 차이 모두 가능 |
| 마찰 / 타이어 그립 | Friction / Lateral/Longitudinal Grip | Wheel BP 또는 Movement | 에디터 확인 필요 | 높음 | 부품 프리셋 또는 DataAsset | 차종 감각에 매우 중요 |
| 휠 BoneName / Offset | WheelSetup의 BoneName / Offset | `VehicleMovementComp` | 기존 문서 일부 확인 | 낮음~중간 | C++ 규약 또는 DataAsset | 현재 구조에선 None/0일 가능성 큼 |
| VehicleMass/차체 물리 | Mass/Center of Mass 유사 항목 | VehicleMovement 또는 Pawn 하위 물리 설정 | 에디터 확인 필요 | 중간~높음 | DataAsset + C++ 해석 | 차종별 무게감 핵심 가능성 |
| 공통 초기화 정책 | 언제 어떤 값으로 Movement를 적용하는지 | C++ | 코드 기준 | 아니오 | C++ | DataAsset에 넣지 말아야 함 |
| 입력 매핑 참조 | IA / IMC 자산 참조 | Pawn Details | 자산 존재 확인 | 아니오 | C++ / 공용 자산 | 차종 데이터 아님 |

---

## 지금 단계에서의 해석

### 1. VehicleMovement는 가장 먼저 정리해야 하는 데이터 층이다

차량 데이터 정리를 시작할 때 가장 먼저 봐야 하는 이유는 아래와 같다.

- 차종별 차이가 가장 많이 나는 값이 여기 있다.
- 플레이 감각을 직접 바꾸는 값이 여기 있다.
- 지금 상태로 차종이 늘어나면 복붙 가능성이 가장 큰 영역도 여기다.

즉, 차종 DataAsset 구조를 만든다고 했을 때, **VehicleMovement 세팅 계층을 안 건드리면 반쪽 정리**가 된다.

### 2. 휠 BP도 함께 봐야 한다

`BP_Wheel_Front`, `BP_Wheel_Rear`는 단순 참조 자산이 아니라, 실제로 휠 반경/폭/조향/핸드브레이크/서스펜션 계열을 들고 있을 가능성이 높다.

따라서 아래를 먼저 결정해야 한다.

- 휠 관련 기본값의 원본을 Wheel BP에 둘 것인가
- 루트 차량 DataAsset에 둘 것인가
- 공용 휠 프리셋 개념으로 분리할 것인가

현재 단계 권장은 아래다.

- **지금은 Wheel BP를 유지한다.**
- 대신 **Wheel BP가 어떤 값을 들고 있는지 먼저 표로 뽑는다.**
- 중복이 심하면 나중에 부품 프리셋 또는 DataAsset 하위 구조로 승격한다.

### 3. 지금은 프리셋 세분화까지는 가지 않는 것이 좋다

프로젝트가 아직 단순한 편이라면, 아래 정도가 적당하다.

- 루트 차량 DA 1개
- 전륜/후륜 Wheel BP 유지
- VehicleMovement 핵심값만 루트 DA에 수용
- C++가 루트 DA를 읽어 적용

즉, **처음부터 MovementPreset, WheelPreset, TirePreset까지 다 분리하지 않는다.**

반복되는 값이 실제로 보이기 시작할 때만 빼는 것이 좋다.

---

## UE 에디터에서 바로 확인해야 할 체크리스트

이 문서는 다음 체크리스트를 채우기 위한 준비 문서다.

### A. `BP_CFVehiclePawn` > `VehicleMovementComp`
- Wheel Setups 0~3 실제 값
- Engine 세팅
- Steering 세팅
- Brake / Handbrake 세팅
- Differential / Transmission 세팅
- Mass / 차체 관련 물리 세팅

### B. `BP_Wheel_Front`
- Radius
- Width
- Steer Enabled / Max Steer Angle
- Affected by Engine
- Affected by Handbrake
- Suspension / Friction 계열 값

### C. `BP_Wheel_Rear`
- Radius
- Width
- Steer Enabled 여부
- Handbrake 관련 값
- Suspension / Friction 계열 값

---

## 지금 문서 기준의 권장 결론

1. `VehicleMovementComp`는 차량 데이터 정리의 최우선 인벤토리 대상이다.
2. `BP_Wheel_Front`, `BP_Wheel_Rear`도 함께 인벤토리해야 한다.
3. 지금은 작게 시작하는 것이 맞다.
4. 다만 작게 시작하더라도, VehicleMovement 핵심값은 처음부터 DataAsset 후보로 분류해 두는 것이 좋다.
5. 수치가 반복되기 전까지는 프리셋 DataAsset 세분화는 보류해도 된다.

---

## 다음 권장 작업

1. `BP_CFVehiclePawn`의 `VehicleMovementComp`를 UE 에디터에서 열어 실제 수치 체크리스트 작성
2. `BP_Wheel_Front`, `BP_Wheel_Rear`의 실제 기본값 체크리스트 작성
3. 그 결과를 바탕으로 루트 `UCFVehicleData` 최소 구조 확정
4. 이후 경찰차 1대만 신규 구조로 시범 이관
