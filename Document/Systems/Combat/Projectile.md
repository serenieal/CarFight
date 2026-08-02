# Projectile

- Version: 1.8.1
- Date: 2026-08-02
- Status: Current System / Same-Source Salvo Isolation User PIE PASS / Different-Source Interception Deferred
- Features: `CF-FQ-023 고속 Projectile 연속 충돌`, `CF-FQ-027 투사체 비행 FX`, `CF-FQ-028 발사체 추진 시스템`
- Tests: `CF-TC-020 PASS`, `CF-TC-023 PASS`, `CF-TC-024 PASS`
- Scope: ProjectileData 기반 발사체 Actor, 비유도 Rocket 추진, 지속형 Trail·Thruster FX, 연속 충돌, 첫 Impact 피해와 Pool 생명주기의 현재 구현 기준

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `Projectile` 런타임이 실제로 어떤 데이터와 컴포넌트를 사용하고, 발사부터 추진·비행 FX·충돌·피해·비활성화·Pool 재사용까지 어떤 순서로 작동하는지 기록한다.

이 문서는 미래 설계서가 아니다.
현재 코드, 공식 Editor 빌드와 사용자 PIE에서 확인된 동작만 현재 기준으로 기록한다.

현재 Projectile 시스템의 핵심 목적은 다음과 같다.

> `ProjectileData`를 읽어 공통 Projectile Actor를 활성화하고, 필요하면 비유도 Rocket 추진과 지속형 비행 FX를 적용하며, 첫 유효 Impact에서 피해를 한 번 처리한 뒤 모든 런타임 상태를 초기화해 Pool로 안전하게 재사용한다.

현재 문서의 범위에서 제외하는 항목:

```text
- Target Homing Missile
- Laser Guided Missile
- 수동 유도와 목표 상실 정책
- 다단 추진과 추진 출력 Curve
- 공기저항·항력·추력 편향
- 폭발 범위 피해
- 장갑 관통과 모듈 손상
- 근접신관
- 네트워크 복제와 서버 권한 Projectile
- 게임 사운드와 AudioComponent
```

---

## 2. 현재 완료 상태

```text
고속 연속 충돌
- Feature: CF-FQ-023 Done
- Test: CF-TC-020 PASS

비유도 Rocket 추진
- Feature: CF-FQ-028 Done
- Test: CF-TC-024 PASS
- 사용자 PIE: PASS
- Current System: Document/Systems/Combat/Projectile.md

지속형 Projectile Flight FX
- Feature: CF-FQ-027 Done
- Test: CF-TC-023 PASS
- C++ 런타임, Trail·Thruster 자산 연결과 사용자 PIE 전체 행렬을 이 Current System에 반영
- Trail-only, Thruster-only, 두 FX 동시, 소켓·Fallback, 종료 Reset, Pool 재사용, Ribbon History와 30 FPS 고속 Bounds 검증 PASS
```

`CF-FQ-028` 완료 Plan:

```text
Document/Plan/ProjectilePropulsionPlan.md
```

`CF-FQ-027` 보존 Plan:

```text
Document/Plan/ProjectileFlightFxPlan.md
Document/Plan/ProjectileFlightFxRoadmap.md
```

---

## 3. 현재 구현 구성

### 3.1 주요 C++ 타입

| 타입 | 현재 역할 |
| --- | --- |
| `UCFProjectileData` | 발사체의 Actor, 이동, 충돌, 추진, 요격, 메시, Trail·Thruster, Impact와 DamageData 설정을 제공한다. |
| `ACFProjectileActor` | ProjectileData를 적용하고 이동·추진·지속형 FX·차량별 충돌 격리·요격·피해·비활성화 생명주기를 소유한다. |
| `UCFProjectileMotorComp` | 비유도 Rocket의 점화 지연, Burning 가속, 속도 상한과 BurnedOut 상태를 관리한다. |
| `UCFProjectilePoolComp` | Projectile Actor Class별 Actor를 재사용하고 모든 버킷을 가로질러 동일 발사 차량 Projectile의 양방향 Ignore 관계를 관리한다. |
| `FCFProjectilePropulsionConfig` | 자체 추진 사용 여부, 점화 지연, 연소 시간, 추진 가속도와 최대 추진 속도를 제공한다. |
| `FCFProjectileMotorSnapshot` | 현재 모터 상태, 경과 시간, 속도, 추진 방향과 FX 활성 요청을 Debug와 Blueprint에 제공한다. |
| `FCFProjectileAttachedFxSettings` | Trail 또는 Thruster의 사용 여부, Niagara, 부착 방식, 소켓과 Transform을 제공한다. |
| `ECFProjectileFxAttachMode` | `ProjectileRelative` 또는 `MeshSocketWithFallback` 부착 방식을 구분한다. |
| `ECFProjectileMotorState` | `Inactive`, `Disabled`, `IgnitionDelay`, `Burning`, `BurnedOut` 상태를 구분한다. |
| `ECFProjectileDeactivateReason` | `InvalidActivation`, `Manual`, `Hit`, `Intercepted`, `LifeExpired` 비활성화 사유를 구분한다. |

### 3.2 주요 소스 파일

```text
UE/Source/CarFight_Re/Public/CFProjectileMotorTypes.h
UE/Source/CarFight_Re/Public/CFProjectileMotorComp.h
UE/Source/CarFight_Re/Private/CFProjectileMotorComp.cpp
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileData.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h
UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp
UE/Source/CarFight_Re/Private/CFProjectileMotorTests.cpp
UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp
UE/Source/CarFight_Re/Private/CFProjectileCollisionTests.cpp
```

---

## 4. 현재 ProjectileData 계약

`UCFProjectileData`는 WeaponData에서 분리된 발사체별 설정 DataAsset이다.
실제 Actor를 만들지 않는 HitScan·Laser도 DamageData를 공유하기 위한 가상 ProjectileData를 사용할 수 있지만, 이 문서의 이동·추진·Pool 설명은 `ProjectileActorClass`가 지정된 실제 Projectile 경로를 대상으로 한다.

### 4.1 Identity와 Actor

```text
ProjectileId
ProjectileActorClass
```

`ProjectileActorClass`가 없거나 Weapon의 `FireMode`가 Projectile이 아니면 실제 Projectile Actor 경로를 사용하지 않으며 현재 WeaponFire의 Dummy HitScan fallback 계약을 유지한다.

