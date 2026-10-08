# CarFight — 고속 Projectile 연속 충돌 구현 설계

- Version: 0.4.0
- Date: 2026-07-14
- Status: Completed / Official Build Passed / P0 Focused Stress PIE Passed
- Feature: `CF-FQ-023 고속 Projectile 연속 충돌`
- Priority: P0
- Scope: 고속 발사체가 프레임 사이의 얇은 충돌체를 통과하는 터널링을 방지하고, Projectile 충돌을 프레임률과 속도 변화에 대해 안정적으로 만든다.

---

## 현재 작업 체크포인트

- 현재 상태: 연속 충돌 Core와 디버그 표시 보강 구현, 공식 Editor 빌드, 일반 속도 Visual HitContext, P0 집중 스트레스 PIE를 모두 통과했다.
- 빌드 상태: `Tools\BuildEditor.bat` PASS
- PIE 상태: 일반 속도 `피격 Actor = BP_CFVehiclePawn_C_1`, `피격 컴포넌트 = SM_Body` PASS / `30 FPS + 기준 속도 4배` 집중 스트레스 PASS
- 기능 판정: `CF-FQ-023` P0 완료

### 완료한 범위

- `UCFProjectileData`에 Sweep, Sub-stepping, 보조 Sphere Sweep과 CCD 설정 추가
- `ACFProjectileActor` PostPhysics Tick 기반 Previous → Current 연속 Sweep 구현
- `OnComponentHit`과 보조 Sweep을 `ResolveProjectileImpact()`로 통합
- 활성화별 첫 Impact 1회 처리와 Pool 재활성화 상태 초기화 구현
- 기존 Projectile 채널, `SM_Body`, `HitComponentName`, `DamageHitContext` 전달 구조 유지
- 2026-07-14 사용자 실행 공식 Editor 빌드 성공
- 2026-07-14 사용자 PIE에서 일반 속도 Projectile 피격과 피격 Actor `BP_CFVehiclePawn_C_1` 기록 확인
- VehicleDebug Panel의 Damage HitContext에 `피격 컴포넌트` 독립 표시 행 추가
- 2026-07-14 사용자 실행 `Tools\BuildEditor.bat`에서 피격 컴포넌트 표시 보강분 빌드 성공
- 2026-07-14 사용자 PIE에서 `피격 Actor = BP_CFVehiclePawn_C_1`, `피격 컴포넌트 = SM_Body` 기대값 출력 확인
- 2026-07-14 사용자 PIE에서 `30 FPS + 기준 속도 4배` 차량 집중 발사 테스트 통과
- 2026-07-14 사용자 PIE에서 얇은 벽 앞 차량 배치 테스트를 통과해 첫 Blocking Hit과 벽 관통 방지 확인
- 2026-07-14 사용자 PIE에서 중복 피격과 Pool 재사용 이상 없음 확인

### 확장 회귀 테스트로 남긴 범위

- 60 / 120 FPS와 기준 속도 1배 / 2배 전체 조합
- 주행 차량 `SM_Body` 교차 충돌
- 동시 Projectile 다수 성능 비용

위 항목은 P0 기능 완료를 막는 미완료가 아니라, 이후 전투 반복 테스트에서 수행할 확장 회귀 범위다.

### 바로 다음 작업

1. `CF-FQ-022` 조준점·터렛·총구 정렬 사용자 PIE를 완료한다.
2. 정렬 검증 통과 후 `CF-FQ-018` 최소 Damage Runtime 구현을 진행한다.
3. 확장 속도·FPS 매트릭스와 이동 차량 검증은 `CF-FQ-019` 주행/전투 반복 테스트에서 수행한다.

### 관련 코드와 문서 경로

