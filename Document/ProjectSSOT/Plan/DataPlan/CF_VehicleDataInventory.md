# CarFight 차량 데이터 인벤토리 초안 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 기준 구현: `BP_CFVehiclePawn` / `CFVehiclePawn`
- 문서 목적: 현재 프로젝트에서 차량과 관련된 데이터가 어디에 퍼져 있는지 빠르게 파악하고, 무엇을 지금 바로 정리해야 하는지 초보자 기준으로 분류한다.
- 관련 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/00_Handover.md`
  - `Document/ProjectSSOT/01_Roadmap.md`

---

## 핵심 판단

현재 CarFight에서 차량 관련 데이터는 `BP_CFVehiclePawn`, `CFVehiclePawn`, `CFVehicleDriveComp`, `CFWheelSyncComp` 4곳만 보면 끝나는 구조가 아니다.

실제로는 아래 6개 층으로 보는 것이 맞다.

1. 차량 코어 클래스
2. 차량 데이터 에셋
3. VehicleMovement / Wheel 클래스 튜닝
4. 차량 입력 자산
5. 차량 표현 자산(메쉬/머티리얼/텍스처)
6. 차량 유틸 / 해석 규칙

즉, 이번 정리는 “클래스 4개 정리”가 아니라 **차량 데이터 생태계 전체 인벤토리 작성**으로 보는 것이 맞다.

---

## 지금 너무 멀리 보는지에 대한 판단

결론부터 말하면, 방향은 과하지 않다.

하지만 아래 둘은 구분해야 한다.

### 좋은 선행 고민
- 어떤 값이 차종 데이터인지 미리 구분하기
- 무엇을 C++ / DataAsset / BP / Debug가 소유해야 하는지 먼저 정하기
- 차가 2대, 3대가 되었을 때 복붙 지옥이 생기지 않게 최소 구조를 잡아두기

### 지금 단계에서는 과한 고민
- 시작부터 DataAsset 계층을 너무 잘게 쪼개기
- 차종 프리셋, 구동 프리셋, 휠 시각 프리셋, 카메라 프리셋, 입력 프리셋까지 한 번에 도입하기
- 아직 차가 1대인데 차량 데이터 상속 체계를 복잡하게 설계하기

따라서 권장 방향은 아래다.

> **지금은 작게 시작하되, 나중에 프리셋 분리가 가능하도록 루트 구조와 소유권만 먼저 맞춘다.**

즉, 현재 권장 최소 구조는 다음과 같다.

- 차종당 루트 차량 DataAsset 1개
- C++ 적용기 1개
- BP는 Thin BP 유지
- 반복값이 2회 이상 보일 때만 프리셋 DataAsset 분리

---

## 권장 최소 시작 구조

### 지금 바로 할 수준
- `UCFVehicleData`를 루트 차량 DataAsset 후보로 사용
- 차종 1대당 DataAsset 1개만 만든다
- 차체/휠 참조 + 레이아웃 + VehicleMovement 핵심값 + WheelVisual 핵심값만 넣는다
- 적용은 C++가 담당한다

### 나중에 필요할 때만 할 수준
- Movement 프리셋 분리
- WheelVisual 프리셋 분리
- 공용 휠 세팅 프리셋 분리
- 차종 계열별 공용 파생 구조 설계

즉, 지금은 **차량 1대 = 루트 DA 1개**로 시작하고, 나중에 값 중복이 명확해질 때만 하위 프리셋을 빼는 것이 가장 실용적이다.

---

## 차량 데이터 인벤토리 표

| 항목 | 현재 위치 | 종류 | 현재 역할 | 최종 소유자 권장 | 우선도 | 비고 |
|---|---|---|---|---|---|---|
| `BP_CFVehiclePawn` | `/Game/CarFight/Vehicles/BP_CFVehiclePawn` | 차량 코어 BP | 현재 기준 플레이 차량, Thin BP 조립 레이어 | BP | 높음 | 현재 기준 구현 |
| `BP_CFVehiclePawn_P0` | `/Game/CarFight/Vehicles/BP_CFVehiclePawn_P0` | 레거시/중간 BP | P0 단계 테스트/이전 흔적 가능성 | Legacy | 중간 | 기준 구현 아님 |
| `CFVehiclePawn` | `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 차량 코어 C++ | 현재 차량 Pawn 기준 클래스 | C++ | 높음 | DataAsset 적용기 진입점 후보 |
| `CFVehicleDriveComp` | `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h` | 차량 코어 C++ | 입력을 Chaos Vehicle Movement로 전달 | C++ | 높음 | 현재는 입력 전달 중심 |
| `CFWheelSyncComp` | `UE/Source/CarFight_Re/Public/CFWheelSyncComp.h` | 차량 코어 C++ | 휠 시각 동기화 / 디버그 / 비교 | C++ + Debug | 높음 | 차종값과 Debug값 분리 필요 |
| `CFModVehiclePawn` | `UE/Source/CarFight_Re/Public/CFModVehiclePawn.h` | 레거시 브리지 C++ | BP_ModularVehicle 시절 브리지 | Legacy | 낮음 | 현재 기준 구현 아님 |
| `VehicleData` 슬롯 | `CFVehiclePawn` Details | 데이터 슬롯 | 차량 DataAsset 참조 슬롯 | C++ + DataAsset | 높음 | 현재 타입이 `UPrimaryDataAsset*`라 느슨함 |
| `UCFVehicleData` | `UE/Source/CarFight_Re/Public/CFVehicleData.h` | 신규 차량 데이터 타입 | Native 차량 DataAsset 시작점 | DataAsset | 매우 높음 | 장기 공식 루트 후보 |
| `FCFVehicleWheelLayout` | `UE/Source/CarFight_Re/Public/CFVehicleData.h` | 신규 레이아웃 구조 | 바퀴 배치용 기본 데이터 | DataAsset | 높음 | 레거시 `S_WheelConfig` 대응 후보 |
| `PDA_VehicleArchetype` | `/Game/CarFight/Vehicles/PDA_VehicleArchetype` | 레거시 데이터 타입 | 기존 차량 시각 데이터 틀 | Legacy → DataAsset 전환 | 높음 | 현재 공식 경로 아님 |
| `DA_PoliceCar` | `/Game/CarFight/Data/Cars/DA_PoliceCar` | 레거시 차량 데이터 인스턴스 | 경찰차 데이터 보관 자산 | Legacy → 신규 DA 이관 | 높음 | 현재 `PDA_VehicleArchetype_C` 기반 |
| `S_WheelConfig` | `/Game/CarFight/Vehicles/S_WheelConfig` | 레거시 구조체 | 휠 배치/오프셋 데이터 | Legacy → 신규 Layout 통합 | 높음 | 대응표 필요 |
| `VehicleMovementComp` 내부 엔진 세팅 | `BP_CFVehiclePawn` 내 Chaos Vehicle Movement | 차종 튜닝 | 엔진/토크/RPM/기어/디퍼렌셜 | DataAsset + C++ 적용 | 매우 높음 | 우선 인벤토리 대상 |
| `VehicleMovementComp` 내부 조향 세팅 | 동일 | 차종 튜닝 | 조향각/조향 반응 | DataAsset + C++ 적용 | 매우 높음 | 차종별 차이 큼 |
| `VehicleMovementComp` 내부 브레이크 세팅 | 동일 | 차종 튜닝 | 브레이크/핸드브레이크 반응 | DataAsset + C++ 적용 | 매우 높음 | 차종별 차이 큼 |
| `VehicleMovementComp` 내부 휠/서스펜션 세팅 | 동일 | 차종 튜닝 | 휠 반경/폭/서스펜션/마찰 | DataAsset + C++ 적용 | 매우 높음 | 가장 먼저 표로 뽑아야 함 |
| `BP_Wheel_Front` | `/Game/CarFight/Vehicles/BP_Wheel_Front` | 부품 데이터 | 앞바퀴 클래스 | 부품 프리셋 또는 DataAsset 참조 | 높음 | 휠 개별값 포함 가능 |
| `BP_Wheel_Rear` | `/Game/CarFight/Vehicles/BP_Wheel_Rear` | 부품 데이터 | 뒷바퀴 클래스 | 부품 프리셋 또는 DataAsset 참조 | 높음 | 핸드브레이크/조향 여부 포함 가능 |
| `DefaultInputMappingContext` | `CFVehiclePawn` Details | 차량 시스템 공통 데이터 | 입력 매핑 컨텍스트 참조 | C++ / 공용 자산 | 중간 | 차종 데이터는 아님 |
| `InputAction_Throttle` 등 | `/Game/CarFight/Input/*` + `CFVehiclePawn` Details | 차량 시스템 공통 데이터 | 차량 입력 액션 참조 | C++ / 공용 자산 | 중간 | 차종 데이터는 아님 |
| `InputDeviceMode` | `CFVehiclePawn` Details | 시스템 정책 | 입력 장치 허용 정책 | C++ | 중간 | 차종 데이터로 두면 안 됨 |
| `bAutoInitializeOnBeginPlay` | `CFVehiclePawn` Details | 런타임 정책 | 초기화 자동화 토글 | C++ / Runtime | 낮음 | 차량 데이터 아님 |
| `bEnableWheelVisualTick` | `CFVehiclePawn` Details | 런타임 정책 | Tick에서 휠 갱신 호출 | C++ / Runtime | 낮음 | 차량 데이터 아님 |
| `ExpectedWheelCount` | `CFWheelSyncComp` | 차종값 후보 | 기대 휠 개수 | DataAsset 또는 C++ 규약 | 높음 | 차종값인지 규약인지 판정 필요 |
| `WheelAnchorComponentNames` | `CFWheelSyncComp` | 차종값 후보 / 규약 | 휠 Anchor 이름 목록 | C++ 규약 우선, 필요 시 DataAsset | 중간 | 표준 이름 강제 가능하면 DataAsset 불필요 |
| `WheelMeshComponentNames` | `CFWheelSyncComp` | 차종값 후보 / 규약 | 휠 Mesh 이름 목록 | C++ 규약 우선, 필요 시 DataAsset | 중간 | 예외 차량만 override 권장 |
| `FrontWheelCountForSteering` | `CFWheelSyncComp` | 차종값 후보 | 조향 대상 바퀴 수 | DataAsset | 높음 | 차종별 차이 가능 |
| `SteeringYawScaleDeg` | `CFWheelSyncComp` | 차종값 후보 | 조향 시각 보정 스케일 | DataAsset | 높음 | 차종별 차이 가능 |
| `SteeringVisualSign` | `CFWheelSyncComp` | 차종값 후보 | 조향 시각 부호 | DataAsset | 중간 | 메시 축 규칙에 따라 필요 |
| `SuspensionVisualSign` | `CFWheelSyncComp` | 차종값 후보 | 서스펜션 시각 부호 | DataAsset | 중간 | 메시/기준축 따라 달라질 수 있음 |
| `WheelSpinVisualSign` | `CFWheelSyncComp` | 차종값 후보 | 휠 스핀 시각 부호 | DataAsset | 중간 | 축 규칙 정리 후 판단 |
| `bVerboseLog` | `CFWheelSyncComp` | Debug | 상세 로그 출력 | Debug | 낮음 | 차종 데이터 아님 |
| `bEnableApplyTransformsInCpp` | `CFWheelSyncComp` | Debug/전환 | C++ 실제 적용 토글 | Debug | 낮음 | 릴리즈용 데이터 아님 |
| `bUseRealWheelTransformHelper` | `CFWheelSyncComp` | Debug/전환 | helper 사용 여부 | Debug | 낮음 | 차종 데이터 아님 |
| `bEnableHelperCompareMode` | `CFWheelSyncComp` | Debug/검증 | helper 비교 모드 | Debug | 낮음 | 검증 전용 |
| `SuspensionZDebugOffset` | `CFWheelSyncComp` | Debug | 강제 서스펜션 오프셋 | Debug | 낮음 | 차종 데이터로 승격 금지 |
| `WheelSpinPitchDebugDeg` | `CFWheelSyncComp` | Debug | 강제 스핀각 | Debug | 낮음 | 차종 데이터로 승격 금지 |
| `CarFightVehicleUtils::GetRealWheelTransform` | `UE/Source/CarFight_Re/Public/CarFightVehicleUtils.h` | 유틸/해석 규칙 | 실제 휠 상태 추출 helper | C++ | 중간 | 데이터 저장소는 아니지만 규칙 지점 |
| 경찰차 차체 메쉬 | `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body` | 표현 참조 자산 | 차체 시각 자산 | DataAsset 참조 | 높음 | 차종별 자산 참조 |
| 경찰차 휠 메쉬들 | `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_*` | 표현 참조 자산 | 휠 시각 자산 | DataAsset 참조 | 높음 | FL/FR/RL/RR 구분 자산 |
| 경찰차 머티리얼/텍스처 | `/Game/CarFight/Vehicles/police_car/Materials`, `/Textures` | 표현 자산 | 시각 표현 리소스 | 자산 자체 유지 | 중간 | DataAsset이 직접 다 들고 있을 필요는 없음 |

