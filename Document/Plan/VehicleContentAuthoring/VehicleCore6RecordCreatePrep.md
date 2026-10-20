# CF-FQ-059 — Core6 신규 Definition·Recipe 생성 준비

- Version: **v0.1.1**
- Date: 2026-10-19
- Authority: CarFight / `project_id=carfight`, `repository_role=main_game`, branch `SwitchSourceEngine`, **UE 5.8 Source Build**
- Status: **Core6 6/6 exact R0 Preview PASS / official build PASS / 2026-10-19 R2 Create execution attempted but trusted R2 operation not exposed; actual Create0/6, Save0/6, Companion0/6**
- Sources: `VehicleCore6EvidencePayloads.md v0.1.4`, `VehicleCore6CitationAudit.md v0.1.1`, `VehicleRepresentativeSpecRoster.md v0.1.2`, `CFVehicleRecordIdentity.cpp v1.0.0`, `CFVehicleUXOps.cpp` current

## 1. 실제 생성 계약과 이번 준비 범위

기존 `FCFVehicleAuthoringService::CreateVehicleRecords()` **하나만** 활용해 새 `UCFVehicleData` Definition과 `UCFVehicleRecipeData` Recipe 1쌍을 생성하는 준비 작업이다. Core6 여섯 차량이므로 추후 별도 R2 Create 6번이 필요하며, 이번 작업은 **mutation0 R0 Preview 6/6만 실제 수행**했다. 기존 `Wagon`과 `TestSedan/TestSUV/DefenseSUV`의 Asset/Recipe는 보호하고 재사용하지 않는다.

**중요:** 이 writer는 두 개의 기록만 만든다. `TargetVehicleData` binding + Recipe `Managed` 상태, 프로젝트 기본 Sensor intent, 선택한 Transmission/Hardpoint mode만 지정한다. 차량별 물리 FACT·Evidence·전용 Profile·Mesh/Socket을 추론하여 채우지 않는다. 새 Definition은 C++ defaults로 출발하며 `bSavePerformed=false`; Dirty와 persisted save는 다르다.

## 2. 6개 정확한 VehicleId / 12개 생성 후보

각 VehicleId는 ASCII 영문·숫자·underscore만 사용하고 기존 `FCFVehicleRecordIdentity::BuildDefaultIdentity()`가 동일 패키지/오브젝트를 도출한다. `ReferenceVehicleId`는 Evidence 연구용 별도 key이며 `VehicleId`와 혼용하지 않는다.

| Core6 | VehicleId | Definition package & object | Recipe package & object | 기존 Research Primary |
| --- | --- | --- | --- | --- |
| CityCar | `Morning27_KR` | `/Game/CarFight/Data/Authoring/DA_Vehicle_Morning27_KR.DA_Vehicle_Morning27_KR` | `/Game/CarFight/Data/Authoring/DA_Recipe_Morning27_KR.DA_Recipe_Morning27_KR` | `ref059-citycar-morning27kr` |
| Compact | `Jimny5_AlphaAT_IN` | `/Game/CarFight/Data/Authoring/DA_Vehicle_Jimny5_AlphaAT_IN.DA_Vehicle_Jimny5_AlphaAT_IN` | `/Game/CarFight/Data/Authoring/DA_Recipe_Jimny5_AlphaAT_IN.DA_Recipe_Jimny5_AlphaAT_IN` | `ref059-compact-jimny5at-in` |
| Coupe | `GR86_2026_OM` | `/Game/CarFight/Data/Authoring/DA_Vehicle_GR86_2026_OM.DA_Vehicle_GR86_2026_OM` | `/Game/CarFight/Data/Authoring/DA_Recipe_GR86_2026_OM.DA_Recipe_GR86_2026_OM` | `ref059-coupe-gr86mt26om` |
| Pickup | `HiLux_SR5_AU` | `/Game/CarFight/Data/Authoring/DA_Vehicle_HiLux_SR5_AU.DA_Vehicle_HiLux_SR5_AU` | `/Game/CarFight/Data/Authoring/DA_Recipe_HiLux_SR5_AU.DA_Recipe_HiLux_SR5_AU` | `ref059-pickup-hiluxsr5at-au` |
| SubCompact | `Venue17_KR` | `/Game/CarFight/Data/Authoring/DA_Vehicle_Venue17_KR.DA_Vehicle_Venue17_KR` | `/Game/CarFight/Data/Authoring/DA_Recipe_Venue17_KR.DA_Recipe_Venue17_KR` | `ref059-subcompact-venue17kr` |
| Van | `StariaHybridCargo5_KR` | `/Game/CarFight/Data/Authoring/DA_Vehicle_StariaHybridCargo5_KR.DA_Vehicle_StariaHybridCargo5_KR` | `/Game/CarFight/Data/Authoring/DA_Recipe_StariaHybridCargo5_KR.DA_Recipe_StariaHybridCargo5_KR` | `ref059-van-stariahybridcargo5kr` |

