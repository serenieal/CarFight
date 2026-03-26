# CarFight 차량 데이터 문서 로드맵 v0.1

- 작성일: 2026-03-17
- 대상 프로젝트: `UE/CarFight_Re`
- 기준 구현: `BP_CFVehiclePawn` / `CFVehiclePawn`
- 문서 목적: 현재 세션에서 작성된 차량 데이터 관련 문서들의 역할, 읽는 순서, 작성 순서, 갱신 순서, 보류 기준, 재개 기준을 하나의 상위 로드맵으로 통합한다.
- 관련 상위 문서:
  - `Document/Document_Entry.md`
  - `Document/ProjectSSOT/00_Handover.md`
  - `Document/ProjectSSOT/01_Roadmap.md`
  - `Document/ProjectSSOT/Plan/CarFight_CPP_RePlan.md`
- 관련 하위 문서:
  - `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleMovementInventory.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataSkeleton.md`
  - `Document/ProjectSSOT/Plan/CF_MinVehicleDataFields.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataDraftDesign.md`
  - `Document/ProjectSSOT/Plan/CF_VehicleDataHeader.md`

---

## 핵심 결론

현재 차량 데이터 관련 문서는 수가 많아 보이지만, 실제로는 역할이 다르기 때문에 무작정 하나로 합치면 오히려 읽기 어려워진다.

지금 필요한 것은 문서를 줄이는 것이 아니라, 아래 4가지를 명확히 하는 것이다.

1. **어떤 문서가 기준 문서인가**
2. **어떤 문서가 인벤토리 문서인가**
3. **어떤 문서가 최소 구현 설계 문서인가**
4. **현재 MCP 기능 한계 때문에 어떤 단계에서 보류하고, 어떤 조건이 되면 재개하는가**

즉, 이번 로드맵 문서의 목적은 문서를 합치는 것이 아니라, **문서들의 순서와 관계를 하나의 흐름으로 고정하는 것**이다.

이 문서가 있으면 아래 문제가 줄어든다.

- 같은 내용을 서로 다른 문서에서 다시 찾는 문제
- 지금 당장 봐야 하는 문서와 나중에 봐야 하는 문서가 섞이는 문제
- 아직 확정되지 않은 수치 문서를 구현 기준으로 오해하는 문제
- MCP 가시화 전 단계와 이후 단계를 같은 수준의 작업으로 착각하는 문제

정리하면, 현재 차량 데이터 문서 세트는 아래처럼 해석하는 것이 맞다.

> **`CF_VehicleDataPlan.md`가 방향을 고정하고, `CF_VehicleDataInventory.md`와 `CF_VehicleMovementInventory.md`가 현재 상태를 분해하며, `CF_MinVehicleDataSkeleton.md`와 `CF_MinVehicleDataFields.md`가 MCP 가시화 전 최소 구현 준비를 담당한다.**

그리고 이 문서 `CF_VehicleDataRoadmap.md`는 그 모든 문서의 **상위 안내서**다.

---

## 1. 왜 별도 로드맵 문서가 필요한가

현재 세션에서 차량 데이터 관련 문서는 자연스럽게 아래 순서로 늘어났다.

1. 전체 계획을 잡는 문서
2. 실제 데이터가 어디 흩어져 있는지 분해하는 문서
3. VehicleMovement 중심으로 더 좁혀 보는 문서
4. 지금 단계에서 구현 가능한 최소 골격 문서
5. 그 골격을 실제 필드 수준으로 내린 문서

이 흐름 자체는 잘못된 것이 아니다.

오히려 이런 구조는 아래 장점이 있다.

- 계획과 인벤토리가 섞이지 않는다.
- 확정된 것과 아직 보류인 것이 분리된다.
- 세부 수치를 모르는 상태에서도 구현 준비를 할 수 있다.
- MCP 기능이 늘어났을 때 어떤 문서부터 업데이트해야 할지 명확해진다.

