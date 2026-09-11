# Launcher

- Version: 1.0.1
- Date: 2026-09-11
- Status: Current System / CF-FQ-029 P0 Complete / LM-P0-06 Final Technical Integration PASS
- Feature: `CF-FQ-029 모듈형 런처 및 발사 인계`
- Validation: Historical USER PIE + Current UE 5.8 Build/Automation Technical PASS
- Scope: 차량 런처의 Launch Context, 가변 Muzzle, SingleCycle/Ripple/Salvo, Direct/Angled/Vertical Release, Carrier Velocity, MuzzleBlocked, Sequence Failure/Cancel과 Projectile 인계의 현재 구현 기준

---

## 1. 문서 목적

이 문서는 CarFight에서 현재 완료된 **모듈형 런처와 Projectile Launch Handoff**의 Current System을 기록한다.

런처의 책임은 발사 전 준비부터 Projectile이 발사관에서 분리되는 순간까지다. 분리된 뒤 미사일의 비행 상태와 유도는 별도 Current System이 소유한다.

```text
Launcher
→ 발사관 선택
→ 발사 패턴 스케줄
→ 실제 사출 위치·방향·초기 속도 결정
→ Launch Context 복사
→ Projectile Pool 활성화
→ 이동 책임 종료

Projectile / Missile
→ Launch Context 소비
→ 런처와 독립 이동
→ 추진·Flight·Guidance·충돌·피해·Pool 수명 처리
```

상세 구현 이력과 과거 USER PIE evidence는 Historical Plan이 보존한다.

```text
Document/Plan/LauncherMissile/LauncherMissilePlan.md v0.15.0
```

해당 Plan은 2026-09-11 `v0.15.0`으로 직접 갱신되어 Done / Historical + Retained Path와 최종 closure evidence를 보존한다. 현재 구현 판단은 실제 Source와 이 Systems 문서를 우선하고, Plan은 완료 당시 상세 evidence와 검증 이력을 제공한다.

---

## 2. 현재 P0 완료 범위

현재 CF-FQ-029 P0 완료 범위는 다음과 같다.

```text
- FCFProjectileLaunchContext 기반 발사 순간 값 복사
- 기존 Direct Projectile 호환 경로
- 가변 Muzzle 배열과 결정론적 발사관 순환
- SingleCycle
- Ripple
- Salvo
- 첫 입력 순간 Command Target 고정
- 진행 중 중복 입력 방지
- SequenceCompleted 쿨다운
- ContinueRemaining 실패 정책
- StopSequence 실패 정책
- Manual Cancel terminal cleanup
- Direct Release
- AngledEjection
- VerticalEjection
- EjectionSpeed
- CarrierVelocityRatio 기반 차량 월드 속도 상속
- 실제 InitialLaunchDirection 기준 비Direct MuzzleBlocked
- 발사 뒤 런처와 Projectile 독립
- 같은 차량 Projectile의 상호 충돌 격리
- Projectile Pool LaunchContext 값 전달과 재사용 경계
```

CF-FQ-029는 2026-09-11 최종 기술 통합 검증으로 Done 처리한다.

---

## 3. 현재 책임 구조

### 3.1 WeaponData

`UCFWeaponData`는 런처의 정적 설정을 제공한다.

```text
LauncherFirePatternConfig
LauncherReleaseConfig
DefaultProjectileData
```

현재 런처 Runtime은 저장된 WeaponData의 설정을 읽되 Product DataAsset을 자동 수정하거나 저장하지 않는다.

### 3.2 VehicleWeaponComp

`UCFVehicleWeaponComp`는 활성 장비/무기 해석, Muzzle 선택과 Sequence 상태를 제공한다.

주요 의미:

```text
- 현재 활성 WeaponData 해석
- Muzzle 이름/인덱스/개수 결정
- SingleCycle 순환
- Ripple/Salvo Scheduler 입력
- Launcher Fire Pattern과 Release Config 제공
- Sequence 실행 중 WeaponChanged 취소 경계
```

### 3.3 VehicleFireComp

`UCFVehicleFireComp`는 현재 실제 Launch Context 작성과 발사 실행 경계를 소유한다.

현재 `BuildDirectProjectileLaunchContext()`는 다음 값을 발사 순간 값으로 복사한다.

```text
LaunchLocation
InitialLaunchDirection
InitialLaunchVelocity
InheritedCarrierVelocity
CommandTargetLocation
GuidanceTargetActor
GuidanceTargetLocation
MuzzleSocketName
MuzzleIndex
MuzzleCount
ReleaseMode
FireRequestId
WeaponGroupId
```

