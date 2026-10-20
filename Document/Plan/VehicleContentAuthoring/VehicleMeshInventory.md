# CF-FQ-059 — VCCAS-P0-01 Vehicle Mesh Inventory / Category Roster

- 문서 버전: v0.1.7
- 작성일: 2026-10-12
- 상태: **HISTORICAL P0-01 placeholder UE inventory (Body Class9/9 PASS); I-02/I-03 temporary physical checks USER-DEFERRED / NON-BLOCKING for current P0-02 reference work**
- 대표 계획: `VehicleContentAuthoringPlan.md v0.1.11`
- Project: `carfight` / `main_game` / `SwitchSourceEngine` / `checkout_eefe2944e0954bed9270908eb95d8fd0`
- 변경 범위: Plan/UDS 문서만. Unreal asset/Workbook/Source/Runtime 변경은 없음.

---

## Current Override — USER temporary meshes deferred (2026-10-13)

**USER confirmed all currently inspected six chassis meshes are temporary placeholders to be replaced by USER later.** Do NOT revisit old mesh shape, body size, Socket0 or geometry dedupe in order to progress six reference candidates, P0-02 Reference Research, or P0-03 schema. Present AssetDump and AABB findings below are historical state of disposable placeholder assets, **not criteria for the future vehicle definitions**. Physical mesh/wheel/Socket integration remains required for later actual Production/Runtime technical gates when replacements exist. Current next = `VehicleReferenceEvidenceDraft.md v0.1.0` claim work. Existing UE assets untouched.

---

## 2026-10-13 Core6 Reference-First Spec Decision — **historical pre-override**

Before editing six socket-empty imported chassis meshes, USER requested real-model-first dimensional targets. **Six external manufacturer L/W/H/Wheelbase exact values have been verified** and recorded in `VehicleRepresentativeSpecRoster.md v0.1.0` with exact trim/market sources and existing 2026-10-13 body bounds comparisons. **Vehicle/mesh pairing is proposed, not USER-accepted; do not distort/scale meshes or move UE assets yet.** First USER review and accept/replace proposed six real cars; then individually evaluate scale, forward axis and pivot, edit Body/Socket and fresh re-measure. I-01 still PASS, I-02/I-03 HOLD.

---

## 2026-10-13 Fresh LIVE Unreal EVIDENCE — Current authority (supersedes older snapshot below)

**Connection is restored**: exact `ue_status` from approved UE read-only bridge returned `connected` / `initialized=true`, `ue_list_toolsets` succeeded, and `AssetTools.find_assets/get_asset_class` + `StaticMeshTools.get_bounds` worked against the actual CarFight Body assets. Managed `ue.batchdump_safe` for `/Game/CarFight/Vehicles/Meshes`, `class_filter=StaticMesh`, `changed_only=false`, `include_details=true` returned **21/21 succeeded** and a fresh, queryable managed dataset `adset_v1_356bb3fbfc606b87a694564a577f07aa.3e489c82d5fdba214fe539db`. This dataset has **Core9 Body StaticMesh exact9 + dedicated Wheel StaticMesh exact12**. Unlike prior stale AssetDump, all Core9 are now *actual live Class verified*, with full Body local bounds measured by approved UE tool.

| Core candidate | Body local Bounds size X × Y × Z (cm, rounded) | Fresh total Sockets | Wheel Anchor | Current User Mesh Handoff |
| --- | ---: | ---: | --- | --- |
| CityCar | 99.805 × 54.492 × 35.352 | **0** | **missing four** | **MeshPreparationRequired**: inspect actual import scale/forward axis/geometry then USER author Wheel Anchor4 |
| Compact | 99.805 × 48.633 × 29.492 | **0** | **missing four** | **MeshPreparationRequired**: scale/orientation review before USER Socket4 |
| Coupe | **45.117 × 99.805 × 26.367** | **0** | **missing four** | **MeshPreparationRequired**: longest axis Y, not standard forward X; inspect forward-axis, import/orientation/scale before USER Socket4 |
| Pickup | 99.805 × 43.164 × 28.320 | **0** | **missing four** | **MeshPreparationRequired**: scale/orientation review before USER Socket4 |
| SUV | 453.274 × 210.152 × 149.596 | **6** | **four present** | Current Socket readiness evidenced; actual Wheel Mesh/bounds/visual/geometry still needs evaluation, no mandatory socket rewrite |
| Sedan | 488.710 × 203.800 × 125.390 | **7** | **four present** | Current Socket readiness evidenced; Wheel Mesh/visual readiness and category comparison pending |
| SubCompact | 99.805 × 49.414 × 33.008 | **0** | **missing four** | **MeshPreparationRequired**: scale/orientation review before USER Socket4 |
| Van | 99.805 × 45.117 × 32.227 | **0** | **missing four** | **MeshPreparationRequired**: scale/orientation review before USER Socket4 |
| Wagon | 487.092 × 233.052 × 138.746 | **7** | **four present** | Reference Case / HP3 now confirmed; fresh Wheel/bounds visual readiness still needs validation |

