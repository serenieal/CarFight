# CarFight Data Authoring Roadmap

- 문서 버전: v0.1.57
- 작성일: 2026-08-17
- 문서 상태: Done / Historical + Retained Path / DEL1~DEL7 PASS / UA-08 Quantitative Comparison Deferred

- 상위 계획: `Document/Plan/DataAuthoring/DataAuthoringPlan.md`
- 상세 설계: `Document/Plan/DataAuthoring/DataAuthoringDesign.md`
- 역할: Data Authoring의 단계별 작업 순서, Gate, 완료 조건, 금지 범위를 관리한다.

---

## 2026-09-11 Final Closure

`CF-FQ-038 Vehicle Data Authoring` Roadmap은 현재 범위를 완료했다. 현재 구현 owner는 main_game `Document/Systems/Vehicles/VehicleBuilder.md v1.5.0`이고 이 Roadmap은 완료 당시 Gate/evidence를 보존하는 Historical + Retained Path로 전환한다.

```text
P0-08A~M / P0-09 / P0-10 / P0-11: Technical PASS
P0-12: Closed with Deferred Feel Comparison / Historical USER PASS 7 of 8
UA-08: Runtime Technical PASS / USER Inconclusive / quantitative comparison Deferred
DG1~DG5: PASS
DEL1~DEL7: PASS
DEL6: required direct caller exact0 + Legacy Wizard physical retirement complete
Final Official UE 5.8 Build: 1fd947042024420aaa7561d380e4548f PASS
Full CarFight.DataAuthoring: 111/111 PASS / failure0 / Engine Exit0
Current owner: main_game Systems/Vehicles/VehicleBuilder.md v1.5.0
Lifecycle: Done -> Historical + Retained Path / G0~G4 PASS / G5 Deferred
```

UA-08 정량 비교는 별도 비차단 observational debt이며 이 Roadmap의 현재 next gate가 아니다. 새 관련 failure evidence나 별도 Feature 승격이 없는 한 완료된 DAUTH-P0/DG/DEL Gate를 replay하지 않는다.

---

## 2026-08-18 Pause Checkpoint — Historical

현재 프로젝트 단일 Active는 UI `CF-FQ-032`로 전환됐다. DAUTH는 아래 상태를 그대로 보존한다.

```text
DAUTH-P0-08A~M: Technical PASS
DAUTH-P0-09: Technical PASS
DAUTH-P0-10: Technical PASS
DAUTH-P0-11: Technical PASS
Historical P0-08 schema evidence: 117 Registry / Performance numeric 17 / five-Profile numeric 78
Current additive compatibility: RedlineStartRPM 포함 118 Registry / Performance numeric 18 / five-Profile numeric 79
Bounded compatibility: Registry dacb88cb09644d0287cfe7a7f3137b51 / Batch projection 03aad228b1784f1b908ed6e82ddced3e / Allowlist c9d07c42428642c186743e8c26043dcb — 각 1/1 PASS
DAUTH-P0-12: Closed with Deferred Feel Comparison / USER PASS 7 of 8 / UA-08 Runtime Technical PASS / USER Inconclusive
재개 체크포인트: DG/DEL Gate — UA-08 transient Sports 4축→Runtime Movement 13 field Technical PASS, persistent mutation0. USER 체감은 Inconclusive라 정량 비교 Deferred, 다음 단계 진행 승인 / P0-12 USER PASS 7 of 8 유지
UA-06 USER fixture correction: 기존 `FrontWheelMaxBrakeTorque 2000→2100` 후보는 Source Trace에서 `Legacy Imported Pinned Baseline — Recipe.ImportState.LegacyPins`가 effective source라 Shared Profile review의 예상 VehicleData 변경이 0으로 확인되어 USER가 승인 전 취소 / mutation0. 대체 `/Game/Test/CarFightDataAuthoring/DA_UA06_Performance`는 현재 `DA_TestSedan` Performance baseline과 동일 결과를 내도록 생성 / `RedlineStartRPM`은 Legacy Pin 없음·PerformanceProfileDirect owner / official fixture build `4a07fa4dc9674746ba56b3a2b14457a1` Exit 0 / headless fixture `9a5f6b110d534fbcb23167576157a1c8` Exit 0 / formatting-only cleanup 뒤 final exact-source official build `7601a83504bc476ab789d7b9ddd7baef` Exit 0 PASS / persisted dataset `adset_v1_82ec6901a50bcb39c1e2a603df40afe4.5cd21309982cced85d66af9e` PASS / Production Recipe dataset `adset_v1_bc9499346bf725234107edc0f6109209.eaed116e9a498d04ffc2fd0c`에서 Performance binding 없음 유지 / Editor0
UA-07 Remote Technical Readiness: PASS / USER 순서 변경 0 / test-only Handling·Performance fixture persisted / Production Recipe UA07 binding 0 / explicit R1 Profile Unbind baseline cleanup PASS / final official build `627107d9de2d49be9be552c0baf2875a` Exit 0 / focused `ProfileUnbind` `e999340c380b4e18af218b57c81dcab3` 1/1 PASS
Deprecated Transition: Technical Complete / DG1~DG5 PASS / Legacy Wizard 기본 Window 메뉴 숨김 Runtime 확인 PASS
Official validation: shared full Official Editor Build PASS 상태 보존 / fresh Editor updated module load PASS / focused TabRegistration 1/1 PASS
Delete Gate: DEL6 required-caller0 Pending / physical Wizard deletion 금지 유지
```

Historical note: 위 2026-08-18~24 checkpoint 당시에는 DEL6 compatibility retirement와 UA-08 quantitative comparison이 남아 있었다. 2026-09-11 Final Closure에서 DEL6는 완료됐고, 현재 남은 것은 Feature를 재개하지 않는 UA-08 비차단 observational debt뿐이다. 기존 UA-01~08 USER/runtime flow와 완료된 P0/DG/DEL validation은 새 failure evidence 없이 replay하지 않는다.

### 2026-08-19 UA-03 transient inventory correction checkpoint

```text
Source correction applied
- CFBatchImport.cpp v1.1.2
- GatherAffectedRecipes live fallback에서 RF_Transient 또는 GetTransientPackage() Recipe 제외
- non-transient unsaved Recipe inventory 유지

Regression added
- CFVehicleAuthoringVMTests.cpp v1.7.0
- invalid /Engine/Transient scratch가 Handling을 참조해도 SharedProfileImpact preview/commit persistent route가 통과해야 함

Closure validation
- initial official build job `26a64c03b02843d798676b60c9ace1af`: 수정 TU compile PASS / 당시 user-owned Editor DLL lock으로 link만 FAIL
- final official build `3fa7b4d4cca14d37b7fea50631e4fa07`: Exit 0 PASS / ReEditor DLL 정상 link
- focused SharedProfileImpact process `db57ea0701234e65b8e5921b1dce1e96`: 1/1 PASS / 0 FAIL / Engine Exit 0
- fresh Editor `d0e658ca67504b0d889b4c4f54a3ebf1`: Ready / port 8100 owner / protocol ready PASS
- persisted Recipe binding: Handling=`DA_Profile_UA_TestHandling`, DriveState=`DA_Profile_UA_TestDriveState`, DriveStateMode=VehicleSpecific
- USER B2 one-shot: FrontWheelMaxBrakeTorque=2000 / affected Vehicle 1 / expected VehicleData change 0 / Auto Apply 0 / Auto Save 0 / actual commit PASS
- live dirty boundary: Handling Profile=true / Recipe=false / VehicleData=false / DriveState Profile=false
- UA-03 USER PASS / P0-12 USER PASS 3

UA-04 acceptance result
1. single-field baseline Apply / Raw 950 external edit / External Drift detection PASS
2. Last Applied 925 / Current Raw 950 / Current Authoring 925 3-way presentation PASS
3. unreviewed Apply block / Keep Authoring review / final 925 recovery PASS
4. USER: 개별 값과 선택지 의미는 이해 가능
5. USER: 전체 Authoring recovery flow는 거시적으로 복잡함 → USER PASS 보류

UA-04 macro-flow remediation contract — Frozen
1. External Drift를 fresh preview/selection에서 발견하면 read-only 3-way review를 자동 준비하고 우측 `동기화` Context로 안내한다.
2. 첫 화면은 `외부 변경 N건 발견` + `Authoring 기준값 → 현재 원본값`을 우선 표시한다. `Last Applied / Raw / Authoring` 용어는 상세 비교로 내린다.
3. `Authoring 값으로 복구…`를 primary action으로 제공하되 기존 KeepAuthoring prepare/review/commit만 사용한다. 자동 decision 금지.
4. Legacy Pin / Advanced Override는 secondary `다른 처리 방법` 영역으로 내린다. Advanced Override reason 필수 계약은 유지한다.
5. Keep Authoring 승인 뒤 같은 Sync panel에 `Authoring 값으로 복구 적용…`을 노출하고 기존 R3 Apply lane을 호출한다. source decision과 target Apply의 2단계 승인은 유지한다.
6. fingerprint/hash/stale/TOCTOU/fail-closed/no-auto-save/no-auto-retry 계약은 변경하지 않는다. 기술 식별값은 기본 USER surface에서만 후순위로 내린다.
7. VM/Core/Resolver/ApplyService/DataAsset schema 변경 0. UI-only scope: `CFVehicleP11UI.cpp`, `CFVehicleAuthoringTab.cpp/.h`.

Implementation / validation closure
1. `CFVehicleP11UI.cpp v1.5.0`, `CFVehicleAuthoringTab.cpp v1.7.0`, `CFVehicleAuthoringTab.h v1.5.0` — UA-04 macro-flow + edge hardening / stale cache presentation guard / terminal failure feedback / 사용자-facing `제작 기준값·차량 데이터·상세 비교` 현지화
2. `CFVehicleAuthoringVM.cpp/.h v1.5.0` — custom Undo exact UE TransactionId top binding / intervening Editor transaction fail-closed
3. `CFVehicleApplyService.cpp v1.2.0` — Definition Apply explicit CarFight transaction context + Target PrimaryObject metadata / Writer 의미 변경 0
4. `CFVehicleAuthoringVMTests.cpp v1.8.0` — intervening transaction regression
5. `CFVehicleUXOps.cpp v1.2.0` — Keep decision 사용자-facing 성공 상태 현지화 / contract mutation 0
6. Runtime UCFVehicleData / Resolver / Inventory / Fitting / DataAsset schema mutation 0
7. readiness exact-source official build `84d2a7d60f7748aa8699c3c7ab4eb512`: Exit 0 PASS
8. localization exact-source official build `7364697cfcc542f090aa5a713fec83a1`: Exit 0 PASS / wording+display-only이므로 focused readiness replay 0
9. UA-06 ApplyUndo `87355a4629b448a09967c28dc0bdab9b`: 1/1 PASS
10. UA-05 ViewModelCoreParity `fbcfc455ac9442eea8ce7e5d3a1869cc`: 1/1 PASS
11. UA-04 DriftRecovery `688a0ce125c5439a8fee214bf76c2cce`: 1/1 PASS
12. source-side Undo `LayoutDrivingUndo` `6907ed3a18514afeb0d36efdc55411d7` + `AdoptionMeasurement` `4b53750254f148e29f50e915acd2a428`: 각각 1/1 PASS
13. UA-04 USER recheck: macro-flow는 이전보다 보기 편하다고 USER 판정 / Keep→final Apply→925 복구 확인 / 현지화 실제 화면 확인 PASS
14. UA-04 USER PASS
15. UA-05 USER PASS / P0-12 USER PASS 5 / 문제 formatting + Source Trace search/effective-first/candidate-toggle 실제 USER 승인
16. UA-07 Remote Technical Readiness 선행 PASS — `CFVehicleAuthoringP10.cpp v1.3.0` Recipe→Diff→Target Apply 설명/failure feedback, Resolver semantic mutation 0
17. `/Game/Test/CarFightDataAuthoring/DA_UA07_Handling`, `DA_UA07_Performance` deterministic test-only fixture 생성 process `30ef23a831184ccba508d45d83ac414d` Exit 0 / persisted dataset `adset_v1_6bc502ac5c5d6be5d91889362f16e2ef.f0989769d1be4e211985389f` exact 2 assets PASS / Production Recipe isolation dataset `adset_v1_34f9089552941e0c7b6df12103a3a65a.58b95c12b8ee87124e4d5496` UA07 ref 0
18. UA-07 baseline cleanup을 위해 `UnbindVehicleProfile` explicit R1 Recipe-only operation + reviewed UI 추가. Bind non-empty path 계약 유지 / Target·Profile Asset·Save mutation 0
19. final official build `627107d9de2d49be9be552c0baf2875a` Exit 0 / focused `CarFight.DataAuthoring.DAUTH_P0_12.Workspace.ProfileUnbind` process `e999340c380b4e18af218b57c81dcab3` exact 1 test Result={Success}. 최초 runner `9ac8e787f8e94098a5dfb69bc030eada`는 quoting 오류로 test result 전 종료하여 PASS 집계 0

UA-07 USER Acceptance result
1. 4축 local slider draft는 VehicleData/Recipe mutation 없이 pending diff0을 유지했고 `4축 주행감 저장`에서만 Recipe Intent를 commit하는 흐름을 USER가 이해 가능하다고 판정했다.
2. `DA_TestSedan` Legacy Pin ownership을 확인했다. Handling 관리 전환 review=Legacy Pin22/prospective diff10, Performance 관리 전환 review=Legacy Pin9/prospective diff3/AutoSave0. VehicleData Apply 없이 각각 Workspace Undo로 원복했다.
3. Performance Adoption 상태에서 Acceleration 0.75가 EngineMaxRPM 6500→6737.000977 / EngineMaxTorque 925→947.400208 / ThrottleInputScale 0.600000→1.098267의 fresh diff3으로 Resolve됐다. warning0/block0/external drift0.
4. Sports preset 1회로 0.85/0.78/0.82/0.78 exact 4축 Recipe Intent가 적용되고 VehicleData auto-apply0인 것을 확인했다. USER preset/slider UX verdict=PASS.
5. cleanup은 Performance Adoption Undo, 4축 0.50/0.50/0.50/0.50 복구, temporary Profile cleanup, Handling `DA_Profile_UA_TestHandling` 복구 순서로 수행했다. final pending diff0 / warning0 / block0 / external drift0 / VehicleData Apply0.
6. UA-07 USER PASS / P0-12 USER PASS 7.

UA-08 Runtime Comparison closure
1. baseline PIE는 TestMap / BP_CFVehiclePawn / persistent DA_TestSedan이며 RuntimeRead baseline MaxTorque925 / MaxRPM6500 / SteeringRatio0.675를 확인했다.
2. test-only CFDAUA08Runtime v1.0.0은 transient Recipe/VehicleData에 Sports 0.85/0.78/0.82/0.78을 shared Resolver로 계산하고 Handling Legacy Pin22 + Performance Pin9를 transient ownership 전환했다.
3. Driving Feel Movement 13 field exact coverage를 PIE Pawn에 적용했고 RuntimeRead에서 transient VehicleData와 MaxTorque1010 / MaxRPM7050 / SteeringRatio0.812를 확인했다. persistent Recipe/Target dirty0, Target Apply0, Save0.
4. USER는 주행 체감 방향을 확신하기 어렵다고 판정했다. USER PASS는 부여하지 않고 정량/계측 비교를 Deferred한다. P0-12 USER PASS 7 of 8 유지.
5. USER가 다음 단계 진행을 승인했으므로 P0-12 USER Acceptance cycle은 Closed with Deferred Feel Comparison으로 종료한다.

Deprecated Gate result
1. DG1~DG5 = PASS. P0-09 replacement route, P0-10 parity, managed-target guard + Drift detection, old Wizard 없는 new/existing workflow, migration difference documentation을 current evidence로 충족했다.
2. `CarFightReEditor.cpp/.h v1.3.0`에서 `CarFight Vehicle DA Wizard` Window 메뉴 항목만 제거하고 `CarFight.VehicleDAWizard` hidden spawner/Source는 compatibility로 유지했다.
3. 최초 build `fe6d5a1b41cc4bd1a1c49ff1336a939c` Exit6의 unrelated CF-FQ-039 blocker는 이후 shared full Official Editor Build PASS에서 해소됐다. fresh Editor는 updated `CarFightReEditor` module을 정상 load했다.
4. focused `Workspace.TabRegistration`은 UE Automation log에서 exact 1/1 `Result={Success}` PASS. `Workspace.LegacyManagedGuard`는 이번 change set에서 `SCFVDAWizardTab`/guard logic mutation0이므로 기존 PASS를 보존하고 replay하지 않았다.
5. fresh Editor `창(Window)` Slate Snapshot에서 `CarFight 차량 데이터 제작` 존재 / `CarFight Vehicle DA Wizard` 부재를 확인해 default-entry deprecation runtime PASS를 확보했다.
6. Deprecated transition = Technical Complete.
7. DEL1~5·DEL7은 현재 evidence상 충족 가능. DEL6는 hidden spawner/OpenVDAWizardTab/Legacy guard test direct ref가 남아 required caller0가 아니므로 Pending, physical deletion 금지.

Next exact order
1. CF-FQ-038은 Paused로 유지하고 현재 deprecation cycle은 종료한다.
2. `SCFVDAWizardTab` physical deletion은 별도 compatibility retirement에서 DEL6 required-caller0를 실제로 충족시킨 뒤에만 재개한다.
3. UA-08 quantitative driving comparison은 non-blocking Deferred다.
4. UA-01~08/P0-08~11/DG1~DG5/TabRegistration/menu validation은 새 failure evidence 없이 replay하지 않는다.

Do not
- replay completed P0-08~11 suites without new failure evidence
- weaken Frozen B2 validation taxonomy
- count transient scratch as real affected Recipe
- replay UA-01~03 without new failure evidence
```

### Changelog

- v0.1.57 (2026-09-11): CF-FQ-038을 Done / Historical + Retained Path로 전환했다. DEL6 required direct caller exact0, Legacy Wizard source physical retirement, final Official UE 5.8 Build `1fd947042024420aaa7561d380e4548f` PASS, 전체 `CarFight.DataAuthoring` 111/111 PASS로 DEL1~DEL7을 마감했다. UA-08은 Runtime Technical PASS / USER Inconclusive / quantitative comparison Deferred이며 역사적 USER PASS 7/8을 확대하지 않는다. Current owner는 main_game `Systems/Vehicles/VehicleBuilder.md v1.5.0`; G5 physical move는 Deferred다.
- v0.1.56 (2026-08-24): Legacy Wizard Deprecated transition을 Technical Complete로 닫았다. shared full Official Editor Build PASS 상태에서 fresh Editor updated module load, focused `Workspace.TabRegistration` UE Automation 1/1 Success, `창(Window)` Slate Snapshot의 Current Workspace 존재 / Legacy Wizard 메뉴 부재를 확인했다. `LegacyManagedGuard`는 이번 change set에서 guard Source mutation0이라 기존 PASS를 replay하지 않고 보존했다. hidden Wizard spawner/Source는 compatibility로 유지하고 DEL6 required-caller0 Pending 때문에 physical deletion은 계속 금지한다. UA-08 quantitative comparison은 non-blocking Deferred 유지.
- v0.1.55 (2026-08-24): Deprecated Gate DG1~DG5를 current Source/Automation/USER evidence로 PASS 판정하고 `CarFightReEditor.cpp/.h v1.3.0`에서 Legacy Wizard 기본 Window 메뉴만 제거했다. hidden `CarFight.VehicleDAWizard` spawner/Source는 DEL Gate 전 compatibility로 유지한다. DEL6 required-caller0는 hidden spawner/OpenVDAWizardTab/Legacy guard test direct ref 때문에 Pending이라 physical deletion은 금지다. official build `fe6d5a1b41cc4bd1a1c49ff1336a939c`는 현재 CF-FQ-039 dirty UI Source compile 오류로 Exit6이므로 exact-source closure는 미완료다. 다음은 UI blocker 해소 후 build 1회 + TabRegistration/LegacyManagedGuard focused validation만 수행한다.
- v0.1.54 (2026-08-24): UA-08에서 transient Sports 4축을 shared Resolver로 PIE Runtime Movement 13 field에 실제 적용하고 RuntimeRead로 baseline 925/6500/0.675 → Sports 1010/7050/0.812 전환을 확인했다. persistent Recipe/Target dirty0, Target Apply/Save0을 보존했다. USER 체감은 Inconclusive라 USER PASS를 부여하지 않고 정량 계측 비교를 Deferred했으며 P0-12 USER PASS는 7/8 유지한다. 사용자 승인에 따라 P0-12 cycle을 종료하고 DG/DEL Gate를 개방한다. 다음은 Deprecated Gate DG1~DG5 감사이며 physical Wizard deletion은 DEL1~DEL7 전까지 금지다.
- v0.1.53 (2026-08-24): UA-07 Driving Feel Authoring USER Acceptance를 완료해 P0-12 USER PASS를 7로 전진했다. slider local draft→Recipe commit 분리, Legacy Pin 관리 전환 후 Recipe→Resolver→Diff, Sports preset shortcut을 실제 USER가 PASS했다. Performance Adoption에서 Acceleration 0.75가 EngineMaxRPM/EngineMaxTorque/ThrottleInputScale 3개 diff로 resolve되는 것을 live 확인했고 Target Apply/Auto Save는 0이다. test Adoption과 4축/Profile 상태를 baseline으로 복구해 final diff0을 확인했다. 다음 Gate는 UA-08 Driving Feel Runtime Comparison이다.
- v0.1.52 (2026-08-22): 문서 정상화 Health Check로 P0-12 본문 `현재 상태`에 남아 있던 stale UA-01~05 / UA-06 USER Pending / PASS 5 projection을 최신 UA-01~06 USER PASS / PASS 6 / 다음 UA-07 Driving Feel Authoring으로 동기화했다. UA-03 등 날짜·단계별 당시 evidence는 삭제하지 않고 Historical 맥락을 명확히 유지한다.
- v0.1.51 (2026-08-21): UA-06 Explicit Apply / Undo를 USER PASS로 닫고 P0-12 USER PASS를 6으로 전진했다. 기존 functional cycle에서 Source↔Target 구분과 Workspace Undo를 USER가 PASS했고, remediation 후 Registry 기반 field selector는 `Id선택 편해졌어`, final Apply 상세 review는 `알아보기 편하네 pass`로 실제 USER 재확인 PASS했다. final Apply는 review에서 취소해 Target mutation0을 유지했고, Source current `RedlineStartRPM=0` 확인 후 Performance unbind를 수행했다. fresh live 화면에서 binding none / pending diff0 / warning0 / block0 / external drift 없음 확인 PASS. 다음 Gate는 UA-07 Driving Feel Authoring USER Acceptance다.
- v0.1.50 (2026-08-21): UA-06 functional Apply/Undo cycle은 baseline 복구까지 완료된 상태를 보존하고, USER가 발견한 raw ColumnId 직접입력과 final Apply 변경 상세 미표시 2건을 Technical Remediation했다. Shared Profile editor는 existing `FCFBatchColumnRegistry` 기반 검색/선택 + 표시명/current value/unit UI로 전환했고, final Apply review는 fresh FieldDiff 최대 8건의 field/current/apply-after를 표시한다. official build `685a6c6452db4d738e2b20b726964ce6` Exit0, focused SharedProfileImpact `9573f32f515b4858b1f89669205bb1a0` 1/1 PASS. USER PASS는 5 유지하며 다음은 selector 사용감 + Apply detail 가독성만 짧게 재확인한다.
- v0.1.49 (2026-08-21): UA-06 실제 USER flow에서 기존 Handling 시험 필드가 Legacy Pin에 의해 비적용 후보임을 확인해 승인 전에 취소했다. mutation0을 보존하고, current `DA_TestSedan` Performance baseline과 동일 결과를 내는 `/Game/Test/CarFightDataAuthoring/DA_UA06_Performance` test-only fixture를 추가했다. `RedlineStartRPM`은 Legacy Pin이 없고 PerformanceProfileDirect owner이므로 다음 USER 검사에서 `0→5000` exact single-field Source edit로 사용한다. official build `4a07fa4dc9674746ba56b3a2b14457a1`, headless fixture `9a5f6b110d534fbcb23167576157a1c8`, persisted exact values와 Production Recipe Performance binding null isolation을 PASS했다. USER 요청대로 GUI Editor는 재기동하지 않고 Editor0로 유지한다. USER PASS는 5 유지한다.
- v0.1.48 (2026-08-20): UA-06 USER Pending / USER PASS 5를 유지하면서 UA-07 Driving Feel Remote Technical Readiness를 선행 완료했다. `CFVehicleAuthoringP10.cpp v1.3.0`의 Recipe/Target 분리 설명·failure feedback, `/Game/Test/CarFightDataAuthoring` test-only Handling/Performance fixture persisted evidence와 Production Recipe isolation을 확보했다. USER fixture 연결 후 원래 null Performance binding으로 안전 복구하기 위해 Bind의 non-empty 계약은 유지하고 explicit `UnbindVehicleProfile` R1 Recipe-only operation/UI review를 additive 추가했다. final official build `627107d9de2d49be9be552c0baf2875a` Exit 0, focused `ProfileUnbind` process `e999340c380b4e18af218b57c81dcab3` exact 1/1 PASS다. 다음 실제 USER Gate는 계속 UA-06이다.
- v0.1.47 (2026-08-20): UA-05를 USER PASS로 닫고 P0-12 USER PASS를 5로 전진했다. `문제` 탭 formatting은 USER 재확인 PASS, `값 출처` 123개 full dump는 실제 사용성 피드백에 따라 `CFVehicleAuthoringTab.cpp v1.9.0 / .h v1.6.0`의 검색 + 실제 적용 Source 우선 + 비적용 후보 toggle UX로 교정했다. official build `e036110014c84f93b1a046f9eb065c6b` Exit 0 PASS, USER `torque` 검색 123→4와 후보 toggle 확인 후 승인. 다음은 UA-06 Explicit Apply / Undo USER usability이며 기존 ApplyUndo readiness는 replay하지 않는다.
- v0.1.46 (2026-08-20): UA-05 USER readability에서 `문제` 탭 검증 문장이 과밀해 읽기 어렵다는 피드백을 반영했다. `CFVehicleAuthoringTab.cpp v1.8.0`에서 Validation issue presentation만 의미 단위로 분리하고 문장 사이 줄바꿈을 추가했으며 official build `609d066fe99a455ab5cbb36871bd8e3a`가 Exit 0 PASS했다. 확인용 fresh Editor는 사용자 조작 전 AI-owned save0 stop해 Editor0로 정리했다. 다음은 `문제` 탭 formatting 재확인 후 `값 출처` 123개 나열 UX 판정이다.
- v0.1.45 (2026-08-20): UA-04 현지화 실제 화면을 USER가 확인해 UA-04를 USER PASS로 닫고 P0-12 USER PASS를 4로 승격했다. 다음 USER Gate를 UA-05 Resolve / Diff / Source Trace / Validation readability로 전환했으며 기존 Technical Readiness는 replay하지 않는다.
- v0.1.44 (2026-08-20): UA-04 USER recheck에서 macro-flow가 이전보다 보기 편해졌다는 사용자 판정과 Keep→same-pane Apply→925 복구를 확보했다. 추가 feedback인 `Authoring` 번역을 `제작 기준값/차량 데이터/상세 비교`로 Source에 반영하고 localization build `7364697cfcc542f090aa5a713fec83a1` PASS를 확보했다. 실제 번역 화면 최소 USER 확인 전까지 P0-12 USER PASS는 3을 유지한다.
- v0.1.43 (2026-08-19): UA-04 edge / UA-05 Resolve·Diff·Trace·Validation / UA-06 Apply·Undo Technical Readiness를 bounded 감사해 stale cached UI, drift recovery edge/dead-end, custom Undo unrelated transaction 위험을 교정했다. final exact-source build `84d2a7d60f7748aa8699c3c7ab4eb512` PASS와 ApplyUndo·ViewModelCoreParity·DriftRecovery·LayoutDrivingUndo·AdoptionMeasurement focused 5건 1/1 PASS를 확보했다. USER PASS는 3 유지하고 다음 실제 Gate는 UA-04 saved 925 baseline one-shot recheck다.
- v0.1.42 (2026-08-19): UA-04 macro-flow UI-only remediation 구현, official build `3b0e8518d66440d1a4f06f59f6c75df4` PASS, focused DriftRecovery `e651fc7af4df4cfb8f4e964c82074bc9` 1/1 PASS를 기록했다. fresh Editor는 USER 퇴근으로 recheck 전에 AI-owned save0 종료해 Editor0를 보존했으며 다음 Gate는 saved 925 baseline의 raw 950 one-shot USER UX recheck다.
- v0.1.41 (2026-08-19): UA-04 USER feedback를 반영해 macro-flow UI-only remediation 계약을 Frozen했다. auto read-only review+Sync 안내, 목적 중심 summary, Keep Authoring primary, secondary recovery collapse, 상세 3-way 보존, same-pane final Apply를 채택하고 기존 2단계 승인/fail-closed/no-auto-save/TOCTOU 계약은 유지한다.
- v0.1.40 (2026-08-19): UA-04 External Drift 3-way 기술 경로는 PASS했지만 USER가 전체 복구 흐름을 거시적으로 복잡하다고 판정해 USER PASS는 부여하지 않았다. P0-12 USER PASS 3을 유지하고 다음 Gate를 Frozen safety semantics를 유지한 macro-flow UX remediation으로 전환했다.
- v0.1.39 (2026-08-19): UA-03 B2 Handling Profile exact save를 live dirty=false와 fresh AssetDump persisted 2000/AuthoringRevision1로 확인했다. current Recipe AppliedState가 empty이므로 UA-04는 single-field baseline Apply 후 same-field raw drift를 생성하는 bounded 절차로 진입한다.
- v0.1.38 (2026-08-19): UA-03 transient inventory correction의 final build·focused regression·persisted binding·B2 USER one-shot commit과 Shared Profile-only dirty boundary를 확인해 UA-03을 USER PASS로 닫고 P0-12 USER PASS를 3으로 전진했다. 다음 Gate는 exact Handling Profile 저장 후 UA-04 External Drift 3-way Recovery다.
- v0.1.37 (2026-08-19): UA-03 transient affected Recipe inventory defect, minimal correction, regression, compile/link evidence와 Editor0 handoff 순서를 반영했다.

