# CarFight Data Authoring Plan

- 문서 버전: v0.2.53
- 작성일: 2026-08-17
- 문서 상태: Done / Historical + Retained Path / DEL1~DEL7 PASS / Legacy Wizard Retired / UA-08 Quantitative Comparison Deferred

- 대상 프로젝트: CarFight
- 대상 엔진: Unreal Engine 5.8 Source Build
- 역할: CarFight 데이터 제작 환경의 목표, 책임 경계, 보호 조건, 단계별 승인 Gate를 정의하는 대표 계획 문서
- 구현 우선 대상: Vehicle
- 후속 확장 후보: Weapon / Equipment / Scanner / Armor
- 상위 Vehicle 제작 Consumer: `CF-FQ-040 Guided Vehicle Builder` — Data Authoring은 Recipe/Resolver/Diff/Validation/Apply/Undo Backend와 Advanced 유지보수 Workspace 역할을 유지

---

## 2026-09-11 Final Closure

`CF-FQ-038 Vehicle Data Authoring`은 현재 범위를 완료해 **Done / Historical + Retained Path**로 전환한다. 현재 구현 계약은 main_game `Document/Systems/Vehicles/VehicleBuilder.md v1.5.0`이 소유한다.

```text
DAUTH-P0-08A~M / P0-09 / P0-10 / P0-11: Technical PASS 보존
DAUTH-P0-12: Closed with Deferred Feel Comparison
Historical USER Acceptance: 7 of 8 보존
UA-08: Runtime Technical PASS / USER Inconclusive / quantitative comparison Deferred
Deprecated Gate DG1~DG5: PASS
Delete Gate DEL1~DEL7: PASS
DEL6 closure: Legacy hidden spawner/open entry/test access 제거 + UE/Source direct reference exact0 + Legacy Wizard Source 3개 physical retirement
Final Official UE 5.8 Build: 1fd947042024420aaa7561d380e4548f PASS / Exit 0
Full Regression: CarFight.DataAuthoring 111/111 PASS / failure 0 / Engine Exit 0
Current owner: main_game Document/Systems/Vehicles/VehicleBuilder.md v1.5.0
Placement: Historical + Retained Path / G0~G4 PASS / G5 Deferred
```

첫 broad regression의 Wagon 2건 실패는 DEL6 회귀가 아니라 persisted Wagon의 current `AppliedDefinitionHash=8aad29d04e008fcd2acb2e6470cd87f1` 대비 과거 test snapshot `0e5b...` 기대값이 stale했던 문제로 분리했다. test-only expectation만 current persisted receipt에 맞춰 교정했고 Product VehicleData/Recipe/Profile Asset의 강제 Apply·Save·schema/content migration은 0이다.

UA-08 quantitative comparison은 사람 체감의 불확실성을 정직하게 보존하는 비차단 observational debt다. 이를 USER PASS로 승격하지 않으며 역사적 P0-12 USER PASS 7/8은 그대로 유지한다. 이 Deferred 항목 하나만으로 CF-FQ-038을 다시 Paused/Active로 되돌리지 않는다.

---

## 2026-08-26 Vehicle Builder Consumer Role Decision

USER 승인으로 차량 신규 제작의 정상 UX는 `CF-FQ-040 Guided Vehicle Builder`가 소유한다.

```text
Guided Vehicle Builder
= 실존 차량 Reference 기반 AI proposal + 단계형 차량 제작 정상 UX

Data Authoring
= Recipe / Resolver / SourceTrace / Diff / Validation / Apply / Undo / Drift Backend
= 필요할 때 사용하는 Advanced / maintenance Workspace

VehicleData
= Runtime 최종 Authority
```

이 결정은 CF-FQ-038의 기존 Technical/USER evidence를 취소하지 않는다. 기존 `CarFight 차량 데이터 제작` Workspace를 삭제하거나 UA-01~08을 replay하지 않는다. 기존 4축 Driving Feel은 Builder의 원천 실차 데이터가 아니라 optional semantic bias / Quick Tune으로 재사용할 수 있다.

`CF-FQ-040`은 Wheel center 자동검출과 Hardpoint 자동배치를 범위에서 제외한다. USER가 Mesh/Wheelbase/Socket을 직접 준비하고 Builder는 필요한 이름·위치 기준·검사를 안내한다.

---

## 2026-08-18 Pause Checkpoint — Historical

사용자가 현재 프로젝트 최우선순위를 인게임 UI(`CF-FQ-032`)로 재지정해 `CF-FQ-038`을 일시 정지한다. 이 전환은 Data Authoring의 완료된 기술 상태를 취소하거나 재검증하는 의미가 아니다.

```text
보존 Technical PASS: DAUTH-P0-08A~M / P0-09 / P0-10 / P0-11
Historical P0-08 schema evidence: 117 Registry leaf / Performance numeric 17 / five-Profile numeric 78
Current additive compatibility: RedlineStartRPM 포함 118 Registry leaf / Performance numeric 18 / five-Profile numeric 79
Bounded compatibility PASS: Registry dacb88cb09644d0287cfe7a7f3137b51 / Batch projection 03aad228b1784f1b908ed6e82ddced3e / Allowlist c9d07c42428642c186743e8c26043dcb — 각 1/1 PASS
보존 Final Build: 98dfceab797942218bd09c844e398ceb PASS
보존 Full Data Authoring: 77b45525549b4866995b3d972fb4a9fa / 63/63 PASS
재개 Gate: DAUTH-P0-12 USER Authoring Acceptance
재개 지점: DG/DEL Gate — UA-08은 transient Sports 4축을 실제 PIE Runtime 13개 Movement field에 적용하는 Technical PASS를 확보했으나 USER 주행 체감은 민감도 부족으로 Inconclusive. 사용자는 정량/계측 비교를 Deferred하고 다음 단계 진행을 승인함 / P0-12 USER PASS는 7 of 8 그대로 유지 / UA-01~07과 UA-08 Runtime technical evidence는 새 failure evidence 없이는 replay하지 않음
UA-06 USER fixture correction: 최초 `Profile.Handling.FrontWheelMaxBrakeTorque 2000→2100` 후보는 실제 Source Trace에서 `Legacy Imported Pinned Baseline — Recipe.ImportState.LegacyPins`가 effective source이고 Handling Profile은 비적용 후보임이 확인돼 review 단계에서 USER가 `아니오`로 취소 / 실제 mutation 0. 대체 fixture `/Game/Test/CarFightDataAuthoring/DA_UA06_Performance`는 실행 시점 `DA_TestSedan` Performance baseline을 상수 Feel response + direct 값으로 복사해 Profile 연결 자체 Diff 0을 목표로 구성 / `RedlineStartRPM`은 Legacy Pin 없음·현재 Project CppDefaults source·PerformanceProfileDirect owner라 정확한 single-field Source edit 후보. official fixture build `4a07fa4dc9674746ba56b3a2b14457a1` Exit 0 PASS / headless fixture process `9a5f6b110d534fbcb23167576157a1c8` Exit 0 PASS / formatting-only cleanup 뒤 final exact-source official build `7601a83504bc476ab789d7b9ddd7baef` Exit 0 PASS / persisted dataset `adset_v1_82ec6901a50bcb39c1e2a603df40afe4.5cd21309982cced85d66af9e` exact values PASS / Production Recipe isolation dataset `adset_v1_bc9499346bf725234107edc0f6109209.eaed116e9a498d04ffc2fd0c`에서 Performance binding 없음 유지 / USER가 `DA_UA06_Performance` reviewed binding을 승인했고 fresh Workspace에서 binding 자체 diff0 확인 PASS
UA-07 Remote Technical Readiness: PASS — USER 순서는 변경하지 않음 / `/Game/Test/CarFightDataAuthoring/DA_UA07_Handling`, `DA_UA07_Performance` test-only fixture persisted / Production Recipe binding 0 / `UnbindVehicleProfile` R1 Recipe-only baseline 복구 경로 추가 / final official build `627107d9de2d49be9be552c0baf2875a` Exit 0 / focused `ProfileUnbind` process `e999340c380b4e18af218b57c81dcab3` 1/1 PASS / USER PASS 확대 0
USER PASS: 7 / UA-01 Workspace First Impression USER PASS / UA-02 Browser·Mesh-only·Create·Existing Import USER PASS / UA-03 Recipe·Shared Profile·Affected Vehicle Impact USER PASS / UA-04 External Drift Recovery USER PASS / UA-05 Resolve·Diff·Source Trace·Validation USER PASS / UA-06 Explicit Apply·Undo USER PASS / UA-07 Driving Feel Authoring USER PASS
Deprecated Gate: DG1~DG5 PASS / `CarFight.VehicleDAWizard` 기본 Window 메뉴 제거 Source 적용 / hidden tab spawner 유지
Official validation: 이후 동시 CF-FQ-039 작업의 full Official Editor Build PASS로 shared compile blocker가 해소된 상태를 보존. DAUTH 변경 이후 fresh Editor가 updated `CarFightReEditor` binary를 정상 로드함
Focused validation: `CarFight.DataAuthoring.DAUTH_P0_09.Workspace.TabRegistration`은 UE Automation log에서 exact 1/1 `Result={Success}` PASS. `Workspace.LegacyManagedGuard`는 이번 deprecation에서 `SCFVDAWizardTab`/guard logic mutation0이므로 기존 PASS를 보존하고 replay하지 않음
Live menu validation: fresh Editor `창(Window)` menu에서 `CarFight 차량 데이터 제작` 존재, `CarFight Vehicle DA Wizard` 부재를 Slate read-only Snapshot으로 직접 확인 PASS
Delete Gate: DEL1~5·DEL7 충족 가능 / DEL6는 hidden compatibility spawner + Legacy guard test direct ref가 의도적으로 남아 Pending / SCFVDAWizardTab 물리 삭제 금지 유지
```

Historical note: 아래 Pause Checkpoint의 재개 지시는 2026-08-18 당시 상태다. 2026-09-11 Final Closure가 현재 lifecycle을 대체하며 P0-12/DEL Gate를 다시 자동 진행하지 않는다.

### 2026-08-19 UA-03 transient affected-inventory handoff

