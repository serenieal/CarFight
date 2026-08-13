# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.2.1
# Date: 2026-08-10
# Description: CF-FQ-032 D1-10B Base Widget Blueprint Probe/DryRun/Apply/Readback Unreal Python 도구
# Scope: 정확한 /Game/CarFight/UI/Base/ 5개 Widget Blueprint만 생성·컴파일·저장·읽기 검증합니다.
# Changelog:
# - v1.2.1: bool-only Bridge DryRun 노출 검증 블록의 Python 들여쓰기를 정규화.
# - v1.2.0: UE 5.8 Python Out FString 마샬링을 우회하는 CFUIBaseEditorBridge bool-only Build/Validate 래퍼를 정식 사용.
# - v1.1.3: CFUIBaseEditorBridge의 Unreal Python 실제 반환형/순서를 구조화 report와 로그에 기록해 Bool/Out FString 마샬링을 진단.
# - v1.1.2: v1.1.1 Class-wrapper 보정 블록의 Python 들여쓰기를 정규화.
# - v1.1.1: Unreal Python 타입 래퍼와 실제 UClass를 분리해 Parent 비교·Factory 입력·DryRun 경로 표시를 교정.
# - v1.1.0: Probe 전용 경로를 DryRun/Apply/Readback까지 완성하고 CFUIBaseEditorBridge 기반 정확한 WidgetTree 검증을 추가.
# - v1.0.2: unreal namespace에서 Widget/UMG/Blueprint Editor Subsystem·Library 후보를 추가 탐색.
# - v1.0.1: 기존 WBP_TargetSelect에서 widget_tree/root_widget property 접근과 PanelWidget add_child 노출 여부 Probe 추가.
# - v1.0.0: D1-10B 구현 전 실제 UE 5.8 Python Reflection Probe를 최초 추가.
# Migration:
# - Probe/DryRun/Readback은 Content Asset을 생성·저장하지 않습니다.
# - Apply는 아래 정확한 5개 경로만 변경할 수 있으며 기존 다른 Asset은 수정하지 않습니다.
# - 대상 Asset이 이미 존재하면 정확한 Native Parent와 WidgetTree 계약을 먼저 검증하고 일치할 때만 재사용합니다.

from __future__ import annotations

import json
import os
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.1.0] 구조화 결과에서 식별할 현재 도구 버전입니다.
TOOL_VERSION = "1.2.1"

# [v1.1.0] 실행할 Probe/DryRun/Apply/Readback 모드를 선택하는 환경 변수입니다.
RUN_MODE = os.environ.get("CARFIGHT_UI_BASE_MODE", "probe").strip().lower()

# [v1.1.0] 현재 Unreal 프로젝트 루트의 절대 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.1.0] D1-10B 구조화 결과를 저장할 Saved 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "UIBaseWidgets"

# [v1.1.0] MCP가 읽을 구조화 결과 JSON 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.1.0] D1-10B에서 생성이 허용된 유일한 Unreal Asset 폴더입니다.
BASE_WIDGET_FOLDER = "/Game/CarFight/UI/Base"

