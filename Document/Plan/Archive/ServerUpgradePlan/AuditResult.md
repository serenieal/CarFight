# 서버 고도화 현재상태 감사 결과

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 상위 문서: `CurrentState.md`, `ServerDesign.md`, `Roadmap.md`
- 상태: Active

---

## 1. 목적

이 문서는 서버 고도화 첫 코드 작업에 들어가기 전에 현재 상태를 확정하기 위한 감사 결과다.

목표는 다음 두 가지다.

```text
1. 현재 서버/클라이언트 상태를 추정이 아니라 확인 결과로 고정한다.
2. 첫 코드 작업이 왜 ACFMPGameMode인지 근거를 남긴다.
```

---

## 2. 감사 대상

이번 감사에서 확인한 대상은 다음이다.

```text
- UE/Source 구조
- Dedicated Server Target
- Build.cs 의존성
- DefaultEngine.ini
- GameMode / DefaultPawn 설정 존재 여부
- ACFVehiclePawn의 AutoPossess / 입력 / UI guard 상태
- UCFVehicleCameraComp의 Dedicated Server guard 상태
- UCFVehicleAimComp의 복제/서버 guard 상태
- TestMap Actor 구성
- BP_CFVehiclePawn 에셋 요약
```

---

## 3. 확정 결과 요약

| 항목 | 결과 | 판단 |
|---|---:|---|
| Server Target | 존재 | 서버 빌드 기반 있음 |
| Dedicated Server 설정 | `TargetType.Server` 확인 | Live Coding 제외 설정 있음 |
| 전용 GameMode | 없음 | 첫 코드 작업 필요 |
| DefaultPawn 설정 | 없음 | 서버 Spawn/Possess 구조 필요 |
| DefaultEngine 맵 설정 | 있음 | TestMap만 지정됨 |
| GlobalDefaultGameMode | 없음 | Config 작업 필요 |
| GlobalDefaultServerGameMode | 없음 | Config 작업 필요 |
| TestMap PlayerStart | 1개 | 2클라 테스트에 부족 |
| TestMap 배치 차량 | 1대 | 서버 Spawn 구조와 충돌 가능 |
| ACFVehiclePawn AutoPossess | Disabled | 서버 Possess 구조에 유리 |
| ACFVehiclePawn 입력 guard | 있음 | DS/비소유 Pawn 입력 차단 있음 |
| ACFVehiclePawn Reticle guard | 있음 | DS UI 생성 차단 일부 있음 |
| ACFVehiclePawn 복제 설정 | C++ 강제 설정 없음 | BP 기본값 재확인 필요 |
| UCFVehicleCameraComp DS guard | 부족 | Phase 7에서 정리 필요 |
| UCFVehicleAimComp 복제 | 일부 있음 | AimVisualState 복제 골격 존재 |

---

## 4. Source / Target 상태

### 4.1 서버 타깃

`UE/Source/CarFight_ReServer.Target.cs`가 존재한다.

확인된 내용:

```text
- Type = TargetType.Server
- DefaultBuildSettings = BuildSettingsVersion.V6
- IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7
- bWithLiveCoding = false
- ExtraModuleNames.Add("CarFight_Re")
```

판단:

```text
서버 Target은 이미 존재한다.
따라서 현재 문제는 서버 Target 부재가 아니라 런타임 멀티플레이 구조 부재다.
```

---

## 5. Config 상태

`UE/Config/DefaultEngine.ini`의 `GameMapsSettings`에는 현재 맵 설정만 있다.

현재 확인된 설정:

```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/Maps/TestMap.TestMap
EditorStartupMap=/Game/Maps/TestMap.TestMap
```

없는 설정:

```text
- GlobalDefaultGameMode
- GlobalDefaultServerGameMode
- DefaultPawnClass 관련 C++/Config 설정
```

판단:

```text
Dedicated Server는 TestMap을 로드할 수 있지만,
접속한 PlayerController에게 어떤 Pawn을 줄지 정하는 프로젝트 기준이 없다.
```

