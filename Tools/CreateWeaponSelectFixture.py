# Copyright (c) CarFight. All Rights Reserved.
# Version: v1.0.1
# Date: 2026-08-19
# Description: CF-FQ-032 UI-P0-06 Weapon Select + truthful Rail USER Visual 전용 2무기 테스트 fixture 3종을 생성·검증합니다.
# Changelog:
# - v1.0.1: Snapshot Invalid 시 ValidationIssues의 Severity/IssueCode/Message/MountProfileId를 손실 없이 오류 문자열에 포함해 fixture 계약을 약화하지 않고 실제 실패 원인을 관측합니다.
# - v1.0.0: Defense Test SUV를 독립 VehicleData로 복제해 Top_01/Top_02 두 실제 하드포인트에 기존 HeavyCannon/RocketLauncher Preset을 배치하고, 독립 Fitting + 복제 TestMap을 구성합니다.
# Migration:
# - 기존 DA_VehicleDefense_TestSUV, DA_Fit_RippleFinite, TestMap_AmmoRipple 및 Production HUD 자산은 수정하지 않습니다.
# - 신규 fixture는 /Game/CarFight/Tests/UI/WeaponSelect의 VehicleData/Fitting 2개와 /Game/Maps/TestMap_WeaponSelect 1개만 소유합니다.
# - 실패하면 이번 실행에서 새로 생성한 target asset만 역순 정리합니다. 이미 완성된 target 3종이 모두 있으면 repair/readback 경로로만 재검증합니다.

import copy
import json
import os
import traceback
import unreal

SOURCE_VEHICLE = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV"
SOURCE_FITTING = "/Game/CarFight/Tests/AmmoIntegration/DA_Fit_RippleFinite"
SOURCE_MAP = "/Game/Maps/TestMap_AmmoRipple"
HEAVY_PRESET = "/Game/CarFight/Tests/AmmoIntegration/Preset_HeavyFinite"
ROCKET_PRESET = "/Game/CarFight/Tests/AmmoIntegration/Preset_RippleFinite"

TARGET_FOLDER = "/Game/CarFight/Tests/UI/WeaponSelect"
TARGET_VEHICLE = TARGET_FOLDER + "/DA_Veh_WeaponSelect"
TARGET_FITTING = TARGET_FOLDER + "/DA_Fit_WeaponSelect"
TARGET_MAP = "/Game/Maps/TestMap_WeaponSelect"

PRIMARY_PROFILE_ID = unreal.Name("UIWeapon_Primary")
SECONDARY_PROFILE_ID = unreal.Name("UIWeapon_Secondary")
PRIMARY_SLOT_ID = unreal.Name("Top_01")
SECONDARY_SLOT_ID = unreal.Name("Top_02")

RESULT_DIR = os.path.join(unreal.Paths.project_saved_dir(), "CarFight")
RESULT_PATH = os.path.join(RESULT_DIR, "WeaponSelectFixtureResult.json")


def write_result(payload):
    """실행 결과를 UTF-8 JSON으로 저장합니다."""
    os.makedirs(RESULT_DIR, exist_ok=True)
    with open(RESULT_PATH, "w", encoding="utf-8") as result_file:
        json.dump(payload, result_file, ensure_ascii=False, indent=2)


def require_asset(asset_path):
    """필수 원본 또는 기존 target 자산을 안전하게 로드합니다."""
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"필수 자산이 없습니다: {asset_path}")
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"자산을 로드하지 못했습니다: {asset_path}")
    return asset


def resolve_fixture_mode():
    """target 3종이 모두 없으면 create, 모두 있으면 repair이며 부분 존재는 fail-closed합니다."""
    target_paths = (TARGET_VEHICLE, TARGET_FITTING, TARGET_MAP)
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
        "WeaponSelect fixture target이 부분적으로 존재해 자동 수정하지 않습니다: "
        + ", ".join(existing_targets)
    )


def duplicate_asset(source_path, target_path, created_assets):
    """원본 자산을 target 경로로 복제하고 이번 실행 생성 목록에 기록합니다."""
    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"자산 복제 실패: {source_path} -> {target_path}")
    created_assets.append(target_path)
    return duplicated_asset


