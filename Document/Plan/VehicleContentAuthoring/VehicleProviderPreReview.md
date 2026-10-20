# CF-FQ-059 — VCCAS-P0-03 Vehicle Typed Provider Pre-Implementation Review

- Version: v0.1.0
- Date: 2026-10-13
- Status: **PRE-IMPLEMENTATION SOURCE CONTRACT REVIEW COMPLETE; IMPLEMENTATION NOT YET OPENED**.
- Verified actual source: `CFContentProvider.h v1.1.0`, `CFContentTypes.h`, `CFContentPilot.h v1.1.0`, `CFVehicleRefEvidence.h v1.3.0`, `CFVehicleAuthoringService.h v1.10.0`, `CFVehicleUXTypes.h v1.2.0`, `CFVehicleUXOps.cpp`, `CFContentCatalog.cpp`, prior `VehicleContentAuthoringPlan.md v0.1.11` frozen C1–C6.
- USER physical resource rule: current StaticMesh bodies are temporary; any P0-03 schema/provider implementation must be usable for references/unknowns/draft data **before** final chassis, Wheel Mesh, Wheel Anchors or user asset editing. Production Build/Apply/Publish later still requires valid actual dependencies.

## 1. Actual Source Contract Results

| Concern | Observed code fact | Implementation consequence |
| --- | --- | --- |
| Provider interface | `ICFContentProvider` already defines `DescribeSchema, ImportExisting, BuildCurrentSnapshot, ValidateRecord, BuildDependencyEdges, BuildDiff, BuildReviewedMutationPlan, ApplyReviewed, ReadbackFingerprint`. `FCFContentProviderRegistry` enforces registration and has schema fingerprint. | Vehicle must implement existing exact interface, not a second custom CCAS framework; review/apply remains typed and fail-closed. |
| Schema types | `FCFContentFieldDescriptor` includes stable ColumnId, `ValueType`, exact `ECFContentFieldOwnership`, `ExternalOwnerId`, canonical unit, `TextMode`, `bAllowNone` and `bRequired`. | **Actual enum**: `Unspecified / CCASManaged / ExternalManaged / LegacyPreserve / Derived / ReadOnly / Deprecated`. Older Plan prose `ExternalReadOnly` is **not** a field ownership enumerator; it belongs to `ECFContentManagementState::ExternalReadOnly`. Older prose `GeneratedProjection` is **not** the concrete enum either: map intended projection to `Derived` (or `ReadOnly` by real semantics) without inventing enum members. |
| Vehicle content key | `CFContentCatalog.cpp` emits `ContentTypeId.Value = "vehicle"` and `ContentId = "vehicle_" + RecipeId.ToString(Digits).ToLower()`. | Must use **exact lowercase `vehicle` type** and preserve existing generated RecipeId key. Do not use `Vehicle` case, Chassis path or arbitrary display label for canonical ContentId. |
| Resource pilot | `FCFVehicleResourcePilot` imports existing Recipe+AssetReader state into shared Resource Core as **read-only pilot**. | Pilot is NOT provider, not Product creator, not capable of saving. Do not mis-register pilot as `ICFContentProvider` or block research schema on absent Wheel Anchors. |
| Reference | `FCFRefVehicleIdentity`, `FCFRefSourceCitation`, `FCFRefClaim`, `FCFRefUnknownFact` and `FCFVehicleRefEvidencePayload` exist. `ValidateInitialResearchPayload()` validates exact-one Primary, unique IDs, FACT citation, no initial GAME_BIAS, no unresolved blocking conflicts. | Map OEM `ReferenceSpec` to existing `UCFVehicleRefEvidence`; 6 candidate vehicles mean **6 independent research payloads**, not a six-Primary single Evidence. Unknown is explicit, not default zero. |
| R2 creation | `FCFVehicleAuthoringService::PreviewVehicleRecords` in `CFVehicleUXOps.cpp` blocks CallerKind except `SlateUI` or `Automation`; its result already supplies exact review ProposalHash. `CreateVehicleRecords` re-previews and checks explicit OwnershipWrite approval + exact ProposalHash before creating Definition/Recipe, neither saves. | A real independent trusted AI admission must verify external Project/Caller capabilities and exact R2 scope before passing to the existing core; **do NOT pass `CallerKind=Automation` from AI** or merely add permissive AI enum check. Protect existing SlateUI/Automation regression. |
| Blank chassis | `FCFVehicleRecordCreateRequest.ChassisMesh` is optional and only validated when non-null; create sets `Recipe->AssetIntent.ChassisMesh=Request.ChassisMesh`. `VehicleBuilder.md` states Blank Chassis can start. | Research/typed draft should represent future Chassis Resource as deferred/unknown, not synthesize fallback mesh. Stable resource ID for final Product cannot be fabricated before user replacement. |
| Output authority | `FCFVehicleApplyService` remains the unique writer for `UCFVehicleData`; `CreateVehicleRecords` creates Definition/Recipe two-record unsaved; `FCFVehicleCatalogPromoService` is only existing membership writer; `CarFight_Content.xlsx` CCAS canonical record authority. | P0-03 provider must NOT directly mutate raw UObjects, `AllowedVehicleData`, Workbook binary or create new Save/Publish systems. P0-04 later owns durable multi-package operations. |

