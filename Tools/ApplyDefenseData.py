# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.2.0
# Date: 2026-07-31
# Description: CF-FQ-033 DR-P0-05 방어·관통 테스트 DataAsset과 전용 VehicleData 생성·연결 도구
# Scope: DA_VehicleDefense_Test, DA_VehicleDefense_TestSUV와 DA_DamageArmorPenTest만 생성·갱신하며 기존 VehicleData·DamageData는 읽기 전용으로 보호합니다.
# Changelog:
# - v1.2.0: BaseDamage 25·ArmorPenetration 50 관통 테스트 DamageData 생성과 기존 AP 0 DamageData 해시 보호 검증 추가.
# - v1.1.0: DA_TestSUV 파일 잠금과 CF-FQ-029 보호를 위해 별도 테스트 VehicleData 복제·연결 방식으로 전환.
# - v1.0.0: 방어 테스트 DataAsset 생성, 명시 수치 설정, SUV 단일 참조 연결, Sedan Legacy None 및 Launcher 핵심값 보호 검증 추가.
# Migration:
# - DA_TestSedan.DefaultDefenseData는 None으로 유지해 Legacy Integrity Fallback 회귀 차량으로 사용합니다.
# - DA_TestSUV는 RocketLauncher 설정을 보존하는 읽기 전용 복제 원본이며 직접 저장하지 않습니다.
# - 기존 DA_DamageAsset은 BaseDamage 25·ArmorPenetration 0 무관통 기준으로 유지하고 직접 수정하지 않습니다.
# - DA_DamageArmorPenTest는 DR-P0-07 관통력 차이 검증용이며 Weapon·Projectile에 자동 연결하지 않습니다.
# - DR-P0-07에서 방어 차량 PIE가 필요하면 DA_VehicleDefense_TestSUV를 명시적으로 선택하거나 배치합니다.

from __future__ import annotations

import hashlib
import json
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.2.0] 도구 결과와 문서에서 식별할 현재 도구 버전입니다.
TOOL_VERSION = "1.2.0"

# [v1.0.0] 명시적 Apply 실행일 때만 .uasset을 생성·저장하는 환경 변수 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_DEFENSE_APPLY", "0") == "1"

# [v1.0.0] Unreal 프로젝트 루트의 절대 경로입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 실행 결과 JSON을 저장할 프로젝트 Saved 하위 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "VehicleDefenseAssetApply"

# [v1.0.0] MCP와 후속 세션이 읽을 구조화 결과 JSON 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] 신규 테스트 VehicleDefenseData를 저장할 Unreal 패키지 폴더입니다.
DEFENSE_DATA_FOLDER = "/Game/CarFight/Vehicles/Data/Defense"

# [v1.0.0] 생성할 테스트 VehicleDefenseData 에셋 이름입니다.
DEFENSE_DATA_NAME = "DA_VehicleDefense_Test"

# [v1.0.0] 생성할 테스트 VehicleDefenseData 전체 에셋 경로입니다.
DEFENSE_DATA_PATH = f"{DEFENSE_DATA_FOLDER}/{DEFENSE_DATA_NAME}"

# [v1.2.0] 관통 테스트 DamageData를 저장할 Unreal 패키지 폴더입니다.
DAMAGE_DATA_FOLDER = "/Game/CarFight/Weapons/Data/DamageDefs"

# [v1.2.0] 생성할 관통 테스트 DamageData 에셋 이름입니다.
DAMAGE_DATA_NAME = "DA_DamageArmorPenTest"

# [v1.2.0] 생성할 관통 테스트 DamageData 전체 에셋 경로입니다.
DAMAGE_DATA_PATH = f"{DAMAGE_DATA_FOLDER}/{DAMAGE_DATA_NAME}"

# [v1.2.0] BaseDamage 25·ArmorPenetration 0을 보존할 기존 읽기 전용 DamageData 경로입니다.
LEGACY_DAMAGE_DATA_PATH = "/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset"

# [v1.1.0] CF-FQ-029 RocketLauncher 설정을 복사할 읽기 전용 SUV VehicleData 원본 경로입니다.
SOURCE_TEST_SUV_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"

