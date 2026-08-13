# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.1.0
# Date: 2026-08-03
# Description: DR-P0-07 방어 테스트 SUV 전용 VehicleFittingData와 테스트 전용 장비 복제본을 생성하고 테스트 맵에 연결합니다.
# Scope: 방어 테스트 VehicleData·DefenseData의 피팅 질량, 테스트 전용 Weapon·Preset·Fitting과 M_VehicleDefensePIE 단일 차량만 생성·수정합니다.
# Changelog:
# - v1.1.0: 공유 RocketLauncher WeaponData의 0kg 계약을 보호하기 위해 테스트 전용 Weapon·Preset 복제본과 P0 자동화 기준 질량을 추가.
# - v1.0.0: 기본 장비·기본 방어 선택 피팅 생성, Snapshot 검증, VehicleData 기준 단일 Actor 연결과 보호 해시 검증을 추가.
# Migration:
# - DA_TestSUV, DA_TestSedan, BP_CFVehiclePawn, 공유 RocketLauncher Weapon·Preset·TurretMount는 읽기 전용으로 보호합니다.
# - 테스트 질량은 생산 밸런스가 아니라 기존 Fitting Automation 기준의 DR-P0-07 검증값입니다.
# - VehicleData가 DA_VehicleDefense_TestSUV인 맵 Actor가 정확히 1대가 아니면 맵을 저장하지 않습니다.

from __future__ import annotations

import hashlib
import json
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.1.0] 실행 결과와 문서에서 식별할 현재 도구 버전입니다.
TOOL_VERSION = "1.1.0"

# [v1.0.0] 명시적 Apply 실행일 때만 신규 에셋과 테스트 맵을 저장하는 환경 변수 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_DEFENSE_FITTING_APPLY", "0") == "1"

# [v1.0.0] Unreal 프로젝트 루트의 절대 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 실행 결과 JSON을 저장할 프로젝트 Saved 하위 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "DefenseTestFittingApply"

# [v1.0.0] MCP와 후속 세션이 읽을 구조화 결과 JSON 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] 방어 테스트 피팅·장비 DataAsset을 저장할 Unreal 패키지 폴더입니다.
TEST_DATA_FOLDER = "/Game/CarFight/Tests/VehicleDefense/Data"

# [v1.0.0] 생성할 방어 테스트 피팅 DataAsset 이름입니다.
FITTING_DATA_NAME = "DA_Fit_DefenseTestSUV"

# [v1.0.0] 생성할 방어 테스트 피팅 DataAsset 전체 경로입니다.
FITTING_DATA_PATH = f"{TEST_DATA_FOLDER}/{FITTING_DATA_NAME}"

# [v1.1.0] 공유 RocketLauncher를 수정하지 않고 사용할 테스트 전용 WeaponData 경로입니다.
TEST_WEAPON_DATA_PATH = f"{TEST_DATA_FOLDER}/DA_Wpn_DefenseTest"

# [v1.1.0] 테스트 전용 WeaponData를 연결할 EquipmentPresetData 경로입니다.
TEST_EQUIPMENT_PRESET_PATH = f"{TEST_DATA_FOLDER}/Preset_DefenseTest"

# [v1.0.0] 피팅의 기준 차량으로 사용할 방어 테스트 SUV VehicleData 경로입니다.
TARGET_VEHICLE_DATA_PATH = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV"

# [v1.0.0] VehicleData의 기본 방어로 해석될 VehicleDefenseData 경로입니다.
TARGET_DEFENSE_DATA_PATH = "/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test"

# [v1.0.0] 방어 테스트 차량에 피팅을 연결할 전용 PIE 맵 경로입니다.
TARGET_MAP_PATH = "/Game/Maps/M_VehicleDefensePIE"

# [v1.1.0] 테스트 WeaponData 복제 원본인 공유 RocketLauncher WeaponData 경로입니다.
SOURCE_WEAPON_DATA_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher"

# [v1.1.0] 테스트 EquipmentPresetData 복제 원본인 공유 RocketLauncher Preset 경로입니다.
SOURCE_EQUIPMENT_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher"

# [v1.1.0] 테스트 Preset이 그대로 사용할 공유 RocketLauncher TurretMountData 경로입니다.
SOURCE_TURRET_MOUNT_PATH = "/Game/CarFight/Weapons/Data/TurretMounts/DA_RocketBody"

