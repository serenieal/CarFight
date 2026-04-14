# CarFight 차량 데이터 소유권 정리 + 차량 DataAsset 총괄 관리 계획서 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 기준 구현: `BP_CFVehiclePawn` (`CFVehiclePawn` 기반)
- 대상 Native 코어: `CFVehiclePawn` + `CFVehicleDriveComp` + `CFWheelSyncComp`
- 문서 목적: 현재 프로젝트에 흩어진 차량 관련 수치와 자산 참조의 소유권을 재정의하고, 차종이 늘어나도 `DataAsset` 기준으로 차량별 세팅을 총괄 관리할 수 있는 구조를 계획한다.
- 관련 문서:
  - `Document/Document_Entry.md`
  - `Document/ProjectSSOT/00_Handover.md`
  - `Document/ProjectSSOT/01_Roadmap.md`
  - `Document/ProjectSSOT/Plan/CarFight_CPP_RePlan.md`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
  - `UE/Source/CarFight_Re/Public/CFWheelSyncComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`

> 상태 주의 (2026-04-01):
> - 이 문서는 차량 데이터 총괄 계획의 초기 기록이다.
> - 본문 일부에는 현재 기준선에서 제거된 레거시 레이아웃 설계 흔적이 남아 있을 수 있다.
> - 현재 운영 기준선은 `수동 Wheel_Anchor 배치 + WheelRadius`다.

---

## 핵심 결론

이번 주제는 단순히 값을 `DataAsset`로 옮겨서 편하게 만들자는 작업이 아니다.

이번 계획의 핵심은 아래 4가지다.

1. **현재 기준 구현을 명확히 고정한다.**  
   현재 기준 플레이 차량은 `BP_CFVehiclePawn`이며, 기준 Native 코어는 `CFVehiclePawn` + `CFVehicleDriveComp` + `CFWheelSyncComp`이다. `BP_ModularVehicle`는 이미 삭제되었고, 지금 시점에서 기준 구현으로 취급하면 안 된다.

2. **차량 데이터의 공식 경로를 다시 정의한다.**  
   현재 `VehicleData`, `PDA_VehicleArchetype`, `DA_PoliceCar`, `S_WheelConfig`는 프로젝트 안에 남아 있지만, `C++`가 이들을 프로젝트의 공식 데이터 경로로 적극 해석하는 상태는 아니다. 즉, 지금은 데이터가 존재해도 “누가 책임지고 읽고 적용하는가”가 불명확하다.

3. **C++가 규칙과 해석을 가져가고, DataAsset은 차종 저장소가 되어야 한다.**  
   프로젝트 전체가 C++ 중심 구조로 이동하는 방향이라면, 차량도 같은 규칙을 따라야 한다. 값의 저장 위치만 `DataAsset`으로 바꾸고, 해석 규칙이 다시 BP나 Details에 남으면 구조 정리는 실패한 것이다.

4. **BP는 얇은 조립/표현 계층으로 유지해야 한다.**  
   `BP_CFVehiclePawn`는 현재도 EventGraph / Construction Script에 실질 로직이 거의 없는 Thin BP에 가깝다. 이 방향을 유지하면서, 차량별 차이는 `DataAsset`에서 바꾸고, C++가 그것을 읽어서 적용하는 구조로 정리해야 한다.

정리하면, 이번 계획의 최종 목표는 아래 한 문장으로 요약할 수 있다.

> **차종별 수치와 자산 참조는 차량 전용 DataAsset이 소유하고, 그 값을 언제 어떻게 해석해 Chaos Vehicle / 휠 시각 / 런타임 세팅에 반영할지는 C++가 소유하며, BP는 조립과 표현만 담당하는 구조로 고정한다.**

---

## 현재 상태 해석

## 1. 왜 이 주제를 지금 분리해서 다뤄야 하는가

지금 이 주제를 별도 계획서로 분리해야 하는 이유는, 현재 차량 작업의 병목이 **주행 로직 자체보다 데이터 소유권**에 있기 때문이다.

현재 프로젝트는 이미 아래 방향으로 이동 중이다.

- 기준 구현이 `BP_ModularVehicle`에서 `BP_CFVehiclePawn`으로 넘어왔다.
- 기준 Native 코어가 `CFVehiclePawn` + `CFVehicleDriveComp` + `CFWheelSyncComp`로 정리되기 시작했다.
- `BP_CFVehiclePawn`는 얇은 BP로 유지하려는 방향이 분명하다.
- `UCFVehicleData`라는 신규 Native DataAsset 타입도 이미 생성되어 있다.

하지만 아직 아래 문제가 남아 있다.

- 어떤 값은 `BP_CFVehiclePawn` Details에 있다.
- 어떤 값은 `VehicleMovementComp` 안에 직접 박혀 있다.
- 어떤 값은 `CFWheelSyncComp`/`WheelSyncComp`의 디버그용 값과 차종 데이터가 섞여 있다.
- 어떤 값은 레거시 `PDA_VehicleArchetype` / `DA_PoliceCar` / `S_WheelConfig`에 남아 있다.
- 신규 `UCFVehicleData`는 생겼지만, 현재 `CFVehiclePawn`의 `VehicleData`는 여전히 `UPrimaryDataAsset*` 타입이며, 주석에도 “현재 단계에서는 BP에서 연결만 유지하고, 데이터 자동 해석은 후속 단계”라고 적혀 있다.

즉, 지금은 **로직보다 먼저 데이터 계층을 정리해야 하는 시점**이다.

이 상태에서 구현만 계속 진행하면 아래 문제가 커진다.

