# 차량 네트워크 물리 흔들림 / 위치 불일치 진단

- 문서 버전: v0.4.0
- 작성일: 2026-06-04
- 최종 갱신일: 2026-06-05
- 대상 프로젝트: CarFight
- 관련 영역: Dedicated Server 차량 이동 복제 / Chaos Vehicle / Actor Movement Replication
- 상태: Active

---

## 1. 목적

이 문서는 Dedicated Server 2클라 차량 테스트 중 발견된 차량 흔들림, 원격 차량 순간이동, 장기 위치 불일치 문제를 정리한다.

서버 1차 안정화는 통과했지만, 차량 네트워크 물리 품질 문제가 확인되었다.

---

## 2. 최초 증상

사용자 보고:

```text
오프라인일 때는 멀쩡하다.
온라인 이후 쓰로틀을 올리면 물리효과로 차 앞부분이 살짝 들리는 시점에서 차가 자꾸 흔들린다.
자연스럽지 않고 뚝뚝 끊기듯이 흔들린다.
차의 각도가 싱크로가 안 맞는 느낌이다.
```

추가 보고:

```text
조작 중인 화면에서는 Client 1, Client 2 가리지 않고 흔들린다.
다른 차 시점으로 보면 차가 뚝뚝 끊겨서 순간이동하듯이 앞으로 간다.
심장박동 그래프처럼 가다가 뚝, 가다가 뚝 하는 느낌이다.
```

---

## 3. 원인 후보 분석

처음 의심한 주요 후보는 다음이었다.

```text
- Actor Movement Replication과 Chaos Vehicle Movement 보정이 중복으로 작동
- 서버 권위 Transform 보정이 클라이언트 물리 표시를 주기적으로 덮어씀
- 원격 차량에 별도 보간 구조가 없어 서버 위치 갱신이 뚝뚝 보임
```

근거:

```text
- BP_CFVehiclePawn의 bReplicateMovement가 true였음
- CFVehiclePawn C++에서는 SetReplicateMovement를 명시적으로 제어하지 않음
- 서버 GameMode 전환 후 차량이 런타임 Spawn/Possess 흐름을 타게 됨
```

---

## 4. 임시 테스트: Replicate Movement OFF

테스트 내용:

```text
BP_CFVehiclePawn에서 Replicate Movement를 false로 임시 변경하고 2클라 테스트
```

사용자 보고 결과:

```text
bReplicateMovement = false 하니까 끊기지 않는다.
```

추가 확인 결과:

```text
1. 조작 중인 자기 차량이 계속 부드러운가 = yes
2. 상대 차량도 순간이동 없이 부드럽게 보이는가 = yes
3. 위치가 장시간 지나도 서버/클라에서 크게 어긋나지 않는가 = 문제 있음
4. 충돌, 회전, 정지 상태도 정상인가 = 위치 불일치 문제가 생기기 전까지 정상
```

---

## 5. Replicate Movement OFF 후 새로 확인된 문제

사용자 보고:

```text
Client 1에서는 Client 2 차량을 안 박고 아슬하게 지나갔는데,
Client 2에서는 Client 1 차량이 Client 2 차량을 박았다고 나온다.

Client 1에서 Client 1 차량이 계속 운전하면서 위치가 크게 틀어지자,
Client 2에서 Client 1 차량이 사라졌다.
```

해석:

```text
Replicate Movement를 끄면 서버 Transform 보정으로 인한 뚝뚝 끊김은 사라진다.
하지만 서버 권위 위치/회전 보정도 사라지거나 약해져서 장기 위치와 충돌 판정이 클라이언트별로 갈라진다.
```

---

## 6. 현재 결론

`bReplicateMovement=true` 상태:

```text
- 서버 권위 Transform 보정은 들어온다.
- 하지만 Chaos Vehicle 물리 표시와 Actor Movement Replication이 충돌해 차체가 뚝뚝 끊긴다.
```

`bReplicateMovement=false` 상태:

```text
- 시각적 끊김은 사라진다.
- 조작 차량과 원격 차량이 더 부드럽게 보인다.
- 하지만 장기 위치/충돌 결과가 클라이언트마다 어긋난다.
- 원격 차량이 큰 위치 차이 이후 사라질 수 있다.
```

따라서 최종 해법은 단순 ON/OFF가 아니다.

현재 필요한 구조:

```text
Actor Movement 자동 복제에만 의존하지 않고,
차량 전용 서버 권위 이동 상태를 복제하고,
클라이언트에서 부드럽게 보간/보정하는 구조
```

---

## 7. 권장 네트워크 이동 정책 방향

### 7.1 단기 정책

