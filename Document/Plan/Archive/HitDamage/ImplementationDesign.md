# CarFight — HitDamage 구현 설계

- Version: 0.6.2
- Date: 2026-07-16
- Status: Done / Functional Build and User PIE PASS / Workflow Compliance FAIL — Final Codex Work Order Missing / Systems Promoted
- Feature: `CF-FQ-018 피격 판정 및 피해 처리`
- Scope: 검증 완료된 HitContext를 `BaseDamage`, 차량 체력 감소, 파괴 상태와 BP 이벤트로 연결한 최소 Damage Runtime 구현·검증 기준

---

## 현재 작업 체크포인트

- 현재 상태: 최소 Damage Runtime C++ 구현, 공식 Editor 빌드와 사용자 싱글 PIE 검증을 완료했다.
- 빌드 상태: `CarFight_ReEditor Win64 Development` PASS (`D:\UnrealEngine_Source`)
- PIE 상태: 체력 누적 감소, 체력 0 이하 파괴 전환, 파괴 이후 추가 피해 거부와 이벤트 1회성 PASS
- 기능 판정: `CF-FQ-018` Done / `CF-TC-016` PASS / Current System 승격 완료
- 워크플로 준수: FAIL — 최소 Damage Runtime 구현 전에 준비됐어야 할 최종 Codex 작업지시서 출처를 확인할 수 없다.

### 코드 작업 준비·실행 출처

- 준비 방식: `Policy Violation — Browser Direct Edit`
- TaskSource: `Missing` — `UCFVehicleHealthComp` 또는 최소 Damage Runtime 구현을 허용하는 TaskSource를 찾지 못했다.
- 최종 Codex 작업지시서: `Missing`
- 작업지시서 상태: `Missing / Provenance Missing`
- 외부 Codex 실행 상태: `Unknown` — 해당 구현이 별도 Codex 환경에서 실행됐다는 증거가 없다.
- 브라우저 검수: 기능 결과는 기존 빌드·사용자 PIE 증거로 PASS지만, 구현 준비·출처 검수는 FAIL이다.
- 빌드 상태: 기존 `CarFight_ReEditor Win64 Development` PASS 유지
- PIE 상태: 기존 사용자 싱글 PIE PASS 유지
- 감사 근거: `Document/Plan/HitDamage/TaskSource_VisualHitCollision.md`는 실제 HP 감소, Health Component와 파괴 상태 구현을 명시적으로 금지하므로 Damage Runtime의 작업지시서 출처로 사용할 수 없다.
- 처리 원칙: 사후 TaskSource나 최종 YAML을 생성해 원래 구현이 Codex 작업지시서에 따라 수행됐던 것처럼 기록하지 않는다.

### 완료한 범위

- 기존 `WeaponHit`, `Projectile`, `VehicleVisualHit`, `SM_Body` 피격 계약 유지
- `FCFVehicleDurabilityConfig.MaxHealth`를 `VehicleData`에 추가하고 기본값 `100` 적용
- 신규 `UCFVehicleHealthComp`에서 `MaxHealth`, `CurrentHealth`, 초기화 여부와 파괴 상태 소유
- `DamageData.BaseDamage`를 직접 체력 감소에 연결하고 기본값 `25` 유지
- `DamageData.bCanDamageSelf=false` 자기 피해 금지 정책 추가
- `FCFDamageApplyResult`와 명시적인 피해 적용 거부 사유 추가
- HitScan과 Projectile이 같은 `FCFDamageHitContext`와 `TryApplyDamageToActor()` 진입점 사용
- Projectile 첫 유효 Impact에서만 피해를 적용하고 Pool 반환에서는 저장 결과만 복사해 중복 차감 방지
- 체력 0 이하에서 파괴 상태와 `OnVehicleDestroyed` 이벤트를 한 번만 발생
- `OnVehicleHealthChanged`, `OnVehicleDamaged`, `OnVehicleDestroyed` BP 이벤트 제공
- VehicleDebug Panel에 피해 적용 여부, 거부 사유, 체력 전후 값과 파괴 전환 요약 추가
- 기존 DataAsset과 WBP/.uasset은 저장하지 않고 C++ 기본값으로 Migration
- 공식 Editor 빌드 성공

### 미완료 또는 후속 범위

- DamageData 미연결, 비차량 Actor와 자기 피해 금지 등 개별 거부 사유의 확장 회귀
- 파괴 시 입력·Chaos 물리 정지와 완성형 파괴 연출
- 장갑, 모듈, 부위별 피해와 범위 피해
- 주행·거리별 조준, 이동 차량과 반복 전투는 `CF-FQ-019` 확장 회귀

