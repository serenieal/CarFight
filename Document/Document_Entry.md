# Document Entry (CarFight)

## 목적
`Document/` 폴더의 **총괄 진입점(Entry)** 이다.

이 문서는 아래 5가지를 빠르게 구분해준다.
- **공용 SSOT**: 여러 UE 프로젝트에 공통 적용되는 기준 (`Document/SSOT/`)
- **프로젝트 SSOT**: CarFight 프로젝트 전용 기준 (`Document/ProjectSSOT/`)
- **프로젝트 MCP 오버레이**: CarFight의 MCP 사용 여부/경로/예외 (`Document/MCP_Entry.md`)
- **MCP 기반 Git 관리 기준**: AI가 MCP로 Git을 다룰 때 따를 공용 기준 (`Document/SSOT/Git_SSOT/`)
- **UE 사용자 협업 운영 기준**: AI가 UE 작업을 사용자에게 설명/지원할 때 따를 공용 운영 SSOT (`Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`)

> 템플릿이 필요하면 `Document/SSOT/Document_Entry_Template.md`를 사용한다.

---

## 1) 역할 분리 (핵심)

### A. `Document/SSOT/` (공용 통합 SSOT)
- 여러 UE 개발 프로젝트에서 공통으로 사용하는 **공용 SSOT**
- 작업 기준(규칙/절차/가이드)의 원본
- CarFight 전용 내용으로 수정하지 않음

**예시 성격**
- 공통 MCP 절차/에러 가이드/골든패스
- 공통 MCP Git 관리 기준
- 공통 UE 문서 규칙/포맷/참조 기준
- 공통 UE 협업/설명 운영 규칙
- 여러 프로젝트에서 동시에 적용 가능한 규칙

### B. `Document/ProjectSSOT/` (CarFight 프로젝트 전용 SSOT)
- **CarFight 프로젝트에만 해당되는 내용**을 담는 SSOT
- 현재 상태 / 확정 결정 / 로드맵 / 검증 기준 / 프로젝트 예외
- 공용 SSOT 내용을 복붙하지 않고 링크로 참조

**예시 성격**
- CarFight 휠 인덱스/컴포넌트 구조/우측 휠 180 규칙
- CarFight P0/P1 로드맵과 DoD
- CarFight P0 검증 체크리스트
- CarFight 전용 문서 운영 규칙/Archive 정책

### C. `Document/MCP_Entry.md` (프로젝트 MCP 오버레이 엔트리)
- CarFight 프로젝트에서 MCP를 사용할 때의 **프로젝트별 오버레이 문서**
- 공통 MCP 기준은 `Document/SSOT/`를 따르고,
  여기에는 **프로젝트 사용 여부/경로/예외/리스크**만 기록

### D. `Document/SSOT/Git_SSOT/` (MCP 기반 Git 관리 SSOT)
- **AI가 MCP를 이용해서 Git 작업을 수행할 때** 따라야 하는 공용 기준
- 일반 Git 상식이 아니라, 현재 MCP에 실제로 노출된 Git 도구 기준으로 판단
- repo_id 판별, read-only 조회 순서, write 가능 범위, 저장소 경계 해석을 정의

### E. `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md` (UE 사용자 협업 운영 SSOT)
- **AI가 UE 작업을 사용자에게 설명하거나 함께 진행할 때** 따라야 하는 공용 운영 기준
- 한국어판 UE 기준 표기 원칙, Details 패널 표기 형식, BP/C++ 전환 설명 방식, 사람-AI 병렬 작업 방식을 정의
- UE 기술 설계 SSOT를 대체하지 않고, 설명/운영/협업 방식의 기준으로 사용

---

## 2) 우선순위 규칙 (무엇을 먼저 볼지)

### 규칙 1. 공통 기준이 필요하면
→ 먼저 `Document/SSOT/`를 본다.

### 규칙 2. CarFight 프로젝트 상태/결정/검증이 필요하면
→ `Document/ProjectSSOT/`를 본다.

### 규칙 3. MCP 작업을 실제로 시작할 때
→ `Document/MCP_Entry.md`를 먼저 보고, 상세 절차는 `Document/SSOT/MCP_SSOT/`를 본다.

### 규칙 4. MCP로 Git 상태 조회 / diff 해석 / write 가능 범위를 판단할 때
→ `Document/SSOT/Git_SSOT/Git_SSOT_Master.md`를 본다.

### 규칙 5. UE 작업을 사용자에게 설명하거나, BP/C++ 전환 작업을 함께 진행할 때
→ `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`를 먼저 확인한다.

---

## 3) 빠른 시작 (상황별 진입 경로)

