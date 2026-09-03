# Vehicle Builder

- 문서 버전: v1.3.0
- 최근 갱신일: 2026-09-03
- 문서 상태: Current Implementation
- 적용 범위: `CF-FQ-040 Guided Vehicle Builder`, `CF-FQ-042 Vehicle Builder 신규 차량 생성 UX`, `CF-FQ-043 Vehicle Builder 장비 장착점 Guidance UX`, `CF-FQ-044 Vehicle Builder Runtime Catalog Promotion`, Guided Builder Editor Shell, Builder-private Authoring ownership, Data Authoring Backend/Advanced Workspace 연계
- 완료 기반: `VB-P0-09 End-to-End USER Acceptance PASS` + `VB-P0-10 Current System Promotion Complete` + `VBCUX-P0-05 USER Acceptance PASS` + `VMG-P0-07 USER Acceptance PASS` + `VMG-P0-08 Current System Promotion Complete` + `VRCP-P0-05 USER Acceptance PASS` + `VRCP-P0-06 Current System Promotion Complete`

---

## 1. 문서 목적

이 문서는 CarFight에서 **신규 차량 한 대를 정상 제작할 때 사용하는 Guided Vehicle Builder의 현재 구현 구조와 책임 경계**를 기록한다.

미래 Builder 기능 계획이나 과거 단계별 구현 이력을 소유하지 않는다.
완료 당시 상세 Build/Automation/USER evidence는 Historical Plan이 보존하고, 현재 구현 판단은 이 문서와 실제 Source/Asset을 우선한다.

```text
정상 차량 제작 UX
= Vehicle Builder

차량 Authoring 공통 엔진
= Data Authoring Backend

전문가용 수동 편집/복구
= Data Authoring Advanced Workspace

런타임 최종 차량 정의
= UCFVehicleData
```

---

## 2. 현재 핵심 구조

현재 Builder의 상위 흐름은 다음과 같다.

```text
SCFVehicleBuilderTab
→ FCFVehicleBuilderVM
→ 기존 FCFVehicleAuthoringVM / Authoring Service
→ Recipe / Resolver / Registry / SourceTrace / Diff / Validation
→ 명시적 Preview / Approval
→ Profile / Recipe / Evidence authoring
→ Final Review
→ DefinitionApply / Guarded Undo
→ UCFVehicleData
→ ACFVehiclePawn / Chaos Vehicle Runtime
```

주요 Source:

```text
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderVM.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAuthoringService.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAIContract.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleProfileTypes.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleRecipeData.h
UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleUXTypes.h
```

Builder는 새로운 Runtime 차량 정의 체계나 별도 raw writer를 만들지 않는다.
최종 Runtime Authority는 계속 `UCFVehicleData`이며 실제 적용 계약은 `VehicleData.md`와 `VehicleRuntime.md`가 소유한다.

---

## 3. Current ownership

### 3.1 Guided Shell

`SCFVehicleBuilderTab`은 사용자가 정상 제작 흐름을 따라가는 Editor Slate 진입점이다.

현재 책임:

```text
차량/메시 후보 표시와 선택
Stable Step 기반 단계 이동
현재 Step의 상태/Blocker/해결 방법 표시
Reference Draft 검토와 승인 UI
Mesh/Socket 준비 안내와 current Chassis Socket 편집 진입
Hardpoint Plan Mode / Standard Hardpoint / MountProfile Guidance
Physics Proposal 검토와 승인 UI
Gameplay Guidance 표시
Final Review / Apply / Undo UI
Technical Benchmark 실행
PIE transient test-drive 준비
USER Driving PASS 명시 입력 + persistent Target Definition receipt
USER Driving PASS 뒤 Default Runtime Demo Catalog 등록 시도 / 상태 / explicit retry
Advanced Workspace 진입
```

한 화면에 VehicleData 전체 필드를 펼치는 방식이 아니라 **현재 단계에 필요한 정보와 행동만 노출**한다.

#### 3.1.1 신규 차량 Creation Entry

`CF-FQ-042` 완료 뒤 Guided Shell은 기존 차량을 선택하는 화면이면서 동시에 **새 차량을 처음부터 시작하는 정상 진입점**이다.

