# 서버 고도화 테스트 계획

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 상위 문서: `Roadmap.md`
- 상태: Active

---

## 1. 목적

이 문서는 서버 고도화 작업의 검증 절차를 정의한다.

목표는 기능을 만들었다고 끝내는 것이 아니라, Dedicated Server에서 실제로 다음 조건을 확인하는 것이다.

```text
- 클라이언트가 접속한다.
- 서버가 차량을 생성한다.
- PlayerController가 차량을 Possess한다.
- 각 클라이언트가 자기 차량만 조작한다.
- 상대 차량 이동이 보인다.
- 서버 로그에 로컬 전용 코드 오류가 없다.
```

---

## 2. 테스트 환경

| 항목 | 기준 |
|---|---|
| 엔진 | UE 최신버전 Source Build 기준 |
| 프로젝트 | `UE/CarFight_Re.uproject` |
| 서버 Target | `CarFight_ReServer` |
| 테스트 맵 | `/Game/Maps/TestMap.TestMap` 또는 후속 서버 테스트맵 |
| 기준 차량 | `BP_CFVehiclePawn` |
| 기준 Native Pawn | `ACFVehiclePawn` |
| 기준 GameMode 후보 | `ACFMPGameMode` |

---

## 3. 공통 로그 확인 항목

서버와 클라이언트 로그에서 아래 항목을 확인한다.

### 3.1 서버 로그

```text
- 맵 로드 성공
- 서버 리슨 시작
- PlayerController 접속
- PostLogin 호출
- 차량 Pawn Spawn 성공
- PlayerController Possess 성공
- 치명 오류 없음
- UI / LocalPlayer / Camera 관련 오류 없음
```

### 3.2 클라이언트 로그

```text
- 서버 접속 성공
- Pawn 소유 확인
- 입력 매핑 등록 성공
- 카메라 초기화 성공
- HUD / Reticle 생성 성공
- 비소유 Pawn 입력 오류 없음
```

---

## 4. Test 1. Dedicated Server 단독 실행

### 목적

서버가 맵을 정상 로드하고 대기 상태에 들어가는지 확인한다.

### 절차

```text
1. Dedicated Server 실행
2. TestMap 로드 확인
3. 서버 리슨 상태 확인
4. 치명 오류 확인
```

### PASS 기준

```text
- 서버 프로세스가 종료되지 않는다.
- TestMap이 로드된다.
- 접속 대기 상태가 된다.
- UI / 카메라 / LocalPlayer 관련 치명 오류가 없다.
```

---

## 5. Test 2. 1클라 접속 / 소유권

### 목적

클라이언트 1명이 서버에 접속하고 자기 차량을 소유하는지 확인한다.

### 절차

```text
1. Dedicated Server 실행
2. Client 1 실행
3. Client 1을 서버에 접속
4. 서버 로그에서 PostLogin 확인
5. 서버 로그에서 차량 Spawn 확인
6. 서버 로그에서 Possess 확인
7. Client 1 화면 확인
8. Client 1 입력 확인
```

### PASS 기준

```text
- Client 1 접속 성공
- 서버가 차량 Pawn 1대를 생성
- Client 1 PlayerController가 생성된 차량을 Possess
- Client 1 화면이 검정 화면으로 고정되지 않음
- Client 1 입력이 자기 차량에 적용됨
```

### FAIL 기록 항목

```text
- 접속 실패
- Pawn 미생성
- Possess 실패
- 카메라 미연결
- 입력 미적용
- 서버 로그 오류
```

---

## 6. Test 3. 2클라 접속 / 차량 분리

### 목적

클라이언트 2명이 각각 다른 차량을 소유하는지 확인한다.

### 절차

```text
1. Dedicated Server 실행
2. Client 1 접속
3. Client 2 접속
4. 서버 로그에서 PlayerController 2개 확인
5. 서버 로그에서 차량 2대 Spawn 확인
6. 각 PlayerController의 Possess 대상 확인
7. Client 1 화면 확인
8. Client 2 화면 확인
```