# [v1.1.0] 생성할 방어 전용 테스트 VehicleData 에셋 이름입니다.
TEST_VEHICLE_DATA_NAME = "DA_VehicleDefense_TestSUV"

# [v1.1.0] 방어 참조를 연결할 별도 테스트 VehicleData 전체 경로입니다.
TEST_VEHICLE_DATA_PATH = f"{DEFENSE_DATA_FOLDER}/{TEST_VEHICLE_DATA_NAME}"

# [v1.0.0] DefaultDefenseData=None Legacy 회귀를 유지할 Sedan VehicleData 경로입니다.
LEGACY_SEDAN_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan"

# [v1.2.0] 이번 도구가 생성 또는 수정할 수 있는 유일한 에셋 경로 집합입니다.
MUTABLE_ASSET_PATHS = {
    DEFENSE_DATA_PATH,
    TEST_VEHICLE_DATA_PATH,
    DAMAGE_DATA_PATH,
}

# [v1.0.0] 실행 모드, 변경 목록, 보호 검증과 파일 해시를 기록할 결과 객체입니다.
REPORT: dict[str, Any] = {
    "schema_version": "vehicle_defense_asset_apply_v1",
    "tool_version": TOOL_VERSION,
        "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "defense_data_path": DEFENSE_DATA_PATH,
    "damage_data_path": DAMAGE_DATA_PATH,
    "legacy_damage_data_path": LEGACY_DAMAGE_DATA_PATH,
    "source_vehicle_data_path": SOURCE_TEST_SUV_PATH,
    "target_vehicle_data_path": TEST_VEHICLE_DATA_PATH,
    "legacy_vehicle_data_path": LEGACY_SEDAN_PATH,
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "planned_changes": [],
    "before_snapshot": {},
    "after_snapshot": {},
    "file_hashes_before": {},
    "file_hashes_after": {},
    "protected_contract_passed": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 방어 자산 도구 메시지를 남깁니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][DefenseData] {message}")


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


# [v1.0.0] Unreal 에셋 경로를 프로젝트 Content의 실제 .uasset 파일 경로로 변환합니다.
def asset_file_path(asset_path: str) -> Path:
    if not asset_path.startswith("/Game/"):
        raise RuntimeError(f"Only /Game asset paths are supported: {asset_path}")
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'): ]}.uasset"


# [v1.0.0] 존재하는 파일의 SHA-256을 계산하고 없으면 빈 문자열을 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
    digest = hashlib.sha256()
    with file_path.open("rb") as source_file:
        for chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


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
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.0.0] 화이트리스트를 확인한 뒤 Apply 모드에서만 에셋 프로퍼티를 씁니다.
def apply_property(asset_path: str, asset: Any, property_name: str, value: Any) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in the mutation whitelist: {asset_path}")
    record_change(asset_path, property_name, value)
    if APPLY_CHANGES:
        set_property(asset, property_name, value)


# [v1.0.0] 방향 장갑 구조체의 최대 장갑과 피해 배율을 명시적으로 설정합니다.
def configure_directional_armor(
    asset_path: str,
    defense_asset: Any,
    property_name: str,
    maximum_armor: float,
    damage_multiplier: float,
) -> None:
    armor_config = get_property(defense_asset, property_name)
    record_change(asset_path, f"{property_name}.MaximumArmor", maximum_armor)
    record_change(asset_path, f"{property_name}.DamageMultiplier", damage_multiplier)
    if APPLY_CHANGES:
        set_property(armor_config, "MaximumArmor", maximum_armor)
        set_property(armor_config, "DamageMultiplier", damage_multiplier)
        set_property(defense_asset, property_name, armor_config)


