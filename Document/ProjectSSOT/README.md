# ProjectSSOT 운영 가이드 (CarFight)

> 문서 버전: v2.9.0
> 마지막 정리(Asia/Seoul): 2026-08-22
> 문서 상태: Current
> 역할: `Document/ProjectSSOT/`의 읽기 순서, 문서 역할과 생명주기를 고정한다.

---

## 1. 목적

`Document/ProjectSSOT/`는 CarFight의 **프로젝트 전용 현재 판단 기준**을 유지한다.

이 폴더는 다음 질문에 빠르게 답할 수 있어야 한다.

```text
- 이 프로젝트가 어디로 가는가?
- 지금 실제로 어디까지 구현되어 있는가?
- 현재 Active는 무엇인가?
- 다음에 선택할 수 있는 작업은 무엇인가?
- 어떤 프로젝트 수준 결정이 확정되어 있는가?
- 완료된 기능의 현재 구현은 어디서 확인하는가?
```

공통 규칙 원본은 `Document/SSOT/`에 두고, CarFight에만 필요한 판단은 `Document/ProjectSSOT/`에 둔다.

---

## 2. 문서 영역 책임

| 위치 | 책임 |
| --- | --- |
| `Document/ActiveWork.md` | 현재 Active/Paused 작업의 세션 복원 projection과 대표 Plan 연결 |
| `Document/ProjectSSOT/` | 프로젝트 방향, 현재 상태, 우선순위, 확정 결정, 회귀 기준 |
| `Document/Plan/` | 착수된 기능의 상세 구현·검증 계획, 체크포인트, evidence |
| `Document/Systems/` | 검증된 현재 구현 구조와 책임 |
| `Document/DesignSource/` | 원본 기획과 장기 방향 |
| `Document/SSOT/` | 여러 프로젝트에 공통 적용되는 기준 |
| `Document/ProjectSSOT/Archive/` | 현재 프로젝트 판단에서 내려온 역사 기록 |
| `Document/Plan/Archive/` | 완료·보류·대체된 Historical Plan |

핵심 경계는 다음과 같다.

```text
ProjectSSOT = 무엇을 왜 개발하는가
ActiveWork = 지금 무엇을 복원하거나 선택하는가
Plan = 선택된 작업을 어떻게 구현하고 검증하는가
Systems = 현재 실제로 어떻게 구현되어 있는가
Archive = 현재 기준에서 내려온 기록을 보존한다
Git = 문서 자체의 변경 역사를 보존한다
```

현재 구현을 판단할 때는 실제 Git/code/asset과 `Document/Systems/`를 우선한다. Plan이나 과거 ProjectSSOT 기록이 더 길거나 최근에 수정되었다는 이유만으로 현재 구현을 대체하지 않는다.

---

## 3. 현재 프로젝트 운용 기준

CarFight의 현재 개발 선로는 **싱글 플레이 기준 차량 전투 게임을 먼저 완성하는 것**이다.

서버 권한 전투, Dedicated Server 검증, 2클라이언트 테스트, 세션/로비와 서버 운영 기능은 삭제하지 않지만 현재 개발 일정에서는 Deferred/Icebox로 취급한다.

프로젝트 전역 사운드 정책은 `04_ProjectDecisions.md`의 `CF-PDL-0009`를 따른다. CarFight는 현재 게임 사운드를 구현·제공하지 않으며 사운드는 기능 완료 조건에 포함하지 않는다.

현재 Feature 상태와 다음 선택지는 `01_ProjectState.md`, `02_Roadmap.md`, `03_FeatureQueue.md`를 본다. 완료 기능의 실제 구현은 `Document/Systems/SystemIndex.md`에서 해당 Current System으로 이동한다.

---

## 4. ProjectSSOT 활성 문서 최소 집합

`Document/ProjectSSOT/` 루트의 Current 문서는 `README.md`와 아래 6개로 제한한다.

