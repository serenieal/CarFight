# -*- coding: utf-8 -*-
"""
StartWorkVs.py
v2.0.0

목적(Visual Studio 기준 작업 세션 기록):
  1) (Start) Git/C++ 상태 스냅샷 저장
  2) (Start) UE SSOTDump 저장
  3) Visual Studio(솔루션) 실행 후 종료까지 대기
  4) (End)   Git/C++ 상태 스냅샷 저장
  5) (End)   UE SSOTDump 저장
  6) (Report) Git/SSOT 통합 세션 리포트 생성 (v2.0)

정책(사용자 합의 반영):
  - Visual Studio가 이미 실행 중이면 즉시 중단(세션 경계 보장)
  - Start SSOTDump 실패 시 즉시 중단(엄격 모드)  -> 단, "덤프 파일이 생성되었으면" 성공으로 간주
  - End SSOTDump 실패 시에도 기록은 남기고 종료 코드로 실패 표시

버전 히스토리:
  - v1.1.0: PowerShell 실행 정책 문제 회피를 위해 Python 기반으로 전환
  - v1.1.1: subprocess 출력 디코딩을 UTF-8(+errors=replace)로 강제하여 UnicodeDecodeError 방지
  - v1.1.2: SSOTDump가 exit code=1을 반환해도 실제 덤프 파일이 생성되면 성공으로 처리
            + SSOTDump stdout/stderr를 세션 로그 파일로 저장(디코딩 이슈 없음)
  - v1.2.0: Start/End Git 스냅샷 비교 결과를 git_delta.txt로 자동 생성
            + summary.txt에 SSOT raw/file count/로그 경로를 함께 기록
  - v2.0.0: ssot_delta.txt 자동 생성(파일셋 + JSON semantic 비교)
            + session_report.txt / session_report.json 통합 리포트 생성
            + summary.txt에 통합 리포트/델타 파일 경로 추가
"""

import datetime
import glob
import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple


# ------------------------------
# [VAR] 스크립트 버전
# ------------------------------
SCRIPT_VERSION: str = "2.0.0"

# ------------------------------
# [VAR] UnrealEditor-Cmd.exe 경로 (SSOTDump용)
# ------------------------------
UNREAL_CMD_PATH: str = r"D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

# ------------------------------
# [VAR] uproject 경로
# ------------------------------
UPROJECT_PATH: str = r"D:\Work\CarFight_git\UE\CarFight_Re.uproject"

# ------------------------------
# [VAR] SSOT 덤프 타겟 (assets / input / wheel / all)
# ------------------------------
DUMP_TARGET: str = "all"

# ------------------------------
# [VAR] Autodetect 플래그 (입력 루트 등 하드코딩 회피)
# ------------------------------
AUTO_DETECT_FLAG: str = "-autodetect"

# ------------------------------
# [VAR] 세션 로그 루트 (비워두면 Uproject 기준으로 자동 결정)
#      기본: <UprojectDir>\Saved\WorkLog
# ------------------------------
WORKLOG_ROOT_DIR: str = ""

# ------------------------------
# [VAR] Visual Studio 솔루션 경로
#      - 비워두면 RepoRoot에서 *.sln을 검색해서 1개일 때 자동 선택
#      - 여러 개면 에러로 중단(오탐 방지)
# ------------------------------
SOLUTION_PATH: str = ""

# ------------------------------
# [VAR] VS가 실행 중이면 중단할지 여부 (합의: True)
# ------------------------------
FAIL_IF_VS_RUNNING: bool = True

# ------------------------------
# [VAR] subprocess 텍스트 디코딩 인코딩(고정)
# ------------------------------
SUBPROC_ENCODING: str = "utf-8"

# ------------------------------
# [VAR] SSOTDump 성공 판정 규칙
#      - exit code가 0이 아니어도, 결과 파일(prefix+ssot_*.json)이 1개 이상 생성되면 성공으로 간주
# ------------------------------
SSOT_REQUIRE_OUTPUT_FILES: bool = True

# ------------------------------
# [VAR] SSOT JSON semantic 비교 사용 여부
#      - True: JSON 파싱 후 정규화 비교
#      - False: 파일 바이트 비교만 수행
# ------------------------------
ENABLE_SSOT_JSON_COMPARE: bool = True


# ===========================================================
# [FUNC] 콘솔 출력 + run_log.txt append
# ===========================================================
def write_log(run_log_path: Path, message: str) -> None:
    """
    [FUNC] run_log.txt에 로그를 남기면서 콘솔에도 출력
    """
    print(message)
    run_log_path.parent.mkdir(parents=True, exist_ok=True)
    with run_log_path.open("a", encoding="utf-8", newline="\n") as log_file:
        log_file.write(message + "\n")


# ===========================================================
# [FUNC] 경로 존재 검증
# ===========================================================
def assert_path_exists(path: str, name: str) -> None:
    """
    [FUNC] 특정 파일/폴더가 존재하는지 확인
    """
    if not Path(path).exists():
        raise FileNotFoundError(f"[ERROR] {name} not found: {path}")


# ===========================================================
# [FUNC] subprocess 실행 유틸 (stdout/stderr 캡처)
# ===========================================================
def run_command(args: List[str], cwd: Optional[str] = None) -> Tuple[int, str, str]:
    """
    [FUNC] 외부 명령 실행 (stdout/stderr 캡처)
    """
    completed = subprocess.run(
        args,
        cwd=cwd,
        text=True,
        capture_output=True,
        shell=False,
        encoding=SUBPROC_ENCODING,
        errors="replace",
    )
    return completed.returncode, completed.stdout, completed.stderr


# ===========================================================
# [FUNC] 줄 목록 파일 읽기(공백줄 제거)
# ===========================================================
def read_lines_file(file_path: Path) -> List[str]:
    """
    [FUNC] 줄 기반 텍스트 파일을 읽어 공백줄 제거 후 반환
    """
    if not file_path.exists():
        return []
    text = file_path.read_text(encoding="utf-8", errors="replace")
    lines = [line.strip() for line in text.splitlines()]
    return [line for line in lines if line]


