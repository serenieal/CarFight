# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.1.1
# Date: 2026-08-10
# Description: CF-FQ-032 D1-09B UI Assetization Probe/DryRun/Apply/Readback Unreal Python 도구
# Scope: Font·Semantic Icon·Style/Density/1080 Layout Asset과 Font Notice의 정확한 D1-09B 화이트리스트만 생성·검증합니다.
# Changelog:
# - v1.1.1: DryRun의 CompositeFont round-trip은 아직 존재하지 않는 FontFace 참조를 UE가 Null로 정규화하는 특성을 반영해 Typeface 이름·문법만 검사하고 실제 Face Reference는 Apply Readback에서 검증하도록 분리.
# - v1.1.0: 공식 Font/License 확보, 8 FontFace, 2 Runtime Composite Font, 18 Semantic Icon Texture, Style/Density/Layout DataAsset, Notice와 Readback 구현.
# - v1.0.1: BatchCreateFontAsset enum·FontFileImportFactory property와 /Engine/EngineFonts Runtime Font Composite export_text Probe를 추가.
# - v1.0.0: D1-09B 구현 전 UE 5.8 실제 Python 타입·enum·editor property 노출을 확인하는 Probe 모드 최초 추가.
# Migration:
# - DryRun은 Saved 임시 Source만 준비하며 Content·Config·ThirdPartyNotices를 수정하지 않습니다.
# - Apply는 이 파일에 정의된 D1-09B Asset/Notice 화이트리스트만 생성·저장합니다. Config는 저장소 도구가 별도로 연결합니다.
# - Readback은 Content·Config·Notice를 수정하지 않고 D1-09B 전체 계약을 검증합니다.

from __future__ import annotations

import hashlib
import io
import json
import math
import os
import struct
import traceback
import urllib.request
import zipfile
import zlib
from pathlib import Path
from typing import Any, Callable

import unreal


# [v1.1.0] 도구 결과에서 식별할 현재 도구 버전입니다.
TOOL_VERSION = "1.1.1"

# [v1.1.0] 실행할 동작을 probe/dry_run/apply/readback 중 하나로 선택하는 환경 변수 값입니다.
RUN_MODE = os.environ.get("CARFIGHT_UI_ASSET_MODE", "probe").strip().lower()

# [v1.1.0] Unreal 프로젝트 루트의 절대 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.1.0] CarFight 저장소 루트의 절대 경로입니다.
REPOSITORY_DIR = PROJECT_DIR.parent

# [v1.1.0] Probe와 D1-09B 구조화 보고서를 저장할 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "UIAssetization"

# [v1.1.0] 공식 Font 원본과 License의 임시 저장 디렉터리입니다.
SOURCE_FONT_DIR = REPORT_DIR / "SourceFonts"

# [v1.1.0] CarFight 소유 Semantic Icon PNG 원본의 임시 저장 디렉터리입니다.
ICON_SOURCE_DIR = REPORT_DIR / "IconSource"

# [v1.1.0] MCP와 후속 세션이 읽을 구조화 결과 JSON 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.1.0] Pretendard 공식 GitHub Release Tag입니다.
PRETENDARD_VERSION = "1.3.9"

# [v1.1.0] Pretendard 공식 Release metadata API URL입니다.
PRETENDARD_RELEASE_API = f"https://api.github.com/repos/orioncactus/pretendard/releases/tags/v{PRETENDARD_VERSION}"

# [v1.1.0] Pretendard 원본 Copyright Notice와 OFL 전문 공식 URL입니다.
PRETENDARD_LICENSE_URL = "https://raw.githubusercontent.com/orioncactus/pretendard/main/LICENSE"

# [v1.1.0] IBM Plex 원본 Copyright Notice와 OFL 전문 공식 URL입니다.
IBM_PLEX_LICENSE_URL = "https://raw.githubusercontent.com/IBM/plex/master/LICENSE.txt"

# [v1.1.0] Pretendard 원본 License를 프로젝트에 보존할 고정 경로입니다.
PRETENDARD_NOTICE_PATH = PROJECT_DIR / "ThirdPartyNotices" / "Fonts" / "Pretendard-OFL-1.1.txt"

# [v1.1.0] IBM Plex 원본 License를 프로젝트에 보존할 고정 경로입니다.
IBM_PLEX_NOTICE_PATH = PROJECT_DIR / "ThirdPartyNotices" / "Fonts" / "IBMPlex-OFL-1.1.txt"

# [v1.1.0] D1-09B Style DataAsset 전체 경로입니다.
STYLE_ASSET_PATH = "/Game/CarFight/UI/Style/DA_CFUIStyle_Default"

# [v1.1.0] D1-09B Compact Density DataAsset 전체 경로입니다.
DENSITY_COMPACT_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Compact"

# [v1.1.0] D1-09B Standard Density DataAsset 전체 경로입니다.
DENSITY_STANDARD_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Standard"

# [v1.1.0] D1-09B Expanded Density DataAsset 전체 경로입니다.
DENSITY_EXPANDED_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Expanded"

# [v1.1.0] D1-07 USER PASS 1920x1080 Layout DataAsset 전체 경로입니다.
LAYOUT_1080_PATH = "/Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16"

# [v1.1.0] UI Runtime Composite UFont 전체 경로입니다.
UI_FONT_PATH = "/Game/CarFight/UI/Fonts/Font_CFUI"

# [v1.1.0] Numeric Runtime Composite UFont 전체 경로입니다.
NUMERIC_FONT_PATH = "/Game/CarFight/UI/Fonts/Font_CFNumeric"

# [v1.1.0] 실제 FontFace와 UFont을 보관할 Unreal Content 폴더입니다.
FONT_ASSET_FOLDER = "/Game/CarFight/UI/Fonts"

# [v1.1.0] Semantic Icon Texture를 보관할 Unreal Content 폴더입니다.
ICON_ASSET_FOLDER = "/Game/CarFight/UI/Icons/Semantic"

# [v1.1.0] D1-09B에서 금지된 후속 Layout Profile 전체 경로입니다.
FORBIDDEN_LAYOUT_PATHS = [
    "/Game/CarFight/UI/Layout/DA_CFHUDLayout_1440_16",
    "/Game/CarFight/UI/Layout/DA_CFHUDLayout_21x9",
    "/Game/CarFight/UI/Layout/DA_CFHUDLayout_32x9",
]

# [v1.1.0] 두 Font Family에서 사용할 논리 Weight 순서입니다.
FONT_WEIGHTS = ["Regular", "Medium", "SemiBold", "Bold"]

# [v1.1.0] Pretendard Weight별 FontFace Asset과 원본 파일 규격입니다.
PRETENDARD_FACE_SPECS = {
    weight: {
        "asset_name": f"FF_Pretendard_{weight}",
        "asset_path": f"{FONT_ASSET_FOLDER}/FF_Pretendard_{weight}",
        "source_basename_candidates": [f"Pretendard-{weight}.otf", f"Pretendard-{weight}.ttf"],
    }
    for weight in FONT_WEIGHTS
}

# [v1.1.0] IBM Plex Mono Weight별 FontFace Asset과 공식 원본 파일 규격입니다.
IBM_PLEX_FACE_SPECS = {
    weight: {
        "asset_name": f"FF_IBMPlexMono_{weight}",
        "asset_path": f"{FONT_ASSET_FOLDER}/FF_IBMPlexMono_{weight}",
        "source_url": f"https://raw.githubusercontent.com/IBM/plex/master/packages/plex-mono/fonts/complete/ttf/IBMPlexMono-{weight}.ttf",
        "source_basename": f"IBMPlexMono-{weight}.ttf",
    }
    for weight in FONT_WEIGHTS
}

# [v1.1.0] D1-09B 최소 Semantic Icon ID와 Texture Asset 이름 규격입니다.
SEMANTIC_ICON_SPECS = [
    ("Vehicle", "T_UII_Vehicle", "vehicle"),
    ("Shield", "T_UII_Shield", "shield"),
    ("Armor", "T_UII_Armor", "armor"),
    ("Integrity", "T_UII_Integrity", "integrity"),
    ("Engine", "T_UII_Engine", "engine"),
    ("Steering", "T_UII_Steering", "steering"),
    ("Turret", "T_UII_Turret", "turret"),
    ("Ammo", "T_UII_Ammo", "ammo"),
    ("Battery", "T_UII_Battery", "battery"),
    ("Charge", "T_UII_Charge", "charge"),
    ("Heat", "T_UII_Heat", "heat"),
    ("Cooldown", "T_UII_Cooldown", "cooldown"),
    ("Reload", "T_UII_Reload", "reload"),
    ("Target", "T_UII_Target", "target"),
    ("Lock", "T_UII_Lock", "lock"),
    ("RadarContact", "T_UII_RadarContact", "radar_contact"),
    ("Warning", "T_UII_Warning", "warning"),
    ("Critical", "T_UII_Critical", "critical"),
]

# [v1.1.0] D1-09B가 직접 생성·갱신할 수 있는 Unreal Asset 경로 집합입니다.
MUTABLE_ASSET_PATHS = {
    STYLE_ASSET_PATH,
    DENSITY_COMPACT_PATH,
    DENSITY_STANDARD_PATH,
    DENSITY_EXPANDED_PATH,
    LAYOUT_1080_PATH,
    UI_FONT_PATH,
    NUMERIC_FONT_PATH,
    *[spec["asset_path"] for spec in PRETENDARD_FACE_SPECS.values()],
    *[spec["asset_path"] for spec in IBM_PLEX_FACE_SPECS.values()],
    *[f"{ICON_ASSET_FOLDER}/{asset_name}" for _, asset_name, _ in SEMANTIC_ICON_SPECS],
}

