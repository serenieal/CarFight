# CF-FQ-059 — Core6 FCFVehicleRefEvidencePayload 연구 입력안

- Version: **v0.1.4**
- Date: 2026-10-19
- Scope: Core6 6개 독립 Reference Evidence research payload; P0-02
- Status: **RESEARCH FACT 23 CANONICAL/61 CANDIDATE; Morning Citation + GR86 KR Secondary OWNER CORRECTED, actual new UE5.8 Build PASS + Core6 exact1 Validator PASS, persistent Evidence approval HOLD**
- Authority: CarFight `main_game`; source: `VehicleReferenceEvidenceDraft.md v0.1.6`, `VehicleReferenceSourceReview.md v0.1.3`, `VehicleRepresentativeSpecRoster.md v0.1.2`, 실제 `CFVehicleRefEvidence.h/.cpp v1.3.0`.

## 1. 정확한 구조와 공통 필드 규약

여섯 절은 **서로 다른 `FCFVehicleRefEvidencePayload` 여섯 개**를 뜻한다. 각 절의 `ReferenceVehicles=[exact1 Primary; GR86 additionally has one KR Secondary]`, `Sources`, `Claims`, `Conflicts`, `UnknownFacts`, `ResearchNotes`만 사용한다. **Payload에는 `EvidenceId`, `TargetRecipeId`, Recipe/Definition 경로, Fingerprint가 없다.** 이들은 승인 후 기존 Evidence owner가 결속하며 지금 생성하지 않는다.

- 각 절의 **ReferenceVehicleId**는 기존 `ref059-*` 연구 key를 그대로 `FName`으로 쓴다. `ReferenceVehicles[0].Role=Primary`, `IdentityConfidence=0.0(미평가)`. 누락된 Generation/정확한 Trim/DriveLayout 등은 실제 객체에서 `FString()`으로 유지(표의 `UNCONFIRMED`는 객체 literal 입력값이 아님). `ModelYearQualifier=Exact`만 `ModelYearStart/End`에 해당 연도를 넣고 `Unknown`/`CurrentAsObserved`는 `0/0`으로 유지한다.
- `Sources` 각 행은 `FCFRefSourceCitation`에 대응한다. SourceId/FName, `Tier`(TierA 제조사 직접, TierB 오만 판매사 후보 — 최종 Tier 승인 전), `SourceKind` FName, `Publisher`, `DocumentTitle`, `CanonicalUrl`, `Locator`, `ReferenceVehicleIds=[해당 절의 ReferenceVehicleId]`, `OriginGroupId`, `OriginIndependence`, `SourceScopeNote`를 행에서 채운다. 근거 없는 `PublishedDate/UpdatedDate`는 비운다. 다른 국가가 같은 Primary와 **정확한 트림이라고 자동 주장하지 않는다**.
- 각 `Claims` 행은 **`FCFRefClaim` 1개**다. 컬럼 순서는 `ClaimId | FactKey | ValueKind:실제 값 필드 | UnitId | CitationIds | SourceValueText 및 제한 | ResolutionState`이다. 모든 Claim에 `ReferenceVehicleId=해당 Primary`, `Provenance=FACT`, `ResolutionState`는 **각 Claim 행의 판정 컬럼**, `ConfidenceScore=0.0(미평가)`, `InputClaimIds=[]`, `MethodId=None`, `MethodRevision=0`, `ConflictId=None` 기본값을 적용한다. `Integer`→`IntegerValue`, `Number`→`NumberValue`, `Range`→`RangeMinValue/RangeMaxValue`, `CanonicalText`→`TextValue`. 미사용 슬롯의 기본 0은 사실값 0이 아니다. `UnitId=None`은 FName None, `kgf_m`은 원문 kgf·m 단위, PS/HP/kW/Nm을 묵시 변환하지 않는다.
- 각 `UnknownFacts` 행은 `FCFRefUnknownFact` 1개다. `UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note` 순서. 미발표/구성 미확정과 현실의 미존재는 다르다. 실제 물리 Mesh·Socket·게임용 EngineCurve·Chaos tuning은 이번 Reference Payload의 FACT도 아니고 승인 없이 채우지 않는다.
- `Conflicts=[]`은 이 연구 입력안에서 동종 수치의 알려진 충돌이 아직 없다는 뜻이지 외부 출처 전수 검증 보증이 아니다. Venue의 후륜 윤거는 **동일 OEM 문서 내부에서 1546 대 1545**, Candidate2 + Unknown1 + Conflict1, Canonical0이다.
- **실제 Validator 게이트:** 현재 `ValidateInitialResearchPayload`는 exact1 Primary, unique Source/Claim/Unknown/Conflict ID, ReferenceVehicleId/SourceId 존재, FACT CitationIds, GAME_BIAS 금지, unresolved Block conflict 금지와 **최소 한 개 Canonical FACT/DERIVED**를 요구한다. 이번 Source 범위 검수 결과 **치수 23개를 연구용 `Canonical`로 선정했고 나머지 61개는 Candidate**로 보존했다. 각 Payload의 Canonical-count 조건은 충족할 가능성이 있지만 실제 Validator 검사 및 최종 모델/트림·출처 수락은 NOT RUN/HOLD다. Claim-to-Source market/trim 교차검사는 추가 감사 대상이다. 실제 Validator는 claim마다 cited source의 차종·시장·트림이 일치하는지 자동으로 검사하지 않는다.
- Dimension 4개 `ExteriorLengthMm/ExteriorWidthMm/ExteriorHeightMm/WheelbaseMm`는 현재 `FCFVehicleCCASProvider`가 식별하는 연구 FactKey. 다른 출력/질량/장비 FactKey는 Evidence 연구 레코드이지 지금 `UCFVehicleData`에 바로 쓰는 runtime 필드가 아니다.

## 2. Core6 독립 입력안

### 2.1 CityCar / Kia Morning 2027 KR Trendy 14-inch

**`ReferenceVehicles[0]`**: `ReferenceVehicleId=ref059-citycar-morning27kr`; `Role=Primary`; `IdentityConfidence=0.0`; Manufacturer=Kia; Model=Morning; Generation=UNCONFIRMED; ModelYearQualifier=Exact; ModelYearStart=2027; ModelYearEnd=2027; Trim=Trendy (working, 14-inch); Powertrain=Gasoline 998cc; Transmission=4AT; DriveLayout=FWD; MarketRegion=KR; BodyVariant=CityCar passenger; WheelTireVariant=175/65R14. **Actual UNCONFIRMED field values are empty FString.**

**`Sources`**

| SourceId | Tier / SourceKind | Publisher / DocumentTitle | CanonicalUrl | Locator / scope / source reference |
| --- | --- | --- | --- | --- |
| `src-kia-morning27-spec` | `TierA` / `ManufacturerSpecifications` | Kia / 2027 Morning specification | https://www.kia.com/kr/vehicles/morning/specification | KR 2027 1.0 Gasoline: dimensions, engine, 14-inch curb, track; `ReferenceVehicleIds=[ref059-citycar-morning27kr]`; `OriginGroupId=KIA_KR`; `OriginIndependence=SameOrigin` |
| `src-kia-morning27-price` | `TierA` / `ManufacturerPrice` | Kia / 2027 Morning price and trims | https://www.kia.com/kr/vehicles/morning/price | KR Trendy/14-inch tire variant; same manufacturer origin; `ReferenceVehicleIds=[ref059-citycar-morning27kr]`; `OriginGroupId=KIA_KR`; `OriginIndependence=SameOrigin` |

**`Claims`: FACT (Dimension 4 Canonical, 10 Candidate)**

