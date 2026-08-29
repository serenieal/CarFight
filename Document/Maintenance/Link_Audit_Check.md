# Link Audit Checklist (Document)

- 문서 버전: v3.1
- 최근 갱신일: 2026-08-29
- 문서 상태: Current
- 역할: `Document/`의 경로, 문서 역할, 색인 등록, 기본 읽기 범위를 점검하는 체크리스트

---

## 1. 목적

이 문서는 CarFight 문서체계에서 다음 오류를 점검할 때 사용한다.

```text
- 존재하지 않는 파일 또는 과거 경로 참조
- Plan과 Systems의 역할 혼동
- Draft·Working·Notes의 불필요한 공식 색인 등록
- 개별 파일 추가로 인한 상위 문서 과잉 갱신
- Archive, Generated, 이미지, 내부 Git 데이터의 기본 검색 노출
- Document_Entry가 작업 라우터가 아니라 전체 파일 목록으로 비대해지는 문제
```

---

## 2. 우선 점검 문서

### 2.1 총괄 진입과 운영 규칙

- [ ] `Document/Document_Entry.md`가 존재한다.
- [ ] `Document/AGENTS.md`가 존재한다.
- [ ] `Document/Document_Entry.md`가 개별 파일 전체 목록이 아니라 작업별 진입 라우터로 작성되어 있다.
- [ ] `Document/AGENTS.md`가 먼저 `Document/Document_Entry.md`를 읽도록 지시한다.

### 2.1.1 ActiveWork 세션 복원

- [ ] `Document/ActiveWork.md`가 존재하고 문서 상태가 `Current`다.
- [ ] `ActiveWork.md`에는 Candidate·Deferred가 아니라 실제 활성 작업만 등록되어 있다.
- [ ] 마지막 작업 초점이 현재 활성 작업 표의 작업 ID와 대표 체크포인트를 가리킨다.
- [ ] 각 대표 체크포인트 경로가 실제로 존재한다.
- [ ] 각 활성 대표 Plan에 `현재 작업 체크포인트` 섹션이 존재한다.
- [ ] 체크포인트에 현재 상태, 완료 범위, 미검증 범위, 빌드 상태, PIE 상태와 바로 다음 작업이 기록되어 있다.
- [ ] `ActiveWork.md`와 대표 Plan의 상태가 FeatureQueue, Git 상태와 실제 코드에 모순되지 않는다.
- [ ] 특정 작업명 또는 작업 ID를 지정한 재개 요청이 마지막 작업 초점보다 우선하도록 규칙에 명시되어 있다.

### 2.1.2 전체 문서 아키텍처

- [ ] `Document/Document_Entry.md`가 코드 작업에서 `AGENTS → CodeWorkGate → Document_Entry → ActiveWork → ProjectSSOT → Plan → 코드·에셋 → Systems → Archive` 전체 흐름을 공식 정의한다.
- [ ] DesignSource, 공용 SSOT와 Link Audit의 보조 책임이 전체 구조에 포함되어 있다.
- [ ] 각 문서 영역의 책임과 비책임이 분리되어 있다.
- [ ] ActiveWork가 작업 선택과 대표 Plan 연결만 담당하고 상세 체크포인트를 복사하지 않는다.
- [ ] 대표 Plan이 완료 범위, 미검증 범위, 빌드·PIE 상태와 다음 작업을 담당한다.
- [ ] `ProjectSSOT/CombatPlan`이 장기 전투 설계이고 일반 `Document/Plan/<Feature>`가 현재 착수 기능의 구현 계획으로 구분된다.
- [ ] 현재 구현, 다음 작업, 장기 방향을 판단하는 문서 권한 우선순위가 각각 정의되어 있다.
- [ ] Draft, Working, Notes, Candidate, Active Plan, Current System, Deferred, Done, Archived 상태 모델이 정의되어 있다.
- [ ] `ProjectSSOT/README.md`가 ActiveWork 역할을 포함하면서 Roadmap과 FeatureQueue를 대체하지 않는다고 명시한다.
- [ ] Systems 색인이 코드·빌드 완료와 사용자 PIE 검증 완료를 구분한다.

### 2.1.3 독립 저장소 경계

