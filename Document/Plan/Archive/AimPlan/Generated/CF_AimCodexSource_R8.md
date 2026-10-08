# Task Source — CarFight Aim Commit F1

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: Replicated Aim Visual State 기반 추가
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2 — Aim 타입 정의 + AimComp 골격, R3 — Local Aim 계산, R4 — VehicleDebug Aim, R5/R6 — Fire Request 더미 + Server Fire 검증, R7 — Server HitScan 더미
- 이번 단계: R8 — RepAimVisual

## Goal

CarFight 프로젝트의 Aim 시스템에 다른 클라이언트가 볼 수 있는 최소 조준 시각 상태 복제 기반을 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 `R8 — RepAimVisual` 범위만 수행한다.

목표는 `UCFVehicleAimComp`가 서버 기준의 조준 방향/목표 위치/발사 시각 상태를 `FCFVehicleRepAimVisualState`로 관리하고, Replication 준비 및 OnRep 흐름을 여는 것이다.

이번 작업은 Reticle UI, Fire FX Multicast, Damage, Projectile, WeaponComp를 구현하지 않는다.

## Current Baseline

현재 기준은 다음과 같다.

- `UCFVehicleAimComp`는 `LocalAimState`, `ServerAimState`, `RepAimVisualState`를 보유한다.
- `FCFVehicleRepAimVisualState`에는 다음 필드가 있다.
  - `RepAimDirection`
  - `RepAimTargetLocation`
  - `bIsFiringVisual`
  - `RepWeaponVisualMode`
- `ACFVehiclePawn::ServerRequestFire_Implementation()`은 서버 검증 후 `VehicleAimComp->BuildServerAimStateFromFireRequest()`와 `VehicleAimComp->ApplyServerFireResult()`를 호출한다.
- `ACFVehiclePawn::RunServerDummyHitScan()`은 서버 Trace 결과를 `FCFVehicleFireResult`에 채운다.
- VehicleDebug Aim은 `RepAimVisualState`를 이미 표시한다.
- 현재 `ACFVehiclePawn`에는 명시적인 `GetLifetimeReplicatedProps()`가 없다.
- 현재 R7 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `UCFVehicleAimComp`를 replication 가능한 컴포넌트로 설정한다.
2. `RepAimVisualState`를 `ReplicatedUsing=OnRep_RepAimVisualState`로 설정한다.
3. `UCFVehicleAimComp::GetLifetimeReplicatedProps()`를 추가한다.
4. 필요한 include로 `Net/UnrealNetwork.h`를 cpp에 추가한다.
5. `OnRep_RepAimVisualState()` 함수를 추가한다.
6. 서버에서 `RepAimVisualState`를 갱신하는 함수 `UpdateRepAimVisualFromFireResult()` 또는 동등한 함수를 추가한다.
7. 서버 발사 검증 성공/실패 결과를 바탕으로 `RepAimVisualState`를 최소 갱신한다.
8. 승인된 FireResult에서는 다음 값을 반영한다.
   - `RepAimDirection`: FireRequest 또는 FireResult 기준 조준 방향
   - `RepAimTargetLocation`: ServerAimTargetLocation 또는 ServerHitLocation
   - `bIsFiringVisual`: true로 설정 가능
   - `RepWeaponVisualMode`: FireRequest.WeaponGroupId 또는 DefaultAimProfile.ProfileName
9. 거부된 FireResult에서는 `bIsFiringVisual = false`로 유지 또는 갱신한다.
10. `ACFVehiclePawn::ServerRequestFire_Implementation()`에서 서버 결과가 나온 후 AimComp의 RepAimVisual 갱신 함수를 호출한다.
11. VehicleDebug Aim에서 RepAimVisual 값이 갱신된 상태를 확인할 수 있게 기존 getter 경로를 유지한다.

## Out of Scope

이번 작업에서 하지 않는다.

- Reticle UI 생성 금지
- WBP 에셋 수정 금지
- BP 에셋 수정 금지
- Multicast Fire FX 구현 금지
- 실제 총구 이펙트 재생 금지
- Damage 적용 금지
- Projectile 구현 금지
- `UCFVehicleWeaponComp` 생성 금지
- Ammo / Cooldown / Reload 구현 금지
- Lock-On 구현 금지
- AI Combat 구현 금지
- 복잡한 네트워크 압축 최적화 금지
- Pawn 전체 네트워크 구조를 대규모로 재설계 금지

## Target Files

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

