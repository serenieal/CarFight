# CF-FQ-059 / P0-02 — Core6 Identity·Citation 수락 검수

- **Version:** v0.1.1
- **Date:** 2026-10-19
- **Scope:** CarFight `main_game` / P0-02 research provenance review, before any persistent Evidence Authoring.
- **Authority latest:** `VehicleCore6EvidencePayloads.md v0.1.4` / `VehicleReferenceSourceReview.md v0.1.3` / `VehicleReferenceEvidenceDraft.md v0.1.6` / `CFVehicleCore6EvidenceTests.cpp v1.1.0`. Original v1.0.0 validator baseline preserved below as historical.
- **Historical tested baseline:** v1.0.0 UE build `8cf022cd62ac46cda8510296acfe34f3`, exact1 Automation `2e70e468bcf64aabb66e76e22c4077c2` PASS, JSON SHA256 `389adb6954fa539b4ef2ede8111cec0e3f232b7edd49bc3d3b09cf77db84cf7f`. **Latest changed v1.1.0:** UE build `32768245e29b4dba9d9fdc0d88f5ed5c` exit0; process `ea72db5bfe2a41ab87a3c8af5729dd3b` exact1 PASS, JSON SHA256 `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3` (§7).
- **Decision (latest):** **84/84 Claim provenance reviewed; Morning Price Citation and GR86 Korea Secondary market owner corrected, changed-payload official UE5.8 build + focused Validator PASS. Research Evidence preparation 6/6 CONDITIONAL READY for owner-contract review, persisted0/6 and P0-04 USER final approval HOLD.**

## 1. Evidence acceptance scope

This audit distinguishes (1) documented real-vehicle identity/trim, (2) direct OEM Source applicability, (3) FACT value with original published units and qualifiers, and (4) authoring authorization. The validated C++ function checks ID/source existence and minimal Canonical, but **does not prove origin scope or required gameplay values**. No new validator, pass gate, host operation, or UE DataAsset is introduced.

- **R** = retain **Research Canonical** only (not user approval or precise in-game physical property).
- **Q** = retain Candidate with qualified provenance. The available source identifies a plausible published field, but this review **does not promote it to Canonical** or claim exact selected options.
- **F** = citation identity/locator or source-market binding **must be corrected before faithful persistent Evidence authoring**. Data numeric value is not necessarily wrong; original C++ test fixture must be updated and revalidated **only if the persisted payload is actually changed**.
- **U** = unresolved contradictory source Claims, both Candidate, no scalar decision.
- A research Evidence DataAsset can preserve Unknown and Warning conflict; ProposalBlock impacts downstream use but is **not** automatically a `ValidateInitialResearchPayload` reject. **Technical validation PASS ≠ approved production writer binding or Save authorization.**

## 2. Official Source registry and ownership

