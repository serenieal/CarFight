# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-08-13
# Description: CF-FQ-031 AMMO-P0-08 Production Runtime 검증용 finite-ammo 격리 에셋과 PIE 준비 맵을 생성합니다.
# Scope: Heavy Cannon·Ripple의 AmmoData, TurretMount, WeaponData, EquipmentPreset, VehicleData, VehicleFittingData와 TestMap 복제본만 생성·수정합니다.
# Changelog:
# - v1.0.0: Heavy 5|15, Ripple 3|7 finite-ammo 저장 체인, Fitting Snapshot 검증, TestMap_AmmoHeavy·TestMap_AmmoRipple Player0 연결과 원본 SHA-256 보호를 추가.
# Migration:
# - 공유 DA_TestSedan, DA_ProtoTurretCannon, HeavyCannon, DA_CannonBody와 기존 LauncherRegression 원본은 읽기 전용입니다.
# - MaximumLoadableAmmoCount는 피팅 상한으로만 사용하며 현재 탄약 수량을 생성하지 않습니다.
# - -Apply에 해당하는 환경 변수가 없으면 에셋과 맵을 생성·저장하지 않습니다.

from __future__ import annotations

import hashlib
import json
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.0.0] 실행 결과에서 식별할 현재 도구 버전입니다.
TOOL_VERSION = "1.0.0"

# [v1.0.0] 실제 격리 에셋과 맵 저장을 허용하는 명시적 Apply 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_AMMO_INTEGRATION_APPLY", "0") == "1"

# [v1.0.0] 현재 CarFight Unreal 프로젝트 루트입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 구조화 실행 보고서를 저장할 Saved 하위 폴더입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "AmmoIntegrationApply"

# [v1.0.0] Admin process.status가 읽을 구조화 실행 보고서입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] P0-08 격리 DataAsset을 저장할 Content 폴더입니다.
TEST_DATA_FOLDER = "/Game/CarFight/Tests/AmmoIntegration"

# [v1.0.0] Heavy Cannon 검증용 AmmoData 경로입니다.
HEAVY_AMMO_PATH = f"{TEST_DATA_FOLDER}/DA_Ammo_HeavyFinite"

# [v1.0.0] Heavy Cannon 검증용 TurretMountData 경로입니다.
HEAVY_MOUNT_PATH = f"{TEST_DATA_FOLDER}/DA_Mount_HeavyFinite"

# [v1.0.0] Heavy Cannon 검증용 finite WeaponData 경로입니다.
HEAVY_WEAPON_PATH = f"{TEST_DATA_FOLDER}/DA_Wpn_HeavyFinite"

# [v1.0.0] Heavy Cannon 검증용 EquipmentPresetData 경로입니다.
HEAVY_PRESET_PATH = f"{TEST_DATA_FOLDER}/Preset_HeavyFinite"

# [v1.0.0] Heavy Cannon 검증용 VehicleData 경로입니다.
HEAVY_VEHICLE_PATH = f"{TEST_DATA_FOLDER}/DA_Veh_HeavyFinite"

# [v1.0.0] Heavy Cannon 검증용 VehicleFittingData 경로입니다.
HEAVY_FITTING_PATH = f"{TEST_DATA_FOLDER}/DA_Fit_HeavyFinite"

# [v1.0.0] Ripple 검증용 AmmoData 경로입니다.
RIPPLE_AMMO_PATH = f"{TEST_DATA_FOLDER}/DA_Ammo_RocketFinite"

# [v1.0.0] Ripple 검증용 TurretMountData 경로입니다.
RIPPLE_MOUNT_PATH = f"{TEST_DATA_FOLDER}/DA_Mount_RippleFinite"

# [v1.0.0] Ripple 검증용 finite WeaponData 경로입니다.
RIPPLE_WEAPON_PATH = f"{TEST_DATA_FOLDER}/DA_Wpn_RippleFinite"

# [v1.0.0] Ripple 검증용 EquipmentPresetData 경로입니다.
RIPPLE_PRESET_PATH = f"{TEST_DATA_FOLDER}/Preset_RippleFinite"

# [v1.0.0] Ripple 검증용 VehicleData 경로입니다.
RIPPLE_VEHICLE_PATH = f"{TEST_DATA_FOLDER}/DA_Veh_RippleFinite"

# [v1.0.0] Ripple 검증용 VehicleFittingData 경로입니다.
RIPPLE_FITTING_PATH = f"{TEST_DATA_FOLDER}/DA_Fit_RippleFinite"

# [v1.0.0] Heavy Cannon 사용자 PIE 준비 맵 경로입니다.
HEAVY_MAP_PATH = "/Game/Maps/TestMap_AmmoHeavy"

# [v1.0.0] Ripple 사용자 PIE 준비 맵 경로입니다.
RIPPLE_MAP_PATH = "/Game/Maps/TestMap_AmmoRipple"

# [v1.0.0] 두 테스트 맵의 읽기 전용 복제 원본입니다.
SOURCE_MAP_PATH = "/Game/Maps/TestMap"

