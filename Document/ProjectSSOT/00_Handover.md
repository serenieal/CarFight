# CarFight — 00_Handover

> 역할: CarFight 프로젝트의 **현재 실제 기준선 / 임시 운영 편차 / 현재 리스크**를 고정한다.  
> 공통 규칙 원본: `Document/SSOT/`  
> 문서 버전: v1.1.0  
> 마지막 정리(Asia/Seoul): 2026-04-15


---

## 먼저 확인할 방향
- 최종 방향은 `03_VisionAlign.md`를 기준으로 본다.
- 이 문서는 그 방향 아래에서 **지금 실제로 굴러가는 상태만** 적는다.

---

## 현재 실제 기준선
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`
- 현재 기준 테스트 맵: `/Game/Maps/TestMap`

---

## 현재 상태 요약
### 1. 차량 구조
- 현재 차량 구조는 `CMVS`가 아니라 `ChaosWheeledVehicle` 기반 하이브리드 구조다.
- 현재 기준 Pawn은 표준 차량 이동 컴포넌트 위에 `DriveState`, `WheelSync`, `VehicleData`를 올려 사용하는 방식이다.

### 2. BP / C++ 책임 분리
- 현재 기준은 C++ 코어 우선이다.
- `BP_CFVehiclePawn`는 Thin BP로 유지한다.
- 실제 판단 / 상태 / 규칙은 Native 쪽에 두는 방향을 유지한다.

### 3. 데이터 축
- 차량별 차이는 `UCFVehicleData` 축으로 분리하는 방향이 이미 적용되어 있다.
- 현재 실차 기준은 `DA_PoliceCar` 하나다.

### 4. 현재 작업 제약
- 2026-03-27 기준으로 `ue-assetdump` 플러그인 개선 작업 때문에 Unreal Editor 실행이 막혀 있었다.
- 2026-03-30 기준 현재는 에디터 복구가 확인되었고, PIE 기반 런타임 검증을 재개할 수 있다.
- 2026-03-31 기준 `WheelSync` 쪽에서 런타임 spin 경로 재검증이 다시 가능해졌고, 방향 반전 수정까지 반영됐다.
- 2026-03-31 후속 종료 체크 기준 `P0-002 WheelSync`는 PASS로 닫았다.
- 2026-04-01 기준 현재 상태:
  - `Runtime` 디버그 가독성 개선은 완료로 본다.
  - 바퀴 파묻힘은 `Anchor Z`가 아니라 `WheelRadius` 조정으로 해결했다.
  - `ACFVehiclePawn` 디테일 패널 카테고리 정리를 위해 `CarFight|VehiclePawn` 루트 분리가 반영됐다.
  - 후속 메타데이터 정리로 `VehicleDriveComp`는 `Vehicle Drive`, 컴포넌트 참조는 `컴포넌트` 루트로 분리되어 `Vehicle Pawn` 중복 표시가 해소됐다.
  - `CFVehicleData` 내부 `AutoFit / VehicleLayoutConfig / WheelLayout` 레거시 구조는 하드 삭제 완료 상태로 본다.
- 따라서 현재 다음 우선순위는 문서 작업이 아니라 고속 휠 시각 품질 검토다.

---

## 실측 스냅샷 (2026-03-27)
아래 항목은 문서 추정이 아니라 자산 덤프와 현재 코드 기준으로 다시 확인한 값이다.

### 1. `BP_CFVehiclePawn` 실측
- 부모 클래스는 `CFVehiclePawn`이다.
- 요약 기준:
  - 컴포넌트 15개
  - 그래프 2개
  - `EventGraph` 노드 8개
  - `UserConstructionScript` 노드 1개
- 현재 확인된 주요 컴포넌트:
  - `VehicleMesh`
  - `VehicleMovementComp`
  - `VehicleDriveComp`
  - `WheelSyncComp`
  - `SM_Chassis`
  - `Wheel_Anchor_FL / FR / RL / RR`
  - `Wheel_Mesh_FL / FR / RL / RR`
  - `SpringArm`
  - `Camera`
- 현재 클래스 기본값 기준:
  - `VehicleData = DA_PoliceCar`
  - `InputDeviceMode = GamepadOnly`
  - `bEnableDriveStateOnScreenDebug = false`
  - `DriveStateDebugDisplayMode = MultiLine`
  - `bShowDriveStateTransitionSummary = true`
- `EventGraph`는 현재 기준으로 매우 얇고, 주 용도는 `BeginPlay` 시점의 디버그 위젯 생성이다.
- 현재 `VehicleMesh`에는 기존 차량용 스켈레탈 메시가 남아 있다.

### 2. `WheelSyncComp` 초기 실측 (2026-03-27, 역사 기록)
- 아래 항목은 `2026-03-27` 시점의 초기 실측 / 실패 기록이다.
- 현재 운영 기준은 아래 `2-2. WheelSyncComp 운영 메모`를 우선해서 본다.
- 현재 기준 설정:
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
- 현재 디버그 / helper / compare 관련 설정:
  - `bDebugMode = false`
  - `bUseRealWheelTransformHelper = false`
  - `bEnableHelperCompareMode = false`
  - 레거시 단일축 테스트 필드는 비활성 상태다.
- 현재 덤프 기준 런타임 준비 상태:
  - `bWheelSyncReady = false`
  - `LastValidationSummary = NotValidated`
- 따라서 현재 문서 기준으로는 "보수적 기본 설정은 확인됨"까지는 적을 수 있지만, "런타임 준비 완료"까지는 아직 적지 않는다.
- 2026-03-30 PIE 기준 확인된 실제 증상:
  - 휠 초기 배치가 비정상이다.
  - 전진 중 바퀴 스핀 시각 반응이 없다.
  - 조향 시 앞바퀴 피벗이 비정상적으로 보인다.
  - 심한 떨림 / 이중 적용은 보이지 않았다.
- 현재 런타임 오버레이에서는 `RuntimeReady = True`, `WheelSync = True`가 표시됐지만, 실제 시각 결과는 FAIL이다.
- 현재 설정 기준으로는 `bUseWheelSpinPitchDebugPipe = false`, `bApplySpinPitchInCpp = false`라 바퀴 스핀 시각 반응이 비활성일 가능성이 크다.

### 2-2. `WheelSyncComp` 운영 메모 (2026-03-31 PIE 기준)
- 현재 런타임 문자열 기준:
  - `SpinOwned = On`
  - `RuntimeSpin = True`
  - `VisualSign = -1.0`
- 현재 `UCFWheelSyncComp` 운영 경로 메모:
  - `CFWheelSyncComp v1.1.8`
  - `bApplySpinPitchInCpp = true`
  - `WheelSpinVisualSign = 1.0`
  - `WheelSpinMeshAxisSign = -1.0`
  - 저속 cap + sign hold + delta local rotation 경로가 활성화된 상태다.
- 현재 판정:
  - 수동 Anchor 기준선에서 `바퀴 위치 PASS`, `조향 피벗 PASS`는 유지된다.
  - 전진 / 후진 시각 회전 방향 반전 문제는 수정 후 눈검사 기준 정상으로 읽힌다.
  - `P0-002 WheelSync`는 종료 체크 기준 PASS로 판정했다.
  - 고속 타이어 효과의 모양새는 아직 개선 여지가 있지만, 현재 기준에서는 기능 FAIL이 아니라 시각 품질 후속 과제다.

### 2-3. 현재 런타임 판정 요약
- `P0-001 기본 주행`: PASS
- `P0-002 WheelSync`: PASS
- `P0-003 DriveState`: PASS
- 현재 기준선에서 가장 큰 남은 문제는 WheelSync 코어 FAIL이 아니라 고속 휠 시각 품질과 레거시 구조 정리다.
- DriveState 코어 자체는 현재 기준선에서 정상적으로 읽힌다.
- 디버그 가독성은 PASS지만 `Runtime` 줄이 너무 길어 보기 편한 형태로 재구성할 필요가 있다.

### 3. `DA_PoliceCar` 실측
- 시각 구성:
  - 차체는 `Combined_Body`
  - 바퀴는 `Wheel_FL / FR / RL / RR`
- 레이아웃:
  - 현재 기준선은 `Wheel_Anchor_*` 수동 배치를 사용한다.
- 레거시 `AutoFit` 필드는 현재 기준 런타임에서 사용하지 않으며, 현재 코드 기준에서는 하드 삭제됐다.
- 현재 코드 정리 기준:
  - `CFVehicleData` 안의 `VehicleLayoutConfig / WheelLayout / AutoFit` 필드는 제거됐다.
  - 현재 운영 기준선은 `수동 Wheel_Anchor 배치 + WheelRadius`만 사용한다.
- `PoliceCar` 수동 기준선 좌표:
    - `FL = (195.069249, -94.740381, 12.036834)`
    - `FR = (195.069249, 94.740381, 12.036834)`
  - `RL = (-162.369103, -94.740381, 12.036834)`
  - `RR = (-162.369103, 94.740381, 12.036834)`
- 물리 기준선:
  - 바퀴 파묻힘 이슈는 `WheelRadius` 조정으로 해결했다.
  - 현재 확인 기준:
    - `FrontWheelRadius = 43`
    - `RearWheelRadius = 43`
- 무브먼트:
  - `bUseMovementOverrides = true`
  - 하지만 현재 덤프 기준 세부 `b_override_*` 플래그는 모두 `false`다.
  - 즉, 오버라이드 게이트는 켜져 있지만 상세 수치 교체는 아직 비활성 상태다.
- 참조형 자산:
  - `FrontWheelClass = BP_Wheel_Front`
  - `RearWheelClass = BP_Wheel_Rear`
- DriveState:
  - `bUseDriveStateOverrides = true`
  - `bEnableDriveStateHysteresis = true`
  - `bUsePerStateHoldTimes = true`
  - `bTreatOppositeThrottleAsBrake = true`
  - `ActiveInputThreshold = 0.05`
  - `IdleEnter/Exit = 0.75 / 1.50`
  - `ReverseEnter/Exit = 1.25 / 0.75`
  - `AirborneMinSpeedThresholdKmh = 3.0`
  - `AirborneVerticalSpeedThresholdCmPerSec = 100.0`

### 4. `TestMap` 실측
- 맵 경로는 `/Game/Maps/TestMap`이다.
- 현재 덤프 기준 액터 수는 11개다.
- 기준 차량으로 `BP_CFVehiclePawn_C_2`가 배치되어 있다.
- `PlayerStart_0`가 함께 존재한다.
- 현재 맵은 매우 작은 기준 맵이며, 기본 주행 / 디버그 / 기준 차량 배치 확인에는 적합하지만 다양한 물리 상황 검증에는 아직 제한적일 수 있다.

---

## 임시 운영 편차
아래 항목은 현재 운영상의 편차이며, 구조 방향 자체를 바꾼 것으로 해석하지 않는다.

### 1. 입력 장치
- 현재 테스트는 게임패드 중심으로 고정되어 있다.
- 키보드 / 게임패드 전환 과정의 오류를 피하기 위한 임시 운영이다.

### 2. 테스트 차량 집중
- 현재 검증은 `DA_PoliceCar` 중심이다.
- 이는 첫 기준선 확보를 위한 선택이며, 최종 차종 구조를 고정한 것은 아니다.

---

## 현재 구조 갭으로 보는 항목
아래 항목은 임시 운영 편차가 아니라 실제 구조 갭이다.

### 1. 최종 구조 미도입
- `CMVS / Cluster Union / Geometry Collection` 기반 구조가 아직 현재 코어에 반영되지 않았다.

### 2. 기존 vehicle root 의존
- 현재 기준 Pawn은 여전히 `ChaosWheeledVehicleMovementComponent` 중심 구조를 사용한다.

### 3. 조립형 차량 파이프라인 미도입
- 데이터에서 차량을 조립하고 재구성하는 최종 파이프라인은 아직 현재 구현 기준선이 아니다.

---

## 현재 리스크
### 1. 코어가 현 구조에 너무 고정될 위험
- `DriveState`와 `WheelSync`가 현 vehicle root 구조에 너무 강하게 묶이면 나중에 구조 전환 비용이 커질 수 있다.

### 2. 테스트 차량이 기준선에서 종착점으로 굳어질 위험
- `DA_PoliceCar` 기준 검증이 길어질수록 다차종 확장 관점이 약해질 수 있다.

### 3. 문서와 구현이 다시 분리될 위험
- 방향 문서와 현재 기준 문서가 다시 섞이면, 어떤 작업이 임시 대응이고 어떤 작업이 구조 전환 준비인지 흐려질 수 있다.

---

## 검증 기준
- 현재 기준선 PASS / FAIL 판정은 `08_P0_Verification.md`를 기준으로 본다.
- 구조 유지 / 교체 판단은 `16_CPP_DecisionLog.md`에 남긴다.
- 실제 작업 순서는 `01_Roadmap.md`를 기준으로 진행한다.

---

## 후속 세션에서 먼저 볼 순서
1. `03_VisionAlign.md`
2. `00_Handover.md`
3. `01_Roadmap.md`
4. `08_P0_Verification.md`
5. `16_CPP_DecisionLog.md`

---

## 현재 카메라 기준선 (2026-04-15 추가)
아래 항목은 현재 카메라 작업의 실제 기준선을 별도 정리한 메모다.
이 섹션은 최종 카메라 철학 전체를 설명하는 문서가 아니라, **현재 프로젝트에서 실제로 붙어 있는 카메라 기준선**만 기록한다.

### 1. 현재 구현 기준
- 현재 카메라 기준 플레이 Pawn은 `BP_CFVehiclePawn`이다.
- 현재 카메라 Native 코어는 `UCFVehicleCameraComp`다.
- 현재 카메라 공용 타입 / 데이터 구조는 아래 파일 기준으로 본다.
  - `CFVehicleCameraTypes.h`
  - `CFVehicleCameraData.h`
  - `CFVehicleCameraComp.h / .cpp`
- 현재 Pawn 입력 연동은 `ACFVehiclePawn`의 `InputAction_Look` 경로를 기준으로 한다.

### 2. 현재 BP 기준 카메라 계층
- 현재 기준 계층은 아래 이름을 기준으로 한다.
  - `CameraPivotRoot`
  - `CameraAimPivot`
  - `CameraBoom`
  - `FollowCamera`
- 현재 방향은 차량 중심 수평 피벗 기준 자유 조준 카메라다.
- 현재 카메라는 일반 레이싱 chase camera보다 차량 기반 3인칭 슈팅 카메라 성격에 가깝다.

### 3. 현재 입력 기준
- 현재 Look 입력 자산 기준은 `IA_LookAround`다.
- 현재 `IA_LookAround`는 `Axis2D` 기준으로 사용한다.
- 현재 `IMC_Vehicle_Default`에서는 마우스 2D 입력과 게임패드 오른쪽 썸스틱 2D축 입력이 정상 동작 기준선이다.
- `ACFVehiclePawn` 생성자 내부 입력 자산 경로는 하드 고정값이 아니라 fallback로만 남기고, BP/파생 클래스 지정값을 우선한다.

### 4. 현재 확인 완료 항목
- 빌드 성공 확인 완료
- 카메라 Yaw 회전 확인 완료
- 카메라 Pitch 회전 확인 완료
- SpringArm 기반 외부 3인칭 거리 계산 경로 반영 완료
- FOV 반영 경로 반영 완료
- Aim Trace 계산 경로 반영 완료

### 5. 현재 범위 한계
- 현재는 기본 카메라 움직임과 뼈대가 구현된 상태다.
- 아직 HUD / 조준점 / 진행 방향 힌트는 미구현이다.
- 아직 무기 시스템의 실제 Aim Profile 공급과 무기별 제한각 연동은 미구현이다.
- 따라서 현재 카메라 기준선은 **기본 회전과 시스템 골격 확보 단계**로 본다.

---

## 변경 이력
- v1.1.0 (2026-04-15)
  - 문서 버전 / 마지막 정리 날짜를 갱신했다.
  - 현재 카메라 기준선 섹션을 추가했다.

