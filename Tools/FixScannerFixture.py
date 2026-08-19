# Copyright (c) CarFight. All Rights Reserved.
# Version: v1.1.0
# Date: 2026-08-18
# Description: TestMap_ScannerP0의 실제 DefaultPawn이 Scanner fixture를 사용하도록 test-only PlayerPawn/GameMode Blueprint와 map override를 구성합니다.
# Changelog:
# - v1.1.0: AutoPossess 기반 배치 Pawn 교환 가설을 폐기하고, BP_ScanPlayerPawn + BP_ScanGameMode + WorldSettings GameMode Override 구조로 교정.
# - v1.0.0: Player0 exact-1 기반 배치 Pawn 교환 시도. 실제 CFSingleGameMode DefaultPawn spawn 구조와 맞지 않아 fail-closed mutation0으로 종료.
# Migration:
# - Production BP_CFVehiclePawn, CFSingleGameMode, CFPlayerController, VehicleData/FittingData, Input asset과 C++ Source는 변경하지 않습니다.
# - 기존 TestMap_ScannerP0의 배치 차량은 변경하지 않습니다.
# - 새 test-only Blueprint 2종과 TestMap_ScannerP0 WorldSettings GameMode Override만 소유합니다.
# - 모든 필수 API/Class/DataAsset preflight가 통과한 뒤에만 asset mutation을 시작합니다.

import json
import os
import traceback
import unreal

# 교정 대상 Scanner 전용 테스트 맵입니다.
TARGET_MAP = "/Game/Maps/TestMap_ScannerP0"
# Scanner 테스트 전용 Blueprint를 둘 폴더입니다.
TARGET_FOLDER = "/Game/CarFight/Tests/Scanner"
# 실제 플레이어가 사용할 Scanner 전용 Pawn Blueprint입니다.
TARGET_PLAYER_PAWN_BP = f"{TARGET_FOLDER}/BP_ScanPlayerPawn"
# Scanner 테스트 맵이 사용할 전용 GameMode Blueprint입니다.
TARGET_GAME_MODE_BP = f"{TARGET_FOLDER}/BP_ScanGameMode"
# Production 기본 차량 Blueprint GeneratedClass 경로입니다.
DEFAULT_VEHICLE_PAWN_CLASS = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"
# 실제 Scanner VehicleData입니다.
SCANNER_VEHICLE_DATA = f"{TARGET_FOLDER}/DA_Vehicle_ScanP0"
# 실제 Scanner FittingData입니다.
SCANNER_FITTING_DATA = f"{TARGET_FOLDER}/DA_Fit_ScannerP0"
# commandlet 결과를 기록할 Saved 폴더입니다.
RESULT_DIR = os.path.join(unreal.Paths.project_saved_dir(), "CarFight")
# Browser runner가 판정할 결정적 결과 JSON입니다.
RESULT_PATH = os.path.join(RESULT_DIR, "ScannerFixtureFixResult.json")


# 실행 결과를 UTF-8 JSON으로 저장합니다.
def write_result(payload):
    os.makedirs(RESULT_DIR, exist_ok=True)
    with open(RESULT_PATH, "w", encoding="utf-8") as result_file:
        json.dump(payload, result_file, ensure_ascii=False, indent=2)


# 필수 기존 자산을 존재 확인 후 로드합니다.
def require_asset(asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"필수 자산이 없습니다: {asset_path}")
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if loaded_asset is None:
        raise RuntimeError(f"필수 자산을 로드하지 못했습니다: {asset_path}")
    return loaded_asset


# 필수 Class를 로드합니다.
def require_class(class_path):
    loaded_class = unreal.load_class(None, class_path)
    if loaded_class is None:
        raise RuntimeError(f"필수 Class를 로드하지 못했습니다: {class_path}")
    return loaded_class


# UE Python 이름과 C++ 원본 이름 후보 중 실제 Reflection Property 하나를 읽습니다.
def get_reflected_property(target_object, property_names):
    errors = []
    for property_name in property_names:
        try:
            return property_name, target_object.get_editor_property(property_name)
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    raise RuntimeError(
        f"Property 조회 실패: {target_object.get_path_name()} / 후보={property_names} / 오류={' | '.join(errors)}"
    )


# UE Python 이름과 C++ 원본 이름 후보 중 실제 Reflection Property 하나를 설정합니다.
def set_reflected_property(target_object, property_names, value):
    errors = []
    for property_name in property_names:
        try:
            target_object.set_editor_property(property_name, value)
            return property_name
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    raise RuntimeError(
        f"Property 설정 실패: {target_object.get_path_name()} / 후보={property_names} / 오류={' | '.join(errors)}"
    )


# Blueprint asset 경로에서 asset 이름을 반환합니다.
def get_asset_name(asset_path):
    return asset_path.rsplit("/", 1)[-1]


