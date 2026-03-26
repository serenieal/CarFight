# MCP Entry (CarFight)

## 목적
`Document/MCP_Entry.md`는 **CarFight 프로젝트의 MCP 오버레이 엔트리**이다.

- 공통 MCP 기준(구조/도구 설명/골든패스/에러 대응)의 원본은 `Document/SSOT/`에 있다.
- 공통 Git 기준 중 **AI가 MCP를 이용해서 Git 관리를 수행할 때의 기준**은 `Document/SSOT/Git_SSOT/`에 있다.
- 공통 UE 협업/설명 운영 기준은 `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`에 있다.
- 이 문서는 CarFight 프로젝트에서 필요한 **실사용 경로 / 사용 범주 / 예외 / 운영 주의사항**만 기록한다.
- 공통 tool 설명은 복붙하지 않고 `Document/SSOT/MCP_SSOT/`, `Document/SSOT/Git_SSOT/`, `Document/SSOT/UE_SSOT/` 문서를 참조한다.

---

## 1) Start Here

### 먼저 확인할 것
1. 이 문서에서 Project MCP Usage / Project Paths / Project Exceptions 확인
2. 현재 runtime 상태 확인
   - `hmd_ping`
   - `hmd_repo_info`
   - UE 작업이면 `hmd_ue_env`
3. 공통 설명은 아래 문서로 이동
   - 구조/정책: `Document/SSOT/MCP_SSOT/MCP_SSOT_Master.md`
   - tool 설명: `Document/SSOT/MCP_SSOT/MCP_Cmd_Guide.md`
   - 작업 흐름: `Document/SSOT/MCP_SSOT/MCP_Golden_Path.md`
   - 문제 대응: `Document/SSOT/MCP_SSOT/MCP_Error_Guide.md`
4. **Git 상태 조회 / repo_id 판별 / write 가능 범위 판단이 필요하면 아래 문서로 이동**
   - Git 기준: `Document/SSOT/Git_SSOT/Git_SSOT_Master.md`
5. **UE 작업을 사용자에게 설명하거나, BP/C++ 전환 작업을 함께 진행할 때는 아래 문서를 함께 확인**
   - UE 운영 기준: `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`

---

## 2) SSOT Reference
- SSOT Hub: `Document/SSOT`
- MCP SSOT Path: `Document/SSOT/MCP_SSOT`
- Git SSOT Path: `Document/SSOT/Git_SSOT`
- UE Workflow SSOT Path: `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`
- Reference Tag: `TODO`
- Reference Commit: `TODO`
- Last Checked: `2026-03-12`
- Verified Source Server Ver: `v1.1.6`
- Verified Live Runtime Ver (`hmd_repo_info`): `v1.1.5`
- Verified Controller: `MCP/py/mcpctl.py` (`fixed-ctl-v1`)
- Verified Runtime Profiles: `work` / `lab`

---

## 3) Project MCP Usage
- [x] Repo 텍스트 읽기 / 탐색 사용 (`repo_*` 우선)
- [x] Asset list / asset dump 사용
- [ ] Map dump 사용
- [x] BPGraph dump 사용 (`hmd_dump_bpgraph_safe` 우선)
- [x] dump_targets 생성 사용
- [x] 비동기 Job 사용
- [x] Chunk read 사용
- [x] Git read-only 조회 사용
- [x] Git 저장소 / repo_id / branch 판별 사용
- [x] Write 기능 사용 (Lab 프로필 한정)
- [x] Git write preflight / execute 사용 (Lab 프로필 한정, 현재 `repo_id='mcp'` 범위)

---

## 4) Project Paths
- Repo Root Rule: `D:\Work\CarFight_git`
- MCP Root Rule: `MCP/`는 Repo Root 바로 아래 고정
- Controller Path: `MCP/py/mcpctl.py`
- Server Entry Path:
  - Work: `MCP/py/mcp_reposrv_work.py`
  - Lab: `MCP/py/mcp_reposrv_lab.py`
  - Base: `MCP/py/mcp_reposrv.py`
- Batch Launcher Path:
  - Work: `MCP/StartWork.bat`, `MCP/StatusWork.bat`, `MCP/StopWork.bat`, `MCP/RestartWork.bat`
  - Lab: `MCP/StartLab.bat`, `MCP/StatusLab.bat`, `MCP/StopLab.bat`, `MCP/RestartLab.bat`
