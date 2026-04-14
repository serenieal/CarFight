# CarFight UCFVehicleData.h 초안 설계서 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 파일: `UE/Source/CarFight_Re/Public/CFVehicleData.h`
- 기준 구현: `BP_CFVehiclePawn` / `CFVehiclePawn`
- 문서 목적: 현재 `CFVehicleData.h`의 실제 상태를 기준으로, 지금 단계에서 안전하게 확장 가능한 `UCFVehicleData.h` 초안 구조를 설계한다.
- 관련 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataRoadmap.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataSkeleton.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataFields.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataDraftDesign.md`
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`

> 상태 주의 (2026-04-01):
> - 이 문서는 `CFVehicleData.h` 초기 확장 초안 기록이다.
> - 본문 일부에는 현재 기준선에서 제거된 레거시 레이아웃 설계 흔적이 남아 있을 수 있다.
> - 실제 구현 판단은 현재 소스 파일과 루트 활성 SSOT 문서를 우선한다.

---

## 핵심 결론

현재 `CFVehicleData.h`는 이미 좋은 출발점을 갖고 있다.

현재 실제 상태는 아래와 같다.

- `UCFVehicleData`가 이미 존재한다.
- `FCFVehicleWheelLayout`가 이미 존재한다.
- 차체/휠 메쉬와 기본 레이아웃을 Native 타입으로 고정하는 목적이 이미 반영되어 있다.

즉, 지금 필요한 것은 `CFVehicleData.h`를 처음부터 다시 설계하는 것이 아니라, **현재 v1.0.0 헤더를 기준으로 v1.1.0 수준의 최소 확장을 설계하는 것**이다.

이번 문서의 결론은 아래와 같다.

1. 현재 `ChassisMesh`, `WheelMeshFL/FR/RL/RR`, `WheelLayout`는 유지한다.
2. `UCFVehicleData`에 바로 모든 Movement 수치를 넣지 않는다.
3. 먼저 상위 구조를 나누는 최소 구조체를 추가한다.
4. 지금 실제 필드로 넣는 것은 `Visual`, `Layout`, `WheelVisual 최소 필드`, `WheelClass 참조`까지만 제한한다.
5. `Movement`는 현재 단계에서 `bUseMovementOverrides`, `MovementProfileName` 정도까지만 두고 세부 수치는 보류한다.
6. `CFVehicleData.h`는 지금 단계에서 완성형 헤더가 아니라, **MCP 수치 가시화 이후 안전하게 확장 가능한 뼈대 헤더**가 되어야 한다.

---

## 1. 현재 `CFVehicleData.h` 실제 상태 정리

현재 헤더에는 아래 구조가 이미 있다.

### 1.1 현재 존재하는 타입
- `FCFVehicleWheelLayout`
- `UCFVehicleData`

### 1.2 현재 `FCFVehicleWheelLayout`가 다루는 값
- `FrontAxleMargin`
- `RearAxleMargin`
- `HeightOffset`
- 레이아웃 관련 보조 정보

### 1.3 현재 `UCFVehicleData`가 가지는 값
- `ChassisMesh`
- `WheelMeshFL`
- `WheelMeshFR`
- `WheelMeshRL`
- `WheelMeshRR`
- `WheelLayout`

### 1.4 현재 헤더의 장점
- 이미 Native DataAsset로 출발했다.
- 변수명과 툴팁이 비교적 직관적이다.
- “값의 해석과 적용 규칙은 C++가 담당한다”는 방향이 이미 주석에 들어가 있다.
- 레이아웃 구조와 시각 자산 구조가 최소 단위로 잡혀 있다.

### 1.5 현재 헤더의 한계
- Visual / Layout / Movement / WheelVisual / References로 상위 계층이 아직 분리되어 있지 않다.
- 휠 클래스 참조가 없다.
- WheelVisual 최소 설정이 없다.
- Movement 계층 예약 슬롯이 없다.
- 아직은 사실상 “시각 자산 + 기본 레이아웃 DataAsset”에 가깝다.

즉, 현재 헤더는 틀린 헤더가 아니라 **범위가 아직 좁은 헤더**다.

---

## 2. 이번 초안의 목표

이번 초안 설계서의 목표는 아래 3가지를 동시에 만족하는 것이다.

### 목표 1. 현재 헤더를 최대한 존중한다

기존 필드를 괜히 대규모로 깨지 않는다.

### 목표 2. 나중에 확장 가능한 상위 구조를 만든다

