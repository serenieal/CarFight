# Task Source — CarFight Aim Commit B

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: 기존 AimComp 기능 확장
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2 — Aim 타입 정의 + `UCFVehicleAimComp` 골격
- 이번 단계: R3 — Local Aim 계산

## Goal

CarFight 프로젝트의 `UCFVehicleAimComp`에 Local Aim 계산을 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 `Commit B — Local Aim 계산` 범위만 수행한다.

R1/R2에서 이미 생성된 타입과 컴포넌트를 사용해, Owner Client 또는 단일 플레이 환경에서 Camera Runtime State를 읽고 `FCFVehicleLocalAimState`를 갱신한다.

이번 작업은 VehicleDebug, Fire Request, RPC, Server 검증, Reticle UI를 구현하지 않는다.

## Current Baseline

R1/R2 결과 현재 파일과 구조는 다음과 같다.

- `CFVehicleAimTypes.h`에 Aim enum / struct가 생성되어 있다.
- `CFVehicleAimComp.h/.cpp`가 생성되어 있다.
- `ACFVehiclePawn`에 `VehicleAimComp`가 생성되어 있다.
- `UCFVehicleAimComp::InitializeAimRuntime()`이 Owner Pawn과 CameraComp를 찾는다.
- `UCFVehicleCameraComp`에는 `GetCameraRuntimeState()`가 존재한다.
- `FCFVehicleCameraRuntimeState`에는 다음 필드가 존재한다.
  - `AimHitLocation`
  - `AimTraceDistance`
  - `bAimBlocked`
  - `bWeaponCanFireAtCurrentAim`
  - `ClampedAimYaw`
  - `ClampedAimPitch`
  - `ActiveAimProfileName`
- 현재 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `UCFVehicleAimComp`에서 Tick을 활성화한다.
2. Tick에서 Local Aim State를 갱신한다.
3. `RefreshLocalAimState(float DeltaSeconds)` 함수를 추가한다.
4. Camera Runtime State를 읽어 `LocalAimState.LocalAimTargetLocation`을 갱신한다.
5. Owner Pawn 위치 또는 Camera Aim Target을 기준으로 `LocalAimState.LocalAimDirection`을 계산한다.
6. 차량 기준 Aim Yaw / Pitch를 계산하는 helper 함수를 추가한다.
7. `DefaultAimProfile`의 `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg`를 이용해 `bLocalWithinWeaponArc`를 계산한다.
8. Camera Runtime State의 `bAimBlocked`를 `bLocalAimBlocked`에 반영한다.
9. `bLocalCanFire`를 `bLocalWithinWeaponArc && !bLocalAimBlocked` 기준으로 계산한다.
10. `LocalReticleState`를 다음 규칙으로 계산한다.
    - Aim 런타임 준비 안 됨: `Hidden`
    - Aim blocked: `Blocked`
    - Weapon Arc 밖: `OutOfArc`
    - 발사 가능: `Ready`
    - 그 외: `Hidden`
11. 계산 결과를 `LocalAimState`에 저장한다.
12. 기존 getter `GetLocalAimState()`와 `GetReticleState()` 또는 신규 getter로 상태를 읽을 수 있게 한다.

## Out of Scope

이번 작업에서 하지 않는다.

- `ServerRequestFire()` 구현 금지
- `InputAction_Fire` 생성 금지
- Fire Request 생성 금지
- Server Aim State 계산 금지
- Server Fire 검증 금지
- HitScan 구현 금지
- Damage 구현 금지
- Projectile 구현 금지
- `UCFVehicleWeaponComp` 생성 금지
- VehicleDebug Aim 카테고리 생성 금지
- Reticle UI 생성 금지
- BP 에셋 수정 금지
- Replication 설정 금지
- PoliceCar 전용 하드코딩 금지
- CameraComp 내부 로직 수정 금지