# [v1.1.0] D1-09B가 직접 쓸 수 있는 프로젝트 내 일반 파일 경로 집합입니다.
MUTABLE_FILE_PATHS = {
    PRETENDARD_NOTICE_PATH.resolve(),
    IBM_PLEX_NOTICE_PATH.resolve(),
}

# [v1.1.0] Density Preset별 정확한 Token과 Caption Policy 계약입니다.
DENSITY_PRESET_VALUES = {
    "Compact": {
        "preset": "COMPACT",
        "panel_padding": 12.0,
        "panel_inner_gap": 8.0,
        "section_gap": 8.0,
        "info_row_gap": 4.0,
        "header_height": 36.0,
        "info_row_height": 28.0,
        "info_row_height_large": 36.0,
        "chip_height": 22.0,
        "icon_small": 16.0,
        "icon_medium": 20.0,
        "icon_large": 28.0,
        "primary_value_scale": 0.86,
        "secondary_text_scale": 0.90,
        "outline_scale": 0.75,
        "caption_policy": "CORE_ONLY",
    },
    "Standard": {
        "preset": "STANDARD",
        "panel_padding": 24.0,
        "panel_inner_gap": 12.0,
        "section_gap": 16.0,
        "info_row_gap": 8.0,
        "header_height": 48.0,
        "info_row_height": 36.0,
        "info_row_height_large": 44.0,
        "chip_height": 28.0,
        "icon_small": 20.0,
        "icon_medium": 24.0,
        "icon_large": 32.0,
        "primary_value_scale": 1.0,
        "secondary_text_scale": 1.0,
        "outline_scale": 1.0,
        "caption_policy": "CONTEXTUAL",
    },
    "Expanded": {
        "preset": "EXPANDED",
        "panel_padding": 32.0,
        "panel_inner_gap": 16.0,
        "section_gap": 24.0,
        "info_row_gap": 12.0,
        "header_height": 56.0,
        "info_row_height": 44.0,
        "info_row_height_large": 52.0,
        "chip_height": 32.0,
        "icon_small": 24.0,
        "icon_medium": 32.0,
        "icon_large": 40.0,
        "primary_value_scale": 1.12,
        "secondary_text_scale": 1.08,
        "outline_scale": 1.0,
        "caption_policy": "EXPANDED",
    },
}

# [v1.1.0] Layout Slot별 D1-07 USER PASS 기대값입니다.
EXPECTED_LAYOUT_SLOTS = {
    "MISSION_SUMMARY": ((0.0, 0.0), (0.0, 0.0), (0.0, 0.0), (24.0, 24.0), (375.0, 87.0), 10),
    "ALERT_FEED": ((0.5, 0.0), (0.5, 0.0), (0.5, 0.0), (6.0, 24.0), (552.0, 81.0), 40),
    "TARGET_PANEL": ((1.0, 0.0), (1.0, 0.0), (1.0, 0.0), (-24.0, 24.0), (312.0, 232.5), 10),
    "VEHICLE_PANEL": ((0.0, 1.0), (0.0, 1.0), (0.0, 1.0), (24.0, -24.0), (672.0, 312.0), 10),
    "RADAR_PANEL": ((0.5, 1.0), (0.5, 1.0), (0.5, 1.0), (0.0, -24.0), (285.0, 270.0), 10),
    "WEAPON_PANEL": ((1.0, 1.0), (1.0, 1.0), (1.0, 1.0), (-24.0, -24.0), (270.0, 190.5), 10),
    "RETICLE_LAYER": ((0.0, 0.0), (1.0, 1.0), (0.0, 0.0), (0.0, 0.0), (0.0, 0.0), 30),
}

# [v1.1.0] 1080p Typography Role별 최소 실효 Font Size 계약입니다.
EXPECTED_FONT_FLOORS = {
    "display_xl": 38,
    "display_l": 32,
    "value_m": 24,
    "value_s": 14,
    "heading_l": 22,
    "heading_m": 18,
    "body": 16,
    "label": 14,
    "caption": 12,
}

# [v1.1.0] 실행 결과, 변경 목록과 검증 Evidence를 기록할 보고서 객체입니다.
REPORT: dict[str, Any] = {
    "schema_version": "carfight_ui_assetization_v2",
    "tool_version": TOOL_VERSION,
    "mode": RUN_MODE,
    "success": False,
    "dry_run_contract_passed": False,
    "asset_contract_passed": False,
    "config_contract_passed": False,
    "license_contract_passed": False,
    "d1_09b_pass": False,
    "planned_assets": sorted(MUTABLE_ASSET_PATHS),
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "source_files": {},
    "notice_files": {},
    "asset_readback": {},
    "config_readback": {},
    "existing_targets_before": [],
    "forbidden_assets_found": [],
    "errors": [],
}


# [v1.1.0] Unreal Output Log에 D1-09B 도구 메시지를 남깁니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][UIAssetization] {message}")


# [v1.1.0] 파일 전체 바이트의 SHA-256 문자열을 반환합니다.
def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


# [v1.1.0] 존재하는 파일의 SHA-256을 반환하고 없으면 빈 문자열을 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
    return sha256_bytes(file_path.read_bytes())


# [v1.1.0] Unreal Object 또는 Soft Reference를 비교 가능한 Object Path 문자열로 변환합니다.
def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        path_name = unreal.SystemLibrary.get_path_name(value)
        if path_name:
            return str(path_name)
    except Exception:
        pass
    return str(value)


# [v1.1.0] Unreal Asset 경로에 해당하는 실제 .uasset 파일 경로를 반환합니다.
def asset_file_path(asset_path: str) -> Path:
    if not asset_path.startswith("/Game/"):
        raise RuntimeError(f"Only /Game asset paths are supported: {asset_path}")
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'): ]}.uasset"


# [v1.1.0] 지정 Unreal Asset을 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return asset


# [v1.1.0] 현재 Asset이 기대한 Unreal Python Class인지 검증합니다.
def require_asset_class(asset_path: str, expected_class: Any) -> Any:
    asset = require_asset(asset_path)
    if not isinstance(asset, expected_class):
        raise RuntimeError(
            f"Asset class mismatch: {asset_path} actual={asset.get_class().get_name()} expected={expected_class.__name__}"
        )
    return asset


# [v1.1.0] Unreal Asset 폴더가 없으면 Apply 모드에서만 생성합니다.
def ensure_asset_directory(asset_folder: str) -> None:
    if unreal.EditorAssetLibrary.does_directory_exist(asset_folder):
        return
    if RUN_MODE != "apply":
        return
    if not unreal.EditorAssetLibrary.make_directory(asset_folder):
        raise RuntimeError(f"Asset folder creation failed: {asset_folder}")


# [v1.1.0] D1-09B 화이트리스트를 확인한 뒤 Asset을 강제로 저장합니다.
def save_asset(asset_path: str, asset: Any) -> None:
    if RUN_MODE != "apply":
        return
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in D1-09B mutation whitelist: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.1.0] 프로젝트 일반 파일 화이트리스트를 확인한 뒤 원본 바이트를 보존해 씁니다.
def write_notice_file(target_path: Path, source_bytes: bytes) -> None:
    if RUN_MODE != "apply":
        return
    resolved_path = target_path.resolve()
    if resolved_path not in MUTABLE_FILE_PATHS:
        raise RuntimeError(f"File path is not in D1-09B mutation whitelist: {target_path}")
    target_path.parent.mkdir(parents=True, exist_ok=True)
    target_path.write_bytes(source_bytes)


# [v1.1.0] 공식 HTTPS URL에서 바이트를 내려받고 빈 응답을 거부합니다.
def download_bytes(url: str) -> bytes:
    request = urllib.request.Request(
        url,
        headers={
            "User-Agent": "CarFight-D1-09B/1.1.0",
            "Accept": "application/vnd.github+json,application/octet-stream;q=0.9,*/*;q=0.8",
        },
    )
    with urllib.request.urlopen(request, timeout=120) as response:
        data = response.read()
    if not data:
        raise RuntimeError(f"Downloaded file is empty: {url}")
    return data


# [v1.1.0] 기존 임시 Source가 있으면 재사용하고 없으면 공식 URL에서 내려받습니다.
def download_to_saved(url: str, target_path: Path) -> Path:
    target_path.parent.mkdir(parents=True, exist_ok=True)
    if target_path.is_file() and target_path.stat().st_size > 0:
        return target_path
    data = download_bytes(url)
    target_path.write_bytes(data)
    return target_path


