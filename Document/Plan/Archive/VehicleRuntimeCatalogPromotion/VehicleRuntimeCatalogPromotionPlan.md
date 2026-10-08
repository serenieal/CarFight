# Vehicle Builder Runtime Catalog Promotion Plan

- Version: 0.2.0
- Date: 2026-09-03
- Status: Completed / Historical + Archived Path / VRCP-P0-06 Current System Promotion Complete / G5 Physical Move Complete
- Feature: `CF-FQ-044 Vehicle Builder Runtime Catalog Promotion`
- Priority: P2
- Current Active Feature: `CF-FQ-039 Production UI Visual Rework` 유지
- Representative Plan: `Document/Plan/Archive/VehicleRuntimeCatalogPromotion/VehicleRuntimeCatalogPromotionPlan.md`
- Current System Owner: `Document/Systems/Vehicles/VehicleBuilder.md v1.4.1`
- Runtime Consumer: `CF-FQ-041 런타임 콘텐츠 적용 메뉴 / RuntimeApplyPlan.md v0.1.17 / RTA-P0-05 USER PASS / RTA-P0-06 Ready / persisted Wagon candidate handoff`
- Completed Upstream: `CF-FQ-043 Vehicle Builder 장비 장착점 Guidance UX` Done / current owner `Systems/Vehicles/VehicleBuilder.md v1.3.0`

---

## 1. 목적

Vehicle Builder에서 하나의 차량이 **Step 8 Technical Driving + exact USER Driving PASS**까지 완료되면, 사용자가 별도로 Runtime Test Catalog를 열어 같은 VehicleData를 다시 등록하지 않아도 되도록 Builder closure와 RuntimeApply allowlist 사이를 연결한다.

목표 흐름은 다음과 같다.

```text
Vehicle Builder managed VehicleData
→ Step 7 Final Review current Complete
→ Step 8 Technical Benchmark
→ active PIE에서 exact Target test-drive
→ USER 주행 PASS
→ exact USER acceptance 기록 성공
→ Default RuntimeTestCatalog exact membership 확인
→ 미등록이면 VehicleData 등록
→ 이미 등록됐으면 no-op success
→ Catalog package dirty
→ USER가 필요할 때 명시 저장
→ RuntimeApply에서 시연 차량으로 사용
```

이 Feature는 중앙 Vehicle Registry를 새로 만드는 작업이 아니다. 기존 Builder의 완료 Gate와 기존 `UCFRuntimeTestCatalogData::AllowedVehicleData` allowlist를 **얇은 Editor integration**으로 연결하는 작업이다.

---

## 2. 현재 계약 감사 결과 — VRCP-P0-00 PASS

### 2.1 Vehicle Builder USER PASS owner

현재 Builder Step 8의 USER Driving acceptance는 다음 경로가 소유한다.

```text
SCFVehicleBuilderTab::HandleAcceptUserDriving()
→ USER Yes/No confirmation
→ FCFVehicleBuilderVM::AcceptCurrentUserDriving()
→ current Technical Benchmark 존재 확인
→ current session PIE test-drive 준비 확인
→ current Recipe/Target 확인
→ exact Target DefinitionHash == benchmark ExpectedTargetDefinitionHash 확인
→ Recipe.BuilderDrivingAcceptanceReceipt에 exact TargetVehicleDataPath + TargetDefinitionHash persistent 기록
→ AcceptedBenchmarkRunId는 진단용으로 기록
→ legacy EditorPerProject local token도 compatibility용으로 병행 기록
→ RebuildStepStates()
```

따라서 본 Feature가 말하는 **USER PASS**는 대화상의 일반 승인 문구가 아니라, Builder Step 8에서 `AcceptCurrentUserDriving()`이 성공해 current Recipe의 `BuilderDrivingAcceptanceReceipt`가 exact Target VehicleData path + Target DefinitionHash에 persistent binding된 상태를 의미한다. Benchmark RunId는 진단용이며 같은 DefinitionHash의 새 RunId가 USER PASS authority를 바꾸지 않는다.

CF-FQ-044는 이 receipt를 복제하는 별도 Catalog acceptance field/state를 만들지 않는다. 새로운 `bUserPassed` Runtime bool, VehicleData 필드, BuilderVM Catalog cache/state도 만들지 않는다.

### 2.2 Runtime Catalog owner

현재 RuntimeApply 후보 목록은 프로젝트 전체 VehicleData 자동 검색이 아니라 다음 explicit allowlist가 소유한다.

```text
UCFRuntimeTestSettings::DefaultCatalog
→ UCFRuntimeTestCatalogData
→ AllowedVehicleData
→ AllowedEquipmentPresetData
```

현재 계약:

- Catalog는 Packaged Runtime에서도 사용할 hard reference 목록이다.
- 프로젝트 전체 Asset Registry enumeration은 사용하지 않는다.
- Tests/Legacy VehicleData를 자동으로 노출하지 않는다.
- 새 시연 대상은 Catalog에 explicit registration한다.
- RuntimeApply는 Catalog membership을 authorization boundary로 사용한다.

본 Feature는 이 allowlist 방식을 유지한다.

### 2.3 현재 관측된 VehicleData inventory

2026-09-02 fresh AssetDump 기준 `/Game/CarFight` 아래 `CFVehicleData`는 12개이며, 일반/Authoring/fitting-ready 차량 4개와 Tests/Legacy 8개가 분리되어 있다.

현재 Default Runtime Catalog의 차량 4개가 적은 이유는 누락이 아니라 **Tests/Legacy 자동 노출을 금지한 explicit allowlist 정책** 때문이다.

따라서 해결책은 전체 VehicleData 자동 검색이 아니라, **Builder에서 최종 USER PASS한 managed VehicleData를 explicit allowlist에 자동 승격**하는 것이다.

### 2.4 선행 Feature 상태

```text
CF-FQ-040 Guided Vehicle Builder
= Done / 재오픈 금지

CF-FQ-042 Vehicle Builder 신규 차량 생성 UX
= Done / USER PASS / Historical + Retained Path
= 생성 즉시 Catalog 등록은 기존 범위에 없음
= 재오픈 금지

CF-FQ-041 Runtime Content Apply
= RTA-P0-05 USER PASS / Closed
= RuntimeApply regression 13/13 PASS
= RTA-P0-06 Packaged Demo Ready

CF-FQ-043 Vehicle Builder Mount Guidance
= Done / USER Acceptance PASS / Current System Promotion Complete
= current owner Systems/Vehicles/VehicleBuilder.md v1.3.0
= Historical + Retained Path
```

본 Feature는 위 완료/병렬 계약을 재구현하지 않는다.

---

## 3. 핵심 설계 결정

### 3.1 등록 시점은 VehicleData 생성 시점이 아니다

다음은 금지한다.

```text
+ 새 차량 만들기 승인
→ 즉시 Runtime Catalog 등록
```

신규 VehicleData는 생성 직후 Mesh, Socket, Physics, Gameplay, DefinitionApply, Driving 검증이 미완성일 수 있다.

따라서 canonical trigger는 다음으로 고정한다.

```text
Step 8 exact USER Driving PASS 기록 성공
→ Runtime Catalog Promotion 시도
```

### 3.2 USER PASS와 Catalog Promotion은 서로 다른 결과다

USER Driving PASS는 차량의 주행 acceptance이고, Catalog 등록은 개발/시연 목록 관리 동작이다.

따라서 Catalog mutation 실패 때문에 이미 성립한 USER Driving PASS를 취소하거나 rollback하지 않는다.

예:

```text
USER Driving PASS = 성공
Catalog Load = 실패

결과:
- Recipe BuilderDrivingAcceptanceReceipt 유지
- Step 8 기존 acceptance 의미 유지
- Catalog Promotion = Pending/Failed로 별도 표시
- 재시도 가능
```

이 분리는 기존 Builder Step 8 완료 계약을 재정의하지 않기 위한 필수 보호 조건이다.

### 3.3 Runtime code가 아니라 전용 Editor Promotion Service + Builder Tab orchestration이 소유한다