# [v1.0.0] TestMap의 기존 Launcher 검증 슬롯 Actor 이름입니다.
SOURCE_PLAYER_SLOT_ACTOR_NAME = "BP_CFVehiclePawn_C_1"

# [v1.0.0] Heavy Cannon 원본 VehicleData 경로입니다.
SOURCE_HEAVY_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan"

# [v1.0.0] Heavy Cannon 원본 WeaponData 경로입니다.
SOURCE_HEAVY_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon"

# [v1.0.0] Heavy Cannon 원본 EquipmentPresetData 경로입니다.
SOURCE_HEAVY_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon"

# [v1.0.0] Heavy Cannon 원본 TurretMountData 경로입니다.
SOURCE_HEAVY_MOUNT_PATH = "/Game/CarFight/Weapons/Data/TurretMounts/DA_CannonBody"

# [v1.0.0] 기존 DR-PIE-06에서 검증된 Ripple VehicleData 원본입니다.
SOURCE_RIPPLE_VEHICLE_PATH = "/Game/CarFight/Tests/LauncherRegression/DA_TestSUV_DRRipple"

# [v1.0.0] 기존 DR-PIE-06에서 검증된 Ripple WeaponData 원본입니다.
SOURCE_RIPPLE_WEAPON_PATH = "/Game/CarFight/Tests/LauncherRegression/DA_RocketLauncher_DRRipple"

# [v1.0.0] 기존 DR-PIE-06에서 검증된 Ripple EquipmentPresetData 원본입니다.
SOURCE_RIPPLE_PRESET_PATH = "/Game/CarFight/Tests/LauncherRegression/RocketLauncher_DRRipple"

# [v1.0.0] Ripple 원본 TurretMountData 경로입니다.
SOURCE_RIPPLE_MOUNT_PATH = "/Game/CarFight/Weapons/Data/TurretMounts/DA_RocketBody"

# [v1.0.0] P0 Fitting 검증에 사용할 격리 차량 플랫폼 질량입니다.
TEST_BASE_VEHICLE_MASS_KG = 1000.0

# [v1.0.0] P0 Fitting 검증에 사용할 격리 차량 최대 총중량입니다.
TEST_MAXIMUM_GROSS_MASS_KG = 2500.0

# [v1.0.0] 두 검증 터렛 마운트에 명시할 질량입니다.
TEST_TURRET_MOUNT_MASS_KG = 350.0

# [v1.0.0] 두 검증 WeaponData에 명시할 무기 본체 질량입니다.
TEST_WEAPON_MASS_KG = 120.0

# [v1.0.0] Heavy 탄약 한 발의 피팅 질량입니다.
HEAVY_AMMO_UNIT_MASS_KG = 2.0

# [v1.0.0] Heavy 피팅에서 선택 가능한 최대 탄약량입니다.
HEAVY_MAX_LOADABLE_AMMO_COUNT = 30

# [v1.0.0] Heavy 출격 시작 탄창 장전량입니다.
HEAVY_INITIAL_LOADED_AMMO_COUNT = 5

# [v1.0.0] Heavy 출격 장전+예비 전체 탄약량입니다.
HEAVY_INITIAL_SORTIE_AMMO_COUNT = 15

# [v1.0.0] Heavy FullMagazine Reload 시간입니다.
HEAVY_RELOAD_SECONDS = 2.0

# [v1.0.0] Rocket 한 발의 피팅 질량입니다.
RIPPLE_AMMO_UNIT_MASS_KG = 5.0

# [v1.0.0] Ripple 피팅에서 선택 가능한 최대 탄약량입니다.
RIPPLE_MAX_LOADABLE_AMMO_COUNT = 12

# [v1.0.0] Ripple 출격 시작 탄창 장전량입니다. 4발 요청보다 작아 부분 Sequence를 검증합니다.
RIPPLE_INITIAL_LOADED_AMMO_COUNT = 3

# [v1.0.0] Ripple 출격 장전+예비 전체 탄약량입니다.
RIPPLE_INITIAL_SORTIE_AMMO_COUNT = 7

# [v1.0.0] Ripple FullMagazine Reload 시간입니다.
RIPPLE_RELOAD_SECONDS = 2.0

# [v1.0.0] 이번 도구가 생성 또는 수정할 수 있는 유일한 에셋과 맵 경로입니다.
MUTABLE_ASSET_PATHS = {
    HEAVY_AMMO_PATH,
    HEAVY_MOUNT_PATH,
    HEAVY_WEAPON_PATH,
    HEAVY_PRESET_PATH,
    HEAVY_VEHICLE_PATH,
    HEAVY_FITTING_PATH,
    RIPPLE_AMMO_PATH,
    RIPPLE_MOUNT_PATH,
    RIPPLE_WEAPON_PATH,
    RIPPLE_PRESET_PATH,
    RIPPLE_VEHICLE_PATH,
    RIPPLE_FITTING_PATH,
    HEAVY_MAP_PATH,
    RIPPLE_MAP_PATH,
}

# [v1.0.0] 실행 전후 SHA-256이 반드시 동일해야 하는 공유 원본 경로입니다.
PROTECTED_ASSET_PATHS = {
    SOURCE_MAP_PATH,
    SOURCE_HEAVY_VEHICLE_PATH,
    SOURCE_HEAVY_WEAPON_PATH,
    SOURCE_HEAVY_PRESET_PATH,
    SOURCE_HEAVY_MOUNT_PATH,
    SOURCE_RIPPLE_VEHICLE_PATH,
    SOURCE_RIPPLE_WEAPON_PATH,
    SOURCE_RIPPLE_PRESET_PATH,
    SOURCE_RIPPLE_MOUNT_PATH,
        "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn",
    "/Game/CarFight/UI/HUD/WBP_CFInGameHUD",
}

# [v1.0.0] 생성·연결·검증 결과를 후속 Build/Automation과 세션에 전달할 구조화 보고서입니다.
REPORT: dict[str, Any] = {
    "schema_version": "ammo_integration_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "planned_changes": [],
    "protected_hashes_before": {},
    "protected_hashes_after": {},
    "protected_contract_passed": False,
    "heavy_snapshot": {},
    "ripple_snapshot": {},
    "heavy_chain_valid": False,
    "ripple_chain_valid": False,
    "heavy_map_connected": False,
    "ripple_map_connected": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 P0-08 에셋 준비 메시지를 기록합니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][AmmoIntegration] {message}")


# [v1.0.0] C++ PascalCase 이름을 Unreal Python snake_case 이름으로 변환합니다.
def pascal_to_snake(name: str) -> str:
    step1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", step1).lower()


# [v1.0.0] Reflection 접근에 사용할 원본 이름과 snake_case 후보를 반환합니다.
def candidate_property_names(name: str) -> list[str]:
    return list(dict.fromkeys([name, pascal_to_snake(name)]))


# [v1.0.0] Unreal Reflection 프로퍼티를 원본 또는 snake_case 이름으로 읽습니다.
def get_property(target: Any, property_name: str) -> Any:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            return target.get_editor_property(candidate)
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property read failed: {type(target).__name__}.{property_name}: {last_error}")


# [v1.0.0] Unreal Reflection 프로퍼티를 원본 또는 snake_case 이름으로 기록합니다.
def set_property(target: Any, property_name: str, value: Any) -> None:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            target.set_editor_property(candidate, value)
            return
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property write failed: {type(target).__name__}.{property_name}: {last_error}")


# [v1.0.0] Unreal enum 타입과 후보 멤버 이름을 실제 Reflection 값으로 변환합니다.
def enum_value(enum_type_name: str, *member_names: str) -> Any:
    enum_type = getattr(unreal, enum_type_name, None)
    if enum_type is None:
        raise RuntimeError(f"Unreal enum type was not found: {enum_type_name}")
    for member_name in member_names:
        for candidate in [member_name, member_name.upper(), pascal_to_snake(member_name).upper()]:
            if hasattr(enum_type, candidate):
                return getattr(enum_type, candidate)
    raise RuntimeError(f"Enum member was not found: {enum_type_name}.{member_names}")


# [v1.0.0] Unreal 객체 전체 Object Path를 안정적인 문자열로 반환합니다.
def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        return unreal.SystemLibrary.get_path_name(value)
    except Exception:
        return str(value)


# [v1.0.0] /Game 에셋 경로를 실제 Content 파일 경로로 변환합니다.
def asset_file_path(asset_path: str) -> Path:
    if not asset_path.startswith("/Game/"):
        raise RuntimeError(f"Only /Game paths are supported: {asset_path}")
    extension = ".umap" if asset_path.startswith("/Game/Maps/") else ".uasset"
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'):]}{extension}"


# [v1.0.0] 존재하는 파일의 SHA-256을 계산하고 누락 파일은 빈 문자열로 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
    digest = hashlib.sha256()
    with file_path.open("rb") as source_file:
        for chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


# [v1.0.0] 지정된 Unreal 에셋 경로들의 실제 파일 SHA-256을 반환합니다.
def capture_hashes(asset_paths: set[str]) -> dict[str, str]:
    return {asset_path: sha256_file(asset_file_path(asset_path)) for asset_path in sorted(asset_paths)}


# [v1.0.0] 필수 Unreal 에셋을 로드하고 없으면 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return asset


# [v1.0.0] 변경 허용 경로인지 확인하고 Dry Run 계획을 보고서에 기록합니다.
def record_change(asset_path: str, property_name: str, value: Any) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in mutation whitelist: {asset_path}")
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.0.0] P0-08 DataAsset 폴더가 존재하도록 생성합니다.
def ensure_test_data_folder() -> None:
    if unreal.EditorAssetLibrary.does_directory_exist(TEST_DATA_FOLDER):
        return
    if not unreal.EditorAssetLibrary.make_directory(TEST_DATA_FOLDER):
        raise RuntimeError(f"Asset folder creation failed: {TEST_DATA_FOLDER}")


# [v1.0.0] 읽기 전용 원본을 P0-08 허용 경로에 복제하거나 기존 격리 에셋을 로드합니다.
def duplicate_or_load_asset(source_path: str, target_path: str) -> Any | None:
    if target_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Target path is not mutable: {target_path}")
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        REPORT["updated_assets"].append(target_path)
        return require_asset(target_path)
    if not APPLY_CHANGES:
        record_change(target_path, "DuplicateAsset", source_path)
        return None
    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"Asset duplication failed: {source_path} -> {target_path}")
    REPORT["created_assets"].append(target_path)
    return duplicated_asset