- [ ] CarFight `Document/ActiveWork.md`에 AssetDump 또는 GoPyMCP의 내부 작업 ID와 마지막 작업 초점이 등록되지 않았다.
- [ ] CarFight `Document/Plan/README.md`에 AssetDump 또는 GoPyMCP의 내부 Plan이 공식 Active Plan으로 등록되지 않았다.
- [ ] `Document/Plan/Archive/AssetDumpPlan/README.md`가 공식 체크포인트가 아니라 폐기·이관 안내 문서로 표시되어 있다.
- [ ] `UE/Plugins/ue-assetdump/AGENTS.md`가 존재한다.
- [ ] `UE/Plugins/ue-assetdump/Documents/Document_Entry.md`가 존재한다.
- [ ] `UE/Plugins/ue-assetdump/Documents/ActiveWork.md`가 존재한다.
- [ ] `UE/Plugins/ue-assetdump/Documents/Plan/README.md`가 존재한다.
- [ ] `GoPyMCP/AGENTS.md`가 GoPyMCP 독립 문서 진입 규칙을 포함한다.
- [ ] `GoPyMCP/Workspace/docs/Document_Entry.md`가 존재한다.
- [ ] `GoPyMCP/Workspace/docs/ActiveWork.md`가 존재한다.
- [ ] `GoPyMCP/Workspace/docs/plan/README.md`가 존재한다.
- [ ] CarFight 문서에는 독립 도구의 공개 계약과 사용 위치만 기록하고 내부 Task, 릴리스 상태와 체크포인트를 복사하지 않는다.
- [ ] 작업 복원 전에 CarFight, AssetDump, GoPyMCP 중 소유 저장소를 먼저 판별하도록 라우터에 명시되어 있다.

### 2.1.4 AGENTS와 Browser 작업 대문

- [ ] CarFight 루트 `AGENTS.md`가 저장소 전체의 기본 작업 대문으로 존재한다.
- [ ] `Document/AGENTS.md`가 루트 규칙을 대체하지 않고 Document 전용 차이만 추가한다.
- [ ] Browser MCP가 대상 경로에 적용 가능한 `AGENTS.md`를 configured repository 루트부터 가장 가까운 경로 순서로 `repository_instructions`에 제공한다.
- [ ] 제공된 `AGENTS.md` 경로와 SHA-256이 동일 `client_request_id` 범위에서 기록된다.
- [ ] `repository_instructions`가 정책 확인을 돕는 소프트 게이트로 운영되며 별도의 서버 측 쓰기 하드 게이트를 필수로 요구하지 않는다.
- [ ] 세션 캐시 만료나 재시작만을 이유로 정상 작업을 영구 차단하는 문서 규칙이 없다.
- [ ] 하위 `AGENTS.md`는 해당 경로에 실제 차별 규칙이 있을 때만 존재하고 상위 규칙을 반복 복사하지 않는다.
- [ ] 더 가까운 하위 `AGENTS.md`가 존재하면 상위 규칙에 추가하거나 명시적으로 재정의하는 범위가 분명하다.
- [ ] `Document/Plan/AGENTS.md`가 별도 configured repository인 `plan_repo`의 루트 대문으로 존재한다.
- [ ] `Document/Plan/AGENTS.md`가 Plan의 논리적 CarFight 소유권과 별도 Git 저장소 경계를 구분한다.
- [ ] Plan 작업이 현재 상태나 구현을 판단할 때 main_game의 ActiveWork, ProjectSSOT, Systems와 실제 코드·에셋을 교차검증하도록 안내한다.
- [ ] `Document/CodeWorkGate.md`가 존재하고 문서 상태가 `Current`다.
- [ ] CarFight 코드 작업의 기본 경로가 현재 AI 세션의 직접 구현, diff 검수와 가능한 빌드·테스트로 정의되어 있다.
- [ ] TaskSource, WorkOrder와 Codex YAML이 선택 산출물이며 직접 구현의 필수 착수 조건으로 사용되지 않는다.
- [ ] 별도 Codex 위임은 사용자가 명시적으로 요청한 경우에만 선택하도록 정의되어 있다.
- [ ] 기존 dirty 변경 보호와 안전한 최소 수정 범위가 작업 시작 규칙에 포함되어 있다.
- [ ] 사용자의 명시적 요청 없는 commit·push·reset·checkout·stash 금지가 유지된다.
- [ ] 코드 적용 후 실제 Git diff, 빌드·자동 테스트와 사용자 PIE 증거를 구분해서 검수한다.
- [ ] 과거 TaskSource, WorkOrder, Codex YAML과 실행 상태는 역사 기록으로 보존하고 사후에 실행 출처를 바꾸지 않는다.
- [ ] AssetDump와 GoPyMCP가 각각 자체 저장소 루트 `AGENTS.md`와 독립 문서 진입점을 사용한다.

