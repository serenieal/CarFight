# Data Asset Management

- Version: 1.0.0
- Date: 2026-09-04
- Status: Current Implementation / CF-FQ-045 Done
- Scope: CarFight Editor 전용 DataAsset 탐색, 의미 분류, 검사, 참조 조회와 관리 화면
- 완료 기반: `DAM-P0-04E USER Acceptance PASS` + official UE 5.8 Editor build PASS + affected Automation 6/6 PASS

---

## 1. 문서 목적

이 문서는 CarFight의 **현재 Data Asset Manager 구현 계약**을 기록한다.

사용자는 DataAsset 클래스명과 저장 경로를 모두 외우지 않고도 에디터의 `CarFight 데이터 관리` 화면에서 현재 데이터 유형과 실제 에셋을 찾고, 용도와 상태를 확인하고, 필요한 경우 명시적으로 검사하거나 참조 관계를 조회할 수 있다.

완료 당시 단계별 설계·Build·Automation·USER 피드백은 Historical Plan인 `Document/Plan/DataAssetManagement/DataAssetManagementPlan.md`가 보존한다. 현재 구현 판단은 이 문서와 실제 `CarFight_ReEditor/DataManagement` Source를 우선한다.

---

## 2. 현재 책임 경계

Data Asset Management는 `CarFight_ReEditor`가 소유하는 **Editor-only read-first 관리층**이다.

```text
Unreal .uasset
= 데이터의 Single Source of Truth

Data Asset Manager
= 발견 / 분류 / 설명 / 검사 / 참조 조회 / 탐색 UI
```

현재 Manager는 다음을 소유하지 않는다.

- Runtime gameplay 계산
- DataAsset 생성·삭제·이름 변경·이동
- Stable ID 자동 수정
- bulk edit
- Runtime Catalog 자동 등록
- 자동 Save / Save All
- Product Asset 자동 변경

Runtime DataAsset 클래스와 Blueprint에 Manager 편의를 위한 새 책임을 추가하지 않는다.

---

## 3. 현재 구조

```text
Native CarFight UDataAsset class discovery
+ Unreal Asset Registry persisted metadata
        │
        ├─ FCFDATypeRegistry
        │    └─ 사용자 의미 / identity / validation policy
        │
        ├─ FCFDAAuditService
        │    └─ metadata-only inventory
        │
        ├─ FCFDATypeAdapter
        │    └─ explicit loaded validation / identity
        │
        ├─ FCFDAReferenceService
        │    └─ explicit package reference / referencer query
        │
        └─ FCFDAManagementVM
             └─ SCFDAManagementTab
```

주요 Source:

```text
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDAManagementTypes.h
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDATypeRegistry.h
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDATypeAdapter.h
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDAAuditService.h
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDAManagementVM.h
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDAManagementTab.h
UE/Source/CarFight_ReEditor/Public/DataManagement/CFDAReferenceService.h
UE/Source/CarFight_ReEditor/Private/DataManagement/
```

---

## 4. Discovery와 Typed Registry 계약

Discovery와 의미 등록은 서로 다른 책임이다.

```text
Discovery
= 실제 존재하는 CarFight DataAsset 타입/에셋을 빠짐없이 찾음

Typed Registry
= 발견된 타입의 사용자용 의미와 타입별 검사/ID 정책을 설명함
```

현재 canonical native universe는 CarFight Runtime/Editor module이 소유하는 `UDataAsset` descendant다. 저장된 에셋이 0개인 타입과 abstract framework 타입도 Type inventory에서 사라지지 않는다.

신규 DataAsset 타입은 Registry descriptor가 없어도 discovery 결과에 `관리 규칙 필요(Unregistered)`로 나타난다. 따라서 **새 타입을 발견하기 위해 Manager UI나 discovery allowlist를 수정할 필요가 없다.** 의미 기반 관리가 필요할 때만 타입 단위 semantic descriptor를 추가한다.

현재 concrete CarFight DataAsset 타입에는 사용자용 이름, `어떤 데이터인가`, `어디에 사용되는가` 설명과 타입별 identity/validation policy가 등록돼 있다.

