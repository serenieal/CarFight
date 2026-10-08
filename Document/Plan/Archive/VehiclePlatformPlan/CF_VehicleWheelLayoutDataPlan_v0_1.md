# CF Vehicle Wheel Layout DataAsset Plan v0.1

- 작성일: 2026-06-23
- 현재 상태: Superseded / Legacy Reference
- 최신 기준: `DA_PoliceCar`는 더 이상 사용하지 않는다. 현재 기본 테스트 DA는 `DA_TestSedan`, 대조 후보는 `DA_TestSUV`다.
- 대상 프로젝트: `D:\Work\CarFight_git`
- 대상 BP: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 대상 C++: `ACFVehiclePawn`, `UCFVehicleData`, `UCFWheelSyncComp`
- 목표: `BP_CFVehiclePawn`에 수동 배치된 `Wheel_Anchor_FL/FR/RL/RR` 기준 위치를 차량별 `UCFVehicleData`에서 제어하는 구조로 전환한다.

---

## 1. 확인 근거

### 문서 기준
- `Document/Plan/DataPlan/CF_VehicleDataPlan.md`
  - 차량별 값은 DataAsset이 소유하고, 해석과 적용 순서는 C++가 소유해야 한다.
  - `Wheel_Anchor_FL/FR/RL/RR`, `Wheel_Mesh_FL/FR/RL/RR` 이름 규약은 가능하면 프로젝트 표준으로 유지한다.
  - BP는 Thin BP로 유지하고, 차종별 차이를 BP 자식 증식으로 풀지 않는다.
- `Document/Plan/VehiclePlatformPlan/README.md`
  - `DA_PoliceCar`는 현재 작업에 사용하지 않는 레거시 감사 자산이다.
  - 현재 기본 테스트 DA는 `DA_TestSedan`, 대조 후보는 `DA_TestSUV`다.
- `Document/SSOT/UE_SSOT/UE_CPP_SSOT/*`
  - 규칙, 계산, 상태, 검증은 C++가 소유한다.
  - BP는 조립, 표현, 에셋 연결, 얇은 UI 이벤트 바인딩 정도로 유지한다.