### 2.2 ProjectSSOT

- [ ] `Document/ProjectSSOT/README.md`
- [ ] `Document/ProjectSSOT/00_Vision.md`
- [ ] `Document/ProjectSSOT/01_ProjectState.md`
- [ ] `Document/ProjectSSOT/02_Roadmap.md`
- [ ] `Document/ProjectSSOT/03_FeatureQueue.md`
- [ ] `Document/ProjectSSOT/04_ProjectDecisions.md`
- [ ] `Document/ProjectSSOT/05_TestChecklist.md`

### 2.3 Plan 색인

- [ ] `Document/Plan/README.md`가 존재한다.
- [ ] `Document/Plan/Archive/README.md`가 존재한다.
- [ ] `Document/Plan/README.md`는 개별 파일이 아니라 Plan 폴더와 대표 진입 문서 단위로 관리된다.
- [ ] `Document/Plan/Archive/README.md`는 Archive가 현재 착수 기준이 아니라고 명시한다.
- [ ] `Document/Plan/` 루트에는 `AGENTS.md`, `README.md` 같은 저장소 대문 외 Current Plan 문서가 직접 흩어져 있지 않다.
- [ ] Active / Paused / Ready Plan은 `Document/Plan/<Feature>/` 기능 폴더 단위로 묶여 있다.
- [ ] 같은 Feature의 Plan / Roadmap / Design / Spec은 가능한 한 같은 기능 폴더에서 탐색된다.
- [ ] Done / Superseded / Deprecated 문서는 Current root가 아니라 `Document/Plan/Archive/<Feature>/`에서 탐색된다.
- [ ] `Generated/Intermediate`와 중복 임시 예시가 Current Plan 경로에 찌꺼기로 남아 있지 않다.

### 2.4 Systems 색인

- [ ] `Document/Systems/SystemIndex.md`가 존재한다.
- [ ] 현재 구현 문서는 해당 폴더 섹션과 기능별 찾기 표에서 접근할 수 있다.
- [ ] 아직 구현되지 않은 기능이 완료된 Systems 문서처럼 등록되지 않았다.

### 2.5 CombatPlan

- [ ] `Document/ProjectSSOT/CombatPlan/README.md`가 존재한다.
- [ ] `Document/ProjectSSOT/CombatPlan/00_Index.md`가 존재한다.
- [ ] 활성 문서가 잘못된 `Document/Plan/CombatPlan/` 경로를 안내하지 않는다.
- [ ] CombatPlan은 `00_Index.md`에서 필요한 번호 문서만 선택하도록 안내한다.

---

## 3. 문서 역할 분리 점검

- [ ] `Document/ProjectSSOT/`가 현재 프로젝트 판단 기준으로 설명된다.
- [ ] `Document/Plan/`이 앞으로 할 작업과 남은 검증을 설명한다.
- [ ] `Document/Systems/`가 현재 완료된 구현 기준을 설명한다.
- [ ] `Document/Plan/Archive/`가 완료·보류·대체된 과거 계획으로 설명된다.
- [ ] `Document/DesignSource/`가 장기 방향과 원본 기획 참고로 설명된다.
- [ ] `Document/SSOT/`를 CarFight 전용 수정 대상으로 오해하게 만드는 표현이 없다.
- [ ] CarFight 전용 판단을 공용 `Document/SSOT/`에 기록하지 않는다.
- [ ] 도구 자체 설명과 MCP 실행 절차를 CarFight `Document/` 안에 중복 기록하지 않는다.

---

## 4. 문서 생성과 공식 승격 점검