# [v1.1.0] Pretendard 공식 GitHub Release ZIP에서 요청 Weight의 정적 OTF/TTF를 Saved로 추출합니다.
def prepare_pretendard_sources() -> dict[str, Path]:
    metadata_bytes = download_bytes(PRETENDARD_RELEASE_API)
    metadata = json.loads(metadata_bytes.decode("utf-8"))
    release_assets = list(metadata.get("assets", []))

    zip_candidates = [
        asset
        for asset in release_assets
        if str(asset.get("name", "")).lower().endswith(".zip")
        and str(asset.get("name", "")).startswith("Pretendard-")
    ]
    if not zip_candidates:
        raise RuntimeError(
            f"Official Pretendard v{PRETENDARD_VERSION} primary release ZIP was not found. assets={[asset.get('name') for asset in release_assets]}"
        )

    zip_asset = zip_candidates[0]
    zip_name = str(zip_asset["name"])
    zip_url = str(zip_asset["browser_download_url"])
    zip_path = download_to_saved(zip_url, SOURCE_FONT_DIR / zip_name)

    prepared_sources: dict[str, Path] = {}
    with zipfile.ZipFile(zip_path, "r") as archive:
        archive_names = archive.namelist()
        for weight, spec in PRETENDARD_FACE_SPECS.items():
            matched_names: list[str] = []
            for basename in spec["source_basename_candidates"]:
                matched_names.extend(
                    name for name in archive_names if Path(name).name == basename
                )
            if not matched_names:
                raise RuntimeError(
                    f"Pretendard {weight} static OTF/TTF was not found in official release ZIP: {zip_name}"
                )
            preferred_name = sorted(
                matched_names,
                key=lambda name: (
                    "public/static" not in name.replace("\\", "/").lower(),
                    "pretendard-jp" in name.lower(),
                    "pretendard-gov" in name.lower(),
                    "pretendard-std" in name.lower(),
                    len(name),
                ),
            )[0]
            source_suffix = Path(preferred_name).suffix.lower()
            target_path = SOURCE_FONT_DIR / f"Pretendard-{weight}{source_suffix}"
            target_path.write_bytes(archive.read(preferred_name))
            prepared_sources[weight] = target_path

    REPORT["source_files"]["pretendard_release"] = {
        "version": PRETENDARD_VERSION,
        "asset_name": zip_name,
        "url": zip_url,
        "sha256": sha256_file(zip_path),
    }
    return prepared_sources


# [v1.1.0] IBM 공식 저장소에서 Plex Mono 정적 TTF 네 Weight를 Saved로 준비합니다.
def prepare_ibm_plex_sources() -> dict[str, Path]:
    prepared_sources: dict[str, Path] = {}
    for weight, spec in IBM_PLEX_FACE_SPECS.items():
        target_path = download_to_saved(
            spec["source_url"],
            SOURCE_FONT_DIR / spec["source_basename"],
        )
        prepared_sources[weight] = target_path
    return prepared_sources


# [v1.1.0] 두 Font Family의 공식 Copyright Notice + OFL 원문을 내려받고 원문 marker를 검증합니다.
def prepare_license_sources() -> dict[str, bytes]:
    pretendard_license = download_bytes(PRETENDARD_LICENSE_URL)
    ibm_license = download_bytes(IBM_PLEX_LICENSE_URL)

    pretendard_text = pretendard_license.decode("utf-8-sig")
    ibm_text = ibm_license.decode("utf-8-sig")
    if "SIL OPEN FONT LICENSE Version 1.1" not in pretendard_text or "Reserved Font Name 'Pretendard'" not in pretendard_text:
        raise RuntimeError("Pretendard official LICENSE does not contain the required OFL 1.1 / Reserved Font Name markers.")
    if "SIL OPEN FONT LICENSE Version 1.1" not in ibm_text or "Reserved Font Name \"Plex\"" not in ibm_text:
        raise RuntimeError("IBM Plex official LICENSE does not contain the required OFL 1.1 / Reserved Font Name markers.")

    REPORT["source_files"]["pretendard_license"] = {
        "url": PRETENDARD_LICENSE_URL,
        "sha256": sha256_bytes(pretendard_license),
        "bytes": len(pretendard_license),
    }
    REPORT["source_files"]["ibm_plex_license"] = {
        "url": IBM_PLEX_LICENSE_URL,
        "sha256": sha256_bytes(ibm_license),
        "bytes": len(ibm_license),
    }
    return {
        "pretendard": pretendard_license,
        "ibm_plex": ibm_license,
    }


# [v1.1.0] Prepared Font 원본과 License의 존재·크기·SHA-256을 보고서에 기록합니다.
def record_prepared_sources(
    pretendard_sources: dict[str, Path],
    ibm_sources: dict[str, Path],
) -> None:
    for family_name, source_map in [
        ("Pretendard", pretendard_sources),
        ("IBMPlexMono", ibm_sources),
    ]:
        for weight, source_path in source_map.items():
            if not source_path.is_file() or source_path.stat().st_size <= 0:
                raise RuntimeError(f"Prepared font source is missing: {source_path}")
            REPORT["source_files"][f"{family_name}_{weight}"] = {
                "path": str(source_path),
                "bytes": source_path.stat().st_size,
                "sha256": sha256_file(source_path),
            }


# [v1.1.0] RGBA Canvas의 단일 Pixel을 범위 검사 후 설정합니다.
def set_pixel(canvas: bytearray, x: int, y: int, rgba: tuple[int, int, int, int]) -> None:
    if x < 0 or y < 0 or x >= 128 or y >= 128:
        return
    offset = (y * 128 + x) * 4
    canvas[offset : offset + 4] = bytes(rgba)


# [v1.1.0] 지정 사각형을 단색 RGBA로 채웁니다.
def fill_rect(canvas: bytearray, x0: int, y0: int, x1: int, y1: int, rgba: tuple[int, int, int, int]) -> None:
    for y in range(max(0, y0), min(128, y1)):
        for x in range(max(0, x0), min(128, x1)):
            set_pixel(canvas, x, y, rgba)