- `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
- `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
- `UE/Source/CarFight_Re/Public/CFProjectileData.h`
- `UE/Source/CarFight_Re/Private/CFProjectileData.cpp`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`
- `Document/Systems/Combat/Projectile.md`
- `Document/Systems/Combat/DamageHitContext.md`

### 보호 범위

- Projectile 충돌 채널과 `VehicleVisualHit` Profile을 다시 설계하지 않는다.
- 실제 HP 감소, 장갑 계산과 파괴 상태는 `CF-FQ-018` 범위로 유지한다.
- P0 집중 스트레스 검증 범위와 확장 회귀 범위를 구분해 기록하며, 수행하지 않은 전체 매트릭스를 통과했다고 기록하지 않는다.

---

## 1. 목적

현재 Projectile Actor는 일반 속도에서 `SM_Body` 시각 차체와 정상 충돌하지만, 속도가 높아지면 프레임과 프레임 사이에 존재하는 충돌체를 통과하는 현상이 확인됐다.

이 현상을 터널링(Tunneling)으로 분류한다.

```text
이전 프레임 위치: 충돌체 앞
다음 프레임 위치: 충돌체 뒤
결과: 충돌 이벤트 없이 통과
```

목표는 다음과 같다.

> Projectile의 이전 위치부터 다음 위치까지 연속 구간을 검사하고, 고속·저프레임 환경에서도 첫 Blocking Hit을 정확히 한 번 기록한다.

이 작업은 실제 피해 적용 전에 완료해야 한다.
피해 시스템은 신뢰 가능한 HitContext를 전제로 해야 하기 때문이다.

### 1.1 2026-07-14 작업 결과

```text
코드 구현 완료:
- UCFProjectileData에 Sweep / Sub-step / 보조 Sphere Sweep / CCD 설정 추가
- 안전 기본값: Sweep=true, Sub-step=true, MaxStep=0.008333, Iterations=8, SupplementalSweep=true, CCD=false
- ACFProjectileActor Tick을 PostPhysics에 배치하고 활성 발사체에서만 보조 Sweep 실행
- PreviousCollisionLocation과 bImpactResolvedThisActivation 추가
- OnComponentHit과 보조 Sweep을 ResolveProjectileImpact로 통합
- Projectile 자신 / Instigator / Owner Ignore 유지
- Pool 재활성화 시 이전 위치와 Impact 처리 상태 초기화
- 기존 LastHitComponentName과 DamageHitContext 전달 경로 유지

검증 완료:
- 2026-07-14 사용자 실행 `Tools\\BuildEditor.bat` 공식 Editor 빌드 성공
- 2026-07-14 일반 속도 사용자 PIE 피격 확인
- 피격 Actor `BP_CFVehiclePawn_C_1` 기록 확인
- 피격 컴포넌트 `SM_Body` 기록 확인
- `30 FPS + 기준 InitialSpeed 4배` 차량 집중 발사 테스트 통과
- 얇은 벽 앞 차량 배치에서 벽 관통 없음과 첫 Blocking Hit 확인
- 중복 Impact, 중복 Pool 반환과 Projectile 재사용 이상 없음 확인

