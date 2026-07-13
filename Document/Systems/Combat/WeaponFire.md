# WeaponFire

- Version: 1.1.0
- Date: 2026-07-09
- Status: Current / Single Player Local Fire
- Scope: 현재 구현된 차량 로컬 발사 명령, 무기 데이터 해석, 터렛 FireOrigin, 쿨다운, HitScan / Projectile 분기, 발사 결과 기록과 UI 피드백 연결 기준

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `WeaponFire` 구현이 실제로 어떤 일을 하는지 기록한다.

이 문서는 미래 설계서가 아니다.
현재 문서 기준 `WeaponFire`는 **싱글플레이 로컬 Pawn에서 Fire 입력을 받아 발사 명령을 만들고, 무기/장착/발사체 데이터를 해석한 뒤 HitScan 또는 Projectile 실행 경로로 넘기며, 그 결과를 Aim / Debug / 후속 UI 피드백이 읽을 수 있게 기록하는 기능**이다.

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 서버 권한 발사, 복제, 2클라 검증, 서버 대기 UI는 현재 구현 범위로 보지 않는다.

---

## 2. 현재 범위 제외 항목

아래 항목은 현재 `WeaponFire` 문서의 완료 범위에서 제외한다.

```text
- 실제 HP 차감
- 차량 파괴 / 사망 상태
- 장갑 방향 계산
- 모듈 손상
- 폭발 범위 피해 적용
- 서버 권한 발사
- 멀티플레이 복제
- 발사 VFX / SFX 완성
- 복잡한 전투 HUD 완성
- 무기 종류 대량 확장
```

위 항목들은 현재 코드에 데이터 필드 또는 Debug 후보가 일부 존재하더라도, 현재 `WeaponFire` 완료 기능으로 보지 않는다.

---

## 3. 현재 구현 범위

현재 `WeaponFire`는 아래 C++ 타입과 함수 묶음으로 동작한다.

| 구분 | 현재 구현 |
| --- | --- |
| 차량 발사 주체 | `ACFVehiclePawn` |
| 조준 입력 공급 | `UCFVehicleAimComp` |
| 무기 / 장착 해석 | `UCFVehicleWeaponComp` |
| 무기 데이터 | `UCFWeaponData` |
| 장비 프리셋 데이터 | `UCFEquipmentPresetData` |
| 터렛 마운트 데이터 | `UCFTurretMountData` |
| 발사체 데이터 | `UCFProjectileData` |
| 발사 원점 타입 | `FCFVehicleFireOrigin` |
| 발사 명령 타입 | `FCFVehicleFireRequest` |
| 발사 결과 타입 | `FCFVehicleFireResult` |
| 발사 입력 함수 | `ACFVehiclePawn::HandleFireStarted` |
| 발사 명령 생성 | `ACFVehiclePawn::BuildFireCommand` |
| 발사 검증 | `ACFVehiclePawn::ValidateFireCommand` |
| 결과 반영 | `ACFVehiclePawn::ApplyFireResult` |

---

## 4. 현재 실제로 하는 일

### 4.1 Fire 입력을 로컬 발사 명령으로 변환

현재 `ACFVehiclePawn::HandleFireStarted`는 Fire 입력이 들어오면 아래 흐름을 실행한다.

```text
HandleFireStarted
→ BuildFireCommand
→ ValidateFireCommand
→ Projectile Actor 사용 가능 시 TrySpawnProjectileActorFromFireCommand
→ Projectile Actor 사용 불가 또는 확보 실패 시 RunLocalDummyHitScan
→ ApplyFireResult
```

현재 발사 흐름은 싱글플레이 로컬 기준이다.
기존 서버 RPC 발사 흐름은 현재 기본 실행 경로가 아니다.

### 4.2 Aim 상태에서 기본 FireRequest 생성

`BuildFireCommand`는 현재 `VehicleAimComp`가 있으면 `VehicleAimComp->BuildFireRequest`를 통해 발사 명령을 만든다.

`VehicleAimComp`가 없으면 크래시 없이 아래 fallback 값을 사용한다.

```text
AimOrigin = ActorLocation
AimDirection = ActorForwardVector
PredictedAimTargetLocation = ActorLocation + ActorForwardVector * 1000
```

이 fallback은 기능 완성용이 아니라 현재 로컬 발사 경로가 안전하게 실패하거나 최소 동작을 유지하기 위한 방어 경로다.

### 4.3 VehicleWeaponComp로 실제 발사 원점 계산