### 완료 후 다음 작업

1. 브라우저 AI는 기존 최소 Damage Runtime 구현을 **새 구현 출처로 소급하지 않는 검토·보정 작업**으로 분리해 TaskSource와 최종 Codex YAML 작업지시서를 생성한다.
2. 품질·증거 게이트와 `final_output_ready`를 통과한 뒤 사용자에게 TaskSource와 최종 YAML 경로를 전달하고 `Ready for External Codex`로 기록한다.
3. 사용자는 별도 Codex 세션 또는 선택한 Codex 환경에서 YAML을 실행해 현재 구현 diff, `UCFVehicleHealthComp`, 공용 피해 적용 경로와 회귀 위험을 검토·보정한다.
4. 외부 Codex 적용 후 브라우저 AI가 실제 diff, 빌드·테스트와 필요한 PIE를 검수한다.
5. 이후 `CF-FQ-017`의 `NoWeapon`, `AimBlocked` 최종 PIE 검증과 `CF-FQ-019` 주행·전투 반복 테스트를 진행한다.
6. HitDamage의 현재 구현을 실제로 변경하는 작업도 `Document/CodeWorkGate.md`의 작업지시서 생성 절차를 거치고, 검증 후 `Document/Systems/Combat/HitDamage.md`를 동기화한다.

### 관련 코드와 문서 경로

- `UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleHealthComp.cpp`
- `UE/Source/CarFight_Re/Public/CFDamageTypes.h`
- `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
- `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
- `Document/CodeWorkGate.md`
- `Document/Plan/HitDamage/TaskSource_VisualHitCollision.md`
- `Document/Systems/Combat/HitDamage.md`
- `Document/Systems/Combat/DamageHitContext.md`
- `Document/Systems/Combat/Projectile.md`
- `Document/Systems/Vehicles/VehicleCoreDecisions.md`
- `Document/Plan/AimFireAlignment/ImplementationDesign.md`
- `Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md`

### 보호 범위

- 선행 PIE 검증 전에 HP 감소와 파괴 상태를 먼저 연결하지 않는다.
- 확정된 Collision Channel과 `VehicleVisualHit` Profile을 다시 설계하지 않는다.
- 사용자 확인 전에는 선행 작업이나 Damage Runtime을 PASS 또는 Completed로 기록하지 않는다.

---

## 1. 목적

이 문서는 `CF-FQ-018`의 시각 차체 피격 구현 결과와 실제 Damage Runtime 착수 전 선행 조건을 관리한다.

핵심 결정은 다음과 같다.

> CarFight의 무기 피격 형상은 `SM_Body` 시각 차체를 기준으로 하며, 실제 피해 처리는 Reticle·터렛·총구 정렬과 고속 Projectile 연속 충돌이 보장된 뒤 연결한다.

현재 진행 상태:

```text
차량 물리 충돌과 무기 피격 충돌 분리       완료
SM_Body 기반 HitScan / Projectile 피격       일반 속도 PIE 확인
HitComponentName DamageHitContext 기록       완료
Reticle·터렛·Muzzle Aim Solution 통합        코드·공식 빌드·P0 사용자 PIE 완료
고속 Projectile 연속 충돌                    코드·공식 빌드·P0 사용자 PIE 완료
BaseDamage / 체력 감소 / 파괴 상태            코드·공식 빌드·사용자 PIE 완료
```

현재 후속 순서:

```text
CF-FQ-022 조준점·터렛·총구 정렬 = Done
CF-FQ-023 고속 Projectile 연속 충돌 = Done
→ 신뢰 가능한 DamageHitContext 입력 계약 준비 완료
→ CF-FQ-018 최소 Damage Runtime 구현
```

---

## 2. 현재 확인된 차량 구조

현재 주력 차량 자산:

```text
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn
```

현재 주요 컴포넌트 역할은 아래와 같다.

| 컴포넌트 | 타입 | 현재 역할 |
| --- | --- | --- |
| `VehicleMesh` | SkeletalMeshComponent | Chaos Vehicle 물리와 Physics Asset 충돌 |
| `SM_Body` | StaticMeshComponent | 화면에 보이는 차량 차체 |
| `SM_Body1` | StaticMeshComponent | 현재 메시 미지정 보조 시각 컴포넌트 |
| `Wheel_Mesh_*` | StaticMeshComponent | 휠 시각 표현 |
| `Turret_*Mesh` | StaticMeshComponent | 터렛 시각 표현 |