확장 회귀 테스트:
- 60 / 120 FPS와 기준 속도 1배 / 2배 전체 조합
- 이동 차량 SM_Body 교차 충돌
- 동시 Projectile 다수의 성능 비용
```

공식 Editor 빌드, 일반 속도 Visual HitContext와 `30 FPS + 기준 속도 4배` P0 집중 스트레스 PIE를 통과했다. 따라서 `CF-FQ-023`은 P0 완료로 판정하며, 전체 속도·FPS 매트릭스와 이동 차량 검증은 후속 확장 회귀로 관리한다.

---

## 2. 현재 확인된 구현

현재 공통 발사체 Actor:

```text
ACFProjectileActor
```

현재 컴포넌트:

```text
CollisionComponent          = USphereComponent
MeshComponent               = UStaticMeshComponent
ProjectileMovementComponent = UProjectileMovementComponent
```

현재 활성화 흐름:

```text
ProjectileData 적용
→ CollisionRadius 적용
→ InitialSpeed / GravityScale 적용
→ ProjectileMovementComponent 활성화
→ Blocking Hit 또는 LifeTime 종료 시 비활성화
→ Pool 반환
```

현재 충돌 구조:

```text
Projectile Object Channel = Projectile
VehicleMesh                = Projectile Ignore
SM_Body                    = Projectile Block
HitComponentName           = DamageHitContext에 기록
```

일반 속도 시각 차체 충돌은 2026-07-13 PIE에서 정상 작동하는 것으로 확인됐다.

---

## 3. 구현 전 확인된 한계

2026-07-13 구현 전 프로젝트 코드에는 아래 값이 명시적으로 설정되어 있지 않았다.

```text
ProjectileMovementComponent->bSweepCollision
ProjectileMovementComponent->bForceSubStepping
ProjectileMovementComponent->MaxSimulationTimeStep
ProjectileMovementComponent->MaxSimulationIterations
```

엔진 기본값에 의존하는 상태이며, CarFight가 보장해야 하는 고속 Projectile 기준이 코드와 데이터에 명시되어 있지 않다.

또한 다음 항목이 없다.

```text
- 이전 Projectile 위치의 명시적 보존
- 이전 위치 → 현재/예측 위치 구간의 보조 Sphere Sweep
- OnComponentHit과 보조 Sweep의 중복 Hit 방지 플래그
- 속도/프레임률별 연속 충돌 테스트 기준
- 탄종별 Sub-step 설정 데이터
```

---

## 4. 확정 구조 결정

### 4.1 1차 해결책은 ProjectileMovement Sweep 명시

모든 실제 충돌 Projectile은 다음 값을 명시적으로 사용한다.

```cpp
bSweepCollision = true;
```

발사체 이동 시 CollisionComponent를 이전 위치에서 다음 위치까지 Sweep하는 것을 기본 충돌 경로로 사용한다.

`UpdatedComponent`는 반드시 `CollisionComponent`여야 한다.
시각 Mesh는 충돌 이동 기준으로 사용하지 않는다.

### 4.2 고속/중력 Projectile은 Sub-stepping 사용

고속 발사체, 중력 발사체, 유도 발사체는 한 프레임 이동을 더 작은 시간 간격으로 분할한다.

초기 P0 기본값 후보:

```text
bForceSubStepping       = true
MaxSimulationTimeStep   = 1 / 120초 ≈ 0.008333
MaxSimulationIterations = 8
```

고속 또는 강한 곡률 탄종 후보:

```text
MaxSimulationTimeStep   = 1 / 240초 ≈ 0.004167
MaxSimulationIterations = 12 ~ 16
```

최종 값은 ProjectileData에서 탄종별로 조정 가능하게 만든다.

### 4.3 CCD는 보조 수단

`BodyInstance.bUseCCD`는 보조 안전장치로 사용할 수 있다.

하지만 CarFight의 Projectile은 `ProjectileMovementComponent`가 UpdatedComponent를 이동시키므로, CCD만으로 터널링 해결을 완료했다고 보지 않는다.

우선순위:

```text
1. Sweep 이동
2. Sub-stepping
3. 보조 연속 Sphere Sweep
4. CCD 보조
```

### 4.4 잔여 터널링은 보조 Sphere Sweep으로 차단

Sweep/Sub-stepping을 명시해도 고속·얇은 충돌체·빠르게 이동하는 목표에서 누락이 남으면 다음 연속 검사를 추가한다.

```text
PreviousCollisionLocation
→ CurrentCollisionLocation 또는 PredictedNextLocation
→ CollisionRadius 기반 Sphere Sweep
→ 가장 가까운 첫 Blocking Hit 선택
```

보조 Sweep은 `LineTrace`가 아니라 실제 `CollisionRadius`를 반영한 `Sphere Sweep`을 사용한다.

```cpp
FCollisionShape::MakeSphere(CollisionRadius)
```

### 4.5 Hit 처리는 단일 함수로 통합

기존 `OnComponentHit`과 보조 Sphere Sweep이 같은 프레임에 동일 충돌을 찾을 수 있다.
따라서 두 경로가 직접 Pool 반환이나 HitContext 기록을 각각 수행하면 안 된다.

다음 의미의 단일 함수로 통합한다.

```cpp
bool ResolveProjectileImpact(
    AActor* HitActor,
    UPrimitiveComponent* HitComponent,
    const FHitResult& HitResult);