# [v1.0.0] 지정 DataAsset 클래스로 신규 격리 DataAsset을 생성하거나 기존 에셋을 로드합니다.
def create_or_load_data_asset(target_path: str, unreal_class_name: str) -> Any | None:
    if target_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Target path is not mutable: {target_path}")
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        REPORT["updated_assets"].append(target_path)
        return require_asset(target_path)
    if not APPLY_CHANGES:
        record_change(target_path, "CreateAsset", unreal_class_name)
        return None
    data_asset_class = getattr(unreal, unreal_class_name, None)
    if data_asset_class is None:
        raise RuntimeError(f"Unreal DataAsset class was not found: {unreal_class_name}")
    package_path, asset_name = target_path.rsplit("/", 1)
    data_asset_factory = unreal.DataAssetFactory()
    set_property(data_asset_factory, "DataAssetClass", data_asset_class)
    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        package_path,
        data_asset_class,
        data_asset_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"DataAsset creation failed: {target_path}")
    REPORT["created_assets"].append(target_path)
    return created_asset


# [v1.0.0] 수정된 격리 DataAsset을 디스크에 저장합니다.
def save_asset(asset_path: str, asset: Any) -> None:
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.0.0] AmmoData에 실제 출격 탄종 ID, 표시 이름, 질량과 피팅 상한을 설정합니다.
def configure_ammo_data(
    asset_path: str,
    ammo_data: Any,
    ammo_id: str,
    display_name: str,
    family_id: str,
    unit_mass_kg: float,
    maximum_loadable_count: int,
) -> None:
    record_change(asset_path, "AmmoId", ammo_id)
    record_change(asset_path, "AmmoDisplayName", display_name)
    record_change(asset_path, "AmmoFamilyId", family_id)
    record_change(asset_path, "UnitMassKg", unit_mass_kg)
    record_change(asset_path, "MaximumLoadableAmmoCount", maximum_loadable_count)
    if not APPLY_CHANGES:
        return
    set_property(ammo_data, "AmmoId", ammo_id)
    set_property(ammo_data, "AmmoDisplayName", display_name)
    set_property(ammo_data, "AmmoFamilyId", family_id)
    set_property(ammo_data, "UnitMassKg", unit_mass_kg)
    set_property(ammo_data, "MaximumLoadableAmmoCount", maximum_loadable_count)
    set_property(ammo_data, "bCanBeResupplied", True)


# [v1.0.0] 복제 TurretMountData에 P0 Fitting이 요구하는 명시적 양수 질량을 설정합니다.
def configure_mount_data(asset_path: str, mount_data: Any, mount_id: str) -> None:
    record_change(asset_path, "TurretMountId", mount_id)
    record_change(asset_path, "TurretMountWeightKg", TEST_TURRET_MOUNT_MASS_KG)
    if not APPLY_CHANGES:
        return
    set_property(mount_data, "TurretMountId", mount_id)
    set_property(mount_data, "TurretMountWeightKg", TEST_TURRET_MOUNT_MASS_KG)


# [v1.0.0] 복제 WeaponData를 기존 발사 설정 보존 상태에서 finite Ammo Runtime으로 전환합니다.
def configure_weapon_data(
    asset_path: str,
    weapon_data: Any,
    weapon_id: str,
    ammo_data: Any,
    magazine_size: int,
    initial_loaded_count: int,
    reload_seconds: float,
) -> None:
    record_change(asset_path, "WeaponId", weapon_id)
    record_change(asset_path, "WeaponMassKg", TEST_WEAPON_MASS_KG)
    record_change(asset_path, "DefaultAmmoData", ammo_data)
    record_change(asset_path, "MagazineSize", magazine_size)
    record_change(asset_path, "InitialLoadedAmmoCount", initial_loaded_count)
    record_change(asset_path, "ReloadTimeSeconds", reload_seconds)
    record_change(asset_path, "bUseInfiniteAmmoForDebug", False)
    if not APPLY_CHANGES:
        return
    set_property(weapon_data, "WeaponId", weapon_id)
    set_property(weapon_data, "WeaponMassKg", TEST_WEAPON_MASS_KG)
    set_property(weapon_data, "DefaultAmmoData", ammo_data)
    set_property(weapon_data, "MagazineSize", magazine_size)
    set_property(weapon_data, "InitialLoadedAmmoCount", initial_loaded_count)
    set_property(weapon_data, "AmmoUnitsPerShot", 1)
    set_property(weapon_data, "ReloadTimeSeconds", reload_seconds)
    set_property(weapon_data, "ReloadMode", enum_value("CFWeaponReloadMode", "FullMagazine"))
    set_property(weapon_data, "bAutoReloadWhenEmpty", True)
    set_property(weapon_data, "bAllowPartialReload", True)
    set_property(weapon_data, "bAllowPartialSequence", True)
    set_property(weapon_data, "bUseInfiniteAmmoForDebug", False)


