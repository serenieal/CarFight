# CF-FQ-059 — Wagon Reference & Core8 Mesh Preparation Handoff

- 문서 버전: v0.1.3
- 작성일: 2026-10-13
- 작업 단계: **HISTORICAL P0-01 physical handoff; USER final mesh replacement/sizing/sockets DEFERRED. P0-02 reference/spec work proceeds without this checklist.**
- 차체 대상: **/Game/CarFight/Vehicles/Meshes 내 Core9 ONLY**, 그중 Wagon 1개 Reference Case + 나머지 8개 USER Mesh 준비 대상
- 근거: `VehicleContentAuthoringPlan.md v0.1.11`, `VehicleMeshInventory.md v0.1.7`, `VehicleRepresentativeSpecRoster.md v0.1.1`, `VehicleReferenceEvidenceDraft.md v0.1.0`, `Document/Systems/Vehicles/VehicleBuilder.md`, `VehicleData.md`, `VehicleCoreDecisions.md`
- 변형 금지: 이번 단계에 StaticMesh/Socket/휠, Production DataAsset, Recipe/Profile, Workbook, Source, Map 변경 없음

## Current Override — HOLD THIS HANDOFF UNTIL USER HAS REPLACEMENT MESHES (2026-10-13)

**USER가 이전 여섯 차체를 임시 메시라고 명시했다. 크기, Pivot, +X 방향, 바퀴 Socket 및 실제 새 메시 교체는 USER가 나중에 담당한다.** 이 체크리스트의 미작성 Socket4, ~100cm Body Bounds, Coupe Y-axis 및 기존 형상 관련 업무를 지금의 필수 Next로 요청하거나 P0-02 Reference/Schema 작업을 막지 않는다. 이 문서는 **나중에 최종 교체 메시가 준비됐을 때 사용할 historical handoff 자료**다. 현 단계 Next는 `VehicleReferenceEvidenceDraft.md v0.1.0`의 P0-02 source/claim work이다. 기존 설명 중 `before mesh editing` 선행수락 조건도 현재 Stage의 필수 승인 Gate가 아니라 **최종 실제 UE 자산 변경 시 검토할 항목**으로만 보존한다.

---

## Current USER Decision First — **historical pre-override** Representative Reference Sizes Before Mesh Editing (2026-10-13)

**USER의 최신 우선순위에 따라 차체 스케일/소켓 편집을 보류한다.** `VehicleRepresentativeSpecRoster.md v0.1.0`의 제조사 제원 검증된 CityCar(Kia Morning), Compact(Suzuki Jimny 5-door), Coupe(Toyota GR86), Pickup(Toyota HiLux SR5), SubCompact(Hyundai Venue), Van(Hyundai STARIA Hybrid Cargo)의 6종 대표차 identity/target dimensions를 먼저 검토·승인한다. **차체 외형과 해당 실차의 유사성은 미검증이므로 USER 수락 전엔 실차 target에 맞춘 회전/스케일 변경과 Socket 배치를 시작하지 않는다.**

USER가 대표 실차를 확정한 뒤에도 각 Body local AABB와 OEM L/W/H 비율이 달라, 먼저 올바른 Forward +X 및 Source Import Scale/Pivot/휠하우스 형상을 판단한다. `ReferenceSpec`은 마땅한 대표치수, `MeshMeasured`는 실제 Geometry, `CarFightTuned`는 나중의 의도적 게임 수치로 구분한다. 6개 Body Socket0 확정은 유지하며, USER 물리 메시 작업 담당 변경은 없다.

---

## 2026-10-13 LIVE UE Evidence — Confirmed Mesh Preparation After Reference Approval

**UE 연결 정상** / `StaticMeshTools.get_bounds`로 Core9 Body 9/9 Bounds 읽기 PASS / fresh managed `ue.batchdump_safe` dataset **21/21** (`adset_v1_356bb3fbfc606b87a694564a577f07aa.3e489c82d5fdba214fe539db`) / class9/9 / Socket current evidence 확보. 자세한 표·각 차체 Bounds 및 Socket 숫자는 `VehicleMeshInventory.md v0.1.5`의 최상단 **Fresh LIVE Unreal EVIDENCE**가 SSOT이다.