# [v1.0.0] VehicleData의 Launcher·하드포인트·내구도 핵심값과 방어 참조를 비교 가능한 사전/사후 스냅샷으로 만듭니다.
def capture_vehicle_snapshot(vehicle_asset: Any) -> dict[str, Any]:
    mount_profiles = list(get_property(vehicle_asset, "MountProfiles"))
    hardpoint_slots = list(get_property(vehicle_asset, "HardpointSlots"))
    durability_config = get_property(vehicle_asset, "VehicleDurabilityConfig")
    visual_config = get_property(vehicle_asset, "VehicleVisualConfig")

    return {
        "default_defense_data": object_path(get_property(vehicle_asset, "DefaultDefenseData")),
        "mount_profile_count": len(mount_profiles),
        "mount_equipment_presets": [
            object_path(get_property(mount_profile, "DefaultEquipmentPresetData"))
            for mount_profile in mount_profiles
        ],
        "hardpoint_count": len(hardpoint_slots),
        "hardpoint_ids": [str(get_property(hardpoint_slot, "LocationSlotId")) for hardpoint_slot in hardpoint_slots],
        "hardpoint_socket_names": [str(get_property(hardpoint_slot, "SocketName")) for hardpoint_slot in hardpoint_slots],
        "maximum_integrity": float(get_property(durability_config, "MaxHealth")),
        "chassis_mesh": object_path(get_property(visual_config, "ChassisMesh")),
        "wheel_mesh_fl": object_path(get_property(visual_config, "WheelMeshFL")),
        "wheel_mesh_fr": object_path(get_property(visual_config, "WheelMeshFR")),
        "wheel_mesh_rl": object_path(get_property(visual_config, "WheelMeshRL")),
        "wheel_mesh_rr": object_path(get_property(visual_config, "WheelMeshRR")),
    }


# [v1.0.0] VehicleDefenseData의 실제 저장값을 검증 가능한 스냅샷으로 만듭니다.
def capture_defense_snapshot(defense_asset: Any) -> dict[str, Any]:
    def armor_values(property_name: str) -> dict[str, float]:
        armor_config = get_property(defense_asset, property_name)
        return {
            "maximum_armor": float(get_property(armor_config, "MaximumArmor")),
            "damage_multiplier": float(get_property(armor_config, "DamageMultiplier")),
        }

    return {
        "asset_path": object_path(defense_asset),
        "defense_id": str(get_property(defense_asset, "DefenseId")),
        "use_shield": bool(get_property(defense_asset, "bUseShield")),
        "maximum_shield": float(get_property(defense_asset, "MaximumShield")),
        "shield_regeneration_delay_seconds": float(
            get_property(defense_asset, "ShieldRegenerationDelaySeconds")
        ),
        "shield_regeneration_per_second": float(
            get_property(defense_asset, "ShieldRegenerationPerSecond")
        ),
        "armor_type": str(get_property(defense_asset, "ArmorType")),
        "armor_resistance": float(get_property(defense_asset, "ArmorResistance")),
        "front_armor": armor_values("FrontArmorConfig"),
        "left_armor": armor_values("LeftArmorConfig"),
        "right_armor": armor_values("RightArmorConfig"),
        "rear_armor": armor_values("RearArmorConfig"),
        "top_armor": armor_values("TopArmorConfig"),
        "bottom_armor": armor_values("BottomArmorConfig"),
        "shield_component_damage_scale": float(
            get_property(defense_asset, "ShieldComponentDamageScale")
        ),
        "armor_component_damage_scale": float(
            get_property(defense_asset, "ArmorComponentDamageScale")
        ),
        "integrity_component_damage_scale": float(
            get_property(defense_asset, "IntegrityComponentDamageScale")
        ),
    }


# [v1.2.0] DamageData의 핵심 직접 피해 설정을 비교 가능한 스냅샷으로 만듭니다.
def capture_damage_snapshot(damage_asset: Any) -> dict[str, Any]:
    return {
        "asset_path": object_path(damage_asset),
        "damage_id": str(get_property(damage_asset, "DamageId")),
        "base_damage": float(get_property(damage_asset, "BaseDamage")),
        "armor_penetration": float(get_property(damage_asset, "ArmorPenetration")),
        "can_damage_self": bool(get_property(damage_asset, "bCanDamageSelf")),
        "damage_type": str(get_property(damage_asset, "DamageType")),
    }