```text
fresh Builder open
→ Stable Step 8개가 Browser refresh 전부터 표시
→ [+ 새 차량 만들기]
→ Blank Start 또는 optional Chassis StaticMesh 선택
→ Vehicle ID 기반 생성 검토
→ explicit approval
→ 새 VehicleData + Recipe 생성
→ exact fresh Browser row로 새 target adoption
→ 기존 Step 1~8 제작 흐름 계속
```

Creation Entry의 현재 계약:

```text
Blank Chassis 허용
임의 StaticMesh 허용
이미 다른 VehicleData가 사용하는 Chassis Mesh 재사용 허용
Mesh-only Candidate Quick Start도 같은 canonical create request 사용
모든 Guided 신규 차량 TransmissionPolicy = VehicleSpecificRequired
선택한 Chassis는 생성 시 Recipe.AssetIntent.ChassisMesh에만 기록
새 VehicleData.VehicleVisualConfig.ChassisMesh 자동 Apply 안 함
실제 Chassis 반영은 Step 7 DefinitionApply가 소유
Preview → explicit approval → commit 재사용
자동 Save 안 함
자동 DefinitionApply 안 함
생성 성공 뒤 exact Definition+Recipe Browser row/highlight 동기화
post-create adoption만 실패하면 생성 성공을 롤백하지 않고 partial success로 구분
```

현재 `Vehicle ID`는 중앙 Vehicle Registry의 게임 전역 ID가 아니라 **Builder가 Definition/Recipe Asset identity를 deterministic하게 만들기 위한 creation naming token**이다. 일반 경로에서는 `DA_Vehicle_<VehicleId>` / `DA_Recipe_<VehicleId>`를 제안하고, 빈값·공백·경로 구분자·비ASCII 등 invalid 입력은 silent sanitize하지 않고 fail-closed한다. package/object exact override는 접힌 Advanced 설정으로 남는다. 차량/무기 통합 데이터 Registry가 아직 없으므로 이 ID를 SaveGame/Network/Ownership의 통합 Primary Key로 해석하지 않는다.

USER Acceptance에서 Vehicle ID 직접 입력 방식은 기능적으로 PASS했지만, USER가 매번 내부 ID를 직접 정해야 하는 관리 부담을 지적했다. 이는 `CF-FQ-042` 완료를 막는 결함으로 보지 않고, 향후 통합 데이터 identity 또는 Builder naming UX를 확장할 때 재검토할 비차단 UX 피드백으로 보존한다.

Final audit에서 기존 차량을 선택한 상태로 `+ 새 차량 만들기`에 진입한 뒤 Browser refresh를 실행하면 stale Authoring selection이 Slate row로 복원되며 New Vehicle mode가 해제될 수 있는 P1을 발견했다. 현재 계약은 `BeginNewVehicleEntry()`가 이전 Authoring selection/cache를 selection level에서 해제하고, Slate refresh도 New Vehicle mode 동안 기존 row highlight를 자동 복원하지 않는 것으로 교정됐다. Browser cache와 persistent Asset은 유지하며 Vehicle ID/Blank-or-Chassis transient 입력과 Step 1 진입은 refresh 뒤에도 보존한다.

재사용 Chassis Mesh는 Mesh-owned geometry도 함께 공유한다.

```text
Wheel_Anchor_* Socket 위치/RelativeScale
WSA가 읽는 Socket 기반 wheel geometry
Chassis Mesh의 다른 Socket/Hardpoint geometry
```

같은 시각 Mesh를 사용하되 wheelbase/Socket geometry가 달라야 하면 별도 Chassis Mesh variant가 필요하다. per-Vehicle Socket geometry override는 현재 Builder 범위가 아니다.

#### 3.1.2 Hardpoint / Mount / Socket Guidance

`CF-FQ-043` 완료 뒤 Step 3/6은 신규 차량의 장비 장착 위치와 장착 규칙을 Guided 흐름 안에서 직접 계획할 수 있다.

