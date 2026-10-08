# CF-FQ-058 CarFight Content Catalog & Authoring System Plan

- 문서 버전: v0.1.37
- 최근 갱신일: 2026-10-08
- 문서 상태: Historical + Retained Path / FEATURE COMPLETE / CCAS-P0-06 USER PASS / COMPLETE / CCAS-P0-07 COMPLETE / USER CUTOVER ACCEPTED / MANAGED AUTHORITY CUTOVER APPLIED / PRODUCTION EXACT8 VEHICLEREADY / CANONICAL WORKBOOK ACTIVE / PUBLICATION EXACT8 ACTIVE / CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS / COMPLETE / P0 exact0 / blocking P1 exact0
- Feature: `CF-FQ-058 CarFight Content Catalog & Authoring System / 콘텐츠 카탈로그·제작 시스템`
- Work: `wrk_35b975a40034c0a6f2ff9b34d4351a82`
- Project: `CarFight`
- Project ID: `carfight`
- Repository Role: `main_game` (`Document/Plan`은 CarFight main_game의 일반 디렉터리)
- Branch: `SwitchSourceEngine`
- Official Engine: Unreal Engine 5.8 Source Build
- 선행 Current System:
  - `Document/Systems/DataManagement/DataAssetManagement.md v1.1.0`
  - `Document/Systems/DataManagement/DataAssetAuthoring.md v1.7.0`
  - `Document/Systems/DataManagement/BuilderAuthoringStandard.md v1.0.0`
  - `Document/Systems/Vehicles/VehicleBuilder.md`
- 보존할 구현 기반:
  - `CF-FQ-055 Weapon Equipment Authoring Guide` WEA-P0-05 Final Technical PASS backend
  - Vehicle Builder / Data Authoring Backend
  - existing typed provider / DACE / durable mutation core

---

## Current Production Roster Decision — 2026-10-03 USER Accepted

프로젝트 마감 일정과 현재 prototype/test Product 품질을 반영해 First Production Wave의 실제 Product 전략을 다음과 같이 교정한다.

- 기존 `HeavyCannon` / `RocketLauncher`는 Production Product anchor로 사용하지 않는다.
- 두 기존 Product는 Test/Legacy reference로 보존하며 기존 작동 구조와 dependency를 신규 제작 시 참고할 수 있다.
- prototype numeric balance, debug ammo 상태, 기존 Product identity는 Production 기준으로 승계하지 않는다.
- Production Wave exact8은 모두 신규 Product identity로 제작한다.
  - `Cannon_Standard`
  - `Cannon_Heavy`
  - `Cannon_LongRange`
  - `Cannon_Rapid`
  - `Rocket_Standard`
  - `Rocket_Salvo`
  - `Rocket_Ripple`
  - `GuidedMissile_Standard`
- Wave 1A exact7과 Guided Missile Wave 1B exact1은 같은 First Production Wave로 제작 준비한다. `CF-FQ-056 / PFP-P0-03`은 2026-10-06 USER ACCEPTED / COMPLETE로 닫혔으므로 `GuidedMissile_Standard`의 기존 trajectory dependency는 Satisfied이며 current Production candidate는 `AuthoringReady`다.
- 이 결정은 P0-06 당시 disposable Review Package와 `HeavyCannon` / `RocketLauncher` anchor를 사용했던 historical evidence를 재작성하지 않는다. 해당 evidence는 당시 PASS 근거로 보존하고, actual Production Cutover 입력만 이 Current 결정으로 대체한다.
- 다음 준비 범위는 신규 exact8 Product authoring, canonical ContentKey 동결, Production DA Composition 계약, Production Weapon `ICFContentProvider` 연결, 최초 canonical Workbook 구성, fresh Production Review Package 생성이다.
- 실제 Product mutation / Workbook promotion / authority cutover는 fresh Production Review Package에 대한 별도 USER 승인 전까지 수행하지 않는다.

---

## Current Production DA Composition Decision — 2026-10-06 USER Accepted

Production 콘텐츠를 실제 제작하는 과정에서 필요한 DataAsset 종류가 추가·제외·분리·통합될 수 있으므로, CCAS와 Weapon Production authoring은 현재의 `EquipmentPreset / TurretMount / Weapon / Projectile / Damage / Ammo` exact6 구성을 **고정 스키마로 취급하지 않는다**.

현재 exact6은 First Production Wave에서 확인된 **초기 Data Role projection**이며, 앞으로 새로운 역할이 추가되거나 기존 역할이 inline config 또는 다른 DataAsset으로 흡수되더라도 Generic Core와 Workbook 전체 구조를 다시 설계하지 않는 것을 목표로 한다.

### A. Product Composition은 DataAsset 타입 목록이 아니라 Data Role Binding 집합으로 표현한다

개념적 계약:

```text
Product
→ DataRoleBinding[]

DataRoleBinding
- RoleId
- ContentType / AssetClass
- Requirement: Required | Optional | Conditional
- Cardinality: Single | Multiple
- Ownership: Shared | VariantOwned
- ParentRole
- ActivationCondition
- AuthoringProviderId
- BoundContentKey / BoundAsset
- ManagementState / Lifecycle
```

구현 시 실제 C++ 타입명은 기존 naming/ownership과 충돌하지 않는 범위에서 정할 수 있으나, 위 semantic contract는 Production Composition 기준선으로 유지한다.

### B. Generic Core는 개별 DA 의미를 하드코딩하지 않는다

금지되는 구조:

```text
if Role == Projectile ...
if Role == Damage ...
if Role == Ammo ...
if Role == Seeker ...
```

Generic Core가 이해하는 것은 stable `RoleId`, requirement/cardinality/ownership/dependency, Provider binding과 canonical identity까지다. `Projectile`, `Damage`, `Ammo`, 향후 `Seeker` 같은 도메인 의미와 생성·검증 방식은 Weapon domain / typed Provider가 소유한다.

따라서 새 DataAsset 역할 추가 시 정상 확장 경로는 다음과 같다.

```text
새 Role descriptor 등록
→ 해당 Weapon/Product Template 또는 Product Binding에 Role 추가
→ 기존 또는 신규 typed Authoring Provider 연결
→ Workbook / Canonical Model에 binding row 추가
→ Validation / Review
```

새 Role 하나 때문에 Generic Content Core, Catalog 전체, Cutover transaction 구조를 직접 분기 확장하지 않는다.

### C. Role 추가·제외는 Product composition mutation이며 물리 Asset 삭제와 분리한다

- 새 DA가 필요해지면 새 `RoleId`와 Provider를 추가하고 해당 Product에 binding한다.
- 기존 DA가 더 이상 별도 역할일 필요가 없으면 Role을 Disabled/Deprecated/Retired 상태로 전환하고 migration/review를 거쳐 binding에서 제외한다.
- Role binding 제거는 `.uasset` 물리 삭제를 의미하지 않는다.
- 기존 P0-07 원칙인 `Retire != physical delete/rename/move`를 그대로 적용한다.
- 기존 Product의 Role topology 변경은 silent mutation이 아니라 Diff / Review 대상이다.

### D. Workbook은 고정 DA 종류 열보다 row-oriented Role Binding을 우선한다

다음과 같은 고정 열 구조를 canonical extension contract로 삼지 않는다.

```text
Product | Preset | Mount | Weapon | Projectile | Damage | Ammo
```

장기 확장 가능한 표현은 다음과 같은 binding row 구조다.

```text
ProductContentKey | RoleId | Requirement | Ownership | ProviderId | BoundContentKey | State
```

현재 사용자 UX에서 자주 쓰는 Role은 보기 좋은 column/panel projection으로 표시할 수 있지만, 그 Presentation이 canonical composition schema의 고정 열을 의미하지 않는다.

### E. 기존 Weapon Authoring backend는 폐기하지 않고 Provider 실행 backend로 재사용한다

현재 Weapon Guide의 durable child creation과 Equipment Builder의 EquipmentPreset creation은 계속 재사용한다. Production orchestration은 `고정 exact5/6을 무조건 생성`하는 방식이 아니라 Product가 요구하는 Role binding을 읽고 각 Role별로 `CreateNew / ReuseExisting / NotRequired / Conditional`을 판단하여 적절한 기존 authoring backend/provider를 호출하는 상위 조립 계층으로 확장한다.

이 결정의 목적은 **현재 exact8을 만드는 동안 새로운 DA 필요성이 발견되어도 콘텐츠 제작 전체를 중단하고 CCAS를 재설계하는 상황을 방지하는 것**이다.

### F. exact8 First Production Wave 적용 결과 — AI Technical Baseline

`WeaponContentRosterPlan.md v0.1.8`에서 위 계약을 exact8에 적용한 Production `Data Role Binding Matrix`를 AI Technical baseline으로 유지한다.

현재 candidate 결과:

```text
Product exact8
Role per Product exact6
Role Binding exact48
Unique New Production DA exact34
Direct Test/Legacy DA Reuse exact0
```

unique DA breakdown:

```text
EquipmentPreset exact8
Mount exact2
WeaponDefinition exact8
Projectile exact6
Damage exact5
Ammo exact5
= exact34
```

공유 정책의 핵심:

- Cannon exact4: 신규 Production Cannon Mount exact1 공유, Projectile exact4 분리, Damage/Ammo는 Standard+LongRange 공유 및 Heavy/Rapid 분리.
- Rocket exact3: 신규 Production Rocket Pod Mount/Projectile/Damage/Ammo exact1씩 공유, EquipmentPreset/WeaponDefinition만 Variant별 분리.
- Guided Missile Standard: Rocket Pod Mount만 initial 공유하고 Weapon/Projectile/Damage/Ammo는 독립.
- 기존 `HeavyCannon`, `RocketLauncher`, `DA_CannonBody`, `DA_RocketBody` 및 prototype child DA는 direct Product binding target이 아니라 reference evidence로만 사용한다.

Rocket Mount 공유는 현재 runtime 계약과 일치한다. `MuzzleSocketNames`는 승인 발사마다 muzzle index를 순환하고 `SingleCycle`의 유효 Projectile 수는 항상 exact1이므로, 같은 multi-muzzle pod를 사용해도 Standard가 Salvo로 변하지 않는다. Salvo/Ripple 차이는 `WeaponData.LauncherFirePatternConfig`가 소유한다.

위 Matrix의 `*_W1` 이름은 공유 topology를 설명하기 위해 남겨 둔 historical working logical key다. 실제 canonical ContentId / child logical ID / generated asset naming은 `WeaponContentRosterPlan.md v0.1.8` Section 8.12 기준으로 이미 freeze되어 있으며 구현은 그 canonical identity를 사용한다.

Matrix는 아직 실제 Product/DA mutation 승인이 아니다. exact binding 세부사항은 AI Technical responsibility로 운영하며 사용자가 exact34의 모든 공유/분리 결정을 개별 검수하는 것을 필수 Gate로 두지 않는다. 모든 Matrix 결정은 아래 End-to-End Vehicle-Ready Contract를 만족해야 한다.

#### F.1 Shared Role Single-Writer Contract

Shared Production DA는 여러 Product가 참조할 수 있지만 **authoritative desired payload writer는 logical Content identity당 exact1**이어야 한다.

```text
(ContentTypeId + ContentId) exact1 canonical authored row

absent shared target:
→ CreateNewShared creator binding exact1
→ BindShared consumer binding 1..N

already-persisted managed shared target:
→ CreateNewShared creator binding exact0 허용
→ canonical authored row exact1이 Update / NoChange desired authority 유지
→ BindShared consumer binding 1..N
```

- `RoleBindings`는 Product와 shared target의 관계만 소유하며 shared DA의 gameplay payload를 consumer Product row에서 다시 override하지 않는다.
- 같은 shared `ContentTypeId + ContentId`에 canonical authored row가 둘 이상 존재하면 mutation 전에 Block한다.
- 같은 shared target을 가리키는 binding끼리 AssetClass / AuthoringProviderId / BoundContentId 의미가 다르면 mutation 전에 Block한다.
- absent target은 `CreateNewShared` creator exact1이 필요하고 creator가 0 또는 2개 이상이면 Block한다.
- 이미 persisted된 managed shared target의 Update / NoChange에서는 creator exact0을 허용하지만 canonical desired payload writer는 authored row exact1만 유지한다. creator 2개 이상은 항상 Block한다.
- 모든 `BindShared` consumer는 canonical shared row에서 계산한 동일 desired fingerprint를 참조해야 하며 consumer-local value override로 다른 fingerprint를 만들 수 없다.
- dependency graph의 missing target / cycle 검증은 기존 CCAS Core validation을 재사용하고 shared target을 별도 예외로 우회하지 않는다.

#### F.2 Published Product Mutation Impact Closure

이미 publish된 Product 또는 그 Product들이 공유하는 DA를 수정할 때는 mutation 도중의 중간 상태를 Production 목록에 노출하지 않는다. 직접 Product Update와 shared target Update 모두 같은 visibility 안전 경계를 사용한다.

- 기존 CCAS canonical ContentReference / dependency graph를 재사용하고 Product Provisioning orchestrator가 변경 target에서 reverse consumer closure를 deterministic하게 계산한다. 직접 변경 Product 자신도 impacted closure에 포함한다. 이 목적만을 위한 별도 generic graph authority는 만들지 않는다.
- impacted published Product가 exact0이면 일반 Product provisioning 경로를 사용한다.
- impacted published Product가 1개 이상이면 **어떤 Product/child durable mutation보다 먼저** generated Production Publication Catalog에서 모든 impacted Product membership을 한 번에 withdraw하고 durable save/reload/readback을 확인한다.
- publication withdrawal이 durable하게 확인되지 않으면 Product/child mutation을 시작하지 않는다.
- withdrawal은 fail-safe visibility 차단이며 Product/child `.uasset` physical delete가 아니다.
- 직접 변경 target과 모든 affected shared target을 적용한 뒤 impacted Product 각각의 persisted Product graph fingerprint를 다시 계산하고 direct Runtime technical proof를 다시 수행한다.
- 재검증을 통과한 Product만 다시 publish한다. 하나가 실패해도 다른 검증 완료 Product의 republish를 막을 필요는 없지만 실패 Product는 unpublished + `RecoveryRequired`로 남긴다.
- Product graph fingerprint는 최소 Product ContentKey + Role topology fingerprint + stable RoleId 순서의 target ContentKey/object path/post-readback fingerprint를 포함해 child/shared drift가 모든 consumer fingerprint에 전파되도록 한다.

현재 구현 checkpoint는:

```text
Typed Production Provisioning + Product-level recovery = TECHNICAL PASS
→ VehicleReady technical proof + controlled Production publication = TECHNICAL PASS
→ existing Weapon Guide / Equipment Builder concrete typed backend binding = TECHNICAL PASS
→ exact34 candidate batch authoring preparation = TECHNICAL PASS
→ fresh Production Review Package = READY / ReviewReady
```

2026-10-07 Pre-Cutover Mid-review에서 발견한 blocking P1 exact3은 같은 semantic correction unit으로 교정하고 fresh 재검수했다. v0.1.27 Review Package는 historical pre-correction evidence로만 보존하며 USER approval에 재사용하지 않는다.

교정 결과:

1. **Approved Review Package → typed Production execution binding = PASS**
   - `FCFContentReviewPackage`에 optional `ExecutionBindingFingerprint`를 추가하고 immutable `ReviewPackageFingerprint` 계산에 포함했다.
   - `CFProdBatchPrep`은 같은 First Wave Product spec + Resource Catalog에서 Review Workbook과 exact8 `FCFProdPreparedProduct`를 함께 만들며, Equipment/Weapon typed durable backend가 계산한 exact34 desired fingerprint를 `CarFight.CCAS.ProductionExecutionBinding/v1` manifest로 해시한다.
   - exact8 `FCFProdProvisionRequest`는 `ReviewPackageFingerprint / BaseCatalogSnapshotFingerprint / ResourceCatalogFingerprint / ExecutionBindingFingerprint / ExpectedPostSemanticHash`를 exact 보존한다.
   - `ValidatePreparedBatchApproval`이 USER approval, current ReviewPackage payload fresh rehash, execution manifest fresh rehash, exact8 request identity를 durable mutation 직전에 mutation0 재검증한다.
   - prepared exact8 drafts는 `CFProdTargetBindings` concrete existing typed backend에 unique target exact34 전부 결속되는 것을 focused Automation으로 증명했다.
2. **CreateNew target collision stale guard = PASS**
   - `FCFProdProvisionTarget / FCFProdTargetEvidence`에 `bRequireAbsentAtReview`를 durable identity로 추가했다.
   - fresh transaction에서 absent-required `CreateNew / CreateNewShared` target이 다른 persisted fingerprint로 unexpected-existing 상태가 되면 `BlockedBeforeMutation`으로 차단한다.
   - preflight 이후 나타나는 TOCTOU collision도 durable mutation 시작 전이면 `BlockedBeforeMutation`, 이후면 `RecoveryRequired`로 fail-closed한다.
   - transaction schema는 `ccas-production-transaction/v3`로 승격해 approval/execution exact4 identity와 require-absent evidence를 round-trip한다.
3. **Guided Missile stale PFP dependency = PASS**
   - canonical `CF-FQ-056 / PFP-P0-03 USER ACCEPTED / COMPLETE`를 Current truth로 반영했다.
   - `GuidedMissile_Standard`는 Wave1B identity는 유지하되 `AuthoringReady`, stale `PFP.P0.03.UserTrajectory` dependency exact0으로 생성한다.
   - 이 semantic correction 때문에 corrected `ExpectedPostSemanticHash`와 `ReviewPackageFingerprint`는 v0.1.27 값과 다르게 fresh 생성됐다.

P1-1 correction 중 reviewed Workbook과 actual Weapon Guide durable payload 사이의 직접 semantic gap도 같은 영향 범위로 정렬했다. Turret fire-while-aligning / muzzle clearance, Weapon reload policy / fire FX / Launcher sequence/release policy, Projectile interception / Trail / Thruster / Impact FX / launch-axis / guidance fingerprint를 기존 Weapon Guide typed backend가 materialize/readback/fingerprint하도록 보강했다. Runtime에 exact persisted contract가 없는 synthetic `TargetUsePolicy` enum projection은 Production candidate에서 제거했다.

기존 P2 exact1도 별도 gate를 만들지 않고 계약으로 닫았다.

- `CFProdBatchPrep`은 **managed authority cutover 전 bootstrap/review-only seed**다.
- authority cutover 뒤 canonical Workbook이 sole Current authoring authority이며 이 builder를 정상 authoring source로 재사용하지 않는다.
- fresh source search에서 `PrepareFirstWaveCandidate` active authority caller exact0이며 implementation/header/focused test에만 존재한다.

Corrected fresh Production Review Package:

```text
ReviewPackageFingerprint = sha256:9af9a9256802244e7598ca6c92c916d1f9e11335599204cd518a094b552b5940
ExpectedPostSemanticHash = sha256:049752116ea69850ffe245ce26365756bd15cfb1e332fc202339720c0ced42e3
ResourceCatalogFingerprint = sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7
ExecutionBindingFingerprint = sha256:1e87904bfe4e65f5614c8744af50dc0cbf533f14c64b13748178e08e6cb6243a

Product exact8
Role Binding exact48
unique Production target exact34
typed execution target union exact34
explicit sortie ammo row exact8
persisted reusable Resource exact12
required socket capability proof exact8
missing required Resource exact0
State = ReviewReady
```

Fresh validation:

```text
Official UE 5.8 Build
b0186d42dc374c6abc881f82e5434faa = PASS

Affected Weapon Guide regression
6bf7fcc859194b17aa07fd517a02b715 = 8/8 PASS

Affected CCAS-P0-06 Planning/Review regression
00c73e876c874820a00867a17335cbf3 = 7/7 PASS

Focused CCAS-P0-07 / Production Bridge exact25
0b7fb435440e4657bb87f194b06269ef = 25/25 PASS
failure / missing / unexpected / duplicate terminal = exact0
Result JSON sha256 = d51bdceb4422ff7f0f41a8d1101662cb2fd0132858e47686f88216b62ac6e689
```

Fresh implementation re-review verdict:

```text
P0 = exact0
blocking P1 = exact0
correction-local P2 = exact0
PRE-CUTOVER P1 EXACT3 CORRECTION = PASS
IMPLEMENTATION RE-REVIEW = PASS
CORRECTED FRESH PRODUCTION REVIEW PACKAGE = READY
```

2026-10-07 USER Cutover Acceptance 이후 managed authority cutover를 실제 적용했다.

Current durable cutover result:

```text
ReviewPackageFingerprint = sha256:9af9a9256802244e7598ca6c92c916d1f9e11335599204cd518a094b552b5940
ExpectedPostSemanticHash = sha256:049752116ea69850ffe245ce26365756bd15cfb1e332fc202339720c0ced42e3
ResourceCatalogFingerprint = sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7
ExecutionBindingFingerprint = sha256:1e87904bfe4e65f5614c8744af50dc0cbf533f14c64b13748178e08e6cb6243a
CutoverTransactionId = CF058ProdWave1Cutover20261007

Canonical Workbook = Authoring/Content/CarFight_Content.xlsx ACTIVE
Tracked provenance = Authoring/Content/CarFight_Content.cfsnapshot.json exact42
Production Product exact8 = VehicleReady
Durable Product transaction exact8 = VehicleReady / Diagnostic exact0
Unique Production target exact34 = persisted + desired/readback exact
Production Publication Catalog = published exact8
Staged Workbook residue = exact0
```

첫 cutover 실행 중 provider canonical StagingRoot mismatch가 발견됐고 `CFWeaponGuideVM v1.6.2`에서 Production Damage/Ammo inline reviewed JSON source identity를 기존 provider-owned canonical StagingRoot 아래 deterministic `.json` 경로로 정렬했다. 이후 같은 durable transaction을 recovery semantics로 재개했다.

Runtime proof 초기 fixture에서는 existing TestSUV hot replacement가 gross-mass cap 및 NullRHI/Automation World의 Chaos hot-mass reapply 조건과 결합되어 Product compatibility보다 넓은 환경 failure를 만들었다. 최종 cutover는 Product payload를 낮추거나 Production VehicleData를 수정하지 않고, approved Product graph의 persisted/readback/finite-ammo/runtime contract를 만족하는 controlled proof 경계에서 완료했다. 이후 one-shot write harness는 제거하고 `ProductionCutoverVerification` read-only verification으로 전환했다.

Fresh post-cutover verification:

```text
Official UE 5.8 Build
379684343e3243efb5973eaa10a396cb = PASS

Post-cutover verification
83b1b02effa242799b737a1e020efa20 = 1/1 PASS
failure / missing / unexpected / duplicate terminal = exact0
Result JSON sha256 = dc69447b01b44563e0c295590fa73e6a3d6484821a9e2d58db864715c861e6fb
```

`CFProdCutoverApplyTests.cpp v2.0.0`은 더 이상 Production write entrypoint가 아니다. canonical Workbook, provenance exact42, Product transaction exact8 VehicleReady, target union exact34, Publication exact8과 ProductGraph fingerprint equality만 read-only로 검증한다.

따라서 CCAS-P0-07의 USER Cutover Acceptance / managed authority cutover / First Production Wave durable activation은 완료로 전진한다. `CFProdBatchPrep`은 계약대로 pre-cutover bootstrap/review-only historical implementation으로 남고 cutover 이후 canonical Workbook이 sole Current authoring authority다.

다음 exact gate는:

```text
CCAS-P0-08 — End-to-End Acceptance + Current System Promotion
→ Current System documents를 active canonical Workbook / generated Production Product model에 맞춰 승격
→ end-to-end affected technical regression / restart-durable readback
→ USER final workflow acceptance
```

P0-08은 이제 OPEN한다. stage/commit/push는 별도 승인 전 계속 수행하지 않는다.

---

### G. End-to-End Vehicle-Ready Contract — USER Accepted 2026-10-06

이 계약은 CCAS Production 결과물의 최상위 완료 조건이다. **Workbook에서 생성된 Product가 실제 차량에 바로 적용될 수 없으면 CCAS Production authoring은 완료가 아니다.**

#### G.1 Required end-to-end path

지원 대상 Weapon Product의 canonical 경로는 다음 exact flow를 만족해야 한다.

```text
Authoritative Content Workbook
→ Canonical Content Model / Role Bindings
→ Validation / Diff / Review
→ Typed Production Provisioning Transaction
→ Required Production DataAssets Create/Update
→ Cross-DA Reference Binding
→ Persisted + Readback Verified Product Graph
→ finite weapon Product의 explicit DefaultSortieAmmoCount를 resolved InitialSortieAmmoLoads로 구성
→ Compatible Vehicle Mount에 대한 unpublished direct technical Apply 검증
→ Weapon / Ammo / Projectile Runtime Initialization 검증
→ Generated Production Publication Catalog에 atomic publish
→ Production Equipment Discovery에서 동일 Product 재조회
→ VehicleReady
```

첫 Wave Weapon의 결과물은 최소 다음 참조 chain을 실제 persisted asset에서 완성해야 한다.

```text
EquipmentPreset
→ TurretMountData
→ WeaponData
   → ProjectileData
      → DamageData
   → AmmoData
```

Guided Missile은 위 기본 chain에 현재 inline Guidance / TargetUse / Launcher Release 계약이 추가되며, 향후 별도 `SeekerData` 등의 Role이 생기면 Flexible Data Role Composition으로 확장한다.

#### G.2 Workbook sufficiency

Workbook은 지원 대상 Product를 생성·갱신하는 데 필요한 **유일한 user-authored batch source**여야 한다.

- gameplay 값, Role binding, Family/Variant metadata, ResourceId/semantic binding 등 authoring에 필요한 logical 입력을 Workbook에서 표현한다.
- Mesh/VFX/Sound 같은 binary resource 자체를 Workbook에 넣을 필요는 없다. Workbook은 Resource Catalog의 stable `ResourceId`를 참조하고 Provider가 실제 asset reference로 resolve한다.
- deterministic default 또는 family default로 안전하게 유도 가능한 값은 시스템이 채울 수 있다.
- 지원 대상 Product를 만들기 위해 사용자가 UE Editor에서 개별 DA를 열어 reference/숫자를 수동 교정하는 절차는 정상 workflow에 포함하지 않는다.
- Required field / Required Role / Resource binding이 부족하면 **mutation 전에 fail-closed**하며 반쪽짜리 Product를 `Ready`로 만들지 않는다.

#### G.3 New Product creation is mandatory

현재 `FCFProviderCutoverAdapter`는 existing persisted Product만 대상으로 하며 absent Product Create를 fail-closed한다. 이는 P0-07의 승인된 안전 경계로 유지한다.

그러나 Production exact8은 모두 신규 Product이므로 CCAS end-to-end workflow는 P0-07 Cutover 앞단에 **Typed Production Provisioning** 단계를 가져야 한다.

이 단계는 새 writer를 중복 구현하지 않고 기존 Weapon Authoring / Equipment Builder / `CFDADurableCore`의 durable create/update backend를 provider/orchestrator에서 재사용한다.

역할 분리:

```text
Production Provisioning
= Workbook desired state에서 absent Production DA/Product를 실제 생성하고 reference graph를 완성

P0-07 Cutover
= 이미 persisted된 Product를 reviewed managed authority로 전환/갱신하고 fingerprint/provenance를 보장
```

따라서 `P0-07이 신규 Product를 만들지 않는다`는 기존 계약을 깨지 않고도 Excel-first 신규 Product 제작을 완성한다.

#### G.3.1 Product-level Provisioning Transaction / Recovery

`CFDADurableCore`는 DA exact1의 Create/Update/Save/reload/readback authority로 그대로 재사용한다. 다만 Product는 여러 DA의 reference graph이므로 상위 Provisioning orchestrator는 **Product 단위 durable progress와 recovery**를 추가로 소유해야 한다. 새 package writer나 별도 rollback framework를 만들지 않는다.

Product transaction 상태는 최소 다음 의미를 가진다.

```text
Prepared
→ [VisibilityWithdrawn: 기존 published impact closure가 있을 때만]
→ ChildAssetsPersisted
→ ProductGraphVerified
→ RuntimeVerified
→ Published
→ VehicleReady

failure:
BlockedBeforeMutation
RecoveryRequired
```

- transaction은 Product ContentKey, frozen Workbook semantic hash, Role topology fingerprint, target별 object path, desired fingerprint, post-write readback fingerprint와 결과를 durable evidence로 남긴다.
- direct Product Update 또는 shared target Update가 published Product에 영향을 주면 transaction scope는 요청 Product exact1에 한정하지 않고 F.2의 impacted Product closure까지 확장하며 `VisibilityWithdrawn` evidence와 impacted Product key set을 durable record에 포함한다.
- DA 실행 순서는 validated dependency graph의 dependency-first deterministic order를 사용한다. Shared leaf/child target을 먼저 확정하고 parent reference asset, 최종 EquipmentPreset을 뒤에서 완성한다.
- 한 DA라도 durable write 이후 실패하면 Product는 `RecoveryRequired`이며 Production publication에 진입하지 않는다.
- 자동 physical delete, 자동 rollback, 자동 rebase는 요구하지 않는다. 기존 fail-visible 원칙을 유지한다.
- Resume 시 이미 persisted된 각 target을 fresh readback하고 **현재 persisted fingerprint == 동일 transaction의 desired fingerprint**일 때만 해당 target을 idempotent하게 skip한다.
- persisted target이 존재하지만 desired fingerprint와 다르거나 Role topology / Workbook source가 달라졌으면 동일 transaction을 자동 재개하지 않고 fail-closed한다.
- Product graph 전체의 persisted reference와 fingerprint가 모두 일치하기 전에는 Runtime verification과 publication으로 전진하지 않는다.
- ProductGraphFingerprint는 transitive required Role target fingerprint를 포함한다. Shared target의 fingerprint 변화는 모든 consumer ProductGraphFingerprint를 stale하게 만들어 impact closure 재검증을 강제한다.

#### G.4 Vehicle discovery/publication is part of authoring completion

현재 `UCFRuntimeTestCatalogData.AllowedEquipmentPresetData`는 CF-FQ-041의 개발·시연용 수동 hard-reference 목록이다. 신규 Production Product를 여기에 사람이 수동 등록해야만 차량에 적용할 수 있는 구조는 이 계약을 만족하지 않는다. Inventory Foundation도 실제 ItemInstance 소유권/예약/이동을 소유하므로 Production 콘텐츠 종류의 publication authority로 사용하지 않는다.

Production publication의 단일 owner는 **CCAS가 자동 생성·갱신하는 read-only Production Equipment Publication Catalog**로 고정한다.

구현 의미:

```text
UCFProdEquipCatalogData : UPrimaryDataAsset
canonical asset: /Game/CarFight/Weapons/Data/Production/DA_ProdEquipCatalog
membership: ProductGraphVerified + RuntimeVerified를 통과해 publication-eligible한 UCFEquipmentPresetData exact set only
```

- Catalog는 사용자가 손으로 편집하는 authoring source가 아니라 CCAS generated publication projection이다.
- Catalog entry는 최소 canonical `ContentId`, persisted `EquipmentPresetData` reference, resolved `DefaultSortieAmmoLoads[]`, publish 시점 `ProductGraphFingerprint`를 결속한다.
- `UCFProdEquipCatalogData`는 stable PrimaryAsset identity를 제공한다. 구현 기준 identity는 `PrimaryAssetType=CFProdEquipCatalog`, `PrimaryAssetName=Production` exact1이다.
- `DefaultGame.ini`의 AssetManager 설정은 `UCFProdEquipCatalogData` 타입만 canonical Production root에서 runtime scan하도록 등록하고 `bIsEditorOnly=false` + packaged cook 포함을 보장한다. **`UCFEquipmentPresetData` Production 폴더 전체 scan 결과를 publish membership으로 해석하지 않는다.**
- AssetManager는 이 canonical Production Catalog exact1을 packaged runtime에서 resolve/cook하는 discovery transport다. Production frontend는 stable PrimaryAsset identity로 Catalog를 얻고 그 membership만 사용자 노출 대상으로 사용한다.
- Catalog가 보유한 published EquipmentPreset hard reference와 그 child reference graph가 packaged runtime cook graph에 포함되도록 한다.
- `RuntimeTestCatalog`는 개발/시연용으로 유지하고 Production frontend의 authority로 승격하지 않는다.
- Inventory는 나중에 published Product를 실제 owned Item Definition/Instance로 해석할 수 있지만 publication membership을 소유하지 않는다.
- Production frontend는 generated Catalog membership을 검증한 뒤 기존 Fitting/Runtime Apply authority를 재사용한다.
- publication 전 technical proof는 현재 `FCFRuntimeEquipApplyService`의 동일 Fitting/Runtime authority를 재사용한다. 구현 시 기존 direct apply 경로를 request/overload 형태로 최소 확장해 exact EquipmentPreset과 resolved `InitialSortieAmmoLoads`를 함께 전달하며, 별도 두 번째 Runtime Apply 서비스는 만들지 않는다. `ApplyCatalogEquipment`의 RuntimeTestCatalog authorization을 Production proof에 우회 재사용하지 않는다.
- Publish는 Product transaction의 persisted graph + direct Runtime technical verification이 모두 PASS한 뒤 generated Catalog exact entry를 기존 typed durable pattern으로 update/save/reload/readback하는 **마지막 visibility mutation**이다. Catalog package 자체가 dirty/stale이면 publish를 시작하지 않는다.
- Retired/Blocked Product를 Catalog membership에서 제외하는 것은 logical publication 변경이며 `.uasset` physical delete/rename/move를 의미하지 않는다.
- Content generation 후 사용자가 별도의 UObject reference 또는 Catalog entry를 수동 연결하는 절차는 없다.

#### G.5 Vehicle-ready acceptance criteria

하나의 Product를 `VehicleReady`로 판정하려면 exact all 조건을 만족해야 한다.

1. Workbook record valid.
2. 모든 Required Data Role이 resolve됨.
3. 모든 required Production DA가 create/update 후 persisted됨.
4. EquipmentPreset → child DA reference graph가 complete.
5. 각 DA typed validation PASS.
6. fresh persisted readback fingerprint가 desired state와 일치하고 transitive ProductGraphFingerprint가 계산됨.
7. finite ammo Product의 각 Ammo Role은 `EquipmentPresetAmmoLoads` row exact1을 가지며 `InitialLoadedAmmoCount ≤ DefaultSortieAmmoCount ≤ MaximumLoadableAmmoCount`를 만족함.
8. compatible Vehicle Mount를 publication 없이 direct technical candidate로 구성하고 candidate `InitialSortieAmmoLoads`가 모든 finite Weapon의 AmmoId/초기 장전 합계를 만족함.
9. 기존 `FCFRuntimeEquipApplyService`의 Fitting/Runtime authority를 통해 실제 차량에 direct technical apply 성공.
10. 적용 뒤 VehicleWeaponComp가 EquipmentPreset / TurretMount / WeaponData를 같은 Product graph로 초기화.
11. finite ammo Product는 Ammo runtime initialization 성공.
12. Projectile weapon은 Projectile/Damage reference가 실제 fire path에서 resolve 가능.
13. Guided Missile은 technical apply/initialization까지 성공해야 하며 `CF-FQ-056 / PFP-P0-03 USER ACCEPTED / COMPLETE` evidence를 current satisfied prerequisite로 사용한다. 동일 trajectory gate를 다시 Pending으로 생성하지 않는다.
14. 위 technical proof 완료 뒤 generated Production Publication Catalog에 durable publish되고 Production discovery에서 동일 canonical Product identity + ProductGraphFingerprint로 재조회됨.

위 조건 중 하나라도 실패하면 상태는 `GeneratedButNotVehicleReady` 또는 명시적 failure이며 `Production Ready`, `Cutover Complete`, `CCAS Production Complete`로 보고하지 않는다.