```text
Observed USER flow
- persistent DA_Recipe_TestSedan에 Handling + DriveState Profile binding 완료
- 검증: 정상
- Handling Profile FrontWheelMaxBrakeTorque=2000 B2 재시도
- B2 preflight가 /Engine/Transient.DA_Recipe_TestSedan을 affected Recipe로 수집해 잘못 Block

RCA
- B2 GatherAffectedRecipes의 live TObjectIterator fallback이 rollback/prospective RF_Transient scratch까지 실제 Recipe로 포함
- Frozen B2 validation 의미 자체는 정상

Implemented source correction
- CFBatchImport.cpp v1.1.2: RF_Transient 또는 GetTransientPackage() scratch 제외
- /Temp·/Game의 non-transient unsaved Recipe inventory는 유지
- CFVehicleAuthoringVMTests.cpp v1.7.0: invalid transient scratch가 살아 있어도 SharedProfileImpact preview/commit이 persistent Recipe만 대상으로 성공해야 하는 regression 추가

Validation status
- build 26a64c03b02843d798676b60c9ace1af: CFBatchImport.cpp + CFVehicleAuthoringVMTests.cpp compile PASS
- final DLL link: running Editor가 UnrealEditor-CarFight_ReEditor.dll 점유 → LNK1104 / Exit 6
- 코드 오류 판정 아님, final official build PASS는 아직 미확정
- 사용자 save: DA_Recipe_TestSedan + DA_Profile_UA_TestDriveState exact 2 assets
- DA_Profile_UA_TestHandling은 B2 mutation0 상태로 저장 불필요
- 사용자 Editor 정상 종료 완료

Closure continuation result
- final official build `3fa7b4d4cca14d37b7fea50631e4fa07`: Exit 0 PASS / ReEditor DLL 정상 link
- focused `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.SharedProfileImpact` process `db57ea0701234e65b8e5921b1dce1e96`: 1/1 PASS / 0 FAIL / Engine Exit 0
- fresh Editor `d0e658ca67504b0d889b4c4f54a3ebf1`: Ready / port 8100 owner / protocol ready PASS
- fresh persisted AssetDump `adset_v1_4f3bcdd508d406c6efb5890b27a612cc.ca3fc2fde0f211cd52688d79`: Recipe Handling=`DA_Profile_UA_TestHandling`, DriveState=`DA_Profile_UA_TestDriveState`, DriveStateMode=VehicleSpecific 확인
- USER B2 one-shot preview: FrontWheelMaxBrakeTorque=2000 / affected Vehicle 1 / expected VehicleData change 0 / Auto Apply 0 / Auto Save 0 PASS
- USER actual commit 후 live dirty boundary: `DA_Profile_UA_TestHandling=true`, `DA_Recipe_TestSedan=false`, `DA_TestSedan=false`, `DA_Profile_UA_TestDriveState=false`
- transient Recipe 재등장 0 / Frozen B2 validation 완화 0
- UA-03 = USER PASS / P0-12 USER PASS 3

UA-04 USER Acceptance result
- single-field baseline Apply → same-field Raw 950 external edit → refresh에서 External Drift 감지 PASS
- unreviewed External Drift가 normal Apply를 차단 PASS
- 3-way review에서 Last Applied=925 / Current Raw=950 / Current Authoring=925 표시 PASS
- USER: 세 값과 세 recovery 선택지의 개별 의미는 이해 가능
- Keep Authoring review token → explicit final Apply → External Drift 0 / pending diff 0 복구 PASS
- USER UX verdict: 전체 Authoring 흐름은 거시적으로 복잡함. UA-04 USER PASS로 확대 금지
- P0-12 USER PASS는 3 유지

UA-04 macro-flow UX remediation design — Frozen
- Core/VM/Resolver/ApplyService 의미 변경 0 / Slate presentation 중심 UI-only remediation
- External Drift가 fresh preview/selection에서 확인되면 read-only Drift Review를 자동 준비하고 우측 Context를 `동기화`로 안내한다. 사용자가 별도 `3-way 검토` 버튼을 먼저 찾아 누르는 단계를 제거한다.
- Sync 첫 화면은 기술 용어보다 목적 중심으로 `외부 변경 N건 발견`을 표시하고, 각 항목은 `원래 Authoring 기준값 → 현재 원본값`을 우선 보여준다.
- 기본 primary action은 `Authoring 값으로 복구…`로 두되, 이는 기존 `KeepAuthoring` decision prepare/review/commit을 그대로 사용한다. 자동 decision/자동 apply는 금지한다.
- `원본값을 레거시 고정값으로 보존…` / `원본값을 고급 덮어쓰기로 승격…`은 `다른 처리 방법`의 secondary/advanced 영역으로 내려 기본 화면의 선택 부담을 줄인다.
- Last Applied / Current Raw / Current Authoring exact 3-way와 override 허용 여부는 `상세 비교`에서 계속 확인 가능하게 유지한다. 내부 fingerprint/hash binding은 그대로 유지하되 기본 USER review dialog에서는 기술 식별값 노출을 최소화한다.
- Keep Authoring 승인 뒤에는 같은 Sync 화면에 `Authoring 값으로 복구 적용…` action을 노출하고 기존 shared `HandleApply` / R3 ApplyService lane을 사용한다. Bottom global Apply를 찾아 이동할 필요는 없게 하지만 source decision과 target Apply의 2단계 승인은 유지한다.
- final target Apply review는 `적용할 변경`, `외부 변경 처리 방향`, `자동 저장 안 함`을 사용자 언어로 표시한다. no-auto-save / no-auto-retry / stale·TOCTOU binding / fail-closed는 그대로 보존한다.
- Raw DA escape hatch는 유지하되 External Drift 복구의 기본 동선으로 승격하지 않는다.

Planned minimal Source scope
- `CFVehicleP11UI.cpp` v1.3.0: Sync macro card / advanced details / same-pane recovery apply presentation
- `CFVehicleAuthoringTab.cpp` v1.5.0: preview/selection drift auto-review+Sync focus, USER-facing Apply review wording 보강
- `CFVehicleAuthoringTab.h` v1.5.0: presentation helper declaration/state만 최소 추가
- VM/Core/Resolver/ApplyService/DataAsset schema mutation 0

Implementation + validation result
- `CFVehicleP11UI.cpp v1.5.0`: External Drift 목적 중심 macro-flow + edge hardening + 사용자-facing `Authoring/VehicleData/3-way`를 `제작 기준값/차량 데이터/상세 비교`로 현지화
- `CFVehicleAuthoringTab.cpp v1.7.0`: fresh preview/selection drift read-only auto-review + Sync 안내 + stale cached 결과 비노출 + Apply terminal failure modal + 사용자-facing `Authoring/VehicleData` 현지화 + 기본 Apply 대상 Asset 이름 표시
- `CFVehicleAuthoringTab.h v1.5.0`: `RefreshDriftGuidanceIfNeeded()` presentation helper
- `CFVehicleAuthoringVM.cpp/.h v1.5.0`: Workspace custom Undo를 current UE Undo stack의 exact TransactionId에 binding하고 intervening Editor transaction이 top이면 fail-closed
- `CFVehicleApplyService.cpp v1.2.0`: Definition Apply transaction에 `CarFight.VehicleAuthoring.DefinitionApply` context + Target PrimaryObject metadata 추가 / Writer·TOCTOU·no-auto-save 의미 변경 0
- `CFVehicleAuthoringVMTests.cpp v1.8.0`: unrelated Editor transaction 개입 시 custom Undo 차단 → intervening standard Undo → exact Workspace Apply Undo 복구 regression
- `CFVehicleUXOps.cpp v1.2.0`: Keep decision 성공 상태 문구를 `제작 기준값 유지 / 현재 외부 변경 / 레시피·프로필 변경 없음`으로 사용자-facing 현지화
- Runtime UCFVehicleData / Resolver / Inventory / Fitting / DataAsset schema mutation 0
- debugging 중 compile typo 1건과 Undo ownership guard 초기 판정 실패를 별도 RCA로 해결했으며 실패 실행을 PASS로 확대하지 않음
- readiness exact-source official build `84d2a7d60f7748aa8699c3c7ab4eb512`: Exit 0 PASS
- UA-04 localization exact-source official build `7364697cfcc542f090aa5a713fec83a1`: Exit 0 PASS / wording+display-only Source 변경이므로 기존 focused readiness 5건 replay 0
- UA-06 `CarFight.DataAuthoring.DAUTH_P0_09.Workspace.ApplyUndo` process `87355a4629b448a09967c28dc0bdab9b`: 1/1 PASS / 0 FAIL / Engine Exit 0
- UA-07 Remote Technical Readiness: `CFVehicleAuthoringP10.cpp v1.3.0`에서 Recipe→fresh Diff→explicit Target Apply 분리 설명과 Driving Feel save/preset failure feedback 보강 / Resolver·Preset semantic mutation 0
- UA-07 test-only fixture: `CFDAUA07FixtureCmdlet v1.0.0` + `RunDAUA07Fixture.ps1 v1.0.0`으로 `/Game/Test/CarFightDataAuthoring/DA_UA07_Handling`, `DA_UA07_Performance` 생성. Feel response는 기존 Resolver Automation monotonic fixture 재사용, direct baseline은 실행 시점 `DA_TestSedan` 복사, production balance truth로 승격 금지
- fixture commandlet process `30ef23a831184ccba508d45d83ac414d`: Exit 0 PASS / persisted fixture dataset `adset_v1_6bc502ac5c5d6be5d91889362f16e2ef.f0989769d1be4e211985389f` 2 assets exact readback PASS / Production Recipe isolation dataset `adset_v1_34f9089552941e0c7b6df12103a3a65a.58b95c12b8ee87124e4d5496`에서 UA07 reference 0 확인
- UA-07 baseline-safe cleanup: `UnbindVehicleProfile` explicit R1 Recipe-only semantic operation 추가. 기존 Bind non-empty path 계약 유지, Target/Profile Asset/Save mutation 0, UI review에 차량 데이터 직접 변경 없음·공유 프로필 값 변경 없음·자산 삭제 없음·자동 저장 안 함 명시
- final exact-source official build `627107d9de2d49be9be552c0baf2875a`: Exit 0 PASS / focused `CarFight.DataAuthoring.DAUTH_P0_12.Workspace.ProfileUnbind` process `e999340c380b4e18af218b57c81dcab3`: exact 1 test / Result={Success} / Save 0 / broad regression 0. 최초 runner invocation `9ac8e787f8e94098a5dfb69bc030eada`는 TestExit quoting 오류로 test result 생성 전 종료했으며 PASS로 집계하지 않음
- UA-05 `CarFight.DataAuthoring.DAUTH_P0_09.Workspace.ViewModelCoreParity` process `fbcfc455ac9442eea8ce7e5d3a1869cc`: 1/1 PASS / 0 FAIL / Engine Exit 0
- UA-04 `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.DriftRecovery` process `688a0ce125c5439a8fee214bf76c2cce`: 1/1 PASS / 0 FAIL / Engine Exit 0
- source-side Undo 보호 `LayoutDrivingUndo` process `6907ed3a18514afeb0d36efdc55411d7` 1/1 PASS + `AdoptionMeasurement` process `4b53750254f148e29f50e915acd2a428` 1/1 PASS
- UA-04 bounded USER recheck: External Drift 자동 Sync 안내 → primary recovery → Keep decision → same-pane final Apply 흐름을 USER가 `전보다 보기 편해졌어`로 평가 / 최종 925 복구 USER 확인
- 추가 USER feedback: `Authoring` 용어 번역 요청 → Source 반영·Build PASS / 실제 번역 화면 USER 확인 PASS
- UA-04 = USER PASS
- UA-05 = USER PASS / P0-12 USER PASS 5 / 문제 formatting + Source Trace search/effective-first/candidate-toggle UX USER 승인
- UA-06 = USER PASS / P0-12 USER PASS 6 — USER가 Source↔Target 구분과 Workspace Undo 흐름을 자연스럽다고 판정했고, remediation 후 Registry 기반 ID 선택을 `편해졌어`, final Apply 상세 review를 `알아보기 편하네 pass`로 직접 승인했다. final recheck에서 Apply는 `아니오`로 취소해 Target mutation0, Source current value 0 확인, Performance unbind 후 fresh binding none + diff0을 live screenshot으로 확인했다.
- UA-06 USER UX blocker #1 Remediated / USER PASS: Shared Profile 숫자 편집을 raw `ColumnId` 문자열 직접입력에서 selected Profile Domain의 `FCFBatchColumnRegistry::ProfileNumericEdit` authority 기반 field 검색/선택 UI로 전환했다. `CFVehicleP11UI.cpp v1.7.0`, `CFVehicleAuthoringTab.h v1.8.0`, `CFVehicleP11VM.cpp v1.3.0`, `CFVehicleAuthoringVM.h v1.7.0`에서 DisplayLabel + Unit + stable ColumnId secondary identity를 사용하고 current authored numeric value를 read-only Reflection으로 표시한다. USER가 실제 재검증에서 ID 선택이 편해졌다고 판정했다.
- UA-06 USER UX blocker #2 Remediated / USER PASS: `CFVehicleAuthoringTab.cpp v1.10.0` final `차량 데이터 적용 검토`가 fresh `FCFVehicleFieldDiff`에서 최대 8건의 field path / 현재 값 / 적용 후 값을 bounded 표시하고 초과분은 `외 N건`으로 요약한다. USER가 actual final review에서 알아보기 편하다고 PASS했다. External Drift recovery Apply에도 같은 상세 projection을 사용하며 ApplyService/approval/TOCTOU/no-auto-save 의미는 변경하지 않는다.
- UX remediation validation: `CFVehicleAuthoringVMTests.cpp v1.10.0`에서 selector current-value read helper를 SharedProfileImpact B2 commit 전 1500 / 후 2000으로 검증했다. final exact-source official build `685a6c6452db4d738e2b20b726964ce6` Exit 0 PASS / focused `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.SharedProfileImpact` process `9573f32f515b4858b1f89669205bb1a0` 1/1 PASS. USER PASS는 실제 selector 사용감 + final Apply detail 가독성 재확인 전까지 5 유지한다.
- UA-06 functional cycle은 이미 baseline 복구까지 완료했다. Source `Profile.Performance.RedlineStartRPM 0→5000` review USER correct → fresh pending diff1 / Target 0 / apply-after 5000 → Apply diff0 → Workspace Undo 후 Target0 + pending diff1 → USER Undo 자연스러움 PASS → Source 5000→0 → diff0 → Performance unbind → final binding none + diff0. 이 전체 functional cycle은 새 failure evidence 없이 replay하지 않는다.

UA-07 USER Acceptance result
- USER가 4축 slider의 local draft와 `4축 주행감 저장` commit 분리를 이해 가능하다고 판정했다. 가속 반응 0.50→0.75 draft 동안 pending diff0을 유지했고 commit 뒤 Recipe Intent만 변경됐다.
- Existing `DA_TestSedan` Legacy Pin precedence를 실제 USER flow에서 확인했다. Handling 관리 전환 review는 Legacy Pin 22 / prospective diff10, Performance 관리 전환 review는 Legacy Pin 9 / prospective diff3 / Auto Save0이었다. 각 Adoption은 VehicleData 적용 없이 Workspace Undo로 즉시 원복했다.
- Performance Adoption 상태에서 가속 반응 0.75가 `EngineMaxRPM 6500→6737.000977`, `EngineMaxTorque 925→947.400208`, `ThrottleInputScale 0.600000→1.098267`의 fresh pending diff3으로 Resolve되는 것을 live 확인했다. warning0 / block0 / external drift0 / Target Apply0.
- `Sports` preset 1회로 4축이 0.85 / 0.78 / 0.82 / 0.78로 즉시 Recipe commit되고 VehicleData auto-apply0인 것을 확인했다. USER는 slider commit 방식과 preset shortcut을 `PASS`로 판정했다.
- Cleanup: Performance Adoption Undo → 4축 0.50 / 0.50 / 0.50 / 0.50 복구 → temporary Profile cleanup → Handling `DA_Profile_UA_TestHandling` 복구. final live 화면 pending diff0 / warning0 / block0 / external drift0. Save All과 VehicleData Apply는 수행하지 않았다.
- UA-07 = USER PASS / P0-12 USER PASS 7.

UA-08 Runtime Comparison result
- baseline PIE: `TestMap` / `BP_CFVehiclePawn_C_2` / persistent `DA_TestSedan`. RuntimeRead baseline은 Engine MaxTorque=925, MaxRPM=6500, Steering AngleRatio=0.675.
- test-only `CFDAUA08Runtime.cpp v1.0.0` harness는 persistent Recipe/VehicleData를 수정하지 않고 transient Recipe + transient VehicleData에서 UA-07 Sports 4축 0.85/0.78/0.82/0.78을 shared Resolver로 계산했다.
- transient ownership에서 Handling Legacy Pin22 / Performance Legacy Pin9를 해제하고 Driving Feel 소유 Movement 13 field 전부를 PIE Pawn에 적용했다. RuntimeRead에서 transient VehicleData `DAUA08_SportsVehicleData_0`, Engine MaxTorque=1010, MaxRPM=7050, Steering AngleRatio=0.812를 확인했다.
- harness PASS log에서 persistent Recipe dirty=0 / persistent Target dirty=0, 적용 13 field exact coverage를 확인했다. PIE 종료로 transient state가 폐기됐고 Production Target Apply/Save/Adoption은 0이다.
- USER는 실제 주행 후 변화 방향을 확신하기 어렵다고 판정했다. 기능 failure로 보지 않으며, 0→100 km/h, Yaw Rate/횡가속, Slip Angle, Suspension stroke/settling 같은 정량 계측 비교를 후속 Deferred 개선으로 남긴다.
- UA-08 USER PASS는 부여하지 않는다. P0-12 USER PASS는 7 of 8 유지한다.
- USER가 이 제한을 인지한 상태에서 다음 단계 진행을 명시 승인했으므로 P0-12 USER Acceptance cycle은 `Closed with Deferred Feel Comparison`으로 종료한다.

Deprecated Gate result
- DG1 PASS: P0-09 Target/Preview/118-current Registry Diff/Validator/Apply/Undo/Raw Open replacement route와 기존 focused/full technical evidence 보존.
- DG2 PASS: P0-10 Reference Compare/Layout/4축 Driving Feel/preset/Undo parity 구현과 USER UA-07 authoring usability evidence 보존.
- DG3 PASS: `SCFVDAWizardTab v1.8.0` managed Target의 Layout/Quick Tune/Revert를 UI enable + handler 재검사 양쪽에서 차단. External Drift recovery는 P0-11/UA-04 evidence 보존.
- DG4 PASS: UA-01~07에서 새 Vehicle/Create/Existing Import/Recipe/Profile/Diff/Apply/Undo까지 새 Workspace만으로 실제 USER workflow 수행 가능 확인.
- DG5 PASS: migration 차이는 `Legacy direct VehicleData mutation` 대 `Recipe/Resolver/Apply`로 Design/Plan에 명시됐고 관련 Automation이 guard/parity를 보존한다.
- Source deprecation 적용: `CarFightReEditor.cpp/.h v1.3.0`에서 `창(Window)` 기본 메뉴의 `CarFight Vehicle DA Wizard` 항목만 제거하고 Current `CarFight 차량 데이터 제작` 메뉴는 유지. `CarFight.VehicleDAWizard` hidden tab identity/spawner와 `SCFVDAWizardTab` Source는 DEL Gate 전 compatibility용으로 유지.
- 최초 official build `fe6d5a1b41cc4bd1a1c49ff1336a939c` Exit6은 당시 CF-FQ-039 dirty UI compile blocker 때문이었다. 이후 Current UI 작업의 full Official Editor Build PASS로 shared blocker가 해소됐고, fresh Editor에서 updated `CarFightReEditor` binary가 정상 load됐다.
- focused `CarFight.DataAuthoring.DAUTH_P0_09.Workspace.TabRegistration`은 process `44a1a50b0f3241a681ca1c54e9a3adeb`가 띄운 UE Automation log에서 1 test / `Result={Success}`를 확인했다. runner wrapper는 `TestExit` 뒤 terminal 회수가 되지 않았고 USER가 해당 AI-owned background Automation Editor를 종료했으나 test result 자체는 PASS다.
- `CarFight.DataAuthoring.DAUTH_P0_10.Workspace.LegacyManagedGuard`는 이번 change set에서 `SCFVDAWizardTab`/managed-target guard logic mutation0이고 기존 PASS evidence가 있으므로 affected-regression 원칙에 따라 replay하지 않았다.
- fresh Ready Editor에서 USER가 `창(Window)` menu를 펼친 뒤 Slate read-only Snapshot으로 `CarFight 차량 데이터 제작` 존재와 `CarFight Vehicle DA Wizard` 부재를 직접 확인했다. hidden `CarFight.VehicleDAWizard` spawner는 Source에 유지돼 rollback/compatibility 경계를 보존한다.
- Deprecated transition = Technical Complete.
- DEL audit: DEL1~5와 DEL7은 현재 evidence상 충족 가능하나 DEL6 required-caller0는 아직 PASS가 아니다. hidden compatibility spawner, `OpenVDAWizardTab`, `FCFVDAWizardTestAccess`/LegacyManagedGuard가 의도적으로 직접 참조하므로 physical deletion은 보류한다.

Next exact gate
1. Data Authoring의 현재 deprecation 작업은 종료한다. UA-01~08, P0-08~11, DG1~DG5와 TabRegistration/menu validation은 새 failure evidence 없이 replay하지 않는다.
2. `SCFVDAWizardTab` physical deletion은 별도 compatibility retirement에서 DEL6 required-caller0를 실제로 충족시킨 뒤 DEL1~DEL7을 다시 최종 판정할 때만 재개한다.
3. UA-08 quantitative driving comparison은 비차단 Deferred로 유지하며 Data Authoring deprecation closure를 다시 열지 않는다.
4. CF-FQ-038은 Paused 상태를 유지하고 현재 단일 Active CF-FQ-039를 변경하지 않는다.
```