**Confirmed USER work required exact6 (each Body under `/Game/CarFight/Vehicles/Meshes/<Name>/<Name>`):**

| Body | Live local Bounds X×Y×Z cm | Live Sockets | 사용자 실물 메시 준비 절차 |
| --- | --- | ---: | --- |
| CityCar | 99.805×54.492×35.352 | 0 | import scale/축·피벗/휠하우스 실물 외형부터 확인 후 4 Wheel Anchor |
| Compact | 99.805×48.633×29.492 | 0 | 위와 동일, 전용 휠 존재하지 않음 |
| Coupe | **45.117×99.805×26.367** | 0 | **가장 긴 축 Y**; +X 전방 규칙과의 축 정합성/임포트 회전부터 확인 후 Wheel Anchor4 |
| Pickup | 99.805×43.164×28.320 | 0 | 스케일·전방·피벗 검토 후 Wheel Anchor4 |
| SubCompact | 99.805×49.414×33.008 | 0 | 스케일·전방·피벗 검토 후 Wheel Anchor4 |
| Van | 99.805×45.117×32.227 | 0 | 스케일·전방·피벗 검토 후 Wheel Anchor4 |

**DO NOT blindly scale by 4 or 5, or rotate Coupe**: 위는 현재 UStaticMesh local Bounds 측정이며 root Component/world transform이나 실차 identity/기하 정확성까지 판정한 결과가 아니다. 사용자와 GPT가 실제 외형/단위를 검증하여 임포트 scale 또는 콘텐츠 정리가 필요한지 먼저 결정한 다음, 실제 바퀴 중심을 기준으로 USER가 소켓을 생성·스케일하고 해당 asset만 저장한다.

**USER edit not required by Socket absence for exact2:** SUV(현재 6 sockets=Wheel4+HP2), Sedan(현재 7=Wheel4+HP2+FX1). 이미 four anchors 존재하므로 무작정 편집하지 않고 실제 wheel bounds·시각적 지면 접촉과 runtime를 추가 검수한다. **Reference Wagon**(현재 sockets7=Wheel4+HP3)은 기존 자산을 재작업 대상으로 두지 않는다.

**Physics wheel visual live evidence (FL only):** Shared Tire `Wheel_FL` 100×25×100cm, Wagon FL 100×25×100cm, SUV FL 79.36×35.48×79.36cm, Sedan FL 68.25×30.51×68.25cm. Wheel bounds는 사용된 메시의 로컬 Geometry이며 각 차종 Runtime에 어느 메시가 바인딩되는지 확정값이 아니다.

**Current per-lane status**: six Body `MeshPreparationRequired` because no Wheel Socket4; SUV/Sedan/Wagon have socket requirements read-back but `MeshReady` **NOT YET** due wheel/composition and category verification. I-01 class coverage **PASS**; I-02 wheel/scale/bindings **HOLD**; I-03 actual silhouette/category dedupe **HOLD**. No Production/Workbook/Source/StaticMesh mutation.

---

## 1. Historical previous recheck (pre-restored UE; superseded by current LIVE summary above)