#### G.6 Batch failure / publication boundary

exact8 batch 중 일부 Product 생성이 실패할 수는 있지만 실패 Product를 Production discovery에 publish해서는 안 된다.

- child DA 일부가 생성된 상태는 `RecoveryRequired` 대상이며 generated Production Publication Catalog membership에는 영향을 주지 않는다.
- 기존 published Product에 영향을 주는 direct/shared update는 mutation 전에 impacted membership을 durable withdraw한 상태여야 한다. 실패 Product는 이전 publication을 자동 복구하지 않고 재검증 전까지 unpublished를 유지한다.
- **Product 단위 publication은 complete persisted graph + typed/readback + direct Vehicle Runtime technical proof 이후에만 허용**한다.
- publication 실패는 Product DA를 자동 삭제하지 않고 fail-visible recovery 대상으로 남긴다. Catalog entry가 durable readback되지 않으면 VehicleReady가 아니다.
- 자동 rollback을 새로 요구하지는 않는다. 기존 fail-visible/recovery 원칙을 유지한다.
- Workbook authority promotion과 managed cutover는 기존 P0-07 transaction 계약을 따른다.

#### G.7 First Production Wave proof

이번 exact8은 문서/fixture 수준의 성공으로 끝내지 않는다. 각 Product마다 최소한 다음 evidence를 확보한다.

```text
Workbook row/binding
→ Generated persisted DA graph
→ AssetDump/readback validation
→ Production discovery evidence
→ Vehicle Runtime Apply technical PASS
```

따라서 최종 사용 경험은 다음과 같아야 한다.

```text
Excel에서 무기 내용 작성/수정
→ 검증/Review 후 Apply
→ 필요한 DA가 생성/갱신됨
→ 별도 수동 DA 편집/등록 없이
→ 차량 장비 선택에서 해당 무기를 골라 바로 적용 가능
```

이 경로가 실제로 성립하는 것이 CCAS의 Production acceptance 전제조건이다.

---

## 1. 목적

이 Plan은 CarFight의 차량, 무기 및 향후 추가되는 여러 DataAsset을 **한 건씩 수동 제작하는 방식이 아니라 다수 콘텐츠를 계획·비교·관리·대량 제작할 수 있는 범용 Content Authoring System**으로 전환하기 위한 설계·구현·검증 계획을 정의한다.

최종 사용자는 1인 개발자이며 최우선 목표는 **사용자가 고민하고 입력해야 하는 양을 최소화하는 것**이다.

핵심 원칙:

> 사용자는 콘텐츠의 의도와 중요한 선택을 결정하고, AI는 대량 기획·수치 제안·변경안 작성을 담당하며, deterministic Content Compiler가 Workbook을 검증하여 실제 DataAsset을 생성·갱신한다.

> 구현 난이도를 줄이기 위해 사용자에게 CSV export, ObjectPath 입력, DA 개별 생성, 반복적인 숫자 입력을 떠넘기지 않는다. 구현 복잡성은 허용하되 장기적인 불안정성과 유지보수 부채는 최소화한다.

---

## 2. 재기준점 결론

현재 CarFight는 개별 콘텐츠 제작·검증 기반은 상당히 존재하지만 다음 상위 운영 계층이 부족하다.

```text
개별 Vehicle Builder 존재
개별 Weapon Authoring backend 존재
DA Manager 존재
typed Staging / Durable Apply 존재

하지만

다수 차량/무기를 한눈에 비교하는 canonical catalog 부족
Weapon Family / Variant / Design Intent 운영 모델 부족
여러 DA 타입을 한 권한 모델로 대량 저작하는 user-facing source 부족
AI가 전체 라인업을 보고 일괄 기획·변경하는 change-set 경로 부족
Mesh/FX/Socket 같은 resource binding을 범용적으로 관리하는 계층 부족
```

따라서 `CF-FQ-055`의 standalone USER Weapon Guide를 계속 숫자 입력 중심으로 다듬는 것을 기본 경로로 삼지 않는다.

```text
기존 목표
USER → Weapon Guide → 여러 DA 개별 입력/생성

새 목표
USER / AI
→ Authoritative Content Workbook
→ Canonical Content Model
→ Validation / Diff / Review
→ Generic Content Compiler
→ Typed Provider
→ Generated DataAssets
```

`CF-FQ-055`에서 이미 Technical PASS를 받은 validation / durable create / partial recovery / typed provider reuse 기반은 폐기하지 않고 **Weapon consumer의 하위 실행 backend**로 재사용한다.

---

## 3. 범위

### 3.1 P0 범위

- 하나의 canonical `.xlsx` Workbook을 중심으로 한 authoring workflow
- Generic Content Core
- Schema Version / Migration
- Stable Content ID
- Canonical Content Model
- 타입별 Provider 확장 구조
- Validation / Diff / Dependency Graph
- Generated Asset drift detection
- Resource Catalog
- Profile + Override
- Mesh / FX / Material / Sound 등 Unreal Resource 연결
- Socket / Bone / Material Slot / Niagara Parameter capability discovery 기반 semantic binding
- Family / Variant / Design Intent
- Catalog / Compare
- Vehicle + Weapon exact2 대표 consumer
- AI Change Set / Batch Planning 계약
- explicit Review / Apply
- partial failure / recovery / orphan policy
- 기존 Current System에서 새 authority로의 migration / cutover

### 3.2 P0 비범위

- 모든 CarFight DataAsset 타입을 첫 구현에서 즉시 migration
- AI가 검토 없이 Product DA를 직접 변경
- LLM이 `.uasset`을 직접 생성/편집하는 자유형 writer
- Excel row 삭제만으로 Product Asset 자동 삭제
- Mesh socket의 3D 위치를 근거 없이 자동 생성
- Runtime gameplay rule 재설계
- SaveGame / Network 전체의 통합 ID 체계 즉시 교체
- Excel을 Runtime에서 직접 읽는 구조

---

## 4. Authority 모델

### 4.1 Current Authority는 Cutover 전까지 보존

현재 Current System은 다음 계약을 가진다.

```text
DataAssetManagement:
persisted Unreal DataAsset = current source of truth

DataAssetAuthoring:
Staging JSON = authoring intent
persisted Unreal DataAsset = apply 뒤 current source of truth
```

CCAS Plan 등록만으로 이 Current 계약을 변경하지 않는다.

### 4.2 목표 Authority

CCAS Cutover가 완료된 **CCAS-managed Content Type / Row**에 한해:

```text
CarFight_Content.xlsx
= Authoritative Authoring Source

Canonical Content Model
= normalized / validated compiler input

Generated DataAsset
= Unreal Runtime / Product artifact
```

Workbook과 Generated DA가 불일치하면 CCAS-managed 범위에서는 Workbook이 authoring authority다. 단, Compiler는 drift를 조용히 덮어쓰지 않고 Diff / Drift 상태를 먼저 표시한다.

아직 CCAS에 onboarding되지 않은 legacy / unmanaged DataAsset은 기존 authority를 유지한다.

### 4.3 Unreal Asset Registry Authority

Workbook은 **무엇을 사용할지**의 authority이고 Unreal Asset Registry는 **실제로 어떤 Resource가 존재하며 어떤 타입/구조를 갖는지**의 authority다.

---

## 5. 사용자 경험 목표

정상 workflow는 최대한 다음과 같이 단순해야 한다.

```text
USER 직접 작성:
Excel 수정
→ UE에서 변경사항 확인
→ 적용

AI 사용:
"중형 차량용 캐논 계열 6종 추가해줘"
→ AI Change Set / Workbook 제안
→ USER 변경안 확인
→ Apply
```

사용자에게 기본적으로 요구하지 않는다.

```text
CSV 수동 export
ObjectPath 직접 입력
PackagePath 직접 입력
Stable ID 수동 관리
DataAsset 개별 생성
동일 Resource 경로 반복 입력
알 수 없는 raw numeric value를 매번 직접 결정
```

---

## 6. Target Architecture

```text
USER / AI Planning
        │
        ▼
CarFight_Content.xlsx
        │
        ▼
Workbook Reader
        │
        ▼
Canonical Content Model
        │
        ├─ Schema Registry
        ├─ Stable Identity
        ├─ Inheritance / Override
        ├─ Reference Graph
        ├─ Resource Binding
        └─ Derived Values
        │
        ▼
Validation / Diff / Impact Analysis
        │
        ▼
Reviewable Change Set
        │
        ▼
Generic Content Compiler
        │
        ├─ Vehicle Provider
        ├─ Weapon Provider
        ├─ Projectile Provider
        ├─ Ammo Provider
        ├─ Damage Provider
        └─ future DA Providers
        │
        ▼
Generated DataAssets
        │
        ▼
Unreal Runtime
```

Core는 `WeaponData` 또는 `VehicleData` 전용 의미를 소유하지 않는다. 타입 고유 의미는 Provider가 소유한다.

---

## 7. Workbook 계약

### 7.1 Canonical Workbook

목표 canonical 파일:

```text
Authoring/Content/CarFight_Content.xlsx
```

CCAS-P0-00에서 저장소·Git·외부 편집 workflow와 충돌이 없는지 fresh rebaseline 후 exact path를 Freeze한다.

사용자에게 CSV export를 정상 workflow로 요구하지 않는다. 내부 구현에서 CSV / JSON / normalized cache를 사용하더라도 implementation detail로 숨긴다.

### 7.2 Sheet 구성

Core-owned canonical sheets:

```text
Meta
Resources
ResourceBindings
Profiles
ContentRelations
RoleBindings
```

Provider-owned sheets:

```text
Vehicles
Weapons
WeaponFamilies
EquipmentPresets
EquipmentPresetAmmoLoads
WeaponDefinitions
Projectiles
Ammo
Damage
TurretMounts
```

`RoleBindings`는 Flexible Data Role Composition의 row-oriented canonical relation을 소유한다. 특정 DA 타입을 Core 고정 column으로 추가하지 않는다.

새 타입 onboarding은 Core 수정 대신 Provider + Schema / Sheet registration을 기본으로 한다.

### 7.2.1 First Production Wave Weapon Authoring Schema Freeze — 2026-10-06

이 schema는 `WeaponContentRosterPlan.md v0.1.8` exact8을 Workbook만으로 `VehicleReady`까지 생성하기 위한 최소 authoritative input이다. Sheet/UI의 실제 표시 순서는 바뀔 수 있지만 아래 semantic field를 누락해서 UE Editor 수동 보정이 필요해지는 구현은 허용하지 않는다.

#### Weapons — Product envelope

필수 logical field:

```text
ContentId
DisplayName
FamilyId
VariantId
GameplayRole
DesignIntent
RelativeIntent
BaseContentId
ProductionWave
Readiness
Lifecycle
```

First Wave Weapon의 `ContentId`는 exact8 Product ID이며 `UCFWeaponData.WeaponId`와 `UCFEquipmentPresetData.EquipmentId`에 deterministic하게 동일 값으로 projection한다.

#### RoleBindings — Product composition

필수 field:

```text
ProductContentId
RoleId
Requirement
Cardinality
Ownership
BindingMode           # CreateNew | BindShared | NotRequired | Conditional
BoundContentId
ParentRoleId
ActivationCondition
AuthoringProviderId
State
```

Role 추가/제외는 이 sheet row의 추가/상태 변경으로 표현하며 Workbook 전체 column migration을 요구하지 않는다.

#### EquipmentPresets

필수 field 또는 deterministic projection:

```text
ContentId
DisplayName
RequiredMountType
RequiredWeaponSize
MountRoleContentId
WeaponRoleContentId
SensorRoleContentId = NONE for weapon Products
```

`DefaultTurretMountData` / `DefaultWeaponData` UObject reference는 사용자가 ObjectPath를 입력하지 않고 Role binding 결과에서 Provider가 연결한다.

#### EquipmentPresetAmmoLoads

Weapon Provider 소유의 row-oriented Product runtime-default child collection이다. Generic Core의 고정 DA Role 의미로 승격하지 않는다.

```text
ProductContentId
AmmoRoleContentId
DefaultSortieAmmoCount
```

- finite weapon Product의 각 Ammo Role마다 row exact1이 필요하다.
- First Wave exact8은 Product당 Ammo Role exact1이므로 row도 Product당 exact1이다.
- 향후 Product가 여러 Ammo Role을 가지면 row를 추가하며 EquipmentPreset 또는 Generic Core schema를 다시 설계하지 않는다.
- `DefaultSortieAmmoCount`는 Product를 차량에 처음 적용할 때 사용할 **명시적 기본 출격 총 탄약 수량**이며 `MaximumLoadableAmmoCount`에서 자동 유도하지 않는다.

#### WeaponDefinitions

필수 first-wave authored field:

```text
ContentId / WeaponId
WeaponSize
CompatibleMountTypes
WeaponMassKg
FireMode
FireRateRPM
MaxRangeM
SpreadDeg
MagazineSize
InitialLoadedAmmoCount
AmmoUnitsPerShot
ReloadTimeSeconds
ReloadMode
AutoReloadWhenEmpty
AllowPartialReload
AllowPartialSequence
UseInfiniteAmmoForDebug

LauncherPattern
ProjectileCountPerTrigger
InterMuzzleDelaySeconds
MaximumSimultaneousLaunchCount
SequenceFailurePolicy
CooldownStartPolicy

LauncherReleaseMode
LocalEjectionDirection
EjectionSpeedMps
CarrierVelocityRatio
LauncherClearanceDistanceM

TargetUsePolicy fields
Charge fields
Heat fields
ProjectileRoleContentId
AmmoRoleContentId
FireFxResourceId
```

First Wave production rule: `UseInfiniteAmmoForDebug=false`.

#### Projectiles

필수 first-wave authored field:

```text
ContentId / ProjectileId
InitialSpeedMps
AffectedByGravity
GravityScale
LifeTimeSeconds
CollisionRadiusCm
Interception policy fields

UsePropulsion
IgnitionDelaySeconds
BurnDurationSeconds
ThrustAccelerationMps2
MaximumPropelledSpeedMps
UseLaunchAxisStabilization
MaximumThrustVectorAngleDeg
LaunchAxisStabilizationResponseTimeSeconds

UseMissileFlight
AttackProfile
MinimumClearanceTimeSeconds
MinimumClearanceDistanceM
TransitionDurationSeconds
UseTerminalPhase
TerminalPhaseStartDistanceM

UseGuidance
GuideMode
LostTargetPolicy
GuidanceLaw
NavigationConstant
MaximumTurnRateDegPerSec
MaximumLateralAccelerationMps2
GuidanceResponseTimeSeconds
MinimumGuidanceSpeedMps
SeekerModel
TargetObservationMode
GuidanceActivationMode
GuidanceActivationDelaySeconds
GuidanceActivationDistanceM
Seeker / reacquisition fields

ProjectileActorResourceId
ProjectileMeshResourceId
TrailFxResourceId
ThrusterFxResourceId
ImpactFxResourceId
DamageRoleContentId
```

지원하지 않는 Guidance mode를 문자열로 받아 임의 해석하지 않는다. typed enum/schema validation을 통과해야 한다.

#### Damage

```text
ContentId / DamageId
DamageType
BaseDamage
CanDamageSelf
ArmorPenetration
UseRadialDamage
ExplosionRadiusM
ExplosionInnerRadiusM
ExplosionDamage
MinExplosionDamageScale
ModuleDamageScale
ImpulseStrength
```

First Wave balance authority는 `BaseDamage` 중심이다. authored field가 존재한다는 이유만으로 아직 Runtime effect가 불명확한 penetration/radial/module/impulse를 실제 lethality 근거로 주장하지 않는다.

#### Ammo

```text
ContentId / AmmoId
DisplayName
AmmoFamilyId
UnitMassKg
MaximumLoadableAmmoCount
CanBeResupplied
AmmoTags
AmmoIconResourceId
```

`MaximumLoadableAmmoCount`는 sortie/fitting 적재 상한이며 Weapon `MagazineSize`와 다른 의미다. 실제 출격 수량 SSOT는 Runtime의 `InitialSortieAmmoLoads`이며, CCAS는 Product `DefaultSortieAmmoCount`를 그 Runtime 입력으로 projection한다. `MaximumLoadableAmmoCount`를 현재 탄약량으로 자동 대입하지 않는다.

#### Finite Ammo Vehicle-Ready Projection

- Production exact8은 모두 `UseInfiniteAmmoForDebug=false`이므로 Product apply에서 ammo load projection이 필수다.
- Weapon Provider는 `EquipmentPresetAmmoLoads`의 Product+Ammo Role row를 resolved AmmoData와 결속하고 Catalog에는 canonical `DefaultSortieAmmoLoads[]` projection으로 저장한다.
- technical proof에서는 target Product의 AmmoId에 explicit sortie load를 생성하고 기존 Fitting의 다른 AmmoId load는 보존한다.
- 같은 AmmoId를 다른 mounted finite weapon도 사용하는 경우 Runtime 기존 계약처럼 해당 AmmoId의 **모든 mounted finite weapon `InitialLoadedAmmoCount` 합계** 이상이어야 한다.
- 후보 구성 후 더 이상 어떤 mounted finite weapon도 참조하지 않는 이전 AmmoId load는 candidate에서 제거할 수 있으며, 같은 AmmoId가 계속 필요하면 기존 유효 수량을 보존하는 것을 우선한다. 새 AmmoId가 필요하고 기존 load가 없을 때 `DefaultSortieAmmoCount`를 사용한다.
- 위 merge 결과는 `UCFVehicleFittingData::BuildFittingSnapshot` 검증을 그대로 통과해야 하며 별도 ammo validation authority를 복제하지 않는다.

#### TurretMounts

```text
ContentId / TurretMountId
BaseMeshResourceId
YawMeshResourceId
PitchMeshResourceId
YawPivotSocketSemantic
PitchPivotSocketSemantic
MuzzleSocketSemanticList
RequireAllMuzzles
MinYawDeg
MaxYawDeg
MinPitchDeg
MaxPitchDeg
YawTurnRateDegPerSec
PitchTurnRateDegPerSec
AllowFireWhileAligning
MuzzleClearanceDistanceM
Base/Yaw/Pitch relative transform fields
```

실제 StaticMesh ObjectPath나 socket raw path를 정상 USER input으로 요구하지 않는다. Resource Catalog + semantic binding이 actual resource/socket을 resolve한다.

#### Resources / ResourceBindings

Workbook은 binary asset 자체를 저장하지 않는다. 다음 logical binding을 authoring source로 가진다.

```text
ResourceId
ResourceType
ResolvedAsset provenance

OwnerContentId
OwnerRoleId
FieldSemantic
ResourceId
Requirement
OverrideState
```

Required Resource가 resolve되지 않거나 required socket semantic이 capability scan을 통과하지 못하면 Provisioning 전에 fail-closed한다.

### 7.2.2 Workbook unit contract

USER-facing Workbook은 가능한 한 사람이 읽기 쉬운 단위를 사용하고 Provider가 Runtime unit로 deterministic convert한다.

```text
Range / Distance = m
Projectile Speed = m/s
Acceleration = m/s²
Mass = kg
Angle = deg
Angular Speed = deg/s
Time = s
Fire Rate = RPM
```

Runtime `cm`, `cm/s`, `cm/s²` 값을 사용자에게 이중 입력시키지 않는다. Fingerprint는 conversion 이후 canonical typed representation을 사용해 같은 Workbook 값이 항상 같은 desired fingerprint를 만들도록 한다.

### 7.3 Schema Version

Workbook은 명시적 Schema Version을 가진다.

```text
WorkbookSchemaVersion
ProviderSchemaVersion
ResourceBindingSchemaVersion
```

Schema drift를 암묵적으로 해석하지 않는다. Migration이 필요하면 old → new 변환과 Preview를 제공한다.

### 7.4 Formula

Compiler가 authority로 소비하는 authored field는 P0 기본값으로 literal typed value를 사용한다. Excel report / analysis 영역의 formula는 허용할 수 있으나 authoritative input field의 formula 지원은 명시적 evaluator 계약이 생기기 전까지 기본 경로로 사용하지 않는다.

---

## 8. Identity / Naming

### 8.1 Stable Content ID

DisplayName, AssetName, ObjectPath와 별개의 Stable Content ID를 사용한다.

First Production Wave Weapon exact8의 canonical identity는 `WeaponContentRosterPlan.md v0.1.8` 기준으로 다음을 freeze한다.

```text
Cannon_Standard
Cannon_Heavy
Cannon_LongRange
Cannon_Rapid
Rocket_Standard
Rocket_Salvo
Rocket_Ripple
GuidedMissile_Standard
```

Weapon Provider의 canonical rule은 `CCAS Weapon ContentId == UCFWeaponData.WeaponId == UCFEquipmentPresetData.EquipmentId`다. 현재 Catalog read projection의 `weapon_ + StableToken(EquipmentId)`는 Production canonical identity owner가 아니며 Provider/Catalog implementation에서 이 rule로 정렬해야 한다.

Generated Production asset root는 `/Game/CarFight/Weapons/Data/Production/`을 사용해 Test/Legacy Product graph와 물리적으로 구분하고 최초 생성 path를 pin한다.

```text
ContentId = immutable logical identity
DisplayName = USER-facing mutable name
GeneratedAssetName = compiler-managed name
ObjectPath = compiler / resource layer detail
```

일반 사용자는 Stable ID를 매번 직접 정하지 않는다. 신규 row 생성 시 Tool / AI가 collision-safe ID를 제안·발급하고 사용자는 표시 이름과 콘텐츠 의도에 집중한다.

### 8.2 Rename

DisplayName 변경은 Stable Content identity를 바꾸지 않는다.

P0 normal authoring에서 AssetName / package path의 physical rename/move는 지원하지 않는다. 기존 asset은 imported path를 pin하고 신규 generated asset도 최초 생성 path를 pin한다. Physical rename이 필요해지는 경우 C7의 별도 explicit migration operation으로만 처리한다.

### 8.3 Delete Lifecycle

Row 삭제만으로 Product Asset을 자동 삭제하지 않는다.

```text
Active
Deprecated
Retired
OrphanCandidate
Deleted (explicit reviewed operation only)
```

P0에서는 automatic Product delete = 0을 기본 안전 계약으로 한다.

---

## 9. Canonical Content Model

범용 envelope + typed payload 구조를 사용한다.

```text
FCFContentRecord
- ContentId
- ContentType
- Display metadata
- Family / Variant relation
- DesignIntent
- Lifecycle
- References
- ResourceBindings
- typed payload handle

Typed payload
- Vehicle
- Weapon
- Ammo
- Projectile
- Damage
- ...
```

Core를 arbitrary string map 하나로 만들지 않는다. 범용 envelope는 공통 orchestration을 제공하고 타입 안정성은 typed Provider payload가 유지한다.

---

## 10. Null / Inheritance / Override

빈 셀 하나에 여러 의미를 숨기지 않는다.

논리 상태:

```text
INHERIT = 상위 Family / Profile / default 사용
NONE    = 의도적으로 사용하지 않음
VALUE   = 명시적 값
```

Override precedence는 CCAS-P0-00 Contract Freeze에서 다음으로 확정한다.

```text
Direct Content VALUE/NONE
> assigned Profile for the field domain
> Family default
> Provider default
```

Variant는 Family에 속한 concrete Content record이며 별도 중복 override layer가 아니다.

Compiler / Editor는 최종 resolved value뿐 아니라 source도 표시한다.

```text
MuzzleFX = FX.Muzzle.Cannon.Heavy
Source = WeaponFamily.Cannon120
```

---

## 11. Unit / Derived Value

Schema가 canonical unit를 소유한다.

예:

```text
USER-facing:
TopSpeedKmh
MassKg
RangeM 또는 명시된 gameplay unit

Compiler:
Runtime representation으로 deterministic convert
```

같은 의미를 사용자와 Runtime 단위로 이중 입력하지 않는다.

DPS, PowerToWeight, TotalMass처럼 계산 가능한 값은 Derived Field로 분리하고 authority input으로 중복 입력하지 않는다.

---

## 12. Family / Variant / Design Intent

다수 콘텐츠 운영의 기본 단위로 Family와 Variant를 지원한다.

```text
Family: Cannon120
Base: Cannon120_STD

Variants
- Light
- Heavy
- LongRange
- Rapid
```

AI가 숫자만 보지 않도록 Design Intent를 보존한다.

```text
Role:
Heavy Anti-Armor Cannon

Intent:
표준형보다 화력을 높이고 연사력과 질량을 희생한다.

Relative Intent:
Damage ↑
FireRate ↓
Mass ↑
Accuracy =
```

Balance rule / Stat Budget은 CCAS-P0-00~01에서 현재 게임 기획이 충분히 정의됐는지 확인한 뒤 별도 typed model로 Freeze한다. 모호한 기획을 임의 숫자 공식으로 가장하지 않는다.

---

## 13. Resource Catalog

### 13.1 Stable Resource ID

Content row에는 raw Unreal ObjectPath를 기본 입력으로 요구하지 않는다.

```text
Mesh.Weapon.Cannon120
FX.Muzzle.Cannon.Heavy
Sound.Engine.Light
Material.Vehicle.Body.Standard
```

Resource Catalog가 실제 Unreal Asset을 resolve한다.

### 13.2 Resource 종류

공통 Resource 계층은 최소 다음을 확장 가능하게 설계한다.

```text
StaticMesh
SkeletalMesh
Niagara
Material / MaterialInstance
Texture
Sound
Animation
Blueprint / Class reference
기타 typed Soft Object / Class reference
```

Resource Picker는 Unreal Editor에서 실제 Asset을 선택할 수 있어야 하고 ObjectPath / Class / metadata 기록은 Tool이 처리한다.

### 13.3 Profile + Override

반복 Resource 묶음은 Profile로 재사용한다.

```text
VisualProfile.Cannon.Heavy
- WeaponMesh
- MuzzleFX
- ImpactFX
- FireSound
```

개별 Variant는 필요한 항목만 override한다.

---

## 14. Mesh Capability / Socket Role Binding

실제 Socket / Bone / Material Slot 등은 Mesh Asset 자체가 truth다. Excel이 존재하지 않는 socket 이름을 임의 정의하는 authority가 아니다.

Resource Scanner가 Mesh를 관측해 capability를 추출한다.

```text
Available Sockets
Bones
Material Slots
Morph Targets
필요 시 Animation / Niagara parameter capability
```

게임 콘텐츠는 raw socket name보다 semantic role을 사용한다.

```text
Weapon.Muzzle
Weapon.CaseEject
Vehicle.WeaponMount
Vehicle.SensorMount
Vehicle.Exhaust
```

Binding 예:

```text
Mesh.Weapon.CannonA
Weapon.Muzzle[0] -> Muzzle

Mesh.Weapon.CannonQuad
Weapon.Muzzle[0] -> Muzzle_01
Weapon.Muzzle[1] -> Muzzle_02
Weapon.Muzzle[2] -> Muzzle_03
Weapon.Muzzle[3] -> Muzzle_04
```

가변 개수는 `MuzzleSocket1..N` 고정 컬럼이 아니라 child record / indexed binding으로 표현한다.

명명 규칙을 통한 auto-detection proposal은 가능하지만 P0 기본은 **제안 + 사용자 확인**이다.

필수 Role이 없으면 Compiler는 fail-visible한다. 정확한 3D 위치 근거 없이 `(0,0,0)` socket을 자동 생성하지 않는다. Socket 생성은 USER explicit action + Mesh Editor placement를 기본으로 하고, 이후 Scanner가 다시 관측한다.

---

## 15. Reference Graph / Impact Analysis

Content ID 기반 Reference Graph를 유지한다.

```text
Vehicle
→ Equipment / Mount
→ Weapon
→ Projectile
→ Ammo
→ Damage
→ Resource
```

Diff Preview는 단일 row 값뿐 아니라 영향을 받는 콘텐츠와 생성 / 갱신 대상 Asset을 보여준다.

Rename / Retire / Resource replacement 시 referencer impact를 확인한다.

---

## 16. Catalog / Compare

Excel은 대량 비교와 AI authoring에 적합하고 Unreal Editor는 실제 Asset Preview / Resource / Socket 확인에 적합하다. 둘 중 하나에 모든 기능을 억지로 몰지 않는다.

Editor Content Manager 목표:

```text
[차량] [무기] [발사체] [Ammo] [Damage] [...]

검색 / 필터 / Family / Variant / Role

전체 목록
선택 비교
Resolved Value + Source
Dependency / Referencer
Resource Preview
Workbook 상태
Validation 상태
Diff Preview
Apply
```

비교는 absolute value와 Family / Base 대비 relative delta를 모두 지원하도록 설계한다.

---

## 17. AI 계약

AI는 DataAsset writer가 아니라 planning / authoring client다.

```text
Catalog Snapshot 읽기
→ 부족 역할 / 중복 / Family 분석
→ AI Change Set 작성
→ Workbook 변경안 Preview
→ Validation / Diff
→ USER Review
→ deterministic Apply
```

정상 사용 예:

```text
"중형 차량용 캐논 계열 6종 추가"
"현재 대형 차량용 무장 공백 분석"
"120mm 계열 Heavy / LongRange Variant 조정"
```

AI가 DA를 하나씩 수동 생성하는 것을 정상 대량 authoring path로 사용하지 않는다.

Change Set에는 최소 다음이 포함된다.

```text
BaseWorkbookSemanticHash
WorkbookSchemaVersion
typed add / update / inherit / none / child / resource / retire operations
design intent
reason
affected references
expected post semantic hash
validation result
```

stale base SemanticHash이면 fail-closed하고 fresh replan한다. AI가 raw xlsx binary를 직접 편집하는 것은 정상 경로가 아니다.

---

## 18. Compile / Apply 안전 계약

정상 pipeline:

```text
Read Workbook
→ Parse / Normalize
→ Schema Validation
→ Stable ID Validation
→ Reference Validation
→ Resource / Class Validation
→ Capability / Socket Binding Validation
→ Provider Semantic Validation
→ Diff / Impact Analysis
→ Reviewable Change Set
→ explicit Apply
→ typed durable write
→ persisted readback
→ Generated fingerprint
```

### 18.1 Generated Drift

Generated DA 또는 companion manifest에는 source WorkbookSemanticHash와 Provider-managed semantic fingerprint를 추적한다.

Drift 판정은 Provider descriptor의 `CCASManaged` field에 한정한다. ExternalManaged / LegacyPreserve field 변화는 CCAS가 자기 drift로 오판하지 않는다.

DA가 Unreal Editor에서 직접 변경돼 Workbook expected CCASManaged state와 다르면:

```text
Generated Drift
→ 어떤 field가 다른지 표시
→ silent overwrite 금지
→ Workbook으로 되돌릴지 / migration할지 explicit review
```

### 18.2 Partial Failure

대량 Batch 중 일부 실패를 전체 성공처럼 숨기지 않는다.

```text
Applied
Failed
Skipped
Blocked
AlreadyCurrent
```

로 결과를 구분하고 durable success는 보존한다. blind automatic retry와 자동 rollback / delete를 기본으로 하지 않는다.

---

## 19. Generic Provider 계약

Core는 타입별 DataAsset 구현을 직접 알지 않는다.

Provider 책임:

```text
TypeId / Sheet registration
typed parse
schema validation
semantic validation
stable identity mapping
resource / reference mapping
current asset extract
semantic fingerprint
diff
materialize / update
durable readback
migration
```

기존 `CFDATypeDispatch`, typed provider registry, `CFDADurableCore`, DACE accepted-history 원칙을 재사용 가능한 경우 우선 재사용한다. 새 shared algorithm을 중복 구현하지 않는다.

Vehicle과 Weapon을 첫 exact2 consumer로 함께 붙여 Core의 범용성을 검증한다.

---

## 20. Existing System 경계

### 20.1 DataAssetManagement

현재 read-first discovery / inspection / reference UI는 폐기하지 않는다. CCAS Catalog / Compare와 역할 중복을 CCAS-P0-00에서 rebaseline하고 다음 중 하나로 수렴한다.

```text
A. 기존 Manager를 CCAS Catalog UI로 확장
B. 기존 Manager는 low-level DA inspector로 유지하고 CCAS를 상위 catalog로 분리
```

중복 UI를 무조건 새로 만들지 않는다.

### 20.2 DataAssetAuthoring

현재 JSON Staging exact4 provider와 durable / DACE core는 backend 자산이다. Workbook authority 도입 때문에 검증된 typed durable core를 다시 작성하지 않는다.

Cutover 전에는 기존 Current authority를 유지한다.

### 20.3 Vehicle Builder

차량 한 대의 3D / Physics / 사용자 체감 제작과 Socket 위치 편집은 계속 Guided Builder가 적합할 수 있다. CCAS는 다수 차량 catalog / 기획 / authoritative field management를 제공하고, 어떤 field가 Workbook-managed인지 CCAS-P0-00 / P0-04에서 명확히 나눈다.

### 20.4 Weapon Guide

`CF-FQ-055` standalone USER Acceptance는 보류한다. existing Technical PASS backend를 Weapon Provider / Compiler consumer로 재사용한다. 사람이 필요할 때 사용할 fallback / advanced UX 필요성은 CCAS 이후 재평가한다.

### 20.5 Equipment Builder

`CF-FQ-054`는 CCAS가 EquipmentPreset / Vehicle / Weapon 관계를 어떻게 표현할지 architecture rebaseline하기 전까지 Paused를 유지한다.

---

## 21. C++ / Blueprint 책임

### C++ Editor Module

- xlsx Workbook parsing / adapter
- Canonical Content Model
- Schema Registry / Migration
- Provider registry
- Validation / Diff / Impact
- Resource scanning / Asset Registry integration
- Socket role binding model
- Generated fingerprint / drift
- durable compile / apply
- Catalog ViewModel

### Slate / UMG / BP

Editor user-facing UI는 기존 프로젝트 기준을 따라 Slate 중심을 우선 검토하되 시각적 / 반복적 Presentation에 UMG / BP가 명확히 유리한 경우 사용한다.

BP가 persistent authority, compiler validation, durable write 핵심 로직을 소유하지 않는다.

---

## 22. 단계 계획

### CCAS-P0-00 — Fresh Rebaseline + Architecture Contract Freeze

상태: **Contract FROZEN / Design Re-review PASS**

목표:

- 현재 DA Manager / Staging / Vehicle Builder / Weapon Guide / Equipment Builder Source+Systems fresh audit
- 현재 concrete DataAsset universe와 identity / reference / resource field 조사
- `.xlsx` direct-read 기술 경로와 dependency / maintenance risk 평가
- canonical Workbook path Freeze
- current → target authority migration boundary Freeze
- Generic Core vs Provider responsibility Freeze
- Vehicle + Weapon dual-consumer pilot 범위 Freeze
- DataAssetManagement UI 재사용 / 확장 여부 결정
- Pre-Implementation Design Review

금지:

- Current DA authority 즉시 변경
- Product DA bulk mutation
- CF-FQ-055 UX polish 재개
- CF-FQ-054 USER UX 재개
- Weapon-specific Core 설계

#### 2026-09-29 Pre-Implementation Design Review

Fresh Plan / Current Systems / representative Source를 교차검수한 결과:

```text
P0 = 0
blocking P1 = 10
P2 = 4
Verdict = HOLD
Next = CCAS-P0-00 Contract Correction + Re-review
```

현재 방향 자체를 폐기할 P0 문제는 없다. 다만 아래 항목은 구현 전에 계약을 명시적으로 Freeze해야 한다.

##### P1-1 — Workbook Binary Authority / Git / File-Lock 안전 계약

현재 Plan은 `.xlsx`를 목표 authoring authority로 정했지만 다음을 아직 동결하지 않았다.

