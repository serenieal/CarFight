# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.1.0
# Date: 2026-08-06
# Description: CF-FQ-033 DR-PIE-03 AP 50 전용 복제 에셋 체인과 격리 테스트 맵 생성 도구
# Scope: /Game/CarFight/Tests/VehicleDefense의 AP50 DataAsset 4개·Pawn Blueprint·GameMode Blueprint와 /Game/Maps/M_VehicleDefensePIE_AP50만 생성·수정합니다.
# Changelog:
# - v1.1.0: 플레이어가 맵 Actor가 아니라 GameMode에서 생성되는 실제 구조에 맞춰 AP50 Pawn CDO·GameMode·맵 Override 격리 경로를 추가.
# - v1.0.0: HeavyShell→Weapon→Preset→Sedan AP50 복제 체인, 전용 맵 Player VehicleData 연결, 원본 SHA-256 보호와 JSON 보고서 추가.
# Migration:
# - 원본 DA_HeavyShell, DA_ProtoTurretCannon, HeavyCannon, DA_TestSedan과 M_VehicleDefensePIE는 읽기 전용으로 유지합니다.
# - AP50 전용 맵은 DR-PIE-03에서만 사용하며 기존 방어 PIE 맵을 대체하지 않습니다.
# - -Apply에 해당하는 환경 변수가 없으면 에셋이나 맵을 저장하지 않습니다.

from __future__ import annotations

import hashlib
import json
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.0.0] 실행 보고서와 문서에서 식별할 도구 버전입니다.
TOOL_VERSION = "1.1.0"

# [v1.0.0] 명시적 Apply 실행에서만 .uasset과 .umap 저장을 허용하는 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_DEFENSE_AP50_APPLY", "0") == "1"

# [v1.0.0] 현재 Unreal 프로젝트의 절대 루트 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 구조화 실행 보고서를 저장할 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "VehicleDefenseAP50Apply"

# [v1.0.0] MCP와 후속 세션에서 읽을 JSON 보고서 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] 모든 AP50 복제 DataAsset을 저장할 테스트 전용 Unreal 폴더입니다.
TEST_ASSET_FOLDER = "/Game/CarFight/Tests/VehicleDefense"

# [v1.0.0] AP 0 기준 HeavyShell ProjectileData 원본입니다.
SOURCE_PROJECTILE_PATH = "/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell"

# [v1.0.0] AP 0 HeavyCannon WeaponData 원본입니다.
SOURCE_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon"

# [v1.0.0] AP 0 HeavyCannon EquipmentPresetData 원본입니다.
SOURCE_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon"

# [v1.0.0] AP 0 플레이어 Sedan VehicleData 원본입니다.
SOURCE_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan"

# [v1.0.0] 기존 DR-PIE-00~04와 DR-PIE-02에서 사용한 읽기 전용 방어 맵입니다.
SOURCE_MAP_PATH = "/Game/Maps/M_VehicleDefensePIE"

# [v1.1.0] 프로젝트 기본 플레이어 차량 Blueprint 원본입니다.
SOURCE_PAWN_BLUEPRINT_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn"

# [v1.1.0] AP50 전용 GameMode Blueprint가 상속할 C++ GameMode 클래스 경로입니다.
GAME_MODE_PARENT_CLASS_PATH = "/Script/CarFight_Re.CFSingleGameMode"


# [v1.0.0] AP50 ProjectileData가 직접 참조할 관통 테스트 DamageData입니다.
AP50_DAMAGE_PATH = "/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageArmorPenTest"

# [v1.0.0] 원본 AP 0 DamageData 보호 검증 경로입니다.
AP0_DAMAGE_PATH = "/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset"

# [v1.0.0] AP50 맵에서도 그대로 유지할 방어 대상 SUV VehicleData입니다.
DEFENSE_TARGET_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV"

# [v1.0.0] 방어 대상 SUV가 참조하는 VehicleDefenseData입니다.
DEFENSE_DATA_PATH = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test"

# [v1.0.0] AP50 DamageData를 연결할 테스트 전용 ProjectileData 복제 경로입니다.
TEST_PROJECTILE_PATH = f"{TEST_ASSET_FOLDER}/DA_HeavyShell_DRAP50"

# [v1.0.0] AP50 ProjectileData를 연결할 테스트 전용 WeaponData 복제 경로입니다.
TEST_WEAPON_PATH = f"{TEST_ASSET_FOLDER}/DA_ProtoCannon_DRAP50"

# [v1.0.0] AP50 WeaponData를 연결할 테스트 전용 EquipmentPresetData 복제 경로입니다.
TEST_PRESET_PATH = f"{TEST_ASSET_FOLDER}/HeavyCannon_DRAP50"

