# Plan Archive Index (CarFight)

- 문서 버전: v1.32
- 작성일: 2026-07-14
- 최근 갱신일: 2026-10-02
- 문서 상태: Current

- 역할: 현재 착수 기준에서 내려온 CarFight Plan의 logical Historical 색인과 physical placement를 관리한다.

---

## 1. 목적

Archive는 과거 기록을 삭제하는 장소가 아니라 **현재 착수 기준에서 내려온 Plan을 선택적으로 찾는 경로**다.

```text
현재 우선순위 = Document/ProjectSSOT/
현재 구현 = Document/Systems/ + 실제 코드·에셋
현재 작업 = Document/ActiveWork.md
현재 Plan = Document/Plan/README.md
Historical Plan = 이 문서
```

Historical과 물리 위치는 같은 개념이 아니다.

```text
semantic lifecycle = Active/In Progress -> Closed -> Historical
physical placement = Retained Path | Move Candidate | Archived Path
```

Historical 판정은 먼저 semantic lifecycle로 결정하고, 물리 이동은 링크·dirty work·rollback 안전성이 확인된 maintenance에서만 수행한다.

---

## 2. 읽기 규칙

Historical Plan은 다음 경우에만 선택해서 읽는다.

- 과거 Acceptance·설계 결정·Migration 근거 확인
- 현재 문서와 과거 결정 충돌 조사
- regression 유래 추적
- 완료 또는 폐기된 기능을 새 lifecycle로 재검토

금지:

```text
Historical 문서의 old next action을 현재 작업으로 사용
과거 Pending/Blocked 상태를 Current에 자동 복원
완료된 USER PIE/Automation을 trigger 없이 반복
Archive에 있다는 이유로 Current 구현 기준으로 해석
```

---

## 3. Archived Path

### 3.1 기존 Archived Path 폴더

| 폴더 | semantic 분류 | placement | 보관 이유 | 대표 진입 문서 |
| --- | --- | --- | --- | --- |
| `CFNetSmoothPlan/` | Deferred / Historical | Archived Path | 현재 싱글 일정에서 제외된 네트워크 Transform 스무딩 계획 | `CFNetSmoothPlan/README.md` |
| `InputPlan/` | Merged / Historical | Archived Path | 현재 입력 기준이 `Systems/Input/Input.md`로 승격됨 | `InputPlan/README.md` |
| `MP_ServerPlan/` | Deferred / Historical | Archived Path | Dedicated Server·2클라 계획이 현재 일정에서 제외됨 | `MP_ServerPlan/README.md` |
| `Plan_UECfgFix/` | Completed / Historical | Archived Path | 과거 UE 설정 수정 작업 기록 | `Plan_UECfgFix/Patch_UECfgFix.md` |
| `ServerUpgradePlan/` | Deferred / Historical | Archived Path | 서버 안정화·네트워크 물리 계획이 현재 일정에서 제외됨 | `ServerUpgradePlan/README.md` |
| `SteeringPlan/` | Merged / Historical | Archived Path | 현재 조향 기준이 `Systems/Vehicles/VehicleSteering.md`로 승격됨 | `SteeringPlan/README.md` |
| `ToCPP/` | Historical | Archived Path | 기존 BP 구조의 C++ 전환 과도기 기록 | `ToCPP/CarFight_CPP_RePlan.md` |

### 3.2 2026-08-22 physical move 완료 항목

아래 항목은 기존 `Historical + Retained Path`에서 **Historical + Archived Path**로 물리 정리됐다. 완료 당시 Plan·TaskSource·Generated evidence는 삭제하지 않고 같은 기능 묶음으로 보존한다.

| Archived Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 |
| --- | --- | --- | --- | --- |
| `InGameUIPlan.md` / `InGameUIRoadmap.md` | `CF-FQ-032` | Done -> Historical | `Systems/UI/InGameUI.md v1.1.0`, `Systems/UI/AimReticle.md v1.10.0`, `Systems/Targeting/SensorContact.md v1.2.0` | `InGameUIPlan.md` |
| `SensorContactPlan.md` | `CF-FQ-036` | Done -> Historical | `Systems/Targeting/SensorContact.md v1.2.0` | `SensorContactPlan.md` |
| `ScannerIntegrationPlan.md` | `CF-FQ-037` | Done -> Historical | `Systems/Targeting/SensorContact.md v1.2.0` | `ScannerIntegrationPlan.md` |
| `WeaponDataPlan.md` | `CF-FQ-008` | Done -> Historical | `Systems/Combat/WeaponData.md` | `WeaponDataPlan.md` |
| `AmmoSystem/AmmoSystemPlan.md` | `CF-FQ-031` | Done -> Historical | `Systems/Combat/Ammo.md` | `AmmoSystem/AmmoSystemPlan.md` |
| `VehicleDefenseDamageDesign.md` | `CF-FQ-033` | Done -> Historical | `Systems/Combat/VehicleDefense.md`, `Systems/Combat/HitDamage.md` | `VehicleDefenseDamageDesign.md` |
| `ProjectilePropulsionPlan.md` | `CF-FQ-028` | Done -> Historical | `Systems/Combat/Projectile.md` | `ProjectilePropulsionPlan.md` |
| `ProjectileFlightFx/ProjectileFlightFxPlan.md` | `CF-FQ-027` | Done -> Historical | `Systems/Combat/Projectile.md` | `ProjectileFlightFx/ProjectileFlightFxPlan.md` |
| `CombatFxAudio/` | `CF-FQ-024` | Done -> Historical | `Systems/Combat/CombatFx.md` | `CombatFxAudio/ImplementationDesign.md` |
| `ReticleAimDirection/` | `CF-FQ-025` | Done -> Historical | `Systems/UI/AimReticle.md`, `Systems/Vehicles/VehicleAim.md` | `ReticleAimDirection/ImplementationDesign.md` |
| `AimFireAlignment/` | `CF-FQ-022` | Done -> Historical | `Systems/Vehicles/VehicleAim.md`, `Systems/Combat/WeaponFire.md` | `AimFireAlignment/ImplementationDesign.md` |
| `ProjectileContinuousCollision/` | `CF-FQ-023` | Done -> Historical | `Systems/Combat/Projectile.md`, `Systems/Combat/DamageHitContext.md` | `ProjectileContinuousCollision/ImplementationDesign.md` |
| `HitDamage/` | `CF-FQ-018` | Done -> Historical | `Systems/Combat/HitDamage.md`, `Systems/Combat/DamageHitContext.md` | `HitDamage/ImplementationDesign.md` |
| `ReticleFireFeedback/` | `CF-FQ-017` | Done -> Historical | `Systems/Combat/FireFeedback.md`, `Systems/UI/AimReticle.md` | `ReticleFireFeedback/ImplementationDesign.md` |
| `EngineSourceBuild/` | engine migration | Superseded / Historical | 루트 `AGENTS.md`의 UE 5.8 Source Build 기준 | `EngineSourceBuild/README.md` |

이 목록은 완료 당시 상세 검증을 복제하지 않는다. 현재 동작은 Systems를 읽고, 완료 당시 이유·증거가 필요할 때만 대표 Historical 문서로 내려간다.

### 3.3 2026-08-29 구조 정리

루트에 남아 있던 완료·대체·외부 저장소 이관 Plan을 기능 단위 Archive로 정리했다. 의미가 있는 설계·TaskSource·Generated evidence는 삭제하지 않는다.