하지만 문서 수가 늘어나면 초보자 입장에서는 아래 혼란이 생긴다.

- 어느 문서가 제일 먼저 읽는 문서인지 모르겠다.
- 이 문서가 구현 기준인지, 참고 문서인지 모르겠다.
- 지금 단계에서 뭘 해야 하는지 모르겠다.
- 나중에 MCP 수치가 보이면 어떤 문서를 먼저 수정해야 하는지 모르겠다.

따라서 지금 시점에서는 상위 로드맵 문서가 필요하다.

이 로드맵 문서는 아래 3가지 역할을 해야 한다.

### A. 문서 역할 구분
각 문서가 무엇을 담당하는지 분명히 말해준다.

### B. 작업 순서 고정
지금 무엇을 먼저 보고, 무엇을 나중에 보는지 순서를 고정한다.

### C. 재개 기준 고정
MCP 수치 가시화 전후에 어떤 작업을 멈추고 다시 시작하는지 기준을 정한다.

---

## 2. 현재 문서 세트의 전체 구조

현재 차량 데이터 문서 세트는 크게 5개 층으로 나누는 것이 가장 이해하기 쉽다.

```text
상위 프로젝트 문서
└── 차량 데이터 상위 로드맵 문서
    ├── 전체 방향 계획 문서
    ├── 현재 상태 인벤토리 문서
    ├── 특정 핵심 계층 인벤토리 문서
    ├── 최소 구현 골격 문서
    └── 최소 필드 설계 문서
```

이 구조를 실제 문서 기준으로 쓰면 아래와 같다.

### 상위 프로젝트 기준 문서
- `Document/Document_Entry.md`
- `Document/ProjectSSOT/00_Handover.md`
- `Document/ProjectSSOT/01_Roadmap.md`
- `Document/ProjectSSOT/Plan/CarFight_CPP_RePlan.md`

### 차량 데이터 상위 로드맵 문서
- `Document/ProjectSSOT/Plan/CF_VehicleDataRoadmap.md`

### 차량 데이터 전체 방향 문서
- `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`

### 차량 데이터 인벤토리 문서
- `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`

### VehicleMovement 특화 인벤토리 문서
- `Document/ProjectSSOT/Plan/CF_VehicleMovementInventory.md`

### 최소 구현 골격 문서
- `Document/ProjectSSOT/Plan/CF_MinVehicleDataSkeleton.md`

### 최소 구현 필드 문서
- `Document/ProjectSSOT/Plan/CF_MinVehicleDataFields.md`

### 구현 직전 적용 구조 초안 설계 문서
- `Document/ProjectSSOT/Plan/CF_VehicleDataDraftDesign.md`

### 실제 헤더 초안 설계 문서
- `Document/ProjectSSOT/Plan/CF_VehicleDataHeader.md`

이렇게 놓고 보면, 문서가 많은 것이 아니라 **문서 단계가 분리되어 있을 뿐**이다.

---

## 3. 문서별 역할 정의

이 절은 가장 중요하다.

문서를 많이 쓸 때 가장 먼저 고정해야 하는 것은 “문서의 역할”이다.  
역할이 겹치면 나중에 서로 다른 문서가 같은 내용을 반복하고, 조금씩 다르게 써서 기준이 흔들린다.

### 3.1 `CF_VehicleDataPlan.md`

### 역할
차량 데이터 정리 전체의 **기준 계획서**다.

### 이 문서가 담당하는 것
- 왜 이 주제를 지금 다뤄야 하는가
- 현재 기준 구현이 무엇인가
- 소유권 원칙이 무엇인가
- 루트 DataAsset 구조를 장기적으로 어떻게 볼 것인가
- 단계별 실행 계획은 무엇인가
- 완료 조건과 리스크는 무엇인가

### 이 문서가 담당하지 않는 것
- 현재 모든 값 목록의 세부 나열
- 엔진 / 브레이크 / 휠 값의 체크리스트 수준 세부표
- 실제 UPROPERTY 설계표

