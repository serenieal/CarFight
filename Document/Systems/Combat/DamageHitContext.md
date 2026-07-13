# DamageHitContext

- Version: 1.0.0
- Date: 2026-07-09
- Status: Current
- Scope: 현재 구현된 DamageData, DamageHitContext 타입, Dummy HitScan / Projectile 충돌 Debug 기록

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `DamageHitContext` 구현이 실제로 어떤 일을 하는지 기록한다.

이 문서는 미래 Damage Runtime 설계서가 아니다.
아래 항목은 이 문서의 범위에서 제외한다.

```text
- 실제 HP 차감
- 차량 파괴 상태 전환
- 장갑 관통 계산
- 모듈 손상 계산
- 범위 피해 적용
- 물리 충격 적용
- 피격 VFX / SFX 출력
- 서버 권한 피해 처리
```

현재 문서 기준 `DamageHitContext`는 **Dummy HitScan과 Projectile Actor 충돌 결과를 같은 형식으로 기록하고, VehicleDebug Panel에서 읽을 수 있게 하는 Debug 중심의 피해 후보 컨텍스트 기능**이다.

---

## 2. 현재 구현 범위

| 구분 | 현재 구현 |
| --- | --- |
| 피해 데이터 | `UCFDamageData` |
| 피해 타입 enum | `ECFDamageType` |
| 명중 컨텍스트 구조 | `FCFDamageHitContext` |
| Dummy HitScan 기록 함수 | `ACFVehiclePawn::RecordDummyHitScanDamageHitContext` |
| Projectile 충돌 기록 함수 | `ACFVehiclePawn::RecordProjectileDamageHitContextFromPool` |
| 최종 저장 함수 | `ACFVehiclePawn::StoreLastDamageHitContext` |
| Debug 요약 생성 | `ACFVehiclePawn::BuildDamageHitContextSummary` |
| Debug 저장 필드 | `LastDamageHitContext`, `bHasLastDamageHitContext`, `LastDamageHitContextSummary` |

---

## 3. 현재 DamageData가 제공하는 값

현재 `UCFDamageData`는 ProjectileData가 참조할 최소 피해 데이터다.

현재 주요 필드는 아래와 같다.

```text
DamageId
DamageType
BaseDamage
ArmorPenetration
bUseRadialDamage
ExplosionRadius
ExplosionInnerRadius
ExplosionDamage
MinExplosionDamageScale
ModuleDamageScale
ImpulseStrength
```

현재 실제로 중요한 점은 다음이다.

```text
- DamageData 직접 참조 슬롯은 ProjectileData.DefaultDamageData 하나다.
- WeaponData는 DamageData를 직접 소유하지 않는다.
- HitScan / Laser처럼 Actor를 스폰하지 않는 발사도 가상 ProjectileData를 통해 DamageData를 참조하는 기준이다.
- 현재 P0 코드에서는 DamageData를 실제 HP 차감에 사용하지 않고 Debug 확인에 사용한다.
```

현재 `BaseDamage`, `ArmorPenetration`, `ExplosionDamage`, `ImpulseStrength` 등은 데이터 필드로 존재하지만, 이 문서 기준 현재 런타임 피해 계산을 수행하지 않는다.

---

## 4. 현재 DamageHitContext 구조

현재 `FCFDamageHitContext`는 Projectile / HitScan 명중 결과를 같은 형식으로 기록하기 위한 최소 런타임 컨텍스트다.

현재 필드는 아래와 같다.

```text
DamageData
DamageId
WeaponId
ProjectileId
HitActor
ImpactLocation
ImpactNormal
IncomingDirection
InstigatorActor
FlightDurationSeconds
bFromProjectileActor
bBlockingHit
```

각 필드의 현재 의미는 아래다.

