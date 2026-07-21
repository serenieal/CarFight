# DamageHitContext

- Version: 1.5.0
- Date: 2026-07-15
- Status: Current / HitDamage Input Contract Verified
- Scope: DamageData와 DamageHitContext 타입, 시각 차체 기반 HitScan/Projectile 기록, 검증된 HitDamage 공용 입력 계약

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `DamageHitContext` 구현이 실제로 어떤 일을 하는지 기록한다.

이 문서는 `DamageHitContext` 자체의 기록 책임을 설명하며, 체력 소유와 피해 계산은 `Document/Systems/Combat/HitDamage.md`가 설명한다.
아래 항목은 `DamageHitContext` 자체의 직접 책임에서 제외한다.

```text
- CurrentHealth와 MaxHealth 소유
- 차량 파괴 상태 판정
- 장갑 관통 계산
- 모듈 손상 계산
- 범위 피해 적용
- 물리 충격 적용
- 피격 VFX / SFX 출력
- 서버 권한 피해 처리
```

현재 문서 기준 `DamageHitContext`는 **Dummy HitScan과 Projectile Actor의 피격 결과를 같은 형식으로 기록하고, HitDamage 공용 피해 적용 입력과 VehicleDebug 표시 데이터로 전달하는 런타임 컨텍스트 기능**이다.

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
- HitScan / Laser처럼 Actor를 스폰하지 않는 발사도 가상 ProjectileData를 통해 DamageData를 참조한다.
- BaseDamage와 bCanDamageSelf는 HitDamage의 최소 차량 체력 계산에 사용된다.
```

현재 `ArmorPenetration`, `ExplosionDamage`, `ModuleDamageScale`, `ImpulseStrength` 등은 데이터 필드로 존재하지만 아직 현재 최소 체력 계산에는 적용하지 않는다.

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
- 생성한 Context는 HitDamage 공용 진입점으로 전달되어 유효 대상에 BaseDamage를 적용한다.
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
LastHitComponentName
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
- 생성한 Context는 HitDamage 공용 진입점으로 전달되어 유효 대상에 BaseDamage를 적용한다.
```

## 6-1. 현재 차량 피격 컴포넌트 기준

2026-07-13 구현과 사용자 PIE 확인 결과, 현재 차량 무기 피격은 시각 차체 `SM_Body` 기준으로 전환됐다.

```text
WeaponHit Trace Channel        = ECC_GameTraceChannel1
Projectile Object Channel      = ECC_GameTraceChannel2
VehicleVisualHit Profile       = QueryOnly
VehicleMesh                    = WeaponHit / Projectile Ignore
SM_Body                        = WeaponHit / Projectile Block
Dummy HitScan                  = WeaponHit
Projectile                     = Projectile Object Type
```

현재 결과:

```text
- 일반 속도 Dummy HitScan이 SM_Body에서 명중한다.
- 일반 속도 Projectile이 SM_Body에서 Blocking Hit을 만든다.
- VehicleMesh Physics Asset은 차량 물리를 유지하면서 무기 피격 Query를 가로채지 않는다.
- FCFDamageHitContext.HitComponentName에 실제 피격 컴포넌트 이름이 기록된다.
- DamageHitContext Summary에서 HitComponent=SM_Body를 확인할 수 있다.
- 30 FPS + 기준 InitialSpeed 4배 Projectile이 차량과 얇은 벽에서 첫 Blocking Hit을 기록한다.
- 중복 Impact와 Pool 재사용에서 DamageHitContext 중복 또는 누락이 관찰되지 않았다.
```

현재 후속 한계:

```text
- BaseDamage 체력 감소와 파괴 상태는 CF-FQ-018에서 구현·검증 완료했다.
- 장갑·모듈·범위 피해와 파괴 연출은 아직 현재 최소 피해 범위가 아니다.
- 전체 FPS·속도 매트릭스, 이동 차량과 주행 중 조준은 CF-FQ-019 확장 회귀 범위다.
```

현재 후속 순서:

```text
1. CF-FQ-017 NoWeapon / AimBlocked 최종 상태 검증
2. CF-FQ-019 주행·전투 반복 및 확장 회귀
3. 장갑·모듈·범위 피해는 별도 FeatureQueue 승인 후 진행
```

관련 설계:

```text
Document/Plan/HitDamage/ImplementationDesign.md
Document/Plan/AimFireAlignment/ImplementationDesign.md
Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md
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
HitComponent
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
- 피격 위치, 노멀, 입사 방향, 발사 주체, 피격 Actor, 피격 컴포넌트 이름을 저장한다.
- 유효한 Context를 HitDamage 공용 피해 적용 입력으로 제공한다.
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

`DamageHitContext` 자체는 체력을 소유하거나 피해량을 최종 계산하지 않는다. 다만 Context가 보유한 `DamageData`의 `BaseDamage`와 `bCanDamageSelf`는 `HitDamage`가 현재 최소 피해 처리에 사용한다.

---

## 11. 현재 문서 기준의 핵심 결론

현재 `DamageHitContext`는 **체력을 직접 소유하는 시스템이 아니라, HitScan과 Projectile의 피격 결과를 동일 형식으로 보존하고 HitDamage에 전달하는 공용 입력 컨텍스트**다.

가장 중요한 현재 역할은 다음 한 줄로 요약할 수 있다.

> `DamageHitContext`는 현재 “무엇이 어디에 맞았는지를 기록하고, 같은 입력을 HitDamage와 VehicleDebug가 함께 사용하게 만드는 런타임 계약”이다.

---

## 12. 현재 확인 및 미완료 항목

확인 완료:

```text
- WeaponHit / Projectile / VehicleVisualHit 충돌 설정 구현
- VehicleMesh 무기 Query 제외와 SM_Body 시각 차체 피격
- 일반 속도 Dummy HitScan과 Projectile의 SM_Body 충돌
- FCFDamageHitContext.HitComponentName 저장
- VehicleDebug Summary의 HitComponent 표시
- 고속 Projectile P0 집중 스트레스 HitContext 기록
- Reticle 목표점, 터렛·Muzzle 정렬과 실제 탄착 P0 PIE
- 정책 true/false 발사 방향·거부와 MuzzleBlocked
- Unreal Editor 타깃 빌드 성공
```

추가 확인 완료:

```text
- DA_HeavyShell.DefaultDamageData → DA_DamageAsset 연결
- DA_DamageAsset BaseDamage=25, DamageType=Kinetic, bCanDamageSelf=false
- BaseDamage 체력 누적 감소와 체력 0 이하 파괴 상태 전환
- 파괴 이후 TargetDestroyed 거부와 파괴 이벤트 1회성
```

후속 검증:

```text
- 전체 FPS·속도 매트릭스, 이동 차량과 주행 중 조준 확장 회귀
- 장갑·모듈·범위 피해와 완성형 파괴 연출
```

---

## 13. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- FCFDamageHitContext 필드 변경
- UCFDamageData의 현재 런타임 사용 범위 변경
- Dummy HitScan HitContext 기록 방식 변경
- Projectile Actor HitContext 기록 방식 변경
- VehicleDebug Damage 표시 방식 변경
- HitDamage 공용 피해 적용 계약 또는 DamageHitContext 전달 시점이 변경되는 경우
```

---

## 14. Migration

### v1.4.0 -> v1.5.0

```text
- DamageHitContext를 Debug 후보 기록에서 HitDamage 공용 피해 적용 입력 계약으로 현재화했다.
- BaseDamage와 bCanDamageSelf가 최소 피해 계산에 사용되는 현재 동작을 반영했다.
- 체력과 파괴 상태 소유권은 UCFVehicleHealthComp 및 Systems/Combat/HitDamage.md에 유지한다.
- 장갑·모듈·범위 피해는 여전히 후속 범위다.
```

### v1.3.0 -> v1.4.0

```text
- CF-FQ-022 조준 정렬 P0 사용자 PIE 통과를 DamageHitContext 입력 계약에 반영한다.
- Reticle 목표점과 실제 Muzzle 발사 방향 불일치를 Damage Runtime 선행 미완료 목록에서 제거한다.
- CF-FQ-022와 CF-FQ-023이 모두 완료돼 최소 Damage Runtime 착수가 가능하다.
- DefaultDamageData, BaseDamage, 체력 감소와 파괴 상태는 CF-FQ-018 구현 범위로 유지한다.
- 확장 주행·전투 회귀는 CF-FQ-019에서 수행한다.
```

### v1.2.0 -> v1.3.0

```text
- 고속 Projectile의 Sweep/Sub-step/보조 Sphere Sweep과 중복 처리 방지를 현재 구현으로 반영한다.
- 30 FPS + 기준 속도 4배 집중 스트레스 PIE에서 신뢰 가능한 DamageHitContext가 유지된 결과를 기록한다.
- 고속 연속 충돌은 Damage Runtime 선행 미완료 목록에서 제거한다.
- 실제 Damage Runtime의 남은 직접 선행 조건은 CF-FQ-022 사용자 PIE 완료다.
- 전체 매트릭스와 이동 차량 검증은 CF-FQ-019 확장 회귀로 이관한다.
```

### v1.1.0 -> v1.2.0