지금은 수치를 못 봐도, 나중에 `VehicleMovementComp`와 `CFWheelSyncComp` 데이터가 들어갈 자리를 만든다.

### 목표 3. 지금 단계에서 과설계하지 않는다

아래는 아직 하지 않는다.

- 엔진/브레이크/조향/기어 세부 필드 전부 선언
- 프리셋 DataAsset 계층 선언
- WheelVisual 상세 보정값 전부 선언
- Debug 필드 편입

즉, 이번 초안은 **보수적 확장안**이어야 한다.

---

## 3. 권장 버전 전략

현재 헤더는 주석 기준 `v1.0.0`이다.

이번 문서 기준 권장 버전 전략은 아래다.

### v1.0.0
현재 상태
- 시각 자산 + 기본 레이아웃

### v1.1.0
이번 초안 목표
- 상위 구조 분리
- Wheel Class 참조 추가
- WheelVisual 최소 필드 추가
- Movement 최소 슬롯 추가

### v1.2.0 이후
MCP 수치 가시화 후 확장
- Engine / Brake / Steering / Transmission / Differential / WheelPhysics 세부 구조 추가
- WheelVisual 상세 보정 구조 추가

즉, 이번 단계는 `v1.1.0` 성격으로 보는 것이 가장 자연스럽다.

---

## 4. 권장 상위 구조체 초안

현재 `UCFVehicleData`를 아래 구조로 바꾸는 방향이 좋다.

```text
UCFVehicleData
├── VehicleVisualConfig
├── VehicleLayoutConfig
├── VehicleMovementConfig
├── WheelVisualConfig
└── VehicleReferenceConfig
```

이 구조의 장점은 아래와 같다.

- 나중에 필드가 늘어나도 카테고리 경계가 유지된다.
- 초보자가 봐도 어떤 값이 어디에 들어가는지 직관적이다.
- `VehicleMovementComp` 쪽과 `CFWheelSyncComp` 쪽이 같은 레벨에서 나뉜다.
- 기존 필드를 점진적으로 이 구조 안으로 이동시키기 쉽다.

---

## 5. 구조체별 권장 설계

## 5.1 `FCFVehicleVisualConfig`

### 역할
차체/휠 시각 자산 참조를 저장한다.

### 권장 필드
- `ChassisMesh`
- `WheelMeshFL`
- `WheelMeshFR`
- `WheelMeshRL`
- `WheelMeshRR`

### 판단
이 구조체는 현재 `UCFVehicleData` 최상위에 있는 기존 필드를 거의 그대로 감싸는 역할이다.  
즉, 위험이 적고 지금 바로 도입해도 좋다.

---

## 5.2 `FCFVehicleLayoutConfig`

### 역할
휠 배치 / 레이아웃 기준을 저장한다.

### 권장 필드
- `WheelLayout`
- `bUseManualLayoutOverrides` 정도까지는 고려 가능

### 판단
현재는 `WheelLayout` 하나만 있어도 충분하다.  
수동 오프셋 필드는 실제 필요가 드러나기 전까지 넣지 않는 편이 더 안전하다.

---

## 5.3 `FCFVehicleMovementConfig`

### 역할
장기적으로 `VehicleMovementComp` 적용 데이터를 담는 최소 슬롯 구조체다.

### 현재 단계 권장 필드
- `bUseMovementOverrides`
- `MovementProfileName`

### 지금 넣지 않는 필드
- Max Torque
- Max RPM
- Idle RPM
- Brake Torque
- Handbrake Torque
- Gear Ratio 배열
- Differential 상세 설정
- 휠 반경 / 폭 / 서스펜션 / 마찰 세부값

### 판단
이 구조체는 지금 단계에서 “비어 있는 껍데기”처럼 보여도 괜찮다.  
중요한 것은 나중에 엔진/브레이크/조향/기어 계층을 어디에 넣을지 자리만 만드는 것이다.

---

## 5.4 `FCFVehicleWheelVisualConfig`

### 역할
장기적으로 `CFWheelSyncComp`가 소비할 차종별 휠 시각 데이터를 담는 최소 구조체다.

