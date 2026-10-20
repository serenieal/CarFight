# CF-FQ-059 — Core6 Representative Vehicle Specification Roster

- Version: v0.1.2
- Date: 2026-10-19
- Phase: **VCCAS-P0-02 reference research working roster**; original mesh comparison is historical only
- Scope: CityCar, Compact, Coupe, Pickup, SubCompact, Van within /Game/CarFight/Vehicles/Meshes ONLY. Wagon remains reference case; SUV/Sedan unchanged.
- Status: **OEM L/W/H/Wheelbase VERIFIED; six working real-car reference identities assigned for P0-02 research. Final vehicle identity/trim approval and persistent UE Evidence remain separate. Physical placeholder mesh resemblance is NOT an acceptance criterion.**
- No C++/BP/UE asset/Recipe/Profile/Production/Workbook changes or Git operations.

## Current scope override — USER temp placeholder replacement later (2026-10-13)

All current Core6 Body meshes are **temporary placeholders** that USER will replace later. Their local geometry, axes, scale, Socket0 status, and silhouette **do not constrain choosing real vehicles or collecting facts**. The old section 3 compares prior placeholder local AABBs only as a historical observation; it is not a review task or a work prerequisite. Do not ask USER to edit any mesh or perform car-to-mesh silhouette acceptance to proceed to P0-02. The **current sourced FACT/Unknown/identity field-level draft** is `VehicleReferenceEvidenceDraft.md v0.1.2` (fresh 2026-10-19 OEM source review; P0-02 acceptance still HOLD). Final physical asset/wheel authoring and runtime validation are deferred, not assumed complete.

---

## 1. Six representative real-vehicle choices and official size targets

Dimensions below are exterior length / width / height and wheelbase, in mm.

| Core body | Exact selected representative identity / market | Length | Width | Height | Wheelbase | OEM source |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| CityCar | 2027 Kia Morning 1.0 Gasoline Trendy, 14-inch, KR | **3595** | **1595** | **1485** | **2400** | https://www.kia.com/kr/vehicles/morning/specification |
| Compact | Suzuki Jimny 5-door Alpha AT / India, Suzuki brochure June 2024 (not asserted 2026 MY) | **3985** | **1645** | **1720** | **2590** | https://www.nexaexperience.com/content/dam/msil/nexa/in/en/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf |
| Coupe | 2026 Toyota GR86 6MT / Oman | **4265** | **1775** | **1310** | **2575** | https://www.toyotaoman.com/wp-content/uploads/2026/01/Gr-86-specsheet.pdf; wheelbase is independently published in Toyota Korea official GR86 specification: https://www.toyota.co.kr/models/gr86/ |
| Pickup | Toyota HiLux SR5 4x4 6AT Double-Cab with V-Active / current Australia Toyota range Oct 2026, MY not independently assigned | **5320** | **1885** | **1865** | **3085** | https://www.toyota.com.au/hilux/range |
| SubCompact | Hyundai Venue KR Smartstream 1.6 IVT 17-inch, KR Hyundai published catalogue; **Premium exact trim not confirmed by these facts** | **4040** | **1770** | **1585** | **2520** | https://www.hyundai.com/content/dam/hyundai/kr/ko/data/vehicles/catalog/en/venue-catalog-eng.pdf |
| Van | Hyundai The New STARIA Hybrid Cargo 5-seat 17-inch / KR (current 2026-10 observation; exact OEM model-year not independently frozen) | **5255** | **1995** | **2000** | **3275** | https://www.hyundai.com/kr/ko/e/vehicles/the-new-staria-hybrid/intro |

Additional trim/market checks:
- Morning grade: https://www.kia.com/kr/vehicles/morning/price (2026-09 KR, MY2027). 14-inch specific weight, tire and axle track; not 16-inch or 2-seat Van.
- GR86 Oman brochure includes 2026 233 HP but no wheelbase; the Toyota Australia OEM GR86 2024 wheelbase 2575mm is a cross-market same-platform supplement, **not falsely claimed as a figure from the Oman sheet**.
- HiLux Australian SR5 current range is **1885mm wide**; a historical 2025 Australian dealer SR5 variant was 1855mm wide and is NOT mixed into the current selected grade. Present listing is the chosen specification baseline.
- Venue height is **1585mm for 17-inch**, not 1565mm for 15-inch or 1610mm with roof rack.
- STARIA Cargo height **2000mm**, NOT Tourer 1990mm.
- Jimny specified **five-door** Indian vehicle with rear spare cover; NOT shorter three-door international variant.