### 4.2 기본 이동

```text
InitialSpeed
bAffectedByGravity
GravityScale
LifeTimeSeconds
```

`InitialSpeed`의 현재 의미:

```text
일반 포탄
→ 활성화 직후 사용할 고정 초기 속도

추진 Rocket
→ 발사대에서 분리되는 순간의 초기 속도
→ 이후 ProjectileMotorComp가 추가 추진 가속 적용
```

### 4.3 연속 충돌

```text
CollisionRadius
bUseSweepCollision
bForceSubStepping
MaxSimulationTimeStep
MaxSimulationIterations
bUseSupplementalContinuousSweep
bUseCCD
```

현재 안전 기본값:

```text
Sweep = true
Force Sub-step = true
MaxSimulationTimeStep = 0.008333
MaxSimulationIterations = 8
Supplemental Continuous Sweep = true
CCD = false
```

CCD는 주 해결책이 아니라 탄종별 선택 보조 장치다.

### 4.3A Projectile 요격

```text
bCanBeIntercepted = true
bDetonateWhenIntercepted = true
```

`bCanBeIntercepted`는 다른 발사 차량의 Projectile 또는 Hitscan 유효 적중 한 번으로 이 Projectile을 `Intercepted` 종료할 수 있는지 결정한다. 기본값은 `true`이므로 기존 ProjectileData를 재저장하지 않아도 P0 요격 대상이 된다.

`bDetonateWhenIntercepted`는 요격 종료 위치에서 이 ProjectileData의 `DefaultImpactFxData`를 폭발 표현으로 요청할지 결정한다. 이 값이 false여도 `Intercepted` 종료와 Pool 반환은 유지하고 FX 요청만 생략한다.

같은 차량의 Projectile은 이 두 값과 관계없이 먼저 충돌 격리되므로 서로 요격하지 않는다.

### 4.4 추진 모터

```text
PropulsionConfig.bUsePropulsion
PropulsionConfig.IgnitionDelaySeconds
PropulsionConfig.BurnDurationSeconds
PropulsionConfig.ThrustAccelerationCmPerSecSq
PropulsionConfig.MaximumPropelledSpeed
```

기본값은 `bUsePropulsion=false`다.
따라서 기존 ProjectileData를 재저장하지 않아도 기존 포탄은 InitialSpeed 기반 비추진 비행을 유지한다.

### 4.5 시각 메시

```text
ProjectileStaticMesh
ProjectileMeshRelativeRotation
ProjectileMeshRelativeScale
```

메시 표시 Scale과 충돌 반경, 지속형 FX Scale은 서로 독립된 데이터다.

### 4.6 지속형 비행 FX

```text
TrailFxSettings
ThrusterFxSettings
```

각 슬롯은 다음 값을 제공한다.

```text
bEnabled
NiagaraSystem
AttachMode
AttachSocketName
RelativeTransform
```

기본 소켓:

```text
Trail = FX_Trail
Thruster = FX_Exhaust
```

두 슬롯 기본값은 비활성이다.
FX가 비활성이거나 Niagara가 비어 있어도 Projectile 이동·추진·충돌·피해는 유지된다.

### 4.7 Impact와 Damage

```text
DefaultImpactFxData
ImpactEffectId
DefaultDamageData
DamageProfileId
```

`DefaultImpactFxData`는 첫 유효 Impact 후 일회성 Impact Niagara를 선택한다.
실제 재생 책임은 `Document/Systems/Combat/CombatFx.md`가 소유한다.

`DefaultDamageData`는 첫 유효 Impact의 공용 Damage 입력이다.
`DamageProfileId`는 DamageData 미연결 상태에서 사용하는 Debug fallback 식별자다.

---

## 5. 현재 Projectile Actor 컴포넌트 구조

```text
ACFProjectileActor
└─ CollisionComponent: USphereComponent
   ├─ MeshComponent: UStaticMeshComponent
   ├─ TrailOriginComponent: USceneComponent
   │  └─ TrailNiagaraComponent: UNiagaraComponent
   └─ ThrusterOriginComponent: USceneComponent
      └─ ThrusterNiagaraComponent: UNiagaraComponent

ProjectileMovementComponent: UProjectileMovementComponent
ProjectileMotorComponent: UCFProjectileMotorComp
```

### 5.1 CollisionComponent

```text
- Actor Root
- Object Type은 Projectile Channel 사용
- Projectile 채널 기본 응답은 Block
- WorldStatic, WorldDynamic과 차량 SM_Body Blocking 응답 유지
- 같은 ActiveInstigatorActor의 Projectile 쌍만 IgnoreActorWhenMoving 양방향 등록
- CollisionRadius 적용
- Blocking Hit 이벤트 제공
- 필요 시 CCD 보조 적용
```

같은 차량에서 새 Projectile이 활성화되면 `UCFProjectilePoolComp`가 모든 Actor Class 버킷의 활성 Projectile을 순회한다. 기존 Projectile의 `ActiveInstigatorActor`가 새 Projectile의 발사 차량과 같으면 두 CollisionComponent에 서로를 양방향 Ignore로 등록한다.

따라서 같은 차량의 캐논탄·로켓·미사일은 탄종, Actor Class, Volley와 FireRequest가 달라도 서로 충돌하지 않는다. 다른 차량 Projectile에는 이 예외를 등록하지 않으므로 Projectile 채널의 기본 Block을 유지한다.

보조 연속 Sphere Sweep도 동일 차량 Ignore 목록을 `FCollisionQueryParams`에 추가한다. Pool 반환 전에는 양쪽 Actor의 Ignore 관계를 모두 해제해 재사용 Actor에 이전 발사 차량 관계가 남지 않게 한다.

### 5.2 MeshComponent

```text
- ProjectileStaticMesh 표시
- ProjectileMeshRelativeRotation 적용
- ProjectileMeshRelativeScale 적용
- 자체 충돌은 사용하지 않음
```

### 5.3 ProjectileMovementComponent

```text
- 초기 속도와 Velocity 적용
- 중력 적용
- Sweep과 Sub-step 처리
- 추진 발사체의 MaximumPropelledSpeed를 MaxSpeed로 적용
```

### 5.4 ProjectileMotorComponent

```text
- ProjectileMovement보다 먼저 Tick
- 발사 시 PropulsionConfig와 LaunchDirection 수신
- IgnitionDelay와 Burning 시간 진행
- Velocity에 추진 가속 적용
- BurnedOut 전환
- Pool 반환 전 Reset
```