```text
BuilderHardpointPlanMode
= LegacyCompatible / Unspecified / NoHardpoints / UseHardpoints

Standard Hardpoint
= LocationSlotId <Category>_<NN>
= SocketName HP_<LocationSlotId>

Standard Mount
= Mount_<LocationSlotId>
= Hardpoint 1:1 기본 규칙
```

핵심 계약:

```text
FQ-043 이전 Recipe는 LegacyCompatible default로 기존 custom/multi-Mount 구조를 보존
Guided 신규 Recipe는 explicit Hardpoint 계획 전 Unspecified
마지막 Hardpoint 삭제가 NoHardpoints를 자동 승인하지 않음
Hardpoint 삭제는 Recipe row만 삭제하고 Chassis StaticMesh Socket은 유지
참조 Mount가 남은 Hardpoint 삭제는 DependencyConflict
MountProfileId / LocationSlotId는 생성 후 silent rename하지 않음
Utility + SizeLimit None은 current valid
weapon Mount는 SizeLimit None 차단
optional EquipmentPreset은 CanUseOnMount 검증
```

Wheel/Hardpoint Socket은 같은 current Chassis StaticMesh를 사용한다.

```text
[Chassis Socket 편집하기]
= Wheel/Hardpoint 공통 Static Mesh Editor 진입

exact SocketName
= read-only text + copy

[추가 후 편집]
= USER explicit action
= current Chassis 원점에 exact 이름 Socket 1개 생성
= 중복 생성 금지
= 자동 rename / 자동 transform 추론 / 자동 Save 금지
```

`추가 후 편집` 활성 여부와 표시 상태는 cached Builder snapshot이 아니라 current `UStaticMesh::FindSocket()` live truth를 사용한다. 같은 Chassis Mesh를 재사용하는 차량은 Socket geometry를 공유하므로 편집 전 shared-Chassis 경고를 노출한다.

`파괴 FX Socket (선택)`은 차량 파괴 시 폭발/잔해 효과를 별도 위치에 붙일 때만 필요하다. 별도 위치가 필요하지 않으면 만들지 않아도 된다.

### 3.2 Builder ViewModel

`FCFVehicleBuilderVM`은 transient workflow state와 Step evaluator를 소유한다.

현재 원칙:

```text
persistent truth를 복제하지 않음
fresh Authoring read를 기준으로 Step 상태 재평가
Stable StepId로 semantic step을 식별
fixed index에 의미를 결합하지 않음
stale transient approval/token을 restart 뒤 자동 부활시키지 않음; exact current truth와 일치하는 persistent Builder/Driving receipt만 durable provenance로 복원 허용
새 writer를 만들지 않음
```

Builder ViewModel은 기존 Authoring facade를 재사용하며 persistent mutation authority를 직접 소유하지 않는다.

### 3.3 Persistent Editor Authoring owner

현재 persistent authoring truth는 다음으로 분리된다.

```text
UCFVehicleRecipeData
= Target binding
+ Asset/Profile/Feel/Mass/Hardpoint/Mount intent
+ Applied state
+ Builder provenance receipt
+ vehicle-specific Transmission review receipt
+ vehicle-specific Engine Curve review receipt
+ USER Driving Target Definition acceptance receipt

UCFVehicleRefEvidence
= 실존 차량 Reference identity/source/claim/conflict/unknown Evidence

Builder-private Profiles
= VehicleBase
+ Drivetrain
+ Handling
+ Performance

UCFVehicleData
= 최종 Runtime 차량 정의
```

Builder-private Profile은 `OwnerRecipeId`가 유효한 차량 전용 Profile이다.
기존 shared/legacy Profile은 Builder-private commit 대상으로 간주하지 않는다.

### 3.4 Intermediate AI Draft

AI가 작성하는 Research/Physics Draft는 Project Saved의 **검토 전 중간 입력**이다.

```text
ResearchDraft
= exact Recipe/Target에 binding된 Reference research 제안

PhysicsDraft
= exact Recipe/Target/accepted Evidence에 binding된 typed vehicle-specific Proposal
```

Draft 파일 자체는 Runtime Authority도, persistent approved authoring truth도 아니다.
Builder가 current identity/fingerprint/hash를 다시 확인하고 Preview를 만든 뒤 USER가 명시 승인해야 persistent owner로 승격된다.

