# CarFight Vehicle DA 입력 보조 툴 기능개발계획서 v0.1.2

- 작성일: 2026-06-23
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 기능: 새 차량 추가 시 `CFVehicleData` 기반 DA 입력을 돕는 에디터 보조 툴
- 문서 상태: Draft
- 기준 구현:
  - 차량 루트 데이터 클래스: `UCFVehicleData`
  - 기준 헤더: `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - 기준 대표 자산: `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan`
  - 기준 소비 주체: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 관련 문서:
  - `Document/Systems/Vehicles/VehicleData.md`
  - `Document/Plan/DataPlan/CF_VehicleDataPlan.md`
  - `Document/Plan/DataPlan/CF_VehicleDataRoadmap.md`
  - `Document/Plan/DataPlan/CF_VehicleDataInventory.md`
  - `Document/Plan/DataPlan/CF_DrivingFeelTunePlan.md`

---

## 1. 문서 목적

이 문서는 새 차량을 추가할 때 `DA_*` 차량 데이터 자산을 안전하게 채우기 위한 기능 개발 계획서다.

현재 `CFVehicleData`는 차량 외형, 레이아웃, Movement, WheelVisual, Wheel Class 참조, DriveState 설정을 한 자산에 묶는다. 이 구조는 차량별 데이터를 한곳에서 관리하기 좋지만, 실제로 새 차량을 만들 때는 입력해야 할 값이 많고 각 수치가 차량에 어떤 영향을 주는지 바로 알기 어렵다.

따라서 이번 기능의 목표는 자동으로 좋은 차량을 튜닝하는 것이 아니라, **사람이 DA를 빠짐없이 채우고, 기준값과 비교하고, 위험한 수치를 인지하고, 테스트 순서대로 검증할 수 있게 돕는 툴**을 만드는 것이다.

---

## 2. 현재 문제

### 2.1 DA 입력 항목이 많다

현재 `CFVehicleData`는 아래 설정 묶음을 가진다.

- `VehicleVisualConfig`
- `VehicleLayoutConfig`
- `VehicleMovementConfig`
- `WheelVisualConfig`
- `VehicleReferenceConfig`
- `DriveStateConfig`

이 중 `VehicleMovementConfig`와 `DriveStateConfig`는 숫자 입력 항목이 많고, 값의 단위와 영향이 서로 다르다.

### 2.2 어떤 값을 먼저 채워야 하는지 알기 어렵다

새 차량을 만들 때 초보자가 모든 값을 같은 중요도로 보면 작업 순서가 흐트러진다.

실제로는 아래 순서가 더 안전하다.

1. 필수 자산 참조
2. 측정 가능한 레이아웃 값
3. 기준 DA에서 복사할 값
4. 감각 튜닝 값
5. 위험도가 높은 고급 물리 값

### 2.3 수치 변화의 체감 결과를 예측하기 어렵다

예를 들어 `EngineMaxTorque`, `FrontWheelMaxSteerAngle`, `FrictionForceMultiplier`, `SpringRate`, `CenterOfMassOverride`는 모두 차량 움직임에 영향을 주지만, 영향 방식과 위험도가 다르다.

이 설명이 에디터 입력 흐름 안에 없으면 사용자는 값을 바꾸고 PIE를 반복하면서 감으로만 판단하게 된다.

### 2.4 기존 기준값과 비교하기 어렵다

현재 대표 기준 자산은 `DA_TestSedan`이다.

새 차량 DA를 만들 때 기준값과 다른 항목만 빠르게 확인할 수 있어야 한다. 그렇지 않으면 어떤 값을 의도적으로 바꿨는지, 어떤 값이 실수로 달라졌는지 구분하기 어렵다.

### 2.5 이미 있는 자동화가 입력 흐름에 통합되어 있지 않다

현재 `UCFVehicleData`에는 `차체 소켓에서 휠 레이아웃 캡처 (Capture Wheel Layout From Chassis Sockets)` 에디터 버튼이 있다.

이 기능은 유용하지만, 새 차량 DA를 만드는 전체 흐름 안에서는 아직 “언제 눌러야 하는지”, “누르기 전에 무엇을 준비해야 하는지”, “실패하면 무엇을 봐야 하는지”가 분리되어 있다.

---

## 3. 핵심 방향

이번 기능은 `Vehicle DA Wizard` 성격으로 개발한다.

핵심 방향은 아래와 같다.

1. 기준 DA를 복제해서 시작한다.
2. 필수 입력값 누락을 먼저 막는다.
3. 기존 `CaptureLayoutFromChassisSockets()`를 적극 활용한다.
4. 수치 입력은 한 화면에서 전부 바꾸게 하지 않고 그룹별로 나눈다.
5. 각 수치의 영향과 위험도를 짧게 보여준다.
6. `DA_TestSedan`과 변경값을 비교한다.
7. PIE 검증 체크리스트와 테스트 결과 메모를 남긴다.

이번 기능은 자동 튜닝기가 아니라 **입력 보조, 검증 보조, 학습 보조 툴**이다.

---

## 4. 비목표

이번 1차 기능에서 하지 않는 것은 아래와 같다.

- 자동으로 최적 주행 세팅을 계산하지 않는다.
- AI가 차량 물리를 자동 밸런싱하지 않는다.
- 외부 Excel/CSV 중심 파이프라인을 먼저 만들지 않는다.
- `CFVehicleData` 구조를 대규모로 갈아엎지 않는다.
- `BP_CFVehiclePawn`의 런타임 적용 구조를 이번 문서 범위에서 변경하지 않는다.
- Chaos Vehicle 내부 물리 계산을 새로 구현하지 않는다.
- 기준 `DA_TestSedan` 값을 무분별하게 덮어쓰지 않는다.

외부 표 기반 입력은 차량 수가 충분히 늘어난 뒤 검토한다. 현재 단계에서는 UObject 참조, Static Mesh, Wheel Class 같은 에셋 참조가 많으므로 언리얼 에디터 안에서 처리하는 방식이 더 안전하다.

---

## 5. 대상 사용자 흐름

새 차량 추가 작업자는 아래 흐름으로 작업한다.

### Step 1. 기준 DA 선택

기본 기준은 `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan`이다.

툴은 기준 DA를 읽고 새 DA의 초기값으로 사용할 수 있게 한다.

### Step 2. 새 DA 생성

사용자는 새 DA 이름을 입력한다.

권장 이름 형식:

- `DA_차량명`
- 예: `DA_Sedan`, `DA_Truck`, `DA_Sports`

파일명은 짧고 의미 있게 유지한다.

### Step 3. 필수 자산 참조 입력

아래 값은 가장 먼저 채운다.

- 차체 메쉬
- 앞왼쪽 휠 메쉬
- 앞오른쪽 휠 메쉬
- 뒤왼쪽 휠 메쉬
- 뒤오른쪽 휠 메쉬
- 전륜 Wheel Class
- 후륜 Wheel Class

툴은 이 값이 비어 있으면 다음 단계로 넘어가기 전에 경고한다.

### Step 4. 차체 소켓 확인

차체 Static Mesh에는 기본적으로 아래 소켓이 있어야 한다.

- `Wheel_Anchor_FL`
- `Wheel_Anchor_FR`
- `Wheel_Anchor_RL`
- `Wheel_Anchor_RR`

툴은 소켓 존재 여부를 검사한다.

소켓 이름을 다르게 써야 하는 예외 차량은 `VehicleLayoutConfig`의 소켓 이름 필드에 명시한다.

### Step 5. 휠 레이아웃 캡처

소켓 검사가 통과하면 기존 에디터 버튼 흐름을 사용한다.

- 실행 기능: `차체 소켓에서 휠 레이아웃 캡처 (Capture Wheel Layout From Chassis Sockets)`

캡처 후에는 `bUseLayoutOverrides`를 켜고 네 휠 앵커 값이 채워졌는지 확인한다.

### Step 6. 기준값 복사 그룹 확인

처음에는 아래 그룹을 기준 DA 값 그대로 유지한다.

- `DriveStateConfig`
- `WheelVisualConfig`
- 브레이크 기본값
- 마찰 기본값
- 서스펜션 기본값

이 값들은 새 차량이 정상 스폰되고 움직이는지 확인한 뒤 단계적으로 바꾼다.

### Step 7. 감각 튜닝 그룹 수정

차량이 정상 스폰되면 아래 순서로 한 그룹씩 수정한다.

1. 엔진 토크 / RPM
2. 조향각
3. 브레이크 / 핸드브레이크
4. 마찰 / 코너링 강성
5. 서스펜션
6. 중심질량 / 다운포스

한 번에 여러 그룹을 바꾸지 않는다.

### Step 8. PIE 검증

각 그룹 수정 후 PIE에서 아래를 확인한다.

- 스폰 직후 뒤집힘이 없는가
- 직진 가속이 가능한가
- 브레이크가 정상 동작하는가
- 좌우 조향이 정상 반응하는가
- 후진 상태가 정상 판정되는가
- 바퀴 시각 위치가 차체와 맞는가
- DriveState가 튀지 않는가

### Step 9. 결과 메모

툴은 변경한 항목, 테스트 결과, 다음 튜닝 후보를 기록할 수 있어야 한다.

---

## 6. MVP 기능 범위

### v0.1.0 문서/체크리스트 단계

목표:
- 새 차량 DA 입력 절차를 문서로 고정한다.
- 필드 그룹, 우선순위, 위험도를 정의한다.
- 이후 툴 구현 범위를 확정한다.

완료 기준:
- 이 문서가 작성되어 있다.
- 새 차량 DA 입력 순서가 설명되어 있다.
- MVP와 보류 범위가 구분되어 있다.

### v0.2.0 에디터 위젯 프로토타입

목표:
- 에디터 유틸리티 위젯(Editor Utility Widget) 기반으로 DA 입력 흐름을 화면화한다.

후보 에셋명:
- `EUW_VDAWizard`

주요 기능:
- 기준 DA 선택
- 대상 DA 선택
- 필수 참조 누락 표시
- 소켓 검사 결과 표시
- 레이아웃 캡처 안내
- 필드 그룹별 체크리스트 표시

완료 기준:
- 사용자가 새 차량 DA를 열기 전에 누락 항목을 확인할 수 있다.
- 기준 DA와 대상 DA를 나란히 비교하는 최소 화면이 있다.

### v0.3.0 검증 기능 추가

목표:
- 데이터 유효성 검사를 자동화한다.

후보 C++ 파일:
- `UE/Source/CarFight_Re/Public/CFVDAValidator.h`
- `UE/Source/CarFight_Re/Private/CFVDAValidator.cpp`

후보 클래스:
- `UCFVDAValidator`

주요 기능:
- 필수 자산 참조 누락 검사
- 차체 소켓 누락 검사
- Wheel Class 누락 검사
- 휠 개수 기준 검사
- 수치 범위 경고
- 위험값 변경 경고

완료 기준:
- 검사 결과가 `정상 / 경고 / 오류`로 나뉜다.
- 오류가 있으면 새 차량 테스트 전 반드시 확인할 수 있다.

### v0.4.0 수치 영향 도움말 추가

목표:
- 각 수치가 차량에 주는 영향을 툴 안에서 바로 확인할 수 있게 한다.

후보 데이터 자산:
- `DT_VDAFieldHelp`

주요 기능:
- 필드명
- 표시 이름
- 단위
- 영향 설명
- 위험도
- 추천 수정 순서
- 기본 테스트 방법

완료 기준:
- 사용자가 `EngineMaxTorque` 같은 항목을 볼 때 “이 값이 무엇을 바꾸는지” 바로 확인할 수 있다.

### v0.5.0 테스트 기록 기능

목표:
- 새 DA를 튜닝하면서 테스트 결과를 남길 수 있게 한다.

주요 기능:
- 테스트 날짜
- 대상 DA
- 변경한 그룹
- PIE 결과
- 문제 현상
- 다음 수정 후보

완료 기준:
- 같은 차량 DA를 다음 세션에서 이어서 튜닝할 때 이전 판단을 확인할 수 있다.

---

## 7. 필드 그룹과 입력 우선순위

### 7.1 필수 참조 그룹

이 그룹이 비어 있으면 새 차량 테스트를 시작하지 않는다.

대상:
- `ChassisMesh`
- `WheelMeshFL`
- `WheelMeshFR`
- `WheelMeshRL`
- `WheelMeshRR`
- `FrontWheelClass`
- `RearWheelClass`

예상 오류:
- 차체가 보이지 않는다.
- 바퀴가 보이지 않는다.
- Chaos Wheel 설정이 적용되지 않는다.
- 런타임 적용 함수에서 fallback이 발생한다.

### 7.2 레이아웃 그룹

대상:
- `bUseLayoutOverrides`
- `BodyWheelSocketFL`
- `BodyWheelSocketFR`
- `BodyWheelSocketRL`
- `BodyWheelSocketRR`
- `WheelAnchorFL`
- `WheelAnchorFR`
- `WheelAnchorRL`
- `WheelAnchorRR`

입력 원칙:
- 수동 입력보다 차체 소켓 캡처를 우선한다.
- 소켓 이름은 가능하면 표준 이름을 쓴다.
- 예외 차량만 소켓 이름을 바꾼다.

### 7.3 Movement 기본 그룹

대상:
- 휠 반지름
- 휠 폭
- 조향각
- 브레이크 토크
- 핸드브레이크 토크
- 엔진 토크
- 엔진 RPM
- 디퍼렌셜
- 중심질량
- 공기 저항
- 다운포스

입력 원칙:
- 기준 DA 값을 먼저 복사한다.
- 새 차체 크기와 휠 크기에 맞는 측정값부터 바꾼다.
- 감각 튜닝값은 PIE 테스트 후 조정한다.

### 7.4 WheelVisual 그룹

대상:
- `bUseWheelVisualOverrides`
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`

