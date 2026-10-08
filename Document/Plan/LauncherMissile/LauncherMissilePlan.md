# Launcher / Missile Implementation Plan

- Version: 0.15.0
- Date: 2026-09-11
- Status: Done / Historical + Retained Path / LM-P0-06 Final Technical Integration PASS / Current System Promotion Complete
- Feature: `CF-FQ-029` 모듈형 런처 및 발사 인계
- Dependent Feature: `CF-FQ-030` 물리 제한형 미사일 비행·유도
- Planned Tests: `CF-TC-025`, `CF-TC-026`, `CF-TC-027`
- Representative Plan: `Document/Plan/LauncherMissile/LauncherMissilePlan.md`

---

## 1. 목적

CarFight의 차량 무기 체계에 다음 두 기능을 서로 분리된 책임으로 구현한다.

```text
CF-FQ-029
= 가변 발사구, Single / Ripple / Salvo, Direct / Angled / Vertical 사출과
  발사 순간 초기 조건 전달을 담당하는 모듈형 런처 시스템

CF-FQ-030
= 런처에서 분리된 뒤 자체 추진·비행 전환·물리 제한형 유도·충돌을
  독립적으로 처리하는 미사일 시스템
```

두 기능은 `Projectile Launch Handoff` 계약으로 연결한다.

```text
터렛·런처
→ 발사관 선택
→ 사출 위치·방향·초기 속도 결정
→ Launch Context 전달
→ 발사체 이동 책임 종료

Projectile / Missile
→ Launch Context 복사
→ 런처와 독립 이동
→ 점화·추진·전환·유도·충돌 처리
```

---

## 2. 현재 작업 체크포인트

| 항목 | 상태 |
|---|---|
| 사용자 요구사항·책임 분리·로드맵 | Done |
| LM-P0-01 Launch Handoff | Code Applied / Build PASS |
| LM-P0-02 가변 Muzzle·SingleCycle | Code Applied / Build PASS / Editor Validation Pending |
| LM-P0-03A Pattern Data Contract | Code Applied / Build PASS |
| LM-P0-03B Ripple·Salvo Runtime Scheduler | Code Applied / Build PASS / Editor Validation Pending |
| Scheduler 상태·실패·취소·쿨다운 계약 | Applied |
| 첫 입력 순간 Command Target 고정·터렛 추적 | Applied |
| Scheduler RuntimeContract Automation | PASS — `CarFight.Launcher.LM_P0_03B.SchedulerContract` / final closure Process `69db5494a03f483e9d8f6d576dd5742c` / 2026-08-16 1/1 |
| LM-P0-04 Angled·Vertical Ejection | Code Applied / Build PASS |
| Release RuntimeContract Automation | PASS — `CarFight.Launcher.LM_P0_04.ReleaseContract` / 2026-08-02 최신 로컬 증거 |
| 최신 공식 Editor 빌드 | PASS / LM-P0-06A final closure Job 6a633eb4cf21481d8ae26ec908d05660 / Exit Code 0 |
| LM-P0-05 Launcher Editor Assets | Assets Applied / Independent AssetDump PASS / Player Entry Connected |
| CF-TC-025 Launch Handoff Regression | Historical PARTIAL / Direct Handoff·Rocket·Pool PASS / Damage 보강 미완료는 Launcher 외부 비차단 범위 |
| CF-TC-026 Modular Launcher·Ejection | Launcher P0 Acceptance Complete / Direct Ripple·SingleCycle·Salvo·동일 차량 Salvo 격리·일반 충돌 USER evidence 보존 / Failure Policy·Manual Cancel Technical PASS / Angled·Vertical·CarrierVelocity·MuzzleBlocked Product-path Technical PASS / 다른 차량 요격·복수 탄종 교차 격리 Deferred |
| LM-P0-06A Failure Policy Technical Closure | Technical PASS — SchedulerContract 1/1 / Launcher 4/4 / Ammo LauncherLock 1/1 / 반복 금지 |
| LM-P0-06 Final Technical Integration | PASS — `CarFight.Launcher.LM_P0_06.FinalIntegration` 1/1 / 전체 `CarFight.Launcher` 5/5 / Official UE 5.8 Build PASS |
| CF-FQ-029 Systems 승격 | Complete — `Document/Systems/Combat/Launcher.md v1.0.1` |

현재 코드 진입점:

```text
Task: CF-FQ-029 Closure Complete / Historical Retained Path
선행 완료: LM-P0-01~04 Code Applied·Build PASS / LM-P0-05 Assets Applied·Independent AssetDump PASS
Player Entry: BP_CFVehiclePawn CDO → DA_TestSUV → RocketLauncher
Map Targets: BP_CFVehiclePawn_C_0 Sedan·HeavyCannon 유지 / BP_CFVehiclePawn_C_1 SUV·RocketLauncher
보호 범위: 기존 Direct Projectile, Fixed Rocket 추진, Damage, FX, Pool, TargetSelect 회귀 유지
사용자 PIE 완료: 플레이어 Launcher 진입 / Direct Handoff / 기존 Rocket 추진 / 발사 후 런처 독립 / Pool 5 Volley 이상 / Direct Ripple 4발·0.15초 / Muzzle 1→4→2→3 / Command Target 고정 / 중복 입력 방지 / SequenceCompleted / SingleCycle / 거부 시 Muzzle 인덱스 유지 / Salvo Launcher 4 Muzzle 동시 사출 / 동일 차량 Salvo 상호 Impact 없음 / 차량·월드 대상 일반 충돌 유지
확정 충돌 정책: 같은 차량이 발사한 모든 탄종·모든 Volley는 ActiveInstigatorActor 기준 상호 Ignore / 다른 차량 Projectile은 기본 Block·요격 가능
적용 수정: ACFProjectileActor v1.11.0 / UCFProjectilePoolComp v1.5.0 / UCFProjectileData v1.9.0 / ACFVehiclePawn v2.128.0 / CFProjectileCollisionTests v2.0.0
공식 빌드: 940272869b77450797347f76b427faf3 / Exit Code 0
별도 차단: Damage 보강 미완료로 차량 Damage 적용 검증 불가 / 런처 실패로 판정하지 않음
Deferred 검증: 다른 차량 Projectile·Hitscan 요격 / 다른 Volley·다른 탄종 교차 격리 / Pool Ignore Reset — AI 또는 복수 무기·사격 주체 환경에서 확인
2026-09-11 Final Technical Integration: 실제 사출 방향 MuzzleBlocked / Angled·Vertical / Carrier Velocity / LaunchContext→ProjectilePool→ProjectileActor 전달 PASS
Failure Policy 기술 검증: `FCFLauncherSequenceRuntime::RecordShotResult(false)`의 기존 구현을 변경하지 않고 ContinueRemaining 실패 누계·Active 유지·후속 Dispatch·최종 Completed와 StopSequence 즉시 Cancelled/ShotFailed·추가 Dispatch 차단을 asset-free `SchedulerContract`에서 PASS했다.
Manual Cancel 보호 회귀: `CFAmmoLauncherTests.cpp`의 `CancelFireSequence(Manual)`이 실제 LauncherComp + Ammo Reservation 0 + ActionLock 해제를 유지함을 `CarFight.Ammo.AMMO_P0_04.LauncherLock` 1/1 PASS로 재확인했다. 플레이어 입력/UI 호출 경로는 추가하지 않았다.
Deferred 취소: OwnerDestroyed·WeaponChanged·TurretMountChanged는 해당 gameplay 상태 전환 수단이 준비된 후 별도 검증한다.
Current owner: `Document/Systems/Combat/Launcher.md v1.0.1`
Product Asset mutation: 0 / Product runtime Source mutation: 0 / test-only Source: `CFLauncherFinalTests.cpp v1.0.0`
```

---

## 3. 문서 구성

| 문서 | 역할 |
|---|---|
| `LauncherMissilePlan.md` | 대표 Plan, 현재 체크포인트와 다음 작업 |
| `LauncherMissileDesign.md` | 전체 아키텍처, 책임 경계와 데이터 계약 |
| `LauncherMissileRoadmap.md` | 단계별 개발 순서와 선행 관계 |
| `ModularLauncherPlan.md` | 다연장 런처, 발사 패턴과 사출 방식 상세 계획 |
| `MissileGuidancePlan.md` | 미사일 비행 상태, 추진·전환·유도 상세 계획 |
| `LaunchTaskSource.md` | 첫 코드 작업의 근거·범위·보호 계약 |
| `LaunchWorkOrder.md` | 첫 코드 작업의 파일별 실행 순서와 검증 절차 |

---

## 4. 확정된 최상위 결정

### 4.1 런처와 미사일을 별도 기능으로 개발한다

```text
런처
= 발사 전과 분리 순간까지

미사일
= 분리 순간 이후
```

`CF-FQ-029`가 먼저 공통 Launch Handoff와 런처 기반을 제공한다.
`CF-FQ-030`은 이 계약 위에서 구현한다.

### 4.2 목표와 초기 발사 방향을 분리한다

현재 직사 발사의 다음 전제는 확장 전에 해제한다.

```text
AimDirection == InitialLaunchDirection
```

확장 후에는 최소한 다음 값을 독립적으로 보존한다.

```text
CommandTargetLocation
InitialLaunchDirection
InitialLaunchVelocity
InheritedCarrierVelocity
GuidanceTargetActor
GuidanceTargetLocation
```

### 4.3 미사일은 발사 즉시 런처와 독립한다

발사 후 금지되는 의존:

```text
- 매 프레임 터렛 Transform 복사
- 차량의 현재 선택 대상 변경을 기존 미사일에 반영
- 런처 방향으로 미사일 Velocity 덮어쓰기
- 런처가 미사일 위치를 직접 이동
```

허용되는 외부 정보:

```text
- 발사 순간 복사한 대상 참조
- 레이저 조사점
- 후속 DataLink 목표 정보
```

외부 정보는 이동력이 아니라 유도 입력이다.

### 4.4 Ejection은 Chaos 강체가 아닌 ProjectileMovement를 기본으로 한다

```text
초기 사출 속도
+ 차량 속도 상속
+ 중력
+ Sweep / Sub-step
```

미사일 본체에는 기본적으로 `Simulate Physics`를 켜지 않는다.
Chaos Physics는 발사관 덮개, 캐니스터, 분리 부품 또는 별도 특수 탄종에만 선택적으로 사용한다.

### 4.5 유도는 명중을 보장하지 않는다

```text
- 위치 순간이동 금지
- 목표 방향으로 Velocity 즉시 교체 금지
- 최대 선회율 제한
- 최대 횡가속도 제한
- 탐색기 시야 제한
- 목표 상실과 오버슈트 허용
- 실제 충돌만 명중 처리
```

### 4.6 Projectile 충돌 예외는 발사 차량 단위로 판정한다

```text
같은 ActiveInstigatorActor
→ 탄종, Actor Class, Volley, FireRequest와 무관하게 상호 충돌 Ignore

다른 ActiveInstigatorActor
→ Projectile 채널 기본 Block
→ bCanBeIntercepted=true이면 Projectile 또는 Hitscan 적중 한 번으로 Intercepted
```

팀, `FireRequestId`, `WeaponGroupId`와 `ProjectileData`는 동일 발사자 판정 기준으로 사용하지 않는다. 같은 차량의 모든 무기 발사체는 하나의 `UCFProjectilePoolComp`가 모든 Actor Class 버킷을 가로질러 격리한다.

Pool 반환 시 양방향 Ignore 관계를 해제하고, 재활성화에서는 현재 Source 기준으로 다시 등록한다. Hitscan은 자기 차량의 활성 Projectile을 Trace Query에서 제외하고 다른 차량 Projectile만 요격 대상으로 본다.

P0 요격은 Projectile 내구도 없이 유효 적중 한 번으로 종료한다. `bDetonateWhenIntercepted`는 종료 여부가 아니라 요격 위치의 기본 Impact FX 요청 여부만 결정한다.

---

## 5. 기능 분할

### 5.1 `CF-FQ-029` 모듈형 런처 및 발사 인계

범위:

```text
- Launch Context
- 기존 직사 Projectile 호환 어댑터
- 가변 Muzzle 배열
- 발사관 순환
- Single / Ripple / Salvo
- Direct / Angled / Vertical Release
- Ejection 속도와 차량 속도 상속
- 발사관 안전 검사
- 런처 Runtime Debug
```

완료 후 Systems 후보:

```text
Document/Systems/Combat/Launcher.md
Document/Systems/Combat/WeaponFire.md 갱신
Document/Systems/Combat/Projectile.md 갱신
```

### 5.2 `CF-FQ-030` 물리 제한형 미사일 비행·유도

범위:

```text
- Missile Flight State
- Ejection / Clearance / Ignition / Transition / GuidedFlight
- 추진 방향 모드
- 제한된 횡가속도·선회율
- Target Actor 추적
- Laser Point 추적
- 목표 상실 정책
- Direct / Loft / Pitch-Over / Top-Attack Profile
- Guidance Debug
```

완료 후 Systems 후보:

```text
Document/Systems/Combat/Missile.md
Document/Systems/Combat/Projectile.md 갱신
Document/Systems/Targeting/TargetSelect.md 연관 계약 갱신
```

---

## 6. 첫 코드 작업 범위

첫 코드 작업은 다연장 발사나 유도를 구현하지 않는다.

```text
LM-P0-01
= 기존 Direct Projectile 동작을 그대로 보존하면서
  발사 목표와 초기 발사 상태를 분리해 전달할 Launch Context 기반 추가
```

첫 코드 단계 완료 조건:

```text
- 신규 Launch Context 타입이 존재한다.
- 기존 단일 Muzzle 직사 발사는 같은 위치·방향·속도로 동작한다.
- ProjectilePool과 ProjectileActor가 Launch Context를 받을 수 있다.
- 기존 함수는 호환 어댑터 또는 안전한 마이그레이션 경로를 유지한다.
- Multi-Muzzle, Ripple, Salvo, Guidance는 아직 동작하지 않는다.
- 공식 Editor 빌드와 RuntimeContract Automation을 통과한다.
```

---

## 7. 보호 범위

```text
- 기존 Projectile 첫 Impact 단일 처리 순서를 변경하지 않는다.
- DamageHitContext와 VehicleHealth 적용 순서를 변경하지 않는다.
- Projectile Pool 반환 정책을 재작성하지 않는다.
- 비유도 Rocket의 Fixed Launch Direction 추진을 첫 Task에서 변경하지 않는다.
- TargetSelect의 선택 상태 소유권을 변경하지 않는다.
- 기존 Heavy Cannon과 단일 Muzzle 에셋을 재저장하지 않는다.
- 사용자 미커밋 .uasset과 unrelated 문서를 수정하지 않는다.
- 게임 Audio를 추가하지 않는다.
- commit / push / reset / checkout / stash를 수행하지 않는다.
```

