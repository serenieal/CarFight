# File: SetVehicleWheelDefaults.py
# Version: v1.0.0
# Changelog:
# - v1.0.0: DA_PoliceCar 의도와 불일치한 BP_Wheel_Front/BP_Wheel_Rear 기본값만 동기화.
# Migration:
# - BP_Wheel_Front WheelRadius를 40.0, bAffectedByEngine을 False로 저장한다.
# - BP_Wheel_Rear WheelRadius를 40.0으로 저장한다.
# - 조향각, 마찰, 코너링 강성, 서스펜션, 입력, RepMove 설정은 변경하지 않는다.

import unreal


# 앞바퀴 Blueprint 에셋 경로입니다.
FRONT_WHEEL_BLUEPRINT_ASSET_PATH = "/Game/CarFight/Vehicles/BP_Wheel_Front"


# 앞바퀴 Blueprint GeneratedClass 경로입니다.
FRONT_WHEEL_BLUEPRINT_CLASS_PATH = "/Game/CarFight/Vehicles/BP_Wheel_Front.BP_Wheel_Front_C"


# 뒷바퀴 Blueprint 에셋 경로입니다.
REAR_WHEEL_BLUEPRINT_ASSET_PATH = "/Game/CarFight/Vehicles/BP_Wheel_Rear"


# 뒷바퀴 Blueprint GeneratedClass 경로입니다.
REAR_WHEEL_BLUEPRINT_CLASS_PATH = "/Game/CarFight/Vehicles/BP_Wheel_Rear.BP_Wheel_Rear_C"


# 휠 반지름 프로퍼티 후보 이름입니다.
WHEEL_RADIUS_PROPERTY_NAMES = ("WheelRadius", "wheel_radius")


# 엔진 구동력 적용 여부 프로퍼티 후보 이름입니다.
WHEEL_AFFECTED_BY_ENGINE_PROPERTY_NAMES = ("bAffectedByEngine", "b_affected_by_engine", "affected_by_engine")


# DA_PoliceCar 기준 휠 반지름입니다.
TARGET_WHEEL_RADIUS = 40.0


# DA_PoliceCar 기준 앞바퀴 엔진 구동력 적용 여부입니다.
TARGET_FRONT_AFFECTED_BY_ENGINE = False


# DA_PoliceCar 기준 뒷바퀴 엔진 구동력 적용 여부입니다.
TARGET_REAR_AFFECTED_BY_ENGINE = True


# 커맨드렛 콘솔과 UE 로그에 일반 정보를 함께 출력합니다.
def emit_info(message):
    print(message)
    unreal.log(message)


# 커맨드렛 콘솔과 UE 로그에 오류 정보를 함께 출력합니다.
def emit_error(message):
    print(message)
    unreal.log_error(message)


# 명시적인 GeneratedClass 경로를 로드해 클래스 기본 오브젝트를 반환합니다.
def get_blueprint_class_default_object(blueprint_class_path):
    # generated_class는 Blueprint가 생성한 런타임 클래스입니다.
    generated_class = unreal.load_class(None, blueprint_class_path)
    if generated_class is None:
        emit_error("[VehicleWheelDefaults] Failed to load class: {0}".format(blueprint_class_path))
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


# 실수 저장 오차를 고려해 두 값을 비교합니다.
def are_property_values_equal(current_value, expected_value):
    if isinstance(current_value, float) or isinstance(expected_value, float):
        try:
            # 실수 비교에 사용할 허용 오차입니다.
            float_tolerance = 0.0001
            return abs(float(current_value) - float(expected_value)) <= float_tolerance
        except Exception:
            return current_value == expected_value

    return current_value == expected_value


# 지정 프로퍼티를 목표값과 동기화합니다.
def synchronize_property(target_object, label, property_names, target_value):
    # 현재 읽기에 성공한 프로퍼티 이름과 값입니다.
    property_name, current_value = try_get_editor_property(target_object, property_names)
    if property_name is None:
        emit_error("[VehicleWheelDefaults] {0} property was not found. Checked={1}".format(label, ", ".join(property_names)))
        return False, False

    if are_property_values_equal(current_value, target_value):
        emit_info("[VehicleWheelDefaults] {0} {1} already {2}".format(label, property_name, target_value))
        return True, False

    # 쓰기에 성공한 프로퍼티 이름입니다.
    changed_property_name = try_set_editor_property(target_object, property_names, target_value)
    if changed_property_name is None:
        emit_error("[VehicleWheelDefaults] Failed to set {0}. Checked={1}".format(label, ", ".join(property_names)))
        return False, False

    emit_info("[VehicleWheelDefaults] {0} {1}: {2} -> {3}".format(label, changed_property_name, current_value, target_value))
    return True, True


