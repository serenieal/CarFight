# CarFight Document 작업 규칙

- 문서 버전: v1.7
- 최근 갱신일: 2026-07-16
- 적용 범위: `Document/` 이하의 문서 읽기, 작성, 정리, 색인 갱신과 코드 작업 게이트 문서 운영

---

## 1. 기본 원칙

이 경로 이하에서는 프로젝트 문서와 SSOT 문서 작업 규칙을 따른다.

문서 정리, 명세 정제, 설계서 압축, 중복 제거, 구조 개선 요청에는 `doc_curation` skill을 우선 사용한다.

문서는 어떤 세션, 어떤 AI가 읽어도 같은 방향으로 구현할 수 있게 작성한다.
설명은 짧지만 빠진 정보가 없게 작성한다.
중복은 줄이고 용어는 일관되게 유지한다.
사용자가 읽는 항목은 한글로 표기한다.
변경 시 Changelog를 포함하고, 경로·역할·운영 방식이 바뀌면 Migration도 포함한다.

---

## 2. 문서 읽기 시작 규칙

문서 전용 작업은 먼저 다음 문서를 읽는다.

```text
Document/Document_Entry.md
```

실제 코드·설정·스크립트 변경 가능성이 있는 작업은 사용자 문구와 관계없이 먼저 다음 순서를 적용한다.

```text
루트 AGENTS.md
→ Document/CodeWorkGate.md
→ Document/Document_Entry.md
```

그 다음 `Document_Entry.md`의 작업별 라우팅에 따라 필요한 색인 문서와 대표 문서만 선택한다.
`CodeWorkGate.md`를 확인하기 전에는 소스·설정·스크립트 쓰기 도구를 호출하지 않는다.

금지:

```text
- 작업 종류를 판별하기 전에 Document 전체 재귀 검색
- Plan 전체를 현재 구현 기준으로 읽기
- Archive 전체를 현재 착수 기준으로 읽기
- Generated, 이미지, 내부 Git 데이터를 기본 문서 검색에 포함
```

현재 구현 확인은 `Document/Systems/SystemIndex.md`를 우선한다.
진행 중 계획 확인은 `Document/Plan/README.md`를 사용한다.
프로젝트 현재 판단은 `Document/ProjectSSOT/README.md`를 사용한다.

이전 세션을 복원하거나 여러 활성 작업 중 특정 작업으로 전환할 때는 `Document/ActiveWork.md`를 추가로 읽는다.
`ActiveWork.md`에서는 작업 ID와 대표 체크포인트만 선택하고, 상세 상태는 해당 대표 Plan과 실제 저장소에서 확인한다.

### 2.1 독립 저장소 경계

CarFight `Document/`는 `main_game` 게임 프로젝트만 관리한다.

```text
UE/Plugins/ue-assetdump
→ UE/Plugins/ue-assetdump/Documents/Document_Entry.md

GoPyMCP
→ GoPyMCP/Workspace/docs/Document_Entry.md
```

AssetDump와 GoPyMCP의 내부 활성 작업, 릴리스 상태, TaskSource, Codex 계약과 체크포인트를 CarFight `ActiveWork.md`, `Plan/README.md` 또는 ProjectSSOT에 등록하지 않는다.
CarFight가 도구의 공개 계약에 의존할 때는 CarFight 사용 위치와 요구 계약만 기록한다.
독립 저장소 문서를 수정할 때는 해당 저장소의 Git 상태와 가장 가까운 `AGENTS.md`를 먼저 확인한다.

---

## 3. 기본 검색 제외

사용자가 명시적으로 요청하거나 해당 경로가 직접 작업 대상인 경우가 아니면 다음을 기본 검색에서 제외한다.

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

`Generated`, `Codex`, `TaskSource_*`, YAML 계약 파일은 대표 설계 문서를 먼저 확인한 뒤 실제 구현 지시가 필요할 때만 읽는다.

---

## 4. 문서 생성과 공식 승격

문서 생성과 공식 색인 등록을 분리한다.

```text
Draft / Working / Notes
→ 상위 색인 갱신 없음

Active Plan 공식 승격
→ Document/Plan/README.md만 갱신

Current System 공식 승격
→ Document/Systems/SystemIndex.md만 갱신

Archive 이동
→ 필요한 경우 Document/Plan/Archive/README.md 갱신
```

새 파일이 생겼다는 사실만으로 `Document_Entry.md`, `ProjectSSOT/README.md`, `Plan/README.md`, `SystemIndex.md`를 모두 수정하지 않는다.

일반적인 문서 변경에서 상위 색인 갱신은 최대 1개로 제한한다.

예외:

- 최상위 폴더 구조 변경
- 프로젝트 개발 방향 또는 ProjectSSOT 판단 기준 변경
- Plan에서 Systems로 공식 승격
- 문서 생명주기나 작업별 라우팅 변경
- 깨진 링크 또는 실제 경로 불일치 수정

---

## 5. 새 문서 생성 전 확인

새 문서를 만들기 전에 다음 순서로 판단한다.

```text
1. 기존 ProjectSSOT 문서에 흡수할 수 있는가
2. 기존 Systems 문서의 갱신으로 해결할 수 있는가
3. 기존 Plan 폴더의 대표 문서에 흡수할 수 있는가
4. 별도 문서가 필요한 경우 역할과 생명주기가 명확한가
5. 공식 승격이 필요한 경우 갱신할 상위 색인 하나가 무엇인가
```

새 공식 문서가 필요하면 목적, 범위, 현재 상태, 등록 위치를 명시한다.

---

## 6. 문서 구성 규칙

설계 문서에서는 다음 항목을 분리해서 적는다.

- 목적
- 범위
- 제약
- 결정 사항
- 미결 사항
- 검증 또는 다음 단계
- Changelog
- Migration이 필요한 경우 Migration

현재 구현 문서는 구현된 사실과 검증 상태를 기록한다.
미래 설계나 미구현 기능을 현재 완료 기능처럼 기록하지 않는다.

### 6.1 작업 종료 상태 기록

작업이 끝났다는 이유만으로 문서를 무조건 수정하지 않는다.

다음 중 하나에 해당하면 관련 대표 Plan을 우선 갱신하고, 대표 Plan이 없는 현재 구현 작업은 관련 Systems 또는 ProjectSSOT 문서에 필요한 범위만 기록한다.

- 실제 코드나 에셋이 변경된 경우
- 기능 상태가 변경된 경우
- 빌드 또는 PIE 검증 상태가 변경된 경우
- 다음 세션에서 이어서 수행할 작업이 생긴 경우

최소 기록 항목은 다음과 같다.

- 현재 상태
- 이번에 완료한 범위
- 아직 검증하지 않은 항목
- 다음 작업
- 관련 코드와 문서 경로

빌드와 PIE 검증 상태는 분리해서 기록한다.
사용자가 직접 확인하지 않은 PIE 결과는 `PASS` 또는 `Completed`로 기록하지 않는다.
단순 설명, 조사, 질의응답만 수행한 경우에는 Plan 또는 Systems 갱신을 강제하지 않는다.

### 6.2 다중 작업 체크포인트와 세션 인계

여러 카테고리의 작업을 병행할 때는 `Document/ActiveWork.md`를 세션 복원용 중앙 색인으로 사용한다.
`ActiveWork.md`에는 활성 작업 목록, 마지막 작업 초점, 작업 간 의존 관계와 대표 체크포인트 경로만 기록하고 상세 구현 상태를 복사하지 않는다.

각 활성 작업의 대표 Plan에는 다음 체크포인트 항목을 유지한다.

- 현재 상태
- 완료한 범위
- 미완료 또는 미검증 범위
- 빌드 상태
- PIE 상태
- 바로 다음 작업
- 관련 코드와 문서 경로
- 다시 변경하지 말아야 할 확정 범위가 있는 경우 보호 범위

다음 상태 변화가 발생하면 해당 대표 Plan 체크포인트를 갱신한다.

- 실제 코드나 에셋 패치 적용
- 중요한 설계 결정 확정
- 빌드 성공 또는 실패
- 사용자 PIE 결과 수신
- 오류 원인 또는 차단 요소 확정
- 다음 작업 단계 변경
- 작업 카테고리 또는 마지막 작업 초점 변경

사용자가 `새 세션 인계 준비해줘`라고 요청하면 이번 세션에서 상태가 바뀐 모든 활성 작업의 대표 Plan을 먼저 갱신하고, 마지막에 `ActiveWork.md`를 갱신한다.
사용자가 `이전 작업 이어서 진행해줘`라고 요청하면 `ActiveWork.md`의 마지막 작업 초점을 사용하되, 특정 작업명이나 작업 ID가 있으면 해당 작업을 우선한다.
새 세션은 바로 수정하지 않고 대표 Plan, 관련 Systems, Git 상태와 실제 코드를 교차검증한 복원 결과를 먼저 보고한다.

장문의 대화 요약을 별도 SSOT로 만들지 않는다.
세션별 로그 파일을 누적 생성하지 않는다.
브라우저가 예고 없이 중단된 경우에는 마지막으로 저장된 체크포인트까지를 복원 기준으로 사용한다.