| Archived Path | 분류 | 보관 이유 |
| --- | --- | --- |
| `AmmoSystem/` | Done / Historical | Ammo Plan·Design·Roadmap·TaskSource·WorkOrder를 한 묶음으로 보존 |
| `ProjectileFlightFx/` | Done / Historical | Flight FX Plan·Roadmap·TaskSource·WorkOrder를 한 묶음으로 보존 |
| `LauncherMissileLegacy/` | Superseded / Historical | 현재 `LauncherMissile/`이 대체한 초기 Launch TaskSource·WorkOrder·ModularLauncher Plan 보존 |
| `AimPlan/` | Superseded / Historical | 현재 Aim 구현은 Systems/Vehicles/VehicleAim 및 Systems/UI/AimReticle이 소유 |
| `CameraPlan/` | Superseded / Historical | 초기 차량 카메라 기획·시스템 설계·체크리스트 보존, 현재 구현은 Systems/Vehicles의 카메라 기준을 우선 |
| `CameraDebugPlan/` | Merged / Historical | Camera Debug는 현재 VehicleDebugPanel 구조에 편입됨 |
| `VehicleDebugPlan/` | Merged / Historical | 현재 VehicleDebugPanel 구현은 Systems/UI/VehicleDebugPanel이 소유 |
| `VehiclePlatformPlan/` | Superseded / Historical | 차량 플랫폼 구현 결과가 Systems와 VehicleBuilder current plan으로 승격됨 |
| `TurretPlan/` | Superseded / Historical | 현재 터렛·Aim·Fire 계약은 Systems/VehicleAim·WeaponFire가 소유 |
| `DataPlan/` | Superseded / Historical | 현재 VehicleData/Data Authoring 작업은 Systems와 `DataAuthoring/`이 소유 |
| `DocSystemPlan/` | Completed / Historical | 문서체계 개선 적용 완료 기록 |
| `AssetDumpPlan/` | Deprecated / External Boundary | AssetDump 독립 저장소 이관 안내 보존 |
| `MeshPaintPrepTool/` | External Tool / Historical | CarFight 측 초기 계획만 보존, 구현 owner는 독립 플러그인 저장소 |

`Generated/Intermediate`의 폴더 요청·중간 TaskSource와 중복 UI 예시 이미지는 최종 evidence가 아니므로 2026-08-29 maintenance에서 Trash 격리했다.

### 3.4 2026-09-02 VehicleBuilder physical move

| Archived Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehicleBuilder/` | `CF-FQ-040` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `VehicleBuilder/VehicleBuilderPlan.md v0.1.47`, `VehicleBuilder/VehicleBuilderRoadmap.md v0.1.39` | G0~G5 PASS / Archived Path |

`CF-FQ-040`은 VB-P0-09 End-to-End USER Acceptance, WSA P0 Complete, ESH-01~06 Final Audit Clean PASS 뒤 VB-P0-10 Current System Promotion까지 완료했다.
2026-09-02 maintenance에서 unrelated ConceptArt deletion을 건드리지 않고 VehicleBuilder Historical 문서 6개만 `Archive/VehicleBuilder/`로 이동했다. 현재 구현은 Systems와 실제 Source/Asset이 소유하고, 완료 당시 상세 설계·Wagon acceptance·ESH evidence는 이 Archived Path가 보존한다.

### 3.5 2026-09-06 G5 physical move 완료

| Archived Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehicleBuilderCreationUX/` | `CF-FQ-042` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `VehicleBuilderCreationUX/VehicleBuilderCreationUXPlan.md v0.2.1` | G0~G5 PASS / Archived Path |
| `VehicleMountGuidance/` | `CF-FQ-043` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `VehicleMountGuidance/VehicleMountGuidancePlan.md v0.2.0` | G0~G5 PASS / Archived Path |
| `VehicleRuntimeCatalogPromotion/` | `CF-FQ-044` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `VehicleRuntimeCatalogPromotion/VehicleRuntimeCatalogPromotionPlan.md v0.2.0` | G0~G5 PASS / Archived Path |
| `VehicleBuilderHardpointIntegrity/` | `CF-FQ-047` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `VehicleBuilderHardpointIntegrity/VehicleBuilderHardpointIntegrityPlan.md v0.2.1` | G0~G5 PASS / Archived Path |
| `DataAssetManagement/` | `CF-FQ-045` | Done -> Historical | main_game `Systems/DataManagement/DataAssetManagement.md v1.0.0` | `DataAssetManagement/DataAssetManagementPlan.md v0.2.0` | G0~G5 PASS / Archived Path |

`CF-FQ-042`는 VBCUX-P0-01~04 Technical PASS와 P0-05 A/B/C USER Acceptance PASS 뒤 신규 차량 Creation Entry를 Current System에 승격했다. Post-closure final audit P1 old-selection Browser refresh restore는 selection clear + Slate row-restore guard로 교정하고 focused/affected 5/0 PASS로 재검증했다. 현재 통합 Vehicle Registry가 없는 단계에서 Vehicle ID 직접 입력 관리 부담은 비차단 UX 피드백으로 보존한다. 2026-09-06 G5 maintenance에서 대표 Plan을 `Archive/VehicleBuilderCreationUX/`로 이동했다.

`CF-FQ-043`은 Hardpoint/Mount/Socket Guidance와 persistent Driving receipt를 `VehicleBuilder.md` Current 계약으로 승격하고 USER/Technical closure를 완료했다. `CF-FQ-044`는 USER Driving PASS 뒤 Runtime Catalog promotion/no-auto-save/retry/same-session RuntimeApply sync를 Current 계약으로 승격했으며, USER explicit Save 후 fresh persisted AssetDump에서 Wagon exact membership 1개를 확인했다. 2026-09-06 G5 maintenance에서 두 대표 Plan을 각각 `Archive/VehicleMountGuidance/`, `Archive/VehicleRuntimeCatalogPromotion/`의 Archived Path로 이동했다.

`CF-FQ-047`은 unbound Hardpoint-like Socket 진단, explicit existing Socket adoption, Mount completeness, Step 5 Physics impact boundary, Step 7 durable final commit과 Step 8 fresh Driving readiness/persistent receipt 계약을 완료했다. P0-07G USER Re-Acceptance PASS와 fresh persisted Wagon Driving receipt readback 뒤 post-closure final audit remediation까지 Technical Clean PASS로 닫았고, representative Historical Plan v0.2.1이 해당 detailed evidence를 보존한다. Current owner는 `Systems/Vehicles/VehicleBuilder.md v1.7.0`이며, common Step 1~8 Page Shell/scroll/overflow는 CF-FQ-046 closure를 통해 같은 Current owner에 통합됐다. 대표 Plan은 `Archive/VehicleBuilderHardpointIntegrity/`로 이동했다.

`CF-FQ-045`는 native/persisted DataAsset 자동 discovery, Typed Semantic Registry, metadata-only Refresh, explicit 검사/참조 조회와 한글 우선 Manager UX를 완료했다. 최종 DAM-P0-04E에서 USER가 `직접 에셋 생성: 가능/불가`, Refresh 후 `재검사 필요 / 재확인 필요`와 전체 정보 구조를 직접 확인하고 PASS했으며 Current owner는 `Systems/DataManagement/DataAssetManagement.md v1.0.0`이다. 대표 Plan은 완료 당시 Build·Automation·USER feedback을 보존한 채 `Archive/DataAssetManagement/` Archived Path로 이동했고 residual P2 2건은 별도 polish lifecycle이 필요할 때만 연다.

### 3.6 CF-FQ-030 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `MissileGuidance/` | `CF-FQ-030` | Done -> Historical | main_game `Systems/Combat/MissileGuidance.md v1.0.1`, `Systems/Combat/Projectile.md v1.9.0` | `MissileGuidance/MissileGuidancePlan.md v0.6.25` | G0~G4 PASS / Post-Closure Final Audit PASS / G5 Deferred / Retained Path |

`CF-FQ-030`은 Direct TargetActor 물리 제한형 Missile Flight/Guidance, PurePursuit·LeadPursuit·PN, Independent Activation, Stateful Seeker/Observation, rear-aspect/Overshoot, Pool Reset과 Guidance Preset authoring/idempotence를 완료했다. `CF-TC-027`은 Technical + USER Guidance Feel Complete / PASS이며 USER가 Low/Normal/High를 직접 확인해 "얼추 PASS"로 승인했다. Current 구현은 main_game Systems가 소유하고, 완료 당시 상세 설계·Technical evidence·USER rejection/correction 이력은 현재 `MissileGuidance/` 경로가 보존한다. LaserPoint·DataLink/Inertial 실제 Runtime, Angled/Vertical/Loft/TopAttack, 실제 Expire와 장비별 Multi-Muzzle/Salvo 연출은 별도 후속 lifecycle로만 연다. 이번 closure에서는 G5 physical move를 수행하지 않았다.

