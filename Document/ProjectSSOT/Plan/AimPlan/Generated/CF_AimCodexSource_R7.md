# Task Source — CarFight Aim Commit E

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: Server HitScan 더미
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2 — Aim 타입 정의 + AimComp 골격, R3 — Local Aim 계산, R4 — VehicleDebug Aim, R5/R6 — Fire Request 더미 + Server Fire 검증
- 이번 단계: R7 — Server HitScan 더미

## Goal

CarFight 프로젝트의 Aim 기반 서버 발사 검증 흐름에 Server HitScan 더미 Trace를 추가한다.

이번 작업은 `CF_AimRoadmap.md`의 `Commit E — Server HitScan 더미` 범위만 수행한다.

목표는 서버가 발사 요청을 승인한 경우 서버 권한으로 LineTrace를 실행하고, Trace 결과를 `FCFVehicleFireResult`와 `FCFVehicleServerAimState`에 반영해 VehicleDebug에서 확인할 수 있게 하는 것이다.

이번 작업은 실제 Damage, Projectile, Multicast FX, Reticle UI, WeaponComp를 구현하지 않는다.

## Current Baseline

현재 기준은 다음과 같다.

- `ACFVehiclePawn`은 `InputAction_Fire`를 보유한다.
- `HandleFireStarted()`는 FireRequest를 만들고 서버 RPC를 호출한다.
- `ServerRequestFire_Implementation()`은 `ValidateFireRequestOnServer()`를 호출한다.
- `ValidateFireRequestOnServer()`는 요청을 검증하고 `FCFVehicleFireResult`를 채운다.
- 현재 `ValidateFireRequestOnServer()`는 실제 서버 Trace를 하지 않고 `ServerHitLocation`을 `PredictedAimTargetLocation`으로 둔다.
- `ClientReceiveFireResult_Implementation()`은 결과를 Owner Client에 반영한다.
- `UCFVehicleAimComp::BuildServerAimStateFromFireRequest()`와 `ApplyServerFireResult()`가 존재한다.
- VehicleDebug Aim은 Server Aim State와 Reject Reason을 표시한다.
- R5/R6 이후 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. 서버에서만 실행되는 HitScan 더미 Trace helper를 추가한다.
2. `ValidateFireRequestOnServer()`가 발사 요청 승인 조건을 통과한 뒤 서버 Trace를 수행하게 한다.
3. Trace 시작점은 `FireRequest.AimOrigin`을 우선 사용한다.
4. Trace 방향은 `FireRequest.AimDirection.GetSafeNormal()`을 사용한다.
5. Trace 거리는 `VehicleAimComp->GetDefaultAimProfile().MaxAimDistance`를 사용한다.
6. Trace 끝점은 `AimOrigin + AimDirection * MaxAimDistance`로 계산한다.
7. Trace는 서버 World에서 `LineTraceSingleByChannel()`을 사용한다.
8. Collision Channel은 1차 더미이므로 `ECC_Visibility`를 사용한다.
9. Trace Query Params에는 Owner Pawn을 ignored actor로 추가한다.
10. Trace가 Hit되면 `FCFVehicleFireResult.ServerHitLocation`과 `ServerHitNormal`에 Hit 결과를 넣는다.
11. Trace가 Miss되면 `ServerHitLocation`은 Trace End로 넣고 `ServerHitNormal`은 `FVector::UpVector` 또는 `-AimDirection` 후보 중 하나로 일관되게 설정한다.
12. Trace 결과와 무관하게, 이번 단계에서는 유효 발사 요청이면 Damage 없이 승인 상태를 유지한다.
13. 선택적으로 서버 Debug Draw를 켜고 끌 수 있는 C++ bool 설정을 추가한다.
14. VehicleDebug Aim에서 ServerAimTargetLocation 또는 ServerHitLocation에 가까운 값을 확인할 수 있도록 기존 FireResult 반영 경로를 유지한다.

## Out of Scope

이번 작업에서 하지 않는다.

- Damage 적용 금지
- Projectile 구현 금지
- Multicast Fire FX 구현 금지
- RepAimVisual 복제 구현 금지
- Reticle UI 생성 금지
- `UCFVehicleWeaponComp` 생성 금지
- Ammo / Cooldown / Reload 구현 금지
- Hit Actor를 기반으로 게임플레이 결과 처리 금지
- BP 에셋 수정 금지
- Input Mapping Context 에셋 수정 금지
- WBP Designer 수정 금지
- Lock-On 구현 금지
- AI Combat 구현 금지

## Target Files

수정 파일:

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

필요 시 수정 가능:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`

단, 가능하면 AimComp 타입/구조는 변경하지 않는다. R7의 핵심은 서버 Trace 결과를 기존 `FCFVehicleFireResult`에 채우는 것이다.

참고 파일:

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`

## Constraints

반드시 지킬 제약은 다음과 같다.

