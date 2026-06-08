# CarFight — 03_FeatureQueue

> 문서 버전: v1.0.0  
> 작성일(Asia/Seoul): 2026-06-02  
> 문서 상태: Active  
> 역할: CarFight의 **기능 후보 / 착수 판단 / 클라이언트·서버·관리툴 필요성**을 한 곳에서 관리한다.

---

## 1. 목적

이 문서는 CarFight에서 앞으로 개발할 기능 후보를 기능 단위로 정리한다.

이 문서는 상세 설계서가 아니다.
상세 설계는 기능 착수 후 `Document/Plan/<기능명>/`에 작성한다.

이 문서의 목적은 아래 판단을 돕는 것이다.

```text
- 다음에 어떤 기능을 착수할지
- 이 기능이 클라이언트만으로 검증 가능한지
- 서버 권한 구조가 필요한지
- 관리툴 또는 임시 운영 도구가 필요한지
- 완료 후 어떤 Systems 문서로 승격할지
```

---

## 2. 문서 위치와 생명주기

CarFight 문서 흐름은 아래 기준으로 본다.

```text
기능 후보
  -> Document/ProjectSSOT/03_FeatureQueue.md

개발 착수 / 상세 계획
  -> Document/Plan/<기능명>/

개발 완료 / 현재 구현 기록
  -> Document/Systems/<분류>/<기능명>.md

오래된 계획 / 완료된 계획 원본
  -> Document/Plan/Archive/
```

중요:
- `FeatureQueue`는 상세 구현 계획을 쓰지 않는다.
- `Plan`은 앞으로 개발할 기능의 상세 계획만 담는다.
- `Systems`는 개발 완료된 기능의 현재 구현 기준만 담는다.

---

## 3. 상태 표기

| 상태 | 의미 |
|---|---|
| `Candidate` | 후보. 아직 착수하지 않음 |
| `Ready` | 착수 가능. Plan 생성 후보 |
| `Active` | 현재 Plan에서 진행 중 |
| `Blocked` | 선행 조건 때문에 막힘 |
| `Done` | 구현 완료. Systems 문서로 승격됨 |
| `Deferred` | 보류 |
| `Rejected` | 기각 |

---

## 4. 우선순위 표기

| 우선순위 | 의미 | 처리 기준 |
|---|---|---|
| `P0` | 없으면 다음 개발/검증이 막힘 | 최우선 착수 후보 |
| `P1` | 핵심 게임 루프에 필요 | 가까운 사이클에서 착수 |
| `P2` | 생산성/운영/품질 개선 | 반복 비용이 커질 때 착수 |
| `P3` | 편의/폴리싱 | 핵심 루프 이후 |
| `Icebox` | 아이디어 보관 | 당장 계획하지 않음 |

---

## 5. 기능 후보 큐