**Critical distinction:** Body bounds dimensions are **asset local-space bounds**, not a verified in-game world size, vehicle mass, real-wheelbase, or accepted import scale. Six Body candidates have around 100cm on their longest axis and Coupe has longest axis **Y**. This strongly signals a **scale/forward-axis review** before user positions sockets, but does not independently prove their import settings or justify automatic scaling or rotating source assets. USER decides actual physical edits; GPT will remeasure.

**Fresh Socket details:**

- Wagon: Wheel FL(152,-84,17), FR(152,84,17), RL(-128,-84,17), RR(-128,84,17), all Scale(0.8,1,0.8); **three Hardpoint Sockets**: `HP_Top_01 (-50,0,130)`, `HP_Front_01 (168.204,0,80.332)` Scale(0.9,0.9,1), `HP_Top_02 (-142,12,136)` Scale(0.5,0.5,1). **Current exact7, older snapshot exact4 is obsolete**; former `HP_* socket0` mismatch inference is no longer valid.
- SUV: Wheel Anchor FL(135,-76,16), FR(135,76,16), RL(-123,-76,16), RR(-123,76,16) scale1, and `HP_Front_01`, `HP_Top_01`; total6.
- Sedan: Wheel Anchor FL(156,-75,7), FR(156,75,7), RL(-133,-75,7), RR(-133,75,7) scale1, and `HP_Top_01`, `HP_Front_01`, `FX_Destroyed`; total7.
- CityCar/Compact/Coupe/Pickup/SubCompact/Van: **fresh StaticMesh socket0 confirmed**; no need to guess from old historical reports.

**Current live wheel local Bounds representative FL:** `Shared/Tire/Wheel_FL` 100 × 25 × 100 cm, `Wagon/Wheel_FL` 100 × 25 × 100 cm, `SUV/Wheel_FL` 79.36 × 35.48 × 79.36 cm, `Sedan/Wheel_FL` 68.25 × 30.51 × 68.25 cm. Measurements are **per mesh** and not proof of effective Recipe/runtime source for each vehicle. Dedicated 12 wheel classes are verified by fresh dataset, but all four wheel bounds and front/rear/righthandedness/visual checks have not all been completed.

**P0-01 evidence gates, after live rebaseline:**

- **I-01 PASS — Core9 body AssetClass and class coverage**: 9/9 actual live `StaticMesh`, dedicated wheels 12/12 actual class `StaticMesh`, Shared FL can be live measured. Other shared wheels only if required by current binding.
- **I-02 HOLD — actual physical mesh readiness**: 9/9 Body local Bounds measured, complete Chassis Socket inventories obtained. Confirmed **six** chassis without wheel sockets require USER mesh preparation. SUV/Sedan/Wagon have all Wheel Anchor4 but are not automatically `MeshReady`; wheel, geometry orientation/scale and typed runtime verification remain.
- **I-03 HOLD — semantic category/geometry dedupe**: local Bounds + folder names do not establish independent categories. Do not publish nine unique real silhouettes without actual shape review.

**Current exact next:** USER receives six confirmed Body mesh preparation requests; **before editing sockets**, validate six candidates' true import scaling and forward axis (not just dimensions). SUV/Sedan/Wagon require no blind socket addition. USER changes/saves only exact relevant assets, GPT fresh reads and verifies before advancing. No Production or Workbook/Source mutation.

---

## Current USER scope — ONLY CarFight Core9 (2026-10-13)

**CF-FQ-059의 First Wave 차량 차체 선정/실차 제원 조사/Vehicle CCAS 제작 대상은 `/Game/CarFight/Vehicles/Meshes/` 폴더의 body-named candidate exact9로 한정한다.** 해당 9개 경로는 아래 2절의 CityCar/Compact/Coupe/Pickup/SUV/Sedan/SubCompact/Van/Wagon이다. 이는 **9개 서로 다른 실제 차종 또는 9개 유효한 StaticMesh 확정**을 뜻하지 않는다.