입력 원칙:
- 4륜 기본 차량은 `ExpectedWheelCount = 4`, `FrontWheelCountForSteering = 2`를 기준으로 한다.
- 특수 차량은 별도 검증 단계가 필요하다.

### 7.5 DriveState 그룹

대상:
- Idle 진입/이탈 임계값
- Reversing 진입/이탈 임계값
- Airborne 임계값
- 상태별 최소 유지 시간
- 입력 임계값
- 반대 스로틀 브레이크 해석 여부

입력 원칙:
- 초반에는 기준 DA 값을 유지한다.
- 차량이 너무 쉽게 Idle/Reversing/Airborne으로 튀는 경우에만 조정한다.

---

## 8. 수치 영향 도움말 초안

| 필드 | 영향 | 위험도 | 수정 순서 |
|---|---|---:|---:|
| `EngineMaxTorque` | 가속력과 휠스핀 가능성을 키운다. | 중간 | 1 |
| `EngineMaxRPM` | 고속 성격과 엔진 회전 한계를 바꾼다. | 중간 | 1 |
| `FrontWheelMaxSteerAngle` | 저속 회전력을 키우지만 고속 불안정을 만들 수 있다. | 중간 | 2 |
| `FrontWheelMaxBrakeTorque` | 앞바퀴 제동력을 바꾼다. | 중간 | 3 |
| `RearWheelMaxBrakeTorque` | 뒷바퀴 제동력을 바꾼다. | 중간 | 3 |
| `RearWheelMaxHandBrakeTorque` | 핸드브레이크 회전성과 급제동 성격을 바꾼다. | 중간 | 3 |
| `FrictionForceMultiplier` | 접지력을 바꾸며 너무 높으면 비현실적으로 붙는다. | 높음 | 4 |
| `CorneringStiffness` | 코너 반응을 바꾸며 과하면 튐이 생길 수 있다. | 높음 | 4 |
| `SpringRate` | 서스펜션 단단함을 바꾼다. | 높음 | 5 |
| `SpringPreload` | 기본 차고와 눌림 상태에 영향을 준다. | 높음 | 5 |
| `SuspensionMaxRaise` | 바퀴가 위로 움직일 수 있는 범위를 바꾼다. | 중간 | 5 |
| `SuspensionMaxDrop` | 바퀴가 아래로 움직일 수 있는 범위를 바꾼다. | 중간 | 5 |
| `CenterOfMassOverride` | 전복, 롤링, 무게중심에 큰 영향을 준다. | 매우 높음 | 6 |
| `DragCoefficient` | 고속 감속 성격을 바꾼다. | 중간 | 6 |
| `DownforceCoefficient` | 고속 안정감과 접지감을 바꾼다. | 중간 | 6 |