### Changelog

- v0.2.53 (2026-09-11): CF-FQ-038을 Done / Historical + Retained Path로 전환했다. DEL6에서 Legacy Wizard hidden spawner/open entry/test access를 제거하고 `UE/Source` direct reference exact0을 확인한 뒤 Legacy Wizard 소스 3개를 물리 폐기해 DEL1~DEL7 PASS를 확정했다. final Official UE 5.8 Build `1fd947042024420aaa7561d380e4548f` PASS와 전체 `CarFight.DataAuthoring` 111/111 PASS를 확보했다. UA-08은 Runtime Technical PASS / USER Inconclusive / quantitative comparison Deferred로 보존하고 P0-12 USER PASS 7/8을 확대하지 않는다. Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.5.0`; Product Asset migration/Apply/Save는 0이다.
- v0.2.52 (2026-08-26): USER가 신규 차량 제작의 목표를 실존 차량 Reference 기반 AI Authoring + 단계형 Guided Vehicle Builder로 확정했다. CF-FQ-038은 폐기하지 않고 Recipe/Resolver/Diff/Validation/Apply/Undo Backend와 Advanced 유지보수 Workspace로 역할을 고정했다. 4축 Driving Feel은 Reference 원천값이 아니라 optional bias/Quick Tune으로 재분류한다. Wheel 자동검출/Hardpoint 자동배치는 CF-FQ-040 Scope Out이며 기존 CF-FQ-038 evidence와 Deprecated transition은 그대로 보존한다.
- v0.2.51 (2026-08-24): Legacy Wizard Deprecated transition을 Technical Complete로 닫았다. 동시 CF-FQ-039의 full Official Editor Build PASS로 이전 shared compile blocker가 해소된 상태를 보존하고, focused `Workspace.TabRegistration` UE Automation log 1/1 Success와 fresh Editor `창(Window)` Slate Snapshot에서 Current `CarFight 차량 데이터 제작` 존재 / Legacy `CarFight Vehicle DA Wizard` 부재를 확인했다. `LegacyManagedGuard`는 이번 change set에서 guard Source mutation0이라 기존 PASS를 replay하지 않고 보존했다. hidden `CarFight.VehicleDAWizard` spawner/Source는 compatibility로 유지한다. DEL6 required-caller0는 아직 미충족이므로 physical `SCFVDAWizardTab` deletion은 계속 금지하며 UA-08 quantitative comparison도 비차단 Deferred로 유지한다.
- v0.2.50 (2026-08-24): Deprecated Gate DG1~DG5를 current Source/Automation/USER evidence로 전부 PASS 판정했다. `CarFightReEditor.cpp/.h v1.3.0`에서 Current `CarFight 차량 데이터 제작` 메뉴만 남기고 Legacy `CarFight Vehicle DA Wizard` 기본 Window 메뉴 항목을 제거했으며 `CarFight.VehicleDAWizard` hidden spawner/Source는 DEL Gate 전 compatibility로 보존했다. DEL1~5·DEL7은 충족 가능하지만 DEL6 required-caller0는 hidden spawner·OpenVDAWizardTab·Legacy guard test direct ref 때문에 Pending이라 physical deletion은 금지한다. official build `fe6d5a1b41cc4bd1a1c49ff1336a939c`는 현재 CF-FQ-039 dirty `CFHUDPresenter.cpp`/`CFHUDDataTests.cpp`의 unrelated compile error로 Exit6이므로 Deprecated exact-source Build/focused validation은 미완료다. 다음은 UI compile blocker 해소 후 build 1회 + TabRegistration/LegacyManagedGuard focused regression만 수행한다.
- v0.2.49 (2026-08-24): UA-08 Driving Feel Runtime Comparison에서 transient Sports 4축 0.85/0.78/0.82/0.78을 shared Resolver로 계산해 PIE `BP_CFVehiclePawn`에 Driving Feel Movement 13 field를 적용했다. RuntimeRead로 baseline MaxTorque925/MaxRPM6500/SteeringRatio0.675에서 Sports 1010/7050/0.812 전환과 transient VehicleData 사용을 확인했고 persistent Recipe/Target dirty0, Target Apply/Save0을 보존했다. USER는 주행 체감 차이를 확신하기 어렵다고 판정해 USER PASS는 부여하지 않고 정량 계측 비교를 Deferred했다. 사용자 승인에 따라 P0-12를 `Closed with Deferred Feel Comparison`으로 종료하며 USER PASS는 7/8 유지하고 DG/DEL Gate를 개방한다. 다음은 Deprecated Gate DG1~DG5 감사이며 SCFVDAWizardTab 물리 삭제는 DEL1~DEL7 전까지 금지다.
- v0.2.48 (2026-08-24): UA-07 Driving Feel Authoring을 실제 USER Acceptance로 완료하고 P0-12 USER PASS를 7로 승격했다. slider local draft→explicit Recipe commit 분리, Legacy Pin ownership 전환 후 Recipe→Resolver→Diff 인과, Sports preset shortcut을 USER가 PASS했다. Performance Adoption에서 가속 0.75가 EngineMaxRPM/EngineMaxTorque/ThrottleInputScale 3개 pending diff로 변환되는 것을 live 확인했으며 VehicleData Apply·Auto Save는 0이다. Handling/Performance Adoption은 각각 검토 후 Workspace Undo로 원복했고 4축을 0.50 전축으로 복구한 뒤 Handling baseline `DA_Profile_UA_TestHandling`을 복원했다. final pending diff0 / warning0 / block0 / external drift0. 다음 USER Gate는 UA-08 Driving Feel Runtime Comparison이다.
- v0.2.47 (2026-08-22): 문서 정상화 Health Check로 `Current additive schema compatibility` 아래 남아 있던 stale USER PASS 5 / 다음 UA-06 projection을 최신 P0-12 USER PASS 6 / 다음 UA-07 Driving Feel Authoring으로 동기화했다. 기존 UA-01~06 상세 evidence와 Changelog의 당시 상태는 삭제하거나 재작성하지 않는다.
- v0.2.46 (2026-08-21): UA-06 Explicit Apply / Undo를 USER PASS로 닫고 P0-12 USER PASS를 6으로 승격했다. 기존 functional cycle에서 Source↔Target 구분과 Workspace Undo를 USER가 PASS했고, blocker #1 Registry 기반 field selector는 `Id선택 편해졌어`, blocker #2 final Apply field/current/apply-after review는 `알아보기 편하네 pass`로 실제 USER 재검증 PASS했다. final Apply는 review에서 취소해 Target mutation0을 유지했고, Source `RedlineStartRPM` current value 0을 확인한 뒤 Performance Profile unbind 후 fresh live 화면에서 binding none / pending diff0 / warning0 / block0 / external drift 없음을 확인했다. UA-06 Build `685a6c6452db4d738e2b20b726964ce6`과 focused SharedProfileImpact `9573f32f515b4858b1f89669205bb1a0`는 보존하고 replay하지 않는다. 다음 USER Gate는 UA-07 Driving Feel Authoring이다.
- v0.2.45 (2026-08-21): UA-06 USER가 발견한 UX blocker 2건을 UI 중심으로 Technical Remediation했다. Shared Profile numeric editor는 raw ColumnId 직접입력을 제거하고 selected Profile Domain의 existing `FCFBatchColumnRegistry` descriptor를 검색/선택하는 UI로 전환했으며 표시명·현재 authored 값·단위·secondary 기술 ID를 제공한다. current value는 `FCFVehicleAuthoringVM::ReadBoundProfileNumericValue` read-only Reflection helper로만 읽고 기존 B2 stable ColumnId contract는 유지한다. final `차량 데이터 적용 검토`는 fresh FieldDiff에서 최대 8건의 field/current/apply-after 상세를 표시한다. `CFVehicleAuthoringTab.cpp v1.10.0`, `.h v1.8.0`, `CFVehicleP11UI.cpp v1.7.0`, `CFVehicleP11VM.cpp v1.3.0`, `CFVehicleAuthoringVM.h v1.7.0`, `CFVehicleAuthoringVMTests.cpp v1.10.0`; official build `685a6c6452db4d738e2b20b726964ce6` Exit0 PASS, focused SharedProfileImpact `9573f32f515b4858b1f89669205bb1a0` 1/1 PASS. USER PASS는 5 유지하며 다음은 selector 사용감 + final Apply detail 가독성의 짧은 recheck만 수행한다.
- v0.2.44 (2026-08-21): UA-06 functional Apply/Undo cycle을 baseline 복구까지 완료했다. `DA_UA06_Performance` binding diff0 → `Profile.Performance.RedlineStartRPM 0→5000` Source edit review USER correct → fresh pending diff1 / Target0 / apply-after5000 → explicit Apply 후 diff0 → `마지막 작업 되돌리기` 1회 후 Target0 복구 + pending diff1 재등장 → USER가 Undo 흐름을 자연스럽다고 판정 → Source 5000→0 원복 후 diff0 → Performance Profile unbind 후 final binding none + diff0을 fresh live screenshot으로 확인했다. Source↔Target 구분도 USER PASS다. 기능 흐름은 정상이나 blocker #1 raw ColumnId 직접입력, blocker #2 final Apply 변경 상세 미표시가 남아 UA-06 USER PASS는 5에서 확대하지 않는다. 다음은 이 두 UI-only remediation 후 짧은 USER recheck다.
- v0.2.43 (2026-08-21): UA-06 final Apply review에서 두 번째 USER UX 결함을 확인했다. `CFVehicleAuthoringTab.cpp`의 `차량 데이터 적용 검토`는 변경 개수·경고·외부 변경·대상·자동 저장 여부만 보여주고, `GetDiffText()`에 이미 존재하는 field/current/apply-after 상세를 final review에 표시하지 않는다. USER가 실제 검토 중 `RedlineStartRPM 0→5000` 같은 내용이 메시지에 뜨지 않는다고 지적했다. UA-06 functional cycle은 Undo까지 계속 관찰하되, USER PASS 전에 final Apply review에 변경 상세를 검토 가능한 형태로 추가한다. USER PASS는 5 유지한다.
- v0.2.42 (2026-08-21): UA-06 USER Acceptance에서 `DA_UA06_Performance` Profile 연결 review 문구를 USER가 명확하다고 판정하고 승인했으며, fresh Workspace에서 연결 자체 diff0을 확인했다. 동시에 Shared Profile 숫자 편집이 raw `ColumnId` 직접입력 방식뿐이라 사용자가 기술 ID를 모르면 값을 편집할 수 없다는 실제 USER UX 결함을 발견했다. UA-06 functional Apply/Undo cycle은 추가 UX 문제 수집을 위해 계속 진행하되 USER PASS는 금지한다. cycle 종료 직후 Profile Domain 기반 field selector/search + 사용자 표시명 + 현재 값 + 단위 + 새 값 입력 UI로 remediation하고, stable ColumnId/Registry/Apply 내부 계약은 유지한다.
- v0.2.41 (2026-08-21): UA-06 USER session에서 기존 `FrontWheelMaxBrakeTorque` 시험안이 유효하지 않음을 실제 Source Trace로 확인했다. Existing Definition Import의 Legacy Pin이 effective source여서 Handling Profile 2000→2100 review가 `예상 차량 데이터 변경 0`을 표시했고 USER는 승인 전 `아니오`로 취소해 mutation 0이다. 대체로 `/Game/Test/CarFightDataAuthoring/DA_UA06_Performance`를 추가해 현재 `DA_TestSedan` Performance baseline을 상수 Feel response와 direct 값으로 복사했다. `RedlineStartRPM`은 Legacy Pin이 없고 PerformanceProfileDirect owner이므로 Profile binding 뒤 single-field `0→5000` Source edit로 UA-06을 검증한다. official build `4a07fa4dc9674746ba56b3a2b14457a1` Exit 0, headless fixture process `9a5f6b110d534fbcb23167576157a1c8` Exit 0, persisted exact values와 Production Recipe Performance binding null isolation을 AssetDump로 확인했다. 사용자 요청에 따라 GUI Editor는 재기동하지 않고 Editor0로 종료했다. USER PASS는 5 유지한다.
- v0.2.40 (2026-08-20): UA-06 USER Pending / P0-12 USER PASS 5를 유지한 채 다음 단계 UA-07 Driving Feel을 remote Technical Readiness까지 선행 감사했다. Driving Feel Recipe/Target 분리 설명과 failure feedback을 `CFVehicleAuthoringP10.cpp v1.3.0`에 보강하고, 기존 Resolver Automation 값을 재사용하는 `/Game/Test/CarFightDataAuthoring` test-only Handling/Performance fixture를 deterministic commandlet으로 생성·persisted 검증했다. Production Recipe의 UA07 reference는 0이다. USER 검사 후 Performance baseline `없음`으로 안전 복구하기 위해 기존 Bind non-empty 계약을 약화하지 않고 explicit `UnbindVehicleProfile` R1 Recipe-only operation/UI review를 additive 추가했다. final official build `627107d9de2d49be9be552c0baf2875a` Exit 0, focused `ProfileUnbind` process `e999340c380b4e18af218b57c81dcab3` exact 1 test PASS를 확보했다. USER PASS는 확대하지 않으며 다음 실제 Gate는 계속 UA-06이다.
- v0.2.39 (2026-08-20): UA-05를 USER PASS로 닫고 P0-12 USER PASS를 5로 승격했다. `문제` 탭 v1.8.0 formatting은 USER 재확인 PASS했고, 최초 `값 출처` 123개 full dump는 과밀로 판정해 `CFVehicleAuthoringTab.cpp v1.9.0 / .h v1.6.0`에서 `필드명/출처 검색`, 실제 적용 Source 우선, `비적용 후보까지 보기` secondary 표시를 추가했다. Resolver/SourceTrace authority mutation은 0이며 official build `e036110014c84f93b1a046f9eb065c6b` Exit 0 PASS다. USER가 `torque` 검색 123→4와 후보 toggle을 실제 화면으로 확인한 뒤 충분히 쓸 만하다고 승인했다. 다음 USER Gate는 UA-06 Explicit Apply / Undo이며 기존 `ApplyUndo` readiness는 replay하지 않는다.
- v0.2.38 (2026-08-20): UA-05 USER readability에서 `문제` 탭 검증 문장이 과밀해 읽기 어렵다는 실제 피드백을 받았다. `CFVehicleAuthoringTab.cpp v1.8.0`에서 Validation issue 표시만 의미 단위로 분리하고 문장 사이 줄바꿈을 추가했으며 official build `609d066fe99a455ab5cbb36871bd8e3a`가 Exit 0 PASS했다. 확인용 fresh Editor는 사용자 조작 전 AI-owned save0 stop해 Editor0로 정리했다. UA-05 USER PASS는 보류하며 다음은 `문제` 탭 formatting 재확인 후 `값 출처` 123개 나열 UX 판정이다.
- v0.2.37 (2026-08-20): UA-04 번역 화면에서 `제작 기준값/차량 데이터/상세 비교` 현지화를 USER가 직접 확인해 UA-04를 USER PASS로 닫고 P0-12 USER PASS를 4로 승격했다. 다음 USER Gate는 UA-05 Resolve / Diff / Source Trace / Validation readability이며 기존 Technical Readiness는 replay하지 않는다.
- v0.2.36 (2026-08-20): UA-04 macro-flow USER recheck에서 사용자가 이전보다 보기 편해졌다고 판정했고 Keep→same-pane Apply→925 복구까지 완료했다. 후속 USER feedback인 `Authoring` 용어 번역을 `제작 기준값/차량 데이터/상세 비교` 정책으로 `CFVehicleAuthoringTab v1.7.0`, `CFVehicleP11UI v1.5.0`, `CFVehicleUXOps v1.2.0`에 반영했으며 localization exact-source build `7364697cfcc542f090aa5a713fec83a1`이 PASS했다. P0-12 USER PASS는 번역된 실제 화면 최소 확인 전까지 3을 유지한다.
- v0.2.35 (2026-08-19): 사용자 부재 중 UA-04 edge / UA-05 Resolve·Diff·Trace·Validation / UA-06 Apply·Undo Technical Readiness를 bounded 감사했다. stale cached validation/diff surface, drift recovery edge/dead-end, custom Undo의 unrelated Editor transaction 오동작 위험을 교정했다. final exact-source build `84d2a7d60f7748aa8699c3c7ab4eb512` PASS, ApplyUndo·ViewModelCoreParity·DriftRecovery·LayoutDrivingUndo·AdoptionMeasurement focused 5건 모두 1/1 PASS다. USER PASS는 3 유지하며 다음 실제 Gate는 saved 925 baseline의 UA-04 one-shot UX recheck다.
- v0.2.34 (2026-08-19): UA-04 macro-flow UI-only remediation을 `CFVehicleP11UI v1.3.0 / CFVehicleAuthoringTab v1.5.0`으로 구현했다. Editor DLL lock으로 첫 build가 Exit 6이었으나 사용자 종료 후 official build `3b0e8518d66440d1a4f06f59f6c75df4` PASS, focused DriftRecovery `e651fc7af4df4cfb8f4e964c82074bc9` 1/1 PASS를 확보했다. fresh Editor는 USER 퇴근으로 재검사 전에 save0 AI-owned 종료해 Editor0로 보존했다. 다음은 saved 925 baseline의 Raw 950 one-shot USER UX recheck이며 P0-12 USER PASS는 3 유지다.
- v0.2.33 (2026-08-19): UA-04 USER 피드백의 핵심을 기능 이해 부족이 아니라 macro-flow 복잡성으로 확정하고 UI-only remediation 계약을 Frozen했다. External Drift auto-review+Sync 안내, 목적 중심 summary, Keep Authoring primary action, secondary recovery collapse, 상세 3-way 보존, same-pane final Apply를 도입하되 기존 fail-closed/2단계 승인/no-auto-save/TOCTOU 계약은 유지한다.
- v0.2.32 (2026-08-19): UA-04 External Drift 3-way의 기술 흐름은 정상 동작했지만 USER는 세 값/선택지의 미시적 의미는 이해하면서도 전체 Authoring 복구 흐름을 거시적으로 복잡하다고 판정했다. UA-04를 USER PASS로 닫지 않고 `Technical flow PASS / USER UX Remediation Required`로 보존하며 P0-12 USER PASS는 3을 유지한다. 다음 Gate는 Frozen 안전 계약을 유지한 macro-flow 단순화다.
- v0.2.31 (2026-08-19): UA-03 B2 결과인 `DA_Profile_UA_TestHandling` exact save를 live dirty=false + fresh AssetDump persisted `FrontWheelMaxBrakeTorque=2000`, `AuthoringRevision=1`로 확인했다. UA-04는 current Recipe AppliedState가 아직 empty이므로 single-field baseline Apply 후 External Drift 3-way를 검사한다.
- v0.2.30 (2026-08-19): UA-03 transient inventory correction의 final official build와 focused SharedProfileImpact PASS, persisted Handling+DriveState binding, FrontWheelMaxBrakeTorque=2000 B2 USER one-shot preview/commit, Shared Profile-only dirty boundary를 확인해 UA-03을 USER PASS로 닫고 P0-12 USER PASS를 3으로 전진했다. 다음 Gate는 exact Handling Profile 저장 후 UA-04 External Drift 3-way Recovery다.
- v0.2.29 (2026-08-19): UA-03 B2 재검사에서 발견된 transient rollback/prospective Recipe 오수집 RCA, source+regression patch, compile PASS·DLL lock build failure, exact 2-asset save와 Editor0 handoff 상태를 기록했다.

### Current additive schema compatibility — UI-P0-06 RPM Redline

`CF-FQ-032 UI-P0-06`의 explicit RPM Redline upstream contract에서 `FCFVehicleMovementConfig.RedlineStartRPM`이 additive VehicleData leaf로 추가됐다. 이는 DAUTH 기능 재개가 아니라 Runtime Definition schema 변경에 대한 bounded compatibility maintenance다.

```text
Original P0-08 frozen implementation evidence
- VehicleData Registry: 117 leaf
- Performance Profile numeric: 17
- five Profile numeric total: 78