### 5.5 Trail·Thruster 컴포넌트

```text
- Actor 생성 시 한 번 생성
- AutoActivate = false
- AutoDestroy = false
- 발사마다 생성·파괴하지 않음
- Projectile Actor와 함께 Pool 재사용
- 모든 비활성화 경로에서 Asset과 실행 상태 Reset
```

---

## 6. Projectile 활성화 순서

현재 `ACFProjectileActor::ActivateProjectile`의 핵심 순서는 아래다.

```text
1. ProjectileData, CollisionComponent와 Movement 유효성 검사
2. 이전 Instigator Ignore 해제
3. ActiveProjectileData와 이번 Instigator 저장
4. 이전 충돌·피해·비활성화 Debug 상태 초기화
5. 안전한 LaunchDirection 계산
6. Actor 회전을 LaunchDirection에 정렬
7. Projectile 메시 적용
8. 이전 Trail·Thruster 상태 Reset
9. 이번 ProjectileData의 Trail·Thruster 부착과 자산 준비
10. 충돌 반경·채널·CCD 적용
11. InitialSpeed·중력·Sweep·Sub-step과 MaxSpeed 적용
12. ProjectileMotorComp에 추진 설정과 고정 LaunchDirection 전달
13. 현재 Motor 상태에 맞춰 Thruster FX 동기화
14. LifeTimeSeconds 타이머 예약
15. Actor 표시·충돌·Tick 활성
```

FX 또는 추진 자산 누락은 전체 Projectile 활성화 실패로 전환하지 않는다.

---

## 7. 비유도 Rocket 추진 상태

### 7.1 상태 흐름

```text
Inactive
→ Disabled

또는

Inactive
→ IgnitionDelay
→ Burning
→ BurnedOut
→ Hit 또는 LifeExpired에서 Projectile 전체 비활성화
```

| 상태 | 현재 의미 |
| --- | --- |
| `Inactive` | 발사 전 또는 Pool 반환 후 모터 Reset 상태 |
| `Disabled` | `bUsePropulsion=false`인 기존 비추진 Projectile 상태 |
| `IgnitionDelay` | InitialSpeed로 분리됐지만 추진 가속이 아직 시작되지 않은 상태 |
| `Burning` | 발사 시 저장한 고정 방향으로 실제 추진 가속을 적용하는 상태 |
| `BurnedOut` | 연소 종료 후 추가 가속 없이 기존 Velocity와 중력으로 관성 비행하는 상태 |

핵심 계약:

```text
BurnedOut != Projectile Deactivate
```

연소가 끝나도 Projectile Actor는 충돌하거나 수명이 종료될 때까지 계속 비행한다.

### 7.2 고정 추진 방향

P0 비유도 Rocket의 추진 방향은 발사 시 전달된 `LaunchDirection`을 정규화해 고정한다.

```text
FixedThrustDirection = SafeNormal(LaunchDirection)
```

P0에서는 다음 요소가 추진 방향을 변경하지 않는다.

```text
- TargetSelect
- Lock-on 대상
- Actor의 후속 회전
- 현재 Velocity 방향 변화
- Reticle 이동
- 유도 목표점
```

유도는 후속 `UCFProjectileGuidanceComp` 후보가 별도로 소유한다.

### 7.3 Burning 가속

Burning 동안 개념적으로 다음 가속을 적용한다.

```text
Velocity += FixedThrustDirection
          * ThrustAccelerationCmPerSecSq
          * AppliedBurnDurationSeconds
```

한 프레임이 IgnitionDelay 종료와 Burning 시작을 함께 포함하면, 점화 지연에 사용하고 남은 프레임 시간만 추진 가속에 사용한다.

### 7.4 최대 추진 속도

추진 적용 후 Velocity 크기를 `MaximumPropelledSpeed`로 제한한다.
단, 잘못된 설정 때문에 `MaximumPropelledSpeed < InitialSpeed`가 되어도 발사 직후 기존 속도를 강제로 낮추지 않는다.

현재 안전 상한은 다음 값을 사용한다.

```text
max(MaximumPropelledSpeed, CurrentSpeedBeforeThrust)
```

### 7.5 중력

중력은 추진 상태와 별도로 `ProjectileMovementComponent`가 처리한다.

```text
IgnitionDelay 중 중력 적용 가능
Burning 중 중력 적용 가능
BurnedOut 관성 비행 중 중력 적용 가능
```

---

## 8. 지속형 Trail과 Thruster FX

### 8.1 책임 경계

```text
ACFProjectileActor
- 비행 중 지속형 Trail
- Motor Burning 상태의 Thruster
- 소켓/Fallback 부착
- Pool 반환 전 Reset

UCFCombatFxComp
- 승인 Fire의 일회성 Muzzle
- 첫 Impact의 일회성 Impact
- 최초 차량 Destroyed의 일회성 FX
```

지속형 Projectile FX를 `CombatFxComp`에 중복 구현하지 않는다.

### 8.2 부착 방식

#### ProjectileRelative

```text
Origin Parent = CollisionComponent
Location / Rotation = RelativeTransform
Scale = RelativeTransform.Scale
Debug Source = ProjectileRelative
```

#### MeshSocketWithFallback — 유효 소켓

```text
Origin Parent = MeshComponent의 AttachSocketName
Location / Rotation = 메시 소켓 Transform
Scale = RelativeTransform.Scale
Debug Source = MeshSocket:<SocketName>
```

#### MeshSocketWithFallback — 소켓 누락

```text
Origin Parent = CollisionComponent
Location / Rotation = RelativeTransform
Scale = RelativeTransform.Scale
Debug Source = MissingSocketFallback:<SocketName>
```

### 8.3 독립 FX Scale

`RelativeTransform.Scale`은 Fallback 경로에서만 사용하는 값이 아니다.
현재 계약에서는 **소켓 사용 여부와 관계없이 적용되는 독립 FX Scale**이다.

```text
Location / Rotation
- 유효 소켓이 있으면 소켓 우선
- ProjectileRelative 또는 소켓 누락이면 RelativeTransform 사용

Scale
- 유효 소켓과 Fallback 양쪽 모두 RelativeTransform.Scale 사용
- ProjectileMeshRelativeScale 상속과 분리
```