# [v1.0.0] 복제 EquipmentPresetData가 격리 Mount와 finite Weapon을 참조하도록 연결합니다.
def configure_equipment_preset(
    asset_path: str,
    preset_data: Any,
    equipment_id: str,
    display_name: str,
    mount_data: Any,
    weapon_data: Any,
) -> None:
    record_change(asset_path, "EquipmentId", equipment_id)
    record_change(asset_path, "DisplayName", display_name)
    record_change(asset_path, "DefaultTurretMountData", mount_data)
    record_change(asset_path, "DefaultWeaponData", weapon_data)
    if not APPLY_CHANGES:
        return
    set_property(preset_data, "EquipmentId", equipment_id)
    set_property(preset_data, "DisplayName", display_name)
    set_property(preset_data, "DefaultTurretMountData", mount_data)
    set_property(preset_data, "DefaultWeaponData", weapon_data)


# [v1.0.0] 복제 VehicleData에 피팅 질량 기준과 격리 finite EquipmentPreset 기본 참조를 설정합니다.
def configure_vehicle_data(asset_path: str, vehicle_data: Any, preset_data: Any) -> None:
    record_change(asset_path, "BaseVehicleMassKg", TEST_BASE_VEHICLE_MASS_KG)
    record_change(asset_path, "MaximumGrossMassKg", TEST_MAXIMUM_GROSS_MASS_KG)
    record_change(asset_path, "MountProfiles[0].DefaultEquipmentPresetData", preset_data)
    if not APPLY_CHANGES:
        return
    set_property(vehicle_data, "BaseVehicleMassKg", TEST_BASE_VEHICLE_MASS_KG)
    set_property(vehicle_data, "MaximumGrossMassKg", TEST_MAXIMUM_GROSS_MASS_KG)
    mount_profiles = list(get_property(vehicle_data, "MountProfiles"))
    if len(mount_profiles) != 1:
        raise RuntimeError(f"P0-08 VehicleData must contain exactly one MountProfile: {asset_path}, count={len(mount_profiles)}")
    set_property(mount_profiles[0], "DefaultEquipmentPresetData", preset_data)
    set_property(vehicle_data, "MountProfiles", mount_profiles)


# [v1.0.0] VehicleFittingData에 명시적 Mount 선택과 실제 출격 탄약 수량을 설정합니다.
def configure_fitting_data(
    asset_path: str,
    fitting_data: Any,
    fitting_id: str,
    display_name: str,
    vehicle_data: Any,
    preset_data: Any,
    ammo_data: Any,
    initial_sortie_count: int,
) -> None:
    if not APPLY_CHANGES:
        record_change(asset_path, "FittingId", fitting_id)
        record_change(asset_path, "VehicleData", vehicle_data)
        record_change(asset_path, "InitialSortieAmmoCount", initial_sortie_count)
        return
    mount_profiles = list(get_property(vehicle_data, "MountProfiles"))
    if len(mount_profiles) != 1:
        raise RuntimeError(f"P0-08 Fitting requires exactly one MountProfile: {asset_path}")
    mount_profile_id = get_property(mount_profiles[0], "MountProfileId")
    mount_selection_class = getattr(unreal, "CFVehicleMountSelection", None)
    ammo_load_class = getattr(unreal, "CFAmmoSortieLoad", None)
    if mount_selection_class is None or ammo_load_class is None:
        raise RuntimeError("Required Fitting/Ammo USTRUCT is unavailable in Unreal Python.")
    mount_selection = mount_selection_class()
    set_property(mount_selection, "MountProfileId", mount_profile_id)
    set_property(mount_selection, "EquipmentPresetData", preset_data)
    set_property(mount_selection, "bEnabled", True)
    ammo_load = ammo_load_class()
    set_property(ammo_load, "AmmoData", ammo_data)
    set_property(ammo_load, "InitialSortieAmmoCount", initial_sortie_count)
    defense_selection = get_property(fitting_data, "DefenseSelection")
    set_property(defense_selection, "SelectionMode", enum_value("CFDefenseSelectionMode", "ExplicitNone"))
    set_property(defense_selection, "DefenseData", None)
    set_property(fitting_data, "FittingId", fitting_id)
    set_property(fitting_data, "DisplayName", display_name)
    set_property(fitting_data, "VehicleData", vehicle_data)
    set_property(fitting_data, "MountSelections", [mount_selection])
    set_property(fitting_data, "MissingMountSelectionPolicy", enum_value("CFMissingMountPolicy", "UseVehicleDefault"))
    set_property(fitting_data, "InitialSortieAmmoLoads", [ammo_load])
    set_property(fitting_data, "DefenseSelection", defense_selection)