이들은 **생성 후보 경로**이며 현 시점에서 존재하는 에셋이라고 주장하지 않는다. 가장 긴 Asset 이름은 32자이다. `PackageName`은 object suffix를 제거한 경로이고 `AssetName`은 그 마지막 이름이다. 기존 `/Game/CarFight/Data/Authoring/` root를 그대로 사용한다.

## 3. 모든 후보에 공통 적용하는 Request 값

| `FCFVehicleRecordCreateRequest` 필드 | 정확한 준비 값 |
| --- | --- |
| `DefinitionPackageName / DefinitionAssetName` | 위 표의 DA_Vehicle_* package/object |
| `RecipePackageName / RecipeAssetName` | 위 표의 DA_Recipe_* package/object |
| `ChassisMesh` | **null** — USER가 교체할 임시 Body 메시를 승인된 Product mesh로 자동 연결하지 않음 |
| `ProfileBindings` | **전부 null** (VehicleBase/Drivetrain/Handling/Performance/DriveState) |
| `bRequireVehicleSpecificTransmission` | **true** → 신규 Recipe는 `VehicleSpecificRequired` 정책 |
| `bRequireExplicitHardpointPlan` | **true** → 신규 Recipe는 `Unspecified` hardpoint plan mode |
| `CallerKind` | 실측 AI R0 Preview=`AI`; UI/Automation 비교 R0=`Automation` (테스트 내부 한정) |
| `bOwnershipWriteApproved` | **false** — 준비/Preview 단계에서 R2 위조 금지 |
| `ApprovalScopeHash` | **empty** — 이후 신뢰된 Create 직전 정확한 fresh ProposalHash 필요 |
| `NewEvidenceId / InitialEvidencePayload` | **Request 필드가 아님** — 생성 후 기존 Builder Companion 경로에서 별도 취급 |

신규 프로덕션 차량의 실제 섀시/휠/소켓은 나중에 승인된 사용자 교체 메시가 준비된 시점에 반영한다. Profile, 물리 질량·엔진 커브, 차량 크기, 하드포인트, 기어비를 임의 생성하지 않는다.

## 4. 차량별 Evidence 연구 제약 (새 Product 값으로 자동 전환하지 않음)

- **CityCar / `Morning27_KR`** — Kia Morning 2027 Trendy KR. Research `ref059-citycar-morning27kr`. 2027 Trendy / 14-inch; user production acceptance separate.
- **Compact / `Jimny5_AlphaAT_IN`** — Suzuki Jimny 5-door Alpha AT India. Research `ref059-compact-jimny5at-in`. 2024 brochure publishing date is not exact vehicle model-year.
- **Coupe / `GR86_2026_OM`** — Toyota GR86 2026 Oman 6MT. Research `ref059-coupe-gr86mt26om`. KR 2575mm wheelbase owns separate KR Secondary; Oman exact unknown.
- **Pickup / `HiLux_SR5_AU`** — Toyota HiLux SR5 4x4 AT V-Active Australia. Research `ref059-pickup-hiluxsr5at-au`. Dimensions approximate; curb maximum and GVM range not exact values.
- **SubCompact / `Venue17_KR`** — Hyundai Venue 1.6 IVT 17 KR. Research `ref059-subcompact-venue17kr`. Rear track 1546 vs 1545 same source; precise retail grade unknown.
- **Van / `StariaHybridCargo5_KR`** — Hyundai STARIA Hybrid Cargo5 KR. Research `ref059-van-stariahybridcargo5kr`. Hybrid system vs ICE vs motor output separate; exact MY unknown.

Core6 Research baseline: Primary6 + GR86 KR Secondary1, Source8, FACT84(Canonical23/Candidate61), Unknown24, Venue WarningConflict1. Morning price Citation과 GR86 KR Secondary ownership은 기존 집중 Validator v1.1.0에서 PASS했다. 이는 **이 신규 Definition+Recipe에 해당 Evidence가 자동 결속된다는 뜻이 아니다**.

## 5. 실제 인벤토리 및 collision 검사 증거

