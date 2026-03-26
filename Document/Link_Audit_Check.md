# Link Audit Checklist (Document)

## 목적
`Document/` 문서에서 **경로 링크/문서명/구 버전 표기(레거시 문자열)** 를 점검할 때 사용하는 체크리스트.

특히 아래 항목을 우선 점검한다.
- `UE_BP_SSOT` 같은 과거 파일명/경로 표기
- `Document/00_*` 형태의 옛 구조 경로 표기
- 공용 SSOT/프로젝트 SSOT 역할 충돌 표기

---

## 1) 점검 대상 범위
### A. 우선 점검 (핵심 엔트리/기준 문서)
- [ ] `Document/Document_Entry.md`
- [ ] `Document/MCP_Entry.md`
- [ ] `Document/ProjectSSOT/README.md`
- [ ] `Document/ProjectSSOT/ProjectSSOT_OpMap.md`
- [ ] `Document/ProjectSSOT/00_Handover.md`
- [ ] `Document/ProjectSSOT/01_Roadmap.md`
- [ ] `Document/ProjectSSOT/08_P0_Verification.md`

### B. 템플릿 점검
- [ ] `Document/SSOT/Document_Entry_Template.md`
- [ ] `Document/SSOT/MCP_Entry_Template.md`

### C. 운영/Archive 점검 (선택)
- [ ] `Document/ProjectSSOT/Archive/README.md`
- [ ] `Document/ProjectSSOT/Archive_Move_Check.md`
- [ ] `Document/ProjectSSOT/Archive_Batch01_Run.md`

---

## 2) 레거시 문자열 점검 항목
### A. UE BP Text SSOT 구 표기
아래 문자열이 남아있는지 확인:
- [ ] `UE_BP_SSOT`

권장 최신 표기(현재 기준):
- `UE_BP_Text_Format_SSOT`
- 또는 파일 경로 전체:
  - `Document/SSOT/UE_SSOT/BP_Text_Format/UE_BP_Text_Format_SSOT.md`

### B. 옛 Document 구조 경로
아래 형태가 남아있는지 확인:
- [ ] `Document/00_Handover.md`
- [ ] `Document/01_Roadmap.md`
- [ ] `Document/02_Conventions.md`

현재 CarFight 기준 권장 경로:
- `Document/ProjectSSOT/00_Handover.md`
- `Document/ProjectSSOT/01_Roadmap.md`
- `Document/ProjectSSOT/02_Conventions.md`

---

## 3) 역할 분리 점검 항목
- [ ] `Document/SSOT`를 CarFight 전용 수정 대상으로 오해하게 만드는 표현이 없다.
- [ ] `Document/ProjectSSOT`에 공용 규칙 원본이 있다고 오해하게 만드는 표현이 없다.
- [ ] `Document/MCP_Entry.md`가 공통 MCP 기준 문서처럼 설명되지 않는다.
- [ ] 공통 절차/에러 대응은 `Document/SSOT/MCP_SSOT/` 참조로 안내된다.

---

## 4) 링크 무결성 점검 항목
- [ ] Quick Links 경로가 현재 파일명과 일치한다.
- [ ] 템플릿 경로 안내가 실제 파일과 일치한다.
- [ ] `UE_BP_Text_Format_SSOT` 관련 링크가 실제 파일명과 일치한다.
- [ ] `UE_CPP_SSOT` 폴더/문서 경로가 실제 구조와 일치한다.

---

## 5) 점검 결과 기록 (복붙용)
- 점검일: `YYYY-MM-DD`
- 점검 범위: `핵심 / 템플릿 / 전체(수동)`
- `UE_BP_SSOT` 발견 건수: `N건`
- 수정 필요 건수: `N건`
- 비고:

---

## 6) 이번 세션 권장 점검 순서
1. `Document/MCP_Entry.md`
2. `Document/SSOT/MCP_Entry_Template.md`
3. `Document/Document_Entry.md`
4. `Document/SSOT/Document_Entry_Template.md`
5. `Document/ProjectSSOT/ProjectSSOT_OpMap.md`
