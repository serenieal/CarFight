# SystemIndex

- Version: 1.55.0
- Date: 2026-10-07

- Status: Active
- Scope: `Document/Systems/` 하위 문서 위치 안내 색인

---

## 1. 문서 목적

이 문서는 `Document/Systems/` 폴더 안에 있는 시스템 문서들이 각각 **어떤 기능을 설명하는 문서인지** 빠르게 찾기 위한 색인이다.

이 문서는 설계 로드맵이나 개발 순서표가 아니다.
문서를 열기 전에 아래 표에서 필요한 기능을 찾고, 해당 경로의 문서를 확인하면 된다.

---

## 2. 최상위 문서

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/SystemIndex.md` | `Document/Systems/` 하위 문서들이 어디에 있고, 어떤 기능을 다루는지 정리한 색인 문서다. |
| `Document/Systems/SystemTemplate.md` | 새 시스템 문서를 작성할 때 사용하는 기본 템플릿이다. 문서 목적, 범위, 현재 역할, 책임, 비책임, 갱신 조건, 버전 관리를 어떤 형식으로 적을지 정의한다. |

---

## 3. Combat 폴더

현재 Combat 폴더 문서는 **현재 구현된 전투 런타임과 싱글플레이 로컬 전투 피드백 기준**을 기록한다.
직접 피해, Shield, 6방향 독립 Armor, ArmorPenetration, Vehicle Integrity, 최초 파괴 상태와 차량 finite Ammo·Reload는 현재 Systems 범위에 포함한다. 실제 부품별 손상, 도탄, 범위 피해, 완성형 파괴 물리와 서버 권한 전투는 후속 범위다.

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Combat/WeaponData.md` | `CF-FQ-008`에서 완료한 `UCFWeaponData` 정적 무기 DataAsset Current System이다. Identity·Mount·Mass·Fire·TargetUse·Launcher·Ammo·Projectile/FX 정적 설정, DataValidation, fallback과 legacy field 책임 경계를 기록하며, 현재 Prototype Cannon/Rocket 질량은 `Provisional Gameplay Balance`로 관리한다. |
| `Document/Systems/Combat/WeaponFire.md` | 싱글플레이 로컬 차량 Pawn에서 Fire 입력을 발사 명령으로 만들고, Weapon Aim Solution을 기준으로 Projectile Actor 또는 Dummy HitScan 경로로 넘기며, TargetActor Guided Projectile에 한해 Vehicle Locked Target을 발사 전 Guidance source로 fail-closed admission하는 현재 발사 기능 문서다. Ammo 수량·Reload 상태는 `UCFVehicleAmmoComp`가 소유한다. |
| `Document/Systems/Combat/Launcher.md` | `CF-FQ-029`에서 완료한 모듈형 Launcher Current System이다. Launch Context, 가변 Muzzle, SingleCycle/Ripple/Salvo, Direct/Angled/Vertical Release, Carrier Velocity, 실제 사출 방향 MuzzleBlocked, Sequence 실패/취소와 Projectile Pool 인계 경계를 기록하며 Phase 7부터 첫 승인 발사의 GuidanceTargetActor snapshot을 Volley 전체에 보존한다. |
| `Document/Systems/Combat/Ammo.md` | `CF-FQ-031`에서 완료한 차량 finite Ammo Current System이다. WeaponInstanceId별 Loaded, AmmoId별 Reserve, SingleCycle Commit·Rollback, Ripple·Salvo 전체 예약, FullMagazine Reload, WeaponPanel `Loaded / MagazineCapacity + Reserve`와 출격 Ammo 질량 계약을 기록한다. Heavy·Ripple USER PIE를 완료했다. |
| `Document/Systems/Combat/FireFeedback.md` | `WeaponFire`가 남긴 로컬 발사 성공·실패·쿨다운·NoWeapon·AimBlocked·TurretAligning·MuzzleBlocked 결과를 Reticle 텍스트와 색상으로 표시한다. P0 상태 전환과 피드백 만료를 사용자 PIE로 확인했다. |
| `Document/Systems/Combat/Projectile.md` | `ProjectileData`, 공통 `CFProjectileActor`, `ProjectileMotorComp`와 `ProjectilePoolComp`를 통한 비추진 포탄·비유도 Rocket 이동, Missile Flight/Guidance 컴포넌트 통합 경계, 지속형 Trail·Thruster, 충돌·Impact·Pool 생명주기를 기록한다. Guidance 세부 Current 계약은 `MissileGuidance.md`가 소유한다. |
| `Document/Systems/Combat/MissileGuidance.md` | `CF-FQ-030`에서 완료한 Direct TargetActor 물리 제한형 Missile Flight/Guidance Current System이다. Phase 7부터 VehicleFireComp가 Vehicle Locked Target에서 확정한 Launch Target Snapshot을 소비하며, Released/Clearance/GuidedFlight, PurePursuit·LeadPursuit·PN, Independent Activation, Stateful Seeker, Sampled Observation, Target Lost/Reacquisition, Pool Reset, passive Guidance Preset과 `CF-TC-027` Technical + USER Feel PASS를 기록한다. |
| `Document/Systems/Combat/DamageHitContext.md` | Dummy HitScan과 Projectile의 시각 차체 Hit 결과, `HitComponentName`, 위치/노멀/입사 방향을 같은 `FCFDamageHitContext` 형식으로 기록한다. 이 Context는 현재 `HitDamage`의 공용 피해 적용 입력으로 사용된다. |
| `Document/Systems/Combat/HitDamage.md` | HitScan·Projectile의 `FCFDamageHitContext`를 정식 `VehicleDefenseComp` 진입점으로 연결하고, Shield·Armor 이후 Vehicle Integrity 적용, Legacy Fallback, 최초 파괴와 기존 `FCFDamageApplyResult` 호환을 기록한다. |
| `Document/Systems/Combat/VehicleDefense.md` | `VehicleDefenseData`와 `VehicleDefenseComp`가 소유하는 Shield, 재생, Front·Left·Right·Rear·Top·Bottom 독립 Armor, 방향 배율, ArmorPenetration, Armor Overflow, Vehicle Integrity 전달과 Fitting Defense Commit을 기록한다. |
| `Document/Systems/Combat/CombatFx.md` | 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 DataAsset 기반 Niagara로 정확히 한 번 표현한다. `NS_BasicHit` Impact, 차량별 `SM_Body.FX_Destroyed` 소켓, 최대 수명 안전 퓨즈와 최종 사용자 PIE PASS를 기록한 현재 전투 FX 문서다. |

---

## 4. Config 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Config/ProjectRuntimeConfig.md` | 프로젝트 시작 맵, 렌더링 기술, 하드웨어 타깃, 입력 백엔드, 축 기본값 같은 런타임 환경 설정을 설명하는 문서다. |

---

## 4.1 DataManagement 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/DataManagement/DataAssetManagement.md` | `CF-FQ-045` Data Asset Manager read-first 관리 경계와 `CF-FQ-058` scoped authority cutover를 함께 기록한다. unmanaged/legacy content는 persisted `.uasset` current truth를 유지하고, CCAS-managed exact42는 canonical Workbook authoring authority → generated Production `.uasset` → Production Publication Catalog / provenance 구조를 사용한다. Manager 자체는 no-auto-mutation/save를 유지한다. |
| `Document/Systems/DataManagement/DataAssetAuthoring.md` | `CF-FQ-049~053` JSON Staging/Reviewed Apply current path와 `CF-FQ-058` CCAS-managed Workbook authority를 scope별로 함께 기록한다. unmanaged/legacy typed-provider scope는 기존 JSON→typed Apply→persisted `.uasset` 계약을 유지하고, managed exact42는 `CarFight_Content.xlsx`를 sole authoring authority로 사용하며 generated Production `.uasset` exact34와 Publication Catalog exact8을 Runtime/Product materialization으로 소비한다. |
| `Document/Systems/DataManagement/BuilderAuthoringStandard.md` | CarFight의 Builder/Authoring Guide 공통 규약이다. Vehicle Builder의 Common Page Shell과 USER 정보 계층을 기준으로 사용자 업무 단위 Builder, Builder Chaining 금지, Stable StepId/Conditional Step, 한 Page 하나의 작업 의도, 신규/기존 Asset 패턴, Naming/Identity, 기존 writer authority 재사용, explicit Preview/Apply/Save, Multi-Asset partial recovery, Final Review/Handoff와 Advanced 경계를 정의한다. |

---

## 5. Input 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Input/Input.md` | 차량 입력 자산, 입력 매핑 컨텍스트, 장치 모드, 2D 입력, Legacy 입력 충돌 제어, 최종 차량 주행/카메라 입력 전달 흐름을 설명하는 문서다. |

---

## 6. Maps 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Maps/.gitkeep` | 현재 `Maps` 폴더 유지를 위한 빈 파일이다. 기능 설명 문서는 아직 없다. |

---

## 7. Network 폴더

현재 `Document/Systems/Network/`에는 Current System 문서가 없다. 과거 Dedicated Server / Multiplayer Spawn 구현 기록은 Current Systems에서 내려와 다음 Historical 경로에 보존한다.

```text
Document/ProjectSSOT/Archive/Systems/Network/ServerSpawn.md
```

서버/멀티 작업을 재개할 때는 이 Historical 기록을 그대로 Current로 복원하지 않고 현재 Source·ProjectSSOT를 다시 감사해 새 lifecycle을 연다.

---

## 8. Targeting 폴더