- 차종이 하나 더 생길 때마다 어떤 값은 BP에서 복사하고, 어떤 값은 `VehicleMovementComp`에서 수동으로 바꾸고, 어떤 값은 휠 동기화 디버그 값을 만지게 된다.
- 같은 의미의 값이 여러 위치에 중복된다.
- “무엇이 기준값인지”를 사람이 기억으로 관리하게 된다.
- C++ 중심 전환을 해도, 실제 튜닝 작업은 계속 BP Details 수작업에 묶인다.

따라서 이번 문서는 구현보다 먼저, **차량 데이터의 저장 위치와 해석 권한을 고정하는 계획서**로 분리해서 다루는 것이 맞다.

---

## 2. 현재 차량 데이터가 어디에 흩어져 있는가

현재 확인 기준으로 차량 데이터는 크게 4개 층에 흩어져 있다.

### 2.1 `BP_CFVehiclePawn` Details

현재 기준 플레이 차량인 `BP_CFVehiclePawn`에는 아래 성격의 값이 노출되어 있다.

- `VehicleData`
- 입력 관련 기본 자산 참조
  - `DefaultInputMappingContext`
  - `InputAction_Throttle`
  - `InputAction_Steering`
  - `InputAction_Brake`
  - `InputAction_Handbrake`
- 런타임 초기화/자동 Tick 관련 토글
  - `bAutoInitializeOnBeginPlay`
  - `bEnableWheelVisualTick`
  - `bAutoRegisterInputMappingContext`
- 입력 장치 정책 값
  - `InputDeviceMode`
  - `InputDeviceAnalogThreshold`
  - `InputMappingPriority`

이 중 일부는 차량 공통 정책이고, 일부는 진짜 차종 데이터가 아니며, 일부는 단순 런타임 편의 옵션이다. 현재는 이들이 한 레벨에서 같이 보이기 때문에, 초보자가 보면 전부 차량별 튜닝값처럼 오해하기 쉽다.

### 2.2 `VehicleMovementComp` 내부 값

사용자가 이번 세션에서 짚은 핵심 지점 중 하나다.

현재 차량 물리 관련 핵심 값들은 `VehicleMovementComp` 내부에 직접 들어가 있을 가능성이 높다.

대표적으로 아래 계열이다.

- 엔진 / 토크 / RPM / 기어비
- 조향각 / 조향 반응
- 브레이크 / 핸드브레이크 반응
- 서스펜션 / 휠 반경 / 휠 폭
- 전륜 / 후륜 휠 클래스
- 변속 관련 기본값
- 차체 물리 반응 관련 Chaos Vehicle 튜닝값

이 값들은 현재 차량마다 가장 많이 바뀔 수 있는 값인데, 지금처럼 `VehicleMovementComp` 안에 직접 박혀 있으면 차종 추가 시 관리가 급격히 어려워진다.

### 2.3 `WheelSyncComp` 내부 값

`CFWheelSyncComp`에는 현재 아래 두 종류의 값이 함께 있다.

#### A. 차종별로 달라질 수 있는 값
- 휠 Anchor / Mesh 이름 규칙
- 전륜 개수
- 조향 Yaw 스케일
- 휠 시각 보정 부호
- 차종별 시각 휠 오프셋 기준

#### B. 디버그 / 단계 전환 / 검증용 값
- `bVerboseLog`
- `bEnableApplyTransformsInCpp`
- `bUseRealWheelTransformHelper`
- `bHelperOverridesDebugInputs`
- `bEnableHelperCompareMode`
- `bLogHelperCompareDetails`
- `HelperCompareWarnDeltaDeg`
- `HelperCompareWarnDeltaZ`
- `bUseSteeringYawDebugPipe`
- `bUseSuspensionZDebugPipe`
- `bUseWheelSpinPitchDebugPipe`
- `SuspensionZDebugOffset`
- `WheelSpinPitchDebugDeg`
- `bEnableSingleAxisSteeringYawCppApplyOnly`
- 축별 C++ 적용 토글

현재 구조는 Phase 2 전환과 검증을 위해 합리적이었지만, 장기 구조 기준으로 보면 **차종 데이터와 디버그용 세션 값이 한 컴포넌트에 같이 있다.**

### 2.4 레거시 데이터 계층

프로젝트 안에는 기존 파이프라인의 흔적이 아직 남아 있다.

- `VehicleData` 슬롯
- `PDA_VehicleArchetype`
- `DA_PoliceCar`
- `S_WheelConfig`

이 자산들은 원래 `BP_ModularVehicle` 계열 시절의 데이터 레이어로 출발했다.

현재 의미는 아래처럼 해석하는 것이 맞다.

- `PDA_VehicleArchetype` / `DA_PoliceCar` / `S_WheelConfig`는 **레거시 보관 슬롯**이다.
- 현재 기준 구현이 이 자산을 “공식 차량 데이터 경로”로 쓰고 있다고 보면 안 된다.
- 다만, 실제 프로젝트에서 차체/바퀴 메시와 기본 레이아웃 참조가 담긴 상태이므로, 신규 구조를 설계할 때 **이전 대상 목록**으로는 매우 중요하다.

### 2.5 신규 Native 데이터 계층

이미 `UE/Source/CarFight_Re/Public/CFVehicleData.h`에 `UCFVehicleData`와 `FCFVehicleWheelLayout`가 정의되어 있다.

현재 이 타입은 아래 의미를 가진다.

- Native DataAsset 기반 차량 데이터 레이어의 시작점
- 시각 자산과 기본 레이아웃 데이터의 Native 고정
- 아직 이동/물리/휠 시각 전체를 총괄하는 루트 구조는 아님

즉, 신규 구조는 **아예 없는 것이 아니라 이미 시작되어 있다.**  
이번 계획의 역할은 이 `UCFVehicleData`를 어떻게 차량 데이터의 루트 구조로 확장할지 정하는 것이다.

---

## 3. 현재 구조의 문제점