| ClaimId | FactKey | ValueKind + typed field | UnitId | CitationIds[0] | SourceValueText / 제한 | ResolutionState |
| --- | --- | --- | --- | --- | --- |
| `clm-1-exteriorlengthmm` | `ExteriorLengthMm` | `Integer`: `IntegerValue=3595` | `mm` | `src-kia-morning27-spec` | 3595mm | `Canonical` |
| `clm-1-exteriorwidthmm` | `ExteriorWidthMm` | `Integer`: `IntegerValue=1595` | `mm` | `src-kia-morning27-spec` | 1595mm | `Canonical` |
| `clm-1-exteriorheightmm` | `ExteriorHeightMm` | `Integer`: `IntegerValue=1485` | `mm` | `src-kia-morning27-spec` | 1485mm | `Canonical` |
| `clm-1-wheelbasemm` | `WheelbaseMm` | `Integer`: `IntegerValue=2400` | `mm` | `src-kia-morning27-spec` | 2400mm | `Canonical` |
| `clm-1-enginedisplacementcc` | `EngineDisplacementCc` | `Integer`: `IntegerValue=998` | `cc` | `src-kia-morning27-spec` | 998cc | `Candidate` |
| `clm-1-publishedenginepowerps` | `PublishedEnginePowerPS` | `Integer`: `IntegerValue=76` | `PS` | `src-kia-morning27-spec` | 76PS | `Candidate` |
| `clm-1-publishedenginepowerrpm` | `PublishedEnginePowerRpm` | `Integer`: `IntegerValue=6200` | `rpm` | `src-kia-morning27-spec` | 6200rpm | `Candidate` |
| `clm-1-publishedenginepeaktorquekgfm` | `PublishedEnginePeakTorqueKgfM` | `Number`: `NumberValue=9.7` | `kgf_m` | `src-kia-morning27-spec` | 9.7kgf·m | `Candidate` |
| `clm-1-publishedenginepeaktorquerpm` | `PublishedEnginePeakTorqueRpm` | `Integer`: `IntegerValue=3750` | `rpm` | `src-kia-morning27-spec` | 3750rpm | `Candidate` |
| `clm-1-curbweightkg` | `CurbWeightKg` | `Integer`: `IntegerValue=975` | `kg` | `src-kia-morning27-spec` | 975kg; 14-inch 4AT only | `Candidate` |
| `clm-1-trackfrontmm` | `TrackFrontMm` | `Integer`: `IntegerValue=1406` | `mm` | `src-kia-morning27-spec` | 1406mm; 14-inch | `Candidate` |
| `clm-1-trackrearmm` | `TrackRearMm` | `Integer`: `IntegerValue=1415` | `mm` | `src-kia-morning27-spec` | 1415mm; 14-inch | `Candidate` |
| `clm-1-tirecode` | `TireCode` | `CanonicalText`: `TextValue="175/65R14"` | `None` | `src-kia-morning27-price` | Trendy 14-inch 175/65R14 | `Candidate` |
| `clm-1-transmissionkind` | `TransmissionKind` | `CanonicalText`: `TextValue="4AT"` | `None` | `src-kia-morning27-spec` | automatic 4-speed | `Candidate` |

**`UnknownFacts`**

| UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note |
| --- | --- | --- | --- | --- | --- |
| `unk-1-grossweightkg` | `GrossWeightKg` | `NotPublished` | `ProposalWarning` | `[src-kia-morning27-spec]` | exact KR 14-inch gross unknown |
| `unk-1-finaldriveratio` | `FinalDriveRatio` | `NotPublished` | `ProposalWarning` | `[src-kia-morning27-spec]` | selected 4AT final drive not sourced |
| `unk-1-forwardgearratios` | `ForwardGearRatios` | `NotPublished` | `ProposalWarning` | `[src-kia-morning27-spec]` | selected 4AT ratios not sourced |

**`Conflicts=[]`** (현재 조사된 사실의 확인된 동종 충돌 없음). **`ResearchNotes`**: 현재 실제 판매 연식 2027로 확인된 Morning 1.0과 14인치 조합. Kia Specification/Price는 동일 원자료 계열; 독립 OEM 둘로 집계하지 않음. Trendy/14-inch 최종 정확한 옵션 승인 전 identity HOLD.

**정적 검수:** Primary1 / Source IDs 2 / FACT 14 / dimension4 / Unknown 3 / Source citation 전부 인스턴스 내부 / ClaimId unique. Dimension research Canonical 검수 반영. UE Validator NOT RUN / final Evidence HOLD.

### 2.2 Compact / Suzuki Jimny 5-door Alpha AT India

**`ReferenceVehicles[0]`**: `ReferenceVehicleId=ref059-compact-jimny5at-in`; `Role=Primary`; `IdentityConfidence=0.0`; Manufacturer=Maruti Suzuki; Model=Jimny 5-door; Generation=UNCONFIRMED; ModelYearQualifier=Unknown; ModelYearStart=0; ModelYearEnd=0; Trim=Alpha AT; Powertrain=K15B petrol 1462cc; Transmission=4AT; DriveLayout=ALLGRIP PRO 2H/4H/4L; MarketRegion=IN; BodyVariant=5-door with rear spare-wheel cover; WheelTireVariant=195/80R15. **Actual UNCONFIRMED field values are empty FString.**

**`Sources`**

| SourceId | Tier / SourceKind | Publisher / DocumentTitle | CanonicalUrl | Locator / scope / source reference |
| --- | --- | --- | --- | --- |
| `src-nexa-jimny5-brochure` | `TierA` / `ManufacturerBrochure` | Maruti Suzuki NEXA / Jimny 5-door brochure (published June 2024) | https://www.nexaexperience.com/content/dam/msil/nexa/in/en/assets/documents/brochure_30-06/Nexa-Jimny-Brochure.pdf | technical specifications p10, India Alpha AT; brochure year != vehicle model year; `ReferenceVehicleIds=[ref059-compact-jimny5at-in]`; `OriginGroupId=NEXA_IN`; `OriginIndependence=OriginUnknown` |

**`Claims`: FACT (Dimension 4 Canonical, 12 Candidate)**

| ClaimId | FactKey | ValueKind + typed field | UnitId | CitationIds[0] | SourceValueText / 제한 | ResolutionState |
| --- | --- | --- | --- | --- | --- |
| `clm-2-exteriorlengthmm` | `ExteriorLengthMm` | `Integer`: `IntegerValue=3985` | `mm` | `src-nexa-jimny5-brochure` | 3985mm | `Canonical` |
| `clm-2-exteriorwidthmm` | `ExteriorWidthMm` | `Integer`: `IntegerValue=1645` | `mm` | `src-nexa-jimny5-brochure` | 1645mm | `Canonical` |
| `clm-2-exteriorheightmm` | `ExteriorHeightMm` | `Integer`: `IntegerValue=1720` | `mm` | `src-nexa-jimny5-brochure` | 1720mm | `Canonical` |
| `clm-2-wheelbasemm` | `WheelbaseMm` | `Integer`: `IntegerValue=2590` | `mm` | `src-nexa-jimny5-brochure` | 2590mm | `Canonical` |
| `clm-2-enginedisplacementcc` | `EngineDisplacementCc` | `Integer`: `IntegerValue=1462` | `cc` | `src-nexa-jimny5-brochure` | 1462cc K15B | `Candidate` |
| `clm-2-publishedenginepowerps` | `PublishedEnginePowerPS` | `Number`: `NumberValue=104.8` | `PS` | `src-nexa-jimny5-brochure` | 104.8PS@6000rpm | `Candidate` |
| `clm-2-publishedenginepowerrpm` | `PublishedEnginePowerRpm` | `Integer`: `IntegerValue=6000` | `rpm` | `src-nexa-jimny5-brochure` | 6000rpm | `Candidate` |
| `clm-2-publishedenginepeaktorquenm` | `PublishedEnginePeakTorqueNm` | `Number`: `NumberValue=134.2` | `Nm` | `src-nexa-jimny5-brochure` | 134.2Nm@4000rpm | `Candidate` |
| `clm-2-publishedenginepeaktorquerpm` | `PublishedEnginePeakTorqueRpm` | `Integer`: `IntegerValue=4000` | `rpm` | `src-nexa-jimny5-brochure` | 4000rpm | `Candidate` |
| `clm-2-curbweightkg` | `CurbWeightKg` | `Integer`: `IntegerValue=1205` | `kg` | `src-nexa-jimny5-brochure` | Alpha AT 1205kg | `Candidate` |
| `clm-2-grossweightkg` | `GrossWeightKg` | `Integer`: `IntegerValue=1545` | `kg` | `src-nexa-jimny5-brochure` | Alpha AT gross 1545kg | `Candidate` |
| `clm-2-trackfrontmm` | `TrackFrontMm` | `Integer`: `IntegerValue=1395` | `mm` | `src-nexa-jimny5-brochure` | 1395mm | `Candidate` |
| `clm-2-trackrearmm` | `TrackRearMm` | `Integer`: `IntegerValue=1405` | `mm` | `src-nexa-jimny5-brochure` | 1405mm | `Candidate` |
| `clm-2-unladengroundclearancemm` | `UnladenGroundClearanceMm` | `Integer`: `IntegerValue=210` | `mm` | `src-nexa-jimny5-brochure` | unladen 210mm | `Candidate` |
| `clm-2-tirecode` | `TireCode` | `CanonicalText`: `TextValue="195/80R15"` | `None` | `src-nexa-jimny5-brochure` | 195/80R15 | `Candidate` |
| `clm-2-transmissionkind` | `TransmissionKind` | `CanonicalText`: `TextValue="4AT"` | `None` | `src-nexa-jimny5-brochure` | 4AT | `Candidate` |