---

## 지금 바로 봐야 하는 우선순위

### 1순위
- `VehicleMovementComp` 내부 현재 값 목록
- `CFWheelSyncComp` 안에서 차종값 후보 / Debug값 분리
- `DA_PoliceCar` / `PDA_VehicleArchetype` / `S_WheelConfig` 대응표

### 2순위
- `BP_Wheel_Front`, `BP_Wheel_Rear`가 실제로 어떤 값을 들고 있는지 목록화
- `UCFVehicleData` 루트 구조 확장 초안 작성

### 3순위
- 입력 자산과 차량 공통 시스템 데이터 분리
- 표현 자산 참조 범위 최소화

---

## 지금 단계에서의 권장 구현 강도

초보자 기준으로는 아래 정도가 가장 적당하다.

### 지금 당장 해야 할 것
- 차량 루트 DataAsset 1개 구조 만들기
- 경찰차 1대만 신규 구조로 옮겨보기
- C++가 그 DataAsset을 읽는 적용 함수 만들기

### 아직 하지 않아도 되는 것
- Movement 프리셋 / WheelVisual 프리셋 / Layout 프리셋을 전부 분리하기
- 차량 계열별 상속 구조 만들기
- 차종별 파생 BP를 여러 개 만드는 것

즉, 지금은 **간단하게 구성하고**, 대신 **나중에 분리할 수 있는 방향으로 이름과 소유권만 맞춰두는 것**이 정답에 가깝다.

---

## 다음 권장 작업

1. `VehicleMovementComp` 현재 값을 실제 항목명 단위로 표 작성
2. `BP_Wheel_Front`, `BP_Wheel_Rear` 값 인벤토리 작성
3. `DA_PoliceCar` / `S_WheelConfig` → `UCFVehicleData` 대응표 작성
4. 그 다음에 `UCFVehicleData` 최소 루트 구조 확정