### 6.3 웹브라우저 AI 코드 작업지시서 강제 게이트

실제 코드·설정·스크립트 작업 준비의 최우선 기준은 `Document/CodeWorkGate.md`다.
이 규칙은 사용자가 세션 복원 문구를 사용했는지와 관계없이 모든 새 브라우저 세션에 적용한다.

```text
Git·AGENTS·CodeWorkGate 확인
→ 관련 Systems·대표 Plan·실제 코드 확인
→ TaskSource 작성 또는 정제
→ 최종 Codex YAML 작업지시서 생성
→ quality_gate·evidence_gate·final_output_ready 확인
→ 사용자에게 TaskSource와 최종 YAML 경로 전달
```

브라우저 AI 세션에 Codex 실행 도구가 연결되어 있을 필요는 없다.
`plan.*` 기능은 Codex 프로세스를 실행하는 기능이 아니라 Codex용 TaskSource와 최종 YAML을 생성하는 기능이다.
최종 YAML 생성과 경로 전달이 끝나면 `Ready for External Codex`로 종료할 수 있다.
실제 코드 수정은 사용자가 여는 별도 Codex 세션 또는 사용자가 선택한 Codex 환경에서 수행한다.

대표 Plan 체크포인트에는 다음 준비·후속 상태를 구분해서 남긴다.

- 준비 방식
- TaskSource 실제 경로 또는 `Missing`
- 최종 Codex 작업지시서 실제 YAML 경로 또는 `Missing`
- 작업지시서 상태: `Ready for External Codex` / `Quality Gate Failed` / `Missing`
- 외부 Codex 실행 상태: `Not Run` / `In Progress` / `PASS` / `FAIL` / `Unknown`
- 브라우저 검수 상태
- 빌드 상태
- PIE 상태

`plan.*` 기능 자체가 사용할 수 없거나 품질 게이트를 해결할 근거가 없으면 작업지시서 생성 단계가 차단되었다고 보고한다.
Codex 실행 도구가 연결되지 않았다는 이유로 차단하지 않는다.
차단 원인과 직접 수정 위험을 사용자에게 먼저 보고하고, 그 보고 이후 사용자가 웹브라우저 AI의 직접 수정을 명시적으로 승인한 경우에만 예외를 적용한다.
사용자의 일반적인 `수정해줘`, `구현해줘`, `계속해줘`는 직접 수정 승인으로 해석하지 않는다.

최종 Codex 작업지시서 없이 직접 수정한 사실이 확인되면 추가 수정을 중단하고 대표 Plan에 `Policy Violation`과 누락된 실행 출처를 사실대로 기록한다.
사후 TaskSource를 만들어 원래 변경이 Codex 작업이었던 것처럼 기록하지 않는다.

문서 전용 수정, 읽기 전용 분석·리뷰와 사용자가 UE 에디터에서 직접 수행하는 Blueprint·바이너리 에셋 작업은 코드 수정 게이트에서 제외한다.

---

## 7. CarFight 빌드 문서 기준

- 신규 또는 수정 문서의 에디터 빌드 명령은 `Tools\BuildEditor.bat`로 적는다.
- 기존 문서의 `D:\UE_5.7` 또는 `D:\UE_5.7_Source` 예시는 과거 기록으로 보고 새 지침으로 승격하지 않는다.

---

## 8. Changelog

### v1.7 - 2026-07-16

- `plan.*` 기능을 Codex 실행기가 아니라 Codex용 TaskSource·최종 YAML 생성기로 명확히 정의.
- 브라우저 세션 완료 조건을 실제 코드 수정에서 `Ready for External Codex` 작업지시서 생성과 경로 전달로 변경.
- Codex 실행 도구 미연결을 정상 상태로 정의하고 차단 사유에서 제거.
- 작업지시서 준비 상태, 외부 Codex 실행 상태와 브라우저 후속 검수 상태를 분리.
- 실제 차단 기준을 `plan.*` 사용 불가, 품질 게이트 미해결과 안전한 범위 확정 불가로 한정.

### v1.6 - 2026-07-16

- `Document/CodeWorkGate.md`를 웹브라우저 AI 코드 작업의 최우선 실행 게이트로 추가.
- 사용자 세션 복원 문구와 관계없이 모든 새 브라우저 세션에서 코드 작업 게이트를 적용하도록 변경.
- 대표 Plan에 실행 방식, TaskSource, Codex 계약, Codex 실행과 브라우저 검수 출처를 기록하도록 추가.
- Plan·Codex 사용 불가를 자동 직접 수정 예외에서 제거하고 사용자 사후 명시 승인 방식으로 강화.
- Codex 계약 없는 직접 수정 발견 시 `Policy Violation` 기록과 추가 수정 중단 절차 추가.