- **OUT OF SCOPE:** `/Game/CarFight/_Legacy/police_car`의 Legacy Police Car 및 `/Game/Vehicles/OffroadCar`, `/Game/Vehicles/SportsCar`, `/Game/Models/SportsCar` 등 CarFight core 후보 외부 차체. Template/example 차량을 대체 후보나 추가 차종으로 검토하지 않는다. 신규 외부 메시를 보충·수입하지 않는다.
- **Current live verified candidate counts: Core9 Body StaticMesh exact9/9**, fresh dedicated Wheel StaticMesh exact12/12; six Body Socket0. Above **Fresh LIVE Unreal EVIDENCE** owns current readiness. Historical old index Body2 is preserved only for temporal comparison. 최종 서로 다른 차종 수와 Production 적격 수는 미확정이다.
- **Shared wheel exception (not a chassis exception):** `/Game/CarFight/Vehicles/Shared/Tire/Wheel_*`은 위 9개 차체의 기존 wheel dependency로만 조사 가능하다. 이 예외는 새 차체를 범위에 추가하지 않는다. Legacy 경찰차 휠을 가리키는 기존 Wagon Socket Preview 경로는 *변경 금지된 관찰된 의존성*으로 기록하며 경찰차를 신규 차량 제작 범위에 포함하는 근거가 아니다.
- **Historical evidence**: 앞선 조사에서 수집된 `/Game` asset index exact37, Legacy 경찰차 및 template paths는 삭제하지 않고 아래 Historical context로만 보존한다. 현재 후보·검증 관문·승격 차종 수에 산입하지 않는다.
- **Current I-01 PASS; I-02/I-03 remain HOLD.** Live Core9 body class/Bounds/Sockets are read. Six missing Wheel Anchor4 and six likely scale/axis review require USER work; wheel-visual and silhouette dedupe are not yet verified. No UE/Workbook/Source mutation.

---

## Current execution policy — Wagon-first, USER performs Core8 physical mesh preparation (2026-10-13)

현재 Core9 중 **Wagon은 이미 Vehicle Builder/실차 증거/주행 수락 경험이 있는 Reference Case**다. 다른 8종의 Mesh에는 아직 편집이 많이 이뤄지지 않았다는 USER 보고에 따라, GPT는 실제 StaticMesh/Socket을 임의 편집·저장하지 않는다.

**Current live Wagon/Core8 USER Handoff:** `VehicleMeshPreparationChecklist.md v0.1.1` (2026-10-13). UE live connection and full Core9 class/bounds/socket query now PASS; **six Socket0** confirmed. Previous `ERR_UE_MCP_TIMEOUT` is historical, not the current connection status.

**Evidence-confirmed Wagon inputs:** `/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon` (persisted AssetDump Body StaticMesh), `DA_Recipe_Wagon`, `DA_Vehicle_Wagon`, Wagon Builder Evidence/DrivingAcceptance receipt. `Wheel_Anchor_*`의 Socket 좌표와 스케일 및 Wagon 실제 실차 근거를 기존 제작 Workflow의 구현 참고로 사용한다. **Wagon 차축 거리·질량·휠 사이즈·엔진 수치를 다른 차의 기본값으로 복제하지 않는다.** Fresh Wagon `HP_*` Socket3가 검증돼 이전 persisted index의 `HP_*` socket0은 오래된 정보로 교정됐다. Shared Wheel FL/Socket Preview cross-reference는 별도 확인 대상이다.

| CarFight core body | 현재 메시 근거 | 현 작업 분류 | GPT→USER 준비 인계 |
| --- | --- | --- | --- |
| Wagon | **Fresh StaticMesh exact7 sockets** (Wheel4+HP3), Body bounds live | **Reference Case — Wheel/Runtime validation pending** | Socket 재추가 금지, 기존 제작 절차·휠 source를 참고 |
| SUV | **Fresh StaticMesh socket6 (Wheel4+HP2)**; Body bounds live, dedicated wheels4 class verified | **Existing wheel sockets verified** | Wheel/visual runtime validation only; existing Socket 일괄 재작업 금지 |
| Sedan | **Fresh StaticMesh socket7 (Wheel4+HP2+FX1)**; Body bounds live, dedicated wheels4 class verified | **Existing wheel sockets verified** | Wheel/visual runtime validation only; existing Socket 일괄 재작업 금지 |
| CityCar, Compact, Coupe, Pickup, SubCompact, Van | **Fresh StaticMesh exact6; Socket0 exact6**, live Body Bounds ~100cm longest dimension; Coupe long axis Y | **MeshPreparationRequired — confirmed** | USER first review asset scale/forward orientation and mesh geometry, then add `Wheel_Anchor_FL/FR/RL/RR` at actual wheel centers; avoid assuming Wagon dimensions |

- **GPT 담당:** 각 기존 메시 read-only inventory → Wagon을 참고한 개별 차체 준비 체크리스트/위치 판단 근거 제시 → USER가 저장한 메시를 fresh re-read·검증 → 허용된 다음 Gate에서 실차 Reference Evidence/Recipe/VehicleData 및 Production 기술 검증.
- **USER 담당:** 다른 Core8의 실제 Chassis Mesh·휠 Mesh·Socket/Anchor·Scale 및 필요한 경우 Hardpoint 배치/에셋 수정·저장, 최종 USER Driving/Feel 검수. USER가 해야 할 것과 GPT가 이미 측정 가능한 것은 분리한다. 모든 차에 hardpoint를 무조건 요구하지 않는다.
- **차량별 상태 언어:** `Unverified`(fresh class/geometry 없음), `MeshPreparationRequired`(실제 확인된 누락으로 USER 편집 대기), `MeshReady`(USER 저장 뒤 fresh read-only 검사와 관련 validator 통과). 단순히 `SUV` 외 차종이니까 자동 `MeshPreparationRequired` 판정을 만들지 않는다.
- 준비되지 않은 한 차량 때문에 Core9 전체의 read-only 검사를 멈추지 않는다. 미준비 차량을 임의 Generated Vehicle/Production으로 만들거나 Wheel/Socket에 가상의 값을 삽입하지 않는다.
- **Current Gate:** Core9 ONLY, P0-01 I-01~I-03 evidence HOLD unchanged. 사용자 메시 편집은 **별도 작업 인계**이며 이 문서 교정 자체에서는 UE asset/Workbook/Source 변경하지 않는다.

