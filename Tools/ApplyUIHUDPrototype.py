# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.3.0
# Date: 2026-08-10
# Description: CF-FQ-032 D1-11 WBP_CFInGameHUD Probe/DryRun/Apply/Readback Unreal Python 도구
# Scope: /Game/CarFight/UI/HUD/WBP_CFInGameHUD 한 개만 생성 또는 Visual Fidelity Tree 재구축·컴파일·저장·정적 Readback합니다.
# Changelog:
# - v1.3.0: 세 번째 User Re-preview FAIL 이후 D1-07 Vehicle Armor 의미별 위치를 잠근 v1.3 Tree 재구축 증거와 네 번째 User Re-preview Pending 상태를 보고하도록 갱신.
# - v1.2.0: 두 번째 User Re-preview FAIL 이후 v1.2 Visual Fidelity Tree 재구축 증거와 후속 User Re-preview Pending 상태를 보고하도록 갱신.
# - v1.1.0: 사용자 Preview FAIL 수정 반영을 위해 기존 exact Target도 같은 Asset 안에서 Build Bridge로 재구축하도록 Apply 경로를 확장.
# - v1.0.0: D1-11 HUD Visual Prototype의 단일 Asset allowlist와 Editor Bridge 실행 경로를 최초 추가.
# Migration:
# - Probe/DryRun/Readback은 Content를 수정하지 않습니다.
# - Apply만 WBP_CFInGameHUD 한 개를 생성 또는 재구축할 수 있으며 D1-09B/D1-10 Asset은 읽기 전용 입력입니다.
# - D1-12, UI-P0-03, Runtime Event Binding과 Gameplay 조회를 생성하지 않습니다.

from __future__ import annotations

import json
import os
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.0.0] 구조화 보고서에서 식별할 현재 도구 버전입니다.
TOOL_VERSION = "1.3.0"

# [v1.0.0] Probe/DryRun/Apply/Readback 중 현재 실행 모드입니다.
RUN_MODE = os.environ.get("CARFIGHT_UI_HUD_MODE", "probe").strip().lower()

# [v1.0.0] 현재 Unreal 프로젝트 루트 절대 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] D1-11 구조화 결과 저장 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "UIHUDPrototype"

# [v1.0.0] D1-11 구조화 결과 JSON 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] D1-11에서 생성 가능한 유일한 Unreal Asset 폴더입니다.
HUD_FOLDER = "/Game/CarFight/UI/HUD"

# [v1.0.0] D1-11에서 생성 가능한 유일한 Widget Blueprint 경로입니다.
HUD_ASSET_PATH = "/Game/CarFight/UI/HUD/WBP_CFInGameHUD"

# [v1.0.0] D1-09B Style DataAsset 읽기 전용 경로입니다.
STYLE_ASSET_PATH = "/Game/CarFight/UI/Style/DA_CFUIStyle_Default"

# [v1.0.0] D1-09B Standard Density 읽기 전용 경로입니다.
DENSITY_STANDARD_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Standard"

# [v1.0.0] D1-09B Compact Density 읽기 전용 경로입니다.
DENSITY_COMPACT_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Compact"

# [v1.0.0] D1-07 승인 1920x1080 Layout 읽기 전용 경로입니다.
LAYOUT_1080_PATH = "/Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16"

# [v1.0.0] D1-10 Panel Base 읽기 전용 경로입니다.
PANEL_BASE_PATH = "/Game/CarFight/UI/Base/WBP_CFPanelBase"

# [v1.0.0] D1-10 Alert Base 읽기 전용 경로입니다.
ALERT_BASE_PATH = "/Game/CarFight/UI/Base/WBP_CFAlertItem"

# [v1.0.0] D1-10 InfoRow Base 읽기 전용 경로입니다.
INFO_ROW_BASE_PATH = "/Game/CarFight/UI/Base/WBP_CFInfoRow"

# [v1.0.0] Apply가 수정할 수 있는 정확한 단일 Asset allowlist입니다.
MUTABLE_ASSET_PATHS = {HUD_ASSET_PATH}