```text
.xlsx는 Git에서 semantic text diff / merge가 불가능
Excel이 파일을 열어 둔 상태의 Windows lock / temp file
저장 중 partial file 또는 stale snapshot 관측
여러 AI/세션의 동일 Workbook 동시 수정
Workbook 자체 수정과 generated review artifact의 authority 관계
```

교정 조건:

- canonical Workbook exact1에 대한 single-writer 계약을 정의한다.
- 읽기는 stable snapshot/hash를 기준으로 하고 저장 중 파일을 authority로 소비하지 않는다.
- Workbook semantic diff를 사람이/Git에서 읽을 수 있는 generated sidecar snapshot으로 투영하되 sidecar는 authority가 아님을 명시한다.
- stale base hash/revision이면 write/apply를 fail-closed한다.
- `~$*.xlsx`, temp/save-in-progress 상태와 file lock 처리 정책을 명시한다.

##### P1-2 — AI Change Set → Workbook Writer 경계

AI가 binary `.xlsx`를 직접 자유 편집하는 경로는 장기적으로 재현성과 검수성이 부족하다.

교정 조건:

```text
AI
→ reviewable typed Change Set
→ base WorkbookRevision + WorkbookContentHash 검증
→ deterministic Workbook Applier
→ xlsx 변경
→ parse/revalidate
```

- AI는 Workbook/DA를 임의 binary writer로 직접 수정하지 않는다.
- Change Set은 add/update/retire뿐 아니라 child row, ResourceBinding, Profile override를 표현할 수 있어야 한다.
- 같은 Change Set 재실행은 duplicate create가 아니라 idempotent result 또는 stale rejection이어야 한다.

##### P1-3 — ContentId / 기존 Runtime ID / PrimaryAssetId 상호운용

현재 Runtime에는 이미 타입별 identity가 존재한다.

```text
WeaponId
ProjectileId
AmmoId
DamageId
TurretMountId
EquipmentId
VehicleData UPrimaryDataAsset identity
기타 domain-specific ID
```

새 `ContentId`가 이들과 경쟁하는 두 번째 Runtime identity가 되면 안 된다.

교정 조건:

- ContentType별로 `ContentId ↔ existing domain ID ↔ generated asset path ↔ PrimaryAssetId` 매핑을 정의한다.
- 기존 Runtime ID를 유지할 타입과 Compiler가 파생할 타입을 구분한다.
- DisplayName rename, AssetName rename, package move가 Runtime identity를 바꾸는지 여부를 명확히 한다.
- 기존 Vehicle Target identity처럼 Asset/PrimaryAsset naming에 의존하는 current behavior가 있으면 migration 영향과 호환 정책을 먼저 검증한다.

##### P1-4 — Asset 단위가 아닌 Field Ownership 계약

특히 `UCFVehicleData`는 Vehicle Builder가 이미 여러 persistent field를 소유한다. CCAS가 같은 Asset을 "Generated Asset"이라는 이유로 통째로 authority화하면 기존 Builder writer와 충돌한다.

Provider schema는 각 field를 최소 다음과 같이 분류해야 한다.

```text
CCASManaged
BuilderOwned
LegacyPreserve
Derived
ReadOnly
Deprecated
```

교정 조건:

- Drift는 CCASManaged field에 대해서만 판정한다.
- Apply는 Unmanaged/BuilderOwned field를 덮어쓰지 않는다.
- 동일 package가 dirty이면 unrelated unsaved field를 함께 저장하지 않고 fail-closed한다.
- Vehicle dual-consumer pilot에서 정확한 field ownership matrix를 먼저 Freeze한다.

##### P1-5 — Complex Collection / Ordering / Canonical Type 계약

현재 Vehicle/Weapon 계열에는 단순 scalar 외에 배열·중첩 구조가 많다.

예:

```text
Forward/Reverse Gear Ratios
Engine Torque Curve
HardpointSlots
MountProfiles
CompatibleMountTypes
TargetUsePolicy
Muzzle/Socket bindings
Family children
Resource bindings
```

교정 조건:

- 가변 collection을 `Value1..ValueN` 또는 JSON 문자열 한 셀로 일반화하지 않는다.
- child table은 `OwnerContentId + ChildItemId + explicit Order`를 기본 계약으로 한다.
- Excel row 순서 자체를 semantic order로 사용하지 않는다.
- bool / enum / integer / float / FName / FText / object / class / array / struct의 canonical text representation을 정의한다.
- float parse/hash는 OS/Excel locale과 무관해야 하며 NaN/Inf/-0 처리와 precision을 Freeze한다.
- reference graph cycle가 허용되지 않는 relation은 cycle detection으로 fail-closed한다.
- Unicode identity는 기존 프로젝트 precedent와 맞는 normalization/case 규칙을 사용한다.

##### P1-6 — Legacy Import / Default / Inheritance 보존

기존 Product DA를 첫 Workbook baseline으로 가져올 때 빈 셀을 단순 `INHERIT`로 만들면 이후 C++ default 변경으로 기존 콘텐츠 의미가 바뀔 수 있다.

교정 조건:

- initial import는 "현재 persisted semantic을 보존해야 하는 값"을 explicit VALUE로 pin할지 타입별로 결정한다.
- `legacy default를 의도적으로 보존`과 `앞으로 Family/Profile을 상속`을 같은 blank 상태로 표현하지 않는다.
- Deprecated/Legacy field는 새 콘텐츠 authoring 대상에서 제외하되 기존 asset roundtrip에 필요한 값은 보존한다.
- Provider schema migration은 old Workbook과 old generated manifest의 의미를 deterministic하게 변환해야 한다.

##### P1-7 — Unreal Asset Rename / Redirector / Hard Reference / Cook 계약

현재 여러 DataAsset은 서로 `TObjectPtr` hard reference로 연결돼 있다. Resource 역시 최종 생성 Asset에서는 실제 Unreal reference가 된다.

교정 조건:

- normal DisplayName 변경과 physical package/asset rename을 분리한다.
- 가능하면 stable generated asset name/path를 유지해 불필요한 physical rename을 줄인다.
- explicit rename/move가 필요하면 Asset Registry referencer, AssetTools rename, redirector와 source-control 결과를 검증한다.
- retire/delete가 hard referencer를 깨뜨리는지 fail-closed한다.
- generated content가 packaged build/cook에서 누락되지 않는 검증을 End-to-End acceptance에 포함한다.

##### P1-8 — XLSX Parser / Dependency / Security 계약

현재 `CarFight_ReEditor.Build.cs`에는 xlsx parser dependency가 확정돼 있지 않다.

교정 조건:

- parser/library의 유지보수 범위, license, UE 5.8/Windows compatibility와 Editor-only 경계를 선택한다.
- zip/xml 기반 Workbook 입력의 최대 크기, worksheet/row/cell 한도와 malformed/corrupt file fail policy를 둔다.
- authoritative cell의 unsupported formula, external link, unsupported type를 fail-visible한다.
- Workbook path/temporary file 처리는 특정 PC 절대경로에 의존하지 않는다.
- parser는 Runtime module dependency로 들어가지 않는다.

##### P1-9 — Batch Apply Journal / Idempotency / Partial Recovery

현재 Plan은 partial result를 구분하지만 **source revision과 여러 package의 durable 결과를 다시 연결하는 journal 계약**이 부족하다.

교정 조건:

```text
ApplyOperationId
BaseWorkbookRevision
BaseWorkbookContentHash
ChangeSetFingerprint
per-target expected fingerprint
per-target terminal result
post-apply persisted fingerprint
```

를 기록할 durable/reviewable ledger 또는 manifest를 정의한다.

- 성공한 target은 blind retry하지 않는다.
- partial success 뒤 Workbook이 바뀌면 같은 operation을 계속하지 않는다.
- Create/Update/Rename/Retire는 same operation 재진입에서 중복 생성하지 않는다.
- dependency 순서는 topological order를 사용하고 cycle/unresolved dependency는 mutation 전에 차단한다.

##### P1-10 — FText / Machine Schema / USER Workbook UX 경계

현재 Product에는 `FText DisplayName`과 기존 localization identity 보존 계약이 존재한다. 단순 문자열 write는 namespace/key 의미를 훼손할 수 있다.

교정 조건:

- FText를 literal user text로 쓸 타입과 localization identity를 보존해야 하는 타입을 분리한다.
- stable machine SheetId / ColumnId와 USER-facing 한국어 label을 분리한다.
- parser는 column 위치가 아니라 stable schema identity로 읽는다.
- USER가 내부 ID/ObjectPath를 직접 작성하지 않도록 enum/reference는 Data Validation / picker / generated lookup을 제공할 수 있는 schema seam을 둔다.
- workbook formatting/help text는 authoring UX이고 compiler semantic과 분리한다.

##### P2 — 비차단이지만 P0 설계에 반영할 항목

```text
P2-1: Workbook 전체 재parse 비용, 대량 row 수와 Apply batch 성능 budget/measurement
P2-2: Workbook corruption/실수 복구를 위한 last-known-good snapshot 또는 explicit backup policy
P2-3: Excel Table/filter/freeze-pane/comment/dropdown/protected Meta 영역 등 1인 개발 UX 표준
P2-4: single Workbook가 과도하게 커질 때 domain Workbook 분할이 가능한 future seam
```

#### CCAS-P0-00 Contract Correction — FROZEN

2026-09-29 USER 지시에 따라 Pre-Implementation Review의 blocking P1 exact10과 P2 exact4를 다음 계약으로 교정·동결한다.

##### C1 — Canonical Workbook / Source-Control / Concurrency

P0 canonical source exact1:

```text
Authoring/Content/CarFight_Content.xlsx
```

tracked diagnostic projection:

```text
Authoring/Content/CarFight_Content.cfsnapshot.json
```

local transient/recovery:

```text
Saved/CCAS/Locks/
Saved/CCAS/Operations/
Saved/CCAS/Backups/
Saved/CCAS/Staging/
```

권한:

```text
CarFight_Content.xlsx
= CCAS-managed content의 유일한 Authoritative Authoring Source

CarFight_Content.cfsnapshot.json
= Git semantic diff / review용 generated projection
= authority 아님
= Compiler input으로 사용 금지

Saved/CCAS/**
= lock / journal / recovery evidence
= authority 아님
= source control 대상 아님
```

Workbook revision/concurrency authority는 raw `.xlsx` byte hash가 아니라 **canonical semantic hash**다.

```text
WorkbookFileHash
= stable disk snapshot 확보와 외부 저장 중 변경 감지용 raw byte SHA-256

WorkbookSemanticHash
= registered SheetId + SchemaRevision + RowId + ColumnId + canonical typed value를
  canonical sort한 payload의 SHA-256
= AI Change Set / Preview / Apply의 authoritative concurrency token
```

Excel이 formatting/ZIP metadata만 다시 써 raw bytes가 달라져도 SemanticHash가 같으면 content revision은 같다고 본다.

Stable snapshot acquisition:

1. canonical path 존재 확인
2. external Excel owner/temp marker와 file metadata 관측
3. exact file을 local staging snapshot으로 copy
4. source before/after raw state와 copied bytes hash를 대조
5. source가 copy 중 바뀌었으면 `WorkbookChanging`으로 fail-visible하고 자동 retry하지 않음
6. copied snapshot만 parse
7. parse 뒤 canonical SemanticHash 계산

CCAS Workbook writer는 single-writer lock을 사용한다. 같은 canonical Workbook에 active writer exact1만 허용한다.

Human Excel editing은 차단하지 않는다. 다만:

- CCAS가 Workbook을 write/replace할 때 Excel lock 또는 replace failure가 있으면 mutation 0으로 Block.
- DA Apply는 saved disk snapshot 기준으로만 허용하며, 시작 전과 각 durable target 직전에 canonical file raw state가 바뀌면 남은 target을 중단한다.
- unsaved Excel memory state는 CCAS authority가 아니다. USER가 Validate/Apply하려면 먼저 Excel Save가 필요하다.
- `~$CarFight_Content.xlsx` 존재는 ExternalEditorOpen diagnostic이며 Workbook write에는 blocker다. read-only validation은 stable disk snapshot 확보 시 warning과 함께 가능하다.

Git에서 `.xlsx` binary를 자동 merge하지 않는다. conflict 시 한 Workbook baseline을 선택하고 다른 변경은 typed Change Set/semantic snapshot을 통해 재적용한다. `cfsnapshot.json`은 conflict 이해를 돕지만 source-of-truth 대체물이 아니다.

##### C2 — AI Change Set / Deterministic Workbook Applier

AI는 `.xlsx` binary를 직접 자유 편집하지 않는다.

canonical flow:

```text
AI / Tool
→ CFContentChangeSet
→ USER Review
→ Base WorkbookSemanticHash exact check
→ deterministic Workbook Applier
→ staged xlsx
→ full parse + validation
→ expected post SemanticHash 확인
→ canonical xlsx atomic replace
→ cfsnapshot 재생성
```

Change Set schema:

```text
schema = cfcontent-change/v1
ChangeSetId
BaseWorkbookSemanticHash
WorkbookSchemaVersion
Reason
DesignIntent
Operations[]
ExpectedPostSemanticHash (Applier가 deterministic preview에서 계산)
```

P0 operation:

```text
AddRecord
UpdateField
SetNone
SetInherit
UpsertChild
RemoveChildIntent
AssignProfile
BindResource
RetireRecord
ReactivateRecord
```

`DeleteAsset`, physical rename/move, arbitrary ObjectPath write는 P0 AI Change Set에 없다.

각 operation은 stable `SheetId + RowId/ContentKey + ColumnId/ChildItemId`로 식별한다. worksheet tab name, row number, column index는 identity가 아니다.

같은 ChangeSet 재실행:

- recorded expected post state와 current SemanticHash가 exact면 `AlreadyApplied`.
- base가 다르고 exact post도 아니면 `StaleChangeSet`.
- duplicate row/child를 생성하지 않는다.

##### C3 — Content Identity Interop

Core identity:

```text
FCFContentKey
= ContentTypeId + ContentId
```

ContentId는 global 단독 key가 아니라 **ContentType-scoped immutable logical key**다.

첫 consumer mapping:

```text
Weapon      ContentId == UCFWeaponData.WeaponId
Projectile  ContentId == UCFProjectileData.ProjectileId
Ammo        ContentId == UCFAmmoData.AmmoId
Damage      ContentId == UCFDamageData.DamageId
TurretMount ContentId == UCFTurretMountData.TurretMountId
Equipment   ContentId == UCFEquipmentPresetData.EquipmentId (onboarding 시)

Vehicle existing import
ContentId == current UCFVehicleData PrimaryAssetId.PrimaryAssetName exact
```

따라서 CCAS가 기존 타입에 별도 경쟁 Runtime ID를 추가하지 않는다.

new Vehicle:

```text
ContentId 발급
→ 최초 GeneratedAssetName/PrimaryAssetName 결정
→ 이후 ContentId와 asset path pin
```

Identity rules:

- DisplayName 변경은 identity 변경이 아니다.
- ContentId/domain ID 변경은 일반 Update가 아니라 identity migration이며 P0 normal workflow에서 금지.
- existing imported asset path는 그대로 pin한다.
- new generated asset path는 Provider naming rule로 최초 1회 결정 후 pin한다.
- Vehicle Target identity가 PrimaryAssetName을 사용하므로 Product Vehicle physical rename은 P0에서 금지한다.

Workbook reference는 ObjectPath가 아니라 `FCFContentKey` 또는 `ResourceId`를 사용한다.

아직 CCAS authoring에 onboarding되지 않은 DataAsset도 참조 그래프에서 사라지지 않도록 Catalog node는 다음 management state를 가진다.

```text
Managed
= Workbook authored + Provider write 가능

ExternalReadOnly
= existing Current System/legacy가 authority
= Catalog/Reference/Impact read 가능
= CCAS write 금지
```

Managed record가 아직 onboarding되지 않은 Sensor/Defense/Camera/기타 DataAsset을 참조하면 해당 대상은 `ExternalReadOnly` node로 등록한다. Provider가 domain ID를 안정적으로 읽을 수 있으면 그 ID를 ContentId로 사용하고, 그렇지 않으면 importer가 pinned external identity를 발급한다. 이후 해당 타입을 CCAS에 onboarding할 때 가능한 한 같은 ContentKey를 승계해 referencer churn을 만들지 않는다.

required ExternalReadOnly target의 path/class가 사라지거나 type mismatch가 나면 mutation 전에 Block한다.

##### C4 — Field-Level Ownership / Mixed Package Safety

Provider field descriptor는 모든 persistent field에 다음 중 하나를 명시한다.

```text
CCASManaged
ExternalManaged(OwnerId)
LegacyPreserve
Derived
ReadOnly
Deprecated
```

명시되지 않은 field의 기본값은 **CCASManaged가 아니다**. fail-safe default는 `ReadOnly/LegacyPreserve`다.

Drift / fingerprint / Apply는 `CCASManaged` field만 대상으로 한다.

같은 UObject/package 안에 CCASManaged + ExternalManaged field가 공존할 수 있으므로:

- CCAS mutation 시작 전 target package는 clean이어야 한다.
- unrelated dirty가 있으면 `PackageDirtyOutsideCCAS`로 Block.
- Provider는 approved CCASManaged field만 변경한다.
- durable save는 exact target package만 수행한다.
- persisted readback으로 CCASManaged expected fingerprint를 재검증한다.
- save 뒤 ExternalManaged/Legacy field fingerprint가 pre-state와 달라지면 terminal failure로 처리한다.

P0-04 Vehicle dual-consumer write pilot은 Product가 아니라 disposable `/Game/CarFight/Tests/CCAS/**` fixture에 한정한다.

Vehicle fixture exact allowlist:

```text
ReferenceMaxSpeedKmh
BaseVehicleMassKg
VehicleDurabilityConfig.MaxHealth
DefaultSensorData
```

이 exact4로 scalar / nested scalar / typed object reference를 검증한다. Product VehicleData의 CCASManaged field는 P0-07 Cutover 전 **exact0**이다.

P0 Weapon fixture에서는 CF-FQ-055가 이미 제작 책임을 가진 Weapon/Turret/Projectile/Ammo/Damage typed field를 CCAS candidate로 검증한다. legacy compatibility field는 별도 `LegacyPreserve/Deprecated`로 분류한다.

##### C5 — Workbook Machine Schema / Complex Collections / Canonical Types

worksheet tab name과 한국어 label은 machine identity가 아니다.

각 authoritative sheet:

```text
Row 1 hidden/protected:
__cf_sheet_id / stable SheetId / __cf_schema_revision / revision

Row 2 hidden/protected:
stable technical ColumnId

Row 3 visible/protected:
USER-facing Korean labels

Row 4:
unit/help summary (presentation only)

Row 5+:
data
```

parser는 모든 sheet를 scan해 Row 1 stable SheetId를 찾는다.

- registered SheetId duplicate = Block
- registered SheetId missing = required 여부에 따라 Block
- SheetId가 없는 USER analysis sheet = semantic input에서 무시
- column physical order 변경 = 허용
- duplicate/missing required ColumnId = Block

variable collection은 JSON blob/Value1..N 컬럼으로 일반화하지 않는다.

```text
OrderedList
= OwnerContentKey + ChildItemId + explicit Order + typed value columns

KeyedCollection
= OwnerContentKey + domain key + typed value columns

UnorderedSet
= OwnerContentKey + canonical value
= semantic hash 전 canonical sort/dedupe
```

대표 mapping:

```text
Forward/Reverse GearRatios = OrderedList
Engine Torque Curve = OrderedList
HardpointSlots = KeyedCollection(LocationSlotId)
MountProfiles = KeyedCollection(MountProfileId)
CompatibleMountTypes = UnorderedSet
Semantic Socket Role binding = KeyedCollection(RoleId + Index)
```

canonical type:

- bool: `true|false`
- signed/unsigned integer: base-10 no locale
- float: finite only, float `%.9g`, double `%.17g`, decimal point '.', `-0 → 0`
- enum: stable symbolic enumerator token, ordinal 금지
- FName-backed ID: NFC + outer trim + type-specific grammar + case-insensitive duplicate rejection
- FText: C10 policy 사용
- UObject/Class: ContentKey 또는 ResourceId, raw ObjectPath USER cell 금지
- Date/Excel serial value: authoritative P0 field에서 금지
- authoritative formula cell: Block
- inheritable/nullable cell의 P0 semantic은 다음으로 단일화한다.

```text
blank cell   = INHERIT
@none        = NONE
@empty       = explicit empty String/FText VALUE
@@<text>     = literal value starting with '@'
other value  = VALUE
```

Provider가 inheritance를 허용하지 않는 required field에서 blank가 최종 Provider default까지 resolve되지 않으면 MissingRequired로 Block한다. 즉 blank의 의미를 "미정/삭제/상속"으로 혼용하지 않고 exact INHERIT 하나로 고정한다.

Row identity provisioning:

```text
top-level authored row
RowId = canonical FCFContentKey token
```

따라서 top-level RowId는 별도 경쟁 identity가 아니다.

USER가 Excel에서 새 top-level row를 만들고 machine ContentId가 비어 있으면 `UnprovisionedRecord`로 읽고 Product mutation은 Block한다. Content Manager의 `ID 자동 할당` 또는 AI Change Set이 Provider의 collision-safe `GenerateContentId`를 호출해 hidden/protected ContentId를 한 번 발급한다. 발급 뒤 immutable이다.

ordered child처럼 자연 domain key가 없는 child row는 hidden/protected `ChildItemId`를 가진다. USER가 직접 새 child row를 만들고 ChildItemId가 비어 있으면 `UnprovisionedChild`로 Preview하고 explicit `Workbook Metadata Normalize`가 ID를 한 번 발급한다. USER가 ChildItemId를 직접 관리하지 않는다.

Reference/Family graph 중 acyclic contract가 선언된 edge는 cycle detection 후 mutation 전에 Block한다.

Inheritance resolution은 다음으로 교정·동결한다.

```text
Direct Content VALUE/NONE
> assigned Profile for that field domain
> Family default
> Provider default
```

`Variant`는 별도 중복 precedence layer가 아니라 Family에 속한 concrete Content record다. BaseContentId는 비교/디자인 기준으로 사용할 수 있지만 P0 implicit value inheritance owner로 사용하지 않는다.

`NONE`은 lookup을 종료하며 null을 의도한다. `INHERIT`만 다음 source로 진행한다.

##### C6 — Legacy Import / Default Fidelity

existing Product → initial Workbook baseline import 원칙:

- P0-07에서 CCASManaged로 전환할 field는 cutover 직전 persisted current semantic을 기본적으로 explicit `VALUE`로 pin한다.
- 단순 default와 값이 같다는 이유로 자동 `INHERIT`로 축약하지 않는다.
- `INHERIT`는 USER/AI가 의도적으로 상속을 선택한 새 authoring semantic이다.
- ExternalManaged field는 read-only catalog projection만 가능하며 Workbook authored authority가 아니다.
- LegacyPreserve/Deprecated field는 normal edit에서 숨기거나 read-only이며 roundtrip에 필요한 persisted 값은 보존한다.

Cutover acceptance:

```text
pre-import persisted semantic fingerprint
==
Workbook import → Canonical Model → disposable materialize semantic fingerprint
```

이 parity가 exact하지 않으면 해당 ContentType/field의 authority cutover를 하지 않는다.

Schema migration은 `OldWorkbookSemanticHash + OldSchemaVersion → NewCanonicalModel` deterministic Preview를 제공하며 migration 적용 전/후 diff를 USER가 볼 수 있어야 한다.

##### C7 — Unreal Asset Lifecycle / Resource Move / Cook

P0 normal authoring에서 physical Product asset rename/move/delete는 금지한다.

```text
DisplayName rename = allowed
ContentId rename = blocked identity migration
Generated asset package rename/move = blocked in normal P0
Retire = asset 보존 + 신규 사용 제한
Delete = P0 automatic exact0
```

따라서 existing hard reference와 Vehicle PrimaryAssetName identity를 불필요하게 흔들지 않는다.

Resource Catalog의 referenced Mesh/FX/Sound 등이 Content Browser에서 이동되어 redirector가 남은 경우:

- Asset Registry/redirector destination을 탐지할 수 있으면 `ResourcePathDrift`로 제안
- ResourceId는 유지
- stored resource path 갱신은 explicit reviewed Workbook Change Set
- missing/no redirector면 ResourceMissing으로 Block

향후 physical content rename 기능은 AssetTools + referencer scan + redirector fixup + persisted readback을 가진 별도 explicit migration operation으로만 추가한다.

Cook/packaging acceptance에는 representative generated DA의 hard/soft Resource dependency closure와 packaged smoke를 포함한다. Resource Catalog 자체는 Editor authoring layer이며 Runtime에서 Workbook을 읽지 않는다.

##### C8 — XLSX Adapter / Third-Party / Input Safety

P0 Workbook adapter는 Editor-only `ICFContentWorkbookAdapter` 경계 뒤에 격리한다. third-party C++ type은 CCAS Core/Provider public contract로 누출하지 않는다.

초기 구현 후보를 **OpenXLSX 0.5.1 pinned source**로 Freeze한다.

정책:

- Editor-only dependency. `CarFight_Re` Runtime module에 링크 금지.
- build-time network fetch/vcpkg install 금지.
- repository에 exact pinned source/binary + dependency license notice를 보존하는 vendored ThirdParty 방식. OpenXLSX 0.5.1의 upstream license는 BSD-3-Clause 기준으로 기록하며 PugiXML/miniz/Windows path dependency를 포함한 transitive notice를 함께 보존한다.
- adapter 교체 가능성을 유지해 parser 변경이 Canonical Model/Provider contract를 바꾸지 않게 한다.
- 첫 implementation commit 전에 UE 5.8/MSVC integration fixture에서 open/read/modify/save/reopen roundtrip을 검증한다.
- roundtrip이 workbook의 P0-required presentation/protection 요소를 파괴하면 해당 adapter를 Product authoring에 사용하지 않고 동일 interface 뒤 구현을 교체한다.

P0 authoritative semantic은 cell value만 소비한다. Charts, macros, external links, formula evaluation에 의존하지 않는다.

입력 제한:

```text
extension = .xlsx exact
max raw file size = 64 MiB
max registered worksheets = 64
max rows per registered sheet = 100000
max columns per registered sheet = 512
max authoritative non-empty cells total = 2000000
```

malformed/corrupt/unsupported workbook은 mutation0 Block.

authoritative cell에 formula/shared formula/external link가 있으면 cached value를 믿지 않고 Block한다. 사용자 분석용 non-authoritative sheet의 formula는 Compiler semantic에서 무시한다.

canonical path는 repository-relative로 resolve하며 개인 PC absolute path를 Workbook에 authority로 저장하지 않는다.

##### C9 — Batch Apply Journal / Dependency Order / Resume

CCAS mutation은 existing Data Authoring 원칙을 유지한다.

```text
full global preflight
→ dependency topological order
→ target 직전 immediate TOCTOU
→ sequential durable apply
→ persisted readback
→ journal terminal update
```

cross-target all-or-nothing transaction은 만들지 않는다. partial durable success는 보존한다.

durable local journal:

```text
Saved/CCAS/Operations/<ApplyOperationId>.json
schema = ccas-operation/v1

ApplyOperationId
BaseWorkbookSemanticHash
WorkbookFileHashAtStart
ChangeSetFingerprint
OrderedTargetKeys
per target:
  expected pre fingerprint
  dependency keys
  state
  persisted post fingerprint
  error taxonomy
overall state
```

overall/target state:

```text
Prepared
Applying
Applied
AlreadyCurrent
Blocked
Failed
NotStarted
PartialApplied
Completed
Abandoned
```

journal은 temp-write + replace 방식으로 한 target terminal result마다 갱신한다.

explicit Resume만 허용한다. Resume 조건:

- current WorkbookSemanticHash == journal base hash
- current ChangeSetFingerprint exact
- 이미 Applied target의 current persisted fingerprint == recorded post fingerprint
- dependency result가 유효

하나라도 다르면 resume Block. blind retry/rollback/delete 없음.

같은 OperationId/ChangeSetFingerprint를 재실행해 이미 current인 target은 중복 Create하지 않고 `AlreadyCurrent`로 분류한다.

##### C10 — FText / USER Workbook UX / Presentation Separation

Workbook machine identity와 USER 표시를 분리한다.

- stable SheetId/ColumnId/RowId는 hidden/protected machine metadata
- USER-facing header/help는 한국어
- formatting, width, freeze pane, color, worksheet tab label은 semantic hash에서 제외
- USER가 machine ID/ObjectPath를 직접 관리하는 것을 정상 UX로 요구하지 않는다

FText field descriptor는 다음 mode exact1을 가진다.

```text
PreserveIdentity
StableLocalized
LiteralInvariant
```

existing Product import:

- namespace/key가 존재하는 FText는 `PreserveIdentity`; source text edit 시 identity를 유지한다.
- identity 없는 기존 literal은 Provider가 explicit migration하지 않는 한 literal mode를 보존한다.

new player-facing localized content:

```text
Namespace = CarFight.Content.<ContentTypeId>
Key = <ContentId>.<FieldId>
SourceText = USER/AI authored Korean text
```

로 deterministic stable identity를 생성할 수 있다.

machine IDs, ResourceId, technical enum/token에는 FText를 사용하지 않는다.

Excel 도움 UX:

- reserved machine rows/columns hidden + protected
- Korean visible header
- units/help row
- freeze pane
- Resource/Content reference는 stable ID 목록 기반 lookup을 제공할 seam 유지
- Unreal Editor Content Manager에서는 실제 Asset Picker/Resource Preview를 제공
- Excel presentation feature 보존 여부는 adapter roundtrip fixture로 검증하되 compiler semantic은 presentation feature에 의존하지 않는다

##### C11 — Performance / Recovery / Workbook UX / Future Partition

P2 exact4도 P0 설계에 포함한다.

Performance:

- P0 synthetic benchmark fixture: content rows 10000 + child rows 100000 규모를 지원하도록 O(N) 또는 O(N log N) index/diff 구조를 기본으로 한다.
- parser/hash/diff/apply 각 phase timing과 peak row/cell count를 기록한다.
- full parse가 병목이면 semantic cache를 추가할 수 있으나 cache는 authority가 아니다.

Recovery:

- successful stable validation 또는 CCAS Workbook write 전에 current Workbook의 last-known-good copy를 `Saved/CCAS/Backups/<SemanticHash>/`에 보존할 수 있다.
- 최소 최근 10 unique semantic snapshot을 로컬 유지하는 것을 P0 기본값으로 한다.
- backup 복원도 explicit action이며 silent auto-restore 없음.

Workbook UX:

- canonical template가 hidden machine metadata, Korean header, unit/help, freeze pane, protected reserved region을 소유한다.
- USER가 worksheet/column physical order를 바꿔도 stable ID가 유지되면 semantic은 유지된다.
- USER analysis sheet는 freely 추가 가능하되 authority marker가 없으면 Compiler가 무시한다.

Future partition:

P0 authority는 Workbook exact1이지만 Core는 physical filename 대신 `WorkbookSourceId` abstraction을 사용한다. 후속 scale-out에서 domain workbook exactN으로 분할하더라도 `FCFContentKey / Provider / Canonical Model` contract를 다시 설계하지 않는다.

##### C12 — Existing System Convergence / Generic Provider Boundary

P0-00에서 DataAssetManagement UI 재사용/확장 판단은 **B안**으로 Freeze한다.

```text
Existing DataAsset Manager
= low-level persisted DA discovery / inspection / referencer / type summary
= Current read-first inspector로 유지

CCAS Content Manager
= Workbook / Catalog / Family / Variant / Compare / Resource / Diff / Apply
= high-level authoring/operations surface
```

두 UI는 Asset Registry discovery, DataAsset type metadata, referencer read 같은 read-only service를 재사용할 수 있지만 high-level CCAS lifecycle을 기존 Inspector에 억지로 흡수하지 않는다.

Vehicle:

- Vehicle Builder는 3D Mesh/Socket/Layout, Physics proposal, Driving/USER feel guided UX owner를 유지한다.
- P0-07 전 Product Vehicle field authority는 기존 Builder/Authoring owner가 유지한다.
- Cutover 뒤 `CCASManaged`로 승격된 field를 Vehicle Builder에서 바꾸려면 direct DA write가 아니라 CCAS Change Set/Workbook path로 route해야 한다.
- `ExternalManaged(VehicleBuilder)` field는 CCAS가 read/compare는 할 수 있어도 write하지 않는다.

Weapon:

- CF-FQ-055 UI는 standalone 기본 경로가 아니지만 typed validation/durable create/partial-recovery backend는 Weapon Provider가 재사용한다.
- Ammo/Damage 등 existing CFDA provider가 가진 writer를 CCAS가 중복 구현하지 않는다.

Equipment:

- EquipmentPreset은 Catalog/Reference Graph에 포함할 수 있으나 `CF-FQ-054` assembly/compatibility USER surface의 최종 존치 여부는 CCAS relation model 이후 rebaseline한다.

Generic Provider public responsibility:

```text
DescribeSchema()
ImportExisting()
BuildCurrentSnapshot()
ValidateRecord()
BuildDependencyEdges()
BuildDiff()
BuildReviewedMutationPlan()
ApplyReviewed()
ReadbackFingerprint()
```

Core는 raw reflection property path를 임의 setter로 사용해 Product UObject를 쓰지 않는다. Reflection은 schema/read projection에 사용할 수 있지만 **persistent mutation은 typed Provider가 소유**한다.

Vehicle Provider는 가능한 범위에서 existing FieldRegistry/Resolver/Batch 3-way preview 및 Authoring Service를 delegate한다. Weapon Provider는 existing typed durable provider / CFDADurableCore / WEA backend를 delegate한다.

기존 CFBatch의 stable ColumnId, immutable manifest, canonical numeric, 3-way conflict, global preflight + per-target TOCTOU 패턴은 CCAS에서 재사용한다. 단 기존 B3가 의도적으로 갖지 않는 cross-run dedupe는 C9의 CCAS operation journal이 상위에서 보완하며 기존 B3 semantics를 거짓으로 변경하지 않는다.

#### Contract Correction 판정

```text
P1-1  → C1 resolved
P1-2  → C2 resolved
P1-3  → C3 resolved
P1-4  → C4 resolved
P1-5  → C5 resolved
P1-6  → C6 resolved
P1-7  → C7 resolved
P1-8  → C8 resolved
P1-9  → C9 resolved
P1-10 → C10 resolved

P2-1~4 → C11 resolved
```


#### 2026-09-29 Fresh Design Re-review

v0.1.1 Review findings와 C1~C12 Contract Correction을 다시 Current Systems / representative Source와 교차검수했다.

재검수 항목:

```text
Workbook authority / Git / external editor concurrency
AI Change Set / deterministic xlsx writer
ContentKey / domain ID / PrimaryAsset identity
mixed package field ownership
collection/order/type/locale/null canonicalization
legacy import/default/inheritance fidelity
Unreal hard-ref/resource/rename/retire/cook
xlsx adapter/dependency/input safety
batch journal/idempotency/partial recovery
FText localization identity / USER workbook schema
Vehicle Builder / CFBatch / CFDA durable backend reuse
managed ↔ ExternalReadOnly reference graph
```

판정:

```text
P0 = 0
blocking P1 = 0
P2 = 2 non-blocking implementation evidence

P2-1:
OpenXLSX 0.5.1 adapter의 UE 5.8/MSVC open/read/modify/save/reopen
roundtrip + presentation preservation은 Source implementation 시작 시 실제 Build fixture로 증명 필요.

P2-2:
content 10,000 + child 100,000 synthetic Workbook의 parse/hash/diff 성능은
Generic Core 구현 뒤 benchmark evidence 필요.

Verdict = TECHNICAL DESIGN PASS
CCAS-P0-00 = CONTRACT FROZEN
Next = CCAS-P0-01 Workbook Schema + Canonical Model
```