**`UnknownFacts`**

| UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note |
| --- | --- | --- | --- | --- | --- |
| `unk-2-exactmodelyear` | `ExactModelYear` | `NotPublished` | `ProposalWarning` | `[src-nexa-jimny5-brochure]` | 2024 brochure publication != exact selected model-year |
| `unk-2-finaldriveratio` | `FinalDriveRatio` | `NotPublished` | `ProposalWarning` | `[src-nexa-jimny5-brochure]` | exact India AT final drive not yet sourced |
| `unk-2-forwardgearratios` | `ForwardGearRatios` | `NotPublished` | `ProposalWarning` | `[src-nexa-jimny5-brochure]` | AT 4 gear ratios not yet sourced |

**`Conflicts=[]`** (현재 조사된 사실의 확인된 동종 충돌 없음). **`ResearchNotes`**: 인도 5-door Alpha AT 단일 후보. 3-door·MT 값을 섞지 않는다. ALLGRIP 2H/4H/4L 출처 사실은 게임 drivetrain 구현이 아님.

**정적 검수:** Primary1 / Source IDs 1 / FACT 16 / dimension4 / Unknown 3 / Source citation 전부 인스턴스 내부 / ClaimId unique. Dimension research Canonical 검수 반영. UE Validator NOT RUN / final Evidence HOLD.

### 2.3 Coupe / Toyota GR86 2026 Oman MT + KR supplemental wheelbase

**`ReferenceVehicles[0]`**: `ReferenceVehicleId=ref059-coupe-gr86mt26om`; `Role=Primary`; `IdentityConfidence=0.0`; Manufacturer=Toyota; Model=GR86; Generation=UNCONFIRMED; ModelYearQualifier=Exact; ModelYearStart=2026; ModelYearEnd=2026; Trim=Oman 6MT grade UNCONFIRMED; Powertrain=2.4L Boxer D-4S gasoline; Transmission=6MT; DriveLayout=RWD; MarketRegion=OM; BodyVariant=Coupe; WheelTireVariant=215/40R18. **Actual UNCONFIRMED field values are empty FString.**

**`ReferenceVehicles[1]` — Korea Secondary:** `ReferenceVehicleId=ref059-coupe-gr86kr-supp`; `Role=Secondary`; Manufacturer=Toyota, Model=GR86, MarketRegion=KR, ModelYearQualifier=Unknown; unverified grade/year/powertrain left empty. Oman Primary remains unchanged; this is a supplemental reference, not a seventh production vehicle.

**`Sources`**

| SourceId | Tier / SourceKind | Publisher / DocumentTitle | CanonicalUrl | Locator / scope / source reference |
| --- | --- | --- | --- | --- |
| `src-toyota-gr86-2026-oman` | `TierB` / `DistributorSpecification` | Toyota Oman distributor / GR86 2026 specification PDF | https://www.toyotaoman.com/wp-content/uploads/2026/01/Gr-86-specsheet.pdf | OM 2026 6MT L/W/H, 233HP, RWD, 215/40R18; wheelbase absent; `ReferenceVehicleIds=[ref059-coupe-gr86mt26om]`; `OriginGroupId=TOYOTA_OM`; `OriginIndependence=OriginUnknown` |
| `src-toyota-gr86-kr-wheelbase` | `TierA` / `ManufacturerSpecifications` | Toyota Korea / GR86 Korean model page | https://www.toyota.co.kr/models/gr86/ | KR same model 2575mm Wheelbase only; NOT OM same-trim approval; `ReferenceVehicleIds=[ref059-coupe-gr86kr-supp]`; `OriginGroupId=TOYOTA_KR`; `OriginIndependence=OriginUnknown` |

**`Claims`: FACT (Dimension 3 Canonical, cross-market wheelbase + 6 other Candidate)**

| ClaimId | FactKey | ValueKind + typed field | UnitId | CitationIds[0] | SourceValueText / 제한 | ResolutionState |
| --- | --- | --- | --- | --- | --- |
| `clm-3-exteriorlengthmm` | `ExteriorLengthMm` | `Integer`: `IntegerValue=4265` | `mm` | `src-toyota-gr86-2026-oman` | 4265mm | `Canonical` |
| `clm-3-exteriorwidthmm` | `ExteriorWidthMm` | `Integer`: `IntegerValue=1775` | `mm` | `src-toyota-gr86-2026-oman` | 1775mm | `Canonical` |
| `clm-3-exteriorheightmm` | `ExteriorHeightMm` | `Integer`: `IntegerValue=1310` | `mm` | `src-toyota-gr86-2026-oman` | 1310mm | `Canonical` |
| `clm-3-wheelbasemm` | `WheelbaseMm` | `Integer`: `IntegerValue=2575` | `mm` | `src-toyota-gr86-kr-wheelbase` | 2575mm KR cross-market supplemental; Claim.ReferenceVehicleId=`ref059-coupe-gr86kr-supp`, Oman exact not proven | `Candidate` |
| `clm-3-enginedisplacementl` | `EngineDisplacementL` | `Number`: `NumberValue=2.4` | `L` | `src-toyota-gr86-2026-oman` | 2.4L Boxer D-4S | `Candidate` |
| `clm-3-publishedenginepowerhp` | `PublishedEnginePowerHP` | `Integer`: `IntegerValue=233` | `HP` | `src-toyota-gr86-2026-oman` | OM 233HP@7000rpm | `Candidate` |
| `clm-3-publishedenginepowerrpm` | `PublishedEnginePowerRpm` | `Integer`: `IntegerValue=7000` | `rpm` | `src-toyota-gr86-2026-oman` | 7000rpm | `Candidate` |
| `clm-3-tirecode` | `TireCode` | `CanonicalText`: `TextValue="215/40R18"` | `None` | `src-toyota-gr86-2026-oman` | OM 215/40R18 | `Candidate` |
| `clm-3-transmissionkind` | `TransmissionKind` | `CanonicalText`: `TextValue="6MT"` | `None` | `src-toyota-gr86-2026-oman` | 6MT | `Candidate` |
| `clm-3-drivelayout` | `DriveLayout` | `CanonicalText`: `TextValue="RWD"` | `None` | `src-toyota-gr86-2026-oman` | RWD | `Candidate` |

**`UnknownFacts`**

| UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note |
| --- | --- | --- | --- | --- | --- |
| `unk-3-curbweightkg` | `CurbWeightKg` | `NotPublished` | `ProposalBlock` | `[src-toyota-gr86-2026-oman]` | exact Oman MT mass not sourced |
| `unk-3-grossweightkg` | `GrossWeightKg` | `NotPublished` | `ProposalWarning` | `[src-toyota-gr86-2026-oman]` | exact Oman 6MT GVWR not sourced |
| `unk-3-publishedenginepeaktorquenm` | `PublishedEnginePeakTorqueNm` | `NotPublished` | `ProposalWarning` | `[src-toyota-gr86-2026-oman]` | Oman peak torque not independently sourced |
| `unk-3-trackfrontmm` | `TrackFrontMm` | `NotPublished` | `ProposalWarning` | `[src-toyota-gr86-2026-oman]` | exact Oman selected track front unknown |
| `unk-3-trackrearmm` | `TrackRearMm` | `NotPublished` | `ProposalWarning` | `[src-toyota-gr86-2026-oman]` | exact Oman selected track rear unknown |