| Vehicle | SourceId | Publisher / title / exact source URL | Actual Source Market/Scope | Verdict |
| --- | --- | --- | --- | --- |
| Kia Morning 2027 Trendy 14 / KR | `src-kia-morning27-spec` | Kia / 2027 Morning specification — https://www.kia.com/kr/vehicles/morning/specification | KR / KR 2027 1.0 Gasoline: dimensions, engine, 14-inch curb, track; `ReferenceVehicleIds=[ref059-citycar-morning27kr]`; `OriginGroupId=KIA_KR`; `OriginIndependence=SameOrigin` | In-scope research (conditions retained) |
| Kia Morning 2027 Trendy 14 / KR | `src-kia-morning27-price` | Kia / 2027 Morning price and trims — https://www.kia.com/kr/vehicles/morning/price | KR / KR Trendy/14-inch tire variant; same manufacturer origin; `ReferenceVehicleIds=[ref059-citycar-morning27kr]`; `OriginGroupId=KIA_KR`; `OriginIndependence=SameOrigin` | In-scope research (conditions retained) |
| Suzuki Jimny 5-door Alpha AT / IN | `src-nexa-jimny5-brochure` | Maruti Suzuki NEXA / Jimny 5-door brochure (published June 2024) — https://www.nexaexperience.com/content/dam/msil/nexa/in/en/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf | IN / technical specifications p10, India Alpha AT; brochure year != vehicle model year; `ReferenceVehicleIds=[ref059-compact-jimny5at-in]`; `OriginGroupId=NEXA_IN`; `OriginIndependence=OriginUnknown` | In-scope research (conditions retained) |
| Toyota GR86 2026 6MT / OM + KR 참고 | `src-toyota-gr86-2026-oman` | Toyota Oman distributor / GR86 2026 specification PDF — https://www.toyotaoman.com/wp-content/uploads/2026/01/Gr-86-specsheet.pdf | OM / OM 2026 6MT L/W/H, 233HP, RWD, 215/40R18; wheelbase absent; `ReferenceVehicleIds=[ref059-coupe-gr86mt26om]`; `OriginGroupId=TOYOTA_OM`; `OriginIndependence=OriginUnknown` | In-scope research (conditions retained) |
| Toyota GR86 2026 6MT / OM + KR 참고 | `src-toyota-gr86-kr-wheelbase` | Toyota Korea / GR86 Korean model page — https://www.toyota.co.kr/models/gr86/ | **KR** same-model wheelbase only; `Source.ReferenceVehicleIds=[ref059-coupe-gr86kr-supp]` Secondary (MarketRegion=KR); OM Primary is unchanged. | **PASS: KR Secondary owner** |
| Toyota HiLux SR5 4x4 AT V-Active / AU | `src-toyota-hiluxsr5-au` | Toyota Australia / Current HiLux SR5 range — https://www.toyota.com.au/hilux/range | AU / AU current SR5 4x4 AT V-Active; 2155 maximum kerb and GVM range 3080–3120 only; `ReferenceVehicleIds=[ref059-pickup-hiluxsr5at-au]`; `OriginGroupId=TOYOTA_AU`; `OriginIndependence=OriginUnknown` | In-scope research (conditions retained) |
| Hyundai Venue 1.6 IVT 17 / KR | `src-hyundai-venue-brochure` | Hyundai Korea / Venue official English catalogue — https://www.hyundai.com/content/dam/hyundai/kr/ko/data/vehicles/catalog/en/venue-catalog-eng.pdf | KR / PDF p20 (viewer p19 zero-based); rear track table 1546 vs dimension drawing 1545; both SAME original source; `ReferenceVehicleIds=[ref059-subcompact-venue17kr]`; `OriginGroupId=HYUNDAI_KR`; `OriginIndependence=OriginUnknown` | In-scope research (conditions retained) |
| Hyundai STARIA Hybrid Cargo5 17 / KR | `src-hyundai-stariahybridcargo-kr` | Hyundai Korea / The New STARIA Hybrid / Cargo official page — https://www.hyundai.com/kr/ko/e/vehicles/the-new-staria-hybrid/intro | KR / KR current page observed 2026-10; Cargo5 17-inch, not Tourer or Lounge. ICE180PS/motor54kW/system245PS separate; `ReferenceVehicleIds=[ref059-van-stariahybridcargo5kr]`; `OriginGroupId=HYUNDAI_KR`; `OriginIndependence=OriginUnknown` | In-scope research (conditions retained) |

### 2.1 Two exact findings

1. **Kia Morning / `clm-1-tirecode` (corrected v1.1.0):** Value `175/65R14` is supported by Kia 2027 **Trendy price/standard equipment** URL `https://www.kia.com/kr/vehicles/morning/price`. Updated Claim cites `src-kia-morning27-price` (previous spec citation was historical), preserving Candidate/model/option qualifiers and same OEM origin group `KIA_KR` (`SameOrigin`).
2. **Toyota GR86 / `clm-3-wheelbasemm` (corrected v1.1.0):** Korea official `https://www.toyota.co.kr/models/gr86/` (more stable primary source PDF `https://www.toyota.co.kr/download/model/GR86_spec.pdf`) publishes `2,575mm`. Oman `https://www.toyotaoman.com/wp-content/uploads/2026/01/Gr-86-specsheet.pdf` publishes L/W/H and MT/AT but not wheelbase. The old KR Source binding to OM Primary was incorrect. Updated C++ adds Secondary `ref059-coupe-gr86kr-supp` (Toyota GR86, MarketRegion=KR, exact MY unknown), binds Korean Source.ReferenceVehicleIds and `clm-3-wheelbasemm.ReferenceVehicleId` to this Secondary; Oman Primary unchanged. Korean wheelbase remains Candidate, not an Oman exact value. **New v1.1.0 changed-payload focused UE5.8 validation PASS.**

**Additional nonblocking scope qualifiers:** Toyota Oman document is headed **GR86 MT/AT** and doesn't freeze a single sale Grade; Toyota AU HiLux calls SR5 dimensions **approximate**, kerb is **maximum** and GVM is a **range**; Hyundai Venue same-publisher p20 table/drawing rear track conflict; STARIA selected Cargo5 17-inch, system245PS versus ICE180PS and motor54kW separate. Jimny NEXA p10 seating capacity is **4** for the 5-door (5 doors does not imply five seats). Publication/observation year alone isn't a precise model year.

## 3. Claim-by-Claim Citation audit (complete 84)

Column `Decision` is audit status, **not a silent change to stored `FCFRefClaim.ResolutionState`**. The current immutable runtime/test baseline remains 23 Canonical + 61 Candidate.

### 3.1 Kia Morning 2027 Trendy 14 / KR

- **ReferenceVehicleId:** `ref059-citycar-morning27kr`; **Market:** KR; **sources** 2, **claims** 14, **UnknownFacts** 3.

