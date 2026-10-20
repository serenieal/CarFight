# CF-FQ-059 — P0-02 Core6 Reference Evidence Draft (Mesh-Independent)

- Version: v0.1.6
- Prepared: 2026-10-19
- Scope: **six existing named vehicle categories** CityCar, Compact, Coupe, Pickup, SubCompact, Van; no extra vehicle class or Police Car.
- Phase: **VCCAS-P0-02 / 2026-10-19 manufacturer source recheck exact24 exterior dimension/wheelbase FACT values PASS (6×4); separate Venue rear track 1545/1546 source-internal discrepancy HOLD; FULL REFERENCE EVIDENCE ACCEPTANCE HOLD**. This is research input/source map, not a saved UE `UCFVehicleRefEvidence`, generated Recipe/Profile/VehicleData, or final P0-02 gate PASS. Detailed review: `VehicleReferenceSourceReview.md v0.1.3`.
- User directive: today's Core6 StaticMeshes are **temporary placeholders to be replaced later**. USER will handle their model dimensions, pivots, Wheel_Anchor sockets, and mesh/wheel authoring later. **Never make their currently missing or poorly scaled placeholders a gate for research/canonical schema work.**
- The six previously selected reference models are **working identities** for research. The user authorized moving on without mesh comparison; this does **not** assert separately confirmed trim/model identity acceptance or physical production fit.
- Prohibited: changing current meshes or their sockets, deriving gameplay WheelRadius/Width from these placeholders, copying Wagon values to new categories, silently using unspecified real-car trim facts, or assigning Production approval/Workbook data prematurely.

## Latest source review (2026-10-19; original baseline 2026-10-13)

**Current claim-level review:** `VehicleReferenceSourceReview.md v0.1.2` directly cross-checks the six OEM source records, 24 individual L/W/H/WB claims, external source/trim/market collisions, and actual `FCFRefClaim`/Evidence contract. `P0-03` source reviewed separately in `VehicleProviderPreReview.md v0.1.0`; do not treat the old pre-review `P0-03 code NOT OPENED` as current implementation success. The six research entities and their published facts can be prepared without placeholder meshes, while identity/trim finalization, user final Evidence acceptance, and durable UE asset creation are still separate work. Cross-market GR86 wheelbase, pickup curb maximum/GVM ranges, STARIA hybrid system vs ICE values, Venue 17-inch trim qualifier must stay explicit.

---

## 1. Working reference identity + recorded source IDs

The identity aliases in this document are **documentation-only research keys**; never treat them as generated GUIDs, UE RecipeId, CCAS ContentId, or a validated final production identity.

