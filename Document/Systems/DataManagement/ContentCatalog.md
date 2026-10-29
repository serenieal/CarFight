# Content Catalog & Authoring — 콘텐츠 카탈로그·제작 시스템

- 문서 버전: v1.1.0
- 최근 갱신일: 2026-10-29
- 문서 상태: Current System / CF-FQ-058 CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS / COMPLETE (2026-10-07)
- 범위: CarFight Production 무기·장비 콘텐츠의 식별·저작·검토·생성·검증·발행과 Runtime 소비
- 근거: 실제 CarFight_ReEditor/CarFight_Re Source, CF-FQ-058 대표 Plan v0.1.37, canonical UDS 종료 기록과 당시 Build·Automation·USER 수락 증거

---

## 1. 현재 실제 역할

이 시스템은 단일 무기 에셋을 수작업으로 수정하는 기능이 아니라, **Product(출시용 장비)와 이를 구성하는 DataAsset(데이터 에셋)의 저작 의도를 관리하고, 검증한 내용만 실제 저장 에셋으로 발행하는 제작 경로**다.

기존 Data Asset Manager(에셋 관리 UI)의 읽기·검사 책임과 JSON Staging(임시 저작 파일)의 타입별 Apply 책임을 대체하지 않는다. CCAS는 명시적으로 관리 대상으로 전환된 콘텐츠의 저작 권한과 Production 발행 경로를 소유한다.

현재 Production First Wave는 다음 범위에서 완료됐다.

| 구분 | 확인된 수량 | 현재 의미 |
| --- | ---: | --- |
| 발행된 Product | 8 | Production Publication Catalog에 실린 장비 |
| 생성·영속 검증된 고유 Production DataAsset | 34 | Runtime이 소비하는 Product/child 에셋 |
| 관리 대상 manifest | 42 | canonical Workbook 저작 권한 적용 범위 |
| 발행 후 RuntimeApply 회귀 | 18/18 PASS | 당시 Equipment 소비 경로 검증 |

**주의:** 위 수량은 CF-FQ-058 완료 시점의 승인된 관리 범위다. 모든 게임 에셋이 CCAS 관리 대상이라는 의미가 아니며, 이후 차량 대량 제작 기능(CF-FQ-059)이 완료됐다는 뜻도 아니다.

## 2. 소스와 저장 결과의 권한 분리

```text
CCAS-managed exact42
Authoring/Content/CarFight_Content.xlsx
    └─ 저작 값의 단일 권한 (sole authoring authority)
          ↓
Content Core / Provider / Compiler
    └─ 타입별 해석, 참조 검사, 차이·드리프트 Preview
          ↓
승인된 Review / 변경 적용 / 영속 검증
          ↓
Production .uasset exact34
    └─ 게임 Runtime의 실제 DataAsset
          ↓
DA_ProdEquipCatalog
    └─ Product exact8 발행 목록과 장비 승인 여부
          ↓
RuntimeApply Equipment discovery / authorization
    └─ 기존 Fitting Snapshot / Prepare / Commit / Recovery 재사용

Authoring/Content/CarFight_Content.cfsnapshot.json
    └─ Workbook과 생성 결과의 provenance(생성 출처)·drift(차이) 검증 증거
```

- **Managed:** 승인된 exact42 범위는 Workbook이 저작 원본이고, 생성된 `.uasset`은 Runtime/Product용 결과물이다.
- **Unmanaged / ExternalReadOnly / Legacy:** 해당 범위의 저장된 `.uasset`이 계속 현재 콘텐츠 기준이다. 기존 JSON Staging/Reviewed Apply 규칙도 유지한다.
- **Data Asset Manager:** 발견·분류·감사·참조 조회를 담당하는 read-first(읽기 우선) UI이며 Workbook 저작 권한이나 자동 Save 권한을 갖지 않는다.

## 3. 현재 구현 요소와 책임

| 실제 구현 주체 | 책임 |
| --- | --- |
| `CFContentCore` | 안정적 Content identity, 셀 값/타입/스키마, 의존 관계·순환 검증, 의미 해시 |
| `CFContentProvider` / Provider 계약 | DataAsset 종류별 읽기·검증·diff·실제 적용 경계 |
| `CFContentCompiler` | 현재 저장 상태와 Workbook 의도를 대조하는 read-only Preview, drift 및 참조 영향 검사 |
| `CFContentPlanning` | Product별 작업 계획·Review 입력과 단계별 작업 판단 |
| `CFContentCutover` | 승인된 관리 범위의 저작 권한 전환, 내구적 결과·복구 상태·출처 관리 |
| `UCFProdEquipCatalogData` | 발행된 장비 목록, ContentId와 저장된 발행 정보의 Runtime 조회·검증 |
| `FCFRuntimeEquipApplyService` | 실제 차량 장비 후보 검증·적용·실패 복구. CCAS가 이를 중복 구현하지 않음 |

대표 Source 위치:

```text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFContentCore.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFContentProvider.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFContentCompiler.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFContentPlanning.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFContentCutover.cpp
UE/Source/CarFight_Re/Private/CFProdEquipCatalogData.cpp
UE/Source/CarFight_Re/Private/CFRuntimeEquipApply.cpp
```

위 파일은 역할별 대표 진입점이다. 실제 WorkBook/Provisioning의 구체적인 호출 순서와 세부 저장 소유자는 해당 구현과 `DataAssetAuthoring.md`를 함께 확인한다.

## 4. 완료된 First Production Wave

발행된 장비 Product는 다음 8개다.

1. `Cannon_Standard`
2. `Cannon_Heavy`
3. `Cannon_LongRange`
4. `Cannon_Rapid`
5. `Rocket_Standard`
6. `Rocket_Salvo`
7. `Rocket_Ripple`
8. `GuidedMissile_Standard`

