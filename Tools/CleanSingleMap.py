# File: CleanSingleMap.py
# Version: v1.0.0
# Changelog:
# - v1.0.0: 싱글플레이 전환에 맞춰 TestMap 안의 CFNetSmooth 테스트 Actor 잔여물을 제거하고 맵을 다시 저장합니다.

import unreal


# 정리할 기본 테스트 맵 에셋 경로입니다.
TEST_MAP_ASSET_PATH = "/Game/Maps/TestMap"


# 제거 대상 Actor 이름/클래스 판정에 사용할 문자열입니다.
REMOVED_ACTOR_KEYWORDS = ("CFNetSmooth", "NetSmooth")


# 콘솔과 Unreal 로그에 일반 정보를 함께 출력합니다.
def emit_info(message):
    print(message)
    unreal.log(message)


# 콘솔과 Unreal 로그에 오류 정보를 함께 출력합니다.
def emit_error(message):
    print(message)
    unreal.log_error(message)


# 지정한 맵을 에디터 월드로 로드합니다.
def load_test_map():
    try:
        # 로드된 테스트 맵의 월드 객체입니다.
        loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(TEST_MAP_ASSET_PATH)
        return loaded_world
    except Exception as exception:
        emit_error("[CleanSingleMap] Failed to load map {0}: {1}".format(TEST_MAP_ASSET_PATH, exception))
        return None


# 현재 열린 에디터 월드의 모든 Actor를 반환합니다.
def get_all_level_actors():
    try:
        # Level Editor Subsystem 기반 Actor 조회 결과입니다.
        level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if level_editor_subsystem is not None and hasattr(level_editor_subsystem, "get_all_level_actors"):
            return list(level_editor_subsystem.get_all_level_actors())
    except Exception:
        pass

    try:
        # 레거시 EditorLevelLibrary 기반 Actor 조회 결과입니다.
        return list(unreal.EditorLevelLibrary.get_all_level_actors())
    except Exception as exception:
        emit_error("[CleanSingleMap] Failed to enumerate level actors: {0}".format(exception))
        return []


# Actor가 CFNetSmooth 테스트 잔여물인지 확인합니다.
def should_remove_actor(actor):
    if actor is None:
        return False

    # Actor 이름입니다.
    actor_name = actor.get_name()

    # Actor 클래스 경로입니다.
    actor_class_path = str(actor.get_class().get_path_name()) if actor.get_class() is not None else ""

    # Actor 이름과 클래스 경로를 합친 검색 문자열입니다.
    search_text = "{0} {1}".format(actor_name, actor_class_path)

    return any(keyword in search_text for keyword in REMOVED_ACTOR_KEYWORDS)


# CFNetSmooth 테스트 Actor 잔여물을 제거합니다.
def remove_netsmooth_test_actors():
    # 현재 레벨의 모든 Actor입니다.
    level_actors = get_all_level_actors()

    # 제거한 Actor 개수입니다.
    removed_count = 0

    for actor in level_actors:
        if not should_remove_actor(actor):
            continue

        # 제거 대상 Actor 이름입니다.
        actor_name = actor.get_name()
        emit_info("[CleanSingleMap] Removing actor: {0}".format(actor_name))
        unreal.EditorLevelLibrary.destroy_actor(actor)
        removed_count += 1

    emit_info("[CleanSingleMap] Removed actors: {0}".format(removed_count))
    return removed_count


# 현재 테스트 맵을 저장합니다.
def save_test_map():
    try:
        # 테스트 맵 저장 성공 여부입니다.
        save_result = unreal.EditorAssetLibrary.save_asset(TEST_MAP_ASSET_PATH, only_if_is_dirty=False)
        if not save_result:
            emit_error("[CleanSingleMap] Failed to save map: {0}".format(TEST_MAP_ASSET_PATH))
            return False

        emit_info("[CleanSingleMap] Saved map: {0}".format(TEST_MAP_ASSET_PATH))
        return True
    except Exception as exception:
        emit_error("[CleanSingleMap] Failed to save map {0}: {1}".format(TEST_MAP_ASSET_PATH, exception))
        return False


# 싱글플레이 맵 정리 작업의 진입점입니다.
def main():
    emit_info("[CleanSingleMap] Cleaning TestMap for single-player rollback v1.0.0.")

    # 로드된 테스트 맵 월드입니다.
    loaded_world = load_test_map()
    if loaded_world is None:
        return 1

    remove_netsmooth_test_actors()

    if not save_test_map():
        return 1

    emit_info("[CleanSingleMap] Completed successfully.")
    return 0


# Unreal Python Commandlet에서 실패를 빌드 실패로 전달합니다.
commandlet_result = main()
if commandlet_result != 0:
    raise RuntimeError("CleanSingleMap failed with code {0}".format(commandlet_result))
