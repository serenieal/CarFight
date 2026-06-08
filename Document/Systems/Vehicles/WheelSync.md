# WheelSync

## 문서 목적
이 문서는 현재 프로젝트에서 `WheelSync` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `WheelSync` 기능은 아래 요소를 묶어서 본다.

- 핵심 클래스: `UCFWheelSyncComp`
- 관련 상태 구조:
  - `FCFWheelVisualInput`
  - `FCFWheelVisualState`
  - `FCFWheelHelperCompareState`
- 현재 대표 소비 주체: `ACFVehiclePawn`
- 현재 대표 설정 공급 주체:
  - `UCFVehicleData::WheelVisualConfig`
  - `BP_CFVehiclePawn.WheelSyncComp` 기본 설정
- 핵심 함수:
  - `TryPrepareWheelSync()`
  - `BuildWheelVisualInputsFromDebugPipe()`
  - `BuildWheelVisualInputsPhase2()`
  - `ApplyWheelVisualInputsPhase2()`
  - `UpdateWheelVisualsPhase2()`
  - `ResolveDesiredWheelSpinDirectionSign()`
  - `UpdateStableWheelSpinDirectionSign()`

즉, 현재 기준 `WheelSync`는 **차량의 휠 Anchor/Mesh를 실제 런타임 Movement 상태에 맞는 시각 휠 동작으로 갱신하는 휠 시각 동기화 기능**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `WheelSync`의 핵심 역할은 **차량의 각 휠에 대해 조향 Yaw, 서스펜션 Z, 휠 스핀 Pitch를 계산하고, 이를 Anchor/Mesh 컴포넌트에 적용해서 실제 차량 휠이 주행 상태에 맞게 보이도록 유지하는 것**이다.

이 기능은 단순히 휠 메쉬를 돌리는 수준이 아니다.
현재 구조상 `WheelSync`는 아래 네 가지 일을 동시에 한다.

### 1. 휠 시각 동기화가 가능한 상태인지 준비하고 검증한다
현재 `WheelSync`는 바로 업데이트를 시작하지 않는다.
먼저 `TryPrepareWheelSync()`를 통해 준비 절차를 수행한다.

현재 준비 순서:
1. `ResetWheelSyncState()`
2. `CacheVehicleMovementComponent()`
3. `BuildWheelComponentCache()`
4. `CaptureBaseWheelVisualState()`
5. `ValidateWheelSyncSetup()`

각 단계의 현재 의미:

#### A. `ResetWheelSyncState()`
- Movement 캐시 초기화
- Anchor/Mesh 캐시 초기화
- Base visual cache 초기화
- 마지막 입력/상태/런타임 스핀 캐시 초기화
- Helper compare 상태 초기화
- `bWheelSyncReady = false`
- `LastValidationSummary = "Reset"`

즉 현재 `WheelSync`는 준비를 다시 시도할 때 **이전 상태를 남기지 않고 전부 리셋한 뒤 다시 시작하는 구조**다.

#### B. `CacheVehicleMovementComponent()`
- OwnerActor에서 `UChaosWheeledVehicleMovementComponent`를 찾는다.
- 못 찾으면 실패로 처리한다.

즉 현재 `WheelSync`는 **실제 Chaos Vehicle Movement가 없으면 동작할 수 없는 기능**이다.

#### C. `BuildWheelComponentCache()`
- `bAutoFindComponentsByName == true`여야 함
- Anchor 이름 배열과 Mesh 이름 배열이 비어 있지 않아야 함
- 두 배열 길이가 같아야 함
- 이름 기준으로 실제 컴포넌트를 찾아 캐시함

즉 현재 `WheelSync`는 **이름 기반으로 휠 Anchor와 휠 Mesh를 자동 연결하는 구조**다.

#### D. `CaptureBaseWheelVisualState()`
- 준비 완료 시점의 Anchor 위치/회전, Mesh 회전을 기준값으로 저장한다.

즉 현재 `WheelSync`는 매 프레임 절대 원점에서 계산하지 않고,
**초기 기준 시각 상태를 베이스로 삼아 그 위에 조향/서스펜션/스핀 변화를 얹는 구조**다.

#### E. `ValidateWheelSyncSetup()`
- `ExpectedWheelCount > 0`
- VehicleMovement cache 존재
- Anchor cache 개수 일치
- Mesh cache 개수 일치
- 각 휠 인덱스별 Anchor/Mesh 유효성 검사

