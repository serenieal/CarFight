# BP_CFVehiclePawn / TestMap 서버 준비 상태 감사

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 상위 문서: `AuditResult.md`, `TaskList.md`
- 상태: Active

---

## 1. 목적

이 문서는 `ACFMPGameMode` 연결 이후 실제 Dedicated Server 접속 테스트 전에 필요한 Blueprint와 Map 상태를 확정한다.

검사 대상은 다음이다.

```text
- /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn
- /Game/Maps/TestMap.TestMap
```

---

## 2. BP_CFVehiclePawn 감사 결과

### 2.1 기본 정보

```text
Asset: /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn
GeneratedClass: /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn_C
NativeParentClass: /Script/CarFight_Re.CFVehiclePawn
Component Count: 19
```

판단:

```text
ACFMPGameMode의 기본 fallback 경로에서 사용할 수 있는 Blueprint 클래스가 존재한다.
```

---

### 2.2 차량 데이터

확인된 값:

```text
VehicleData = /Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar.DA_PoliceCar
```

판단:

```text
BP_CFVehiclePawn은 현재 DA_PoliceCar를 사용한다.
서버 Spawn 테스트에서 최소 차량 데이터 기준은 충족한다.
```

---

### 2.3 Player Auto Possess

확인된 값:

```text
AutoPossessPlayer = Disabled
```

판단:

```text
서버 GameMode가 명시적으로 Possess하는 구조와 충돌하지 않는다.
```

---

### 2.4 AI Auto Possess

확인된 값:

```text
AutoPossessAI = PlacedInWorld
```

판단:

```text
맵에 직접 배치된 BP_CFVehiclePawn은 AI 자동 빙의 대상이 될 수 있다.
따라서 Dedicated Server 테스트에서는 맵에 배치된 차량을 제거하거나 서버 테스트용 맵에서 제외하는 것이 안전하다.
```

---

### 2.5 Actor 복제

확인된 값:

```text
bReplicates = true
```

판단:

```text
Actor 자체는 네트워크 복제 대상이다.
```

---

### 2.6 Movement 복제

확인된 값:

```text
bReplicateMovement = true
```

판단:

```text
Actor Movement 복제는 켜져 있다.
다만 Chaos Vehicle의 실제 이동 품질은 별도 2클라 테스트에서 확인해야 한다.
```

---

### 2.7 주요 컴포넌트 복제 상태 참고

확인된 대표 값:

```text
VehicleMovementComp.bReplicates = true
VehicleMesh.bReplicates = false
VehicleDriveComp.bReplicates = false
WheelSyncComp.bReplicates = false
VehicleCameraComp.bReplicates = false
```

판단:

```text
현재 구조는 Actor/Movement/VehicleMovementComp 중심으로 복제하고,
Drive/Wheel/Camera 등은 별도 ActorComponent 복제 대상이 아닌 구조로 보인다.
이 자체는 즉시 문제라고 단정하지 않는다.
2클라 이동 관찰 테스트에서 실제 필요한 복제 범위를 다시 판단한다.
```

---

## 3. TestMap 감사 결과

### 3.1 맵 기본 정보

확인된 값:

```text
Map: /Game/Maps/TestMap.TestMap
Actor Count: 14
```

확인된 주요 Actor:

```text
PlayerStart_0
BP_CFVehiclePawn_C_1
Floor_0
Floor_1
StaticMeshActor_0
StaticMeshActor_1
StaticMeshActor_2
```

---

### 3.2 PlayerStart 상태

확인된 값:

```text
PlayerStart 수: 1개
```

판단:

```text
1클라 Dedicated 접속 테스트는 가능하다.
2클라 Dedicated 접속 테스트에는 부족하다.
```

---

### 3.3 배치 차량 상태

확인된 값:

```text
BP_CFVehiclePawn_C_1 1대가 맵에 배치되어 있다.
```

판단:

```text
현재 서버 구조는 GameMode가 플레이어 접속 시 차량을 Spawn/Possess하는 방식이다.
따라서 맵에 배치된 BP_CFVehiclePawn_C_1은 테스트 혼선을 만들 수 있다.
```

가능한 문제:

```text
- 서버가 Spawn한 차량 외에 배치 차량이 하나 더 존재한다.
- 배치 차량의 AutoPossessAI = PlacedInWorld 설정으로 AI 컨트롤러가 붙을 수 있다.
- 플레이어 차량과 배치 차량 구분이 어려워진다.
- 2클라 테스트에서 소유권/복제 관찰이 혼동될 수 있다.
```

---

## 4. 현재 판단

현재 서버 1차 테스트 진입 상태는 다음과 같다.

```text
1클라 Dedicated 접속 테스트: 조건부 가능
2클라 Dedicated 접속 테스트: 맵 정리 후 권장
```

1클라 테스트를 바로 할 수 있는 이유:

```text
- GameMode 연결 완료
- BP_CFVehiclePawn fallback 완료
- BP_CFVehiclePawn Actor 복제 true
- BP_CFVehiclePawn Movement 복제 true
- AutoPossessPlayer Disabled
- TestMap에 PlayerStart 1개 있음
```

2클라 테스트 전 정리가 필요한 이유:

```text
- PlayerStart가 1개뿐임
- 맵에 배치 차량 1대가 남아 있음
- 배치 차량의 AutoPossessAI가 PlacedInWorld임
```

---

## 5. 권장 다음 작업

### 5.1 바로 가능한 작업

```text
- 1클라 Dedicated 접속 테스트
```

목표:

```text
- 서버 로그에서 ACFMPGameMode PostLogin 확인
- BP_CFVehiclePawn Spawn 확인
- Possess success 확인
- 클라이언트 화면 검정 화면 여부 확인
- 클라이언트 입력 가능 여부 확인
```

### 5.2 2클라 테스트 전 권장 작업

```text
- TestMap에 PlayerStart 1개 추가
- PlayerStart 간 충분한 거리 확보
- 맵에 배치된 BP_CFVehiclePawn_C_1 제거 또는 서버 테스트에서 사용하지 않도록 분리
```

권장 방향:

```text
차량은 맵에 미리 배치하지 않고, ACFMPGameMode가 서버에서 Spawn한다.
```

---

## 6. 코드 작업 필요 여부

현재 이 감사 결과만으로는 추가 C++ 코드 작업이 필수라고 판단하지 않는다.

필요한 것은 다음 중 하나다.

```text
- 1클라 Dedicated 접속 테스트를 먼저 진행한다.
- 또는 TestMap을 2클라 테스트 구조로 정리한 뒤 접속 테스트를 진행한다.
```

맵/에셋 수정은 코드 작업이 아니므로 Codex 작업지시서가 아니라 에디터 작업지시서로 분리하는 것이 적합하다.

---

## 7. 변경 기록

### v0.1.0

```text
- BP_CFVehiclePawn 서버 테스트 준비 상태 감사
- VehicleData / AutoPossess / Replicates / ReplicateMovement 상태 기록
- TestMap PlayerStart 1개 / 배치 차량 1대 상태 기록
- 1클라 테스트 가능, 2클라 테스트 전 맵 정리 필요 판단 기록
```
