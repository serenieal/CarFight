# CarFight Data Asset Management Plan

- Version: 0.2.0
- Date: 2026-09-04
- Status: Done / DAM-P0-04E USER Acceptance PASS / CF-FQ-045 P0 Complete / Current System Promotion Complete / Historical + Archived Path / G5 Physical Move Complete
- Feature: `CF-FQ-045 CarFight Data Asset Management`
- Priority: P2
- Current Active Feature: `CF-FQ-039 Production UI Visual Rework` 유지
- Representative Plan: `Document/Plan/Archive/DataAssetManagement/DataAssetManagementPlan.md`
- Current System Owner: `Document/Systems/DataManagement/DataAssetManagement.md v1.0.0`
- Runtime SSOT: 기존 Unreal `.uasset` DataAsset 유지
- Initial Owner Module: `CarFight_ReEditor`

---

## 1. 목적

CarFight의 DataAsset 구조가 커져도 사용자가 모든 DA 클래스와 관계를 암기하지 않고 프로젝트 데이터를 탐색·이해·검사할 수 있는 Editor 전용 관리 기반을 만든다.

핵심 사용자 경험은 다음 한 문장으로 고정한다.

> 데이터 관련해서 궁금하면 DA Manager를 연다.

이 Feature는 단순 Asset 목록이나 Validate Assets 래퍼가 아니다. 사용자가 먼저 **무슨 데이터가 있는지, 왜 존재하는지, 실제 Asset이 몇 개인지, 어디에서 참조되는지, 현재 알려진 문제가 있는지**를 사람 친화적으로 이해할 수 있는 Project Data Observatory를 제공한다. P0의 참조 정보는 Asset Registry evidence이며 Feature/System ownership SSOT를 새로 만들지 않는다.

---

## 2. DAM-P0-00 Read-only Audit 결과 — PASS

2026-09-03 current Source + persisted Asset read-only audit 기준:

- DA class declaration: 28종
  - Runtime family declaration 21종
  - 이 중 concrete 20종 + abstract base 1종
  - Editor authoring concrete `UDataAsset` 7종
- persisted DA Asset: 53개
  - Runtime Primary DA 41개
  - Editor authoring DA 12개
- Runtime / Authoring / Test / Debug / Legacy 경로가 이미 공존한다.
- 일부 DA는 stable ID를 갖고 일부는 갖지 않는다.
- 일부 DA는 Unreal `IsDataValid()`를 사용하고 일부는 별도 contract helper를 사용한다.
- Asset instance가 0개인 타입도 정상 계약일 수 있다.
- 현재 구조는 사용자가 클래스명·경로·관계를 머릿속에 기억해야 전체 상황을 파악하기 어렵다.

명시적 Unreal `IsDataValid(FDataValidationContext&)` 구현은 InventoryItem, VehicleDefense, VehicleFitting, VehicleSensor, Weapon 계열에서 확인했다. Ammo/Sensor/CombatFx 등은 별도 helper 계약도 존재한다. 따라서 Manager는 모든 타입을 하나의 validation API로 강제 통일하지 않는다.

Stable identity도 AmmoId, WeaponId, ProjectileId, DamageId, EquipmentId, TurretMountId, DefenseId, FittingId 등 타입별로 다르고 VehicleData/VehicleSensorData/VehicleCameraData/UI 일부는 universal ID가 없다. P0에서 공통 `DataAssetId`를 모든 DA에 추가하지 않는다.

---

## 3. 핵심 제품 원칙

### 3.1 Unreal Asset이 SSOT다

```text
Unreal .uasset
= Single Source of Truth
```

Excel, CSV, JSON 또는 Manager 전용 외부 DB를 두 번째 원본으로 만들지 않는다.

### 3.2 Editor-only다

초기 구현은 `CarFight_ReEditor`가 소유한다.

- packaged Runtime에 Manager 코드를 넣지 않는다.
- Runtime gameplay class를 DA Manager 편의 때문에 변경하지 않는다.
- Blueprint에 신규 Runtime 책임을 추가하지 않는다.

### 3.3 P0는 read-first다

P0 허용:

- discover / classify
- search / filter
- describe
- validate
- reference / referencer inspect
- Asset open
- Content Browser sync
- refresh

P0 금지:

- DA 생성/삭제
- rename/move
- stable ID 자동 수정
- bulk edit
- auto-save
- Runtime Catalog 자동 추가
- lifecycle 자동 변경
- 공통 base class migration

### 3.4 기존 28종을 대규모 리팩터링하지 않는다

DA Manager를 위해 모든 ID/validation/base class를 통일하지 않는다. 기존 데이터를 바꾸지 않고 위에서 관찰·해석하는 관리층으로 시작한다.

---

## 4. 미래 확장성 및 Discovery 계약

CF-FQ-045는 현재 28종 정리용 임시 도구가 아니라 미래 Feature와 DA 타입 증가를 흡수하는 기반이다.

### 4.1 Discovery와 Typed Registry는 서로 다른 책임이다

```text
Discovery
= 실제 존재하는 CarFight DA class / asset을 빠짐없이 찾는 책임

Typed Registry
= 발견된 class의 사람용 의미와 type-specific rule을 해석하는 책임
```

금지하는 것은 **Discovery 결과를 현재 Typed Registry 등록 목록으로 제한하는 allowlist hardcoding**이다. 반면 `CFWeaponData → 무기 데이터 / Combat / WeaponId`처럼 class별 semantic descriptor를 Typed Registry에 명시 등록하는 것은 허용하고 P0의 정상 경로로 사용한다.

필수 invariant:

1. DA instance allowlist hardcoding 금지
2. DA class discovery allowlist hardcoding 금지
3. 신규 DA는 Generic discovery로 자동 발견
4. 전용 semantic descriptor가 없는 타입도 `Unregistered`로 표시하고 숨기지 않음
5. Registry Coverage scan이 신규/누락 타입을 감지
6. UI Domain 고정 분기 금지
7. Typed Registry explicit semantic mapping은 허용

### 4.2 P0 canonical class universe

P0의 canonical Type inventory는 native CarFight module 소유의 `UDataAsset` descendant를 기준으로 한다.

```text
/Script/CarFight_Re
/Script/CarFight_ReEditor
```

위 module이 소유한 concrete/abstract `UDataAsset` descendant는 Asset instance가 0개여도 Type inventory에 나타나야 한다.

Persisted Asset discovery는 package path보다 **발견된 CarFight DA class identity**를 우선한다. 따라서 CarFight DA가 표준 `/Game/CarFight/**` 밖에 잘못 배치되어도 Manager에서 숨기지 않고 `Unclassified` Scope로 보여준다.

향후 Blueprint-generated DA class 또는 다른 CarFight-owned module이 persisted Asset scan에서 발견되지만 canonical native set에 없으면 숨기지 않고 `Unregistered` candidate로 투영한다. P0에서 Blueprint DA class를 canonical supported type으로 승격하는 별도 framework는 만들지 않는다.

### 4.3 신규 DA 흐름

```text
새 native CarFight UDataAsset / UPrimaryDataAsset descendant
→ canonical class auto-discovery
→ persisted Asset auto-discovery
→ Typed descriptor 없으면 Unregistered Coverage
→ 의미 기반 관리가 필요할 때 class 단위 descriptor 1회 등록
→ Coverage PASS
```

Asset 하나마다 수동 등록하는 구조는 사용하지 않는다.

---

## 5. 목표 Architecture 및 비용 경계

```text
Native CarFight Class Discovery + Unreal Asset Registry
          │
          ├─ Generic Metadata
          ├─ CFDATypeRegistry
          │      └─ Typed Semantic Adapters
          └─ Coverage Auditor
                    │
             CFDAAuditService
                    │
            CFDAManagementVM
                    │
             CFDAManagerTab
```

### 5.1 기본 화면은 Asset load 없이 구성한다

DA Manager의 기본 `Refresh`는 metadata-only fast lane이다.

```text
Refresh
→ class discovery
→ FAssetData inventory
→ class/asset merge + dedupe
→ abstract/concrete
→ Scope
→ typed descriptor lookup 여부
→ Coverage
→ aggregate counts
```

`Refresh` 중에는 원칙적으로 다음을 수행하지 않는다.

- `FAssetData::GetAsset()`을 이용한 전체 Asset load
- Stable ID property resolve
- native/custom Validation
- 전체 Reference/Referencer expansion

Asset load가 필요한 작업은 별도 loaded lane으로 분리한다.

```text
Validate / 선택 Asset Detail
→ 필요한 Asset만 load
→ Stable ID resolve
→ typed validation
→ duplicate analysis
→ detailed semantic data
```

Reference/Referencer query도 선택 Asset 상세 또는 explicit 요청 시 on-demand로 수행한다.

### 5.2 Registry readiness

Asset Registry가 아직 gathering 중이면 0개 결과를 complete inventory로 확정하지 않는다. Core result는 최소 `Ready / Gathering / Failed` 상태를 구분하며 UI는 `Registry Scanning` 상태를 표시할 수 있어야 한다. Automation은 inventory assertion 전에 registry completion prerequisite를 명시적으로 충족한다.

### 5.3 초기 파일 후보

```text
UE/Source/CarFight_ReEditor/
├─ Public/DataManagement/
│  ├─ CFDAAuditService.h
│  ├─ CFDATypeRegistry.h
│  ├─ CFDATypeAdapter.h
│  ├─ CFDAManagementTypes.h
│  └─ CFDAManagementVM.h
└─ Private/DataManagement/
   ├─ CFDAAuditService.cpp
   ├─ CFDATypeRegistry.cpp
   ├─ CFDAManagementVM.cpp
   └─ CFDAManagerTab.cpp
```

실제 DAM-P0-01 구현 직전 기존 Editor module의 tab registration/Public-Private convention을 fresh Source로 다시 확인한 뒤 최소 파일 집합을 확정한다. DAM-P0-01에는 Nomad Tab 등록과 Manager UI를 넣지 않는다.

---

## 6. Generic Inventory 계약

전용 타입 지식 없이 모든 발견 DA에 최소 다음 정보를 제공한다.

### Type Record

```text
ClassPath
TechnicalClassName
bAbstract
AssetCount
CoverageState
```

### Asset Record

```text
AssetName
ObjectPath
PackagePath
ClassPath
Scope
CoverageState
HealthState
StableIdState
StableId(optional / unresolved 가능)
```

`References / Referencers`는 Generic Refresh 필수 필드가 아니라 on-demand Detail 결과다.

Generic discovery만 된 신규 타입도 Manager에 반드시 나타난다. abstract framework base는 Type inventory에 보존하되 기본 사용자 화면에서는 `Framework / 고급`으로 구분해 concrete authoring type 수와 섞어 과장하지 않는다.

2026-09-03 baseline의 `28 class declarations`는 `27 concrete + 1 abstract UCFInventoryItemData`로 해석한다. 사용자 Overview의 기본 타입 수는 concrete type을 우선하고 abstract 수는 별도 표기한다.

---

## 7. Typed Semantic Registry 계약

`CFDATypeRegistry`는 discovery allowlist가 아니라 **발견된 type의 의미를 설명하는 semantic registry**다.

P0 완료 시 현재 concrete CarFight DA type 전체가 최소 semantic descriptor coverage를 가져야 한다. 단, 모든 type에 복잡한 validator를 강제하지 않는다.

최소 descriptor:

- DomainId / DomainDisplayName
- TypeDisplayName
- 사용자용 역할 설명
- Identity Policy (`Required / Optional / N/A`)
- stable identity accessor가 필요한 경우 해당 resolver
- native/custom validation bridge가 있는 경우 해당 resolver

예:

```text
CFWeaponData
Domain      = Combat
DisplayName = 무기 데이터
Description = 무기의 발사 방식과 전투 설정을 정의합니다.
Identity    = Required / WeaponId
Validation  = current Weapon contract
```

향후 `CFAttachmentData`가 생기면 discovery/UI를 고치지 않고 typed descriptor 1개를 추가하는 방향을 유지한다.

`Unregistered` type은 기술 Class 이름만으로도 숨김 없이 노출되지만, P0 USER acceptance에서 기존 concrete CarFight 타입이 이유 없이 Unregistered로 남아 있으면 실패다.

---

## 8. Coverage / Type State 계약

항상 다음 집합을 비교한다.

```text
Discovered canonical DataAsset Class Set
vs
CFDATypeRegistry Semantic Descriptor Set
```

Coverage는 Asset Health와 분리한다.

```text
CoverageState
- Registered
- Unregistered

TypeInstanceState
- HasAssets
- NoAssetInstance

HealthState
- OK
- Warning
- Error
- NotValidated
- N/A
```

예:

```text
CFMissileSeekerData
Coverage: Unregistered
TypeInstanceState: HasAssets
Health: NotValidated
```

`Unregistered` 자체를 데이터 오류 Warning으로 오해시키지 않는다. 새 DA class가 추가됐지만 semantic descriptor 등록을 빼먹으면 Coverage가 즉시 드러내야 한다.

---

## 9. 사용자 중심 정보 구조

### 9.1 첫 화면 — Data Overview

53개 Asset 전체를 첫 화면에 나열하지 않는다. **사람이 이해하는 큰 Domain과 관리 상태**부터 보여준다.

예:

```text
CarFight Data Management

차량
차량 자체와 주행·방어·카메라 데이터를 관리합니다.
N Assets / N Concrete Types

전투
무기, 장비, 발사체, 피해, 탄약 데이터를 관리합니다.
N Assets / N Concrete Types

타겟·센서
센서와 타겟 선택 관련 데이터를 관리합니다.
N Assets / N Concrete Types

UI
HUD 레이아웃과 시각 데이터를 관리합니다.
N Assets / N Concrete Types

제작 지원
Vehicle Builder Recipe/Profile 데이터를 관리합니다.
N Assets / N Concrete Types

------------------------
Unregistered Types  N
Validation Errors   N
Unclassified Assets N
```

숫자는 현재 discovery/Asset Registry 결과에서 계산하며 문서 숫자를 UI에 하드코딩하지 않는다. abstract framework type은 기본 concrete count와 분리한다.

### 9.2 Type View

Asset instance가 0개인 타입도 표시한다.

| 항목 | 의미 |
| --- | --- |
| 사용자용 이름 | 기술 클래스명보다 우선 |
| 역할 설명 | 왜 존재하는 데이터인지 |
| Domain | 동적 Domain |
| Concrete / Abstract | 사용자 데이터 타입과 framework base 구분 |
| Asset Count | persisted instance 수 |
| Coverage | Registered / Unregistered |
| Health Summary | validation 수행 후 type issue 요약 |

### 9.3 Asset View

metadata-only Refresh 직후 기본 열:

```text
Health | Coverage | Domain | Type | Asset | ID | Scope
```

Stable ID를 아직 load/resolve하지 않았다면 `ID = — / Not Resolved`를 허용한다. Manager 때문에 기존 DA 전체의 UPROPERTY를 `AssetRegistrySearchable`로 일괄 변경하지 않는다.

검색, Domain, Scope, Health, Coverage 필터를 제공한다.

### 9.4 Asset Details

처음에는 **역할·상태·경로**를 보여준다. `Vehicle Runtime / Vehicle Builder / Runtime Apply` 같은 Feature/System 사용 영역 문자열을 P0 semantic descriptor에 하드코딩하지 않는다.

고급 정보 또는 on-demand detail에서 다음을 제공한다.

