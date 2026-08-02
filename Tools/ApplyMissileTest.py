# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.1
# Date: 2026-08-02
# Description: CF-FQ-030 Direct 미사일 전용 PIE 테스트 자산·맵 생성 도구
# Scope: 신규 MissileTest 자산과 MissileDirectTest 복제 맵만 생성·갱신하고 기존 Rocket·Launcher·SUV·TestMap 원본은 읽기 전용으로 보호합니다.
# Changelog:
# - v1.0.1: 맵 .umap 보호 해시, 파괴 시나리오 거리·시간과 생성 블록 들여쓰기 검증을 보강.
# - v1.0.0: Direct Missile Projectile·Weapon·Preset·VehicleData와 5종 테스트 타겟 복제 맵 생성, 원본 SHA-256 보호 검증 추가.
# Migration:
# - 기존 DA_Rocket_PropTest, DA_RocketLauncher, RocketLauncher, DA_TestSUV와 TestMap은 저장하지 않습니다.

from __future__ import annotations

import hashlib
import json
import os
import re
import traceback
from pathlib import Path
from typing import Any

import unreal


# [v1.0.1] 결과 보고서와 재실행 호환성을 식별할 도구 버전입니다.
TOOL_VERSION = "1.0.1"

# [v1.0.0] 명시적 Apply 실행에서만 신규 .uasset과 복제 맵을 저장하는 상태입니다.
APPLY_CHANGES = os.environ.get("CARFIGHT_MISSILE_TEST_APPLY", "0") == "1"

# [v1.0.0] 현재 Unreal 프로젝트의 실제 루트 디렉터리입니다.
PROJECT_DIR = Path(unreal.Paths.project_dir()).resolve()

# [v1.0.0] 구조화 실행 결과를 저장할 Saved 디렉터리입니다.
REPORT_DIR = PROJECT_DIR / "Saved" / "MissileTestApply"

# [v1.0.0] Admin process.status가 읽을 UTF-8 JSON 보고서 경로입니다.
REPORT_PATH = REPORT_DIR / "report.json"

# [v1.0.0] 테스트 자산을 격리할 Unreal 패키지 폴더입니다.
TEST_ASSET_FOLDER = "/Game/CarFight/Tests/Missile"

# [v1.0.0] 기존 비유도 Rocket ProjectileData 읽기 전용 복제 원본입니다.
SOURCE_PROJECTILE_PATH = "/Game/CarFight/Weapons/Data/ProjectileDefs/DA_Rocket_PropTest"

# [v1.0.0] Direct Guidance를 활성화할 신규 미사일 ProjectileData 경로입니다.
TEST_PROJECTILE_PATH = f"{TEST_ASSET_FOLDER}/DA_Missile_DirectTest"

# [v1.0.0] 기존 RocketLauncher WeaponData 읽기 전용 복제 원본입니다.
SOURCE_WEAPON_PATH = "/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher"

# [v1.0.0] SingleCycle·Direct 전용 신규 미사일 WeaponData 경로입니다.
TEST_WEAPON_PATH = f"{TEST_ASSET_FOLDER}/DA_Missile_DirectWeapon"

# [v1.0.0] 기존 RocketLauncher EquipmentPresetData 읽기 전용 복제 원본입니다.
SOURCE_PRESET_PATH = "/Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher"

# [v1.0.0] 기존 터렛 시각을 참조하되 신규 WeaponData만 사용하는 테스트 프리셋 경로입니다.
TEST_PRESET_PATH = f"{TEST_ASSET_FOLDER}/EQ_Missile_DirectTest"

# [v1.0.0] 기존 Launcher SUV VehicleData 읽기 전용 복제 원본입니다.
SOURCE_VEHICLE_PATH = "/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV"

# [v1.0.0] 신규 미사일 프리셋을 첫 MountProfile에 연결할 테스트 VehicleData 경로입니다.
TEST_VEHICLE_PATH = f"{TEST_ASSET_FOLDER}/DA_Missile_TestSUV"

# [v1.0.0] 플레이 환경과 차량 배치를 복제할 읽기 전용 원본 맵입니다.
SOURCE_MAP_PATH = "/Game/Maps/TestMap"

# [v1.0.0] 미사일 테스트 차량과 경량 타겟만 별도로 저장할 신규 맵입니다.
TEST_MAP_PATH = "/Game/Maps/MissileDirectTest"