| 순서 | 문서 | 역할 |
| ---: | --- | --- |
| 0 | `00_Vision.md` | 최종 방향과 장기 구조 원칙 |
| 1 | `01_ProjectState.md` | 현재 실제 기준선, 현재 Feature 상태, 현재 리스크 |
| 2 | `02_Roadmap.md` | 현재 사이클의 진행 순서와 선택 가능한 다음 작업 |
| 3 | `03_FeatureQueue.md` | 전체 Feature 후보와 상태 |
| 4 | `04_ProjectDecisions.md` | 프로젝트 전체 확정 결정 로그 |
| 5 | `05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 |

루트에 새 문서를 추가하기 전에 기존 00~05 문서, Plan, Systems 또는 Archive가 적절한 소유자인지 먼저 판단한다.

---

## 5. 읽기 순서

프로젝트 전체 상태를 처음 복원할 때:

```text
1. Document/ProjectSSOT/README.md
2. Document/ProjectSSOT/01_ProjectState.md
3. Document/ProjectSSOT/02_Roadmap.md
4. Document/ProjectSSOT/03_FeatureQueue.md
```

장기 방향이 필요한 경우 `00_Vision.md`와 `DesignSource`를 추가로 읽는다.

현재 구현 상세가 필요한 경우 `Document/Systems/SystemIndex.md`에서 관련 Systems 문서만 읽는다.

특정 작업을 재개하는 경우 `Document/ActiveWork.md`에서 대표 Plan과 next gate를 찾은 뒤 해당 Plan과 관련 Systems만 읽는다. Plan 전체와 Archive 전체를 기본 입력으로 재귀 탐색하지 않는다.

---

## 6. Feature와 문서 생명주기

### 6.1 Candidate

Feature 후보는 `03_FeatureQueue.md`에 한 행으로 등록한다. 이 단계에서는 상세 구현 로그나 Build evidence를 기록하지 않는다.

### 6.2 Ready / Active / Paused

착수된 Feature의 상세 설계, 구현 체크포인트, 빌드·Automation·PIE·USER evidence는 대표 Plan이 소유한다.

`ActiveWork.md`에는 다음만 projection한다.

```text
Feature ID / 상태
대표 Plan
현재 또는 다음 Gate
반복하면 안 되는 대표 완료 Gate
현재 구현 owner 또는 주요 dependency
```

### 6.3 Done

Feature가 Done이 되면:

```text
대표 Plan = 구현 과정과 검증 evidence 보존
Systems = 현재 구현 계약으로 승격 또는 기존 Current System 갱신
FeatureQueue = 상태를 Done으로 변경
ActiveWork = 상세 로그 제거, 필요하면 최근 완료 1건만 짧게 유지
ProjectState/Roadmap = 프로젝트 수준 상태나 순서가 실제로 바뀐 경우에만 갱신
```

Done이라고 해서 대표 Plan의 evidence를 삭제하지 않는다. 반대로 Build ID, Automation run history, USER 조작 로그를 Current projection 문서에 계속 복제하지 않는다.

### 6.4 Historical

현재 판단 기준이 아닌 과거 상태는 Archive 또는 Git history가 보존한다. 과거의 `현재 Active`, 오래된 Next Gate와 stale version pointer를 Current 문서 안에 Historical 섹션으로 계속 누적하지 않는다.

---

## 7. 비파괴 문서 정리 원칙

문서 정리는 **정보 삭제가 아니라 소유권 정상화**를 기본으로 한다.

Current 문서에서 상세 내용을 제거하기 전에 다음 중 하나의 authoritative owner가 존재하는지 확인한다.

```text
현재 구현 계약 → Systems
상세 구현·검증·USER Acceptance → 대표 Plan
프로젝트 확정 결정 → ProjectDecisions
과거 문서 변경 이력 → Git / Archive
```

owner가 없는 고유 정보는 단순 압축을 이유로 삭제하지 않는다.

정리 전에는 답할 수 있었는데 정리 후에는 답할 수 없는 프로젝트 관련 질문이 생기면 정리 실패로 판정한다. 답을 찾는 경로가 더 명확하고 짧아지는 것은 정상적인 개선이다.

---

## 8. Current 문서 건강 기준

Current projection 문서는 시간이 지날수록 과거 로그를 붙여넣는 방식으로 성장시키지 않는다.

다음 신호가 나타나면 짧은 문서 Health Check를 수행한다.

```text
- ActiveWork에 완료 Feature의 상세 Build/Automation/USER 로그가 누적된다.
- FeatureQueue가 상세 구현 설계서처럼 변한다.
- ProjectState 또는 Roadmap에 서로 다른 세대의 "현재 Active"가 둘 이상 존재한다.
- 같은 상태가 ProjectState/Roadmap/FeatureQueue/ActiveWork에서 서로 다르다.
- Current 문서가 오래된 Plan/System 버전을 계속 가리킨다.
```

Health Check는 일정 주기로 강제하지 않는다. 큰 Feature 여러 개를 닫았거나 위 신호가 발생했을 때 수행하며, 정상적인 Feature 종료 절차에서 작은 정리를 함께 처리하는 것을 기본으로 한다.

---

## 9. Changelog

### v2.9.0 - 2026-08-22

- 2026-06~07의 과거 "현재 개발 기준"과 완료 Feature 세부 로그를 Current 운영 가이드에서 제거하고 문서 역할 중심으로 정상화했다.
- `Current → Plan/Systems → Archive/Git` 정보 소유권과 비파괴 정리 원칙을 명시했다.
- Feature 종료 시 ActiveWork/FeatureQueue에 상세 evidence가 누적되지 않도록 Lifecycle 규칙을 고정했다.
- 정기적인 대청소 대신 종료 시 소규모 정리와 신호 기반 Health Check를 기본 운영 방식으로 정했다.

Migration: 기존 ProjectSSOT/Plan/Systems/Archive 경로는 변경하지 않는다. 과거 Current 상태와 상세 evidence는 기존 대표 Plan, Systems, Archive와 Git history에서 계속 조회한다.