| Category / research key | Exact representative working identity (market / grade/year qualifier) | Primary public OEM source key and URL | Related/cross-origin notes |
| --- | --- | --- | --- |
| CityCar / `ref059-citycar-morning27kr` | 2027 Kia Morning **1.0 Gasoline Trendy**, 14-inch, South Korea | `src-kia-morning27-spec` https://www.kia.com/kr/vehicles/morning/specification ; `src-kia-morning27-price` https://www.kia.com/kr/vehicles/morning/price | Kia 2026-09 current grade list; do not use 2-seat Morning Van or 16-inch curb weight. |
| Compact / `ref059-compact-jimny5at-in` | Maruti Suzuki **Jimny 5-door Alpha AT**, Indian NEXA brochure **June 2024** (model year not independently claimed) | `src-nexa-jimny5-brochure` https://www.nexaexperience.com/content/dam/msil/nexa/in/en/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf ; Technical Specification p10 | DO NOT replace with 3-door Jimny; 3985mm length includes rear spare wheel/cover. |
| Coupe / `ref059-coupe-gr86mt26om` | **2026 Toyota GR86 6-speed MT**, Oman distributor model grade | `src-toyota-gr86-2026-oman` https://www.toyotaoman.com/wp-content/uploads/2026/01/Gr-86-specsheet.pdf | 2575mm wheelbase from **Toyota OEM Korea** same-model published spec `src-toyota-gr86-kr-wheelbase` https://www.toyota.co.kr/models/gr86/ ; cross-market dimension corroboration, not an Oman publication claim. Power in Oman **233 HP**, not Korea **231 PS**. Oman exact curb weight and peak torque remain Unknown. |
| Pickup / `ref059-pickup-hiluxsr5at-au` | Toyota **HiLux SR5 4x4 Automatic Double-Cab Pickup V-Active**, Toyota Australia **current range October 2026**; not assigned arbitrary MY | `src-toyota-hiluxsr5-au` https://www.toyota.com.au/hilux/range | Do not blend SR 1855mm width vs SR5 1885mm, or Rogue body height. GVM range is variant/config-dependent. |
| SubCompact / `ref059-subcompact-venue17kr` | Hyundai **Venue Smartstream Gasoline 1.6, IVT, 17-inch**, South Korean Hyundai manufacturer brochure (contains 2025 reference) | `src-hyundai-venue-brochure` https://www.hyundai.com/content/dam/hyundai/kr/ko/data/vehicles/catalog/en/venue-catalog-eng.pdf ; Specs pp19–20 | 17-inch-specific 1585mm (without roof rack), 1215kg; trim **Premium** from earlier plan is not proven by the published brochure alone; identify 17-inch spec rather than falsely claim unique grade. |
| Van / `ref059-van-stariahybridcargo5kr` | Hyundai **The New STARIA Hybrid Cargo 5-seat**, 17-inch, KR manufacturer current October 2026 | `src-hyundai-stariahybridcargo-kr` https://www.hyundai.com/kr/ko/e/vehicles/the-new-staria-hybrid/intro | Cargo 2000mm height, not Tourer 1990mm; 2070kg is Cargo5 17-inch, not Tourer/Lounge. Hybrid system PS vs ICE-only PS must stay distinct. |

## 2. Research claims — public vehicle FACT, native units

**Every numeric item in this table is a real-vehicle reference, NOT a CarFight UE mesh measurement nor a direct UCFVehicleData Runtime value.** Manufacturer specification page URL/source keys are row-scoped references for the listed claims. Values missing for the exact identity are explicitly Unknown, not zero.

| Category | Exterior Length/Width/Height mm | Wheelbase mm | Engine/fuel/displacement | Published power & peak torque | Drive + transmission | Kerb / gross kg | Tyre + track mm (when evidenced) |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| CityCar | **3595 / 1595 / 1485** | **2400** | gasoline 998cc | **76PS@6200rpm**, **9.7kgf·m@3750rpm** | FWD / automatic 4-speed | **975** (14-inch) / **Unknown** gross | 175/65R14; front **1406**, rear **1415** |
| Compact | **3985 / 1645 / 1720** | **2590** | petrol K15B 1462cc | **104.8PS@6000rpm** (77.1kW), **134.2Nm@4000rpm** | selectable ALLGRIP PRO 2H/4H/4L / 4AT | **1205** Alpha AT / **1545** gross | 195/80R15; front **1395**, rear **1405** |
| Coupe | **4265 / 1775 / 1310** | **2575** (Toyota KR official cross-market supporting evidence) | 2.4L gasoline D-4S horizontally opposed four cylinder | **233HP@7000rpm** Oman sheet; **Unknown** exact Oman peak torque | RWD / 6MT | **Unknown** Oman kerb / **Unknown** Oman gross | 215/40R18; **Unknown** exact Oman track (Toyota KR separate market available) |
| Pickup | **5320 / 1885 / 1865** | **3085** | 2.8L 4cyl turbodiesel V-Active | **150kW**, **500Nm**; peak RPM **Unknown** from cited grade page | 4WD / automatic 6-speed | **up to 2155** max kerb, *not exact vehicle kerb* / **3080–3120** GVM range, *not exact selected GVWR* | front **1530**, rear **1555**; tire dimensions not resolved to exact grade |
| SubCompact | **4040 / 1770 / 1585** | **2520** | gasoline Smartstream G1.6 1598cc | **123PS@6300rpm**, **15.7kgf·m@4500rpm** | FWD / IVT | **1215** 17-inch IVT / **Unknown** gross | 205/55R17; front **1535**, rear **1546 in spec table / 1545 in same brochure dimension drawing** (**rear-track claim unresolved; neither scalar is Canonical**) |
| Van | **5255 / 1995 / 2000** | **3275** | 1.6L Smartstream gasoline turbo **hybrid** 1598cc | ICE-only **180PS@5500rpm**, **27.0kgf·m@1500–4500rpm**; motor **54kW, 304Nm**; **system 245PS** | hybrid / 6AT; specific driven-axle declaration **not yet confirmed from this source** | **2070** Cargo5 17-inch / **Unknown** gross | 17-inch tire; front **1732**, rear **1716**; exact tyre aspect ratio **Unknown** |