현재 Targeting 폴더는 TargetSelect의 후보·선택 authority, 차량 Sensor Contact/Knowledge Current System, Vehicle Target Lock Runtime, Phase 6 HUD Presentation과 Phase 7 Guided Weapon source 경계를 서로 분리해 기록한다. Phase 5 이후 `ACFVehiclePawn`은 이 독립 Runtime을 합치지 않고 현재 Selection을 명령 시점에만 읽어 Lock/Scan command를 전달하는 얇은 Gameplay facade다. Phase 6 HUD도 Selection/Knowledge, Target Lock, Target Scan을 서로 다른 ViewData 채널로 소비한다. Phase 7에서는 TargetActor Guided Projectile의 발사 전 Guidance source만 Vehicle Locked Target으로 이동했으며 TargetSelect의 후보 검색·선택 수명, Sensor의 Detection·Contact lifecycle·Knowledge, Vehicle Targeting의 Lock state/quality 책임을 합치지 않는다.

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Targeting/TargetSelect.md` | `CF-FQ-026`에서 구현된 Target Candidate/Selected Actor authority Current System이다. Targetable/TargetPoint, TargetRegistry 후보 공급, 선택 수명·입력, Sensor/Scanner와 Aim/Fire 소비 경계, UISubsystem Target Marker/Production TargetPanel 경계와 Deferred USER tuning debt를 기록한다. |
| `Document/Systems/Targeting/SensorContact.md` | `CF-FQ-036/037` 기반 Sensor Runtime Current System이다. bounded Detection, Contact/Knowledge/Persistent Store, actor-free Snapshot, Target Scan과 Phase 5 `CancelTargetScan()` scan-only cancel, broad Sensor Operation cancel 분리, Phase 6 `Snapshot.ScanAttempt → FCFTargetScanHUDData` Selection-independent HUD 경계를 기록한다. |
| `Document/Systems/Targeting/VehicleTargeting.md` | Phase 4 Vehicle Target Lock Runtime + Phase 5 Gameplay Command Boundary + Phase 6 HUD Presentation + Phase 7 Guided Weapon source migration의 Current System이다. `Idle / Acquiring / Locked`, Live Contact-only Lock, actor-free Snapshot, 독립 command 의미, `FCFTargetingSnapshot → FCFTargetLockHUDData` HUD 경계와 TargetActor Guided Projectile의 `Vehicle Locked Target → GuidanceTargetActor` fail-closed 발사 전 bridge를 기록한다. 새 Lock 물리 Input은 후속이다. |


---

## 9. UI 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/UI/InGameUI.md` | `CF-FQ-032` 인게임 UI Current System이다. LocalPlayer `UCFUISubsystem → UCFUIRootWidget` 8 Layer 수명, Production `UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter` 데이터 경계, HUD·AimReticle·Target Marker·Pause·Radar·Weapon과 VehiclePanel persisted editable 구조의 현재 구현, USER Visual/Feel Deferred 경계를 기록한다. |
| `Document/Systems/UI/AimReticle.md` | `UCFUISubsystem`이 HUD Layer Z10에서 `WBP_AimReticle` singleton 수명과 Current Pawn Rebind를 소유하고, Widget은 Weak Pawn 참조로 `Image_CenterDot` 조준 레티클·`CurrentMuzzleDirection` 기반 `Image_WeaponReticle` 터렛 레티클과 로컬 Aim/FireFeedback 상태를 표시한다. 기존 Aim/FireFeedback USER PIE 의미 계약을 보존한다. |
| `Document/Systems/UI/DisplayTextPolicy.md` | 내부 식별자는 영문으로 유지하고, 화면에 보이는 UI/Debug UI 텍스트는 한국어로 표시한다는 표시 텍스트 정책 문서다. |
| `Document/Systems/UI/VehicleDebugPanel.md` | `VehicleDebug Panel`의 Navigation + Selected Section 구조, TopLevel Section, Camera Debug 편입 상태, 표시 언어 정책을 설명하는 문서다. |

---

## 10. Vehicles 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Vehicles/VehicleAim.md` | `VehicleCamera`가 만든 조준 결과와 Weapon Aim Solution을 Local 표시·검증 상태로 관리하고, 사용자 조준점과 `CurrentMuzzleDirection` 기반 터렛 레티클 월드 지점을 분리해 제공한다. 정렬 정책, `MuzzleBlocked`와 CF-FQ-025 터렛 레티클을 사용자 PIE로 확인했다. |
| `Document/Systems/Vehicles/VehicleCamera.md` | 차량 기준 자유 조준과 AimTrace를 유지하면서 normalized Driving FX, Gameplay/Presentation View 분리, Speed FOV/Arm·Accel/Brake Kick·body-motion Lateral Roll·충돌 복귀를 계산/적용하는 차량 카메라 Current owner다. Combat/Aim/Airborne 감쇠 producer wiring과 일부 feel tuning은 명시적 후속 debt로 분리한다. |
| `Document/Systems/Vehicles/VehicleCoreDecisions.md` | 현재 차량 코어의 유지 결정, 교체 결정, 임시 운영 판단을 기록하는 결정 로그 문서다. 차량 코어 변경 전 확인해야 하는 기준 문서다. |
| `Document/Systems/Vehicles/VehicleBuilder.md` | `CF-FQ-015` + `CF-FQ-038` + `CF-FQ-040` + `CF-FQ-042` + `CF-FQ-043` + `CF-FQ-044` + `CF-FQ-046` + `CF-FQ-047` 완료 기준 Vehicle Builder·Data Authoring·Performance Tuning Current System이다. 8-Step Guided Shell, User-Facing Information Architecture, 단일 Common Page Shell/scroll/overflow 계약, Data Authoring Backend, 전문가용 Advanced Workspace, Controlled Axis Tuning, Technical Benchmark + USER Feel Pair, Vehicle Character / Reference Baseline, Measurement Gap ownership, Legacy Vehicle DA Wizard retirement, 신규 차량 Creation Entry, Hardpoint/Mount/Socket authoring integrity, durable final commit와 persistent USER Driving receipt, Runtime Catalog promotion 경계를 기록한다. |
| `Document/Systems/Vehicles/VehicleData.md` | `UCFVehicleData`의 외형·Layout·Hardpoint·MountProfile·Fitting Mass·Movement·WheelVisual·Reference·Defense/Fx·DriveState 구성과 실제 Pawn 적용 순서를 기록한다. CF-FQ-015 Historical Validator/Representative Compare/Runtime Apply 기반을 보존하며, 실제 성능 튜닝 workflow는 VehicleBuilder가 소유한다. WSA 완료 기준 USER-authored Wheel Socket Scale → visual/physics size authority도 포함한다. |
| `Document/Systems/Vehicles/VehicleDrive.md` | 차량 입력을 Chaos Vehicle Movement에 적용하고, 속도/방향/접지/입력 상태를 바탕으로 DriveState를 계산/유지하는 주행 상태 기능 문서다. |
| `Document/Systems/Vehicles/VehiclePawnLegacy.md` | `CFModVehiclePawn / BP_ModularVehicle` 계열을 현재 주력 차량 Pawn이 아닌 레거시 계열로 정리하는 문서다. |
| `Document/Systems/Vehicles/VehicleRuntime.md` | `CF-FQ-048 Vehicle Pawn Slimming` 완료 기준 Current owner다. Pawn은 lifecycle·composition root·input entry·target identity·Public/BP/Automation/RuntimeApply compatibility facade·observable state Authority를 유지하고, Visual/Fire/Runtime behavior는 `CFVehicleVisualComp`·`CFVehicleFireComp`·`CFVehicleRuntimeComp` coordinator로 분리한다. VehicleData/Prepared Fitting Snapshot/Initial Sortie·Field Mass/Ready 판정, WSA Wheel Visual deterministic reapply와 Input/Debug reflected type owner 분리 경계도 함께 기록한다. |
| `Document/Systems/Vehicles/VehicleInventory.md` | `CF-FQ-035`에서 구현된 실제 ItemInstance 소유권, VehicleCargo/MountedEquipment, 접근·Capacity, Reservation, Atomic Transfer/Rollback, read-only ViewData, Inventory→Fitting Adapter와 Field Fit completion 경계를 기록한다. Formal ownership-aware USER frontend는 Deferred이며 RuntimeApply 비소유권 경로와 구분한다. |
| `Document/Systems/Vehicles/RuntimeApply.md` | `CF-FQ-041` RuntimeApply의 Current System이다. Vehicle 후보는 RuntimeTestCatalog, Equipment 후보/authorization은 CF-FQ-058 Production Publication Catalog exact8 우선(부재/invalid 시 legacy fallback)으로 분리하며, Complete Mount State, Legacy→Snapshot, Empty Mount, Fitting Snapshot/GrossMass, Prepare/Commit/Recovery와 향후 Garage/Inventory 재사용 경계를 기록한다. Current Wagon은 Production Cannon_Standard/Rocket_Standard와 구조 호환이지만 gross-mass 계약에서 fail-closed한다. |
| `Document/Systems/Vehicles/VehicleSteering.md` | 게임패드 VehicleMove 2D 입력 방향을 목표 조향으로 해석하고, 제한 속도와 차량 속도 기반 중립 복귀 규칙을 거쳐 실제 조향값을 적용하는 문서다. |
| `Document/Systems/Vehicles/WheelSync.md` | 실제 Movement와 휠 회전 상태를 읽어 각 휠의 조향, 서스펜션, 스핀 시각 입력을 만들고 Anchor/Mesh에 적용한다. WSA 완료 기준 FL-only Right fallback의 per-wheel spin handedness와 absolute/delta local-axis spin 계약을 포함한다. |

---

## 11. 기능별로 찾기