`VehicleWeaponComp`가 있으면 `BuildFireCommand`는 `UCFVehicleWeaponComp::BuildFireOrigin`을 호출한다.

현재 FireOrigin 계산은 아래 정보를 사용한다.

```text
VehicleData
→ MountProfiles
→ ActiveMountProfileId
→ HardpointSlots
→ EquipmentPresetData
→ TurretMountData / WeaponData
→ FireOrigin
```

현재 기본 활성 장착 프로파일 ID는 `RoofTurret_MediumOrLarge`다.
현재 기본 위치 슬롯 참조는 `Top_01` 계열 하드포인트를 기준으로 해석된다.

### 4.4 Muzzle Socket이 있으면 총구 기준으로 보정

`BuildFireCommand`는 `VehicleWeaponComp`가 계산한 하드포인트 FireOrigin을 그대로 쓰지 않고, 조건이 맞으면 `TryBuildMuzzleFireOrigin`으로 총구 기준 보정을 시도한다.

현재 Muzzle 보정 조건은 아래와 같다.

```text
- LastTurretMountData가 유효하다.
- TurretPitchMeshComp가 존재한다.
- TurretPitchMeshComp에 StaticMesh가 있다.
- TurretMountData.MuzzleSocketName이 비어 있지 않다.
- Pitch 메쉬에 해당 Muzzle 소켓이 존재한다.
```

조건을 만족하면 최종 FireOrigin은 Pitch 메쉬의 `Muzzle` 소켓 위치와 소켓 X축 방향을 우선 사용한다.
조건을 만족하지 않으면 기존 하드포인트 FireOrigin fallback을 유지한다.

### 4.5 발사 가능 조건 검증

`ValidateFireCommand`는 현재 아래 조건을 검사한다.

```text
- Controller 존재 여부
- VehicleAimComp 존재 여부
- Aim runtime 준비 여부
- AimDirection 유효성
- AimOrigin 유효성
- Pawn 위치와 AimOrigin 사이 거리
- LocalAimState.bLocalAimBlocked 여부
- 활성 WeaponData 호환 여부
- 활성 무기 쿨다운 여부
```

검증 실패 시 `FCFVehicleFireResult.RejectReason`에 실패 사유를 기록한다.
현재 `ECFVehicleFireRejectReason` enum은 아래 값을 가진다.

```text
None
InvalidLocalState
InvalidOwner
VehicleDisabled
NoWeapon
WeaponCooldown
NoAmmo
OutOfWeaponArc
AimBlocked
InvalidAimOrigin
InvalidAimDirection
TraceMiss
```

현재 `ValidateFireCommand`에서 실제로 핵심 사용되는 거부 사유는 아래 범주다.

```text
InvalidOwner
NoWeapon
VehicleDisabled
InvalidAimDirection
InvalidAimOrigin
AimBlocked
WeaponCooldown
None
```

`OutOfWeaponArc`는 호환용 enum 값으로 남아 있지만, 현재 P0 싱글플레이 기준에서는 조준각 초과만으로 발사를 막는 기본 거부 사유로 사용하지 않는다.
`OutOfArc` / `bLocalWithinWeaponArc` 정책은 `Document/Systems/Vehicles/VehicleAim.md`를 우선 기준으로 본다.

### 4.6 WeaponData 기반 발사 제한

현재 `UCFWeaponData`는 아래 값을 공급한다.

```text
WeaponId
WeaponSize
CompatibleMountTypes
FireMode
FireRatePerMinute
MaxRange
SpreadDeg
MagazineSize
ReloadTimeSeconds
HeatPerShot
MaxHeat
DefaultProjectileData
ProjectileDataId
```

다만 현재 실제 발사 검증에서 핵심적으로 쓰이는 값은 아래다.

```text
- 장착 타입 호환성
- 무기 크기 호환성
- FireMode
- FireRatePerMinute 기반 발사 간격
- MaxRange
- DefaultProjectileData
```

현재 `MagazineSize`, `ReloadTimeSeconds`, `HeatPerShot`, `MaxHeat`는 데이터 필드로 존재하지만 실제 탄창 / 재장전 / 과열 런타임을 수행하지 않는다.

### 4.7 Projectile Actor 또는 Dummy HitScan 분기

검증이 통과하면 현재 발사 경로는 두 가지 중 하나로 간다.

```text
1. Projectile Actor 경로
2. Dummy HitScan fallback 경로
```

Projectile Actor 경로 조건은 `ShouldUseProjectileActorFire`가 결정한다.
현재 조건은 `VehicleWeaponComp->IsActiveProjectileSpawnReady()`다.