## 2. Contract corrections mandatory before writing provider implementation

**C-03-01 — ownership type exactness (P1 design correction)**

Use only existing enum `ECFContentFieldOwnership`:
- `CCASManaged` for explicitly reviewed declarative `vehicle` sheet fields/citation bindings.
- `ExternalManaged` with `ExternalOwnerId="VehicleBuilder"` for current physical Body/Socket/Hardpoint, and relevant external Profile owners.
- `ReadOnly` or `Derived` for imported/deterministically projected values that should not be independently authored by Workbook.
- `LegacyPreserve` for unmanaged legacy-only data that CCAS must not overwrite.
- `Unspecified` invalid; `Deprecated` only when an actual field is retired.
- `ECFContentManagementState::ExternalReadOnly` means the **entire record state**, not a per-field ownership enum. Never conflate them. The Plan's old conceptual `GeneratedProjection` corresponds to actual `Derived/ReadOnly` semantics and requires a reviewed mapping per field rather than new enum.

**C-03-02 — independent research readiness vs Product validity**

Two explicit validation modes must be distinguished rather than pretending a single `FCFContentRecord` describes a fully created Product:
- `ResearchCandidate / PreRecipe`: no RecipeId and therefore **no canonical `vehicle_<RecipeId>` ContentKey** yet; holds six human-readable Category/ReferenceVehicle identity and sourced FACT/Unknown as per `VehicleReferenceSourceReview.md`, no ResourceId binding, no `ICFContentProvider::ApplyReviewed`.
- `ReviewedManagedRecord / PostRecipe`: after approved exact R2 creation, actual RecipeId readback, then CCAS vehicle ContentKey, stable schema and workbook/manifest authority, typed diff/preview, and approved apply. No arbitrary temporary GUID to resolve the circularity.

This is an operational separation of **research input and existing core record**, not a second persistent Vehicle authoring store. No additional mutable Product copy is created.

**C-03-03 — true trusted AI R2 admission**

Existing `PreviewVehicleRecords` denies AI callers *before* path/type checks. P0-03 must add a well-scoped **trusted managed orchestration adapter** whose authority comes from a real caller capability and reviewed exact `OwnershipWrite` scope, then call existing core's guarded two-record operations through a deliberately designed approved trusted seam. Never merely alter the core to accept `CallerKind=AI` because this enum is an identity, not proof of authorization. Inability to prove approved capability => fail-closed `UnsupportedOperation`/`ApprovalRequired`.

**C-03-04 — research fields to Profile/VehicleData**

- `ExteriorLengthMm`, `ExteriorWidthMm`, `ExteriorHeightMm`, `WheelbaseMm`: reference FACT (existing `FCFRefClaim`), **not** known runtime body size/socket location.
- `PublishedEnginePower` and `PublishedEnginePeakTorque` (with native PS/HP/kW/Nm/kgf·m, RPM, market): reference facts, not a complete UE engine torque curve.
- `CurbWeightKg` and `GrossWeightKg`: research facts with exact-grade qualifier; they are **not automatically approved** `BaseVehicleMassKg` and `MaximumGrossMassKg`.
- `DriveLayout` and `TransmissionKind`: exact real-car facts, while shift ratios, Chaos driveline, 4x4 selectable low-range, IVT transmission, hybrid motor representation require independent typed resolver/gameplay mapping.
- `WheelTireCode` and `TrackFrontMm/TrackRearMm` are real-vehicle references, not direct physics `WheelRadius/Width`/mesh/socket.
- Runtime `WheelAnchor`, `HardpointSlots`, `MountProfiles`, cameras/default sensor/defense and equipment remain delegated to existing typed owners; do not synthesize as required by `ResearchCandidate`.
- Special case: STARIA **245PS total hybrid system output ≠ ICE 180PS**, so provider cannot set `EnginePeakPower=245PS` without a separate approved hybrid powertrain model.

**C-03-05 — read/diff/approval**