| 찾고 싶은 내용 | 확인할 문서 |
| --- | --- |
| 무기 정적 DataAsset 계약, 장착 호환·질량·Fire·TargetUse·Launcher·Ammo 설정, DataValidation과 legacy/fallback 경계 | `Combat/WeaponData.md` |
| 현재 로컬 발사 명령, WeaponData 해석, 쿨다운, FireOrigin, TargetActor Guided Weapon의 Locked Target admission, 발사 결과 기록 | `Combat/WeaponFire.md` |
| 가변 Muzzle, SingleCycle/Ripple/Salvo, Direct/Angled/Vertical 사출, Carrier Velocity, MuzzleBlocked, 첫 승인 발사의 Guidance Actor Volley Snapshot과 Launch Context 인계 | `Combat/Launcher.md` |
| 차량 finite Ammo, 무기별 장전량·탄종별 Reserve, Launcher 예약, FullMagazine Reload, WeaponPanel 탄약 표시와 출격 탄약 질량 | `Combat/Ammo.md` |
| 발사 성공/실패/쿨다운/무기 없음 상태를 Reticle, HUD와 시각 VFX로 표시하는 기준 | `Combat/FireFeedback.md` |
| 프로젝트 전역 게임 사운드 비지원 결정과 오디오 도입 금지 기준 | `Document/ProjectSSOT/04_ProjectDecisions.md` |
| Projectile Actor 활성화, 비유도 Rocket 추진 상태, Missile 컴포넌트 통합 경계, Trail·Thruster 지속형 FX, 일반·고속 충돌, 첫 Impact 피해와 Pool Reset 기준 | `Combat/Projectile.md` |
| Direct TargetActor 미사일의 Vehicle Locked Target 기반 Launch Snapshot, 비행 상태, Guidance Law, 물리 제한, Guidance Activation, Seeker/Observation/Reacquisition, Guidance Preset과 USER Feel 기준 | `Combat/MissileGuidance.md` |
| Dummy HitScan / Projectile 시각 차체 HitContext와 HitComponent 기록 | `Combat/DamageHitContext.md` |
| HitScan·Projectile 공용 피해 진입점, Vehicle Integrity 적용, Legacy Fallback과 최초 파괴 상태 | `Combat/HitDamage.md` |
| Shield, 6방향 Armor, 관통·Overflow, 재생과 방어층별 전체 결과 | `Combat/VehicleDefense.md` |
| Muzzle·Impact·Destroyed Niagara의 데이터 연결, 발생 위치, 1회성, 중복 방지와 잔류 안전 계약 | `Combat/CombatFx.md` |
| Reticle 목표점, 터렛 추적, Muzzle 방향을 하나의 Aim Solution으로 통합한 완료 설계·검증 기록 | `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md` |
| Sweep/Sub-stepping/보조 Sphere Sweep을 통한 고속 Projectile 연속 충돌 완료 설계·검증 기록 | `Document/Plan/Archive/ProjectileContinuousCollision/ImplementationDesign.md` |
| 프로젝트 시작 맵, 렌더링, 입력 백엔드 설정 | `Config/ProjectRuntimeConfig.md` |
| CarFight DataAsset 종류·용도·관리 상태 탐색, 고유 ID/중복 검사, 참조 관계 조회, Refresh 후 재검사 필요 상태와 no-auto-save 관리 경계 | `DataManagement/DataAssetManagement.md` |
| Editor가 꺼진 동안 DataAsset JSON 의도를 작성하고 exact Preview/Review 뒤 명시적으로 typed DataAsset을 Batch 적용하는 절차, MissileGuidePreset+AmmoData mixed Explicit Paths, stale/conflict/dirty 보호와 shared durable save/readback, Staging 지원 DA의 C++ contract drift·revision·migration·accepted snapshot 및 provider-centric 새 타입 확장 기준 | `DataManagement/DataAssetAuthoring.md` |
| 신규 Builder/제작 가이드의 공통 레이아웃, Step 분할, Builder 연쇄 금지, 신규/재사용 자산 UX, 저장·검증·복구·Advanced 경계와 착수 체크리스트 | `DataManagement/BuilderAuthoringStandard.md` |
| 입력 액션, 매핑 컨텍스트, 키보드/게임패드 입력 처리 | `Input/Input.md` |
| 과거 Dedicated Server 접속 후 차량 Pawn 생성과 Possess 기록 | `Document/ProjectSSOT/Archive/Systems/Network/ServerSpawn.md` |
| Target Candidate/Selected Actor authority, Targetable/TargetPoint, TargetRegistry 후보 공급, 선택/해제 입력과 Sensor·Aim·Fire·HUD 경계 | `Targeting/TargetSelect.md` |
| 차량 Sensor 탐지, ContactId, Live/LastKnown/Lost/DestroyedHold, Knowledge/Persistent Store, Target Scan, Scan-only cancel, broad Sensor Operation cancel과 Selection-independent Scan HUD 경계 | `Targeting/SensorContact.md` |
| Vehicle Target Lock의 Idle/Acquiring/Locked 상태, Live Contact-only RequestLock, Lock Progress/Quality, Break revision/reason, Selection→Lock/Scan command 독립 경계, Target Lock HUD Presentation과 TargetActor Guided Weapon의 Locked Target source 경계 | `Targeting/VehicleTargeting.md` |

| UI 텍스트를 한국어로 표시하는 기준 | `UI/DisplayTextPolicy.md` |
| 레거시 `WBP_VehicleDebug` 문자열 표시 비교 기준 | `Document/ProjectSSOT/Archive/Systems/UI/VehicleDebug.md` |
| 차량 디버그 패널의 탭/섹션 구조 | `UI/VehicleDebugPanel.md` |
| 인게임 UI Root·Layer 수명, Production HUD 데이터 흐름, Pause, Radar·Target·Weapon UI의 현재 구현 | `UI/InGameUI.md` |
| 조준점/Reticle 표시, 로컬 발사 결과 피드백과 UISubsystem singleton/Rebind | `UI/AimReticle.md` |
| 차량 코어 변경 전 결정 기준 | `Vehicles/VehicleCoreDecisions.md` |
| Guided Vehicle Builder 8-Step 제작 흐름, USER 정보 우선순위와 Level 0/1/2 정보 계층, 단일 Common Page Shell/Scroll/overflow 계약, Data Authoring Backend·Advanced Workspace, 차량 성능 튜닝의 Controlled Axis / Technical Benchmark+USER Feel / Vehicle Character Baseline / Measurement Gap 계약, Blank/Unused/Reused Mesh Creation Entry, Hardpoint/Mount/Socket integrity, AI Reference/Physics Proposal, Step 7 durable final commit, Step 8 persistent Driving acceptance, Runtime Demo Catalog promotion과 Legacy Vehicle DA Wizard retirement | `Vehicles/VehicleBuilder.md` |
| 차량 DataAsset 구조 | `Vehicles/VehicleData.md` |
| 차량 BeginPlay 준비, Fitting Snapshot, Initial/Field Mass 적용·검증·rollback과 Ready 판정 | `Vehicles/VehicleRuntime.md` |
| 실제 소유 ItemInstance, VehicleCargo/MountedEquipment, Inventory Reservation·Atomic Transfer·ViewData와 Inventory→Fitting 경계 | `Vehicles/VehicleInventory.md` |
| Runtime Catalog/향후 Garage·Inventory frontend와 분리된 Vehicle/Equipment Runtime Apply, Complete Mount State, Empty Mount, Snapshot 검증·Prepare/Commit/Recovery 경계 | `Vehicles/RuntimeApply.md` |
| 차량 주행 입력 적용과 DriveState | `Vehicles/VehicleDrive.md` |
| 게임패드 2D 조향 해석과 조향 복귀 | `Vehicles/VehicleSteering.md` |
| 바퀴 위치, 조향 피벗, 스핀, 휠 시각 동기화 | `Vehicles/WheelSync.md` |
| 차량 카메라, FOV, AimTrace | `Vehicles/VehicleCamera.md` |
| 차량 조준 상태, 로컬 발사 검증용 Aim 상태, OutOfArc / 조준각 경고 기준 | `Vehicles/VehicleAim.md` |
| 예전 `BP_ModularVehicle` 계열 정리 | `Vehicles/VehiclePawnLegacy.md` |

---

## 12. 문서 추가 시 갱신 규칙

`Document/Systems/` 아래에 새 시스템 문서를 추가하면 이 문서도 함께 갱신한다.

갱신할 위치:

1. 해당 폴더 섹션의 문서 표
2. 필요한 경우 `기능별로 찾기` 표
3. 아래 Changelog

---

## 13. Changelog

### v1.55.0 - 2026-10-07

- `Vehicles/RuntimeApply.md v1.2.0`을 반영해 CF-FQ-058 managed cutover 이후 normal Equipment discovery/authorization source가 Production Publication Catalog exact8 우선으로 전환된 Current 경계를 색인에 동기화했다.
- Vehicle discovery는 기존 RuntimeTestCatalog를 유지하고 Equipment legacy catalog는 Production Catalog absent/invalid 시 fallback으로만 남는다. 기존 Fitting Snapshot/Prepare/Commit/Recovery application authority는 중복 구현하지 않는다.
- current Wagon은 Production Cannon_Standard/Rocket_Standard와 Mount 구조상 호환되지만 3216kg/3112kg candidate total이 2350kg gross limit을 초과해 ValidationFailed + mutation0가 정상이다. 과거 Prototype Provisional Balance 장착 성공은 Historical evidence로 내린다.
- Official UE 5.8 Build `97030b7aba1348d4b1688c34afc19954` PASS, RuntimeApply `12eb4517709249cd9d5a8b0e5a4d25a6` 18/18 PASS, ProductionCutoverVerification `d719964509e14441b7160972c56694bd` 1/1 PASS를 Current evidence로 연결했다.

### v1.54.0 - 2026-10-07

- `CF-FQ-058` managed authority cutover의 Current System promotion을 DataManagement 색인에 반영했다.
- CCAS-managed exact42 manifest는 canonical `Authoring/Content/CarFight_Content.xlsx`를 sole Current authoring authority로 사용하고, persisted Production `.uasset` exact34는 generated Runtime/Product materialization, `DA_ProdEquipCatalog`은 published Product exact8 membership authority, `CarFight_Content.cfsnapshot.json`은 provenance/drift evidence로 구분했다.
- unmanaged / ExternalReadOnly / legacy 콘텐츠의 기존 DataAsset + JSON Staging workflow는 변경하지 않았다.
- Data Asset Manager 자체는 read-first/no-auto-save 경계를 유지하며 Workbook write authority를 흡수하지 않는다.

### v1.53.0 - 2026-10-02

- `CF-FQ-057 Vehicle Camera Driving FX`의 VCFX-P0-04 Current System Promotion을 반영해 `Vehicles/VehicleCamera.md v1.3.0`을 최종 Current owner로 등록했다.
- Normalized Driving FX, Gameplay/Presentation View 분리, body-motion Lateral Presentation, Overspeed, collision recovery와 Aim/Targeting 보호를 현재 계약으로 반영했다.
- Comfort Deferred, Rear/Brake tuning Deferred, Combat/Aim/Airborne producer wiring debt는 완료로 확대하지 않고 Current 제약으로 보존한다.

### v1.52.1 - 2026-09-18

- Phase 7 Final Audit 문서 교정을 반영해 `Combat/WeaponFire.md v1.9.1`을 Current owner로 갱신했다.
- WeaponFire의 현재 실행 owner를 Pawn 직접 실행이 아니라 `ACFVehiclePawn` input/observable/compatibility facade + `UCFVehicleFireComp` 계산·검증·Guidance admission·실행 coordinator 구조로 명확히 했다.
- Runtime behavior, Product Asset, MissileGuide/Flight 계약은 변경하지 않았다.
- Final Audit correction 뒤 Official UE 5.8 Build `437efa728f814c0e85ddfba59163d69c` PASS와 `CarFight.Targeting.Phase7.GuidedWeaponLockedTargetSource` process `6325eb8d13124656b00b6427b40f7042` 1/1 PASS를 재확인했다.

Migration: Phase 7 Current 의미는 v1.52.0과 동일하며 v1.52.1은 실행 owner/RejectReason 문서 정합성 교정이다.

### v1.52.0 - 2026-09-18