- technical Class / ObjectPath
- stable identity resolve 결과
- validation detail
- actual Asset Registry reference/referencer evidence

Feature/System ownership 표현은 별도 owner authority가 설계되기 전 P0 범위 밖이다.

---

## 10. Domain 계약

Domain은 **Overview와 동일한 사람이 이해하는 큰 데이터 영역**이다. 세부 기술 타입과 Domain을 중복 계층으로 만들지 않는다.

초기 Domain 예:

- Vehicle
- Combat
- Targeting
- UI
- Authoring
- Other

예:

```text
Combat
├─ WeaponData
├─ ProjectileData
├─ DamageData
├─ AmmoData
├─ EquipmentPresetData
├─ TurretMountData
└─ CombatFxData
```

`Unregistered`는 Domain이 아니라 CoverageState다. 아직 descriptor가 없는 type의 Domain은 `Other` 또는 `Unknown`으로 표시할 수 있다.

UI는 Domain별 고정 `if/else` 분기를 만들지 않는다. Registry가 반환하는 DomainId/DisplayName으로 필터와 Overview group을 동적 구성한다.

향후 Attachment, Missile, Mission, AI, Audio, VFX 등의 Domain 분리가 실제로 유용해질 때 descriptor 추가만으로 확장할 수 있어야 한다.

---

## 11. Scope 계약

P0에서는 기존 DA에 새 metadata를 추가하지 않고 path-derived classification을 사용한다.

```text
/Game/CarFight/Tests/**          → Test
/Game/Test/**                    → Test
/Game/CarFight/_Legacy/**        → Legacy
/Game/CarFight/Debug/**          → Debug
/Game/CarFight/Data/Authoring/** → Authoring
/Game/CarFight/**                → RuntimeContent
그 외                           → Unclassified
```

`RuntimeContent`는 Production 승인 상태를 뜻하지 않는다.

`Unclassified`는 숨김 또는 자동 Error가 아니다. **CarFight DA class의 Asset인데 표준 Scope 경로 밖에 있음**을 나타내며 관리상 확인 대상으로 보여준다.

---

## 12. Health / Issue 모델

Coverage, Scope와 Health를 분리한다.

최소 Health 상태:

- `OK`: 해당 타입에 적용되는 알려진 계약을 통과
- `Warning`: validation 결과 또는 semantic rule상 확인 필요하지만 즉시 invalid는 아님
- `Error`: known contract 위반
- `NotValidated`: 아직 loaded validation을 실행하지 않음
- `N/A`: 해당 type에 validation rule이 적용되지 않음

`Unregistered`는 Coverage 상태이며 Health Warning 자체가 아니다. `NoAssetInstance`도 Type instance 상태이며 Health가 아니다.

Warning 예:

- validation contract가 있으나 비차단 권고 사항 발견
- validated asset의 legacy/deprecated semantic 조합

Error 예:

- required reference 누락
- stable ID가 Required인 타입에서 ID 누락
- namespace-local duplicate stable ID
- native/custom validation Invalid

`Legacy`, `Unclassified`, `No Known Referencer`는 별도 Scope/Reference issue로 표시할 수 있으나 이를 곧바로 데이터 계약 Error로 승격하지 않는다.

Asset count 0은 자동 Error가 아니다.

### 12.1 Validation cache freshness

P0에서 복잡한 asset-change event subscription을 만들지 않는다. 기본 정책은 단순하게 유지한다.

```text
Inventory Result
= metadata-only snapshot + InventoryGeneration

Loaded Validation Result
= 해당 InventoryGeneration에 종속된 lazy 결과

Refresh
→ 새 InventoryGeneration 생성
→ 이전 generation의 loaded Validation result 무효화
→ Health = NotValidated 또는 N/A
```

사용자가 다시 Validate하면 현재 Asset 기준 결과를 계산한다. DAM-P0-02 서비스 계층은 validation 결과와 generation/freshness 계약을 제공하되 장기 static global cache를 소유하지 않는다. Manager session의 실제 cache owner는 DAM-P0-03 `CFDAManagementVM`로 두며, Refresh 뒤 이전 generation 결과를 재사용하지 않는다.

---

## 13. Stable ID / Duplicate 계약

Universal ID를 새로 강제하지 않는다. Typed semantic descriptor가 해당 타입의 Identity Policy와 resolver를 명시한다.

```text
Required
Optional
N/A
```

metadata-only Refresh에서는 stable ID가 Asset Registry tag로 이미 안전하게 존재하는 경우를 제외하고 UObject load를 강제하지 않는다. unresolved 상태를 정식 허용한다.

```text
StableIdState
- Resolved
- NotResolved
- N/A
- MissingRequired
```

Manager 편의를 위해 기존 DA의 ID UPROPERTY를 전부 `AssetRegistrySearchable`로 바꾸는 migration은 P0에서 금지한다.

Duplicate 검사는 loaded identity resolve 이후 namespace-local이다.

```text
WeaponId ↔ WeaponId
ProjectileId ↔ ProjectileId
AmmoId ↔ AmmoId
```

Identity가 `N/A`인 타입은 Missing ID error 대상이 아니다.

---

## 14. Validation 계약

Typed Adapter는 필요에 따라 기존 검사를 연결한다.

```text
Unreal IsDataValid()
existing IsXXXValid()
existing ValidateXXXContract()
Manager-specific non-mutating structural check
```

Manager 편의를 위해 Runtime validation API를 일괄 리팩터링하지 않는다. 검사 동작은 no-mutation / no-auto-save를 유지한다.

---

## 15. Reference / Unused 계약

Reference/Referencer는 `Refresh` 필수 계산이 아니라 **on-demand detail query**다.

P0에서 Asset Registry dependency evidence를 관측할 수 있지만 `Reference 0 = 불필요 Asset`로 단정하지 않는다.

Soft reference, runtime lookup, test fixture, config load와 dependency category 차이가 있을 수 있으므로 보수적으로 표현한다.

- Referenced
- No Known Referencer
- Unknown / Not Queried

Reference 결과를 Feature/System owner SSOT로 해석하지 않는다. 자동 삭제/이동은 P0 범위 밖이다.

---

## 16. C++ / Blueprint 책임 분배

### C++

P0의 class/asset discovery, generic metadata, registry, adapter, scope, validation, coverage, duplicate check, ViewModel, Slate tab을 담당한다.

### Blueprint

P0 신규 책임 없음. Editor tooling을 위해 BP 중간계층을 만들지 않는다.

---

## 17. P0 범위 밖

- DA creation assistant
- delete / move / rename
- bulk edit
- ID auto-fix
- Production lifecycle editing
- DA ↔ FeatureQueue owner metadata
- DA ↔ Systems owner 자동 저장
- Runtime Catalog promotion
- Excel/CSV/JSON second SSOT
- Attachment authoring workflow
- migration framework

특히 DA ↔ Feature owner는 P0에 넣지 않는다. FeatureQueue와 Manager 사이에 두 번째 SSOT가 생길 위험이 있다.

---

## 18. 구현 Gate

### DAM-P0-00 — Read-only Audit + Design

**PASS**

완료:

- current DA class/persisted asset inventory
- validation/identity 차이 확인
- scope 혼재 확인
- Generic + Typed Adapter
- Coverage Auditor
- Data Overview / Type View / Asset View
- extensibility invariants

### DAM-P0-01 — Inventory Core

**Technical PASS — 2026-09-03**

완료:

1. native CarFight DA class auto-discovery
2. persisted DA Asset auto-discovery
3. class/asset merge + deterministic dedupe
4. abstract/concrete 판별
5. zero-instance type visibility
6. metadata-only Generic Type/Asset record
7. path-based Scope + `Unclassified`
8. Typed Registry descriptor lookup 여부
9. CoverageState / TypeInstanceState
10. Asset Registry readiness state
11. focused automation

구현 경계:

- `CFDAManagementTypes.h`가 metadata-only Type/Asset DTO와 Registry/Coverage/TypeInstance/Scope/Health/StableId 상태를 소유한다.
- `CFDAAuditService`가 `/Script/CarFight_Re` + `/Script/CarFight_ReEditor` native `UDataAsset` 후손을 자동 발견한다.
- persisted discovery는 `UDataAsset` 전체 Asset Registry metadata에서 project-local `/Game` 후보 또는 CarFight native/derived class identity를 가진 Asset만 포함한다.
- class/asset merge는 ClassPath/ObjectPath 기준 deterministic dedupe를 수행하며 native instance 0 타입과 persisted-only unknown candidate를 모두 숨기지 않는다.
- Scope는 path segment 경계로 Test / Legacy / Debug / Authoring / RuntimeContent / Unclassified를 분류한다.
- DAM-P0-01의 descriptor lookup은 `RegisteredDescriptorClassPaths`를 통한 **descriptor 존재 여부 seam**까지만 소유한다. 실제 `CFDATypeRegistry`, 사용자 semantic descriptor와 health adapter는 DAM-P0-02 owner다.
- `Refresh`는 `FAssetData` metadata만 사용하고 전체 Asset `GetAsset()`을 호출하지 않는다.
- Stable ID resolve, Validation, Duplicate ID 검사, Reference/Referencer expansion, Manager UI, Nomad Tab은 구현하지 않았다.
- 기존 Runtime DA class와 persisted Product Asset mutation은 0이다.

Validation:

- Official Editor Build: `0ad7210fafd34904a7fea35428425859` / ExitCode 0 / UE 5.8 Source Build
- Focused Automation: `197471da83424c8ab5158a565e9c6acd` / 3 PASS / 0 FAIL / EngineExitCode 0
- focused tests:
  - `CurrentInventory`
  - `MergeScopeCoverage`
  - `RegistryGathering`
- result: `UE/Saved/CarFight/DataAuthoringAutomationResult.json`
- post-correction build 전 AI-owned Editor DLL lock 1건은 ownership-safe `stop_ai_owned` 후 재빌드했으며 final build는 PASS다.

DAM-P0-01 evidence는 유지한다. 현재 next exact internal checkpoint는 `DAM-P0-02C Loaded Health Lane`이다.

### DAM-P0-02 — Typed Semantic + Health Adapter

정식 Gate는 하나로 유지하되 구현과 검수는 다음 내부 checkpoint 순서로 진행한다.

#### DAM-P0-02A — Registry Foundation

**Technical PASS — 2026-09-03**

완료:

1. `ECFDADomain` 사용자 semantic Domain 구조
   - Vehicle / Combat / Targeting / UI / Authoring / Unclassified
2. `FCFDASemanticDescriptor`
   - canonical ClassPath
   - Domain
   - 사용자용 Type 이름
   - 역할 설명
3. `FCFDATypeRegistry`
   - exact class path 등록/lookup
   - 동일 descriptor 재등록 idempotent
   - 동일 class path의 conflicting descriptor fail-closed
   - canonical class path deterministic enumeration
4. `FCFDAResolvedSemantic`
   - Registered descriptor 반환
   - 미등록 타입은 `Unregistered + Unclassified + technical display` fallback
5. Registry → P0-01 Coverage bridge
   - `BuildRefreshOptions()`가 descriptor class path identity만 metadata Inventory에 전달
   - `CFDAAuditService::Refresh(TypeRegistry)` / pure `BuildInventory(..., TypeRegistry)` overload
6. empty Registry current integration
   - 현재 native/persisted Inventory를 숨기지 않음
   - P0-02B 실제 semantic mapping 전 canonical native 타입은 전부 Unregistered fallback으로 유지
7. metadata-only Refresh 경계 유지

구현 경계:

- DAM-P0-02A는 **Registry foundation만** 구현했다. 현재 27 concrete CarFight DA의 실제 semantic descriptor 등록은 아직 0이며 DAM-P0-02B가 current Source 계약을 읽고 등록한다.
- Registry는 `CFDAAuditService` discovery를 대체하지 않는다. Generic discovery 결과가 먼저 존재하고 Registry class-path set은 Coverage만 보강한다.
- Unregistered 타입을 숨기거나 Error로 승격하지 않는다. fallback Domain은 `Unclassified`, 표시명은 technical class name을 사용한다.
- Registry는 process-global static singleton/cache를 만들지 않는다.
- Stable ID resolve, Validation execution, Duplicate ID, Reference/Referencer, Manager UI, Nomad Tab 구현은 0이다.
- Runtime DA class, persisted Product Asset, Builder/RuntimeApply/Catalog Promotion mutation은 0이다.

Validation:

- first Official Editor Build `287e873abaf2435ebddb2b97d80467e1`
  - 신규 `CFDATypeRegistry.cpp`, `CFDARegistryTests.cpp`, 기존 `CFDAInventoryTests.cpp`, `CFDAAuditService.cpp` compile PASS
  - running `UnrealEditor.exe`의 `UnrealEditor-CarFight_ReEditor.dll` 점유만으로 link ExitCode 6
- ownership-safe Editor release `ea08fe23158341d5a6d92ed61e991cab`
  - server가 `carfight.editor.stop_ai_owned`를 허용한 exact AI-owned/no-user-work lifetime만 정리
  - `save_requested=false`
- final Official Editor Build `dcbd3d6c1e6c4ff489fe466832af1250` / ExitCode 0 / PASS
- focused Automation `734d2954a5124d449179aaa0de844702` / 2 PASS / 0 FAIL / EngineExitCode 0
  - `CurrentUnregisteredFallback`
  - `RegistryFoundation`
- affected P0-01 Automation `8f72d9d1870d41e5ada620d109851b27` / 3 PASS / 0 FAIL / EngineExitCode 0
  - `CurrentInventory`
  - `MergeScopeCoverage`
  - `RegistryGathering`
- post-implementation source audit:
  - `AssetData.GetAsset(` / `->GetAsset(` 0
  - `SavePackage(` 0
  - `.IsDataValid(` 0
  - `GetReferencers(` / `GetDependencies(` 0
  - `RegisterNomadTabSpawner(` 0
  - `static FCFDATypeRegistry` 0

DAM-P0-02A evidence는 유지한다. 현재 next exact internal checkpoint는 `DAM-P0-02C Loaded Health Lane`이다.

#### DAM-P0-02B — Current Type Semantics

**Technical PASS — 2026-09-03**

완료:

1. current concrete CarFight DA 27종 전체 semantic descriptor 등록
   - Runtime concrete 20종
   - Editor Authoring concrete 7종
   - abstract framework base `UCFInventoryItemData`는 concrete mapping에서 제외하고 Unregistered framework type으로 유지
2. 사용자 semantic
   - Domain
   - 사용자용 Type 이름
   - 사용자용 역할 설명
3. Identity Policy와 resolver kind 분리
   - `IdentityPolicy = Required / Optional / NotApplicable`
   - `IdentityResolverKind = None / ExplicitFName / ExplicitGuid / PrimaryAssetId`
   - current mapping은 Required 15종 / NotApplicable 12종 / Optional 0종
   - resolver kind는 ExplicitFName 10종 / PrimaryAssetId 3종 / ExplicitGuid 2종 / None 12종
4. Validation Policy 등록
   - `NativeDataValidation` 6종
   - `CustomContract` 8종
   - `None` 13종
   - 실제 Source에서 재사용 가능한 whole-asset validator만 연결하고 문맥 종속 private preflight는 generic health validator로 승격하지 않음