- [ ] Draft 문서는 공식 색인 갱신 없이 유지할 수 있다.
- [ ] Working 문서는 공식 색인 갱신 없이 유지할 수 있다.
- [ ] Notes 문서는 공식 색인 갱신 없이 유지할 수 있다.
- [ ] 새 Active Plan 폴더만 `Document/Plan/README.md`에 등록한다.
- [ ] 새 Current System 문서만 `Document/Systems/SystemIndex.md`에 등록한다.
- [ ] Plan 하위 세부 파일이 추가될 때마다 `ProjectSSOT/README.md`를 수정하지 않는다.
- [ ] 일반 문서 변경에서 상위 색인 갱신이 최대 1개인지 확인한다.

2개 이상의 상위 문서 갱신은 다음 예외인지 확인한다.

- [ ] 최상위 구조 변경
- [ ] 프로젝트 방향 또는 ProjectSSOT 판단 기준 변경
- [ ] Plan에서 Systems로 공식 승격
- [ ] 문서 생명주기 또는 작업 라우팅 변경
- [ ] 깨진 링크 또는 실제 경로 불일치 수정

---

## 5. 링크 무결성 점검

- [ ] `Document_Entry.md`의 Quick Links가 실제 파일과 일치한다.
- [ ] `ProjectSSOT/README.md`의 Plan 색인 경로가 실제 파일과 일치한다.
- [ ] `ProjectSSOT/README.md`의 CombatPlan 경로가 실제 위치와 일치한다.
- [ ] `CombatPlan/README.md`의 구조 표가 실제 파일명과 일치한다.
- [ ] `Plan/README.md`의 대표 진입 문서가 실제로 존재한다.
- [ ] `Plan/Archive/README.md`의 대표 진입 문서가 실제로 존재한다.
- [ ] 이동하거나 이름을 바꾸지 않은 문서를 새 경로로 잘못 안내하지 않는다.
- [ ] README 간 순환 참조만 반복하고 실제 대표 문서에 도달하지 못하는 구조가 없다.
- [ ] `Document_Entry → 폴더 색인 → 대표 문서` 흐름이 유지된다.

---

## 6. 기본 검색 제외 점검

다음 경로가 기본 검색 제외 규칙에 포함되어 있는지 확인한다.

- [ ] `Document/**/.git/**`
- [ ] `Document/Plan/Archive/**`
- [ ] `Document/Plan/**/Generated/**`
- [ ] `Document/Plan/ConceptArt/Image/**`
- [ ] `Document/SSOT/**/.git/**`
- [ ] `Document/SSOT/**/Archive/**`
- [ ] `Document/SSOT/UE_SSOT/BP_Text_Format/Archive/**`
- [ ] `Document/SSOT/UE_SSOT/BP_Text_Format/cases/**`

예외적으로 읽을 때 다음 조건 중 하나가 명확한지 확인한다.

- [ ] 사용자가 과거 기록을 명시적으로 요청함
- [ ] 현재 문서와 과거 결정의 충돌 원인을 조사함
- [ ] Migration 또는 회귀 원인을 추적함
- [ ] 이미지 자체가 작업 입력임

---

## 7. 상태와 최신성 점검

- [ ] 문서 상단 버전과 Changelog의 최신 버전이 일치한다.
- [ ] 최근 갱신일이 실제 변경 날짜와 일치한다.
- [ ] 문서 상태가 `Draft / Working / Current / Active / Completed / Archived` 등의 실제 생명주기와 맞는다.
- [ ] Archive 내부 문서의 과거 `Active` 또는 `Draft` 표기를 현재 착수 상태로 오해하지 않도록 상위 Archive 색인이 안내한다.
- [ ] Systems 문서의 구현 완료·PIE Pending·Known Limit 상태가 실제 검증 결과와 일치한다.

---

## 8. 점검 결과 기록

- 점검일: `2026-07-22`
- 점검 범위: `ActiveWork / ProjectSSOT / Plan / Systems 색인의 CF-FQ-025 완료 및 CF-FQ-024 활성 상태 정합성`
- 확인한 문서 수: `13개`
- 깨진 링크 수: `0건`
- 역할·상태 불일치 수: `5건`
- 수정 완료 수: `5건`
- 보류 수: `0건`
- 비고:
  - 이번 기록은 전체 저장소 전수 감사가 아니라 이번 수정 대상과 직접 교차검증 문서에 한정한다.
  - 기존 사용자 미커밋 CombatPlan 변경 2건과 미추적 DOCX는 점검·수정 범위에서 제외하고 보존했다.
  - CF-FQ-025는 완료 Plan과 Current Systems로, CF-FQ-024는 현재 활성 Plan과 세션 복원 초점으로 정렬했다.

