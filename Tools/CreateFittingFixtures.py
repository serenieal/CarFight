# Copyright (c) CarFight. All Rights Reserved.
# Version: v1.1.0
# Date: 2026-08-17
# Description: CF-FQ-034 FIT-P0-07B 동일 SUV 플랫폼의 Light / Default / Heavy 공식 Mobility Fitting Fixture 3종을 생성하고 Snapshot 질량과 사용자 표시명을 검증합니다.
# Changelog:
# - v1.1.0: 공식 Fixture 3종의 DisplayName을 모빌리티 경량/기본/중량 피팅으로 고정하고, 기존 3종이 모두 존재할 때 Snapshot 계약 검증 후 DisplayName만 안전하게 교정하는 경로를 추가.
# - v1.0.0: 기존 저장 VehicleData / Defense / HeavyFinite payload만 재사용해 1000kg / 1570kg / 1600kg Fixture를 생성하고 BuildFittingSnapshot readback을 검증하는 최초 버전.
# Migration:
# - Production VehicleData, WeaponData, DefenseData, AmmoData의 질량값은 변경하지 않습니다.
# - 대상 3종이 모두 없을 때만 생성하고, 부분 존재는 안전하게 거부합니다. 3종이 모두 존재하면 질량·참조 계약을 먼저 검증한 뒤 DisplayName만 공식 이름으로 교정합니다.

import json
import os
import traceback
import unreal

# 공식 Mobility Fixture가 공유할 Fitting-ready SUV VehicleData입니다.
SOURCE_VEHICLE = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV"
# Default 1570kg 구성의 검증된 기존 FittingData입니다.
SOURCE_DEFAULT_FITTING = "/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV"
# Heavy payload의 Mount / Weapon / Ammo 선택을 보존한 기존 FittingData입니다.
SOURCE_HEAVY_FITTING = "/Game/CarFight/Tests/AmmoIntegration/DA_Fit_HeavyFinite"
# 공식 Mobility Fixture를 격리할 테스트 폴더입니다.
TARGET_FOLDER = "/Game/CarFight/Tests/Fitting"
# 장비·방어·탄약이 없는 동일 SUV Light Fixture입니다.
TARGET_LIGHT = f"{TARGET_FOLDER}/DA_Fit_MobilityLight"
# 기존 검증된 1570kg 구성을 복제한 Default Fixture입니다.
TARGET_DEFAULT = f"{TARGET_FOLDER}/DA_Fit_MobilityDefault"
# 기존 HeavyFinite payload를 동일 SUV와 기본 방어에 재조합한 Heavy Fixture입니다.
TARGET_HEAVY = f"{TARGET_FOLDER}/DA_Fit_MobilityHeavy"
# Light Fixture의 사용자 표시 이름입니다.
DISPLAY_NAME_LIGHT = "모빌리티 경량 피팅"
# Default Fixture의 사용자 표시 이름입니다.
DISPLAY_NAME_DEFAULT = "모빌리티 기본 피팅"
# Heavy Fixture의 사용자 표시 이름입니다.
DISPLAY_NAME_HEAVY = "모빌리티 중량 피팅"
# commandlet 실행 결과를 기록할 Saved 폴더입니다.
RESULT_DIR = os.path.join(unreal.Paths.project_saved_dir(), "CarFight")
# Browser process runner가 판정할 결과 JSON입니다.
RESULT_PATH = os.path.join(RESULT_DIR, "FittingMobilityFixtureResult.json")


# 실행 결과를 UTF-8 JSON으로 저장합니다.
def write_result(payload):
    os.makedirs(RESULT_DIR, exist_ok=True)
    with open(RESULT_PATH, "w", encoding="utf-8") as result_file:
        json.dump(payload, result_file, ensure_ascii=False, indent=2)


# 필수 기존 자산을 존재 확인 후 로드합니다.
def require_asset(asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"필수 원본 자산이 없습니다: {asset_path}")
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"필수 원본 자산을 로드하지 못했습니다: {asset_path}")
    return asset