| 필드 | 현재 의미 |
| --- | --- |
| `DamageData` | 이번 명중에서 적용 후보로 해석된 DamageData |
| `DamageId` | DamageData가 없을 때도 Debug에 남길 피해 ID |
| `WeaponId` | 이번 명중을 만든 무기 ID |
| `ProjectileId` | 이번 명중을 만든 발사체 ID |
| `HitActor` | 맞은 Actor. Miss이면 None일 수 있음 |
| `ImpactLocation` | Hit 또는 Trace 끝점 위치 |
| `ImpactNormal` | Hit 표면 노멀 또는 fallback UpVector |
| `IncomingDirection` | 탄이 들어온 방향 |
| `InstigatorActor` | 발사 주체 Actor |
| `FlightDurationSeconds` | Projectile Actor 비행 시간. Dummy HitScan은 0 |
| `bFromProjectileActor` | Projectile Actor 충돌에서 온 기록인지 여부 |
| `bBlockingHit` | 실제 Hit / 충돌이 있었는지 여부 |

---

## 5. Dummy HitScan 경로의 현재 기록 방식

`ACFVehiclePawn::RunLocalDummyHitScan`은 로컬 Trace 결과를 만든 뒤 `RecordDummyHitScanDamageHitContext`를 호출한다.

현재 Dummy HitScan 기록은 아래 정보를 사용한다.

```text
FireCommand
FireResult
HitResult 또는 nullptr
bBlockingHit
VehicleWeaponComp의 ActiveDamageData
VehicleWeaponComp의 ActiveDamageId
VehicleWeaponComp의 ActiveWeaponData
VehicleWeaponComp의 ActiveProjectileData
```

Hit이 있으면 `HitActor`, `ImpactPoint`, `ImpactNormal`을 기록한다.
Hit이 없으면 Trace 끝점과 fallback 노멀을 기록한다.

Dummy HitScan의 현재 특징은 아래다.

```text
- bFromProjectileActor = false
- FlightDurationSeconds = 0.0
- bBlockingHit으로 Hit / Miss를 구분한다.
- 실제 피해 적용은 하지 않는다.
```

---

## 6. Projectile Actor 경로의 현재 기록 방식

Projectile Actor가 Pool로 반환될 때 `UCFProjectilePoolComp::ReleaseProjectile`은 Hit 사유를 읽고 Owner Pawn에 기록을 전달할 수 있다.

차량 Pawn은 `RecordProjectileDamageHitContextFromPool`에서 Projectile Actor의 마지막 충돌 정보를 읽어 `FCFDamageHitContext`를 만든다.

현재 Projectile Actor 경로가 사용하는 정보는 아래다.

```text
LastDeactivatedProjectileData
DefaultDamageData
DamageProfileId
ActiveWeaponData
LastDeactivatedProjectileId
LastHitActor
LastImpactLocation
LastImpactNormal
LastIncomingDirection
LastDeactivatedInstigatorActor
LastFlightDurationSeconds
```

Projectile Actor 경로의 현재 특징은 아래다.

```text
- bFromProjectileActor = true
- bBlockingHit = true
- FlightDurationSeconds에 실제 비행 시간이 기록된다.
- 실제 피해 적용은 하지 않는다.
```

---

## 7. 현재 저장 및 표시 방식

현재 최종 저장은 `ACFVehiclePawn::StoreLastDamageHitContext`가 담당한다.

저장되는 값은 아래다.

```text
LastDamageHitContext
bHasLastDamageHitContext
LastDamageHitContextSummary
```

`BuildDamageHitContextSummary`는 VehicleDebug Panel에 표시할 문자열을 만든다.
현재 요약 문자열에는 아래 정보가 포함된다.

```text
Source
Hit
DamageId
Weapon
Projectile
HitActor
Instigator
Location
Normal
Incoming
Flight
```

현재 이 문자열은 Debug 확인용이며, 전투 결과 UI나 최종 로그 포맷으로 고정된 것은 아니다.

---

## 8. 현재 데이터 해석 경로

현재 피해 데이터 연결은 아래 단일 경로를 기준으로 한다.

```text
UCFEquipmentPresetData.DefaultWeaponData
→ UCFWeaponData.DefaultProjectileData
→ UCFProjectileData.DefaultDamageData
→ UCFDamageData
```

