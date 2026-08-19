# Copyright (c) CarFight. All Rights Reserved.
# Version: v1.1.0
# Date: 2026-08-14
# Description: 기존 Salvo Launcher와 finite Ammo fixture를 조합해 Ammo+Salvo 전용 테스트 자산 4종을 생성·검증합니다.
# Changelog:
# - v1.1.0: fresh AssetDump에서 Fitting의 구조체 배열 원소 제자리 수정이 직렬화되지 않은 것을 확인해 새 CFVehicleMountSelection을 생성하고 배열 전체를 교체하도록 교정. 목적지 4종이 모두 존재하면 이번 fixture만 안전하게 재검증·repair하는 idempotent 경로를 추가.
# - v1.0.0: 기존 자산 불변, 목적지 사전 충돌 검사, 실패 시 신규 자산 정리, 저장/Readback 검증을 포함한 최초 버전.
# Migration:
# - 기존 TestMap_DRSalvo / TestMap_AmmoRipple 및 Production HUD/Gameplay 자산은 수정하지 않습니다.
# - 새 fixture는 /Game/CarFight/Tests/AmmoIntegration 및 /Game/Maps/TestMap_AmmoSalvo에만 생성합니다.

import json
import os
import traceback
import unreal

SOURCE_WEAPON = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher"
SOURCE_PRESET = "/Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher"
SOURCE_FITTING = "/Game/CarFight/Tests/AmmoIntegration/DA_Fit_RippleFinite"
SOURCE_MAP = "/Game/Maps/TestMap_AmmoRipple"
SOURCE_AMMO = "/Game/CarFight/Tests/AmmoIntegration/DA_Ammo_RocketFinite"

TARGET_WEAPON = "/Game/CarFight/Tests/AmmoIntegration/DA_Wpn_SalvoFinite"
TARGET_PRESET = "/Game/CarFight/Tests/AmmoIntegration/Preset_SalvoFinite"
TARGET_FITTING = "/Game/CarFight/Tests/AmmoIntegration/DA_Fit_SalvoFinite"
TARGET_MAP = "/Game/Maps/TestMap_AmmoSalvo"

RESULT_DIR = os.path.join(unreal.Paths.project_saved_dir(), "CarFight")
RESULT_PATH = os.path.join(RESULT_DIR, "AmmoSalvoFixtureResult.json")


def write_result(payload):
    """실행 결과를 UTF-8 JSON으로 저장합니다."""
    os.makedirs(RESULT_DIR, exist_ok=True)
    with open(RESULT_PATH, "w", encoding="utf-8") as result_file:
        json.dump(payload, result_file, ensure_ascii=False, indent=2)


def require_asset(asset_path):
    """필수 원본 자산이 존재하는지 확인하고 로드합니다."""
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"필수 원본 자산이 없습니다: {asset_path}")
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"필수 원본 자산을 로드하지 못했습니다: {asset_path}")
    return asset


def resolve_fixture_mode():
    """목적지 4종이 모두 없으면 create, 모두 있으면 repair이며 부분 존재는 안전하게 거부합니다."""
    target_paths = (TARGET_WEAPON, TARGET_PRESET, TARGET_FITTING, TARGET_MAP)
    existing_targets = [
        asset_path
        for asset_path in target_paths
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    ]
    if not existing_targets:
        return "create"
    if len(existing_targets) == len(target_paths):
        return "repair"
    raise RuntimeError(
        "목적지 fixture가 부분적으로 존재해 자동 수정하지 않습니다: " + ", ".join(existing_targets)
    )



def duplicate_asset(source_path, target_path, created_assets):
    """원본 자산을 새 경로로 복제하고 생성 목록에 기록합니다."""
    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"자산 복제 실패: {source_path} -> {target_path}")
    created_assets.append(target_path)
    return duplicated_asset


def set_reflected_property(target_object, property_names, value):
    """UE Python 이름과 C++ 원본 이름 후보 중 실제 Reflection Property 하나를 찾아 값을 설정합니다."""
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


def get_reflected_property(target_object, property_names):
    """UE Python 이름과 C++ 원본 이름 후보 중 실제 Reflection Property 하나의 값을 읽습니다."""
    errors = []
    for property_name in property_names:
        try:
            return property_name, target_object.get_editor_property(property_name)
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    raise RuntimeError(
        f"Property 조회 실패: {target_object.get_path_name()} / 후보={property_names} / 오류={' | '.join(errors)}"
    )


def save_asset(asset_path):
    """신규 DataAsset 또는 맵 패키지를 저장합니다."""
    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        raise RuntimeError(f"자산 저장 실패: {asset_path}")


