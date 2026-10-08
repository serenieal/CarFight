# RuntimeTest 2026-06-05 Vehicle Net Physics

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 테스트 유형: Dedicated Server 2클라 차량 네트워크 물리 진단
- 관련 작업: CFVNetDbg / CFVNetDbgFix
- 상태: Analyzed

---

## 1. 테스트 목적

`CFVNetDbgFix` 적용 이후 Dedicated Server 2클라 상태에서 차량 네트워크 물리 오차를 다시 확인한다.

이번 재테스트는 이전 게임패드 테스트와 달리 같은 PC 2클라 입력 오염을 줄이기 위해 키보드 입력을 사용했다.

---

## 2. 테스트 조건

```text
서버: Dedicated Server
클라이언트: Client 1 / Client 2
입력 조건: Client 1 중심 키보드 입력
로그 경로: RuntimeLogs/VehicleNet/
테스트 A: A_RepMoveTrue
테스트 B: B_RepMoveFalse
추출 방식: Tools/ExtractNetLog.bat
```

기존 게임패드 테스트는 다음 이유로 최종 판단에서 제외하거나 보조 자료로만 사용한다.

```text
동일 PC에서 하나의 물리 게임패드 입력이 두 클라이언트 프로세스에 동시에 읽히는 현상이 있었다.
이 현상은 현재 단계에서 네트워크 복제 결함으로 단정하지 않고 로컬 입력 폴링 오염 조건으로 분리한다.
```

---

## 3. 확인된 로그 형식

`CFVNetDbgFix` 이후 로그에는 아래 항목이 포함된다.

```text
VehicleNetDebug:
Pawn
NetMode
Role
RemoteRole
HasAuthority
IsLocal
bReplicates
RepMove
OwnerController
RuntimeReady
Seq
ServerTimeValid
CurrentServerTimeValid
SampleAge
LocErr
RotErr
VelErr
SpeedErr
AngVelErr
ServerLoc / LocalLoc
ServerVel / LocalVel
ServerAngVel / LocalAngVel
```

---

## 4. SampleAge 상태

### 4.1 개선된 점

`Client1 / RepMove=true`에서는 초기 1줄 이후 `SampleAge`가 정상적인 양수로 전환된다.

예시:

```text
RuntimeReady=false
CurrentServerTimeValid=false
SampleAge=-1.000s
```

이 값은 서버 시간 기준이 아직 준비되지 않은 초기 fallback이므로 정상이다.

RuntimeReady 이후 예시:

```text
ServerTimeValid=true
CurrentServerTimeValid=true
SampleAge=0.107s
LocErr=57.37cm
```

판정:

```text
기존의 -12s, -34s, -96s 같은 명백한 월드 시간 기준 오류는 상당 부분 개선되었다.
```

### 4.2 남은 문제

일부 클라이언트/일부 Pawn에서는 `ServerTimeValid=true`, `CurrentServerTimeValid=true`인데도 `SampleAge`가 음수로 나온다.

예시:

```text
SampleAge=-1.568s
LocErr=59.98cm
```

후반부 예시:

```text
SampleAge=-6.135s
LocErr=1441.57cm
```

판정:

```text
GameState->GetServerWorldTimeSeconds() 기준으로 바꿨지만, 클라이언트의 서버 시간 추정값과 복제 샘플의 서버 시간 사이에 음수 델타가 남아 있다.
따라서 현재 SampleAge는 보조 진단값으로만 사용하고, 최종 원격 보간 기준 시간으로 쓰면 안 된다.
```

권장 보강:

```text
SampleAge를 두 값으로 분리한다.

1. ServerTimeDelta
   - 클라이언트 추정 서버 시간 - 서버 샘플 시간
   - 현재 SampleAge와 유사하며 음수가 발생할 수 있다.

2. ReceivedAge
   - OnRep로 해당 샘플을 받은 뒤 클라이언트 로컬 시간 기준으로 흐른 시간
   - 수신 후 경과 시간이라 로그/보간 진단에 더 안정적으로 사용할 수 있다.
```