### Current additive schema compatibility note

`CF-FQ-032 UI-P0-06`의 explicit `RedlineStartRPM`이 `UCFVehicleData`에 additive leaf로 추가돼 Current Authoring compatibility surface가 118 leaf로 확장됐다. 이는 CF-FQ-038 재개가 아니라 외부 Runtime Definition schema 변경에 대한 bounded compatibility maintenance다.

```text
Historical P0-08 baseline
- Registry leaf 117
- Performance Profile numeric 17
- five Profile numeric total 78

Current Source
- Registry leaf 118
- 신규 VehicleMovementConfig.RedlineStartRPM
- Owner Performance / ResolveRule PerformanceProfileDirect
- Performance Profile numeric 18
- five Profile numeric total 79
- 0 = unconfigured
- EngineMaxRPM/변속값 자동 유도 금지
```

기존 P0-08 결과 문단의 117/78 숫자는 당시 schema의 Historical evidence로 유지한다. Current 작업만 118/79를 사용한다.

## 1. 운영 원칙


이 Roadmap은 기존 CF-FQ 작업을 자동으로 중단하거나 Active 상태를 변경하지 않는다.
Data Authoring이 Project Feature로 정식 승격되기 전에는 Working Plan으로 사용한다.

핵심 원칙:

```text
P0-00~02 = 감사 / 분류 / Architecture Freeze
P0-03~07 = 상세 설계
P0-08 이후 = 구현
```

P0-02 전에는 Recipe / Profile / Resolver 신규 구현을 시작하지 않는다.

---

## 2. 전체 단계

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

---

# 3. DAUTH-P0-00 Current Authoring Foundation Audit

## 목적

현재 실제 구현과 문서의 Authoring 관련 책임을 수정 없이 감사한다.

## 대상

```text
UCFVehicleData
UCFVDAValidator
ACFVehiclePawn VehicleData Apply 경로
CarFight_ReEditor
Vehicle DA Wizard
Driving Feel Quick Tune
Layout Capture
VehicleDataTuningPlan
VehicleFittingPlan
InventoryFoundationPlan
기존 DataPlan 문서
대표 VehicleData Assets
```

## 확인 항목

- VehicleData의 현재 실제 필드 목록
- Editor-only / Runtime 소비 필드
- Validator 범위
- VDA Wizard의 기능과 직접 Mutation 경로
- Quick Tune 하드코딩 범위
- Layout Capture의 Asset-derived 범위
- Inventory Definition / Instance 경계
- Fitting Definition / Installed / Derived 경계
- 현재 Mesh-only Vehicle 후보와 Definition 존재 여부

## 변경 허용

```text
Source: 0
Content Asset: 0
Runtime: 0
```

문서 조사 결과만 갱신할 수 있다.

## 완료 조건

- Current 구현과 Historical 문서의 차이를 구분함
- 기존 VDA Wizard 기능 목록이 정확함
- VehicleData → Inventory / Fitting / Runtime 소비 관계를 그릴 수 있음
- P0-01의 Field Matrix 입력 Source가 준비됨

## 2026-08-17 Foundation Audit 결과

상태:

```text
DAUTH-P0-00 = Audit Complete / P0-01 Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

### Current 구현 Authority

P0-01의 필드 책임 판정은 Historical DataPlan이 아니라 다음 Current 구현을 우선한다.

```text
UCFVehicleData 및 관련 Struct
UCFVDAValidator
ACFVehiclePawn VehicleData Apply 경로
UCFVehicleFittingData::BuildFittingSnapshot
UCFVehicleFittingComp
FCFInventoryFitAdapter
Vehicle DA Wizard / Driving Feel Quick Tune / Layout Capture
대표 VehicleData Asset의 실제 저장값
```

현재 `UCFVehicleData`는 초기 DataPlan 범위를 넘어 다음 Definition 데이터를 함께 소유한다.

```text
VehicleVisualConfig
VehicleLayoutConfig
HardpointSlots
MountProfiles
BaseVehicleMassKg
MaximumGrossMassKg
VehicleMovementConfig
WheelVisualConfig
VehicleReferenceConfig
VehicleDurabilityConfig
DefaultDefenseData
DefaultDestroyedFxData
DestroyedFxSocketName
DriveStateConfig
```

### Vehicle DA Wizard / Quick Tune 감사

현재 Wizard에서 보존 가치가 확인된 기능:

```text
Target / Source 선택
Target / Source DA Editor 열기
Compare / Validator 결과 표시
Layout Capture
Driving Feel 4축
Quick Tune Preview
Apply
FScopedTransaction 기반 Undo
Revert
Apply / Capture / Revert 후 Validation
```

Driving Feel 4축은 다음이다.

```text
Acceleration Feel
Steering Agility
Grip Feel
Suspension Firmness
```

현재 Quick Tune은 Editor Slate 구현 내부의 고정 Min/Max `FMath::Lerp`와 Sedan / SUV / Sports / Heavy 프리셋을 사용하며, Apply 시 `VehicleMovementConfig` 일부 Raw Field를 직접 변경한다.

확인된 직접 변경 범위:

```text
bUseMovementOverrides
EngineMaxTorque
EngineMaxRPM
ThrottleInputScale
FrontWheelMaxSteerAngle
SteeringAngleRatio
FrontWheelFrictionForceMultiplier
RearWheelFrictionForceMultiplier
FrontWheelCorneringStiffness
RearWheelCorneringStiffness
FrontWheelSpringRate
RearWheelSpringRate
FrontWheelSpringPreload
RearWheelSpringPreload
```

따라서 Quick Tune은 VehicleMovementConfig 전체의 owner가 아니며, P0-01에서 나머지 Movement 필드를 별도로 분류해야 한다.

현재 Slider 역산은 여러 Raw 값을 정규화한 뒤 평균하는 방식이므로 기존 Definition의 원래 Authoring Intent를 복원하는 근거로 사용하지 않는다.

### Layout Capture 감사

`CaptureLayoutFromChassisSockets()`는 Editor-only Asset-derived 선행 구현으로 확인됐다.

```text
ChassisMesh의 4개 Wheel Socket
→ VehicleLayoutConfig의 SocketName / WheelAnchor Transform

HardpointSlots[].SocketName이 실제 ChassisMesh Socket으로 존재
→ 해당 Hardpoint의 LocalLocation / LocalRotation 갱신
```

이 경로는 `Modify()` / Package Dirty를 사용하는 명시적 Definition Mutation이므로 새 Authoring에서는 Preview와 Apply를 분리하되, Asset Derived 계산 근거 자체는 재사용 후보로 유지한다.

### Validator 감사

`UCFVDAValidator`는 Definition Validation의 Current 중심으로 유지한다.

현재 주요 검증 범위:

```text
Required References
Wheel Sockets
VehicleLayoutConfig
HardpointSlots
Fitting Mass
MountProfiles
VehicleMovementConfig
WheelVisualConfig
DriveStateConfig
VehicleData Compare
```

다음 top-level 필드는 현재 `UCFVDAValidator` 직접 검사 범위에 포함되지 않은 것으로 확인됐다.

```text
VehicleDurabilityConfig
DefaultDefenseData
DefaultDestroyedFxData
DestroyedFxSocketName
```

이 항목은 P0-01에서 기존 Domain Validator / Runtime Fallback에 맡길지, Definition Validation 보강 대상인지 구분해야 한다.

### Runtime 소비 경로 감사

현재 Definition 소비는 단일 경로가 아니다.

```text
UCFVehicleData
├─ ACFVehiclePawn 직접 Apply
│  ├─ Visual / Layout
│  ├─ Movement / Wheel Physics
│  ├─ Wheel Visual / WheelSync
│  ├─ VehicleReference
│  ├─ VehicleDurability
│  ├─ DriveState
│  └─ Destroyed FX
│
├─ VehicleFittingData::BuildFittingSnapshot
│  ├─ HardpointSlots / MountProfiles
│  ├─ BaseVehicleMassKg / MaximumGrossMassKg
│  ├─ Default Equipment / Defense
│  └─ Equipment / Ammo / Defense Mass
│      ↓
│    TotalVehicleMassKg / Resolved Runtime Input
│      ↓
│    Pre-Physics Mass + Weapon / Defense / Sensor / Ammo Runtime
│
└─ Legacy Runtime Fallback
   ├─ VehicleData Default Equipment
   ├─ VehicleData Default Defense
   └─ 기존 Chaos Mass
```

중요한 책임 판정:

- `BaseVehicleMassKg`와 `MaximumGrossMassKg`는 Driving Tune 값이 아니라 Vehicle Definition의 Fitting 기준 / 제약값이다.
- 실제 출격 총중량은 Fitting Derived이며 Authoring이 저장 소유하지 않는다.
- `MountProfiles`의 Compatibility와 Default Equipment는 Definition이 소유하고, 실제 Item 선택 / Installed State는 Inventory / Fitting이 소유한다.
- `DefaultDefenseData`는 Definition 기본값이며 실제 Fitting은 Override / ExplicitNone을 해석할 수 있다.

### Inventory / Fitting 경계 감사

Inventory는 다음을 소유한다.

```text
ItemInstanceId
DefinitionId
Container 위치
접근 권한
Reservation
Instance State
```

Inventory는 다음을 복제 소유하지 않는다.

```text
Vehicle MountType / SizeLimit
Hardpoint 위치 규칙
Vehicle Gross Mass 규칙
Equipment / Defense Combat Stat
Fitting Snapshot 계산
```

현재 Adapter 경로는 다음으로 확인됐다.

```text
Inventory Item Instance
→ Strong Typed Inventory Definition
→ EquipmentPresetData / VehicleDefenseData
→ Transient UCFVehicleFittingData
→ 기존 BuildFittingSnapshot()
```

따라서 새 Data Authoring은 Inventory / Fitting 계산을 다시 구현하지 않는다.

### Override Flag 의미 차이

이름이 비슷해도 현재 런타임 의미가 서로 다르므로 P0-01에서 별도 분류한다.

- `VehicleMovementConfig.bUseMovementOverrides`: 상세 Wheel Runtime Tuning과 `ThrottleInputScale` 적용을 Gate한다. Engine / Drag / Downforce / COM / Differential / Steering 등 일부 값은 이 Flag와 무관하게 적용된다.
- `WheelVisualConfig.bUseWheelVisualOverrides`: 현재 Pawn Runtime 적용을 차단하지 않는다. WheelVisual 값은 그대로 소비되며 Validator의 준비 상태 표식에 가깝다.
- `DriveStateConfig.bUseDriveStateOverrides`: `VehicleDriveComp`로 DriveState 설정 복사를 실제로 Gate한다.

### 대표 Asset / Mesh-only 후보 감사

AssetDump read-only 기준 `/Game` 전체 `CFVehicleData`는 11개다.

구성:

```text
정식 Vehicle Definition 폴더 = 2
Legacy = 1
Ammo / Launcher / Missile / Scanner / Defense 테스트 Fixture = 8
```

정식 Vehicle Definition:

```text
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV
```

두 Definition 모두 현재 `BaseVehicleMassKg=0`, `MaximumGrossMassKg=0`인 Legacy-unset 상태이며, FittingData에 명시적으로 연결하기 전까지 기존 Runtime을 유지하는 Current 계약과 일치한다.

차량 Chassis StaticMesh 후보는 다음 9종을 확인했다.

```text
CityCar
Compact
Coupe
Pickup
Sedan
SubCompact
SUV
Van
Wagon
```

이 중 정식 Definition이 현재 확인된 것은 Sedan / SUV 2종이므로, 다음 7종은 P0-01 이후 Data Browser / 신규 Vehicle Authoring에서 `Unregistered / Visual-only Candidate`로 다룰 수 있는 후보이다.

```text
CityCar
Compact
Coupe
Pickup
SubCompact
Van
Wagon
```

단, 이 판정은 자동 Definition 생성 승인이 아니라 production Definition 존재 여부를 기준으로 한 Authoring 후보 분류다.

### Historical 문서 판정

기존 `DataPlan` 문서는 삭제하지 않고 Historical Design Source로 유지한다.

- `CF_DAFillToolPlan.md`, `CF_DAFillWizardUX.md`: 초기 EUW / 입력 도우미 설계이며 Current C++ Slate Wizard보다 오래된 구조다.
- `CF_DrivingFeelTunePlan.md`: 4축 UX 방향은 유효하지만 현재 Quick Tune의 실제 mapping / 수치와 일부 차이가 있다.
- `CF_DAValidatorPlan.md`: Validator 설계 배경 자료이며 실제 규칙은 현재 `UCFVDAValidator` Source가 Authority다.
- `VehicleDataTuningPlan.md`, Current Source, 실제 Asset 값은 P0-01의 우선 근거로 사용한다.

### P0-01 입력 준비 판정

다음 Matrix를 작성할 수 있을 정도로 Current Owner, 저장 위치, Fitting 영향, Runtime Consumer, Validation Owner의 근거가 확보됐다.

특히 P0-01에서는 다음을 임의로 합치지 않는다.

```text
Asset 선택과 Asset에서 파생되는 Transform
Definition 기본값과 Fitting Derived 결과
Quick Tune이 현재 만지는 필드와 전체 Movement 필드
Editor 준비 상태 Flag와 실제 Runtime Gate Flag
Definition Validation과 USER 주행감 / 시각 Acceptance
Legacy 저장 필드와 Current Authoring Truth
```

---

# 4. DAUTH-P0-01 Vehicle Field Ownership Matrix

## 목적

VehicleData 관련 실제 필드를 하나도 임의로 생략하지 않고 Authoring 책임으로 분류한다.

## 분류

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

## 산출물

`DataAuthoringDesign.md`의 Vehicle Ownership Matrix를 실제 필드 전체로 확장한다.

필수 컬럼:

```text
Field Path
Current Owner
Authoring Classification
Input Source
Default Strategy
Definition Storage
Fitting Impact
Runtime Consumer
Validation Owner
USER Verification Need
Migration Note
```

## 특별 감사 대상

- VehicleVisualConfig
- VehicleLayoutConfig
- HardpointSlots
- MountProfiles
- BaseVehicleMassKg
- MaximumGrossMassKg
- VehicleMovementConfig 전체
- WheelVisualConfig 전체
- VehicleReferenceConfig
- VehicleDurabilityConfig
- DefaultDefenseData
- Destroyed FX 관련
- DriveStateConfig 전체

## 완료 조건

- 모든 필드에 최소 하나의 분류가 있음
- "사람이 직접 입력해야 하는 이유가 없는 Raw Field"가 식별됨
- Fitting Derived를 Authoring이 소유하지 않음
- Runtime-only 상태를 Recipe에 넣지 않음
- 분류 불확실 필드는 미결로 명시됨

## 변경 허용

Source / Asset 변경 없음.

## 2026-08-17 P0-01 결과

```text
DAUTH-P0-01 = Matrix Complete
DataAuthoringDesign.md = v0.2.0에서 126행 Ownership Matrix 완료
Top-level aggregate = 9
Actual leaf field = 117
Total matrix row = 126
Source / Content Asset / Runtime 변경 = 0
```

P0-01은 `VehicleMovementConfig` 47개 Raw Field를 하나의 Quick Tune/Recipe 소유로 묶지 않고 Profile / Derived / Asset Derived / Advanced Override / Definition Stored로 분리했다.
또한 Fitting Derived와 Runtime State를 Recipe에서 제외하고 P0-02 Architecture Freeze에 필요한 12개 결정 입력을 확정했다.

---

# 5. DAUTH-P0-02 Authoring Contract Freeze

## 목적

P0 구현에 들어가기 전에 핵심 Architecture를 잠근다.

## 결정 대상

- Recipe 필요 여부와 저장 형태
- Profile 필요 여부와 최소 Profile 수
- Resolver 책임
- Derived Value 정책
- Asset Derived 정책
- Explicit Override 모델
- Source Tracking 모델
- Stale / Regenerate 정책
- Existing Definition Import 방식
- Definition ID 정책
- Apply Transaction 정책
- Inventory / Fitting 경계

## Freeze 조건

다음 질문에 답할 수 있어야 한다.

```text
새 Vehicle을 만들 때 사용자가 무엇을 입력하는가?
AI가 무엇을 입력하는가?
시스템이 무엇을 계산하는가?
최종 VehicleData에는 무엇이 저장되는가?
왜 특정 값이 그 값인지 어떻게 추적하는가?
Profile이 바뀌면 기존 차량은 어떻게 처리되는가?
Inventory / Fitting은 새 Authoring Layer를 모르고도 계속 동작하는가?
```

## Gate

P0-02 PASS 전에는 다음 금지:

- Recipe 클래스 구현
- Profile 클래스 구현
- Resolver 구현
- 기존 Quick Tune 변경
- VehicleData 구조 변경
- 기존 Wizard 삭제

## 2026-08-17 Contract Freeze 결과

상태:

```text
DAUTH-P0-02 = Contract Freeze Complete / PASS
DAUTH-P0-03 = Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

Frozen Contract 요약:

```text
Authoring Intent SSOT
= Persistent Recipe + 5 Profile Binding + Source Tracking + Explicit Override

Runtime Canonical Result
= 기존 UCFVehicleData 유지

Profile Domain
= Vehicle Base / Drivetrain / Handling / Performance / DriveState 5개
= P0 Profile inheritance 금지

Resolver
= Pure Preview + deterministic resolve + per-field trace + diff + validation
= Target Asset mutation 없음

Source Tracking
= Recipe-side persistent metadata
= VehicleData Runtime field 추가 없음

Existing Definition
= Legacy Pinned Baseline import
= Profile 자동 역산/자동 adoption 금지
= field/group 단위 점진적 Adopt 허용

Profile / Asset 변경
= Stale 표시만 수행
= 자동 Definition mutation 금지

Apply
= Preview → Field Diff → Validate → Single Transaction → Revalidate
= Changed leaf / Stable-ID array diff만 적용
= 자동 Save 금지

Inventory / Fitting
= 기존 UCFVehicleData contract만 계속 소비
= 새 Authoring Layer 의존성 없음
```

Asset Derived P0 범위는 다음처럼 동결했다.

```text
Authoritative
- Wheel Anchor Socket Transform
- 명시 Socket이 있는 Hardpoint Local Transform

Measurement-assisted Preview
- Front/Rear Wheel Radius
- Front/Rear Wheel Width
- Wheel Mesh radius measure mode suggestion

P0 자동 계산 제외
- ChassisHeight
- CenterOfMassOverride
- Front/Rear WheelAdditionalOffset
- DestroyedFxSocketName 임의 자동 선택
```

세 기존 override/readiness flag는 하나의 generic override로 합치지 않는다.

- Layout flag: resolved layout adoption state에 가까운 derived output.
- Movement flag: Wheel detail + Throttle gate 의미를 유지하는 derived output.
- WheelVisual flag: Runtime gate가 아니며 readiness 의미로 유지.
- DriveState flag: 실제 Runtime gate이므로 Project Default / Vehicle-specific semantic mode로 표현.

P0-02 PASS는 구현 착수 승인이 아니다.
`P0-03~07`은 Frozen Contract를 구현 가능한 구조/API/UX로 세분화하며 신규 Source 구현은 `P0-08`부터 시작한다.

---

# 6. DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design

## 목적

P0-02에서 확정한 계약을 구현 가능한 C++ 데이터 구조로 구체화한다.

## 설계 항목

- Recipe struct / UObject 후보
- Vehicle Base / Drivetrain / Handling / Performance / DriveState 5 Profile Domain의 data structure / binding
- Profile inheritance 없는 flat composition
- Resolver Input
- Resolver Output
- Source Trace
- Override representation
- Validation result
- Apply service
- Revision / Stale detection

## 완료 조건

- C++ 클래스 후보와 책임이 겹치지 않음
- UI 없이 Resolver 단독 테스트 가능
- Asset Mutation 없이 Preview 가능
- 동일 입력 Deterministic 결과 가능

## 2026-08-17 P0-03 결과

상태:

```text
DAUTH-P0-03 = Detailed Design Complete
DAUTH-P0-04 = Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

`DataAuthoringDesign.md v0.4.0` Section 22에서 Frozen Contract를 다음 구현 단위로 구체화했다.

```text
Module
- 기존 CarFight_ReEditor 재사용
- CarFight_Re Runtime Authoring dependency 0

Persistent Authoring Asset
- UCFVehicleRecipeData : Editor-only UDataAsset
- Vehicle Base / Drivetrain / Handling / Performance / DriveState 5 UDataAsset Profile
- PrimaryDataAsset / AssetManager 신규 계약 없음

Core
- Structured Stable Field Path
- Reflection Field Value Codec
- 117 leaf Field Registry
- immutable Profile / Asset Snapshot
- Pure Resolver
- Source Trace
- Stale / Shadow Stale / External Drift
- Stable-ID Field Diff
- Import / Adoption / Rebase
- Atomic Apply Service
```

Field Resolver Map은 P0-01의 117 actual leaf를 모두 다음 구현 rule에 배정한다.

```text
Project Compatibility Default
Recipe Semantic / Asset Intent
5 Profile Domain
Feel Derived
Asset Socket Derived
Measurement Proposal + Explicit Adoption
Legacy Pinned Baseline
Legacy Serialized Passthrough
Advanced Leaf Override
Derived Gate / Readiness
```

Current Quick Tune의 4축은 그대로 보존하되 hard-coded UI Lerp를 새 시스템의 source truth로 유지하지 않는다.
Profile이 `Low / Neutral / High` response를 제공하고 Resolver가 deterministic piecewise-linear mapping을 수행하는 구조로 설계했다.
질량 context가 필요한 필드는 Profile이 opt-in `MassScaleRule`을 제공하며 P0-03에서 임의 balance exponent/threshold를 만들지 않았다.

Existing Definition migration은 다음으로 구체화됐다.

```text
Initial Import
→ Current 117 leaf snapshot
→ Legacy Pin
→ lossless semantic candidate copy
→ Preview
→ field/group Adopt
→ Recipe만 변경
→ 별도 Explicit Apply에서 VehicleData 변경
```

Raw Movement → Driving Feel/Profile inverse inference는 계속 금지한다.

Apply는 stale Preview를 막기 위해 다음 precondition을 갖는다.

```text
Expected Recipe Fingerprint
Expected Source Signature
Expected Target Definition Hash
Expected Resolved Definition Hash
Expected Resolver Contract Revision
```

하나라도 달라지면 `PreviewOutOfDate`로 Block하고 automatic retry하지 않는다.

P0-08 구현 검증 입력으로 Field Registry 117 coverage, deterministic resolve, Legacy/Advanced precedence, Stale/Drift, stable-array diff, preview-out-of-date, transaction rollback, import non-mutation, Fitting/Runtime non-regression을 포함한 20개 automation 범주를 고정했다.

---

# 7. DAUTH-P0-04 VDA Wizard Migration Design

## 목적

기존 Vehicle DA Wizard를 제거하기 전에 기능별 Migration 경로를 고정한다.

## 기능별 판정

```text
Target / Source
Compare
Validator
Layout Capture
Driving Feel
Preview
Undo
Revert
Open Raw DA
```

각 항목을 다음 중 하나로 판정한다.

```text
Reuse As-Is
Refactor and Reuse
Replace
Retire After Parity
```

## 완료 조건

- 기존 기능 손실 없이 새 위치가 정의됨
- 기존 Wizard 삭제 시점이 명확함
- Migration 전까지 기존 코드 Freeze 범위가 명확함

## 2026-08-17 P0-04 결과

상태:

```text
DAUTH-P0-04 = Migration Design Complete
DAUTH-P0-05 = Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

`DataAuthoringDesign.md v0.5.0` Section 23에서 Current `SCFVDAWizardTab` 실제 Source를 기준으로 다음 Migration 판정을 고정했다.

| 기능 | 판정 |
|---|---|
| Target selection | Refactor and Reuse |
| Source selection | Refactor and Reuse |
| Object Path direct load | Retire After Parity |
| Current partial-threshold Compare | Replace |
| `UCFVDAValidator` | Reuse As-Is |
| Validator presentation | Refactor and Reuse |
| Layout Capture capability | Refactor and Reuse |
| Layout direct mutation shell | Replace / Retire After Parity |
| Driving Feel 4축 | Refactor and Reuse |
| Sedan/SUV/Sports/Heavy shortcut | Refactor and Reuse |
| hard-coded raw Feel mapping | Replace |
| authoritative reverse inference | Retire After Parity |
| Quick Tune Preview | Replace |
| Quick Tune / Layout Apply | Replace |
| `FScopedTransaction` primitive | Reuse As-Is |
| Quick Tune Revert snapshot | Retire After Parity |
| Raw DA Open | Reuse As-Is |
| Clipboard report copy | Reuse As-Is |

Migration의 핵심 분리는 다음과 같다.

```text
Definition Validation
= transient resolved Candidate
→ UCFVDAValidator::ValidateVehicleData(Candidate, nullptr)

Authoring Diff
= Resolved Result vs Current Target
→ 117-leaf Registry exact diff
→ Apply Authority

Reference Compare
= A snapshot vs B snapshot
→ 117-leaf read-only compare
→ Apply/Validation severity와 분리
```

Layout은 Current socket fallback/4-wheel atomic requirement/optional hardpoint warning 의미를 보존하되 direct DA write를 새 Workspace에서 호출하지 않는다.

```text
Recipe Asset Intent
→ Asset Snapshot Reader
→ Resolver Asset Derived
→ Field Diff
→ Apply Service
```

Driving Feel은 4축과 기존 named shortcut의 semantic input 기능만 보존한다.
Current Wizard의 hard-coded raw Min/Max `FMath::Lerp`, raw Movement → 4축 average inverse, ad-hoc revert snapshot은 신규 Authoring Authority로 이전하지 않는다.

P0-08~10 coexistence:

```text
P0-08 Core 구현 / Old Wizard 유지
P0-09 Target + Preview + 117 Diff + Validator + Apply/Undo + Raw Open parity
P0-10 Reference Compare + Layout + Driving Feel + preset + managed-target guard migration
```

Current Wizard는 P0-04부터 Legacy Current Tool로 Freeze한다.
crash/data-loss/schema compatibility correction 외 신규 Quick Tune/직접 mutation/독자 Authoring 기능을 추가하지 않는다.

Deprecated Gate:

```text
DG1 P0-09 core parity
DG2 P0-10 Compare/Layout/Feel parity
DG3 managed Target Legacy Wizard direct write 차단 + Raw DA/CallInEditor escape-hatch mutation의 확실한 External Drift detection
DG4 Old Wizard 없이 new/existing imported technical workflow 완료 가능
DG5 old/new migration difference automation/documentation 확보
```

실제 `SCFVDAWizardTab` 삭제 Gate:

```text
DEL1 DG1~DG5 PASS
DEL2 P0-11 Technical Validation PASS
DEL3 Inventory/Fitting/Vehicle Runtime non-regression PASS
DEL4 기존 기능 replacement route 전부 존재
DEL5 P0-12 USER Authoring Acceptance 확인
DEL6 필수 direct caller 0 확인
DEL7 Existing VehicleData 자동 migration 불필요
```

Wizard UI 삭제와 `UCFVehicleData::CaptureLayoutFromChassisSockets()` CallInEditor entry 삭제는 별도 Gate로 유지한다.

---

# 8. DAUTH-P0-05 Vehicle Authoring UX Design

## 목적

실제 일상 제작에서 Raw DA Details Panel을 최소화할 수 있는 UX를 설계한다.

## 확정 Main Navigation

```text
Overview
Recipe & Profiles
Assets & Layout
Driving Feel
Mounts & Defaults
Compare
Validation
Advanced
```

`Fitting`은 편집 page가 아니라 Overview의 read-only Preview Context로 둔다.

## 확정 Vehicle Browser 범위

- Search
- Management / Sync / Validation Filter
- Sort
- Validation Filter
- Read-only Multi-asset Reference Compare
- Stale / External Drift
- Override count
- Mesh-only / Unregistered candidate
- P0-07 전 Bulk Apply 금지

## 완료 조건

- 새 Vehicle 생성 흐름을 처음부터 끝까지 설명 가능
- 기존 Vehicle 수정 흐름 설명 가능
- Raw DA Editor가 필요한 경우가 구분됨
- Apply 전 Diff / Validation을 확인할 수 있음

