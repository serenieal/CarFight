# Document Entry (CarFight)

- 문서 버전: v2.11
- 작성일: 2026-06-19
- 최근 갱신일: 2026-08-19
- 문서 상태: Current
- 역할: `Document/` 전체의 작업별 진입 라우터

---

## 1. 목적

이 문서는 `Document/`에 있는 모든 파일을 나열하는 목록이 아니다.

사용자, AI, Codex가 작업을 시작할 때 다음을 결정하도록 돕는 총괄 진입점이다.

```text
1. 이번 작업은 어떤 종류인가
2. 어느 색인 문서를 먼저 읽어야 하는가
3. 추가로 어떤 문서만 선택해서 읽어야 하는가
4. 어떤 경로는 기본 검색에서 제외해야 하는가
```

전체 `Document/`를 먼저 재귀 탐색하지 않는다.
이 문서에서 작업 종류를 판별한 뒤 필요한 색인과 대표 문서만 선택한다.

---

## 2. 문서 영역 역할

| 위치 | 역할 | 현재 기준 여부 |
| --- | --- | --- |
| `Document/DesignSource/` | 장기 방향과 원본 기획 참고 | 북극성 참고 자료 |
| `Document/CodeWorkGate.md` | 현재 AI 세션의 직접 코드 구현, 변경 보호, 검증과 완료 판정 통제 | 코드 작업 실행 최우선 기준 |
| `Document/ActiveWork.md` | 현재 활성 작업, 마지막 작업 초점과 대표 체크포인트 연결 | 세션 복원 기준 |
| `Document/ProjectSSOT/` | CarFight의 현재 상태, 우선순위, 확정 결정, 검증 기준 | 프로젝트 판단 기준 |
| `Document/Systems/` | 구현 완료된 기능의 현재 구조와 책임 | 현재 구현 기준 |
| `Document/Plan/` | 앞으로 개발하거나 검증할 기능의 상세 계획 | 진행 중 계획 기준 |
| `Document/Plan/Archive/` | 현재 착수 기준에서 내려온 Historical Plan의 logical 색인과 물리 Archive | 현재 착수 기준 아님 |
| `Document/SSOT/` | 여러 프로젝트에 공통 적용되는 규칙과 UE 협업 원칙 | 공용 기준 |

핵심 구분:

```text
ProjectSSOT = 무엇을 왜 개발하는가
Plan = 앞으로 무엇을 어떻게 개발하는가
Systems = 현재 실제로 어떻게 구현되어 있는가
Archive = 더 이상 현재 착수 기준이 아닌 기록
DesignSource = 장기 방향과 원본 기획
```

Plan이 Systems보다 최근에 수정되었다는 이유만으로 현재 구현 기준으로 사용하지 않는다.

### 2.1 전체 문서 아키텍처 공식 정의

CarFight 문서체계는 다음 계층이 서로 다른 책임을 가지는 구조로 운영한다.

```text
AGENTS
→ CodeWorkGate (실제 코드·설정·스크립트 작업일 때)
→ Document_Entry
→ ActiveWork
→ ProjectSSOT
→ Plan
→ 실제 코드와 에셋
→ Systems
→ Archive
```

`DesignSource`와 공용 `SSOT`는 위 실행 흐름 옆에서 장기 방향과 공통 규칙을 제공한다.
`Link_Audit_Check`는 전체 구조의 링크, 역할과 상태 정합성을 검사한다.

각 계층의 한 줄 책임은 다음과 같다.

```text
AGENTS = 작업 방법과 금지사항을 통제한다.
CodeWorkGate = 현재 AI 세션의 직접 구현, 변경 보호, 검증, 완료 판정과 선택적 외부 위임을 통제한다.
Document_Entry = 작업 종류에 맞는 진입 경로를 선택한다.
ActiveWork = 현재 어떤 활성 작업을 복원하거나 전환할지 선택한다.
ProjectSSOT = 현재 무엇을 왜 해야 하는지 판단한다.
Plan = 선택된 기능을 어떻게 구현하고 검증할지 정의한다.
코드와 에셋 = 실제 구현 그 자체다.
Systems = 구현된 기능이 현재 어떻게 작동하는지 설명한다.
DesignSource = 장기적으로 어디로 갈지 보여준다.
공용 SSOT = 여러 프로젝트에서 공통으로 지킬 규칙을 정의한다.
Archive = 현재 기준에서 내려온 기록을 보존한다.
Link_Audit_Check = 위 관계가 서로 모순되지 않는지 검사한다.
```

### 2.2 문서 영역별 책임과 비책임