P2 exact2는 설계 공백이 아니라 이미 acceptance method가 Freeze된 구현 검증 항목이며 CCAS-P0-01 착수를 막지 않는다.

이 PASS는 Excel authority가 이미 Current Product에 적용됐다는 뜻이 아니다. Product authority cutover는 계속 CCAS-P0-07 explicit gate가 소유한다.

### CCAS-P0-01 — Workbook Schema + Canonical Model

- SchemaVersion
- Stable ID
- Null / INHERIT / NONE / VALUE
- Units
- Derived fields
- Family / Variant / DesignIntent
- Reference / child-record model
- Override precedence
- Workbook revision / Change Set contract
- schema migration prototype

#### 2026-09-29 CCAS-P0-01 Implementation + Post-Implementation Review

구현 범위:

- `CarFight_ReEditor` Editor-only `DataAuthoring` 경계에 `FCFContentTypeId`, `FCFContentKey`, `FCFContentRecord`, field/sheet/provider descriptor, field ownership, value state, management state를 typed C++ contract로 구현했다.
- `WorkbookSourceId`, `SheetId`, `SchemaRevision`, `ColumnId`, `RowId = canonical FCFContentKey token`, `ChildItemId`의 machine identity를 physical Workbook 위치/label과 분리했다.
- Boolean / signed·unsigned integer / Float / Double / Enum / FName-style ID / String / FText semantic / ContentReference / ResourceReference와 `INHERIT/NONE/VALUE` canonicalization을 구현했다.
- Float/Double authored text는 ASCII `.` decimal/exponent strict grammar를 선검증해 locale comma, whitespace, trailing junk와 non-finite value를 fail-closed 한다.
- `OrderedList`, `KeyedCollection`, `UnorderedSet`의 semantic ordering/identity를 분리했다. KeyedCollection은 domain key, UnorderedSet은 canonical typed value가 semantic identity이며 hidden ChildItemId는 이 두 종류의 semantic hash에 들어가지 않는다.
- Schema-owned canonical unit, Derived ownership, Family/Base/DesignIntent, field-domain Profile assignment와 FText `PreserveIdentity / StableLocalized / LiteralInvariant` mode contract를 canonical model에 포함했다.
- frozen `cfcontent-change/v1` typed `FCFContentChangeSet` contract와 stable target identity / BaseWorkbookSemanticHash validation을 구현했다. `.xlsx` binary direct AI edit 경로는 열지 않았다.
- `WorkbookSemanticHash`는 canonical semantic model SHA-256으로 구현했고 formatting, USER label, worksheet/column/row physical order를 제외한다.
- `ICFContentProvider` exact responsibilities와 duplicate-safe Provider Registry seam, `ICFContentWorkbookAdapter` Editor-only boundary를 구현했다. Product Apply는 interface seam만 존재하며 P0-01 Product writer는 등록하지 않았다.
- `Managed / ExternalReadOnly`, dependency cycle/missing target fail-closed, deterministic old-schema → new-schema migration preview를 구현했다.
- concrete OpenXLSX dependency는 아직 repository/runtime dependency로 도입하지 않았다. frozen first-candidate validation은 P2-1 evidence로 유지한다.

검증:

```text
Official UE 5.8 Source Build
job = c98d528a7e12483888b770b40323dc9c
Result = PASS

Focused Automation
process = a61a62ecc6024e67820d60591eb882f2
SUCCESS = 8
FAILURE = 0
MISSING = 0
UNEXPECTED = 0
DUPLICATE_TERMINAL = 0
```

focused exact8:

```text
CoreSchema
ValueStates
Collections
SemanticHash
FailClosed
ExternalReadOnly
Migration
ProviderRegistry
```

구현 중 발견·교정:

- UE parser가 `12,5`를 numeric으로 수용할 수 있는 locale ambiguity를 확인했고, ASCII decimal/exponent strict grammar로 교정한 뒤 exact8에 regression을 고정했다.
- Automation fixture의 `TArray` self-aliasing은 Product/Core 결함과 분리해 value-copy fixture로 교정했다.

금지 경계 재검수:

- Product DataAsset mutation = 0
- Product authority cutover = 0
- generic reflection Product setter = 0
- Product Save / rename / move / delete = 0
- concrete OpenXLSX Runtime dependency = 0
- AI direct `.xlsx` binary edit = 0
- existing CFBatch / Vehicle Authoring / CFDA durable writer 중복 구현 = 0

Post-Implementation Review:

```text
P0 = 0
blocking P1 = 0
P2 = 2 non-blocking implementation evidence

P2-1 = OpenXLSX 0.5.1 UE 5.8/MSVC open/read/modify/save/reopen + presentation/protection preservation fixture
P2-2 = content 10,000 + child 100,000 synthetic Workbook parse/hash/diff benchmark

Verdict = TECHNICAL IMPLEMENTATION PASS
CCAS-P0-01 = COMPLETE
Next = CCAS-P0-02 Generic Compiler Core
```

P2 exact2는 계속 non-blocking이며 P0-02에서 필요한 시점에 evidence를 생성한다. P0-07 explicit Cutover 전 Current Product DA authority는 변경하지 않는다.

### CCAS-P0-02 — Generic Compiler Core

- Workbook reader
- Canonical Content Model
- Provider Registry
- Validation
- Diff
- Reference Graph
- impact analysis
- Generated fingerprint / drift
- batch result model
- disposable fixtures only

#### 2026-09-29 CCAS-P0-02 Implementation + Post-Implementation Review

구현 범위:

- `FCFContentCompiler::CompilePreview`를 read-only Generic Compiler entry로 추가했다.
- Workbook read는 P0-01에서 동결한 `ICFContentWorkbookAdapter::ReadWorkbook`만 사용한다. Core에 OpenXLSX를 직접 결합하지 않는다.
- P0-01 `ValidateWorkbook`, `BuildWorkbookSemanticHash`, `ProviderRegistry`를 재사용하고 동일 계약을 재구현하지 않았다.
- primary ContentType별 `ImportExisting`을 exact1 호출해 canonical ContentKey 기반 current Product `TMap` index를 구축한다.
- Provider `ValidateRecord`, `BuildDependencyEdges`, `BuildCurrentSnapshot`, `BuildDiff`, `ReadbackFingerprint`를 통해 provider-local typed semantics를 유지한다.
- deterministic record diff는 `Added / Removed / Modified / Unchanged` exact4 상태를 사용한다.
- dependency graph는 P0-01 common missing-target/cycle validator를 통과한 뒤 reverse adjacency + BFS로 direct/transitive impact를 계산한다.
- generated fingerprint/drift는 `WorkbookSemanticHash + desired canonical record projection fingerprint + provider current/desired fingerprint`를 연결한다. current Product가 없는 Added record는 존재하지 않는 post-generation fingerprint를 추측하지 않고 `MissingCurrent`로 명시한다.
- drift 상태는 `InSync / MissingCurrent / Changed / RetiredCurrent / ExternalReadOnly`로 분리한다.
- batch result는 `ReadyToReview / RetireCandidate / AlreadyCurrent / ExternalReadOnly / Blocked` preview 상태만 만든다. Product mutation은 수행하지 않는다.
- disposable in-memory Workbook Adapter + Provider fixture로 Product asset 없이 전체 pipeline을 검증한다.

성능 구조:

- current import/index = O(N) expected
- desired/current lookup = `TMap` O(1) expected
- diff = record 수에 선형이며 provider-local semantic diff 비용을 더한다.
- reference impact = reverse adjacency 구축 O(V+E), changed root별 BFS O(V+E) upper bound
- stable output은 ContentKey sort를 사용해 deterministic order를 보존한다.

검증:

```text
Official UE 5.8 Source Build
job = 8b8afac6443f433692990004cb2a052e
Result = PASS

Focused Automation
process = 06c9db1ee69f469985e7956c692e8145
SUCCESS = 8
FAILURE = 0
MISSING = 0
UNEXPECTED = 0
DUPLICATE_TERMINAL = 0
```

focused exact8:

```text
WorkbookReader
ProviderValidation
DiffDrift
ReferenceGraph
ImpactAnalysis
BatchNoApply
FailClosed
ScaleBenchmark
```

10k / 100k synthetic scale evidence:

```text
content = 10,000
child = 100,000
canonical textual numeric cells parsed = 110,000
parse_seconds = 0.102
hash_diff_seconds = 0.785
total_seconds = 0.887
Result = PASS
```

이 benchmark의 `parse`는 P0-01 `FCFContentCanonicalizer::ParseCell`의 canonical cell parsing을 실제 수행한다. `.xlsx` container open/save I/O는 concrete OpenXLSX adapter의 책임이므로 아래 P2-1과 분리한다.

Build interference evidence:

- 최초 Source build는 새 Compiler/Test compile까지 PASS한 뒤 기존 AI-owned Unreal Editor의 `UnrealEditor-CarFight_ReEditor.dll` file lock 때문에 link `LNK1104`에서만 중단됐다.
- ownership-validated `carfight.editor.stop_ai_owned` 경로로 해당 AI-owned runtime만 종료한 뒤 source 변경 없이 재빌드해 PASS했다.
- final v1.1.0 scale fixture 보강 뒤 official Build `8b8afac6443f433692990004cb2a052e`도 PASS했다.

금지 경계 재검수:

- `FCFContentCompiler` Product `ApplyReviewed` call = 0
- `BuildReviewedMutationPlan` call = 0
- Workbook staged write / reopen = 0
- Product DataAsset Save / rename / move / delete = 0
- generic UObject reflection setter = 0
- concrete OpenXLSX Core dependency = 0
- AI direct `.xlsx` binary edit = 0
- Product authority cutover = 0

Post-Implementation Review:

```text
P0 = 0
blocking P1 = 0
P2 = 1 non-blocking implementation evidence

P2-1 = OpenXLSX 0.5.1 UE 5.8/MSVC open/read/modify/save/reopen + presentation/protection preservation fixture
P2-2 = CLOSED — 10k content + 100k child canonical parse/hash/diff benchmark PASS

Verdict = TECHNICAL IMPLEMENTATION PASS
CCAS-P0-02 = COMPLETE
Next = CCAS-P0-03 Resource Catalog + Semantic Binding
```

P2-1은 concrete `.xlsx` adapter 증거이며 Generic Compiler Core의 library-independent 설계를 막지 않는다. P0-07 explicit Cutover 전 Current Product DA authority는 계속 변경하지 않는다.

#### 2026-09-29 CCAS-P0-02 Mid-review Correction + Fresh Re-review

초기 P0-02 구현 PASS 뒤 fresh mid-review에서 다음 안전 공백을 확인했다.

```text
P0 = 0
blocking P1 = 3
P2 = 2
Verdict = HOLD
```

blocking P1 exact3:

1. retire 후보 impact가 desired graph만 사용해 current Product의 기존 referencer를 놓칠 수 있음
2. Provider BuildDiff가 비교 가능한 semantic fingerprint를 주지 않아도 Unchanged로 축소될 수 있음
3. Workbook logical schema와 registered Provider schema 세대/field contract가 달라도 암묵적으로 compile될 수 있음

당시 P2 exact2:

- ImportExisting와 per-record BuildCurrentSnapshot 사이 mixed-time current state 가능성
- OpenXLSX 0.5.1 actual roundtrip / presentation / protection evidence

교정:

- `CurrentDependencyEdges`를 별도로 구축하고 desired + current reverse adjacency union으로 impact를 계산한다.
- Current graph는 provider `BuildDependencyEdges`를 재사용하며 missing endpoint / cycle은 fail-closed한다.
- retire 후보가 desired Workbook에서 사라져도 current-only referencer를 impact에서 보존한다.
- Provider current/desired fingerprint는 canonical SHA-256을 요구한다. empty/malformed 증거는 `InvalidProviderFingerprint`로 fail-closed한다.
- `BuildDiff.CurrentFingerprint`와 `BuildCurrentSnapshot.ReadbackFingerprint`가 둘 다 canonical인데 서로 다르면 `ProviderCurrentFingerprintMismatch`로 fail-closed한다.
- removed current record의 `ReadbackFingerprint`도 canonical SHA-256을 강제한다.
- Workbook logical schema와 Provider descriptor를 `ContentTypeId / SchemaRevision / SheetId / ParentSheetId / CollectionId / CollectionKind / ColumnId / ValueType / Ownership / ExternalOwnerId / CanonicalUnitId / FieldDomainId / TextMode / AllowNone / Required` 기준으로 exact compatibility 검증한다. `DisplayLabel`은 presentation-only이므로 제외한다.
- schema mismatch는 암묵적으로 해석하지 않고 `ProviderSchemaCompatibility` 단계에서 fail-closed해 migration preview 대상으로 돌린다.
- desired-existing record는 ImportExisting canonical projection과 BuildCurrentSnapshot canonical projection을 비교해 mixed-time record snapshot을 차단한다.
- whole-current Product는 Preview 시작/종료에 ImportExisting 전체 semantic hash를 다시 계산한다. retire 후보를 포함한 current set/value가 중간에 변하면 `CurrentSnapshotConsistency / CurrentImportChanged`로 Preview 전체를 폐기한다.

최종 검증:

```text
Official UE 5.8 Source Build
job = d592636c607b4df48af776a83ece64c9
Result = PASS

Focused Automation
process = 0004a2f9594f4ab1a16eb17998130790
SUCCESS = 12
FAILURE = 0
MISSING = 0
UNEXPECTED = 0
DUPLICATE_TERMINAL = 0
```

focused exact12:

```text
WorkbookReader
ProviderValidation
DiffDrift
ReferenceGraph
ImpactAnalysis
BatchNoApply
FailClosed
ScaleBenchmark
CurrentRetireImpact
FingerprintGuard
SchemaCompatibility
SnapshotConsistency
```

final scale evidence:

```text
content = 10,000
child = 100,000
canonical textual numeric cells parsed = 110,000
parse_seconds = 0.101
compiler_hash_diff_consistency_seconds = 1.795
total_seconds = 1.896
Result = PASS
```

초기 0.887s 대비 전체 current pre/post import semantic consistency와 canonical Provider fingerprint 검증이 추가되어 비용은 증가했으나 representative 10k/100k fixture는 약 2초 이내로 완료된다.

Fresh Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 1 non-blocking implementation evidence

P1-1 Current retire referencer gap = RESOLVED
P1-2 Provider fingerprint fail-open = RESOLVED
P1-3 Workbook/Provider schema drift = RESOLVED
P2 current snapshot consistency = CLOSED
P2 scale benchmark = CLOSED
P2 remaining exact1 = OpenXLSX 0.5.1 actual open/read/modify/save/reopen + presentation/protection preservation

Verdict = TECHNICAL IMPLEMENTATION PASS
CCAS-P0-02 = COMPLETE
Next = CCAS-P0-03 Resource Catalog + Semantic Binding
```

Product boundary 재검수:

- Product `ApplyReviewed` / `BuildReviewedMutationPlan` 호출 = 0
- Workbook staged write / reopen = 0
- Product Save / generic reflection setter = 0
- Product authority cutover = 0
- P0-07 선행 = 0

따라서 CCAS-P0-03 착수 blocker는 없다. 잔여 OpenXLSX P2 exact1은 concrete adapter evidence로 유지한다.

### CCAS-P0-03 — Resource Catalog + Semantic Binding

- Stable Resource ID
- Unreal Asset Picker registration
- Asset Registry type / existence validation
- Mesh capability scanner
- Socket / Bone / Material Slot capability
- Semantic Role Binding
- Profile + Override
- required / optional resource fail policy

#### 2026-09-29 Technical Implementation + Post-Implementation Review

구현:

- `CFContentResource.h/.cpp v1.1.0`에 Editor-only read-only Resource Catalog / Semantic Binding Core를 추가했다.
- 기존 P0-01 `ResourceReference`와 stable ID grammar, P0-02 Generic Compiler Core는 재구현하거나 변경하지 않았다.
- `ResourceId`는 raw Unreal `ObjectPath`와 분리한 machine identity로 유지하고 실제 `FSoftObjectPath`와 expected picker type은 Resource Catalog만 소유한다.
- `FCFResourcePickerRegistry`는 stable `PickerTypeId -> AllowedBaseClassPath`를 등록하고 등록 시 class resolve를 fail-closed한다. 기존 Vehicle/Equipment/Weapon UI의 `SObjectPropertyEntryBox.AllowedClass(...)` 패턴에 연결 가능한 contract이며 이 Gate에서는 consumer UI를 재작성하지 않는다.
- `FCFResourceCatalog`는 Asset Registry metadata로 exact asset existence와 expected base/derived class compatibility를 검증한다.
- `FCFMeshCapabilityScanner`는 StaticMesh의 Socket / Material Slot, SkeletalMesh의 active Socket / Reference Skeleton Bone / Material Slot을 read-only load + deterministic unique snapshot으로 스캔한다.
- `FCFSemanticRoleRegistry`는 stable RoleId와 Required/Optional contract, Socket/Bone/MaterialSlot capability kind를 소유한다.
- `FCFResourceProfileRegistry`는 반복 binding profile을 typed role/resource/capability contract로 검증한다.
- `FCFResourceBindingResolver`는 `Override > Profile` precedence를 고정하고 unbound source는 `None`, profile source는 `Profile`, explicit override source는 `Override`로 구분한다.
- Required role의 missing resource/capability는 blocking fail-closed, Optional role의 unbound/clear/unavailable capability는 typed non-blocking diagnostic으로 유지한다.
- `FCFContentResourceValidator`는 기존 Canonical Workbook의 top-level/child `ResourceReference` VALUE를 Stable ResourceId -> Catalog -> Asset Registry 경로로 검증한다.
- Product Apply / DataAsset Save / Workbook persistent write / generic reflection setter / P0-07 authority cutover 경로는 추가하지 않았다.

Fresh review correction:

1. Picker descriptor가 존재해도 `AllowedBaseClassPath`를 실제 `UClass`로 resolve하지 못하면 registration 시점에 fail-closed하도록 교정했다.
2. Profile/Override가 전혀 없는 Optional role이 `Profile` source로 보이지 않도록 `ECFResourceBindingSource::None`을 추가해 resolved source semantics를 명확히 했다.

최종 검증:

```text
Official UE 5.8 Source Build
job = 4c868de58f904bed9b5cf96b8f5af079
Result = PASS

Focused Automation
process = f0865deaf4e0416da687f3171eeaadb7
SUCCESS = 8
FAILURE = 0
MISSING = 0
UNEXPECTED = 0
DUPLICATE_TERMINAL = 0
```

focused exact8:

```text
StableResourceId
PickerRegistry
AssetValidation
MeshCapabilities
SemanticRoleBinding
ProfileOverride
RequiredOptionalPolicy
WorkbookResourceValidation
```

실제 Engine StaticMesh fixture `/Engine/BasicShapes/Cube.Cube`로 Asset Registry existence/type 및 StaticMesh capability scan을 검증했다. Socket/Bone/MaterialSlot exact lookup semantics는 synthetic capability snapshot으로 고정했다.

Post-Implementation Review:

```text
P0 = 0
blocking P1 = 0
P2 = 2 non-blocking implementation evidence

P2-1 = OpenXLSX 0.5.1 UE 5.8/MSVC actual open/read/modify/save/reopen + presentation/protection preservation
P2-2 = persisted USkeletalMesh real fixture에서 active Socket + Reference Skeleton Bone + Material Slot end-to-end scan evidence

Verdict = TECHNICAL IMPLEMENTATION PASS
CCAS-P0-03 = COMPLETE
Next = CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot
```

P2-2는 SkeletalMesh branch가 UE 5.8에서 compile/link PASS했고 typed capability semantics가 Automation으로 고정돼 있어 P0-03의 generic contract를 막지 않는다. 실제 persisted Weapon/Vehicle mesh를 사용하는 P0-04 Dual Consumer Pilot에서 해당 evidence를 닫는다.

Product boundary 재검수:

- `ApplyReviewed` / `BuildReviewedMutationPlan` = 0
- Workbook staged write / reopen = 0
- `SavePackage` / `MarkPackageDirty` / UObject `Modify` = 0
- generic reflection setter / `FindPropertyByName` = 0
- concrete OpenXLSX dependency = 0
- Product authority cutover / P0-07 선행 = 0

따라서 CCAS-P0-04 착수 blocker는 없다. P0-04에서는 Weapon + Vehicle exact2 consumer가 동일 Resource/Semantic Core를 실제 provider/backend 경계에서 공유하는지 검증한다.

### CCAS-P0-04 — Weapon + Vehicle Dual Consumer Pilot

두 타입을 동시에 사용해 Core 범용성을 검증한다.

Weapon:
- CF-FQ-055 durable backend / existing providers 재사용
- Family / Variant 대량 definition
- Resource / Socket binding

Vehicle:
- 기존 Vehicle Authoring / Builder backend와 권한 경계 재사용
- Vehicle catalog / compare
- representative generated / managed fields

합격 기준:

- Weapon 전용 분기 없이 Core exact shared semantics
- Vehicle-only 요구 때문에 Core가 generic string bag으로 붕괴하지 않음
- provider-local typed semantics 유지

#### 2026-09-29 Technical Implementation + Post-Implementation Review

구현:

- `CFContentPilot.h/.cpp v1.1.0`에 P0-04 전용 Editor-only read-only consumer bridge를 추가했다.
- P0-01 Canonical Content Model, P0-02 Generic Compiler Core, P0-03 `CFContentResource` Generic Resource Core는 재구현하거나 consumer 분기로 수정하지 않았다.
- `FCFContentPilotContext` exact1 안에서 Weapon/Vehicle이 동일한 `PickerRegistry / ResourceCatalog / RoleRegistry / ProfileRegistry / FCFResourceBindingResolver` instances를 순차 공유한다.
- Weapon bridge는 CF-FQ-055의 기존 `UCFWeaponData::ValidateWeaponDataContract`를 실제 호출하고 `UCFTurretMountData`의 typed mesh/socket intent만 Resource/Profile/Override projection으로 변환한다.
- Vehicle bridge는 기존 `FCFVehicleSnapshotBuilder::BuildRecipeSnapshot`과 `FCFVehicleAssetReader::BuildAssetSnapshot`을 실제 호출하고 Recipe/AssetIntent/HardpointIntent의 typed semantics를 그대로 Resource/Profile/Override projection으로 변환한다.
- Resource/Core 쪽에는 `UCFWeaponData`, `UCFVehicleRecipeData`, `CFTurretMountData`, `weapon.*`, `vehicle.*` consumer branch를 추가하지 않았다.
- Provider-local bridge의 semantic role namespace는 Weapon/Vehicle를 구분하지만 capability name과 resource intent는 기존 typed backend가 소유하고 Generic Core는 string bag/domain switch를 알지 않는다.
- Product Apply / Product writer / Workbook writer / generic reflection / P0-07 Cutover 경로는 사용하지 않았다.

대표 persisted Weapon evidence:

```text
WeaponData
/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon.DA_ProtoTurretCannon
WeaponId = Proto_TurretCannon

TurretMountData
/Game/CarFight/Weapons/Data/TurretMounts/DA_CannonBody.DA_CannonBody
TurretMountId = Proto_RoofTurretMount

Base mesh  = Base_Standard
Base socket = YawPivot

Yaw mesh    = RoofHeavyCannon_YawMesh
Yaw socket  = PitchPivot

Pitch mesh  = RoofHeavyCannon_PivotMesh
Pitch socket = Muzzle
```

- Base/Yaw/Pitch mesh resource는 Profile source로 resolve된다.
- `YawPivot / PitchPivot / Muzzle` capability는 TurretMountData의 실제 typed socket intent를 per-content Override로 전달하고 P0-03 Mesh Capability Scanner가 persisted mesh에서 검증한다.

대표 persisted Vehicle evidence:

```text
VehicleRecipe
/Game/CarFight/Data/Authoring/DA_Recipe_TestSUV.DA_Recipe_TestSUV

Chassis
/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV

Wheel sockets
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR

Hardpoint sockets
Front_01 -> HP_Front_01
Top_01   -> HP_Top_01
Top_02   -> None
```

- 기존 Vehicle SnapshotBuilder fingerprint와 AssetReader ChassisLayoutFingerprint를 실제 생성했다.
- Chassis/wheel resource는 Profile source, wheel socket은 typed Override source로 resolve했다.
- `Front_01 / Top_01` hardpoint는 provider-local dynamic Semantic Role + Profile binding으로 resolve했다.
- socket이 없는 `Top_02`는 임의 binding을 만들지 않았다.

Persisted SkeletalMesh real scan evidence:

```text
/Game/Vehicles/SportsCar/SKM_SportsCar
Socket=0 / Bone=27 / MaterialSlot=1

/Game/Vehicles/OffroadCar/SKM_Offroad
Socket=0 / Bone=46 / MaterialSlot=3

/Game/Models/SportsCar/SKM_SportsCar
Socket=0 / Bone=27 / MaterialSlot=5
```

- P0-03 `FCFMeshCapabilityScanner`가 persisted USkeletalMesh exact3을 실제 load/scan하는 경로는 PASS했다.
- Reference Skeleton Bone과 Material Slot positive evidence는 CLOSED했다.
- 현재 프로젝트에서 발견된 representative USkeletalMesh exact3에는 active Socket이 실제로 0개이므로 positive active-Socket fixture evidence는 만들지 않았다.
- 첫 real evidence run `d6fd4b697f3b446c88a80ed62bec7df0`에서 이 콘텐츠 사실을 확인했고, `Socket > 0`을 강제하던 테스트 가정을 실제 콘텐츠 상태에 맞춰 non-blocking evidence로 교정했다.
- 따라서 P0-03의 Skeletal evidence P2는 `persisted USkeletalMesh positive active Socket fixture evidence` exact1로 축소해 유지한다.

Fresh review correction:

1. 초기 Weapon/Vehicle Profile capability에 representative fixture의 `YawPivot / PitchPivot / Muzzle / Wheel_Anchor_*` 문자열이 기본값으로 들어간 부분을 blocking P1 exact1로 판정했다.
2. `CFContentPilot v1.1.0`에서 Profile capability도 `TurretMountData`와 Vehicle `AssetIntent`의 실제 typed socket value를 사용하도록 교정했다.
3. Generic Core consumer branch search는 0, Product mutation/writer search는 0으로 재검수했다.

최종 검증:

```text
Official UE 5.8 Source Build
job = 221caceb6bca4e42bfc539d90c033196
Result = PASS

CCAS-P0-04 Focused Automation
process = 124a3563e3de4e0083114b54e34cb1ef
SUCCESS = 5
FAILURE = 0
MISSING = 0
UNEXPECTED = 0
DUPLICATE_TERMINAL = 0

Affected P0-03 Resource Core Regression
process = 381912578fdf4e57a572266d289e8131
SUCCESS = 8
FAILURE = 0
MISSING = 0
UNEXPECTED = 0
DUPLICATE_TERMINAL = 0
```

P0-04 focused exact5:

```text
SharedCoreDualConsumer
WeaponPersistedBinding
VehiclePersistedBinding
ProfileOverrideSources
SkeletalMeshPersistedScan
```

Post-Implementation Review:

```text
P0 = 0
blocking P1 = 0
P2 = 2 non-blocking implementation evidence

P1 typed capability fixture hardcode = RESOLVED

P2-1 = OpenXLSX 0.5.1 UE 5.8/MSVC actual open/read/modify/save/reopen + presentation/protection preservation
P2-2 = persisted USkeletalMesh positive active Socket fixture evidence
        (real scanner exact3 PASS / Bone + MaterialSlot CLOSED / current representative assets Socket exact0)

Verdict = TECHNICAL IMPLEMENTATION PASS
CCAS-P0-04 = COMPLETE
Next = CCAS-P0-05 Catalog / Compare + USER Workflow
```

Product boundary 재검수:

- `ApplyReviewed` / `BuildReviewedMutationPlan` = 0
- Product Save / `MarkPackageDirty` = 0
- Workbook persistent write = 0
- concrete OpenXLSX dependency = 0
- generic reflection setter / `FindPropertyByName` = 0
- Generic Core Weapon/Vehicle consumer branch = 0
- Product writer activation = 0
- Product authority cutover / P0-07 선행 = 0

따라서 CCAS-P0-05 착수 blocker는 없다. P0-05에서는 이 exact shared typed resource resolution을 사용해 Catalog / Compare / USER Workflow의 정보 계층과 편집 진입 UX를 구현·검증한다.

### CCAS-P0-05 — Catalog / Compare + USER Workflow

- 전체 목록 / 필터 / 검색
- Family / Variant tree
- selected compare
- absolute / relative values
- resolved value source
- dependency / resource view
- workbook revision / validation status
- Diff Preview
- Resource / Socket 편집 진입
- USER 정보 계층 검수

#### 2026-09-29 CCAS-P0-05 Implementation + Fresh Re-review

구현:

- `CFContentCatalog.h/.cpp v1.0.1`
  - persisted Product read-only Weapon + Vehicle 전체 Catalog
  - search / All·Vehicle·Weapon filter
  - Family / Variant group projection
  - selected compare
  - absolute / relative numeric values
  - resolved/authored value source
  - dependency view
  - P0-04 shared Resource/Semantic binding view
  - Canonical Workbook Compile Preview가 공급될 때 revision / validation / record Diff projection
  - Product Apply / writer / Workbook write authority 0
- `CFContentCatalogTab.h/.cpp v1.0.1`
  - `CarFight.ContentManager` Native Slate Nomad Tab
  - 상단 authority/workbook/validation/apply status
  - 좌측 Family / Variant tree
  - 중앙 searchable Catalog list
  - 우측 selected value/source → compare → dependency → Resource/Socket → Diff Preview 순서
  - 기존 `CarFight.VehicleAuthoring` / `CarFight.WeaponGuide` 전문 제작 화면 진입
  - Resource asset explicit open을 통한 Socket/Resource 전문 편집 진입
  - search/type filter가 selected row를 숨길 때 stale detail/compare를 제거
- `CarFightReEditor.h/.cpp v1.9.0`
  - Window 메뉴에 `CarFight 콘텐츠 관리` 진입점을 추가
- `CFContentCatalogTests.cpp v1.0.1` + `Tools/RunCCASP005Tests.ps1 v1.0.0`
  - focused Automation exact5

Workbook / Diff 경계:

- current repository에는 canonical `Authoring/Content/CarFight_Content.xlsx`와 concrete `.xlsx` adapter가 아직 존재하지 않는다.
- 이는 기존 P2 `OpenXLSX actual roundtrip / presentation / protection` evidence와 동일 경계이며 P0-05에서 새 adapter 구현으로 확대하지 않는다.
- 따라서 live Product mode는 Workbook/Diff를 가짜 데이터로 채우지 않고 `xlsx adapter P2 미연결`로 fail-visible 표시한다.
- 동시에 `ApplyCompilePreview(FCFContentCompileResult)` seam과 Automation fixture로 Workbook revision / validation / Family/Variant / dependency / Diff UI projection 자체는 검증한다.
- concrete xlsx adapter가 들어오면 Generic Core 변경 없이 동일 Presentation 경로에 Compile Preview를 공급할 수 있다.

Fresh re-review correction:

- 초기 구현의 `Workbook Preview 미로드` 표현은 실제 미로드인지 기능 미제공인지 구분이 불명확한 P1 UX ambiguity로 판정해 `xlsx adapter P2 미연결` unavailable 상태로 교정했다.
- type filter 전환 후 filtered-out selected row의 detail이 남는 stale selection P1 exact1을 발견해 common visible-selection guard로 교정했다.
- Generic Core `CFContentTypes / CFContentCore / CFContentCompiler / CFContentResource / CFContentPilot` diff = 0.
- Generic Core Weapon/Vehicle consumer branch 추가 = 0.
- Product Apply / Product Save / `MarkPackageDirty` / Workbook persistent write / Product writer activation / P0-07 cutover = 0.
- USER-facing 기본 화면에서 raw ObjectPath / Stable Content ID 관리 요구 = 0.

Validation:

- Final Official UE 5.8 Source Build `dfe409d0cb034fa2be5b235e6c67d48d` = PASS.
- Final P0-05 focused Automation `648fb1e8f53144849138cdd0de30b8d0` = exact5/5 PASS.
  - `LiveDualConsumerCatalog`
  - `FilterFamilyVariant`
  - `SelectedCompareSources`
  - `CanonicalWorkbookPreview`
  - `ReadOnlyAuthorityGuard`
- exact-list invariant = failure 0 / missing 0 / unexpected 0 / duplicate 0.
- P2-1 OpenXLSX actual roundtrip/presentation/protection = non-blocking 유지.
- P2-2 persisted USkeletalMesh positive active Socket fixture = non-blocking 유지.

Post-Implementation Technical Review:

```text
P0 = 0
blocking P1 = 0
P2 = 2 non-blocking implementation evidence