현재 구조의 문제는 단순히 값이 여러 곳에 있다는 사실 자체가 아니다. 진짜 문제는 **값의 저장 위치와 적용 책임이 분리되어 있지 않다**는 점이다.

### 3.1 공식 데이터 경로가 불분명하다

현재 `CFVehiclePawn`의 `VehicleData`는 `UPrimaryDataAsset*` 타입으로 열려 있고, 실제로 어떤 클래스가 공식인지 고정돼 있지 않다.

이 상태에서는 아래 문제가 생긴다.

- `PDA_VehicleArchetype`를 넣어도 되고,
- 나중에 `UCFVehicleData`를 넣어도 되는 것처럼 보이며,
- C++가 어떤 구조를 기대하는지 에디터에서 바로 드러나지 않는다.

즉, **데이터 타입이 느슨해서 공식 데이터 계약이 없다.**

### 3.2 차종 데이터와 런타임 정책이 섞여 있다

예를 들어 아래 둘은 성격이 다르다.

- 경찰차와 스포츠카에 따라 바뀌는 값
- 모든 차량 공통으로 유지해야 하는 시스템 정책 값

현재는 이 둘이 BP Details와 컴포넌트 옵션에 섞여 있다. 그러면 차종별 튜닝을 바꾸다가 시스템 정책까지 건드릴 위험이 커진다.

### 3.3 디버그값이 실제 데이터처럼 보인다

`CFWheelSyncComp`는 현재 전환용이라 디버그 파이프가 많이 열려 있다. 이것은 개발 단계에서는 좋지만, 장기 구조에서는 아래 문제가 있다.

- `SuspensionZDebugOffset` 같은 값이 차종 데이터처럼 보일 수 있다.
- `WheelSpinPitchDebugDeg`를 실제 차량 스핀 세팅처럼 오해할 수 있다.
- 단일 축 적용 테스트 토글이 차종 특성처럼 남을 수 있다.

즉, **디버그값은 필요하지만, 차종 데이터와 물리적으로 분리해서 보여줘야 한다.**

### 3.4 VehicleMovement 세팅이 차종 확장에 취약하다

현재 `VehicleMovementComp` 안에 값이 직접 들어 있으면, 차종이 늘어날수록 아래 작업이 반복된다.

1. 새 BP 생성
2. `VehicleMovementComp` 직접 수정
3. 휠 클래스 확인
4. 조향 / 엔진 / 브레이크 값 수동 복사
5. 누락된 값 확인

이 방식은 차종이 1대일 때는 버틸 수 있지만, 3대, 5대, 10대로 늘어나면 운영 비용이 급격히 커진다.

### 3.5 C++ 중심 전환과 맞지 않는다

`CarFight_CPP_RePlan.md`의 방향은 분명하다.

- 규칙/판단/계산은 C++
- 값 저장은 DataAsset
- BP는 얇은 표현/조립 계층

그런데 실제 차량 튜닝의 주 저장소가 계속 BP Details와 `VehicleMovementComp` 내부라면, 겉으로만 C++ 중심이고 실질 운영은 여전히 BP 중심이 된다.

즉, 현재 데이터 구조는 프로젝트 전체 C++화 방향과 정면으로 연결된 문제다.

---

## 소유권 원칙

## 4. C++ / DataAsset / BP / Debug 가 각각 무엇을 소유해야 하는가

이번 구조 정리의 핵심은 “무엇을 어디에 둬야 하는가”보다 먼저 “누가 어떤 종류의 책임을 가져야 하는가”를 고정하는 것이다.

### 4.1 C++가 소유해야 하는 것

C++는 아래를 반드시 소유해야 한다.

- 데이터 타입 정의
- 데이터 유효성 검사
- 데이터 해석 규칙
- `VehicleMovementComp` 적용 순서
- 휠 시각 입력 계산 규칙
- 차종 데이터에서 런타임 값으로 변환하는 함수
- 기본값 누락 시 폴백 규칙
- 레거시 데이터에서 신규 데이터로 전환하는 브리지

쉽게 말하면, **DataAsset이 무슨 뜻인지 해석하는 책임은 전부 C++**가 가져야 한다.

### 4.2 DataAsset이 소유해야 하는 것

DataAsset은 아래를 소유해야 한다.

- 차종별 자산 참조
- 차종별 튜닝 수치
- 차종별 레이아웃/시각 보정 데이터
- 차종별 휠/구동/엔진/조향 기본 세팅

하지만 DataAsset은 아래를 소유하면 안 된다.

- 계산 로직
- 상태 전이 규칙
- 디버그 실행 토글
- 런타임 임시 결과

즉, **DataAsset은 저장소이지 두뇌가 아니다.**

### 4.3 BP가 소유해도 되는 것

BP는 아래를 소유해도 된다.

- 컴포넌트 배치
- 카메라 / VFX / SFX / 머티리얼 연결
- 시각 표현용 컴포넌트 참조
- 에디터에서 확인하기 좋은 조립 구조
- 차체/휠/카메라의 표현 레이어 연결

다만 BP는 아래를 소유하면 안 된다.

- 차량 규칙값의 원본 저장소
- 차종별 물리 튜닝의 기준값
- 휠 계산 규칙
- 엔진/기어/브레이크 해석 규칙

### 4.4 Debug가 소유해야 하는 것

Debug는 아래를 소유해야 한다.

- 검증용 강제값
- 축/부호 확인값
- 비교용 임계값
- 로그 세부 출력 토글
- 단계 전환용 임시 토글

Debug는 아래로 승격되면 안 된다.

- 차종 공식 데이터
- 출시 기준 튜닝값
- 프로젝트 공통 정책

즉, Debug는 **정답 데이터**가 아니라 **정답을 검증하기 위한 임시 관측 장치**다.

---

## 목표 구조