# [v1.0.0] 복제 맵의 플레이 차량으로 사용할 기존 Blueprint Generated Class 경로입니다.
PLAYER_PAWN_CLASS_PATH = "/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"

# [v1.0.0] 신규 C++ 경량 테스트 타겟 클래스 경로입니다.
MISSILE_TARGET_CLASS_PATH = "/Script/CarFight_Re.CFMissileTestTarget"

# [v1.0.0] 이번 도구가 생성 또는 수정할 수 있는 유일한 에셋 경로입니다.
MUTABLE_ASSET_PATHS = {
    TEST_PROJECTILE_PATH,
    TEST_WEAPON_PATH,
    TEST_PRESET_PATH,
    TEST_VEHICLE_PATH,
    TEST_MAP_PATH,
}

# [v1.0.0] 실행 모드, 생성·저장 목록과 보호 검증을 기록할 결과 객체입니다.
REPORT: dict[str, Any] = {
    "schema_version": "missile_test_apply_v1",
    "tool_version": TOOL_VERSION,
    "mode": "apply" if APPLY_CHANGES else "dry_run",
    "success": False,
    "test_projectile_path": TEST_PROJECTILE_PATH,
    "test_weapon_path": TEST_WEAPON_PATH,
    "test_preset_path": TEST_PRESET_PATH,
    "test_vehicle_path": TEST_VEHICLE_PATH,
    "test_map_path": TEST_MAP_PATH,
    "created_assets": [],
    "updated_assets": [],
    "saved_assets": [],
    "spawned_targets": [],
    "planned_changes": [],
    "protected_hashes_before": {},
    "protected_hashes_after": {},
    "protected_contract_passed": False,
    "errors": [],
}


# [v1.0.0] Unreal Output Log에 미사일 테스트 도구 메시지를 출력합니다.
def log(message: str) -> None:
    unreal.log(f"[CarFight][MissileTest] {message}")


# [v1.0.0] PascalCase C++ 이름을 Unreal Python snake_case 후보로 변환합니다.
def pascal_to_snake(name: str) -> str:
    step1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", step1).lower()


# [v1.0.0] Reflection 프로퍼티 접근에 사용할 원본·snake_case 이름 후보를 반환합니다.
def candidate_property_names(name: str) -> list[str]:
    return list(dict.fromkeys([name, pascal_to_snake(name)]))


# [v1.0.0] C++ 또는 Unreal Python 이름으로 프로퍼티를 읽습니다.
def get_property(target: Any, property_name: str) -> Any:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            return target.get_editor_property(candidate)
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property read failed: {type(target).__name__}.{property_name}: {last_error}")


# [v1.0.0] C++ 또는 Unreal Python 이름으로 프로퍼티를 씁니다.
def set_property(target: Any, property_name: str, value: Any) -> None:
    last_error: Exception | None = None
    for candidate in candidate_property_names(property_name):
        try:
            target.set_editor_property(candidate, value)
            return
        except Exception as error:
            last_error = error
    raise RuntimeError(f"Property write failed: {type(target).__name__}.{property_name}: {last_error}")


# [v1.0.0] Unreal 객체의 전체 Object Path를 안정적으로 문자열화합니다.
def object_path(value: Any) -> str:
    if value is None:
        return ""
    try:
        return unreal.SystemLibrary.get_path_name(value)
    except Exception:
        return str(value)


# [v1.0.1] /Game 에셋 경로를 Content 아래 실제 .uasset 또는 .umap 파일 경로로 변환합니다.
def asset_file_path(asset_path: str) -> Path:
    if not asset_path.startswith("/Game/"):
        raise RuntimeError(f"Only /Game asset paths are supported: {asset_path}")

    # [v1.0.1] 원본·복제 테스트 맵은 일반 에셋과 달리 .umap 확장자를 사용합니다.
    package_extension = ".umap" if asset_path in {SOURCE_MAP_PATH, TEST_MAP_PATH} else ".uasset"
    return PROJECT_DIR / "Content" / f"{asset_path[len('/Game/'): ]}{package_extension}"