---

## 0. Historical USER scope correction — Police Car excluded (superseded by Core9-only decision on 2026-10-13)

USER 결정: **Legacy Police Car와 그 차체/휠은 CF-FQ-059 First Wave 및 후속 Vehicle CCAS 온보딩의 선택·제작·재사용 후보에서 제외한다.** 경찰차의 기존 에셋이나 타 기능을 삭제·변경하지 않는다. 이미 취득한 AssetDump 사실은 아래 Historical Evidence로만 보존하며 경찰차에 대해 추가 Asset Class/소켓/바운드 측정, 복구, 신규 Recipe/VehicleData 생성, Production 등록, 실차 reference 조사는 수행하지 않는다.

- **Historical 2026-10-12 intermediate in-scope body-named package candidates = 11 (SUPERSEDED on 2026-10-13)**: 당시 core9 + template Offroad/SportsCar2였으나 **현재 범위는 Core9 ONLY**이며 template 2는 더 이상 선택 가능한 후보가 아니다.
- **Historical 2026-10-12 intermediate class coverage (SUPERSEDED)**: 당시 verified body SUV/Wagon2, class 미검증 body9(core7+template2)였다. **2026-10-13 Current Core9 기준은 verified2, 미검증 core7**이다.
- **Historical stored discovery에는 Police body1 및 wheel4가 포함돼 있으므로 기존 raw `/Game` StaticMesh indexed37, 차량 차체 발견3, 차량 wheel 발견12라는 기록은 변경하지 않는다.** 그 수치는 과거 조사 범위이고 이번 선정 범위의 숫자가 아니다.
- Wagon body `PreviewStaticMesh`가 Legacy Police 휠을 가리키는 관찰은 *Wagon의 독립적인 바인딩 정합성 조사 참고 증거*로만 유지한다. 이 참조로 경찰차를 제작 후보에 복귀시키거나 경찰차 에셋을 변경하지 않는다.

---

## 1. Historical 2026-10-12 조사 기준·증거 등급 (superseded by Fresh LIVE EVIDENCE above)

실제 Project의 물리 asset을 숫자만 세어 "StaticMesh 검증 완료"로 오판하지 않는다.

- **A — Persisted AssetDump verified**: 기존 AssetDump index `/Game`의 class-filter `StaticMesh`에 등록되고, 해당 asset_sections summary/details를 읽은 asset. Asset index fingerprint: `7abf97d379deb846cf317eebcebe061c9bfeffb38a87afe4e08b2d905cc3c48b`. 이 자료는 **stored snapshot**이지 이번 세션의 fresh Editor 재생성 결과가 아니다.
- **B — Managed Git tracked package**: `CarFightMCP_Admin.git_exec(["ls-files",...])`에서 실제 `UE/Content` 추적 경로로 확인한 `.uasset`. 파일명·폴더가 차량 Body/StaticMesh를 시사해도 **Editor asset class, socket, bounds/valid geometry를 검증한 것은 아니다**.
- **C — Persisted VehicleData/Recipe intent**: 기존 `UCFVehicleData` / `UCFVehicleRecipeData`의 stored `data_asset_values`로 확인한 authored reference/pose/derived parameter. 참조 문자열이 있다는 것은 참조 대상의 실재·class·load success 증명이 아니다.
- **D — UI/shape candidate**: category folder/model/material naming으로 임시 분류. 실차 차종·제원·실제 silhouette 일치의 증거가 아니다.

조회 증거:

1. Persisted AssetDump `discover(root="/Game",class_filter="StaticMesh",discovery_max_items=100)`: indexed StaticMesh **exact37 / returned37 / omitted0 / pagination0**; 이 중 `CarFight/Vehicles` 및 `_Legacy`에 있는 차량 차체 **3**, 해당 네 바퀴 **12**, 기타 Projectile/Turret/Generated/Map mesh **22**. Index `/Game/CarFight/Vehicles` root는 indexed 31 assets (모든 class).
2. Managed Git `ls-files` read-only: `UE/Content/CarFight/Vehicles/Meshes`, `Shared/Tire`, `_Legacy/police_car/StaticMeshes`, `UE/Content/Vehicles`, 별도 `SM_*` / `SKM_*` file-pattern 검토. **프로젝트 파일의 tracked 존재와 stored dump가 불일치함이 증명됨**.
3. `asset_sections`에서 SUV/Wagon/LegacyPolice 차체 socket summary/digest/details, 12개 기존 indexed wheel summary, Wagon/SUV/Sedan VehicleData 및 Wagon/Sedan Recipe typed values 읽음.
4. Unreal Editor MCP `ue_status`: Timeout → 재확인 `ERR_UE_MCP_UNAVAILABLE`. `ue_batchdump_safe(root="/Game/CarFight/Vehicles/Meshes",class_filter="StaticMesh",changed_only=false,managed_dataset=true)`: `ue_managed_dataset_prepare_failed`. Sedan/CityCar/SportsCar asset_sections 재조회는 `ue_assetdump_evidence_source_missing`. **실시간 Class/Bounds/Socket·Index refresh는 이번 실행에서 확보하지 못함**.