Origin Component는 메시의 비균일 Scale을 그대로 상속하지 않는다.
이를 통해 발사체 메시 크기 때문에 Ribbon 폭이나 추진 화염이 의도하지 않게 찌그러지는 것을 방지한다.

2026-07-28 사용자 PIE에서 다음을 확인했다.

```text
DA_PFX_ThrusterTest
- FX_Exhaust 소켓 부착: PASS
- RelativeTransform.Scale = 1.0 기준 표시: PASS
- RelativeTransform.Scale = 0.2 적용 시 추진 화염 축소: PASS
```

### 8.4 Trail 상태

Trail 슬롯이 활성이고 Niagara가 유효하면 Projectile 활성화와 함께 재생한다.

```text
Projectile 활성화
→ Trail 시작
→ 비행 중 유지
→ InvalidActivation / Manual / Hit / LifeExpired에서 즉시 Reset
```

### 8.5 Thruster 상태

Thruster Niagara는 Projectile 전체 수명이 아니라 실제 Motor 상태와 동기화한다.

| Motor State | Thruster 상태 |
| --- | --- |
| `Inactive` | `Inactive` |
| `Disabled` | `MotorDisabled` |
| `IgnitionDelay` | `IgnitionPending` |
| `Burning` | `Active` 또는 `ActivationRequested` |
| `BurnedOut` | `BurnedOut` |

의존 방향은 다음으로 고정한다.

```text
Motor State → Thruster FX
```

Niagara 활성 성공 여부가 추진 가속이나 충돌 판정을 바꾸는 역방향 의존은 없다.

### 8.6 누락 안전성

```text
bEnabled=false
→ Disabled

bEnabled=true + NiagaraSystem 없음
→ MissingSystem

Dedicated Server
→ DedicatedServerSkipped
```

모든 경우 Projectile 이동·추진·충돌·피해는 계속 유지된다.

---

## 9. 연속 충돌과 첫 Impact

### 9.1 Projectile 충돌 채널

```text
Projectile Collision Object Type = CFCollisionChannels::Projectile
Projectile ↔ Projectile 기본값 = Block
같은 발사 차량 Projectile 쌍 = Actor별 양방향 Ignore
다른 발사 차량 Projectile 쌍 = Block·Interception 후보
VehicleMesh = Projectile Ignore
SM_Body = QueryOnly / Projectile Block
벽·지형·일반 월드 Blocking Object = Block 유지
DamageHitContext = HitComponentName 기록
```

동일 발사자 기준은 `ActiveInstigatorActor` Actor 동일성이다. FireRequestId, WeaponGroupId와 팀은 충돌 예외 기준으로 사용하지 않는다.

### 9.2 Sweep과 Sub-step

```text
- ProjectileMovement bSweepCollision 적용
- bForceSubStepping 적용
- MaxSimulationTimeStep 적용
- MaxSimulationIterations 적용
```

### 9.3 보조 연속 Sphere Sweep

`ACFProjectileActor`는 이전 Collision 위치부터 현재 위치까지 `CollisionRadius` 기반 Sphere Sweep을 수행할 수 있다.

```text
PreviousCollisionLocation
→ CurrentCollisionLocation
→ 첫 Blocking Hit 검사
```

### 9.4 동일 발사 차량 필터·요격과 중복 Impact 방지

`OnComponentHit`과 보조 Sweep은 다음 단일 진입점을 공유한다.

```text
ResolveProjectileImpact()
```

기본 무효 대상:

```text
- 자기 자신
- ActiveInstigatorActor
- Owner Actor
- ActiveInstigatorActor가 같은 Projectile
```

서로 다른 발사 차량의 Projectile이 충돌하면 양쪽 ProjectileData의 `bCanBeIntercepted`를 각각 평가한다. 요격 가능한 Projectile은 `Intercepted` 사유로 종료하고, 요격 불가 Projectile은 충돌 전 Velocity를 복구해 계속 비행한다. 양쪽 모두 요격 불가인 특수 조합은 반복 Blocking Hit 방지를 위해 해당 Actor 쌍만 런타임 Ignore한다.

Hitscan은 발사 차량 Pool의 활성 Projectile을 Trace Query에서 제외하므로 자기 Projectile을 맞히지 않는다. 다른 차량 Projectile을 맞히면 같은 `TryResolveProjectileInterception()` 진입점을 사용한다.

한 활성화에서 첫 유효 World·차량 Impact 또는 Interception이 처리되면 다음 값이 중복 처리를 차단한다.

```text
bImpactResolvedThisActivation = true
```

Pool 재활성화마다 이 값, PreviousCollisionLocation과 동일 차량 Ignore 관계를 초기화한다.

### 9.5 현재 검증 결과

```text
일반 속도 SM_Body 충돌: PASS
피격 Actor / 피격 컴포넌트 기록: PASS
30 FPS + 기준 InitialSpeed 4배 차량 집중 발사: PASS
얇은 벽 첫 Blocking Hit과 관통 방지: PASS
중복 Impact 없음: PASS
Pool 재사용: PASS
동일 차량 Salvo 사출 직후 상호 Impact 없음: PASS
Salvo Projectile의 차량·월드 대상 일반 충돌 유지: PASS
다른 차량 Projectile·Hitscan 요격: DEFERRED / 현재 적 사격 주체 없음 / AI 전투 단계에서 검증
다른 Volley·다른 탄종 교차 격리와 Pool Ignore Reset: DEFERRED / 복수 무기 환경에서 검증
CF-TC-020: PASS
```

60·120 FPS 전체 속도 조합과 이동 차량 교차 충돌은 `CF-FQ-019` 확장 반복 회귀 범위다.

---

## 10. 첫 Impact 피해 적용

