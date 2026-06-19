# Link Audit Checklist (Document)

- 문서 버전: v2.1
- 최근 갱신일: 2026-06-19

## 목적

`Document/` 문서에서 경로 링크, 문서명, 구 버전 표기를 점검할 때 사용하는 체크리스트다.

---

## 1) 점검 대상 범위

### A. 우선 점검

- [ ] `Document/Document_Entry.md`
- [ ] `Document/ProjectSSOT/README.md`
- [ ] `Document/ProjectSSOT/00_Vision.md`
- [ ] `Document/ProjectSSOT/01_ProjectState.md`
- [ ] `Document/ProjectSSOT/02_Roadmap.md`
- [ ] `Document/ProjectSSOT/03_FeatureQueue.md`
- [ ] `Document/ProjectSSOT/04_ProjectDecisions.md`
- [ ] `Document/ProjectSSOT/05_TestChecklist.md`

### B. 템플릿 점검

- [ ] `Document/SSOT/Document_Entry_Template.md`

### C. 운영 / Archive 점검

- [ ] `Document/ProjectSSOT/Archive/README.md`

---

## 2) 역할 분리 점검 항목

- [ ] `Document/SSOT` 를 CarFight 전용 수정 대상으로 오해하게 만드는 표현이 없다.
- [ ] `Document/ProjectSSOT` 에 공용 규칙 원본이 있다고 오해하게 만드는 표현이 없다.
- [ ] 도구 자체 설명과 실행 절차가 `Document/` 안에 남아 있지 않다.

---

## 3) 링크 무결성 점검 항목

- [ ] Quick Links 경로가 현재 파일명과 일치한다.
- [ ] 템플릿 경로 안내가 실제 파일과 일치한다.
- [ ] UE 관련 링크가 실제 파일명과 일치한다.
- [ ] `Document/Plan`의 서버/멀티 관련 문서가 현재 활성 계획처럼 보이지 않는다.
- [ ] `Document/Systems/Network` 문서가 싱글 전환 기준에서 보류 상태임을 드러낸다.

---

## 4) 점검 결과 기록

- 점검일: `YYYY-MM-DD`
- 점검 범위: `핵심 / 템플릿 / 전체`
- 수정 필요 건수: `N건`
- 비고:

---

## 5) Changelog

- v2.1
- ProjectSSOT 점검 대상을 현재 파일명 기준으로 갱신
- 서버/멀티 계획과 Network Systems 문서의 활성 상태 오해 방지 체크 추가

- v2.0
- Document 쪽 MCP 관련 점검 항목 제거
- Document 허브 / ProjectSSOT / UE_SSOT 중심 체크리스트로 단순화