# [v1.1.0] 각 Base Widget의 경로·Native Parent·Bridge Template·정확한 Tree 계약입니다.
WIDGET_SPECS: tuple[dict[str, str], ...] = (
    {
        "asset_name": "WBP_CFButtonBase",
        "asset_path": "/Game/CarFight/UI/Base/WBP_CFButtonBase",
        "parent_type": "CFButtonBaseWidget",
        "template_id": "Button",
        "tree": "SizeBox_Root/Button_Interaction/Overlay_Visual/[Border_Background,Border_FocusOutline,HorizontalBox_Content/[Image_Icon,Text_Label,Spacer_Fill,Text_InputHint],Border_StateMarker]",
    },
    {
        "asset_name": "WBP_CFPanelBase",
        "asset_path": "/Game/CarFight/UI/Base/WBP_CFPanelBase",
        "parent_type": "CFStyledWidgetBase",
        "template_id": "Panel",
        "tree": "Overlay_Root/[Border_Background,Border_Outline,Border_AccentMarker,VerticalBox_Layout/[SizeBox_Header/HorizontalBox_Header/[Image_HeaderIcon,Text_Header,NamedSlot_HeaderExtra],NamedSlot_Content]]",
    },
    {
        "asset_name": "WBP_CFStatusBar",
        "asset_path": "/Game/CarFight/UI/Base/WBP_CFStatusBar",
        "parent_type": "CFStyledWidgetBase",
        "template_id": "StatusBar",
        "tree": "SizeBox_Root/Overlay_Bar/[Border_Track,ProgressBar_Continuous,HorizontalBox_ArmorSegments,Text_PrimaryValue,Text_SecondaryValue]",
    },
    {
        "asset_name": "WBP_CFInfoRow",
        "asset_path": "/Game/CarFight/UI/Base/WBP_CFInfoRow",
        "parent_type": "CFStyledWidgetBase",
        "template_id": "InfoRow",
        "tree": "HorizontalBox_Root/[SizeBox_Label/Text_Label,SizeBox_Value/Text_Value,Spacer_Fill,Image_StateIcon]",
    },
    {
        "asset_name": "WBP_CFAlertItem",
        "asset_path": "/Game/CarFight/UI/Base/WBP_CFAlertItem",
        "parent_type": "CFStyledWidgetBase",
        "template_id": "AlertItem",
        "tree": "SizeBox_Root/Overlay_Root/[Border_Background,Border_AccentLine,HorizontalBox_Content/[Image_Severity,Text_Display]]",
    },
)

# [v1.1.0] Apply가 변경할 수 있는 정확한 Unreal Asset 경로 화이트리스트입니다.
MUTABLE_ASSET_PATHS = {spec["asset_path"] for spec in WIDGET_SPECS}

# [v1.1.0] 실행 결과와 검증 Evidence를 보존할 보고서 객체입니다.
REPORT: dict[str, Any] = {
    "schema_version": "carfight_ui_base_widgets_v2",
    "tool_version": TOOL_VERSION,
    "mode": RUN_MODE,
    "success": False,
    "base_widget_folder": BASE_WIDGET_FOLDER,
    "target_assets": [spec["asset_path"] for spec in WIDGET_SPECS],
    "created_assets": [],
    "reused_assets": [],
    "compiled_assets": [],
    "saved_assets": [],
    "dry_run": {},
    "readback": {},
        "contracts": {},
    "probe": {},
    "bridge_calls": [],
    "errors": [],
}


# [v1.1.0] Unreal Output Log에 D1-10B 도구 메시지를 남깁니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][UIBaseWidgets] {message}")


# [v1.1.0] Python에 노출된 객체의 공개 멤버 이름을 정렬해 반환합니다.
def public_members(value: Any) -> list[str]:
    return sorted(name for name in dir(value) if not name.startswith("_"))


# [v1.1.0] UObject/UStruct에서 후보 Editor Property를 안전하게 읽고 실제 노출 여부를 기록합니다.
def probe_editor_properties(instance: Any, property_names: list[str]) -> dict[str, Any]:
    # [v1.1.0] 각 후보 Property의 읽기 성공 여부와 실제 Python 타입을 담을 결과입니다.
    result: dict[str, Any] = {}
    for property_name in property_names:
        try:
            # [v1.1.0] 현재 후보 Editor Property에서 실제로 읽힌 값입니다.
            value = instance.get_editor_property(property_name)
            result[property_name] = {
                "available": True,
                "python_type": type(value).__name__,
                "value": str(value),
            }
        except Exception as error:
            result[property_name] = {
                "available": False,
                "error": str(error),
            }
    return result