# [v1.0.0] AP50 EquipmentPresetData를 기본 장비로 사용할 테스트 전용 Sedan VehicleData입니다.
TEST_VEHICLE_PATH = f"{TEST_ASSET_FOLDER}/DA_TestSedan_DRAP50"

# [v1.1.0] AP50 Sedan VehicleData를 CDO에 연결할 테스트 전용 차량 Pawn Blueprint입니다.
TEST_PAWN_BLUEPRINT_PATH = f"{TEST_ASSET_FOLDER}/BP_CFVehiclePawn_DRAP50"

# [v1.1.0] AP50 Pawn Blueprint를 기본 차량으로 생성할 테스트 전용 GameMode Blueprint입니다.
TEST_GAME_MODE_BLUEPRINT_PATH = f"{TEST_ASSET_FOLDER}/BP_CFSingleGameMode_DRAP50"

# [v1.0.0] 원본 방어 맵을 보존하면서 AP50 플레이어 차량만 연결할 테스트 맵입니다.
TEST_MAP_PATH = "/Game/Maps/M_VehicleDefensePIE_AP50"


# [v1.0.0] 이번 도구가 생성하거나 저장할 수 있는 유일한 Unreal 에셋 경로입니다.
MUTABLE_ASSET_PATHS = {
    TEST_PROJECTILE_PATH,
    TEST_WEAPON_PATH,
        TEST_PRESET_PATH,
    TEST_VEHICLE_PATH,
    TEST_PAWN_BLUEPRINT_PATH,
    TEST_GAME_MODE_BLUEPRINT_PATH,
    TEST_MAP_PATH,
}

# [v1.0.0] 실행 전후 SHA-256이 같아야 하는 읽기 전용 원본과 공용 테스트 에셋입니다.
PROTECTED_ASSET_PATHS = {
    SOURCE_PROJECTILE_PATH,
    SOURCE_WEAPON_PATH,
    SOURCE_PRESET_PATH,
    SOURCE_VEHICLE_PATH,
        SOURCE_MAP_PATH,
    SOURCE_PAWN_BLUEPRINT_PATH,
    AP50_DAMAGE_PATH,
    AP0_DAMAGE_PATH,
    DEFENSE_TARGET_VEHICLE_PATH,
    DEFENSE_DATA_PATH,
}

# [v1.0.0] 실행 모드, 생성·수정 목록, 연결 스냅샷과 보호 결과를 기록할 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "vehicle_defense_ap50_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "source_paths": {
        "projectile": SOURCE_PROJECTILE_PATH,
        "weapon": SOURCE_WEAPON_PATH,
        "preset": SOURCE_PRESET_PATH,
        "vehicle": SOURCE_VEHICLE_PATH,
        "map": SOURCE_MAP_PATH,
        "ap0_damage": AP0_DAMAGE_PATH,
        "ap50_damage": AP50_DAMAGE_PATH,
        "defense_target_vehicle": DEFENSE_TARGET_VEHICLE_PATH,
        "defense_data": DEFENSE_DATA_PATH,
    },
    "target_paths": {
        "projectile": TEST_PROJECTILE_PATH,
        "weapon": TEST_WEAPON_PATH,
        "preset": TEST_PRESET_PATH,
                "vehicle": TEST_VEHICLE_PATH,
        "pawn_blueprint": TEST_PAWN_BLUEPRINT_PATH,
        "game_mode_blueprint": TEST_GAME_MODE_BLUEPRINT_PATH,
        "map": TEST_MAP_PATH,
    },
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "planned_changes": [],
    "protected_hashes_before": {},
    "protected_hashes_after": {},
    "asset_snapshots": {},
    "map_vehicle_actors_before": [],
    "map_vehicle_actors_after": [],
    "player_actor_before": {},
    "player_actor_after": {},
    "defense_target_actor_before": {},
    "defense_target_actor_after": {},
        "asset_chain_valid": False,
    "pawn_blueprint_valid": False,
    "game_mode_blueprint_valid": False,
    "map_game_mode_override": "",
    "map_connected": False,
    "protected_contract_passed": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 AP50 테스트 에셋 도구 메시지를 기록합니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][DefenseAP50] {message}")


# [v1.0.0] PascalCase C++ 프로퍼티 이름을 Unreal Python snake_case 후보로 변환합니다.
def pascal_to_snake(name: str) -> str:
    first_step = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", first_step).lower()


# [v1.0.0] Reflection 프로퍼티 접근에 사용할 원본·snake_case 후보를 중복 없이 반환합니다.
def candidate_property_names(name: str) -> list[str]:
    return list(dict.fromkeys([name, pascal_to_snake(name)]))