---

## 6. GameMode / PlayerController 상태

소스와 Config 검색 기준으로 `GameMode` 관련 직접 구현은 없다.

확인 결과:

```text
- ACFMPGameMode 없음
- 프로젝트 전용 GameMode 없음
- 프로젝트 전용 PlayerController 없음
- DefaultPawnClass 설정 없음
```

판단:

```text
현재 Dedicated Server 접속 후 검정 화면 또는 빙의 실패의 가장 큰 원인은
PlayerController별 차량 Spawn/Possess 구조 부재로 본다.
```

---

## 7. TestMap 상태

`/Game/Maps/TestMap.TestMap` 덤프 결과:

```text
Actor 수: 14
PlayerStart_0: 1개
BP_CFVehiclePawn_C_1: 1대
```

판단:

```text
현재 TestMap은 싱글/초기 차량 테스트용 구조에 가깝다.
2클라 Dedicated Server 테스트용으로는 PlayerStart가 부족하고,
맵 배치 차량 1대가 서버 Spawn/Possess 구조와 충돌할 수 있다.
```

권장:

```text
- PlayerStart 최소 2개 필요
- 차량은 GameMode가 서버에서 Spawn하는 방향으로 통일
- 기존 배치 차량은 제거하거나 서버 테스트 전용 맵을 따로 만든다.
```

---

## 8. ACFVehiclePawn 상태

### 8.1 유리한 점

`ACFVehiclePawn` 생성자에서 `AutoPossessPlayer`가 Disabled로 설정되어 있다.

```cpp
AutoPossessPlayer = EAutoReceiveInput::Disabled;
```

판단:

```text
서버 GameMode가 명시적으로 Possess하는 구조와 충돌할 가능성이 낮다.
```

### 8.2 입력 guard

`SetupPlayerInputComponent`는 Dedicated Server 또는 비로컬 컨트롤 Pawn이면 바로 return한다.

```cpp
if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
{
    return;
}
```

`RegisterDefaultInputMappingContext`도 동일한 기준으로 막는다.

판단:

```text
OwnerOnly 입력 구조의 기본 guard는 이미 존재한다.
```

### 8.3 UI guard

Aim Reticle 생성은 다음 조건을 통과해야 한다.

```text
- bShowAimReticle == true
- Dedicated Server가 아님
- IsLocallyControlled() == true
- AimReticleWidgetClass 존재
```

판단:

```text
Reticle은 Dedicated Server에서 생성되지 않도록 기본 guard가 있다.
```

### 8.4 미확정 항목

소스 검색 기준으로 `ACFVehiclePawn` C++ 생성자에서 아래 설정을 강제하는 코드는 확인되지 않았다.

```text
- bReplicates = true
- bReplicateMovement = true
- SetReplicates(true)
- SetReplicateMovement(true)
```

판단:

```text
복제 설정은 BP_CFVehiclePawn 기본값에 의존하고 있을 가능성이 높다.
첫 Codex 작업에서는 C++에서 명시적으로 복제 기본값을 보강할지 검토해야 한다.
```

---

## 9. UCFVehicleCameraComp 상태

현재 `UCFVehicleCameraComp`는 생성자에서 Tick을 켜고, Tick에서 카메라 런타임 초기화와 카메라/AimTrace 갱신을 수행한다.

확인된 흐름:

```text
- PrimaryComponentTick.bCanEverTick = true
- PrimaryComponentTick.bStartWithTickEnabled = true
- TickComponent에서 InitializeCameraRuntime 호출
- UpdateAimState 호출
- UpdateCameraTransform 호출
- UpdateAimTrace 호출
```

현재 확인 범위에서는 Dedicated Server 차단 guard가 직접 보이지 않는다.

판단:

```text
이 항목은 첫 Spawn/Possess 작업을 막지는 않는다.
하지만 Dedicated Server 로그 안정화 단계에서 반드시 정리해야 한다.
```