> 결론: `37 StaticMesh`는 **현재 인덱스에 포함된 집합의 exact count**일 뿐 **프로젝트 전체 StaticMesh exact count**가 아니다. Git의 `.uasset` 추적 이름을 곧바로 Unreal `StaticMesh` class라고 승격하지 않는다.

---

## 2. Historical 2026-10-12 candidate roster — 9 tracked (class/socket column superseded by LIVE table above)

아래 asset object path는 Git-tracked `.uasset` 파일명에서 계산한 **조회 후보 identity**이다. AssetDump Class Verified 칼럼이 `No`이면 해당 object path 로드/AssetClass/geometry가 아직 확인되지 않았다는 뜻이다.

| 임시 Category | Body object candidate | Tracked dedicated wheels | AssetDump Body class | Socket evidence | Representative decision |
| --- | --- | --- | --- | --- | --- |
| CityCar | `/Game/CarFight/Vehicles/Meshes/CityCar/CityCar.CityCar` | 0 | No / missing indexed source | Unknown | **Candidate exact1**, verify type+shape first |
| Compact | `/Game/CarFight/Vehicles/Meshes/Compact/Compact.Compact` | 0 | No / missing indexed source | Unknown | **Candidate exact1**, verify type+shape first |
| Coupe | `/Game/CarFight/Vehicles/Meshes/Coupe/Coupe.Coupe` | 0 | No / missing indexed source | Unknown | **Candidate exact1**, verify type+shape first |
| Pickup | `/Game/CarFight/Vehicles/Meshes/Pickup/Pickup.Pickup` | 0 | No / missing indexed source | Unknown | **Candidate exact1**, verify type+shape first |
| SUV | `/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV` | 4 (FL/FR/RL/RR) | **Yes / StaticMesh** `35D7C167` | **6 sockets: 4 wheels + 2 HP** | **Primary verified body candidate** |
| Sedan | `/Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan` | 4 (FL/FR/RL/RR) | No / missing indexed source | Existing VehicleData/Recipe poses only | **Priority candidate** with existing authoring; asset facts pending |
| SubCompact | `/Game/CarFight/Vehicles/Meshes/SubCompact/SubCompact.SubCompact` | 0 | No / missing indexed source | Unknown | **Candidate exact1**, verify type+shape first |
| Van | `/Game/CarFight/Vehicles/Meshes/Van/Van.Van` | 0 | No / missing indexed source | Unknown | **Candidate exact1**, verify type+shape first |
| Wagon | `/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon` | 4 (FL/FR/RL/RR) | **Yes / StaticMesh** `F8C21192` | **4 wheel sockets; no HP socket in snapshot** | **Primary verified body candidate**; authored Wagon exists |

Wheels exact12 under those folders:

- `/Game/CarFight/Vehicles/Meshes/SUV/Wheel_{FL,FR,RL,RR}`, fingerprints `6655B4D7 / 0A1A4979 / 3BB611C7 / 8A15D754`.
- `/Game/CarFight/Vehicles/Meshes/Sedan/Wheel_{FL,FR,RL,RR}`, **Git-tracked only**, AssetDump source missing.
- `/Game/CarFight/Vehicles/Meshes/Wagon/Wheel_{FL,FR,RL,RR}`, fingerprints `A0E4E076 / 5256A5A0 / 5AAF68AF / 8E12D415`.

Shared wheel exact4: `/Game/CarFight/Vehicles/Shared/Tire/Wheel_{FL,FR,RL,RR}` are Git-tracked and **absent from current AssetDump index**. Existing Wagon Recipe and Wagon VehicleData point to the Shared `Wheel_FL`; the other three Wheel fields are unset in indexed data. Do not assume the four dedicated Wagon wheels are effective Runtime bindings.

**Folder naming vs actual design:** CityCar/SubCompact/SUV material filenames share `armored_suv_3d_model`; Sedan/Coupe share `futuristic_car_3d_model`; Compact material mentions `military_off-road`; Wagon material mentions `futuristic_truck`. Consequently, the nine folder labels are **nine candidate lanes, not yet nine distinct verified vehicle silhouettes or real-world categories**. Comparison of full mesh shape/resource identity and dedupe is required before promoting the category exact count.

---

## 3. Excluded police and template/sample resources — HISTORICAL evidence ONLY

