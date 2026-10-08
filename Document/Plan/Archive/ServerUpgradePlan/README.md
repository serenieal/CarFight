# ServerUpgradePlan 문서 인덱스

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 위치: `Document/ProjectSSOT/Plan/ServerUpgradePlan/`
- 상태: Active

---

## 1. 목적

이 폴더는 CarFight의 서버 상태를 현재 클라이언트 개발 수준에 맞춰 끌어올리기 위한 계획, 설계, 단계별 로드맵을 관리한다.

현재 서버는 Dedicated Server 빌드와 실행은 가능하지만, 아직 게임 서버로서 필요한 Spawn, Possess, OwnerOnly 입력, 이동 복제, 서버 전용 실행 안정화가 완료되지 않았다.

이 폴더의 목표는 다음 하나다.

```text
현재 구현된 클라이언트 차량 기능을 Dedicated Server 기준에서 안정적으로 동작하게 만든다.
```

---

## 2. 기존 문서와의 관계

### 2.1 기존 `MP_ServerPlan`

`Document/ProjectSSOT/Plan/MP_ServerPlan/`은 Dedicated Server 최소 실행선과 초기 조사 기록을 담고 있다.

역할:

```text
- Dedicated Server 빌드/실행 검증 기록
- 2클라 접속 전 점검 기록
- Play As Client / Listen Server / open 접속 관찰 기록
- 최소 Spawn/Possess 필요성 도출
```

### 2.2 새 `ServerUpgradePlan`

이 폴더는 위 조사 결과를 바탕으로 실제 서버 고도화 작업을 단계별로 진행하기 위한 실행 계획이다.

역할:

```text
- 서버와 클라이언트 현재 상태 비교
- 서버 책임 구조 설계
- GameMode / PlayerController / Pawn 소유권 작업 계획
- 이동 복제와 OwnerOnly 입력 검증 계획
- Dedicated Server 금지 코드 정리 계획
- 단계별 완료 기준 관리
```

---

## 3. 문서 목록

| 문서 | 역할 |
|---|---|
| `Scope.md` | 이번 서버 고도화 작업의 포함/제외 범위 고정 |
| `CurrentState.md` | 현재 서버/클라이언트 상태와 격차 정리 |
| `ServerDesign.md` | 서버 책임 구조와 최소 GameMode 설계 |
| `Roadmap.md` | 단계별 작업 순서와 완료 기준 |
| `TaskList.md` | 실제 작업 단위 체크리스트 |
| `TestPlan.md` | PIE / Dedicated Server 검증 절차 |
| `DecisionLog.md` | 결정 사항과 변경 이력 기록 |

---

## 4. 권장 읽기 순서

처음 읽을 때는 아래 순서를 따른다.

```text
1. README.md
2. Scope.md
3. CurrentState.md
4. ServerDesign.md
5. Roadmap.md
6. TaskList.md
7. TestPlan.md
8. DecisionLog.md
```

실제 작업 중에는 아래 순서로 사용한다.

```text
1. Scope.md로 범위 이탈 여부 확인
2. CurrentState.md로 현재 병목 확인
3. ServerDesign.md로 구현 방향 확인
4. TaskList.md에서 현재 작업 선택
5. Roadmap.md의 완료 기준으로 단계 종료 판단
6. TestPlan.md로 검증
7. DecisionLog.md에 결정 기록
```

---

## 5. 현재 최우선 결론

현재 서버 고도화의 첫 병목은 다음이다.

```text
멀티플레이용 최소 Spawn/Possess 구조가 없다.
```

따라서 첫 코드 작업 후보는 다음으로 고정한다.

```text
ACFMPGameMode 기반 최소 서버 Spawn/Possess 구조 작성
```

---

## 6. 현재 비목표

이번 단계에서는 아래 기능을 새로 만들지 않는다.

```text
- 무기 시스템 완성
- 체력 시스템
- 대미지 시스템
- 스코어 시스템
- 킬/데스
- 리스폰
- 로비
- Steam 세션
- 매치메이킹
- 서버 브라우저
- 서버 배포 자동화
```

위 기능들은 현재 서버 안정화 이후 확장 단계에서 다시 계획한다.

---

## 7. 운영 원칙

```text
- 클라이언트 기능을 서버에 억지로 옮기지 않는다.
- 서버는 Spawn, Possess, 실제 상태 권한, 복제 기준을 담당한다.
- 클라이언트는 입력, 카메라, UI, 사운드, 개인 피드백을 담당한다.
- Dedicated Server에서 Viewport, LocalPlayer, Camera, HUD, 사운드 코드를 실행하지 않는다.
- 새 시스템보다 현재 구현 기능 안정화를 우선한다.
```

---

## 8. 변경 기록

### v0.1.0

```text
- ServerUpgradePlan 폴더 인덱스 최초 작성
- 기존 MP_ServerPlan과의 역할 분리 정의
- 서버 고도화 첫 병목을 Spawn/Possess 구조 부재로 고정
- 신규 문서 목록과 읽기 순서 정의
```