---

## 8. 코드 진입 판정

```text
Implementation Source: Browser AI Direct
Documentation: Complete
TaskSource: Ready
WorkOrder: Ready
Source Changes: Applied — Launch Context / Pool·Actor Handoff / Legacy Direct Adapter / 가변 Muzzle·SingleCycle / Ripple·Salvo Scheduler / Release Config / 차량 속도 상속 / 실제 사출 방향 안전 검사 / RuntimeContract Sources
Diff Review: Scoped Git Diff Reviewed
Build: PASS — LM-P0-01 Job 056397a4b43d49c9845c9cc296b23b9c / LM-P0-02 Job 409b7e9a4a9b49978fc5fe83ae29cb41 / LM-P0-03B Job 58761c19b152486ba783baf57526f9e5 / LM-P0-04 Final Job acd575ffac2f4d64bd73194e160a3f62 / LM-P0-06A closure Job 6a633eb4cf21481d8ae26ec908d05660 / Final closure Job 740913a4673a4d028bae8c9b8a945f3c / Exit Code 0
Automation: PASS — LM-P0-06A SchedulerContract 1/1 / Launcher 4/4 / Ammo LauncherLock 1/1 / 2026-09-11 `CarFight.Launcher.LM_P0_06.FinalIntegration` 1/1 / final `CarFight.Launcher` 5/5 / 공용 Product runtime Source 변경 0
Assets: PASS — DA_RocketBody / DA_RocketLauncher / RocketLauncher Preset / DA_TestSUV / TestMap / BP_CFVehiclePawn CDO / fresh AssetDump에서 `DA_RocketLauncher` Direct·EjectionSpeed 0·CarrierVelocityRatio 0 저장 기본값 확인
Independent Verification: PASS — LauncherPost 10/10 / VehiclePost 3/3 / MapPost 19/19 / 신규 Unreal 프로세스 CDO 재검증
PIE: Historical USER evidence preserved — Direct Handoff·Ripple·SingleCycle·Salvo Launcher·Same-Source Salvo Isolation·일반 충돌 유지 PASS. 2026-09-11 남은 순수 기술 invariant인 MuzzleBlocked·Angled/Vertical·Carrier Velocity는 Product-path Automation으로 PASS. Interception·복수 탄종 교차 격리·lifecycle cancel은 비차단 Deferred.
Verification Gate: COMPLETE — CF-FQ-029 Done / Systems Promotion Complete
```

코드 착수 전 다음 세션은 아래를 재확인한다.

```text
1. main_game Git 상태
2. AGENTS.md
3. Document/CodeWorkGate.md
4. LauncherMissilePlan.md
5. LauncherMissileDesign.md
6. LaunchTaskSource.md
7. LaunchWorkOrder.md
8. 관련 실제 코드의 현재 버전과 미커밋 중첩
```

---

## 9. 최종 Closure

### LM-P0-06A Failure Policy Technical Closure — Historical Technical PASS

```text
Runtime Source: 변경 0 — FCFLauncherSequenceRuntime::RecordShotResult(false) 기존 구현 유지
Test Source: CFLauncherSchedTests.cpp v1.2.0 — StopSequence terminal 이후 GetDispatchBudget=0 / MarkShotDispatched=false 보호 assert 추가
ContinueRemaining: 실패 누계 + Active 유지 + 후속 Dispatch + 최종 Completed PASS
StopSequence: 첫 후속 실패 → Cancelled / ShotFailed + 이후 Dispatch 차단 PASS
Manual Cancel: CancelFireSequence(Manual) → 남은 Ammo Reservation 0 / ActionLock 해제 PASS
Official Editor Build: 6a633eb4cf21481d8ae26ec908d05660 / Exit Code 0
Targeted Automation: SchedulerContract `69db5494a03f483e9d8f6d576dd5742c` 1/1 PASS / CarFight.Launcher `7ab7a286e5484cab845bb0cbfd5ac04e` 4/4 PASS / AMMO_P0_04.LauncherLock `1781accd3d9c47a59421fb1d2228e4ab` 1/1 PASS
Content Asset / Blueprint / USER PIE: 변경·실행 0
Closure Scope: Failure Policy + Manual Cancel 기술 검증만 완료. CF-FQ-029 전체 Done과 LM-P0-06 USER PASS로 확대하지 않음
```

### LM-P0-06 Final Technical Integration — PASS