- Site / URL / PID Files:
  - Work: `MCP/mcp_site_work.txt`, `MCP/mcp_url_work.txt`, `MCP/mcp_pids_work.txt`
  - Lab: `MCP/mcp_site_lab.txt`, `MCP/mcp_url_lab.txt`, `MCP/mcp_pids_lab.txt`
  - Shared: `MCP/mcp_tunnel_pid.txt`
- UE Project Path Rule: `UE/CarFight_Re.uproject` (`HMD_UE_UPROJ` 일치)
- Main Asset Root: `/Game/CarFight`
- Project MCP Work Root: `Document/ProjectSSOT/MCP/`
- Generated / Work Examples:
  - `Document/ProjectSSOT/MCP/Gen/`
  - `Document/ProjectSSOT/MCP/Work/`
  - `Document/ProjectSSOT/MCP/Config/`

---

## 5) Project Exceptions

### EX-01
- 예외 항목: `MCP_Entry`는 공통 설명서가 아니라 프로젝트 운영 카드로 유지
- 이유: 공통 tool 설명과 절차는 SSOT에서 관리하고, 프로젝트 문서는 얇게 유지해야 드리프트를 막을 수 있음
- 적용 범위: `Document/MCP_Entry.md`
- 리스크: 공통 설명을 다시 쓰기 시작하면 SSOT와 프로젝트 문서가 쉽게 어긋남
- 대체/보완: tool 설명/절차/에러 대응은 SSOT 링크만 두고 상세는 SSOT에서만 관리

### EX-02
- 예외 항목: 런처/사이트/PID가 단일 파일이 아니라 `work` / `lab` 프로필 분리 구조
- 이유: Work를 read-only 기준선으로 유지하고, Lab을 write 허용 운영 경로로 분리하기 위해서
- 적용 범위: `MCP/*.bat`, `MCP/mcp_site_*.txt`, `MCP/mcp_url_*.txt`, `MCP/mcp_pids_*.txt`, `MCP/mcp_tunnel_pid.txt`
- 리스크: 예전 단일 파일명 기준 절차를 쓰면 잘못된 파일을 찾게 됨
- 대체/보완: Work/Lab 구분을 항상 먼저 확인하고, 파일명도 프로필 기준으로 본다

### EX-03
- 예외 항목: 프로젝트 MCP 실사용 경로를 `Document/MCP/`가 아니라 `Document/ProjectSSOT/MCP/` 기준으로 우선 운영
- 이유: 현재 프로젝트의 MCP 산출물과 작업 메모가 `ProjectSSOT` 흐름과 함께 관리되고 있기 때문
- 적용 범위: `Document/ProjectSSOT/MCP/`, `Document/ProjectSSOT/MCP/Gen/`, `Document/ProjectSSOT/MCP/Work/`, `Document/ProjectSSOT/MCP/Config/`
- 리스크: 코드 기본값(`Document/MCP/...`)만 보고 작업하면 실제 프로젝트 결과물 경로와 어긋날 수 있음
- 대체/보완: 코드 기본값은 참고만 하고, 실제 작업은 이 문서의 경로를 우선 기준으로 삼음

### EX-04
- 예외 항목: write 계열은 Work/Lab 공통 기능이 아니라 Lab + write 허용 상태에서만 노출
- 이유: Work를 read-only 기준선으로 유지하기 위해서
- 적용 범위: `hmd_write_text`, `hmd_append_text`, `hmd_patch_text`, `git_add_*`, `git_unstage_*`, `git_commit_*`, `git_preflight_diag`
- 리스크: Work에서도 보일 것이라고 가정하면 절차 자체가 틀어짐
- 대체/보완: write 작업 전 `hmd_repo_info`로 `profile`, `write_allowed`, `write_tools_exposed` 확인

### EX-05
- 예외 항목: Git write/preflight는 현재 일반 Git 기능이 아니라 `repo_id='mcp'` 범위 제한 구현으로 운영
- 이유: 현재 단계의 안전 범위를 명확히 제한하기 위해서
- 적용 범위: `git_add_prepare/execute`, `git_unstage_prepare/execute`, `git_commit_prepare/execute`
- 리스크: 모든 repo_id에서 동작한다고 가정하면 실행 절차와 문서가 모두 틀어짐
- 대체/보완: Git write 전에는 항상 현재 repo_id와 제한 범위를 먼저 확인

