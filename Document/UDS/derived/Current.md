# CarFight UDS — Current

> Derived view. **Authority0 cache/view**; canonical Active/Ready Work lifecycle authority is `Document/UDS/records/**`.

| Feature | State | Phase | Next |
| --- | --- | --- | --- |
| CF-FQ-039 Production UI Visual Rework | Active | VPR-P0-01 | VT12 SPEED/RPM + ARMOR composition USER review |
| CF-FQ-041 Runtime Content Apply | Ready | RTA-P0-06 | Packaged Demo |
| CF-FQ-054 Equipment Authoring / Guided Equipment Builder | Paused | EBA-P0-05 | 무장 제작 가이드 구현·통합 준비 후 USER Editor UX 재개 |
| CF-FQ-055 Weapon Equipment Authoring Guide | Ready | WEA-P0-05 | USER Acceptance Pending — Cannon / Rocket / Guided Missile / HitScan |
| CF-FQ-056 Projectile Flight Physics / 발사체 공통 비행 물리 | Ready | PFP-P0-03 | Reticle Technical RESOLVED / USER trajectory blocked — review fixture + managed PIE start |

`CF-FQ-039`는 representative Plan v0.1.33, `CF-FQ-041`은 v0.1.18, `CF-FQ-054`는 v0.1.17의 HOLD checkpoint, `CF-FQ-055`는 WeaponEquipmentAuthoringPlan v0.2.6의 WEA-P0-05 Final Technical PASS / USER Acceptance Pending을 반영한다. `CF-FQ-056`은 ProjectileFlightPhysicsPlan v0.1.5를 반영한다. PFP-P0-02 TECHNICAL IMPLEMENTATION PASS는 보존되고 Reticle Technical Review는 RESOLVED다. USER trajectory는 persisted Rocket/Missile review fixture 부재와 managed PIE start infrastructure blocker 때문에 아직 NOT RUN이며 PFP-P0-03에 머문다.

`Document/ProjectSSOT/03_FeatureQueue.md`는 backlog / priority / catalog의 CarFight planning authority를 계속 소유한다. 다만 UDS로 승격된 Work의 state/phase/next는 이 view가 아니라 canonical UDS record가 authority다.

대표 Plan은 상세 checkpoint와 validation/evidence를 계속 소유하고 Systems는 Current implementation Knowledge를 계속 소유한다.