# [v1.0.0] D1-11 실행 결과와 Evidence를 보존할 구조화 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "carfight_ui_hud_prototype_v1",
    "tool_version": TOOL_VERSION,
    "mode": RUN_MODE,
    "success": False,
    "target_asset": HUD_ASSET_PATH,
    "mutable_assets": sorted(MUTABLE_ASSET_PATHS),
    "read_only_dependencies": [
        STYLE_ASSET_PATH,
        DENSITY_STANDARD_PATH,
        DENSITY_COMPACT_PATH,
        LAYOUT_1080_PATH,
        PANEL_BASE_PATH,
        ALERT_BASE_PATH,
        INFO_ROW_BASE_PATH,
    ],
    "created_assets": [],
        "reused_assets": [],
    "rebuilt_assets": [],
    "compiled_assets": [],
    "saved_assets": [],
    "probe": {},
    "dry_run": {},
    "readback": {},
    "contracts": {},
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 D1-11 도구 메시지를 남깁니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][UIHUDPrototype] {message}")


# [v1.0.0] 이름으로 Unreal Python 타입을 가져오고 누락 시 즉시 실패합니다.
def require_unreal_type(type_name: str) -> Any:
    # [v1.0.0] 현재 이름으로 Python에 실제 노출된 Unreal 타입입니다.
    unreal_type = getattr(unreal, type_name, None)
    if unreal_type is None:
        raise RuntimeError(f"Required Unreal Python type is unavailable: {type_name}")
    return unreal_type


# [v1.0.0] 지정 Unreal Asset을 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    # [v1.0.0] Editor Asset Library가 실제로 로드한 Asset입니다.
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required D1-11 dependency is missing: {asset_path}")
    return asset


# [v1.0.0] D1-11 C++ Editor Bridge를 반환하고 필수 Bool-only 메서드 노출을 확인합니다.
def require_bridge() -> Any:
    # [v1.0.0] Python에 노출된 D1-11 HUD Editor Bridge 타입입니다.
    bridge_type = require_unreal_type("CFUIHUDEditorBridge")
    for method_name in ("build_hud_prototype_result", "validate_hud_prototype_result"):
        if not callable(getattr(bridge_type, method_name, None)):
            raise RuntimeError(f"CFUIHUDEditorBridge method is unavailable: {method_name}")
    return bridge_type


# [v1.0.0] D1-11 Bridge에 전달할 모든 읽기 전용 Data/Base Widget Asset을 로드합니다.
def load_dependencies() -> dict[str, Any]:
    return {
        "layout": require_asset(LAYOUT_1080_PATH),
        "style": require_asset(STYLE_ASSET_PATH),
        "standard_density": require_asset(DENSITY_STANDARD_PATH),
        "compact_density": require_asset(DENSITY_COMPACT_PATH),
        "panel": require_asset(PANEL_BASE_PATH),
        "alert": require_asset(ALERT_BASE_PATH),
        "info_row": require_asset(INFO_ROW_BASE_PATH),
    }


# [v1.0.0] D1-11 Bridge Build 또는 Validate 메서드를 공통 인수로 호출합니다.
def call_bridge(method_name: str, blueprint_asset: Any, dependencies: dict[str, Any]) -> bool:
    # [v1.0.0] 실제 호출할 bool-only D1-11 C++ Bridge 함수입니다.
    bridge_method = getattr(require_bridge(), method_name)
    # [v1.0.0] UE 5.8 Python에서 직접 반환된 Bool 결과입니다.
    result = bridge_method(
        blueprint_asset,
        dependencies["layout"],
        dependencies["style"],
        dependencies["standard_density"],
        dependencies["compact_density"],
        dependencies["panel"],
        dependencies["alert"],
        dependencies["info_row"],
    )
    return bool(result)


# [v1.0.0] Blueprint의 현재 Native Parent Class를 UE 5.8 공식 Editor Library로 읽습니다.
def get_blueprint_parent_class(blueprint_asset: Any) -> Any:
    # [v1.0.0] Blueprint Parent Class 읽기에 사용할 UE 5.8 Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    # [v1.0.0] 현재 Blueprint가 실제로 상속하는 Parent Class입니다.
    parent_class = blueprint_editor_library.get_blueprint_parent_class(blueprint_asset)
    if parent_class is None:
        raise RuntimeError(f"Blueprint parent class is unavailable: {HUD_ASSET_PATH}")
    return parent_class


