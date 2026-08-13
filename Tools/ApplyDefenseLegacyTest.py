# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-06
# Description: CF-FQ-033 DR-PIE-05 Legacy Integrity Fallback 전용 복제 맵 생성 도구
# Scope: /Game/Maps/M_VehicleDefensePIE_Legacy만 생성·수정하며 원본 VehicleData, FittingData, 무기·Projectile과 방어 테스트 맵은 읽기 전용으로 보호합니다.
# Changelog:
# - v1.0.0: M_VehicleDefensePIE를 복제하고 대상 SUV Actor를 DA_TestSUV + VehicleFittingData=None으로 연결하는 원본 비저장 경로와 SHA-256 보호 보고서를 추가.
# Migration:
# - DR-PIE-05는 /Game/Maps/M_VehicleDefensePIE_Legacy에서만 수행합니다.
# - 원본 DA_TestSUV, DA_VehicleDefense_TestSUV, DA_Fit_DefenseTestSUV와 M_VehicleDefensePIE는 수정하거나 재저장하지 않습니다.
# - CARFIGHT_DEFENSE_LEGACY_APPLY=1이 아니면 에셋이나 맵을 저장하지 않습니다.

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
TOOL_VERSION = "1.0.0"

# [v1.0.0] 명시적 Apply 실행에서만 복제 맵 저장을 허용하는 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_DEFENSE_LEGACY_APPLY", "0") == "1"

# [v1.0.0] 현재 Unreal 프로젝트의 절대 루트 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 구조화 실행 보고서를 저장할 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "VehicleDefenseLegacyApply"

# [v1.0.0] MCP와 후속 세션에서 읽을 JSON 보고서 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] DR-PIE-00~04와 AP0 검증에서 사용한 읽기 전용 방어 테스트 맵입니다.
SOURCE_MAP_PATH = "/Game/Maps/M_VehicleDefensePIE"

# [v1.0.0] Legacy Integrity Fallback 사용자 PIE에 사용할 유일한 신규 복제 맵입니다.
TEST_MAP_PATH = "/Game/Maps/M_VehicleDefensePIE_Legacy"

# [v1.0.0] DefaultDefenseData=None과 MaxHealth 100을 유지하는 Legacy 대상 SUV VehicleData입니다.
LEGACY_TARGET_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"

# [v1.0.0] 원본 방어 맵에서 교체할 대상 SUV의 현재 VehicleData입니다.
DEFENSE_TARGET_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV"

# [v1.0.0] 원본 방어 맵 대상 SUV에 연결된 방어 피팅 데이터입니다.
DEFENSE_TARGET_FITTING_PATH = "/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV"

# [v1.0.0] 런타임 플레이어 Pawn 클래스 기본값을 보호할 원본 Blueprint입니다.
SOURCE_PAWN_BLUEPRINT_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn"

# [v1.0.0] 원본 GameMode 플레이어가 사용하는 AP0 Sedan VehicleData입니다.
SOURCE_PLAYER_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan"

# [v1.0.0] AP0 플레이어 Sedan의 기본 EquipmentPresetData입니다.
SOURCE_PLAYER_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon"

# [v1.0.0] AP0 플레이어 HeavyCannon의 기본 WeaponData입니다.
SOURCE_PLAYER_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon"

# [v1.0.0] AP0 플레이어 WeaponData의 기본 ProjectileData입니다.
SOURCE_PLAYER_PROJECTILE_PATH = "/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell"

# [v1.0.0] Legacy 대상에 직접 적용될 BaseDamage 25·ArmorPenetration 0 DamageData입니다.
SOURCE_DAMAGE_PATH = "/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset"

# [v1.0.0] 이번 도구가 생성하거나 저장할 수 있는 유일한 Unreal 에셋 경로입니다.
MUTABLE_ASSET_PATHS = {
    TEST_MAP_PATH,
}

# [v1.0.0] 실행 전후 SHA-256이 같아야 하는 원본 맵·차량·피팅·공격 체인입니다.
PROTECTED_ASSET_PATHS = {
    SOURCE_MAP_PATH,
    LEGACY_TARGET_VEHICLE_PATH,
    DEFENSE_TARGET_VEHICLE_PATH,
    DEFENSE_TARGET_FITTING_PATH,
    SOURCE_PAWN_BLUEPRINT_PATH,
    SOURCE_PLAYER_VEHICLE_PATH,
    SOURCE_PLAYER_PRESET_PATH,
    SOURCE_PLAYER_WEAPON_PATH,
    SOURCE_PLAYER_PROJECTILE_PATH,
    SOURCE_DAMAGE_PATH,
}

