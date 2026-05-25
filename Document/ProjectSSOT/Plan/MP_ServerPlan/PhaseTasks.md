# 다음 단계 작업 분해표

- 문서 버전: 0.1.0
- 작성일: 2026-05-22
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`

---

## 1. 목적

이 문서는 CarFight의 Dedicated Server 전환을 실제 작업 단위로 쪼갠다.

현재 목표는 다음 순서다.

```text
1. 현재 구현 기능을 파악한다.
2. 네트워크 위험 요소를 찾는다.
3. Dedicated Server 최소 실행선을 만든다.
4. 기존 기능의 서버/클라이언트 책임을 분리한다.
5. 현재 구현 기능 안정화 결과를 정리한다.
```

현재 단계에서는 새 게임플레이 시스템을 만들지 않는다.

---

## 2. 전체 작업 흐름

```text
T-001 현재 구현 기능 목록 작성
T-002 네트워크 위험 패턴 검색
T-003 Dedicated Server Target 확인
T-004 테스트 맵과 GameMode 확인
T-005 Pawn 또는 차량 소유권 확인
T-006 현재 구현된 입력/이동 복제 확인
T-007 UI / 카메라 / 사운드 서버 분리
T-008 Dedicated Server 최소 실행선 통과
T-009 현재 구현 기능 안정화 결과 정리
```

---

## 3. 작업 상세

### T-001 현재 구현 기능 목록 작성

목적:

```text
현재 프로젝트에 실제로 구현된 기능을 목록화한다.
```

작업:

- [ ] C++ 클래스 목록 확인
- [ ] Blueprint Asset 목록 확인
- [ ] 현재 사용하는 GameMode 확인
- [ ] 현재 사용하는 PlayerController 확인
- [ ] 현재 사용하는 Pawn 또는 Vehicle Pawn 확인
- [ ] 현재 UI 생성 방식 확인
- [ ] 차량 관련 기능 목록 작성

산출물:

```text
FeatureList.md 갱신
```

완료 기준:

```text
현재 구현된 주요 기능이 FeatureList.md에 기록되어 있다.
```

---

### T-002 네트워크 위험 패턴 검색

목적:

```text
Dedicated Server에서 문제가 될 가능성이 높은 코드를 찾는다.
```

작업:

- [ ] `GetPlayerController(0)` 검색
- [ ] `GetFirstPlayerController` 검색
- [ ] `CreateWidget` 검색
- [ ] `SpawnActor` 검색
- [ ] `PlaySound` 검색
- [ ] Niagara / Particle Spawn 검색
- [ ] 카메라 접근 코드 검색
- [ ] 입력 함수에서 상태 변경하는 코드 검색

산출물:

```text
NetAudit.md 이슈 기록 갱신
```

완료 기준:

```text
위험 코드 후보가 목록화되어 있고, 수정 우선순위가 정해져 있다.
```

---

### T-003 Dedicated Server Target 확인

목적:

```text
CarFight Dedicated Server 빌드와 실행 가능성을 확인한다.
```

작업:

- [ ] `CarFightServer.Target.cs` 존재 여부 확인
- [ ] 없으면 생성 필요 항목으로 기록
- [ ] Server Target 빌드 가능 여부 확인
- [ ] 빌드 실패 시 오류 기록

산출물:

```text
DSMinRun.md 테스트 환경 항목 갱신
```

완료 기준:

```text
Dedicated Server Target 존재 여부와 다음 액션이 명확히 기록되어 있다.
```

---

### T-004 테스트 맵과 GameMode 확인

목적:

```text
Dedicated Server 테스트에 사용할 맵과 GameMode를 확정한다.
```

작업:

- [ ] 테스트 맵 후보 확인
- [ ] PlayerStart 개수 확인
- [ ] GameMode 설정 확인
- [ ] DefaultPawnClass 확인
- [ ] PlayerControllerClass 확인
- [ ] 싱글 전용 초기화 코드 확인

산출물:

```text
DSMinRun.md 테스트 환경 항목 갱신
```

완료 기준:

```text
Dedicated Server 테스트에 사용할 맵과 GameMode가 정해져 있다.
```

---

### T-005 차량 Pawn 소유권 확인

목적:

```text
클라이언트가 자기 차량만 조작하도록 소유권 흐름을 확인한다.
```

작업:

- [ ] 서버에서 Pawn이 Spawn되는지 확인
- [ ] PlayerController가 Pawn을 Possess하는지 확인
- [ ] 클라이언트에서 `IsLocallyControlled`가 올바른지 확인
- [ ] 입력이 소유자에게만 적용되는지 확인
- [ ] 비소유 차량에 입력이 적용되지 않는지 확인

산출물:

```text
NetAudit.md 차량 소유권 항목 갱신
DSMinRun.md 테스트 결과 갱신
```

완료 기준:

```text
클라이언트 2개가 각각 자기 차량만 조작한다.
```

---

### T-006 차량 이동 복제 확인

목적:

```text
다른 플레이어 차량 이동이 클라이언트 화면에서 보이는지 확인한다.
```

작업:

- [ ] 차량 Pawn의 `bReplicates` 확인
- [ ] 이동 복제 설정 확인
- [ ] 클라이언트 A에서 B 차량 이동 확인
- [ ] 클라이언트 B에서 A 차량 이동 확인
- [ ] 위치/회전 튐 현상 기록

산출물:

```text
DSMinRun.md 이동 복제 테스트 결과 갱신
```

완료 기준:

```text
두 클라이언트가 서로의 차량 이동을 볼 수 있다.
```

---

### T-007 UI / 카메라 / 사운드 서버 분리

목적:

```text
Dedicated Server에서 실행되면 안 되는 로컬 전용 코드를 분리한다.
```

작업:

- [ ] UI 생성 위치를 PlayerController 소유 클라이언트 흐름으로 제한
- [ ] 카메라 코드를 소유 클라이언트 전용으로 제한
- [ ] 사운드 재생을 서버 판정 로직에서 분리
- [ ] 이펙트 재생을 서버 판정 로직에서 분리
- [ ] Dedicated Server 로그 오류 확인

산출물:

```text
NetAudit.md 이슈 해결 기록 갱신
```

완료 기준:

```text
Dedicated Server 로그에 UI/카메라/로컬 사운드 관련 오류가 없다.
```

---

### T-008 Dedicated Server 최소 실행선 통과

목적:

```text
현재 구현 기능 안정화에 들어가기 전 최소 멀티플레이 실행 기준을 통과한다.
```

작업:

- [ ] Dedicated Server 실행
- [ ] 클라이언트 2개 접속
- [ ] 각 클라이언트 차량 소유 확인
- [ ] 입력 분리 확인
- [ ] 이동 복제 확인
- [ ] 서버 오류 없음 확인

산출물:

```text
DSMinRun.md 최종 테스트 결과 기록
```

완료 기준:

```text
DSMinRun.md의 필수 완료 기준을 모두 통과한다.
```

---

### T-009 현재 구현 기능 안정화 결과 정리

목적:

```text
현재 안정화 단계에서 확인한 내용과 남은 문제를 정리한다.
```

작업:

- [ ] 통과한 현재 구현 기능 기록
- [ ] 수정한 현재 구현 기능 기록
- [ ] 아직 위험하지만 보류한 현재 구현 기능 기록
- [ ] 현재 구현되지 않아 작업하지 않은 기능 기록
- [ ] 다음 확장 후보는 안정화 완료 후 별도 문서에서 논의

산출물:

```text
FeatureList.md 최종 상태 갱신
NetAudit.md 해결/보류 기록 갱신
DSMinRun.md 최종 테스트 결과 갱신
```

완료 기준:

```text
현재 구현 기능 안정화 단계의 결과가 문서화되어 있다.
```

---

## 4. 현재 안정화 완료 게이트

현재 구현 기능 안정화 단계는 아래 조건을 만족하면 완료로 본다.

```text
[필수]
- T-001 완료
- T-002 완료
- T-003 완료
- T-005 완료
- T-006 완료
- T-007 완료
- T-008 완료