### Source quality and exact-value restrictions

- **KR Kia Morning**: specification page directly publishes L/W/H/WB, 76PS, 9.7kgf·m, 975kg for 14-inch 4AT; grade page supports Trendy with 175/65R14.
- **Jimny OEM brochure (p10)**: directly publishes all dimensions, 1395/1405mm track, curb mass **Alpha AT 1205**, gross 1545, 195/80R15, K15B 77.1kW/104.8PS/134.2Nm and 5MT/4AT selection. Alpha AT is the selected trim.
- **Oman GR86 2026**: official 2026 distributor sheet directly publishes 4265×1775×1310mm, 233HP@7000, 215/40R18 and 6MT/RWD. Exact Oman **wheelbase absent**; use official KR Toyota 2575mm as independently tagged same-model cross-market dimension, not an Oman claim. Exact Oman curb and torque remain unknown; do **not** import 2019/2025 versions or Korean 231PS/1275kg as exact Oman facts.
- **Australia Toyota Hilux SR5**: current manufacturer webpage explicitly labels SR5 5320×1885×1865mm, 3085mm wheelbase, 150kW/500Nm, 1530/1555mm tracks, max kerb 2155 and GVM range 3080–3120. Never turn a range or maximum into exact selected trim number.
- **Hyundai Venue catalogue (PDF p20 / viewer zero-based p19)**: body L/W/WB, 17-inch H, displacement, 123PS, 15.7kgf·m, exact 17-inch kerb1215kg, front track1535 and 205/55R17. The **same official page contradicts itself for rear track: specification table 1546mm, diagram 1545mm**. Preserve both with exact locators and leave rear track unresolved (do not silently choose 1546/1545). Do not use 15-inch 1180kg/1565mm or rack +25mm values.
- **Hyundai STARIA Cargo**: current manufacturer page explicitly publishes Cargo body size, track, Cargo5 2070kg, hybrid engine and motor values, system 245PS. **System output cannot be copied as peak value into a single combustion-only torque curve**.

## 3. Explicit unknowns and non-source values (guardrails)

All six: `FinalDriveRatio`, complete forward gear-ratio vector, `ChangeUpRPM/ChangeDownRPM` where automatic, `EngineTorqueCurve` full shape, Chaos `WheelRadius/Width` adoption, spring/damper/roll-bar/steering/friction tuning, validated in-game `ReferenceMaxSpeedKmh` and `0-100km/h` are **not established from the row-scoped sources**. These are either future real-world research (when an exact publication exists) or subsequent explicit `CarFightTuned/GAME_BIAS` approved values, **never silently guessed or filled using Wagon**.

Further specific unknowns:
- CityCar: gross vehicle mass and automatic ratio map.
- Compact: whether exact ALLGRIP low range has an adequately modeled Game engine/drivetrain capability. 4AT official exists, but exact ratios not yet sourced.
- Coupe: selected Oman 2026 curb mass / max gross / exact Oman peak torque / cross-market specification conflicts.
- Pickup: exact selected SR5 kerb/GVM and torque peak RPM / exact tyre grade.
- SubCompact: precise sale trim identity with specified 17-inch and exact gross mass; **17-inch rear wheel track 1546mm (table) vs 1545mm (drawing), same Hyundai official brochure p20**, mark `ConflictingUnresolved` until source/variant resolution; IVT model is *not* a step-gear ratio source.
- Van: hybrid system integration into existing CarFight pure-ICE Chaos movement, gross mass/drive architecture, battery, exact tire size, and torque combination. Hybrid engine+motive motor torque must not be arithmetically summed or equated with system output.