첫 유효 Impact에서 현재 Actor는 다음 정보를 보존한다.

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
LastDamageHitContext
LastDamageApplyResult
```

첫 Impact 흐름:

```text
첫 Blocking Hit
→ ResolveProjectileImpact
→ FCFDamageHitContext 생성
→ DefaultDamageData 해석
→ UCFVehicleHealthComp::TryApplyDamageToActor
→ FCFDamageApplyResult 보존
→ Hit 사유 비활성화
→ 지속형 FX와 Motor Reset
→ Pool 반환
```

Pool 반환 경로는 피해를 다시 계산하거나 적용하지 않는다.
Projectile Actor에 저장된 DamageHitContext와 DamageApplyResult를 Debug에 전달할 뿐이다.

차량 체력과 파괴 상태 자체는 `Document/Systems/Combat/HitDamage.md`가 소유한다.

---

## 11. 비활성화와 Pool Reset

현재 비활성화 사유:

```text
InvalidActivation
Manual
Hit
Intercepted
LifeExpired
```

모든 사유는 공용 비활성화 경로를 사용한다.

```text
1. 수명 타이머 해제
2. 마지막 Projectile·Impact·비행 시간 Debug 보존
3. Actor Tick 정지
4. ProjectileMotorComp Reset
5. ProjectileMovement 정지
6. Collision 비활성화
7. Trail Niagara DeactivateImmediate / ResetSystem / Asset 해제
8. Thruster Niagara DeactivateImmediate / ResetSystem / Asset 해제
9. Origin을 CollisionComponent로 재부착
10. Origin 위치·회전 초기화
11. Origin Scale을 1,1,1로 초기화
12. Actor 숨김과 ActiveProjectileData 해제
13. ProjectilePoolComp 반환 또는 Destroy
```

중요 순서:

```text
Motor와 Flight FX Reset
→ FinishDeactivatePolicy
```

Pool에 Actor가 반환된 뒤 이전 연소 시간, Niagara Asset, 소켓 부착과 FX Scale이 남지 않도록 한다.

---

## 12. Projectile Pool

`UCFProjectilePoolComp`는 Projectile Actor Class별 Bucket을 관리한다.

```text
UCFProjectilePoolComp
→ TArray<FCFProjectilePoolBucket>
  → ProjectileActorClass
  → SpawnedProjectileArray
  → InactiveProjectileArray