**`Conflicts=[]`** (현재 조사된 사실의 확인된 동종 충돌 없음). **`ResearchNotes`**: WheelbaseMm=2575 source must be Toyota KR `src-toyota-gr86-kr-wheelbase` owned by Secondary `ref059-coupe-gr86kr-supp`, explicitly cross-market supplemental **Candidate** only. Oman OEM sheet has NO wheelbase; Korean 231PS, curb and torque must not be imported as Oman facts. Exact Oman applicability/trim requires reviewed decision.

**정적 검수:** Oman Primary1 + Korea Secondary1 / Source IDs 2 / FACT 10 / dimension4 (OM3 Canonical + KR1 Candidate) / Unknown 5 / direct Claim/Source Reference ownership / ClaimId unique. Dimension research Canonical 검수 반영. UE Validator NOT RUN / final Evidence HOLD.

### 2.4 Pickup / Toyota HiLux SR5 4x4 AT Double-Cab V-Active AU

**`ReferenceVehicles[0]`**: `ReferenceVehicleId=ref059-pickup-hiluxsr5at-au`; `Role=Primary`; `IdentityConfidence=0.0`; Manufacturer=Toyota; Model=HiLux; Generation=UNCONFIRMED; ModelYearQualifier=CurrentAsObserved; ModelYearStart=0; ModelYearEnd=0; Trim=SR5 4x4 AT Double-Cab V-Active (exact options UNCONFIRMED); Powertrain=2.8L turbodiesel; Transmission=6AT; DriveLayout=4WD; MarketRegion=AU; BodyVariant=Double-Cab Pickup; WheelTireVariant=UNCONFIRMED. **UNCONFIRMED field은 실제 객체에 빈 FString 저장.**

**`Sources`**

| SourceId | Tier / SourceKind | Publisher / DocumentTitle | CanonicalUrl | Locator / Scope / source reference |
| --- | --- | --- | --- | --- |
| `src-toyota-hiluxsr5-au` | `TierA` / `ManufacturerRangePage` | Toyota Australia / Current HiLux SR5 range | https://www.toyota.com.au/hilux/range | AU current SR5 4x4 AT V-Active; 2155 maximum kerb and GVM range 3080–3120 only; `ReferenceVehicleIds=[ref059-pickup-hiluxsr5at-au]`; `OriginGroupId=TOYOTA_AU`; `OriginIndependence=OriginUnknown` |

**`Claims`: FACT with per-row Canonical/Candidate review**

| ClaimId | FactKey | ValueKind + typed value | UnitId | CitationIds[0] | SourceValueText / scope | ResolutionState |
| --- | --- | --- | --- | --- | --- | --- |
| `clm-4-exteriorlengthmm` | `ExteriorLengthMm` | `Integer`: `IntegerValue=5320` | `mm` | `src-toyota-hiluxsr5-au` | 5320mm (Toyota AU published approx. SR5 body dimensions; 옵션별 보장값이 아님) | `Canonical` |
| `clm-4-exteriorwidthmm` | `ExteriorWidthMm` | `Integer`: `IntegerValue=1885` | `mm` | `src-toyota-hiluxsr5-au` | 1885mm (Toyota AU published approx. SR5 body dimensions; 옵션별 보장값이 아님) | `Canonical` |
| `clm-4-exteriorheightmm` | `ExteriorHeightMm` | `Integer`: `IntegerValue=1865` | `mm` | `src-toyota-hiluxsr5-au` | 1865mm (Toyota AU published approx. SR5 body dimensions; 옵션별 보장값이 아님) | `Canonical` |
| `clm-4-wheelbasemm` | `WheelbaseMm` | `Integer`: `IntegerValue=3085` | `mm` | `src-toyota-hiluxsr5-au` | 3085mm (Toyota AU published approx. SR5 body dimensions; 옵션별 보장값이 아님) | `Canonical` |
| `clm-4-enginedisplacementl` | `EngineDisplacementL` | `Number`: `NumberValue=2.8` | `L` | `src-toyota-hiluxsr5-au` | 2.8L turbodiesel | `Candidate` |
| `clm-4-publishedenginepowerkw` | `PublishedEnginePowerKw` | `Integer`: `IntegerValue=150` | `kW` | `src-toyota-hiluxsr5-au` | 150kW | `Candidate` |
| `clm-4-publishedenginepeaktorquenm` | `PublishedEnginePeakTorqueNm` | `Integer`: `IntegerValue=500` | `Nm` | `src-toyota-hiluxsr5-au` | 500Nm | `Candidate` |
| `clm-4-maxpublishedcurbweightkg` | `MaxPublishedCurbWeightKg` | `Integer`: `IntegerValue=2155` | `kg` | `src-toyota-hiluxsr5-au` | MAXIMUM 2155kg — NOT exact CurbWeightKg | `Candidate` |
| `clm-4-publishedgvmrangekg` | `PublishedGvmRangeKg` | `Range`: `RangeMinValue=3080, RangeMaxValue=3120` | `kg` | `src-toyota-hiluxsr5-au` | variant GVM RANGE, not exact selected GrossWeightKg | `Candidate` |
| `clm-4-trackfrontmm` | `TrackFrontMm` | `Integer`: `IntegerValue=1530` | `mm` | `src-toyota-hiluxsr5-au` | 1530mm | `Candidate` |
| `clm-4-trackrearmm` | `TrackRearMm` | `Integer`: `IntegerValue=1555` | `mm` | `src-toyota-hiluxsr5-au` | 1555mm | `Candidate` |
| `clm-4-transmissionkind` | `TransmissionKind` | `CanonicalText`: `TextValue="6AT"` | `None` | `src-toyota-hiluxsr5-au` | 6AT | `Candidate` |
| `clm-4-drivelayout` | `DriveLayout` | `CanonicalText`: `TextValue="4WD"` | `None` | `src-toyota-hiluxsr5-au` | 4WD | `Candidate` |

**`UnknownFacts`**

| UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note |
| --- | --- | --- | --- | --- | --- |
| `unk-4-curbweightkg` | `CurbWeightKg` | `IdentityAmbiguous` | `ProposalBlock` | `[src-toyota-hiluxsr5-au]` | exact chosen SR5 configuration curb mass unknown; 2155 only published max |
| `unk-4-grossweightkg` | `GrossWeightKg` | `IdentityAmbiguous` | `ProposalBlock` | `[src-toyota-hiluxsr5-au]` | GVM 3080–3120 range not a chosen truck GVWR |
| `unk-4-tirecode` | `TireCode` | `IdentityAmbiguous` | `ProposalWarning` | `[src-toyota-hiluxsr5-au]` | selected SR5 options/tire exact not frozen |
| `unk-4-publishedenginepeaktorquerpm` | `PublishedEnginePeakTorqueRpm` | `NotPublished` | `ProposalWarning` | `[src-toyota-hiluxsr5-au]` | RPM on selected AU SR5 current page not sourced |
| `unk-4-exactmodelyear` | `ExactModelYear` | `NotPublished` | `ProposalWarning` | `[src-toyota-hiluxsr5-au]` | current listing observation date does not prove model-year |

**`Conflicts=[]`** (현재 확인된 동종 FACT 충돌 없음).

**`ResearchNotes`**: Exact chosen options and GVM/curb must remain Unknown even though manufacturer posts maximum/range claims. `CurbWeightKg=2155` or `GrossWeightKg=3080` prohibited. Date is observation not exact MY.

**정적 검수:** Primary1; Sources1; FACT13(dimension4); Unknown5; Citation & SearchedSource IDs in own Sources;  연구용 Dimension Canonical 심사 반영. **UE Validator NOT RUN / Final identity acceptance HOLD.**


### 2.5 SubCompact / Hyundai Venue KR 1.6 IVT 17-inch

**`ReferenceVehicles[0]`**: `ReferenceVehicleId=ref059-subcompact-venue17kr`; `Role=Primary`; `IdentityConfidence=0.0`; Manufacturer=Hyundai; Model=Venue; Generation=UNCONFIRMED; ModelYearQualifier=Unknown; ModelYearStart=0; ModelYearEnd=0; Trim=UNCONFIRMED (17-inch IVT variant only); Powertrain=Smartstream Gasoline G1.6 1598cc; Transmission=IVT; DriveLayout=FWD; MarketRegion=KR; BodyVariant=Subcompact SUV; WheelTireVariant=205/55R17. **UNCONFIRMED field은 실제 객체에 빈 FString 저장.**