# [v1.0.0] 원본 또는 snake_case 프로퍼티 이름으로 Unreal Reflection 값을 읽습니다.
def get_property(target: Any, property_name: str) -> Any:
    last_error: Exception | None = None
    for candidate_name in candidate_property_names(property_name):
        try:
            return target.get_editor_property(candidate_name)
        except Exception as error:
            last_error = error
    raise RuntimeError(
        f"Property read failed: {type(target).__name__}.{property_name}: {last_error}"
    )


# [v1.0.0] 원본 또는 snake_case 프로퍼티 이름으로 Unreal Reflection 값을 씁니다.
def set_property(target: Any, property_name: str, value: Any) -> None:
    last_error: Exception | None = None
    for candidate_name in candidate_property_names(property_name):
        try:
            target.set_editor_property(candidate_name, value)
            return
        except Exception as error:
            last_error = error
    raise RuntimeError(
        f"Property write failed: {type(target).__name__}.{property_name}: {last_error}"
    )


# [v1.0.0] Unreal 객체 참조를 비교 가능한 전체 Object Path 문자열로 변환합니다.
def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        return unreal.SystemLibrary.get_path_name(value)
    except Exception:
        return str(value)


# [v1.0.0] /Game 에셋 경로를 프로젝트 Content 아래 실제 파일 경로로 변환합니다.
def asset_file_path(asset_path: str) -> Path:
    if not asset_path.startswith("/Game/"):
        raise RuntimeError(f"Only /Game asset paths are supported: {asset_path}")
    file_extension = ".umap" if asset_path.startswith("/Game/Maps/") else ".uasset"
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'):]}{file_extension}"


# [v1.0.0] 존재하는 파일의 SHA-256을 계산하고 파일이 없으면 빈 문자열을 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
    digest = hashlib.sha256()
    with file_path.open("rb") as source_file:
        for file_chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(file_chunk)
    return digest.hexdigest()


# [v1.0.0] 지정한 Unreal 에셋 경로 집합의 실제 파일 SHA-256을 반환합니다.
def capture_hashes(asset_paths: set[str]) -> dict[str, str]:
    return {
        asset_path: sha256_file(asset_file_path(asset_path))
        for asset_path in sorted(asset_paths)
    }


# [v1.0.0] 필수 Unreal 에셋을 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if loaded_asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return loaded_asset


# [v1.0.0] 계획된 단일 에셋 또는 맵 변경을 구조화 보고서에 기록합니다.
def record_change(asset_path: str, property_name: str, value: Any) -> None:
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.0.0] 화이트리스트를 확인하고 Apply 모드에서만 Unreal 프로퍼티를 변경합니다.
def apply_property(asset_path: str, asset: Any, property_name: str, value: Any) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in the mutation whitelist: {asset_path}")
    record_change(asset_path, property_name, value)
    if APPLY_CHANGES:
        set_property(asset, property_name, value)