Projectile Actor 경로가 가능하면 `TrySpawnProjectileActorFromFireCommand`가 `ProjectilePoolComp->AcquireProjectile`을 호출한다.
Projectile Actor 확보에 실패하면 `RunLocalDummyHitScan`으로 fallback한다.

### 4.8 발사 결과를 Aim / Debug 상태에 반영

`ApplyFireResult`는 현재 아래 일을 한다.

```text
- LastFireRequest 저장
- LastFireResult 저장
- 발사 성공 시 VehicleWeaponComp에 승인 발사 시각 기록
- VehicleAimComp의 FireValidationState 갱신
- VehicleAimComp의 FireResult 적용
- VehicleAimComp의 AimVisualState 갱신
```

현재 이 함수는 실제 대상 체력 차감이나 파괴 처리를 하지 않는다.

---

## 5. 현재 데이터 연결 구조

현재 장비/무기/발사체/피해 데이터 연결은 아래 단일 경로를 따른다.

```text
FCFVehicleMountProfile.DefaultEquipmentPresetData
→ UCFEquipmentPresetData.DefaultTurretMountData
→ UCFEquipmentPresetData.DefaultWeaponData
→ UCFWeaponData.DefaultProjectileData
→ UCFProjectileData.DefaultDamageData
```

현재 `MountProfile`에서 `WeaponData`나 `TurretMountData`를 직접 fallback으로 읽는 경로는 제거된 기준이다.
`EquipmentPresetData` 내부 참조가 비어 있으면 해당 항목은 `Missing` 상태로 표시된다.

---

## 6. 현재 Debug / 확인 가능 항목

현재 `VehicleDebug`의 Weapon 카테고리에서 아래 항목을 읽을 수 있다.

```text
bHasVehicleWeaponComponent
bWeaponRuntimeReady
ActiveMountProfileId
LastFireOrigin
LastWeaponRuntimeSummary
ActiveEquipmentPresetData
bActiveEquipmentPresetDataAssigned
bActiveEquipmentPresetDataCompatible
ActiveEquipmentPresetId
ActiveEquipmentPresetSummary
ActiveTurretMountData
bActiveTurretMountDataAssigned
ActiveTurretMountId
ActiveTurretMountSummary
bTurretVisualAttached
TurretVisualSummary
TurretState
TurretRuntimeSummary
ActiveWeaponData
ActiveWeaponId
bActiveWeaponDataCompatible
ActiveWeaponSummary
ActiveProjectileData
ActiveProjectileId
ActiveProjectileSummary
bActiveProjectileSpawnReady
ActiveProjectileExecutionSummary
ActiveDamageData
ActiveDamageId
ActiveDamageSummary
ActiveDamageResolutionSummary
bHasLastDamageHitContext
LastDamageHitContext
LastDamageHitContextSummary
ProjectilePool 상태
ActiveWeaponMaxRange
ActiveWeaponFireRatePerMinute
ActiveWeaponCooldownSeconds
ActiveWeaponRemainingCooldownSeconds
LastAcceptedWeaponFireTimeSeconds
```

이 Debug 항목은 현재 구현 상태 확인용이다.
전투 밸런스나 최종 UI 표시 체계로 해석하지 않는다.

---

## 7. WeaponFire 결과와 UI 표시 연결 기준

`WeaponFire`는 판정과 결과 기록을 담당한다.
`AimReticle`과 `FireFeedback`은 `WeaponFire`의 결과를 읽어 플레이어가 인지할 수 있는 표시로 변환한다.

책임 분리 기준은 아래와 같다.

```text
WeaponFire = Fire 입력 처리, 발사 가능 검증, 실행 경로 결정, 결과 기록
AimReticle = 로컬 조준 상태와 간단한 발사 결과 상태 표시
FireFeedback = 발사 성공/실패/쿨다운을 UI/VFX/SFX 피드백으로 변환
```

따라서 UI는 `WeaponFire`의 판정 결과를 임의로 바꾸지 않는다.
피드백이 없어도 판정은 Debug / FireResult / FireValidationState로 추적 가능해야 한다.

### 7.1 UI 표시 후보 데이터

Reticle 또는 FireFeedback이 읽을 수 있는 대표 후보 데이터는 아래와 같다.

