# CF-FQ-059 — VCCAS-P0-02 Reference Claim Source Review

- Version: v0.1.3
- Review date: 2026-10-19
- Scope: CarFight Core6 reference research independent of temporary Body Mesh (CityCar, Compact, Coupe, Pickup, SubCompact, Van).
- Review basis: current `VehicleReferenceEvidenceDraft.md v0.1.6` + `VehicleCore6EvidencePayloads.md v0.1.4` + `VehicleCore6CitationAudit.md v0.1.1`; actual `UCFVehicleRefEvidence` and `FCFRefClaim/FCFRefSourceCitation/FCFRefUnknownFact` source contract; live manufacturer sources. **No UE Evidence UObject or Product was changed or saved.**
- Verdict: **2026-10-19 six manufacturer sources / exterior L/W/H/Wheelbase 24/24 SOURCE FACT PASS; dimension research Canonical 23/24, GR86 KR cross-market Wheelbase Candidate 1/24**; **Venue 17-inch rear track manufacturer page contradictory 1545/1546 and UNRESOLVED (outside 24 dimension facts)**; *P0-02 data research review PARTIAL PASS*; **full Vehicle Reference Evidence user-final/persisted gate NOT PASSED** due identity/trim choices and explicit unknowns.
- USER priority: current source-art meshes will be replaced later; no mesh identity match, Socket, wheelhouse, Body AABB or temporary geometry dedupe is a prerequisite.

## 1. Direct OEM source trace and normalization

Source URLs below are verified as manufacturer/distributor publications; sources remain separately identified by market and grade. Direct citations are not automatically user-final canonical evidence.

| Research category | Research reference identity | Exterior L/W/H/WB in mm | Primary/source-reviewed evidence | Claim caveat |
| --- | --- | --- | --- | --- |
| CityCar | Kia Morning 2027, KR, gasoline 1.0/14-inch (Trendy working trim) | 3595 / 1595 / 1485 / 2400 | `src-kia-morning27-spec` https://www.kia.com/kr/vehicles/morning/specification (specification, dimensions/engine/14-inch certified mass) | 14/16-inch curb weight differ: 975kg vs1010kg. Working `Trendy` trim mapping needs chosen-option check. |
| Compact | Maruti Suzuki Jimny 5-door Alpha AT, Indian June2024 brochure | 3985 / 1645 / 1720 / 2590 | `src-nexa-jimny5-brochure` https://www.nexaexperience.com/content/dam/msil/nexa/in/en/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf , technical specification page10 | 3985mm length **with spare wheel cover**; 1205kg Alpha AT curb, 1545kg gross. Publication date June2024 is not proof of 2026 model year. |
| Coupe | Toyota GR86 2026 MT, Oman manufacturer-distributor sheet | 4265 / 1775 / 1310 / 2575 | `src-toyota-gr86-2026-oman` https://www.toyotaoman.com/wp-content/uploads/2026/01/Gr-86-specsheet.pdf for L/W/H, RWD/6MT, 233HP; `src-toyota-gr86-kr-wheelbase` https://www.toyota.co.kr/models/gr86/ for independently confirmed 2575mm wheelbase | Oman sheet does not publish Wheelbase; KR page is separate-market evidence, **not same-trim output/mass proof**. Exact Oman MT curb, torque and GVWR remain Unknown. |
| Pickup | Toyota HiLux SR5 4x4 Automatic Double-Cab V-Active, current Toyota Australia | 5320 / 1885 / 1865 / 3085 | `src-toyota-hiluxsr5-au` https://www.toyota.com.au/hilux/range , SR5 row (not SR/Rogue) | 2155kg is **maximum** published kerb weight and 3080–3120kg is **variant GVM range**, not scalar exact car values. No arbitrarily assigned model year. |
| SubCompact | Hyundai Venue KR Smartstream G1.6 IVT / 17-inch variant, exact sale trim not fixed | 4040 / 1770 / 1585 / 2520 | `src-hyundai-venue-brochure` https://www.hyundai.com/content/dam/hyundai/kr/ko/data/vehicles/catalog/en/venue-catalog-eng.pdf ; corroboration `src-hyundai-venue-history` https://www.hyundai.com/kr/ko/brand/brandstory/heritage/2019-venue-qx | Published 17-inch vs15-inch and rack height are variant-specific; **17-inch rear track table 1546mm vs illustration 1545mm within same PDF p20** (not an accepted scalar). **Premium trim and exact MY not supported solely by cited specs**; 2019 Heritage is model-history, not a 2025 MY spec. |
| Van | Hyundai The New STARIA Hybrid Cargo 5-seat 17-inch KR, current page | 5255 / 1995 / 2000 / 3275 | `src-hyundai-stariahybridcargo-kr` https://www.hyundai.com/kr/ko/e/vehicles/the-new-staria-hybrid/intro | Cargo2000mm vs Tourer1990mm; ICE 180PS vs motor54kW vs combined245PS are distinct quantities and cannot be collapsed into one ICE curve. |