# [v1.0.0] Blueprint Generated Class Object Path를 읽고 누락 시 빈 문자열을 반환합니다.
def read_generated_class_path(blueprint_asset: Any) -> str:
    try:
        # [v1.0.0] 현재 Blueprint Compile 결과 Generated Class입니다.
        generated_class = blueprint_asset.generated_class()
        return generated_class.get_path_name() if generated_class is not None else ""
    except Exception:
        return ""


# [v1.0.0] D1-11 Blueprint를 변경 없이 Bridge와 Parent 계약으로 검증합니다.
def validate_hud_asset(blueprint_asset: Any, dependencies: dict[str, Any]) -> dict[str, Any]:
    # [v1.0.0] WBP_CFInGameHUD가 정확히 상속해야 하는 Native Parent UClass입니다.
    expected_parent_class = require_unreal_type("CFStyledWidgetBase").static_class()
    # [v1.0.0] 실제 WBP_CFInGameHUD Native Parent입니다.
    actual_parent_class = get_blueprint_parent_class(blueprint_asset)
    # [v1.0.0] C++ Bridge가 정확한 Designer Tree와 정적 안전 계약을 검증한 결과입니다.
    bridge_valid = call_bridge("validate_hud_prototype_result", blueprint_asset, dependencies)
    return {
        "asset_path": HUD_ASSET_PATH,
        "parent_valid": actual_parent_class == expected_parent_class,
        "expected_parent": expected_parent_class.get_path_name(),
        "actual_parent": actual_parent_class.get_path_name(),
        "bridge_valid": bridge_valid,
        "generated_class": read_generated_class_path(blueprint_asset),
    }


# [v1.0.0] Probe에서 D1-11 필수 Python/C++ 타입과 모든 읽기 전용 Dependency 존재 여부를 수집합니다.
def run_probe() -> None:
    # [v1.0.0] D1-11 실행에 반드시 필요한 Unreal Python 타입 이름입니다.
    required_type_names = [
        "WidgetBlueprint",
        "WidgetBlueprintFactory",
        "BlueprintEditorLibrary",
        "CFStyledWidgetBase",
        "CFUIHUDEditorBridge",
    ]
    # [v1.0.0] 각 필수 타입의 실제 노출 여부입니다.
    type_state = {
        type_name: getattr(unreal, type_name, None) is not None
        for type_name in required_type_names
    }
    # [v1.0.0] 모든 D1-09B/D1-10 읽기 전용 Dependency의 실제 존재 여부입니다.
    dependency_state = {
        asset_path: unreal.EditorAssetLibrary.does_asset_exist(asset_path)
        for asset_path in REPORT["read_only_dependencies"]
    }
    require_bridge()
    REPORT["probe"] = {
        "types": type_state,
        "dependencies": dependency_state,
        "target_exists": unreal.EditorAssetLibrary.does_asset_exist(HUD_ASSET_PATH),
    }
    if not all(type_state.values()) or not all(dependency_state.values()):
        raise RuntimeError("D1-11 Probe found a missing required type or read-only dependency")
    REPORT["success"] = True


# [v1.0.0] Content 변경 없이 단일 Target과 D1-09B/D1-10 Dependency 보호 상태를 검증합니다.
def run_dry_run() -> None:
    # [v1.0.0] D1-11이 읽기만 할 실제 Dependency 객체들입니다.
    dependencies = load_dependencies()
    # [v1.0.0] WBP_CFInGameHUD의 현재 존재 여부입니다.
    target_exists = unreal.EditorAssetLibrary.does_asset_exist(HUD_ASSET_PATH)
        # [v1.1.0] 기존 Target이 있을 경우 재구축 전에 정확한 Native Parent 보호 조건만 확인한 결과입니다.
    existing_validation = None
    if target_exists:
        existing_asset = require_asset(HUD_ASSET_PATH)
        expected_parent_class = require_unreal_type("CFStyledWidgetBase").static_class()
        actual_parent_class = get_blueprint_parent_class(existing_asset)
        existing_validation = {
            "parent_valid": actual_parent_class == expected_parent_class,
            "expected_parent": expected_parent_class.get_path_name(),
            "actual_parent": actual_parent_class.get_path_name(),
        }
        if not existing_validation["parent_valid"]:
            raise RuntimeError("Existing WBP_CFInGameHUD parent differs from the D1-11 protected target contract")

    REPORT["dry_run"] = {
        "mutable_asset_count": len(MUTABLE_ASSET_PATHS),
        "mutable_assets": sorted(MUTABLE_ASSET_PATHS),
        "target_exists": target_exists,
        "action": "rebuild_exact" if target_exists else "create",
        "existing_validation": existing_validation,
        "d1_12_started": False,
        "ui_p0_03_started": False,
    }
    REPORT["success"] = True


