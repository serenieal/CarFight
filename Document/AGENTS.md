# CarFight Document 작업 규칙

- 문서 버전: v2.2
- 최근 갱신일: 2026-08-19
- 문서 상태: Current
- 적용 범위: `Document/` 이하의 문서 읽기, 작성, 정리, 색인 갱신과 상태 기록

---

## 1. 역할과 상속

이 파일은 CarFight 루트 `AGENTS.md`를 상속하며, `Document/`에 필요한 차이만 추가한다.
Browser MCP가 제공하는 `repository_instructions`는 configured repository 루트부터 가장 가까운 `AGENTS.md` 순서로 적용하고, 명시적으로 다른 규칙은 가까운 파일을 우선한다.

이 파일은 코드 작업 주체나 실행 게이트를 별도로 재정의하지 않는다.
CarFight 코드 작업의 현재 기준은 루트 `AGENTS.md`와 `Document/CodeWorkGate.md`다.

`repository_instructions`는 작업 시작 시 정책을 자연스럽게 확인하게 하는 소프트 게이트다.
별도의 서버 측 pre-write 하드 게이트나 캐시 상태만을 이유로 한 작업 중단을 요구하지 않는다.

하위 `AGENTS.md`는 해당 경로에 실제 차별 규칙이 있을 때만 추가하고 상위 내용을 반복 복사하지 않는다.

---

## 2. 문서 작업 진입

문서 전용 작업은 먼저 다음 라우터에서 작업 종류를 판별한다.

```text
Document/Document_Entry.md
```

코드·설정·스크립트 변경 가능성이 있으면 다음 순서로 시작한다.

```text
저장소 루트 AGENTS.md
→ Document/CodeWorkGate.md
→ Document/Document_Entry.md
```

현재 AI 세션의 직접 구현이 CarFight 코드 작업의 기본 경로다.
TaskSource, WorkOrder와 Codex YAML은 선택 산출물이며 필수 착수 조건이 아니다.
별도 Codex 위임은 사용자가 명시적으로 요청한 경우에만 선택한다.

작업별로 필요한 대표 문서만 선택한다.

```text
현재 프로젝트 판단: Document/ProjectSSOT/README.md
현재 구현 확인: Document/Systems/SystemIndex.md
진행 중 계획 확인: Document/Plan/README.md
활성 작업 복원: Document/ActiveWork.md
```

`Document/` 전체, Plan 전체와 Archive 전체를 작업 시작 입력으로 재귀 탐색하지 않는다.

---

## 3. 문서 역할과 작성 규칙

문서 역할은 다음 구분을 유지한다.

```text
ProjectSSOT = 무엇을 왜 개발하는가
Plan = 앞으로 무엇을 어떻게 개발하고 검증하는가
Systems = 현재 실제로 어떻게 구현되어 있는가
Archive = 현재 착수 기준에서 내려온 과거 기록
DesignSource = 장기 방향과 원본 기획
```

현재 구현은 Git 상태, 실제 코드·에셋과 Systems를 우선한다.
Plan이 더 최근에 수정되었다는 이유만으로 실제 구현이나 Systems를 대체하지 않는다.
미구현 설계를 완료된 Current System처럼 기록하지 않는다.

문서는 어떤 세션과 AI가 읽어도 같은 방향으로 작업할 수 있게 작성한다.
중복을 줄이고 용어를 일관되게 유지하며 사용자가 읽는 항목은 한글로 표기한다.
변경 시 Changelog를 포함하고, 경로·역할·운영 방식이 바뀌면 Migration을 포함한다.

새 문서 생성과 공식 색인 등록을 분리한다.

```text
Draft / Working / Notes
→ 상위 색인 갱신 없음

Active Plan 공식 승격
→ Document/Plan/README.md

Current System 공식 승격
→ Document/Systems/SystemIndex.md
```

일반 문서 변경에서 상위 색인 갱신은 최대 1개로 제한한다.
구조 변경, ProjectSSOT 판단 변경, Plan의 Systems 승격과 깨진 링크 수정은 필요한 관련 문서를 함께 갱신할 수 있다.

---

## 4. 검색 제외와 저장소 경계

명시적 요청이나 직접 작업 대상이 아니면 다음을 기본 검색에서 제외한다.

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

Generated, TaskSource, WorkOrder, Codex와 YAML 산출물은 대표 문서를 먼저 확인한 뒤 실제 필요가 있을 때만 읽는다.

`Document/Plan`은 CarFight 문서의 일부이지만 별도 configured repository인 `plan_repo`다.
상향 탐색이 `plan_repo` 루트에서 끝나므로 Plan 작업은 다음 대문을 사용한다.

```text
Document/Plan/AGENTS.md
```

`UE/Plugins/ue-assetdump`와 `GoPyMCP`도 각각 자체 루트 `AGENTS.md`와 독립 문서체계를 사용한다.
독립 저장소의 내부 작업 상태를 CarFight ActiveWork나 Plan에 복사하지 않는다.

---

## 5. 상태 기록과 검증

실제 코드·에셋, 기능 상태, 빌드·PIE 결과, 중요한 결정이나 다음 작업이 바뀐 경우에만 관련 대표 Plan 또는 Current 문서를 필요한 범위로 갱신한다.
단순 설명, 조사와 질의응답은 상태 문서 갱신을 강제하지 않는다.