# Blueprint asset의 GeneratedClass 경로를 반환합니다.
def get_generated_class_path(asset_path):
    asset_name = get_asset_name(asset_path)
    return f"{asset_path}.{asset_name}_C"


# Blueprint를 compile하고 GeneratedClass를 확정합니다.
def resolve_generated_class(blueprint_asset, asset_path):
    blueprint_editor_library = getattr(unreal, "BlueprintEditorLibrary", None)
    if blueprint_editor_library is not None:
        blueprint_editor_library.compile_blueprint(blueprint_asset)

    generated_class = None
    try:
        generated_class = blueprint_asset.generated_class()
    except Exception:
        generated_class = None

    if generated_class is None:
        try:
            generated_class = blueprint_asset.get_editor_property("generated_class")
        except Exception:
            generated_class = None

    if generated_class is None:
        generated_class = unreal.load_class(None, get_generated_class_path(asset_path))

    if generated_class is None:
        raise RuntimeError(f"Blueprint GeneratedClass를 해결하지 못했습니다: {asset_path}")
    return generated_class


# 지정 Parent Class의 Blueprint를 기존 asset이면 로드하고 없으면 test-only로 생성합니다.
def load_or_create_blueprint(asset_path, parent_class, created_assets):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        existing_blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
        if existing_blueprint is None:
            raise RuntimeError(f"기존 Blueprint 로드 실패: {asset_path}")
        return existing_blueprint, False

    blueprint_factory_type = getattr(unreal, "BlueprintFactory", None)
    asset_tools_helpers_type = getattr(unreal, "AssetToolsHelpers", None)
    blueprint_type = getattr(unreal, "Blueprint", None)
    if blueprint_factory_type is None or asset_tools_helpers_type is None or blueprint_type is None:
        raise RuntimeError("Blueprint 생성에 필요한 UE Python API가 노출되지 않았습니다.")

    blueprint_factory = blueprint_factory_type()
    set_reflected_property(blueprint_factory, ("parent_class", "ParentClass"), parent_class)

    asset_tools = asset_tools_helpers_type.get_asset_tools()
    asset_name = get_asset_name(asset_path)
    package_path = asset_path.rsplit("/", 1)[0]
    created_blueprint = asset_tools.create_asset(asset_name, package_path, blueprint_type, blueprint_factory)
    if created_blueprint is None:
        raise RuntimeError(f"Blueprint 생성 실패: {asset_path}")

    created_assets.append(asset_path)
    return created_blueprint, True


# 지정 Blueprint asset을 강제로 저장합니다.
def save_blueprint(asset_path):
    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        raise RuntimeError(f"Blueprint 저장 실패: {asset_path}")


# 현재 로드된 맵을 저장합니다.
def save_current_map(level_subsystem):
    saved_level = False
    try:
        saved_level = bool(level_subsystem.save_current_level())
    except Exception:
        saved_level = False
    if saved_level:
        return
    if not unreal.EditorAssetLibrary.save_asset(TARGET_MAP, only_if_is_dirty=False):
        raise RuntimeError(f"Scanner 테스트 맵 저장 실패: {TARGET_MAP}")


# 이번 실행에서 새로 만든 test-only asset만 역순으로 삭제합니다.
def cleanup_created_assets(created_assets):
    cleanup_errors = []
    for asset_path in reversed(created_assets):
        try:
            if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                if not unreal.EditorAssetLibrary.delete_asset(asset_path):
                    cleanup_errors.append(f"삭제 실패: {asset_path}")
        except Exception as exc:
            cleanup_errors.append(f"삭제 예외: {asset_path}: {exc}")
    return cleanup_errors


