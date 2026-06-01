# PIE / Dedicated 접속 테스트 관찰 기록

- 문서 버전: 0.1.0
- 작성일: 2026-05-26
- 대상 프로젝트: CarFight
- 관련 문서: `PawnOwnCheck.md`, `DSMinRun.md`, `MapModeCheck.md`, `RiskSearch.md`

---

## 1. 목적

이 문서는 현재 구현 상태에서 PIE 멀티플레이와 Dedicated Server 접속 테스트를 시도한 결과를 기록한다.

현재 단계에서는 코드 작성, 맵 수정, Blueprint 수정은 하지 않는다.

---

## 2. 테스트 결과 요약

### 2.1 Play As Listen Server

결과:

```text
빙의 성공
```

판정:

```text
차량 Pawn 자체, 입력 바인딩, 카메라 기본 흐름은 살아 있는 것으로 본다.
```

의미:

```text
Listen Server에서는 호스트가 서버이면서 로컬 클라이언트이기 때문에,
배치 차량 또는 기존 로컬 플레이어 흐름이 작동할 수 있다.
```

---

### 2.2 Play As Client

결과:

```text
빙의 실패
```

판정:

```text
클라이언트 전용 실행에서는 서버가 플레이어에게 차량 Pawn을 할당하거나 Possess해주는 구조가 부족한 것으로 본다.
```

의미:

```text
차량 기능 자체의 문제라기보다,
Dedicated Server / 클라이언트 구조에서 PlayerController가 소유할 Pawn 또는 차량을 받지 못하는 문제로 본다.
```

---

### 2.3 Dedicated Server + open 접속

테스트 방식:

```text
빙의가 안 된 상태에서 open 명령어로 서버 접속
```

결과:

```text
접속은 됨
화면은 검정색
```

판정:

```text
네트워크 접속 자체는 가능하지만,
클라이언트의 소유 Pawn, ViewTarget, Camera 연결이 정상 설정되지 않은 상태로 추정한다.
```

의미:

```text
서버가 플레이어 접속 시 차량 Pawn을 Spawn/Possess하지 않거나,
현재 맵의 배치 차량 구조가 멀티플레이 클라이언트 접속 흐름과 맞지 않을 가능성이 높다.
```

---

## 3. 현재까지 확정한 내용

```text
1. Dedicated Server 빌드와 실행은 성공했다.
2. 서버 접속 자체도 가능하다.
3. Listen Server에서는 차량 빙의가 된다.
4. Play As Client에서는 차량 빙의가 되지 않는다.
5. Dedicated Server 접속 후 검정 화면이 나온다.
6. 차량 기능 자체보다 Spawn/Possess 구조가 문제일 가능성이 높다.
```

---

## 4. 현재 구조상 원인 후보

기존 조사 결과와 이번 관찰을 합치면 원인 후보는 다음이다.

```text
- TestMap에 PlayerStart가 1개뿐이다.
- TestMap에 BP_CFVehiclePawn이 1대뿐이다.
- Config에 GlobalDefaultGameMode 또는 GlobalDefaultServerGameMode가 없다.
- 커스텀 GameMode C++ 클래스가 없다.
- 커스텀 PlayerController C++ 클래스가 없다.
- 서버가 접속한 PlayerController마다 차량을 Spawn/Possess하는 구조가 없다.
- BP_CFVehiclePawn의 AutoPossessPlayer 설정은 기존에 Player0였고, Codex 작업 이후 C++는 Disabled 상태다.
```

---

## 5. 판정

현재 문제는 테스트 방법 문제가 아니라, 다음 구조가 아직 없어서 발생한 것으로 본다.

```text
멀티플레이용 최소 Spawn/Possess 구조
```

필요한 최소 구조:

```text
1. 서버가 플레이어 접속을 받는다.
2. PlayerStart를 기준으로 차량 Pawn을 생성한다.
3. PlayerController가 생성된 차량 Pawn을 Possess한다.
4. 각 클라이언트는 자기 차량만 조작한다.
5. 다른 클라이언트는 그 차량 이동을 관찰한다.
```

---

## 6. 다음 코드 작업 후보

다음 코드 작업 후보는 아래 하나로 좁힌다.

```text
CarFight Dedicated Server 테스트용 최소 GameMode Spawn/Possess 구조 작성
```

포함 범위:

```text
- 최소 GameMode 또는 GameModeBase 클래스
- 접속한 PlayerController마다 BP_CFVehiclePawn 또는 ACFVehiclePawn 생성
- PlayerStart 기반 Spawn 위치 사용
- PlayerController Possess 처리
- PIE 2인 / Dedicated Server 접속 테스트 가능 상태 만들기
```

제외 범위:

```text
- 무기
- 체력
- 대미지
- 스코어
- 킬/데스
- 리스폰
- 로비
- 세션
- 매치메이킹
```

---

## 7. 변경 기록

### 0.1.0

```text
- Play As Listen Server 빙의 성공 기록
- Play As Client 빙의 실패 기록
- Dedicated Server open 접속 후 검정 화면 기록
- 최소 GameMode Spawn/Possess 구조 필요성 기록
```
