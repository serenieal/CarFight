# CarFight UDS Retention Inventory

- 버전: v0.1.0
- 날짜: 2026-09-16
- 상태: Current / Full Namespace Retention Triage
- 기준: Shared UDS Migration Spec v0.1.3
- 역할: CarFight main_game 문서 semantic owner의 retention disposition을 완전하게 정의하는 migration evidence/catalog
- 비책임: 기존 ProjectSSOT/Systems/CombatPlan/Plan/DesignSource 본문의 내용을 재소유하지 않는다.

## 1. 핵심 원칙

CarFight의 상세 body는 기존 owner에 남긴다. UDS는 semantic identity와 stable owner reference만 추가한다.

공식 색인에 직접 열거되지 않은 문서를 누락 또는 discard로 취급하지 않는다. 아래 명시 집합에 속하지 않는 CarFight-owned `Document/**` semantic unit은 `KEEP_PENDING_REVIEW`다. 독립 저장소 `Document/SSOT`는 Shared SSOT external authority로 취급하며 CarFight가 내용을 재소유하지 않는다.

## 2. Retention Set

| Owner family / namespace | Disposition | 근거 / 현재 owner |
| --- | --- | --- |
| root `AGENTS.md`, `Document/AGENTS.md`, `Document/CodeWorkGate.md` | KEEP | durable 작업·보호·validation 운영 규칙 |
| `Document/Document_Entry.md` | KEEP | CarFight 전체 문서 owner/routing catalog |
| `Document/ProjectSSOT/README.md` + `00_Vision.md` | KEEP | 프로젝트 목적·장기 방향·owner model |
| `Document/ProjectSSOT/01_ProjectState.md` | KEEP + lifecycle slice DERIVE | 고유 프로젝트 baseline/risk는 KEEP; UDS와 중복되는 promoted Work lifecycle은 authority0 reference |
| `Document/ProjectSSOT/02_Roadmap.md` | KEEP + lifecycle slice DERIVE | cycle/sequence planning은 KEEP; UDS current lifecycle duplicate는 authority0 reference |
| `Document/ProjectSSOT/03_FeatureQueue.md` | KEEP + lifecycle slice DERIVE | backlog/priority/catalog/착수 판단은 KEEP; promoted Work lifecycle duplicate는 authority0 reference |
| `Document/ProjectSSOT/04_ProjectDecisions.md`, `05_TestChecklist.md` | KEEP | durable project decision / regression baseline owner |
| `Document/Systems/**`에서 `SystemIndex.md`가 Current로 열거한 모든 System body | KEEP | verified Current implementation contract owner. Combat/Config/DataManagement/Input/Targeting/UI/Vehicles 전체 Current set 포함 |
| `Document/Systems/SystemIndex.md` | KEEP | Current System body의 공식 complete routing index |
| `Document/ProjectSSOT/CombatPlan/README.md`, `00_Index.md`, `01~18`, `DecisionLog.md`, `Template.md` | KEEP | 전투 정체성·장기 도메인 설계·decision/balancing/prototype owner family |
| `Document/DesignSource/**` | KEEP | Document_Entry가 정의한 long-term north-star/source owner; Current 착수 authority는 아님 |
| current UDS Work가 참조하는 representative Plan | KEEP | detailed checkpoint/evidence body owner |
| 완료/대체 Plan 및 `Document/Plan/Archive/**` | KEEP | terminal outcome/evidence/causal history retained reference |
| `Document/ProjectSSOT/Archive/**` | KEEP | historical project/system reference; Current authority 아님 |
| `Document/Maintenance/**` | KEEP | document structure/link/role audit utility and evidence |
| `Document/ActiveWork.md` | DERIVE / DE-DUPLICATE | pre-cutover retained authority0 snapshot; 새 Current write 금지 |
| `Document/Plan/README.md` lifecycle/status projection | DERIVE / DE-DUPLICATE | navigation/projection authority0; detailed Plan body는 KEEP |
| `Document/UDS/derived/**` | DERIVE / DE-DUPLICATE | canonical records에서 재생성하는 authority0 view/cache |
| `Document/SSOT/**` | KEEP as External Authority Reference | 독립 Shared SSOT repository. CarFight가 body를 재소유/수정하지 않음 |
| 위 명시 집합에 포함되지 않는 CarFight-owned `Document/**` semantic unit | KEEP_PENDING_REVIEW | catch-all 보존 집합; 현재 역할/대체 owner가 확정될 때까지 discard 금지 |

## 3. Current System completeness

`Document/Systems/SystemIndex.md`의 Current set 전체를 하나의 durable-owner family로 inventory한다. 현재 UDS Knowledge exact8에 없던 WeaponData/WeaponFire/Launcher/Ammo/FireFeedback/Projectile/MissileGuidance/DamageHitContext/HitDamage/VehicleDefense/CombatFx/ProjectRuntimeConfig/DataAssetManagement/DataAssetAuthoring/Input/InGameUI/AimReticle/DisplayTextPolicy/VehicleDebugPanel/VehicleAim/VehicleCamera/VehicleCoreDecisions/VehicleDrive/VehiclePawnLegacy/RuntimeApply/VehicleSteering/WheelSync 등도 모두 KEEP 대상이다.

이 목록을 UDS JSON object 수십 개로 중복 복제하지 않는다. `SystemIndex.md`가 상세 owner exact set을 유지하고 UDS Retention Catalog Knowledge가 그 index를 stable reference로 소유한다.

## 4. Work / Status 특례

오래된 ActiveWork/ProjectState/Roadmap/FeatureQueue의 Work status slice는 완료·대체가 증명되면 terminal/history/authority0로 수렴시킬 수 있다. 그러나 planning 의미, project decision, current System contract, CombatPlan principle, USER/Technical boundary, accepted evidence는 함께 제거하지 않는다.

## 5. Discard Audit

```text
RETIRE / DISCARD candidate                         = 0
Unknown / ambiguous semantic discarded             = 0
KEEP_PENDING_REVIEW physically removed              = 0
Retained Legacy body deleted / emptied              = 0
Product source/asset/runtime mutation by triage     = 0
Legacy lifecycle write-authority re-enabled          = 0
```

## 6. Coverage Closure

```text
Explicit KEEP
+ Mixed KEEP/DERIVE project planning owners
+ Explicit DERIVE / DE-DUPLICATE
+ External Shared reference
+ Catch-all KEEP_PENDING_REVIEW
= CarFight-owned Document semantic namespace 전체
```

따라서 index에 없는 문서도 삭제 가능 상태로 빠지지 않는다. `KEEP_PENDING_REVIEW`는 향후 명시적 review에서만 좁힐 수 있고 RETIRE/DISCARD에는 M-11 loss0 개별 증명이 필요하다.

## 7. Acceptance

- full durable-owner namespace inventory: COMPLETE
- Current System owner family coverage: COMPLETE via `SystemIndex.md`
- ProjectSSOT current owner set coverage: COMPLETE
- CombatPlan owner family coverage: COMPLETE
- terminal Plan/Archive reachability: preserved
- unknown/ambiguous discard: 0
- UDS detailed body duplication: 0
- authority cutover rollback: 0

Verdict: **PASS — Retention Triage Complete with conservative KEEP_PENDING_REVIEW catch-all.**