이 표는 1차 도움말이며, 실제 튜닝 결과가 쌓이면 갱신한다.

---

## 9. 유효성 검사 규칙 초안

### 9.1 오류로 볼 항목

아래는 테스트 전에 반드시 고쳐야 한다.

- `ChassisMesh`가 비어 있음
- `WheelMeshFL`이 비어 있음
- `FrontWheelClass`가 비어 있음
- `RearWheelClass`가 비어 있음
- `bUseMovementOverrides = true`인데 휠 반지름이 0 이하
- `bUseWheelVisualOverrides = true`인데 `ExpectedWheelCount`가 0 이하
- `bUseLayoutOverrides = true`인데 휠 앵커 위치가 전부 0에 가까움

### 9.2 경고로 볼 항목

아래는 바로 막지는 않지만 확인해야 한다.

- FR/RL/RR 휠 메쉬가 비어 있음
- 전륜/후륜 휠 반지름 차이가 큼
- 전륜/후륜 마찰 배수 차이가 큼
- 조향각이 기준 DA보다 지나치게 큼
- 중심질량 오버라이드가 켜져 있음
- `DriveStateConfig`가 기준 DA와 크게 다름

### 9.3 정보로 볼 항목

아래는 사용자가 의도를 확인하면 된다.

- 기준 DA와 다른 엔진 토크
- 기준 DA와 다른 RPM
- 기준 DA와 다른 브레이크 토크
- 기준 DA와 다른 다운포스
- 기준 DA와 다른 DriveState 임계값