```

### 12.1 AcquireProjectile

```text
1. ProjectileData와 ProjectileActorClass 검사
2. Class Bucket 검색 또는 생성
3. 비활성 Actor가 있으면 재사용
4. 없으면 클래스별 한도 안에서 새 Actor 생성
5. Actor Transform 설정
6. Pool Owner 지정
7. ActivateProjectile 호출
8. 확보 실패 시 nullptr 반환
```

### 12.2 ReleaseProjectile

```text
- 마지막 비활성화 사유와 결과 읽기
- Pool Debug 요약 갱신
- Hit인 경우 Owner Pawn에 마지막 Damage Debug 전달
- Actor를 Inactive 배열에 반환
```

### 12.3 확보 실패

Projectile Pool 확보 또는 활성화가 실패하면 현재 WeaponFire 경로는 Dummy HitScan fallback을 사용할 수 있다.

---

## 13. Debug 계약

### 13.1 ProjectileData 요약

`BuildProjectileSummary()`는 다음을 포함한다.

```text
ProjectileId
InitialSpeed
Propulsion 활성 여부와 수치
Gravity
LifeTime
Collision과 연속 충돌 설정
Actor와 Mesh 상태
Mesh Scale
Trail 상태·부착 방식·소켓·Scale
Thruster 상태·부착 방식·소켓·Scale
Impact와 DamageData 상태
```

### 13.2 Motor 요약

`BuildProjectileMotorSummary()` / `BuildMotorSummary()`는 다음을 포함한다.

```text
CurrentMotorState
경과 비행 시간
점화 지연 진행
연소 진행과 남은 시간
현재 속도
추진 가속도와 최대 속도
고정 추진 방향
실제 추진 여부
Thruster FX 활성 요청
MotorActivationCount
```

### 13.3 Flight FX 요약

`BuildProjectileFlightFxSummary()`는 다음을 포함한다.

```text
TrailStatus
TrailAttachment
TrailScale
ThrusterStatus
ThrusterAttachment
ThrusterScale
```

예시:

```text
ProjectileFlightFx:
TrailStatus=Disabled,
TrailAttachment=None,
TrailScale=(1.000, 1.000, 1.000),
ThrusterStatus=Active,
ThrusterAttachment=MeshSocket:FX_Exhaust,
ThrusterScale=(0.200, 0.200, 0.200)
```

### 13.4 Pool과 Vehicle Debug

```text
bActiveProjectileSpawnReady
ActiveProjectileExecutionSummary
bHasProjectilePoolComponent
TotalPooledProjectileCount
ActivePooledProjectileCount
InactivePooledProjectileCount
LastProjectileReleaseSummary
```

---

## 14. C++와 Unreal Editor 책임 분리

### 14.1 C++ 책임

```text
- 데이터 안전 기본값
- Projectile 활성화·비활성화와 Pool 생명주기
- 모터 상태와 추진 계산
- Trail·Thruster 컴포넌트 소유와 Reset
- 소켓 존재 여부와 Fallback 해석
- 독립 FX Scale 적용
- Sweep·Sub-step·보조 Sweep과 첫 Impact 단일 처리
- DamageHitContext와 DamageApplyResult 보존
- Debug와 Automation 소스
```

### 14.2 DataAsset·Blueprint·Editor 책임

```text
- 탄종별 ProjectileData 생성과 수치 저장
- ProjectileStaticMesh 선택
- PropulsionConfig 튜닝
- Trail·Thruster Niagara 선택
- StaticMesh의 FX_Trail·FX_Exhaust 소켓 배치
- Fallback 위치·회전과 독립 FX Scale 튜닝
- 최종 PIE 시각·운동 확인
```

Blueprint Tick에 별도 추진 가속이나 Niagara 생명주기를 중복 구현하지 않는다.

---

## 15. 현재 검증 근거

### 15.1 공식 Editor 빌드

추진 Foundation:

```text
Build Job: 2ffd09357e654bb7970a2e379f5ab45f
Result: PASS
Exit Code: 0
```

FX Scale 보정 최종 빌드:

```text
Build Job: e8b812bd479549299dd116f9bae8996f
Target: CarFight_ReEditor
Platform: Win64
Configuration: Development
UHT: PASS
Compile: PASS
Link: PASS
Exit Code: 0
```

### 15.2 Automation

```text
CarFight.ProjectilePropulsion.PP_P0_01.RuntimeContract
CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract
CarFight.Projectile.LM_P0_06.SourceIsolation
```

초기 체크포인트:

```text
Automation 소스 작성: 완료
공식 Editor 빌드에서 컴파일: PASS
Validation: Not Run
Historical Scope: 2026-07-28~2026-07-30 당시 작업 범위에서 승인된 실행 경로 미확보
```

후속 검증:

```text
Validation: PASS
Evidence: UE/Saved/Automation/CombatRuntime/index.json, result.json
Result: 전체 42 Success / 0 Failed / PP_P0_01·PFX_P0_01·LM_P0_06 Success
Execution Method: 작업 전용 임시 실행 경로
Reusable Entry Point: Not Defined
Historical Scope: Current evidence generated 2026-08-02 05:43 KST
```

현재 Automation 상태는 최신 증거 기준 PASS다.
다만 당시 실행 수단은 공용 저장소 도구나 영구 재실행 계약으로 승격하지 않으며, 향후 재실행 시 현재 도구·작업 범위를 다시 확인한다.
`CF-TC-023`과 `CF-TC-024`의 완료 판정은 Automation과 별도로 공식 빌드와 사용자 PIE 결과도 함께 유지한다.

차량별 Projectile 격리·요격 공식 빌드:

```text
첫 Build Job: 363ccb2d040a45dbb4e980c49631310b / FVector 형식 오류 / Exit Code 6
최종 Build Job: 940272869b77450797347f76b427faf3
Target: CarFight_ReEditor
Platform: Win64
Configuration: Development
UHT: PASS
CFProjectileActor.cpp: PASS
CFProjectilePoolComp.cpp: PASS
CFProjectileData.cpp: PASS
CFVehiclePawn.cpp: PASS
CFProjectileCollisionTests.cpp: PASS
Link: PASS
Exit Code: 0
```

`SourceIsolation`은 동일 Source의 다른 FireRequest Projectile 양방향 Ignore, 다른 Source 기본 Block·Interception, Intercepted 사유, 비요격 설정과 Pool 반환·재활성화 관계를 검증하는 소스다. 최신 Automation 보고서에서 `CarFight.Projectile.LM_P0_06.SourceIsolation`은 Success다. 다만 다른 차량 Projectile·Hitscan을 실제 전투 환경에서 확인하는 사용자 PIE는 별도 Deferred 상태로 유지한다.

### 15.3 사용자 PIE — CF-TC-024

```text
DA_PFX_ThrusterTest 실제 발사: PASS
비유도 Rocket 추진 동작: PASS
IgnitionDelay → Burning → BurnedOut 표현: PASS
Thruster FX의 Burning 상태 동기화: PASS
FX_Exhaust 소켓 부착: PASS
RelativeTransform.Scale 1.0 적용: PASS
RelativeTransform.Scale 0.2 축소: PASS
사용자 관찰 기준 나머지 동작 정상: PASS
```

최종 판정:

```text
CF-FQ-028 = Done
CF-TC-024 = PASS
Current System = Document/Systems/Combat/Projectile.md
```

### 15.4 사용자 PIE — CF-TC-023

```text
Trail-only Projectile: PASS
Thruster-only Projectile: PASS
Trail + Thruster 동시 재생: PASS
FX_Trail·FX_Exhaust 유효 소켓 부착: PASS
Missing Socket RelativeTransform Fallback: PASS
Hit 종료 FX Reset: PASS
LifeExpired 종료 FX Reset: PASS
Trail-only·Thruster-only·Both·FX 없음 교차 Pool 재사용 20발 이상: PASS
이전 Ribbon History·연기·화염 잔류 없음: PASS
30 FPS 고속 비행 Bounds·컬링·Ribbon 연속성: PASS
첫 Impact·Damage 단일 처리와 Pool 반환 회귀: PASS
게임 오디오 참조 0개 유지: PASS
```

최종 판정:

```text
CF-FQ-027 = Done
CF-TC-023 = PASS
Automation = PASS / 2026-08-02 최신 CombatRuntime 증거
Reusable Entry Point = Not Defined
Current System = Document/Systems/Combat/Projectile.md
```

---

## 16. 현재 기능 책임

현재 Projectile 시스템의 책임:

```text
- ProjectileData를 공통 Projectile Actor에 적용한다.
- 비추진 포탄과 비유도 추진 Rocket을 같은 Actor 생명주기에서 처리한다.
- 점화 지연, Burning 가속, 속도 상한과 BurnedOut 관성 비행을 처리한다.
- Trail과 추진 화염을 데이터 기반으로 배치·재생한다.
- Thruster를 실제 Burning 상태와 동기화한다.
- 소켓 누락 시 Projectile Relative Fallback을 제공한다.
- 메시 Scale과 분리된 독립 FX Scale을 적용한다.
- Sweep·Sub-step과 보조 Sweep으로 고속 충돌을 처리한다.
- 첫 유효 Impact에서 피해를 한 번 적용하고 결과를 보존한다.
- 모든 상태를 Reset한 뒤 Actor를 Pool로 반환한다.
```

---

## 17. 현재 비책임 항목

```text
- 차량 체력과 파괴 상태의 최종 소유
- 일회성 Muzzle·Impact·Destroyed Niagara 생성
- TargetSelect와 Lock-on
- Missile Guidance
- 폭발 범위 피해
- 장갑·모듈 피해
- 탄약 소비와 재장전
- 네트워크 복제와 서버 권한 판정
- 게임 사운드
```

연관 책임:

```text
HitDamage
→ 체력 감소와 파괴 상태

CombatFx
→ 승인 Fire, 첫 Impact와 최초 Destroyed의 일회성 FX

TargetSelect
→ 공용 선택 대상