| Location/role | Tracked body candidate | Adjacent resources | Persisted verification | Decision |
| --- | --- | --- | --- | --- |
| Legacy Police Car (**OUT OF SCOPE**) | `/Game/CarFight/_Legacy/police_car/StaticMeshes/Combined/Combined_Body.Combined_Body` | `Wheel_{FL,FR,RL,RR}` exact4 | Historical stored evidence: Body **StaticMesh** `D2D968C3`, socket **0**, all 4 wheels indexed | **USER EXCLUDED**. No selection/repair/reuse/mutation in CF-FQ-059; historical evidence only |
| Template Offroad (**OUT OF SCOPE**) | `/Game/Vehicles/OffroadCar/SM_Offroad_Body.SM_Offroad_Body` | `SM_Offroad_Tire`, `SKM_Offroad`, AnimBP/CtrlRig/physics | Historical managed Git tracked only; no AssetDump asset sections | **USER EXCLUDED** from CF-FQ-059 body/First Wave selection; historical evidence only |
| Template SportsCar (**OUT OF SCOPE**) | `/Game/Vehicles/SportsCar/SM_SportsCar.SM_SportsCar` | `SM_SportsCar_Glass`, `SM_SportsCar_Wheel`, `SKM_SportsCar`, `SK_SportsCar` | Historical managed Git tracked only; no AssetDump asset sections | **USER EXCLUDED** from CF-FQ-059 body/First Wave selection; historical evidence only |
| Separate Models/SportsCar (**OUT OF SCOPE**) | `/Game/Models/SportsCar/SKM_SportsCar` | Skeletal-named asset | Historical Git tracked only | **USER EXCLUDED** from CF-FQ-059; no typed verification required |

The **historical repository cross-inventory** contains **core folder9 + Legacy1 + template2 = 12 distinct tracked body-named packages**. The 2026-10-12 intermediate scope had core9+optional template2=11; it is **superseded**. The **current USER-approved selection scope is core9 ONLY**, stored AssetDump-verified core chassis2 and core class-unverified7. Historical out-of-scope package counts must never be presented as current First Wave candidates. `SM_SportsCar_Glass` is a part, not an additional vehicle chassis. Projectile/weapon/FX and map-generated StaticMesh are excluded from vehicle roster.

---

## 4. Actual socket / geometry evidence from stored AssetDump

### 4.1 SUV — proven StaticMesh sockets (Unreal cm)

`/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV` / `asset_0143` / fingerprint `35D7C167` / socket count **6**

| Socket | X | Y | Z | Scale |
| --- | ---: | ---: | ---: | --- |
| `Wheel_Anchor_FL` | 135 | -76 | 16 | (1,1,1) |
| `Wheel_Anchor_FR` | 135 | 76 | 16 | (1,1,1) |
| `Wheel_Anchor_RL` | -123 | -76 | 16 | (1,1,1) |
| `Wheel_Anchor_RR` | -123 | 76 | 16 | (1,1,1) |
| `HP_Front_01` | 156 | 0 | 92 | (1,1,1) |
| `HP_Top_01` | -61 | -1 | 147 | (1,1,1) |

Socket-anchor **derived** geometry: axle X separation `258 cm` (2.58m); left-right Y separation `152 cm` (1.52m). Both are **socket coordinates**, not verified car wheelbase/tire contact width from mesh bounds or real-world reference. Four wheel `StaticMesh` classes are indexed and their section summaries report socket0; wheel local bounds/radius not available in current AssetDump sections. `DA_TestSUV` persisted `VehicleVisualConfig` binds all 4 indexed SUV wheels and has wheel-anchor values consistent with the persisted body sockets. Existing DA `BaseVehicleMassKg=0`, `MaximumGrossMassKg=0`: not ready as fitting-balanced Production Vehicle merely because the body and wheels exist.

### 4.2 Wagon — **historical** 4-wheel-only StaticMesh Socket snapshot (superseded: fresh total7 includes HP3)

`/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon` / `asset_0154` / fingerprint `F8C21192` / socket count **4**

| Socket | X | Y | Z | Scale |
| --- | ---: | ---: | ---: | --- |
| `Wheel_Anchor_FL` | 152 | -84 | 17 | (0.8,1,0.8) |
| `Wheel_Anchor_FR` | 152 | 84 | 17 | (0.8,1,0.8) |
| `Wheel_Anchor_RL` | -128 | -84 | 17 | (0.8,1,0.8) |
| `Wheel_Anchor_RR` | -128 | 84 | 17 | (0.8,1,0.8) |

Socket-anchor **derived** X separation `280 cm` (2.80m), Y separation `168 cm` (1.68m). Socket scale is not wheel size; wheel mesh bounds are still required.