### 현재 단계 권장 필드
- `bUseWheelVisualOverrides`
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`

### 지금 넣지 않는 필드
- `SteeringYawScaleDeg`
- `SteeringVisualSign`
- `SuspensionVisualSign`
- `WheelSpinVisualSign`
- 상세 오프셋
- 이름 매핑 배열

### 판단
이 구조체는 지금 최소한의 차종 구분치만 받고, 나머지는 Pending으로 두는 것이 맞다.

---

## 5.5 `FCFVehicleReferenceConfig`

### 역할
차량이 직접 소비하는 참조형 자산을 담는다.

### 현재 단계 권장 필드
- `FrontWheelClass`
- `RearWheelClass`

### 후보지만 보류 가능한 필드
- `TorqueCurve`
- `SteeringCurve`
- `BrakeCurve`

### 판단
현재 프로젝트에 `BP_Wheel_Front`, `BP_Wheel_Rear`가 실제 존재하므로, 휠 클래스 참조는 지금 추가해도 자연스럽다.

---

## 6. `UCFVehicleData` 권장 필드 구성

이번 초안 설계 기준으로 `UCFVehicleData`는 아래처럼 정리하는 것이 가장 적당하다.

```cpp
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.1.0] 차량 시각 자산 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차체와 휠 시각 자산 참조를 묶은 설정입니다."))
	FCFVehicleVisualConfig VehicleVisualConfig;

	// [v1.1.0] 차량 레이아웃 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 시각 휠 배치와 기본 레이아웃 설정입니다."))
	FCFVehicleLayoutConfig VehicleLayoutConfig;

	// [v1.1.0] 차량 이동 설정의 최소 슬롯입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovement 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleMovementConfig VehicleMovementConfig;

	// [v1.1.0] 차량 휠 시각 설정의 최소 슬롯입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="WheelSync 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleWheelVisualConfig WheelVisualConfig;

	// [v1.1.0] 차량 참조 자산 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="전륜과 후륜 Wheel Class 등 차량 참조형 자산 설정입니다."))
	FCFVehicleReferenceConfig VehicleReferenceConfig;
};
```

이 구조는 현재 단계에서 가장 균형이 좋다.

- 너무 비어 있지 않다.
- 너무 과하게 세부 수치를 선언하지도 않는다.
- 나중에 하위 구조를 확장하기 쉽다.

---

## 7. 현재 헤더에서 무엇을 유지하고 무엇을 바꿀지

이 절은 실제 수정할 때 가장 중요하다.

### 7.1 유지하는 것
- `UPrimaryDataAsset` 상속
- `FCFVehicleWheelLayout` 기본 존재
- 현재 메쉬 필드들의 의미
- 툴팁 중심 주석 스타일
- C++가 해석 규칙을 가진다는 방향

### 7.2 구조적으로 바꾸는 것
- 현재 `UCFVehicleData`의 최상위 필드들을 구조체 안으로 묶는다.
- `WheelLayout`도 `VehicleLayoutConfig` 안으로 이동한다.
- 휠 클래스 참조용 구조를 추가한다.
- WheelVisual 최소 구조를 추가한다.
- Movement 최소 구조를 추가한다.

### 7.3 지금은 바꾸지 않는 것
- `FCFVehicleWheelLayout`의 세부 수치 전면 개편
- 엔진/브레이크/조향/기어 상세 구조 선언
- 커브 자산 대량 편입
- 디버그 관련 필드 추가

즉, 이 단계는 **대개편이 아니라 구조 정리**다.

---

## 8. 권장 헤더 초안 예시

아래는 현재 단계 기준으로 가장 추천하는 초안 예시다.

```cpp
// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-03-17
// Description: CarFight 차량 루트 DataAsset 최소 확장안
// Scope: 차량 시각 자산, 기본 레이아웃, Wheel Class 참조, Movement/WheelVisual 최소 슬롯을 Native 타입으로 고정합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChaosVehicleWheel.h"
#include "CFVehicleData.generated.h"

class UStaticMesh;
class UCurveFloat;