### Exact typed data values — minimum atomic claim matrix

For each of the above six research ReferenceVehicle identities, the following **four separate factual claims** are source-supported: `ExteriorLengthMm` / `ExteriorWidthMm` / `ExteriorHeightMm` / `WheelbaseMm`. Each claim requires `FCFRefClaim` fields: `ClaimId` unique within Evidence, `ReferenceVehicleId` exact current identity, `FactKey` stable, `ValueKind=Integer`, `IntegerValue` from the source row, `UnitId=mm`, `SourceValueText` preserving source value/unit, `Provenance=FACT`, `CitationIds` nonempty and resolvable to the **correct source and market**; the GR86 WheelbaseMm specifically cites KR source rather than Oman PDF. `ResolutionState`는 현재 `VehicleCore6EvidencePayloads.md v0.1.1`의 **차량별 검수 결과대로 적용한다 (dimension Canonical 23, cross-market GR86 Wheelbase Candidate 1)**. No sourced wheelbase for Oman: mark provenance as KR supplemental with its own `CitationId`, and do not claim that the Oman sheet printed it.

| Vehicle | ExteriorLengthMm | ExteriorWidthMm | ExteriorHeightMm | WheelbaseMm |
| --- | ---: | ---: | ---: | ---: |
| CityCar Kia Morning KR | 3595 | 1595 | 1485 | 2400 |
| Compact Suzuki Jimny5 Alpha AT India | 3985 | 1645 | 1720 | 2590 |
| Coupe Toyota GR86 MT Oman + KR wheelbase | 4265 | 1775 | 1310 | 2575 |
| Pickup Toyota HiLux SR5 4x4 AT AU | 5320 | 1885 | 1865 | 3085 |
| SubCompact Hyundai Venue 17 IVT KR | 4040 | 1770 | 1585 | 2520 |
| Van Hyundai STARIA Hybrid Cargo5 KR | 5255 | 1995 | 2000 | 3275 |

**Important `FCFRefClaim` validation:** `ValidateInitialResearchPayload` checks exact1 Primary Reference Vehicle per Evidence, unique `ReferenceVehicleId`/`SourceId`/`ClaimId`/`UnknownFactId`, source→reference linkage, at least one resolved Canonical FACT/DERIVED, nonempty CitationIds for FACT, no GAME_BIAS, no unresolved Block conflict. These six rows are **six separate per-vehicle Evidence payloads**, NOT six Primary identities in one Evidence UObject. Current document is research staging, not already validated `FCFVehicleRefEvidencePayload`; mark at least one verified dimension canonical only after final evidence review.

## 2. Explicit incomplete input review (no synthetic defaults)

| Vehicle | Source review shortfall | Blocking only for what? |
| --- | --- | --- |
| CityCar | Exact gross vehicle weight, 4AT forward ratios/final drive, full ICE torque-vs-RPM curve | Future exact engine/vehicle physics authoring; **not P0-02 source L/W/H/WB research** |
| Compact | 4AT ratio/final drive and selectable 2H/4H/4L physical drivetrain gameplay modelling | Future vehicle powertrain/runtime, not external dimensional source |
| Coupe | Oman exact trim mass, exact torque, gross mass; Oman wheelbase cross-origin | Freeze exact Oman physical identity/mass, future drivetrain/engine physics. Toyota KR values must not be silently substituted |
| Pickup | Exact SR5 options, individual curb mass/GVM, tire fitment of selected SR5 and peak torque RPM | BaseMass/Gross Mass and tire/engine mapping in future actual vehicle |
| SubCompact | Exact year/market sale trim for 17-inch variant, gross mass, IVT physical mapping; **official Venue brochure 17-inch rear track 1546 in spec table vs 1545 in adjacent graphic** | Evidence identity finalization, unresolved TrackRearMm source conflict and drivetrain runtime; not an L/W/H/WB claim conflict |
| Van | Hybrid ICE/motor torque combination, gross mass, exact tire aspect size, propulsion/gear architecture | Hybrid-specific runtime/physics validation |