Provide canonical 1:1 field/UnitId ownership schema, explicit missing value state (`Inherit/None/Value` vs `UnknownFact` source meaning), and stable hash for evidence identity + source/citation/Claim + revision. Imported legacy vehicle `ExternalReadOnly` record stays read-only; no silent recipe import. Reviewed mutation plan must revalidate base WorkbookSemanticHash, schema fingerprint, RecipeId/RecipeFingerprint, target DefinitionHash, EvidenceFingerprint, Resource capability when physical asset exists; no silent rebase.

## 3. Implementation units and required technical tests (no code yet)

**P0-03A — canonical read-only Vehicle schema + source provenance mapping**: new Vehicle-specific provider module located in existing `UE/Source/CarFight_ReEditor/Public|Private/DataAuthoring/`, but exact file name only when implementing. Register lower-case `vehicle`, stable field ownership types and units; fail if duplicate type, invalid empty identity or `Unspecified`, source fact lacking citation; import known Vehicle records without editing legacy. **Physical resource source absent must NOT block research-only validation**.

**P0-03B — typed readback/diff and builder authority bridge**: import current existing Recipe/Profile/Evidence/Target via typed facade; preserve `vehicle_<RecipeId>`; deterministic changed-path diff independent from worksheet row order; protect `ExternalManaged` per field and no raw target write. Current temporary meshes must not become a forced dependency for research.

**P0-03C — trusted R2 admission with exact scoped review**: security-sensitive typed entry does not bypass current SlateUI/Automation guard, uses capability-backed proof and existing R2 OwnershipWrite+fresh proposal; tests for identity spoof, unauthorized requests, changed hash, path collision, foreign project/checkout, attempt to modify pre-existing user assets.

**P0-03D — integration/acceptance**: full focused Automation and reviewed readback. Durable per-package Save/Recover/Workbook promotion/AllowedVehicleData Publication belongs **P0-04**, not an experimental P0-03 provider side effect. If full P0-03 requires an external capability not present, stop at specific Blocked test, don't pretend runtime product exists.

| Required test | Expected result |
| --- | --- |
| Six ReferenceSpec records without current real Chassis | **Allowed in research-only lane**, not published/mutated |
| A sixth candidate with same car facts but altered market/trim | Different identity, no silent field merge |
| FACT claim with absent citation | Validation FAIL |
| Unknown mass/ratio but no invented scalar | Explicit unknown recorded; published/required runtime field stays blocked |
| Legacy Recipe/Target and shared Wagon data | No silent overwrite; read-only import |
| Wrong ContentType `Vehicle` vs `vehicle` | No false matches; canonical lowercase enforced |
| Draft lacking RecipeId attempting CCAS managed vehicle ContentKey | FAIL CLOSED; actual ID only after R2 |
| AI caller tries `CallerKind=Automation` to create | FAIL CLOSED at trusted boundary |
| Altered ReviewPackageFingerprint, stale Workbook/Target/Recipe | FAIL CLOSED; no Asset mutation |
| Mixed ICE/Hybrid outputs, exact vs MAX curb weight, torque units | Typed source provenance conflict warning/block as appropriate |
| Fail during Product Save/Catalog publication | Deferred P0-04 tests; must never falsely report P0-03 success as durable Production |

## 4. Review verdict / execution

**P0-03 Pre-Implementation Review verdict: DESIGN CORRECTION REQUIRED exact2 technical issues to resolve during the implementation unit:**
1. Match real C++ ownership enum and distinguish record `ExternalReadOnly` from field ownership (C-03-01). Existing Plan conceptual `ExternalReadOnly` / `GeneratedProjection` are **not** valid C++ field enum variants.
2. Implement authority-backed AI R2 admission without spoofing `SlateUI/Automation`; current core code actively denies AI request (C-03-03).

Other critical boundaries — separate research pre-Recipe keys, post-R2 IDs, no physical-mesh prerequisite for source research, no second Target writer — have explicit design resolution in this document and the frozen C1–C6. No need to invent new CCAS framework, UI, Workbook editor or temporary virtual Mesh assets.

**Current status**: reviewed source contracts PASS / two design-to-code corrections identified / P0-03 implementation **NOT RUN**, build and automation **NOT RUN**. Because this turn is research and pre-implementation technical review, no UE source modification, UE Asset mutation, DataAsset creation or Production enrollment was attempted.

## Changelog

- v0.1.0: Fresh source-aware P0-03 pre-implementation review, actual enum mismatch and trusted AI R2 admission gap, specific unit/citation/identity/missing-resource handling, 4 bounded implementation units and focused acceptance cases.