- Phase 7 Guided Weapon target source migration Technical PASS를 Current 색인에 반영했다.
- `Targeting/VehicleTargeting.md v1.3.0`, `Combat/WeaponFire.md v1.9.0`, `Combat/Launcher.md v1.1.0`, `Combat/MissileGuidance.md v1.1.0`, `09A_TargetingSensorArch.md v0.1.16`을 최신 Current owner/baseline으로 동기화했다.
- 실제 Projectile Actor + TargetActor Guidance일 때만 Vehicle Locked Target을 발사 전 Guidance source로 요구하고, Targeting 미준비/Idle/Acquiring/invalid Actor는 `GuidanceTargetUnavailable`으로 Projectile acquire와 Ammo reservation 전에 fail-closed하는 경계를 색인에 반영했다.
- HitScan/비유도 Projectile/다른 GuideMode는 새 Lock 요구 없이 기존 Fire 계약을 유지하고, Ripple/Salvo는 첫 승인 발사의 Guidance Actor snapshot을 Volley 전체에 유지하며 Selection/새 Lock 변경으로 자동 retarget하지 않는다.
- Final Official UE 5.8 Build `052b1fc8d0414f6187013655dd19f44d` PASS, Phase 7 exact `731a047740c244e096e43c1f048243a1` 1/1, Launcher Scheduler `1e0ad2dafbee477e83e8bbd07583c951` 1/1, Direct Missile Runtime `fe4e25d60dcb4bfaa752325555bbd362` 1/1 PASS를 최신 evidence로 등록했다.

Migration: Phase 7 이후 TargetActor Guided Projectile의 발사 전 목표 authority는 Vehicle Targeting의 Locked Target이다. TargetSelect는 Selection authority로 남고, MissileGuide는 LaunchContext Snapshot 이후의 Seeker/Guidance만 소유한다. 새 Lock 물리 Input은 여전히 별도 후속 범위다.

### v1.51.0 - 2026-09-18

- Phase 6 HUD Presentation Technical PASS를 Targeting Current 색인에 반영했다.
- `VehicleTargeting.md v1.2.0`의 actor-free `FCFTargetingSnapshot → FCFTargetLockHUDData` Lock Presentation과 `SensorContact.md v1.15.0`의 Selection-independent `FCFSensorSnapshot.ScanAttempt → FCFTargetScanHUDData` 경계를 최신 Current owner로 등록했다.
- Production `WBP_CFTargetPanel`은 기존 Designer Tree를 유지하면서 `Text_TargetLock / ProgressBar_TargetLock` exact2만 additive 추가됐으며 Phase 6 focused Automation 2종이 각각 1/1 PASS했다.
- Phase 7 Guided Weapon source migration은 미착수이며 Launcher/MissileGuide 관련 worktree diff exact0을 확인했다.

Migration: Targeting의 현재 구현 판단은 `Targeting/VehicleTargeting.md v1.2.0`, `Targeting/SensorContact.md v1.15.0`, `09A_TargetingSensorArch.md v0.1.14`과 실제 Source를 우선한다. `Selected == Locked == Scanning`으로 해석하거나 Selection 변경을 자동 Lock/Scan retarget으로 해석하지 않는다.

### v1.50.0 - 2026-09-18

- Phase 5 Selection / Lock / Scan Gameplay Command Technical PASS를 Current Targeting 색인에 반영했다.
- `SensorContact.md v1.14.0`의 Target Scan-only `CancelTargetScan()`과 broad Sensor Operation cancel 분리, `VehicleTargeting.md v1.1.0`의 Pawn Selected Target→Lock/Scan 얇은 command facade를 최신 Current owner로 등록했다.
- Selection 변경/해제가 기존 Lock/Scan을 자동 변경하지 않고, Lock-only clear와 Scan-only cancel이 서로 다른 Runtime을 침범하지 않는 현재 경계를 색인에 명시했다.
- Final evidence는 Official UE 5.8 Build `8c3d5e3eb4d94be88492d4b55e6c6577` PASS, Targeting 5/5, TargetSelect 11/11, Sensor 17/17 PASS다. 새 Lock InputAction/key, HUD Presentation과 Guided Weapon source migration은 아직 미구현이다.

Migration: Targeting의 현재 구현 판단은 `Targeting/VehicleTargeting.md v1.1.0`, `Targeting/SensorContact.md v1.14.0`과 실제 Source를 우선한다. `Selected == Locked == Scanning`으로 해석하거나 Selection 변경을 자동 Lock/Scan retarget으로 해석하지 않는다.

### v1.49.0 - 2026-09-18

- `Document/Systems/Targeting/VehicleTargeting.md v1.0.1`을 Phase 4 Vehicle Target Lock Runtime의 Current System owner로 등록했다.
- TargetSelect의 Selection authority, Sensor의 Contact/Knowledge authority, VehicleTargeting의 Lock state/quality authority를 서로 분리한 현재 책임 경계를 Targeting 색인에 반영했다.
- Phase 4 중간검수 교정 후 Official UE 5.8 Build PASS, `CarFight.Targeting` 4/4, `CarFight.TargetSelect` 11/11, `CarFight.Sensor` 17/17 PASS를 최종 Current evidence로 유지하며 Input/HUD/Guided Weapon source migration은 Phase 5~7 후속으로 둔다.

Migration: Vehicle Target Lock의 현재 구현 판단은 `Targeting/VehicleTargeting.md v1.0.1`과 실제 Source를 우선한다. Selection 또는 Sensor Contact가 존재한다는 이유만으로 Lock이 자동 생성·복구되는 것으로 해석하지 않는다.

### v1.48.0 - 2026-09-17

- `DataManagement/BuilderAuthoringStandard.md v1.0.0`을 CarFight 신규 Builder/Authoring Guide 공통 규약으로 등록했다.
- Vehicle Builder의 검증된 Common Page Shell/USER 정보 계층을 공통 기준으로 승격하고, Builder Chaining 금지, 도메인 완결성, Stable StepId/Conditional Step, 한 Page 하나의 작업 의도, 기존 writer authority 재사용, explicit Apply/Save와 Multi-Asset partial recovery 기준을 색인에 추가했다.
- 기존 Vehicle Builder를 강제 재작성하지 않으며 신규 Builder와 큰 UX 재설계 작업부터 기본 규약으로 적용한다.

### v1.47.0 - 2026-09-15

- `CF-FQ-046 Vehicle Builder 사용자 정보 UX` P0-06 Current System Promotion을 반영해 `Vehicles/VehicleBuilder.md v1.7.0`을 Current owner로 갱신했다.
- VehicleBuilder 색인에 USER 정보 계층, Step 1~8 단일 Common Page Shell/scroll/overflow와 고정 Action/Navigation 접근 계약을 추가했다.
- backend authoring, durable save/Driving, Performance Tuning owner는 기존 Current System 경계를 유지한다.

### v1.46.0 - 2026-09-15

- `CF-FQ-048 Vehicle Pawn Slimming` VPS-P0-00~05 완료와 USER representative regression smoke PASS를 반영해 `Vehicles/VehicleRuntime.md v1.6.0`을 최종 Current owner로 승격했다.
- Pawn의 lifecycle/composition/input/target identity/compatibility facade/state Authority와 Visual/Fire/Runtime coordinator behavior 분리 경계를 색인에 동기화했다. final facade audit의 추가 삭제 대상은 0건이다.
- closure evidence는 P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS와 final affected exact28 `6431180d7e7d4b11ac9f8dc51a25b5c2` 28/28 PASS다. RuntimeRead T0는 UE MCP unavailable로 `Waived / Deferred Observation`이며 미관측 내부값을 PASS로 확대하지 않는다.
- USER smoke 중 확인된 Vehicle Builder 신규 차량의 기본 Sensor/Active Scan baseline 누락은 VPS 회귀가 아닌 별도 Sensor/Builder 설계-구현 불일치로 분리했다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

Migration: VehicleRuntime의 현재 구현 판단은 `Vehicles/VehicleRuntime.md v1.6.0`과 실제 Source를 우선한다. 기존 Blueprint/Product Asset resave는 필요하지 않으며 Sensor/Active Scan baseline correction은 별도 lifecycle에서 다룬다.

### v1.45.0 - 2026-09-14

- `CF-FQ-048 / VPS-P0-04 Debug / Header Cleanup` 완료를 반영해 `Vehicles/VehicleRuntime.md v1.5.0`의 Current type/include ownership 경계를 색인에 동기화했다.
- Vehicle input reflected declaration은 `CFVehicleInputTypes.h`, VehicleDebug reflected declaration은 `CFVehicleDebugTypes.h`가 단일 owner이며 Pawn은 lifecycle/state/Public·BP facade를 유지한다.
- Debug HUD/Panel/VehicleUtils Public consumer가 Pawn 전체 header 대신 DebugTypes를 직접 소비하는 현재 경계를 반영했다. 상세 Build/exact12 evidence는 대표 `VehiclePawnSlimmingPlan.md v0.6.0`이 소유한다.

Migration: VehicleRuntime의 현재 헤더 의존성 판단은 `Vehicles/VehicleRuntime.md v1.5.0`과 실제 Source를 우선한다. 기존 Blueprint/Product Asset에는 migration이나 resave가 필요하지 않다.

### v1.44.0 - 2026-09-14

- `Combat/WeaponData.md v1.2.0`에 자체 제작 Prototype 무기 질량을 `Provisional Gameplay Balance`로 관리하는 Current 정책과 Cannon 300kg / Rocket Launcher 230kg 조합을 동기화했다.
- `Vehicles/RuntimeApply.md v1.1.0`에 Wagon Top Mount에서 두 Product Large 무기의 첫 장착·교체 성공과 비대상 Front Mount `ExplicitEmpty` 보존을 Current evidence로 갱신했다.
- strict GrossMass validator는 유지하며 이전 350+120kg placeholder의 Wagon 6kg 초과 거부는 Historical evidence로 분리했다.
- fresh AssetDump 11/11, UE 5.8 Build `8c957a50fe594132b3448d6f0358eed2` PASS, RuntimeApply `54dbac4be35b45ce914e915002867561` 16/16 PASS를 현재 검색 경계에 반영했다.

Migration: Prototype 무기 질량은 현실 공식 제원으로 해석하지 않고 Current WeaponData의 임시 게임플레이 밸런스 값으로 읽는다. 향후 정식 무기군 밸런싱은 Product DataAsset 값을 조정하되 Fitting/RuntimeApply GrossMass safety contract를 우회하지 않는다.

### v1.43.0 - 2026-09-14