즉, 이 문서는 **방향 문서**다.

---

### 3.2 `CF_VehicleDataInventory.md`

### 역할
현재 차량 데이터가 프로젝트 전체에서 어디에 퍼져 있는지를 분해한 **전체 인벤토리 문서**다.

### 이 문서가 담당하는 것
- 차량 관련 데이터의 생태계를 넓게 본다.
- 차량 코어 클래스, 데이터 에셋, 입력 자산, 표현 자산, 유틸 계층을 모두 한 번에 정리한다.
- 어떤 항목이 차종 데이터 후보인지 넓게 분류한다.

### 이 문서가 담당하지 않는 것
- VehicleMovement 세부 수치 설계
- 최소 `UCFVehicleData` 구조 설계
- 실제 코드 필드 결정

즉, 이 문서는 **지도 문서**다.

---

### 3.3 `CF_VehicleMovementInventory.md`

### 역할
`VehicleMovementComp`와 Wheel BP 계층을 중심으로, 실제 튜닝 후보를 좁혀 보는 **핵심 세부 인벤토리 문서**다.

### 이 문서가 담당하는 것
- VehicleMovement 계열 항목 정리
- Wheel BP가 들고 있을 가능성이 높은 항목 정리
- MCP로 확정 가능한 것과 UE 에디터 확인이 필요한 것을 구분
- DataAsset 이전 우선순위를 정하는 데 필요한 근거 정리

### 이 문서가 담당하지 않는 것
- 전체 차량 데이터 방향 선언
- 최종 루트 구조 확정
- 구현 직전 필드명 고정

즉, 이 문서는 **핵심 문제영역 확대 문서**다.

---

### 3.4 `CF_MinVehicleDataSkeleton.md`

### 역할
MCP가 아직 Details 수치를 읽지 못하는 상태에서, 지금 당장 구현 가능한 **최소 골격 설계 문서**다.

### 이 문서가 담당하는 것
- 루트 `UCFVehicleData`를 어떤 상위 구조로 볼지 정리
- `Visual / Layout / Movement / WheelVisual / References` 같은 상위 계층 이름을 고정
- 지금 넣어도 되는 필드와 나중에 넣을 필드를 구분
- 지금은 프리셋 분리를 보류한다는 운영 원칙 정리

### 이 문서가 담당하지 않는 것
- 실제 C++ 필드명 세부안
- 각 구조체의 타입 예시 표
- UPROPERTY 수준 설계

즉, 이 문서는 **상위 구조 골격 문서**다.

---

### 3.5 `CF_MinVehicleDataFields.md`

### 역할
최소 골격안을 실제 `UCFVehicleData` C++ 필드 수준으로 내린 **구현 직전 필드 설계 문서**다.

### 이 문서가 담당하는 것
- 상위 구조체 이름 제안
- 구조체별 필드 후보 제안
- 지금 넣을 필드 / 보류할 필드 구분
- 실제 구현 시 주의점 정리

### 이 문서가 담당하지 않는 것
- VehicleMovement 실제 수치 확정
- Wheel BP 실제 수치 확정
- 최종 적용 코드 구현

즉, 이 문서는 **구현 직전 필드 설계 문서**다.

---

### 3.6 `CF_VehicleDataDraftDesign.md`

### 역할
`UCFVehicleData`가 실제로 어떤 C++ 계층을 통해 소비되고, 어떤 컴포넌트에 어떤 순서로 적용될지를 정리한 **구현 직전 적용 구조 초안 설계 문서**다.

### 이 문서가 담당하는 것
- `UCFVehicleData` → `CFVehiclePawn` → `VehicleMovementComp` / `CFWheelSyncComp` 흐름 정리
- `CFVehiclePawn`, `CFVehicleDriveComp`, `CFWheelSyncComp`, `BP_CFVehiclePawn`의 책임 분해
- 현재 MCP 한계를 반영한 Reserved / Pending 전략 정리
- 지금 적용 가능한 계층과 아직 보류할 계층 구분