- 2026-10-13 managed Git tracked package 재조회: Core9 차체명 `.uasset` **9개 존재**. **`.uasset`가 존재한다는 사실만으로 Unreal `StaticMesh` class/소켓/유효 geometry 확인이 되는 것은 아니다.**
- Stored AssetDump index fingerprint `7abf97d379deb846cf317eebcebe061c9bfeffb38a87afe4e08b2d905cc3c48b` 재조회: 현재 인덱스에서 Core9 `StaticMesh`로 확인된 Body는 **Wagon·SUV 2개**. 둘의 Socket detail은 재조회했다. **fresh Editor index가 아니다.**
- **Historical pre-recovery only:** UE read-only `ue_status` 당시 `ERR_UE_MCP_TIMEOUT`; 이번 2026-10-13 후속 LIVE 재조회는 연결/데이터셋/BodyBounds 9/9 PASS. 과거 기록은 상황 변화 증거로만 남긴다. 반복 재시작, GoPyMCP/AssetDump 변경 또는 에셋 편집으로 우회하지 않는다.
- 이전 실제 Vehicle Builder Wagon E2E 기록(`Document/Plan/Archive/VehicleBuilder/VehicleBuilderPlan.md`): 초기 당시 `Wagon/Van/SubCompact/Pickup/Coupe/Compact/CityCar` **소켓0**, 비교 대상으로 `Sedan socket7 / SUV socket6`을 관찰했다. **그 후 Wagon은 USER 편집으로 stored snapshot Wheel Anchor4를 확보했고, 이번 LIVE fresh index에서는 실제 total7 (Wheel4+HP3)을 확인했다.** 따라서 당시 나머지 6종 소켓0 기록은 *오래된 준비 이전 증거*이며 현재 여전히0인지 단정할 수 없다.
- **Historical provisional conclusion (now superseded):** Core8 전체를 당시 일괄 `MeshPreparationRequired`로 확정하지 않았다. **현재는 fresh evidence로 Core8 중 six exact confirmed `MeshPreparationRequired`**, SUV/Sedan은 Socket present. 분류는 **`Unverified` → fresh 실제 누락 확인 시 `MeshPreparationRequired` → USER 수정 저장 + fresh validator/readback 시 `MeshReady`**. 이 상태는 이번 Work의 **체크리스트용 판정 용어**이지 새로운 Engine/Recipe 상태 필드가 아니다.

## 2. Wagon 재사용 가능한 실제 기준

| 층위 | 읽은 데이터/확인된 사실 | 후속 차량에 사용할 규칙 |
| --- | --- | --- |
| 차체 Body | `/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon`, stored `StaticMesh`, fingerprint `F8C21192` | Asset class, bounds, wheel-center Socket 측정 **절차** 재사용. Wagon Body 규격 복사 금지 |
| Wheel Socket | `Wheel_Anchor_FL/FR/RL/RR` 네 개, 위치 FL(152,-84,17), FR(152,84,17), RL(-128,-84,17), RR(-128,84,17) cm, 각 Scale(0.8,1,0.8) | +X=차체 전방, -Y=좌측, +Y=우측; 각 차량 실제 바퀴 중심에 **USER가 별도로 위치/Scale 배치**. 이 좌표는 Wagon 고유 값 |
| Wheel source | `DA_Recipe_Wagon.AssetIntent.WheelMeshFL` 및 `DA_Vehicle_Wagon.VehicleVisualConfig.WheelMeshFL` = `/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL`; FR/RL/RR null fallback | FL 1개 공유 Wheel Mesh + 나머지 FL fallback은 **기존 정상 계약**. 타 차종도 이 경로 사용을 검토할 수 있으나 실제 Mesh bounds/휠하우스 적합성·우측 방향 보정 확인 전 **자동 결정 금지** |
| Wheel Size Authority | stored Recipe `WheelVisualIntent.Mode=SocketScaleFromChassis`; stored Target `bUseWheelSocketScale=true`, `bAutoScaleWheelMeshToRadius=false` | 휠 반경/폭은 실차 자료만으로 설정하지 않고 **휠 메시 bounds + USER Socket Scale**을 authority로 파생. 2026-08 historical live shared wheel bounds ≈(100×25×100)cm, Wagon Scale X/Z=0.8 → 역사적 radius≈40cm, width≈25cm. **현재 live Wheel bounds 미확인** |
| Wagon authored target | `DA_Recipe_Wagon` 및 `DA_Vehicle_Wagon`, `BaseVehicleMassKg=1886`, `MaximumGrossMassKg=2350`, WheelAnchor·Layout 적용, Drive/Handling/Performance/VehicleBase private profiles exact4, `BuilderDrivingAcceptanceReceipt` persisted | 기존 Authoring→Review/Apply→Save→driving receipt 체인 참조. 1886/2350과 차량 구동·핸들링 값은 **Wagon 고유 값**이며 다른 차에 복제하지 않는다 |
| Reference Evidence | `/Game/CarFight/Data/Authoring/Builder/Wagon_05F69DD3/DA_Ref_Wagon`; 주 Reference = **2026 Volvo V60 Cross Country Ultra B5 AWD, KR**; 국내 미공개 peak torque/gear ratios/final ratio는 명시적 Unknown, 일본·미국 참조는 secondary로 분리 | Mesh 실물 치수·형태를 먼저 확인한 후 차종별 실차 후보 선정. ReferenceSpec, MeshMeasured, Derived, CarFightTuned의 **출처/결정 구분** 재사용 |
| USER driving receipt | `DA_Recipe_Wagon.BuilderDrivingAcceptanceReceipt`에 exact Target 경로, DefinitionHash, Benchmark RunId persisted | 이전 Wagon 수락이 신규 차량 수락을 대신하지 않음. 신규 차량별 Save/benchmark/PIE USER 승인 필수 |
| Hardpoint | **Historical old snapshot: Wagon Body `HP_*` 없음; NOW SUPERSEDED by live HP3 `HP_Top_01`, `HP_Front_01`, `HP_Top_02`**; persisted VehicleData `HardpointSlots`에는 `Top_01/Front_01`와 `HP_Top_01/HP_Front_01` intent 존재 | Wagon 현재 Socket-vs-Profile fallback 정확성은 후속 확인할 항목. **초기 차체 준비 시 모든 차량에 HP/FX Socket을 강제하지 않음**; 전투장비 계획이 있으면 실제 연결 가능성을 확인 |