## 2026-08-17 P0-05 결과

상태:

```text
DAUTH-P0-05 = UX Design Complete
DAUTH-P0-06 = Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

`DataAuthoringDesign.md v0.6.0` Section 24에서 P0 Production Authoring UX를 다음 구조로 고정했다.

```text
C++ Slate Workspace

Header
+ Vehicle Browser
+ Main Authoring View
+ Persistent Context Pane
+ Bottom Apply Bar
```

Main navigation:

```text
Overview
Recipe & Profiles
Assets & Layout
Driving Feel
Mounts & Defaults
Compare
Validation
Advanced
```

Persistent Context:

```text
Changes
Source Trace
Issues
Sync
```

상태는 하나의 generic enum으로 합치지 않고:

```text
Management
- Unmanaged
- Legacy Imported
- Partially Managed
- Managed

Sync
- In Sync
- Effective Stale
- Shadow Changed
- External Drift
- Preview Out Of Date

Validation
- Not Evaluated
- Valid
- Warning
- Blocked
```

으로 분리한다.

Vehicle Browser는 Managed/Partial/Legacy/Unmanaged/mesh-only candidate를 한 view에서 검색·필터·정렬하며, multi-select는 P0-07 전까지 read-only Reference Compare에만 사용한다.

New Vehicle workflow:

```text
+ New Vehicle / Mesh Candidate
→ Create Definition + Recipe Records
→ Setup Incomplete 허용
→ Profile / Asset / Semantic Input
→ Resolve Preview
→ Diff / Validation
→ Explicit Apply
→ UE standard Save
```

Create와 Apply를 합치지 않으며 신규 balance default를 P0-05에서 임의 생성하지 않는다.

Existing Definition workflow:

```text
Unmanaged
→ Import Into Authoring
→ Legacy Pin
→ Profile Suggestion/Binding
→ Group Adoption Preview
→ Adopt Recipe only
→ Effective Stale when applicable
→ Diff / Validation
→ Explicit Apply
```

Raw Definition → Driving Feel/Profile ownership의 authoritative inverse는 계속 금지한다.

Assets/Layout UX는 editable asset/socket binding과 read-only Asset Derived 결과를 분리하고 Wheel measurement는:

```text
Measured Proposal
→ Review
→ Use Measured / Confirm Compatibility Default / Advanced Manual Override
→ Recipe adoption metadata
→ Preview
→ Apply
```

로 고정했다.

Driving Feel 4축과 Sedan/SUV/Sports/Heavy semantic shortcut은 유지하되 Target direct mutation이 없고, Legacy Pin에 가려진 intent는 `Shadowed by Legacy Pin`으로 표시한다.

Compare는:

```text
Pending Authoring Changes
= Apply authority exact 117-leaf diff

Reference Vehicle Compare
= read-only A/B compare
```

로 분리한다.

External Drift는:

```text
Last Trusted / Applied
Current Raw
Current Authoring Resolve
```

3-way review를 사용하며 `Keep Authoring / Preserve Raw As Legacy Pin / Promote Raw To Advanced Override / Cancel`을 구분한다.
일반 Apply는 unresolved drift 상태에서 disabled다.

Apply는 fresh preview, target/recipe 존재, Error/Blocked 0을 요구한다.
External Drift가 있으면 일반 Apply는 disabled이며, current Target hash/Recipe fingerprint/Source signature/Resolver revision에 묶인 `Keep Authoring` review decision이 유효한 **Reviewed Apply mode**에서만 동일 Apply Service를 사용할 수 있다.
Warning-only는 count를 표시하고 적용 가능하며 automatic Save는 하지 않는다.

Advanced는:

```text
Advanced Leaf Overrides
Legacy Pin Inspector
Legacy Serialized Passthrough
Resolver Diagnostics
Raw VehicleData Escape Hatch
```

로 제한하고 117 Raw field editable mirror를 만들지 않는다.
Advanced Override는 typed value editor와 Reason required UX를 사용한다.

P0-09 MVP UI는 Target/Import/Overview/Recipe/Resolve/Diff/Source Trace/Definition Validation/Apply/Undo/Raw DA Open을 우선하고, P0-10에서 Reference Compare/Layout/Measurement/Driving Feel/Mounts/Unified Validation/Legacy guard parity를 확장한다.

---

# 9. DAUTH-P0-06 AI Authoring Contract

## 목적

AI가 사람 대신 반복 입력을 수행하되 Raw Field 임의 작성 경로를 만들지 않는다.

## 표준 Workflow

```text
Read Existing Context
→ Build Intent / Recipe
→ Resolve Preview
→ Diff
→ Validate
→ Apply
→ Revalidate
```

## 필수 안전 조건

- 기존 Profile / Definition 조회
- Source Trace
- 임의 Balance Threshold 생성 금지
- USER Driving Feel PASS 추정 금지
- Apply 전 Preview 가능
- Transaction / Undo 가능

## 완료 조건

- UI와 AI가 동일 Resolver 사용
- AI 전용 Raw Mutation 로직 없음
- 사용자 자연어 요청을 Authoring Intent로 매핑 가능

## 2026-08-17 P0-06 결과

상태:

```text
DAUTH-P0-06 = AI Authoring Contract Complete
DAUTH-P0-07 = Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

`DataAuthoringDesign.md v0.7.0` Section 25에서 AI를 별도 Authoring owner가 아니라 공통 C++ Authoring Service의 typed client로 고정했다.

```text
Slate UI ─┐
AI       ─┼→ Common Authoring Service
Automation┘   → Snapshot / Resolver / Diff / Validation
               → Recipe mutation services
               → FCFVehicleApplyService
```

AI 전용 Resolver / Validation / Raw writer는 만들지 않는다.

Operation Risk Class:

```text
R0 Read Only
- Context / Profile / Preview / Diff / Trace / Validation / Compare / Drift read

R1 Authoring Record Write
- Profile Binding
- Driving Feel
- Asset / Mass / Durability / Default / DriveState semantic intent
- Hardpoint / Mount normal semantic edit

R2 Ownership / Exceptional Write
- New Vehicle record creation
- Existing Definition Initial Import
- Adoption
- Measurement decision
- Stable ID rename/remove
- Advanced Override
- External Drift Rebase decision

R3 Definition Apply
- exact resolved diff → FCFVehicleApplyService only
```

Approval은 operation 이름만이 아니라 exact payload/current state에 binding한다.
R3는 Fresh Preview 이후:

```text
Target
Recipe
Resolved Definition Hash
Diff Hash
Target Definition Hash
Source Signature
Resolver Contract Revision
```

이 확정된 뒤에만 reviewed Apply approval을 만들 수 있다.
오래된 approval/preview에 `force`를 적용하는 경로는 없다.

모든 write는 fresh `ReadVehicleContext`와 expected fingerprint를 요구하고, state가 바뀌면 mutation 0 + `StateChanged/PreviewOutOfDate`로 종료한다.

Idempotency:

```text
Desired-state operation 사용
ClientOperationId + Request Hash transient dedupe
같은 desired state는 NoChange
write automatic retry 금지
Apply가 이미 동일 hash이면 second transaction 없이 NoChange
```

AI는 Recipe를 영구 변경하기 전에:

```text
PreviewRecipeChange
= Current Recipe Snapshot
→ typed semantic command transient apply
→ same Resolver
→ prospective Diff / Trace / Validation
→ persistent mutation 0
```

를 사용할 수 있다.

P0 typed operation 범위에는 다음이 포함된다.

```text
List/Read Vehicle
List/Read Profile
Resolve Preview
Pending Diff / Source Trace / Validation
Reference Compare

Bind Profile
Set Driving Feel / named preset
Set Asset Intent
Set Mass / Durability / Default / DriveState Intent
Hardpoint / Mount semantic edit

Create Vehicle Records
Import Existing Definition
Preview/Commit Adoption
Measurement Accept / Compatibility acknowledgement
Advanced Override
External Drift Review / Decision
Apply Resolved Vehicle
```

Raw generic:

```text
SetField(path, value)
Raw UObject property patch
Legacy Wizard direct write
Raw VehicleData editor automation write
```

는 AI contract에 존재하지 않는다.

Advanced Override만 structured Stable Field Path + typed value + Registry allowlist + Reason + R2 approval로 exceptional leaf write를 표현한다.

Shared Profile payload는 P0-06 AI write 범위에서 제외했다.
AI는 existing Profile을 read/recommend/bind할 수 있지만 Shared Profile 자체를 수정하지 않는다.
Cross-vehicle Profile/Batch 영향은 P0-07 이후 별도 근거로 다룬다.

External Drift는 read만으로 해결 처리하지 않는다.
`KeepAuthoring / PreserveRawAsLegacyPin / PromoteRawToAdvancedOverride` exact decision을 current Target/Recipe/Source/Resolver state에 binding하며 Target write는 최종 R3 Apply에서만 수행한다.

AI contract의 금지 범위:

```text
Raw VehicleData write
Recipe internal hash/AppliedState/Pin representation 직접 patch
Derived override flag 직접 write
Measurement 자동 accept
Legacy Pin 자동 해제
External Drift 자동 흡수
Validation skip / force apply
Auto Save
Global blind Undo
AI-only Source Type / Recipe provenance field
USER Driving Feel/Visual/UX PASS 추정
```

모든 R1/R2/R3 mutation은 Editor Transaction으로 undoable하게 남지만 AI가 global Ctrl+Z 성격의 Undo를 호출하지 않는다.
Apply failure는 `FCFVehicleApplyService` atomic rollback을 사용한다.

P0-06 write scope는 single Vehicle / single Recipe다.
AI가 차량 목록을 순회하며 hidden bulk write/apply를 하는 것은 P0-07 Batch Gate 우회로 금지한다.

P0-08 automation 입력으로 AI read/no-mutation, prospective preview, fingerprint/state conflict, duplicate id, import/adopt target unchanged, measurement stale, override deny, drift review freshness, Apply approval mismatch/shared service/no-save/idempotency, UI parity, no bulk write 등 20개 검증 후보를 추가했다.

---

# 10. DAUTH-P0-07 Batch / Excel / CSV Contract

## 목적

대량 비교와 Numeric Balance 작업에서 외부 표를 어디까지 사용할지 결정한다.

## 검토 항목

- Export 단위
- Import 단위
- Column ownership
- Numeric-only 대상
- Asset Reference 제외 정책
- Diff Preview
- Validation
- Conflict / Stale 처리

## 완료 조건

- Excel과 Unreal 중 SSOT가 중복되지 않음
- Import가 Preview 없이 직접 대량 Mutation하지 않음
- Vehicle P0에서 실제 필요한 Numeric Batch 범위가 근거로 확인됨

## 2026-08-17 P0-07 결과

상태:

```text
DAUTH-P0-07 = Batch / Excel / CSV Contract Complete
DAUTH-P0-08 = Ready
Project 상태 = Working / Pre-Implementation 유지
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
```

`DataAuthoringDesign.md v0.8.0` Section 26에서 Spreadsheet를 Authoring SSOT가 아닌 **Export Snapshot + Edit Staging**으로 고정했다.

P0 file contract:

```text
Canonical Import / Export
= UTF-8 CSV + adjacent .cfbatch.json manifest

Excel
= exported CSV를 실제 spreadsheet editor로 사용

Native XLSX parser/writer
= P0 구현 필수 아님
```

Unreal 안의 Recipe/Profile이 계속 Authoring Truth이며 Runtime에서 CSV를 읽거나 live-link하지 않는다.

P0 Dataset Kind:

```text
VehicleSummaryReport   [Read Only]
ResolvedFieldReport    [Read Only]
RecipeNumericEdit      [Editable]
ProfileNumericEdit     [Editable]
```

Writable external scope:

```text
Recipe
- Section 21/22에서 Recipe가 직접 소유하는 scalar numeric semantic input
- 최소 Driving Feel 4축
- 추가 numeric field는 Batch Column Registry allowlist 필요
- Source Mode 변경은 Spreadsheet에서 하지 않음

Shared Profile
- 한 file = 한 Domain
- typed payload scalar numeric leaf만 allowlist
- affected Vehicle 전체 prospective resolve / validation 필수
```

Spreadsheet write 금지:

```text
Resolved VehicleData raw leaf
Asset Reference
Hardpoint / Mount Stable ID / Array 구조
Transform / Vector / Rotator / 복합 identity struct
Asset Derived transform
Measurement acceptance
Advanced Override 생성/수정
Legacy Adoption
External Drift decision
```

ResolvedFieldReport는 **117 leaf pattern 전체 coverage** + Stable Field Path + Current/Resolved/Source/Diff/Validation을 export할 수 있지만 Import는 불가하다. Scalar pattern은 Vehicle당 한 row, array pattern은 Stable-ID element마다 expand되므로 실제 report row 수를 고정 117로 해석하지 않는다.
Raw resolved numeric을 수정하고 싶으면 Source Trace의 owning Recipe/Profile semantic field를 바꾸거나 exceptional case는 기존 Advanced Override workflow를 사용한다.

Column contract:

```text
Stable technical ColumnId
__cf_ reserved metadata
Dataset / Schema revision
row identity
export fingerprint
unit / typed value metadata
```

CSV 숫자는 canonical invariant value를 사용하며 blank는 `No Change`다.
Formula/macro를 editable value로 평가하지 않고 plain numeric만 Import한다.
Read-only/reserved column 수정은 silent ignore하지 않고 Block한다.

CSV 옆 Manifest는:

```text
BatchExportId
Dataset Kind
Schema Id / Revision
Resolver Revision
Export row identities
editable descriptors
baseline canonical values
baseline fingerprints
baseline ownership/source mode
ExportSetHash
```

를 보존하며 Import의 3-way baseline evidence로 사용한다.

Import conflict는:

```text
Export Baseline
Edited Spreadsheet
Current Unreal
```

3-way로 판정한다.

```text
Current == Baseline, Edited != Baseline
→ Safe Candidate

Edited == Baseline, Current changed
→ User did not edit this cell / Current Unreal 유지

Edited == Current
→ NoChange

Current와 Edited가 Baseline에서 서로 다르게 변경
→ ConcurrentEditConflict
```

Export 뒤 Source Mode/ownership이 바뀐 경우 값이 같더라도 `OwnershipChangedSinceExport`로 Block한다.

Spreadsheet Import workflow:

```text
CSV + Manifest
→ Schema / Parse
→ Current Unreal read
→ 3-way merge
→ Prospective Recipe/Profile Patch
→ same Resolver / Diff / Validation
→ Batch Authoring Review
→ B1/B2 Authoring Source Commit
→ Fresh Current Read
→ Fresh Resolve All Affected Vehicles
→ Batch Definition Apply Plan
→ separate B3 Apply Review / Approval
→ per-Vehicle FCFVehicleApplyService
```

즉 CSV Import와 VehicleData Apply는 같은 approval/transaction으로 묶지 않는다.

Batch class:

```text
B0 Read / Export / Preview
B1 Recipe Numeric Authoring Commit
B2 Shared Profile Numeric Commit
B3 Definition Apply Orchestration
```

B1/B2는 source object에 대해 **all-or-nothing Editor Transaction**을 사용하고 Target VehicleData를 수정하지 않는다.
모든 included object precondition을 먼저 검사하며 하나라도 stale이면 mutation 0이다.

B2 Profile Batch는 Shared Source이므로 binding된 dependent Recipe를 모두 inventory하고 effective/shadowed를 분류한 뒤, **effective affected Recipe**를 prospective resolve한다.
Affected scope/분류가 불완전하거나 effective affected Vehicle에서 Error/Blocked가 나오면 Profile commit을 Block한다.
완전히 shadowed되어 effective change가 0인 Vehicle은 usage/audit에는 남기되 unrelated 기존 오류만으로 commit을 막지 않는다. Warning-only는 review 후 허용할 수 있다.

Source commit 후 Definition Apply는 반드시 fresh plan을 다시 만든다.

```text
FCFBatchDefinitionApplyPlan
= Section 25 single-Vehicle R3 evidence collection
```

B3는 첫 mutation 전에 모든 target precondition을 global preflight한다.
Mismatch 하나라도 있으면 전체 시작을 취소하고 Applied=0이다.

실행은 canonical Target asset path 순서이며 각 item은 기존 `FCFVehicleApplyService`를 정확히 한 번 사용한다.

Atomicity:

```text
Per Vehicle Apply = Atomic
Whole Batch Apply = Non-Atomic
```

중간 실패:

```text
Stop On First Failure
Prior Success 자동 rollback 없음
Remaining 자동 continue 없음
Automatic retry 없음

Result
= Applied / Failed / NotStarted exact target list
```

부분 성공 후에는 fresh read → 새 Plan → 새 approval로 실패/미실행 대상만 다시 처리한다.

External Drift는 Recipe/Profile Source Commit 자체와 구분하지만 **Bulk Definition Apply 대상에서는 제외**한다.
P0에는 KeepAuthoringAll/RebaseAll/OverrideAll 같은 bulk Drift 해결 operation을 만들지 않는다.

모든 Batch operation은 Auto Save 0이고 global blind `Undo Entire Batch` operation도 만들지 않는다.

AI Batch는 P0-07부터 `FCFVehicleBatchService` client가 될 수 있으나 Section 25 single-Vehicle write를 자체 loop로 반복하는 hidden batch는 금지한다.
Shared Profile AI 수정도 arbitrary writer가 아니라 `ProfileNumericEdit` Batch model → affected Vehicle preview → B2 approval을 사용한다.

P0-08 automation 입력으로 CSV schema/allowlist/3-way merge/B1-B2 atomic source commit/fresh Apply Plan/global preflight/per-Vehicle ApplyService/partial result/no-auto-save/no-bulk-drift/AI Batch gate 등 30개 검증 후보를 추가했다.

---

# 11. DAUTH-P0-08 Implementation Foundation

## 목적

Architecture Freeze 이후 최소 기반을 구현한다.

## 예상 C++ 범위

- Authoring data types
- Recipe / 5 Profile assets
- Field Registry / Codec
- Resolver
- Diff model
- Source trace
- Stale / Drift
- Validation orchestration
- Import / Adoption / Measurement decisions
- Apply transaction
- Common Authoring Service / AI typed contract
- Batch Column Registry / CSV + Manifest / 3-way merge
- Batch source commit / Definition Apply orchestration
- Automation tests

## 예상 Editor 범위

- `CarFight_ReEditor` 모듈 재사용
- 새 Data Authoring Tab / Workspace

## 보호 조건

- Runtime Module에 Editor 의존성 추가 금지
- 기존 VehicleData Runtime Apply 의미 변경 금지
- 기존 VDA Wizard 삭제 금지

## 2026-08-17 P0-08 구현 체크포인트

상태:

```text
DAUTH-P0-08 = Technical PASS
CF-FQ-038 = Active
P0-08A Schema / Never-Cook Foundation = Technical PASS
P0-08B Stable Field / Codec / 117 Registry Coverage = Technical PASS
P0-08C Immutable Snapshot Foundation = Technical PASS
P0-08D Asset Snapshot Reader Foundation = Technical PASS
P0-08E Pure Resolver Foundation = Technical PASS
P0-08F Definition Materializer / Validation Foundation = Technical PASS
P0-08G Existing Definition Import / Adoption Foundation = Technical PASS
P0-08H Apply Transaction Foundation = Technical PASS
P0-08I Common Authoring Service / AI Typed Contract Foundation = Technical PASS
P0-08J Batch Column Registry / Canonical Export Foundation = Technical PASS
P0-08K Batch Import Session / 3-way Preview Foundation = Technical PASS
P0-08L B1/B2 Batch Authoring Source Commit Foundation = Technical PASS
P0-08M B3 Batch Definition Apply Foundation = Technical PASS
DAUTH-P0-09 Vehicle Authoring MVP = Technical PASS
DAUTH-P0-10 Existing Wizard Migration = Technical PASS
DAUTH-P0-11 Technical Validation = In Progress / Core Technical PASS / Frozen 24.90~24.94 Completeness Blocked
DAUTH-P0-12 USER Authoring Acceptance = Not Ready
```

P0-08A 구현:

```text
CarFight_ReEditor/Public/DataAuthoring
- CFVehicleAuthoringTypes.h
- CFVehicleProfileTypes.h
- CFVehicleRecipeData.h
- CFVehicleBaseProfile.h
- CFDrivetrainProfile.h
- CFHandlingProfile.h
- CFPerformanceProfile.h
- CFDriveStateProfile.h

CarFight_ReEditor/Private/DataAuthoring
- CFVehicleRecipeData.cpp
```

확정 구현 결과:

```text
UCFVehicleRecipeData : Editor-only UDataAsset
5 Profile : Editor-only flat UDataAsset
Profile inheritance : 없음
Recipe duplicate : non-PIE duplicate에서 새 RecipeId 발급
Never-Cook : Recipe/Profile IsEditorOnly()=true
Runtime UCFVehicleData Authoring metadata 추가 : 0
Content Authoring Asset 생성 : 0
```

P0-08B 구현:

```text
CFVehicleFieldCodec.h
- FProperty type signature
- Reflection canonical Export / Import
- type mismatch block
- deterministic value hash 기반

CFVehicleFieldRegistry.h/.cpp
- Frozen 117 leaf pattern descriptor
- Stable-ID wildcard array pattern
- Current UCFVehicleData Reflection leaf discovery
- Registry ↔ Current source bidirectional coverage
- duplicate / missing / removed field detect
```

Current Reflection coverage:

```text
Registry Patterns = 117
Current UCFVehicleData Leaf Patterns = 117
Bidirectional Coverage = PASS
```

P0-08C 구현:

```text
CFVehicleSnapshotTypes.h
- FCFVehicleRecipeSnapshot
- FCFVehicleProfileSource
- FCFVehicleProfileSnapshotSet
- FCFVehicleFieldEntry
- FCFVehicleDefinitionSnapshot

CFVehicleSnapshotBuilder.h/.cpp
- Recipe UObject → immutable-style value-copy Snapshot
- 5 Profile UObject → source metadata + typed payload Snapshot Set
- Current UCFVehicleData → Registry-expanded exact Stable-ID field entries
- UCFVehicleData C++ defaults → 117-pattern Project Compatibility Default Snapshot
- UTF-8 canonical payload + hash format revision 1 + deterministic MD5 fingerprint/hash

CFVehicleFieldRegistry.cpp v1.1.0
- Frozen Section 22.17 Main Dependency → RequiredDependencies stable key 확장
- 117 descriptor dependency metadata non-empty / duplicate-free
```

Fingerprint / hash 경계:

```text
RecipeFingerprint
- Authoring Intent / ownership state만 포함
- AppliedState / AuthoringRevision / diagnostic metadata 제외

ProfileFingerprint
- typed resolver payload만 포함
- DisplayName / Description / diagnostic revision 제외

DefinitionHash
- canonical Stable Field Path + type signature + canonical field value
- Stable-ID array physical order와 독립

Project Compatibility Default Snapshot
- Current Definition과 동일 codec / ordering / hash pipeline
- CDO scalar defaults + default-constructed array element C++ defaults
- Content Asset 또는 CDO array mutation 없음
```

P0-08D 구현:

```text
CFVehicleSnapshotTypes.h v1.1.0
- FCFVehicleSocketSnapshot
- FCFVehicleWheelAssetSnapshot
- FCFVehicleAssetSnapshot

CFVehicleAssetReader.h/.cpp v1.0.0
- Recipe Snapshot의 Chassis/Wheel soft reference read
- Wheel socket None → Current Layout Capture와 같은 Wheel_Anchor_* fallback
- Wheel/Hardpoint requested socket name dedupe + canonical lexical sort
- Chassis socket found/missing fact + local RelativeLocation/Rotation/Scale value copy
- Wheel StaticMesh local Bounds Origin/Extent value copy
- Chassis Layout / Wheel Measure resolver-relevant deterministic fingerprint
```

Reader 책임 경계:

```text
Missing Chassis/Wheel asset path
→ Setup Incomplete fact 허용

non-empty asset path load failure
→ Reader Error

missing requested socket
→ bFound=false fact로 Snapshot에 보존
→ required Wheel 4/4 / optional Hardpoint 판정은 Resolver validation 책임

Wheel measurement
→ Bounds fact/fingerprint까지만 제공
→ Radius/Width Proposal 계산과 Adoption 적용은 Resolver 후속 slice
```

Asset fingerprint는 Section 22.13/22.27대로 whole package hash가 아니라 resolver가 실제 소비하는 추출 사실만 사용한다. Chassis layout fingerprint는 Object Path + requested socket identity/found state/relative transform, Wheel measure fingerprint는 Object Path + local Bounds Origin/Extent를 canonical hash한다.

P0-08D는 기존 `UCFVehicleData::CaptureLayoutFromChassisSockets()`를 수정하거나 호출하지 않았으며 Resolver / Apply / UI / CSV도 구현하지 않았다.

P0-08E 구현:

```text
CFVehicleResolverTypes.h v1.0.0
- FCFVehicleResolveRequest / FCFVehicleResolveResult
- Resolve / Validation / Stage status enum
- Frozen R0~R16 stage record
- ResolvedField / SourceLayer / SourceTrace
- MeasurementProposal
- FieldDiff
- StaleReport

CFVehicleResolver.h/.cpp v1.0.0
- ResolverContractRevision = 1
- Snapshot-only Pure Resolver entry
- deterministic exact-field candidate stack
- stage 실행 순서와 분리된 Frozen source precedence rank
- Project Default → Profile → Rule/Asset → Recipe → Legacy Pin → Legacy Serialized → Advanced
- same-precedence same-field conflict = Error / no last-write-wins
- R0 Request / Recipe / Binding validation
- R1 Project Compatibility Default baseline
- R2 5 Profile typed payload layer
- R3 Recipe semantic layer
- R4 Frozen piecewise-linear Driving Feel + opt-in MassScale
- R5 authoritative Chassis Socket Derived
- R6 Wheel Radius/Width Measurement Proposal
- R7 explicit adoption + matching fingerprint만 AssetDerived effective
- R8 Legacy Pin
- R9 Legacy Serialized Passthrough
- R10 Advanced Override
- R11 Derived Gate
- R12 cross-field / managed wheel geometry validation
- R13 Source Trace / SourceSignature / DefinitionHash
- R14 value-copy FieldDiff foundation
- R15 Definition Materialization = Deferred
- R16 Effective/Shadow/External Drift foundation

CFVehicleFieldRegistry.cpp v1.2.0
- 이미 Frozen인 일반 Project Compatibility Default baseline을 normal descriptor AllowedSourceMask에 공통 투영
- ProjectDefaultThenRecipeSemantic / RecipeBinding / RecipeWheelVisualPolicy / BaseProfileMeasurementPolicy의 Frozen source mask 누락 교정
- hidden legacy serialized field의 전용 passthrough mask는 유지

CFVehicleResolverTests.cpp v1.1.0
- Frozen stage determinism
- precedence / Source Trace
- measurement fingerprint stale block
- legacy serialized passthrough
- R14 Diff + R16 Stale/Drift
- same-precedence fail-closed conflict
```

P0-08E 책임 경계:

```text
Pure Resolver는 Recipe/Profile/Definition/ProjectDefault/Asset Snapshot value만 소비
live UObject / StaticMesh / Slate 재조회 = 0
Fitting Preview Context는 Source winner가 될 수 없음
Measurement Proposal은 adoption 전 effective value를 변경하지 않음
R15 transient UCFVehicleData materialization + UCFVDAValidator = 다음 P0-08F
Apply / UI / CSV / Content mutation = 0
```

Wheel width proposal은 현재 차량 휠 convention의 차축 Y를 기준으로 `2 * abs(BoundsExtent.Y)`를 사용한다. Radius는 current Base/Profile/default `WheelMeshRadiusMeasureMode`의 AutoMaxXZ/AxisX/AxisY/AxisZ를 따른다. Bounds에서 Radius Measure Mode 자체를 자동 추정하는 Frozen rule은 없으므로 `bUseSuggestedRadiusMeasureMode=true`이면 임의 추정하지 않고 Blocked로 닫는다.

검증:

```text
Final Official Build
= b201d87cd13f4987a0907e08c8f00a6c
= PASS / Exit Code 0

Targeted Automation Process
= 3b52a251ab244096b78bb872a06c0069
= CarFight.DataAuthoring.DAUTH_P0_08
= 20 PASS / 0 FAIL
= Result JSON SHA-256 a84a02f81fb8f6cc1e023a6d4304cd7c4a3b854a175be7b19118870930ed5a02

PASS
- Foundation.EditorOnlyAssets
- Foundation.Registry117
- Foundation.StableFieldPath
- Foundation.FieldCodec
- Snapshot.RequiredDependencies
- Snapshot.RecipeFingerprint
- Snapshot.ProfileSet
- Snapshot.DefinitionAndProjectDefaults
- Snapshot.AssetReader
- Resolver.StageDeterminism
- Resolver.PrecedenceTrace
- Resolver.MeasurementFingerprint
- Resolver.LegacySerialized
- Resolver.DiffStaleFoundation
- Resolver.ConflictFailClosed
- Materializer.StableArraysHash
- Materializer.ImportFailClosed
- Import.LosslessPins
- Import.AdoptionPreview
- Import.AdoptionCommit

Reusable Runner
= Tools/RunDataAuthoringTests.ps1
```

