# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.2.0
# Date: 2026-08-18
# Description: CF-FQ-032 D1-11 Production HUD + UI-P0-06 WeaponPanel Targeted Apply Unreal Python 도구
# Scope: 기존 전체 Production Probe/DryRun/Apply/Readback을 보존하고, UI-P0-06 Stage B에서는 WBP_CFWeaponPanel 정확히 1개만 재구축·Compile·Validate·Save할 수 있습니다.
# Changelog:
# - v1.2.0: `weapon_panel_apply` targeted mode를 추가해 기존 Production 전체 10 Asset 재작성 없이 WBP_CFWeaponPanel 하나만 Bridge Build→Compile→Validate→Save하도록 제한.
# - v1.1.0: 사용자 결정에 맞춰 D1-11을 Structure-first Gate로 판정하고, 미술 승인/세부 배치를 별도 D1-11-ART Pending 상태로 분리.
# - v1.0.0: Production 의미 단위 Widget 9개와 DA_CFHUDVisual_Default의 exact allowlist, 순차 Compile/Root Composition/Readback을 최초 추가.
# Migration:
# - 기존 ApplyUIHUDPrototype.py v1.x는 Historical Border Mock 재현용으로 보존합니다.
# - Production Apply는 D1-09B Style/Density/Layout/Icon Asset을 읽기 전용으로 사용합니다.
# - D1-11 Structure PASS는 Root/Panel/Element/VisualData exact 구조와 Validator 계약을 의미하며 최종 HUD Art 승인과 픽셀 폴리시는 D1-11-ART에서 별도 판정합니다.
# - D1-12, UI-P0-03, Runtime Event Binding과 Gameplay 조회를 생성하지 않습니다.
# - v1.2.0 `weapon_panel_apply`는 기존 Asset 생성을 허용하지 않고 정확한 WBP_CFWeaponPanel이 이미 존재해야 하며, 다른 Production Asset은 읽기 전용 Dependency로만 사용합니다.

from __future__ import annotations

import json
import os
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.1.0] 구조화 보고서에서 식별할 현재 Production 도구 버전입니다.
TOOL_VERSION = "1.2.0"

# [v1.0.0] Probe/DryRun/Apply/Readback 중 현재 실행 모드입니다.
RUN_MODE = os.environ.get("CARFIGHT_UI_HUD_PROD_MODE", "probe").strip().lower()

# [v1.0.0] 현재 Unreal 프로젝트 루트 절대 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] Production HUD 구조화 결과 저장 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "UIHUDProduction"

# [v1.0.0] Production HUD 구조화 결과 JSON 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] 기존 D1-09B Style DataAsset 읽기 전용 경로입니다.
STYLE_ASSET_PATH = "/Game/CarFight/UI/Style/DA_CFUIStyle_Default"

# [v1.0.0] 기존 Standard Density DataAsset 읽기 전용 경로입니다.
DENSITY_STANDARD_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Standard"

# [v1.0.0] 기존 Compact Density DataAsset 읽기 전용 경로입니다.
DENSITY_COMPACT_PATH = "/Game/CarFight/UI/Density/DA_CFUIDensity_Compact"

# [v1.0.0] D1-07 승인 1920x1080 Layout 읽기 전용 경로입니다.
LAYOUT_1080_PATH = "/Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16"

# [v1.0.0] Production HUD Root Widget Blueprint 경로입니다.
ROOT_ASSET_PATH = "/Game/CarFight/UI/HUD/WBP_CFInGameHUD"

# [v1.0.0] Production HUD Panel Asset 경로와 Bridge Role의 안정적인 매핑입니다.
PANEL_ASSETS: dict[str, str] = {
    "MissionPanel": "/Game/CarFight/UI/HUD/Panels/WBP_CFMissionPanel",
    "AlertFeed": "/Game/CarFight/UI/HUD/Panels/WBP_CFAlertFeed",
    "TargetPanel": "/Game/CarFight/UI/HUD/Panels/WBP_CFTargetPanel",
    "VehiclePanel": "/Game/CarFight/UI/HUD/Panels/WBP_CFVehiclePanel",
    "RadarPanel": "/Game/CarFight/UI/HUD/Panels/WBP_CFRadarPanel",
    "WeaponPanel": "/Game/CarFight/UI/HUD/Panels/WBP_CFWeaponPanel",
}

