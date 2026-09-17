# ProjectSSOT 운영 가이드 (CarFight)

> 문서 버전: v2.10.1
> 마지막 정리(Asia/Seoul): 2026-09-17
> 문서 상태: Current
> 역할: `Document/ProjectSSOT/`의 읽기 순서, 문서 역할과 생명주기를 고정한다.

---

## 1. 목적

`Document/ProjectSSOT/`는 CarFight의 **프로젝트 전용 현재 판단 기준**을 유지한다.

이 폴더는 다음 질문에 빠르게 답할 수 있어야 한다.

```text
- 이 프로젝트가 어디로 가는가?
- 지금 실제로 어디까지 구현되어 있는가?
- 현재 Active/Ready Work는 어디에서 canonical하게 확인하는가?
- 다음에 선택할 수 있는 Feature 후보와 순서는 무엇인가?
- 어떤 프로젝트 수준 결정이 확정되어 있는가?
- 완료된 기능의 현재 구현은 어디서 확인하는가?
```

공통 규칙 원본은 `Document/SSOT/`에 두고, CarFight에만 필요한 판단은 `Document/ProjectSSOT/`에 둔다.

---

## 2. 문서 영역 책임

| 위치 | 책임 |
| --- | --- |
| `Document/UDS/records/**` | UDS로 승격된 Active/Ready Work lifecycle identity/state/phase/next의 canonical authority |
| `Document/UDS/derived/Current.md` | canonical Work에서 재생성하는 bounded 세션 복원 view; authority0 |
| `Document/ActiveWork.md` | MIG-05 이전 세션 복원 projection의 retained snapshot; authority0/frozen |
| `Document/ProjectSSOT/` | 프로젝트 방향, 기준선, planning 우선순위, 확정 결정, 회귀 기준 |
| `Document/Plan/` | 착수된 기능의 상세 구현·검증 계획, 체크포인트, evidence |
| `Document/Systems/` | 검증된 현재 구현 구조와 책임 |
| `Document/DesignSource/` | 원본 기획과 장기 방향 |
| `Document/SSOT/` | 여러 프로젝트에 공통 적용되는 기준 |
| `Document/ProjectSSOT/Archive/` | 현재 프로젝트 판단에서 내려온 역사 기록 |
| `Document/Plan/Archive/` | 완료·보류·대체된 Historical Plan |

핵심 경계는 다음과 같다.