### EX-06
- 예외 항목: `Document/SSOT/...`는 메인 프로젝트 문서처럼 보여도 MCP Git 기준에서는 별도 저장소 `ssot`로 취급
- 이유: 공용 SSOT 허브가 별도 Git 저장소로 관리되기 때문
- 적용 범위: `Document/SSOT/README.md`, `Document/SSOT/Git_SSOT/...`, `Document/SSOT/MCP_SSOT/...`, `Document/SSOT/UE_SSOT/...`
- 리스크: 메인 프로젝트 문서 변경과 SSOT 변경을 같은 저장소 변경으로 오해할 수 있음
- 대체/보완: Git 관련 판단은 항상 `Git_SSOT` 기준으로 repo_id와 저장소 경계를 먼저 확인

### EX-07
- 예외 항목: UE 작업 설명/협업 방식은 MCP 절차 문서가 아니라 UE 운영 SSOT를 함께 참조
- 이유: 한국어판 UE 기준 표기, Details 패널 한국어(영어) 병기, BP/C++ 전환 설명 방식은 MCP 문서가 아니라 UE 협업 운영 규칙에 속하기 때문
- 적용 범위: UE 작업 안내, BP 생성/세팅 안내, 차량/Chaos/Blueprint/C++ 전환 설명 전반
- 리스크: UE 작업을 설명하면서 영어 UI명만 사용하거나, 기존 BP를 기준 구현으로 잘못 가정할 수 있음
- 대체/보완: UE 작업을 시작할 때는 `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`를 함께 확인

---

## 6) Project Risks / Notes
- Work 기본 포트는 `8000`, Lab 기본 포트는 `8001`
- Work에서는 `hmd_preflight_begin`이 직접 노출되지 않으므로, BPGraph 기본 경로는 `hmd_dump_bpgraph_safe`
- 텍스트/코드 읽기 기본 경로는 `hmd_read_text`보다 `repo_*` 계열을 우선 사용
- Git 상태 조회 / repo_id 판별 / 브랜치 해석 / write 가능 범위 판단은 `Document/SSOT/Git_SSOT/Git_SSOT_Master.md`를 우선 기준으로 사용
- UE 작업을 사용자에게 설명하거나 함께 진행할 때는 `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`를 함께 확인한다
- 코드 기본값이 `Document/MCP/...`를 가리켜도 실제 프로젝트 작업 경로는 이 문서의 `Document/ProjectSSOT/MCP/...`를 우선 기준으로 사용
- Source 버전과 Live Runtime 버전은 다를 수 있으므로 둘을 구분해서 기록/확인

---

## 7) Quick Links
- MCP Master: `Document/SSOT/MCP_SSOT/MCP_SSOT_Master.md`
- MCP Cmd Guide: `Document/SSOT/MCP_SSOT/MCP_Cmd_Guide.md`
- MCP Golden Path: `Document/SSOT/MCP_SSOT/MCP_Golden_Path.md`
- MCP Error Guide: `Document/SSOT/MCP_SSOT/MCP_Error_Guide.md`
- Git Master: `Document/SSOT/Git_SSOT/Git_SSOT_Master.md`
- UE Workflow SSOT: `Document/SSOT/UE_SSOT/UE_AI_User_Workflow_SSOT_v0_1.md`
- Project MCP Work Root: `Document/ProjectSSOT/MCP/`
- ProjectSSOT Op Map: `Document/ProjectSSOT/ProjectSSOT_OpMap.md`

---

## 8) Template Guard
- 이 문서는 **프로젝트 전용 정보만** 기록한다.
- 공통 tool 설명/절차/에러 대응은 **SSOT 문서에서만 관리**한다.
- 공통 Git 절차/판단 기준은 **Git SSOT 문서에서만 관리**한다.
- 공통 UE 협업/설명 운영 기준은 **UE Workflow SSOT 문서에서만 관리**한다.
- 현재 프로젝트에서 실제로 중요한 경로/예외/운영 주의사항만 남긴다.
