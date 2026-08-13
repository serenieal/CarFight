# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-06
# Description: CF-FQ-033 DR-PIE-06 Launcher·Projectile 회귀용 Salvo·Ripple 격리 에셋과 맵 생성 도구
# Scope: /Game/CarFight/Tests/LauncherRegression과 /Game/Maps/TestMap_DRSalvo·TestMap_DRRipple만 생성·수정합니다.
# Changelog:
# - v1.0.0: 원본 Salvo WeaponData 보호, Ripple 4발·0.15초 복제 체인, 전용 SUV VehicleData 2개, TestMap 복제 맵 2개와 SHA-256 보호 보고서 추가.
# Migration:
# - 원본 TestMap, DA_RocketLauncher, RocketLauncher, DA_TestSUV, DA_RocketBody와 DA_Rocket_PropTest는 읽기 전용으로 유지합니다.
# - DR-PIE-06 Salvo는 TestMap_DRSalvo, Ripple은 TestMap_DRRipple에서 수행합니다.
# - CARFIGHT_LAUNCHER_REGRESSION_APPLY=1이 아니면 Unreal 에셋이나 맵을 저장하지 않습니다.

from __future__ import annotations

import hashlib
import json
import math
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.0.0] 실행 보고서와 문서에서 식별할 도구 버전입니다.
TOOL_VERSION = "1.0.0"

# [v1.0.0] 명시적 Apply 실행에서만 테스트 에셋과 맵 저장을 허용하는 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_LAUNCHER_REGRESSION_APPLY", "0") == "1"

# [v1.0.0] 현재 Unreal 프로젝트의 절대 루트 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 구조화 실행 보고서를 저장할 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "LauncherRegressionApply"

# [v1.0.0] MCP와 후속 세션에서 읽을 JSON 보고서 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] DR-PIE-06 전용 DataAsset을 저장할 Unreal 폴더입니다.
TEST_ASSET_FOLDER = "/Game/CarFight/Tests/LauncherRegression"

# [v1.0.0] 기존 Launcher 사용자 검증 환경인 읽기 전용 원본 맵입니다.
SOURCE_MAP_PATH = "/Game/Maps/TestMap"

# [v1.0.0] 현재 저장값 Salvo 4발을 검증할 격리 맵입니다.
SALVO_MAP_PATH = "/Game/Maps/TestMap_DRSalvo"

# [v1.0.0] Ripple 4발·0.15초를 검증할 격리 맵입니다.
RIPPLE_MAP_PATH = "/Game/Maps/TestMap_DRRipple"

# [v1.0.0] 현재 Salvo 설정을 소유하는 읽기 전용 원본 WeaponData입니다.
SOURCE_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher"

# [v1.0.0] 원본 WeaponData와 TurretMountData를 연결하는 읽기 전용 EquipmentPresetData입니다.
SOURCE_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher"

# [v1.0.0] 네 개 Muzzle와 RocketLauncher 시각 구성을 소유하는 읽기 전용 TurretMountData입니다.
SOURCE_TURRET_PATH = "/Game/CarFight/Weapons/Data/TurretMounts/DA_RocketBody"

# [v1.0.0] 추진·Trail·Thruster·Impact FX를 소유하는 읽기 전용 ProjectileData입니다.
SOURCE_PROJECTILE_PATH = "/Game/CarFight/Weapons/Data/ProjectileDefs/DA_Rocket_PropTest"

# [v1.0.0] 테스트 SUV 복제의 외형·하드포인트·주행 기준 원본 VehicleData입니다.
SOURCE_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"

# [v1.0.0] 원본 Pawn 클래스 기본값과 컴포넌트 구성을 보호할 Blueprint입니다.
SOURCE_PAWN_BLUEPRINT_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn"

# [v1.0.0] TestMap의 다른 차량 장비 참조를 보호할 HeavyCannon EquipmentPresetData입니다.
SOURCE_HEAVY_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon"

# [v1.0.0] Ripple 발사 패턴만 분리할 테스트 전용 WeaponData입니다.
RIPPLE_WEAPON_PATH = f"{TEST_ASSET_FOLDER}/DA_RocketLauncher_DRRipple"

# [v1.0.0] Ripple WeaponData를 원본 RocketLauncher TurretMountData와 묶을 테스트 전용 EquipmentPresetData입니다.
RIPPLE_PRESET_PATH = f"{TEST_ASSET_FOLDER}/RocketLauncher_DRRipple"

# [v1.0.0] 원본 Salvo EquipmentPresetData를 장착할 테스트 전용 SUV VehicleData입니다.
SALVO_VEHICLE_PATH = f"{TEST_ASSET_FOLDER}/DA_TestSUV_DRSalvo"

# [v1.0.0] Ripple EquipmentPresetData를 장착할 테스트 전용 SUV VehicleData입니다.
RIPPLE_VEHICLE_PATH = f"{TEST_ASSET_FOLDER}/DA_TestSUV_DRRipple"

# [v1.0.0] 원본 TurretMountData의 RocketLauncher 시각 계약을 확인할 Pitch StaticMesh 경로 조각입니다.
ROCKET_PITCH_MESH_PATH_FRAGMENT = "/Game/CarFight/Weapons/Turrets/RocketLauncher/Meshes/RocketLauncherPitch"

# [v1.0.0] 기존 CF-FQ-029 TestMap Launcher 검증 슬롯의 안정된 Actor 인스턴스 이름입니다.
SOURCE_LAUNCHER_ACTOR_NAME = "BP_CFVehiclePawn_C_1"

# [v1.0.0] TestMap Launcher 검증 슬롯이 원본과 같은 위치인지 확인할 월드 좌표입니다.
EXPECTED_LAUNCHER_ACTOR_LOCATION = (1725.0, 48.0, 0.0)

# [v1.0.0] 원본 TurretMountData에서 유지해야 하는 사용자 검증 Muzzle 순서입니다.
EXPECTED_MUZZLE_ORDER = ["Muzzle_1", "Muzzle_4", "Muzzle_2", "Muzzle_3"]

# [v1.0.0] 이번 도구가 생성하거나 저장할 수 있는 유일한 Unreal 에셋 경로입니다.
MUTABLE_ASSET_PATHS = {
    RIPPLE_WEAPON_PATH,
    RIPPLE_PRESET_PATH,
    SALVO_VEHICLE_PATH,
    RIPPLE_VEHICLE_PATH,
    SALVO_MAP_PATH,
    RIPPLE_MAP_PATH,
}