- `Document/Systems/Vehicles/RuntimeApply.md v1.0.0`을 `CF-FQ-041` RuntimeApply의 Current System owner로 등록했다.
- 현재 VehicleDebug/Catalog UI는 non-owning 후보 frontend이고 `FCFRuntimeVehicleApplyService`/`FCFRuntimeEquipApplyService`는 향후 Garage·Inventory에서도 재사용하는 application seam으로 분리했다.
- 2026-09-14 Multi-Mount remediation의 Complete Mount State, 정상 Empty Mount=`ExplicitEmpty`, Legacy→Snapshot 최초 승격, 실제 Applied Snapshot이 있을 때만 기존 선택 보존, GrossMass 검증 비우회 계약을 Current 검색 경계에 승격했다.
- Official UE 5.8 Build PASS 뒤 fresh RuntimeApply Automation 16/16 PASS(Process Job `a96777df47924589a909ddf4cd4d965b`, EngineExitCode=0)를 확보해 Multi-Mount remediation Technical PASS로 닫았다. 과거 14/14 PASS는 Historical evidence로 별도 보존한다.

Migration: RuntimeApply의 현재 backend 계약은 `Vehicles/RuntimeApply.md v1.0.0`과 실제 Source를 우선한다. 향후 Garage/Inventory는 소유권·후보 선택 정책만 추가하고 Fitting Snapshot/Prepare/Commit/Recovery를 별도 구현하지 않는다.

### v1.42.0 - 2026-09-14

- `Document/Systems/Targeting/TargetSelect.md v1.0.0`을 `CF-FQ-026` Rebaseline closure의 Current System owner로 등록했다.
- TargetSelect는 Candidate/Selected Actor authority와 TargetRegistry 후보 공급·선택 수명·입력을 소유하고, Sensor/Scanner는 Detection/Contact/Knowledge, Aim/Fire와 HUD는 소비 경계를 소유하도록 현재 책임을 분리했다.
- old `TS-P0-08 USER PIE`는 current next gate로 재개하지 않는다. USER Inconclusive와 Deferred tuning/observational debt는 TargetSelect Current System에 보존하며 사후 USER PASS로 확대하지 않는다.

Migration: Target selection의 현재 구현 판단은 `Targeting/TargetSelect.md v1.0.0`과 실제 Source를 우선한다. `CF-FQ-026` Historical Plan의 old `TS-P0-08 USER PIE`는 current next gate가 아니며 실제 UX/tuning 요구가 생길 때 별도 lifecycle을 연다.

### v1.41.0 - 2026-09-14

- `CF-FQ-035 Inventory Foundation` Rebaseline closure를 반영해 `Vehicles/VehicleInventory.md v1.0.0`을 신규 Current System으로 등록했다.
- ItemInstance 단일 소유, VehicleCargo/MountedEquipment, Access/Capacity, Reservation, Atomic Transfer/Rollback, read-only ViewData, Inventory→Fitting Adapter와 Field Fit completion/recovery 경계를 Current 검색 경계로 승격했다.
- `CF-FQ-041 RuntimeApply`는 ownership/Reservation/Transaction을 조작하지 않는 즉시 적용·시연용 별도 경로이며 Inventory USER PASS를 대체하지 않는다. 과거 `FFIT-P0-05 Field Fitting UI and PIE`는 USER PASS가 아닌 `Superseded / Not Executed`다.

Migration: Inventory 소유권과 Transfer의 현재 판단은 `Vehicles/VehicleInventory.md v1.0.0`과 실제 Source를 우선한다. 정식 ownership-aware USER frontend가 실제 요구될 때 새 Product/UI lifecycle을 열며 `CF-FQ-035` Historical Plan의 `FFIT-P0-05`를 current next gate로 자동 재개하지 않는다.

### v1.40.0 - 2026-09-14

- `CF-FQ-034` Rebaseline closure를 반영해 `Vehicles/VehicleRuntime.md v1.3.0`을 Fitting/Mass Runtime의 Current owner로 확장했다.
- Prepared Fitting Snapshot의 `TotalVehicleMassKg` pre-physics 적용, BeginPlay configured/actual Mass + Physics State 검증, 같은 Snapshot의 Weapon/Defense Commit 전제, Field Runtime/Mass 원자 적용·rollback을 Current 검색 경계에 추가했다.
- 질량 체감 자체는 Technical Runtime Ready와 분리되며 향후 필요 시 `VehicleBuilder.md v1.6.0` Performance Tuning Protocol이 소유한다. `FIT-P0-07D` USER PASS를 새로 부여하지 않았다.

Migration: Fitting/Mass 현재 구현은 `Vehicles/VehicleRuntime.md v1.3.0`과 실제 Source를 우선한다. `CF-FQ-034` Historical Plan의 old `FIT-P0-07D`는 current next gate가 아니다.

### v1.39.0 - 2026-09-11

- `CF-FQ-015 Vehicle Data Tuning` Rebaseline closure를 반영해 `Vehicles/VehicleBuilder.md v1.6.0`을 성능 튜닝 protocol의 Current owner로 확장했다.
- `Vehicles/VehicleData.md v2.3.0`은 VD-P0-00~03 Historical Validator/Compare/Runtime Apply evidence를 계속 소유하고, VD-P0-04는 USER PASS가 아니라 `Superseded / Not Executed`로 정리했다.
- 제동·Yaw Rate·횡가속·Slip Angle·Suspension 정량값은 현재 구현 완료로 확대하지 않고 VehicleBuilder의 Measurement Gap / future benchmark-extension owner로만 등록했다.

Migration: 새 차량 성능 튜닝은 CF-FQ-015의 Raw Sedan/SUV 절차를 재개하지 않고 `Vehicles/VehicleBuilder.md v1.6.0` protocol을 사용한다. VehicleData의 Validator/Compare는 계속 current foundation이다.

### v1.38.0 - 2026-09-11

- `CF-FQ-038 Vehicle Data Authoring` Done을 반영해 `Vehicles/VehicleBuilder.md v1.5.0`을 Data Authoring Backend + Advanced Workspace까지 포함하는 Current owner로 동기화했다.
- DEL6 closure에서 Legacy Vehicle DA Wizard direct reference exact0과 소스 물리 폐기를 완료했고 final UE 5.8 Build PASS + 전체 `CarFight.DataAuthoring` 111/111 PASS를 확보했다.
- UA-08 quantitative driving comparison은 Runtime Technical PASS / USER Inconclusive 비차단 Deferred로 남기며 P0-12 USER PASS 7/8을 확대하지 않는다.

Migration: 차량 제작·Authoring의 현재 구현 판단은 `Vehicles/VehicleBuilder.md v1.5.0`과 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. Legacy Vehicle DA Wizard 진입점은 더 이상 Current 경로가 아니다.

### v1.37.1 - 2026-09-11

- `CF-FQ-029` closure 문서 정합성을 `Combat/Launcher.md v1.0.1`과 `LauncherMissilePlan.md v0.15.0`으로 동기화했다.
- `plan_repo policy.read_only`를 Plan 텍스트 수정 불가로 해석했던 잘못된 상태를 제거했다. representative Plan은 정상적으로 v0.15.0 Done / Historical + Retained Path까지 갱신됐다.
- Launcher runtime 구현과 Build/Automation evidence는 변경하지 않았다.

### v1.37.0 - 2026-09-11

- `CF-FQ-029 / LM-P0-06 Final Technical Integration` PASS와 `Combat/Launcher.md v1.0.0` Current System Promotion을 index에 추가했다.
- Launcher Current 범위는 Launch Context, 가변 Muzzle, SingleCycle/Ripple/Salvo, Direct/Angled/Vertical Release, Carrier Velocity, 실제 사출 방향 MuzzleBlocked와 Sequence failure/cancel이다.
- 기존 Missile Guidance는 분리 이후 Flight/Guidance를 계속 소유하며 Launcher와 책임을 합치지 않는다.

Migration: CF-FQ-029 완료 이후 런처 현재 구현 판단은 `Combat/Launcher.md v1.0.1`과 실제 Source를 우선한다. 현재 retained Plan은 `LauncherMissilePlan.md v0.15.0`이며, v0.14.0의 Paused/USER PIE Pending은 2026-08-16 완료 전 Historical checkpoint다.

### v1.36.0 - 2026-09-11

- `CF-FQ-053 / VDR-P0-03 Final Acceptance + Process Benchmark` PASS와 `DataManagement/DataAssetAuthoring.md v1.6.0` Current promotion을 index에 동기화했다.
- Current production authoring 범위는 MissileGuidePreset + AmmoData + DamageData + VehicleDefenseData provider/mixed operational admission explicit exact4이며 VehicleDefense는 `ReviewedMutationReady / DACE ContractReady / accepted history exact1 / canonical target exact0`이다.
- CF-FQ-053은 Routine 4-Gate 첫 production 적용에서 `Faster Confirmed`로 종료했다. 5th+ type도 Routine을 기본값으로 사용하며 shared core semantic rewrite가 필요할 때만 Architecture Gap lifecycle을 연다.

Migration: Data Asset Authoring의 현재 구현 판단은 `DataManagement/DataAssetAuthoring.md v1.6.0`과 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. v1.35.1의 provider exact2 설명은 CF-FQ-051 당시 Historical projection이며 현재 지원 범위로 사용하지 않는다.

### v1.35.1 - 2026-09-10

- `CF-FQ-051 / DAO-P0-06 Final Audit Correction + Re-review` PASS에 맞춰 Data Asset Authoring Current owner를 `DataManagement/DataAssetAuthoring.md v1.4.1`로 동기화했다.
- AmmoData는 MissileGuidePreset과 함께 Current Production provider exact2에 포함되고, DamageData/VehicleSensorData/기타 신규 타입만 별도 provider-centric onboarding 대상임을 Current index 의미와 일치시켰다.
- Source/Asset/Test 계약은 변경하지 않았고 hybrid shared/legacy physical ownership P2는 non-blocking maintenance debt로 유지한다.

Migration: CF-FQ-051 완료 이후 Data Asset Authoring의 현재 구현 판단은 `DataManagement/DataAssetAuthoring.md v1.4.1`과 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. v1.35.0의 v1.4.0 포인터는 당시 Systems Promotion 기록이며 새 DataAsset 타입은 별도 lifecycle에서 onboarding한다.

### v1.35.0 - 2026-09-10

- `CF-FQ-051 / DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion` 완료를 반영해 `DataManagement/DataAssetAuthoring.md v1.4.0`을 Current index에 동기화했다.
- MissileGuidePreset+AmmoData Production provider exact2, provider-neutral mixed Explicit Paths와 shared Preview/Review/TOCTOU/Apply/Durable/DACE core를 Current 검색 경계에 추가했다. Third type는 typed provider/DACE + production provider registration + explicit operational admission으로 확장하며 shared core algorithm rewrite는 요구하지 않는다.
- DAO-P0-06 사전검수는 P0 0 / blocking P1 0 / P2 1 non-blocking PASS다. P2는 generic implementation과 legacy Missile compatibility의 물리적 owner 혼재이며 실제 third onboarding 반복 비용이 확인될 때만 별도 maintenance를 검토한다.
- DAO-P0-06 executable Source/Asset mutation은 0이며 Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10과 single Active `CF-FQ-039`를 유지한다.