# [v1.0.0] 테스트 VehicleDefenseData를 로드하거나 Apply 모드에서 지정 경로에 생성합니다.
def load_or_create_defense_asset() -> Any | None:
    if unreal.EditorAssetLibrary.does_asset_exist(DEFENSE_DATA_PATH):
        REPORT["updated_assets"].append(DEFENSE_DATA_PATH)
        return require_asset(DEFENSE_DATA_PATH)

    if not APPLY_CHANGES:
        log(f"Dry Run create planned: {DEFENSE_DATA_PATH}")
        return None

    if not unreal.EditorAssetLibrary.does_directory_exist(DEFENSE_DATA_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(DEFENSE_DATA_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {DEFENSE_DATA_FOLDER}")

    defense_data_class = getattr(unreal, "CFVehicleDefenseData", None)
    if defense_data_class is None:
        raise RuntimeError("Unreal class was not found: CFVehicleDefenseData")

    data_asset_factory = unreal.DataAssetFactory()
    set_property(data_asset_factory, "DataAssetClass", defense_data_class)

    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        DEFENSE_DATA_NAME,
        DEFENSE_DATA_FOLDER,
        defense_data_class,
        data_asset_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"VehicleDefenseData asset creation failed: {DEFENSE_DATA_PATH}")

    REPORT["created_assets"].append(DEFENSE_DATA_PATH)
    return created_asset


# [v1.2.0] 관통 테스트 DamageData를 로드하거나 Apply 모드에서 지정 경로에 생성합니다.
def load_or_create_damage_asset() -> Any | None:
    if unreal.EditorAssetLibrary.does_asset_exist(DAMAGE_DATA_PATH):
        REPORT["updated_assets"].append(DAMAGE_DATA_PATH)
        return require_asset(DAMAGE_DATA_PATH)

    if not APPLY_CHANGES:
        log(f"Dry Run create planned: {DAMAGE_DATA_PATH}")
        return None

    if not unreal.EditorAssetLibrary.does_directory_exist(DAMAGE_DATA_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(DAMAGE_DATA_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {DAMAGE_DATA_FOLDER}")

    damage_data_class = getattr(unreal, "CFDamageData", None)
    if damage_data_class is None:
        raise RuntimeError("Unreal class was not found: CFDamageData")

    data_asset_factory = unreal.DataAssetFactory()
    set_property(data_asset_factory, "DataAssetClass", damage_data_class)

    created_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        DAMAGE_DATA_NAME,
        DAMAGE_DATA_FOLDER,
        damage_data_class,
        data_asset_factory,
    )
    if created_asset is None:
        raise RuntimeError(f"DamageData asset creation failed: {DAMAGE_DATA_PATH}")

    REPORT["created_assets"].append(DAMAGE_DATA_PATH)
    return created_asset


# [v1.1.0] 읽기 전용 DA_TestSUV를 복제해 방어 전용 테스트 VehicleData를 로드하거나 생성합니다.
def load_or_create_test_vehicle_data(source_test_suv: Any) -> Any | None:
    if unreal.EditorAssetLibrary.does_asset_exist(TEST_VEHICLE_DATA_PATH):
        REPORT["updated_assets"].append(TEST_VEHICLE_DATA_PATH)
        return require_asset(TEST_VEHICLE_DATA_PATH)

    if not APPLY_CHANGES:
        log(
            f"Dry Run duplicate planned: {SOURCE_TEST_SUV_PATH} -> {TEST_VEHICLE_DATA_PATH}"
        )
        return None

    if not unreal.EditorAssetLibrary.does_directory_exist(DEFENSE_DATA_FOLDER):
        if not unreal.EditorAssetLibrary.make_directory(DEFENSE_DATA_FOLDER):
            raise RuntimeError(f"Asset folder creation failed: {DEFENSE_DATA_FOLDER}")

    duplicated_vehicle_data = unreal.EditorAssetLibrary.duplicate_asset(
        SOURCE_TEST_SUV_PATH,
        TEST_VEHICLE_DATA_PATH,
    )
    if duplicated_vehicle_data is None:
        raise RuntimeError(
            f"VehicleData duplication failed: {SOURCE_TEST_SUV_PATH} -> {TEST_VEHICLE_DATA_PATH}"
        )

    REPORT["created_assets"].append(TEST_VEHICLE_DATA_PATH)
    return duplicated_vehicle_data


# [v1.0.0] 테스트 방어 자산에 P0 명시 기준값을 설정합니다.
def configure_defense_asset(defense_asset: Any | None) -> None:
    if defense_asset is None:
        record_change(DEFENSE_DATA_PATH, "CreateAsset", "CFVehicleDefenseData")
        return

    apply_property(DEFENSE_DATA_PATH, defense_asset, "DefenseId", "VehicleDefense_Test")
    apply_property(DEFENSE_DATA_PATH, defense_asset, "bUseShield", True)
    apply_property(DEFENSE_DATA_PATH, defense_asset, "MaximumShield", 100.0)
    apply_property(DEFENSE_DATA_PATH, defense_asset, "ShieldRegenerationDelaySeconds", 5.0)
    apply_property(DEFENSE_DATA_PATH, defense_asset, "ShieldRegenerationPerSecond", 10.0)
    apply_property(
        DEFENSE_DATA_PATH,
        defense_asset,
        "ArmorType",
        enum_value("CFArmorType", "Standard"),
    )
    apply_property(DEFENSE_DATA_PATH, defense_asset, "ArmorResistance", 100.0)

    configure_directional_armor(
        DEFENSE_DATA_PATH, defense_asset, "FrontArmorConfig", 100.0, 1.0
    )
    configure_directional_armor(
        DEFENSE_DATA_PATH, defense_asset, "LeftArmorConfig", 100.0, 1.2
    )
    configure_directional_armor(
        DEFENSE_DATA_PATH, defense_asset, "RightArmorConfig", 100.0, 1.2
    )
    configure_directional_armor(
        DEFENSE_DATA_PATH, defense_asset, "RearArmorConfig", 100.0, 1.5
    )
    configure_directional_armor(
        DEFENSE_DATA_PATH, defense_asset, "TopArmorConfig", 100.0, 1.3
    )
    configure_directional_armor(
        DEFENSE_DATA_PATH, defense_asset, "BottomArmorConfig", 100.0, 1.6
    )

    apply_property(DEFENSE_DATA_PATH, defense_asset, "ShieldComponentDamageScale", 0.0)
    apply_property(DEFENSE_DATA_PATH, defense_asset, "ArmorComponentDamageScale", 0.25)
    apply_property(DEFENSE_DATA_PATH, defense_asset, "IntegrityComponentDamageScale", 1.0)


# [v1.2.0] 관통 테스트 DamageData에 직접 피해와 관통 기준값을 설정합니다.
def configure_damage_asset(damage_asset: Any | None) -> None:
    if damage_asset is None:
        record_change(DAMAGE_DATA_PATH, "CreateAsset", "CFDamageData")
        return

    apply_property(DAMAGE_DATA_PATH, damage_asset, "DamageId", "ArmorPenetration_Test")
    apply_property(DAMAGE_DATA_PATH, damage_asset, "BaseDamage", 25.0)
    apply_property(DAMAGE_DATA_PATH, damage_asset, "ArmorPenetration", 50.0)
    apply_property(DAMAGE_DATA_PATH, damage_asset, "bCanDamageSelf", False)
    apply_property(
        DAMAGE_DATA_PATH,
        damage_asset,
        "DamageType",
        enum_value("CFDamageType", "Kinetic"),
    )


# [v1.0.0] Apply 모드에서 화이트리스트된 에셋을 강제로 저장합니다.
def save_asset(asset_path: str, asset: Any) -> None:
    if not APPLY_CHANGES:
        return
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Save path is not in the mutation whitelist: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.2.0] 읽기 전용 원본을 보존하면서 방어·관통 테스트 DataAsset과 별도 테스트 VehicleData를 생성·연결합니다.
def apply_vehicle_defense_assets() -> None:
    source_test_suv = require_asset(SOURCE_TEST_SUV_PATH)
    legacy_sedan = require_asset(LEGACY_SEDAN_PATH)
    legacy_damage_data = require_asset(LEGACY_DAMAGE_DATA_PATH)

    REPORT["file_hashes_before"] = {
        DEFENSE_DATA_PATH: sha256_file(asset_file_path(DEFENSE_DATA_PATH)),
        DAMAGE_DATA_PATH: sha256_file(asset_file_path(DAMAGE_DATA_PATH)),
        LEGACY_DAMAGE_DATA_PATH: sha256_file(asset_file_path(LEGACY_DAMAGE_DATA_PATH)),
        SOURCE_TEST_SUV_PATH: sha256_file(asset_file_path(SOURCE_TEST_SUV_PATH)),
        TEST_VEHICLE_DATA_PATH: sha256_file(asset_file_path(TEST_VEHICLE_DATA_PATH)),
        LEGACY_SEDAN_PATH: sha256_file(asset_file_path(LEGACY_SEDAN_PATH)),
    }

    before_source_suv_snapshot = capture_vehicle_snapshot(source_test_suv)
    before_sedan_snapshot = capture_vehicle_snapshot(legacy_sedan)
    before_legacy_damage_snapshot = capture_damage_snapshot(legacy_damage_data)
    REPORT["before_snapshot"] = {
        "source_test_suv": before_source_suv_snapshot,
        "legacy_sedan": before_sedan_snapshot,
        "legacy_damage_data": before_legacy_damage_snapshot,
    }

    if before_source_suv_snapshot["default_defense_data"]:
        raise RuntimeError(
            "DA_TestSUV.DefaultDefenseData must remain None as the CF-FQ-029 protected source."
        )
    if before_sedan_snapshot["default_defense_data"]:
        raise RuntimeError(
            "DA_TestSedan.DefaultDefenseData must remain None before DR-P0-05 apply."
        )

    defense_asset = load_or_create_defense_asset()
    configure_defense_asset(defense_asset)

    damage_asset = load_or_create_damage_asset()
    configure_damage_asset(damage_asset)

    test_vehicle_data = load_or_create_test_vehicle_data(source_test_suv)
    if defense_asset is not None and test_vehicle_data is not None:
        apply_property(
            TEST_VEHICLE_DATA_PATH,
            test_vehicle_data,
            "DefaultDefenseData",
            defense_asset,
        )
        save_asset(DEFENSE_DATA_PATH, defense_asset)
        save_asset(TEST_VEHICLE_DATA_PATH, test_vehicle_data)
    else:
        record_change(
            TEST_VEHICLE_DATA_PATH,
            "DefaultDefenseData",
            DEFENSE_DATA_PATH,
        )

    if damage_asset is not None:
        save_asset(DAMAGE_DATA_PATH, damage_asset)

    if APPLY_CHANGES:
        reloaded_defense_asset = require_asset(DEFENSE_DATA_PATH)
        reloaded_damage_asset = require_asset(DAMAGE_DATA_PATH)
        reloaded_legacy_damage_data = require_asset(LEGACY_DAMAGE_DATA_PATH)
        reloaded_source_test_suv = require_asset(SOURCE_TEST_SUV_PATH)
        reloaded_test_vehicle_data = require_asset(TEST_VEHICLE_DATA_PATH)
        reloaded_legacy_sedan = require_asset(LEGACY_SEDAN_PATH)

        after_source_suv_snapshot = capture_vehicle_snapshot(reloaded_source_test_suv)
        after_test_vehicle_snapshot = capture_vehicle_snapshot(reloaded_test_vehicle_data)
        after_sedan_snapshot = capture_vehicle_snapshot(reloaded_legacy_sedan)
        defense_snapshot = capture_defense_snapshot(reloaded_defense_asset)
        damage_snapshot = capture_damage_snapshot(reloaded_damage_asset)
        after_legacy_damage_snapshot = capture_damage_snapshot(reloaded_legacy_damage_data)

        protected_target_snapshot = dict(after_test_vehicle_snapshot)
        protected_target_snapshot.pop("default_defense_data", None)
        protected_source_snapshot = dict(before_source_suv_snapshot)
        protected_source_snapshot.pop("default_defense_data", None)

        if before_source_suv_snapshot != after_source_suv_snapshot:
            raise RuntimeError("DA_TestSUV changed unexpectedly during DR-P0-05 apply.")
        if before_sedan_snapshot != after_sedan_snapshot:
            raise RuntimeError("DA_TestSedan changed unexpectedly during DR-P0-05 apply.")
        if before_legacy_damage_snapshot != after_legacy_damage_snapshot:
            raise RuntimeError("DA_DamageAsset changed unexpectedly during DR-P0-05 apply.")
        if protected_target_snapshot != protected_source_snapshot:
            raise RuntimeError(
                "DA_VehicleDefense_TestSUV does not preserve the source SUV Launcher, hardpoint, durability or visual fields."
            )
        if DEFENSE_DATA_PATH not in after_test_vehicle_snapshot["default_defense_data"]:
            raise RuntimeError(
                "DA_VehicleDefense_TestSUV.DefaultDefenseData verification failed: "
                f"{after_test_vehicle_snapshot['default_defense_data']}"
            )
        if after_source_suv_snapshot["default_defense_data"]:
            raise RuntimeError("DA_TestSUV protected DefaultDefenseData=None contract was broken.")
        if after_sedan_snapshot["default_defense_data"]:
            raise RuntimeError("DA_TestSedan Legacy DefaultDefenseData=None contract was broken.")

        expected_damage_values = {
            "damage_id": "ArmorPenetration_Test",
            "base_damage": 25.0,
            "armor_penetration": 50.0,
            "can_damage_self": False,
        }
        for field_name, expected_value in expected_damage_values.items():
            if damage_snapshot[field_name] != expected_value:
                raise RuntimeError(
                    f"Damage value verification failed: {field_name}={damage_snapshot[field_name]} expected={expected_value}"
                )

        if before_legacy_damage_snapshot["base_damage"] != 25.0:
            raise RuntimeError("DA_DamageAsset BaseDamage 25 protection contract failed.")
        if before_legacy_damage_snapshot["armor_penetration"] != 0.0:
            raise RuntimeError("DA_DamageAsset ArmorPenetration 0 protection contract failed.")

        expected_defense_values = {
            "defense_id": "VehicleDefense_Test",
            "use_shield": True,
            "maximum_shield": 100.0,
            "shield_regeneration_delay_seconds": 5.0,
            "shield_regeneration_per_second": 10.0,
            "armor_resistance": 100.0,
        }
        for field_name, expected_value in expected_defense_values.items():
            if defense_snapshot[field_name] != expected_value:
                raise RuntimeError(
                    f"Defense value verification failed: {field_name}={defense_snapshot[field_name]} expected={expected_value}"
                )

        REPORT["after_snapshot"] = {
            "source_test_suv": after_source_suv_snapshot,
            "test_vehicle_data": after_test_vehicle_snapshot,
            "legacy_sedan": after_sedan_snapshot,
            "legacy_damage_data": after_legacy_damage_snapshot,
            "damage_data": damage_snapshot,
            "vehicle_defense_data": defense_snapshot,
        }
        REPORT["protected_contract_passed"] = True

    REPORT["file_hashes_after"] = {
        DEFENSE_DATA_PATH: sha256_file(asset_file_path(DEFENSE_DATA_PATH)),
        DAMAGE_DATA_PATH: sha256_file(asset_file_path(DAMAGE_DATA_PATH)),
        LEGACY_DAMAGE_DATA_PATH: sha256_file(asset_file_path(LEGACY_DAMAGE_DATA_PATH)),
        SOURCE_TEST_SUV_PATH: sha256_file(asset_file_path(SOURCE_TEST_SUV_PATH)),
        TEST_VEHICLE_DATA_PATH: sha256_file(asset_file_path(TEST_VEHICLE_DATA_PATH)),
        LEGACY_SEDAN_PATH: sha256_file(asset_file_path(LEGACY_SEDAN_PATH)),
    }



try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_vehicle_defense_assets()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][DefenseData] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("VehicleDefense asset application failed. Read report.json for details.")
