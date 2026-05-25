# Task Source — CarFight Aim Commit A

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: 신규 C++ 기반 추가
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 기준 작업지시서: `Document/ProjectSSOT/Plan/AimPlan/CF_AimCodexTask.md`

## Goal

CarFight 프로젝트에 Aim 시스템의 1차 C++ 기반을 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 `Commit A — Aim 타입과 컴포넌트 골격` 범위만 수행한다.

구현 범위는 다음 2개 단계로 제한한다.

- R1 — Aim 타입 정의
- R2 — `UCFVehicleAimComp` 골격 추가

이번 작업은 전체 조준 시스템 완성이 아니다. Local Aim 계산, VehicleDebug Aim 카테고리, Fire RPC, 서버 검증, Reticle UI는 구현하지 않는다.

## In Scope

이번 작업에 포함한다.

1. `CFVehicleAimTypes.h` 생성
2. `UCFVehicleAimComp` 헤더 / cpp 생성
3. `ACFVehiclePawn`에 AimComp 멤버 추가
4. `ACFVehiclePawn` 생성자에서 AimComp 생성
5. `GetVehicleAimComp()` getter 추가
6. AimComp 초기화 함수 추가
7. AimComp가 Owner Pawn과 CameraComp를 안전하게 찾을 수 있는 기본 구조 추가
8. C++ 컴파일 가능한 상태 유지

`CFVehicleAimTypes.h`에는 다음 타입을 정의한다.

- `ECFVehicleReticleState`
- `ECFVehicleFireRejectReason`
- `FCFVehicleAimProfile`
- `FCFVehicleLocalAimState`
- `FCFVehicleServerAimState`
- `FCFVehicleRepAimVisualState`
- `FCFVehicleFireRequest`
- `FCFVehicleFireResult`

`UCFVehicleAimComp`에는 다음 기본 멤버를 둔다.

- `OwnerVehiclePawn`
- `CachedVehicleCameraComp`
- `DefaultAimProfile`
- `LocalAimState`
- `ServerAimState`
- `RepAimVisualState`
- `bAimRuntimeReady`
- `LastAimRuntimeSummary`

`UCFVehicleAimComp`에는 다음 기본 함수를 둔다.

- `InitializeAimRuntime()`
- `RefreshAimRuntimeReferences()`
- `GetLocalAimState()`
- `GetServerAimState()`
- `GetRepAimVisualState()`
- `GetDefaultAimProfile()`
- `IsAimRuntimeReady()`
- `GetLastAimRuntimeSummary()`
- `ResolveOwnerVehiclePawn()`
- `ResolveVehicleCameraComp()`

## Out of Scope

이번 작업에서 하지 않는다.

- `ServerRequestFire()` 구현 금지
- `InputAction_Fire` 생성 금지
- Reticle UI 생성 금지
- VehicleDebug Aim 카테고리 생성 금지
- Local Aim 계산 로직 완성 금지
- Fire RPC 구현 금지
- 서버 발사 검증 구현 금지
- HitScan 구현 금지
- Damage 구현 금지
- Projectile 구현 금지
- `UCFVehicleWeaponComp` 생성 금지
- BP 에셋 수정 금지
- PoliceCar 전용 하드코딩 금지
- CameraComp 안에 무기 발사 판정 추가 금지

## Target Files

생성 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

참고 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
- `UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. 새 C++ 타입과 변수명은 직관적으로 작성한다.
3. 모든 BP 노출 변수에는 한국어 `ToolTip`을 작성한다.
4. `USTRUCT`는 필요한 경우 `BlueprintType`으로 선언한다.
5. Runtime State 계열 변수는 기본적으로 `VisibleAnywhere, BlueprintReadOnly`를 사용한다.
6. 튜닝 가능한 Profile 값은 `EditAnywhere, BlueprintReadOnly`를 사용한다.
7. Category는 `CarFight|Aim`, Fire Request 계열은 `CarFight|Weapon`, Pawn 컴포넌트 참조는 `CarFight|Components`를 사용한다.
8. `UCFVehicleAimComp`는 이번 작업에서 Tick을 사용하지 않는다. `PrimaryComponentTick.bCanEverTick = false`를 기본으로 한다.
9. 기존 `UCFVehicleCameraComp` 책임을 AimComp로 옮기지 않는다.
10. 기존 VehicleDebug / CameraDebug 코드를 이번 작업에서 변경하지 않는다.
11. 기존 주행, 조향, 카메라 입력 동작을 변경하지 않는다.
12. include 순환을 만들지 않는다. 가능한 forward declaration을 사용한다.
13. `ACFVehiclePawn`의 기존 스타일과 카테고리 규칙을 따른다.
14. 빌드 실패 상태로 끝내지 않는다.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `CFVehicleAimTypes.h`가 생성되어 있고 요구 enum / struct가 모두 정의되어 있다.
2. `CFVehicleAimComp.h`와 `CFVehicleAimComp.cpp`가 생성되어 있다.
3. `UCFVehicleAimComp`가 `UActorComponent` 기반으로 컴파일된다.
4. `ACFVehiclePawn`에 AimComp UPROPERTY가 추가되어 있다.
5. `ACFVehiclePawn` 생성자에서 `VehicleAimComp`가 `CreateDefaultSubobject`로 생성된다.
6. `GetVehicleAimComp()`가 선언 / 구현되어 있다.
7. `InitializeAimRuntime()` 호출 시 Owner Pawn과 CameraComp 참조를 갱신할 수 있다.
8. Owner Pawn 또는 CameraComp를 찾지 못해도 크래시하지 않고 `bAimRuntimeReady = false`가 된다.
9. `LastAimRuntimeSummary`에 초기화 상태를 설명하는 문자열이 남는다.
10. 기존 카메라 / 주행 / 조향 코드의 동작을 바꾸지 않는다.
11. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 신규 파일 3개가 올바른 경로에 생성되어야 한다.
3. `CFVehiclePawn.h`와 `CFVehiclePawn.cpp`만 기존 파일 중 수정되어야 한다.
4. `BP_CFVehiclePawn`에서 AimComp가 컴포넌트로 보일 수 있어야 한다.
5. BeginPlay 또는 기존 런타임 초기화 흐름에서 AimComp 초기화가 호출되어야 한다.
6. CameraComp가 없는 경우에도 에디터 또는 PIE가 크래시하지 않아야 한다.
7. 기존 Look 입력, CameraComp, VehicleDriveComp, WheelSyncComp 동작에 회귀가 없어야 한다.

## Notes

이번 작업 이후 다음 작업은 `Commit B — Local Aim 계산`이다.

다음 작업에서 처리할 항목은 다음이다.

- CameraRuntimeState 읽기
- LocalAimTargetLocation 계산
- LocalAimDirection 계산
- AimProfile 범위 판정
- Local Reticle State 계산

이번 작업에서는 위 항목을 구현하지 않는다.

## Unresolved

없음.