## 2. Additional published reference facts (record; no Game DataAsset mutation)

| Lane | Drivetrain / engine / transmission | Output (OEM units) | Mass evidence | Tire/track and risk |
| --- | --- | --- | --- | --- |
| CityCar | 998cc gasoline, FWD, 4AT | **76PS at 6200rpm, 9.7kgf·m at 3750rpm** | **975kg** 14-inch Korean certified curb weight | 175/65R14; front track1406mm rear1415mm (14-inch). Different 16-inch curb is 1010kg. |
| Compact | Suzuki 1462cc K15B petrol, ALLGRIP PRO low-range selectable 4WD, 4AT | **104.8PS at 6000rpm; 134.2Nm at 4000rpm** | **1205kg curb**, 1545kg gross for Alpha AT | OEM 2024 India catalogue, 210mm unladen ground clearance, market-specific identity. |
| Coupe | 2026 Oman 2.4L Boxer D-4S, RWD, 6MT | **233 HP at 7000rpm** (Oman sheet) | **Unknown for exact Oman MT**, do not borrow US/AU curb mass as exact Oman mass | 215/40R18. Engine power in HP rather than PS/kW must not be silently converted to a different market claim. |
| Pickup | Toyota AU 2.8L turbo-diesel 4x4 V-Active, 6AT | **150kW, 500Nm** | **2155kg maximum curb** according to Toyota AU current SR5 range, not unique proven weight of each option; GVM range3080–3120kg, exact selected GVM NOT frozen | Front track1530mm rear1555mm; high bed/bumper visual proportions unverified against existing mesh. |
| SubCompact | 1598cc Smartstream Gasoline 1.6, FWD, IVT | **123PS at 6300rpm; 15.7kgf·m at 4500rpm** | **1215kg curb** 17-inch IVT, Hyundai KR catalogue | 205/55R17; front track1535mm; **rear track 1546mm table / 1545mm diagram in the same OEM brochure, unresolved**. |
| Van | 1598cc G1.6T hybrid 6AT, KR Cargo 5-seat | **245PS system**; ICE 180PS at5500rpm, ICE torque27.0kgf·m 1500–4500rpm; e-motor54kW/304Nm | **2070kg curb** Cargo5 17-inch | Front track1732mm rear1716mm. **245PS hybrid system output cannot be mapped as a single ICE peak engine output without explicit gameplay design.** |

OEM data provenance is separate from CarFight measured mesh geometry. Weight applies only to the shown trim/tire conditions. Numbers not verified for a specific grade remain unknown rather than guessed.

## 3. Current six original Core Body meshes vs chosen public exterior sizes

Actual live 2026-10-13 StaticMeshTools.get_bounds local-space AABB, BEFORE USER edits. Unit cm. The target exterior dims are OEM real vehicle in cm; wheelbase is NOT derivable from the Body AABB.

| Lane | Measured local AABB X×Y×Z (cm) | OEM L×W×H goal (cm) | Review / user handoff |
| --- | --- | --- | --- |
| CityCar | **99.805×54.492×35.352** | **359.5×159.5×148.5** | Actual import scale, shape category (materials suggest armored SUV), forward/pivot review BEFORE editing Wheel Anchors. |
| Compact | **99.805×48.633×29.492** | **398.5×164.5×172.0** | Military-offroad mesh may fit Jimny role but low Z relative to desired height. Check actual mesh aspect ratios before scaling. |
| Coupe | **45.117×99.805×26.367** | **426.5×177.5×131.0** | **Mesh long axis Y, UE vehicle forward is +X**; reorient model/import only after visual verification; existing sockets 0. |
| Pickup | **99.805×43.164×28.320** | **532×188.5×186.5** | Existing X/Y/Z proportions differ from Hilux; do not force nonuniform scaling without intentional shape decision. |
| SubCompact | **99.805×49.414×33.008** | **404×177×158.5** | Both this and CityCar material labels point to armored_suv source; geometry/silhouette distinctness NOT verified. |
| Van | **99.805×45.117×32.227** | **525.5×199.5×200** | Van source appears relatively low vs tall Cargo van, verify pivot and actual geometry/axis first. |

