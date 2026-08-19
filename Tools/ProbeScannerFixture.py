# Copyright (c) CarFight. All Rights Reserved.
# Version: v1.0.0
# Date: 2026-08-18
# Description: Scanner P0-06 fixture 교정에 필요한 UE 5.8 Python Blueprint/CDO/WorldSettings API와 source/target map 차량 구성을 read-only로 조사합니다.
# Changelog:
# - v1.0.0: BlueprintFactory/AssetTools/GeneratedClass/CDO/WorldSettings와 두 테스트 맵의 차량 DataAsset 연결을 저장 없이 기록합니다.
# Migration:
# - Asset 생성/수정/저장은 수행하지 않습니다.

import json
import os
import traceback
import unreal

# 원본 scanner-less 비교 맵입니다.
SOURCE_MAP = "/Game/Maps/TestMap_AmmoRipple"
# 교정 대상 Scanner 테스트 맵입니다.
TARGET_MAP = "/Game/Maps/TestMap_ScannerP0"
# 기본 차량 Blueprint GeneratedClass 경로입니다.
DEFAULT_PAWN_CLASS_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"
# probe 결과 저장 폴더입니다.
RESULT_DIR = os.path.join(unreal.Paths.project_saved_dir(), "CarFight")
# probe 결과 JSON입니다.
RESULT_PATH = os.path.join(RESULT_DIR, "ScannerFixtureProbeResult.json")


# 결과를 UTF-8 JSON으로 기록합니다.
def write_result(payload):
    os.makedirs(RESULT_DIR, exist_ok=True)
    with open(RESULT_PATH, "w", encoding="utf-8") as result_file:
        json.dump(payload, result_file, ensure_ascii=False, indent=2)


# 후보 Reflection property를 읽고 이름/값 문자열을 반환합니다.
def read_property(target_object, property_names):
    errors = []
    for property_name in property_names:
        try:
            value = target_object.get_editor_property(property_name)
            value_path = value.get_path_name() if value is not None and hasattr(value, "get_path_name") else str(value)
            return {"property_name": property_name, "value": value_path}
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    return {"property_name": "", "value": "", "errors": errors}


# 현재 로드된 레벨의 차량 Actor와 DataAsset 연결을 읽습니다.
def read_current_level_vehicles(editor_actor_subsystem):
    vehicles = []
    for actor in editor_actor_subsystem.get_all_level_actors():
        vehicle_data = read_property(actor, ("vehicle_data", "VehicleData"))
        fitting_data = read_property(actor, ("vehicle_fitting_data", "VehicleFittingData"))
        if not vehicle_data["property_name"] or not fitting_data["property_name"]:
            continue
        actor_location = actor.get_actor_location()
        vehicles.append(
            {
                "actor_name": actor.get_name(),
                "actor_path": actor.get_path_name(),
                "location": [float(actor_location.x), float(actor_location.y), float(actor_location.z)],
                "vehicle_data": vehicle_data,
                "fitting_data": fitting_data,
                "auto_possess_player": read_property(actor, ("auto_possess_player", "AutoPossessPlayer")),
            }
        )
    return vehicles


# Scanner fixture 교정 API와 current asset wiring을 read-only로 조사합니다.
def main():
    try:
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if level_subsystem is None or editor_actor_subsystem is None:
            raise RuntimeError("필수 Editor subsystem을 가져오지 못했습니다.")

        if not level_subsystem.load_level(SOURCE_MAP):
            raise RuntimeError(f"원본 맵 로드 실패: {SOURCE_MAP}")
        source_vehicles = read_current_level_vehicles(editor_actor_subsystem)

        if not level_subsystem.load_level(TARGET_MAP):
            raise RuntimeError(f"대상 맵 로드 실패: {TARGET_MAP}")
        target_vehicles = read_current_level_vehicles(editor_actor_subsystem)

        current_world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        world_settings = current_world.get_world_settings() if current_world is not None else None

        default_pawn_class = unreal.load_class(None, DEFAULT_PAWN_CLASS_PATH)
        default_pawn_cdo = unreal.get_default_object(default_pawn_class) if default_pawn_class is not None else None

        cf_single_game_mode = getattr(unreal, "CFSingleGameMode", None)
        cf_single_game_mode_cdo = unreal.get_default_object(cf_single_game_mode) if cf_single_game_mode is not None else None

        blueprint_factory = getattr(unreal, "BlueprintFactory", None)
        asset_tools_helpers = getattr(unreal, "AssetToolsHelpers", None)
        blueprint_editor_library = getattr(unreal, "BlueprintEditorLibrary", None)

        write_result(
            {
                "status": "success",
                "mutation": 0,
                "save": 0,
                "api": {
                    "BlueprintFactory": blueprint_factory is not None,
                    "AssetToolsHelpers": asset_tools_helpers is not None,
                    "BlueprintEditorLibrary": blueprint_editor_library is not None,
                    "get_default_object": hasattr(unreal, "get_default_object"),
                    "load_class": hasattr(unreal, "load_class"),
                },
                "source_vehicles": source_vehicles,
                "target_vehicles": target_vehicles,
                "default_pawn_class": default_pawn_class.get_path_name() if default_pawn_class is not None else "",
                "default_pawn_cdo": {
                    "vehicle_data": read_property(default_pawn_cdo, ("vehicle_data", "VehicleData")) if default_pawn_cdo is not None else {},
                    "fitting_data": read_property(default_pawn_cdo, ("vehicle_fitting_data", "VehicleFittingData")) if default_pawn_cdo is not None else {},
                },
                                "cf_single_game_mode_class": str(cf_single_game_mode) if cf_single_game_mode is not None else "",

                "cf_single_game_mode_cdo": {
                    "default_pawn_class": read_property(cf_single_game_mode_cdo, ("default_pawn_class", "DefaultPawnClass")) if cf_single_game_mode_cdo is not None else {},
                },
                "world_settings": {
                    "path": world_settings.get_path_name() if world_settings is not None else "",
                    "default_game_mode": read_property(world_settings, ("default_game_mode", "DefaultGameMode")) if world_settings is not None else {},
                },
            }
        )
        unreal.log("[CarFight] Scanner fixture read-only probe 완료")
    except Exception as exc:
        write_result({"status": "failed", "mutation": 0, "save": 0, "error": str(exc), "traceback": traceback.format_exc()})
        unreal.log_error(f"[CarFight] Scanner fixture probe 실패: {exc}")
        raise


main()