## Target Files

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`

참고 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
- `UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`

기존 파일 중 `CFVehiclePawn.h/.cpp`는 이번 작업에서 수정하지 않는다. R1/R2에서 이미 AimComp가 Pawn에 연결되어 있기 때문이다.

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. 변경 대상은 기본적으로 `CFVehicleAimComp.h/.cpp`로 제한한다.
3. 기존 `CFVehicleAimTypes.h`의 struct 필드명을 바꾸지 않는다.
4. 기존 `ACFVehiclePawn` 생성자와 런타임 초기화 흐름을 변경하지 않는다.
5. 기존 `UCFVehicleCameraComp` 책임을 AimComp로 옮기지 않는다.
6. `UCFVehicleCameraComp` 내부 코드를 수정하지 않는다.
7. Tick은 AimComp 내부 Local Aim 계산 용도로만 사용한다.
8. Tick에서 서버 RPC, DebugPanel, UI, Damage, HitScan을 호출하지 않는다.
9. CameraComp가 없거나 Aim Runtime이 준비되지 않은 경우 크래시하지 않는다.
10. Owner Pawn과 CameraComp 참조는 기존 `InitializeAimRuntime()` / `RefreshAimRuntimeReferences()` 흐름을 재사용한다.
11. `DefaultAimProfile` 기본값은 이번 작업에서 임의 변경하지 않는다.
12. `LastAimRuntimeSummary`는 필요할 때 Local Aim 갱신 실패 이유를 짧게 기록할 수 있다.
13. `UFUNCTION`과 `UPROPERTY`를 추가할 경우 한국어 `ToolTip`을 작성한다.
14. 빌드 실패 상태로 끝내지 않는다.

## Implementation Notes

권장 함수 후보는 다음과 같다.

- `virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;`
- `void RefreshLocalAimState(float DeltaSeconds);`
- `bool CalculateAimAnglesRelativeToVehicle(const FVector& AimDirection, float& OutYawDeg, float& OutPitchDeg) const;`
- `bool IsAimWithinDefaultProfile(float AimYawDeg, float AimPitchDeg) const;`
- `ECFVehicleReticleState BuildLocalReticleState(bool bWithinWeaponArc, bool bAimBlocked, bool bCanFire) const;`
- `ECFVehicleReticleState GetReticleState() const;`

방향 계산 권장 방식:

1. CameraRuntimeState의 `AimHitLocation`을 읽는다.
2. 기준 시작점은 Owner Pawn의 Actor Location을 사용한다.
3. `AimHitLocation - OwnerLocation`을 정규화해 `LocalAimDirection`을 만든다.
4. 길이가 너무 작으면 Owner Pawn의 Forward Vector를 fallback으로 사용한다.
5. Owner Pawn의 Transform 기준 inverse 방향으로 변환한다.
6. 차량 로컬 기준 Yaw는 `atan2(LocalDirection.Y, LocalDirection.X)`로 계산한다.
7. 차량 로컬 기준 Pitch는 `atan2(LocalDirection.Z, sqrt(X^2 + Y^2))`로 계산한다.
8. 각도 단위는 degree를 사용한다.

Reticle 계산 권장 우선순위:

1. Aim runtime not ready -> `Hidden`
2. `bLocalAimBlocked == true` -> `Blocked`
3. `bLocalWithinWeaponArc == false` -> `OutOfArc`
4. `bLocalCanFire == true` -> `Ready`
5. fallback -> `Hidden`

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `UCFVehicleAimComp`의 Tick이 활성화되어 있다.
2. `TickComponent`가 구현되어 있고 Super 호출을 포함한다.
3. Tick에서 `RefreshLocalAimState(DeltaTime)`가 호출된다.
4. `RefreshLocalAimState()`가 CameraComp의 `GetCameraRuntimeState()`를 읽는다.
5. `LocalAimState.LocalAimTargetLocation`이 CameraRuntimeState의 `AimHitLocation`으로 갱신된다.
6. `LocalAimState.LocalAimDirection`이 유효한 방향으로 갱신된다.
7. `LocalAimState.bLocalWithinWeaponArc`가 `DefaultAimProfile` 기준으로 계산된다.
8. `LocalAimState.bLocalAimBlocked`가 CameraRuntimeState의 `bAimBlocked`를 반영한다.
9. `LocalAimState.bLocalCanFire`가 `bLocalWithinWeaponArc && !bLocalAimBlocked` 기준으로 계산된다.
10. `LocalAimState.LocalReticleState`가 `Hidden / Blocked / OutOfArc / Ready` 중 하나로 계산된다.
11. Aim Runtime이 준비되지 않았거나 CameraComp가 없어도 크래시하지 않는다.
12. 기존 `ServerAimState`와 `RepAimVisualState`는 이번 작업에서 계산하지 않는다.
13. 기존 Pawn, CameraComp, VehicleDebug 코드는 변경되지 않는다.
14. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 수정 파일은 원칙적으로 `CFVehicleAimComp.h`와 `CFVehicleAimComp.cpp`여야 한다.
3. `CFVehiclePawn.h/.cpp`가 이번 작업에서 변경되지 않아야 한다.
4. `CFVehicleCameraComp.h/.cpp`가 이번 작업에서 변경되지 않아야 한다.
5. Single Player PIE에서 크래시 없이 시작되어야 한다.
6. CameraComp가 준비된 경우 LocalAimState가 Tick에서 갱신되어야 한다.
7. AimTarget이 Zero에 고정되지 않고 CameraRuntimeState의 AimHitLocation을 따라갈 수 있어야 한다.
8. 조준각 밖에서는 `OutOfArc`가 나올 수 있어야 한다.
9. AimBlocked 상태에서는 `Blocked`가 우선되어야 한다.
10. 서버 RPC, DebugPanel, Reticle UI, Damage 관련 코드가 추가되지 않았는지 확인한다.

## Notes

R1/R2 확인 결과 현재 `DefaultAimProfile` 기본값은 다음과 같다.

- `ProfileName = DefaultAimProfile`
- `MinYawDeg = -20.0f`
- `MaxYawDeg = 20.0f`
- `MinPitchDeg = -10.0f`
- `MaxPitchDeg = 10.0f`
- `MaxAimDistance = 10000.0f`

R3에서는 이 기본값을 임의로 바꾸지 않는다. 기본값 조정은 별도 튜닝 단계에서 처리한다.

## Unresolved

없음.