```text
Historical USER evidence preserved:
- Direct Launch Handoff / Ripple / SingleCycle
- Salvo 4 Muzzle 동시 사출
- 동일 차량 Salvo 상호 Impact 없음
- 차량·월드 대상 일반 충돌 유지

2026-09-11 Current Product-path Technical Closure:
- MuzzleBlocked 실제 InitialLaunchDirection trace PASS
- AngledEjection PASS
- VerticalEjection PASS
- CarrierVelocityRatio / Vehicle GetVelocity snapshot PASS
- LaunchContext → ProjectilePool → ProjectileActor 전달 PASS
- Blocked 발사 시 Pool mutation 0 PASS
- 장애물 제거 후 정상 발사 PASS
- Official UE 5.8 Build Job `740913a4673a4d028bae8c9b8a945f3c` PASS
- `CarFight.Launcher.LM_P0_06.FinalIntegration` 1/1 PASS
- 전체 `CarFight.Launcher` 5/5 PASS / Failure 0
- fresh AssetDump 저장 기본값 Direct / EjectionSpeed 0 / CarrierVelocityRatio 0 확인
- Product `.uasset` mutation 0
- Product runtime Source mutation 0

Current System Promotion:
- `Document/Systems/Combat/Launcher.md v1.0.1`
- `CF-FQ-029` = Done

Non-blocking Deferred:
- 다른 차량 Projectile·Hitscan 실제 요격
- 다른 Volley·다른 탄종 교차 격리
- Pool Ignore Reset 다중 사격 주체 회귀
- OwnerDestroyed·WeaponChanged·TurretMountChanged 실제 상태 전환 취소 회귀
```

---

## 10. Changelog

### v0.15.0 - 2026-09-11

- `CF-FQ-029 / LM-P0-06 Final Technical Integration`을 PASS로 닫고 Feature를 Done / Historical + Retained Path로 전환했다.
- 기존 USER PIE의 Direct/Ripple/SingleCycle/Salvo, Muzzle 순환, Command Target 고정, 동일 차량 Salvo 격리와 일반 충돌 evidence는 반복하지 않고 보존했다.
- 남아 있던 Angled/Vertical Ejection, Carrier Velocity와 실제 InitialLaunchDirection MuzzleBlocked를 transient World의 Current Product 실행 경로 `VehiclePawn → VehicleWeaponComp → VehicleFireComp → ProjectilePool → ProjectileActor`로 검증했다.
- Official UE 5.8 Build `740913a4673a4d028bae8c9b8a945f3c` PASS, `CarFight.Launcher.LM_P0_06.FinalIntegration` 1/1 PASS, 전체 `CarFight.Launcher` 5/5 PASS를 최종 closure evidence로 기록했다.
- fresh AssetDump에서 저장 `DA_RocketLauncher`의 Direct / EjectionSpeed 0 / CarrierVelocityRatio 0 기본값을 확인했다. Product `.uasset` mutation과 Product runtime Source mutation은 0이며 신규 Source는 test-only `CFLauncherFinalTests.cpp v1.0.0`이다.
- Current owner를 main_game `Document/Systems/Combat/Launcher.md v1.0.1`으로 동기화했고 ProjectSSOT/ActiveWork/Roadmap은 Done 상태로 정렬됐다.
- 다른 차량 요격, cross-type/Volley 격리, Pool Ignore Reset과 lifecycle cancel은 비차단 후속 회귀로 유지한다.

Migration: CF-FQ-029은 현재 Active/Paused/Ready 작업이 아니다. 현재 Launcher 구현 판단은 main_game `Document/Systems/Combat/Launcher.md v1.0.1`과 실제 Source를 우선한다. 이 Plan은 완료 당시 상세 evidence를 보존하는 Historical + Retained Path이며 과거 USER PIE Pending 문구를 현재 착수 지시로 사용하지 않는다.

### v0.14.0 - 2026-08-16

- `CFLauncherSchedTests.cpp v1.2.0`에서 StopSequence의 `Cancelled / ShotFailed` 이후 `GetDispatchBudget()==0`, `MarkShotDispatched()==false`를 명시적으로 고정했다.
- Launcher Runtime은 재구현·수정하지 않았으며 `RecordShotResult(false)`의 기존 ContinueRemaining / StopSequence 상태 전이를 그대로 검증했다.
- 최종 closure 공식 Editor Build `6a633eb4cf21481d8ae26ec908d05660`가 Exit Code 0으로 PASS했다.
- 최종 closure source에서 `SchedulerContract` `69db5494a03f483e9d8f6d576dd5742c` 1/1, `CarFight.Launcher` `7ab7a286e5484cab845bb0cbfd5ac04e` 4/4, `AMMO_P0_04.LauncherLock` `1781accd3d9c47a59421fb1d2228e4ab` 1/1 PASS를 LM-P0-06A closure evidence로 기록했다.
- Content Asset·Blueprint·USER PIE는 변경·실행하지 않았다. MuzzleBlocked·Angled/Vertical Ejection·Carrier Velocity USER PIE와 CF-FQ-037 SCAN-P0-06 USER PIE는 Pending을 유지한다.