## 4. Provenance/ownership contract for next authoring gate

| Proposed future input group | Truth/owner | Can prepare now? | Persist to authoritative UE Runtime? |
| --- | --- | --- | --- |
| Manufacturer vehicle identity, source URL, L/W/H/WB, quoted output/torque, tyre, curb and gross (where known) | `ReferenceSpec` factual claims → existing `UCFVehicleRefEvidence` accepted `FACT` | **YES, six draft claim groups above**; exact citations/units/trim attached | **NO** until accepted typed authoring/creation/admission |
| Game vehicle class/categories: CityCar/Compact/Coupe/Pickup/SubCompact/Van | Project-first-wave content roster, USER-defined | **YES**, independent of provisional imported Body silhouette | After reviewed schema identity / provider |
| Future chassis resource path, Socket transforms, wheel visual geometry, actual WheelRadius/Width | Physical USER owned `MeshMeasured` / AssetReader evidence | **DEFERRED by USER**, may change when final meshes replace temporary placeholders | NO preemptive placeholders or copied Wagon numbers |
| Physics and gameplay speed/steering/suspension, powertrain mapping, mass use in CarFight | Derived + `CarFightTuned` and existing five Profile/Resolver/Approval | Can prepare **explicit proposals only**, not assume real-world FACT equals Chaos runtime | Only after reviewed typed authoring, specific mandatory fields and validators PASS |
| Runtime membership/spawn/driving USER acceptance | Existing persistent VehicleData/Runtime Catalog and vehicle UI | **NO**, blocked by physical assets/final reviewed Product state | Last integration/acceptance gates only |

**Existing authority**, as documented in `VehicleContentAuthoringPlan.md`: `UCFVehicleRefEvidence` owns primary car facts/unknown/conflict; existing Recipe/Profile/Resolver/Apply is exclusive Target VehicleData writer; CCAS `FCFVehicleCCASProvider` read-only schema/import/current/diff **P0-03A/B is already implemented and technically accepted**, but durable reviewed mutation/apply is not. AI R2 creation cannot be enabled by pretending to be SlateUI or Automation. This draft must not create a parallel writer, raw UObject mutation, raw .xlsx edit, or placeholder Product records.

## 5. Scope/gate decision after USER reordering

1. CarFight Core9 chassis candidate inventory and UE AssetClass9/9: **PASS for initial resource identity scope**. Physical models **are explicitly temporary** and are **not a classification or reference-selection constraint**.
2. Previously P0-01 I-02 physical mesh/socket/wheel readiness and I-03 imported silhouette dedupe: **DEFERRED TO PHYSICAL ASSET INTEGRATION**, user-owned when replacement meshes are ready. Preserve exact six old meshes Socket0 as historic state/evidence, **do not call MeshReady or erase missing-field contract**.
3. **P0-02 reference research ACTIVE** in parallel without triggering P0-04 production materialization. Six current working real-car representatives can be researched as exact candidate identities. Final explicit model/trim identity acceptance, unresolved source claims and typed admitted authoring remain future review tasks.
4. **Next work after this source research**: P0-02 Reference Evidence review per claim, optionally reconcile exact model identities/market values, then P0-03 canonical Vehicle schema and trusted UI-less entry according to existing C1–C6; only when those gates legitimately permit write, materialize authoring data. Return to user physical mesh/wheel Socket authoring just before Asset integration/runtime acceptance, NOT before all research.
5. Production/Workbook/Editor asset/Source mutation **NONE** in this checkpoint.

## 6. P0-02 2026-10-19 provenance and evidence consistency recheck

