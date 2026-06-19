# File: SetVehicleSingleDefaults.py
# Version: v1.1.0
# Changelog:
# - v1.1.0: 제거된 멀티플레이 네트워크 진단 플래그를 기본값 동기화 목록에서 제외합니다.
# - v1.0.0: BP_CFVehiclePawn의 싱글플레이 기본 Actor/물리/디버그 값을 저장합니다.

import unreal


# 대상 차량 블루프린트 에셋 경로입니다.
VEHICLE_BLUEPRINT_ASSET_PATH = "/Game/CarFight/Vehicles/BP_CFVehiclePawn"


# 대상 차량 블루프린트 GeneratedClass 경로입니다.
VEHICLE_BLUEPRINT_CLASS_PATH = "/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn_C"


# Actor 복제 관련 싱글플레이 기본값 목록입니다.
ACTOR_SINGLE_DEFAULTS = (
    (("replicates", "b_replicates", "bReplicates"), False),
    (("replicate_movement", "b_replicate_movement", "bReplicateMovement", "ReplicateMovement"), False),
)


# 시각 실험 플래그 싱글플레이 기본값 목록입니다.
EXPERIMENT_FLAG_DEFAULTS = (
    (("bEnableOwnerVisualStabilization", "b_enable_owner_visual_stabilization", "enable_owner_visual_stabilization"), False),
    (("bEnableOwnerBodyVisualStabilization", "b_enable_owner_body_visual_stabilization", "enable_owner_body_visual_stabilization"), False),
    (("bHideOwnerPhysicsMeshWhenStabilized", "b_hide_owner_physics_mesh_when_stabilized", "hide_owner_physics_mesh_when_stabilized"), False),
)


# VehicleMesh 후보 프로퍼티 이름 목록입니다.
MESH_PROPERTY_NAMES = ("mesh", "Mesh", "vehicle_mesh", "VehicleMesh")


# VehicleMesh 후보 컴포넌트 이름 목록입니다.
MESH_COMPONENT_NAMES = ("VehicleMesh", "Mesh")


# Autonomous Proxy 물리 복제 후보 프로퍼티 이름 목록입니다.
AUTO_PHYSICS_PROPERTY_NAMES = (
    "replicate_physics_to_autonomous_proxy",
    "b_replicate_physics_to_autonomous_proxy",
    "bReplicatePhysicsToAutonomousProxy",
)


# 콘솔과 Unreal 로그에 일반 정보를 함께 출력합니다.
def emit_info(message):
    print(message)
    unreal.log(message)


# 콘솔과 Unreal 로그에 오류 정보를 함께 출력합니다.
def emit_error(message):
    print(message)
    unreal.log_error(message)


# 명시적인 GeneratedClass 경로를 로드해 클래스 기본 오브젝트를 반환합니다.
def get_blueprint_class_default_object():
    # 차량 Blueprint GeneratedClass입니다.
    generated_class = unreal.load_class(None, VEHICLE_BLUEPRINT_CLASS_PATH)
    if generated_class is None:
        emit_error("[VehicleSingleDefaults] Failed to load class: {0}".format(VEHICLE_BLUEPRINT_CLASS_PATH))
        return None

    return unreal.get_default_object(generated_class)


# 후보 프로퍼티 이름 목록으로 UObject 프로퍼티를 읽습니다.
def try_get_editor_property(target_object, property_names):
    for property_name in property_names:
        try:
            # 현재 후보 이름으로 읽은 프로퍼티 값입니다.
            property_value = target_object.get_editor_property(property_name)
            return property_name, property_value
        except Exception:
            continue

    return None, None


# 후보 프로퍼티 이름 목록으로 UObject 프로퍼티를 씁니다.
def try_set_editor_property(target_object, property_names, property_value):
    for property_name in property_names:
        try:
            target_object.set_editor_property(property_name, property_value)
            return property_name
        except Exception:
            continue

    return None


# 현재 값과 기대 값이 같은지 확인합니다.
def are_property_values_equal(current_value, expected_value):
    if isinstance(current_value, float) or isinstance(expected_value, float):
        try:
            # 실수 비교에 사용할 허용 오차입니다.
            float_tolerance = 0.0001
            return abs(float(current_value) - float(expected_value)) <= float_tolerance
        except Exception:
            return current_value == expected_value

    return current_value == expected_value


# 지정한 기본값 목록을 CDO에 적용합니다.
def apply_default_group(class_default_object, group_name, default_items):
    # 변경된 프로퍼티 개수입니다.
    changed_count = 0

    for property_names, target_value in default_items:
        # 현재 읽기에 성공한 프로퍼티 이름과 값입니다.
        property_name, current_value = try_get_editor_property(class_default_object, property_names)
        if property_name is None:
            emit_error("[VehicleSingleDefaults] {0} property was not found. Checked={1}".format(group_name, ", ".join(property_names)))
            return False

        if are_property_values_equal(current_value, target_value):
            emit_info("[VehicleSingleDefaults] {0} {1} already {2}".format(group_name, property_name, target_value))
            continue

        # 쓰기에 성공한 프로퍼티 이름입니다.
        changed_property_name = try_set_editor_property(class_default_object, property_names, target_value)
        if changed_property_name is None:
            emit_error("[VehicleSingleDefaults] Failed to set {0} property. Checked={1}".format(group_name, ", ".join(property_names)))
            return False

        emit_info("[VehicleSingleDefaults] {0} {1}: {2} -> {3}".format(group_name, changed_property_name, current_value, target_value))
        changed_count += 1

    emit_info("[VehicleSingleDefaults] {0} synchronized. Changed={1}".format(group_name, changed_count))
    return True