# [v1.1.0] 지정 중심·반지름의 원을 단색 RGBA로 채웁니다.
def fill_circle(canvas: bytearray, cx: float, cy: float, radius: float, rgba: tuple[int, int, int, int]) -> None:
    radius_squared = radius * radius
    for y in range(max(0, int(cy - radius - 1)), min(128, int(cy + radius + 2))):
        for x in range(max(0, int(cx - radius - 1)), min(128, int(cx + radius + 2))):
            if (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2 <= radius_squared:
                set_pixel(canvas, x, y, rgba)


# [v1.1.0] 두 점 사이를 원형 Brush로 이어 굵은 선을 그립니다.
def draw_thick_line(
    canvas: bytearray,
    x0: float,
    y0: float,
    x1: float,
    y1: float,
    thickness: float,
    rgba: tuple[int, int, int, int],
) -> None:
    distance = max(1.0, math.hypot(x1 - x0, y1 - y0))
    step_count = max(1, int(distance * 1.5))
    for step_index in range(step_count + 1):
        alpha = step_index / step_count
        x = x0 + (x1 - x0) * alpha
        y = y0 + (y1 - y0) * alpha
        fill_circle(canvas, x, y, thickness * 0.5, rgba)


# [v1.1.0] Point-in-Polygon 홀짝 규칙으로 다각형 내부 여부를 반환합니다.
def point_in_polygon(x: float, y: float, points: list[tuple[float, float]]) -> bool:
    inside = False
    previous_index = len(points) - 1
    for current_index, current_point in enumerate(points):
        previous_point = points[previous_index]
        intersects = (
            (current_point[1] > y) != (previous_point[1] > y)
            and x
            < (previous_point[0] - current_point[0])
            * (y - current_point[1])
            / ((previous_point[1] - current_point[1]) or 1e-6)
            + current_point[0]
        )
        if intersects:
            inside = not inside
        previous_index = current_index
    return inside


# [v1.1.0] 지정 다각형 내부 Pixel을 단색 RGBA로 채웁니다.
def fill_polygon(canvas: bytearray, points: list[tuple[float, float]], rgba: tuple[int, int, int, int]) -> None:
    minimum_x = max(0, int(min(point[0] for point in points)))
    maximum_x = min(127, int(max(point[0] for point in points)) + 1)
    minimum_y = max(0, int(min(point[1] for point in points)))
    maximum_y = min(127, int(max(point[1] for point in points)) + 1)
    for y in range(minimum_y, maximum_y + 1):
        for x in range(minimum_x, maximum_x + 1):
            if point_in_polygon(x + 0.5, y + 0.5, points):
                set_pixel(canvas, x, y, rgba)


# [v1.1.0] Solid Core 형상에 공통 Tactical Cut을 추가합니다.
def apply_tactical_cut(canvas: bytearray) -> None:
    draw_thick_line(canvas, 91, 99, 111, 79, 5, (0, 0, 0, 0))


# [v1.1.0] Semantic ID별 128x128 흰색 단색 Icon 형상을 그립니다.
def draw_semantic_icon(icon_key: str) -> bytearray:
    canvas = bytearray(128 * 128 * 4)
    white = (255, 255, 255, 255)
    clear = (0, 0, 0, 0)

    if icon_key == "vehicle":
        fill_rect(canvas, 42, 14, 86, 114, white)
        fill_rect(canvas, 32, 42, 96, 86, white)
        fill_rect(canvas, 43, 45, 85, 78, clear)
        fill_rect(canvas, 27, 50, 38, 72, white)
        fill_rect(canvas, 90, 50, 101, 72, white)
    elif icon_key == "shield":
        fill_polygon(canvas, [(64, 10), (106, 29), (97, 78), (64, 116), (31, 78), (22, 29)], white)
        fill_polygon(canvas, [(64, 28), (89, 39), (83, 70), (64, 94), (45, 70), (39, 39)], clear)
    elif icon_key == "armor":
        fill_polygon(canvas, [(33, 18), (95, 18), (113, 48), (96, 104), (64, 116), (32, 104), (15, 48)], white)
        fill_polygon(canvas, [(43, 35), (85, 35), (94, 52), (84, 88), (64, 97), (44, 88), (34, 52)], clear)
    elif icon_key == "integrity":
        fill_rect(canvas, 22, 46, 106, 90, white)
        fill_polygon(canvas, [(36, 46), (48, 26), (80, 26), (92, 46)], white)
        fill_circle(canvas, 40, 94, 12, white)
        fill_circle(canvas, 88, 94, 12, white)
        fill_rect(canvas, 34, 58, 94, 66, clear)
    elif icon_key == "engine":
        fill_rect(canvas, 28, 38, 91, 93, white)
        fill_rect(canvas, 18, 51, 30, 79, white)
        fill_rect(canvas, 91, 51, 108, 65, white)
        fill_rect(canvas, 47, 26, 70, 40, white)
        fill_rect(canvas, 42, 51, 76, 63, clear)
    elif icon_key == "steering":
        fill_circle(canvas, 64, 64, 48, white)
        fill_circle(canvas, 64, 64, 35, clear)
        fill_circle(canvas, 64, 64, 10, white)
        draw_thick_line(canvas, 64, 64, 64, 104, 8, white)
        draw_thick_line(canvas, 64, 64, 30, 43, 8, white)
        draw_thick_line(canvas, 64, 64, 98, 43, 8, white)
    elif icon_key == "turret":
        fill_circle(canvas, 54, 72, 30, white)
        fill_rect(canvas, 54, 62, 114, 76, white)
        fill_rect(canvas, 26, 96, 83, 108, white)
        fill_circle(canvas, 54, 72, 13, clear)
    elif icon_key == "ammo":
        fill_polygon(canvas, [(54, 10), (78, 10), (88, 29), (88, 88), (43, 88), (43, 29)], white)
        fill_rect(canvas, 38, 88, 93, 108, white)
        fill_rect(canvas, 52, 39, 79, 48, clear)
    elif icon_key == "battery":
        fill_rect(canvas, 20, 32, 102, 102, white)
        fill_rect(canvas, 102, 52, 112, 82, white)
        fill_rect(canvas, 34, 46, 88, 88, clear)
        fill_rect(canvas, 56, 54, 67, 80, white)
        fill_rect(canvas, 48, 62, 75, 72, white)
    elif icon_key == "charge":
        fill_polygon(canvas, [(70, 8), (31, 67), (57, 67), (48, 119), (98, 52), (70, 52)], white)
    elif icon_key == "heat":
        fill_circle(canvas, 51, 94, 24, white)
        fill_rect(canvas, 43, 23, 59, 94, white)
        fill_circle(canvas, 51, 94, 11, clear)
        fill_rect(canvas, 47, 45, 55, 88, clear)
        fill_polygon(canvas, [(82, 30), (101, 58), (89, 58), (105, 85), (77, 70), (88, 70)], white)
    elif icon_key == "cooldown":
        fill_circle(canvas, 64, 64, 50, white)
        fill_circle(canvas, 64, 64, 38, clear)
        draw_thick_line(canvas, 64, 64, 64, 34, 8, white)
        draw_thick_line(canvas, 64, 64, 88, 70, 8, white)
        fill_polygon(canvas, [(93, 18), (117, 22), (105, 43)], white)
    elif icon_key == "reload":
        draw_thick_line(canvas, 24, 54, 42, 29, 12, white)
        draw_thick_line(canvas, 42, 29, 79, 24, 12, white)
        draw_thick_line(canvas, 79, 24, 102, 44, 12, white)
        fill_polygon(canvas, [(97, 30), (118, 47), (93, 54)], white)
        draw_thick_line(canvas, 104, 76, 86, 99, 12, white)
        draw_thick_line(canvas, 86, 99, 49, 104, 12, white)
        draw_thick_line(canvas, 49, 104, 26, 84, 12, white)
        fill_polygon(canvas, [(31, 98), (10, 81), (35, 74)], white)
    elif icon_key == "target":
        fill_circle(canvas, 64, 64, 38, white)
        fill_circle(canvas, 64, 64, 25, clear)
        fill_circle(canvas, 64, 64, 7, white)
        fill_rect(canvas, 59, 6, 69, 34, white)
        fill_rect(canvas, 59, 94, 69, 122, white)
        fill_rect(canvas, 6, 59, 34, 69, white)
        fill_rect(canvas, 94, 59, 122, 69, white)
    elif icon_key == "lock":
        fill_rect(canvas, 28, 54, 100, 110, white)
        fill_circle(canvas, 64, 52, 32, white)
        fill_circle(canvas, 64, 52, 19, clear)
        fill_rect(canvas, 23, 51, 105, 65, clear)
        fill_circle(canvas, 64, 80, 7, clear)
        fill_rect(canvas, 61, 80, 67, 97, clear)
    elif icon_key == "radar_contact":
        fill_circle(canvas, 64, 64, 48, white)
        fill_circle(canvas, 64, 64, 41, clear)
        draw_thick_line(canvas, 64, 64, 99, 37, 5, white)
        fill_polygon(canvas, [(80, 70), (91, 81), (80, 92), (69, 81)], white)
        fill_circle(canvas, 43, 48, 6, white)
    elif icon_key == "warning":
        fill_polygon(canvas, [(64, 9), (119, 108), (9, 108)], white)
        fill_polygon(canvas, [(64, 31), (97, 94), (31, 94)], clear)
        fill_rect(canvas, 59, 48, 69, 75, white)
        fill_circle(canvas, 64, 84, 6, white)
    elif icon_key == "critical":
        fill_polygon(canvas, [(43, 9), (85, 9), (119, 43), (119, 85), (85, 119), (43, 119), (9, 85), (9, 43)], white)
        fill_polygon(canvas, [(48, 27), (80, 27), (101, 48), (101, 80), (80, 101), (48, 101), (27, 80), (27, 48)], clear)
        fill_rect(canvas, 59, 39, 69, 72, white)
        fill_circle(canvas, 64, 84, 7, white)
    else:
        raise RuntimeError(f"Unknown semantic icon key: {icon_key}")

    apply_tactical_cut(canvas)
    return canvas


# [v1.1.0] 128x128 RGBA Canvas를 표준 PNG 바이트로 인코딩합니다.
def encode_png_rgba(canvas: bytearray) -> bytes:
    def png_chunk(chunk_type: bytes, chunk_data: bytes) -> bytes:
        return (
            struct.pack(">I", len(chunk_data))
            + chunk_type
            + chunk_data
            + struct.pack(">I", zlib.crc32(chunk_type + chunk_data) & 0xFFFFFFFF)
        )

    raw_rows = bytearray()
    row_size = 128 * 4
    for row_index in range(128):
        raw_rows.append(0)
        row_start = row_index * row_size
        raw_rows.extend(canvas[row_start : row_start + row_size])

    return (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", struct.pack(">IIBBBBB", 128, 128, 8, 6, 0, 0, 0))
        + png_chunk(b"IDAT", zlib.compress(bytes(raw_rows), 9))
        + png_chunk(b"IEND", b"")
    )


# [v1.1.0] 18개 Semantic Icon PNG 원본을 Saved에 생성하고 SHA-256을 기록합니다.
def prepare_icon_sources() -> dict[str, Path]:
    ICON_SOURCE_DIR.mkdir(parents=True, exist_ok=True)
    prepared_sources: dict[str, Path] = {}
    for semantic_id, asset_name, icon_key in SEMANTIC_ICON_SPECS:
        png_bytes = encode_png_rgba(draw_semantic_icon(icon_key))
        source_path = ICON_SOURCE_DIR / f"{asset_name}.png"
        source_path.write_bytes(png_bytes)
        prepared_sources[semantic_id] = source_path
        REPORT["source_files"][f"Icon_{semantic_id}"] = {
            "path": str(source_path),
            "bytes": len(png_bytes),
            "sha256": sha256_bytes(png_bytes),
        }
    return prepared_sources


# [v1.1.0] 모든 D1-09B Target Asset의 현재 존재 여부를 기록합니다.
def capture_existing_targets() -> list[str]:
    return sorted(
        asset_path
        for asset_path in MUTABLE_ASSET_PATHS
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    )


# [v1.1.0] 금지된 후속 Layout Profile이 D1-09B 실행으로 생성되지 않았는지 확인합니다.
def capture_forbidden_assets() -> list[str]:
    return sorted(
        asset_path
        for asset_path in FORBIDDEN_LAYOUT_PATHS
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    )


# [v1.1.0] FontFace Asset Object Path를 CompositeFont text의 정규 Class Reference 문자열로 변환합니다.
def font_face_reference(asset_path: str) -> str:
    asset_name = asset_path.rsplit("/", 1)[-1]
    return f"/Script/Engine.FontFace'{asset_path}.{asset_name}'"


# [v1.1.0] 4 Weight FontFace를 사용하는 Runtime CompositeFont 직렬화 텍스트를 생성합니다.
def build_composite_font_text(face_paths: dict[str, str]) -> str:
    entries = ",".join(
        f'(Name="{weight}",Font=(FontFaceAsset="{font_face_reference(face_paths[weight])}"))'
        for weight in FONT_WEIGHTS
    )
    return (
        f"(DefaultTypeface=(Fonts=({entries})),"
        "FallbackTypeface=(Typeface=(Fonts=),ScalingFactor=1.000000),"
        "SubTypefaces=,bEnableAscentDescentOverride=True)"
    )


# [v1.1.1] UE 5.8 CompositeFont.import_text/export_text 문법과 4 Typeface 이름을 FontFace 생성 전에 Transient로 검증합니다.
def validate_composite_text_contract() -> dict[str, Any]:
    # [v1.1.1] DryRun에서는 아직 실제 Asset이 존재하지 않으므로 문법 검증용 예정 FontFace 경로입니다.
    planned_face_paths = {
        weight: PRETENDARD_FACE_SPECS[weight]["asset_path"]
        for weight in FONT_WEIGHTS
    }
    # [v1.1.1] Engine Runtime Font와 같은 형식으로 만든 D1-09B CompositeFont 입력 문자열입니다.
    composite_text = build_composite_font_text(planned_face_paths)
    # [v1.1.1] 실제 Content를 만들지 않고 Import/Export 파서를 검증할 Transient CompositeFont입니다.
    composite_font = unreal.CompositeFont()
    composite_font.import_text(composite_text)
    # [v1.1.1] UE가 파싱한 뒤 다시 직렬화한 CompositeFont 문자열입니다.
    exported_text = composite_font.export_text()
    for weight in FONT_WEIGHTS:
        if f'Name="{weight}"' not in exported_text:
            raise RuntimeError(f"CompositeFont text round-trip lost Typeface: {weight}")

    # [v1.1.1] 존재하지 않는 FontFace Object Path는 UE가 Import 시 Null로 정규화하므로 실제 참조 검증은 Apply 후 validate_runtime_fonts가 담당합니다.
    return {
        "input": composite_text,
        "exported": exported_text,
        "face_reference_validation": "DeferredUntilApplyReadback",
    }


# [v1.1.0] 외부 공식 Font/License와 내부 Icon Source를 Saved에 준비합니다.
def prepare_all_sources() -> tuple[dict[str, Path], dict[str, Path], dict[str, Path], dict[str, bytes]]:
    SOURCE_FONT_DIR.mkdir(parents=True, exist_ok=True)
    pretendard_sources = prepare_pretendard_sources()
    ibm_sources = prepare_ibm_plex_sources()
    license_sources = prepare_license_sources()
    icon_sources = prepare_icon_sources()
    record_prepared_sources(pretendard_sources, ibm_sources)
    return pretendard_sources, ibm_sources, icon_sources, license_sources


# [v1.1.0] FontFace 원본 파일을 지정 이름으로 Import하거나 기존 D1-09B Target을 재사용합니다.
def import_font_face(asset_path: str, source_file: Path) -> Any:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"FontFace target is outside D1-09B whitelist: {asset_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        REPORT["updated_assets"].append(asset_path)
        font_face = require_asset_class(asset_path, unreal.FontFace)
    else:
        ensure_asset_directory(FONT_ASSET_FOLDER)
        asset_name = asset_path.rsplit("/", 1)[-1]
        import_factory = unreal.FontFileImportFactory()
        import_factory.set_editor_property(
            "batch_create_font_asset",
            unreal.BatchCreateFontAsset.NO,
        )
        import_task = unreal.AssetImportTask()
        import_task.set_editor_properties(
            {
                "filename": str(source_file),
                "destination_path": FONT_ASSET_FOLDER,
                "destination_name": asset_name,
                "automated": True,
                "replace_existing": False,
                "replace_existing_settings": False,
                "save": False,
                "factory": import_factory,
            }
        )
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([import_task])
        font_face = require_asset_class(asset_path, unreal.FontFace)
        REPORT["created_assets"].append(asset_path)

    font_face.set_editor_property("hinting", unreal.FontHinting.DEFAULT)
    font_face.set_editor_property("loading_policy", unreal.FontLoadingPolicy.LAZY_LOAD)
    font_face.set_editor_property("layout_method", unreal.FontLayoutMethod.METRICS)
    font_face.set_editor_property("enable_distance_field_rendering", False)
    save_asset(asset_path, font_face)
    return font_face


# [v1.1.0] 128x128 PNG를 UI Texture2D로 Import하거나 기존 D1-09B Target을 재사용합니다.
def import_icon_texture(asset_path: str, source_file: Path) -> Any:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Texture target is outside D1-09B whitelist: {asset_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        REPORT["updated_assets"].append(asset_path)
        texture = require_asset_class(asset_path, unreal.Texture2D)
    else:
        ensure_asset_directory(ICON_ASSET_FOLDER)
        asset_name = asset_path.rsplit("/", 1)[-1]
        import_task = unreal.AssetImportTask()
        import_task.set_editor_properties(
            {
                "filename": str(source_file),
                "destination_path": ICON_ASSET_FOLDER,
                "destination_name": asset_name,
                "automated": True,
                "replace_existing": False,
                "replace_existing_settings": False,
                "save": False,
                "factory": unreal.TextureFactory(),
            }
        )
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([import_task])
        texture = require_asset_class(asset_path, unreal.Texture2D)
        REPORT["created_assets"].append(asset_path)

    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    save_asset(asset_path, texture)
    return texture


# [v1.1.0] Runtime Composite UFont을 생성하거나 기존 Target을 재사용하고 정확한 4 Typeface를 적용합니다.
def create_or_configure_runtime_font(asset_path: str, face_paths: dict[str, str]) -> Any:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Font target is outside D1-09B whitelist: {asset_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        REPORT["updated_assets"].append(asset_path)
        font_asset = require_asset_class(asset_path, unreal.Font)
    else:
        ensure_asset_directory(FONT_ASSET_FOLDER)
        asset_name = asset_path.rsplit("/", 1)[-1]
        font_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            asset_name,
            FONT_ASSET_FOLDER,
            unreal.Font,
            unreal.FontFactory(),
        )
        if font_asset is None:
            raise RuntimeError(f"Runtime Font asset creation failed: {asset_path}")
        REPORT["created_assets"].append(asset_path)

    font_asset.set_editor_property("font_cache_type", unreal.FontCacheType.RUNTIME)
    font_asset.set_editor_property("runtime_font_source", unreal.RuntimeFontSource.ASSET)
    composite_font = font_asset.get_editor_property("composite_font")
    composite_font.import_text(build_composite_font_text(face_paths))
    font_asset.set_editor_property("composite_font", composite_font)
    save_asset(asset_path, font_asset)
    return font_asset


# [v1.1.0] 지정 C++ DataAsset Class의 D1-09B Target을 생성하거나 기존 Target을 재사용합니다.
def create_or_load_data_asset(asset_path: str, data_asset_class: Any) -> Any:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"DataAsset target is outside D1-09B whitelist: {asset_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        REPORT["updated_assets"].append(asset_path)
        return require_asset_class(asset_path, data_asset_class)

    asset_folder, asset_name = asset_path.rsplit("/", 1)
    ensure_asset_directory(asset_folder)
    data_asset_factory = unreal.DataAssetFactory()
    data_asset_factory.set_editor_property("data_asset_class", data_asset_class)
    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        asset_folder,
        data_asset_class,
        data_asset_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"DataAsset creation failed: {asset_path}")
    REPORT["created_assets"].append(asset_path)
    return created_asset


# [v1.1.0] Style DataAsset에 두 Runtime Font Hard Reference와 18 Semantic Icon Soft Reference를 연결합니다.
def configure_style_asset(ui_font: Any, numeric_font: Any, icon_assets: dict[str, Any]) -> Any:
    style_asset = create_or_load_data_asset(STYLE_ASSET_PATH, unreal.CFUIStyleData)

    font_assets = style_asset.get_editor_property("font_assets")
    font_assets.set_editor_property("ui_font_asset", ui_font)
    font_assets.set_editor_property("numeric_font_asset", numeric_font)
    font_assets.set_editor_property("regular_typeface_name", unreal.Name("Regular"))
    font_assets.set_editor_property("medium_typeface_name", unreal.Name("Medium"))
    font_assets.set_editor_property("semi_bold_typeface_name", unreal.Name("SemiBold"))
    font_assets.set_editor_property("bold_typeface_name", unreal.Name("Bold"))
    style_asset.set_editor_property("font_assets", font_assets)

    semantic_entry_type = getattr(unreal, "CFUISemanticIconEntry", None)
    if semantic_entry_type is None:
        raise RuntimeError("Unreal Python type is missing: CFUISemanticIconEntry")

    semantic_entries = []
    for semantic_id, _, _ in SEMANTIC_ICON_SPECS:
        entry = semantic_entry_type()
        entry.set_editor_property("semantic_id", unreal.Name(semantic_id))
        entry.set_editor_property("icon_asset", icon_assets[semantic_id])
        semantic_entries.append(entry)

    icon_set = style_asset.get_editor_property("icon_set")
    icon_set.set_editor_property("style_name", unreal.Name("Solid Core + Tactical Cut"))
    icon_set.set_editor_property("base_grid_size", 24.0)
    icon_set.set_editor_property("icon_small", 16.0)
    icon_set.set_editor_property("icon_medium", 20.0)
    icon_set.set_editor_property("icon_large", 28.0)
    icon_set.set_editor_property("weapon_compact", 18.0)
    icon_set.set_editor_property("weapon_selected", 20.0)
    icon_set.set_editor_property("semantic_icons", semantic_entries)
    style_asset.set_editor_property("icon_set", icon_set)

    save_asset(STYLE_ASSET_PATH, style_asset)
    return style_asset


# [v1.1.0] Compact·Standard·Expanded Density DataAsset에 승인된 정확한 Token을 적용합니다.
def configure_density_asset(asset_path: str, preset_name: str) -> Any:
    expected = DENSITY_PRESET_VALUES[preset_name]
    density_asset = create_or_load_data_asset(asset_path, unreal.CFUIDensityData)
    density_asset.set_editor_property(
        "preset",
        getattr(unreal.CFUIDensityPreset, expected["preset"]),
    )

    tokens = density_asset.get_editor_property("tokens")
    for property_name in [
        "panel_padding",
        "panel_inner_gap",
        "section_gap",
        "info_row_gap",
        "header_height",
        "info_row_height",
        "info_row_height_large",
        "chip_height",
        "icon_small",
        "icon_medium",
        "icon_large",
        "primary_value_scale",
        "secondary_text_scale",
        "outline_scale",
    ]:
        tokens.set_editor_property(property_name, expected[property_name])
    tokens.set_editor_property(
        "caption_policy",
        getattr(unreal.CFUICaptionPolicy, expected["caption_policy"]),
    )
    density_asset.set_editor_property("tokens", tokens)
    save_asset(asset_path, density_asset)
    return density_asset


# [v1.1.0] D1-07 USER PASS Native Default를 소유한 1080p Layout DataAsset을 생성하고 핵심 Scalar를 명시합니다.
def configure_layout_asset() -> Any:
    layout_asset = create_or_load_data_asset(LAYOUT_1080_PATH, unreal.CFHUDLayoutData)
    layout_asset.set_editor_property("profile_id", unreal.Name("HUD_1080_16"))
    layout_asset.set_editor_property("profile_version", 1)
    layout_asset.set_editor_property("reference_viewport_size", unreal.Vector2D(1920.0, 1080.0))
    layout_asset.set_editor_property("geometry_scale", 0.75)
    layout_asset.set_editor_property("typography_scale", 0.75)
    save_asset(LAYOUT_1080_PATH, layout_asset)
    return layout_asset


# [v1.1.0] FontFace Asset 8개를 공식 정적 Font 원본에서 Import하고 설정합니다.
def apply_font_faces(
    pretendard_sources: dict[str, Path],
    ibm_sources: dict[str, Path],
) -> tuple[dict[str, Any], dict[str, Any]]:
    pretendard_faces: dict[str, Any] = {}
    ibm_faces: dict[str, Any] = {}
    for weight in FONT_WEIGHTS:
        pretendard_faces[weight] = import_font_face(
            PRETENDARD_FACE_SPECS[weight]["asset_path"],
            pretendard_sources[weight],
        )
        ibm_faces[weight] = import_font_face(
            IBM_PLEX_FACE_SPECS[weight]["asset_path"],
            ibm_sources[weight],
        )
    return pretendard_faces, ibm_faces


# [v1.1.0] 18개 CarFight 소유 PNG Source를 Semantic Icon UI Texture로 Import하고 설정합니다.
def apply_icon_textures(icon_sources: dict[str, Path]) -> dict[str, Any]:
    icon_assets: dict[str, Any] = {}
    for semantic_id, asset_name, _ in SEMANTIC_ICON_SPECS:
        asset_path = f"{ICON_ASSET_FOLDER}/{asset_name}"
        icon_assets[semantic_id] = import_icon_texture(
            asset_path,
            icon_sources[semantic_id],
        )
    return icon_assets


# [v1.1.0] 한 Float 값이 기대값과 지정 오차 안에서 같은지 검증합니다.
def require_nearly_equal(label: str, actual: float, expected: float, tolerance: float = 0.001) -> None:
    if abs(float(actual) - float(expected)) > tolerance:
        raise RuntimeError(f"{label} mismatch: actual={actual} expected={expected}")


# [v1.1.0] FVector2D Reflection 값을 비교 가능한 (x,y) Tuple로 변환합니다.
def vector2_tuple(value: Any) -> tuple[float, float]:
    return (
        float(value.get_editor_property("x")),
        float(value.get_editor_property("y")),
    )


# [v1.1.0] Unreal enum Reflection 값에서 대문자 멤버 이름을 추출합니다.
def enum_member_name(value: Any) -> str:
    text = str(value)
    if "." in text:
        text = text.split(".")[-1]
    return text.strip("<> ").split(":", 1)[0]


# [v1.1.0] FontFace 8개의 Class, 설정과 원본 Source Name을 검증합니다.
def validate_font_faces() -> dict[str, Any]:
    readback: dict[str, Any] = {}
    all_specs = [
        ("Pretendard", PRETENDARD_FACE_SPECS),
        ("IBMPlexMono", IBM_PLEX_FACE_SPECS),
    ]
    for family_name, family_specs in all_specs:
        for weight in FONT_WEIGHTS:
            asset_path = family_specs[weight]["asset_path"]
            font_face = require_asset_class(asset_path, unreal.FontFace)
            if font_face.get_editor_property("hinting") != unreal.FontHinting.DEFAULT:
                raise RuntimeError(f"FontFace Hinting mismatch: {asset_path}")
            if font_face.get_editor_property("loading_policy") != unreal.FontLoadingPolicy.LAZY_LOAD:
                raise RuntimeError(f"FontFace LoadingPolicy mismatch: {asset_path}")
            if font_face.get_editor_property("layout_method") != unreal.FontLayoutMethod.METRICS:
                raise RuntimeError(f"FontFace LayoutMethod mismatch: {asset_path}")
            if bool(font_face.get_editor_property("enable_distance_field_rendering")):
                raise RuntimeError(f"FontFace DistanceField must be Off: {asset_path}")
            source_filename = str(font_face.get_editor_property("source_filename"))
            readback[f"{family_name}_{weight}"] = {
                "path": asset_path,
                "class": font_face.get_class().get_name(),
                "source_filename": source_filename,
                "hinting": str(font_face.get_editor_property("hinting")),
                "loading_policy": str(font_face.get_editor_property("loading_policy")),
                "layout_method": str(font_face.get_editor_property("layout_method")),
                "distance_field": bool(font_face.get_editor_property("enable_distance_field_rendering")),
            }
    return readback


# [v1.1.0] Runtime Composite UFont 두 개의 Cache Type, 4 Typeface Name과 정확한 FontFace 참조를 검증합니다.
def validate_runtime_fonts() -> dict[str, Any]:
    expected_families = {
        UI_FONT_PATH: {
            weight: PRETENDARD_FACE_SPECS[weight]["asset_path"]
            for weight in FONT_WEIGHTS
        },
        NUMERIC_FONT_PATH: {
            weight: IBM_PLEX_FACE_SPECS[weight]["asset_path"]
            for weight in FONT_WEIGHTS
        },
    }
    readback: dict[str, Any] = {}
    for font_path, face_paths in expected_families.items():
        font_asset = require_asset_class(font_path, unreal.Font)
        if font_asset.get_editor_property("font_cache_type") != unreal.FontCacheType.RUNTIME:
            raise RuntimeError(f"Font must use Runtime cache: {font_path}")
        composite_text = font_asset.get_editor_property("composite_font").export_text()
        for weight, face_path in face_paths.items():
            if f'Name="{weight}"' not in composite_text:
                raise RuntimeError(f"Font Typeface missing: {font_path} weight={weight}")
            if face_path not in composite_text:
                raise RuntimeError(f"FontFace reference missing: {font_path} face={face_path}")
        readback[font_path] = {
            "class": font_asset.get_class().get_name(),
            "cache_type": str(font_asset.get_editor_property("font_cache_type")),
            "runtime_font_source": str(font_asset.get_editor_property("runtime_font_source")),
            "composite_export_text": composite_text,
        }
    return readback


# [v1.1.0] 18 Semantic Icon Texture의 Class, 128x128, UI Group, UserInterface2D 압축과 Source 매핑을 검증합니다.
def validate_icon_textures() -> dict[str, Any]:
    readback: dict[str, Any] = {}
    for semantic_id, asset_name, _ in SEMANTIC_ICON_SPECS:
        asset_path = f"{ICON_ASSET_FOLDER}/{asset_name}"
        texture = require_asset_class(asset_path, unreal.Texture2D)
        width = int(texture.blueprint_get_size_x())
        height = int(texture.blueprint_get_size_y())
        if width != 128 or height != 128:
            raise RuntimeError(f"Semantic Icon size mismatch: {asset_path} actual={width}x{height}")
        if texture.get_editor_property("lod_group") != unreal.TextureGroup.TEXTUREGROUP_UI:
            raise RuntimeError(f"Semantic Icon TextureGroup must be UI: {asset_path}")
        if texture.get_editor_property("compression_settings") != unreal.TextureCompressionSettings.TC_EDITOR_ICON:
            raise RuntimeError(f"Semantic Icon compression must be UserInterface2D/TC_EDITOR_ICON: {asset_path}")
        readback[semantic_id] = {
            "path": asset_path,
            "class": texture.get_class().get_name(),
            "size": [width, height],
            "lod_group": str(texture.get_editor_property("lod_group")),
            "compression": str(texture.get_editor_property("compression_settings")),
            "srgb": bool(texture.get_editor_property("srgb")),
            "mip_gen_settings": str(texture.get_editor_property("mip_gen_settings")),
        }
    return readback


# [v1.1.0] Style DataAsset의 Font Hard Reference와 Semantic Icon 18개 Soft Reference를 검증합니다.
def validate_style_asset() -> dict[str, Any]:
    style_asset = require_asset_class(STYLE_ASSET_PATH, unreal.CFUIStyleData)
    font_assets = style_asset.get_editor_property("font_assets")
    ui_font_path = object_path(font_assets.get_editor_property("ui_font_asset"))
    numeric_font_path = object_path(font_assets.get_editor_property("numeric_font_asset"))
    if UI_FONT_PATH not in ui_font_path:
        raise RuntimeError(f"Style UI Font binding mismatch: {ui_font_path}")
    if NUMERIC_FONT_PATH not in numeric_font_path:
        raise RuntimeError(f"Style Numeric Font binding mismatch: {numeric_font_path}")

    expected_typeface_names = {
        "regular_typeface_name": "Regular",
        "medium_typeface_name": "Medium",
        "semi_bold_typeface_name": "SemiBold",
        "bold_typeface_name": "Bold",
    }
    for property_name, expected_name in expected_typeface_names.items():
        actual_name = str(font_assets.get_editor_property(property_name))
        if actual_name != expected_name:
            raise RuntimeError(
                f"Style Typeface Name mismatch: {property_name}={actual_name} expected={expected_name}"
            )

    icon_set = style_asset.get_editor_property("icon_set")
    semantic_entries = list(icon_set.get_editor_property("semantic_icons"))
    if len(semantic_entries) != len(SEMANTIC_ICON_SPECS):
        raise RuntimeError(
            f"Style Semantic Icon count mismatch: actual={len(semantic_entries)} expected={len(SEMANTIC_ICON_SPECS)}"
        )

    expected_icon_paths = {
        semantic_id: f"{ICON_ASSET_FOLDER}/{asset_name}"
        for semantic_id, asset_name, _ in SEMANTIC_ICON_SPECS
    }
    actual_icon_paths: dict[str, str] = {}
    for entry in semantic_entries:
        semantic_id = str(entry.get_editor_property("semantic_id"))
        if semantic_id in actual_icon_paths:
            raise RuntimeError(f"Duplicate Semantic Icon ID: {semantic_id}")
        icon_path = object_path(entry.get_editor_property("icon_asset"))
        actual_icon_paths[semantic_id] = icon_path

    if set(actual_icon_paths) != set(expected_icon_paths):
        raise RuntimeError(
            f"Semantic Icon ID set mismatch: actual={sorted(actual_icon_paths)} expected={sorted(expected_icon_paths)}"
        )
    for semantic_id, expected_path in expected_icon_paths.items():
        if expected_path not in actual_icon_paths[semantic_id]:
            raise RuntimeError(
                f"Semantic Icon reference mismatch: {semantic_id} actual={actual_icon_paths[semantic_id]} expected={expected_path}"
            )

    return {
        "path": STYLE_ASSET_PATH,
        "class": style_asset.get_class().get_name(),
        "ui_font": ui_font_path,
        "numeric_font": numeric_font_path,
        "style_name": str(icon_set.get_editor_property("style_name")),
        "semantic_icons": actual_icon_paths,
    }


# [v1.1.0] Density DataAsset 한 개의 Preset과 모든 승인 Token 값을 검증합니다.
def validate_density_asset(asset_path: str, preset_name: str) -> dict[str, Any]:
    expected = DENSITY_PRESET_VALUES[preset_name]
    density_asset = require_asset_class(asset_path, unreal.CFUIDensityData)
    preset = density_asset.get_editor_property("preset")
    if preset != getattr(unreal.CFUIDensityPreset, expected["preset"]):
        raise RuntimeError(f"Density Preset mismatch: {asset_path} actual={preset}")

    tokens = density_asset.get_editor_property("tokens")
    token_readback: dict[str, Any] = {}
    for property_name in [
        "panel_padding",
        "panel_inner_gap",
        "section_gap",
        "info_row_gap",
        "header_height",
        "info_row_height",
        "info_row_height_large",
        "chip_height",
        "icon_small",
        "icon_medium",
        "icon_large",
        "primary_value_scale",
        "secondary_text_scale",
        "outline_scale",
    ]:
        actual_value = float(tokens.get_editor_property(property_name))
        require_nearly_equal(
            f"Density {preset_name}.{property_name}",
            actual_value,
            expected[property_name],
        )
        token_readback[property_name] = actual_value

    caption_policy = tokens.get_editor_property("caption_policy")
    if caption_policy != getattr(unreal.CFUICaptionPolicy, expected["caption_policy"]):
        raise RuntimeError(
            f"Density CaptionPolicy mismatch: {asset_path} actual={caption_policy}"
        )
    token_readback["caption_policy"] = str(caption_policy)
    return {
        "path": asset_path,
        "class": density_asset.get_class().get_name(),
        "preset": str(preset),
        "tokens": token_readback,
    }


# [v1.1.0] 1080p Layout DataAsset의 Profile, Scale, Font Floor와 7 Slot D1-07 USER PASS 값을 검증합니다.
def validate_layout_asset() -> dict[str, Any]:
    layout_asset = require_asset_class(LAYOUT_1080_PATH, unreal.CFHUDLayoutData)
    if str(layout_asset.get_editor_property("profile_id")) != "HUD_1080_16":
        raise RuntimeError("1080 Layout ProfileId mismatch")
    if int(layout_asset.get_editor_property("profile_version")) != 1:
        raise RuntimeError("1080 Layout ProfileVersion mismatch")

    reference_viewport = vector2_tuple(layout_asset.get_editor_property("reference_viewport_size"))
    require_nearly_equal("Layout ReferenceViewport.X", reference_viewport[0], 1920.0)
    require_nearly_equal("Layout ReferenceViewport.Y", reference_viewport[1], 1080.0)
    require_nearly_equal("Layout GeometryScale", layout_asset.get_editor_property("geometry_scale"), 0.75)
    require_nearly_equal("Layout TypographyScale", layout_asset.get_editor_property("typography_scale"), 0.75)

    font_floors = layout_asset.get_editor_property("minimum_effective_font_sizes")
    floor_readback: dict[str, int] = {}
    for property_name, expected_value in EXPECTED_FONT_FLOORS.items():
        actual_value = int(font_floors.get_editor_property(property_name))
        if actual_value != expected_value:
            raise RuntimeError(
                f"Layout Font Floor mismatch: {property_name}={actual_value} expected={expected_value}"
            )
        floor_readback[property_name] = actual_value

    slot_layouts = list(layout_asset.get_editor_property("slot_layouts"))
    if len(slot_layouts) != 7:
        raise RuntimeError(f"1080 Layout must have exactly 7 slots: actual={len(slot_layouts)}")

    slot_readback: dict[str, Any] = {}
    seen_slots: set[str] = set()
    for slot_layout in slot_layouts:
        slot_id_value = slot_layout.get_editor_property("slot_id")
        slot_id = enum_member_name(slot_id_value)
        if slot_id in seen_slots:
            raise RuntimeError(f"Duplicate Layout Slot: {slot_id}")
        seen_slots.add(slot_id)
        if slot_id not in EXPECTED_LAYOUT_SLOTS:
            raise RuntimeError(f"Unexpected Layout Slot: {slot_id}")

        expected = EXPECTED_LAYOUT_SLOTS[slot_id]
        actual_anchor_min = vector2_tuple(slot_layout.get_editor_property("anchor_minimum"))
        actual_anchor_max = vector2_tuple(slot_layout.get_editor_property("anchor_maximum"))
        actual_alignment = vector2_tuple(slot_layout.get_editor_property("alignment"))
        actual_offset = vector2_tuple(slot_layout.get_editor_property("pixel_offset"))
        actual_size = vector2_tuple(slot_layout.get_editor_property("desired_size"))
        actual_z_order = int(slot_layout.get_editor_property("z_order"))
        actual_render_scale = float(slot_layout.get_editor_property("render_scale"))
        actual_visible = bool(slot_layout.get_editor_property("visible_by_default"))

        for label, actual_vector, expected_vector in [
            ("AnchorMinimum", actual_anchor_min, expected[0]),
            ("AnchorMaximum", actual_anchor_max, expected[1]),
            ("Alignment", actual_alignment, expected[2]),
            ("PixelOffset", actual_offset, expected[3]),
            ("DesiredSize", actual_size, expected[4]),
        ]:
            require_nearly_equal(f"{slot_id}.{label}.X", actual_vector[0], expected_vector[0])
            require_nearly_equal(f"{slot_id}.{label}.Y", actual_vector[1], expected_vector[1])
        if actual_z_order != expected[5]:
            raise RuntimeError(f"{slot_id}.ZOrder mismatch: actual={actual_z_order} expected={expected[5]}")
        require_nearly_equal(f"{slot_id}.RenderScale", actual_render_scale, 1.0)
        if not actual_visible:
            raise RuntimeError(f"{slot_id}.VisibleByDefault must be true")

        slot_readback[slot_id] = {
            "anchor_minimum": actual_anchor_min,
            "anchor_maximum": actual_anchor_max,
            "alignment": actual_alignment,
            "pixel_offset": actual_offset,
            "desired_size": actual_size,
            "z_order": actual_z_order,
        }

    if seen_slots != set(EXPECTED_LAYOUT_SLOTS):
        raise RuntimeError(
            f"Layout required slot set mismatch: actual={sorted(seen_slots)} expected={sorted(EXPECTED_LAYOUT_SLOTS)}"
        )

    return {
        "path": LAYOUT_1080_PATH,
        "class": layout_asset.get_class().get_name(),
        "profile_id": str(layout_asset.get_editor_property("profile_id")),
        "reference_viewport": reference_viewport,
        "geometry_scale": float(layout_asset.get_editor_property("geometry_scale")),
        "typography_scale": float(layout_asset.get_editor_property("typography_scale")),
        "font_floors": floor_readback,
        "slots": slot_readback,
    }


# [v1.1.0] 프로젝트에 보존된 Font Notice가 공식 원문 바이트와 정확히 일치하는지 검증합니다.
def validate_notice_files(official_license_sources: dict[str, bytes]) -> dict[str, Any]:
    expected_files = {
        "pretendard": (PRETENDARD_NOTICE_PATH, official_license_sources["pretendard"]),
        "ibm_plex": (IBM_PLEX_NOTICE_PATH, official_license_sources["ibm_plex"]),
    }
    readback: dict[str, Any] = {}
    for license_id, (notice_path, official_bytes) in expected_files.items():
        if not notice_path.is_file():
            raise RuntimeError(f"Font Notice file is missing: {notice_path}")
        actual_bytes = notice_path.read_bytes()
        if actual_bytes != official_bytes:
            raise RuntimeError(f"Font Notice differs from official source: {notice_path}")
        readback[license_id] = {
            "path": str(notice_path),
            "bytes": len(actual_bytes),
            "sha256": sha256_bytes(actual_bytes),
        }
    return readback


# [v1.1.0] Config CDO에서 Style·Density·Layout Soft Reference가 정확한 D1-09B Asset을 가리키는지 검증합니다.
def validate_config_contract() -> dict[str, Any]:
    subsystem_default = unreal.get_default_object(unreal.CFUISubsystem)
    expected_values = {
        "default_style_data_asset": STYLE_ASSET_PATH,
        "default_density_data_asset": DENSITY_STANDARD_PATH,
        "default_hud_layout_data_asset": LAYOUT_1080_PATH,
    }
    readback: dict[str, Any] = {}
    for property_name, expected_asset_path in expected_values.items():
        actual_value = subsystem_default.get_editor_property(property_name)
        actual_path = object_path(actual_value)
        if expected_asset_path not in actual_path:
            raise RuntimeError(
                f"CFUISubsystem Config binding mismatch: {property_name} actual={actual_path} expected={expected_asset_path}"
            )
        readback[property_name] = actual_path
    return readback


# [v1.1.0] 저장된 D1-09B Unreal Asset 전체를 읽어 Class·Reference·값 계약을 검증합니다.
def validate_all_assets() -> dict[str, Any]:
    forbidden_assets = capture_forbidden_assets()
    REPORT["forbidden_assets_found"] = forbidden_assets
    if forbidden_assets:
        raise RuntimeError(f"Forbidden deferred Layout Assets exist in D1-09B scope: {forbidden_assets}")

    return {
        "font_faces": validate_font_faces(),
        "runtime_fonts": validate_runtime_fonts(),
        "icons": validate_icon_textures(),
        "style": validate_style_asset(),
        "density_compact": validate_density_asset(DENSITY_COMPACT_PATH, "Compact"),
        "density_standard": validate_density_asset(DENSITY_STANDARD_PATH, "Standard"),
        "density_expanded": validate_density_asset(DENSITY_EXPANDED_PATH, "Expanded"),
        "layout_1080": validate_layout_asset(),
    }


# [v1.1.0] UE 5.8 Font·Texture·DataAsset Reflection과 Engine Font 직렬화를 읽기 전용으로 수집합니다.
def run_probe() -> None:
    type_names = [
        "Font",
        "FontFace",
        "FontFactory",
        "FontFileImportFactory",
        "CompositeFont",
        "AssetImportTask",
        "Texture2D",
        "TextureFactory",
        "DataAssetFactory",
        "CFUIStyleData",
        "CFUIDensityData",
        "CFHUDLayoutData",
        "CFUISubsystem",
        "CFUISemanticIconEntry",
    ]
    type_probe = {
        type_name: {"available": getattr(unreal, type_name, None) is not None}
        for type_name in type_names
    }
    enum_probe = {
        enum_name: sorted(
            name
            for name in dir(getattr(unreal, enum_name, object()))
            if name.isupper()
        )
        for enum_name in [
            "FontCacheType",
            "FontHinting",
            "FontLoadingPolicy",
            "FontLayoutMethod",
            "TextureCompressionSettings",
            "TextureGroup",
            "TextureMipGenSettings",
            "BatchCreateFontAsset",
        ]
    }
    engine_font = require_asset_class("/Engine/EngineFonts/Roboto", unreal.Font)
    REPORT["probe"] = {
        "types": type_probe,
        "enums": enum_probe,
        "engine_roboto_cache_type": str(engine_font.get_editor_property("font_cache_type")),
        "engine_roboto_composite": engine_font.get_editor_property("composite_font").export_text(),
        "composite_round_trip": validate_composite_text_contract(),
    }
    REPORT["success"] = True


# [v1.1.0] D1-09B Target 충돌·공식 Source·License·Composite 직렬화를 Content 변경 없이 사전 검증합니다.
def run_dry_run() -> None:
    REPORT["existing_targets_before"] = capture_existing_targets()
    REPORT["forbidden_assets_found"] = capture_forbidden_assets()
    if REPORT["existing_targets_before"]:
        raise RuntimeError(
            f"D1-09B target assets already exist before Apply; refusing to overwrite pre-existing assets: {REPORT['existing_targets_before']}"
        )
    if REPORT["forbidden_assets_found"]:
        raise RuntimeError(
            f"Forbidden deferred Layout Assets already exist: {REPORT['forbidden_assets_found']}"
        )

    prepare_all_sources()
    REPORT["composite_round_trip"] = validate_composite_text_contract()
    REPORT["dry_run_contract_passed"] = True
    REPORT["success"] = True


# [v1.1.0] D1-09B Unreal Asset과 Font Notice만 화이트리스트에 따라 생성·저장하고 Asset 사후조건을 검증합니다.
def run_apply() -> None:
    pretendard_sources, ibm_sources, icon_sources, license_sources = prepare_all_sources()

    write_notice_file(PRETENDARD_NOTICE_PATH, license_sources["pretendard"])
    write_notice_file(IBM_PLEX_NOTICE_PATH, license_sources["ibm_plex"])

    apply_font_faces(pretendard_sources, ibm_sources)
    icon_assets = apply_icon_textures(icon_sources)

    pretendard_face_paths = {
        weight: PRETENDARD_FACE_SPECS[weight]["asset_path"]
        for weight in FONT_WEIGHTS
    }
    ibm_face_paths = {
        weight: IBM_PLEX_FACE_SPECS[weight]["asset_path"]
        for weight in FONT_WEIGHTS
    }
    ui_font = create_or_configure_runtime_font(UI_FONT_PATH, pretendard_face_paths)
    numeric_font = create_or_configure_runtime_font(NUMERIC_FONT_PATH, ibm_face_paths)

    configure_style_asset(ui_font, numeric_font, icon_assets)
    configure_density_asset(DENSITY_COMPACT_PATH, "Compact")
    configure_density_asset(DENSITY_STANDARD_PATH, "Standard")
    configure_density_asset(DENSITY_EXPANDED_PATH, "Expanded")
    configure_layout_asset()

    REPORT["asset_readback"] = validate_all_assets()
    REPORT["notice_files"] = validate_notice_files(license_sources)
    REPORT["asset_contract_passed"] = True
    REPORT["license_contract_passed"] = True
    REPORT["success"] = True


# [v1.1.0] 저장된 Asset·공식 Notice·CFUISubsystem Config Soft Reference를 변경 없이 최종 검증합니다.
def run_readback() -> None:
    license_sources = prepare_license_sources()
    REPORT["asset_readback"] = validate_all_assets()
    REPORT["notice_files"] = validate_notice_files(license_sources)
    REPORT["config_readback"] = validate_config_contract()
    REPORT["asset_contract_passed"] = True
    REPORT["license_contract_passed"] = True
    REPORT["config_contract_passed"] = True
    REPORT["d1_09b_pass"] = True
    REPORT["success"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={RUN_MODE}")

    mode_handlers: dict[str, Callable[[], None]] = {
        "probe": run_probe,
        "dry_run": run_dry_run,
        "apply": run_apply,
        "readback": run_readback,
    }
    if RUN_MODE not in mode_handlers:
        raise RuntimeError(f"Unsupported UI Assetization mode: {RUN_MODE}")
    mode_handlers[RUN_MODE]()
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][UIAssetization] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("UI Assetization operation failed. Read report.json for details.")