# ===========================================================
# [FUNC] Git repo root 감지
# ===========================================================
def get_repo_root(base_dir: str) -> str:
    """
    [FUNC] git 레포 루트를 자동 감지
    """
    git_path = shutil.which("git")
    if not git_path:
        raise RuntimeError("[ERROR] git not found. Git 설치/환경변수를 확인하세요.")

    code, out, err = run_command(["git", "-C", base_dir, "rev-parse", "--show-toplevel"])
    if code != 0 or not out.strip():
        raise RuntimeError(f"[ERROR] RepoRoot auto-detect failed. (git rev-parse)\n{err}")
    return out.strip()


# ===========================================================
# [FUNC] RepoRoot 아래 *.sln 1개 자동 선택
# ===========================================================
def find_single_solution(repo_root_dir: str) -> str:
    """
    [FUNC] repo_root_dir 하위에서 .sln 파일을 찾아 1개면 반환
    """
    solution_pattern = str(Path(repo_root_dir) / "**" / "*.sln")
    solution_list = glob.glob(solution_pattern, recursive=True)

    if len(solution_list) == 1:
        return solution_list[0]
    if len(solution_list) == 0:
        raise RuntimeError("[ERROR] RepoRoot 하위에서 .sln을 찾지 못했습니다.")
    raise RuntimeError("[ERROR] RepoRoot 하위에 .sln이 여러 개입니다. SOLUTION_PATH를 직접 지정하세요.")


# ===========================================================
# [FUNC] devenv.exe 경로 탐색
# ===========================================================
def find_devenv_path() -> str:
    """
    [FUNC] Visual Studio devenv.exe 경로를 찾는다
    """
    devenv_path = shutil.which("devenv.exe")
    if devenv_path and Path(devenv_path).exists():
        return devenv_path

    program_files_x86 = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere_path = Path(program_files_x86) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"

    if vswhere_path.exists():
        code, out, err = run_command([
            str(vswhere_path),
            "-latest", "-products", "*",
            "-requires", "Microsoft.Component.MSBuild",
            "-property", "installationPath",
        ])

        install_path = out.strip()
        if code == 0 and install_path:
            candidate_path = Path(install_path) / "Common7" / "IDE" / "devenv.exe"
            if candidate_path.exists():
                return str(candidate_path)

        if err.strip():
            raise RuntimeError(f"[ERROR] vswhere failed:\n{err}")

    raise RuntimeError("[ERROR] devenv.exe not found. Visual Studio 설치를 확인하세요.")


# ===========================================================
# [FUNC] VS 실행 중인지 확인
# ===========================================================
def is_vs_running() -> bool:
    """
    [FUNC] devenv.exe 프로세스가 이미 실행 중인지 확인
    """
    code, out, _ = run_command(["tasklist", "/FI", "IMAGENAME eq devenv.exe"])
    if code != 0:
        return False
    return "devenv.exe" in out.lower()


# ===========================================================
# [FUNC] UE 잔류 여부 확인
# ===========================================================
def is_ue_running() -> bool:
    """
    [FUNC] UnrealEditor.exe 프로세스가 남아있는지 확인(경고용)
    """
    code, out, _ = run_command(["tasklist", "/FI", "IMAGENAME eq UnrealEditor.exe"])
    if code != 0:
        return False
    return "unrealeditor.exe" in out.lower()


# ===========================================================
# [FUNC] SSOT 출력 폴더 준비 + cfg 생성
# ===========================================================
def prep_out_dir(out_dir: Path, prefix: str) -> None:
    """
    [FUNC] 출력 폴더 생성 + 이전 결과 삭제 + ssot_cfg.lst 생성
    """
    out_dir.mkdir(parents=True, exist_ok=True)

    delete_pattern_list = [
        str(out_dir / f"{prefix}ssot_*.json"),
        str(out_dir / "ssot_*.json"),
        str(out_dir / "ssot_cfg.lst"),
    ]

    for delete_pattern in delete_pattern_list:
        for file_path_str in glob.glob(delete_pattern):
            try:
                Path(file_path_str).unlink(missing_ok=True)
            except Exception:
                pass

    cfg_path = out_dir / "ssot_cfg.lst"
    cfg_text = "\n".join([
        "# ssot_cfg.lst (auto-generated)",
        "DefaultEngine.ini|/Script/EngineSettings.GameMapsSettings|GameDefaultMap",
        "DefaultEngine.ini|/Script/EngineSettings.GameMapsSettings|EditorStartupMap",
        "",
    ])
    cfg_path.write_text(cfg_text, encoding="utf-8")


# ===========================================================
# [FUNC] ssot_*.json -> prefix+filename rename
# ===========================================================
def rename_ssot_json(out_dir: Path, prefix: str) -> None:
    """
    [FUNC] ssot_*.json을 prefix를 붙여 리네임
    """
    for source_path in out_dir.glob("ssot_*.json"):
        target_path = out_dir / f"{prefix}{source_path.name}"
        try:
            source_path.rename(target_path)
        except Exception:
            pass


# ===========================================================
# [FUNC] SSOTDump 실행 + 성공 판정
# ===========================================================
def run_ssot_dump(
    unreal_cmd_path: str,
    uproject_path: str,
    out_dir: Path,
    prefix: str,
    phase_name: str,
    run_log_path: Path,
) -> Tuple[int, int, int, Path]:
    """
    [FUNC] SSOTDump 실행 + 결과 rename + 성공 판정

    반환값:
      - raw_exit_code: UE 커맨드렛이 반환한 원본 코드
      - normalized_exit_code: 성공 판정 규칙 적용 후 코드(0=성공)
      - output_file_count: 생성된 prefix+ssot_*.json 개수
      - ssot_log_path: 커맨드렛 stdout/stderr 로그 파일 경로
    """
    prep_out_dir(out_dir, prefix)
    write_log(run_log_path, f"[SSOT] {phase_name} dump...")

    # [VAR] SSOTDump 실행 인자
    ssot_args = [
        unreal_cmd_path,
        uproject_path,
        "-run=SSOTDump",
        f"-out={str(out_dir)}",
        f"-target={DUMP_TARGET}",
        AUTO_DETECT_FLAG,
        "-unattended",
        "-nop4",
        "-NullRHI",
        "-notime",
    ]

    # [VAR] 커맨드렛 로그 파일
    ssot_log_path = out_dir.parent / f"ssot_{phase_name.lower()}_cmdlet.log"

    # stdout/stderr를 파일로 직접 기록 (디코딩 이슈 없음)
    with ssot_log_path.open("wb") as cmdlet_log_file:
        raw_exit_code = subprocess.call(ssot_args, stdout=cmdlet_log_file, stderr=cmdlet_log_file)

    rename_ssot_json(out_dir, prefix)

    # [VAR] 생성 결과 파일 개수 체크
    output_file_count = len(list(out_dir.glob(f"{prefix}ssot_*.json")))

    # [VAR] 성공 판정 코드
    normalized_exit_code = raw_exit_code
    if SSOT_REQUIRE_OUTPUT_FILES:
        if output_file_count > 0:
            normalized_exit_code = 0
        else:
            normalized_exit_code = raw_exit_code if raw_exit_code != 0 else 1

    return raw_exit_code, normalized_exit_code, output_file_count, ssot_log_path