## 5. 루트 차량 DataAsset 구조를 어떻게 잡을지

권장 방향은 `UCFVehicleData`를 차량 데이터의 루트 타입으로 승격하는 것이다.

현재 `UCFVehicleData`는 시각 자산 + 기본 레이아웃 중심으로 시작되어 있다. 이를 아래처럼 확장하는 것이 맞다.

```text
UCFVehicleData (루트 차량 DataAsset)
├── Visual
│   ├── ChassisMesh
│   ├── WheelMeshFL/FR/RL/RR
│   ├── 시각 휠 미러링/축 보정 규칙용 데이터
│   └── 차종별 시각 오프셋
├── Layout
│   ├── WheelLayout
│   ├── 휠 축 위치/마진
│   ├── 차체 기준 휠 기준점
│   └── 수동 Wheel_Anchor 기준 배치 정보
├── Movement
│   ├── 엔진 / 드라이브 / 기어 / 디퍼렌셜
│   ├── 조향 / 브레이크 / 핸드브레이크
│   ├── 휠 반경 / 폭 / 서스펜션 / 마찰 계열
│   └── VehicleMovement 적용용 튜닝 데이터
├── WheelVisual
│   ├── 조향 시각 보정
│   ├── 스핀 축/부호
│   ├── 서스펜션 시각 보정
│   └── 휠 이름 규칙 또는 인덱스 규칙
├── References
│   ├── 전륜 Wheel Class
│   ├── 후륜 Wheel Class
│   ├── 필요한 Curve / 보조 자산 참조
│   └── 차종 식별용 메타 정보
└── ValidationMeta
    ├── ExpectedWheelCount
    ├── 지원하는 휠 이름 규칙
    └── 누락 허용 범위 / 폴백 정책
```

핵심은 루트 DataAsset 하나를 열면, **그 차종을 구성하는 값이 한 곳에서 보이게** 만드는 것이다.

---

## 6. 어떤 값을 DataAsset으로 옮길 후보인지

아래 값들은 차종별로 달라질 가능성이 높고, 동시에 사람이 에디터에서 반복 조정하게 되는 값이므로 DataAsset 후보로 보는 것이 맞다.

### 6.1 시각 자산 참조

- 차체 메쉬
- 휠 메쉬 FL / FR / RL / RR
- 전륜 / 후륜 휠 클래스 참조
- 차종별 추가 Curve / Material / 보조 자산 참조가 필요하면 그 경로

### 6.2 차체/휠 레이아웃 값

- `S_WheelConfig`가 갖고 있던 성격의 값
- 앞바퀴 축 마진
- 뒷바퀴 축 마진
- 좌우 폭 마진
- 높이 오프셋
- `Wheel_Anchor` 기준 수동 배치 정보
- 차종별 수동 배치 보정값

### 6.3 VehicleMovement 관련 차종 튜닝값

- 엔진 토크 / 출력 관련 값
- 최대 RPM / idle RPM / 토크 커브 참조
- 브레이크 강도 / 핸드브레이크 강도
- 조향 최대각 / 속도 기반 조향 감소 계수
- 기어비 / 변속 세팅
- 디퍼렌셜 / 구동축 정책
- 휠 반경 / 폭 / 서스펜션 / 마찰 계열
- 전륜/후륜 휠 세팅 차이

### 6.4 휠 시각 동기화의 차종별 값

- 조향 Yaw 시각 보정치
- 스핀 Pitch 축/부호 보정치
- 서스펜션 시각 보정치
- 전륜 개수
- 휠 인덱스 규약
- 차종별 Anchor / Mesh 이름 규칙이 정말 다를 경우 그 이름 매핑 데이터

### 6.5 검증용 메타 값

- 기대 휠 개수
- 필수 Anchor 이름 목록
- 필수 Mesh 이름 목록
- 누락 시 폴백 허용 여부

이 메타 값은 디버그용이 아니라 **데이터 검증용**이다. 따라서 Debug가 아니라 DataAsset 또는 C++ 검증 레이어에서 관리하는 것이 맞다.

---

## 7. 어떤 값은 BP에 남겨도 되는지

BP에 남겨도 되는 값은 “차종 정의”가 아니라 “해당 Pawn 인스턴스의 표현 조립” 성격이어야 한다.

권장 잔류 대상은 아래와 같다.

- 카메라 컴포넌트 배치
- 스프링암 길이의 에디터 조립용 값
- Niagara / Audio / 머티리얼 연결 참조
- 장착용 컴포넌트 위치
- HUD, VFX, SFX 연결을 위한 표현용 참조
- 테스트 맵에서만 쓰는 임시 노출값

중요한 기준은 하나다.

> **그 값을 다른 차종에서도 공통 규칙으로 재사용해야 한다면 BP에 두지 않는다.**

즉, 차량이 바뀔 때마다 다시 손봐야 하는 값이라면 BP가 아니라 DataAsset 또는 C++ 정책으로 옮겨야 한다.

---

## 8. 어떤 값은 Debug 전용으로 남겨야 하는지

아래 값들은 Debug 전용으로 남겨야 한다.

- `bVerboseLog`
- helper 비교 모드 on/off
- helper delta 경고 임계값
- 강제 디버그 오프셋
- 강제 스핀 각도
- 단일 축 적용 테스트 토글
- 축별 C++ 적용 단계 토글
- fallback 로그 1회 출력 여부

이 값들은 문서에서도 “차종 데이터”로 부르면 안 된다.  
반드시 **디버그 파이프**, **전환 토글**, **검증 보조값**으로 분리해서 표기해야 한다.

권장 방향은 아래 둘 중 하나다.

1. `CFWheelSyncComp` 내부에 Debug 카테고리로만 남긴다.  
2. 또는 장기적으로 `FCFWheelSyncDebugConfig` 같은 별도 구조체로 묶어 분리한다.