**`Sources`**

| SourceId | Tier / SourceKind | Publisher / DocumentTitle | CanonicalUrl | Locator / Scope / source reference |
| --- | --- | --- | --- | --- |
| `src-hyundai-venue-brochure` | `TierA` / `ManufacturerBrochure` | Hyundai Korea / Venue official English catalogue | https://www.hyundai.com/content/dam/hyundai/kr/ko/data/vehicles/catalog/en/venue-catalog-eng.pdf | PDF p20 (viewer p19 zero-based); rear track table 1546 vs dimension drawing 1545; both SAME original source; `ReferenceVehicleIds=[ref059-subcompact-venue17kr]`; `OriginGroupId=HYUNDAI_KR`; `OriginIndependence=OriginUnknown` |

**`Claims`: FACT with per-row Canonical/Candidate review**

| ClaimId | FactKey | ValueKind + typed value | UnitId | CitationIds[0] | SourceValueText / scope | ResolutionState |
| --- | --- | --- | --- | --- | --- | --- |
| `clm-5-exteriorlengthmm` | `ExteriorLengthMm` | `Integer`: `IntegerValue=4040` | `mm` | `src-hyundai-venue-brochure` | 4040mm | `Canonical` |
| `clm-5-exteriorwidthmm` | `ExteriorWidthMm` | `Integer`: `IntegerValue=1770` | `mm` | `src-hyundai-venue-brochure` | 1770mm | `Canonical` |
| `clm-5-exteriorheightmm` | `ExteriorHeightMm` | `Integer`: `IntegerValue=1585` | `mm` | `src-hyundai-venue-brochure` | 1585mm | `Canonical` |
| `clm-5-wheelbasemm` | `WheelbaseMm` | `Integer`: `IntegerValue=2520` | `mm` | `src-hyundai-venue-brochure` | 2520mm | `Canonical` |
| `clm-5-enginedisplacementcc` | `EngineDisplacementCc` | `Integer`: `IntegerValue=1598` | `cc` | `src-hyundai-venue-brochure` | 1598cc | `Candidate` |
| `clm-5-publishedenginepowerps` | `PublishedEnginePowerPS` | `Integer`: `IntegerValue=123` | `PS` | `src-hyundai-venue-brochure` | 123PS@6300rpm | `Candidate` |
| `clm-5-publishedenginepowerrpm` | `PublishedEnginePowerRpm` | `Integer`: `IntegerValue=6300` | `rpm` | `src-hyundai-venue-brochure` | 6300rpm | `Candidate` |
| `clm-5-publishedenginepeaktorquekgfm` | `PublishedEnginePeakTorqueKgfM` | `Number`: `NumberValue=15.7` | `kgf_m` | `src-hyundai-venue-brochure` | 15.7kgf·m@4500rpm | `Candidate` |
| `clm-5-publishedenginepeaktorquerpm` | `PublishedEnginePeakTorqueRpm` | `Integer`: `IntegerValue=4500` | `rpm` | `src-hyundai-venue-brochure` | 4500rpm | `Candidate` |
| `clm-5-curbweightkg` | `CurbWeightKg` | `Integer`: `IntegerValue=1215` | `kg` | `src-hyundai-venue-brochure` | 17-inch IVT only; not 15-inch 1180kg | `Candidate` |
| `clm-5-tirecode` | `TireCode` | `CanonicalText`: `TextValue="205/55R17"` | `None` | `src-hyundai-venue-brochure` | 205/55R17 | `Candidate` |
| `clm-5-trackfrontmm` | `TrackFrontMm` | `Integer`: `IntegerValue=1535` | `mm` | `src-hyundai-venue-brochure` | 1535mm | `Candidate` |
| `clm-5-trackrearmm-table` | `TrackRearMm` | `Integer`: `IntegerValue=1546` | `mm` | `src-hyundai-venue-brochure` | PDF p20 SPECIFICATION TABLE 1546mm; Candidate; conflict c-venue-track-rear | `Candidate` |
| `clm-5-trackrearmm-drawing` | `TrackRearMm` | `Integer`: `IntegerValue=1545` | `mm` | `src-hyundai-venue-brochure` | PDF p20 DIMENSION DRAWING 1545mm; Candidate; conflict c-venue-track-rear | `Candidate` |
| `clm-5-transmissionkind` | `TransmissionKind` | `CanonicalText`: `TextValue="IVT"` | `None` | `src-hyundai-venue-brochure` | IVT | `Candidate` |

**`UnknownFacts`**

| UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note |
| --- | --- | --- | --- | --- | --- |
| `unk-5-exactsaletrim` | `ExactSaleTrim` | `IdentityAmbiguous` | `ProposalBlock` | `[src-hyundai-venue-brochure]` | Premium or other unique sale trim not proved by this 17-inch source alone |
| `unk-5-exactmodelyear` | `ExactModelYear` | `IdentityAmbiguous` | `ProposalWarning` | `[src-hyundai-venue-brochure]` | catalogue 2025 mention/history not sole proof of exact sale MY |
| `unk-5-grossweightkg` | `GrossWeightKg` | `NotPublished` | `ProposalWarning` | `[src-hyundai-venue-brochure]` | 17-inch IVT exact gross not sourced |
| `unk-5-trackrearmm` | `TrackRearMm` | `ConflictingUnresolved` | `ProposalWarning` | `[src-hyundai-venue-brochure]` | same official Hyundai PDF p20 table1546 vs illustration1545, neither canonical |

**`Conflicts=[FCFRefConflict]`**

- `ConflictId=c-venue-track-rear`; `ReferenceVehicleId=ref059-subcompact-venue17kr`; `FactKey=TrackRearMm`; `CandidateClaimIds=[clm-5-trackrearmm-table, clm-5-trackrearmm-drawing]`; `ConflictType=SourceDisagreement`; `Severity=Warning`; `ResolutionPolicy=UnresolvedBlock`; `ResolutionClaimId=None`; `ResolutionReasonCode=None`.
- 두 `FCFRefClaim`의 `ConflictId=c-venue-track-rear`, 모두 `Candidate`. **Severity=Warning**은 현재 `ValidateInitialResearchPayload`의 unresolved `Severity=Block` 차단 조건은 충족하지 않는다. 단 해당 후륜 윤거 값은 절대로 Canonical로 승격하지 않는다.
- Source Locator가 구조체 기준으로 Source 단위라는 한계 때문에 같은 PDF 내부 `1546(table)`/`1545(drawing)`을 별개 Claim `SourceValueText`와 `ResearchNotes`에서 정확히 구분한다. 별도 SourceId를 만들어 독립 OEM 출처라고 주장하지 않는다.

**`ResearchNotes`**: RearTrack conflict candidates both use **one** `src-hyundai-venue-brochure` (not falsely independent), claim locators `spec table` and `dimension drawing` recorded in SourceValueText, ConflictId identical. Full 4 dimensional facts unaffected. 17-inch height1585 (no rack), curb1215; Premium exact trim unknown.

**정적 검수:** Primary1; Sources1; FACT15(dimension4); Unknown4; Citation & SearchedSource IDs in own Sources;  연구용 Dimension Canonical 심사 반영. **UE Validator NOT RUN / Final identity acceptance HOLD.**


### 2.6 Van / Hyundai The New STARIA Hybrid Cargo5 KR 17-inch

**`ReferenceVehicles[0]`**: `ReferenceVehicleId=ref059-van-stariahybridcargo5kr`; `Role=Primary`; `IdentityConfidence=0.0`; Manufacturer=Hyundai; Model=The New STARIA Hybrid; Generation=UNCONFIRMED; ModelYearQualifier=CurrentAsObserved; ModelYearStart=0; ModelYearEnd=0; Trim=Cargo 5-seat Hybrid, 17-inch; Powertrain=Smartstream G1.6T Hybrid 1598cc; Transmission=6AT; DriveLayout=UNCONFIRMED; MarketRegion=KR; BodyVariant=Cargo 5-seat; WheelTireVariant=17-inch (exact tire code UNCONFIRMED). **UNCONFIRMED field은 실제 객체에 빈 FString 저장.**

**`Sources`**