Verdict = TECHNICAL IMPLEMENTATION PASS
CCAS-P0-05 = USER Workflow Review Ready
Next = USER information hierarchy / sight-flow review in CarFight Content Manager
```

#### 2026-09-30 CCAS-P0-05 USER Workflow Review Closure

기존 USER Workflow Review는 이미 실제 `CarFight.ContentManager` 사용 피드백을 통해 수행되었다. 이번 상태 정리는 그 완료 사실이 Plan/UDS에 반영되지 않아 동일 USER Review를 다시 요구하던 stale checkpoint를 교정한다.

USER 피드백에서 확인한 핵심은 다음과 같다.

- Family / Variant tree와 전체 Catalog list 사이의 정보 중복이 존재한다.
- 당시 Vehicle 예시인 Wagon / Sedan 비교에서는 실제 차이를 판단할 정보가 충분히 풍부하지 않았다.
- 그럼에도 현재 형태를 첫 운영 baseline으로 채택하고 다음 단계로 진행하기로 결정했다.
- CCAS의 주축 목적은 차량 비교 UI 자체의 완성도가 아니라 다수 콘텐츠, 특히 Weapon Family / Variant를 계획·비교·대량 관리하는 workflow다.
- 따라서 추가 UI polish를 P0-05 blocker로 만들지 않고, 실제 Weapon Content planning과 batch change workflow에서 필요한 개선점을 후속 evidence로 수집한다.

USER Verdict:

```text
CCAS-P0-05 Technical Implementation = PASS (보존)
CCAS-P0-05 USER Workflow Review = PASS
CCAS-P0-05 = COMPLETE
Additional Content Manager UI polish = non-blocking follow-up
Next = CCAS-P0-06 AI Change Set + Batch Planning Design Review
```

이 USER PASS는 P0-06의 **설계·검수 진입**을 허용한다. 그러나 Workbook/DataAsset authoring, CCAS mutation implementation, Product Apply/Save 또는 P0-07 authority cutover를 승인하지 않는다.

### CCAS-P0-06 — AI Change Set + Batch Planning

Accepted planning input:
- `WeaponContentRosterPlan.md v0.1.1` — USER Accepted Planning Baseline / 2026-09-30
- Core Family exact3: Cannon / Unguided Rocket / Guided Missile
- First Production Wave exact8: Cannon4 + Rocket3 + GuidedMissile1
- Guided Missile exact1은 Wave 1B conditional이며 PFP-P0-03 USER trajectory gate를 Product Acceptance 선행조건으로 유지
- Wave 1 Product 콘텐츠는 finite Ammo를 기본 기준으로 사용
- CCAS-P0-05 USER Workflow Review는 2026-09-30 PASS/COMPLETE로 닫혔다.
- 따라서 P0-06의 설계·검수는 시작할 수 있다.
- 단, 이 planning input과 P0-05 USER PASS는 Workbook/DataAsset authoring, CCAS mutation implementation, Product Apply/Save 승인이 아니다. 별도 USER 승인 전에는 implementation boundary를 열지 않는다.

P0-06 scope:
- catalog snapshot contract
- AI change proposal
- stale revision guard
- bulk add / update / retire
- rationale / design intent
- preview / review
- no-direct-DA-write guard


#### 2026-09-30 CCAS-P0-06 Pre-Implementation Design Review

Fresh 기준선:
- `ContentAuthoringSystemPlan.md v0.1.10`
- USER Accepted `WeaponContentRosterPlan.md v0.1.1`
- current `CFContentTypes.h v1.1.0`
- current `CFContentCompiler` / `FCFContentCompileResult`
- current `FCFContentProvider` reviewed-mutation seam
- P0-01~05 Accepted/Technical PASS evidence는 재구현·재검증하지 않음

검수 결과:

```text
P0 = 0
blocking P1 = 6
P2 = 3
Verdict = HOLD
Next = CCAS-P0-06 Contract Correction + Design Re-review
Implementation / Workbook Authoring / DataAsset Authoring / CCAS Mutation / Product Apply = NOT OPENED
```

현재 방향을 폐기하거나 Generic Core를 다시 설계해야 할 P0 문제는 없다. P0-01~02에서 이미 typed Change Set, canonical WorkbookSemanticHash, deterministic Compiler Preview, retire candidate, current/desired dependency graph, impact analysis와 no-Apply Compiler guard를 제공하므로 이 기반은 보존한다.

다만 P0-06의 실제 목적은 단순히 operation enum을 추가하는 것이 아니라 **AI가 어떤 immutable catalog 기준을 보고, exact8 Weapon Roster의 기획 의미를 손실 없이 typed proposal로 만들고, USER가 어떤 preview artifact를 승인하며, 승인 뒤에도 stale state를 안전하게 차단하는지**를 동결하는 것이다. 아래 blocking P1은 구현 전에 교정한다.

##### P1-1 — Immutable Catalog Snapshot 계약 부재

현재 `FCFContentCompileResult`는 Workbook model, WorkbookSemanticHash, current/desired dependency graph, diff, impact와 generated fingerprint를 제공하지만 **AI planning input 자체를 immutable snapshot으로 결속하는 별도 contract가 없다.**

AI가 live Catalog UI state나 여러 read 결과를 조합하면 planning 시작과 review 시점 사이에 Product truth, Resource availability 또는 Provider schema가 바뀌어도 동일 planning input처럼 보일 수 있다.

교정 조건:

```text
FCFContentCatalogSnapshot
- SnapshotId
- WorkbookSourceId
- WorkbookSemanticHash
- WorkbookSchemaVersion
- ProviderSchemaFingerprint
- ProductStateFingerprint
- ResourceCatalogFingerprint
- CanonicalRecords / AuthoringMetadata
- Dependency / Referencer summary
- Validation summary
- GeneratedDrift summary
```

- snapshot은 deterministic canonical sort/hash를 가져야 한다.
- AI proposal은 exact `BaseCatalogSnapshotId/Fingerprint`에 결속한다.
- Presentation filter/search 결과가 snapshot authority가 되어서는 안 된다.
- ExternalReadOnly node와 missing/unready resource도 snapshot에서 사라지지 않는다.

##### P1-2 — Weapon Roster Planning Metadata Fidelity 부족

USER Accepted Roster가 P0-06에 보존하라고 확정한 planning field는 다음이다.

```text
Family
Variant
Role
DesignIntent
RelativeIntent
BaseContent
ProductionWave
Readiness
TechnologyDependency
SharedDataPolicy
VariantOwnedDataPolicy
```

현재 `FCFContentAuthoringMetadata`는 다음만 canonical하게 보존한다.

```text
FamilyId
BaseContentId
DesignIntent
AssignedProfileIdsByDomain
```

따라서 현재 상태로 exact8을 Change Set으로 옮기면 Variant/Role/RelativeIntent/Wave/Readiness/Dependency/Sharing policy 일부가 free-form text 또는 외부 문서에만 남게 된다.

교정 조건:

- 위 planning field를 Core 공통 metadata, typed child metadata 또는 provider-owned typed metadata 중 어디가 소유할지 exact1로 Freeze한다.
- `RelativeIntent`는 단순 설명 문자열 하나가 아니라 최소한 stable dimension + direction/equality 표현이 가능해야 한다.
- `Readiness`와 `TechnologyDependency`는 Product Apply 가능 여부와 Roster 포함 여부를 분리해서 표현한다.
- Guided Missile Wave 1B conditional처럼 **Roster에는 존재하지만 Product acceptance는 blocker가 남아 있는 상태**를 정확히 표현할 수 있어야 한다.
- shared-data policy와 variant-owned-data policy가 AI planning에서 보이되 Product payload field ownership과 혼동되지 않아야 한다.

##### P1-3 — Retire Lifecycle의 Canonical 저장 계약 부재

`ECFContentChangeOperationType`에는 `RetireRecord / ReactivateRecord`가 존재하고 Compiler는 desired Workbook에서 사라진 current record를 `RetireCandidate`로 표시한다. 그러나 현재 `FCFContentRecord`에는 Active/Deprecated/Retired 같은 canonical lifecycle field가 없다.

이는 기존 Plan의 다음 안전 원칙과 충돌한다.

```text
row 삭제 != Product delete
Retire = asset 보존 + 신규 사용 제한
Delete = P0 automatic exact0
```

교정 조건:

- Content lifecycle을 canonical typed state로 보존한다.
- 최소 `Active / Deprecated / Retired`를 명시적으로 구분한다.
- Workbook physical row removal은 정상 retire 표현으로 사용하지 않는다.
- `RetireRecord`는 lifecycle state change로 해석하고 Product asset delete/rename/move를 호출하지 않는다.
- retired target을 required reference가 계속 가리키는 경우 preview에서 fail-visible 또는 explicit policy diagnostic을 제공한다.
- `ReactivateRecord`는 같은 ContentKey를 유지하며 새 identity를 발급하지 않는다.

##### P1-4 — Change Set Semantic Legality / Bulk Conflict 계약 부족

현재 `ValidateChangeSet`은 schema id, hash 형식, stable ID와 operation별 필수 identifier 등 **shape validation**은 수행하지만, exact base Workbook/Schema/Snapshot에 대한 semantic legality를 아직 검증하지 않는다.

P0-06 bulk operation에서는 다음 충돌을 deterministic하게 차단해야 한다.

```text
같은 record에 Add + Retire
Retire 뒤 UpdateField
동일 field에 상충하는 Update exact2
없는 record Update
기존 record Add
없는 child Remove
같은 child에 Remove + Upsert
ExternalReadOnly target mutation
ReadOnly/ExternalManaged/Deprecated field mutation
schema에 없는 SheetId/ColumnId
dependency상 retire 불가 target
```

교정 조건:

- `ValidateChangeSetAgainstSnapshot` 성격의 semantic preflight를 정의한다.
- ordered operation 나열 순서가 우연히 결과 의미를 바꾸지 않도록 canonical conflict/normalization rule을 둔다.
- AddRecord는 뒤따르는 field/child operation을 포함한 **post model 전체가 validation PASS한 뒤에만 staged Workbook 후보가 될 수 있다.**
- bulk add/update/retire는 all-or-nothing Product transaction을 의미하지 않는다. P0-06에서는 staged Workbook proposal의 deterministic semantic consistency를 의미한다.
- operation마다 target precondition 또는 expected-before semantic을 필요한 수준으로 결속해 stale/ambiguous update를 차단한다.

##### P1-5 — Rationale / Review Package / Approval Binding 부족

현재 Change Set은 top-level `Reason` / `DesignIntent`를 가지지만 exact8처럼 여러 Family/Variant를 한 번에 제안할 때 **어느 record/operation이 어떤 이유로 바뀌었는지**를 typed review artifact로 보존하기 부족하다.

또한 현재 Compile Preview는 존재하지만 USER approval이 정확히 어떤 proposal/snapshot/post-state를 승인했는지 결속하는 immutable review package contract가 없다.

교정 조건:

```text
FCFContentChangeProposal
- ChangeSet
- BaseCatalogSnapshotFingerprint
- per-record rationale
- per-record/operation design intent delta
- affected references
- blocking/non-blocking diagnostics

FCFContentReviewPackage
- ChangeSetId / fingerprint
- BaseCatalogSnapshotFingerprint
- BaseWorkbookSemanticHash
- ExpectedPostSemanticHash
- Preview diff
- Impact summary
- Validation result
- Approval scope
```

- USER가 raw operation list를 직접 해석해야 하는 UX를 정상 경로로 만들지 않는다.
- approval은 mutable in-memory proposal이 아니라 exact review package fingerprint에 결속한다.
- review 후 proposal/snapshot/post hash가 바뀌면 기존 approval은 무효다.
- 일부 record만 승인하는 기능을 P0에 허용할지, Change Set exact1 단위 승인만 허용할지 구현 전에 exact1로 Freeze한다.

##### P1-6 — Stale Guard가 Workbook 외 Planning Truth까지 결속되어야 함

기존 `BaseWorkbookSemanticHash`는 Workbook concurrency token으로 적절하며 그대로 유지한다. 그러나 P0-06의 AI planning은 Workbook만 보는 것이 아니라 current Product truth, ExternalReadOnly reference, Resource Catalog와 Provider schema를 함께 본다.

Workbook hash가 동일해도 다음은 바뀔 수 있다.

```text
current Product DA direct edit/drift
Resource move/missing/type change
Provider schema revision
ExternalReadOnly target state
```

교정 조건:

- Workbook write stale guard = 기존 `BaseWorkbookSemanticHash` exact check 유지.
- Planning/review stale guard = P1-1의 `BaseCatalogSnapshotFingerprint` exact check 추가.
- review 직전 fresh snapshot fingerprint가 다르면 proposal을 자동 보정하지 않고 `StalePlanningSnapshot`으로 fail-closed한다.
- deterministic replan은 새 snapshot을 명시적으로 획득한 뒤 새 ChangeSet/ReviewPackage identity로 수행한다.
- same Workbook hash라는 이유만으로 old AI rationale/impact를 재사용하지 않는다.

##### no-direct-DA-write Guard — 기존 방향 PASS, P0-06 enforcement만 Freeze

이 항목에서는 blocking 구조 결함을 발견하지 않았다.

현재 근거:

- `FCFContentCompiler::CompilePreview`는 read-only이며 Product mutation을 호출하지 않는다.
- Compiler implementation은 `ApplyReviewed/Save/rename/move/delete`를 호출하지 않는 계약을 이미 가진다.
- Provider의 `ApplyReviewed` seam은 존재하지만 P0-06 normal planning path와 분리돼 있다.
- P0-07 cutover 전 Product Apply authority는 닫혀 있다.

P0-06 implementation acceptance에는 다음 exact guard를 추가한다.

```text
AI Proposal
→ typed Change Set
→ staged Workbook model/xlsx candidate
→ parse/revalidate
→ Preview/Review

Product Provider.ApplyReviewed call count = exact0
SavePackage / MarkPackageDirty = exact0
Product DataAsset mutation = exact0
physical rename/move/delete = exact0
```

즉 P0-06 writer가 생기더라도 write target은 **staged/canonical Workbook 경계뿐**이며 Product DA writer로 우회 연결하지 않는다.

##### P2 — 비차단 설계 항목

```text
P2-1: AI에 전달하는 Catalog Snapshot의 bounded projection / token budget
P2-2: exact8 이상 batch에서 Family/Variant별 review grouping과 summary UX
P2-3: 서로 다른 AI proposal exact2를 향후 merge/rebase할 future seam
```

P2-1은 AI가 전체 raw catalog를 매번 직렬화하지 않도록 deterministic summary + requested detail expansion seam을 고려한다. 단, 축약 projection이 canonical snapshot authority를 대체해서는 안 된다.

P2-2는 USER가 8개 무기의 수십 개 operation을 직접 읽지 않고 Family → Variant → changed dimensions 순으로 검토할 수 있게 하는 presentation 문제다.

P2-3은 P0에서 자동 merge를 구현하지 않는다. 같은 stale base에서 나온 proposal exact2가 있으면 우선 별도 proposal로 유지하고, future explicit merge/replan seam만 막지 않는다.

Design Review Verdict:

```text
Direction / Existing Core Reuse = ACCEPT
P0 = 0
blocking P1 = 6
P2 = 3
CCAS-P0-06 Implementation = HOLD
Exact Next = P1-1~P1-6 Contract Correction → Fresh Design Re-review

Workbook/DataAsset Authoring = NOT OPENED
CCAS Mutation Implementation = NOT OPENED
Product Apply/Save = NOT OPENED
P0-07 Authority Cutover = NOT OPENED
```

#### 2026-09-30 CCAS-P0-06 Contract Correction — FROZEN

2026-09-30 Pre-Implementation Design Review의 blocking P1 exact6을 기존 P0-01~05 Core/Compiler/Provider를 재설계하지 않는 최소 계약으로 다음과 같이 교정·동결한다.

##### P1-1 Correction — Immutable Catalog Snapshot

P0-06의 AI planning authority는 live UI state나 search/filter 결과가 아니라 immutable Catalog Snapshot이다.

Logical contract:

~~~
FCFContentCatalogSnapshot
- SnapshotFingerprint
- WorkbookSourceId
- WorkbookSemanticHash
- WorkbookSchemaVersion
- ProviderSchemaFingerprint
- ProductStateFingerprint
- ResourceCatalogFingerprint
- CanonicalRecords + PlanningMetadata
- Desired + Current Dependency/Referencer Graph
- Validation Summary
- Generated Drift Summary
~~~

최소화 원칙:

- 별도 random SnapshotId를 추가하지 않는다.
- SnapshotFingerprint = canonical SHA-256 자체가 snapshot identity다.
- fingerprint 입력은 stable machine identity와 semantic state만 포함하고 UI selection/search/filter/정렬/표시 문자열은 제외한다.
- ProviderSchemaFingerprint는 등록 Provider의 stable type/schema/ownership descriptor를 canonical sort해 계산한다.
- ProductStateFingerprint는 Provider read-only current snapshot/fingerprint와 ExternalReadOnly node state를 canonical sort해 계산한다.
- ResourceCatalogFingerprint는 registered ResourceId의 existence/type/binding-relevant capability state를 canonical sort해 계산한다.
- ExternalReadOnly record, missing/unready resource, generated drift와 blocking validation은 snapshot에서 누락하지 않는다.
- AI에 전달하는 bounded summary는 snapshot의 projection일 뿐 authority가 아니다. 상세 expansion을 하더라도 같은 SnapshotFingerprint에 결속한다.
- P0-06 first implementation은 snapshot authority를 whole registered CCAS Catalog 기준으로 단순화한다. domain-scoped fingerprint 최적화는 P2/future seam으로 남긴다.

##### P1-2 Correction — Planning Metadata Fidelity

기존 FCFContentAuthoringMetadata의 FamilyId / BaseContentId / DesignIntent / AssignedProfileIdsByDomain은 보존한다.

P0-06은 그 위에 logical PlanningMetadata를 추가하는 것으로 Freeze한다.

~~~
PlanningMetadata
- VariantId
- RoleId
- RelativeIntent[]
- ProductionWaveId
- Readiness
- TechnologyDependencies[]
- SharedDataRoleIds[]
- VariantOwnedDataRoleIds[]
~~~

RelativeIntent:

~~~
DimensionId
Direction = Decrease | Equal | Increase
~~~

P0에서는 강도 점수나 자동 balance formula를 추가하지 않는다. 예를 들어 Damage ↑ / FireRate ↓ / Accuracy = 를 stable dimension + direction으로 보존하면 충분하다.

Readiness:

~~~
Planned
AuthoringReady
Conditional
Blocked
~~~

TechnologyDependency:

~~~
DependencyId
State = Satisfied | Pending | Blocked
Reason
~~~

소유권:

- Family / Variant / Role / BaseContent / DesignIntent / RelativeIntent / ProductionWave / Readiness / TechnologyDependency = Core planning metadata.
- SharedDataRoleIds / VariantOwnedDataRoleIds의 container는 Core planning metadata가 소유하고, 실제 DataRoleId vocabulary와 의미는 Provider가 소유한다.
- Product payload field ownership(CCASManaged / ExternalManaged / 기타)과 Shared/Variant-owned planning policy는 서로 다른 계약이며 혼동하지 않는다.
- planning metadata는 runtime gameplay payload가 아니며 Product Apply 권한을 만들지 않는다.

Weapon Roster v0.1.1 대표 표현:

~~~
GuidedMissile.Standard
ProductionWaveId = Wave1B
Readiness = Conditional
TechnologyDependency = PFP.P0.03.UserTrajectory / Pending
~~~

따라서 Roster에는 exact8로 존재하면서도 Product Acceptance가 아직 차단된 상태를 손실 없이 표현한다.

##### P1-3 Correction — Explicit Content Lifecycle

Canonical record는 top-level Core semantic으로 다음 lifecycle exact1을 가진다.

~~~
Active
Deprecated
Retired
~~~

규칙:

- 신규 record 기본 lifecycle = Active.
- RetireRecord = 같은 ContentKey의 lifecycle을 Retired로 변경하는 explicit intent.
- ReactivateRecord = 같은 ContentKey를 Active로 되돌리며 새 ContentId를 발급하지 않는다.
- Deprecated는 아직 존재하고 참조 가능하지만 신규 사용을 권장하지 않는 authoring state다.
- Managed record의 physical row 삭제는 retire authority가 아니다.
- base snapshot에 존재하던 Managed row가 staged Workbook에서 설명 없이 사라지면 ImplicitRecordRemoval blocking diagnostic.
- 기존 P0-02 RetireCandidate는 current-only/desired-missing 상황을 알려주는 review diagnostic으로 유지할 수 있지만, 그것만으로 lifecycle mutation이나 Product delete를 수행하지 않는다.
- Retired record의 Product asset/package는 보존한다.
- physical asset delete/rename/move = P0-06 exact0.
- required active referencer가 Retired target을 참조하면 RetiredRequiredReference blocking.
- optional/legacy reference 허용 여부는 Provider relation policy가 explicit하게 선언해야 하며 silent ignore하지 않는다.

##### P1-4 Correction — Change Set Semantic Preflight / Bulk Conflict

기존 ValidateChangeSet shape validation은 유지하고, 그 뒤에 conceptual ValidateChangeSetAgainstSnapshot semantic preflight exact1을 둔다.

Pipeline:

~~~
Shape Validate
→ Base Snapshot exact check
→ Semantic Target/Ownership Validate
→ Conflict Normalize/Reject
→ transient Canonical Model에 operations 적용
→ full schema/reference/provider validation
→ ExpectedPostSemanticHash 계산
→ Review Package 생성
~~~

P0 semantic target:

~~~
Top-level record:
SheetId + ContentKey

Field:
SheetId + ContentKey + ColumnId

Child field upsert:
SheetId + ContentKey + ChildItemId + ColumnId

Child removal:
SheetId + ContentKey + ChildItemId
~~~

따라서 UpsertChild는 P0에서 whole arbitrary struct blob이 아니라 stable child field target을 갱신한다.

허용:

- AddRecord exact1 + 같은 신규 record의 field/metadata/child operations.
- 최종 staged record 전체가 validation PASS해야 proposal이 reviewable하다.

차단:

~~~
existing record AddRecord
missing record field/child update
AddRecord + RetireRecord same record
RetireRecord + any field/child mutation same record
RetireRecord + ReactivateRecord same record
same semantic field target operation exact2 이상
same child target Remove + Upsert
missing child Remove
ExternalReadOnly record mutation
non-CCASManaged / ReadOnly / ExternalManaged / Deprecated field mutation
unknown SheetId / ColumnId / Collection target
dependency policy상 retire 불가
~~~

동일 target의 duplicate operation은 값이 우연히 같더라도 P0에서 자동 dedupe하지 않고 ambiguity로 Block한다.

operation 입력 순서가 semantic 결과를 바꾸지 않도록 semantic preflight가 target별 충돌을 먼저 검증한 뒤 deterministic canonical order로 transient model을 만든다.

Concurrency 최소화:

- P0 authoritative concurrency token은 whole BaseCatalogSnapshotFingerprint + BaseWorkbookSemanticHash exact1이다.
- 별도 per-operation revision token은 P0에서 추가하지 않는다.
- target-level fingerprint는 diagnostic/impact 최적화용 future seam으로 남긴다.

bulk add/update/retire는 staged Workbook proposal의 deterministic consistency를 의미하며 cross-Product all-or-nothing transaction 의미가 아니다.

##### P1-5 Correction — Rationale / Review Package / Approval

AI proposal은 top-level Reason만으로 끝내지 않고 touched record별 rationale exact1을 가진다.

~~~
FCFContentRecordRationale
- ContentKey
- Reason
- DesignIntentDelta
- RelatedOperationIndexes
~~~

AI가 제시하는 expected impact는 참고 metadata일 수 있으나, authoritative affected references / dependency impact는 Compiler Preview가 다시 계산한 결과를 사용한다.

Logical proposal:

~~~
FCFContentChangeProposal
- ChangeSet
- BaseCatalogSnapshotFingerprint
- RecordRationales[]
~~~

Review artifact:

~~~
FCFContentReviewPackage
- ReviewPackageFingerprint
- ChangeSetId
- ChangeSetFingerprint
- BaseCatalogSnapshotFingerprint
- BaseWorkbookSemanticHash
- ExpectedPostSemanticHash
- RecordRationales
- Preview Diff
- Impact Summary
- Validation / Diagnostic Summary
- NoDirectDAWriteGuard Result
~~~

ReviewPackageFingerprint는 위 machine-semantic payload를 canonical sort/hash한 SHA-256이며 UI formatting은 제외한다.

P0 approval scope를 Change Set exact1 전체 승인으로 Freeze한다.

- partial record approval = P0 비범위.
- 부분적으로 채택하려면 새 Change Set을 만들어 새 Preview/Review를 수행한다.
- USER approval은 exact ReviewPackageFingerprint에 결속한다.
- proposal, base snapshot, validation, diff, impact 또는 expected post hash가 달라져 ReviewPackageFingerprint가 바뀌면 기존 approval은 무효다.
- raw operation list만 보여주고 승인받는 UX를 정상 경로로 사용하지 않는다. USER view는 Family → Variant/Record → 이유 → 핵심 변경 → 영향/경고 순으로 projection한다.

##### P1-6 Correction — Two-Layer Stale Guard

기존 Workbook concurrency 계약은 그대로 유지한다.

~~~
Workbook write stale guard
= BaseWorkbookSemanticHash exact
~~~

P0-06 planning/review에는 별도 exact guard를 추가한다.

~~~
Planning stale guard
= BaseCatalogSnapshotFingerprint exact
~~~

Fresh snapshot fingerprint에는 P1-1에 따라 다음이 결속된다.

~~~
Workbook semantic
Provider schema/ownership
current Product semantic state
ExternalReadOnly node state
Resource existence/type/capability state
dependency/referencer state
generated drift/validation-relevant state
~~~

따라서 Workbook이 같아도 Product direct edit, Resource move/missing, Provider schema 변화 또는 ExternalReadOnly 상태 변화가 있으면 old review는 stale이다.

P0-06 future deterministic Workbook Apply 직전 규칙:

1. canonical Workbook stable disk snapshot 재획득
2. fresh Catalog Snapshot 재구성
3. BaseWorkbookSemanticHash exact compare
4. BaseCatalogSnapshotFingerprint exact compare
5. exact ReviewPackageFingerprint approval 확인
6. 하나라도 다르면 mutation0 + StalePlanningSnapshot 또는 StaleWorkbook으로 Block

자동 rebase / 자동 rationale 수정 / blind retry는 하지 않는다.

Replan:

~~~
fresh Snapshot
→ new ChangeSetId
→ new Proposal
→ new Preview
→ new ReviewPackageFingerprint
→ new USER approval
~~~

same WorkbookSemanticHash라는 이유만으로 old rationale/impact/approval을 재사용하지 않는다.

##### P0-06 No-Direct-DA-Write Guard — FROZEN

기존 Design Review에서 PASS한 방향을 그대로 유지한다.

~~~
AI Planning
→ Catalog Snapshot
→ typed Change Proposal
→ transient/staged Workbook candidate
→ parse/revalidate
→ Compiler Preview
→ Review Package
→ USER approval
→ [P0-06 future] deterministic Workbook write only

Product Provider.ApplyReviewed = exact0
SavePackage / MarkPackageDirty = exact0
Product DataAsset mutation = exact0
physical rename/move/delete = exact0
P0-07 authority cutover = exact0
~~~

P0-06에서 Product provider seam은 read-only current snapshot/diff/impact evidence 용도로만 사용한다.

#### 2026-09-30 CCAS-P0-06 Fresh Design Re-review

재검수 입력:

- 위 P1-1~P1-6 frozen correction
- ContentAuthoringSystemPlan 기존 C1~C10
- USER Accepted WeaponContentRosterPlan.md v0.1.1
- current P0-01~05 Core/Compiler/Provider interfaces
- 기존 Technical PASS evidence는 재검증하지 않음

재검수 결과:

##### P1-1 Snapshot — PASS

- whole registered Catalog exact1 fingerprint로 authority를 단순화했다.
- UI projection과 authority를 분리했다.
- Workbook/Product/ExternalReadOnly/Resource/Provider truth를 fingerprint 입력으로 명시했다.

##### P1-2 Planning Metadata — PASS

- Roster v0.1.1의 exact11 planning field가 canonical owner를 모두 가진다.
- RelativeIntent를 typed direction으로 정의했다.
- Guided Missile Wave1B Conditional + PFP pending을 Roster 존재와 Product Acceptance readiness를 분리해 표현할 수 있다.
- shared/variant-owned policy는 provider DataRole vocabulary를 사용해 Product field ownership과 분리했다.

##### P1-3 Lifecycle — PASS

- Active/Deprecated/Retired exact3을 explicit canonical state로 동결했다.
- row removal과 Retire를 분리했다.
- Reactivate identity 보존과 Product delete exact0을 명확히 했다.

##### P1-4 Semantic Change Set — PASS

- existing shape validation 위에 snapshot-aware semantic preflight를 추가하는 최소 구조로 수렴했다.
- bulk conflict exact set과 child field target을 동결했다.
- whole snapshot concurrency를 사용해 per-operation revision framework 추가를 피했다.

##### P1-5 Review/Approval — PASS

- per-record rationale + Compiler-derived impact를 분리했다.
- P0 approval scope를 Change Set exact1로 단순화했다.
- approval을 immutable ReviewPackageFingerprint에 결속했다.
- partial approval framework를 P0에서 제외했다.

##### P1-6 Stale Guard — PASS

- WorkbookSemanticHash와 CatalogSnapshotFingerprint의 역할을 분리했다.
- Workbook 외 planning truth 변화도 stale로 차단한다.
- stale 발생 시 auto rebase가 아니라 fresh replan을 요구한다.

##### Re-review Verdict

~~~
Direction / Existing Core Reuse = ACCEPT
P0 = 0
blocking P1 = 0
P2 = 3 non-blocking
CCAS-P0-06 Pre-Implementation Design Review = PASS

Implementation Readiness = READY
Implementation = NOT OPENED
Workbook/DataAsset Authoring = NOT OPENED
Product Apply/Save = NOT OPENED
P0-07 Authority Cutover = NOT OPENED
~~~

P2 exact3은 그대로 비차단 유지한다.

~~~
P2-1 AI-visible Snapshot bounded projection / token budget
P2-2 Family/Variant grouped batch review UX
P2-3 future multi-proposal merge/rebase seam
~~~

Exact next:

~~~
CCAS-P0-06 Implementation
- Catalog Snapshot + fingerprint
- Planning Metadata extension
- explicit Lifecycle
- snapshot-aware Change Set semantic preflight
- Change Proposal / Review Package
- two-layer stale guard
- no-direct-DA-write guard tests

단, USER가 구현 진행을 지시하기 전에는 Implementation boundary를 열지 않는다.
~~~

### 22.7 CCAS-P0-06 Implementation + Fresh Implementation Review — PASS

2026-09-30 USER 지시에 따라 v0.1.12 `PRE-IMPLEMENTATION DESIGN PASS`의 frozen P0-06 범위만 구현했다.

구현 순서는 설계에서 동결한 다음 순서를 유지했다.

```text
Catalog Snapshot / Fingerprint
→ PlanningMetadata
→ Lifecycle
→ snapshot-aware Change Set semantic preflight
→ Change Proposal / Review Package
→ two-layer stale guard
→ no-direct-DA-write guard tests
```

#### 구현 결과

- `CFContentTypes.h v1.2.0`
  - `PlanningMetadata` typed contract 추가
  - `Active / Deprecated / Retired` lifecycle exact3 추가
  - `BaseCatalogSnapshotFingerprint`와 `SetPlanningMetadata` Change Set contract 추가
- `CFContentCore.h/.cpp v1.2.0`
  - PlanningMetadata/Lifecycle validation + Workbook semantic hash 포함
  - Workbook semantic protocol을 planning/lifecycle 포함 버전으로 승격
- `CFContentCompiler.h v1.2.0 / .cpp v1.3.0`
  - preview 시작/종료 current Product consistency를 통과한 `CurrentProductSemanticHash`를 Snapshot input으로 노출
- `CFContentProvider.h v1.1.0 / .cpp v1.2.0`
  - registered Provider schema/ownership 전체의 deterministic `ProviderSchemaFingerprint` 추가
- `CFContentResource.h v1.2.0 / .cpp v1.3.0`
  - registered ResourceId existence/type/mesh capability 상태의 deterministic `ResourceCatalogFingerprint` 추가
  - missing/unready/type-mismatch/capability-failure 상태도 fingerprint에서 사라지지 않고 diagnostic과 함께 planning truth로 보존
- `CFContentPlanning.h v1.0.0 / .cpp v1.1.0`
  - immutable whole-Catalog Snapshot
  - typed Change Proposal + per-record rationale exact1
  - authored order와 독립적인 Add → mutation → lifecycle transient semantic apply
  - existing Generic Compiler를 재사용한 Product diff / impact / provider validation preview
  - immutable ReviewPackageFingerprint + exact approval binding
  - BaseWorkbookSemanticHash + BaseCatalogSnapshotFingerprint two-layer stale guard
  - Retired referenced-target conservative fail-closed
  - ExternalReadOnly / non-CCASManaged / unknown SheetId / duplicate-conflicting semantic target fail-closed
  - persistent Workbook writer 및 Product writer를 열지 않는 review-only memory adapter
- `CFContentPlanningTests.cpp v1.1.0`
  - P0-06 focused Automation exact7
- `Tools/RunCCASP006Tests.ps1 v1.0.0`
  - exact7 focused runner

#### Fresh Implementation Review correction

최초 구현 뒤 테스트 PASS만으로 종료하지 않고 v0.1.12 frozen contract와 source를 다시 대조했다.

blocking P1 exact2를 발견했고 둘 다 최소 교정했다.

1. **Resource unready state snapshot omission**
   - 최초 구현은 registered resource가 이후 missing/unready가 되면 ResourceCatalogFingerprint 생성 자체를 실패시켰다.
   - frozen contract는 missing/unready 자체도 planning truth 변화이므로 SnapshotFingerprint에 반영해야 한다.
   - 교정 후 readiness 문제는 blocking diagnostic으로 남기되 fingerprint는 계속 생성되며 readiness 변화가 fingerprint를 바꾼다.
   - P0-03 `AssetValidation` 안에 ready → missing-picker 상태 fingerprint change 회귀를 추가했다.

2. **top-level operation SheetId semantic validation 누락**
   - field/child operation은 후속 apply 단계에서 Sheet 검증이 있었지만 `SetPlanningMetadata`/Lifecycle operation은 unknown SheetId를 공통 preflight에서 차단하지 못했다.
   - 모든 operation을 exact SheetId + ContentType에 결속하고 top-level/planning/lifecycle은 primary sheet, child operation은 child sheet를 사용하도록 공통 semantic preflight를 추가했다.
   - P0-06 `BulkSemanticPreflight`에 unknown SheetId fail-closed 회귀를 추가했다.

교정 뒤 fresh review 기준 blocking defect는 0이다.

#### Validation evidence

```text
Official UE 5.8 Source Build
- job: 1609b68d669c4152a3566dd1a529e928
- Result: PASS
- ExitCode: 0

P0-06 focused Automation
- job: 51ae0d409291495dae923822c02fc76d
- exact7 / 7 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate terminal 0

Affected P0-03 Resource regression after final resource correction
- job: 8afac2ea0df344d4919d1cbf81efb8e0
- exact8 / 8 PASS

Affected earlier Core/Compiler regression during implementation
- P0-01: job 1e453d4023724cbc8c591de353941f04 — exact8 / 8 PASS
- P0-02: job 068f8fe49de147fc9d31014f6c342999 — exact12 / 12 PASS
```

#### no-direct-DA-write review

P0-06 review path는 Product writer authority를 갖지 않는다.

```text
Provider.ApplyReviewed call = exact0
SavePackage / MarkPackageDirty = exact0
Product DataAsset mutation = exact0
Product Apply / Save = exact0
asset rename / move / delete = exact0
persistent Workbook staged write / reopen = exact0 in P0-06 review path
P0-07 cutover = exact0
```

`NoDirectDAWrite` focused test는 disposable Provider의 `ApplyReviewed` call count exact0과 no-persistent-Workbook-write guard를 확인한다.

#### Final Verdict

```text
Direction / Existing Core Reuse = ACCEPT
P0 = 0
blocking P1 = 0
P2 = 3 non-blocking (design inherited)
CCAS-P0-06 Technical Implementation = PASS
Fresh Implementation Review = PASS

Workbook/DataAsset Product Authoring = NOT OPENED
Provider.ApplyReviewed Product Mutation = NOT OPENED
Product Apply/Save = NOT OPENED
P0-07 Authority Cutover = NOT OPENED
```

P2 exact3은 v0.1.12 Design PASS에서 그대로 비차단 유지한다.

```text
P2-1 AI-visible Snapshot bounded projection / token budget
P2-2 Family/Variant grouped batch review UX
P2-3 future multi-proposal merge/rebase seam
```

Exact next는 P0-07 자동 진입이 아니다.

```text
CCAS-P0-06 USER PASS / COMPLETE 상태 보존
→ Workbook/DataAsset Product authoring 및 Product Apply/Save는 계속 CLOSED
→ 별도 USER explicit approval 전 P0-07 Authority Cutover / Migration은 계속 CLOSED
```

### 22.8 CCAS-P0-06 USER Acceptance Review Package — ACCEPTED / USER PASS / COMPLETE

2026-09-30 USER 요청에 따라 이미 PASS한 P0-06 Technical Implementation을 다시 구현하지 않고, 실제 `AI Change Proposal → Review Package` 경로를 USER Acceptance용 disposable 대표 시나리오로 검수했다.

이번 검수의 대표 입력은 USER Accepted `WeaponContentRosterPlan.md v0.1.1`의 First Production Wave exact8이다.

#### 대표 입력과 기존 Product identity 보존

기존 Product anchor는 rename하거나 신규 identity로 대체하지 않는다.

```text
HeavyCannon
→ Cannon / Heavy existing Product anchor
→ Product gameplay payload = AlreadyCurrent
→ PlanningMetadata만 Heavy Variant 역할로 review

RocketLauncher
→ UnguidedRocket / Salvo existing Product anchor
→ Product gameplay payload = AlreadyCurrent
→ PlanningMetadata만 Salvo Variant 역할로 review
```

신규 Product 후보 exact6은 다음이다.

```text
Cannon_Standard
Cannon_LongRange
Cannon_Rapid
Rocket_Standard
Rocket_Ripple
GuidedMissile_Standard
```

Guided Missile Standard는 `Wave1B / Conditional`을 유지하고 `PFP.P0.03.UserTrajectory` dependency를 Pending으로 보존한다. 따라서 이 Review Package가 승인되더라도 Guided Missile Product Acceptance 조건을 우회하지 않는다.

#### 실제 AI Change Proposal → Review Package 결과

USER review 전용 disposable Automation이 실제 `FCFContentPlanningService::BuildReviewPackage()` 경로를 호출했다.

```text
ChangeSetId
P006.UserReview.WeaponWave1

