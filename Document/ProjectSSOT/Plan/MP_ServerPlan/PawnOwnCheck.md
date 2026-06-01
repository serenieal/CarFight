# T-005 Pawn 또는 차량 소유권 확인 결과

- 문서 버전: 0.1.0
- 작성일: 2026-05-26
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`
- 관련 문서: `DSMinRun.md`, `MapModeCheck.md`, `RiskSearch.md`
- 현재 작업: `T-005 Pawn 또는 차량 소유권 확인`

---

## 1. 목적

이 문서는 현재 구현된 CarFight 차량 Pawn이 Dedicated Server 환경에서 어떤 소유권 검증 상태인지 기록한다.

현재 단계에서는 코드 작성, 맵 수정, Blueprint 수정은 하지 않는다.

---

## 2. 확인 대상

이번 작업에서 확인한 대상은 다음과 같다.

```text
- UE/Binaries/Win64/CarFight_ReServer.exe
- UE/Saved/Logs/CodexDedicatedServerTest.log
- UE/Saved/Logs/CodexStagedServerTest.log
- UE/Saved/Logs/CarFight_Re.log
- UE/Saved/Logs/CarFight_Re_2.log
- UE/Saved/Logs/CarFight_Re_3.log
- /Game/Maps/TestMap.TestMap
- BP_CFVehiclePawn 관련 기존 문서 기록
```

---

## 3. 서버 바이너리 확인 결과

`UE/Binaries/Win64`에서 Dedicated Server 산출물이 확인되었다.

```text
CarFight_ReServer.exe
CarFight_ReServer.target
CarFight_ReServer.pdb
CarFight_ReServer.lib
CarFight_ReServer.exp
```

판정:

```text
Dedicated Server 빌드 산출물은 존재한다.
```

---

## 4. 서버 실행 로그 확인 결과

### 4.1 비스테이지 서버 실행 로그

파일:

```text
UE/Saved/Logs/CodexDedicatedServerTest.log
```

확인 결과:

```text
ExecutableName: CarFight_ReServer.exe
Platform: WindowsServer
Command Line: /Game/Maps/TestMap -log -unattended -NoSound
```

하지만 아래 크래시가 확인되었다.

```text
Assertion failed: ReaderPos + Num <= ReaderSize
File: BufferReader.h
Runnable thread IOThreadPool #0 crashed
```

판정:

```text
비스테이지 상태 직접 실행은 패키지/에셋 읽기 문제로 크래시한 기록이 있다.
이 로그는 T-005 소유권 검증에 사용할 수 없다.
```

---

### 4.2 스테이지 서버 실행 로그

파일:

```text
UE/Saved/Logs/CodexStagedServerTest.log
```

확인 결과:

```text
Error: 0건
Warning: 0건
```

주요 로그:

```text
IpNetDriver listening on port 7777
Bringing World /Game/Maps/TestMap.TestMap up for play
Load map complete /Game/Maps/TestMap
Engine is initialized. Leaving FEngineLoop::Init()
```

판정:

```text
스테이지 서버 실행은 성공했다.
서버는 /Game/Maps/TestMap을 로드했고 7777 포트에서 리슨 상태까지 올라갔다.
```

---

## 5. 클라이언트 접속 로그 확인 결과

검색 대상:

```text
UE/Saved/Logs/CodexStagedServerTest.log
UE/Saved/Logs/CarFight_Re.log
UE/Saved/Logs/CarFight_Re_2.log
UE/Saved/Logs/CarFight_Re_3.log
```

검색 결과:

```text
CodexStagedServerTest.log:
- Join 관련 로그 없음
- 클라이언트 접속 관련 로그 없음

CarFight_Re.log:
- 일반 Editor/Client 로그로 보이는 LogNetVersion만 확인
- 서버 접속 성공 근거 없음

CarFight_Re_2.log:
- LogNet 검색 결과 없음

CarFight_Re_3.log:
- 과거 로그로 보이는 LogNetVersion만 확인
- 서버 접속 성공 근거 없음
```

판정:

```text
현재 남아 있는 로그만으로는 클라이언트 1 또는 클라이언트 2의 서버 접속을 확인할 수 없다.
```

---

## 6. Pawn / 차량 소유권 검증 상태

현재 확인 가능한 상태는 다음과 같다.

```text
서버 실행:
- 성공

서버 맵 로드:
- 성공

서버 리슨 포트:
- 7777 확인

클라이언트 1 접속:
- 미확인

클라이언트 2 접속:
- 미확인

PlayerController 생성:
- 미확인

Pawn 또는 차량 Spawn:
- 미확인

Possess:
- 미확인

각 클라이언트가 서로 다른 차량을 소유하는지:
- 미확인

입력 가능 여부:
- 미확인

