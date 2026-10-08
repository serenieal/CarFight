# Task Source — CarFight Aim Commit D

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: Fire Request 더미 + Server Fire 검증
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2 — Aim 타입 정의 + AimComp 골격, R3 — Local Aim 계산, R4 — VehicleDebug Aim
- 이번 단계: R5/R6 — Fire Request 더미 + Server Fire 검증

## Goal

CarFight 프로젝트에 Aim 기반 발사 요청의 1차 서버 검증 흐름을 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 다음 범위만 수행한다.

- R5 — Fire Request 더미
- R6 — Server Fire 검증

목표는 실제 무기 시스템 완성이 아니라, 클라이언트가 발사 요청을 만들고 서버가 그 요청을 승인/거부한 뒤 Owner Client가 결과를 받을 수 있는 최소 C++ 경로를 여는 것이다.

이번 작업에서 HitScan, Damage, Projectile, Reticle UI, BP 에셋 생성은 구현하지 않는다.

## Current Baseline

현재 기준은 다음과 같다.

- `ACFVehiclePawn`은 `VehicleAimComp`를 소유한다.
- `UCFVehicleAimComp`는 Local Aim State를 Tick에서 계산한다.
- `FCFVehicleFireRequest`와 `FCFVehicleFireResult`는 `CFVehicleAimTypes.h`에 이미 정의되어 있다.
- `FCFVehicleServerAimState`에는 다음 필드가 있다.
  - `ServerAimTargetLocation`
  - `bServerWithinWeaponArc`
  - `bServerCanFire`
  - `LastServerRejectReason`
  - `LastAcceptedFireRequestId`
  - `LastRejectedFireRequestId`
- `FCFVehicleDebugAim`은 VehicleDebug Snapshot과 Panel에 연결되어 있다.
- 기존 입력 액션은 `ACFVehiclePawn::SetupPlayerInputComponent()`에서 Enhanced Input으로 바인딩한다.
- 현재 `InputAction_Fire`는 없다.
- 현재 실제 `IA_Fire` 에셋도 생성하지 않는다.
- R4 이후 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `ACFVehiclePawn`에 `InputAction_Fire` UPROPERTY를 추가한다.
2. `SetupPlayerInputComponent()`에서 `InputAction_Fire`가 유효할 때 Started 입력을 `HandleFireStarted()`에 바인딩한다.
3. `HandleFireStarted(const FInputActionValue& InputActionValue)`를 추가한다.
4. `BuildFireRequest()` 함수를 추가한다.
5. `NextFireRequestId` 또는 동등한 요청 ID 카운터를 추가한다.
6. `LastFireRequest`와 `LastFireResult` 디버그용 캐시를 추가한다.
7. `ServerRequestFire(const FCFVehicleFireRequest& FireRequest)` RPC를 추가한다.
8. `ClientReceiveFireResult(const FCFVehicleFireResult& FireResult)` RPC를 추가한다.
9. 서버 검증 함수 `ValidateFireRequestOnServer()` 또는 동등한 helper를 추가한다.
10. 서버 검증 성공 시 `FCFVehicleFireResult.bAccepted = true`를 반환한다.
11. 서버 검증 실패 시 적절한 `ECFVehicleFireRejectReason`을 반환한다.
12. 서버 검증 결과를 `VehicleAimComp` 또는 Pawn의 Debug 상태에 반영할 최소 경로를 추가한다.
13. `FCFVehicleDebugAim` 또는 VehicleDebug Aim 표시에서 Last Fire Request / Result / Reject Reason을 확인할 수 있게 한다.
14. 서버 검증은 최소한 다음을 검사한다.
    - 요청 Pawn 소유권 또는 Controller 유효성
    - AimComp 유효성
    - Aim runtime ready
    - AimDirection 유효성
    - AimOrigin 유효성
    - 로컬/기본 AimProfile 기준 Weapon Arc 유효성
    - Aim blocked 여부

## Out of Scope

이번 작업에서 하지 않는다.