### 이 문서가 담당하지 않는 것
- 최종 C++ 헤더/CPP 코드 작성
- 모든 VehicleMovement 수치 확정
- Wheel BP 실제 세부 수치 확정

즉, 이 문서는 **구현 직전 적용 경로 설계 문서**다.

---

### 3.7 `CF_VehicleDataHeader.md`

### 역할
현재 실제 `CFVehicleData.h` 파일 상태를 기준으로, `UCFVehicleData` 헤더를 어떤 구조로 확장할지 정리한 **실제 헤더 초안 설계 문서**다.

### 이 문서가 담당하는 것
- 현재 `CFVehicleData.h`의 실제 필드 상태 정리
- `v1.0.0`에서 `v1.1.0` 수준으로 어떤 최소 확장을 할지 정리
- `FCFVehicleVisualConfig`, `FCFVehicleLayoutConfig`, `FCFVehicleMovementConfig`, `FCFVehicleWheelVisualConfig`, `FCFVehicleReferenceConfig` 구조 제안
- 실제 헤더 코드 예시 제시

### 이 문서가 담당하지 않는 것
- 실제 CPP 구현
- VehicleMovement 세부 수치 확정
- WheelVisual 상세 보정 수치 확정

즉, 이 문서는 **실제 `UCFVehicleData.h` 수정 기준 문서**다.

---

## 4. 문서 읽는 순서

문서는 순서 없이 보면 중복으로 보인다.  
하지만 올바른 순서로 읽으면 하나의 작업 흐름으로 이어진다.

아래 순서를 기준 읽기 순서로 고정하는 것이 좋다.

### 4.1 처음 진입한 사람이 읽는 순서

1. `Document/ProjectSSOT/00_Handover.md`
2. `Document/ProjectSSOT/01_Roadmap.md`
3. `Document/ProjectSSOT/Plan/CarFight_CPP_RePlan.md`
4. `Document/ProjectSSOT/Plan/CF_VehicleDataRoadmap.md`
5. `Document/ProjectSSOT/Plan/CF_VehicleDataPlan.md`
6. `Document/ProjectSSOT/Plan/CF_VehicleDataInventory.md`
7. `Document/ProjectSSOT/Plan/CF_VehicleMovementInventory.md`
8. `Document/ProjectSSOT/Plan/CF_MinVehicleDataSkeleton.md`
9. `Document/ProjectSSOT/Plan/CF_MinVehicleDataFields.md`
10. `Document/ProjectSSOT/Plan/CF_VehicleDataDraftDesign.md`
11. `Document/ProjectSSOT/Plan/CF_VehicleDataHeader.md`

이 순서의 이유는 아래와 같다.

- 먼저 프로젝트 전체 기준을 이해한다.
- 그 다음 차량 데이터 문서 묶음의 역할을 이해한다.
- 그 다음 전체 방향을 이해한다.
- 그 다음 현재 데이터 분포와 핵심 병목을 본다.
- 마지막에 최소 구현 설계로 들어간다.

### 4.2 현재 작업자가 빠르게 복귀할 때 읽는 순서

이미 이 세션을 이해한 사람이 다시 돌아올 때는 아래 순서가 더 효율적이다.

1. `CF_VehicleDataRoadmap.md`
2. `CF_VehicleDataPlan.md`
3. 현재 작업 성격에 따라 아래 중 선택
   - 전체 분포 확인 필요 → `CF_VehicleDataInventory.md`
   - VehicleMovement 집중 필요 → `CF_VehicleMovementInventory.md`
   - 구현 준비 필요 → `CF_MinVehicleDataSkeleton.md` → `CF_MinVehicleDataFields.md`

즉, 이 로드맵 문서는 **복귀 지점 문서** 역할도 한다.

---

## 5. 문서 작성 순서와 갱신 순서