---

## 5. RepMove=true 결과

`RepMove=true` 상태에서는 서버 보정이 들어오지만 조작 중 위치/속도 오차가 튄다.

Client 1 대표 구간:

```text
RepMove=true
IsLocal=true
SampleAge=0.227s
LocErr=132.63cm
RotErr=0.40deg
VelErr=689.82cm/s
SpeedErr=24.83km/h
AngVelErr=1.84deg/s
```

다른 구간:

```text
RepMove=true
LocErr=267.38cm
RotErr=0.34deg
VelErr=1249.99cm/s
SpeedErr=45.00km/h
AngVelErr=3.21deg/s
```

판정:

```text
bReplicateMovement=true에서도 서버/클라 차량 상태 오차가 발생한다.
이번 로그에서 특히 두드러지는 것은 큰 RotErr보다 위치/선형 속도 오차다.
원격 차량이 뚝뚝 끊겨 보이는 현상은 위치/속도 오차와 서버 보정의 시각적 반영 문제일 가능성이 높다.
```

---

## 6. RepMove=false 결과

`RepMove=false` 상태에서는 위치/회전/각속도 오차가 더 빠르게 커진다.

Client 1 대표 구간:

```text
RepMove=false
IsLocal=true
LocErr=1052.22cm
RotErr=16.67deg
VelErr=158.74cm/s
AngVelErr=4.01deg/s
```

Client 2 대표 구간:

```text
RepMove=false
IsLocal=true
LocErr=473.98cm
RotErr=29.14deg
VelErr=181.17cm/s
AngVelErr=17.59deg/s
```

원격 차량 대표 구간:

```text
RepMove=false
IsLocal=false
LocErr=450.05cm
RotErr=8.40deg
AngVelErr=36.87deg/s
```

판정:

```text
bReplicateMovement=false는 시각적 끊김을 줄일 수 있지만 서버/클라 차량 상태가 빠르게 갈라진다.
이 상태를 최종 정책으로 확정하면 장기 위치, 충돌, 원격 차량 표시 문제가 남는다.
```

---

## 7. 현재 확정 결론

```text
1. bReplicateMovement=true는 서버 보정이 들어오지만 Chaos Vehicle 상태와 클라이언트 표시/물리 상태 사이에 순간 오차가 발생한다.
2. bReplicateMovement=false는 서버 보정 충돌을 줄일 수 있지만 서버/클라 위치, 회전, 속도 상태가 빠르게 갈라진다.
3. CarFight는 UE 기본 Actor Movement Replication만으로 차량 네트워크 물리를 안정화하기 어렵다.
4. 다음 단계는 차량 전용 서버 권위 NetState 복제와 원격 차량 보간 구조 설계다.
5. 다만 소유 차량 예측/서버 보정은 아직 다음 단계가 아니다. 먼저 SimulatedProxy 원격 차량 보간부터 분리한다.
```

---

## 8. 다음 작업 제안

다음 작업명 후보:

```text
CFVNetStatePlan
```

작업 성격:

```text
설계/진단 준비 패치
```

포함할 내용:

```text
- FCFVehicleNetState 설계
- SimulatedProxy 원격 차량 보간 정책 설계
- SampleAge를 ServerTimeDelta / ReceivedAge로 분리
- OnRep 수신 로컬 시간 기록
- bReplicateMovement 정책 전환 조건 정리
```

제외할 내용:

```text
- 원격 차량 보간 실제 구현
- 소유 차량 예측/서버 보정 실제 구현
- 무기/전투 서버 권한 구조
- bReplicateMovement 기본 정책 확정
```

---

## 9. Changelog

### v0.1.0

```text
- 키보드 재테스트 로그 분석 결과 정리
- SampleAge 보정의 부분 성공과 잔여 문제 기록
- RepMove=true / false 비교 결과 정리
- 다음 작업을 CFVNetStatePlan으로 제안
```