# [v1.0.0] 실행 전후 SHA-256이 같아야 하는 원본 맵·차량·Pawn·Launcher·Projectile 에셋입니다.
PROTECTED_ASSET_PATHS = {
    SOURCE_MAP_PATH,
    SOURCE_WEAPON_PATH,
    SOURCE_PRESET_PATH,
    SOURCE_TURRET_PATH,
    SOURCE_PROJECTILE_PATH,
    SOURCE_VEHICLE_PATH,
    SOURCE_PAWN_BLUEPRINT_PATH,
    SOURCE_HEAVY_PRESET_PATH,
}

# [v1.0.0] 실행 모드, 직접 참조 체인, 맵 Actor, 보호 계약과 오류를 기록할 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "launcher_regression_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "source_paths": {
        "map": SOURCE_MAP_PATH,
        "weapon": SOURCE_WEAPON_PATH,
        "preset": SOURCE_PRESET_PATH,
        "turret": SOURCE_TURRET_PATH,
        "projectile": SOURCE_PROJECTILE_PATH,
        "vehicle": SOURCE_VEHICLE_PATH,
        "pawn_blueprint": SOURCE_PAWN_BLUEPRINT_PATH,
        "heavy_preset": SOURCE_HEAVY_PRESET_PATH,
    },
    "target_paths": {
        "ripple_weapon": RIPPLE_WEAPON_PATH,
        "ripple_preset": RIPPLE_PRESET_PATH,
        "salvo_vehicle": SALVO_VEHICLE_PATH,
        "ripple_vehicle": RIPPLE_VEHICLE_PATH,
        "salvo_map": SALVO_MAP_PATH,
        "ripple_map": RIPPLE_MAP_PATH,
    },
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "planned_changes": [],
    "protected_hashes_before": {},
    "protected_hashes_after": {},
    "source_snapshots": {},
    "test_snapshots": {},
    "source_map_vehicle_actors": [],
    "salvo_map_vehicle_actors": [],
    "ripple_map_vehicle_actors": [],
    "salvo_player_actor": {},
    "ripple_player_actor": {},
    "source_contract_valid": False,
    "salvo_chain_valid": False,
    "ripple_chain_valid": False,
    "salvo_map_connected": False,
    "ripple_map_connected": False,
    "protected_contract_passed": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 Launcher 회귀 도구 메시지를 기록합니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][LauncherRegression] {message}")


# [v1.0.0] PascalCase C++ 프로퍼티 이름을 Unreal Python snake_case 후보로 변환합니다.
def pascal_to_snake(name: str) -> str:
    # [v1.0.0] 첫 번째 대문자 경계를 분리한 중간 문자열입니다.
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
    # [v1.0.0] 요청 경로에서 로드한 필수 Unreal 에셋입니다.
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if loaded_asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return loaded_asset


# [v1.0.0] 지정 경로가 이번 도구의 저장 허용 대상인지 검증합니다.
def ensure_mutable_asset_path(asset_path: str) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in the mutation whitelist: {asset_path}")


# [v1.0.0] 계획된 단일 에셋 또는 맵 변경을 구조화 보고서에 기록합니다.
def record_change(asset_path: str, property_name: str, value: Any) -> None:
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.0.0] Unreal Python enum 타입과 멤버를 대소문자 후보로 해석합니다.
def resolve_enum_value(enum_type_name: str, member_names: list[str]) -> Any:
    # [v1.0.0] Unreal Python에 노출된 enum 타입입니다.
    enum_type = getattr(unreal, enum_type_name, None)
    if enum_type is None:
        raise RuntimeError(f"Unreal enum type is unavailable: {enum_type_name}")

    for member_name in member_names:
        # [v1.0.0] 현재 이름 후보로 조회한 enum 멤버입니다.
        enum_member = getattr(enum_type, member_name, None)
        if enum_member is not None:
            return enum_member
    raise RuntimeError(
        f"Unreal enum member is unavailable: {enum_type_name}.{member_names}"
    )


# [v1.0.0] Unreal enum 문자열과 기대 멤버 이름을 영문·숫자로 정규화해 포함 여부를 반환합니다.
def enum_text_contains(enum_value_text: str, expected_member_name: str) -> bool:
    # [v1.0.0] Unreal Python enum 출력에서 대소문자·밑줄·구두점을 제거한 비교 문자열입니다.
    normalized_enum_text = re.sub(r"[^a-z0-9]", "", enum_value_text.lower())

    # [v1.0.0] C++ enum 멤버 이름에서 대소문자·밑줄·구두점을 제거한 기대 문자열입니다.
    normalized_member_name = re.sub(r"[^a-z0-9]", "", expected_member_name.lower())
    return normalized_member_name in normalized_enum_text


# [v1.0.0] 부동소수점 계약을 작은 허용 오차로 비교합니다.
def require_nearly_equal(actual_value: float, expected_value: float, label: str) -> None:
    if not math.isclose(actual_value, expected_value, rel_tol=0.0, abs_tol=0.0001):
        raise RuntimeError(f"{label} mismatch: expected={expected_value}, actual={actual_value}")


