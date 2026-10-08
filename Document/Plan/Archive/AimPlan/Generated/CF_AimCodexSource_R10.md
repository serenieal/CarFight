# Task Source — CarFight Aim Commit G

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: Reticle Widget C++ 연결 준비
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2, R3, R4, R5/R6, R7, R8, R9
- 이번 단계: R10 — Reticle Widget Pawn 연결 준비

## Goal

CarFight 프로젝트에서 R9에 추가된 `UCFAimReticleWidget`을 `ACFVehiclePawn`이 선택적으로 생성하고 Viewport에 추가할 수 있는 C++ 연결 기반을 추가한다.

이번 작업의 목표는 WBP 에셋을 만들지 않고, BP에서 Reticle Widget Class를 지정했을 때 로컬 플레이어 차량에서만 안전하게 Reticle Widget을 생성/갱신/제거할 수 있는 경로를 여는 것이다.

이번 작업은 실제 `WBP_AimReticle` 생성, Designer 배치, 이미지/애니메이션/머티리얼 작업을 하지 않는다.

## Current Baseline

현재 기준은 다음과 같다.

- `UCFAimReticleWidget` C++ 부모가 존재한다.
- `UCFAimReticleWidget::SetVehiclePawnRef()`가 Pawn 참조를 받고 즉시 갱신한다.
- `UCFAimReticleWidget`은 AimComp 상태를 읽어 Reticle 텍스트/가시성을 갱신한다.
- `ACFVehiclePawn`은 `GetVehicleAimComp()`를 제공한다.
- `ACFVehiclePawn`에는 아직 Reticle Widget Class / Instance 연결이 없다.
- `ACFVehiclePawn::BeginPlay()`는 Input Mapping Context 등록과 Runtime 초기화를 수행한다.
- `ACFVehiclePawn`에는 현재 `EndPlay()` override가 없다.
- R9 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `ACFVehiclePawn`에 `AimReticleWidgetClass` UPROPERTY를 추가한다.
2. `ACFVehiclePawn`에 `AimReticleWidgetInstance` 런타임 캐시를 추가한다.
3. `ACFVehiclePawn`에 `bShowAimReticle` 토글을 추가한다.
4. `ACFVehiclePawn`에 `AimReticleZOrder` 설정값을 추가한다.
5. `ShouldShowAimReticle()` BlueprintPure 함수를 추가한다.
6. `CreateAimReticleWidget()` BlueprintCallable 함수를 추가한다.
7. `DestroyAimReticleWidget()` BlueprintCallable 함수를 추가한다.
8. `RefreshAimReticleWidget()` BlueprintCallable 함수를 추가한다.
9. `BeginPlay()` 또는 기존 런타임 초기화 흐름 이후에 `CreateAimReticleWidget()`을 호출한다.
10. `EndPlay()` override를 추가해 `DestroyAimReticleWidget()`을 호출한다.
11. Reticle Widget은 로컬 제어 Pawn에서만 생성한다.
12. `AimReticleWidgetClass == nullptr`이면 아무것도 생성하지 않고 안전하게 종료한다.
13. 생성된 Widget에는 `SetVehiclePawnRef(this)`를 호출한다.
14. 기존 주행, 입력, AimComp, VehicleDebug 흐름을 유지한다.

## Out of Scope

이번 작업에서 하지 않는다.

- `WBP_AimReticle` 에셋 생성 금지
- WBP Designer 수정 금지
- Reticle 이미지/머티리얼/애니메이션 구현 금지
- PlayerController/HUD 클래스 생성 금지
- Fire Input / RPC / Server Validation 수정 금지
- HitScan / Damage / Projectile 수정 금지
- RepAimVisual 로직 수정 금지
- WeaponComp 생성 금지
- Input Mapping Context 수정 금지
- BP 에셋 값 지정 금지
- Multicast Fire FX 구현 금지

## Target Files

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

참고 파일:

- `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`

이번 작업에서 `CFAimReticleWidget.h/.cpp`는 원칙적으로 수정하지 않는다. R9에서 이미 C++ 부모 위젯이 준비되어 있기 때문이다.

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. Reticle Widget은 로컬 제어 Pawn에서만 생성한다.
3. 서버 전용 또는 Simulated Proxy에서 Widget을 생성하지 않는다.
4. `AimReticleWidgetClass`가 null이면 크래시하지 않는다.
5. 이미 Widget Instance가 있으면 중복 생성하지 않는다.
6. Destroy 시 Viewport에서 제거하고 참조를 null로 정리한다.
7. `EndPlay()` 추가 시 반드시 `Super::EndPlay(EndPlayReason)`를 호출한다.
8. `CreateWidget<UCFAimReticleWidget>()`를 사용할 때 유효한 PlayerController 또는 World를 확인한다.
9. UI 연결은 표시 전용이다. Trace/RPC/Damage/Weapon 로직을 호출하지 않는다.
10. 기존 VehicleDebug UI 토글과 Aim Reticle 토글을 섞지 않는다.
11. 기존 `bEnableDriveStateOnScreenDebug`는 Reticle 표시 조건으로 사용하지 않는다.
12. 빌드 실패 상태로 끝내지 않는다.