Catalog 등록은 Builder authoring workflow의 closure side effect이므로 `CarFight_ReEditor`가 소유한다. CF-FQ-043는 2026-09-03 Done으로 닫혔고 BuilderVM/BuilderTab 4파일의 fresh worktree diff 0을 확인했다. CF-FQ-044는 기존 설계대로 **BuilderVM persistent mutation owner를 추가하지 않는다.**

P0 exact 분배:

```text
전용 C++ Editor Promotion Service
- Default Catalog raw resolve / diagnostic
- exact persistent VehicleData validation
- Catalog full pre-validation
- membership 검사
- transaction / rollback / post-validation / readback
- add / already-registered / failure result
- package dirty
- no auto save

SCFVehicleBuilderTab
- 기존 HandleAcceptUserDriving() confirmation 문구 확장
- AcceptCurrentUserDriving() 성공 이후 orchestration
- HasCurrentUserDrivingAcceptance() 재확인
- GetRecipe()->TargetVehicleData를 exact promotion target으로 전달
- USER PASS 결과와 Promotion 결과를 분리 표시
- 실패 시 explicit retry UX

FCFVehicleBuilderVM
- 기존 public read/acceptance API만 재사용
- `BuilderDrivingAcceptanceReceipt`를 canonical USER PASS authority로 유지
- CF-FQ-044 신규 persistent field / mutation 함수 / Catalog cache 추가 금지

Blueprint
- 신규 책임 없음
```

RuntimeApply Runtime module의 Vehicle/Equipment apply service는 수정하지 않는다. RuntimeApply UI는 Catalog option cache의 change-aware 동기화만 bounded 수정할 수 있다.

### 3.4 Default Catalog는 Settings 경유로 raw resolve하고 오류를 분리한다

Catalog Asset 경로를 Builder 코드에 새로 하드코딩하지 않는다.

```text
GetDefault<UCFRuntimeTestSettings>()
→ DefaultCatalog soft reference 존재 확인
→ LoadSynchronous raw load
→ raw object/class validity 확인
→ ValidateRuntimeTestCatalog()
```

현재 RuntimeApply와 같은 `UCFRuntimeTestSettings::DefaultCatalog` source-of-truth를 사용한다.

기존 `UCFRuntimeTestSettings::LoadDefaultCatalog()`는 `설정 없음 / load 실패 / Catalog invalid`를 모두 `nullptr`로 합치므로 **Promotion Service의 sole resolver로 사용하지 않는다.** 기존 Runtime helper 자체는 변경하지 않는다.

필수 오류 분류:

```text
Settings/soft reference 없음 또는 raw load 실패
→ CatalogUnavailable

Catalog UObject load 성공 + full validation 실패
→ CatalogInvalid
```

### 3.5 exact identity + full Catalog validation + idempotent add

등록 기준은 사람이 읽는 Vehicle ID 문자열이나 Asset 이름이 아니라 **exact persistent UCFVehicleData object identity**다.

판정 순서는 다음으로 고정한다.

```text
1. Target validation
2. Default Catalog raw resolve
3. Catalog 전체 pre-validation
   - invalid면 CatalogInvalid / mutation 0
4. AllowedVehicleData exact membership 검사
   - 존재하면 AlreadyRegistered / mutation 0
5. transaction 시작
6. exact Target Add
7. Catalog 전체 post-validation
8. exact membership/readback 확인
9. 성공이면 Registered
10. post-validation/readback 실패면 rollback + MutationFailed
```

따라서 Catalog가 invalid한 상태에서는 해당 VehicleData가 배열에 이미 존재하더라도 `AlreadyRegistered`로 성공 처리하지 않는다.

동일 Asset의 중복 등록은 허용하지 않는다.

### 3.6 persistent VehicleData만 허용하고 path denylist는 만들지 않는다

다음은 등록 대상이 아니다.

```text
null / invalid UObject
Runtime transient duplicate
PIE transient VehicleData
RF_Transient object
TransientPackage object
valid persistent object path가 없는 object
/Game persistent package가 아닌 target
Mesh-only Candidate
아직 VehicleData가 생성되지 않은 New Vehicle entry
```

Promotion 대상은 Builder가 current exact Recipe/Target으로 관리하고 USER Driving acceptance가 exact binding된 persistent VehicleData여야 한다.

`Tests` 또는 `_Legacy` 문자열을 경로 denylist로 하드코딩하지 않는다. P0 금지사항은 **Tests/Legacy 자동 검색·자동 등록**이며, 향후 USER가 Builder에서 명시적으로 검증하고 exact USER PASS한 persistent 차량까지 문자열 경로만으로 차단하지 않는다.

### 3.7 자동 저장 금지 + internal rollback dirty 복원

정상 등록 성공 시:

```text
FScopedTransaction
Catalog->Modify()
AllowedVehicleData.Add(...)
PostEditChange()
post-validation/readback
Catalog package dirty
```

까지 수행한다.

다음은 금지한다.

```text
SavePackage
SaveAsset
Save All
VehicleData 자동 저장
Recipe 자동 저장
```

Transaction 전 Catalog package dirty 상태를 기억하고, **service 내부 실패로 rollback할 때만** 배열/notification/dirty를 이전 상태로 exact 복원한다.

정상 등록 뒤 USER가 Unreal Undo/Redo를 수행하는 경우는 표준 Editor transaction semantics를 따른다. USER Undo까지 이전 dirty=false 상태로 강제 복원하는 custom logic은 만들지 않는다.

USER가 저장 전이라면 Builder UI에 **등록됨 / Catalog 변경사항 미저장**을 구분해 표시할 수 있어야 한다.

### 3.8 Undo / Redo + fresh membership readback

Catalog 등록은 Editor transaction으로 수행한다.

기대 동작:

```text
등록
→ Undo
→ exact VehicleData membership 제거
→ USER Driving PASS token은 유지

Redo
→ membership 복원
```

USER acceptance와 Catalog membership의 undo domain을 억지로 하나로 합치지 않는다.

Builder의 `Runtime Demo Catalog` 상태는 마지막 terminal message를 authority로 캐시하지 않는다. 필요할 때 Default Catalog의 **현재 exact membership을 fresh read**해 `등록됨 / 미등록 / Catalog 오류`를 파생한다. 따라서 Undo/Redo 뒤 별도 거대한 transaction listener 없이도 상태가 stale하지 않아야 한다.

### 3.9 Promotion 결과와 USER PASS 결과는 별도 typed outcome이다

Promotion service 최소 결과 상태는 다음을 구분한다.

```text
Registered
AlreadyRegistered
InvalidTarget
CatalogUnavailable
CatalogInvalid
MutationFailed
```

결과에는 최소한 다음 진단을 포함한다.

```text
Status
Message
bCatalogChanged
bPackageDirty
bSavePerformed = false
```

`Registered`와 `AlreadyRegistered`만 promotion success다. USER Driving PASS 자체의 성공 여부는 이 결과에 흡수하지 않는다.

### 3.10 자동 제거/rename 동기화는 P0 Scope Out

P0는 **USER PASS → add promotion**만 다룬다.

다음은 별도 후속 문제다.

```text
VehicleData 삭제 시 자동 Catalog 정리
VehicleData rename 시 별도 registry migration
USER PASS 취소 시 자동 unregister
Deprecated 차량 lifecycle
여러 Runtime Catalog profile
중앙 Vehicle/Equipment Registry
대량 backfill
```

UObject hard reference의 정상 rename fix-up과 별개로, 새 관리 시스템을 만들지 않는다.

---

## 4. 사용자 UX 계약

### 4.1 정상 성공 + confirmation disclosure

Step 8 USER PASS confirmation에서 Yes를 선택하고 기존 acceptance가 성공하면 Catalog promotion을 즉시 시도한다.

기존 confirmation에는 이번 persistent side effect를 명시한다.

```text
이 exact 차량 상태를 USER Driving PASS로 승인하시겠습니까?

승인하면 기본 Runtime Demo Catalog 등록도 함께 시도합니다.
Catalog는 자동 저장하지 않습니다.
```

두 번째 확인창은 추가하지 않는다. 한 번의 Yes가 `USER Driving acceptance + Catalog promotion 시도`를 명확히 승인하도록 한다.

권장 상태 예:

```text
USER Driving PASS: PASS
Runtime Demo Catalog: 등록됨
저장 상태: Catalog 변경사항 미저장
```

이미 등록된 차량이면:

```text
USER Driving PASS: PASS
Runtime Demo Catalog: 이미 등록됨
```

`Runtime Demo Catalog` 상태는 last-message cache가 아니라 fresh membership readback에서 파생한다.

### 4.2 실패

Catalog load/validation 또는 target validation이 실패하면 USER PASS 성공 메시지를 숨기지 않는다.

예:

```text
USER Driving PASS: PASS
Runtime Demo Catalog: 등록 실패
원인: Default Runtime Test Catalog validation 실패
[Runtime Catalog 등록 재시도]
```

UI는 `CatalogUnavailable / CatalogInvalid / InvalidTarget / MutationFailed`를 구분해 실제 원인을 표시한다. promotion 실패 때문에 Step 8 USER PASS token이나 Complete 의미를 rollback하지 않는다.

### 4.3 재시도

P0에는 최소 explicit retry 경로를 제공한다.

재시도는:

- current exact USER Driving acceptance가 여전히 유효해야 한다.
- `HasCurrentUserDrivingAcceptance()`가 true여야 한다. persistent Receipt를 Tab에서 직접 읽어 이 guard를 우회하지 않는다.
- current `GetRecipe()->TargetVehicleData`가 valid persistent target이어야 한다.
- same Target path + DefinitionHash의 persistent Receipt가 유효하면 새 USER Driving PASS를 다시 요구하지 않는다.
- Editor 재기동 후 current benchmark result가 없거나 stale이면 기존 Step 8 authority에 따라 benchmark state를 먼저 복원/재실행해야 할 수 있지만, DefinitionHash가 같다면 USER Driving PASS 자체를 다시 입력하지 않는다.
- Catalog full pre-validation을 다시 수행한다.
- 이미 valid Catalog에 등록됐으면 `AlreadyRegistered` no-op success다.

### 4.4 기존 차량

본 Feature 도입 전에 이미 USER PASS된 차량을 background scan해서 일괄 등록하지 않는다.

현재 Builder target에 exact USER acceptance가 복원되어 있고 Catalog membership이 없으면 retry/action으로 승격할 수 있다.

---

## 5. RuntimeApply 연계 계약

### 5.1 RuntimeApply authorization은 유지

`FCFRuntimeVehicleApplyService`의 Catalog membership validation을 약화하지 않는다.

```text
Builder USER PASS
→ Catalog에 exact persistent VehicleData 등록

RuntimeApply
→ 기존 Catalog authorization
→ 기존 transient Vehicle apply
```

즉 Builder가 RuntimeApply의 authorization contract를 우회하지 않는다.

### 5.2 RuntimeApply Catalog option cache는 change-aware 동기화가 P0 필수다

Current Source에서 `UCFVehicleDebugPanelWidget`은 `UCFRuntimeApplyWidget`을 한 번 생성해 캐시하고 재사용한다. 따라서 단순 Section 재진입만으로 Vehicle ComboBox가 새 Catalog 내용을 자동 반영하지 않는다.

P0는 RuntimeApply에 bounded change-aware synchronization을 포함한다.

```text
RefreshRuntimeApplyState()
→ Default Catalog current object 확인
→ AllowedVehicleData와 cached VehicleOptionDataArray exact sequence 비교
   ├─ 같음: rebuild 0
   └─ 다름: RebuildVehicleOptions() exactly once
→ AllowedEquipmentPresetData와 cached EquipmentOptionDataArray exact sequence 비교
   ├─ 같음: rebuild 0
   └─ 다름: RebuildEquipmentOptions() exactly once
```

중요 보호 조건:

```text
일반 Tick마다 ClearOptions() 금지
Catalog 실제 변경이 없으면 ComboBox rebuild 0
Catalog promotion/Undo/Redo로 actual array가 달라졌을 때만 rebuild
```

이 계약은 v1.0.1에서 교정한 Equipment dropdown 즉시 닫힘 회귀를 다시 만들지 않으면서 같은 세션의 cached RuntimeApply Widget에도 새 차량을 반영하기 위한 필수 조건이다.

전역 event bus는 만들지 않는다.

### 5.3 Packaged Demo는 Catalog explicit Save 뒤에만 handoff한다

Promotion 성공은 memory/package dirty 상태의 authoring success이지 packaged readiness와 동일하지 않다.

```text
Promotion Success
→ Catalog package dirty
→ USER explicit save
→ persisted Catalog membership evidence 확인
→ RTA-P0-06 Packaged Demo candidate
```

등록된 VehicleData는 **저장된** Default Catalog hard reference chain을 통해 RTA-P0-06 Packaged Demo 후보가 될 수 있다.

다만 Packaged Runtime 동작 자체의 최종 owner는 계속 `CF-FQ-041 / RTA-P0-06`이다.

본 Feature는 RTA-P0-06을 복제하지 않고 **Builder-produced vehicle이 Default Catalog에 들어오는 producer-side contract와 persisted handoff precondition**만 소유한다.

---

## 6. 구현 단계

### VRCP-P0-00 — Current Contract Audit

Status: **PASS**

확정 사항:

- Step 8 exact USER Driving PASS owner 확인.
- Default Runtime Catalog/Settings owner 확인.
- Catalog가 explicit hard-reference allowlist임을 확인.
- Tests/Legacy 자동 검색 금지 확인.
- CF-FQ-042 생성 UX에 Catalog 연계가 없음을 확인.
- CF-FQ-041 RTA-P0-05 USER PASS와 RuntimeApply 13/13 regression 보존.
- Editor-only integration으로 해결 가능함을 확인.

### VRCP-P0-01 — Detailed Promotion Contract Design Review

Status: **Design PASS / Closed**

2026-09-02 Source 기반 설계감사에서 P1 5건 + P2 3건을 확인했고 v0.1.1에 다음 교정을 반영했다.

```text
P1-1 cached RuntimeApply의 change-aware Catalog option sync를 P0 필수로 승격
P1-2 LoadDefaultCatalog sole resolver 금지, raw resolve로 Unavailable/Invalid 분리
P1-3 Catalog full pre-validation → membership → transaction → post-validation/readback 순서 고정
P1-4 USER Driving confirmation에 Catalog promotion/no-auto-save side effect 명시
P1-5 CF-FQ-043 보호를 위해 BuilderVM 신규 mutation/state 추가 금지, Tab + 전용 Service owner 고정
P2-1 Catalog status를 fresh membership readback에서 파생해 Undo/Redo stale 차단
P2-2 internal rollback dirty exact restore와 normal USER Undo semantics 분리
P2-3 Promotion Success와 Packaged Ready를 분리하고 explicit Save/persisted evidence gate 추가
```

2026-09-02 v0.1.2 Correction Re-review에서 current Source와 다시 대조해 다음을 확인했다.

```text
- UCFRuntimeTestSettings::DefaultCatalog public soft reference와 UCFRuntimeTestCatalogData::ValidateRuntimeTestCatalog()가 raw resolve / Invalid 분리에 필요한 API를 제공한다.
- AllowedVehicleData / AllowedEquipmentPresetData는 EditAnywhere public hard-reference 배열이라 Editor-only Service가 exact identity add/readback을 소유할 수 있다.
- CarFight_ReEditor는 이미 CarFight_Re + UnrealEd에 의존하므로 Catalog runtime types와 FScopedTransaction 사용을 위해 Build.cs dependency를 새로 만들 필요가 없다.
- FCFVehicleBuilderVM public API에 AcceptCurrentUserDriving(), HasCurrentUserDrivingAcceptance(), GetRecipe()가 그대로 존재하고 Recipe.TargetVehicleData가 persistent exact target owner다.
- CF-FQ-043 current diff는 CFVehicleBuilderVM.h/.cpp와 CFVehicleBuilderTab.h/.cpp를 수정 중이지만 Tab 변경은 Step 3 Hardpoint UI/handler hunks이며 Step 8 HandleAcceptUserDriving()/CanAcceptUserDriving() hunk는 건드리지 않는다.
- 따라서 CF-FQ-044는 BuilderVM no-touch를 유지하고 BuilderTab의 Step 8 exact hunk만 fresh-read/minimal patch하는 방식으로 병렬 충돌을 분리할 수 있다.
- UCFRuntimeApplyWidget은 RuntimeCatalog와 VehicleOptionDataArray/EquipmentOptionDataArray를 별도 cache하고 RefreshRuntimeApplyState()가 반복 호출되므로 exact sequence compare 뒤 actual change에서만 rebuild하는 bounded sync seam이 존재한다.
- 기존 CFBuilderEvidenceRefresh 등 Editor authoring code에 FScopedTransaction + Modify + PostEditChange + pre-dirty restore + Transaction.Cancel 패턴이 이미 있어 internal rollback/no-save 설계와 충돌하지 않는다.
- Step 8은 saved Recipe/Target + exact benchmark + PIE test-drive를 선행하므로 USER PASS 후 persistent Target promotion 계약과 일치한다.
```