| SourceId | Tier / SourceKind | Publisher / DocumentTitle | CanonicalUrl | Locator / Scope / source reference |
| --- | --- | --- | --- | --- |
| `src-hyundai-stariahybridcargo-kr` | `TierA` / `ManufacturerModelPage` | Hyundai Korea / The New STARIA Hybrid / Cargo official page | https://www.hyundai.com/kr/ko/e/vehicles/the-new-staria-hybrid/intro | KR current page observed 2026-10; Cargo5 17-inch, not Tourer or Lounge. ICE180PS/motor54kW/system245PS separate; `ReferenceVehicleIds=[ref059-van-stariahybridcargo5kr]`; `OriginGroupId=HYUNDAI_KR`; `OriginIndependence=OriginUnknown` |

**`Claims`: FACT with per-row Canonical/Candidate review**

| ClaimId | FactKey | ValueKind + typed value | UnitId | CitationIds[0] | SourceValueText / scope | ResolutionState |
| --- | --- | --- | --- | --- | --- | --- |
| `clm-6-exteriorlengthmm` | `ExteriorLengthMm` | `Integer`: `IntegerValue=5255` | `mm` | `src-hyundai-stariahybridcargo-kr` | 5255mm | `Canonical` |
| `clm-6-exteriorwidthmm` | `ExteriorWidthMm` | `Integer`: `IntegerValue=1995` | `mm` | `src-hyundai-stariahybridcargo-kr` | 1995mm | `Canonical` |
| `clm-6-exteriorheightmm` | `ExteriorHeightMm` | `Integer`: `IntegerValue=2000` | `mm` | `src-hyundai-stariahybridcargo-kr` | 2000mm | `Canonical` |
| `clm-6-wheelbasemm` | `WheelbaseMm` | `Integer`: `IntegerValue=3275` | `mm` | `src-hyundai-stariahybridcargo-kr` | 3275mm | `Canonical` |
| `clm-6-enginedisplacementcc` | `EngineDisplacementCc` | `Integer`: `IntegerValue=1598` | `cc` | `src-hyundai-stariahybridcargo-kr` | G1.6T 1598cc | `Candidate` |
| `clm-6-icepublishedpowerps` | `IcePublishedPowerPS` | `Integer`: `IntegerValue=180` | `PS` | `src-hyundai-stariahybridcargo-kr` | combustion engine ONLY 180PS@5500rpm | `Candidate` |
| `clm-6-icepublishedpowerrpm` | `IcePublishedPowerRpm` | `Integer`: `IntegerValue=5500` | `rpm` | `src-hyundai-stariahybridcargo-kr` | ICE only 5500rpm | `Candidate` |
| `clm-6-icepublishedpeaktorquekgfm` | `IcePublishedPeakTorqueKgfM` | `Number`: `NumberValue=27.0` | `kgf_m` | `src-hyundai-stariahybridcargo-kr` | ICE 27.0kgf·m@1500–4500 | `Candidate` |
| `clm-6-icepeaktorquerpmrange` | `IcePeakTorqueRpmRange` | `Range`: `RangeMinValue=1500, RangeMaxValue=4500` | `rpm` | `src-hyundai-stariahybridcargo-kr` | ICE torque plateau 1500–4500rpm | `Candidate` |
| `clm-6-electricmotorpublishedpowerkw` | `ElectricMotorPublishedPowerKw` | `Integer`: `IntegerValue=54` | `kW` | `src-hyundai-stariahybridcargo-kr` | electric motor 54kW | `Candidate` |
| `clm-6-electricmotorpublishedpeaktorquenm` | `ElectricMotorPublishedPeakTorqueNm` | `Integer`: `IntegerValue=304` | `Nm` | `src-hyundai-stariahybridcargo-kr` | motor peak torque 304Nm | `Candidate` |
| `clm-6-hybridsystempublishedpowerps` | `HybridSystemPublishedPowerPS` | `Integer`: `IntegerValue=245` | `PS` | `src-hyundai-stariahybridcargo-kr` | system 245PS; NOT ICE power | `Candidate` |
| `clm-6-curbweightkg` | `CurbWeightKg` | `Integer`: `IntegerValue=2070` | `kg` | `src-hyundai-stariahybridcargo-kr` | Cargo5 17-inch only | `Candidate` |
| `clm-6-trackfrontmm` | `TrackFrontMm` | `Integer`: `IntegerValue=1732` | `mm` | `src-hyundai-stariahybridcargo-kr` | 1732mm | `Candidate` |
| `clm-6-trackrearmm` | `TrackRearMm` | `Integer`: `IntegerValue=1716` | `mm` | `src-hyundai-stariahybridcargo-kr` | 1716mm | `Candidate` |
| `clm-6-transmissionkind` | `TransmissionKind` | `CanonicalText`: `TextValue="6AT"` | `None` | `src-hyundai-stariahybridcargo-kr` | 6AT | `Candidate` |

**`UnknownFacts`**

| UnknownFactId | FactKey | Reason | BlockingUse | SearchedSourceIds | Note |
| --- | --- | --- | --- | --- | --- |
| `unk-6-exactmodelyear` | `ExactModelYear` | `NotPublished` | `ProposalWarning` | `[src-hyundai-stariahybridcargo-kr]` | October 2026 page observation != exact OEM model-year |
| `unk-6-grossweightkg` | `GrossWeightKg` | `NotPublished` | `ProposalWarning` | `[src-hyundai-stariahybridcargo-kr]` | Cargo5 17-inch gross not sourced |
| `unk-6-drivelayout` | `DriveLayout` | `NotPublished` | `ProposalWarning` | `[src-hyundai-stariahybridcargo-kr]` | exact driven axle from selected source not yet confirmed |
| `unk-6-tirecode` | `TireCode` | `NotPublished` | `ProposalWarning` | `[src-hyundai-stariahybridcargo-kr]` | 17-inch known but exact size/aspect unspecified |

**`Conflicts=[]`** (현재 확인된 동종 FACT 충돌 없음).

**`ResearchNotes`**: Hybrid system245PS, combustion180PS, motor54kW/304Nm are separate facts. No direct Chaos single-ICE horsepower sum or ICE+motor torque sum. Cargo height2000, not Tourer1990. No exact 2026 model year promotion.

**정적 검수:** Primary1; Sources1; FACT16(dimension4); Unknown4; Citation & SearchedSource IDs in own Sources;  연구용 Dimension Canonical 심사 반영. **UE Validator NOT RUN / Final identity acceptance HOLD.**

## 3. 검수·수락 판정

### 3.1 이번 문서의 정확한 독립 구조·정적 증거

| 연구 Payload | Primary | Sources | FACT (Canonical + Candidate) | Dimension FACT | UnknownFact | Conflict |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| CityCar Morning | 1 | 2 | 14 | 4 | 3 | 0 |
| Compact Jimny5 | 1 | 1 | 16 | 4 | 3 | 0 |
| Coupe GR86 Oman Primary + KR Secondary | 1 Primary + 1 Secondary | 2 | 10 | 4 (OM3 + KR1) | 5 | 0 |
| Pickup HiLux SR5 | 1 | 1 | 13 | 4 | 5 | 0 |
| SubCompact Venue 17 | 1 | 1 | 15 (rear-track 후보 2) | 4 | 4 | 1 |
| Van STARIA Hybrid Cargo | 1 | 1 | 16 | 4 | 4 | 0 |
| **합계** | **6 Primary + 1 Secondary** | **8** | **84 (Canonical 23 / Candidate 61)** | **24 (Canonical 23 / Candidate 1)** | **24** | **1** |

**정적 문서 검사 PASS** (2026-10-19, `CarFightMCP_Admin.repo_read_batch` readback에 대한 독립 JS 구조 검사): 6개 절, ClaimId 84개 unique, SourceId 8개 unique, UnknownFactId 24개 unique, 6종 모두 4개 치수 Claim, 모든 Claim `CitationIds`와 모든 Unknown `SearchedSourceIds`가 **자기 Payload의** Sources에 존재, 최초 입력안에서는 Source `ReferenceVehicleIds`가 모두 각자의 Primary와 일치했다(역사적 검수). 현재 GR86 한국 Source/Claim은 한국 Secondary로 분리됐으며 이 소유 관계는 §7의 최신 UE 집중 검증에서 확인했다. typed 필드 `IntegerValue/NumberValue/RangeMinValue+RangeMaxValue/TextValue` 형식 PASS. 현재 확인된 Venue `clm-5-trackrearmm-table=1546` / `clm-5-trackrearmm-drawing=1545`의 정확한 별도 후보 PASS. OEM 원본 제원 6×4와 표 값은 `VehicleReferenceSourceReview.md v0.1.2`의 verified 24개 baseline과 동일하다.

