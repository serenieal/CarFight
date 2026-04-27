# VehicleDebug

## 주의
이 문서는 `2026-04-22` 기준의 **레거시 `WBP_VehicleDebug` 현재 상태 기록 문서**다.
현재 v2 구현 방향은 `CFVehicleDebugHudWidget + WBP_VehicleDebugHud` 중심 구조이며, `WBP_VehicleDebug`는 제거 대상으로 본다.
즉, 이 문서는 앞으로도 회귀 비교용 기준선으로는 유효하지만, **새 구현 기본안 문서로 사용하면 안 된다.**
또한 현재 최신 구현에서는 HUD/Panel UI 표시 토글과 `AddOnScreenDebugMessage` 기반 개발용 문자열 출력 토글이 분리되기 시작했으므로, 이 문서의 표시 조건 설명은 레거시 기준으로만 읽어야 한다.

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleDebug` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleDebug` 기능은 아래 세 요소를 묶어서 본다.

- 표시 UI 자산: `/Game/CarFight/UI/WBP_VehicleDebug`
- 디버그 문자열 생성 주체: `ACFVehiclePawn`
- 실제 생성/부착 블루프린트: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`

즉, 현재 기준 `VehicleDebug`는 **위젯 하나만의 기능이 아니라, Pawn이 만든 차량 상태 문자열을 위젯이 화면에 표시하는 기능 묶음**이다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleDebug`의 핵심 역할은 **차량 Pawn의 현재 런타임 상태를 사람이 즉시 읽을 수 있는 문자열로 요약해서 화면에 표시하는 것**이다.

이 기능은 단순히 "디버그 텍스트를 띄운다" 수준이 아니다. 현재 `ACFVehiclePawn::BuildVehicleDebugSummary()` 구현을 기준으로 보면, 이 기능은 아래 정보를 하나의 문자열로 조합한다.

### 1. 차량 런타임 준비 상태 표시
가장 먼저 현재 Pawn이 런타임 준비를 마쳤는지 표시한다.

- `Ready=True/False`

이 값은 `bVehicleRuntimeReady`를 사용한다.
즉, 디버그 텍스트는 단순 주행 상태만 보여주는 것이 아니라, **이 차량 Pawn이 런타임 초기화에 성공했는지**부터 보여준다.

### 2. 현재 Drive 상태 표시
`VehicleDriveComp`가 유효하면 현재 Drive 상태 스냅샷을 읽어 아래 값을 문자열에 포함한다.

- `State=` 현재 Drive 상태 enum
- `Speed=` 현재 전체 속도 km/h
- `ForwardSpeed=` 전후 방향성을 가진 속도 km/h
- `Throttle=` 현재 쓰로틀 입력값
- `Brake=` 현재 브레이크 입력값
- `Steering=` 현재 조향 입력값
- `Handbrake=` 핸드브레이크 On/Off

즉 현재 `VehicleDebug`는 **차량이 어느 상태에 있는지**, **얼마나 움직이고 있는지**, **지금 어떤 입력이 들어가고 있는지**를 한 번에 보여준다.

### 3. 입력 해석 상태 표시
현재 구현에서는 입력 상태도 별도로 포함한다.
다음 값들이 출력된다.

- `DeviceMode=` 현재 입력 장치 모드
- `InputOwner=` 현재 입력 주도권 소유 경로
- `MoveZone=` 현재 2D 이동 입력이 해석된 영역
- `MoveIntent=` 마지막 진행 방향 의도
- `MoveRaw=(X,Y)` 원본 2D 이동 입력값
- `MoveMag=` 입력 강도
- `MoveAngle=` 입력 각도
- `BlackHold=` 검은 영역 방향 유지 정책 사용 여부

이 부분이 중요하다.
현재 `VehicleDebug`는 단순 차량 물리 디버그가 아니라, **차량 입력 시스템이 지금 입력을 어떻게 해석하고 있는지까지 함께 보여주는 입력 해석 디버그** 역할도 한다.

### 4. 상태 전이 요약 표시
설정에 따라 마지막 상태 전이 요약 문자열도 함께 표시한다.

- `DriveStateTransition: ...`

이 값은 `VehicleDriveComp->GetLastDriveStateTransitionSummary()`에서 받아온다.
즉 현재 기능은 정적인 현재 상태만 보여주는 것이 아니라, **바로 직전에 어떤 Drive 상태 전이가 있었는지**까지 노출한다.