첫 Build의 Generated-code link failure는 `CarFight_ReEditor`가 새 Profile reflection에서 사용하는 ChaosVehicle type에 직접 링크하지 않았기 때문이며 `CarFight_ReEditor.Build.cs v1.1.0`의 private `ChaosVehicles` dependency 추가 후 final Build에서 닫았다. Runtime module dependency는 변경하지 않았다.

보호 결과:

```text
Runtime UCFVehicleData Source 변경 = 0
Inventory / Fitting consumer 변경 = 0
SCFVDAWizardTab 변경 = 0
Content Asset 변경 = 0
Auto Save = 0
USER Driving Feel 판정 = 0
```

P0-08F 구현:

```text
CFVehicleMaterializer.h/.cpp v1.0.0
- SortedResolvedFields → RF_Transient UCFVehicleData candidate
- Registry stable collection discovery
- HardpointSlots / MountProfiles exact selector set reconstruction
- transient physical array order = selector lexical canonical order
- identity leaf 필수 / duplicate·missing selector fail-closed
- FCFVehicleFieldCodec::ImportValue checked typed leaf write
- materialized Resolver-owned projection readback
- existing UCFVDAValidator::ValidateVehicleData(Candidate, nullptr)
- Validator report → DefinitionValidation value-copy bridge

CFVehicleSnapshotBuilder.h/.cpp v1.1.0
- BuildDefinitionHashFromFields 공용 entry
- 기존 Definition Snapshot hash format revision 1 재사용

CFVehicleResolverTypes.h v1.1.0
- UCFVDAValidator의 기존 문자열 경로를 보존하는 ValidatorFieldPath
- DefinitionValidation을 R15 actual result slot으로 승격

CFVehicleResolver.cpp v1.1.0
- R15 DefinitionMaterialization = Completed actual stage
- materialization/codec/readback/hash failure → R15 Failed + ResolveStatus Error
- Definition Error/Blocked → R15 Completed + ResolveStatus Blocked
- Resolver ResolvedDefinitionHash를 SnapshotBuilder Definition hash authority와 통합
- materialized readback hash mismatch = fail-closed Error

CFVehicleResolverTests.cpp v1.2.0
- R15 Completed semantics
- DefinitionValidation Blocked와 Resolver internal Error 분리
- Stable-ID array reconstruction/remove semantics
- Resolver-owned readback hash consistency
- FieldCodec import type mismatch fail-closed
```

R15 transient materialization 경계:

```text
candidate outer = GetTransientPackage()
flags = RF_Transient
Content package save/dirty = 0
Stable-ID transient physical order = lexical validation order only
Apply persisted order policy를 정의하거나 변경하지 않음

Mount legacy serialized 10 leaf
- SortedResolvedFields에 LegacySerializedPassthrough로 존재할 때만 import
- 새 Authoring source가 생성하지 않음
- transient struct의 물리 default가 존재하더라도 Resolver-owned readback hash에는 자동 포함하지 않음

Definition Validation
- 기존 UCFVDAValidator source 수정 0
- SourceVehicleData = nullptr
- Validator Error/Blocked는 Preview 계산 failure가 아니라 Apply eligibility Blocked
```

P0-08F에서 기존 `UCFVDAValidator` Runtime source를 수정하지 않았다. Resolver가 기존 Validator를 transient candidate에 호출하는 Editor-only orchestration만 추가했다.

P0-08G 구현:

```text
CFVehicleImportService.h/.cpp v1.0.0
- Current Definition Snapshot hash 재검증 후 Initial Import
- exact Definition field 전체를 Legacy baseline으로 보존
- normal field → LegacyPinnedFields
- Mount hidden serialized descriptor → LegacySerializedFields
- one Mount당 hidden serialized exact 10 leaf 분리
- ManageState = LegacyImported initial state
- ImportedDefinitionHash 보존

Lossless semantic candidate copy
- Chassis/Wheel asset hard reference → same-class Recipe soft reference
- BodyWheelSocket FL/FR/RL/RR
- Base/Gross Mass → ExplicitValue
- MaxHealth → ExplicitValue
- Hardpoint ID / Category / Socket
- Mount active 6 leaf
- Default Defense / Destroyed FX refs + DestroyedFxSocketName
- DriveState bUseDriveStateOverrides → semantic DriveStateMode

금지 유지
- Movement raw → Driving Feel 역산 0
- Movement raw → Handling/Performance Profile 자동 선택 0
- 값 일치 기반 Profile ownership 자동 확정 0
- Asset Measurement adoption 자동 생성 0

Adoption Preview
- Current immutable request를 복제해 selected Legacy Pin만 virtual remove
- Group Preview와 exact Field Preview 분리
- identity / DerivedState / hidden LegacySerialized field-level adoption 차단
- LegacyTechnical / DerivedState normal group adoption 차단
- prospective effective winner가 non-Legacy Source인지 검사
- Preview persistent Recipe mutation = 0

Adoption Commit
- approved Preview의 BaselineRecipeFingerprint fresh precondition
- stale Preview → mutation 0 / fresh Preview 요구
- FScopedTransaction + Recipe.Modify()
- approved exact Legacy Pin만 제거
- group commit만 AdoptedGroups 갱신
- ManageState LegacyImported / PartiallyManaged / Managed 갱신
- AuthoringRevision +1
- Recipe Package Dirty
- Target UCFVehicleData mutation = 0

Movement Derived Gate
- Drivetrain/Handling/Performance/WheelGeometry/TechnicalHandling Pin이 모두 해제된 뒤에만
  VehicleMovementConfig.bUseMovementOverrides DerivedState Pin을 함께 release
- DerivedState를 독립 사용자 Adoption Group으로 노출하지 않음
```

CFVehicleSnapshotBuilder.h/.cpp v1.2.0:

```text
BuildRecipeFingerprintFromSnapshot
- UObject를 다시 읽지 않는 prospective Recipe Snapshot fingerprint
- 기존 Recipe fingerprint canonical authority 재사용
- Adoption Preview/Commit stale precondition에 사용
```

CFVehicleImportTests.cpp v1.0.0:

```text
Import.LosslessPins
- transient UCFVehicleData → Definition Snapshot → Initial Import
- normal Pin + hidden serialized = source exact field 전체 coverage
- one Mount hidden 10 leaf exact partition
- semantic candidate copy
- Driving Feel/Profile non-inference
- full Legacy Resolve hash == imported Definition hash
- normal Legacy Pin / hidden LegacySerialized precedence
- Target Definition hash 불변

Import.AdoptionPreview
- MassDurability group virtual pin release
- current Legacy source → prospective non-Legacy source
- Preview persistent fingerprint/pin/revision 불변
- identity / hidden serialized field adoption rejection
- normal exact field preview

Import.AdoptionCommit
- group Recipe-only commit / AdoptedGroups / PartiallyManaged
- hidden serialized 보존
- prospective fingerprint == committed Recipe semantic fingerprint
- stale preview rejection / revision·pin 불변
- fresh exact field Recipe-only commit
- field commit은 whole group AdoptedGroups를 만들지 않음
- Target UCFVehicleData hash 불변
```

P0-08G Import API에는 Target `UCFVehicleData` write 인자를 두지 않았다. Initial Import와 Adoption Commit은 Recipe UDataAsset만 transaction 대상으로 하며 Definition Apply는 여전히 별도 단계다.

P0-08H 구현:

```text
CFVehicleApplyService.h/.cpp v1.0.0
- FCFVehicleApplyRequest / FCFVehicleApplyResult
- ECFVehicleApplyStatus / stable failure code
- FCFVehicleApplyService = production Target UCFVehicleData 유일 writer

A0~A1 Fresh TOCTOU precondition
- persistent Recipe → fresh Recipe Snapshot
- current Target → fresh full Definition Snapshot
- ExpectedRecipeFingerprint
- ExpectedSourceSignature
- ExpectedTargetDefinitionHash
- ExpectedResolvedDefinitionHash
- ExpectedResolverContractRevision
- approved ResolveStatus / SourceSignature / ResolvedDefinitionHash
- reviewed immutable request에 fresh Recipe/Target만 교체해 Pure Resolver 재실행
- fresh FieldDiff와 reviewed FieldDiff exact equality
- mismatch = mutation 0 / PreviewOutOfDate 또는 ReviewedDiffMismatch

A2~A3 transient preflight
- Current Target DuplicateObject → GetTransientPackage
- dependency-safe exact Apply plan 실행
- Resolver-owned readback projection hash == ExpectedResolvedDefinitionHash
- existing UCFVDAValidator::ValidateVehicleData(candidate, nullptr)
- Error/Blocked = persistent mutation 0

A4~A14 atomic transaction
- FScopedTransaction
- Target->Modify() / Recipe->Modify()
- full Target transient backup + Recipe AppliedState backup + package dirty-state backup
- dependency-safe exact Diff apply
- actual Target Resolver-owned readback hash verification
- actual Target UCFVDAValidator verification
- Recipe FCFVehicleAppliedState field traces 갱신
- Target + Recipe MarkPackageDirty
- PostEditChange
- Save/SavePackage = 0

Dependency-safe Apply plan
- Remove MountProfiles
- Remove HardpointSlots
- Add HardpointSlots
- Add MountProfiles
- Set scalar/nested leaf
- Set array element leaf
- explicit order intent가 없는 MoveArrayElement는 fail-closed

R14 Foundation 보완 경계
- R14의 reviewed SetLeaf/AddArrayElement 의미는 변경하지 않음
- current-only stable selector removal은 ApplyService가 Current Snapshot vs Resolved selector set으로 deterministic synthesize
- Resolver source precedence/hash 의미는 재개방하지 않음

Rollback
- A8~A10 failure 시 Target reflected state를 pre-apply backup에서 복원
- Recipe AppliedState 복원
- transaction Cancel
- 원래 package dirty flags 복원
- rollback 뒤 full Target DefinitionHash == pre-apply hash 검증
- partial success 금지
```

`CFVehicleApplyTestAccess.h` v1.0.0은 Private Automation bridge로만 존재하며 production public API에 failure injection flag를 노출하지 않는다.

`CFVehicleApplyTests.cpp` v1.0.0:

```text
Apply.SuccessAtomic
- current-only old Hardpoint removal
- new Hardpoint add
- new Hardpoint를 참조하는 new Mount add
- synthesized removal을 포함한 dependency-safe actual plan
- resolved readback hash
- AppliedState source/value traces
- Target + Recipe package dirty / explicit save 대기

Apply.Preconditions
- Recipe fingerprint stale
- Source signature stale
- Target Definition hash stale
- Resolved Definition hash stale
- Resolver contract revision stale
- reviewed FieldDiff tamper
- 전부 mutation 0

Apply.Rollback
- A8 actual Target mutation 직후 controlled failure injection
- Target full Definition hash exact restore
- old/new Stable-ID arrays exact restore
- Recipe AppliedState sentinel restore
- Target/Recipe pre-apply dirty flag restore
```

Final official Build:

```text
b88831b71797415fac9dc0a23d12c39e
PASS / Exit Code 0
```

Targeted Data Authoring Automation:

```text
6f261c0b76f04d0faa6b9e855865818e
23 PASS / 0 FAIL
Result JSON SHA-256 382959329a4cefd1e1c884b5c87fc48352f15092b3207f32d63ca4cd806fce9b
```

Source review에서 `SavePackage`, `SavePackage2`, `UPackage::Save` production 호출은 0이고 Target `Modify/MarkPackageDirty/PostEditChange`와 actual mutation은 `CFVehicleApplyService.cpp`에 중앙화되어 있다. `CFVehicleApplyTests.cpp`의 direct Target assignment는 in-memory fixture setup 전용이다.

P0-08I 구현:

```text
CFVehicleAIContract.h v1.0.0
- ECFAuthoringRiskClass R0/R1/R2/R3
- exact ECFAuthoringApprovalClass None/AuthoringWrite/OwnershipWrite/DefinitionApply
- typed operation status/error taxonomy
- FCFAuthoringCallContext / Proposal / MutationFootprint / ValidationSummary / OpResult
- FCFVehicleSemanticChange discriminated command
- Raw Stable Field path/value writer payload 없음

CFVehicleAuthoringService.h v1.0.0
CFVehicleAuthoringService.cpp v1.1.0
- ListVehicles / ListProfiles / ReadProfile / ReadVehicleContext
- ResolveVehiclePreview / ReadPendingDiff / ReadSourceTrace / ReadValidation / ReviewExternalDrift
- PreviewRecipeChange
- CommitRecipeChange
- BuildApplyApprovalProposal
- ApplyResolvedVehicle
- BuildDiffHash
```

Common facade 경계:

```text
Live Recipe / Target
→ existing SnapshotBuilder
→ existing Profile SnapshotBuilder
→ existing AssetReader
→ existing Pure Resolver
→ existing SourceTrace / Diff / Validation / Stale authority
```

새 Resolver, 새 precedence 계산, 새 Validator, 새 Target writer는 만들지 않았다.

R0:

```text
List / Read / Resolve / Diff / Trace / Validation / Drift
persistent mutation = 0
approval = 없음
Save = 0
```

`PreviewRecipeChange`:

```text
Current Recipe Snapshot value-copy
→ typed semantic command
→ existing SnapshotBuilder fingerprint authority
→ existing AssetReader / Pure Resolver
→ prospective Source / Diff / Validation
→ ProposalHash
→ persistent Recipe / Target mutation = 0
```

R1 normal Recipe semantic write:

```text
BindVehicleProfile
SetVehicleArchetype
SetVehicleAssetIntent
SetDrivingFeel
SetMassIntent
SetDurabilityIntent
SetDefaultDataIntent
SetWheelVisualIntent
SetDriveStateMode
UpsertHardpointIntent
UpsertMountIntent
```

- exact `AuthoringWrite` approval만 허용하고 higher `DefinitionApply` approval 재사용도 거부한다.
- `ClientOperationId`, `ApprovalScopeHash`, `ExpectedRecipeFingerprint`, `ExpectedTargetDefinitionHash`, `ExpectedResolverContractRevision`을 fresh server-side Preview와 다시 비교한다.
- Driving Feel partial patch는 포함된 axis만 exact Set하고 omitted axis를 보존한다. 0..1 밖 값은 clamp하지 않고 `InvalidSemanticInput`이다.
- Hardpoint/Mount는 Stable-ID만 사용하고 Mount의 Hardpoint reference dependency를 검사한다.
- persistent commit은 `FScopedTransaction`으로 Recipe만 `Modify → semantic change → AuthoringRevision++ → MarkPackageDirty → PostEditChange`한다.
- Target/Profile mutation 0, Save 0, automatic retry 0.
- internal commit mismatch는 transient Recipe backup으로 자체 rollback + transaction cancel한다.

Editor-lifetime idempotency:

```text
bounded dedupe cache = max 128
same ClientOperationId + same request = terminal result replay / mutation 0
same ClientOperationId + different request = OperationIdConflict
write automatic retry = 0
```

RecipeFingerprint 경계 확인:

- 첫 Automation에서 stale probe가 `VehicleArchetypeId`를 concurrent change로 사용해 26/27이 발생했다.
- Section 22.27 Frozen RecipeFingerprint input set과 current Resolver를 재확인한 결과 `VehicleArchetypeId`는 Recipe schema의 typed semantic field이지만 Resolver fingerprint input에는 의도적으로 포함되지 않는다.
- P0-08C Frozen fingerprint를 임의 확장하지 않았다.
- facade v1.1.0에서 `NoChange`를 fingerprint equality가 아니라 operation-specific typed desired-state equality로 판정하도록 교정해 `SetVehicleArchetype`도 정상 commit한다.
- stale expected-fingerprint guard는 fingerprint-covered `DrivingFeelIntent` concurrent change로 별도 검증했다.

R3:

```text
reviewed FCFVehicleApplyRequest
+ ExpectedDiffHash
→ BuildApplyApprovalProposal
→ exact DefinitionApply approval
→ fresh facade read / evidence check
→ FCFVehicleApplyService::Apply exactly one actual call
```

`FCFVehicleAuthoringService`는 `FCFVehicleApplyService` 외 Target mutation을 수행하지 않는다.

Source surface audit:

```text
SetField(                         = 0
SavePackage                      = 0
SavePackage2                     = 0
UPackage::Save                   = 0
FCFVehicleFieldCodec::ImportValue= 0
Direct Target mutation           = 0
FCFVehicleApplyService::Apply    = actual call exact 1
force / skip-validation API      = 0
Batch / CSV implementation       = 0
```

P0-08I Automation:

```text
Authoring.FacadeCoreParity
- direct Snapshot/AssetReader/Pure Resolver와 facade의 RecipeFingerprint/TargetHash/SourceSignature/ResolvedHash/DiffHash/Trace/Validation/Drift parity
- R0 package dirty / mutation / save / retry = 0

Authoring.PreviewMutationZero
- partial Driving Feel prospective preview
- omitted axis 보존
- persistent Recipe fingerprint/revision/Target hash/package dirty 불변

Authoring.TypedWriteGuard
- higher approval reuse reject
- exact R1 Recipe-only commit
- same operation id dedupe mutation 0
- Frozen resolver RecipeFingerprint 밖의 SetVehicleArchetype도 typed desired-state로 정상 commit
- fingerprint-covered concurrent change에서 stale write block
- Target/save/automatic retry = 0

Authoring.ApplySharedLane
- exact R3 approval + ExpectedDiffHash
- old Hardpoint removal → new Hardpoint add → dependent Mount add
- existing FCFVehicleApplyService lane 실제 재사용
- AppliedState 갱신
- duplicate operation id에서 second Apply mutation 0
- Save/automatic retry = 0
```

Build / Automation evidence:

```text
Foundation facade build d6bd97cb31014e239158f8357ec3a14c PASS
Test-inclusive build acc0aa80ea1a46ae9dce833c1e486914 PASS
Initial automation 7529f6afc86d43828ee5ccbecdc9494a = 26/27, Archetype stale-test assumption RCA 완료
Final build 252fdbd097f943379f2a1e2a942bedef PASS / Exit Code 0
Final automation f2011a206c964fadb269d13c4dba3c86 = 27 PASS / 0 FAIL
Result JSON SHA-256 b9bfea5be21f1bcc521e0bcfa8b8a43c017e18a4dec65a208bfd57a68805bfbf
```

P0-08A~H의 기존 23개 Automation도 final 27/27에서 모두 유지됐다.

P0-08J 구현:

```text
CFBatchTypes.h v1.0.0
- DatasetKind / ValueType / Access / AuthoringOwner / TypedMutationKind / BlankPolicy
- FCFBatchColumnDescriptor
- FCFBatchResolvedProjection
- FCFBatchBaselineCell / ExportRow
- FCFBatchManifest / ManifestRow / ExportArtifact

CFBatchColumnRegistry.h/.cpp v1.0.0
- FCFBatchColumnRegistry
- stable technical ColumnId
- reserved __cf_ metadata
- Recipe numeric projection
- 5 Profile typed payload numeric projection
- existing 117 FCFVehicleFieldRegistry read-only projection
- duplicate/reserved/editability invariant validation

CFBatchExport.h v1.0.0
CFBatchExport.cpp v1.0.1
- Recipe/Profile read-only baseline row projection
- canonical comma CSV + standard quote escaping
- invariant decimal point / finite numeric enforcement
- BOM 없는 UTF-8 bytes
- deterministic row/column ordering
- fixed-order condensed .cfbatch.json
- immutable-style baseline evidence
- deterministic ExportSetHash
- current Registry/schema/hash baseline validation
```

Registry authority 경계:

```text
Resolved report metadata
→ existing FCFVehicleFieldRegistry::GetDescriptors() 117 projection

Recipe numeric edit
→ UCFVehicleRecipeData typed semantic USTRUCT reflection projection

Profile numeric edit
→ 5 typed Profile Data USTRUCT reflection projection

Spreadsheet / CSV / Batch
→ 새 Resolver Source Type 아님
→ 새 Authoring SSOT 아님
```

RecipeNumericEdit writable projection:

```text
DrivingFeelIntent
- AccelerationFeel
- SteeringAgility
- GripFeel
- SuspensionFirmness

MassIntent
- ExplicitBaseMassKg [BaseMassMode == ExplicitValue일 때 edit 가능]
- ExplicitGrossMassKg [GrossMassMode == ExplicitValue일 때 edit 가능]

DurabilityIntent
- ExplicitMaxHealth [MaxHealthMode == ExplicitValue일 때 edit 가능]
```

총 7개 numeric semantic Column이다. `UseProfile` 상태의 Mass/Durability explicit shadow 값은 manifest baseline에는 보존하지만 CSV cell은 blank/unavailable로 export하고 Source Mode를 자동 변경하지 않는다.

ProfileNumericEdit projection은 exact one Domain file 규칙을 유지하고 enum/bool/object/class/array/map/set/identity를 제외한 typed payload scalar numeric leaf만 허용한다.

Automation으로 확정된 current schema numeric count:

```text
VehicleBase  = 8
Drivetrain   = 1
Handling     = 41
Performance  = 17
DriveState   = 11
Total        = 78
```

`FCFFeelResponse`는 Low/Neutral/High numeric leaf로 flatten하고 `FCFMassScaleRule`은 bool Enabled를 제외한 numeric parameter만 projection한다.

Reserved metadata:

```text
Recipe
__cf_row_id
__cf_target
__cf_recipe
__cf_export_recipe_fingerprint
__cf_export_target_hash
__cf_schema_id
__cf_schema_revision

Profile
__cf_row_id
__cf_profile
__cf_profile_domain
__cf_export_profile_fingerprint
__cf_schema_id
__cf_schema_revision
```

`__cf_` prefix는 importer-owned metadata 전용이고 read-only / mutation kind None만 허용한다. editable descriptor가 이 prefix를 사용하거나 duplicate ColumnId가 생기면 fail-closed한다.

Resolved report projection:

```text
FCFVehicleFieldRegistry ExpectedLeafPatternCount = 117
FCFBatchResolvedProjection count = 117
StableFieldPattern identity = existing Registry canonical pattern exact reuse
ValueType / Unit = current UCFVehicleData reflected leaf read-only projection
ResolveRule / AdoptionGroup / PrimaryProfileDomain / identity / legacy serialized = existing Registry descriptor projection
```

Runtime `UCFVehicleData`를 새 Batch registry source로 복제하지 않았고 const Reflection/Snapshot read에만 사용한다.

Canonical export:

```text
one CSV = one DatasetKind
stable ColumnId header
columns = canonical ColumnId lexical order
rows = RowId canonical lexical order
float = invariant decimal '.'
NaN / Infinity = reject
comma/quote/newline = standard CSV quoting
UTF-8 = BOM 없음
editable blank semantics = NoChange
```

`.cfbatch.json` baseline:

```text
BatchExportId
DatasetKind / optional ProfileDomain
SchemaId / SchemaRevision
ResolverContractRevision
CsvColumnIds
editable Column descriptor snapshot
row identities
Recipe/Profile fingerprint
Target Definition hash [Recipe dataset]
baseline canonical values
bEditableAtExport
OwnershipSourceMode
ExportSetHash
```

`ExportSetHash`는 localized DisplayLabel을 제외한 technical schema + rows + baseline values/source mode를 canonical 정렬 후 deterministic hash한다. Manifest baseline tamper 또는 current Registry/schema/revision mismatch는 validation에서 Block한다.

P0-08J source surface audit:

```text
ImportSession                 = 0
CommitRecipe / CommitProfile  = 0
ApplyResolvedVehicle          = 0
FCFVehicleApplyService::Apply = 0
SavePackage / UPackage::Save  = 0
FFileHelper / IFileManager    = 0
MarkPackageDirty / Modify     = 0
FieldCodec ImportValue        = 0
Raw SetField                  = 0
Fitting / SCFVDAWizardTab     = 0
Target UCFVehicleData access  = const read / Snapshot only
```

P0-08J Automation:

```text
Batch.ColumnIdStability
- Recipe stable technical ColumnId repeatability
- exact Recipe numeric 7 columns
- existing 117 Field Registry projection parity

Batch.AllowlistProjection
- Recipe numeric-only allowlist
- Mass/Durability source-mode condition metadata
- 5 Profile domains typed numeric-only projection
- 8/1/41/17/11 = total 78
- enum/bool/asset/array/Stable-ID excluded

Batch.ReservedCollision
- __cf_ metadata read-only invariant
- reserved prefix collision reject
- duplicate ColumnId reject

Batch.CanonicalExport
- input row order independent CSV/JSON/ExportSetHash
- standard quote escaping
- invariant numeric decimal
- BOM-less UTF-8
- UseProfile shadow cell blank
- current manifest validation

Batch.ManifestMutationZero
- Recipe/Profile/Target fingerprint/hash/revision/authored value unchanged
- package dirty 0
- manifest baseline tamper → ExportSetHash mismatch block
```

Build evidence:

```text
814c9a32c1a441fbacfee89fbce5123b
- FAILED / UE 5.8 FNumericProperty::IsUnsigned API mismatch 1건 RCA

0e7c1f31a16b4c708972ebf2606dbe89
- FAILED / test helper TArray::CountByPredicate API mismatch 1건 RCA
- production CFBatchExport compile PASS

Final official Build
3249098c1b99487a8fd173694573bd5e
PASS / Exit Code 0
```

두 failure는 source/contract 의미 문제가 아니라 UE 5.8 API surface 차이였고 각각 concrete unsigned reflected property 판정과 explicit test loop로 교정했다. blind retry는 하지 않았다.

Targeted Data Authoring Automation:

```text
534486583a1d4689bc5137505a03f806
32 PASS / 0 FAIL
Result JSON SHA-256 84fc15de1ad23b4e13891d91db09fa3668a49c900a438dec17b70d818a45d36f
```

P0-08A~I 기존 27개도 final 32/32에서 전부 유지됐다.

P0-08K 구현:

```text
CFBatchImport.h v1.0.0
- ECFBatchCellMergeState
- ECFBatchRowStatus
- ECFBatchIssueSeverity / ECFBatchIssueCode
- FCFBatchParsedCell / ParsedRow
- FCFBatchCellReview
- FCFBatchIssue
- FCFBatchVehiclePreview
- FCFBatchRowPreview / PreviewSummary
- FCFBatchImportRequest / FCFBatchImportSession
- FCFBatchImportService

CFBatchImport.cpp v1.0.3
- companion .cfbatch.json parse + current Registry/schema/ExportSetHash validation
- quoted canonical CSV state-machine parse
- stable ColumnId header set validation / physical column order semantic 0
- duplicate RowId + duplicate Recipe/Profile source identity block
- reserved/read-only modification block
- formula/non-numeric/NaN/Infinity/locale decimal fail-closed
- blank = No Change
- current Unreal Recipe/Profile truth re-read
- cell 3-way classification
- OwnershipChangedSinceExport conflict
- transient Recipe/Profile typed numeric candidate patch
- existing Authoring facade / Pure Resolver prospective preview reuse
- shared Profile affected Recipe/Vehicle fan-out preview
- per-row status/issues / summary
- deterministic BatchPlanHash
```

3-way 구현은 Frozen Section 26.34~26.36을 그대로 따른다.

```text
Current == Baseline && Edited != Baseline
→ SafeCandidate

Edited == Baseline && Current != Baseline
→ NoSpreadsheetChange
→ Current Unreal 유지

Current != Baseline && Edited == Current
→ ConvergedNoChange

Baseline / Edited / Current가 모두 다름
→ ConcurrentEditConflict

Spreadsheet 실제 edit + ownership/source mode changed
→ OwnershipChangedSinceExport
→ Conflict
```

`Recipe/Profile fingerprint changed since export`는 row freshness signal로만 기록하며 그 자체로 Block하지 않는다. import preview는 current Unreal을 다시 projection한 뒤 cell 3-way를 수행한다.

Reserved/read-only 규칙:

```text
__cf_ / read-only cell edited
→ ReadOnlyColumnModified
→ silent ignore 금지

same Recipe/Profile identity duplicate row
→ DuplicateRowIdentity
→ file plan blocked

row/column physical order
→ semantic precedence 0
→ RowId/source identity + stable ColumnId로 canonicalize
```

Numeric parser 규칙:

```text
blank        = NoChange
=1+1         = InvalidNumericValue
comma decimal= InvalidNumericValue
NaN/Infinity = InvalidNumericValue
trailing junk= InvalidNumericValue
'.' decimal + optional scientific notation = 허용
```

Recipe prospective preview:

```text
Current persistent Recipe read
→ transient DuplicateObject
→ SafeCandidate numeric leaf만 Registry typed target로 patch
→ original RecipeId identity 복구
→ existing FCFVehicleAuthoringService::ResolveVehiclePreview
→ current/prospective resolved hash + source signature + Diff + validation
→ persistent mutation 0
```

Profile prospective preview:

```text
Current persistent Profile read
→ transient dynamic-class duplicate
→ SafeCandidate typed numeric leaf patch
→ existing SnapshotBuilder로 one-domain prospective Profile snapshot
→ original shared Profile source identity 유지
→ Authoring facade inventory + live Editor Recipe에서 affected Recipe 수집
→ each current resolve request의 exact Profile Domain snapshot만 prospective payload로 대체
→ existing FCFVehicleResolver::Resolve
→ affected Recipe/Vehicle drill-down
→ persistent mutation 0
```