Current indexed `DA_Recipe_Wagon` binds `Wagon.Wagon` chassis and `/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL`; `DA_Vehicle_Wagon` binds same chassis/shared wheel FL with others unset. It has `BaseVehicleMassKg=1886`, `MaximumGrossMassKg=2350`, `UseHardpoints` intent and authored `HP_Top_01/HP_Front_01` HardpointSlots. **Current Wagon body dump has no `HP_*` socket**; this is a mismatch to inspect using existing Builder socket/fallback semantics, not permission to silently edit the mesh. Existing Wagon body Socket Preview references mix Wagon FL and Legacy Police FR resources (FR/RR point to Legacy Police). Preview metadata is not proof of actual runtime wheel assignment.

### 4.3 Sedan — VehicleData/Recipe-authored only, NOT mesh-measured

`DA_Recipe_TestSedan` and `DA_TestSedan` persistently refer to `/Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan` and four dedicated Sedan wheels, and `Wheel_Anchor_*` names. `DA_TestSedan` authored front X=156, rear X=-133, left/right Y=±75, Z=7cm → **authored layout estimate** `289cm longitudinal / 150cm lateral`. No persisted AssetDump Sedan body class, socket or bounds is currently available. Do **not** publish these as actual Sedan StaticMesh socket measurements. Existing mass fields are 0/0, and recipe still has legacy profile policies without accepted Evidence/Driving receipts.

### 4.4 Police Car — historical evidence only / USER EXCLUDED / no further investigation

`/Game/CarFight/_Legacy/police_car/StaticMeshes/Combined/Combined_Body.Combined_Body` / `asset_0204` / fingerprint `D2D968C3`: indexed `StaticMesh`, socket count `0`. Four related indexed wheel meshes fingerprints `99D2993F / 0A21F7E0 / 3E026437 / 6957E3EC` (FL/FR/RL/RR). Full body bounds and wheel mesh local bounds not available. Existing `DA_PoliceCar` values were unavailable due to source parse failure; this remains historical context. **USER scope exclusion overrides any readiness/recovery follow-up for Police Car in CF-FQ-059.**

### 4.5 Required measurements still missing (do not invent)

- Body full local bounding box / centroid / axis conventions / import scale for **all 9 in-scope CarFight core body-named candidates** (including indexed SUV/Wagon). The existing AssetDump StaticMesh `summary/details` contract exposes socket transforms but not full `UStaticMesh::GetBounds()` values.
- Wheel bounds, extent/radius/width and effective runtime binding for **core9 lanes and their approved wheel dependencies only**. Police Car and external template bodies/extra wheels are excluded as new selection candidates.
- Fresh loaded `UStaticMesh` class + socket/mesh section evidence for the **seven core body candidates not yet AssetDump verified**: CityCar, Compact, Coupe, Pickup, Sedan, SubCompact, Van.
- Recheck category silhouette/dimensions. File/folder names and material path alone cannot prove 9 distinct silhouettes.
- Fresh body/socket fingerprint consistency and explicit Socket-vs-Recipe Hardpoint/missing wheel treatment.

Reusable existing read-only seam is `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAssetReader.cpp`: `BuildAssetSnapshot` reads loaded `UStaticMesh` chassis sockets and wheel `GetBounds()` to stable snapshots/fingerprints without editing assets. It does **not** expose complete chassis bounds today; for body bounds use **an existing approved read-only UE asset info capability** when Editor Ready, or treat body bounds as uncovered (do not expand plugin/UE Source in this inventory-only gate). Existing `FCFVehicleAuthoringService::ReadMeasurementProposals` can be used once a Recipe/asset can be loaded with current typed permission.

---

## 5. Provisional First Wave decisions (NOT Production publish)

1. **Primary core tracks exact9**: retain one tracked candidate body per named folder for follow-up typed load, with SUV and Wagon as verified Body StaticMesh anchors. Sedan is third priority due to persisted Recipe/VehicleData intent and dedicated 4 wheel packages; it is not yet StaticMesh-verified.
2. **Six core body-only packages** (CityCar, Compact, Coupe, Pickup, SubCompact, Van) are provisional. No dedicated wheel package in their folders; existing Shared Tire wheel reuse is *an option to evaluate*, not an accepted binding.
3. **Legacy Police Car is USER EXCLUDED** from this Work's category roster and Production/upgrade/reuse candidates. Its old socket and asset evidence remains historical context only; do not reopen readiness gates for it.
4. **Offroad/SportsCar sample vehicles are USER EXCLUDED**, not auxiliary alternatives. Excluded source content remains untouched and is not investigated or adopted for CF-FQ-059 chassis selection.
5. Category count and eligible representative exactN remain **provisional** until class validation + body shape dedupe + wheel/socket/bounds verification. Do not fill gaps by importing new meshes; do not choose real make/model/year/trim in P0-01.

### Remaining P0-01 evidence gates

