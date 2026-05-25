# 현재 구현 기능 목록 및 네트워크 책임표

- 문서 버전: 0.2.0
- 작성일: 2026-05-22
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`
- 현재 범위 기준 문서: `Scope.md`
- 현재 작업: `T-001 실제 구현 기능 목록 작성`

---

## 1. 목적

이 문서는 CarFight에 현재 실제로 구현되어 있는 기능만 정리한다.

현재 단계의 목표는 신규 시스템 구현이 아니다.

현재 단계의 목표는 아래 하나다.

```text
현재 구현된 기능만 Dedicated Server 기준으로 안정화한다.
```

따라서 이 문서에서는 아직 없는 기능을 작업 대상으로 넣지 않는다.

---

## 2. 확인 기준

이번 확인은 아래 자료를 기준으로 했다.

```text
- UE/Source/CarFight_Re/ C++ 클래스 목록
- UE/Content/CarFight/ 프로젝트 전용 에셋 목록
- UE/Content/Maps/TestMap.umap 맵 덤프
- UE/Config/DefaultEngine.ini
- UE/Config/DefaultInput.ini
- /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn 상세 덤프
```

주의:

```text
ue.list_assets는 브리지 타임아웃으로 실패했다.
대신 repo.list 기반 파일 목록과 개별 맵/에셋 덤프로 확인했다.
```

---

## 3. 기능 분류 태그

```text
[ServerAuth]
서버 권한이 필요한 기능이다.
예: Pawn 생성, Possess, 실제 상태 변경, 판정

[Replicate]
클라이언트에게 복제되어야 하는 기능이다.
예: 차량 Pawn, 차량 위치/회전, 공개 Aim 시각 상태

[OwnerOnly]
소유자 클라이언트에게만 필요한 기능이다.
예: 입력, 카메라, HUD, 조준선, 개인 피드백

[Cosmetic]
게임 판정에는 영향이 없는 연출 기능이다.
예: UI, 카메라 표시, 디버그 라인, 시각 효과

[Data]
런타임 설정을 공급하는 데이터 기능이다.
예: VehicleData, CameraData, Wheel/DataAsset

[Partial]
일부 골격은 있지만 완성된 게임플레이 시스템은 아닌 기능이다.

[Missing]
현재 프로젝트에서 구현을 확인하지 못한 기능이다.

[Deferred]
현재 안정화 범위에서는 다루지 않는 기능이다.