## Recommended Pawn Members

`ACFVehiclePawn`에 다음 멤버를 권장한다.

UPROPERTY:

- `TSubclassOf<UCFAimReticleWidget> AimReticleWidgetClass`
- `TObjectPtr<UCFAimReticleWidget> AimReticleWidgetInstance`
- `bool bShowAimReticle = true`
- `int32 AimReticleZOrder = 20`

권장 Category:

- `CarFight|VehiclePawn|Aim|UI`

권장 Tooltip:

- `AimReticleWidgetClass`: 로컬 플레이어 차량에서 생성할 Aim Reticle WBP 클래스입니다.
- `AimReticleWidgetInstance`: 런타임에 생성된 Aim Reticle Widget 인스턴스입니다.
- `bShowAimReticle`: True이면 로컬 플레이어 차량에서 Aim Reticle Widget 생성을 허용합니다.
- `AimReticleZOrder`: Viewport에 추가할 때 사용할 Reticle Widget ZOrder입니다.

## Recommended Pawn Functions

Public 또는 BlueprintCallable/Pure:

- `bool ShouldShowAimReticle() const;`
- `bool CreateAimReticleWidget();`
- `void DestroyAimReticleWidget();`
- `void RefreshAimReticleWidget();`

Protected:

- `virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;`

## Recommended Create Flow

`CreateAimReticleWidget()` 권장 흐름:

1. `ShouldShowAimReticle()`가 false이면 return false.
2. `AimReticleWidgetInstance`가 이미 유효하면 `RefreshAimReticleWidget()` 후 return true.
3. `AimReticleWidgetClass`가 null이면 return false.
4. `APlayerController* PlayerController = Cast<APlayerController>(GetController())`를 시도한다.
5. PlayerController가 없으면 return false.
6. `CreateWidget<UCFAimReticleWidget>(PlayerController, AimReticleWidgetClass)` 호출.
7. 생성 실패 시 return false.
8. `AimReticleWidgetInstance->SetVehiclePawnRef(this)` 호출.
9. `AddToViewport(AimReticleZOrder)` 호출.
10. return true.

`ShouldShowAimReticle()` 권장 조건:

- `bShowAimReticle == true`
- `IsLocallyControlled() == true`
- `AimReticleWidgetClass != nullptr`

`DestroyAimReticleWidget()` 권장 흐름:

1. `AimReticleWidgetInstance`가 유효하면 `RemoveFromParent()` 호출.
2. `AimReticleWidgetInstance = nullptr`.

`RefreshAimReticleWidget()` 권장 흐름:

1. `AimReticleWidgetInstance`가 유효하면 `SetVehiclePawnRef(this)` 또는 `RefreshFromPawn()` 호출.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `ACFVehiclePawn`에 Aim Reticle Widget Class 설정이 추가되어 있다.
2. `ACFVehiclePawn`에 Aim Reticle Widget Instance 캐시가 추가되어 있다.
3. `ShouldShowAimReticle()`이 구현되어 있다.
4. `CreateAimReticleWidget()`이 구현되어 있다.
5. `DestroyAimReticleWidget()`이 구현되어 있다.
6. `RefreshAimReticleWidget()`이 구현되어 있다.
7. `BeginPlay()`에서 안전하게 Reticle 생성 시도를 한다.
8. `EndPlay()`에서 Reticle Widget을 제거한다.
9. 로컬 제어 Pawn이 아니면 Reticle Widget을 생성하지 않는다.
10. Reticle Widget Class가 null이면 크래시하지 않는다.
11. Widget 중복 생성이 방지된다.
12. 생성된 Widget에 `SetVehiclePawnRef(this)`가 호출된다.
13. WBP 에셋 생성/수정 없이 C++ 빌드가 성공한다.
14. Fire/RPC/HitScan/Damage/Projectile 로직이 변경되지 않는다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 수정 파일은 `CFVehiclePawn.h/.cpp`여야 한다.
3. `CFAimReticleWidget.h/.cpp`는 변경하지 않는 것이 기본이다.
4. `AimReticleWidgetClass`가 null인 상태에서도 PIE 시작 시 크래시하지 않아야 한다.
5. BP에서 `AimReticleWidgetClass`에 WBP 자식을 지정할 수 있어야 한다.
6. WBP가 지정된 로컬 Pawn에서만 Widget이 생성될 수 있어야 한다.
7. Simulated Proxy나 서버 전용 경로에서 Widget이 생성되지 않아야 한다.
8. 기존 VehicleDebug HUD/Panel 동작이 변경되지 않아야 한다.
9. Reticle 연결 코드가 Trace/RPC/Damage/Projectile을 호출하지 않는지 확인한다.

## Notes

이번 단계는 C++ 연결 준비다.

실제 `WBP_AimReticle` 에셋 생성, TextBlock 배치, `AimReticleWidgetClass` 지정, `IA_Fire` 에셋 연결은 다음 에셋/검증 단계에서 처리한다.

## Unresolved

없음.