문서가 많아지면 “어느 문서를 먼저 수정해야 하는가”가 중요해진다.  
이 순서를 안 정하면 아래 문제가 생긴다.

- 구현은 바뀌었는데 계획 문서가 안 바뀐다.
- 인벤토리는 바뀌었는데 최소 설계 문서가 그대로다.
- 보류 조건이 해제됐는데도 문서가 이전 상태를 유지한다.

따라서 문서 수정은 아래 순서를 권장한다.

### 5.1 방향이 바뀌었을 때

먼저 수정할 문서:
1. `CF_VehicleDataPlan.md`
2. `CF_VehicleDataRoadmap.md`
3. 필요한 인벤토리 문서들
4. 최소 골격 / 필드 문서들

이유:
방향은 계획 문서가 먼저 바뀌어야 하고, 로드맵은 그 계획과 하위 문서의 관계를 다시 맞춰야 한다.

### 5.2 현재 상태 인식이 바뀌었을 때

예:
- MCP에서 새 수치를 읽을 수 있게 됨
- 휠 BP에서 실제 값 구조가 확인됨
- 레거시 자산 필드가 더 확인됨

먼저 수정할 문서:
1. `CF_VehicleDataInventory.md`
2. `CF_VehicleMovementInventory.md`
3. `CF_VehicleDataRoadmap.md`
4. `CF_MinVehicleDataSkeleton.md`
5. `CF_MinVehicleDataFields.md`
6. 필요 시 `CF_VehicleDataPlan.md`

이유:
현재 상태가 바뀌면 먼저 인벤토리가 바뀌고, 그 다음 로드맵과 최소 설계가 영향을 받는다.

### 5.3 구현 준비 수준이 바뀌었을 때

예:
- 이제 `UCFVehicleData` 필드 구현을 시작함
- `CFVehiclePawn`에 실제 적용 함수를 넣기 시작함

먼저 수정할 문서:
1. `CF_MinVehicleDataSkeleton.md`
2. `CF_MinVehicleDataFields.md`
3. `CF_VehicleDataRoadmap.md`
4. 필요 시 `CF_VehicleDataPlan.md`

이유:
구현 준비는 최소 설계 문서가 직접 대상이고, 로드맵은 단계 상태를 갱신해야 한다.

---

## 6. 현재 단계 기준 작업 흐름

현재 단계는 명확히 **MCP 수치 가시화 전 단계**다.

이 단계에서의 작업 흐름은 아래처럼 정리하는 것이 가장 안전하다.

### Step 1. 기준과 방향 고정
참조 문서:
- `CF_VehicleDataPlan.md`

목표:
- 현재 기준 구현 고정
- 소유권 원칙 고정
- 장기 목표 고정

### Step 2. 데이터 분포 파악
참조 문서:
- `CF_VehicleDataInventory.md`

목표:
- 차량 관련 데이터 전체 분포 이해
- 레거시 / 신규 / 공통 / Debug 구분

### Step 3. VehicleMovement 핵심 병목 파악
참조 문서:
- `CF_VehicleMovementInventory.md`

목표:
- VehicleMovement와 Wheel BP가 핵심 병목이라는 점 확정
- 실제 수치 확인이 필요한 목록 고정

### Step 4. 지금 가능한 최소 구조만 설계
참조 문서:
- `CF_MinVehicleDataSkeleton.md`
- `CF_MinVehicleDataFields.md`

목표:
- 지금 구현 가능한 최소 골격만 만든다.
- 수치 확정이 필요한 세부 영역은 보류한다.

### Step 5. MCP 수치 가시화 완료를 기다리며 구조만 유지
참조 문서:
- `CF_VehicleDataRoadmap.md`
- `CF_VehicleDataPlan.md`

목표:
- 보류 작업과 재개 조건을 흔들리지 않게 유지한다.

### Step 5-A. 현재 임시중단 상태 기록

현재 구현은 원래의 Step 5 서술보다 앞서 진행되었다.