# [v1.0.0] 존재하는 파일의 SHA-256을 계산하고 없으면 빈 문자열을 반환합니다.
def sha256_file(file_path: Path) -> str:
    if not file_path.is_file():
        return ""
    digest = hashlib.sha256()
    with file_path.open("rb") as source_file:
        for chunk in iter(lambda: source_file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


# [v1.0.0] 필수 원본 에셋을 로드하고 누락 시 즉시 실패합니다.
def require_asset(asset_path: str) -> Any:
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Required asset is missing: {asset_path}")
    return asset


# [v1.0.0] Unreal enum 타입과 멤버를 Reflection 값으로 변환합니다.
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


# [v1.0.0] 실제 쓰기 전에 계획된 프로퍼티 변경을 JSON에 기록합니다.
def record_change(asset_path: str, property_name: str, value: Any) -> None:
    REPORT["planned_changes"].append(
        {
            "asset_path": asset_path,
            "property_name": property_name,
            "value": object_path(value) if hasattr(value, "get_class") else str(value),
        }
    )


# [v1.0.0] 화이트리스트를 검사한 뒤 Apply 모드에서만 프로퍼티를 씁니다.
def apply_property(asset_path: str, asset: Any, property_name: str, value: Any) -> None:
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Asset path is not in mutation whitelist: {asset_path}")
    record_change(asset_path, property_name, value)
    if APPLY_CHANGES:
        set_property(asset, property_name, value)


# [v1.0.0] 읽기 전용 원본을 지정 테스트 경로에 복제하거나 기존 테스트 에셋을 로드합니다.
def duplicate_or_load(source_path: str, target_path: str) -> Any | None:
    if target_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Target path is not in mutation whitelist: {target_path}")

    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        REPORT["updated_assets"].append(target_path)
        return require_asset(target_path)

    if not APPLY_CHANGES:
        require_asset(source_path)
        record_change(target_path, "DuplicateAsset", source_path)
        return None

    target_folder = target_path.rsplit("/", 1)[0]
    if not unreal.EditorAssetLibrary.does_directory_exist(target_folder):
        if not unreal.EditorAssetLibrary.make_directory(target_folder):
            raise RuntimeError(f"Asset folder creation failed: {target_folder}")

    duplicated_asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if duplicated_asset is None:
        raise RuntimeError(f"Asset duplication failed: {source_path} -> {target_path}")
    REPORT["created_assets"].append(target_path)
    return duplicated_asset


# [v1.0.0] 화이트리스트된 테스트 에셋을 강제로 저장합니다.
def save_asset(asset_path: str, asset: Any) -> None:
    if not APPLY_CHANGES:
        return
    if asset_path not in MUTABLE_ASSET_PATHS:
        raise RuntimeError(f"Save path is not in mutation whitelist: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Asset save failed: {asset_path}")
    REPORT["saved_assets"].append(asset_path)


# [v1.0.0] Direct Missile ProjectileData의 추진·Flight·Guidance 설정을 적용합니다.
def configure_projectile(projectile_asset: Any | None) -> None:
    if projectile_asset is None:
        record_change(TEST_PROJECTILE_PATH, "Configure", "Direct TargetActor Missile")
        return

    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "ProjectileId", "Missile_DirectTest")
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "InitialSpeed", 2500.0)
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "LifeTimeSeconds", 12.0)
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "bAffectedByGravity", False)
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "CollisionRadius", 12.0)
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "bUseSweepCollision", True)
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "bForceSubStepping", True)
    apply_property(TEST_PROJECTILE_PATH, projectile_asset, "bUseSupplementalContinuousSweep", True)

    propulsion_config = get_property(projectile_asset, "PropulsionConfig")
    record_change(TEST_PROJECTILE_PATH, "PropulsionConfig.bUsePropulsion", True)
    record_change(TEST_PROJECTILE_PATH, "PropulsionConfig.IgnitionDelaySeconds", 0.05)
    record_change(TEST_PROJECTILE_PATH, "PropulsionConfig.BurnDurationSeconds", 4.0)
    record_change(TEST_PROJECTILE_PATH, "PropulsionConfig.ThrustAccelerationCmPerSecSq", 3500.0)
    record_change(TEST_PROJECTILE_PATH, "PropulsionConfig.MaximumPropelledSpeed", 8000.0)
    if APPLY_CHANGES:
        set_property(propulsion_config, "bUsePropulsion", True)
        set_property(propulsion_config, "IgnitionDelaySeconds", 0.05)
        set_property(propulsion_config, "BurnDurationSeconds", 4.0)
        set_property(propulsion_config, "ThrustAccelerationCmPerSecSq", 3500.0)
        set_property(propulsion_config, "MaximumPropelledSpeed", 8000.0)
        set_property(projectile_asset, "PropulsionConfig", propulsion_config)

    flight_config = get_property(projectile_asset, "MissileFlightConfig")
    record_change(TEST_PROJECTILE_PATH, "MissileFlightConfig.bUseMissileFlight", True)
    record_change(TEST_PROJECTILE_PATH, "MissileFlightConfig.AttackProfile", "Direct")
    record_change(TEST_PROJECTILE_PATH, "MissileFlightConfig.MinimumClearanceTimeSeconds", 0.15)
    record_change(TEST_PROJECTILE_PATH, "MissileFlightConfig.MinimumClearanceDistanceCm", 300.0)
    record_change(TEST_PROJECTILE_PATH, "MissileFlightConfig.TransitionDurationSeconds", 0.0)
    record_change(TEST_PROJECTILE_PATH, "MissileFlightConfig.bUseTerminalPhase", False)
    if APPLY_CHANGES:
        set_property(flight_config, "bUseMissileFlight", True)
        set_property(flight_config, "AttackProfile", enum_value("CFMissileAttackProfile", "Direct"))
        set_property(flight_config, "MinimumClearanceTimeSeconds", 0.15)
        set_property(flight_config, "MinimumClearanceDistanceCm", 300.0)
        set_property(flight_config, "TransitionDurationSeconds", 0.0)
        set_property(flight_config, "bUseTerminalPhase", False)
        set_property(projectile_asset, "MissileFlightConfig", flight_config)

    guide_config = get_property(projectile_asset, "MissileGuideConfig")
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.bUseGuidance", True)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.GuideMode", "TargetActor")
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.LostTargetPolicy", "ContinueStraight")
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.NavigationConstant", 3.0)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.MaximumTurnRateDegPerSec", 45.0)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq", 6000.0)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.GuidanceResponseTimeSeconds", 0.1)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.MinimumGuidanceSpeedCmPerSec", 500.0)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.SeekerFieldOfViewDeg", 120.0)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.LockBreakAngleDeg", 85.0)
    record_change(TEST_PROJECTILE_PATH, "MissileGuideConfig.TargetLostGraceTimeSeconds", 0.25)
    if APPLY_CHANGES:
        set_property(guide_config, "bUseGuidance", True)
        set_property(guide_config, "GuideMode", enum_value("CFMissileGuideMode", "TargetActor"))
        set_property(guide_config, "LostTargetPolicy", enum_value("CFMissileLostTargetPolicy", "ContinueStraight"))
        set_property(guide_config, "NavigationConstant", 3.0)
        set_property(guide_config, "MaximumTurnRateDegPerSec", 45.0)
        set_property(guide_config, "MaximumLateralAccelerationCmPerSecSq", 6000.0)
        set_property(guide_config, "GuidanceResponseTimeSeconds", 0.1)
        set_property(guide_config, "MinimumGuidanceSpeedCmPerSec", 500.0)
        set_property(guide_config, "SeekerFieldOfViewDeg", 120.0)
        set_property(guide_config, "LockBreakAngleDeg", 85.0)
        set_property(guide_config, "TargetLostGraceTimeSeconds", 0.25)
        set_property(projectile_asset, "MissileGuideConfig", guide_config)