발사 뒤 차량 속도, 터렛 방향과 현재 선택 Target이 기존 Projectile의 초기 상태를 다시 덮어쓰지 않는다.

### 3.4 ProjectilePool / ProjectileActor

`UCFProjectilePoolComp`와 `ACFProjectileActor`는 Launch Context를 값으로 받아 Projectile을 활성화한다.

```text
VehicleFireComp
→ AcquireProjectileWithContext
→ ACFProjectileActor Activate
→ Launch Context 값 복사
→ 런처와 독립된 이동/비행 수명
```

공통 Projectile 이동·충돌·Pool 세부 계약은 `Document/Systems/Combat/Projectile.md`가 소유한다.

---

## 4. 발사 패턴

### 4.1 SingleCycle

입력 한 번에 한 발을 승인하고, 성공한 발사 뒤 다음 Muzzle로 순환한다.

검증된 대표 순서:

```text
Muzzle 1 → 4 → 2 → 3 → 1
```

발사가 거부되면 Projectile과 FX를 만들지 않고 다음 Muzzle 인덱스로 진행하지 않는다.

### 4.2 Ripple

여러 Projectile을 지정 간격으로 순차 발사한다.

대표 검증 설정:

```text
ProjectileCountPerTrigger = 4
InterMuzzleDelaySeconds = 0.15
MaximumSimultaneousLaunchCount = 1
SequenceFailurePolicy = ContinueRemaining
CooldownStartPolicy = SequenceCompleted
```

첫 입력 순간 Command Target을 Sequence 전체에 고정하고, 진행 중 추가 입력은 두 번째 Sequence를 중복 시작하지 않는다.

### 4.3 Salvo

여러 Muzzle에서 같은 Trigger의 Projectile을 동시에 또는 같은 프레임 수준으로 사출한다.

Historical USER PIE에서 네 Muzzle 동시 사출과 같은 차량 Salvo Projectile끼리 사출 직후 상호 Impact가 발생하지 않는 것을 확인했다.

---

## 5. Release 계약

### 5.1 Direct

```text
InitialLaunchDirection = Command Aim Direction
```

기존 직사 Projectile과 비유도 Rocket 호환 기본값이다.

### 5.2 AngledEjection

`LocalEjectionDirection`을 실제 Muzzle Transform 기준 월드 방향으로 변환해 초기 사출 방향으로 사용한다.

```text
InitialLaunchDirection
= Normalize(MuzzleTransform.TransformVectorNoScale(LocalEjectionDirection))
```

Command Target 방향과 초기 사출 방향을 분리하므로, 발사 직후 Projectile을 목표 방향으로 순간 회전시키지 않는다.

### 5.3 VerticalEjection

현재 계약에서는 선택 Muzzle의 로컬 `+X` 방향을 초기 사출 방향으로 사용한다.

Command Target은 별도 값으로 유지된다.

### 5.4 초기 속도와 Carrier Velocity

비Direct Release의 초기 월드 속도는 다음 의미를 가진다.

```text
InheritedCarrierVelocity
= VehicleWorldVelocity × CarrierVelocityRatio

InitialLaunchVelocity
= InitialLaunchDirection × EjectionSpeed
+ InheritedCarrierVelocity
```

상속은 발사 순간 Snapshot이다. 발사 후 차량의 추가 가속·회전은 이미 발사된 Projectile에 계속 전달되지 않는다.

---

## 6. MuzzleBlocked 안전 계약

비Direct Release는 Command Target 방향이 아니라 **실제 InitialLaunchDirection** 앞쪽을 검사한다.

```text
TraceStart = LaunchLocation
TraceEnd = LaunchLocation
         + InitialLaunchDirection
         × LauncherClearanceTraceDistanceCm
```

현재 규칙:

```text
- 일반 Blocking Actor가 실제 사출 안전 구간을 막으면 MuzzleBlocked
- 발사 거부 시 Ammo transaction rollback
- Dummy HitScan으로 우회하지 않음
- Projectile Pool 활성 수 증가 없음
- 장애물 제거 뒤 같은 설정으로 정상 발사 가능
- VehicleHealthComp를 가진 유효 피해 차량은 기존 WeaponFire 안전 정책에 따라 별도 처리
```

`LauncherClearanceTraceDistanceCm <= 0`이면 비Direct Release 전용 clearance trace를 비활성화한다.

---

## 7. Sequence 실패와 취소

현재 Scheduler 실패 정책은 다음 두 가지다.