5. current Source 예외 계약 교정
   - `UCFVehicleData` identity는 별도 `VehicleId`가 아니라 `GetPrimaryAssetId()`를 사용
   - `UCFVehicleData` validation은 공개 read-only `UCFVDAValidator::ValidateVehicleData(Target, nullptr)` facade를 CustomContract metadata로 등록
   - Authoring Profile 5종의 `Meta.OwnerRecipeId`는 Profile 자체 stable identity가 아니라 Builder ownership precondition이므로 Identity N/A
   - `UCFVehicleRefEvidence::ValidateInitialResearchPayload()`와 Builder Evidence refresh validation은 생성/refresh 문맥 종속이라 generic Evidence validator로 승격하지 않음
   - `UCFCombatFxData::IsCombatFxConfigured()`는 optional FX presence helper이며 whole-asset validity 계약이 아니므로 Validation None
6. Registry exact coverage
   - current concrete 27/27 Registered
   - abstract `UCFInventoryItemData` 1종 Unregistered 유지
   - 동일 current descriptor set 재등록 idempotent
7. metadata-only Refresh 경계 유지
   - descriptor의 `IdentitySourceName` / `ValidationSourceName`은 선언 metadata일 뿐 resolver/validator를 실행하지 않음

구현 경계:

- Runtime DA Source는 read-only audit만 수행했고 기존 Runtime class와 persisted Product Asset mutation은 0이다.
- `CFDATypeRegistry::RegisterCurrentCarFightDescriptors()`가 current concrete 27종 mapping을 등록한다.
- Identity Policy는 필수성, IdentityResolverKind는 actual Source 해석 방식으로 분리해 Plan의 `Required / Optional / N/A` 계약과 저장 형식을 혼동하지 않는다.
- 기본 `Refresh`는 계속 Asset Registry metadata-only다.
- Stable ID 실제 resolve, native/custom Validation 실행, namespace-local Duplicate ID 검사, Reference/Referencer, Manager UI/Nomad Tab은 구현하지 않았다.
- Builder/RuntimeApply/Catalog Promotion과 CF-FQ-039 UI 작업의 기존 dirty는 건드리지 않는다.

Validation:

- final Official Editor Build: `832ae2e554fc4f4ab5f0b58af0f1c6f4` / ExitCode 0 / PASS
- DAM-P0-02B focused Automation: `50158f55e3364174854ec6347e9f57b7` / 2 PASS / 0 FAIL / EngineExitCode 0
  - `CurrentDescriptorMatrix`
  - `CurrentCoverageIntegration`
- affected DAM-P0-02A Automation: `b5d568a0306b4420b125071dc1885d79` / 2 PASS / 0 FAIL / EngineExitCode 0
  - `CurrentUnregisteredFallback`
  - `RegistryFoundation`
- affected DAM-P0-01 Automation: `b8085cfe677d4cdfb9e79849221c6c78` / 3 PASS / 0 FAIL / EngineExitCode 0
  - `CurrentInventory`
  - `MergeScopeCoverage`
  - `RegistryGathering`
- final source boundary audit:
  - actual `FAssetData::GetAsset()` / `LoadSynchronous` / `TryLoad` / `LoadObject` / `StaticLoadObject` / `ResolveObject` call 0
  - actual `IsDataValid` / custom validator execution call 0
  - `ECFDAStableIdState::Resolved` / `MissingRequired` assignment 0, StableId write 0
  - namespace-local stable ID Duplicate analysis 0
  - `GetReferencers` / `GetDependencies` 0
  - `FGlobalTabmanager` / `SDockTab` / Nomad Tab registration 0

Mid-review correction / re-review — PASS:

- P1-1 미래 DA 확장성: `CurrentCoverageIntegration`이 canonical concrete exact 27/all-Registered를 강제하던 회귀를 제거했다. audited current 27종은 exact Registered를 요구하되 future canonical type은 Generic Discovery에서 Registered/Unregistered 모두 정상 visibility로 허용한다.
- P1-2 Source mapping drift: `IdentitySourceName` / `ValidationSourceName`을 exact source symbol provenance로 정규화하고 expected matrix에서 exact equality를 검사한다. identity UPROPERTY 13개는 `GET_MEMBER_NAME_CHECKED`, custom/native validation과 PrimaryAsset resolver entry는 직접 symbol reference로 compile-time drift guard를 추가했다.
- P1-3 batch atomicity: `RegisterCurrentCarFightDescriptors()`는 caller Registry 복제본에 27종을 먼저 등록하고 전부 성공할 때만 move-commit한다. 중간 VehicleData conflict synthetic test에서 호출 전 Registry exact 보존과 앞선 Ammo descriptor leak 0을 검증했다.
- P1-4 duplicate namespace: `ECFDADuplicateNamespacePolicy` + `ResolveDuplicateNamespacePolicy()`를 추가해 Explicit FName/GUID=`ExactClassPath`, PrimaryAssetId=`PrimaryAssetType`, Identity N/A=`NotApplicable`로 deterministic하게 고정했다. duplicate 분석 자체는 아직 실행하지 않는다.
- P2 Unregistered policy authority: `FCFDAResolvedSemantic::HasAuthoritativePolicy()`를 추가해 Registered semantic만 Identity/Validation policy authoritative로 취급하고 Unregistered fallback은 false임을 P0-02A synthetic test에서 검증했다.
- first correction build `c2aa8d6f58d140529ce8f90fd8aeb13e`는 DataManagement source compile 6/6 PASS 후 실행 중 AI-owned Editor DLL 점유로 link ExitCode 6만 발생했다.
- server-guarded `carfight.editor.stop_ai_owned` `26814cf9a8bd4898bc15bbcbb34abea7`이 exact AI-owned/no-user-work lifetime만 save0 정리했다 (`save_requested=false`).
- final Official Editor Build `4e880dbb47de4bca95e96d3308aa2b56` / ExitCode 0 / PASS.
- corrected P0-02B focused `3e3f7b9ee2794aa5b2f82782beced1d3` / 2 PASS / 0 FAIL.
- affected P0-02A `2508aceae95148b59c06dd7f62a6c442` / 2 PASS / 0 FAIL.
- affected P0-01 `9c56c81cd47a4aec8e24abdc5400d4b1` / 3 PASS / 0 FAIL.
- re-review source audit: P0 0 / P1 0. Asset load, Validation execution, Stable ID resolve/write, Duplicate ID analysis, Reference/Referencer, Manager UI/Nomad Tab 구현은 여전히 0이다.

Next exact internal checkpoint: `DAM-P0-02C Loaded Health Lane`.

#### DAM-P0-02C — Loaded Health Lane

**Technical PASS — 2026-09-03**

완료:

1. metadata-only Refresh와 loaded lane 분리
   - 기존 `FCFDAAuditService::Refresh()`는 Asset Registry metadata-only 동작을 유지한다.
   - generation-aware overload는 caller가 발급한 `InventoryGeneration`만 snapshot에 stamp하며 Asset을 load하지 않는다.
   - actual lazy load는 `FCFDAHealthService::ValidateAsset(s)`의 explicit Validate/Detail 경로에만 존재한다.
   - DataManagement 전체 actual lazy load 지점은 `FSoftObjectPath::TryLoad()` 1곳이다.
2. InventoryGeneration freshness
   - `FCFDAInventoryResult::InventoryGeneration`
   - `FCFDALoadedAssetResult::InventoryGeneration`
   - `IsCurrentGeneration()` / `IsFreshForInventoryGeneration()`
   - `FCFDAAuditService::NextInventoryGeneration()`은 pure next-value helper이며 static generation/cache state를 소유하지 않는다.
   - session-local cache owner는 계획대로 DAM-P0-03 `CFDAManagementVM`에 남긴다.
3. Stable ID loaded resolve
   - Required missing → `MissingRequired` + Health Error
   - Optional missing → `NotResolved`, Error로 승격하지 않음
   - N/A → `NotApplicable`
   - Explicit FName → audited exact FName property를 read-only reflection으로 resolve
   - Explicit Guid → audited exact FGuid property를 read-only reflection으로 resolve
   - PrimaryAssetId → `UPrimaryDataAsset::GetPrimaryAssetId()` resolve
4. typed Validation adapter
   - `FCFDATypeAdapter`가 이미 load된 UObject만 해석한다.
   - NativeDataValidation 6종은 existing public contract를 사용하고, `UCFVehicleFittingData`는 `IsDataValid()` 의미와 맞게 `ValidateFittingDataContract()` 뒤 `BuildFittingSnapshot()` Error/Warning까지 반영한다.
   - CustomContract 8종은 P0-02B에서 감사한 public read-only validator를 exact class별 adapter로 연결한다.
   - VehicleData는 `UCFVDAValidator::ValidateVehicleData(Target, nullptr)` report의 Error/Warning/Blocked를 Health에 투영한다.
5. Unregistered policy non-authoritative
   - `HasAuthoritativePolicy()==false`이면 identity/validation을 추측하지 않고 UObject도 load하지 않는다.
   - Health=`NotValidated`, StableId=`NotResolved`를 유지한다.
6. namespace-local Duplicate ID
   - Explicit FName/Guid → exact ClassPath namespace
   - PrimaryAssetId → PrimaryAssetType namespace
   - 같은 generation + namespace + StableId에서만 duplicate Error를 표시한다.
   - 다른 namespace 또는 다른 generation 결과는 duplicate로 섞지 않는다.
7. 결과와 기본 inventory 상태 분리
   - metadata `FCFDAAssetRecord`는 Refresh 후 계속 Health=`NotValidated`, StableId=`NotResolved`다.
   - loaded 결과는 별도 `FCFDALoadedAssetResult`로 반환하며 Product Asset을 수정하거나 기본 metadata row에 write-back하지 않는다.

구현 파일:

- `Public/DataManagement/CFDATypeAdapter.h`
- `Private/DataManagement/CFDATypeAdapter.cpp`
- `Public/DataManagement/CFDAHealthService.h`
- `Private/DataManagement/CFDAHealthService.cpp`
- `Private/DataManagement/CFDAHealthTests.cpp`
- `Tools/RunDAMHealthTests.ps1`
- `CFDAManagementTypes.h` v1.3.0
- `CFDAAuditService.h/.cpp` v1.2.0

Validation:

- initial build `7459127f3a284210a43f0d17e62b695f`: 새 P0-02C 파일 다수 compile 후 잘못된 `Engine/PrimaryDataAsset.h` include 1건으로 실패 → `Engine/DataAsset.h`로 교정
- intermediate build `0f0db2f4cb3d439399566833b7aeeaf4`: source compile PASS, 실행 중 AI-owned Editor DLL 점유로 link-only 실패
- ownership-safe stop `f7e3e68149c14c1f84dc909a7a99769d`: server가 exact AI-owned/no-user-work lifetime을 증명한 경우에만 save0 stop (`save_requested=false`)
- first implementation build `219ea87b28e34c11b2fccde06840b333`: PASS
- code re-review에서 VehicleFitting `IsDataValid()`의 Snapshot Error/Warning 누락 P1 1건 발견 → adapter v1.0.2로 교정
- correction compile `941d7cadd06b4634a32bb86d67b85717`: source compile PASS, Automation-created AI-owned Editor DLL 점유로 link-only 실패
- ownership-safe stop `ab63522496214d159fa9a6f62ec735ab`: exact AI-owned/no-user-work save0 stop
- **final Official Editor Build `e4826d2a72b34465aefe96b9396dcfa6` / ExitCode 0 / PASS**
- final DAM-P0-02C focused `4ba69d997b80490ba00961cfdea21343` / **3 PASS / 0 FAIL / EngineExitCode 0**
  - `GenerationAndExplicitLoad`
  - `NamespaceDuplicate`
  - `TypedIdentityValidation`
- final affected DAM-P0-02B `01f29bdb934847458ab11b990c16b579` / **2 PASS / 0 FAIL**
- final affected DAM-P0-02A `13601090af114f3e93ff3e6f4fb5def0` / **2 PASS / 0 FAIL**
- final affected DAM-P0-01 `eda94a44bac3466089355b88f613ed9e` / **3 PASS / 0 FAIL**
- final source boundary audit:
  - DataManagement `TryLoad()` actual call = 1, `CFDAHealthService` explicit loaded lane only
  - default `CFDAAuditService::Refresh()` object load = 0
  - `GetReferencers` / `GetDependencies` = 0
  - `SavePackage` = 0
  - `RegisterNomadTabSpawner` / `FGlobalTabmanager` / `SDockTab` = 0
  - static validation/generation cache = 0

Mid-review correction / re-review — PASS:

- P1-1 generation provenance: `FCFDAHealthService::ValidateAsset(s)`가 `AssetRecord + uint64 Generation`을 별도 인자로 받던 API를 제거하고 `FCFDAInventoryResult` snapshot + exact ObjectPath를 authority로 받도록 교정했다. loaded result generation은 Inventory snapshot에서만 파생되며 stale row와 current generation을 임의 조합할 수 없다.
- P1-2 duplicate completeness: `bDuplicateStableId=false`를 제거하고 `ECFDADuplicateState = NotAnalyzed / NotApplicable / Unique / Duplicate`로 분리했다. single Validate도 요청 Asset의 duplicate namespace 후보를 내부 bounded closure로 평가하며, ExactClassPath는 같은 class 전체, PrimaryAssetId는 registered PrimaryAssetId 후보 전체를 평가한 경우에만 `Unique`를 허용한다. candidate load/adapter 실패가 있으면 `NotAnalyzed`를 유지하고 이미 확인된 duplicate evidence는 그대로 `Duplicate + Health Error`로 표시한다.
- P1-3 typed canonical identity: 사용자 표시용 `StableId` 문자열과 duplicate equality key를 분리해 `FCFDACanonicalStableIdentity`를 추가했다. Explicit FName/PrimaryAssetName은 `FName`, Explicit Guid는 `FGuid` 원본 equality를 사용하며 `FName("SharedId")`와 `FName("sharedid")`의 Unreal name identity 동등성도 focused Automation으로 검증했다.
- P1-4 operational failure/DA Health 분리: `ECFDAEvaluationState = NotRequested / Succeeded / PolicyUnavailable / InvalidRequest / LoadFailed / ClassMismatch / AdapterUnavailable`를 추가했다. missing object, stale/unknown request, class mismatch, adapter/source drift는 `Health=NotValidated`로 남기고 DA 자체 known contract 위반만 Health Error로 유지한다.
- P2-1 duplicate namespace authority: `CFDATypeAdapter`와 `CFDAHealthService` 모두 `FCFDATypeRegistry::ResolveDuplicateNamespacePolicy()`를 실제 owner로 사용하도록 통합했다.
- P2-2 focused coverage: unknown path InvalidRequest, load failure, class mismatch, adapter unavailable, single Weapon/Vehicle namespace closure, incomplete coverage NotAnalyzed, canonical FName equality를 추가했다.
- correction Official Editor Build `7651c0fe87da4ee481f6ff9995ec793c` / **ExitCode 0 / PASS**.
- corrected DAM-P0-02C focused `8aaa13197c4d431b8ca72cdd03f69765` / **3 PASS / 0 FAIL / EngineExitCode 0**.
- affected DAM-P0-02B `0c55803041ed41609f76e0d590fbbf9b` / **2 PASS / 0 FAIL**.
- affected DAM-P0-02A `3642b46a41ae43eaabe3fa253cdf4684` / **2 PASS / 0 FAIL**.
- affected DAM-P0-01 `8eb2b67a6ef147ce98c30fd471c85e45` / **3 PASS / 0 FAIL**.
- correction re-review source audit: **P0 0 / P1 0**. DataManagement actual `TryLoad()`는 explicit HealthService 1곳만 유지하고 Reference/Referencer, Save, Manager UI/Nomad Tab, static cache는 여전히 0이다.