| 영역 | 책임 | 이 영역의 책임이 아닌 것 |
| --- | --- | --- |
| `AGENTS.md` | 작업 절차, Git 보호, 빌드·검증, 세션 복원 규칙 | 기능 설계, 현재 구현 설명, 진행률 기록 |
| `CodeWorkGate.md` | 직접 코드 구현의 사전 확인, 변경 보호, 검증, 완료 판정과 선택적 외부 위임 통제 | 개별 기능 설계와 현재 구현 상세 설명 |
| `Document_Entry.md` | 문서 아키텍처와 작업별 라우팅 | 개별 기능 상세 상태와 파일 전체 목록 |
| `ActiveWork.md` | 활성 작업 선택, 마지막 작업 초점, 대표 Plan 연결 | 상세 구현 설계, 장문의 세션 로그, 전체 FeatureQueue 복사 |
| `DesignSource/` | 원본 기획과 장기 북극성 | 현재 착수 승인, 현재 구현 판정 |
| `ProjectSSOT/` | 프로젝트 전용 방향, 상태, 우선순위, 결정, 회귀 기준 | 개별 기능의 상세 구현 절차, 공용 UE 규칙 |
| `ProjectSSOT/CombatPlan/` | 전투 정체성과 장기 도메인 설계 | 현재 기능의 착수 승인과 세부 구현 체크포인트 |
| `Plan/` | 착수된 기능의 상세 설계, 작업 체크포인트, 남은 검증 | 완료된 현재 구현의 영구 기준 |
| 실제 코드와 에셋 | 실제 런타임 동작과 구성 | 프로젝트 방향과 문서 생명주기 정의 |
| `Systems/` | 검증된 현재 구현 구조, 책임, 제한과 데이터 흐름 | 아직 구현되지 않은 미래 설계와 희망 사항 |
| `SSOT/` | 여러 프로젝트에 공통 적용되는 UE·협업 규칙 | CarFight 전용 우선순위와 예외 결정 |
| `Archive/` | 완료·보류·대체된 과거 기록 보존 | 현재 착수 또는 현재 구현 기준 |
| `Link_Audit_Check.md` | 링크, 역할, 버전과 상태 정합성 감사 | 프로젝트 기능 우선순위 결정 |

### 2.3 ActiveWork와 대표 Plan의 책임 경계

```text
ActiveWork
= 현재 실제로 진행 중인 작업을 찾고 대표 Plan으로 연결한다.

대표 Plan
= 그 작업의 완료 범위, 미완료 범위, 빌드 상태, PIE 상태와 바로 다음 작업을 기록한다.
```

`ActiveWork.md`에는 대표 Plan의 상세 내용을 복사하지 않는다.
대표 Plan이 detailed work status owner이고 `ActiveWork.md`와 `Plan/README.md`는 현재 상태를 짧게 보여주는 projection이다.
projection과 대표 Plan이 충돌하면 FeatureQueue·실제 저장소·대표 Plan을 다시 확인하고 projection을 stale로 판정해 교정한다. 최신 수정 시각만으로 projection을 authoritative 상태로 취급하지 않는다.
사용자가 특정 작업명이나 작업 ID를 지정하면 마지막 작업 초점보다 해당 작업을 우선한다.
새 세션은 대표 Plan을 읽은 뒤 관련 Systems, ProjectSSOT와 실제 코드·에셋을 교차검증해야 한다.

### 2.4 CombatPlan과 일반 Plan의 차이

```text
ProjectSSOT/CombatPlan
= CarFight 전투가 장기적으로 어떤 원칙과 정체성을 가져야 하는지 정의한다.

Document/Plan/<Feature>
= FeatureQueue에서 착수가 승인된 특정 기능을 지금 어떻게 구현하고 검증할지 정의한다.
```

CombatPlan에 존재하는 아이디어는 자동으로 현재 구현 대상이 되지 않는다.
현재 착수 여부는 `ProjectSSOT/03_FeatureQueue.md`가 결정하고, 착수 후의 상세 상태는 해당 대표 Plan이 관리한다.

### 2.5 문서 충돌 시 권한 우선순위

현재 실제 구현을 판단할 때는 다음 순서를 사용한다.

```text
1. 현재 Git 상태와 실제 코드·에셋
2. 관련 Systems 문서
3. ProjectSSOT/01_ProjectState.md
4. 대표 Plan의 현재 작업 체크포인트
5. ActiveWork의 상태 요약
6. 이전 대화와 AI 기억
```

다음 작업과 우선순위를 판단할 때는 다음 순서를 사용한다.

```text
1. ProjectSSOT/02_Roadmap.md
2. ProjectSSOT/03_FeatureQueue.md
3. ActiveWork.md
4. 해당 대표 Plan
5. 실제 코드의 차단 상태와 의존 관계
6. DesignSource와 장기 CombatPlan
```

장기 방향을 판단할 때는 다음 순서를 사용한다.

```text
1. ProjectSSOT/00_Vision.md
2. DesignSource
3. ProjectSSOT/CombatPlan
4. 현재 Roadmap
```

과거 Archive가 현재 문서와 충돌하면 현재 ProjectSSOT, Systems와 실제 저장소를 우선한다.

### 2.6 공식 상태와 lifecycle 모델

FeatureQueue의 기능 상태와 Plan 문서 lifecycle을 구분한다.

#### Feature 상태

| 상태 | 의미 |
| --- | --- |
| Candidate | 아직 착수하지 않은 기능 후보 |
| Ready | 설계·선행조건이 준비됐지만 현재 Active는 아님 |
| Active | 현재 대표 Plan에서 진행 중 |
| Paused | 체크포인트를 보존한 채 일시중지 |
| Blocked | 선행 조건 때문에 진행 불가 |
| Done | 정의된 완료 조건 통과 + Current Knowledge 승격 완료 |
| Deferred | 현재 일정에서 보류 |
| Rejected | 기각 |

#### Plan 문서 lifecycle

```text
Active / In Progress
→ Closed
→ Historical
```

`Done` 기능의 Plan을 Historical로 내릴 때는 다음 순서를 사용한다.

```text
실제 구현 + 필요한 Build/Automation/USER PIE
→ Current Knowledge를 Systems와 필요한 ProjectSSOT로 승격
→ ActiveWork / Plan Index의 stale current route 제거
→ 대표 Plan/Result 탐색 경로 보존
→ semantic Historical
→ 필요할 때만 optional physical move
```

