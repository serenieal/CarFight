# Runtime Test 2026-06-05 - Vehicle Net Keyboard

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 테스트 유형: Dedicated Server 2클라 차량 네트워크 물리 진단
- 입력 조건: 같은 PC 2클라, Client 1 키보드 조작, 게임패드 입력 오염 배제 목적
- 관련 문서:
  - `VehicleNetPhysicsJitter.md`
  - `CodexTasks/CFVNetDbg_Review.md`
  - `CodexTasks/CFVNetDbgFix_Review.md`

---

## 1. 목적

이 문서는 `CFVNetDbgFix` 적용 후 키보드 입력으로 다시 수집한 차량 네트워크 물리 로그의 해석 결과를 기록한다.

이전 게임패드 테스트에서는 하나의 물리 게임패드 입력이 Client 1 / Client 2 양쪽 프로세스에 동시에 읽힐 수 있었으므로, 이번 테스트는 키보드 입력으로 입력 오염을 줄인 상태에서 진행했다.

---

## 2. 테스트 전제

```text
서버: Dedicated Server
클라이언트: Client 1 / Client 2
실행 방식: Tools/RunNetTrue.bat, Tools/RunNetFalse.bat
로그 추출: Tools/ExtractNetLog.bat
로그 경로: RuntimeLogs/VehicleNet/
```

테스트 세트:

```text
A_RepMoveTrue
- BP_CFVehiclePawn Replicate Movement = true
- Client 1 키보드 조작

B_RepMoveFalse
- BP_CFVehiclePawn Replicate Movement = false
- Client 1 키보드 조작
```

---

## 3. 로그 필드 확인

`CFVNetDbgFix` 이후 새 로그에는 다음 필드가 포함된다.

```text
VehicleNetDebug:
Pawn
NetMode
LocalRole / RemoteRole
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

## 4. SampleAge 판정

### 4.1 개선된 점

이전 게임패드 테스트에서는 `SampleAge`가 큰 음수로 나왔다.

```text
예: -12s, -34s, -96s 수준
```

`CFVNetDbgFix` 이후에는 초기 런타임 준비 전 구간에서 다음처럼 의도된 fallback 값이 찍힌다.

```text
RuntimeReady=false
CurrentServerTimeValid=false
SampleAge=-1.000s
```

RuntimeReady 이후 Client 1 / RepMove=true에서는 `SampleAge`가 0.1~0.3초대 양수로 정상화되는 구간이 확인되었다.

### 4.2 남은 문제

Client 2 또는 RepMove=false 일부 구간에서는 다음 조건임에도 `SampleAge`가 음수로 밀리는 현상이 남았다.

```text
ServerTimeValid=true
CurrentServerTimeValid=true
SampleAge < 0
```

이는 `GameState->GetServerWorldTimeSeconds()` 기반 시간 보정이 개선은 되었지만, 로그 진단값으로 완전히 안정적이지 않다는 뜻이다.

### 4.3 다음 조치

`SampleAge`를 하나의 값으로만 보지 말고 다음 두 값으로 분리해야 한다.

```text
ServerTimeDelta
- 클라이언트가 추정한 현재 서버 시간 - 서버 샘플 시간
- 서버 시간 동기화 상태를 보는 값

ReceivedAge
- OnRep 수신 시점 이후 클라이언트 로컬 시간 기준으로 흐른 시간
- 보간 버퍼/수신 지연 진단에 사용할 값
```

---

## 5. RepMove=true 결과

키보드 테스트에서도 `RepMove=true` 상태에서 위치/속도 오차가 튄다.

대표 관찰:

```text
RepMove=true
IsLocal=true
LocErr: 약 1~3m급 발생
VelErr: 큰 구간에서 약 690~1250cm/s 수준 발생
SpeedErr: 큰 구간에서 약 25~45km/h 수준 발생
RotErr: 관찰 구간 기준 대체로 작음
```

해석:

```text
Actor Movement Replication이 켜져 있어 서버 Transform 보정은 들어오지만,
Chaos Vehicle 로컬 물리 상태와 서버 권위 상태 사이의 위치/속도 차이가 순간적으로 크게 발생한다.
```

이번 로그만으로는 Pitch/Roll 오차가 주 원인이라고 단정하기보다는, 선형 위치/속도 보정 충돌을 먼저 의심하는 것이 안전하다.

---

## 6. RepMove=false 결과

키보드 테스트에서도 `RepMove=false` 상태는 위치/회전/각속도 오차가 빠르게 커진다.

대표 관찰:

```text
RepMove=false
LocErr: 수 m ~ 10m 이상으로 증가
RotErr: 수십 도 수준으로 증가 가능
AngVelErr: 원격/비소유 차량에서 크게 발생 가능
```

해석:

```text
RepMove=false는 시각적 끊김을 줄이는 데 도움이 될 수 있지만,
서버 권위 위치/회전 보정이 약해지거나 사라져 장기 상태 불일치가 빠르게 커진다.
```

따라서 `bReplicateMovement=false` 고정은 최종 해법이 아니다.

---

## 7. 현재 결론

키보드 재테스트 결과, 이전 게임패드 입력 오염을 제외해도 차량 네트워크 물리 문제는 남아 있다.

확정 가능한 결론:

```text
1. RepMove=true는 위치/속도 오차가 순간적으로 튄다.
2. RepMove=false는 위치/회전/각속도 오차가 빠르게 누적된다.
3. UE Actor Movement Replication만으로 CarFight 차량 물리를 안정화하기 어렵다.
4. 다음 구조는 차량 전용 NetState 복제와 원격 차량 보간으로 가야 한다.
5. 소유 차량 예측/서버 보정은 원격 차량 보간 이후 단계로 미룬다.
```

---

## 8. 다음 작업

### 8.1 문서/설계 작업

```text
VehicleNetStatePlan.md 작성
- 차량 전용 서버 권위 NetState 구조 정의
- 원격 차량 보간 정책 정의
- 소유 차량 예측/서버 보정은 후순위로 분리
```

### 8.2 코드 작업 후보

코드 수정이 필요한 다음 작업은 Codex 작업지시서로 분리한다.

```text
작업명 후보: CFVNetAgeSplit
목표:
- SampleAge를 ServerTimeDelta / ReceivedAge로 분리
- OnRep 수신 로컬 시간 기록
- 로그에 ServerTimeDelta, ReceivedAge를 추가
- 기존 SampleAge는 제거하지 않고 Deprecated 표시 또는 의미 변경 없이 유지 여부 판단
```

---

## 9. 변경 기록

### v0.1.0

```text
- CFVNetDbgFix 이후 키보드 재테스트 결과 기록
- SampleAge 개선/잔여 문제 정리
- RepMove=true / false 각각의 로그 해석 기록
- 다음 설계 작업과 Codex 코드 작업 후보 분리
```