# [v1.0.0] Production HUD Element Asset 경로와 Bridge Role의 안정적인 매핑입니다.
ELEMENT_ASSETS: dict[str, str] = {
    "SpeedGauge": "/Game/CarFight/UI/HUD/Elements/WBP_CFSpeedGauge",
    "ArmorBodyMap": "/Game/CarFight/UI/HUD/Elements/WBP_CFArmorBodyMap",
}

# [v1.0.0] Production HUD 전용 교체 가능 Visual DataAsset 경로입니다.
VISUAL_ASSET_PATH = "/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default"

# [v1.0.0] Production Apply가 생성 또는 재구축할 수 있는 정확한 10개 Asset allowlist입니다.
MUTABLE_ASSET_PATHS = {
    ROOT_ASSET_PATH,
    VISUAL_ASSET_PATH,
    *PANEL_ASSETS.values(),
    *ELEMENT_ASSETS.values(),
}

# [v1.0.0] Production 실행 결과와 Evidence를 보존할 구조화 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "carfight_ui_hud_production_v1",
    "tool_version": TOOL_VERSION,
    "mode": RUN_MODE,
    "success": False,
    "mutable_assets": sorted(MUTABLE_ASSET_PATHS),
    "read_only_dependencies": [
        STYLE_ASSET_PATH,
        DENSITY_STANDARD_PATH,
        DENSITY_COMPACT_PATH,
        LAYOUT_1080_PATH,
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


# [v1.0.0] Unreal Output Log에 Production HUD 도구 메시지를 남깁니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][UIHUDProduction] {message}")


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
        raise RuntimeError(f"Required Production HUD asset is missing: {asset_path}")
    return asset


# [v1.0.0] Production C++ Editor Bridge와 필수 bool-only 메서드 노출을 확인합니다.
def require_bridge() -> Any:
    # [v1.0.0] Python에 노출된 Production HUD Editor Bridge 타입입니다.
    bridge_type = require_unreal_type("CFUIHUDProdEditorBridge")
    for method_name in (
        "build_production_widget_result",
        "validate_production_widget_result",
        "build_production_root_result",
        "validate_production_root_result",
    ):
        if not callable(getattr(bridge_type, method_name, None)):
            raise RuntimeError(f"CFUIHUDProdEditorBridge method is unavailable: {method_name}")
    return bridge_type


# [v1.0.0] Production Build/Validate에 사용할 기존 읽기 전용 DataAsset을 로드합니다.
def load_read_only_dependencies() -> dict[str, Any]:
    return {
        "layout": require_asset(LAYOUT_1080_PATH),
        "style": require_asset(STYLE_ASSET_PATH),
        "standard_density": require_asset(DENSITY_STANDARD_PATH),
        "compact_density": require_asset(DENSITY_COMPACT_PATH),
    }


# [v1.0.0] Blueprint의 현재 Native Parent Class를 공식 Editor Library로 읽습니다.
def get_blueprint_parent_class(blueprint_asset: Any) -> Any:
    # [v1.0.0] Blueprint Parent Class 읽기에 사용할 UE 5.8 Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    # [v1.0.0] 현재 Blueprint가 실제로 상속하는 Parent Class입니다.
    parent_class = blueprint_editor_library.get_blueprint_parent_class(blueprint_asset)
    if parent_class is None:
        raise RuntimeError(f"Blueprint parent class is unavailable: {blueprint_asset.get_path_name()}")
    return parent_class


# [v1.0.0] 모든 Production Widget Blueprint가 정확한 CFStyledWidgetBase Native Parent인지 검증합니다.
def validate_widget_parent(blueprint_asset: Any) -> None:
    # [v1.0.0] Production Widget이 정확히 상속해야 하는 Native Parent UClass입니다.
    expected_parent_class = require_unreal_type("CFStyledWidgetBase").static_class()
    # [v1.0.0] 현재 Widget Blueprint의 실제 Native Parent UClass입니다.
    actual_parent_class = get_blueprint_parent_class(blueprint_asset)
    if actual_parent_class != expected_parent_class:
        raise RuntimeError(
            f"Production Widget parent mismatch: asset={blueprint_asset.get_path_name()} "
            f"expected={expected_parent_class.get_path_name()} actual={actual_parent_class.get_path_name()}"
        )


# [v1.0.0] Widget Blueprint Generated Class Object Path를 읽고 누락 시 빈 문자열을 반환합니다.
def read_generated_class_path(blueprint_asset: Any) -> str:
    try:
        # [v1.0.0] 현재 Blueprint Compile 결과 Generated Class입니다.
        generated_class = blueprint_asset.generated_class()
        return generated_class.get_path_name() if generated_class is not None else ""
    except Exception:
        return ""


# [v1.0.0] 정확한 CFStyledWidgetBase Parent로 지정 Widget Blueprint를 생성하거나 기존 Asset을 보호 재사용합니다.
def get_or_create_widget(asset_path: str) -> Any:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Production Widget is outside mutation allowlist: {asset_path}")
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        # [v1.0.0] 기존 정확한 Target Asset입니다.
        existing_asset = require_asset(asset_path)
        validate_widget_parent(existing_asset)
        REPORT["reused_assets"].append(asset_path)
        return existing_asset
    if RUN_MODE != "apply":
        raise RuntimeError(f"Production Widget creation is allowed only in apply mode: {asset_path}")

    # [v1.0.0] 신규 Widget Blueprint를 저장할 Content 폴더입니다.
    folder_path, asset_name = asset_path.rsplit("/", 1)
    unreal.EditorAssetLibrary.make_directory(folder_path)
    # [v1.0.0] 신규 Widget Blueprint의 정확한 Native Parent입니다.
    parent_class = require_unreal_type("CFStyledWidgetBase").static_class()
    # [v1.0.0] UE 5.8 Widget Blueprint 생성 Factory입니다.
    widget_factory = require_unreal_type("WidgetBlueprintFactory")()
    widget_factory.set_editor_property("parent_class", parent_class)
    # [v1.0.0] AssetTools가 생성한 실제 Production Widget Blueprint입니다.
    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        folder_path,
        require_unreal_type("WidgetBlueprint"),
        widget_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"Production Widget Blueprint creation failed: {asset_path}")
    REPORT["created_assets"].append(asset_path)
    return created_asset