# 대상 Fixture 3종의 생성/검증 모드를 안전하게 판정합니다.
def resolve_fixture_mode():
    target_paths = (TARGET_LIGHT, TARGET_DEFAULT, TARGET_HEAVY)
    existing_targets = [
        asset_path
        for asset_path in target_paths
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    ]
    if not existing_targets:
        return "create"
    if len(existing_targets) == len(target_paths):
        return "verify_existing"
    raise RuntimeError(
        "Mobility Fixture가 부분적으로 존재해 자동 수정하지 않습니다: " + ", ".join(existing_targets)
    )


# 원본 자산을 새 경로로 복제하고 이번 실행의 생성 목록에 기록합니다.
def duplicate_asset(source_path, target_path, created_assets):
    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"자산 복제 실패: {source_path} -> {target_path}")
    created_assets.append(target_path)
    return duplicated_asset


# UE Python 이름과 C++ 원본 이름 후보 중 실제 Reflection Property 하나를 찾아 설정합니다.
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


# UE Python 이름과 C++ 원본 이름 후보 중 실제 Reflection Property 하나를 읽습니다.
def get_reflected_property(target_object, property_names):
    errors = []
    for property_name in property_names:
        try:
            return property_name, target_object.get_editor_property(property_name)
        except Exception as exc:
            errors.append(f"{property_name}: {exc}")
    raise RuntimeError(
        f"Property 조회 실패: 후보={property_names} / 오류={' | '.join(errors)}"
    )


# Unreal Python Enum 타입/멤버 이름 후보에서 정확한 값을 찾습니다.
def resolve_enum_value(enum_type_names, member_names):
    for enum_type_name in enum_type_names:
        enum_type = getattr(unreal, enum_type_name, None)
        if enum_type is None:
            continue
        for member_name in member_names:
            if hasattr(enum_type, member_name):
                return getattr(enum_type, member_name)
    raise RuntimeError(
        f"Enum 값을 찾지 못했습니다: types={enum_type_names}, members={member_names}"
    )


# 저장된 MountSelection과 같은 MountProfileId를 쓰는 독립 장착 선택 구조체를 생성합니다.
def build_mount_selection(source_selection, equipment_preset, enabled):
    mount_selection_type = getattr(unreal, "CFVehicleMountSelection", None)
    if mount_selection_type is None:
        raise RuntimeError("Unreal Python에 CFVehicleMountSelection 구조체가 노출되지 않았습니다.")
    _, mount_profile_id = get_reflected_property(source_selection, ("mount_profile_id", "MountProfileId"))
    replacement_selection = mount_selection_type()
    set_reflected_property(replacement_selection, ("mount_profile_id", "MountProfileId"), mount_profile_id)
    set_reflected_property(
        replacement_selection,
        ("equipment_preset_data", "EquipmentPresetData"),
        equipment_preset,
    )
    set_reflected_property(
        replacement_selection,
        ("enabled", "b_enabled", "bEnabled"),
        bool(enabled),
    )
    return replacement_selection


# Fixture의 DisplayName이 공식 사용자 표시 이름과 다를 때만 해당 FText Property를 교정하고 변경 여부를 반환합니다.
def ensure_display_name(fitting_asset, expected_display_name):
    property_name, current_display_name = get_reflected_property(
        fitting_asset, ("display_name", "DisplayName")
    )
    if str(current_display_name) == expected_display_name:
        return property_name, False
    fitting_asset.set_editor_property(property_name, expected_display_name)
    return property_name, True