추가로 re-review에서 테스트 경계를 현실화했다.

```text
- FMessageDialog Yes/No 자체를 자동 클릭하기 위해 production test hook을 추가하지 않는다.
- No branch / AcceptCurrentUserDriving 실패 branch에서 Promotion Service call이 존재하지 않는지는 focused source/integration review로 확인하고 USER Gate에서 confirmation UX를 검증한다.
- Promotion Service의 Registered/AlreadyRegistered/Unavailable/InvalidTarget/InvalidCatalog/no-save/Undo는 isolated Catalog fixture automation으로 검증한다.
- post-validation/readback failure rollback은 current deterministic Add 경로에서 자연 발생시키기 어려우므로 production-only fault injection을 만들지 않는다. rollback 코드는 source audit 대상으로 유지하고, 재사용 가능한 기존 failure seam이 구현 중 자연스럽게 존재할 때만 자동화한다.
- RuntimeApply sync는 Product Catalog를 테스트에서 수정하지 않고 isolated/test-owned Catalog state로 검증한다.
```

P1 5건/P2 3건은 모두 current Source와 양립하며 신규 blocker가 없다. **VRCP-P0-01은 Design PASS로 닫고 VRCP-P0-02 구현 진입을 허용한다.**

### VRCP-P0-02 — Editor Promotion Service

P0-01 correction 기준 exact owner 후보:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/
  CFVehicleCatalogPromoService.h
  CFVehicleCatalogPromoService.cpp
```

예상 책임:

```text
ResolveDefaultRuntimeCatalogRaw()
ValidatePromotionTarget()
ReadPromotionMembership()
PromoteVehicleToRuntimeCatalog()
BuildPromotionStatus()
```

Service는 BuilderVM을 소유하거나 수정하지 않으며 persistent `UCFVehicleData*`와 Settings/Catalog만 입력으로 받는다.

필수 결과 분류:

```text
Registered
AlreadyRegistered
InvalidTarget
CatalogUnavailable
CatalogInvalid
MutationFailed
```

`Registered / AlreadyRegistered`만 success이며 `bSavePerformed=false`를 항상 보존한다.

#### 2026-09-02 VRCP-P0-02 Implementation Checkpoint

구현 자체는 완료했다.

```text
신규 Editor-only owner
- CFVehicleCatalogPromoService.h/.cpp
- CFVehicleCatalogPromoTests.cpp
- Tools/RunCatalogPromoTests.ps1

구현 계약
- persistent /Game VehicleData fail-closed validation
- raw DefaultCatalog resolve와 CatalogUnavailable/CatalogInvalid 분리
- full Catalog pre-validation 선행
- exact object identity membership / idempotent add
- FScopedTransaction + Modify + PostEditChange
- full post-validation + exact membership/count readback
- internal failure 시 exact AllowedVehicleData + pre-dirty restore + Transaction.Cancel
- SavePackage / SaveAsset / SaveAll 호출 0
- bSavePerformed=false
```

검증 evidence:

```text
1차 official Editor build
- Service 본체 compile PASS
- Test include 누락 1건 교정 후 official Editor build PASS (exit 0)

1차 focused Automation
- RegisterIdempotent PASS
- InvalidCatalogBeforeMembership PASS
- UndoRedoFreshMembership PASS
- UnavailableAndInvalidTarget FAIL
  원인: Config=Game test Settings가 CDO DefaultCatalog를 상속한 fixture 오류
  교정: test-owned Settings에서 DefaultCatalog.Reset() 명시

교정본 compile evidence
- CFVehicleCatalogPromoTests.cpp compile PASS
- 전체 Editor link는 protected parallel CF-FQ-043 CFVehicleBuilderVM.cpp의
  TArray::CountByPredicate compile error(C2039)로 차단됨
```

따라서 **P0-02 구현은 Complete지만 Technical PASS는 아직 선언하지 않는다.** CF-FQ-043 보호 조건 때문에 BuilderVM을 이번 Gate에서 교정하지 않았고, 해당 병렬 compile blocker가 해소된 뒤 corrected focused Automation exact4를 재실행해 4/4 PASS를 받아야 P0-02를 닫고 P0-03으로 전진한다.

### VRCP-P0-03 — Builder Step 8 Integration / Retry UX

Status: **Technical PASS / Closed**

기존 USER Driving PASS 성공 뒤에만 `SCFVehicleBuilderTab`이 promotion을 orchestration한다.

Canonical seam:

```text
HandleAcceptUserDriving()
→ confirmation Yes
→ ViewModel->AcceptCurrentUserDriving()
→ persistent BuilderDrivingAcceptanceReceipt write 성공
→ ViewModel->HasCurrentUserDrivingAcceptance() == true 재확인
→ Recipe = ViewModel->GetRecipe()
→ Target = Recipe->TargetVehicleData.LoadSynchronous()
→ Promotion Service 호출
→ fresh Catalog membership/status readback
→ USER PASS / Promotion 결과 분리 표시
```

Retry seam:

```text
HandleRetryRuntimeCatalogPromotion()
→ ViewModel->HasCurrentUserDrivingAcceptance() == true
→ current Recipe/Target resolve
→ Promotion Service 호출
→ fresh membership/status readback
```

Tab은 Catalog 등록 완료 여부를 별도 persistent/transient SSOT로 캐시하지 않는다.

보호 조건:

- USER가 No를 누르면 Catalog mutation 0.
- `AcceptCurrentUserDriving()` 실패면 Catalog mutation 0.
- `HasCurrentUserDrivingAcceptance()` 재확인 실패면 Catalog mutation 0.
- stale benchmark/Target이면 Catalog mutation 0.
- promotion 실패가 USER PASS를 rollback하지 않음.
- explicit retry 제공.
- Existing Builder Step 8 Complete semantics 유지.
- `BuilderDrivingAcceptanceReceipt`의 Target path + DefinitionHash authority를 유지하고 Benchmark RunId를 Catalog identity로 사용하지 않음.
- CF-FQ-044 때문에 `CFVehicleBuilderVM.h/.cpp`에 신규 Catalog state/mutation API를 추가하지 않음.

2026-09-03 구현/검증 결과:

```text
BuilderTab
- confirmation에 persistent Receipt + Catalog promotion/no-auto-save disclosure 추가
- AcceptCurrentUserDriving() 성공 뒤에만 promotion 호출
- USER PASS와 Promotion 결과 분리 표시
- fresh membership 기반 Runtime Demo Catalog 상태 표시
- explicit retry 추가
- invalid Catalog containing target을 registered success로 표시하지 않음

RuntimeApply
- AllowedVehicleData / AllowedEquipmentPresetData valid-entry filtered exact sequence change-aware sync
- append/remove/reorder 감지
- raw null/invalid entry는 cache와 동일하게 제외해 반복 rebuild를 유발하지 않음
- 변화 없는 refresh에서 options rebuild 0
- 기존 Equipment dropdown lifecycle 보존

보호 범위
- CFVehicleBuilderVM.h/.cpp mutation 0
- RuntimeApply authorization/service 재구현 0
- Product Default Catalog test mutation/save 0
- auto Save 0
```

Technical evidence:

```text
Official Editor build
- 35958c7c940f4741917b5ed3cf54b221
- PASS / Exit Code 0
- final same-refresh Equipment rebuild ordering correction 포함