Current Source compatibility surface
- VehicleData Registry: 118 leaf
- 신규: VehicleMovementConfig.RedlineStartRPM
- Owner: Performance Profile
- ResolveRule: PerformanceProfileDirect
- Performance Profile numeric: 18
- five Profile numeric total: 79
- RedlineStartRPM=0: 명시적 미설정 / 기존 Asset 호환
- 자동 EngineMaxRPM/변속값 fallback: 금지
```

P0-08 당시 117/78 결과는 그 시점 schema에 대한 Historical evidence로 보존한다. Current Source 판단과 향후 신규 export/import는 118/79 compatibility surface를 사용한다. 현재 P0-12 USER PASS는 7이며 UA-01~07은 USER PASS다. UA-08 USER Feel은 Inconclusive / quantitative comparison Deferred로 P0-12 cycle을 종료했다. Legacy Wizard Deprecated transition은 Technical Complete이며 현재 남은 non-blocking gate는 DEL6 compatibility retirement와 UA-08 quantitative comparison Deferred뿐이다. UA-01~08 flow와 completed deprecation validation은 새 failure evidence 없이 replay하지 않는다.

## 1. 문서 목적


CarFight는 Vehicle, Weapon, Equipment, Scanner, Armor 등 정적 Definition 데이터가 점점 늘어나고 있고, VehicleData 하나만 보더라도 사람이 직접 이해하고 입력해야 하는 필드 수가 많다.

현재 문제는 단순히 DataAsset 파일 수가 많다는 데 있지 않다.

```text
1. 하나의 DataAsset 내부 필드 수가 많다.
2. 어떤 필드를 사람이 직접 결정해야 하는지 구분하기 어렵다.
3. 서로 영향을 주는 수치가 많아 입력 순서를 판단하기 어렵다.
4. 기준 차량과 여러 차량을 동시에 비교하기 불편하다.
5. 반복되는 기본 설정을 사람이 매번 직접 입력한다.
6. AI가 기본 설정을 도와주더라도 현재는 개별 Raw Field에 직접 값을 쓰는 방식이 되기 쉽다.
7. Inventory / Fitting / Runtime이 사용하는 데이터와 Editor Authoring 편의 데이터의 책임 경계가 아직 명시적으로 분리되어 있지 않다.
```

따라서 이번 작업은 DataAsset 자체를 없애거나 Excel로 대체하는 것이 아니라, **DataAsset 위에 CarFight 전용 Authoring Layer를 추가하여 생성·입력·비교·튜닝·검증을 일관된 방식으로 수행할 수 있게 만드는 것**을 목표로 한다.

---

## 2. 핵심 목표

이번 작업의 최종 목표는 다음과 같다.

```text
사용자는 차량의 의도와 성격을 결정한다.
AI와 Authoring Tool은 기존 Profile / Rule / Asset / Definition을 사용해 기본 설정을 구성한다.
Resolver는 실제 Definition에 적용할 값을 계산한다.
Preview / Diff / Validator가 결과를 검증한다.
사용자가 승인한 뒤 Definition에 반영한다.
Inventory / Fitting / Runtime은 Authoring 과정이 아니라 확정된 Definition만 소비한다.
```

이 목표를 위해 다음 기능을 단계적으로 준비한다.

- Data Browser / Catalog
- Recipe / Intent 입력
- Profile / Rule 기반 기본값
- Derived Value 계산
- Asset-derived Value 추출
- Override 관리
- Source Tracking
- Compare / Diff
- Validation
- AI Authoring Workflow
- Excel / CSV 대량 편집 연계
- 기존 Vehicle DA Wizard 기능 흡수
- Driving Feel Authoring

---

## 3. 최우선 설계 원칙

### 3.1 Authoring과 Runtime을 분리한다

Authoring은 Editor 작업이다.
Runtime은 확정된 Definition을 사용한다.

```text
Authoring Intent / Recipe
        ↓
Profile / Rule
        ↓
Resolver
        ↓
Resolved Definition
        ↓
Inventory / Fitting / Runtime
```

Runtime은 Recipe, Profile 조합 과정을 알 필요가 없어야 한다.

### 3.2 Inventory와 Fitting은 Authoring 시스템에 종속되지 않는다

Inventory는 "무엇을 소유하고 있는가"와 Instance State를 소유한다.
Fitting은 "어떤 장비가 어느 슬롯에 장착되어 있는가"와 장착 결과를 소유한다.

Authoring Layer는 이 두 시스템을 대체하지 않는다.

```text
Authoring
= Definition을 제작하고 검증

Inventory
= Definition을 참조하는 Instance 보관

Fitting
= Instance / Definition의 장착 가능성과 장착 결과 관리

Runtime
= 최종 적용 상태 사용
```

### 3.3 사람에게 Raw Field 전체 입력을 요구하지 않는다

사용자가 반드시 결정해야 하는 값과 시스템이 만들 수 있는 값을 분리한다.

초기 분류 체계:

```text
Human Intent
Profile
Derived
Asset Derived
Advanced Override
Definition Stored
Fitting Derived
Runtime State
USER Acceptance
```

VehicleData의 모든 실제 필드는 DAUTH-P0-01에서 이 분류 기준으로 전수 감사한다.

### 3.4 AI는 임의 수치 생성기가 아니라 Authoring Client다

AI가 차량 기본 설정을 입력하는 것은 적극적으로 허용한다.
다만 AI가 임의 숫자를 만들어 Raw Field에 직접 기록하는 구조는 금지한다.

AI Authoring은 다음 순서를 따른다.

```text
사용자 요구
→ 기존 Definition / Profile / Rule 조회
→ Authoring Intent / Recipe 구성
→ Resolver 실행
→ Preview / Diff
→ Validator
→ 적용
```

사용자 확인이 필요한 주행감, 시각 결과, 게임플레이 체감은 AI가 PASS로 추정하지 않는다.

### 3.5 값의 출처를 추적 가능하게 한다

가능한 최종 Authoring UI는 값 자체뿐 아니라 Source를 보여줘야 한다.

예:

```text
Property                  Value       Source
BaseVehicleMassKg         1570        Vehicle Intent
EngineMaxTorque           820         MidSedan + Sport Profile
FrontWheelMaxSteerAngle   38          Responsive Steering Profile
WheelRadius               35          Asset / Explicit Definition
SteeringAngleRatio        0.74        Manual Override
```

### 3.6 Profile 변경은 자동 Runtime Live Inheritance로 전파하지 않는다

Profile 수정으로 기존 모든 Vehicle Definition이 암묵적으로 바뀌는 구조를 기본안으로 사용하지 않는다.

권장 흐름:

```text
Profile 변경
→ 영향받는 Definition = Stale
→ Diff 확인
→ 선택 Regenerate
→ Validate
→ Apply
```

이 방식으로 Definition의 변경 시점과 변경 이유를 추적한다.

### 3.7 Excel / CSV를 SSOT로 만들지 않는다

초기 기준은 Unreal Definition이 Canonical Authoring Result다.

```text
Unreal Authoring Tool
= 구조 / 자산 참조 / 개별 작성 / 검증