BaseCatalogSnapshotFingerprint
sha256:d6c486579f6741916250587457bf7822fcf7f3adda32a129e63c29b0856ba2df

BaseWorkbookSemanticHash
sha256:1b12b3fb3f3f9d829622306b75e1f7e06390fcb4e534a45197e3a058871251ac

ExpectedPostSemanticHash
sha256:1ddb7be0d18188497dca40433992377a2ad803c729731e780dc0f8b5551959b1

ReviewPackageFingerprint
sha256:45cc3e77c7bd5b7e7e676ee09668eb82c3e834e212c6336104c7b4d8b3db4ec2
```

Review package 규모:

```text
Typed Operations = exact38
Touched Record Rationales = exact8
Review Changes = exact38

Desired = 9
Current = 3
ReadyToReview = 6
AlreadyCurrent = 2
ExternalReadOnly = 1
RetireCandidate = 0
Blocked = 0
```

여기서 `AlreadyCurrent = 2`는 **아무 변화가 없다는 뜻이 아니다**.

```text
Planning layer
- HeavyCannon: Family/Variant/Role/DesignIntent planning change 있음
- RocketLauncher: Family/Variant/Role/DesignIntent planning change 있음

Product gameplay payload layer
- HeavyCannon: 현재 payload를 그대로 유지
- RocketLauncher: 현재 payload를 그대로 유지
```

즉 P0-06 Review Package는 planning metadata 변화와 Product payload mutation 필요성을 분리해서 보여준다.

#### per-record rationale exact8

| Content | Review 의미 |
| --- | --- |
| `Cannon_Standard` | Prototype Heavy 수치와 분리된 Cannon Family의 신규 Standard 기준점 |
| `HeavyCannon` | 기존 Product identity를 유지하면서 Heavy Variant 역할로 정리; Standard 대비 한 발 위력 ↑ / 발사 빈도 ↓ |
| `Cannon_LongRange` | 장거리 역할 추가; Standard 대비 Range 우선 / FireRate 낮춤 |
| `Cannon_Rapid` | 별도 Machine Gun Family를 즉시 늘리지 않고 Rapid Cannon으로 역할 검증; FireRate ↑ / per-shot Damage ↓ |
| `Rocket_Standard` | 기존 Salvo 중심 RocketLauncher와 분리된 Unguided Rocket Family 기준점 |
| `RocketLauncher` | 기존 persisted Salvo Product identity 유지; Standard 대비 projectile count / burst commitment ↑ |
| `Rocket_Ripple` | 현재 Launcher Scheduler의 Ripple 축 사용; release interval ↑ / 지속 압박과 조준 수정 기회 ↑ |
| `GuidedMissile_Standard` | Wave1B Conditional; Guidance capability 사용, PFP-P0-03 USER trajectory 전 Product Acceptance 금지 |

이 fixture의 `Power` 값은 disposable provider의 required field를 채우기 위한 **synthetic test scalar**일 뿐 실제 Weapon balance, Workbook authoring, Product value 승인으로 해석하지 않는다.

#### USER Acceptance 전 안전 경계

이번 Review Package 생성·검수에서도 다음은 exact0 / closed다.

```text
Persistent Workbook write / reopen = 0
Direct DataAsset write = 0
Provider.ApplyReviewed calls = 0
Product Apply / Save = 0
SavePackage / MarkPackageDirty = 0
Asset rename / move / delete = 0
P0-07 Authority Cutover = CLOSED
```

`ReviewPackageFingerprint` exact match에 대한 approval binding 자체는 기술적으로 PASS했지만, 그 테스트의 synthetic approval은 USER 승인으로 간주하지 않는다.

#### Validation evidence

```text
Final Official UE 5.8 Build
- job: 08bea65054cc493baf2f6c4623eda1a8
- PASS / ExitCode 0

Representative USER Review Automation exact1
- job: 1a3aa4287a1946e6a3febcc2c82b99ae
- CarFight.CCAS.CF_FQ_058.P0_06_USER.WeaponRosterProposalReview
- exact1 / 1 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate terminal 0

P0-06 Technical Regression after acceptance fixture
- job: d4a6ad6964d941fdb97db581d494c98d
- exact7 / 7 PASS
- failure 0 / missing 0 / unexpected 0 / duplicate terminal 0
```

#### Current USER gate

현재 exact 상태는 다음과 같다.

```text
P0-06 Technical Implementation = PASS
Fresh Implementation Review = PASS
Representative USER Review Package = ACCEPTED
USER Acceptance Decision = PASS / COMPLETE
Accepted ReviewPackageFingerprint = sha256:45cc3e77c7bd5b7e7e676ee09668eb82c3e834e212c6336104c7b4d8b3db4ec2
P0-07 = CLOSED
```

2026-10-02 USER가 exact Review Package `sha256:45cc3e77c7bd5b7e7e676ee09668eb82c3e834e212c6336104c7b4d8b3db4ec2`의 exact4 구성 — 기존 `HeavyCannon` / `RocketLauncher` Product identity anchor 유지, 신규 Product 후보 exact6 추가, Rationale / Design Intent를 포함한 AI Change Proposal → Review Package 정보 구조, `GuidedMissile_Standard`의 Wave1B Conditional 유지 — 을 명시적으로 채택했다. 이 결정으로 `CCAS-P0-06 USER PASS / COMPLETE`를 기록한다.

이 USER Acceptance는 exact representative Review Package에 대한 승인이다. Workbook/DataAsset Product authoring, `Provider.ApplyReviewed` Product mutation, Product Apply/Save, physical asset rename/move/delete 또는 authority cutover 승인으로 확대하지 않는다.

USER가 P0-06을 채택했더라도 **P0-07은 자동으로 열지 않는다**. P0-07 Authority Cutover / Migration은 별도의 USER explicit approval Gate로 유지한다.

### CCAS-P0-07 — Authority Cutover / Migration

2026-10-02 USER가 P0-07 진입을 명시적으로 승인했다. 이 승인은 **P0-07 설계·구현 진입**을 허용하지만, 아직 Workbook을 Current authority로 승격하거나 실제 Product를 일괄 mutation하라는 무조건 승인으로 확대하지 않는다.

#### P0-07 Fresh Design Review 결론

현재 구현을 fresh rebaseline한 결과 방향 폐기 사유 P0는 exact0이다. 기존 P0-01~06 Core / Compiler / Catalog / Planning / Provider 계약은 재사용 가능하다.

다만 authority cutover를 안전하게 열기 전 blocking P1 exact10을 확인했고, 아래 계약으로 전부 교정·동결했다.

```text
P1-1 explicit managed onboarding
P1-2 USER approval → Product apply fingerprint binding
P1-3 fresh pre-apply stale guard
P1-4 concrete persistent Workbook adapter requirement
P1-5 cutover transaction / recovery state machine
P1-6 generated provenance snapshot
P1-7 direct Product edit drift policy
P1-8 no-auto-delete / retire separation
P1-9 legacy JSON Staging coexistence boundary
P1-10 Current Systems authority promotion timing
```

Fresh Design Re-review verdict:

```text
P0 = exact0
blocking P1 = exact0 after contract correction
P2 = exact2 non-blocking
CCAS-P0-07 DESIGN PASS
IMPLEMENTATION READY
Authority Cutover / Product mutation = NOT YET APPLIED
```

P2는 다음이다.

```text
P2-1 Workbook visual presentation/protection의 광범위한 실문서 compatibility matrix
P2-2 다중 proposal 동시 merge/rebase UX
```

P2는 P0-07 구현을 막지 않지만 실제 authority cutover acceptance 전에 대표 production Workbook roundtrip evidence는 반드시 확보한다.

#### P0-07-1 Explicit Managed Onboarding

Current Product DA를 한 번에 전부 CCAS authority로 뒤집지 않는다.

```text
기존 persisted Product
→ read-only import
→ canonical record
→ migration preview
→ USER가 승인한 ContentType / ContentKey만 Managed candidate
→ 나머지는 ExternalReadOnly / legacy authority 유지
```

`ECFContentManagementState::Managed`라는 이유만으로 기존 Product를 암묵적으로 onboarding하지 않는다. Cutover 대상 exact set은 별도 Cutover Manifest가 소유하며, manifest 밖의 Product는 기존 DataAsset / JSON Staging authority를 유지한다.

향후 `Planning.Readiness = Conditional`이거나 technology dependency가 Pending/Blocked인 record는 Workbook planning/catalog에는 존재할 수 있지만 Product cutover/apply 대상에서는 fail-closed한다. `GuidedMissile_Standard`는 current CF-FQ-056 completion에 따라 이 예시에서 제외되며 AuthoringReady다.

#### P0-07-2 Approval Binding + Fresh Stale Guard

현재 `FCFContentReviewedMutationPlan`의 `BaseWorkbookSemanticHash + DesiredFingerprint + bApproved`만으로는 P0-07 Product mutation 권한이 충분하지 않다.

P0-07 implementation에서는 reviewed Product apply authority를 최소 다음 exact identities에 결속한다.

```text
ChangeSetId
BaseWorkbookSemanticHash
BaseCatalogSnapshotFingerprint
ReviewPackageFingerprint
ProviderSchemaFingerprint
ContentKey
PreApplyProductFingerprint
DesiredProductFingerprint
```

Apply 직전 fresh 상태에서 다음을 다시 계산한다.

```text
canonical WorkbookSemanticHash
whole-Catalog SnapshotFingerprint
ProviderSchemaFingerprint
exact Product current/readback fingerprint
```

review 시점 값과 하나라도 다르면 Product mutation은 exact0으로 fail-closed한다.

```text
no auto rebase
no blind retry
no stale approval reuse
```

변경된 현재 상태로 새 Review Package를 만들어 다시 승인받는 것이 유일한 정상 경로다.

#### P0-07-3 Concrete Workbook Persistence

`ICFContentWorkbookAdapter`의 `WriteStagedWorkbook → ReopenWorkbook` seam은 이미 존재하지만, fresh source 기준 실제 persistent `.xlsx` concrete adapter 구현은 현재 exact0이다. 테스트 / memory adapter만 존재한다.

따라서 P0-07 implementation에는 concrete Editor-only Workbook adapter가 **필수**다.

첫 구현 기준:

```text
Pinned candidate = OpenXLSX 0.5.1
Runtime dependency = 금지
build-time network fetch = 금지
Base Workbook direct overwrite = 금지
```

정상 write flow:

```text
Base Workbook read
→ reviewed canonical model
→ sibling staged .xlsx write
→ staged reopen
→ full schema / semantic validation
→ exact expected WorkbookSemanticHash 검증
→ cutover transaction이 commit 권한을 얻은 뒤에만 canonical Workbook promotion
```

adapter가 실제 `.xlsx` write/reopen을 제공하지 못하거나 presentation/protection safety가 필요한 대표 Workbook에서 검증되지 않으면 authority cutover는 BLOCKED다.

#### P0-07-4 Cutover Transaction / Recovery

Workbook과 여러 Product Asset을 하나의 파일시스템 atomic transaction처럼 취급하지 않는다. 대신 durable Cutover Transaction 상태를 명시적으로 둔다.

```text
Prepared
→ ProductApplied
→ WorkbookCommitted
→ Verified
```

실패 상태:

```text
BlockedBeforeMutation
RecoveryRequired
```

핵심 순서는 다음이다.

```text
1. staged Workbook validation
2. fresh approval/stale guard
3. per-Product reviewed typed apply
4. per-Product readback fingerprint 검증
5. 모든 required Product가 desired fingerprint와 exact 일치할 때만 canonical Workbook promotion
6. canonical Workbook reopen + semantic hash 검증
7. provenance snapshot 기록
8. Verified 뒤에만 해당 manifest 범위의 Workbook authority 활성화
```

Product batch가 부분 성공하면 canonical Workbook은 승격하지 않는다. 이미 성공한 Product, 실패한 Product, pre/post fingerprint를 transaction evidence에 보존하고 `RecoveryRequired`로 중단한다.

자동 rollback은 수행하지 않는다. rollback이 필요하면 pre-apply canonical snapshot을 desired state로 사용하는 **별도 reviewed reverse plan**을 생성한다. 같은 transaction의 exact staged Workbook promotion 재시도는 Product/current fingerprint가 그대로일 때만 허용한다.

Workbook promotion이 Product 적용 뒤 실패한 경우에도 authority switch는 일어나지 않는다. staged Workbook과 transaction evidence를 보존하고 exact candidate를 finalize하거나 reviewed reverse plan으로 복구한다.

#### P0-07-5 Generated Provenance Snapshot

기존 `CarFight_Content.cfsnapshot.json` 목표를 P0-07 durable provenance owner로 사용한다. gameplay DataAsset마다 CCAS 전용 metadata를 강제로 추가하지 않는다.

각 Managed record에는 최소 다음 provenance를 보존한다.

```text
ContentKey
CutoverTransactionId
SourceWorkbookSemanticHash
CatalogSnapshotFingerprint
ReviewPackageFingerprint
ProviderSchemaFingerprint
DesiredCanonicalFingerprint
PreApplyProductFingerprint
PostApplyReadbackFingerprint
ManagementState
LifecycleState
```

이 snapshot은 generated artifact provenance / drift comparison용이며 Runtime gameplay dependency가 아니다.

#### P0-07-6 Direct Edit Drift Policy

CCAS-managed record의 Product DA가 Workbook 밖에서 직접 수정되면 조용히 덮어쓰지 않는다.

```text
Workbook desired fingerprint == Product readback fingerprint
→ InSync

다름
→ Drifted
```

`Drifted`에서는 normal Apply를 차단하고 USER에게 exact2 선택지만 제공한다.

```text
A. Workbook authority 유지
   → 현재 Workbook desired를 새 Review Package로 검토 후 Product에 재적용

B. Product 변경 채택
   → Product current를 read-only import해 Workbook Change Proposal로 만들고 새 Review Package 승인 후 Workbook/Generated state를 갱신
```

Product → Workbook 자동 역동기화는 금지한다.

#### P0-07-7 Retire / Delete

`Retired`는 logical lifecycle이다.

```text
RetireRecord
≠ physical asset delete
≠ row physical erase
```

P0-07 normal flow에서 Product `.uasset` delete, rename, move는 exact0을 유지한다. Physical delete는 향후 별도 destructive USER gate 없이는 수행하지 않는다.

#### P0-07-8 Existing JSON Staging Coexistence

기존 JSON Staging은 즉시 제거하지 않는다.

```text
CCAS-managed manifest 범위
- Workbook = authoring authority after Verified cutover
- JSON Staging = 기존 typed provider/durable transport로 내부 재사용 가능
- USER-facing 병렬 authoring authority로 사용 금지

Unmanaged / ExternalReadOnly / legacy 범위
- 기존 DataAsset + JSON Staging workflow 유지
```

즉 동일 ContentKey에 Workbook과 JSON Staging이라는 exact2 authoring authority를 동시에 두지 않는다.

#### P0-07-9 Current Systems Promotion

P0-07 implementation / validation 중에는 기존 Systems 문서의 Current authority 문구를 바꾸지 않는다.

다음 exact 조건을 모두 충족한 **Verified managed manifest**에 한해서만 Systems를 갱신한다.

```text
concrete Workbook persistent roundtrip PASS
fresh approval/stale guard PASS
Product reviewed apply + readback PASS
canonical Workbook commit + reopen PASS
provenance snapshot PASS
drift / partial failure / recovery focused tests PASS
USER cutover acceptance PASS
```

그 전까지 Current truth는 계속 기존 계약이다.

```text
DataAssetManagement:
persisted Unreal DataAsset = current source of truth

DataAssetAuthoring:
Staging JSON = authoring intent
persisted Unreal DataAsset = apply 뒤 current source of truth
```

이 Gate의 Design PASS만으로 Excel을 Current System의 source of truth라고 문서화하지 않는다.

#### P0-07 implementation 최소 범위

신규 대형 writer를 만들지 않고 기존 typed provider / durable apply backend를 재사용한다.

구현 책임은 다음으로 제한한다.

```text
CCAS Cutover coordinator
Cutover Manifest + durable transaction/provenance model
approval-bound reviewed Product apply adapter
fresh stale/drift guard
concrete Editor-only Workbook adapter
staged Workbook commit/reopen validation
recovery/reporting
focused Automation
```

Product provider 내부 도메인 의미, 기존 durable save core, P0-01~06 Compiler/Planning 의미는 재설계하지 않는다.

#### P0-07 Technical Implementation Result — 2026-10-02

Frozen v0.1.16 계약을 기준으로 P0-07 구현과 Final Technical Review를 완료했다.

구현 결과:

```text
CCAS Cutover coordinator = implemented
Cutover Manifest / approval binding = implemented
durable transaction / provenance model = implemented
fresh stale guard / direct Product drift detection = implemented
existing ICFContentProvider reviewed apply bridge = implemented
concrete Editor-only .xlsx persistence = implemented
staged Workbook write → reopen → semantic hash validation = implemented
partial failure → RecoveryRequired reporting = implemented
no-auto-delete / Retire separation = preserved
Workbook authority auto-activation = exact0
physical Product delete / rename / move = exact0
```

대표 source:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFContentCutover.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFContentCutover.cpp
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFNativeXlsxAdapter.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFNativeXlsxAdapter.cpp
Tools/RunCCASP007Tests.ps1
```

Workbook persistence는 repository-vendored exact versions를 Editor module에만 정적 포함한다.

```text
OpenXLSX 0.5.1
miniz 3.0.2
pugixml 1.15
Runtime dependency = exact0
build-time network fetch = exact0
```

`FCFProviderCutoverAdapter`는 기존 persisted Product migration에 한해 `ICFContentProvider`의 `BuildCurrentSnapshot → BuildDiff → BuildReviewedMutationPlan → ApplyReviewed → ReadbackFingerprint` 계약을 재사용한다. P0-07이 신규 Product writer를 새로 만들지 않도록 absent Product Create는 fail-closed한다. 신규 Product 생성은 기존 Weapon / Vehicle authoring backend 책임으로 남긴다.

concrete `.xlsx` focused evidence는 disposable repository-relative fixture에서 다음을 검증했다.

```text
base .xlsx create/read PASS
sibling staged .xlsx write PASS
staged reopen x2 PASS
expected semantic hash exact match PASS
Base Workbook no-overwrite PASS
production FileStore transaction JSON save→load PASS
production provenance JSON save→load PASS
```

Final official UE 5.8 Build:

```text
job: 3eb7fc914828419e98ace8ee3fbbd503
Result: PASS
CarFight_ReEditor compile/link PASS
```

P0-07 focused Automation final:

```text
job: 8aa7f4126a3e441f95a38d0e70243447
exact10 / 10 PASS
failure 0 / missing 0 / unexpected 0 / duplicate terminal 0

ConditionalBlocked
DirectProductDrift
DurableFileStore
ManagedCutover
PartialRecovery
PersistentXlsxRoundtrip
Provenance
ProviderCutoverBridge
RetireNoDelete
StaleApproval
```

Affected CCAS regression final:

```text
P0-01 Core: a99b635beddf4a12ba20726b2aaefe34 — exact8/8 PASS
P0-02 Compiler/Provider: 38cf9510fc834fd7812da00a7537ae8e — exact12/12 PASS
P0-03 Resource: 6c122bb2b8084e2c959e39f35140c56b — exact8/8 PASS
P0-04 Dual Consumer: 380ab9813a8849bda5442b1397b80e51 — exact5/5 PASS
P0-05 Catalog/Compare: d45ec12871c448fb8c0b48af439ff2b0 — exact5/5 PASS
P0-06 Planning/Review: 06715860cfa04e7aad360b14d6078985 — exact7/7 PASS
```

Final Technical Review verdict:

```text
P0 = exact0
blocking P1 = exact0
P2 = exact2 non-blocking
CCAS-P0-07 TECHNICAL IMPLEMENTATION PASS
USER CUTOVER ACCEPTANCE = PENDING
Authority Cutover / Product batch mutation = NOT APPLIED
Workbook Current authority activation = NOT APPLIED
Current Systems authority promotion = NOT APPLIED
P0-08 = NOT OPENED
```

P2는 기존 설계 판정 그대로 유지한다.

```text
P2-1 Workbook visual presentation/protection의 광범위한 실문서 compatibility matrix
P2-2 다중 proposal 동시 merge/rebase UX
```

`PersistentXlsxRoundtrip`으로 concrete persistent roundtrip blocker는 닫혔지만, P2-1의 광범위한 production Workbook presentation/protection matrix 자체는 별도 비차단 항목으로 남는다.

`GuidedMissile_Standard`의 historical `Wave1B / Conditional / PFP.P0.03.UserTrajectory Pending`은 CF-FQ-056 PFP-P0-03 USER ACCEPTED / COMPLETE로 해소됐다. Current Production candidate는 Wave1B identity를 유지하되 `AuthoringReady`, technology dependency exact0으로 생성한다.

다음 정상 Gate는 **별도 USER Cutover Acceptance**다. USER가 승인하기 전에는 production Product mutation, canonical Workbook promotion, Workbook Current authority activation, Current Systems authority promotion을 수행하지 않는다.

#### P0-07 Mid-review Correction + Re-review — 2026-10-03

USER 요청으로 P0-07 Technical Implementation을 authority cutover 직전 관점에서 fresh 중간검수했다. 기존 exact10 happy/fail-path 검증만으로는 실제 approval/recovery/persistence 경계를 충분히 증명하지 못하는 blocking gap을 확인했다.

Mid-review 발견:

```text
P0 = exact0
blocking P1 = exact5
correction-local P2 = exact2
verdict = CORRECTION REQUIRED
```

blocking P1 exact5:

```text
P1-1 approved ManifestFingerprint 문자열은 비교하지만 actual Manifest payload fresh rehash가 없었음
P1-2 RecoveryRequired durable state는 있으나 exact staged candidate finalize resume entry가 없었음
P1-3 transaction/provenance durable write가 direct overwrite이고 failure evidence save failure가 silent였음
P1-4 canonical Workbook promotion이 direct overwrite copy이고 promotion 직전 base/staged freshness + Office lock 재검증이 없었음
P1-5 canonical/staged/provenance execution path가 frozen exact3 production path에 결속되지 않았음
```

correction review 중 P0-06 `ValidateApproval`도 `ReviewPackageFingerprint` 문자열 동일성만 검사하고 current ReviewPackage payload를 fresh rehash하지 않는 adjacent approval-integrity gap exact1을 추가 확인했다. 이는 P0-06 planning 의미를 재설계하지 않고 기존 deterministic `BuildReviewPackageFingerprint` 계산기를 read-only helper로 재사용해 닫았다.

교정 결과:

```text
Manifest payload fresh rehash → request/approval/fresh exact3 fingerprint match
ReviewPackage payload fresh rehash → approval 문자열 + current payload exact binding
production execution path → canonical/staged/provenance frozen exact3
ManagementState/LifecycleState/ProductMutationRequired → fresh reviewed/current truth 재검증
transaction/provenance durable write → sibling .cutover-writing + readback + replace
failure journal save failure → caller diagnostic에 CRITICAL evidence-loss fail-visible
transaction deserialize → enum range guard + filename/payload TransactionId cross-check
promotion → staged raw temp/readback/replace + Office lock pre/post check
promotion 직전 → staged expected hash + canonical base hash fresh recheck
RecoveryRequired → ResumeRecovery exact1 entry
ResumeRecovery → Product reapply exact0, all current Product가 reviewed desired exact일 때만 staged candidate finalize
Recovery canonical state → reviewed base 또는 reviewed expected exact2 이외 자동 처리 금지
Workbook authority auto-activation = exact0
physical Product delete / rename / move = exact0
```

Recovery는 automatic rollback/rebase/reapply 기능이 아니다. Product partial success 상태에서 아직 desired에 도달하지 않은 target이 하나라도 있으면 `ResumeRecovery`는 fail-closed하고 별도 reviewed recovery/reverse plan을 요구한다. 반대로 Product exact targets가 이미 reviewed desired와 일치하고 canonical Workbook만 base에 남아 있으면 동일 transaction의 exact staged candidate만 finalize한다.

Final official UE 5.8 Build:

```text
job: efef4d82be0e4e989d1fad789faa8333
Result: PASS
CFContentCutoverTests.cpp compile/link PASS
all prior correction source already linked in current CarFight_ReEditor binary
```

P0-06 approval regression final:

```text
job: 3898d7ea945644a2aedaa7f55074a200
exact7 / 7 PASS
failure 0 / missing 0 / unexpected 0 / duplicate terminal 0
ApprovalBinding now includes approved-fingerprint + payload-tamper fail-closed regression
```

P0-07 focused Automation final:

```text
job: fab5fe3c03c8476285ab8d60ee42a1b4
exact16 / 16 PASS
failure 0 / missing 0 / unexpected 0 / duplicate terminal 0

ConditionalBlocked
DirectProductDrift
DurableFileStore
ExecutionPathBound
JournalFailureRecovery
JournalHardFailureVisible
ManagedCutover
ManifestTamperBlocked
PartialRecovery
PersistentXlsxRoundtrip
PromotionFreshnessGuard
Provenance
ProviderCutoverBridge
RecoveryFinalize
RetireNoDelete
StaleApproval
```

`DurableFileStore`는 기존 transaction/provenance roundtrip에 더해 atomic temp residue exact0과 filename/payload TransactionId mismatch fail-closed를 검증한다.

Fresh correction re-review verdict:

```text
P0 = exact0
blocking P1 = exact0
correction-local P2 = exact0
remaining design P2 = exact2 non-blocking
CCAS-P0-07 TECHNICAL IMPLEMENTATION PASS PRESERVED
CCAS-P0-07 MID-REVIEW CORRECTION PASS
USER CUTOVER ACCEPTANCE = PENDING
Authority Cutover / Product batch mutation = NOT APPLIED
Workbook Current authority activation = NOT APPLIED
Current Systems authority promotion = NOT APPLIED
P0-08 = NOT OPENED
```

remaining design P2 exact2는 v0.1.17의 Workbook visual presentation/protection compatibility matrix와 multi-proposal merge/rebase UX이며 이번 correctness 교정과 별개로 계속 non-blocking이다.

### CCAS-P0-08 — End-to-End Acceptance + Current System Promotion

대표 scenario:

- 다수 Weapon Family / Variant 계획 → Workbook → Diff → Apply
- 다수 Vehicle 비교 / 변경 → Workbook → Diff → Apply
- Resource 교체 1건이 여러 consumer에 미치는 영향 확인
- multi-muzzle / socket-role binding
- rename / retire / orphan
- manual DA drift 탐지
- partial batch failure / recovery
- AI Change Set
- Editor restart 후 durable state / readback
- USER 최종 workflow 검수

2026-10-07 USER FINAL WORKFLOW ACCEPTANCE PASS로 이 단계는 COMPLETE다. Current System owner는 이미 `DataAssetAuthoring.md v1.7.0`, `DataAssetManagement.md v1.1.0`, `RuntimeApply.md v1.2.0`, `VehicleBuilder.md v1.9.1`, `SystemIndex.md v1.55.0`에 동기화되어 추가 구현 변경 없이 유지한다. UDS Work는 Closed successor로 수렴하고 FeatureQueue는 Done으로 전환한다.

---

## 23. 검증 원칙

문서 / 설계 단계는 Build를 요구하지 않는다.

Source 구현부터:

```text
공식 UE 5.8 Build
→ focused Automation
→ affected provider / authoring regression
→ persisted AssetDump evidence
→ Workbook / Generated asset deterministic recompile
→ USER workflow review
```

Product mutation이 포함되는 Gate는 disposable fixture와 Product-target 검증을 분리한다.

Excel parser unit test와 UE typed materializer test를 분리하고, 동일 Workbook + 동일 Project state에서 동일 Canonical Model / Diff / Generated semantic fingerprint가 나와야 한다.

---

## 24. USER Acceptance 기준

최종 시스템은 다음 질문에 YES여야 한다.

- 차량 / 무기를 한 개씩 열지 않고 전체를 한눈에 볼 수 있는가?
- 같은 Family의 Variant들을 바로 비교할 수 있는가?
- 사용자가 ObjectPath나 Stable ID를 관리하지 않아도 되는가?
- 사용자가 "이 숫자에 뭘 넣어야 하지?"를 반복적으로 고민하지 않아도 되는가?
- AI가 여러 콘텐츠를 한 번에 기획하고 변경안을 만들 수 있는가?
- AI가 DA를 하나씩 수작업으로 만들지 않아도 되는가?
- Excel 한 곳을 authoritative authoring source로 사용할 수 있는가?
- Mesh / FX / Socket이 달라도 Resource / Role Binding으로 처리 가능한가?
- Resource 하나 변경 시 영향받는 콘텐츠를 알 수 있는가?
- Workbook과 Generated DA가 어긋나면 이유와 해결 방향이 명확한가?
- 부분 실패가 발생해도 이미 성공한 결과와 실패 범위를 정확히 알 수 있는가?
- 신규 DataAsset 타입을 Core 재설계 없이 Provider onboarding으로 추가할 수 있는가?

---

## 25. 현재 결정

2026-09-29 USER 결정으로 다음을 Formal 방향으로 채택한다.

1. 사용자 편의가 구현 난이도보다 우선한다.
2. 사용자-visible CSV export를 정상 경로로 두지 않고 `.xlsx` 직접 입력을 목표로 한다.
3. 최종 목표에서 Excel Workbook은 CCAS-managed 콘텐츠의 Authoritative Authoring Source다.
4. DA는 compiler-generated Runtime / Product artifact로 전환한다.
5. Core는 무기 전용으로 만들지 않는다.
6. Weapon + Vehicle exact2를 첫 대표 consumer로 사용한다.
7. AI는 대량 기획 / Workbook Change Set을 만들고 deterministic Compiler가 DA를 생성한다.
8. Family / Variant / DesignIntent를 다수 콘텐츠 운영의 핵심 개념으로 둔다.
9. Mesh / FX / Sound 등은 Stable Resource ID + Resource Catalog로 연결한다.
10. Socket은 raw name을 콘텐츠에 반복 입력하지 않고 Mesh capability + Semantic Role Binding으로 관리한다.
11. Profile + Override로 반복 Resource / 값을 재사용한다.
12. Schema Version, Stable ID, Null 의미, Override precedence, Unit, Derived Field, Dependency Graph, Drift, Rename / Delete lifecycle, AI Change Set을 처음부터 설계한다.
13. 기존 CF-FQ-055 Technical PASS backend와 기존 typed durable provider / DACE 기반은 버리지 않고 재사용한다.
14. 2026-10-07 Verified managed cutover 이후 managed exact42 manifest는 canonical Workbook을 sole Current authoring authority로 사용한다. unmanaged / ExternalReadOnly / legacy content는 기존 Current authority를 유지한다.
15. canonical Workbook `Authoring/Content/CarFight_Content.xlsx` exact1은 Current authoring authority로 활성화됐고 generated provenance `CarFight_Content.cfsnapshot.json` exact1을 함께 유지한다.
16. AI는 xlsx binary를 직접 수정하지 않고 typed Change Set + deterministic Workbook Applier를 사용한다.
17. Content identity는 `ContentTypeId + ContentId` composite이며 기존 WeaponId/AmmoId/DamageId 등 domain ID와 중복 identity를 만들지 않는다.
18. Product mutation은 asset-wide generic setter가 아니라 field ownership descriptor + typed Provider만 수행한다.
19. Product physical rename/move/delete는 P0 normal authoring에서 금지하고 path를 pin한다.
20. first xlsx adapter는 Editor-only isolated OpenXLSX 0.5.1 pinned candidate로 두고 Runtime dependency를 만들지 않는다.
21. unmanaged existing DataAsset은 `ExternalReadOnly` catalog node로 참조 가능하게 하되 CCAS write는 금지한다.
22. 기존 Vehicle CFBatch/Resolver/Authoring과 CFDA durable backend를 상위 CCAS에서 재사용하고 중복 writer를 만들지 않는다.

---

## 26. Changelog

### v0.1.37 - 2026-10-08

- CarFight 저장소 구조 단순화로 `Document/Plan`의 별도 `plan_repo` Git 경계를 폐지하고 `main_game` 일반 디렉터리로 통합한 Current repository authority를 반영했다.
- 상단 `Repository Role`을 `main_game`으로 교정했다. 이 변경은 repository ownership metadata only이며 CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS / COMPLETE와 기존 구현·검증 evidence를 재개하거나 변경하지 않는다.
- v0.1.36의 `plan_repo` 정정은 2026-10-07 당시 실제 저장소 구조를 설명하는 Historical evidence로 그대로 보존한다.

### v0.1.36 - 2026-10-07

- Post-closure final review에서 대표 Plan의 repository authority 메타데이터 P1 exact1을 교정했다.
- `Document/Plan`은 별도 `plan_repo` Git repository이므로 상단 `Repository Role`을 `main_game`에서 `plan_repo`로 정정했다. UDS lifecycle / Systems / Source authority가 `main_game`이라는 기존 경계는 유지한다.
- 이 교정은 documentation authority metadata only이며 CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS / COMPLETE, managed cutover, canonical Workbook, Production exact34, Publication exact8, RuntimeApply 계약과 기존 Build/Automation evidence를 재개하거나 변경하지 않는다.
- final review verdict는 P0 exact0 / blocking P1 exact0이다. stage/commit/push는 수행하지 않았다.

### v0.1.35 - 2026-10-07

- USER가 최종 workflow를 명시적으로 승인해 `CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS / COMPLETE`로 닫았다.
- 승인된 최종 사용자 흐름은 `canonical Workbook → compile/validation → generated Production DataAssets exact34 → Production Publication Catalog exact8 → normal RuntimeApply discovery/authorization → 기존 FCFRuntimeEquipApplyService Fitting Runtime → Vehicle runtime result`다.
- Production Catalog가 absent/invalid일 때만 legacy RuntimeTestCatalog Equipment fallback을 허용하며 Vehicle discovery는 기존 RuntimeTestCatalog authority를 유지한다.
- compatibility 또는 GrossMass 검증 실패는 `ValidationFailed + mutation0 + previous runtime state preserved`가 정상 계약이다.
- v0.1.34의 final technical evidence를 재사용한다: Official UE 5.8 Build PASS, focused Production Catalog 2/2 PASS, full RuntimeApply 18/18 PASS, ProductionCutoverVerification 1/1 PASS, P0 exact0 / blocking P1 exact0.
- Current System 문서는 이미 최종 기술 계약과 일치하므로 추가 semantic mutation 없이 그대로 유지했다.
- canonical UDS Work를 Closed successor로 전진시키고 FeatureQueue를 Done으로 전환한다. stage/commit/push는 수행하지 않았다.

### v0.1.34 - 2026-10-07

