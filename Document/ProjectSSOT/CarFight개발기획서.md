# 언리얼 엔진 5.7 기반 'CarFight' 프로젝트를 위한 모듈형 차량 시뮬레이션 및 물리 아키텍처 (V5)

## 1. 서론: 프로젝트 비전 및 기술적 방향성

'CarFight' 프로젝트는 단순한 아케이드 레이싱을 넘어, 파괴 가능한 환경과 차량 간의 물리적 상호작용이 게임플레이의 핵심이 되는 차량 전투 시뮬레이션입니다. 1인 개발자라는 제한된 리소스 환경에서 고품질의 물리 엔진을 구현하기 위해서는 기존의 경직된 워크플로우를 탈피하고, 언리얼 엔진 5.7(Unreal Engine 5.7)의 최신 기능을 극대화하는 전략이 필요합니다.

본 보고서는 **카오스 모듈형 차량 시스템(Chaos Modular Vehicle System, CMVS)**을 채택하여, 외부 DCC 툴(Blender 등)에서의 리깅 과정을 제거하고, 에셋 스토어에서 구한 다양한 비율의 차량을 **자동화된 절차(Procedural)**로 설정하는 파이프라인을 제안합니다.

---

## 2. 차세대 차량 아키텍처: 카오스 모듈형 차량 시스템 (CMVS) 분석

### 2.1 클러스터 유니언(Cluster Union) 기반의 구조적 혁신

언리얼 엔진 5.7의 CMVS는 **클러스터 유니언(Cluster Union)** 컴포넌트를 사용하여 다수의 물리 객체(차체, 바퀴, 무기 등)를 하나의 강체(Rigid Body)로 통합 시뮬레이션합니다. 이를 통해 스켈레탈 메시의 본(Bone) 구조에 얽매이지 않고, 블루프린트 상에서 부품을 조립하는 것만으로 차량을 구성할 수 있습니다.

### 2.2 지오메트리 컬렉션과 파괴 메커니즘

정적 메시 대신 **지오메트리 컬렉션(Geometry Collection)**을 기본 구성 요소로 사용합니다. 이는 차량의 특정 부위가 피격되었을 때 물리적으로 정확하게 파괴되거나 분리되는 연출을 가능하게 하며, 부품 탈락 시 무게 중심(Center of Mass)이 실시간으로 변하여 주행 특성에 영향을 줍니다.

---

## 3. 에셋 정규화 프로토콜 (Asset Normalization)

에셋 스토어의 모델들은 크기(Scale)와 축(Axis)이 제각각이므로, 게임 로직에 투입하기 전 엔진 내에서 **'물리적 기준(Physics Standard)'**을 맞추는 전처리 과정이 필수적입니다.

### 3.1 모델링 모드(Modeling Mode)를 활용한 전처리

`Shift + 5`를 눌러 모델링 모드에 진입하여 다음 작업을 수행합니다. 이 과정은 외부 툴 없이 언리얼 에디터 내에서 수행됩니다.

1. **방향 정렬 및 베이크 (Orientation Bake):**
* 차량의 앞부분이 **X축(Red Axis)**을 향하게 회전시킵니다.
* `XForm` > `BakeRS` (Rotate & Scale) 도구를 실행하여 현재의 회전과 크기 값을 메시에 영구적으로 적용합니다. (Scale은 (1,1,1), Rotation은 (0,0,0)이 되어야 함).


2. **피벗 재설정 (Pivot Center):**
* **차체(Body):** `Pivot` > `Center`를 눌러 XY축 중앙에 맞추고, 높이(Z)는 바닥면(Bottom)에 맞춥니다.
* **바퀴(Wheel):** `Pivot` > `Center`를 눌러 정확히 회전 중심에 위치시킵니다. 피벗이 어긋나면 바퀴가 회전할 때 편심 운동을 하게 됩니다.



---

## 4. 절차적 휠 배치 시스템 (Procedural Wheel Placement)

차량마다 폭과 길이가 다르므로, 일일이 오프셋 수치를 입력하는 대신 **차체(Chassis)의 크기(Bounding Box)를 측정하여 바퀴를 자동으로 부착하는 로직**을 구현합니다.

### 4.1 데이터 에셋 확장: 자동화 옵션