카메라 / HUD 정상 여부:
- 미확인
```

---

## 7. 현재 구조상 예상 위험

`MapModeCheck.md`와 기존 조사 결과를 기준으로 현재 예상되는 위험은 다음이다.

```text
- TestMap에는 PlayerStart가 1개뿐이다.
- TestMap에는 BP_CFVehiclePawn이 1대뿐이다.
- Config에는 GlobalDefaultGameMode 또는 GlobalDefaultServerGameMode가 없다.
- C++ 커스텀 GameMode 클래스가 없다.
- C++ 커스텀 PlayerController 클래스가 없다.
- BP_CFVehiclePawn Class Defaults 기준 AutoPossessPlayer = Player0가 남아 있다.
```

따라서 2클라 접속을 실제로 수행하면 다음 문제가 발생할 가능성이 있다.

```text
- 클라이언트 2명이 같은 시작 위치 또는 불명확한 위치를 사용할 수 있다.
- 각 클라이언트가 별도 차량을 소유하지 못할 수 있다.
- 배치 차량 1대를 누가 소유하는지 불명확할 수 있다.
- 기본 Pawn이 생성되고 차량은 별도로 남을 수 있다.
- 입력이 기대한 차량에 연결되지 않을 수 있다.
```

---

## 8. T-005 판정

현재 T-005는 아직 완료할 수 없다.

이유:

```text
서버 리슨 성공까지는 확인됐지만,
클라이언트 2개 접속과 차량 소유권 로그가 없다.
```

현재 상태:

```text
T-005 진행 중
```

완료 조건:

```text
1. 클라이언트 1이 서버에 접속한다.
2. 클라이언트 2가 서버에 접속한다.
3. 각 클라이언트의 PlayerController가 확인된다.
4. 각 클라이언트가 소유한 Pawn 또는 차량이 확인된다.
5. 클라이언트 1과 클라이언트 2가 같은 차량을 조작하지 않는다는 점이 확인된다.
```

---

## 9. 다음 수동 검증 절차

현재 도구에서 클라이언트 2개를 안정적으로 실행하는 전용 기능은 확인되지 않았다.
따라서 다음 검증은 수동 실행으로 진행하는 것이 안전하다.

### 9.1 서버 실행

스테이지 서버 기준으로 실행한다.

```text
CarFight_ReServer.exe /Game/Maps/TestMap -log -unattended -NoSound
```

기대 로그:

```text
IpNetDriver listening on port 7777
Load map complete /Game/Maps/TestMap
Engine is initialized
```

---

### 9.2 클라이언트 1 접속

클라이언트 1에서 접속한다.

```text
open 127.0.0.1:7777
```

확인할 것:

```text
- 서버 로그에 접속 기록이 생기는지
- 클라이언트 1 화면이 서버 맵으로 이동하는지
- 조작 가능한 Pawn 또는 차량이 있는지
- 카메라와 HUD가 정상인지
```

---

### 9.3 클라이언트 2 접속

클라이언트 2에서 접속한다.

```text
open 127.0.0.1:7777
```

확인할 것:

```text
- 서버 로그에 두 번째 접속 기록이 생기는지
- 클라이언트 2 화면이 서버 맵으로 이동하는지
- 클라이언트 1과 별도의 Pawn 또는 차량을 소유하는지
- 클라이언트 1 입력과 클라이언트 2 입력이 분리되는지
```

---

## 10. 로그 수집 권장

수동 검증 시 아래 로그를 따로 저장하는 것을 권장한다.

```text
Server_T005.log
Client1_T005.log
Client2_T005.log
```

파일명은 32자를 넘기지 않는다.

기록할 항목:

```text
- 서버 실행 명령
- 클라이언트 1 접속 방식
- 클라이언트 2 접속 방식
- 접속 성공 여부
- 각 클라이언트가 보는 Pawn 또는 차량
- 각 클라이언트 입력 결과
- 서버 오류 여부
- 클라이언트 오류 여부
```

---

## 11. 코드 작성 또는 에디터 수정 단계 도달 여부

아직 코드 작성 단계로 확정하지 않는다.

다만 수동 검증에서 아래 문제가 확인되면 다음 단계는 코드 작성 또는 에디터 설정 변경으로 이어질 가능성이 높다.

```text
- 클라이언트가 차량을 소유하지 못함
- 두 클라이언트가 같은 차량을 조작함
- 클라이언트 2가 별도 Pawn 또는 차량을 얻지 못함
- DefaultPawn이 잘못 생성됨
- BP_CFVehiclePawn AutoPossessPlayer = Player0가 멀티플레이를 방해함
- GameMode 또는 PlayerController가 필요함
- TestMap에 PlayerStart나 차량 추가가 필요함
```

사용자 지시에 따라 위 항목은 현재 작업에서 구현하지 않는다.

---

## 12. 변경 기록

### 0.1.0

```text
- T-005 Pawn 또는 차량 소유권 확인 결과 최초 작성
- 서버 바이너리 존재 확인 기록
- 비스테이지 서버 실행 크래시 로그 기록
- 스테이지 서버 실행 성공 로그 기록
- 클라이언트 2개 접속 미검증 상태 기록
- 다음 수동 검증 절차 기록
```