# [v1.0.0] SingleCycle·Direct 전용 WeaponData를 구성합니다.
def configure_weapon(weapon_asset: Any | None, projectile_asset: Any | None) -> None:
    if weapon_asset is None:
        record_change(TEST_WEAPON_PATH, "Configure", "SingleCycle Direct Missile Weapon")
        return

        apply_property(TEST_WEAPON_PATH, weapon_asset, "WeaponId", "Missile_DirectTest")
    apply_property(TEST_WEAPON_PATH, weapon_asset, "FireMode", enum_value("CFWeaponFireMode", "Projectile"))
    apply_property(TEST_WEAPON_PATH, weapon_asset, "FireRatePerMinute", 30.0)
    apply_property(TEST_WEAPON_PATH, weapon_asset, "MaxRange", 60000.0)
    apply_property(TEST_WEAPON_PATH, weapon_asset, "MagazineSize", 1)
    apply_property(TEST_WEAPON_PATH, weapon_asset, "AmmoTypeId", "MissileDirectTest")
    if projectile_asset is not None:
        apply_property(TEST_WEAPON_PATH, weapon_asset, "DefaultProjectileData", projectile_asset)
    else:
        record_change(TEST_WEAPON_PATH, "DefaultProjectileData", TEST_PROJECTILE_PATH)

    pattern_config = get_property(weapon_asset, "LauncherFirePatternConfig")
    record_change(TEST_WEAPON_PATH, "LauncherFirePatternConfig.FirePattern", "SingleCycle")
    record_change(TEST_WEAPON_PATH, "LauncherFirePatternConfig.ProjectileCountPerTrigger", 1)
    record_change(TEST_WEAPON_PATH, "LauncherFirePatternConfig.InterMuzzleDelaySeconds", 0.0)
    record_change(TEST_WEAPON_PATH, "LauncherFirePatternConfig.MaximumSimultaneousLaunchCount", 1)
    if APPLY_CHANGES:
        set_property(pattern_config, "FirePattern", enum_value("CFLauncherFirePattern", "SingleCycle"))
        set_property(pattern_config, "ProjectileCountPerTrigger", 1)
        set_property(pattern_config, "InterMuzzleDelaySeconds", 0.0)
        set_property(pattern_config, "MaximumSimultaneousLaunchCount", 1)
        set_property(weapon_asset, "LauncherFirePatternConfig", pattern_config)

    release_config = get_property(weapon_asset, "LauncherReleaseConfig")
    record_change(TEST_WEAPON_PATH, "LauncherReleaseConfig.ReleaseMode", "Direct")
    record_change(TEST_WEAPON_PATH, "LauncherReleaseConfig.CarrierVelocityRatio", 0.0)
    record_change(TEST_WEAPON_PATH, "LauncherReleaseConfig.LauncherClearanceTraceDistanceCm", 150.0)
    if APPLY_CHANGES:
        set_property(release_config, "ReleaseMode", enum_value("CFProjectileReleaseMode", "Direct"))
        set_property(release_config, "LocalEjectionDirection", unreal.Vector(1.0, 0.0, 0.0))
        set_property(release_config, "EjectionSpeed", 0.0)
        set_property(release_config, "CarrierVelocityRatio", 0.0)
        set_property(release_config, "LauncherClearanceTraceDistanceCm", 150.0)
        set_property(weapon_asset, "LauncherReleaseConfig", release_config)