---

## 3. 현재 구현된 충돌 설정

### 3.1 공용 Collision 기준

```text
WeaponHit Trace Channel   = ECC_GameTraceChannel1
Projectile Object Channel = ECC_GameTraceChannel2
VehicleVisualHit Profile  = QueryOnly
```

C++ 단일 기준은 `CFCollisionChannels.h`가 제공한다.

### 3.2 물리 메시 `VehicleMesh`

```text
Collision Enabled = 기존 QueryAndPhysics 유지
Collision Profile = 기존 Vehicle 유지
WeaponHit         = Ignore
Projectile        = Ignore
```

따라서 `VehicleMesh`는 Chaos Vehicle 물리와 월드 충돌을 담당하며 무기 피격 Query를 가로채지 않는다.

### 3.3 시각 차체 `SM_Body`

```text
Collision Enabled = QueryOnly
Collision Profile = VehicleVisualHit
WeaponHit         = Block
Projectile        = Block
Simulate Physics  = false
```

### 3.4 HitScan / Projectile 기록

```text
Dummy HitScan = WeaponHit Trace
Projectile    = Projectile Object Type
HitComponent  = FCFDamageHitContext.HitComponentName
```

2026-07-13 사용자 PIE에서 일반 속도 HitScan과 Projectile이 `SM_Body`에서 정상 충돌하는 것을 확인했다.

---

## 4. 해결 완료된 Damage Runtime 선행 조건

시각 차체 피격 분리 이후 Damage Runtime 입력 신뢰성을 막던 두 문제는 P0 구현과 사용자 PIE를 통과했다.

### 4.1 Aim Solution 정렬 완료

```text
Reticle 목표점       = DesiredAimTargetLocation
터렛 요구 방향      = Muzzle → DesiredAimTargetLocation
정렬 중 실제 방향   = 정책 true일 때 CurrentMuzzleDirection
정렬 완료 실제 방향 = DesiredAimDirection
```

정책 true/false 정렬 중 발사와 정렬 완료 후 Reticle 탄착, 정책 양쪽 `MuzzleBlocked`를 사용자 PIE에서 확인했다.

### 4.2 고속 Projectile 연속 충돌 완료

```text
ProjectileMovement Sweep / Sub-step
+ Previous → Current 보조 Sphere Sweep
+ 활성화별 첫 Impact 1회 처리
```

`30 FPS + 기준 속도 4배` 차량 집중 발사, 얇은 벽 첫 Blocking Hit, 중복 Impact 방지와 Pool 재사용을 사용자 PIE에서 확인했다.

따라서 현재 `DamageHitContext`는 최소 Damage Runtime을 연결할 P0 입력으로 사용 가능하다.

관련 완료 설계:

```text
Document/Plan/AimFireAlignment/ImplementationDesign.md
Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md
```

---

## 5. 확정 구조 결정

### 5.1 `VehicleMesh`는 차량 물리 전용

`VehicleMesh`는 아래 책임을 유지한다.

```text
- Chaos Vehicle 시뮬레이션
- 차량 질량과 물리 상태
- 지면, 벽, 다른 차량과의 물리 충돌
- 주행과 서스펜션의 물리 기준
```

무기 피격 Query에서는 제외한다.

```text
- HitScan 전용 채널 Ignore
- Projectile Object Type Ignore
```

`VehicleMesh`의 일반 물리 Collision을 끄지 않는다.
그렇게 하면 차량 주행과 월드 충돌이 깨질 수 있다.

### 5.2 `SM_Body`는 차체 무기 피격 기준

`SM_Body`는 아래 책임을 가진다.

```text
- 화면에 보이는 차체 표현
- HitScan 차체 피격 표면
- Projectile 차체 피격 표면
- HitResult.HitComponent의 P0 차체 기준
```

목표 설정:

```text
Collision Enabled = QueryOnly
Simulate Physics   = false
무기 HitScan       = Block
Projectile         = Block
차량 물리 채널     = Ignore 또는 물리 상호작용 없음
```

### 5.3 P0의 피격 파츠 범위

P0에서는 우선 `SM_Body`만 피해 가능한 차량 표면으로 본다.

```text
SM_Body       = 피격 대상
Wheel_Mesh_*  = 무기 피격 Ignore
Turret Mesh   = 우선 무기 피격 Ignore
VehicleMesh   = 무기 피격 Ignore
```