Excel / CSV
= 반복 숫자의 대량 비교 / Batch Edit / Balance 작업
```

외부 표와 Unreal Asset이 동시에 Authoritative한 구조는 피한다.

---

## 4. 데이터 책임 계층

### 4.1 Authoring Intent / Recipe

사람 또는 AI가 지정하는 고수준 의도다.

Vehicle 예:

```text
Vehicle Archetype = Mid Sedan
Drive Layout = RWD
Handling Character = Sport Balanced
Performance Bias = Acceleration
Requested Base Mass = 1550 kg
Armor Class = Light
Weapon Layout = Front Dual
```

Recipe는 Runtime 데이터가 아니라 Definition을 만들기 위한 제작 입력이다.

### 4.2 Profile / Rule

여러 Definition이 공유할 수 있는 Authoring 기준이다.

예:

```text
MidSedan Base Profile
Sport Handling Profile
Acceleration Performance Profile
RWD Drivetrain Rule
```

초기 P0에서는 실제 Profile UObject/DataAsset 형태를 미리 확정하지 않는다.
P0-00~02 감사 결과를 기준으로 저장 형태를 결정한다.

### 4.3 Derived

다른 Authoring 입력에서 계산할 수 있는 값이다.

예:

- 차급과 질량 범위를 이용한 추천 기반값
- Vehicle Context를 반영한 Driving Feel 실제 물리값
- 규칙으로 결정되는 기본 태그

### 4.4 Asset Derived

Mesh, Socket, Bounds 등 Unreal Asset 자체에서 얻을 수 있는 값이다.

현재 Vehicle DA Wizard의 Layout Capture는 이 계층의 선행 구현으로 본다.

### 4.5 Advanced Override

Profile / Derived 결과에서 벗어나야 하는 특수 차량을 위한 명시적 예외다.

Override는 최소화하고 UI에서 Source와 함께 표시해야 한다.

### 4.6 Definition

게임에서 "이 물건은 무엇인가"를 설명하는 확정 정적 데이터다.

Vehicle의 현재 Definition 역할은 `UCFVehicleData`가 수행한다.

### 4.7 Inventory Instance

플레이어가 실제로 소유하는 개체 상태다.
Definition을 참조하되 Authoring Recipe를 소유하지 않는다.

### 4.8 Fitting State

어떤 Instance / Definition이 어느 슬롯에 장착되었는지와 장착 결과를 소유한다.

### 4.9 Runtime State

Damage, Ammo, Heat, 현재 Velocity 등 실행 중 변하는 상태다.

---

## 5. Vehicle 우선 원칙

P0 구현은 Vehicle 하나에 집중한다.

Vehicle에서 다음을 검증한다.

- 복잡한 DataAsset 필드 분류가 실제로 가능한가
- Recipe / Profile / Derived / Override 구분이 유용한가
- AI가 Raw Field 직접 작성 없이 기본 설정을 만들 수 있는가
- 기존 VDA Validator를 재사용할 수 있는가
- 기존 Fitting / Inventory 계약을 깨지 않고 Authoring Layer를 추가할 수 있는가
- 기존 Vehicle DA Wizard를 통합할 수 있는가
- Driving Feel을 고수준 입력으로 표현할 수 있는가
- 대량 비교 / Excel 연계가 실제로 필요한 범위를 찾을 수 있는가

Weapon / Equipment / Scanner / Armor는 Vehicle P0에서 공통 패턴이 검증되기 전까지 실제 범용 구현을 시작하지 않는다.

---

## 6. 기존 Vehicle DA Wizard 처리 방침

현재 `CarFight_ReEditor` 모듈에는 `Vehicle DA Wizard`가 존재한다.

현재 기능:

```text
Target / Source VehicleData 선택
Validator 실행
검사 리포트
Layout Capture
Driving Feel Quick Tune
Preview
Undo / Revert
DA Editor 열기
```

현재 판정:

```text
삭제 금지
신규 기능 확장 보류
새 Authoring 설계의 Predecessor / Migration Source로 사용
```

새 Authoring Tool이 기존 기능을 모두 대체하고 회귀 검증을 통과하기 전까지 기존 Wizard는 제거하지 않는다.

### 승계 후보

- Target / Source 개념
- Compare
- Validator 재사용
- Layout Capture
- Preview before Apply
- FScopedTransaction 기반 Undo
- Revert
- 적용 후 Validation

### 재설계 후보

- Sedan / SUV / Sports / Heavy 하드코딩 프리셋
- 고정 Min/Max 기반 선형 보간
- Slate UI 내부 물리 계산
- Driving Feel Slider → Raw VehicleMovementConfig 직접 기록
- 여러 Raw 값을 평균내어 Feel Slider를 역산하는 방식

---

## 7. Driving Feel 방향

현재 Quick Tune의 고수준 UX 개념은 유지 가치가 있다.

현재 4축:

```text
Acceleration Feel
Steering Agility
Grip Feel
Suspension Firmness
```

단, 새 구조에서는 다음과 같은 Context-aware Resolver를 검토한다.

```text
Driving Feel Intent
+
Vehicle Context
+
Vehicle Profile
+
Explicit Overrides
↓
Resolved Vehicle Movement Values
```

즉 같은 `Suspension Firmness = 0.7`이라도 1500kg Sedan과 2500kg Pickup이 반드시 동일 SpringRate를 가지도록 만들지 않는다.

Driving Feel의 최종 실제 수치는 USER 주행감 검증 없이 자동 PASS 처리하지 않는다.

---

## 8. Inventory / Fitting 통합 원칙

### Inventory

Inventory는 Authoring Recipe를 저장하지 않는다.
Inventory Instance는 안정적인 Definition 참조와 Instance State를 가진다.

```text
Item Instance
- InstanceId
- DefinitionId / Definition Reference
- Durability
- Quantity / Ammo State 등
```

### Fitting

Fitting은 Definition의 Compatibility와 장착 결과를 사용한다.

VehicleData의 현재 HardpointSlots / MountProfiles / BaseVehicleMassKg / MaximumGrossMassKg는 Authoring 감사 대상이지만, 최종 Fitting 책임을 Authoring으로 이동시키지 않는다.

### Fitting Derived와 Authoring Derived를 분리한다

```text
Authoring Derived
= Definition 자체를 만들 때 결정되는 값

Fitting Derived
= 실제 장비를 조립한 결과로 결정되는 값
```

예:

```text
BaseVehicleMassKg
= Definition / Authoring