1. 파일명은 32자를 넘기지 않는다.
2. 서버 Trace는 서버 권한 경로에서만 실행한다.
3. `ValidateFireRequestOnServer()`의 기존 거부 검증 순서를 유지한다.
4. 요청이 거부된 경우 서버 Trace를 실행하지 않는다.
5. 클라이언트가 보낸 Hit 결과를 신뢰하지 않는다. Hit 결과는 서버 Trace로만 만든다.
6. Trace에서 Owner Pawn을 ignored actor로 추가한다.
7. Damage, Projectile, Multicast FX, WeaponComp를 구현하지 않는다.
8. Trace 실패를 곧바로 Fire Reject로 처리하지 않는다. 이번 단계에서는 더미 HitScan 시각/디버그 결과만 만든다.
9. 기존 Fire Request / Result / ServerAimState 흐름을 깨지 않는다.
10. 기존 주행 / 카메라 / VehicleDebug 기능을 변경하지 않는다.
11. 빌드 실패 상태로 끝내지 않는다.

## Recommended Pawn Additions

`ACFVehiclePawn`에 다음을 추가하는 것을 권장한다.

UPROPERTY:

- `bDrawServerAimTraceDebug: bool`
- `ServerAimTraceDebugDuration: float`

함수:

- `bool RunServerDummyHitScan(const FCFVehicleFireRequest& FireRequest, FCFVehicleFireResult& InOutFireResult) const;`

권장 Category:

- `CarFight|VehiclePawn|Aim`

권장 Tooltip:

- `bDrawServerAimTraceDebug`: 서버 HitScan 더미 Trace를 디버그 라인으로 표시할지 여부입니다.
- `ServerAimTraceDebugDuration`: 서버 HitScan 더미 Trace 디버그 라인을 표시할 시간입니다.

## Recommended Server Trace Flow

`ValidateFireRequestOnServer()`에서 기존 검증을 모두 통과한 뒤 다음 순서로 처리한다.

1. `OutFireResult.bAccepted = true`
2. `OutFireResult.RejectReason = ECFVehicleFireRejectReason::None`
3. `RunServerDummyHitScan(FireRequest, OutFireResult)` 호출
4. Trace Hit 시:
   - `OutFireResult.ServerAimTargetLocation = Hit.ImpactPoint`
   - `OutFireResult.ServerHitLocation = Hit.ImpactPoint`
   - `OutFireResult.ServerHitNormal = Hit.ImpactNormal`
5. Trace Miss 시:
   - `OutFireResult.ServerAimTargetLocation = TraceEnd`
   - `OutFireResult.ServerHitLocation = TraceEnd`
   - `OutFireResult.ServerHitNormal = FVector::UpVector` 또는 `-AimDirection`
6. 이번 단계에서는 Hit Actor를 저장하거나 Damage를 적용하지 않는다.

## Debug Draw Rules

Debug Draw는 선택 옵션이다.

- `bDrawServerAimTraceDebug == true`일 때만 그린다.
- Hit 시 Trace 라인과 Impact Point를 표시한다.
- Miss 시 Trace 라인만 표시해도 된다.
- 색상 지정은 프로젝트 기존 스타일이 있으면 따른다.
- Debug Draw는 서버에서만 실행한다.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `ValidateFireRequestOnServer()`의 기존 검증이 유지되어 있다.
2. 기존 검증 실패 시 서버 Trace가 실행되지 않는다.
3. 검증 성공 시 서버에서 LineTrace가 실행된다.
4. Trace 시작점은 `FireRequest.AimOrigin` 기준이다.
5. Trace 방향은 `FireRequest.AimDirection` 기준이다.
6. Trace 거리는 `DefaultAimProfile.MaxAimDistance` 기준이다.
7. Trace Hit 결과가 `FCFVehicleFireResult.ServerHitLocation`과 `ServerHitNormal`에 반영된다.
8. Trace Miss 결과도 일관된 위치와 노멀로 `FCFVehicleFireResult`에 반영된다.
9. `ServerAimTargetLocation`이 서버 Trace 결과 기준으로 갱신된다.
10. `ClientReceiveFireResult_Implementation()`과 `AimComp->ApplyServerFireResult()` 경로가 유지된다.
11. VehicleDebug Aim에서 서버 결과 위치 또는 서버 상태를 확인할 수 있다.
12. Damage / Projectile / Multicast FX / WeaponComp 구현이 추가되지 않았다.
13. 프로젝트가 C++ 빌드된다.

## Verification

검증 기준은 다음과 같다.

1. `CarFight_ReEditor / Win64 / Development` 빌드가 성공해야 한다.
2. 수정 파일은 기본적으로 `CFVehiclePawn.h/.cpp`여야 한다.
3. `CFVehicleAimTypes.h`의 기존 필드명을 변경하지 않아야 한다.
4. Single Player PIE에서 크래시 없이 시작되어야 한다.
5. Listen Server + 1 Client에서 Fire Action이 연결된 경우 서버 Trace가 실행될 수 있어야 한다.
6. Trace 결과가 `FCFVehicleFireResult`와 `FCFVehicleServerAimState`에 반영되어야 한다.
7. VehicleDebug Aim에서 Server Aim 위치와 Last Accepted/Rejected ID가 유지되어야 한다.
8. Damage / Projectile / Multicast FX 코드가 없음을 확인한다.

## Notes

R7은 서버 권한 HitScan 더미 단계다. 실제 무기 연출이나 데미지는 다음 단계가 아니다.

R7이 통과하면 다음 작업은 `R8/R9 — RepAimVisual / Reticle UI` 또는 프로젝트 상황에 따라 `Fire FX Multicast` 분리 단계로 갈 수 있다.

## Unresolved

없음.
