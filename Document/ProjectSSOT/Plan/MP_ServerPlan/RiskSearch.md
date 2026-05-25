# T-003 위험 패턴 상세 검색 결과

- 문서 버전: 0.1.0
- 작성일: 2026-05-22
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`
- 범위 기준 문서: `Scope.md`
- 관련 문서: `FeatureList.md`, `NetAudit.md`, `DSMinRun.md`
- 현재 작업: `T-003 위험 패턴 검색`

---

## 1. 목적

이 문서는 `T-003 위험 패턴 검색`의 상세 결과를 기록한다.

현재 단계에서는 코드를 작성하지 않는다.

목표는 다음과 같다.

```text
1. 현재 구현 기능에서 Dedicated Server 전환 시 위험한 부분을 더 좁힌다.
2. 코드 작성이나 에디터 설정 변경이 필요한 지점을 식별한다.
3. 실제 수정 단계로 넘어가기 전에 중단 지점을 명확히 기록한다.
```

---

## 2. 이번 검색 대상

이번 작업에서 확인한 대상은 다음과 같다.

```text
- BP_CFVehiclePawn Class Defaults
- BP_CFVehiclePawn EventGraph
- TestMap 맵 덤프
- 기존 C++ 위험 패턴 검색 결과
```

사용한 주요 확인 자료:

```text
- /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn
- /Game/Maps/TestMap.TestMap
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
```

---

## 3. BP_CFVehiclePawn Class Defaults 확인 결과

### 3.1 Actor 복제

확인 결과:

```text
class_defaults.bReplicates = true
```

판정:

```text
양호
```

메모:

```text
C++ 생성자에서는 bReplicates / SetReplicates 호출이 확인되지 않았지만,
BP_CFVehiclePawn Class Defaults에서는 Actor 복제가 켜져 있다.
```

---

### 3.2 Movement 복제

확인 결과:

```text
class_defaults.bReplicateMovement = true
```

판정:

```text
양호 후보
```

메모:

```text
BP Class Defaults 기준으로 Movement Replication은 켜져 있다.
다만 Chaos Vehicle 기반 이동이 Dedicated Server 2클라 환경에서 실제로 안정적으로 보이는지는 실행 테스트가 필요하다.
```

---

### 3.3 Auto Possess Player

확인 결과:

```text
class_defaults.AutoPossessPlayer = Player0
```

판정:

```text
위험 높음
```

메모:

```text
ACFVehiclePawn C++ 생성자에서도 AutoPossessPlayer = EAutoReceiveInput::Player0 로 확인됐고,
BP_CFVehiclePawn Class Defaults에서도 Player0가 유지되고 있다.