# [v1.0.0] 실행 모드, 대상 연결, 원본 보호와 사후조건을 기록할 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "vehicle_defense_legacy_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "source_paths": {
        "map": SOURCE_MAP_PATH,
        "legacy_target_vehicle": LEGACY_TARGET_VEHICLE_PATH,
        "defense_target_vehicle": DEFENSE_TARGET_VEHICLE_PATH,
        "defense_target_fitting": DEFENSE_TARGET_FITTING_PATH,
        "player_pawn_blueprint": SOURCE_PAWN_BLUEPRINT_PATH,
        "player_vehicle": SOURCE_PLAYER_VEHICLE_PATH,
        "player_preset": SOURCE_PLAYER_PRESET_PATH,
        "player_weapon": SOURCE_PLAYER_WEAPON_PATH,
        "player_projectile": SOURCE_PLAYER_PROJECTILE_PATH,
        "damage": SOURCE_DAMAGE_PATH,
    },
    "target_paths": {
        "map": TEST_MAP_PATH,
    },
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "planned_changes": [],
    "protected_hashes_before": {},
    "protected_hashes_after": {},
    "legacy_vehicle_snapshot": {},
    "damage_snapshot": {},
    "map_vehicle_actors_before": [],
    "map_vehicle_actors_after": [],
    "target_actor_before": {},
    "target_actor_after": {},
    "map_game_mode_before": "",
    "map_game_mode_after": "",
    "legacy_vehicle_contract_valid": False,
    "damage_contract_valid": False,
    "map_connected": False,
    "protected_contract_passed": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 Legacy 테스트 맵 도구 메시지를 기록합니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][DefenseLegacy] {message}")


# [v1.0.0] PascalCase C++ 프로퍼티 이름을 Unreal Python snake_case 후보로 변환합니다.
def pascal_to_snake(name: str) -> str:
    # [v1.0.0] 첫 번째 대문자 경계까지 분리한 중간 snake_case 문자열입니다.
    first_step = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", first_step).lower()


# [v1.0.0] Reflection 프로퍼티 접근에 사용할 원본·snake_case 후보를 중복 없이 반환합니다.
def candidate_property_names(name: str) -> list[str]:
    return list(dict.fromkeys([name, pascal_to_snake(name)]))


# [v1.0.0] 원본 또는 snake_case 프로퍼티 이름으로 Unreal Reflection 값을 읽습니다.
def get_property(target: Any, property_name: str) -> Any:
    # [v1.0.0] 모든 Reflection 이름 후보가 실패했을 때 보고할 마지막 예외입니다.
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
    # [v1.0.0] 모든 Reflection 이름 후보가 실패했을 때 보고할 마지막 예외입니다.
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
        # [v1.0.0] 맵과 일반 에셋을 실제 파일로 찾기 위한 확장자입니다.
    file_extension = ".umap" if asset_path.startswith("/Game/Maps/") else ".uasset"
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'):]}{file_extension}"


# [v1.0.0] 존재하는 파일의 SHA-256을 계산하고 파일이 없으면 빈 문자열을 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
        # [v1.0.0] 보호 파일 전체 내용을 누적할 SHA-256 계산기입니다.
    digest = hashlib.sha256()

    # [v1.0.0] SHA-256 계산을 위해 읽기 전용으로 연 보호 대상 파일입니다.
    with file_path.open("rb") as source_file:
        for file_chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(file_chunk)
    return digest.hexdigest()


# [v1.0.0] 지정 Unreal 에셋 경로 집합의 실제 파일 SHA-256을 반환합니다.
def capture_hashes(asset_paths: set[str]) -> dict[str, str]:
    return {
        asset_path: sha256_file(asset_file_path(asset_path))
        for asset_path in sorted(asset_paths)
    }


# [v1.0.0] 필수 Unreal 에셋을 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    # [v1.0.0] 요청한 Unreal 경로에서 로드한 필수 에셋입니다.
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if loaded_asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return loaded_asset


