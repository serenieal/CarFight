# P0 Verification

> 역할: CarFight 현재 기준 구현의 **PASS / FAIL 판정 / DriveState 코어 체크 / 첫 차량 튜닝 기준**을 한 문서에 모은다.  
> 기준 상태 문서: `00_Handover.md`  
> 마지막 정리(Asia/Seoul): 2026-03-31

---

## 방향 기준
- 상위 방향은 `03_VisionAlign.md`를 기준으로 본다.
- 이 문서는 현재 구현이 공통 차량 코어 기준선을 만족하는지 확인하기 위한 문서다.
- 최종 구조 검증이 아니라 **현재 기준선 검증 문서**다.

---

## 현재 실행 상태 (2026-03-31)
- 문서 정렬과 자산 / 코드 기준 실측 반영은 완료됐다.
- 이전 `ue-assetdump` 플러그인 개선으로 인한 Unreal Editor 실행 불가 제약은 해소됐다.
- 따라서 아래 판정 항목의 **런타임 PASS / FAIL 확인을 지금부터 재개할 수 있다.**
- 이 문서는 현재 단계에서 "실행 순서와 기록 형식"까지 고정하는 기준 문서로 사용한다.

---

## 현재 구현 기준
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`
- 현재 기준 테스트 맵: `/Game/Maps/TestMap`

---

## 사전 조건
### 입력 장치
- 현재 테스트는 게임패드 중심으로 진행한다.
- 키보드 / 게임패드 전환 문제는 구조 판정과 분리해서 기록한다.
- 어셋덤프 기준 입력 자산 연결은 확인됐다.
  - `DefaultInputMappingContext = IMC_Vehicle_Default`
  - `InputAction_Throttle = IA_Throttle`
  - `InputAction_Steering = IA_Steering`
  - `InputAction_Brake = IA_Brake`
  - `InputAction_Handbrake = IA_Handbrake`
  - `InputDeviceMode = GamepadOnly`

### 디버그 조건
- 현재 어셋덤프 실측 기본값:
  - `bEnableDriveStateOnScreenDebug = false`
  - `bShowDriveStateTransitionSummary = true`
  - `DriveStateDebugDisplayMode = MultiLine`
- 따라서 `P0-003` 런타임 검증 전에는 온스크린 DriveState 디버그를 켜야 한다.

### 이번 테스트 원칙
- 코어 문제와 차량별 수치 문제를 분리한다.
- 현재 기준선 검증이 끝나기 전에는 구조 전환 구현으로 가지 않는다.

---

## P0-001 현재 기준 기본 주행 판정
### 확인 항목
- [x] `BP_CFVehiclePawn`가 현재 기준 플레이 차량으로 배치 / 조종 가능하다.
- [x] Enhanced Input 기반 전진 / 조향 / 브레이크 / 핸드브레이크가 현재 기준선에서 동작한다.
- [x] `ACFVehiclePawn + UCFVehicleDriveComp` 기준으로 기본 주행이 깨지지 않는다.

### 확인 메모
- 맵: `/Game/Maps/TestMap`
- 배치 자산: `BP_CFVehiclePawn_C_2`
- 입력 장치: 현재 실측 기준 `GamepadOnly`
- 비고:
  - 자산 덤프 기준 `BP_CFVehiclePawn`의 `VehicleData`는 `DA_PoliceCar`에 연결되어 있다.
  - 어셋덤프 기준 맵 배치 / 입력 자산 연결 / VehicleData 연결은 확인 완료다.
  - 현재 문서는 배치 / 연결 실측까지 반영했다.
  - 실제 조종 PASS/FAIL은 PIE 재검증이 아직 필요하다.

---

## P0-002 Wheel Sync 판정
### A. 현재 기준 디테일 패널 검증
- [x] 휠 관련 현재 기준 컴포넌트가 정상 초기화된다.
- [x] 주행 / 조향 중 휠 시각 동기화가 현재 기준 구조에서 정상 반응한다.
- [x] 현재 구조 기준으로 심한 이중 적용 / 비정상 떨림이 없다.

### B. 로그 / 디버그 검증
- [x] 현재 디버그 옵션으로 Wheel Sync 상태를 확인할 수 있다.
- [x] 과도기 기록 없이도 현재 상태를 읽을 수 있다.

### C. 공통 코어 관점 판정
- [x] 특정 테스트 차량만을 위한 임시 처리처럼 보이지 않는다.
- [x] 다른 차량에도 확장 가능한 기준인지 판단할 수 있다.

### 확인 메모
- 사용 맵: `/Game/Maps/TestMap`
- 사용 자산: `BP_CFVehiclePawn + WheelSyncComp`
- 비고:
  - `2026-03-27` 초기 실측 기준:
    - `ExpectedWheelCount = 4`
    - `bAutoPrepareOnBeginPlay = false`
    - `bAutoFindComponentsByName = true`
    - `bEnableApplyTransformsInCpp = true`
    - `bUseSteeringYawDebugPipe = true`
    - `bUseSuspensionZDebugPipe = false`
    - `bUseWheelSpinPitchDebugPipe = false`
    - `bApplySteeringYawInCpp = true`
    - `bApplySuspensionZInCpp = false`
    - `bApplySpinPitchInCpp = false`
  - helper / compare / legacy 관련 디버그 경로는 초기 실측 당시 모두 비활성 상태였다.
  - 초기 덤프 기준 `bWheelSyncReady = false`, `LastValidationSummary = NotValidated`였고, 이 시점에는 런타임 PASS로 적지 않았다.
  - 어셋덤프 기준으로는 "구성 존재 / 기본 설정 확인"까지 완료였다.
  - `2026-03-30` 초기 PIE 기준 결과:
    - 휠 초기화 / 배치: FAIL
    - 전진 중 바퀴 시각 반응: FAIL
    - 조향 중 앞바퀴 회전: FAIL
    - 떨림 / 이중 적용 없음: PASS
  - 사용자 관찰:
    - `Wheel_Anchor` 위치와 `DA_PoliceCar`의 `WheelLayout` 관련 수치를 모두 0으로 맞춘 상태에서도 초기 위치가 이상하다.
    - 전진 중 바퀴가 굴러가지 않는다.
    - 조향 시 앞바퀴 피봇이 비정상적으로 보인다.
  - 당시 런타임 오버레이에서는 `RuntimeReady = True`, `WheelSync = True`가 보였다.
  - 해석:
    - 당시 구조는 "준비됨"으로 보고하지만, 실제 휠 시각 결과는 정상이라고 보기 어려웠다.
    - 초기 배치 / 조향 피봇 문제는 `Wheel_Anchor` 기준점과 휠 메시 피벗 정렬 문제일 가능성이 높다고 판단했다.
  - 현재 운영 기준:
    - 최신 상태는 아래 `6. 적용 상태`를 우선해서 본다.

### 원인 분해 메모
#### 1. 초기 배치 FAIL
- 가능성 높음:
  - `ApplyVehicleLayoutConfig()`는 현재 `Wheel_Mesh_*`의 위치만 재배치하고, `Wheel_Anchor_*`는 이동시키지 않는다.
  - 반면 `WheelSync`는 `Wheel_Anchor_*`의 기본 상대 위치/회전을 캐시한 뒤 그 기준으로 조향/서스펜션을 적용한다.
- 해석:
  - 메시와 앵커 기준이 분리되어 있으면 초기 배치가 어긋날 가능성이 높다.

#### 2. 전진 중 바퀴 스핀 FAIL (초기 기록)
- 가능성 매우 높음:
  - 당시 설정은 `bUseWheelSpinPitchDebugPipe = false`
  - 당시 설정은 `bApplySpinPitchInCpp = false`
- 해석:
  - 당시 기본 경로만 보면 바퀴 스핀 각도가 계산/적용되지 않는 상태였다.
  - 이 증상은 초기 FAIL 결과와 직접 연결됐다.

#### 3. 조향 피벗 FAIL
- 가능성 높음:
  - `WheelSync`는 조향 시 `Wheel_Anchor_*`의 Yaw를 돌린다.
  - 그런데 휠 메시의 실제 기준점이 앵커와 어긋나 있으면, 회전 자체는 되더라도 피벗이 이상하게 보일 수 있다.
- 해석:
  - 초기 배치 문제와 같은 축으로 보는 것이 타당하다.

#### 4. 떨림 / 이중 적용 PASS
- 의미:
  - 최소한 현재 구조가 두 경로에서 동시에 같은 값을 덮어쓰며 흔들리는 문제는 강하게 보이지 않는다.
  - 따라서 이번 수정의 우선순위는 "중복 제거"보다 "기준점 정렬과 스핀 경로 복구"다.

#### 5. 런타임 디버그 UX
- 현재 상태:
  - 디버그 정보 자체는 유용하다.
  - 하지만 `Runtime` 줄이 너무 길어 한눈에 읽기 어렵다.
- 해석:
  - 기능 실패는 아니지만, 수정 후 재검증 단계에서 가독성 개선 가치가 있다.

### 수정 전 확정 기준
#### A. 현재 확정된 사실
- `ApplyVehicleLayoutConfig()`는 현재 `Wheel_Mesh_*`만 재배치한다.
- `WheelSync`는 `Wheel_Anchor_*`의 기본 상대 위치/회전을 기준으로 조향 / 서스펜션 / 스핀 적용 기준을 잡는다.
- 현재 설정상 `bUseWheelSpinPitchDebugPipe = false`, `bApplySpinPitchInCpp = false`라 스핀 시각 반응은 꺼진 상태로 읽힌다.
- 런타임 오버레이의 `RuntimeReady = True`, `WheelSync = True`는 준비 상태를 뜻하며, 시각 결과 정상 여부를 보장하지 않는다.

#### B. 현재 가장 강한 가설
- 초기 배치 FAIL과 조향 피벗 FAIL은 같은 원인 축일 가능성이 높다.
- 현재 구조는 `Anchor는 회전 기준`, `Mesh는 배치 대상`으로 갈라져 있어서, 눈에 보이는 바퀴 위치와 실제 회전 중심이 분리됐을 가능성이 높다.

#### C. 아직 확정되지 않은 부분
- 최종 기준을 `Anchor 중심 구조`로 고정할지
- 현재 휠 메시 원본 피벗이 바퀴 중심에 맞는지
- 스핀 값을 단기 복구용 경로로 먼저 넣을지, 실제 휠 회전값 경로로 바로 연결할지

#### D. 수정 전 기본안
- `Wheel_Anchor_*`를 바퀴 중심점 / 조향 피벗의 단일 기준으로 둔다.
- `Wheel_Mesh_*`는 Anchor 자식 기준 시각 표현 전용으로 둔다.
- Layout은 수동 `Wheel_Anchor_*` 배치를 기준선으로 둔다.
- 조향은 Anchor, 스핀은 Mesh에 나눠 적용하는 구조를 기본안으로 본다.

#### E. 수정 전 확인 순서
1. `BP_CFVehiclePawn`에서 각 `Wheel_Mesh_*`의 상대 위치 / 회전이 Anchor 기준으로 얼마나 어긋나 있는지 확인한다.
2. 휠 메시 원본 피벗이 바퀴 중심에 있는지 확인한다.
3. `DA_PoliceCar`의 WheelLayout이 최종적으로 어느 컴포넌트를 움직여야 하는지 확정한다.
4. 그 다음에만 코드 수정안을 작성한다.

### 수정 설계 v1.0
#### 1. 설계 결론
- 현재 기본안은 `Wheel_Anchor_*`를 바퀴 중심점 / 조향 피벗 / 레이아웃 배치 기준으로 둔다.
- `Wheel_Mesh_*`는 Anchor 자식 기준의 시각 표현 전용으로 둔다.
- `PoliceCar`는 수동 Anchor 배치를 현재 기준선으로 고정한다.
- 따라서 레거시 `AutoFit / WheelLayout` 자동 배치 경로는 현재 런타임 기준에서 제거한다.

#### 2. 파일 단위 수정 대상
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `ApplyVehicleLayoutConfig()` 수정 대상
- `UE/Source/CarFight_Re/Private/CFWheelSyncComp.cpp`
  - 스핀 적용 경로 복구 대상
- 필요 시 확인:
  - `UE/Source/CarFight_Re/Public/CFWheelSyncComp.h`

#### 3. 함수 단위 수정 방향
- `ApplyVehicleLayoutConfig()`
  - 런타임 자동 배치를 제거하고, 수동 Anchor 배치를 기준선으로 존중한다.
  - 레거시 `AutoFit` 요청이 남아 있어도 현재 기준 런타임에서는 무시한다.
- `ApplySingleWheelInputPhase2()`
  - 조향은 현재처럼 `Wheel_Anchor_*` Yaw 적용을 유지한다.
  - 스핀은 `Wheel_Mesh_*` Pitch 적용 경로를 복구한다.
  - 배치 기준 정리 후에도 피벗이 어색하면 Anchor/Mesh 축 방향 보정을 별도 검토한다.

#### 4. 수정 순서
1. `PoliceCar` 수동 Anchor 기준선 고정
2. 런타임 자동 배치 제거
3. PIE에서 초기 배치 / 조향 피벗 재검증
4. 그 다음에 스핀 적용 경로 복구
5. 마지막으로 `Runtime` 디버그 줄 가독성 개선

#### 5. 기대 결과
- 수동 Anchor 기준선에서 바퀴가 의도한 위치를 유지한다.
- 조향 시 앞바퀴 피벗이 차체 중앙이 아니라 각 바퀴 중심에서 자연스럽게 돈다.
- 스핀 경로 복구 후 전진 시 바퀴가 굴러간다.
- 현재 PASS인 `P0-001`, `P0-003`은 건드리지 않는다.

#### 6. 적용 상태
- 2026-03-30 기준 1차 코드 수정:
  - 수동 `Wheel_Anchor_*` 기준선에서 바퀴 위치와 조향 피벗이 PASS였다.
  - 이후 현재 기준선은 런타임 자동 배치를 제거하고 수동 Anchor 배치를 공식 기준으로 유지하는 방향으로 바뀌었다.
  - 자동 배치 / 반자동 배치는 현재 프로젝트 계획에서 제외한다.
  - `CFWheelSyncComp v1.1.1`에서 실제 휠 회전값 기반 SpinPitch 입력 복구 코드를 적용했다.
  - `CFWheelSyncComp v1.1.2`에서 helper 회전값의 Pitch / Roll 중 스핀에 더 가까운 축을 선택하는 보강을 추가했다.
  - `CarFight_ReEditor Win64 Development` 빌드는 성공했다.
  - 현재 단계는 수동 기준선 유지 상태에서 spin PIE 재검증 대기다.
- 2026-03-31 기준 추가 확인:
  - 수동 Anchor 기준선에서는 `바퀴 위치 PASS`, `조향 피벗 PASS`가 유지된다.
  - 현재 남은 실패 본체는 `Wheel spin 방향 안정성`이다.
  - 실제 런타임 문자열상 `RuntimeSpin=True`, `Success=True`가 확인되므로 스핀 경로 자체가 완전히 죽은 상태는 아니다.
  - 다만 `Speed=0`, `Forward=0` 근처에서도 휠 각속도 부호가 작은 음수/양수로 흔들리고, 현재 구현은 이 raw 부호를 너무 직접 믿고 있다.
  - 그 결과 전진 / 후진 반복 이후 어느 순간 시각 바퀴가 정방향 / 역방향으로 번갈아 보이는 불안정이 발생한다.
  - 따라서 다음 수정은 `절대각 / raw angular velocity`를 그대로 쓰지 않고, `Forward 기반 방향 안정화 + 휠 각속도 기반 회전 크기`를 조합하는 하이브리드 해석기로 간다.
  - 이 단계에서는 `DriveState` FSM을 확장하기보다 `WheelSpinRuntimeState` 성격의 가벼운 시각 회전 안정화 상태를 `UCFWheelSyncComp` 내부에 둔다.
- 2026-03-31 기준 추가 수정 / 재검증:
  - `CFWheelSyncComp v1.1.7`에서 누적 absolute pitch 적용 대신 `delta local rotation` 경로로 바꿨다.
  - 동일 날짜 후속 수정 `CFWheelSyncComp v1.1.8`에서 `WheelSpinMeshAxisSign = -1.0` 축 보정을 추가했다.
  - 현재 `WheelSyncBuild` 문자열에서 `VisualSign=-1.0`이 확인되며, 전진 / 후진 시각 회전 방향은 눈검사 기준 정상이다.
  - 런타임 디버그 기준 해석 규칙:
    - `Forward`는 차량 진행 방향 기준 속도다.
    - `Vel`은 Chaos 휠 각속도 부호이며, 현재 프로젝트 PoliceCar 메시 축과는 반대 부호로 읽힌다.
    - 따라서 현재 운영 기준에서 최종 시각 회전 판정은 `Forward + Sign + VisualSign + Step` 조합으로 본다.
  - 현재 단계 판정:
    - `Wheel spin 방향 반전` 버그는 수정 성공으로 본다.
    - 다만 `P0-002 WheelSync` 전체 PASS는 아직 확정하지 않고, 저속 blur / 과회전 잔여 여부를 추가 확인한다.
- 2026-03-31 기준 바퀴 파묻힘 재진단:
  - 수동 `Wheel_Anchor_*` 기준선은 그대로 유지한 상태에서 파묻힘 원인을 다시 분리 확인했다.
  - 사용자 PIE 재검증 기준 `WheelRadius = 39`에서는 바퀴가 더 작게 잡혀 바닥에 묻혀 보였고, `WheelRadius = 43`에서는 겉보기 접지 높이가 자연스러워졌다.
  - 같은 세션에서 `WheelSync` 쪽 `Suspension Z` 보정 실험도 있었지만, 이 경로는 오히려 파묻힘을 더 키웠다.
  - 따라서 이번 이슈의 본체는 `WheelSync` 런타임 동기화 실패보다 `DA_PoliceCar`의 물리 휠 반경 불일치로 판정한다.
  - 현재 안전 기준:
    - 바퀴 파묻힘 수정은 `WheelRadius` 값 조정으로 해결한다.
    - `WheelSync`의 실험성 `Suspension Z` 보정은 운영 기본안으로 채택하지 않는다.
    - `bApplySuspensionZInCpp = false` 상태를 유지한다.
  - 현재 세션 확인 기준:
    - `FrontWheelRadius = 43`
    - `RearWheelRadius = 43`
  - 해석:
    - 이 이슈는 시각 Anchor 기준선 보정보다 실제 물리 반경 기준선 정렬이 먼저였다고 본다.
- 2026-03-31 기준 최종 종료 체크:
  - `기준선 유지`: PASS
  - `저속 전진 스핀`: PASS
  - `저속 후진 스핀`: PASS
  - `전진 / 후진 반복 후 방향 안정성`: PASS
  - `조향 동시 반응`: PASS
  - `시각 깨짐`: PASS
  - 최종 판정: `P0-002 PASS`
  - 사용자 메모:
    - 고속 타이어 효과의 모양새는 아직 아주 만족스럽지 않다.
    - 다만 현재 단계에서는 기능 기준으로는 정상 작동으로 본다.
  - 후속 과제:
    - 이는 `WheelSync FAIL`이 아니라 고속 휠 시각 품질 개선 항목으로 분리해서 본다.

---

## P0-003 DriveState 공통 코어 판정
### 확인 항목
- [x] 전진 / 제동 / 정지 / 후진 기본 상태 흐름이 정상이다.
- [x] 작은 요철에서 `Grounded` 유지가 과도하게 흔들리지 않는다.
- [x] 분명한 공중 상태에서 `Airborne` 진입이 가능하다.
- [x] 현재 판정 구조가 다른 차량에도 옮겨갈 수 있는 공통 규칙처럼 읽힌다.

### 확인 메모
- VehicleData: `DA_PoliceCar`
- Override 사용 여부: `bUseDriveStateOverrides = true`
- 비고:
  - 현재 실측값:
    - `bEnableDriveStateHysteresis = true`
    - `bUsePerStateHoldTimes = true`
    - `bTreatOppositeThrottleAsBrake = true`
    - `ActiveInputThreshold = 0.05`
    - `IdleEnter/Exit = 0.75 / 1.50`
    - `ReverseEnter/Exit = 1.25 / 0.75`
    - `AirborneMinSpeedThresholdKmh = 3.0`
    - `AirborneVerticalSpeedThresholdCmPerSec = 100.0`
  - 현재 문서는 실제 자산값 기준으로 반영했다.
  - 어셋덤프 기준으로는 DriveState 오버라이드 연결과 현재 수치 확인까지 완료다.
  - 다만 현재 기본값에서 `bEnableDriveStateOnScreenDebug = false`이므로, 런타임 상태 문자열 검증 전 사전 조정이 필요하다.
  - 2026-03-30 PIE 기준 결과:
    - `Idle -> 전진`: PASS
    - `전진 유지`: PASS
    - `입력 해제 후 상태 변화`: PASS
    - `브레이크 / 정지 / Idle 복귀`: PASS
    - `후진 진입`: PASS
    - `Grounded 유지`: PASS
    - `Airborne 진입`: PASS
    - `디버그 가독성`: PASS
  - 사용자 메모:
    - 상태 정보 자체는 읽기 충분하다.
    - 다만 `Runtime` 줄이 너무 길어서 보기 편한 형태로 재구성할 필요가 있다.

---

## DriveState 공통 코어 체크리스트
아래 항목은 차량별 세부 튜닝 전에 먼저 본다.

| 항목 | 상태 | 메모 |
|---|---|---|
| 전진 입력 진입 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 전진 유지 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 입력 해제 후 Coasting | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 브레이크 진입 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 정지 판정(Idle) | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 후진 진입 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 후진 유지 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 전진↔후진 경계 안정성 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 작은 요철에서 Grounded 유지 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 큰 점프/낙하에서 Airborne 진입 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 착지 후 상태 복귀 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |
| 디버그 표시 가독성 | [ ] 통과 / [ ] 보류 / [ ] 실패 | |

---

## DA_PoliceCar 시작값 기준
아래 값은 2026-03-27 기준 `DA_PoliceCar` 실측값이다.
즉, 추천 예시가 아니라 **현재 프로젝트에 실제로 들어 있는 기준값**으로 본다.

### 기본 토글
- `bUseDriveStateOverrides = true`
- `bEnableDriveStateHysteresis = true`
- `bUsePerStateHoldTimes = true`
- `bTreatOppositeThrottleAsBrake = true`

### 입력 판정
- `ActiveInputThreshold = 0.05`

### Idle 판정
- `IdleEnterSpeedThresholdKmh = 0.75`
- `IdleExitSpeedThresholdKmh = 1.50`
- `IdleStateMinimumHoldTimeSeconds = 0.10`

### Reverse 판정
- `ReverseEnterSpeedThresholdKmh = 1.25`
- `ReverseExitSpeedThresholdKmh = 0.75`
- `ReversingStateMinimumHoldTimeSeconds = 0.18`

### Airborne 판정
- `AirborneMinSpeedThresholdKmh = 3.00`
- `AirborneVerticalSpeedThresholdCmPerSec = 100.0`

### 바퀴 반경 검증 메모
- 2026-03-31 사용자 PIE 재검증 기준:
  - `WheelRadius = 39`: FAIL
    - 바퀴가 실제보다 작게 잡혀 바닥에 묻혀 보였다.
  - `WheelRadius = 43`: PASS
    - 정지 상태와 저속 주행 상태에서 접지 높이가 더 자연스럽게 보였다.
- 현재 운영 메모:
  - 이 값은 `WheelSync` 대체 로직이 아니라 `DA_PoliceCar` 물리 기준선 조정 결과로 기록한다.
  - 같은 문제를 다시 만났을 때는 `Anchor Z`보다 먼저 `WheelRadius` 실측을 확인한다.

---

## TestMap 검증 메모
아래는 반복 테스트 시 같이 적는다.

- 시작 위치: `PlayerStart_0`
- 차량 배치 방식: 맵에 `BP_CFVehiclePawn_C_2`가 직접 배치된 기준 구성
- 기본 주행 루프: 현재 문서 기준으로는 "배치된 차량 진입 -> 기본 주행 -> 디버그 확인"까지가 최소 루프다.
- 요철 / 점프 확인 위치: 현재 덤프만으로는 확정 불가
- 디버그 확인 절차:
  - 현재 기준 Pawn 선택
  - WheelSync / DriveState 표시 상태 확인
  - 필요 시 PIE에서 추가 재검증
- 맵 보강 필요 사항:
  - 작은 요철 구간
  - 분명한 점프 / 낙하 구간
  - 반복 가능한 코어 검증 루프 표지

---

## 현재 실행 순서
### 1. P0-001 기본 주행
- `TestMap` 열기
- `BP_CFVehiclePawn` 기준으로 PIE 실행
- 게임패드 입력 확인
- 기본 주행 가능 여부 기록

### 2. P0-002 Wheel Sync
- 주행 / 조향 중 휠 시각 반응 확인
- 떨림 / 이중 적용 여부 기록
- 필요 시 인스턴스의 WheelSync 상태 확인

### 3. P0-003 DriveState
- 전진 / 입력 해제 / 브레이크 / 정지 / 후진 순서로 상태 확인
- 작은 요철과 큰 낙하 상황에서 `Grounded / Airborne` 반응 기록
- 코어 문제와 차량 수치 문제를 분리해서 메모

---

## 카메라 관련 추적 항목
- [ ] 현재 기준 Pawn에서 플레이 시점이 정상이다.
- [ ] 카메라 문제를 주행 / 휠 코어 판정과 분리해서 기록할 수 있다.
- [ ] 후속 상태 기반 카메라 반응 확장 여지를 남긴다.

---

## 최종 판정
- D0 문서 정렬: PASS
- P0 문서 실측 반영: PASS
- P0-001: PASS
- P0-002: PASS
- P0-003: PASS
- 어셋덤프 기준 부분 체크:
  - P0-001 구조 / 배치 / 입력 자산 연결: PASS
  - P0-002 컴포넌트 / 기본 설정 확인: PASS
  - P0-003 오버라이드 / 기준값 확인: PASS
- 작성자 / 날짜:
