# CarFight 문서체계 개선 계획

- 문서 버전: v0.9
- 작성일: 2026-07-14
- 최근 갱신일: 2026-07-16
- 문서 상태: Completed
- 담당 범위: `Document/` 문서 진입 구조, 문서 생명주기, 세션 복원, 독립 저장소 경계와 웹브라우저 AI의 Codex 작업지시서 생성 게이트

---

## 1. 목적

CarFight 문서체계의 기존 역할 분리인 `ProjectSSOT / Plan / Systems / Archive`는 유지한다.

이번 개선의 목적은 문서 수를 줄이는 것이 아니라 다음 문제를 해결하는 것이다.

```text
- AI나 Codex가 작업마다 Document 전체를 재귀 탐색하는 문제
- 임시 작업 문서까지 공식 색인에 등록되어 문서 유지비가 커지는 문제
- 문서 하나를 추가할 때 여러 상위 문서를 함께 수정하게 되는 문제
- Archive, Generated, 이미지, 내부 Git 데이터가 기본 검색 범위에 노출되는 문제
- 실제 경로와 문서 내부 안내 경로가 서로 다른 문제
```

최종 목표는 다음과 같다.

```text
Document_Entry.md = 작업별 진입 라우터
Plan/README.md = Plan 폴더 단위 색인
Plan/Archive/README.md = 보류·완료·과거 Plan 색인
Systems/SystemIndex.md = 완료된 현재 구현 색인
ProjectSSOT = 현재 프로젝트 판단 기준
검색 제외 규칙 = AI/Codex 토큰 방어선
```

---

## 2. 적용 범위

### 2.1 이번 작업에 포함

- `Document/Plan/README.md` 신규 작성
- `Document/Plan/Archive/README.md` 신규 작성
- `Document/Document_Entry.md`를 작업별 라우터로 보강
- `Document/AGENTS.md`에 문서 읽기 범위와 갱신 최소화 규칙 추가
- `Document/ProjectSSOT/README.md`의 Plan 색인 운영 원칙 및 잘못된 CombatPlan 경로 정정
- `Document/ProjectSSOT/CombatPlan/README.md`의 실제 경로 정정과 선택 읽기 규칙 추가
- 현재 위치 `Document/Maintenance/Link_Audit_Check.md`에 해당하는 링크 감사 문서에 새 색인과 읽기 범위 점검 항목 추가
- 적용 결과를 이 문서의 Changelog와 Migration에 기록

### 2.2 이번 작업에서 제외

- `Document/Plan/ConceptArt/Image/` 대규모 이동
- 기존 Archive 전체 재분류 또는 삭제
- 기존 Plan 파일명 일괄 변경
- `Document/SSOT/` 내부 Git 저장소 제거 또는 구조 변경
- 과거 서버·멀티 문서 삭제
- 모든 기존 Plan 문서에 README 일괄 생성
- 현재 사용자가 수정 중인 전투 코드와 Systems 문서 변경

---

## 3. 현재 기준선

2026-07-14 확인 기준:

```text
Document/
  AGENTS.md
  Document_Entry.md
  Link_Audit_Check.md
  DesignSource/
  Plan/
  ProjectSSOT/
  SSOT/
  Systems/
```

확인된 주요 상태:

- `Document/Plan/README.md`가 없다.
- `Document/Plan/Archive/README.md`가 없다.
- `Document/Document_Entry.md`는 최상위 역할 설명은 있으나 작업별 선택 라우팅과 검색 제외 규칙이 부족하다.
- `Document/ProjectSSOT/README.md`는 존재하지 않는 Plan 색인을 참조하며 `Document/Plan/CombatPlan/`을 안내한다.
- 실제 CombatPlan 위치는 `Document/ProjectSSOT/CombatPlan/`이다.
- `Document/ProjectSSOT/CombatPlan/README.md` 내부 경로도 실제 위치와 다르다.
- `Document/Systems/Combat/`과 Combat Systems 문서는 이미 존재하므로 새 폴더 생성은 필요 없다.
- 현재 Plan에는 `AimFireAlignment`, `HitDamage`, `ProjectileContinuousCollision`, `ReticleFireFeedback` 등 최근 전투 작업 폴더가 존재한다.
- `Archive`, `Generated`, `ConceptArt/Image`는 기본 작업 검색에서 제외할 필요가 있다.

---

## 4. 핵심 개선 원칙

### 4.1 문서 생성과 공식 승격 분리

```text
Draft / Working / Notes
→ 상위 색인 갱신 없음

Active Plan으로 공식 승격
→ Document/Plan/README.md만 갱신

Current System으로 공식 승격
→ Document/Systems/SystemIndex.md만 갱신

ProjectSSOT 판단 기준 변경
→ 관련 ProjectSSOT 문서만 갱신
```

새 파일이 생겼다는 사실만으로 공식 색인에 등록하지 않는다.

### 4.2 상위 문서 갱신 최대 1개 원칙

일반적인 문서 추가 또는 변경 시 상위 색인 갱신은 최대 1개로 제한한다.

2개 이상의 상위 문서를 수정할 수 있는 예외는 다음과 같다.

- 최상위 폴더 구조 변경
- 프로젝트 개발 방향 변경
- ProjectSSOT 판단 기준 변경
- Plan에서 Systems로 공식 승격
- 문서 생명주기 규칙 변경
- 깨진 링크 또는 실제 경로 불일치 수정

### 4.3 폴더 단위 색인

`Plan/README.md`는 Plan 내부 모든 파일을 나열하지 않는다.

다음만 등록한다.

- Plan 폴더명
- 현재 분류 또는 진행 상태
- 폴더의 역할
- 대표 진입 문서

### 4.4 Document Entry 안정화

`Document_Entry.md`는 개별 작업 파일 목록이 아니다.

다음 경우에만 갱신한다.

- 새 최상위 문서 영역이 생긴 경우
- 작업별 라우팅이 바뀐 경우
- 문서 생명주기 또는 우선순위 규칙이 바뀐 경우
- 반복적으로 잘못 읽히는 경로가 발견된 경우

### 4.5 현재 구현과 계획 분리

```text
ProjectSSOT = 무엇을 왜 개발하는가
Plan = 앞으로 무엇을 어떻게 개발하는가
Systems = 현재 실제로 어떻게 구현되어 있는가
Archive = 더 이상 현재 착수 기준이 아닌 기록
```

Plan이 Systems보다 최신 날짜라는 이유만으로 현재 구현 기준으로 사용하지 않는다.

---

## 5. 문서 생명주기

```text
Draft / Working / Notes
→ Active Plan
→ Current System
→ Archive
```

| 단계 | 의미 | 공식 색인 갱신 |
| --- | --- | --- |
| Draft | 구조와 결론이 확정되지 않은 초안 | 없음 |
| Working | 현재 작업 지시, 조사, 임시 구현 기록 | 없음 |
| Notes | 참고 메모와 실험 기록 | 없음 |
| Active Plan | 실제 착수 또는 검증 중인 계획 | `Plan/README.md` |
| Current System | 구현 완료 후 현재 기준 | `Systems/SystemIndex.md` |
| Archive | 완료·보류·대체된 과거 기록 | `Plan/Archive/README.md`를 필요한 경우만 갱신 |

---

## 6. 작업별 읽기 라우팅

| 작업 종류 | 첫 진입 문서 | 추가 선택 문서 |
| --- | --- | --- |
| 현재 프로젝트 상태와 우선순위 | `Document/ProjectSSOT/README.md` | `01_ProjectState.md`, `02_Roadmap.md`, `03_FeatureQueue.md` |
| 현재 구현 구조 확인 | `Document/Systems/SystemIndex.md` | 해당 Systems 문서 |
| 진행 중 Plan 확인 | `Document/Plan/README.md` | 선택한 Plan의 대표 진입 문서 |
| 전투 정체성·장기 전투 설계 | `Document/ProjectSSOT/CombatPlan/00_Index.md` | 필요한 번호 문서만 선택 |
| 원화·AI 메시·콘셉트 작업 | `Document/Plan/ConceptArt/README.md` | 필요한 규칙 문서와 이미지 |
| 공통 UE 협업 기준 | `Document/SSOT/README.md` | 필요한 UE SSOT 문서 |
| 과거 결정과 Migration 조사 | 현재 문서 우선 확인 | 명시적으로 필요한 Archive만 선택 |

---

## 7. 기본 검색 제외 규칙

다음 경로는 사용자가 명시적으로 요청하거나 과거 결정 조사에 필요한 경우가 아니면 기본 검색에서 제외한다.

```text
Document/**/.git/**
Document/Plan/**/Archive/**
Document/Plan/**/Generated/**
Document/Plan/ConceptArt/Image/**
Document/SSOT/**/.git/**
Document/SSOT/**/Archive/**
Document/SSOT/UE_SSOT/BP_Text_Format/Archive/**
Document/SSOT/UE_SSOT/BP_Text_Format/cases/**
```

예외적으로 읽을 수 있는 경우:

- 사용자가 과거 기록을 명시적으로 요청한 경우
- 현재 문서와 과거 결정의 충돌 원인을 조사하는 경우
- Migration 또는 회귀 원인을 추적하는 경우
- 이미지 자체가 작업 입력인 경우

---

## 8. 적용 단계

### 단계 A — 기준 문서와 Plan 색인

1. 이 개선 계획 문서를 작성하고 `Approved` 상태로 확정한다.
2. `Document/Plan/README.md`를 생성한다.
3. `Document/Plan/Archive/README.md`를 생성한다.

### 단계 B — 진입점과 운영 규칙

1. `Document/Document_Entry.md`에 작업별 라우팅을 추가한다.
2. 문서 생성과 공식 승격 분리 원칙을 추가한다.
3. 검색 제외 규칙과 Archive 예외 규칙을 추가한다.
4. `Document/AGENTS.md`에 AI/Codex 강제 읽기 규칙을 추가한다.

### 단계 C — 경로와 역할 정합성

1. `Document/ProjectSSOT/README.md`의 CombatPlan 경로를 실제 위치로 정정한다.
2. Plan 개별 파일 추가 시 ProjectSSOT README를 갱신하지 않는다는 원칙을 추가한다.
3. `Document/ProjectSSOT/CombatPlan/README.md`의 내부 경로를 정정한다.
4. CombatPlan 전체 읽기 대신 `00_Index.md` 선택 읽기 규칙을 추가한다.

### 단계 D — 검증

1. 현재 위치 `Document/Maintenance/Link_Audit_Check.md`의 링크 감사 문서에 새 점검 항목을 추가한다.
2. 신규 파일과 수정된 링크가 실제 경로와 일치하는지 확인한다.
3. 현재 사용자 작업 파일과 충돌하지 않았는지 Git 상태로 확인한다.
4. 적용 결과를 이 문서에 기록하고 상태를 `Completed`로 변경한다.

---

## 9. 파일별 변경 예정 사항

| 경로 | 작업 유형 | 변경 내용 |
| --- | --- | --- |
| `Document/Plan/DocSystemPlan/CF_DocSystemPlan_v0_1.md` | 신규 | 개선 기준, 단계, 검증 및 Migration 기록 |
| `Document/Plan/README.md` | 신규 | 활성·검증 중 Plan의 폴더 단위 색인 |
| `Document/Plan/Archive/README.md` | 신규 | 보류·완료·과거 Plan의 폴더 단위 색인 |
| `Document/Document_Entry.md` | 수정 | 작업 라우터, 생명주기, 검색 제외 규칙 |
| `Document/AGENTS.md` | 수정 | AI/Codex 읽기 및 갱신 최소화 강제 규칙 |
| `Document/ProjectSSOT/README.md` | 수정 | Plan 운영 규칙과 CombatPlan 경로 정정 |
| `Document/ProjectSSOT/CombatPlan/README.md` | 수정 | 실제 경로와 선택 읽기 규칙 정정 |
| `Document/Maintenance/Link_Audit_Check.md` | 수정 | 색인·경로·승격·검색 제외 점검 추가 / 현재 물리 위치 기준 |

`Document/Systems/SystemIndex.md`는 현재 Combat 폴더와 실제 Systems 문서를 이미 반영하고 있으므로 이번 작업에서 불필요하게 수정하지 않는다.

---

## 10. 검증 기준

- `Document/Plan/README.md`가 존재하고 Plan 폴더 단위로만 안내한다.
- `Document/Plan/Archive/README.md`가 존재하고 Archive가 현재 착수 기준이 아님을 명시한다.
- `Document_Entry.md`가 전체 파일 목록이 아니라 작업별 라우터로 동작한다.
- Draft, Working, Notes 문서는 상위 색인 갱신 대상이 아님을 명시한다.
- 문서 하나의 일반 변경은 상위 색인 최대 1개 원칙을 따른다.
- `Document/Plan/CombatPlan/`이라는 잘못된 안내가 활성 문서에서 제거된다.
- CombatPlan 작업은 `00_Index.md`를 먼저 보고 필요한 문서만 선택한다.
- Archive, Generated, Image, `.git` 경로가 기본 검색 제외로 명시된다.
- 기존 사용자 미커밋 전투 코드와 Systems 문서는 변경하지 않는다.

---

## 11. 롤백 기준

다음 문제가 발견되면 해당 적용 단계만 되돌린다.

- 기존 유효 링크가 끊어진 경우
- 새 색인이 현재 구현과 계획을 혼동하게 만드는 경우
- AI/Codex가 필요한 현재 문서까지 읽지 못하게 되는 경우
- 사용자의 기존 미커밋 변경을 덮어쓴 경우
- Plan과 Systems의 역할이 중복된 경우

대규모 폴더 이동을 하지 않으므로 롤백은 생성 파일 삭제와 수정 문서 원문 복구로 제한한다.

---

## 12. 미결 사항

이번 적용 이후 별도 계획으로 검토한다.

- `Document/Plan/ConceptArt/Image/`를 `_Media/`로 이동할지 여부
- README가 없는 기존 Plan 폴더에 대표 진입 README를 추가할지 여부
- 완료된 Plan을 Archive로 옮기는 시점과 자동 점검 방식
- 문서 링크 자동 검사 스크립트 도입 여부
- `Document/SSOT/` 내부 Git 구조를 검색 계층에서 더 강하게 격리할 방법

---

## 13. 적용 결과

### 13.1 생성한 문서

- `Document/Plan/DocSystemPlan/CF_DocSystemPlan_v0_1.md`
- `Document/Plan/README.md`
- `Document/Plan/Archive/README.md`
- `Document/ActiveWork.md`

### 13.2 수정한 문서

- `AGENTS.md`
- `Document/Document_Entry.md`
- `Document/AGENTS.md`
- `Document/ProjectSSOT/README.md`
- `Document/ProjectSSOT/CombatPlan/README.md`
- `Document/Maintenance/Link_Audit_Check.md`
- `Document/Systems/SystemIndex.md`
- `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md` — 현재 Historical physical path
- `Document/Plan/Archive/ProjectileContinuousCollision/ImplementationDesign.md` — 현재 Historical physical path
- `Document/Plan/Archive/HitDamage/ImplementationDesign.md` — 현재 Historical physical path
- `Document/Plan/Archive/ReticleFireFeedback/ImplementationDesign.md` — 현재 Historical physical path

### 13.3 적용 완료 내용

- `Document_Entry.md`를 개별 파일 목록이 아닌 작업별 진입 라우터로 전환했다.
- Plan 루트와 Plan Archive 루트에 폴더 단위 색인을 추가했다.
- Draft, Working, Notes와 공식 Plan·Systems 승격을 분리했다.
- 일반 문서 변경의 상위 색인 갱신을 최대 1개로 제한하는 원칙을 추가했다.
- Archive, Generated, ConceptArt Image, `.git` 경로를 기본 검색 제외로 지정했다.
- CombatPlan 경로를 실제 `Document/ProjectSSOT/CombatPlan/` 위치로 정정했다.
- CombatPlan은 `00_Index.md`에서 필요한 번호 문서만 선택해서 읽도록 변경했다.
- 링크 감사 체크리스트에 역할 분리, 공식 승격, 검색 제외, 상태 최신성 검사를 추가했다.

#### v0.3 후속 개선

- 저장소 루트 `AGENTS.md`에 Git 상태 확인, `Document_Entry.md` 진입, 저장소 우선 판단, 기존 미커밋 변경 보호 규칙을 추가했다.
- 루트 `AGENTS.md`에 작업 상태가 바뀐 경우 대표 Plan 또는 현재 구현 문서에 완료 범위, 미검증 항목, 다음 작업을 남기도록 추가했다.
- `Document/AGENTS.md`를 v1.2로 갱신하고, 실제 변경이나 상태 전환이 있는 작업만 종료 기록 대상으로 지정했다.
- 종료 기록의 최소 항목을 현재 상태, 완료 범위, 미검증 항목, 다음 작업, 관련 코드·문서 경로로 확정했다.
- 빌드와 PIE 상태를 분리하고 사용자가 직접 확인하지 않은 PIE 결과는 PASS로 기록하지 않도록 했다.
- 단순 설명, 조사, 질의응답은 문서 갱신을 강제하지 않도록 했다.

#### v0.4 다중 작업 세션 복원 적용

- `Document/ActiveWork.md`를 활성 작업과 대표 체크포인트를 연결하는 중앙 색인으로 생성했다.
- ActiveWork에는 상세 구현을 복사하지 않고 활성 작업, 마지막 작업 초점, 의존 관계와 대표 Plan 경로만 유지하도록 했다.
- `AGENTS.md`, `Document/AGENTS.md`, `Document_Entry.md`에 짧은 세션 인계·복원 명령의 처리 흐름을 추가했다.
- `CF-FQ-017`, `CF-FQ-018`, `CF-FQ-022`, `CF-FQ-023` 대표 Plan에 표준 `현재 작업 체크포인트`를 적용했다.
- 체크포인트에서 완료 범위, 미검증 항목, 빌드 상태, PIE 상태, 바로 다음 작업과 보호 범위를 분리했다.
- `Link_Audit_Check.md`에 ActiveWork, 마지막 작업 초점과 대표 Plan 상태 정합성 점검을 추가했다.
- `Document/Plan/README.md`의 전투 우선 Plan 상태를 ActiveWork와 각 대표 Plan 체크포인트에 맞게 동기화했다.
- 장문의 대화 요약과 세션별 로그 파일을 만들지 않고 실제 Git 상태와 코드를 최종 기준으로 사용하도록 했다.

#### v0.5 전체 문서 아키텍처 공식 정의

- `Document/Document_Entry.md` v2.4에 전체 문서 아키텍처를 공식 정의했다.
- `AGENTS → Document_Entry → ActiveWork → ProjectSSOT → Plan → 코드·에셋 → Systems → Archive` 실행 흐름을 고정했다.
- DesignSource, 공용 SSOT와 Link Audit의 보조 책임을 전체 구조에 포함했다.
- 각 문서 영역의 책임과 비책임을 분리했다.
- ActiveWork와 대표 Plan, CombatPlan과 일반 Plan의 책임 경계를 명시했다.
- 현재 구현, 다음 작업, 장기 방향별 문서 권한 우선순위를 추가했다.
- Draft, Working, Notes, Candidate, Active Plan, Current System, Deferred, Done, Archived 공식 상태 모델을 추가했다.
- `ProjectSSOT/README.md`에 ActiveWork 역할을 추가하되 Roadmap과 FeatureQueue를 대체하지 않도록 명시했다.
- `Systems/SystemIndex.md`의 고속 Projectile 상태를 코드·공식 빌드 완료, 사용자 PIE Pending으로 동기화했다.
- `Link_Audit_Check.md`에 전체 아키텍처, 책임 경계, 권한 우선순위와 상태 모델 점검을 추가했다.

#### v0.6 독립 저장소 문서체계 분리

- CarFight 문서체계의 소유 범위를 `main_game` 게임 프로젝트로 제한했다.
- 잘못 등록됐던 AssetDump 내부 작업을 CarFight `ActiveWork.md`와 `Plan/README.md`에서 제거했다.
- CarFight의 마지막 작업 초점을 `CF-FQ-023`으로 복원했다.
- 기존 `Document/Plan/AssetDumpPlan/README.md`는 삭제 대신 `Deprecated / External Repository Boundary` migration stub으로 전환했다.
- AssetDump 독립 저장소에 다음 문서 진입 구조를 생성했다.

```text
UE/Plugins/ue-assetdump/AGENTS.md
UE/Plugins/ue-assetdump/Documents/Document_Entry.md
UE/Plugins/ue-assetdump/Documents/ActiveWork.md
UE/Plugins/ue-assetdump/Documents/Plan/README.md
```

- GoPyMCP 독립 저장소에 다음 문서 진입 구조를 생성 또는 보강했다.

```text
GoPyMCP/AGENTS.md
GoPyMCP/Workspace/docs/Document_Entry.md
GoPyMCP/Workspace/docs/ActiveWork.md
GoPyMCP/Workspace/docs/plan/README.md
```

- `AGENTS.md`, `Document/AGENTS.md`, `Document_Entry.md`에 물리적 하위 경로와 Git·문서 소유권을 구분하는 경계 규칙을 추가했다.
- 세션 복원 전에 CarFight, AssetDump, GoPyMCP 중 작업 소유 저장소를 먼저 판별하도록 라우팅을 변경했다.
- CarFight에는 독립 도구의 공개 계약명, 요구 버전, 사용 위치와 호환성 조건만 기록하도록 제한했다.
- `Link_Audit_Check.md`에 세 저장소의 독립 AGENTS, Document Entry, ActiveWork와 Plan 색인 검사를 추가했다.

#### v0.7 웹브라우저 AI의 Plan/Codex 코드 위임

- 루트 `AGENTS.md`에 웹브라우저 AI와 Codex 실행 주체를 구분하는 기본 위임 규칙을 추가했다.
- C++, Go, Python, PowerShell, 배치와 텍스트 설정 변경은 브라우저 AI가 직접 패치하지 않고 `plan.*` 기능으로 TaskSource와 Codex 실행 계약을 만든 뒤 Codex가 수행하도록 했다.
- `Document/AGENTS.md` v1.5에 TaskSource·Codex 계약의 최소 필드, 직접 수정 예외와 검수 절차를 상세 정의했다.
- `Document/Document_Entry.md` v2.6의 기능 구현 흐름을 `현재 구현 확인 → Plan 계약 생성 → Codex 실행 → 실제 diff·검증 증거 검수`로 변경했다.
- AssetDump `AGENTS.md` v1.1에는 commandlet, report schema, parser, full closure, process-log evidence와 콘텐츠 불변성 검증을 포함했다.
- GoPyMCP `AGENTS.md` v1.2에는 MCP SSOT, 공개 contracts, 계층 경계, feature flag off 회귀와 compatibility test를 포함했다.
- Codex가 이미 계약을 실행하는 중에는 다시 Codex 위임을 만들지 않도록 재귀 위임을 금지했다.
- Codex 보고만으로 완료를 선언하지 않고 웹브라우저 AI가 실제 Git diff, 빌드·테스트·PIE 또는 저장소별 검증 증거를 확인하도록 했다.
- 검수에 실패한 경우 브라우저 AI가 직접 임시 패치하지 않고 TaskSource 또는 계약을 보완해 Codex에 재작업을 요청하도록 했다.
- 직접 코드 수정은 사용자의 명시적 요청 또는 Plan·Codex 기능 사용 불가로 작업이 차단된 경우에만 허용하고 사유와 범위를 먼저 알리도록 했다.
- 문서 전용 수정, 읽기 전용 분석·리뷰와 UE 에디터의 Blueprint·바이너리 에셋 작업은 코드 위임 대상에서 제외했다.
- `Link_Audit_Check.md` v2.6에 위임 경로, 계약 필드, 검수 증거, 예외와 저장소별 규칙을 점검하는 감사 항목을 추가했다.

#### v0.8 코드 작업 강제 게이트 보완

v0.7은 코드 작업을 Codex에 위임한다는 문구를 추가했지만 실제 강제 구조로는 부족했다.

확인된 원인은 다음과 같다.

- 루트 `AGENTS.md`의 위임 규칙이 일반 코드 작성 규칙 뒤에 있어 우선순위가 불명확했다.
- 새 브라우저 세션이 로컬 미커밋 문서를 자동으로 읽는다는 보장이 없었다.
- Plan 또는 Codex 기능 사용 불가만으로 직접 수정 예외를 열 수 있었다.
- 대표 Plan에 TaskSource와 Codex 계약의 실제 경로를 남기는 실행 출처 필드가 없었다.
- `UCFVehicleHealthComp` 구현은 관련 TaskSource·Codex 계약이 발견되지 않았고 기존 `TaskSource_VisualHitCollision.md`는 Health Component 구현을 명시적으로 제외했다.

보완 결과:

- `Document/CodeWorkGate.md` v1.0을 코드·설정·스크립트 작업의 최우선 실행 게이트로 생성했다.
- 루트 `AGENTS.md` 최상단에 CodeWorkGate 우선 규칙과 소스 직접 수정 금지를 배치했다.
- `Document/AGENTS.md` v1.6과 `Document_Entry.md` v2.7에 사용자 트리거 문구와 무관한 필수 진입 경로를 추가했다.
- 대표 Plan에 실행 방식, TaskSource, Codex 계약, Codex 실행 결과, 브라우저 검수, 빌드와 PIE 상태를 기록하도록 했다.
- Plan·Codex 사용 불가 시 자동 직접 수정을 금지하고 사용자에게 차단을 보고한 뒤 명시적 승인을 받아야만 예외가 열리도록 변경했다.
- 계약 없는 기존 변경은 기능 검증 결과와 별개로 `Workflow Compliance FAIL`, `Policy Violation` 또는 `Provenance Missing`으로 기록하도록 했다.
- 사후 계약 생성으로 과거 직접 수정의 출처를 Codex로 바꾸는 행위를 금지했다.
- `Link_Audit_Check.md` v2.7에 강제 게이트와 실행 출처 감사 항목을 추가했다.

#### v0.9 Codex 작업지시서 생성과 외부 실행 분리

v0.8은 직접 코드 수정 방지를 강화했지만 브라우저 AI의 작업지시서 생성 단계와 실제 Codex 실행 단계를 하나의 필수 흐름처럼 표현했다.
이 때문에 다른 세션이 `Codex 실행 도구가 연결되지 않았으므로 작업을 진행할 수 없다`고 잘못 판단했다.

교정 결과:

- `plan.*` 기능을 Codex 실행기가 아니라 TaskSource·최종 Codex YAML 작업지시서 생성기로 정의했다.
- 브라우저 세션 완료 조건을 실제 코드 수정에서 품질 게이트를 통과한 최종 YAML 생성과 경로 전달로 변경했다.
- 브라우저 세션에 Codex 실행 도구가 연결되지 않은 상태를 정상으로 정의하고 차단 사유에서 제거했다.
- 작업 흐름을 `브라우저 작업지시서 생성 → 별도 Codex 실행 → 브라우저 후속 검수`의 세 단계로 분리했다.
- `quality_gate.passed`, `evidence_gate.passed`, `final_output_ready == true`를 최종 작업지시서 준비 조건으로 지정했다.
- 대표 Plan 상태를 `Ready for External Codex`, 외부 Codex 실행 `Not Run`, 브라우저 검수 `Not Started`처럼 독립적으로 기록하도록 했다.
- 실제 차단 기준을 `plan.*` 사용 불가, 품질 게이트 미해결, 안전한 범위 확정 불가와 사용자 결정 필요로 한정했다.
- AssetDump와 GoPyMCP의 독립 `AGENTS.md`도 동일한 3단계 모델로 갱신했다.
- HitDamage의 과거 직접 수정 위반 판정은 유지하되 누락 항목 명칭을 `최종 Codex 작업지시서`로 교정한다.

### 13.4 의도적으로 수정하지 않은 항목

- `Document/Systems/SystemIndex.md`는 고속 Projectile 상태 요약만 현재 코드·빌드·PIE 단계와 동기화했으며, 개별 Combat Systems 문서의 내용과 책임은 변경하지 않았다.
- 사용자가 작업 중인 `03_FeatureQueue.md`, `05_TestChecklist.md`, Combat Systems 문서와 UE 코드·에셋은 변경하지 않았다.
- 폴더 이동, Archive 재분류, ConceptArt 이미지 이동은 수행하지 않았다.
- 문서 전용 변경이므로 Unreal Editor 빌드는 실행하지 않았다.

### 13.5 검증 결과

- 신규 Plan 색인 2개와 개선 계획 문서의 생성 상태를 확인했다.
- `ProjectSSOT/README.md`와 `CombatPlan/README.md`에서 실제 CombatPlan 경로를 사용하도록 정정했다.
- `Document/ActiveWork.md`가 존재하고 CF-FQ-017, 018, 022, 023의 대표 체크포인트를 연결하는 것을 확인했다.
- 마지막 작업 초점 `CF-FQ-023`이 활성 작업 표와 ProjectileContinuousCollision 대표 Plan에 일치하는 것을 확인했다.
- 활성 작업 네 개의 대표 Plan에 `현재 작업 체크포인트`가 하나씩 존재하는 것을 확인했다.
- 각 체크포인트가 완료 범위, 미검증 범위, 빌드 상태, PIE 상태, 바로 다음 작업과 보호 범위를 포함하는 것을 확인했다.
- `Document_Entry.md`에 전체 문서 아키텍처 공식 정의가 하나만 존재하는 것을 확인했다.
- `ProjectSSOT/README.md`가 ActiveWork를 현재 활성 작업 연결 계층으로 정의하고 Roadmap·FeatureQueue를 대체하지 않는 것을 확인했다.
- `Systems/SystemIndex.md`에서 오래된 고속 터널링 미완료 표현이 제거되고 코드·공식 빌드 완료와 사용자 PIE Pending이 분리된 것을 확인했다.
- `Link_Audit_Check.md`에 전체 아키텍처와 문서 권한 우선순위 점검 섹션이 존재하는 것을 확인했다.
- CarFight `ActiveWork.md`의 현재 활성 표와 마지막 작업 초점에서 AssetDump 내부 작업이 제거된 것을 확인했다.
- CarFight `Plan/README.md`에 `AssetDumpPlan/` 공식 표 행이 존재하지 않는 것을 확인했다.
- `AssetDumpPlan/README.md`가 폐기·이관 안내 상태인 것을 확인했다.
- AssetDump 독립 `AGENTS.md`, `Document_Entry.md`, `ActiveWork.md`, `Plan/README.md`가 존재하는 것을 확인했다.
- GoPyMCP 독립 `AGENTS.md`, `Document_Entry.md`, `ActiveWork.md`, `plan/README.md`가 존재하는 것을 확인했다.
- 세 저장소의 기존 코드, 스크립트, 에셋과 미커밋 변경을 정리하거나 되돌리지 않은 것을 확인했다.
- 각 수정은 원본 메타데이터 확인과 원자적 안전 쓰기 또는 단일 앵커 패치로 적용했다.
- 기존 미커밋 코드, 에셋과 이번 작업 대상이 아닌 문서는 수정하거나 정리하지 않았다.
- 문서 전용 변경이므로 Unreal Editor 빌드는 실행하지 않았다.

---

## 14. Changelog

### v0.9 - 2026-07-16

- `plan.*` 기능을 Codex 실행기가 아니라 TaskSource·최종 YAML 작업지시서 생성기로 교정.
- 브라우저 세션 완료 조건을 실제 코드 수정에서 `Ready for External Codex` 산출물 생성과 경로 전달로 변경.
- Codex 실행 도구 미연결을 정상 상태로 정의하고 차단 사유에서 제거.
- 작업 흐름을 브라우저 작업지시서 생성, 별도 Codex 실행, 브라우저 후속 검수의 세 단계로 분리.
- 품질·증거 게이트와 `final_output_ready`를 작업지시서 완료 조건으로 추가.
- CarFight, AssetDump와 GoPyMCP의 활성 규칙과 감사 기준을 같은 용어로 동기화.

### v0.8 - 2026-07-16

- v0.7 위임 규칙이 문구 수준에 머물러 새 세션에서 우회된 원인 분석 기록.
- `Document/CodeWorkGate.md`를 코드 작업 최우선 실행 게이트로 생성.
- 루트 AGENTS 최상단 우선 규칙, 모든 세션 필수 진입과 실행 출처 필드 추가.
- Plan·Codex 사용 불가 자동 직접 수정 예외 폐기와 사용자 사후 명시 승인 방식 적용.
- `UCFVehicleHealthComp` 작업의 TaskSource·Codex 계약 누락을 실제 위반 사례로 확인.
- 기능 검증과 워크플로 준수를 별도 상태로 판정하도록 감사 기준 강화.

### v0.7 - 2026-07-14

- 웹브라우저 AI의 실제 코드 작업 기본 경로를 `plan.* TaskSource·Codex 계약 → Codex 실행 → 브라우저 AI 검수`로 확정.
- 루트 `AGENTS.md`, `Document/AGENTS.md`와 `Document_Entry.md`에 기본 위임 흐름과 직접 수정 예외 추가.
- Codex 재귀 위임 금지와 완료 보고 대신 실제 diff·빌드·테스트·PIE 증거 검수 원칙 추가.
- AssetDump와 GoPyMCP에 저장소별 Codex 계약 보호 범위와 검증 기준 추가.
- `Link_Audit_Check.md`에 Plan/Codex 위임 감사 항목 추가.

### v0.6 - 2026-07-14

- CarFight 문서체계에서 AssetDump 내부 활성 작업과 Plan 등록 제거.
- CarFight 마지막 작업 초점을 `CF-FQ-023`으로 복원.
- 기존 CarFight `AssetDumpPlan`을 폐기·이관 안내 migration stub으로 전환.
- AssetDump에 독립 `AGENTS`, `Document_Entry`, `ActiveWork`, `Plan Index` 생성.
- GoPyMCP에 독립 `Document_Entry`, `ActiveWork`, `Plan Index` 생성 및 루트 `AGENTS.md` 보강.
- 물리적 하위 경로와 Git·문서 소유권을 분리하는 규칙 추가.
- 작업 복원 전 소유 저장소 판별과 독립 저장소 감사 기준 추가.

### v0.5 - 2026-07-14

- `Document_Entry.md` v2.4에 CarFight 전체 문서 아키텍처를 공식 정의.
- 각 문서 영역의 책임과 비책임, ActiveWork와 대표 Plan, CombatPlan과 일반 Plan의 책임 경계 추가.
- 현재 구현·다음 작업·장기 방향별 문서 권한 우선순위 추가.
- Candidate, Deferred, Done, Archived를 포함한 공식 문서 상태 모델 추가.
- `ProjectSSOT/README.md`에 ActiveWork 역할과 Roadmap·FeatureQueue 비대체 원칙 추가.
- `Systems/SystemIndex.md`의 고속 Projectile 상태를 코드·공식 빌드 완료, 사용자 PIE Pending으로 동기화.
- `Link_Audit_Check.md`에 전체 아키텍처와 권한·상태 정합성 검사 추가.

### v0.4 - 2026-07-14

- `Document/ActiveWork.md` 다중 작업 세션 복원 색인 생성.
- `새 세션 인계 준비해줘`, `이전 작업 이어서 진행해줘`, 특정 작업 재개 명령 처리 규칙 적용.
- CF-FQ-017, 018, 022, 023 대표 Plan에 표준 현재 작업 체크포인트 추가.
- 활성 작업 상세 상태는 대표 Plan에 유지하고 ActiveWork에는 작업·초점·의존 관계·경로만 유지하도록 역할 분리.
- 새 세션에서 Git, 대표 Plan, Systems와 실제 코드·에셋을 교차검증하고 수정 전에 복원 결과를 보고하도록 규칙 추가.
- 링크 감사 체크리스트에 ActiveWork와 대표 체크포인트 정합성 검사를 추가.
- 장문의 대화 요약과 세션별 로그 파일을 사용하지 않는 운영 방식 확정.

### v0.3 - 2026-07-14

- 저장소 루트 `AGENTS.md`에 새 세션의 Git 상태 확인과 `Document_Entry.md` 진입 규칙 추가.
- 이전 대화나 AI 기억보다 현재 저장소와 ProjectSSOT를 우선하도록 규칙 추가.
- 기존 미커밋 변경 보호와 작업 상태 변경 시 인수인계 기록 규칙 추가.
- `Document/AGENTS.md`를 v1.2로 갱신하고 작업 종료 상태 기록 조건과 최소 항목 확정.
- 빌드와 PIE 검증 상태 분리 및 사용자 미확인 PIE 결과의 PASS 기록 금지.
- 단순 설명과 조사 작업에는 문서 갱신을 강제하지 않도록 예외 명시.

### v0.2 - 2026-07-14

- 단계 A부터 D까지 문서체계 개선 적용 완료.
- Plan 및 Archive 폴더 단위 색인 생성.
- Document Entry 작업 라우터와 AGENTS 읽기 제한 규칙 적용.
- ProjectSSOT 및 CombatPlan 경로와 운영 규칙 정정.
- 링크 감사 체크리스트 확장.
- 실제 적용 결과와 미수정 범위를 기록하고 문서 상태를 `Completed`로 변경.

### v0.1 - 2026-07-14

- CarFight 문서체계 개선 계획 최초 작성.
- 현재 실제 Plan 및 Systems 상태를 기준선으로 기록.
- 문서 생성과 공식 승격 분리, 상위 갱신 최대 1개, 작업별 라우팅, 검색 제외 규칙을 확정.
- 사용자의 실행 지시에 따라 문서 상태를 `Approved`로 설정.

---

## 15. Migration

### v0.9 적용 완료 안내

- 브라우저 AI 세션의 책임은 실제 코드 수정이 아니라 `plan.*`으로 TaskSource와 최종 Codex YAML 작업지시서를 생성하고 사용자에게 경로를 전달하는 것이다.
- `plan.*` 기능은 Codex 실행기가 아니며, 브라우저 세션에 Codex 실행 도구가 연결되지 않은 상태는 정상이다.
- 실제 코드 수정과 자동 검증은 사용자가 여는 별도 Codex 세션 또는 사용자가 선택한 Codex 환경에서 수행한다.
- 브라우저 세션은 `quality_gate.passed`, `evidence_gate.passed`, `final_output_ready == true`와 최종 YAML 경로를 확인한 뒤 `Ready for External Codex`로 종료할 수 있다.
- 외부 Codex가 아직 실행되지 않았다면 `External Codex Execution: Not Run`으로 기록하며 이를 실패나 차단으로 처리하지 않는다.
- 작업지시서 생성 차단은 `plan.*` 사용 불가, 품질 게이트 미해결, 안전한 범위 확정 불가 또는 사용자 결정 필요일 때만 적용한다.
- 기존 문서의 `Codex 실행 계약`은 `최종 Codex 작업지시서`, `Codex 실행 결과`는 `외부 Codex 실행 상태`로 해석한다.
- 이번 v0.9 작업은 문서 규칙 교정만 수행했으므로 소스 수정, 빌드, PIE, 외부 Codex 실행, commit과 push는 수행하지 않았다.

### v0.8 적용 완료 안내 — 폐기됨, v0.9가 대체

> 아래 내용은 변경 이력 보존용이며 현재 작업 판단에는 사용하지 않는다.

- v0.7의 `Plan 또는 Codex 기능 사용 불가 시 직접 수정 가능` 해석은 폐기한다.
- 모든 새 브라우저 세션의 코드·설정·스크립트 작업은 사용자 시작 문구와 관계없이 `루트 AGENTS.md → Document/CodeWorkGate.md → Document/Document_Entry.md` 순서로 진입한다.
- 실제 코드 변경 전 대표 Plan, TaskSource와 Codex 실행 계약의 실제 경로가 존재해야 한다.
- Plan 또는 Codex 기능을 사용할 수 없으면 `Blocked — Plan/Codex Unavailable`로 중단하고 차단 원인, 직접 수정 위험과 대상 범위를 사용자에게 보고한다.
- 직접 수정 예외는 위 보고 이후 사용자가 웹브라우저 AI의 직접 수정을 명시적으로 승인한 경우에만 허용한다.
- 일반적인 `수정해줘`, `구현해줘`, `계속해줘`는 직접 수정 승인으로 해석하지 않는다.
- 대표 Plan에는 실행 방식, TaskSource, Codex 계약, Codex 실행 결과, 브라우저 검수, 빌드와 PIE 상태를 기록한다.
- 계약 없이 수행된 기존 변경은 자동으로 되돌리지 않지만 기능 검증과 별개로 `Workflow Compliance FAIL`, `Policy Violation` 또는 `Provenance Missing`으로 기록한다.
- 사후 TaskSource나 Codex 계약을 생성해 과거 직접 수정의 실행 출처를 Codex로 변경하지 않는다.
- 이번 v0.8 작업은 문서 구조와 기존 변경의 출처 감사만 수행했으므로 코드 수정, Unreal 빌드, PIE, Codex 실행, commit과 push는 수행하지 않았다.

### v0.7 적용 완료 안내

- 앞으로 웹브라우저 AI가 실제 코드 변경을 수행해야 할 때는 해당 저장소의 Git 상태, SSOT·Systems·대표 Plan과 실제 코드를 먼저 확인한다.
- 기본 실행 경로는 `plan.*` 기능으로 TaskSource와 Codex 실행 계약을 생성하고 Codex가 구현·검증을 수행하는 방식이다.
- TaskSource와 Codex 계약은 작업 소유 저장소의 Plan 폴더에 저장하며 독립 저장소 경계를 넘지 않는다.
- 웹브라우저 AI는 Codex 완료 보고가 아니라 실제 Git diff, 빌드·테스트·PIE와 저장소별 검증 증거를 확인한 뒤 완료 상태를 기록한다.
- 검수 미달 항목은 계약을 보완해 Codex에 재작업을 요청하며 브라우저 AI가 임시로 직접 패치하지 않는다.
- 브라우저 AI의 직접 코드 수정은 사용자의 명시적 직접 수정 요청 또는 Plan·Codex 기능 사용 불가로 작업이 차단된 경우에만 허용하며 사유와 범위를 먼저 알린다.
- Codex가 이미 작업지시서를 실행하는 단계에서는 새 Codex 위임을 재귀적으로 만들지 않는다.
- 문서 전용 변경, 읽기 전용 분석·리뷰와 UE 에디터의 Blueprint·바이너리 에셋 수동 변경은 기존 방식으로 진행한다.
- 이번 v0.7 적용은 문서 규칙 변경이므로 Codex 작업지시서 생성, Unreal Editor 빌드와 코드 테스트는 실행하지 않았다.