후속 Guidance
→ 유도 목표에 따른 추진 방향 변경
```

---

## 18. 확장 회귀와 후속 범위

`CF-FQ-028` P0는 완료됐지만, 다음 항목은 반복 전투 확장 회귀로 분리한다.

```text
- 주행 중 Rocket 반복 발사
- 60 / 120 FPS와 전체 속도 조합
- 이동 차량 SM_Body 교차 충돌
- 다수 Rocket 동시 비행 성능
- 장시간 Pool 재사용의 Motor·Niagara 상태 오염
- 기존 포탄과 Rocket을 번갈아 발사하는 데이터 전환
- Muzzle·Impact·Destroyed와 지속형 FX의 반복 중복·잔류
```

소유 기능:

```text
CF-FQ-019 주행/전투 반복 테스트
```

이는 `CF-FQ-028` Systems 승격을 취소하거나 Active로 되돌리는 조건이 아니다.
회귀에서 실제 결함이 발견되면 해당 책임 문서와 코드의 별도 수정 작업으로 연다.

`CF-FQ-027`의 Trail-only, Thruster-only, 두 FX 동시, Missing Socket, Hit·LifeExpired Reset, 20발 이상 Pool 재사용, Ribbon History와 30 FPS 고속 Bounds 체크리스트는 사용자 PIE에서 PASS했다.

---

## 19. 회귀 보호 기준

Projectile 관련 변경 시 최소 확인:

```text
- bUsePropulsion=false 기존 포탄이 InitialSpeed 기반으로 비행한다.
- 추진 Rocket이 InitialSpeed로 분리된다.
- IgnitionDelay 중 추진 화염과 추가 가속이 없다.
- Burning에서 고정 LaunchDirection 가속과 Thruster FX가 활성화된다.
- MaximumPropelledSpeed를 초과하지 않는다.
- BurnedOut에서 추가 가속과 Thruster가 종료되고 Projectile 비행은 유지된다.
- Trail·Thruster 누락이 이동·충돌·피해를 취소하지 않는다.
- 유효 소켓과 Missing Socket Fallback이 크래시 없이 처리된다.
- RelativeTransform.Scale이 소켓·Fallback 양쪽에서 적용된다.
- Hit과 LifeExpired에서 Motor와 Niagara가 Reset된다.
- 첫 Impact 피해가 한 번만 적용된다.
- Pool 반환이 피해를 다시 적용하지 않는다.
- 기존 Sweep·Sub-step·보조 Sweep 계약이 유지된다.
- 동시에 발사된 Projectile들이 서로 Blocking Hit·Impact·Damage·Pool 반환을 만들지 않는다.
- Projectile 상호 Ignore 상태에서도 차량 SM_Body, 벽과 일반 월드 Blocking Hit은 유지된다.
- Pool 재활성화 뒤에도 Projectile 채널 Ignore와 월드 Block 응답표가 유지된다.
- 게임 오디오 참조가 추가되지 않는다.
```

연관 테스트:

```text
CF-TC-015 시각 메시 기반 피격
CF-TC-016 HitDamage
CF-TC-020 고속 Projectile 연속 충돌
CF-TC-021 CombatFx
CF-TC-023 Projectile Flight FX
CF-TC-024 Projectile Propulsion
```

---

## 20. 연관 Systems 문서

```text
Document/Systems/Combat/WeaponFire.md
- Projectile Actor 사용 여부와 Pool Acquire 호출을 결정한다.

Document/Systems/Combat/DamageHitContext.md
- 첫 Impact의 위치, 노멀, 입사 방향과 피격 컴포넌트 형식을 정의한다.

Document/Systems/Combat/HitDamage.md
- DamageData 적용, 체력 감소와 최초 Destroyed 상태를 소유한다.

Document/Systems/Combat/CombatFx.md
- 승인 Fire, 첫 Impact와 최초 Destroyed의 일회성 Niagara를 소유한다.

