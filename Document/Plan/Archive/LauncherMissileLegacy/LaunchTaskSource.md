# LM-P0-01 TaskSource

- Version: 0.1.0
- Date: 2026-07-28
- Status: Ready for Direct Implementation
- Feature: `CF-FQ-029`
- Task: `LM-P0-01 Projectile Launch Handoff Foundation`

---

## 1. 작업 목적

현재 단일 Muzzle 직사 Projectile의 결과를 변경하지 않으면서, 플레이어가 지시한 목표와 Projectile의 실제 초기 발사 상태를 별도 계약으로 전달할 기반을 추가한다.

```text
현재
AimOrigin + AimDirection
→ Projectile Spawn / Activate

목표
Fire Command
→ Direct Legacy Launch Context 생성
→ Projectile Pool
→ Projectile Actor가 Context 복사
```

이 Task에서는 Multi-Muzzle, Ripple, Salvo, Angled/Vertical Ejection과 Guidance를 구현하지 않는다.

---

## 2. 현재 확인된 코드 근거

```text
UCFTurretMountData v1.6.0
- MuzzleSocketName 단일 필드

FCFVehicleFireRequest
- AimOrigin
- AimDirection
- PredictedAimTargetLocation
- WeaponGroupId

FCFVehicleFireOrigin
- WorldFireLocation
- WorldFireDirection

ACFVehiclePawn::TrySpawnProjectileActorFromFireCommand
- FireCommand.AimOrigin과 AimDirection으로 SpawnTransform 생성

UCFProjectilePoolComp::AcquireProjectile
- ProjectileData, SpawnTransform, LaunchDirection, InstigatorActor 전달

ACFProjectileActor::ActivateProjectile
- ProjectileData, LaunchDirection, InstigatorActor 전달

UCFProjectileData
- PropulsionConfig

UCFProjectileMotorComp
- FixedThrustDirection
```

---

## 3. 첫 Task의 확정 범위

### 신규 타입

```text
UE/Source/CarFight_Re/Public/CFProjectileLaunchTypes.h
```

계획 enum:

```text
ECFProjectileReleaseMode
- Direct
- AngledEjection
- VerticalEjection
```

첫 Task 런타임 사용값은 `Direct`다.

계획 구조체:

```text
FCFProjectileLaunchContext
```

최소 필드:

```text
LaunchTransform
InitialLaunchDirection
InitialLaunchVelocity
InheritedCarrierVelocity
CommandTargetLocation
ReleaseMode
FireRequestId
WeaponGroupId
```

`GuidanceTargetActor`와 `GuidanceTargetLocation`은 첫 Task에 포함할 수 있으나 실제 소비하지 않는다. UHT, 약한 참조와 직렬화 위험이 있으면 후속 Task로 미룬다.

### 기존 경로 연결

```text
ACFVehiclePawn
→ Direct Launch Context 생성

UCFProjectilePoolComp
→ Context 기반 Acquire 경로

ACFProjectileActor
→ Context 기반 Activate 경로와 Runtime 복사
```

---

## 4. 호환 전략

기존 함수 시그니처를 한 번에 제거하지 않는다.

허용 전략:

```text
A. 신규 Context 오버로드 추가
   + 기존 함수가 Context를 만들어 신규 함수 호출

B. 기존 함수를 유지하고 내부 전용 Context 함수 추가
```

선호:

```text
신규 Context 오버로드
+ Legacy Adapter
```

기존 Blueprint 노출 API가 있다면 호환을 우선한다.

---

## 5. Direct Legacy Context 규칙

```text
LaunchTransform.Location = FireCommand.AimOrigin
LaunchTransform.Rotation = FireCommand.AimDirection.Rotation
InitialLaunchDirection = SafeNormal(FireCommand.AimDirection)
CommandTargetLocation = FireCommand.PredictedAimTargetLocation
ReleaseMode = Direct
FireRequestId = FireCommand.FireRequestId
WeaponGroupId = FireCommand.WeaponGroupId
```

초기 Velocity는 기존 ProjectileData의 `InitialSpeed`를 기준으로 생성한다.

```text
InitialLaunchVelocity
= InitialLaunchDirection * ProjectileData.InitialSpeed
```

첫 Task에서는 차량 속도를 새로 상속하지 않는다.

```text
InheritedCarrierVelocity = Zero
```

이 결정은 기존 직사 Projectile의 운동 회귀를 막기 위한 호환 기준이다.
신규 LauncherData 단계에서 `CarrierVelocityRatio`를 도입한다.