**Source tier policy:** manufacturer or manufacturer-distributor materials are candidate TierA/TierB *subject to existing source-tier contract*; **do not label different countries/grades as independent evidence for the exact same trim**. A `CurrentAsObserved` model year qualifier is appropriate for a live seller page without an exact OEM MY value; only exact 2027 Morning and Oman 2026 PDF are exact MY claimed. No `UnknownFact` creates a numeric `0` placeholder.

## 3. Source-to-Editor authority map

1. Per vehicle source data is held as **research draft** (this doc + `VehicleReferenceEvidenceDraft.md`), scoped to an exact ReferenceVehicle identity, not UE DataAsset truth.
2. Once identity accepted, form a **single** `FCFVehicleRefEvidencePayload` for that vehicle with the exact1 Primary, `FCFRefSourceCitation` records, individual `FCFRefClaim` records, and `FCFRefUnknownFact` for unresolved facts.
3. Existing `UCFVehicleRefEvidence::ValidateInitialResearchPayload()` / `ApplyInitialResearchPayload()` and `FCFVehicleAuthoringService` are reuse contracts. `PreviewBuilderEvidenceRefresh/CommitBuilderEvidenceRefresh` is for already-created existing companion, **not raw creation of arbitrary new Evidence nor Product `VehicleData`**.
4. This research can progress without a Chassis ResourceId. Final `MeshMeasured`/WheelSocket values require USER's later real replacement model and **must not be generated from today's temporary mesh**.
5. **P0-02 exit criterion:** all six current working identity/market/trim decisions reviewed; each accepted Evidence payload has exact1 Primary, field-specific citations, missing facts, no blocker conflict; record consistent provenance; then P0-02 acceptance. This review completes **manufacturer dimension verification** but not final six Evidence UObject acceptance nor saved output.

## 4. Verdict and next

- **PASS:** manufacturer 24 dimension claims (six × four), cited vehicle engine/mass/fuel/drive values only where source supports grade; source/market conflicts identified.
- **HOLD:** exact evidence identity/variant decisions for 17-inch Venue, exact selected SR5 options/curb, exact Oman GR86 mass; associated UnknownFacts remain explicit. None may be silently turned into scalar runtime values.
- **READY TO START P0-03 DESIGN/TEST SPEC:** provider schema/ownership, AI authorization and immutable review path independent of temporary mesh; exact code contracts inspected.
- **NOT RUN:** P0-02 DataAsset authoring, UE build, UE automation, source changes. User did not request new vehicle physical asset mutation; none performed.

## 5. 2026-10-19 fresh manufacturer recheck and typed Evidence integrity

**Observed manufacturer primary references (per existing SourceId):** Kia Morning KR 2027 specification+price confirms 3595/1595/1485/2400 and Trendy 14-inch option; Jimny NEXA India brochure last-page technical specs confirms 3985/1645/1720/2590, 1205kg Alpha AT and 1545kg GVW; Toyota Oman GR86 2026 spec PDF confirms 4265/1775/1310 and 233 HP @7000, with **wheelbase absent**; Toyota KR GR86 published `2575mm` as independently cited cross-market; Toyota Australia HiLux SR5 range confirms 5320/1885/1865/3085, `MaxKerb=2155` and `GVM range 3080–3120`; Hyundai Venue KR brochure confirms 4040/1770/1585(17-inch)/2520; Hyundai STARIA Cargo Hybrid tab confirms 5255/1995/2000/3275 and 2070kg Cargo5 17-inch. **Exact24 exterior dimension/wheelbase values remain source-consistent**; no assertions about exact Oman MT curb/torque, SR5 fixed GVM, system=ICE power, current Venue exact sale trim.