# [v1.0.0] 기존 Rocket 터렛 시각 참조와 신규 미사일 WeaponData를 묶는 테스트 프리셋을 구성합니다.
def configure_preset(preset_asset: Any | None, source_preset: Any, weapon_asset: Any | None) -> None:
    if preset_asset is None:
        record_change(TEST_PRESET_PATH, "Configure", "Missile Direct Equipment Preset")
        return

    apply_property(TEST_PRESET_PATH, preset_asset, "EquipmentId", "Missile_DirectTestKit")
    apply_property(TEST_PRESET_PATH, preset_asset, "DisplayName", "Direct Missile Test Kit")
    apply_property(
        TEST_PRESET_PATH,
        preset_asset,
        "DefaultTurretMountData",
        get_property(source_preset, "DefaultTurretMountData"),
    )
    if weapon_asset is not None:
        apply_property(TEST_PRESET_PATH, preset_asset, "DefaultWeaponData", weapon_asset)
    else:
        record_change(TEST_PRESET_PATH, "DefaultWeaponData", TEST_WEAPON_PATH)


# [v1.0.0] 복제 SUV 첫 MountProfile에 테스트 미사일 프리셋을 연결합니다.
def configure_vehicle(vehicle_asset: Any | None, preset_asset: Any | None) -> None:
    if vehicle_asset is None:
        record_change(TEST_VEHICLE_PATH, "MountProfiles[0].DefaultEquipmentPresetData", TEST_PRESET_PATH)
        return

    mount_profiles = list(get_property(vehicle_asset, "MountProfiles"))
    if len(mount_profiles) < 1:
        raise RuntimeError("Test VehicleData MountProfiles is empty.")

    record_change(TEST_VEHICLE_PATH, "MountProfiles[0].DefaultEquipmentPresetData", TEST_PRESET_PATH)
    if APPLY_CHANGES:
        first_mount_profile = mount_profiles[0]
        set_property(first_mount_profile, "DefaultEquipmentPresetData", preset_asset)
        mount_profiles[0] = first_mount_profile
        set_property(vehicle_asset, "MountProfiles", mount_profiles)