```text
- LastFireRequest
- LastFireResult
- LastFireResult.bAccepted
- LastFireResult.RejectReason
- FireValidationState.bValidationCanFire
- FireValidationState.LastValidationRejectReason
- AimVisualState.bIsFiringVisual
- ActiveWeaponCooldownSeconds
- ActiveWeaponRemainingCooldownSeconds
- LastAcceptedWeaponFireTimeSeconds
```

### 7.2 RejectReason -> UI 표시 후보 매핑

현재 Reticle / FireFeedback 표시 후보는 아래처럼 둔다.

| RejectReason / 결과 | UI 표시 후보 | 현재 의미 |
| --- | --- | --- |
| `None` + `bAccepted == true` | 발사 성공 피드백 | 정상 발사 |
| `InvalidLocalState` | `FireRejected` | 로컬 상태가 발사 처리에 부적합 |
| `InvalidOwner` | `FireRejected` | Pawn / Controller 상태 문제 |
| `VehicleDisabled` | `FireRejected` | 차량 또는 Aim 런타임 준비 안 됨 |
| `NoWeapon` | `NoWeapon` | 무기 없음 또는 무기 데이터 비호환 |
| `WeaponCooldown` | `Cooldown` | 연사 제한 / 쿨다운 |
| `NoAmmo` | `FireRejected` 또는 `Reloading` | 현재 탄약 시스템 미구현. 후속 상태 |
| `OutOfWeaponArc` | `OutOfArc` | 호환용 상태. 현재 기본 거부 조건 아님 |
| `AimBlocked` | `Blocked` | 조준선 막힘 |
| `InvalidAimOrigin` | `FireRejected` | 발사 원점 비정상 |
| `InvalidAimDirection` | `FireRejected` | 발사 방향 비정상 |
| `TraceMiss` | 별도 Miss 피드백 후보 | 현재 피해 판정 후속 상태 |

현재 P0 싱글플레이 기준에서 `OutOfWeaponArc`는 enum 호환용 상태다.
기본 발사 거부 조건으로 승격하려면 `VehicleAim`, `WeaponFire`, `AimReticle`, `FireFeedback` 문서를 함께 갱신해야 한다.

### 7.3 Cooldown 표시 기준

쿨다운 UI는 아래 값을 우선 후보로 사용한다.

```text
ActiveWeaponCooldownSeconds
ActiveWeaponRemainingCooldownSeconds
LastAcceptedWeaponFireTimeSeconds
```

표시 비율은 후속 UI에서 아래처럼 계산할 수 있다.

```text
CooldownRatio = ActiveWeaponRemainingCooldownSeconds / ActiveWeaponCooldownSeconds
```

단, `ActiveWeaponCooldownSeconds <= 0`이면 쿨다운 표시를 하지 않거나 0으로 처리해야 한다.

---

## 8. 현재 기능 책임

현재 `WeaponFire`의 책임은 아래로 제한한다.

```text
- Fire 입력을 로컬 발사 명령으로 변환한다.
- Aim 상태와 Weapon 상태를 기준으로 발사 가능 여부를 검증한다.
- MountProfile / EquipmentPresetData / WeaponData / ProjectileData를 해석한다.
- 하드포인트 또는 Muzzle 소켓 기준 발사 원점과 방향을 만든다.
- 쿨다운을 기록하고 검증한다.
- Projectile Actor 실행 경로와 Dummy HitScan fallback 경로를 분기한다.
- 발사 결과를 AimComp와 VehicleDebug 상태에 반영한다.
- Reticle / FireFeedback이 읽을 수 있는 발사 결과와 거부 사유를 기록한다.
```

---

## 9. 현재 기준 비책임 항목

현재 `WeaponFire`는 아래를 직접 수행하지 않는다.

```text
- 실제 HP 차감
- 차량 파괴 처리
- 장갑 관통 계산
- 모듈 손상 계산
- 탄창 소모
- 재장전 상태 전이
- 열 누적 / 과열 상태 전이
- 서버 권한 검증
- 멀티플레이 복제
- 완성된 발사 이펙트 / 사운드 출력
- Reticle WBP 표시 스타일 결정
- FireFeedback 연출 품질 결정
```

---

## 10. 현재 문서 기준의 핵심 결론

현재 `WeaponFire`는 **싱글플레이 로컬 차량 Pawn에서 조준 상태와 장비 데이터를 읽어 발사 명령을 만들고, 발사 가능 여부와 실행 경로를 결정한 뒤 그 결과를 Aim / Debug / 후속 UI 피드백이 읽을 수 있게 남기는 전투 진입 기능**이다.

가장 중요한 현재 역할은 다음 한 줄로 요약할 수 있다.