def set_reflected_property(target_object, property_names, value):
    """UE Python/C++ reflection 이름 후보 중 실제 Property 하나를 찾아 설정합니다."""
    errors = []
    for property_name in property_names:
        try:
            target_object.set_editor_property(property_name, value)
            return property_name
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    target_name = target_object.get_path_name() if hasattr(target_object, "get_path_name") else str(target_object)
    raise RuntimeError(
        f"Property 설정 실패: {target_name} / 후보={property_names} / 오류={' | '.join(errors)}"
    )


def get_reflected_property(target_object, property_names):
    """UE Python/C++ reflection 이름 후보 중 실제 Property 하나를 읽습니다."""
    errors = []
    for property_name in property_names:
        try:
            return property_name, target_object.get_editor_property(property_name)
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    target_name = target_object.get_path_name() if hasattr(target_object, "get_path_name") else str(target_object)
    raise RuntimeError(
        f"Property 조회 실패: {target_name} / 후보={property_names} / 오류={' | '.join(errors)}"
    )


def save_asset(asset_path):
    """지정 target DataAsset package만 명시 저장합니다."""
    if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
        raise RuntimeError(f"자산 저장 실패: {asset_path}")


def copy_mount_profile(source_profile):
    """기존 검증된 MountProfile struct를 값 복사해 mount geometry/size/turret 계약을 보존합니다."""
    try:
        copied_profile = copy.copy(source_profile)
    except Exception as exc:
        raise RuntimeError(f"CFVehicleMountProfile 값 복사 실패. 임의 필드 재작성은 하지 않습니다: {exc}")
    if copied_profile is None:
        raise RuntimeError("CFVehicleMountProfile 값 복사 결과가 None입니다.")
    return copied_profile


def build_mount_selection(mount_profile_id, equipment_preset):
    """실제 test-only MountProfileId와 기존 Player-facing Preset을 가진 독립 MountSelection을 생성합니다."""
    mount_selection_type = getattr(unreal, "CFVehicleMountSelection", None)
    if mount_selection_type is None:
        raise RuntimeError("Unreal Python에 CFVehicleMountSelection 구조체가 노출되지 않았습니다.")
    mount_selection = mount_selection_type()
    set_reflected_property(mount_selection, ("mount_profile_id", "MountProfileId"), mount_profile_id)
    set_reflected_property(
        mount_selection,
        ("equipment_preset_data", "EquipmentPresetData"),
        equipment_preset,
    )
    set_reflected_property(mount_selection, ("enabled", "b_enabled", "bEnabled"), True)
    return mount_selection


def verify_hardpoint_slot(vehicle_data, expected_slot_id):
    """복제 원본에 USER fixture가 사용할 실제 hardpoint slot이 존재하는지 확인합니다."""
    _, hardpoint_slots = get_reflected_property(vehicle_data, ("hardpoint_slots", "HardpointSlots"))
    for hardpoint_slot in hardpoint_slots:
        _, location_slot_id = get_reflected_property(
            hardpoint_slot,
            ("location_slot_id", "LocationSlotId"),
        )
        if str(location_slot_id) == str(expected_slot_id):
            return True
    return False