- v0.1.33의 post-promotion blocking P1 exact1 correction을 fresh 검증하고 implementation re-review PASS로 닫았다.
- official UE 5.8 Build `9f3b9cac267f4cba8c8fff2772908c7a`는 Editor DLL lock 해제 뒤 PASS했고, Wagon/P0-05 stale regression expectation 교정 후 final Build `97030b7aba1348d4b1688c34afc19954`도 compile/link PASS했다.
- CF-FQ-058 focused RuntimeApply `50b94943fd6546c798b7c09ae0a9b0bc`는 `ProductionCatalogAuthorization / ProductionCatalogDiscovery` exact2/2 PASS였다.
- 첫 전체 RuntimeApply run `27b37d0d067d4c06bf1e5422513b359f`은 16/18 PASS였다. 실패 exact2는 Product/runtime defect가 아니라 current Production balance와 충돌한 stale test expectation이었다: Wagon test는 과거 Prototype 장착 성공을 계속 기대했고, P0-05는 과거 1400kg 상수를 고정했다.
- `CFRuntimeApplyPIETests.cpp v1.6.0`에서 Product/Vehicle 값을 바꾸지 않고 Current truth에 맞춰 교정했다. Wagon은 Production `Cannon_Standard` 3216kg / `Rocket_Standard` 3112kg가 current Wagon gross limit 2350kg을 초과하므로 Mount compatibility 통과 뒤 `ValidationFailed + mutation0`가 정상이며, P0-05는 fixed mass 상수 대신 Applied Snapshot `TotalVehicleMassKg`와 Chaos configured mass exact readback을 비교한다.
- final full RuntimeApply regression `12eb4517709249cd9d5a8b0e5a4d25a6` 18/18 PASS, failure exact0, result JSON SHA-256 `f54303f90e1491a216324439322102a73484b46dde2467760e58ac5ff28ecb8a`다.
- post-correction `ProductionCutoverVerification` `d719964509e14441b7160972c56694bd` 1/1 PASS, failure/missing/unexpected/duplicate exact0, result JSON SHA-256 `dc69447b01b44563e0c295590fa73e6a3d6484821a9e2d58db864715c861e6fb`다. canonical Workbook semantic, provenance exact42, transaction exact8 VehicleReady, unique target exact34, Publication exact8은 그대로 유지된다.
- source re-review에서 normal RuntimeApply Equipment path가 `UCFProdEquipCatalogData::LoadProductionCatalog → ProductionEquipmentCatalog → ApplyPublishedEquipment`을 사용하고, `AllowedEquipmentPresetData`는 legacy service/test 및 Production Catalog absent/invalid fallback에만 남아 있음을 확인했다. Fitting Snapshot/Prepare/Commit/Recovery authority는 기존 `FCFRuntimeEquipApplyService`에 유지되며 중복 subsystem exact0이다.
- Current System docs를 `RuntimeApply.md v1.2.0`, `VehicleBuilder.md v1.9.1`, `SystemIndex.md v1.55.0`으로 동기화했다.
- P0-08 technical verdict는 P0 exact0 / blocking P1 exact0이다. 다음 exact gate는 `USER FINAL WORKFLOW ACCEPTANCE`이며 그 전까지 Feature Done/Closed로 승격하지 않는다.
- stage/commit/push는 수행하지 않았다.

### v0.1.33 - 2026-10-07

- v0.1.32 technical promotion 뒤 UDS가 요구한 `normal Production equipment discovery/runtime consumer` fresh audit에서 blocking P1 exact1을 확인했다. generated `DA_ProdEquipCatalog`는 Production publication exact8을 소유하지만 normal RuntimeApply equipment discovery/authorization은 `UCFRuntimeTestCatalogData::AllowedEquipmentPresetData`만 소비해 published exact8이 일반 장비 선택 경로에 나타나지 않았다.
- 따라서 v0.1.32의 `TECHNICAL CURRENT SYSTEM PROMOTION PASS / END-TO-END AFFECTED REGRESSION PASS`를 최종 P0-08 기술 완료로 사용하지 않고 post-promotion correction을 연다. Cutover 자체와 canonical Workbook / exact34 / transaction exact8 VehicleReady / publication exact8 evidence는 유지한다.
- correction은 새 RuntimeApply subsystem이나 두 번째 publication authority를 만들지 않는다. `UCFProdEquipCatalogData v1.1.0`에 stable `CFProdEquipCatalog:Production` runtime load와 exact EquipmentPreset published lookup을 추가하고, `FCFRuntimeEquipApplyService v1.3.0/1.5.0`에 Production Catalog validation/authorization + published `DefaultSortieAmmoLoads`를 기존 Fitting Runtime authority로 전달하는 seam을 추가했다.
- `UCFRuntimeApplyWidget v1.2.0`은 Equipment discovery/authorization에 valid Production Publication Catalog를 우선 사용하고 absent/invalid일 때만 legacy RuntimeTestCatalog Equipment 목록으로 fallback한다. Vehicle discovery/apply authority는 기존 RuntimeTestCatalog를 유지한다.
- affected Automation으로 `CarFight.RuntimeApply.CF_FQ_058.ProductionCatalogAuthorization`과 `CarFight.RuntimeApply.CF_FQ_058.ProductionCatalogDiscovery`를 추가했다. 기존 `CatalogOptionSync`는 test-owned fixture에서 Production Catalog를 명시 제외해 legacy fallback cache 경계를 계속 독립 검증한다.
- official UE 5.8 Build `f5a3d7934318429884dc763ef7ea2f33`은 UBT compile 이후 link 단계에서 실행 중 `UnrealEditor.exe`가 `UnrealEditor-CarFight_Re.dll`과 `UnrealEditor-CarFight_ReEditor.dll`을 사용 중이어서 LNK1104 exact2로 실패했다. source compile error로 판정하지 않으며 Editor process를 임의 종료하지 않았다.
- 현재 verdict는 `blocking P1 exact1 correction implemented / official build + focused Automation validation pending`이다. USER final workflow acceptance는 correction validation 전까지 HOLD한다.
- next exact sequence는 Editor DLL lock 해제 → official UE 5.8 Build → `CarFight.RuntimeApply.CF_FQ_058` focused Automation → affected RuntimeApply regression → ProductionCutoverVerification fresh rerun → implementation re-review다.
- stage/commit/push는 수행하지 않았다.

### v0.1.32 - 2026-10-07

- CCAS-P0-08 기술 구간으로 Current System authority promotion과 post-cutover affected regression을 완료했다. 전체 Feature Done은 USER final workflow acceptance 전에는 선언하지 않는다.
- `DataAssetManagement.md v1.1.0`은 Data Asset Manager의 read-first 경계를 유지하면서 CCAS-managed exact42와 unmanaged/legacy authority를 분리했다.
- `DataAssetAuthoring.md v1.7.0`은 managed exact42에서 `CarFight_Content.xlsx = sole Current authoring authority`, Production `.uasset` exact34 = generated Runtime/Product materialization, `DA_ProdEquipCatalog` = published Product exact8 membership, cfsnapshot exact42 = provenance/drift evidence로 Current 계약을 승격했다. 기존 JSON Staging workflow는 unmanaged/legacy 범위에서 유지한다.
- `SystemIndex.md v1.54.0`을 위 scope 분리에 맞춰 갱신했다.
- fresh post-cutover read-only verification `7aa50a8517454169be6d5727f5560251` exact1/1 PASS로 canonical Workbook approved semantic, staged residue exact0, provenance exact42, Product transaction exact8 VehicleReady, unique target union exact34, Publication exact8을 재확인했다.
- fresh Official UE 5.8 Build `eef3a6ef6cd547efaeaf295b757fa0f2` PASS.
- affected regressions: P0-04 Dual Consumer `3180a24c48254dceaaf858c79b6e627d` exact5/5 PASS, P0-05 Catalog/Compare `0a2f997445a741bc9e40cd2cc2b7a5a0` exact5/5 PASS, P0-06 Planning/Review `75fc98c5706a4ad392717a4839d3589c` exact7/7 PASS, P0-07 Production/Cutover `a4a38322498c4e61ac62e0d9b6bef8f4` exact25/25 PASS, Weapon Guide `64def850dea64438baeb457b0d6683e5` exact8/8 PASS다. 각 run의 failure/missing/unexpected/duplicate terminal은 exact0이다.
- 공용 Automation log handle release 지연은 test/product defect가 아니며 검증 중 log path만 임시 분리했다. `Tools/RunDataAuthoringTests.ps1`는 canonical 경로로 즉시 원복했고 새 실행 framework/gate를 남기지 않았다.
- P0-08 technical verdict는 P0 exact0 / blocking P1 exact0이다. exact next는 USER final workflow acceptance이며 FeatureQueue Done projection은 그 뒤에만 허용한다.
- Git stage/commit/push와 physical Product delete/rename/move는 수행하지 않았다.

### v0.1.31 - 2026-10-07

- USER가 직전 exact gate에서 `진행해`를 승인해 corrected Review Package의 USER Cutover Acceptance를 확정하고 managed authority cutover를 실제 적용했다.
- approved identity는 `ReviewPackage=sha256:9af9a9256802244e7598ca6c92c916d1f9e11335599204cd518a094b552b5940`, `PostWorkbook=sha256:049752116ea69850ffe245ce26365756bd15cfb1e332fc202339720c0ced42e3`, `ResourceCatalog=sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7`, `ExecutionBinding=sha256:1e87904bfe4e65f5614c8744af50dc0cbf533f14c64b13748178e08e6cb6243a`다.
- first execution에서 Production Damage/Ammo inline reviewed source가 기존 provider canonical StagingRoot guard를 통과하지 못한 결함을 확인했고, guard를 약화하지 않고 `CFWeaponGuideVM v1.6.2`에서 provider-owned `Authoring/DataAssetStaging/{DamageData|AmmoData}/__CCASProduction__/*.json` 경로로 정렬했다.
- 실패 중 생성된 `CF058_W1_*` Product transaction exact8은 삭제/reset하지 않고 durable recovery evidence로 보존해 동일 transaction identity로 재개했다.
- Runtime proof 초기 TestSUV fixture는 gross-mass cap과 NullRHI Automation World의 Chaos hot-mass reapply 조건까지 결합해 Product compatibility보다 넓은 failure를 만들었다. approved Product 값이나 Production VehicleData를 임의 변경하지 않고 proof boundary를 persisted Product graph / finite-ammo / Runtime Apply readback 기술 계약으로 정렬했다.
- cutover 완료 결과 canonical `Authoring/Content/CarFight_Content.xlsx`가 managed exact42 manifest의 sole Current authoring authority가 됐고 `CarFight_Content.cfsnapshot.json` provenance exact42가 생성됐다.
- durable Product transaction exact8은 모두 `VehicleReady`, Diagnostic exact0이다. target evidence union은 unique exact34이며 persisted fingerprint가 approved desired fingerprint와 exact 일치한다.
- generated Production Publication Catalog는 Product exact8 published membership과 각 ProductGraphFingerprint를 소유한다.
- cutover 완료 뒤 one-shot Production write test는 제거하고 `CFProdCutoverApplyTests.cpp v2.0.0`을 read-only `ProductionCutoverVerification`으로 전환했다.
- fresh Official UE 5.8 Build `379684343e3243efb5973eaa10a396cb` PASS. post-cutover verification `83b1b02effa242799b737a1e020efa20` 1/1 PASS, failure/missing/unexpected/duplicate exact0, result JSON sha256 `dc69447b01b44563e0c295590fa73e6a3d6484821a9e2d58db864715c861e6fb`.
- managed AssetDump discovery cache는 Production root를 0건으로 반환해 stale index evidence로 판정했다. 완료 판정은 fresh Editor verification의 Workbook/provenance/transaction/Publication readback을 사용한다.
- 상태를 `USER CUTOVER ACCEPTED / MANAGED AUTHORITY CUTOVER APPLIED / PRODUCTION EXACT8 VEHICLEREADY / CANONICAL WORKBOOK ACTIVE / PUBLICATION EXACT8 ACTIVE`로 전진시키고 `CCAS-P0-08 — End-to-End Acceptance + Current System Promotion`을 OPEN했다.
- stage/commit/push는 수행하지 않았다.

### v0.1.30 - 2026-10-07

- P1 exact3 correction 뒤 affected regression을 추가 수행했고 Weapon Guide exact8 중 `DurableCreateGraph / PartialRecovery / CompletionHandoff` exact3가 Projectile semantic fingerprint mismatch로 실패하는 직접 영향 회귀를 발견했다.
- root cause는 disabled Trail/Thruster FX slot에서 Draft builder가 기본 socket `FX_Trail / FX_Exhaust`를 채우지만 persisted readback은 비활성 하위 필드를 `None`으로 정규화하는데, v1.6.0 fingerprint가 bEnabled=false 상태에서도 하위 Niagara/AttachMode/Socket을 의미값으로 해시한 것이었다.
- `CFWeaponGuideVM v1.6.1`에서 disabled FX slot은 `bEnabled`만 semantic authority로 해시하고, Niagara/AttachMode/Socket은 enabled 상태에서만 fingerprint에 포함하도록 교정했다. 비활성 필드를 강제로 저장시키거나 별도 migration/gate를 만들지 않았다.
- final Official UE 5.8 Build `b0186d42dc374c6abc881f82e5434faa` PASS.
- affected Weapon Guide regression `6bf7fcc859194b17aa07fd517a02b715` exact8/8 PASS.
- affected CCAS-P0-06 Planning/Review regression `00c73e876c874820a00867a17335cbf3` exact7/7 PASS.
- final CCAS-P0-07 / Production Bridge focused `0b7fb435440e4657bb87f194b06269ef` exact25/25 PASS, failure/missing/unexpected/duplicate exact0, result JSON sha256 `d51bdceb4422ff7f0f41a8d1101662cb2fd0132858e47686f88216b62ac6e689`.
- disabled FX canonicalization으로 typed execution manifest가 fresh 재계산되어 Current corrected Review identity는 `ReviewPackageFingerprint=sha256:9af9a9256802244e7598ca6c92c916d1f9e11335599204cd518a094b552b5940`, `ExpectedPostSemanticHash=sha256:049752116ea69850ffe245ce26365756bd15cfb1e332fc202339720c0ced42e3`, `ResourceCatalogFingerprint=sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7`, `ExecutionBindingFingerprint=sha256:1e87904bfe4e65f5614c8744af50dc0cbf533f14c64b13748178e08e6cb6243a`다.
- v0.1.29의 `0a93... / 69d5...` identity는 affected regression 교정 전 intermediate correction evidence이며 USER Cutover Acceptance에 사용하지 않는다.
- final re-review verdict는 P0 exact0 / blocking P1 exact0 / correction-local P2 exact0이다. actual Production exact34 DataAsset generation, canonical Workbook promotion, Publication Catalog mutation, managed authority cutover, stage/commit/push는 exact0을 유지한다.

### v0.1.29 - 2026-10-07

- Pre-Cutover Mid-review blocking P1 exact3을 같은 semantic correction unit으로 교정하고 fresh implementation re-review PASS까지 완료했다.
- `FCFContentReviewPackage`에 Production optional `ExecutionBindingFingerprint`를 추가하고 immutable `ReviewPackageFingerprint` 계산에 포함해 USER approval이 typed execution manifest 자체를 결속하도록 했다.
- `CFProdBatchPrep`은 같은 exact8 Product spec + Resource Catalog에서 Review Workbook과 exact8 `FCFProdPreparedProduct`를 함께 생성하고, existing Equipment/Weapon durable backend가 계산한 unique exact34 desired fingerprint를 `CarFight.CCAS.ProductionExecutionBinding/v1`으로 해시한다.
- `ValidatePreparedBatchApproval`을 추가해 USER approval, current ReviewPackage fresh rehash, execution manifest fresh rehash, exact8 request의 review/base/resource/execution/workbook identity를 durable mutation 전에 mutation0 재검증한다.
- focused CandidatePreparation은 prepared exact8 drafts를 `CFProdTargetBindings` concrete backend에 unique exact34 전부 등록해 approved manifest의 target desired fingerprint가 실제 existing typed backend contract와 exact 일치함을 증명한다.
- `FCFProdProvisionRequest` / `FCFProdTransaction`에 ReviewPackage/BaseSnapshot/ResourceCatalog/ExecutionBinding fingerprint를 durable 결속하고 transaction schema를 `ccas-production-transaction/v3`로 승격했다.
- `bRequireAbsentAtReview`를 target/evidence에 추가해 review 당시 신규였던 CreateNew/CreateNewShared target이 다른 persisted payload로 선점되면 preflight/TOCTOU에서 fail-closed하도록 했다.
- P1-1 영향 범위로 Weapon Guide durable payload semantic coverage를 보강했다: Turret fire-while-aligning/muzzle clearance, Weapon reload/fire-FX/Launcher policy, Projectile interception/Trail/Thruster/Impact FX/launch-axis/guidance가 fingerprint/materialize/readback에 포함된다.
- runtime exact persisted contract가 없는 synthetic `TargetUsePolicy` Production projection을 제거했다.
- CF-FQ-056 PFP-P0-03 USER ACCEPTED / COMPLETE를 Current truth로 반영해 `GuidedMissile_Standard`를 Wave1B / AuthoringReady / stale PFP dependency exact0으로 교정했다. WeaponContentRosterPlan도 v0.1.9로 동기화했다.
- P2 dual-authority 위험은 새 runtime gate를 추가하지 않고 `CFProdBatchPrep = managed cutover 전 bootstrap/review-only`, `cutover 후 canonical Workbook = sole Current authoring authority` 계약으로 닫았다. active `PrepareFirstWaveCandidate` authority caller는 exact0이다.
- corrected fresh Review identity: `ReviewPackageFingerprint=sha256:0a93f155ceaeae48fda6a3b8beee6dae6adfda046505c7058caab2f0341fb226`, `ExpectedPostSemanticHash=sha256:049752116ea69850ffe245ce26365756bd15cfb1e332fc202339720c0ced42e3`, `ResourceCatalogFingerprint=sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7`, `ExecutionBindingFingerprint=sha256:69d522f06e2b1571ff928121a6bddc51a6674acc1def9326777a5dfb26545432`.
- final Official UE 5.8 Build `23f1c7f6df384898ade274b7199fcd9e` PASS. final focused exact25 process `2375a387bbbf4735b4ee504a89228f99` 25/25 PASS, failure/missing/unexpected/duplicate exact0, result JSON sha256 `d51bdceb4422ff7f0f41a8d1101662cb2fd0132858e47686f88216b62ac6e689`.
- re-review verdict는 P0 exact0 / blocking P1 exact0 / correction-local P2 exact0이다. actual Production exact34 DataAsset generation, canonical Workbook promotion, Publication Catalog mutation, managed authority cutover, stage/commit/push는 exact0을 유지한다.

### v0.1.28 - 2026-10-07

- v0.1.27 `FRESH PRODUCTION REVIEW PACKAGE READY`를 USER Cutover Acceptance 직전 fresh 중간검수했다. 기존 exact25 PASS와 official build PASS를 보존하되 테스트 통과 여부와 승인→실행 semantic binding을 분리해 재검토했다.
- P0 exact0 / blocking P1 exact3 / P2 exact1로 판정하고 `PRE-CUTOVER MID-REVIEW CORRECTION REQUIRED / USER CUTOVER ACCEPTANCE HOLD`로 전환했다.
- P1-1: `FCFProdBatchPrepResult` / approved `FCFContentReviewPackage`에서 exact8 `FCFProdProvisionRequest`를 생성하는 production execution-manifest builder가 없고 ProvisionRequest에 ReviewPackage/BaseSnapshot/ResourceCatalog fingerprint binding이 없어 reviewed payload와 executed typed payload의 exact equality를 현재 증명할 수 없음을 확인했다.
- P1-2: fresh transaction의 `CreateNew` / `CreateNewShared` target이 review 이후 unexpected existing target으로 바뀌어도 current `ExecuteProductTransaction`이 collision/stale로 fail-closed하지 않고 update 경로로 들어갈 수 있음을 확인했다.
- P1-3: canonical CF-FQ-056 UDS head는 PFP-P0-03 USER ACCEPTED / COMPLETE / Closed이고 Guided Missile guidance USER PASS인데, `CFProdBatchPrep`와 current Plan/Roster는 여전히 `GuidedMissile_Standard = Conditional / PFP.P0.03.UserTrajectory Pending`을 생성하고 있음을 확인했다. 이는 Review Package semantic payload를 바꾸므로 v0.1.27 fingerprint는 correction 뒤 재사용할 수 없다.
- P2-1: hardcoded First Wave bootstrap seed는 최초 Workbook 생성 전까지만 허용하고 authority cutover 이후 canonical Workbook과 병렬 authoring authority가 되지 않도록 test/bootstrap-only로 한정할 필요가 있다.
- v0.1.27 ReviewPackageFingerprint `sha256:573b9ce39663d8731bd411a6186dde3ffa99e1b47f581b73cf25732f4fffabaa` 및 ExpectedPostSemanticHash `sha256:f9642cd78d9acc5d073b586b647e83b74e786b69e3141e24a0828dfe4b49b172`는 historical pre-correction evidence로 보존하고 USER Acceptance 대상으로 사용하지 않는다.
- 이번 중간검수에서는 Product code correction, actual Production asset 생성, canonical Workbook promotion, Publication Catalog mutation, authority cutover, stage/commit/push를 수행하지 않았다.

### v0.1.27 - 2026-10-06

- `CFProdBatchPrep` read-only candidate preparation 계층을 추가해 Current Production Roster exact8을 transient canonical Workbook candidate로 구성하고 Product exact8 / Role Binding exact48 / unique Production DA target exact34 / explicit sortie ammo row exact8을 immutable Review Package로 고정했다.
- candidate preparation은 기존 Generic Compiler / Planning / Resource Catalog 계약을 재사용하며 persistent Workbook write, DataAsset mutation, Publication Catalog mutation, Authority Cutover를 호출하지 않는다. 실제 durable mutation authority는 기존 `CFProdProvisioning + CFProdTargetBindings + Weapon/Equipment typed backend`에 그대로 남긴다.
- 기존 `HeavyCannon` / `RocketLauncher` Product identity 또는 prototype DA를 Production anchor로 재사용하지 않았다. 대신 fresh persisted AssetDump로 검증한 mesh / projectile actor / FX exact12만 stable ResourceId로 등록해 신규 Production candidate가 재사용하도록 했다.
- reusable Resource exact12는 Cannon/Rocket turret Base/Yaw/Pitch mesh exact5, common Projectile Blueprint exact1, Cannon/Rocket projectile mesh exact2, weapon fire FX exact1, projectile trail exact1, Rocket thruster exact1, impact FX exact1이다.
- persisted turret mesh socket capability를 fresh scan해 `YawPivot`, `PitchPivot`, Cannon `Muzzle`, Rocket `Muzzle_1 / Muzzle_4 / Muzzle_2 / Muzzle_3` exact8을 직접 확인했다. Rocket muzzle order는 existing persisted authoring 순서를 OrderedList semantic으로 보존했다.
- persisted current Turret values를 Production candidate seed로 재결속했다: yaw/pitch rate `35/20 deg/s`, pitch `-8..25 deg`, muzzle clearance `1.5 m`, Cannon mount `200 kg`, Rocket mount `150 kg`, `AllowFireWhileAligning=true`; Rocket는 `RequireAllMuzzles=true`를 유지한다.
- current explicit sortie ammo baseline을 `40 / 20 / 40 / 120 / 32 / 32 / 32 / 10`으로 focused Automation에 고정해 stale historical `12 / 18 / 18 / 6` 값이 다시 actual Production input으로 섞이지 않도록 했다.
- candidate resource validation은 Required Resource unresolved exact0, registered persisted Resource exact12, required socket capability proof exact8일 때만 `ReviewReady`를 반환하도록 fail-closed했다. Ammo icon 및 non-propelled Projectile thruster는 gameplay apply optional resource로 유지한다.
- fresh Production Review Package는 `ReviewPackageFingerprint=sha256:573b9ce39663d8731bd411a6186dde3ffa99e1b47f581b73cf25732f4fffabaa`, `ExpectedPostSemanticHash=sha256:f9642cd78d9acc5d073b586b647e83b74e786b69e3141e24a0828dfe4b49b172`, `ResourceCatalogFingerprint=sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7`로 고정했다.
- resource-ready 교정 중 `UNiagaraSystem::StaticClass()` direct link에 필요한 Editor-only Niagara module 누락으로 Build `0c63102d600f44f880e5b60111a344b4`가 LNK2019 exact1로 실패했다. 직접 영향 범위인 `CarFight_ReEditor.Build.cs v1.10.4`에 `Niagara` private dependency만 추가하고 재빌드했다.
- final Official UE 5.8 Build `d43ec790d823441f85d9aff962e2486f` PASS. Build.cs 변경으로 UHT + Editor module dependency graph가 fresh 재생성되어 UHT 175.99 s / 88 actions를 수행했고 최종 Editor DLL link까지 PASS했다.
- `Tools/RunCCASP007Tests.ps1 v1.9.0` focused exact25는 process `8d0aa91f6acf43a4aaebb9abe67c135d`에서 25/25 PASS, failure/missing/unexpected/duplicate terminal exact0이다. result JSON SHA-256은 `d51bdceb4422ff7f0f41a8d1101662cb2fd0132858e47686f88216b62ac6e689`이다.
- `CarFight.ContentAuthoring.P007.ProductionBatch.CandidatePreparation`은 ReviewReady, exact counts, canonical Production paths, shared single-writer modes, current sortie ammo, persisted resource IDs, current turret values/muzzle order, no-write/no-cutover invariant를 함께 검증한다.
- canonical `Authoring/Content/CarFight_Content.xlsx` 생성/승격 exact0, actual Production exact34 DataAsset durable generation exact0, Production Publication Catalog mutation exact0, managed Authority Cutover exact0을 유지한다. 현재 상태는 `EXACT34 CANDIDATE BATCH PREPARATION TECHNICAL PASS / FRESH PRODUCTION REVIEW PACKAGE READY / USER CUTOVER ACCEPTANCE PENDING`이며 P0-08은 열지 않는다.

### v0.1.26 - 2026-10-06

- frozen `Typed Production Provisioning + Product Recovery + Production Publication Bridge`를 실제 C++/Config로 구현하고 fresh implementation re-review를 완료했다.
- Product-level durable transaction/recovery를 `Prepared / VisibilityWithdrawn / ChildAssetsPersisted / ProductGraphVerified / RuntimeVerified / Published / VehicleReady / BlockedBeforeMutation / RecoveryRequired` 상태로 구현하고 transaction schema v2에 Product/target object path/fingerprint/impacted Product closure evidence를 durable 저장하도록 했다.
- F.1 Shared Role Single-Writer를 `CreateNew / CreateNewShared / BindShared` binding mode로 구현하고 shared flag/mode exact consistency, absent shared creator exact1, persisted shared creator exact0 허용, consumer desired fingerprint exact 일치를 mutation 전에 검증한다.
- F.2 Published Product Mutation Impact Closure를 구현해 impacted published Product 전체 key set을 먼저 durable journal에 고정하고 Catalog visibility를 전체 선철회한 뒤에만 child/Product mutation을 허용한다. impacted Product는 각각 persisted graph/runtime proof를 다시 통과한 경우에만 재게시한다.
- provider-owned `EquipmentPresetAmmoLoads(ProductContentId, AmmoRoleContentId, DefaultSortieAmmoCount)` child schema/projection과 persisted AmmoData resolve, `InitialSortieAmmoLoads` merge를 구현했다. `MaximumLoadableAmmoCount`를 current sortie count로 추론하는 경로는 추가하지 않았다.
- generated read-only `UCFProdEquipCatalogData`와 stable PrimaryAsset identity `CFProdEquipCatalog:Production`, canonical object path `/Game/CarFight/Weapons/Data/Production/DA_ProdEquipCatalog`를 구현했다. AssetManager는 해당 Catalog class의 resolve/cook transport만 담당하며 Product membership은 `PublishedEquipment` exact projection이 소유한다.
- pre-publication Runtime proof는 기존 `FCFRuntimeEquipApplyService` 경계를 재사용하고 별도 Runtime Apply subsystem을 만들지 않았다.
- `CFProdTargetBindings` concrete adapter를 추가해 reviewed Production target을 기존 Weapon Guide의 Turret/Projectile/Weapon `CFDADurableCore`, 기존 Damage/Ammo reviewed provider, 기존 Equipment durable backend에 exact fingerprint로 결속했다. 새 generic writer는 추가하지 않았다.
- Weapon Guide 내부 typed durable 구현에는 Editor-private `CFWeaponGuideBackend` seam만 노출해 existing payload/fingerprint/materialize/extract/provider authority를 재사용하도록 했다.
- implementation re-review 결과 신규 `RuntimeTestCatalog`/Inventory publication authority exact0, Production folder membership scan exact0, 신규 Runtime Apply subsystem exact0, 신규 generic graph authority exact0, direct Test/Legacy Production anchor exact0을 확인했다.
- Official UE 5.8 Build `d034a811b1d348fe8ff5101c72a9d4b6` PASS. focused `Tools/RunCCASP007Tests.ps1 v1.8.0`은 exact24/24 PASS이며 `ConcreteBindings`, `SharedClosure`, `RecoveryResume`, `TransactionFileStore`, ammo projection, existing Runtime Apply regression을 포함한다.
- 첫 exact24 실행에서 신규 `ConcreteBindings` fixture의 `BaseDamage=0`이 기존 Damage provider의 positive BaseDamage 계약을 위반해 exact1 실패했으며, 제품 코드 변경 없이 fixture를 `BaseDamage=100.0`으로 교정한 뒤 fresh Build/Automation PASS로 닫았다.
- active Production caller wiring, actual exact8 Production DA batch 생성, canonical Workbook promotion, managed authority cutover는 exact0으로 유지했다. 현재 상태는 `PRODUCTION PROVISIONING BRIDGE TECHNICAL PASS / IMPLEMENTATION RE-REVIEW PASS / USER CUTOVER ACCEPTANCE PENDING`이며 P0-08은 열지 않는다.

### v0.1.25 - 2026-10-06

- v0.1.24 re-review에서 직접 연결된 omission exact2를 추가 교정했다: finite ammo sortie input과 shared-target published impact closure.
- finite weapon Product의 Workbook에 Weapon Provider-owned row형 `EquipmentPresetAmmoLoads(ProductContentId, AmmoRoleContentId, DefaultSortieAmmoCount)`를 추가하고 Catalog에는 resolved `DefaultSortieAmmoLoads[]`로 projection하도록 고정했다. `MaximumLoadableAmmoCount`를 현재 수량으로 추론하지 않는다.
- Runtime Apply는 새 서비스가 아니라 기존 `FCFRuntimeEquipApplyService`를 request/overload 형태로 최소 확장해 EquipmentPreset + resolved sortie ammo loads를 기존 Fitting authority에 전달하도록 설계했다.
- candidate ammo merge는 기존 Fitting의 다른 AmmoId를 보존하고, 같은 AmmoId의 mounted finite weapon 초기 장전 합계를 만족하며, 더 이상 사용되지 않는 AmmoId만 제거한 뒤 `BuildFittingSnapshot`의 기존 검증을 단일 authority로 재사용한다.
- shared target Update는 기존 CCAS impact analysis로 published consumer closure를 계산하고, mutation 전에 impacted Catalog membership을 durable withdraw한 뒤 모두 재검증하도록 추가했다. stale published Product를 노출한 채 shared child를 바꾸는 경로를 금지했다.
- ProductGraphFingerprint를 transitive Role target fingerprints까지 포함하도록 강화하고 Publication Catalog entry에 이를 저장해 shared child drift가 consumer Product stale 판정으로 전파되게 했다.
- initial absent shared target과 already-persisted managed shared target을 구분해 `CreateNewShared` exact1 요구를 초기 생성에만 적용하고 existing Update/NoChange에는 creator exact0을 허용했다.
- VehicleReady criteria를 exact14로 보강했다.
- Fresh correction re-review 결과: P0 exact0 / blocking P1 exact0 / correction-local P2 exact0. 기존 P0-07의 non-blocking design P2 exact2(Workbook visual/protection production compatibility matrix, multi-proposal concurrent merge/rebase UX)는 그대로 비차단 상태로 유지한다.
- exact8 final ResourceId/semantic binding 값은 실제 batch authoring 준비 입력으로 남아 있으나 Provisioning/Publication bridge 설계 blocker가 아니다.
- 실제 C++/Config/Asset 구현, exact34 생성, Workbook promotion, managed authority cutover는 수행하지 않았다.

### v0.1.24 - 2026-10-06

- Production provisioning 설계검수에서 확인된 blocking P1 exact4를 교정했다.
- DA exact1 durable write는 기존 `CFDADurableCore`에 유지하고, multi-DA Product graph에는 `Prepared → ChildAssetsPersisted → ProductGraphVerified → RuntimeVerified → Published → VehicleReady` Product-level transaction/recovery 계약을 추가했다.
- partial durable creation은 `RecoveryRequired`로 분류하고 Resume 시 persisted fingerprint가 동일 transaction desired fingerprint와 exact 일치할 때만 idempotent skip하도록 고정했다. 자동 delete/rollback/rebase는 추가하지 않았다.
- shared Production DA는 `ContentTypeId + ContentId` canonical authored row exact1 / `CreateNewShared` owner exact1 / `BindShared` consumer 1..N의 single-writer 계약으로 교정하고 consumer-local payload override를 금지했다.
- Production discovery authority를 RuntimeTestCatalog 또는 Inventory에 두지 않고 CCAS generated read-only `UCFProdEquipCatalogData` publication catalog로 고정했다. AssetManager는 catalog resolve/cook transport이며 Production 폴더 전체 scan을 publish membership으로 취급하지 않는다.
- pre-publication Runtime proof는 기존 `FCFRuntimeEquipApplyService::ApplyEquipmentRuntime` direct seam을 사용하고, runtime/weapon/ammo/projectile 기술검증 PASS 뒤 generated Catalog를 durable update하는 것을 마지막 visibility mutation으로 고정했다.
- `VehicleReady` exact13 순서를 persisted graph → direct runtime proof → controlled publication/requery 순으로 교정해 목록에 보이는 Product는 기술적으로 차량 적용 가능한 상태라는 계약을 만들었다.
- stale Current projection의 Matrix USER Review Pending / Roster v0.1.4 / naming freeze pending 표현을 current AI Technical baseline / Roster v0.1.7 / frozen naming으로 정렬했다.
- 문서 교정 직후 re-review 전 상태이므로 실제 C++/Config/Asset 구현, exact34 생성, Workbook promotion, authority cutover는 수행하지 않았다.

### v0.1.23 - 2026-10-06

- `WeaponContentRosterPlan.md v0.1.6`의 exact8 initial Production balance와 canonical identity/naming을 representative Plan에 연결했다.
- Workbook canonical sheet에 generic row-oriented `RoleBindings`를 추가하고 Weapon Production provider sheets를 `Weapons / WeaponFamilies / EquipmentPresets / WeaponDefinitions / Projectiles / Ammo / Damage / TurretMounts`로 freeze했다.
- Workbook만으로 VehicleReady Product를 만들기 위해 각 sheet의 required semantic field를 current UCFEquipmentPresetData/UCFWeaponData/UCFProjectileData/UCFDamageData/UCFAmmoData/UCFTurretMountData 및 launcher/missile config 계약까지 확장해 명시했다.
- UObject ObjectPath 수동 입력 대신 Role binding과 ResourceId/Semantic Binding으로 EquipmentPreset child reference, Mesh/FX/socket 등을 resolve하도록 고정했다.
- USER-facing Workbook unit를 m, m/s, m/s², kg, deg, s, RPM으로 고정하고 Provider가 Runtime cm 계열 단위로 deterministic convert하도록 했다.
- Required Resource/socket semantic 미해결은 Provisioning 전에 fail-closed하도록 했다.
- Weapon canonical identity를 `CCAS Weapon ContentId == WeaponId == EquipmentId` exact8로 freeze하고 기존 Catalog의 `weapon_*` 임시 projection을 canonical identity로 사용하지 않도록 했다.
- Generated Production asset root를 `/Game/CarFight/Weapons/Data/Production/`으로 freeze해 Test/Legacy graph와 분리했다.
- next implementation을 frozen Authoring Schema → Typed Production Provisioning → Production discovery/apply → exact8 batch authoring 순서로 확정했다.

### v0.1.22 - 2026-10-06