```text
ContinueRemaining
- 한 Shot 실패를 누계
- Sequence Active 유지
- 남은 Dispatch 계속
- 마지막에 Completed

StopSequence
- 후속 Shot 실패 즉시 Cancelled / ShotFailed
- 이후 Dispatch 차단
```

Manual Cancel은 남은 Ammo Reservation과 Weapon Action Lock을 정리한다.

OwnerDestroyed·WeaponChanged·TurretMountChanged 같은 실제 gameplay lifecycle 취소는 해당 상태 전환이 존재하는 소비 경로에서 별도 회귀한다. 이것은 현재 CF-FQ-029 P0 완료를 막지 않는다.

---

## 8. Projectile 충돌 격리 경계

같은 차량이 발사한 Projectile은 탄종, Actor Class, Volley와 FireRequest가 달라도 같은 `ActiveInstigatorActor`를 기준으로 상호 충돌을 무시한다.

```text
같은 ActiveInstigatorActor
→ Projectile 상호 Ignore

다른 ActiveInstigatorActor
→ Projectile 채널 기본 Block
→ bCanBeIntercepted=true이면 요격 가능
```

Historical USER PIE에서 같은 차량 Salvo의 상호 Impact 없음과 차량·월드 일반 충돌 유지를 확인했다.

다른 차량 Projectile/Hitscan 실제 요격, 다른 탄종/Volley 교차 격리와 Pool Ignore Reset의 다중 사격 주체 회귀는 AI/복수 무기 전투 환경이 준비된 뒤 별도 회귀한다. 현재 Launcher P0 기능 차단 항목은 아니다.

---

## 9. 저장 Product 기준선

2026-09-11 fresh AssetDump에서 저장된 `DA_RocketLauncher`는 다음 기본 Release 상태를 유지했다.

```text
ReleaseMode = Direct
EjectionSpeed = 0
CarrierVelocityRatio = 0
LauncherClearanceTraceDistanceCm = 150
LocalEjectionDirection = (1, 0, 1)
```

현재 저장 Pattern은 Salvo 4발 구성이다. 최종 검증은 Product Asset을 임시 수정/저장하지 않고 transient Runtime data로 Angled/Vertical/Carrier 조건을 검증했다.

따라서 테스트 뒤 Product 기본값 복구를 위해 `.uasset`을 다시 저장할 필요가 없다.

---

## 10. 최종 검증 기준

### 10.1 Historical USER PIE 보존

기존 USER PIE에서 다음을 이미 확인했다.

```text
- Player Launcher 진입
- Direct Launch Handoff
- 기존 Rocket 추진
- 발사 후 런처 독립
- Pool 5 Volley 이상 재사용
- Ripple 4발 / 0.15초
- Muzzle 1→4→2→3
- Command Target 고정
- 중복 입력 방지
- SequenceCompleted
- SingleCycle
- 거부 시 Muzzle 인덱스 유지
- Salvo 4 Muzzle 동시 사출
- 같은 차량 Salvo 상호 Impact 없음
- 차량·월드 일반 충돌 유지
```

새 failure evidence가 없으므로 이 USER evidence를 반복하지 않는다.

### 10.2 2026-09-11 Current Technical Closure

현재 Product 실행 경로의 남은 기술 invariant를 transient World 기반 Automation으로 닫았다.

```text
Official UE 5.8 Editor Build
- Job: 740913a4673a4d028bae8c9b8a945f3c
- PASS

LM-P0-06 Final Integration
- CarFight.Launcher.LM_P0_06.FinalIntegration
- Process: ca83a385322a4e8f9d92ec0c54c9f2aa
- 1/1 PASS

Launcher affected regression
- Process: 845ca421e44844a2868c5ee5f5ba6beb
- 5/5 PASS
- Failure 0
```

Final Integration은 실제 Current Product 경로인 다음 연결을 실행한다.

```text
VehiclePawn
→ VehicleWeaponComp
→ VehicleFireComp
→ BuildDirectProjectileLaunchContext
→ MuzzleBlocked Trace
→ ProjectilePoolComp
→ ProjectileActor Active LaunchContext
```

검증 내용:

```text
- Angled ReleaseMode/방향
- EjectionSpeed
- 실제 Vehicle GetVelocity 기반 CarrierVelocityRatio
- InitialLaunchVelocity 합성
- 실제 InitialLaunchDirection MuzzleBlocked
- Blocked 발사 시 Pool mutation 0
- 장애물 제거 후 정상 발사
- ProjectileActor의 LaunchContext 값 복사
- Vertical Release
- Pool 반환
```