---

## 9. Changelog

### v3.1 - 2026-08-29

- Plan root 평면 파일 누적을 막기 위해 Current Plan의 기능 폴더 배치 검사를 추가했다.
- Done/Superseded/Deprecated의 Archive 물리 배치와 Generated Intermediate 잔여물 검사를 추가했다.
- AssetDump migration stub의 실제 경로를 `Document/Plan/Archive/AssetDumpPlan/README.md`로 교정했다.

### v3.0 - 2026-07-30

- Plan/Codex 강제 작업지시서 감사를 AGENTS 기반 Browser 작업 대문과 소프트 게이트 감사로 교체했다.
- 루트에서 가장 가까운 `AGENTS.md`까지의 적용 순서, 경로·SHA-256 기록과 configured repository 경계를 점검하도록 변경했다.
- `Document/Plan/AGENTS.md` 존재와 plan_repo의 논리적·Git 소유권 분리를 감사 항목에 추가했다.
- 현재 AI 세션 직접 구현, 선택적 TaskSource·WorkOrder·Codex와 사용자 명시 외부 위임 기준을 `CodeWorkGate.md v2.0`에 맞췄다.
- 기존 dirty 변경 보호, Git 쓰기 금지, diff·빌드·테스트·PIE 증거 분리 점검은 유지했다.

### v2.9 - 2026-07-22

- CF-FQ-025 완료와 CF-FQ-024 활성 상태의 ActiveWork, ProjectSSOT, Plan과 Systems 색인 정합성 점검 결과를 기록했다.
- ActiveWork의 완료 작업 초점, ProjectSSOT와 SystemIndex의 완료 기준 누락을 정리했다.
- FeatureQueue와 ProjectDecisions의 상단·내부 버전 불일치를 정정했다.
- 이번 결과가 전체 저장소 전수 감사가 아니라 대상 문서 정합성 점검임을 명시했다.

### v2.8 - 2026-07-16

- Plan/Codex 감사 기준을 실제 Codex 실행 여부가 아니라 TaskSource·최종 YAML 작업지시서 생성 여부 중심으로 변경.
- Codex 실행 도구 미연결이 정상 상태이며 차단 사유가 아닌지 점검 추가.
- `quality_gate`, `evidence_gate`, `final_output_ready`와 `Ready for External Codex` 상태 점검 추가.
- 작업지시서 준비 상태, 외부 Codex 실행 상태와 브라우저 후속 검수 상태의 분리 여부 점검 추가.
- 실제 차단 기준과 독립 저장소별 작업지시서 검증 요구사항을 갱신.

### v2.7 - 2026-07-16

- `CodeWorkGate.md` 존재, Current 상태와 루트 AGENTS 최상단 우선 적용 여부 점검 추가.
- 모든 새 세션의 코드 작업 게이트 진입과 TaskSource·Codex 계약 사전 존재 여부 점검 추가.
- Plan·Codex 사용 불가 시 자동 직접 수정 금지와 사용자 사후 명시 승인 조건 점검 추가.
- 대표 Plan의 실행 출처 필드와 `Workflow Compliance FAIL`, `Policy Violation`, `Provenance Missing` 판정 점검 추가.
- 과거 직접 수정 출처를 사후 Codex 계약으로 위조하지 않는지 점검 추가.

### v2.6 - 2026-07-14

- Plan/Codex 코드 위임 감사 섹션 추가.
- 브라우저 AI와 Codex 실행 주체 구분, 재귀 위임 금지와 기본 위임 경로 점검 추가.
- TaskSource·Codex 계약의 필수 범위, 보호 조건, 완료·실패 조건과 Git 쓰기 금지 점검 추가.
- 실제 diff·빌드·테스트·PIE 증거 검수와 미달 시 Codex 재작업 여부 점검 추가.
- CarFight, AssetDump와 GoPyMCP 저장소별 Codex 위임·검증 규칙 점검 추가.

### v2.5 - 2026-07-14

