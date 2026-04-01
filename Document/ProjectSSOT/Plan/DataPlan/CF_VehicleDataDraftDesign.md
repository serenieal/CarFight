# CarFight 차량 데이터 적용 구조 초안 설계서 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 기준 구현: `BP_CFVehiclePawn` / `CFVehiclePawn`
- 관련 Native 코어: `CFVehiclePawn` / `CFVehicleDriveComp` / `CFWheelSyncComp`
- 문서 목적: 현재 MCP에서 `VehicleMovementComp` 및 휠 Details 세부 수치를 완전히 읽기 전 단계에서, `UCFVehicleData`를 중심으로 차량 데이터가 어떤 경로로 저장되고, 어떤 C++ 계층에서 해석되고, 어떤 컴포넌트에 적용될지 구현 직전 수준으로 정리한다.
- 관련 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataRoadmap.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleMovementInventory.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataSkeleton.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataFields.md`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
  - `UE/Source/CarFight_Re/Public/CFWheelSyncComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`

> 상태 주의 (2026-04-01):
> - 이 문서는 적용 구조 초안 기록이다.
> - 본문에 남은 `VehicleLayoutConfig / FCFVehicleWheelLayout` 언급은 현재 구현 기준이 아니다.
> - 현재 기준선에서는 레거시 AutoFit 경로를 제거했고, 수동 Anchor 배치와 WheelRadius를 사용한다.

---

## 핵심 결론

현재 단계에서 필요한 것은 차량 데이터 시스템 완성이 아니라, **`UCFVehicleData`를 중심으로 한 적용 구조의 뼈대**를 확정하는 것이다.

이번 초안 설계서의 결론은 아래와 같다.

1. `UCFVehicleData`는 차량 한 대의 **루트 저장소**가 된다.
2. `CFVehiclePawn`는 이 DataAsset을 읽는 **진입점**이 된다.
3. `CFVehicleDriveComp`는 입력 처리와 구동 전달의 책임을 유지하고, 차량 데이터 해석의 중심이 되지는 않는다.
4. `CFWheelSyncComp`는 WheelVisual 계층을 소비하는 **시각 적용기**가 된다.
5. `VehicleMovementComp`는 여전히 실제 물리 적용 대상이지만, 장기적으로는 직접 수동 편집하는 곳이 아니라 **C++가 DataAsset 결과를 반영하는 대상**이 되어야 한다.
6. MCP 수치 가시화 전 단계에서는 실제 엔진/브레이크/조향/기어 수치 이관을 강행하지 않고, **Visual / Layout / References 우선, Movement / WheelVisual은 최소 슬롯만 유지**한다.

즉, 지금 당장 구현 직전 수준에서 확정해야 할 것은 “숫자”보다 **적용 경로**다.

---

## 1. 이 문서의 위치와 역할

이 문서는 현재 차량 데이터 문서 흐름에서 아래 위치에 있다.

```text
CF_VehicleDataRoadmap.md
    ↓
CF_VehicleDataPlan.md
    ↓
CF_VehicleDataInventory.md / CF_VehicleMovementInventory.md
    ↓
CF_MinVehicleDataSkeleton.md
    ↓
CF_MinVehicleDataFields.md
    ↓