즉 현재 `WheelSync`는 단순히 준비만 해보고 넘어가지 않고,
**휠 개수와 실제 연결 상태가 기대치와 맞는지 마지막 검증 단계까지 수행**한다.

현재 이 준비 단계가 성공해야 `bWheelSyncReady`가 살아 있는 상태로 해석된다.

### 2. 현재 프레임에 적용할 휠 시각 입력을 만든다
현재 `WheelSync`는 실제 적용 전에 먼저 휠별 입력 구조 `FCFWheelVisualInput`를 만든다.

이 입력 구조에는 현재 아래 항목이 들어간다.
- `WheelIndex`
- `bIsValidInput`
- `SteeringYawDeg`
- `SpinPitchDeg`
- `SpinPitchDeltaDeg`
- `bApplySpinPitchAsDelta`
- `SuspensionOffsetZ`

즉 현재 `WheelSync`는 바로 컴포넌트를 만지는 게 아니라,
**휠별 시각 입력 데이터를 먼저 조합하는 2단계 파이프라인**을 가진다.

현재 입력 생성은 두 단계로 나뉜다.

#### A. `BuildWheelVisualInputsFromDebugPipe()`
현재 기본 입력 파이프는 이름 그대로 `DebugPipe` 기준이다.
이 단계는 현재 각 휠에 대해 조향/서스펜션/스핀 입력의 기본 형태를 만든다.

현재 이 단계는 아래 조건을 요구한다.
- `bWheelSyncReady == true`
- `CachedVehicleMovementComponent` 존재
- `ExpectedWheelCount > 0`

즉 현재 `WheelSync`의 기본 입력 생성은 **준비가 끝난 WheelSync 상태에서만 동작**한다.

#### B. `BuildWheelVisualInputsPhase2()`
현재 실제 프레임 입력 생성의 중심은 `Phase2`다.
이 함수는 아래 흐름으로 동작한다.

1. 먼저 `BuildWheelVisualInputsFromDebugPipe()`로 기본 입력 생성
2. 디버그 모드에서 helper 사용 시 helper 결과를 입력에 덮거나 비교
3. 런타임 휠 회전값을 읽어 실제 스핀 Pitch를 적분
4. 최종 `OutWheelInputs`를 `LastWheelVisualInputs`에 저장
5. `LastInputBuildSummary`와 `LastValidationSummary` 갱신

즉 현재 `WheelSync`는 단순 DebugPipe 결과만 쓰는 게 아니라,
**필요 시 helper 입력과 실제 런타임 휠 회전값까지 섞어 최종 시각 입력을 만든다.**

### 3. 실제 런타임 휠 스핀을 안정적으로 시각 회전으로 적분한다
현재 `WheelSync`에서 가장 중요한 부분 중 하나는 휠 스핀 처리다.
현재 구조는 디버그용 고정 각도 대신, 가능하면 실제 런타임 휠 회전 상태를 이용해 휠 스핀을 만든다.

현재 주요 흐름:
- 현재 차량 Forward 속도(km/h)를 읽음
- 각 휠의 실제 회전각(deg)을 읽음
- 각 휠의 실제 각속도(deg/s)를 읽음
- dead zone 적용
- 저속 접지 상태에서 visual overspeed cap 적용
- 원하는 회전 방향 후보 부호 계산
- hold 규칙을 반영한 안정화 방향 부호 계산
- `DeltaSeconds` 기준으로 스핀 스텝 적분
- 누적 시각 스핀 각도 `AccumulatedRuntimeWheelSpinDeg` 갱신
- 최종 `SpinPitchDeg`, `SpinPitchDeltaDeg`에 반영

즉 현재 `WheelSync`는 단순한 애니메이션 회전이 아니라,
**실제 런타임 휠 각속도와 차량 진행 방향을 해석해서 휠 시각 스핀을 누적 적분하는 기능**이다.

#### 현재 방향 부호 결정 규칙
현재 `ResolveDesiredWheelSpinDirectionSign()`는 아래 순서로 방향을 정한다.

1. Forward 속도가 dead zone 이상이면 Forward 방향 부호를 신뢰
2. 그렇지 않으면 휠 자체 각속도가 충분히 크면 휠 각속도 부호를 신뢰
3. 둘 다 아니면 `0`

즉 현재는 **차량 진행 방향을 우선 신뢰하되, 정지 근처나 슬립 상황에서는 휠 자체 회전 부호도 사용할 수 있는 구조**다.