```

활성화 단위 중복 방지 상태:

```text
bImpactResolvedThisActivation
```

처리 규칙:

```text
- false일 때 첫 유효 Hit만 처리
- 처리 시작 시 즉시 true
- HitContext 저장
- Projectile 비활성화 및 Pool 반환
- 이후 OnComponentHit / Sweep 결과는 무시
- 다음 ActivateProjectile에서 false로 초기화
```

### 4.6 첫 Blocking Hit만 유효

하나의 Sweep 구간에 여러 충돌 후보가 있어도 이동 시작점에서 가장 가까운 첫 Blocking Hit만 처리한다.

```text
벽 뒤의 차량보다 앞의 벽이 먼저 맞아야 함
차량 앞 장갑 표면보다 차량 내부 파츠가 먼저 처리되면 안 됨
```

### 4.7 자기 차량과 발사 주체 Ignore 유지

기존 `IgnoreActorWhenMoving()` 정책과 별도로 보조 Sweep의 QueryParams에도 발사 주체를 Ignore한다.

```text
InstigatorActor
OwnerActor
ProjectileActor 자신
```

필요 시 발사 직후 총구 주변 자기 차체와 충돌하지 않도록 짧은 거리 또는 시간 기반 안전 구간을 후속 옵션으로 검토할 수 있다.
기본안은 명시적 Ignore Actor 정책이다.

---

## 5. ProjectileData 확장 구현

탄종별 성능과 충돌 비용을 조정할 수 있도록 다음 필드를 `UCFProjectileData`에 구현했다.

```text
bUseSweepCollision
bForceSubStepping
MaxSimulationTimeStep
MaxSimulationIterations
bUseSupplementalContinuousSweep
bUseCCD
```

권장 기본값:

```text
bUseSweepCollision              = true
bForceSubStepping               = true
MaxSimulationTimeStep           = 0.008333
MaxSimulationIterations         = 8
bUseSupplementalContinuousSweep = true
bUseCCD                         = false 또는 보조 true
```

P0에서는 모든 실제 Projectile에 안전한 기본값을 적용하고, 성능 문제가 확인되면 저속 탄종만 완화한다.

---

## 6. Projectile 분류 기준

모든 무기를 같은 런타임 판정 방식으로 처리하지 않는다.

| 무기 성격 | 권장 판정 |
| --- | --- |
| 레이저 | HitScan |
| 일반 기관총탄 | HitScan 판정 + 시각 Tracer |
| 초고속 철갑탄 / 레일건 | HitScan 또는 지연 HitScan |
| 기관포탄 | Swept Projectile |
| 전차포탄 | Swept Projectile + 후속 탄도 |
| 로켓 / 미사일 | Sub-step Projectile |
| 수류탄 / 곡사탄 | Sub-step Ballistic Projectile |

플레이어가 비행 시간을 인지할 수 없는 초고속 탄환은 Projectile Actor를 실제 판정에 강제로 사용하지 않는다.

```text
실제 판정 = HitScan
시각 표현 = Tracer Actor
```

비행 시간, 회피, 낙하가 게임플레이 요소인 탄종만 실제 Projectile 판정을 사용한다.

---

## 7. 구현 단계

### Phase 1 — 현재 이동 설정 Debug

```text
1. UpdatedComponent가 CollisionComponent인지 확인
2. bSweepCollision 현재값 기록
3. bForceSubStepping 현재값 기록
4. MaxSimulationTimeStep / Iterations 기록
5. InitialSpeed, CollisionRadius, 프레임 Delta 기록
6. 한 프레임 이동 거리 계산
```

한 프레임 이동 거리:

```text
TravelDistancePerFrame = Speed * DeltaSeconds
```

이 값이 대상 충돌체 두께보다 크면 고속 검증 대상으로 본다.

### Phase 2 — Sweep / Sub-step 명시

```text
1. bSweepCollision = true
2. UpdatedComponent = CollisionComponent 보장
3. bForceSubStepping = ProjectileData 값
4. MaxSimulationTimeStep 적용
5. MaxSimulationIterations 적용
6. 기존 OnComponentHit 경로 유지
```

### Phase 3 — 고속 회귀 테스트

```text
1. 현재 속도 1배
2. 현재 속도 2배
3. 현재 속도 4배
4. 30 / 60 / 120 FPS 제한
5. 얇은 벽과 SM_Body 차체
6. 정지 대상과 이동 대상
```

Phase 2만으로 모든 테스트가 통과하면 보조 Sweep은 비활성 상태로 둘 수 있다.

### Phase 4 — 보조 Sphere Sweep

터널링이 남을 때 적용한다.

```text
1. 활성화 시 PreviousCollisionLocation 초기화
2. Projectile 이동 후 현재 위치 확보
3. Previous → Current Sphere Sweep
4. 첫 Blocking Hit 선택
5. ResolveProjectileImpact() 호출
6. Previous 위치 갱신
7. 비활성 상태에서는 검사하지 않음
```

이동 후 검사하는 경우 ProjectileMovementComponent보다 늦게 실행되도록 Tick 순서를 명시적으로 보장한다.
Tick 순서가 보장되지 않으면 별도 Projectile Movement 파생 컴포넌트에서 이동과 Sweep을 통합한다.

### Phase 5 — 중복 Hit 통합

```text
1. HandleProjectileHit()은 ResolveProjectileImpact()만 호출
2. 보조 Sweep도 ResolveProjectileImpact()만 호출
3. bImpactResolvedThisActivation으로 1회 처리 보장
4. Pool 반환 전 HitActor / HitComponent / Transform 보존
5. Pool 반환 후 DamageHitContext 정상 확인
```

### Phase 6 — 성능 검증

```text
- 동시 Projectile 10 / 50 / 100개
- Sweep/Sub-step 활성 상태 프레임 비용
- Pool 재사용 시 이전 위치/Hit 상태 초기화
- 비활성 Projectile이 Tick/Sweep하지 않는지 확인
```

---

## 8. C++ / BP / Data 책임 분리

### C++

```text
- ProjectileMovement Sweep/Sub-step 적용
- UpdatedComponent 보장
- PreviousLocation 저장
- 보조 Sphere Sweep 수행
- 첫 Hit 선택
- 중복 Hit 방지
- HitContext 기록과 Pool 반환
- Debug 값 제공
```

### BP

```text
- Projectile 메시와 시각 효과 구성
- 충돌 판정 계산을 BP Tick에 구현하지 않음
- OnHit에서 별도 Damage를 중복 적용하지 않음
```

### ProjectileData

```text
- InitialSpeed
- CollisionRadius
- 중력 여부/GravityScale
- Sweep/Sub-step 사용 여부
- Simulation TimeStep / Iterations
- 보조 Sweep 사용 여부
```

---

## 9. 완료 기준

```text
1. ProjectileMovement의 UpdatedComponent가 CollisionComponent다.
2. 실제 Projectile은 bSweepCollision=true를 명시적으로 사용한다.
3. 고속/중력 Projectile은 Sub-stepping 설정을 데이터에서 적용한다.
4. 일반 속도에서 기존 SM_Body 충돌이 유지된다.
5. 30 FPS에서 현재 속도 4배 Projectile이 얇은 충돌체를 통과하지 않는다.
6. 이동 차량의 SM_Body에서도 Hit이 누락되지 않는다.
7. 한 발사체가 동일 충돌을 두 번 처리하지 않는다.
8. 첫 Blocking Hit만 DamageHitContext에 기록된다.
9. 자기 차량 Ignore가 유지된다.
10. HitComponentName이 SM_Body 또는 실제 월드 충돌 컴포넌트로 기록된다.
11. Pool 반환 후 다음 활성화에서 이전 위치/Hit 상태가 초기화된다.
12. LifeExpired와 Manual 비활성화에서는 잘못된 HitContext가 생성되지 않는다.
13. Unreal Editor 타깃 빌드가 성공한다.
14. 싱글 PIE 고속 테스트를 통과한다.
```

---

## 10. 테스트 매트릭스

### 속도

```text
- 기준 InitialSpeed
- 기준의 2배
- 기준의 4배
- 프로젝트에서 허용할 최대 Projectile 속도
```

### 프레임률

```text
- 30 FPS
- 60 FPS
- 120 FPS
```

### 대상

```text
- 얇은 월드 벽
- 두꺼운 월드 벽
- 정지 차량 SM_Body
- 주행 차량 SM_Body
- 비스듬한 차체 표면
- 차체 모서리
```

### 결과 확인

```text
- Projectile이 충돌 지점에서 정지/비활성화
- HitActor 정확
- HitComponentName 정확
- ImpactLocation / ImpactNormal 정상
- LastDeactivateReason = Hit
- Pool 반환 1회
- DamageHitContext 저장 1회
```

---

## 11. 실패 분류

```text
MovementConfig
- Sweep/Sub-step 값 미적용