### 5. 런타임 초기화 결과 요약 표시
설정에 따라 마지막 런타임 요약 문자열도 함께 포함한다.

- `LastVehicleRuntimeSummary`

이 값에는 현재 구조상 WheelSync 관련 런타임 요약이 덧붙을 수 있다.
즉 이 기능은 단순 HUD가 아니라, **차량 런타임 준비 / 적용 / 검증 결과를 함께 읽는 진단 출력창** 역할도 한다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleDebug`는 아래 세 역할을 동시에 가진다.

1. **주행 상태 디버그**
   - 현재 Drive 상태
   - 속도
   - 전/후진 방향 속도

2. **입력 상태 디버그**
   - 쓰로틀 / 브레이크 / 조향 / 핸드브레이크
   - 입력 장치 모드
   - 입력 주도권
   - 2D 이동 입력 해석 결과

3. **런타임 진단 디버그**
   - 런타임 준비 완료 여부
   - 마지막 상태 전이 요약
   - 마지막 런타임 결과 요약

따라서 현재 이 기능은 "화면에 차량 디버그 텍스트를 띄우는 위젯"이라기보다,
**차량 Pawn의 현재 주행/입력/런타임 진단 상태를 한 번에 읽기 위한 텍스트 기반 디버그 패널**이라고 보는 것이 더 정확하다.

## 현재 표시 포맷 방식
현재 `ACFVehiclePawn`에는 디버그 텍스트 포맷 모드 enum이 있다.

- `Off`
- `SingleLine`
- `MultiLine`

각 모드의 현재 동작은 아래와 같다.

- `Off`: 빈 텍스트 반환
- `SingleLine`: ` | ` 구분자로 이어진 한 줄 요약
- `MultiLine`: 줄바꿈(`\n`) 기반의 멀티라인 출력

즉 현재 `VehicleDebug` 기능은 **표시 정보의 내용은 거의 동일하고, 표현 형식만 한 줄/여러 줄로 바꿀 수 있는 구조**다.

## 현재 표시 조건
현재 디버그 위젯이 표시되어야 하는지는 `ACFVehiclePawn::ShouldShowDebugWidget()` 기준으로 판단한다.

표시 조건:
- `bEnableDriveStateOnScreenDebug == true`
- `DriveStateDebugDisplayMode != Off`

그리고 Visibility는 아래처럼 결정된다.

- 표시 가능: `HitTestInvisible`
- 표시 안 함: `Collapsed`

즉 현재 구현에서는 위젯이 생성돼 있더라도,
**디버그 표시 스위치가 꺼져 있거나 표시 모드가 Off이면 실제 화면 기능은 비활성화 상태**가 된다.

## 현재 UI 자산 역할
`WBP_VehicleDebug` 자체는 현재 구조상 복잡한 로직 UI가 아니다.

확인된 내용:
- 자산 클래스: `WidgetBlueprint`
- 루트 위젯: `CanvasPanel`
- 위젯 트리 수: 3
- 변수 수: 1
- 위젯 바인딩 수: 1
- 확인된 바인딩: `TextDebug.Text <- GetDebugTextByDisplayMode`

즉 현재 위젯은 **직접 계산을 많이 하는 UI가 아니라, Pawn이 만든 디버그 문자열을 받아서 화면에 출력하는 얇은 표시 레이어**다.

## 현재 생성 및 연결 구조
현재 `BP_CFVehiclePawn` 이벤트 그래프에서 확인된 생성 흐름은 아래와 같다.

1. `BeginPlay`
2. `Is Locally Controlled`
3. `Get Controller`
4. `Cast To PlayerController`
5. `Create WBP Vehicle Debug Widget`
6. `Set DebugWidgetRef`
7. `Add to Viewport`

현재 구조 해석:

- 위젯은 `BP_CFVehiclePawn`이 직접 생성한다.
- 로컬 제어 차량일 때만 생성 흐름이 진행된다.
- 생성된 위젯 참조는 `DebugWidgetRef`에 저장된다.
- 생성 직후 뷰포트에 추가된다.

즉 현재 `VehicleDebug`는 **전역 디버그 창이 아니라 로컬 플레이어가 제어 중인 차량 Pawn에 종속된 디버그 UI**다.

## 현재 기능 책임
현재 구현 기준에서 이 기능의 책임은 아래와 같다.

- 차량 현재 상태를 읽기 쉬운 문자열로 조합한다.
- 주행 상태, 입력 상태, 런타임 진단 상태를 함께 보여준다.
- 표시 형식을 SingleLine / MultiLine으로 바꿀 수 있다.
- 로컬 제어 차량용 디버그 위젯을 생성하고 화면에 부착한다.
- 설정값에 따라 실제 화면 표시를 켜거나 끈다.

## 현재 기준 비책임 항목
아래 항목은 현재 구현상 이 기능의 직접 책임으로 보지 않는다.

- 디버그 정보 계산의 근본 로직 자체
  - 예: Drive 상태 계산 자체는 `VehicleDriveComp` 책임
- 입력 시스템의 실제 처리 로직 자체
  - 예: 2D 입력 해석 로직은 Pawn 입력 처리 계층 책임
- 복수 패널/탭 UI 관리
- 그래프/게이지 기반 시각화
- 상호작용형 디버그 툴 윈도우 기능

즉 현재 `VehicleDebug`는 **계산기**가 아니라 **표시기/관찰기**에 가깝다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleDebug` 기능은,