어느 쪽이든 원칙은 같다.

- 차종 저장소에 넣지 않는다.
- `DA_PoliceCar` 같은 차량 자산에 섞지 않는다.
- 릴리즈 기준 튜닝값처럼 보이게 하지 않는다.

---

## 값 분류

## 9. VehicleMovement 관련 값들을 어떤 계층으로 옮길지

`VehicleMovementComp` 내부 값은 성격별로 3층으로 나눠 옮기는 것이 맞다.

### 9.1 차종 고유값 → `UCFVehicleData`

차량마다 달라질 값은 모두 여기로 이동한다.

예시:

- 엔진 세팅
- 브레이크 세팅
- 핸드브레이크 세팅
- 조향 세팅
- 변속 세팅
- 휠 반경 / 폭
- 전륜 / 후륜 Wheel Class 참조
- 서스펜션 기본값

### 9.2 적용 규칙 → `CFVehiclePawn` 또는 `CFVehicleDriveComp`

DataAsset의 값을 언제 어떤 순서로 `VehicleMovementComp`에 넣을지는 C++가 소유한다.

예시:

- BeginPlay 전에 적용할지
- Construction 성격으로 적용할지
- 런타임 재적용을 허용할지
- 누락값일 때 어떤 기본값을 쓸지
- DataAsset 버전 차이를 어떻게 흡수할지

이 계층은 절대 BP로 보내면 안 된다.

### 9.3 엔진/플러그인 고정 정책 → C++ 또는 기본 클래스

차종이 달라도 바뀌지 않는 프로젝트 공통 정책은 DataAsset로 빼지 않는다.

예시:

- Chaos Vehicle 사용 방식 자체
- 입력 장치 허용 정책
- 프로젝트 공통 런타임 초기화 순서
- 공통 안전 가드

즉, VehicleMovement 관련 값은 전부 DataAsset로 보내는 것이 아니라, **차종 고유값만 DataAsset로 보내고 적용 규칙은 C++에 남겨야 한다.**

---

## 10. WheelSyncComp 관련 값들 중 차종 데이터와 디버그값을 어떻게 구분할지

`CFWheelSyncComp`는 이번 문서에서 가장 분류가 중요한 영역이다.

### 10.1 차종 데이터로 승격할 후보

아래는 차량이 바뀌면 달라질 수 있으므로 DataAsset 후보다.

- `ExpectedWheelCount`
- 휠 Anchor / Mesh 이름 목록
- 전륜 개수
- 조향 Yaw 스케일
- 휠 시각 보정 부호
- 차종별 시각 오프셋 기준

단, 여기서도 주의할 점이 있다.

휠 이름 목록은 차종마다 진짜로 다를 때만 DataAsset으로 보내는 것이 좋다.  
프로젝트가 `Wheel_Anchor_FL/FR/RL/RR`, `Wheel_Mesh_FL/FR/RL/RR` 규칙을 표준으로 강제할 수 있다면, 이름은 DataAsset이 아니라 **프로젝트 규약**으로 유지하는 편이 더 좋다.

즉, 이름까지 차종 데이터로 보낼지는 아래 기준으로 판단한다.

- 모든 차종이 같은 이름 규칙을 쓰게 만들 수 있다 → C++ 규약으로 고정
- 외부 차량 에셋마다 구조가 달라서 이름 차이를 흡수해야 한다 → DataAsset 이름 매핑 허용

### 10.2 Debug 전용으로 유지할 값

아래는 DataAsset으로 보내면 안 된다.

- helper 사용 여부
- helper override 여부
- helper compare 모드
- compare 로그 상세 출력
- 경고 임계값
- 강제 스핀 / 강제 서스펜션 오프셋
- 단일 축 테스트 모드
- 축별 C++ 적용 단계 토글

이 값들은 모두 **전환 단계의 관측용 제어판**이기 때문이다.

### 10.3 런타임 결과값으로만 유지할 값

아래는 저장 데이터가 아니라 런타임 상태다.

- `LastWheelVisualInputs`
- `LastWheelVisualStates`
- `LastHelperCompareStates`
- `LastHelperCompareSummary`
- `SingleAxisCppApplyTestSummary`
- `bWheelSyncReady`
- `LastValidationSummary`

이 값들은 문서상으로도 “데이터”가 아니라 “상태”라고 구분해서 써야 한다.

---

## 11. 차종이 여러 개 생겼을 때 어떤 식으로 관리될지

이 계획이 성공하면 차량 추가 흐름은 아래처럼 단순해져야 한다.

### 11.1 신규 차종 추가 절차

1. `UCFVehicleData` 기반 새 차량 DataAsset 생성
2. 차체/휠 메쉬 참조 입력
3. 차량 레이아웃 값 입력
4. VehicleMovement 튜닝값 입력
5. 휠 시각 보정값 입력
6. 기존 `BP_CFVehiclePawn` 또는 차종별 파생 Thin BP에 해당 DataAsset만 연결
7. C++가 동일한 적용 파이프를 통해 자동 반영

즉, 차종이 추가되어도 **코드가 늘어나는 것이 아니라 데이터 인스턴스가 늘어나는 구조**가 되어야 한다.

### 11.2 권장 운영 방식

- 차량 공통 규격은 `UCFVehicleData` 클래스 정의에서 고정한다.
- 실제 차량별 차이는 `DA_*` 인스턴스에서만 만든다.
- `DA_PoliceCar`는 레거시 `PDA_VehicleArchetype` 기반에서 출발했더라도, 최종적으로는 `UCFVehicleData` 기반 신규 차량 데이터로 이관한다.
- 차종 파생 BP는 가능하면 최소화한다.
- 차종 차이는 가급적 DataAsset만으로 흡수한다.