Fitted Vehicle Mass
= Fitting
```

Authoring Tool은 Fitting 결과를 Preview / Test Context로 읽을 수 있지만 소유하지 않는다.

---

## 9. AI Authoring 계약 초안

AI는 다음 작업에 적합하다.

- 기존 Definition 검색
- Profile 후보 선택
- 반복 기본값 구성
- Asset에서 추출 가능한 값 탐지
- Naming / ID 제안
- 누락 필드 탐지
- Compare / Diff 작성
- Validation 실행
- Bulk Edit 후보 생성

사용자 판단을 우선하는 영역:

- 차량 역할과 개성
- 주행 성향
- 전투 성향
- 예외 Override 승인
- 실제 주행감
- 실제 시각 결과

AI가 새 차량을 작성하는 표준 흐름 후보:

```text
1. 요구 해석
2. 기존 Definition / Profile 조회
3. Intent / Recipe 구성
4. Resolver Preview
5. Source 추적
6. Diff 출력
7. Validator
8. 적용
9. 재검사
10. USER Tuning Pending 유지
```

---

## 10. Excel / CSV 역할

Excel / CSV는 P0 초기 구현의 선행조건이 아니다.

도입 후보 기능:

- 여러 차량 수치 비교
- 밸런스 시트
- 반복 Numeric Field 대량 수정
- Profile / Balance Value Import / Export
- 변경값만 Export

금지:

- Unreal Asset Reference를 단순 문자열 경로 중심으로 관리하는 External SSOT
- Excel과 DataAsset 양쪽을 동시에 Authoritative하게 운영
- Vehicle P0 검증 전 모든 도메인을 하나의 거대 Spreadsheet로 통합

---

## 11. UI 장기 방향

장기적으로 `CarFight Data Authoring` 또는 동등한 Editor Workspace를 목표로 한다.

Vehicle 화면 후보:

```text
Overview
Recipe
Assets
Layout
Driving Feel
Fitting
Compare
Validation
Advanced
```

공통 Browser 기능 후보:

```text
Search
Class / Type Filter
GameplayTag Filter
Sort
Multi-asset Compare
Changed-only View
Validation Status
Reference / Usage
Open Raw Asset
CSV Export / Import
```

초기 구현은 UI를 먼저 고정하지 않고 P0-00~02 계약 확정 후 결정한다.

---

## 12. 보호 범위

이번 작업에서 다음을 보호한다.

```text
현재 UCFVehicleData Runtime Apply 계약
현재 UCFVDAValidator 단일 Validator 원칙
현재 Inventory Definition / Instance 책임
현재 Fitting Compatibility / Installed State / Fitted Result 책임
현재 Sensor / Defense / Ammo 등 완료된 Runtime 계약
기존 Vehicle DA Wizard 기능
기존 CF-FQ-034 Fitting Technical Evidence
기존 CF-FQ-015 VehicleData Technical Evidence
기존 USER PIE Pending 상태
기존 dirty / untracked 사용자 작업
```

설계 단계에서 기존 Runtime 계약을 변경하지 않는다.

---

## 13. 명시적 비목표

P0 Architecture Freeze 전에는 다음을 하지 않는다.

- 새 Recipe DataAsset 생성
- 새 Profile DataAsset 생성
- Resolver C++ 구현
- 기존 Quick Tune 계산식 변경
- 기존 VDA Wizard 삭제
- VehicleData 대규모 구조 변경
- Inventory / Fitting Runtime 재설계
- Excel SSOT 도입
- Weapon Authoring 구현
- 자동 Balance AI 구현
- USER 주행감 자동 판정

---

## 14. P0 Gate 요약

상세 단계는 `DataAuthoringRoadmap.md`가 소유한다.

```text
DAUTH-P0-00 Current Authoring Foundation Audit
DAUTH-P0-01 Vehicle Field Ownership Matrix
DAUTH-P0-02 Authoring Contract Freeze
DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design
DAUTH-P0-04 VDA Wizard Migration Design
DAUTH-P0-05 Vehicle Authoring UX Design
DAUTH-P0-06 AI Authoring Contract
DAUTH-P0-07 Batch / Excel / CSV Contract
DAUTH-P0-08 Implementation Foundation
DAUTH-P0-09 Vehicle Authoring MVP
DAUTH-P0-10 Existing Wizard Migration
DAUTH-P0-11 Technical Validation
DAUTH-P0-12 USER Authoring Acceptance
```

P0-00~02는 기본적으로 설계 / 감사 단계이며 코드와 Content Asset 변경을 요구하지 않는다.

---

## 15. 선행 / 관련 문서

### 선행 설계 자료

```text
Document/Plan/DataPlan/CF_DAFillToolPlan.md
Document/Plan/DataPlan/CF_DAFillWizardUX.md
Document/Plan/DataPlan/CF_DrivingFeelTunePlan.md
```

이 문서들은 즉시 삭제하지 않는다.
새 Authoring 계약이 확정된 뒤 Current / Superseded / Historical 역할을 판정한다.

### 현재 구현 / 계획 참고

```text
Document/Systems/Vehicles/VehicleData.md
Document/Plan/VehicleDataTuning/VehicleDataTuningPlan.md
Document/Plan/VehicleFitting/VehicleFittingPlan.md
Document/Plan/InventoryFoundation/InventoryFoundationPlan.md
```

---

## 16. 완료 기준

Data Authoring P0는 최소한 다음이 성립해야 한다.

- VehicleData 전체 필드의 책임 분류가 완료됨
- 사람이 직접 입력해야 하는 필드와 자동 작성 가능한 필드가 구분됨
- Recipe / Profile / Derived / Override 필요성이 실제 필드 기준으로 검증됨
- AI가 Raw Field 임의 작성 없이 기본 차량 Definition을 만들 수 있는 계약이 있음
- VDA Wizard의 기존 유용 기능이 손실 없이 통합됨
- Inventory / Fitting Runtime 계약을 훼손하지 않음
- Compare / Diff / Validator를 통해 적용 전후 변경을 추적할 수 있음
- 기존 Raw DA Editor는 Advanced 경로로 계속 접근 가능함
- USER 주행감과 기술적 데이터 검증을 분리함

---

## 17. Changelog

### v0.2.28 - 2026-08-18

- `CF-FQ-032 UI-P0-06`의 explicit `RedlineStartRPM` additive VehicleData schema 변경을 bounded DAUTH compatibility로 반영했다. DAUTH USER Acceptance나 CF-FQ-038 Active 전환은 하지 않았다.
- P0-08 original 117 Registry / Performance numeric 17 / five-Profile numeric 78은 당시 검증의 Historical evidence로 보존한다. Current Source는 `VehicleMovementConfig.RedlineStartRPM` 1 leaf가 추가되어 118 Registry / Performance numeric 18 / five-Profile numeric 79다.
- 신규 field는 Performance domain `PerformanceProfileDirect`이며 `Profile.Performance` dependency를 사용한다. `RedlineStartRPM=0`은 미설정으로 유지하고 EngineMaxRPM/변속 설정에서 자동 유도하지 않는다.
- bounded compatibility evidence는 Registry `dacb88cb09644d0287cfe7a7f3137b51`, Batch resolved projection `03aad228b1784f1b908ed6e82ddced3e`, Batch allowlist `c9d07c42428642c186743e8c26043dcb` 각각 1/1 PASS다.
- P0-12 USER PASS 2와 UA-03 DriveState `RequiredProfileMissing` blocker, 다음 PC의 DriveState Profile binding→Handling B2 재검사 체크포인트는 그대로 보존한다.

Migration: 과거 117/78 문구를 전역 치환하지 않는다. 당시 P0-08 evidence는 Historical로 읽고 Current Registry/Batch 작업만 118/79를 사용한다. DAUTH USER Acceptance는 사용자 명시 재개 전 계속 Paused다.

### v0.2.27 - 2026-08-18

- fresh Editor에서 실제 저장 Recipe 이름이 `/Game/CarFight/Data/Authoring/DA_Recipe_TestSedan`임을 확인했다. 이전 checkpoint의 `DA_Recipe_UA_TestSedan` 표기는 사용자가 rename을 생략한 세션 중 이름이었으며 새 Recipe를 다시 만들거나 rename하지 않는다.
- UA-03 Profile Binding remediation USER recheck는 PASS다. Profile 0개 상태에서 `현재 연결: 없음 / 사용 가능 0 / 연결·열기·값 변경 비활성` fail-closed를 확인했고, 사용자가 `DA_Profile_UA_TestHandling`을 만든 뒤 후보 1개 발견, explicit `선택 프로필 연결 검토…` modal에서 Recipe Profile Binding만 변경 / VehicleData 직접 변경 없음 / Shared Profile 값 변경 없음 / Auto Save 없음 의미를 확인했다.
- 사용자가 Handling Profile binding을 실제 승인한 뒤 `DA_Recipe_TestSedan`은 dirty=true, `DA_TestSedan`은 dirty=false로 유지되어 Recipe-only binding 경계를 live read로 재확인했다.
- `Profile.Handling.FrontWheelMaxBrakeTorque = 2000` B2 preview는 affected Vehicle 1 / 예상 VehicleData 변경 0 / Auto Apply 0 / Auto Save 0을 사용자 화면에 표시해 Affected Vehicle Impact Preview USER 검사를 PASS했다.
- 실제 B2 commit은 하단 terminal에 `Shared Profile prospective validation이 commit 직전 Block됐습니다`로 종료됐고 `DA_TestSedan`은 dirty=false를 유지했다. 따라서 `2000` Profile value는 commit되지 않았으며 UA-03 전체 PASS로 확대하지 않는다.
- Source/Frozen 26.49 감사 결과 이는 결함이 아니라 의도된 fail-closed다. `DA_TestSedan`의 `DriveStateMode=VehicleSpecific` 상태에서 DriveState Profile Snapshot이 없어 `RequiredProfileMissing` Blocker가 존재하며, B2는 affected Vehicle prospective validation Blocked/Error 0을 전제로 한다. External Drift/Effective Stale/Shadow-only와 달리 Required Profile validation Blocker는 source commit을 막는 계약이다.
- 다음 PC Gate는 `DA_Profile_UA_TestDriveState` 생성 → Recipe의 `주행 상태` Domain에 explicit binding → `적용 차단 1` 해소 확인 → 동일 Handling `FrontWheelMaxBrakeTorque=2000` B2 preview/commit 재검사다. VehicleData Apply/Save All은 준비 동작으로 사용하지 않는다.
- PC 종료 전에는 `DA_Recipe_TestSedan`과 새 `DA_Profile_UA_TestHandling` 두 Authoring Asset만 저장하고 일반 종료하도록 안내했다. 실제 저장/종료 완료는 별도 확인 전까지 추정하지 않는다. USER PASS는 2 유지한다.

Migration: UA-01~02, Initial Import, Profile Binding remediation 기술 검증과 현재 impact preview를 반복하지 않는다. 다음 PC 세션은 DriveState Profile requirement 해소 후 동일 Handling B2 commit 재검사부터 시작한다.

### v0.2.26 - 2026-08-18

- UA-03 재개 후 live Editor에서 `/Game/CarFight/Data/Authoring/DA_Recipe_UA_TestSedan` 생성 성공과 `DA_TestSedan` Browser의 `관리됨` 전환을 사용자가 직접 확인했다. Recipe는 dirty, Target `DA_TestSedan`은 dirty=false였으며 VehicleData Apply/Save는 수행하지 않았다.
- 첫 UA-03 Recipe/Profile 화면에서 현재 Recipe가 `레거시 가져옴` 상태이고 Shared Profile binding이 비어 있으며, Production Workspace에 Frozen 24.25의 existing Profile 선택/명시적 Recipe binding 경로가 없어 Shared Profile/Affected Vehicle USER 검사를 정상 진행할 수 없음을 확인했다. 프로젝트 Authoring 경로에는 실제 Shared Profile Asset도 0개였다.
- remediation으로 existing `FCFVehicleAuthoringService::ListProfiles`와 `BindVehicleProfile` R1 semantic contract를 재사용해 5 Domain별 existing Profile 후보, current binding, explicit Recipe-only `선택 프로필 연결 검토…`, `현재 프로필 열기` route를 추가했다. Profile payload/Target direct writer, Auto Apply, Auto Save는 추가하지 않았다.
- `CFVehicleAuthoringVMTests.cpp v1.6.0`의 SharedProfileImpact는 raw `ProfileBindings` fixture 대입 대신 actual reviewed `CommitProfileBinding`을 통과하도록 강화했다.
- 첫 remediation Build `eb2eb56867ba4557ad325fc04cf4f7a4`는 header forward declaration 누락으로 compile FAIL했고 즉시 교정했다. final Official Build `74d818f4c7a149b89d730c2a615912c2` Exit 0 PASS, focused `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.SharedProfileImpact` process `c81052d602ae4f869d6aecb431b28d6c` 1/1 PASS / Failure 0 / Engine Exit 0이다.
- USER PASS는 2를 유지한다. 새 UI는 fresh Editor binary reload가 필요하지만 현재 user-owned Editor의 `DA_Recipe_UA_TestSedan`이 unsaved이므로 Browser가 임의 저장/종료하지 않았다. 다음은 Recipe 보존 방식에 대한 사용자 명시 선택 후 fresh Editor에서 UA-03 Profile binding/Shared Profile/Affected Vehicle impact를 재확인한다.

Migration: UA-01~02와 Initial Import commit을 반복하지 않는다. `DA_Recipe_UA_TestSedan`을 잃지 않도록 user-owned Editor의 unsaved 상태를 먼저 처리한 뒤 fresh binary에서 UA-03 USER recheck를 진행한다. VehicleData Apply와 Save All을 UA-03 준비 동작으로 사용하지 않는다.

### v0.2.25 - 2026-08-18

- UA-03 착수를 위해 관리됨 차량이 0개임을 사용자 화면에서 확인했다.
- UA-03 USER Acceptance용 관리 차량 준비 대상으로 기존 미관리 `/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan`을 선택하고 새 Editor-only Recipe 이름을 `DA_Recipe_UA_TestSedan`으로 지정했다.
- 사용자가 `제작 관리 시작` actual commit을 실행했으며 세션 전환 요청 시점에는 아직 생성 중이므로 성공/실패, 관리됨 전환, dirty package 상태를 완료로 추정하지 않는다.
- 다음 세션 첫 단계는 현재 Editor 상태를 보존한 채 생성 terminal 결과를 확인하고, 성공 시 `DA_TestSedan`의 `관리됨` 표시와 새 Recipe 존재를 확인한 뒤 UA-03 Recipe / Shared Profile / Affected Vehicle Impact 사용자 검사를 이어가는 것이다.
- Auto Save/VehicleData Apply/추가 저장은 이번 checkpoint에서 수행 완료로 기록하지 않는다. USER PASS는 2를 유지한다.

Migration: 새 세션에서 UA-01~02를 반복하지 않는다. `DA_Recipe_UA_TestSedan` 생성을 재실행하기 전에 현재 생성 결과부터 확인한다.

### v0.2.24 - 2026-08-18

- 사용자가 fresh Editor에서 Mesh-only Candidate 표시와 `메시에서 차량 만들기` 검토 의미를 직접 확인해 `UA-02 Browser / Mesh-only / Create / Existing Import = USER PASS`로 판정했다.
- UA-02는 Existing Vehicle `제작 관리 시작` subflow와 Mesh-only Candidate/Create subflow를 모두 USER PASS로 닫았다. 실제 Recipe/VehicleData 생성 commit은 USER acceptance에서 강제하지 않았다.
- P0-12 USER PASS checkpoint를 2로 전진하고 다음 USER Gate를 `UA-03 Recipe / Shared Profile / Affected Vehicle Impact`로 이동했다.
- DG/DEL/Wizard deletion은 계속 미개방이며 P0-08~11 및 UA-01~02를 반복하지 않는다.

Migration: bounded USER Acceptance를 이어갈 때는 UA-03부터 시작한다. UA-02 Existing Vehicle/Mesh-only 검사를 재실행하지 않는다.

### v0.2.23 - 2026-08-18

- UA-02 Mesh-only USER 검사에서 기본 Browser에 `차체 메시 후보`가 0개로 보여 subflow를 FAIL로 판정했다.
- live Asset Registry에서 `/Game/CarFight/Vehicles/Meshes` 아래 SUV/Sedan 외 Wagon/CityCar/Van/Compact/Coupe/Pickup/SubCompact StaticMesh가 존재함을 확인했다. Source RCA 결과 Mesh-only discovery가 기존 Definition Chassis의 exact vehicle folder만 재귀 검색해 sibling vehicle folders를 놓치고 있었다.
- `CFVehicleAuthoringService.cpp v1.5.0`은 hardcoded `/Game` scan 없이 기존 Chassis folder의 한 단계 위 common vehicle mesh collection root를 계산하고 그 root 아래 StaticMesh를 bounded recursive query하도록 교정했다. used Chassis 및 actual VehicleData WheelMesh 제외 규칙은 유지한다.
- `CFVehicleAuthoringVMTests.cpp v1.5.0`의 MeshCreate fixture는 candidate를 existing chassis와 다른 sibling vehicle directory에 두도록 강화했다.
- official Build `82ac0966aa764bc282a92e59495a6be2` PASS / Exit 0, focused MeshCreate process `2a6729146c0c4e1985d53de73cb54665` 1/1 PASS / 0 FAIL / Engine Exit 0을 확보했다.
- 현재 UA-02 Mesh-only subflow는 `Remediation Technical PASS / USER Recheck Pending`이며 fresh Editor에서 candidate visibility를 사용자가 직접 확인하기 전 PASS하지 않는다.

Migration: Existing Vehicle `제작 관리 시작` USER PASS는 반복하지 않는다. 다음 bounded step은 fresh Editor restart 후 Mesh-only Candidate 표시와 `메시에서 차량 만들기` 검토 UX 재확인이다.

### v0.2.22 - 2026-08-18

- 사용자가 fresh Editor에서 미관리 VehicleData의 `제작 관리 시작 검토…` 팝업 도달성과 용어 의미를 직접 확인해 UA-02 Existing Vehicle subflow를 USER PASS 처리했다.
- USER 확인 범위는 `기존 VehicleData는 그대로 두고 편집용 Recipe를 새로 만들어 Authoring 관리를 시작한다`는 의미 이해와 review modal 정상 표시까지다. 실제 Recipe 생성 commit은 이번 USER checkpoint에서 수행하지 않았다.
- P0-12 전체 USER PASS count는 UA-01 기준 1을 유지하며 UA-02 전체는 아직 In Progress다. 다음 확인은 Mesh-only Candidate → `메시에서 차량 만들기` flow다.

Migration: UA-02 Existing Vehicle `제작 관리 시작` 검토는 반복하지 않는다. 다음 bounded USER step은 Mesh-only Candidate/Create flow부터 진행한다.

### v0.2.21 - 2026-08-18

- `UA-02 Browser / Mesh-only / Create / Existing Import` 첫 USER 검사에서 미관리 Vehicle의 `기존 차량 가져오기 검토…` 버튼이 무반응처럼 보이고 용어 자체도 의미 전달에 실패해 UA-02를 FAIL로 판정했다.
- RCA 결과 Recipe 이름 입력칸이 실제 값 없이 hint만 가진 상태였고 empty name이 Core preview에서 fail-closed됐지만 Slate handler가 실패 메시지를 표시하지 않고 return해 무반응처럼 보였음을 확인했다. Core Import/approval 계약 결함은 아니었다.
- `CFVehicleAuthoringTab.cpp v1.4.0`에서 UX 용어를 `제작 관리 시작`으로 변경하고, 미관리 VehicleData 선택 시 `DA_Recipe_<VehicleStem>` 기본 이름 제안, 저장 폴더/새 레시피 이름 visible label, preview/commit 실패 modal feedback을 추가했다. 기존 VehicleData는 그대로 두고 새 Editor-only Recipe만 만든다는 설명을 명시했다.
- official Build `388a625d1ed24abda9ded23fe47be88c` PASS / Exit 0, focused `CarFight.DataAuthoring.DAUTH_P0_09.Workspace.InitialImport` process `3e74ee05ff634ed298e30ec4259f4473` 1/1 PASS / 0 FAIL / Engine Exit 0을 확보했다.
- UA-02 USER PASS는 아직 올리지 않는다. 새 binary/UI를 사용자가 직접 다시 확인한 뒤에만 Existing Vehicle 관리 시작 subflow를 PASS할 수 있다.

Migration: P0-08~11 및 UA-01을 반복하지 않는다. 다음은 Editor를 fresh reload한 뒤 UA-02의 `제작 관리 시작` 화면/검토창 USER 재확인이다.

### v0.2.20 - 2026-08-18

- 사용자가 remediation 적용 후 새 Workspace를 직접 재확인해 `UA-01 Workspace First Impression = USER PASS`로 판정했다.
- USER evidence는 한국어 우선 가독성, 기본 Browser relevance, `DA_PoliceCar`/테스트·레거시 기본 숨김, WheelMesh 차체 후보 오분류 제거 상태를 포함한다.
- `P0-12 USER PASS` checkpoint는 1로 전진하고 다음 USER Gate를 `UA-02 Browser / Mesh-only / Create / Existing Import`로 이동했다.
- 전역 우선순위상 `CF-FQ-038 = Paused`와 DG/DEL/Wizard deletion 미개방은 그대로 유지한다. P0-08~11 Technical PASS를 반복하지 않는다.

Migration: DAUTH bounded USER Acceptance를 이어갈 때는 UA-02부터 시작한다. UA-01 기술/사용자 검증을 재실행하지 않는다.

### v0.2.19 - 2026-08-18

- UA-01 remediation 후 user-owned Editor 종료 상태에서 fresh official Build `97df66558d694369915fcfa7d08ef261`을 실행해 Exit Code 0 / Succeeded를 확인했다.
- 직접 관련 focused regression `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate`를 Process `7c5ad7cf8d744994bcb1c2229e5f961e`로 실행해 1/1 PASS / 0 FAIL / Engine Exit 0을 확인했다. WheelMesh→Mesh-only Chassis Candidate 오분류 방지 회귀가 포함된다.
- fresh Editor start `2d70f68ea0f747169d70b9d2afa30bce`는 editor_ready=true / port_8100_owned_by_editor=true / protocol_ready=true로 완료했다.
- 따라서 UA-01 remediation은 Technical PASS다. 다만 한국어 우선 가독성, 기본 Browser relevance, DA_PoliceCar/테스트·레거시 숨김의 실제 사용성은 사용자가 새 화면을 직접 확인해야 하므로 UA-01 전체 USER PASS는 아직 0이다.

Migration: 다음 단계는 추가 기술 재구현이 아니라 새 Editor lifetime에서 UA-01 사용자 재확인이다. PASS 전에는 UA-02로 이동하지 않는다.

### v0.2.18 - 2026-08-18

- 보존된 `DAUTH-P0-12 / UA-01 Workspace First Impression`의 실제 사용자 피드백을 기록했다. 진입 위치 발견은 쉬웠지만, 영문 중심 UI로 즉시 이해가 어렵고 폐기 `DA_PoliceCar` 및 기술/테스트 레코드가 기본 Browser를 혼잡하게 해 UA-01 전체는 PASS하지 않았다.
- 스크린샷과 Source audit에서 `Wheel_FL/FR/RL/RR` 같은 VehicleData WheelMesh 참조가 Mesh-only Chassis Candidate로 오분류될 수 있는 결함도 확인했다.
- UA-01 remediation source는 한국어 우선 Workspace, 기본 `테스트/레거시 표시` off, exact 폐기 `DA_PoliceCar` 기본 숨김, 실제 WheelMesh 참조의 Mesh Candidate 제외로 수정했다. Asset 삭제, Runtime `UCFVehicleData`, Inventory/Fitting, Wizard DG/DEL은 변경하지 않았다.
- 공식 Build `d98c2c48481a453891d7f9471253d5cb`는 변경 C++ compile 단계까지 통과했으나 사용자 사용 중 Editor가 `UnrealEditor-CarFight_ReEditor.dll`을 점유해 link `LNK1104`로 종료됐다. 코드 FAIL로 판정하지 않으며 Editor 사용자 종료 후 한 번의 fresh official build와 직접 관련 MeshCreate focused regression만 재개한다.
- UA-01 상태는 `FAIL → Remediation / Recheck Pending`, USER PASS는 계속 0이다. 새 UI를 사용자가 다시 확인하기 전 UA-02로 이동하지 않는다.

Migration: 전역 일정상 CF-FQ-038 Paused projection은 유지한다. DAUTH를 재개하거나 이 bounded UA-01 follow-up을 계속할 때는 P0-08~11 전체 Technical PASS를 반복하지 말고 Editor 종료 → fresh build → MeshCreate focused regression → UA-01 사용자 재확인 순서만 수행한다.

### v0.2.17 - 2026-08-18

- 사용자 우선순위 변경에 따라 `CF-FQ-038`을 `DAUTH-P0-12 USER Authoring Acceptance` 체크포인트에서 Paused로 전환했다.
- P0-08A~M/P0-09~11 Technical PASS, final Build `98dfceab797942218bd09c844e398ceb`, full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS를 그대로 보존한다.
- P0-12 USER PASS는 0이며 재개 지점은 `UA-01 Workspace First Impression`이다. USER 결과를 추정하지 않는다.
- DG/DEL Gate와 `SCFVDAWizardTab` 삭제는 계속 미개방/금지다. 이 pause는 Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset 계약을 변경하지 않는다.

Migration: CF-FQ-038 재개 시 P0-08A~M/P0-09~11을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.25 / DAUTH-P0-12 UA-01`에서 시작한다. UI 우선 작업 중 자동 재개하지 않는다.