CF_VehicleDataDraftDesign.md
```

즉, 이 문서는 다음 두 문서의 뒤를 잇는다.

- `CF_MinVehicleDataSkeleton.md`  
  → 상위 구조를 정리한 문서
- `CF_MinVehicleDataFields.md`  
  → 최소 필드 구성을 정리한 문서

그리고 이 문서는 그 다음 단계 문서의 출발점이 된다.

- 실제 `UCFVehicleData.h` 초안 설계
- `CFVehiclePawn` 초기 적용 함수 설계
- `CFWheelSyncComp` WheelVisual 소비 경로 설계
- MCP 수치 가시화 이후 Movement 확장 설계

즉, 이 문서는 **“어떤 구조를 만들 것인가” 다음에 오는 “그 구조를 누가 어떻게 소비할 것인가”**를 정리하는 문서다.

---

## 2. 현재 기준 구현 재선언

이 문서에서는 반드시 현재 기준 구현을 다시 분명히 선언해야 한다.

### 현재 기준 구현
- 기준 플레이 차량: `BP_CFVehiclePawn`
- 기준 Native Pawn: `CFVehiclePawn`
- 기준 구동 코어: `CFVehicleDriveComp`
- 기준 휠 시각 코어: `CFWheelSyncComp`

### 현재 기준이 아닌 것
- `BP_ModularVehicle` 계열
- `CFModVehiclePawn` 계열
- 레거시 `PDA_VehicleArchetype` 중심 파이프라인

### 현재 레거시이지만 무시하면 안 되는 것
- `VehicleData` 슬롯
- `PDA_VehicleArchetype`
- `DA_PoliceCar`
- `S_WheelConfig`
- `BP_Wheel_Front`
- `BP_Wheel_Rear`

이 재선언이 중요한 이유는, 설계 문서가 구현 직전 단계로 갈수록 과거 구조와 현재 구조가 다시 섞이기 쉽기 때문이다.

---

## 3. 적용 구조를 왜 지금 먼저 정해야 하는가

지금 MCP에서 `VehicleMovementComp`의 세부 수치를 전부 읽지 못하므로, 수치 중심 설계를 먼저 확정하는 것은 위험하다.

하지만 적용 구조는 지금 먼저 정할 수 있다.

왜냐하면 아래는 이미 충분히 확정 가능하기 때문이다.

- 루트 저장소 후보는 `UCFVehicleData`
- 진입점 후보는 `CFVehiclePawn`
- WheelVisual 소비기는 `CFWheelSyncComp`
- 실제 물리 적용 대상은 `VehicleMovementComp`
- BP는 Thin BP 유지가 맞다는 방향

즉, 세부 숫자는 보류할 수 있어도 아래는 지금 확정해도 된다.

- 누가 데이터를 저장하는가
- 누가 데이터를 읽는가
- 누가 어떤 계층을 해석하는가
- 누가 실제 컴포넌트에 반영하는가

따라서 지금 필요한 설계는 **값 목록 설계**보다 **적용 경로 설계**다.

---

## 4. 상위 구조 요약

현재 단계에서 권장하는 상위 데이터 구조는 아래다.

```text
UCFVehicleData
├── VehicleVisualConfig
├── VehicleLayoutConfig
├── VehicleMovementConfig
├── WheelVisualConfig
└── VehicleReferenceConfig
```

이 구조는 이미 `CF_MinVehicleDataSkeleton.md`와 `CF_MinVehicleDataFields.md`에서 정리한 최소 구조를 전제로 한다.

이번 문서에서는 이 상위 구조가 실제로 어떤 흐름으로 소비되는지를 아래처럼 해석한다.

```text
UCFVehicleData
    ↓
CFVehiclePawn (데이터 진입점 / 검증 / 적용 순서 제어)
    ├── VehicleMovementComp (물리/구동 적용 대상)
    ├── CFWheelSyncComp (휠 시각 적용 대상)
    └── BP_CFVehiclePawn (표현/조립 유지)