휠, 터렛, 장갑 패널, 모듈별 독립 피해는 후속 부위 피해 시스템으로 분리한다.

---

## 6. 시각 메시 Collision Asset 기준

`SM_Body`가 Query Collision을 사용하려면 실제 StaticMesh 자산에 Collision 형상이 존재해야 한다.

권장 기준:

```text
- 시각 메시를 따라 만든 Simple Collision 또는 Convex Collision 사용
- 차량 외곽을 지나치게 크게 감싸는 단일 Box는 지양
- 매 프레임 움직이는 차량에 무조건 Complex Collision을 사용하는 방식은 기본안으로 채택하지 않음
- 차종별 StaticMesh 자산의 Simple Collision 존재 여부를 구현 전에 확인
```

피격 정확도 기준은 시각 메시이지만, 실제 런타임 Query는 시각 메시에서 파생한 단순 Collision 형상을 사용하는 것을 기본안으로 본다.

---

## 7. 확정된 Collision Channel / Profile

현재 구현 기준:

```text
Trace Channel    : WeaponHit
Trace Slot       : ECC_GameTraceChannel1
Object Channel   : Projectile
Object Slot      : ECC_GameTraceChannel2
Collision Profile: VehicleVisualHit
```

현재 응답표:

| 대상 | 일반 물리 | WeaponHit | Projectile |
| --- | ---: | ---: | ---: |
| `VehicleMesh` | 사용 | Ignore | Ignore |
| `SM_Body` | 물리 미사용 | Block | Block |
| `Wheel_Mesh_*` | 물리 미사용 | Ignore | Ignore |
| 터렛 시각 메시 | 물리 미사용 | P0 Ignore | P0 Ignore |
| 벽과 지형 | 사용 | Block | Block |

이 설정은 `DefaultEngine.ini`와 `CFCollisionChannels.h`가 공유한다.

---

## 8. 구현 순서와 현재 상태

### Phase 1 — 시각 차체 Collision 자산/구조 확인

```text
상태: 완료
- 기준 차량 BodyStaticMesh와 SM_Body 적용 경로 확인
- VehicleMesh / SM_Body 기존 충돌 차이 확인
```

### Phase 2 — 물리 충돌과 무기 피격 충돌 분리

```text
상태: 완료
- WeaponHit / Projectile / VehicleVisualHit 구현
- VehicleMesh 무기 Query Ignore
- SM_Body QueryOnly + 무기 채널 Block
```

### Phase 3 — HitScan 경로 전환

```text
상태: 완료
- Dummy HitScan을 WeaponHit으로 전환
- HitComponentName 기록
- 일반 속도 PIE에서 SM_Body Hit 확인
```

### Phase 4 — Projectile 경로 전환

```text
상태: 완료 / P0 사용자 PIE 확인
- Projectile Object Type 적용
- VehicleMesh Ignore / SM_Body Block 구현
- 일반 속도 PIE에서 SM_Body Hit 확인
- 고속 연속 충돌 공식 빌드와 30 FPS + 기준 속도 4배 사용자 PIE 확인
```

### Phase 5 — Aim Solution 통합

```text
상태: 완료 / P0 사용자 PIE 확인
기준 문서: Document/Plan/AimFireAlignment/ImplementationDesign.md
- Reticle 목표점 SSOT
- 터렛 / Muzzle / 실제 발사 방향 통일
- 정렬 오차와 총구 장애물 검사
- 정렬 중 발사 정책 true/false와 정렬 완료 탄착 확인
```

### Phase 6 — 고속 Projectile 연속 충돌

```text
상태: 완료 / P0 사용자 PIE 확인
기준 문서: Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md
- Sweep / Sub-stepping 명시
- 보조 Sphere Sweep
- 단일 Impact 처리와 중복 방지
- 차량 집중 발사, 얇은 벽 첫 Blocking Hit와 Pool 재사용 확인
```

### Phase 7 — 최소 피해 처리

```text
상태: 완료 / 공식 Editor 빌드 PASS / 사용자 싱글 PIE PASS
1. UCFVehicleHealthComp로 차량 최대/현재 체력과 파괴 상태 소유
2. DamageData.BaseDamage를 공용 피해 적용 입력으로 사용
3. HitScan과 Projectile이 FCFDamageHitContext 및 TryApplyDamageToActor 진입점 공유
4. CurrentHealth 감소와 FCFDamageApplyResult 기록
5. 0 이하에서 파괴 상태와 OnVehicleDestroyed 이벤트 1회 전환
6. OnVehicleHealthChanged / OnVehicleDamaged / OnVehicleDestroyed BP 이벤트 제공
7. Projectile 첫 Impact에서만 적용하고 Pool 반환에서는 결과만 복사
```