# [v1.0.0] 신규 복제 맵에 플레이 차량과 여섯 시나리오용 경량 타겟을 구성합니다.
def configure_test_map(vehicle_asset: Any | None) -> None:
    if not APPLY_CHANGES:
        record_change(TEST_MAP_PATH, "DuplicateMap", SOURCE_MAP_PATH)
        record_change(TEST_MAP_PATH, "PlaceTargets", "Static,Lateral,Change,Destroy,Overshoot")
        return

    if not unreal.EditorAssetLibrary.does_asset_exist(TEST_MAP_PATH):
        duplicated_map = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP_PATH, TEST_MAP_PATH)
        if duplicated_map is None:
            raise RuntimeError(f"Map duplication failed: {SOURCE_MAP_PATH} -> {TEST_MAP_PATH}")
        REPORT["created_assets"].append(TEST_MAP_PATH)
    else:
        REPORT["updated_assets"].append(TEST_MAP_PATH)

    loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(TEST_MAP_PATH)
    if loaded_world is None:
        raise RuntimeError(f"Test map load failed: {TEST_MAP_PATH}")

    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if actor_subsystem is None or level_subsystem is None:
        raise RuntimeError("Required Editor Actor or Level subsystem is unavailable.")

    target_class = unreal.load_class(None, MISSILE_TARGET_CLASS_PATH)
    player_pawn_class = unreal.load_class(None, PLAYER_PAWN_CLASS_PATH)
    if target_class is None or player_pawn_class is None:
        raise RuntimeError("Missile test target class or player pawn class load failed.")

    all_actors = list(actor_subsystem.get_all_level_actors())
    existing_test_targets = [actor for actor in all_actors if actor and actor.get_class() == target_class]
    for existing_target in existing_test_targets:
        actor_subsystem.destroy_actor(existing_target)

    player_pawns = [actor for actor in all_actors if actor and actor.get_class() == player_pawn_class]
    if player_pawns:
        player_pawn = player_pawns[0]
    else:
        player_pawn = actor_subsystem.spawn_actor_from_class(
            player_pawn_class,
            unreal.Vector(0.0, 0.0, 150.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
    if player_pawn is None:
        raise RuntimeError("Player test pawn spawn failed.")

    set_property(player_pawn, "VehicleData", vehicle_asset)
    try:
        set_property(player_pawn, "auto_possess_player", enum_value("AutoReceiveInput", "Player0"))
    except Exception as error:
        log(f"AutoPossessPlayer assignment warning: {error}")
    player_pawn.set_actor_label("MissileTest_Player")

    target_specs = [
        {
            "label": "MissileTarget_Static",
            "id": "MissileTarget_Static",
            "location": unreal.Vector(8000.0, 0.0, 250.0),
            "movement": "Stationary",
            "auto_destroy": False,
            "block_projectile": True,
        },
        {
            "label": "MissileTarget_Lateral",
            "id": "MissileTarget_Lateral",
            "location": unreal.Vector(12000.0, 2500.0, 250.0),
            "movement": "LateralPingPong",
            "auto_destroy": False,
            "block_projectile": True,
        },
        {
            "label": "MissileTarget_Change",
            "id": "MissileTarget_Change",
            "location": unreal.Vector(10000.0, -3000.0, 250.0),
            "movement": "Stationary",
            "auto_destroy": False,
            "block_projectile": True,
        },
                {
            "label": "MissileTarget_Destroy",
            "id": "MissileTarget_Destroy",
            "location": unreal.Vector(50000.0, 0.0, 250.0),
            "movement": "Stationary",
            "auto_destroy": True,
            "auto_destroy_delay": 5.0,
            "block_projectile": True,
        },
        {
            "label": "MissileTarget_Overshoot",
            "id": "MissileTarget_Overshoot",
            "location": unreal.Vector(3000.0, 4500.0, 250.0),
            "movement": "Stationary",
            "auto_destroy": False,
            "block_projectile": False,
        },
    ]

    for target_spec in target_specs:
        target_actor = actor_subsystem.spawn_actor_from_class(
                        target_class,
            target_spec["location"],
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        if target_actor is None:
            raise RuntimeError(f"Missile test target spawn failed: {target_spec['label']}")
        target_actor.set_actor_label(target_spec["label"])
        set_property(target_actor, "TargetId", target_spec["id"])
        set_property(
            target_actor,
            "MovementMode",
            enum_value("CFMissileTestMoveMode", target_spec["movement"]),
        )
        set_property(target_actor, "LateralMoveDirection", unreal.Vector(0.0, 1.0, 0.0))
        set_property(target_actor, "LateralAmplitudeCm", 2500.0)
        set_property(target_actor, "LateralSpeedCmPerSec", 1200.0)
        set_property(target_actor, "bAutoDestroy", target_spec["auto_destroy"])
        set_property(
            target_actor,
            "AutoDestroyDelaySeconds",
            float(target_spec.get("auto_destroy_delay", 5.0)),
        )
        set_property(target_actor, "bBlockProjectileHits", target_spec["block_projectile"])
        try:
            target_actor.call_method("RefreshTestTargetConfig")
        except Exception:
            try:
                target_actor.refresh_test_target_config()
            except Exception as error:
                log(f"Target refresh warning {target_spec['label']}: {error}")
        REPORT["spawned_targets"].append(target_spec["label"])

    if not level_subsystem.save_current_level():
        raise RuntimeError(f"Test map save failed: {TEST_MAP_PATH}")
    REPORT["saved_assets"].append(TEST_MAP_PATH)


# [v1.0.0] 보호 원본 SHA-256을 수집합니다.
def capture_protected_hashes() -> dict[str, str]:
    return {
        SOURCE_PROJECTILE_PATH: sha256_file(asset_file_path(SOURCE_PROJECTILE_PATH)),
        SOURCE_WEAPON_PATH: sha256_file(asset_file_path(SOURCE_WEAPON_PATH)),
        SOURCE_PRESET_PATH: sha256_file(asset_file_path(SOURCE_PRESET_PATH)),
        SOURCE_VEHICLE_PATH: sha256_file(asset_file_path(SOURCE_VEHICLE_PATH)),
        SOURCE_MAP_PATH: sha256_file(asset_file_path(SOURCE_MAP_PATH)),
    }


# [v1.0.0] 신규 테스트 에셋과 복제 맵을 생성하고 원본 보호 계약을 검증합니다.
def apply_missile_test_assets() -> None:
    source_projectile = require_asset(SOURCE_PROJECTILE_PATH)
    source_weapon = require_asset(SOURCE_WEAPON_PATH)
    source_preset = require_asset(SOURCE_PRESET_PATH)
    source_vehicle = require_asset(SOURCE_VEHICLE_PATH)
    source_map = require_asset(SOURCE_MAP_PATH)
    del source_projectile, source_weapon, source_vehicle, source_map

    REPORT["protected_hashes_before"] = capture_protected_hashes()

    projectile_asset = duplicate_or_load(SOURCE_PROJECTILE_PATH, TEST_PROJECTILE_PATH)
    weapon_asset = duplicate_or_load(SOURCE_WEAPON_PATH, TEST_WEAPON_PATH)
    preset_asset = duplicate_or_load(SOURCE_PRESET_PATH, TEST_PRESET_PATH)
    vehicle_asset = duplicate_or_load(SOURCE_VEHICLE_PATH, TEST_VEHICLE_PATH)

    configure_projectile(projectile_asset)
    configure_weapon(weapon_asset, projectile_asset)
    configure_preset(preset_asset, source_preset, weapon_asset)
    configure_vehicle(vehicle_asset, preset_asset)

    if projectile_asset is not None:
        save_asset(TEST_PROJECTILE_PATH, projectile_asset)
    if weapon_asset is not None:
        save_asset(TEST_WEAPON_PATH, weapon_asset)
    if preset_asset is not None:
        save_asset(TEST_PRESET_PATH, preset_asset)
    if vehicle_asset is not None:
        save_asset(TEST_VEHICLE_PATH, vehicle_asset)

    configure_test_map(vehicle_asset)

    REPORT["protected_hashes_after"] = capture_protected_hashes()
    if REPORT["protected_hashes_before"] != REPORT["protected_hashes_after"]:
        raise RuntimeError("Protected Rocket, Launcher, SUV or TestMap source asset hash changed.")
    REPORT["protected_contract_passed"] = True


try:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    log(f"Mode={REPORT['mode']}")
    apply_missile_test_assets()
    REPORT["success"] = True
except Exception as error:
    REPORT["errors"].append(
        {
            "message": str(error),
            "traceback": traceback.format_exc(),
        }
    )
    unreal.log_error(f"[CarFight][MissileTest] {error}")
finally:
    REPORT_PATH.write_text(
        json.dumps(REPORT, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    log(f"REPORT_JSON_PATH={REPORT_PATH}")

if not REPORT["success"]:
    raise RuntimeError("Missile test asset application failed. Read report.json for details.")