```text
- WeaponHit / Projectile / VehicleVisualHit 기반 시각 차체 피격을 현재 구현으로 사용한다.
- FCFDamageHitContext.HitComponentName과 VehicleDebug HitComponent 표시를 현재 기준에 포함한다.
- 일반 속도 SM_Body Hit은 확인됐지만 고속 Projectile HitContext는 아직 신뢰 완료 상태로 보지 않는다.
- 조준 정렬과 고속 연속 충돌 완료 후 실제 Damage Runtime을 연결한다.
- 관련 Plan은 AimFireAlignment, ProjectileContinuousCollision, HitDamage 순서로 확인한다.
```

### v1.0.0 -> v1.1.0

```text
- 코드와 자산은 변경하지 않는다.
- 현재 VehicleMesh Physics Asset 기반 피격 상태를 현행 구현으로 기록한다.
- 시각 차체 기반 피격 전환은 Document/Plan/HitDamage/ImplementationDesign.md의 후속 코드 작업에서 수행한다.
- HitComponent 기록 필드가 추가되면 FCFDamageHitContext 사용처와 VehicleDebug 표시를 함께 갱신한다.
```

---

## 15. Changelog

### v1.5.0 - 2026-07-15

```text
- CF-FQ-018 Done과 CF-TC-016 사용자 PIE PASS를 반영했다.
- DamageHitContext가 HitDamage 공용 피해 적용 입력으로 사용되는 현재 흐름을 기록했다.
- DefaultDamageData, BaseDamage, 자기 피해 정책과 파괴 후 추가 피해 거부 확인 결과를 반영했다.
- 체력 소유와 파괴 판정은 Systems/Combat/HitDamage.md 책임으로 분리했다.
```

### v1.4.0 - 2026-07-14

```text
- CF-FQ-022 P0 사용자 PIE 통과를 Damage Runtime 입력 계약에 반영했다.
- Reticle·Muzzle 방향 불일치를 현재 남은 한계에서 제거했다.
- CF-FQ-018 Damage Runtime 착수에 필요한 조준 정렬과 고속 충돌 선행 조건 완료를 기록했다.
- 남은 미구현 범위를 DefaultDamageData, BaseDamage, 체력 감소와 파괴 상태로 정리했다.
```

### v1.3.0 - 2026-07-14

```text
- P0 고속 Projectile 연속 충돌 구현과 집중 스트레스 PIE 결과를 반영했다.
- 30 FPS + 기준 속도 4배에서 차량/얇은 벽 첫 HitContext 기록과 중복 방지, Pool 재사용 정상 동작을 기록했다.
- 고속 Projectile 누락을 현재 남은 한계에서 제거했다.
- Damage Runtime의 남은 직접 선행 조건을 CF-FQ-022 사용자 PIE로 정리했다.
```

### v1.2.0 - 2026-07-13

```text
- WeaponHit / Projectile / VehicleVisualHit 기반 시각 차체 피격 구현 결과 반영
- FCFDamageHitContext.HitComponentName과 Summary HitComponent 표시 반영
- 일반 속도 HitScan / Projectile의 SM_Body 충돌 사용자 PIE 확인 결과 반영
- 고속 Projectile 터널링과 Reticle·Muzzle 방향 불일치를 Damage Runtime 선행 한계로 기록
- AimFireAlignment / ProjectileContinuousCollision Plan 연결
```

### v1.1.0 - 2026-07-13

```text
- 현재 VehicleMesh Physics Asset이 HitScan / Projectile 피격을 받는 구조 확인 결과 추가
- SM_Body가 NoCollision이라 현재 피격에 참여하지 않는 상태 추가
- FCFDamageHitContext가 HitActor만 저장하고 HitComponent는 저장하지 않는 현재 한계 명시
- CF-FQ-018에서 시각 차체 기반 피격 분리 후 피해 처리로 진행하는 순서와 Plan 문서 연결
```

### v1.0.0 - 2026-07-09

```text
- 현재 구현된 DamageHitContext Systems 문서 최초 작성
- UCFDamageData, ECFDamageType, FCFDamageHitContext, Dummy HitScan 기록, Projectile Actor 충돌 기록, VehicleDebug 요약 범위를 현재 코드 기준으로 정리
- 실제 HP 차감, 파괴, 장갑, 모듈, 폭발, 물리 충격 적용은 현재 비책임으로 분리
```

---

## 16. 마지막 확인 기준

- 확인 일시: 2026-07-15
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFDamageData.h`
  - `UE/Source/CarFight_Re/Public/CFDamageTypes.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h`
    - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
  - `/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn` 자산 상세 덤프
  - `Document/Plan/HitDamage/ImplementationDesign.md`