---

## 10. 에디터 화면 구성 초안

`EUW_VDAWizard`는 아래 섹션을 가진다.

### 10.1 상단 영역

- 기준 DA 선택
- 대상 DA 선택
- 새 DA 생성 버튼
- 전체 검사 실행 버튼

### 10.2 진행 단계 영역

- 필수 참조
- 소켓/레이아웃
- Movement 기본값
- WheelVisual
- DriveState
- PIE 검증
- 결과 메모

### 10.3 비교 영역

- 기준 DA 값
- 대상 DA 값
- 차이 여부
- 위험도
- 설명

### 10.4 결과 영역

- 오류 목록
- 경고 목록
- 정보 목록
- 다음 권장 작업

---

## 11. 구현 방식 판단

### 기본안

처음에는 에디터 유틸리티 위젯(Editor Utility Widget) 기반으로 만든다.

이유:
- 새 차량 DA 입력은 런타임 기능이 아니라 에디터 작업이다.
- 초반에는 화면 흐름과 체크리스트가 더 중요하다.
- 기존 `CaptureLayoutFromChassisSockets()` 에디터 버튼을 그대로 활용할 수 있다.
- C++ 변경 없이도 일부 입력 보조 흐름을 먼저 검증할 수 있다.

### 보강안

검증 로직이 복잡해지면 `UCFVDAValidator` 같은 C++ 검증 헬퍼를 추가한다.

