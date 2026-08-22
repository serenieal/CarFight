# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.7.0
# Date: 2026-08-21
# Description: CF-FQ-032 Production HUD Scaffold/Validate + UI-P0-09B View Mode Layout-Preserving Targeted Apply 도구
# Scope: 기존 Production Designer Layout을 보존하고 신규 Scaffold 또는 명시적 additive Radar/ViewMode Visual migration만 허용합니다.
# Changelog:
# - v1.7.0: `view_mode_visual_apply`를 추가해 기존 WBP_CFInGameHUD 정확히 1개 Asset만 저장하며 ReticleLayer 내부 Vehicle Direction Track/Image 누락분만 additive 추가. Root/Panel 기존 Slot Layout 재적용과 Gameplay Camera/Aim mutation 금지.
# - v1.6.0: `radar_visual_apply`를 추가해 Radar PNG 7종 + DA_CFHUDVisual_Default + 기존 WBP_CFRadarPanel 정확히 9개 Asset만 갱신. RadarPanel은 Tree rebuild 없이 Frame/Range/Player/SelectedEdge 누락 Widget만 additive 추가하고 기존 Slot Layout은 보존.
# - v1.5.0: 기존 Production Widget/Root의 Build→Compile→Save를 제거하고 Validate-only로 전환. 신규 Asset만 Scaffold Build를 허용하며 RPM targeted Apply는 Texture+Material+VisualData 3개만 저장하고 SpeedGauge Layout은 읽기/검증 전용으로 보호.

# - v1.4.1: UE 5.8 MaterialExpressionIf의 실제 입력 핀 `A > B` / `A == B` / `A < B`를 사용하도록 교정하고 모든 Material expression 연결 결과를 fail-closed 검사해 silent graph wiring 실패를 차단.
# - v1.4.0: `rpm_gauge_apply`를 추가해 T_UI_RPMTrack 선형 데이터 Texture + M_UI_RPMGauge UI Material + DA_CFHUDVisual_Default 연결 + WBP_CFSpeedGauge 재구축 정확히 4개 Asset만 저장합니다.
# - v1.3.0: 재사용 WBP_CFArmorSector를 세 번째 Element로 추가하고 `armor_map_apply`에서 ArmorSector 생성/재구축 + ArmorBodyMap 재구축 정확히 2개 Asset만 Compile·Validate·Save하도록 추가.
# - v1.2.0: `weapon_panel_apply` targeted mode를 추가해 기존 Production 전체 10 Asset 재작성 없이 WBP_CFWeaponPanel 하나만 Bridge Build→Compile→Validate→Save하도록 제한.
# - v1.1.0: 사용자 결정에 맞춰 D1-11을 Structure-first Gate로 판정하고, 미술 승인/세부 배치를 별도 D1-11-ART Pending 상태로 분리.
# - v1.0.0: Production 의미 단위 Widget 9개와 DA_CFHUDVisual_Default의 exact allowlist, 순차 Compile/Root Composition/Readback을 최초 추가.
# Migration:
# - 기존 ApplyUIHUDPrototype.py v1.x는 Historical Border Mock 재현용으로 보존합니다.
# - Production Apply는 D1-09B Style/Density/Layout/Icon Asset을 읽기 전용으로 사용합니다.
# - D1-11 Structure PASS는 Root/Panel/Element/VisualData exact 구조와 Validator 계약을 의미하며 최종 HUD Art 승인과 픽셀 폴리시는 D1-11-ART에서 별도 판정합니다.
# - D1-12, UI-P0-07 Runtime Event Binding과 Gameplay 조회를 생성하지 않습니다.
# - v1.2.0 `weapon_panel_apply`는 기존 Asset 생성을 허용하지 않고 정확한 WBP_CFWeaponPanel이 이미 존재해야 하며, 다른 Production Asset은 읽기 전용 Dependency로만 사용합니다.
# - v1.3.0 `armor_map_apply`는 신규 WBP_CFArmorSector 생성과 기존 WBP_CFArmorBodyMap 재구축만 허용하며 SpeedGauge/VehiclePanel/VisualData를 저장하지 않습니다.
# - v1.4.0 `rpm_gauge_apply`는 RPM SourceArt 한 장만 reimport하고 Material/VisualData/SpeedGauge만 저장합니다. VehiclePanel/Armor/Root는 저장하지 않습니다.
# - v1.5.0부터 저장된 Widget Blueprint는 기본적으로 Validate-only입니다. Position/Size/Anchor/Alignment/Padding/AutoSize를 다시 쓰는 Tree rebuild는 신규 Scaffold에만 허용합니다.
# - v1.6.0 `radar_visual_apply`만 예외적으로 기존 RadarPanel에 명시된 새 의미 Widget 4개를 누락 시 additive 추가합니다. 기존 Widget Slot Layout은 수정하지 않으며 다른 Production Widget은 저장하지 않습니다.
# - v1.7.0 `view_mode_visual_apply`는 기존 Root의 7 direct child와 ReticleLayer Slot을 그대로 유지하고 ReticleLayer 내부에 Vehicle Direction Track/Image만 누락 시 추가합니다. 저장 허용 대상은 WBP_CFInGameHUD 1개뿐입니다.



from __future__ import annotations

import json
import os
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.1.0] 구조화 보고서에서 식별할 현재 Production 도구 버전입니다.
TOOL_VERSION = "1.7.0"


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
    "ArmorSector": "/Game/CarFight/UI/HUD/Elements/WBP_CFArmorSector",
    "SpeedGauge": "/Game/CarFight/UI/HUD/Elements/WBP_CFSpeedGauge",
    "ArmorBodyMap": "/Game/CarFight/UI/HUD/Elements/WBP_CFArmorBodyMap",
}

# [v1.0.0] Production HUD 전용 교체 가능 Visual DataAsset 경로입니다.
VISUAL_ASSET_PATH = "/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default"

# [v1.4.0] 동적 RPM Gauge 데이터 Texture의 정확한 Content 경로입니다.
RPM_TRACK_ASSET_PATH = "/Game/CarFight/UI/HUD/Visual/T_UI_RPMTrack"

# [v1.4.0] 동적 RPM Gauge UI Material의 정확한 Content 경로입니다.
RPM_MATERIAL_ASSET_PATH = "/Game/CarFight/UI/HUD/Visual/M_UI_RPMGauge"

# [v1.4.0] 저장소 SourceArt 아래 재생성된 RPM 데이터 PNG의 절대 경로입니다.
RPM_TRACK_SOURCE_PATH = PROJECT_DIR.parent / "SourceArt" / "UI" / "HUD" / "P2" / "T_UI_RPMTrack.png"

# [v1.5.0] RPM targeted Apply가 의미상 다루는 Texture/Material/VisualData/SpeedGauge 4개 Target입니다.
RPM_GAUGE_TARGET_ASSET_PATHS = {
    RPM_TRACK_ASSET_PATH,
    RPM_MATERIAL_ASSET_PATH,
    VISUAL_ASSET_PATH,
    ELEMENT_ASSETS["SpeedGauge"],
}

# [v1.5.0] RPM targeted Apply에서 실제 저장이 허용되는 3개 비-Designer Asset입니다. SpeedGauge는 Validate-only입니다.
RPM_GAUGE_MUTABLE_ASSET_PATHS = {
    RPM_TRACK_ASSET_PATH,
    RPM_MATERIAL_ASSET_PATH,
    VISUAL_ASSET_PATH,
}