### PASS 기준

```text
- Client 1과 Client 2 모두 접속 성공
- 차량 Pawn이 2대 생성됨
- Client 1과 Client 2가 서로 다른 차량을 소유함
- 두 클라이언트 모두 자기 차량 기준 카메라가 있음
```

---

## 7. Test 4. 입력 분리

### 목적

각 클라이언트 입력이 자기 차량에만 적용되는지 확인한다.

### 절차

```text
1. Client 1만 가속 입력
2. Client 2 화면에서 Client 1 차량 이동 확인
3. Client 2만 조향 입력
4. Client 1 화면에서 Client 2 차량 이동 확인
5. 두 클라이언트가 동시에 입력
6. 차량 조작 대상이 섞이지 않는지 확인
```

### PASS 기준

```text
- Client 1 입력은 Client 1 차량에만 적용
- Client 2 입력은 Client 2 차량에만 적용
- 입력 교차 오염 없음
- 비소유 차량이 입력에 반응하지 않음
```

---

## 8. Test 5. 이동 복제

### 목적

상대 차량 이동이 각 클라이언트 화면에서 보이는지 확인한다.

### 절차

```text
1. Client 1이 직진
2. Client 2 화면에서 Client 1 차량 이동 확인
3. Client 2가 직진
4. Client 1 화면에서 Client 2 차량 이동 확인
5. 양쪽에서 조향
6. 정지 상태 확인
7. 급가속 / 급조향 시 움직임 품질 확인
```

### PASS 기준

```text
- 상대 차량 위치가 갱신된다.
- 상대 차량 회전이 갱신된다.
- 정지 상태도 동기화된다.
- 움직임 품질 문제가 있더라도 원인 기록이 가능하다.
```

### 품질 이슈 분리 기준

아래는 Test FAIL이 아니라 후속 튜닝 이슈로 분리할 수 있다.

```text
- 약한 위치 보정 흔들림
- 원격 차량 휠 시각 품질 부족
- 고속 주행 시 보간 품질 부족
```

아래는 Test FAIL로 본다.

```text
- 상대 차량이 전혀 보이지 않음
- 상대 차량 위치가 전혀 갱신되지 않음
- 소유하지 않은 차량이 자기 입력에 반응함
- 차량이 서버와 클라이언트에서 완전히 다른 위치로 분리됨
```

---

## 9. Test 6. Dedicated Server 금지 코드

### 목적

Dedicated Server에서 로컬 전용 코드가 실행되어 오류를 만들지 않는지 확인한다.

### 확인 대상

```text
- 위젯 생성
- Viewport 추가
- LocalPlayer 접근
- 카메라 전용 Tick
- 로컬 사운드
- 로컬 이펙트
- Debug HUD / Panel
```

### PASS 기준

```text
- 서버 로그에 위젯 생성 관련 오류 없음
- 서버 로그에 LocalPlayer 접근 오류 없음
- 서버 로그에 카메라 Null 오류 없음
- 서버 로그에 로컬 전용 사운드/이펙트 오류 없음
- 서버에서 불필요한 시각 전용 Tick이 최소화됨
```

---

## 10. 테스트 기록 양식

각 테스트 후 아래 형식으로 기록한다.

```text
테스트 일시:
테스트 환경:
서버 실행 방식:
클라이언트 수:
테스트 맵:
사용 GameMode:
결과: PASS / FAIL / PARTIAL
관찰 내용:
서버 로그 요약:
클라이언트 로그 요약:
수정 필요 항목:
다음 액션:
```

---

## 11. 변경 기록

### v0.1.0

```text
- Dedicated Server 단독 실행 테스트 정의
- 1클라 접속 / 2클라 접속 / 입력 분리 / 이동 복제 테스트 정의
- Dedicated Server 금지 코드 검증 항목 정의
- 테스트 기록 양식 작성
```