- `IA_Fire` InputAction 에셋 생성 금지
- Input Mapping Context 에셋 수정 금지
- BP 에셋 수정 금지
- Reticle UI 생성 금지
- HitScan Trace 구현 금지
- Projectile 구현 금지
- Damage 구현 금지
- `UCFVehicleWeaponComp` 생성 금지
- Ammo / Cooldown / Reload 완성 구현 금지
- Multicast Fire FX 구현 금지
- RepAimVisual 복제 구현 금지
- Lock-On 구현 금지
- AI Combat 구현 금지
- VehicleDebug Panel WBP Designer 수정 금지

## Target Files

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`

필요하면 수정 가능:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`

단, `CFVehicleAimTypes.h`의 기존 필드명을 변경하지 않는다. 필요 시 Debug 표시용 필드 추가만 허용한다.

참고 파일:

- `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`

이번 작업에서 UI Panel 코드는 가능하면 수정하지 않는다. R4에서 이미 Last Reject Reason과 Server Aim State를 표시하고 있기 때문이다.

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. 기존 주행/조향/카메라 입력 동작을 변경하지 않는다.
3. `InputAction_Fire`가 null이면 아무 입력 바인딩도 하지 않고 크래시하지 않는다.
4. 이번 작업에서 Fire Input Action 에셋을 생성하지 않는다.
5. `ServerRequestFire`는 서버 권한 경로만 담당한다.
6. 클라이언트가 보낸 Fire Request를 그대로 신뢰하지 않는다.
7. 서버에서 최소 검증 후 `FCFVehicleFireResult`를 만든다.
8. HitScan/Damage/Projectile 로직을 넣지 않는다.
9. 서버 검증 실패 이유는 `ECFVehicleFireRejectReason`으로 남긴다.
10. `ClientReceiveFireResult`는 Owner Client의 Debug 상태 갱신만 수행한다.
11. `UCFVehicleAimComp`는 서버 상태 저장과 검증 보조까지만 확장한다.
12. `UCFVehicleAimComp` 안에 WeaponComp 완성 책임을 넣지 않는다.
13. RPC는 Unreal 규칙에 맞게 `UFUNCTION(Server, Reliable)` / `UFUNCTION(Client, Reliable)` 형태로 선언한다.
14. `_Implementation` 함수명을 정확히 구현한다.
15. 빌드 실패 상태로 끝내지 않는다.

## Recommended Pawn Additions

`ACFVehiclePawn`에 다음을 추가하는 것을 권장한다.

UPROPERTY:

- `InputAction_Fire: TObjectPtr<UInputAction>`
- `NextFireRequestId: int32`
- `LastFireRequest: FCFVehicleFireRequest`
- `LastFireResult: FCFVehicleFireResult`

함수:

- `HandleFireStarted(const FInputActionValue& InputActionValue)`
- `BuildFireRequest() const` 또는 `BuildFireRequest()`
- `ServerRequestFire(const FCFVehicleFireRequest& FireRequest)`
- `ServerRequestFire_Implementation(const FCFVehicleFireRequest& FireRequest)`
- `ClientReceiveFireResult(const FCFVehicleFireResult& FireResult)`
- `ClientReceiveFireResult_Implementation(const FCFVehicleFireResult& FireResult)`
- `ValidateFireRequestOnServer(const FCFVehicleFireRequest& FireRequest, FCFVehicleFireResult& OutFireResult)`

## Recommended AimComp Additions

`UCFVehicleAimComp`에 다음을 추가하는 것을 권장한다.

함수:

- `BuildFireRequest(int32 FireRequestId, float ClientFireTimeSeconds) const`
- `ApplyServerFireResult(const FCFVehicleFireResult& FireResult)`
- `BuildServerAimStateFromFireRequest(const FCFVehicleFireRequest& FireRequest, ECFVehicleFireRejectReason RejectReason, bool bAccepted)`
- `IsFireRequestWithinDefaultProfile(const FCFVehicleFireRequest& FireRequest) const`

역할:

- `BuildFireRequest()`는 LocalAimState를 기준으로 AimOrigin/AimDirection/PredictedAimTargetLocation을 채운다.
- `ApplyServerFireResult()`는 `ServerAimState`에 LastAccepted/Rejected Request ID와 RejectReason을 반영한다.
- 서버 검증 자체의 최종 판단은 Pawn 서버 RPC 또는 helper가 수행한다.

## Recommended Server Validation Rules