# [v1.6.0] Radar 전용 SourceArt가 위치한 저장소 경로입니다.
RADAR_SOURCE_ROOT = PROJECT_DIR.parent / "SourceArt" / "UI" / "HUD" / "P2"

# [v1.6.0] Radar VisualData property별 Content Texture 경로와 PNG 파일명의 안정 매핑입니다.
RADAR_TEXTURE_SPECS: dict[str, tuple[str, str]] = {
    "radar_frame": ("/Game/CarFight/UI/HUD/Visual/T_UI_RadarFrame", "T_UI_RadarFrame.png"),
    "radar_friendly_blip": ("/Game/CarFight/UI/HUD/Visual/T_UI_RadarFriend", "T_UI_RadarFriend.png"),
    "radar_hostile_blip": ("/Game/CarFight/UI/HUD/Visual/T_UI_RadarHostile", "T_UI_RadarHostile.png"),
    "radar_unknown_blip": ("/Game/CarFight/UI/HUD/Visual/T_UI_RadarUnknown", "T_UI_RadarUnknown.png"),
    "radar_player_marker": ("/Game/CarFight/UI/HUD/Visual/T_UI_RadarPlayer", "T_UI_RadarPlayer.png"),
    "selected_target_bracket": ("/Game/CarFight/UI/HUD/Visual/T_UI_TargetBracket", "T_UI_TargetBracket.png"),
    "radar_selected_edge_bracket": ("/Game/CarFight/UI/HUD/Visual/T_UI_RadarEdge", "T_UI_RadarEdge.png"),
}

# [v1.6.0] Radar targeted Apply가 import/reimport할 정확한 7개 Texture 경로입니다.
RADAR_TEXTURE_ASSET_PATHS = {asset_path for asset_path, _ in RADAR_TEXTURE_SPECS.values()}

# [v1.6.0] Radar targeted Apply에서 실제 저장이 허용되는 7 Texture + VisualData + RadarPanel 정확한 9개 Asset입니다.
RADAR_VISUAL_MUTABLE_ASSET_PATHS = {
    *RADAR_TEXTURE_ASSET_PATHS,
    VISUAL_ASSET_PATH,
    PANEL_ASSETS["RadarPanel"],
}

# [v1.7.0] ViewMode targeted Apply에서 실제 저장이 허용되는 기존 Production Root 정확히 1개 Asset입니다.
VIEW_MODE_MUTABLE_ASSET_PATHS = {ROOT_ASSET_PATH}


# [v1.3.0] Production Apply가 생성 또는 재구축할 수 있는 정확한 11개 Asset allowlist입니다.
MUTABLE_ASSET_PATHS = {
    ROOT_ASSET_PATH,
    VISUAL_ASSET_PATH,
    *PANEL_ASSETS.values(),
    *ELEMENT_ASSETS.values(),
}

# [v1.7.0] 현재 실행 모드가 실제로 저장할 수 있는 exact Asset allowlist입니다.
REPORT_MUTABLE_ASSET_PATHS = (
    RPM_GAUGE_MUTABLE_ASSET_PATHS
    if RUN_MODE == "rpm_gauge_apply"
    else RADAR_VISUAL_MUTABLE_ASSET_PATHS
    if RUN_MODE == "radar_visual_apply"
    else VIEW_MODE_MUTABLE_ASSET_PATHS
    if RUN_MODE == "view_mode_visual_apply"
    else MUTABLE_ASSET_PATHS
)


# [v1.0.0] Production 실행 결과와 Evidence를 보존할 구조화 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "carfight_ui_hud_production_v1",
    "tool_version": TOOL_VERSION,
    "mode": RUN_MODE,
    "success": False,
    "mutable_assets": sorted(REPORT_MUTABLE_ASSET_PATHS),
        "read_only_dependencies": [
        STYLE_ASSET_PATH,
        DENSITY_STANDARD_PATH,
        DENSITY_COMPACT_PATH,
        LAYOUT_1080_PATH,
    ],
    "created_assets": [],
    "reused_assets": [],

    "layout_preserved_assets": [],
    "scaffolded_assets": [],
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
                "apply_radar_visual_migration_result",
        "apply_view_mode_visual_migration_result",
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


# [v1.3.0] Production Widget 역할에 따라 정확한 Native Parent Class를 반환합니다.
def get_expected_widget_parent_class(role: str) -> Any:
    # [v1.3.0] 재사용 ArmorSector는 Armor 비율 적용 API를 가진 전용 Native Presentation Base를 사용합니다.
    if role == "ArmorSector":
        return require_unreal_type("CFArmorSectorWidget").static_class()
    # [v1.3.0] 기존 Root/Panel/다른 Element는 모두 공통 CFStyledWidgetBase 계약을 유지합니다.
    return require_unreal_type("CFStyledWidgetBase").static_class()


# [v1.3.0] Production Widget Blueprint가 역할별 정확한 Native Parent를 사용하는지 검증합니다.
def validate_widget_parent(blueprint_asset: Any, role: str) -> None:
    # [v1.3.0] 현재 역할이 정확히 상속해야 하는 Native Parent UClass입니다.
    expected_parent_class = get_expected_widget_parent_class(role)
    # [v1.0.0] 현재 Widget Blueprint의 실제 Native Parent UClass입니다.
    actual_parent_class = get_blueprint_parent_class(blueprint_asset)
    if actual_parent_class != expected_parent_class:
        raise RuntimeError(
            f"Production Widget parent mismatch: role={role} asset={blueprint_asset.get_path_name()} "
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


# [v1.3.0] 역할별 정확한 Native Parent로 지정 Widget Blueprint를 생성하거나 기존 Asset을 보호 재사용합니다.
def get_or_create_widget(asset_path: str, role: str) -> Any:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Production Widget is outside mutation allowlist: {asset_path}")
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        # [v1.3.0] 기존 정확한 Target Asset입니다.
        existing_asset = require_asset(asset_path)
        validate_widget_parent(existing_asset, role)
        REPORT["reused_assets"].append(asset_path)
        return existing_asset

    # [v1.3.0] 전체 Apply 또는 ArmorMap targeted Apply에서만 신규 Widget 생성을 허용합니다.
    creation_allowed = RUN_MODE == "apply" or (RUN_MODE == "armor_map_apply" and role == "ArmorSector")
    if not creation_allowed:
        raise RuntimeError(f"Production Widget creation is not allowed in mode={RUN_MODE}: role={role} asset={asset_path}")

    # [v1.0.0] 신규 Widget Blueprint를 저장할 Content 폴더입니다.
    folder_path, asset_name = asset_path.rsplit("/", 1)
    unreal.EditorAssetLibrary.make_directory(folder_path)
    # [v1.3.0] 신규 Widget Blueprint의 역할별 정확한 Native Parent입니다.
    parent_class = get_expected_widget_parent_class(role)
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


# [v1.4.0] SourceArt의 RPM 데이터 PNG 한 장을 동일 이름 Texture2D로 reimport하고 선형 UI 데이터 Texture 규격을 강제합니다.
def import_rpm_track_texture() -> Any:
    if not RPM_TRACK_SOURCE_PATH.is_file():
        raise RuntimeError(f"RPM Gauge SourceArt is missing: {RPM_TRACK_SOURCE_PATH}")

    # [v1.4.0] 대화상자 없이 기존 T_UI_RPMTrack을 교체 import할 UE 5.8 AssetImportTask입니다.
    import_task = require_unreal_type("AssetImportTask")()
    import_task.set_editor_property("filename", str(RPM_TRACK_SOURCE_PATH))
    import_task.set_editor_property("destination_path", "/Game/CarFight/UI/HUD/Visual")
    import_task.set_editor_property("destination_name", "T_UI_RPMTrack")
    import_task.set_editor_property("automated", True)
    import_task.set_editor_property("replace_existing", True)
    import_task.set_editor_property("replace_existing_settings", False)
    import_task.set_editor_property("save", False)

    # [v1.4.0] 저장 없이 import만 수행할 AssetTools 인터페이스입니다.
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([import_task])

    # [v1.4.0] reimport 직후 정확한 Content 경로에서 다시 읽은 RPM 데이터 Texture입니다.
    rpm_texture = require_asset(RPM_TRACK_ASSET_PATH)
    if not isinstance(rpm_texture, require_unreal_type("Texture2D")):
        raise RuntimeError(f"RPM Gauge imported asset is not Texture2D: {RPM_TRACK_ASSET_PATH}")

            # [v1.4.0] UI용 데이터 Texture이므로 색공간 변환 없이 RGBA 채널 값을 그대로 Material에 전달합니다.
    rpm_texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)

    rpm_texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    rpm_texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    rpm_texture.set_editor_property("srgb", False)
    REPORT["rebuilt_assets"].append(RPM_TRACK_ASSET_PATH)
    return rpm_texture