이 구조는 싱글플레이 테스트에는 편하지만 Dedicated Server 멀티플레이에서는 적합하지 않다.
서버가 각 PlayerController에 대해 Pawn 또는 차량을 명시적으로 Spawn/Possess하는 구조로 전환해야 할 가능성이 높다.
```

---

## 4. BP_CFVehiclePawn Component Replication 확인 결과

확인된 주요 컴포넌트 복제 상태는 다음과 같다.

| 컴포넌트 | bReplicates | 판정 |
|---|---:|---|
| `VehicleMovementComp` | true | 양호 후보 |
| `VehicleAimComp` | true | 양호 |
| `VehicleDriveComp` | false | 의도 확인 필요 |
| `WheelSyncComp` | false | Cosmetic이면 허용 가능 |
| `VehicleCameraComp` | false | OwnerOnly/Cosmetic이면 적절 |
| `SM_Body` | false | Actor/Movement 복제로 충분한지 확인 필요 |
| `Wheel_Mesh_*` | false | Cosmetic이면 허용 가능 |
| `FollowCamera` | false | 적절 |
| `CameraBoom` | false | 적절 |
| `CameraPivotRoot` | false | 적절 |

판정:

```text
대체로 합리적이지만 VehicleDriveComp 비복제 의도는 확인 필요하다.
```

메모:

```text
VehicleAimComp는 C++에서도 SetIsReplicatedByDefault(true), DOREPLIFETIME(RepAimVisualState)이 확인됐다.
VehicleMovementComp는 BP 기준 컴포넌트 복제가 true다.
DriveComp는 입력/상태 계산 컴포넌트로 보이며, 실제 상태를 복제할 필요가 있는지 여부는 Dedicated Server 테스트 결과를 보고 판단한다.
```

---

## 5. BP_CFVehiclePawn EventGraph 확인 결과

EventGraph 요약:

```text
노드 수: 21
링크 수: 24
주요 흐름:
- BeginPlay 이벤트
- Is Locally Controlled
- Get Controller
- Cast To PlayerController
- Create WBP Vehicle Debug Hud Widget
- Set VehicleDebugHudRef
- SetVehiclePawnRef
- Add to Viewport
- Create WBP Vehicle Debug Panel Widget
- Set VehicleDebugPanelRef
- SetVehiclePawnRef
- Add to Viewport
```

판정:

```text
Debug UI 생성은 로컬 소유자 조건 뒤에서 실행되는 것으로 보인다.
```

메모:

```text
BP_CFVehiclePawn EventGraph에는 BeginPlay 이후 Is Locally Controlled Branch가 있으며,
그 뒤에 Debug HUD / Debug Panel 생성과 AddToViewport 흐름이 존재한다.
따라서 Dedicated Server나 비소유 클라이언트에서 Debug UI가 생성될 위험은 낮아 보인다.

다만 실제 실행 테스트에서 Dedicated Server 로그에 Viewport/UI 관련 오류가 없는지 확인해야 한다.
```

---

## 6. TestMap 확인 결과

확인 결과:

```text
맵: /Game/Maps/TestMap.TestMap
Actor 수: 14
PlayerStart: PlayerStart_0 1개
배치 차량: BP_CFVehiclePawn_C_1 1대
```

판정:

```text
Dedicated Server 2클라 테스트 준비 부족
```

메모:

```text
현재 TestMap은 싱글플레이 테스트 맵 성격이 강하다.
PlayerStart가 1개뿐이고, 배치 차량도 1대만 확인된다.

클라이언트 2개 접속 테스트를 하려면 다음 중 하나가 필요하다.

1. 멀티 테스트용 PlayerStart와 차량을 추가한다.
2. 서버가 클라이언트 접속마다 차량을 Spawn/Possess하도록 구조를 만든다.
3. 별도 Dedicated Server 테스트 맵을 만든다.

위 작업은 에디터 설정 변경 또는 코드 작성 단계로 이어질 수 있으므로 현재 작업에서는 실행하지 않는다.
```

---

## 7. 기존 이슈 상태 갱신

### NA-001 AutoPossessPlayer Player0 고정

상태:

```text
확정
```

근거:

```text
- ACFVehiclePawn C++ 생성자에서 AutoPossessPlayer = Player0 확인
- BP_CFVehiclePawn Class Defaults에서도 AutoPossessPlayer = Player0 확인
```

판정:

```text
수정 필요 가능성 높음
```

중단 지점:

```text
이 이슈를 해결하려면 코드 작성 또는 Blueprint Class Defaults 수정이 필요하다.
사용자 지시에 따라 현재는 코드 작성하지 않고 중단 대상으로 기록한다.
```

---

### NA-002 ACFVehiclePawn 복제 설정 미확인

상태:

```text
부분 해소
```

근거:

```text
- C++에서는 bReplicates / SetReplicates 확인 안 됨
- BP_CFVehiclePawn Class Defaults에서 bReplicates = true 확인
- BP_CFVehiclePawn Class Defaults에서 bReplicateMovement = true 확인
```

판정:

```text
복제 설정은 BP 기준 켜져 있다.
실제 Dedicated Server 이동 복제는 실행 테스트 필요.
```

---

### NA-005 Debug UI 루트 생성 위치 미확인

상태:

```text
부분 해소
```

근거:

```text
- BP_CFVehiclePawn EventGraph에서 BeginPlay -> Is Locally Controlled -> Debug UI 생성 흐름 확인
```

판정:

```text
Debug UI 생성 흐름은 OwnerOnly 조건을 타는 것으로 보인다.
실제 Dedicated Server 로그 확인은 필요하다.
```

---

### NA-006 TestMap 멀티플레이 준비 부족

상태:

```text
확정
```

근거:

```text
- PlayerStart 1개
- BP_CFVehiclePawn 1대
```

판정:

```text
2클라 Dedicated Server 테스트 전에 맵 또는 Spawn/Possess 구조 정리가 필요하다.
```

중단 지점:

```text
이 이슈를 해결하려면 맵 수정, Blueprint 설정 수정, 또는 GameMode/PlayerController 코드 작성이 필요할 수 있다.
사용자 지시에 따라 현재는 코드 작성하지 않고 중단 대상으로 기록한다.
```

---

## 8. 현재 기준 위험 우선순위

### 1순위: AutoPossessPlayer Player0 제거 또는 대체 구조 결정

```text
현재 상태:
- C++와 BP 모두 Player0 자동 빙의 구조다.