이유:
- 필수 참조 검사와 수치 범위 검사는 반복된다.
- BP 위젯 안에 검증 규칙이 흩어지면 유지보수가 어렵다.
- C++ 검증 함수로 빼면 문서, 툴, 테스트에서 같은 규칙을 재사용할 수 있다.

### 보류안

외부 CSV/Excel 입력은 보류한다.

이유:
- 현재 DA는 UObject 참조가 많다.
- 외부 표는 자산 참조 경로 오타와 리다이렉트 문제가 생기기 쉽다.
- 차량 수가 충분히 늘어나기 전에는 유지 비용이 이득보다 크다.

---

## 12. 단계별 개발 계획

### Phase 0. 계획 문서 작성

작업:
- 이 문서를 작성한다.
- MVP 범위와 비목표를 고정한다.

완료 기준:
- 다음 세션에서 같은 방향으로 구현을 시작할 수 있다.

### Phase 1. 수동 체크리스트 문서화

작업:
- 새 차량 DA 입력 체크리스트를 별도 문서로 만든다.
- 필수 입력값과 테스트 순서를 짧게 정리한다.

후보 문서:
- `Document/Plan/DataPlan/CF_DAFillChecklist.md`

완료 기준:
- 툴이 없어도 새 차량 DA를 같은 순서로 만들 수 있다.

### Phase 2. 에디터 위젯 초안

작업:
- `EUW_VDAWizard`를 만든다.
- 기준 DA와 대상 DA를 선택하는 UI를 만든다.
- 필수 참조 누락 검사 결과를 보여준다.

완료 기준:
- 대상 DA의 누락값을 화면에서 확인할 수 있다.

### Phase 3. 레이아웃 검사 연결

작업:
- 차체 메쉬 소켓 검사 기능을 추가한다.
- 기존 레이아웃 캡처 버튼 사용 순서를 UI에 안내한다.

완료 기준:
- 소켓 누락 상태를 알 수 있다.
- 레이아웃 캡처 전 준비 상태를 확인할 수 있다.

### Phase 4. 기준 DA 비교

작업:
- `DA_TestSedan`과 대상 DA의 주요 필드 차이를 표시한다.
- 차이가 있는 항목에 위험도와 설명을 붙인다.

완료 기준:
- 의도하지 않은 차이와 의도한 튜닝 차이를 구분할 수 있다.

### Phase 5. 수치 영향 도움말 연결