**DAM-P0-02 전체 Technical PASS 유지.** Registry Foundation → Current Type Semantics → Loaded Health Lane 세 internal checkpoint와 correction re-review를 모두 닫았다.

Next exact Gate: `DAM-P0-03 Manager UI + On-demand Detail`.

### DAM-P0-03 — Manager UI + On-demand Detail

**Implementation Technical Evidence PASS / Mid-review Correction Required — 2026-09-03**

구현 완료:

- `CarFight.DataAssetManager` Nomad Tab + Window > CarFight > `CarFight 데이터 관리` 진입
- `SCFDAManagementTab`
  - Data Overview
  - Type View / Asset View
  - search
  - dynamic Domain filter
  - Scope / Health filter
  - Type/Asset Detail
  - explicit Validate
  - explicit References
  - Asset Editor open
  - Content Browser sync
- `FCFDAManagementVM`
  - session-local current `InventoryGeneration` owner
  - metadata-only Refresh
  - generation-bound loaded validation cache
  - generation-bound Reference cache
  - Refresh 시 두 cache invalidation
  - selection/filter/search/list projection
- `FCFDAReferenceService`
  - selected Asset의 package-level Asset Registry `GetDependencies` / `GetReferencers`만 on-demand 실행
  - `Unknown / Not Queried`, `Referenced`, `No Known Referencer`, `QueryFailed` 보수적 상태
  - Reference 0을 unused/delete 판정으로 승격하지 않음
- Product Asset mutation / Save / `MarkPackageDirty` / `Modify` = 0

구현 파일:

- `Public/DataManagement/CFDAReferenceService.h`
- `Private/DataManagement/CFDAReferenceService.cpp`
- `Public/DataManagement/CFDAManagementVM.h`
- `Private/DataManagement/CFDAManagementVM.cpp`
- `Public/DataManagement/CFDAManagementTab.h`
- `Private/DataManagement/CFDAManagementTab.cpp`
- `Private/DataManagement/CFDAManagementTests.cpp`
- `Tools/RunDAMManagerTests.ps1`
- `Public/CarFightReEditor.h` v1.5.0
- `Private/CarFightReEditor.cpp` v1.5.0

Technical evidence:

- first implementation official Editor build `5c70f20492cb493fa576a374191c69b7` / ExitCode 0 / PASS
- first test build `da6057ab53404bc5ab33dec5f22b8420`은 test-only `FCFDADomain` typo 1건으로 compile fail → `ECFDADomain`으로 즉시 교정
- final official Editor build `f85846158f2e4ff5b3fd9a8a0abdbbf7` / ExitCode 0 / PASS
- DAM-P0-03 focused `ca894ff0112c4c089038326ed00cd4e5` / 3 PASS / 0 FAIL
  - `InventoryFilter`
  - `CacheReference`
  - `TabSurface`
- affected DAM-P0-02C `fbcdd7cf8a5640f19c847c8b3c24a99c` / 3 PASS / 0 FAIL
- affected DAM-P0-02B `fe1a8fa36d364215bd2b3fde3924bd6d` / 2 PASS / 0 FAIL
- affected DAM-P0-02A `b5f38489d2db4e8291cc36fb50f5f065` / 2 PASS / 0 FAIL
- affected DAM-P0-01 `6fa1242cc03e4314b8821b7a575b7039` / 3 PASS / 0 FAIL
- current related Automation total 13 PASS / 0 FAIL

Implementation boundary audit:

- base metadata Refresh object load = 0
- DataManagement `TryLoad()` = 2 explicit action only
  - HealthService Validate
  - Manager Asset Open
- `GetDependencies()` / `GetReferencers()` = ReferenceService on-demand 각 1곳
- `SavePackage()` / `MarkPackageDirty()` / `Modify()` = 0
- Slate가 HealthService/ReferenceService/AssetRegistry를 직접 호출하지 않고 VM을 경유
- static validation/reference cache = 0

Mid-review findings:

- **P1-1 — Unregistered Validate success wording**
  - P0-02C의 Unregistered contract는 `EvaluationState=PolicyUnavailable / Health=NotValidated`이며 policy non-authoritative다.
  - 그러나 `FCFDAManagementVM::ValidateSelectedAsset()`은 `PolicyUnavailable`을 failure/non-conclusive 분기에 포함하지 않아 `true`를 반환하고, Slate가 `선택 Asset Validate 완료`로 표시한다.
  - future Unregistered DA를 실제로 검증 완료한 것처럼 오해시킬 수 있으므로 P0-03 Technical PASS 전 교정 필요.
- **P2-1 — filter/search 뒤 hidden Asset selection 유지**
  - 검색/Domain/Scope/Health filter로 selected Asset row가 목록에서 사라져도 VM selection/Detail/action이 유지된다.
  - mutation 위험은 없지만 현재 보이는 목록과 Detail/action 대상이 어긋날 수 있다.
- **P2-2 — Type View 전환 시 이전 Asset action 유지**
  - Asset을 선택한 뒤 Type View 버튼만 누르면 이전 Asset selection과 Validate/Reference/Open/Sync action이 계속 활성화된다.
  - Type row를 새로 선택하면 정리되지만 view switch 자체는 selection을 정리하지 않는다.

Mid-review 판정: **P0 0 / P1 1 / P2 2 — Correction Required.**

Correction / re-review — source PASS, full verification pending:

1. **P1-1 PolicyUnavailable Validate 오표시 교정**
   - `FCFDAManagementVM::IsConclusiveValidationResult()`를 추가해 `EvaluationState==Succeeded`일 때만 Validate action을 완료로 인정한다.
   - Unregistered future DA의 `PolicyUnavailable / Health=NotValidated`는 loaded result cache에는 보존하지만 action은 false를 반환한다.
   - Status는 raw `Typed Semantic descriptor...`보다 `이 DataAsset 타입은 아직 관리 검증 정책이 등록되지 않아 검사를 완료하지 않았습니다.`처럼 사용자 의미를 우선한다.
   - DA Health가 Error여도 evaluation 자체가 Succeeded이면 `검사를 정상 완료했고 실제 DA 오류를 발견한 것`이므로 conclusive success를 유지한다.
2. **P2 selection context 교정**
   - Search / Domain / Scope / Health filter 변경마다 `ReconcileSelectionWithCurrentAssetFilters()`를 실행한다.
   - selected Asset이 current filtered Asset View에서 사라지면 VM selection과 Slate selection을 함께 해제한다.
   - Validate 결과로 Health가 바뀌어 current Health filter에서 row가 사라지는 경우도 즉시 selection을 재조정한다.
   - Type View 전환 시 이전 Asset selection/action context를 제거하고, Asset View 전환 시 이전 Type Detail selection을 제거한다.
   - filter/search로 selected Type이 숨겨지는 경우 `ReconcileVisibleSelections()`가 Type Detail selection도 해제한다.
   - `SelectAsset()` 자체도 current filter에서 보이지 않는 row를 선택하지 못하게 fail-closed 처리한다.
3. **Correction re-review 중 신규 P1 — Reference query successful-empty 오판 교정**
   - UE 5.8 local Engine Source를 read-only 대조했다.
   - `FAssetRegistryState::GetDependencies()` / `GetReferencers()`는 `CachedDependsNodes`에 해당 node가 있으면 empty list여도 `true`, node가 없으면 `false`다.
   - 기존 P0-03 구현은 bool 반환을 무시해 `false + empty`도 `No Known Referencer`로 표시할 수 있었다.
   - `FCFDAReferenceService` v1.1.0에서 `bReferenceEvidenceAvailable`, `bReferencerEvidenceAvailable`을 결과에 보존하고 `FinalizeQueryEvidence()`로 의미를 단일화했다.
   - 두 query가 모두 성공한 경우에만 empty Referencer를 `NoKnownReferencer`로 확정한다. 어느 한쪽이라도 false면 `QueryFailed`로 유지하며 0건으로 해석하지 않는다.
4. **focused test source 보강**
   - PolicyUnavailable non-conclusive
   - Succeeded + Health Error conclusive
   - search/filter hidden Asset selection cleanup
   - Type/Asset View context cleanup
   - Validate-driven Health filter selection cleanup
   - successful-empty Reference → `NoKnownReferencer`
   - missing Referencer node evidence → `QueryFailed`
   - non-empty successful Referencer evidence → `Referenced`

Correction compile evidence:

- full official build `b4c3c195a1a04ba3ad12f2a852fa1915` / ExitCode 6 — **parallel CF-FQ-046 `CFVehicleBuilderTab.cpp:1352` compile failure**, DAM source가 원인이 아님.
- full official build retry `6771042f0a3d4b1e8fe11f2366e02f84` / ExitCode 6 — 같은 protected parallel error.
- P0-03 correction source isolation UBT `-SingleFile` job `e6f20c90fea64b25a15fb20cfeff57db` / **VM + Tab + Tests compile 3/3 PASS**.
- Reference evidence correction 뒤 final isolation UBT job `5d92e03cfba442d89c5871e17c6ced2d` / **ReferenceService + VM + Tab + Tests compile 4/4 PASS**.
- parallel CF-FQ-046 파일이 갱신된 뒤 full official build `f44d982cacbb43fa9503e901112ef1d9`를 재시도했으나 동일한 `CFVehicleBuilderTab.cpp:1352` compile failure로 차단됐다. 해당 파일은 CF-FQ-045에서 수정하지 않았다.
- USER가 Editor를 종료한 뒤 fresh status를 다시 확인하고 official build `6b1839fc8f364dd18f6127f7800222a4`를 재시도했다. Live Coding/DLL lock 없이도 `CFVehicleBuilderTab.cpp:1354`의 동일 계열 Slate 구문 오류로 ExitCode 6이 재현되어, blocker가 Editor ownership이 아니라 protected parallel CF-FQ-046 source compile error임을 확정했다.
- final source/boundary re-review: **P0 0 / P1 0**.
  - `SavePackage` / `MarkPackageDirty` / `Modify` = 0
  - base metadata Refresh load/reference = 0
  - `TryLoad` = explicit Validate + Asset Open 두 action 지점만
  - `GetDependencies` / `GetReferencers` = ReferenceService on-demand 각 1곳
  - static validation/reference cache = 0

Verification Closure — PASS:

- pre-correction implementation build `f85846158f2e4ff5b3fd9a8a0abdbbf7`와 당시 DAM Automation 13/13 PASS는 initial implementation baseline evidence로 유지한다.
- USER 요청으로 병렬 CF-FQ-046 compile blocker를 최소 범위 교정했다.
  - `CFVehicleBuilderTab.cpp` Step 3 Socket 진단용 `SVerticalBox::Slot()`의 누락된 닫힘 `]` 1개를 복구했다. source v1.30.1 changelog에 기록했다.
  - `CFVehicleBuilderTab.h`의 Browser row callback 시그니처가 사용하는 `ITableRow` / `STableViewBase` forward declaration을 복구했다. header v1.22.1 changelog에 기록했다.
  - Builder logic/state/mutation authority는 변경하지 않았다.
- blocker fix single-file UBT `35d290e84c2246bfb68b441dabb34869` / `CFVehicleBuilderTab.cpp` compile PASS.
- **final Official Editor Build `de5aeba4c2b54f2dbd27e2778d07cfb6` / ExitCode 0 / PASS**.
- corrected DAM-P0-03 focused `552ad4c11d6543fdaf8bbbc1b8083ac7` / **3 PASS / 0 FAIL**.
  - `CacheReference`
  - `InventoryFilter`
  - `TabSurface`
- corrected affected DAM-P0-02C `c925c6d705634af1a84a5251cb0501ef` / **3 PASS / 0 FAIL**.
- corrected affected DAM-P0-02B `2fae6d2e0df1438fbccd1d6c85512301` / **2 PASS / 0 FAIL**.
- corrected affected DAM-P0-02A `4f8d772088dd48fb82821d8f33af44ef` / **2 PASS / 0 FAIL**.
- corrected affected DAM-P0-01 `e7d58e884e8149f6ae3c9283154254a1` / **3 PASS / 0 FAIL**.
- linked final binary 기준 DAM related Automation **13 PASS / 0 FAIL**.
- final source/boundary re-review: **P0 0 / P1 0**.
  - `SavePackage` / `MarkPackageDirty` / `Modify` = 0
  - base metadata Refresh object load/reference = 0
  - `TryLoad` = explicit Validate + Asset Open 두 action 지점만
  - `GetDependencies` / `GetReferencers` = ReferenceService on-demand 각 1곳
  - static validation/reference cache = 0
  - `PolicyUnavailable` non-conclusive, hidden selection reconciliation, successful-empty vs query-failed Reference evidence regression coverage 유지

**DAM-P0-03 Manager UI + On-demand Detail Technical PASS.**

Next exact internal action: `DAM-P0-04 Acceptance`.

### DAM-P0-04 — Acceptance

대표 검증:

- Vehicle / Combat / Targeting / UI / Authoring Domain
- Test / Legacy / RuntimeContent / Unclassified Scope
- concrete vs abstract type presentation
- instance 0 type
- Unregistered type scenario
- Stable ID NotResolved → Resolved lifecycle
- duplicate ID focused scenario
- Registry Gathering state
- lazy Reference query
- Refresh 후 stale validation cache reset
- no-auto-save / no-mutation

Unregistered/zero-instance/duplicate 같은 edge case를 검증하기 위해 Product module에 가짜 DA class를 추가하지 않는다. Discovery 결과와 pure analyzer 사이의 DTO seam을 두고 synthetic class/asset rows를 Automation에 주입한다. 실제 28/53 baseline은 별도 integration evidence로 확인한다.

USER acceptance 핵심은 **구조를 외우지 않고도 원하는 DA를 찾고 역할과 상태를 이해할 수 있는가**다.

**Technical Acceptance PASS — 2026-09-03**

- final linked Editor binary 기준 DAM 관련 Automation 13/13 PASS 증거를 P0-04 technical acceptance evidence로 채택한다.
- Vehicle / Combat / Targeting / UI / Authoring Domain 존재를 Manager focused Automation에서 확인했다.
- Test / Legacy / RuntimeContent / Unclassified Scope 분류를 Inventory Automation에서 확인했다.
- concrete/abstract, zero-instance, Unregistered future type, Registry Gathering, duplicate ID, Stable ID resolve, lazy Reference, Refresh cache invalidation을 synthetic/integration seam에서 모두 확인했다.
- current concrete CarFight DA 27종은 Registered semantic coverage이며 abstract `CFInventoryItemData`만 의도적으로 Unregistered framework base로 남는다.
- base Refresh의 object load/reference 0, `SavePackage`/`MarkPackageDirty`/`Modify` 0, explicit Validate/Asset Open `TryLoad` 2곳과 on-demand Reference query 각 1곳 경계를 재확인했다.
- P0-04 시작/종료 fresh Git status를 대조했으며 Acceptance 검증 때문에 새 Product Asset dirty가 추가되지 않았다.

Representative USER Acceptance checklist:

1. `Window > CarFight > CarFight 데이터 관리`를 연다.
2. Overview에서 Type/Asset 수와 Error/Warning/Not Validated 요약이 이해 가능한지 본다.
3. Type View에서 `Vehicle` Domain을 선택하고 기술 클래스명을 몰라도 설명을 보고 차량 관련 데이터 타입을 찾을 수 있는지 본다.
4. Asset View로 전환해 검색/Domain/Scope/Health filter가 무엇을 줄이는지 직관적으로 이해되는지 본다.
5. Asset 하나를 선택해 Detail의 역할/경로/Health/Stable ID/Duplicate 상태가 구분되어 보이는지 확인한다.
6. `Validate`를 한 번 눌러 상태 변화가 이해되는지 확인한다. Unregistered 타입은 완료로 오표시하지 않아야 한다.
7. `References`를 눌러 `Unknown / Not Queried`에서 조회 결과로 바뀌고 `No Known Referencer`가 삭제 가능 판정처럼 보이지 않는지 확인한다.
8. `Refresh` 후 이전 loaded/reference 결과가 초기화되는 것이 이해 가능한지 확인한다.
9. 전체적으로 **'DA 구조를 외우지 않아도 찾고 역할/상태를 이해할 수 있다'**고 판단되면 USER PASS다.

현재 Gate: **DAM-P0-04B/C/D Technical PASS / DAM-P0-04E USER Re-Acceptance PASS / CF-FQ-045 P0 Complete.** 최초 USER Acceptance FAIL과 중간 partial feedback는 pre-correction historical evidence로 보존하며, 현재 구현 owner는 `Document/Systems/DataManagement/DataAssetManagement.md v1.0.0`으로 승격했다.

#### DAM-P0-04 USER Acceptance 결과 — FAIL

USER가 실제 `CarFight 데이터 관리` 기본 화면을 열어 representative Acceptance를 시작한 결과, 기능 자체는 노출되지만 **관리 업무용 정보 구조와 시선 흐름이 불명확해 처음 봤을 때 무엇을 어떻게 읽어야 하는지 알기 어렵다**고 판정했다.

대표 관찰:

- Type View가 실제 열(column) 기반 표가 아니라 `이름 | Domain | Asset 수 | Registered` 식의 한 줄 문자열 목록이라 각 정보의 시작 위치가 행마다 달라 반복 비교가 어렵다.
- Overview의 `Generation / Type / Asset / Registered / Unregistered / Instance 0 / Not Validated`는 기술 telemetry 성격이 강하고 사용자가 우선 판단해야 할 관리 상태와 행동 필요성을 즉시 전달하지 못한다.
- Type Detail의 `기술 클래스 / Domain / Coverage / Abstract / Persisted Asset / Instance 상태`가 사용자 설명과 같은 계층에 놓여 무엇이 우선 정보인지 불명확하다.
- Type을 선택한 상태에서도 Asset 전용 `Validate / References / Asset 열기 / Content Browser`가 disabled 상태로 노출되어 왜 사용할 수 없는지 추가 해석을 요구한다.
- `CFInventoryItemData`, `InputAction`, `InputMappingContext` 같은 서로 다른 의미의 Unregistered 유형이 동일한 시각 계층에 노출되어 `Unregistered`가 곧 관리 오류처럼 보일 수 있다.
- 검색/필터 위치, 좌측 목록-우측 Detail 구조, Type/Asset View 분리 방향 자체는 유지 가치가 있다.

USER 표현의 핵심은 **'엑셀처럼 칸이 깔끔하게 나눠져 있지 않고 시선의 흐름 자체가 중구난방으로 흘러가서, 딱 열어보고 잘 모르겠다'**는 것이다.

이 Acceptance FAIL은 backend 기능 부족이나 Runtime/Product Asset 결함으로 해석하지 않는다. 원인은 **presentation information architecture / table UX**이며 P0-03의 discovery/registry/health/reference/identity 계약은 유지한다.

#### DAM-P0-04A — Information Architecture Lock

**Design PASS — 2026-09-03**

핵심 원칙:

1. **같은 종류의 정보는 항상 같은 화면 위치에 표시한다.** 반복 비교되는 값은 실제 column으로 정렬하고 문자열 구분자 `|`에 의존하지 않는다.
2. **사용자가 먼저 알아야 할 정보와 개발자가 진단할 때 필요한 정보를 같은 시각적 계층에 두지 않는다.** 사용자 의미가 기본 surface, technical metadata는 secondary/diagnostic surface다.
3. **발견(discovery)과 기본 표시(presentation)를 분리한다.** Generic UDataAsset discovery 계약은 그대로 유지하되 기본 Manager 화면은 CarFight 관리 대상 중심으로 보여준다.
4. **Unregistered는 Error가 아니다.** Framework/기타 UDataAsset/관리 규칙 필요 상태를 presentation에서 구분한다.
5. **이번 correction은 read/manage UX다.** DA create/delete/rename/move, inline edit, bulk edit, auto-fix, auto-save는 열지 않는다.

##### P0-04A-1 — 화면 정보 흐름

기본 시선 흐름을 다음으로 고정한다.

```text
1. 관리 상태 요약
   ↓
2. 검색 / 현재 View에 맞는 필터
   ↓
3. 실제 Multi-column 관리표
   ↓ 행 선택
4. 우측 사용자용 Detail
   ↓ 필요 시
5. 기술 정보 펼치기
```

좌측 목록-우측 Detail의 60/40 계열 분할 방향은 유지하되, 표와 Detail의 역할을 겹치지 않게 한다.

##### P0-04A-2 — Overview 계약

기본 Overview에서 다음 raw telemetry는 사용자 1차 정보에서 제외한다.

- InventoryGeneration
- raw Type count
- raw Registered/Unregistered count
- raw Instance 0

기본 Overview는 행동/관리 의미 중심으로 표현하며 **management universe만 집계**한다.

```text
관리 대상 Asset
문제 발견
미검사
관리 규칙 필요 유형
```

집계 계약:

- `관리 대상 Asset` = Managed 또는 NeedsManagementRule 타입에 속한 persisted Asset 수
- `문제 발견` = 관리 대상 Asset 중 loaded Health가 Error 또는 Warning인 Asset 수
- `미검사` = 관리 대상 Asset 중 Health=NotValidated인 Asset 수
- `관리 규칙 필요 유형` = NeedsManagementRule Type 수

`NotValidated`는 Error가 아니므로 `문제 발견`과 분리한다. `전체 발견 Asset`, InventoryGeneration, raw Type count, Framework/Auxiliary count 같은 값은 기술 정보/진단 영역에서만 확인한다.

##### P0-04A-3 — Type View는 실제 Multi-column Table

현재 `SListView` 행에서 `FString::Printf("%s | %s | ...")`로 표시하는 방식은 폐기한다.

구현 계약:

```cpp
SHeaderRow
SListView<TSharedPtr<FCFDAManagerTypeRow>>
SMultiColumnTableRow<TSharedPtr<FCFDAManagerTypeRow>>
```

기본 column:

| Column | 사용자 표시 | 기본 역할 |
| --- | --- | --- |
| Type | 데이터 유형 | 사람이 읽는 Type 이름 |
| Domain | 영역 | 차량 / 전투 / 타게팅 / UI / 제작 등 |
| Purpose | 용도 | 한 줄 사용자 역할 설명 |
| AssetCount | Asset 수 | persisted instance 개수 |
| ManagementState | 관리 상태 | 관리됨 / 관리 규칙 필요 / 프레임워크 / 기타 DataAsset |

`ClassPath`, `Abstract`, raw `Coverage`는 기본 column에 두지 않는다.

##### P0-04A-4 — Asset View는 별도 Multi-column Table

Asset View는 Type View column을 그대로 재사용하지 않는다.

기본 column:

| Column | 사용자 표시 |
| --- | --- |
| AssetName | Asset 이름 |
| TypeDisplayName | 데이터 유형 |
| Domain | 영역 |
| Scope | 사용 범위 |
| Health | 상태 |
| StableId | Stable ID |

`ObjectPath`는 기본 column이 아니라 Detail 기술 정보에서 보여준다.

##### P0-04A-5 — presentation state 분리

core `CoverageState`는 바꾸지 않고 Manager 전용 presentation state를 파생한다.

권장 구조:

```cpp
enum class ECFDAManagerTypePresentationState : uint8
{
    Managed,
    NeedsManagementRule,
    Framework,
    AuxiliaryDataAsset
};
```

파생 원칙은 `FCFDATypeRecord::bCanonicalNative`와 `bAbstract`를 authority로 사용한다.

```text
Coverage=Registered
→ Managed

Coverage=Unregistered
+ bCanonicalNative=true
+ bAbstract=false
→ NeedsManagementRule

Coverage=Unregistered
+ bCanonicalNative=true
+ bAbstract=true
→ Framework

bCanonicalNative=false
→ AuxiliaryDataAsset
```

`bAbstract`만으로 Framework를 판정하지 않는다. non-CarFight abstract UDataAsset이 발견되더라도 AuxiliaryDataAsset이어야 한다.

core `FCFDATypeRecord`에는 이미 `bCanonicalNative` / `bAbstract`가 있으므로 discovery DTO에 새로운 module/source provenance를 추가하지 않는다.

기본 화면에서는 `Managed + NeedsManagementRule`을 **기본 관리 universe**로 사용한다. `Framework + AuxiliaryDataAsset`는 `전체 발견 보기` 같은 명시적 session-local toggle/filter에서 포함한다. Generic discovery 자체는 절대 축소하지 않는다.

기본 management universe와 전체 discovery universe를 숫자/필터/검색에서 혼용하지 않는다.

- 기본 Type/Asset table, Overview 관리 수치 → management universe 기준
- `전체 발견 보기` ON → Framework/Auxiliary까지 list/search 대상으로 확장
- 전체 discovery raw Type/Asset 수 → 기술 정보/진단에서만 확인

`관리 규칙 필요`는 Asset 개수가 아니라 **Type 개수**이므로 UI label을 `관리 규칙 필요 유형`처럼 단위를 명확히 한다.

##### P0-04A-6 — View별 필터 의미

Type View와 Asset View가 동일 필터 집합을 공유하지 않는다.

Type View:

```text
검색
영역
관리 상태
전체 발견 보기
```

Asset View:

```text
검색
영역
사용 범위
상태
전체 발견 보기
```

UI 표시명:

```text
Domain → 영역
Scope  → 사용 범위
Health → 상태
```

내부 enum/API 이름은 그대로 유지할 수 있다.

##### P0-04A-7 — Detail 정보 계층

Type Detail의 기본 순서:

```text
[Type 이름]
[영역] [관리 상태]

어떤 데이터인가
사용자용 설명

어디에 사용되는가
사용자용 사용처 설명

현재 상태
Asset 수 / 관리 상태

▶ 기술 정보
  ClassPath
  Coverage
  Abstract
  Persisted Asset
  Instance State
  Identity/Validation policy 필요 시 표시
```

Asset Detail은 사용자용 기본 정보 뒤에 Health / Stable ID / Duplicate / Reference 의미를 배치하고 ObjectPath/ClassPath 같은 기술값은 진단 정보로 내린다.

Type View에서는 Asset 전용 action을 **disabled로 보여주지 않고 아예 렌더링하지 않는다.** `Validate / References / Asset 열기 / Content Browser`는 Asset Detail에서만 표시한다.

##### P0-04A-8 — semantic 설명 보강

현재 `RoleDescription`만으로 사용자 설명과 technical contract를 모두 담당시키지 않는다.

권장 semantic descriptor 확장:

```cpp
FString UserPurposeDescription;
FString UserUsageDescription;
FString RoleDescription; // technical/detail
```

현재 27개 Registered CarFight concrete type에 대해 사용자용 목적/사용처 설명을 제공한다. 이 설명은 Manager presentation owner가 사용하고 Runtime DA class에는 넣지 않는다.

Registry foundation 전체에 새 필드를 무조건 required로 만들어 synthetic/future descriptor 등록을 깨뜨리지 않는다. 대신 P0-04C에서 **current 27 registered descriptor는 두 사용자 설명이 모두 non-empty**라는 focused coverage를 추가한다. Unregistered fallback은 `관리 설명이 아직 등록되지 않았습니다.` 같은 안전한 presentation fallback을 사용한다.

##### P0-04A-9 — 정렬

관리표 성격상 최소 column sort는 correction 범위에 포함한다.

Type View 최소 정렬:

- 데이터 유형
- 영역
- Asset 수
- 관리 상태

Asset View 최소 정렬:

- Asset 이름
- 데이터 유형
- 영역
- 사용 범위
- 상태

정렬 상태는 session-local Manager VM/presentation owner가 소유하고 deterministic stable sort를 사용한다.

- Type row tie-breaker: canonical `ClassPath`
- Asset row tie-breaker: canonical `ObjectPath`
- `용도` column은 한 줄 ellipsis + full tooltip을 기본으로 해 긴 설명이 row 높이를 흔들지 않게 한다.
- 상태는 텍스트를 항상 표시하고 색은 보조 수단으로만 사용한다.

column layout 저장, 사용자별 column customization, Excel export/import는 P0 범위 밖이다.

##### P0-04A-10 — 비범위

이번 correction에서 열지 않는다.

- DA inline editing
- create/delete/rename/move
- drag/drop
- bulk edit
- Stable ID auto-fix
- Runtime Catalog auto-add
- auto-save
- dashboard chart
- graph visualization
- Excel import/export
- 사용자별 column layout persistence
- Runtime/Product Asset mutation

##### P0-04A-11 — 구현 owner와 C++/BP 경계

구현은 Editor C++만 사용한다. Blueprint 수정은 0이다.

주요 수정 owner 후보:

```text
CFDAManagementTab.h/.cpp
CFDAManagementVM.h/.cpp
CFDATypeRegistry semantic descriptor
CFDAManagementTests.cpp
필요 시 Manager 전용 presentation DTO/header
```

원칙적으로 건드리지 않는다.

```text
Runtime DA classes
Product .uasset
Vehicle Builder logic
Runtime Apply
Health core semantics
Reference core query semantics
Stable ID resolver
Asset Registry discovery core
```

##### P0-04A-12 — 내부 Gate

| Gate | 내용 | 종료 조건 |
| --- | --- | --- |
| `DAM-P0-04A` | Information Architecture Lock | **Design PASS** |
| `DAM-P0-04B` | Type/Asset Multi-column Table Surface | **Technical PASS** — actual column/header + view-specific filter + sortable header |
| `DAM-P0-04C` | Information Hierarchy + User Semantics | **Technical PASS** — Overview/Detail/presentation state/current 27 사용자 설명 교정 |
| `DAM-P0-04D` | Technical Verification | **PASS** — mid-review P1 4 correction + official build + corrected focused/affected 6/6 + final re-review P0/P1 0 (P2 4 non-blocking) |
| `DAM-P0-04E` | USER Re-Acceptance | **PASS** — 최신 한글 우선 화면, `직접 에셋 생성`, Refresh 후 `재검사 필요 / 재확인 필요` 표현과 전체 정보 구조를 USER가 직접 확인 후 승인 |

##### P0-04A-13 — 병렬 작업 HOLD / bounded release

초기 계약은 `CF-FQ-047 Vehicle Builder Hardpoint Authoring Integrity` 완료 전 **DAM-P0-04B 이후 소스 구현 HOLD**였다. 목적은 기능 dependency가 아니라 shared Editor/Plan dirty와 병렬 verification 충돌을 피하는 것이었다.