# [v1.0.0] 계획된 단일 맵 변경을 구조화 보고서에 기록합니다.
def record_change(asset_path: str, property_name: str, value: Any) -> None:
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.0.0] 지정 경로가 이번 도구의 유일한 저장 허용 대상인지 검증합니다.
def ensure_mutable_asset_path(asset_path: str) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in the mutation whitelist: {asset_path}")


# [v1.0.0] Legacy 대상 VehicleData의 무방어·내구도 계약을 검증합니다.
def validate_legacy_vehicle_contract(legacy_vehicle_asset: Any) -> None:
    # [v1.0.0] Legacy Fallback을 요구하기 위해 None이어야 하는 방어 데이터 참조입니다.
    default_defense_data = get_property(legacy_vehicle_asset, "DefaultDefenseData")

    # [v1.0.0] Legacy 대상의 최대 Integrity를 제공하는 차량 내구도 설정입니다.
    durability_config = get_property(legacy_vehicle_asset, "VehicleDurabilityConfig")

    # [v1.0.0] 한 발 후 75가 되는 기준을 고정할 최대 Integrity입니다.
    maximum_integrity = float(get_property(durability_config, "MaxHealth"))

    REPORT["legacy_vehicle_snapshot"] = {
        "asset_path": object_path(legacy_vehicle_asset),
        "default_defense_data": object_path(default_defense_data),
        "maximum_integrity": maximum_integrity,
    }

    if default_defense_data is not None:
        raise RuntimeError(
            "Legacy target VehicleData must preserve DefaultDefenseData=None: "
            f"{object_path(default_defense_data)}"
        )
    if maximum_integrity != 100.0:
        raise RuntimeError(
            f"Legacy target VehicleData MaxHealth must be 100 for DR-PIE-05: {maximum_integrity}"
        )

    REPORT["legacy_vehicle_contract_valid"] = True


# [v1.0.0] 공격에 사용할 DamageData의 BaseDamage 25·AP0 계약을 검증합니다.
def validate_damage_contract(damage_asset: Any) -> None:
    # [v1.0.0] Legacy 직접 Integrity 피해의 입력 기준을 기록한 DamageData 스냅샷입니다.
    damage_snapshot = {
        "asset_path": object_path(damage_asset),
        "damage_id": str(get_property(damage_asset, "DamageId")),
        "base_damage": float(get_property(damage_asset, "BaseDamage")),
        "armor_penetration": float(get_property(damage_asset, "ArmorPenetration")),
    }
    REPORT["damage_snapshot"] = damage_snapshot

    if damage_snapshot["base_damage"] != 25.0:
        raise RuntimeError(f"Legacy test BaseDamage must be 25: {damage_snapshot}")
    if damage_snapshot["armor_penetration"] != 0.0:
        raise RuntimeError(f"Legacy test ArmorPenetration must be 0: {damage_snapshot}")

    REPORT["damage_contract_valid"] = True


# [v1.0.0] 현재 로드된 맵에서 VehicleData 프로퍼티를 가진 차량 Actor 상태를 수집합니다.
def capture_map_vehicle_actors() -> list[dict[str, str]]:
    # [v1.0.0] 현재 로드된 Level Actor를 읽을 Unreal Editor 서브시스템입니다.
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    # [v1.0.0] VehicleData 프로퍼티를 가진 모든 맵 차량 Actor의 구조화 목록입니다.
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
    # [v1.0.0] 현재 로드된 Level Actor를 검색할 Unreal Editor 서브시스템입니다.
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    # [v1.0.0] 후보 VehicleData 경로와 일치한 차량 Actor 목록입니다.
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
            "Expected exactly one target vehicle Actor for candidates "
            f"{sorted(vehicle_path_candidates)}, found {len(matching_actors)}."
        )
    return matching_actors[0]


# [v1.0.0] 맵 Actor의 비교 가능한 차량·피팅·Possess 상태를 반환합니다.
def capture_actor_snapshot(vehicle_actor: Any) -> dict[str, str]:
    return {
        "actor_name": vehicle_actor.get_name(),
        "actor_label": vehicle_actor.get_actor_label(),
        "actor_path": vehicle_actor.get_path_name(),
        "vehicle_data": object_path(get_property(vehicle_actor, "VehicleData")),
        "vehicle_fitting_data": object_path(get_property(vehicle_actor, "VehicleFittingData")),
        "auto_possess_player": str(get_property(vehicle_actor, "AutoPossessPlayer")),
    }