---

## 9. C++ / BP / Asset 책임 분리

### C++

```text
- 전용 Collision Channel 상수 사용
- HitScan Trace 채널 선택
- Projectile Object Type과 충돌 응답 적용
- VehicleMesh / SM_Body 역할 검증과 안전한 기본값 적용
- HitResult.HitComponent 기록
- DamageHitContext 생성
- 피해 계산과 체력 변경
- 파괴 상태 판정
```

### BP

```text
- BP_CFVehiclePawn 컴포넌트 조립 유지
- SM_Body 정확한 이름과 컴포넌트 연결 유지
- 차량별 시각 메시와 Collision Profile 확인
- 피격/파괴 시각 반응 연결
- 상태 판정과 피해 계산 로직은 BP에 추가하지 않음
```

### StaticMesh Asset

```text
- 차량 시각 차체 메시의 Simple/Convex Collision 제공
- 시각 외곽과 피격 외곽의 오차 관리
- 차종별 Collision 형상 품질 확인
```

### Config

```text
- DefaultEngine.ini 또는 Project Settings에 Collision Channel/Profile 정의
- 채널 슬롯과 이름을 코드 상수와 일치시킴
```

---

## 10. 선행 검증 기준

피해 처리 코드에 들어가기 전에 아래 조건을 통과해야 한다.

```text
1. VehicleMesh Physics Asset의 보이지 않는 돌출부는 무기 피격되지 않는다. [확인 완료]
2. SM_Body의 보이는 차체 표면에서 일반 속도 HitScan/Projectile이 명중한다. [확인 완료]
3. HitComponentName이 SM_Body 또는 승인된 시각 피격 컴포넌트로 기록된다. [확인 완료]
4. Reticle 목표점과 Muzzle 요구 발사 방향이 동일 Aim Solution을 사용한다. [확인 완료]
5. 터렛별 정렬 중 발사 정책과 총구 앞 장애물 차단이 의도대로 동작한다. [확인 완료]
6. P0 집중 조건의 고속/저프레임 Projectile이 차량과 얇은 벽을 통과하지 않는다. [확인 완료]
7. 동일 Projectile Impact가 정확히 한 번만 HitContext에 기록된다. [확인 완료]
8. 차량 주행, 지면 충돌, 벽 충돌은 기존처럼 정상이다. [시각 피격 변경 기준 확인]
9. 발사 주체 자기 차량 Ignore가 유지된다.
10. DamageHitContext의 HitActor, HitComponentName, 위치, 노멀, 입사 방향 기록이 유지된다.
```

---

## 11. 피해 처리 완료 기준

```text
1. 유효한 시각 차체 피격만 피해 입력으로 전달된다.
2. BaseDamage가 대상 차량 체력에서 정확히 한 번 차감된다.
3. Miss, 월드 충돌, 무효 대상은 차량 체력을 감소시키지 않는다.
4. 자기 피해 허용 정책이 명시적으로 동작한다.
5. 연속 사격에서 피해가 정상 누적된다.
6. 체력이 0 이하가 되면 파괴 상태가 한 번만 발생한다.
7. HitScan과 Projectile이 같은 피해 적용 인터페이스를 사용한다.
8. 싱글 PIE와 Unreal Editor 타깃 빌드가 통과한다.
```

---

## 12. 이번 범위에서 제외

```text
- 부위별 장갑 두께
- 입사각과 도탄 계산
- 휠 독립 파괴
- 터렛 독립 파괴
- 엔진/변속기/연료 모듈 피해
- 폭발 범위 피해
- 지속 피해와 화재
- 서버 권한 피해와 복제
- 리스폰
- 완성형 파괴 연출
- Geometry Collection 차량 파괴 전환
```

---

## 13. 현재 확정 및 후속 항목

확정:

```text
- 자기 피해 기본값은 DamageData.bCanDamageSelf=false다.
- 최대 체력 데이터는 VehicleData.VehicleDurabilityConfig.MaxHealth가 소유한다.
- 런타임 체력과 파괴 상태는 UCFVehicleHealthComp가 소유한다.
- 공용 피해 적용 진입점은 UCFVehicleHealthComp::TryApplyDamageToActor다.
- BP 이벤트는 OnVehicleHealthChanged / OnVehicleDamaged / OnVehicleDestroyed다.
```