이미 반영된 범위:
- `UCFVehicleData` 루트 구조 확장
- `CFVehiclePawn`의 `VehicleData` 적용 경로 연결
- `Visual` / `Layout` / `Reference` / `WheelVisual` 실제 적용
- Wheel Class 핵심 수치 다수 이관
- `VehicleMovementComp` 본체 1차 값 이관
- `WheelSetups` 정책(`bLegacyWheelFrictionPosition`, `AdditionalOffset`) 반영

임시중단 이유:
- 현재 시점 이후의 후보 항목은 블루프린트 자산 또는 MCP 수치 확인이 필요하다.
- 마이그레이션 C++ / 현재 소스 / 문서 중 직접 근거가 잡히는 항목이 거의 소진되었다.
- 따라서 추정성 구조 확장을 계속하는 것보다 현재 기준선을 고정하는 편이 더 안전하다.

현재 고정 원칙:
- 직접 근거가 있는 항목만 반영한다.
- 직접 근거가 약한 항목은 보류한다.
- Front / Rear 판정은 가능하면 기존 `FrontWheelCountForSteering` 정책과 일치시킨다.

이 문서 기준 현재 상태는 **"Step 5를 유지하되, 근거가 확인된 범위까지 구현을 선행한 뒤 임시중단한 상태"**로 본다.

---

## 7. MCP 수치 가시화 이후 작업 흐름

이 절은 미래 단계지만, 지금 미리 적어 두는 것이 중요하다.

왜냐하면 지금 만든 문서들의 진짜 가치는 **나중에 어디서 다시 시작할지 아는 것**에 있기 때문이다.

### 재개 조건
다음 조건이 충족되면 본격적인 다음 단계로 넘어간다.

- MCP에서 `VehicleMovementComp` Details 수치를 읽을 수 있다.
- MCP에서 `BP_Wheel_Front`, `BP_Wheel_Rear` 기본 수치를 읽을 수 있다.
- 현재 차량 기준값을 실제 데이터로 캡처할 수 있다.

### 재개 후 권장 순서

1. `CF_VehicleMovementInventory.md` 갱신
2. `CF_VehicleDataInventory.md` 갱신
3. `CF_MinVehicleDataSkeleton.md` 갱신
4. `CF_MinVehicleDataFields.md` 갱신
5. `CF_VehicleDataDraftDesign.md` 갱신
6. `CF_VehicleDataHeader.md` 갱신
7. 필요 시 `CF_VehicleDataPlan.md` 보완
8. 그 다음 실제 헤더 수정안 검토 또는 코드 반영 단계 진행

이 순서의 이유는 아래와 같다.

- 먼저 사실관계를 갱신한다.
- 그 다음 최소 구조와 필드 설계를 현실에 맞게 수정한다.
- 그 다음 적용 경로 초안 설계를 현실에 맞게 수정한다.
- 마지막에 실제 헤더 초안 수준 문서로 내려간다.

즉, `CF_VehicleDataDraftDesign.md`는 현재 단계의 구현 직전 초안 설계 문서로 유지하고, MCP 수치 가시화 이후에는 이 문서를 갱신한 뒤 실제 `UCFVehicleData.h` 초안 설계로 내려가는 것이 맞다.

---

## 8. 문서 간 의존 관계

문서는 아래 의존 관계를 가진다.

```text
ProjectSSOT 상위 문서
    ↓
CF_VehicleDataRoadmap.md
    ↓
CF_VehicleDataPlan.md
    ↓
CF_VehicleDataInventory.md ──→ CF_VehicleMovementInventory.md
    ↓                               ↓
CF_MinVehicleDataSkeleton.md ──→ CF_MinVehicleDataFields.md
                                    ↓
                         CF_VehicleDataDraftDesign.md
```

이 흐름은 아래 뜻이다.