### 3.7 CF-FQ-049 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `DataAssetStaging/` | `CF-FQ-049` | Done -> Historical | main_game `Systems/DataManagement/DataAssetAuthoring.md v1.0.0` | `DataAssetStaging/DataAssetStagingPlan.md v0.7.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-049`는 Editor-off canonical JSON Staging, strict typed schema/fingerprint, exact selection Preview, same-selection fresh Review, stale/conflict/dirty fail-closed, one-shot Reviewed Apply와 typed exact-package durable Save/Reload/semantic readback을 P0 Current 계약으로 완료했다. Product Low/Normal/High는 closure에서 `ApplyReviewed`를 실행하지 않아 Product UE Asset Apply·Save 0을 유지했다. test-owned durable fixture와 final Build/OperationalEntry/exact13 evidence, Current System Promotion 직전 fresh persisted AssetDump는 representative Historical Plan이 보존한다. Current 구현은 main_game `DataAssetAuthoring.md`와 실제 `CarFight_ReEditor/DataAuthoring` Source가 소유한다. Ammo/Damage/Sensor 등 새 타입 확장이나 별도 UI는 old next-action을 재사용하지 않고 새 lifecycle로만 연다. 이번 closure에서는 plan_repo existing dirty와 retained reference를 보호하기 위해 G5 physical move를 수행하지 않았다.

### 3.8 CF-FQ-050 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `DAContractEvolution/` | `CF-FQ-050` | Done -> Historical | main_game `Systems/DataManagement/DataAssetAuthoring.md v1.1.0` | `DAContractEvolution/DAContractEvolutionPlan.md v0.7.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-050`은 Staging 지원 DataAsset의 Source/Adapter/Mapping/Semantic descriptor와 deterministic signature, native Reflection/production behavior drift guard, SchemaRevision/AdapterContractRevision guard, explicit migration impact/Resolution/Evidence와 append-only accepted snapshot promotion gate를 MissileGuidePreset Pilot으로 완료했다. final integration은 DACE exact15/15 + affected CF-FQ-049 exact13/13 + OperationalEntry exact1/1 PASS이며 Product Low/Normal/High Apply·Save 0, canonical Product Staging mutation 0, accepted snapshot append 0을 유지했다. 현재 구현은 main_game `DataAssetAuthoring.md v1.1.0`과 실제 `CFDAContractGuard*` / typed Staging Source가 소유한다. 다른 DA 타입은 이 guard lifecycle을 선행조건으로 사용할 수 있지만 자동 지원되는 것은 아니며 별도 Typed Adapter/Schema/descriptor/probe lifecycle이 필요하다. G5 physical move는 plan_repo existing dirty와 retained evidence 보호를 위해 Deferred했다.

### 3.9 CF-FQ-051 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `DataAssetOnboarding/` | `CF-FQ-051` | Done -> Historical | main_game `Systems/DataManagement/DataAssetAuthoring.md v1.4.1` | `DataAssetOnboarding/DataAssetOnboardingPlan.md v0.3.22` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-051`은 MissileGuidePreset 단일 authoring/DACE 기반을 AmmoData까지 확장해 Production provider exact2와 provider-neutral mixed Explicit Paths authoring을 완료했다. DAO-P0-06 reuse measurement에서 prohibited shared algorithm duplication 0, third-type shared core algorithm rewrite 0 required를 확인했고 Final Audit Correction + Re-review까지 `P0 0 / blocking P1 0 / P2 1 non-blocking / PASS`로 닫았다. Current owner는 main_game `DataAssetAuthoring.md v1.4.1`이며, P2는 generic implementation과 legacy Missile compatibility의 물리적 owner 혼재 유지보수 부채뿐이다. AmmoData는 Current 지원 타입이고 DamageData는 Candidate / Not Started이며 VehicleSensorData 및 기타 신규 DataAsset 타입과 함께 별도 lifecycle에서만 onboarding한다. Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10과 CF-FQ-039 Active를 보존했다. G5 physical move는 existing plan_repo dirty와 retained reference 보호를 위해 Deferred한다.

### 3.10 CF-FQ-052 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `DamageDataOnboarding/` | `CF-FQ-052` | Done -> Historical | main_game `Systems/DataManagement/DataAssetAuthoring.md v1.5.14` | `DamageDataOnboarding/DamageDataOnboardingPlan.md v0.2.14` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-052`는 DamageData third onboarding을 완료해 Production provider와 mixed operational admission을 MissileGuidePreset + AmmoData + DamageData exact3로 확장했고, Damage DACE `ContractReady`, accepted history `DACE-DamageData-S1-A1-Bootstrap` exact1, Product canonical Damage exact0를 Current 계약으로 승격했다. DDO-P0-05 reuse measurement에서 AmmoData second onboarding 대비 type-owned Production은 exact5 / 1,789 LOC에서 exact5 / 1,574 LOC로 215 LOC(약 12.0%) 감소했고, shared foundation touch exact5는 전부 provider registration, explicit admission 또는 read-only projection에 한정됐다. prohibited shared algorithm duplication exact0, shared core algorithm rewrite 0 required, 신규 architecture-gap correction exact0을 확인해 fourth-type onboarding readiness를 PASS로 판정했다. 최종 판정은 `P0 0 / blocking P1 0 / P2 2 non-blocking / PASS`다. DDO-P0-05 executable mutation은 0이므로 latest Build/DDO exact14와 predecessor4/affected13 Accepted evidence를 재사용하고 재실행하지 않았다. Current owner는 main_game `DataAssetAuthoring.md v1.5.14`이며 G5 physical move는 existing plan_repo dirty와 retained reference 보호를 위해 Deferred한다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### 3.11 CF-FQ-053 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehicleDefenseOnboarding/` | `CF-FQ-053` | Done -> Historical | main_game `Systems/DataManagement/DataAssetAuthoring.md v1.6.0` | `VehicleDefenseOnboarding/VehicleDefenseOnboardingPlan.md v0.1.5` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-053`은 §2.8 Routine 4-Gate를 첫 실제 fourth production type에 적용해 VehicleDefenseData를 Current 지원 타입으로 승격했다. Production provider와 mixed operational admission은 MissileGuidePreset + AmmoData + DamageData + VehicleDefenseData explicit exact4이며 VehicleDefense는 `ReviewedMutationReady / DACE ContractReady / accepted history exact1 / canonical Product target exact0`이다. Gate 4 fresh AssetDump에서 protected `DA_VehicleDefense_Test` authored exact17 / reference exact0 / errors0과 baseline values를 확인했고 Git `.uasset` dirty0으로 onboarding Apply·Save0을 재확인했다. Gate 3 final executable 뒤 Source mutation0이라 Build/VDR7/DACE15/DDO14 accepted evidence를 반복 실행하지 않았다. Process Benchmark는 `Faster Confirmed`이며 VehicleDefense Production exact5/1,613 LOC가 Damage exact5/1,574보다 약 2.5% 크고 SourceShape exact29로 더 복잡함에도 Primary Gate 6→4, feature C++ test LOC 2,808→2,266, ActiveWork/FeatureQueue projection increments +16→+6, redundant defect-free Review0으로 process churn이 감소했다. shared semantic rewrite0 / prohibited duplication0 / Architecture Gap 없음이다. Current owner는 main_game `DataAssetAuthoring.md v1.6.0`이며 G5 physical move는 Deferred한다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### 3.12 CF-FQ-029 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `LauncherMissile/` | `CF-FQ-029` | Done -> Historical | main_game `Systems/Combat/Launcher.md v1.0.1` | `LauncherMissile/LauncherMissilePlan.md v0.15.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-029`은 가변 Muzzle, SingleCycle/Ripple/Salvo, Direct/Angled/Vertical Release, Carrier Velocity, 실제 사출 방향 MuzzleBlocked, Sequence failure/cancel과 Launch Context → ProjectilePool → ProjectileActor 인계를 완료했다. 기존 USER PIE의 Direct/Ripple/SingleCycle/Salvo, Muzzle 순환, Command Target 고정, 동일 차량 Salvo 격리와 일반 충돌 evidence는 반복하지 않고 보존했다. 2026-09-11 남은 순수 기술 invariant를 Current Product-path Automation으로 닫아 Official UE 5.8 Build PASS, `CarFight.Launcher.LM_P0_06.FinalIntegration` 1/1, 전체 `CarFight.Launcher` 5/5 PASS를 확보했다. fresh AssetDump에서 저장 `DA_RocketLauncher`의 Direct / EjectionSpeed 0 / CarrierVelocityRatio 0 기본값을 확인했고 Product `.uasset` 및 Product runtime Source mutation은 0이다. Current owner는 main_game `Launcher.md v1.0.1`이며 다른 차량 요격, cross-type/Volley 격리, Pool Ignore Reset과 lifecycle cancel은 비차단 후속 회귀다. G5 physical move는 별도 maintenance로 Deferred한다.