def validate_snapshot(fitting_data, heavy_preset, rocket_preset):
    """새 Fitting Snapshot이 두 actual preset을 fixed order로 resolve하고 gross mass 안에 있는지 검증합니다."""
    snapshot = fitting_data.build_fitting_snapshot()
    _, validation_state = get_reflected_property(snapshot, ("validation_state", "ValidationState"))
    validation_text = str(validation_state).lower()

    # [v1.0.1] Snapshot Invalid 원인을 임의 추정하지 않고 구조화된 ValidationIssues 그대로 보존할 목록입니다.
    _, validation_issues = get_reflected_property(snapshot, ("validation_issues", "ValidationIssues"))
    issue_rows = []
    for validation_issue in validation_issues:
        _, severity = get_reflected_property(validation_issue, ("severity", "Severity"))
        _, issue_code = get_reflected_property(validation_issue, ("issue_code", "IssueCode"))
        _, message = get_reflected_property(validation_issue, ("message", "Message"))
        _, mount_profile_id = get_reflected_property(validation_issue, ("mount_profile_id", "MountProfileId"))
        issue_rows.append(
            {
                "severity": str(severity),
                "issue_code": str(issue_code),
                "message": str(message),
                "mount_profile_id": str(mount_profile_id),
            }
        )

    if "invalid" in validation_text or "not_evaluated" in validation_text or "notevaluated" in validation_text:
        raise RuntimeError(
            "WeaponSelect Fitting Snapshot이 유효하지 않습니다: "
            + str(validation_state)
            + " / issues="
            + json.dumps(issue_rows, ensure_ascii=False)
        )

    _, resolved_mounts = get_reflected_property(snapshot, ("resolved_mounts", "ResolvedMounts"))
    if len(resolved_mounts) != 2:
        raise RuntimeError(f"WeaponSelect ResolvedMounts가 2개가 아닙니다: {len(resolved_mounts)}")

    expected_presets = (heavy_preset, rocket_preset)
    expected_profile_ids = (PRIMARY_PROFILE_ID, SECONDARY_PROFILE_ID)
    resolved_rows = []
    for mount_index, resolved_mount in enumerate(resolved_mounts):
        _, mount_profile_id = get_reflected_property(
            resolved_mount,
            ("mount_profile_id", "MountProfileId"),
        )
        _, equipment_preset = get_reflected_property(
            resolved_mount,
            ("equipment_preset_data", "EquipmentPresetData"),
        )
        if str(mount_profile_id) != str(expected_profile_ids[mount_index]):
            raise RuntimeError(
                f"ResolvedMount[{mount_index}] ProfileId 불일치: {mount_profile_id}"
            )
        if equipment_preset != expected_presets[mount_index]:
            raise RuntimeError(
                f"ResolvedMount[{mount_index}] EquipmentPreset 불일치: {equipment_preset}"
            )
        resolved_rows.append(
            {
                "mount_profile_id": str(mount_profile_id),
                "equipment_preset": equipment_preset.get_path_name(),
            }
        )

    _, total_mass = get_reflected_property(snapshot, ("total_vehicle_mass_kg", "TotalVehicleMassKg"))
    _, gross_mass = get_reflected_property(snapshot, ("maximum_gross_mass_kg", "MaximumGrossMassKg"))
    if float(total_mass) > float(gross_mass) + 0.01:
        raise RuntimeError(
            f"WeaponSelect fixture 총중량이 Gross Mass를 초과합니다: total={total_mass}, gross={gross_mass}"
        )
        return {
        "validation_state": str(validation_state),
        "validation_issues": issue_rows,
        "resolved_mounts": resolved_rows,
        "total_mass_kg": float(total_mass),
        "maximum_gross_mass_kg": float(gross_mass),
    }