# [v1.0.0] 저장 FittingData가 finite Weapon·Ammo·질량 계약을 정확히 해석하는지 검증합니다.
def validate_fitting_snapshot(
    fitting_path: str,
    weapon_path: str,
    ammo_path: str,
    expected_sortie_count: int,
    expected_ammo_mass_kg: float,
) -> dict[str, Any]:
    fitting_data = require_asset(fitting_path)
    build_snapshot = getattr(fitting_data, "build_fitting_snapshot", None)
    if not callable(build_snapshot):
        raise RuntimeError("CFVehicleFittingData.build_fitting_snapshot is unavailable in Unreal Python.")
    snapshot = build_snapshot()
    validation_state = get_property(snapshot, "ValidationState")
    valid_state = enum_value("CFFittingValidationState", "Valid")
    valid_with_warnings_state = enum_value("CFFittingValidationState", "ValidWithWarnings")
    resolved_mounts = list(get_property(snapshot, "ResolvedMounts"))
    sortie_loads = list(get_property(snapshot, "InitialSortieAmmoLoads"))
    validation_issues = list(get_property(snapshot, "ValidationIssues"))
    issue_rows = [
        {
            "severity": str(get_property(issue, "Severity")),
            "issue_code": str(get_property(issue, "IssueCode")),
            "message": str(get_property(issue, "Message")),
        }
        for issue in validation_issues
    ]
    if validation_state not in (valid_state, valid_with_warnings_state):
        raise RuntimeError(f"Fitting Snapshot invalid: {fitting_path}, state={validation_state}, issues={issue_rows}")
    if len(resolved_mounts) != 1 or len(sortie_loads) != 1:
        raise RuntimeError(f"Fitting Snapshot must resolve one mount and one ammo load: {fitting_path}")
    resolved_weapon_path = object_path(get_property(resolved_mounts[0], "WeaponData"))
    resolved_ammo_path = object_path(get_property(sortie_loads[0], "AmmoData"))
    sortie_count = int(get_property(sortie_loads[0], "InitialSortieAmmoCount"))
    ammo_mass_kg = float(get_property(snapshot, "AmmoMassKg"))
    total_vehicle_mass_kg = float(get_property(snapshot, "TotalVehicleMassKg"))
    if weapon_path not in resolved_weapon_path:
        raise RuntimeError(f"Fitting did not resolve finite WeaponData: {fitting_path} -> {resolved_weapon_path}")
    if ammo_path not in resolved_ammo_path:
        raise RuntimeError(f"Fitting did not resolve AmmoData: {fitting_path} -> {resolved_ammo_path}")
    if sortie_count != expected_sortie_count:
        raise RuntimeError(f"Fitting sortie count mismatch: expected={expected_sortie_count}, actual={sortie_count}")
    if abs(ammo_mass_kg - expected_ammo_mass_kg) > 0.001:
        raise RuntimeError(f"Fitting AmmoMassKg mismatch: expected={expected_ammo_mass_kg}, actual={ammo_mass_kg}")
    if total_vehicle_mass_kg <= 0.0 or total_vehicle_mass_kg > TEST_MAXIMUM_GROSS_MASS_KG:
        raise RuntimeError(f"Fitting TotalVehicleMassKg invalid: {total_vehicle_mass_kg}")
    return {
        "validation_state": str(validation_state),
        "issues": issue_rows,
        "resolved_weapon_data": resolved_weapon_path,
        "resolved_ammo_data": resolved_ammo_path,
        "initial_sortie_ammo_count": sortie_count,
        "ammo_mass_kg": ammo_mass_kg,
        "equipment_mass_kg": float(get_property(snapshot, "EquipmentMassKg")),
        "total_vehicle_mass_kg": total_vehicle_mass_kg,
    }


# [v1.0.0] 현재 로드된 TestMap에서 기존 Launcher 검증 슬롯 Actor를 정확히 한 대 반환합니다.
def find_player_slot_actor() -> Any:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")
    matching_actors = [
        actor
        for actor in actor_subsystem.get_all_level_actors()
        if actor is not None and actor.get_name() == SOURCE_PLAYER_SLOT_ACTOR_NAME
    ]
    if len(matching_actors) != 1:
        raise RuntimeError(f"Expected one {SOURCE_PLAYER_SLOT_ACTOR_NAME}, found {len(matching_actors)}")
    return matching_actors[0]