Row status:

```text
Unchanged
Candidate
ShadowOnly
Warning
Blocked
Conflict
ExternalDrift
```

`BatchPlanHash`는 localized message와 physical row/column order를 제외하고 export baseline identity, current fingerprint/hash, 3-way cell result, prospective resolver evidence, stable issue code를 canonical 정렬해 deterministic hash한다.

Build evidence:

```text
790c54be3c464b6cbd51994c78652d25
- FAILED / UE 5.8에 없는 Misc/LexFromString.h include 1건 RCA

6437ecb4a41443a6abdbea1ec8f28874
- FAILED / UE 5.8 FCString::Strtod API mismatch 1건 RCA

Production core checkpoint
 d6ea14ae4d854b37a465836037567c66
- PASS / CFBatchImport.cpp compile + link

Final official Build
 a3d37d4c8520442d8ceb09a72eb6a68f
- PASS / Exit Code 0
- CFBatchImportTests.cpp compile + link 포함
```

두 compile failure는 계약/3-way logic 변경이 아니라 UE 5.8 API surface 교정이었다. strict integer parser와 invariant decimal grammar + `FCString::Atod`로 교정했고 blind retry는 하지 않았다.

P0-08K Automation:

```text
BatchImport.ThreeWay
- SafeCandidate
- NoSpreadsheetChange
- ConvergedNoChange
- ConcurrentEditConflict
- fingerprint mismatch signal-only
- OwnershipChangedSinceExport

BatchImport.Guards
- manifest ExportSetHash tamper reject
- reserved/read-only edit block
- duplicate RowId/source identity block
- formula numeric reject

BatchImport.OrderDeterminism
- physical row order reverse
- physical column order reverse
- BatchPlanHash exact same

BatchImport.RecipePreviewMutationZero
- SafeCandidate transient Recipe patch
- existing Resolver Vehicle preview
- persistent Recipe/Target value/revision/fingerprint/hash/dirty unchanged

BatchImport.ProfilePreviewMutationZero
- shared Profile transient patch
- affected Recipe/Vehicle preview fan-out
- persistent Profile/Recipe/Target value/revision/fingerprint/hash/dirty unchanged
```

Final targeted Data Authoring Automation:

```text
7d2db5a9baa54dd9abf25857138d3994
37 PASS / 0 FAIL
Engine Exit Code 0
Result JSON SHA-256 bbdf497d472e5ec2fbe4458e075907897ca79b2ac984d43ba3290cff3c55eeec
```

P0-08A~J 기존 32개도 final 37/37에서 모두 유지됐다.

P0-08K production source audit:

```text
CommitRecipe / CommitProfile       = 0
ApplyResolvedVehicle               = 0
FCFVehicleApplyService::Apply      = 0
SavePackage / UPackage::Save       = 0
FFileHelper / IFileManager         = 0
MarkPackageDirty / Modify          = 0
FieldCodec ImportValue / SetField  = 0
Fitting / SCFVDAWizardTab          = 0
Runtime Inventory system call      = 0
```

`inventory` 문자열은 affected Recipe 목록 수집 의미의 Authoring inventory 주석/진단뿐이며 Runtime Inventory subsystem 의존은 없다.

P0-08L 구현:

```text
CFBatchImport.h v1.1.0
- ECFBatchCommitStatus
- ECFBatchCommitErrorCode
- FCFBatchCommitApprovalRow
- FCFBatchCommitApproval
- FCFBatchAuthoringCommitRequest
- FCFBatchAuthoringCommitResult
- BuildRowProposedPatchHash
- BuildCommitApproval
- CommitAuthoringSources

CFBatchImport.cpp v1.1.1
- exact BatchPlanHash / Dataset / Schema / BatchExportId approval binding
- exact Included RowId set / per-row ProposedPatchHash / current fingerprint binding
- old approval partial row-set 축소 재사용 금지
- all included source global TOCTOU preflight before first mutation
- B1 fresh Recipe validation gate
- B2 fresh affected Recipe inventory + prospective validation gate
- one logical FScopedTransaction
- deterministic typed numeric patch
- Recipe/Profile AuthoringRevision +1
- semantic fingerprint + revision postcheck
- full reflected source backup + all-or-nothing rollback
- rollback 후 original fingerprint/revision/dirty-state readback verification
- success source package Dirty
- Auto Save 0
- Target mutation 0
- successful commit 뒤 old Preview/Approval stale + fresh read/resolve required
```

Approval은 current reviewed Session에서 **모든 commit-candidate row**를 exact set으로 binding한다.

```text
Dataset Kind
Profile Domain
Schema Id / Revision
BatchExportId
BatchPlanHash
Conflict / Warning / ExternalDrift / ShadowOnly summary
Included RowId exact set
각 Row ProposedPatchHash
각 Row CurrentFingerprint
```

사용자가 일부 row만 제외하려면 old approval을 줄여 재사용하지 않는다.

```text
Exclude / selection change
→ New Preview / Plan
→ New BatchPlanHash
→ New Approval
```

B1 Global Preflight:

```text
all included Recipe source path/type
current Recipe fingerprint == approval fingerprint
ProposedPatchHash exact
expected post semantic fingerprint calculable
fresh patched Recipe Input/Recipe validation Blocker = 0

하나라도 mismatch
→ transaction 시작 전 중단
→ 모든 included Recipe mutation 0
```

B1에서는 K prospective Resolver의 downstream Definition validation, ShadowOnly, External Drift를 Authoring Source commit blocker로 승격하지 않는다. Frozen 26.47의 source validation layer를 보존한다.

B2 Global Preflight:

```text
current Profile fingerprint == approval fingerprint
affected Recipe exact identity set == approved Preview set
fresh shared Profile prospective preview 재계산
affected Recipe/Vehicle validation Blocker/Error = 0
array structural change = 0

하나라도 mismatch
→ transaction 시작 전 중단
→ Profile mutation 0
```

`External Drift`와 effective stale/shadow-only는 Frozen 26.50대로 B1/B2 source commit blocker가 아니다. Target Definition은 B1/B2에서 수정하지 않는다.

Transaction:

```text
Global Preflight ALL PASS
→ 모든 source full reflected backup
→ one FScopedTransaction
→ 모든 source Modify()
→ canonical source identity + RowId order
→ typed numeric patch
→ AuthoringRevision +1
→ semantic fingerprint + revision postcheck
→ all PASS
→ PostEditChange
→ final fingerprint readback
→ MarkPackageDirty
→ no save
```

중간 실패:

```text
모든 included source를 pre-batch backup으로 restore
→ transaction Cancel
→ PostEditChange
→ pre-batch dirty state restore
→ fingerprint + revision rollback readback

성공 source 일부만 남기는 상태 금지
```

성공 후:

```text
bApprovalConsumed = true
bRequiresFreshPreview = true
bTargetMutationPerformed = false
bSavePerformed = false
```

즉 B1/B2 approval을 B3에 재사용할 수 없다. source fingerprint가 변경됐으므로 old Session/Approval replay는 fail-closed한다.

P0-08L Automation:

```text
BatchCommit.RecipeAtomic
- 2 Recipe / one approval / one transaction
- both typed patch success
- revision +1 each
- source packages Dirty
- Target hash unchanged
- save 0

BatchCommit.StaleAllZero
- approval 뒤 included source 1개 independent edit
- SourceFingerprintMismatch
- transaction 시작 전 whole plan abort
- fresh source mutation 0
- stale external edit 보존

BatchCommit.Rollback
- first source patch 직후 controlled failure injection
- all included source fingerprint/revision/dirty restore
- FailedRolledBack
- partial success 0

BatchCommit.ExternalDrift
- approval 뒤 Target Definition만 external drift
- Recipe source fingerprint fresh
- B1 source commit success
- drifted Target hash 그대로 유지

BatchCommit.ProfileInvalidation
- valid Existing Definition Legacy Pin baseline + shared Handling Profile
- dependent Target external drift와 무관하게 B2 source commit success
- Profile value/fingerprint/revision update
- dependent Recipe/Target mutation 0
- old Session/Approval replay → SourceFingerprintMismatch

BatchCommit.ProfileInventoryStale
- approval 뒤 same Profile을 참조하는 Recipe 추가
- AffectedRecipeInventoryChanged
- Profile mutation 0
```

RCA history:

```text
6717c985ea064becbf58e8463fc37edb
- 43 run / 37 PASS / 6 FAIL
- 원인: K downstream Definition validation을 L source approval blocker로 과잉 승격
- 교정: B1 Recipe/Input layer와 B2 affected-Vehicle layer를 Frozen 26.47/26.49대로 분리

afe51a3d72b1439ea7c2fe63636e6027
- 43 run / 39 PASS / 4 FAIL
- 원인: 성공/rollback fixture가 default Managed Recipe인데 필수 4 Profile 미바인딩
- production gate가 정상 차단한 fixture contract 위반

62e04ff0df7a44fe965fefbb92e04dd6
- focused 5/6 PASS
- 남은 B2 fixture는 current Target만 valid하고 Resolver source baseline이 없어 affected Definition validation Block

0436e7f86e17493aa24c6e4ff8d9bd05
- Existing Definition import Legacy Pin baseline fixture로 교정 후 focused BatchCommit 6/6 PASS
```

RCA 동안 Frozen ownership/precedence/Resolver/Validator 계약은 변경하지 않았다. fixture와 L source-commit validation boundary만 실제 Section 26 계약에 맞췄다.

Final official Build:

```text
2e2923f31d5a4f3fa703f3b7623c0741
CarFight_ReEditor Win64 Development
PASS / Exit Code 0
```

Final targeted Data Authoring Automation:

```text
3853d42dfb344bfd8eb6e516903a8d78
43 PASS / 0 FAIL
Engine Exit Code 0
Result JSON SHA-256 eef2051ec4b3ccb5dce10b8b76761299885d18a8bf7f2c9db42cc2967320abcf
```

P0-08A~K 기존 37개도 final 43/43에서 모두 유지됐다.

P0-08L production source audit:

```text
FCFVehicleApplyService::Apply / ApplyResolvedVehicle = 0
SavePackage / UPackage::Save                       = 0
FFileHelper / IFileManager                         = 0
TargetVehicleData direct write                     = 0
SCFVDAWizardTab / Fitting                          = 0
Runtime Inventory subsystem call                   = 0
```

허용된 source mutation surface는 reviewed B1/B2 transaction의 `Modify()`, 성공 `MarkPackageDirty()`, notification `PostEditChange()`, rollback dirty-state restore뿐이다. `inventory` 문자열은 shared Profile affected Recipe inventory 의미이며 Runtime Inventory 시스템 호출이 아니다.

P0-08M 구현:

```text
CFBatchApply.h v1.0.0
- ECFBatchApplyEligibility
- ECFBatchApplyStatus
- ECFBatchApplyErrorCode
- ECFBatchVehicleApplyState
- FCFBatchDefinitionApplyPlanRequest
- FCFBatchDefinitionApplyItem
- FCFBatchDefinitionApplyPlan
- FCFBatchDefinitionApplyApprovalItem
- FCFBatchDefinitionApplyApproval
- FCFBatchDefinitionApplyRequest
- FCFBatchVehicleApplyResult
- FCFBatchDefinitionApplyResult
- FCFBatchApplyService

CFBatchApply.cpp v1.0.0
- B1/B2 성공 뒤 persistent Recipe/Target fresh read/resolve
- per-target R3 evidence hash
- deterministic BatchApplyPlanHash
- exact ordered eligible Target set approval
- all-eligible global preflight before first Target mutation
- canonical TargetPath ascending sequence
- FCFVehicleApplyService::Apply exact once per attempted Vehicle
- Stop On First Failure
- Applied / Failed / NotStarted / Ineligible result
- per-Vehicle atomic only
- global transaction / global rollback 0
- automatic retry / save 0
```

Fresh R3 evidence:

```text
TargetPath
RecipePath
ExpectedRecipeFingerprint
ExpectedSourceSignature
ExpectedTargetDefinitionHash
ExpectedResolvedDefinitionHash
ExpectedDiffHash
ExpectedResolverContractRevision
ValidationSummary
WarningCount
FieldDiffCount
ArrayStructuralDiffCount
ExternalDrift
```

Eligibility:

```text
Resolve Error      -> Ineligible
Resolve Blocked    -> Ineligible
Validation Blocked -> Ineligible
External Drift     -> Ineligible
Diff 0 + Shadow    -> ShadowOnly / Ineligible
Diff 0             -> NoChange / Ineligible
그 외 fresh Diff>0 -> Eligible
```

External Drift는 B3에서 bulk resolution하지 않는다. 새 per-Vehicle Drift Review 뒤 fresh Plan/Approval이 필요하다.

Approval:

```text
BatchApplyPlanHash
OrderedTargetPaths
PerTarget R3EvidenceHash
TotalVehicleCount
TotalFieldDiffCount
WarningCount
ArrayStructuralDiffCount
AutoSave=false
```

B1/B2 source-commit approval과 B3 approval은 별도 C++ type이다. `CFBatchApplyTests.cpp`의 compile-time type assertion까지 포함해 source approval의 B3 재사용을 금지했다.

Global Preflight:

```text
모든 eligible item을 first mutation 전에 fresh Resolve
→ 여전히 Eligible인지
→ Recipe/Target identity 동일한지
→ R3EvidenceHash exact match인지
전부 검사

하나라도 mismatch
→ Batch Blocked
→ ApplyServiceCallCount=0
→ AppliedVehicleCount=0
```

Execution:

```text
Global Preflight ALL PASS
→ TargetPath ascending
→ Vehicle A: FCFVehicleApplyService::Apply exact once
→ success면 Applied 상태 보존
→ 다음 Vehicle
```

Batch 자체에는 `FScopedTransaction`을 만들지 않는다. 각 Vehicle atomicity/Undo는 기존 `FCFVehicleApplyService`의 transaction 계약이 소유한다.

Failure:

```text
A Applied
B Failed
C 이후 NotStarted

→ Stop On First Failure
→ A 자동 rollback 금지
→ B 이후 auto continue 금지
→ automatic retry 금지
→ global rollback 금지
```

Private Automation access `CFBatchApplyTestAccess.h v1.0.0`은 Global Preflight가 끝난 뒤 특정 Apply 호출 직전 current Target을 바꾸는 controlled hook만 제공한다. Public B3 request/result에는 test flag를 추가하지 않았다.

P0-08M Automation:

```text
BatchApply.PreflightAllZero
- 2 eligible plan/approval 뒤 second Target external edit
- global preflight stale 검출
- ApplyService 0
- Applied 0
- first Target mutation 0

BatchApply.DeterministicOrder
- Recipe input reverse/forward order와 무관하게 same BatchApplyPlanHash
- same ordered target set
- TargetPath ascending
- 2 Vehicle Apply success
- each ApplyServiceCallCount=1

BatchApply.PartialFailure
- 3 eligible global preflight ALL PASS
- first Apply success
- second Apply 직전 controlled stale injection
- second FCFVehicleApplyService가 PreviewOutOfDate fail
- result Applied=1 / Failed=1 / NotStarted=1
- ApplyService total=2
- first success 보존 / third mutation0
- global rollback/retry/save 0

BatchApply.EligibilityApproval
- NoChange / ExternalDrift / ResolveBlocked ineligible
- exact B3 approval에는 eligible Target만 포함
- tampered per-target R3 evidence → mutation0 / ApplyService0
```

Implementation-time compile RCA:

```text
bc66878ac4b1469abfc073f16dbaadc2
- CFBatchApply.cpp initial compile FAIL
- UE TArray<T*> Sort comparator dereference convention에 맞춰 predicate signature만 교정

487fe07650f34aa0a2fc37b35bfb1a5d
- core B3 service Build PASS

da8c7823363747b798fa688629056dbc
- test compile FAIL
- unavailable TIsSame helper를 C++ std::is_same_v로 교체
```

Final official Build:

```text
10027d18360e48f399a5b439f280c24c
CarFight_ReEditor Win64 Development
PASS / Exit Code 0
```

Focused M Automation:

```text
a49d21b6b9b94d648142d5a555a79c95
4 PASS / 0 FAIL
Engine Exit Code 0
```

Final targeted Data Authoring Automation:

```text
cb2ae69e9c834657a553fe52c00f5a96
47 PASS / 0 FAIL
Engine Exit Code 0
Result JSON SHA-256 472ef5bb8f5fdeb46ebd1b7e68f1fc5a66acba065828ff025167b475aaf25595
```

P0-08A~L 기존 43개도 final 47/47에서 모두 유지됐다.

P0-08M production source audit:

```text
FCFVehicleApplyService::Apply actual call = 1 call site
FScopedTransaction in BatchApply         = 0
SavePackage / UPackage::Save             = 0
FFileHelper / IFileManager               = 0
TargetVehicleData direct write           = 0
Modify / MarkPackageDirty                = 0
B1/B2 CommitAuthoringSources call        = 0
SCFVDAWizardTab / Fitting / Inventory    = 0
ApplyResolvedVehicle alternate lane      = 0
```

즉 Target mutation authority는 계속 `FCFVehicleApplyService` 하나이며 Batch layer는 orchestration만 소유한다.

Section 26.66~26.69 dependency 판정:

```text
26.66 BatchOperationId transient dedupe = P0-08M 필수 dependency 아님
26.67 automatic retry 금지               = M에서 이미 준수
26.68 generic Batch error 후보           = M 전용 typed error로 필요한 범위 충족
26.69 generic FCFBatchOpResult 후보       = 26.61 B3 전용 partial result로 필요한 범위 충족
```

현재 external Batch transport/client가 없으므로 `BatchOperationId` dedupe cache를 Foundation에 선행 구현하면 사용되지 않는 operation-lifetime state가 생긴다. Frozen 후보 계약은 폐기하지 않고 **첫 실제 external Batch client/transport integration 전에 재검토할 follow-up**으로 보존한다.

이로써 Section 11 P0-08 예상 C++ 범위의 Authoring Core, Apply, Common Service/AI typed contract, Batch Registry/CSV/3-way merge, B1/B2 source commit, B3 Definition Apply orchestration과 Automation이 모두 구현·검증됐다.

```text
DAUTH-P0-08 Implementation Foundation = Technical PASS
```

다음 Gate는 Section 12의 `DAUTH-P0-09 Vehicle Authoring MVP`다. P0-09에서 실제 single-Vehicle Workspace/Selection/Recipe Intent/Resolver Preview/Trace/Diff/Validator/Apply/Undo/Raw DA Open 흐름을 사용자-facing Editor MVP로 연결한다. Batch UI는 Frozen 26.70대로 P0-09 필수가 아니다.

---

# 12. DAUTH-P0-09 Vehicle Authoring MVP

## 목적

실제 VehicleData 하나를 새 Authoring Flow로 안전하게 읽고 Preview / Apply할 수 있게 한다.

## MVP 후보

- Vehicle Selection
- Recipe / Intent
- Resolver Preview
- Source Trace
- Compare / Diff
- Validator
- Apply / Undo
- Raw DA Open

Driving Feel / Layout Capture의 완전 Migration은 P0-10에서 처리할 수 있다.

## 완료 조건

- 기존 Definition을 수정하지 않는 Preview 가능
- 승인된 Apply 후 Validator 통과
- Undo 가능
- 기존 Runtime 소비 경로 변경 없음

## 2026-08-17 구현 결과

```text
DAUTH-P0-09 Vehicle Authoring MVP = Technical PASS
```

구현 구조:

```text
CFVehicleAuthoringVM.h/.cpp v1.1.0
- transient single-Vehicle ViewModel
- Vehicle Browser / Selection
- facade Resolve / Diff / Trace / Validation projection
- typed Recipe Intent preview/commit
- reviewed Initial Import R2 proposal/commit
- exact R3 Apply preparation/execution
- Raw VehicleData editor navigation

CFVehicleAuthoringTab.h/.cpp v1.0.0
- CarFight.VehicleAuthoring Nomad Tab
- Frozen 3-pane + Bottom Action Bar
- Left Vehicle Browser
- Center 8-page navigation
- Right Changes / Source Trace / Issues / Sync
- persistent Preview freshness / operation status

CarFightReEditor.h/.cpp v1.1.0
- CarFight Vehicle Authoring Window menu entry
- 기존 CarFight.VehicleDAWizard / SCFVDAWizardTab 병행 유지
```

P0-09 UI 범위:

```text
Vehicle Selection             = 구현
Management / Sync / Validation= 구현
Initial Import                = 구현
Overview                      = 구현
Recipe basic view             = 구현
Resolver Preview              = 구현
Pending Authoring Changes     = Resolver FieldDiff authority로 구현
Source Trace                  = Resolver SourceTrace authority로 구현
Definition Validation         = facade Validation projection으로 구현
Apply                         = facade -> existing FCFVehicleApplyService
Undo                          = existing FScopedTransaction -> Unreal standard Undo
Raw DA Open                   = AssetEditorSubsystem navigation
```

P0-09 `Compare / Diff`는 Pending Authoring Changes만 제공한다. Reference Vehicle Compare는 Frozen P0-10 범위다. Assets/Layout full capture, Driving Feel 4축/materialization, Mounts/Defaults full parity도 P0-10으로 보존한다.

ViewModel/UI authority 경계:

```text
SnapshotBuilder direct call = 0
Resolver direct call        = 0
ImportService direct call   = 0
ApplyService direct call    = 0
UCFVDAValidator direct call = 0
Batch API call              = 0
SavePackage/file writer     = 0
```

Slate/ViewModel의 Authoring read/write는 `FCFVehicleAuthoringService` facade만 사용한다. Initial Import의 facade 구현 내부는 기존 SnapshotBuilder/ImportService Core를 재사용하며 규칙을 복제하지 않는다.

Initial Import:

```text
R2 OwnershipWrite approval
Target Definition mutation 0
Preview mutation 0
Existing managed Target block
exact destination + Target hash + import summary approval binding
transient Recipe에서 existing Import Core preflight
summary exact match 뒤에만 persistent Editor-only Recipe 생성
Package Dirty 가능
Auto Save 0
Automatic Retry 0
```

UI가 review dialog에서 본 exact R2 proposal을 ViewModel이 transient하게 보존하며 Commit 시 임의 fresh proposal로 바꿔치기하지 않는다. Service가 current Target/destination을 fresh TOCTOU revalidation한다.

Apply / Undo:

```text
Fresh Preview + Diff>0 + Blocking=0 + ExternalDrift=0
→ BuildApplyApprovalProposal
→ exact reviewed R3 proposal
→ ApplyResolvedVehicle
→ existing FCFVehicleApplyService transaction

Undo
→ GEditor standard Undo / Ctrl+Z
→ Target DefinitionHash + Recipe AppliedState 복원
→ Refresh Preview
```

별도 custom revert 또는 silent retry는 만들지 않았다. stale approval은 Target mutation 0으로 fail-closed하고 ViewModel은 `Preview Out Of Date`로 전환한다.

Raw DA Open은 navigation-only escape hatch다. Managed/Partially Managed에서는 Raw edit가 Source 추적을 우회하고 이후 External Drift가 될 수 있다는 경고를 표시한다. Raw edit를 자동 Override/Recipe intent로 변환하지 않는다.

P0-09 Automation:

```text
Workspace.ViewModelCoreParity
Workspace.PreviewMutationZero
Workspace.InitialImport
Workspace.ApplyUndo
Workspace.StaleApprovalBlock
Workspace.TabRegistration
```

`TabRegistration`은 실제 `FGlobalTabmanager`에서 `CarFight.VehicleAuthoring`과 legacy `CarFight.VehicleDAWizard`가 동시에 등록됐는지 확인하고 `TryInvokeTab(CarFight.VehicleAuthoring)`으로 실제 Slate content를 생성한다.

Final official Build:

```text
76fc476c8ccb4daf895a3b567fe0c992
CarFight_ReEditor Win64 Development
PASS / Exit Code 0
```

Focused P0-09 Automation:

```text
ef56e0b60ae944c19bf481fa070503c9
6 PASS / 0 FAIL
Engine Exit Code 0
```

Final full Data Authoring Automation:

```text
fba6b0793f90450caf2a38eb82844e7c
53 PASS / 0 FAIL
Engine Exit Code 0
Result JSON SHA-256 697230dbce1c9a6281a0e56c4aa292544d3becd28b701a25285a487a50592654
```

P0-08 기존 47개도 전부 유지됐다.

Live technical evidence:

```text
AI-owned Editor start: 72c52520f86444018daa4805818bb910
editor_ready=true
port 8100 owned by exact Editor
UE Bridge connected / generation 11
```

SlateInspector Snapshot은 현재 GoPyMCP server policy에서 tool 실행 전에 `ERR_UE_TOOL_BLOCKED`로 차단됐다. 이를 CarFight UI 실패로 확대하지 않았고 새 GoPyMCP capability validation도 열지 않았다. 실제 tab registration/spawn/content 생성은 위 `Workspace.TabRegistration` Automation으로 기술 검증했다.

보호 범위:

```text
Runtime UCFVehicleData 변경 0
Inventory 변경 0
Fitting 변경 0
Content Asset 변경 0
Batch main page 0
SCFVDAWizardTab 삭제/대체 0
Auto Save 0
Automatic Retry 0
```

P0-09 Technical PASS는 UX 사용성/시각 품질/입력 편의의 USER PASS가 아니다. 해당 판단은 P0-12 USER Authoring Acceptance까지 보존한다.

다음 Gate는 `DAUTH-P0-10 Existing Wizard Migration`이다. P0-10은 기존 Wizard의 Validator UI / Reference Compare / Layout Capture / Driving Feel / Preview / Undo·Revert 유용 기능을 새 Workspace에 parity시키고 managed-target guard를 붙이는 단계다. P0-10 자체는 Wizard 삭제 승인이 아니며 DG/DEL Gate는 그대로 유지한다.

---

# 13. DAUTH-P0-10 Existing Wizard Migration

## 목적

기존 VDA Wizard의 유용 기능을 새 Workspace로 흡수한다.

## Migration 대상

- Validator UI
- Compare
- Layout Capture
- Driving Feel
- Preview
- Undo / Revert

## 기존 Wizard Migration / 삭제 Gate

P0-04에서 정의한 `DataAuthoringDesign.md v0.5.0` Section 23의 Gate를 사용한다.

```text
Deprecated
= DG1~DG5

Physical Delete
= DEL1~DEL7
```

P0-10 단계 자체가 삭제 승인은 아니다.
이 단계에서는 parity와 managed-target guard를 구현해 Deprecated 후보 상태까지만 만들 수 있다.

삭제보다 Hidden / Deprecated 상태를 먼저 사용하며 실제 Source 삭제는 P0-11 Technical Validation과 P0-12 USER Acceptance를 포함한 DEL Gate 이후다.

## 2026-08-18 구현 결과

```text
DAUTH-P0-10 Existing Wizard Migration = Technical PASS
```

새 Workspace parity:

```text
Validator UI
= P0-09 Unified Validation projection 유지 / existing UCFVDAValidator authority

Reference Vehicle Compare
= Current Resolved Preview vs Reference managed Resolved 또는 unmanaged Current Definition
= 117 Stable Field Registry projection
= Same / Different + A/B Source 표시
= read-only / Copy From Reference 0

Assets & Layout
= Recipe AssetIntent + wheel socket intent 편집
= AssetReader derived chassis sockets read-only
= Resolver Preview / Diff / Trace / Validation / Apply 경로 유지
= managed Target direct Layout Capture 0

Wheel Measurement
= Resolver R6 proposal
= Use Measured / Keep Compatibility Default explicit decision
= exact field/rule/candidate/asset fingerprint review
= R2 OwnershipWrite approval
= Recipe AssetAdoption only
= Target mutation 0 / Auto Save 0 / Retry 0

Driving Feel
= Frozen 4-axis semantic intent
= Acceleration / Steering / Grip / Suspension
= Sedan/SUV/Sports/Heavy exact migration preset
= raw Movement Lerp 0
= raw -> 4-axis reverse inference 0

Mounts & Defaults
= Stable-ID Hardpoint / Mount typed intent
= DefaultData semantic intent
= Raw array index authoring 0

Undo / Revert
= Recipe semantic/adoption/measurement transaction -> Unreal standard Undo
= Definition Apply -> existing ApplyService transaction Undo
= Workspace-owned Undo Last Action
= legacy memory Revert를 새 Workspace authority로 사용하지 않음
```

Common Authoring facade P0-10 확장:

```text
ReadManagedTarget
CompareReferenceVehicles
PreviewAdoption / CommitAdoption
ReadMeasurementProposals
PreviewMeasurementDecision / CommitMeasurementDecision
```

UI/ViewModel은 SnapshotBuilder / Resolver / ImportService / ApplyService / UCFVDAValidator를 직접 호출하지 않는다. P0-10 facade 내부에서 기존 Snapshot/Resolver/Import Core를 조합하며 새 Source Truth를 만들지 않는다.

Measurement UX에서 `검토 안 함`과 explicit `Compatibility Default 유지`를 구분하기 위해 Editor-only Recipe `FCFVehicleAssetAdoption`에 four reviewed-default bool metadata를 추가했다. Runtime `UCFVehicleData` schema는 변경하지 않았다.