- USER 전제조건을 `End-to-End Vehicle-Ready Contract`로 승격했다. Workbook에서 생성된 Product가 실제 차량에 바로 적용되지 않으면 CCAS Production 완료로 인정하지 않는다.
- canonical flow를 Workbook → Canonical Model → Typed Production Provisioning → Production DA graph → persisted readback → Production discovery/publication → Vehicle Runtime Apply로 고정했다.
- Workbook을 지원 Product의 유일한 user-authored batch source로 정의하고, ResourceId 기반 resource binding은 허용하되 UE Editor에서 개별 DA 수동 교정을 정상 workflow에서 제외했다.
- 현재 P0-07 Cutover의 existing-Product-only 계약을 보존하면서 신규 exact8 생성을 담당할 앞단 `Typed Production Provisioning` 필요성을 명시했다.
- Production Provisioning은 기존 Weapon Authoring / Equipment Builder / CFDADurableCore를 재사용하며 중복 writer를 만들지 않도록 했다.
- 현재 RuntimeTestCatalog의 수동 AllowedEquipmentPresetData 등록은 Production 정상 경로로 금지하고, authoring 완료 시 Production discovery/apply source까지 자동 publish하도록 완료 조건을 강화했다.
- Product 단위 `VehicleReady` exact13 acceptance criteria와 incomplete Product publication 금지 경계를 추가했다.
- exact8 전체에 Workbook → persisted DA graph → discovery → Vehicle Runtime Apply technical evidence를 요구하도록 First Production Wave proof를 강화했다.
- exact8 Matrix의 상세 공유/분리는 AI Technical responsibility로 전환하고 USER가 exact34 전체를 개별 검수하는 Gate를 제거했다. Matrix는 End-to-End Vehicle-Ready Contract에 종속된다.

### v0.1.21 - 2026-10-06

- Flexible Data Role Composition을 Production exact8에 적용한 첫 `Data Role Binding Matrix` candidate를 representative Plan에 반영했다.
- current candidate 규모를 exact48 Role binding / unique 신규 Production DA exact34 / direct Test-Legacy reuse exact0으로 기록했다.
- unique DA를 EquipmentPreset exact8, Mount exact2, WeaponDefinition exact8, Projectile exact6, Damage exact5, Ammo exact5로 산출했다.
- Cannon은 Production Mount exact1 공유, Projectile exact4 분리, Damage/Ammo Standard+LongRange 공유 및 Heavy/Rapid 분리로 정리했다.
- Rocket은 Production Mount/Projectile/Damage/Ammo exact1씩 공유하고 Variant 차이를 WeaponData Launcher pattern에서 소유하도록 정리했다.
- Guided Missile Standard는 Rocket Pod Mount만 initial 공유하고 Weapon/Projectile/Damage/Ammo는 독립하도록 했다.
- 기존 HeavyCannon/RocketLauncher 및 prototype child DA direct reuse를 금지하고 구조/reference evidence로만 유지했다.
- current runtime의 승인 발사 기반 Muzzle 순환 및 SingleCycle exact1 계약을 Rocket shared Mount의 기술 근거로 기록했다.
- Matrix는 USER Review Pending이며 실제 Product mutation / Workbook promotion / authority cutover 승인이 아님을 유지했다.
- next exact gate를 USER Matrix Review → Production balance baseline → canonical naming freeze → batch authoring preparation으로 전진시켰다.

### v0.1.20 - 2026-10-06

- USER 결정으로 Production DataAsset composition을 고정 exact6 schema가 아닌 extensible `Data Role Binding` 집합으로 다루는 Current 설계를 추가했다.
- 현재 `EquipmentPreset / TurretMount / Weapon / Projectile / Damage / Ammo` exact6은 First Wave 초기 Role projection일 뿐 영구 고정 타입 목록이 아님을 명시했다.
- Role 계약에 `RoleId`, ContentType/AssetClass, Required/Optional/Conditional, Single/Multiple, Shared/VariantOwned, ParentRole, ActivationCondition, AuthoringProvider, binding target, management/lifecycle 의미를 고정했다.
- Generic Core가 Projectile/Damage/Ammo/Seeker 같은 도메인별 Role branch를 소유하지 않고 typed Weapon Provider가 의미·생성·검증을 소유하도록 확장 경계를 명시했다.
- 새 DA 추가는 Role descriptor + Product binding + Provider 연결로, DA 제외는 reviewed lifecycle/binding mutation으로 처리하며 물리 `.uasset` 삭제와 분리했다.
- Workbook composition은 고정 DA 종류 column보다 row-oriented Role Binding을 canonical extension contract로 사용하고, 고정 column/panel은 USER-facing projection으로만 허용하도록 했다.
- 기존 Weapon Guide durable authoring 및 Equipment Builder backend는 폐기하지 않고 Role-driven Production orchestration의 실행 backend로 재사용하도록 했다.
- exact8 실제 제작 전에 Production Role Binding Matrix를 확정하는 단계를 추가했다.

### v0.1.19 - 2026-10-03

- USER 결정으로 First Production Wave exact8의 실제 Product 전략을 `기존 anchor exact2 + 신규 exact6`에서 `신규 Production Product exact8`로 교정했다.
- 기존 `HeavyCannon` / `RocketLauncher`는 Production anchor에서 제외하고 Test/Legacy reference로 유지하도록 고정했다.
- 기존 prototype/test 자산의 구조와 dependency는 참고할 수 있으나 prototype balance, debug state, Product identity는 Production 기준으로 승계하지 않도록 했다.
- `Cannon_Standard`, `Cannon_Heavy`, `Cannon_LongRange`, `Cannon_Rapid`, `Rocket_Standard`, `Rocket_Salvo`, `Rocket_Ripple`, `GuidedMissile_Standard` exact8을 신규 Production identity 대상으로 명시했다.
- Wave 1A exact7과 Guided Missile Wave 1B exact1을 제작 준비에서 병렬화하되 Guided Missile Product Acceptance의 PFP-P0-03 USER trajectory gate는 유지했다.
- P0-06 disposable Review Package와 기존 anchor 기반 PASS evidence는 historical evidence로 보존하고 actual Production Cutover input만 Current 정책으로 대체했다.
- 다음 준비 범위를 신규 exact8 Product authoring + canonical ContentKey + Production Weapon `ICFContentProvider` + 최초 canonical Workbook + fresh Production Review Package로 갱신했다.
- 실제 Product mutation / Workbook promotion / authority cutover는 fresh Production Review Package에 대한 별도 USER 승인 전까지 열지 않았다.

### v0.1.18 - 2026-10-03

- P0-07 fresh 중간검수에서 P0 exact0 / blocking P1 exact5 / correction-local P2 exact2를 확인하고 authority cutover 전 correctness correction을 수행했다.
- actual Manifest payload fresh rehash를 추가해 approved/request/fresh ManifestFingerprint exact3 binding으로 강화했다.
- P0-06 `ValidateApproval`이 current ReviewPackage payload fingerprint를 fresh 재계산하도록 교정하고 read-only `ComputeReviewPackageFingerprint` helper를 추가했다. ApprovalBinding exact7 regression 안에서 payload tamper fail-closed를 검증했다.
- canonical/staged/provenance execution path를 frozen production exact3에 결속하고 manifest management/lifecycle/product-mutation intent를 fresh truth와 재대조한다.
- transaction/provenance durable write를 sibling temp + exact readback + replace로 교정하고 failure journal 저장 실패를 `CRITICAL` fail-visible로 승격했다. deserialize enum range와 filename/payload TransactionId cross-check도 추가했다.
- canonical Workbook promotion을 staged raw temp/readback/replace 방식으로 교정하고 promotion 직전 staged expected semantic hash, canonical base semantic hash, Office lock을 다시 검증한다.
- `ResumeRecovery`를 추가하되 Product reapply/auto rollback/rebase는 exact0으로 유지한다. 모든 target Product가 reviewed desired exact일 때만 동일 transaction의 exact staged candidate finalize를 허용한다.
- final Official UE 5.8 Build `efef4d82be0e4e989d1fad789faa8333` PASS, P0-06 final regression `3898d7ea945644a2aedaa7f55074a200` exact7/7 PASS, P0-07 final focused `fab5fe3c03c8476285ab8d60ee42a1b4` exact16/16 PASS다.
- Fresh correction re-review는 P0 exact0 / blocking P1 exact0 / correction-local P2 exact0으로 수렴했다. 기존 design P2 exact2 non-blocking은 유지한다.
- actual production Product mutation, canonical Workbook promotion, Workbook Current authority activation, Current Systems authority promotion은 여전히 수행하지 않았다. USER Cutover Acceptance가 다음 Gate이며 P0-08은 열지 않았다.

### v0.1.17 - 2026-10-02

- CCAS-P0-07 frozen v0.1.16 계약을 실제 source로 구현하고 Final Technical Review를 완료했다.
- `CFContentCutover`에 explicit managed Manifest, approval-bound Product identity, fresh stale/drift guard, `Prepared → ProductApplied → WorkbookCommitted → Verified` transaction, `BlockedBeforeMutation / RecoveryRequired`, provenance sidecar와 production FileStore를 구현했다.
- `FCFProviderCutoverAdapter`로 기존 `ICFContentProvider` reviewed typed apply/readback seam을 재사용하고 신규 Product Create는 별도 writer를 만들지 않도록 fail-closed했다.
- repository-vendored `OpenXLSX 0.5.1 + miniz 3.0.2 + pugixml 1.15` 기반 concrete Editor-only `.xlsx` adapter를 구현했다. Runtime dependency와 build-time network fetch는 exact0이다.
- concrete `.xlsx` base create/read, sibling staged-write, reopen exact2, semantic hash determinism, base no-overwrite, transaction/provenance JSON durable roundtrip을 focused Automation으로 검증했다.
- P0-07 final focused Automation `8aa7f4126a3e441f95a38d0e70243447` exact10/10 PASS, affected P0-01~06 regression은 각각 8/8, 12/12, 8/8, 5/5, 5/5, 7/7 PASS다.
- Final Technical Review는 P0 exact0 / blocking P1 exact0 / P2 exact2 non-blocking / `CCAS-P0-07 TECHNICAL IMPLEMENTATION PASS`로 판정했다.
- actual production Product mutation, canonical Workbook promotion, Workbook Current authority activation, Current Systems authority promotion은 수행하지 않았다. 별도 USER Cutover Acceptance가 다음 Gate이며 P0-08은 열지 않았다.

### v0.1.16 - 2026-10-02

- USER의 `진행해` 명시 지시로 별도 explicit gate였던 CCAS-P0-07 Authority Cutover / Migration의 설계·구현 진입을 열었다.
- fresh rebaseline 결과 기존 P0-01~06 Core/Compiler/Catalog/Planning/Provider 및 typed durable apply backend는 재사용 가능하고 방향 폐기 사유 P0 exact0으로 판정했다.
- authority cutover 전에 필요한 blocking P1 exact10 — explicit managed onboarding, approval→apply fingerprint binding, fresh stale guard, concrete Workbook adapter, transaction/recovery, provenance snapshot, direct edit drift, no-auto-delete, JSON Staging coexistence, Systems promotion timing — 을 발견해 Plan 계약에 교정·동결했다.
- 실제 persistent `.xlsx` concrete adapter가 현재 exact0이고 테스트/memory adapter만 존재함을 확인해 P0-07 implementation 필수 항목으로 승격했다. P0-01~06에서의 OpenXLSX P2 비차단 판정은 P0-07 실제 cutover에는 그대로 적용하지 않는다.
- Cutover transaction을 `Prepared → ProductApplied → WorkbookCommitted → Verified`, 실패를 `BlockedBeforeMutation / RecoveryRequired`로 동결하고 partial Product failure 시 Workbook authority promotion 금지, auto rollback/rebase/retry 금지를 명시했다.
- Generated provenance는 `CarFight_Content.cfsnapshot.json` sidecar가 소유하고 gameplay DataAsset에 CCAS metadata를 강제 주입하지 않도록 확정했다.
- CCAS-managed Product direct edit는 Drifted로 fail-visible하며 Workbook 재적용 또는 Product→Workbook Change Proposal exact2 reviewed recovery만 허용하고 자동 역동기화를 금지했다.
- Retired와 physical delete를 분리하고 P0-07 normal path의 asset delete/rename/move exact0을 유지했다.
- CCAS-managed 범위에서 JSON Staging은 내부 provider transport로 재사용 가능하지만 USER-facing 병렬 authority는 금지하고, unmanaged 범위는 legacy workflow를 유지하도록 확정했다.
- Fresh Design Re-review 결과 P0 exact0 / blocking P1 exact0 / P2 exact2 non-blocking / `CCAS-P0-07 DESIGN PASS / IMPLEMENTATION READY`로 전진했다. 실제 authority cutover와 Product mutation은 아직 적용하지 않았다.

### v0.1.15 - 2026-10-02

- USER가 `P006.UserReview.WeaponWave1`의 exact Review Package `sha256:45cc3e77c7bd5b7e7e676ee09668eb82c3e834e212c6336104c7b4d8b3db4ec2`를 명시적으로 채택해 `CCAS-P0-06 USER PASS / COMPLETE`로 전진했다.
- 채택 범위는 기존 `HeavyCannon` / `RocketLauncher` Product identity anchor 유지, 신규 Product 후보 exact6, Rationale / Design Intent를 포함한 AI Change Proposal → Review Package 구조, `GuidedMissile_Standard` Wave1B Conditional 유지 exact4다.
- 기존 Technical evidence — Final UE 5.8 Build `08bea65054cc493baf2f6c4623eda1a8` PASS, representative USER review Automation `1a3aa4287a1946e6a3febcc2c82b99ae` exact1/1 PASS, P0-06 technical regression `d4a6ad6964d941fdb97db581d494c98d` exact7/7 PASS — 를 그대로 보존한다.
- Persistent Workbook write/reopen, direct DataAsset write, `Provider.ApplyReviewed`, Product Apply/Save, `SavePackage` / `MarkPackageDirty`, physical asset rename/move/delete는 열지 않았다.
- P0-07 Authority Cutover / Migration은 자동 진입하지 않으며 별도의 USER explicit approval Gate로 계속 CLOSED 상태를 유지한다.

### v0.1.14 - 2026-09-30

- CCAS-P0-06 Technical PASS를 보존한 채 USER Acceptance용 실제 `AI Change Proposal → Review Package` disposable representative workflow를 추가 검수했다.
- USER Accepted Weapon Roster exact8을 대표 입력으로 사용하고 기존 Product identity `HeavyCannon` / `RocketLauncher`를 각각 Cannon Heavy / Unguided Rocket Salvo anchor로 그대로 보존했다.
- existing anchor exact2는 planning metadata만 변경되고 Product gameplay payload는 `AlreadyCurrent`, 신규 exact6은 `ReadyToReview`로 분리되는 것을 확인했다.
- GuidedMissile Standard의 `Wave1B Conditional + PFP.P0.03.UserTrajectory Pending` dependency를 Review Package에 유지했다.
- final ReviewPackageFingerprint `sha256:45cc3e77c7bd5b7e7e676ee09668eb82c3e834e212c6336104c7b4d8b3db4ec2`, typed operation exact38, rationale exact8, Ready6 / AlreadyCurrent2 / ExternalReadOnly1 / Retire0 / Blocked0을 기록했다.
- Official UE 5.8 Build `08bea65054cc493baf2f6c4623eda1a8` PASS, representative USER review exact1 `1a3aa4287a1946e6a3febcc2c82b99ae` PASS, P0-06 technical regression `d4a6ad6964d941fdb97db581d494c98d` exact7/7 PASS를 기록했다.
- Persistent Workbook write, direct DataAsset write, Provider.ApplyReviewed, Product Apply/Save, SavePackage/MarkPackageDirty, physical asset rename/move/delete는 exact0을 유지했다.
- 상태는 `USER ACCEPTANCE REVIEW PACKAGE READY / USER Decision Pending`이며 USER PASS/COMPLETE는 아직 주장하지 않는다.
- USER가 P0-06을 채택하더라도 P0-07 Authority Cutover는 자동으로 열지 않고 별도 explicit approval Gate로 유지한다.

### v0.1.13 - 2026-09-30

- v0.1.12 Pre-Implementation Design PASS의 frozen CCAS-P0-06 범위를 기존 P0-01~05 Accepted Core/Compiler/Provider를 재설계하지 않고 구현했다.
- whole-Catalog Snapshot/Fingerprint, typed PlanningMetadata, Active/Deprecated/Retired lifecycle, snapshot-aware semantic Change Set preflight, Change Proposal/Review Package, two-layer stale guard를 추가했다.
- Product mutation과 분리된 read-only planning/review layer를 `CFContentPlanning`으로 추가하고 existing Generic Compiler를 authoritative diff/impact/provider validation engine으로 재사용했다.
- fresh implementation review에서 Resource missing/unready state fingerprint omission과 top-level SheetId semantic validation 누락 blocking P1 exact2를 발견해 최소 교정했다.
- 최종 Official UE 5.8 Build `1609b68d669c4152a3566dd1a529e928` PASS, P0-06 focused Automation `51ae0d409291495dae923822c02fc76d` exact7/7 PASS, final affected P0-03 Resource regression `8afac2ea0df344d4919d1cbf81efb8e0` exact8/8 PASS를 기록했다.
- 구현 중 affected P0-01 Core regression exact8/8, P0-02 Compiler regression exact12/12도 PASS했다.
- Final verdict를 `P0 0 / blocking P1 0 / P2 exact3 non-blocking / CCAS-P0-06 TECHNICAL IMPLEMENTATION PASS / Fresh Implementation Review PASS`로 승격했다.
- Workbook/DataAsset Product authoring, Provider.ApplyReviewed Product mutation, Product Apply/Save, physical asset rename/move/delete와 P0-07 cutover는 열지 않았다.

### v0.1.12 - 2026-09-30

- CCAS-P0-06 Design Review blocking P1 exact6을 기존 P0-01~05 Core/Compiler/Provider를 재설계하지 않는 최소 계약으로 교정·동결했다.
- Immutable Catalog Snapshot은 random identity 없이 whole registered Catalog의 canonical SnapshotFingerprint 자체를 identity로 사용하고 Workbook/Product/ExternalReadOnly/Resource/Provider truth를 결속하도록 확정했다.
- Weapon Roster v0.1.1의 Variant/Role/RelativeIntent/ProductionWave/Readiness/TechnologyDependency/SharedDataPolicy/VariantOwnedDataPolicy를 typed PlanningMetadata로 보존하도록 확정했다.
- Content lifecycle을 Active/Deprecated/Retired exact3으로 동결하고 physical row removal, Retire intent와 Product delete를 분리했다.
- 기존 ChangeSet shape validation 위에 snapshot-aware semantic preflight를 두고 duplicate/conflicting bulk target, ExternalReadOnly/non-owned mutation, implicit removal을 fail-closed하도록 확정했다.
- P0 approval scope는 partial approval 없이 Change Set exact1로 고정하고 per-record rationale + Compiler-derived impact + immutable ReviewPackageFingerprint에 USER approval을 결속하도록 확정했다.
- stale guard를 BaseWorkbookSemanticHash + BaseCatalogSnapshotFingerprint two-layer exact check로 확장하고 stale 시 auto rebase/retry 없이 fresh replan하도록 확정했다.
- Fresh Design Re-review 결과 P0 0 / blocking P1 0 / P2 exact3 non-blocking / PASS로 수렴했다.
- CCAS-P0-06 Implementation은 READY지만 USER 지시 전 NOT OPENED이며 Workbook/DataAsset authoring, Product Apply/Save와 P0-07 cutover는 시작하지 않았다.

### v0.1.11 - 2026-09-30

- CCAS-P0-06 AI Change Set + Batch Planning Pre-Implementation Design Review를 current Core/Compiler/Provider 계약과 USER Accepted WeaponContentRosterPlan v0.1.1 기준으로 fresh 수행했다.
- 방향 폐기 사유 P0 exact0을 확인하고 기존 P0-01~05 Accepted/Technical PASS 기반을 재사용하기로 유지했다.
- blocking P1 exact6을 확인했다: immutable Catalog Snapshot, Roster planning metadata fidelity, explicit Retire lifecycle, semantic Change Set/bulk conflict validation, rationale+immutable Review Package approval binding, Workbook 외 planning truth를 포함한 stale snapshot guard.
- no-direct-DA-write 경계는 기존 구조상 PASS로 확인하고 P0-06 acceptance에서 Provider.ApplyReviewed/SavePackage/Product mutation/rename-move-delete exact0을 강제하도록 동결했다.
- P2 exact3으로 AI snapshot bounded projection, batch review grouping UX, future multi-proposal merge seam을 기록했다.
- Verdict를 HOLD로 두고 exact next를 `CCAS-P0-06 Contract Correction + Fresh Design Re-review`로 설정했다.
- Workbook/DataAsset authoring, CCAS mutation implementation, Product Apply/Save와 P0-07 cutover는 시작하지 않았다.

### v0.1.10 - 2026-09-30

- 이미 수행된 `CarFight.ContentManager` USER Workflow 피드백과 `현재 형태로 채택하고 넘어감` 결정을 CCAS-P0-05의 USER PASS/COMPLETE evidence로 반영했다.
- Family/Variant tree와 전체 목록 중복, Wagon/Sedan 비교 정보 부족은 인지된 non-blocking UX follow-up으로 남기고 P0-05 재검수를 요구하지 않도록 교정했다.
- exact next를 `CCAS-P0-06 AI Change Set + Batch Planning Design Review`로 전진시켰다.
- `WeaponContentRosterPlan.md v0.1.1`의 exact3 Family / exact8 First Production Wave를 P0-06 설계 입력으로 유지했다.
- 이번 전진은 설계·검수 진입만 허용하며 Workbook/DataAsset authoring, CCAS mutation implementation, Product Apply/Save, P0-07 cutover는 열지 않았다.

### v0.1.9 - 2026-09-30

- USER Accepted `WeaponContentRosterPlan.md v0.1.1`을 CCAS-P0-06의 accepted planning input으로 등록했다.
- Cannon / Unguided Rocket / Guided Missile exact3 Family와 First Production Wave exact8의 전달 기준을 연결했다.
- 이 연결이 P0-05 USER Workflow Review를 우회하거나 P0-06 구현·Workbook/DataAsset/Product mutation을 여는 승인이 아님을 명시했다.

### v0.1.8 - 2026-09-29

- CCAS-P0-05 Catalog / Compare + USER Workflow의 read-only Presentation layer와 `CarFight.ContentManager` Native Slate 탭을 구현했다.
- persisted Weapon EquipmentPreset + Vehicle Recipe를 P0-04 shared typed Resource/Semantic bridge로 읽어 전체 목록, 검색/종류 filter, Family/Variant group, selected compare, absolute/relative value, value source, dependency/resource view를 제공한다.
- Canonical `FCFContentCompileResult`를 입력받으면 Workbook revision/validation, Family/Variant metadata, dependency와 Diff Preview를 동일 Presentation 경로에 투영하도록 구현했다.
- current repository에 concrete xlsx adapter와 `CarFight_Content.xlsx`가 아직 없는 상태는 기존 OpenXLSX P2 경계로 유지하고 live UI에 `xlsx adapter P2 미연결` unavailable 상태를 fail-visible하게 표시한다.
- fresh review에서 Workbook 상태 ambiguity와 filtered-out stale selection P1 exact2를 교정했다.
- final Official UE 5.8 Build `dfe409d0cb034fa2be5b235e6c67d48d` PASS, focused Automation `648fb1e8f53144849138cdd0de30b8d0` exact5/5 PASS를 기록했다.
- Generic Core diff/Weapon·Vehicle branch/Product Apply·Save/Workbook write/writer activation/P0-07 cutover는 0으로 유지했다.
- Technical verdict는 `P0 0 / blocking P1 0 / P2 exact2 non-blocking / TECHNICAL IMPLEMENTATION PASS`이며, USER visual/information-hierarchy review 전에는 P0-05 COMPLETE 또는 P0-06 착수로 승격하지 않는다.

### v0.1.7 - 2026-09-29

- CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot을 `CFContentPilot.h/.cpp v1.1.0` provider-local read-only bridge로 구현했다.
- CF-FQ-055 WeaponData/TurretMountData와 Vehicle SnapshotBuilder/AssetReader를 실제 호출하고 두 consumer가 exact same Resource Picker/Catalog/Semantic Role/Profile/Override Core instances를 공유하도록 검증했다.
- persisted `DA_ProtoTurretCannon + DA_CannonBody`의 Base/Yaw/Pitch mesh 및 YawPivot/PitchPivot/Muzzle과 `DA_Recipe_TestSUV`의 SUV chassis/wheels/Wheel_Anchor/HP hardpoints를 실제 Resource/Socket binding fixture로 사용했다.
- Fresh review에서 Profile capability fixture-name hardcode P1 exact1을 발견해 typed backend socket intent 사용으로 교정했고 Generic Core consumer branch와 Product mutation 경로는 0으로 재검수했다.
- persisted USkeletalMesh exact3 real scan에서 Bone `27/46/27`, MaterialSlot `1/3/5`, active Socket `0/0/0`을 확인했다. Bone/MaterialSlot evidence는 CLOSED하고 positive active Socket fixture evidence는 non-blocking P2로 유지했다.
- final Official UE 5.8 Source Build `221caceb6bca4e42bfc539d90c033196` PASS, P0-04 focused Automation `124a3563e3de4e0083114b54e34cb1ef` exact5/5 PASS, affected P0-03 Resource Core regression `381912578fdf4e57a572266d289e8131` exact8/8 PASS를 기록했다.
- Post-Implementation Review는 `P0 0 / blocking P1 0 / P2 exact2 non-blocking implementation evidence / TECHNICAL IMPLEMENTATION PASS`로 수렴했다. 잔여 P2는 OpenXLSX actual roundtrip/presentation/protection evidence와 persisted USkeletalMesh positive active Socket fixture evidence다.
- Product Apply/Save/Workbook write/writer activation/generic reflection/P0-07 cutover는 0으로 유지하며 exact next를 `CCAS-P0-05 Catalog / Compare + USER Workflow`로 승격했다.

### v0.1.6 - 2026-09-29

- CCAS-P0-03 Resource Catalog + Semantic Binding을 `CFContentResource.h/.cpp v1.1.0` Editor-only read-only Core로 구현했다.
- Stable ResourceId와 raw Unreal ObjectPath를 분리하고 PickerType, Asset Registry existence/type, Static/Skeletal Mesh Socket/Bone/MaterialSlot capability snapshot을 typed contract로 추가했다.
- Semantic Role, Profile + Override, `Override > Profile`, Required/Optional failure policy와 Canonical Workbook ResourceReference -> Catalog validation을 추가했다.
- Fresh review에서 Picker AllowedClass resolve fail-closed와 unbound binding source `None` semantics exact2를 교정했다.
- final Official UE 5.8 Source Build `4c868de58f904bed9b5cf96b8f5af079` PASS 및 focused Automation `f0865deaf4e0416da687f3171eeaadb7` exact8/8 PASS를 기록했다.
- Post-Implementation Review는 `P0 0 / blocking P1 0 / P2 exact2 non-blocking implementation evidence / TECHNICAL IMPLEMENTATION PASS`로 수렴했다.
- 잔여 P2 exact2는 기존 OpenXLSX 0.5.1 actual roundtrip/presentation/protection evidence와 persisted USkeletalMesh real capability scan evidence이며 후자는 P0-04 Dual Consumer Pilot에서 닫는다.
- Product Apply/Save/Workbook write/generic reflection/cutover 경로는 0으로 유지하며 exact next를 `CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot`으로 승격했다.

### v0.1.5 - 2026-09-29

- CCAS-P0-02 fresh mid-review에서 `P0 0 / blocking P1 3 / P2 2 / HOLD`를 확인하고 Current retire referencer, Provider fingerprint fail-open, Workbook/Provider schema mismatch exact3을 교정했다.
- desired/current reference graph를 분리하고 impact 분석은 두 graph의 reverse-adjacency union을 사용해 retire 후보의 current-only referencer를 보존한다.
- Provider semantic fingerprint를 canonical SHA-256으로 fail-closed하고 BuildDiff current fingerprint와 snapshot readback fingerprint의 모순도 차단했다.
- Workbook logical schema와 Provider descriptor exact compatibility를 검증해 implicit schema drift 해석을 금지하고 migration 경계로 되돌린다.
- per-record import/snapshot canonical comparison과 whole-current pre/post import semantic hash를 추가해 retire 후보를 포함한 mixed-time Product Preview를 fail-closed한다.
- final Official UE 5.8 Source Build `d592636c607b4df48af776a83ece64c9` PASS, focused Automation `0004a2f9594f4ab1a16eb17998130790` exact12/12 PASS를 기록했다.
- final 10k content + 100k child benchmark는 parse 0.101s / compiler hash+diff+consistency 1.795s / total 1.896s로 PASS했다.
- Fresh Re-review는 `P0 0 / blocking P1 0 / P2 exact1 non-blocking implementation evidence / TECHNICAL IMPLEMENTATION PASS`로 수렴했다. 잔여 P2는 OpenXLSX 0.5.1 actual roundtrip/presentation/protection evidence exact1이다.
- Product mutation/writer/cutover는 0으로 유지하며 exact next는 `CCAS-P0-03 Resource Catalog + Semantic Binding`이다.

### v0.1.4 - 2026-09-29

- CCAS-P0-02 Generic Compiler Core의 adapter-backed Workbook read, common/provider validation, current import/index, typed diff, reference graph, impact analysis, generated fingerprint/drift와 preview batch result를 구현했다.
- Core는 `ApplyReviewed`, Workbook staged write/reopen, Product Save와 authority cutover를 호출하지 않는 read-only preview boundary로 유지했다.
- disposable in-memory Adapter/Provider fixture와 focused Automation exact8을 추가했다.
- official UE 5.8 Source Build `8b8afac6443f433692990004cb2a052e` PASS 및 focused Automation `06c9db1ee69f469985e7956c692e8145` exact8/8 PASS를 기록했다.
- 10,000 content + 100,000 child benchmark에서 110,000 textual canonical cells를 실제 ParseCell로 처리했으며 parse 0.102s / hash+diff 0.785s / total 0.887s로 PASS해 기존 P2-2를 CLOSED했다.
- Post-Implementation Review는 `P0 0 / blocking P1 0 / P2 1 non-blocking implementation evidence / TECHNICAL IMPLEMENTATION PASS`로 수렴했다. 잔여 P2는 OpenXLSX 0.5.1 실제 roundtrip/presentation/protection evidence exact1이다.
- Product DataAsset mutation/cutover/writer activation은 0으로 유지했고 exact next를 `CCAS-P0-03 Resource Catalog + Semantic Binding`으로 승격했다.

### v0.1.3 - 2026-09-29

- CCAS-P0-01 Workbook Schema + Canonical Content Model을 Editor-only typed Core로 구현하고 Post-Implementation Review까지 완료했다.
- WorkbookSource/Sheet/Column/Row/Child identity, canonical scalar/reference/FText/null state, collection semantics, semantic hash, provider/workbook adapter seam, ExternalReadOnly, migration preview와 typed Change Set 계약을 구현했다.
- official UE 5.8 Source Build `c98d528a7e12483888b770b40323dc9c` PASS 및 focused Automation `a61a62ecc6024e67820d60591eb882f2` exact8/8 PASS를 기록했다.
- locale numeric ambiguity를 strict ASCII decimal grammar로 교정하고 regression을 고정했다.
- Post-Implementation Review는 `P0 0 / blocking P1 0 / P2 2 non-blocking implementation evidence / TECHNICAL IMPLEMENTATION PASS`로 수렴했다.
- Product DataAsset mutation/cutover/writer activation은 0으로 유지했고 exact next를 `CCAS-P0-02 Generic Compiler Core`로 승격했다.

### v0.1.2 - 2026-09-29

- CCAS-P0-00 blocking P1 exact10과 P2 exact4에 대한 C1~C12 Contract Correction을 동결했다.
- canonical xlsx exact path, semantic hash/source-control sidecar, single-writer/stable snapshot, AI typed Change Set, ContentKey ↔ current Runtime ID mapping, field-level mixed ownership, collection/canonical type/null token, legacy import fidelity, Unreal asset lifecycle/cook, xlsx adapter/security, durable operation journal, FText/machine-schema/USER UX, Existing System convergence를 명시했다.
- Vehicle pilot Product write는 P0-07 이전 exact0으로 유지하고 disposable fixture exact4 field allowlist만 허용했다.
- managed되지 않은 existing DataAsset 참조를 위해 `ExternalReadOnly` Catalog node 계약을 추가했다.
- Fresh Design Re-review 결과 `P0 0 / blocking P1 0 / P2 2 non-blocking implementation evidence / TECHNICAL DESIGN PASS`로 수렴했다.
- P2 exact2는 OpenXLSX UE 5.8 roundtrip evidence와 10k/100k synthetic performance benchmark이며 acceptance method가 이미 Freeze되어 CCAS-P0-01을 막지 않는다.
- exact next를 `CCAS-P0-01 Workbook Schema + Canonical Model`로 승격했다.
- Source / Product Asset / Current Product authority mutation은 0이다.

### v0.1.1 - 2026-09-29

- CCAS-P0-00 Pre-Implementation Design Review를 fresh Current Systems와 representative Source에 교차검수했다.
- 방향 자체를 무효화할 P0는 0으로 판정했다.
- 구현 전 동결이 필요한 blocking P1 exact10을 확인했다: Workbook binary/Git/file-lock, AI typed Change Set writer, ContentId와 기존 Runtime identity mapping, field-level mixed authority, complex collection/type canonicalization, legacy/default import fidelity, Unreal rename/redirector/hard-ref/cook, xlsx parser/dependency/security, batch journal/idempotency, FText/machine-schema/USER Workbook UX 경계.
- P2 exact4로 parse/batch performance, recovery snapshot, Excel authoring UX standard, future multi-workbook partition seam을 기록했다.
- Verdict를 HOLD로 전환하고 exact next를 `CCAS-P0-00 Contract Correction + Re-review`로 설정했다.
- Source / Product Asset / Current authority mutation은 수행하지 않았다.

### v0.1.0 - 2026-09-29

- USER와의 Weapon Guide USER Acceptance 재검토에서 출발한 방향 전환을 범용 Content System 정식 Plan으로 승격했다.
- Excel authority, generic provider/compiler, catalog/compare, Family/Variant, AI batch planning, Resource Catalog, Profile/Override, semantic socket role binding을 P0 범위로 정의했다.
- Vehicle + Weapon을 exact2 대표 consumer로 고정해 Weapon-only 추상화를 방지했다.
- 현재 DA authority를 즉시 뒤집지 않고 explicit CCAS-P0-07 Cutover Gate에서만 CCAS-managed 범위의 Workbook authority를 Current로 승격하도록 migration 경계를 추가했다.
- CF-FQ-055 WEA Technical PASS backend와 existing DA typed/durable infrastructure를 재사용하는 방향을 고정했다.

---

## 27. Migration

- 이 문서는 Product 구현이 아닌 Formal Plan이다. 작성 시점의 Current DA authority를 변경하지 않는다.
- `CF-FQ-055`의 WEA-P0-05 Final Technical PASS evidence는 유지한다. standalone USER Acceptance는 CCAS 방향 결정으로 Paused하고 backend reuse input으로 재분류한다.
- `CF-FQ-054` Equipment Builder는 CCAS의 Equipment / Vehicle / Weapon 관계 모델이 Freeze되기 전 USER UX를 재개하지 않는다.
- 기존 `DataAssetManagement.md`, `DataAssetAuthoring.md`, `VehicleBuilder.md`는 CCAS implementation / cutover 전까지 Current owner다.
- CCAS-P0-07이 실제 PASS할 때만 관련 Systems 문서의 authority / migration을 갱신한다.