# [v1.0.0] HUD Visual DataAsset을 생성하거나 기존 정확한 Asset을 재사용합니다.
def get_or_create_visual_data() -> Any:
    if unreal.EditorAssetLibrary.does_asset_exist(VISUAL_ASSET_PATH):
        # [v1.0.0] 기존 HUD Visual DataAsset입니다.
        visual_asset = require_asset(VISUAL_ASSET_PATH)
        if not isinstance(visual_asset, require_unreal_type("CFHUDVisualData")):
            raise RuntimeError(f"Existing HUD Visual asset has unexpected class: {VISUAL_ASSET_PATH}")
        REPORT["reused_assets"].append(VISUAL_ASSET_PATH)
        return visual_asset
    if RUN_MODE != "apply":
        raise RuntimeError("HUD Visual DataAsset creation is allowed only in apply mode")

    # [v1.0.0] 신규 HUD Visual DataAsset을 저장할 Content 폴더입니다.
    folder_path, asset_name = VISUAL_ASSET_PATH.rsplit("/", 1)
    unreal.EditorAssetLibrary.make_directory(folder_path)
    # [v1.0.0] UCFHUDVisualData 전용 DataAsset Factory입니다.
    data_asset_factory = require_unreal_type("DataAssetFactory")()
    data_asset_factory.set_editor_property("data_asset_class", require_unreal_type("CFHUDVisualData").static_class())
    # [v1.0.0] AssetTools가 생성한 실제 HUD Visual DataAsset입니다.
    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        folder_path,
        require_unreal_type("CFHUDVisualData"),
        data_asset_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"HUD Visual DataAsset creation failed: {VISUAL_ASSET_PATH}")
    REPORT["created_assets"].append(VISUAL_ASSET_PATH)
    return created_asset