# 명시적인 방어 선택 구조체를 새로 생성합니다.
def build_defense_selection(selection_mode, defense_data=None):
    defense_selection_type = getattr(unreal, "CFVehicleDefenseSelection", None)
    if defense_selection_type is None:
        raise RuntimeError("Unreal Python에 CFVehicleDefenseSelection 구조체가 노출되지 않았습니다.")
    defense_selection = defense_selection_type()
    set_reflected_property(defense_selection, ("selection_mode", "SelectionMode"), selection_mode)
    set_reflected_property(defense_selection, ("defense_data", "DefenseData"), defense_data)
    return defense_selection


# FittingData의 BlueprintPure BuildFittingSnapshot 결과에서 질량 계약을 읽어 숫자 사전으로 반환합니다.
def read_snapshot_mass(fitting_asset):
    snapshot = fitting_asset.build_fitting_snapshot()
    _, validation_state = get_reflected_property(snapshot, ("validation_state", "ValidationState"))
    _, base_mass = get_reflected_property(snapshot, ("base_vehicle_mass_kg", "BaseVehicleMassKg"))
    _, equipment_mass = get_reflected_property(snapshot, ("equipment_mass_kg", "EquipmentMassKg"))
    _, ammo_mass = get_reflected_property(snapshot, ("ammo_mass_kg", "AmmoMassKg"))
    _, defense_mass = get_reflected_property(snapshot, ("defense_mass_kg", "DefenseMassKg"))
    _, total_mass = get_reflected_property(snapshot, ("total_vehicle_mass_kg", "TotalVehicleMassKg"))
    _, gross_mass = get_reflected_property(snapshot, ("maximum_gross_mass_kg", "MaximumGrossMassKg"))
    return {
        "validation_state": str(validation_state),
        "base_mass_kg": float(base_mass),
        "equipment_mass_kg": float(equipment_mass),
        "ammo_mass_kg": float(ammo_mass),
        "defense_mass_kg": float(defense_mass),
        "total_mass_kg": float(total_mass),
        "maximum_gross_mass_kg": float(gross_mass),
    }


# 하나의 Snapshot 질량과 유효 상태가 기대 계약과 정확히 일치하는지 검증합니다.
def validate_snapshot(label, snapshot_mass, expected):
    validation_text = snapshot_mass["validation_state"].lower()
    if "invalid" in validation_text or "not_evaluated" in validation_text or "notevaluated" in validation_text:
        raise RuntimeError(f"{label} Snapshot이 유효하지 않습니다: {snapshot_mass['validation_state']}")
    for field_name, expected_value in expected.items():
        actual_value = snapshot_mass[field_name]
        if abs(actual_value - expected_value) > 0.01:
            raise RuntimeError(
                f"{label} {field_name} 불일치: expected={expected_value}, actual={actual_value}"
            )


# 이번 create 실행에서 새로 만든 Fixture만 역순으로 삭제합니다.
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