- CarFight, AssetDump, GoPyMCP의 독립 저장소 경계 점검 섹션 추가.
- CarFight ActiveWork와 Plan Index에 독립 도구 내부 작업이 등록되지 않았는지 점검 추가.
- AssetDump와 GoPyMCP의 독립 AGENTS, Document Entry, ActiveWork와 Plan Index 존재 여부 점검 추가.
- CarFight에는 공개 도구 계약과 사용 위치만 기록하도록 감사 기준 추가.
- 세션 복원 전에 작업 소유 저장소를 판별하는지 점검 추가.

### v2.4 - 2026-07-14

- 전체 문서 아키텍처 공식 흐름 존재 여부 점검 추가.
- 각 문서 영역의 책임과 비책임 분리 여부 점검 추가.
- ActiveWork와 대표 Plan, CombatPlan과 일반 Plan의 책임 경계 점검 추가.
- 현재 구현·다음 작업·장기 방향별 문서 권한 우선순위 점검 추가.
- Candidate, Deferred, Done, Archived를 포함한 공식 상태 모델 점검 추가.
- ProjectSSOT의 ActiveWork 역할과 Systems의 코드·빌드·PIE 상태 구분 점검 추가.

### v2.3 - 2026-07-14

- `Document/ActiveWork.md` 존재, 상태와 역할 점검 항목 추가.
- 마지막 작업 초점과 활성 작업 표·대표 체크포인트 연결 검증 추가.
- 각 활성 대표 Plan의 표준 `현재 작업 체크포인트` 필드 검증 추가.
- ActiveWork, FeatureQueue, Git 상태와 실제 코드의 상태 정합성 점검 추가.
- 특정 작업 재개 요청이 마지막 작업 초점보다 우선하는지 점검 추가.

### v2.2 - 2026-07-14

- Plan 루트와 Plan Archive 루트 색인 존재 여부 점검 추가.
- Document Entry 작업 라우터, 문서 생성과 공식 승격 분리, 상위 갱신 최대 1개 점검 추가.
- CombatPlan 실제 경로와 `00_Index.md` 선택 읽기 점검 추가.
- Archive, Generated, ConceptArt Image, `.git` 기본 검색 제외 점검 추가.
- 링크뿐 아니라 문서 역할과 상태 최신성 점검까지 범위 확장.

### v2.1 - 2026-06-19

- ProjectSSOT 점검 대상을 현재 파일명 기준으로 갱신.
- 서버·멀티 계획과 Network Systems 문서의 활성 상태 오해 방지 체크 추가.

### v2.0

- Document 쪽 MCP 관련 점검 항목 제거.
- Document 허브, ProjectSSOT, UE SSOT 중심 체크리스트로 단순화.

---

## 10. Migration

### v3.0 적용 안내

- 새 문서 감사는 루트 `AGENTS.md`와 필요한 최소 하위 `AGENTS.md`가 작업 대문 역할을 하는지 우선 확인한다.
- Browser MCP의 `repository_instructions`는 소프트 게이트로 감사하며 서버 측 pre-write 하드 차단의 존재를 요구하지 않는다.
- `Document/Plan`은 별도 configured repository이므로 `Document/Plan/AGENTS.md`가 Plan 작업 규칙을 독립적으로 제공해야 한다.
- CarFight 코드 작업은 현재 AI 직접 구현을 기본으로 감사하고 TaskSource, WorkOrder와 Codex YAML 부재를 정책 위반으로 판정하지 않는다.
- 과거 외부 Codex 강제 정책과 당시 상태는 역사 기록으로 유지하되 현재 작업 판단에 사용하지 않는다.

### v2.8 적용 안내 — 폐기됨, v3.0이 대체

> 아래 내용은 당시 감사 기준 보존용이며 현재 작업 판단에는 사용하지 않는다.

- 코드 작업 감사의 1차 대상은 브라우저 세션에서 생성한 TaskSource와 최종 Codex YAML 작업지시서다.
- Codex 실행 도구 미연결은 오류나 차단 상태로 기록하지 않는다.
- 브라우저 세션은 최종 YAML 생성과 경로 전달 후 `Ready for External Codex`, 외부 실행은 `Not Run`으로 기록할 수 있다.
- 실제 코드 적용 후에는 별도 검수 단계에서 Git diff, 빌드·테스트·PIE 증거를 확인한다.
- 과거 문서의 `Codex 실행 계약`은 최종 작업지시서로, `Codex 실행`은 별도 외부 단계로 해석한다.

