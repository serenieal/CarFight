# Task Source — CarFight Aim Commit C

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: VehicleDebug 표시 확장
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2 — Aim 타입 정의 + AimComp 골격, R3 — Local Aim 계산
- 이번 단계: R4 — VehicleDebug Aim

## Goal

CarFight 프로젝트의 VehicleDebug 시스템에 Aim 카테고리를 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 `Commit C — VehicleDebug Aim` 범위만 수행한다.

R3에서 구현된 `UCFVehicleAimComp::GetLocalAimState()`, `GetServerAimState()`, `GetRepAimVisualState()`, `GetReticleState()`, `IsAimRuntimeReady()`, `GetLastAimRuntimeSummary()`를 읽어, VehicleDebug Snapshot과 Panel ViewData에 Aim 상태를 표시한다.

이번 작업은 Fire Request, RPC, Server 검증, HitScan, Reticle UI를 구현하지 않는다.

## Current Baseline

현재 기준은 다음과 같다.

- `ACFVehiclePawn`은 `VehicleAimComp`를 소유한다.
- `UCFVehicleAimComp`는 Tick에서 `FCFVehicleLocalAimState`를 갱신한다.
- `UCFVehicleAimComp`는 다음 getter를 제공한다.
  - `GetLocalAimState()`
  - `GetServerAimState()`
  - `GetRepAimVisualState()`
  - `GetReticleState()`
  - `IsAimRuntimeReady()`
  - `GetLastAimRuntimeSummary()`
- `FCFVehicleDebugSnapshot`에는 현재 `Overview`, `Drive`, `Input`, `Camera`, `Runtime` 카테고리가 있다.
- `ACFVehiclePawn::GetVehicleDebugSnapshot()`에서 Camera 카테고리를 채우고 있다.
- `UCFVehicleDebugPanelWidget`은 `CachedOverview`, `CachedDrive`, `CachedInput`, `CachedCamera`, `CachedRuntime`를 사용한다.
- `BuildVehicleDebugPanelViewData()`는 현재 `Overview`, `Drive`, `Input`, `Camera`, `Runtime` 순서로 Section을 추가한다.
- `BuildCameraSectionViewData()` 패턴을 Aim Section 구현의 기준으로 삼는다.
- R3 이후 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `CFVehiclePawn.h`에 `FCFVehicleDebugAim` 구조체를 추가한다.
2. `FCFVehicleDebugSnapshot`에 `Aim` 카테고리를 추가한다.
3. `ACFVehiclePawn`에 `GetVehicleDebugAim()` BlueprintPure getter를 추가한다.
4. `ACFVehiclePawn::GetVehicleDebugSnapshot()`에서 `VehicleAimComp` 상태를 읽어 `DebugSnapshot.Aim`을 채운다.
5. `ACFVehiclePawn::GetVehicleDebugAim()`을 구현한다.
6. `UCFVehicleDebugPanelWidget`에 `CachedAim` 멤버를 추가한다.
7. `RefreshFromPawn()`에서 `VehiclePawnRef->GetVehicleDebugAim()`을 읽어 `CachedAim`에 저장한다.
8. `BuildVehicleDebugPanelViewData()`에 Aim Section을 추가한다.
9. `BuildAimSectionViewData(const FCFVehicleDebugAim& InAim)` 선언과 구현을 추가한다.
10. Aim Section은 Camera 다음, Runtime 이전에 표시한다.
11. Aim Section Navigation 설정은 다음을 권장한다.
    - SectionId: `Aim`
    - Title: `조준`
    - NavigationGroup: `Vehicle`
    - NavigationOrder: `45`
    - BadgeText: Reticle 상태 또는 `Ready`/`Blocked`/`OutOfArc` 요약

## Out of Scope

이번 작업에서 하지 않는다.

- `UCFVehicleAimComp` Local Aim 계산 로직 변경 금지
- `ServerRequestFire()` 구현 금지
- `InputAction_Fire` 생성 금지
- Fire Request 생성 금지
- Server Fire 검증 금지
- HitScan 구현 금지
- Damage 구현 금지
- Projectile 구현 금지
- Reticle UI 생성 금지
- BP 에셋 수정 금지
- Replication 설정 금지
- WBP Designer 수정 금지
- `UCFVehicleWeaponComp` 생성 금지
- CameraComp 내부 로직 수정 금지

## Target Files

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`

참고 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h`

이번 작업에서 `CFVehicleAimComp.h/.cpp`는 원칙적으로 수정하지 않는다. R3에서 이미 Local Aim 계산이 완료되었기 때문이다.

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. 기존 `FCFVehicleDebugSnapshot` 구조를 제거하거나 이름 변경하지 않는다.
3. 기존 `Overview`, `Drive`, `Input`, `Camera`, `Runtime` 카테고리를 제거하지 않는다.
4. 기존 `BuildCameraSectionViewData()` 동작을 변경하지 않는다.
5. 기존 VehicleDebug Navigation 구조를 유지한다.
6. Aim Section은 Camera 다음, Runtime 이전에 추가한다.
7. `FCFVehicleDebugAim`은 `USTRUCT(BlueprintType)`로 선언한다.
8. BP 노출 변수에는 `VisibleAnywhere, BlueprintReadOnly`와 한국어 `ToolTip`을 사용한다.
9. `UCFVehicleDebugPanelWidget`에 추가되는 `CachedAim`은 `BlueprintReadOnly`로 둔다.
10. AimComp가 없는 경우에도 VehicleDebug가 크래시하지 않아야 한다.
11. AimComp가 없는 경우 `bHasVehicleAimComponent = false`, ReticleState는 `Hidden`으로 표시한다.
12. ServerAimState와 RepAimVisualState는 현재 기본값을 읽어 표시만 한다. 계산하지 않는다.
13. Fire/RPC/HitScan/Damage/UI 생성 로직을 넣지 않는다.
14. 빌드 실패 상태로 끝내지 않는다.