def build_mount_selection(source_selection, equipment_preset):
    """기존 MountProfileId·Enabled를 보존하면서 새 EquipmentPresetData를 가진 독립 구조체를 생성합니다."""
    mount_selection_type = getattr(unreal, "CFVehicleMountSelection", None)
    if mount_selection_type is None:
        raise RuntimeError("Unreal Python에 CFVehicleMountSelection 구조체가 노출되지 않았습니다.")

    _, mount_profile_id = get_reflected_property(source_selection, ("mount_profile_id", "MountProfileId"))
    _, enabled = get_reflected_property(source_selection, ("enabled", "b_enabled", "bEnabled"))

    replacement_selection = mount_selection_type()
    set_reflected_property(replacement_selection, ("mount_profile_id", "MountProfileId"), mount_profile_id)
    set_reflected_property(
        replacement_selection,
        ("equipment_preset_data", "EquipmentPresetData"),
        equipment_preset,
    )
    set_reflected_property(replacement_selection, ("enabled", "b_enabled", "bEnabled"), bool(enabled))
    return replacement_selection



def cleanup_created_assets(created_assets):
    """실패한 실행에서 이번에 새로 만든 자산만 역순 삭제합니다."""
    cleanup_errors = []
    for asset_path in reversed(created_assets):
        try:
            if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                if not unreal.EditorAssetLibrary.delete_asset(asset_path):
                    cleanup_errors.append(f"삭제 실패: {asset_path}")
        except Exception as exc:
            cleanup_errors.append(f"삭제 예외: {asset_path}: {exc}")
    return cleanup_errors