**차량 Pawn이 자신의 런타임 준비 상태, Drive 상태, 입력 상태, 마지막 상태 전이 및 런타임 요약을 문자열로 조합하고, 이를 로컬 플레이어용 위젯으로 화면에 표시하는 텍스트 기반 차량 진단 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleDebug`는 현재 차량이 "준비됐는지, 어떤 상태인지, 어떤 입력을 받고 있는지, 방금 어떤 상태 변화가 있었는지"를 한 번에 읽게 해주는 현재 상태 진단 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `WBP_VehicleDebug`가 Pawn 참조를 어떤 방식으로 보유하는지의 세부 연결 방식
- 위젯 내 Visibility 바인딩이 추가로 존재하는지 여부
- 블루프린트 레벨에서 별도 토글 입력이 있는지 여부
- PIE 외 환경에서의 운영 기준

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `BuildVehicleDebugSummary()`의 출력 항목 변경
- `GetDebugTextSingleLine / MultiLine / ByDisplayMode()` 동작 변경
- `ShouldShowDebugWidget()` 표시 조건 변경
- `BP_CFVehiclePawn`의 위젯 생성 흐름 변경
- `WBP_VehicleDebug`의 텍스트 표시 방식 변경
- 디버그 패널이 텍스트 기반에서 다른 UI 구조로 확장될 때

## 문서 버전 관리
- 현재 문서 버전: `1.0.2`
- 문서 상태: `Initial`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준
- `Major`
  - 기능 해석 자체가 바뀌는 수준의 대규모 재작성
  - 문서 범위가 다른 기능 묶음까지 확장되거나 재정의될 때
- `Minor`
  - 현재 기능 설명에 중요한 항목이 추가될 때
  - 새로운 표시 항목, 동작 조건, 연결 구조가 확인되어 본문 의미가 확장될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.2 - 2026-04-23
- 레거시 `WBP_VehicleDebug` 제거 이후에도 C++ 내부 `AddOnScreenDebugMessage` 경로가 별도로 남아 있음을 문서에 반영했다.
- `bEnableDriveStateOnScreenDebug`와 `bEnableVehicleDebugOnScreenMessage`가 서로 다른 역할을 가진다는 현재 토글 구조를 문서에 반영했다.
- 이 문서의 버전 관리 원칙에 따라 후속 구현 변경 시 체인지로그를 함께 갱신해야 한다는 기준을 다시 명시했다.

### v1.0.1 - 2026-04-23
- 이 문서를 레거시 `WBP_VehicleDebug` 현재 상태 기록 문서로 명확히 재정의
- v2 구현 기본안은 HUD/Panel 중심이며 `WBP_VehicleDebug` 제거 방향임을 주의 절에 추가

### v1.0.0 - 2026-04-22
- `VehicleDebug` 문서 최초 작성
- `WBP_VehicleDebug`, `BP_CFVehiclePawn`, `CFVehiclePawn` 기준으로 현재 기능 정리
- 단순 위젯 설명이 아니라, 현재 기능이 실제로 수행하는 상태 진단 역할 중심으로 본문 재작성
- 현재 표시 항목, 표시 조건, 생성 구조, 책임/비책임 범위를 문서화

## 마지막 확인 기준
- 확인 일시: 2026-04-22
- 확인 근거:
  - `/Game/CarFight/UI/WBP_VehicleDebug` 요약/상세
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn` 이벤트 그래프 덤프
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