### 코드 기준
- `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - 현재 `UCFVehicleData`에는 `VehicleVisualConfig`, `VehicleMovementConfig`, `WheelVisualConfig`, `VehicleReferenceConfig`, `DriveStateConfig`가 있다.
  - 현재 레이아웃 전용 `VehicleLayoutConfig`는 없다.
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `ApplyVehicleLayoutConfig()`는 현재 `VehicleLayout: ManualAnchorLayout=Required` 요약만 남긴다.
  - `ApplyVehicleWheelPhysicsConfig()`는 `VehicleMovementConfig.FrontWheelAdditionalOffset / RearWheelAdditionalOffset`를 Chaos `WheelSetup.AdditionalOffset`에 적용한다.
  - `OnConstruction()`은 `ApplyVehicleVisualConfig()`, `ApplyVehicleWheelVisualConfig()`, `ApplyVehicleLayoutConfig()`를 호출한다.
  - `InitializeVehicleRuntime()`은 `ApplyVehicleDataConfig()` 이후 `PrepareOwnerVisualStabilization()`과 `PrepareWheelSync()`를 실행한다.
- `UE/Source/CarFight_Re/Private/CFWheelSyncComp.cpp`
  - `TryPrepareWheelSync()`는 `BuildWheelComponentCache()` 후 `CaptureBaseWheelVisualState()`를 호출한다.
  - `CaptureBaseWheelVisualState()`는 `Wheel_Anchor_*`의 현재 `RelativeLocation / RelativeRotation`을 기준값으로 캡처한다.

### BP / DataAsset 덤프 기준
- `BP_CFVehiclePawn` 부모 클래스는 `/Script/CarFight_Re.CFVehiclePawn`이다.
- 과거 기록상 `BP_CFVehiclePawn`의 `VehicleData`는 `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar`에 연결되어 있었다.
- 현재 저장 기준은 `DA_TestSedan`이다.
- `BP_CFVehiclePawn`의 현재 수동 바퀴 앵커 기준값:

| Anchor | RelativeLocation | RelativeRotation |
|---|---:|---:|
| `Wheel_Anchor_FL` | `(X=195.069249, Y=-94.740381, Z=12.036834)` | `(Pitch=0, Yaw=0, Roll=0)` |
| `Wheel_Anchor_FR` | `(X=195.069249, Y=94.740381, Z=12.036834)` | `(Pitch=0, Yaw=0, Roll=0)` |
| `Wheel_Anchor_RL` | `(X=-162.369103, Y=-94.740381, Z=12.036834)` | `(Pitch=0, Yaw=0, Roll=0)` |
| `Wheel_Anchor_RR` | `(X=-162.369103, Y=94.740381, Z=12.036834)` | `(Pitch=0, Yaw=0, Roll=0)` |

---

## 2. 현재 문제

현재 시각 휠 기준 위치는 `BP_CFVehiclePawn`의 컴포넌트 배치값에 묶여 있다.

문제는 다음과 같다.

- 차마다 휠베이스, 트레드, 휠 중심 높이가 다르면 BP 컴포넌트 배치를 직접 바꿔야 한다.
- 차량별 차이를 BP 자식으로 복제하기 시작하면 Thin BP 원칙이 깨진다.
- 물리 휠 오프셋은 이미 `UCFVehicleData.VehicleMovementConfig`에서 읽지만, 시각 휠 기준 위치는 DataAsset 경로가 없다.
- `CFWheelSyncComp`는 `CaptureBaseWheelVisualState()` 시점의 앵커 위치를 기준으로 삼기 때문에, DataAsset 레이아웃은 반드시 WheelSync 준비 전에 적용되어야 한다.

---

## 3. 목표 구조

목표는 `VehicleData`만 교체해도 차량별 바퀴 기준 위치가 바뀌는 구조다.

기본 구조:

1. `BP_CFVehiclePawn`은 공통 Thin BP로 유지한다.
2. `Wheel_Anchor_FL/FR/RL/RR` 컴포넌트 이름은 프로젝트 표준으로 유지한다.
3. `UCFVehicleData`에 차량 레이아웃 설정을 추가한다.
4. `ACFVehiclePawn::ApplyVehicleLayoutConfig()`가 `VehicleData`의 레이아웃 값을 읽어 `Wheel_Anchor_*` 상대 위치/회전을 적용한다.
5. `CFWheelSyncComp::CaptureBaseWheelVisualState()`는 DataAsset 적용 후의 앵커 값을 캡처한다.

핵심 원칙:

- DataAsset은 값 저장소다.
- C++는 값의 의미, 적용 순서, 실패 처리, 검증 요약을 소유한다.
- BP는 컴포넌트 이름 규약과 표현 조립만 유지한다.

---

## 4. 범위 / 비범위

### 범위
- `UCFVehicleData`에 최소 레이아웃 설정 구조 추가
- 네 바퀴 앵커의 상대 위치와 상대 회전 저장
- `ACFVehiclePawn::ApplyVehicleLayoutConfig()`에서 앵커 위치/회전 적용
- `WheelSync` 기준 캡처 전에 레이아웃이 적용되도록 런타임 순서 정리
- `BP_CFVehiclePawn` 현재 수동 좌표를 `DA_PoliceCar`로 옮기는 마이그레이션 절차 문서화
- `Tools\BuildEditor.bat` 기준 빌드 검증 계획 수립

### 비범위
- 레거시 AutoFit / 자동 휠베이스 계산 구조 복구
- `Wheel_Anchor_*` / `Wheel_Mesh_*` 이름 매핑의 차종별 가변화
- Chaos 물리 `WheelSetup.AdditionalOffset`와 시각 앵커 위치를 하나의 필드로 통합
- 휠 반경, 서스펜션, 마찰, 엔진 튜닝 재설계
- BP 컴포넌트 트리 대규모 리네이밍
- 새 차량 BP 자식 생성

---

## 5. 데이터 구조 제안

### 신규 구조체 1: `FCFWheelAnchorPose`

역할: 바퀴 앵커 하나의 기준 상대 Transform 중 위치와 회전을 담는다.

필드 제안:

| 필드 | 타입 | 기본값 | 설명 |
|---|---|---|---|
| `RelativeLocation` | `FVector` | `FVector::ZeroVector` | `Wheel_Anchor_*`에 적용할 상대 위치 |
| `RelativeRotation` | `FRotator` | `FRotator::ZeroRotator` | `Wheel_Anchor_*`에 적용할 상대 회전 |

### 신규 구조체 2: `FCFVehicleLayoutConfig`

역할: 차량 한 대의 표준 바퀴 앵커 기준 배치를 담는다.

필드 제안:

| 필드 | 타입 | 기본값 | 설명 |
|---|---|---|---|
| `bUseLayoutOverrides` | `bool` | `false` | `true`일 때 DataAsset 레이아웃을 실제 앵커에 적용 |
| `WheelAnchorFL` | `FCFWheelAnchorPose` | Zero | 앞왼쪽 바퀴 앵커 기준값 |
| `WheelAnchorFR` | `FCFWheelAnchorPose` | Zero | 앞오른쪽 바퀴 앵커 기준값 |
| `WheelAnchorRL` | `FCFWheelAnchorPose` | Zero | 뒤왼쪽 바퀴 앵커 기준값 |
| `WheelAnchorRR` | `FCFWheelAnchorPose` | Zero | 뒤오른쪽 바퀴 앵커 기준값 |

### `UCFVehicleData` 추가 필드

```cpp
FCFVehicleLayoutConfig VehicleLayoutConfig;
```

기본값 정책:

- `bUseLayoutOverrides = false`로 둔다.
- 기존 DataAsset은 코드 변경 직후에도 현재 BP 수동 배치 그대로 동작해야 한다.
- `DA_PoliceCar`처럼 마이그레이션이 끝난 자산만 `bUseLayoutOverrides = true`로 켠다.

---

## 6. 수정 대상 파일

### 신규 문서
- `Document/Plan/VehiclePlatformPlan/CF_VehicleWheelLayoutDataPlan_v0_1.md`

### C++ 수정 후보
- `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - `FCFWheelAnchorPose` 추가
  - `FCFVehicleLayoutConfig` 추가
  - `UCFVehicleData::VehicleLayoutConfig` 추가
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `ApplyVehicleLayoutConfig()` 실제 적용 구현
  - 런타임 초기화에서 `PrepareWheelSync()` 직전 레이아웃 재적용
  - 실패/성공 요약 문자열 정리
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - 새 private helper를 멤버 함수로 둘 경우 선언 추가
  - 단순 파일 로컬 helper로 충분하면 수정하지 않는다.

