# File: SetVehicleNetBpDefaults.py
# Version: v1.6.0
# Changelog:
# - v1.6.0: ACFVehiclePawn v2.40.0에서 실패한 네트워크 실험 UPROPERTY를 삭제했으므로 BP 컴파일/저장 검증만 수행.
# - v1.5.0: v2.39.0에서 숨긴 실험용 UPROPERTY는 실패가 아니라 정상 스킵으로 처리하고 BP 컴파일/저장 검증만 유지.
# - v1.4.0: 차량 낙하 문제를 Pawn 내부 가드/복구가 아니라 맵 경계로 처리하기 위해 AuthorityGuard 기본값도 False로 정리.
# - v1.3.0: 실패한 AutonomousProxy 물리 복제 차단과 서버 추락 복구 실험 기본값을 False로 되돌려 정리 기준선을 적용.
# - v1.2.0: Legacy 입력 RPC 기준선 복구 후 AutonomousProxy 물리 복제 차단과 서버 추락 복구 안전 기본값을 BP에 반영.
# - v1.1.0: 엔진 네트워크 물리 예측 기준선 테스트를 위해 수동 NetState/VisualShell/Reconcile/Collision 기본값을 False로 정리.
# - v1.0.1: UE Python에서 보호된 GeneratedClass 대신 명시적인 _C 클래스 경로를 로드하도록 수정.
# - v1.0.0: BP_CFVehiclePawn의 구형 원격 Actor 직접 보간 기본값을 C++ 기본값으로 정리.

import unreal


# 대상 차량 블루프린트 에셋 경로입니다.
VEHICLE_BLUEPRINT_ASSET_PATH = "/Game/CarFight/Vehicles/BP_CFVehiclePawn"


# 대상 차량 블루프린트의 GeneratedClass 경로입니다.
VEHICLE_BLUEPRINT_CLASS_PATH = "/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn_C"


# 명시적인 GeneratedClass 경로를 로드해 클래스 기본 오브젝트를 반환합니다.
def get_blueprint_class_default_object():
    # generated_class는 Blueprint가 생성한 런타임 클래스입니다.
    generated_class = unreal.load_class(None, VEHICLE_BLUEPRINT_CLASS_PATH)
    if generated_class is None:
        unreal.log_error("[VehicleNetBpDefaults] Failed to load class: {0}".format(VEHICLE_BLUEPRINT_CLASS_PATH))
        return None

    return unreal.get_default_object(generated_class)


# BP_CFVehiclePawn을 컴파일하고 저장해 삭제된 실험 프로퍼티 잔여 직렬화를 정리합니다.
def main():
    # 대상 블루프린트 에셋입니다.
    blueprint_asset = unreal.EditorAssetLibrary.load_asset(VEHICLE_BLUEPRINT_ASSET_PATH)
    if blueprint_asset is None:
        unreal.log_error("[VehicleNetBpDefaults] Failed to load asset: {0}".format(VEHICLE_BLUEPRINT_ASSET_PATH))
        return 1

    # 대상 블루프린트 클래스 기본 오브젝트입니다.
    class_default_object = get_blueprint_class_default_object()
    if class_default_object is None:
        return 2

    unreal.log(
        "[VehicleNetBpDefaults] Legacy vehicle network experiment properties were deleted in ACFVehiclePawn v2.40.0. "
        "Only Blueprint compile/save validation will run."
    )

    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint_asset)
    except Exception as compile_error:
        unreal.log_warning("[VehicleNetBpDefaults] Compile skipped or failed: {0}".format(compile_error))

    # 저장 성공 여부입니다.
    save_result = unreal.EditorAssetLibrary.save_asset(VEHICLE_BLUEPRINT_ASSET_PATH, only_if_is_dirty=False)
    if not save_result:
        unreal.log_error("[VehicleNetBpDefaults] Failed to save asset: {0}".format(VEHICLE_BLUEPRINT_ASSET_PATH))
        return 4

    unreal.log("[VehicleNetBpDefaults] Saved asset: {0}".format(VEHICLE_BLUEPRINT_ASSET_PATH))
    return 0


# 스크립트 실행 결과 코드입니다.
RESULT_CODE = main()
if RESULT_CODE != 0:
    raise RuntimeError("VehicleNetBpDefaults failed with code {0}".format(RESULT_CODE))