[BeforeDSStable]
Dedicated Server 최소 실행선 전에 확인해야 하는 기능이다.
```

---

## 4. 현재 C++ 클래스 목록

### 4.1 Target / Module

| 구분 | 파일 | 현재 상태 | 메모 |
|---|---|---|---|
| Game Target | `UE/Source/CarFight_Re.Target.cs` | 존재 | 일반 게임 Target |
| Editor Target | `UE/Source/CarFight_ReEditor.Target.cs` | 존재 | 에디터 Target |
| Server Target | `UE/Source/CarFight_ReServer.Target.cs` | 없음 | `T-004 Dedicated Server Target 확인`에서 처리 필요 |
| Module | `UE/Source/CarFight_Re/CarFight_Re.Build.cs` | 존재 | EnhancedInput, UMG, Slate, ChaosVehicles, PhysicsCore 사용 |

---

### 4.2 차량 / 주행 / 데이터 클래스

| 클래스 | 파일 | 현재 상태 | 역할 |
|---|---|---|---|
| `ACFVehiclePawn` | `CFVehiclePawn.h/.cpp` | 구현됨 | 신규 차량 Pawn 기준 클래스. 입력, DriveComp, WheelSyncComp, VehicleCameraComp, VehicleAimComp, Debug Snapshot, Reticle 위젯 생성 흐름 보유 |
| `ACFModVehiclePawn` | `CFModVehiclePawn.h/.cpp` | 존재 | 별도/레거시 차량 Pawn 후보. 현재 실제 사용 여부는 추가 확인 필요 |
| `UCFVehicleDriveComp` | `CFVehicleDriveComp.h/.cpp` | 구현됨 | Throttle, Steering, Brake, Handbrake 입력을 Chaos Vehicle Movement에 전달하고 Drive 상태를 계산 |
| `UCFWheelSyncComp` | `CFWheelSyncComp.h/.cpp` | 구현됨 | 휠 Anchor/Mesh 캐시, 휠 조향/스핀/서스펜션 시각 동기화 |
| `UCFVehicleCameraComp` | `CFVehicleCameraComp.h/.cpp` | 구현됨 | 차량 중심 Pivot 기반 자유 조준, 카메라 Transform/FOV, Aim Trace 계산 |
| `UCFVehicleAimComp` | `CFVehicleAimComp.h/.cpp` | 부분 구현 | Local Aim, Server Aim, Rep Aim Visual 상태 보유. FireRequest/FireResult 처리 골격 포함 |
| `UCFVehicleData` | `CFVehicleData.h/.cpp` | 구현됨 | 차량 시각/이동/휠/DriveState 설정 DataAsset |
| `UCFVehicleCameraData` | `CFVehicleCameraData.h` | 구현됨 | 차량 카메라 튜닝값 DataAsset |
| `UCarFightVehicleUtils` | `CarFightVehicleUtils.h/.cpp` | 구현됨 | 차량 관련 Blueprint Function Library |

---

### 4.3 UI 클래스

| 클래스 | 파일 | 현재 상태 | 역할 |
|---|---|---|---|
| `UCFAimReticleWidget` | `UI/CFAimReticleWidget.h/.cpp` | 구현됨 | AimComp의 Reticle 상태를 UI로 표시 |
| `UCFVehicleDebugHudWidget` | `UI/CFVehicleDebugHudWidget.h/.cpp` | 구현됨 | VehicleDebug Overview HUD 표시 |
| `UCFVehicleDebugPanelWidget` | `UI/CFVehicleDebugPanelWidget.h/.cpp` | 구현됨 | VehicleDebug 상세 Panel 표시, Section 선택, 상호작용 모드 지원 |
| `UCFVehicleDebugFieldRowWidget` | `UI/CFVehicleDebugFieldRowWidget.h/.cpp` | 구현됨 | Debug Panel 필드 행 표시 |
| `UCFVehicleDebugNavItemWidget` | `UI/CFVehicleDebugNavItemWidget.h/.cpp` | 구현됨 | Debug Panel 네비게이션 항목 |
| `UCFVehicleDebugSectionWidget` | `UI/CFVehicleDebugSectionWidget.h/.cpp` | 구현됨 | Debug Panel 섹션 표시 |

---

## 5. 현재 Blueprint / Asset 목록

### 5.1 CarFight 차량 에셋

| 에셋 | 현재 상태 | 메모 |
|---|---|---|
| `/Game/CarFight/Vehicles/BP_CFVehiclePawn` | 존재 | 상세 덤프 기준 Blueprint. 컴포넌트 19개, 클래스 기본값 166개 확인 |
| `/Game/CarFight/Vehicles/BP_Wheel_Front` | 존재 | 전륜 Wheel Blueprint 후보 |
| `/Game/CarFight/Vehicles/BP_Wheel_Rear` | 존재 | 후륜 Wheel Blueprint 후보 |
| `/Game/CarFight/Vehicles/PDA_VehicleArchetype` | 존재 | 차량 Archetype 후보 |
| `/Game/CarFight/Vehicles/S_WheelConfig` | 존재 | Wheel 설정 구조/에셋 후보 |
| `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar` | 존재 | PoliceCar 차량 데이터 |
| `/Game/CarFight/Vehicles/Data/Camera/DA_Cam_Default` | 존재 | 기본 카메라 데이터 |
| `/Game/CarFight/Vehicles/police_car/...` | 존재 | PoliceCar Mesh/Material/Texture 자산 |

---

### 5.2 CarFight UI 에셋

| 에셋 | 현재 상태 | 메모 |
|---|---|---|
| `/Game/CarFight/UI/WBP_AimReticle` | 존재 | Aim Reticle UI |
| `/Game/CarFight/UI/WBP_VehicleDebugHud` | 존재 | 차량 디버그 HUD |
| `/Game/CarFight/UI/WBP_VehicleDebugPanel` | 존재 | 차량 디버그 상세 Panel |
| `/Game/CarFight/UI/WBP_VehicleDebugFieldRow` | 존재 | Field Row UI |
| `/Game/CarFight/UI/WBP_VehicleDebugNavItem` | 존재 | Navigation Item UI |
| `/Game/CarFight/UI/WBP_VehicleDebugSection` | 존재 | Section UI |

---

### 5.3 CarFight Input 에셋

| 에셋 | 현재 상태 | 메모 |
|---|---|---|
| `/Game/CarFight/Input/IMC_Vehicle_Default` | 존재 | 차량 기본 Input Mapping Context |
| `/Game/CarFight/Input/IA_VehicleMove` | 존재 | 2D 차량 이동 입력 |
| `/Game/CarFight/Input/IA_Throttle` | 존재 | 스로틀 입력 |
| `/Game/CarFight/Input/IA_Brake` | 존재 | 브레이크 입력 |
| `/Game/CarFight/Input/IA_Steering` | 존재 | 조향 입력 |
| `/Game/CarFight/Input/IA_Handbrake` | 존재 | 핸드브레이크 입력 |
| `/Game/CarFight/Input/IA_LookAround` | 존재 | 카메라 Look 입력 |
| `/Game/CarFight/Input/IA_ToggleCamera` | 존재 | 카메라 토글 입력 후보 |
| `/Game/CarFight/Input/IA_Headlights` | 존재 | 헤드라이트 입력 후보 |
| `/Game/CarFight/Input/IA_Reset` | 존재 | 리셋 입력 후보 |
| `/Game/CarFight/Input/IA_ShifterUp` | 존재 | 기어 상승 입력 후보 |
| `/Game/CarFight/Input/IA_ShifterDown` | 존재 | 기어 하강 입력 후보 |
| `/Game/CarFight/Input/IA_Fire` | 존재 | 발사 입력 액션. 현재는 무기 완성 기능이 아니라 Aim/FireRequest 골격과 연결된 후보로 분류 |

---

### 5.4 맵 / 테스트 환경

| 항목 | 현재 상태 | 메모 |
|---|---|---|
| 기본 맵 | `/Game/Maps/TestMap.TestMap` | `DefaultEngine.ini`에서 `GameDefaultMap`, `EditorStartupMap`으로 설정됨 |
| 맵 Actor 수 | 14개 | `ue.dump_map` 결과 기준 |
| PlayerStart | 1개 | `PlayerStart_0` 확인 |
| 배치 차량 | 1개 | `BP_CFVehiclePawn_C_1` 확인 |
| StaticMeshActor / Floor | 존재 | 테스트 맵 오브젝트로 확인 |

---

## 6. 현재 구현 기능 목록

| ID | 기능 | 현재 구현 상태 | 네트워크 분류 | DS 안정화 전 필요 | 메모 |
|---|---|---|---|---|---|
| F-001 | 차량 Pawn | 구현됨 | [ServerAuth], [Replicate], [OwnerOnly] | 예 | `ACFVehiclePawn`, `BP_CFVehiclePawn` 존재. 서버 Possess/Spawn 흐름 점검 필요 |
| F-002 | 차량 입력 | 구현됨 | [OwnerOnly], [BeforeDSStable] | 예 | Enhanced Input 기반. `IA_VehicleMove`, `Throttle`, `Brake`, `Steering`, `Handbrake`, `Look`, `Fire` 참조 존재 |
| F-003 | 차량 이동 / Chaos Vehicle Movement | 구현됨 | [Replicate], [BeforeDSStable] | 예 | 실제 멀티 이동 동기화는 DS에서 확인 필요 |
| F-004 | Drive 상태 계산 | 구현됨 | [OwnerOnly], [Replicate] 후보 | 예 | `UCFVehicleDriveComp`가 속도, 전후 속도, DriveState, 입력 상태 Snapshot 계산 |
| F-005 | 차량 데이터 적용 | 구현됨 | [Data] | 예 | `UCFVehicleData`, `DA_PoliceCar`, `DA_Cam_Default` 존재 |
| F-006 | 휠 시각 동기화 | 구현됨 | [Cosmetic], [OwnerOnly] 후보 | 예 | `UCFWheelSyncComp`가 휠 Anchor/Mesh 시각 갱신 담당. 서버에서 불필요한 시각 갱신 실행 여부 확인 필요 |
| F-007 | 차량 카메라 / 자유 조준 | 구현됨 | [OwnerOnly], [Cosmetic] | 예 | `UCFVehicleCameraComp`가 Pivot, SpringArm, FollowCamera, Aim Trace 계산 |
| F-008 | Aim 상태 / Reticle | 부분 구현 | [OwnerOnly], [Replicate], [Partial] | 예 | `UCFVehicleAimComp`, `UCFAimReticleWidget`, `WBP_AimReticle` 존재 |
| F-009 | 발사 요청 골격 | 부분 구현 | [ServerAuth], [OwnerOnly], [Partial] | 조건부 | `IA_Fire`, `FCFVehicleFireRequest`, `ServerRequestFire`, `ClientReceiveFireResult` 존재. 실제 무기/대미지/투사체는 없음 |
| F-010 | Debug HUD / Debug Panel | 구현됨 | [OwnerOnly], [Cosmetic], [Deferred] | 조건부 | HUD/Panel/Section/NavItem/FieldRow 구현됨. 서버에서 UI 생성 금지 확인 필요 |
| F-011 | 테스트 맵 | 구현됨 | [ServerAuth] 후보 | 예 | `TestMap`에 `PlayerStart_0`, `BP_CFVehiclePawn_C_1` 존재 |
| F-012 | 차량 스폰 / Possess | 확인 필요 | [ServerAuth], [BeforeDSStable] | 예 | 커스텀 GameMode 미확인. 맵에는 차량이 배치되어 있음 |
| F-013 | 커스텀 GameMode | 미확인 / 미구현 | [Missing] | 예 | Source와 Config에서 전용 GameMode 설정을 확인하지 못함 |
| F-014 | 커스텀 PlayerController | 미확인 / 미구현 | [Missing] | 예 | Source에서 전용 PlayerController 클래스를 확인하지 못함 |
| F-015 | Dedicated Server Target | 미구현 | [Missing] | 예 | `CarFight_ReServer.Target.cs` 없음. `T-004`에서 처리 필요 |
| F-016 | 부스터 | 미구현 | [Missing], [Deferred] | 아니오 | 현재 안정화 범위 제외 |
| F-017 | 드리프트 전용 시스템 | 미확인 / 미구현 | [Missing], [Deferred] | 아니오 | Drive 상태와 별도 드리프트 시스템은 확인되지 않음 |
| F-018 | 체력 / 대미지 / 스코어 / 리스폰 | 미구현 | [Missing], [Deferred] | 아니오 | 현재 안정화 범위 제외 |
| F-019 | 로비 / 세션 / 매치메이킹 | 미구현 | [Missing], [Deferred] | 아니오 | 현재 안정화 범위 제외 |

---

## 7. 현재 사용 흐름 추정

확인된 파일 기준 현재 기본 흐름은 아래로 추정한다.

```text
1. 프로젝트 기본 맵은 /Game/Maps/TestMap.TestMap 이다.
2. TestMap에는 PlayerStart 1개와 BP_CFVehiclePawn 1대가 배치되어 있다.
3. BP_CFVehiclePawn은 ACFVehiclePawn 기반 Blueprint로 보인다.
4. ACFVehiclePawn은 DriveComp / WheelSyncComp / VehicleCameraComp / VehicleAimComp를 보유한다.
5. 입력은 IMC_Vehicle_Default와 여러 IA_* 에셋을 통해 차량 Pawn에 연결되는 구조다.
6. 차량 이동, 카메라, Aim, Reticle, Debug UI는 이미 상당 부분 구현되어 있다.
7. Dedicated Server 기준 Spawn/Possess/GameMode/PlayerController 흐름은 아직 명확하지 않다.
```

확정되지 않은 부분:

```text
- TestMap의 WorldSettings GameMode Override 여부
- DefaultPawnClass 설정 여부
- PlayerControllerClass 설정 여부
- Dedicated Server에서 차량을 서버가 Spawn/Possess하는지 여부
- 배치 차량 1대를 멀티플레이에서 어떻게 소유할지 여부
```

---

## 8. 기능별 결정 기록

### F-001 차량 Pawn

```text
현재 상태:
- 구현됨
- ACFVehiclePawn C++ 클래스 존재
- BP_CFVehiclePawn Blueprint 존재
- TestMap에 BP_CFVehiclePawn_C_1 배치 확인