### 에디터 / 에셋 수정 후보
- `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar`
  - 현재 `BP_CFVehiclePawn`의 `Wheel_Anchor_*` 좌표를 `VehicleLayoutConfig`로 이관
  - `bUseLayoutOverrides = true` 설정
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
  - 첫 단계에서는 컴포넌트 이름과 현재 배치값을 유지한다.
  - 검증 후에도 BP는 공통 Thin BP로 유지한다.

---

## 7. 예상 C++ 적용 흐름

### 에디터 미리보기
1. `ACFVehiclePawn::OnConstruction()`
2. `ApplyVehicleVisualConfig()`
3. `ApplyVehicleWheelVisualConfig()`
4. `ApplyVehicleLayoutConfig()`
5. 에디터 뷰포트에서 DataAsset 기준 앵커 배치 확인

### 런타임
1. `ACFVehiclePawn::BeginPlay()`
2. `InitializeVehicleRuntime()`
3. `ApplyVehicleDataConfig()`
4. `PrepareOwnerVisualStabilization()`
5. `ApplyVehicleLayoutConfig()`
   - Owner 표시 루트 재부착 이후 최종 부모 기준 상대 Transform을 보장하기 위한 재적용이다.
6. `PrepareWheelSync()`
7. `UCFWheelSyncComp::TryPrepareWheelSync()`
8. `CaptureBaseWheelVisualState()`
   - DataAsset에서 적용한 앵커 위치/회전이 기준값으로 캡처된다.

### 실패 처리 기본안
- `VehicleData == nullptr`
  - 레이아웃 적용 생략, 기존 BP 배치 유지
