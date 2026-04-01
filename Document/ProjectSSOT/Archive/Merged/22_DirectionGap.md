# CarFight — 22_DirectionGap v1.0

> 역할: `Document/ProjectSSOT/MyDocument/`의 **상위 방향 문서**와 현재 UE 프로젝트의 **실제 구현 기준선** 사이의 차이를 비교한다.  
> 마지막 정리(Asia/Seoul): 2026-03-27

---

## 이 문서의 목적
- 이 문서는 "현재 구현이 완성도 면에서 부족한가"를 판단하는 문서가 아니다.
- 이 문서는 "현재 구현이 상위 방향을 향해 가고 있는가"를 판단하는 문서다.
- 즉, 완료율이 아니라 **방향 정렬 여부**를 본다.

---

## 비교에 사용한 기준
### 상위 방향 문서
- `Document/ProjectSSOT/MyDocument/CarFight개발기획서.md`
- `Document/ProjectSSOT/MyDocument/22_Project_BigPicture.md`

### 현재 구현 기준선
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`

---

## 한 줄 판정
현재 프로젝트는 **C++ 중심 / 데이터 중심 / Thin BP** 방향은 상위 문서와 맞지만,  
**CMVS 기반 모듈형 차량 / Geometry Collection 기반 파괴 / 조립형 물리 구조** 방향은 아직 현재 구현 구조에 반영되지 않았다.

즉, 지금 상태는 **부분 정렬 / 부분 미도달**이다.

---

## 먼저 고정할 전제
### 1. `BP_ModularVehicle`의 부재는 단독 이슈가 아니다
- `BP_ModularVehicle`는 초기 BP 중심 접근에서 C++ 전환으로 넘어가던 과도기 자산이었다.
- 따라서 현재 프로젝트에 그 자산이 없다는 사실만으로 방향 이탈이라고 보지 않는다.
- 핵심은 그 자리를 무엇이 대체했는지다.

### 2. 게임패드 고정은 임시 우회로로 본다
- 입력 장치 전환 과정의 에러 때문에 현재 테스트가 게임패드 중심으로 고정된 상태다.
- 이것은 현재 검증 운영상의 편차이며, 상위 방향 자체를 바꾼 결정으로 해석하지 않는다.

---

## 비교 결과 요약
### 1. 큰 방향에서 맞는 부분
- 차량 하나만 완성하는 것이 아니라 **여러 차량으로 확장 가능한 공통 코어**를 만들려는 의도는 유지되고 있다.
- 판단 / 상태 / 규칙을 C++로 옮기고 BP를 얇게 유지하려는 방향은 실제 구현에도 반영돼 있다.
- 차량별 차이를 데이터 자산으로 분리하려는 방향도 실제 `UCFVehicleData` 축에서 살아 있다.

### 2. 큰 방향에서 아직 갈라진 부분
- 상위 문서의 차량 구조는 `CMVS + Cluster Union + Geometry Collection` 중심이다.
- 현재 프로젝트의 차량 구조는 `AWheeledVehiclePawn + ChaosWheeledVehicleMovementComponent` 중심이다.
- 즉 "데이터 기반 차량 확장"이라는 상위 목표는 유지되지만, **그 차량을 구성하는 물리/조립 방식**은 아직 상위 방향과 다르다.

---

## 항목별 상세 비교
### A. 차량 구조의 중심축
### 상위 방향 문서의 목표
- `CarFight개발기획서.md`는 `Chaos Modular Vehicle System (CMVS)`를 채택한다고 밝힌다.
- 차량은 `Cluster Union`으로 통합되고, 부품 단위 조립과 물리 반응을 전제로 한다.
- 차체와 부품은 `Geometry Collection` 기반 파괴/분리를 염두에 둔다.

### 현재 프로젝트의 실제 상태
- 현재 기준 Pawn은 `ACFVehiclePawn`이며 `AWheeledVehiclePawn`를 상속한다.
- 현재 주행 이동은 `UChaosWheeledVehicleMovementComponent` 기반이다.
- 현재 코어는 표준 Chaos wheeled vehicle 계열 위에 `DriveState`, `WheelSync`, `VehicleData`를 얹는 구조다.

### 판정
- **부분 불일치**
- 상위 목표가 말하는 "모듈형 물리 조립 기반 차량"이 아니라, 현재는 "기존 차량 이동 컴포넌트 기반 공통 코어"에 가깝다.
- 따라서 구조적 중심축은 아직 상위 방향에 도달하지 못했다.

---

### B. BP 역할과 C++ 코어 분리
### 상위 방향 문서의 목표
- `22_Project_BigPicture.md`는 판단 / 상태 / 규칙을 C++에 두고 BP는 Thin BP로 유지한다고 정리한다.
- 이는 장기적으로 다양한 차량과 전투 규칙을 안정적으로 확장하기 위한 방향이다.

### 현재 프로젝트의 실제 상태
- `BP_CFVehiclePawn`는 현재 기준 플레이 자산으로 존재한다.
- 실제 판단 축은 `ACFVehiclePawn`, `UCFVehicleDriveComp`, `UCFWheelSyncComp`, `UCFVehicleData`에 분산되어 있다.
- 실측 기준 `BP_CFVehiclePawn`의 이벤트 그래프는 매우 얇고, 디버그 위젯 생성 같은 표현 계층 위주다.

### 판정
- **정렬**
- 이 부분은 상위 방향과 현재 구현이 가장 잘 맞는 영역이다.

---

### C. 데이터 기반 차량 확장
### 상위 방향 문서의 목표
- 다양한 비율과 형태의 차량을 같은 절차로 설정하는 자동화 파이프라인을 지향한다.
- 차량별 차이는 코드 복붙이 아니라 데이터 차이로 처리되어야 한다.

### 현재 프로젝트의 실제 상태
- `UCFVehicleData`가 현재 데이터 축으로 존재한다.
- `DA_PoliceCar`에 시각 구성, 휠 시각 오버라이드, DriveState 오버라이드가 실제로 연결되어 있다.
- 현재 기준선에서는 데이터 중심 확장 가능성이 보이지만, 실제 운영 중인 차량 데이터는 아직 `DA_PoliceCar` 중심이다.

### 판정
- **부분 정렬**
- "데이터 축 확보"는 되어 있다.
- 하지만 "여러 차량이 같은 절차로 조립되는 단계"까지는 아직 가지 않았다.

---

### D. 조립형 차량 파이프라인
### 상위 방향 문서의 목표
- `BP_ModularVehicle`의 `BuildVehicleFromData`
- `CMD_RebuildVehicle`의 `Call In Editor`
- 즉, 데이터로부터 차량 부품을 재조립하는 반복 가능한 제작 루프를 상정한다.

### 현재 프로젝트의 실제 상태
- 과거 흔적으로 `CFModVehiclePawn`에 `BP_ModularVehicle -> C++ base class (skeleton)` 주석이 남아 있다.
- 현재 살아 있는 실제 흐름은 `ACFVehiclePawn`의 `ApplyVehicleVisualConfig`, `ApplyVehicleLayoutConfig`다.
- 즉, 현재 구조는 "에디터 재조립형 모듈 차량"보다 "현재 Pawn 위에 시각/배치 설정을 적용하는 구조"에 가깝다.

### 판정
- **부분 불일치**
- 조립 파이프라인이라는 상위 개념은 남아 있지만, 구현 형태는 이미 다른 방향으로 재편되었다.

---

### E. 파괴 / 분리 / 전투 물리 반영
### 상위 방향 문서의 목표
- 피격 시 부위 파괴
- 부품 분리
- 무게 중심 변화
- 차량 전투와 물리 상호작용을 구조적으로 지원하는 차량 본체

### 현재 프로젝트의 실제 상태
- 현재 활성 코어 문서와 실제 코드의 중심은 `DriveState`, `WheelSync`, `VehicleData` 안정화다.
- 현재 프로젝트 소스와 기준 자산 구조에서는 `Geometry Collection` 기반 차량 본체나 부품 분리형 조립 구조가 전면에 드러나지 않는다.
- 현재 테스트 기준도 주행 코어 검증에 맞춰져 있다.

### 판정
- **미도달**
- 이 부분은 "나중에 붙일 예정" 수준이 아니라, 아직 현재 구조의 중심 설계로 들어왔다고 보기 어렵다.

---

### F. 현재 기준선 운영의 현실적 편차
### 입력 장치
- 현재 `BP_CFVehiclePawn`의 `InputDeviceMode`는 `GamepadOnly`다.
- 이는 입력 장치 전환 이슈를 피하기 위한 임시 운용으로 해석한다.
- 상위 방향과의 구조 차이를 만드는 핵심 원인은 아니다.

### 기존 VehicleMesh 의존
- 현재 기준 Pawn에는 `VehicleMesh`와 `ChaosWheeledVehicleMovementComponent`가 실질 축으로 남아 있다.
- 이 점은 상위 문서가 말하는 "리깅 의존 제거" 방향과 직접적으로 충돌하는 현재 흔적이다.

---

## 차이의 성격 분류
### 1. 의도된 과도기 편차
- `BP_ModularVehicle` 제거
- 게임패드 중심 테스트 운용
- 단일 테스트 차량(`DA_PoliceCar`) 중심 검증

### 2. 실제 구조 차이
- `CMVS / Cluster Union / Geometry Collection` 대신 `ChaosWheeledVehicleMovementComponent` 중심 구조
- 조립형 차량 파이프라인 대신 현재 Pawn 설정 적용 구조
- 차량 전투/부품 파괴 구조가 아직 현재 코어 설계의 전면에 오지 않음

---

## 지금 시점의 결론
현재 프로젝트는 **상위 방향을 완전히 잃어버린 상태는 아니다.**

다만 더 정확히 말하면 아래와 같다.
- **맞는 방향**
  - C++ 중심 코어
  - Thin BP
  - 데이터 자산 축
  - 다차종 확장을 의식한 문서 구조
- **아직 반영되지 않은 방향**
  - CMVS 기반 조립
  - Cluster Union 기반 물리 통합
  - Geometry Collection 기반 파괴/분리
  - 차량 전투 물리 구조의 본체 반영

따라서 현재 프로젝트는  
"상위 방향의 운영 철학"에는 맞지만,  
"상위 방향의 차량 구조 방식"에는 아직 도달하지 못한 상태라고 보는 것이 가장 정확하다.

---

## 권장 후속 작업
### 1. 상위 방향 문서의 강제 우선순위 재확정
- `CarFight개발기획서.md`의 `CMVS / Geometry Collection` 축을 앞으로도 강하게 유지할지 먼저 확정해야 한다.
- 이 결정이 확정되어야 현재 구조를 "임시 기반"으로 볼지, "새 기준선"으로 볼지 정할 수 있다.

### 2. 현재 구조를 상위 방향으로 연결하는 중간 설계 문서 작성
- 현재 `ACFVehiclePawn` 기반 구조에서 어떤 부분이 나중에 교체될 예정인지 명확히 적어야 한다.
- 예:
  - 유지할 것: `VehicleData`, DriveState 코어, WheelSync 코어
  - 교체 후보: Vehicle root 구조, 조립 방식, 파괴 표현 구조

### 3. 문서 내 편차를 "의도된 편차"와 "미해결 갭"으로 분리
- 게임패드 고정 같은 운영 편차는 별도 메모로 분리한다.
- 구조적 갭은 로드맵 항목으로 승격한다.

### 4. 다음 비교 문서의 입력값 고정
- 이후 비교에는 아래 항목을 항상 같이 본다.
- `ACFVehiclePawn`
- `UCFVehicleDriveComp`
- `UCFWheelSyncComp`
- `UCFVehicleData`
- `DA_PoliceCar`
- `BP_CFVehiclePawn`

---

## 관련 문서
- `03_VisionAlign.md`
- `00_Handover.md`
- `01_Roadmap.md`
- `16_CPP_DecisionLog.md`
- `Document/ProjectSSOT/MyDocument/CarFight개발기획서.md`
- `Document/ProjectSSOT/MyDocument/22_Project_BigPicture.md`
