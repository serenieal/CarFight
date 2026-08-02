# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-07-29
# Description: LM-P0-05 RocketLauncher 전용 DataAsset 생성·연결 도구
# Scope: 화이트리스트된 RocketLauncher 자산과 DA_TestSUV MountProfile만 생성·수정합니다.
# Changelog:
# - v1.0.0: Dry Run, DA_RocketBody 수정, Launcher Weapon/Preset 생성, DA_TestSUV 연결과 JSON 보고서 추가.
# Migration:
# - DA_TestSedan과 HeavyCannon 원본은 변경하지 않습니다.

from __future__ import annotations

import json
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


TOOL_VERSION = "1.0.0"
APPLY_CHANGES = os.environ.get("CARFIGHT_LAUNCHER_APPLY", "0") == "1"
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()
REPORT_DIR = PROJECT_DIR / "Saved" / "LauncherAssetApply"
REPORT_PATH = REPORT_DIR / "report.json"

ROCKET_BODY_PATH = "/Game/CarFight/Weapons/Data/TurretMounts/DA_RocketBody"
SOURCE_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon"
ROCKET_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher"
SOURCE_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon"
ROCKET_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher"
TEST_SUV_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"
ROCKET_PROJECTILE_PATH = "/Game/CarFight/Weapons/Data/ProjectileDefs/DA_Rocket_PropTest"
ROCKET_YAW_MESH_PATH = "/Game/CarFight/Weapons/Turrets/RocketLauncher/Meshes/RocketLauncherYaw"
ROCKET_PITCH_MESH_PATH = "/Game/CarFight/Weapons/Turrets/RocketLauncher/Meshes/RocketLauncherPitch"

MUTABLE_ASSET_PATHS = {
    ROCKET_BODY_PATH,
    ROCKET_WEAPON_PATH,
    ROCKET_PRESET_PATH,
    TEST_SUV_PATH,
}

REPORT: dict[str, Any] = {
    "schema_version": "launcher_asset_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "validated_assets": [],
    "changes": [],
    "errors": [],
}


def log(message: str) -> None:
    unreal.log(f"[CarFight][LauncherAssets] {message}")


def pascal_to_snake(name: str) -> str:
    step1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", step1).lower()


def candidate_property_names(name: str) -> list[str]:
    names = [name, pascal_to_snake(name)]
    return list(dict.fromkeys(names))


def get_property(target: Any, property_name: str) -> Any:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            return target.get_editor_property(candidate)
        except Exception as error:  # Unreal raises generic exceptions for reflection errors.
            last_error = error
    raise RuntimeError(f"Property read failed: {type(target).__name__}.{property_name}: {last_error}")


def set_property(target: Any, property_name: str, value: Any) -> None:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            target.set_editor_property(candidate, value)
            return
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property write failed: {type(target).__name__}.{property_name}: {last_error}")


def require_asset(asset_path: str) -> Any:
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    REPORT["validated_assets"].append(asset_path)
    return asset


def duplicate_or_load(source_path: str, target_path: str) -> Any:
    if target_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Target path is not in the mutation whitelist: {target_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        existing_asset = require_asset(target_path)
        REPORT["updated_assets"].append(target_path)
        return existing_asset

    if not APPLY_CHANGES:
        source_asset = require_asset(source_path)
        log(f"Dry Run duplicate planned: {source_path} -> {target_path}")
        return source_asset

    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"Asset duplication failed: {source_path} -> {target_path}")
    REPORT["created_assets"].append(target_path)
    return duplicated_asset


def enum_value(enum_type_name: str, member_name: str) -> Any:
    enum_type = getattr(unreal, enum_type_name, None)
    if enum_type is None:
        raise RuntimeError(f"Unreal enum type was not found: {enum_type_name}")

    candidates = [
        member_name,
        member_name.upper(),
        pascal_to_snake(member_name).upper(),
    ]
    for candidate in candidates:
        if hasattr(enum_type, candidate):
            return getattr(enum_type, candidate)

    available = [name for name in dir(enum_type) if name.isupper()]
    raise RuntimeError(
        f"Enum member was not found: {enum_type_name}.{member_name}; available={available}"
    )


def record_change(asset_path: str, property_name: str, value: Any) -> None:
    REPORT["changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": str(value),
        }
    )