### v0.2.16 - 2026-08-18


- `DAUTH-P0-12 USER Authoring Acceptance`를 실제 사용자 검증 단계로 착수했다.
- P0-08A~M/P0-09~11 Technical PASS는 재실행·재판정하지 않고 그대로 보존한다.
- Acceptance 대상은 Frozen Section 24의 normal single-Vehicle 흐름인 Vehicle Browser → Mesh-only/Create/Import → Recipe/Profile authoring → affected Vehicle impact → External Drift 3-way recovery → Resolve/Diff/Source Trace/Validation → explicit Apply/Undo와 Driving Feel workflow다.
- P0-12의 PASS 근거는 Automation이 아니라 사용자의 실제 화면·입력·주행 판단이다. 현재 USER PASS는 0이며 결과를 추정하지 않는다.
- DG/DEL Gate와 `SCFVDAWizardTab` 삭제는 P0-12 USER Acceptance 완료 전까지 열지 않는다.

### v0.2.15 - 2026-08-18

- `DAUTH-P0-11 Frozen UX Completeness Closure`를 완료해 P0-11 overall을 Technical PASS로 닫았다.
- Frozen 24.90 Existing Import의 Handling/Performance Adoption은 existing generic Import/Adoption Core를 그대로 사용하고 normal Workspace route만 추가했다.
- Frozen 24.91/24.93 Shared Profile single-Vehicle authoring은 stable Batch ColumnId + canonical numeric input을 existing ProfileNumericEdit 3-way/B2 source commit에 연결하고 affected Vehicle / pending Definition impact navigation을 추가했다. dependent Definition auto Apply/Save/retry는 0이다.
- Frozen 24.92 External Drift는 Last Applied / Current Raw / Current Authoring 3-way와 Keep Authoring / Preserve Raw As Legacy Pin / Promote Raw To Advanced Override reviewed decision을 구현했다. Preserve/Advanced는 Recipe-side ownership mutation만 수행하고 existing Target writer는 계속 `FCFVehicleApplyService` 하나다.
- 새 Apply trace는 exact typed `LastAppliedValue`를 보존하되 기존 `LastAppliedValueHash` authority를 유지한다. Drift prospective simulation duplicate의 `PostDuplicate()` RecipeId 재발급으로 approval hash가 비결정적이던 defect는 원본 RecipeId 복원으로 교정했다.
- Frozen 24.94 Mesh-only Candidate는 existing Chassis sibling directory에서 unused StaticMesh만 bounded projection하고, explicit R2 OwnershipWrite 뒤 Definition + Editor-only Recipe 두 record만 생성한다. Profile/차급/물리/밸런스 추론과 auto Apply/Save는 0이다.
- final Build `98dfceab797942218bd09c844e398ceb` PASS, focused P0-11 `b53998e6cacf48a1a554d784b77e013c` 6/6 PASS, focused result JSON SHA-256 `bce3414ec30cb4612209ff1f39f649990140950b73d4136929a3fa8a4be47263`를 확보했다.
- full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS / Engine Exit 0, result JSON SHA-256 `a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f`를 확보했다.
- Runtime `UCFVehicleData`, Inventory/Fitting production source, Content Asset, `SCFVDAWizardTab` 삭제, DG/DEL은 변경/개방하지 않았다. 다음 dependency는 `DAUTH-P0-12 USER Authoring Acceptance`지만 이번 작업에서는 착수하지 않았고 USER UX/Driving Feel PASS를 추정하지 않는다.

### v0.2.14 - 2026-08-18

- `DAUTH-P0-11` missing E2E coverage 감사 결과 기존 단위 테스트를 반복하지 않고 representative Authoring E2E와 applied VehicleData consumer regression 2개만 보강했다.
- Core technical behavior는 PASS다: Recipe edit→approval invalidation→fresh Resolve/Trace/Diff/Validation→Apply→Undo→re-Apply→Raw Drift→normal Apply fail-closed와 기존 Fitting/Inventory canonical `UCFVehicleData` consumption을 확인했다.
- final Build `ae0e92c60a094be09b036ae12b106717` PASS, focused P0-11 2/2 PASS, Inventory representative 1/1 PASS, Fitting representative 2/2 PASS, full Data Authoring `071d20db24f24c15bb2cb6c553973433` 59/59 PASS를 확보했다. result JSON SHA-256은 `a78df08c8e8d29f02d8906d0805dbd621aba8b5a0e4a07c7f3b64d1c3722475f`다.
- Frozen 24.90~24.94 전체 workflow audit에서 normal Workspace Handling/Performance Adoption UI, Shared Profile authoring/affected Vehicle impact, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh production flow가 아직 없음을 확인했다.
- Frozen 24.97의 scoped P0-09/P0-10 범위를 고려해 과거 Technical PASS를 소급 취소하지 않지만, 이번 P0-11 전체 scope에서는 completeness blocker이므로 P0-11은 `In Progress`를 유지하고 P0-12를 Ready로 올리지 않는다.
- P0-11에서 production Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset, Wizard를 변경하지 않았다. DG/DEL Gate도 열지 않았다.
- 다음 checkpoint는 `DAUTH-P0-11 Frozen UX Completeness Closure`다.

### v0.2.13 - 2026-08-18

- `DAUTH-P0-10 Existing Wizard Migration`을 Technical PASS로 반영했다.
- 새 Vehicle Authoring Workspace에 Assets/Layout semantic intent + derived socket readback, explicit Wheel Measurement decision, Frozen 4축 Driving Feel/preset, read-only 117-field Reference Compare, Stable-ID Hardpoint/Mount/Default intent와 standard Undo를 parity했다.
- 새 Workspace/ViewModel은 Common Authoring facade만 사용한다. P0-10 facade는 기존 SnapshotBuilder/Resolver/ImportService Core를 orchestration하며 raw Target writer, 새 Source Truth, generic SetField를 만들지 않는다.
- Legacy Wizard는 managed Recipe Target에서 Layout Capture / Quick Tune Apply / Quick Tune Revert를 facade managed-state guard로 막고 read-only Validator/Compare/Open/Copy와 unmanaged legacy path를 유지했다.
- P0-10은 기존 Wizard 삭제 또는 Deprecated 판정이 아니다. DG/DEL Gate는 열지 않았고 `SCFVDAWizardTab` / `CarFight.VehicleDAWizard`는 그대로 유지한다.
- final Build `03fa78af4e124a3db33893ca8bfe436d` PASS, focused P0-10 `3e6c924318124fbebc742e2400d83308` 4/4 PASS, full Data Authoring `c25dbb8e10eb4bdda5733ead78aa4a43` 57/57 PASS, JSON SHA `90c01eda8b1df96f03d55feb1e3000f99db16f8a717baa697f5c15cdb9f6884b`를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset, Batch main page는 변경하지 않았다. USER UX/Driving Feel Acceptance는 P0-12에 남긴다.
- 다음 Gate는 `DAUTH-P0-11 Technical Validation`이다.

### v0.2.12 - 2026-08-17

- `DAUTH-P0-09 Vehicle Authoring MVP`를 Technical PASS로 반영했다.
- `CarFight.VehicleAuthoring` Nomad Tab과 transient ViewModel을 추가해 single-Vehicle Browser/Selection, Initial Import, Recipe basic intent, Resolver Preview, Pending Diff, Source Trace, Validation, Apply/standard Undo, Raw DA Open의 최소 Workspace를 구현했다.
- UI/ViewModel은 Common Authoring facade만 사용하고 Core Snapshot/Resolver/Import/Apply/Validator 규칙을 복제하지 않는다. Apply Target mutation authority는 기존 `FCFVehicleApplyService`만 유지한다.
- Initial Import는 R2 OwnershipWrite exact proposal/TOCTOU 계약으로 Target mutation 없이 새 Editor-only Recipe만 생성하며 preview/commit auto-save/retry는 0이다.
- `Workspace.ApplyUndo`에서 existing Apply transaction 뒤 Unreal standard Undo가 Target DefinitionHash와 Recipe AppliedState를 실제 복원하는 것을 검증했고, stale prepared approval도 Target mutation0으로 차단했다.
- 기존 `SCFVDAWizardTab`과 `CarFight.VehicleDAWizard`를 그대로 보존하고 new Workspace tab과 병행 등록했다. Batch main page와 P0-10 parity 범위는 구현하지 않았다.
- final Build `76fc476c8ccb4daf895a3b567fe0c992` PASS, focused P0-09 `ef56e0b60ae944c19bf481fa070503c9` 6/6 PASS, final Data Authoring `fba6b0793f90450caf2a38eb82844e7c` 53/53 PASS, JSON SHA-256 `697230dbce1c9a6281a0e56c4aa292544d3becd28b701a25285a487a50592654`를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset은 변경하지 않았다. P0-09 Technical PASS를 P0-12 USER Authoring Acceptance로 확대하지 않는다.
- 다음 Gate는 `DAUTH-P0-10 Existing Wizard Migration`이다. P0-10 자체는 Wizard deletion approval이 아니다.

### v0.2.11 - 2026-08-17

- `DAUTH-P0-08M B3 Batch Definition Apply Foundation`을 Technical PASS로 반영하고 `DAUTH-P0-08 Implementation Foundation` 전체를 Technical PASS로 닫았다.
- fresh per-target R3 evidence, exact BatchApplyPlanHash/ordered Target/per-target evidence approval, global preflight와 TargetPath ascending `FCFVehicleApplyService::Apply` exact-once orchestration을 구현했다.
- NoChange/ShadowOnly/ExternalDrift/Resolve Error·Blocked/Validation Blocked는 B3 ineligible이며 External Drift bulk resolution은 없다.
- B3는 batch global atomic이 아니라 per-Vehicle atomic이고 Stop On First Failure 시 Applied/Failed/NotStarted를 보존한다. 이미 성공한 Vehicle의 automatic rollback/실패 이후 auto continue/automatic retry/save는 0이다.
- B1/B2 approval과 B3 approval은 별도 type이며 source-commit approval 재사용을 금지했다.
- final official Build `10027d18360e48f399a5b439f280c24c` PASS, focused M process `a49d21b6b9b94d648142d5a555a79c95` 4/4 PASS, final targeted Automation `cb2ae69e9c834657a553fe52c00f5a96` 47/47 PASS, result JSON SHA-256 `472ef5bb8f5fdeb46ebd1b7e68f1fc5a66acba065828ff025167b475aaf25595`를 기술 evidence로 기록했다.
- Frozen 26.66~26.69는 M의 선행 dependency가 아니라고 판정했다. BatchOperationId/generic result envelope는 첫 실제 external Batch transport/client integration 전에 다시 여는 follow-up으로 보존한다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader를 변경하지 않았고 Batch UI/file save도 구현하지 않았다.
- `DAUTH-P0-09 Vehicle Authoring MVP`는 Technical PASS다. 다음 Gate는 `DAUTH-P0-10 Existing Wizard Migration`이다.

### v0.2.10 - 2026-08-17

- `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`을 Technical PASS로 반영했다.
- exact BatchPlanHash/Included RowId/ProposedPatchHash/current fingerprint에 binding된 Batch Authoring approval과 B1 Recipe Numeric Commit / B2 Shared Profile Numeric Commit을 구현했다.
- 모든 included source global TOCTOU preflight를 mutation 전에 끝내고 stale 하나라도 있으면 whole plan mutation 0, one logical `FScopedTransaction`, deterministic typed numeric patch, AuthoringRevision +1, semantic fingerprint/revision postcheck와 all-or-nothing rollback을 구현했다.
- B1은 Input/Recipe validation layer만 source blocker로 fresh 재검사하고 External Drift/ShadowOnly/downstream Definition validation을 source blocker로 승격하지 않는다. B2는 affected Recipe exact inventory와 prospective affected-Vehicle validation을 commit 직전 fresh 재검사한다.
- successful B1/B2는 source package Dirty만 남기고 automatic save와 Target mutation은 수행하지 않는다. old Preview/Approval은 source fingerprint 변화로 stale해져 fresh read/resolve가 필요하며 B3 approval로 재사용할 수 없다.
- focused BatchCommit `0436e7f86e17493aa24c6e4ff8d9bd05` 6/6 PASS, final official Build `2e2923f31d5a4f3fa703f3b7623c0741` PASS, final targeted Automation `3853d42dfb344bfd8eb6e516903a8d78` 43/43 PASS, result JSON SHA-256 `eef2051ec4b3ccb5dce10b8b76761299885d18a8bf7f2c9db42cc2967320abcf`를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader를 변경하지 않았고 B3 Definition Apply, UI, file save도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 Frozen Section 26.54~26.65의 `DAUTH-P0-08M B3 Batch Definition Apply Foundation`이다. `P0-08M`은 Frozen B3 구간을 Roadmap에서 지칭하기 위해 새로 부여한 implementation checkpoint label이다.