서버 검증은 최소 다음 순서로 처리한다.

1. FireResult 기본값 생성
2. `FireResult.FireRequestId = FireRequest.FireRequestId`
3. 서버 권한 확인: 권한 없으면 `NoAuthority`
4. Controller / Owner 유효성 확인: 실패 시 `InvalidOwner`
5. AimComp 유효성 확인: 실패 시 `NoWeapon` 또는 `VehicleDisabled`
6. Aim runtime ready 확인: 실패 시 `VehicleDisabled`
7. AimDirection이 거의 zero인지 확인: 실패 시 `InvalidAimDirection`
8. AimOrigin이 유효한 월드 위치인지 확인: 실패 시 `InvalidAimOrigin`
9. AimDirection이 DefaultAimProfile 조준각 안인지 확인: 실패 시 `OutOfWeaponArc`
10. Aim blocked 상태 확인: 막힘이면 `AimBlocked`
11. 통과하면 `bAccepted = true`, `RejectReason = None`
12. 이번 단계에서는 서버 Trace를 하지 않는다. `ServerHitLocation`은 Zero 또는 Request target을 그대로 둘 수 있다.

## Debug Requirements

R4에서 이미 VehicleDebug Aim Panel은 다음을 표시한다.

- Server Can Fire
- Server Within Weapon Arc
- Last Server Reject Reason
- Last Accepted Fire Request Id
- Last Rejected Fire Request Id

이번 작업은 서버 검증 결과가 위 필드에 반영되도록 해야 한다.

`FCFVehicleDebugAim` 또는 `FCFVehicleServerAimState`를 통해 확인 가능해야 한다.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `InputAction_Fire` UPROPERTY가 `ACFVehiclePawn`에 추가되어 있다.
2. `SetupPlayerInputComponent()`에서 Fire 입력 바인딩이 추가되어 있다.
3. `InputAction_Fire == nullptr`일 때 크래시하지 않는다.
4. `HandleFireStarted()`가 발사 요청을 생성한다.
5. 클라이언트에서 호출된 `HandleFireStarted()`는 서버 RPC를 호출할 수 있다.
6. 서버에서 호출된 경우 직접 서버 검증을 수행할 수 있다.
7. `ServerRequestFire_Implementation()`이 존재한다.
8. `ClientReceiveFireResult_Implementation()`이 존재한다.
9. 서버 검증 실패 시 적절한 RejectReason이 기록된다.
10. 서버 검증 성공 시 `bAccepted = true`, `RejectReason = None`이 된다.
11. `UCFVehicleAimComp` 또는 Pawn을 통해 `FCFVehicleServerAimState`가 갱신된다.
12. VehicleDebug Aim Section에서 Last Reject Reason / Last Accepted ID / Last Rejected ID를 확인할 수 있다.
13. HitScan/Damage/Projectile/Reticle UI/WeaponComp 코드는 추가되지 않는다.
14. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 수정 파일은 기본적으로 Target Files 4개여야 한다.
3. `CFVehicleDebugPanelWidget.h/.cpp`는 가능하면 변경하지 않아야 한다.
4. `InputAction_Fire`가 비어 있어도 기존 입력 바인딩이 유지되어야 한다.
5. Single Player PIE에서 크래시 없이 시작되어야 한다.
6. Listen Server + 1 Client에서 Fire Action이 연결된 경우 ServerRequestFire가 호출될 수 있어야 한다.
7. 서버 검증 결과가 `FCFVehicleServerAimState`에 반영되어야 한다.
8. VehicleDebug Aim에서 Last Reject Reason / Last Accepted ID / Last Rejected ID가 확인 가능해야 한다.
9. HitScan/Damage/Projectile/Multicast FX 코드가 없음을 확인한다.

## Notes

이번 단계는 Fire Request와 Server Validation의 골격 단계다.

`IA_Fire` 에셋과 Input Mapping Context 연결은 BP/에셋 작업이므로 이번 Codex 작업에 포함하지 않는다. 에셋 연결은 C++ 경로가 빌드되고 Debug 상태가 확인된 뒤 별도 단계에서 처리한다.

R5/R6이 통과하면 다음 작업은 `R7 — Server HitScan 더미`다.

## Unresolved

없음.
