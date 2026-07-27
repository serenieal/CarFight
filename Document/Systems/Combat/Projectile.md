# Projectile

- Version: 1.3.1
- Date: 2026-07-24
- Status: Current / P0 Collision and First-Impact Damage Verified
- Scope: 현재 ProjectileData/Actor/Pool, 시각 차체 충돌, 첫 Impact 피해 적용과 P0 고속 연속 충돌 구현

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `Projectile` 구현이 실제로 어떤 일을 하는지 기록한다.

이 문서는 미래 설계서가 아니다.
아래 항목은 이 문서의 범위에서 제외한다.

```text
- 차량 체력과 파괴 상태 자체의 소유
- 폭발 범위 피해 적용
- 충돌 시 VFX / Decal 출력
- 탄종별 고급 탄도 모델
- 네트워크 복제 Projectile
- 서버 권한 Projectile 판정
```

현재 문서 기준 `Projectile`은 **ProjectileData를 읽어 공통 Projectile Actor를 활성화하고, 첫 유효 Impact에서 HitDamage를 한 번 호출하며, 충돌/수명 결과를 보존한 뒤 Pool로 재사용하는 기능**이다.

---

## 2. 현재 구현 범위

| 구분 | 현재 구현 |
| --- | --- |
| 발사체 데이터 | `UCFProjectileData` |
| 공통 발사체 Actor | `ACFProjectileActor` |
| 발사체 Pool 컴포넌트 | `UCFProjectilePoolComp` |
| Pool 버킷 구조 | `FCFProjectilePoolBucket` |
| 비활성화 사유 enum | `ECFProjectileDeactivateReason` |
| 차량 발사 연결 | `ACFVehiclePawn::TrySpawnProjectileActorFromFireCommand` |
| Pool 확보 함수 | `UCFProjectilePoolComp::AcquireProjectile` |
| Pool 반환 함수 | `UCFProjectilePoolComp::ReleaseProjectile` |
| Projectile 활성화 함수 | `ACFProjectileActor::ActivateProjectile` |
| Projectile 비활성화 함수 | `ACFProjectileActor::DeactivateProjectile` |

---

## 3. 현재 ProjectileData가 제공하는 값

현재 `UCFProjectileData`는 무기에서 분리된 발사체 데이터다.

현재 주요 필드는 아래와 같다.

```text
ProjectileId
InitialSpeed
bAffectedByGravity
GravityScale
LifeTimeSeconds
CollisionRadius
ProjectileActorClass
ProjectileStaticMesh
ProjectileMeshRelativeRotation
ProjectileMeshRelativeScale
ImpactEffectId
DefaultDamageData
DamageProfileId
```

현재 실제 Projectile Actor 실행 경로에서 핵심적으로 사용되는 값은 아래다.

```text
- ProjectileActorClass
- InitialSpeed
- bAffectedByGravity
- GravityScale
- LifeTimeSeconds
- CollisionRadius
- ProjectileStaticMesh
- ProjectileMeshRelativeRotation
- ProjectileMeshRelativeScale
```

`ImpactEffectId`는 현재 데이터 필드로 존재하지만, 실제 VFX / Decal 출력은 현재 구현 범위로 보지 않는다.
`DefaultDamageData`는 첫 유효 Impact의 `DamageHitContext`와 HitDamage 입력에 사용된다. `DamageProfileId`는 DamageData 미연결 시 Debug fallback 식별자로만 사용한다.

---

## 4. 현재 Projectile Actor가 실제로 하는 일

`ACFProjectileActor`는 `ProjectileData`를 적용받아 발사체를 활성화하는 공통 Actor다.

현재 컴포넌트 구성은 아래다.

```text
CollisionComponent: USphereComponent
MeshComponent: UStaticMeshComponent
ProjectileMovementComponent: UProjectileMovementComponent
```

현재 `ActivateProjectile`은 아래 역할을 한다.

```text
- ProjectileData 저장
- 발사 주체 Actor 저장
- ProjectileData의 메시 설정 적용
- ProjectileData의 충돌 반경 적용
- ProjectileData의 이동 속도 / 중력 설정 적용
- 발사 방향으로 ProjectileMovementComponent 활성화
- LifeTimeSeconds 기준 수명 타이머 예약
- Projectile 활성 상태로 전환
```

현재 `DeactivateProjectile`은 발사체 이동과 충돌을 멈추고 비활성화한다.
Pool 소유자가 없으면 기본적으로 Destroy 경로를 사용할 수 있고, Pool 소유자가 있으면 Pool 반환 경로를 우선한다.

---

## 5. 현재 비활성화 사유

현재 Projectile Actor는 아래 비활성화 사유를 기록한다.

```text
None
InvalidActivation
Manual
Hit
LifeExpired
```

각 의미는 현재 기준으로 아래와 같다.