Historical과 물리 위치는 별도다.

```text
Historical + Retained Path
Historical + Move Candidate
Historical + Archived Path
```

따라서 파일이 `Document/Plan/` 기존 경로에 남아 있어도 현재 착수 기준에서 내려왔다면 Historical일 수 있다. 반대로 Archive 폴더에 있다는 이유만으로 현재 구현이나 완료 증거가 자동 확정되는 것도 아니다.

핵심 원칙:

```text
파일이 존재한다
≠ 공식 Current 기준

파일이 Archive 경로에 있다
≠ 완료 증거

Historical이다
≠ 반드시 물리 이동 완료
```

### 2.7 독립 저장소와 문서 소유권 경계

CarFight 문서체계의 소유 범위는 `main_game` 게임 프로젝트다.
파일이 CarFight 작업 폴더 아래에 물리적으로 존재하더라도 별도 Git 저장소이면 해당 저장소의 문서체계가 상태와 계획을 소유한다.

| 독립 저장소 | 문서 진입점 | 활성 작업 | Plan 색인 |
| --- | --- | --- | --- |
| AssetDump | `UE/Plugins/ue-assetdump/Documents/Document_Entry.md` | `UE/Plugins/ue-assetdump/Documents/ActiveWork.md` | `UE/Plugins/ue-assetdump/Documents/Plan/README.md` |
| GoPyMCP | `GoPyMCP/Workspace/docs/Document_Entry.md` | `GoPyMCP/Workspace/docs/ActiveWork.md` | `GoPyMCP/Workspace/docs/plan/README.md` |

다음 정보는 CarFight `ActiveWork`, Plan, ProjectSSOT와 FeatureQueue에 등록하지 않는다.

```text
- 독립 저장소의 내부 활성 Task와 마지막 작업 초점
- 내부 릴리스 gate와 로드맵 진행률
- TaskSource와 Codex 계약
- 내부 빌드·검증 체크포인트와 다음 작업
```

CarFight가 독립 도구의 공개 기능에 의존할 경우 다음 정보만 CarFight 문서에 기록할 수 있다.

```text
- 공개 command 또는 tool contract 이름
- 요구 schema 또는 contract 버전
- CarFight에서 사용하는 코드·문서 위치
- 호환성 또는 최소 요구 조건
```

독립 저장소 작업을 요청받으면 CarFight 라우터에서 작업을 복원하지 않고 해당 저장소의 Git 상태, 가장 가까운 `AGENTS.md`와 문서 진입점으로 전환한다.

---

## 3. 작업별 라우팅

| 작업 종류 | 첫 진입 문서 | 다음에 선택할 문서 |
| --- | --- | --- |
| 실제 코드·설정·스크립트 구현 또는 수정 | `Document/CodeWorkGate.md` | 소유 저장소 Git 상태, 대표 Plan, 실제 코드, 변경 범위와 검증 방법 |
| 이전 세션 복원 또는 활성 작업 전환 | `Document/ActiveWork.md` | 선택한 작업의 대표 Plan, 관련 Systems, 실제 코드와 에셋 |
| 프로젝트 현재 상태와 우선순위 확인 | `Document/ProjectSSOT/README.md` | `01_ProjectState.md`, `02_Roadmap.md`, `03_FeatureQueue.md` |
| 프로젝트 비전과 장기 방향 확인 | `Document/ProjectSSOT/00_Vision.md` | 필요 시 `Document/DesignSource/README.md` |
| 현재 구현 구조 확인 | `Document/Systems/SystemIndex.md` | 해당 기능의 Systems 문서 |
| 진행 중 계획과 남은 검증 확인 | `Document/Plan/README.md` | 선택한 Plan의 대표 진입 문서 |
| 전투 정체성·장기 전투 설계 확인 | `Document/ProjectSSOT/CombatPlan/00_Index.md` | 필요한 번호 문서만 선택 |
| 차량·터렛·발사체 콘셉트 작업 | `Document/Plan/ConceptArt/README.md` | 필요한 규칙 문서와 작업 대상 이미지 |
| 완료·보류된 과거 Plan 조사 | 현재 ProjectSSOT와 Systems를 먼저 확인 | `Document/Plan/Archive/README.md`에서 필요한 폴더만 선택 |
| 공통 UE 협업 방식 확인 | `Document/SSOT/README.md` | `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md` 및 필요한 기술 SSOT |
| 문서체계 자체 개선 | `Document/Plan/DocSystemPlan/CF_DocSystemPlan_v0_1.md` | `Document/AGENTS.md`, 이 문서, 관련 색인 |

도구 자체의 계약, 실행 절차, MCP 운영 문서는 `Document/`에서 찾지 않는다.
그 내용은 별도 도구 저장소 문서를 기준으로 한다.

---

## 4. 기본 읽기 순서

### 4.1 CarFight 현재 상태를 파악할 때

```text
1. Document/ProjectSSOT/README.md
2. Document/ProjectSSOT/00_Vision.md
3. Document/ProjectSSOT/01_ProjectState.md
4. Document/ProjectSSOT/02_Roadmap.md
5. Document/ProjectSSOT/03_FeatureQueue.md
6. 필요한 경우 04_ProjectDecisions.md와 05_TestChecklist.md
```

### 4.2 기능 구현 또는 수정 작업을 준비할 때

이 절차는 사용자의 첫 요청 문구와 세션 복원 여부에 관계없이 강제한다.

```text
1. 저장소 루트 AGENTS.md 확인
2. main_game Git 브랜치와 미커밋 변경 확인
3. Document/CodeWorkGate.md 확인
4. Document/Document_Entry.md 확인
5. Document/ProjectSSOT/01_ProjectState.md 확인
6. Document/ProjectSSOT/03_FeatureQueue.md 확인
7. Document/Systems/SystemIndex.md에서 현재 구현 문서 선택
8. Document/Plan/README.md에서 해당 작업 대표 Plan 선택
9. 필요한 실제 코드와 에셋 확인
10. 작업 범위, 보호 범위, 완료 조건과 검증 방법 확정
11. 현재 AI 세션이 승인된 저장소 도구로 실제 코드·설정·스크립트 수정
12. 수정 직후 Git diff 검수
13. 공식 빌드와 관련 자동 테스트 실행
14. 필요한 후속 보정과 재검증
15. PIE-runtime 기술 검증이 필요하면 Accepted `GoPyMCP.RuntimeRead`로 관측 가능한 사실을 AI가 먼저 직접 검증
16. 시각·UX·조작감·주행감·조준감·연출 감각처럼 사람 판단이 남거나 현재 capability 밖인 경우에만 사용자 PIE 또는 Editor 작업 절차 제공
17. 관련 대표 Plan, ActiveWork, ProjectSSOT 또는 Systems 동기화
```

현재 AI 세션의 기본 완료 조건은 실제 구현, diff 검수와 가능한 검증을 같은 작업 흐름에서 수행하는 것이다.
TaskSource, WorkOrder와 Codex YAML은 복잡한 구현을 구조화할 때 사용할 수 있지만 직접 구현을 시작하기 위한 필수 게이트가 아니다.
`plan.*` 기능이나 외부 Codex 실행기가 없다는 이유로 소스 작업을 차단하지 않는다.

직접 구현이 차단되는 경우는 기존 dirty 변경과의 안전한 분리가 불가능하거나, 사용자 결정 없이 구현 방향을 확정할 수 없거나, 필수 도구가 반복 실패하는 경우로 제한한다.
사용자의 명시적 요청 없는 commit, push, reset, checkout과 stash는 수행하지 않는다.

별도 Codex 실행은 사용자가 명시적으로 요청한 경우에만 선택적으로 사용한다.
외부 결과를 검수할 때는 실제 Git diff, 빌드·테스트와 PIE 증거를 확인하고 검증된 결과만 Systems와 작업 상태 문서에 반영한다.

### 4.3 과거 기록을 조사할 때

```text
1. 현재 ProjectSSOT와 Systems 기준 확인
2. 조사 목적과 필요한 과거 기능을 명시
3. Plan/Archive/README.md에서 해당 폴더만 선택
4. 필요한 기록만 읽고 현재 기준과 분리해 해석
```

### 4.4 이전 세션 작업을 복원할 때

먼저 사용자가 이어가려는 작업의 소유 저장소를 판별한다.

```text
CarFight 게임 작업
→ main_game Git 상태
→ 저장소 루트 AGENTS.md
→ 코드 작업이면 Document/CodeWorkGate.md
→ Document/Document_Entry.md
→ Document/ActiveWork.md

AssetDump 작업
→ assetdump_repo Git 상태
→ UE/Plugins/ue-assetdump/AGENTS.md
→ UE/Plugins/ue-assetdump/Documents/Document_Entry.md
→ UE/Plugins/ue-assetdump/Documents/ActiveWork.md

GoPyMCP 작업
→ gopymcp_repo Git 상태
→ GoPyMCP/AGENTS.md
→ GoPyMCP/Workspace/docs/Document_Entry.md
→ GoPyMCP/Workspace/docs/ActiveWork.md
```

CarFight 게임 작업의 복원 순서는 다음과 같다.

```text
1. 저장소 루트 AGENTS.md 확인
2. main_game Git 상태 확인
3. 코드 작업이면 Document/CodeWorkGate.md 확인
4. Document/Document_Entry.md 확인
5. Document/ActiveWork.md에서 마지막 작업 초점 또는 사용자가 지정한 CarFight 작업 선택
6. 선택한 작업의 대표 Plan 체크포인트와 실행 출처 확인
7. 관련 Systems, ProjectSSOT와 실제 코드·에셋 확인
8. 완료 범위, 미완료 범위, 빌드 상태, PIE 상태와 다음 작업을 먼저 보고
9. 코드 작업 재개 전 변경 허용 파일, 보호 범위와 검증 방법 확인
10. 저장소와 체크포인트가 일치할 때 현재 AI 세션이 미완료 단계부터 직접 구현 재개
```

사용자가 CarFight 작업명 또는 작업 ID를 지정하면 `ActiveWork.md`의 마지막 작업 초점보다 해당 작업을 우선한다.
독립 저장소 작업을 지정하면 CarFight `ActiveWork.md`를 사용하지 않는다.

---

## 5. 문서 생성과 공식 승격

새 파일 생성과 공식 기준 등록을 분리한다.

| 단계 | 의미 | 상위 색인 갱신 |
| --- | --- | --- |
| Draft | 확정되지 않은 초안 | 없음 |
| Working | 현재 작업 지시, 조사, 임시 기록 | 없음 |
| Notes | 참고 메모와 실험 기록 | 없음 |
| Active Plan | 실제 착수 또는 검증 중인 계획 | `Document/Plan/README.md`만 |
| Current System | 구현 완료 후 현재 기준 | `Document/Systems/SystemIndex.md`만 |
| Archive | 완료·보류·대체된 과거 기록 | 필요한 경우 `Document/Plan/Archive/README.md`만 |

핵심 원칙:

```text
문서 생성 ≠ 공식 등록
```

일반적인 문서 추가 시 상위 문서 갱신은 최대 1개로 제한한다.

2개 이상의 상위 문서를 수정할 수 있는 예외:

- 최상위 폴더 구조 변경
- 프로젝트 개발 방향 변경
- ProjectSSOT 판단 기준 변경
- Plan에서 Systems로 공식 승격
- 문서 생명주기 또는 작업 라우팅 변경
- 깨진 링크와 실제 경로 불일치 수정

---

## 6. 색인 갱신 기준

| 변경 내용 | 갱신 대상 |
| --- | --- |
| Draft, Working, Notes 파일 추가 | 없음 |
| 기존 Plan 폴더 내부 세부 파일 추가 | 없음 |
| 새 Plan 폴더 공식 착수 | `Document/Plan/README.md` |
| 완료된 Systems 문서 추가 | `Document/Systems/SystemIndex.md` |
| Plan 폴더 Archive 이동 | `Document/Plan/README.md`와 `Document/Plan/Archive/README.md` |
| 프로젝트 현재 방향 변경 | 관련 ProjectSSOT 문서 |
| 최상위 라우팅 또는 생명주기 변경 | 이 `Document_Entry.md` |

이 문서는 개별 Plan 또는 Systems 파일이 추가될 때마다 갱신하지 않는다.

---

## 7. 기본 검색 제외

사용자가 명시적으로 요청하거나 해당 경로가 직접 작업 대상인 경우가 아니면 다음 경로는 기본 검색에서 제외한다.

```text
Document/**/.git/**
Document/Plan/Archive/**
Document/Plan/**/Generated/**
Document/Plan/ConceptArt/Image/**
Document/SSOT/**/.git/**
Document/SSOT/**/Archive/**
Document/SSOT/UE_SSOT/BP_Text_Format/Archive/**
Document/SSOT/UE_SSOT/BP_Text_Format/cases/**
```

다음 경우에는 필요한 범위만 예외적으로 읽을 수 있다.

- 사용자가 과거 기록을 명시적으로 요청한 경우
- 현재 문서와 과거 결정의 충돌 원인을 조사하는 경우
- Migration 또는 회귀 원인을 추적하는 경우
- 이미지 자체가 작업 입력인 경우

`Generated`, Codex 산출물, TaskSource, YAML 계약 파일은 대표 설계 문서를 먼저 읽은 뒤 실제 구현 지시가 필요할 때만 선택한다.

---

## 8. 현재 Document 구조

```text
Document/
  AGENTS.md                 # 이 경로 이하 AI 문서 작업 강제 규칙
  CodeWorkGate.md           # 브라우저 AI 코드 작업 사전 게이트와 실행 출처 통제
  ActiveWork.md             # 현재 활성 작업과 대표 체크포인트 연결
  Document_Entry.md         # 작업별 총괄 진입 라우터
  Link_Audit_Check.md       # 링크와 역할 정합성 점검표
  DesignSource/             # 장기 방향과 원본 기획 참고
  Plan/                     # 진행 중 상세 계획과 검증
    README.md               # Plan 폴더 단위 색인
    Archive/
      README.md             # 과거·보류 Plan 폴더 단위 색인
  ProjectSSOT/              # CarFight 현재 판단 기준
  SSOT/                     # 공용 규칙과 UE 협업 기준
  Systems/                  # 완료된 현재 구현 기준
```

---

## 9. 유지보수 규칙

- 공용 규칙을 `ProjectSSOT`에 복사하지 않는다.
- CarFight 전용 내용을 공용 `SSOT`에 넣지 않는다.
- Plan을 현재 구현 문서처럼 사용하지 않는다.
- Systems 문서에 아직 구현되지 않은 미래 설계를 현재 기능처럼 기록하지 않는다.
- Archive를 현재 착수 기준으로 읽지 않는다.
- 개별 파일 추가를 이유로 이 문서를 반복 수정하지 않는다.
- 새 문서는 기존 문서에 흡수할 수 있는지 먼저 판단한다.
- 새 공식 문서가 필요하면 역할과 등록할 색인 하나를 명확히 한다.

---

## 10. Quick Links

- `Document/CodeWorkGate.md`
- `Document/ActiveWork.md`
- `Document/ProjectSSOT/README.md`
- `Document/Systems/SystemIndex.md`
- `Document/Plan/README.md`
- `Document/Plan/Archive/README.md`
- `Document/ProjectSSOT/CombatPlan/00_Index.md`
- `Document/DesignSource/README.md`
- `Document/SSOT/README.md`
- `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`
- `Document/Plan/DocSystemPlan/CF_DocSystemPlan_v0_1.md`

---

## 11. Changelog

### v2.11 - 2026-08-19

- CarFight 구현 검증 라우팅에 Accepted `GoPyMCP.RuntimeRead` 우선 단계를 추가했다.
- PIE-runtime의 관측 가능한 기술 사실은 AI가 먼저 직접 검증하고, 사용자 PIE는 시각·UX·조작감·주행감·조준감·연출 감각 또는 현재 capability 밖의 항목에만 남기도록 작업 순서를 교정했다.
- `AI Runtime Technical Validation`과 `USER Visual/Feel Validation`을 서로 다른 evidence gate로 취급하도록 총괄 라우팅을 `CodeWorkGate.md v2.7`과 정합화했다.

### v2.10 - 2026-08-13

- 대표 Plan을 detailed work status owner, ActiveWork와 Plan Index를 Current projection으로 명확히 했다.
- Feature 상태와 Plan 문서 lifecycle을 분리하고 `Active/In Progress → Closed → Historical` semantics를 추가했다.
- 완료 Plan의 Historical 전환에 Current Knowledge Promotion과 Current Route Cleanup을 필수 Gate로 추가했다.
- semantic Historical과 physical placement를 분리해 `Historical + Retained Path`를 정식 허용했다.

### v2.9 - 2026-07-27

- CarFight 코드 작업 기본 경로를 별도 Codex 위임에서 현재 AI 세션의 직접 구현으로 변경했다.
- `CodeWorkGate.md`의 역할을 직접 구현, 변경 보호, 검증과 완료 판정 통제로 갱신했다.
- 기능 구현 라우팅을 실제 수정 → Git diff → 빌드·자동 테스트 → 사용자 PIE → 문서 동기화 순서로 변경했다.
- TaskSource, WorkOrder와 Codex YAML을 필수 게이트가 아닌 선택적 보조 산출물로 재분류했다.
- 별도 Codex 실행은 사용자가 명시적으로 요청한 경우에만 사용하는 선택 경로로 변경했다.

### v2.8 - 2026-07-16

- `plan.*` 기능을 Codex 실행기가 아니라 TaskSource·최종 Codex YAML 작업지시서 생성기로 명확히 정의.
- 브라우저 세션의 완료 조건을 실제 코드 수정에서 `Ready for External Codex` 작업지시서 생성과 경로 전달로 변경.
- Codex 실행 도구 미연결을 정상 상태로 정의하고 작업 차단 사유에서 제거.
- 기능 구현 라우팅을 작업지시서 준비 단계와 별도 Codex 실행·후속 검수 단계로 분리.
- 작업지시서 생성 차단 상태를 `Blocked — Plan Work-Order Generation Unavailable`로 구체화.

### v2.7 - 2026-07-16

- `Document/CodeWorkGate.md`를 실제 코드·설정·스크립트 작업의 최우선 진입 문서로 추가.
- 사용자의 첫 요청 문구와 세션 복원 여부에 관계없이 모든 브라우저 세션에서 코드 작업 게이트를 강제하도록 라우팅 변경.
- 코드 변경 전 대표 Plan, TaskSource와 Codex 실행 계약의 실제 존재를 필수 조건으로 추가.
- 대표 Plan에 실행 방식, TaskSource, Codex 계약, Codex 실행 결과와 브라우저 검수 출처를 기록하도록 추가.
- Plan·Codex 사용 불가 시 자동 직접 수정 예외를 제거하고 `Blocked — Plan/Codex Unavailable` 처리로 변경.
- 일반적인 구현 요청을 직접 수정 승인으로 해석하지 않으며, 정책 위반과 출처 누락을 사후 위조 없이 기록하도록 추가.

### v2.6 - 2026-07-14

- 기능 구현·수정 기본 경로를 `Plan 기능으로 Codex 작업지시서 생성 → Codex 실행 → 브라우저 AI 검수`로 변경.
- 코드 변경 전 작업 범위, 보호 범위, 완료 조건과 검증 방법을 확정하도록 읽기 순서 확장.
- Codex 완료 후 실제 Git diff, 빌드·테스트·PIE 증거를 확인하고 검증된 결과만 문서 상태에 반영하도록 추가.
- C++, Go, Python, PowerShell, 배치와 텍스트 설정 변경을 브라우저 AI 직접 패치 대신 Codex 위임 대상으로 명시.

### v2.5 - 2026-07-14

- CarFight 문서체계의 소유 범위를 `main_game` 게임 프로젝트로 제한.
- AssetDump와 GoPyMCP의 독립 문서 진입점, ActiveWork와 Plan 색인 경로 추가.
- 물리적 하위 경로와 Git·문서 소유권이 다를 수 있다는 경계 원칙 명시.
- 독립 저장소의 내부 Task, 릴리스 상태와 체크포인트를 CarFight 문서에 등록하지 않도록 규칙 추가.
- 세션 복원 전에 작업 소유 저장소를 판별하고 해당 저장소의 라우터로 전환하도록 복원 순서 수정.

### v2.4 - 2026-07-14

- CarFight 전체 문서 아키텍처를 `AGENTS → Document_Entry → ActiveWork → ProjectSSOT → Plan → 코드·에셋 → Systems → Archive` 흐름으로 공식 정의.
- DesignSource, 공용 SSOT와 Link Audit의 보조 책임을 전체 구조에 포함.
- 각 문서 영역의 책임과 비책임을 표로 명시.
- ActiveWork와 대표 Plan, CombatPlan과 일반 Plan의 책임 경계를 명시.
- 현재 구현, 다음 작업, 장기 방향을 판단할 때의 문서 권한 우선순위를 추가.
- Draft부터 Archived까지의 공식 문서 상태 모델을 통합.

### v2.3 - 2026-07-14

- `Document/ActiveWork.md`를 현재 활성 작업과 대표 체크포인트를 연결하는 세션 복원 기준으로 등록.
- 이전 세션 복원과 활성 작업 전환을 위한 작업별 라우팅 추가.
- 저장소, ActiveWork, 대표 Plan, Systems와 실제 코드의 교차검증 순서 추가.
- 사용자가 지정한 작업명 또는 작업 ID를 마지막 작업 초점보다 우선하도록 명시.

### v2.2 - 2026-07-14

- 총괄 진입점 역할을 전체 파일 목록이 아닌 작업별 라우터로 명확화.
- `Plan/README.md`, `Plan/Archive/README.md`, `Systems/SystemIndex.md` 중심의 폴더 색인 구조 적용.
- 문서 생성과 공식 승격 분리, 상위 색인 갱신 최대 1개 원칙 추가.
- Archive, Generated, ConceptArt Image, `.git` 기본 검색 제외 규칙 추가.
- 전투 설계 진입 경로를 실제 `ProjectSSOT/CombatPlan/00_Index.md` 위치로 연결.

### v2.1 - 2026-06-19

- ProjectSSOT 빠른 시작 경로를 현재 파일명 기준으로 갱신.
- `Document/Plan`, `Document/Systems`, `Document/DesignSource` 역할을 현재 구조에 맞게 추가.
- UE 작업 시 `ProjectSSOT`를 실행 기준으로 보고 `Plan`은 필요 시 참고하는 흐름으로 정리.

### v2.0

- `Document/` 허브에서 MCP 전용 엔트리 구조 제거.
- Document 허브 역할을 `SSOT / ProjectSSOT / UE_SSOT` 중심으로 재정의.
- 도구 문서는 별도 도구 저장소에서 관리한다는 원칙 명시.

---

## 12. Migration

### v2.11 적용 안내

- 새 CarFight 작업은 PIE-runtime technical fact가 필요한 경우 사용자 확인을 요청하기 전에 Accepted `GoPyMCP.RuntimeRead` 적용 가능성을 먼저 판정한다.
- RuntimeRead로 기술 PASS를 확보해도 USER Visual/Feel PASS를 추정하지 않으며, 반대로 사용자 미확인만으로 RuntimeRead technical evidence를 Pending 처리하지 않는다.
- RuntimeRead 세부 tool/policy/lifecycle 계약은 GoPyMCP Current 문서가 소유하며 CarFight 문서에는 Consumer 사용 원칙만 유지한다.

### v2.10 적용 안내

- `Document/ActiveWork.md`와 `Document/Plan/README.md`는 대표 Plan의 상세 상태를 복제하지 않고 현재 작업 선택에 필요한 projection만 유지한다.
- Feature `Done` 후 현재 구현은 Systems와 필요한 ProjectSSOT가 소유하며 완료 Plan은 `Document/Plan/Archive/README.md`에서 Historical로 찾는다.
- 완료 Plan의 물리 move/rename은 closure 필수조건이 아니다. 링크·dirty work·rollback 안전성이 별도 확인될 때만 maintenance로 수행한다.
- Historical 문서의 과거 Pending/Blocked/next-action을 현재 작업으로 자동 복원하지 않는다.

### v2.9 적용 안내

- 새 CarFight 코드 작업은 `AGENTS.md → CodeWorkGate.md → Document_Entry.md` 순서로 진입한 뒤 현재 AI 세션이 직접 구현한다.
- TaskSource, WorkOrder와 Codex YAML은 복잡한 작업을 구조화할 때만 선택적으로 사용한다.
- `plan.*` 기능 또는 외부 Codex 실행기가 없다는 이유로 코드 작업을 차단하지 않는다.
- 직접 구현 후 실제 Git diff, 공식 빌드, 관련 자동 테스트와 사용자 PIE 증거를 확인한다.
- 사용자가 명시적으로 요청하지 않으면 commit, push, reset, checkout과 stash를 수행하지 않는다.
- AssetDump와 GoPyMCP는 독립 저장소이므로 각 저장소의 자체 정책을 따른다.

### v2.8 적용 안내 — 폐기됨, v2.9가 대체

> 아래 내용은 당시 실행 기준 보존용이며 현재 코드 작업 판단에는 사용하지 않는다.

- 브라우저 AI 세션은 `plan.*`으로 TaskSource와 최종 Codex YAML 작업지시서를 생성하고 경로를 전달하는 단계다.
- `plan.*`은 Codex 실행기가 아니며, Codex 실행 도구가 연결되지 않은 상태는 정상이다.
- 실제 코드 수정은 별도 Codex 세션 또는 사용자가 선택한 Codex 환경에서 최종 YAML을 입력해 수행한다.
- 브라우저 세션은 `quality_gate.passed`, `evidence_gate.passed`, `final_output_ready == true`와 최종 YAML 경로를 확인한 뒤 `Ready for External Codex`로 종료할 수 있다.
- 작업 차단은 `plan.*` 사용 불가, 품질 게이트 미해결 또는 안전한 대상 범위 확정 불가일 때만 적용한다.
- 기존 문서의 `Codex 실행 계약`은 `최종 Codex 작업지시서`, `Codex 실행`은 별도 외부 실행 단계로 해석한다.

### v2.7 적용 안내 — 폐기됨, v2.8이 대체

> 아래 내용은 변경 이력 보존용이며 현재 작업 판단에는 사용하지 않는다.

- v2.6의 `Plan·Codex 사용 불가 시 직접 수정 가능` 해석은 폐기한다.
- 실제 코드·설정·스크립트 작업은 모든 새 브라우저 세션에서 `AGENTS.md → CodeWorkGate.md → Document_Entry.md` 순서로 진입한다.
- 대표 Plan, TaskSource와 Codex 실행 계약이 실제 경로에 존재하기 전에는 코드 쓰기 작업을 시작하지 않는다.
- Plan 또는 Codex 기능을 사용할 수 없으면 `Blocked — Plan/Codex Unavailable`로 보고하고, 그 보고 이후 사용자가 브라우저 AI의 직접 수정을 명시적으로 승인한 경우에만 예외를 연다.
- 기존의 일반적인 `수정해줘`, `구현해줘`, `계속해줘`는 직접 수정 승인으로 간주하지 않는다.
- 계약 없이 수행된 기존 변경은 자동으로 되돌리지 않지만 대표 Plan에 `Policy Violation` 또는 `Provenance Missing`으로 사실대로 기록한다.
- 사후 TaskSource나 Codex 계약을 생성해 과거 직접 수정의 실행 출처를 Codex로 바꾸지 않는다.

### v2.6 적용 안내

- CarFight의 실제 코드 변경은 웹브라우저 AI가 직접 수행하는 방식에서 Plan 기능으로 Codex 작업지시서를 생성하고 Codex가 실행하는 방식으로 전환한다.
- 기존 Systems, 대표 Plan과 FeatureQueue 위치는 변경하지 않는다.
- TaskSource와 Codex 실행 계약은 해당 기능의 소유 저장소 Plan 폴더에 두고 독립 저장소 경계를 유지한다.
- 브라우저 AI는 Codex 산출물을 실제 Git diff, 빌드·테스트·PIE 결과와 교차검증한 후에만 완료 상태를 기록한다.
- 사용자가 직접 수정을 명시하거나 Plan·Codex 기능을 사용할 수 없는 경우에만 직접 코드 수정 예외를 적용한다.
- 문서만 수정하는 작업과 UE 에디터에서 수행하는 Blueprint·바이너리 에셋 변경은 이 코드 위임 전환의 대상이 아니다.

### v2.5 적용 안내

- CarFight `Document/`는 `main_game` 게임 프로젝트만 관리한다.
- AssetDump와 GoPyMCP의 문서체계는 각 독립 저장소 내부로 분리한다.
- 기존 CarFight `AssetDumpPlan` 연결은 공식 색인과 ActiveWork에서 제거하고 폐기 안내 문서로 전환한다.
- 독립 저장소 작업을 재개할 때는 CarFight 문서가 아니라 해당 저장소의 Git 상태, `AGENTS.md`, `Document_Entry.md`와 `ActiveWork.md`를 사용한다.
- CarFight에는 독립 도구의 공개 계약명, 요구 버전, 사용 위치와 호환성 조건만 기록할 수 있다.
- 물리적으로 CarFight 폴더 아래에 위치한다는 이유만으로 중첩 Git 저장소의 내부 Plan을 CarFight 문서에 등록하지 않는다.

### v2.4 적용 안내

- 별도 `DocumentArchitecture.md`를 만들지 않고 기존 총괄 라우터인 `Document_Entry.md`에 전체 문서 아키텍처를 통합했다.
- 기존 폴더와 색인 경로는 변경하지 않았다.
- `ActiveWork`는 작업 선택과 대표 Plan 연결만 담당하고 상세 체크포인트는 각 대표 Plan에 유지한다.
- `ProjectSSOT/CombatPlan`은 장기 전투 설계, `Document/Plan/<Feature>`는 현재 착수 기능의 구현 계획으로 구분한다.
- 문서가 충돌하면 질문 종류에 맞는 권한 우선순위를 적용하며, 현재 구현 판정에서는 Git 상태와 실제 코드·에셋을 최우선으로 한다.
- 기존 Draft, Working, Notes, Active Plan, Current System, Archive 구분은 Candidate, Deferred, Done, Archived까지 포함하는 공식 상태 모델로 확장한다.

### v2.3 적용 안내

- 기존 최상위 폴더, FeatureQueue, Plan Index와 Systems Index의 역할은 변경하지 않는다.
- `Document/ActiveWork.md`를 현재 활성 작업과 대표 Plan 체크포인트를 연결하는 세션 복원용 색인으로 추가한다.
- 새 세션에서는 `이전 작업 이어서 진행해줘` 또는 `<작업명/ID> 이어서 진행해줘`로 복원 경로를 시작할 수 있다.
- 세션 이동 전에는 `새 세션 인계 준비해줘`로 상태가 바뀐 대표 Plan과 ActiveWork를 갱신한다.
- 체크포인트와 실제 저장소가 다르면 Git 상태와 실제 코드·에셋을 우선한다.

### 기존 적용 안내

- 기존 최상위 폴더와 ProjectSSOT 문서 경로는 변경하지 않았다.
- 앞으로 개별 Plan 파일은 이 문서에 직접 등록하지 않고 `Plan/README.md`의 폴더 단위 색인을 사용한다.
- 현재 구현 문서는 `Systems/SystemIndex.md`를 통해 선택한다.
- Archive, Generated, Image, `.git`은 기본 검색 범위에서 제외한다.
- 기존에 `Document/Plan/CombatPlan/`으로 안내하던 경로는 실제 위치인 `Document/ProjectSSOT/CombatPlan/`을 사용한다.