### v0.6 적용 완료 안내

- CarFight `Document/`는 `main_game` 게임 프로젝트의 작업 상태와 계획만 관리한다.
- AssetDump 내부 상태는 `UE/Plugins/ue-assetdump/Documents/`, GoPyMCP 내부 상태는 `GoPyMCP/Workspace/docs/`에서만 관리한다.
- 기존 CarFight `AssetDumpPlan`은 공식 색인에서 제거하고 경계 오류 이력을 남기는 migration stub으로만 유지한다.
- AssetDump와 GoPyMCP는 각각 자체 `AGENTS.md`, `Document_Entry.md`, `ActiveWork.md`와 Plan 색인을 사용한다.
- 세션 복원 전에는 사용자 요청이 CarFight, AssetDump, GoPyMCP 중 어느 저장소 소유인지 먼저 판별한다.
- CarFight 문서에는 독립 도구의 공개 command·tool contract, 요구 schema·버전, 사용 위치와 호환성 조건만 기록한다.
- 독립 저장소의 내부 Task, 릴리스 gate, TaskSource, Codex 계약, 검증 체크포인트와 다음 작업을 CarFight 문서로 복사하지 않는다.
- 문서 전용 경계 수정이므로 Unreal Editor 빌드와 각 저장소 테스트는 실행하지 않았다.

### v0.5 적용 완료 안내

- 기존 폴더와 색인 경로는 변경하지 않는다.
- 전체 문서 아키텍처의 공식 기준은 `Document/Document_Entry.md` v2.4다.
- 현재 구현 판정은 Git 상태와 실제 코드·에셋, 관련 Systems 문서를 우선한다.
- 프로젝트 우선순위와 기능 착수 승인은 `ProjectSSOT/02_Roadmap.md`와 `03_FeatureQueue.md`가 담당한다.
- `ActiveWork.md`는 현재 활성 작업과 대표 Plan을 연결하고, 대표 Plan은 상세 체크포인트와 남은 검증을 관리한다.
- `ProjectSSOT/CombatPlan`은 장기 전투 설계이며 일반 `Document/Plan/<Feature>`는 현재 착수 기능의 구현 계획이다.
- 코드·공식 빌드 완료와 사용자 PIE 검증 완료를 별도 상태로 기록한다.
- Draft, Working, Notes, Candidate, Active Plan, Current System, Deferred, Done, Archived 공식 상태 모델을 적용한다.

### v0.4 적용 완료 안내

- 기존 FeatureQueue, Plan Index, Systems Index와 ProjectSSOT의 역할은 변경하지 않는다.
- `Document/ActiveWork.md`는 현재 실제로 진행 중인 작업, 마지막 작업 초점, 의존 관계와 대표 체크포인트 경로만 관리한다.
- 각 작업의 상세 중단 상태는 해당 대표 Plan의 `현재 작업 체크포인트`에 유지한다.
- 세션 이동 전 사용자는 `새 세션 인계 준비해줘`만 요청하면 된다.
- 새 세션에서는 `이전 작업 이어서 진행해줘` 또는 `<작업명/ID> 이어서 진행해줘`로 복원을 시작한다.
- 브라우저가 예고 없이 중단된 경우 마지막으로 저장된 체크포인트까지를 복원 기준으로 사용한다.
- 체크포인트와 실제 저장소가 충돌하면 Git 상태와 실제 코드·에셋을 우선한다.
- 장문의 대화 요약이나 세션별 인수인계 로그 파일은 만들지 않는다.

### v0.3 적용 완료 안내

- 새 세션은 저장소 루트 `AGENTS.md`의 규칙에 따라 Git 상태를 확인하고 `Document/Document_Entry.md`에서 작업별 문서로 진입한다.
- 이전 대화와 저장소가 충돌하면 현재 저장소와 ProjectSSOT를 우선한다.
- 기존 미커밋 변경은 임의로 정리하거나 되돌리지 않는다.
- 실제 변경이나 기능·검증 상태 전환이 있는 작업만 관련 대표 Plan 또는 현재 구현 문서에 종료 상태를 남긴다.
- 종료 기록에는 현재 상태, 완료 범위, 미검증 항목, 다음 작업, 관련 코드·문서 경로를 포함한다.
- 사용자가 직접 확인하지 않은 PIE 결과는 PASS 또는 Completed로 승격하지 않는다.
- 사용자는 별도의 세션 인수인계 문구나 상태 문서를 직접 작성할 필요가 없다.

### v0.2 적용 완료 안내

- 기존 문서 파일과 폴더는 이동하거나 삭제하지 않았다.
- Plan 루트는 폴더와 대표 진입 문서 단위로 관리한다.
- Draft, Working, Notes는 공식 색인 등록 대상에서 제외한다.
- Active Plan 승격 시 `Document/Plan/README.md`만 갱신한다.
- Current System 승격 시 `Document/Systems/SystemIndex.md`만 갱신한다.
- 완료된 현재 구현은 계속 `Document/Systems/`를 우선한다.
- AI/Codex는 `Document/Document_Entry.md`를 먼저 읽고 작업별 색인에서 필요한 문서만 선택한다.
- Archive, Generated, Image, `.git`은 기본 검색 범위에서 제외하며 명시적인 조사 목적이 있을 때만 선택한다.
- CombatPlan의 실제 경로는 `Document/ProjectSSOT/CombatPlan/`이다.

### v0.1 적용 전 안내

- 기존 문서 파일과 폴더는 이동하거나 삭제하지 않는다.
- Plan 루트에 폴더 단위 색인을 추가하지만 개별 Plan 파일을 모두 등록하지 않는다.
- Draft, Working, Notes는 공식 색인 등록 대상에서 제외한다.
- 완료된 현재 구현은 계속 `Document/Systems/`를 우선한다.
- AI/Codex는 `Document_Entry.md`와 해당 작업 색인을 먼저 읽고 전체 `Document/` 재귀 탐색을 피한다.