---

## 4. 현재 8-Step Guided Workflow

정상 제작 흐름은 다음 Stable Step 의미로 운영한다.

| Step | 의미 | Current 책임 |
| --- | --- | --- |
| 1 | Identity / Reference | 차량 record 선택·생성, ResearchDraft, Reference Evidence/Companion review, Reference USER acceptance |
| 2 | Mesh Prep | Chassis/Wheel Mesh 준비와 typed AssetIntent 반영 |
| 3 | Socket Guide | Wheel/Hardpoint exact 이름·live 존재 확인, unified Chassis Socket editor, explicit add-at-origin, optional Destroyed FX guidance |
| 4 | Layout Capture | current Socket truth와 persisted Layout의 readiness/current/stale 판정 |
| 5 | Physics Proposal | accepted Evidence 기반 private 4 Profile typed proposal Preview/Commit |
| 6 | Gameplay Setup | Gameplay/Hardpoint/Fitting/default completeness, Standard 1:1 Mount rule, MountType/SizeLimit/EquipmentPreset guidance |
| 7 | Final Review | fresh Resolve/Validation/Drift/전체 Diff/provenance 검토 후 explicit DefinitionApply, guarded Undo |
| 8 | Driving Test | saved Target exact benchmark, PIE transient test-drive, explicit USER Driving acceptance |

Step 상태는 현재 authoritative truth를 다시 읽어 평가한다.
Background full Resolve나 매-frame authoring refresh를 사용하지 않고 selection/step/explicit refresh/Builder mutation 같은 의미 있는 trigger에서 갱신한다.

---

## 5. Data Authoring 역할 고정

`CF-FQ-038 차량 데이터 Authoring`은 Vehicle Builder 완료 뒤에도 폐기하지 않는다.
Current 역할은 다음으로 고정한다.

### Builder Backend

```text
Recipe
Resolver
Field Registry
SourceTrace
Diff
Validation
typed Preview / Proposal
explicit Approval
Apply
Guarded Undo
External Drift
record creation
Profile authoring facade
```

Vehicle Builder는 위 Backend를 사용해 정상 제작 UX를 제공한다.

### Advanced Workspace

Data Authoring의 기존 Workspace는 다음 전문 작업을 위한 별도 화면으로 유지한다.

```text
shared/legacy Profile 유지보수
고급 Field 확인
External Drift 검토·복구
compatibility/legacy 상태 확인
세부 authoring 진단
일반 사용자 Guided 흐름 밖의 전문가 수동 작업
```

따라서 현재 책임 관계는 다음과 같다.

```text
Vehicle Builder
= 정상 신규 차량 제작 UX와 단계형 진행 owner

Data Authoring
= Builder Backend + Advanced Workspace

VehicleData
= Runtime 최종 결과 owner
```

`CF-FQ-038`의 기존 USER/Technical evidence는 `CF-FQ-040` 완료 evidence로 재포장하지 않는다.
DEL6 compatibility retirement와 UA-08 quantitative comparison 같은 CF-FQ-038 자체 잔여 항목은 별도 Paused lifecycle을 유지한다.

---

## 6. Reference → Vehicle-specific Authoring 계약

### 6.1 기본 Reference

정상 실존 차량 경로는 특정 차량의 정확한 identity를 Primary Reference로 사용한다.

가능하면 다음을 분리해 잠근다.

```text
Manufacturer
Model
Generation
Model Year
Trim
Powertrain
Transmission
Drive Layout
Market / Region
```

다른 연식·트림·시장의 수치를 조용히 혼합하지 않는다.

### 6.2 Provenance

AI proposal의 수치는 의미를 구분한다.

```text
FACT
= 공개 Reference에서 직접 확인

DERIVED
= FACT를 입력으로 결정적으로 계산

GAME_BIAS
= CarFight 플레이 목적의 의도적 조정

Unknown
= 현재 신뢰 가능한 authority가 없는 값
```

Unknown을 차급 평균이나 편의상 숫자로 숨겨 채우지 않는다.
GAME_BIAS는 USER Review 대상임을 유지한다.

### 6.3 Vehicle-specific Engine / Transmission