# [v1.6.0] 지정 Radar SourceArt PNG를 동일 이름 Texture2D로 import/reimport하고 일반 색상 UI Texture 규격을 적용합니다.
def import_radar_texture(asset_path: str, source_file_name: str) -> Any:
    # [v1.6.0] 현재 Radar Texture가 읽을 저장소 SourceArt PNG 절대 경로입니다.
    source_path = RADAR_SOURCE_ROOT / source_file_name
    if not source_path.is_file():
        raise RuntimeError(f"Radar SourceArt is missing: {source_path}")

    # [v1.6.0] 현재 Radar Content 경로에서 사용할 정확한 Asset 이름입니다.
    asset_name = asset_path.rsplit("/", 1)[1]

    # [v1.6.0] 대화상자 없이 Radar Texture를 import/reimport할 UE 5.8 AssetImportTask입니다.
    import_task = require_unreal_type("AssetImportTask")()
    import_task.set_editor_property("filename", str(source_path))
    import_task.set_editor_property("destination_path", "/Game/CarFight/UI/HUD/Visual")
    import_task.set_editor_property("destination_name", asset_name)
    import_task.set_editor_property("automated", True)
    import_task.set_editor_property("replace_existing", True)
    import_task.set_editor_property("replace_existing_settings", False)
    import_task.set_editor_property("save", False)

    # [v1.6.0] 저장 없이 import/reimport만 수행할 AssetTools 인터페이스입니다.
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([import_task])

    # [v1.6.0] import 직후 정확한 Content 경로에서 다시 읽은 Radar Texture2D입니다.
    radar_texture = require_asset(asset_path)
    if not isinstance(radar_texture, require_unreal_type("Texture2D")):
        raise RuntimeError(f"Radar imported asset is not Texture2D: {asset_path}")

    radar_texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    radar_texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    radar_texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    radar_texture.set_editor_property("srgb", True)
    REPORT["rebuilt_assets"].append(asset_path)
    return radar_texture


# [v1.4.0] 동적 RPM Gauge 전용 UI Material Asset을 생성하거나 기존 정확한 Material을 재사용합니다.
def get_or_create_rpm_material() -> Any:
    if unreal.EditorAssetLibrary.does_asset_exist(RPM_MATERIAL_ASSET_PATH):
        # [v1.4.0] 기존 동적 RPM UI Material입니다.
        existing_material = require_asset(RPM_MATERIAL_ASSET_PATH)
        if not isinstance(existing_material, require_unreal_type("Material")):
            raise RuntimeError(f"Existing RPM Gauge asset is not Material: {RPM_MATERIAL_ASSET_PATH}")
        REPORT["reused_assets"].append(RPM_MATERIAL_ASSET_PATH)
        return existing_material

    # [v1.4.0] RPM targeted mode 또는 전체 Apply 외에는 Material 신규 생성을 허용하지 않습니다.
    if RUN_MODE not in {"rpm_gauge_apply", "apply"}:
        raise RuntimeError(f"RPM Gauge Material creation is not allowed in mode={RUN_MODE}")

    # [v1.4.0] RPM UI Material을 저장할 Content 폴더와 Asset 이름입니다.
    folder_path, asset_name = RPM_MATERIAL_ASSET_PATH.rsplit("/", 1)
    unreal.EditorAssetLibrary.make_directory(folder_path)

    # [v1.4.0] 일반 UMaterial 생성을 위한 UE 5.8 Factory입니다.
    material_factory = require_unreal_type("MaterialFactoryNew")()

    # [v1.4.0] 생성된 동적 RPM Gauge Material Asset입니다.
    created_material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        folder_path,
        require_unreal_type("Material"),
        material_factory,
    )
    if created_material is None:
        raise RuntimeError(f"RPM Gauge Material creation failed: {RPM_MATERIAL_ASSET_PATH}")
    REPORT["created_assets"].append(RPM_MATERIAL_ASSET_PATH)
    return created_material


# [v1.4.0] Material Graph Expression을 만들고 생성 실패를 즉시 차단합니다.
def create_material_expression(material: Any, expression_type_name: str, x: int, y: int) -> Any:
    # [v1.4.0] 현재 만들 Material Expression의 Python 타입입니다.
    expression_type = require_unreal_type(expression_type_name)

    # [v1.4.0] 현재 UI Material Graph에 실제로 생성된 Expression입니다.
    expression = unreal.MaterialEditingLibrary.create_material_expression(material, expression_type, x, y)
    if expression is None:
        raise RuntimeError(f"RPM Gauge Material expression creation failed: {expression_type_name}")
    return expression


# [v1.4.1] Material Expression 연결 결과를 fail-closed 검사해 잘못된 핀 이름이나 연결 실패가 저장까지 진행되지 않도록 합니다.
def connect_material_expressions_checked(
    material_library: Any,
    from_expression: Any,
    from_output_name: str,
    to_expression: Any,
    to_input_name: str,
) -> None:
    # [v1.4.1] UE 5.8 MaterialEditingLibrary가 반환한 실제 연결 성공 여부입니다.
    connected = material_library.connect_material_expressions(
        from_expression,
        from_output_name,
        to_expression,
        to_input_name,
    )
    if not connected:
        raise RuntimeError(
            f"RPM Gauge Material connection failed: output={from_output_name or '<default>'} input={to_input_name}"
        )