# CDO 또는 Blueprint 컴포넌트 목록에서 VehicleMesh 컴포넌트를 찾습니다.
def find_vehicle_mesh_component(class_default_object, blueprint_asset):
    # CDO에서 직접 읽은 Mesh 컴포넌트입니다.
    for property_name in MESH_PROPERTY_NAMES:
        try:
            # 현재 후보 프로퍼티로 읽은 컴포넌트입니다.
            mesh_component = class_default_object.get_editor_property(property_name)
            if mesh_component is not None:
                return mesh_component
        except Exception:
            continue

    try:
        # Blueprint 에셋에서 조회한 모든 컴포넌트입니다.
        blueprint_components = unreal.SubobjectDataSubsystem.get().k2_gather_subobject_data_for_blueprint(blueprint_asset)
    except Exception:
        return None

    for subobject_data in blueprint_components:
        try:
            # 현재 SubobjectData가 가리키는 객체입니다.
            subobject = unreal.SubobjectDataBlueprintFunctionLibrary.get_object(subobject_data)
        except Exception:
            subobject = None

        if subobject is None:
            continue

        # 현재 Subobject 이름입니다.
        subobject_name = subobject.get_name()
        if subobject_name in MESH_COMPONENT_NAMES:
            return subobject

    return None


# VehicleMesh의 Autonomous Proxy 물리 복제 기본값을 싱글플레이 기준으로 끕니다.
def apply_vehicle_mesh_single_defaults(class_default_object, blueprint_asset):
    # 싱글플레이 기준으로 저장할 VehicleMesh 컴포넌트입니다.
    vehicle_mesh_component = find_vehicle_mesh_component(class_default_object, blueprint_asset)
    if vehicle_mesh_component is None:
        emit_error("[VehicleSingleDefaults] VehicleMesh component was not found.")
        return False

    # 현재 읽기에 성공한 프로퍼티 이름과 값입니다.
    property_name, current_value = try_get_editor_property(vehicle_mesh_component, AUTO_PHYSICS_PROPERTY_NAMES)
    if property_name is None:
        emit_info("[VehicleSingleDefaults] VehicleMesh AutoPhysics property was not found. Skipped.")
        return True

    if are_property_values_equal(current_value, False):
        emit_info("[VehicleSingleDefaults] VehicleMesh {0} already False".format(property_name))
        return True

    # 쓰기에 성공한 프로퍼티 이름입니다.
    changed_property_name = try_set_editor_property(vehicle_mesh_component, AUTO_PHYSICS_PROPERTY_NAMES, False)
    if changed_property_name is None:
        emit_error("[VehicleSingleDefaults] Failed to set VehicleMesh AutoPhysics property.")
        return False

    emit_info("[VehicleSingleDefaults] VehicleMesh {0}: {1} -> False".format(changed_property_name, current_value))
    return True


# BP_CFVehiclePawn을 컴파일하고 저장합니다.
def compile_and_save_blueprint(blueprint_asset):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint_asset)

    # Blueprint 저장 성공 여부입니다.
    save_result = unreal.EditorAssetLibrary.save_asset(VEHICLE_BLUEPRINT_ASSET_PATH, only_if_is_dirty=False)
    if not save_result:
        emit_error("[VehicleSingleDefaults] Failed to save asset: {0}".format(VEHICLE_BLUEPRINT_ASSET_PATH))
        return False

    emit_info("[VehicleSingleDefaults] Saved asset: {0}".format(VEHICLE_BLUEPRINT_ASSET_PATH))
    return True


# 싱글플레이 차량 BP 기본값 동기화 작업의 진입점입니다.
def main():
    emit_info("[VehicleSingleDefaults] Synchronizing BP_CFVehiclePawn with single-player defaults v1.1.0.")

    # 대상 Blueprint 에셋입니다.
    blueprint_asset = unreal.EditorAssetLibrary.load_asset(VEHICLE_BLUEPRINT_ASSET_PATH)
    if blueprint_asset is None:
        emit_error("[VehicleSingleDefaults] Failed to load asset: {0}".format(VEHICLE_BLUEPRINT_ASSET_PATH))
        return 1

    # 대상 Blueprint 클래스 기본 오브젝트입니다.
    class_default_object = get_blueprint_class_default_object()
    if class_default_object is None:
        return 1

    if not apply_default_group(class_default_object, "ActorSingle", ACTOR_SINGLE_DEFAULTS):
        return 1

    if not apply_default_group(class_default_object, "ExperimentFlags", EXPERIMENT_FLAG_DEFAULTS):
        return 1

    if not apply_vehicle_mesh_single_defaults(class_default_object, blueprint_asset):
        return 1

    if not compile_and_save_blueprint(blueprint_asset):
        return 1

    emit_info("[VehicleSingleDefaults] Completed successfully.")
    return 0


# Unreal Python Commandlet에서 실패를 빌드 실패로 전달합니다.
commandlet_result = main()
if commandlet_result != 0:
    raise RuntimeError("SetVehicleSingleDefaults failed with code {0}".format(commandlet_result))