---

## 10. UCFVehicleAimComp 상태

`UCFVehicleAimComp`는 다음 상태가 확인되었다.

```text
- SetIsReplicatedByDefault(true)
- RepAimVisualState 복제 등록
- Dedicated Server Tick에서는 LocalReticleState를 Hidden으로 두고 return
```

판단:

```text
AimComp는 서버/복제 대비가 일부 되어 있다.
다만 Fire/무기/대미지 시스템은 아직 1차 범위에 포함하지 않는다.
```

---

## 11. BP_CFVehiclePawn 상태

`BP_CFVehiclePawn` 에셋 요약은 확인되었다.

확인된 내용:

```text
- Blueprint 에셋 존재
- resolved_class: /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn_C
- component_count: 19
- CameraBoom / FollowCamera / SM_Body 등 주요 컴포넌트 존재
```

미확정:

```text
- Replicates 실제 값
- Replicate Movement 실제 값
- Auto Possess Player BP Override 여부
- VehicleData 실제 지정값
```

판단:

```text
BP 상세 덤프 요약만으로 복제 관련 값을 확정하지 못했다.
에디터 직접 확인 또는 후속 MCP 세부 덤프가 필요하다.
```

---

## 12. 첫 코드 작업 판단

현재 상태에서 첫 코드 작업은 아래가 맞다.

```text
ACFMPGameMode 기반 최소 서버 Spawn/Possess 구조 작성
```

이유:

```text
1. Server Target은 이미 있다.
2. Dedicated Server는 맵을 로드할 수 있다.
3. 전용 GameMode가 없다.
4. DefaultPawnClass 설정도 없다.
5. TestMap은 PlayerStart 1개와 배치 차량 1대뿐이다.
6. ACFVehiclePawn은 AutoPossess Disabled라 서버 Possess 구조와 맞다.
7. 입력/UI guard는 일부 준비되어 있다.
```

즉, 다음 병목은 빌드/실행이 아니라 다음 구조다.

```text
PlayerController 접속 → 서버 차량 Spawn → 서버 Possess → Owner Client 입력/카메라 연결
```

---

## 13. 첫 Codex 작업 범위

첫 Codex 작업은 코드 작업이므로 Codex 작업지시서로 분리한다.

작업 범위 후보:

```text
- CFMPGameMode.h 추가
- CFMPGameMode.cpp 추가
- ACFMPGameMode 클래스 구현
- PostLogin 기반 SpawnVehicleForController 호출
- PlayerStart 기반 Spawn Transform 결정
- VehiclePawnClass UPROPERTY 추가
- Spawn/Possess 로그 추가
- 필요 시 ACFVehiclePawn 복제 기본값 보강 검토
```

첫 작업에서 제외할 것:

```text
- 무기
- 체력
- 대미지
- 스코어
- 리스폰
- 로비
- Steam 세션
- 이동 보간 튜닝
```

---

## 14. 다음 액션

```text
1. 이 감사 결과를 기준으로 Codex용 첫 작업지시서를 생성한다.
2. Codex는 CFMPGameMode 추가 작업을 수행한다.
3. 작업 후 빌드 결과와 변경 파일을 기준으로 재감사한다.
4. 이후 Config 연결과 TestMap 정리로 넘어간다.
```

---

## 15. 변경 기록

### v0.1.0

```text
- 서버 고도화 첫 코드 작업 전 현재상태 감사 결과 작성
- Server Target 존재 확인 반영
- GameMode / DefaultPawn / Config 부재 확인 반영
- TestMap의 PlayerStart 1개 / 차량 1대 구조 반영
- ACFVehiclePawn의 AutoPossess Disabled, 입력/UI guard 상태 반영
- UCFVehicleCameraComp의 Dedicated Server guard 부족 항목 기록
- 첫 코드 작업을 ACFMPGameMode로 확정
```