def main():
    """Ammo+Salvo 테스트 fixture 4종을 기존 자산 복제로 생성하고 연결합니다."""
    created_assets = []
    resolved_properties = {}
    changed_map_actor_paths = []

        try:
        fixture_mode = resolve_fixture_mode()

        source_weapon = require_asset(SOURCE_WEAPON)
        source_preset = require_asset(SOURCE_PRESET)
        source_fitting = require_asset(SOURCE_FITTING)
        source_ammo = require_asset(SOURCE_AMMO)
        require_asset(SOURCE_MAP)

                new_weapon = (
            duplicate_asset(SOURCE_WEAPON, TARGET_WEAPON, created_assets)
            if fixture_mode == "create"
            else require_asset(TARGET_WEAPON)
        )
        resolved_properties["weapon_id"] = set_reflected_property(
            new_weapon, ("weapon_id", "WeaponId"), unreal.Name("Proto_RocketLauncher_SalvoFinite")
        )
        resolved_properties["weapon_mass_kg"] = set_reflected_property(
            new_weapon, ("weapon_mass_kg", "WeaponMassKg"), 120.0
        )
        resolved_properties["default_ammo_data"] = set_reflected_property(
            new_weapon, ("default_ammo_data", "DefaultAmmoData"), source_ammo
        )
        resolved_properties["initial_loaded_ammo_count"] = set_reflected_property(
            new_weapon, ("initial_loaded_ammo_count", "InitialLoadedAmmoCount"), 4
        )
        resolved_properties["reload_time_seconds"] = set_reflected_property(
            new_weapon, ("reload_time_seconds", "ReloadTimeSeconds"), 2.0
        )
        resolved_properties["use_infinite_ammo_for_debug"] = set_reflected_property(
            new_weapon,
            ("use_infinite_ammo_for_debug", "b_use_infinite_ammo_for_debug", "bUseInfiniteAmmoForDebug"),
            False,
        )
        save_asset(TARGET_WEAPON)

                new_preset = (
            duplicate_asset(SOURCE_PRESET, TARGET_PRESET, created_assets)
            if fixture_mode == "create"
            else require_asset(TARGET_PRESET)
        )
        resolved_properties["preset_default_weapon_data"] = set_reflected_property(
            new_preset, ("default_weapon_data", "DefaultWeaponData"), new_weapon
        )
        resolved_properties["preset_equipment_id"] = set_reflected_property(
            new_preset, ("equipment_id", "EquipmentId"), unreal.Name("AmmoP008_SalvoPreset")
        )
        save_asset(TARGET_PRESET)

                new_fitting = (
            duplicate_asset(SOURCE_FITTING, TARGET_FITTING, created_assets)
            if fixture_mode == "create"
            else require_asset(TARGET_FITTING)
        )
        resolved_properties["fitting_id"] = set_reflected_property(
            new_fitting, ("fitting_id", "FittingId"), unreal.Name("AmmoP008_Salvo")
        )
        mount_property_name, mount_selections = get_reflected_property(
            new_fitting, ("mount_selections", "MountSelections")
        )
        if len(mount_selections) != 1:
            raise RuntimeError(f"예상한 MountSelection 1개가 아닙니다: {len(mount_selections)}")
                replacement_selection = build_mount_selection(mount_selections[0], new_preset)
        new_fitting.set_editor_property(mount_property_name, [replacement_selection])
        resolved_properties["mount_selection_array"] = mount_property_name
        save_asset(TARGET_FITTING)

        if fixture_mode == "create":
            duplicate_asset(SOURCE_MAP, TARGET_MAP, created_assets)
        else:
            require_asset(TARGET_MAP)
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if level_subsystem is None:
            raise RuntimeError("LevelEditorSubsystem을 가져오지 못했습니다.")
        if not level_subsystem.load_level(TARGET_MAP):
            raise RuntimeError(f"신규 테스트 맵 로드 실패: {TARGET_MAP}")

        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if editor_actor_subsystem is None:
            raise RuntimeError("EditorActorSubsystem을 가져오지 못했습니다.")

        for actor in editor_actor_subsystem.get_all_level_actors():
            try:
                fitting_property_name, current_fitting = get_reflected_property(
                    actor, ("vehicle_fitting_data", "VehicleFittingData")
                )
            except Exception:
                continue
                        if current_fitting == source_fitting or current_fitting == new_fitting:
                actor.set_editor_property(fitting_property_name, new_fitting)
                changed_map_actor_paths.append(actor.get_path_name())

        if len(changed_map_actor_paths) != 1:
            raise RuntimeError(
                f"TestMap_AmmoRipple 복제본에서 DA_Fit_RippleFinite를 참조하는 차량이 정확히 1대가 아닙니다: {changed_map_actor_paths}"
            )

        saved_level = False
        try:
            saved_level = bool(level_subsystem.save_current_level())
        except Exception:
            saved_level = False
        if not saved_level:
            save_asset(TARGET_MAP)

        _, readback_ammo = get_reflected_property(new_weapon, ("default_ammo_data", "DefaultAmmoData"))
        _, readback_loaded = get_reflected_property(new_weapon, ("initial_loaded_ammo_count", "InitialLoadedAmmoCount"))
        _, readback_infinite = get_reflected_property(
            new_weapon,
            ("use_infinite_ammo_for_debug", "b_use_infinite_ammo_for_debug", "bUseInfiniteAmmoForDebug"),
        )
                _, readback_preset_weapon = get_reflected_property(new_preset, ("default_weapon_data", "DefaultWeaponData"))
        _, readback_mount_selections = get_reflected_property(new_fitting, ("mount_selections", "MountSelections"))
        if len(readback_mount_selections) != 1:
            raise RuntimeError("신규 Fitting MountSelection Readback 개수가 1이 아닙니다.")
        _, readback_mount_preset = get_reflected_property(
            readback_mount_selections[0], ("equipment_preset_data", "EquipmentPresetData")
        )

        if readback_ammo != source_ammo or int(readback_loaded) != 4 or bool(readback_infinite):
            raise RuntimeError("신규 Salvo WeaponData finite Ammo Readback이 예상과 다릅니다.")
                if readback_preset_weapon != new_weapon:
            raise RuntimeError("신규 EquipmentPreset의 WeaponData Readback이 예상과 다릅니다.")
        if readback_mount_preset != new_preset:
            raise RuntimeError("신규 Fitting의 EquipmentPresetData Readback이 예상과 다릅니다.")

        write_result(
            {
                                "status": "success",
                "fixture_mode": fixture_mode,
                "created_assets": created_assets,
                "changed_map_actor_paths": changed_map_actor_paths,
                "resolved_properties": resolved_properties,
                "source_contract": {
                    "salvo_weapon": SOURCE_WEAPON,
                    "finite_ammo": SOURCE_AMMO,
                    "finite_fitting": SOURCE_FITTING,
                    "source_map": SOURCE_MAP,
                },
                "target_contract": {
                    "weapon": TARGET_WEAPON,
                    "preset": TARGET_PRESET,
                    "fitting": TARGET_FITTING,
                    "map": TARGET_MAP,
                    "initial_loaded_ammo_count": 4,
                    "initial_sortie_ammo_count": 7,
                    "expected_initial_reserve_ammo_count": 3,
                },
            }
        )
        unreal.log("[CarFight] AmmoSalvo fixture 생성 완료")

    except Exception as exc:
        cleanup_errors = []
        try:
            if TARGET_MAP in created_assets:
                level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
                if level_subsystem is not None:
                    level_subsystem.load_level(SOURCE_MAP)
        except Exception:
            pass
        cleanup_errors = cleanup_created_assets(created_assets)
        write_result(
            {
                "status": "failed",
                "error": str(exc),
                "traceback": traceback.format_exc(),
                "created_assets_before_cleanup": created_assets,
                "cleanup_errors": cleanup_errors,
            }
        )
        unreal.log_error(f"[CarFight] AmmoSalvo fixture 생성 실패: {exc}")
        raise


main()