# [v1.1.0] 이름으로 Unreal Python 타입을 가져오고 누락 시 즉시 실패합니다.
def require_unreal_type(type_name: str) -> Any:
    # [v1.1.0] 현재 이름으로 Python에 노출된 Unreal 타입입니다.
    unreal_type = getattr(unreal, type_name, None)
    if unreal_type is None:
        raise RuntimeError(f"Required Unreal Python type is unavailable: {type_name}")
    return unreal_type


# [v1.1.0] 지정 Asset 경로를 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    # [v1.1.0] Editor Asset Library가 실제로 로드한 대상 Asset입니다.
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return asset


# [v1.1.0] Blueprint의 현재 Native Parent Class를 UE 5.8 공개 API로 읽습니다.
def get_blueprint_parent_class(blueprint_asset: Any) -> Any:
    # [v1.1.0] Blueprint Parent Class 읽기에 사용할 UE 5.8 Blueprint Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    # [v1.1.0] 현재 Blueprint가 실제로 상속하는 Parent Class입니다.
    parent_class = blueprint_editor_library.get_blueprint_parent_class(blueprint_asset)
    if parent_class is None:
        raise RuntimeError(f"Blueprint parent class is unavailable: {blueprint_asset.get_path_name()}")
    return parent_class


# [v1.1.0] Blueprint의 현재 Compile Status를 변경 없이 읽어 문자열로 반환합니다.
def read_blueprint_status(blueprint_asset: Any) -> str:
    try:
        # [v1.1.0] WidgetBlueprint Python 표면의 status 멤버 또는 호출 결과입니다.
        status_member = getattr(blueprint_asset, "status", None)
        # [v1.1.0] status가 함수형으로 노출된 경우 호출한 실제 값입니다.
        status_value = status_member() if callable(status_member) else status_member
        return str(status_value)
    except Exception as error:
        return f"Unavailable: {error}"


# [v1.1.0] Blueprint Generated Class가 존재하는지 확인하고 Object Path를 반환합니다.
def read_generated_class_path(blueprint_asset: Any) -> str:
    try:
        # [v1.1.0] 현재 Blueprint가 컴파일해 생성한 실제 Generated Class입니다.
        generated_class = blueprint_asset.generated_class()
        return generated_class.get_path_name() if generated_class is not None else ""
    except Exception:
        return ""


# [v1.1.0] CFUIBaseEditorBridge의 Bool + OutFailureReason 반환 형식을 공통 처리합니다.
def call_editor_bridge(method_name: str, blueprint_asset: Any, template_id: str) -> tuple[bool, str]:
    # [v1.1.0] Python에 노출된 D1-10B C++ Editor Bridge 타입입니다.
    bridge_type = require_unreal_type("CFUIBaseEditorBridge")
    # [v1.1.0] 호출할 Build/Validate 정적 Bridge 함수입니다.
    bridge_method = getattr(bridge_type, method_name, None)
    if not callable(bridge_method):
        raise RuntimeError(f"CFUIBaseEditorBridge method is unavailable: {method_name}")

        # [v1.1.0] UFUNCTION의 ReturnValue와 OutFailureReason이 Python으로 변환된 실제 반환값입니다.
    result = bridge_method(blueprint_asset, template_id)
    # [v1.1.3] Unreal Python의 실제 Bool/Out Parameter 마샬링 순서를 판별할 진단 Evidence입니다.
    bridge_call_evidence = {
        "method": method_name,
        "asset_path": blueprint_asset.get_path_name(),
        "template_id": template_id,
        "result_type": type(result).__name__,
        "result_repr": repr(result),
    }
    REPORT["bridge_calls"].append(bridge_call_evidence)
    log(f"BridgeResult method={method_name} template={template_id} type={type(result).__name__} repr={result!r}")

    if isinstance(result, tuple):
        # [v1.1.3] 진단 단계에서는 기존 해석을 유지하고 실제 tuple 순서를 report로 확정합니다.
        success = bool(result[0]) if len(result) >= 1 else False
        # [v1.1.3] 기존 해석 기준 두 번째 항목을 FailureReason 후보로 기록합니다.
        failure_reason = str(result[1]) if len(result) >= 2 and result[1] else ""
        return success, failure_reason

    return bool(result), ""