#### 현재 방향 안정화 규칙
현재 `UpdateStableWheelSpinDirectionSign()`는 방향 반전이 바로 일어나지 않게 hold 규칙을 적용한다.

현재 동작:
- Desired sign이 0이면 pending 초기화, stable 유지
- stable sign이 0이면 즉시 desired 채택
- desired와 stable이 같으면 pending 초기화, stable 유지
- desired와 stable이 다르면 pending으로 보관
- pending이 같은 방향으로 충분히 오래 유지되면 stable sign을 변경

즉 현재 `WheelSync`는 **휠 회전 방향이 프레임마다 튀지 않게 시간을 두고 방향 반전을 승인하는 안정화 기능**을 가진다.

### 4. 계산된 휠 입력을 실제 Anchor/Mesh에 적용한다
현재 `ApplyWheelVisualInputsPhase2()`는 최종 입력 배열을 받아 각 휠에 적용한다.

현재 적용 흐름:
1. `bWheelSyncReady` 확인
2. 입력 개수가 `ExpectedWheelCount`와 맞는지 확인
3. 각 휠에 대해 `ApplySingleWheelInputPhase2()` 호출
4. 마지막 상태 배열 `LastWheelVisualStates` 갱신
5. `LastValidationSummary`에 적용 요약 문자열 기록

현재 `ApplySingleWheelInputPhase2()`에서 실제로 하는 일:
- Base Anchor 위치에 `SuspensionOffsetZ` 반영
- Base Anchor 회전에 `SteeringYawDeg` 반영
- Base Mesh 회전에 `SpinPitchDeg` 반영

그리고 `bEnableApplyTransformsInCpp`가 켜져 있을 때,
소유권 스위치에 따라 실제 컴포넌트 변환을 직접 적용한다.

현재 소유권 스위치:
- `bApplySteeringYawInCpp`
- `bApplySuspensionZInCpp`
- `bApplySpinPitchInCpp`

즉 현재 `WheelSync`는 **입력 생성 기능**과 **실제 변환 적용 기능**을 분리하고,
필요한 축만 C++이 직접 소유하는 선택형 구조다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `WheelSync`는 아래 역할을 가진다.

1. **휠 시각 준비/검증 기능**
   - VehicleMovement, Anchor, Mesh, 기준 상태가 준비됐는지 확인함

2. **휠 시각 입력 생성 기능**
   - 각 휠에 필요한 조향/서스펜션/스핀 입력을 구조체 배열로 생성함

3. **실제 런타임 스핀 적분 기능**
   - 휠 각속도와 차량 진행 방향을 이용해 시각 스핀을 누적 계산함

4. **휠 방향 안정화 기능**
   - 방향 반전 hold 규칙으로 휠 회전 방향 튐을 줄임

5. **실제 Transform 적용 기능**
   - Anchor/Mesh 컴포넌트에 최종 조향/서스펜션/스핀 변환을 반영함

6. **디버그/헬퍼 비교 기능**
   - helper 입력과 디버그 파이프 입력을 비교하고 요약 문자열을 유지함

따라서 현재 이 기능은 단순한 `휠 회전 컴포넌트`라기보다,
**차량 휠의 시각 상태를 준비·검증하고, 매 프레임 입력을 생성하고, 런타임 회전을 안정화해서 실제 휠 시각 변환으로 적용하는 휠 시각 동기화 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `WheelSync`는 아래 방식으로 동작한다.

### 1. 준비 방식
현재 `BeginPlay()`에서 `bAutoPrepareOnBeginPlay == true`면 자동 준비를 시도한다.

하지만 현재 `BP_CFVehiclePawn.WheelSyncComp` 기본값 기준:
- `bAutoPrepareOnBeginPlay = false`

즉 현재 기본 프로젝트 구성에서는 `WheelSyncComp`가 BeginPlay에서 혼자 자동 준비를 끝내는 구조가 아니라,
**상위 런타임 파이프라인(`VehicleRuntime`)에서 `PrepareWheelSync()`를 통해 준비되는 구조**에 가깝다.

### 2. 업데이트 방식
현재 실제 프레임 갱신은 `UpdateWheelVisualsPhase2(DeltaSeconds)`를 통해 수행된다.

흐름:
- `BuildWheelVisualInputsPhase2()`
- `ApplyWheelVisualInputsPhase2()`