Migration: LM-P0-06A 기술 검증을 반복하지 않는다. CF-FQ-029 재개 시 다음 체크포인트는 LM-P0-06 USER PIE이며, 사용자 확인 전 CF-FQ-029 Done 또는 Systems 승격을 금지한다.

### v0.13.0 - 2026-08-16

- 사용자 선택에 따라 CF-FQ-029를 다시 단일 Active로 전환하고 `LM-P0-06A Failure Policy Technical Closure`를 USER PIE 앞의 원격 기술 Gate로 추가했다.
- 현재 Source 감사에서 `FCFLauncherSequenceRuntime::RecordShotResult(false)`가 ContinueRemaining과 StopSequence 상태 전이를 이미 직접 소유함을 확인해, 전용 실패 Content fixture가 없더라도 asset-free Automation으로 검증 가능하다고 판정했다.
- 기존 `CFAmmoLauncherTests.cpp`의 Manual Cancel + Ammo Reservation/ActionLock terminal cleanup을 현재 보호 회귀로 재분류했다.
- 이번 Gate는 기존 Launcher Runtime을 재구현하거나 플레이어 Cancel 입력을 추가하지 않는다.
- MuzzleBlocked·AngledEjection·VerticalEjection·CarrierVelocityRatio·Direct 기본값 복구 USER PIE는 계속 Pending이며 CF-FQ-029 전체 Done으로 승격하지 않는다.
- CF-FQ-037 Scanner는 `ScannerIntegrationPlan.md v0.7.1 / SCAN-P0-06 USER PIE` 체크포인트가 보존된 Paused로 전환했다.

Migration: 새 세션은 LM-P0-06A에서 Failure Policy Automation을 구현·검증한다. PASS 후에도 LM-P0-06 USER PIE는 별도 Pending으로 남긴다.

### v0.12.1 - 2026-08-02

- Scheduler와 Release를 포함한 Launcher Automation의 초기 Not Run을 당시 체크포인트 상태로 분리했다.
- `UE/Saved/Automation/CombatRuntime/index.json`의 최신 결과에서 Launch Handoff·MuzzleSequence·Pattern·Scheduler·Release Success를 반영했다.
- 검증 결과와 실행 수단을 분리하고 당시 작업 전용 임시 실행 경로를 공용 도구나 영구 재실행 계약으로 승격하지 않았다.
- 남은 MuzzleBlocked·Ejection·Carrier Velocity 사용자 PIE와 Deferred 항목은 Automation PASS와 별개로 유지했다.

### v0.12.0 - 2026-07-30

- 사용자 결정에 따라 후속 발사 실패 정책 `ContinueRemaining·StopSequence` 검증을 현재 LM-P0-06 즉시 완료 조건에서 제외했다.
- 후속 Muzzle만 의도적으로 실패시키는 전용 배치 또는 테스트 훅이 준비된 뒤 해당 정책을 검증하도록 Deferred했다.
- 현재 즉시 Pending은 실제 사출 방향 MuzzleBlocked, AngledEjection, VerticalEjection, CarrierVelocityRatio와 기본값 복구로 축소했다.
- 코드와 에셋은 변경하지 않고 계획 상태만 갱신했다.

### v0.11.0 - 2026-07-30

- 실제 호출 경로를 재검토해 `CancelFireSequence()`는 BlueprintCallable API로 존재하지만 현재 플레이어 입력, UI와 게임플레이 이벤트에서 호출되지 않음을 확인했다.
- 기존 `ContinueRemaining·취소` 표현을 후속 발사 실패 정책과 수동 취소로 분리했다.
- `ContinueRemaining`과 `StopSequence`는 후속 예약 발사의 성공·실패 결과를 처리하는 정책이며, `StopSequence`의 `Cancelled / ShotFailed`는 자동 중단이다.
- 수동 취소와 OwnerDestroyed·WeaponChanged·TurretMountChanged 취소는 현재 검증 수단이 없어 Deferred했다.
- LM-P0-06의 즉시 Pending을 후속 발사 실패 정책, MuzzleBlocked, Ejection, Carrier Velocity와 기본값 복구로 정정했다.

### v0.10.0 - 2026-07-30

- 사용자 PIE에서 Salvo 설정의 네 Projectile이 정상 사출되고 동일 차량 Projectile끼리 사출 직후 상호 Impact되는 결함이 재현되지 않음을 확인했다.
- Projectile의 차량·월드 대상 일반 충돌 동작도 유지됨을 확인했다.
- 다른 차량 Projectile·Hitscan 요격은 현재 플레이어 외 사격 주체가 없어 AI 전투 단계까지 Deferred했다.
- 다른 Volley·다른 탄종 교차 격리와 Pool Ignore Reset도 복수 무기·사격 주체 환경의 후속 회귀로 분리했다.
- 요격 미검증을 LM-P0-06의 즉시 차단 조건에서 제외하고 다음 검증을 ContinueRemaining·취소·MuzzleBlocked·Ejection·Carrier Velocity로 이동했다.
- CF-TC-026은 남은 런처 통합 항목 때문에 PARTIAL을 유지한다.

