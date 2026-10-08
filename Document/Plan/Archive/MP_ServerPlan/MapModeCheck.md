# T-004 테스트 맵과 GameMode 확인 결과

- 문서 버전: 0.1.0
- 작성일: 2026-05-26
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`
- 관련 문서: `DSMinRun.md`, `PhaseTasks.md`, `RiskSearch.md`
- 현재 작업: `T-004 테스트 맵과 GameMode 확인`

---

## 1. 목적

이 문서는 Dedicated Server 2클라 테스트에 사용할 맵과 GameMode 상태를 확인한 결과를 기록한다.

현재 단계에서는 코드 작성이나 맵 수정을 하지 않는다.

---

## 2. 확인 대상

이번 작업에서 확인한 대상은 다음과 같다.

```text
- UE/Config/DefaultEngine.ini
- UE/Config/DefaultGame.ini
- UE/CarFight_Re.uproject
- /Game/Maps/TestMap.TestMap
- UE/Source/CarFight_Re C++ 클래스 검색
```

---

## 3. Config 확인 결과

### 3.1 기본 맵

`DefaultEngine.ini`의 GameMapsSettings에서 확인된 값은 다음이다.

```text
GameDefaultMap=/Game/Maps/TestMap.TestMap
EditorStartupMap=/Game/Maps/TestMap.TestMap
```

판정:

```text
기본 맵은 TestMap으로 설정되어 있다.
```

---

### 3.2 기본 GameMode 설정

`DefaultEngine.ini`에서 다음 항목은 확인되지 않았다.

```text
GlobalDefaultGameMode
GlobalDefaultServerGameMode
ServerDefaultMap
TransitionMap
```

판정:

```text
프로젝트 Config 기준으로 명시적인 기본 GameMode 또는 서버 전용 GameMode가 지정되어 있지 않다.
```

---

### 3.3 DefaultGame.ini

`DefaultGame.ini`에는 프로젝트 ID와 CommonUI 설정만 확인되었다.

```text
ProjectID=E2CD9FBC4939B5318BEB71A9E2210655
CommonButtonAcceptKeyHandling=TriggerClick
```

판정:

```text
DefaultGame.ini에도 GameMode 관련 설정은 확인되지 않았다.
```

---

## 4. 프로젝트 파일 확인 결과

`UE/CarFight_Re.uproject`에서 확인된 내용은 다음이다.

```text
Module: CarFight_Re
Type: Runtime
EngineAssociation: Source Build GUID
Plugins:
- ModelingToolsEditorMode
- VisualStudioTools
- ChaosModularVehicle
- ChaosVehiclesPlugin
```

판정:

```text
Source Build 전환은 프로젝트 파일에 반영되어 있다.
uproject에는 GameMode 또는 PlayerController 설정이 없다.
```

---

## 5. C++ 클래스 검색 결과

### 5.1 GameMode

검색 결과:

```text
UE/Source/CarFight_Re 내 GameMode 검색 결과: 0건
```

판정:

```text
커스텀 GameMode C++ 클래스는 현재 확인되지 않았다.
```

---

### 5.2 PlayerController

검색 결과:

```text
커스텀 PlayerController 클래스는 확인되지 않았다.
APlayerController 참조는 ACFVehiclePawn과 Debug UI 내부 사용으로 확인됐다.
```

주요 사용 위치:

```text
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- RegisterDefaultInputMappingContext
- CreateAimReticleWidget
- 입력 상태 확인

UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
- Debug Panel 입력 모드 전환
- 동적 Debug Widget 생성
```

판정:

```text
커스텀 PlayerController C++ 클래스는 현재 확인되지 않았다.
```

---

## 6. TestMap 확인 결과

`/Game/Maps/TestMap.TestMap` 덤프 결과는 다음이다.

```text
Actor 수: 14
PlayerStart: PlayerStart_0 1개
배치 차량: BP_CFVehiclePawn_C_1 1대
```

Actor 목록:

```text
WorldSettings_1
Brush_1
DirectionalLight_0
SkyAtmosphere_0
SkyLight_0
ExponentialHeightFog_0
VolumetricCloud_0
PlayerStart_0
StaticMeshActor_0
Floor_0
StaticMeshActor_1
Floor_1
StaticMeshActor_2
BP_CFVehiclePawn_C_1
```

판정:

```text
현재 TestMap은 싱글플레이 차량 테스트 맵 성격이 강하다.
Dedicated Server 2클라 테스트에는 PlayerStart와 차량 수가 부족할 가능성이 높다.
```

---

## 7. GameMode Override 확인 한계

`ue.dump_asset_details_safe`로 `/Game/Maps/TestMap.TestMap`을 확인했으나, 안전 요약에는 World 기본 정보만 노출되었다.

확인된 내용:

```text
class_name: World
component_count: 0
class_default_count: 8
```

한계:

```text
WorldSettings의 GameMode Override 값은 안전 요약에서 확인되지 않았다.
```

판정:

```text
현재 도구 결과만으로는 TestMap WorldSettings의 GameMode Override를 확정할 수 없다.
다만 Config와 Source 기준으로는 커스텀 GameMode 지정 근거가 없다.
```

---

## 8. T-004 판정

현재 확인 결과를 바탕으로 `T-004 테스트 맵과 GameMode 확인`의 판정은 다음이다.

```text
테스트 맵:
- /Game/Maps/TestMap.TestMap 사용 중

PlayerStart:
- 1개 확인
- 2클라 테스트에는 부족 가능성 높음

배치 차량:
- BP_CFVehiclePawn 1대 확인
- 2클라 차량 소유권 테스트에는 부족 가능성 높음

기본 GameMode:
- Config에서 명시 설정 없음
- C++ 커스텀 GameMode 클래스 없음
- TestMap WorldSettings Override는 도구 요약으로 확정 불가

DefaultPawnClass:
- Config에서 명시 설정 없음
- WorldSettings Override 확인 필요

PlayerControllerClass:
- Config에서 명시 설정 없음
- 커스텀 PlayerController C++ 클래스 없음
```

---

## 9. 코드 작성 또는 에디터 설정 변경 단계 도달 여부

현재 상태는 다음 중 하나를 선택해야 하는 지점에 가깝다.

```text
1. TestMap을 멀티 테스트용으로 수정한다.
2. 별도 Dedicated Server 테스트 맵을 만든다.
3. GameMode/PlayerController/Spawn-Possess 구조를 코드로 만든다.
4. 우선 현재 맵 그대로 2클라 접속을 시도해 실패 양상을 기록한다.
```

위 1~3번은 에디터 설정 변경 또는 코드 작성 단계로 이어진다.

사용자 지시에 따라 현재 작업에서는 코드 작성과 맵 수정을 진행하지 않는다.

---

## 10. 추천 다음 액션

가장 안전한 다음 액션은 다음이다.

```text
1. 현재 TestMap 그대로 Dedicated Server + 클라이언트 2개 접속을 시도한다.
2. 실제 실패 양상을 기록한다.
3. 실패가 확인되면 TestMap 수정 또는 GameMode/Spawn 구조 작성 중 어느 방향으로 갈지 결정한다.
```

이유:

```text
현재 설정만 보면 2클라 테스트에 부족할 가능성이 높지만,
실제 실패 양상을 보기 전에는 코드 작성 범위를 확정하기 어렵다.
```

---

## 11. 변경 기록

### 0.1.0

```text
- T-004 테스트 맵과 GameMode 확인 결과 최초 작성
- DefaultEngine.ini / DefaultGame.ini / uproject 확인 결과 기록
- TestMap Actor 구성 기록
- 커스텀 GameMode / PlayerController 미확인 상태 기록
- 코드 작성 또는 에디터 설정 변경 중단 지점 기록
```