def cleanup_created_assets(created_assets):
    """실패한 create 실행에서 이번에 새로 만든 target asset만 역순 삭제합니다."""
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
    """독립 2무기 VehicleData/Fitting/Map을 생성하거나 기존 완성 target 3종을 안전하게 repair·검증합니다."""
    created_assets = []
    changed_map_actor_paths = []
    resolved_properties = {}
    fixture_mode = "unknown"
    try:
        fixture_mode = resolve_fixture_mode()
        source_vehicle = require_asset(SOURCE_VEHICLE)
        source_fitting = require_asset(SOURCE_FITTING)
        require_asset(SOURCE_MAP)
        heavy_preset = require_asset(HEAVY_PRESET)
        rocket_preset = require_asset(ROCKET_PRESET)

        if not verify_hardpoint_slot(source_vehicle, PRIMARY_SLOT_ID):
            raise RuntimeError(f"원본 VehicleData에 {PRIMARY_SLOT_ID} hardpoint가 없습니다.")
        if not verify_hardpoint_slot(source_vehicle, SECONDARY_SLOT_ID):
            raise RuntimeError(f"원본 VehicleData에 {SECONDARY_SLOT_ID} hardpoint가 없습니다.")

        if fixture_mode == "create":
            if not unreal.EditorAssetLibrary.does_directory_exist(TARGET_FOLDER):
                if not unreal.EditorAssetLibrary.make_directory(TARGET_FOLDER):
                    raise RuntimeError(f"Fixture 폴더 생성 실패: {TARGET_FOLDER}")

            target_vehicle = duplicate_asset(SOURCE_VEHICLE, TARGET_VEHICLE, created_assets)
        else:
            target_vehicle = require_asset(TARGET_VEHICLE)

        mount_profiles_property, source_mount_profiles = get_reflected_property(
            source_vehicle,
            ("mount_profiles", "MountProfiles"),
        )
        if len(source_mount_profiles) != 1:
            raise RuntimeError(
                f"원본 Defense SUV MountProfile count가 1이 아닙니다: {len(source_mount_profiles)}"
            )

        primary_profile = copy_mount_profile(source_mount_profiles[0])
        secondary_profile = copy_mount_profile(source_mount_profiles[0])
        set_reflected_property(primary_profile, ("mount_profile_id", "MountProfileId"), PRIMARY_PROFILE_ID)
        set_reflected_property(primary_profile, ("location_slot_ref", "LocationSlotRef"), PRIMARY_SLOT_ID)
        set_reflected_property(
            primary_profile,
            ("default_equipment_preset_data", "DefaultEquipmentPresetData"),
            heavy_preset,
        )
        set_reflected_property(secondary_profile, ("mount_profile_id", "MountProfileId"), SECONDARY_PROFILE_ID)
        set_reflected_property(secondary_profile, ("location_slot_ref", "LocationSlotRef"), SECONDARY_SLOT_ID)
        set_reflected_property(
            secondary_profile,
            ("default_equipment_preset_data", "DefaultEquipmentPresetData"),
            rocket_preset,
        )
        target_vehicle.set_editor_property(mount_profiles_property, [primary_profile, secondary_profile])
        resolved_properties["vehicle_mount_profiles"] = mount_profiles_property
        save_asset(TARGET_VEHICLE)

        if fixture_mode == "create":
            target_fitting = duplicate_asset(SOURCE_FITTING, TARGET_FITTING, created_assets)
        else:
            target_fitting = require_asset(TARGET_FITTING)

        resolved_properties["fitting_id"] = set_reflected_property(
            target_fitting,
            ("fitting_id", "FittingId"),
            unreal.Name("UI_WeaponSelect"),
        )
        resolved_properties["fitting_vehicle_data"] = set_reflected_property(
            target_fitting,
            ("vehicle_data", "VehicleData"),
            target_vehicle,
        )
        mount_selections_property, _ = get_reflected_property(
            target_fitting,
            ("mount_selections", "MountSelections"),
        )
        target_fitting.set_editor_property(
            mount_selections_property,
            [
                build_mount_selection(PRIMARY_PROFILE_ID, heavy_preset),
                build_mount_selection(SECONDARY_PROFILE_ID, rocket_preset),
            ],
        )
        resolved_properties["fitting_mount_selections"] = mount_selections_property
        ammo_loads_property, _ = get_reflected_property(
            target_fitting,
            ("initial_sortie_ammo_loads", "InitialSortieAmmoLoads"),
        )
        target_fitting.set_editor_property(ammo_loads_property, [])
        resolved_properties["fitting_ammo_loads"] = ammo_loads_property
        save_asset(TARGET_FITTING)

        snapshot_readback = validate_snapshot(target_fitting, heavy_preset, rocket_preset)

        if fixture_mode == "create":
            duplicate_asset(SOURCE_MAP, TARGET_MAP, created_assets)
        else:
            require_asset(TARGET_MAP)

        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if level_subsystem is None:
            raise RuntimeError("LevelEditorSubsystem을 가져오지 못했습니다.")
        if not level_subsystem.load_level(TARGET_MAP):
            raise RuntimeError(f"WeaponSelect 테스트 맵 로드 실패: {TARGET_MAP}")

        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if editor_actor_subsystem is None:
            raise RuntimeError("EditorActorSubsystem을 가져오지 못했습니다.")

        for actor in editor_actor_subsystem.get_all_level_actors():
            try:
                fitting_property_name, current_fitting = get_reflected_property(
                    actor,
                    ("vehicle_fitting_data", "VehicleFittingData"),
                )
                vehicle_property_name, current_vehicle = get_reflected_property(
                    actor,
                    ("vehicle_data", "VehicleData"),
                )
            except Exception:
                continue

            if current_fitting == source_fitting or current_fitting == target_fitting:
                actor.set_editor_property(vehicle_property_name, target_vehicle)
                actor.set_editor_property(fitting_property_name, target_fitting)
                changed_map_actor_paths.append(actor.get_path_name())

        if len(changed_map_actor_paths) != 1:
            raise RuntimeError(
                "WeaponSelect 복제 맵에서 source/target Fitting을 참조하는 차량이 정확히 1대가 아닙니다: "
                + str(changed_map_actor_paths)
            )

        saved_level = False
        try:
            saved_level = bool(level_subsystem.save_current_level())
        except Exception:
            saved_level = False
        if not saved_level:
            save_asset(TARGET_MAP)

        _, vehicle_mount_profiles = get_reflected_property(
            target_vehicle,
            ("mount_profiles", "MountProfiles"),
        )
        _, fitting_mount_selections = get_reflected_property(
            target_fitting,
            ("mount_selections", "MountSelections"),
        )
        if len(vehicle_mount_profiles) != 2 or len(fitting_mount_selections) != 2:
            raise RuntimeError(
                f"WeaponSelect fixture readback count 불일치: profiles={len(vehicle_mount_profiles)}, selections={len(fitting_mount_selections)}"
            )

        write_result(
            {
                "status": "success",
                "fixture_mode": fixture_mode,
                "created_assets": created_assets,
                "changed_map_actor_paths": changed_map_actor_paths,
                "resolved_properties": resolved_properties,
                "source_contract": {
                    "vehicle": SOURCE_VEHICLE,
                    "fitting": SOURCE_FITTING,
                    "map": SOURCE_MAP,
                    "heavy_preset": HEAVY_PRESET,
                    "rocket_preset": ROCKET_PRESET,
                },
                "target_contract": {
                    "vehicle": TARGET_VEHICLE,
                    "fitting": TARGET_FITTING,
                    "map": TARGET_MAP,
                    "primary_profile_id": str(PRIMARY_PROFILE_ID),
                    "secondary_profile_id": str(SECONDARY_PROFILE_ID),
                    "primary_slot": str(PRIMARY_SLOT_ID),
                    "secondary_slot": str(SECONDARY_SLOT_ID),
                                        "primary_player_name": "유한탄 중포 키트",
                    "secondary_player_name": "유한탄 Ripple 키트",
                },
                "snapshot_readback": snapshot_readback,
                "user_gate": [
                    "Play TestMap_WeaponSelect.",
                    "초기 선택 Header 이름과 Rail의 비선택 이름이 서로 다른지 확인.",
                                        "숫자 1을 눌러 유한탄 중포 키트 선택과 유한탄 Ripple 키트 Rail 표시 확인.",
                    "숫자 2를 눌러 유한탄 Ripple 키트 선택과 유한탄 중포 키트 Rail 표시 확인.",
                    "숫자 3을 눌러 선택이 바뀌지 않는지 확인.",
                    "Mouse Wheel로 무기 선택이 바뀌지 않는지 확인.",
                    "Rail/Header에 MountProfileId, WeaponId, AssetName이 노출되지 않는지 확인.",
                ],
            }
        )
        unreal.log("[CarFight] WeaponSelect USER fixture 생성/검증 완료")

    except Exception as exc:
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
                "fixture_mode": fixture_mode,
                "error": str(exc),
                "traceback": traceback.format_exc(),
                "created_assets_before_cleanup": created_assets,
                "cleanup_errors": cleanup_errors,
            }
        )
        unreal.log_error(f"[CarFight] WeaponSelect fixture 생성 실패: {exc}")
        raise


main()
