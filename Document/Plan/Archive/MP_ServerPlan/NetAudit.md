# 현재 구현 기능 네트워크 책임 분류 및 점검 기록

- 문서 버전: 0.2.0
- 작성일: 2026-05-22
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`
- 범위 기준 문서: `Scope.md`
- 기능 목록 문서: `FeatureList.md`
- 현재 작업: `T-002 현재 구현 기능의 네트워크 책임 분류`

---

## 1. 목적

이 문서는 현재 구현된 CarFight 기능을 Dedicated Server 멀티플레이 기준으로 분류한다.

현재 단계의 목표는 신규 시스템 구현이 아니다.

현재 단계의 목표는 아래와 같다.

```text
현재 구현된 기능이 서버 / 소유 클라이언트 / 비소유 클라이언트 / 연출 책임을 제대로 나누고 있는지 확인한다.
```

---

## 2. 이번 점검 기준

이번 문서는 아래 확인 결과를 기준으로 작성했다.

```text
- FeatureList.md 0.2.0
- UE/Source/CarFight_Re C++ 검색
- UE/Content/CarFight 에셋 목록
- UE/Content/Maps/TestMap.umap 맵 덤프
- BP_CFVehiclePawn 상세 덤프 요약
```

이번 점검에서 실제로 검색한 위험 패턴은 아래와 같다.

```text
- GetPlayerController
- GetFirstPlayerController
- CreateWidget
- AddToViewport
- HasAuthority
- IsLocallyControlled
- SpawnActor
- PlaySound
- Niagara
- SpawnEmitter
- bReplicates
- SetReplicates
- ReplicateMovement
- DOREPLIFETIME
- UFUNCTION(Server
- UFUNCTION(Client
- NetMulticast
- NM_DedicatedServer
- SetIsReplicatedByDefault
```

---

## 3. 네트워크 책임 분류 기준

### 3.1 서버 책임

```text
서버는 실제 게임 상태를 확정한다.
```

서버 책임 후보:

```text
- Pawn 생성
- PlayerController Possess
- 차량 이동 최종 권한
- 발사 요청 검증
- 공개 복제 상태 갱신
- 향후 체력/대미지/스코어/리스폰
```

현재 단계에서는 체력/대미지/스코어/리스폰은 구현하지 않는다.

---

### 3.2 소유 클라이언트 책임

```text
소유 클라이언트는 자기 입력, 자기 카메라, 자기 HUD를 처리한다.
```

소유 클라이언트 책임 후보:

```text
- Enhanced Input 수신
- Input Mapping Context 등록
- 카메라 Look 입력
- Aim Reticle 생성/표시
- Vehicle Debug HUD / Panel 표시
- 발사 요청 시작
```

---

### 3.3 비소유 클라이언트 책임

```text
비소유 클라이언트는 다른 플레이어 차량과 공개 상태를 관찰한다.
```

비소유 클라이언트 책임 후보:

```text
- 다른 차량 위치/회전 관찰
- 다른 차량 휠 시각 관찰
- 다른 차량 공개 Aim 시각 상태 관찰
```

---

### 3.4 Cosmetic 책임

```text
Cosmetic은 실제 판정을 바꾸지 않는다.
```

Cosmetic 후보:

```text
- Reticle UI
- Debug HUD / Panel
- 휠 시각 Transform
- 카메라 Transform / FOV
- 디버그 라인
- 온스크린 디버그 메시지
```

Dedicated Server에서는 Cosmetic을 실행하지 않거나, 실행돼도 오류가 없어야 한다.

---

## 4. 공통 위험 패턴 검색 결과

### 4.1 PlayerController 직접 접근

검색 결과:

```text
GetPlayerController: 0건
GetFirstPlayerController: 0건
```

판정:

```text
양호
```

메모:

```text
C++에서는 전역 PlayerController 0번 접근이 발견되지 않았다.
ACFVehiclePawn은 GetController()를 Cast<APlayerController>() 하는 방식으로 소유 컨트롤러를 찾는다.
```

---

### 4.2 UI 생성

검색 결과:

```text
CreateWidget: 11건
AddToViewport: 1건
```

발견 위치:

```text
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- CreateAimReticleWidget
- AddToViewport

UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
- DynamicSectionWidget 생성
- NavigationItemWidget 생성
- SelectedSectionWidget 생성

UE/Source/CarFight_Re/Private/UI/CFVehicleDebugSectionWidget.cpp
- FieldRowWidget 생성
- ChildSectionWidget 생성
```

현재 확인된 안전장치:

```text
ACFVehiclePawn::ShouldShowAimReticle()
- GetNetMode() != NM_DedicatedServer
- IsLocallyControlled()
```

판정:

```text
부분 양호 / 추가 확인 필요
```

메모:

```text
Aim Reticle은 Dedicated Server와 비소유 Pawn에서 생성되지 않도록 조건이 있다.
Debug Panel 내부의 동적 위젯 생성은 이미 생성된 UserWidget 내부 흐름이므로 서버에서 루트 Debug UI가 생성되지 않는다면 위험이 낮다.
다만 루트 Debug HUD / Panel 생성 위치는 Blueprint 또는 BP_CFVehiclePawn 상세에서 추가 확인이 필요하다.
```

---

### 4.3 카메라

검색/구조 결과:

```text
UCFVehicleCameraComp 존재
ACFVehiclePawn이 VehicleCameraComp를 기본 서브오브젝트로 생성
HandleLookInput에서 VehicleCameraComp에 Look 입력 전달
```

현재 책임 분류:

```text
카메라 계산 / FOV / SpringArm / FollowCamera: OwnerOnly, Cosmetic
Aim Trace 계산: OwnerOnly 후보, 서버 검증용 데이터와 분리 필요
```

판정:

```text
주의 필요
```

메모:

```text
VehicleCameraComp는 모든 ACFVehiclePawn에 기본 생성된다.
Dedicated Server에서도 컴포넌트 자체는 존재할 수 있다.
카메라 참조 검색, SpringArm/Camera 접근, Tick 갱신이 Dedicated Server에서 문제를 만들지 않는지 T-003에서 추가 점검한다.
```

---

### 4.4 사운드 / 이펙트

검색 결과:

```text
PlaySound: 0건
Niagara: 0건
SpawnEmitter: 0건
```

판정:

```text
현재 C++ 기준 위험 낮음
```

메모:

```text
현재 C++에서는 로컬 사운드/이펙트 호출이 발견되지 않았다.
Blueprint 내부에 있을 수 있으므로 BP 덤프 또는 수동 확인은 추후 필요하다.
```

---

### 4.5 Actor Spawn

검색 결과:

```text
SpawnActor: 0건
```

판정:

```text
현재 C++ 기준 SpawnActor 위험 없음
```

메모:

```text
현재 C++에서는 게임플레이 Actor를 Spawn하는 코드가 발견되지 않았다.
TestMap에는 BP_CFVehiclePawn_C_1이 배치되어 있다.
따라서 현재 위험은 SpawnActor보다 배치 차량 소유권 / AutoPossess 구조에 있다.
```

---

### 4.6 복제 설정

검색 결과:

```text
bReplicates: 0건
SetReplicates: 0건
ReplicateMovement: 0건
DOREPLIFETIME: 1건
SetIsReplicatedByDefault: 1건
```

발견 위치:

```text
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
- SetIsReplicatedByDefault(true)
- DOREPLIFETIME(UCFVehicleAimComp, RepAimVisualState)
```

판정:

```text
위험 높음
```

메모:

```text
ACFVehiclePawn 생성자에서 bReplicates 또는 SetReplicates 호출이 확인되지 않았다.
BP_CFVehiclePawn 상세 덤프 요약에서도 Replicates 여부는 아직 명확히 확인되지 않았다.
멀티플레이에서 차량 위치/회전이 보이려면 Pawn/Vehicle 복제와 Movement 복제 전략을 반드시 확인해야 한다.
```

---

### 4.7 RPC 사용

검색 결과:

```text
UFUNCTION(Server): 1건
UFUNCTION(Client): 1건
NetMulticast: 0건
```

발견 위치:

```text
ACFVehiclePawn::ServerRequestFire
ACFVehiclePawn::ClientReceiveFireResult
```

현재 흐름:

```text
HandleFireStarted
-> BuildFireRequest
-> 서버 권한이면 ServerRequestFire_Implementation 직접 호출
-> 클라이언트면 ServerRequestFire RPC 호출

ServerRequestFire_Implementation
-> ValidateFireRequestOnServer
-> VehicleAimComp ServerAimState 갱신
-> RepAimVisualState 갱신
-> ClientReceiveFireResult 호출
```

판정:

```text
부분 구현 / 검증 필요
```

메모:

```text
현재 RPC는 발사 요청 골격에만 존재한다.
이 단계에서는 무기 시스템을 확장하지 않고, Dedicated Server에서 해당 RPC가 오류 없이 동작하는지와 악용 가능한 클라이언트 권한 확정이 없는지만 확인한다.
```

---

## 5. 기능별 네트워크 책임 분류

### 5.1 ACFVehiclePawn

현재 확인:

```text
- PrimaryActorTick.bCanEverTick = true
- VehicleDriveComp 생성
- WheelSyncComp 생성
- VehicleCameraComp 생성
- VehicleAimComp 생성
- AutoPossessPlayer = EAutoReceiveInput::Player0
- BeginPlay에서 Input Mapping 등록 시도
- BeginPlay에서 Runtime 초기화
- BeginPlay 말미에서 CreateAimReticleWidget 호출
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| Pawn 존재 / 소유 | ServerAuth | 커스텀 GameMode 미확인, 맵 배치 차량 존재 | 위험 |
| 입력 바인딩 | OwnerOnly | SetupPlayerInputComponent 사용 | 추가 확인 필요 |
| Input Mapping 등록 | OwnerOnly | GetController -> LocalPlayer 사용 | 부분 양호 |
| Runtime 초기화 | Server + Client 혼합 | Drive/Wheel/Camera/Aim 초기화가 한 흐름에 묶임 | 주의 |
| Reticle 생성 | OwnerOnly | NM_DedicatedServer, IsLocallyControlled 검사 있음 | 양호 |
| AutoPossessPlayer | 싱글 전용 위험 | Player0 고정 | 위험 높음 |

결론:

```text
ACFVehiclePawn은 현재 안정화의 핵심 위험 지점이다.
특히 AutoPossessPlayer = Player0, 복제 설정 부재, GameMode/Possess 미확정이 우선 점검 대상이다.
```

---

### 5.2 UCFVehicleDriveComp

현재 확인:

```text
- ChaosWheeledVehicleMovementComponent 캐시
- Throttle / Steering / Brake / Handbrake 입력 적용
- DriveState 계산
- DriveState Snapshot 제공
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| 실제 차량 이동 입력 적용 | ServerAuth 후보 / Owner Input 후보 | 현재는 입력 함수에서 직접 VehicleMovement에 적용 | 위험 |
| DriveState 계산 | ServerAuth 또는 관찰용 Local 계산 | 현재 로컬 계산 구조 | 추가 결정 필요 |
| Debug Snapshot | OwnerOnly / Cosmetic | UI용 Snapshot 제공 | 양호 |

결론:

```text
Dedicated Server에서 차량 물리가 서버 권한으로 굴러갈지, 소유 클라이언트 예측/복제에 맡길지 결정해야 한다.
현재 단계에서는 새 구조를 만들지 않고, DS 2클라 환경에서 실제 동작을 먼저 확인한다.
```

---

### 5.3 UCFWheelSyncComp

현재 확인:

```text
- 휠 Anchor / Wheel Mesh 캐시
- 휠 시각 Transform 적용
- 조향/서스펜션/스핀 시각 계산
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| 휠 시각 갱신 | Cosmetic | Tick에서 UpdateVehicleWheelVisuals 호출 | 서버 실행 여부 확인 필요 |
| Wheel Movement 값 참조 | Observed State | Chaos Movement에서 값 읽음 | 주의 |
| Debug Helper Compare | Cosmetic / Deferred | 개발용 | 낮음 |

결론:

```text
WheelSyncComp는 게임 판정보다는 시각 동기화에 가깝다.
Dedicated Server에서는 실행을 생략하거나, 실행돼도 Mesh/SceneComponent 접근 오류가 없어야 한다.
```

---

### 5.4 UCFVehicleCameraComp

현재 확인:

```text
- CameraPivotRoot / CameraAimPivot / CameraBoom / FollowCamera 참조
- Look 입력 처리
- Aim Profile Override
- Aim Trace 계산
- CameraRuntimeState 제공
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| 카메라 Transform / FOV | OwnerOnly, Cosmetic | VehicleCameraComp Tick에서 처리 | 서버 실행 여부 확인 필요 |
| Look 입력 | OwnerOnly | HandleLookInput에서 전달 | 추가 확인 필요 |
| Aim Trace | OwnerOnly 후보 / Server 검증 후보 | 카메라 기준 계산 | 분리 필요 가능성 |

결론:

```text
카메라 자체는 소유 클라이언트 전용이어야 한다.
서버 판정에 필요한 Aim 값은 카메라 컴포넌트에 직접 의존하기보다 요청 데이터로 분리해야 한다.
현재 FireRequest 골격은 이 방향의 초안으로 보인다.
```

---

### 5.5 UCFVehicleAimComp

현재 확인:

```text
- SetIsReplicatedByDefault(true)
- RepAimVisualState 복제
- LocalAimState / ServerAimState / RepAimVisualState 보유
- BuildFireRequest
- ApplyServerFireResult
- UpdateRepAimVisualFromFireResult
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| LocalAimState | OwnerOnly | 로컬 Aim 표시용 | 적절 |
| ServerAimState | ServerAuth | 서버 발사 검증 상태 | 적절 후보 |
| RepAimVisualState | Replicate / Cosmetic | DOREPLIFETIME 확인 | 양호 |
| FireRequest 생성 | OwnerOnly 요청 | 현재 Local Aim 기반 생성 | 검증 필요 |
| FireResult 적용 | ServerAuth + OwnerOnly 피드백 | ClientReceiveFireResult 존재 | 부분 양호 |

결론:

```text
AimComp는 현재 코드에서 가장 멀티플레이를 의식한 구조다.
다만 Owner Actor인 ACFVehiclePawn 자체의 복제/소유권이 정리되어야 AimComp 복제도 의미가 있다.
```

---

### 5.6 Aim Reticle UI

현재 확인:

```text
- UCFAimReticleWidget 존재
- WBP_AimReticle 존재
- ACFVehiclePawn::ShouldShowAimReticle에서 Dedicated Server와 비소유 Pawn 차단
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| Reticle 생성 | OwnerOnly | ShouldShowAimReticle 검사 있음 | 양호 |
| Reticle 상태 갱신 | OwnerOnly / Cosmetic | VehicleAimComp 읽음 | 양호 |
| Viewport 추가 | OwnerOnly | AddToViewport 1건 | 조건부 양호 |

결론:

```text
Reticle은 현재 C++ 기준으로 Dedicated Server 방어가 들어가 있다.
DS 로그에서 실제 오류가 없는지 확인하면 된다.
```

---

### 5.7 Vehicle Debug UI

현재 확인:

```text
- UCFVehicleDebugHudWidget
- UCFVehicleDebugPanelWidget
- UCFVehicleDebugFieldRowWidget
- UCFVehicleDebugNavItemWidget
- UCFVehicleDebugSectionWidget
- WBP_VehicleDebug* 에셋 존재
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| Debug HUD 표시 | OwnerOnly, Cosmetic, Deferred | 구현됨 | 생성 위치 확인 필요 |
| Debug Panel 표시 | OwnerOnly, Cosmetic, Deferred | 구현됨 | 생성 위치 확인 필요 |
| 동적 Row/Section 생성 | Cosmetic | UserWidget 내부 CreateWidget | 루트 생성 조건에 종속 |
| Panel Interaction Mode | OwnerOnly | PlayerController 입력 모드 조작 후보 | 서버 차단 확인 필요 |

결론:

```text
Debug UI는 개발 편의 기능으로 유지하되, 서버에서 생성되면 안 된다.
루트 위젯 생성 위치가 Blueprint 쪽에 있을 가능성이 있어 T-003에서 BP 그래프 또는 상세 덤프를 추가 확인한다.
```

---

### 5.8 BP_CFVehiclePawn

현재 확인:

```text
- /Game/CarFight/Vehicles/BP_CFVehiclePawn 존재
- 컴포넌트 19개 확인
- CameraAimPivot, CameraBoom, CameraPivotRoot, FollowCamera, SM_Body 등 확인
- 클래스 기본값 166개 확인
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| 차량 조립 | Server + Client | C++ Pawn 기반 Blueprint | 확인 필요 |
| 카메라 컴포넌트 구성 | OwnerOnly, Cosmetic | 컴포넌트 존재 | 양호 후보 |
| Mesh 구성 | Replicate 관찰 / Cosmetic | 컴포넌트 존재 | 확인 필요 |
| Replicates 설정 | Replicate | 아직 명확히 확인 못함 | 위험 |

결론:

```text
BP_CFVehiclePawn의 Class Defaults에서 Replicates / Replicate Movement / Auto Possess 관련 값을 명확히 확인해야 한다.
현재 C++에는 복제 설정이 없으므로 Blueprint 설정에 의존 중일 수 있다.
```

---

### 5.9 TestMap

현재 확인:

```text
- /Game/Maps/TestMap.TestMap
- Actor 14개
- PlayerStart_0 1개
- BP_CFVehiclePawn_C_1 1대 배치
```

책임 분류:

| 항목 | 책임 | 현재 상태 | 판정 |
|---|---|---|---|
| PlayerStart | ServerAuth | 1개만 확인 | 멀티 테스트 부족 |
| 배치 차량 | ServerAuth / Replicate | 1대만 확인 | 위험 |
| GameMode Override | ServerAuth | 아직 미확인 | 확인 필요 |

결론:

```text
현재 TestMap은 싱글플레이 테스트 맵 성격이 강하다.
Dedicated Server 2클라 테스트를 위해 PlayerStart 추가, 차량 Spawn/Possess 구조 확인이 필요하다.
```

---

## 6. 현재 발견 이슈

### NA-001 AutoPossessPlayer Player0 고정

```text
이슈 ID: NA-001
관련 기능: ACFVehiclePawn / BP_CFVehiclePawn / TestMap
발견 위치: ACFVehiclePawn 생성자
문제 설명: AutoPossessPlayer = EAutoReceiveInput::Player0 로 고정되어 있다.
서버/클라이언트 영향: Dedicated Server 멀티플레이에서 배치 차량 1대가 싱글플레이 방식으로 소유될 수 있다.
수정 우선순위: 높음
수정 방안: Dedicated Server 기준 GameMode 또는 PlayerController에서 서버가 차량 Spawn/Possess하도록 분리한다. 현재 단계에서는 바로 구현하지 않고 T-006에서 확인 후 수정한다.
완료 기준: 클라이언트 2개가 각각 자기 Pawn 또는 차량만 조작한다.
상태: 확인 필요
```

---

### NA-002 ACFVehiclePawn 복제 설정 미확인

```text
이슈 ID: NA-002
관련 기능: 차량 Pawn / 차량 이동
발견 위치: ACFVehiclePawn C++ 검색 결과
문제 설명: C++에서 bReplicates, SetReplicates, ReplicateMovement 설정이 발견되지 않았다.
서버/클라이언트 영향: 다른 클라이언트에서 차량 위치/회전이 보이지 않을 수 있다.
수정 우선순위: 높음
수정 방안: BP_CFVehiclePawn Class Defaults와 C++ 생성자에서 Replicates / Movement Replication 설정 여부를 확인한다.
완료 기준: 클라이언트 2개 접속 시 서로의 차량 위치/회전이 보인다.
상태: 확인 필요
```

---

### NA-003 차량 입력이 권한 구조 없이 Movement에 직접 적용됨

```text
이슈 ID: NA-003
관련 기능: 차량 입력 / DriveComp
발견 위치: ACFVehiclePawn::SetVehicleThrottleInput, SetVehicleSteeringInput, SetVehicleBrakeInput, SetVehicleHandbrakeInput
문제 설명: 입력 핸들러가 DriveComp를 통해 Chaos Vehicle Movement에 직접 값을 적용한다.
서버/클라이언트 영향: Dedicated Server에서 소유 클라이언트 입력이 서버 물리로 어떻게 전달되는지 불명확하다.
수정 우선순위: 높음
수정 방안: 먼저 DS 2클라에서 실제 동작 확인. 필요하면 입력 요청과 서버 권한 적용 흐름으로 분리한다.
완료 기준: 자기 입력은 자기 차량에만 적용되고, 다른 클라이언트 차량에는 적용되지 않는다.
상태: 확인 필요
```

---

### NA-004 Runtime 초기화에 서버/클라이언트/Cosmetic 책임이 섞여 있음

```text
이슈 ID: NA-004
관련 기능: ACFVehiclePawn BeginPlay / InitializeVehicleRuntime
발견 위치: ACFVehiclePawn::BeginPlay, InitializeVehicleRuntime
문제 설명: Drive, WheelSync, Aim 초기화가 한 흐름에 묶여 있고, 카메라/시각/UI 계열과 게임플레이 계열의 책임 경계가 명확하지 않다.
서버/클라이언트 영향: Dedicated Server에서 불필요한 카메라/휠 시각/위젯 의존 코드가 실행될 수 있다.
수정 우선순위: 중간
수정 방안: T-003에서 위험 패턴을 더 좁히고, 서버 전용 초기화 / 소유 클라이언트 초기화 / Cosmetic 초기화로 분리할 후보를 정한다.
완료 기준: Dedicated Server 로그에 UI/카메라/시각 컴포넌트 관련 오류가 없다.
상태: 확인 필요
```

---

### NA-005 Debug UI 루트 생성 위치 미확인

```text
이슈 ID: NA-005
관련 기능: VehicleDebug HUD / Panel
발견 위치: C++ 검색 결과 CreateWidget은 UI 내부 동적 생성 중심으로 확인됨
문제 설명: WBP_VehicleDebugHud / WBP_VehicleDebugPanel 루트 생성 위치가 아직 명확하지 않다.
서버/클라이언트 영향: Dedicated Server에서 Debug UI 루트가 생성되면 오류가 발생할 수 있다.
수정 우선순위: 중간
수정 방안: BP_CFVehiclePawn 그래프 또는 위젯 생성 경로를 추가 덤프한다.
완료 기준: Debug UI 루트는 소유 클라이언트에서만 생성된다.
상태: 확인 필요
```

---

### NA-006 TestMap 멀티플레이 준비 부족

```text
이슈 ID: NA-006
관련 기능: TestMap / Spawn / Possess
발견 위치: TestMap 덤프
문제 설명: PlayerStart 1개, 배치 차량 1대만 확인됐다.
서버/클라이언트 영향: 클라이언트 2개 접속 시 Spawn 위치와 차량 소유가 충돌할 가능성이 있다.
수정 우선순위: 높음
수정 방안: DS 최소 실행선 테스트용 맵 또는 현재 TestMap을 멀티 테스트 가능 상태로 정리한다.
완료 기준: 클라이언트 2개가 충돌 없이 접속하고 각자 Pawn 또는 차량을 가진다.
상태: 확인 필요
```

---

### NA-007 AimComp 복제는 있으나 Owner Pawn 복제가 선행 조건임

```text
이슈 ID: NA-007
관련 기능: UCFVehicleAimComp / RepAimVisualState
발견 위치: UCFVehicleAimComp::GetLifetimeReplicatedProps
문제 설명: AimComp는 SetIsReplicatedByDefault와 DOREPLIFETIME이 있으나, 소유 Actor인 ACFVehiclePawn 복제 설정이 불명확하다.
서버/클라이언트 영향: Owner Actor 복제가 안 되면 컴포넌트 복제도 기대대로 동작하지 않을 수 있다.
수정 우선순위: 중간
수정 방안: Pawn 복제 설정 확인 후 AimComp 복제 확인으로 넘어간다.
완료 기준: 서버 발사 결과의 RepAimVisualState가 비소유 클라이언트에도 관찰 가능하다.
상태: 확인 필요
```

---

## 7. 책임 분리 우선순위

현재 안정화 단계의 우선순위는 아래 순서다.

```text
1. Pawn 복제 설정 확인
2. AutoPossessPlayer Player0 제거 또는 DS 전용 Possess 구조 결정
3. TestMap의 PlayerStart / 배치 차량 구조 확인
4. 입력이 소유 차량에만 적용되는지 확인
5. 차량 위치/회전이 다른 클라이언트에 보이는지 확인
6. 카메라 / Reticle / Debug UI가 Dedicated Server에서 실행되지 않는지 확인
7. AimComp 복제 상태는 Pawn 복제 확인 이후 점검
```

---

## 8. 현재 안정화 완료 조건

`T-002` 기준 완료 조건:

```text
- 현재 구현 기능의 서버/클라이언트 책임 분류 작성 완료
- 주요 C++ 위험 패턴 검색 완료
- 발견 이슈를 NA-* 형식으로 기록 완료
- 다음 작업 T-003에서 실제 위험 패턴을 더 좁힐 수 있는 상태
```

현재 상태:

```text
T-002 완료
```

---

## 9. 다음 작업

다음 작업은 `T-003 위험 패턴 검색`이다.

다음 작업에서 확인할 항목:

```text
- BP_CFVehiclePawn Class Defaults의 Replicates / Replicate Movement / Auto Possess 값 확인
- BP_CFVehiclePawn Event Graph에서 CreateWidget / AddToViewport / Debug UI 생성 위치 확인
- VehicleCameraComp Tick이 Dedicated Server에서 안전한지 확인
- WheelSyncComp Tick이 Dedicated Server에서 안전한지 확인
- ServerRequestFire 검증 흐름이 클라이언트 신뢰에 과도하게 의존하지 않는지 확인
- TestMap WorldSettings GameMode Override 확인
```

---

## 10. 변경 기록

### 0.2.0

```text
- FeatureList.md 0.2.0 기준으로 현재 구현 기능의 네트워크 책임 분류 작성
- C++ 위험 패턴 검색 결과 반영
- AutoPossessPlayer Player0 위험 기록
- Pawn 복제 설정 미확인 위험 기록
- 차량 입력 직접 적용 위험 기록
- Debug UI 생성 위치 미확인 위험 기록
- TestMap 멀티플레이 준비 부족 위험 기록
- AimComp 복제 선행 조건 기록
- 다음 작업을 T-003 위험 패턴 검색으로 지정
```

### 0.1.0

```text
- 네트워크 점검 체크리스트 최초 작성
- 공통 위험 패턴과 기능별 점검 항목 정의
- 현재 안정화 전 통과 게이트 정의
```