# [v1.1.0] Base Widget Blueprint의 Native Parent와 정확한 WidgetTree/Graph/Spacer 계약을 읽기 전용으로 검증합니다.
def validate_widget_asset(spec: dict[str, str], blueprint_asset: Any) -> dict[str, Any]:
    # [v1.1.0] 이 Asset이 반드시 정확히 상속해야 하는 Native Parent Python 타입입니다.
    expected_parent_type = require_unreal_type(spec["parent_type"])
    # [v1.1.1] Unreal Python 타입 래퍼에서 얻은 실제 Native UClass입니다.
    expected_parent_class = expected_parent_type.static_class()
    # [v1.1.0] 현재 Blueprint에서 실제로 읽은 Native Parent Class입니다.
    actual_parent_class = get_blueprint_parent_class(blueprint_asset)
    # [v1.1.0] C++ Bridge가 실제 WidgetTree와 Graph 안전성을 검증한 결과입니다.
    tree_valid, failure_reason = call_editor_bridge(
                "validate_base_widget_tree_result",
        blueprint_asset,
        spec["template_id"],
    )
    # [v1.1.0] 정확한 Native Parent Class 일치 여부입니다.
    parent_valid = actual_parent_class == expected_parent_class
    # [v1.1.0] Style/Density 외부 전달 API가 Native Parent 계층에 실제 노출됐는지 여부입니다.
    visual_context_api_available = hasattr(expected_parent_type, "set_ui_visual_context")

    return {
        "asset_path": spec["asset_path"],
        "asset_class": blueprint_asset.get_class().get_name(),
        "expected_parent": expected_parent_class.get_path_name(),
        "actual_parent": actual_parent_class.get_path_name(),
        "parent_valid": parent_valid,
        "template_id": spec["template_id"],
        "expected_tree": spec["tree"],
        "exact_tree_and_safety_valid": tree_valid,
        "bridge_failure_reason": failure_reason,
        "visual_context_api_available": visual_context_api_available,
        "blueprint_status": read_blueprint_status(blueprint_asset),
        "generated_class": read_generated_class_path(blueprint_asset),
    }


# [v1.1.0] Apply 전용으로 /Game/CarFight/UI/Base 폴더를 생성합니다.
def ensure_base_widget_folder() -> None:
    if unreal.EditorAssetLibrary.does_directory_exist(BASE_WIDGET_FOLDER):
        return
    if RUN_MODE != "apply":
        return
    if not unreal.EditorAssetLibrary.make_directory(BASE_WIDGET_FOLDER):
        raise RuntimeError(f"Asset folder creation failed: {BASE_WIDGET_FOLDER}")


# [v1.1.0] 정확한 화이트리스트 경로에 Native Parent 기반 Widget Blueprint를 새로 생성합니다.
def create_widget_blueprint(spec: dict[str, str]) -> Any:
    if RUN_MODE != "apply":
        raise RuntimeError("Widget Blueprint creation is allowed only in apply mode")
    if spec["asset_path"] not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is outside D1-10B allowlist: {spec['asset_path']}")

        # [v1.1.1] 새 Widget Blueprint가 정확히 상속할 Native Parent Python 타입입니다.
    parent_type = require_unreal_type(spec["parent_type"])
    # [v1.1.1] WidgetBlueprintFactory.parent_class에 전달할 실제 Native UClass입니다.
    parent_class = parent_type.static_class()
    # [v1.1.0] UE 5.8 Widget Blueprint 생성에 사용할 Factory입니다.
    widget_factory = require_unreal_type("WidgetBlueprintFactory")()
    widget_factory.set_editor_property("parent_class", parent_class)

    # [v1.1.0] UE AssetTools가 생성한 실제 Widget Blueprint Asset입니다.
    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        spec["asset_name"],
        BASE_WIDGET_FOLDER,
        require_unreal_type("WidgetBlueprint"),
        widget_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"Widget Blueprint creation failed: {spec['asset_path']}")

    # [v1.1.0] C++ Bridge가 새 Blueprint에 정확한 D1-10B Designer Tree를 생성한 결과입니다.
    built, failure_reason = call_editor_bridge(
                "build_base_widget_tree_result",
        created_asset,
        spec["template_id"],
    )
    if not built:
        raise RuntimeError(
            f"WidgetTree build failed: {spec['asset_path']} reason={failure_reason}"
        )

    REPORT["created_assets"].append(spec["asset_path"])
    return created_asset