현재 Builder는 신규 차량을 generic compatibility 값만으로 완료 처리하지 않는다.

Drivetrain Profile은 opt-in된 vehicle-specific Transmission complete typed payload를 소유할 수 있다.

```text
Automatic / AutoReverse
Forward / Reverse Gear Ratios
FinalRatio
ChangeUpRPM
ChangeDownRPM
GearChangeTime
TransmissionEfficiency
```

Performance Profile은 opt-in된 vehicle-specific Engine TorqueCurve를 소유할 수 있다.

```text
bUseEngineTorqueCurve
EngineTorqueCurve
```

Transmission/Engine Curve review는 Evidence/method/proposal hash와 persistent Recipe receipt에 binding되며 Final Review에서 fresh 재검사한다.

---

## 7. Wheel / Mesh / Socket 책임 경계

신규 정상 차량의 Wheel Size Authority는 기존 WSA Current 계약을 그대로 사용한다.

```text
canonical shared Wheel StaticMesh Bounds
+ USER-authored Wheel_Anchor Socket RelativeScale
→ Wheel visual size
→ derived Front/Rear Wheel Radius/Width
→ VehicleData Runtime value
```

USER가 직접 수행하는 영역:

```text
Chassis/Wheel Mesh 준비
Wheel_Anchor_FL/FR/RL/RR 배치·스케일
Hardpoint Socket 배치
외형과 휠하우스에 맞는 최종 시각 판단
```

Builder가 수행하는 영역:

```text
필요 exact 이름 안내와 복사 UX
live 존재/중복/유효성 검사
current Socket/Layout 상태 표시
Wheel/Hardpoint 공통 Chassis Socket Editor 진입
USER explicit missing Socket add-at-origin
필요한 Reference 비교 정보 제공
```

Builder는 Socket을 자동 배치하지 않는다. `추가 후 편집`은 USER가 명시적으로 선택한 exact 이름 Socket 1개를 원점에 추가할 뿐이며 위치·회전·Scale 조정과 Save는 USER가 소유한다.
Wheel center 자동 검출, Hardpoint 자동 geometry 배치, Mesh surface 추론은 Current Builder 범위가 아니다.

상세 Current Runtime 계약은 다음 문서를 따른다.

```text
Document/Systems/Vehicles/VehicleData.md
Document/Systems/Vehicles/VehicleRuntime.md
Document/Systems/Vehicles/WheelSync.md
```

---

## 8. Mutation / Approval / Save 안전 계약

Builder write는 기존 Authoring risk/approval 계약을 따른다.

핵심 원칙:

```text
Preview 먼저
→ exact ProposalHash / Recipe fingerprint / Target DefinitionHash / Resolver revision binding
→ USER explicit approval
→ commit/apply 직전 stale 재검사
→ mismatch면 fail-closed
```

금지:

```text
raw UObject property writer 추가
shared Profile 무단 mutation
Preview 없는 Apply
stale approval 재사용
blind automatic write retry
자동 Save / Save All
USER 승인 없는 DefinitionApply
```

Authoring operation result는 Recipe/Target/Profile/Evidence mutation footprint와 Save 여부를 명시한다.
P0 normal authoring facade는 disk Save를 자동 수행하지 않는다.

Step 7 guarded Undo는 마지막 Builder-owned exact transaction과 post-Apply current state가 일치할 때만 허용한다.

---

## 9. Driving Test / Acceptance 계약

Step 8 Technical Benchmark는 **현재 saved Target exact object path + DefinitionHash**에 binding한다.

High-speed benchmark는 expected TargetHash를 실제 saved VehicleData semantic hash와 benchmark 시작 전에 비교한다.

```text
expected != actual
→ benchmark 시작 전 fail-closed

expected == actual
→ result에 target_hash_verified=true
```

PIE test-drive는 current selected VehicleData의 transient duplicate를 active Player VehiclePawn에 적용한다.
이 transient 적용은 Product Asset 저장이나 DefinitionApply를 대신하지 않는다.