# [v1.0.0] 읽기 전용 원본을 테스트 전용 경로에 복제하거나 기존 복제본을 로드합니다.
def duplicate_or_load_asset(source_path: str, target_path: str) -> Any:
    if target_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Target path is not in the mutation whitelist: {target_path}")

    source_asset = require_asset(source_path)
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        target_asset = require_asset(target_path)
        REPORT["updated_assets"].append(target_path)
        return target_asset

    if not APPLY_CHANGES:
        record_change(target_path, "DuplicateAsset", source_path)
        return source_asset

    if not unreal.EditorAssetLibrary.does_directory_exist(TEST_ASSET_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(TEST_ASSET_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {TEST_ASSET_FOLDER}")

    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"Asset duplication failed: {source_path} -> {target_path}")
    REPORT["created_assets"].append(target_path)
    return duplicated_asset


# [v1.0.0] Apply 모드에서 화이트리스트된 DataAsset을 강제로 저장합니다.
def save_data_asset(asset_path: str, asset: Any) -> None:
    if not APPLY_CHANGES:
        return
    if asset_path not in MUTABLE_ASSET_PATHS or asset_path == TEST_MAP_PATH:
        raise RuntimeError(f"Save path is not an allowed DataAsset path: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.1.0] Blueprint 에셋 경로를 생성 클래스 Object Path로 변환합니다.
def blueprint_generated_class_path(blueprint_asset_path: str) -> str:
    blueprint_name = blueprint_asset_path.rsplit("/", 1)[-1]
    return f"{blueprint_asset_path}.{blueprint_name}_C"


# [v1.1.0] Blueprint 에셋의 생성 클래스를 안전하게 로드합니다.
def load_blueprint_generated_class(blueprint_asset_path: str) -> Any:
    load_blueprint_class = getattr(unreal.EditorAssetLibrary, "load_blueprint_class", None)
    generated_class = load_blueprint_class(blueprint_asset_path) if callable(load_blueprint_class) else None
    if generated_class is None:
        generated_class = unreal.load_class(None, blueprint_generated_class_path(blueprint_asset_path))
    if generated_class is None:
        raise RuntimeError(f"Blueprint generated class load failed: {blueprint_asset_path}")
    return generated_class


# [v1.1.0] 새 Blueprint 생성 직후 생성 클래스가 준비되도록 Blueprint를 컴파일합니다.
def compile_blueprint_asset(blueprint_asset: Any) -> None:
    blueprint_editor_library = getattr(unreal, "BlueprintEditorLibrary", None)
    if blueprint_editor_library is not None and hasattr(blueprint_editor_library, "compile_blueprint"):
        blueprint_editor_library.compile_blueprint(blueprint_asset)
        return

    kismet_editor_utilities = getattr(unreal, "KismetEditorUtilities", None)
    if kismet_editor_utilities is not None and hasattr(kismet_editor_utilities, "compile_blueprint"):
        kismet_editor_utilities.compile_blueprint(blueprint_asset)
        return

    raise RuntimeError("No supported Unreal Python Blueprint compile API is available.")


# [v1.1.0] C++ CFSingleGameMode를 부모로 하는 AP50 전용 GameMode Blueprint를 로드하거나 생성합니다.
def create_or_load_game_mode_blueprint() -> Any | None:
    if unreal.EditorAssetLibrary.does_asset_exist(TEST_GAME_MODE_BLUEPRINT_PATH):
        REPORT["updated_assets"].append(TEST_GAME_MODE_BLUEPRINT_PATH)
        return require_asset(TEST_GAME_MODE_BLUEPRINT_PATH)

    if not APPLY_CHANGES:
        record_change(
            TEST_GAME_MODE_BLUEPRINT_PATH,
            "CreateBlueprint",
            GAME_MODE_PARENT_CLASS_PATH,
        )
        return None

    if not unreal.EditorAssetLibrary.does_directory_exist(TEST_ASSET_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(TEST_ASSET_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {TEST_ASSET_FOLDER}")

    parent_game_mode_class = unreal.load_class(None, GAME_MODE_PARENT_CLASS_PATH)
    if parent_game_mode_class is None:
        raise RuntimeError(f"GameMode parent class load failed: {GAME_MODE_PARENT_CLASS_PATH}")

    blueprint_factory = unreal.BlueprintFactory()
    set_property(blueprint_factory, "ParentClass", parent_game_mode_class)

    game_mode_blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        TEST_GAME_MODE_BLUEPRINT_PATH.rsplit("/", 1)[-1],
        TEST_ASSET_FOLDER,
        unreal.Blueprint,
        blueprint_factory,
    )
    if game_mode_blueprint is None:
        raise RuntimeError(f"GameMode Blueprint creation failed: {TEST_GAME_MODE_BLUEPRINT_PATH}")

    compile_blueprint_asset(game_mode_blueprint)
    REPORT["created_assets"].append(TEST_GAME_MODE_BLUEPRINT_PATH)
    return game_mode_blueprint


# [v1.1.0] AP50 Pawn CDO와 GameMode CDO의 런타임 생성 참조를 설정합니다.
def configure_blueprint_runtime_chain(
    pawn_blueprint: Any,
    game_mode_blueprint: Any | None,
    vehicle_asset: Any,
) -> Any | None:
    record_change(TEST_PAWN_BLUEPRINT_PATH, "CDO.VehicleData", vehicle_asset)
    record_change(
        TEST_GAME_MODE_BLUEPRINT_PATH,
        "CDO.ConfiguredDefaultVehiclePawnClass",
        blueprint_generated_class_path(TEST_PAWN_BLUEPRINT_PATH),
    )

    if not APPLY_CHANGES:
        REPORT["pawn_blueprint_valid"] = True
        REPORT["game_mode_blueprint_valid"] = True
        return None

    if game_mode_blueprint is None:
        raise RuntimeError("Apply mode requires a valid AP50 GameMode Blueprint.")

    pawn_class = load_blueprint_generated_class(TEST_PAWN_BLUEPRINT_PATH)
    pawn_default_object = unreal.get_default_object(pawn_class)
    if pawn_default_object is None:
        raise RuntimeError("AP50 Pawn CDO load failed.")
    set_property(pawn_default_object, "VehicleData", vehicle_asset)
    save_data_asset(TEST_PAWN_BLUEPRINT_PATH, pawn_blueprint)

    compile_blueprint_asset(game_mode_blueprint)
    game_mode_class = load_blueprint_generated_class(TEST_GAME_MODE_BLUEPRINT_PATH)
    game_mode_default_object = unreal.get_default_object(game_mode_class)
    if game_mode_default_object is None:
        raise RuntimeError("AP50 GameMode CDO load failed.")
    set_property(
        game_mode_default_object,
        "ConfiguredDefaultVehiclePawnClass",
        pawn_class,
    )
    save_data_asset(TEST_GAME_MODE_BLUEPRINT_PATH, game_mode_blueprint)
    return game_mode_class


# [v1.1.0] 저장된 AP50 Pawn·GameMode CDO 직접 참조를 재로드해 검증합니다.
def validate_saved_blueprint_runtime_chain() -> Any:
    pawn_class = load_blueprint_generated_class(TEST_PAWN_BLUEPRINT_PATH)
    pawn_default_object = unreal.get_default_object(pawn_class)
    pawn_vehicle_data_path = object_path(get_property(pawn_default_object, "VehicleData"))
    if TEST_VEHICLE_PATH not in pawn_vehicle_data_path:
        raise RuntimeError(f"AP50 Pawn CDO VehicleData verification failed: {pawn_vehicle_data_path}")

    game_mode_class = load_blueprint_generated_class(TEST_GAME_MODE_BLUEPRINT_PATH)
    game_mode_default_object = unreal.get_default_object(game_mode_class)
    configured_pawn_class_path = object_path(
        get_property(game_mode_default_object, "ConfiguredDefaultVehiclePawnClass")
    )
    if TEST_PAWN_BLUEPRINT_PATH not in configured_pawn_class_path:
        raise RuntimeError(
            "AP50 GameMode ConfiguredDefaultVehiclePawnClass verification failed: "
            f"{configured_pawn_class_path}"
        )

    REPORT["asset_snapshots"]["pawn_blueprint"] = {
        "generated_class": object_path(pawn_class),
        "vehicle_data": pawn_vehicle_data_path,
    }
    REPORT["asset_snapshots"]["game_mode_blueprint"] = {
        "generated_class": object_path(game_mode_class),
        "configured_default_vehicle_pawn_class": configured_pawn_class_path,
    }
    REPORT["pawn_blueprint_valid"] = True
    REPORT["game_mode_blueprint_valid"] = True
    return game_mode_class


# [v1.0.0] DamageData의 BaseDamage와 ArmorPenetration 핵심값을 반환합니다.
def capture_damage_snapshot(damage_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(damage_asset),
        "damage_id": str(get_property(damage_asset, "DamageId")),
        "base_damage": float(get_property(damage_asset, "BaseDamage")),
        "armor_penetration": float(get_property(damage_asset, "ArmorPenetration")),
    }


# [v1.0.0] ProjectileData의 ID와 직접 DamageData 참조를 반환합니다.
def capture_projectile_snapshot(projectile_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(projectile_asset),
        "projectile_id": str(get_property(projectile_asset, "ProjectileId")),
        "default_damage_data": object_path(get_property(projectile_asset, "DefaultDamageData")),
    }


# [v1.0.0] WeaponData의 ID와 기본 ProjectileData 참조를 반환합니다.
def capture_weapon_snapshot(weapon_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(weapon_asset),
        "weapon_id": str(get_property(weapon_asset, "WeaponId")),
        "default_projectile_data": object_path(get_property(weapon_asset, "DefaultProjectileData")),
    }


# [v1.0.0] EquipmentPresetData의 ID, TurretMount와 Weapon 참조를 반환합니다.
def capture_preset_snapshot(preset_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(preset_asset),
        "equipment_id": str(get_property(preset_asset, "EquipmentId")),
        "default_turret_mount_data": object_path(get_property(preset_asset, "DefaultTurretMountData")),
        "default_weapon_data": object_path(get_property(preset_asset, "DefaultWeaponData")),
    }


# [v1.0.0] VehicleData의 방어 참조와 MountProfile 기본 장비 참조를 반환합니다.
def capture_vehicle_snapshot(vehicle_asset: Any) -> dict[str, Any]:
    mount_profiles = list(get_property(vehicle_asset, "MountProfiles"))
    return {
        "asset_path": object_path(vehicle_asset),
        "default_defense_data": object_path(get_property(vehicle_asset, "DefaultDefenseData")),
        "mount_profile_count": len(mount_profiles),
        "mount_profile_ids": [
            str(get_property(mount_profile, "MountProfileId"))
            for mount_profile in mount_profiles
        ],
        "mount_equipment_presets": [
            object_path(get_property(mount_profile, "DefaultEquipmentPresetData"))
            for mount_profile in mount_profiles
        ],
    }


# [v1.0.0] AP50 복제 DataAsset 네 개의 직접 참조 체인을 설정합니다.
def configure_ap50_asset_chain(
    projectile_asset: Any,
    weapon_asset: Any,
    preset_asset: Any,
    vehicle_asset: Any,
    ap50_damage_asset: Any,
) -> None:
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "ProjectileId", "HeavyShell_DRAP50")
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "DefaultDamageData", ap50_damage_asset)

    apply_property(TEST_WEAPON_PATH, weapon_asset, "WeaponId", "ProtoCannon_DRAP50")
    apply_property(TEST_WEAPON_PATH, weapon_asset, "DefaultProjectileData", projectile_asset)

    apply_property(TEST_PRESET_PATH, preset_asset, "EquipmentId", "HeavyCannon_DRAP50")
    apply_property(TEST_PRESET_PATH, preset_asset, "DisplayName", "Heavy Cannon AP50 Test")
    apply_property(TEST_PRESET_PATH, preset_asset, "DefaultWeaponData", weapon_asset)

    mount_profiles = list(get_property(vehicle_asset, "MountProfiles"))
    if len(mount_profiles) != 1:
        raise RuntimeError(
            "DA_TestSedan must contain exactly one MountProfile for the bounded AP50 test: "
            f"count={len(mount_profiles)}"
        )
    record_change(TEST_VEHICLE_PATH, "MountProfiles[0].DefaultEquipmentPresetData", preset_asset)
    if APPLY_CHANGES:
        first_mount_profile = mount_profiles[0]
        set_property(first_mount_profile, "DefaultEquipmentPresetData", preset_asset)
        mount_profiles[0] = first_mount_profile
        set_property(vehicle_asset, "MountProfiles", mount_profiles)


# [v1.0.0] 현재 로드된 맵에서 VehicleData 프로퍼티를 가진 차량 Actor 상태를 수집합니다.
def capture_map_vehicle_actors() -> list[dict[str, str]]:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    actor_rows: list[dict[str, str]] = []
    for level_actor in actor_subsystem.get_all_level_actors():
        if level_actor is None:
            continue
        try:
            vehicle_data = get_property(level_actor, "VehicleData")
            vehicle_fitting_data = get_property(level_actor, "VehicleFittingData")
            auto_possess_player = get_property(level_actor, "AutoPossessPlayer")
        except Exception:
            continue
        actor_rows.append(
            {
                "actor_name": level_actor.get_name(),
                "actor_label": level_actor.get_actor_label(),
                "actor_path": level_actor.get_path_name(),
                "vehicle_data": object_path(vehicle_data),
                "vehicle_fitting_data": object_path(vehicle_fitting_data),
                "auto_possess_player": str(auto_possess_player),
            }
        )
    actor_rows.sort(key=lambda actor_row: actor_row["actor_path"])
    return actor_rows


# [v1.0.0] VehicleData 경로 후보와 일치하는 맵 차량 Actor를 정확히 한 대 반환합니다.
def find_vehicle_actor(vehicle_path_candidates: set[str]) -> Any:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    matching_actors: list[Any] = []
    for level_actor in actor_subsystem.get_all_level_actors():
        if level_actor is None:
            continue
        try:
            vehicle_data_path = object_path(get_property(level_actor, "VehicleData"))
        except Exception:
            continue
        if any(candidate_path in vehicle_data_path for candidate_path in vehicle_path_candidates):
            matching_actors.append(level_actor)

    if len(matching_actors) != 1:
        raise RuntimeError(
            "Expected exactly one vehicle Actor for candidates "
            f"{sorted(vehicle_path_candidates)}, found {len(matching_actors)}."
        )
    return matching_actors[0]


# [v1.0.0] 맵 Actor의 비교 가능한 핵심 차량 연결 상태를 반환합니다.
def capture_actor_snapshot(vehicle_actor: Any) -> dict[str, str]:
    return {
        "actor_name": vehicle_actor.get_name(),
        "actor_label": vehicle_actor.get_actor_label(),
        "actor_path": vehicle_actor.get_path_name(),
        "vehicle_data": object_path(get_property(vehicle_actor, "VehicleData")),
        "vehicle_fitting_data": object_path(get_property(vehicle_actor, "VehicleFittingData")),
        "auto_possess_player": str(get_property(vehicle_actor, "AutoPossessPlayer")),
    }


# [v1.1.0] 원본 맵을 복제하고 AP50 전용 GameMode를 맵별 Override로 연결합니다.
def configure_ap50_test_map(game_mode_class: Any | None) -> None:
    source_map_asset = require_asset(SOURCE_MAP_PATH)
    del source_map_asset

    if unreal.EditorAssetLibrary.does_asset_exist(TEST_MAP_PATH):
        REPORT["updated_assets"].append(TEST_MAP_PATH)
        map_to_load = TEST_MAP_PATH
    elif not APPLY_CHANGES:
        record_change(TEST_MAP_PATH, "DuplicateAsset", SOURCE_MAP_PATH)
        map_to_load = SOURCE_MAP_PATH
    else:
        duplicated_map = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP_PATH, TEST_MAP_PATH)
        if duplicated_map is None:
            raise RuntimeError(f"Map duplication failed: {SOURCE_MAP_PATH} -> {TEST_MAP_PATH}")
        REPORT["created_assets"].append(TEST_MAP_PATH)
        map_to_load = TEST_MAP_PATH

    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(map_to_load)
    if loaded_world is None:
        raise RuntimeError(f"Map load failed: {map_to_load}")

    REPORT["map_vehicle_actors_before"] = capture_map_vehicle_actors()
    defense_target_actor = find_vehicle_actor({DEFENSE_TARGET_VEHICLE_PATH})
    REPORT["defense_target_actor_before"] = capture_actor_snapshot(defense_target_actor)

    world_settings = loaded_world.get_world_settings()
    if world_settings is None:
        raise RuntimeError("Loaded AP50 map WorldSettings is unavailable.")

    current_game_mode_path = object_path(get_property(world_settings, "DefaultGameMode"))
    record_change(
        TEST_MAP_PATH,
        "WorldSettings.DefaultGameMode",
        blueprint_generated_class_path(TEST_GAME_MODE_BLUEPRINT_PATH),
    )
    if not APPLY_CHANGES:
        REPORT["map_game_mode_override"] = current_game_mode_path
        return

    if game_mode_class is None:
        raise RuntimeError("Apply mode requires a valid AP50 GameMode generated class.")

    set_property(world_settings, "DefaultGameMode", game_mode_class)
    assigned_game_mode_path = object_path(get_property(world_settings, "DefaultGameMode"))
    if TEST_GAME_MODE_BLUEPRINT_PATH not in assigned_game_mode_path:
        raise RuntimeError(f"AP50 map GameMode Override assignment failed: {assigned_game_mode_path}")

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_subsystem is None:
        raise RuntimeError("LevelEditorSubsystem is unavailable.")
    if not level_subsystem.save_current_level():
        raise RuntimeError(f"AP50 test map save failed: {TEST_MAP_PATH}")
    REPORT["saved_assets"].append(TEST_MAP_PATH)

    reloaded_world = unreal.EditorLoadingAndSavingUtils.load_map(TEST_MAP_PATH)
    if reloaded_world is None:
        raise RuntimeError(f"Saved AP50 map reload failed: {TEST_MAP_PATH}")

    reloaded_world_settings = reloaded_world.get_world_settings()
    if reloaded_world_settings is None:
        raise RuntimeError("Reloaded AP50 map WorldSettings is unavailable.")
    reloaded_game_mode_path = object_path(get_property(reloaded_world_settings, "DefaultGameMode"))
    if TEST_GAME_MODE_BLUEPRINT_PATH not in reloaded_game_mode_path:
        raise RuntimeError(f"Saved AP50 map GameMode Override verification failed: {reloaded_game_mode_path}")

    reloaded_defense_target_actor = find_vehicle_actor({DEFENSE_TARGET_VEHICLE_PATH})
    REPORT["map_vehicle_actors_after"] = capture_map_vehicle_actors()
    REPORT["defense_target_actor_after"] = capture_actor_snapshot(reloaded_defense_target_actor)
    if REPORT["defense_target_actor_before"] != REPORT["defense_target_actor_after"]:
        raise RuntimeError("Defense target Actor changed while creating the AP50 test map.")

    REPORT["map_game_mode_override"] = reloaded_game_mode_path
    REPORT["map_connected"] = True