**중요:** Wagon의 `Wheel_Anchor` 스케일0.8 또는 `Reference 2026 Volvo` 결과가 모든 차종에서 정답이라는 뜻은 아니다. 과거 Wheel bounds ≈100×25×100cm는 historical verified live measurement이지 **이번 fresh measurement가 아니다**. 또한 `Wagon.Wagon`의 StaticMesh Socket Preview는 FR/RR에 legacy Police wheel reference가 관찰됐지만 이것은 preview 메타데이터이며 실제 Recipe Wheel binding이 아니다. 경찰차 자산은 CF-FQ-059 사용 대상이 아니다.

## 3. Core8 차량별 판정·USER 예상 준비 항목

`/Game/CarFight/Vehicles/Meshes/<Name>/<Name>.<Name>`의 아래 exact9 차체만 사용한다.

| 차체 | 과거/현재 증거 등급 | USER 메시 작업 준비 항목(확인 후 필요한 것만) | 현재 판정 |
| --- | --- | --- | --- |
| **Wagon (기준)** | LIVE Body StaticMesh + Bounds; Wheel Socket4+HP3=total7 | reference 사례, Socket 재작업 금지. Wheel/runtime evidence 추가 검수 | Reference / composition readiness pending |
| **SUV** | LIVE StaticMesh + Bounds; Wheel4+HP2=6 Socket | 기존 Wheel Anchor 재작성 금지; wheel/visual 확인 | Socket complete / composition pending |
| **Sedan** | LIVE StaticMesh + Bounds; Wheel4+HP2+FX1=7 Socket | 기존 Wheel Anchor 재작성 금지; wheel/visual 확인 | Socket complete / composition pending |
| **CityCar** | LIVE StaticMesh; Socket0; Body X≈100cm | import scale/axis 확인 후 USER Wheel Anchor4 + shared wheel fit 검토 | **MeshPreparationRequired** |
| **Compact** | LIVE StaticMesh; Socket0; Body X≈100cm | import scale/axis 확인 후 USER Wheel Anchor4 + shared wheel fit 검토 | **MeshPreparationRequired** |
| **Coupe** | LIVE StaticMesh; Socket0; **Body long axis Y≈100cm** | USER forward axis / import rotation/scale first, then Wheel Anchor4 | **MeshPreparationRequired** |
| **Pickup** | LIVE StaticMesh; Socket0; Body X≈100cm | import scale/axis 확인 후 USER Wheel Anchor4 | **MeshPreparationRequired** |
| **SubCompact** | LIVE StaticMesh; Socket0; Body X≈100cm | import scale/axis 확인 후 USER Wheel Anchor4 | **MeshPreparationRequired** |
| **Van** | LIVE StaticMesh; Socket0; Body X≈100cm | import scale/axis 확인 후 USER Wheel Anchor4 | **MeshPreparationRequired** |