USER Driving PASS는 `UCFVehicleRecipeData::BuilderDrivingAcceptanceReceipt`에 **exact Target VehicleData path + Target DefinitionHash**로 persistent binding된다.
Benchmark RunId는 PASS 당시 진단/추적 정보이며 same DefinitionHash에서 benchmark를 다시 실행해 RunId가 바뀌어도 USER PASS 자체는 유지한다.

```text
same Target path + same DefinitionHash
→ Editor 재기동 / local config 유실 / 새 benchmark RunId에서도 PASS 유지

Target path 또는 DefinitionHash drift
→ acceptance stale
→ USER Driving PASS 다시 필요
```

Builder는 이 receipt를 자동 Save하지 않는다. USER PASS 뒤 Recipe Asset을 직접 저장해야 디스크에 영구 보존된다.

### 9.1 Runtime Demo Catalog Promotion

`CF-FQ-044` 완료 뒤 Step 8은 current USER Driving PASS와 Runtime Demo Catalog 등록 상태를 연결한다. Catalog mutation owner는 BuilderVM이 아니라 Editor 전용 `FCFVehicleCatalogPromoService`이며, `SCFVehicleBuilderTab`이 USER PASS 성공 뒤 promotion을 orchestration하고 상태/재시도를 표시한다.

```text
exact current USER Driving PASS 성공
→ Default RuntimeTestCatalog raw resolve
→ Catalog full validation
→ exact persistent /Game VehicleData target validation
→ exact UObject identity membership 확인
→ 미등록이면 transaction 안에서 1회 add
→ post-validation + fresh membership readback
→ Catalog package dirty
→ 자동 Save 안 함
```

현재 안전 계약:

```text
USER Driving PASS와 Catalog promotion 결과는 별도 outcome
Catalog 실패가 USER Driving PASS를 rollback하지 않음
이미 등록된 exact VehicleData는 AlreadyRegistered no-op success
중복 entry를 만들지 않음
invalid Catalog는 membership shortcut보다 먼저 차단
internal failure는 이전 AllowedVehicleData 배열 + package dirty 상태로 rollback
retry는 current HasCurrentUserDrivingAcceptance()를 다시 통과해야 함
persistent /Game VehicleData만 등록 대상
BuilderVM에 Catalog state/cache/mutation authority를 추가하지 않음
```

Catalog 등록 성공은 **메모리 authoring 성공**이며 저장 성공이 아니다. Builder는 Catalog를 dirty로 남기고 USER가 명시 저장한다. Packaged Runtime에서 해당 VehicleData를 확실히 소비하려면 explicit Save 뒤 persisted Catalog membership이 확인되어야 한다.

RuntimeApply 후보/authorization owner는 계속 `CF-FQ-041`이다. RuntimeApply는 동일 `UCFRuntimeTestCatalogData::AllowedVehicleData`를 읽으며, cached Vehicle/Equipment option은 Catalog의 valid-entry exact sequence가 실제로 바뀔 때만 rebuild한다. 따라서 같은 Editor lifetime에서 Builder가 VehicleData를 promotion하면 RuntimeApply 차량 목록이 Editor 재시작이나 Catalog Save 없이 즉시 갱신된다.

대표 closure에서 `DA_Vehicle_Wagon`을 USER가 명시 저장한 뒤 fresh AssetDump로 persisted `AllowedVehicleData` 4개 중 Wagon exact membership 1개를 확인했다. 이 persisted Wagon은 `CF-FQ-041 / RTA-P0-06 Packaged Demo`의 Builder-produced consumer candidate로 handoff된다.

---

## 10. CF-FQ-040 완료 검증 기준선

`VB-P0-09 End-to-End USER Acceptance`와 후속 ESH final audit까지 완료된 대표 Wagon 기준선은 다음과 같다.

```text
RecipeId
= 05F69DD34990A868DEFFD29C1AF5B2F9

TargetHash
= 0e5b48e8dcd39deba441da9237218be6

Step8 RunId
= 256cd822-419e-4a3e-ac23-388b570d78c2

Persisted high-speed
= Peak 206.700 km/h
= FinalGear 6
= T200 31.183 s
= target_hash_verified=true

Affected regression after final hash-preflight correction
= ESH-01 3/3 PASS
= ESH-02 5/5 PASS
= ESH-03 2/2 PASS
= ESH-01~06 Final Audit Clean PASS
```