Promotion Service focused
- 4/4 PASS

RuntimeApply full regression
- 14/14 PASS
- 신규 CF_FQ_044.VRCP_P0_03.CatalogOptionSync PASS
- mid-review P1 교정 뒤 14/14 재PASS
- CatalogOptionSync는 Product Default Catalog 개수와 독립된 test-owned transient Vehicle/Equipment fixture 사용
- raw null entry repeated refresh 안정성 포함
- 기존 RTA-P0-01~05 전부 PASS

Builder affected focused
- RunBuilderTransTests PASS
- BuilderStep8Driving PASS
- persistent USER Driving receipt semantics 보존

Source guards
- confirmation No / AcceptCurrentUserDriving failure는 promotion 이전 return
- HasCurrentUserDrivingAcceptance 재확인 뒤에만 service 호출
- SavePackage / SaveAll 신규 호출 0
```

위 evidence로 VRCP-P0-03을 Technical PASS로 닫는다. P0-04 항목 중 service/runtime/builder 회귀 상당수는 선행 PASS했지만 CF-FQ-042 creation regression 등 전체 matrix closure는 아직 수행하지 않았으므로 VRCP-P0-04 전체 PASS로 확대하지 않는다.

### VRCP-P0-04 — Focused / Affected Regression

Status: **Technical PASS / Closed**

최소 자동화 범위:

1. fresh exact persistent VehicleData + valid Catalog → Registered
2. second call → AlreadyRegistered / duplicate 0
3. invalid Catalog에 target이 이미 존재 → CatalogInvalid, AlreadyRegistered 금지
4. DefaultCatalog 없음/load 실패 → CatalogUnavailable
5. null/transient/TransientPackage/non-persistent target → InvalidTarget
6. normal Undo → membership 제거 / USER PASS 보존
7. Redo → membership 복원
8. no auto Save / bSavePerformed=false
9. fresh membership status가 Undo/Redo를 따라감
10. cached RuntimeApply Widget에서 Catalog array 변경 후 first refresh에 새 Vehicle option 반영
11. unchanged Catalog 후속 refresh에서 option identity/count/selection 유지 + 불필요 rebuild 경로 0 source audit
12. Equipment dropdown v1.0.1 lifecycle regression 보존
13. CF-FQ-040 Builder Step 8 regression 보존
14. CF-FQ-042 creation regression 보존
15. CF-FQ-041 CatalogContract + RuntimeApply full regression 보존
16. CF-FQ-043 affected regression은 실제 touched overlap이 있을 때만 실행

Integration/source guard 범위:

```text
USER confirmation No → Promotion Service call 0
AcceptCurrentUserDriving() failure → Promotion Service call 0
HasCurrentUserDrivingAcceptance() recheck failure → Promotion Service call 0
post-validation/readback defensive rollback → exact array + pre-dirty restore + Transaction.Cancel
```

`FMessageDialog` 자동 클릭이나 deterministic Add 뒤 강제 failure만을 위해 production fault-injection API를 추가하지 않는다.

Promotion Service 기본 자동화는 transient/test-owned Catalog + persisted read-only Vehicle/Equipment fixture를 사용해 Default Product Catalog에 테스트 찌꺼기를 남기지 않는다. RuntimeApply cache sync 테스트도 Product Catalog mutation 없이 isolated fixture/object state로 검증한다.

2026-09-03 P0-04 closure evidence:

```text
Matrix 1~9
- Promotion Service focused 4/4 PASS evidence 재사용
- Registered / AlreadyRegistered / CatalogInvalid / CatalogUnavailable / InvalidTarget
- Undo/Redo fresh membership / no-auto-save

Matrix 10~12, 15
- RuntimeApply full regression 14/14 PASS evidence 재사용
- CatalogOptionSync PASS
- valid-entry filtered append/remove/reorder/null repeated refresh 안정성
- Equipment dropdown lifecycle 보존
- CatalogContract + RuntimeApply RTA-P0-01~05 전부 PASS

Matrix 13
- CF-FQ-040 Builder affected focused / BuilderStep8Driving PASS evidence 재사용

Matrix 14
- CF-FQ-042 affected prefix 3/3 PASS
  - VBCUX_P0_02.RecordCreation
  - VBCUX_P0_03.Naming
  - VBCUX_P0_04.FocusedRegression

Matrix 16
- CF-FQ-044가 CFVehicleBuilderTab을 실제 수정했으므로 CF-FQ-043 touched overlap = Yes
- CF-FQ-043 affected prefix 5/5 PASS
  - VMG_P0_02.RecipeStateTypedRemove
  - VMG_P0_03.Step3HardpointPlanning
  - VMG_P0_04.Step6MountPlanning
  - VMG_P0_05.ExistingPreservation
  - VMG_P0_06.ContractMatrix

Source guards
- USER confirmation No branch는 Accept/promotion 이전 return
- AcceptCurrentUserDriving() failure는 promotion 이전 return
- Promote helper가 HasCurrentUserDrivingAcceptance()를 재확인하고 false면 mutation 0
- defensive rollback = exact PreviousAllowedVehicleData restore → PostEditChange → Transaction.Cancel → pre-dirty SetDirtyFlag
- integration/service/runtime touched source SavePackage / SaveAll / SaveAsset 호출 0

Runner
- Tools/RunCatalogPromoAffectedTests.ps1
- broad DataAuthoring 100건 replay 없이 CF-FQ-042/043 prefix만 bounded 실행
- EngineExitCode=0 / CF-FQ-042 3/3 / CF-FQ-043 5/5
```

위 evidence로 VRCP-P0-04 Focused / Affected Regression을 Technical PASS로 닫는다. 이미 PASS한 Promotion/RuntimeApply/BuilderStep8 suites는 변경 영향이 없는 한 재실행하지 않는다.

### VRCP-P0-05 — USER Acceptance

**Status: USER Acceptance PASS / Closed — 2026-09-03**

USER가 실제 Builder에서 확인한다.

시나리오 A — 미등록 차량:

```text
Technical Benchmark
→ PIE test-drive
→ confirmation에서 Catalog promotion/no-auto-save 안내 확인
→ USER 주행 PASS
→ Runtime Demo Catalog 등록됨 + 미저장 변경 표시
→ 같은 세션의 cached RuntimeApply section 진입
→ 차량 목록에 exact VehicleData 즉시 표시
```

시나리오 B — 이미 등록된 차량:

```text
USER PASS / retry
→ 이미 등록됨
→ 중복 entry 0
```

시나리오 C — no auto save:

```text
Catalog promotion
→ dirty 상태 확인
→ 자동 Save 없음
```

P0-05 USER-operated UAT evidence:

```text
Baseline / already-registered
- Wagon Step 8 USER Driving PASS = 예
- Runtime Demo Catalog = 등록됨
- Catalog package = clean
- Runtime Catalog 등록 재시도 = disabled
- duplicate를 유도하는 재등록 action 없음

Persisted-unregistered fixture
- USER가 Default Runtime Catalog에서 DA_Vehicle_Wagon을 제거하고 명시 Save
- fresh persisted AssetDump AllowedVehicleData = 3
  - DA_TestSedan
  - DA_TestSUV
  - DA_VehicleDefense_TestSUV
- Step 8 fresh read = 미등록 | USER Driving PASS는 유지됨
- Runtime Catalog 등록 재시도 = enabled

Retry promotion
- USER가 Runtime Catalog 등록 재시도 실행
- Step 8 = 등록됨 | Catalog 저장 상태: 미저장 변경 있음
- Catalog live dirty = true
- Editor status = 1개 저장되지 않음
- retry button = disabled
- fresh persisted AssetDump는 여전히 3개로 Wagon 없음
- 따라서 promotion auto-save = 0