**Citation registry consistency:** Draft v0.1.1 had an inaccessible Jimny URL `/assets/cars/jimny/documents/`; reviewed manufacturer URL is `https://www.nexaexperience.com/content/dam/msil/nexa/in/en/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf` with `SourceId=src-nexa-jimny5-brochure`. Draft also used `src-hyundai-staria-cargo-hybrid` instead of the existing source-review `src-hyundai-stariahybridcargo-kr` for the same Hyundai Cargo page. Draft v0.1.2 has been aligned. This has **no persisted source registry impact** because six payloads/UE Evidence assets are not yet created.

**New documented source-internal inconsistency (Venue, beyond dimension24):** The same Hyundai 17-inch brochure specifications page has rear wheel track **1546mm** in specifications table, **1545mm** in the dimension illustration. The existing research text retained 1546 without acknowledging the conflict. TrackRearMm must remain noncanonical; tag competing text/value with source page/table-vs-drawing locators. On later typed conversion, use `FCFRefUnknownFact` (`ConflictingUnresolved`) and optionally `FCFRefConflict` (`SourceDisagreement`) in that one Venue payload; do not invent a second independent publisher or force a resolved scalar. Initial Research validator requires exactly one Primary, unique IDs, resolvable FACT citations, at least one Canonical FACT/DERIVED, and rejects unresolved Block conflicts; no actual typed `ValidateInitialResearchPayload()` execution took place in this document audit.

**Actual Validator scope caveat:** `UCFVehicleRefEvidence::ValidateInitialResearchPayload()` presently checks `Source.ReferenceVehicleIds` entries against existing Reference IDs and checks every FACT `CitationId` for existence in the payload's SourceId set. It **does not cross-check each cited Source's ReferenceVehicleIds/MarketRegion/Trim against the citing Claim's ReferenceVehicleId/variant**, so passing this validator alone cannot certify cross-market/source-owner linkage. For each of the six eventual payloads, require a **readback/audit of ClaimId → CitationIds → SourceId → Source.ReferenceVehicleIds + market/variant/Locator** before claiming provenance acceptance. Especially the GR86 KR wheelbase citation must be explicitly marked supplemental to the Oman reference identity. No validator code change in this audit.

**Bounded status:** manufacturer value recheck 24/24; source URL/SourceId consistency review corrected; one track scalar unresolved; each vehicle remains independent research `Candidate` (not approved canonical trim identity); no claim-level typed payload, `UCFVehicleRefEvidence` DataAsset, source fingerprint/RecipeId, vehicle actor, Workbook, Product publication, or new Editor build/Automation performed. P0-02 full acceptance remains HOLD. Current P0-03A/B read-only Provider already implemented and historical PASS, not a pending implementation prerequisite.

## 6. Source-scoped Canonical FACT 판정 — 2026-10-19

**원문 재확인 및 research claim 판정:** Kia KR 모닝 제원·트렌디 기본 14인치 옵션, NEXA 인도 짐니5 브로슈어 p10, Toyota Oman 2026 GR86 PDF 원문 검색 결과(원문 PDF viewer 접근 제한은 별도 남김), Toyota Korea GR86 공식 spec PDF, Toyota Australia HiLux SR5 4x4 AT V-Active 공개 사양, Hyundai KR Venue 브로슈어 p20, Hyundai KR STARIA Hybrid Cargo5 제원 페이지를 교차 조회했다. 기존 6×4 원문 FACT 24개는 출처 존재·값 확인 기준 PASS.

| Reference 연구 scope | L / W / H / Wheelbase 수락 상태 | 정확한 한계 |
| --- | --- | --- |
| Morning KR 2027 1.0 Trendy 기본14 | Canonical 4/4 | Kia spec+price 동일 제조사 계열; Trendy 14-inch 기본 tire, 16-inch 옵션 질량 섞지 않음 |
| Jimny India 5-door Alpha AT | Canonical 4/4 | 2024-06 brochure 발행월은 exact MY 아님. L3985는 뒤 spare cover 포함 |
| GR86 Oman 2026 6MT | Canonical 3/4 | L4265/W1775/H1310 Oman 공식 distributor sheet; **Wheelbase2575는 KR Toyota 별도 시장 자료만 있으므로 Candidate** |
| HiLux AU SR5 4x4 AT V-Active | Canonical 4/4 (approx. qualified) | Toyota가 `Dimensions (approximate)`라고 출처에서 직접 한정함. SR5 options별 exact 물리치수/질량으로 확대 금지 |
| Venue KR 17-inch IVT | Canonical 4/4 | 17-inch H1585 without rack; 후륜 윤거1546 table/1545 drawing은 별도 Candidate2/Unknown/Warning conflict |
| STARIA KR Hybrid Cargo5 17-inch | Canonical 4/4 | Cargo H2000 not Tourer H1990, current published page가 exact MY를 증명하지 않음 |