## Recommended FCFVehicleDebugAim Fields

`FCFVehicleDebugAim`에는 다음 필드를 권장한다.

- `bHasVehicleAimComponent: bool`
- `bAimRuntimeReady: bool`
- `LocalAimState: FCFVehicleLocalAimState`
- `ServerAimState: FCFVehicleServerAimState`
- `RepAimVisualState: FCFVehicleRepAimVisualState`
- `ReticleState: ECFVehicleReticleState`
- `AimRuntimeSummary: FString`

Category는 `CarFight|VehiclePawn|Debug|Aim`을 사용한다.

## Recommended Aim Section Fields

`BuildAimSectionViewData()`에서 표시할 필드 후보는 다음과 같다.

상위 Aim Section 필드:

- `Has AimComp`
- `Runtime Ready`
- `Reticle State`
- `Local Can Fire`
- `Within Weapon Arc`
- `Aim Blocked`
- `Runtime Summary`

Local Aim 하위 섹션:

- `Local Aim Target Location`
- `Local Aim Direction`

Server Aim 하위 섹션:

- `Server Can Fire`
- `Server Within Weapon Arc`
- `Last Server Reject Reason`
- `Last Accepted Fire Request Id`
- `Last Rejected Fire Request Id`

Rep Aim Visual 하위 섹션:

- `Rep Aim Direction`
- `Rep Aim Target Location`
- `Is Firing Visual`
- `Rep Weapon Visual Mode`

## Implementation Notes

문자열 변환은 현재 파일 스타일에 맞춘다.

필요하다면 helper를 cpp 내부 익명 namespace에 추가할 수 있다.

권장 helper 후보:

- `FString LexToString(ECFVehicleReticleState InState)` 또는 로컬 switch 함수
- `FString LexToString(ECFVehicleFireRejectReason InReason)` 또는 로컬 switch 함수
- `FString FormatVectorForDebug(const FVector& InVector)`
- `FString FormatBoolForDebug(bool bValue)`

단, 전역 public API를 과도하게 늘리지 않는다.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `FCFVehicleDebugAim`이 `CFVehiclePawn.h`에 추가되어 있다.
2. `FCFVehicleDebugSnapshot`에 `Aim` 필드가 추가되어 있다.
3. `ACFVehiclePawn::GetVehicleDebugAim()`이 선언/구현되어 있다.
4. `GetVehicleDebugSnapshot()`이 `VehicleAimComp` 상태를 안전하게 읽어 `DebugSnapshot.Aim`을 채운다.
5. AimComp가 없어도 `GetVehicleDebugSnapshot()`이 크래시하지 않는다.
6. `UCFVehicleDebugPanelWidget`에 `CachedAim`이 추가되어 있다.
7. `RefreshFromPawn()`이 `CachedAim`을 갱신한다.
8. `BuildVehicleDebugPanelViewData()`가 Aim Section을 Camera 다음, Runtime 이전에 추가한다.
9. `BuildAimSectionViewData()`가 선언/구현되어 있고 필드가 비어 있지 않다.
10. Navigation에 Aim 항목이 표시될 수 있는 Section ViewData가 생성된다.
11. 기존 Overview/Drive/Input/Camera/Runtime Section은 유지된다.
12. Fire/RPC/HitScan/Damage/Reticle UI 관련 구현이 추가되지 않는다.
13. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 수정 파일은 기본적으로 Target Files 4개여야 한다.
3. `CFVehicleAimComp.h/.cpp`가 이번 작업에서 변경되지 않아야 한다.
4. `CFVehicleCameraComp.h/.cpp`가 이번 작업에서 변경되지 않아야 한다.
5. VehicleDebug Panel ViewData에 Aim Section이 추가되어야 한다.
6. `BuildVehicleDebugPanelViewData()`의 Section 순서는 `Overview`, `Drive`, `Input`, `Camera`, `Aim`, `Runtime`이어야 한다.
7. AimComp가 유효하면 LocalAimState 값이 Aim Section에 반영되어야 한다.
8. AimComp가 없거나 준비되지 않아도 크래시하지 않아야 한다.
9. 기존 Camera Section이 유지되어야 한다.
10. 기존 VehicleDebug Navigation 구조가 유지되어야 한다.

## Notes

R4는 디버그 표시 단계다. 게임플레이 발사 기능을 추가하는 단계가 아니다.

R4가 통과하면 다음 작업은 `R5/R6 — Fire Request 더미 + Server Fire 검증`이다.

## Unresolved

없음.