### 11.3 기대 효과

- 경찰차 / 스포츠카 / 오프로드 차량이 추가되어도 튜닝 위치가 일관된다.
- 차종 비교가 쉬워진다.
- 밸런싱 작업이 BP 비교가 아니라 DataAsset 비교로 바뀐다.
- C++는 적용기 하나만 유지하면 된다.

---

## 단계별 실행 계획

## 11.5 현 단계 권장 운영 전략 (MCP 수치 가시화 전)

현재 사용자는 MCP에서 `VehicleMovementComp`와 휠 관련 Details 수치를 직접 읽을 수 있게 만드는 작업을 진행 중이다.

이 상황에서는 차량 데이터 정리를 끝까지 밀어붙이는 것보다, **후에 확장 가능한 최소 골격만 먼저 만들고, 수치가 가시화되면 본격 이관을 재개하는 방식**이 더 안전하다.

### 지금 당장 해도 되는 작업

- `UCFVehicleData`를 루트 차량 DataAsset 후보로 유지한다.
- `CFVehiclePawn`가 장기적으로 `UCFVehicleData`를 공식 입력으로 받도록 방향만 고정한다.
- `Visual`, `Layout`, `Movement`, `WheelVisual`, `References` 같은 상위 계층 이름만 먼저 정리한다.
- `VehicleMovementComp`와 `CFWheelSyncComp`에서 무엇이 차종 데이터 후보인지 분류만 먼저 끝낸다.
- 레거시 `PDA_VehicleArchetype`, `DA_PoliceCar`, `S_WheelConfig`가 신규 구조에서 어디로 들어갈지 대응 방향만 잡는다.

### 지금 보류하는 것이 좋은 작업

- `VehicleMovementComp`의 세부 수치값을 억지로 추측해서 DataAsset 구조에 확정하는 작업
- `BP_Wheel_Front`, `BP_Wheel_Rear`의 실제 숫자값을 모른 채 하위 프리셋 구조를 먼저 고정하는 작업
- Movement 프리셋 / Wheel 프리셋 / Tire 프리셋 등을 한 번에 세분화하는 작업
- 경찰차 1대의 전체 튜닝값을 수작업 복사로 먼저 신규 구조에 박아 넣는 작업

### 왜 이 방식이 맞는가

이 방식의 장점은 아래와 같다.

1. 지금 할 수 있는 구조 정리는 먼저 진행할 수 있다.
2. MCP 수치 가시화가 완료되면, 그 시점의 실제 값을 기준으로 정확한 이관이 가능하다.
3. 현재 단계에서 잘못된 가정으로 DataAsset 필드를 과하게 설계하는 위험을 줄일 수 있다.
4. 프로젝트가 아직 비교적 단순한 상태이므로, 지금은 구조의 방향만 맞추고 수치 이관은 근거가 생긴 뒤 진행하는 편이 비용이 적다.

### 현 단계의 실무 권장안

현재 시점의 권장안은 아래 한 문장으로 정리할 수 있다.

> **지금은 차량 데이터 구조의 뼈대와 소유권만 정리하고, `VehicleMovementComp` / 휠 Details 실제 수치 이관은 MCP 가시화 완료 이후 재개한다.**

즉, 지금 단계에서는 `대충 짜놓는다`기보다, **나중에 다시 작업할 수 있게 구조를 느슨하지 않게만 잡아두고 숫자 확정은 보류한다**는 표현이 더 정확하다.

### 현재 세션 기준 예외 메모

현재 세션에서는 위 원칙을 유지하되, **마이그레이션 C++ / 현재 소스 / 문서 중 최소 한 곳에서 직접 근거가 잡히는 항목에 한해** 구현을 일부 선행했다.

현재까지 선행 반영된 범위:
- `Visual` / `Layout` / `Reference` / `WheelVisual` 적용 경로
- Wheel Class 핵심 수치 이관
- `VehicleMovementComp` 본체 1차 값 이관
- `WheelSetups` 정책(`bLegacyWheelFrictionPosition`, `AdditionalOffset`) 반영

현재 보류하는 범위:
- 블루프린트 자산 확인이 필요한 곡선/세부 변속 계열
- MCP 수치 가시화 없이는 확정하기 어려운 항목
- 의미 해석이 필요한 확장 필드

따라서 현 시점의 실무 판단은 **"근거 있는 범위까지는 구현을 선행했고, 그 이후는 임시중단 후 재개 조건 충족 시 다시 진행한다"**로 정리한다.

### 재개 조건

아래 조건이 충족되면 본격 데이터 이관 단계를 다시 시작하는 것이 좋다.

- MCP에서 `VehicleMovementComp`의 엔진 / 조향 / 브레이크 / 기어 / 휠 세팅 수치를 읽을 수 있다.
- MCP에서 `BP_Wheel_Front`, `BP_Wheel_Rear`의 반경 / 폭 / 조향 / 핸드브레이크 / 서스펜션 관련 기본값을 읽을 수 있다.
- 현재 차량 기준값을 실제 데이터로 캡처할 수 있다.

이 조건이 충족되면 그때 아래 순서로 재개한다.

1. `VehicleMovementComp` 실제 값 인벤토리 확정
2. 휠 BP 실제 값 인벤토리 확정
3. `UCFVehicleData` 하위 구조체 구체화
4. 경찰차 1대 시범 이관
5. 검증 후 추가 차종 확장


## 12. 단계별 실행 계획

이번 문서는 구현서가 아니라 계획서이므로, 아래 순서를 기준 실행 계획으로 고정한다.

### Phase 0. 기준 선언 고정

목표는 현재 기준 구현과 레거시 경계를 문서에서 먼저 못 박는 것이다.

실행 항목:

- 현재 기준 구현을 `BP_CFVehiclePawn`으로 고정
- Native 코어를 `CFVehiclePawn` + `CFVehicleDriveComp` + `CFWheelSyncComp`로 고정
- `BP_ModularVehicle`는 이미 삭제되었고 기준 구현이 아님을 명시
- `PDA_VehicleArchetype`, `DA_PoliceCar`, `S_WheelConfig`는 레거시 보관 슬롯임을 명시
- `UCFVehicleData`를 신규 공식 루트 후보로 명시

완료 기준:

- 이후 문서와 구현에서 기준 구현 혼선이 없어야 한다.

### Phase 1. 값 인벤토리 작성

목표는 현재 흩어진 값을 빠짐없이 분류하는 것이다.

실행 항목:

- `BP_CFVehiclePawn` Details의 차량 관련 값 목록화
- `VehicleMovementComp` 내부 조정값 목록화
- `CFWheelSyncComp`의 차종값 / 디버그값 / 런타임 상태값 분리표 작성
- 레거시 `PDA_VehicleArchetype`, `DA_PoliceCar`, `S_WheelConfig` 필드 목록 작성
- 신규 `UCFVehicleData` 필드와 대응 관계 작성

완료 기준:

- 모든 차량 관련 값이 “현재 위치 / 최종 위치 / 소유자” 3열 표로 정리된다.

### Phase 2. `UCFVehicleData` 루트 구조 확장 설계

목표는 신규 공식 차량 DataAsset 구조를 확정하는 것이다.

실행 항목:

- Visual / Layout / Movement / WheelVisual / References / ValidationMeta 계층 설계
- 기존 `FCFVehicleWheelLayout` 확장 여부 결정
- VehicleMovement용 하위 구조체 설계
- 휠 시각 보정용 하위 구조체 설계
- 레거시 `S_WheelConfig` 대응 필드 설계

완료 기준:

- `UCFVehicleData`만 보면 차량 한 대의 기본 세팅을 전부 파악할 수 있어야 한다.

### Phase 3. C++ 적용 경로 설계

목표는 DataAsset을 “저장소”에서 “공식 입력”으로 승격하는 것이다.

실행 항목:

- `CFVehiclePawn`의 `VehicleData` 타입을 장기적으로 `UCFVehicleData*`로 고정하는 계획 수립
- BeginPlay / 초기화 단계에서 DataAsset을 읽는 적용 함수 설계
- `VehicleMovementComp` 적용 함수 설계
- 휠 시각 데이터 적용 함수 설계
- 누락값 / 기본값 / fallback 규칙 설계

완료 기준:

- DataAsset을 연결했을 때 어떤 C++ 함수가 어떤 순서로 해석하는지 설명 가능해야 한다.

### Phase 4. 레거시 브리지 설계

목표는 기존 자산을 한 번에 버리지 않고 안전하게 이전하는 것이다.

실행 항목:

- `PDA_VehicleArchetype` → `UCFVehicleData` 필드 매핑 표 작성
- `DA_PoliceCar`의 신규 DataAsset 이관 경로 설계
- `S_WheelConfig` → `FCFVehicleWheelLayout` 또는 신규 Layout 구조 대응 설계
- 필요 시 임시 변환 함수 또는 수동 이관 체크리스트 작성

완료 기준:

- 현재 차량 한 대를 신규 DataAsset 구조로 옮기는 절차가 문서화되어야 한다.

### Phase 5. 디버그 분리

목표는 `CFWheelSyncComp` 내부 디버그 파이프를 차종 데이터와 분리하는 것이다.

실행 항목:

- 차종 데이터 후보 필드와 디버그 필드 분리
- Debug 카테고리 명확화
- 릴리즈 기준값과 검증값을 문서와 코드 카테고리에서 분리
- helper compare / single axis / 강제 오프셋은 Debug 전용으로 고정

완료 기준:

- 초보자가 Details를 열어도 무엇이 차종 데이터이고 무엇이 디버그인지 헷갈리지 않아야 한다.

### Phase 6. 차종 운영 규칙 고정

목표는 이후 차량이 늘어나도 같은 규칙으로 관리하는 것이다.

실행 항목:

- 새 차량 추가 절차 문서화
- 차종별 BP 파생 최소화 원칙 문서화
- DataAsset 네이밍 / 보관 경로 / 생성 순서 규칙화
- 튜닝 변경은 DataAsset 우선, BP 직접 수정 금지 원칙 정리

완료 기준:

- 신규 차량 추가 시 “무엇을 복사해야 하지?”가 아니라 “어떤 DataAsset을 만들지?”로 질문이 바뀌어야 한다.

---

## 완료 조건

## 13. 완료 조건

이번 계획이 완료되었다고 보려면 아래 조건이 필요하다.

### 문서 기준 완료 조건

- 현재 기준 구현 / 레거시 경계가 문서에 명확히 선언되어 있다.
- 모든 차량 관련 값이 소유자별로 분류되어 있다.
- `UCFVehicleData` 루트 구조가 정의되어 있다.
- `VehicleMovement` 값의 이전 계층이 정의되어 있다.
- `CFWheelSyncComp` 값 중 차종 데이터 / 디버그 / 런타임 상태가 분리되어 있다.
- 다차종 운영 방식이 설명되어 있다.

### 구현 전 준비 완료 조건

- `DA_PoliceCar`를 어떤 신규 구조로 옮길지 대응표가 있다.
- `VehicleData` 타입을 어떻게 공식화할지 계획이 있다.
- 레거시 `PDA_VehicleArchetype`를 언제 퇴역시킬지 기준이 있다.

### 구조 완료 조건