Migration: CF-FQ-051 완료 이후 Data Asset Authoring의 현재 구현 판단은 `DataManagement/DataAssetAuthoring.md v1.4.0`과 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. 새 DataAsset 타입은 완료된 CF-FQ-051 Plan의 old gate를 재사용하지 않고 별도 lifecycle에서 provider-centric onboarding을 연다.

### v1.34.0 - 2026-09-09

- `CF-FQ-050 / DACE-P0-06 Final Acceptance` PASS와 Current System Promotion을 반영해 `DataManagement/DataAssetAuthoring.md`를 v1.1.0으로 전진했다.
- 기존 CF-FQ-049 Staging/Reviewed Apply owner에 MissileGuidePreset Source/Adapter/Mapping/Semantic descriptor, production behavior drift guard, Schema/Adapter revision guard, migration Resolution/Evidence와 append-only accepted snapshot promotion gate를 통합했다.
- CF-FQ-050 closure 기준 Product Low/Normal/High Apply·Save 0 / canonical Product Staging mutation 0 / accepted snapshot append 0이며 현재 single Active `CF-FQ-039`는 변경하지 않았다.

Migration: Staging 지원 DataAsset의 C++ contract가 바뀌면 `DataAssetAuthoring.md v1.1.0`의 Contract Evolution Guard 절차를 따른다. 새 DA 타입은 자동 지원되지 않으며 별도 Typed Adapter/Schema/descriptor/probe lifecycle이 필요하다.

### v1.33.1 - 2026-09-09

- `DataManagement/DataAssetAuthoring.md`를 v1.0.1로 전진해 AdapterContractRevision 2의 FText persistence metadata 예외를 Current Source와 정확히 정렬했다.
- Unreal이 저장 과정에서 자동 부여한 package-only namespace / stable key는 authored semantic identity에서 제외하고, StringTable·명시적 authored namespace·lossless source string이 없는 generated/formatted FText만 fail-closed한다.
- Source/Asset/Build/Automation 변경은 없으며 CF-FQ-049 Done, Product Low/Normal/High Apply·Save 0, CF-FQ-039 Active는 그대로다.

Migration: Data Asset Authoring의 현재 FText 계약은 `DataAssetAuthoring.md v1.0.1`을 따른다. v1.0.0의 포괄적인 namespace-key 차단 표현은 현재 판단에 사용하지 않는다.

### v1.33.0 - 2026-09-09

- `CF-FQ-049 / DAS-P0-05 Final Acceptance` PASS와 Current System Promotion을 반영해 `DataManagement/DataAssetAuthoring.md v1.0.0`을 신규 등록했다.
- Editor-off canonical JSON, exact selection Preview→fresh Review→explicit ApplyReviewed, dirty/stale/conflict fail-closed, typed exact-package SavePackage + disk reload semantic readback을 현재 DataAsset Authoring 계약으로 승격했다.
- closure 기준 Product Low/Normal/High Apply·Save는 0이며 fresh persisted acceptance audit에서 `CFMissileGuidePresetData` 3개를 확인했다. CF-FQ-045 Data Asset Manager read-first owner와 CF-FQ-039 Active lifecycle은 변경하지 않았다.

Migration: CF-FQ-049 완료 이후 Staging/Batch Apply 현재 구현 판단은 `DataManagement/DataAssetAuthoring.md`와 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. Data Asset 발견·검사·참조 조회는 계속 `DataAssetManagement.md`가 소유한다.

### v1.32.0 - 2026-09-08

- `CF-FQ-030` P0 Current System Promotion으로 `Combat/MissileGuidance.md v1.0.0`을 신규 등록했다.
- `Projectile.md v1.9.0`의 공통 Actor/Missile 컴포넌트 통합 경계와 Guidance 상세 owner 분리를 색인에 반영했다.
- CF-TC-027 Technical + USER Guidance Feel PASS를 현재 Missile Guidance 탐색 경로로 승격했다.
- Source/Asset mutation과 Build/Automation/AssetDump 재실행 없이 Current 문서 projection만 갱신했다.

### v1.31.0 - 2026-09-05

- CF-FQ-047 post-closure final audit remediation을 반영해 Current owner를 `Vehicles/VehicleBuilder.md v1.4.1`로 동기화했다.
- Step 7 exact package save는 raw `SavePackage` 성공뿐 아니라 package clean + persisted 확인까지 필요하며 확인 실패는 `SaveStateUnconfirmed`, durable write 뒤 refresh-only failure는 `CommittedRefreshWarning`으로 분리하는 현재 계약을 검색 설명에 반영했다.
- guarded Undo가 실제로 되돌린 Target/Recipe package를 `Target → Recipe` 순서로 durable 저장하고 partial failure에서 자동 rollback/retry하지 않는 현재 계약을 추가했다. CF-FQ-047 lifecycle은 Done/Historical 그대로이며 Product Asset migration은 없다.

Migration: Vehicle Builder 현재 구현 판단은 `Vehicles/VehicleBuilder.md v1.4.1`과 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. post-closure remediation은 저장 완료/실패 판정과 회귀 증거를 강화한 것이며 기존 Wagon 2/2 및 USER Driving acceptance를 재승인하거나 migration하지 않는다.

### v1.30.0 - 2026-09-05

- `CF-FQ-047 / VBHAI-P0-08 Current System Promotion`을 반영해 `Vehicles/VehicleBuilder.md v1.4.0`을 Current index에 동기화했다.
- Hardpoint/Mount/Socket authoring integrity, Step 5 Physics impact boundary, Step 7 exact Target→Recipe durable final commit와 partial-save/fresh-restart recovery, Step 8 fresh saved-handoff readiness/persistent USER Driving receipt를 Vehicle Builder 검색 설명에 추가했다.
- CF-FQ-047 final-audit remediation은 Current 계약을 바꾸는 신규 기능이 아니라 durable persistence failure taxonomy와 Automation coverage를 보강하는 post-closure correction이다.

Migration: Vehicle Builder 현재 구현 판단은 `Vehicles/VehicleBuilder.md v1.4.0`과 실제 `CarFight_ReEditor/DataAuthoring` Source를 우선한다. CF-FQ-047 당시 상세 Build/Automation/USER evidence는 Historical Plan이 보존하며 Product Asset migration은 없다.

### v1.29.0 - 2026-09-04

- `CF-FQ-045 / DAM-P0-04E USER Acceptance PASS`와 P0 완료를 Current System으로 승격해 `DataManagement/DataAssetManagement.md v1.0.0`을 신규 등록했다.
- native/persisted 자동 discovery, semantic Registry, metadata-only Refresh, explicit 검사/참조 조회, Refresh generation에 따른 `재검사 필요 / 재확인 필요`, 한글 우선 Manager UX와 no-auto-mutation/save 경계를 기능 색인에 추가했다.
- 완료 당시 Build·Automation·USER feedback은 `Document/Plan/DataAssetManagement/DataAssetManagementPlan.md v0.2.0` Historical + Retained Path가 보존하며 residual P2 2건은 현재 시스템을 재오픈하지 않는 별도 폴리싱 후보로 남긴다.

Migration: Data Asset Manager의 현재 구현 판단은 `DataManagement/DataAssetManagement.md`와 실제 `CarFight_ReEditor/DataManagement` Source를 우선한다. Runtime/Content/Product Asset migration은 없다.

### v1.28.0 - 2026-09-03

- `CF-FQ-044 / VRCP-P0-06 Current System Promotion`을 반영해 `Vehicles/VehicleBuilder.md v1.3.0`을 Current index에 동기화했다.
- persistent USER Driving PASS 뒤 Default Runtime Catalog promotion, idempotent exact membership, dirty/no-auto-save, explicit retry와 same-session RuntimeApply sync 경계를 검색 설명에 추가했다.
- RuntimeApply authorization/apply owner는 계속 CF-FQ-041이며 Builder는 Catalog 등록 orchestration만 소유한다.

### v1.27.0 - 2026-09-03

- `CF-FQ-043 / VMG-P0-08 Current System Promotion`을 반영해 `Vehicles/VehicleBuilder.md v1.2.0`을 Current index에 동기화했다.
- Hardpoint Plan Mode, Standard Hardpoint/Mount, unified Chassis Socket edit/copy/explicit add, Legacy preservation과 persistent Target DefinitionHash USER Driving acceptance를 검색 설명에 추가했다.

### v1.26.1 - 2026-09-02

- `Vehicles/VehicleBuilder.md v1.1.1` final audit correction을 반영해 신규 차량 Creation Entry의 Browser refresh-safe selection 계약을 Current index에 동기화했다.

### v1.26.0 - 2026-09-02

- `CF-FQ-042` 완료 승격을 반영해 `Vehicles/VehicleBuilder.md v1.1.0`의 신규 차량 Creation Entry를 Current index에 추가했다.
- Blank/Unused/Reused Chassis 생성, Vehicle ID naming, Recipe-only initial Chassis intent와 no-auto-save/no-auto-apply 계약을 Vehicle Builder 검색 설명에 포함했다.

### v1.25.0 - 2026-09-02

- `CF-FQ-040 / VB-P0-10 Current System Promotion` 완료를 반영해 `Vehicles/VehicleBuilder.md v1.0.0`을 Guided Vehicle Builder Current owner로 신규 등록했다.
- 정상 신규 차량 제작은 Vehicle Builder, 공통 Recipe/Resolver/Diff/Validation/Apply/Undo/Drift 엔진과 전문가용 화면은 Data Authoring Backend + Advanced Workspace, Runtime 최종 정의는 VehicleData가 소유하는 현재 경계를 색인에 반영했다.
- VB-P0-09 USER Acceptance와 ESH Final Audit Clean PASS를 완료 기준선으로 연결하되 CF-FQ-038 자체 잔여 lifecycle이나 Wagon tuning을 다시 열지 않는다.

### v1.24.0 - 2026-08-24

- `UI/InGameUI.md v1.1.1`의 CF-FQ-039 VehiclePanel persisted editable Structure Readiness 감사를 Current UI owner 설명에 반영했다.
- 신규 기능 승격이 아니라 기존 `WBP_CFArmorSector` 6개 재사용 + BodyMap Canvas + 탑다운 VehicleSilhouette의 실제 persisted 구조를 문서화한 변경이다.
- USER Production Visual PASS와 CF-FQ-039 완료 상태는 변경하지 않았다.