Legacy `SCFVDAWizardTab` v1.8.0 managed-target guard:

```text
managed Recipe Target
- Validate = 유지
- Compare = 유지
- Raw Open = 유지
- Copy Report = 유지
- Layout Capture = 비활성 + handler 재검사
- Quick Tune Apply = 비활성 + handler 재검사
- Quick Tune Revert = 비활성 + handler 재검사

unmanaged Existing Definition
- 기존 legacy write path 유지
```

관리 상태 조회 실패도 legacy write action은 fail-closed한다. 기존 `CarFight.VehicleDAWizard` tab/spawner는 삭제하지 않았다.

P0-10 Automation:

```text
Workspace.ReferenceCompare
Workspace.LayoutDrivingUndo
Workspace.AdoptionMeasurement
Workspace.LegacyManagedGuard
```

Focused P0-10:

```text
3e6c924318124fbebc742e2400d83308
4 PASS / 0 FAIL
Engine Exit Code 0
```

Final official Build:

```text
03fa78af4e124a3db33893ca8bfe436d
CarFight_ReEditor Win64 Development
PASS / Exit Code 0
```

Final full Data Authoring Automation:

```text
c25dbb8e10eb4bdda5733ead78aa4a43
57 PASS / 0 FAIL
Engine Exit Code 0
Result JSON SHA-256 90c01eda8b1df96f03d55feb1e3000f99db16f8a717baa697f5c15cdb9f6884b
```

P0-08 47개 + P0-09 6개 회귀를 모두 보존했다. P0-09 `Workspace.TabRegistration`도 업데이트된 `CarFight.VehicleAuthoring`을 실제 `TryInvokeTab()`으로 생성하므로 P0-10 page construction을 포함한 Slate 생성 경로가 full Automation에서 다시 PASS했다. 별도 신규 UE MCP capability를 열지 않았다.

Production surface audit:

```text
New Workspace/ViewModel direct SnapshotBuilder = 0
New Workspace/ViewModel direct Resolver = 0
New Workspace/ViewModel direct ImportService = 0
New Workspace/ViewModel direct ApplyService = 0
New Workspace/ViewModel direct UCFVDAValidator = 0
New Workspace Batch API = 0
SavePackage = 0
```

Legacy Wizard의 기존 `UCFVDAValidator` 직접 read/validate 경로는 migration source로 유지한다. managed-target write guard만 추가했으며 Validator 계산식을 복제/변경하지 않았다.

보호 범위:

```text
Runtime UCFVehicleData 변경 0
Inventory 변경 0
Fitting 변경 0
Content Asset 변경 0
Batch main page 0
SCFVDAWizardTab 삭제 0
DG/DEL Gate open 0
Automatic Save / Retry 0 / 0
```

P0-10 Technical PASS는 Wizard Deprecated/physical delete 승인, USER UX PASS, USER Driving Feel PASS가 아니다. P0-11 Technical Validation과 P0-12 USER Authoring Acceptance를 거치기 전 DG/DEL 판정을 확대하지 않는다.

다음 Gate는 `DAUTH-P0-11 Technical Validation`이다.

---

# 14. DAUTH-P0-11 Technical Validation

## 검증 범위

- Resolver deterministic test
- Source trace test
- Override test
- Stale / Regenerate test
- Existing Definition import safety
- Validator integration
- Undo / Transaction
- Inventory non-regression
- Fitting non-regression
- VehicleData Runtime Apply non-regression

## 2026-08-18 P0-11 검증 결과

현재 판정:

```text
DAUTH-P0-11 Core Technical Validation = PASS
DAUTH-P0-11 Frozen UX Completeness Closure = PASS
DAUTH-P0-11 Overall Technical Validation = Technical PASS
Frozen 24.90~24.94 Workspace Completeness = PASS
DAUTH-P0-12 USER Authoring Acceptance = Ready by dependency / Not Started / 이번 작업에서 미개방
```

P0-09/P0-10의 scoped Technical PASS는 그대로 유지한다. Frozen 24.97이 의도적으로 제한했던 MVP/parity 범위는 소급 변경하지 않았고, P0-11에서 Frozen 24.90~24.94 normal Workspace production gap만 Common Authoring facade와 existing Core 재사용으로 닫았다. USER 시각/사용성/Driving Feel 판정은 여전히 P0-12 전용이며 이번 closure에서 PASS로 추정하지 않는다.

### Missing E2E coverage 감사와 보강

기존에 독립적으로 이미 검증된 Resolver deterministic/source trace/stale/conflict, Apply TOCTOU/rollback/exact diff, Initial Import mutation boundary, P0-09 Apply/Undo·stale approval, P0-10 Reference/Measurement/Adoption/legacy guard는 반복하지 않았다.

P0-11에서 실제 부족했던 통합 coverage 두 개만 `CFVehicleAuthoringVMTests.cpp v1.2.0`에 추가했다.

```text
CarFight.DataAuthoring.DAUTH_P0_11.Workspace.AuthoringE2E
CarFight.DataAuthoring.DAUTH_P0_11.Workspace.ConsumerRegression
```

`AuthoringE2E`는 한 representative managed Vehicle/Recipe fixture에서 다음을 연속 검증한다.

```text
fresh managed selection
→ pending Diff / Source Trace / Validation
→ old R3 approval prepare
→ typed Driving Feel Recipe edit
→ old approval invalidation
→ fresh Resolve / Trace / Diff / Validation
→ fresh explicit Apply
→ Target mutation + AppliedState
→ Workspace standard Undo
→ exact pre-Apply Target restoration
→ pending Diff restoration
→ re-Apply
→ Raw VehicleData edit
→ External Drift detection
→ normal Apply disabled
→ Apply prepare fail-closed
→ Recipe fingerprint unchanged
→ Last Applied state preserved
→ raw Target state silently absorbed하지 않음
```

`ConsumerRegression`은 실제 Authoring Apply 결과의 canonical `UCFVehicleData`를 새 Authoring 전용 adapter 없이 기존 consumer에 그대로 전달한다.

```text
UCFVehicleFittingData::BuildFittingSnapshot
FCFInventoryFitAdapter::BuildFittingBinding
```

Applied `Front_New` Hardpoint / `M_Front_New` Mount / BaseVehicleMassKg를 기존 Fitting/Inventory가 읽고, consumer read 전후 full Target hash가 동일함을 확인했다.

### AppliedDefinitionHash RCA

첫 P0-11 focused run에서 두 테스트가 마지막 hash assertion 한 줄씩 실패했다. 원인은 production bug가 아니라 test assumption 오류였다.

Frozen P0-08H 의미는 다음이다.

```text
Recipe.AppliedState.AppliedDefinitionHash
= Resolver-owned field projection의 ResolvedDefinitionHash
!= 반드시 117 전체 Current Definition full hash
```

`FCFVehicleApplyService`도 Apply 후 actual Target full snapshot에서 Resolver-owned field만 다시 projection한 hash를 approved `ResolvedDefinitionHash`와 비교한다. 따라서 production hash 의미는 변경하지 않았고 P0-11 test assertion만 `AppliedState.AppliedDefinitionHash == ApplyResult.CurrentResolvedDefinitionHash`로 교정했다.

### P0-11 Evidence

최종 official Build:

```text
ae0e92c60a094be09b036ae12b106717
CarFight_ReEditor Win64 Development
PASS / Exit Code 0
```

Focused P0-11:

```text
8e0bd3dd8dd449a5b2b776466cb13d4d
2 PASS / 0 FAIL
Engine Exit Code 0
```

대표 consumer non-regression:

```text
Inventory
4dfa10d20f00451abcab209b0db49d40
CarFight.Inventory.INV_P0_04.FittingAdapter
1 PASS / 0 FAIL

Fitting
1c66a7139313458f9c5ab68ff03a7b0f
CarFight.Fitting.FIT_P0_03.Compatibility
CarFight.Fitting.FIT_P0_03.MassSnapshot
2 PASS / 0 FAIL
```

Final full Data Authoring regression:

```text
071d20db24f24c15bb2cb6c553973433
59 PASS / 0 FAIL
Engine Exit Code 0
Result JSON SHA-256 a78df08c8e8d29f02d8906d0805dbd621aba8b5a0e4a07c7f3b64d1c3722475f
```

이는 기존 P0-08 47개 + P0-09 6개 + P0-10 4개 + P0-11 2개를 모두 포함한다.

### Section 14 / Frozen 24.98 최소 기술 계약 판정

다음은 현재 PASS다.

```text
Unmanaged selection mutation0
Initial Import Target mutation0
Recipe edit / old approval invalidation / fresh Resolve
Error·Blocked / External Drift normal Apply fail-closed
Adoption Recipe-only
Measurement Recipe-only
Apply shared lane / exact reviewed projection
Undo Recipe/Definition/AppliedState coherent restoration
Raw DA edit → External Drift
Reference Compare mutation0
Managed Legacy Wizard write guard
Resolver deterministic / Trace / Validator integration
Inventory / Fitting consumer non-regression
Runtime VehicleData canonical consumer contract 유지
Save0 / automatic retry0
```

### Frozen 24.90~24.94 completeness closure

P0-11 Core PASS 뒤 남았던 normal Workspace production gap을 dependency 순서로 닫았다.

```text
24.90 Existing Import
- Handling / Performance Adoption normal Workspace route 추가
- existing generic PreviewAdoption / CommitAdoption + ECFVehicleAdoptGroup 재사용
- 새 ownership Core 0

24.91 Managed Edit / 24.93 Profile Impact
- Shared Profile single-Vehicle numeric edit route 추가
- stable Batch ColumnId + canonical numeric input만 허용
- existing ProfileNumericEdit export/import 3-way + B2 CommitAuthoringSources 재사용
- affected Vehicle / pending Definition impact와 navigation 제공
- dependent Definition auto Apply 0 / Save 0 / retry 0

24.92 External Drift Recovery
- Last Applied / Current Raw / Current Authoring exact 3-way review 추가
- Keep Authoring / Preserve Raw As Legacy Pin / Promote Raw To Advanced Override explicit decision 추가
- Preserve Raw는 existing ImportService Legacy Pin ownership primitive 재사용
- Advanced Override는 Frozen Field Registry allowlist + 이유 필수
- Keep Authoring은 Recipe fingerprint + Target hash + Source signature + Resolver revision에 binding된 transient reviewed token이며 refresh/selection/source mutation 시 폐기
- 새 Apply부터 AppliedTrace에 exact typed LastAppliedValue를 함께 보존하고 기존 LastAppliedValueHash authority는 유지

24.94 Mesh-only Candidate
- existing Definition이 실제 사용하는 Chassis Mesh sibling directory 기반 bounded candidate discovery 추가
- already-used Chassis Mesh 제외
- Create Vehicle From Mesh는 explicit R2 OwnershipWrite review 뒤 Definition + Editor-only Recipe 두 record만 one transaction으로 생성
- Definition은 C++ defaults, Recipe는 explicit Chassis intent/profile bindings만 보존
- Profile/차급/물리/밸런스 추론 0 / auto Apply 0 / auto Save 0
```

Drift focused test에서 Preserve Raw approval scope가 재현되지 않는 실제 defect를 발견했다. 원인은 prospective simulation용 `DuplicateObject<UCFVehicleRecipeData>`가 `PostDuplicate()`에서 새 `RecipeId`를 발급해 Resolver `SourceSignature`와 ProposalHash가 매 preview마다 달라진 것이었다. Approval binding을 약화하지 않고, 새 asset 복제가 아닌 simulation copy에 한해 원본 `RecipeId`를 복원하도록 교정했다.

최종 보호 범위:

```text
P0-08A~M Technical PASS 보존
P0-09 Technical PASS 보존
P0-10 Technical PASS 보존
Runtime UCFVehicleData production schema/source 변경 0
Inventory production source 변경 0
Fitting production source 변경 0
Content Asset 변경 0
SCFVDAWizardTab 삭제 0
DG/DEL Gate open 0
P0-12 execution 0
Generic Raw SetField writer 0
Automatic Save / Retry 0 / 0
Existing Target Definition writer = FCFVehicleApplyService only
```

P0-11 overall은 Technical PASS다. 다음 dependency는 `DAUTH-P0-12 USER Authoring Acceptance`지만 이번 작업에서는 착수하지 않았고 USER UX/Driving Feel PASS를 추정하지 않는다.

## 금지

Technical PASS를 USER Driving Feel PASS로 확대하지 않는다.

---

# 15. DAUTH-P0-12 USER Authoring Acceptance

## 목적

실제로 사람이 새 Vehicle을 만들거나 기존 Vehicle을 튜닝할 때 편리한지 확인한다.

현재 상태:

```text
DAUTH-P0-12 = 전역 일정상 Paused / bounded USER Acceptance checkpoint preserved
UA-01~06 = USER PASS
USER Acceptance PASS = 6
Next USER Gate = UA-07 Driving Feel Authoring USER Acceptance
UA-07 Remote Technical Readiness = PASS / USER PASS로 확대 금지
Completed technical suite replay = 금지 without new failure evidence
DG/DEL/Wizard deletion = 미개방
```

## 확인 항목

- Raw Field를 찾는 시간이 줄었는가
- 새 차량 기본 설정을 AI와 함께 만들기 쉬운가
- 값의 출처를 이해할 수 있는가
- 여러 차량 비교가 쉬운가
- Driving Feel 조정이 Raw 물리값 직접 수정보다 편한가
- 잘못된 자동값을 발견하고 되돌리기 쉬운가
- Fitting / Inventory와 혼동이 없는가

## USER Acceptance 실행 체크포인트

```text
UA-01 Workspace First Impression — USER PASS
- 최초 사용자 결과: 진입 위치 발견 PASS / 영문 중심 가독성 FAIL / 기본 Browser relevance FAIL
- Remediation: 한국어 우선 UI / 테스트·레거시 기본 숨김 / DA_PoliceCar 기본 숨김 / actual WheelMesh candidate 제외
- Technical evidence: official Build 97df66558d694369915fcfa7d08ef261 PASS / MeshCreate focused 7c5ad7cf8d744994bcb1c2229e5f961e 1/1 PASS / fresh Editor Ready
- USER recheck: PASS
- Vehicle Authoring Workspace를 처음 열었을 때 어디서 시작해야 하는지 이해 가능함
- Vehicle Browser / Main Page / Context Panel / Apply Gate의 역할이 이전보다 읽히며 UA-02 진행 허용

UA-02 Browser / Mesh-only / Create / Existing Import — USER PASS
- Existing Vehicle 최초 USER 결과: `기존 차량 가져오기 검토…` 무반응 FAIL / 용어 이해 FAIL
- Existing Vehicle remediation: `제작 관리 시작` 용어 / 기존 VehicleData 불변 + 새 편집용 Recipe 생성 설명 / Vehicle identity 기반 Recipe 이름 자동 제안 / 입력·preview·commit 실패 modal feedback
- Existing Vehicle technical evidence: official Build 388a625d1ed24abda9ded23fe47be88c PASS / Workspace.InitialImport 3e74ee05ff634ed298e30ec4259f4473 1/1 PASS
- Existing Vehicle USER recheck: 의미 이해 PASS / review modal 정상 표시 PASS / subflow USER PASS
- Mesh-only 첫 USER 결과: 기본 Browser에 `차체 메시 후보` 0개 → FAIL
- Mesh-only live evidence: `/Game/CarFight/Vehicles/Meshes` 아래 SUV/Sedan 외 Wagon/CityCar/Van/Compact/Coupe/Pickup/SubCompact StaticMesh 존재
- Mesh-only RCA: existing Definition Chassis의 exact vehicle folder만 query해 sibling vehicle folders를 discovery하지 못함
- Mesh-only remediation: existing chassis folder의 parent collection root를 계산해 bounded recursive StaticMesh query / used chassis + actual WheelMesh 제외 유지
- Mesh-only technical evidence: official Build 82ac0966aa764bc282a92e59495a6be2 PASS / MeshCreate 2a6729146c0c4e1985d53de73cb54665 1/1 PASS
- USER recheck: fresh Editor에서 `차체 메시 후보` 표시 PASS / `메시에서 차량 만들기` 의미 이해 PASS
- UA-02 전체 = USER PASS
- 다음: UA-03 Recipe / Shared Profile / Affected Vehicle Impact
- Managed / Unmanaged / Mesh-only Candidate 구분이 자연스러운가
- Mesh-only의 `메시에서 차량 만들기`와 Existing Vehicle의 `제작 관리 시작` 차이가 명확한가
- 새 Vehicle 생성에서 Raw UCFVehicleData Details를 열 필요가 없는가
- Existing Vehicle의 Legacy Pin/Adoption 흐름이 과하게 번거롭지 않은가

UA-03 Recipe / Shared Profile / Affected Vehicle Impact — USER PASS
- Recipe와 Shared Profile 중 무엇을 수정하는지 구분 가능한가 — PASS
- Shared Profile 수정의 affected Vehicle / pending Definition impact가 이해되는가 — PASS
- 여러 Vehicle에 영향을 주는 변경을 실수로 single Target Apply한다고 오해하지 않는가 — PASS
- 첫 USER 결과: Recipe 생성·`관리됨` 전환은 PASS. Shared Profile binding이 비어 있는데 기존 UI에는 Profile 선택/연결 경로가 없어 subflow FAIL.
- remediation Technical PASS: 5 Domain existing Profile 후보/current binding/explicit Recipe-only binding/Open Profile route 추가, final Build `74d818f4c7a149b89d730c2a615912c2` PASS, focused SharedProfileImpact `c81052d602ae4f869d6aecb431b28d6c` 1/1 PASS.
- Profile Binding remediation USER recheck: Profile 0개 fail-closed 표시 PASS / `DA_Profile_UA_TestHandling` 후보 1개 discovery PASS / explicit Recipe-only binding review 의미 PASS / actual binding 후 Recipe dirty=true·VehicleData dirty=false 확인 PASS.
- Handling B2 impact preview: `Profile.Handling.FrontWheelMaxBrakeTorque=2000`에서 affected Vehicle 1 / 예상 VehicleData 변경 0 / Auto Apply 0 / Auto Save 0 USER 표시 PASS.
- 첫 B2 actual commit: `Shared Profile prospective validation이 commit 직전 Block됐습니다`로 mutation0 Block. `DA_TestSedan` dirty=false 유지, `2000` value commit 안 됨.
- 첫 blocker RCA: `DriveStateMode=VehicleSpecific`인데 DriveState Profile Snapshot이 없어 `RequiredProfileMissing` 1건. Frozen 26.49 prospective validation gate는 정상 동작했으며 완화하지 않았다.
- DriveState Profile binding 후 one-shot 재검사에서 transient rollback/prospective Recipe 오수집 결함을 발견했고 `CFBatchImport.cpp v1.1.2` + `CFVehicleAuthoringVMTests.cpp v1.7.0`으로 scratch exclusion과 regression을 교정했다.
- final official build `3fa7b4d4cca14d37b7fea50631e4fa07` PASS / focused SharedProfileImpact `db57ea0701234e65b8e5921b1dce1e96` 1/1 PASS.
- fresh persisted Recipe에서 Handling+DriveState binding과 VehicleSpecific mode를 확인했다.
- 최종 B2 USER one-shot preview/commit: 2000 / affected Vehicle 1 / expected VehicleData change 0 / Auto Apply 0 / Auto Save 0 PASS. transient Recipe 표출 0.
- commit 직후 live dirty boundary: `DA_Profile_UA_TestHandling=true`, Recipe=false, `DA_TestSedan=false`, DriveState Profile=false. Shared Profile source-only mutation 계약 PASS.
- 당시 USER 상태: UA-01~03 USER PASS / P0-12 USER PASS 3. 이후 UA-04 macro-flow remediation·현지화와 UA-05 readability remediation까지 진행한 중간 checkpoint는 P0-12 USER PASS 5 / UA-06 Explicit Apply·Undo USER Pending이었다. 이후 UA-06까지 USER PASS해 현재 상단 checkpoint는 USER PASS 6 / 다음 UA-07이다.


UA-04 External Drift Recovery — USER PASS
- Last Applied / Current Raw / Current Authoring 차이를 이해할 수 있는가 — PASS
- Keep Authoring / Preserve Raw Legacy Pin / Advanced Override 선택 의미가 충분히 명확한가 — PASS
- Drift 해결 과정이 Raw DA 직접 수정으로 돌아가는 것보다 낫다고 느껴지는가 — macro-flow remediation 후 `전보다 보기 편해졌어` USER 개선 체감 PASS
- Keep decision → same-pane final Apply → 925 recovery USER 확인 PASS
- `Authoring/VehicleData/3-way` 사용자-facing 현지화를 `제작 기준값/차량 데이터/상세 비교`로 반영하고 actual translated screen USER 확인 PASS
- Frozen safety semantics / approval / no-auto-save / no-auto-retry / stale·TOCTOU 계약 변경 0

UA-05 Resolve / Diff / Source Trace / Validation — USER PASS
- Preview → Pending Diff → Source Trace → Validation 흐름 USER 승인
- `문제` formatting remediation USER PASS
- `값 출처`는 검색 + 실제 적용 Source 우선 + 비적용 후보 secondary 표시로 교정 후 `torque` 123→4 실제 확인 USER PASS
- Resolver SourceTrace authority / Validation contract mutation 0

UA-06 Explicit Apply / Undo
- Source edit와 Target Apply의 차이가 분명한가
- Apply 전 실제 변경 범위를 확신할 수 있는가
- 잘못 적용했을 때 UE 표준 Undo로 되돌리는 과정이 자연스러운가

UA-07 Driving Feel Authoring
- Acceleration / Steering / Grip / Suspension 4축 의미가 직관적인가
- Slider local draft → Commit → Resolve → Diff의 흐름이 이해되는가
- Sedan/SUV/Sports/Heavy preset이 의미 있는 shortcut인가
- Raw Movement 물리값 직접 편집보다 빠르고 이해하기 쉬운가

UA-08 Driving Feel Runtime Comparison
- 실제 차량 주행에서 4축 변경이 의도한 방향으로 체감되는가
- 좋은/나쁜 절대값 판정이 아니라 Authoring 입력과 실제 주행 변화의 인과가 이해되는가
```

USER Acceptance 전에는 "입력 문제가 해결됐다"고 완료 판정하지 않는다.

---

## 16. 이후 확장 후보

Vehicle P0가 검증된 뒤에만 다음을 검토한다.

```text
P1 Weapon Authoring
P2 Equipment / Scanner / Armor Authoring
P3 Cross-domain Catalog
P4 Balance Spreadsheet Workflow
P5 Advanced AI Bulk Authoring
```

번호와 범위는 P0 결과에 따라 다시 계획한다.

---

## 17. 현재 착수 지점

DAUTH-P0-00 Foundation Audit, DAUTH-P0-01 Vehicle Field Ownership Matrix, DAUTH-P0-02 Authoring Contract Freeze, DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design, DAUTH-P0-04 VDA Wizard Migration Design, DAUTH-P0-05 Vehicle Authoring UX Design, DAUTH-P0-06 AI Authoring Contract, DAUTH-P0-07 Batch / Excel / CSV Contract는 모두 완료 조건을 충족했다.

다음 작업 준비 지점은:

```text
DAUTH-P0-08 Implementation Foundation
```

이다.

P0-08부터 처음으로 신규 Authoring Source 구현에 진입한다.
Section 22~26의 Core / Migration / UX / AI / Batch 계약을 구현 입력으로 사용하며, 첫 단계는 Editor-only Recipe/Profile schema, Registry/Codec, immutable snapshots, Resolver, Diff/Trace/Stale, Validation/Apply, 공통 Authoring Service와 Automation 기반부터 구축한다.

Batch는 CSV parser부터 UI를 먼저 만드는 것이 아니라 Core Authoring Foundation 위에 `FCFBatchColumnRegistry` / canonical table / manifest / 3-way merge를 얹는다.
P0-01~07에서 동결한 ownership/precedence/UX/AI/Batch 경계를 구현 편의를 이유로 다시 열지 않는다.

P0-08 실제 Source 구현 착수와 함께 `CF-FQ-038`을 Project Feature로 정식 등록했다.
2026-08-18 당시 사용자 우선순위 변경으로 단일 Active는 `CF-FQ-032 UI`였으며, `CF-FQ-038`은 Paused로 전환됐다. 이후 USER Acceptance를 추가 진행해 최신 checkpoint는 `DAUTH-P0-12 USER PASS 7 / UA-01~07 USER PASS / 다음 UA-08 Driving Feel Runtime Comparison`이다. `CF-FQ-034`는 계속 `FIT-P0-07D USER Driving Feel Comparison`을 보존한 Paused다.
P0-08A/B/C/D/E/F/G/H/I/J/K/L/M, `DAUTH-P0-08 Implementation Foundation`, `DAUTH-P0-09 Vehicle Authoring MVP`, `DAUTH-P0-10 Existing Wizard Migration`, `DAUTH-P0-11 Technical Validation` Technical PASS를 반복하지 않는다. Frozen 24.90~24.94 normal Workspace completeness도 closure PASS다. P0-08 당시 117/78 schema evidence는 Historical로 유지하고 UI-P0-06 additive `RedlineStartRPM` 이후 Current compatibility는 118 Registry / Performance numeric 18 / five-Profile numeric 79다. DG/DEL/Wizard 삭제는 USER Acceptance 결과 전까지 열지 않는다.


---

## 18. Changelog

### v0.1.36 - 2026-08-18

- `CF-FQ-032 UI-P0-06`의 explicit `RedlineStartRPM` additive VehicleData schema 변경을 Current DAUTH compatibility surface에 반영했다. CF-FQ-038은 Paused이며 USER Acceptance를 재개하지 않았다.
- P0-08 original Registry 117 / Performance numeric 17 / five-Profile numeric 78은 당시 PASS의 Historical evidence로 유지한다. Current Source는 RedlineStartRPM 1 leaf 추가로 Registry 118 / Performance numeric 18 / five-Profile numeric 79다.
- `RedlineStartRPM`은 Performance domain `PerformanceProfileDirect`, dependency `Profile.Performance`이며 0은 unconfigured다. EngineMaxRPM이나 변속 설정에서 자동 유도하지 않는다.
- bounded compatibility는 Registry `dacb88cb09644d0287cfe7a7f3137b51`, Batch projection `03aad228b1784f1b908ed6e82ddced3e`, Allowlist `c9d07c42428642c186743e8c26043dcb` 각각 1/1 PASS다.
- P0-12 USER PASS 2와 UA-03 `RequiredProfileMissing` blocker, 다음 DriveState Profile binding → Handling B2 actual commit 재검사 체크포인트는 그대로다.

Migration: Historical 117/78 기록을 전역 수정하지 않는다. Current schema/Batch 작업은 118/79를 사용하고, DAUTH USER Acceptance는 사용자가 명시적으로 재개하기 전 진행하지 않는다.

### v0.1.35 - 2026-08-18

- UA-03 Profile Binding remediation을 fresh Editor에서 사용자가 직접 재검증해 0-profile fail-closed, Handling 후보 discovery, Recipe-only binding review와 actual binding을 USER PASS했다. 실제 저장 Recipe identity는 `DA_Recipe_TestSedan`이며 이전 `DA_Recipe_UA_TestSedan` 표기는 rename 미실행에 따른 세션 중 이름으로 정정한다.
- `DA_Profile_UA_TestHandling` 연결 후 `Profile.Handling.FrontWheelMaxBrakeTorque=2000` B2 preview에서 affected Vehicle 1 / 예상 VehicleData 변경 0 / Auto Apply 0 / Auto Save 0을 확인해 Affected Vehicle Impact Preview는 PASS했다.
- actual B2 commit은 prospective validation에서 mutation0 Block됐다. live read에서 `DA_TestSedan` dirty=false를 유지했고 `2000` source value는 commit되지 않았다.
- RCA는 `DriveStateMode=VehicleSpecific` + DriveState Profile Snapshot 없음의 `RequiredProfileMissing`이다. Frozen 26.49는 B2 source commit 전에 affected Vehicle prospective validation 통과를 요구하므로 정상 fail-closed로 판정하고 Core gate는 변경하지 않는다.
- 다음 PC Gate는 `DA_Profile_UA_TestDriveState`를 만들고 `주행 상태` binding을 명시적으로 연결해 `적용 차단 1`을 해소한 뒤 동일 Handling B2를 재실행하는 것이다. UA-03 전체 USER PASS와 USER PASS count 증가는 보류한다.
- PC 종료 전 Recipe/Handling Profile 두 Authoring Asset만 저장하고 일반 종료하도록 안내했으며 실제 save/close 완료는 별도 확인 전 추정하지 않는다.

Migration: UA-01~02, Initial Import, Profile Binding remediation 기술 검증, Handling impact preview는 반복하지 않는다. 다음 USER 세션은 DriveState blocker 해소 → Handling B2 actual commit 확인부터 이어간다.

### v0.1.34 - 2026-08-18