| Gate | Current evidence | Required to close |
| --- | --- | --- |
| I-01 Complete managed asset class coverage | **In-scope tracked core9** vs AssetDump-verified **2** (SUV/Wagon); Editor MCP unavailable | Fresh Unreal AssetRegistry/class query or accepted refreshed AssetDump covering **only `/Game/CarFight/Vehicles/Meshes` core9**; unverified core7, not Police/template chassis |
| I-02 Wheel/socket/bounds measurable readiness | SUV/Wagon sockets A; Sedan C; wheel package list B; **body bounds unknown all** | Approved current read-only UE measurements for chassis bounds, wheel local bounds and required sockets, per candidate; validate missing wheel fallback explicitly |
| I-03 Category/representative semantic dedupe | core9 folder labels D, materials partly overlap | verify shape/geometry for **core9 only**, then select one representative per distinct actual category; Police/template alternatives do not participate |

**Decision:** `VCCAS-P0-01 PARTIAL EVIDENCE / GATE HOLD`. We finished **stored-index and managed-repository cross-inventory** and documented the provisional roster, but cannot legitimately report a fresh current Project StaticMesh exactN, completed wheel/body measurements, or final First Wave category count while all three acceptance evidence gates remain open. This is an **environment/evidence limitation**, not a redesign of P0-00, nor authorization to mutate Production.

**Exact next action inside P0-01:** restore **existing** approved Unreal Editor read-only connectivity (no GoPyMCP/AssetDump plugin repair in this CarFight Work) → gather fresh AssetRegistry / AssetDump targeted query for **core9 only** body-named candidates and related CarFight wheel resources (**Police/template vehicle body candidates excluded**) → capture chassis bounds, wheel extents/sockets and actual resource class → dedupe categories and freeze **one true representative per actual category** → P0-01 fresh technical review. P0-02 real-spec authoring remains NOT OPENED.

---

## 6. Guardrails & changelog

- `VCCAS-P0-00 v0.1.3 DESIGN PASS` frozen; CCAS Core / Vehicle Builder / `FCFVehicleApplyService` / `RuntimeTestCatalog` authority preserved.
- No `UCFVehicleData` or Weapon/Vehicle Product mutation, `.xlsx` edit, StaticMesh/socket edit, Blueprint/C++ edit, plugin or runtime source change.
- Existing unrelated Dirty remains owned by existing tasks; no Git stage/commit/push.
- 2026-10-13 v0.1.7: USER confirms all six inspected body meshes disposable placeholders and personally owns later swap/size/Socket updates. P0-02 research now independent of source shape; I-02/03 placeholder checks deferred to physical integration; never fake MeshReady. Plan v0.1.11 and ReferenceEvidenceDraft v0.1.0.
- 2026-10-13 v0.1.6: USER Core6 representative reference size-first decision; six manufacturer verified L/W/H/Wheelbase benchmarks, precise market/trim and local geometry risk documented in `VehicleRepresentativeSpecRoster.md v0.1.0`. Real model-to-Body visual match requires USER acceptance before mesh editing; no existing I-01/I-02/I-03 verdict altered.
- 2026-10-13 v0.1.5: UE read-only `connected` + Core9 StaticMesh 9/9 class and body local bounds fresh measured; managed AssetDump fresh21/21 with Socket inventory Wagon7, SUV6, Sedan7, six no sockets. Historical Wagon HP absence corrected, USER prep exact6 confirmed; I-01 PASS / I-02,I-03 HOLD. No UE Product or Source mutation.
- 2026-10-13 v0.1.4: Wagon/SUV stored socket re-read, Wagon Recipe/VehicleData/RefEvidence/Driving receipt 재조사, historical Builder E2E 당시 일부 Core 차량 Socket0과 이후 Wagon Socket4의 시점 차이를 교차 검증. `VehicleMeshPreparationChecklist.md v0.1.0` 별도 USER 준비 가이드 생성; UE live timeout이므로 Core8 current readiness는 모두 Unverified, P0-01 I-01~I-03 HOLD 유지.
- 2026-10-13 v0.1.3: USER directs Wagon as existing reference implementation and reserves **actual Core8 mesh/socket/wheel work for USER**, with GPT read-only evidence, per-vehicle preparation guide, fresh result validation and subsequent typed authoring. Preserve partial Wagon/SUV data, don't copy Wagon specs; Core9 scope and I-01~I-03 HOLD unchanged.
- 2026-10-13 v0.1.2: USER freezes CF-FQ-059 vehicle chassis selection to **`/Game/CarFight/Vehicles/Meshes` tracked Core9 ONLY**; Police/Offroad/SportsCar external chassis excluded. In-scope AssetDump verified2, unverified7, pending I-01~I-03 unchanged. Past discovery numbers preserved as historical evidence only; no asset mutation.
- 2026-10-12 v0.1.1: USER excludes Legacy Police Car from CF-FQ-059 authoring/Production/representative selection; keep historical observations; intermediate scope of core9 + optional template2 = 11 was **superseded on 2026-10-13**; I-01~I-03 remain HOLD. No asset mutation.
- 2026-10-12 v0.1.0: tracked repository and stored AssetDump inventory cross-audit, provisional exact9 core candidates + legacy/template alternatives, measured socket facts vs authored layout separated, unresolved freshness/measurement gates documented.