### v1.23.0 - 2026-08-22

- 문서 물리 정리 closure를 반영해 Decommissioned `ServerSpawn`과 레거시 `VehicleDebug`를 Current Systems 색인에서 내리고 `ProjectSSOT/Archive/Systems/` Historical 경로로 연결했다.
- 물리 Archive로 이동된 완료 Plan의 설계·검증 참조를 `Document/Plan/Archive/` 실제 경로로 정상화했다.
- Current 구현 owner, PASS evidence와 USER 판정은 변경하지 않았다.

Migration: 현재 구현은 계속 `Document/Systems/`와 실제 Source/Asset을 우선한다. 과거 완료 설계·검증 또는 내려온 시스템 기준선이 필요할 때만 각 Archive 경로를 선택한다.

### v1.22.0 - 2026-08-22

- CF-FQ-032 post-closure remediation 완료 후 Current owner 버전을 `UI/InGameUI.md v1.1.0`, `UI/AimReticle.md v1.10.0`, `Targeting/SensorContact.md v1.2.0`으로 동기화했다.
- CF-FQ-032 Done과 USER Visual/Zoom Feel Deferred 상태는 변경하지 않았다.

Migration: CF-FQ-032 현재 구현은 최신 Systems와 실제 Source/Asset을 우선한다.

### v1.21.0 - 2026-08-22

- `CF-FQ-032 / UI-P0-11 Systems Promotion`을 반영해 `Document/Systems/UI/InGameUI.md v1.1.0`을 인게임 UI Current System으로 신규 등록했다.
- LocalPlayer `UCFUISubsystem → UCFUIRootWidget` 수명, Production `UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter` 경계, 실제 Pause·HUD·Radar·Target·Weapon 계약과 UI-P0-10 AI Runtime Technical Validation을 Current 지식으로 승격했다.
- `AimReticle.md v1.10.0`의 생성·수명 owner를 실제 Source에 맞춰 `UCFUISubsystem` HUD Layer singleton + Current Pawn Rebind로 교정했다. 기존 Aim/FireFeedback/Turret Reticle USER PASS 의미는 변경하지 않았다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 Deferred/Pending으로 유지하며 Current Systems 등록을 USER Visual PASS로 확대하지 않는다.

Migration: CF-FQ-032의 현재 구현 판단은 `UI/InGameUI.md`와 실제 Source/Asset을 우선한다. `UI/AimReticle.md`는 Aim/FireFeedback 상세 의미 owner로 유지하며 과거 Pawn direct AddToViewport 설명을 Current 생성 경로로 사용하지 않는다.

### v1.20.0 - 2026-08-18

- `CF-FQ-037 SCAN-P0-00~07` 완료를 반영해 `SensorContact.md v1.1.0`을 Scanner Utility 장비/Fitting Source와 Pawn-owned V Active Scan 입력까지 포함하는 Current System으로 갱신했다.
- SCAN-P0-06 USER PIE에서 5초 timed scan, 반복 입력 무연장과 Target Knowledge `???` 해제를 확인했고, 임시 관측 코드를 제거한 최종 Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 closure evidence로 연결했다.
- Radar Range/Zoom·동적 Blip·CF-FQ-032 USER Visual과 Sensor energy/heat·AI/Network는 완료 범위에 포함하지 않았다.

### v1.19.0 - 2026-08-15


- `Document/Systems/Targeting/SensorContact.md v1.0.0`을 `CF-FQ-036 SEN-P0-00~07` Technical Acceptance PASS의 Current System으로 신규 등록했다.
- Sensor의 bounded Passive/Visual/Active Detection, Contact lifecycle, Tactical Analysis·Knowledge, VehicleHealth authoritative DestroyedHold와 actor-free Snapshot 계약을 Targeting 색인에 추가했다.
- TargetSelect는 후보 검색·선택·TrackState owner로 유지하고 HUD는 Sensor Snapshot을 read-only 소비하며 Radar Range/Zoom·동적 Blip·CF-FQ-032 USER Visual은 완료로 해석하지 않는 책임 경계를 명시했다.
- 현재 기술 기준선은 Build `7dff9da7aaa24e76b0871348762c9d93` PASS, `CarFight.Sensor` `85d008613a104df9b7107795995c6ac5` 14/14 PASS다.

### v1.18.0 - 2026-08-15

- `Document/Systems/Combat/WeaponData.md v1.0.0`을 CF-FQ-008 Current System으로 신규 등록했다.
- WeaponData의 정적 SSOT, DataValidation, Launcher·Ammo·Projectile/Fitting 소비 경계와 legacy/fallback 계약을 Combat 색인에 추가했다.
- WeaponFire가 Ammo Runtime 상태를 직접 소유하지 않고 `UCFVehicleAmmoComp`가 Loaded·Reserve·Reload를 소유하는 현재 책임 경계를 색인 설명에 반영했다.

### v1.17.0 - 2026-08-15

- `Document/Systems/Vehicles/VehicleData.md`를 v2.0.0으로 전면 교정해 DA_PoliceCar 중심 구형 설명을 현재 `DA_TestSedan / DA_TestSUV` 기준으로 갱신했다.
- Layout·Hardpoint·MountProfile·Fitting Mass까지 확장된 현재 VehicleData 계약과 `bUseMovementOverrides` 실제 런타임 의미를 반영했다.
- CF-FQ-015 VD-P0-00~03의 Build·VehicleData Automation 3/3 Technical PASS를 기록하되 실제 주행감 USER 튜닝은 미완료 상태로 유지했다.

### v1.16.0 - 2026-08-13