완료된 초기 구성은 EquipmentPreset 8, Mount 2, WeaponDefinition 8, Projectile 6, Damage 5, Ammo 5로 고유 DA 34개다. Product별 역할 연결은 총 48개였으며, 같은 DA를 여러 Product가 참조하는 공유 구성도 포함한다.

`HeavyCannon`과 `RocketLauncher` 등 기존 Prototype은 신규 Production Product identity가 아닌 Test/Legacy 참조다. 별도 Product 승인 없이 기존 Prototype을 Production 목록과 동일하게 취급하지 않는다.

현재 Consumer는 차량 후보를 기존 `RuntimeTestCatalog`에서 찾고, 장비 후보·승인은 **Production Publication Catalog 우선**으로 해석한다. Production Catalog가 없거나 유효하지 않을 때만 기존 Legacy Equipment Catalog를 fallback으로 사용한다. 차량의 Fitting/GrossMass 제약과 적용 실패 시 mutation0 계약은 계속 유효하다.

## 5. 완료 검증과 경계

- `CF-FQ-058`: 2026-10-07 CCAS-P0-08 USER FINAL WORKFLOW ACCEPTANCE PASS / COMPLETE.
- 승인된 Workbook/생성 에셋·Publication Catalog·provenance 정합성: 당시 fresh 검증 PASS.
- 후속 공식 UE 5.8 Build `97030b7aba1348d4b1688c34afc19954` PASS.
- RuntimeApply 전체 `12eb4517709249cd9d5a8b0e5a4d25a6` 18/18 PASS.
- ProductionCutoverVerification `d719964509e14441b7160972c56694bd` 1/1 PASS.
- 현재 Wagon에 일부 Production 무기를 바로 장착하지 못하는 **GrossMass 제약**은 별도 게임 데이터·장착 문제다. CCAS의 기술 완료 증거가 차량별 장착 성공을 의미하지 않는다.
- 2026-10-29 **새로운 읽기 전용 Production AssetDump 35/35 PASS**를 실행했다. `/Game/CarFight/Weapons/Data/Production`의 generated Product/child DataAsset exact34와 `DA_ProdEquipCatalog` exact1을 독립 목록으로 읽었다. Catalog의 `PublishedEquipment`는 exact8이고 각 ContentId는 `Cannon_Standard`, `Cannon_Heavy`, `Cannon_LongRange`, `Cannon_Rapid`, `Rocket_Standard`, `Rocket_Salvo`, `Rocket_Ripple`, `GuidedMissile_Standard`와 일치한다. `EquipmentPresetData` 참조 및 ProductGraphFingerprint 필드가 8개 entry에 있음을 확인했다.
- 새 덤프의 정확한 증거 ID는 `adset_v1_9cc61ceb42f3616ffedb129e716d6f2b.e35cc266ba4124d57bfbe064`다. 이 결과는 **현재 저장 패키지의 추출/조회 PASS**이며 Workbook 원본↔전체 SHA 재계산·Cook/PIE 장착 성공·모든 dependency 유효성의 재검증이 아니다. 기존 2026-10-07 Build·Automation의 완료 증거와 분리한다.

## 6. 관련 문서 및 비책임

- `Document/Systems/DataManagement/DataAssetAuthoring.md`: 기존 JSON Staging, 타입별 provider와 CCAS 관리 범위의 상세 Apply 계약
- `Document/Systems/DataManagement/DataAssetManagement.md`: 탐색·관리 UI와 read-first 검사 책임
- `Document/Systems/DataManagement/BuilderAuthoringStandard.md`: Builder/Authoring Guide 공통 사용자 워크플로
- `Document/Systems/Vehicles/RuntimeApply.md`: 실제 게임 내 장비 교체·질량 검증·복구
- `Document/Plan/ContentAuthoringSystem/ContentAuthoringSystemPlan.md`: 완료 과정과 상세 Historical evidence
- `Document/Plan/VehicleContentAuthoring/VehicleContentAuthoringPlan.md`: CF-FQ-059 차량 CCAS 온보딩. 별도의 진행 중 Work이며 본 완료 판정에 포함하지 않음

본 문서는 CCAS의 **전체 현재 구조를 찾는 진입 문서**다. 타입별 내부 상세를 반복 복사하거나 차량 제작의 미완료 단계를 완료로 승격하지 않는다.

## 7. Changelog

### v1.1.0 - 2026-10-29
- Production 전체 35개 패키지(생성 DA 34 + Publication Catalog 1) Fresh AssetDump를 실행하고 Catalog PublishedEquipment exact8과 Product ID·Preset 참조·fingerprint를 재확인했다.
- Dump readback PASS와 과거 CCAS Feature 완료/USER acceptance 및 새 PIE 미실행을 구분해 Current evidence를 최신화했다.
- Source·Workbook·에셋을 변경하거나 다른 CCAS Consumer의 현장 장착 성공을 새로 주장하지 않았다.

### v1.0.0 - 2026-10-29
- 종료된 `CF-FQ-058`의 Content Core → Provider/Compiler → 관리 권한 Cutover → Production Catalog → RuntimeApply 소비 경계를 Current Systems 진입 문서로 정리했다.
- First Wave exact8, generated exact34, managed exact42, 기존 Legacy/Unmanaged 권한과 사용 범위를 분리했다.
- 기존 Source·에셋·UDS·Plan 수정 및 검증 재실행 없이 문서만 추가했다.

### Migration
- 기존 파일/에셋 경로를 이동하거나 이름을 바꾸지 않는다. 본 문서는 새 기술 소유자가 아니라 기존 구현의 Current navigation을 보강한다.