```

핵심은 아래다.

- DataAsset은 저장한다.
- `CFVehiclePawn`는 해석 시작점이다.
- 각 컴포넌트는 자기 책임 범위의 데이터만 소비한다.

---

## 5. 책임 분해

이 절은 실제 구현이 시작될 때 가장 많이 참고하게 될 부분이다.

### 5.1 `UCFVehicleData`

### 책임
- 차량 한 대의 차종 데이터를 저장한다.
- 차체/휠 자산 참조를 저장한다.
- 기본 레이아웃을 저장한다.
- Movement 계층의 최소 슬롯을 저장한다.
- WheelVisual 계층의 최소 슬롯을 저장한다.
- 전륜/후륜 Wheel Class 참조를 저장한다.

### 책임이 아닌 것
- 런타임 계산
- 실제 `VehicleMovementComp` 세팅 적용 순서
- WheelSync Tick 계산
- 디버그 토글

즉, `UCFVehicleData`는 **해석 대상**이지 **실행 주체**가 아니다.

---

### 5.2 `CFVehiclePawn`

### 책임
- `VehicleData` 참조를 보유한다.
- `VehicleData`의 실제 타입을 검증한다.
- BeginPlay 또는 초기화 구간에서 데이터 적용 루틴을 호출한다.
- Visual / Layout / Movement / WheelVisual / References 계층을 적절한 대상에게 분배한다.
- 누락된 필드가 있을 때 fallback 또는 skip 정책을 적용한다.

### 책임이 아닌 것
- 입력 처리 상세 로직
- WheelVisual Tick 상세 계산
- VehicleMovement 개별 휠 상태 계산

즉, `CFVehiclePawn`는 **차량 데이터 적용의 오케스트레이터**다.

---

### 5.3 `CFVehicleDriveComp`

### 책임
- 입력을 받아 차량 구동 시스템으로 전달한다.
- VehicleMovement와 연결된 입력 계층을 담당한다.
- DataAsset 적용 이후의 실제 입력-주행 연동을 유지한다.

### 현재 단계에서의 위치
지금 단계에서는 `CFVehicleDriveComp`가 DataAsset을 직접 해석할 필요는 없다.

권장 방향은 아래다.

- DataAsset 해석은 `CFVehiclePawn` 중심
- `CFVehicleDriveComp`는 적용된 구동 설정을 사용하는 소비자

즉, 지금은 이 컴포넌트에 데이터 해석 책임을 과하게 싣지 않는 것이 좋다.

---

### 5.4 `CFWheelSyncComp`

### 책임
- WheelVisual 계층 데이터를 소비한다.
- 휠 Anchor / Mesh / 전륜 수 / 기본 규약을 기준으로 시각 동기화를 수행한다.
- WheelVisual 관련 차종 데이터와 Debug 데이터를 분리해 유지한다.

### 책임이 아닌 것
- VehicleMovement의 엔진/브레이크/기어 해석
- 차종 전체 DataAsset 유효성 전체 검증

즉, `CFWheelSyncComp`는 **WheelVisual 적용기**다.

---

### 5.5 `VehicleMovementComp`

### 책임
- 실제 차량 물리/구동 세팅의 적용 대상이 된다.
- DataAsset 해석 결과가 최종적으로 반영되는 물리 컴포넌트다.

### 장기 목표
- 이 컴포넌트는 직접 수동 편집 지점이 아니라, C++ 적용의 최종 대상이 되어야 한다.

즉, 장기적으로는 아래처럼 바뀌어야 한다.

```text
지금: 사람이 VehicleMovementComp Details를 직접 조정
미래: C++가 UCFVehicleData를 읽고 VehicleMovementComp에 적용
```

---

### 5.6 `BP_CFVehiclePawn`

### 책임
- 표현 / 조립 / 컴포넌트 배치
- 카메라 / 머티리얼 / 시각적 조립 유지
- DataAsset을 연결하는 얇은 에디터 레이어

### 책임이 아닌 것
- 차량 기준 수치 보관
- 차종별 물리 튜닝 원본
- WheelVisual 계산 규칙

즉, 이 BP는 계속 **Thin BP**로 유지되어야 한다.

---

## 6. 권장 적용 순서

현재 단계에서 `CFVehiclePawn`가 `UCFVehicleData`를 읽는 적용 순서는 아래처럼 정리하는 것이 좋다.

### Step 1. DataAsset 존재 확인
- `VehicleData` 슬롯이 비어 있는지 확인
- 비어 있으면 기본 동작 또는 안전 skip

### Step 2. 타입 검증
- 장기적으로 `UCFVehicleData` 기준으로 수렴
- 현재는 레거시 타입 가능성을 염두에 두되, 신규 경로는 `UCFVehicleData`를 기준으로 본다

### Step 3. 기본 참조 계층 확인
- `VehicleVisualConfig`
- `VehicleLayoutConfig`
- `VehicleReferenceConfig`

이 계층은 현재 단계에서도 비교적 안정적으로 적용 가능하다.

### Step 4. Movement 계층 확인
- `VehicleMovementConfig`
- 현재 단계에서는 최소 슬롯만 유지
- `bUseMovementOverrides`가 false이면 skip 가능
- 세부 수치 적용은 MCP 가시화 이후 확장

### Step 5. WheelVisual 계층 확인
- `WheelVisualConfig`
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`
- `bUseWheelVisualOverrides`

