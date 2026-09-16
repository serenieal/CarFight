# CarFight UDS

- 상태: `Permanent Adoption / Retention Triage Complete / PASS`
- UDS profile: Shared UDS Core v0.1.2 / Migration v0.1.3
- full retention inventory: `Document/UDS/RetentionInventory.md v0.1.0`
- 전환일: 2026-09-16
- correction: 초기 MIG-03/05에서 Active/Ready Work만 materialize하고 terminal Work / Current Knowledge / accepted Evidence 일부를 누락한 completeness defect를 2026-09-16 재감사에서 교정했다.
- canonical semantic state: **`Document/UDS/records/**`**
- bounded current view: `Document/UDS/derived/Current.md` — authority0
- knowledge view: `Document/UDS/derived/Knowledge.md` — authority0
- historical view: `Document/UDS/derived/History.md` — authority0
- head cache: `Document/UDS/derived/heads.json` — authority0 rebuildable cache

## Corrected semantic coverage

```text
Active / Ready Work   = 3
Terminal Work         = 5
Current Knowledge     = 8
Recorded Evidence     = 7
Unresolved conflict   = 0

MIG-03 semantic loss
active/restartable Work missing = 0
next-action meaning loss        = 0
Current Knowledge owner missing = 0
accepted Evidence ref loss      = 0
USER/Technical boundary loss    = 0
```

기존 dirty 문서 본문을 비우거나 삭제해 이관하지 않았다. 의미 단위를 UDS record로 등록하고, 기존 Systems / ProjectSSOT / representative Plan이 이미 좋은 canonical detailed body인 경우 UDS는 stable `body_ref` / evidence reference를 소유한다. 따라서 전체 본문을 JSON으로 중복 복제하지 않는다.

## CarFight ownership split

`Document/ProjectSSOT/03_FeatureQueue.md`는 Feature backlog / priority / catalog / 착수 판단의 unique planning semantic을 계속 소유한다. UDS로 승격된 Work의 lifecycle은 canonical UDS Work record가 소유하고 FeatureQueue lifecycle 표기는 authority0 mirror/reference다.

`Document/ActiveWork.md`는 pre-cutover retained frozen authority0 snapshot이다. `01_ProjectState.md`와 `02_Roadmap.md`는 baseline/cycle-order owner를 유지하지만 promoted Work lifecycle slice는 UDS reference다. `Document/Plan/README.md`는 representative Plan navigation을 위한 authority0 projection이다.

대표 Plan은 detailed checkpoint/evidence body를, Systems는 Current implementation contract body를 계속 소유한다. UDS Work/Knowledge/Evidence object는 그 semantic identity, lifecycle, owner relation과 stable body/evidence reference를 canonical하게 소유한다.

## Migration state

초기 MIG-07 Complete 판정은 semantic completeness 재감사에서 철회됐고 immutable Project successor로 correction 상태를 기록했다. 이후 dirty documentation owner-set을 다시 분해해 누락된 terminal Work / Knowledge / Evidence를 추가했고, dirty-focused correction successor `ver_6377737a8ffe4641a0a4dc95cf55df1c`에서 그 범위의 MIG-03 loss exact5를 모두 0으로 재검증했다. 현재 Project head는 `ver_8d5af5db4c7d4b46b32cdb1b93a6429e`이며 full durable-owner Retention Triage HOLD다.

Retention Triage full-inventory correction에서 `Document_Entry.md`, `Systems/SystemIndex.md`, `ProjectSSOT/README.md`, `CombatPlan/README.md`를 기준으로 Current Systems/ProjectSSOT/CombatPlan/DesignSource/Plan/Archive owner family를 전부 분류했다. explicit set 밖의 CarFight-owned `Document/**` semantic은 `KEEP_PENDING_REVIEW` catch-all로 보존한다. 따라서 기존 UDS Knowledge exact8에 개별 materialize되지 않은 Ammo/CombatFx/HitDamage/Launcher/MissileGuidance/Projectile 등 Current System도 `SystemIndex.md`를 통해 KEEP/reachable하며, 본문을 JSON으로 중복 복제하지 않는다. unknown/ambiguous discard=0, RETIRE/DISCARD candidate=0이고 full durable-owner inventory와 Retention Triage는 PASS다.