```
`UVehicleArchetype` 데이터 에셋에 자동 계산 방식을 지원하는 구조체를 정의합니다.cpp
USTRUCT(BlueprintType)
struct FWheelPlacementConfig
{
GENERATED_BODY()

// [자동화 옵션]
UPROPERTY(EditAnywhere)
bool bAutoFitToChassis = true; // True일 경우 차체 크기 기반 자동 배치

// [여백 설정]
UPROPERTY(EditAnywhere, meta=(EditCondition="bAutoFitToChassis"))
float SideMargin = 0.0f; // 차체 옆면 기준 추가 돌출 거리 (오버펜더 등)

UPROPERTY(EditAnywhere, meta=(EditCondition="bAutoFitToChassis"))
float ForwardMargin = 20.0f; // 차체 앞/뒤 끝에서 안쪽으로 들어오는 거리

UPROPERTY(EditAnywhere, meta=(EditCondition="bAutoFitToChassis"))
float HeightOffset = 10.0f; // 서스펜션 높이 보정

};

```

### 4.2 'Auto-Fit' 알고리즘 로직
`BP_ModularVehicle`의 구성 스크립트(`BuildVehicleFromData`)에서 실행될 로직입니다.

1.  **차체 바운딩 박스 측정:** `GC_Chassis` 컴포넌트의 `GetLocalBounds`를 호출하여 차체의 절반 크기(`BoxExtent`)를 구합니다.
2.  **좌표 계산:**
    *   `WheelX = BoxExtent.X - ForwardMargin`
    *   `WheelY = BoxExtent.Y + SideMargin`
    *   `WheelZ = HeightOffset`
3.  **컴포넌트 배치:** 계산된 `(WheelX, WheelY, WheelZ)`를 기준으로 4개의 바퀴 위치를 설정합니다.

---

## 5. 단일 바퀴 에셋 & 미러링 로직

하나의 바퀴 에셋(왼쪽 기준)만 사용하여 4개의 바퀴를 모두 구성합니다.

1.  **우측 바퀴 미러링 (Visual):**
    *   오른쪽 바퀴(FR, RR)는 Z축으로 180도 회전 `(0, 0, 180)`시켜 림(Rim)이 바깥쪽을 보게 합니다.
2.  **물리 반전 설정 (Physics):**
    *   오른쪽 바퀴의 `VehicleSimWheel` 컴포넌트 설정에서 **`Reverse Direction` = True**로 설정합니다. 이를 누락할 경우 물리 엔진은 바퀴가 거꾸로 달려있다고 판단하여 가속 시 제자리에서 회전하게 됩니다.

---

## 6. 라이브 튜닝 툴 (Live Tuning Tool)

1인 개발자가 에디터 뷰포트에서 즉각적인 피드백을 받을 수 있도록 **Call In Editor** 기능을 활용합니다.

1.  **구현:** 블루프린트 내 `CMD_RebuildVehicle` 커스텀 이벤트를 만들고 디테일 패널에서 `Call In Editor`를 체크합니다.
2.  **워크플로우:**
    *   데이터 에셋에서 '차체 메시'를 변경하거나 '마진' 값을 수정합니다.
    *   디테일 패널의 `Rebuild Vehicle` 버튼을 클릭합니다.
    *   **결과:** 기존 컴포넌트가 파괴되고, 새로운 설정(변경된 차체 크기에 맞춘 바퀴 위치 등)으로 차량이 즉시 재조립됩니다. 게임을 실행(Play)하지 않고도 외형과 비율을 확정할 수 있습니다.

---

## 7. 물리 엔지니어링 데이터 (Archetypes)

### 7.1 아키타입 A: 'Interceptor' (스포츠카)
*   **특성:** 낮은 차체, 넓은 윤거, 고속 주행.
*   **엔진:** V8 자연흡기 (Max RPM 9,000, Torque 470 Nm).
*   **서스펜션:** 단단함 (Damping 0.75), 차체 롤링 억제.
*   **배치:** `SideMargin`을 양수로 주어 '와이드 바디' 스타일 연출.

### 7.2 아키타입 B: 'Juggernaut' (트럭/오프로더)
*   **특성:** 높은 차체, 긴 서스펜션 트래블, 높은 토크.
*   **엔진:** V6 터보 디젤 (Max RPM 6,000, Torque 690 Nm).
*   **서스펜션:** 부드러움 (Damping 0.45), 험지 주파력 중시.
*   **배치:** `HeightOffset`을 낮게(음수) 설정하여 차체를 들어 올리는 '리프트 업' 효과 연출.

---

## 8. 결론

이 V5 설계를 통해 'CarFight' 프로젝트는 **모델러 없이도** 에셋 스토어의 다양한 차량 모델을 즉시 게임에 통합할 수 있는 강력한 파이프라인을 구축했습니다.

1.  **No-Rigging:** 모델링 툴 없이 엔진 내에서 해결.
2.  **Auto-Fit:** 차체 크기에 맞춰 바퀴 자동 부착.
3.  **Live Tuning:** 버튼 하나로 즉각적인 시각적 피드백 확인.

이는 1인 개발자가 기술적 제약을 극복하고 콘텐츠의 양과 질을 동시에 확보할 수 있는 최적의 방법론입니다.