- 로드맵이 전체 문서 관계를 묶는다.
- 계획 문서가 방향을 고정한다.
- 인벤토리 문서가 현실을 보여준다.
- 최소 골격 문서가 현재 단계에서 가능한 상위 구조를 만든다.
- 필드 문서가 구현 직전 수준으로 내린다.
- 초안 설계 문서가 실제 적용 경로와 책임 분해를 정리한다.

이 의존 관계를 안 지키면 아래 문제가 생긴다.

- 인벤토리가 불완전한데 필드 설계를 확정함
- 계획 문서가 보류를 말하는데 필드 문서는 전면 구현을 전제로 씀
- 로드맵이 없는 상태에서 문서별 상태가 제각각으로 움직임

---

## 9. 현재 문서 상태 평가

현재 시점 기준으로 각 문서를 아래처럼 평가할 수 있다.

| 문서 | 현재 역할 | 상태 | 지금 기준 신뢰도 | 비고 |
|---|---|---|---|---|
| `CF_VehicleDataRoadmap.md` | 문서 상위 운영 문서 | 작성 완료 | 높음 | 상위 안내서 |
| `CF_VehicleDataPlan.md` | 전체 방향 계획서 | 작성 완료 | 높음 | 기준 문서 |
| `CF_VehicleDataInventory.md` | 전체 분포 인벤토리 | 작성 완료 | 높음 | 전체 지도 |
| `CF_VehicleMovementInventory.md` | VehicleMovement 핵심 인벤토리 | 작성 완료 | 중간~높음 | 수치 확정은 보류 |
| `CF_MinVehicleDataSkeleton.md` | 최소 골격 문서 | 작성 완료 | 높음 | MCP 전 단계 기준 |
| `CF_MinVehicleDataFields.md` | 최소 필드 설계 문서 | 작성 완료 | 높음 | 구현 직전 참고 가능 |
| `CF_VehicleDataDraftDesign.md` | 구현 직전 적용 구조 초안 설계 문서 | 작성 완료 | 높음 | 적용 경로 / 책임 분해 기준 |

여기서 주의할 점은 하나다.

`CF_VehicleMovementInventory.md`는 방향과 항목 분류의 신뢰도는 높지만, 실제 숫자 확정 문서는 아니다.  
따라서 이 문서를 직접 구현 수치 기준으로 쓰면 안 된다.

이런 해석 기준도 로드맵 문서에 적어두는 것이 중요하다.

---

## 10. 문서 축소 / 통합 원칙

문서가 많아졌다고 해서 바로 합치는 것은 좋은 선택이 아니다.

오히려 아래 경우에만 통합을 고려하는 것이 좋다.

### 통합을 고려해도 되는 경우
- MCP 수치 가시화가 완료되어 보류 문서와 구현 문서의 경계가 줄어들었을 때
- 실제 구현이 어느 정도 끝나서 최소 골격 문서와 필드 문서가 사실상 같은 내용을 가리킬 때
- 인벤토리 문서가 안정되어 더 이상 자주 바뀌지 않을 때

### 아직 통합하면 안 되는 경우
- 지금처럼 MCP 기능이 확장 중일 때
- 수치가 아직 확정되지 않았을 때
- 계획 / 인벤토리 / 최소 구현 준비의 성격이 분명히 다를 때

즉, 현재 단계에서는 **문서를 합치지 않는 것이 맞다.**

대신 아래만 지키면 된다.

- 역할 겹침 금지
- 상위 로드맵 문서 유지
- 문서 간 링크와 순서 명시

---

## 11. 현재 단계 권장 운영 원칙

지금 단계에서는 아래 원칙을 문서 운영 기준으로 고정하는 것이 좋다.

### 원칙 1. 방향은 `CF_VehicleDataPlan.md`에서만 바꾼다
하위 문서에서 방향을 새로 발명하지 않는다.

### 원칙 2. 사실관계 변화는 인벤토리 문서부터 반영한다
MCP 기능 확대, 실제 값 확인, 레거시 필드 발견은 인벤토리 문서에서 먼저 반영한다.