### v1.5 - 2026-07-14

- 웹브라우저 AI의 실제 코드 수정 기본 경로를 `plan.* → Codex 작업지시서 → Codex 실행 → 브라우저 AI 검수`로 지정.
- TaskSource와 Codex 실행 계약의 최소 필드, 보호 범위와 검증 요구사항 추가.
- Codex 완료 보고가 아니라 실제 Git diff, 빌드와 테스트 증거로 완료를 판정하도록 규칙 추가.
- 브라우저 AI의 직접 코드 수정 예외를 사용자 명시 요청 또는 Plan·Codex 사용 불가로 제한.
- 문서 전용 작업, 읽기 전용 분석과 UE 에디터의 Blueprint·바이너리 에셋 작업을 예외로 구분.

### v1.4 - 2026-07-14

- CarFight `Document/`의 관리 범위를 `main_game` 게임 프로젝트로 제한.
- AssetDump와 GoPyMCP의 독립 문서 진입점, ActiveWork와 Plan 경로 추가.
- 독립 저장소의 내부 Task, 릴리스 상태, TaskSource와 체크포인트를 CarFight 문서에 등록하지 않는 규칙 추가.
- 독립 저장소 작업 시 해당 Git 상태와 가장 가까운 `AGENTS.md`를 우선하도록 명시.

### v1.3 - 2026-07-14

- `Document/ActiveWork.md`를 다중 작업 세션 복원용 중앙 색인으로 지정.
- 활성 작업 상세 상태를 각 대표 Plan의 체크포인트에 유지하도록 역할 분리.
- 코드·에셋 패치, 설계 확정, 빌드·PIE 결과, 차단 요소와 작업 초점 변경 시 체크포인트 갱신 규칙 추가.
- `새 세션 인계 준비해줘`, `이전 작업 이어서 진행해줘`, 특정 작업 재개 명령의 처리 흐름 추가.
- 장문의 대화 요약과 세션별 로그 누적 생성을 금지.

### v1.2 - 2026-07-14

- 상태 변화가 있는 작업만 종료 기록을 남기도록 기준 추가.
- 대표 Plan 우선 기록과 Plan이 없는 경우의 Systems·ProjectSSOT 기록 기준 추가.
- 현재 상태, 완료 범위, 미검증 항목, 다음 작업, 관련 경로를 최소 기록 항목으로 지정.
- 빌드와 PIE 검증 상태를 분리하고 사용자 미확인 PIE를 PASS로 기록하지 않는 규칙 추가.
- 단순 설명, 조사, 질의응답은 문서 갱신 대상에서 제외.

### v1.1 - 2026-07-14

- `Document_Entry.md` 우선 읽기와 작업별 선택 라우팅 규칙 추가.
- Archive, Generated, ConceptArt Image, `.git` 기본 검색 제외 규칙 추가.
- 문서 생성과 공식 승격 분리, 상위 색인 갱신 최대 1개 원칙 추가.
- 새 문서 생성 전 기존 문서 흡수 가능성 확인 절차 추가.

### v1.0

- CarFight 문서 작성, doc_curation 우선, 빌드 명령, 설계 문서 구성 기준 정리.

---

## 9. Migration

### v1.7 적용 안내

- 브라우저 AI 세션의 완료 조건은 실제 코드 수정이 아니라 품질 게이트를 통과한 TaskSource와 최종 Codex YAML 생성 및 경로 전달이다.
- `plan.*` 기능은 Codex 실행기가 아니며, Codex 실행 도구 미연결은 정상 상태다.
- 실제 코드 수정은 별도 Codex 세션 또는 사용자가 선택한 Codex 환경에서 최종 YAML을 입력해 수행한다.
- 브라우저 세션은 작업지시서 생성 후 `Ready for External Codex`, 외부 실행은 `Not Run`으로 기록할 수 있다.
- 차단 상태는 `plan.*` 사용 불가, 품질 게이트 미해결 또는 안전한 범위 확정 불가일 때만 사용한다.
- 기존 문서의 `Codex 실행 계약`은 `최종 Codex 작업지시서`로 해석하며, `Codex 실행`은 별도 외부 단계로 해석한다.

### v1.6 적용 안내 — 폐기됨, v1.7이 대체

> 아래 내용은 변경 이력 보존용이며 현재 작업 판단에는 사용하지 않는다.