**Scope/result:** six×(ExteriorLengthMm, ExteriorWidthMm, ExteriorHeightMm, WheelbaseMm) numeric reference values retain source-backed exact24/24 PASS; this is **source facts only**, not six constructed or validated `FCFVehicleRefEvidencePayload` objects, persisted DataAssets, accepted exact trim, or P0-04 materialization. Recheck sources are Kia KR 2027 Morning specification/price; Maruti NEXA India June2024 brochure PDF technical specifications last page; Toyota Oman GR86 2026 PDF + **separate Toyota Korea** GR86 specification for wheelbase; Toyota Australia current SR5 range; Hyundai Korea Venue 17-inch catalogue PDF specifications; Hyundai Korea STARIA Hybrid Cargo tab. Other variants/options do not inherit scalar facts.

- **Source registry correction:** Jimny `src-nexa-jimny5-brochure` now references the directly accessible manufacturer PDF `/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf` rather than the failed alternate `/assets/cars/jimny/documents/` URL. STARIA source key normalized to the source-review name `src-hyundai-stariahybridcargo-kr`. This is a documentation-only key correction; no already-created `FCFRefSourceCitation`/`CitationIds` or Evidence fingerprint is migrated.
- **Venue source-internal conflict:** 17-inch rear `TrackRearMm` candidate 1546 (PDF spec table) versus 1545 (same PDF dimension drawing). Preserve both source snippets under the same `SourceId` with separate `Locator`/claim provenance; do not mark either as a Canonical value. The dimension L/W/H/WB 24-claim set is unaffected. Future typed payload should represent unresolved rear track using `FCFRefUnknownFact` Reason=`ConflictingUnresolved` and, if the candidate numeric claims are authored, a `FCFRefConflict` with `SourceDisagreement` and no false resolution. Existing initial payload validator rejects unresolved `Severity=Block` conflicts; do not fabricate a resolution to pass it. Final rear track is deferred from accepted factual scalar until resolved.
- **GR86 cross-market boundary:** Oman 2026 233 HP @7000rpm + 4265/1775/1310; KR Toyota 2575mm wheelbase is supplemental origin, not Oman publication; Korean 231PS/curb/torque cannot become exact Oman MT facts. **HiLux:** 2155kg `MaxKerb` and 3080–3120kg GVM remain qualifier/range, not a new exact `CurbWeightKg`/`GrossWeightKg` scalar. **STARIA:** 245PS is system power; 180PS is ICE only. **Venue:** 17-inch exact sales trim not yet accepted.
- **Evidence formation gate:** six independent payloads, each exact1 Primary, unique/consistent `ReferenceVehicleId`/`SourceId`/`ClaimId`/`UnknownFactId`, FACT claim `CitationIds` resolving to source for **that variant/market** (this market/reference binding requires an explicit additional audit; `ValidateInitialResearchPayload` checks SourceId existence but does not enforce cited Source.ReferenceVehicleIds/MarketRegion/Trim equality per Claim), and at least one approved `Canonical` FACT/DERIVED claim as required by `ValidateInitialResearchPayload`; current input draft research dimension facts are now **23 Canonical + GR86 cross-market wheelbase 1 Candidate**, remaining source claims Candidate; actual UE Validator not run. No new GUID, RecipeId, DataAsset, Profile, Chassis binding or Product mutation. `P0-02` USER final identity/trim/Evidence acceptance remains HOLD.

## 7. Core6 typed Research Payload 입력안 독립 구성 (2026-10-19)

`VehicleCore6EvidencePayloads.md v0.1.0`의 최초 입력안에서 실제 `FCFVehicleRefEvidencePayload` 구조를 기준으로 여섯 개 독립 입력 단위를 작성했다. 현재 Claim ResolutionState는 최신 `VehicleCore6EvidencePayloads.md v0.1.1` 및 이 문서 §8에 따른다. 각 ReferenceVehicleId마다 Primary exact1, Source/Claim/Unknown/Conflict/ResearchNotes가 독립적이다. 전체 **Primary6 / Source8 / FACT Claim84(치수24 포함) / UnknownFact24 / Venue Conflict1**. 현재 문서 조회 결과 ClaimId·SourceId·UnknownFactId unique, claim citation과 searched source가 자기 Payload의 SourceId와 일치, 값 표현 `Integer/Number/CanonicalText/Range` 정적 검사 PASS. Kia Trendy14, Jimny Alpha AT India, GR86 Oman+KR 보조 축거, HiLux SR5 최대값과 GVM 범위, Venue 1545/1546 불일치, STARIA ICE/Hybrid/motor 구분 보존.