# [v1.0.0] 저장 후 AP50 직접 참조 체인과 원본 AP 0·AP50 피해 기준을 검증합니다.
def validate_saved_asset_chain() -> None:
    ap0_damage_asset = require_asset(AP0_DAMAGE_PATH)
    ap50_damage_asset = require_asset(AP50_DAMAGE_PATH)
    source_preset_asset = require_asset(SOURCE_PRESET_PATH)
    test_projectile_asset = require_asset(TEST_PROJECTILE_PATH)
    test_weapon_asset = require_asset(TEST_WEAPON_PATH)
    test_preset_asset = require_asset(TEST_PRESET_PATH)
    test_vehicle_asset = require_asset(TEST_VEHICLE_PATH)

    ap0_damage_snapshot = capture_damage_snapshot(ap0_damage_asset)
    ap50_damage_snapshot = capture_damage_snapshot(ap50_damage_asset)
    projectile_snapshot = capture_projectile_snapshot(test_projectile_asset)
    weapon_snapshot = capture_weapon_snapshot(test_weapon_asset)
    source_preset_snapshot = capture_preset_snapshot(source_preset_asset)
    preset_snapshot = capture_preset_snapshot(test_preset_asset)
    vehicle_snapshot = capture_vehicle_snapshot(test_vehicle_asset)

    REPORT["asset_snapshots"] = {
        "ap0_damage": ap0_damage_snapshot,
        "ap50_damage": ap50_damage_snapshot,
        "projectile": projectile_snapshot,
        "weapon": weapon_snapshot,
        "source_preset": source_preset_snapshot,
        "preset": preset_snapshot,
        "vehicle": vehicle_snapshot,
    }

    if ap0_damage_snapshot["base_damage"] != 25.0 or ap0_damage_snapshot["armor_penetration"] != 0.0:
        raise RuntimeError(f"AP 0 DamageData protection contract failed: {ap0_damage_snapshot}")
    if ap50_damage_snapshot["base_damage"] != 25.0 or ap50_damage_snapshot["armor_penetration"] != 50.0:
        raise RuntimeError(f"AP 50 DamageData contract failed: {ap50_damage_snapshot}")
    if AP50_DAMAGE_PATH not in projectile_snapshot["default_damage_data"]:
        raise RuntimeError(f"AP50 ProjectileData DamageData connection failed: {projectile_snapshot}")
    if TEST_PROJECTILE_PATH not in weapon_snapshot["default_projectile_data"]:
        raise RuntimeError(f"AP50 WeaponData ProjectileData connection failed: {weapon_snapshot}")
    if TEST_WEAPON_PATH not in preset_snapshot["default_weapon_data"]:
        raise RuntimeError(f"AP50 EquipmentPresetData WeaponData connection failed: {preset_snapshot}")
    if preset_snapshot["default_turret_mount_data"] != source_preset_snapshot["default_turret_mount_data"]:
        raise RuntimeError("AP50 EquipmentPresetData no longer preserves the HeavyCannon TurretMountData.")
    if vehicle_snapshot["mount_profile_count"] != 1:
        raise RuntimeError(f"AP50 Sedan MountProfile count is invalid: {vehicle_snapshot}")
    if TEST_PRESET_PATH not in vehicle_snapshot["mount_equipment_presets"][0]:
        raise RuntimeError(f"AP50 Sedan EquipmentPresetData connection failed: {vehicle_snapshot}")
    if vehicle_snapshot["default_defense_data"]:
        raise RuntimeError("AP50 attacker Sedan must preserve DefaultDefenseData=None.")

    REPORT["asset_chain_valid"] = True


