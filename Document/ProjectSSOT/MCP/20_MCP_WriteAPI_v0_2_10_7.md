# MCP Write API Patch Validation (CarFight Project)

- Date: 2026-03-04 (Asia/Seoul)
- Scope: CarFight 프로젝트 MCP 서버 패치 적용 후 스모크 테스트 결과
- Type: **ProjectSSOT / MCP 프로젝트 사항**
- Common Spec Reference: `Document/SSOT/MCP_SSOT/20_MCP_WriteAPI_Spec_v0_2_10_7.md`
- Status: **Smoke Test Passed**
- Re-test After Server Restart: **Passed** (live endpoint verification, 2026-03-04)

## 테스트 환경 메모
- 서버 버전: `v0.2.10.7`
- 테스트 대상 서버: CarFight MCP (`hmd_mcp_reposrv.py`)
- 테스트 방식: `api_tool.call_tool` 직접 호출

## 스모크 테스트 결과 (2026-03-04)

### 서버 재시작 후 라이브 재검증 (2026-03-04)
- 서버 인스턴스 재시작 후 `hmd_diag_context`로 새 `server_instance_id` 확인
- Append / Atomic / Chunk API를 라이브 엔드포인트로 재실행
- 결과: **전부 성공**
- 검증 파일:
  - `Document/ProjectSSOT/MCP/mcp_write_api_test_after_restart.txt`
  - `Document/ProjectSSOT/MCP/mcp_chunk_test_after_restart.txt`


### Append 테스트
- 대상 파일: `Document/ProjectSSOT/MCP/mcp_write_api_test.txt`
- 결과: **성공**
- 검증:
  - 2회 append 후 `hmd_read_text`로 내용 확인

### Atomic 테스트
- 대상 파일: `Document/ProjectSSOT/MCP/mcp_write_api_test.txt`
- 결과: **성공**
- 검증:
  - `expected_sha256` 일치 상태에서 overwrite 성공
  - `sha256_before` / `sha256_after` 반환 확인
  - 최종 내용: `ATOMIC_WRITE_OK`

### Chunk 테스트
- 대상 파일: `Document/ProjectSSOT/MCP/mcp_chunk_test.txt`
- 결과: **성공**
- 검증:
  - `begin -> append(2회) -> commit`
  - `expected_upload_sha256` 검증 성공
  - `hmd_read_text`로 최종 내용 확인 (`CHUNK_LINE_1~3`)

## 프로젝트 운영 메모 (CarFight)
- 긴 수정 작업은 `link_...` 경로 대신 정식 경로(`/CarFightMCP/...`) 기준으로 재시도 권장
- base64 payload가 전송 중 필터에 걸리면 chunk를 더 잘게 나누어 append
- 대형 코드/문서 변경은 `hmd_patch_text` 또는 Chunk API 우선 고려

## mcpctl.py 보조 점검 커맨드 추가
- `MCP/py/mcpctl.py`에 `selftest-writeapi` 서브커맨드 추가
- 목적: 서버 스크립트에 append/atomic/chunk 함수 블록이 존재하는지 빠르게 점검(feature probe)
- 예시:
  - `python MCP/py/mcpctl.py selftest-writeapi --launcher-root MCP --repo-root .`