---

## 5. Refresh와 Loaded Lane

기본 `새로고침`은 metadata-only fast lane이다.

```text
새로고침
→ native class discovery
→ FAssetData inventory
→ class/asset merge + deterministic dedupe
→ abstract/concrete / Scope / Coverage 계산
→ 관리표 갱신
```

기본 Refresh에서는 전체 `FAssetData::GetAsset()` 로드를 수행하지 않는다. Stable ID resolve, 실제 validation, duplicate 분석과 Reference/Referencer expansion도 자동 실행하지 않는다.

Asset load가 필요한 작업은 사용자가 에셋을 선택한 뒤 명시적으로 실행한다.

```text
[검사]
→ 선택 에셋만 load
→ Stable ID resolve
→ 타입별 validation
→ duplicate 분석

[참조 관계 조회]
→ Asset Registry package dependency / referencer evidence 조회
```

---

## 6. 상태 모델

Manager는 서로 다른 의미의 상태를 하나의 성공/실패 값으로 합치지 않는다.

- `Coverage`: 해당 타입에 CarFight 관리 의미 규칙이 등록됐는가
- `Scope`: Runtime / Authoring / Test / Debug / Legacy / 미분류 중 어디에 속하는가
- `Health`: 실제 검사를 통해 확인된 정상 / 경고 / 오류 / 검사 필요 상태
- `Stable ID`: 확인됨 / 해당 없음 / 필수 ID 누락 / 미확인
- `Evaluation`: 검사 자체를 정상 수행했는가, load/adapter 요청 문제가 있었는가
- `Duplicate`: 중복 분석 전 / 해당 없음 / 중복 없음 / 중복 있음
- `Referencer`: 사용처 있음 / 알려진 사용처 없음 / 조회 실패 / 조회 전

`검사 규칙 없음`이나 load 실패 같은 운영 상태를 DataAsset 자체의 Health 오류로 오인하지 않는다.

---

## 7. Refresh 이후 재검사 계약

검사 결과는 해당 `InventoryGeneration`에 묶인다.

새로고침으로 inventory generation이 바뀌면 과거 loaded 검사 결과를 현재 결과처럼 재사용하지 않는다. 대신 같은 Manager 세션에서 이전에 검사했던 ObjectPath는 다음처럼 표시한다.

```text
Health     → 재검사 필요
Stable ID  → 재확인 필요
```

이는 과거 검사값을 보존한다는 뜻이 아니라 **현재 snapshot 기준으로 다시 확인해야 한다는 사실만 보존**하는 presentation 상태다.

사용자가 다시 `검사`하면 current generation 결과를 새로 만들고 재검사 필요 표시는 해제된다.

---

## 8. 현재 사용자 화면

Manager는 Overview → 관리표 → 선택 상세의 흐름을 사용한다.

### Type View

- 데이터 유형
- 영역
- 용도
- 에셋 수
- 관리 상태
- `직접 에셋 생성: 가능 / 불가`를 포함한 필요 기술 정보

### Asset View

- 에셋 이름
- 데이터 유형
- 영역
- 사용 범위
- 상태
- 고유 ID
- 검사 / 참조 관계 조회 / 에셋 열기 / 콘텐츠 브라우저에서 찾기

사용자-facing label, 상태와 안내는 한글을 우선한다. `/Game/...`, `/Script/...`, 실제 Object/Class Path, C++ 클래스처럼 검색·디버깅에 필요한 식별값은 원문을 유지하고 한글 의미를 병기할 수 있다.

Type View와 Asset View의 검색·필터 상태는 서로의 전용 filter를 오염시키지 않는다. 표 정렬은 canonical ClassPath/ObjectPath tie-breaker를 사용해 deterministic하게 유지한다.

---

## 9. 참조 관계 안전 계약

`참조 관계 조회`는 Unreal Asset Registry가 현재 알고 있는 package dependency/referencer evidence를 보여준다.