2026-09-03 USER가 CF-FQ-046 HOLD 상태에서 CF-FQ-045 진행을 승인한 뒤 fresh preflight를 다시 수행했다. 그 결과 CF-FQ-047의 남은 Gate가 DataAuthoring/Product Wagon recovery 영역에 있고, DAM-P0-04B~D의 수정 owner는 `DataManagement` Editor C++ 전용으로 분리 가능함을 확인했다. 이에 아래 보호 조건으로 **P0-04B~D에 한해 bounded HOLD를 해제**했다.

- `DataManagement` + DAM runner/test만 수정한다.
- CF-FQ-047 `DataAuthoring` source와 Wagon Product Asset은 수정하지 않는다.
- CF-FQ-046 source/asset도 수정하지 않는다.
- build/test 전후 fresh status로 병렬 dirty를 보존한다.
- Runtime/Product Asset mutation/save는 0을 유지한다.

P0-04B~D 완료 후에는 더 이상 병렬 Source implementation을 넓히지 않고 `DAM-P0-04E USER Re-Acceptance`만 남긴다.

#### DAM-P0-04A Design Audit / Correction Re-review

read-only current Source audit 결과:

- `FCFDAManagerTypeRow`와 `FCFDAManagerAssetRow`가 presentation 확장을 수용할 수 있으며 core inventory DTO를 재설계할 필요는 없다.
- `FCFDATypeRecord`는 이미 `bCanonicalNative`와 `bAbstract`를 보유하므로 Managed/NeedsManagementRule/Framework/Auxiliary 분류에 필요한 provenance가 충분하다.
- current Manager는 Type/Asset 공통 `Search/Domain/Scope/Health` filter state를 쓰므로 P0-04B에서 view-specific presentation filter state로 분리할 필요가 있다.
- current semantic descriptor는 `TypeDisplayName + RoleDescription`만 갖고 있어 P0-04C의 사용자 목적/사용처 설명을 별도 metadata로 추가하는 것이 적절하다.
- current UI는 `GenerateTypeRow/GenerateAssetRow`에서 문자열 summary를 생성하므로 Multi-column row 교체 지점이 명확하다.

설계 감사 교정:

- **P1-D1**: `Unregistered + abstract → Framework`만으로 정의하면 non-CarFight abstract 타입을 잘못 Framework로 볼 수 있음 → `bCanonicalNative` 우선 규칙으로 교정.
- **P1-D2**: Overview의 `관리 Asset/관리 규칙 필요` 집계 universe와 단위가 모호함 → management universe + Asset/Type 단위를 명시.
- **P2-D1**: 사용자 설명 필드를 Registry global required로 만들면 synthetic/future descriptor compatibility를 깨뜨릴 수 있음 → current 27 coverage test로 제한.
- **P2-D2**: 정렬 동률과 긴 Purpose row의 안정성 미정 → canonical tie-breaker + single-line ellipsis/tooltip 고정.

교정 후 설계 재검수: **P0 0 / P1 0 — PASS.**

##### P0-04A Implementation Test Matrix

`DAM-P0-04B` focused:

- Type table이 `SHeaderRow + SMultiColumnTableRow` actual column surface를 사용한다.
- Asset table이 별도 actual column surface를 사용한다.
- 기존 `" | "` concatenated row presentation이 0이다.
- default management universe에서 AuxiliaryDataAsset/Framework가 숨겨지고 `전체 발견 보기`에서 다시 보인다.
- `bCanonicalNative/bAbstract/Coverage` 조합 4종의 presentation state가 exact하다.
- Type/Asset view별 filter set이 서로 오염되지 않는다.
- column sort가 canonical tie-breaker로 deterministic하다.

`DAM-P0-04C` focused:

- current 27 registered descriptors의 `UserPurposeDescription/UserUsageDescription` coverage 27/27.
- Unregistered fallback 사용자 설명이 technical class string만 노출하지 않는다.
- Overview 4개 수치가 management universe 기준으로 계산된다.
- `NotValidated`가 `문제 발견`에 포함되지 않는다.
- Type Detail에는 Asset action widget이 존재하지 않고 Asset Detail에서만 action이 노출된다.
- ClassPath/Coverage/Abstract/ObjectPath 등 technical metadata는 기본 사용자 설명보다 secondary section에 있다.
- search가 Type 이름뿐 아니라 user purpose/usage 설명도 찾을 수 있다.

`DAM-P0-04D` verification:

- 최초 구현 verification PASS 뒤 latest mid-review에서 **P0 0 / P1 4 / P2 4**를 발견해 correction-required로 재개했다.
- P1 4건을 bounded DataManagement Presentation correction으로 교정했다: 사용자 상태 문구의 `InventoryGeneration` 제거, Stable ID의 `미확인 / 해당 없음 / 필수 ID 누락 / 실제 값` 구분, `No Known Referencer`의 삭제 안전 비보장 안내, list regeneration/sort 뒤 Type/Asset Slate highlight와 Detail selection key 재결합.
- corrected official Editor build `9faa491f7cc144ba9aad50dd18de069b` PASS.
- corrected focused/affected는 P0-04D 1/1 + P0-04C 1/1 + P0-04B 1/1 + P0-03 3/3 = **6/6 PASS**다. 기존 lower backend P0-02C 3/3 + P0-02B 2/2 + P0-02A 2/2 + P0-01 3/3 evidence는 이번 Presentation-only 변경으로 계약이 바뀌지 않아 유지한다.
- final re-review는 **P0 0 / P1 0 / P2 4**다. P2는 기본 액션 용어 정리, Detail section widget화, 검색 힌트 user semantic화, active Type/Asset View 시각 표시이며 P0-04D를 차단하지 않는 USER-facing polish debt로 남긴다.
- DataManagement Source의 `SavePackage` / `MarkPackageDirty` / `Modify()`는 0이고, 이번 correction의 Product Asset mutation/save도 0이다.

`DAM-P0-04E` USER re-acceptance:

- 2026-09-04 USER partial feedback: 전체 표/정보 구조는 **"확실히 보기 편해졌다"**고 확인했다. 다만 `기술 정보` 내부의 영어 상태/레이블과 Asset View 액션의 영어 용어가 이해를 방해하므로 USER PASS는 아직 보류한다.
- feedback correction source에서 사용자-facing 문구를 한글 우선으로 교정했다. `Refresh/Type View/Asset View/Validate/References/Content Browser`, Stable ID/Coverage/Evaluation/Duplicate/Referencer 상태 의미, 참조 관계 성공·실패 문구, Registry 오류를 사용자 표현으로 전환했다.
- `/Game/...`, `/Script/...`, 실제 Object/Class Path처럼 검색·디버깅에 필요한 기술 식별값은 원문을 유지하되 `오브젝트 경로 (Object Path)`, `클래스 경로 (Class Path)`처럼 한글 의미를 병기한다.
- 기존 P2 4건 중 `기본 액션 영어 용어 정리`와 `검색 힌트 사용자 의미화`는 source 반영 완료다. 남은 non-blocking P2는 `Detail 실제 section widget화`, `active Type/Asset View 시각 표시` 2건이다.
- 첫 official build job `917e258f6592448a895892beb69a79f8`은 변경 C++ compile까지 통과했지만 당시 실행 중 Editor의 `UnrealEditor-CarFight_ReEditor.dll` 점유 때문에 final link가 LNK1104로 차단됐다. Editor는 자동 종료하지 않았다.
- 이후 USER가 Editor 종료 후 linked build 성공을 확인했다. 새 linked binary 기준 affected Automation은 P0-04D 1/1 + P0-04C 1/1 + P0-04B 1/1 + P0-03 3/3 = **6/6 PASS**다.
- P0-04D `PresentationCorrection`은 이번 한글화의 기술 정보 한글 레이블/상태값, 조회 전 참조 관계 한글 문구, 기존 selection/detail 동기화 회귀까지 포함해 PASS했다.
- DataManagement Source의 `SavePackage` / `MarkPackageDirty` / `Modify()` 검색 결과는 계속 0이며 Product Asset mutation/save 0 경계를 유지한다.
- 추가 USER 피드백을 반영해 `추상 유형`처럼 구현 내부 관점의 표현은 `직접 에셋 생성: 가능/불가`로 사용자 판단 의미에 맞게 교체했다.
- Refresh 이후 이전 generation의 실제 검사값은 계속 현재값으로 재사용하지 않는다. 대신 이전에 검사했던 ObjectPath만 session-local `재검사 필요 / 재확인 필요` 상태로 표시해 `한 번도 검사하지 않음`과 구분하며, 다시 Validate하면 해당 stale 표시를 해제한다.
- Overview/Health filter의 `미검사` 상위 의미는 `검사 필요`로 교정해 최초 미검사와 Refresh 후 재검사 필요를 모두 자연스럽게 포함한다.
- correction official Editor build `51326d9704ac4a51b2febf6332d5e959` / ExitCode 0 / **PASS**. `CFDAManagementVM.cpp`, `CFDAManagementTab.cpp`, `CFDAManagementTests.cpp`가 실제 재컴파일되고 final DLL link까지 완료됐다.
- correction linked binary 기준 P0-04D `9659bc57dc9c4f56a3713b608a769777` 1/1 PASS, P0-04C `b18214e55f704a31a89677449c55fc5a` 1/1 PASS, P0-04B `38c0449d0d7d433b98af34b1812b683a` 1/1 PASS, P0-03 `15551c8790344d9fb2d3f69d92739d7b` 3/3 PASS로 affected **6/6 PASS**를 다시 확보했다.
- P0-03 `CacheReference`는 `Validate → Refresh → old loaded result 폐기 + 재검사 필요 표시 → Revalidate → stale 표시 해제` 전이를 실제 Automation으로 검증한다.
- DataManagement 구현의 `SavePackage` / `MarkPackageDirty` / `Modify()` 호출은 계속 0이며 Product Asset mutation/save 0 경계를 유지한다.
- 최종 USER 재검수에서 최신 한글화, `직접 에셋 생성: 가능/불가`, Refresh 후 `재검사 필요 / 재확인 필요`, 재검사 후 current 결과 복원을 확인했고 USER가 **"패스야"**로 DAM-P0-04E를 승인했다.
- 이 승인으로 representative USER Acceptance와 P0 완료 정의 11개를 모두 충족했다.
- Current System은 `Document/Systems/DataManagement/DataAssetManagement.md v1.0.0`으로 승격했다. 잔여 non-blocking P2 2건은 별도 폴리싱 후보이며 CF-FQ-045를 재오픈하지 않는다.

- 첫 화면에서 표의 각 열 의미를 별도 설명 없이 구분할 수 있음
- 차량/전투/UI 등 원하는 영역을 바로 찾을 수 있음
- Type과 실제 Asset의 차이를 화면 구조만 보고 이해할 수 있음
- 선택 항목의 '무엇/어디에 사용/현재 상태'를 기술 클래스 지식 없이 이해할 수 있음
- 기술 정보가 필요할 때만 추가로 볼 수 있음
- 최종 핵심 질문 **'엑셀처럼 비교 기준이 정렬되어 있고 시선이 위→표→상세로 자연스럽게 흐르는가'** USER PASS

Next exact action: **없음 — CF-FQ-045 Done.** 현재 구현은 `Document/Systems/DataManagement/DataAssetManagement.md v1.0.0`을 우선하며, 잔여 P2 2건은 별도 폴리싱 lifecycle이 필요할 때만 연다.

---

## 19. 구현 시작 보호 범위

각 DAM 구현 checkpoint 시작 전 main_game/plan_repo fresh Git status와 관련 dirty diff를 다시 확인한다.

현재 병렬 작업 보호:

- `CF-FQ-039` Production UI Visual Rework Active
- `CF-FQ-041` Runtime Apply Ready dirty
- `CF-FQ-046` Vehicle Builder 사용자 정보 UX Ready dirty
- `CF-FQ-047` Vehicle Builder Hardpoint Authoring Integrity Ready / Product Recovery 진행 전후 dirty
- 기존 Runtime/Asset dirty
- `Document/Plan` 별도 Git 경계의 기존 dirty

CF-FQ-045는 위 Feature의 파일을 편의상 정리하거나 되돌리지 않는다.

DAM-P0-02B는 current concrete DA의 실제 Source 계약을 **read-only audit**하여 Registry semantic mapping만 추가하고 기존 DA Runtime class 수정은 원칙적으로 0이다. Runtime class 변경 필요성이 발견되면 현재 checkpoint에 섞지 않고 별도 bounded change로 Plan에 근거를 먼저 기록한다.

---

## 20. 완료 정의

CF-FQ-045 P0는 다음을 만족할 때 닫는다.

**2026-09-04 최종 판정: 아래 11개 전부 PASS / CF-FQ-045 P0 Complete.**

1. native CarFight DA type과 persisted Asset을 discovery allowlist 없이 자동 발견한다.
2. 신규 DA type이 semantic descriptor 없이도 Unregistered Coverage로 보인다.
3. instance 0 type과 abstract framework type을 Type View에서 구분 확인할 수 있다.
4. 현재 concrete CarFight DA type 전체가 최소 사람용 semantic descriptor를 가진다.
5. 사용자가 기술 클래스명을 외우지 않고 Domain/설명으로 원하는 데이터를 찾을 수 있다.
6. Refresh 기본 화면은 전체 Asset load 없이 구성된다.
7. Coverage / Scope / Health / Stable ID resolve state가 서로 섞이지 않는다.
8. 기존 validation/identity 차이를 타입별로 안전하게 해석한다.
9. Manager가 Asset을 자동 변경/저장하지 않는다.
10. future DA 추가가 Manager UI 구조 수정 없이 기본 discovery에 포함된다.
11. focused automation + representative USER acceptance가 PASS한다.

---

## 21. Changelog

### Maintenance - 2026-09-06

- G5 Physical Move를 완료해 대표 Plan을 `Document/Plan/Archive/DataAssetManagement/DataAssetManagementPlan.md`로 이동했다. DAM-P0-04E USER PASS와 Current System 계약은 변경하지 않는다.
- Migration: 이전 `Document/Plan/DataAssetManagement/DataAssetManagementPlan.md`는 당시 Historical 기록에서만 유효하며 현재 탐색 경로는 Archive 경로다.

### v0.2.0 - 2026-09-04

- `DAM-P0-04E USER Re-Acceptance`를 최종 PASS로 닫았다. USER가 최신 한글화, `직접 에셋 생성: 가능/불가`, Refresh 후 `재검사 필요 / 재확인 필요`와 전체 정보 구조를 직접 확인하고 **"패스야"**로 승인했다.
- P0 완료 정의 11개를 전부 충족해 `CF-FQ-045 CarFight Data Asset Management`를 Done으로 전환했다. 기존 official UE 5.8 Editor build PASS와 affected Automation 6/6 PASS, Product Asset mutation/save 0 evidence는 반복하지 않고 최종 closure evidence로 보존한다.
- 현재 구현 계약을 `Document/Systems/DataManagement/DataAssetManagement.md v1.0.0`으로 승격했다. 이 Plan은 Historical + Retained Path로 전환하며 old next-action은 현재 착수 지시로 사용하지 않는다.
- residual non-blocking P2는 `Detail section widget화`, `active Type/Asset View 시각 표시` 2건이며 별도 폴리싱 lifecycle이 필요할 때만 다룬다.