# [v1.0.0] 원본 Launcher 구성을 보존할 읽기 전용 VehicleData 경로입니다.
PROTECTED_SOURCE_SUV_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"

# [v1.0.0] Legacy 방어 없음 계약을 보존할 읽기 전용 VehicleData 경로입니다.
PROTECTED_SEDAN_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan"

# [v1.0.0] 기존 차량 Blueprint 기본값을 보존할 읽기 전용 에셋 경로입니다.
PROTECTED_VEHICLE_BP_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn"

# [v1.1.0] Fitting Automation과 같은 기준으로 사용할 테스트 차량 플랫폼 질량입니다.
TEST_BASE_VEHICLE_MASS_KG = 1000.0

# [v1.1.0] Fitting Automation과 같은 기준으로 사용할 테스트 차량 최대 허용 총중량입니다.
TEST_MAXIMUM_GROSS_MASS_KG = 2500.0

# [v1.1.0] Fitting Automation의 Legacy Defense 기준으로 사용할 테스트 방어 패키지 질량입니다.
TEST_DEFENSE_MASS_KG = 100.0

# [v1.1.0] Fitting Automation의 Legacy Weapon 기준으로 사용할 테스트 전용 무기 질량입니다.
TEST_WEAPON_MASS_KG = 120.0

# [v1.1.0] 이번 도구가 생성 또는 수정할 수 있는 유일한 에셋·맵 경로 집합입니다.
MUTABLE_ASSET_PATHS = {
    TARGET_VEHICLE_DATA_PATH,
    TARGET_DEFENSE_DATA_PATH,
    TEST_WEAPON_DATA_PATH,
    TEST_EQUIPMENT_PRESET_PATH,
    FITTING_DATA_PATH,
    TARGET_MAP_PATH,
}

# [v1.1.0] 파일 해시가 변경되면 실패시킬 읽기 전용 보호 에셋 경로입니다.
PROTECTED_ASSET_PATHS = {
    PROTECTED_SOURCE_SUV_PATH,
    PROTECTED_SEDAN_PATH,
    PROTECTED_VEHICLE_BP_PATH,
    SOURCE_WEAPON_DATA_PATH,
    SOURCE_EQUIPMENT_PRESET_PATH,
    SOURCE_TURRET_MOUNT_PATH,
}

# [v1.1.0] 실행 모드, 생성·저장 목록, Snapshot과 맵 연결 검증을 기록할 결과 객체입니다.
REPORT: dict[str, Any] = {
    "schema_version": "defense_test_fitting_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "fitting_data_path": FITTING_DATA_PATH,
    "test_weapon_data_path": TEST_WEAPON_DATA_PATH,
    "test_equipment_preset_path": TEST_EQUIPMENT_PRESET_PATH,
    "target_vehicle_data_path": TARGET_VEHICLE_DATA_PATH,
    "target_defense_data_path": TARGET_DEFENSE_DATA_PATH,
    "target_map_path": TARGET_MAP_PATH,
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "planned_changes": [],
    "vehicle_mass_contract": {},
    "defense_mass_contract": {},
    "equipment_mass_contract": [],
    "snapshot": {},
    "map_actor_before": {},
    "map_actor_after": {},
    "map_vehicle_actors_before": [],
    "map_vehicle_actors_after": [],
    "protected_hashes_before": {},
    "protected_hashes_after": {},
    "mutable_hashes_before": {},
    "mutable_hashes_after": {},
    "snapshot_valid": False,
    "map_connected": False,
    "protected_contract_passed": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 방어 테스트 피팅 도구 메시지를 남깁니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][DefenseTestFitting] {message}")


# [v1.0.0] C++ PascalCase 프로퍼티 이름을 Unreal Python snake_case 후보로 변환합니다.
def pascal_to_snake(name: str) -> str:
    step1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", step1).lower()


# [v1.0.0] Reflection 프로퍼티 접근에 사용할 원본·snake_case 후보 목록을 반환합니다.
def candidate_property_names(name: str) -> list[str]:
    return list(dict.fromkeys([name, pascal_to_snake(name)]))


# [v1.0.0] 원본 또는 snake_case 이름으로 Unreal Reflection 프로퍼티를 읽습니다.
def get_property(target: Any, property_name: str) -> Any:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            return target.get_editor_property(candidate)
        except Exception as error:
            last_error = error
    raise RuntimeError(
        f"Property read failed: {type(target).__name__}.{property_name}: {last_error}"
    )