현재 검증 정책에서 이 항목들은 시각·조작감·체감이 아니라 **기술 상태 전이와 데이터 전달 invariant**다. 따라서 USER 시각/체감 판정으로 남길 항목이 아니며 Current Technical PASS로 완료할 수 있다.

자동 PIE lifecycle 진입은 당일 GoPyMCP의 `Managed UE Bridge interpreter identity mismatch`로 PIE 시작 전에 차단됐지만, Product failure evidence는 아니며 위 Current Product-path Automation이 남은 기술 계약을 직접 검증했다.

---

## 11. 관련 Current Systems

```text
Document/Systems/Combat/WeaponData.md
- Launcher 정적 설정 owner

Document/Systems/Combat/WeaponFire.md
- 일반 Fire validation/result와 Aim/Ammo/Heat 통합 경계

Document/Systems/Combat/Ammo.md
- Launcher Sequence용 Loaded/Reserve/Reservation/Reload owner

Document/Systems/Combat/Projectile.md
- Projectile Actor/Movement/Collision/Pool 생명주기 owner

Document/Systems/Combat/MissileGuidance.md
- Launcher 분리 뒤 Missile Flight/Guidance owner
```

---

## 12. 현재 비책임 / 후속 범위

다음은 CF-FQ-029 P0 완료를 막지 않는다.

```text
- 다른 차량 Projectile/Hitscan 실제 요격 전투 회귀
- 복수 탄종·Volley 교차 격리 다중 사격 주체 회귀
- Pool Ignore Reset 다중 사격 주체 회귀
- OwnerDestroyed 실제 상태 전환 취소 회귀
- WeaponChanged 실제 플레이 입력 취소 회귀
- TurretMountChanged 실제 상태 전환 취소 회귀
- 장비별 특수 Salvo 시각 연출
- 네트워크 권한 Launcher
```

새 failure evidence가 발견되면 관련 Current System과 실제 Source를 기준으로 별도 회귀/Feature를 연다.

---

## 13. Changelog

### v1.0.1 - 2026-09-11

- representative Historical Plan을 `LauncherMissilePlan.md v0.15.0`으로 동기화했다.
- `plan_repo policy.read_only`를 Plan 텍스트 수정 불가로 해석했던 잘못된 설명을 제거했다. Plan 텍스트 write는 정상 수행됐고 v0.15.0이 Done / Historical + Retained Path를 직접 기록한다.
- Launcher runtime 계약과 기존 Build/Automation/Asset evidence는 변경하지 않았다.

### v1.0.0 - 2026-09-11

- `CF-FQ-029` 모듈형 런처 및 발사 인계를 Current System으로 최초 승격했다.
- 기존 SingleCycle/Ripple/Salvo와 USER PIE evidence를 보존하고, 남아 있던 Angled/Vertical Release, Carrier Velocity와 실제 InitialLaunchDirection MuzzleBlocked를 Product-path transient Automation으로 닫았다.
- Official UE 5.8 Build PASS, LM-P0-06 Final Integration 1/1, 전체 `CarFight.Launcher` 5/5 PASS를 closure evidence로 기록했다.
- fresh AssetDump에서 `DA_RocketLauncher`의 Direct / EjectionSpeed 0 / CarrierVelocityRatio 0 저장 기본값을 확인했다.
- Product runtime Source와 Product DataAsset mutation은 0이며 신규 Source 변경은 test-only `CFLauncherFinalTests.cpp v1.0.0`이다.
- 다른 차량 요격, 다중 사격 주체 cross-type/Pool reset과 lifecycle cancel은 비차단 후속 회귀로 유지했다.

---

## 14. Migration

```text
- CF-FQ-029 완료 이후 Launcher 현재 구현 판단은 이 문서와 실제 Source를 우선한다.
- LauncherMissilePlan.md v0.15.0은 완료 당시 상세 checkpoint와 최종 closure evidence를 보존하는 Historical + Retained Path로 읽는다.
- v0.14.0의 `USER PIE Pending / CF-FQ-029 Not Done`은 2026-08-16 당시 Historical checkpoint이며 현재 lifecycle을 대체하지 않는다.
- Direct 기존 WeaponData는 Release 기본값으로 호환된다.
- Angled/Vertical은 WeaponData가 명시적으로 해당 ReleaseMode와 유효한 Ejection 값을 제공할 때만 활성화한다.
- Missile Flight/Guidance는 Launcher가 아니라 MissileGuidance Current System이 계속 소유한다.
```