문제:
- Dedicated Server 멀티플레이에서 각 클라이언트 차량 소유 구조와 충돌할 가능성이 높다.

다음 단계:
- 코드 작성 또는 BP Class Defaults 수정 단계로 넘어가기 전 사용자 확인 필요.
```

---

### 2순위: TestMap 2클라 테스트 가능 상태 만들기

```text
현재 상태:
- PlayerStart 1개
- 차량 1대

문제:
- 2클라 접속 테스트 기준이 부족하다.

다음 단계:
- 맵 수정 또는 서버 Spawn/Possess 구조 중 하나를 선택해야 한다.
```

---

### 3순위: 실제 Dedicated Server 실행선 확인

```text
현재 상태:
- BP 복제 설정은 일부 확인됐다.
- 실제 DS 실행 테스트는 아직 하지 않았다.

문제:
- 설정상 복제와 실제 Chaos Vehicle 멀티 동작은 별도로 검증해야 한다.

다음 단계:
- Dedicated Server Target 존재 여부 확인으로 넘어간다.
```

---

## 9. 코드 작성 단계 도달 여부

현재 `T-003 위험 패턴 검색`은 문서 작업으로 완료 가능하다.

하지만 다음 이슈를 해결하려면 코드 작성 또는 에디터 설정 변경 단계로 넘어간다.

```text
- AutoPossessPlayer Player0 제거
- Dedicated Server용 Spawn/Possess 구조 작성
- CarFight_ReServer.Target.cs 생성 여부 검토
- TestMap PlayerStart / 차량 배치 수정
- GameMode / PlayerController 생성 또는 지정
```

사용자 지시에 따라 위 항목은 현재 작업에서 진행하지 않는다.

---

## 10. 다음 작업 제안

다음 문서상 작업은 `T-004 Dedicated Server Target 확인`이다.

다만 `T-004`에서 `CarFight_ReServer.Target.cs`가 없다는 것이 확정되면, 그 다음은 코드 작성 단계다.

따라서 다음 턴에서는 아래까지만 진행하는 것을 권장한다.

```text
1. Server Target 파일 존재 여부 재확인
2. 프로젝트 Target 구조 확인
3. 생성이 필요하면 코드 작성 전에 중단하고 사용자에게 보고
```

---

## 11. 변경 기록

### 0.1.0

```text
- T-003 위험 패턴 상세 검색 결과 최초 작성
- BP_CFVehiclePawn 복제/Movement 복제/Auto Possess 값 확인
- BP_CFVehiclePawn EventGraph UI 생성 흐름 확인
- TestMap 멀티플레이 준비 부족 확정
- 코드 작성 단계로 이어지는 중단 지점 기록
```