후속:

```text
- 파괴 상태에서 차량 입력과 Chaos 물리를 언제 중지할지
- 파괴 VFX/SFX와 완성형 파괴 연출
- 장갑, 모듈, 부위별 피해와 폭발 피해
```

Collision Channel/Profile, `SM_Body` 피격 정책, CF-FQ-022 조준 정렬, CF-FQ-023 고속 연속 충돌과 최소 Damage Runtime C++ 계약은 구현·빌드 완료 상태다.

---

## 14. Migration

### v0.6.1 -> v0.6.2

```text
- 기능 Build·PIE PASS와 Workflow Compliance FAIL 판정은 변경하지 않는다.
- 누락된 실행 출처를 `Codex 실행 계약`이 아니라 구현 전에 준비됐어야 할 TaskSource와 최종 Codex YAML 작업지시서로 교정했다.
- 브라우저 세션에 Codex 실행기가 연결되어 있지 않았다는 사실은 위반 사유가 아니다.
- 위반 사유는 브라우저 AI가 최종 작업지시서를 생성·전달하지 않고 소스를 직접 수정한 것이다.
- 후속 검토·보정 작업은 브라우저 AI가 TaskSource와 최종 YAML을 생성해 `Ready for External Codex`로 전달하고, 사용자가 별도 Codex 환경에서 실행하는 단계로 분리한다.
- 이 문서 갱신에서는 소스 수정, 빌드, PIE와 외부 Codex 실행을 수행하지 않았다.
```

### v0.6.0 -> v0.6.1

```text
- 최소 Damage Runtime의 기능 판정은 기존 Build PASS, 사용자 PIE PASS, CF-FQ-018 Done과 CF-TC-016 PASS를 유지한다.
- UCFVehicleHealthComp와 Damage Runtime 구현을 허용하는 TaskSource 및 Codex 실행 계약을 찾지 못해 Workflow Compliance FAIL로 분리 기록했다.
- 기존 TaskSource_VisualHitCollision.md는 Health Component와 실제 HP 감소 구현을 금지하므로 Damage Runtime의 실행 출처로 인정하지 않는다.
- 기존 코드를 자동으로 되돌리거나 기능 PASS를 취소하지 않는다.
- 사후 TaskSource로 과거 구현 출처를 Codex로 변경하지 않으며, 필요한 후속 작업은 현재 구현 검토·보정용 새 Codex 작업으로 구분한다.
- 이 문서 갱신에서는 코드, 빌드, PIE와 Codex 실행을 수행하지 않았다.
```

### v0.5.0 -> v0.6.0

```text
- 사용자 싱글 PIE에서 BaseDamage 누적 감소, 체력 0 이하 파괴 전환, 추가 타격 TargetDestroyed 거부와 파괴 이벤트 1회성을 확인했다.
- CF-FQ-018을 Done, CF-TC-016을 PASS로 전환했다.
- 현재 구현 기준을 Document/Systems/Combat/HitDamage.md로 승격했다.
- 이 Plan은 완료 당시 설계와 검증 기록 보존용으로 유지한다.
```

### v0.4.1 -> v0.5.0

```text
- UCFVehicleHealthComp, FCFDamageApplyResult와 VehicleDurabilityConfig가 최소 Damage Runtime 현재 계약으로 추가됐다.
- HitScan과 Projectile은 같은 FCFDamageHitContext와 TryApplyDamageToActor 진입점을 사용한다.
- Projectile은 첫 유효 Impact에서만 피해를 적용하고 Pool 반환 경로에서는 저장된 결과만 전달한다.
- 기존 VehicleData는 MaxHealth=100, DamageData는 BaseDamage=25와 bCanDamageSelf=false C++ 기본값을 사용한다.
- 기존 DataAsset과 WBP/.uasset은 저장하지 않았다.
- 공식 Editor 빌드는 성공했으며 사용자 PIE 전까지 CF-FQ-018은 Active / PARTIAL이다.
```

### v0.4.0 -> v0.4.1

```text
- 상단 현재 작업 체크포인트와 어긋나던 Phase 4~6의 오래된 PIE Pending 표기를 P0 사용자 PIE 완료 상태로 정정했다.
- 선행 검증 목록의 Aim Solution, 정렬 중 발사 정책, 고속 연속 충돌과 중복 Impact 항목을 확인 완료로 정정했다.
- Damage Runtime의 미구현 범위와 바로 다음 작업은 변경하지 않았다.
- 코드, 에셋, 빌드와 PIE를 새로 수행하지 않았다.
```