> `WeaponFire`는 현재 “쏠 수 있는가, 어디서 어떤 방향으로 쏘는가, Projectile Actor를 쓸 것인가 HitScan fallback을 쓸 것인가, 그리고 왜 실패했는가”를 결정하고 기록하는 기능이다.

---

## 11. 현재 미확인 항목

아래 항목은 코드상 경로는 존재하지만, 이 문서 작성 시점에 에디터 자산 연결 상태를 직접 확인하지 않았다.

```text
- BP_CFVehiclePawn의 InputAction_Fire 실제 자산 연결 상태
- 기준 VehicleData의 Active MountProfile 실제 DataAsset 연결 상태
- EquipmentPresetData / TurretMountData / WeaponData / ProjectileData / DamageData 실제 에셋 연결 상태
- 기준 터렛 Pitch 메쉬의 Muzzle 소켓 실제 존재 여부
- PIE에서 Fire 입력을 눌렀을 때 VehicleDebug Panel에 표시되는 실제 런타임 값
- AimReticle / FireFeedback이 LastFireResult, RejectReason, Cooldown 값을 소비하는 최종 UI 경로
```

이 항목은 추측으로 PASS 처리하지 않는다.

---

## 12. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- BuildFireCommand / ValidateFireCommand / ApplyFireResult 흐름 변경
- VehicleWeaponComp의 데이터 해석 경로 변경
- EquipmentPresetData 기준 연결 정책 변경
- FireOrigin 또는 Muzzle Socket 보정 방식 변경
- Projectile Actor와 Dummy HitScan 분기 조건 변경
- Weapon Debug Snapshot 필드 변경
- 실제 피해 적용이 WeaponFire 안으로 들어오는 구조 변경
- RejectReason 종류 또는 의미 변경
- OutOfWeaponArc를 실제 발사 거부 조건으로 사용하게 될 때
- AimReticle / FireFeedback이 WeaponFire 결과를 읽는 최종 경로가 확정될 때
```

---

## 13. 문서 버전 관리

- 현재 문서 버전: `1.1.0`
- 문서 상태: `Current / Single Player Local Fire`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준

- `Major`
  - WeaponFire가 실제 피해/파괴/전투 결과 처리까지 직접 담당하게 될 때
  - 서버 권한 발사 또는 복제 기준으로 기능 해석이 바뀔 때
- `Minor`
  - RejectReason, FireResult, Debug Snapshot, Projectile 분기, UI 피드백 연결 기준이 추가될 때
  - OutOfWeaponArc 같은 거부 사유 운영 정책이 바뀔 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

---

## 14. Migration

### v1.0.0 -> v1.1.0

```text
- WeaponFire는 판정/기록 담당으로 명시한다.
- AimReticle / FireFeedback은 표시 담당으로 분리한다.
- RejectReason을 UI 표시 후보로 변환하는 기준을 추가한다.
- OutOfWeaponArc는 현재 P0 싱글플레이 기준에서 기본 발사 거부 조건으로 사용하지 않는다고 명시한다.
- Cooldown UI는 ActiveWeaponCooldownSeconds / ActiveWeaponRemainingCooldownSeconds를 우선 후보로 본다.
```

---

## 15. Changelog

### v1.1.0 - 2026-07-09

```text
- WeaponFire / AimReticle / FireFeedback 책임 분리 기준 추가
- RejectReason -> UI 표시 후보 매핑 추가
- Cooldown 표시 후보 데이터와 비율 계산 기준 추가
- OutOfWeaponArc가 현재 기본 발사 거부 조건이 아님을 명시
- UI 피드백 경로 미확정 항목과 문서 갱신 조건 추가
```

### v1.0.0 - 2026-07-09

```text
- 현재 구현된 WeaponFire Systems 문서 최초 작성
- ACFVehiclePawn 로컬 Fire 흐름, VehicleWeaponComp 데이터 해석, Muzzle FireOrigin 보정, Projectile / HitScan 분기, Debug 표시 범위를 현재 코드 기준으로 정리
- 실제 HP 차감, 파괴, 서버 권한 발사, VFX/SFX 완성 항목은 현재 비책임으로 분리
```

---

## 16. 마지막 확인 기준

- 확인 일시: `2026-07-09`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleWeaponTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `UE/Source/CarFight_Re/Public/CFEquipmentPresetData.h`
  - `UE/Source/CarFight_Re/Public/CFTurretMountData.h`
  - `UE/Source/CarFight_Re/Public/CFWeaponData.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/UI/AimReticle.md`