# [v1.1.0] Blueprint를 UE 5.8 공식 Editor Library로 컴파일하고 Error 상태 또는 Generated Class 누락을 거부합니다.
def compile_widget_blueprint(spec: dict[str, str], blueprint_asset: Any) -> dict[str, Any]:
    # [v1.1.0] Blueprint 컴파일에 사용할 UE 5.8 Blueprint Editor Library입니다.
    blueprint_editor_library = require_unreal_type("BlueprintEditorLibrary")
    blueprint_editor_library.compile_blueprint(blueprint_asset)

    # [v1.1.0] Compile 직후 읽은 Blueprint Status 문자열입니다.
    status_text = read_blueprint_status(blueprint_asset)
    # [v1.1.0] Compile 직후 생성된 Blueprint Class Object Path입니다.
    generated_class_path = read_generated_class_path(blueprint_asset)
    if "ERROR" in status_text.upper():
        raise RuntimeError(f"Blueprint compile status is Error: {spec['asset_path']} status={status_text}")
    if not generated_class_path:
        raise RuntimeError(f"Blueprint generated class is missing after compile: {spec['asset_path']}")

    REPORT["compiled_assets"].append(spec["asset_path"])
    return {
        "asset_path": spec["asset_path"],
        "status": status_text,
        "generated_class": generated_class_path,
        "success": True,
    }


# [v1.1.0] Apply 화이트리스트를 확인한 뒤 Widget Blueprint를 강제로 저장합니다.
def save_widget_blueprint(spec: dict[str, str], blueprint_asset: Any) -> None:
    if RUN_MODE != "apply":
        return
    if spec["asset_path"] not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is outside D1-10B save allowlist: {spec['asset_path']}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint_asset, only_if_is_dirty=False):
        raise RuntimeError(f"Widget Blueprint save failed: {spec['asset_path']}")
    REPORT["saved_assets"].append(spec["asset_path"])


# [v1.1.0] 5개 Base Widget을 모두 다시 로드해 Parent·정확한 Tree·Graph·Spacer 계약을 검증합니다.
def run_readback(require_all_assets: bool = True) -> dict[str, Any]:
    # [v1.1.0] Asset별 실제 Readback 결과입니다.
    asset_results: dict[str, Any] = {}
    for spec in WIDGET_SPECS:
        # [v1.1.0] 현재 목표 Asset의 존재 여부입니다.
        exists = unreal.EditorAssetLibrary.does_asset_exist(spec["asset_path"])
        if not exists:
            if require_all_assets:
                raise RuntimeError(f"D1-10B readback asset is missing: {spec['asset_path']}")
            asset_results[spec["asset_path"]] = {"exists": False}
            continue

        # [v1.1.0] 실제 저장된 Widget Blueprint Asset입니다.
        blueprint_asset = require_asset(spec["asset_path"])
        # [v1.1.0] C++ Bridge와 Native Parent를 사용한 정확한 Readback 결과입니다.
        validation = validate_widget_asset(spec, blueprint_asset)
        validation["exists"] = True
        if not validation["parent_valid"]:
            raise RuntimeError(
                f"D1-10B parent readback failed: {spec['asset_path']} actual={validation['actual_parent']} expected={validation['expected_parent']}"
            )
        if not validation["exact_tree_and_safety_valid"]:
            raise RuntimeError(
                f"D1-10B exact WidgetTree readback failed: {spec['asset_path']} reason={validation['bridge_failure_reason']}"
            )
        if not validation["visual_context_api_available"]:
            raise RuntimeError(f"D1-10B Style/Density context API missing: {spec['asset_path']}")
        asset_results[spec["asset_path"]] = validation

    return asset_results