### 3.13 CF-FQ-038 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `DataAuthoring/` | `CF-FQ-038` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `DataAuthoring/DataAuthoringPlan.md v0.2.53`, `DataAuthoring/DataAuthoringRoadmap.md v0.1.57` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-038`은 Recipe/Resolver/SourceTrace/Diff/Validation/Apply/Undo 기반 Data Authoring Backend와 전문가용 Advanced Workspace를 완료했다. P0-12 USER Acceptance는 7/8 Historical로 보존하며 UA-08은 Runtime Technical PASS / USER Inconclusive / quantitative comparison Deferred다. 2026-09-11 DEL6 compatibility retirement에서 Legacy Vehicle DA Wizard hidden spawner/open entry/test access를 제거하고 `UE/Source` direct reference exact0을 확인한 뒤 Legacy Wizard 소스 3개를 물리 폐기해 DEL1~DEL7 PASS를 확정했다. final Official UE 5.8 Build `1fd947042024420aaa7561d380e4548f` PASS와 전체 `CarFight.DataAuthoring` 111/111 PASS / failure0 / Engine Exit0을 확보했다. 첫 broad regression의 Wagon 2건은 stale historical test hash expectation으로 분리해 test-only 교정했고 Product Asset migration/Apply/Save는 0이다. 현재 구현은 main_game `VehicleBuilder.md v1.7.0`과 실제 `CarFight_ReEditor/DataAuthoring` Source가 소유한다. G5 physical move는 기존 retained reference 보호를 위해 Deferred한다.

### 3.14 CF-FQ-015 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehicleDataTuning/` | `CF-FQ-015` | Done -> Historical / Rebaseline Complete | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` + `Systems/Vehicles/VehicleData.md v2.3.0` | `VehicleDataTuning/VehicleDataTuningPlan.md v0.3.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-015`는 2026-08-15에 VD-P0-00 Foundation Audit, VD-P0-01 Validator Contract, VD-P0-02 Representative Compare, VD-P0-03 Runtime Apply Contract를 Technical PASS로 닫았고 당시 Official UE 5.8 Build `0cffed2f02674b6692d4f8af811d9036` PASS와 `CarFight.VehicleData` 3/3 PASS를 확보했다. 남겨둔 VD-P0-04 Sedan/SUV USER Tuning은 실제 USER PASS로 수행되지 않았으며, 후속 Data Authoring/VehicleBuilder가 Recipe/Profile/Resolver/Diff/Approval/DefinitionApply, Technical Driving Benchmark와 persistent USER Driving Acceptance를 제공하게 되어 **Superseded / Not Executed**로 종료했다.

Rebaseline에서 유지할 가치는 네 Current 계약으로 승격했다: `Controlled Axis Tuning`, `Technical Benchmark + USER Feel Pair`, `Vehicle Character / Reference Baseline`, `Measurement Gap / Benchmark Extension Ownership`. 성능 튜닝 workflow는 main_game `VehicleBuilder.md v1.7.0`, 데이터 저장·Validator·Representative Compare·Runtime 입력 기반은 `VehicleData.md v2.3.0`이 소유한다. 제동거리, Yaw Rate, 횡가속, Slip Angle, Suspension stroke/settling은 현재 구현 완료로 확대하지 않고 Measurement Gap 후보로 남겼다. 이 closure는 문서/책임 Rebaseline이며 새 Source/Asset/Build/Automation/PIE mutation은 0이다. G5 physical move는 optional maintenance로 Deferred한다.

### 3.15 CF-FQ-048 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehiclePawnSlimming/` | `CF-FQ-048` | Done -> Historical | main_game `Systems/Vehicles/VehicleRuntime.md v1.6.0` | `VehiclePawnSlimming/VehiclePawnSlimmingPlan.md v0.7.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-048`은 Pawn에 누적된 Visual/Fire/Runtime orchestration을 `UCFVehicleVisualComp`, `UCFVehicleFireComp`, `UCFVehicleRuntimeComp`로 분리하고 reflected Input/Debug type owner를 독립 header로 정리했다. Lifecycle/Composition root/Input entry/Target identity/Compatibility facade/Contract state Authority는 Pawn에 유지하고 final facade deletion audit에서 추가 source mutation 필요 0을 확인했다. 최종 evidence는 P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS, final affected exact28 process `6431180d7e7d4b11ac9f8dc51a25b5c2` 28/28 PASS, 사용자 representative regression smoke PASS다. RuntimeRead T0는 UE MCP unavailable로 `Waived / Deferred Observation`이며 미관측 runtime fact를 PASS로 확대하지 않는다.

사용자 smoke 중 확인된 Vehicle Builder 신규 차량의 기본 Sensor/Active Scan baseline 누락은 current scanner-less 0-range 계약과 상위 기본 센서 기획의 불일치로 분리했으며 VPS가 새로 만든 회귀 근거가 없어 CF-FQ-048 Acceptance에서 제외했다. 해당 문제는 별도 correction lifecycle이 소유해야 한다. G5 physical move는 기존 plan_repo dirty와 retained reference를 보호하기 위해 Deferred한다.

### 3.16 CF-FQ-046 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehicleBuilderInfoUX/` | `CF-FQ-046` | Done -> Historical | main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0` | `VehicleBuilderInfoUX/VehicleBuilderInfoUXPlan.md v0.2.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-046`은 Vehicle Builder Step 1~8의 USER 정보 우선순위와 Common Page Shell/height/scroll/overflow UX를 완료했다. post-CF-FQ-047 fresh rebaseline 뒤 단일 FillHeight `CommonPageScrollBox`, nested vertical Scroll 제거, Step 5/7/8 fixed action, Global Navigation, Step/target 전환 scroll-to-start와 refresh 위치 유지 계약을 구현했다. Official UE 5.8 Build PASS, focused/affected VBIUX exact8 8/8 PASS, Post-Implementation Mid-review `P0 0 / blocking P1 0 / P2 0`, P0-05F USER Acceptance PASS를 closure evidence로 보존한다. Current 구현은 main_game `VehicleBuilder.md v1.7.0`과 실제 `CFVehicleBuilderTab` Source가 소유하며 CF-FQ-038 Data Authoring, CF-FQ-047 durable Save/Driving, CF-FQ-015 Performance Tuning backend authority는 변경하지 않았다. G5 physical move는 existing plan_repo dirty와 retained reference를 보호하기 위해 Deferred한다.

### 3.17 CF-FQ-057 Historical + Retained Path

| Retained Path | Feature | semantic 분류 | 현재 구현 owner | 대표 Historical 문서 | placement 판단 |
| --- | --- | --- | --- | --- | --- |
| `VehicleCameraFX/` | `CF-FQ-057` | Done -> Historical | main_game `Systems/Vehicles/VehicleCamera.md v1.3.0` | `VehicleCameraFX/VehicleCameraFXPlan.md v0.3.0` | G0~G4 PASS / G5 Deferred / Retained Path |