필요 시 수정 가능:

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`

단, 가능하면 `CFVehiclePawn.h`에는 새 API를 추가하지 않는다. R8의 핵심은 AimComp 내부의 RepAimVisual 기반이다.

참고 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. `FCFVehicleRepAimVisualState`의 기존 필드명을 변경하지 않는다.
3. 기존 Local Aim 계산을 변경하지 않는다.
4. 기존 Server Fire 검증 순서를 변경하지 않는다.
5. 기존 Server HitScan Trace 흐름을 변경하지 않는다.
6. RepAimVisual은 전투 판정용으로 사용하지 않는다.
7. RepAimVisual은 시각 표현과 Debug용 최소 상태로만 사용한다.
8. Damage, Projectile, Multicast FX, Reticle UI를 구현하지 않는다.
9. 클라이언트가 RepAimVisual을 직접 전투 결과로 확정하지 않게 한다.
10. `UCFVehicleAimComp`가 Owner Pawn을 찾지 못해도 크래시하지 않는다.
11. Replication 관련 include 순환을 만들지 않는다.
12. `GetLifetimeReplicatedProps()` 구현 시 Super 호출을 포함한다.
13. 빌드 실패 상태로 끝내지 않는다.

## Recommended AimComp Additions

`UCFVehicleAimComp`에 다음을 추가하는 것을 권장한다.

함수:

- `virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;`
- `UFUNCTION() void OnRep_RepAimVisualState();`
- `void UpdateRepAimVisualFromFireResult(const FCFVehicleFireRequest& FireRequest, const FCFVehicleFireResult& FireResult);`
- `void ClearRepAimFiringVisual();` 후보

생성자:

- `SetIsReplicatedByDefault(true);`

UPROPERTY:

- `RepAimVisualState`에 `ReplicatedUsing=OnRep_RepAimVisualState` 적용

## Recommended RepAimVisual Update Rules

서버에서 FireResult를 받은 후 다음 규칙으로 갱신한다.

1. `FireResult.bAccepted == true`인 경우:
   - `RepAimDirection = FireRequest.AimDirection`
   - `RepAimTargetLocation = FireResult.ServerAimTargetLocation`
   - `bIsFiringVisual = true`
   - `RepWeaponVisualMode = FireRequest.WeaponGroupId`
2. `FireResult.bAccepted == false`인 경우:
   - `RepAimDirection = FireRequest.AimDirection`
   - `RepAimTargetLocation = FireResult.ServerAimTargetLocation`
   - `bIsFiringVisual = false`
   - `RepWeaponVisualMode = FireRequest.WeaponGroupId`
3. FireRequest 방향이 거의 zero이면 기존 `LocalAimState.LocalAimDirection` 또는 Owner Pawn Forward Vector fallback을 사용한다.
4. FireResult 위치가 zero이면 FireRequest.PredictedAimTargetLocation fallback을 사용할 수 있다.

## Recommended Pawn Integration

`ACFVehiclePawn::ServerRequestFire_Implementation()`에서 서버 검증과 Trace가 끝난 뒤 다음 순서로 호출한다.

1. `VehicleAimComp->BuildServerAimStateFromFireRequest(...)`
2. `VehicleAimComp->ApplyServerFireResult(FireResult)`
3. `VehicleAimComp->UpdateRepAimVisualFromFireResult(FireRequest, FireResult)`
4. `ClientReceiveFireResult(FireResult)`

기존 호출 순서와 의미를 최대한 유지한다.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `UCFVehicleAimComp`가 replication 가능한 컴포넌트로 설정되어 있다.
2. `RepAimVisualState`가 `ReplicatedUsing=OnRep_RepAimVisualState`로 선언되어 있다.
3. `GetLifetimeReplicatedProps()`가 AimComp에 구현되어 있다.
4. `DOREPLIFETIME(UCFVehicleAimComp, RepAimVisualState)` 또는 동등한 복제 등록이 있다.
5. `OnRep_RepAimVisualState()`가 구현되어 있다.
6. 서버에서 FireResult 생성 후 `RepAimVisualState`가 갱신된다.
7. 승인된 발사 결과는 `bIsFiringVisual = true`로 반영될 수 있다.
8. 거부된 발사 결과는 `bIsFiringVisual = false`로 반영된다.
9. VehicleDebug Aim에서 Rep Aim Direction / Target / Firing Visual / Visual Mode 값이 유지 또는 갱신될 수 있다.
10. RepAimVisual이 Damage/Hit 판정에 사용되지 않는다.
11. Reticle UI / Multicast FX / Projectile / Damage 구현이 추가되지 않는다.
12. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 수정 파일은 기본적으로 `CFVehicleAimComp.h/.cpp`, `CFVehiclePawn.cpp`여야 한다.
3. `CFVehicleAimTypes.h`의 기존 필드명을 변경하지 않아야 한다.
4. Single Player PIE에서 크래시 없이 시작되어야 한다.
5. Listen Server + 1 Client에서 Fire Action이 연결된 경우 서버 FireResult 이후 RepAimVisualState가 갱신될 수 있어야 한다.
6. VehicleDebug Aim에서 RepAimVisual 값을 확인할 수 있어야 한다.
7. Damage / Projectile / Multicast FX / Reticle UI 코드가 없음을 확인한다.

## Notes

R8은 복제 시각 상태 기반만 여는 단계다.

R8이 통과하면 다음 작업은 `R9 — Reticle UI 최소 버전`이다. Reticle UI는 WBP/BP 연결이 걸리므로 R8과 분리한다.

## Unresolved

없음.