# [v1.0.0] TestMap 복제본의 검증 슬롯을 격리 VehicleData+FittingData와 Player0으로 연결합니다.
def configure_test_map(map_path: str, vehicle_data: Any, fitting_data: Any) -> dict[str, str]:
    if map_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Map path is not mutable: {map_path}")
    if unreal.EditorAssetLibrary.does_asset_exist(map_path):
        REPORT["updated_assets"].append(map_path)
        map_to_load = map_path
    elif not APPLY_CHANGES:
        record_change(map_path, "DuplicateAsset", SOURCE_MAP_PATH)
        return {}
    else:
        duplicated_map = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP_PATH, map_path)
        if duplicated_map is None:
            raise RuntimeError(f"Map duplication failed: {SOURCE_MAP_PATH} -> {map_path}")
        REPORT["created_assets"].append(map_path)
        map_to_load = map_path
    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(map_to_load)
    if loaded_world is None:
        raise RuntimeError(f"Map load failed: {map_to_load}")
    player_slot_actor = find_player_slot_actor()
    player_zero_value = enum_value("AutoReceiveInput", "PLAYER0", "Player0")
    disabled_value = enum_value("AutoReceiveInput", "DISABLED", "Disabled")
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actor_subsystem is None:
        raise RuntimeError("EditorActorSubsystem is unavailable.")
    for actor in actor_subsystem.get_all_level_actors():
        if actor is None:
            continue
        try:
            get_property(actor, "VehicleData")
            get_property(actor, "AutoPossessPlayer")
        except Exception:
            continue
        set_property(actor, "AutoPossessPlayer", disabled_value)
    set_property(player_slot_actor, "VehicleData", vehicle_data)
    set_property(player_slot_actor, "VehicleFittingData", fitting_data)
    set_property(player_slot_actor, "AutoPossessPlayer", player_zero_value)
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_subsystem is None or not level_subsystem.save_current_level():
        raise RuntimeError(f"Map save failed: {map_path}")
    REPORT["saved_assets"].append(map_path)
    reloaded_world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    if reloaded_world is None:
        raise RuntimeError(f"Saved map reload failed: {map_path}")
    reloaded_actor = find_player_slot_actor()
    vehicle_path = object_path(get_property(reloaded_actor, "VehicleData"))
    fitting_path = object_path(get_property(reloaded_actor, "VehicleFittingData"))
    auto_possess = str(get_property(reloaded_actor, "AutoPossessPlayer"))
    if object_path(vehicle_data) not in vehicle_path or object_path(fitting_data) not in fitting_path:
        raise RuntimeError(f"Saved map Vehicle/Fitting connection mismatch: {map_path}")
    if "Player0" not in auto_possess and "PLAYER0" not in auto_possess:
        raise RuntimeError(f"Saved map Player0 connection mismatch: {map_path}, {auto_possess}")
    return {
        "actor": reloaded_actor.get_path_name(),
        "vehicle_data": vehicle_path,
        "vehicle_fitting_data": fitting_path,
        "auto_possess_player": auto_possess,
    }