/**
 * 차량 휠 레이아웃용 기본 데이터입니다.
 * - 현재 단계에서는 시각 조립과 배치에 필요한 값만 보관합니다.
 * - 값의 해석과 적용 규칙은 반드시 C++가 담당합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleWheelLayout
{
	GENERATED_BODY()

	// [v1.0.0] 앞바퀴 축의 전방 마진(cm)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 전방 기준으로 앞바퀴 축을 얼마나 안쪽에 둘지 결정하는 마진(cm)입니다."))
	float FrontAxleMargin = 40.0f;

	// [v1.0.0] 뒷바퀴 축의 후방 마진(cm)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 후방 기준으로 뒷바퀴 축을 얼마나 안쪽에 둘지 결정하는 마진(cm)입니다."))
	float RearAxleMargin = 40.0f;

	// [v1.0.0] 바퀴 높이 오프셋(cm)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 기준 로컬 공간에서 바퀴 높이 오프셋(cm)입니다."))
	float HeightOffset = 10.0f;
};

/**
 * 차량 시각 자산 설정입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleVisualConfig
{
	GENERATED_BODY()

	// [v1.1.0] 차체 시각용 Static Mesh 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 차체 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> ChassisMesh = nullptr;

	// [v1.1.0] 앞왼쪽 바퀴 시각용 Static Mesh 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="앞왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> WheelMeshFL = nullptr;

	// [v1.1.0] 앞오른쪽 바퀴 시각용 Static Mesh 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="앞오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshFR = nullptr;

	// [v1.1.0] 뒤왼쪽 바퀴 시각용 Static Mesh 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="뒤왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRL = nullptr;

	// [v1.1.0] 뒤오른쪽 바퀴 시각용 Static Mesh 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="뒤오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRR = nullptr;
};

/**
 * 차량 레이아웃 설정입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleLayoutConfig
{
	GENERATED_BODY()

	// [v1.1.0] 차량 시각 휠 배치용 레이아웃 데이터입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 시각 휠 배치에 사용할 기본 레이아웃 데이터입니다."))
	FCFVehicleWheelLayout WheelLayout;
};

/**
 * 차량 이동 설정의 최소 슬롯입니다.
 * - 현재 단계에서는 세부 VehicleMovement 수치를 아직 담지 않습니다.
 * - MCP 수치 가시화 이후 하위 필드를 확장합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMovementConfig
{
	GENERATED_BODY()

	// [v1.1.0] 이 차량이 별도 Movement 오버라이드 구조를 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 이 차량이 별도 VehicleMovement 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseMovementOverrides = false;

	// [v1.1.0] Movement 설정 식별용 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="현재 단계에서는 실제 수치 대신 식별용 이름 또는 메모성 구분자로 사용합니다."))
	FName MovementProfileName = NAME_None;
};

/**
 * 차량 휠 시각 설정의 최소 슬롯입니다.
 * - 현재 단계에서는 세부 보정 수치를 아직 담지 않습니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleWheelVisualConfig
{
	GENERATED_BODY()

	// [v1.1.0] 이 차량이 별도 휠 시각 오버라이드 구조를 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 이 차량이 별도 WheelVisual 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseWheelVisualOverrides = false;

	// [v1.1.0] 기대 바퀴 개수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 휠 시각 동기화에서 기대하는 바퀴 개수입니다."))
	int32 ExpectedWheelCount = 4;

	// [v1.1.0] 조향 대상으로 보는 전륜 개수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 휠 시각 동기화에서 조향 대상으로 보는 전륜 개수입니다."))
	int32 FrontWheelCountForSteering = 2;
};

/**
 * 차량 참조 자산 설정입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleReferenceConfig
{
	GENERATED_BODY()

	// [v1.1.0] 전륜 Wheel Class 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 전륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> FrontWheelClass;

	// [v1.1.0] 후륜 Wheel Class 입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 후륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> RearWheelClass;
};

/**
 * CarFight 차량용 Native DataAsset 입니다.
 * - 차량 한 대의 루트 저장소 역할을 합니다.
 * - 차종별 자산 참조와 최소 설정 슬롯을 소유합니다.
 * - 해석과 적용 규칙은 반드시 C++가 담당합니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.1.0] 차량 시각 자산 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차체와 휠 시각 자산 참조를 묶은 설정입니다."))
	FCFVehicleVisualConfig VehicleVisualConfig;

	// [v1.1.0] 차량 레이아웃 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 시각 휠 배치와 기본 레이아웃 설정입니다."))
	FCFVehicleLayoutConfig VehicleLayoutConfig;

	// [v1.1.0] 차량 이동 설정의 최소 슬롯입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovement 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleMovementConfig VehicleMovementConfig;

	// [v1.1.0] 차량 휠 시각 설정의 최소 슬롯입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="WheelSync 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleWheelVisualConfig WheelVisualConfig;

	// [v1.1.0] 차량 참조 자산 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="전륜과 후륜 Wheel Class 등 차량 참조형 자산 설정입니다."))
	FCFVehicleReferenceConfig VehicleReferenceConfig;
};
```

이 초안은 지금 바로 코드로 옮겨도 큰 무리는 없는 수준이다.  
다만 Movement와 WheelVisual의 세부 수치는 의도적으로 비워 둔 상태다.

---

## 9. 현재 단계에서 일부러 비워 두는 이유

초보자 입장에서는 “어차피 나중에 넣을 거면 지금 다 만들어 두면 되는 것 아닌가?”라고 생각하기 쉽다.

하지만 지금은 그게 더 위험하다.

### 이유 1. 실제 수치 위치가 아직 완전히 확정되지 않았다
- `VehicleMovementComp`에서 읽어야 할 값
- `BP_Wheel_Front`, `BP_Wheel_Rear`에서 읽어야 할 값
- WheelSync 차종 데이터로 옮길 값

이 경계가 MCP 수치 가시화 전에는 완전히 고정되지 않았다.

### 이유 2. 잘못 만든 필드는 나중에 정리 비용이 더 크다
필드가 많아질수록 아래 문제가 생긴다.

- 에디터 Details가 복잡해진다.
- 초보자가 뭘 채워야 하는지 더 헷갈린다.
- 실제 사용 안 하는 필드가 쌓인다.
- 이름이 잘못 잡히면 나중에 전부 바꿔야 한다.

### 이유 3. 지금 단계 목적은 전체 적용기 완성이 아니다
지금은 `UCFVehicleData`를 **공식 루트 저장소로 고정하는 단계**다.  
세부 튜닝 자동화는 그 다음 단계다.

즉, 이번 헤더 초안은 **적절히 비어 있어야 맞다.**

---

## 10. 현재 헤더 수정 시 권장 순서

실제 `CFVehicleData.h`를 수정할 때는 아래 순서를 권장한다.

### Step 1. 기존 필드 백업 개념 유지
- 현재 필드 의미를 잃지 않도록 문서 기준을 먼저 유지

### Step 2. 새 구조체 선언 추가
- `FCFVehicleVisualConfig`
- `FCFVehicleLayoutConfig`
- `FCFVehicleMovementConfig`
- `FCFVehicleWheelVisualConfig`
- `FCFVehicleReferenceConfig`

### Step 3. `UCFVehicleData` 최상위 필드 교체
- 기존 개별 필드를 새 구조체 필드로 묶기

### Step 4. 기존 DataAsset 자산 마이그레이션 경로 점검
- 현재 `DA_PoliceCar`가 레거시이므로 직접 적용은 아니지만, 신규 `UCFVehicleData` 인스턴스 작성 시 어떤 값이 어디로 가는지 확인

### Step 5. `CFVehiclePawn` 적용기와 연결 준비
- 지금 당장은 실제 전면 적용 안 하더라도, 적어도 `VehicleVisualConfig`, `VehicleLayoutConfig`, `VehicleReferenceConfig`를 읽을 방향을 맞춤

---

## 11. 로드맵 기준 현재 위치

이 문서는 현재 문서 흐름에서 아래 위치에 있다.

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
    ↓
CF_VehicleDataHeader.md
```

즉, 이 문서는 다음 질문에 답하는 문서다.

> **“좋다. 그럼 실제 `CFVehicleData.h`는 어떤 모양으로 바꾸면 되나?”**

---

## 12. 최종 판단

현재 `CFVehicleData.h`는 버려야 할 헤더가 아니라, **최소 구조를 이미 잘 시작한 헤더**다.

따라서 이번 단계의 최종 판단은 아래와 같다.

- 현재 헤더는 유지 가치가 높다.
- 다만 차체/휠 메쉬 + 레이아웃 중심 구조를 루트 차량 DataAsset 구조로 한 단계 확장해야 한다.
- 그 확장은 상위 구조 분리 중심으로 진행해야 한다.
- Movement / WheelVisual 세부 수치는 지금 억지로 넣지 않는다.
- `v1.1.0` 수준의 최소 확장 헤더로 먼저 정리한 뒤, MCP 수치 가시화 이후 `v1.2.0+` 단계로 확장하는 것이 가장 안전하다.

즉, 이번 문서의 최종 한 줄 결론은 아래와 같다.

> **`CFVehicleData.h`는 지금 단계에서 “상위 구조를 분리한 최소 루트 차량 DataAsset 헤더”로 확장하고, 세부 튜닝 수치 구조는 MCP 수치 가시화 이후 붙이는 것이 맞다.**

---

## 다음 권장 작업

1. 이 문서를 기준으로 실제 `CFVehicleData.h` 수정 초안 검토
2. `CFVehiclePawn`의 `VehicleData` 타입 수렴 방향 정리
3. `VehicleVisualConfig` / `VehicleLayoutConfig` / `VehicleReferenceConfig` 우선 연결 경로 검토
4. MCP 수치 가시화 완료 후 `FCFVehicleMovementConfig`와 `FCFVehicleWheelVisualConfig` 세부 확장
