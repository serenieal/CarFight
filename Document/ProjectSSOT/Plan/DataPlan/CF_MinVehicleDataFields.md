# CarFight 최소 차량 DataAsset 필드 설계표 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 기준 구현: `BP_CFVehiclePawn` / `CFVehiclePawn`
- 문서 목적: `CF_MinVehicleDataSkeleton.md`의 최소 골격안을 실제 `UCFVehicleData` C++ 필드 수준으로 풀어 써서, 구현 직전 참고용 설계표로 사용한다.
- 관련 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleMovementInventory.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataSkeleton.md`

> 상태 주의 (2026-04-01):
> - 이 문서는 초기 설계 기록이다.
> - 본문에 남아 있는 `VehicleLayoutConfig / FCFVehicleWheelLayout / AutoFit / WheelLayout` 언급은 현재 구현 기준이 아니다.
> - 현재 코드 기준은 해당 구조 하드 삭제 완료이며, 실제 운영 기준은 `수동 Wheel_Anchor 배치 + WheelRadius`와 `VehicleVisualConfig / VehicleMovementConfig / WheelVisualConfig / VehicleReferenceConfig / DriveStateConfig`다.

---

## 핵심 결론

지금 단계에서는 `UCFVehicleData`를 완성형 구조로 만들지 않는다.

대신 아래 목표만 달성하면 충분하다.

1. `UCFVehicleData`가 차량 1대의 루트 DataAsset이라는 점을 명확히 한다.
2. 실제 자산 참조가 필요한 필드는 지금 바로 넣는다.
3. 세부 수치가 아직 확정되지 않은 영역은 최소 구조체와 예약 슬롯만 둔다.
4. 나중에 MCP에서 Details 수치를 읽을 수 있게 되면, 기존 필드명을 깨지 않고 하위 구조만 확장한다.

즉, 이 문서의 목적은 “지금 당장 어떤 UPROPERTY를 만들면 되는가”를 정리하는 것이다.

---

## 설계 원칙

### 1. 필드는 많이 만들지 않는다

지금 가장 피해야 하는 것은, 아직 보지 못한 `VehicleMovementComp` 세부값을 예상으로 미리 다 만들어 넣는 것이다.

따라서 현재 단계에서는 다음 기준을 적용한다.

- 지금 확실히 필요한 필드만 만든다.
- 확실하지 않은 값은 예약 구조체로 둔다.
- Debug 계열은 넣지 않는다.
- 차종 데이터와 시스템 정책을 섞지 않는다.

### 2. 이름은 확장 전제를 갖고 짓는다

필드 이름은 나중에 봐도 직관적이어야 한다.

예:

- `VehicleVisualConfig`
- `VehicleLayoutConfig`
- `VehicleMovementConfig`
- `WheelVisualConfig`
- `VehicleReferenceConfig`

이런 식으로 상위 구조체 이름을 먼저 안정적으로 고정하면, 나중에 하위 필드를 늘려도 흔들리지 않는다.

### 3. `bool` 토글은 최소만 둔다

토글이 많아지면 구조가 금방 더러워진다.

지금 단계에서는 아래 정도만 허용하는 것이 좋다.

- `bUseMovementOverrides`
- `bUseWheelVisualOverrides`

그 외 세부 단계 전환 토글은 전부 Debug 또는 C++ 내부 정책으로 남긴다.

---

## 권장 상위 구조체

현재 단계에서 `UCFVehicleData`가 가지는 상위 구조체는 아래가 적당하다.

```text
UCFVehicleData
├── FCFVehicleVisualConfig
├── FCFVehicleLayoutConfig
├── FCFVehicleMovementConfig
├── FCFVehicleWheelVisualConfig
└── FCFVehicleReferenceConfig
```

---

## 1. FCFVehicleVisualConfig

### 역할
차량의 차체/휠 시각 자산 참조를 담는다.

### 지금 바로 넣어도 되는 필드

| 필드명 | 타입 예시 | 지금 필요 여부 | 설명 |
|---|---|---:|---|
| `ChassisMesh` | `TObjectPtr<UStaticMesh>` | 높음 | 차체 메쉬 참조 |
| `WheelMeshFL` | `TObjectPtr<UStaticMesh>` | 높음 | 앞왼쪽 휠 메쉬 |
| `WheelMeshFR` | `TObjectPtr<UStaticMesh>` | 높음 | 앞오른쪽 휠 메쉬 |
| `WheelMeshRL` | `TObjectPtr<UStaticMesh>` | 높음 | 뒤왼쪽 휠 메쉬 |
| `WheelMeshRR` | `TObjectPtr<UStaticMesh>` | 높음 | 뒤오른쪽 휠 메쉬 |

### 지금은 넣지 않는 것
- 머티리얼 배열 전체
- 텍스처 참조 전체
- FX / Audio 참조

이유는 지금 단계에서 차량 데이터 핵심은 “튜닝과 자산 기준점”이지, 표현 리소스 전체 패키징이 아니기 때문이다.

---

## 2. FCFVehicleLayoutConfig

### 역할
차량의 기본 휠 배치와 레이아웃 기준점을 담는다.

### 권장 방향
현재 이미 있는 `FCFVehicleWheelLayout`를 최대한 활용하고, 필요 시 감싸는 구조로 가는 것이 좋다.

### 지금 바로 넣어도 되는 필드

| 필드명 | 타입 예시 | 지금 필요 여부 | 설명 |
|---|---|---:|---|
| `WheelLayout` | `FCFVehicleWheelLayout` | 높음 | 현재 레이아웃 구조 재사용 |
| `bUseManualLayoutOverrides` | `bool` | 중간 | 수동 레이아웃 오버라이드 사용 여부 |

### 지금은 예약만 두는 것이 좋은 필드

| 필드명 | 타입 예시 | 비고 |
|---|---|---|
| `ManualWheelOffsetFL` | `FVector` | 실제 필요가 보일 때 추가 |
| `ManualWheelOffsetFR` | `FVector` | 실제 필요가 보일 때 추가 |
| `ManualWheelOffsetRL` | `FVector` | 실제 필요가 보일 때 추가 |
| `ManualWheelOffsetRR` | `FVector` | 실제 필요가 보일 때 추가 |

현재 단계에서는 이 오프셋 필드들을 미리 넣지 않고, `WheelLayout`만 유지하는 쪽이 더 안전하다.

---

## 3. FCFVehicleMovementConfig

### 역할
나중에 `VehicleMovementComp`에 적용할 차종 튜닝 데이터를 담는 최소 구조체다.

### 현재 단계 핵심
이 구조체는 **세부 수치 저장소**가 아니라 **확장 가능한 예약 슬롯**에 가깝다.

### 지금 바로 넣어도 되는 필드

| 필드명 | 타입 예시 | 지금 필요 여부 | 설명 |
|---|---|---:|---|
| `bUseMovementOverrides` | `bool` | 높음 | 이 차량이 별도 Movement 설정을 쓰는지 여부 |
| `MovementProfileName` | `FName` | 중간 | 임시 식별용 이름 또는 메모용 구분자 |

### 지금은 예약 구조체로 두는 것이 좋은 필드

| 필드명 | 타입 예시 | 설명 |
|---|---|---|
| `ReservedEngineConfig` | `FCFReservedEngineConfig` | 엔진 수치 확장 예약 |
| `ReservedSteeringConfig` | `FCFReservedSteeringConfig` | 조향 수치 확장 예약 |
| `ReservedBrakeConfig` | `FCFReservedBrakeConfig` | 브레이크 수치 확장 예약 |
| `ReservedTransmissionConfig` | `FCFReservedTransmissionConfig` | 변속 수치 확장 예약 |
| `ReservedWheelPhysicsConfig` | `FCFReservedWheelPhysicsConfig` | 휠 반경/폭/서스펜션/마찰 확장 예약 |

### 현재 단계에서 중요한 판단
위 예약 구조체는 실제 필드를 빽빽하게 채울 필요가 없다.  
현재는 아래 둘 중 하나만 해도 충분하다.

1. 빈 구조체만 선언해 둔다.  
2. 또는 아예 아직 선언하지 않고 `FCFVehicleMovementConfig` 자체만 먼저 만든다.

현재 프로젝트 규모를 감안하면 **2번이 더 단순하고 안전하다.**

즉, 지금은 실제 구현 시 아래 정도가 적당하다.

- `bUseMovementOverrides`
- `MovementProfileName`

그리고 나머지는 문서에만 확장 포인트로 남긴다.

---

## 4. FCFVehicleWheelVisualConfig

### 역할
나중에 `CFWheelSyncComp`가 읽을 차종별 휠 시각 데이터를 담는 최소 구조체다.

### 지금 바로 넣어도 되는 필드

| 필드명 | 타입 예시 | 지금 필요 여부 | 설명 |
|---|---|---:|---|
| `bUseWheelVisualOverrides` | `bool` | 높음 | 이 차량이 WheelVisual 오버라이드를 쓰는지 여부 |
| `ExpectedWheelCount` | `int32` | 높음 | 기대 바퀴 개수 |
| `FrontWheelCountForSteering` | `int32` | 높음 | 조향 대상 전륜 개수 |

### 지금은 넣지 않는 것이 좋은 필드
- `SteeringYawScaleDeg`
- `SteeringVisualSign`
- `SuspensionVisualSign`
- `WheelSpinVisualSign`
- 상세 오프셋값
- 이름 매핑 배열

이유는 아직 실제 기준값과 필요 범위를 확정하지 못했기 때문이다.

---

## 5. FCFVehicleReferenceConfig

### 역할
차량이 직접 사용하는 참조형 자산을 담는다.

### 지금 바로 넣어도 되는 필드

| 필드명 | 타입 예시 | 지금 필요 여부 | 설명 |
|---|---|---:|---|
| `FrontWheelClass` | `TSubclassOf<UChaosVehicleWheel>` | 높음 | 전륜 Wheel Class |
| `RearWheelClass` | `TSubclassOf<UChaosVehicleWheel>` | 높음 | 후륜 Wheel Class |

### 후보지만 지금은 보류 가능한 필드
| 필드명 | 타입 예시 | 비고 |
|---|---|---|
| `TorqueCurve` | `TObjectPtr<UCurveFloat>` | 실제 사용이 확인될 때 추가 |
| `SteeringCurve` | `TObjectPtr<UCurveFloat>` | 실제 사용이 확인될 때 추가 |
| `BrakeCurve` | `TObjectPtr<UCurveFloat>` | 필요가 보이면 추가 |

현재는 휠 클래스 참조만으로도 충분히 의미가 있다.

---

## UCFVehicleData 권장 필드 구성

현재 단계에서 `UCFVehicleData`는 아래처럼 구성하는 것이 가장 단순하다.

| 필드명 | 타입 예시 | 설명 |
|---|---|---|
| `VehicleVisualConfig` | `FCFVehicleVisualConfig` | 차체/휠 시각 자산 참조 |
| `VehicleLayoutConfig` | `FCFVehicleLayoutConfig` | 휠 배치 / 레이아웃 |
| `VehicleMovementConfig` | `FCFVehicleMovementConfig` | Movement 예약 슬롯 |
| `WheelVisualConfig` | `FCFVehicleWheelVisualConfig` | WheelSync 예약 슬롯 |
| `VehicleReferenceConfig` | `FCFVehicleReferenceConfig` | Wheel Class 등 참조 자산 |

---

## 지금 구현 시 주의할 점

### 1. `VehicleData` 타입을 너무 늦게까지 느슨하게 두지 않는다

장기적으로는 `CFVehiclePawn`의 `VehicleData`가 `UPrimaryDataAsset*`가 아니라 `UCFVehicleData*` 또는 그에 준하는 명확한 타입으로 수렴해야 한다.

지금 당장 전면 교체를 안 하더라도, 최소한 문서상 기준은 `UCFVehicleData`로 고정해야 한다.

### 2. Blueprint 편집 편의성을 고려한다

초보자가 보기 쉬우려면 상위 구조체 카테고리가 너무 복잡하면 안 된다.

따라서 현재 단계에서는 아래가 좋다.

- 구조체 수는 적게 유지
- 카테고리 이름은 직관적으로 유지
- 실제 값이 없는 구조는 과하게 펼치지 않기

### 3. Debug 필드를 섞지 않는다

중요한 원칙이다.

아래 계열은 절대 `UCFVehicleData`에 넣지 않는다.

- 로그 토글
- helper compare
- 강제 시각 오프셋
- 테스트 전환 토글

이 값들은 전부 `CFWheelSyncComp` 또는 Debug 전용 레이어에 남아야 한다.

---

## 지금 단계 권장 구현안

현재 시점에서 실제 구현 수준으로 권장하는 최소안은 아래다.

### 구현 우선 대상
- `FCFVehicleVisualConfig`
- `FCFVehicleLayoutConfig`
- `FCFVehicleMovementConfig`
- `FCFVehicleWheelVisualConfig`
- `FCFVehicleReferenceConfig`
- `UCFVehicleData` 상위 필드 5개

### 실제 값이 들어가는 필드
- `ChassisMesh`
- `WheelMeshFL/FR/RL/RR`
- `WheelLayout`
- `FrontWheelClass`
- `RearWheelClass`
- `bUseMovementOverrides`
- `MovementProfileName`
- `bUseWheelVisualOverrides`
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`