- 이전 fresh `/Game` class inventory: Recipe **4개** (`Wagon`, `TestSedan`, `TestSUV`, `DefenseSUV`), VehicleData **12개**, ReferenceEvidence **1개** (`DA_Ref_Wagon`). Core6 전용 Recipe/Definition/Evidence persisted **0/6**.
- 이번 fresh managed AssetDump: root `/Game/CarFight/Data/Authoring`, `changed_only=false`, **14/14 성공**, dataset fingerprint `d7a6227620404778081481dd547479e8cfb010c7e514fdb1524bcf1d96fdec`. 기존 Wagon Builder private Profiles / 기존 Recipes / Wagon Definition이 확인됐고, 위 신규 12개 후보 이름과 일치하는 기존 인덱스 자산은 **0개**.
- 최신 실제 R0 `PreviewAIVehicleRecords` / `PreviewVehicleRecords`에서는 **메모리 패키지 + 디스크 + UObject 경로 충돌 검사**까지 통과했음. Preview 직후 실제 새 Package도 생성되지 않음을 검증. 단, 이후의 Create 시점에도 fresh Preview를 재실행해야 하며 지금 받은 hash/사전승인을 재사용하지 않는다.

## 6. 실측 R0 focused test 및 실행 경계

- 신규 **test-only** `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFCore6CreatePreviewTests.cpp v1.0.0`: Core6 6개 VehicleId로 기존 helper의 정확한 package/object를 생성해 두 Preview (AI R0 + Automation R0)의 path/hash/승인 요구/Save0 일치를 검증. Asset Creation, Actual R2 Create, Save 호출은 없음.
- UE5.8 공식 Editor Build job `3b10cc8ec4ef4f07ac403e3747f0dbd5` **PASS exit0**, 테스트 C++ 컴파일 및 DLL 링크 성공.
- 실제 focused Automation: `CarFight.CCAS.CF_FQ_059.P0_04.Core6RecordPreview` one selected test, 내부 6종 **6/6 PASS**. Process `3b48af0e8a674371a78b9b4028eef694` **PASS exit0**, requested1/success1/failure0/missing0/unexpected0/duplicate0. 독립 `EvidenceId=CF059_Core6_RecordPreview_20261019_V1`. JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_Core6_RecordPreview_20261019_V1.json`, SHA-256 `02e3f8bfd2ef0fb7106382b71c09ad21c7a6f1d8dc9d52cd4c9d7d5b79024201`, process.status JSON readback PASS.

## 7. 기존 Create 실행 시점의 정확한 작업 단위

1. **한 차량 단위 R0:** 위 동일 ID/Definition/Recipe/flags에 대해 `PreviewAIVehicleRecords` 또는 기존 인증 UI route에서 현재 path/collision, ProposalHash를 fresh 재확인.
2. **R2 승인된 내부 실행:** `CreateVehicleRecords`는 기존 소스상 `SlateUI`/`Automation` caller에서만 직접 사용 가능하며, AI R0 Preview 결과 자체로 R2 Create 권한이 생기지 않는다. 인증된 Host Trusted AI R2 경계가 확보되기 전 GPT가 caller를 `Automation`으로 위장·Boolean/Hash 자가주입하지 않는다. 외부 전용 Writer/승인 시스템을 새로 만드는 대신 P0-03C 기존 계약을 재사용한다.
3. **한 쌍의 Create:** 명시적 OwnershipWrite와 exact fresh proposal이 유효한 한 호출에서 Definition+Recipe를 생성. `TargetVehicleData`의 exact path/객체, `RecipeId` 유효성, `ManageState=Managed`, null Chassis/Profile, BasicSensor, `VehicleSpecificRequired`/`Unspecified`, Definition/Recipe Dirty/Save0 결과 readback. Builder adoption은 생성과 별도 terminal 결과이며 사후 오류 때문에 이미 생성된 레코드를 자동 삭제하지 않는다.
4. **영구 저장:** 별도 승인된 Save + 재조회가 끝나야 저장된 두 기록이라고 보고. 최종 제품 수치나 물리 자산 검증은 아직 아니다.
5. **Research Evidence Companion:** 저장/식별된 exact Recipe GUID 및 Target VehicleData를 owner로 `NewEvidenceId`, Evidence path, InitialResearchPayload를 나중에 결속해 기존 `PreviewBuilderCompanions`→허용된 `CreateBuilderCompanions` 후 별도 Save/readback. Production/Workbook/Catalog/Publish는 여전히 독립.

**Current decision:** `SIX CORE6 R0 PREVIEW PASS / CREATE INPUT READY / TRUSTED AI R2 CREATE NOT RUN / PERSISTED PAIRS 0/6`. 사용자 최종 exact grade/physics와 physical mesh는 이번 두 레코드 생성 준비와 별개로 계속 관리한다.

## 8. 실제 R2 Create 요청 실행 환경 점검 — 2026-10-19

**실행 판정: NO R2 OPERATION DISPATCHED / NO NEW VEHICLE RECORDS.** 사용자는 앞선 Core6 6종 생성 준비 이후 실제 R2 Create 실행을 명시 요청했다. 기존 입력·승인 범위를 다른 새 기능 개발 없이 실행하려고 공식 Project 운영 도구를 조사했다.

- **Authority:** Company `CarFightMCP_Admin`, project=`carfight`, repository_role=`main_game`, branch=`SwitchSourceEngine`. Project Registry의 공식 `editor.start` operation을 1회 호출했다. 관리형 process job `4346b78a686f499faac1afe6b1e15683` **succeeded/exit0**. Exact checkout=`checkout_eefe2944e0954bed9270908eb95d8fd0`, runtime=`runtime_50d8c839234241c0b9ed3d4d3ebc59e0`, Editor PID=46680, `editor_ready=true`, `project_runtime_ready=true`, UE Bridge `connected`, protocol_ready=true. 서버/프로젝트 연결은 복구됨.
- **Trusted call contract review:** 해당 Ready Runtime에서 `CarFightMCP_UE_WriteProbe.ue_list_toolsets` 53개 exposed UE toolset 목록 확인. **CarFight `CreateVehicleRecords` 전용 R2 operation/toolset 없음.** Generic `DataAssetTools.create`는 `CreateVehicleRecords()`의 atomic Definition+Recipe, Managed/Target binding, exact R2 approval/transaction/fresh ownership semantics를 구현하지 않으므로 사용하지 않았다. Public `SlateInspector.Observe`는 server policy `ERR_UE_TOOL_BLOCKED`를 반환해 기존 Builder UI 상호작용 경로도 현 tool authority로 열리지 않았다. `SlateInspector.Snapshot` R0는 성공했지만 쓰기/조작 허가는 아니다.
- **Current C++ guard preserved:** `PreviewAIVehicleRecords()`는 AI mutation0, `CreateVehicleRecords()`는 내부 SlateUI/Automation caller + fresh ProposalHash + explicit OwnershipWrite에서만 실행. User request 자체를 `CallerKind=Automation`/`SlateUI`로 포장하거나 신규 Automation test에서 Production assets를 생성하는 우회를 하지 않았다. `process.run` 공식 project op에는 `editor.start` 등이 등록되지만 Vehicle authoring R2 Create command는 등록되지 않았다.
- **Live UE AssetRegistry post-attempt readback:** official exact Ready Runtime `AssetTools.find_assets` root=`/Game/CarFight/Data/Authoring`: existing Definition=`DA_Vehicle_Wagon` 1; Recipe=`DA_Recipe_Wagon`, `DA_Recipe_TestSedan`, `DA_Recipe_TestSUV`, `DA_Recipe_DefenseSUV` 4. Core6 six nominated Definition/Recipe **0/12**. No Create writer invoked, no new dirty packages, no Save/Workbook/Catalog, no existing assets altered. Editor is left running by successful `editor.start`; no stop/restart/kill attempted.
- **Actual required next integration:** Continue the independent P0-03C **existing trusted Host→CarFight R2 adapter/operation dispatch** contract work under the owning project/session (not GoPyMCP generic feature coding inside CarFight). Once exposed and verified against exact Project/Checkout/Runtime/caller, call the existing single writer for one Core6 pair at a time with fresh preview, exact ownership scope, terminal receipt/unknown-outcome checking and readback. **Do not add a second VehicleRecord writer** and do not let this capability gap make P0-02 Evidence research or P0-04 R0 preparation regress.

## Changelog

- v0.1.1 / 2026-10-19: Actual R2 Create attempt investigated. Editor managed start+UE Bridge Ready PASS; public CarFight R2 writer unavailable and Slate UI write tool blocked. Live AssetRegistry Core6 0/12, actual Create0, Save0. No fake Automation caller or generic data asset write.
- v0.1.0 / 2026-10-19: 기존 canonical `CreateVehicleRecords` 진입 계약과 6개 VehicleId/12개 패키지 결정, fresh inventory 충돌 검수, 정확한 Blank Chassis/Profile과 차량별 Transmission/Hardpoint 공통 정책, 실측 공식 UE5.8 Build + 6종 R0 Preview PASS. R2/Save/Companion/Production mutation0.