Same-session RuntimeApply
- USER가 같은 Editor lifetime에서 PIE를 시작
- VehicleDebugPanel의 런타임 적용 section이 즉시 Vehicles=4를 표시
- Vehicle ComboBox에 `4. DA_Vehicle_Wagon` 노출
- Editor restart / Catalog save 없이 live Catalog change를 cached RuntimeApply가 즉시 반영
```

Scenario A는 기존 exact persistent USER Driving PASS를 보존한 Wagon의 explicit retry path로 수행했다. USER PASS→promotion primary trigger 자체는 P0-03/P0-04 source ordering과 focused automation으로 이미 검증되어 있으며, P0-05에서는 실제 USER-operated Catalog status/retry/no-auto-save/same-session consumer UX를 확인했다.

### VRCP-P0-06 — Current System Promotion / Downstream Handoff

**Status: PASS / Closed — 2026-09-03**

Closure evidence:

```text
USER explicit Catalog Save 완료
→ fresh persisted AssetDump AllowedVehicleData count=4
→ DA_Vehicle_Wagon exact membership=1
→ Systems/Vehicles/VehicleBuilder.md v1.3.0 Current contract 승격
→ RuntimeApplyPlan.md v0.1.17 / RTA-P0-06 persisted Wagon candidate handoff
→ ActiveWork / FeatureQueue / Plan Index Ready route cleanup
→ Historical + Retained Path 전환
```

완료 시:

- `Systems/Vehicles/VehicleBuilder.md`에 USER PASS → Runtime Catalog Promotion current contract 승격.
- RuntimeApply의 allowlist/authorization owner는 CF-FQ-041에 유지.
- USER가 Catalog를 explicit Save한 뒤 AssetDump/readback으로 exact persisted membership을 확인한다.
- persisted membership 확인 전에는 `RTA-P0-06 Packaged Demo Ready`로 handoff하지 않는다.
- ActiveWork / FeatureQueue / Plan Index에서 stale Ready route 정리.
- 대표 Plan Historical 전환.
- RTA-P0-06 Packaged Demo가 Builder-produced promoted vehicle을 consumer candidate로 사용할 수 있음을 handoff.

---

## 7. C++ / BP 책임 분배

| 영역 | C++ | BP |
| --- | --- | --- |
| Catalog raw resolve/validation | 전용 Editor C++ Promotion Service | 없음 |
| exact membership/add/readback | 전용 Editor C++ Promotion Service | 없음 |
| transaction/internal rollback/dirty | 전용 Editor C++ Promotion Service | 없음 |
| Step 8 USER acceptance | 기존 BuilderVM C++ 재사용 | 없음 |
| promotion orchestration/confirmation/retry/status | `SCFVehicleBuilderTab` Slate/C++ | 없음 |
| RuntimeApply Catalog option change sync | 기존 RuntimeApply C++ Widget의 bounded cache sync | 기존 UI 구조 유지 |
| RuntimeApply Vehicle/Equipment apply | 기존 Runtime C++ service 재사용 | 기존 UI 구조 유지 |
| 차량/장비 Asset | 기존 DataAsset | 자동 생성/수정 없음 |

이번 Feature를 위해 Blueprint graph를 추가하거나 Runtime BP authority를 만들지 않는다. CF-FQ-044는 `FCFVehicleBuilderVM`에 신규 Catalog owner/state를 추가하지 않는다.

---

## 8. 변경 예상 범위

P0-01 correction 기준 기본 변경 범위:

```text
신규 — CarFight_ReEditor/Private/DataAuthoring/
- CFVehicleCatalogPromoService.h
- CFVehicleCatalogPromoService.cpp
- CFVehicleCatalogPromoTests.cpp 또는 동등 focused test owner

수정 — Builder orchestration
- CFVehicleBuilderTab.h
- CFVehicleBuilderTab.cpp

수정 — RuntimeApply bounded cache sync
- CFRuntimeApplyWidget.h
- CFRuntimeApplyWidget.cpp
- CFRuntimeApplyUITests.cpp

가급적 수정 금지 — CF-FQ-043 / 기존 authoring owner 보호
- CFVehicleBuilderVM.h
- CFVehicleBuilderVM.cpp
- CFVehicleAuthoringService.*
- CFVehicleRecipeData.*

UE/Content/
- Product VehicleData mutation 없음
- Default Runtime Catalog는 실제 USER PASS promotion에서만 controlled mutation 가능
- 자동 Save 없음