Document/Systems/UI/VehicleDebugPanel.md
- ProjectileData, Pool, 충돌과 후속 Debug를 화면에 표시한다.
```

---

## 21. 문서 갱신 조건

다음 변경이 생기면 이 문서를 함께 갱신한다.

```text
- UCFProjectileData 필드 또는 의미 변경
- PropulsionConfig와 Motor State 변경
- 추진 계산 또는 최대 속도 정책 변경
- Trail·Thruster 부착·Scale·생명주기 변경
- ACFProjectileActor 활성화·비활성화 순서 변경
- Projectile Pool 확보·반환 정책 변경
- Sweep·Sub-step·보조 Sweep 정책 변경
- 첫 Impact 피해 적용과 결과 보존 방식 변경
- Missile Guidance가 Current System으로 승격
- CF-TC-024 회귀 기준 변경
```

---

## 22. 문서 버전 관리

- 현재 문서 버전: `1.8.1`
- 문서 상태: `Current System / Same-Source Salvo Isolation User PIE PASS / Different-Source Interception Deferred`

### Changelog

#### v1.8.1 - 2026-08-02

```text
- 발사체 추진, 비행 FX와 SourceIsolation Automation의 초기 Not Run을 2026-07-28~30 당시 체크포인트 상태로 분리했다.
- UE/Saved/Automation/CombatRuntime/index.json과 result.json의 최신 결과를 Current 증거로 반영했다.
- PP_P0_01, PFX_P0_01과 LM_P0_06이 Success이며 전체 42 Success / 0 Failed임을 기록했다.
- 검증 결과, 증거, 실행 수단과 공용 재실행 진입점을 분리했다.
- 당시 작업 전용 임시 실행 경로를 공용 도구나 영구 재실행 계약으로 승격하지 않았다.
- 다른 차량 Projectile·Hitscan의 실제 전투 환경 사용자 PIE는 Automation PASS와 별개로 Deferred 상태를 유지했다.
```

#### v1.8.0 - 2026-07-30

```text
- 사용자 PIE에서 Salvo 다중 발사가 정상적으로 유지되고 동일 차량 Projectile끼리 사출 직후 상호 Impact되는 결함이 재현되지 않음을 확인했다.
- Salvo Projectile이 차량·월드 대상에는 기존대로 정상 충돌함을 확인했다.
- 다른 차량 Projectile·Hitscan 요격은 현재 플레이어 외 사격 주체가 없어 AI 전투 단계까지 Deferred했다.
- 다른 Volley·다른 탄종 교차 격리와 Pool Ignore Reset도 복수 무기·사격 주체 환경의 후속 회귀로 분리했다.
- 구현·공식 빌드 상태는 유지하며 미실행 요격 항목을 PASS로 기록하지 않는다.
```

#### v1.7.0 - 2026-07-30

```text
- 같은 차량이 발사한 모든 탄종과 모든 Volley를 ActiveInstigatorActor 기준 단일 충돌 격리 범위로 확정했다.
- Projectile 채널 기본 Block을 복구하고 Pool의 모든 Actor Class 버킷을 가로질러 동일 Source Projectile만 양방향 Ignore한다.
- 다른 차량 Projectile과 Hitscan의 유효 적중으로 bCanBeIntercepted Projectile을 Intercepted 처리하고 선택적 폭발 FX를 연결했다.
- Pool 반환 시 양방향 Ignore 관계를 해제하고 Hitscan Query에서 자기 차량 활성 Projectile을 제외한다.
- ACFProjectileActor v1.11.0, UCFProjectilePoolComp v1.5.0, UCFProjectileData v1.9.0, ACFVehiclePawn v2.128.0과 CFProjectileCollisionTests v2.0.0을 기록했다.
- 최종 Editor Build Job 940272869b77450797347f76b427faf3 / Exit Code 0을 확인했다.
- Automation Source Compile PASS / Execution Not Run이며 사용자 PIE 전에는 CF-TC-026 PASS를 주장하지 않는다.
```

#### v1.6.0 - 2026-07-30

```text
- Salvo 사용자 PIE에서 런처의 네 Muzzle 동시 사출은 정상이나 Projectile끼리 즉시 Blocking Hit을 발생시키는 결함을 확인했다.
- 원인을 Projectile Object Type이 자기 Projectile 채널까지 Block하던 Collision Response로 확정했다.
- ACFProjectileActor v1.10.0에서 Projectile 채널 Ignore, 충돌 활성화 전 응답표 적용과 Impact Guard를 추가했다.
- 차량 SM_Body, 벽과 일반 월드 Blocking Hit은 기존대로 유지한다.
- CFProjectileCollisionTests.cpp v1.0.0과 Build Job c96631773b2543b0b480e82da5d63e7c / Exit Code 0을 기록했다.
- Automation Source Compile PASS / Execution Not Run이며 Salvo 사용자 PIE 재검증을 남겼다.
```

#### v1.5.0 - 2026-07-30

```text
- CF-FQ-027 투사체 비행 FX의 사용자 PIE 전체 행렬 완료를 Current System에 반영했다.
- Trail-only, Thruster-only, Trail+Thruster, 유효 소켓과 Missing Socket Fallback을 PASS 처리했다.
- Hit·LifeExpired Reset, 20발 이상 Pool 교차 재사용, Ribbon History 무잔류와 30 FPS 고속 Bounds를 PASS 처리했다.
- 첫 Impact·Damage 단일 처리, Pool 반환과 게임 오디오 참조 0개 회귀를 유지했다.
- CF-FQ-027 Done과 CF-TC-023 PASS를 등록했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run이며 현재 Admin Runner 미노출 상태를 유지한다.
```

#### v1.4.0 - 2026-07-28

```text
- CF-FQ-028 비유도 Rocket 추진 시스템을 기존 Projectile Current System에 통합했다.
- CFProjectileMotorTypes, UCFProjectileMotorComp, PropulsionConfig와 Motor 상태 계약을 기록했다.
- InitialSpeed 분리, IgnitionDelay, Burning 고정 방향 가속, MaximumPropelledSpeed와 BurnedOut 관성 비행을 기록했다.
- Trail·Thruster Origin/Niagara 컴포넌트, 소켓/Fallback과 Pool 반환 전 Reset을 현재 구현에 추가했다.
- Thruster를 Motor Burning 상태에만 동기화하는 책임을 기록했다.
- RelativeTransform.Scale을 유효 소켓과 Fallback에 공통 적용하는 독립 FX Scale 계약을 기록했다.
- Build Job e8b812bd479549299dd116f9bae8996f Exit Code 0과 사용자 Scale 0.2 PIE PASS를 기록했다.
- CF-FQ-028 Done과 CF-TC-024 PASS를 Current System으로 승격했다.
- 반복 전투 확장 회귀를 CF-FQ-019로 이관했다.
- CF-FQ-027의 별도 전체 검증은 Paused 상태이며 CF-TC-023 PASS를 주장하지 않음을 명시했다.
```

#### v1.3.1 - 2026-07-24

```text
- Projectile 비책임 항목과 ImpactEffectId 설명에서 SFX와 사운드 표현을 제거했다.
- 후속 충돌 표현을 VFX / Decal 전용으로 정리했다.
```

#### v1.3.0 - 2026-07-15

```text
- CF-FQ-018 최소 Damage Runtime과 사용자 PIE PASS를 Projectile 현재 동작에 반영했다.
- 첫 유효 Impact 피해 적용, DamageApplyResult 보존과 Pool 반환 중복 차감 방지를 기록했다.
```

#### v1.2.0 - 2026-07-14

```text
- Sweep, Sub-stepping, 보조 Sphere Sweep과 중복 Impact 방지의 현재 구현을 반영했다.
- 30 FPS + 기준 속도 4배 P0 집중 스트레스 PIE 통과를 기록했다.
```

#### v1.1.0 - 2026-07-13

```text
- Projectile 전용 Object Channel과 SM_Body 시각 차체 충돌을 반영했다.
- LastHitComponentName과 DamageHitContext 기록을 반영했다.
```

#### v1.0.0 - 2026-07-09

```text
- ProjectileData, CFProjectileActor, ProjectilePoolComp와 충돌·수명·Pool Debug의 최초 Current System 문서를 작성했다.
```

### Migration

#### v1.3.1 → v1.4.0

```text
- CF-FQ-028의 현재 구현 판단은 이 Projectile.md를 우선한다.
- Document/Plan/ProjectilePropulsionPlan.md는 완료 당시 설계·빌드·PIE 체크포인트 보존용이다.
- 기존 ProjectileData는 bUsePropulsion=false와 비활성 Trail·Thruster 기본값으로 이전 동작을 유지한다.
- 추진 Rocket은 InitialSpeed를 발사대 분리 속도로 해석한다.
- BurnedOut은 Projectile 비활성화가 아니라 추가 추진 종료다.
- 지속형 Trail·Thruster는 ACFProjectileActor가 소유하며 CombatFxComp에 넣지 않는다.
- RelativeTransform.Scale은 소켓·Fallback 공통 독립 FX Scale이다.
- CF-TC-024는 PASS이며 반복 전투 확장 회귀는 CF-FQ-019가 소유한다.
- CF-FQ-027과 CF-TC-023의 별도 전체 체크리스트는 자동 완료로 해석하지 않는다.
```

---

## 23. 마지막 확인 기준

- 확인 일시: `2026-07-28`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFProjectileMotorTypes.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileMotorComp.h`
  - `UE/Source/CarFight_Re/Private/CFProjectileMotorComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Private/CFProjectileData.cpp`
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
  - `UE/Source/CarFight_Re/Private/CFProjectileMotorTests.cpp`
  - `UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp`
  - Build Job `2ffd09357e654bb7970a2e379f5ab45f` / Exit Code 0
  - Build Job `e8b812bd479549299dd116f9bae8996f` / Exit Code 0
  - `DA_PFX_ThrusterTest` 사용자 PIE 추진·FX·Scale 정상 동작 확인