결정:
- 현재 안정화의 핵심 대상이다.

수정 필요 여부:
- Dedicated Server에서 Spawn/Possess 흐름 확인 후 판단한다.

완료 기준:
- 클라이언트 2개 접속 시 각 클라이언트가 자기 Pawn 또는 차량만 조작한다.
```

---

### F-002 차량 입력

```text
현재 상태:
- 구현됨
- IA_VehicleMove, IA_Throttle, IA_Brake, IA_Steering, IA_Handbrake, IA_LookAround, IA_Fire 존재
- ACFVehiclePawn에 Enhanced Input 바인딩 흐름 존재

결정:
- OwnerOnly 입력으로 취급한다.

수정 필요 여부:
- Dedicated Server에서 입력이 비소유 차량에 적용되지 않는지 확인 필요

완료 기준:
- 클라이언트 A 입력이 클라이언트 B 차량에 영향을 주지 않는다.
```

---

### F-003 차량 이동 / Drive 상태

```text
현재 상태:
- 구현됨
- UCFVehicleDriveComp가 Chaos Vehicle Movement에 입력을 전달한다.
- DriveState Snapshot과 Debug 문자열을 제공한다.

결정:
- 이동 복제 확인이 필요하다.

수정 필요 여부:
- DS 2클라 테스트 후 판단한다.