- UA-03 재개 후 `DA_Recipe_UA_TestSedan` 생성 성공과 `DA_TestSedan`의 `관리됨` 전환을 사용자 화면으로 확인했다. Recipe dirty=true / Target dirty=false이며 Initial Import blind 재실행은 금지한다.
- 첫 Shared Profile USER subflow에서 5 Profile binding이 비어 있고 Production Workspace에 existing Profile 선택/명시적 binding UI가 없어 정상 검사를 진행할 수 없음을 확인했다. 실제 Authoring Shared Profile Asset도 0개였다.
- existing `ListProfiles` + `BindVehicleProfile` R1 contract 기반 최소 remediation을 구현했다. VehicleData Target writer, Shared Profile payload direct writer, Auto Apply/Save/Retry는 추가하지 않았다.
- first Build `eb2eb56867ba4557ad325fc04cf4f7a4` compile FAIL은 missing forward declaration 한 줄로 교정했고 final Build `74d818f4c7a149b89d730c2a615912c2` Exit 0 PASS했다. focused SharedProfileImpact process `c81052d602ae4f869d6aecb431b28d6c` 1/1 PASS다.
- USER PASS는 2 유지한다. current user-owned Editor에는 unsaved Recipe가 있어 Browser가 restart/save/discard하지 않았으며 다음 Gate는 fresh binary USER recheck다.

Migration: UA-01~02 및 Initial Import actual을 반복하지 않는다. Recipe 보존을 사용자 명시 방식으로 처리한 뒤 fresh Editor에서 UA-03 Profile binding → Shared Profile change review → affected Vehicle impact를 확인한다.

### v0.1.33 - 2026-08-18

- UA-03 착수 시 관리됨 차량이 0개임을 사용자 화면에서 확인했다.
- UA-03용 관리 차량 준비 대상으로 미관리 `DA_TestSedan`을 선택했고 Recipe 이름은 `DA_Recipe_UA_TestSedan`으로 지정했다.
- 사용자가 `제작 관리 시작` actual commit을 실행했으나 세션 종료 요청 시점에 아직 생성 중이므로 성공/실패 및 `관리됨` 전환을 미확인 상태로 보존한다.
- 다음 세션은 생성 결과 확인을 최우선으로 하며, 실패 시 blind 재실행하지 않고 오류를 먼저 확인한다. 성공 시 UA-03 Recipe / Shared Profile / Affected Vehicle Impact USER 검사를 이어간다.
- USER PASS는 2 유지, UA-03 PASS 추정 금지, Auto Save/VehicleData Apply 완료 추정 금지다.

Migration: UA-01~02는 반복하지 않는다. 새 세션은 현재 `DA_Recipe_UA_TestSedan` 생성 terminal 결과 확인부터 시작한다.

### v0.1.32 - 2026-08-18

- fresh Editor에서 사용자가 Mesh-only Candidate 표시와 `메시에서 차량 만들기` 의미를 직접 확인해 UA-02 전체를 USER PASS 처리했다.
- Existing Vehicle `제작 관리 시작` PASS와 Mesh-only Candidate/Create PASS를 결합해 P0-12 USER PASS checkpoint를 2로 전진했다.
- 다음 USER Gate는 `UA-03 Recipe / Shared Profile / Affected Vehicle Impact`다. DG/DEL/Wizard deletion은 계속 미개방이다.

Migration: UA-01~02는 반복하지 않는다. bounded USER Acceptance를 이어갈 때는 UA-03부터 진행한다.

### v0.1.31 - 2026-08-18

- UA-02 Mesh-only 첫 USER 확인에서 후보 0개를 확인해 subflow를 FAIL로 유지했다.
- live Asset Registry와 Source RCA로 actual sibling vehicle folders가 존재하지만 discovery가 existing chassis exact folder에 갇혀 있음을 확인했다.
- `CFVehicleAuthoringService.cpp v1.5.0`은 common mesh collection root bounded recursive query로 교정했고 used Chassis / actual WheelMesh exclusion을 유지했다.
- `CFVehicleAuthoringVMTests.cpp v1.5.0`은 sibling vehicle directory candidate fixture로 회귀를 강화했다.
- official Build `82ac0966aa764bc282a92e59495a6be2` PASS, focused MeshCreate `2a6729146c0c4e1985d53de73cb54665` 1/1 PASS를 확보했다.
- USER PASS는 1을 유지한다. fresh Editor에서 candidate visibility와 Create UX를 사용자가 확인하기 전 UA-02 전체를 PASS하지 않는다.

Migration: 다음은 fresh Editor restart 후 UA-02 Mesh-only Candidate/Create USER recheck다. Existing Vehicle subflow는 반복하지 않는다.

### v0.1.30 - 2026-08-18

- fresh Editor에서 사용자가 `제작 관리 시작`의 의미와 검토 팝업 정상 도달을 직접 확인해 UA-02 Existing Vehicle subflow를 USER PASS 처리했다.
- 실제 Recipe 생성 commit은 이번 checkpoint에서 수행하지 않았다. 기존 VehicleData 불변 / 새 편집용 Recipe 생성이라는 UX 의미 확인만 PASS 근거로 사용했다.
- UA-02 전체는 계속 In Progress이며 다음 USER step은 Mesh-only Candidate → `메시에서 차량 만들기`다. P0-12 USER PASS count는 아직 1이다.

Migration: Existing Vehicle 관리 시작 recheck는 반복하지 않는다. UA-02는 Mesh-only Create flow부터 이어간다.

### v0.1.29 - 2026-08-18

- UA-02 첫 USER 검사에서 Existing Vehicle의 버튼 무반응과 `기존 차량 가져오기` 용어 이해 실패를 확인해 UA-02를 FAIL로 유지했다.
- RCA는 비어 있는 Recipe name + silent preview failure였으며 Core Import/approval 계약은 정상이다.
- `CFVehicleAuthoringTab.cpp v1.4.0`에서 사용자-facing flow를 `제작 관리 시작`으로 교정하고 기본 Recipe name suggestion, visible input labels, preview/commit failure modal을 추가했다.
- official Build `388a625d1ed24abda9ded23fe47be88c` PASS, focused `Workspace.InitialImport` `3e74ee05ff634ed298e30ec4259f4473` 1/1 PASS를 확보했다.
- USER PASS는 1을 유지한다. UA-02 Existing Vehicle subflow는 새 UI 직접 재확인 전 PASS하지 않는다.

Migration: UA-01과 P0-08~11은 반복하지 않는다. 다음은 fresh Editor UI에서 UA-02 Existing Vehicle `제작 관리 시작` 재확인이다.

### v0.1.28 - 2026-08-18

- remediation 적용 후 사용자가 새 Workspace를 직접 재확인해 `UA-01 Workspace First Impression = USER PASS`로 판정했다.
- P0-12 USER PASS checkpoint를 1로 전진하고 다음 USER Gate를 `UA-02 Browser / Mesh-only / Create / Existing Import`로 이동했다.
- CF-FQ-038 Paused, P0-08~11 Technical PASS 보호, DG/DEL/Wizard deletion 미개방은 그대로 유지한다.

Migration: UA-01은 반복하지 않는다. bounded USER Acceptance를 계속할 때는 UA-02부터 진행한다.

### v0.1.27 - 2026-08-18

- UA-01 remediation 후 fresh official Build `97df66558d694369915fcfa7d08ef261` PASS / Exit 0을 확보했다.
- 직접 관련 `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate` focused regression `7c5ad7cf8d744994bcb1c2229e5f961e`는 1/1 PASS / 0 FAIL / Engine Exit 0이며 actual WheelMesh의 Mesh-only Chassis Candidate 오분류 방지 회귀를 포함한다.
- fresh Editor `2d70f68ea0f747169d70b9d2afa30bce`는 Ready / port 8100 Editor ownership / protocol_ready까지 확인했다.
- UA-01 remediation은 Technical PASS지만 USER PASS는 계속 0이다. 새 한국어 우선 UI와 기본 Browser relevance를 사용자가 직접 재확인하기 전 UA-02로 이동하지 않는다.

Migration: 다음 실행은 Workspace를 다시 열어 UA-01 사용자 재확인만 수행한다. P0-08~11 replay, DG/DEL, Wizard deletion은 계속 금지한다.

### v0.1.26 - 2026-08-18

- UA-01 실제 사용자 확인에서 Workspace 진입 발견은 쉬웠으나 영문 중심 가독성과 폐기/기술 record가 섞인 Vehicle Browser가 사용성 FAIL로 확인돼 전체 UA-01을 PASS하지 않았다.
- Source audit에서 VehicleData가 wheel visual로 사용하는 StaticMesh가 existing Chassis sibling query 때문에 Mesh-only Chassis Candidate로 오분류될 수 있음을 추가 확인했다.
- remediation source는 한국어 우선 UI, `테스트/레거시 표시` 기본 off, exact `DA_PoliceCar` 기본 숨김, actual WheelMesh 참조 candidate 제외로 교정했다. DG/DEL/Wizard 삭제와 Runtime/Inventory/Fitting 계약은 열지 않았다.
- official Build `d98c2c48481a453891d7f9471253d5cb`는 변경 C++ compile까지 성공했지만 사용자 사용 중 Editor DLL lock으로 link `LNK1104` 종료됐다. Editor를 자동 종료하지 않고 USER 종료 후 fresh build 1회만 재개한다.
- USER PASS는 0이며 UA-01 재확인 전 UA-02로 이동하지 않는다.

Migration: P0-08~11 Technical PASS를 반복하지 않는다. 다음 bounded 순서는 Editor 사용자 종료 → official build → `DAUTH_P0_11.FrozenUX.MeshCreate` focused regression → UA-01 사용자 재확인이다.

### v0.1.25 - 2026-08-18

- 사용자 우선순위 변경으로 `CF-FQ-038`을 `DAUTH-P0-12`에서 Paused로 전환하고 `CF-FQ-032 UI`를 현재 단일 Active로 넘겼다.
- P0-08A~M/P0-09~11 Technical PASS와 Frozen 24.90~24.94 completeness, final Build/Automation evidence를 그대로 보존한다.
- P0-12 USER PASS는 0이고 재개 지점은 `UA-01 Workspace First Impression`이다. DG/DEL과 Wizard deletion은 미개방 상태를 유지한다.
- pause 자체로 Source·Content Asset·Runtime·Inventory/Fitting 계약을 변경하지 않았다.

Migration: DAUTH 재개 시 P0-08~11을 반복하지 않고 UA-01부터 사용자 직접 Acceptance만 진행한다. UI 우선 작업 중 자동 재개하지 않는다.

### v0.1.24 - 2026-08-18


- `DAUTH-P0-12 USER Authoring Acceptance`를 In Progress로 착수했다. 기존 P0-08A~M/P0-09~11 Technical PASS는 반복하지 않는다.
- Frozen Section 24.99를 실제 USER Gate로 사용하고 UA-01~08 체크포인트를 추가했다: Workspace first impression, Browser/Create/Import, Recipe/Profile impact, Drift 3-way, Resolve/Diff/Trace/Validation, Apply/Undo, Driving Feel authoring, runtime comparison.
- USER PASS는 사용자 직접 화면/입력/주행 확인만 근거로 하며 현재 PASS 0이다.
- DG/DEL Gate와 Wizard physical deletion은 계속 미개방이다.

### v0.1.23 - 2026-08-18

- `DAUTH-P0-11 Frozen UX Completeness Closure`를 완료해 Frozen 24.90~24.94 normal Workspace gap을 닫고 P0-11 overall을 Technical PASS로 판정했다.
- Handling/Performance Adoption은 existing generic Import/Adoption Core를 그대로 사용하고 UI route만 추가했다.
- Shared Profile single-Vehicle edit는 stable Batch ColumnId/canonical numeric input을 existing ProfileNumericEdit 3-way + B2 `CommitAuthoringSources`에 연결했으며 affected Vehicle/pending Definition impact navigation을 제공한다. Target auto Apply/Save/retry는 0이다.
- External Drift는 Last Applied / Current Raw / Current Authoring 3-way와 Keep / Preserve Raw Legacy Pin / Advanced Override decision을 구현했다. new Apply trace는 exact typed LastAppliedValue를 추가 저장하되 기존 hash authority를 유지한다.
- Drift prospective duplicate의 `PostDuplicate()` RecipeId 재발급 때문에 approval hash가 비결정적이던 defect를 원본 RecipeId 복원으로 교정했고 approval binding 항목은 약화하지 않았다.
- Mesh-only Candidate는 existing Chassis sibling directory의 unused mesh를 bounded projection하고 explicit R2 review 뒤 Definition + Recipe 두 record만 생성한다. Profile/차급/물리/밸런스 추론과 auto Apply/Save는 0이다.
- final Build `98dfceab797942218bd09c844e398ceb` PASS, focused P0-11 `b53998e6cacf48a1a554d784b77e013c` 6/6 PASS, focused JSON SHA-256 `bce3414ec30cb4612209ff1f39f649990140950b73d4136929a3fa8a4be47263`를 확보했다.
- full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS / Engine Exit 0, result JSON SHA-256 `a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f`를 확보했다.
- Runtime `UCFVehicleData`, Inventory/Fitting production source, Content Asset, Wizard 삭제, DG/DEL은 변경/개방하지 않았다. P0-12는 다음 dependency지만 이번 작업에서 실행하지 않았다.

### v0.1.22 - 2026-08-18

- `DAUTH-P0-11 Technical Validation`의 missing integration coverage를 감사하고 representative `AuthoringE2E`와 `ConsumerRegression` 2개만 추가했다. production Authoring/Runtime/Inventory/Fitting source는 변경하지 않았다.
- final Build `ae0e92c60a094be09b036ae12b106717` PASS, focused P0-11 `8e0bd3dd8dd449a5b2b776466cb13d4d` 2/2 PASS, Inventory representative `4dfa10d20f00451abcab209b0db49d40` 1/1 PASS, Fitting representative `1c66a7139313458f9c5ab68ff03a7b0f` 2/2 PASS를 확보했다.
- final full Data Authoring `071d20db24f24c15bb2cb6c553973433` 59/59 PASS, JSON SHA-256 `a78df08c8e8d29f02d8906d0805dbd621aba8b5a0e4a07c7f3b64d1c3722475f`를 확보했다.
- `AppliedState.AppliedDefinitionHash`가 full 117 Current Definition hash가 아니라 Resolver-owned projection의 `ResolvedDefinitionHash`라는 Frozen P0-08H 의미를 재확인했고 production을 바꾸지 않고 잘못된 P0-11 assertion만 교정했다.
- Section 14와 Frozen 24.98의 최소 technical handoff contract는 PASS로 판정했다.
- 사용자가 명시한 Frozen 24.90~24.94 전체 workflow audit에서는 normal Workspace Handling/Performance Adoption UI, Shared Profile edit/impact UX, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh가 미구현임을 확인했다.
- 따라서 P0-08/P0-09/P0-10 scoped Technical PASS는 보존하되 `DAUTH-P0-11 Overall Technical Validation`은 `In Progress / Completeness Blocked`로 유지하고 P0-12를 열지 않았다.
- 다음 착수 지점은 `DAUTH-P0-11 Frozen UX Completeness Closure`다. Wizard 삭제/DG/DEL/USER Acceptance는 여전히 금지한다.

### v0.1.21 - 2026-08-18

- `DAUTH-P0-10 Existing Wizard Migration`을 Technical PASS로 완료했다.
- `CFVehicleAuthoringVM v1.2.0`, `CFVehicleAuthoringTab v1.1.0`, `CFVehicleAuthoringP10.cpp v1.0.0`으로 Assets/Layout, Wheel Measurement review, Frozen 4축 Driving Feel/preset, Reference Vehicle Compare, Stable-ID Mount/Default authoring, Workspace-owned standard Undo를 새 Workspace에 parity했다.
- Reference Compare는 기존 Snapshot/Resolver의 117 Stable Field projection만 사용하며 read-only이고 Copy From Reference를 만들지 않았다.
- Measurement/Adoption은 Common Authoring facade의 R0/R2 reviewed operation으로 기존 Resolver R6 proposal과 ImportService Core를 재사용하며 Recipe-only transaction / Target mutation0 / auto-save0 / retry0을 유지한다.
- `검토 안 함`과 explicit Compatibility Default 확인을 구분하는 four Editor-only AssetAdoption metadata를 추가했으며 Runtime `UCFVehicleData` schema는 변경하지 않았다.
- Legacy `SCFVDAWizardTab v1.8.0`은 managed Recipe Target에서 Layout Capture / Quick Tune Apply / Quick Tune Revert를 비활성화하고 handler에서도 facade managed state를 재검사한다. Validate/Compare/Raw Open/Copy는 유지하며 unmanaged path도 보존했다.
- Wizard source 삭제, DG/DEL Gate, Batch main page는 열지 않았다.
- final Build `03fa78af4e124a3db33893ca8bfe436d` PASS, focused P0-10 `3e6c924318124fbebc742e2400d83308` 4/4 PASS, full Data Authoring `c25dbb8e10eb4bdda5733ead78aa4a43` 57/57 PASS를 확보했다.
- final result JSON SHA-256은 `90c01eda8b1df96f03d55feb1e3000f99db16f8a717baa697f5c15cdb9f6884b`이며 P0-08 47개 + P0-09 6개 회귀를 모두 유지했다.
- 다음 Gate를 `DAUTH-P0-11 Technical Validation`으로 이동했다. P0-10 Technical PASS를 USER UX/Driving Feel PASS나 Wizard deletion approval로 확대하지 않는다.

### v0.1.20 - 2026-08-17

- `DAUTH-P0-09 Vehicle Authoring MVP`를 Technical PASS로 완료했다.
- `FCFVehicleAuthoringVM v1.1.0`과 `SCFVehicleAuthoringTab v1.0.0`으로 Frozen Section 24의 single-Vehicle 3-pane Workspace, Browser/Selection, Overview, Recipe basic intent, Resolver Preview, Pending Diff, Source Trace, Validation, Apply/Undo, Raw DA Open을 구현했다.
- UI/ViewModel은 `FCFVehicleAuthoringService` facade만 사용하며 SnapshotBuilder/Resolver/ImportService/ApplyService/Validator/Batch를 직접 호출하지 않는다.
- Initial Import R2 reviewed proposal/commit facade를 추가해 unmanaged Existing Definition을 Target mutation 없이 Legacy baseline Recipe로 가져오며, transient Core import preflight가 성공한 뒤에만 persistent Recipe를 만든다.
- Apply는 existing `FCFVehicleApplyService` shared lane만 사용하고 standard Unreal Undo가 Target DefinitionHash와 Recipe AppliedState를 복원하는 것을 Automation으로 검증했다. stale prepared approval은 mutation 0 / retry 0으로 차단한다.
- 기존 `CarFight.VehicleDAWizard` / `SCFVDAWizardTab`은 그대로 유지하고 새 `CarFight.VehicleAuthoring` Nomad Tab을 병행 등록했다. Batch main page와 P0-10 Reference Compare/Driving Feel/Layout/Mount parity는 만들지 않았다.
- final Build `76fc476c8ccb4daf895a3b567fe0c992` PASS, focused P0-09 `ef56e0b60ae944c19bf481fa070503c9` 6/6 PASS, final full Data Authoring `fba6b0793f90450caf2a38eb82844e7c` 53/53 PASS를 확보했다.
- final result JSON SHA-256은 `697230dbce1c9a6281a0e56c4aa292544d3becd28b701a25285a487a50592654`이며 P0-08 기존 47개도 전부 유지됐다.
- AI-owned fresh Editor가 8100 Ready/UE Bridge connected까지 올라오는 것을 확인했다. SlateInspector Snapshot은 server policy에서 실행 전 block됐으나 실제 Nomad Tab 등록/생성/content는 `Workspace.TabRegistration` Automation으로 검증했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset은 변경하지 않았고 auto save/retry는 0이다.
- 다음 Gate를 `DAUTH-P0-10 Existing Wizard Migration`으로 이동했다. P0-10은 parity/guard 단계이며 Wizard physical delete 승인이 아니다.

### v0.1.19 - 2026-08-17

- `DAUTH-P0-08M B3 Batch Definition Apply Foundation`을 Technical PASS로 완료하고 `DAUTH-P0-08 Implementation Foundation` 전체를 Technical PASS로 닫았다.
- `CFBatchApply.h/.cpp v1.0.0`에 fresh per-target R3 evidence, deterministic BatchApplyPlanHash, exact ordered Target/per-target evidence approval, all-target global preflight와 canonical per-Vehicle ApplyService orchestration을 추가했다.
- NoChange/ShadowOnly/ExternalDrift/Resolve Error·Blocked/Validation Blocked를 ineligible로 분리하고 External Drift bulk resolution은 만들지 않았다.
- first mutation 전에 모든 eligible item을 fresh resolve해 exact R3 evidence를 재검증하며 하나라도 stale이면 ApplyService 0 / Applied 0으로 fail-closed한다.
- 실행은 TargetPath ascending이며 각 attempted Vehicle이 `FCFVehicleApplyService::Apply`를 exact once 호출한다. Batch global transaction/rollback은 없고 per-Vehicle atomicity는 기존 ApplyService가 소유한다.
- Stop On First Failure와 Applied/Failed/NotStarted/Ineligible partial result를 구현했다. 이미 Applied Vehicle 자동 rollback, 실패 이후 auto continue, automatic retry, automatic save는 모두 0이다.
- Private-only `CFBatchApplyTestAccess.h`로 global preflight 이후 second Apply 직전 stale을 주입해 실제 ApplyService precondition failure 기반 partial result를 검증했다. Public contract에 test flag는 없다.
- focused process `a49d21b6b9b94d648142d5a555a79c95` 4/4 PASS, final official Build `10027d18360e48f399a5b439f280c24c` PASS, final targeted process `cb2ae69e9c834657a553fe52c00f5a96` 47/47 PASS를 확보했다.
- final result JSON SHA-256은 `472ef5bb8f5fdeb46ebd1b7e68f1fc5a66acba065828ff025167b475aaf25595`이며 P0-08A~L 기존 43개도 모두 유지됐다.
- production source audit에서 Batch-level transaction/save/direct Target write/B1-B2 commit reuse/Wizard/Fitting/Inventory/alternate Apply lane은 0이고 actual Target mutation call site는 `FCFVehicleApplyService::Apply` 하나다.
- Frozen 26.66~26.69를 재대조한 결과 BatchOperationId transient dedupe/generic envelope는 M의 선행 dependency가 아니다. 현재 external transport/client가 없으므로 구현하지 않고 첫 실제 external Batch client/transport integration 전에 다시 여는 Frozen follow-up으로 보존한다. 26.67 automatic retry 금지는 이미 준수한다.
- 다음 Gate를 `DAUTH-P0-09 Vehicle Authoring MVP`로 이동했다. Batch UI는 Frozen 26.70대로 P0-09 필수가 아니다.

### v0.1.18 - 2026-08-17

- `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`을 Technical PASS로 완료했다.
- `CFBatchImport.h v1.1.0 / CFBatchImport.cpp v1.1.1`에 exact BatchPlanHash, included RowId set, per-row ProposedPatchHash/current fingerprint approval binding과 B1/B2 commit contract를 추가했다.
- 모든 included source의 current fingerprint/type/patch hash와 B1 Recipe validation 또는 B2 affected Recipe inventory/validation을 mutation 전에 global preflight하며 하나라도 stale이면 transaction 시작 전 mutation 0으로 중단한다.
- B1/B2는 one logical `FScopedTransaction`에서 deterministic typed numeric patch와 `AuthoringRevision +1`을 수행하고 semantic fingerprint/revision postcheck를 통과한 뒤에만 package Dirty를 남긴다. automatic save는 0이다.
- transaction 중 실패하면 모든 included source를 full reflected backup으로 restore하고 transaction cancel, original fingerprint/revision/dirty state readback까지 확인한다. partial source success를 남기지 않는다.
- External Drift와 ShadowOnly/downstream Definition validation을 B1 source blocker로 승격하지 않고, B2는 Frozen 26.49대로 affected Recipe inventory와 prospective affected-Vehicle validation을 fresh 재검사한다.
- successful B1/B2 뒤 old Session/Approval replay는 source fingerprint mismatch로 fail-closed하며 fresh read/resolve가 필요하다. B1/B2 approval을 B3에 재사용하지 않는다.
- RCA에서 K downstream Definition validation을 source approval blocker로 과잉 승격한 경계를 교정했고, Managed/valid Definition 계약을 위반하던 Automation fixture를 Frozen validation 계약에 맞췄다. Resolver/ownership/Validator 계약 자체는 변경하지 않았다.
- focused BatchCommit process `0436e7f86e17493aa24c6e4ff8d9bd05` 6/6 PASS를 확보한 뒤 final official Build `2e2923f31d5a4f3fa703f3b7623c0741` PASS와 final targeted process `3853d42dfb344bfd8eb6e516903a8d78` 43/43 PASS를 확보했다.
- final result JSON SHA-256은 `eef2051ec4b3ccb5dce10b8b76761299885d18a8bf7f2c9db42cc2967320abcf`이며 P0-08A~K 기존 37개도 모두 유지됐다.
- production source audit에서 B3 Apply/ApplyService 호출, file save, direct Target writer, Fitting/Wizard/Runtime Inventory 경로는 0이다. Runtime `UCFVehicleData`, Inventory, Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader를 변경하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 checkpoint는 Frozen Section 26.54~26.65의 `DAUTH-P0-08M B3 Batch Definition Apply Foundation`이다. 이 `P0-08M` 명칭은 Frozen B3 구간을 구현 Roadmap에서 지칭하기 위해 새로 부여한 checkpoint label이다.

### v0.1.17 - 2026-08-17

- `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`을 Technical PASS로 완료했다.
- `CFBatchImport.h/.cpp`를 추가해 companion manifest/current Registry validation, standard quoted CSV parse, stable ColumnId mapping, duplicate row/source identity와 reserved/read-only modification fail-closed guard를 구현했다.
- import 시 current Recipe/Profile truth를 P0-08J row projection으로 다시 읽고 fingerprint mismatch를 단독 실패가 아닌 freshness signal로만 사용하도록 Section 26.37을 구현했다.
- Export Baseline / Edited Spreadsheet / Current Unreal의 SafeCandidate, NoSpreadsheetChange, ConvergedNoChange, ConcurrentEditConflict와 `OwnershipChangedSinceExport`를 exact cell state로 구현했다.
- blank는 NoChange, formula/locale decimal/non-finite/trailing garbage는 invalid numeric으로 처리하고 source mode 자동 전환을 금지했다.
- Recipe SafeCandidate는 transient duplicate + existing Authoring facade Resolver로, Profile SafeCandidate는 transient shared Profile payload + affected Recipe별 existing Pure Resolver로 prospective preview한다. persistent Recipe/Profile/Target mutation은 0이다.
- per-row status/issues, affected Vehicle drill-down, preview summary와 physical row/column order에 독립적인 deterministic `BatchPlanHash`를 구현했다.
- first build `790c54be3c464b6cbd51994c78652d25`와 second build `6437ecb4a41443a6abdbea1ec8f28874`의 UE 5.8 API mismatch를 RCA·교정했고 production checkpoint `d6ea14ae4d854b37a465836037567c66`, final official Build `a3d37d4c8520442d8ceb09a72eb6a68f` PASS를 확보했다.
- targeted process `7d2db5a9baa54dd9abf25857138d3994`에서 P0-08A~K 전체 37/37 PASS, result JSON SHA-256 `bbdf497d472e5ec2fbe4458e075907897ca79b2ac984d43ba3290cff3c55eeec`를 확보했다.
- production source audit에서 B1/B2 commit, B3 Apply, file save, package mutation, FieldCodec/raw SetField, Fitting/Wizard/Runtime Inventory 경로는 0이다.
- Runtime `UCFVehicleData`, Inventory, Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader를 변경하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 checkpoint는 Section 26.44~26.53의 `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`이다. Section 26.54+ B3는 다음 단계로 분리한다.

### v0.1.16 - 2026-08-17

- `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`을 Technical PASS로 완료했다.
- `FCFBatchColumnRegistry`를 existing Recipe/Profile typed schema와 Frozen 117 Field Registry의 projection으로 구현했으며 Spreadsheet/CSV를 새 Source Type 또는 SSOT로 만들지 않았다.
- RecipeNumericEdit는 Driving Feel 4 + Mass explicit 2 + Durability explicit 1의 7 numeric semantic leaf만 허용하고 Mass/Durability는 current Source Mode가 ExplicitValue일 때만 editable-at-export다.
- ProfileNumericEdit는 exact one Domain typed `Data` payload의 scalar numeric leaf만 projection하며 Automation으로 VehicleBase 8 / Drivetrain 1 / Handling 41 / Performance 17 / DriveState 11, 총 78개를 확정했다. enum/bool/asset/array/Stable-ID/complex identity는 제외했다.
- `__cf_` reserved metadata, stable technical ColumnId, typed mutation metadata, NoChange blank policy와 duplicate/reserved collision fail-closed validation을 구현했다.
- Resolved report metadata는 `FCFVehicleFieldRegistry::GetDescriptors()` 117개를 1:1 projection하고 current reflected leaf의 ValueType/Unit만 read-only로 보강했다.
- canonical CSV는 stable header/row order, invariant decimal, finite number, standard quote escaping, BOM 없는 UTF-8을 사용한다.
- `.cfbatch.json` manifest에는 export identity/schema/revision/resolver revision/column descriptors/row identities/fingerprints/baseline values/ownership source mode/ExportSetHash를 immutable-style evidence로 보존하고 tamper/current schema mismatch를 검출한다.
- first build `814c9a32c1a441fbacfee89fbce5123b`와 second build `0e7c1f31a16b4c708972ebf2606dbe89`의 UE 5.8 API mismatch를 각각 RCA·교정했으며 final official Build `3249098c1b99487a8fd173694573bd5e` PASS를 확보했다.
- targeted process `534486583a1d4689bc5137505a03f806`에서 P0-08A~J 전체 32/32 PASS, result JSON SHA-256 `84fc15de1ad23b4e13891d91db09fa3668a49c900a438dec17b70d818a45d36f`를 확보했다.
- source audit에서 Import Session, source commit, Definition Apply, file save, package mutation, raw SetField/FieldCodec ImportValue, Fitting/Wizard 경로는 0이다.
- Runtime `UCFVehicleData` schema/source, Inventory, Fitting, `SCFVDAWizardTab`, Content Asset, existing Validator/Asset Reader는 변경하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 checkpoint는 `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`이다.