### v0.3.1 -> v0.4.0

```text
- CF-FQ-022 조준점·터렛·총구 정렬의 공식 빌드와 P0 사용자 PIE 완료를 반영했다.
- Damage Runtime 선행 미완료 목록에서 CF-FQ-022를 제거했다.
- CF-FQ-022와 CF-FQ-023이 모두 Done이므로 CF-FQ-018 최소 Damage Runtime 착수를 허용한다.
- 미확정 범위를 DefaultDamageData, BaseDamage, Health 인터페이스, 파괴 상태와 BP 이벤트로 재정리했다.
- 실제 Damage Runtime 코드는 아직 추가하지 않았다.
```

### v0.3.0 -> v0.3.1

```text
- CF-FQ-023 고속 Projectile 연속 충돌의 공식 빌드와 P0 집중 스트레스 PIE 완료를 반영했다.
- Damage Runtime 선행 미완료 목록에서 CF-FQ-023을 제거했다.
- 남은 직접 선행 조건을 CF-FQ-022 조준 정렬 사용자 PIE로 축소했다.
- 전체 FPS·속도 매트릭스와 이동 차량 검증은 CF-FQ-019 확장 회귀로 유지한다.
- 실제 Damage Runtime 코드는 아직 추가하지 않았다.
```

### v0.2.0 -> v0.3.0

```text
- 문서 상단에 다중 작업 세션 복원용 현재 작업 체크포인트를 추가했다.
- CF-FQ-022와 CF-FQ-023의 코드·공식 빌드 완료 및 사용자 PIE 대기 상태를 반영했다.
- Damage Runtime은 두 선행 작업의 사용자 PIE 검증 전까지 착수하지 않는다.
- 기존 Collision Channel, Profile, 코드와 에셋 동작은 변경하지 않았다.
```

### v0.1.0 -> v0.2.0

```text
- WeaponHit / Projectile / VehicleVisualHit과 CFCollisionChannels.h를 현재 확정 구현으로 사용한다.
- VehicleMesh는 무기 Query Ignore, SM_Body는 QueryOnly 무기 피격 표면으로 유지한다.
- HitComponentName 기록을 DamageHitContext 현재 기준에 포함한다.
- 일반 속도 시각 차체 Hit은 완료 상태로 전환한다.
- 실제 피해 처리는 AimFireAlignment와 ProjectileContinuousCollision 완료 후 진행한다.
- 이 문서 갱신에서는 코드, Config, BP, StaticMesh 자산을 변경하지 않는다.
```

---

## 15. Changelog

### v0.6.2 - 2026-07-16

```text
- 워크플로 위반의 누락 항목을 Codex 실행기가 아니라 TaskSource와 최종 Codex YAML 작업지시서로 교정했다.
- 브라우저 세션에 Codex 실행 도구가 없었던 것은 위반 사유가 아니라고 명시했다.
- 위반 사유를 작업지시서 생성·전달 없이 브라우저 AI가 소스를 직접 수정한 것으로 정확히 기록했다.
- 대표 체크포인트를 준비 방식, 작업지시서 상태, 외부 Codex 실행 상태와 브라우저 검수 상태로 분리했다.
- 후속 검토·보정 작업을 브라우저 작업지시서 생성, 별도 Codex 실행, 브라우저 후속 검수 단계로 재정의했다.
- 기존 기능 Build·PIE PASS와 Systems 승격 상태는 유지했다.
```

### v0.6.1 - 2026-07-16

```text
- 최소 Damage Runtime의 기능 Build·PIE PASS와 워크플로 준수 상태를 분리했다.
- UCFVehicleHealthComp 및 Damage Runtime 구현에 대응하는 TaskSource와 Codex 계약이 없어 Workflow Compliance FAIL로 기록했다.
- TaskSource_VisualHitCollision.md는 Health Component 구현 금지 계약이므로 실행 출처로 사용할 수 없음을 명시했다.
- 대표 체크포인트에 실행 방식, TaskSource, Codex 계약, Codex 실행 결과와 브라우저 검수 출처를 추가했다.
- 기존 기능 PASS와 Systems 승격은 유지하고 후속을 Codex 검토·보정 작업으로 정의했다.
- 코드, 빌드, PIE와 Codex 실행은 새로 수행하지 않았다.
```

### v0.6.0 - 2026-07-15