# 세 Mobility Fixture를 생성하거나 기존 3종을 수정 없이 검증합니다.
def main():
    created_assets = []
    resolved_properties = {}
    fixture_mode = "unknown"
    try:
        fixture_mode = resolve_fixture_mode()
        source_vehicle = require_asset(SOURCE_VEHICLE)
        source_default_fitting = require_asset(SOURCE_DEFAULT_FITTING)
        source_heavy_fitting = require_asset(SOURCE_HEAVY_FITTING)
        explicit_none_mode = resolve_enum_value(
            ("CFDefenseSelectionMode", "ECFDefenseSelectionMode"),
            ("EXPLICIT_NONE", "ExplicitNone"),
        )
        use_vehicle_default_mode = resolve_enum_value(
            ("CFDefenseSelectionMode", "ECFDefenseSelectionMode"),
            ("USE_VEHICLE_DEFAULT", "UseVehicleDefault"),
        )

        if fixture_mode == "create":
            if not unreal.EditorAssetLibrary.does_directory_exist(TARGET_FOLDER):
                if not unreal.EditorAssetLibrary.make_directory(TARGET_FOLDER):
                    raise RuntimeError(f"Fixture 폴더 생성 실패: {TARGET_FOLDER}")

            light_fitting = duplicate_asset(SOURCE_DEFAULT_FITTING, TARGET_LIGHT, created_assets)
            resolved_properties["light_fitting_id"] = set_reflected_property(
                light_fitting, ("fitting_id", "FittingId"), unreal.Name("Mobility_Light")
            )
            resolved_properties["light_display_name"] = set_reflected_property(
                light_fitting, ("display_name", "DisplayName"), DISPLAY_NAME_LIGHT
            )
            resolved_properties["light_vehicle"] = set_reflected_property(
                light_fitting, ("vehicle_data", "VehicleData"), source_vehicle
            )
            light_mount_property, light_mount_selections = get_reflected_property(
                light_fitting, ("mount_selections", "MountSelections")
            )
            if len(light_mount_selections) != 1:
                raise RuntimeError(f"Light 원본 MountSelection이 1개가 아닙니다: {len(light_mount_selections)}")
            light_empty_selection = build_mount_selection(light_mount_selections[0], None, False)
            light_fitting.set_editor_property(light_mount_property, [light_empty_selection])
            resolved_properties["light_mount_selections"] = light_mount_property
            light_ammo_property, _ = get_reflected_property(
                light_fitting, ("initial_sortie_ammo_loads", "InitialSortieAmmoLoads")
            )
            light_fitting.set_editor_property(light_ammo_property, [])
            resolved_properties["light_ammo_loads"] = light_ammo_property
            light_defense = build_defense_selection(explicit_none_mode, None)
            resolved_properties["light_defense_selection"] = set_reflected_property(
                light_fitting, ("defense_selection", "DefenseSelection"), light_defense
            )
            if not unreal.EditorAssetLibrary.save_asset(TARGET_LIGHT, only_if_is_dirty=False):
                raise RuntimeError(f"Light Fixture 저장 실패: {TARGET_LIGHT}")

            default_fitting = duplicate_asset(SOURCE_DEFAULT_FITTING, TARGET_DEFAULT, created_assets)
            resolved_properties["default_fitting_id"] = set_reflected_property(
                default_fitting, ("fitting_id", "FittingId"), unreal.Name("Mobility_Default")
            )
            resolved_properties["default_display_name"] = set_reflected_property(
                default_fitting, ("display_name", "DisplayName"), DISPLAY_NAME_DEFAULT
            )
            resolved_properties["default_vehicle"] = set_reflected_property(
                default_fitting, ("vehicle_data", "VehicleData"), source_vehicle
            )
            if not unreal.EditorAssetLibrary.save_asset(TARGET_DEFAULT, only_if_is_dirty=False):
                raise RuntimeError(f"Default Fixture 저장 실패: {TARGET_DEFAULT}")

            heavy_fitting = duplicate_asset(SOURCE_HEAVY_FITTING, TARGET_HEAVY, created_assets)
            resolved_properties["heavy_fitting_id"] = set_reflected_property(
                heavy_fitting, ("fitting_id", "FittingId"), unreal.Name("Mobility_Heavy")
            )
            resolved_properties["heavy_display_name"] = set_reflected_property(
                heavy_fitting, ("display_name", "DisplayName"), DISPLAY_NAME_HEAVY
            )
            resolved_properties["heavy_vehicle"] = set_reflected_property(
                heavy_fitting, ("vehicle_data", "VehicleData"), source_vehicle
            )
            heavy_defense = build_defense_selection(use_vehicle_default_mode, None)
            resolved_properties["heavy_defense_selection"] = set_reflected_property(
                heavy_fitting, ("defense_selection", "DefenseSelection"), heavy_defense
            )
            if not unreal.EditorAssetLibrary.save_asset(TARGET_HEAVY, only_if_is_dirty=False):
                raise RuntimeError(f"Heavy Fixture 저장 실패: {TARGET_HEAVY}")
        else:
            light_fitting = require_asset(TARGET_LIGHT)
            default_fitting = require_asset(TARGET_DEFAULT)
            heavy_fitting = require_asset(TARGET_HEAVY)

        light_snapshot = read_snapshot_mass(light_fitting)
        default_snapshot = read_snapshot_mass(default_fitting)
        heavy_snapshot = read_snapshot_mass(heavy_fitting)

        validate_snapshot(
            "Light",
            light_snapshot,
            {
                "base_mass_kg": 1000.0,
                "equipment_mass_kg": 0.0,
                "ammo_mass_kg": 0.0,
                "defense_mass_kg": 0.0,
                "total_mass_kg": 1000.0,
                "maximum_gross_mass_kg": 2500.0,
            },
        )
        validate_snapshot(
            "Default",
            default_snapshot,
            {
                "base_mass_kg": 1000.0,
                "equipment_mass_kg": 470.0,
                "ammo_mass_kg": 0.0,
                "defense_mass_kg": 100.0,
                "total_mass_kg": 1570.0,
                "maximum_gross_mass_kg": 2500.0,
            },
        )
        validate_snapshot(
            "Heavy",
            heavy_snapshot,
            {
                "base_mass_kg": 1000.0,
                "equipment_mass_kg": 470.0,
                "ammo_mass_kg": 30.0,
                "defense_mass_kg": 100.0,
                "total_mass_kg": 1600.0,
                "maximum_gross_mass_kg": 2500.0,
            },
        )
        if not (
            light_snapshot["total_mass_kg"]
            < default_snapshot["total_mass_kg"]
            < heavy_snapshot["total_mass_kg"]
        ):
            raise RuntimeError(
                "공식 Mobility Fixture 총중량 순서가 Light < Default < Heavy를 만족하지 않습니다."
            )

        # [v1.1.0] 기존 Fixture 검증 모드에서는 구조·질량 계약이 PASS한 뒤에만 사용자 표시 이름을 교정합니다.
        display_name_changes = {}
        for label, fitting_asset, asset_path, expected_display_name in (
            ("light", light_fitting, TARGET_LIGHT, DISPLAY_NAME_LIGHT),
            ("default", default_fitting, TARGET_DEFAULT, DISPLAY_NAME_DEFAULT),
            ("heavy", heavy_fitting, TARGET_HEAVY, DISPLAY_NAME_HEAVY),
        ):
            display_property_name, display_name_changed = ensure_display_name(
                fitting_asset, expected_display_name
            )
            resolved_properties[f"{label}_display_name"] = display_property_name
            display_name_changes[label] = display_name_changed
            if display_name_changed:
                if not unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False):
                    raise RuntimeError(f"{label} Fixture DisplayName 저장 실패: {asset_path}")

        write_result(
            {
                "status": "success",
                "fixture_mode": fixture_mode,
                "created_assets": created_assets,
                "resolved_properties": resolved_properties,
                "target_contract": {
                    "vehicle": SOURCE_VEHICLE,
                    "light": TARGET_LIGHT,
                    "default": TARGET_DEFAULT,
                    "heavy": TARGET_HEAVY,
                },
                "snapshot_mass": {
                    "light": light_snapshot,
                    "default": default_snapshot,
                    "heavy": heavy_snapshot,
                },
                "mass_order_passed": True,
                "display_name_changes": display_name_changes,
                "display_names": {
                    "light": DISPLAY_NAME_LIGHT,
                    "default": DISPLAY_NAME_DEFAULT,
                    "heavy": DISPLAY_NAME_HEAVY,
                },
                "production_mass_value_mutation": False,
            }
        )
        unreal.log("[CarFight] Mobility Fitting Fixture 생성/검증 완료")

    except Exception as exc:
        cleanup_errors = cleanup_created_assets(created_assets) if fixture_mode == "create" else []
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
        unreal.log_error(f"[CarFight] Mobility Fitting Fixture 실패: {exc}")
        raise


main()
