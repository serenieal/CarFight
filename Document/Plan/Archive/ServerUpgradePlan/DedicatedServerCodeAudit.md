# Dedicated Server 금지 코드 정밀 감사

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 관련 단계: Phase 7
- 상태: Active

---

## 1. 목적

이 문서는 CarFight Dedicated Server 1차 안정화 이후, 서버에서 실행되면 안 되는 로컬 전용 코드 경로를 점검한 결과다.

점검 대상은 다음이다.

```text
- Viewport UI 생성
- LocalPlayer 접근
- Camera Tick / Camera Transform 갱신
- Debug HUD / Debug Panel
- Sound 호출
- FX / Niagara / Emitter 호출
- DrawDebug 호출
```

---

## 2. 감사 요약

| 영역 | 결과 | 판단 |
|---|---:|---|
| Aim Reticle UI | Guard 있음 | PASS |
| Enhanced Input LocalPlayer | Guard 있음 | PASS |
| VehicleDebug HUD / Panel | Guard 있음 | PASS |
| Sound 호출 | 없음 | PASS |
| Niagara / Emitter 호출 | 없음 | PASS |
| 서버 Aim Trace DrawDebug | 토글 뒤에 있음 | PASS / 의도적 서버 디버그 |
| VehicleCameraComp Tick | Dedicated Server guard 없음 | FIX 권장 |

현재 테스트는 모두 PASS였지만, `UCFVehicleCameraComp`는 Dedicated Server에서 불필요한 Tick과 카메라 참조 탐색을 계속 수행할 수 있으므로 정리 대상이다.

---

## 3. UI / LocalPlayer 감사

### 3.1 Aim Reticle

검색 결과:

```text
CreateWidget<UCFAimReticleWidget>
AddToViewport
```

위 경로는 `ACFVehiclePawn::CreateAimReticleWidget`에서만 Viewport 추가로 이어진다.

확인된 Guard:

```cpp
bool ACFVehiclePawn::ShouldShowAimReticle() const
{
    const bool bHasViewportContext = GetNetMode() != NM_DedicatedServer;
    return bShowAimReticle && bHasViewportContext && IsLocallyControlled();
}
```

판단:

```text
Aim Reticle은 Dedicated Server에서 생성되지 않는다.
```

### 3.2 Enhanced Input / LocalPlayer

검색 결과:

```text
ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
```

확인된 Guard:

```cpp
if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
{
    return false;
}
```

판단:

```text
Enhanced Input Mapping Context 등록은 Dedicated Server와 비소유 Pawn에서 실행되지 않는다.
```

---

## 4. Debug UI 감사

VehicleDebug HUD / Panel 표시 조건은 `ShouldShowVehicleDebugUi`를 통해 제어된다.

확인된 Guard:

```cpp
return bEnableDriveStateOnScreenDebug && (GetNetMode() != NM_DedicatedServer) && IsLocallyControlled();
```

판단:

```text
Debug HUD / Panel은 Viewport가 있는 로컬 제어 Pawn에서만 표시된다.
Dedicated Server에서 직접 표시될 조건은 없다.
```

단, Debug 위젯 내부에는 `CreateWidget` 경로가 많다. 이 경로는 Debug Panel Widget이 이미 생성된 이후 내부 섹션/네비게이션 위젯을 만드는 용도다.

판단:

```text
상위 Debug UI 생성 경로가 Dedicated Server에서 막혀 있으므로 현재는 PASS로 본다.
```

---

## 5. Sound / FX 감사

검색 결과:

```text
PlaySound: 0건
SpawnEmitter: 0건
Niagara: 0건
```

판단:

```text
현재 C++ 기준 서버에서 문제 될 사운드/이펙트 호출 경로는 확인되지 않았다.
```

---

## 6. DrawDebug 감사

검색 결과:

```text
UCFVehicleCameraComp::UpdateAimTrace
ACFVehiclePawn::ResolveServerAimTrace
```

### 6.1 CameraComp DrawDebug

`UCFVehicleCameraComp`의 Aim Trace Debug는 카메라 Tick에서 실행될 수 있다.

문제:

```text
UCFVehicleCameraComp 자체에 Dedicated Server guard가 없으므로, 서버에서 불필요하게 Tick / AimTrace / DrawDebug 조건 확인이 돌 수 있다.
```

### 6.2 Server Aim Trace DrawDebug

`ACFVehiclePawn::ResolveServerAimTrace`의 DrawDebug는 `bDrawServerAimTraceDebug` 조건 뒤에 있다.

판단:

```text
서버 Aim Trace 시각화는 의도적인 서버 디버그 기능으로 본다.
현재 즉시 수정 대상은 아니다.
```

---

## 7. CameraComp 감사

확인된 현재 구조:

```cpp
UCFVehicleCameraComp::UCFVehicleCameraComp()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}
```

```cpp
void UCFVehicleCameraComp::TickComponent(...)
{
    Super::TickComponent(...);

    if (!bCameraRuntimeReady && !InitializeCameraRuntime())
    {
        return;
    }

    UpdateAimState(...);
    UpdateCameraTransform(...);
    UpdateAimTrace(...);
}
```

문제:

```text
Dedicated Server에서는 카메라 Transform, FOV, Aim Trace, SpringArm/Camera 갱신이 필요 없다.
하지만 현재 TickComponent에 Dedicated Server early return이 없다.
```

위험도:

```text
치명 오류: 낮음
불필요한 서버 비용: 있음
장기 안정성: 정리 권장
```

---

## 8. 수정 권장 사항

Codex 후속 작업으로 아래를 권장한다.

```text
- UCFVehicleCameraComp에 Dedicated Server guard 추가
- BeginPlay에서 Dedicated Server이면 Tick 비활성화
- TickComponent 초반에 Dedicated Server early return 추가
- InitializeCameraRuntime이 Dedicated Server에서 카메라 참조 검색을 하지 않도록 방어
```

권장 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
```

수정 제외:

```text
- CFVehiclePawn.h/.cpp
- Debug UI 위젯 파일
- Config
- Map
- Blueprint
```

---

## 9. 결론

Dedicated Server 금지 코드 감사 결과, 대부분의 UI/LocalPlayer/Debug 경로는 이미 guard가 되어 있다.

유일한 명확한 수정 후보는 다음이다.

```text
UCFVehicleCameraComp Dedicated Server Tick guard 추가
```

테스트는 이미 PASS이므로 이 작업은 버그 긴급 수정이 아니라, 서버 비용과 장기 안정성을 위한 정리 작업이다.

---

## 10. 변경 기록

### v0.1.0

```text
- Dedicated Server 금지 코드 정밀 감사 최초 작성
- UI / LocalPlayer / Debug / Sound / FX / DrawDebug / CameraComp 감사 결과 정리
- UCFVehicleCameraComp Dedicated Server guard 추가를 후속 Codex 작업으로 지정
```