### v0.2.9 - 2026-08-17

- `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`을 Technical PASS로 반영했다.
- canonical CSV/companion manifest parse와 current Registry/schema/ExportSetHash validation, duplicate identity/read-only-reserved guard를 구현했다.
- current Unreal을 import 시 다시 읽고 Export Baseline / Edited / Current 3-way의 SafeCandidate, NoSpreadsheetChange, ConvergedNoChange, ConcurrentEditConflict와 OwnershipChangedSinceExport를 구현했다. Fingerprint mismatch는 signal-only다.
- transient Recipe/Profile typed numeric patch 후 existing Authoring facade/Pure Resolver를 재사용해 prospective per-Vehicle Diff/Validation/Drift를 계산하며 persistent Recipe/Profile/Target mutation은 0이다.
- per-row status/issues, shared Profile affected Vehicle drill-down, preview summary와 deterministic BatchPlanHash를 구현했다.
- final official Build `a3d37d4c8520442d8ceb09a72eb6a68f` PASS, targeted Data Authoring Automation `7d2db5a9baa54dd9abf25857138d3994` 37/37 PASS, result JSON SHA-256 `bbdf497d472e5ec2fbe4458e075907897ca79b2ac984d43ba3290cff3c55eeec`를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader를 변경하지 않았고 B1/B2 source commit, B3 Definition Apply, UI, file save도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 Frozen Section 26.44~26.53의 `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`이다. B3는 Section 26.54+로 분리한다.

### v0.2.8 - 2026-08-17

- `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`을 Technical PASS로 반영했다.
- Batch Registry는 existing Recipe/Profile typed schema와 117 Field Registry projection으로만 구현해 CSV/Spreadsheet가 별도 Authoring Source/SSOT가 되지 않게 했다.
- Recipe editable numeric 7개, 5 Profile typed numeric 총 78개, reserved `__cf_` metadata, stable ColumnId, typed mutation metadata와 source-mode editability를 구현했다.
- canonical BOM-less UTF-8 CSV와 immutable-style `.cfbatch.json` baseline manifest/ExportSetHash를 구현하고 row/column order determinism, numeric invariant, reserved collision, manifest tamper, persistent mutation0을 Automation으로 검증했다.
- final official Build `3249098c1b99487a8fd173694573bd5e` PASS, targeted Data Authoring Automation `534486583a1d4689bc5137505a03f806` 32/32 PASS, result JSON SHA-256 `84fc15de1ad23b4e13891d91db09fa3668a49c900a438dec17b70d818a45d36f`를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader를 변경하지 않았고 Import Session/B1·B2/B3/UI도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`이다.

### v0.2.7 - 2026-08-17

- `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`을 Technical PASS로 반영했다.
- `FCFVehicleAuthoringService`는 기존 SnapshotBuilder / AssetReader / Resolver / ApplyService의 orchestration facade로만 구현하고 Core source precedence/validation/write 책임을 복제하지 않았다.
- R0 read facade, persistent mutation 없는 prospective `PreviewRecipeChange`, exact approval/expected-state 기반 R1 Recipe semantic transaction, bounded ClientOperationId dedupe와 typed result/error/mutation footprint를 구현했다.
- R1은 Raw SetField/direct VehicleData write/force/skip-validation/auto-retry/auto-save를 제공하지 않으며, `ApplyResolvedVehicle`은 existing `FCFVehicleApplyService` lane만 사용한다.
- Frozen Section 22.27 RecipeFingerprint에서 `VehicleArchetypeId`를 임의 추가하지 않고 typed desired-state equality로 non-resolver semantic NoChange/commit을 처리하도록 facade v1.1.0을 교정했다.
- final official Build `252fdbd097f943379f2a1e2a942bedef` PASS와 targeted Data Authoring Automation `f2011a206c964fadb269d13c4dba3c86` 27/27 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator, Asset Reader는 변경하지 않았고 UI/CSV/Batch도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`이다.

### v0.2.6 - 2026-08-17

- `DAUTH-P0-08H Apply Transaction Foundation`을 Technical PASS로 반영했다.
- `FCFVehicleApplyService`를 유일한 production Target writer로 추가하고 five-hash/signature/revision TOCTOU precondition, fresh Resolver/Diff revalidation, transient duplicate preflight와 existing Validator를 구현했다.
- Section 22.31 dependency order로 current-only stable selector removal, Hardpoint/Mount add, scalar/array leaf write를 실행하며 R14 Resolver Foundation의 source/precedence 의미는 변경하지 않았다.
- Target readback hash와 actual Validator 통과 뒤에만 Recipe AppliedState를 갱신하며, A8~A10 실패 시 Target/AppliedState/dirty state를 자체 rollback하고 transaction을 cancel한다. automatic Save는 수행하지 않는다.
- final official Build `b88831b71797415fac9dc0a23d12c39e` PASS와 targeted Data Authoring Automation `6f261c0b76f04d0faa6b9e855865818e` 23/23 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator, Asset Reader는 변경하지 않았고 UI/CSV/Batch도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`이다.

### v0.2.5 - 2026-08-17

- `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`을 Technical PASS로 반영했다.
- Current Definition Snapshot exact field를 Recipe ImportState의 normal Legacy Pin과 Mount hidden LegacySerialized passthrough로 lossless partition하고 direct semantic candidate copy를 구현했다.
- semantic candidate copy는 source ownership을 자동 이전하지 않으며 Movement raw → Driving Feel/Profile inverse inference와 auto Profile binding을 금지 상태로 유지한다.
- group/field Adoption Preview는 persistent mutation 없이 virtual Pin removal + fresh Resolver/Diff/Validation을 수행하고, approved Commit은 stale fingerprint precondition 아래 Recipe Pin/AdoptedGroups/ManageState/revision만 transaction으로 갱신한다.
- final official Build `b201d87cd13f4987a0907e08c8f00a6c` PASS와 targeted Data Authoring Automation `3b52a251ab244096b78bb872a06c0069` 20/20 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator, Asset Reader를 변경하지 않았고 Target Definition mutation / Apply / UI / CSV도 0이다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`이다.

### v0.2.4 - 2026-08-17

- `DAUTH-P0-08F Definition Materializer / Validation Foundation`을 Technical PASS로 반영했다.
- `SortedResolvedFields`를 transient `UCFVehicleData` candidate로 복원하고 Stable-ID Hardpoint/Mount exact selector 재구성, FieldCodec typed import와 Resolver-owned readback hash consistency를 구현했다.
- 기존 `UCFVDAValidator::ValidateVehicleData(Candidate, nullptr)`를 수정 없이 R15에 연결하고 `DefinitionValidation`을 실제 결과로 채우도록 했다.
- Definition Error/Blocked와 Materializer internal Error를 분리하고, Definition Snapshot/Resolver/materialized readback이 기존 hash format revision 1 authority를 공유하도록 통합했다.
- final official Build `e5d925c6531e4ae6ac58aa356e4078eb` PASS와 targeted Data Authoring Automation `4b21d25b780c4a398de7a1b47c595c5e` 17/17 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader는 변경하지 않았고 Apply / UI / CSV도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 Gate는 `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`이다.

### v0.2.3 - 2026-08-17

- `DAUTH-P0-08E Pure Resolver Foundation`을 Technical PASS로 반영했다.
- Snapshot-only `FCFVehicleResolveRequest/Result`, Frozen R0~R16 stage identity, deterministic source candidate/precedence, Proposal/Adoption, Source Trace/Hash, R14 Diff와 R16 Stale/Drift foundation을 구현했다.
- R15 transient Definition materialization + existing `UCFVDAValidator` integration은 구현하지 않고 다음 `DAUTH-P0-08F Definition Materializer / Validation Foundation`으로 분리했다.
- final official Build `db202f0797af41fe86a859609cb4dfd9` PASS, targeted Data Authoring Automation `93d445f78cbe4eaabca1478eb6d27078` 15/15 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset과 Asset Snapshot Reader는 변경하지 않았고 Apply / UI / CSV도 구현하지 않았다.
- `DAUTH-P0-08`은 아직 Foundation 잔여 범위가 있으므로 `In Progress`를 유지한다.

### v0.2.2 - 2026-08-17

- `DAUTH-P0-08D Asset Snapshot Reader Foundation`을 Technical PASS로 반영했다.
- Chassis requested socket relative transform facts와 4개 Wheel StaticMesh local bounds를 live UObject와 분리한 immutable-style `FCFVehicleAssetSnapshot`으로 구현했다.
- Chassis Layout / Wheel Measurement fingerprint를 resolver-relevant extracted facts에만 결합해 Material 등 무관한 package 변경이 Authoring stale을 만들지 않는 Section 22.27 경계를 구현했다.
- final official Build `780d30c4a44f48feb179f7f6da5480e1` PASS, targeted Data Authoring Automation `ab2cd64308484ce0ae0e24cb1143ca33` 9/9 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting 소비 경로, `SCFVDAWizardTab`, Content Asset은 변경하지 않았고 Resolver / Apply / UI / CSV는 아직 구현하지 않았다.
- 다음 Gate는 `DAUTH-P0-08E Pure Resolver Foundation`이다.

### v0.2.1 - 2026-08-17

- `DAUTH-P0-08C Immutable Snapshot Foundation`을 Technical PASS로 반영했다.
- `CFVehicleSnapshotTypes` / `CFVehicleSnapshotBuilder`로 Recipe, 5 Profile, Current Definition, Project Compatibility Default의 immutable-style value-copy Snapshot 기반을 구현했다.
- Registry Stable-ID array를 exact Definition field entry로 확장하고 deterministic fingerprint/hash와 Frozen Section 22.17 `RequiredDependencies` metadata를 구현했다.
- final official Build `6c81b2149de44d00a99554a44d2135aa` PASS, targeted Data Authoring Automation `340050d2c911445da3634ebb92acc014` 8/8 PASS를 기술 evidence로 기록했다.
- Runtime `UCFVehicleData`, Inventory/Fitting 소비 경로, `SCFVDAWizardTab`, Content Asset은 변경하지 않았고 Asset Snapshot Reader / Resolver / Apply / UI / CSV는 아직 구현하지 않았다.
- 다음 Gate는 `DAUTH-P0-08D Asset Snapshot Reader Foundation`이다.

### v0.2.0 - 2026-08-17

- `CF-FQ-038 차량 데이터 Authoring 시스템`의 대표 Active Plan으로 정식 승격했다.
- P0-01~07 설계 Gate 완료 상태를 구현 입력으로 고정하고 `DAUTH-P0-08 Implementation Foundation`에 실제 Source 구현 착수했다.
- `P0-08A`에서 `CarFight_ReEditor`에 Editor-only `UCFVehicleRecipeData` + 5 flat Profile `UDataAsset`, Never-Cook `IsEditorOnly()`, Authoring common types와 Recipe duplicate identity를 구현했다.
- `P0-08B`에서 Stable Field Path, Reflection Field Value Codec, current `UCFVehicleData` 117 leaf Registry와 bidirectional coverage를 구현했다.
- official Build `5c745a31d19b448d9b1049877bc877ca` PASS와 targeted Automation process `ab9de3e8c8cd410a8de0641178690dff` 4/4 PASS를 현재 기술 evidence로 기록했다.
- `Tools/RunDataAuthoringTests.ps1`를 Data Authoring targeted Automation 재실행 진입점으로 추가했다.
- Runtime `UCFVehicleData`, Inventory/Fitting 소비 경로, `SCFVDAWizardTab`, Content Asset은 변경하지 않았다.
- 다음 Gate는 `DAUTH-P0-08C Immutable Snapshot Foundation`이다.

### v0.1.0 - 2026-08-17

- CarFight Data Authoring 신규 대표 Plan 초안 작성.
- Vehicle 우선, 이후 Weapon / Equipment / Scanner / Armor 확장 원칙을 정의.
- Authoring / Definition / Inventory Instance / Fitting State / Runtime 책임 경계를 분리.
- AI Authoring을 정식 Authoring Client로 정의하고 Raw Field 임의 작성 금지 원칙을 추가.
- Excel / CSV를 보조 Batch Interface로 제한하고 Unreal Definition을 초기 Canonical Result로 유지.
- 기존 Vehicle DA Wizard를 삭제하지 않고 Migration Source로 사용하는 방침을 고정.
- DAUTH-P0-00~12 Gate 초안을 정의하고 P0-02 전 구현 금지 범위를 명시.

---

## 18. Migration

- 기존 `CF_DAFillToolPlan.md`, `CF_DAFillWizardUX.md`, `CF_DrivingFeelTunePlan.md`는 당장 삭제하거나 덮어쓰지 않는다.
- 기존 Vehicle DA Wizard는 새 Authoring Tool의 기능 동등성 및 회귀 검증 전까지 유지한다.
- 기존 VehicleData / Inventory / Fitting Runtime 계약은 P0-00~02 설계 감사 동안 변경하지 않는다.
- 현재 이 문서는 `CF-FQ-038`의 대표 Plan이다. `DAUTH-P0-08 Implementation Foundation` P0-08A~M, `DAUTH-P0-09 Vehicle Authoring MVP`, `DAUTH-P0-10 Existing Wizard Migration`, `DAUTH-P0-11 Technical Validation`은 Technical PASS다. Frozen 24.90~24.94 normal Workspace completeness도 closure PASS다. 전역 우선순위상 Feature는 Paused지만 bounded `DAUTH-P0-12 USER Authoring Acceptance`는 사용자 직접 판정으로 이어갈 수 있으며 현재 USER PASS는 1, 다음 Gate는 UA-02다. 기존 Wizard는 P0-12와 DG/DEL Gate가 완료될 때까지 유지한다. Frozen 26.66~26.69 Batch idempotency/generic envelope는 첫 실제 external Batch transport/client integration 전 follow-up이다. Current feature 상태는 main_game의 ProjectSSOT / ActiveWork가 소유하고 이 Plan은 상세 Gate와 검증 checkpoint를 소유한다.