**판정 정의:** `Canonical`은 검수한 **제조사 원문 범위에서 해당 연구 Claim의 사실성**에 관한 선택이며, 모델연식·트림·옵션에 대한 USER 최종 수락/완성 차량/P0-02 완료 판정이 아니다. 다른 60개 출력·중량·윤거 등 FACT는 그대로 Candidate. `GR86 KR Wheelbase`는 이 교차시장 동일성 판단이 풀리기 전 오만 Canonical로 승격하지 않는다. 공식 UE `ValidateInitialResearchPayload()` 자체는 실행하지 않았다. 저장된 Evidence UObject도 없다.

**Source origin과 Validator 한계:** `FCFRefSourceCitation.ReferenceVehicleIds`의 ID 존재와 `Claim.CitationIds`의 SourceId 존재는 Validator가 검사하지만, **시장·트림과 Claim 간 실제 원문 일치**는 별도 근거 확인이 필요하다. 본 document-only 검수는 provenance scope에서 확인된 치수 Claim만 23개 수락했고, Product·UE Build/Automation은 변경하지 않는다. P0-02 Full Evidence HOLD.

## 7. Corrected Source reference ownership & reviewed typed acceptance (2026-10-19)

- Morning 2027 1.0 Trendy 14-inch `clm-1-tirecode` now cites the directly supporting Kia KR price/trim Source `src-kia-morning27-price` instead of the broad dimensions source. Both OEM links retain one `KIA_KR` origin group.
- GR86 Oman source `src-toyota-gr86-2026-oman` owns OM Primary `ref059-coupe-gr86mt26om`. The Korean Toyota Source `src-toyota-gr86-kr-wheelbase` and Wheelbase Candidate `clm-3-wheelbasemm=2575` now own *only* `ref059-coupe-gr86kr-supp` Secondary (MarketRegion=KR, exact MY unknown), rather than claiming Oman Primary applicability. This does not solve unknown exact Oman wheelbase.
- `CFVehicleCore6EvidenceTests.cpp v1.1.0` verified 6 payloads and Claim-to-Citation-Source Reference ownership. UE5.8 Editor Build `32768245e29b4dba9d9fdc0d88f5ed5c` PASS, focused exact1 Automation `ea72db5bfe2a41ab87a3c8af5729dd3b` PASS. JSON SHA `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3` readback.
- OEM source market/grade semantics remain a **human-reviewed research provenance** matter; the generic `ValidateInitialResearchPayload` itself was not extended. Persistent `UCFVehicleRefEvidence` still requires existing Builder managed Recipe+Target+EvidenceId/path, R2 OwnershipWrite, separate Save/readback and USER final scope approval. No such new DataAsset persisted here.

## Changelog

- v0.1.3 / 2026-10-19: Source owner correction for Morning direct-price and GR86 KR Secondary with successful changed-payload UE validator receipt. Production evidence HOLD.
- v0.1.2 / 2026-10-19: 원문 기반 23개 dimension 연구 FACT Canonical, GR86 KR cross-market Wheelbase 1 Candidate 판정; Toyota AU SR5 `approximate`, Venue rear-track 충돌·연식/트림/원본 분리 보존. Validator/UE 저장 NOT RUN.
- v0.1.1 / 2026-10-19: Fresh six manufacturer-source recheck exact24/24, Jimny URL/STARIA SourceId reconciliation, Venue brochure 1545/1546 rear track conflict tracked, current typed Evidence/provider boundary updated. Documentation only.
- v0.1.0: Six manufacturer source cross-review with exact claim fields and provenance; P0-03 input acceptance boundaries and physical deferral preserved.