작업:
- `DT_VDAFieldHelp` 또는 동등한 도움말 데이터 구조를 만든다.
- 각 필드 설명, 단위, 위험도, 테스트 방법을 연결한다.

완료 기준:
- 사용자가 값을 바꾸기 전에 영향과 테스트 방법을 볼 수 있다.

### Phase 6. 테스트 기록 기능

작업:
- 대상 DA별 테스트 메모를 기록한다.
- 변경 그룹, PIE 결과, 다음 수정 후보를 남긴다.

완료 기준:
- 다음 세션에서 이전 튜닝 판단을 이어갈 수 있다.

---

## 13. 검증 방법

### 문서 검증

- 이 문서만 읽어도 기능 목표와 구현 순서를 이해할 수 있어야 한다.
- 기존 `CFVehicleData` 구조와 충돌하지 않아야 한다.
- 새 파일명과 후보 클래스명이 32자를 넘지 않아야 한다.

### 툴 검증

- 기준 DA를 선택할 수 있어야 한다.
- 대상 DA를 선택할 수 있어야 한다.
- 필수 참조 누락을 찾을 수 있어야 한다.
- 소켓 누락을 찾을 수 있어야 한다.
- 기준 DA와 다른 값을 표시할 수 있어야 한다.

### 차량 검증

- 새 DA를 연결한 차량이 PIE에서 스폰된다.
- 직진, 조향, 브레이크, 후진이 동작한다.
- 휠 시각 위치가 차체와 맞는다.
- DriveState가 비정상적으로 튀지 않는다.

---

## 14. 리스크와 대응

### 리스크 1. 툴이 너무 커진다

대응:
- 처음에는 누락 검사와 체크리스트만 만든다.
- 자동 튜닝, CSV 입력, 고급 비교는 뒤로 미룬다.

### 리스크 2. 검증 규칙이 BP 위젯에 흩어진다

대응:
- 규칙이 3개 이상 반복되면 C++ 검증 헬퍼로 분리한다.

### 리스크 3. 기준 DA가 바뀌었는데 툴 기준이 오래된다

대응:
- 기준 DA를 하드코딩하지 않고 사용자가 선택하게 한다.
- 기본 추천값만 `DA_TestSedan`으로 둔다.

### 리스크 4. 도움말이 실제 체감과 다를 수 있다

대응:
- 도움말은 절대값이 아니라 방향성으로 표기한다.
- 테스트 기록을 쌓아 도움말을 갱신한다.

### 리스크 5. 소켓 캡처를 만능으로 오해한다

대응:
- 소켓 캡처는 휠 레이아웃 입력 보조일 뿐이라고 명시한다.
- Movement 튜닝과 DriveState 검증은 별도 단계로 둔다.

---

## 15. Changelog

### v0.1.2 - 2026-06-25

- `DA_PoliceCar` 폐기 결정에 맞춰 DA 입력 보조 툴의 기준 대표 자산을 `DA_TestSedan`으로 변경했다.
- 기준 비교, 복제, 기본 추천 문구를 현재 P0 기본 DA 기준으로 갱신했다.

### v0.1.0 - 2026-06-23

- 새 차량 `CFVehicleData` DA 입력 보조 툴 기능개발계획서 최초 작성
- MVP 범위를 `누락 방지`, `기준값 비교`, `수치 영향 설명`, `테스트 기록`으로 정의
- 기본 구현 방향을 에디터 유틸리티 위젯(Editor Utility Widget) 기반으로 설정
- 기존 `CaptureLayoutFromChassisSockets()` 기능을 전체 입력 흐름에 포함
- 외부 CSV/Excel 파이프라인과 자동 튜닝 기능을 1차 범위에서 제외

### v0.1.1 - 2026-06-23

- 차종별 주행감 튜닝 방향 문서 `CF_DrivingFeelTunePlan.md`를 관련 문서에 추가
- 원시 수치 입력 보조와 주행감 Quick Tune을 별도 계획으로 분리

---

## 16. 다음 작업

다음 문서 작업은 `CF_DAFillChecklist.md` 작성이다.

그 문서는 툴 구현 전에도 바로 사용할 수 있는 실무 체크리스트로 만든다.

권장 다음 순서:

1. `CF_DAFillChecklist.md` 작성
2. `EUW_VDAWizard` 화면 구조 초안 작성
3. 필수 참조 검사 규칙 구현
4. 소켓 검사 규칙 구현
5. 기준 DA 비교 기능 구현