| 사유 | 현재 의미 |
| --- | --- |
| `None` | 아직 비활성화 사유가 없음 |
| `InvalidActivation` | ProjectileData 또는 활성화 조건이 유효하지 않음 |
| `Manual` | 코드에서 수동으로 비활성화함 |
| `Hit` | 충돌 컴포넌트 Blocking Hit으로 비활성화됨 |
| `LifeExpired` | LifeTimeSeconds 타이머가 끝나 비활성화됨 |

---

## 6. 현재 충돌·피해 기록

현재 `ACFProjectileActor`는 첫 유효 Impact에서 `FCFDamageHitContext`를 만들고 HitDamage 공용 진입점을 한 번 호출한다.
그 뒤 Pool 반환과 Debug 표시를 위해 마지막 충돌 정보와 `FCFDamageApplyResult`를 함께 보존한다.

현재 보존하는 값은 아래다.

```text
LastDeactivatedProjectileData
LastDeactivatedInstigatorActor
LastHitActor
LastHitComponentName
LastImpactLocation
LastImpactNormal
LastIncomingDirection
LastDeactivatedProjectileId
LastDeactivateReason
LastHitActorName
LastFlightDurationSeconds
```

이 정보는 `ACFVehiclePawn::RecordProjectileDamageHitContextFromPool`에서 `FCFDamageHitContext`를 만드는 데 사용된다.

현재 충돌 채널과 차량 응답:

```text
Projectile Collision Object Type = CFCollisionChannels::Projectile
VehicleMesh                       = Projectile Ignore
SM_Body                           = QueryOnly / Projectile Block
DamageHitContext                  = HitComponentName 기록
```

2026-07-13 PIE에서 일반 속도 Projectile이 `SM_Body` 시각 차체에 정상 충돌하는 것을 확인했다.

### 6.1 고속 Projectile 연속 충돌

2026-07-14 `CF-FQ-023`에서 다음 고속 충돌 정책을 구현했다.

```text
- ProjectileMovement bSweepCollision 명시 적용
- bForceSubStepping과 MaxSimulationTimeStep / MaxSimulationIterations 데이터 적용
- PreviousCollisionLocation → CurrentCollisionLocation 보조 Sphere Sweep
- ResolveProjectileImpact() 단일 처리 경로
- bImpactResolvedThisActivation 활성화별 중복 Impact 방지
- Pool 재활성화 시 PreviousLocation과 Impact 상태 초기화
```

사용자 PIE 확인 결과:

```text
일반 속도 SM_Body 충돌 = PASS
피격 Actor / 피격 컴포넌트 기록 = PASS
30 FPS + 기준 InitialSpeed 4배 차량 집중 발사 = PASS
얇은 벽 첫 Blocking Hit과 관통 방지 = PASS
중복 Impact와 Pool 재사용 = PASS
```

전체 60 / 120 FPS 조합과 이동 차량 교차 충돌은 `CF-FQ-019` 확장 회귀 테스트에서 추가 확인한다.

상세 설계와 검증 근거:

```text
Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md
```

---

## 7. 현재 Projectile Pool이 실제로 하는 일

`UCFProjectilePoolComp`는 Projectile Actor Class별로 재사용 가능한 발사체를 보관한다.

현재 Pool 구조는 아래다.

```text
UCFProjectilePoolComp
→ TArray<FCFProjectilePoolBucket>
  → ProjectileActorClass
  → SpawnedProjectileArray
  → InactiveProjectileArray
```

현재 `AcquireProjectile`은 아래 흐름을 가진다.

```text
1. ProjectileData와 ProjectileActorClass 유효성 확인
2. ProjectileActorClass에 해당하는 Pool Bucket 찾기 또는 생성
3. 비활성 Projectile Actor가 있으면 재사용
4. 없으면 MaxPooledProjectileCountPerClass 한도 안에서 새 Actor 생성
5. 확보한 Actor를 지정 Transform으로 배치
6. ActivateProjectile 호출
7. 확보 실패 시 nullptr 반환
```

현재 `ReleaseProjectile`은 아래 역할을 한다.

```text
- 반환된 Projectile Actor의 마지막 비활성화 정보 읽기
- 마지막 반환 요약 Debug 값 갱신
- Hit 사유로 반환된 경우 Owner Pawn에 Damage HitContext Debug 기록 전달
- Actor를 비활성 배열에 넣어 재사용 가능 상태로 보관
```

---

## 8. 현재 차량 발사 흐름과의 연결

현재 차량 Pawn은 Projectile Actor 경로를 사용할 수 있으면 `TrySpawnProjectileActorFromFireCommand`를 호출한다.

이 함수는 아래 조건을 검사한다.

```text
- VehicleWeaponComp 존재
- ProjectilePoolComp 존재
- ActiveProjectileData 존재
- ActiveProjectileData.ProjectileActorClass 존재
- FireCommand.AimDirection 유효
```