현재 단계에서는 이 정도만 소비 가능하다.

### Step 6. 컴포넌트별 분배
- Wheel Class / 기본 참조 → VehicleMovementComp
- WheelVisual 최소 설정 → CFWheelSyncComp
- 시각 자산 / 레이아웃 → Pawn 조립 계층 또는 초기화 루틴

### Step 7. 검증 요약 기록
- 누락된 값
- skip된 계층
- fallback 사용 여부

이 요약은 Debug 로그 또는 내부 상태로 남기되, 차량 DataAsset에 넣지 않는다.

---

## 7. 현재 단계에서 실제로 적용 가능한 계층

지금 단계에서는 모든 계층을 같은 강도로 구현하면 안 된다.  
현재 MCP 한계를 반영하면 계층별 적용 강도는 아래가 맞다.

### 7.1 지금 바로 강하게 적용해도 되는 계층

#### Visual
- 차체 메쉬
- 휠 메쉬 4개

#### Layout
- `FCFVehicleWheelLayout` 기반 레이아웃

#### References
- `FrontWheelClass`
- `RearWheelClass`

이 세 계층은 비교적 구체적인 자산 참조이므로 지금 먼저 연결해도 된다.

### 7.2 지금은 약하게만 적용하는 계층

#### Movement
- `bUseMovementOverrides`
- `MovementProfileName`
- 나머지는 Reserved / Pending

