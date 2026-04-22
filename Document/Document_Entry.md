# Document Entry (CarFight)

- 문서 버전: v2.0
- 작성일: 2026-04-21
- 문서 상태: Current

## 목적

`Document/` 폴더의 **총괄 진입점(Entry)** 이다.

이 문서는 아래 3가지를 빠르게 구분해준다.

- **공용 SSOT**: 여러 프로젝트에 공통 적용되는 기준 (`Document/SSOT/`)
- **프로젝트 SSOT**: CarFight 프로젝트 전용 기준 (`Document/ProjectSSOT/`)
- **UE 사용자 협업 운영 기준**: AI가 UE 작업을 사용자와 함께 진행할 때 따를 공용 운영 SSOT (`Document/SSOT/UE_SSOT/`)

도구 자체 문서와 실행 절차는 `Document/` 안에서 관리하지 않는다.  
그 내용은 별도 도구 레포 문서를 기준으로 본다.

---

## 1) 역할 분리

### A. `Document/SSOT/`

- 여러 프로젝트에서 공통으로 사용하는 SSOT 허브
- 공통 규칙, 공통 UE 협업 원칙, 공통 문서 작성 원칙을 관리
- 특정 프로젝트 전용 내용으로 수정하지 않음

### B. `Document/ProjectSSOT/`

- CarFight 프로젝트 전용 기준 문서 모음
- 현재 상태, 확정 결정, 로드맵, 검증 기준, 프로젝트 예외를 관리
- 공용 규칙은 복사하지 않고 링크로만 참조

### C. `Document/SSOT/UE_SSOT/`

- UE 사용자 협업 운영 기준
- 한국어판 UE 기준 설명 방식
- 사용자와 AI가 병렬로 작업할 때의 운영 원칙

---

## 2) 우선순위 규칙

### 규칙 1. 공통 기준이 필요하면

→ 먼저 `Document/SSOT/` 를 본다.

### 규칙 2. CarFight 프로젝트 상태 / 결정 / 검증이 필요하면

→ `Document/ProjectSSOT/` 를 본다.

### 규칙 3. UE 작업을 사용자와 함께 진행할 때

→ `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md` 를 먼저 본다.

### 규칙 4. 도구 사용 절차나 도구 계약이 필요할 때

→ `Document/` 안에서 찾지 말고, 별도 도구 레포 문서를 확인한다.

---

## 3) 빠른 시작

### 3.1 CarFight 현재 상태를 빠르게 파악하고 싶을 때

1. `Document/ProjectSSOT/README.md`
2. `Document/ProjectSSOT/00_Handover.md`
3. `Document/ProjectSSOT/01_Roadmap.md`
4. `Document/ProjectSSOT/08_P0_Verification.md`

### 3.2 CarFight 문서 정리 / Archive 작업을 하고 싶을 때

1. `Document/ProjectSSOT/README.md`
2. `Document/ProjectSSOT/Archive/README.md`

### 3.3 UE 작업을 사용자와 함께 진행하고 싶을 때

1. `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`
2. 필요한 UE 기술 SSOT
3. 프로젝트 전용 계획서 (`Document/ProjectSSOT/...`)

### 3.4 공통 규칙 원본을 찾고 싶을 때

1. `Document/SSOT/README.md`
2. 필요한 카테고리 (`UE_SSOT`, `Shared` 등)

---

## 4) 문서 작성 원칙

### 공용 내용은 어디에?

- 여러 프로젝트에 동시에 적용되는 기준과 절차는 `Document/SSOT/`

### 프로젝트 내용은 어디에?

- CarFight 전용 상태, 결정, 검증, 예외는 `Document/ProjectSSOT/`

### 도구 문서는 어디에?

- `Document/` 안에서 관리하지 않는다
- 별도 도구 레포 문서를 기준으로 본다

---

## 5) 현재 Document 구조

```text
Document/
  Document_Entry.md          # 이 문서 (총괄 진입점)
  ProjectSSOT/               # CarFight 프로젝트 전용 SSOT
  SSOT/                      # 공용 통합 SSOT
```

---

## 6) 유지보수 체크

- 공용 규칙을 `ProjectSSOT`에 복붙하지 않는다
- CarFight 전용 내용을 `SSOT`에 넣지 않는다
- 도구 자체 설명과 실행 절차는 `Document/` 안에 다시 쓰지 않는다
- UE 협업 설명은 `UE_SSOT`를 먼저 확인한다

---

## 7) Quick Links

- `Document/ProjectSSOT/README.md`
- `Document/ProjectSSOT/00_Handover.md`
- `Document/ProjectSSOT/01_Roadmap.md`
- `Document/ProjectSSOT/08_P0_Verification.md`
- `Document/SSOT/README.md`
- `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`

---

## 8) Changelog

- v2.0
- `Document/` 허브에서 MCP 전용 엔트리 구조 제거
- Document 허브 역할을 `SSOT / ProjectSSOT / UE_SSOT` 중심으로 재정의
- 도구 문서는 별도 도구 레포에서 관리한다는 원칙 명시