- 차종별 튜닝의 원본이 BP Details가 아니라 DataAsset이다.
- C++가 DataAsset을 읽어 `VehicleMovementComp`와 `WheelSyncComp`에 적용한다.
- Debug 토글을 건드리지 않아도 차종 세팅이 정상 작동한다.
- BP는 더 이상 차량 수치의 원본 저장소가 아니다.

---

## 리스크와 대응

## 14. 리스크와 대응

### 리스크 1. 레거시와 신규 구조가 오래 공존해 이중화된다

설명:

- `PDA_VehicleArchetype`와 `UCFVehicleData`가 동시에 살아 있으면 기준이 흔들릴 수 있다.

대응:

- 문서에서 먼저 `UCFVehicleData`를 신규 공식 루트 후보로 선언한다.
- 레거시는 브리지 기간만 허용하고, 최종 기준은 하나로 고정한다.

### 리스크 2. VehicleMovement 값이 일부만 옮겨져 절반만 DataAsset화된다

설명:

- 시각 자산만 DataAsset으로 가고, 핵심 튜닝은 계속 `VehicleMovementComp`에 남을 수 있다.

대응:

- VehicleMovement 값을 엔진/프로젝트 공통 정책과 차종 고유값으로 먼저 분해한다.
- 차종 고유값은 반드시 DataAsset 대상 목록에 포함한다.

### 리스크 3. Debug값이 차종값으로 굳어진다

설명:

- 지금 `CFWheelSyncComp`에는 디버그용 강제값이 많다.

대응:

- Debug 카테고리를 별도로 유지한다.
- 차종 DataAsset으로 이동 가능한 값과 금지 값을 문서에서 먼저 고정한다.

### 리스크 4. BP가 다시 데이터 원본 저장소가 된다

설명:

- 구현이 급하면 다시 BP Details에 값을 열고 끝내기 쉽다.

대응:

- 차량별로 달라지는 값은 BP에 추가하지 않는 원칙을 유지한다.
- 필요한 값이 생기면 먼저 “차종 데이터인가 / 시스템 정책인가 / 디버그값인가”를 분류하고 위치를 정한다.

### 리스크 5. 차종마다 컴포넌트 이름 규칙이 달라 관리가 복잡해진다

설명:

- 휠 Anchor / Mesh 이름이 차종마다 제각각이면 DataAsset이 이름 매핑 표로 비대해질 수 있다.

대응:

- 가능하면 `Wheel_Anchor_FL/FR/RL/RR`, `Wheel_Mesh_FL/FR/RL/RR`를 표준 규약으로 유지한다.
- 예외 차량만 이름 매핑 데이터를 허용한다.

---

## 권장 시작 순서

## 15. 권장 시작 순서

실제 작업은 아래 순서로 시작하는 것을 권장한다.

1. `BP_CFVehiclePawn` / `VehicleMovementComp` / `CFWheelSyncComp`의 현재 값 목록을 먼저 표로 정리한다.
2. `PDA_VehicleArchetype`, `DA_PoliceCar`, `S_WheelConfig`의 필드와 의미를 신규 구조 기준으로 대응표로 만든다.
3. `UCFVehicleData` 루트 구조를 확정한다.
4. `CFVehiclePawn`의 `VehicleData`를 장기적으로 `UCFVehicleData` 기준으로 고정하는 적용 계획을 만든다.
5. `VehicleMovementComp` 적용용 C++ 함수 설계를 먼저 한다.
6. `CFWheelSyncComp`의 차종 데이터와 Debug 데이터를 분리한다.
7. `DA_PoliceCar` 1대를 신규 구조로 옮겨서 검증한다.
8. 그 다음에야 추가 차종을 만든다.

이 순서를 권장하는 이유는, 경찰차 1대를 기준으로 이전이 성공해야 이후 차종 확장이 안전하기 때문이다.

---

## 최종 판단

지금 CarFight에서 필요한 것은 “VehicleMovement 수치를 몇 개 DataAsset으로 빼는 작업”이 아니다.

지금 필요한 것은 아래 3가지를 함께 고정하는 것이다.

- **현재 기준 구현은 무엇인가**  
  → `BP_CFVehiclePawn` + `CFVehiclePawn` + `CFVehicleDriveComp` + `CFWheelSyncComp`

- **차량 데이터의 공식 저장소는 무엇인가**  
  → 장기적으로 `UCFVehicleData` 기반 루트 차량 DataAsset

- **그 데이터를 누가 해석하고 적용하는가**  
  → BP가 아니라 C++

따라서 이번 계획의 최종 판단은 아래와 같다.

> **CarFight 차량 데이터 레이어 정리는 프로젝트 전체 C++화 방향의 하위 작업이 아니라, 그 방향을 실제 운영 구조로 굳히기 위한 핵심 기반 작업이다.**

`BP_CFVehiclePawn`를 Thin BP로 유지하려면, 차종별 차이는 BP가 아니라 DataAsset에서 나와야 한다.  
그리고 DataAsset이 진짜 공식 경로가 되려면, 그 의미를 해석하고 `VehicleMovementComp`와 `CFWheelSyncComp`에 반영하는 권한을 C++가 가져야 한다.

이 기준이 고정되면, 이후 차량이 2대, 5대, 10대로 늘어나도 작업 방식은 복잡해지지 않고, 단지 관리할 DataAsset 인스턴스가 늘어나는 구조로 유지할 수 있다.

---

## 다음 구현 단계

이번 세션의 범위는 계획서 작성까지다.  
다음 구현 단계는 짧게 아래 순서로 진행하는 것을 권장한다.

1. `VehicleMovementComp` 현재 값 인벤토리 문서화
2. `PDA_VehicleArchetype` / `DA_PoliceCar` / `S_WheelConfig` → `UCFVehicleData` 대응표 작성
3. `UCFVehicleData` 하위 구조체 초안 정의
4. `CFVehiclePawn`의 DataAsset 적용 함수 설계 시작