# [v1.0.0] 원본 또는 snake_case 이름으로 Unreal Reflection 프로퍼티를 씁니다.
def set_property(target: Any, property_name: str, value: Any) -> None:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            target.set_editor_property(candidate, value)
            return
        except Exception as error:
            last_error = error
    raise RuntimeError(
        f"Property write failed: {type(target).__name__}.{property_name}: {last_error}"
    )


# [v1.0.0] Unreal 객체의 안정적인 전체 Object Path 문자열을 반환합니다.
def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        return unreal.SystemLibrary.get_path_name(value)
    except Exception:
        return str(value)


# [v1.0.0] Unreal 에셋 경로를 프로젝트 Content의 실제 패키지 파일 경로로 변환합니다.
def asset_file_path(asset_path: str) -> Path:
    if not asset_path.startswith("/Game/"):
        raise RuntimeError(f"Only /Game asset paths are supported: {asset_path}")
    extension = ".umap" if asset_path == TARGET_MAP_PATH else ".uasset"
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'):]}{extension}"


# [v1.0.0] 존재하는 파일의 SHA-256을 계산하고 없으면 빈 문자열을 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
    digest = hashlib.sha256()
    with file_path.open("rb") as source_file:
        for chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


# [v1.0.0] 지정된 Unreal 에셋 경로 집합의 파일 해시를 반환합니다.
def capture_hashes(asset_paths: set[str]) -> dict[str, str]:
    return {
        asset_path: sha256_file(asset_file_path(asset_path))
        for asset_path in sorted(asset_paths)
    }


# [v1.0.0] 필수 에셋을 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return asset


# [v1.0.0] Unreal enum 타입과 멤버 이름을 안전하게 Reflection 값으로 변환합니다.
def enum_value(enum_type_name: str, member_name: str) -> Any:
    enum_type = getattr(unreal, enum_type_name, None)
    if enum_type is None:
        raise RuntimeError(f"Unreal enum type was not found: {enum_type_name}")

    for candidate in [member_name, member_name.upper(), pascal_to_snake(member_name).upper()]:
        if hasattr(enum_type, candidate):
            return getattr(enum_type, candidate)

    available_members = [name for name in dir(enum_type) if name.isupper()]
    raise RuntimeError(
        f"Enum member was not found: {enum_type_name}.{member_name}; available={available_members}"
    )