```text
- 사용자 싱글 PIE 정상 동작 확인을 반영했다.
- 체력 누적 감소, 파괴 상태 전환, 파괴 이후 추가 피해 거부와 이벤트 1회성을 PASS 처리했다.
- CF-FQ-018 Done / CF-TC-016 PASS로 전환했다.
- 현재 구현 문서 Document/Systems/Combat/HitDamage.md를 생성하고 Current System으로 승격했다.
```

### v0.5.0 - 2026-07-14

```text
- 최소 Damage Runtime C++ 구현과 공식 CarFight_ReEditor Win64 Development 빌드 성공을 반영했다.
- VehicleHealthComp, BaseDamage, 자기 피해 정책, 피해 적용 결과와 BP 이벤트 계약을 현재 구현으로 기록했다.
- HitScan/Projectile 공용 적용 경로와 Projectile Pool 중복 차감 방지 구조를 기록했다.
- 사용자 PIE 피해 누적과 파괴 전환은 Pending으로 유지했다.
```

### v0.4.1 - 2026-07-14

```text
- 새 문서구조 기준 세션 복원 과정에서 대표 체크포인트와 본문의 상태 불일치를 정정했다.
- Phase 4~6과 Damage Runtime 선행 검증을 현재 P0 사용자 PIE 완료 기준으로 통일했다.
- 다음 실제 작업은 최소 Damage Runtime 데이터 소유권과 인터페이스 확정으로 유지했다.
- 코드와 에셋은 변경하지 않았다.
```

### v0.4.0 - 2026-07-14

```text
- CF-FQ-022 P0 사용자 PIE 통과와 Done 판정을 반영했다.
- CF-FQ-022와 CF-FQ-023 선행 조건 완료로 CF-FQ-018 Damage Runtime 착수 가능 상태를 기록했다.
- 남은 미구현 범위를 DefaultDamageData, BaseDamage, 체력 감소, 파괴 상태와 BP 이벤트로 정리했다.
- 실제 Damage Runtime 코드는 아직 추가하지 않았다.
```

### v0.3.1 - 2026-07-14

```text
- CF-FQ-023 P0 집중 스트레스 사용자 PIE 통과와 Done 판정을 반영했다.
- HitDamage의 고속 연속 충돌 선행 대기 항목을 완료 처리했다.
- Damage Runtime의 남은 직접 선행 조건을 CF-FQ-022 조준 정렬 PIE로 정리했다.
- 실제 BaseDamage, 체력 감소와 파괴 상태 구현은 아직 시작하지 않았다.
```

### v0.3.0 - 2026-07-14

```text
- 다중 작업 세션 복원을 위한 표준 현재 작업 체크포인트 추가
- CF-FQ-022와 CF-FQ-023의 코드·공식 빌드 완료 및 사용자 PIE Pending 상태 반영
- 오래된 Aim Solution과 고속 Projectile 미구현 표기를 현재 상태로 정정
- Damage Runtime 착수 조건, 다음 작업, 관련 경로와 보호 범위를 명시
```

### v0.2.0 - 2026-07-13

```text
- 시각 차체 기반 WeaponHit / Projectile 충돌 구현과 일반 속도 PIE 확인 결과 반영
- HitComponentName 기록과 확정 Collision Channel/Profile 반영
- Reticle·터렛·Muzzle 방향 불일치와 고속 Projectile 터널링을 Damage Runtime 선행 문제로 추가
- 구현 단계를 시각 피격 완료 → Aim 정렬 → 연속 충돌 → Damage Runtime 순서로 재정렬
- AimFireAlignment / ProjectileContinuousCollision Plan 연결
```

### v0.1.0 - 2026-07-13

```text
- 현재 무기 피격이 VehicleMesh Physics Asset 기준이라는 자산/코드 확인 결과 기록
- SM_Body가 NoCollision이어서 현재 피격 판정에 참여하지 않는 상태 기록
- 무기 피격 형상을 시각 차체 기준으로 전환한다는 구조 결정 기록
- 물리 충돌과 무기 피격 충돌 분리, HitScan/Projectile 전환, 피해 처리 순서 정의
- C++/BP/StaticMesh Asset/Config 책임 분리 정의
- Collision Channel/Profile 이름을 미확정 작업명 후보로 분리
```

---

## 16. 확인 근거

- 확인 일시: 2026-07-13
- 확인 대상:
  - `/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn` 자산 상세 덤프
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
  - `Document/Systems/Combat/DamageHitContext.md`
  - `Document/Systems/Combat/Projectile.md`