def apply_property(asset_path: str, asset: Any, property_name: str, value: Any) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in the mutation whitelist: {asset_path}")
    record_change(asset_path, property_name, value)
    if APPLY_CHANGES:
        set_property(asset, property_name, value)


def configure_pattern(
    asset_path: str,
    weapon_asset: Any,
    pattern_name: str,
    projectile_count: int,
    interval_seconds: float,
    simultaneous_count: int,
    failure_policy_name: str,
    cooldown_policy_name: str,
) -> None:
    pattern_config = get_property(weapon_asset, "LauncherFirePatternConfig")
    fire_pattern = enum_value("CFLauncherFirePattern", pattern_name)
    failure_policy = enum_value("CFLauncherSequenceFailurePolicy", failure_policy_name)
    cooldown_policy = enum_value("CFLauncherCooldownStartPolicy", cooldown_policy_name)

    record_change(asset_path, "LauncherFirePatternConfig.FirePattern", fire_pattern)
    record_change(asset_path, "LauncherFirePatternConfig.ProjectileCountPerTrigger", projectile_count)
    record_change(asset_path, "LauncherFirePatternConfig.InterMuzzleDelaySeconds", interval_seconds)
    record_change(asset_path, "LauncherFirePatternConfig.MaximumSimultaneousLaunchCount", simultaneous_count)
    record_change(asset_path, "LauncherFirePatternConfig.SequenceFailurePolicy", failure_policy)
    record_change(asset_path, "LauncherFirePatternConfig.CooldownStartPolicy", cooldown_policy)

    if APPLY_CHANGES:
        set_property(pattern_config, "FirePattern", fire_pattern)
        set_property(pattern_config, "ProjectileCountPerTrigger", projectile_count)
        set_property(pattern_config, "InterMuzzleDelaySeconds", interval_seconds)
        set_property(pattern_config, "MaximumSimultaneousLaunchCount", simultaneous_count)
        set_property(pattern_config, "SequenceFailurePolicy", failure_policy)
        set_property(pattern_config, "CooldownStartPolicy", cooldown_policy)
        set_property(weapon_asset, "LauncherFirePatternConfig", pattern_config)


def configure_release(
    asset_path: str,
    weapon_asset: Any,
    release_mode_name: str,
    local_direction: unreal.Vector,
    ejection_speed: float,
    carrier_velocity_ratio: float,
    clearance_distance_cm: float,
) -> None:
    release_config = get_property(weapon_asset, "LauncherReleaseConfig")
    release_mode = enum_value("CFProjectileReleaseMode", release_mode_name)

    record_change(asset_path, "LauncherReleaseConfig.ReleaseMode", release_mode)
    record_change(asset_path, "LauncherReleaseConfig.LocalEjectionDirection", local_direction)
    record_change(asset_path, "LauncherReleaseConfig.EjectionSpeed", ejection_speed)
    record_change(asset_path, "LauncherReleaseConfig.CarrierVelocityRatio", carrier_velocity_ratio)
    record_change(asset_path, "LauncherReleaseConfig.LauncherClearanceTraceDistanceCm", clearance_distance_cm)

    if APPLY_CHANGES:
        set_property(release_config, "ReleaseMode", release_mode)
        set_property(release_config, "LocalEjectionDirection", local_direction)
        set_property(release_config, "EjectionSpeed", ejection_speed)
        set_property(release_config, "CarrierVelocityRatio", carrier_velocity_ratio)
        set_property(release_config, "LauncherClearanceTraceDistanceCm", clearance_distance_cm)
        set_property(weapon_asset, "LauncherReleaseConfig", release_config)


def save_asset(asset_path: str, asset: Any) -> None:
    if not APPLY_CHANGES:
        return
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Save path is not in the mutation whitelist: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