조건을 만족하면 아래 호출로 Projectile Actor를 확보한다.

```text
ProjectilePoolComp->AcquireProjectile(
    ActiveProjectileData,
    ProjectileSpawnTransform,
    LaunchDirection,
    this
)
```

Projectile Actor 확보에 실패하면 현재 차량 발사 경로는 Dummy HitScan fallback을 사용할 수 있다.

---

## 9. 현재 Debug / 확인 가능 항목

현재 Projectile / Pool 관련 Debug는 아래 값을 통해 확인할 수 있다.

```text
bActiveProjectileSpawnReady
ActiveProjectileExecutionSummary
bHasProjectilePoolComponent
TotalPooledProjectileCount
ActivePooledProjectileCount
InactivePooledProjectileCount
LastProjectileReleaseSummary
```

Projectile Actor 자체는 아래 값을 제공한다.

```text
IsProjectileActive
GetActiveProjectileData
GetLastDeactivateReason
GetLastDeactivatedProjectileId
GetLastFlightDurationSeconds
GetLastHitActorName
GetLastDeactivatedProjectileData
GetLastHitActor
GetLastHitComponentName
GetLastImpactLocation
GetLastImpactNormal
GetLastIncomingDirection
GetLastInstigatorActor
```

---

## 10. 현재 기능 책임

현재 `Projectile` 기능의 책임은 아래로 제한한다.

```text
- ProjectileData를 읽어 공통 Projectile Actor에 적용한다.
- 발사체 메시, 충돌 반경, 이동 속도, 중력, 수명을 설정한다.
- 발사체를 활성화하고 이동시킨다.
- 충돌 또는 수명 종료로 비활성화한다.
- Projectile Actor를 Pool로 재사용한다.
- 첫 유효 Impact에서 HitDamage 공용 진입점을 정확히 한 번 호출한다.
- 마지막 충돌/비활성화 정보와 피해 적용 결과를 Debug로 보존한다.
- Hit으로 Pool 반환된 Projectile 정보를 차량 Pawn의 DamageHitContext와 DamageApplyResult 기록으로 전달한다.
```

---

## 11. 현재 기준 비책임 항목

현재 `Projectile` 기능은 아래를 직접 수행하지 않는다.

```text
- 차량 MaxHealth / CurrentHealth 소유
- 파괴 상태의 최종 판정과 이벤트 소유
- 실제 폭발 피해 계산
- 장갑 관통 계산
- 모듈 손상 계산
- 충돌 VFX / Decal 출력
- 탄약 소모
- 네트워크 복제
- 서버 권한 충돌 판정
```

Projectile은 피해 적용을 요청하고 결과를 보존하지만 체력과 파괴 상태 자체는 `UCFVehicleHealthComp`와 HitDamage가 소유한다.

---

## 12. 현재 문서 기준의 핵심 결론

현재 `Projectile`은 **ProjectileData 기반 발사체 Actor 실행, 첫 Impact 피해 요청, 충돌 결과 보존과 Pool 재사용을 담당하는 로컬 발사체 런타임 기능**이다.

가장 중요한 현재 역할은 다음 한 줄로 요약할 수 있다.

> `Projectile`은 현재 “발사체를 실제 Actor로 날리고 첫 유효 충돌의 피해를 한 번 적용한 뒤 결과를 보존해 Pool로 재사용하는 기능”이다.

---

## 13. 현재 미확인 / 미완료 항목

확인 완료:

```text
- Projectile Object Channel과 VehicleVisualHit 응답 구현
- 일반 속도 Projectile의 SM_Body 시각 차체 충돌
- HitComponentName의 DamageHitContext 기록
- bSweepCollision과 UpdatedComponent 적용
- Sub-stepping / MaxSimulationTimeStep / MaxSimulationIterations 적용
- Previous → Current 보조 Sphere Sweep
- 활성화별 중복 Impact 방지와 Pool 재사용 상태 초기화
- 30 FPS + 기준 InitialSpeed 4배 차량 집중 발사
- 얇은 벽 첫 Blocking Hit과 관통 방지
- Unreal Editor 타깃 빌드
- 첫 유효 Impact의 BaseDamage 적용과 파괴 상태 전환 사용자 PIE
- Pool 반환 시 피해 중복 적용 없음
```

확장 회귀 항목:

```text
- 60 / 120 FPS와 기준 속도 1배 / 2배 전체 조합
- 이동 차량 SM_Body 교차 충돌
- 동시 Projectile 다수의 성능 비용
```

확장 항목은 `CF-FQ-019`에서 검증하며 현재 P0 Projectile 연속 충돌 기능은 완료 상태다.

---