**자료 경계 교정:** Sedan의 historical `socket7`이 **이번 2026-10-13 LIVE fresh AssetDump에서도 총7로 재확인됐다.** 이전 문장 `이번 AssetDump에서 미확인`은 pre-recovery snapshot 문맥으로만 유효하다. 또한 소재/폴더 유사명(CityCar/SubCompact/SUV, Coupe/Sedan)은 실제 geometry 같은지 증거가 없으므로 최종 카테고리 합병·변경 안 한다.

### USER에게 안내할 최소 실제 Unreal StaticMesh 편집 (누락이 확인된 차량만)

1. `Content Browser`에서 대상 **`/Game/CarFight/Vehicles/Meshes/<Name>/<Name>` StaticMesh**를 열어 외형·전방/좌우/중심·휠하우스를 시각 확인한다. **애셋 class가 StaticMesh가 아니면 편집 시작 전 GPT가 Type 판단**한다. 차체를 임의 복제·스케일 변경하지 않는다.
2. Static Mesh Editor의 **Socket Manager**에서 **`Wheel_Anchor_FL`, `Wheel_Anchor_FR`, `Wheel_Anchor_RL`, `Wheel_Anchor_RR`** 네 이름이 모두 있는지 확인한다. 없다면 **부족한 Socket만** 작성한다.
3. 위치는 각 **실제 바퀴 중심(회전축 기준)**에 사용자 시각 판단으로 설정한다. 기존 CarFight 기준 **+X 앞, -Y 왼쪽, +Y 오른쪽**. 배치는 cm로 저장된다. Wagon 값(FL 152/−84/17 등)을 복사하지 않는다.
4. 타이어 크기는 **선택된 휠 StaticMesh actual bounds와 Socket RelativeScale**로 결정한다. `SocketScaleFromChassis`에서는 **Scale X=Z**로 직경을, **Scale Y**로 폭을 정한다(양수·유효·프런트/리어 축의 요구 정합성 준수). `0.8,1,0.8`은 Wagon 사례일 뿐 공통 기본값이 아니다.
5. Wheel Mesh 선택: 차량 전용 휠이 있는 SUV/Sedan은 해당 차종 휠과 fallback/시각 방향을 검사한다. 다른 차종은 기존 `/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL`를 **후보**로 비교할 수 있다. Wheel FL binding과 Socket Preview Mesh는 서로 다른 소유권이므로 Preview에서 보여도 Recipe에 자동 적용된 것으로 가정하지 않는다.
6. **하드포인트는 필요할 때만** `HP_Top_01`, `HP_Front_01` 등 계획된 Mount별 Socket/Relative 위치와 Runtime fallback을 확인한다. 단순 차량 Spawn/주행의 초기 Mesh 준비에 일괄 필수 아님. Destroyed FX Socket도 정상 fallback이 가능한 계약을 먼저 확인한다.
7. USER가 **해당 StaticMesh(및 실제 편집한 Wheel Asset만)** 저장한다. 전체 Save All, 다른 Dirty 에셋 저장, Wagon·경찰차 에셋의 임의 수정은 금지한다. 편집/저장 후 GPT는 fresh Socket/Bounds/Recipe 참조를 다시 측정한다.

## 4. GPT 재확인 항목 / Gate handoff