### 원칙 3. 최소 구현 범위는 Skeleton / Fields 문서에서만 다룬다
계획 문서에 구현 세부안을 과도하게 넣지 않는다.

### 원칙 4. MCP 가시화 전에는 수치 확정 문서를 만들지 않는다
지금 단계에서 수치 확정 문서를 만들면, 나중에 문서 정합성이 깨질 가능성이 높다.

### 원칙 5. 이 로드맵 문서를 복귀 출발점으로 사용한다
세션이 길어질수록, 다음 세션 첫 진입 문서는 이 문서가 되어야 한다.

---

## 12. 다음 문서 작업 순서

사용자가 요청한 흐름상, 현재 이 로드맵 문서 다음 단계는 **초안 설계서** 작성이다.

그런데 여기서 중요한 점은, 초안 설계서의 성격을 먼저 고정해야 한다는 것이다.

현재 단계에서 다음 초안 설계서는 아래 조건을 만족해야 한다.

- 구현 문서가 아니라 **구현 직전 설계 문서**여야 한다.
- 현재 MCP 한계를 반영해야 한다.
- `UCFVehicleData` 최소 골격과 최소 필드 설계표를 바탕으로 써야 한다.
- 실제 수치 확정이 필요한 곳은 TODO / Reserved / Pending 으로 남겨야 한다.
- `CFVehiclePawn` / `CFVehicleDriveComp` / `CFWheelSyncComp` / `VehicleMovementComp`와의 연결 책임을 설명해야 한다.

즉, 다음 문서의 역할은 아래 한 문장으로 정리된다.

> **현재 단계에서 구현 가능한 `UCFVehicleData` 기반 차량 데이터 적용 구조를, 코드 작성 직전 수준으로 풀어 쓴 초안 설계서**

이 문서가 작성되면, 현재 문서 흐름은 아래 상태가 된다.

- 상위 방향 있음
- 현재 상태 분석 있음
- 최소 구조 있음
- 최소 필드 있음
- 구현 직전 초안 설계 있음

이 상태면 MCP 수치 가시화 전 단계의 문서 준비는 사실상 충분하다.

---

## 13. 최종 판단

현재 차량 데이터 관련 문서 흐름은 이미 나쁘지 않다.  
문서가 많아 보일 뿐, 실제로는 아래처럼 단계가 분리되어 있다.

- 방향
- 인벤토리
- 핵심 병목 인벤토리
- 최소 골격
- 최소 필드

지금 필요한 것은 이 문서들을 억지로 줄이는 것이 아니라, **문서 간 관계와 순서를 고정하는 것**이다.

따라서 이번 로드맵 문서의 최종 판단은 아래와 같다.

> **현재 차량 데이터 문서 세트는 분산된 것이 아니라 단계적으로 분리된 상태이며, `CF_VehicleDataRoadmap.md`를 상위 안내서로 두면 하나의 작업 흐름으로 충분히 통합된다.**

이 기준이 있으면 다음 세션부터는 아래처럼 단순해진다.

- 먼저 이 로드맵 문서를 본다.
- 지금 단계가 어디인지 확인한다.
- 그 단계에 맞는 하위 문서를 본다.
- MCP 상태에 따라 보류 또는 재개를 판단한다.
- 다음 문서를 이어서 작성한다.

즉, 이 문서 하나가 현재 세션 전체 문서 흐름의 **진입점**, **복귀점**, **정렬 기준**이 된다.

---

## 다음 권장 작업

1. 이 로드맵 문서를 기준으로 `CF_VehicleDataDraftDesign.md`를 구현 직전 적용 구조 기준 문서로 사용한다.
2. 그 다음 실제 `UCFVehicleData.h` 초안 설계 문서를 작성한다.
3. MCP 수치 가시화 전 단계에서는 `CF_VehicleDataDraftDesign.md`와 이후 헤더 초안 문서 모두 Reserved / Pending 구조를 유지한다.