# [v1.0.0] Widget Blueprint를 공식 Editor Library로 Compile하고 Generated Class를 확인합니다.
def compile_widget(asset_path: str, blueprint_asset: Any) -> None:
    # [v1.0.0] Blueprint Compile에 사용할 UE 5.8 Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    blueprint_editor_library.compile_blueprint(blueprint_asset)
    if not read_generated_class_path(blueprint_asset):
        raise RuntimeError(f"Production Widget generated class is missing after compile: {asset_path}")
    REPORT["compiled_assets"].append(asset_path)


# [v1.2.0] 현재 실행 모드의 exact mutation allowlist 안에 있는 Production Asset 한 개만 저장합니다.
def save_asset(asset_path: str, asset: Any) -> None:
    # [v1.2.0] 전체 Apply는 기존 10 Asset, WeaponPanel-only Apply는 정확히 WeaponPanel 하나만 허용하는 mode-scoped 저장 allowlist입니다.
    allowed_asset_paths = (
        MUTABLE_ASSET_PATHS
        if RUN_MODE == "apply"
        else {PANEL_ASSETS["WeaponPanel"]}
        if RUN_MODE == "weapon_panel_apply"
        else set()
    )
    if asset_path not in allowed_asset_paths:
        raise RuntimeError(f"Production HUD save is outside apply contract: mode={RUN_MODE} asset={asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Production HUD asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.0.0] Production Child 역할 Build/Validate C++ Bridge를 공통 인수로 호출합니다.
def call_widget_bridge(
    method_name: str,
    blueprint_asset: Any,
    role: str,
    dependencies: dict[str, Any],
    visual_data: Any,
    speed_gauge_asset: Any | None,
    armor_body_map_asset: Any | None,
) -> bool:
    # [v1.0.0] 실제 호출할 Production Child bool-only C++ Bridge 메서드입니다.
    bridge_method = getattr(require_bridge(), method_name)
    # [v1.0.0] UE 5.8 Python에서 직접 반환되는 Build/Validate Bool 결과입니다.
    result = bridge_method(
        blueprint_asset,
        role,
        dependencies["layout"],
        dependencies["style"],
        dependencies["standard_density"],
        dependencies["compact_density"],
        visual_data,
        speed_gauge_asset,
        armor_body_map_asset,
    )
    return bool(result)


# [v1.0.0] Production Root Build/Validate C++ Bridge를 여섯 Panel Asset과 함께 호출합니다.
def call_root_bridge(method_name: str, root_asset: Any, dependencies: dict[str, Any], panels: dict[str, Any]) -> bool:
    # [v1.0.0] 실제 호출할 Production Root bool-only C++ Bridge 메서드입니다.
    bridge_method = getattr(require_bridge(), method_name)
    return bool(
        bridge_method(
            root_asset,
            dependencies["layout"],
            panels["MissionPanel"],
            panels["AlertFeed"],
            panels["TargetPanel"],
            panels["VehiclePanel"],
            panels["RadarPanel"],
            panels["WeaponPanel"],
        )
    )


# [v1.0.0] Probe에서 필수 Python/C++ 타입과 읽기 전용 Dependency 존재 여부를 수집합니다.
def run_probe() -> None:
    # [v1.0.0] Production 실행에 반드시 필요한 Unreal Python 타입 이름입니다.
    required_type_names = [
        "WidgetBlueprint",
        "WidgetBlueprintFactory",
        "BlueprintEditorLibrary",
        "DataAssetFactory",
        "CFStyledWidgetBase",
        "CFHUDVisualData",
        "CFUIHUDProdEditorBridge",
    ]
    # [v1.0.0] 각 필수 타입의 실제 Python 노출 여부입니다.
    type_state = {type_name: getattr(unreal, type_name, None) is not None for type_name in required_type_names}
    # [v1.0.0] D1-09B/D1-07 읽기 전용 Dependency의 실제 존재 여부입니다.
    dependency_state = {
        asset_path: unreal.EditorAssetLibrary.does_asset_exist(asset_path)
        for asset_path in REPORT["read_only_dependencies"]
    }
    require_bridge()
    REPORT["probe"] = {
        "types": type_state,
        "dependencies": dependency_state,
        "current_mutable_asset_state": {
            asset_path: unreal.EditorAssetLibrary.does_asset_exist(asset_path)
            for asset_path in sorted(MUTABLE_ASSET_PATHS)
        },
    }
    if not all(type_state.values()) or not all(dependency_state.values()):
        raise RuntimeError("Production HUD Probe found a missing required type or read-only dependency")
    REPORT["success"] = True


# [v1.0.0] Content 변경 없이 정확한 10개 Target allowlist와 기존 Parent 보호 상태를 검증합니다.
def run_dry_run() -> None:
    load_read_only_dependencies()
    # [v1.0.0] 기존 Widget Target의 Native Parent 보호 결과입니다.
    existing_widget_validation: dict[str, Any] = {}
    for asset_path in [ROOT_ASSET_PATH, *ELEMENT_ASSETS.values(), *PANEL_ASSETS.values()]:
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            asset = require_asset(asset_path)
            validate_widget_parent(asset)
            existing_widget_validation[asset_path] = "CFStyledWidgetBase"
    REPORT["dry_run"] = {
        "mutable_asset_count": len(MUTABLE_ASSET_PATHS),
        "mutable_assets": sorted(MUTABLE_ASSET_PATHS),
        "existing_widget_parent_validation": existing_widget_validation,
        "actions": {
            asset_path: "rebuild_exact" if unreal.EditorAssetLibrary.does_asset_exist(asset_path) else "create"
            for asset_path in sorted(MUTABLE_ASSET_PATHS)
        },
        "legacy_mock_bridge_mutated": False,
        "d1_12_started": False,
        "ui_p0_03_started": False,
    }
    REPORT["success"] = True


# [v1.0.0] Production 10개 Asset을 의존 순서대로 생성·재구축·Compile·Validate·Save합니다.
def run_apply() -> None:
    # [v1.0.0] Production Tree Build에 사용할 기존 읽기 전용 DataAsset들입니다.
    dependencies = load_read_only_dependencies()
    # [v1.0.0] 실제 HUD 전용 아트 연결을 보유할 신규/기존 Visual DataAsset입니다.
    visual_data = get_or_create_visual_data()
    save_asset(VISUAL_ASSET_PATH, visual_data)

    # [v1.0.0] 다른 Widget이 의존하지 않는 두 의미 Element Asset입니다.
    elements: dict[str, Any] = {}
    for role, asset_path in ELEMENT_ASSETS.items():
        blueprint_asset = get_or_create_widget(asset_path)
        if not call_widget_bridge(
            "build_production_widget_result",
            blueprint_asset,
            role,
            dependencies,
            visual_data,
            None,
            None,
        ):
            raise RuntimeError(f"Production Element build failed: role={role}")
        REPORT["rebuilt_assets"].append(asset_path)
        compile_widget(asset_path, blueprint_asset)
        if not call_widget_bridge(
            "validate_production_widget_result",
            blueprint_asset,
            role,
            dependencies,
            visual_data,
            None,
            None,
        ):
            raise RuntimeError(f"Production Element validation failed: role={role}")
        save_asset(asset_path, blueprint_asset)
        elements[role] = blueprint_asset

    # [v1.0.0] SpeedGauge/ArmorBodyMap Generated Class를 소비할 여섯 의미 Panel Asset입니다.
    panels: dict[str, Any] = {}
    for role, asset_path in PANEL_ASSETS.items():
        blueprint_asset = get_or_create_widget(asset_path)
        if not call_widget_bridge(
            "build_production_widget_result",
            blueprint_asset,
            role,
            dependencies,
            visual_data,
            elements["SpeedGauge"],
            elements["ArmorBodyMap"],
        ):
            raise RuntimeError(f"Production Panel build failed: role={role}")
        REPORT["rebuilt_assets"].append(asset_path)
        compile_widget(asset_path, blueprint_asset)
        if not call_widget_bridge(
            "validate_production_widget_result",
            blueprint_asset,
            role,
            dependencies,
            visual_data,
            elements["SpeedGauge"],
            elements["ArmorBodyMap"],
        ):
            raise RuntimeError(f"Production Panel validation failed: role={role}")
        save_asset(asset_path, blueprint_asset)
        panels[role] = blueprint_asset

    # [v1.0.0] 기존 WBP_CFInGameHUD Asset을 보존 재사용하면서 Production Panel Composition으로 Tree만 교체합니다.
    root_asset = get_or_create_widget(ROOT_ASSET_PATH)
    if not call_root_bridge("build_production_root_result", root_asset, dependencies, panels):
        raise RuntimeError("Production HUD Root composition failed")
    REPORT["rebuilt_assets"].append(ROOT_ASSET_PATH)
    compile_widget(ROOT_ASSET_PATH, root_asset)
    if not call_root_bridge("validate_production_root_result", root_asset, dependencies, panels):
        raise RuntimeError("Production HUD Root validation failed")
    save_asset(ROOT_ASSET_PATH, root_asset)

    REPORT["readback"] = {
        "root_generated_class": read_generated_class_path(root_asset),
        "element_generated_classes": {
            role: read_generated_class_path(asset) for role, asset in elements.items()
        },
        "panel_generated_classes": {
            role: read_generated_class_path(asset) for role, asset in panels.items()
        },
        }
    finalize_contracts()
    REPORT["success"] = True


# [v1.2.0] UI-P0-06 Stage B에서 기존 WBP_CFWeaponPanel 정확히 1개만 새 Bridge 계약으로 재구축·검증·저장합니다.
def run_weapon_panel_apply() -> None:
    # [v1.2.0] WeaponPanel Build/Validate가 읽기 전용으로 참조할 기존 Style/Density/Layout DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()
    # [v1.2.0] WeaponPanel Style/Image Resolve에 사용할 기존 HUD Visual DataAsset이며 이번 targeted mode에서는 저장하지 않습니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)
    # [v1.2.0] 공용 call_widget_bridge 시그니처를 만족할 기존 SpeedGauge Asset이며 WeaponPanel Build에서는 읽기 전용입니다.
    speed_gauge_asset = require_asset(ELEMENT_ASSETS["SpeedGauge"])
    # [v1.2.0] 공용 call_widget_bridge 시그니처를 만족할 기존 ArmorBodyMap Asset이며 WeaponPanel Build에서는 읽기 전용입니다.
    armor_body_map_asset = require_asset(ELEMENT_ASSETS["ArmorBodyMap"])
    # [v1.2.0] 이번 targeted mutation이 허용하는 유일한 Production Asset 경로입니다.
    weapon_panel_path = PANEL_ASSETS["WeaponPanel"]
    # [v1.2.0] 신규 생성 없이 반드시 이미 존재해야 하는 현재 Production WeaponPanel Blueprint입니다.
    weapon_panel_asset = require_asset(weapon_panel_path)
    validate_widget_parent(weapon_panel_asset)
    REPORT["reused_assets"].append(weapon_panel_path)

    if not call_widget_bridge(
        "build_production_widget_result",
        weapon_panel_asset,
        "WeaponPanel",
        dependencies,
        visual_data,
        speed_gauge_asset,
        armor_body_map_asset,
    ):
        raise RuntimeError("Targeted Production WeaponPanel build failed")
    REPORT["rebuilt_assets"].append(weapon_panel_path)

    compile_widget(weapon_panel_path, weapon_panel_asset)
    if not call_widget_bridge(
        "validate_production_widget_result",
        weapon_panel_asset,
        "WeaponPanel",
        dependencies,
        visual_data,
        speed_gauge_asset,
        armor_body_map_asset,
    ):
        raise RuntimeError("Targeted Production WeaponPanel validation failed")
    save_asset(weapon_panel_path, weapon_panel_asset)

    REPORT["readback"] = {
        "weapon_panel_generated_class": read_generated_class_path(weapon_panel_asset),
        "weapon_panel_asset_path": weapon_panel_path,
    }
    REPORT["contracts"] = {
        "operation_scope": "WeaponPanelOnly",
        "exact_mutated_asset_count": 1,
        "exact_mutated_assets": [weapon_panel_path],
        "other_production_asset_mutation_count": 0,
        "stage_b_compact_resource_slots": True,
        "raw_resource_channel_direct_row_generation": False,
        "reserve_ammo_header_owner_preserved": True,
        "legacy_fixed_resource_rows_forbidden": True,
    }
    REPORT["success"] = True


# [v1.0.0] 저장된 Production 10개 Asset을 새 프로세스에서 수정 없이 다시 검증합니다.
def run_readback() -> None:
    # [v1.0.0] Saved Readback에 사용할 기존 읽기 전용 DataAsset입니다.
    dependencies = load_read_only_dependencies()
    # [v1.0.0] Saved Readback에서 실제 로드된 HUD Visual DataAsset입니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)
    # [v1.0.0] Saved Readback에서 실제 로드된 두 의미 Element Asset입니다.
    elements = {role: require_asset(asset_path) for role, asset_path in ELEMENT_ASSETS.items()}
    # [v1.0.0] Saved Readback에서 실제 로드된 여섯 의미 Panel Asset입니다.
    panels = {role: require_asset(asset_path) for role, asset_path in PANEL_ASSETS.items()}
    # [v1.0.0] Saved Readback에서 실제 로드된 Production Root Asset입니다.
    root_asset = require_asset(ROOT_ASSET_PATH)

    for asset in [root_asset, *elements.values(), *panels.values()]:
        validate_widget_parent(asset)

    for role, asset in elements.items():
        if not call_widget_bridge(
            "validate_production_widget_result", asset, role, dependencies, visual_data, None, None
        ):
            raise RuntimeError(f"Saved Production Element validation failed: {role}")
    for role, asset in panels.items():
        if not call_widget_bridge(
            "validate_production_widget_result",
            asset,
            role,
            dependencies,
            visual_data,
            elements["SpeedGauge"],
            elements["ArmorBodyMap"],
        ):
            raise RuntimeError(f"Saved Production Panel validation failed: {role}")
    if not call_root_bridge("validate_production_root_result", root_asset, dependencies, panels):
        raise RuntimeError("Saved Production HUD Root validation failed")

    REPORT["readback"] = {
        "root_generated_class": read_generated_class_path(root_asset),
        "element_generated_classes": {
            role: read_generated_class_path(asset) for role, asset in elements.items()
        },
        "panel_generated_classes": {
            role: read_generated_class_path(asset) for role, asset in panels.items()
        },
    }
    finalize_contracts()
    REPORT["success"] = True


# [v1.1.0] Production Structure Gate와 후속 D1-11-ART 사용자 검토 상태를 분리해 기록합니다.
def finalize_contracts() -> None:
    # [v1.1.0] 현재 함수가 기록하는 exact 구조/Validator 계약이 모두 통과했음을 나타냅니다.
    d1_11_structure_pass = True
    REPORT["contracts"] = {
        "exact_mutable_asset_count": len(MUTABLE_ASSET_PATHS),
        "exact_root_widget_count": 1,
        "exact_panel_widget_count": len(PANEL_ASSETS),
        "exact_element_widget_count": len(ELEMENT_ASSETS),
        "exact_hud_visual_data_count": 1,
        "semantic_child_widget_structure_pass": True,
        "image_brush_structure_pass": True,
        "border_mosaic_residue_count": 0,
        "legacy_mock_bridge_mutated": False,
        "gameplay_cast_count": 0,
        "runtime_event_binding_count": 0,
        "ui_p0_03_view_data_count": 0,
        "d1_12_asset_count": 0,
        "preview_1920x1080_user_gate": "StructureSanityAccepted",
        "d1_11_structure_pass": d1_11_structure_pass,
        "d1_11_pass": d1_11_structure_pass,
        "d1_11_art_user_approval": "Pending",
        "note": "D1-11은 Production 내부 구조·의미 단위·안전 Fallback을 판정하는 Structure Gate입니다. 최종 HUD Art 형태, 세부 간격과 픽셀 배치는 D1-11-ART 사용자 검토에서 별도 승인합니다.",
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
    elif RUN_MODE == "weapon_panel_apply":
        run_weapon_panel_apply()
    elif RUN_MODE == "readback":
        run_readback()
    else:
        raise RuntimeError(f"Unsupported Production HUD run mode: {RUN_MODE}")
except Exception as error:
    REPORT["errors"].append({"message": str(error), "traceback": traceback.format_exc()})
    unreal.log_error(f"[CarFight][UIHUDProduction] {error}")
finally:
    REPORT_PATH.write_text(json.dumps(REPORT, ensure_ascii=False, indent=2), encoding="utf-8")
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Production UI HUD operation failed. Read report.json for details.")