| ClaimId / FactKey | Native typed value (UnitId) | CitationId | Current | Audit | Exact source/identity limitation |
| --- | --- | --- | --- | --- | --- |
| `clm-1-exteriorlengthmm` / `ExteriorLengthMm` | `IntegerValue=3595` (`mm`) | `src-kia-morning27-spec` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-1-exteriorwidthmm` / `ExteriorWidthMm` | `IntegerValue=1595` (`mm`) | `src-kia-morning27-spec` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-1-exteriorheightmm` / `ExteriorHeightMm` | `IntegerValue=1485` (`mm`) | `src-kia-morning27-spec` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-1-wheelbasemm` / `WheelbaseMm` | `IntegerValue=2400` (`mm`) | `src-kia-morning27-spec` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-1-enginedisplacementcc` / `EngineDisplacementCc` | `IntegerValue=998` (`cc`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-publishedenginepowerps` / `PublishedEnginePowerPS` | `IntegerValue=76` (`PS`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-publishedenginepowerrpm` / `PublishedEnginePowerRpm` | `IntegerValue=6200` (`rpm`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-publishedenginepeaktorquekgfm` / `PublishedEnginePeakTorqueKgfM` | `NumberValue=9.7` (`kgf_m`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-publishedenginepeaktorquerpm` / `PublishedEnginePeakTorqueRpm` | `IntegerValue=3750` (`rpm`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-curbweightkg` / `CurbWeightKg` | `IntegerValue=975` (`kg`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-trackfrontmm` / `TrackFrontMm` | `IntegerValue=1406` (`mm`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-trackrearmm` / `TrackRearMm` | `IntegerValue=1415` (`mm`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |
| `clm-1-tirecode` / `TireCode` | `TextValue="175/65R14"` (`None`) | `src-kia-morning27-price` | `Candidate` | **Q** | Trendy 14-inch의 직접 원문은 Kia price; 현재 CitationId는 Kia spec로만 연결돼 있어 `src-kia-morning27-price`로 재결속 필요. |
| `clm-1-transmissionkind` / `TransmissionKind` | `TextValue="4AT"` (`None`) | `src-kia-morning27-spec` | `Candidate` | **Q** | Kia 2027 1.0 gasoline 규격의 Candidate; 전후 트림 선택/옵션은 KR 가격·제원 원문 범위 유지. |

**UnknownFacts (all preserved; no default scalar zero):** `GrossWeightKg` (`NotPublished`, `ProposalWarning`); `FinalDriveRatio` (`NotPublished`, `ProposalWarning`); `ForwardGearRatios` (`NotPublished`, `ProposalWarning`).

### 3.2 Suzuki Jimny 5-door Alpha AT / IN

- **ReferenceVehicleId:** `ref059-compact-jimny5at-in`; **Market:** IN; **sources** 1, **claims** 16, **UnknownFacts** 3.

| ClaimId / FactKey | Native typed value (UnitId) | CitationId | Current | Audit | Exact source/identity limitation |
| --- | --- | --- | --- | --- | --- |
| `clm-2-exteriorlengthmm` / `ExteriorLengthMm` | `IntegerValue=3985` (`mm`) | `src-nexa-jimny5-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-2-exteriorwidthmm` / `ExteriorWidthMm` | `IntegerValue=1645` (`mm`) | `src-nexa-jimny5-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-2-exteriorheightmm` / `ExteriorHeightMm` | `IntegerValue=1720` (`mm`) | `src-nexa-jimny5-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-2-wheelbasemm` / `WheelbaseMm` | `IntegerValue=2590` (`mm`) | `src-nexa-jimny5-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-2-enginedisplacementcc` / `EngineDisplacementCc` | `IntegerValue=1462` (`cc`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-publishedenginepowerps` / `PublishedEnginePowerPS` | `NumberValue=104.8` (`PS`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-publishedenginepowerrpm` / `PublishedEnginePowerRpm` | `IntegerValue=6000` (`rpm`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-publishedenginepeaktorquenm` / `PublishedEnginePeakTorqueNm` | `NumberValue=134.2` (`Nm`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-publishedenginepeaktorquerpm` / `PublishedEnginePeakTorqueRpm` | `IntegerValue=4000` (`rpm`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-curbweightkg` / `CurbWeightKg` | `IntegerValue=1205` (`kg`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-grossweightkg` / `GrossWeightKg` | `IntegerValue=1545` (`kg`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-trackfrontmm` / `TrackFrontMm` | `IntegerValue=1395` (`mm`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-trackrearmm` / `TrackRearMm` | `IntegerValue=1405` (`mm`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-unladengroundclearancemm` / `UnladenGroundClearanceMm` | `IntegerValue=210` (`mm`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-tirecode` / `TireCode` | `TextValue="195/80R15"` (`None`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |
| `clm-2-transmissionkind` / `TransmissionKind` | `TextValue="4AT"` (`None`) | `src-nexa-jimny5-brochure` | `Candidate` | **Q** | 인도 5-door Alpha AT/차종 공통 스펙 Candidate; 2024년은 Brochure 발행일일 뿐 MY 아님. |

**UnknownFacts (all preserved; no default scalar zero):** `ExactModelYear` (`NotPublished`, `ProposalWarning`); `FinalDriveRatio` (`NotPublished`, `ProposalWarning`); `ForwardGearRatios` (`NotPublished`, `ProposalWarning`).

### 3.3 Toyota GR86 2026 6MT / OM + KR 참고

- **ReferenceVehicleId:** `ref059-coupe-gr86mt26om`; **Market:** OM; **sources** 2, **claims** 10, **UnknownFacts** 5.

| ClaimId / FactKey | Native typed value (UnitId) | CitationId | Current | Audit | Exact source/identity limitation |
| --- | --- | --- | --- | --- | --- |
| `clm-3-exteriorlengthmm` / `ExteriorLengthMm` | `IntegerValue=4265` (`mm`) | `src-toyota-gr86-2026-oman` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-3-exteriorwidthmm` / `ExteriorWidthMm` | `IntegerValue=1775` (`mm`) | `src-toyota-gr86-2026-oman` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-3-exteriorheightmm` / `ExteriorHeightMm` | `IntegerValue=1310` (`mm`) | `src-toyota-gr86-2026-oman` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-3-wheelbasemm` / `WheelbaseMm` | `IntegerValue=2575` (`mm`) | `src-toyota-gr86-kr-wheelbase` | `Candidate` | **Q** | 한국 KR Secondary에만 연결된 2575mm Candidate, Oman exact wheelbase는 계속 UNKNOWN. |
| `clm-3-enginedisplacementl` / `EngineDisplacementL` | `NumberValue=2.4` (`L`) | `src-toyota-gr86-2026-oman` | `Candidate` | **Q** | 오만 배포사 MT/AT 공통 또는 선택 모델 안내; 정확한 세부 Grade 지정 전 Candidate. 한국 사양 인용 금지. |
| `clm-3-publishedenginepowerhp` / `PublishedEnginePowerHP` | `IntegerValue=233` (`HP`) | `src-toyota-gr86-2026-oman` | `Candidate` | **Q** | 오만 배포사 MT/AT 공통 또는 선택 모델 안내; 정확한 세부 Grade 지정 전 Candidate. 한국 사양 인용 금지. |
| `clm-3-publishedenginepowerrpm` / `PublishedEnginePowerRpm` | `IntegerValue=7000` (`rpm`) | `src-toyota-gr86-2026-oman` | `Candidate` | **Q** | 오만 배포사 MT/AT 공통 또는 선택 모델 안내; 정확한 세부 Grade 지정 전 Candidate. 한국 사양 인용 금지. |
| `clm-3-tirecode` / `TireCode` | `TextValue="215/40R18"` (`None`) | `src-toyota-gr86-2026-oman` | `Candidate` | **Q** | 오만 배포사 MT/AT 공통 또는 선택 모델 안내; 정확한 세부 Grade 지정 전 Candidate. 한국 사양 인용 금지. |
| `clm-3-transmissionkind` / `TransmissionKind` | `TextValue="6MT"` (`None`) | `src-toyota-gr86-2026-oman` | `Candidate` | **Q** | 오만 배포사 MT/AT 공통 또는 선택 모델 안내; 정확한 세부 Grade 지정 전 Candidate. 한국 사양 인용 금지. |
| `clm-3-drivelayout` / `DriveLayout` | `TextValue="RWD"` (`None`) | `src-toyota-gr86-2026-oman` | `Candidate` | **Q** | 오만 배포사 MT/AT 공통 또는 선택 모델 안내; 정확한 세부 Grade 지정 전 Candidate. 한국 사양 인용 금지. |

**UnknownFacts (all preserved; no default scalar zero):** `CurbWeightKg` (`NotPublished`, `ProposalBlock`); `GrossWeightKg` (`NotPublished`, `ProposalWarning`); `PublishedEnginePeakTorqueNm` (`NotPublished`, `ProposalWarning`); `TrackFrontMm` (`NotPublished`, `ProposalWarning`); `TrackRearMm` (`NotPublished`, `ProposalWarning`).

### 3.4 Toyota HiLux SR5 4x4 AT V-Active / AU

- **ReferenceVehicleId:** `ref059-pickup-hiluxsr5at-au`; **Market:** AU; **sources** 1, **claims** 13, **UnknownFacts** 5.

| ClaimId / FactKey | Native typed value (UnitId) | CitationId | Current | Audit | Exact source/identity limitation |
| --- | --- | --- | --- | --- | --- |
| `clm-4-exteriorlengthmm` / `ExteriorLengthMm` | `IntegerValue=5320` (`mm`) | `src-toyota-hiluxsr5-au` | `Canonical` | **R** | 현행 Toyota AU SR5 Dimensions (approximate); SourceValueText/Qualifier 보존한 연구 Canonical. |
| `clm-4-exteriorwidthmm` / `ExteriorWidthMm` | `IntegerValue=1885` (`mm`) | `src-toyota-hiluxsr5-au` | `Canonical` | **R** | 현행 Toyota AU SR5 Dimensions (approximate); SourceValueText/Qualifier 보존한 연구 Canonical. |
| `clm-4-exteriorheightmm` / `ExteriorHeightMm` | `IntegerValue=1865` (`mm`) | `src-toyota-hiluxsr5-au` | `Canonical` | **R** | 현행 Toyota AU SR5 Dimensions (approximate); SourceValueText/Qualifier 보존한 연구 Canonical. |
| `clm-4-wheelbasemm` / `WheelbaseMm` | `IntegerValue=3085` (`mm`) | `src-toyota-hiluxsr5-au` | `Canonical` | **R** | 현행 Toyota AU SR5 Dimensions (approximate); SourceValueText/Qualifier 보존한 연구 Canonical. |
| `clm-4-enginedisplacementl` / `EngineDisplacementL` | `NumberValue=2.8` (`L`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |
| `clm-4-publishedenginepowerkw` / `PublishedEnginePowerKw` | `IntegerValue=150` (`kW`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |
| `clm-4-publishedenginepeaktorquenm` / `PublishedEnginePeakTorqueNm` | `IntegerValue=500` (`Nm`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |
| `clm-4-maxpublishedcurbweightkg` / `MaxPublishedCurbWeightKg` | `IntegerValue=2155` (`kg`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | 제조사 Max/Range 문구 그대로 보존. 선택 한 대의 exact Curb/Gross 값으로 전환 금지. |
| `clm-4-publishedgvmrangekg` / `PublishedGvmRangeKg` | `RangeMinValue=3080, RangeMaxValue=3120` (`kg`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | 제조사 Max/Range 문구 그대로 보존. 선택 한 대의 exact Curb/Gross 값으로 전환 금지. |
| `clm-4-trackfrontmm` / `TrackFrontMm` | `IntegerValue=1530` (`mm`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |
| `clm-4-trackrearmm` / `TrackRearMm` | `IntegerValue=1555` (`mm`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |
| `clm-4-transmissionkind` / `TransmissionKind` | `TextValue="6AT"` (`None`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |
| `clm-4-drivelayout` / `DriveLayout` | `TextValue="4WD"` (`None`) | `src-toyota-hiluxsr5-au` | `Candidate` | **Q** | Toyota AU 현재 SR5 4x4 AT V-Active 한정 Candidate. Exact option·GVM·선택 질량 미지정. |

**UnknownFacts (all preserved; no default scalar zero):** `CurbWeightKg` (`IdentityAmbiguous`, `ProposalBlock`); `GrossWeightKg` (`IdentityAmbiguous`, `ProposalBlock`); `TireCode` (`IdentityAmbiguous`, `ProposalWarning`); `PublishedEnginePeakTorqueRpm` (`NotPublished`, `ProposalWarning`); `ExactModelYear` (`NotPublished`, `ProposalWarning`).

### 3.5 Hyundai Venue 1.6 IVT 17 / KR

- **ReferenceVehicleId:** `ref059-subcompact-venue17kr`; **Market:** KR; **sources** 1, **claims** 15, **UnknownFacts** 4.

| ClaimId / FactKey | Native typed value (UnitId) | CitationId | Current | Audit | Exact source/identity limitation |
| --- | --- | --- | --- | --- | --- |
| `clm-5-exteriorlengthmm` / `ExteriorLengthMm` | `IntegerValue=4040` (`mm`) | `src-hyundai-venue-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-5-exteriorwidthmm` / `ExteriorWidthMm` | `IntegerValue=1770` (`mm`) | `src-hyundai-venue-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-5-exteriorheightmm` / `ExteriorHeightMm` | `IntegerValue=1585` (`mm`) | `src-hyundai-venue-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-5-wheelbasemm` / `WheelbaseMm` | `IntegerValue=2520` (`mm`) | `src-hyundai-venue-brochure` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-5-enginedisplacementcc` / `EngineDisplacementCc` | `IntegerValue=1598` (`cc`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-publishedenginepowerps` / `PublishedEnginePowerPS` | `IntegerValue=123` (`PS`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-publishedenginepowerrpm` / `PublishedEnginePowerRpm` | `IntegerValue=6300` (`rpm`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-publishedenginepeaktorquekgfm` / `PublishedEnginePeakTorqueKgfM` | `NumberValue=15.7` (`kgf_m`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-publishedenginepeaktorquerpm` / `PublishedEnginePeakTorqueRpm` | `IntegerValue=4500` (`rpm`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-curbweightkg` / `CurbWeightKg` | `IntegerValue=1215` (`kg`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-tirecode` / `TireCode` | `TextValue="205/55R17"` (`None`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-trackfrontmm` / `TrackFrontMm` | `IntegerValue=1535` (`mm`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |
| `clm-5-trackrearmm-table` / `TrackRearMm` | `IntegerValue=1546` (`mm`) | `src-hyundai-venue-brochure` | `Candidate` | **U** | 동일 현대 PDF p20 specification table 1546mm; drawing 1545와 불일치, 승격 금지. |
| `clm-5-trackrearmm-drawing` / `TrackRearMm` | `IntegerValue=1545` (`mm`) | `src-hyundai-venue-brochure` | `Candidate` | **U** | 동일 현대 PDF p20 dimension illustration 1545mm; table 1546과 불일치, 승격 금지. |
| `clm-5-transmissionkind` / `TransmissionKind` | `TextValue="IVT"` (`None`) | `src-hyundai-venue-brochure` | `Candidate` | **Q** | 현대 17-inch IVT 제원 범위의 candidate; 정확한 판매 등급·연식 미확정. 15-inch/roof rack 혼용 금지. |

**UnknownFacts (all preserved; no default scalar zero):** `ExactSaleTrim` (`IdentityAmbiguous`, `ProposalBlock`); `ExactModelYear` (`IdentityAmbiguous`, `ProposalWarning`); `GrossWeightKg` (`NotPublished`, `ProposalWarning`); `TrackRearMm` (`ConflictingUnresolved`, `ProposalWarning`).

### 3.6 Hyundai STARIA Hybrid Cargo5 17 / KR

- **ReferenceVehicleId:** `ref059-van-stariahybridcargo5kr`; **Market:** KR; **sources** 1, **claims** 16, **UnknownFacts** 4.

| ClaimId / FactKey | Native typed value (UnitId) | CitationId | Current | Audit | Exact source/identity limitation |
| --- | --- | --- | --- | --- | --- |
| `clm-6-exteriorlengthmm` / `ExteriorLengthMm` | `IntegerValue=5255` (`mm`) | `src-hyundai-stariahybridcargo-kr` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-6-exteriorwidthmm` / `ExteriorWidthMm` | `IntegerValue=1995` (`mm`) | `src-hyundai-stariahybridcargo-kr` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-6-exteriorheightmm` / `ExteriorHeightMm` | `IntegerValue=2000` (`mm`) | `src-hyundai-stariahybridcargo-kr` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-6-wheelbasemm` / `WheelbaseMm` | `IntegerValue=3275` (`mm`) | `src-hyundai-stariahybridcargo-kr` | `Canonical` | **R** | 해당 제조사 원문에 표시된 선택 차형 범위의 research Canonical. 최종 옵션/생산 수락과 구분. |
| `clm-6-enginedisplacementcc` / `EngineDisplacementCc` | `IntegerValue=1598` (`cc`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | 현대 KR current Cargo5 17-inch Candidate. 게시 시점과 OEM ModelYear를 혼동하지 않음. |
| `clm-6-icepublishedpowerps` / `IcePublishedPowerPS` | `IntegerValue=180` (`PS`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-icepublishedpowerrpm` / `IcePublishedPowerRpm` | `IntegerValue=5500` (`rpm`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-icepublishedpeaktorquekgfm` / `IcePublishedPeakTorqueKgfM` | `NumberValue=27.0` (`kgf_m`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-icepeaktorquerpmrange` / `IcePeakTorqueRpmRange` | `RangeMinValue=1500, RangeMaxValue=4500` (`rpm`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-electricmotorpublishedpowerkw` / `ElectricMotorPublishedPowerKw` | `IntegerValue=54` (`kW`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-electricmotorpublishedpeaktorquenm` / `ElectricMotorPublishedPeakTorqueNm` | `IntegerValue=304` (`Nm`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-hybridsystempublishedpowerps` / `HybridSystemPublishedPowerPS` | `IntegerValue=245` (`PS`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | system/ICE/motor의 별도 published 성능. 합산·게임용 ICE 출력 곡선 직접 매핑 금지. |
| `clm-6-curbweightkg` / `CurbWeightKg` | `IntegerValue=2070` (`kg`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | 현대 KR current Cargo5 17-inch Candidate. 게시 시점과 OEM ModelYear를 혼동하지 않음. |
| `clm-6-trackfrontmm` / `TrackFrontMm` | `IntegerValue=1732` (`mm`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | 현대 KR current Cargo5 17-inch Candidate. 게시 시점과 OEM ModelYear를 혼동하지 않음. |
| `clm-6-trackrearmm` / `TrackRearMm` | `IntegerValue=1716` (`mm`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | 현대 KR current Cargo5 17-inch Candidate. 게시 시점과 OEM ModelYear를 혼동하지 않음. |
| `clm-6-transmissionkind` / `TransmissionKind` | `TextValue="6AT"` (`None`) | `src-hyundai-stariahybridcargo-kr` | `Candidate` | **Q** | 현대 KR current Cargo5 17-inch Candidate. 게시 시점과 OEM ModelYear를 혼동하지 않음. |

**UnknownFacts (all preserved; no default scalar zero):** `ExactModelYear` (`NotPublished`, `ProposalWarning`); `GrossWeightKg` (`NotPublished`, `ProposalWarning`); `DriveLayout` (`NotPublished`, `ProposalWarning`); `TireCode` (`NotPublished`, `ProposalWarning`).

## 4. Conflict and blocking semantics

- **Venue:** `clm-5-trackrearmm-table=1546` and `clm-5-trackrearmm-drawing=1545` cite the **same** Hyundai brochure p20. `c-venue-track-rear` stays `SourceDisagreement / Warning / UnresolvedBlock`, two Claim `Candidate`, and `unk-5-trackrearmm=ConflictingUnresolved/ProposalWarning`. It is **not** a resolved `TrackRearMm` Canonical and does not by itself invalidate initial Research payload under the current Validator. Do not artificially escalate Severity=Block or resolve to one measurement to pass a test.
- **GR86:** candidate Korean `WheelbaseMm` must not acquire `Canonical` under Oman selected Primary or be used as Oman exact physical wheel geometry. Exactly one Primary per Research payload remains required; extra reviewed Secondary is allowed by the type but requires explicit corresponding source ownership design and fresh test.
- **HiLux:** `MaxPublishedCurbWeightKg=2155` and `PublishedGvmRangeKg=[3080,3120]` are contextual max/range fields; `CurbWeightKg` and `GrossWeightKg` remain `IdentityAmbiguous / ProposalBlock`. Neither range endpoint is a real selected-trim scalar.
- **Venue:** `ExactSaleTrim=IdentityAmbiguous/ProposalBlock` means this Evidence can describe **17-inch IVT research configuration**, not a named final Premium grade.
- **STARIA:** `ExactModelYear=NotPublished`, `GrossWeightKg=NotPublished`, `DriveLayout=NotPublished`, `TireCode=NotPublished` keep their typed `UnknownFact` entry; system power cannot substitute for ICE engine power.
- **Others:** Jimny `ExactModelYear` from Jun-2024 brochure is unknown; Morning GVM/gears unknown; GR86 Oman exact MT curb/GVM/torque/front-rear track unknown. Preserve all 24 existing UnknownFact IDs, searched source references and blocking status.

## 5. Per-vehicle Research Evidence authoring admission

| Vehicle | Research Identity / Citation Review | **Research Evidence DataAsset authoring preparation** | Binding to Product / save final approval |
| --- | --- | --- | --- |
| Morning KR 2027 Trendy 14 | Exact MY/grade/base tire directly supported, **Kia price Citation remap completed** | **CONDITIONAL READY**, changed typed source exact1 Automation PASS | **HOLD** for authoring owner/binding and user's final acceptance |
| Jimny India 5-door Alpha AT | Publication brochure June2024 + Alpha AT confirmed, **ExactModelYear unknown**, seating capacity4 | **CONDITIONAL READY** when exact MY remains Unknown and owner/binding are reviewed | **HOLD** for owner/binding and user's final acceptance |
| GR86 Oman 2026 MT | OM 2026 MT/AT general spec supports L/W/H; KR Wheelbase Source and Claim now belong to KR Secondary, Oman Primary unchanged | **CONDITIONAL READY** for research companion after owner/Recipe/Target authority admission (Oman exact Wheelbase still unknown) | **HOLD**; no invented OM wheelbase/torque/kerb |
| HiLux Australia current SR5 4x4 AT V-Active | Selected range scope supported; dimensions approximate, mass maximum/range; exact options/MY unknown | **CONDITIONAL READY** with published qualifiers and Unknowns unchanged | **HOLD** for exact production option/mass and authoring owner |
| Venue KR 1.6 IVT 17-inch | Published 17-inch configuration, exact MY/sale grade unknown, same-source rear-track conflict | **CONDITIONAL READY** as **configuration-scoped Research only**, retaining both unresolved Claim/Conflict and `ExactSaleTrim` ProposalBlock | **HOLD** until final grade and unsupported rear-track usage are resolved |
| STARIA KR current Hybrid Cargo5 17 | Cargo5/six-speed/17-inch, published 245PS system vs180PS ICE; exact MY unknown | **CONDITIONAL READY** with CurrentAsObserved and typed output separation | **HOLD** for final MY/owner binding and user approval |

**Readiness arithmetic (latest after citation correction):** **6 CONDITIONAL READY / 0 BLOCKED for researching future companion authoring; permanent asset creation 0/6**. This is **not** 5 saved Evidence DataAssets: **0/6 persisted and 0/6 authorized production VehicleData/Recipe/Workbook writes.** Even for a conditional-ready vehicle, resolve approved EvidenceId / exact TargetRecipeId+paths / ownership, authoring capability and readback/rollback procedure before an actual Save. Inability to fill owner bindings must not become a speculative new generic Host implementation.

## 6. Revalidation boundary and exact next

- **Preserve historical PASS:** Do not rerun existing `Core6PayloadValidation` simply to prove unchanged source IDs, and do not rewrite `ValidateInitialResearchPayload()`. On a changed Claim/Citation/Reference binding, update both `VehicleCore6EvidencePayloads.md` draft and corresponding `CFVehicleCore6EvidenceTests.cpp` fixture in the same semantic unit, recompile/focused validate once and preserve a **new** EvidenceId; no stale PASS for a different input.
- **Do not generate assets yet.** Morning citation and GR86 Secondary owner have been corrected and focused v1.1.0 validator re-run PASS. Existing Builder companion EvidenceId/Recipe/Definition binding/OwnershipWrite were read-only inspected; none of Core6 have accepted managed Recipe/Target binding or approved persistent Save here. For Venue keep `ProposalBlock` visible to downstream production decisions; do not force all unknowns to disappear before documenting research.
- **Work lifecycle:** CarFight Ready/VCCAS-P0-03C unchanged; P0-02 latest audit is source/identity review checkpoint, not a P0-02 final USER acceptance. No Code/Asset/Workbook/GoPyMCP changes in this audit; no stage/commit/push.

## 7. Corrected provenance & existing EvidenceId/Recipe/Definition owner contract (2026-10-19)

**Current authoritative checkpoint:** Source8, Claim84 (research Canonical23, Candidate61), Unknown24, Venue conflict1. The per-Claim correction row classifications are now **R23/Q59/U2/F0** after Morning and GR86 correction. Original §2.1 identification is historical and confirmed resolved above; GR86 Secondary 1 is additional to Core6 Primary6, not a seventh vehicle. Existing custom assertion verifies Claim.CitationIds Source ownership on the 6 payloads.

**Build:** official UE5.8 build job `32768245e29b4dba9d9fdc0d88f5ed5c` exit0, all compiler/link jobs PASS. **Focused Automation:** v1.1.0 exact test `CarFight.CCAS.CF_FQ_059.P0_02.Core6PayloadValidation`, process `ea72db5bfe2a41ab87a3c8af5729dd3b` exit0, EvidenceId `CF059_P002_CiteOwners_20261019_V2`, exact requested1/success1/failure0/missing0/unexpected0/duplicate0, JSON readback SHA-256 `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3`.

**Actual owner contract findings:** No standalone `UCFVehicleRefEvidence` Save contract was proven. Existing `PreviewBuilderCompanions` requires managed persisted Recipe/Target VehicleData, noncolliding Evidence package identity, fixed `NewEvidenceId`, a complete initial Research payload and prospective fingerprint. `CreateBuilderCompanions` requires current OwnershipWrite R2 approval + fresh ProposalHash, Recipe/Definition owner-state checks, and sets `Evidence->EvidenceId`, `TargetRecipeId=Recipe.RecipeId`, `TargetRecipePath`, `TargetDefinitionPath` before Research Apply/Fingerprint verification. It also creates other missing Builder-private profiles and modifies Recipe bindings, marks package Dirty, **SavePerformed=false**, and never automatically saves/Publishes. Existing Evidence is preserved/not overwritten via the new-companion route; duplicate/mismatched RecipeId/paths and stale fingerprint block reuse. All six Core6 Production-managed Recipe/VehicleData authority and user-final grades remain unprovisioned/not accepted, so **persistent Evidence creation and saving remain HOLD 0/6** despite the successful initial Research Validator.

## Changelog

- v0.1.1 / 2026-10-19: Morning Citation and GR86 KR Secondary ownership corrected, new UE5.8 Build + exact1 Validator PASS/JSON readback; six Research payloads conditional for owner review, no persistent Save; exact Builder companion ownership constraints read-only verified.
- v0.1.0 / 2026-10-19: Complete Core6 Claim84→Citation8 audit, identity/year/trim/market/source ownership review and Unknown24/Conflict1 acceptance classification; Morning tire CitationId remap and GR86 KR supplemental Source ownership identified; conditional research authoring 5/6 vs blocked1/6, persistent Save0. Existing validator PASS preserved.