이 값은 **CF-FQ-040 완료 당시 대표 acceptance/evidence**다.
새 차량의 목표 성능값이나 모든 차량에 복사할 tuning preset이 아니다.

새 관련 failure가 없는 한 ESH-01~06이나 Wagon 재튜닝을 Vehicle Builder 정상 사용의 선행 절차로 반복하지 않는다.

---

## 11. Current Scope Out / 후속 경계

현재 Builder가 소유하지 않는 범위:

```text
Wheel center 자동 geometry 검출
Hardpoint 자동 geometry 배치
UE Editor 내부 LLM/Web crawler
사용자 확인 없는 Save/Apply
가변 shift map Runtime 재설계
실차 전자식 최고속 limiter 강제 적용
브랜드/IP 표현 결정
CF-FQ-038 DEL6/UA-08 자체 잔여 작업
```

새 Builder 기능을 추가할 때는 이 Current 구조를 기본으로 하고, 실제 계약 변경이 필요한 경우에만 새 Feature/Plan lifecycle을 연다.

---

## 12. 관련 Current 문서

```text
Document/Systems/Vehicles/VehicleData.md
Document/Systems/Vehicles/VehicleRuntime.md
Document/Systems/Vehicles/WheelSync.md
Document/Systems/Vehicles/VehicleCoreDecisions.md
```

완료 당시 상세 설계·검증:

```text
Document/Plan/Archive/VehicleBuilder/VehicleBuilderPlan.md
Document/Plan/Archive/VehicleBuilder/VehicleBuilderRoadmap.md
Document/Plan/VehicleBuilderCreationUX/VehicleBuilderCreationUXPlan.md
Document/Plan/VehicleMountGuidance/VehicleMountGuidancePlan.md
Document/Plan/VehicleRuntimeCatalogPromotion/VehicleRuntimeCatalogPromotionPlan.md
Document/Plan/Archive/README.md
```

`CF-FQ-040` 대표 Plan은 `Done → Historical + Archived Path`로 `Document/Plan/Archive/VehicleBuilder/`에 보존한다. `CF-FQ-042`, `CF-FQ-043`, `CF-FQ-044` 대표 Plan은 Current route에서 내려온 `Historical + Retained Path`로 각각 기존 `VehicleBuilderCreationUX/`, `VehicleMountGuidance/`, `VehicleRuntimeCatalogPromotion/` 경로를 보존한다.

---

## 13. Changelog

### v1.3.0 - 2026-09-03

- `CF-FQ-044 / VRCP-P0-06 Current System Promotion`으로 Step 8 USER Driving PASS 이후 Default RuntimeTestCatalog promotion 계약을 Vehicle Builder Current System에 승격했다.
- Editor 전용 Promotion Service, BuilderTab orchestration, exact persistent target/full Catalog validation/idempotent membership/transaction rollback/no-auto-save와 USER PASS 분리 outcome을 Current 안전 계약으로 기록했다.
- RuntimeApply authorization은 `CF-FQ-041`에 유지하고 Catalog valid-entry exact-sequence change-aware sync로 same-session 즉시 노출되는 경계를 기록했다.
- USER가 Default Catalog를 explicit Save한 뒤 fresh AssetDump에서 `AllowedVehicleData` 4개와 `DA_Vehicle_Wagon` exact membership 1개를 확인했다. 이 persisted Wagon을 `RTA-P0-06 Packaged Demo` consumer candidate로 handoff했다.

### v1.2.0 - 2026-09-03