- `bUseLayoutOverrides == false`
  - 레이아웃 적용 생략, 요약 문자열에 `ManualAnchorLayout=Fallback` 계열로 기록
- 특정 `Wheel_Anchor_*` 컴포넌트 누락
  - 누락 이름을 요약 문자열에 남기고 `WheelSync` 검증 실패 원인을 분리 가능하게 한다.

---

## 8. BP_CFVehiclePawn 마이그레이션 방식

1. `BP_CFVehiclePawn`의 부모는 `CFVehiclePawn` 그대로 유지한다.
2. `Wheel_Anchor_FL/FR/RL/RR` 이름은 그대로 유지한다.
3. `Wheel_Mesh_FL/FR/RL/RR` 이름도 그대로 유지한다.
4. EventGraph에는 레이아웃 계산 로직을 추가하지 않는다.
5. 첫 구현 단계에서는 BP의 기존 수동 좌표를 지우지 않는다.
   - 이유: DataAsset 오버라이드가 꺼져 있거나 누락된 경우 안전한 fallback으로 동작해야 한다.
6. `DA_TestSedan` 검증이 끝난 뒤에만 BP 기본 앵커값을 중립화할지 별도 결정한다.
   - 현재 계획에서는 중립화하지 않는다.

---

## 9. DataAsset 마이그레이션 방식

### 과거 `DA_PoliceCar` 시작값

`VehicleLayoutConfig.bUseLayoutOverrides = true`

| 필드 | RelativeLocation | RelativeRotation |
|---|---:|---:|
| `WheelAnchorFL` | `(195.069249, -94.740381, 12.036834)` | `(0, 0, 0)` |
| `WheelAnchorFR` | `(195.069249, 94.740381, 12.036834)` | `(0, 0, 0)` |
| `WheelAnchorRL` | `(-162.369103, -94.740381, 12.036834)` | `(0, 0, 0)` |
| `WheelAnchorRR` | `(-162.369103, 94.740381, 12.036834)` | `(0, 0, 0)` |

### 에디터 작업 순서
1. 콘텐츠 브라우저(`Content Browser`)에서 `DA_TestSedan`을 연다.
2. 차량 레이아웃 설정(`VehicleLayoutConfig`) 섹션을 찾는다.
3. 레이아웃 덮어쓰기 사용(`bUseLayoutOverrides`)을 켠다.
4. 위 표의 네 바퀴 값을 입력한다.
5. 저장(`Save`)한다.
6. `BP_CFVehiclePawn`의 차량 데이터(`VehicleData`)가 `DA_TestSedan`인지 확인한다.
7. 에디터 뷰포트 또는 PIE에서 바퀴 위치가 기존과 동일한지 확인한다.

### 신규 차종 DataAsset
신규 세단/SUV DataAsset은 BP를 복제하지 않고 같은 `BP_CFVehiclePawn`에 다른 `VehicleData`만 연결해서 검증한다.

---

## 10. 검증 방법

### 빌드 검증
- 명령:
  - `D:\Work\CarFight_git\Tools\BuildEditor.bat`
- 예상 결과:
  - `CFVehicleData.h` 신규 USTRUCT / UPROPERTY가 정상 반영된다.
  - `CFVehiclePawn.cpp`의 레이아웃 적용 코드가 컴파일된다.

### 에디터 검증
- 실행:
  - `D:\Work\CarFight_git\Tools\RunEditor.bat`
- 확인 절차:
  1. `DA_TestSedan`에서 `VehicleLayoutConfig` 값이 보이는지 확인한다.
  2. `BP_CFVehiclePawn`을 열고 컴파일(`Compile`)한다.
  3. `VehicleData = DA_TestSedan` 상태로 뷰포트에서 바퀴 위치가 기존과 같은지 확인한다.
  4. `bUseLayoutOverrides`를 잠시 끄면 기존 BP 수동 배치 fallback으로 돌아가는지 확인한다.
  5. 다시 켠 뒤 PIE를 실행한다.