### v0.1.15 - 2026-08-17

- `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`을 Technical PASS로 완료했다.
- `CFVehicleAIContract.h`와 `FCFVehicleAuthoringService`를 추가해 Section 25의 R0~R3 risk, exact approval class, typed result/error/mutation footprint, proposal scope와 ClientOperationId foundation을 구현했다.
- R0 List/Read/Resolve/Diff/Trace/Validation/Drift는 기존 SnapshotBuilder / AssetReader / Pure Resolver 결과를 얇게 orchestration하며 persistent mutation은 0이다.
- `PreviewRecipeChange`는 Recipe Snapshot value-copy에 typed semantic command를 적용하고 같은 Resolver를 호출해 ProposalHash / prospective source/hash/diff/validation을 만들며 persistent mutation은 0이다.
- R1 11개 normal semantic operation은 exact `AuthoringWrite`, expected Recipe/Target/revision, fresh proposal scope를 요구하며 Recipe만 transaction mutation한다. higher approval 재사용, Raw SetField, direct Target write, Save, automatic retry는 허용하지 않는다.
- bounded Editor-lifetime dedupe는 same ClientOperationId+same request를 terminal result replay로 처리해 write를 반복하지 않고, 다른 request 재사용은 conflict로 차단한다.
- 첫 26/27 Automation RCA에서 `VehicleArchetypeId`가 Section 22.27 Frozen Resolver RecipeFingerprint input이 아님을 재확인하고 Frozen hash 계약은 변경하지 않았다. v1.1.0 facade에서 typed desired-state equality로 NoChange를 판정하고 fingerprint-covered Feel concurrent edit로 stale guard를 별도 검증했다.
- R3 `ApplyResolvedVehicle`는 ExpectedDiffHash/DefinitionApply approval을 검사한 뒤 existing `FCFVehicleApplyService::Apply`를 actual exact 1회 호출하며 별도 Target writer를 만들지 않았다.
- final official Build `252fdbd097f943379f2a1e2a942bedef` PASS, final targeted process `f2011a206c964fadb269d13c4dba3c86` 27/27 PASS, result JSON SHA-256 `b9bfea5be21f1bcc521e0bcfa8b8a43c017e18a4dec65a208bfd57a68805bfbf`를 current evidence로 연결했다.
- source audit에서 Raw `SetField`, FieldCodec ImportValue, SavePackage, direct Target mutation, force/skip-validation API는 0이고 actual ApplyService call은 1개임을 확인했다.
- Runtime `UCFVehicleData` / Inventory / Fitting / `SCFVDAWizardTab` / Content Asset / existing Validator / Asset Reader 변경 0, UI / CSV / Batch 구현 0을 유지했다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 checkpoint는 `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`이다.

### v0.1.14 - 2026-08-17

- `DAUTH-P0-08H Apply Transaction Foundation`을 Technical PASS로 완료했다.
- `FCFVehicleApplyRequest/Result`와 `FCFVehicleApplyService`를 추가해 Section 22.33~22.35의 Recipe/Source/Target/Resolved/ResolverRevision TOCTOU precondition과 fresh Resolve/Diff equality를 구현했다.
- Current Target transient duplicate에 dependency-safe exact Apply plan을 먼저 실행하고 Resolver-owned readback hash와 기존 `UCFVDAValidator`를 통과한 경우에만 persistent transaction으로 진입한다.
- R14 Foundation의 SetLeaf/Add identity 의미는 보존하고 current-only stable selector RemoveArrayElement만 ApplyService가 current-vs-resolved selector set으로 deterministic synthesize한다. explicit order intent가 없는 MoveArrayElement는 fail-closed한다.
- actual transaction은 Target/Recipe Modify, exact Diff write, Target readback/Validator, `FCFVehicleAppliedState` source/value trace 갱신, Package Dirty/PostEditChange까지만 수행하며 automatic Save는 0이다.
- A8~A10 failure는 full Target transient backup과 Recipe AppliedState를 자체 복원하고 transaction cancel, pre-apply dirty flag 복원, full Target Definition hash 재검증까지 수행한다.
- Private `CFVehicleApplyTestAccess`로 actual Target mutation 직후 controlled failure를 주입해 rollback branch를 제품 public API 오염 없이 Automation에서 검증했다.
- final official Build `b88831b71797415fac9dc0a23d12c39e` PASS, targeted process `6f261c0b76f04d0faa6b9e855865818e` 23/23 PASS, result JSON SHA-256 `382959329a4cefd1e1c884b5c87fc48352f15092b3207f32d63ca4cd806fce9b`를 current evidence로 연결했다.
- production Apply source에서 SavePackage 계열 호출 0을 확인하고, Target write lane은 `FCFVehicleApplyService` 하나로 중앙화했다. Automation fixture setup direct assignment는 제품 writer가 아니다.
- Runtime `UCFVehicleData` schema / Inventory / Fitting / `SCFVDAWizardTab` / Content Asset / existing Validator / Asset Reader 변경 0, UI / CSV / Batch 구현 0을 유지했다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 checkpoint는 `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`이다.

### v0.1.13 - 2026-08-17

- `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`을 Technical PASS로 완료했다.
- `CFVehicleImportService`를 추가해 Current Definition Snapshot 전체 exact field를 Initial Import baseline으로 보존하고 normal `LegacyPinnedFields`와 Mount hidden `LegacySerializedFields`를 Registry metadata 기준으로 deterministic 분리했다.
- one-Mount fixture에서 hidden serialized exact 10 leaf가 별도 passthrough로 보존되고 normal Pin + serialized baseline이 source Definition 전체 exact field를 lossless cover함을 Automation으로 검증했다.
- Asset refs, wheel socket binding, Hardpoint identity/category/socket, Mount active six fields, Base/Gross Mass, MaxHealth, Default Defense/Destroyed FX refs, DestroyedFxSocketName, DriveState mode만 direct semantic candidate로 복사하고 ownership은 Legacy Pin이 계속 유지하도록 구현했다.
- Movement raw → Driving Feel/Profile authoritative inverse inference, Profile 자동 binding, measurement auto-adoption은 구현하지 않았다.
- group/field Adoption Preview는 immutable request clone에서 selected Pin만 virtual remove하고 persistent Recipe를 수정하지 않으며 identity/DerivedState/LegacySerialized/LegacyTechnical 제한을 enforce한다.
- Adoption Commit은 approved preview fingerprint fresh precondition, exact Pin removal, group AdoptedGroups, ManageState/revision 갱신만 `FScopedTransaction`으로 Recipe에 반영하며 stale Preview는 mutation 없이 거부한다.
- `BuildRecipeFingerprintFromSnapshot`을 기존 Recipe fingerprint authority의 public internal entry로 추가해 prospective Preview와 commit stale precondition이 같은 semantic hash를 사용하게 했다.
- final official Build `b201d87cd13f4987a0907e08c8f00a6c` PASS, targeted process `3b52a251ab244096b78bb872a06c0069` 20/20 PASS, result JSON SHA-256 `a84a02f81fb8f6cc1e023a6d4304cd7c4a3b854a175be7b19118870930ed5a02`를 current evidence로 연결했다.
- 첫 Foundation compile checkpoint `86062ce060994123b6e9f420d6a6b15e`도 PASS했으며 UHT 219초 지연 외 C++ 오류는 없었다.
- Runtime `UCFVehicleData` schema / Inventory / Fitting / `SCFVDAWizardTab` / Content Asset / existing Validator / Asset Reader 변경 0, Target Definition mutation 0, Apply / UI / CSV 구현 0을 유지했다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 checkpoint는 `DAUTH-P0-08H Apply Transaction Foundation`이다.

### v0.1.12 - 2026-08-17

- `DAUTH-P0-08F Definition Materializer / Validation Foundation`을 Technical PASS로 완료했다.
- `CFVehicleMaterializer`를 추가해 `SortedResolvedFields`를 `RF_Transient UCFVehicleData`로 복원하고 Stable-ID Hardpoint/Mount를 exact selector 집합으로 재구성했다.
- 모든 leaf write는 기존 `FCFVehicleFieldCodec::ImportValue`를 사용하며 identity 누락, type mismatch, readback/hash 불일치는 fail-closed Error로 처리한다.
- R15에서 기존 `UCFVDAValidator::ValidateVehicleData(Candidate, nullptr)`를 그대로 재사용하고 결과를 `FCFVehicleResolveResult.DefinitionValidation`에 연결했다. Runtime Validator source 수정은 0이다.
- Definition Error/Blocked는 Resolver 내부 실패와 분리해 R15 Completed + ResolveStatus Blocked로 유지하고, materialization/codec/hash 자체 실패만 R15 Failed + Error로 처리한다.
- Resolver `ResolvedDefinitionHash`와 Definition Snapshot이 `CFVehicleSnapshotBuilder`의 기존 hash format revision 1을 공유하도록 hash authority를 통합했다.
- transient Mount struct의 미소유 Legacy Serialized default는 Resolver-owned readback projection hash에 자동 포함하지 않고, `LegacySerializedPassthrough`가 존재할 때만 import하도록 경계를 보존했다.
- final official Build `e5d925c6531e4ae6ac58aa356e4078eb` PASS, targeted process `4b21d25b780c4a398de7a1b47c595c5e` 17/17 PASS, result JSON SHA-256 `d0297bdb26e9d929ba57fe1694ddc79f13ce2380e00f1fc2322daed0897b9b46`를 current evidence로 연결했다.
- 첫 compile checkpoint `bbcdf536476344f294b5edb7f62c2af6`는 R13 hash 교체부의 중복 brace 1개로 실패했으며 해당 단일 문법 오류를 교정한 뒤 final Build에서 닫았다. Materializer/Generated code 자체의 별도 compile error는 없었다.
- Runtime `UCFVehicleData` schema / Inventory / Fitting / `SCFVDAWizardTab` / Content Asset / Asset Snapshot Reader 변경 0, Apply / UI / CSV 구현 0을 유지했다.
- `DAUTH-P0-08`은 계속 `In Progress`이며 다음 checkpoint는 `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`이다.

### v0.1.11 - 2026-08-17

- `DAUTH-P0-08E Pure Resolver Foundation`을 Technical PASS로 완료했다.
- `FCFVehicleResolveRequest/Result`, Frozen R0~R16 stage record, deterministic candidate stack과 stage-independent source precedence를 실제 C++로 구현했다.
- Project Default / 5 Profile / Recipe / Feel Rule / Asset Socket / Measurement Proposal·Adoption / Legacy Pin / Legacy Serialized / Advanced Override / Derived Gate / Source Trace / Hash / Diff / Stale foundation을 snapshot-only 계산으로 연결했다.
- same-precedence 동일 field 충돌은 silent last-write-wins 없이 Error로 fail-closed한다.
- Registry v1.2.0은 새 정책을 만든 것이 아니라 Section 21.6/22.17에 이미 Frozen인 Project Default 및 보조 Source 허용 관계의 mask projection 누락만 교정했다.
- R15 transient materialization + `UCFVDAValidator`는 아직 구현하지 않고 `Deferred` stage로 명시했으며 다음 `DAUTH-P0-08F Definition Materializer / Validation Foundation`으로 분리했다.
- final official Build `db202f0797af41fe86a859609cb4dfd9` PASS와 targeted process `93d445f78cbe4eaabca1478eb6d27078` 15/15 PASS를 current evidence로 연결했다.
- Asset Snapshot Reader 재구현 0, Runtime `UCFVehicleData` / Inventory / Fitting / `SCFVDAWizardTab` / Content Asset 변경 0, Apply / UI / CSV 구현 0을 유지했다.
- `DAUTH-P0-08` 자체는 예상 Foundation 범위가 남아 있으므로 `In Progress`를 유지한다.

### v0.1.10 - 2026-08-17

- `DAUTH-P0-08D Asset Snapshot Reader Foundation`을 Technical PASS로 완료했다.
- `FCFVehicleAssetSnapshot`에 Chassis Object Path, requested socket facts, 4개 Wheel Object Path/local Bounds와 resolver-relevant fingerprint를 구현했다.
- 기존 Layout Capture의 Wheel socket fallback과 optional Hardpoint socket 의미를 read-only extraction으로 승계하되 required/optional 최종 판정은 Resolver validation으로 분리했다.
- Asset fingerprint는 whole package가 아니라 Section 22.13/22.27의 resolver-relevant extracted facts만 canonical hash하도록 구현했다.
- final official Build `780d30c4a44f48feb179f7f6da5480e1` PASS와 targeted process `ab2cd64308484ce0ae0e24cb1143ca33` 9/9 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting consumer, `SCFVDAWizardTab`, Content Asset 변경은 0이며 Resolver / Apply / UI / CSV는 구현하지 않았다.
- 다음 착수 지점을 `DAUTH-P0-08E Pure Resolver Foundation`으로 이동했다.

### v0.1.9 - 2026-08-17

- `DAUTH-P0-08C Immutable Snapshot Foundation`을 Technical PASS로 완료했다.
- Recipe Snapshot, 5 Profile Snapshot Set, Registry-expanded Definition Snapshot, 117-pattern Project Compatibility Default Snapshot을 `CarFight_ReEditor` Editor-only C++로 구현했다.
- Snapshot hash format revision 1의 UTF-8 canonical payload + deterministic MD5를 도입하고 Recipe/Typed Profile/Definition fingerprint 경계를 Frozen Section 22.27과 일치시켰다.
- Stable-ID array는 array index가 아니라 selector 기반 exact field entry로 확장하고 canonical path 정렬로 physical array reorder에 독립적인 Definition hash를 만들었다.
- Frozen Section 22.17 Main Dependency를 Registry `RequiredDependencies` stable key로 채우고 117 descriptor non-empty/duplicate-free 및 대표 dependency mapping을 Automation으로 검증했다.
- final official Build `6c81b2149de44d00a99554a44d2135aa` PASS와 targeted process `340050d2c911445da3634ebb92acc014` 8/8 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting consumer, `SCFVDAWizardTab`, Content Asset 변경은 0이며 Asset Snapshot Reader / Resolver / Apply / UI / CSV는 구현하지 않았다.
- 다음 착수 지점을 `DAUTH-P0-08D Asset Snapshot Reader Foundation`으로 이동했다.

### v0.1.8 - 2026-08-17

- `CF-FQ-038` Active 등록과 `DAUTH-P0-08 Implementation Foundation` 실제 Source 착수를 기록.
- P0-08A Editor-only Recipe + 5 Profile / Never-Cook / common types / Recipe duplicate identity를 Technical PASS로 기록.
- P0-08B Stable Field Path / Field Value Codec / current `UCFVehicleData` 117 leaf Registry bidirectional coverage를 Technical PASS로 기록.
- final official Build `5c745a31d19b448d9b1049877bc877ca` PASS와 targeted process `ab9de3e8c8cd410a8de0641178690dff` 4/4 PASS를 evidence로 연결.
- `Tools/RunDataAuthoringTests.ps1`를 재실행 가능한 Data Authoring targeted Automation entry로 기록.
- Runtime `UCFVehicleData`, Inventory/Fitting consumer, `SCFVDAWizardTab`, Content Asset 변경 0을 확인.
- `CF-FQ-034`는 `FIT-P0-07D`를 보존한 Paused로 전환하고 USER 주행감 PASS를 추정하지 않음.
- 현재 착수 지점을 `DAUTH-P0-08C Immutable Snapshot Foundation`으로 이동.

### v0.1.7 - 2026-08-17

- DAUTH-P0-07 Batch / Excel / CSV Contract 완료와 DAUTH-P0-08 Ready를 기록.
- Unreal Recipe/Profile을 Authoring SSOT, Spreadsheet를 Export Snapshot + Edit Staging으로 고정.
- P0 canonical interchange를 UTF-8 CSV + `.cfbatch.json` Manifest로 확정하고 Excel은 CSV editor로 지원, native XLSX는 P0 필수에서 제외.
- VehicleSummaryReport / ResolvedFieldReport read-only와 RecipeNumericEdit / ProfileNumericEdit editable 4 Dataset Kind를 고정.
- Recipe semantic scalar numeric과 Shared Profile typed scalar numeric만 external writable allowlist로 허용.
- Raw Resolved field / Asset Reference / Stable-ID Array / Complex Struct / Asset Derived / Measurement / Advanced Override를 Spreadsheet write에서 제외.
- Stable ColumnId / reserved metadata / Manifest baseline / blank/formula/read-only cell contract를 기록.
- Export Baseline / Edited / Current Unreal 3-way merge와 ownership-changed conflict를 고정.
- Spreadsheet Import → Prospective Preview → B1/B2 Authoring Source Commit → Fresh Resolve → 별도 B3 Definition Apply Plan으로 phase를 분리.
- B1/B2 source commit all-or-nothing, Target mutation 0, no-auto-save를 고정.
- Shared Profile B2는 모든 dependent Recipe affected-scope inventory와 prospective validation을 요구.
- B3는 global preflight 후 existing per-Vehicle `FCFVehicleApplyService`를 사용하며 per-Vehicle atomic / whole-batch non-atomic으로 고정.
- Stop On First Failure, prior success auto rollback 없음, exact Applied/Failed/NotStarted 결과, blind retry 금지를 기록.
- External Drift bulk resolution / global blind Batch Undo / Auto Save를 금지.
- AI Batch는 `FCFVehicleBatchService`를 사용하며 hidden single-Vehicle loop를 금지.
- P0-08 Batch automation 30개 추가 후보를 기록.
- 현재 착수 지점을 DAUTH-P0-08 Implementation Foundation으로 전환.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.6 - 2026-08-17

- DAUTH-P0-06 AI Authoring Contract 완료와 DAUTH-P0-07 Ready를 기록.
- AI를 공통 C++ Authoring Service의 typed client로 고정하고 UI/AI가 동일 Snapshot/Resolver/Diff/Validation/Apply를 사용하도록 확정.
- R0 Read / R1 normal Recipe write / R2 ownership·exceptional write / R3 Definition Apply risk class를 Roadmap checkpoint에 반영.
- exact payload/state/preview hash에 binding되는 approval과 Preview 이후 Apply approval 원칙을 고정.
- Read-before-write / expected fingerprint / typed error & mutation footprint / desired-state idempotency / no automatic write retry를 기록.
- persistent mutation 없는 prospective `PreviewRecipeChange` 흐름을 추가.
- Vehicle/Profile 조회, semantic intent write, New Record/Import/Adoption/Measurement/Override/Drift/Apply typed operation 범위를 기록.
- generic Raw `SetField(path,value)`, direct UObject/VehicleData write, validation bypass, force apply, auto-save를 금지.
- Shared Profile AI payload write를 P0-06에서 제외하고 existing Profile read/bind만 허용.
- AI global blind Undo를 금지하면서 모든 persistent mutation의 transaction undoability와 Apply atomic rollback은 유지.
- AI-only Source Type/Recipe provenance field를 금지하고 single Vehicle write scope로 P0-07 Batch Gate를 보호.
- P0-08 AI Contract automation 20개 추가 입력을 기록.
- 현재 착수 지점을 DAUTH-P0-07 Batch / Excel / CSV Contract로 전환.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.5 - 2026-08-17

- DAUTH-P0-05 Vehicle Authoring UX Design 완료와 DAUTH-P0-06 Ready를 기록.
- P0 Production UI를 C++ Slate 3-pane + Bottom Apply Bar Workspace로 확정.
- Vehicle Browser와 Overview / Recipe & Profiles / Assets & Layout / Driving Feel / Mounts & Defaults / Compare / Validation / Advanced navigation을 고정.
- Management / Sync / Validation 상태 축을 분리하고 Unmanaged / Legacy Imported / Partial / Managed / Stale / Shadow / External Drift / Preview Out Of Date UX를 반영.
- New Vehicle record creation과 Existing Definition Initial Import / Legacy Pin / Adoption workflow를 기록.
- Shared Profile edit 경계, Assets/Layout derived read-only, Measurement proposal/adoption, Driving Feel 4축 shadow state, stable-ID dependency guard를 기록.
- Pending Authoring Diff와 Reference Compare를 분리하고 persistent Changes / Source Trace / Issues / Sync Context Pane을 고정.
- External Drift 3-way review, Apply eligibility, warning-only Apply, no-auto-save, transaction/Undo behavior를 기록.
- Advanced Override typed/reason-required UX와 Raw DA escape hatch를 고정.
- P0-09 MVP UI와 P0-10 Wizard parity expansion 범위를 분리.
- 현재 착수 지점을 DAUTH-P0-06 AI Authoring Contract로 전환.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.4 - 2026-08-17

- DAUTH-P0-04 VDA Wizard Migration Design 완료와 DAUTH-P0-05 Ready를 기록.
- Current Target/Source/Compare/Validator/Layout/Driving Feel/Preview/Apply/Revert/Transaction/Raw DA Open 기능의 Reuse/Refactor/Replace/Retire 판정을 Roadmap checkpoint에 반영.
- Definition Validation, 117-leaf Authoring Diff, read-only Reference Compare를 서로 분리.
- Layout socket extraction 의미는 재사용하되 direct DA mutation shell은 새 Workspace에서 사용하지 않는 방향을 고정.
- 4 Driving Feel semantic 축과 named shortcut은 보존하고 hard-coded raw mapping/authoritative inverse는 retirement 대상으로 고정.
- `FScopedTransaction`은 재사용하되 ownership을 Apply/Recipe service로 중앙화.
- P0-08~10 coexistence, Legacy Current Wizard Freeze, managed-target guard를 기록.
- Deprecated Gate DG1~DG5와 Physical Delete Gate DEL1~DEL7을 공식 migration checkpoint로 고정.
- P0-10 자체는 Wizard 삭제 승인이 아니며 P0-11 Technical Validation + P0-12 USER Acceptance 이후 DEL Gate가 필요함을 명시.
- 현재 착수 지점을 DAUTH-P0-05 Vehicle Authoring UX Design으로 전환.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.3 - 2026-08-17

- DAUTH-P0-03 Vehicle Recipe / Profile / Resolver 상세 설계 완료와 DAUTH-P0-04 Ready를 기록.
- 기존 `CarFight_ReEditor` 재사용과 Runtime Authoring dependency 0 경계를 고정.
- Editor-only UDataAsset Recipe + 5 flat Profile 구현 구조를 Roadmap checkpoint에 반영.
- 117 leaf Field Registry / Resolver Map, Reflection Codec, immutable snapshot, pure Resolver, Source Trace, Stale/Drift, Stable-ID Diff 구조를 기록.
- 4축 Driving Feel을 Profile-authored Low/Neutral/High response와 opt-in mass context rule로 구현하는 방향을 고정.
- Existing Definition Legacy Pin → candidate → field/group Adopt → 별도 Apply migration 흐름을 기록하고 inverse intent inference 금지를 유지.
- TOCTOU-safe Apply precondition과 transaction rollback / no-auto-save 계약을 기록.
- P0-08 automation 입력 20개 범주를 설계 checkpoint로 고정.
- 현재 착수 지점을 DAUTH-P0-04 VDA Wizard Migration Design으로 전환.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.2 - 2026-08-17

- DAUTH-P0-01 126행 Vehicle Field Ownership Matrix 완료 상태를 Roadmap에 반영.
- DAUTH-P0-02 Authoring Contract Freeze PASS와 DAUTH-P0-03 Ready를 기록.
- Persistent Recipe와 Vehicle Base / Drivetrain / Handling / Performance / DriveState 5 Profile Domain을 Frozen Contract로 고정.
- Resolver / Source Tracking / Override / Legacy Pinned Import / Stale / Apply Transaction / Inventory-Fitting boundary를 Freeze 결과로 요약.
- Asset Derived P0 범위를 Layout/Hardpoint authoritative, wheel geometry measurement-assisted preview, COM/ChassisHeight/AdditionalOffset/DestroyedFxSocketName 자동 생성 제외로 고정.
- 현재 착수 지점을 DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design으로 전환.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.1 - 2026-08-17

- DAUTH-P0-00 Current Authoring Foundation Audit의 read-only 결과를 기록.
- Current `UCFVehicleData`, `UCFVDAValidator`, Pawn Apply, Vehicle DA Wizard, Quick Tune, Layout Capture의 실제 책임과 소비 경로를 확정.
- Inventory Item Instance → 강타입 Definition → Transient Fitting → `BuildFittingSnapshot()` 경계를 확인하고 Authoring 비소유 범위를 고정.
- Movement / WheelVisual / DriveState override flag의 서로 다른 Current 의미를 기록.
- `/Game` 전체 VehicleData 11개와 production Definition 2개를 구분하고, production Definition이 없는 Chassis Mesh 후보 7종을 기록.
- P0-01 Vehicle Field Ownership Matrix 작성에 필요한 입력 근거가 준비되었음을 기록.
- Source / Content Asset / Runtime 변경 없이 문서 증거만 갱신.

### v0.1.0 - 2026-08-17

- Data Authoring P0 Roadmap 최초 작성.
- P0-00~02를 코드 변경 없는 Audit / Classification / Architecture Freeze 구간으로 고정.
- P0-03~07을 상세 설계, P0-08 이후를 구현 구간으로 분리.
- VDA Wizard Migration, AI Authoring, Excel/CSV를 독립 Gate로 분리.
- 기존 Runtime / USER Pending 상태를 보호하는 검증 원칙을 추가.

---

## 19. Migration

- 이 Roadmap은 아직 Working이며 FeatureQueue / ActiveWork 상태를 자동 변경하지 않는다.
- 정식 착수 승인 전에는 현재 Active CF-FQ 작업의 우선순위를 대체하지 않는다.
- 기존 DataPlan 문서의 과거 Gate는 Historical Design Source로 참고하되 새 Authoring P0 Gate와 혼합하지 않는다.
- v0.1.1의 DAUTH-P0-00 완료 표기는 Data Authoring의 Project Feature 정식 승격을 의미하지 않으며 Plan Index / FeatureQueue / ActiveWork 등록을 요구하지 않는다.
- DAUTH-P0-01은 `DataAuthoringDesign.md` v0.2.0의 126행 Matrix로 완료되었으며 Source / Content Asset 변경은 없었다.
- v0.1.2부터 DAUTH-P0-02 Frozen Contract는 `DataAuthoringDesign.md v0.3.0` Section 21이 Authority다.
- v0.1.3부터 DAUTH-P0-03 구현 상세 Authority는 `DataAuthoringDesign.md v0.4.0` Section 22다.
- v0.1.4부터 DAUTH-P0-04 Wizard Migration Authority는 `DataAuthoringDesign.md v0.5.0` Section 23이다.
- v0.1.5부터 DAUTH-P0-05 Vehicle Authoring UX Authority는 `DataAuthoringDesign.md v0.6.0` Section 24다.
- v0.1.6부터 DAUTH-P0-06 AI Authoring Contract Authority는 `DataAuthoringDesign.md v0.7.0` Section 25다.
- v0.1.7부터 DAUTH-P0-07 Batch / Excel / CSV Contract Authority는 `DataAuthoringDesign.md v0.8.0` Section 26이다.
- v0.1.19부터 `CF-FQ-038`은 계속 Active이며 `DAUTH-P0-08 Implementation Foundation`은 P0-08A~M 전체 Technical PASS로 닫혔다. 다음 구현 Gate는 `DAUTH-P0-09 Vehicle Authoring MVP`다. Frozen 26.66~26.69 Batch idempotency/generic envelope는 첫 실제 external Batch transport/client 통합 전 follow-up으로 보존한다.
- P0-07 Complete로 P0-03~07 상세 설계 구간이 종료되었고 DAUTH-P0-08 Implementation Foundation이 실제 구현 단계로 전환됐다.
- P0-08 구현은 Section 22~26을 구현 입력으로 사용하며 Batch/AI 편의를 이유로 single-Vehicle ApplyService, Source ownership, Spreadsheet non-SSOT 계약을 우회하지 않는다.
- P0-08 실제 코드/Asset 착수 시 Project projection의 정식 등록 여부는 별도 승인으로 처리하며 현재 Plan Index / FeatureQueue / ActiveWork는 그대로 둔다.
- Current `SCFVDAWizardTab`은 P0-10 parity까지 보존하되 P0-04 이후 legacy freeze를 적용한다.
- Wizard physical delete는 P0-10 완료만으로 허용되지 않고 DG1~DG5 + DEL1~DEL7을 충족해야 한다.
- Plan Index / FeatureQueue / ActiveWork에는 아직 정식 등록하지 않는다.