# [v1.0.0] 실제 쓰기 전에 계획된 단일 프로퍼티 변경을 결과 JSON에 기록합니다.
def record_change(asset_path: str, property_name: str, value: Any) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in the mutation whitelist: {asset_path}")
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.1.0] 테스트 전용 복제 대상 에셋을 로드하거나 Apply 모드에서 생성합니다.
def duplicate_or_load_asset(source_path: str, target_path: str) -> Any | None:
    if target_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Target path is not in the mutation whitelist: {target_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        REPORT["updated_assets"].append(target_path)
        return require_asset(target_path)

    if not APPLY_CHANGES:
        record_change(target_path, "DuplicateAsset", source_path)
        return None

    if not unreal.EditorAssetLibrary.does_directory_exist(TEST_DATA_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(TEST_DATA_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {TEST_DATA_FOLDER}")

    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"Asset duplication failed: {source_path} -> {target_path}")

    REPORT["created_assets"].append(target_path)
    return duplicated_asset


# [v1.0.0] 방어 테스트 피팅 DataAsset을 로드하거나 Apply 모드에서 신규 생성합니다.
def load_or_create_fitting_asset() -> Any | None:
    if unreal.EditorAssetLibrary.does_asset_exist(FITTING_DATA_PATH):
        REPORT["updated_assets"].append(FITTING_DATA_PATH)
        return require_asset(FITTING_DATA_PATH)

    if not APPLY_CHANGES:
        record_change(FITTING_DATA_PATH, "CreateAsset", "CFVehicleFittingData")
        return None

    if not unreal.EditorAssetLibrary.does_directory_exist(TEST_DATA_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(TEST_DATA_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {TEST_DATA_FOLDER}")

    fitting_data_class = getattr(unreal, "CFVehicleFittingData", None)
    if fitting_data_class is None:
        raise RuntimeError("Unreal class was not found: CFVehicleFittingData")

    data_asset_factory = unreal.DataAssetFactory()
    set_property(data_asset_factory, "DataAssetClass", fitting_data_class)

    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        FITTING_DATA_NAME,
        TEST_DATA_FOLDER,
        fitting_data_class,
        data_asset_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"VehicleFittingData asset creation failed: {FITTING_DATA_PATH}")

    REPORT["created_assets"].append(FITTING_DATA_PATH)
    return created_asset


# [v1.1.0] 테스트 소유 Vehicle·Defense·Weapon·Preset에 P0 Snapshot 검증용 질량과 참조를 설정합니다.
def configure_test_assets(
    vehicle_data: Any,
    defense_data: Any,
    test_weapon_data: Any,
    test_equipment_preset: Any,
    source_turret_mount: Any,
) -> None:
    record_change(TARGET_VEHICLE_DATA_PATH, "BaseVehicleMassKg", TEST_BASE_VEHICLE_MASS_KG)
    record_change(TARGET_VEHICLE_DATA_PATH, "MaximumGrossMassKg", TEST_MAXIMUM_GROSS_MASS_KG)
    record_change(TARGET_DEFENSE_DATA_PATH, "DefenseMassKg", TEST_DEFENSE_MASS_KG)
    record_change(TEST_WEAPON_DATA_PATH, "WeaponId", "DefenseTestLauncher")
    record_change(TEST_WEAPON_DATA_PATH, "WeaponMassKg", TEST_WEAPON_MASS_KG)
    record_change(TEST_EQUIPMENT_PRESET_PATH, "EquipmentId", "DefenseTestLauncherKit")
    record_change(TEST_EQUIPMENT_PRESET_PATH, "DisplayName", "방어 테스트 런처 키트")
    record_change(TEST_EQUIPMENT_PRESET_PATH, "DefaultTurretMountData", source_turret_mount)
    record_change(TEST_EQUIPMENT_PRESET_PATH, "DefaultWeaponData", test_weapon_data)

    set_property(vehicle_data, "BaseVehicleMassKg", TEST_BASE_VEHICLE_MASS_KG)
    set_property(vehicle_data, "MaximumGrossMassKg", TEST_MAXIMUM_GROSS_MASS_KG)
    set_property(defense_data, "DefenseMassKg", TEST_DEFENSE_MASS_KG)
    set_property(test_weapon_data, "WeaponId", "DefenseTestLauncher")
    set_property(test_weapon_data, "WeaponMassKg", TEST_WEAPON_MASS_KG)
    set_property(test_equipment_preset, "EquipmentId", "DefenseTestLauncherKit")
    set_property(test_equipment_preset, "DisplayName", "방어 테스트 런처 키트")
    set_property(test_equipment_preset, "DefaultTurretMountData", source_turret_mount)
    set_property(test_equipment_preset, "DefaultWeaponData", test_weapon_data)


# [v1.1.0] 신규 피팅이 테스트 전용 장비와 차량 기본 방어를 사용하도록 설정합니다.
def configure_fitting_asset(fitting_asset: Any, vehicle_data: Any, test_equipment_preset: Any) -> None:
    mount_profiles = list(get_property(vehicle_data, "MountProfiles"))
    if len(mount_profiles) != 1:
        raise RuntimeError(
            "DA_VehicleDefense_TestSUV must contain exactly one MountProfile for this bounded test fitting: "
            f"count={len(mount_profiles)}"
        )

    mount_selection_class = getattr(unreal, "CFVehicleMountSelection", None)
    if mount_selection_class is None:
        raise RuntimeError("Unreal struct was not found: CFVehicleMountSelection")

    mount_selection = mount_selection_class()
    mount_profile_id = get_property(mount_profiles[0], "MountProfileId")
    set_property(mount_selection, "MountProfileId", mount_profile_id)
    set_property(mount_selection, "EquipmentPresetData", test_equipment_preset)
    set_property(mount_selection, "bEnabled", True)

    missing_mount_policy = enum_value("CFMissingMountPolicy", "UseVehicleDefault")
    defense_selection_mode = enum_value("CFDefenseSelectionMode", "UseVehicleDefault")
    defense_selection = get_property(fitting_asset, "DefenseSelection")

    record_change(FITTING_DATA_PATH, "FittingId", "DefenseTestSUV")
    record_change(FITTING_DATA_PATH, "DisplayName", "방어 테스트 SUV 피팅")
    record_change(FITTING_DATA_PATH, "VehicleData", vehicle_data)
    record_change(FITTING_DATA_PATH, "MountSelections[0].MountProfileId", mount_profile_id)
    record_change(FITTING_DATA_PATH, "MountSelections[0].EquipmentPresetData", test_equipment_preset)
    record_change(FITTING_DATA_PATH, "MissingMountSelectionPolicy", missing_mount_policy)
    record_change(FITTING_DATA_PATH, "DefenseSelection.SelectionMode", defense_selection_mode)
    record_change(FITTING_DATA_PATH, "DefenseSelection.DefenseData", "None")

    set_property(fitting_asset, "FittingId", "DefenseTestSUV")
    set_property(fitting_asset, "DisplayName", "방어 테스트 SUV 피팅")
    set_property(fitting_asset, "VehicleData", vehicle_data)
    set_property(fitting_asset, "MountSelections", [mount_selection])
    set_property(fitting_asset, "MissingMountSelectionPolicy", missing_mount_policy)
    set_property(defense_selection, "SelectionMode", defense_selection_mode)
    set_property(defense_selection, "DefenseData", None)
    set_property(fitting_asset, "DefenseSelection", defense_selection)


# [v1.1.0] Vehicle·Defense·Equipment 질량 계약을 읽어 보고하고 유효성을 검증합니다.
def validate_mass_contracts(vehicle_data: Any, defense_data: Any) -> None:
    base_vehicle_mass_kg = float(get_property(vehicle_data, "BaseVehicleMassKg"))
    maximum_gross_mass_kg = float(get_property(vehicle_data, "MaximumGrossMassKg"))
    defense_mass_kg = float(get_property(defense_data, "DefenseMassKg"))
    mount_profiles = list(get_property(vehicle_data, "MountProfiles"))
    equipment_mass_rows: list[dict[str, Any]] = []

    for mount_profile in mount_profiles:
        equipment_preset = get_property(mount_profile, "DefaultEquipmentPresetData")
        turret_mount_data = (
            get_property(equipment_preset, "DefaultTurretMountData")
            if equipment_preset is not None
            else None
        )
        weapon_data = (
            get_property(equipment_preset, "DefaultWeaponData")
            if equipment_preset is not None
            else None
        )
        equipment_mass_rows.append(
            {
                "mount_profile_id": str(get_property(mount_profile, "MountProfileId")),
                "equipment_preset": object_path(equipment_preset),
                "turret_mount_data": object_path(turret_mount_data),
                "turret_mount_mass_kg": (
                    float(get_property(turret_mount_data, "TurretMountWeightKg"))
                    if turret_mount_data is not None
                    else 0.0
                ),
                "weapon_data": object_path(weapon_data),
                "weapon_mass_kg": (
                    float(get_property(weapon_data, "WeaponMassKg"))
                    if weapon_data is not None
                    else 0.0
                ),
            }
        )

    REPORT["vehicle_mass_contract"] = {
        "base_vehicle_mass_kg": base_vehicle_mass_kg,
        "maximum_gross_mass_kg": maximum_gross_mass_kg,
    }
    REPORT["defense_mass_contract"] = {
        "defense_mass_kg": defense_mass_kg,
    }
    REPORT["equipment_mass_contract"] = equipment_mass_rows

    if base_vehicle_mass_kg <= 0.0:
        raise RuntimeError("Test VehicleData BaseVehicleMassKg must be greater than 0kg.")
    if maximum_gross_mass_kg < base_vehicle_mass_kg:
        raise RuntimeError("Test VehicleData MaximumGrossMassKg must be at least BaseVehicleMassKg.")
    if defense_mass_kg <= 0.0:
        raise RuntimeError("Test VehicleDefenseData DefenseMassKg must be greater than 0kg.")


# [v1.0.0] 피팅 Snapshot을 생성하고 장비·방어 해석과 질량 검증 결과를 기록합니다.
def validate_fitting_snapshot(fitting_asset: Any) -> None:
    build_snapshot = getattr(fitting_asset, "build_fitting_snapshot", None)
    if not callable(build_snapshot):
        raise RuntimeError("CFVehicleFittingData.build_fitting_snapshot is unavailable in Unreal Python.")

    snapshot = build_snapshot()
    validation_state = get_property(snapshot, "ValidationState")
    validation_issues = list(get_property(snapshot, "ValidationIssues"))
    resolved_mounts = list(get_property(snapshot, "ResolvedMounts"))
    resolved_defense_data = get_property(snapshot, "ResolvedDefenseData")
    valid_state = enum_value("CFFittingValidationState", "Valid")
    valid_with_warnings_state = enum_value("CFFittingValidationState", "ValidWithWarnings")

    issue_rows: list[dict[str, str]] = []
    for validation_issue in validation_issues:
        issue_rows.append(
            {
                "severity": str(get_property(validation_issue, "Severity")),
                "issue_code": str(get_property(validation_issue, "IssueCode")),
                "message": str(get_property(validation_issue, "Message")),
                "mount_profile_id": str(get_property(validation_issue, "MountProfileId")),
            }
        )

    REPORT["snapshot"] = {
        "validation_state": str(validation_state),
        "validation_issues": issue_rows,
        "resolved_mount_count": len(resolved_mounts),
        "resolved_equipment_preset": (
            object_path(get_property(resolved_mounts[0], "EquipmentPresetData"))
            if resolved_mounts
            else ""
        ),
        "resolved_weapon_data": (
            object_path(get_property(resolved_mounts[0], "WeaponData"))
            if resolved_mounts
            else ""
        ),
        "resolved_defense_data": object_path(resolved_defense_data),
        "base_vehicle_mass_kg": float(get_property(snapshot, "BaseVehicleMassKg")),
        "equipment_mass_kg": float(get_property(snapshot, "EquipmentMassKg")),
        "defense_mass_kg": float(get_property(snapshot, "DefenseMassKg")),
        "payload_mass_kg": float(get_property(snapshot, "PayloadMassKg")),
        "total_vehicle_mass_kg": float(get_property(snapshot, "TotalVehicleMassKg")),
        "maximum_gross_mass_kg": float(get_property(snapshot, "MaximumGrossMassKg")),
    }

    if validation_state not in (valid_state, valid_with_warnings_state):
        raise RuntimeError(
            "DA_Fit_DefenseTestSUV Snapshot validation failed: "
            f"state={validation_state}, issues={issue_rows}"
        )
    if TEST_EQUIPMENT_PRESET_PATH not in REPORT["snapshot"]["resolved_equipment_preset"]:
        raise RuntimeError(
            "Snapshot did not resolve the test-only EquipmentPresetData: "
            f"{REPORT['snapshot']['resolved_equipment_preset']}"
        )
    if TEST_WEAPON_DATA_PATH not in REPORT["snapshot"]["resolved_weapon_data"]:
        raise RuntimeError(
            "Snapshot did not resolve the test-only WeaponData: "
            f"{REPORT['snapshot']['resolved_weapon_data']}"
        )
    if TARGET_DEFENSE_DATA_PATH not in object_path(resolved_defense_data):
        raise RuntimeError(
            "Snapshot did not resolve DA_VehicleDefense_Test through UseVehicleDefault: "
            f"{object_path(resolved_defense_data)}"
        )

    REPORT["snapshot_valid"] = True


# [v1.0.0] 현재 로드된 맵에서 VehicleData와 VehicleFittingData를 가진 차량 Actor 스냅샷을 만듭니다.
def capture_map_vehicle_actors() -> list[dict[str, str]]:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    vehicle_actor_rows: list[dict[str, str]] = []
    for actor in actor_subsystem.get_all_level_actors():
        if actor is None:
            continue
        try:
            vehicle_data = get_property(actor, "VehicleData")
            vehicle_fitting_data = get_property(actor, "VehicleFittingData")
        except Exception:
            continue
        vehicle_actor_rows.append(
            {
                "actor_name": actor.get_name(),
                "actor_path": actor.get_path_name(),
                "vehicle_data": object_path(vehicle_data),
                "vehicle_fitting_data": object_path(vehicle_fitting_data),
            }
        )
    vehicle_actor_rows.sort(key=lambda row: row["actor_path"])
    return vehicle_actor_rows


# [v1.0.0] VehicleData가 방어 테스트 SUV인 맵 Actor를 정확히 한 대 찾아 반환합니다.
def find_target_map_actor() -> Any:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")

    matching_actors: list[Any] = []
    for actor in actor_subsystem.get_all_level_actors():
        if actor is None:
            continue
        try:
            vehicle_data = get_property(actor, "VehicleData")
        except Exception:
            continue
        if TARGET_VEHICLE_DATA_PATH in object_path(vehicle_data):
            matching_actors.append(actor)

    if len(matching_actors) != 1:
        raise RuntimeError(
            "Expected exactly one M_VehicleDefensePIE actor using DA_VehicleDefense_TestSUV, "
            f"found {len(matching_actors)}."
        )
    return matching_actors[0]


# [v1.0.0] 방어 테스트 맵을 로드하고 단일 방어 차량에 신규 피팅을 연결해 저장합니다.
def connect_fitting_to_map(fitting_asset: Any) -> None:
    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET_MAP_PATH)
    if loaded_world is None:
        raise RuntimeError(f"Map load failed: {TARGET_MAP_PATH}")

    REPORT["map_vehicle_actors_before"] = capture_map_vehicle_actors()
    target_actor = find_target_map_actor()
    current_fitting_data = get_property(target_actor, "VehicleFittingData")
    current_fitting_path = object_path(current_fitting_data)

    REPORT["map_actor_before"] = {
        "actor_name": target_actor.get_name(),
        "actor_path": target_actor.get_path_name(),
        "vehicle_data": object_path(get_property(target_actor, "VehicleData")),
        "vehicle_fitting_data": current_fitting_path,
    }

    if current_fitting_path and FITTING_DATA_PATH not in current_fitting_path:
        raise RuntimeError(
            "Target defense vehicle already has a different VehicleFittingData: "
            f"{current_fitting_path}"
        )

    record_change(TARGET_MAP_PATH, f"{target_actor.get_name()}.VehicleFittingData", fitting_asset)
    if not APPLY_CHANGES:
        return

    set_property(target_actor, "VehicleFittingData", fitting_asset)
    assigned_fitting_path = object_path(get_property(target_actor, "VehicleFittingData"))
    if FITTING_DATA_PATH not in assigned_fitting_path:
        raise RuntimeError(f"VehicleFittingData assignment verification failed: {assigned_fitting_path}")

    level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_editor_subsystem is None:
        raise RuntimeError("LevelEditorSubsystem is unavailable.")
    if not level_editor_subsystem.save_current_level():
        raise RuntimeError(f"Map save failed: {TARGET_MAP_PATH}")
    REPORT["saved_assets"].append(TARGET_MAP_PATH)

    reloaded_world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET_MAP_PATH)
    if reloaded_world is None:
        raise RuntimeError(f"Saved map reload failed: {TARGET_MAP_PATH}")

    reloaded_target_actor = find_target_map_actor()
    reloaded_fitting_path = object_path(get_property(reloaded_target_actor, "VehicleFittingData"))
    REPORT["map_vehicle_actors_after"] = capture_map_vehicle_actors()
    REPORT["map_actor_after"] = {
        "actor_name": reloaded_target_actor.get_name(),
        "actor_path": reloaded_target_actor.get_path_name(),
        "vehicle_data": object_path(get_property(reloaded_target_actor, "VehicleData")),
        "vehicle_fitting_data": reloaded_fitting_path,
    }

    if FITTING_DATA_PATH not in reloaded_fitting_path:
        raise RuntimeError(f"Saved map VehicleFittingData verification failed: {reloaded_fitting_path}")

    before_non_target = [
        row
        for row in REPORT["map_vehicle_actors_before"]
        if TARGET_VEHICLE_DATA_PATH not in row["vehicle_data"]
    ]
    after_non_target = [
        row
        for row in REPORT["map_vehicle_actors_after"]
        if TARGET_VEHICLE_DATA_PATH not in row["vehicle_data"]
    ]
    if before_non_target != after_non_target:
        raise RuntimeError("A non-target vehicle actor changed while connecting the defense test fitting.")

    REPORT["map_connected"] = True


# [v1.1.0] 화이트리스트된 테스트 에셋 하나를 명시적으로 저장합니다.
def save_mutable_asset(asset_path: str, asset: Any) -> None:
    if not APPLY_CHANGES:
        return
    if asset_path not in MUTABLE_ASSET_PATHS or asset_path == TARGET_MAP_PATH:
        raise RuntimeError(f"Save path is not an allowed DataAsset path: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.1.0] 보호 에셋 해시가 실행 전과 동일한지 검증합니다.
def verify_protected_assets_unchanged() -> None:
    REPORT["protected_hashes_after"] = capture_hashes(PROTECTED_ASSET_PATHS)
    if REPORT["protected_hashes_before"] != REPORT["protected_hashes_after"]:
        raise RuntimeError("One or more protected Vehicle, Blueprint or shared Launcher assets changed unexpectedly.")
    REPORT["protected_contract_passed"] = True


# [v1.1.0] 방어 테스트 피팅과 테스트 전용 장비를 생성·검증하고 전용 맵의 방어 차량에만 연결합니다.
def apply_defense_test_fitting() -> None:
    REPORT["protected_hashes_before"] = capture_hashes(PROTECTED_ASSET_PATHS)
    REPORT["mutable_hashes_before"] = capture_hashes(MUTABLE_ASSET_PATHS)

    vehicle_data = require_asset(TARGET_VEHICLE_DATA_PATH)
    defense_data = require_asset(TARGET_DEFENSE_DATA_PATH)
    source_weapon_data = require_asset(SOURCE_WEAPON_DATA_PATH)
    source_equipment_preset = require_asset(SOURCE_EQUIPMENT_PRESET_PATH)
    source_turret_mount = require_asset(SOURCE_TURRET_MOUNT_PATH)

    vehicle_default_defense = get_property(vehicle_data, "DefaultDefenseData")
    if TARGET_DEFENSE_DATA_PATH not in object_path(vehicle_default_defense):
        raise RuntimeError(
            "DA_VehicleDefense_TestSUV.DefaultDefenseData does not reference DA_VehicleDefense_Test: "
            f"{object_path(vehicle_default_defense)}"
        )

    if object_path(get_property(source_equipment_preset, "DefaultWeaponData")) != object_path(source_weapon_data):
        raise RuntimeError("RocketLauncher preset no longer references DA_RocketLauncher as expected.")
    if object_path(get_property(source_equipment_preset, "DefaultTurretMountData")) != object_path(source_turret_mount):
        raise RuntimeError("RocketLauncher preset no longer references DA_RocketBody as expected.")

    test_weapon_data = duplicate_or_load_asset(SOURCE_WEAPON_DATA_PATH, TEST_WEAPON_DATA_PATH)
    test_equipment_preset = duplicate_or_load_asset(
        SOURCE_EQUIPMENT_PRESET_PATH,
        TEST_EQUIPMENT_PRESET_PATH,
    )
    fitting_asset = load_or_create_fitting_asset()

    if not APPLY_CHANGES:
        record_change(TARGET_VEHICLE_DATA_PATH, "BaseVehicleMassKg", TEST_BASE_VEHICLE_MASS_KG)
        record_change(TARGET_VEHICLE_DATA_PATH, "MaximumGrossMassKg", TEST_MAXIMUM_GROSS_MASS_KG)
        record_change(TARGET_DEFENSE_DATA_PATH, "DefenseMassKg", TEST_DEFENSE_MASS_KG)
        return

    if test_weapon_data is None or test_equipment_preset is None or fitting_asset is None:
        raise RuntimeError("Apply mode failed to create or load one or more test fitting assets.")

    configure_test_assets(
        vehicle_data,
        defense_data,
        test_weapon_data,
        test_equipment_preset,
        source_turret_mount,
    )
    configure_fitting_asset(fitting_asset, vehicle_data, test_equipment_preset)
    validate_mass_contracts(vehicle_data, defense_data)
    validate_fitting_snapshot(fitting_asset)

    save_mutable_asset(TARGET_VEHICLE_DATA_PATH, vehicle_data)
    save_mutable_asset(TARGET_DEFENSE_DATA_PATH, defense_data)
    save_mutable_asset(TEST_WEAPON_DATA_PATH, test_weapon_data)
    save_mutable_asset(TEST_EQUIPMENT_PRESET_PATH, test_equipment_preset)
    save_mutable_asset(FITTING_DATA_PATH, fitting_asset)

    reloaded_fitting_asset = require_asset(FITTING_DATA_PATH)
    validate_fitting_snapshot(reloaded_fitting_asset)
    connect_fitting_to_map(reloaded_fitting_asset)
    verify_protected_assets_unchanged()
    REPORT["mutable_hashes_after"] = capture_hashes(MUTABLE_ASSET_PATHS)


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_defense_test_fitting()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][DefenseTestFitting] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Defense test fitting application failed. Read report.json for details.")