- `CF-FQ-043 / VMG-P0-08 Current System Promotion`으로 Hardpoint/Mount/Socket Guidance를 Vehicle Builder Current 계약에 승격했다.
- Recipe-owned 4-state Hardpoint Plan Mode, stable Standard Hardpoint/Mount identity, Utility/Size/Preset validation, Legacy/custom/multi-Mount preservation과 dependency-safe no-cascade delete를 Current로 기록했다.
- Wheel/Hardpoint 공통 `Chassis Socket 편집하기`, exact SocketName copy, USER explicit `추가 후 편집` add-at-origin/no-auto-save, live `FindSocket()` 상태, shared Chassis warning과 optional Destroyed FX Socket 의미를 Current UX로 승격했다.
- USER Driving PASS를 host-local RunId token이 아니라 persistent Target path + DefinitionHash receipt로 보존하는 현재 계약을 기록했다. same DefinitionHash에서는 Editor 재기동/새 benchmark RunId에도 유지하고 Definition drift에서만 stale 처리한다.
- CF-FQ-043 final technical baseline은 `CarFight.DataAuthoring 100/100 PASS`; USER는 Socket/Naming과 Wagon Step 8 re-accept/save/Complete를 PASS했고 fresh AssetDump에서 persisted `BuilderDrivingAcceptanceReceipt`를 확인했다.

### v1.1.1 - 2026-09-02

- CF-FQ-042 final audit P1을 교정해 기존 managed 차량 선택 → `+ 새 차량 만들기` → Vehicle ID 입력 → Browser refresh에서도 New Vehicle mode와 Vehicle ID가 유지되고 old Authoring selection이 복원되지 않도록 Current 계약을 보강했다.
- `BeginNewVehicleEntry()`가 이전 Authoring selection을 해제하고 Slate refresh가 New Vehicle mode에서 row highlight를 복원하지 않는 이중 방어를 기록했다. Product Asset/Save/DefinitionApply/Runtime mutation은 없다.
- USER manual Editor build PASS 후 `CF_FQ_040.VB_P0_09.BuilderShell` 1/0, `CF_FQ_042` 3/0, `DAUTH_P0_11.FrozenUX.MeshCreate` 1/0 focused/affected Automation PASS를 확인했다.

### v1.1.0 - 2026-09-02

- `CF-FQ-042 / VBCUX-P0-05 USER Acceptance` A Blank Start, B 기존 Chassis 재사용, C 미사용 Mesh Quick Start를 실제 Guided Builder UI에서 USER PASS로 닫고 신규 차량 Creation Entry를 Current System으로 승격했다.
- Stable Step 8개 pre-refresh 표시, selection-independent `+ 새 차량 만들기`, Blank/Unused/Reused Mesh 공통 create request, `VehicleSpecificRequired`, Recipe-only initial Chassis intent, no-auto-save/no-auto-DefinitionApply, exact Browser adoption과 partial-success/no-hidden-rollback 계약을 Current로 기록했다.
- reused Chassis Mesh가 Socket/WSA/Hardpoint geometry를 공유한다는 제약과 per-Vehicle Socket override Scope Out을 Current 계약으로 승격했다.
- 현재 Vehicle ID를 중앙 데이터 Registry의 영구 PK가 아니라 Definition/Recipe 생성용 naming token으로 명확히 하고, USER의 직접 ID 입력 관리 부담 피드백은 비차단 후속 UX 검토 대상으로 보존했다.

### Maintenance - 2026-09-02

- CF-FQ-040 G5 Physical Move에 따라 완료 당시 Historical Plan 링크만 `Document/Plan/Archive/VehicleBuilder/`로 교정했다. Current runtime/authoring 계약과 문서 버전은 변경하지 않는다.

### v1.0.0 - 2026-09-02

- `CF-FQ-040 / VB-P0-10 Current System Promotion`으로 Guided Vehicle Builder의 실제 Shell→ViewModel→Data Authoring Backend→VehicleData 구조를 Current Systems에 신규 승격했다.
- 정상 신규 차량 제작 UX는 Vehicle Builder, 공통 Authoring 엔진과 전문가용 화면은 Data Authoring Backend + Advanced Workspace, Runtime 최종 결과는 VehicleData가 소유하도록 역할을 고정했다.
- Reference/Evidence, Builder-private Profile, Transmission/Engine Curve provenance, Preview→explicit approval, Final Review/Undo, exact TargetHash benchmark와 USER Driving acceptance 경계를 Current 계약으로 정리했다.
- VB-P0-09 USER Acceptance와 ESH-01~06 Final Audit Clean PASS를 완료 기준선으로 보존하되 Wagon 수치를 일반 차량 preset으로 해석하지 않도록 분리했다.