# ===========================================================
# [FUNC] meta 기록
# ===========================================================
def write_meta(session_dir: Path, phase: str, repo_root_dir: str, devenv_path: str) -> None:
    """
    [FUNC] 메타 정보를 파일로 기록
    """
    meta_path = session_dir / f"meta_{phase}.txt"

    meta_line_list = [
        f"ScriptVersion={SCRIPT_VERSION}",
        f"Phase={phase}",
        f"RepoRootDir={repo_root_dir}",
        f"UprojectPath={UPROJECT_PATH}",
        f"SolutionPath={SOLUTION_PATH}",
        f"DevenvPath={devenv_path}",
        f"DumpTarget={DUMP_TARGET}",
        f"AutoDetectFlag={AUTO_DETECT_FLAG}",
        f"TimestampLocal={datetime.datetime.now().isoformat(timespec='seconds')}",
        "",
    ]
    meta_path.write_text("\n".join(meta_line_list), encoding="utf-8")


# ===========================================================
# [FUNC] Git 스냅샷 기록
# ===========================================================
def write_git_snapshot(session_dir: Path, repo_root_dir: str, phase: str) -> None:
    """
    [FUNC] git 상태를 파일로 저장
    """
    git_head_path = session_dir / f"git_{phase}_head.txt"
    git_status_path = session_dir / f"git_{phase}_status.txt"
    git_porcelain_path = session_dir / f"git_{phase}_porcelain.txt"
    git_names_path = session_dir / f"git_{phase}_names.txt"
    git_names_staged_path = session_dir / f"git_{phase}_names_s.txt"
    git_untracked_path = session_dir / f"git_{phase}_untracked.txt"

    _, branch_out, _ = run_command(["git", "-C", repo_root_dir, "branch", "--show-current"])
    _, head_out, _ = run_command(["git", "-C", repo_root_dir, "rev-parse", "HEAD"])

    git_head_path.write_text(
        f"branch:\n{branch_out}\nhead:\n{head_out}\n",
        encoding="utf-8",
    )

    _, status_out, _ = run_command(["git", "-C", repo_root_dir, "status", "-sb"])
    git_status_path.write_text(status_out, encoding="utf-8")

    _, porcelain_out, _ = run_command(["git", "-C", repo_root_dir, "status", "--porcelain=v1"])
    git_porcelain_path.write_text(porcelain_out, encoding="utf-8")

    _, names_out, _ = run_command(["git", "-C", repo_root_dir, "diff", "--name-only"])
    git_names_path.write_text(names_out, encoding="utf-8")

    _, names_staged_out, _ = run_command(["git", "-C", repo_root_dir, "diff", "--cached", "--name-only"])
    git_names_staged_path.write_text(names_staged_out, encoding="utf-8")

    _, untracked_out, _ = run_command(["git", "-C", repo_root_dir, "ls-files", "--others", "--exclude-standard"])
    git_untracked_path.write_text(untracked_out, encoding="utf-8")


# ===========================================================
# [FUNC] Git head 파일 파싱 (branch/head)
# ===========================================================
def parse_git_head_file(git_head_path: Path) -> Dict[str, str]:
    """
    [FUNC] git_*_head.txt에서 branch/head를 추출
    """
    result = {"branch": "", "head": ""}
    if not git_head_path.exists():
        return result

    lines = git_head_path.read_text(encoding="utf-8", errors="replace").splitlines()

    current_key = ""
    value_buffer: List[str] = []

    def flush_value() -> None:
        nonlocal current_key, value_buffer
        if current_key in result and value_buffer:
            result[current_key] = "\n".join([v for v in value_buffer if v.strip()]).strip()
        value_buffer = []

    for line in lines:
        lower_line = line.strip().lower()
        if lower_line == "branch:":
            flush_value()
            current_key = "branch"
            continue
        if lower_line == "head:":
            flush_value()
            current_key = "head"
            continue
        if current_key:
            value_buffer.append(line)

    flush_value()
    return result


# ===========================================================
# [FUNC] 리스트 파일 비교 (added/removed 유지)
# ===========================================================
def compare_line_set_files(start_file_path: Path, end_file_path: Path) -> Dict[str, Any]:
    """
    [FUNC] 줄 목록 파일 두 개를 비교하여 added/removed 집합 반환
    """
    start_set = set(read_lines_file(start_file_path))
    end_set = set(read_lines_file(end_file_path))

    added_list = sorted(end_set - start_set)
    removed_list = sorted(start_set - end_set)

    return {
        "start_count": len(start_set),
        "end_count": len(end_set),
        "added": added_list,
        "removed": removed_list,
        "added_count": len(added_list),
        "removed_count": len(removed_list),
    }


