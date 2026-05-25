# Task Source — CarFight Aim Commit F2

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: Reticle UI C++ 부모 최소 버전
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2 — Aim 타입 정의 + AimComp 골격, R3 — Local Aim 계산, R4 — VehicleDebug Aim, R5/R6 — Fire Request 더미 + Server Fire 검증, R7 — Server HitScan 더미, R8 — RepAimVisual
- 이번 단계: R9 — Reticle UI 최소 버전

## Goal

CarFight 프로젝트에 Aim Reticle UI용 C++ 부모 위젯을 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 `R9 — Reticle UI 최소 버전` 중 C++ 기반만 수행한다.

목표는 `UCFVehicleAimComp`의 Local Reticle State와 Fire Result 상태를 읽어 WBP 자식이 표시할 수 있는 텍스트/가시성/상태 캐시를 제공하는 것이다.

이번 작업에서 WBP 에셋 생성, HUD 배치, Input Mapping 수정, 실제 Reticle 디자인은 하지 않는다.

## Current Baseline

현재 기준은 다음과 같다.

- `UCFVehicleAimComp`는 `GetLocalAimState()`, `GetReticleState()`, `GetServerAimState()`, `GetRepAimVisualState()`를 제공한다.
- `ACFVehiclePawn`은 `GetVehicleAimComp()`를 제공한다.
- `ACFVehiclePawn`은 `LastFireResult`를 보유하지만 직접 getter는 아직 없을 수 있다.
- VehicleDebug HUD는 `UCFVehicleDebugHudWidget : UUserWidget` 형태로 구현되어 있다.
- 기존 UI C++ 부모들은 `BindWidgetOptional` 패턴을 사용한다.
- R8 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `CFAimReticleWidget.h` 생성
2. `CFAimReticleWidget.cpp` 생성
3. `UCFAimReticleWidget : UUserWidget` 클래스 생성
4. `SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)` 추가
5. `RefreshFromPawn()` 추가
6. `ApplyReticleState(ECFVehicleReticleState InReticleState)` 추가
7. `UpdateReticleVisibility()` 추가
8. `GetReticleStateDisplayText()` 또는 동등한 BlueprintPure getter 추가
9. `GetCanFireDisplayText()` 또는 동등한 BlueprintPure getter 추가
10. `NativeConstruct()`에서 초기 갱신 수행
11. `NativeTick()`에서 `bAutoRefreshEveryTick`가 true일 때 자동 갱신
12. `BindWidgetOptional` TextBlock 후보를 추가한다.
13. AimComp가 없거나 Pawn이 없을 때 크래시하지 않고 Hidden 상태로 처리한다.
14. Reticle State별 텍스트와 가시성을 최소 구분한다.

## Out of Scope

이번 작업에서 하지 않는다.

- WBP_AimReticle 에셋 생성 금지
- WBP Designer 수정 금지
- HUD 또는 Viewport에 Reticle 자동 생성/배치 금지
- ACFVehiclePawn에 ReticleWidgetClass 추가 금지
- PlayerController/HUD 클래스 생성 금지
- Reticle 이미지/머티리얼/애니메이션 구현 금지
- Fire Input / RPC / Server Validation 수정 금지
- Damage 적용 금지
- Projectile 구현 금지
- Multicast Fire FX 구현 금지
- RepAimVisual 로직 변경 금지
- WeaponComp 생성 금지
- BP 에셋 수정 금지

## Target Files

생성 파일:

- `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`

참고 파일:

- `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugHudWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugHudWidget.cpp`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`

기존 Pawn, AimComp, VehicleDebug 파일은 이번 작업에서 수정하지 않는다.

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. 클래스 이름은 `UCFAimReticleWidget`을 사용한다.
3. 부모 클래스는 `UUserWidget`이다.
4. 기존 `CFVehicleDebugHudWidget` 스타일을 따른다.
5. `BindWidgetOptional`을 사용해 WBP 자식이 없어도 크래시하지 않게 한다.
6. Pawn 또는 AimComp가 없으면 Reticle State를 `Hidden`으로 처리한다.
7. Reticle UI는 직접 Trace를 수행하지 않는다.
8. Reticle UI는 Fire RPC를 호출하지 않는다.
9. Reticle UI는 Damage/Weapon/Projectile을 처리하지 않는다.
10. Reticle UI는 AimComp 상태를 읽어 표시만 한다.
11. BP 노출 함수와 변수에는 한국어 `ToolTip`을 작성한다.
12. 빌드 실패 상태로 끝내지 않는다.

## Recommended Widget Members

`UCFAimReticleWidget`에 다음 멤버를 권장한다.

BindWidgetOptional:

- `Text_ReticleState: UTextBlock*`
- `Text_CanFire: UTextBlock*`
- `Text_ServerState: UTextBlock*`

Runtime Cache:

- `VehiclePawnRef: TObjectPtr<ACFVehiclePawn>`
- `CachedReticleState: ECFVehicleReticleState`
- `CachedLocalAimState: FCFVehicleLocalAimState`
- `CachedServerAimState: FCFVehicleServerAimState`
- `bAutoRefreshEveryTick: bool`
- `bHideWhenReticleHidden: bool`

## Recommended Functions

Public:

- `void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);`
- `void RefreshFromPawn();`
- `void ApplyReticleState(ECFVehicleReticleState InReticleState);`
- `void UpdateReticleVisibility();`
- `FText GetReticleStateDisplayText() const;`
- `FText GetCanFireDisplayText() const;`
- `FText GetServerStateDisplayText() const;`

Protected:

- `virtual void NativeConstruct() override;`
- `virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;`

Private:

- `FString ConvertReticleStateToDisplayString(ECFVehicleReticleState InReticleState) const;`
- `FString ConvertFireRejectReasonToDisplayString(ECFVehicleFireRejectReason InRejectReason) const;`

## Recommended Reticle State Display

Reticle State별 기본 텍스트는 다음을 권장한다.

- `Hidden`: `Reticle: Hidden`
- `Ready`: `Reticle: Ready`
- `Blocked`: `Reticle: Blocked`
- `OutOfArc`: `Reticle: Out Of Arc`
- `NoWeapon`: `Reticle: No Weapon`
- `Cooldown`: `Reticle: Cooldown`
- `Reloading`: `Reticle: Reloading`
- `WaitingServer`: `Reticle: Waiting Server`
- `ServerRejected`: `Reticle: Server Rejected`

이번 단계에서는 색상/이미지/애니메이션을 구현하지 않는다.

## Recommended Refresh Flow

`RefreshFromPawn()` 권장 흐름:

1. Pawn이 유효하지 않으면 `CachedReticleState = Hidden`으로 설정한다.
2. Pawn에서 `GetVehicleAimComp()`를 가져온다.
3. AimComp가 없으면 `CachedReticleState = Hidden`으로 설정한다.
4. AimComp가 있으면 `GetLocalAimState()`, `GetServerAimState()`, `GetReticleState()`를 읽는다.
5. `ApplyReticleState(CachedReticleState)`를 호출한다.
6. `UpdateReticleVisibility()`를 호출한다.

`ApplyReticleState()` 권장 흐름:

1. `CachedReticleState`를 갱신한다.
2. `Text_ReticleState`가 있으면 표시 텍스트를 갱신한다.
3. `Text_CanFire`가 있으면 Local CanFire 텍스트를 갱신한다.
4. `Text_ServerState`가 있으면 Server CanFire / RejectReason 텍스트를 갱신한다.

`UpdateReticleVisibility()` 권장 흐름:

1. `bHideWhenReticleHidden == true`이고 State가 Hidden이면 `Collapsed`
2. 그 외는 `HitTestInvisible`

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `CFAimReticleWidget.h`가 생성되어 있다.
2. `CFAimReticleWidget.cpp`가 생성되어 있다.
3. `UCFAimReticleWidget`이 `UUserWidget` 기반으로 컴파일된다.
4. `SetVehiclePawnRef()`가 Pawn 참조를 저장하고 갱신을 수행한다.
5. `RefreshFromPawn()`이 Pawn/AimComp를 안전하게 읽는다.
6. Pawn 또는 AimComp가 없을 때 크래시하지 않는다.
7. `ApplyReticleState()`가 TextBlock에 상태 문자열을 반영한다.
8. `UpdateReticleVisibility()`가 Hidden 상태를 처리한다.
9. Reticle UI가 Trace/RPC/Damage/Projectile/WeaponComp를 처리하지 않는다.
10. 기존 Pawn/AimComp/VehicleDebug 로직을 변경하지 않는다.
11. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 신규 파일 2개만 생성되는 것이 기본이다.
3. 기존 `CFVehiclePawn`, `CFVehicleAimComp`, `VehicleDebug` 파일이 변경되지 않아야 한다.
4. `UCFAimReticleWidget`이 Blueprintable 위젯 부모로 노출되어야 한다.
5. WBP 자식 없이도 C++ 컴파일이 성공해야 한다.
6. TextBlock이 BindWidgetOptional이므로 WBP에 해당 위젯이 없어도 크래시하지 않아야 한다.
7. Reticle UI가 Trace/RPC/Damage/Projectile을 호출하지 않는지 확인한다.

## Notes

R9는 Reticle UI의 C++ 부모만 만든다.

WBP_AimReticle 생성, Viewport 배치, HUD 통합은 별도 에셋/연결 단계에서 처리한다.

R9가 통과하면 다음 작업은 `Aim Integration Polish` 또는 `Input/Reticle Asset 연결` 단계로 분리한다.

## Unresolved

없음.