- `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System으로 신규 등록했다.
- `CF-FQ-031`의 AMMO-P0-00~08, 공식 Build·Automation과 Heavy·Ripple USER PIE PASS를 완료 근거로 반영했다.
- Vehicle finite Ammo의 WeaponInstanceId별 Loaded, AmmoId별 Reserve, Launcher Sequence 예약, FullMagazine Reload, WeaponPanel 표시와 Fitting Ammo 질량을 Combat 현재 범위에 추가했다.
- Launcher 전체 완료 여부는 별도 `CF-FQ-029`가 소유하므로 Ammo 승격과 함께 Launcher를 자동 완료 처리하지 않는다.

### v1.15.0 - 2026-08-06

- `Document/Systems/Combat/VehicleDefense.md v1.0.0`을 Current System으로 신규 등록했다.
- Combat 현재 범위를 Shield, 6방향 독립 Armor, ArmorPenetration과 Vehicle Integrity까지 확장해 실제 구현과 일치시켰다.
- `HitDamage.md v1.1.0`의 정식 VehicleDefense 진입점, Legacy Fallback과 기존 Health·Pool 호환 관계를 색인에 반영했다.
- 공식 Build `48c0a81e19af4b20a17f628bcc7b723b`, Combat Automation 46/46과 DR-PIE-00~06 USER PASS를 `CF-FQ-033 Done` 근거로 등록했다.

### v1.14.0 - 2026-07-30

- `Document/Systems/Combat/Projectile.md`를 v1.5.0으로 갱신해 `CF-FQ-027 투사체 비행 FX` 완료 상태를 반영했다.
- Trail-only, Thruster-only, Trail+Thruster, 유효 소켓·Missing Socket Fallback 사용자 PIE PASS를 색인에 추가했다.
- Hit·LifeExpired Reset, Pool 20발 이상, Ribbon History 무잔류와 30 FPS 고속 Bounds PASS를 반영했다.
- `CF-FQ-027 Done / CF-TC-023 PASS`를 Current System으로 등록했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run이며 Runner 미노출 상태를 유지했다.

### v1.13.0 - 2026-07-28

- `Document/Systems/Combat/Projectile.md`를 v1.4.0으로 갱신해 `CF-FQ-028 발사체 추진 시스템`을 기존 Projectile Current System에 통합했다.
- `UCFProjectileMotorComp`, 점화 지연, Burning 고정 방향 가속, MaximumPropelledSpeed와 BurnedOut 관성 비행을 색인 설명에 추가했다.
- Trail·Thruster Origin/Niagara, 메시 소켓/Fallback, Pool Reset과 `RelativeTransform.Scale` 독립 FX Scale 계약을 추가했다.
- 최종 Build Job `e8b812bd479549299dd116f9bae8996f` Exit Code 0과 사용자 PIE Scale 0.2 정상 동작을 반영했다.
- `CF-FQ-028 Done / CF-TC-024 PASS`를 등록하고 반복 전투 확장 회귀를 `CF-FQ-019`로 이관했다.
- `CF-FQ-027` 별도 전체 체크리스트와 `CF-TC-023`은 자동 완료로 해석하지 않도록 유지했다.

### v1.12.0 - 2026-07-27

- `Document/Systems/Combat/CombatFx.md`를 Current System으로 등록했다.
- `CF-FQ-024 Done`, `CF-TC-021 PASS`와 최종 사용자 PIE 전체 PASS를 반영했다.
- `NS_BasicHit` Impact 현재 크기 승인과 차량별 `SM_Body.FX_Destroyed` 소켓 위치 기준을 색인에 추가했다.
- 기능별 찾기 표에 데이터 기반 Muzzle·Impact·Destroyed Niagara 런타임 문서를 연결했다.

### v1.11.0 - 2026-07-22

- `CF-FQ-025` Done과 `CF-TC-022` 사용자 PIE PASS를 반영했다.
- `AimReticle` 색인에 Image_CenterDot 조준 레티클과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클 책임을 추가했다.
- `VehicleAim` 색인에 터렛 레티클 월드 지점 제공과 기존 정렬·MuzzleBlocked 회귀 검증 상태를 반영했다.
- 기능별 현재 구현 설명을 이중 레티클 완료 기준에 맞게 갱신했다.

### v1.10.0 - 2026-07-15

- `CF-FQ-017` Done과 `CF-TC-014` 사용자 PIE PASS를 반영했다.
- `FireFeedback` 색인에 NoWeapon, AimBlocked, 피드백 만료 검증 완료를 추가했다.
- `AimReticle` 색인에 실패 상태 색상과 정상 상태 복귀 검증 완료를 추가했다.

### v1.9.0 - 2026-07-15

- `Document/Systems/Combat/HitDamage.md`를 Current System으로 등록했다.
- `CF-FQ-018` 최소 Damage Runtime의 공식 빌드와 사용자 PIE PASS를 반영했다.
- DamageHitContext를 실제 피해 적용 입력으로 사용하는 현재 관계를 색인에 반영했다.
- 장갑·모듈 피해와 완성형 파괴 연출은 후속 범위로 유지했다.

### v1.8.0 - 2026-07-14

- `VehicleAim`, `AimReticle`, `Projectile`, `DamageHitContext` 색인 설명을 현재 P0 사용자 PIE 완료 상태로 갱신했다.
- `CF-FQ-022` 정렬 중 발사 정책 true/false, 정렬 완료 탄착과 `MuzzleBlocked` 검증 완료를 반영했다.
- `CF-FQ-023` 고속 Projectile 집중 스트레스 검증 완료를 반영했다.
- 실제 HP 차감과 파괴 상태는 여전히 `CF-FQ-018` 미구현 범위로 유지했다.

### v1.7.0 - 2026-07-14

- Projectile 고속 연속 충돌 상태를 코드 구현·공식 Editor 빌드 완료, 사용자 PIE 검증 Pending으로 정정했다.
- `DamageHitContext` 설명에 고속 Projectile 경로 연결과 사용자 PIE 신뢰성 검증 대기를 분리해 반영했다.
- 오래된 `고속 터널링 미완료 한계` 표현을 제거하고 구현 상태와 검증 상태를 구분했다.
- 실제 피해 적용은 여전히 후속 범위임을 유지했다.

### v1.6.0 - 2026-07-13

- `WeaponFire.md`, `FireFeedback.md`, `AimReticle.md`, `VehicleAim.md` 설명을 AimFireAlignment 빌드 완료 / PIE Pending 상태로 갱신했다.
- Weapon Aim Solution, TurretAligning, MuzzleBlocked 표시 연결 상태를 색인 설명에 반영했다.

### v1.5.0 - 2026-07-13

- `VehicleAim.md`와 `AimReticle.md` 설명에 Reticle 목표와 Muzzle 발사 방향 정렬 한계를 반영했다.
- `Projectile.md`와 `DamageHitContext.md` 설명을 시각 차체 피격 완료 / 고속 Projectile 부분 완료 상태로 갱신했다.
- `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md`를 조준 해 통합 설계의 Historical evidence로 연결했다.
- `Document/Plan/Archive/ProjectileContinuousCollision/ImplementationDesign.md`를 고속 Projectile 연속 충돌 설계의 Historical evidence로 연결했다.
- `HitDamage` Plan 설명을 시각 피격 구현 완료 이후 Damage Runtime 대기 상태로 정정했다.

### v1.4.0 - 2026-07-13

- `DamageHitContext.md` 설명에 현재 차량 피격이 `VehicleMesh` Physics Asset 기준이라는 한계를 반영했다.
- 시각 차체 기반 피격 콜리전 분리와 피해 처리 완료 설계 기록 `Document/Plan/Archive/HitDamage/ImplementationDesign.md`를 Historical evidence로 연결했다.
- `SM_Body` 기반 무기 피격 전환은 아직 Systems 완료 기능이 아니라 Plan 단계임을 유지했다.

### v1.3.0 - 2026-07-09

- `Document/Systems/Combat/FireFeedback.md` 신규 문서를 Combat 폴더 색인에 추가했다.
- 기능별 찾기 표에 발사 성공/실패/쿨다운/무기 없음 상태를 표시하는 기준 문서로 `Combat/FireFeedback.md`를 추가했다.
- `Vehicles/VehicleAim.md` 설명에서 서버 검증 / 복제 시각화 표현을 제거하고 로컬 발사 검증 상태 / 로컬 시각화 상태 기준으로 정정했다.
- `UI/AimReticle.md`, `Combat/WeaponFire.md` 설명을 Reticle / FireFeedback 교통정리 이후 기준에 맞춰 보강했다.

### v1.2.0 - 2026-07-09

- `Document/Systems/Combat/` 섹션을 추가했다.
- 현재 구현된 전투 기능 문서 `WeaponFire.md`, `Projectile.md`, `DamageHitContext.md`를 색인에 등록했다.
- 아직 구현 기준으로 확인되지 않은 HP 차감, 파괴, 장갑/모듈 피해, 서버 권한 전투는 Combat Systems 현재 범위에 포함하지 않는다고 명시했다.

### v1.1.0 - 2026-06-19

- 싱글 플레이 1대 차량 고도화 전환 기준에 맞춰 Network 폴더를 Deferred 기록으로 표시했다.
- 기능별 찾기 표에서 서버 Spawn/Possess 항목이 현재 활성 작업처럼 보이지 않게 정리했다.

### v1.0.0 - 2026-06-02

- `Document/Systems/` 하위 문서 위치 안내 색인 문서로 최초 작성했다.
- 각 폴더별 문서 경로와 문서가 다루는 기능을 표로 정리했다.
- 기능별로 어떤 문서를 열면 되는지 찾기 표를 추가했다.

---

## 14. Migration

### v1.19.0 적용 안내

- `CF-FQ-036` 완료 이후 Sensor/Contact 현재 구현은 `Targeting/SensorContact.md v1.0.0`과 실제 Source를 우선한다.
- `Document/Plan/Archive/SensorContactPlan.md`는 완료 당시 설계·검증 evidence를 보존하는 Historical + Archived Path로 읽는다.
- TargetSelect 후보 검색·선택 수명과 Sensor Contact lifecycle·Knowledge를 합치지 않는다.
- Radar Range/Zoom·NormalizedPosition·동적 Blip 및 CF-FQ-032 Target/Radar USER Visual은 이 Current System 승격으로 자동 완료되지 않는다.
- CF-FQ-026 TS-P0-08 USER PIE도 별도 Pending 상태를 유지한다.

### v1.18.0 적용 안내

- `CF-FQ-008` 완료 이후 WeaponData 현재 구현은 `Combat/WeaponData.md`와 실제 `UCFWeaponData` 코드를 우선한다.
- `Document/Plan/Archive/WeaponDataPlan.md`는 완료 당시 설계·검증 기록인 Historical + Archived Path로 읽는다.
- `MagazineSize`와 `ReloadTimeSeconds`는 현재 Ammo Runtime의 정적 입력이며, Loaded·Reserve·Reload 진행 상태는 `UCFVehicleAmmoComp`가 소유한다.
- `HeatPerShot / MaxHeat`는 현재 과열 Runtime 완료를 의미하지 않는다.

### v1.15.0 적용 안내

- 차량 방어·손상 현재 구현은 `Combat/VehicleDefense.md`와 `Combat/HitDamage.md`를 함께 우선한다.
- `VehicleDefenseDamageDesign.md`는 완료 당시 설계·검증 기록이며 Current System을 대체하지 않는다.
- `VehicleData.DefaultDefenseData=None`은 오류가 아니라 기존 차량을 위한 정식 Legacy Fallback으로 읽는다.
- 실제 부품 손상, 도탄, 범위 피해와 서버 권한 피해는 Current System으로 간주하지 않는다.

### v1.14.0 적용 안내

- `CF-FQ-027`의 현재 구현 판단은 `Document/Systems/Combat/Projectile.md v1.5.0`을 우선한다.
- `Document/Plan/Archive/ProjectileFlightFx/ProjectileFlightFxPlan.md v1.0.0`은 완료 당시 설계·빌드·사용자 PIE 기록으로 유지한다.
- `CF-TC-023`은 PASS이며 CF-FQ-027을 Active 또는 Paused로 복원하지 않는다.
- Automation 실행은 Runner 미노출로 Not Run 상태를 유지한다.

### v1.13.0 적용 안내

- `CF-FQ-028`의 현재 구현 판단은 `Document/Systems/Combat/Projectile.md`를 우선한다.
- `Document/Plan/Archive/ProjectilePropulsionPlan.md`는 완료 당시 설계·빌드·PIE 체크포인트로 유지한다.
- `CF-TC-024`는 PASS이며 비유도 Rocket 추진과 Burning 기반 Thruster는 Current System이다.
- 반복 전투 확장 회귀는 `CF-FQ-019`가 소유하며 자동 착수하지 않는다.
- `CF-FQ-027`과 `CF-TC-023`의 별도 전체 Trail·Fallback·Pool 검증은 Paused 상태를 유지한다.

### v1.12.0 적용 안내

- `CF-FQ-024`의 현재 구현 판단은 `Document/Systems/Combat/CombatFx.md`를 우선한다.
- `Document/Plan/Archive/CombatFxAudio/ImplementationDesign.md`는 완료 당시 설계와 검증 기록으로 유지한다.
- `CF-TC-021`은 PASS이며 CombatFx는 P0 사용자 PIE 완료된 Current System이다.

### v1.10.0 적용 안내

- `CF-FQ-017`의 현재 구현 판단은 `Document/Systems/Combat/FireFeedback.md`와 `Document/Systems/UI/AimReticle.md`를 우선한다.
- `Document/Plan/Archive/ReticleFireFeedback/ImplementationDesign.md`는 완료 당시 설계와 검증 체크포인트 보존용으로 유지한다.
- NoWeapon, AimBlocked와 정상 발사 회귀는 P0 사용자 PIE 완료 상태로 읽는다.

### v1.9.0 적용 안내

- `CF-FQ-018`의 현재 구현 판단은 `Document/Systems/Combat/HitDamage.md`를 우선한다.
- `Document/Plan/Archive/HitDamage/ImplementationDesign.md`는 완료 당시 설계와 검증 체크포인트 보존용으로 유지한다.
- 최소 Damage Runtime은 Current System이지만 파괴 시 입력·물리 정지와 장갑·모듈 피해는 구현된 것으로 간주하지 않는다.

### v1.8.0 적용 안내

- `CF-FQ-022`와 `CF-FQ-023`은 P0 사용자 PIE까지 완료된 Current System 기준으로 읽는다.
- AimFireAlignment와 ProjectileContinuousCollision Plan은 완료 체크포인트 보존용이며 현재 구현 판단은 관련 Systems 문서를 우선한다.
- 실제 `BaseDamage`, 체력 감소와 파괴 상태의 완료 당시 설계 evidence는 `Document/Plan/Archive/HitDamage/ImplementationDesign.md`의 `CF-FQ-018` 기록에 보존한다.

### v1.7.0 적용 안내

- 기존 Systems 문서 경로와 책임은 변경하지 않는다.
- 고속 연속 충돌 코드와 공식 Editor 빌드 완료 사실은 색인에 반영하되, 사용자 PIE 검증 전에는 검증 완료된 Current System 동작으로 승격하지 않는다.
- 일반 속도 `SM_Body` 충돌의 기존 사용자 PIE 확인 결과는 유지한다.
- 고속 Projectile 신뢰성 검증과 실제 피해 적용은 계속 Pending으로 구분한다.
