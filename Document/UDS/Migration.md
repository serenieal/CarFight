# CarFight UDS Migration Checkpoint

## Correction Summary — Historical Dirty-focused Partial PASS / Retention Triage Full-Inventory Complete / PASS

초기 UDS-08 cutover는 Active/Ready Work exact3과 routing de-authorization은 성공했지만, MIG-01~03 semantic decomposition을 **현재 보이는 Active/Ready lifecycle에 과도하게 축소**했다. 그 결과 pre-existing dirty에 있던 terminal Work, Current Knowledge, accepted Evidence reference 일부가 UDS canonical namespace에 materialize되지 않은 채 MIG-07을 Complete로 판정했다.

이 completeness 판정은 2026-09-16 사용자 지적 후 철회했다. 기존 Legacy dirty 본문은 비워지거나 삭제되지 않았으며 retained body에서 semantic correction을 수행했다.

## MIG-00 Inventory — Dirty Owner-set Re-audit PASS / Full Durable-owner Inventory HOLD

Cutover 대상과 semantic source owner-set을 다음처럼 다시 inventory했다.

- Current/restore projections: `ActiveWork`, ProjectState active slice, Roadmap active slice, FeatureQueue promoted lifecycle, Plan Index
- Representative Plan dirty: RuntimeApply, EquipmentAuthoring, VehicleFitting, InventoryFoundation, TargetSelect, VehicleDataTuning
- Current Knowledge dirty: SensorContact, VehicleBuilder, VehicleData, VehicleRuntime, TargetSelect, VehicleInventory, LockSensorEW specification
- Product source/assets/config: documentation migration source가 아니며 UDS가 mutate하지 않음

Planning-only FeatureQueue candidate backlog는 UDS Work로 무차별 복제하지 않고 기존 planning authority에 유지했다.

## MIG-01 Semantic Decomposition — CORRECTED / PASS

```text
promoted active/ready lifecycle → Work exact3
terminal migrated Feature       → Work exact5
current implementation/contract → Knowledge exact7 system/spec + tuning policy exact1
accepted Build/Test/Audit/USER  → Evidence exact7
Current/Knowledge/History pages → Derived authority0 views
Git/Runtime/Asset reality       → External Authority / referenced evidence
```

기존 Systems/Plan 본문이 이미 상세 canonical body인 경우 UDS는 full-body copy 대신 semantic identity + stable body/evidence reference를 canonical하게 소유한다.

## MIG-02 Mapping / Collision Audit — CORRECTED / PASS

Active/Ready exact3:

1. CF-FQ-039 Production UI Visual Rework — Active
2. CF-FQ-041 Runtime Content Apply — Ready
3. CF-FQ-054 Equipment Authoring / Guided Equipment Builder — Ready

Terminal dirty Work exact5:

1. CF-FQ-015 Vehicle Data Tuning Pass — closed / superseded
2. CF-FQ-034 Vehicle Fitting / Mass Runtime — closed / superseded
3. CF-FQ-035 Inventory Foundation — closed / superseded
4. CF-FQ-026 Target Select System — closed / superseded
5. CF-FQ-046 Vehicle Builder User Information UX — closed / completed

USER/Technical boundary를 별도 확인했다.

- CF-FQ-015 VD-P0-04 USER Tuning = Superseded / Not Executed / USER PASS 아님
- CF-FQ-034 FIT-P0-07D USER Driving Feel = Superseded / Not Executed / USER PASS 아님
- CF-FQ-035 FFIT-P0-05 ownership-aware USER UI/PIE = Superseded / Not Executed
- CF-FQ-026 old USER workflow = Superseded / Not Executed, TargetPoint USER = Inconclusive
- CF-FQ-046 VBIUX-P0-05F USER Acceptance = PASS
- Sensor / VehicleData correction evidence = Technical PASS only; 미실행 live PIE를 USER PASS로 확대하지 않음

## MIG-03 Shadow / Semantic Materialization — CORRECTED / PASS

Canonical head-set final coverage:

```text
Project           = 1
Active/Ready Work = 3
Terminal Work     = 5
Knowledge         = 8
Evidence          = 7
Total heads       = 24
Conflict          = 0
```

MIG-03 exit exact5:

```text
active/restartable Work missing = 0
next action meaning loss        = 0
Current Knowledge owner missing = 0
accepted evidence reference loss = 0
USER/Technical boundary loss    = 0
```

Dirty-focused correction head:
`ver_6377737a8ffe4641a0a4dc95cf55df1c`

Full-inventory HOLD head:
`ver_8d5af5db4c7d4b46b32cdb1b93a6429e`

Current Project head:
`ver_651687541f8d4d8b8269eb1a322626ad` / Retention Triage Complete / PASS

이 successor는 correction-pending Project version `ver_defa7085b07f4590a6a115fcf1685095`를 parent로 가지며, 최초 premature permanent-adoption version도 immutable history로 보존한다.

## MIG-04~07 Authority Cutover — REVALIDATED / PASS

Authority cutover 자체는 유지 가능했다.

- UDS Work/Knowledge/Evidence = canonical semantic objects
- `derived/Current.md`, `Knowledge.md`, `History.md`, `heads.json` = authority0 views/cache
- ActiveWork = retained frozen authority0 snapshot
- ProjectState/Roadmap = project baseline/sequence owner + promoted lifecycle UDS reference
- FeatureQueue = planning catalog owner + promoted lifecycle authority0 mirror
- Plan Index = representative Plan navigation authority0
- Representative Plan = detailed checkpoint/evidence body owner
- Systems = detailed Current implementation body owner

Legacy body를 physical delete/move하지 않았다. Detailed body를 UDS JSON에 전부 복제하지 않고 stable reference로 유지하므로 Source/View/Owner Exact1 원칙도 유지된다.

## Final Correction Validation

- pre-existing dirty body deleted/emptied = 0
- unrelated dirty overwrite = 0
- Product source/asset/runtime mutation by correction = 0
- dual writer introduced = 0
- unresolved canonical conflict = 0
- MIG-03 semantic loss exact5 = 0
- stage / commit / push = 0

Full durable-owner inventory는 `Document/UDS/RetentionInventory.md v0.1.0`에서 `Document_Entry`, `Systems/SystemIndex`, ProjectSSOT, CombatPlan과 namespace complement까지 분류했다. Structured UDS Knowledge exact8에 개별 materialize되지 않은 Current System/long-term design body도 공식 index를 통해 KEEP/reachable하며, 나머지 CarFight-owned `Document/**` semantic은 `KEEP_PENDING_REVIEW` catch-all로 보존한다.

CarFight full-project migration verdict: **PASS — Retention Triage Complete**.

Work/Status 계열의 오래된 항목은 완료/대체 evidence가 있으면 terminal/history/authority0로 정리할 수 있다. Principle/Contract/Spec/Decision/System/DesignSource 계열은 명시적 superseding owner와 semantic-loss0가 없으면 KEEP이며, 판단이 모호하면 `KEEP_PENDING_REVIEW`다. unknown/ambiguous discard=0, RETIRE/DISCARD candidate=0이다.

Authority cutover는 유지하고 Legacy lifecycle dual-write는 재활성화하지 않는다. retained body 삭제/비움=0, Product source/asset/runtime mutation=0이다.