즉 현재는 **입력 생성과 적용이 분리된 Phase2 갱신 루프**가 기본 운영 방식이다.

### 3. 적용 소유권 방식
현재 `BP_CFVehiclePawn.WheelSyncComp` 기본값 기준:
- `bEnableApplyTransformsInCpp = true`
- `bApplySteeringYawInCpp = true`
- `bApplySuspensionZInCpp = false`
- `bApplySpinPitchInCpp = true`

즉 현재 기본 프로젝트에서는
- 조향 Yaw는 C++이 직접 적용
- 스핀 Pitch도 C++이 직접 적용
- 서스펜션 Z는 C++이 직접 소유하지 않음

구조로 운영된다.

## 현재 표시 조건 / 실행 조건
현재 `WheelSync`가 제대로 동작하려면 아래 조건이 중요하다.

- `ExpectedWheelCount > 0`
- `CachedVehicleMovementComponent`가 존재해야 함
- Anchor/Mesh 캐시 개수가 `ExpectedWheelCount`와 일치해야 함
- 각 인덱스의 Anchor/Mesh 컴포넌트가 유효해야 함
- `bWheelSyncReady == true` 상태여야 입력 생성/적용이 가능함

즉 현재 `WheelSync`는 준비 검증이 통과되지 않으면,
**업데이트를 아예 막고 `Invalid: ...` 형태의 실패 요약으로 남기는 엄격한 게이트 구조**를 가진다.

## 현재 자산 / 클래스 역할
### `UCFWheelSyncComp`
- 종류: `C++ Component`
- 현재 역할: 휠 시각 준비, 입력 생성, 실제 적용의 중심 컴포넌트

### `FCFWheelVisualInput`
- 종류: `USTRUCT`
- 현재 역할: 각 휠에 적용할 최종 시각 입력 구조

### `FCFWheelVisualState`
- 종류: `USTRUCT`
- 현재 역할: 마지막으로 실제 적용된 Anchor/Mesh 상태 기록

### `FCFWheelHelperCompareState`
- 종류: `USTRUCT`
- 현재 역할: helper 입력과 debug pipe 입력의 차이 비교 기록

### `UCFVehicleData::WheelVisualConfig`
- 종류: `DataAsset 하위 구조`
- 현재 역할: `ExpectedWheelCount`, `FrontWheelCountForSteering` 같은 기본 휠 시각 구성값 공급

### `BP_CFVehiclePawn.WheelSyncComp`
- 종류: `Blueprint 컴포넌트 인스턴스`
- 현재 역할: 실제 프로젝트 기본 WheelSync 설정의 소유자

## 현재 생성 및 연결 구조
현재 `WheelSync` 연결 구조는 아래와 같다.

1. Pawn 생성 시 `WheelSyncComp` 생성
2. `VehicleRuntime`에서 `PrepareWheelSync()` 호출
3. `TryPrepareWheelSync()`가 내부 캐시/기준 상태/검증 수행
4. 준비 성공 시 `bWheelSyncReady = true`
5. 이후 Runtime Tick에서 `UpdateVehicleWheelVisuals()`가 `UpdateWheelVisualsPhase2()` 호출
6. Phase2가 각 휠 입력 생성 후 실제 Anchor/Mesh에 적용

현재 구조 해석:
- WheelSync는 독립 시스템이 아니라 VehicleRuntime의 하위 시각 기능이다.
- 데이터 입력은 `VehicleData`, 실행 타이밍은 `VehicleRuntime`, 실제 적용은 `WheelSyncComp`가 맡는다.

## 현재 기본 프로젝트 설정 (`BP_CFVehiclePawn.WheelSyncComp`)
현재 기본값 기준으로 확인된 주요 설정은 아래와 같다.

- `ExpectedWheelCount = 4`
- `FrontWheelCountForSteering = 2`
- `bAutoPrepareOnBeginPlay = false`
- `bAutoFindComponentsByName = true`
- `bDebugMode = false`
- `bEnableApplyTransformsInCpp = true`
- `bUseRealWheelTransformHelper = false`
- `bEnableHelperCompareMode = false`
- `bUseSteeringYawDebugPipe = true`
- `bUseSuspensionZDebugPipe = false`
- `bUseWheelSpinPitchDebugPipe = false`
- `WheelSpinVisualSign = 1`
- `WheelSpinMeshAxisSign = -1`
- `bApplySteeringYawInCpp = true`
- `bApplySuspensionZInCpp = false`
- `bApplySpinPitchInCpp = true`