| ID | 기능 | 목적 | 우선순위 | 상태 | 클라 | 서버 | 관리툴 | 완료 후 Systems 위치 |
|---|---|---|---|---|---|---|---|---|
| `CF-FQ-001` | 서버 권한 발사 요청 | 2클라 환경에서 발사 요청을 서버 권한 구조로 통과시키기 | `P0` | `Ready` | 필요 | 필요 | 불필요 | `Document/Systems/Network/ServerFire.md` 또는 `Document/Systems/Combat/Fire.md` |
| `CF-FQ-002` | 조준/발사 피드백 분리 | 서버 판정과 로컬 조준/이펙트/Reticle 피드백 책임 분리 | `P0` | `Ready` | 필요 | 필요 | 불필요 | `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-003` | 체력/대미지 최소 구조 | 차량 전투의 피해 판정과 생존 상태 기반 만들기 | `P1` | `Candidate` | 필요 | 필요 | 낮음 | `Document/Systems/Combat/Damage.md` |
| `CF-FQ-004` | 리스폰 최소 구조 | 2클라 전투 테스트 반복 가능 상태 만들기 | `P1` | `Candidate` | 필요 | 필요 | 낮음 | `Document/Systems/Network/Respawn.md` |
| `CF-FQ-005` | 전투 결과 기록 | 매치 종료/승패/기본 결과 기록 기반 만들기 | `P1` | `Candidate` | 필요 | 필요 | 중간 | `Document/Systems/Combat/MatchResult.md` |
| `CF-FQ-006` | 테스트 계정/상태 초기화 도구 | 반복 테스트 준비 비용 줄이기 | `P2` | `Candidate` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/TestReset.md` |
| `CF-FQ-007` | 차량 로드아웃 저장 | 차량/무장 장착 상태를 재접속 후 유지 | `P1` | `Candidate` | 필요 | 필요 | 중간 | `Document/Systems/Data/VehicleLoadout.md` |
| `CF-FQ-008` | 무장 데이터 정의 | 차량 장착형 터렛/무기 데이터 기준 만들기 | `P1` | `Candidate` | 필요 | 낮음 | 낮음 | `Document/Systems/Data/WeaponData.md` |
| `CF-FQ-009` | 운영 로그 조회 기준 | 서버 전투/스폰/에러 로그를 추적 가능한 형태로 정리 | `P2` | `Candidate` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/LogView.md` |
| `CF-FQ-010` | 세션/로비 기초 | Dedicated Server 이후 접속 흐름 확장 | `P2` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Network/Session.md` |

---

## 6. 현재 최우선 착수 후보

현재 기준 최우선 후보는 아래 두 개다.

```text
1. CF-FQ-001 서버 권한 발사 요청
2. CF-FQ-002 조준/발사 피드백 분리
```

이유:
- `Document/Systems/Network/ServerSpawn.md` 기준으로 Dedicated Server 접속/Spawn/Possess 기반은 생겼다.
- 다음 병목은 차량을 조작하는 수준을 넘어, 전투 입력을 서버 권한 구조로 통과시키는 것이다.
- CarFight 전투 방향상 조준/발사 피드백은 클라이언트 감각과 서버 판정 책임을 분리해야 한다.

---

## 7. 기능을 Plan으로 승격하는 기준

기능을 `Document/Plan/<기능명>/`로 승격하려면 아래 조건을 만족해야 한다.

```text
- 목적이 한 문장으로 설명된다.
- 클라이언트 / 서버 / 관리툴 필요성이 1차로 판단됐다.
- 완료 후 Systems 문서 위치가 정해졌다.
- 이번 범위에서 제외할 항목이 정해졌다.
- 선행 Systems 문서가 무엇인지 확인됐다.
```

승격 시 `Plan` 폴더에는 아래 문서 구성을 권장한다.

```text
Document/Plan/<기능명>/README.md
Document/Plan/<기능명>/Scope.md
Document/Plan/<기능명>/CurrentState.md
Document/Plan/<기능명>/Design.md
Document/Plan/<기능명>/TaskList.md
Document/Plan/<기능명>/TestPlan.md
Document/Plan/<기능명>/DecisionLog.md
```

단, 기능 규모가 작으면 `README.md`, `TaskList.md`, `TestPlan.md`만으로 줄일 수 있다.

---

## 8. 관리툴 후보 큐

관리툴은 너무 빨리 만들지 않는다.
하지만 같은 운영 작업이 3회 이상 반복될 것 같으면 후보로 등록한다.

| ID | 후보 | 발생 조건 | 임시 대체 | 정식화 시점 | 상태 |
|---|---|---|---|---|---|
| `CF-ADM-001` | 테스트 계정/상태 초기화 | 같은 테스트 세팅을 3회 이상 반복 | 콘솔 명령 / 임시 서버 명령 | 계정/로드아웃/인벤토리 도입 후 | `Candidate` |
| `CF-ADM-002` | 유저 차량 상태 조회 | 차량 소유/로드아웃 저장 도입 | 로그 / DB 직접 조회 | 로드아웃 서버 저장 후 | `Candidate` |
| `CF-ADM-003` | 보상 지급/회수 | 전투 결과와 보상 도입 | 수동 DB 수정 | MatchResult/RewardLog 도입 후 | `Candidate` |
| `CF-ADM-004` | 전투 로그 조회 | 서버 판정 전투 도입 | 로그 파일 검색 | MatchResult 저장 후 | `Candidate` |
| `CF-ADM-005` | 서버 상태 확인 | 장시간 Dedicated Server 테스트 반복 | 콘솔 로그 확인 | 테스트 서버 상시 운용 전 | `Candidate` |

현재 관리툴 원칙:

```text
1. 처음부터 웹 관리툴을 만들지 않는다.
2. 먼저 콘솔 명령 / CLI / 임시 서버 명령으로 반복 작업을 줄인다.
3. 데이터 구조가 안정화된 뒤 정식 관리툴로 승격한다.
4. 관리툴 기능도 완료되면 Document/Systems/Admin/ 아래에 기록한다.
```

---

## 9. 기능 큐 갱신 조건

아래 상황이 발생하면 이 문서를 갱신한다.

```text
- 새 기능 후보가 생김
- 기능 우선순위가 바뀜
- 기능이 Plan으로 승격됨
- 기능이 완료되어 Systems로 승격됨
- 관리툴 후보가 생김
- 기능이 보류/기각됨
```

---

## 10. 문서 버전 관리

- 현재 문서 버전: `v1.0.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 기능 큐 운영 방식 자체 변경 |
| Minor | 기능 후보/관리툴 후보/승격 기준 추가 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 11. 체인지로그

### v1.0.0 - 2026-06-02

```text
- FeatureQueue 문서 최초 작성
- 기능 후보 큐 작성
- Plan 승격 기준 정의
- Systems 승격 위치 개념 정의
- 관리툴 후보 큐 추가
```