---

## 6. Projectile Actor 계약

Actor는 Context를 자신의 활성화 Runtime 데이터로 복사한다.

계획 Debug Getter 또는 Summary:

```text
GetActiveLaunchContext
BuildProjectileLaunchSummary
```

Blueprint 노출은 실제 디버그 필요성과 USTRUCT 안전성을 확인한 뒤 결정한다.

Deactivate에서 다음 값을 Reset한다.

```text
ActiveLaunchContext
bHasActiveLaunchContext
```

Pool 반환 뒤 이전 FireRequestId와 목표가 남지 않아야 한다.

---

## 7. 초기 Velocity 적용

`ApplyProjectileMovement`가 LaunchDirection과 ProjectileData.InitialSpeed만 사용하는 현재 계약을 확인하고 다음 중 안전한 최소 변경을 선택한다.

```text
A. Context.InitialLaunchVelocity를 직접 적용
B. 첫 Task에서는 Direction과 InitialSpeed로 기존 결과를 유지하고 Context는 기록만 사용
```

권장:

```text
Context.InitialLaunchVelocity를 적용하되
Legacy Context가 기존 Direction * InitialSpeed를 생성하여 결과가 동일하도록 한다.
```

ProjectileMovement `InitialSpeed`, `MaxSpeed`, Propulsion maximum과 충돌하지 않도록 공식 빌드와 Automation에서 확인한다.

---

## 8. 변경 허용 파일

신규:

```text
UE/Source/CarFight_Re/Public/CFProjectileLaunchTypes.h
UE/Source/CarFight_Re/Private/CFProjectileLaunchTests.cpp
```

수정 후보:

```text
UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h
UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

실제 수정 파일은 구현 전 현재 diff를 다시 확인해 최소화한다.

---

## 9. 보호 파일과 금지 변경

```text
- CFTurretMountData는 LM-P0-02 전까지 변경하지 않는다.
- CFWeaponData는 변경하지 않는다.
- TargetSelect 코드 변경 금지.
- ProjectileMotorComp의 FixedThrustDirection 변경 금지.
- CombatFx, Damage와 VehicleHealth 변경 금지.
- ProjectilePool 전체 재작성 금지.
- .uasset 변경 금지.
- Config 변경 금지.
- Audio 추가 금지.
```

---

## 10. 품질 기준

```text
- 신규 파일명 32자 이하
- 모든 변수와 함수 위 역할 주석
- Blueprint 노출 항목 한국어 DisplayName·ToolTip
- 버전, Changelog, Migration
- Null / NaN / Zero Direction 안전 처리
- 기존 함수 호환
- Pool Reset
- Tick 검색·할당 추가 금지
```

---

## 11. Automation 계약

계획 테스트:

```text
CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract
```

검증:

```text
- ReleaseMode 기본 Direct
- Context 기본값 안전
- Legacy Context 위치·방향·속도
- Pool Acquire Context 전달
- Actor Active Context 보존
- Deactivate Reset
- 비유도 Rocket Propulsion Start 방향 회귀
- 기존 Projectile InitialSpeed 회귀
```

---

## 12. 완료 조건

```text
- 신규 타입과 Context 전달 경로 적용
- 기존 단일 Muzzle 직사 결과 동일
- scoped Git diff 검수 PASS
- Tools\BuildEditor.bat PASS
- Automation PASS 또는 Runner 미노출을 명확히 구분
- 사용자 PIE 대상과 절차 제공
- 대표 Plan 체크포인트 갱신
```

---

## 13. 실패 조건

```text
- 기존 Projectile 속도 또는 방향 변경
- Pool 재사용 시 이전 Context 잔류
- Fixed Rocket 추진 방향 회귀
- 첫 Impact / Damage 횟수 변경
- Blueprint Asset 재저장 요구
- unrelated dirty 파일 변경
```

---

## 14. Changelog

### v0.1.0 - 2026-07-28

```text
- LM-P0-01 Launch Handoff Foundation의 근거와 최소 범위를 작성했다.
- Legacy Direct Context와 초기 Velocity 호환 규칙을 확정했다.
- 변경 허용 파일, 보호 범위, Automation과 완료·실패 조건을 정의했다.
```

---

## 15. Migration

```text
- 기존 AcquireProjectile과 ActivateProjectile 함수는 Legacy Adapter로 유지한다.
- 기존 FireRequest 필드를 삭제하지 않는다.
- 차량 속도 상속과 Multi-Muzzle는 후속 Task로 분리한다.
```