**Historical 2026-10-19 입력안 단계의 정적 검수 (후속 실제 실행은 §5 참조):** 당시 `ValidateInitialResearchPayload()`는 실행 전이었다. **23개 Source-verified Dimension FACT를 문서 차원의 Canonical로 판정했고 나머지 61개는 Candidate**이다. 이는 필수 Canonical-count 조건을 문서상 만족한다는 뜻이지 **실제 API 테스트를 실행했다는 의미가 아니다.** 개별 Claim-to-Source/Market과 User final identity acceptance가 별개로 남아 있으므로 시험 결과를 PASS라 주장하지 않는다.

### 3.2 특수 위험과 명시적인 차단 항목

- **GR86**: 2026 Oman `WheelbaseMm=2575`은 정확한 Oman sheet의 FACT가 아닌 별도 Toyota KR 시장의 same-model 보조 Candidate이다. 현재 Source.ReferenceVehicleIds는 연구 Primary 참조와 연결되나, 이것만으로 KR 시장 치수가 OM 트림의 exact FACT로 승인되는 것은 아니다. 필요하면 허용된 후속 Identity split/보조 Reference를 사용하고 실제 승인은 별도 판단한다.
- **Venue**: 1,546 vs 1,545는 `FCFRefConflict(Severity=Warning, ResolutionPolicy=UnresolvedBlock, ResolutionClaimId=None)` + 후보2 + Unknown1로 표현. Warning이므로 validator unresolved *Block* 조건을 만족하지 않지만, 후륜 윤거 scalar를 Canonical로 수락한 것은 아니다. 제조사 정오표/정확한 트림 증거 없이 수치 승격 금지.
- **HiLux**: `MaxPublishedCurbWeightKg`와 `PublishedGvmRangeKg`은 정확한 1대의 `CurbWeightKg`/`GrossWeightKg` 아님. 두 exact 필드는 Unknown. `Range`는 두 float 범위를 유지한다.
- **STARIA**: HybridSystemPublishedPowerPS·IcePublishedPowerPS·ElectricMotorPublishedPowerKw·ElectricMotorPublishedPeakTorqueNm은 서로 다른 Claim. 단일 ICE 엔진 스펙으로 합치지 않는다.
- **입력 타입·원본 교차검사 한계**: 현재 API는 `FactKey`와 `ValueKind`의 source 값의 물리적 타당성을 자동 입증하지 않으며, Claim의 Citation Source가 동일 시장/트림인지도 검증하지 않는다. 이번 사전 검수는 현재 기록된 출처·타입·입력 ID의 **정적 일치**만 증명한다. 실제 `UCFVehicleRefEvidence` UObject 생성·DataAsset Save·Workbook/Recipe/VehicleData 변환·UE compile/Automation·GoPyMCP 구현/수정 **0**.

**최종 판정:** `P0-02 CORE6 DIMENSION RESEARCH CANONICAL 23/24 / SOURCE KR CROSS-MARKET WHEELBASE CANDIDATE 1 / ALL 84 CLAIMS REVIEWED FOR THIS STAGE / STATIC CONSISTENCY PASS / USER FINAL IDENTITY ACCEPTANCE & UE VALIDATOR NOT RUN / P0-02 FULL EVIDENCE HOLD`. 현재 `VCCAS-P0-03C` Work lifecycle 변화 없음.

## 4. Canonical FACT 수락 검수 — 2026-10-19 (Document-only)

- **수락 의미:** 선택된 연구 ReferenceVehicleId/시장/사양에 제조사가 직접 명시한 `FACT`를 **Source-scoped canonical research fact**로 인정한다. 최종 사용자 차량(모델/연식/트림) 수락·영구 `UCFVehicleRefEvidence`·게임 적용의 승인과 다르다. 제조사 페이지가 `approximate`라고 한 값은 수치가 같더라도 의미적 한정을 유지한다.
- **CityCar 4/4 `Canonical`:** Kia KR 2027 Morning Gasoline L3595/W1595/H1485/WB2400. 1.0 Trendy 기본 175/65R14/4AT는 Kia 가격·제원 페이지로 대조. 다만 가격표 변경·사양 옵션 최종 검수는 별도이며 14/16인치 공차중량을 혼합하지 않는다.
- **Compact 4/4 `Canonical`:** June2024 NEXA Jimny Indian **5-door** brochure page10 L3985 (rear spare cover 포함)/W1645/H1720/WB2590. Alpha AT 1205kg·GVW1545kg는 별도 후보로 이번 승격 대상이 아니다. 브로슈어 발행월을 차량 MY로 승격하지 않는다.
- **Coupe 3/4 `Canonical`:** 2026 Toyota Oman 6MT GR86 L4265/W1775/H1310만 오만 사양 자료로 수락. `clm-3-wheelbasemm`=2575는 **Toyota KR 다른 시장/등급을 다룬 독립 자료**이므로 `Candidate` 유지; 오만 완전 동일 트림 인증 출처 아님. 한국 231PS나 한국 차중량으로 오만 233HP를 대체하지 않는다.
- **Pickup 4/4 `Canonical` (approx-qualified):** Toyota Australia SR5 4x4 Automatic Double-Cab with V-Active L5320/W1885/H1865/WB3085. **제조사 원문은 치수를 `Dimensions (approximate)`로 표시**한다. 따라서 SourceValueText에 approximate 설명을 남기고 정확한 실측 Product 물리값이라는 의미로 사용하지 않는다. 최대공차 2155kg 및 GVM3080~3120은 여전히 exact mass 불가.
- **SubCompact 4/4 `Canonical`:** Hyundai Venue KR brochure 17-inch variant L4040/W1770/H1585 (without roof rack)/WB2520. 같은 PDF p20의 17-inch 후륜 윤거 1546(spec table)/1545(drawing)는 둘 다 `Candidate`, `Conflict` 및 `UnknownFact` 보존. 15-inch 높이 1565 및 17-inch+roof-rack 1610은 섞지 않는다. 정확한 판매 트림/MY 미수락.
- **Van 4/4 `Canonical`:** Hyundai The New STARIA Hybrid Cargo 5-seat 17-inch KR current webpage L5255/W1995/H2000/WB3275. Tourer H1990 혼용 금지. system245PS/ICE180PS/motor54kW는 아직 개별 `Candidate`로 구분. current observation ≠ exact manufacturer MY.
- **기술 상태 (Canonical review 당시 기록):** `ECFRefClaimResolution::Canonical`은 실제 C++ enum에 존재하므로 별도 schema 확장 불필요. 이후 §5에서 여섯 typed payload를 실제 C++로 구성하고 transient UObject validator를 직접 호출해 focused UE Automation PASS까지 확인했다. 개별 원문 source/시장/트림 최종 승인·Production Evidence 저장은 계속 HOLD.
- **검수 후 다음:** 출처 scope 검토와 User final model/trim 선택 검토를 분리한 채, **CarFight 내부의 독립된 메모리 payload validator focused test**로 기존 계약의 정확한 PASS/FAIL을 확인하는 것이 가능하다. P0-04 DataAsset Save/Create·Trusted AI R2·GoPyMCP 작업은 이 검수와 무관하다.

## 5. 실제 typed C++ Payload Validator 집중 Automation — 2026-10-19

**결과: OFFICIAL UE5.8 BUILD PASS / Core6 NORMAL 6/6 PASS / NEGATIVE GUARD 11/11 PASS / exact Automation1/1 PASS / persistent Evidence changes0.**