# [v1.0.0] P0-08 Heavy/Ripple 격리 에셋 체인을 생성·설정·저장하고 Snapshot을 검증합니다.
def apply_asset_chains() -> None:
    ensure_test_data_folder()
    heavy_mount = duplicate_or_load_asset(SOURCE_HEAVY_MOUNT_PATH, HEAVY_MOUNT_PATH)
    heavy_weapon = duplicate_or_load_asset(SOURCE_HEAVY_WEAPON_PATH, HEAVY_WEAPON_PATH)
    heavy_preset = duplicate_or_load_asset(SOURCE_HEAVY_PRESET_PATH, HEAVY_PRESET_PATH)
    heavy_vehicle = duplicate_or_load_asset(SOURCE_HEAVY_VEHICLE_PATH, HEAVY_VEHICLE_PATH)
    ripple_mount = duplicate_or_load_asset(SOURCE_RIPPLE_MOUNT_PATH, RIPPLE_MOUNT_PATH)
    ripple_weapon = duplicate_or_load_asset(SOURCE_RIPPLE_WEAPON_PATH, RIPPLE_WEAPON_PATH)
    ripple_preset = duplicate_or_load_asset(SOURCE_RIPPLE_PRESET_PATH, RIPPLE_PRESET_PATH)
    ripple_vehicle = duplicate_or_load_asset(SOURCE_RIPPLE_VEHICLE_PATH, RIPPLE_VEHICLE_PATH)
    heavy_ammo = create_or_load_data_asset(HEAVY_AMMO_PATH, "CFAmmoData")
    heavy_fitting = create_or_load_data_asset(HEAVY_FITTING_PATH, "CFVehicleFittingData")
    ripple_ammo = create_or_load_data_asset(RIPPLE_AMMO_PATH, "CFAmmoData")
    ripple_fitting = create_or_load_data_asset(RIPPLE_FITTING_PATH, "CFVehicleFittingData")
    configure_ammo_data(HEAVY_AMMO_PATH, heavy_ammo, "HeavyShell_Finite", "중포탄", "HeavyShell", HEAVY_AMMO_UNIT_MASS_KG, HEAVY_MAX_LOADABLE_AMMO_COUNT)
    configure_mount_data(HEAVY_MOUNT_PATH, heavy_mount, "AmmoP008_CannonMount")
    configure_weapon_data(HEAVY_WEAPON_PATH, heavy_weapon, "Proto_TurretCannon_Finite", heavy_ammo, 10, HEAVY_INITIAL_LOADED_AMMO_COUNT, HEAVY_RELOAD_SECONDS)
    configure_equipment_preset(HEAVY_PRESET_PATH, heavy_preset, "AmmoP008_HeavyKit", "유한탄 중포 키트", heavy_mount, heavy_weapon)
    configure_vehicle_data(HEAVY_VEHICLE_PATH, heavy_vehicle, heavy_preset)
    configure_fitting_data(HEAVY_FITTING_PATH, heavy_fitting, "AmmoP008_Heavy", "유한탄 중포 피팅", heavy_vehicle, heavy_preset, heavy_ammo, HEAVY_INITIAL_SORTIE_AMMO_COUNT)
    configure_ammo_data(RIPPLE_AMMO_PATH, ripple_ammo, "ProtoRocket_Finite", "로켓", "Rocket", RIPPLE_AMMO_UNIT_MASS_KG, RIPPLE_MAX_LOADABLE_AMMO_COUNT)
    configure_mount_data(RIPPLE_MOUNT_PATH, ripple_mount, "AmmoP008_RocketMount")
    configure_weapon_data(RIPPLE_WEAPON_PATH, ripple_weapon, "Proto_RocketLauncher_Finite", ripple_ammo, 4, RIPPLE_INITIAL_LOADED_AMMO_COUNT, RIPPLE_RELOAD_SECONDS)
    configure_equipment_preset(RIPPLE_PRESET_PATH, ripple_preset, "AmmoP008_RippleKit", "유한탄 Ripple 키트", ripple_mount, ripple_weapon)
    configure_vehicle_data(RIPPLE_VEHICLE_PATH, ripple_vehicle, ripple_preset)
    configure_fitting_data(RIPPLE_FITTING_PATH, ripple_fitting, "AmmoP008_Ripple", "유한탄 Ripple 피팅", ripple_vehicle, ripple_preset, ripple_ammo, RIPPLE_INITIAL_SORTIE_AMMO_COUNT)
    if not APPLY_CHANGES:
        return
    for asset_path, asset in [
        (HEAVY_AMMO_PATH, heavy_ammo),
        (HEAVY_MOUNT_PATH, heavy_mount),
        (HEAVY_WEAPON_PATH, heavy_weapon),
        (HEAVY_PRESET_PATH, heavy_preset),
        (HEAVY_VEHICLE_PATH, heavy_vehicle),
        (HEAVY_FITTING_PATH, heavy_fitting),
        (RIPPLE_AMMO_PATH, ripple_ammo),
        (RIPPLE_MOUNT_PATH, ripple_mount),
        (RIPPLE_WEAPON_PATH, ripple_weapon),
        (RIPPLE_PRESET_PATH, ripple_preset),
        (RIPPLE_VEHICLE_PATH, ripple_vehicle),
        (RIPPLE_FITTING_PATH, ripple_fitting),
    ]:
        save_asset(asset_path, asset)
    REPORT["heavy_snapshot"] = validate_fitting_snapshot(
        HEAVY_FITTING_PATH,
        HEAVY_WEAPON_PATH,
        HEAVY_AMMO_PATH,
        HEAVY_INITIAL_SORTIE_AMMO_COUNT,
        HEAVY_INITIAL_SORTIE_AMMO_COUNT * HEAVY_AMMO_UNIT_MASS_KG,
    )
    REPORT["ripple_snapshot"] = validate_fitting_snapshot(
        RIPPLE_FITTING_PATH,
        RIPPLE_WEAPON_PATH,
        RIPPLE_AMMO_PATH,
        RIPPLE_INITIAL_SORTIE_AMMO_COUNT,
        RIPPLE_INITIAL_SORTIE_AMMO_COUNT * RIPPLE_AMMO_UNIT_MASS_KG,
    )
    REPORT["heavy_chain_valid"] = True
    REPORT["ripple_chain_valid"] = True
    REPORT["heavy_map"] = configure_test_map(HEAVY_MAP_PATH, heavy_vehicle, heavy_fitting)
    REPORT["ripple_map"] = configure_test_map(RIPPLE_MAP_PATH, ripple_vehicle, ripple_fitting)
    REPORT["heavy_map_connected"] = True
    REPORT["ripple_map_connected"] = True


# [v1.0.0] 보호 원본 검증, Dry Run 또는 Apply와 구조화 보고서 기록을 수행합니다.
def main() -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    REPORT["protected_hashes_before"] = capture_hashes(PROTECTED_ASSET_PATHS)
    missing_protected_assets = [path for path, digest in REPORT["protected_hashes_before"].items() if not digest]
    if missing_protected_assets:
        raise RuntimeError(f"Protected source asset files are missing: {missing_protected_assets}")
    for required_source_path in sorted(PROTECTED_ASSET_PATHS):
        require_asset(required_source_path)
    apply_asset_chains()
    if not APPLY_CHANGES:
        record_change(HEAVY_MAP_PATH, "PlayerAmmoExpected", "5 | 15")
        record_change(RIPPLE_MAP_PATH, "PlayerAmmoExpected", "3 | 7")
    REPORT["protected_hashes_after"] = capture_hashes(PROTECTED_ASSET_PATHS)
    REPORT["protected_contract_passed"] = REPORT["protected_hashes_before"] == REPORT["protected_hashes_after"]
    if not REPORT["protected_contract_passed"]:
        raise RuntimeError("One or more protected source assets changed during AMMO-P0-08 preparation.")
    REPORT["success"] = True


try:
    main()
except Exception as error:
    REPORT["errors"].append(str(error))
    REPORT["errors"].append(traceback.format_exc())
    unreal.log_error(f"[CarFight][AmmoIntegration] FAILED: {error}")
finally:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(REPORT, ensure_ascii=False, indent=2), encoding="utf-8")
    log(f"Report: {REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("AMMO-P0-08 asset preparation failed. See Saved/AmmoIntegrationApply/report.json")