완료 기준:
- 두 클라이언트가 서로의 차량 위치/회전을 볼 수 있다.
```

---

### F-004 휠 시각 동기화

```text
현재 상태:
- 구현됨
- UCFWheelSyncComp가 휠 Anchor/Mesh를 찾아 시각 Transform을 적용한다.

결정:
- Cosmetic 기능으로 분류한다.

수정 필요 여부:
- Dedicated Server에서 불필요하게 실행되는지 확인 필요

완료 기준:
- 서버 로그에 시각 컴포넌트 의존 오류가 없다.
```

---

### F-005 차량 카메라 / Aim / Reticle

```text
현재 상태:
- 차량 카메라 구현됨
- AimComp 부분 구현됨
- Reticle UI 구현됨
- FireRequest/FireResult 골격 존재

결정:
- 카메라와 Reticle은 OwnerOnly / Cosmetic으로 취급한다.
- FireRequest 골격은 현재 안정화 대상이지만, 무기 시스템으로 확장하지 않는다.

수정 필요 여부:
- 서버에서 Camera/HUD/Viewport 의존 코드가 실행되는지 확인 필요

완료 기준:
- Dedicated Server에서 카메라와 UI 때문에 오류가 발생하지 않는다.
```

---

### F-006 Debug UI

```text
현재 상태:
- 구현됨
- WBP_VehicleDebugHud, WBP_VehicleDebugPanel 등 존재