### 3.1 CarFight 현재 상태를 빠르게 파악하고 싶을 때
1. `Document/ProjectSSOT/ProjectSSOT_OpMap.md`
2. `Document/ProjectSSOT/00_Handover.md`
3. `Document/ProjectSSOT/01_Roadmap.md`
4. `Document/ProjectSSOT/08_P0_Verification.md`

### 3.2 CarFight 문서 정리 / Archive 작업을 하고 싶을 때
1. `Document/ProjectSSOT/README.md`
2. `Document/ProjectSSOT/Archive/README.md`
3. `Document/ProjectSSOT/Archive_Move_Check.md`
4. `Document/ProjectSSOT/Archive_Batch01_Run.md`

### 3.3 MCP 작업을 시작하고 싶을 때
1. `Document/MCP_Entry.md` (프로젝트 오버레이)
2. `Document/SSOT/README.md` (공용 허브)
3. `Document/SSOT/MCP_SSOT/MCP_SSOT_Master.md`
4. `Document/SSOT/MCP_SSOT/MCP_Golden_Path.md`

### 3.4 MCP로 Git 작업을 판단하고 싶을 때
1. `Document/MCP_Entry.md`
2. `Document/SSOT/Git_SSOT/Git_SSOT_Master.md`
3. 필요 시 `Document/SSOT/MCP_SSOT/MCP_SSOT_Master.md`

### 3.5 UE 작업을 사용자와 함께 진행하고 싶을 때
1. `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`
2. `Document/MCP_Entry.md`
3. 필요한 UE 기술 SSOT (`Document/SSOT/UE_SSOT/...`)
4. 프로젝트 전용 계획서 (`Document/ProjectSSOT/...`)

### 3.6 공통 규칙/절차 원본을 찾고 싶을 때
1. `Document/SSOT/README.md`
2. 필요한 카테고리(MCP_SSOT / Git_SSOT / UE_SSOT / Shared)

---

## 4) 문서 작성/수정 원칙 (요약)

### 공용 내용은 어디에?
- 여러 프로젝트에 동시에 적용되는 기준/절차/가이드는 `Document/SSOT/`

### 프로젝트 내용은 어디에?
- CarFight에만 해당되는 상태/결정/검증/예외는 `Document/ProjectSSOT/`

### MCP 프로젝트 설정은 어디에?
- CarFight 프로젝트의 MCP 사용 여부/경로/예외는 `Document/MCP_Entry.md`

### MCP 기반 Git 공통 기준은 어디에?
- AI가 MCP로 Git을 다룰 때의 공통 기준은 `Document/SSOT/Git_SSOT/`

### UE 설명/협업 운영 기준은 어디에?
- AI가 UE 작업을 사용자에게 설명하거나 함께 진행할 때의 공용 운영 기준은 `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`

---

## 5) 현재 Document 구조 (요약)
```text
Document/
  Document_Entry.md          # 이 문서 (총괄 진입점)
  MCP_Entry.md               # CarFight MCP 오버레이 엔트리
  MCP/                       # MCP 작업 산출물/설정/워크 폴더
  ProjectSSOT/               # CarFight 프로젝트 전용 SSOT
  SSOT/                      # 공용 통합 SSOT (다중 프로젝트 공용)
```

---

## 6) 유지보수 체크 (권장)
문서 체계가 꼬이지 않게 아래만 지킨다.
- 공용 규칙을 `ProjectSSOT`에 복붙하지 않는다.
- CarFight 전용 내용을 `SSOT`에 넣지 않는다.
- MCP 작업 시작 전에는 `MCP_Entry.md`를 먼저 확인한다.
- MCP Git 작업 판단은 `Git_SSOT`를 먼저 확인한다.
- UE 작업을 사용자에게 설명하거나 함께 진행할 때는 `UE_AI_User_Workflow_SSOT_v0_1.md`를 먼저 확인한다.
- 프로젝트 문서 정리/Archive 작업은 `ProjectSSOT` 운영 문서를 따른다.

---

## 7) Quick Links
- `Document/MCP_Entry.md`
- `Document/ProjectSSOT/ProjectSSOT_OpMap.md`
- `Document/ProjectSSOT/00_Handover.md`
- `Document/ProjectSSOT/01_Roadmap.md`
- `Document/ProjectSSOT/08_P0_Verification.md`
- `Document/ProjectSSOT/README.md`
- `Document/SSOT/README.md`
- `Document/SSOT/Git_SSOT/Git_SSOT_Master.md`
- `Document/SSOT/MCP_SSOT/MCP_SSOT_Master.md`
- `Document/SSOT/MCP_SSOT/MCP_Golden_Path.md`
- `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`