`CF-FQ-057`은 normalized Driving FX, Gameplay/Presentation View 분리, Speed FOV/Arm, Accel/Brake Kick, actual body-motion + free-look Lateral Presentation, collision recovery와 Aim/Targeting 보호 계약을 Current System으로 승격했다. USER PASS는 Lateral Roll, Overspeed, Camera Pop/Collision, Aim/Targeting Interference 범위다. Acceleration Rear Kick과 Braking Forward Kick은 기능 확인 후 추가 tuning을 고성능/강제동 차량 이후로 Deferred했고, 멀미/피로감도 USER 결정으로 Deferred하여 PASS로 확대하지 않는다. Combat/Aim/Airborne attenuation은 계산/API는 구현됐으나 fresh C++ Product caller 0 및 current Blueprint exact6 graph usage 0으로 authoritative producer가 없는 wiring debt다. 기존 official UE 5.8 Build 성공과 focused Automation exact8/8 PASS를 closure evidence로 보존하며 이번 P0-04에서 Source/DataAsset mutation은 없다. G5 physical move는 existing dirty/reference 보호를 위해 Deferred한다.

---

## 4. Historical 전환 Gate

Plan을 Historical로 내릴 때 최소 다음을 확인한다.

```text
G0 Evidence
= 구현 + 필요한 Build/Automation/USER PIE가 실제 증거로 완료

G1 Current Knowledge Promotion
= 현재 구현·제약·데이터 흐름이 Systems와 필요한 ProjectSSOT에 반영

G2 Current Route Cleanup
= ActiveWork와 Plan Index의 current next-action에서 제거

G3 Reference Preservation
= 완료 당시 대표 Plan/Result를 찾을 경로 보존

G4 Historical
= 현재 착수 기준에서 내려옴

G5 Optional Physical Move
= 링크·dirty work 보호가 가능한 별도 maintenance에서만 수행
```

**물리 Archive 이동은 G4의 필수조건이 아니다.**

---

## 5. 재활성화 규칙

Historical Plan을 그대로 Active로 되돌리지 않는다.

```text
1. 현재 ProjectSSOT / FeatureQueue 확인
2. 현재 Systems / 실제 구현 확인
3. 과거 결정 중 유효·폐기 범위 분리
4. 새 work ID 또는 명시적 새 lifecycle 정의
5. 필요한 대표 Plan을 Plan Index에 새 Current route로 등록
```

과거 Plan은 baseline/history로 사용할 수 있지만 새 Acceptance를 대신하지 않는다.

---

## 6. Physical Move 정책

`Move Candidate`는 semantic 상태가 아니라 maintenance 분류다.

2026-08-22 maintenance에서 기존 Historical 15개 묶음은 old-path 참조 감사와 dirty-work 보호를 거쳐 physical Archive로 이동했다. 이후에도 move/rename은 다음 조건을 모두 확인한 별도 maintenance에서만 수행한다.

```text
- semantic Historical 확정
- Current Knowledge가 Systems/ProjectSSOT로 승격됨
- Current route에서 제거됨
- 새 Archive 경로의 대표 진입점 존재
- Current 문서 old-path 참조를 함께 교정 가능
- 기존 dirty work와 충돌하지 않음
- 실제 삭제가 아니라 이동으로 evidence 보존
```

---

## 7. Changelog / Migration

### v1.32 - 2026-10-02

- `CF-FQ-057 Vehicle Camera Driving FX`를 VCFX-P0-04 Current System Promotion + Closure 뒤 `Done -> Historical + Retained Path`로 등록했다.
- Current owner는 main_game `Systems/Vehicles/VehicleCamera.md v1.3.0`, representative Historical Plan은 `VehicleCameraFX/VehicleCameraFXPlan.md v0.3.0`이다.
- Lateral/Overspeed/Camera Pop-Collision/Aim-Targeting USER PASS와 Rear/Brake tuning Deferred, Comfort Deferred, Mode attenuation producer wiring debt 경계를 그대로 보존한다.
- G0~G4 PASS / G5 Deferred이며 physical move는 수행하지 않았다.

Migration: CF-FQ-057의 old Ready/P0-03/P0-04 next-action은 current route가 아니다. 현재 Vehicle Camera 구현 판단은 main_game `VehicleCamera.md v1.3.0`과 실제 Source/Asset을 우선하며 retained Plan v0.3.0은 완료 당시 상세 evidence와 deferred debt를 보존한다.

### v1.31 - 2026-09-15

- `CF-FQ-046 Vehicle Builder 사용자 정보 UX`를 P0-06 Final Closure Audit 뒤 `Done -> Historical + Retained Path`로 등록했다. 통합 VehicleBuilder를 가리키는 Archive current-owner projection도 `VehicleBuilder.md v1.7.0`으로 동기화했다.
- Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.7.0`, representative Historical Plan은 `VehicleBuilderInfoUX/VehicleBuilderInfoUXPlan.md v0.2.0`이다. Official Build PASS + VBIUX exact8 8/8 PASS + USER Acceptance PASS를 보존하며 G0~G4 PASS / G5 Deferred다.
- CF-FQ-038/047/015 backend authority와 Product Asset은 변경하지 않았고, USER UAT 중 관측된 `DA_Recipe_Wagon.uasset` dirty는 closure 범위에서 정리하지 않았다.

Migration: CF-FQ-046의 old Ready/P0-06 next-action은 current route가 아니다. 현재 Vehicle Builder USER 정보·Page Shell 판단은 main_game `VehicleBuilder.md v1.7.0`과 실제 Source를 우선하며 retained Plan v0.2.0은 완료 당시 상세 evidence를 보존한다.

### v1.30 - 2026-09-15

- `CF-FQ-048 Vehicle Pawn Slimming`을 VPS-P0-00~05 완료와 USER regression smoke PASS 뒤 `Done -> Historical + Retained Path`로 등록했다.
- Current owner는 main_game `Systems/Vehicles/VehicleRuntime.md v1.6.0`, representative Historical Plan은 `VehiclePawnSlimming/VehiclePawnSlimmingPlan.md v0.7.0`이다. Official Build baseline PASS와 final exact28 28/28 PASS를 closure evidence로 보존한다.
- RuntimeRead T0는 UE MCP unavailable에 따른 `Waived / Deferred Observation`으로 남기며, Vehicle Builder-created vehicle의 기본 Sensor/Active Scan baseline 누락은 VPS regression이 아닌 별도 선행 설계/구현 불일치로 분리했다. G0~G4 PASS / G5 Deferred다.

Migration: CF-FQ-048의 old Ready/USER Smoke next-action은 current route가 아니다. 현재 Vehicle Runtime 판단은 main_game `VehicleRuntime.md v1.6.0`과 실제 Source를 우선하며 retained Plan은 완료 당시 상세 evidence와 deferred RuntimeRead 경계를 보존한다.

### v1.29 - 2026-09-11

- `CF-FQ-015 Vehicle Data Tuning`을 Rebaseline Complete 뒤 Done -> Historical + Retained Path로 등록했다.
- VD-P0-00~03 Historical Technical PASS는 보존하고 VD-P0-04는 USER PASS가 아니라 `Superseded / Not Executed`로 종료했다. Current owner는 main_game `VehicleBuilder.md v1.6.0` + `VehicleData.md v2.3.0`, representative Historical Plan은 `VehicleDataTuningPlan.md v0.3.0`이다.
- 4개 Performance Tuning 계약을 VehicleBuilder로 승격했으며 미구현 제동·Yaw Rate·횡가속·Slip Angle·Suspension 계측은 Measurement Gap 후보로 유지한다. VehicleBuilder를 공유하는 CF-FQ-038/040/042/043/044/047 current owner pointer도 최신 v1.6.0으로 동기화했다. G0~G4 PASS / G5 Deferred다.

Migration: CF-FQ-015의 old Paused/`VD-P0-04 USER Tuning Pending`은 current route가 아니다. 새 차량 성능 튜닝은 main_game VehicleBuilder v1.6.0 protocol을 사용하며 retained Plan v0.3.0은 Historical technical evidence와 supersession 근거를 보존한다.

### v1.28 - 2026-09-11

- `CF-FQ-038 Vehicle Data Authoring`을 DEL1~DEL7 PASS / Legacy Wizard physical retirement / 전체 `CarFight.DataAuthoring` 111/111 PASS 뒤 Done -> Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.5.0`, representative Historical Plan/Roadmap은 `DataAuthoring/DataAuthoringPlan.md v0.2.53` / `DataAuthoring/DataAuthoringRoadmap.md v0.1.57`이다.
- VehicleBuilder owner를 공유하는 CF-FQ-040/042/043/044/047의 Current owner projection도 v1.5.0으로 동기화했다. UA-08은 비차단 Deferred이며 G0~G4 PASS / G5 Deferred다.