# Blueprint를 컴파일하고 저장합니다.
def compile_and_save_blueprint(blueprint_asset_path):
    # 저장할 Blueprint 에셋입니다.
    blueprint_asset = unreal.EditorAssetLibrary.load_asset(blueprint_asset_path)
    if blueprint_asset is None:
        emit_error("[VehicleWheelDefaults] Failed to load blueprint asset: {0}".format(blueprint_asset_path))
        return False

    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint_asset)
    except Exception as compile_error:
        unreal.log_warning("[VehicleWheelDefaults] Compile skipped or failed: {0}".format(compile_error))

    # Blueprint 에셋 저장 결과입니다.
    save_result = unreal.EditorAssetLibrary.save_asset(blueprint_asset_path, only_if_is_dirty=False)
    if not save_result:
        emit_error("[VehicleWheelDefaults] Failed to save blueprint asset: {0}".format(blueprint_asset_path))
        return False

    emit_info("[VehicleWheelDefaults] Saved blueprint asset: {0}".format(blueprint_asset_path))
    return True


# 단일 Wheel Blueprint 기본값을 동기화합니다.
def synchronize_wheel_blueprint(blueprint_asset_path, blueprint_class_path, label, target_radius, target_affected_by_engine):
    # 휠 Blueprint GeneratedClass의 클래스 기본 오브젝트입니다.
    class_default_object = get_blueprint_class_default_object(blueprint_class_path)
    if class_default_object is None:
        return False

    # 현재 휠 Blueprint에서 실제로 변경된 프로퍼티 개수입니다.
    changed_count = 0

    # 휠 반지름 동기화 성공 여부와 변경 여부입니다.
    radius_ok, radius_changed = synchronize_property(
        class_default_object,
        "{0} WheelRadius".format(label),
        WHEEL_RADIUS_PROPERTY_NAMES,
        target_radius)
    if not radius_ok:
        return False
    if radius_changed:
        changed_count += 1

    # 엔진 구동력 적용 여부 동기화 성공 여부와 변경 여부입니다.
    affected_by_engine_ok, affected_by_engine_changed = synchronize_property(
        class_default_object,
        "{0} bAffectedByEngine".format(label),
        WHEEL_AFFECTED_BY_ENGINE_PROPERTY_NAMES,
        target_affected_by_engine)
    if not affected_by_engine_ok:
        return False
    if affected_by_engine_changed:
        changed_count += 1

    if not compile_and_save_blueprint(blueprint_asset_path):
        return False

    emit_info("[VehicleWheelDefaults] {0} synchronized. Changed={1}".format(label, changed_count))
    return True


# 스크립트 진입점입니다.
def main():
    # 앞바퀴 Blueprint 동기화 성공 여부입니다.
    front_ok = synchronize_wheel_blueprint(
        FRONT_WHEEL_BLUEPRINT_ASSET_PATH,
        FRONT_WHEEL_BLUEPRINT_CLASS_PATH,
        "FrontWheel",
        TARGET_WHEEL_RADIUS,
        TARGET_FRONT_AFFECTED_BY_ENGINE)

    # 뒷바퀴 Blueprint 동기화 성공 여부입니다.
    rear_ok = synchronize_wheel_blueprint(
        REAR_WHEEL_BLUEPRINT_ASSET_PATH,
        REAR_WHEEL_BLUEPRINT_CLASS_PATH,
        "RearWheel",
        TARGET_WHEEL_RADIUS,
        TARGET_REAR_AFFECTED_BY_ENGINE)

    if not front_ok or not rear_ok:
        raise RuntimeError("[VehicleWheelDefaults] Wheel defaults synchronization failed.")

    emit_info("[VehicleWheelDefaults] Wheel defaults synchronization complete.")


main()