### 런타임 검증
- 체크포인트:
  - `WheelSync = Ready`
  - `RuntimeReady = True`
  - 조향 시 앞바퀴 피벗이 기존 수동 배치 기준과 동일
  - 전진/후진 시 휠 스핀 시각이 기존 PASS 기준에서 후퇴하지 않음
  - `FrontWheelAdditionalOffset / RearWheelAdditionalOffset` 변경 없이도 시각 레이아웃만 DA로 이동됨

### AssetDump 검증
- 후속 덤프에서 확인할 항목:
  - `DA_TestSedan.VehicleLayoutConfig.bUseLayoutOverrides = true`
  - 네 `WheelAnchor*` 값이 표와 일치
  - `BP_CFVehiclePawn`에 새 레이아웃 그래프 로직이 추가되지 않음

---

## 11. 실패 체크포인트

- `bUseLayoutOverrides=false`인데 레이아웃이 바뀐다.
  - 실패: fallback 정책 위반
- `WheelSync` 준비 전에 레이아웃이 적용되지 않는다.
  - 실패: `CaptureBaseWheelVisualState()`가 BP 수동 좌표를 계속 캡처함
- Owner 표시 안정화 이후 앵커 상대 위치가 예상과 다르다.
  - 실패: 런타임 재적용 순서가 잘못됨
- `Wheel_Anchor_*` 이름이 바뀐다.
  - 실패: 표준 이름 규약 위반
- `WheelSetup.AdditionalOffset`를 시각 앵커 좌표처럼 사용한다.
  - 실패: 물리 휠 오프셋과 시각 기준점의 책임 혼동
- 차종별 BP 자식을 만들어 레이아웃을 분기한다.
  - 실패: Thin BP / DataAsset 중심 목표 위반
- 기존 DataAsset을 열자마자 모든 차량 바퀴가 원점으로 이동한다.
  - 실패: 신규 필드 기본값 또는 적용 게이트 설계 오류

---

## 12. 승인 후 작업 순서

1. `CFVehicleData.h`에 `FCFWheelAnchorPose`, `FCFVehicleLayoutConfig`, `VehicleLayoutConfig` 추가
2. `CFVehiclePawn.cpp`의 `ApplyVehicleLayoutConfig()` 구현
3. `InitializeVehicleRuntime()`에서 `PrepareWheelSync()` 직전 레이아웃 재적용 순서 추가
4. `Tools\BuildEditor.bat`로 빌드 검증
5. 에디터에서 `DA_TestSedan` 레이아웃 값 입력
6. `BP_CFVehiclePawn` + `DA_TestSedan` 기준 PIE 검증
7. 검증 후 세단/SUV DataAsset에도 같은 구조 적용

---

## 13. Changelog

### v0.2 - 2026-06-25

- `DA_PoliceCar` 폐기 결정에 맞춰 문서를 Superseded 레거시 참고로 표시했다.
- 실행 단계의 기준 DA를 `DA_TestSedan`으로 변경했다.

### v0.1
- `BP_CFVehiclePawn`의 수동 `Wheel_Anchor_*` 배치를 `UCFVehicleData` 기반으로 옮기기 위한 최소 계획을 추가했다.
- 기존 AutoFit / 레거시 `WheelLayout` 복구가 아니라, 네 바퀴 앵커의 명시적 위치/회전 오버라이드만 다루는 범위로 제한했다.
- `WheelSync` 기준 캡처 전에 레이아웃을 적용해야 한다는 초기화 순서 요구사항을 명시했다.
- `DA_PoliceCar`로 옮길 시작 좌표를 BP 덤프 기준으로 기록했다.

## 14. Migration 메모

- 기존 BP 좌표는 첫 단계에서 삭제하지 않는다.
- `bUseLayoutOverrides` 기본값은 `false`로 유지해 기존 DataAsset의 즉시 동작 변화를 막는다.
- 실제 이관은 현재 기준 `DA_TestSedan`에서 `bUseLayoutOverrides=true`를 켜고 네 좌표를 입력하는 방식으로 진행한다.
- 다차종 확장은 BP 복제가 아니라 차량별 `UCFVehicleData` 인스턴스 추가로 진행한다.
- 물리 휠 `AdditionalOffset`은 이번 마이그레이션 대상이 아니며, 기존 `VehicleMovementConfig` 책임으로 유지한다.