[정리]
- T-009 완료
```

예외:

```text
현재 구현되지 않은 기능은 완료 기준에 포함하지 않는다.
안정화 중 새 시스템을 만들 필요가 생기면 현재 단계에서 멈추고 별도 확장 계획으로 분리한다.
```

---

## 5. 커밋 단위 제안

실제 Git 커밋은 아래처럼 작게 나누는 것을 권장한다.

```text
Commit 1:
- MP_ServerPlan 문서 추가

Commit 2:
- Dedicated Server Target 확인 또는 추가

Commit 3:
- 테스트 맵 / GameMode 설정 정리

Commit 4:
- 차량 Pawn 소유권 점검 로그 추가

Commit 5:
- UI / 카메라 서버 분리 1차

Commit 6:
- 차량 이동 복제 확인 또는 수정

Commit 7:
- DS 최소 실행선 테스트 결과 기록

Commit 8:
- 현재 구현 기능 안정화 결과 기록
```

---

## 6. 현재 보류 작업

```text
- 체력 시스템 구현
- 스코어 시스템 구현
- 킬/데스 구현
- 리스폰 구현
- Steam 세션
- 로비 UI
- 매치메이킹
- 전용 서버 배포 자동화
```

보류 이유:

```text
현재 단계에서는 Dedicated Server 최소 실행선과 기존 기능 멀티 안전화가 우선이다.
```

---

## 7. 변경 기록

### 0.1.0

```text
- 다음 단계 작업 분해표 최초 작성
- T-001부터 T-011까지 작업 정의
- 무기 시스템 착수 게이트 정의
```