## 14. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- UCFProjectileData 필드 의미 변경
- ACFProjectileActor 활성화 / 비활성화 생명주기 변경
- Projectile Pool 확보 / 반환 정책 변경
- Projectile Actor 충돌 정보 보존 방식 변경
- Vehicle Pawn과 Projectile Pool 연결 방식 변경
- Projectile Actor의 첫 Impact 피해 적용 또는 DamageApplyResult 보존 방식 변경
```

---

## 15. Migration

### v1.3.0 -> v1.3.1

```text
- Projectile 런타임과 데이터 구조는 변경하지 않는다.
- ImpactEffectId의 후속 표현 의미를 VFX와 Decal로 제한한다.
- 게임 사운드 구현은 CF-PDL-0009에 따라 Projectile 책임과 후속 범위에서 제외한다.
```

### v1.2.0 -> v1.3.0

```text
- Projectile Actor가 첫 유효 Impact에서 HitDamage 공용 진입점을 한 번 호출하는 현재 동작을 반영한다.
- Pool 반환 경로는 피해를 재적용하지 않고 저장된 DamageApplyResult만 전달한다.
- 차량 체력과 파괴 상태 소유권은 UCFVehicleHealthComp와 Systems/Combat/HitDamage.md에 유지한다.
- 폭발·장갑·모듈 피해는 후속 범위다.
```

### v1.1.0 -> v1.2.0

```text
- UCFProjectileData의 Sweep/Sub-step/보조 Sweep/CCD 설정을 현재 구현으로 반영한다.
- ACFProjectileActor의 Previous → Current Sphere Sweep과 단일 ResolveProjectileImpact 경로를 현재 기준으로 사용한다.
- 30 FPS + 기준 속도 4배 집중 스트레스 PIE 통과를 P0 완료 근거로 사용한다.
- 전체 FPS·속도 조합과 이동 차량 검증은 CF-FQ-019 확장 회귀로 이관한다.
- 실제 HP 차감과 Damage Runtime은 여전히 Projectile의 책임이 아니다.
```

### v1.0.0 -> v1.1.0

```text
- Projectile Object Type은 CFCollisionChannels::Projectile을 사용한다.
- 차량 시각 차체 SM_Body가 Projectile을 Block하고 VehicleMesh는 Ignore한다.
- LastHitComponentName을 보존하고 DamageHitContext.HitComponentName으로 전달한다.
- 현재 일반 속도 충돌은 확인됐지만 고속 연속 충돌은 보장되지 않는다.
- 고속 충돌 구현 기준은 Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md를 우선한다.
- 이 문서 갱신에서는 Projectile 코드와 DataAsset을 변경하지 않는다.
```

---

## 16. Changelog

### v1.3.1 - 2026-07-24

```text
- Projectile 비책임 항목과 ImpactEffectId 설명에서 SFX와 사운드 표현을 제거했다.
- 후속 충돌 표현을 VFX / Decal 전용으로 정리했다.
```

### v1.3.0 - 2026-07-15

```text
- CF-FQ-018 최소 Damage Runtime과 사용자 PIE PASS를 Projectile 현재 동작에 반영했다.
- 첫 유효 Impact 피해 적용, DamageApplyResult 보존과 Pool 반환 중복 차감 방지를 기록했다.
- Projectile과 HitDamage의 책임 경계를 현재 구현 기준으로 정리했다.
```

### v1.2.0 - 2026-07-14

```text
- Sweep/Sub-stepping/보조 Sphere Sweep과 중복 Impact 방지의 현재 구현을 반영했다.
- 일반 속도 Visual HitContext와 30 FPS + 기준 속도 4배 집중 스트레스 PIE 통과를 기록했다.
- 얇은 벽 첫 Blocking Hit, 관통 방지와 Pool 재사용 정상 동작을 기록했다.
- CF-FQ-023 P0 완료와 CF-FQ-019 확장 회귀 이관을 반영했다.
```

### v1.1.0 - 2026-07-13

```text
- Projectile 전용 Object Channel과 SM_Body 시각 차체 충돌 현재 구현 반영
- LastHitComponentName / DamageHitContext HitComponentName 기록 반영
- 일반 속도 Projectile 시각 차체 충돌 사용자 PIE 확인 결과 반영
- 고속 Projectile 터널링을 현재 미완료 한계로 기록
- Sweep/Sub-stepping/보조 Sphere Sweep 설계 문서 연결
```

### v1.0.0 - 2026-07-09

```text
- 현재 구현된 Projectile Systems 문서 최초 작성
- ProjectileData, CFProjectileActor, ProjectilePoolComp, Pool Bucket, 충돌/수명/비활성화 Debug 기록을 현재 코드 기준으로 정리
- 실제 HP 차감, 폭발 피해, VFX/SFX, 서버 판정은 현재 비책임으로 분리
```

---

## 17. 마지막 확인 기준

- 확인 일시: 2026-07-15
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