```text
- BP_CFVehiclePawn의 Replicates는 true 유지
- bReplicateMovement는 최종 결정 전까지 실험 변수로 유지
- bReplicateMovement=false는 시각 끊김 원인 분리용으로는 유효
- 하지만 이 상태를 그대로 최종 확정하면 장기 위치/충돌 불일치가 남음
```

### 7.2 중기 정책

차량 전용 권위 상태 구조를 만든다.

후보 구조:

```text
FCFVehicleNetState
- ServerTransform
- LinearVelocity
- AngularVelocity
- ServerTimestamp
- Throttle
- Brake
- Steering
- bHandbrake
```

서버 책임:

```text
- 차량 최종 물리 위치/회전 권위 보유
- 일정 주기로 VehicleNetState 복제
- 충돌/겹침/권위 판정 기준 유지
```

클라이언트 책임:

```text
- 소유 차량은 즉각적인 조작감을 유지
- 원격 차량은 서버 상태를 목표값으로 부드럽게 보간
- 큰 오차는 부드러운 보정 또는 제한적 스냅으로 처리
```

---

## 8. 다음 작업 제안

바로 완성형 네트워크 이동 시스템을 만들기 전에, 먼저 진단 패치를 추가한다.

진단 목적:

```text
- 서버와 각 클라이언트에서 같은 차량의 위치/회전 차이를 수치로 확인
- 소유 차량과 원격 차량의 Role 차이 확인
- bReplicateMovement true/false 상태에서 오차 변화 확인
- 사라짐 발생 시점의 거리, Role, NetDormancy, Relevancy 관련 상태 확인
```

진단 로그 후보:

```text
- NetMode
- LocalRole
- RemoteRole
- IsLocallyControlled
- HasAuthority
- bReplicates
- bReplicateMovement
- ActorLocation
- ActorRotation
- LinearVelocity
- AngularVelocity
- SpeedKmh
- ForwardSpeedKmh
- Owner Controller
```

---

## 9. 다음 Codex 작업 후보

작업 이름:

```text
CFVehicleNetDebug 진단 로그 추가
```

대상 파일 후보:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

추가 후보:

```text
bEnableNetPhysicsDebug
NetPhysicsDebugInterval
LogVehicleNetPhysicsState()
```

주의:

```text
이 작업은 정식 해결 패치가 아니라 원인 확인을 위한 진단 패치다.
```

---

## 10. 1차 진단 패치 적용 상태

### 10.1 적용 작업

```text
작업명: CFVNetDbg
대상 파일:
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
리뷰 문서:
- Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbg_Review.md
```

### 10.2 현재 구현된 진단값

```text
FCFVehicleNetDebugSample
- bValid
- SampleSequenceId
- ServerWorldTimeSeconds
- ServerLocation
- ServerRotation
- ServerLinearVelocity
```

클라이언트 로그 출력값:

```text
- Pawn
- LocalRole
- RemoteRole
- RepMove
- Seq
- SampleAge
- LocErr
- RotErr
- VelErr
- SpeedErr
- ServerLoc
- LocalLoc
- ServerVel
- LocalVel
```

### 10.3 빌드 검증

```text
CarFight_ReEditor Win64 Development: Succeeded
CarFight_ReServer Win64 Development: Succeeded
```

### 10.4 현재 판정

```text
CFVNetDbg 1차 진단 패치는 빌드 기준 통과.
Dedicated Server 2클라 런타임 로그 수집은 아직 필요.
```

### 10.5 남은 진단 보강 후보

```text
- AngularVelocityDeg 추가
- NetMode 로그 추가
- HasAuthority 로그 추가
- IsLocallyControlled 로그 추가
- bReplicates 로그 추가
- OwnerController 이름 로그 추가
- 서버 자체 로그 추가
- bVehicleRuntimeReady 이후에만 진단 Tick 수행하도록 이동
- 진단 기본값 true -> false 전환
```

---

## 11. CFVNetDbgFix 진단 보강 적용 상태

### 11.1 적용 작업

```text
작업명: CFVNetDbgFix
대상 파일:
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
리뷰 문서:
- Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbgFix_Review.md
```

### 11.2 보강된 진단값

```text
FCFVehicleNetDebugSample
- bValid
- SampleSequenceId
- ServerWorldTimeSeconds
- bServerTimeValid
- ServerLocation
- ServerRotation
- ServerLinearVelocity
- ServerAngularVelocityDeg
```

클라이언트 로그 출력값:

```text
- VehicleNetDebug:
- Pawn
- NetMode
- LocalRole / RemoteRole
- HasAuthority
- IsLocal
- bReplicates
- RepMove
- OwnerController
- RuntimeReady
- Seq
- ServerTimeValid
- CurrentServerTimeValid
- SampleAge
- LocErr
- RotErr
- VelErr
- SpeedErr
- AngVelErr
- ServerLoc / LocalLoc
- ServerVel / LocalVel
- ServerAngVel / LocalAngVel
```

### 11.3 SampleAge 보강 방식

```text
이전 방식:
- 서버의 GetWorld()->GetTimeSeconds()와 클라이언트의 GetWorld()->GetTimeSeconds()를 직접 비교했다.
- 서로 다른 프로세스 기준 시간이어서 SampleAge가 큰 음수로 나올 수 있었다.

보강 방식:
- 서버 샘플은 GameState->GetServerWorldTimeSeconds() 기준 시간을 저장한다.
- 클라이언트도 GameState->GetServerWorldTimeSeconds() 기준 현재 서버 시간을 읽어 SampleAge를 계산한다.
- 양쪽 모두 GameState 서버 시간이 유효할 때만 SampleAge를 계산한다.
- 유효하지 않으면 SampleAge=-1.0으로 표시하고 ServerTimeValid / CurrentServerTimeValid를 false로 남긴다.
```

### 11.4 같은 PC 게임패드 입력 오염 주의

```text
같은 PC에서 2클라를 실행하면 하나의 물리 게임패드 입력이 두 클라이언트 프로세스에 동시에 읽힐 수 있다.
이 현상은 현재 단계에서 네트워크 복제 결함으로 단정하지 않는다.
로컬 입력 폴링 오염 조건으로 분리해서 재검증해야 한다.
```

재검증 권장:

```text
- Client 1은 키보드 입력으로 테스트한다.
- Client 2는 입력을 주지 않거나 별도 입력 장치 격리가 확인된 상태에서 테스트한다.
- bReplicateMovement=true / false를 각각 실행한다.
- Tools/ExtractNetLog.bat로 VehicleNetDebug 로그를 추출한다.
```

### 11.5 빌드 검증

```text
CarFight_ReEditor Win64 Development: Succeeded
CarFight_ReServer Win64 Development: Succeeded
```

### 11.6 현재 판정

```text
CFVNetDbgFix는 진단 보강 패치다.
차량 Transform 보정, 원격 차량 보간, 소유 차량 예측/서버 보정은 아직 구현하지 않았다.
bReplicateMovement 기본 정책도 변경하지 않았다.
다음 단계는 보강된 로그로 RepMove true/false의 위치/회전/선형속도/각속도 오차를 비교하는 것이다.
```

---

## 12. 변경 기록

### v0.4.0

```text
- CFVNetDbgFix 이후 키보드 입력 기반 Dedicated Server 2클라 재테스트 결과 기록
- Runtime Test 문서 `RT_20260605_VNetKey.md` 추가
- SampleAge는 개선되었지만 일부 클라이언트/설정에서 음수 잔여 문제가 있음을 기록
- RepMove=true에서도 위치/속도 오차가 순간적으로 튀는 점 확인
- RepMove=false에서는 위치/회전/각속도 오차가 빠르게 누적되는 점 재확인
- 차량 전용 NetState / 원격 차량 보간 설계 문서 `VehicleNetStatePlan.md` 추가
```

### v0.3.0

```text
- CFVNetDbgFix 진단 보강 적용 상태 기록
- SampleAge를 GameState 서버 시간 기준으로 계산하도록 보강
- 서버 시간 기준이 유효하지 않으면 SampleAge=-1.0과 유효성 false를 로그에 남기도록 정리
- ServerAngularVelocityDeg / LocalAngularVelocityDeg / AngVelErr 진단 항목 추가
- NetMode, HasAuthority, IsLocallyControlled, bReplicates, OwnerController, RuntimeReady 로그 항목 추가
- 같은 PC 2클라 테스트에서 하나의 물리 게임패드 입력이 두 클라이언트에 동시에 읽힐 수 있음을 주의사항으로 기록
- CFVNetDbgFix Editor / Server Target 빌드 성공 기록
```

### v0.2.0

```text
- CFVNetDbg 1차 진단 패치 적용 상태 기록
- Editor / Server Target 빌드 성공 기록
- 런타임 테스트 대기 상태와 후속 보강 후보 정리
```

### v0.1.0

```text
- 온라인 차량 흔들림 / 원격 차량 순간이동 증상 기록
- Replicate Movement OFF 테스트 결과 기록
- 시각 끊김은 사라졌지만 장기 위치/충돌 불일치가 발생함을 기록
- 단순 bReplicateMovement OFF는 최종 해법이 아님을 명시
- 차량 전용 서버 권위 상태 복제/보간 구조 필요성 정리
```