#### WheelVisual
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`
- `bUseWheelVisualOverrides`
- 나머지는 Reserved / Pending

즉, 지금 단계에서 적용 구조는 갖추되, 세부 숫자 적용은 일부러 비워 둬야 한다.

---

## 8. Reserved / Pending 전략

이 문서에서 중요한 운영 원칙은 `Reserved / Pending` 전략이다.

현재 단계에서는 아래 필드들이 실제 구현에서 비어 있을 수 있다.

### Movement Pending 대상
- 엔진 토크
- RPM
- 브레이크 토크
- 핸드브레이크 세팅
- 기어비
- 디퍼렌셜 세팅
- 휠 반경 / 폭
- 서스펜션 / 마찰 세팅

### WheelVisual Pending 대상
- 조향 시각 보정치
- 부호 보정치
- 서스펜션 오프셋 보정치
- 스핀 보정치
- 이름 매핑 데이터

이 값들은 지금 단계에서 아래처럼 다루는 것이 맞다.

- 문서상으로는 확장 포인트로 남긴다.
- 실제 코드에서는 비어 있는 슬롯 또는 TODO 구조로 남긴다.
- 현재 구현 기준값처럼 보이게 하면 안 된다.

즉, 보류는 “잊어버리는 것”이 아니라 **다음 단계 확장 포인트를 명시적으로 남기는 것**이다.

---

## 9. `VehicleData` 슬롯 공식화 방향

현재 `CFVehiclePawn`의 `VehicleData`는 느슨한 타입으로 열려 있을 가능성이 높다.  
이 상태는 장기적으로 유지하면 안 된다.

### 장기 권장 방향
- `VehicleData`는 `UCFVehicleData*` 또는 그에 준하는 명확한 타입으로 수렴해야 한다.

### 현재 단계 권장 방향
- 코드 전체 교체는 보류 가능
- 문서 기준과 신규 구조 기준은 `UCFVehicleData`로 고정
- 신규 차량 데이터는 `UCFVehicleData` 기반으로 만든다
- 레거시 `PDA_VehicleArchetype`는 브리지 대상으로만 유지한다

즉, 지금 단계에서 중요한 것은 **문서상 기준을 먼저 고정**하는 것이다.

---

## 10. `BP_Wheel_Front`, `BP_Wheel_Rear`와의 관계

이 초안 설계서에서는 Wheel BP를 당장 없애지 않는다.

그 이유는 아래와 같다.

- 휠 반경 / 폭 / 조향 / 핸드브레이크 / 서스펜션 계열의 실제 값이 아직 정확히 확정되지 않았다.
- Wheel BP가 부품 프리셋 역할을 일부 하고 있을 가능성이 높다.
- 현재 MCP에서 이 값들을 안정적으로 읽지 못하는 상태다.

따라서 현재 단계 권장 방향은 아래다.

### 현재
- `FrontWheelClass`, `RearWheelClass`를 DataAsset이 참조한다.
- 실제 휠 개별 수치는 Wheel BP가 계속 들고 있어도 된다.

### 나중
- MCP 가시화 후 Wheel BP 값 인벤토리 완료
- 중복이 많으면 DataAsset 하위 구조 또는 부품 프리셋으로 승격

즉, 현재 초안 설계는 **Wheel BP 유지 전제**로 쓰는 것이 맞다.

---

## 11. `CFWheelSyncComp`와 WheelVisual 계층의 연결

현재 단계에서 WheelVisual은 최소 구조만 가지지만, 연결 원칙은 지금 먼저 잡아도 된다.

### 권장 연결 원칙
- `ExpectedWheelCount`는 WheelSync의 기본 검증 기준으로 사용 가능
- `FrontWheelCountForSteering`는 조향 대상 수 판단에 사용 가능
- 이름 규약은 가능하면 C++ 표준 규약으로 유지
- 예외 차량이 생기기 전까지 이름 매핑 배열은 보류

### 현재 단계에서 넣지 않는 것
- 상세 축 보정 수치
- 부호 보정 수치
- 강제 오프셋
- Debug helper 전환값

즉, `CFWheelSyncComp`는 현재 초안 설계에서 **최소한의 차종 데이터만 받고, 나머지는 기존 Debug/검증 체계를 유지**하는 편이 맞다.

---

## 12. 현재 단계 구현 범위 제안

이 초안 설계서를 실제 작업 강도로 바꾸면 아래 정도가 적당하다.

### 지금 구현해도 되는 것

#### A. `UCFVehicleData` 구조
- 최소 상위 구조체 추가
- Visual / Layout / Movement / WheelVisual / References 필드 추가

#### B. `CFVehiclePawn` 쪽
- `VehicleData`가 `UCFVehicleData` 경로로 수렴하도록 방향 고정
- 최소한의 DataAsset 읽기 진입점 설계
- Visual / Layout / References 소비 루틴 또는 TODO 위치 정리

#### C. `CFWheelSyncComp` 쪽
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`
- 최소 WheelVisual 소비 지점 설계

### 지금 구현하지 않는 것

#### A. VehicleMovement 전면 자동 적용
- 엔진/브레이크/기어/디퍼렌셜 전부 자동 반영하는 구현

#### B. WheelVisual 세부 보정 적용
- SteeringYawScaleDeg
- Sign 보정
- 자세한 오프셋 반영

#### C. 프리셋 계층 분리
- MovementPreset
- WheelVisualPreset
- TirePreset

즉, 지금 단계에서는 **경로를 만든다**, 그러나 **세부 수치 전면 적용은 하지 않는다**.

---

## 13. 나중에 확장할 때의 연결 변화

MCP 수치 가시화가 완료되면 현재 초안 설계는 아래 식으로 자연스럽게 확장될 수 있다.

### 현재
```text
UCFVehicleData
├── Visual (실제 사용)
├── Layout (실제 사용)
├── Movement (최소 슬롯)
├── WheelVisual (최소 슬롯)
└── References (실제 사용)
```