# [v1.0.0] 원본 맵을 복제하고 대상 SUV를 Legacy VehicleData·무피팅 상태로 연결합니다.
def configure_legacy_test_map(legacy_vehicle_asset: Any) -> None:
    ensure_mutable_asset_path(TEST_MAP_PATH)
    require_asset(SOURCE_MAP_PATH)

    if unreal.EditorAssetLibrary.does_asset_exist(TEST_MAP_PATH):
        REPORT["updated_assets"].append(TEST_MAP_PATH)

        # [v1.0.0] 기존 Legacy 복제본을 재검증할 때 로드할 맵 경로입니다.
        map_to_load = TEST_MAP_PATH
    elif not APPLY_CHANGES:
        record_change(TEST_MAP_PATH, "DuplicateAsset", SOURCE_MAP_PATH)

        # [v1.0.0] Dry Run에서 구조만 검증하기 위해 로드할 읽기 전용 원본 맵 경로입니다.
        map_to_load = SOURCE_MAP_PATH
    else:
        # [v1.0.0] 원본 방어 맵을 저장 없이 복제한 Legacy 테스트 전용 World 에셋입니다.
        duplicated_map = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP_PATH, TEST_MAP_PATH)
        if duplicated_map is None:
            raise RuntimeError(f"Map duplication failed: {SOURCE_MAP_PATH} -> {TEST_MAP_PATH}")
        REPORT["created_assets"].append(TEST_MAP_PATH)

        # [v1.0.0] 새로 생성한 Legacy 복제 맵을 수정·저장하기 위해 로드할 경로입니다.
        map_to_load = TEST_MAP_PATH

    # [v1.0.0] 대상 Actor와 WorldSettings를 확인하기 위해 로드한 원본 또는 Legacy 복제 월드입니다.
    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(map_to_load)
    if loaded_world is None:
        raise RuntimeError(f"Map load failed: {map_to_load}")

    # [v1.0.0] GameMode가 원본과 같게 유지되는지 비교할 현재 월드 설정입니다.
    world_settings = loaded_world.get_world_settings()
    if world_settings is None:
        raise RuntimeError("Loaded Legacy test map WorldSettings is unavailable.")

    REPORT["map_game_mode_before"] = object_path(get_property(world_settings, "DefaultGameMode"))
    REPORT["map_vehicle_actors_before"] = capture_map_vehicle_actors()

    # [v1.0.0] 방어 SUV에서 Legacy SUV로 교체할 유일한 비소유 대상 차량 Actor입니다.
    target_actor = find_vehicle_actor(
        {
            DEFENSE_TARGET_VEHICLE_PATH,
            LEGACY_TARGET_VEHICLE_PATH,
        }
    )
    REPORT["target_actor_before"] = capture_actor_snapshot(target_actor)

    # [v1.0.0] 대상 차량이 플레이어 차량으로 바뀌지 않았음을 확인할 기존 Possess 설정입니다.
    original_auto_possess = str(get_property(target_actor, "AutoPossessPlayer"))
    record_change(TEST_MAP_PATH, f"{target_actor.get_name()}.VehicleData", legacy_vehicle_asset)
    record_change(TEST_MAP_PATH, f"{target_actor.get_name()}.VehicleFittingData", None)

    if not APPLY_CHANGES:
        return

    set_property(target_actor, "VehicleData", legacy_vehicle_asset)
    set_property(target_actor, "VehicleFittingData", None)

    # [v1.0.0] 저장 전에 다시 읽은 Legacy 대상 VehicleData 전체 경로입니다.
    assigned_vehicle_path = object_path(get_property(target_actor, "VehicleData"))

    # [v1.0.0] 방어 피팅 제거가 적용됐는지 확인할 현재 VehicleFittingData 경로입니다.
    assigned_fitting_path = object_path(get_property(target_actor, "VehicleFittingData"))
    if LEGACY_TARGET_VEHICLE_PATH not in assigned_vehicle_path:
        raise RuntimeError(f"Legacy target VehicleData assignment failed: {assigned_vehicle_path}")
    if assigned_fitting_path:
        raise RuntimeError(f"Legacy target VehicleFittingData must be None: {assigned_fitting_path}")
    if str(get_property(target_actor, "AutoPossessPlayer")) != original_auto_possess:
        raise RuntimeError("Legacy target AutoPossessPlayer changed unexpectedly.")

    # [v1.0.0] 현재 Legacy 복제 맵만 저장할 Unreal Level Editor 서브시스템입니다.
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_subsystem is None:
        raise RuntimeError("LevelEditorSubsystem is unavailable.")
    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Legacy test map save failed: {TEST_MAP_PATH}")
    REPORT["saved_assets"].append(TEST_MAP_PATH)

    # [v1.0.0] 디스크 저장 사후조건을 확인하기 위해 다시 로드한 Legacy 테스트 월드입니다.
    reloaded_world = unreal.EditorLoadingAndSavingUtils.load_map(TEST_MAP_PATH)
    if reloaded_world is None:
        raise RuntimeError(f"Saved Legacy test map reload failed: {TEST_MAP_PATH}")

    # [v1.0.0] 저장 후 GameMode 보존을 확인할 다시 로드된 월드 설정입니다.
    reloaded_world_settings = reloaded_world.get_world_settings()
    if reloaded_world_settings is None:
        raise RuntimeError("Reloaded Legacy test map WorldSettings is unavailable.")

    # [v1.0.0] 저장 후 Legacy VehicleData로 식별한 유일한 대상 차량 Actor입니다.
    reloaded_target_actor = find_vehicle_actor({LEGACY_TARGET_VEHICLE_PATH})
    REPORT["map_vehicle_actors_after"] = capture_map_vehicle_actors()
    REPORT["target_actor_after"] = capture_actor_snapshot(reloaded_target_actor)
    REPORT["map_game_mode_after"] = object_path(get_property(reloaded_world_settings, "DefaultGameMode"))

    if REPORT["target_actor_after"]["vehicle_fitting_data"]:
        raise RuntimeError(
            "Reloaded Legacy target unexpectedly contains VehicleFittingData: "
            f"{REPORT['target_actor_after']['vehicle_fitting_data']}"
        )
    if REPORT["target_actor_after"]["auto_possess_player"] != original_auto_possess:
        raise RuntimeError("Reloaded Legacy target AutoPossessPlayer changed unexpectedly.")
    if REPORT["map_game_mode_after"] != REPORT["map_game_mode_before"]:
        raise RuntimeError(
            "Legacy map GameMode changed unexpectedly: "
            f"{REPORT['map_game_mode_before']} -> {REPORT['map_game_mode_after']}"
        )

    REPORT["map_connected"] = True