**최초 입력안 당시 한계 (Historical):** v0.1.0에서는 연구 FACT 84개 전부 `Candidate`여서 최소 Canonical FACT/DERIVED 1개 조건을 충족하지 못했다. 후속 §8에서 연구 치수 23개를 Canonical로 검토했으나, 실제 `ValidateInitialResearchPayload()` 호출 성공과 USER의 최종 model/trim/source 수락은 아직 입증되지 않았다. 실제 C++ payload/UObject 생성, `ValidateInitialResearchPayload()`·`ApplyInitialResearchPayload()` 실행, Evidence DataAsset Save/Production 등록은 전부 NOT RUN, P0-02 Full Evidence Acceptance HOLD. P0-03A/B 과거 PASS 및 P0-03C 독립 작업과 혼합하지 않는다.

## 8. Conditional dimension research Canonical review — 2026-10-19

`VehicleCore6EvidencePayloads.md v0.1.1`의 84 FACT Claim별 `ResolutionState`를 최신 제조사 근거 범위에 따라 구분했다: **Source-scoped Canonical 23 / Candidate 61**. 수락한 23개는 Morning4, Jimny4, GR86 Oman L/W/H3, HiLux SR5 4(제조사 approximate qualifier 유지), Venue KR 17-inch4, STARIA Hybrid Cargo5 4다. GR86 Toyota KR `WheelbaseMm=2575`는 한국 시장 출처의 same-model 사실이지만 **Oman 2026 6MT exact wheelbase를 직접 입증하지 않으므로 Candidate로 유지**한다. Hyundai Venue 17-inch 후륜윤거 후보2와 Unknown/Conflict는 승격하지 않았다. 다른 60개 엔진/출력/중량/기어/윤거 FACT의 개별 Canonical 판정은 이후 별도 검수다.

**수락 경계:** Canonical은 연구 Claim의 제조사 원문 범위에 대한 내부 검수 판정이다. 모델연식·세부트림 선택을 사용자에게 강제하거나 해당 ReferenceIdentity를 최종 Production 수락한 것이 아니다. 실제 `UCFVehicleRefEvidence::ValidateInitialResearchPayload` 호출과 UE 에셋 저장은 NOT RUN, P0-02 Full Evidence HOLD. Source / Citation / FactKey / ValueKind 및 원본 단위는 기존대로 유지한다. 현재 P0-03C Work lifecycle 불변.

## 9. Core6 initial Research Validator actual C++ verification — 2026-10-19

- `VehicleCore6EvidencePayloads.md v0.1.2`에 실제 `FCFVehicleRefEvidencePayload` 여섯 건을 작성한 `CFVehicleCore6EvidenceTests.cpp v1.0.0`와 UE5.8 집중 검증 결과를 기록했다. 여섯 각자의 Primary1/Source/FACT/Unknown/Conflict를 메모리에서 구현하고 `UCFVehicleRefEvidence::ValidateInitialResearchPayload`를 정상 6종 + 11개 부정 사례로 직접 호출했다. **Normal6/6 PASS, invalid11/11 rejection PASS.**
- 공식 `CarFight_ReEditor` 빌드 job `8cf022cd62ac46cda8510296acfe34f3` exit0. 독립 focused Automation process `2e70e468bcf64aabb66e76e22c4077c2`, exact1 success1/failure0/missing0/unexpected0/duplicate0, isolated JSON SHA256 `389adb6954fa539b4ef2ede8111cec0e3f232b7edd49bc3d3b09cf77db84cf7f` readback PASS.
- 이 PASS는 **existing initial semantic validator 통과**이며 OEM Source의 실제 market/trim/fact 공인 최종 승인도 `ApplyInitialResearchPayload`·SavePackage·Authoring Workbook 작업도 아니다. Venue rear track conflict, GR86 cross-market wheelbase Candidate, HiLux approx/max/range, USER 최종 차량 identity/trim 수락은 그대로 HOLD. 다른 CarFight source writer/GoPyMCP 수정 없음.