`UCFProjectileData.DefaultDamageData`가 비어 있으면 `DamageProfileId`가 Debug fallback 피해 ID로 표시될 수 있다.

현재 `UCFWeaponData.BaseDamage`와 `UCFWeaponData.DamageProfileId`는 레거시 확인용 값이며, 현재 DamageData 해석 경로에는 사용하지 않는다.

---

## 9. 현재 기능 책임

현재 `DamageHitContext` 기능의 책임은 아래로 제한한다.

```text
- DamageData 참조와 DamageId를 Debug 컨텍스트에 기록한다.
- Dummy HitScan 결과와 Projectile Actor 충돌 결과를 같은 구조로 기록한다.
- Hit / Miss를 bBlockingHit으로 구분한다.
- Projectile Actor 경로와 Dummy HitScan 경로를 bFromProjectileActor로 구분한다.
- 피격 위치, 노멀, 입사 방향, 발사 주체, 피격 Actor를 저장한다.
- VehicleDebug Panel에서 읽을 요약 문자열을 만든다.
```

---

## 10. 현재 기준 비책임 항목

현재 `DamageHitContext`는 아래를 직접 수행하지 않는다.

```text
- CurrentHealth 감소
- MaxHealth 관리
- Destroyed 상태 전환
- 피해량 최종 계산
- 장갑 관통 계산
- 폭발 범위 피해 계산
- 모듈 손상 적용
- 물리 충격 적용
- 피격 이펙트 / 사운드 출력
```

현재 `UCFDamageData` 안에 피해량, 관통력, 폭발, 모듈, 충격 관련 필드가 있더라도, 이 문서 기준으로는 **저장과 Debug 확인용 데이터**로만 본다.

---

## 11. 현재 문서 기준의 핵심 결론

현재 `DamageHitContext`는 **피해 처리 시스템이 아니라, 발사 결과가 무엇을 맞혔는지와 어떤 DamageData 후보를 사용할 수 있는지 기록하는 Debug 컨텍스트 기능**이다.

가장 중요한 현재 역할은 다음 한 줄로 요약할 수 있다.

> `DamageHitContext`는 현재 “HitScan과 Projectile 충돌 결과를 같은 형태로 저장하고 VehicleDebug에서 읽을 수 있게 만드는 현재 구현 기능”이다.

---

## 12. 현재 미확인 항목

아래 항목은 코드상 경로는 존재하지만, 이 문서 작성 시점에 에디터 자산 연결 상태를 직접 확인하지 않았다.

```text
- 실제 ProjectileData 에셋의 DefaultDamageData 연결 상태
- 실제 DamageData 에셋의 BaseDamage / DamageType 값
- PIE에서 Dummy HitScan 명중 시 LastDamageHitContextSummary 표시 여부
- PIE에서 Projectile Actor 충돌 시 LastDamageHitContextSummary 표시 여부
- 충돌 대상 Actor의 Collision 설정이 HitContext 기록에 적합한지 여부
```

이 항목은 추측으로 PASS 처리하지 않는다.

---

## 13. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- FCFDamageHitContext 필드 변경
- UCFDamageData의 현재 런타임 사용 범위 변경
- Dummy HitScan HitContext 기록 방식 변경
- Projectile Actor HitContext 기록 방식 변경
- VehicleDebug Damage 표시 방식 변경
- 실제 HP 차감 기능이 별도 시스템으로 추가되어 현재 책임 경계가 바뀌는 경우
```

---

## 14. Changelog

### v1.0.0 - 2026-07-09

```text
- 현재 구현된 DamageHitContext Systems 문서 최초 작성
- UCFDamageData, ECFDamageType, FCFDamageHitContext, Dummy HitScan 기록, Projectile Actor 충돌 기록, VehicleDebug 요약 범위를 현재 코드 기준으로 정리
- 실제 HP 차감, 파괴, 장갑, 모듈, 폭발, 물리 충격 적용은 현재 비책임으로 분리
```

---

## 15. 마지막 확인 기준

- 확인 일시: 2026-07-09
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFDamageData.h`
  - `UE/Source/CarFight_Re/Public/CFDamageTypes.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