### v0.9.0 - 2026-07-30

- 사용자 확정에 따라 같은 차량이 발사한 모든 탄종과 모든 Volley를 단일 Projectile 충돌 격리 범위로 고정했다.
- Projectile 채널 기본 Block을 복구하고 동일 ActiveInstigatorActor Projectile만 Pool의 모든 Actor Class 버킷에서 양방향 Ignore한다.
- 다른 차량 Projectile과 Hitscan의 유효 적중으로 bCanBeIntercepted Projectile을 Intercepted 처리하고 선택적 폭발 FX를 연결했다.
- ACFProjectileActor v1.11.0, UCFProjectilePoolComp v1.5.0, UCFProjectileData v1.9.0, ACFVehiclePawn v2.128.0과 CFProjectileCollisionTests v2.0.0을 기록했다.
- 최종 Editor Build Job 940272869b77450797347f76b427faf3 / Exit Code 0을 확인했다.
- 다음 검증을 동일 차량 전체 범위 무충돌, 다른 차량 Projectile·Hitscan 요격과 Pool 관계 초기화로 전환했다.

### v0.8.0 - 2026-07-30

- 사용자 PIE에서 Salvo 런처의 네 Muzzle 동시 사출 자체는 정상임을 확인했다.
- Projectile Object Channel이 자기 채널도 Block해 네 Projectile이 사출 직후 상호 Impact 처리되는 결함을 확인했다.
- ACFProjectileActor v1.10.0에서 Projectile 채널 Ignore와 Impact Guard를 적용하고 CFProjectileCollisionTests.cpp v1.0.0을 추가했다.
- 공식 Editor Build Job c96631773b2543b0b480e82da5d63e7c / Exit Code 0을 확인했다.
- Salvo 전체 PASS는 수정 후 사용자 PIE 전이므로 보류하고 다음 실행을 Salvo 상호 충돌 재검증으로 제한했다.

### v0.7.0 - 2026-07-30

- 사용자 PIE에서 Direct Launch Handoff, 기존 Rocket 추진, 발사 후 런처 독립과 Pool 5 Volley 이상을 PASS했다.
- Direct Ripple 4발·0.15초, Muzzle 1→4→2→3, 고정 Command Target, 추가 입력 중복 방지와 SequenceCompleted 쿨다운을 PASS했다.
- SingleCycle 입력당 1발, Muzzle 순환 1→4→2→3→1, 발사 거부 시 Projectile·FX 없음과 Muzzle 인덱스 유지를 PASS했다.
- Damage는 현재 보강 미완료로 검증 차단이며 런처 실패로 판정하지 않았다.
- CF-TC-025·026을 PARTIAL로 유지하고 다음 검증을 Salvo로 이동했다.

### v0.6.0 - 2026-07-29

- `DA_RocketBody`를 RocketLauncherYaw·Pitch와 Muzzle 순서 `1→4→2→3`에 연결하고 모든 Muzzle 필수 정책을 저장했다.
- `DA_RocketLauncher`와 `RocketLauncher` EquipmentPresetData를 생성해 Ripple 4발·0.15초·ContinueRemaining·SequenceCompleted·Direct Release를 연결했다.
- `DA_TestSUV`, `TestMap.BP_CFVehiclePawn_C_1`과 `BP_CFVehiclePawn` CDO를 Launcher 검증 경로로 연결하고 Sedan·HeavyCannon 표적을 유지했다.
- 독립 AssetDump에서 LauncherPost 10/10, VehiclePost 3/3, MapPost 19/19를 확인했고 신규 Unreal 프로세스에서 CDO의 DA_TestSUV 저장값을 재확인했다.
- `LM-P0-05`를 완료하고 `LM-P0-06 CF-TC-025·026 통합 사용자 PIE`를 Active로 이동했다.
- 사용자 PIE 전에는 테스트 PASS, CF-FQ-029 Done 또는 Systems 승격을 기록하지 않는다.

### v0.5.0 - 2026-07-29

- `FCFLauncherReleaseConfig`에 Direct·AngledEjection·VerticalEjection 방향, EjectionSpeed, CarrierVelocityRatio와 LauncherClearanceTraceDistanceCm 계약을 추가했다.
- WeaponData → WeaponComp → Pawn → Launch Context → Pool·Projectile Actor의 Release-aware 경로를 연결했다.
- CommandTargetLocation과 InitialLaunchDirection을 분리하고 발사 순간 차량 Velocity 상속 성분을 값으로 복사한다.
- 비Direct 발사는 실제 InitialLaunchDirection 앞쪽을 검사하고 같은 Launch Context를 Projectile 활성화에 전달한다.
- `CFLauncherReleaseTests.cpp`를 추가하고 최종 Editor Build Job `acd575ffac2f4d64bd73194e160a3f62`에서 Exit Code 0을 확인했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run, 통합 PIE는 Pending으로 유지했다.
- 다음 단계를 `LM-P0-05 Launcher Editor Assets`로 이동했다.