Migration: CF-FQ-038의 old Paused/DEL6 Pending/UA-07 next-action은 current route가 아니다. 현재 차량 제작·Authoring 구현은 main_game VehicleBuilder v1.5.0과 실제 Source를 우선한다.

### v1.27 - 2026-09-11

- `CF-FQ-029 Modular Launcher`를 LM-P0-06 Final Technical Integration PASS 뒤 Done -> Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/Combat/Launcher.md v1.0.1`, representative Historical Plan은 `LauncherMissile/LauncherMissilePlan.md v0.15.0`이다.
- 기존 USER PIE evidence를 보존하고 남은 Angled/Vertical·Carrier Velocity·MuzzleBlocked를 Current Product-path Automation 1/1 + Launcher 5/5로 닫았다. G0~G4 PASS / G5 Deferred다.

Migration: CF-FQ-029의 old Paused/USER PIE Pending은 current route가 아니다. 현재 Launcher 동작 판단은 main_game Systems와 실제 Source를 우선한다.

### v1.26 - 2026-09-11

- `CF-FQ-053 VehicleDefenseData Routine Onboarding / Process Benchmark`를 VDR-P0-03 Final Acceptance + Process Benchmark PASS 뒤 Done -> Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/DataManagement/DataAssetAuthoring.md v1.6.0`, representative Historical Plan은 `VehicleDefenseOnboarding/VehicleDefenseOnboardingPlan.md v0.1.5`이며 Process Benchmark는 `Faster Confirmed`다.
- Gate 4 executable mutation0이라 Gate 3 final Build/VDR7/DACE15/DDO14 evidence를 재사용했고 fresh persisted AssetDump + shared semantic diff audit만 추가했다. G0~G4 PASS / G5 Deferred, shared semantic rewrite0 / prohibited duplication0 / Architecture Gap 없음이다.

Migration: CF-FQ-053 현재 구현 판단은 main_game `DataAssetAuthoring.md v1.6.0`과 실제 DataAuthoring Source를 우선한다. retained `VehicleDefenseOnboarding/VehicleDefenseOnboardingPlan.md v0.1.5`는 Historical Acceptance/Process Benchmark evidence owner이며 old Ready/next-action을 현재 작업으로 복원하지 않는다.

### v1.25 - 2026-09-11

- `CF-FQ-052 DamageData Third-Type Onboarding / Reuse Verification`을 DDO-P0-05 Reuse Measurement / Acceptance / Current System Promotion PASS 뒤 Done -> Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/DataManagement/DataAssetAuthoring.md v1.5.14`, representative Historical Plan은 `DamageDataOnboarding/DamageDataOnboardingPlan.md v0.2.14`다. prohibited shared algorithm duplication exact0, shared core algorithm rewrite 0 required, fourth-type onboarding readiness PASS를 보존한다.
- G0~G4 PASS / G5 Deferred이며 DDO-P0-05 executable mutation0이므로 latest Build/DDO exact14/predecessor4/affected13 evidence를 반복 실행하지 않았다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

Migration: CF-FQ-052 현재 구현 판단은 main_game `DataAssetAuthoring.md v1.5.14`와 실제 DataAuthoring Source를 우선한다. retained `DamageDataOnboarding/DamageDataOnboardingPlan.md v0.2.14`는 Historical evidence owner로 사용하며 이전 Ready/next-action을 현재 작업으로 복원하지 않는다.

### v1.24 - 2026-09-10

- `CF-FQ-051 / DAO-P0-06 Final Audit Correction + Re-review`를 Historical projection에 동기화했다. Current §2의 stale Ammo 미지원 표현과 완료 Feature header 누락을 문서-only로 교정한 뒤 최종 판정은 `P0 0 / blocking P1 0 / P2 1 non-blocking / PASS`다.
- Current owner는 main_game `Systems/DataManagement/DataAssetAuthoring.md v1.4.1`, representative Historical Plan은 `DataAssetOnboarding/DataAssetOnboardingPlan.md v0.3.22`다. hybrid shared/legacy physical ownership P2는 non-blocking maintenance debt로 유지한다.
- G0~G4 PASS / G5 Deferred와 Done -> Historical + Retained Path 상태는 변경하지 않았다. Source/Asset/Test mutation과 Build/Automation 재실행은 0이며 기존 executable evidence와 protected exact10을 보존했다.

Migration: CF-FQ-051 현재 구현 판단은 main_game `DataAssetAuthoring.md v1.4.1`과 실제 DataAuthoring Source를 우선하고 retained `DataAssetOnboarding/DataAssetOnboardingPlan.md v0.3.22`는 Historical evidence owner로 사용한다. v1.23의 v1.4.0/v0.3.21 포인터는 당시 closure projection 기록이다.

### v1.23 - 2026-09-10

- `CF-FQ-051 Data Asset Multi-Type Onboarding`을 DAO-P0-06 Reuse Measurement / Acceptance / Current System Promotion Complete 뒤 Done → Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/DataManagement/DataAssetAuthoring.md v1.4.0`, representative Historical Plan은 `DataAssetOnboarding/DataAssetOnboardingPlan.md v0.3.21`이다. second onboarding에서 prohibited shared algorithm duplication 0, third-type shared core algorithm rewrite 0 required를 확인했고 사전검수는 P0 0 / blocking P1 0 / P2 1 non-blocking PASS다.
- G0~G4를 PASS로 닫고 G5 Physical Move는 Deferred했다. Product canonical Ammo exact0, Missile Product exact3, HeavyFinite/RocketFinite exact2, protected exact10과 현재 Active CF-FQ-039는 변경하지 않았다.

Migration: CF-FQ-051 현재 구현 판단은 main_game `DataAssetAuthoring.md v1.4.0`과 실제 DataAuthoring Source를 우선한다. retained `DataAssetOnboarding/` Plan의 old Ready/HOLD/next-action은 현재 착수 지시로 사용하지 않으며 DamageData를 포함한 새 DataAsset onboarding은 별도 lifecycle로 연다.

### v1.22 - 2026-09-09

- `CF-FQ-050 Data Asset Contract Evolution Guard`를 DACE-P0-06 Final Acceptance PASS / P0 Complete / Current System Promotion Complete 뒤 Done → Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/DataManagement/DataAssetAuthoring.md v1.1.0`, representative Historical Plan은 `DAContractEvolution/DAContractEvolutionPlan.md v0.7.0`이다.
- G0~G4를 PASS로 닫고 G5 Physical Move는 Deferred했다. Product Low/Normal/High Apply·Save 0, canonical Product Staging mutation 0, accepted snapshot append 0과 현재 Active CF-FQ-039는 변경하지 않았다.

Migration: CF-FQ-050 현재 구현 판단은 main_game Systems와 실제 `CFDAContractGuard*` / typed Staging Source를 우선한다. retained `DAContractEvolution/` Plan의 old Ready/Pending/next-action은 현재 착수 지시로 사용하지 않으며 다른 DA onboarding은 별도 lifecycle로 연다.

### v1.21 - 2026-09-09

- `CF-FQ-049 Data Asset Staging·Batch Authoring`을 DAS-P0-05 Final Acceptance PASS / P0 Complete / Current System Promotion Complete 뒤 Done → Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/DataManagement/DataAssetAuthoring.md v1.0.0`, representative Historical Plan은 `DataAssetStaging/DataAssetStagingPlan.md v0.7.0`이다.
- G0~G4를 PASS로 닫고 G5 Physical Move는 Deferred했다. Product Low/Normal/High Apply·Save 0, CF-FQ-045 read-first Manager ownership과 현재 Active CF-FQ-039는 변경하지 않았다.

Migration: CF-FQ-049 현재 구현 판단은 main_game Systems와 실제 Source를 우선한다. retained `DataAssetStaging/` Plan의 old Ready/Pending/next-action은 현재 착수 지시로 사용하지 않으며 새 DataAsset 타입 확장은 별도 lifecycle로 연다.