# [v1.1.0] D1-10B 생성에 필요한 UE 5.8 Widget Blueprint Editor Python 표면을 읽기 전용으로 수집합니다.
def run_probe() -> None:
    # [v1.1.0] 존재 여부와 공개 멤버를 확인할 Unreal Python 타입 이름입니다.
    type_names = [
        "WidgetBlueprint",
        "WidgetBlueprintFactory",
        "BlueprintEditorLibrary",
        "CFStyledWidgetBase",
        "CFButtonBaseWidget",
        "CFUIBaseEditorBridge",
    ]

    # [v1.1.0] 타입별 존재 여부와 Python 공개 멤버 목록입니다.
    type_probe: dict[str, Any] = {}
    for type_name in type_names:
        # [v1.1.0] 현재 이름으로 Python에 실제 노출된 Unreal 타입입니다.
        unreal_type = getattr(unreal, type_name, None)
        type_probe[type_name] = {
            "available": unreal_type is not None,
            "members": public_members(unreal_type) if unreal_type is not None else [],
        }

    # [v1.1.0] WidgetBlueprintFactory의 실제 부모 Class 지정 Property를 확인할 Probe입니다.
    factory_probe: dict[str, Any] = {}
    # [v1.1.0] UE 5.8 WidgetBlueprintFactory Python 타입입니다.
    widget_blueprint_factory_type = getattr(unreal, "WidgetBlueprintFactory", None)
    if widget_blueprint_factory_type is not None:
        try:
            # [v1.1.0] Asset을 만들지 않고 Property만 조사할 Transient WidgetBlueprintFactory입니다.
            widget_blueprint_factory = widget_blueprint_factory_type()
            factory_probe = {
                "properties": probe_editor_properties(
                    widget_blueprint_factory,
                    ["parent_class", "supported_class", "edit_after_new", "create_new"],
                )
            }
        except Exception as error:
            factory_probe = {"construct_error": str(error)}

    REPORT["probe"] = {
        "types": type_probe,
        "widget_blueprint_factory": factory_probe,
        "base_folder_exists": unreal.EditorAssetLibrary.does_directory_exist(BASE_WIDGET_FOLDER),
        "target_exists": {
            spec["asset_path"]: unreal.EditorAssetLibrary.does_asset_exist(spec["asset_path"])
            for spec in WIDGET_SPECS
        },
    }
    REPORT["success"] = all(entry["available"] for entry in type_probe.values())
    if not REPORT["success"]:
        raise RuntimeError("One or more D1-10B Unreal Python/Bridge types are unavailable")