| 검사 | PASS 가능한 근거 | 부족하면 다음 행동 |
| --- | --- | --- |
| B1 Body class & visible mesh | 현재 Editor AssetRegistry에서 대상 exact Body가 StaticMesh로 로드되고 치수/axis 확인 | `Unverified` 유지 / 실제 class 불일치면 별도 분류 |
| B2 Wheel Anchor 4개 | 현재 StaticMesh exact name/loc/rot/scale + 축별 front/rear/left/right 교차 확인 | 확인된 누락만 USER Socket 편집 인계 |
| B3 Wheel Mesh & bounds | actual Wheel FL/FR/RL/RR 또는 FL fallback, local wheel bounds·중심, 축·미러·회전 적용이 유효 | Shared wheel 도입/스케일 결정은 제안까지만, USER visual 확인 필요 |
| B4 Dimension & SocketScale | chassis full bounds·pivot/forward orientation, wheel anchor spacing, X/Z scale/width 파생 일관성 | 독립 측정되지 않은 `Socket` 거리·Authoring 좌표를 실차 치수로 둔갑 금지 |
| B5 Hardpoint / FX | **그 차량의 승인된 사용 계획에 필요할 때만** 실제 Socket vs valid fallback 확인 | 무관한 차량에 `HP_*` 일괄 작업 추가하지 않음 |
| B6 Body Category | 실제 silhouette/geometry로 Core9 duplicate/차종 정합성 확인 | 폴더명·Material 명만으로 서로 다른 차종 확정 금지 |
| B7 Fresh evidence | USER 저장 후 현재 exact 자산 지문·조회 시각·검증 결과를 독립 readback | 이전 저장 AssetDump로 `MeshReady` 승격 금지 |

**차량별 인계 상태**

- `Unverified`: 필수 결과가 일부 부족한 경우. **현재 Core9 class/bodyBounds/socket read는 모두 확인**했으며 특히 Core6 missing socket은 아래대로 confirmed. SUV/Sedan/Wagon의 Wheel visual/runtime readiness는 여전히 별도 검증 대기.
- `MeshPreparationRequired`: 최신 읽기에서 **실제 누락이 확인된 차**만. **Current exact6**: CityCar/Compact/Coupe/Pickup/SubCompact/Van Wheel Anchor4 missing. 누락 자산 정확한 경로, Socket 명, 필요한 편집·검수 방법과 함께 USER에게 인계.
- `MeshReady`: USER 저장 후 exact saved Mesh + Wheel deps의 fresh measurement/readback, 관련 Authoring validator가 일치하는 차량. 이것은 **메시 준비 게이트**이며 Production Vehicle/Driving acceptance 완료라는 뜻이 아니다.

**P0-01 I-01 PASS / I-02·I-03 HOLD**은 별개: I-01 Core9 class coverage, I-02 bounds/wheel/socket readiness, I-03 실제 형상 기반 카테고리/대표 중복 판정이다. USER 메시 작업을 필요로 하는 개별 차량 때문에 다른 차량의 inventory를 중단하지는 않지만, 전체 대표 차종을 임의 축소해 `P0-01 PASS` 선언하거나 `P0-02`를 시작하지 않는다.

## 5. 확인된 재사용 연결 / 다음 정확한 실행

```text
Wagon Current Recipe / Target / Ref Evidence / Acceptance   [Stored Evidence PASS]
                 |
                 v
Current CarFight Core9 Asset class / Body & Wheel Bounds / Socket re-read
                 |      (2026-10-13 UE MCP TIMEOUT → NOT RUN)
                 v
Individual Core8 missing-work report / USER StaticMesh edit & Save
                 |
                 v
GPT fresh asset readback → MeshReady per vehicle
                 |
                 v
Core9 silhouette/category and exact representative review → P0-01 Technical PASS
                 |
                 v
P0-02 이후 적법한 typed ReferenceSpec / CCAS / Product / Driving gates
```

**2026-10-13 이전 Blocker(해소):** 당시 Unreal Editor read-only 연결 불가였으나 이후 연결을 직접 확인하고 Live class/bounds/socket을 취득했다. 따라서 이 문서는 **실행 가능한 USER 체크리스트 + 과거 근거로 뒷받침된 준비 우선순위**이며, 현재 CarFight Core8의 실제 Socket이 비어 있다고 확정하는 검수 결과가 아니다.

2026-10-13 v0.1.0: Core9-only Wagon-first procedural reference, historical E2E vs stored snapshot temporal distinction, per-vehicle USER preparation handoff and objective MeshReady criteria documented. No Unreal/Workbook/Source/Production mutation.