### v0.3.0 - 2026-07-28

- `UCFTurretMountData`에 가변 `MuzzleSocketNames`와 `bRequireAllMuzzles`를 추가하고 기존 `MuzzleSocketName` 단일 fallback을 유지했다.
- `UCFVehicleWeaponComp`가 다음 Muzzle 인덱스를 소유하고 승인 발사 후에만 SingleCycle 순서를 진행하도록 구현했다.
- 선택 Muzzle 이름·인덱스·설정 슬롯 수를 FireOrigin, FireRequest와 Projectile Launch Context에 값으로 복사한다.
- 누락 Muzzle은 `bRequireAllMuzzles=false`에서 건너뛰며 True에서는 해결 실패로 처리한다.
- `CarFight.Launcher.LM_P0_02.MuzzleSequence` RuntimeContract 소스를 추가했다.
- 첫 빌드 Job `2182cb31e8984dfd98b741536f5a6b96`의 멤버·지역 변수 이름 충돌을 수정하고 최종 Job `409b7e9a4a9b49978fc5fe83ae29cb41`에서 Exit Code 0을 확인했다.
- `.uasset`은 변경하지 않았으며 실제 Muzzle 소켓 존재·배열 순서·시각 발사는 Editor Pending이다.

### v0.2.0 - 2026-07-28

- `FCFProjectileLaunchContext`와 `ECFProjectileReleaseMode`를 추가했다.
- `ACFVehiclePawn`이 기존 Fire Command에서 Direct Launch Context를 생성하도록 연결했다.
- `UCFProjectilePoolComp`와 `ACFProjectileActor`에 Context 경로와 Legacy Direct Adapter를 추가했다.
- ProjectileMovement 초기 Velocity는 Context 값을 사용하고 무효 시 기존 Direction × InitialSpeed로 복구한다.
- 비활성화와 Pool 반환 전에 Active Launch Context를 초기화한다.
- `CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract` 자동화 소스를 추가했다.
- 공식 Editor Build Job `056397a4b43d49c9845c9cc296b23b9c`에서 Exit Code 0을 확인했다.
- 자동화 실행은 도구 미제공으로 Not Run, 사용자 PIE는 Pending으로 유지했다.

### v0.1.0 - 2026-07-28

```text
- CF-FQ-029 모듈형 런처 및 발사 인계를 Active 기능으로 준비했다.
- CF-FQ-030 물리 제한형 미사일 비행·유도를 선행 작업 대기 Ready 기능으로 분리했다.
- 런처와 미사일 사이의 Projectile Launch Handoff 계약을 확정했다.
- Direct, Angled, Vertical 사출과 Single, Ripple, Salvo 발사 확장 방향을 고정했다.
- Ejection 기본 이동을 ProjectileMovement 기반 통제된 탄도 운동으로 결정했다.
- 미사일의 발사 후 독립 이동과 유도 명중 비보장 원칙을 강제 계약으로 기록했다.
- 설계, 로드맵, 런처·미사일 상세 Plan, TaskSource와 WorkOrder를 생성했다.
- 첫 코드 작업을 LM-P0-01 Launch Handoff Foundation으로 제한하고 Source Not Modified 상태를 유지했다.
```

---

## 11. Migration

```text
- 기존 단일 Muzzle 직사 발사는 Direct / CarrierVelocityRatio 0 기본값으로 같은 결과를 유지한다.
- 기존 FCFVehicleFireRequest.AimDirection과 AcquireProjectile / ActivateProjectile 호환 어댑터는 유지한다.
- 실제 Angled·Vertical 동작은 LM-P0-05에서 연결한 WeaponData와 Muzzle 소켓 설정이 있을 때만 활성화된다.
- 현재 ProjectileData는 Guidance 설정 없이 기존 포탄 또는 비유도 Rocket으로 동작한다.
- LM-P0-01~06과 LM-P0-06A 완료 evidence를 새 failure evidence 없이 반복하지 않는다.
- 기본 Launcher WeaponData는 Direct / Ripple 4발 / 0.15초 / SequenceCompleted 상태이며 fresh AssetDump에서 저장 `DA_RocketLauncher`의 Direct / EjectionSpeed 0 / CarrierVelocityRatio 0 기본값을 확인했다.
- CF-FQ-029은 Done이며 현재 Launcher 구현 owner는 main_game `Document/Systems/Combat/Launcher.md v1.0.1`이다.
- CF-FQ-030 Missile Guidance도 별도 lifecycle로 Done이며 Launcher와 Missile Flight/Guidance 책임 경계를 유지한다.
- 과거 Automation Not Run은 당시 체크포인트 이력으로만 읽고 현재 증거는 main_game의 Current 문서와 최신 결과 파일을 우선한다.
- 당시 작업 전용 임시 실행 경로를 공용 도구, Git 등록 대상 또는 다음 세션의 고정 재실행 경로로 가정하지 않는다.
```