# [v1.1.0] Content를 수정하지 않고 정확한 5개 Target·Parent·Bridge·기존 Asset 안전성을 검증합니다.
def run_dry_run() -> None:
    # [v1.1.0] DryRun에서 확인한 Target별 현재 존재/검증 상태입니다.
    target_state: dict[str, Any] = {}
    for spec in WIDGET_SPECS:
        # [v1.1.0] Target Native Parent Type이 현재 Python에 노출되어 있는지 확인한 타입입니다.
        parent_type = require_unreal_type(spec["parent_type"])
        # [v1.1.1] DryRun 보고서에 기록할 실제 Native UClass입니다.
        parent_class = parent_type.static_class()
        # [v1.1.0] 현재 Target Asset의 존재 여부입니다.
        exists = unreal.EditorAssetLibrary.does_asset_exist(spec["asset_path"])
        # [v1.1.0] 존재하는 Target은 보호를 위해 변경 전 정확 계약을 검증한 결과입니다.
        validation = None
        if exists:
            validation = validate_widget_asset(spec, require_asset(spec["asset_path"]))
            if not validation["parent_valid"] or not validation["exact_tree_and_safety_valid"]:
                raise RuntimeError(f"Existing D1-10B target is not safe to reuse: {spec['asset_path']}")

        target_state[spec["asset_path"]] = {
            "exists": exists,
            "action": "reuse_exact" if exists else "create",
            "parent_type": parent_class.get_path_name(),
            "template_id": spec["template_id"],
            "expected_tree": spec["tree"],
            "existing_validation": validation,
        }

        # [v1.1.0] C++ Editor Bridge가 실제로 노출한 Build 함수입니다.
    bridge_type = require_unreal_type("CFUIBaseEditorBridge")
    if not callable(getattr(bridge_type, "build_base_widget_tree_result", None)):
        raise RuntimeError("CFUIBaseEditorBridge.build_base_widget_tree_result is unavailable")
    if not callable(getattr(bridge_type, "validate_base_widget_tree_result", None)):
        raise RuntimeError("CFUIBaseEditorBridge.validate_base_widget_tree_result is unavailable")

    REPORT["dry_run"] = {
        "target_count": len(WIDGET_SPECS),
        "mutable_asset_count": len(MUTABLE_ASSET_PATHS),
        "base_folder_exists": unreal.EditorAssetLibrary.does_directory_exist(BASE_WIDGET_FOLDER),
        "targets": target_state,
        "will_create_only": sorted(
            spec["asset_path"]
            for spec in WIDGET_SPECS
            if not unreal.EditorAssetLibrary.does_asset_exist(spec["asset_path"])
        ),
    }
    REPORT["success"] = True


# [v1.1.0] 정확한 5개 Widget Blueprint만 생성/재사용하고 Tree 생성·5/5 Compile·저장·Readback을 수행합니다.
def run_apply() -> None:
    ensure_base_widget_folder()

    # [v1.1.0] Apply에서 최종 Compile할 5개 Widget Blueprint 객체입니다.
    blueprint_assets: dict[str, Any] = {}
    for spec in WIDGET_SPECS:
        # [v1.1.0] 현재 Target Asset의 기존 존재 여부입니다.
        exists = unreal.EditorAssetLibrary.does_asset_exist(spec["asset_path"])
        if exists:
            # [v1.1.0] 기존 Target을 덮어쓰지 않고 정확 계약을 먼저 검증한 결과입니다.
            existing_asset = require_asset(spec["asset_path"])
            existing_validation = validate_widget_asset(spec, existing_asset)
            if not existing_validation["parent_valid"] or not existing_validation["exact_tree_and_safety_valid"]:
                raise RuntimeError(f"Existing D1-10B target differs from exact contract: {spec['asset_path']}")
            REPORT["reused_assets"].append(spec["asset_path"])
            blueprint_assets[spec["asset_path"]] = existing_asset
            continue

        # [v1.1.0] 화이트리스트에 따라 새로 생성한 Widget Blueprint입니다.
        blueprint_assets[spec["asset_path"]] = create_widget_blueprint(spec)

    # [v1.1.0] 5/5 Compile 결과를 보존하는 구조화 목록입니다.
    compile_results: list[dict[str, Any]] = []
    for spec in WIDGET_SPECS:
        # [v1.1.0] 현재 Target 경로에 대응하는 실제 Widget Blueprint 객체입니다.
        blueprint_asset = blueprint_assets[spec["asset_path"]]
        compile_results.append(compile_widget_blueprint(spec, blueprint_asset))

        # [v1.1.0] Compile 직후 정확한 Tree·Native Parent·Bind·Graph 계약 검증 결과입니다.
        validation = validate_widget_asset(spec, blueprint_asset)
        if not validation["parent_valid"] or not validation["exact_tree_and_safety_valid"]:
            raise RuntimeError(
                f"Post-compile D1-10B contract failed: {spec['asset_path']} reason={validation['bridge_failure_reason']}"
            )
        save_widget_blueprint(spec, blueprint_asset)

    REPORT["compile_results"] = compile_results
    REPORT["readback"] = run_readback(require_all_assets=True)
    finalize_contracts()
    REPORT["success"] = True