# Scanner 전용 PlayerPawn/GameMode와 map override를 구성합니다.
def main():
    # 이번 실행에서 새로 생성한 test-only Blueprint 경로입니다.
    created_assets = []
    # 맵 저장 여부입니다.
    map_saved = False
    # WorldSettings에 GameMode override를 적용했는지 여부입니다.
    world_settings_mutated = False
    # 실패 시 메모리 상태만 복원할 기존 GameMode override입니다.
    previous_game_mode = None

    try:
        # 저장 가능한 Scanner VehicleData입니다.
        scanner_vehicle_data = require_asset(SCANNER_VEHICLE_DATA)
        # 저장 가능한 Scanner FittingData입니다.
        scanner_fitting_data = require_asset(SCANNER_FITTING_DATA)
        # Production 기본 차량 Blueprint Class입니다.
        default_vehicle_pawn_class = require_class(DEFAULT_VEHICLE_PAWN_CLASS)
        # 기존 CFPlayerController Python class입니다.
        cf_player_controller = getattr(unreal, "CFPlayerController", None)
        if cf_player_controller is None:
            raise RuntimeError("CFPlayerController Python class가 노출되지 않았습니다.")
        # test-only GameMode가 상속할 Engine GameModeBase class입니다.
        game_mode_base_class = getattr(unreal, "GameModeBase", None)
        if game_mode_base_class is None:
            raise RuntimeError("GameModeBase Python class가 노출되지 않았습니다.")
        # CDO 접근 API 존재 여부를 preflight합니다.
        if not hasattr(unreal, "get_default_object"):
            raise RuntimeError("unreal.get_default_object API가 노출되지 않았습니다.")

        if not unreal.EditorAssetLibrary.does_directory_exist(TARGET_FOLDER):
            raise RuntimeError(f"Scanner test folder가 없습니다: {TARGET_FOLDER}")

        # Scanner 전용 Player Pawn Blueprint입니다.
        player_blueprint, player_blueprint_created = load_or_create_blueprint(
            TARGET_PLAYER_PAWN_BP,
            default_vehicle_pawn_class,
            created_assets,
        )
        # Scanner Player Pawn GeneratedClass입니다.
        player_pawn_class = resolve_generated_class(player_blueprint, TARGET_PLAYER_PAWN_BP)
        # Scanner Player Pawn CDO입니다.
        player_pawn_cdo = unreal.get_default_object(player_pawn_class)
        if player_pawn_cdo is None:
            raise RuntimeError("Scanner PlayerPawn CDO를 가져오지 못했습니다.")

        # 실제 플레이어 기본 VehicleData를 Scanner fixture로 설정합니다.
        player_vehicle_property = set_reflected_property(
            player_pawn_cdo,
            ("vehicle_data", "VehicleData"),
            scanner_vehicle_data,
        )
        # 실제 플레이어 기본 FittingData를 Scanner fixture로 설정합니다.
        player_fitting_property = set_reflected_property(
            player_pawn_cdo,
            ("vehicle_fitting_data", "VehicleFittingData"),
            scanner_fitting_data,
        )
        save_blueprint(TARGET_PLAYER_PAWN_BP)

        # 저장 뒤 Scanner PlayerPawn GeneratedClass를 fresh resolve합니다.
        player_pawn_class = require_class(get_generated_class_path(TARGET_PLAYER_PAWN_BP))
        # 저장 뒤 Scanner PlayerPawn CDO입니다.
        player_pawn_cdo = unreal.get_default_object(player_pawn_class)
        _, readback_vehicle_data = get_reflected_property(player_pawn_cdo, ("vehicle_data", "VehicleData"))
        _, readback_fitting_data = get_reflected_property(player_pawn_cdo, ("vehicle_fitting_data", "VehicleFittingData"))
        if readback_vehicle_data != scanner_vehicle_data or readback_fitting_data != scanner_fitting_data:
            raise RuntimeError("Scanner PlayerPawn CDO DataAsset readback이 예상과 다릅니다.")

        # Scanner 테스트 전용 GameMode Blueprint입니다.
        game_mode_blueprint, game_mode_blueprint_created = load_or_create_blueprint(
            TARGET_GAME_MODE_BP,
            game_mode_base_class,
            created_assets,
        )
        # Scanner GameMode GeneratedClass입니다.
        game_mode_class = resolve_generated_class(game_mode_blueprint, TARGET_GAME_MODE_BP)
        # Scanner GameMode CDO입니다.
        game_mode_cdo = unreal.get_default_object(game_mode_class)
        if game_mode_cdo is None:
            raise RuntimeError("Scanner GameMode CDO를 가져오지 못했습니다.")

        # Scanner GameMode의 실제 DefaultPawnClass를 Scanner PlayerPawn으로 설정합니다.
        game_mode_default_pawn_property = set_reflected_property(
            game_mode_cdo,
            ("default_pawn_class", "DefaultPawnClass"),
            player_pawn_class,
        )
        # 기존 UI/Input lifecycle을 유지하도록 PlayerControllerClass를 CFPlayerController로 설정합니다.
        game_mode_player_controller_property = set_reflected_property(
            game_mode_cdo,
            ("player_controller_class", "PlayerControllerClass"),
            cf_player_controller,
        )
        save_blueprint(TARGET_GAME_MODE_BP)

        # 저장 뒤 Scanner GameMode GeneratedClass를 fresh resolve합니다.
        game_mode_class = require_class(get_generated_class_path(TARGET_GAME_MODE_BP))
        # 저장 뒤 Scanner GameMode CDO입니다.
        game_mode_cdo = unreal.get_default_object(game_mode_class)
        _, readback_default_pawn_class = get_reflected_property(game_mode_cdo, ("default_pawn_class", "DefaultPawnClass"))
        _, readback_player_controller_class = get_reflected_property(game_mode_cdo, ("player_controller_class", "PlayerControllerClass"))
        if readback_default_pawn_class != player_pawn_class:
            raise RuntimeError("Scanner GameMode DefaultPawnClass readback이 예상과 다릅니다.")

        # Scanner 테스트 맵을 로드합니다.
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if level_subsystem is None:
            raise RuntimeError("LevelEditorSubsystem을 가져오지 못했습니다.")
        if not level_subsystem.load_level(TARGET_MAP):
            raise RuntimeError(f"Scanner 테스트 맵 로드 실패: {TARGET_MAP}")

        # 현재 Editor World입니다.
        unreal_editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if unreal_editor_subsystem is None:
            raise RuntimeError("UnrealEditorSubsystem을 가져오지 못했습니다.")
        current_world = unreal_editor_subsystem.get_editor_world()
        if current_world is None:
            raise RuntimeError("현재 Editor World를 가져오지 못했습니다.")
        # 현재 맵 WorldSettings입니다.
        world_settings = current_world.get_world_settings()
        if world_settings is None:
            raise RuntimeError("WorldSettings를 가져오지 못했습니다.")

        # 실패 시 메모리 복원에 사용할 기존 GameMode Override입니다.
        world_game_mode_property, previous_game_mode = get_reflected_property(
            world_settings,
            ("default_game_mode", "DefaultGameMode"),
        )
        # TestMap_ScannerP0에만 Scanner 전용 GameMode Override를 적용합니다.
        world_settings.set_editor_property(world_game_mode_property, game_mode_class)
        world_settings_mutated = True

        # 저장 전 메모리 readback입니다.
        _, pre_save_game_mode = get_reflected_property(world_settings, ("default_game_mode", "DefaultGameMode"))
        if pre_save_game_mode != game_mode_class:
            raise RuntimeError("WorldSettings GameMode Override 메모리 readback이 예상과 다릅니다.")

        save_current_map(level_subsystem)
        map_saved = True

        # 저장 뒤 맵을 다시 로드해 persisted GameMode Override를 검증합니다.
        if not level_subsystem.load_level(TARGET_MAP):
            raise RuntimeError("저장 후 Scanner 테스트 맵 재로드 실패")
        current_world = unreal_editor_subsystem.get_editor_world()
        world_settings = current_world.get_world_settings()
        _, persisted_game_mode = get_reflected_property(world_settings, ("default_game_mode", "DefaultGameMode"))
        if persisted_game_mode != game_mode_class:
            raise RuntimeError("저장 후 WorldSettings GameMode Override readback이 예상과 다릅니다.")

        write_result(
            {
                "status": "success",
                "mode": "dedicated_test_player_gamemode",
                "created_assets": created_assets,
                "player_blueprint_created": player_blueprint_created,
                "game_mode_blueprint_created": game_mode_blueprint_created,
                "map_saved": map_saved,
                "target_map": TARGET_MAP,
                "player_pawn_blueprint": TARGET_PLAYER_PAWN_BP,
                "game_mode_blueprint": TARGET_GAME_MODE_BP,
                "scanner_vehicle_data": SCANNER_VEHICLE_DATA,
                "scanner_fitting_data": SCANNER_FITTING_DATA,
                "player_vehicle_property": player_vehicle_property,
                "player_fitting_property": player_fitting_property,
                "game_mode_default_pawn_property": game_mode_default_pawn_property,
                "game_mode_player_controller_property": game_mode_player_controller_property,
                "player_pawn_cdo_scanner_readback": True,
                "game_mode_default_pawn_readback": True,
                "map_game_mode_override_readback": True,
                "production_asset_mutation": False,
                "placed_vehicle_mutation": False,
                "runtime_source_mutation": False,
            }
        )
        unreal.log("[CarFight] Scanner P0-06 dedicated PlayerPawn/GameMode fixture 교정 완료")

    except Exception as exc:
        # 맵 저장 전 실패한 WorldSettings 메모리 변경만 복원합니다.
        if world_settings_mutated and not map_saved:
            try:
                unreal_editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
                current_world = unreal_editor_subsystem.get_editor_world() if unreal_editor_subsystem is not None else None
                world_settings = current_world.get_world_settings() if current_world is not None else None
                if world_settings is not None:
                    set_reflected_property(world_settings, ("default_game_mode", "DefaultGameMode"), previous_game_mode)
            except Exception:
                pass

        # 아직 맵에 persisted reference가 생기지 않은 실패에서는 이번 실행 신규 asset만 정리합니다.
        cleanup_errors = cleanup_created_assets(created_assets) if not map_saved else []
        write_result(
            {
                "status": "failed",
                "map_saved": map_saved,
                "created_assets_before_cleanup": created_assets,
                "cleanup_errors": cleanup_errors,
                "error": str(exc),
                "traceback": traceback.format_exc(),
            }
        )
        unreal.log_error(f"[CarFight] Scanner fixture 교정 실패: {exc}")
        raise


main()