# [v1.4.1] T_UI_RPMTrack 데이터 채널과 RPMRatio 하나만 소비하는 동적 UI Material Graph를 결정론적으로 재생성합니다.
def rebuild_rpm_material(material: Any, rpm_texture: Any) -> None:
    # [v1.4.0] Material을 Slate/UMG 전용 UI Domain + Translucent Blend로 고정합니다.
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    # [v1.4.0] 기존 Graph를 누적하지 않고 정확한 RPM Gauge Graph로 재구축하는 Editor Library입니다.
    material_library = unreal.MaterialEditingLibrary
    material_library.delete_all_material_expressions(material)

    # [v1.4.0] R=진행 위치, G=선 강도, B=Redline, A=Coverage를 제공하는 RPM 데이터 Texture Sample입니다.
    gauge_data = create_material_expression(material, "MaterialExpressionTextureSample", -900, 0)
    gauge_data.set_editor_property("texture", rpm_texture)
    gauge_data.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)

    # [v1.4.0] Presenter가 매 갱신마다 0~1 현재 RPM Gauge 진행 비율을 설정할 단일 Scalar Parameter입니다.
    rpm_ratio = create_material_expression(material, "MaterialExpressionScalarParameter", -900, -260)
    rpm_ratio.set_editor_property("parameter_name", unreal.Name("RPMRatio"))
    rpm_ratio.set_editor_property("default_value", 0.0)

    # [v1.4.0] RPM 진행 위치 이하를 활성화할 If 결과의 true 상수입니다.
    active_one = create_material_expression(material, "MaterialExpressionConstant", -900, -420)
    active_one.set_editor_property("r", 1.0)

    # [v1.4.0] RPM 진행 위치를 아직 통과하지 않은 픽셀을 비활성화할 false 상수입니다.
    inactive_zero = create_material_expression(material, "MaterialExpressionConstant", -900, -520)
    inactive_zero.set_editor_property("r", 0.0)

        # [v1.4.1] RPMRatio >= GaugeData.R이면 1, 아니면 0을 반환하는 활성 Mask입니다.
    active_mask = create_material_expression(material, "MaterialExpressionIf", -560, -180)
    connect_material_expressions_checked(material_library, rpm_ratio, "", active_mask, "A")
    connect_material_expressions_checked(material_library, gauge_data, "R", active_mask, "B")
    connect_material_expressions_checked(material_library, active_one, "", active_mask, "A > B")
    connect_material_expressions_checked(material_library, active_one, "", active_mask, "A == B")
    connect_material_expressions_checked(material_library, inactive_zero, "", active_mask, "A < B")

    # [v1.4.0] 아직 RPM이 도달하지 않은 Gauge 선의 어두운 청회색입니다.
    inactive_color = create_material_expression(material, "MaterialExpressionVectorParameter", -540, 180)
    inactive_color.set_editor_property("parameter_name", unreal.Name("InactiveColor"))
    inactive_color.set_editor_property("default_value", unreal.LinearColor(0.055, 0.12, 0.15, 1.0))

    # [v1.4.0] 일반 활성 RPM Gauge 구간의 Cyan 색입니다.
    active_color = create_material_expression(material, "MaterialExpressionVectorParameter", -540, 320)
    active_color.set_editor_property("parameter_name", unreal.Name("ActiveColor"))
    active_color.set_editor_property("default_value", unreal.LinearColor(0.0, 0.82, 1.0, 1.0))

    # [v1.4.0] 85% 이후 활성 Redline 구간의 Orange-Red 색입니다.
    redline_color = create_material_expression(material, "MaterialExpressionVectorParameter", -540, 460)
    redline_color.set_editor_property("parameter_name", unreal.Name("RedlineColor"))
    redline_color.set_editor_property("default_value", unreal.LinearColor(1.0, 0.20, 0.055, 1.0))

    # [v1.4.0] GaugeData.B Redline Mask에 따라 활성 Cyan과 Orange-Red를 선택합니다.
    active_zone_color = create_material_expression(material, "MaterialExpressionLinearInterpolate", -220, 340)
    connect_material_expressions_checked(material_library, active_color, "", active_zone_color, "A")
    connect_material_expressions_checked(material_library, redline_color, "", active_zone_color, "B")
    connect_material_expressions_checked(material_library, gauge_data, "B", active_zone_color, "Alpha")

    # [v1.4.0] 현재 RPM보다 앞은 어두운 색, 현재 RPM까지는 활성 색을 선택합니다.
    state_color = create_material_expression(material, "MaterialExpressionLinearInterpolate", 80, 160)
    connect_material_expressions_checked(material_library, inactive_color, "", state_color, "A")
    connect_material_expressions_checked(material_library, active_zone_color, "", state_color, "B")
    connect_material_expressions_checked(material_library, active_mask, "", state_color, "Alpha")

    # [v1.4.0] Texture G 채널의 Major/Minor/Guide 강도를 최종 색에 곱합니다.
    final_color = create_material_expression(material, "MaterialExpressionMultiply", 360, 120)
    connect_material_expressions_checked(material_library, state_color, "", final_color, "A")
    connect_material_expressions_checked(material_library, gauge_data, "G", final_color, "B")

    # [v1.4.0] UI Domain의 최종 색은 Emissive, 투명도는 Texture A Coverage로 연결합니다.
    if not material_library.connect_material_property(final_color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        raise RuntimeError("RPM Gauge Material failed to connect final color")
    if not material_library.connect_material_property(gauge_data, "A", unreal.MaterialProperty.MP_OPACITY):
        raise RuntimeError("RPM Gauge Material failed to connect opacity")

    # [v1.4.1] Material Graph 변경을 컴파일하고 오류 메시지가 하나라도 있으면 저장 전에 차단합니다.
    compile_errors = material_library.recompile_material(material)
    if compile_errors:
        raise RuntimeError(f"RPM Gauge Material compile failed: {compile_errors}")
    REPORT["rebuilt_assets"].append(RPM_MATERIAL_ASSET_PATH)


# [v1.0.0] Widget Blueprint를 공식 Editor Library로 Compile하고 Generated Class를 확인합니다.
def compile_widget(asset_path: str, blueprint_asset: Any) -> None:
    # [v1.0.0] Blueprint Compile에 사용할 UE 5.8 Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    blueprint_editor_library.compile_blueprint(blueprint_asset)
    if not read_generated_class_path(blueprint_asset):
        raise RuntimeError(f"Production Widget generated class is missing after compile: {asset_path}")
    REPORT["compiled_assets"].append(asset_path)


# [v1.7.0] 현재 실행 모드의 exact mutation allowlist 안에 있는 Asset만 저장합니다.
def save_asset(asset_path: str, asset: Any) -> None:
    # [v1.7.0] 전체 Scaffold / 기존 targeted mode / Radar 9 Asset / ViewMode Root 1 Asset에 대한 mode-scoped 저장 allowlist입니다.
    allowed_asset_paths = (
        MUTABLE_ASSET_PATHS
        if RUN_MODE == "apply"
        else {PANEL_ASSETS["WeaponPanel"]}
        if RUN_MODE == "weapon_panel_apply"
        else {ELEMENT_ASSETS["ArmorSector"], ELEMENT_ASSETS["ArmorBodyMap"]}
        if RUN_MODE == "armor_map_apply"
        else RPM_GAUGE_MUTABLE_ASSET_PATHS
        if RUN_MODE == "rpm_gauge_apply"
                else RADAR_VISUAL_MUTABLE_ASSET_PATHS
        if RUN_MODE == "radar_visual_apply"
        else VIEW_MODE_MUTABLE_ASSET_PATHS
        if RUN_MODE == "view_mode_visual_apply"
        else set()
    )
    if asset_path not in allowed_asset_paths:
        raise RuntimeError(f"Production HUD save is outside apply contract: mode={RUN_MODE} asset={asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Production HUD asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.3.0] Production Child 역할 Build/Validate C++ Bridge를 공통 인수로 호출합니다.
def call_widget_bridge(
    method_name: str,
    blueprint_asset: Any,
    role: str,
    dependencies: dict[str, Any],
    visual_data: Any,
    speed_gauge_asset: Any | None,
    armor_body_map_asset: Any | None,
    armor_sector_asset: Any | None,
) -> bool:
    # [v1.3.0] 실제 호출할 Production Child bool-only C++ Bridge 메서드입니다.
    bridge_method = getattr(require_bridge(), method_name)
    # [v1.3.0] UE 5.8 Python에서 직접 반환되는 Build/Validate Bool 결과입니다.
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
        armor_sector_asset,
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


# [v1.5.0] 현재 실행에서 새로 생성된 Asset인지 확인해 Scaffold와 기존 Designer 유지 경로를 분리합니다.
def was_created_this_run(asset_path: str) -> bool:
    return asset_path in REPORT["created_assets"]


# [v1.5.0] 기존 Designer Widget은 Validate-only로 보호하고, 이번 실행에서 새로 생성한 Widget만 최초 Scaffold→Compile→Validate→Save합니다.
def scaffold_or_validate_widget(
    asset_path: str,
    role: str,
    blueprint_asset: Any,
    dependencies: dict[str, Any],
    visual_data: Any,
    speed_gauge_asset: Any | None,
    armor_body_map_asset: Any | None,
    armor_sector_asset: Any | None,
) -> None:
    # [v1.5.0] 신규 Asset만 destructive Tree construction이 허용되는 Scaffold 대상입니다.
    is_new_scaffold = was_created_this_run(asset_path)
    if is_new_scaffold:
        if not call_widget_bridge(
            "build_production_widget_result",
            blueprint_asset,
            role,
            dependencies,
            visual_data,
            speed_gauge_asset,
            armor_body_map_asset,
            armor_sector_asset,
        ):
            raise RuntimeError(f"Production Widget scaffold failed: role={role} asset={asset_path}")
        REPORT["scaffolded_assets"].append(asset_path)
        REPORT["rebuilt_assets"].append(asset_path)
        compile_widget(asset_path, blueprint_asset)
    else:
        # [v1.5.0] 저장된 UMG의 Position/Size/Anchor/Alignment/Padding/AutoSize를 그대로 남겼음을 보고합니다.
        REPORT["layout_preserved_assets"].append(asset_path)

    if not call_widget_bridge(
        "validate_production_widget_result",
        blueprint_asset,
        role,
        dependencies,
        visual_data,
        speed_gauge_asset,
        armor_body_map_asset,
        armor_sector_asset,
    ):
        raise RuntimeError(f"Production Widget validation failed: role={role} asset={asset_path}")

    if is_new_scaffold:
        save_asset(asset_path, blueprint_asset)


# [v1.5.0] 기존 Production Root는 Validate-only로 보호하고 이번 실행에서 새로 생성한 Root만 최초 Scaffold→Compile→Validate→Save합니다.
def scaffold_or_validate_root(root_asset: Any, dependencies: dict[str, Any], panels: dict[str, Any]) -> None:
    # [v1.5.0] Root가 이번 실행에서 생성됐을 때만 초기 D1-07 Slot Scaffold를 허용합니다.
    is_new_scaffold = was_created_this_run(ROOT_ASSET_PATH)
    if is_new_scaffold:
        if not call_root_bridge("build_production_root_result", root_asset, dependencies, panels):
            raise RuntimeError("Production HUD Root scaffold failed")
        REPORT["scaffolded_assets"].append(ROOT_ASSET_PATH)
        REPORT["rebuilt_assets"].append(ROOT_ASSET_PATH)
        compile_widget(ROOT_ASSET_PATH, root_asset)
    else:
        REPORT["layout_preserved_assets"].append(ROOT_ASSET_PATH)

    if not call_root_bridge("validate_production_root_result", root_asset, dependencies, panels):
        raise RuntimeError("Production HUD Root validation failed")

    if is_new_scaffold:
        save_asset(ROOT_ASSET_PATH, root_asset)


# [v1.0.0] Probe에서 필수 Python/C++ 타입과 읽기 전용 Dependency 존재 여부를 수집합니다.

def run_probe() -> None:
    # [v1.0.0] Production 실행에 반드시 필요한 Unreal Python 타입 이름입니다.
    required_type_names = [
        "WidgetBlueprint",
        "WidgetBlueprintFactory",
        "BlueprintEditorLibrary",
        "DataAssetFactory",
        "AssetImportTask",
        "Material",
        "MaterialFactoryNew",
        "MaterialEditingLibrary",
        "MaterialExpressionTextureSample",
        "MaterialExpressionScalarParameter",
        "MaterialExpressionVectorParameter",
        "MaterialExpressionIf",
        "MaterialExpressionLinearInterpolate",
        "MaterialExpressionMultiply",
        "MaterialExpressionConstant",
        "CFStyledWidgetBase",
        "CFArmorSectorWidget",
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


# [v1.5.0] Content 변경 없이 기존 Widget은 Validate-only, 누락 Widget은 최초 Scaffold 대상인지 판정합니다.
def run_dry_run() -> None:
    load_read_only_dependencies()
    # [v1.5.0] 기존 Widget Target의 역할별 Native Parent 보호 결과입니다.
    existing_widget_validation: dict[str, Any] = {}
    # [v1.5.0] Root/Element/Panel 경로를 역할과 연결해 기존 Designer Asset 여부를 확인합니다.
    widget_roles = {
        ROOT_ASSET_PATH: "Root",
        **{asset_path: role for role, asset_path in ELEMENT_ASSETS.items()},
        **{asset_path: role for role, asset_path in PANEL_ASSETS.items()},
    }
    for asset_path, role in widget_roles.items():
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            # [v1.5.0] 현재 저장된 Production Widget입니다.
            asset = require_asset(asset_path)
            validate_widget_parent(asset, role)
            existing_widget_validation[asset_path] = get_expected_widget_parent_class(role).get_path_name()
    REPORT["dry_run"] = {
        "mutable_asset_count": len(MUTABLE_ASSET_PATHS),
        "mutable_assets": sorted(MUTABLE_ASSET_PATHS),
        "existing_widget_parent_validation": existing_widget_validation,
        "actions": {
            asset_path: "validate_preserve_layout" if unreal.EditorAssetLibrary.does_asset_exist(asset_path) else "create_scaffold"
            for asset_path in sorted(MUTABLE_ASSET_PATHS)
        },
        "existing_widget_layout_owner": "PersistedUMGDesignerAsset",
        "existing_widget_tree_rebuild": False,
        "legacy_mock_bridge_mutated": False,
        "d1_12_started": False,
        "ui_p0_03_started": False,
    }
    REPORT["success"] = True



# [v1.5.0] Production dependency 순서를 유지하되 기존 Widget/Root는 Validate-only, 이번 실행에서 생성한 누락 Asset만 최초 Scaffold합니다.
def run_apply() -> None:
    # [v1.5.0] Production Scaffold/Validate에 사용할 기존 읽기 전용 DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()
    # [v1.5.0] HUD 전용 시각 참조를 보유할 신규 또는 기존 Visual DataAsset입니다.
    visual_data = get_or_create_visual_data()
    if was_created_this_run(VISUAL_ASSET_PATH):
        save_asset(VISUAL_ASSET_PATH, visual_data)

    # [v1.5.0] 다른 Element가 재사용할 ArmorSector를 먼저 확보하고 신규/기존 경로를 분리합니다.
    elements: dict[str, Any] = {}
    # [v1.5.0] 재사용 ArmorSector의 정확한 Production Asset 경로입니다.
    armor_sector_path = ELEMENT_ASSETS["ArmorSector"]
    # [v1.5.0] 신규면 Scaffold, 기존이면 Layout 보존 검증할 ArmorSector Asset입니다.
    armor_sector_asset = get_or_create_widget(armor_sector_path, "ArmorSector")
    scaffold_or_validate_widget(
        armor_sector_path,
        "ArmorSector",
        armor_sector_asset,
        dependencies,
        visual_data,
        None,
        None,
        None,
    )
    elements["ArmorSector"] = armor_sector_asset

    # [v1.5.0] SpeedGauge는 독립 Element이고 ArmorBodyMap은 ArmorSector Generated Class 의미 dependency만 소비합니다.
    for role in ("SpeedGauge", "ArmorBodyMap"):
        # [v1.5.0] 현재 Element 역할의 정확한 Production Asset 경로입니다.
        asset_path = ELEMENT_ASSETS[role]
        # [v1.5.0] 신규면 Scaffold, 기존이면 Designer Layout을 보존할 Production Widget입니다.
        blueprint_asset = get_or_create_widget(asset_path, role)
        # [v1.5.0] ArmorBodyMap 의미 구조 검증에만 필요한 재사용 ArmorSector dependency입니다.
        sector_dependency = armor_sector_asset if role == "ArmorBodyMap" else None
        scaffold_or_validate_widget(
            asset_path,
            role,
            blueprint_asset,
            dependencies,
            visual_data,
            None,
            None,
            sector_dependency,
        )
        elements[role] = blueprint_asset

    # [v1.5.0] 여섯 Panel도 같은 원칙으로 신규만 Scaffold하고 저장된 Panel은 의미 구조만 검증합니다.
    panels: dict[str, Any] = {}
    for role, asset_path in PANEL_ASSETS.items():
        # [v1.5.0] 현재 Panel 역할의 신규/기존 Production Widget입니다.
        blueprint_asset = get_or_create_widget(asset_path, role)
        scaffold_or_validate_widget(
            asset_path,
            role,
            blueprint_asset,
            dependencies,
            visual_data,
            elements["SpeedGauge"],
            elements["ArmorBodyMap"],
            elements["ArmorSector"],
        )
        panels[role] = blueprint_asset

    # [v1.5.0] Production Root도 기존 Designer Layout을 유지하고 누락돼 이번 실행에서 생성된 경우에만 최초 Slot Scaffold를 수행합니다.
    root_asset = get_or_create_widget(ROOT_ASSET_PATH, "Root")
    scaffold_or_validate_root(root_asset, dependencies, panels)

    REPORT["readback"] = {
        "root_generated_class": read_generated_class_path(root_asset),
        "element_generated_classes": {
            role: read_generated_class_path(asset) for role, asset in elements.items()
        },
        "panel_generated_classes": {
            role: read_generated_class_path(asset) for role, asset in panels.items()
        },
        "layout_preserved_assets": sorted(set(REPORT["layout_preserved_assets"])),
    }
    finalize_contracts()
    REPORT["success"] = True



# [v1.5.0] 기존 WBP_CFWeaponPanel targeted 경로는 의미 구조 Validate-only로 동작하고 Designer Layout을 저장하거나 재구축하지 않습니다.
def run_weapon_panel_apply() -> None:
    # [v1.5.0] WeaponPanel 의미 구조 검증이 참조할 기존 Style/Density/Layout DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()
    # [v1.5.0] WeaponPanel 의미 Image 계약에 사용할 기존 HUD Visual DataAsset입니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)
    # [v1.5.0] Vehicle 계열 의존 시그니처를 만족할 기존 SpeedGauge Asset입니다.
    speed_gauge_asset = require_asset(ELEMENT_ASSETS["SpeedGauge"])
    # [v1.5.0] Vehicle 계열 의존 시그니처를 만족할 기존 ArmorBodyMap Asset입니다.
    armor_body_map_asset = require_asset(ELEMENT_ASSETS["ArmorBodyMap"])
    # [v1.5.0] 확장 Bridge 시그니처를 만족할 기존 ArmorSector Asset입니다.
    armor_sector_asset = require_asset(ELEMENT_ASSETS["ArmorSector"])
    # [v1.5.0] 신규 생성 없이 Validate-only로 보호할 현재 Production WeaponPanel 경로입니다.
    weapon_panel_path = PANEL_ASSETS["WeaponPanel"]
    # [v1.5.0] 저장된 Designer Layout을 그대로 유지할 기존 WeaponPanel Blueprint입니다.
    weapon_panel_asset = require_asset(weapon_panel_path)
    validate_widget_parent(weapon_panel_asset, "WeaponPanel")
    REPORT["reused_assets"].append(weapon_panel_path)

    scaffold_or_validate_widget(
        weapon_panel_path,
        "WeaponPanel",
        weapon_panel_asset,
        dependencies,
        visual_data,
        speed_gauge_asset,
        armor_body_map_asset,
        armor_sector_asset,
    )

    REPORT["readback"] = {
        "weapon_panel_generated_class": read_generated_class_path(weapon_panel_asset),
        "weapon_panel_asset_path": weapon_panel_path,
        "layout_preserved_assets": sorted(set(REPORT["layout_preserved_assets"])),
    }
    REPORT["contracts"] = {
        "operation_scope": "WeaponPanelValidateOnly",
        "exact_mutated_asset_count": len(REPORT["saved_assets"]),
        "exact_mutated_assets": sorted(REPORT["saved_assets"]),
        "other_production_asset_mutation_count": 0,
        "existing_widget_apply_mode": "ValidateOnly",
        "designer_layout_preserved": True,
        "stage_b_compact_resource_slots": True,
        "raw_resource_channel_direct_row_generation": False,
        "reserve_ammo_header_owner_preserved": True,
        "legacy_fixed_resource_rows_forbidden": True,
    }
    REPORT["success"] = True



# [v1.5.0] ArmorMap targeted 경로는 기존 ArmorSector/ArmorBodyMap Designer Layout을 Validate-only로 보호하고 누락 ArmorSector만 최초 Scaffold합니다.
def run_armor_map_apply() -> None:
    # [v1.5.0] Targeted Scaffold/Validate가 읽기 전용으로 사용할 Style/Density/Layout DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()
    # [v1.5.0] 이미 연결된 Vehicle/Armor Texture 의미 참조를 읽을 HUD Visual DataAsset입니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)

    # [v1.5.0] 누락 시에만 최초 Scaffold가 허용되는 재사용 ArmorSector 경로입니다.
    armor_sector_path = ELEMENT_ASSETS["ArmorSector"]
    # [v1.5.0] 신규면 Scaffold, 기존이면 Designer Layout Validate-only로 처리할 ArmorSector Asset입니다.
    armor_sector_asset = get_or_create_widget(armor_sector_path, "ArmorSector")
    scaffold_or_validate_widget(
        armor_sector_path,
        "ArmorSector",
        armor_sector_asset,
        dependencies,
        visual_data,
        None,
        None,
        None,
    )

    # [v1.5.0] 기존 사용자 배치를 절대 재구축하지 않고 재사용 Sector 6개 의미 구조만 검증할 ArmorBodyMap 경로입니다.
    armor_body_map_path = ELEMENT_ASSETS["ArmorBodyMap"]
    # [v1.5.0] 현재 저장된 ArmorBodyMap Designer Asset입니다.
    armor_body_map_asset = get_or_create_widget(armor_body_map_path, "ArmorBodyMap")
    scaffold_or_validate_widget(
        armor_body_map_path,
        "ArmorBodyMap",
        armor_body_map_asset,
        dependencies,
        visual_data,
        None,
        None,
        armor_sector_asset,
    )

    REPORT["readback"] = {
        "armor_sector_generated_class": read_generated_class_path(armor_sector_asset),
        "armor_body_map_generated_class": read_generated_class_path(armor_body_map_asset),
        "layout_preserved_assets": sorted(set(REPORT["layout_preserved_assets"])),
    }
    REPORT["contracts"] = {
        "operation_scope": "ArmorSectorAndBodyMapLayoutPreserving",
        "exact_mutated_asset_count": len(REPORT["saved_assets"]),
        "exact_mutated_assets": sorted(REPORT["saved_assets"]),
        "other_production_asset_mutation_count": 0,
        "existing_widget_apply_mode": "ValidateOnly",
        "designer_layout_preserved": True,
        "armor_sector_reuse_count": 6,
        "armor_body_map_direct_direction_widgets_forbidden": True,
        "defense_view_data_contract_changed": False,
    }
    REPORT["success"] = True



# [v1.5.0] RPM targeted 경로는 Texture/Material/VisualData만 갱신하고 기존 SpeedGauge Designer Tree는 Validate-only로 보호합니다.
def run_rpm_gauge_apply() -> None:
    # [v1.5.0] SpeedGauge 의미 구조 검증이 읽기 전용으로 참조할 Style/Density/Layout DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()

    # [v1.5.0] SourceArt의 데이터 채널을 선형 Texture2D로 reimport한 결과입니다.
    rpm_texture = import_rpm_track_texture()

    # [v1.5.0] Texture 데이터와 Presenter의 RPMRatio 하나를 소비할 RPM 전용 UI Material입니다.
    rpm_material = get_or_create_rpm_material()
    rebuild_rpm_material(rpm_material, rpm_texture)

    # [v1.5.0] 기존 Vehicle/Armor Visual Reference를 보존하면서 RPM Track/Material 참조만 갱신할 HUD Visual DataAsset입니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)
    if not isinstance(visual_data, require_unreal_type("CFHUDVisualData")):
        raise RuntimeError(f"RPM Gauge Visual asset has unexpected class: {VISUAL_ASSET_PATH}")
    REPORT["reused_assets"].append(VISUAL_ASSET_PATH)
    visual_data.set_editor_property("speed_arc_track", rpm_texture)
    visual_data.set_editor_property("speed_arc_material", rpm_material)

    # [v1.5.0] 현재 단일 Image_RPMGauge 의미 구조만 검증하고 위치·크기를 재구축하지 않을 기존 SpeedGauge Blueprint입니다.
    speed_gauge_path = ELEMENT_ASSETS["SpeedGauge"]
    # [v1.5.0] RPM targeted mode에서는 신규 생성이 금지되며 반드시 기존 Designer Asset을 재사용합니다.
    speed_gauge_asset = get_or_create_widget(speed_gauge_path, "SpeedGauge")
    scaffold_or_validate_widget(
        speed_gauge_path,
        "SpeedGauge",
        speed_gauge_asset,
        dependencies,
        visual_data,
        None,
        None,
        None,
    )

    # [v1.5.0] 실제 저장은 비-Designer 3개 Asset으로 제한하며 SpeedGauge는 저장하지 않습니다.
    save_asset(RPM_TRACK_ASSET_PATH, rpm_texture)
    save_asset(RPM_MATERIAL_ASSET_PATH, rpm_material)
    save_asset(VISUAL_ASSET_PATH, visual_data)

    REPORT["readback"] = {
        "rpm_track_asset": RPM_TRACK_ASSET_PATH,
        "rpm_material_asset": RPM_MATERIAL_ASSET_PATH,
        "speed_gauge_generated_class": read_generated_class_path(speed_gauge_asset),
        "rpm_ratio_parameter": "RPMRatio",
        "layout_preserved_assets": sorted(set(REPORT["layout_preserved_assets"])),
    }
    REPORT["contracts"] = {
        "operation_scope": "DynamicRpmGaugeLayoutPreserving",
        "exact_mutated_asset_count": len(REPORT["saved_assets"]),
        "exact_mutated_assets": sorted(REPORT["saved_assets"]),
        "other_production_asset_mutation_count": 0,
        "speed_gauge_apply_mode": "ValidateOnly",
        "designer_layout_preserved": True,
        "rpm_track_srgb": False,
        "rpm_track_data_channels": "R=Progress,G=Intensity,B=Redline,A=Coverage",
        "rpm_widget_image_count": 1,
        "legacy_rpm_progressbar_tick_count": 0,
        "runtime_parameter_count": 1,
        "runtime_parameter_name": "RPMRatio",
        "vehicle_panel_mutated": False,
                "armor_body_map_mutated": False,
    }
    REPORT["success"] = True


# [v1.6.0] Radar targeted 경로는 전용 Texture 7종과 VisualData, 기존 RadarPanel additive migration만 갱신합니다.
def run_radar_visual_apply() -> None:
    # [v1.6.0] RadarPanel Style/Caption과 Validator가 읽기 전용으로 참조할 Style/Density/Layout DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()

    # [v1.6.0] 기존 Vehicle/RPM 참조를 보존하면서 Radar 전용 참조 7개만 갱신할 HUD Visual DataAsset입니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)
    if not isinstance(visual_data, require_unreal_type("CFHUDVisualData")):
        raise RuntimeError(f"Radar Visual asset has unexpected class: {VISUAL_ASSET_PATH}")
    REPORT["reused_assets"].append(VISUAL_ASSET_PATH)

    # [v1.6.0] property 이름별 import 완료 Radar Texture 객체입니다.
    radar_textures: dict[str, Any] = {}
    for property_name, (asset_path, source_file_name) in RADAR_TEXTURE_SPECS.items():
        # [v1.6.0] 현재 property에 연결할 정확한 Radar Texture import 결과입니다.
        radar_texture = import_radar_texture(asset_path, source_file_name)
        radar_textures[property_name] = radar_texture
        visual_data.set_editor_property(property_name, radar_texture)

    if not bool(visual_data.has_complete_radar_presentation_art()):
        raise RuntimeError("Radar VisualData complete presentation contract failed after Texture binding")

    # [v1.6.0] 기존 Designer Layout을 보존하면서 새 Radar 의미 Widget만 additive migration할 정확한 Panel 경로입니다.
    radar_panel_path = PANEL_ASSETS["RadarPanel"]
    # [v1.6.0] 신규 생성 없이 반드시 재사용할 저장 Production RadarPanel Blueprint입니다.
    radar_panel_asset = require_asset(radar_panel_path)
    validate_widget_parent(radar_panel_asset, "RadarPanel")
    REPORT["reused_assets"].append(radar_panel_path)

    # [v1.6.0] 기존 Tree를 교체하지 않고 누락된 Frame/Range/Player/SelectedEdge만 추가하는 C++ Bridge입니다.
    bridge = require_bridge()
    if not bool(
        bridge.apply_radar_visual_migration_result(
            radar_panel_asset,
            dependencies["layout"],
            dependencies["style"],
            dependencies["standard_density"],
            dependencies["compact_density"],
            visual_data,
        )
    ):
        raise RuntimeError("RadarPanel layout-preserving visual migration failed")

    REPORT["layout_preserved_assets"].append(radar_panel_path)
    compile_widget(radar_panel_path, radar_panel_asset)

    if not call_widget_bridge(
        "validate_production_widget_result",
        radar_panel_asset,
        "RadarPanel",
        dependencies,
        visual_data,
        None,
        None,
        None,
    ):
        raise RuntimeError("RadarPanel post-migration Production validation failed")

    # [v1.6.0] 실제 저장은 Radar Texture 7개 + VisualData + RadarPanel 정확히 9개로 제한합니다.
    for _, (asset_path, _) in RADAR_TEXTURE_SPECS.items():
        save_asset(asset_path, radar_textures[next(name for name, spec in RADAR_TEXTURE_SPECS.items() if spec[0] == asset_path)])
    save_asset(VISUAL_ASSET_PATH, visual_data)
    save_asset(radar_panel_path, radar_panel_asset)

    REPORT["readback"] = {
        "radar_panel_generated_class": read_generated_class_path(radar_panel_asset),
        "radar_panel_asset_path": radar_panel_path,
        "radar_texture_assets": sorted(RADAR_TEXTURE_ASSET_PATHS),
        "complete_radar_presentation_art": bool(visual_data.has_complete_radar_presentation_art()),
        "layout_preserved_assets": sorted(set(REPORT["layout_preserved_assets"])),
    }
    REPORT["contracts"] = {
        "operation_scope": "RadarVisualLayoutPreservingAdditiveMigration",
        "exact_mutated_asset_count": len(REPORT["saved_assets"]),
        "exact_mutated_assets": sorted(REPORT["saved_assets"]),
        "expected_mutated_asset_count": 9,
        "other_production_asset_mutation_count": 0,
        "radar_texture_count": len(RADAR_TEXTURE_ASSET_PATHS),
        "radar_panel_tree_rebuilt": False,
        "existing_radar_widget_slot_layout_reapplied": False,
        "new_radar_semantic_widgets_additive_only": True,
        "runtime_contact_visual_type": "Image",
        "text_glyph_icon_count": 0,
        "sensor_runtime_mutated": False,
        "radar_sweep_material_required": False,
    }
    if len(REPORT["saved_assets"]) != 9:
        raise RuntimeError(f"Radar targeted save count mismatch: expected=9 actual={len(REPORT['saved_assets'])}")
    REPORT["success"] = True


# [v1.7.0] ViewMode targeted 경로는 기존 Production Root 한 개에 Vehicle Direction 의미 Widget만 additive migration합니다.
def run_view_mode_visual_apply() -> None:
    # [v1.7.0] Root 의미 구조와 Vehicle Semantic Icon을 읽을 기존 Style/Density/Layout DataAsset 묶음입니다.
    dependencies = load_read_only_dependencies()

    # [v1.7.0] 신규 생성 없이 반드시 재사용할 저장 Production Root Blueprint입니다.
    root_asset = require_asset(ROOT_ASSET_PATH)
    validate_widget_parent(root_asset, "Root")
    REPORT["reused_assets"].append(ROOT_ASSET_PATH)

    # [v1.7.0] 기존 Root Validator가 여섯 Panel Class 의미 구조를 재확인할 읽기 전용 Panel Asset 묶음입니다.
    panels = {role: require_asset(asset_path) for role, asset_path in PANEL_ASSETS.items()}
    for role, panel_asset in panels.items():
        validate_widget_parent(panel_asset, role)

    # [v1.7.0] Root Tree를 교체하지 않고 기존 ReticleLayer 내부 누락 Track/Image만 추가하는 C++ Bridge입니다.
    bridge = require_bridge()
    if not bool(bridge.apply_view_mode_visual_migration_result(root_asset, dependencies["style"])):
        raise RuntimeError("Production Root layout-preserving ViewMode visual migration failed")

    REPORT["layout_preserved_assets"].append(ROOT_ASSET_PATH)
    compile_widget(ROOT_ASSET_PATH, root_asset)
    if not call_root_bridge("validate_production_root_result", root_asset, dependencies, panels):
        raise RuntimeError("Production Root post-ViewMode migration validation failed")

    # [v1.7.0] 실제 저장은 기존 WBP_CFInGameHUD 정확히 1개 Asset으로 제한합니다.
    save_asset(ROOT_ASSET_PATH, root_asset)

    REPORT["readback"] = {
        "root_generated_class": read_generated_class_path(root_asset),
        "root_asset_path": ROOT_ASSET_PATH,
        "layout_preserved_assets": sorted(set(REPORT["layout_preserved_assets"])),
    }
    REPORT["contracts"] = {
        "operation_scope": "ViewModeVisualLayoutPreservingAdditiveMigration",
        "exact_mutated_asset_count": len(REPORT["saved_assets"]),
        "exact_mutated_assets": sorted(REPORT["saved_assets"]),
        "expected_mutated_asset_count": 1,
        "other_production_asset_mutation_count": 0,
        "root_tree_rebuilt": False,
        "existing_root_slot_layout_reapplied": False,
        "new_view_mode_semantic_widgets_additive_only": True,
        "vehicle_direction_visual_type": "Image",
        "semantic_icon_id": "Vehicle",
        "gameplay_camera_aim_mutated": False,
        "world_compass_created": False,
    }
    if len(REPORT["saved_assets"]) != 1:
        raise RuntimeError(f"ViewMode targeted save count mismatch: expected=1 actual={len(REPORT['saved_assets'])}")
    REPORT["success"] = True


# [v1.3.0] 저장된 Production 11개 Asset을 새 프로세스에서 수정 없이 다시 검증합니다.
def run_readback() -> None:
    # [v1.0.0] Saved Readback에 사용할 기존 읽기 전용 DataAsset입니다.
    dependencies = load_read_only_dependencies()
    # [v1.0.0] Saved Readback에서 실제 로드된 HUD Visual DataAsset입니다.
    visual_data = require_asset(VISUAL_ASSET_PATH)
    # [v1.3.0] Saved Readback에서 실제 로드된 세 의미 Element Asset입니다.
    elements = {role: require_asset(asset_path) for role, asset_path in ELEMENT_ASSETS.items()}
    # [v1.0.0] Saved Readback에서 실제 로드된 여섯 의미 Panel Asset입니다.
    panels = {role: require_asset(asset_path) for role, asset_path in PANEL_ASSETS.items()}
    # [v1.0.0] Saved Readback에서 실제 로드된 Production Root Asset입니다.
    root_asset = require_asset(ROOT_ASSET_PATH)

    validate_widget_parent(root_asset, "Root")
    for role, asset in elements.items():
        validate_widget_parent(asset, role)
    for role, asset in panels.items():
        validate_widget_parent(asset, role)

    for role, asset in elements.items():
        # [v1.3.0] ArmorBodyMap Validator에만 실제 ArmorSector Blueprint dependency를 전달합니다.
        armor_sector_dependency = elements["ArmorSector"] if role == "ArmorBodyMap" else None
        if not call_widget_bridge(
            "validate_production_widget_result",
            asset,
            role,
            dependencies,
            visual_data,
            None,
            None,
            armor_sector_dependency,
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
            elements["ArmorSector"],
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


# [v1.5.0] Production 의미 구조 Gate와 persisted Designer Layout Ownership 계약을 함께 기록합니다.
def finalize_contracts() -> None:
    # [v1.5.0] 현재 함수가 기록하는 의미 구조/Validator 계약이 모두 통과했음을 나타냅니다.
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
        "existing_widget_apply_mode": "ValidateOnly",
        "new_widget_creation_mode": "InitialScaffold",
        "designer_layout_owner": "PersistedUMGDesignerAsset",
        "layout_fields_reapplied_on_existing_asset": False,
        "layout_preserved_asset_count": len(set(REPORT["layout_preserved_assets"])),
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
    elif RUN_MODE == "armor_map_apply":
        run_armor_map_apply()
    elif RUN_MODE == "rpm_gauge_apply":
        run_rpm_gauge_apply()
    elif RUN_MODE == "radar_visual_apply":
        run_radar_visual_apply()
    elif RUN_MODE == "view_mode_visual_apply":
        run_view_mode_visual_apply()
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