**Known physical UE readiness:** These six Core chassis are all real StaticMesh and all currently have **Socket0** (2026-10-13 fresh managed AssetDump 21/21, detailed Socket evidence). Hence all six are **MeshPreparationRequired**. Prioritize choosing compatible real vehicle / dimensions BEFORE USER changes chassis scale/rotation/wheelhouse or authors the four mandatory Wheel_Anchor_FL/FR/RL/RR sockets. SUV/Sedan/Wagon have existing four Wheel Anchors, leave them undisturbed.

**No blanket mesh scale operation is authorized**: measured local dimensions are not the final component world size; uniform scale to match length would not match all width and height targets, and arbitrary nonuniform scale would distort shapes and wheel arches. A reference car is a benchmark for dimension authoring, not proof the imported fantasy body is a faithful real car model.

## 4. Decision status and exact next action

- **PASS**: sourced six exact chassis real-world L/W/H and wheelbase baselines, and recorded the associated market, grade and other verifiable powertrain/mass/tire facts.
- **USER ACCEPTANCE PENDING**: selected model ↔ imported fantasy mesh silhouette compatibility and desirability. The six are **working P0-02 reference identities**; final production identity/trim approvals still require normal reviewed authoring. The existing temporary body meshes are not identity-fit evidence or a prerequisite to advance research. No Body Geometry or Socket edit is performed here.
- **Current gate state, USER override**: P0-01 class9/9 and declared category roster suffice for Reference-track handoff. Placeholder I-02 wheel/Socket and I-03 source-art silhouette reviews are USER-DEFERRED, not waived for later Product/Runtime integration. **P0-02 research ACTIVE; UE Reference Evidence DataAsset/Production materialization NOT OPENED**. P0-00 Design PASS unchanged.
- **Next**: GPT independently continues P0-02 claim/source/unknown review and P0-03 typed provider work; after existing reviewed-authoring gates and USER later replaces final physical meshes/wheels/sockets, GPT fresh-measures replacement assets for P0-04/P0-06. Do not force a mesh-related USER acceptance now.
- CarFight Core9 chassis scope preserved; no police, sample Offroad or sample SportsCar.

## Fresh reference consistency note — 2026-10-19

- OEM-rechecked L/W/H/WB values across Core6 preserve **24/24 source-supported dimension values**; this is reference research, not a 6-vehicle final Product or accepted persistent Evidence batch.
- Corrected Jimny NEXA manufacturer PDF to accessible Technical Specifications p10 path (`/assets/documents/brochure_30-06/`), matching `VehicleReferenceSourceReview.md v0.1.1`.
- Hyundai Venue 17-inch rear track **1546mm in specs table and 1545mm in diagram** on the same PDF page: keep as unresolved, do not use a single exact `TrackRearMm` FACT for production without source conflict resolution. Exact trim/year remains unfrozen.
- STARIA Cargo is the current 2026-10 KR observed model, not an independently documented **exact MY 2026** claim; its 245PS system rating is distinct from 180PS ICE. GR86 Oman HP/WB sourcing and HiLux max/range mass qualification unchanged.
- Physical placeholder-mesh observations below are historical only and do not become a P0-02 Research Gate. See `VehicleReferenceEvidenceDraft.md v0.1.2` for the current source/Claim/Unknown identity policy.

## Changelog

- 2026-10-19 v0.1.2: Six-vehicle reference source recheck and cross-document consistency; corrected NEXA PDF URL, marked conflicting Venue rear track, clarified STARIA observation year vs exact MY. No UE Asset/code changes.
- 2026-10-13 v0.1.1: USER confirms all inspected meshes are temporary and will be replaced later by USER; source spec research P0-02 proceeds without mesh likeness/size/socket review. Explicitly demoted prior placeholder AABB and size/forward concerns to historical. Corrected Hyundai Venue exact Premium trim not independently supported by brochure, and GR86 wheelbase source to official Toyota Korea. Physical integration remains a later mandatory valid-assets gate.
- 2026-10-13 v0.1.0: Proposed six vehicle identities, manufacturer-verified size and wheelbase, transparent different markets/grade variants, technical specifications as available, original Body AABB mismatch and USER acceptance gate. Documentation only.
