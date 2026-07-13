# Projectile

- Version: 1.0.0
- Date: 2026-07-09
- Status: Current
- Scope: 현재 구현된 ProjectileData, 공통 Projectile Actor, Projectile Pool, 충돌/수명/비활성화 Debug 기록

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `Projectile` 구현이 실제로 어떤 일을 하는지 기록한다.

이 문서는 미래 설계서가 아니다.
아래 항목은 이 문서의 범위에서 제외한다.

```text
- 실제 HP 차감
- 폭발 범위 피해 적용
- 충돌 시 VFX / SFX / Decal 출력
- 탄종별 고급 탄도 모델
- 네트워크 복제 Projectile
- 서버 권한 Projectile 판정
```

현재 문서 기준 `Projectile`은 **ProjectileData를 읽어 공통 Projectile Actor를 활성화하고, 반복 발사 부담을 줄이기 위해 Projectile Actor Pool로 재사용하며, 충돌/수명/비활성화 결과를 Debug 정보로 남기는 기능**이다.

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

`ImpactEffectId`는 현재 데이터 필드로 존재하지만, 실제 VFX/SFX 출력은 현재 구현 범위로 보지 않는다.
`DefaultDamageData`와 `DamageProfileId`는 현재 Damage HitContext Debug 연결에 사용된다.

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

## 6. 현재 충돌 기록

현재 `ACFProjectileActor`는 충돌 시 실제 Damage 적용을 하지 않는다.
대신 Pool 반환 뒤 Damage HitContext Debug를 만들 수 있도록 마지막 충돌 정보를 보존한다.

현재 보존하는 값은 아래다.

```text
LastDeactivatedProjectileData
LastDeactivatedInstigatorActor
LastHitActor
LastImpactLocation
LastImpactNormal
LastIncomingDirection
LastDeactivatedProjectileId
LastDeactivateReason
LastHitActorName
LastFlightDurationSeconds
```

이 정보는 `ACFVehiclePawn::RecordProjectileDamageHitContextFromPool`에서 `FCFDamageHitContext`를 만드는 데 사용된다.

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
- 마지막 충돌/비활성화 정보를 Debug로 보존한다.
- Hit으로 Pool 반환된 Projectile 정보를 차량 Pawn의 Damage HitContext Debug 기록으로 전달한다.
```

---

## 11. 현재 기준 비책임 항목

현재 `Projectile` 기능은 아래를 직접 수행하지 않는다.

```text
- 실제 HP 차감
- 실제 폭발 피해 계산
- 장갑 관통 계산
- 모듈 손상 계산
- 충돌 이펙트 / 사운드 출력
- 탄약 소모
- 네트워크 복제
- 서버 권한 충돌 판정
```

현재 충돌 정보는 Debug 기록과 HitContext 생성 후보로만 사용된다.

---

## 12. 현재 문서 기준의 핵심 결론

현재 `Projectile`은 **ProjectileData 기반 발사체 Actor 실행과 Pool 재사용, 충돌/수명 종료 Debug 기록을 담당하는 로컬 발사체 런타임 기능**이다.

가장 중요한 현재 역할은 다음 한 줄로 요약할 수 있다.

> `Projectile`은 현재 “발사체를 실제 Actor로 날리고, 충돌이나 수명 종료를 기록한 뒤, Pool로 재사용하는 기능”이다.

---

## 13. 현재 미확인 항목

아래 항목은 코드상 경로는 존재하지만, 이 문서 작성 시점에 에디터 자산 연결 상태를 직접 확인하지 않았다.

```text
- 실제 ProjectileData 에셋의 ProjectileActorClass 연결 상태
- ProjectileStaticMesh 연결 상태
- Projectile Actor Blueprint의 부모 클래스 설정 상태
- PIE에서 Pool 재사용이 실제로 발생하는지 여부
- Projectile Actor 충돌 채널과 맵 내 목표물 충돌 설정
```

이 항목은 추측으로 PASS 처리하지 않는다.

---

## 14. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- UCFProjectileData 필드 의미 변경
- ACFProjectileActor 활성화 / 비활성화 생명주기 변경
- Projectile Pool 확보 / 반환 정책 변경
- Projectile Actor 충돌 정보 보존 방식 변경
- Vehicle Pawn과 Projectile Pool 연결 방식 변경
- 실제 Damage 적용이 Projectile Actor 안으로 들어오는 구조 변경
```

---

## 15. Changelog

### v1.0.0 - 2026-07-09

```text
- 현재 구현된 Projectile Systems 문서 최초 작성
- ProjectileData, CFProjectileActor, ProjectilePoolComp, Pool Bucket, 충돌/수명/비활성화 Debug 기록을 현재 코드 기준으로 정리
- 실제 HP 차감, 폭발 피해, VFX/SFX, 서버 판정은 현재 비책임으로 분리
```

---

## 16. 마지막 확인 기준

- 확인 일시: 2026-07-09
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