# ===========================================================
# [FUNC] Git delta 생성 + 파일 저장
# ===========================================================
def write_git_delta(session_dir: Path) -> Tuple[Path, Dict[str, Any]]:
    """
    [FUNC] Start/End Git 스냅샷 비교 결과를 git_delta.txt/json으로 저장
    """
    git_delta_txt_path = session_dir / "git_delta.txt"
    git_delta_json_path = session_dir / "git_delta.json"

    # [VAR] 비교 대상 파일 쌍
    compare_target_map = {
        "unstaged": ("git_start_names.txt", "git_end_names.txt"),
        "staged": ("git_start_names_s.txt", "git_end_names_s.txt"),
        "untracked": ("git_start_untracked.txt", "git_end_untracked.txt"),
    }

    compare_result_map: Dict[str, Any] = {}
    for category_name, (start_name, end_name) in compare_target_map.items():
        compare_result_map[category_name] = compare_line_set_files(
            session_dir / start_name,
            session_dir / end_name,
        )

    # [VAR] head 정보
    start_head_info = parse_git_head_file(session_dir / "git_start_head.txt")
    end_head_info = parse_git_head_file(session_dir / "git_end_head.txt")

    git_delta_data = {
        "script_version": SCRIPT_VERSION,
        "start_branch": start_head_info.get("branch", ""),
        "end_branch": end_head_info.get("branch", ""),
        "start_head": start_head_info.get("head", ""),
        "end_head": end_head_info.get("head", ""),
        "branch_changed": start_head_info.get("branch", "") != end_head_info.get("branch", ""),
        "head_changed": start_head_info.get("head", "") != end_head_info.get("head", ""),
        "categories": compare_result_map,
    }

    # txt 리포트 작성
    report_line_list: List[str] = []
    report_line_list.append(f"ScriptVersion={SCRIPT_VERSION}")
    report_line_list.append(f"StartBranch={git_delta_data['start_branch']}")
    report_line_list.append(f"EndBranch={git_delta_data['end_branch']}")
    report_line_list.append(f"BranchChanged={git_delta_data['branch_changed']}")
    report_line_list.append(f"StartHead={git_delta_data['start_head']}")
    report_line_list.append(f"EndHead={git_delta_data['end_head']}")
    report_line_list.append(f"HeadChanged={git_delta_data['head_changed']}")
    report_line_list.append("")

    for category_name in ["unstaged", "staged", "untracked"]:
        category_data = compare_result_map[category_name]
        report_line_list.append(f"[{category_name}]")
        report_line_list.append(f"StartCount={category_data['start_count']}")
        report_line_list.append(f"EndCount={category_data['end_count']}")
        report_line_list.append(f"AddedCount={category_data['added_count']}")
        report_line_list.append(f"RemovedCount={category_data['removed_count']}")

        report_line_list.append("Added:")
        if category_data["added"]:
            report_line_list.extend([f"  + {item}" for item in category_data["added"]])
        else:
            report_line_list.append("  (none)")

        report_line_list.append("Removed:")
        if category_data["removed"]:
            report_line_list.extend([f"  - {item}" for item in category_data["removed"]])
        else:
            report_line_list.append("  (none)")

        report_line_list.append("")

    git_delta_txt_path.write_text("\n".join(report_line_list), encoding="utf-8")
    git_delta_json_path.write_text(
        json.dumps(git_delta_data, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    return git_delta_txt_path, git_delta_data


# ===========================================================
# [FUNC] JSON 정규화 (semantic 비교용)
# ===========================================================
def normalize_json_value(json_value: Any) -> Any:
    """
    [FUNC] JSON 값을 재귀 정규화하여 순서 영향 최소화
    """
    if isinstance(json_value, dict):
        return {key: normalize_json_value(json_value[key]) for key in sorted(json_value.keys())}
    if isinstance(json_value, list):
        return [normalize_json_value(item) for item in json_value]
    return json_value


# ===========================================================
# [FUNC] JSON/바이트 비교 결과 반환
# ===========================================================
def compare_json_or_bytes(start_file_path: Path, end_file_path: Path) -> Dict[str, Any]:
    """
    [FUNC] 파일 2개를 비교해서 semantic 동일 여부/파싱 상태를 반환
    """
    start_bytes = start_file_path.read_bytes()
    end_bytes = end_file_path.read_bytes()

    if not ENABLE_SSOT_JSON_COMPARE:
        same_bytes = start_bytes == end_bytes
        return {
            "same": same_bytes,
            "compare_mode": "bytes",
            "parse_error": "",
            "start_sha256": hashlib.sha256(start_bytes).hexdigest(),
            "end_sha256": hashlib.sha256(end_bytes).hexdigest(),
        }

    try:
        start_json = json.loads(start_bytes.decode("utf-8"))
        end_json = json.loads(end_bytes.decode("utf-8"))

        normalized_start = normalize_json_value(start_json)
        normalized_end = normalize_json_value(end_json)

        same_semantic = normalized_start == normalized_end
        return {
            "same": same_semantic,
            "compare_mode": "json",
            "parse_error": "",
            "start_sha256": hashlib.sha256(start_bytes).hexdigest(),
            "end_sha256": hashlib.sha256(end_bytes).hexdigest(),
        }
    except Exception as parse_exception:
        same_bytes = start_bytes == end_bytes
        return {
            "same": same_bytes,
            "compare_mode": "bytes_fallback",
            "parse_error": str(parse_exception),
            "start_sha256": hashlib.sha256(start_bytes).hexdigest(),
            "end_sha256": hashlib.sha256(end_bytes).hexdigest(),
        }


# ===========================================================
# [FUNC] SSOT 파일 키 정규화 (start_/end_ 접두사 제거)
# ===========================================================
def normalize_ssot_file_key(file_name: str) -> str:
    """
    [FUNC] start_/end_ 접두사를 제거해 Start/End 파일 매칭 키 생성
    """
    if file_name.startswith("start_"):
        return file_name[len("start_"):]
    if file_name.startswith("end_"):
        return file_name[len("end_"):]
    return file_name


# ===========================================================
# [FUNC] SSOT delta 생성 + 파일 저장
# ===========================================================
def write_ssot_delta(ssot_start_dir: Path, ssot_end_dir: Path, session_dir: Path) -> Tuple[Path, Dict[str, Any]]:
    """
    [FUNC] Start/End SSOT 결과를 비교해 ssot_delta.txt/json을 저장
    """
    ssot_delta_txt_path = session_dir / "ssot_delta.txt"
    ssot_delta_json_path = session_dir / "ssot_delta.json"

    # [VAR] Start/End 파일 맵 (정규화 키 -> 실제 경로)
    start_file_map: Dict[str, Path] = {}
    end_file_map: Dict[str, Path] = {}

    for start_file_path in sorted(ssot_start_dir.glob("*.json")):
        start_file_map[normalize_ssot_file_key(start_file_path.name)] = start_file_path

    for end_file_path in sorted(ssot_end_dir.glob("*.json")):
        end_file_map[normalize_ssot_file_key(end_file_path.name)] = end_file_path

    start_key_set = set(start_file_map.keys())
    end_key_set = set(end_file_map.keys())

    added_key_list = sorted(end_key_set - start_key_set)
    removed_key_list = sorted(start_key_set - end_key_set)
    common_key_list = sorted(start_key_set & end_key_set)

    changed_key_list: List[str] = []
    unchanged_key_list: List[str] = []
    parse_fallback_key_list: List[str] = []
    compare_detail_map: Dict[str, Any] = {}

    for common_key in common_key_list:
        compare_result = compare_json_or_bytes(start_file_map[common_key], end_file_map[common_key])
        compare_detail_map[common_key] = compare_result

        if compare_result["compare_mode"] == "bytes_fallback":
            parse_fallback_key_list.append(common_key)

        if compare_result["same"]:
            unchanged_key_list.append(common_key)
        else:
            changed_key_list.append(common_key)

    ssot_delta_data = {
        "script_version": SCRIPT_VERSION,
        "start_dir": str(ssot_start_dir),
        "end_dir": str(ssot_end_dir),
        "start_file_count": len(start_key_set),
        "end_file_count": len(end_key_set),
        "added_files": added_key_list,
        "removed_files": removed_key_list,
        "common_files": common_key_list,
        "changed_files": changed_key_list,
        "unchanged_files": unchanged_key_list,
        "parse_fallback_files": parse_fallback_key_list,
        "compare_mode_json_enabled": ENABLE_SSOT_JSON_COMPARE,
        "compare_detail": compare_detail_map,
    }

    # txt 리포트 작성
    report_line_list: List[str] = []
    report_line_list.append(f"ScriptVersion={SCRIPT_VERSION}")
    report_line_list.append(f"StartDir={ssot_start_dir}")
    report_line_list.append(f"EndDir={ssot_end_dir}")
    report_line_list.append(f"StartFileCount={len(start_key_set)}")
    report_line_list.append(f"EndFileCount={len(end_key_set)}")
    report_line_list.append(f"AddedCount={len(added_key_list)}")
    report_line_list.append(f"RemovedCount={len(removed_key_list)}")
    report_line_list.append(f"CommonCount={len(common_key_list)}")
    report_line_list.append(f"ChangedCount={len(changed_key_list)}")
    report_line_list.append(f"UnchangedCount={len(unchanged_key_list)}")
    report_line_list.append(f"ParseFallbackCount={len(parse_fallback_key_list)}")
    report_line_list.append("")

    report_line_list.append("[ChangedFiles]")
    if changed_key_list:
        report_line_list.extend([f"  * {item}" for item in changed_key_list])
    else:
        report_line_list.append("  (none)")
    report_line_list.append("")

    report_line_list.append("[AddedFiles]")
    if added_key_list:
        report_line_list.extend([f"  + {item}" for item in added_key_list])
    else:
        report_line_list.append("  (none)")
    report_line_list.append("")

    report_line_list.append("[RemovedFiles]")
    if removed_key_list:
        report_line_list.extend([f"  - {item}" for item in removed_key_list])
    else:
        report_line_list.append("  (none)")
    report_line_list.append("")

    report_line_list.append("[ParseFallbackFiles]")
    if parse_fallback_key_list:
        report_line_list.extend([f"  ! {item}" for item in parse_fallback_key_list])
    else:
        report_line_list.append("  (none)")
    report_line_list.append("")

    ssot_delta_txt_path.write_text("\n".join(report_line_list), encoding="utf-8")
    ssot_delta_json_path.write_text(
        json.dumps(ssot_delta_data, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    return ssot_delta_txt_path, ssot_delta_data


# ===========================================================
# [FUNC] summary 작성 (v2.0 확장)
# ===========================================================
def write_summary(
    session_dir: Path,
    start_ssot_norm: int,
    end_ssot_norm: int,
    vs_exit_code: int,
    *,
    start_ssot_raw: int = -1,
    end_ssot_raw: int = -1,
    start_ssot_file_count: int = -1,
    end_ssot_file_count: int = -1,
    start_ssot_log_path: str = "",
    end_ssot_log_path: str = "",
    git_delta_data: Optional[Dict[str, Any]] = None,
    ssot_delta_data: Optional[Dict[str, Any]] = None,
    session_report_txt_path: str = "",
    session_report_json_path: str = "",
    git_delta_txt_path: str = "",
    ssot_delta_txt_path: str = "",
    start_time_iso: str = "",
    end_time_iso: str = "",
    duration_seconds: int = -1,
) -> Path:
    """
    [FUNC] summary.txt 생성 (핵심 상태 + 리포트 경로 요약)
    """
    summary_path = session_dir / "summary.txt"

    line_list: List[str] = []
    line_list.append(f"ScriptVersion={SCRIPT_VERSION}")
    line_list.append(f"StartTime={start_time_iso}")
    line_list.append(f"EndTime={end_time_iso}")
    line_list.append(f"DurationSeconds={duration_seconds}")
    line_list.append(f"StartSsotCode={start_ssot_norm}")
    line_list.append(f"EndSsotCode={end_ssot_norm}")
    line_list.append(f"VsExitCode={vs_exit_code}")
    line_list.append(f"StartSsotRawExit={start_ssot_raw}")
    line_list.append(f"EndSsotRawExit={end_ssot_raw}")
    line_list.append(f"StartSsotFileCount={start_ssot_file_count}")
    line_list.append(f"EndSsotFileCount={end_ssot_file_count}")
    line_list.append(f"StartSsotLog={start_ssot_log_path}")
    line_list.append(f"EndSsotLog={end_ssot_log_path}")
    line_list.append(f"GitDeltaTxt={git_delta_txt_path}")
    line_list.append(f"SsotDeltaTxt={ssot_delta_txt_path}")
    line_list.append(f"SessionReportTxt={session_report_txt_path}")
    line_list.append(f"SessionReportJson={session_report_json_path}")
    line_list.append(f"SessionDir={session_dir}")

    if git_delta_data:
        line_list.append(f"GitBranchChanged={git_delta_data.get('branch_changed', False)}")
        line_list.append(f"GitHeadChanged={git_delta_data.get('head_changed', False)}")
        for category_name in ["unstaged", "staged", "untracked"]:
            category_data = git_delta_data.get("categories", {}).get(category_name, {})
            line_list.append(f"Git_{category_name}_Added={category_data.get('added_count', -1)}")
            line_list.append(f"Git_{category_name}_Removed={category_data.get('removed_count', -1)}")

    if ssot_delta_data:
        line_list.append(f"SsotStartFileCount={ssot_delta_data.get('start_file_count', -1)}")
        line_list.append(f"SsotEndFileCount={ssot_delta_data.get('end_file_count', -1)}")
        line_list.append(f"SsotAdded={len(ssot_delta_data.get('added_files', []))}")
        line_list.append(f"SsotRemoved={len(ssot_delta_data.get('removed_files', []))}")
        line_list.append(f"SsotChanged={len(ssot_delta_data.get('changed_files', []))}")
        line_list.append(f"SsotUnchanged={len(ssot_delta_data.get('unchanged_files', []))}")
        line_list.append(f"SsotParseFallback={len(ssot_delta_data.get('parse_fallback_files', []))}")

    line_list.append("")
    summary_path.write_text("\n".join(line_list), encoding="utf-8")
    return summary_path


# ===========================================================
# [FUNC] 통합 세션 리포트 작성 (txt/json)
# ===========================================================
def write_session_report(
    session_dir: Path,
    repo_root_dir: str,
    solution_path: str,
    devenv_path: str,
    start_time: datetime.datetime,
    end_time: datetime.datetime,
    vs_exit_code: int,
    start_ssot_info: Dict[str, Any],
    end_ssot_info: Dict[str, Any],
    git_delta_data: Dict[str, Any],
    ssot_delta_data: Dict[str, Any],
) -> Tuple[Path, Path]:
    """
    [FUNC] 사람이 읽는 session_report.txt + 기계용 session_report.json 생성
    """
    session_report_txt_path = session_dir / "session_report.txt"
    session_report_json_path = session_dir / "session_report.json"

    duration_seconds = int((end_time - start_time).total_seconds())

    report_json_data = {
        "script_version": SCRIPT_VERSION,
        "session_dir": str(session_dir),
        "repo_root_dir": repo_root_dir,
        "uproject_path": UPROJECT_PATH,
        "solution_path": solution_path,
        "devenv_path": devenv_path,
        "dump_target": DUMP_TARGET,
        "auto_detect_flag": AUTO_DETECT_FLAG,
        "start_time": start_time.isoformat(timespec="seconds"),
        "end_time": end_time.isoformat(timespec="seconds"),
        "duration_seconds": duration_seconds,
        "exit_codes": {
            "vs_exit_code": vs_exit_code,
            "start_ssot_raw_exit": start_ssot_info.get("raw_exit", -1),
            "start_ssot_norm_exit": start_ssot_info.get("norm_exit", -1),
            "end_ssot_raw_exit": end_ssot_info.get("raw_exit", -1),
            "end_ssot_norm_exit": end_ssot_info.get("norm_exit", -1),
        },
        "ssot": {
            "start_file_count": start_ssot_info.get("file_count", -1),
            "end_file_count": end_ssot_info.get("file_count", -1),
            "start_cmdlet_log": start_ssot_info.get("cmdlet_log", ""),
            "end_cmdlet_log": end_ssot_info.get("cmdlet_log", ""),
            "delta": ssot_delta_data,
        },
        "git": {
            "delta": git_delta_data,
        },
        "artifacts": {
            "run_log_txt": str(session_dir / "run_log.txt"),
            "summary_txt": str(session_dir / "summary.txt"),
            "git_delta_txt": str(session_dir / "git_delta.txt"),
            "git_delta_json": str(session_dir / "git_delta.json"),
            "ssot_delta_txt": str(session_dir / "ssot_delta.txt"),
            "ssot_delta_json": str(session_dir / "ssot_delta.json"),
        },
    }

    # 사람이 읽는 텍스트 리포트
    text_line_list: List[str] = []
    text_line_list.append("=== StartWorkVs Session Report ===")
    text_line_list.append(f"ScriptVersion: {SCRIPT_VERSION}")
    text_line_list.append(f"SessionDir: {session_dir}")
    text_line_list.append("")
    text_line_list.append("[Session]")
    text_line_list.append(f"StartTime: {report_json_data['start_time']}")
    text_line_list.append(f"EndTime:   {report_json_data['end_time']}")
    text_line_list.append(f"DurationSec: {duration_seconds}")
    text_line_list.append(f"RepoRoot: {repo_root_dir}")
    text_line_list.append(f"Solution: {solution_path}")
    text_line_list.append(f"Uproject: {UPROJECT_PATH}")
    text_line_list.append("")

    text_line_list.append("[ExitCodes]")
    text_line_list.append(f"VS Exit: {vs_exit_code}")
    text_line_list.append(f"Start SSOT Raw/Norm: {start_ssot_info.get('raw_exit', -1)} / {start_ssot_info.get('norm_exit', -1)}")
    text_line_list.append(f"End   SSOT Raw/Norm: {end_ssot_info.get('raw_exit', -1)} / {end_ssot_info.get('norm_exit', -1)}")
    text_line_list.append("")

    text_line_list.append("[Git Delta]")
    text_line_list.append(f"BranchChanged: {git_delta_data.get('branch_changed', False)}")
    text_line_list.append(f"HeadChanged: {git_delta_data.get('head_changed', False)}")
    text_line_list.append(f"StartBranch: {git_delta_data.get('start_branch', '')}")
    text_line_list.append(f"EndBranch:   {git_delta_data.get('end_branch', '')}")
    for category_name in ["unstaged", "staged", "untracked"]:
        category_data = git_delta_data.get("categories", {}).get(category_name, {})
        text_line_list.append(
            f"{category_name}: +{category_data.get('added_count', 0)} / -{category_data.get('removed_count', 0)} "
            f"(start={category_data.get('start_count', 0)}, end={category_data.get('end_count', 0)})"
        )
    text_line_list.append("")

    text_line_list.append("[SSOT Delta]")
    text_line_list.append(f"StartFiles: {ssot_delta_data.get('start_file_count', 0)}")
    text_line_list.append(f"EndFiles:   {ssot_delta_data.get('end_file_count', 0)}")
    text_line_list.append(f"Changed:    {len(ssot_delta_data.get('changed_files', []))}")
    text_line_list.append(f"Unchanged:  {len(ssot_delta_data.get('unchanged_files', []))}")
    text_line_list.append(f"Added:      {len(ssot_delta_data.get('added_files', []))}")
    text_line_list.append(f"Removed:    {len(ssot_delta_data.get('removed_files', []))}")
    text_line_list.append(f"Fallback:   {len(ssot_delta_data.get('parse_fallback_files', []))}")
    text_line_list.append("")

    changed_file_list = ssot_delta_data.get("changed_files", [])
    text_line_list.append("[Changed SSOT Files]")
    if changed_file_list:
        text_line_list.extend([f"  * {item}" for item in changed_file_list[:50]])
        if len(changed_file_list) > 50:
            text_line_list.append(f"  ... ({len(changed_file_list) - 50} more)")
    else:
        text_line_list.append("  (none)")
    text_line_list.append("")

    text_line_list.append("[Artifacts]")
    for artifact_name, artifact_path in report_json_data["artifacts"].items():
        text_line_list.append(f"{artifact_name}: {artifact_path}")
    text_line_list.append(f"start_cmdlet_log: {start_ssot_info.get('cmdlet_log', '')}")
    text_line_list.append(f"end_cmdlet_log: {end_ssot_info.get('cmdlet_log', '')}")
    text_line_list.append("")

    session_report_txt_path.write_text("\n".join(text_line_list), encoding="utf-8")
    session_report_json_path.write_text(
        json.dumps(report_json_data, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    return session_report_txt_path, session_report_json_path


# ===========================================================
# [FUNC] main
# ===========================================================
def main() -> int:
    """
    [FUNC] 엔트리 포인트
    """
    try:
        # [VAR] 세션 시작 시각
        session_start_time = datetime.datetime.now()

        assert_path_exists(UNREAL_CMD_PATH, "UnrealCmdPath")
        assert_path_exists(UPROJECT_PATH, "UprojectPath")

        # [VAR] UprojectDir
        uproject_dir = str(Path(UPROJECT_PATH).parent) + "\\"

        # [VAR] RepoRoot
        repo_root_dir = get_repo_root(uproject_dir)

        # [VAR] WorkLogRoot
        worklog_root_dir = WORKLOG_ROOT_DIR.strip() or str(Path(uproject_dir) / "Saved" / "WorkLog")

        # [VAR] devenv.exe 경로
        devenv_path = find_devenv_path()

        # [VAR] VS 중복 실행 방지
        if FAIL_IF_VS_RUNNING and is_vs_running():
            print("[ERROR] Visual Studio(devenv.exe)가 이미 실행 중입니다.")
            print("        이 스크립트는 VS 프로세스 시작/종료를 세션 경계로 사용합니다.")
            print("        모든 VS를 종료한 뒤 다시 실행하세요.")
            return 1

        # [VAR] SolutionPath 결정
        global SOLUTION_PATH
        if not SOLUTION_PATH.strip():
            SOLUTION_PATH = find_single_solution(repo_root_dir)

        if not Path(SOLUTION_PATH).exists():
            raise FileNotFoundError(f"[ERROR] SolutionPath not found: {SOLUTION_PATH}")

        # [VAR] 세션 폴더
        session_id = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        session_dir = Path(worklog_root_dir) / session_id
        ssot_start_dir = session_dir / "ssot_start"
        ssot_end_dir = session_dir / "ssot_end"
        run_log_path = session_dir / "run_log.txt"

        session_dir.mkdir(parents=True, exist_ok=True)
        ssot_start_dir.mkdir(parents=True, exist_ok=True)
        ssot_end_dir.mkdir(parents=True, exist_ok=True)

        write_log(run_log_path, "==========================================================")
        write_log(run_log_path, f"[SESSION] StartWorkVs v{SCRIPT_VERSION}")
        write_log(run_log_path, f"[SESSION] RepoRootDir={repo_root_dir}")
        write_log(run_log_path, f"[SESSION] UprojectPath={UPROJECT_PATH}")
        write_log(run_log_path, f"[SESSION] SolutionPath={SOLUTION_PATH}")
        write_log(run_log_path, f"[SESSION] DevenvPath={devenv_path}")
        write_log(run_log_path, f"[SESSION] SessionDir={str(session_dir)}")
        write_log(run_log_path, "==========================================================")

        # [VAR] 기본 기록용 변수 초기화
        vs_exit_code = -1

        start_ssot_raw_exit = -1
        start_ssot_norm_exit = -1
        start_ssot_file_count = -1
        start_ssot_cmdlet_log = ""

        end_ssot_raw_exit = -1
        end_ssot_norm_exit = -1
        end_ssot_file_count = -1
        end_ssot_cmdlet_log = ""

        git_delta_data: Dict[str, Any] = {}
        ssot_delta_data: Dict[str, Any] = {}
        git_delta_txt_path = ""
        ssot_delta_txt_path = ""
        session_report_txt_path = ""
        session_report_json_path = ""

        # Start meta + git
        write_meta(session_dir, "start", repo_root_dir, devenv_path)
        write_git_snapshot(session_dir, repo_root_dir, "start")
        write_log(run_log_path, "[GIT] Wrote snapshot (start)")

        # Start SSOT (엄격: 파일 생성 여부로 성공 판정)
        start_raw, start_norm, start_count, start_cmdlet_log_path = run_ssot_dump(
            UNREAL_CMD_PATH, UPROJECT_PATH, ssot_start_dir, "start_", "Start", run_log_path
        )
        start_ssot_raw_exit = start_raw
        start_ssot_norm_exit = start_norm
        start_ssot_file_count = start_count
        start_ssot_cmdlet_log = str(start_cmdlet_log_path)

        write_log(
            run_log_path,
            f"[SSOT] Start raw_exit={start_ssot_raw_exit} norm_exit={start_ssot_norm_exit} "
            f"files={start_ssot_file_count} log={start_ssot_cmdlet_log}",
        )

        if start_ssot_norm_exit != 0:
            write_log(run_log_path, "[ERROR] Start SSOTDump failed. (no output files) Session aborted (strict).")
            session_end_time_fail = datetime.datetime.now()
            duration_seconds_fail = int((session_end_time_fail - session_start_time).total_seconds())

            write_summary(
                session_dir,
                start_ssot_norm=start_ssot_norm_exit,
                end_ssot_norm=-1,
                vs_exit_code=-1,
                start_ssot_raw=start_ssot_raw_exit,
                end_ssot_raw=-1,
                start_ssot_file_count=start_ssot_file_count,
                end_ssot_file_count=-1,
                start_ssot_log_path=start_ssot_cmdlet_log,
                end_ssot_log_path="",
                git_delta_data=None,
                ssot_delta_data=None,
                session_report_txt_path="",
                session_report_json_path="",
                git_delta_txt_path="",
                ssot_delta_txt_path="",
                start_time_iso=session_start_time.isoformat(timespec="seconds"),
                end_time_iso=session_end_time_fail.isoformat(timespec="seconds"),
                duration_seconds=duration_seconds_fail,
            )
            return start_ssot_norm_exit

        # VS 실행
        write_log(run_log_path, "[VS] Launch Visual Studio (wait until exit)...")
        vs_exit_code = subprocess.call([devenv_path, SOLUTION_PATH])
        write_log(run_log_path, f"[VS] devenv.exe exit code={vs_exit_code}")

        # UE 잔류 체크(경고)
        if is_ue_running():
            write_log(run_log_path, "[WARN] UnrealEditor.exe 프로세스가 아직 남아있습니다. (예외 케이스)")

        # End git
        write_git_snapshot(session_dir, repo_root_dir, "end")
        write_log(run_log_path, "[GIT] Wrote snapshot (end)")

        # End SSOT
        end_raw, end_norm, end_count, end_cmdlet_log_path = run_ssot_dump(
            UNREAL_CMD_PATH, UPROJECT_PATH, ssot_end_dir, "end_", "End", run_log_path
        )
        end_ssot_raw_exit = end_raw
        end_ssot_norm_exit = end_norm
        end_ssot_file_count = end_count
        end_ssot_cmdlet_log = str(end_cmdlet_log_path)

        write_log(
            run_log_path,
            f"[SSOT] End raw_exit={end_ssot_raw_exit} norm_exit={end_ssot_norm_exit} "
            f"files={end_ssot_file_count} log={end_ssot_cmdlet_log}",
        )

        # End meta
        write_meta(session_dir, "end", repo_root_dir, devenv_path)

        # Delta 생성 (v1.2 + v2.0)
        git_delta_txt_path_obj, git_delta_data = write_git_delta(session_dir)
        git_delta_txt_path = str(git_delta_txt_path_obj)
        write_log(run_log_path, f"[GIT] Delta report={git_delta_txt_path}")

        ssot_delta_txt_path_obj, ssot_delta_data = write_ssot_delta(ssot_start_dir, ssot_end_dir, session_dir)
        ssot_delta_txt_path = str(ssot_delta_txt_path_obj)
        write_log(run_log_path, f"[SSOT] Delta report={ssot_delta_txt_path}")

        # 통합 세션 리포트 (v2.0)
        session_end_time = datetime.datetime.now()

        start_ssot_info = {
            "raw_exit": start_ssot_raw_exit,
            "norm_exit": start_ssot_norm_exit,
            "file_count": start_ssot_file_count,
            "cmdlet_log": start_ssot_cmdlet_log,
        }
        end_ssot_info = {
            "raw_exit": end_ssot_raw_exit,
            "norm_exit": end_ssot_norm_exit,
            "file_count": end_ssot_file_count,
            "cmdlet_log": end_ssot_cmdlet_log,
        }

        session_report_txt_obj, session_report_json_obj = write_session_report(
            session_dir=session_dir,
            repo_root_dir=repo_root_dir,
            solution_path=SOLUTION_PATH,
            devenv_path=devenv_path,
            start_time=session_start_time,
            end_time=session_end_time,
            vs_exit_code=vs_exit_code,
            start_ssot_info=start_ssot_info,
            end_ssot_info=end_ssot_info,
            git_delta_data=git_delta_data,
            ssot_delta_data=ssot_delta_data,
        )
        session_report_txt_path = str(session_report_txt_obj)
        session_report_json_path = str(session_report_json_obj)
        write_log(run_log_path, f"[REPORT] Session report={session_report_txt_path}")

        # summary (v2.0 확장)
        duration_seconds = int((session_end_time - session_start_time).total_seconds())

        write_summary(
            session_dir,
            start_ssot_norm=start_ssot_norm_exit,
            end_ssot_norm=end_ssot_norm_exit,
            vs_exit_code=vs_exit_code,
            start_ssot_raw=start_ssot_raw_exit,
            end_ssot_raw=end_ssot_raw_exit,
            start_ssot_file_count=start_ssot_file_count,
            end_ssot_file_count=end_ssot_file_count,
            start_ssot_log_path=start_ssot_cmdlet_log,
            end_ssot_log_path=end_ssot_cmdlet_log,
            git_delta_data=git_delta_data,
            ssot_delta_data=ssot_delta_data,
            session_report_txt_path=session_report_txt_path,
            session_report_json_path=session_report_json_path,
            git_delta_txt_path=git_delta_txt_path,
            ssot_delta_txt_path=ssot_delta_txt_path,
            start_time_iso=session_start_time.isoformat(timespec="seconds"),
            end_time_iso=session_end_time.isoformat(timespec="seconds"),
            duration_seconds=duration_seconds,
        )

        write_log(
            run_log_path,
            f"[DONE] StartSsot={start_ssot_norm_exit} EndSsot={end_ssot_norm_exit} VsExit={vs_exit_code}",
        )
        write_log(run_log_path, f"[DONE] SessionDir={str(session_dir)}")

        # End SSOT 실패 시 종료 코드로 반영
        return end_ssot_norm_exit if end_ssot_norm_exit != 0 else 0

    except Exception as exception_object:
        print(str(exception_object))
        return 1


if __name__ == "__main__":
    sys.exit(main())