- 신규 **`UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleCore6EvidenceTests.cpp v1.0.0`**가 문서의 여섯 독립 Payload를 메모리에서 실제 `FCFVehicleRefEvidencePayload`로 생성한다. Primary6, Source8, FACT84(Canonical23/Candidate61), Unknown24, Venue WarningConflict1을 typed field/enum과 원래 ID/Citation/SourceScopeNote로 구성한다. 기록용 `SourceValueText`는 수치의 원단위·variant/approximate qualifier를 보존한다.
- 실제 `UCFVehicleRefEvidence`는 `NewObject`를 `GetTransientPackage()`에 생성하고 `ValidateInitialResearchPayload(Payload, Error)`만 호출한다. `ApplyInitialResearchPayload`, AssetRegistry, SavePackage, 생성형 차량 Writer는 호출하지 않는다. 여섯 정상 Payload가 전부 accept되는지 검사하고, 잘못된 입력 11종(모든 Canonical 제거, 없는 CitationId, 다른 Vehicle ID Claim, 다른 Vehicle ID Source, 중복 SourceId, 중복 ClaimId, 없는 Unknown searched SourceId, Primary 부재, GAME_BIAS, Venue unresolved Block, Venue missing conflict candidate)을 각각 reject하는지 검증한다.
- **공식 Editor Build:** CarFightMCP_Admin legacy `repository_id=main_game`, preset `carfight.editor.development`, job `8cf022cd62ac46cda8510296acfe34f3`, `UnrealEditor-CarFight_ReEditor.dll` 포함 최종 링크 성공, **exit code0**.
- **집중 Automation:** `Tools/RunDataAuthoringTests.ps1 v1.3.2 -ExactTestNames CarFight.CCAS.CF_FQ_059.P0_02.Core6PayloadValidation -SuppressModelContextProtocolLog -EvidenceId CF059_P002_Core6_20261019_Validation01`. 관리형 process `2e70e468bcf64aabb66e76e22c4077c2`, **exit0**, exact requested1/success1/fail0/missing0/unexpected0/duplicate_terminal0. Result JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_P002_Core6_20261019_Validation01.json` 실제 `process.status` readback, SHA-256 `389adb6954fa539b4ef2ede8111cec0e3f232b7edd49bc3d3b09cf77db84cf7f`, schema `carfight_data_authoring_automation_result_v2`.
- **판정 범위:** 실제 C++ Validator의 6개 Research Payload에 대한 참조·ID·Citation 존재·최소 Canonical 조건과 명시적 invalid11 회귀는 기술 PASS. 그러나 해당 Validator는 개별 Claim→Source의 **실제 market/trim/fact 원문 정합성**을 자동 확증하지 않고, `UnknownFact::BlockingUse=ProposalBlock`도 초기 Research 유효성 자체는 막지 않는다. GR86 KR wheelbase Candidate, Venue rear 1546/1545 Candidate+WarningConflict/Unknown, HiLux approximate/max/range, 사용자 최종 모델·트림 수락 및 P0-04 영구 Evidence Authoring은 여전히 HOLD이다.
- **이력 보존:** 앞선 §3~§4의 ‘Validator NOT RUN’은 정적 입력안/Canonical 분류 당시 역사적 상태였으며 이 §5의 최신 PASS가 우선한다. 기존 다른 자동화 증거 덮어쓰기0, Git stage/commit/push0, GoPyMCP·Production UE DataAsset 변경0.

## 6. Citation ownership rebaseline (2026-10-19)

- Morning TireCode `clm-1-tirecode` rebinds to directly applicable OEM trim source `src-kia-morning27-price`, while all dimensions remain bound to `src-kia-morning27-spec`.
- GR86 Primary `ref059-coupe-gr86mt26om` continues to own Oman L/W/H and other OEM Oman facts. New Secondary `ref059-coupe-gr86kr-supp` (KR, unknown MY) owns source `src-toyota-gr86-kr-wheelbase` and Claim `clm-3-wheelbasemm` Candidate. Korean measurement is *not* an Oman exact physical specification.
- Six Payloads, Primary6, Secondary1, Source8, Claim84 (Canonical23/Candidate61), Unknown24, Venue Conflict1. The old focused UE validator PASS applies to v1.0.0 inputs only; the **changed v1.1.0 fixture was subsequently revalidated with a new EvidenceId, PASS (§7)**. EvidenceId/Recipe/Definition binding and persisted authoring are separate.

## 7. Changed-payload UE5.8 Validator acceptance & companion ownership contract (2026-10-19)

**Latest focused result PASS:** source `CFVehicleCore6EvidenceTests.cpp v1.1.0` with Morning direct Price Citation, GR86 Primary OM + Secondary KR ownership (Korean wheelbase Claim is Secondary Candidate), plus claim-to-source owner alignment assertions. Official UE 5.8 Editor build job `32768245e29b4dba9d9fdc0d88f5ed5c` exit0, DLL link PASS. Focused `CarFight.CCAS.CF_FQ_059.P0_02.Core6PayloadValidation` process `ea72db5bfe2a41ab87a3c8af5729dd3b` exit0, exact1/success1/failure0/missing0/unexpected0/duplicate0, new `EvidenceId=CF059_P002_CiteOwners_20261019_V2`, result `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_P002_CiteOwners_20261019_V2.json`, SHA-256 `0d3f8c8145bc97fba24b8e5b059aaf6d854ea79e7909b4e9231e87bdae1396f3`, actual JSON readback PASS.

**Companion ownership, current source contract:** `FCFVehicleRefEvidencePayload` contains no persistent GUID or Target binding. Existing `FCFVehicleAuthoringService::PreviewBuilderCompanions` / `CreateBuilderCompanions` are the only already verified new Evidence companion creation pathway, **not** independent direct Save. Requires managed persisted `UCFVehicleRecipeData` with valid `RecipeId`, bound loadable `UCFVehicleData`, fixed approved `NewEvidenceId`, `bHasInitialEvidencePayload=true`, empty/new noncolliding `EvidenceAsset` package/object identity; existing Evidence cannot be overwritten with new Research payload through companion create. Preview calculates prospective fingerprint from transient Evidence with `EvidenceId=NewEvidenceId`, `TargetRecipeId=Recipe.RecipeId`, `TargetRecipePath=Recipe exact object`, `TargetDefinitionPath=Target VehicleData exact object`. Commit requires **fresh OwnershipWrite R2 approval and matching proposal hash / stale Recipe & Target guards**, creates/updates missing Evidence and other private Builder companion profiles inside one transaction, marks packages Dirty and increments Recipe AuthoringRevision. **`bSavePerformed=false`; no automatic Save/Publish**, and package dirty is not durable persistence. Registry/VM lookup rejects ambiguous same Recipe Evidence and validates exact Recipe GUID, Recipe object path, Target Definition path and fresh fingerprint.

**P0-04 dependencies:** six Core6 Production managed Recipe/Target bindings, EvidenceId, exact asset identities, missing profiles, lawful R2 preview/approval/commit & separate save/readback have not been provisioned or accepted in this review. `ValidateInitialResearchPayload PASS` only proves typed research integrity, not `CreateBuilderCompanions` prerequisites, an accessible trusted Host R2 create contract, or USER final model/trim/physics acceptance. Existing dirty assets not touched; persistent Evidence remains **0/6**. Current 6 research payloads are **conditional ready for owner binding review**, not production authoring approved.

## Changelog

- v0.1.4 / 2026-10-19: Changed Core6 payload v1.1.0 official UE5.8 build/focused validator PASS with unique JSON SHA. Existing Builder companion persistent EvidenceId/Recipe/Definition R2 ownership gate inspected; save0, all six final authoring HOLD.
- v0.1.3 / 2026-10-19: Correct Morning TireCode Source and GR86 Korea Secondary claim+source scope, without production writes. New focused validation pending.
- v0.1.2 / 2026-10-19: Core6 실타입 C++ Validation 집중 검수 완료. 공식 UE5.8 Editor Build exit0, 새 isolated exact1 Automation PASS 및 JSON readback. Normal6+negative11 tests, persistent Asset 수정0. Full Evidence USER acceptance HOLD.
- v0.1.1 / 2026-10-19: Core6 84개 FACT를 조건부 source-level Canonical 연구 수락 검수. dimension23 Canonical / GR86 KR cross-market wheelbase1 Candidate / 다른60 Candidate; Venue rear-track warning unresolved 및 HiLux approximate 제원 한정 보존. Actual UE Validator·DataAsset NOT RUN.
- v0.1.0 / 2026-10-19: Core6 차량별 실제 `FCFVehicleRefEvidencePayload` 타입 기반 연구 입력안 분리, 출처·원단위·Unknown·Conflict 검수. 문서만 수정, C++/UE 에셋/Workbook/GoPyMCP/Git 작업 없음.