`Document/ActiveWork.md`는 활성 작업 선택과 대표 체크포인트 연결만 담당한다.
상세 상태는 대표 Plan과 실제 저장소가 소유한다.
특정 작업명이나 작업 ID가 있으면 마지막 작업 초점보다 해당 작업을 우선한다.

빌드와 PIE 상태를 분리한다. Accepted GoPyMCP UE MCP/`GoPyMCP.RuntimeRead`로 직접 관측한 PIE runtime 기술 사실은 AI Technical PASS로 기록할 수 있지만, 시각 품질·UX·조작감·주행감·조준감·연출 감각은 사용자가 확인하지 않은 상태에서 USER PASS나 Completed로 확대하지 않는다.

검증 기록은 가능한 범위에서 다음을 분리한다.

```text
Validation = 실제 결과
Evidence = Job, 보고서, 로그, 결과 파일 또는 사용자 확인
Execution Method = 고정 프리셋, 공용 도구, 작업 전용 임시 경로 또는 사용자 절차
Reusable Entry Point = 다음 세션에서 그대로 재사용할 공용 진입점의 존재 여부
Historical Scope = 현재 상태 또는 특정 날짜·체크포인트의 당시 상태
```

작업 전용 단발성 스크립트나 임시 실행 경로는 해당 검증의 실행 수단일 뿐 공용 도구로 자동 승격하지 않는다.
Git 미추적 상태만으로 등록이나 재사용이 필요하다고 판단하지 않는다.
과거 문서의 Runner Unavailable, Tool Unavailable과 Not Run 기록은 해당 시점의 역사 상태이며 현재의 영구적인 실행 불가 계약이 아니다.
Current 문서는 최신 검증 증거를 반영하고, 당시 상태는 날짜가 명확한 Changelog나 체크포인트 기록으로 보존한다.

문서 전용 변경은 링크, 실제 경로, 역할, 버전·Changelog와 Current 정책 충돌을 검증하며 코드 빌드를 요구하지 않는다.

신규 문서의 CarFight Editor 빌드 예시는 다음 공식 경로만 사용한다.

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

---

## 6. Changelog

### v2.2 - 2026-08-19

- Accepted `GoPyMCP.RuntimeRead`로 직접 관측 가능한 PIE runtime 기술 사실은 사용자 확인 없이도 AI Technical PASS로 기록할 수 있도록 문서 증거 규칙을 갱신했다.
- USER PASS는 시각 품질, UX, 조작감, 주행감, 조준감과 연출 감각처럼 사람 판단이 필요한 항목에만 유지하며 AI Technical PASS와 구분한다.

### v2.1 - 2026-08-02

- 검증 결과, 증거, 실행 수단, 공용 재실행 진입점과 역사 범위를 분리해서 기록하도록 문서 기준을 추가했다.
- 작업 전용 단발성 스크립트와 임시 실행 경로를 공용 저장소 도구로 자동 승격하지 않도록 명시했다.
- Git 미추적 상태만으로 임시 스크립트 등록이나 재사용을 요구하지 않도록 했다.
- 과거 Runner Unavailable 또는 Not Run을 현재의 영구적인 실행 불가 상태로 해석하지 않고 Current 문서는 최신 증거를 반영하도록 했다.

### v2.0 - 2026-07-30

- 루트 `AGENTS.md` 상속과 가장 가까운 규칙 우선 원칙을 명확히 했다.
- Browser MCP `repository_instructions`를 작업 시작 소프트 게이트로 정의했다.
- 폐기된 TaskSource·최종 Codex YAML 강제 게이트와 직접 수정 Policy Violation 규칙을 제거했다.
- 현재 AI 직접 구현과 선택적 외부 Codex 기준을 `CodeWorkGate.md v2.0`에 맞췄다.
- `plan_repo` 경계와 `Document/Plan/AGENTS.md` 진입 규칙을 추가했다.
- 상세 절차는 `Document_Entry.md`와 Current 문서로 연결하고 이 파일은 대문 역할 중심으로 축약했다.

---

## 7. Migration

- v1.7의 Browser 작업지시서 강제 게이트는 폐기되었으며 현재 판단에 사용하지 않는다.
- 기존 TaskSource, WorkOrder와 Codex YAML은 당시 기록 또는 선택 참고 자료로 유지한다.
- `Document/Plan` 작업은 `Document/Plan/AGENTS.md`에서 시작한다.
- 기존 ProjectSSOT, Plan, Systems, Archive와 ActiveWork 경로는 변경하지 않는다.
- 기존 미커밋 변경은 자동으로 정리하거나 되돌리지 않는다.
- 과거 검증 도구 부재와 Not Run 기록은 날짜가 있는 역사 상태로 유지하며 현재 검증 가능 여부는 최신 도구·저장소·범위·증거로 다시 판단한다.
- 작업 전용 임시 실행 경로는 명시적 공용화 결정이 없는 한 공식 도구나 Git 등록 대상으로 해석하지 않는다.
- PIE 관련 문서 상태를 기록할 때 `AI Runtime Technical Validation`과 `USER Visual/Feel Validation`을 구분한다. Accepted RuntimeRead evidence가 존재하는 기술 사실을 단순히 사용자 미확인이라는 이유로 Pending으로 되돌리지 않는다.