```text
ProjectSSOT = 무엇을 왜 개발하고 어떤 후보·순서로 갈 것인가
UDS Work = 지금 승격된 Work의 canonical lifecycle state/phase/next
UDS Current = bounded session restore view; authority0
ActiveWork = pre-cutover retained snapshot; authority0/frozen
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

현재 승격 Work의 lifecycle은 `Document/UDS/derived/Current.md`에서 찾고 canonical `Document/UDS/records/**`로 확인한다. 다음 Feature 후보·우선순위·cycle 순서는 `02_Roadmap.md`, `03_FeatureQueue.md`가 소유하며, `01_ProjectState.md`는 프로젝트 기준선과 리스크를 유지한다. 완료 기능의 실제 구현은 `Document/Systems/SystemIndex.md`에서 해당 Current System으로 이동한다.

---

## 4. ProjectSSOT 활성 문서 최소 집합

`Document/ProjectSSOT/` 루트의 Current 문서는 `README.md`와 아래 6개로 제한한다.

| 순서 | 문서 | 역할 |
| ---: | --- | --- |
| 0 | `00_Vision.md` | 최종 방향과 장기 구조 원칙 |
| 1 | `01_ProjectState.md` | 프로젝트 전역 기준선, 현재 리스크와 전역 결정; Work lifecycle 목록은 소유하지 않음 |
| 2 | `02_Roadmap.md` | 현재 사이클 목표, 거시 진행 순서, dependency와 Candidate ordering |
| 3 | `03_FeatureQueue.md` | Feature 후보·우선순위·착수 판단·UDS promotion pointer·완료 후 Current owner |
| 4 | `04_ProjectDecisions.md` | 프로젝트 전체 확정 결정 로그 |
| 5 | `05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 |

루트에 새 문서를 추가하기 전에 기존 00~05 문서, Plan, Systems 또는 Archive가 적절한 소유자인지 먼저 판단한다.

---

## 5. 읽기 순서

승격된 특정 Work를 재개할 때의 기본 복원 경로는 다음으로 제한한다.

```text
1. Document/UDS/derived/Current.md를 탐색 힌트로 사용
2. records/work/** fresh head-set 검증 후 선택한 canonical Work record 확인
3. canonical Work가 가리키는 representative Plan 확인
4. 작업에 실제 필요한 Systems / Source / Asset만 확인
```

프로젝트 전역 기준선·리스크 판단이 필요할 때만 `01_ProjectState.md`, 거시 순서·dependency 판단이 필요할 때만 `02_Roadmap.md`, 승격 전 Candidate·우선순위·착수 판단이 필요할 때만 `03_FeatureQueue.md`를 추가로 읽는다. 장기 방향이 필요한 경우 `00_Vision.md`와 `DesignSource`를 추가한다.

`Document/ActiveWork.md`는 Current 복원에 사용하지 않는다. ProjectSSOT 전체, Plan Index 전체와 Archive 전체를 기본 입력으로 재귀 탐색하지 않는다.

---

## 6. Feature와 문서 생명주기

### 6.1 Candidate

Feature 후보는 `03_FeatureQueue.md`에 한 행으로 등록한다. 이 단계에서는 상세 구현 로그나 Build evidence를 기록하지 않는다.

### 6.2 Ready / Active / Paused

UDS로 승격된 Feature/Work의 lifecycle identity/state/phase/next는 canonical `Document/UDS/records/**`가 소유한다. 착수된 Feature의 상세 설계, 구현 체크포인트, 빌드·Automation·PIE·USER evidence는 representative Plan이 소유한다.

`Document/UDS/derived/Current.md`는 필요한 최소 restore fields만 projection하며 authority0다. `ActiveWork.md`에는 새 Current를 기록하지 않는다.

### 6.3 Done

Feature가 Done이 되면:

```text
대표 Plan = 구현 과정과 검증 evidence 보존
Systems = 현재 구현 계약으로 승격 또는 기존 Current System 갱신
UDS Work = immutable successor로 closed/retired state에 수렴
UDS Current/head = canonical record에서 authority0로 재생성
FeatureQueue = planning catalog에서 완료 disposition / Current owner 정리
ProjectState/Roadmap = 프로젝트 수준 기준선이나 순서가 실제로 바뀐 경우에만 갱신
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
- authority0 UDS Current/head projection이 canonical records와 달라진다.
- ActiveWork에 cutover 이후 새 Current 상태가 추가된다.
- FeatureQueue가 상세 구현 설계서 또는 promoted Work lifecycle second authority처럼 변한다.
- ProjectState 또는 Roadmap이 UDS와 독립적으로 current Work lifecycle을 주장한다.
- 같은 lifecycle fact가 UDS와 Legacy surface에서 서로 다르다.
- Current 문서가 오래된 Plan/System 버전을 계속 가리킨다.
```

Health Check는 일정 주기로 강제하지 않는다. 큰 Feature 여러 개를 닫았거나 위 신호가 발생했을 때 수행하며, 정상적인 Feature 종료 절차에서 작은 정리를 함께 처리하는 것을 기본으로 한다.

---

## 9. Changelog

### v2.10.1 - 2026-09-17

- ProjectState를 전역 기준선·리스크·전역 결정, Roadmap을 macro sequence/dependency/Candidate ordering, FeatureQueue를 후보·우선순위·착수·promotion pointer·closure owner로 역할 축소했다.
- 승격된 Work의 기본 복원 경로를 `UDS Current hint → fresh canonical Work → representative Plan → 필요한 Systems/Source/Asset`로 고정했다.
- ProjectState/Roadmap/FeatureQueue는 해당 판단이 실제로 필요한 경우에만 추가 읽도록 하여 Current session restore의 토큰 비용과 stale projection 노출을 줄였다.
- `ActiveWork.md`, ProjectSSOT 전체, Plan Index 전체와 Archive 전체를 기본 복원 입력에서 명시적으로 제외했다.

### v2.10.0 - 2026-09-16

- UDS-08 MIG-05 permanent adoption으로 UDS 승격 Work의 current lifecycle authority를 immutable `Document/UDS/records/**`로 전환했다.
- `UDS/derived/Current.md`를 authority0 bounded restore view로 연결하고 `ActiveWork.md`를 pre-cutover retained frozen snapshot으로 retirement했다.
- ProjectSSOT는 프로젝트 기준선/planning, FeatureQueue는 후보·우선순위·착수 판단, Roadmap은 cycle/sequence, representative Plan은 detailed checkpoint/evidence를 계속 소유하도록 경계를 정리했다.
- Current lifecycle dual-write와 stale Legacy projection을 Health Check failure signal로 추가했다.

### v2.9.0 - 2026-08-22

- 2026-06~07의 과거 "현재 개발 기준"과 완료 Feature 세부 로그를 Current 운영 가이드에서 제거하고 문서 역할 중심으로 정상화했다.
- `Current → Plan/Systems → Archive/Git` 정보 소유권과 비파괴 정리 원칙을 명시했다.
- Feature 종료 시 ActiveWork/FeatureQueue에 상세 evidence가 누적되지 않도록 Lifecycle 규칙을 고정했다.
- 정기적인 대청소 대신 종료 시 소규모 정리와 신호 기반 Health Check를 기본 운영 방식으로 정했다.

Migration: 기존 ProjectSSOT/Plan/Systems/Archive 경로는 변경하지 않는다. 과거 Current 상태와 상세 evidence는 기존 대표 Plan, Systems, Archive와 Git history에서 계속 조회한다.