# [v1.0.0] Legacy 테스트 맵을 준비하고 원본 보호 계약을 검증합니다.
def apply_legacy_test_map() -> None:
    REPORT["protected_hashes_before"] = capture_hashes(PROTECTED_ASSET_PATHS)

    # [v1.0.0] 복제 맵 대상 Actor에 연결할 DefaultDefenseData=None 원본 SUV VehicleData입니다.
    legacy_vehicle_asset = require_asset(LEGACY_TARGET_VEHICLE_PATH)

    # [v1.0.0] 플레이어 HeavyShell이 전달할 BaseDamage 25·AP0 원본 DamageData입니다.
    damage_asset = require_asset(SOURCE_DAMAGE_PATH)
    require_asset(DEFENSE_TARGET_VEHICLE_PATH)
    require_asset(DEFENSE_TARGET_FITTING_PATH)
    require_asset(SOURCE_PAWN_BLUEPRINT_PATH)
    require_asset(SOURCE_PLAYER_VEHICLE_PATH)
    require_asset(SOURCE_PLAYER_PRESET_PATH)
    require_asset(SOURCE_PLAYER_WEAPON_PATH)
    require_asset(SOURCE_PLAYER_PROJECTILE_PATH)

    validate_legacy_vehicle_contract(legacy_vehicle_asset)
    validate_damage_contract(damage_asset)
    configure_legacy_test_map(legacy_vehicle_asset)

    REPORT["protected_hashes_after"] = capture_hashes(PROTECTED_ASSET_PATHS)
    if REPORT["protected_hashes_before"] != REPORT["protected_hashes_after"]:
        raise RuntimeError("One or more protected source, vehicle, fitting or attack assets changed unexpectedly.")
    REPORT["protected_contract_passed"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_legacy_test_map()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][DefenseLegacy] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("VehicleDefense Legacy test map application failed. Read report.json for details.")
