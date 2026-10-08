# 서버 런타임 테스트 기록 2026-06-01

- 문서 버전: v0.2.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 관련 단계: Phase 3, Phase 4, Phase 5, Phase 6, Phase 8
- 상태: PASS_REPORTED

---

## 1. 목적

이 문서는 `ACFMPGameMode` 기반 Dedicated Server 테스트 결과를 기록한다.

이번 기록은 사용자 보고 기준으로 작성한다.

---

## 2. 테스트 전 준비 상태

테스트 전 확인된 상태는 다음과 같다.

```text
- ACFMPGameMode 추가 완료
- BP_CFVehiclePawn 기본 fallback 연결 완료
- DefaultEngine.ini에 GlobalDefaultGameMode / GlobalDefaultServerGameMode 연결 완료
- TestMap에 PlayerStart 2개 존재
- TestMap에 배치 차량 없음
- CarFight_ReServer Win64 Development 빌드 성공
```

---

## 3. 사용자 보고 결과

### 3.1 2클라 테스트

사용자 보고:

```text
2클라 테스트 성공
```

해석:

```text
Dedicated Server 환경에서 클라이언트 2개 접속 테스트가 성공한 것으로 기록한다.
```

### 3.2 입력 분리 / 이동 복제 / 서버 안정성

사용자 보고:

```text
전부 PASS
```

해석:

```text
Phase 5 입력 분리 검증, Phase 6 이동 복제 검증, Phase 8 1차 안정화 핵심 조건을 통과한 것으로 기록한다.
```

---

## 4. PASS 처리 항목

사용자 보고 기준으로 아래 항목을 PASS 처리한다.

### 4.1 접속 / Spawn / Possess

```text
- Dedicated Server 실행
- Client 1 접속
- Client 2 접속
- 서버 GameMode 기반 차량 Spawn
- Client 1 차량 Possess
- Client 2 차량 Possess
- 두 클라이언트가 서로 다른 차량 소유
- 각 클라이언트 카메라가 자기 차량 기준으로 동작
```

### 4.2 입력 분리

```text
- Client 1 입력은 Client 1 차량에만 적용
- Client 2 입력은 Client 2 차량에만 적용
- Client 1 입력이 Client 2 차량에 적용되지 않음
- Client 2 입력이 Client 1 차량에 적용되지 않음
```

### 4.3 이동 복제

```text
- Client 1 차량 이동이 Client 2 화면에서 보임
- Client 2 차량 이동이 Client 1 화면에서 보임
- 위치 복제 확인
- 회전 복제 확인
- 정지 상태 확인
- 저속 / 고속 / 급조향 테스트 확인
```

### 4.4 Dedicated Server 금지 코드 오류

```text
- 서버 로그에서 UI 관련 치명 오류 없음
- 서버 로그에서 LocalPlayer 관련 치명 오류 없음
- 서버 로그에서 Camera 관련 치명 오류 없음
```

---

## 5. 현재 결론

2클라 테스트와 후속 검증이 모두 PASS로 보고되었으므로, CarFight 서버는 더 이상 단순 실행 수준이 아니다.

현재 기준 서버는 아래 단계에 도달했다.

```text
Dedicated Server 기반 2클라 차량 Spawn/Possess, 입력 분리, 이동 복제 1차 검증 통과
```

---

## 6. 다음 병목

서버 1차 안정화 핵심 조건은 통과했다.

다음 단계에서 검토할 후보는 아래와 같다.

```text
1. Dedicated Server 금지 코드 정밀 감사
2. UCFVehicleCameraComp Dedicated Server Tick guard 정리
3. 서버 테스트 절차 자동화 문서화
4. 무기/발사 요청의 서버 권한 구조 설계
5. 체력/대미지/리스폰 시스템 진입 조건 정리
```

---

## 7. 변경 기록

### v0.2.0

```text
- 사용자 보고 기준 입력 분리 PASS 반영
- 사용자 보고 기준 이동 복제 PASS 반영
- 사용자 보고 기준 서버 금지 코드 치명 오류 없음 PASS 반영
- Phase 8 1차 안정화 핵심 조건 통과로 결론 갱신
```

### v0.1.0

```text
- 사용자 보고 기준 2클라 Dedicated Server 테스트 성공 기록
- Phase 5 / Phase 6 후속 검증 항목 정리
```