### v0.1.19 - 2026-09-04

- DAM-P0-04E 추가 USER 피드백을 반영해 `추상 유형`을 `직접 에셋 생성: 가능/불가`로 교체하고, Overview/Health filter의 상위 표현을 `검사 필요`로 정리했다.
- Refresh는 과거 loaded 검사값을 current 결과로 재사용하지 않는 freshness 계약을 유지하면서, 이전에 검사했던 ObjectPath만 session-local `재검사 필요 / 재확인 필요`로 구분한다. 재검사 완료 시 stale 표시는 해제된다.
- official Editor build `51326d9704ac4a51b2febf6332d5e959` PASS. correction linked binary에서 P0-04D 1/1 + P0-04C 1/1 + P0-04B 1/1 + P0-03 3/3 = **6/6 PASS**를 다시 확보했다. P0-03은 Validate→Refresh→재검사 필요→Revalidate 전이를 포함한다.
- DataManagement `SavePackage` / `MarkPackageDirty` / `Modify()` 0과 Product Asset mutation/save 0을 유지한다. USER PASS는 아직 아니며 exact next는 새 Editor에서 최신 표현 재확인이다.

### v0.1.18 - 2026-09-04

- USER가 Editor 종료 후 linked build 성공을 확인했고, Korean-first correction 기준 affected Automation을 재실행해 P0-04D 1/1 + P0-04C 1/1 + P0-04B 1/1 + P0-03 3/3 = **6/6 PASS**로 닫았다.
- P0-04D `PresentationCorrection`에서 기술 정보 한글 레이블/상태값, 참조 관계 조회 전 문구, 기존 selection/detail 동기화가 새 linked binary에서 PASS했다.
- DataManagement `SavePackage` / `MarkPackageDirty` / `Modify()` 0과 Product Asset mutation/save 0 경계를 재확인했다. USER PASS는 아직 아니며 exact next는 새 Editor에서 한글화 화면 재확인이다.

### v0.1.17 - 2026-09-04

- `DAM-P0-04E USER Re-Acceptance`에서 USER가 전체 정보 구조는 "확실히 보기 편해졌다"고 부분 승인했지만 기술 정보와 Asset View 액션의 영어 표현을 추가 comprehension blocker로 지적했다. USER PASS는 아직 보류한다.
- DataManagement Presentation/VM/Reference/Audit user-facing 문구를 한글 우선으로 교정했다. 기술 상태값, 버튼, 검색 힌트, 참조 관계, 성공/실패 안내를 한글화하고 실제 Object/Class Path와 C++ 같은 식별·기술 키워드만 원문 또는 병기로 유지했다.
- 기존 P2 4건 중 액션 영어 용어와 검색 힌트 2건은 source 반영 완료이며 residual P2는 Detail section widget화와 active Type/Asset View 시각 표시 2건이다.
- official build `917e258f6592448a895892beb69a79f8`은 변경 C++ compile까지 PASS했으나 실행 중 Editor가 `UnrealEditor-CarFight_ReEditor.dll`을 점유해 final DLL link만 LNK1104로 차단됐다. user-owned/current Editor를 자동 종료하지 않았고 새 linked binary 전 Automation은 실행하지 않았다.
- Product Asset mutation/save 0, CF-FQ-046/047 병렬 dirty 보존, commit/push 0을 유지한다. linked build와 affected Automation 6/6은 완료됐고 exact next는 USER 한글화 화면 재확인이다.

### v0.1.16 - 2026-09-04

- `DAM-P0-04D` latest mid-review P0 0 / P1 4 / P2 4를 correction-required로 재개하고 P1 4건을 bounded DataManagement Presentation correction으로 교정했다. 사용자 상태에서 InventoryGeneration을 제거하고, Stable ID 4상태 의미를 분리하며, No Known Referencer의 삭제 안전 비보장 안내와 sort/list regeneration 뒤 Slate selection/Detail 재결합을 추가했다.
- corrected official Editor build `9faa491f7cc144ba9aad50dd18de069b` PASS, P0-04D 1/1 + P0-04C 1/1 + P0-04B 1/1 + P0-03 3/3 = corrected focused/affected **6/6 PASS**를 확보했다.
- final re-review는 **P0 0 / P1 0 / P2 4**다. P2 4건은 non-blocking UX polish debt로 유지하고 exact next는 `DAM-P0-04E USER Re-Acceptance`다.
- DataManagement `SavePackage` / `MarkPackageDirty` / `Modify()` 0과 Product Asset 신규 mutation/save 0을 재확인했다. CF-FQ-046/047 existing dirty를 보존했으며 commit/push는 수행하지 않았다.

### v0.1.15 - 2026-09-03

- USER 승인 + fresh preflight로 기존 CF-FQ-047 implementation HOLD를 bounded하게 재판정했다. CF-FQ-046은 HOLD, CF-FQ-047은 DataAuthoring/Product Wagon recovery 영역, CF-FQ-045는 DataManagement-only 수정으로 분리 가능함을 확인해 P0-04B~D만 진행했으며 CF-FQ-046/047 source·Product Asset은 건드리지 않았다.
- `DAM-P0-04B`를 Technical PASS로 닫았다. Type/Asset을 실제 `SHeaderRow + SMultiColumnTableRow` 관리표로 교체하고, Managed/NeedsManagementRule/Framework/AuxiliaryDataAsset presentation state, view-specific filter, `전체 발견 보기`, UE 5.8 header-click sort와 canonical ClassPath/ObjectPath tie-breaker를 구현했다.
- `DAM-P0-04C`를 Technical PASS로 닫았다. current registered concrete 27종에 `UserPurposeDescription/UserUsageDescription` 27/27을 제공하고, management-universe Overview 4종, user-first Detail, 접힌 `기술 정보`, Asset-only action visibility를 구현했다.
- `DAM-P0-04D` 최종 검수에서 최초 구현에 빠진 manual column sort와 `전체 발견 보기` 전환 시 ManagementState combo stale option을 발견해 교정했다. final source review는 P0/P1 0이며 구형 문자열 row/Type-Asset filter coupling 0, `SavePackage`/`MarkPackageDirty`/`Modify` 0이다.
- final official Editor build `28852efeaf3c4b61860502eb8da6edc3` PASS. 최종 P0-04B 1/1, P0-04C 1/1, P0-03 3/3과 직전 동일 semantic/backend의 P0-02B 2/2 + P0-02C 3/3 + P0-02A 2/2 + P0-01 3/3을 모두 PASS로 유지한다. Product Asset mutation/save 0이다.
- exact next는 `DAM-P0-04E USER Re-Acceptance`이며 USER PASS 전 Feature Done/Current System Promotion은 금지한다.

### v0.1.14 - 2026-09-03

- `DAM-P0-04A` read-only Source 설계 감사를 수행해 current Manager DTO/Registry가 table/presentation correction을 수용할 수 있음을 확인했다.
- 설계 P1 2건을 교정했다. presentation state는 `bCanonicalNative + bAbstract + Coverage` exact rule로 고정하고 Overview는 management universe와 Asset/Type 단위를 명시했다.
- 사용자 설명 metadata는 Registry global required가 아니라 current 27 descriptor coverage로 검증하도록 해 synthetic/future compatibility를 보존했다. deterministic sort tie-breaker, Purpose ellipsis/tooltip과 상태 텍스트 우선 원칙도 고정했다.
- P0-04B/C/D/E implementation/verification/USER re-acceptance test matrix를 추가하고 설계 재검수 P0/P1 0 PASS로 닫았다. CF-FQ-047 완료 전 implementation HOLD는 유지한다.

### v0.1.13 - 2026-09-03

- representative USER Acceptance에서 Manager 화면의 정보 구조와 시선 흐름이 불명확하다는 USER FAIL을 기록했다. 기능/Backend 결함이 아니라 table/presentation information architecture 문제로 분류했다.
- `DAM-P0-04A Information Architecture Lock`을 Design PASS로 승격했다. 실제 Multi-column Type/Asset Table, 사용자 의미 Overview/Detail, Managed/NeedsManagementRule/Framework/Auxiliary presentation state, view-specific filter와 사용자용 semantic 설명 계약을 고정했다.
- core discovery/registry/health/reference/identity 계약과 Runtime/Product Asset은 유지하며 Editor C++ presentation correction만 허용한다. BP/Asset mutation 0이다.
- `CF-FQ-047` 완료 전 `DAM-P0-04B` source implementation은 HOLD한다. exact next after 047 completion은 `DAM-P0-04B Table Surface`다.

### v0.1.12 - 2026-09-03

- `DAM-P0-04` technical acceptance를 PASS로 준비했다. 기존 final linked binary의 DAM Automation 13/13, current 28/53 integration baseline, Domain/Scope/abstract/zero-instance/Unregistered/Gathering/StableId/duplicate/reference/cache-reset/no-mutation evidence를 acceptance checklist와 대조했다.
- fresh Git status 재확인 결과 Acceptance 검증으로 Product Asset dirty가 새로 추가되지 않았고 metadata Refresh/no-save/no-mutation 경계가 유지된다.
- representative USER Acceptance checklist 9단계를 고정했다. 핵심 판정은 기술 클래스명을 외우지 않고도 원하는 DA를 찾고 역할/상태를 이해할 수 있는지다.
- exact next는 representative USER Acceptance이며 USER PASS 전 Feature Done/Current promotion은 금지한다.

### v0.1.11 - 2026-09-03

- `DAM-P0-03 Verification Closure`를 완료해 Manager UI + On-demand Detail Gate를 Technical PASS로 승격했다.
- parallel CF-FQ-046 compile blocker는 Step 3 Socket 진단 Slot 닫힘 bracket 1개와 `ITableRow/STableViewBase` forward declaration 누락을 최소 교정해 제거했다. Builder logic/state/mutation authority는 변경하지 않았다.
- final official Editor build `de5aeba4c2b54f2dbd27e2778d07cfb6` PASS, corrected P0-03 3/3 + P0-02C 3/3 + P0-02B 2/2 + P0-02A 2/2 + P0-01 3/3 = 13/13 PASS를 확보했다.
- final source re-review P0/P1 0이며 metadata-only Refresh, explicit Validate/Reference, no-mutation/no-save 경계가 유지된다.
- exact next Gate를 `DAM-P0-04 Acceptance`로 전진했다.

### v0.1.10 - 2026-09-03

- `DAM-P0-03` mid-review P1 1 + P2 2를 bounded correction으로 교정하고 source re-review P0/P1 0을 확보했다.
- `PolicyUnavailable` validation을 non-conclusive로 분리하고 filter/view/validation-driven hidden selection을 VM + Slate 양쪽에서 정리했다.
- correction re-review에서 UE 5.8 Asset Registry bool semantics를 대조해 query failure가 `No Known Referencer`로 오판될 수 있는 신규 P1을 발견·교정했다. successful-empty evidence일 때만 NoKnownReferencer를 허용한다.
- final correction source는 UBT `-SingleFile` 4/4 compile PASS지만 full official Editor build는 protected parallel CF-FQ-046 `CFVehicleBuilderTab.cpp:1352` compile error로 세 차례 차단됐다. CF-FQ-045는 해당 병렬 파일을 수정하지 않았다.
- corrected runtime Automation은 아직 재실행하지 않았으므로 P0-03 Technical PASS 승격을 보류한다. next exact action은 `DAM-P0-03 Verification Closure`다.

### v0.1.9 - 2026-09-03

- `DAM-P0-03 Manager UI + On-demand Detail` 구현을 완료해 Data Overview, Type/Asset View, search/filter, Detail, explicit Validate/Reference, Asset Open/Content Browser Sync와 `CarFight.DataAssetManager` Nomad Tab을 추가했다.
- `CFDAManagementVM`을 session-local InventoryGeneration/cache owner로 두고 `FCFDAReferenceService`를 on-demand package Reference/Referencer owner로 분리했다. metadata Refresh는 load/reference 0 fast lane을 유지한다.
- final official Editor build `f85846158f2e4ff5b3fd9a8a0abdbbf7` PASS, P0-03 focused 3/3 + affected P0-02C 3/3 + P0-02B 2/2 + P0-02A 2/2 + P0-01 3/3 PASS다.
- 중간검수에서 P0 0 / P1 1 / P2 2를 발견했다. `PolicyUnavailable` Validate를 성공으로 오표시하는 P1과 hidden selection/view-switch selection P2 두 건을 P0-03 correction으로 남긴다.
- 따라서 P0-03은 아직 Technical PASS로 승격하지 않고 exact next를 `DAM-P0-03 Correction + Re-review`로 둔다.

### v0.1.8 - 2026-09-03

- DAM-P0-02C 중간검수 P1 4건을 bounded correction으로 교정하고 재검수 P0/P1 0으로 닫았다. loaded service API를 Inventory snapshot-bound로 바꿔 generation provenance를 보장한다.
- duplicate 상태를 NotAnalyzed/N-A/Unique/Duplicate로 분리하고 single Validate의 complete namespace closure, typed FName/FGuid canonical equality와 incomplete coverage fail-closed를 구현했다.
- operational evaluation failure를 `ECFDAEvaluationState`로 분리해 LoadFailed/ClassMismatch/AdapterUnavailable/InvalidRequest가 DA Health Error로 오인되지 않게 했다. duplicate namespace owner는 `ResolveDuplicateNamespacePolicy()`로 통합했다.
- correction official Editor build `7651c0fe87da4ee481f6ff9995ec793c` PASS, corrected P0-02C 3/3 + affected P0-02B 2/2 + P0-02A 2/2 + P0-01 3/3 PASS를 확보했다.
- `DAM-P0-02 Typed Semantic + Health Adapter` Technical PASS와 next exact Gate `DAM-P0-03 Manager UI + On-demand Detail`은 유지한다.

### v0.1.7 - 2026-09-03

- `DAM-P0-02C Loaded Health Lane`을 Technical PASS로 닫아 정식 `DAM-P0-02 Typed Semantic + Health Adapter` Gate 전체를 완료했다.
- 기본 Refresh는 metadata-only를 유지하고 explicit Validate/Detail에서만 `CFDAHealthService`가 대상 Asset을 lazy load한다. `CFDATypeAdapter`는 Required/Optional/N/A와 FName/Guid/PrimaryAssetId identity, NativeDataValidation/CustomContract를 read-only로 해석한다.
- InventoryGeneration/freshness 계약, Unregistered non-authoritative guard, ExactClassPath/PrimaryAssetType namespace-local duplicate를 구현했다. generation/cache global state는 추가하지 않았다.
- 코드 재검수에서 VehicleFitting의 `IsDataValid()`가 `BuildFittingSnapshot()` Error/Warning까지 포함하는 사실을 반영해 Native adapter를 교정했다.
- final official Editor build `e4826d2a72b34465aefe96b9396dcfa6` PASS, P0-02C focused 3/3 + affected P0-02B 2/2 + P0-02A 2/2 + P0-01 3/3 PASS다.
- Reference/Referencer, Manager UI/Nomad Tab, Asset mutation/save는 열지 않았다. next exact Gate는 `DAM-P0-03 Manager UI + On-demand Detail`이다.

### v0.1.6 - 2026-09-03