# [v1.1.0] 저장된 5개 Widget Blueprint를 변경 없이 재검증하고 D1-10 종료 계약을 구조화합니다.
def run_readback_mode() -> None:
    REPORT["readback"] = run_readback(require_all_assets=True)
    finalize_contracts()
    REPORT["success"] = True


# [v1.1.0] Readback Evidence에서 D1-10 요구사항별 PASS/0-count 계약을 계산합니다.
def finalize_contracts() -> None:
    # [v1.1.0] 5개 Target 모두의 Readback 결과 목록입니다.
    readback_values = list(REPORT["readback"].values())
    # [v1.1.0] 정확히 Button Base 하나의 Readback 결과입니다.
    button_result = REPORT["readback"].get("/Game/CarFight/UI/Base/WBP_CFButtonBase", {})
    # [v1.1.0] Apply 실행이면 5개 모두 Compile됐는지, Readback이면 Generated Class가 모두 존재하는지 판정한 Compile 계약입니다.
    compile_pass = (
        len(REPORT["compiled_assets"]) == len(WIDGET_SPECS)
        if RUN_MODE == "apply"
        else all(bool(entry.get("generated_class")) for entry in readback_values)
    )
    # [v1.1.0] 5개 모두 Native Parent를 통해 외부 Style/Density Context API를 상속하는지 여부입니다.
    visual_context_pass = all(bool(entry.get("visual_context_api_available")) for entry in readback_values)
    # [v1.1.0] C++ Bridge의 exact validation이 5개 모두 통과했는지 여부입니다.
    exact_validation_pass = all(bool(entry.get("exact_tree_and_safety_valid")) for entry in readback_values)

    REPORT["contracts"] = {
        "target_count": len(WIDGET_SPECS),
        "exact_five_assets_present": len(readback_values) == len(WIDGET_SPECS),
        "compile_5_of_5_pass": compile_pass,
        "exact_widget_tree_readback_5_of_5_pass": exact_validation_pass,
        "button_interaction_bind_pass": bool(button_result.get("exact_tree_and_safety_valid")),
        "style_density_external_context_pass": visual_context_pass,
        "gameplay_cast_count": 0 if exact_validation_pass else None,
        "direct_data_asset_load_count": 0 if exact_validation_pass else None,
        "collapsed_spacer_residue_count": 0 if exact_validation_pass else None,
        "graph_call_function_count": 0 if exact_validation_pass else None,
        "note": "CFUIBaseEditorBridge validation is stricter than the D1-10 graph contract: any DynamicCast or CallFunction node and any Collapsed Spacer causes failure; Button template additionally validates the native BindWidget contract.",
    }

    if not all(
        [
            REPORT["contracts"]["exact_five_assets_present"],
            REPORT["contracts"]["compile_5_of_5_pass"],
            REPORT["contracts"]["exact_widget_tree_readback_5_of_5_pass"],
            REPORT["contracts"]["button_interaction_bind_pass"],
            REPORT["contracts"]["style_density_external_context_pass"],
            REPORT["contracts"]["gameplay_cast_count"] == 0,
            REPORT["contracts"]["direct_data_asset_load_count"] == 0,
            REPORT["contracts"]["collapsed_spacer_residue_count"] == 0,
        ]
    ):
        raise RuntimeError("One or more D1-10B closure contracts failed")


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
        run_readback_mode()
    else:
        raise RuntimeError(f"Unsupported D1-10B run mode: {RUN_MODE}")
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][UIBaseWidgets] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("UI Base Widget operation failed. Read report.json for details.")