### 확장 후
```text
UCFVehicleData
├── Visual
├── Layout
├── Movement
│   ├── Engine
│   ├── Steering
│   ├── Brake
│   ├── Transmission
│   ├── Differential
│   └── WheelPhysics
├── WheelVisual
│   ├── SteeringVisual
│   ├── SuspensionVisual
│   ├── SpinVisual
│   └── Naming / Mapping
└── References
```

즉, 현재 초안 설계는 나중에 갈아엎을 구조가 아니라 **확장 전 기본 프레임**으로 설계되어야 한다.

---

## 14. 리스크와 대응

### 리스크 1. 초안 설계가 실제 구현처럼 오해된다

설명:
- 현재 문서는 구현 직전 초안이므로, 일부 필드는 실제 값이 비어 있을 수 있다.

대응:
- 문서 곳곳에 `Reserved / Pending / TODO` 개념을 분명히 남긴다.
- 실제 기준값 문서처럼 쓰지 않는다.

### 리스크 2. `CFVehicleDriveComp`까지 데이터 해석 책임이 퍼진다

설명:
- 구동 컴포넌트가 DataAsset까지 직접 해석하면 책임이 분산된다.

대응:
- DataAsset 해석 시작점은 `CFVehiclePawn`으로 유지한다.
- 구동 컴포넌트는 적용된 결과를 소비하는 계층으로 유지한다.

### 리스크 3. Wheel BP와 DataAsset의 책임 경계가 흐려진다

설명:
- 휠 관련 값 일부가 Wheel BP에 남아 있고, 일부가 DataAsset으로 가면 기준이 흔들릴 수 있다.

대응:
- 현재 단계에서는 Wheel Class 참조만 DataAsset이 소유한다.
- 휠 세부 물리값은 MCP 가시화 후 인벤토리 완료 뒤 이전 여부를 결정한다.

### 리스크 4. 로드맵 문서와 초안 설계서가 따로 놀게 된다

설명:
- 상위 로드맵과 하위 설계 문서의 단계 상태가 맞지 않을 수 있다.

대응:
- 이 문서를 로드맵의 다음 단계 문서로 명시한다.
- 보류 기준과 재개 기준을 로드맵에도 반영한다.

---

## 15. 현 단계 최종 판단

현재 단계에서 가장 좋은 차량 데이터 적용 구조는 아래처럼 정리할 수 있다.

- `UCFVehicleData`는 루트 저장소다.
- `CFVehiclePawn`는 데이터 적용 진입점이자 오케스트레이터다.
- `VehicleMovementComp`는 물리 적용 대상이다.
- `CFWheelSyncComp`는 WheelVisual 적용기다.
- `CFVehicleDriveComp`는 구동 입력 소비 계층이다.
- `BP_CFVehiclePawn`는 Thin BP 조립 계층이다.

그리고 현재 MCP 한계를 반영하면 구현 강도는 아래가 맞다.

- Visual / Layout / References는 지금 적용 경로를 설계한다.
- Movement / WheelVisual은 최소 슬롯만 설계한다.
- 실제 세부 수치 이관은 MCP 가시화 이후 확장한다.

따라서 이번 초안 설계서의 최종 판단은 아래와 같다.

> **지금 단계에서는 `UCFVehicleData` 기반 차량 데이터 적용 구조의 경로와 책임만 먼저 확정하고, 실제 튜닝 수치 자동 적용은 MCP 수치 가시화 이후 확장하는 것이 가장 안전하다.**

이 기준이면 지금 프로젝트 규모에 비해 과도하지 않고, 나중에 구조를 갈아엎지 않고 그대로 이어서 확장할 수 있다.

---

## 다음 권장 작업

1. 이 초안 설계서를 기준으로 `UCFVehicleData.h` 구현 직전 초안 설계 문서를 작성한다.
2. `CFVehiclePawn`의 `VehicleData` 공식 타입 수렴 방향을 문서화한다.
3. Visual / Layout / References 우선 연결 경로를 정리한다.
4. MCP 수치 가시화 완료 후 Movement / WheelVisual 확장 설계를 갱신한다.