- 실제 코드·설정·스크립트 변경 가능성이 있는 작업은 사용자 요청 문구와 관계없이 `AGENTS.md → CodeWorkGate.md → Document_Entry.md` 순서로 시작한다.
- 코드 변경 전 대표 Plan, TaskSource와 Codex 계약이 실제 경로에 존재해야 한다.
- Plan 또는 Codex 기능을 사용할 수 없으면 직접 수정하지 않고 `Blocked — Plan/Codex Unavailable`로 보고한다.
- 직접 수정은 차단 보고 이후 사용자가 명시적으로 승인한 경우에만 허용하고 대표 Plan에 예외 출처를 기록한다.
- 기존 구현 중 계약이 없는 변경은 사후 위조하지 않고 `Policy Violation` 또는 `Provenance Missing`으로 기록한다.
- 이번 규칙 강화는 기존 코드와 검증 결과를 자동으로 되돌리지 않으며, 다음 코드 변경 전에 Codex 검토·보정 작업을 만들 수 있다.

### v1.5 적용 안내 — 폐기됨, v1.7이 대체

> 아래 내용은 변경 이력 보존용이며 현재 작업 판단에는 사용하지 않는다.

- 웹브라우저 AI가 실제 코드 변경을 시작할 때는 먼저 소유 저장소의 대표 Plan과 실제 코드를 확인한다.
- 기본 코드 실행 경로는 `plan.*` 기능으로 TaskSource와 Codex 실행 계약을 만든 뒤 Codex가 수정하는 방식이다.
- 브라우저 AI는 Codex 완료 후 실제 diff, 빌드와 테스트를 검수하고 검증된 결과만 문서 상태에 반영한다.
- 기존 활성 Plan은 이동하지 않으며 필요한 TaskSource와 Codex 계약은 해당 작업의 소유 저장소 Plan 폴더에 둔다.
- 사용자가 직접 수정을 요청하거나 Plan·Codex가 사용 불가능한 경우를 제외하면 브라우저 AI는 소스 코드를 직접 패치하지 않는다.
- 문서 전용 변경과 UE 에디터의 Blueprint·바이너리 에셋 수동 작업은 기존 방식으로 진행한다.

### v1.4 적용 안내

- CarFight 문서체계는 `main_game` 게임 프로젝트만 관리한다.
- AssetDump는 `UE/Plugins/ue-assetdump/Documents/`, GoPyMCP는 `GoPyMCP/Workspace/docs/`의 독립 문서체계를 사용한다.
- 기존 CarFight ActiveWork와 Plan에 등록된 AssetDump 내부 상태는 제거한다.
- CarFight가 독립 도구를 사용할 때는 공개 계약과 사용 위치만 기록하고 내부 Task와 체크포인트는 복사하지 않는다.
- 독립 저장소 작업은 해당 저장소의 Git 상태와 가장 가까운 `AGENTS.md`에서 시작한다.

### v1.3 적용 안내

- 기존 FeatureQueue, Plan Index와 Systems Index의 역할은 유지한다.
- 새 `Document/ActiveWork.md`는 현재 활성 작업과 대표 체크포인트를 연결하는 세션 복원 색인으로만 사용한다.
- 각 활성 작업의 상세 중단 상태는 해당 대표 Plan의 `현재 작업 체크포인트`에 기록한다.
- 세션 이동 전 사용자는 `새 세션 인계 준비해줘`만 요청하면 된다.
- 새 세션에서는 `이전 작업 이어서 진행해줘` 또는 `<작업명/ID> 이어서 진행해줘`만 요청하면 된다.
- 브라우저가 갑자기 중단되면 마지막 체크포인트 이후 대화만으로 확정된 내용은 복원되지 않을 수 있다.

### v1.2 적용 안내

- 기존 Plan과 Systems 문서의 파일명과 위치는 변경하지 않는다.
- 작업 종료 기록은 모든 대화에 강제하지 않고, 실제 변경이나 상태 전환이 있을 때만 남긴다.
- 관련 대표 Plan이 있으면 그 문서를 우선 갱신한다.
- 대표 Plan이 없는 현재 구현 작업은 관련 Systems 또는 ProjectSSOT 문서에 필요한 범위만 기록한다.
- 다음 세션은 기록된 현재 상태, 미검증 항목, 다음 작업, 관련 경로를 기준으로 작업을 복원한다.

### 기존 적용 안내

- 기존 문서의 위치와 파일명은 변경하지 않았다.
- 앞으로 AI와 Codex는 `Document/Document_Entry.md`를 먼저 읽고 작업별로 필요한 문서만 선택한다.
- Draft, Working, Notes는 공식 색인 등록 대상이 아니다.
- 개별 문서 추가만으로 여러 상위 색인을 동시에 갱신하지 않는다.