TickOrder
- Previous/Current 위치 검사 순서 오류

CollisionResponse
- Projectile Object Channel 응답 오류

AssetCollision
- 대상 SM_Body Simple Collision 누락/부정확

DuplicateHit
- OnComponentHit과 보조 Sweep 중복 처리

PoolState
- 재사용 Projectile의 PreviousLocation/Hit 플래그 초기화 누락

Performance
- 과도한 Sub-step/Sweep으로 프레임 비용 증가
```

---

## 12. 이번 범위에서 제외

```text
- 실제 HP 감소
- 장갑 관통과 도탄
- 폭발 범위 피해
- 완성형 탄도 Solver
- 유도 미사일 조향 로직
- 네트워크 Projectile 복제
- 서버 권한 Hit 판정
- 파괴 연출
- Tracer 전용 시각 시스템 완성
```

---

## 13. 구현 기본값 및 남은 튜닝 항목

2026-07-14 P0 코드 기본값:

```text
- 모든 기존 실제 ProjectileData: 보조 Sphere Sweep 기본 활성
- MaxSimulationTimeStep: 0.008333초, 약 1/120초
- MaxSimulationIterations: 8
- CCD: 기본 비활성, 탄종별 보조 옵션
- 보조 Sweep 위치: ACFProjectileActor PostPhysics Tick
```

남은 튜닝 항목:

```text
- 동시 발사체 수에 따른 보조 Sweep과 Sub-step 비용
- 저속 탄종에서 보조 Sweep을 비활성화할 기준
- 1/240초와 12~16회 반복이 필요한 고곡률 탄종 기준
- 실제 Projectile 방식과 HitScan+Tracer 방식의 무기별 분류
- ACFProjectileActor 구현을 별도 UCFProjectileMoveComp로 분리할 필요성
```

구조 기본값은 코드에 반영됐지만, 성능과 고속 충돌 PIE 결과 전에는 최종 튜닝값으로 확정하지 않는다.

---

## 14. Migration

### v0.4.0 P0 완료 판정 적용

```text
- 30 FPS와 기준 InitialSpeed 4배의 집중 스트레스 테스트를 P0 완료 기준으로 적용했다.
- 차량 집중 발사, 얇은 벽 앞 차량, 첫 Blocking Hit, 중복 Impact와 Pool 재사용 검증 통과를 기록했다.
- 수행하지 않은 60 / 120 FPS 전체 매트릭스와 이동 차량 검증은 CF-FQ-019 확장 회귀로 이관했다.
- Projectile 코드와 DataAsset 기본값은 변경하지 않았다.
```

### v0.3.0 세션 복원 체크포인트 적용

```text
- 문서 상단에 현재 상태, 완료 범위, 미검증 범위, 다음 작업, 관련 경로와 보호 범위를 추가했다.
- 기존 구현 설계와 코드·에셋의 동작은 변경하지 않았다.
- 새 세션은 이 체크포인트를 우선 읽고 실제 Git 상태와 코드를 교차검증한다.
```

### v0.2.0 코드 적용

```text
- 기존 ProjectileData 자산은 신규 연속 충돌 필드의 C++ 기본값을 사용하며 이번 작업에서 .uasset을 저장하지 않았다.
- 기존 ProjectileActor Blueprint 부모와 시각 에셋은 변경하지 않았다.
- 기존 OnComponentHit 경로는 제거하지 않고 ResolveProjectileImpact()로 위임하도록 변경했다.
- 보조 Sphere Sweep도 같은 ResolveProjectileImpact()를 사용해 활성화별 첫 Impact만 처리한다.
- Pool 재활성화 시 PreviousCollisionLocation과 bImpactResolvedThisActivation을 초기화한다.
- 기존 Projectile Object Channel, SM_Body 차체 충돌, LastHitComponentName, DamageHitContext 전달 구조를 유지한다.
- 실제 HP 감소, 장갑 계산, 파괴 상태는 아직 연결하지 않았다.
- 2026-07-14 사용자 실행 `Tools\\BuildEditor.bat` 공식 Editor 빌드는 성공했다.
- 사용자 PIE 검증 전에는 완료 또는 PASS로 판정하지 않는다.
```

---

## 15. Changelog

### v0.4.0 - 2026-07-14

```text
- 사용자 PIE 축약 스트레스 테스트 통과를 기록했다.
- 30 FPS + 기준 속도 4배 차량 집중 발사, 얇은 벽 첫 Blocking Hit, 중복 Impact와 Pool 재사용 검증을 PASS 처리했다.
- CF-FQ-023을 P0 Completed로 판정했다.
- 전체 FPS·속도 매트릭스와 이동 차량 검증은 CF-FQ-019 확장 회귀로 이관했다.
```

### v0.3.3 - 2026-07-14

```text
- 사용자 PIE에서 피격 Actor BP_CFVehiclePawn_C_1과 피격 컴포넌트 SM_Body 기대값 출력 확인
- 일반 속도 Visual HitContext 경로를 PIE PASS로 기록
- 남은 검증 범위를 고속·저프레임·얇은 충돌체·중복 처리 테스트로 축소
- 고속 PIE 완료 전 CF-FQ-023 전체 PASS 또는 Done 판정을 보류
```

### v0.3.2 - 2026-07-14

```text
- 사용자 실행 Tools\\BuildEditor.bat에서 피격 컴포넌트 독립 표시 보강분 공식 Editor 빌드 성공 기록
- 문서 상태를 Official Build Passed / HitComponent PIE Pending / High-Speed PIE Pending으로 변경
- 남은 작업을 피격 컴포넌트 SM_Body 확인과 고속 충돌 PIE로 축소
```

### v0.3.1 - 2026-07-14

```text
- 사용자 PIE에서 일반 속도 Projectile 피격과 피격 Actor BP_CFVehiclePawn_C_1 기록 확인
- HitComponentName 데이터 경로는 존재하지만 Debug Panel에 독립 표시 행이 없던 상태 확인
- Damage HitContext에 피격 컴포넌트 독립 표시 행 추가
- 표시 보강분 공식 빌드와 SM_Body 재확인, 고속 PIE는 Pending으로 유지
```

### v0.3.0 - 2026-07-14

```text
- 다중 작업 세션 복원을 위한 표준 현재 작업 체크포인트 추가
- 공식 Editor 빌드 PASS와 사용자 PIE Pending 상태를 상단에서 즉시 확인하도록 정리
- 완료 범위, 미검증 항목, 다음 작업, 관련 경로와 보호 범위를 명시
```

### v0.2.1 - 2026-07-14

```text
- 사용자 실행 Tools\\BuildEditor.bat 공식 Editor 빌드 성공을 기록
- 문서 상태를 Official Build Passed / PIE Pending으로 변경
- PIE 결과 확인 전에는 PASS 또는 Done으로 판정하지 않는 기준 유지
```

### v0.2.0 - 2026-07-14

```text
- UCFProjectileData에 Sweep / Sub-step / 보조 연속 Sphere Sweep / CCD 설정과 안전 기본값 추가
- ACFProjectileActor PostPhysics Tick 기반 Previous → Current Sphere Sweep 구현
- OnComponentHit과 보조 Sweep을 ResolveProjectileImpact로 통합
- 활성화별 중복 Impact 방지와 Pool 재활성화 상태 초기화 구현
- 기존 Projectile 채널, 시각 차체 충돌, HitComponentName, DamageHitContext 전달 경로 유지
- 공식 BuildEditor.bat 빌드와 PIE 검증은 Pending으로 기록
```

### v0.1.0 - 2026-07-13

```text
- 고속 Projectile이 프레임 사이 충돌체를 통과하는 사용자 PIE 관찰 기록
- Sweep 명시, Sub-stepping, 보조 Sphere Sweep의 단계적 해결 구조 정의
- CCD를 보조 수단으로 분리
- OnComponentHit과 보조 Sweep의 단일 Impact 처리 및 중복 방지 정책 정의
- 탄종별 HitScan / Swept Projectile / Ballistic Projectile 분류 기준 추가
- 속도·프레임률·대상별 테스트 매트릭스 정의
```

---

## 16. 확인 근거

- 설계 확인 일시: 2026-07-13
- 코드 반영 확인 일시: 2026-07-14
- 확인 대상:
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Private/CFProjectileData.cpp`
  - `UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp`
  - `Document/Systems/Combat/Projectile.md`
  - `Document/Systems/Combat/DamageHitContext.md`
  - 2026-07-13 사용자 PIE 관찰: 일반 충돌 정상, 고속 이동 시 터널링 발생
- 확인 완료:
  - 2026-07-14 사용자 실행 `Tools\\BuildEditor.bat` 공식 Editor 빌드 성공
  - 2026-07-14 사용자 PIE 일반 속도 피격 확인
  - 피격 Actor `BP_CFVehiclePawn_C_1`, 피격 컴포넌트 `SM_Body` 기록 확인
- 미확인:
  - 30 / 60 / 120 FPS와 기준 속도 1배 / 2배 / 4배 고속 Projectile 사용자 PIE 검증
  - 얇은 벽, 이동 차량, 비스듬한 표면과 모서리 충돌 및 중복 처리 검증
