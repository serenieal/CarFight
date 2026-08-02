# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.2.0
# Date: 2026-07-29
# Description: LM-P0-05 TestMap 플레이 차량 인스턴스를 Launcher 검증용 DA_TestSUV로 연결합니다.
# Scope: /Game/Maps/TestMap의 BP_CFVehiclePawn_C_1.VehicleData 하나만 변경합니다.
# Changelog:
# - v1.2.0: CFSingleGameMode가 스폰하는 BP_CFVehiclePawn CDO의 VehicleData 진단을 추가.
# - v1.1.0: EditorActorSubsystem·LevelEditorSubsystem으로 전환하고 맵 차량 Actor의 VehicleData·AutoPossessPlayer 진단을 추가.
# - v1.0.0: Dry Run, 단일 Actor 검증, VehicleData 할당, 맵 저장과 JSON 보고서 추가.
# Migration:
# - BP_CFVehiclePawn 클래스 기본값, DA_TestSedan과 다른 맵 Actor는 변경하지 않습니다.

from __future__ import annotations

import json
import os
import traceback
from pathlib import Path
from typing import Any

import unreal


TOOL_VERSION = "1.2.0"
APPLY_CHANGES = os.environ.get("CARFIGHT_LAUNCHER_MAP_APPLY", "0") == "1"
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()
REPORT_DIR = PROJECT_DIR / "Saved" / "LauncherMapApply"
REPORT_PATH = REPORT_DIR / "report.json"

MAP_PATH = "/Game/Maps/TestMap"
TARGET_ACTOR_NAME = "BP_CFVehiclePawn_C_1"
TARGET_ACTOR_CLASS_NAME = "BP_CFVehiclePawn_C"
TARGET_VEHICLE_DATA_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"
DEFAULT_PAWN_CLASS_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"

REPORT: dict[str, Any] = {
    "schema_version": "launcher_map_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "map_path": MAP_PATH,
    "target_actor_name": TARGET_ACTOR_NAME,
    "target_actor_class_name": TARGET_ACTOR_CLASS_NAME,
    "previous_vehicle_data": "",
    "planned_vehicle_data": TARGET_VEHICLE_DATA_PATH,
            "saved_map": False,
    "default_pawn_class_path": DEFAULT_PAWN_CLASS_PATH,
    "default_pawn_vehicle_data": "",
    "vehicle_actors": [],
    "errors": [],
}


def log(message: str) -> None:
    unreal.log(f"[CarFight][LauncherMap] {message}")


def get_property(target: Any, property_name: str) -> Any:
    candidates = [property_name, "vehicle_data"]
    last_error: Exception | None = None
    for candidate in candidates:
        try:
            return target.get_editor_property(candidate)
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property read failed: {property_name}: {last_error}")


def set_property(target: Any, property_name: str, value: Any) -> None:
    candidates = [property_name, "vehicle_data"]
    last_error: Exception | None = None
    for candidate in candidates:
        try:
            target.set_editor_property(candidate, value)
            return
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property write failed: {property_name}: {last_error}")


def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        return unreal.SystemLibrary.get_path_name(value)
    except Exception:
        return str(value)


def load_target_map() -> None:
    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if loaded_world is None:
        raise RuntimeError(f"Map load failed: {MAP_PATH}")


def find_target_actor() -> Any:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    actors = actor_subsystem.get_all_level_actors()
    matches = []
    for actor in actors:
        if actor is None:
            continue
        actor_name = actor.get_name()
        actor_class = actor.get_class()
        actor_class_name = actor_class.get_name() if actor_class else ""
        if actor_class_name == TARGET_ACTOR_CLASS_NAME:
            vehicle_data = get_property(actor, "VehicleData")
            try:
                auto_possess_player = actor.get_editor_property("auto_possess_player")
            except Exception:
                auto_possess_player = "Unavailable"
            REPORT["vehicle_actors"].append(
                {
                    "actor_name": actor_name,
                    "actor_path": actor.get_path_name(),
                    "vehicle_data": object_path(vehicle_data),
                    "auto_possess_player": str(auto_possess_player),
                }
            )
        if actor_name == TARGET_ACTOR_NAME and actor_class_name == TARGET_ACTOR_CLASS_NAME:
            matches.append(actor)

    if len(matches) != 1:
        raise RuntimeError(
            f"Expected exactly one target actor, found {len(matches)}: "
            f"name={TARGET_ACTOR_NAME}, class={TARGET_ACTOR_CLASS_NAME}"
        )
    return matches[0]


def inspect_default_pawn() -> None:
    default_pawn_class = unreal.load_class(None, DEFAULT_PAWN_CLASS_PATH)
    if default_pawn_class is None:
        raise RuntimeError(f"Default Pawn class load failed: {DEFAULT_PAWN_CLASS_PATH}")
    default_pawn_object = unreal.get_default_object(default_pawn_class)
    if default_pawn_object is None:
        raise RuntimeError(f"Default Pawn CDO load failed: {DEFAULT_PAWN_CLASS_PATH}")
    REPORT["default_pawn_vehicle_data"] = object_path(
        get_property(default_pawn_object, "VehicleData")
    )


def apply_map_connection() -> None:
    if not unreal.EditorAssetLibrary.does_asset_exist(TARGET_VEHICLE_DATA_PATH):
        raise RuntimeError(f"VehicleData asset is missing: {TARGET_VEHICLE_DATA_PATH}")

    target_vehicle_data = unreal.EditorAssetLibrary.load_asset(TARGET_VEHICLE_DATA_PATH)
    if target_vehicle_data is None:
        raise RuntimeError(f"VehicleData load failed: {TARGET_VEHICLE_DATA_PATH}")

        inspect_default_pawn()
    load_target_map()
    target_actor = find_target_actor()
    current_vehicle_data = get_property(target_actor, "VehicleData")
    REPORT["previous_vehicle_data"] = object_path(current_vehicle_data)

    log(f"Target actor: {target_actor.get_path_name()}")
    log(f"Previous VehicleData: {REPORT['previous_vehicle_data']}")
    log(f"Planned VehicleData: {TARGET_VEHICLE_DATA_PATH}")

    if not APPLY_CHANGES:
        return

    set_property(target_actor, "VehicleData", target_vehicle_data)
    updated_vehicle_data = get_property(target_actor, "VehicleData")
    updated_vehicle_data_path = object_path(updated_vehicle_data)
    if TARGET_VEHICLE_DATA_PATH not in updated_vehicle_data_path:
        raise RuntimeError(
            f"VehicleData assignment verification failed: {updated_vehicle_data_path}"
        )

        level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_editor_subsystem is None:
        raise RuntimeError("LevelEditorSubsystem is unavailable.")
    if not level_editor_subsystem.save_current_level():
        raise RuntimeError(f"Map save failed: {MAP_PATH}")
    REPORT["saved_map"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_map_connection()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][LauncherMap] {error}")
finally:
    REPORT_PATH.write_text(json.dumps(REPORT, ensure_ascii=False, indent=2), encoding="utf-8")
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Launcher map application failed. Read report.json for details.")