`알려진 사용처 없음`은 **현재 조회에서 알려진 referencer가 없었다는 뜻**이며 다음을 의미하지 않는다.

- 삭제해도 안전함
- 런타임에서 절대 사용하지 않음
- soft/dynamic/config 기반 사용이 없음

따라서 Data Asset Manager의 Reference 결과를 삭제 안전 판정으로 확대하지 않는다.

---

## 10. Mutation / Save 안전 계약

CF-FQ-045 P0의 Current Manager는 read-first 관리 도구다.

DataManagement Source에서 다음 mutation/save 호출을 사용하지 않는다.

```text
SavePackage
MarkPackageDirty
Modify()
```

검사, 새로고침, 참조 조회, Asset Editor 열기와 Content Browser 동기화는 Product DataAsset의 값을 자동 변경하거나 저장하지 않는다.

---

## 11. C++ / Blueprint 소유권

### C++ Editor Module 소유

- discovery / metadata inventory
- semantic Registry
- identity / validation adapter
- duplicate analysis
- reference/referencer query
- Manager ViewModel
- Slate Type/Asset 관리표와 사용자 Presentation

### Blueprint / Product Asset

CF-FQ-045 구현을 위해 신규 Blueprint 책임을 만들지 않았다. 기존 Product `.uasset`은 Manager의 SSOT 입력이며 Manager가 자동 수정하거나 저장하지 않는다.

---

## 12. 검증된 Current 범위

최종 기술 검증:

- Official UE 5.8 Editor Build `51326d9704ac4a51b2febf6332d5e959`: PASS / ExitCode 0
- DAM-P0-04D: 1/1 PASS
- DAM-P0-04C: 1/1 PASS
- DAM-P0-04B: 1/1 PASS
- DAM-P0-03: 3/3 PASS
- affected 합계: **6/6 PASS**
- lower discovery/registry/typed-health baseline PASS는 대표 Historical Plan에 보존
- Product Asset mutation/save: 0

최종 USER Acceptance:

- 2026-09-04 최신 한글 우선 화면
- `직접 에셋 생성: 가능/불가`
- Refresh 뒤 `재검사 필요 / 재확인 필요`
- 재검사 뒤 current 결과 복원
- 전체 정보 구조 및 이해 가능성

위 항목을 사용자가 직접 확인하고 **DAM-P0-04E USER Acceptance PASS**를 승인했다.

---

## 13. 비차단 후속 폴리싱

CF-FQ-045 완료를 막지 않는 P2 UI polish 2건은 향후 필요 시 별도 lifecycle로 다룬다.

1. 오른쪽 Detail을 하나의 긴 TextBlock이 아니라 의미별 section widget으로 더 구조화
2. Type View / Asset View 중 현재 활성 View를 더 명확한 시각 상태로 표시

현재 기능 계약이나 USER PASS를 이유 없이 다시 열지 않는다.

---

## 14. Changelog

### v1.0.0 - 2026-09-04

- `CF-FQ-045 CarFight Data Asset Management`의 P0 완료 내용을 Current System으로 최초 승격했다.
- native/persisted auto-discovery, Typed Semantic Registry, metadata-only Refresh, explicit Validate/Reference, 상태 분리, generation-bound 재검사 필요 표시와 한글 우선 Manager UX를 Current 계약으로 고정했다.
- official UE 5.8 Editor build와 affected Automation 6/6 PASS, 최종 DAM-P0-04E USER Acceptance PASS를 완료 기반으로 기록했다.
- Product Asset 자동 mutation/save가 없는 read-first 경계를 Current 계약으로 고정했다.

---

## 15. Migration

- CF-FQ-045 완료 이후 Data Asset Manager의 현재 구현 판단은 이 문서와 실제 `CarFight_ReEditor/DataManagement` Source를 우선한다.
- `Document/Plan/DataAssetManagement/DataAssetManagementPlan.md`는 완료 당시 설계·검증·USER 피드백 evidence를 보존하는 Historical + Retained Path로 읽는다.
- 기존 Runtime/Content/Product Asset의 데이터 형식이나 저장 방식에는 migration이 없다.