def configure_launcher_assets() -> None:
    rocket_body = require_asset(ROCKET_BODY_PATH)
    source_weapon = require_asset(SOURCE_WEAPON_PATH)
    source_preset = require_asset(SOURCE_PRESET_PATH)
    rocket_projectile = require_asset(ROCKET_PROJECTILE_PATH)
    rocket_yaw_mesh = require_asset(ROCKET_YAW_MESH_PATH)
    rocket_pitch_mesh = require_asset(ROCKET_PITCH_MESH_PATH)
    test_suv = require_asset(TEST_SUV_PATH)

    rocket_weapon = duplicate_or_load(SOURCE_WEAPON_PATH, ROCKET_WEAPON_PATH)
    rocket_preset = duplicate_or_load(SOURCE_PRESET_PATH, ROCKET_PRESET_PATH)

    # DA_RocketBody: Heavy Cannon 복사 상태를 실제 RocketLauncher 메시와 4개 Muzzle로 교체합니다.
    apply_property(ROCKET_BODY_PATH, rocket_body, "TurretMountId", "Proto_RocketLauncherMount")
    apply_property(ROCKET_BODY_PATH, rocket_body, "TurretYawMesh", rocket_yaw_mesh)
    apply_property(ROCKET_BODY_PATH, rocket_body, "TurretPitchMesh", rocket_pitch_mesh)
    apply_property(ROCKET_BODY_PATH, rocket_body, "PitchPivotSocketName", "PitchPivot")
    apply_property(
        ROCKET_BODY_PATH,
        rocket_body,
        "MuzzleSocketNames",
        ["Muzzle_1", "Muzzle_4", "Muzzle_2", "Muzzle_3"],
    )
    apply_property(ROCKET_BODY_PATH, rocket_body, "bRequireAllMuzzles", True)
    apply_property(ROCKET_BODY_PATH, rocket_body, "MuzzleSocketName", "Muzzle_1")

    # Launcher 전용 WeaponData: 기존 Cannon DataAsset을 변경하지 않고 복제본만 설정합니다.
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "WeaponId", "Proto_RocketLauncher")
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "FireMode", enum_value("CFWeaponFireMode", "Projectile"))
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "FireRatePerMinute", 20.0)
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "MaxRange", 15000.0)
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "MagazineSize", 4)
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "AmmoTypeId", "ProtoRocket")
    apply_property(ROCKET_WEAPON_PATH, rocket_weapon, "DefaultProjectileData", rocket_projectile)
    configure_pattern(
        ROCKET_WEAPON_PATH,
        rocket_weapon,
        "Ripple",
        4,
        0.15,
        1,
        "ContinueRemaining",
        "SequenceCompleted",
    )
    configure_release(
        ROCKET_WEAPON_PATH,
        rocket_weapon,
        "Direct",
        unreal.Vector(1.0, 0.0, 1.0),
        0.0,
        0.0,
        150.0,
    )

    # Launcher 전용 EquipmentPresetData를 분리합니다.
    apply_property(ROCKET_PRESET_PATH, rocket_preset, "EquipmentId", "Proto_RocketLauncherKit")
    apply_property(ROCKET_PRESET_PATH, rocket_preset, "DisplayName", "Prototype Rocket Launcher Kit")
    apply_property(ROCKET_PRESET_PATH, rocket_preset, "DefaultTurretMountData", rocket_body)
    apply_property(ROCKET_PRESET_PATH, rocket_preset, "DefaultWeaponData", rocket_weapon)

    # Sedan은 HeavyCannon을 유지하고 SUV의 첫 Top_01 프로파일만 Launcher 프리셋으로 전환합니다.
    mount_profiles = get_property(test_suv, "MountProfiles")
    if len(mount_profiles) < 1:
        raise RuntimeError("DA_TestSUV MountProfiles is empty; launcher preset cannot be assigned.")
    record_change(TEST_SUV_PATH, "MountProfiles[0].DefaultEquipmentPresetData", rocket_preset)
    if APPLY_CHANGES:
        first_mount_profile = mount_profiles[0]
        set_property(first_mount_profile, "DefaultEquipmentPresetData", rocket_preset)
        mount_profiles[0] = first_mount_profile
        set_property(test_suv, "MountProfiles", mount_profiles)

    save_asset(ROCKET_BODY_PATH, rocket_body)
    save_asset(ROCKET_WEAPON_PATH, rocket_weapon)
    save_asset(ROCKET_PRESET_PATH, rocket_preset)
    save_asset(TEST_SUV_PATH, test_suv)


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    configure_launcher_assets()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][LauncherAssets] {error}")
finally:
    REPORT_PATH.write_text(json.dumps(REPORT, ensure_ascii=False, indent=2), encoding="utf-8")
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Launcher asset application failed. Read report.json for details.")