Document/
- 본 Plan
- ActiveWork / FeatureQueue / Plan Index projection
- 완료 후 VehicleBuilder Systems
```

2026-09-03 CF-FQ-043 closure 후 `CFVehicleBuilderVM.h/.cpp`와 `CFVehicleBuilderTab.h/.cpp`의 fresh worktree diff는 모두 0이다. VRCP-P0-02에서는 해당 4파일과 RuntimeApply integration을 수정하지 않았으며, VRCP-P0-03 착수 시에도 BuilderVM no-touch를 유지하고 BuilderTab Step 8 exact integration hunk만 최소 수정한다.

---

## 9. Scope Out

다음은 본 Plan에서 구현하지 않는다.

- 중앙 Vehicle Registry
- Asset Registry 기반 전체 차량 자동검색
- Tests/Legacy 자동 등록
- VehicleData 생성 즉시 등록
- USER PASS 이전 등록
- auto Save / Save All
- VehicleData/Recipe/Chaos tuning 자동 수정
- RuntimeApply Vehicle/Equipment service 재구현
- Catalog에서 자동 삭제
- 여러 Catalog profile 관리
- 정식 Inventory/Fitting UI
- Network/SaveGame vehicle identity

---

## 10. 완료 조건

Feature 완료에는 다음이 모두 필요하다.

```text
VRCP-P0-01 Design Review PASS
VRCP-P0-02 Editor Promotion Service Technical PASS
VRCP-P0-03 Step 8 Integration Technical PASS
VRCP-P0-04 Focused/Affected Regression PASS
VRCP-P0-05 USER Acceptance PASS
VRCP-P0-06 Current System Promotion PASS
```

그리고 아래 불변조건을 유지해야 한다.

```text
CF-FQ-040 Done 유지
CF-FQ-042 Done 유지
CF-FQ-041 RuntimeApply authorization 유지
CF-FQ-043 Mount Guidance 별도 lifecycle 유지
자동 Save 0
Runtime/Chaos mutation 0
Catalog duplicate 0
Tests/Legacy auto-registration 0
```

---

## 11. 다음 Gate

```text
VRCP-P0-06 Current System Promotion PASS
→ CF-FQ-044 Feature Complete / Historical + Retained Path
```

CF-FQ-044 자체의 후속 Gate는 없다. 다음 제품 검증은 consumer owner인 `CF-FQ-041 / RTA-P0-06 Packaged Demo`가 persisted `DA_Vehicle_Wagon`을 실제 packaged executable에서 load/apply하는 흐름을 소유한다. 새 failure가 없다면 CF-FQ-044 technical/UAT suite를 재개하지 않는다.

---

## 12. Changelog

### Maintenance - 2026-09-06

- G5 Physical Move를 완료해 대표 Plan을 `Document/Plan/Archive/VehicleRuntimeCatalogPromotion/VehicleRuntimeCatalogPromotionPlan.md`로 이동했다. 완료 evidence와 RuntimeApply handoff 계약은 변경하지 않는다.
- Migration: 이전 `Document/Plan/VehicleRuntimeCatalogPromotion/VehicleRuntimeCatalogPromotionPlan.md`는 당시 Historical 기록에서만 유효하며 현재 탐색 경로는 Archive 경로다.

### v0.2.0 - 2026-09-03

- VRCP-P0-06 Current System Promotion을 PASS로 닫았다. USER explicit Save 뒤 fresh persisted AssetDump에서 Default Runtime Catalog `AllowedVehicleData` 4개와 `DA_Vehicle_Wagon` exact membership 1개를 확인했다.
- `Systems/Vehicles/VehicleBuilder.md v1.3.0`에 USER Driving PASS → Runtime Catalog promotion, idempotent membership, rollback/dirty/no-auto-save/retry, same-session RuntimeApply sync 계약을 Current로 승격했다.
- RuntimeApply authorization/apply owner는 CF-FQ-041에 유지하고 `RuntimeApplyPlan.md v0.1.17 / RTA-P0-06`에 persisted Wagon을 Builder-produced Packaged Demo candidate로 handoff했다.
- ActiveWork/FeatureQueue/Plan Index Ready route를 제거하고 본 Plan을 `Historical + Retained Path`로 전환했다. physical move는 별도 maintenance이며 완료 조건이 아니다.

### v0.1.10 - 2026-09-03

- VRCP-P0-05 USER Acceptance를 실제 Builder/PIE flow로 PASS했다. Wagon의 existing USER Driving PASS가 유지된 상태에서 persisted Catalog membership을 제거한 clean 미등록 baseline을 만들고, Step 8 `미등록 | USER Driving PASS는 유지됨`과 retry enable을 확인했다.
- USER가 `Runtime Catalog 등록 재시도`를 실행한 뒤 Step 8 `등록됨 | 미저장 변경 있음`, Catalog live dirty=true, Editor `1개 저장되지 않음`, retry disabled를 확인했다. fresh persisted AssetDump는 계속 Wagon 없는 3개여서 Builder auto-save 0을 직접 증명했다.
- 같은 Editor lifetime에서 USER가 PIE를 시작했고 VehicleDebugPanel `런타임 적용` section이 `Vehicles=4`, ComboBox `4. DA_Vehicle_Wagon`을 즉시 표시해 RuntimeApply cached option same-session sync를 USER flow로 확인했다.
- 초기 already-registered baseline은 Catalog clean + Wagon 등록됨 + retry disabled였으므로 idempotent/no-duplicate UI 계약도 유지됐다. next Gate를 `VRCP-P0-06 Current System Promotion / Downstream Handoff`로 전진했다.

### v0.1.9 - 2026-09-03

- VRCP-P0-04 남은 affected matrix만 bounded 실행했다. CF-FQ-042 creation regression 3/3 PASS, BuilderTab touched overlap이 실제 존재하는 CF-FQ-043 affected regression 5/5 PASS를 확인했다.
- USER No / Accept failure / acceptance recheck failure의 promotion call 0 source ordering과 defensive rollback exact array/pre-dirty/Transaction.Cancel 경계를 재감사했다. touched integration/service/runtime source의 SavePackage/SaveAll/SaveAsset 호출은 0이다.
- P0-03에서 이미 PASS한 Promotion Service 4/4, RuntimeApply 14/14, CF-FQ-040 BuilderStep8 evidence는 반복하지 않고 P0-04 matrix evidence로 재사용했다.
- broad DataAuthoring replay를 피하기 위해 `Tools/RunCatalogPromoAffectedTests.ps1`을 추가해 CF-FQ-042/043 prefix만 실행했다. wrapper terminal PASS / engine exit 0이다.
- VRCP-P0-04 Focused + Affected Regression을 Technical PASS로 닫고 next Gate를 `VRCP-P0-05 USER Acceptance`로 전진했다.

### v0.1.8 - 2026-09-03

- P0-03 중간검수에서 RuntimeApply exact-sequence comparator가 raw Catalog count와 valid-entry filtered cache count를 직접 비교해 null/invalid entry가 존재할 때 selected RuntimeApply section의 auto-refresh마다 `ClearOptions()`가 반복될 수 있는 P1을 발견했다.
- Vehicle/Equipment comparator를 `Rebuild*Options()`와 동일한 valid-entry filtered sequence 비교로 교정했다. raw null/invalid entry는 Catalog validation에서는 계속 invalid로 남지만 UI cache 동기화에서는 반복 invalidation 원인이 되지 않는다.
- `CatalogOptionSync` Automation의 Product Default Catalog 최소 2개 Vehicle/Equipment fixture 의존도 제거하고 test-owned transient Vehicle/Equipment로 전환했다. null entry 삽입 후 repeated refresh의 option identity/count/selection 안정성도 추가했다.
- 교정 후 official Editor build `2691ee932be24a86b958c16bc3889853` PASS, RuntimeApply full regression 14/14 PASS를 재확인했다. BuilderVM diff 0, Promotion/Builder Step8 기존 PASS evidence는 영향 범위 밖으로 보존했다.
- 재검수 결과 P0 0건 / P1 0건. Slate status/IsEnabled getter가 synchronous fresh read를 중복 수행할 수 있는 성능 P2 1건만 비차단 잔여로 남기며, 새 SSOT/cache 설계 없이 P0-04를 진행한다.

### v0.1.7 - 2026-09-03

- P0-03 final code audit에서 Pawn VehicleData 변화와 Catalog Equipment 변화가 같은 refresh에 겹치면 Equipment options가 두 번 rebuild될 수 있는 bounded-lifecycle gap을 발견했다.
- Refresh 순서를 Vehicle Catalog sync → Pawn/Mount sync → Equipment Catalog sync로 교정해 `RebuildMountOptions()`가 이미 Equipment cache를 갱신한 경우 후속 exact-sequence check가 추가 ClearOptions를 생략하도록 했다.
- 교정 후 official Editor build `35958c7c940f4741917b5ed3cf54b221` PASS, RuntimeApply full regression 14/14 재PASS를 확인했다. Promotion 4/4와 Builder affected/Step8 PASS는 변경 영향 밖 evidence로 보존한다.
- VRCP-P0-03 Technical PASS / VRCP-P0-04 Ready 상태는 유지한다.

### v0.1.6 - 2026-09-03

- VRCP-P0-03 Step 8 Integration / Retry UX를 구현하고 Technical PASS로 닫았다. BuilderTab이 persistent `BuilderDrivingAcceptanceReceipt` 성공 이후만 Promotion Service를 호출하며 USER PASS와 Catalog 결과를 분리하고 explicit retry/fresh membership status를 제공한다.
- RuntimeApply cached Widget에 Vehicle/Equipment exact-sequence change-aware sync를 추가했다. append/reorder를 반영하되 unchanged refresh는 Combo options를 재구성하지 않아 기존 Equipment dropdown lifecycle을 보존한다.
- final official Editor build `35958c7c940f4741917b5ed3cf54b221` PASS, Promotion focused 4/4 PASS, RuntimeApply 14/14 PASS(신규 CatalogOptionSync 포함, final ordering correction 뒤 재PASS), Builder affected focused suite 및 BuilderStep8Driving PASS를 확보했다.
- BuilderVM mutation 0, Product Default Catalog test mutation/save 0, auto Save 0을 유지했다. P0-04 matrix 일부 evidence를 선행 확보했지만 전체 P0-04 PASS로 확대하지 않고 next Gate를 `VRCP-P0-04 Focused / Affected Regression`으로 전진했다.

### v0.1.5 - 2026-09-03

- CF-FQ-043 Done / `VehicleBuilder.md v1.2.0` baseline에 맞춰 VRCP-P0-03 설계를 rebase했다. USER Driving PASS authority를 legacy host-local token 중심 설명에서 persistent `BuilderDrivingAcceptanceReceipt`의 exact Target path + DefinitionHash로 교정했다.
- Benchmark RunId는 진단용이며 same DefinitionHash의 새 benchmark에서 USER PASS를 다시 요구하지 않는 043 계약을 044 retry semantics에 반영했다. 단, retry는 `HasCurrentUserDrivingAcceptance()`를 우회하지 않으므로 stale/missing benchmark state는 기존 Step 8 authority가 먼저 복원한다.
- BuilderVM 신규 Catalog state/cache를 계속 금지하고, BuilderTab이 acceptance 성공 뒤 Promotion Service를 orchestration하며 Catalog status는 fresh resolve/membership readback에서 파생하도록 고정했다.
- upstream Current System pointer를 `VehicleBuilder.md v1.2.0`으로 갱신하고 VRCP-P0-03 Current Design Rebase PASS / Implementation In Progress로 전진했다.

### v0.1.4 - 2026-09-03

- CF-FQ-043가 Done / Current System Promotion Complete로 닫힌 뒤 BuilderVM/BuilderTab 4파일의 fresh worktree diff 0을 확인하고 VRCP-P0-02 최종 검증을 재개했다.
- 공식 Editor build는 첫 시도에서 AI-owned Editor의 `UnrealEditor-CarFight_ReEditor.dll` 점유로 link만 실패했다. 안전한 `stop_ai_owned` 후 재빌드가 PASS해 Source/Link 기준을 충족했다.
- corrected focused Automation을 재실행해 `RegisterIdempotent`, `InvalidCatalogBeforeMembership`, `UnavailableAndInvalidTarget`, `UndoRedoFreshMembership` 4/4 PASS를 확인했다.
- no-auto-save 정적 감사에서 `SavePackage`/`SaveAsset`/`SaveAll` 0건, transaction/cancel/pre-dirty restore와 exact membership add 경로를 재확인했다.
- VRCP-P0-02 Editor Promotion Service를 Technical PASS로 닫고 next Gate를 `VRCP-P0-03 Step 8 Integration / Retry UX`로 전진했다. BuilderVM/BuilderTab/RuntimeApply integration은 P0-02에서 수정하지 않았다.

### v0.1.3 - 2026-09-02

- VRCP-P0-02 Editor-only `FCFVehicleCatalogPromoService`와 focused Automation/runner를 구현했다. BuilderVM/BuilderTab/RuntimeApply integration은 변경하지 않았다.
- raw DefaultCatalog resolve, persistent target validation, full pre-validation, exact identity/idempotent add, transaction, defensive rollback, post-validation/readback, package dirty/no-auto-save 계약을 구현했다.
- official Editor build PASS 뒤 focused Automation에서 3/4 PASS를 확인했고, 실패 1건은 Config Settings fixture가 CDO DefaultCatalog를 상속한 test 오류로 판정해 `DefaultCatalog.Reset()`으로 교정했다.
- 교정본 test source compile은 PASS했으나, 재빌드가 보호 대상 CF-FQ-043 `CFVehicleBuilderVM.cpp`의 `TArray::CountByPredicate` C2039로 link 전에 차단됐다. CF-FQ-044 P0-02에서 해당 파일을 수정하지 않는 계약을 지켜 Technical PASS는 보류한다.
- next exact gate는 CF-FQ-043 compile blocker 해소 후 VRCP-P0-02 corrected focused Automation 4/4 재실행이다.

### v0.1.2 - 2026-09-02

- `VRCP-P0-01 Design Audit Correction Re-review`를 current Source 기준으로 완료하고 P1 5건/P2 3건 교정이 모두 현재 구현 seam과 양립함을 확인해 Design PASS로 닫았다.
- CF-FQ-043가 VMG-P0-03에서 BuilderVM뿐 아니라 BuilderTab도 current dirty로 수정 중임을 반영했다. fresh diff상 CF-FQ-043 Tab 변경은 Step 3 Hardpoint hunks이고 CF-FQ-044 Step 8 acceptance hunk와 겹치지 않아 BuilderVM no-touch + Tab Step 8 exact minimal patch로 병렬 경계를 확정했다.
- Editor module이 이미 `CarFight_Re`/`UnrealEd` dependency를 보유하고 Catalog public arrays/validation, Settings public soft reference, BuilderVM Step 8 read API, RuntimeApply option cache가 모두 필요한 current implementation seam을 제공함을 재확인했다.
- 기존 `CFBuilderEvidenceRefresh`의 transaction/rollback/no-save 패턴을 precedent로 확인해 pre-dirty restore와 normal USER Undo 분리 계약을 유지했다.
- 자동화 범위를 현실화해 `FMessageDialog` 클릭이나 deterministic Add 뒤 강제 실패만을 위한 production fault-injection hook은 금지했다. Service/Undo/RuntimeApply sync는 isolated automation, confirmation/guard branch는 source integration + USER Gate로 분리했다.
- 신규 Source/UE Asset 구현은 0이며 next Gate를 `VRCP-P0-02 Editor Promotion Service Implementation`으로 전진했다.

### v0.1.1 - 2026-09-02

- `VRCP-P0-01 Detailed Promotion Contract Design Review`에서 P1 5건 + P2 3건을 확인해 구현 전 설계 교정을 반영했다. 상태는 `Design Audit Correction Applied / Re-review Ready`이며 아직 PASS나 Implementation Ready로 확대하지 않는다.
- CF-FQ-043가 current `CFVehicleBuilderVM.h/.cpp`를 수정 중임을 fresh diff로 확인해 CF-FQ-044 owner를 전용 Editor `FCFVehicleCatalogPromoService` + `SCFVehicleBuilderTab` orchestration으로 고정하고 BuilderVM 신규 Catalog state/mutation 추가를 금지했다.
- 기존 `UCFRuntimeTestSettings::LoadDefaultCatalog()`의 nullptr collapse를 피하기 위해 raw Settings soft reference → raw load → full Catalog validation 순서를 고정하고 `CatalogUnavailable / CatalogInvalid`을 분리했다.
- Catalog promotion order를 target validation → full pre-validation → exact membership → transaction → full post-validation/readback으로 교정해 invalid Catalog가 `AlreadyRegistered`로 통과하지 못하게 했다.
- USER Driving confirmation에 Catalog promotion 시도와 no-auto-save를 명시하고, USER PASS와 Promotion typed outcome을 계속 분리했다.
- RuntimeApply child가 cached/reused되는 current Source를 반영해 Vehicle/Equipment option exact sequence가 실제 바뀔 때만 rebuild하는 change-aware synchronization을 P0 필수로 올렸다. 일반 Tick `ClearOptions()` 금지로 v1.0.1 Equipment dropdown lifecycle fix를 보호한다.
- Catalog status는 fresh membership에서 파생하도록 하고 internal rollback dirty exact restore와 normal USER Undo/Redo semantics를 분리했다.
- Promotion Success와 Packaged Ready를 분리해 USER explicit Save + persisted Catalog membership evidence 뒤에만 RTA-P0-06 handoff하도록 보강했다.
- Source/UE Asset 구현은 0이며 다음 Gate는 `VRCP-P0-01 Design Audit Correction Re-review`다.

### v0.1.0 - 2026-09-02

- USER 결정으로 `CF-FQ-044 Vehicle Builder Runtime Catalog Promotion`을 P2 / Ready 정식 Plan으로 승격했다.
- Builder 신규 차량 생성 즉시 등록이 아니라 **Step 8 exact USER Driving PASS 성공 후 Default RuntimeTestCatalog explicit allowlist promotion**을 canonical trigger로 고정했다.
- USER Driving PASS와 Catalog Promotion 결과를 분리해 Catalog 실패가 기존 acceptance를 rollback하지 않도록 했다.
- exact persistent VehicleData identity, idempotent add, transaction/Undo, no-auto-save, explicit retry, Tests/Legacy auto-search 금지를 P0 계약으로 고정했다.
- VRCP-P0-00 Current Contract Audit을 PASS로 기록하고 next Gate를 `VRCP-P0-01 Detailed Promotion Contract Design Review`로 설정했다.
- CF-FQ-040/042 Done을 재오픈하지 않고 CF-FQ-041 RuntimeApply authorization과 CF-FQ-043 Mount Guidance lifecycle을 보호한다.

---

## 13. Migration

- 기존 Vehicle Builder Step 8 USER Driving acceptance token은 그대로 유지한다. 새 Runtime Catalog 전용 acceptance field로 이관하지 않는다.
- v0.1.1부터 CF-FQ-044는 `CFVehicleBuilderVM`에 Catalog persistent state/mutation owner를 추가하지 않고 기존 acceptance/read API만 소비한다.
- v0.1.1부터 Promotion Service는 Runtime용 `LoadDefaultCatalog()` 결과 하나로 오류를 합치지 않고 Settings soft reference/raw load/full validation을 분리한다.
- v0.1.1부터 RuntimeApply option cache는 실제 Catalog array 변경 때만 동기화하며 일반 Tick rebuild를 금지한다.
- v0.1.2부터 구현 직전 CF-FQ-043 BuilderTab/BuilderVM fresh diff를 다시 읽고 BuilderVM no-touch + BuilderTab Step 8 exact minimal patch 경계를 유지한다.
- v0.1.2부터 `FMessageDialog`나 defensive rollback만을 위한 production test hook/fault injection을 추가하지 않고 Service automation, source guard review, USER Gate를 구분한다.
- 기존 Default Runtime Catalog의 현재 4 Vehicle / 2 Equipment entry는 그대로 보존한다.
- 기존 차량을 background scan해 일괄 backfill하지 않는다.
- 본 Feature 구현 전 기존 Builder/RuntimeApply 동작에는 변화가 없다.