# [v1.0.0] Apply 모드에서만 HUD 폴더를 생성합니다.
def ensure_hud_folder() -> None:
    if unreal.EditorAssetLibrary.does_directory_exist(HUD_FOLDER):
        return
    if RUN_MODE != "apply":
        raise RuntimeError("HUD folder creation is allowed only in apply mode")
    if not unreal.EditorAssetLibrary.make_directory(HUD_FOLDER):
        raise RuntimeError(f"HUD folder creation failed: {HUD_FOLDER}")


# [v1.0.0] 정확한 CFStyledWidgetBase Parent로 WBP_CFInGameHUD 한 개를 생성합니다.
def create_hud_blueprint() -> Any:
    if RUN_MODE != "apply":
        raise RuntimeError("WBP_CFInGameHUD creation is allowed only in apply mode")
    if HUD_ASSET_PATH not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"D1-11 target is outside mutation allowlist: {HUD_ASSET_PATH}")

    ensure_hud_folder()
    # [v1.0.0] WBP_CFInGameHUD가 정확히 상속할 Native Parent UClass입니다.
    parent_class = require_unreal_type("CFStyledWidgetBase").static_class()
    # [v1.0.0] UE 5.8 Widget Blueprint 생성 Factory입니다.
    widget_factory = require_unreal_type("WidgetBlueprintFactory")()
    widget_factory.set_editor_property("parent_class", parent_class)
    # [v1.0.0] AssetTools가 생성한 실제 WBP_CFInGameHUD입니다.
    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "WBP_CFInGameHUD",
        HUD_FOLDER,
        require_unreal_type("WidgetBlueprint"),
        widget_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"Widget Blueprint creation failed: {HUD_ASSET_PATH}")
    REPORT["created_assets"].append(HUD_ASSET_PATH)
    return created_asset


# [v1.0.0] WBP_CFInGameHUD를 공식 Editor Library로 Compile하고 Generated Class를 확인합니다.
def compile_hud_blueprint(blueprint_asset: Any) -> None:
    # [v1.0.0] Blueprint Compile에 사용할 UE 5.8 Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    blueprint_editor_library.compile_blueprint(blueprint_asset)
    if not read_generated_class_path(blueprint_asset):
        raise RuntimeError("WBP_CFInGameHUD generated class is missing after compile")
    REPORT["compiled_assets"].append(HUD_ASSET_PATH)


# [v1.0.0] D1-11 단일 allowlist를 확인한 뒤 WBP_CFInGameHUD를 저장합니다.
def save_hud_blueprint(blueprint_asset: Any) -> None:
    if RUN_MODE != "apply" or HUD_ASSET_PATH not in MUTABLE_ASSET_PATHS:
        raise RuntimeError("WBP_CFInGameHUD save is outside D1-11 apply contract")
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint_asset, only_if_is_dirty=False):
        raise RuntimeError(f"Widget Blueprint save failed: {HUD_ASSET_PATH}")
    REPORT["saved_assets"].append(HUD_ASSET_PATH)