# [v1.1.0] AP50 복제 체인, 전용 Pawn·GameMode와 격리 테스트 맵을 생성하고 원본 보호 계약을 검증합니다.
def apply_ap50_test_assets() -> None:
    REPORT["protected_hashes_before"] = capture_hashes(PROTECTED_ASSET_PATHS)

    ap50_damage_asset = require_asset(AP50_DAMAGE_PATH)
    projectile_asset = duplicate_or_load_asset(SOURCE_PROJECTILE_PATH, TEST_PROJECTILE_PATH)
    weapon_asset = duplicate_or_load_asset(SOURCE_WEAPON_PATH, TEST_WEAPON_PATH)
    preset_asset = duplicate_or_load_asset(SOURCE_PRESET_PATH, TEST_PRESET_PATH)
    vehicle_asset = duplicate_or_load_asset(SOURCE_VEHICLE_PATH, TEST_VEHICLE_PATH)
    pawn_blueprint = duplicate_or_load_asset(
        SOURCE_PAWN_BLUEPRINT_PATH,
        TEST_PAWN_BLUEPRINT_PATH,
    )
    game_mode_blueprint = create_or_load_game_mode_blueprint()

    configure_ap50_asset_chain(
        projectile_asset,
        weapon_asset,
        preset_asset,
        vehicle_asset,
        ap50_damage_asset,
    )

    if APPLY_CHANGES:
        save_data_asset(TEST_PROJECTILE_PATH, projectile_asset)
        save_data_asset(TEST_WEAPON_PATH, weapon_asset)
        save_data_asset(TEST_PRESET_PATH, preset_asset)
        save_data_asset(TEST_VEHICLE_PATH, vehicle_asset)
        validate_saved_asset_chain()
    else:
        REPORT["asset_chain_valid"] = True

    game_mode_class = configure_blueprint_runtime_chain(
        pawn_blueprint,
        game_mode_blueprint,
        vehicle_asset,
    )
    if APPLY_CHANGES:
        game_mode_class = validate_saved_blueprint_runtime_chain()

    configure_ap50_test_map(game_mode_class)

    REPORT["protected_hashes_after"] = capture_hashes(PROTECTED_ASSET_PATHS)
    if REPORT["protected_hashes_before"] != REPORT["protected_hashes_after"]:
        raise RuntimeError("One or more protected source, map, damage or defense assets changed unexpectedly.")
    REPORT["protected_contract_passed"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_ap50_test_assets()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][DefenseAP50] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("VehicleDefense AP50 asset application failed. Read report.json for details.")