### v2.7 적용 안내 — 폐기됨, v2.8이 대체

> 아래 내용은 변경 이력 보존용이며 현재 감사 판단에는 사용하지 않는다.

- v2.6의 직접 수정 예외 감사 기준은 폐기하고 `CodeWorkGate.md`의 사후 명시 승인 조건으로 교체한다.
- 코드 작업 감사에서는 문구 존재 여부뿐 아니라 TaskSource, Codex 계약과 대표 Plan 실행 출처의 실제 경로를 확인한다.
- 계약이 없는 코드 변경은 기능 검증 PASS와 별개로 `Workflow Compliance FAIL`로 판정한다.
- 기존 직접 수정은 자동으로 되돌리지 않지만 사후 계약으로 출처를 바꾸지 않는다.
- 모든 새 브라우저 세션이 사용자 트리거와 관계없이 CodeWorkGate로 진입하는지 확인한다.

### v2.6 적용 안내

- 기존 링크·역할·저장소 경계 감사 항목은 유지한다.
- 앞으로 코드 작업 감사 시 Plan 기능을 통한 TaskSource·Codex 계약 생성 여부와 Codex 실행 후 실제 증거 검수를 함께 확인한다.
- 브라우저 AI의 직접 코드 패치는 명시된 예외에 해당하고 사유·범위를 먼저 알린 경우에만 허용한다.
- AssetDump와 GoPyMCP는 각 저장소 특유의 검증 계약을 Codex 작업지시서에 포함해야 한다.
- 문서 전용 변경과 Blueprint·바이너리 에셋 수동 작업은 코드 위임 감사 대상과 구분한다.

### v2.5 적용 안내

- 기존 링크, 역할, ActiveWork와 전체 문서 아키텍처 점검 항목은 유지한다.
- 앞으로 문서 감사 시 CarFight, AssetDump와 GoPyMCP의 저장소 소유권 경계를 함께 확인한다.
- CarFight ActiveWork와 Plan에는 독립 저장소의 내부 상태가 없어야 한다.
- AssetDump와 GoPyMCP는 각각 자체 `AGENTS.md`, `Document_Entry.md`, `ActiveWork.md`와 Plan 색인을 가져야 한다.
- CarFight 측 기존 `AssetDumpPlan` 파일은 폐기·이관 안내 문서로만 남고 공식 색인에서 접근되지 않아야 한다.

### v2.4 적용 안내

- 기존 링크, 역할, 공식 승격과 ActiveWork 체크포인트 점검 항목은 유지한다.
- 앞으로 전체 문서 감사 시 `Document_Entry.md`의 공식 아키텍처 정의와 각 영역의 책임·비책임을 함께 확인한다.
- ActiveWork와 대표 Plan, CombatPlan과 일반 Plan의 책임 경계가 무너지지 않았는지 점검한다.
- 문서 충돌 시 현재 구현·다음 작업·장기 방향별 권한 우선순위가 적용되는지 점검한다.
- Systems에서는 코드·빌드 완료와 사용자 PIE 검증 완료를 별도 상태로 검사한다.

### v2.3 적용 안내

- 기존 링크·역할·공식 승격 점검 항목은 유지한다.
- 앞으로 문서 감사 시 `Document/ActiveWork.md`와 활성 대표 Plan의 체크포인트 정합성을 함께 확인한다.
- ActiveWork는 세부 구현 내용을 복사하는 문서가 아니라 대표 체크포인트 연결 색인으로 점검한다.
- 체크포인트와 실제 저장소가 다르면 Git 상태와 실제 코드·에셋을 기준으로 불일치를 기록한다.

### 기존 적용 안내

- 기존 점검 항목은 삭제하지 않고 현재 문서체계에 맞게 재분류했다.
- 앞으로 링크 감사는 경로 존재 여부뿐 아니라 역할 분리, 공식 승격, 검색 제외 규칙까지 함께 확인한다.
- `Document/Plan/CombatPlan/`은 올바른 경로가 아니며 실제 `Document/ProjectSSOT/CombatPlan/`을 기준으로 점검한다.