결정:
- 현재 안정화 중에는 OwnerOnly 개발용 UI로 취급한다.

수정 필요 여부:
- 서버에서 CreateWidget 또는 Viewport 접근이 실행되는지 확인 필요

완료 기준:
- Dedicated Server 로그에 UI 생성 관련 오류가 없다.
```

---

### F-007 GameMode / PlayerController / Spawn 흐름

```text
현재 상태:
- 커스텀 GameMode 클래스 확인 안 됨
- 커스텀 PlayerController 클래스 확인 안 됨
- DefaultEngine.ini에는 기본 맵만 확인됨
- TestMap에는 PlayerStart 1개와 배치 차량 1대가 있음

결정:
- T-002/T-004/T-006에서 우선 점검해야 한다.

수정 필요 여부:
- 매우 가능성 높음

완료 기준:
- Dedicated Server에서 각 PlayerController가 올바른 Pawn 또는 차량을 Possess한다.
```

---

## 9. 현재 안정화 우선순위

```text
1. GameMode / PlayerController / DefaultPawnClass / 배치 차량 소유 구조 확인
2. BP_CFVehiclePawn의 Replicates / Movement Replication 설정 확인
3. ACFVehiclePawn의 입력이 OwnerOnly로 제한되는지 확인
4. VehicleCameraComp와 Reticle UI가 로컬 클라이언트에서만 실행되는지 확인
5. Debug HUD/Panel이 Dedicated Server에서 생성되지 않는지 확인
6. WheelSyncComp가 서버에서 불필요한 시각 갱신을 하지 않는지 확인
7. FireRequest 골격은 유지하되 무기/대미지/투사체 구현으로 확장하지 않음
```

---

## 10. 다음 갱신 작업

다음 문서 작업은 `T-002 현재 구현 기능의 네트워크 책임 분류`다.

작업 대상:

```text
- ACFVehiclePawn
- UCFVehicleDriveComp
- UCFWheelSyncComp
- UCFVehicleCameraComp
- UCFVehicleAimComp
- UCFAimReticleWidget
- UCFVehicleDebugHudWidget
- UCFVehicleDebugPanelWidget
- BP_CFVehiclePawn
- TestMap
```

갱신 대상 문서:

```text
NetAudit.md
```

---

## 11. 변경 기록

### 0.2.0

```text
- 실제 C++ 클래스 목록 반영
- 실제 CarFight Blueprint/Input/UI/Vehicle 에셋 목록 반영
- TestMap 덤프 결과 반영
- BP_CFVehiclePawn 상세 덤프 결과 반영
- 구현됨 / 부분 구현 / 미구현 기능을 분리
- 무기/체력/스코어/리스폰을 현재 안정화 범위에서 제외
- 다음 작업을 T-002 NetAudit으로 지정
```

### 0.1.0

```text
- 현재 구현 기능 목록 문서 최초 작성
- 기능 분류 태그 정의
- 현재 안정화 전 점검 항목 정의
```
