# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-07-29
# Description: CFSingleGameMode가 스폰하는 BP_CFVehiclePawn의 기본 VehicleData를 LM-P0-06 Launcher 검증 차량으로 연결합니다.
# Scope: /Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn의 CDO.VehicleData 하나만 변경합니다.
# Changelog:
# - v1.0.0: Dry Run, Blueprint CDO 진단·할당·저장과 JSON 보고서 추가.
# Migration:
# - GameMode, 맵 배치 Actor, DA_TestSedan과 VehicleData 원본 자산은 변경하지 않습니다.

from __future__ import annotations

import json
import os
import traceback
from pathlib import Path
from typing import Any

import unreal


TOOL_VERSION = "1.0.0"
APPLY_CHANGES = os.environ.get("CARFIGHT_LAUNCHER_PAWN_APPLY", "0") == "1"
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()
REPORT_DIR = PROJECT_DIR / "Saved" / "LauncherPawnApply"
REPORT_PATH = REPORT_DIR / "report.json"

BLUEPRINT_ASSET_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn"
BLUEPRINT_CLASS_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"
TARGET_VEHICLE_DATA_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"

REPORT: dict[str, Any] = {
    "schema_version": "launcher_pawn_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "blueprint_asset_path": BLUEPRINT_ASSET_PATH,
    "blueprint_class_path": BLUEPRINT_CLASS_PATH,
    "previous_vehicle_data": "",
    "planned_vehicle_data": TARGET_VEHICLE_DATA_PATH,
    "verified_vehicle_data": "",
    "saved_blueprint": False,
    "errors": [],
}


def log(message: str) -> None:
    unreal.log(f"[CarFight][LauncherPawn] {message}")


def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        return unreal.SystemLibrary.get_path_name(value)
    except Exception:
        return str(value)


def load_default_object() -> Any:
    pawn_class = unreal.load_class(None, BLUEPRINT_CLASS_PATH)
    if pawn_class is None:
        raise RuntimeError(f"Blueprint generated class load failed: {BLUEPRINT_CLASS_PATH}")
    default_object = unreal.get_default_object(pawn_class)
    if default_object is None:
        raise RuntimeError(f"Blueprint CDO load failed: {BLUEPRINT_CLASS_PATH}")
    return default_object


def read_vehicle_data(default_object: Any) -> Any:
    try:
        return default_object.get_editor_property("vehicle_data")
    except Exception as error:
        raise RuntimeError(f"CDO VehicleData read failed: {error}") from error


def write_vehicle_data(default_object: Any, vehicle_data: Any) -> None:
    try:
        default_object.set_editor_property("vehicle_data", vehicle_data)
    except Exception as error:
        raise RuntimeError(f"CDO VehicleData write failed: {error}") from error


def apply_default_vehicle_data() -> None:
    if not unreal.EditorAssetLibrary.does_asset_exist(BLUEPRINT_ASSET_PATH):
        raise RuntimeError(f"Blueprint asset is missing: {BLUEPRINT_ASSET_PATH}")
    if not unreal.EditorAssetLibrary.does_asset_exist(TARGET_VEHICLE_DATA_PATH):
        raise RuntimeError(f"VehicleData asset is missing: {TARGET_VEHICLE_DATA_PATH}")

    blueprint_asset = unreal.EditorAssetLibrary.load_asset(BLUEPRINT_ASSET_PATH)
    target_vehicle_data = unreal.EditorAssetLibrary.load_asset(TARGET_VEHICLE_DATA_PATH)
    if blueprint_asset is None or target_vehicle_data is None:
        raise RuntimeError("Required Blueprint or VehicleData load failed.")

    default_object = load_default_object()
    current_vehicle_data = read_vehicle_data(default_object)
    REPORT["previous_vehicle_data"] = object_path(current_vehicle_data)
    log(f"Previous VehicleData: {REPORT['previous_vehicle_data']}")
    log(f"Planned VehicleData: {TARGET_VEHICLE_DATA_PATH}")

    if not APPLY_CHANGES:
        REPORT["verified_vehicle_data"] = REPORT["previous_vehicle_data"]
        return

    write_vehicle_data(default_object, target_vehicle_data)
    updated_vehicle_data_path = object_path(read_vehicle_data(default_object))
    REPORT["verified_vehicle_data"] = updated_vehicle_data_path
    if TARGET_VEHICLE_DATA_PATH not in updated_vehicle_data_path:
        raise RuntimeError(
            f"CDO VehicleData assignment verification failed: {updated_vehicle_data_path}"
        )

    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint_asset, only_if_is_dirty=False):
        raise RuntimeError(f"Blueprint save failed: {BLUEPRINT_ASSET_PATH}")
    REPORT["saved_blueprint"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_default_vehicle_data()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][LauncherPawn] {error}")
finally:
    REPORT_PATH.write_text(json.dumps(REPORT, ensure_ascii=False, indent=2), encoding="utf-8")
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Launcher Pawn application failed. Read report.json for details.")