### 아직 넣지 않는 필드
- 엔진 / 브레이크 / 조향 / 기어의 세부 숫자
- WheelVisual 세부 보정 숫자
- 프리셋 참조 계층
- 디버그 전환 토글

---

## 최종 판단

지금 단계에서 가장 좋은 선택은, `UCFVehicleData`를 **크게 완성하지도 않고, 너무 빈 껍데기로 두지도 않는 것**이다.

즉 아래 수준이 가장 적당하다.

- 차량 한 대를 대표하는 루트 구조는 만든다.
- 실제 자산 참조는 바로 넣는다.
- 수치가 불명확한 영역은 구조 이름만 먼저 잡는다.
- 세부 수치는 MCP 수치 가시화 이후 확장한다.

이렇게 하면 지금 구현을 시작할 수 있고, 나중에 실제 값을 확인한 뒤 구조를 깨지 않고 이어서 확장할 수 있다.

---

## 다음 권장 작업

1. 이 설계표 기준으로 `UCFVehicleData` 최소 필드 초안 검토
2. `CFVehiclePawn`의 `VehicleData` 공식 타입 방향 정리
3. 경찰차 1대에 Visual / Layout / References만 우선 연결
4. MCP 수치 가시화 완료 후 Movement / WheelVisual 하위 구조 확장