- DAM-P0-02B mid-review P1 4건을 교정하고 재검수 PASS로 닫았다. future DA Unregistered visibility와 current 27종 exact Registered baseline을 분리해 신규 DA 추가가 올바른 확장 동작인데 테스트가 실패하던 문제를 제거했다.
- Identity/Validation SourceName exact comparison + source symbol compile guard를 추가하고 current batch registration을 all-or-nothing으로 바꿔 conflict 실패 시 caller Registry partial mutation을 제거했다.
- duplicate namespace derivation을 FName/GUID=ExactClassPath, PrimaryAssetId=PrimaryAssetType, N/A=NotApplicable로 고정하고 Unregistered semantic policy는 non-authoritative라는 guard를 추가했다. Stable ID resolve/duplicate analysis 자체는 P0-02C에 남겼다.
- correction final official Editor build `4e880dbb47de4bca95e96d3308aa2b56` PASS, corrected focused 2/2 + affected P0-02A 2/2 + P0-01 3/3 PASS다. 재검수 P0/P1 0이며 next는 `DAM-P0-02C Loaded Health Lane` 유지다.

### v0.1.5 - 2026-09-03

- `DAM-P0-02B Current Type Semantics`를 Technical PASS로 닫고 current concrete CarFight DA 27종 전체를 actual Source audit 기준으로 `CFDATypeRegistry`에 등록했다.
- Domain/사용자용 Type 이름/역할 설명과 타입별 Identity/Validation policy를 고정했다. Identity는 `Required/Optional/N/A`와 `FName/GUID/PrimaryAssetId/None` resolver kind를 분리했으며 current concrete coverage는 27/27 Registered, abstract `UCFInventoryItemData`는 Unregistered framework type으로 유지한다.
- VehicleData의 `GetPrimaryAssetId()` identity와 공개 read-only `UCFVDAValidator::ValidateVehicleData` validation facade를 반영했고, Authoring Profile의 `OwnerRecipeId`·Evidence/Builder private preflight·CombatFx configured helper는 generic identity/whole-asset validator로 잘못 승격하지 않았다.
- final official Editor build `832ae2e554fc4f4ab5f0b58af0f1c6f4` PASS, focused 2/2 + affected P0-02A 2/2 + affected P0-01 3/3 PASS를 확보했다.
- final source boundary audit에서 Asset load, Stable ID resolve/write, Validation execution, stable-ID Duplicate analysis, Reference/Referencer, Manager UI/Nomad Tab 구현 0을 재확인했다. metadata-only Refresh는 유지한다.
- next exact internal checkpoint를 `DAM-P0-02C Loaded Health Lane`으로 전진했다.

### v0.1.4 - 2026-09-03

- DAM-P0-02A Registry Foundation을 Technical PASS로 닫았다. `ECFDADomain`, `FCFDASemanticDescriptor`, `FCFDAResolvedSemantic`, `FCFDATypeRegistry`와 Registry→Coverage bridge를 구현했다.
- exact class-path registration/lookup, identical idempotency, conflicting descriptor fail-closed, deterministic enumeration, `Unregistered + Unclassified + technical display` fallback을 고정했다.
- empty Registry current integration에서도 P0-01의 native/persisted inventory가 숨지 않고 현재 canonical native 타입이 Unregistered로 유지됨을 검증했다. 현재 concrete 타입의 실제 semantic mapping은 DAM-P0-02B로 남겼다.
- first build는 새 소스 compile PASS 후 실행 중 Editor DLL 점유로만 실패했고, server-guarded AI-owned/no-user-work stop 후 final official Editor build PASS를 확보했다. focused 2/2 + affected P0-01 3/3 PASS다.
- Stable ID resolve, Validation, Duplicate ID, Reference/Referencer, Manager UI/Nomad Tab은 구현하지 않았고 metadata-only Refresh를 유지했다.
- next exact internal checkpoint를 `DAM-P0-02B Current Type Semantics`로 전진했다.

### v0.1.3 - 2026-09-03

- DAM-P0-01 중간점검 후 P0-02/P0-03 실행 경계를 교정했다. DAM-P0-01 Technical PASS 자체는 유지한다.
- DAM-P0-02를 하나의 정식 Gate로 유지하면서 `02A Registry Foundation → 02B Current Type Semantics → 02C Loaded Health Lane` 내부 checkpoint로 분할했다.
- Validation result를 `InventoryGeneration`에 종속시키고 Refresh가 새 generation을 만들면 이전 loaded validation 결과를 무효화하도록 freshness 계약을 명시했다. 서비스 계층의 static global cache는 금지하고 session cache owner는 DAM-P0-03 `CFDAManagementVM`으로 지정했다.
- P0-04에서 검증하던 lazy Reference query의 구현 owner 누락을 교정해 DAM-P0-03 `Manager UI + On-demand Detail`에 Reference/Referencer query와 conservative state를 명시했다.
- next exact Gate는 `DAM-P0-02 Typed Semantic + Health Adapter`, 첫 내부 checkpoint는 `DAM-P0-02A Registry Foundation`으로 유지한다.

### v0.1.2 - 2026-09-03

- DAM-P0-01 Inventory Core를 Technical PASS로 닫았다. native CarFight DA class와 persisted DataAsset metadata를 자동 발견하고 class/asset merge, deterministic dedupe, abstract/concrete, zero-instance visibility를 구현했다.
- metadata-only Type/Asset DTO, path-boundary Scope + Unclassified, descriptor 존재 여부 seam, CoverageState / TypeInstanceState, Asset Registry Ready/Gathering/Failed 상태를 추가했다.
- persisted discovery를 project-local `/Game` unknown candidate와 CarFight native/derived class identity까지 포함하도록 교정해 future DA가 semantic descriptor 없이도 Unregistered candidate로 보이게 했다.
- `Refresh`의 전체 `GetAsset()`, Stable ID resolve, Validation, Duplicate ID, Reference/Referencer, Manager UI, Nomad Tab 구현은 0으로 유지했다.
- official Editor Build `0ad7210fafd34904a7fea35428425859` PASS, focused Automation `197471da83424c8ab5158a565e9c6acd` 3/3 PASS로 닫았다.
- next Gate를 `DAM-P0-02 Typed Semantic + Health Adapter`로 전진했다.

### v0.1.1 - 2026-09-03

- DAM-P0-00 설계감사 P1/P2 교정을 반영했다. Discovery allowlist 금지와 Typed semantic mapping 허용을 분리하고 native CarFight class universe / persisted unknown candidate 경계를 명시했다.
- 기본 `Refresh`를 metadata-only fast lane으로 고정하고 Stable ID/Validation/Reference는 lazy loaded lane으로 분리했다. Registry Gathering, validation cache reset과 StableId NotResolved 상태를 추가했다.
- Coverage / TypeInstance / Health / Scope를 분리하고 `Unclassified` Scope, concrete/abstract 표시와 27 concrete + 1 abstract baseline 해석을 추가했다.
- Domain을 사용자 Overview의 큰 영역과 일치시키고 `Unregistered`를 Domain이 아닌 CoverageState로 교정했다. P0 상세 화면의 Feature/System 사용 영역 하드코딩을 제거했다.
- DAM-P0-01을 class/asset discovery + metadata inventory + Coverage까지만 닫도록 축소하고, current concrete 타입 전체 semantic descriptor는 DAM-P0-02 완료 조건으로 옮겼다.
- Automation edge case는 Product fake DA class 생성 대신 synthetic DTO seam으로 검증하도록 고정했다.
- 교정 후 current Source/Editor module 경계와 재대조해 `Design Audit Correction + Re-review PASS`로 확정했다. next Gate `DAM-P0-01 Inventory Core`는 변경하지 않았다.

### v0.1.0 - 2026-09-03

- `CF-FQ-045 CarFight Data Asset Management`를 P2 / Ready 정식 Plan으로 승격했다.
- DAM-P0-00 read-only audit의 DA class 28종 / persisted DA Asset 53개를 baseline으로 기록했다.
- 기존 DA를 리팩터링하지 않는 Editor-only read-first management layer를 확정했다.
- Generic Adapter + Typed Adapter + Coverage Auditor와 신규 DA auto-discovery / Unregistered-visible 계약을 고정했다.
- 첫 화면을 전체 Asset 목록이 아닌 Data Overview로 두고 Type View / Asset View / 사람용 설명 중심 UX를 확정했다.
- next Gate를 `DAM-P0-01 Inventory Core`로 설정했다.

---

## 22. Migration

- v0.2.0부터 Data Asset Manager의 현재 구현 판단은 `Document/Systems/DataManagement/DataAssetManagement.md v1.0.0`과 실제 `CarFight_ReEditor/DataManagement` Source를 우선한다. 이 Plan은 완료 당시 설계·Build·Automation·USER feedback을 보존하는 Historical + Retained Path이며 Runtime/Content/Product Asset migration은 없다.
- v0.1.19는 Editor-only presentation/freshness-state 표현 교정이며 Runtime/Content/Product Asset migration은 없다. Refresh 뒤 과거 loaded 결과값은 여전히 폐기하고 session-local 재검사 필요 표시만 보존하므로 validation authority나 저장 데이터 형식은 바뀌지 않는다.
- v0.1.18은 v0.1.17 한글 우선 Presentation correction의 linked binary verification closure이며 Runtime/Content/Product Asset migration은 없다. 사용자 표시 변경만 새 binary에서 검증됐고 데이터/identity/reference authority는 그대로다.
- v0.1.17은 Editor-only 사용자 문구/Presentation 교정이며 Runtime/Content/Product Asset migration은 없다. 내부 enum/class/path/identity/reference authority는 변경하지 않고 사용자-facing label/message만 한글 우선으로 바꾼다. `/Game`, `/Script`, Object/Class Path 등 실제 식별값은 그대로 유지한다.
- v0.1.16은 Editor-only Data Management Presentation correction이며 Runtime/Content/Product Asset migration은 없다. InventoryGeneration freshness authority와 Stable ID/Reference core semantics는 유지하고 사용자 표시와 Slate selection reconciliation만 교정했다. P2 4건은 USER Re-Acceptance를 차단하지 않는 polish debt로 남긴다.
- v0.1.15는 Editor-only Manager presentation/semantic metadata correction이며 Runtime/Content/Product Asset migration은 없다. 정렬/filter/presentation state는 session-local이고 저장하지 않으며, current 27 사용자 설명은 Editor Registry metadata일 뿐 Runtime DA class를 변경하지 않는다. 기존 metadata-only Refresh와 explicit Validate/Reference 경계는 유지한다.
- v0.1.14는 P0-04A 설계 감사/교정과 test matrix 추가뿐이며 Source/Product Asset migration은 없다. presentation state/Overview universe/semantic coverage 규칙이 구현 전 고정됐고 CF-FQ-047 완료 전 HOLD를 유지한다.
- v0.1.13은 P0-04 USER Acceptance FAIL과 Manager IA/Table UX correction 설계 승격뿐이며 Source/Product Asset migration은 없다. P0-04B 구현은 CF-FQ-047 완료 전 HOLD다.
- v0.1.12는 Acceptance evidence/checklist 갱신뿐이며 Runtime/Content/Product Asset migration은 없다. USER acceptance 전 lifecycle promotion은 없다.
- v0.1.11은 Editor-only Manager verification closure와 shared Builder compile-only repair이며 Runtime/Content/Product Asset migration은 없다. Data Manager consumer 계약은 P0-03 Technical PASS 상태로 고정됐고, Builder compile repair는 Slate 구조/forward declaration만 복구해 mutation authority를 바꾸지 않는다.
- v0.1.10은 Editor-only Manager correction이며 Runtime/Content/Product Asset migration은 없다. Validate consumer는 non-Succeeded evaluation을 완료 상태로 표시하지 않고, Reference consumer는 `QueryFailed`와 successful-empty `NoKnownReferencer`를 구분한다. selection은 current filter/view context 밖으로 나가면 자동 해제된다. full verification은 parallel CF-FQ-046 compile blocker 해소 후 이어진다.
- v0.1.9는 Editor-only Data Asset Manager UI/VM/Reference query 추가이며 Runtime/Content/Product Asset migration은 없다. Window > CarFight에 `CarFight 데이터 관리` 진입이 추가되고 Manager tab lifetime의 transient cache만 생성된다. P1/P2 mid-review correction 전이므로 P0-03 current consumer 계약은 아직 provisional이다.
- v0.1.8은 Editor-only loaded result/API correction이며 Runtime/Content/Product Asset migration은 없다. Health consumer는 old `AssetRecord + Generation` API 대신 current `FCFDAInventoryResult + ObjectPath`를 전달해야 하며, duplicate UI는 bool 대신 `ECFDADuplicateState`, operational 상태는 `ECFDAEvaluationState`를 사용한다. Product Asset mutation/save는 없다.
- v0.1.7은 Editor-only loaded read lane 추가이며 Runtime/Content/Product Asset migration은 없다. 기존 metadata-only `Refresh()` caller는 기존처럼 generation 0 결과를 받을 수 있고, DAM-P0-03 session owner는 `NextInventoryGeneration()`으로 발급한 non-zero generation을 generation-aware Refresh/loaded result에 전달한다. loaded validation 결과는 Product Asset에 write-back하거나 저장하지 않는다.
- v0.1.6은 Editor-only Registry/test contract correction이며 Runtime/Content/Asset migration은 없다. future DA는 descriptor 없이 Unregistered로 보일 수 있고, duplicate namespace policy는 loaded identity result를 비교할 기준만 선언하며 실제 resolve/duplicate execution은 P0-02C가 소유한다.
- v0.1.5는 Editor-only current semantic/policy descriptor mapping 추가이며 Runtime/Content/Asset migration은 없다. Identity/Validation source 이름은 선언 metadata이고 resolver/validator 실행은 DAM-P0-02C가 소유한다. 기존 metadata-only Refresh caller는 동작이 바뀌지 않는다.
- v0.1.4는 Editor-only Registry foundation 추가이며 Runtime/Content/Asset migration은 없다. 기존 P0-01 discovery 호출자는 계속 `FCFDARefreshOptions` 경로를 사용할 수 있고, Typed Registry consumer는 새 `Refresh(TypeRegistry)` overload를 선택할 수 있다. 실제 current type semantic 등록은 DAM-P0-02B에서 추가한다.
- v0.1.3은 구현 전 설계 경계 교정이며 Runtime/Content/Asset migration은 없다. P0-02 구현은 02A→02B→02C 순서로 진행하고, Reference/Referencer는 P0-03 on-demand Detail owner로 둔다. Validation cache는 InventoryGeneration에 종속되며 Refresh 뒤 이전 generation 결과를 재사용하지 않는다.
- v0.1.2는 Editor-only metadata inventory foundation 추가이며 Runtime/Content/Asset migration은 없다. 기존 DA class와 persisted Asset은 수정하지 않으며 Stable ID/Validation/Reference/UI는 후속 Gate가 소유한다.
- v0.1.1은 설계 계약 정밀화이며 기존 Runtime/Content/Asset migration은 없다. 기존 28 DA class와 53 persisted Asset을 수정하지 않고 Manager 내부 discovery/semantic/validation 책임만 분리한다.
- v0.1.0은 신규 Editor management layer 설계 문서이며 기존 Runtime/Content migration은 없다.
- 기존 DA class와 persisted Asset은 수정하지 않는다.
- 기존 `.uasset`이 SSOT이며 별도 data mirror를 만들지 않는다.
- 향후 새 DA type은 Generic discovery로 먼저 보이고 semantic 관리가 필요할 때 Typed Adapter를 추가한다.