# [v1.0.0] 읽기 전용 원본을 테스트 경로에 복제하거나 기존 복제본을 로드합니다.
def duplicate_or_load_asset(source_path: str, target_path: str) -> Any:
    ensure_mutable_asset_path(target_path)

    # [v1.0.0] 복제 입력으로 사용할 읽기 전용 원본 에셋입니다.
    source_asset = require_asset(source_path)
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        # [v1.0.0] 재실행 시 연결을 다시 검증·보정할 기존 테스트 에셋입니다.
        target_asset = require_asset(target_path)
        REPORT["updated_assets"].append(target_path)
        return target_asset

    if not APPLY_CHANGES:
        record_change(target_path, "DuplicateAsset", source_path)
        return source_asset

    if not unreal.EditorAssetLibrary.does_directory_exist(TEST_ASSET_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(TEST_ASSET_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {TEST_ASSET_FOLDER}")

    # [v1.0.0] 원본을 테스트 전용 경로에 복제한 Unreal 에셋입니다.
    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"Asset duplication failed: {source_path} -> {target_path}")
    REPORT["created_assets"].append(target_path)
    return duplicated_asset


# [v1.0.0] Apply 모드에서 화이트리스트된 DataAsset을 강제로 저장합니다.
def save_data_asset(asset_path: str, asset: Any) -> None:
    if not APPLY_CHANGES:
        return
    ensure_mutable_asset_path(asset_path)
    if asset_path in {SALVO_MAP_PATH, RIPPLE_MAP_PATH}:
        raise RuntimeError(f"Map must be saved through LevelEditorSubsystem: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.0.0] WeaponData의 Launcher 패턴과 직접 Projectile 참조를 기록합니다.
def capture_weapon_snapshot(weapon_asset: Any) -> dict[str, Any]:
    # [v1.0.0] 현재 WeaponData에 저장된 Launcher 발사 패턴 구조체입니다.
    pattern_config = get_property(weapon_asset, "LauncherFirePatternConfig")

    # [v1.0.0] 현재 WeaponData에 저장된 Launcher Release 구조체입니다.
    release_config = get_property(weapon_asset, "LauncherReleaseConfig")
    return {
        "asset_path": object_path(weapon_asset),
        "weapon_id": str(get_property(weapon_asset, "WeaponId")),
        "fire_pattern": str(get_property(pattern_config, "FirePattern")),
        "projectile_count_per_trigger": int(get_property(pattern_config, "ProjectileCountPerTrigger")),
        "inter_muzzle_delay_seconds": float(get_property(pattern_config, "InterMuzzleDelaySeconds")),
        "maximum_simultaneous_launch_count": int(get_property(pattern_config, "MaximumSimultaneousLaunchCount")),
        "sequence_failure_policy": str(get_property(pattern_config, "SequenceFailurePolicy")),
        "cooldown_start_policy": str(get_property(pattern_config, "CooldownStartPolicy")),
        "release_mode": str(get_property(release_config, "ReleaseMode")),
        "default_projectile_data": object_path(get_property(weapon_asset, "DefaultProjectileData")),
    }


# [v1.0.0] EquipmentPresetData의 TurretMount와 Weapon 직접 참조를 기록합니다.
def capture_preset_snapshot(preset_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(preset_asset),
        "equipment_id": str(get_property(preset_asset, "EquipmentId")),
        "default_turret_mount_data": object_path(get_property(preset_asset, "DefaultTurretMountData")),
        "default_weapon_data": object_path(get_property(preset_asset, "DefaultWeaponData")),
    }


# [v1.0.0] VehicleData의 방어 참조와 MountProfile 장비 참조를 기록합니다.
def capture_vehicle_snapshot(vehicle_asset: Any) -> dict[str, Any]:
    # [v1.0.0] VehicleData에 저장된 모든 장착 프로파일입니다.
    mount_profiles = list(get_property(vehicle_asset, "MountProfiles"))
    return {
        "asset_path": object_path(vehicle_asset),
        "default_defense_data": object_path(get_property(vehicle_asset, "DefaultDefenseData")),
        "mount_profile_count": len(mount_profiles),
        "mount_equipment_presets": [
            object_path(get_property(mount_profile, "DefaultEquipmentPresetData"))
            for mount_profile in mount_profiles
        ],
    }


# [v1.0.0] TurretMountData의 필수 Muzzle 순서와 RocketLauncher PitchMesh를 기록합니다.
def capture_turret_snapshot(turret_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(turret_asset),
        "muzzle_socket_names": [str(value) for value in list(get_property(turret_asset, "MuzzleSocketNames"))],
        "require_all_muzzles": bool(get_property(turret_asset, "bRequireAllMuzzles")),
        "pitch_mesh": object_path(get_property(turret_asset, "TurretPitchMesh")),
    }


# [v1.0.0] ProjectileData의 추진·Trail·Thruster·Impact FX 기준을 기록합니다.
def capture_projectile_snapshot(projectile_asset: Any) -> dict[str, Any]:
    # [v1.0.0] Rocket 추진 시간·가속·상한을 제공하는 구조체입니다.
    propulsion_config = get_property(projectile_asset, "PropulsionConfig")

    # [v1.0.0] Projectile 비행 Trail의 활성·Niagara·부착 설정입니다.
    trail_settings = get_property(projectile_asset, "TrailFxSettings")

    # [v1.0.0] Motor Burning 동안 재생할 Thruster FX 설정입니다.
    thruster_settings = get_property(projectile_asset, "ThrusterFxSettings")
    return {
        "asset_path": object_path(projectile_asset),
        "projectile_id": str(get_property(projectile_asset, "ProjectileId")),
        "initial_speed": float(get_property(projectile_asset, "InitialSpeed")),
        "life_time_seconds": float(get_property(projectile_asset, "LifeTimeSeconds")),
        "gravity_scale": float(get_property(projectile_asset, "GravityScale")),
        "use_propulsion": bool(get_property(propulsion_config, "bUsePropulsion")),
        "ignition_delay_seconds": float(get_property(propulsion_config, "IgnitionDelaySeconds")),
        "burn_duration_seconds": float(get_property(propulsion_config, "BurnDurationSeconds")),
        "thrust_acceleration": float(get_property(propulsion_config, "ThrustAccelerationCmPerSecSq")),
        "maximum_propelled_speed": float(get_property(propulsion_config, "MaximumPropelledSpeed")),
        "trail_enabled": bool(get_property(trail_settings, "bEnabled")),
        "trail_system": object_path(get_property(trail_settings, "NiagaraSystem")),
        "thruster_enabled": bool(get_property(thruster_settings, "bEnabled")),
        "thruster_system": object_path(get_property(thruster_settings, "NiagaraSystem")),
        "impact_fx_data": object_path(get_property(projectile_asset, "DefaultImpactFxData")),
    }


# [v1.0.0] 원본 Salvo·Muzzle·Projectile 추진·FX 계약을 검증합니다.
def validate_source_contracts(
    source_weapon: Any,
    source_preset: Any,
    source_turret: Any,
    source_projectile: Any,
    source_vehicle: Any,
) -> None:
    # [v1.0.0] 원본 Salvo WeaponData 저장값입니다.
    source_weapon_snapshot = capture_weapon_snapshot(source_weapon)

    # [v1.0.0] 원본 RocketLauncher EquipmentPresetData 저장값입니다.
    source_preset_snapshot = capture_preset_snapshot(source_preset)

    # [v1.0.0] 원본 RocketLauncher TurretMountData 저장값입니다.
    source_turret_snapshot = capture_turret_snapshot(source_turret)

    # [v1.0.0] 원본 Rocket ProjectileData 추진·FX 저장값입니다.
    source_projectile_snapshot = capture_projectile_snapshot(source_projectile)

    # [v1.0.0] 테스트 SUV 복제 기준 VehicleData 저장값입니다.
    source_vehicle_snapshot = capture_vehicle_snapshot(source_vehicle)

    REPORT["source_snapshots"] = {
        "weapon": source_weapon_snapshot,
        "preset": source_preset_snapshot,
        "turret": source_turret_snapshot,
        "projectile": source_projectile_snapshot,
        "vehicle": source_vehicle_snapshot,
    }

    if not enum_text_contains(source_weapon_snapshot["fire_pattern"], "Salvo"):
        raise RuntimeError(f"Source RocketLauncher must remain Salvo: {source_weapon_snapshot}")
    if source_weapon_snapshot["projectile_count_per_trigger"] != 4:
        raise RuntimeError(f"Source Salvo projectile count must be 4: {source_weapon_snapshot}")
    if source_weapon_snapshot["maximum_simultaneous_launch_count"] != 4:
        raise RuntimeError(f"Source Salvo simultaneous count must be 4: {source_weapon_snapshot}")
    if not enum_text_contains(source_weapon_snapshot["sequence_failure_policy"], "ContinueRemaining"):
        raise RuntimeError(f"Source sequence failure policy changed: {source_weapon_snapshot}")
    if not enum_text_contains(source_weapon_snapshot["cooldown_start_policy"], "SequenceCompleted"):
        raise RuntimeError(f"Source cooldown policy changed: {source_weapon_snapshot}")
    if not enum_text_contains(source_weapon_snapshot["release_mode"], "Direct"):
        raise RuntimeError(f"Source release mode must remain Direct: {source_weapon_snapshot}")
    if SOURCE_PROJECTILE_PATH not in source_weapon_snapshot["default_projectile_data"]:
        raise RuntimeError(f"Source WeaponData ProjectileData connection failed: {source_weapon_snapshot}")
    if SOURCE_TURRET_PATH not in source_preset_snapshot["default_turret_mount_data"]:
        raise RuntimeError(f"Source preset TurretMountData connection failed: {source_preset_snapshot}")
    if SOURCE_WEAPON_PATH not in source_preset_snapshot["default_weapon_data"]:
        raise RuntimeError(f"Source preset WeaponData connection failed: {source_preset_snapshot}")
    if source_turret_snapshot["muzzle_socket_names"] != EXPECTED_MUZZLE_ORDER:
        raise RuntimeError(f"RocketLauncher Muzzle order changed: {source_turret_snapshot}")
    if not source_turret_snapshot["require_all_muzzles"]:
        raise RuntimeError("RocketLauncher must require all four Muzzle sockets.")
    if ROCKET_PITCH_MESH_PATH_FRAGMENT not in source_turret_snapshot["pitch_mesh"]:
        raise RuntimeError(f"RocketLauncher PitchMesh changed: {source_turret_snapshot}")
    if not source_projectile_snapshot["use_propulsion"]:
        raise RuntimeError(f"Rocket propulsion must be enabled: {source_projectile_snapshot}")

    require_nearly_equal(source_projectile_snapshot["initial_speed"], 3000.0, "InitialSpeed")
    require_nearly_equal(source_projectile_snapshot["ignition_delay_seconds"], 0.05, "IgnitionDelaySeconds")
    require_nearly_equal(source_projectile_snapshot["burn_duration_seconds"], 1.0, "BurnDurationSeconds")
    require_nearly_equal(source_projectile_snapshot["thrust_acceleration"], 9000.0, "ThrustAcceleration")
    require_nearly_equal(source_projectile_snapshot["maximum_propelled_speed"], 10000.0, "MaximumPropelledSpeed")

    if not source_projectile_snapshot["trail_enabled"] or not source_projectile_snapshot["trail_system"]:
        raise RuntimeError(f"Rocket Trail FX contract failed: {source_projectile_snapshot}")
    if not source_projectile_snapshot["thruster_enabled"] or not source_projectile_snapshot["thruster_system"]:
        raise RuntimeError(f"Rocket Thruster FX contract failed: {source_projectile_snapshot}")
    if not source_projectile_snapshot["impact_fx_data"]:
        raise RuntimeError(f"Rocket Impact FX contract failed: {source_projectile_snapshot}")
    if source_vehicle_snapshot["mount_profile_count"] != 1:
        raise RuntimeError(f"DA_TestSUV must contain one MountProfile: {source_vehicle_snapshot}")

    # [v1.0.0] Apply 전에도 Unreal Python enum 노출이 유효한지 확인할 Ripple enum 값입니다.
    resolve_enum_value("CFLauncherFirePattern", ["RIPPLE", "Ripple"])

    # [v1.0.0] 테스트 맵에서 기존 차량의 AutoPossess를 해제할 enum 값입니다.
    resolve_enum_value("AutoReceiveInput", ["DISABLED", "Disabled"])

    # [v1.0.0] 테스트 맵의 RocketLauncher 차량을 Player 0으로 고정할 enum 값입니다.
    resolve_enum_value("AutoReceiveInput", ["PLAYER0", "Player0"])
    REPORT["source_contract_valid"] = True


# [v1.0.0] Ripple WeaponData와 Salvo·Ripple VehicleData 직접 참조를 설정합니다.
def configure_test_asset_chains(
    ripple_weapon: Any,
    ripple_preset: Any,
    salvo_vehicle: Any,
    ripple_vehicle: Any,
    source_weapon: Any,
    source_preset: Any,
    source_turret: Any,
) -> None:
    record_change(RIPPLE_WEAPON_PATH, "WeaponId", "Proto_RocketLauncher_DRRipple")
    record_change(RIPPLE_WEAPON_PATH, "LauncherFirePatternConfig.FirePattern", "Ripple")
    record_change(RIPPLE_WEAPON_PATH, "LauncherFirePatternConfig.ProjectileCountPerTrigger", 4)
    record_change(RIPPLE_WEAPON_PATH, "LauncherFirePatternConfig.InterMuzzleDelaySeconds", 0.15)
    record_change(RIPPLE_WEAPON_PATH, "LauncherFirePatternConfig.MaximumSimultaneousLaunchCount", 1)
    record_change(RIPPLE_PRESET_PATH, "EquipmentId", "RocketLauncher_DRRipple")
    record_change(RIPPLE_PRESET_PATH, "DefaultWeaponData", ripple_weapon)
    record_change(SALVO_VEHICLE_PATH, "MountProfiles[0].DefaultEquipmentPresetData", source_preset)
    record_change(RIPPLE_VEHICLE_PATH, "MountProfiles[0].DefaultEquipmentPresetData", ripple_preset)

    if not APPLY_CHANGES:
        REPORT["salvo_chain_valid"] = True
        REPORT["ripple_chain_valid"] = True
        return

    set_property(ripple_weapon, "WeaponId", "Proto_RocketLauncher_DRRipple")

    # [v1.0.0] 원본 Salvo 설정을 복제한 뒤 Ripple 관련 필드만 변경할 구조체입니다.
    ripple_pattern_config = get_property(ripple_weapon, "LauncherFirePatternConfig")

    # [v1.0.0] Unreal Python에서 해석한 Ripple enum 값입니다.
    ripple_enum_value = resolve_enum_value("CFLauncherFirePattern", ["RIPPLE", "Ripple"])
    set_property(ripple_pattern_config, "FirePattern", ripple_enum_value)
    set_property(ripple_pattern_config, "ProjectileCountPerTrigger", 4)
    set_property(ripple_pattern_config, "InterMuzzleDelaySeconds", 0.15)
    set_property(ripple_pattern_config, "MaximumSimultaneousLaunchCount", 1)
    set_property(ripple_weapon, "LauncherFirePatternConfig", ripple_pattern_config)

    set_property(ripple_preset, "EquipmentId", "RocketLauncher_DRRipple")
    set_property(ripple_preset, "DisplayName", "Rocket Launcher Ripple Regression")
    set_property(ripple_preset, "DefaultTurretMountData", source_turret)
    set_property(ripple_preset, "DefaultWeaponData", ripple_weapon)

    # [v1.0.0] Salvo 테스트 SUV에 저장된 단일 MountProfile 배열입니다.
    salvo_mount_profiles = list(get_property(salvo_vehicle, "MountProfiles"))
    if len(salvo_mount_profiles) != 1:
        raise RuntimeError(f"Salvo test SUV MountProfile count must be 1: {len(salvo_mount_profiles)}")

    # [v1.0.0] 원본 Salvo EquipmentPresetData를 연결할 MountProfile입니다.
    salvo_mount_profile = salvo_mount_profiles[0]
    set_property(salvo_mount_profile, "DefaultEquipmentPresetData", source_preset)
    salvo_mount_profiles[0] = salvo_mount_profile
    set_property(salvo_vehicle, "MountProfiles", salvo_mount_profiles)

    # [v1.0.0] Ripple 테스트 SUV에 저장된 단일 MountProfile 배열입니다.
    ripple_mount_profiles = list(get_property(ripple_vehicle, "MountProfiles"))
    if len(ripple_mount_profiles) != 1:
        raise RuntimeError(f"Ripple test SUV MountProfile count must be 1: {len(ripple_mount_profiles)}")

    # [v1.0.0] Ripple EquipmentPresetData를 연결할 MountProfile입니다.
    ripple_mount_profile = ripple_mount_profiles[0]
    set_property(ripple_mount_profile, "DefaultEquipmentPresetData", ripple_preset)
    ripple_mount_profiles[0] = ripple_mount_profile
    set_property(ripple_vehicle, "MountProfiles", ripple_mount_profiles)

    # [v1.0.0] source_weapon 인자는 원본 Salvo 참조를 명시적으로 유지하기 위한 읽기 계약입니다.
    del source_weapon


# [v1.0.0] 저장 후 Salvo·Ripple 직접 참조 체인과 패턴값을 검증합니다.
def validate_saved_test_asset_chains() -> None:
    # [v1.0.0] 저장 후 다시 로드한 원본 Salvo WeaponData입니다.
    source_weapon = require_asset(SOURCE_WEAPON_PATH)

    # [v1.0.0] 저장 후 다시 로드한 Ripple WeaponData입니다.
    ripple_weapon = require_asset(RIPPLE_WEAPON_PATH)

    # [v1.0.0] 저장 후 다시 로드한 원본 RocketLauncher EquipmentPresetData입니다.
    source_preset = require_asset(SOURCE_PRESET_PATH)

    # [v1.0.0] 저장 후 다시 로드한 Ripple EquipmentPresetData입니다.
    ripple_preset = require_asset(RIPPLE_PRESET_PATH)

    # [v1.0.0] 저장 후 다시 로드한 Salvo 테스트 SUV VehicleData입니다.
    salvo_vehicle = require_asset(SALVO_VEHICLE_PATH)

    # [v1.0.0] 저장 후 다시 로드한 Ripple 테스트 SUV VehicleData입니다.
    ripple_vehicle = require_asset(RIPPLE_VEHICLE_PATH)

    # [v1.0.0] 원본 Salvo WeaponData 검증 스냅샷입니다.
    source_weapon_snapshot = capture_weapon_snapshot(source_weapon)

    # [v1.0.0] Ripple WeaponData 검증 스냅샷입니다.
    ripple_weapon_snapshot = capture_weapon_snapshot(ripple_weapon)

    # [v1.0.0] 원본 Salvo EquipmentPresetData 검증 스냅샷입니다.
    source_preset_snapshot = capture_preset_snapshot(source_preset)

    # [v1.0.0] Ripple EquipmentPresetData 검증 스냅샷입니다.
    ripple_preset_snapshot = capture_preset_snapshot(ripple_preset)

    # [v1.0.0] Salvo 테스트 SUV VehicleData 검증 스냅샷입니다.
    salvo_vehicle_snapshot = capture_vehicle_snapshot(salvo_vehicle)

    # [v1.0.0] Ripple 테스트 SUV VehicleData 검증 스냅샷입니다.
    ripple_vehicle_snapshot = capture_vehicle_snapshot(ripple_vehicle)

    REPORT["test_snapshots"] = {
        "source_weapon": source_weapon_snapshot,
        "ripple_weapon": ripple_weapon_snapshot,
        "source_preset": source_preset_snapshot,
        "ripple_preset": ripple_preset_snapshot,
        "salvo_vehicle": salvo_vehicle_snapshot,
        "ripple_vehicle": ripple_vehicle_snapshot,
        }

    if not enum_text_contains(source_weapon_snapshot["fire_pattern"], "Salvo"):
        raise RuntimeError(f"Source WeaponData changed from Salvo: {source_weapon_snapshot}")
    if not enum_text_contains(ripple_weapon_snapshot["fire_pattern"], "Ripple"):
        raise RuntimeError(f"Ripple WeaponData pattern failed: {ripple_weapon_snapshot}")
    if ripple_weapon_snapshot["projectile_count_per_trigger"] != 4:
        raise RuntimeError(f"Ripple projectile count failed: {ripple_weapon_snapshot}")
    require_nearly_equal(ripple_weapon_snapshot["inter_muzzle_delay_seconds"], 0.15, "Ripple interval")
    if ripple_weapon_snapshot["maximum_simultaneous_launch_count"] != 1:
        raise RuntimeError(f"Ripple simultaneous count failed: {ripple_weapon_snapshot}")
    if ripple_weapon_snapshot["sequence_failure_policy"] != source_weapon_snapshot["sequence_failure_policy"]:
        raise RuntimeError("Ripple duplicate no longer preserves SequenceFailurePolicy.")
    if ripple_weapon_snapshot["cooldown_start_policy"] != source_weapon_snapshot["cooldown_start_policy"]:
        raise RuntimeError("Ripple duplicate no longer preserves CooldownStartPolicy.")
    if ripple_weapon_snapshot["release_mode"] != source_weapon_snapshot["release_mode"]:
        raise RuntimeError("Ripple duplicate no longer preserves Direct ReleaseMode.")
    if ripple_weapon_snapshot["default_projectile_data"] != source_weapon_snapshot["default_projectile_data"]:
        raise RuntimeError("Ripple duplicate no longer preserves DA_Rocket_PropTest.")
    if SOURCE_TURRET_PATH not in ripple_preset_snapshot["default_turret_mount_data"]:
        raise RuntimeError(f"Ripple preset TurretMountData connection failed: {ripple_preset_snapshot}")
    if RIPPLE_WEAPON_PATH not in ripple_preset_snapshot["default_weapon_data"]:
        raise RuntimeError(f"Ripple preset WeaponData connection failed: {ripple_preset_snapshot}")
    if SOURCE_PRESET_PATH not in salvo_vehicle_snapshot["mount_equipment_presets"][0]:
        raise RuntimeError(f"Salvo SUV preset connection failed: {salvo_vehicle_snapshot}")
    if RIPPLE_PRESET_PATH not in ripple_vehicle_snapshot["mount_equipment_presets"][0]:
        raise RuntimeError(f"Ripple SUV preset connection failed: {ripple_vehicle_snapshot}")
    if salvo_vehicle_snapshot["default_defense_data"] or ripple_vehicle_snapshot["default_defense_data"]:
        raise RuntimeError("Launcher regression attacker SUVs must preserve DefaultDefenseData=None.")

    REPORT["salvo_chain_valid"] = True
    REPORT["ripple_chain_valid"] = True


# [v1.0.0] 현재 로드된 맵에서 VehicleData 프로퍼티를 가진 차량 Actor 상태를 수집합니다.
def capture_map_vehicle_actors() -> list[dict[str, Any]]:
    # [v1.0.0] 현재 로드된 Level Actor를 읽을 Unreal Editor 서브시스템입니다.
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    # [v1.0.0] VehicleData 프로퍼티를 가진 모든 맵 차량 Actor의 구조화 목록입니다.
    actor_rows: list[dict[str, Any]] = []
    for level_actor in actor_subsystem.get_all_level_actors():
        if level_actor is None:
            continue
        try:
            # [v1.0.0] 현재 차량 Actor에 직접 저장된 VehicleData입니다.
            vehicle_data = get_property(level_actor, "VehicleData")

            # [v1.0.0] VehicleData 기본 장비를 덮어쓸 수 있는 선택적 VehicleFittingData입니다.
            vehicle_fitting_data = get_property(level_actor, "VehicleFittingData")

            # [v1.0.0] 현재 차량 Actor의 플레이어 자동 빙의 설정입니다.
            auto_possess_player = get_property(level_actor, "AutoPossessPlayer")

            # [v1.0.0] RocketLauncher 차량 시각 식별에 사용할 PitchMesh 컴포넌트입니다.
            turret_pitch_mesh_component = get_property(level_actor, "TurretPitchMeshComp")

            # [v1.0.0] 현재 PitchMesh 컴포넌트에 저장된 StaticMesh입니다.
            turret_pitch_static_mesh = get_property(turret_pitch_mesh_component, "StaticMesh")
        except Exception:
            continue

        # [v1.0.0] 맵 저장 전후 위치 보존을 확인할 차량 Actor 위치입니다.
        actor_location = level_actor.get_actor_location()

        # [v1.0.0] 맵 저장 전후 회전 보존을 확인할 차량 Actor 회전입니다.
        actor_rotation = level_actor.get_actor_rotation()
        actor_rows.append(
            {
                "actor_name": level_actor.get_name(),
                "actor_label": level_actor.get_actor_label(),
                "actor_path": level_actor.get_path_name(),
                "vehicle_data": object_path(vehicle_data),
                "vehicle_fitting_data": object_path(vehicle_fitting_data),
                "auto_possess_player": str(auto_possess_player),
                "turret_pitch_mesh": object_path(turret_pitch_static_mesh),
                "location": [float(actor_location.x), float(actor_location.y), float(actor_location.z)],
                "rotation": [float(actor_rotation.pitch), float(actor_rotation.yaw), float(actor_rotation.roll)],
            }
        )
    actor_rows.sort(key=lambda actor_row: actor_row["actor_path"])
    return actor_rows


# [v1.0.0] 기존 CF-FQ-029 TestMap Launcher 검증 슬롯 Actor를 이름과 고정 위치로 반환합니다.
def find_launcher_test_actor() -> Any:
    # [v1.0.0] 현재 로드된 Level Actor를 검색할 Unreal Editor 서브시스템입니다.
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    # [v1.0.0] 기존 Launcher 검증 슬롯의 안정된 Actor 이름과 일치하는 후보 목록입니다.
    matching_actors: list[Any] = [
        level_actor
        for level_actor in actor_subsystem.get_all_level_actors()
        if level_actor is not None and level_actor.get_name() == SOURCE_LAUNCHER_ACTOR_NAME
    ]
    if len(matching_actors) != 1:
        raise RuntimeError(
            f"Expected exactly one {SOURCE_LAUNCHER_ACTOR_NAME} Actor in TestMap, "
            f"found {len(matching_actors)}."
        )

    # [v1.0.0] 이름으로 식별한 기존 CF-FQ-029 Launcher 검증 슬롯 Actor입니다.
    launcher_test_actor = matching_actors[0]

    # [v1.0.0] 잘못된 Actor를 수정하지 않도록 비교할 현재 월드 위치입니다.
    actor_location = launcher_test_actor.get_actor_location()
    require_nearly_equal(actor_location.x, EXPECTED_LAUNCHER_ACTOR_LOCATION[0], "Launcher Actor Location X")
    require_nearly_equal(actor_location.y, EXPECTED_LAUNCHER_ACTOR_LOCATION[1], "Launcher Actor Location Y")
    require_nearly_equal(actor_location.z, EXPECTED_LAUNCHER_ACTOR_LOCATION[2], "Launcher Actor Location Z")
    return launcher_test_actor


# [v1.0.0] VehicleData 경로와 일치하는 저장 후 플레이어 차량 Actor를 정확히 한 대 반환합니다.
def find_vehicle_actor_by_data(vehicle_data_path: str) -> Any:
    # [v1.0.0] 현재 로드된 Level Actor를 검색할 Unreal Editor 서브시스템입니다.
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    # [v1.0.0] 지정 VehicleData 경로와 일치하는 차량 Actor 후보 목록입니다.
    matching_actors: list[Any] = []
    for level_actor in actor_subsystem.get_all_level_actors():
        if level_actor is None:
            continue
        try:
            # [v1.0.0] 현재 차량 Actor의 직접 VehicleData 전체 경로입니다.
            current_vehicle_data_path = object_path(get_property(level_actor, "VehicleData"))
        except Exception:
            continue
        if vehicle_data_path in current_vehicle_data_path:
            matching_actors.append(level_actor)

    if len(matching_actors) != 1:
        raise RuntimeError(
            f"Expected exactly one vehicle Actor using {vehicle_data_path}, found {len(matching_actors)}."
        )
    return matching_actors[0]


# [v1.0.0] 원본 TestMap의 차량 Actor 구성과 RocketLauncher 시각 차량 존재를 검증합니다.
def inspect_source_map() -> None:
    # [v1.0.0] 읽기 전용 원본 TestMap을 로드한 World입니다.
    source_world = unreal.EditorLoadingAndSavingUtils.load_map(SOURCE_MAP_PATH)
    if source_world is None:
        raise RuntimeError(f"Source TestMap load failed: {SOURCE_MAP_PATH}")

    REPORT["source_map_vehicle_actors"] = capture_map_vehicle_actors()

        # [v1.0.0] 기존 CF-FQ-029 문서와 고정 위치가 가리키는 Launcher 검증 슬롯 Actor입니다.
    source_rocket_actor = find_launcher_test_actor()

    # [v1.0.0] Launcher 검증 슬롯의 VehicleData 기본 장비를 덮어쓸 수 있는 FittingData 경로입니다.
    source_fitting_path = object_path(get_property(source_rocket_actor, "VehicleFittingData"))
    if source_fitting_path:
        raise RuntimeError(
            "Serialized RocketLauncher actor has VehicleFittingData that would override test VehicleData: "
            f"{source_fitting_path}"
        )


# [v1.0.0] 테스트 맵 복제본의 RocketLauncher 차량을 지정 VehicleData와 Player0으로 고정합니다.
def configure_test_map(test_map_path: str, test_vehicle_path: str, test_vehicle_asset: Any) -> None:
    ensure_mutable_asset_path(test_map_path)
    require_asset(SOURCE_MAP_PATH)

    if unreal.EditorAssetLibrary.does_asset_exist(test_map_path):
        REPORT["updated_assets"].append(test_map_path)

        # [v1.0.0] 재실행 시 보정·검증할 기존 테스트 맵 경로입니다.
        map_to_load = test_map_path
    elif not APPLY_CHANGES:
        record_change(test_map_path, "DuplicateAsset", SOURCE_MAP_PATH)

        # [v1.0.0] Dry Run에서 Actor 구조를 검증하기 위해 로드할 원본 맵 경로입니다.
        map_to_load = SOURCE_MAP_PATH
    else:
        # [v1.0.0] 원본 TestMap을 테스트 전용 경로에 복제한 World 에셋입니다.
        duplicated_map = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP_PATH, test_map_path)
        if duplicated_map is None:
            raise RuntimeError(f"Map duplication failed: {SOURCE_MAP_PATH} -> {test_map_path}")
        REPORT["created_assets"].append(test_map_path)

        # [v1.0.0] 새로 생성한 테스트 맵을 수정·저장하기 위해 로드할 경로입니다.
        map_to_load = test_map_path

    # [v1.0.0] RocketLauncher 차량 연결을 확인하거나 수정할 현재 World입니다.
    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(map_to_load)
    if loaded_world is None:
        raise RuntimeError(f"Launcher regression map load failed: {map_to_load}")

    # [v1.0.0] 맵별 GameMode가 원본과 같게 유지되는지 확인할 WorldSettings입니다.
    world_settings = loaded_world.get_world_settings()
    if world_settings is None:
        raise RuntimeError("Launcher regression map WorldSettings is unavailable.")

    # [v1.0.0] 맵 변경 전 GameMode 전체 경로입니다.
    game_mode_before = object_path(get_property(world_settings, "DefaultGameMode"))

        # [v1.0.0] 기존 CF-FQ-029 TestMap Launcher 검증 슬롯에 해당하는 차량 Actor입니다.
    rocket_actor = find_launcher_test_actor()

    # [v1.0.0] RocketLauncher 차량을 Player0으로 설정할 enum 값입니다.
    player_zero_value = resolve_enum_value("AutoReceiveInput", ["PLAYER0", "Player0"])

    # [v1.0.0] 다른 배치 차량의 자동 빙의를 해제할 enum 값입니다.
    disabled_value = resolve_enum_value("AutoReceiveInput", ["DISABLED", "Disabled"])

    record_change(test_map_path, f"{rocket_actor.get_name()}.VehicleData", test_vehicle_asset)
    record_change(test_map_path, f"{rocket_actor.get_name()}.VehicleFittingData", None)
    record_change(test_map_path, f"{rocket_actor.get_name()}.AutoPossessPlayer", "Player0")

    if not APPLY_CHANGES:
        return

    # [v1.0.0] 현재 로드된 Level Actor를 순회할 Unreal Editor 서브시스템입니다.
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    for level_actor in actor_subsystem.get_all_level_actors():
        if level_actor is None:
            continue
        try:
            get_property(level_actor, "VehicleData")
            get_property(level_actor, "AutoPossessPlayer")
        except Exception:
            continue
        set_property(level_actor, "AutoPossessPlayer", disabled_value)

    set_property(rocket_actor, "VehicleData", test_vehicle_asset)
    set_property(rocket_actor, "VehicleFittingData", None)
    set_property(rocket_actor, "AutoPossessPlayer", player_zero_value)

    # [v1.0.0] 현재 테스트 맵을 저장할 Unreal Level Editor 서브시스템입니다.
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_subsystem is None:
        raise RuntimeError("LevelEditorSubsystem is unavailable.")
    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Launcher regression map save failed: {test_map_path}")
    REPORT["saved_assets"].append(test_map_path)

    # [v1.0.0] 디스크 저장 사후조건을 확인하기 위해 다시 로드한 테스트 World입니다.
    reloaded_world = unreal.EditorLoadingAndSavingUtils.load_map(test_map_path)
    if reloaded_world is None:
        raise RuntimeError(f"Saved launcher regression map reload failed: {test_map_path}")

    # [v1.0.0] 저장 후 GameMode 보존을 확인할 WorldSettings입니다.
    reloaded_world_settings = reloaded_world.get_world_settings()
    if reloaded_world_settings is None:
        raise RuntimeError("Reloaded launcher regression map WorldSettings is unavailable.")

    # [v1.0.0] 저장 후 지정 VehicleData로 식별한 Player0 차량 Actor입니다.
    reloaded_player_actor = find_vehicle_actor_by_data(test_vehicle_path)

    # [v1.0.0] 저장 후 Player 차량의 VehicleFittingData 전체 경로입니다.
    reloaded_fitting_path = object_path(get_property(reloaded_player_actor, "VehicleFittingData"))

    # [v1.0.0] 저장 후 Player 차량의 자동 빙의 설정입니다.
    reloaded_auto_possess = str(get_property(reloaded_player_actor, "AutoPossessPlayer"))

    # [v1.0.0] 저장 후 맵 GameMode 전체 경로입니다.
    game_mode_after = object_path(get_property(reloaded_world_settings, "DefaultGameMode"))
    if reloaded_fitting_path:
        raise RuntimeError(f"Launcher regression Player VehicleFittingData must be None: {reloaded_fitting_path}")
    if "Player0" not in reloaded_auto_possess and "PLAYER0" not in reloaded_auto_possess:
        raise RuntimeError(f"Launcher regression Player AutoPossess must be Player0: {reloaded_auto_possess}")
    if game_mode_after != game_mode_before:
        raise RuntimeError(f"Launcher regression map GameMode changed: {game_mode_before} -> {game_mode_after}")

    # [v1.0.0] 저장 후 Player 차량의 핵심 연결 상태입니다.
    player_snapshot = {
        "actor_name": reloaded_player_actor.get_name(),
        "actor_path": reloaded_player_actor.get_path_name(),
        "vehicle_data": object_path(get_property(reloaded_player_actor, "VehicleData")),
        "vehicle_fitting_data": reloaded_fitting_path,
        "auto_possess_player": reloaded_auto_possess,
        "game_mode": game_mode_after,
    }

    # [v1.0.0] Salvo 또는 Ripple 맵별 차량 Actor 전체 저장 상태입니다.
    reloaded_actor_rows = capture_map_vehicle_actors()
    if test_map_path == SALVO_MAP_PATH:
        REPORT["salvo_player_actor"] = player_snapshot
        REPORT["salvo_map_vehicle_actors"] = reloaded_actor_rows
        REPORT["salvo_map_connected"] = True
    elif test_map_path == RIPPLE_MAP_PATH:
        REPORT["ripple_player_actor"] = player_snapshot
        REPORT["ripple_map_vehicle_actors"] = reloaded_actor_rows
        REPORT["ripple_map_connected"] = True


# [v1.0.0] Salvo·Ripple 테스트 에셋과 맵을 생성하고 원본 보호 계약을 검증합니다.
def apply_launcher_regression_assets() -> None:
    REPORT["protected_hashes_before"] = capture_hashes(PROTECTED_ASSET_PATHS)

    # [v1.0.0] 원본 Salvo 설정을 유지해야 하는 RocketLauncher WeaponData입니다.
    source_weapon = require_asset(SOURCE_WEAPON_PATH)

    # [v1.0.0] 원본 Salvo WeaponData와 Rocket TurretMountData를 묶는 EquipmentPresetData입니다.
    source_preset = require_asset(SOURCE_PRESET_PATH)

    # [v1.0.0] 네 개 Muzzle와 RocketLauncher 시각 구성을 제공하는 TurretMountData입니다.
    source_turret = require_asset(SOURCE_TURRET_PATH)

    # [v1.0.0] 추진·Trail·Thruster·Impact FX를 제공하는 Rocket ProjectileData입니다.
    source_projectile = require_asset(SOURCE_PROJECTILE_PATH)

    # [v1.0.0] Salvo·Ripple 테스트 SUV 복제 기준 VehicleData입니다.
    source_vehicle = require_asset(SOURCE_VEHICLE_PATH)

    require_asset(SOURCE_PAWN_BLUEPRINT_PATH)
    require_asset(SOURCE_HEAVY_PRESET_PATH)

    validate_source_contracts(
        source_weapon,
        source_preset,
        source_turret,
        source_projectile,
        source_vehicle,
    )
    inspect_source_map()

    # [v1.0.0] 원본 Salvo 설정에서 발사 패턴만 Ripple로 바꿀 테스트 WeaponData입니다.
    ripple_weapon = duplicate_or_load_asset(SOURCE_WEAPON_PATH, RIPPLE_WEAPON_PATH)

    # [v1.0.0] Ripple WeaponData와 원본 Rocket TurretMountData를 묶을 테스트 EquipmentPresetData입니다.
    ripple_preset = duplicate_or_load_asset(SOURCE_PRESET_PATH, RIPPLE_PRESET_PATH)

    # [v1.0.0] 원본 Salvo EquipmentPresetData를 장착할 테스트 SUV VehicleData입니다.
    salvo_vehicle = duplicate_or_load_asset(SOURCE_VEHICLE_PATH, SALVO_VEHICLE_PATH)

    # [v1.0.0] Ripple EquipmentPresetData를 장착할 테스트 SUV VehicleData입니다.
    ripple_vehicle = duplicate_or_load_asset(SOURCE_VEHICLE_PATH, RIPPLE_VEHICLE_PATH)

    configure_test_asset_chains(
        ripple_weapon,
        ripple_preset,
        salvo_vehicle,
        ripple_vehicle,
        source_weapon,
        source_preset,
        source_turret,
    )

    if APPLY_CHANGES:
        save_data_asset(RIPPLE_WEAPON_PATH, ripple_weapon)
        save_data_asset(RIPPLE_PRESET_PATH, ripple_preset)
        save_data_asset(SALVO_VEHICLE_PATH, salvo_vehicle)
        save_data_asset(RIPPLE_VEHICLE_PATH, ripple_vehicle)
        validate_saved_test_asset_chains()

    configure_test_map(SALVO_MAP_PATH, SALVO_VEHICLE_PATH, salvo_vehicle)
    configure_test_map(RIPPLE_MAP_PATH, RIPPLE_VEHICLE_PATH, ripple_vehicle)

    REPORT["protected_hashes_after"] = capture_hashes(PROTECTED_ASSET_PATHS)
    if REPORT["protected_hashes_before"] != REPORT["protected_hashes_after"]:
        raise RuntimeError("One or more protected TestMap, Launcher, Projectile or Vehicle assets changed unexpectedly.")
    REPORT["protected_contract_passed"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_launcher_regression_assets()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][LauncherRegression] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Launcher regression asset application failed. Read report.json for details.")