### v1.20 - 2026-09-08

- `CF-FQ-030 Post-Closure Final Audit` 교정 후 재검수 PASS를 Historical 색인에 동기화했다.
- Current owner는 main_game `Systems/Combat/MissileGuidance.md v1.0.1`, representative Historical Plan은 `MissileGuidance/MissileGuidancePlan.md v0.6.25`다.
- Final Audit 최초 P0 0 / P1 3 / P2 0은 Current Flight 상태/lifecycle·Tick prerequisite 문서 표현과 Plan Index stale route였고, 문서 교정 후 P0 0 / P1 0 / P2 0 PASS로 닫았다.
- semantic lifecycle은 Done -> Historical, placement는 Retained Path / G5 Deferred 그대로다. Product Source/Asset과 기존 검증 evidence는 재실행하지 않았다.

### v1.19 - 2026-09-08

- `CF-FQ-030 물리 제한형 미사일 비행·유도`를 Done -> Historical + Retained Path로 등록했다.
- Current owner는 main_game `Systems/Combat/MissileGuidance.md v1.0.0`, 공통 Projectile 통합 경계는 `Systems/Combat/Projectile.md v1.9.0`이다.
- representative Historical Plan은 `MissileGuidance/MissileGuidancePlan.md v0.6.24`, detailed design evidence는 같은 폴더 `GuidancePerformanceDesign.md v0.1.24`가 보존한다.
- G0~G4를 닫았고 G5 Physical Move는 Deferred다. 문서 lifecycle 전환만을 이유로 Build/Automation/AssetDump/USER PIE를 재실행하지 않았다.

### v1.18 - 2026-09-06

- `CF-FQ-042/043/044/045/047`의 G5 Physical Move를 완료해 기존 Historical + Retained Path 5개를 `Archive/<Feature>/`의 Historical + Archived Path로 전환했다.
- 이동은 대표 Plan 파일의 삭제·재생성이 아니라 동일 evidence 파일의 물리 move로 수행했고, semantic Done/Historical 상태와 Current System owner, USER/Technical PASS는 변경하지 않았다.
- Current projection의 탐색 경로만 새 Archive 경로로 동기화하며 Build/Automation/PIE/Benchmark/USER Acceptance는 재실행하지 않았다.

Migration: 이전 `Document/Plan/VehicleBuilderCreationUX/`, `VehicleMountGuidance/`, `VehicleRuntimeCatalogPromotion/`, `VehicleBuilderHardpointIntegrity/`, `DataAssetManagement/` 경로는 2026-09-06 이전 Changelog/Historical 문맥에서만 해석한다. 현재 Historical 문서는 각각 `Document/Plan/Archive/<Feature>/`에서 찾는다.

### v1.17 - 2026-09-05

- `CF-FQ-047` post-closure final audit remediation evidence sync 완료를 반영해 representative Historical Plan pointer를 `VehicleBuilderHardpointIntegrity/VehicleBuilderHardpointIntegrityPlan.md v0.2.1`, Current owner를 main_game `Systems/Vehicles/VehicleBuilder.md v1.4.1`로 동기화했다.
- CF-FQ-040/042/043/044/047의 Current owner projection을 최신 VehicleBuilder v1.4.1로 맞췄다. 각 Feature의 당시 승격 버전과 완료 evidence는 기존 Historical Plan/Changelog에 그대로 보존한다.
- semantic lifecycle과 placement는 변경하지 않는다. CF-FQ-047은 Done → Historical + Retained Path, G5 Deferred 그대로이며 이번 동기화에서 Build/Test/Benchmark/USER Driving을 재실행하지 않았다.

Migration: CF-FQ-047의 현재 구현은 `Systems/Vehicles/VehicleBuilder.md v1.4.1`과 실제 Source를 우선하고, post-closure remediation 상세 evidence는 retained Plan v0.2.1을 사용한다. 이전 v1.16의 v1.4.0/v0.2.0 표기는 당시 closure projection의 Historical 기록이다.

### v1.16 - 2026-09-05

- `CF-FQ-047 Vehicle Builder Hardpoint Authoring Integrity`을 Done → Historical + Retained Path로 등록했다. VBHAI-P0-07G USER Re-Acceptance PASS와 fresh persisted Driving receipt readback, P0-08 Current System Promotion으로 G0~G4를 닫았다.
- Current owner를 main_game `Systems/Vehicles/VehicleBuilder.md v1.4.0`, representative Historical Plan을 `VehicleBuilderHardpointIntegrity/VehicleBuilderHardpointIntegrityPlan.md v0.2.0`으로 고정했다.
- CF-FQ-040/042/043/044의 Current owner pointer도 최신 VehicleBuilder v1.4.0으로 동기화했다. G5 physical move는 기능 완료 조건이 아니므로 별도 maintenance까지 Deferred다.

Migration: CF-FQ-047의 현재 구현은 Systems/실제 Source를 우선한다. retained Plan의 old Ready/Pending/next-action은 현재 착수 지시로 사용하지 않으며, CF-FQ-046 common Page Shell 후속은 별도 Ready lifecycle로 유지한다.

### v1.15 - 2026-09-04

- `CF-FQ-045 CarFight Data Asset Management`를 Done → Historical + Retained Path로 등록했다. DAM-P0-04E USER Acceptance PASS와 P0 완료 정의 11개 충족으로 G0~G4를 닫았다.
- Current owner를 main_game `Systems/DataManagement/DataAssetManagement.md v1.0.0`으로 고정하고 representative Historical Plan을 `DataAssetManagement/DataAssetManagementPlan.md v0.2.0`으로 등록했다.
- residual P2 2건은 non-blocking polish이며 physical G5 move나 Feature 재오픈을 요구하지 않는다.

Migration: CF-FQ-045의 현재 구현은 Systems/실제 `CarFight_ReEditor/DataManagement` Source를 우선한다. retained Plan의 old next-action은 현재 착수 지시로 사용하지 않는다.

### v1.14 - 2026-09-03

- `CF-FQ-043`와 `CF-FQ-044`를 Done → Historical + Retained Path로 등록하고 Current owner를 main_game `Systems/Vehicles/VehicleBuilder.md v1.3.0`으로 동기화했다.
- CF-FQ-044는 VRCP-P0-05 USER Acceptance와 P0-06 Current System Promotion, USER explicit Catalog Save 및 persisted Wagon membership 확인까지 G0~G4를 완료했다.
- 기존 CF-FQ-040 Archived Path와 CF-FQ-042 Retained Path의 Current owner 포인터도 최신 VehicleBuilder v1.3.0으로 갱신했다. physical G5 move는 별도 maintenance로 남긴다.

Migration: CF-FQ-043/044의 현재 구현은 Systems/실제 Source를 우선하고 retained Plan의 old next-action은 현재 착수 지시로 사용하지 않는다.

### v1.13 - 2026-09-02

- CF-FQ-042 post-closure final audit P1 교정 완료를 반영해 retained Historical Plan을 v0.2.1, Current owner를 `VehicleBuilder.md v1.1.1`로 동기화했다.
- semantic lifecycle은 Done/Historical 그대로이며 physical G5 Deferred도 변경하지 않았다.

### v1.12 - 2026-09-02