## 10. Current Core6 Citation/Secondary binding and reviewed Authoring owner (2026-10-19)

**Latest payload:** `VehicleCore6EvidencePayloads.md v0.1.4`, `VehicleCore6CitationAudit.md v0.1.1`, test fixture `CFVehicleCore6EvidenceTests.cpp v1.1.0`. Morning `clm-1-tirecode` is Candidate citing existing `src-kia-morning27-price`. Oman GR86 remains the exact1 Primary for that payload; separate `ref059-coupe-gr86kr-supp` is KR Secondary, and Korean-only source/2575mm Candidate Claim both belong to it. Canonical23/Candidate61 and all 24 Unknown/one Venue Warning Conflict preserved. This **does not** promote Korean 2575mm to Oman exact Wheelbase.

**Actual changed-payload PASS:** Official UE5.8 Editor build `32768245e29b4dba9d9fdc0d88f5ed5c` exit0 and focused test process `ea72db5bfe2a41ab87a3c8af5729dd3b` exit0 / exact1 success1 failure0 missing0 unexpected0 duplicate0. JSON SHA256 `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3` read back from new EvidenceId result.

**Owner contract:** persistent Evidence is not an isolated file write. Builder Companion preview binds exact valid `NewEvidenceId`, managed Recipe GUID/path and Recipe-bound Target VehicleData path plus initial Research payload/fingerprint, and rejects existing package collisions or attempt to replace Existing Evidence Research. Approved `CreateBuilderCompanions` requires R2 `OwnershipWrite` current scope/staleness checks, updates Recipe and missing Builder-private companions, **marks Dirty but `SavePerformed=false`**, and never authorizes automatic Product publish. All six are **research Input conditionally prepared**, but zero managed Core6 Evidence companions were created/saved or independently approved. USER final year/trim and Production owner binding remain HOLD; existing validator was not modified.

## Changelog

- v0.1.6 / 2026-10-19: Morning Price Citation and GR86 Korea Secondary source/claim correction, official changed fixture v1.1.0 Build + Validator focused PASS, existing Evidence/Recipe/Definition owner-gated companion and Save0 boundary inspected. No persistent DataAsset created.
- v0.1.5 / 2026-10-19: Core6 typed Evidence 6건 정상 및 부정 11건 실제 Validator 호출, 공식 Build/UE exact1 focused Automation PASS 및 독립 JSON readback 근거 연결. 최종 Evidence authoring/acceptance 보류.
- v0.1.4 / 2026-10-19: Core6 24 외형 치수 연구 Claim 원문 조건부 Canonical 판정(23 Canonical, GR86 KR supplemental Wheelbase 1 Candidate)과 전체 84 Claim 상태 연결. Product·Persistent Evidence·UE Validator/Git mutation 없음.
- v0.1.3 / 2026-10-19: 여섯 차량별 실제 Evidence 타입 기반 독립 research payload 입력안 문서 연결, ID·Source·Claim·Unknown·Conflict 정적 검수 PASS 기록. UE Validator/Asset 저장 없음; P0-02 acceptance HOLD 유지.
- v0.1.2 / 2026-10-19: Live manufacturer six-source dimension recheck exact24/24; NEXA broken PDF alias and STARIA SourceId corrected; Venue 17-inch rear-track within-publication contradiction 1545/1546 newly visible; P0-03B read-only Provider status updated. No UE/data edits.
- v0.1.1 / 2026-10-13: Exact24 dimension/wheelbase FACT source verification and actual typed Evidence mapping cross-review linked. P0-02 source review PASS, full acceptance and data persistence HOLD; current placeholder meshes irrelevant to this gate.
- v0.1.0 / 2026-10-13: USER placeholder-mesh deferral applied; one documented sourced reference claim group for each Core6 category, unknown/mixed-market boundaries, future typed authoring separation and active P0-02 research lane. No UE DataAsset, Recipe, profiles or Production mutations.