# [v1.1.0] WBP_CFInGameHUD 한 개만 생성 또는 Visual Fidelity Tree 재구축·Compile·Validate·Save·Readback합니다.
def run_apply() -> None:
    # [v1.1.0] D1-11 Build와 검증에 사용할 읽기 전용 Dependency 객체입니다.
    dependencies = load_dependencies()
    # [v1.1.0] 기존 Target이 있으면 같은 Asset을 재사용하고 Parent 보호 조건만 확인합니다.
    if unreal.EditorAssetLibrary.does_asset_exist(HUD_ASSET_PATH):
        blueprint_asset = require_asset(HUD_ASSET_PATH)
        expected_parent_class = require_unreal_type("CFStyledWidgetBase").static_class()
        actual_parent_class = get_blueprint_parent_class(blueprint_asset)
        if actual_parent_class != expected_parent_class:
            raise RuntimeError("Existing WBP_CFInGameHUD is protected because its Native Parent differs from D1-11")
        REPORT["reused_assets"].append(HUD_ASSET_PATH)
    else:
        blueprint_asset = create_hud_blueprint()

    if not call_bridge("build_hud_prototype_result", blueprint_asset, dependencies):
        raise RuntimeError("CFUIHUDEditorBridge failed to build/rebuild D1-11 HUD Visual Fidelity Prototype")
    REPORT["rebuilt_assets"].append(HUD_ASSET_PATH)

    compile_hud_blueprint(blueprint_asset)
    validation = validate_hud_asset(blueprint_asset, dependencies)
    if not validation["parent_valid"] or not validation["bridge_valid"]:
        raise RuntimeError("D1-11 post-compile HUD Prototype validation failed")
    save_hud_blueprint(blueprint_asset)
    REPORT["readback"] = validation
    finalize_contracts(validation)
    REPORT["success"] = True


# [v1.0.0] 저장된 WBP_CFInGameHUD를 수정 없이 새 프로세스에서 다시 검증합니다.
def run_readback() -> None:
    # [v1.0.0] D1-11 Readback에 사용할 읽기 전용 Dependency 객체입니다.
    dependencies = load_dependencies()
    if not unreal.EditorAssetLibrary.does_asset_exist(HUD_ASSET_PATH):
        raise RuntimeError(f"D1-11 target is missing: {HUD_ASSET_PATH}")
    # [v1.0.0] 저장된 WBP_CFInGameHUD의 실제 정적 검증 결과입니다.
    validation = validate_hud_asset(require_asset(HUD_ASSET_PATH), dependencies)
    if not validation["parent_valid"] or not validation["bridge_valid"]:
        raise RuntimeError("Saved WBP_CFInGameHUD failed D1-11 readback")
    REPORT["readback"] = validation
    finalize_contracts(validation)
    REPORT["success"] = True


# [v1.0.0] D1-11 자동 검증과 사용자 Preview Gate를 명확히 분리한 계약 결과를 기록합니다.
def finalize_contracts(validation: dict[str, Any]) -> None:
    REPORT["contracts"] = {
        "exact_single_target_asset": True,
        "native_parent_pass": bool(validation["parent_valid"]),
                "static_tree_layout_mock_graph_contract_pass": bool(validation["bridge_valid"]),
        "visual_fidelity_contract_pass": bool(validation["bridge_valid"]),
        "gameplay_cast_count": 0 if validation["bridge_valid"] else None,
        "runtime_event_binding_count": 0 if validation["bridge_valid"] else None,
        "ui_p0_03_view_data_count": 0,
        "d1_12_asset_count": 0,
        "preview_1920x1080_user_gate": "Pending",
        "d1_11_pass": False,
                "note": "D1-11 최종 PASS는 현재 InGameUIAssetizationSpec의 User Preview Gate에 따라 v1.3 Vehicle Semantic Visual Fix를 네 번째 1920x1080 Designer Preview에서 재확인한 뒤에만 승격합니다.",
    }


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={RUN_MODE}")
    if RUN_MODE == "probe":
        run_probe()
    elif RUN_MODE == "dry_run":
        run_dry_run()
    elif RUN_MODE == "apply":
        run_apply()
    elif RUN_MODE == "readback":
        run_readback()
    else:
        raise RuntimeError(f"Unsupported D1-11 run mode: {RUN_MODE}")
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][UIHUDPrototype] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("UI HUD Prototype operation failed. Read report.json for details.")