현재 Anchor 이름 배열:
- `Wheel_Anchor_FL`
- `Wheel_Anchor_FR`
- `Wheel_Anchor_RL`
- `Wheel_Anchor_RR`

현재 Mesh 이름 배열:
- `Wheel_Mesh_FL`
- `Wheel_Mesh_FR`
- `Wheel_Mesh_RL`
- `Wheel_Mesh_RR`

즉 현재 기본 차량은 **4륜 / 전륜 2조향 / 이름 기반 Anchor·Mesh 탐색 / 조향과 스핀은 C++ 직접 적용** 구조로 WheelSync를 운영한다.

## 현재 기능 책임
현재 구현 기준에서 `WheelSync`의 책임은 아래와 같다.

- VehicleMovement와 휠 시각 컴포넌트 연결을 준비한다.
- 휠 Anchor/Mesh 캐시를 만든다.
- 기준 휠 시각 상태를 캡처한다.
- 준비 상태를 검증하고 `bWheelSyncReady`를 관리한다.
- 현재 프레임의 휠 시각 입력을 생성한다.
- 실제 런타임 휠 회전을 누적 적분해 시각 스핀으로 반영한다.
- 회전 방향 튐을 줄이기 위해 방향 안정화 규칙을 적용한다.
- 최종 Anchor/Mesh 변환을 C++에서 직접 적용한다.
- helper 비교 및 검증 요약 문자열을 유지한다.

## 현재 기준 비책임 항목
현재 구현상 `WheelSync`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- 차량 주행 상태 계산 자체
  - `VehicleDrive` 책임
- 차량 입력 해석 자체
  - `Input` / Pawn 계층 책임
- 차량 런타임 전체 준비 판정 자체
  - 최종 Ready 조합은 `VehicleRuntime` 책임
- 차체/휠 자산 선정 자체
  - 어떤 메시/휠 수를 쓸지는 `VehicleData` 책임
- 디버그 UI 표시 자체
  - `VehicleDebug` 기능 책임

즉 현재 `WheelSync`는 입력기나 상태 계산기라기보다,
**휠 시각 상태를 계산·안정화·적용하는 시각 동기화 기능**에 가깝다.

## 현재 문서 기준의 핵심 결론
현재 `WheelSync` 기능은,

**차량의 실제 런타임 Movement 상태와 휠 회전 상태를 읽어 각 휠의 조향/서스펜션/스핀 시각 입력을 만들고, 이를 Anchor/Mesh 컴포넌트에 적용해 휠이 실제 주행 상태에 맞게 보이도록 유지하는 현재 휠 시각 동기화 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `WheelSync`는 현재 차량 휠이 어떻게 보여야 하는지를 매 프레임 계산해서, 실제 휠 Anchor와 Mesh에 시각 변환으로 반영하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `BuildWheelVisualInputsFromDebugPipe()` 내부에서 조향/서스펜션 기본 입력을 만드는 상세 수식 전체
- helper 경로가 실제 프로젝트 운영에서 사용되는 시나리오 상세
- `bApplySuspensionZInCpp = false`일 때 서스펜션 Z가 어디에서 최종 적용되는지
- `LastHelperCompareSummary`를 실제로 소비하는 UI/운영 경로 존재 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `TryPrepareWheelSync()` 준비 단계 변경
- `BuildWheelVisualInputsPhase2()` 입력 생성 방식 변경
- `ResolveDesiredWheelSpinDirectionSign()` 방향 결정 규칙 변경
- `UpdateStableWheelSpinDirectionSign()` hold 규칙 변경
- `BP_CFVehiclePawn.WheelSyncComp` 기본 설정 변경
- Anchor/Mesh 이름 배열 변경

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
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
### v1.0.0 - 2026-04-23
- `WheelSync` 문서 최초 작성
- `UCFWheelSyncComp` 기준으로 현재 준비/검증, 입력 생성, 실제 적용, 방향 안정화 기능 정리
- `BP_CFVehiclePawn.WheelSyncComp` 기본 설정과 Anchor/Mesh 이름 배열 반영
- 단순 휠 회전 기능이 아니라 휠 시각 동기화 파이프라인 중심으로 본문 작성

## 마지막 확인 기준
- 확인 일시: 2026-04-23
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFWheelSyncComp.h`
  - `UE/Source/CarFight_Re/Private/CFWheelSyncComp.cpp`
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar`