- `CF-FQ-042 Vehicle Builder 신규 차량 생성 UX`를 Done → Historical + Retained Path로 등록했다. Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.1.0`이다.
- VBCUX-P0-05 A/B/C USER PASS와 Current System Promotion으로 G0~G4를 닫고 G5 physical move는 별도 maintenance로 보류했다.
- 기존 CF-FQ-040 Archived Path의 Current owner 포인터도 최신 `VehicleBuilder.md v1.1.0`으로 동기화했다.

Migration: CF-FQ-042의 현재 구현은 Systems/실제 Source를 우선하고, retained `VehicleBuilderCreationUX/` Plan의 old next-action은 현재 착수 지시로 사용하지 않는다.

### v1.11 - 2026-09-02

- `CF-FQ-040 Guided Vehicle Builder`의 G5 Physical Move를 완료해 `Archive/VehicleBuilder/`를 새 Historical 진입점으로 확정했다.
- 대표 Plan/Roadmap과 supporting Proposal/Shell/Reference/WSA 문서를 같은 기능 묶음에 보존했다.
- unrelated ConceptArt deletion 2건은 건드리지 않았다.

Migration: CF-FQ-040 Historical 문서는 이제 `Document/Plan/Archive/VehicleBuilder/`에서 찾는다. 이전 `Document/Plan/VehicleBuilder/` 경로는 2026-09-02 이전 Historical 기록에서만 해석한다.

### v1.10 - 2026-09-02

- `CF-FQ-040 Guided Vehicle Builder`를 `Done -> Historical / Retained Path`로 등록했다. Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.0.0`이다.
- VB-P0-10 closure에서 G0 Evidence, G1 Current Knowledge Promotion, G2 Current Route Cleanup, G3 Reference Preservation, G4 Historical을 PASS로 닫았다.
- G5 physical move는 완료 필수조건이 아니며 current `plan_repo` dirty/unrelated deletion 보호를 위해 수행하지 않았다. 대표 Plan v0.1.46 / Roadmap v0.1.38은 기존 `VehicleBuilder/` 경로에 보존한다.

Migration: CF-FQ-040의 현재 구현은 Systems/실제 Source를 우선하고, `VehicleBuilder/` retained Plan의 old next-action은 현재 착수 지시로 사용하지 않는다.

### v1.9 - 2026-08-29

- Current Plan 루트 구조 정리에 맞춰 완료·대체·Deprecated 구형 Plan 10묶음을 Archive로 물리 이동했다.
- Ammo와 Projectile Flight FX의 흩어진 Plan/Design/Roadmap/TaskSource/WorkOrder를 기능별 Archive 폴더로 합쳤고 초기 Launcher 작업 문서를 `LauncherMissileLegacy/`로 묶었다.
- 재생성 가능한 Generated Intermediate 및 중복 UI 예시 7파일은 영구 삭제 대신 Trash 격리했다.

Migration: 2026-08-29 이후 Current Plan은 `Document/Plan/<Feature>/`에서 찾고, 위 구형 경로는 `Document/Plan/Archive/<Feature>/`에서 조회한다.

### v1.8 - 2026-08-22

- 기존 `Historical + Retained Path` 15개 묶음의 G5 physical move를 완료해 실제 `Archive/` 경로로 정리했다.
- InGameUI 2개 대표 파일, Sensor/Scanner/WeaponData/Ammo/Defense/Projectile 4계열 단일 Plan, CombatFxAudio·ReticleAimDirection·AimFireAlignment·ProjectileContinuousCollision·HitDamage·ReticleFireFeedback 폴더, EngineSourceBuild 폴더를 Archived Path로 등록했다.
- TaskSource·Generated YAML·검증 evidence를 기능 묶음과 함께 보존했으며 semantic PASS/USER Deferred 상태는 변경하지 않았다.

Migration: 새 Historical 진입은 이 `Archive/README.md`의 실제 Archived Path를 사용한다. 기존 `Document/Plan/<완료기능>` old path는 Current 문서에서 사용하지 않는다.

### v1.7 - 2026-08-22

- Retained Path 색인의 Current owner 포인터를 post-closure 최신 Systems와 동기화했다: CF-FQ-032은 InGameUI v1.1.0 / AimReticle v1.10.0 / SensorContact v1.2.0, CF-FQ-036·037은 SensorContact v1.2.0.
- Historical Plan 자체와 v1.6 이하 Changelog의 당시 승격 버전/evidence는 변경하지 않았다.

Migration: Archive Index의 `현재 구현 owner` 열은 최신 Systems를 가리키고, 아래 과거 Changelog는 당시 상태를 보존한다.

### v1.6 - 2026-08-22

- `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`를 `Done -> Historical / Retained Path`로 등록했다.
- 현재 구현 owner는 main_game `Systems/UI/InGameUI.md v1.0.0`이며 Aim/FireFeedback 상세와 Reticle singleton/Rebind 계약은 `Systems/UI/AimReticle.md v1.9.0`이 소유한다. `InGameUIPlan.md v0.59.34`와 Roadmap은 완료 당시 설계·검증·USER Deferred 경계를 보존한다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 비차단 Deferred/Pending이며 Historical 전환으로 USER PASS 처리하지 않는다.
- 당시에는 물리 파일 이동 없이 기존 경로를 유지했다.

Migration: CF-FQ-032의 현재 구현 판단은 InGameUI Current System과 실제 Source/Asset을 우선한다. Historical Plan의 old next-action을 현재 작업으로 사용하지 않으며 Deferred Visual follow-up은 별도 재개 lifecycle로만 연다.

### v1.5 - 2026-08-18

- `CF-FQ-037 차량 스캐너 입력·장비 통합`을 `Done -> Historical / Retained Path`로 등록했다.
- 현재 구현 owner는 main_game `Systems/Targeting/SensorContact.md v1.1.0`이며 `ScannerIntegrationPlan.md v0.8.0`은 SCAN-P0-00~07, fixture RCA, final Build와 USER Acceptance evidence를 보존한다.
- 당시에는 물리 파일 이동 없이 기존 경로를 유지했다.
- Scanner 완료로 Radar Range/Zoom·동적 Blip·Radar/TargetPanel USER Visual, Sensor energy/heat·AI/Network를 완료 처리하지 않는다.

Migration: CF-FQ-037의 현재 구현 판단은 SensorContact Current System과 실제 Source/Asset을 우선하며 Historical Plan의 old next-action을 현재 작업으로 사용하지 않는다.

### v1.4 - 2026-08-15

- `CF-FQ-036 차량 센서·Contact Intelligence Runtime`을 `Done -> Historical / Retained Path`로 등록했다.
- 현재 구현 owner는 main_game `Systems/Targeting/SensorContact.md v1.0.0`이며 `SensorContactPlan.md v0.9.0`은 SEN-P0-00~07 설계·Build·Automation·Acceptance evidence를 보존한다.
- 당시에는 물리 파일 이동 없이 기존 경로를 유지했다.
- Radar Range/Zoom·동적 Blip·CF-FQ-032 Radar/TargetPanel USER Visual과 CF-FQ-026 TS-P0-08 USER PIE는 Historical 전환으로 완료 처리되지 않는다.

Migration: CF-FQ-036의 현재 구현 판단은 SensorContact Current System과 실제 Source를 우선하며 Historical Plan의 old next-action을 현재 작업으로 사용하지 않는다.

### v1.3 - 2026-08-15

- CF-FQ-008 무장 데이터 정의를 `Done -> Historical / Retained Path`로 등록했다.
- 현재 구현 owner는 `Systems/Combat/WeaponData.md`이며 `WeaponDataPlan.md v0.3.0`은 완료 당시 설계·검증 이력을 보존한다.
- 당시에는 물리 파일 이동 없이 기존 경로를 유지했다.

Migration: CF-FQ-008의 현재 구현 판단은 Systems와 실제 코드를 우선하며 Historical Plan의 과거 next-action을 현재 작업으로 사용하지 않는다.

### v1.2 - 2026-08-13

- semantic Historical과 physical placement를 분리했다.
- Ammo, VehicleDefense, Projectile/FX/Reticle/Damage 완료 Plan을 `Historical + Retained Path`로 등록했다.
- Evidence → Current Knowledge Promotion → Current Route Cleanup → Reference Preservation → Historical → optional Physical Move Gate를 추가했다.
- `EngineSourceBuild`의 기존 retained-path 운영을 일반 완료 Plan에도 적용했다.

Migration: 당시 기존 Plan 파일·폴더는 이동하지 않았다. 완료 기능의 현재 구현은 Systems가 소유하며, Historical Plan은 현재 next-action으로 사용하지 않는다.

### v1.1 - 2026-08-08

- `EngineSourceBuild/`를 Superseded / Historical 경로 유지형 종료 Plan으로 등록했다.

### v1.0 - 2026-07-14

- Plan Archive 루트 색인을 최초 작성했다.