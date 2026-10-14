# CF-FQ-060 Vehicle Builder Socket Prep UX / 차량 메시 소켓 준비 UX Plan

- 문서 버전: v0.1.13
- 최근 갱신일: 2026-10-15
- 문서 상태: Ready / VSPUX-P0-04 USER Coupe 소켓 저장·재로드 PASS + managed focused Automation 6/6 PASS / Socket Manager live-list refresh Deferred / Step 3 복귀 직접 UX 확인 미보고
- **현재 계약: §4.9 + §4.7 + §4.8; Hardpoint Preview 구현 §4.14, 기술 검증 §4.15; Socket Manager 표시 Deferred 결정 §4.16; Coupe 저장·재로드 USER PASS §4.17(Current).** 이전 §4.13과 §4.5/§4.6은 Historical 참고만.
- Feature: `CF-FQ-060 Vehicle Builder Socket Prep UX`
- Work: `wrk_60d4b7f9c3584ab9b5e21ad04a875e3f`
- Project / Authority: `CarFight` / `project_id=carfight` / `repository_role=main_game`
- Branch / Engine: `SwitchSourceEngine` / Unreal Engine 5.8 Source Build
- 구현 기준: `Document/Systems/Vehicles/VehicleBuilder.md v1.9.1` 및 현재 Source 우선
- 현재 범위: **VSPUX-P0-04 USER Coupe 소켓 준비 및 재로드 후 전체 부착 정상 확인 PASS, 신규 Hardpoint Base_Standard 프리뷰 적용 + Automation 6/6 PASS**. Socket Manager 실시간 행 갱신은 Deferred/non-blocking으로 분리; 기존 Step 3 복귀의 별도 USER 실제 UI 확인만 미보고.

---

## 1. 목적과 문제

차량용 StaticMesh를 준비할 때 바퀴·Hardpoint Socket 이름을 사용자가 외우거나 문서에서 찾지 않고, **StaticMesh 하나만으로** 정확한 이름을 확인·생성·수동 배치할 수 있게 한다.

기존 Vehicle Builder Step 3에는 Wheel Socket 이름 표시·복사·생성, live 존재 검사, StaticMesh Editor 열기 기능이 이미 있다. 그러나 현행 Step 3 진입은 Recipe 기반 차량 제작 흐름에 묶여 있어 메시 준비만 하려는 사용자는 불필요한 단계들을 진행해야 한다.

**성공 기준**: Recipe, VehicleData, 차량 ID, Wheel Mesh, Content authoring 없이 StaticMesh 선택 → 필요한 Wheel/Hardpoint Socket 확인·생성 → 기존 StaticMesh Editor에서 Transform 수동 조정 → 사용자가 명시적으로 Save. 기존 차량 제작 Step 1~8 및 Recipe 기반 Step 3는 불변.

## 2. 확정한 선택: A안 (같은 Vehicle Builder에 독립 Quick Socket Prep 진입)

```text
Vehicle Builder
 ├─ 기존 Step 1~8 차량 제작 → Step 3 Socket Guide (Recipe 기반 / 현행 계약 유지)
 └─ 소켓 미리 준비 (Quick Socket Prep / Recipe 불필요)
      → StaticMesh 직접 선택
      → 선택 StaticMesh의 모든 기존 소켓 확인·복사·Editor 열기
      → 현행 Wheel 4역할 + 지원 Standard HP_* 누락 생성
      → 기타·미래 소켓 목록 보존 및 Editor 수동 편집 경로
      → StaticMesh Editor 열기
      → USER Transform 편집 / USER 수동 Save
```

- **A안 채택**: 기존 Builder 안에서 Socket Guide의 UI·생성 함수·이름 규칙 재사용.
- **B안(독립 Socket Tool/Tab) 비채택**: 중복 UI, 상태 모델, 검증·유지보수를 늘리기 때문.
- Quick 모드는 기존 Step 3의 `Locked/Unavailable/Complete` 평가를 풀어주는 구현이 아니다. **차량 생성 상태와 독립된 화면 모드**여야 한다.
- 후보 목록에서 Mesh-only Candidate 선택 시 그 메시를 초기 대상으로 제안하고, 동시에 **직접 StaticMesh Picker**로 임의 메시를 선택할 수 있어야 한다. 기존 후보 검색은 Mesh Collection Root/이미 사용 중인 Mesh 제외 조건이 있어 후보 목록만으로 충분하지 않다.
- Quick 모드 복귀 시 기존 선택 차량·단계·Recipe state를 파괴하지 않는다.

## 3. Fresh Source Rebaseline — 2026-10-13, Read-only

| Source / 현재 동작 | 판단 |
| --- | --- |
| `SCFVehicleBuilderTab::GetSocketPreparationVisibility`는 Recipe, Step 3, Mesh-only Candidate 제한 | 기존 조건은 유지; Quick 모드에서 필요한 소켓 UI만 재사용 |
| `FCFVehicleBuilderVM::RebuildStepStates`는 Recipe 없는 Mesh 후보의 Step 3를 Locked로 종료 | 전역 Step 상태 모델 완화 금지 |
| `GetCurrentChassisMeshPath`는 Mesh-only Candidate에 대해서는 경로를 반환 | 초기 후보에 재사용; 별도 Picker는 Quick 전용 대상 저장 |
| `GetRequiredWheelSocketNames`는 Recipe 없으면 FL/FR/RL/RR 기본 이름 반환 | Quick에서는 기본 네 개를 항상 사용; managed Recipe custom binding 불변 |
| `BuildRequiredWheelSocketRow`는 이름 표시·복사·live 존재 검사·생성 UI 보유 | UI 재사용 우선 |
| `AddChassisSocketAtOriginAndOpen`는 기존 이름 중복 방지·기본 Transform 생성·Editor 열기 수행 | 핵심 소켓 생성 경로 재사용; 자동 Transform 수정/Save 금지 |
| `IsCurrentChassisSocketPresentLive`는 StaticMesh의 실제 `FindSocket` 사용 | Quick 선택 메시 기준 live 조회 |
| `GetSocketPreparationSummaryText`는 Recipe Resolver snapshot 의존 | Quick에서는 live 소켓 상태로 표시; false missing 금지 |
| `BuildHardpointPlanningPanel` 및 VM `AddStandardHardpoint`는 Recipe intent 쓰기 의존 | Quick에서는 **StaticMesh 소켓만**; Recipe 등록 호출 금지 |
| `ReadCurrentChassisSocketInventory`는 Recipe 의존 | Quick은 선택 UStaticMesh의 실제 Socket 목록 사용 |
| `TryParseCanonicalStandardSocketName`은 표준 `HP_<Category>_<NN>` 이름 검증 제공 | 기존 parser와 category contract 재사용 |
| `FCFVehicleAuthoringService`의 후보 검색은 모든 StaticMesh를 노출하지 않음 | 직접 Picker가 필수 |

관련 Source:
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTab.cpp`
- `UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleBuilderTab.h`
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp`
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderHardpointIntegrity.cpp`
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAuthoringService.cpp`
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderPresent.cpp`

기존 검증 위치:
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderTests.cpp`
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAuthoringVMTests.cpp`
- `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderHardpointIntegrityTests.cpp`

## 4. 보존해야 할 기능 계약

### 4.1 Wheel 소켓

Quick 모드 기본 이름 exact4:

```text
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR
```

각 행에 정확한 이름/복사/존재 또는 없음/누락 소켓 생성 기능을 제공한다. 이미 있는 소켓은 이름·Transform을 변경하지 않는다. 기존 Recipe 기반 Step 3에 사용자 정의 Wheel Binding이 있으면 **그 binding을 계속 존중**해야 하며 Quick의 기본 네 개로 덮어쓰지 않는다.

### 4.2 Hardpoint 소켓

현재 지원하는 Hardpoint **위치 분류**는 `Top`, `Front`, `Back`, `LeftSide`, `RightSide`, `Bottom`, `Internal`이다. 사용자 요구의 '위치'는 좌표가 아니라 **분류(Category)**를 뜻하며 이 7개는 영구 고정값이 아니다. canonical 소켓 이름은 `HP_<Category>_<NN>` (예: `HP_Top_01`, `HP_Top_02`)이며 신규 분류 추가 시에도 같은 규칙을 따른다. 최신 요구·확장 계약은 §4.7을 따른다.

Quick에서 사용자가 **현재 지원하는 표준 category를 동적 선택**하면 선택 StaticMesh의 **실제 소켓 목록**을 확인해 해당 범주 최대 기존 번호 다음 번호를 제안하고, 명시 클릭마다 한 개씩 생성한다. 중복 금지, category 간 독립 번호. 위 7종은 현재 지원 종류이지 영구 고정 상한이 아니다. 기타/미인식/미래 용도 소켓도 이름을 확인하고 수동 편집 경로를 유지하며, 정식 게임 연동은 해당 naming/consumer 계약 확정 뒤에만 허용한다. **현재 구현 필수 범위·레이아웃 소유권은 §4.9, category 확장 계약은 §4.7**을 따른다.

**물리 소켓 생성 ≠ Recipe Hardpoint/LocationSlotId/MountProfile 작성**. Quick 모드에서 `FCFVehicleBuilderVM::AddStandardHardpoint`를 호출하거나 Recipe, VehicleData, HardpointPlanMode, Catalog, Fitting Data를 수정하지 않는다. 나중에 기존 Builder에서 소켓 채택/장착 슬롯 구성은 현재 방식대로 진행한다.

### 4.3 편집 및 보호

- 새 소켓의 Transform은 기본값으로 생성한다. 실제 위치/회전/스케일은 USER가 StaticMesh Editor에서 조정한다.
- **자동 Save, Save All, 메시 복제/교체, 임의 Transform 수정 없음.**
- 여러 차량이 참조하는 공유 StaticMesh를 편집하면 사용 차량 모두에 영향이 갈 수 있음을 화면에 알린다.
- 선택한 메시가 누락/로드 불가/편집 불가능한 경우의 생성 동작을 안전하게 처리한다.
- Quick 전환·종료로 기존 차량 선택, Step state, Recipe, dirty asset에 영향이 없어야 한다.
- live 메시의 실제 상태가 기준이며 Recipe Resolver가 없다는 이유로 `소켓 없음`이라고 표시하지 않는다.

### 4.4 범위 제외

- 독립 Socket Tool/별도 Editor Tab/일반 목적 Socket Framework
- `CF-FQ-059` Vehicle CCAS Domain Onboarding, `CF-FQ-058` CCAS, Excel/Workbook, 콘텐츠 대량생산
- GoPyMCP, AssetDump, 독립 도구 저장소 수정
- Recipe/VehicleData/장비 슬롯 자동 생성, Wheel Mesh 생성·편집
- StaticMesh Transform 자동 배치, 자동 저장
- 기존 Step 1~8 state machine 완화/재설계


## 4.5 VSPUX-P0-01 구현 전 계약 검수 — 2026-10-13 당시 PASS (Historical)

> 이 절은 당시 검수 원문을 보존하며 **현재 구현 계약을 소유하지 않는다**. 미결정으로 남긴 모드 종료/Quick UI host/생성 처리 등은 최신 **§4.9를 우선** 적용한다. §4.7 category 의미와 §4.8 layout 분리도 함께 적용한다.

**결론:** A안은 기존 Builder 계약을 변경하지 않는 Tab-local Quick 모드로 구현 가능하다. 핵심은 `선택된 소켓 편집 대상`과 `Recipe 기반 단계 상태`를 분리하는 것이며, 새 BuilderVM/제작툴/별도 Editor Tab은 불필요하다.

### UI 소유권과 화면 교체

1. `SCFVehicleBuilderTab`에만 `bQuickSocketPreparationActive` (일시적인 Quick 모드 여부)와 `QuickSocketChassisMeshPath` (편집 대상 임시 StaticMesh 경로)를 둔다. Recipe/VehicleData 또는 BuilderVM의 current selection/state를 이 목적으로 수정하지 않는다.
2. Browser 왼쪽 기존 `+ 새 차량 만들기`와 별도의 `소켓 미리 준비` 버튼으로 진입한다. 초기 경로는 기존 ViewModel의 현재 선택 ChassisMeshPath가 있으면 복사하며, 선택이 없어도 빈 Picker로 진입할 수 있다. Quick 전용 `SObjectPropertyEntryBox`는 현재 Step 2와 동일한 `AllowedClass(UStaticMesh::StaticClass())`, `FAssetData::GetSoftObjectPath()` 계약을 재사용한다.
3. Quick 활성 중 상단 Target/Step/State/Summary/Next 설명은 **Quick 전용 문구**로 표시하고, 기존 8개 Step의 페이지, 비관련 Recipe/Physics/Gameplay/Driving 액션, 이전/다음 진행 버튼을 실제로 숨기거나 비활성화한다. 선택한 Step을 `SocketGuide`로 강제 전환하거나 Locked 상태를 Complete로 위조하지 않는다.
4. 기존 Step 3 Socket Guide의 Wheel 행/소켓 생성/Editor 열기 UI는 공통 화면 내에서 재사용한다. `HardpointPlanningHost`는 Quick에서 물리 Socket 추가 전용 하위 UI를 만들고, 일반 Step 3에서는 현행 Recipe Hardpoint/Adoption/Mount 준비 UI를 그대로 빌드한다. Recipe 기반 선택 FX·Layout 진단은 Quick에서 숨기거나 명확히 별도로 표시한다.
5. Quick 종료/차량 선택/신규 제작 진입 시 Quick 표시 모드는 종료하거나 명시적으로 유지 여부를 결정하지만, 어느 경우에도 기존 VM의 Recipe, Selection, current Step, Creation Draft는 **진입·복귀 때문에 변경하지 않는다**. Quick Picker만 바뀌어도 기존 Step 2 pending picker를 덮어쓰지 않는다. Common Page Scroll은 Quick 진입·종료 시 상단으로 되돌린다.

### 한 가지 편집 대상과 한 가지 Wheel 이름의 원칙

1. `GetSocketEditingChassisMeshPath()`: Quick에서 `QuickSocketChassisMeshPath`, 일반 Step 3에서 `ViewModel->GetCurrentChassisMeshPath()`를 반환하는 Tab-local 경로 하나로 Socket open/add/live check를 통일한다. Step 2 pending Mesh open은 **기존 방식 그대로** 유지한다.
2. `GetDisplayedRequiredWheelSocketNames()`: Quick은 `Wheel_Anchor_FL / FR / RL / RR` 기본 exact4, 일반 Step 3은 `ViewModel->GetRequiredWheelSocketNames()` custom binding 결과를 반환한다. Wheel Name 표시, Status, Copy, 추가/Enable, Summary는 반드시 이 **같은 결정 함수**를 사용한다.
3. **확인된 기존 직접 영향 오류**: 현재 `BuildRequiredWheelSocketRow()`는 Slate Construct 시 `SocketName`을 값으로 캡처해 버튼에 고정하지만, Text/Status/Copy는 현재 VM Recipe의 동적 이름을 읽는다. 선택 차량의 custom Wheel Binding이 달라지면 버튼이 **표시된 것과 다른 SocketName을 생성할 수 있다**. 이름은 버튼 클릭 시 WheelRoleIndex로 fresh 재조회하도록 교정한다. 이 교정은 Quick과 직접 영향 받는 기존 Step 3 전체에 적용하고, 비관련 경로는 건드리지 않는다.
4. Quick Summary의 Socket Found는 **실제 UStaticMesh::FindSocket()**를 사용하고 Resolver `IsCurrentChassisSocketFound` / `ReadCurrentLayoutFacts`를 사용하지 않는다. 존재 상태 확인 불가능과 실제 누락을 명확히 구별한다. 일반 Step 3의 기존 Layout 기반 Summary는 보존한다.

### Hardpoint 번호 정책과 실제 소켓 작성

1. `FCFVehicleBuilderVM::GetStandardHardpointCategories()`와 `FCFVehicleBuilderHardpointIntegrity::TryParseCanonicalStandardSocketName()`을 재사용한다. Quick 소켓 이름은 현재 선택 `UStaticMesh::Sockets`에서 같은 범주의 **canonical 번호 최대값 + 1**로 제안한다.
2. 제안 번호가 기존 SocketName과 충돌하면 `FindSocket` 기준으로 추가 번호를 탐색한다. noncanonical `HP_*`, 다른 범주, Wheel/FX Socket은 임의 이름 변경·삭제·이동하지 않는다. `%02d`는 **최소 두 자리 형식**이지 99 제한을 뜻하지 않는다. 100 이상도 기존 parser 문법에 맞으면 인식한다. 정수 범위 초과·대상 Mesh 없음·유효하지 않은 Category는 소켓 생성 없이 오류로 처리한다.
3. 번호 추천/중복 처리의 순수 로직은 **기존 `CFVehicleBuilderHardpointIntegrity`에 한 개의 additive helper**로 작성해 transient `UStaticMesh` fixture로 테스트한다. 새 데이터 스토어·별도 서비스·VM 계약 변경은 없다.
4. 최초 UX는 Category 단추를 누를 때 **소켓 1개씩 명시적으로 추가**, 반복 클릭으로 원하는 개수를 준비한다. 한 번에 다수 소켓을 자동 생성하거나 새로운 개수 편집 상태를 만들지 않는다. 각 클릭마다 actual live Socket Inventory 재조회.
5. Quick은 `FCFVehicleBuilderVM::AddStandardHardpoint`, `CommitHardpointPlanMode`, `HandleAdoptExistingHardpointSocket` 등을 호출하지 않는다. 이 기능들은 Recipe/Target에 semantic intent를 기록하므로 물리 소켓 준비와 분리해야 한다. 기존 Step 3은 사용자가 명시적으로 기존 물리 소켓을 채택하는 현행 계약을 보존한다.

### 소켓 생성·저장·오류 경계

1. 기존 `AddChassisSocketAtOriginAndOpen` transaction, duplicate protection, default Transform, `MarkPackageDirty/PostEditChange`, `OpenStaticMeshAssetPath`를 재사용한다. **실제 `Save`는 호출하지 않는다.**
2. Quick에서 물리 Socket 1개 추가 후 live UI를 갱신하되, Recipe가 없거나 무관한 VM 선택을 이유로 `RefreshCurrentState()`를 무조건 호출하지 않는다. 기존 Step 3에서는 현재 VM refresh 계약을 유지한다.
3. 현재 `CanAddCurrentChassisSocket()`은 경로가 invalid해도 `FindSocket == false`만으로 버튼을 활성화할 수 있다. Quick에서는 유효한 편집 가능한 `UStaticMesh`가 실제로 확인될 때만 생성 버튼/편집 버튼을 활성화한다. 없거나 로드 불가라면 사용자에게 `먼저 StaticMesh를 선택` 또는 `StaticMesh를 열 수 없음`을 표시하고 중복/생성 작업을 하지 않는다.
4. 처음부터 존재하는 Socket은 기존 Transform/이름을 유지하고 Edit만 허용한다. 동일 Mesh 공유 경고, User-controlled Save, 기존 dirty 보존은 전체 경로 공통 규칙이다.

### 실제 수정 단위 (P0-02)

| 파일 | 최소 변경 |
| --- | --- |
| `Public/DataAuthoring/CFVehicleBuilderTab.h` | Quick 모드의 Tab-local 상태, 기존 Socket 접근을 일원화하는 helper/handler 선언 |
| `Private/DataAuthoring/CFVehicleBuilderTab.cpp` | 좌측 진입 버튼, Quick Picker/표시·복귀, Step UI 차단, 현행 Socket Guide UI 재사용, Wheel 동적 이름 클릭 버그 교정, live 편집 경로 분기 |
| `Public/DataAuthoring/CFVehicleBuilderHardpointIntegrity.h` | 순수 다음 canonical HP SocketName 제안 helper 선언 |
| `Private/DataAuthoring/CFVehicleBuilderHardpointIntegrity.cpp` | 실제 StaticMesh Socket 목록/기존 parser/충돌 검사 활용 |
| `Private/DataAuthoring/CFVehicleBuilderHardpointIntegrityTests.cpp` | HP 번호·결손·collision·no Recipe transient 테스트 추가 |
| 기존 `CFVehicleBuilderTests.cpp` / `CFVehicleAuthoringVMTests.cpp` | 직접 영향 회귀만 필요한 범위에서 추가·실행; VM/Product 계약 변경은 예상하지 않음 |

위 파일 범위는 구현 전 설계 확정 범위다. 실제 호출자·테스트를 읽는 도중 추가 직접 의존성이 확인되면 semantic-unit 범위에서만 반영한다. 구현에서 변경한 모든 함수/변수 위 한국어 역할 주석, Source 버전/Changelog/Migration은 기존 프로젝트 규칙을 따른다.

### 검증 시 정확한 완료 판정

- **STATIC/PRE-IMPLEMENTATION PASS**: 선택 대상·이름·상태·UI 소유권·Mutation 경계를 실제 소스와 대조해 구현 안을 확정함.
- **아직 NOT RUN**: Editor Build, Automation, Slate 화면, StaticMesh Editor 실제 조작, USER Save/Acceptance.
- 구현 후 focused Automation에서 Helper `max+1`, 비정형/중복/invalid, Wheel custom binding 선택 전환, 버튼이 표시 이름과 같은 exact 이름을 생성하는지, Quick의 Recipe mutation0, 기존 8-step 회귀를 확인한다.
- 빠른 독립 UX는 UE Editor 내 사용자 수락이 필요하다. USER가 실제 StaticMesh를 배치·저장하기 전 최종 UX PASS는 기록하지 않는다.

## 4.6 VSPUX-P0-01 Design Re-review Correction — 2026-10-14 당시 해석 (Historical; §4.7/§4.9로 정정)

> 아래 §4.6.1~4.6.4는 당시 검수 설명이며 **구현 필수 항목·UI 배치를 결정하지 않는다**. 좌표/Transform 수치 패널·필터/검색 등은 사용자의 현재 요구가 아니다. 기존 소켓 보존 및 Quick 복귀/새로고침의 목적은 유지하되, 정확한 구현 범위·두 UI host·전환/검증은 **§4.9**, 위치 분류의 의미는 **§4.7**, 레이아웃 분리는 **§4.8**을 따른다.

### 4.6.1 확장 가능한 소켓 모델 — USER 추가 요구 반영

**물리적 위치와 논리적 용도를 혼동하지 않는다.** 선택 StaticMesh 안의 모든 `UStaticMeshSocket`은 `SocketName`(고유 식별자), `RelativeLocation`, `RelativeRotation`, `RelativeScale`을 갖는 Mesh-owned 물리 Socket이다. `Top`, `Front` 등은 **이름/장착 의도에 사용하는 위치 분류**일 뿐 실제 메시 상 좌표를 제한하지 않는다. 같은 `Top`에도 `HP_Top_01`, `HP_Top_02`, `HP_Top_03`처럼 여러 개의 서로 다른 위치를 둘 수 있다.

- Quick의 **주 화면은 실제 선택된 StaticMesh의 소켓 전체 인벤토리**다. Wheel/Standard HP뿐 아니라 FX/사용자 정의/미인식/미래 분류 소켓도 이름을 그대로 표시한다. 현재 미인식은 `게임용 미연결/이름 규칙 미확인`으로 구분하며 잘못된 이름이라고 단정하거나 자동 삭제·변경하지 않는다.
- 소켓 행에는 exact `SocketName`·복사·기존 StaticMesh Editor 열기·가능한 실제 `RelativeLocation/Rotation/Scale`을 read-only 정보로 제공한다. 엔진에서 특정 소켓 자동 선택/포커스가 검증되지 않았다면 `편집`은 **메시 Editor 열기**까지만 약속한다.
- 소켓 수가 늘어도 탐색하기 쉽도록 `전체 / 바퀴 / 표준 Hardpoint / 기타` 필터와 개수를 제공한다. 새로운 용도는 `기타`에서 사라지지 않으며, 고정 7개 버튼을 전부 펼쳐 놓는 화면을 강제하지 않는다. 초기 구현에서 텍스트 검색은 실제 목록 규모와 사용성 검수 후 필요하면 추가한다. 이 작업 때문에 새 영구 분류 DB, 플러그인 또는 범용 Socket Framework를 도입하지 않는다.
- 표준 Hardpoint 신규 생성은 **기존 `GetStandardHardpointCategories()`에서 현재 지원하는 category를 가져오는 동적 종류 선택 UI** 하나로 제공한다. 2026-10-14 기준 표준 7개(`Top`, `Front`, `Back`, `LeftSide`, `RightSide`, `Bottom`, `Internal`)는 **현재 지원 값**이지 영구 상한이 아니다. 표시명은 현재 7개 한국어 별칭 + 새 category의 원문 이름 fallback으로 처리한다.
- 현재 표준 category가 추가·개정되면 UI 하드코딩 목록만 늘리는 방식이 아니라 `StandardHardpointCategories`를 통한 기존 VM/Parser/Recipe validator/Consumers의 **동일 semantic contract**를 함께 검수한다. 새로운 category string을 한 곳에 넣었다는 이유로 Runtime 장착 지원이 자동 완성된다고 주장하지 않는다.
- 현행 Wheel role exact4는 **기존 차량의 4-role binding 계약**이다. 메시에서 추가 Wheel-like 소켓을 발견해도 인벤토리에서 숨기지 않지만, 6륜 등 Wheel role/runtime 확장은 별도 Feature가 소유한다.
- 새 종류의 `정식 게임 연동`은 별도 domain contract 변경이며 이 Feature는 **물리 소켓 준비 UX**만 담당한다. 물리 Socket 존재를 Recipe Hardpoint/FX/무장/Runtime binding 완료로 오인시키지 않는다.

### 4.6.2 제작 동선 — 신규/기존/기타 모두 지원

```text
Vehicle Builder > 소켓 미리 준비
  └─ StaticMesh Picker (선택 없이도 Quick 진입 가능)
     ├─ 현재 Mesh 소켓 전체 목록 [전체/바퀴/표준 HP/기타]
     │    ├─ 기존 소켓: 이름·현재 Transform read-only 표시/복사/메시 Editor 열기
     │    └─ 미인식·미래 소켓: 이름/Transform 그대로 보존, 게임 자동 연결 표시 금지
     ├─ 바퀴 소켓 준비: FL/FR/RL/RR exact4 + 누락분 1개씩 생성
     ├─ 표준 Hardpoint 준비: 현재 지원 category 선택 → 다음 canonical HP 이름 제안
     │    └─ 명시 클릭마다 Socket 1개 생성 / 같은 category 다수 배치 가능
     └─ 기타 새 위치: 기존 StaticMesh Editor에서 자유 이름·Transform 작성
          (추후 표준화된 용도는 해당 Feature의 naming/consumer 계약과 통합)
```

새로운 용도에 대한 임의 이름/타입을 **게임에 자동 등록하는 일반-purpose 입력 기능은 이번 범위에 넣지 않는다**. 지금도 실제 Socket Editor에서 사용자 정의 이름을 만들고 Quick 전체 목록에서 다시 확인할 수 있다. 자동으로 이름을 제안할 수 있는 것은 **현행 정식 이름 계약에 포함된 역할과 category**뿐이며, 사용자에게 알 수 없는 내부 이름을 임의 발명해 넣게 하지 않는다. 향후 새 표준 용도가 생기면 해당 consumer naming contract를 먼저 확정하고 Quick 동적 종류 선택/표시에 연결한다.

### 4.6.3 앞선 재검수 P1 교정 확정

1. **P1-01 기존 Hardpoint 편집:** Quick에서는 Recipe가 없는 `BuildHardpointPlanningPanel()` 기존 오류 경로를 노출하지 않는다. 대신 actual Mesh의 모든 기존 HP/기타 소켓을 읽는 Quick 전용 인벤토리 섹션을 사용한다. 기존 소켓은 `추가`가 아니라 `열어 편집`으로 안내하고 이름/Transform 자동 수정은 하지 않는다. 기존 Step 3 Recipe 기반 `HardpointPlanningHost`는 유지한다.
2. **P1-02 진입·복귀:** `소켓 미리 준비` 클릭은 기존 VM current selection/Step/creation draft를 **변경하지 않고** Quick mode와 임시 Mesh path만 지정한다. `돌아가기`는 Quick state만 종료하여 이전 Builder 화면을 그대로 표시한다. Quick 중 일반 Browser 차량 선택 또는 `+ 새 차량 만들기` 클릭 시 Quick을 종료한 뒤 해당 기존 action을 **원래대로** 수행한다(사용자가 명시 클릭한 action에 따른 VM selection 변경은 허용). Quick 중 차량 목록 새로고침은 Quick mode와 Picker mesh path를 유지한다. Picker 값 변경만으로 기존 Step 2 pending selection/Recipe를 바꾸지 않는다.
3. **P1-03 Quick 새로고침:** Quick 상태의 `현재 상태 다시 확인`은 선택 StaticMesh의 live `Sockets`/Transform을 readback하여 Quick 목록 및 카테고리별 다음 이름을 갱신한다. `ViewModel->RefreshCurrentState()`, Recipe Resolver, Hardpoint intent writer를 호출하지 않는다. 일반 차량 제작 화면의 Refresh 경로는 변경하지 않는다. Editor에서 소켓 이동 후 이 단추로 확인하며 자동 Save는 하지 않는다.
4. **기존 Wheel 버튼 직접 영향 P1 유지:** Wheel 이름 Text/Copy/Status/Enable/Click 모두 동일 current role→name resolver를 사용한다. Constructor에서 캡처한 과거 SocketName은 사용하지 않는다. 두 차량 custom binding이 서로 다른 경우에도 **보이는 이름만** 실제 Mesh에 추가한다.
5. **오류/중복 경계:** `Mesh가 선택되지 않음`, `Mesh를 읽을 수 없음`, `선택한 이름이 이미 존재함`, `신규 추가 가능`을 분리해서 표시한다. 분류와 이름만으로 좌표나 Mount binding을 추론하지 않는다. 새 소켓 생성은 live re-check 뒤 기존 Transaction, 기본 Transform, dirty 표시, Editor 열기, USER 수동 Save 순서를 유지한다.

### 4.6.4 구현·테스트 범위 제한

- 구현은 기존 `SCFVehicleBuilderTab`의 Quick 화면/선택/상태/전체 인벤토리 UI와 기존 `FCFVehicleBuilderHardpointIntegrity`의 표준 HP 번호 계산 helper(필요한 최소 분량)로 한정한다. 새 Runtime Schema, Asset Catalog, DataAsset, 공용 분류 Registry/JSON/Excel은 만들지 않는다.
- Quick의 전체 소켓 목록은 `UStaticMesh::Sockets`을 직접 읽는다. **Recipe가 없어도** 0개/다수/비표준/미래 이름 모두 표시해야 한다. 기존 소켓이 이미 있는 경우 Transform을 보존하는 것을 Automation으로 확인한다.
- 표준 7종 중 같은 category 복수/서로 다른 Transform, 99→100 이상, 번호 간 결손, 다른 category 병렬 생성, 비표준·미인식 이름 보존/필터 표시, 소켓이 많은 Mesh에서 목록 탐색, invalid Mesh/Category, 동적 category 소스 변경 시 UI 출처, 역할 이름 전환, Quick 복귀/새로고침 Recipe mutation0 테스트를 추가한다.
- 실제 Editor UX 수락에서는 다양한 위치의 기존 소켓을 다시 열어 수동 Transform 수정·Save 가능한지, 표준/기타 구분이 사용자를 혼란시키지 않는지 확인한다. 핵심 기능을 넘어서는 자동 위치 배치·종류별 preset 좌표는 채택하지 않는다.
- **검수 범위:** 위 교정은 신규 기능 구현 전 문서 계약이며 UE 빌드/자동화/실제 화면의 성공을 뜻하지 않는다. 기존 CF-FQ-059/058, GoPyMCP, AssetDump, 기타 Dirty 보호.

## 4.7 VSPUX-P0-01 USER 의미 정정 — Hardpoint 위치 **분류** 확장 (2026-10-14, 현재 category 계약; 구현 필수 범위는 §4.9)

> **사용자 명시 정정:** 여기서 "소켓 위치"는 `RelativeLocation` 좌표나 회전이 아니라 **`Top`, `Front`, `LeftSide` 등 Hardpoint 위치 분류(Category)**다. §4.6의 좌표·Transform 다양성이 요구 핵심이라고 해석한 부분은 폐기한다. 현재 7개 위치 분류는 초기에 제공되는 목록이며, 향후 새로운 **위치 분류 자체가 추가될 수 있어야 한다.**

### 4.7.1 확장 단위와 단일 출처

1. `Top`, `Front`, `Back`, `LeftSide`, `RightSide`, `Bottom`, `Internal`은 **현재 지원 중인 7개 위치 분류**이고 변경 불가능한 enum/화면 슬롯/버튼 개수로 취급하지 않는다.
2. 위치 분류 후보의 출처는 기존 `FCFVehicleBuilderVM::GetStandardHardpointCategories()`가 노출하는 `StandardHardpointCategories()` 계약을 우선 재사용한다. Quick UI는 여기서 **항목 수에 관계없이** 선택 옵션을 구성하고, `Top` 등의 이름을 UI 자체에 고정 `switch`/분기해 유효성을 결정하지 않는다. 기존 한국어 표시명 7개를 가진 경우에만 별칭을 쓰고 추가된 이름은 정확한 원문을 fallback으로 보여준다.
3. 모든 분류에 동일한 이름 생성·증분 규칙을 적용한다. `<Category>_<NN>` / `HP_<Category>_<NN>`의 `Category`는 current supported 목록의 **값**이다. 예를 들어 *향후 정식 지원 목록에 `RearUpper`가 추가된다면* `HP_RearUpper_01`, `HP_RearUpper_02`를 별도 버튼 코드 없이 제안할 수 있어야 한다. `RearUpper`는 **가상 예시**이며 현행 supported 목록에 있다고 주장하지 않는다.
4. `FCFVehicleBuilderHardpointIntegrity::TryParseCanonicalStandardSocketName()`은 caller가 제공한 `StandardCategories` 목록에 따라 파싱하므로 확장할 때도 **화면 선택 목록·이름 제안·파서·Recipe Hardpoint validation/Consumers**가 동일 category 집합을 해석하도록 영향 범위를 함께 검수한다. 목록만 바꿔 게임 Runtime에서 새 분류가 자동 지원된다고 주장하지 않는다.
5. 새 위치 분류에 대한 임의 문자열 자유 입력, 신규 영구 Category Registry/Config, 새로운 Runtime enum/DA/Schema, 미리 알 수 없는 위치 목록 선생성은 **이번 P0-02의 필수 범위가 아니다**. 실제로 신규 분류의 게임 의미가 확정되는 시점에 기존 단일 출처에 항목을 추가하고 직접 영향 받는 사용처를 함께 갱신한다.

### 4.7.2 Quick 화면의 실제 필수 UX

```text
소켓 미리 준비
└─ StaticMesh 선택
   ├─ Wheel FL / FR / RL / RR: 현행 이름 확인·누락 생성
   └─ Hardpoint 위치 분류 선택: [Top ▼]  ← 지원 목록을 동적으로 표시
       ├─ 기존 HP_Top_01, HP_Top_02 ... 확인·편집기 열기
       ├─ 다음 제안 HP_Top_03
       └─ [추가 후 편집] → 기존 StaticMesh Editor에서 USER가 직접 배치
```

- 기존 소켓 확인·중복 방지·기존 UI 복귀·Recipe 격리는 이전 P1 교정대로 유지한다. **핵심은 분류 목록 확장성**이며 부차적인 임의 좌표/Transform 수치 패널, 필터·검색·각종 물리 위치 프리셋은 구현 완료 조건에 넣지 않는다.
- 기존 비표준/미래 category로 보이는 소켓도 삭제·rename 없이 보존하고 가능하면 현재 소켓 목록에 나타낸다. 단, **현재 지원하지 않는 category의 새 canonical HP 이름을 Quick에서 자동 생성하거나 게임 연결 가능하다고 표시하지 않는다.**
- 테스트는 기존 7개 선택, 각 분류 독립적인 max+1 및 다수 인덱스, **지원 목록에 새 category 항목이 생겼을 때 UI 항목·동일 생성 규칙·Parser validation이 연동될 수 있는 구조**, 기존 Step 3 custom/Recipe 경로 불변에 집중한다. 가상 미래 category는 transient 테스트 입력으로만 사용하고 생산 코드를 임의 확장하지 않는다.

**최신 판정:** P0-01 사용자 의미 정정 및 설계 계약 교정 PASS. 기존 P0-01에서 발견한 P1 3건과 Wheel 이름 불일치 교정은 P0-02에 그대로 포함한다. 구현·빌드·Automation·UE UX/USER 수락은 별도로 수행해야 한다.

## 4.8 VSPUX-P0-01 USER 레이아웃 후속 수정성 계약 — 2026-10-14 (현재 구조 계약; 화면 격리는 §4.9)

**USER 결정:** 지금은 텍스트·도식만으로 레이아웃을 확정하지 않는다. **P0-02에서 실제 사용 가능한 최소 화면을 구현하고, P0-04에서 UE Editor 화면을 USER가 본 뒤 배치/간격/버튼 위치/표시 순서를 조정한다.** 그 과정에서 소켓 생성·선택·검증이 망가지지 않도록 구현 경계를 정한다. 특정 시안·행 배치·필터·좌표 패널을 지금 최종 UX로 동결하지 않는다.

### 화면과 동작의 실질 분리 (기존 Slate Tab 안에서)

```text
SCFVehicleBuilderTab (기존 단일 Tab / 별도 신규 도구 없음)
│
├─ Quick UI 빌더: 화면 배치만 소유
│  ├─ 메시 선택 영역
│  ├─ Wheel 영역 (FL/FR/RL/RR)
│  └─ Hardpoint 영역 (지원 Category 선택 / 현재 소켓 / 신규 추가)
│
├─ Tab-local Quick 상태 + 조회/액션 Handler: 기능 연결
│  ├─ bQuickSocketPreparationActive, QuickSocketChassisMeshPath
│  ├─ 선택된 QuickHardpointCategory (현재 지원 분류 FName)
│  ├─ Quick 대상 메시·live Socket readback
│  ├─ Wheel role → 현재 exact 이름 조회
│  └─ 추가·복사·Editor 열기·Refresh·Quick 종료
│
└─ 기존 기능/검증 owner 재사용
   ├─ 현재 GetStandardHardpointCategories / HardpointIntegrity parser
   ├─ category별 HP_<Category>_<NN> 다음 이름 계산
   └─ 기존 Transaction 기반 StaticMeshSocket 1개 생성 (Save0)
```

1. `BuildQuickSocketPreparationPage()`(화면 조립), 메시 선택·Wheel·Hardpoint 각 **작은 UI 생성 helper**(실제 함수명은 프로젝트 스타일에 따라 결정)를 기존 `SCFVehicleBuilderTab` 안에서 분리한다. 이 helper는 **Slate widget의 레이아웃, 텍스트, 정렬, 패딩, 표시 순서**만 맡는다. 소켓 생성·이름/Category 유효성·Recipe 변경 판단을 `SNew(...)` 블록이나 `OnClicked_Lambda` 안에 복제하지 않는다. 새 UI class/일반-purpose framework/영구 Layout 설정 시스템은 만들지 않는다.
2. 기능 Handler는 UI 요소의 생성 순서, widget 주소, 트리의 slot/child index, 특정 `SBox`의 배치에 의존하지 않는다. Wheel은 **stable RoleIndex**, Hardpoint는 **현재 선택된 Category FName**, 대상은 **현재 Quick Mesh path**를 입력으로 사용한다. 분류 목록은 기존 VM source에서 읽으며 버튼 형태·드롭다운 형태를 교체해도 같은 선택/추가 Handler를 호출한다.
3. 특히 기존 Step 3에서 확인된 `BuildRequiredWheelSocketRow`의 **creation-time `SocketName` 캡처 버그**를 P0-02에서 교정한다. 모든 Wheel 표시/복사/활성/생성은 동일한 current role→SocketName resolver를 사용하고, 버튼 클릭 시 현재 role/name/mesh를 fresh 재조회한다. Hardpoint도 선택 Category·다음 SocketName을 **클릭 시 live 검증**하며 옛 UI render 시점의 candidate를 무조건 생성하지 않는다. 표시된 후보가 외부 변경으로 stale해졌다면 먼저 재표시하고 이전 이름으로 생성하지 않는다.
4. Quick 전용 임시 상태(메시/선택 Category/Quick 모드)는 Widget 자체가 아닌 **Tab-local state**에 저장한다. Widget 재생성, 패널 교체, 메시 편집기 왕복, Quick 전용 새로고침에서는 해당 상태를 **유효한 범위에서 보존**한다. 메시가 바뀌면 목록/후보만 해당 새 메시로 다시 계산한다. Category가 더는 supported set에 없으면 잘못된 이름을 생성하지 않고 선택을 해제하거나 첫 유효 항목을 표시하되 **USER에게 변경을 알린다**. 기존 VM current vehicle/Step2 pending/Step1~8 state는 화면 레이아웃 변경이나 Quick refresh로 수정하지 않는다.
5. UI 갱신은 기존 `SBox::SetContent(...)` 기반 presentation host 패턴을 재사용할 수 있다. UI만 재빌드하여 이름/상태를 최신 표시하고, **widget rebuild가 Asset mutation·Recipe refresh·저장·selection reset을 유발해서는 안 된다**. 가능하면 Quick 영역만 재생성한다. 기존 `CommonPageScrollBox` 및 Step 3 전용 host의 소유권을 보존하고, 화면 교체가 일반 Step 1~8 페이지 visibility를 풀지 않게 한다.
6. 현행 기본 Layout은 **최소한으로 조립**한다. UX 확정 전 Flex/Grid/layout helper 과다 일반화, UI 스타일 설정 DataAsset, Blueprint/UMG 이중 구조, 사용자에게 보이지 않는 추가 옵션은 만들지 않는다. 실제 USER 피드백이 들어온 후 각 UI helper의 Slate 계층·배치만 국소 변경하도록 한다.

### Layout-only 변경의 안전 검수 계약

**기능 불변 확인 기준**: UI helper 안의 `SNew` 배치/라벨/패딩/표시 순서만 변경했다면 이름 제안·Socket 생성 Transaction·Category source·VM/Recipe writer·Asset Save 로직의 semantic diff는 **0**이어야 한다. 실제로 행동·상태를 변경해야 할 이유가 생겼다면 layout-only로 위장하지 않고 별도 semantic-unit 변경/검증을 한다.

- **기능 자동 검증**: 지원 category 선택→생성 이름·번호·중복/존재 판정, Wheel role exact 이름 display/copy/click 정합성, invalid mesh, Quick Recipe mutation0, 기존 Step 3/8-step 계약은 focused regression으로 보존한다. 순수 helper 테스트는 Slate 실제 버튼 연결을 검증하지 못하므로 둘을 구분한다.
- **화면 수락(실제 UE Editor)**: 메시 선택→Top/Front 등 전환→새 이름 생성→기존 소켓 편집→Quick 새로고침→나갔다 재진입, 다른 차량 선택 및 +새 차량 만들기, 공유 메시 경고, 좁은/넓은 화면에서 버튼 가림·오동작·스크롤 경계가 없는지를 확인한다. UI 후속 수정 때는 이 동선의 **짧은 수동 smoke**를 반복한다. 자동 Slate UI test harness가 실제로 없다면 자동 테스트라고 포장하지 않는다.
- **후속 레이아웃 교정 반복 단위**: ① 사용자 화면 피드백 → ② 해당 UI builder만 배치 수정 → ③ 컴파일/diff에서 기능 Handler 변경 여부 확인 → ④ focused test + 편집기 핵심 동선 smoke → ⑤ USER 화면 재수락. 레이아웃 변경만으로 수락된 기존 기능을 재설계하지 않는다.
- **판정 경계**: 코드 경계와 테스트가 **고장 가능성을 줄이고 발견하기 쉽게 만들지만 절대적 무고장을 보증하지는 않는다**. P0-02는 우선 기능 동작, P0-03은 기술 검증, P0-04는 **실제 화면 USER 확인 후 필요한 레이아웃 교정 및 재수락**으로 구분한다. UI 최종 배치는 지금 PASS로 기록하지 않는다.

## 4.9 VSPUX-P0-01 Post-review P1/P2 Correction — 2026-10-14 (P0-02 구현의 단일 최신 계약)

> **현재 구현 계약의 우선순위:** 이 §4.9가 P0-02의 **필수 화면 범위·모드 전환·검증 조건**을 소유한다. §4.7의 위치 *분류(Category)* 규칙과 §4.8의 UI/기능 분리는 그대로 적용한다. §4.5(초기 계약)·§4.6(좌표 중심 추가 해석)은 Historical이며, 서로 다른 레이아웃 시안·좌표/Transform 수치 패널·필터/검색을 현재 필수 기능으로 다시 승격하지 않는다.

### P1-01. Browser refresh와 사용자 선택 이벤트를 분리한다

현재 `SCFVehicleBuilderTab::HandleRefreshVehicles()`는 `ViewModel->RefreshVehicles()` 뒤 `VehicleListView->SetSelection(CurrentRow, ESelectInfo::Direct)`로 시각적 row highlight를 복원한다. `HandleVehicleSelectionChanged()`는 `SelectInfo`를 사용하지 않고 `ViewModel->SelectVehicle()`을 호출한다. 실제 `SelectVehicle()`은 생성 draft/selection/Step과 일부 임시 검토 상태를 변경하므로 **프로그램이 row highlight를 복원하는 것만으로 사용자 차량 선택이 다시 실행되면 안 된다**.

- `대상 목록 새로고침`은 차량 목록/하이라이트 갱신 작업이다. 사용자 의도가 없는 자동 `SetSelection(..., Direct)`, `RequestListRefresh` 파생 선택 이벤트로 `SelectVehicle()`을 다시 호출하지 않는다. **같은 current target의 자동 복원 이벤트를 식별해 무시**하며, 필요한 경우 refresh 구간의 programmatic selection만 보호하는 좁은 guard를 둔다. 다른 의도로 발생한 `Direct` 이벤트까지 무조건 차단하는 전역 규칙은 만들지 않는다.
- Quick 활성 상태에서 목록 새로고침은 `bQuickSocketPreparationActive`, `QuickSocketChassisMeshPath`, 현재 `QuickHardpointCategory` 및 화면 표시 상태를 유지한다. 일반 제작 상태에서는 선택 차량 identity, current Step index, 신규 제작 draft 및 이미 준비된 검토 작업을 **단순 하이라이트 복원 때문에** 재시작·초기화하지 않는다.
- `ViewModel->RefreshVehicles()` 자체도 내부 `AuthoringViewModel->RefreshBrowser()`와 `RebuildStepStates()`를 호출한다. 그러므로 **콜백 차단만으로 Quick/기존 VM 불변을 입증했다고 간주하지 않는다**. 구현 시 Refresh 전후 해당 VM transient state와 선택 identity를 실제로 비교하고, refresh 내부에서 의도하지 않은 clear/reselect가 확인되면 이 직접 영향 경로를 좁게 교정한다. authoritative Step 상태의 정상 재평가와 무관한 선택·draft 파괴는 분리 판정한다.
- 사용자 **직접 차량 row 선택**은 Quick 종료 후 기존 `HandleVehicleSelectionChanged()` → `SelectVehicle()`을 **정확히 한 번** 실행한다. `+ 새 차량 만들기` 직접 클릭도 Quick 종료 후 기존 `HandleBeginNewVehicleEntry()`를 한 번 실행한다. Quick `돌아가기`는 기존 VM 선택/Step을 변경하지 않는다. 프로그램 복원과 사용자 직접 클릭의 차이를 테스트로 확인한다.
- 대상 목록 새로고침과 Quick의 `현재 소켓 다시 확인`은 서로 다른 동작이다. 후자는 **Quick 대상 StaticMesh live Sockets만** readback하고 VM Recipe refresh를 호출하지 않는다. 일반 Step 3의 `현재 상태 다시 확인`은 현행 `ViewModel->RefreshCurrentState()` 경로를 그대로 유지한다.

### P1-02. Quick 표시 영역은 Step 3의 UI host를 재사용하지 않는다

현재 일반 Step 3 본문은 `GetSocketPreparationVisibility()`의 Recipe/Step3 조건 안에 있고, `HardpointPlanningHost`의 `SetContent(BuildHardpointPlanningPanel())`은 **Recipe 기반 Hardpoint 작성/채택**과 연결된다. 이것을 Quick 전용 subtree로 갈아 끼우면 나중에 Quick 레이아웃 재배치 시 일반 Step 3까지 의도치 않게 영향을 받는다.

- 동일 `SCFVehicleBuilderTab` 안에 **서로 다른 표시 owner** 두 개를 둔다. (1) 일반 Step 1~8 body + 기존 `HardpointPlanningHost` (2) **Quick 전용 `QuickSocketPreparationHost` 또는 이름이 명확한 동등 `SBox`**. Quick 전용 UI는 자체 `BuildQuickSocketPreparationPage()` 및 작은 section builder로만 재생성한다.
- 두 화면의 Visibility는 **상호 배타적**이다. Quick에서 기존 Step 1~8 본문/Recipe·Physics·Gameplay·Driving 액션/고정 Step 이전·다음·Advanced/footer navigation은 노출하지 않는다. 기존 Step 3의 `GetSocketPreparationVisibility()` 규칙은 변경하지 않고 Quick에는 **별도 Quick visibility**를 적용한다. 상단 Target/Step 요약은 Quick 전용 문구를 사용하며 `CommonPageScrollBox` 단일 스크롤 owner와 reset semantics를 보존한다. Quick 전용 복귀/소켓 refresh 액션은 Recipe Step handler와 연결하지 않는다.
- 기존 `HardpointPlanningHost`를 Quick 내용으로 `SetContent`하거나 `BuildHardpointPlanningPanel()`의 Recipe 작성 버튼을 Quick에 노출하지 않는다. **공유할 것은 read-only 역할별 이름 조회·live Socket 검사·SocketName 생성 helper·StaticMesh Transaction 기반 1개 추가/Editor 열기 경로**다. 이 기능들의 대상은 `GetSocketEditingChassisMeshPath()`(Quick 경로 또는 일반 현재 Chassis)로 결정하고, 생성 성공 후 Quick은 Quick subtree만 readback/refresh, 일반 Step 3은 기존 VM refresh를 유지한다.
- Wheel `RoleIndex`와 Hardpoint `FName Category` 및 선택 `StaticMesh` 경로는 Tab-local로 유지하고, UI slot index/Widget 인스턴스/Construct-time SocketName을 mutation 권한으로 쓰지 않는다. **한번 그려진 Hardpoint 후보와 클릭 당시 소켓 목록이 달라지면** 자동으로 다른 이름을 추가하지 말고 후보를 갱신·사용자에게 알린 뒤 다시 클릭하게 한다.
- Quick 종료 후 기존 Step 3 `HardpointPlanningHost`의 presentation은 이전 Recipe/current selection에 일치해야 하며, Quick에서의 UI 재배치·새로고침 때문에 기존 Step 3 선택 상태나 callback을 재실행하지 않는다.

### P2-01. 필수 기능과 후속 UX 실험을 구분한다

| P0-02 **필수** | 지금 **필수 아님** |
| --- | --- |
| 기존 Builder의 Quick 진입/복귀 + StaticMesh 직접 Picker | 별도 Socket Tool/별도 Editor Tab/전체 Builder 재설계 |
| Wheel 4-role exact 이름 확인·복사·필요한 이름 1개 생성 | Wheel role/차량 차륜 수 확장 |
| 현재 지원 category의 동적 선택 및 `HP_<Category>_<NN>` live max+1 생성 | 현재 미지원 category 임의 이름 입력이나 Runtime 자동 지원 |
| 선택한 Mesh의 **기존 Socket 이름** 확인·복사·StaticMesh Editor 열기 및 누락 구분 | 전체 Transform 수치 패널, 고급 필터/검색, 좌표/회전 자동 배치 |
| Quick/Step3 화면 소유권 분리와 현재 대상·Category 상태 보존 | 데이터 Schema/Category Registry/새 ViewModel/Blueprint 이중 구현 |
| Live re-check/중복/invalid guard, 명시 추가, Transaction, 사용자 수동 Save | 자동 Save/Recipe Hardpoint intent·VehicleData·Mount 작성 |
| 기존 Step 1~8/Recipe/custom Wheel binding 회귀와 USER 사후 레이아웃 교정 동선 | 초기 화면 배치를 최종 USER UX로 조기 확정 |

기존 비표준·미지원 category Socket은 임의 삭제/rename하지 않고, **기존 Socket 목록에 표시해 Editor에서 접근**할 수 있어야 한다. 단, 미지원 분류의 게임 연결을 약속하지 않는다. 목록의 세부 배치·스크롤 표현은 구현 후 USER 검수에서 조정한다.

### P0-02→P0-04 검증/판정

1. **선택/Refresh 진입 분리:** Quick 활성 + 대상 목록 Refresh + 프로그램 `Direct` highlight 복원에서도 Quick Mesh/Category/Mode 및 기존 VM current selection/Step/신규 draft가 유지되고, 직접 row 클릭이나 신규 차량 버튼에서만 해당 기존 액션이 한 번 실행되는지 확인한다.
2. **두 UI host 격리:** Quick on/off, Picker 전환, 소켓 추가/Editor 왕복, Quick subtree UI rebuild 뒤 기존 Step 3/1~8 액션과 visibility가 변경되지 않았는지 확인한다. Quick `현재 소켓 다시 확인`은 Recipe state를 변경하지 않는다.
3. **소켓 기능:** Wheel 이름 Text/Status/Copy/Click 일치, Hardpoint category별 max+1/번호 결손/중복/unknown/invalid/overflow/다른 category 분리, 기존 Socket/Transform 보존, no Recipe/VehicleData mutation, 명시 저장0. Engine helper Automation 결과와 실제 Slate 버튼 클릭 USER smoke 결과를 **별개**로 기록한다.
4. **레이아웃 변경 내구성:** P0-04 USER가 구현된 Slate 화면을 본 뒤 각 Quick UI builder의 표시 순서·크기·위치만 국소 수정; 기능 Handler semantic diff0 확인, 공식 컴파일/focused regression + 실제 Editor 좁은/넓은 화면과 클릭 smoke를 거쳐 다시 USER 수락한다.

**설계 재검수 범위:** 이 §4.9는 2026-10-14 검수에서 발견한 P1-01, P1-02, P2-01의 구현 계약 교정이다. 현재 C++/UE Asset/Build/Automation/PIE/USER UX는 아직 실행하지 않았다. P0-02 구현을 완료하거나 기술/사용자 수락을 선행했다고 주장하지 않는다.

## 4.10 VSPUX-P0-02 C++ 최소 구현 체크포인트 — 2026-10-14 (Retained Implementation Evidence)

**구현 상태:** 실제 Source 6개 파일에 적용. 기존 Builder 안에 Recipe-free Quick Socket Prep를 추가했고 별도 Socket Tool/Blueprint/Category DB/Runtime Data schema는 만들지 않았다. 초기 Slate 레이아웃은 **USER 시각 수락 이전 시제품**이다. §4.9는 이 구현이 따라야 할 계약이며 §4.5~§4.6의 당시 UI 시안은 현재 구현 요구가 아니다.

### 실제 적용 내용

1. **Quick 독립 화면:** `SCFVehicleBuilderTab`에 좌측 `소켓 미리 준비` 진입, Quick `SObjectPropertyEntryBox` StaticMesh Picker, `QuickSocketPreparationHost`, Quick 복귀·새로고침, Tab-local 대상 메시/선택 category/화면 모드를 구현했다. 기존 `HardpointPlanningHost`와 Step 1~8 body는 유지하고 Quick/일반 표시와 기존 Footer를 상호 배타적으로 분리했다. 기존 `CommonPageScrollBox` 한 개를 공유한다.
2. **Wheel/Hardpoint:** Quick Wheel 4-role 기본 이름을 현재 메시 live `FindSocket`으로 확인·복사·추가한다. `GetStandardHardpointCategories()`에서 현재 지원하는 위치 분류를 읽어 목록을 구성하고, 별도 `TryGetNextStandardSocketName()`에서 canonical 이름의 분류별 최대 인덱스 +1, 결손·충돌·99→100·정수 오버플로를 검사한다. 기존 비표준/미래 Socket 이름은 그대로 나열하고 이름 복사·메시 Editor 열기로 접근한다. `FCString::Atoi` 범위 위험은 `LexTryParseString`으로 교정했다.
3. **공통 생성/상태 보호:** `AddChassisSocketAtOriginAndOpenForMesh(MeshPath, SocketName, bRefreshRecipe)`에서 기존 `FScopedTransaction`, `StaticMesh->Modify()`, 생성/중복 검사, `MarkPackageDirty()`, `PostEditChange()`, Editor 열기 경로를 재사용한다. Quick 생성 후에는 Quick 화면만 갱신하고 **VM Recipe Refresh/Save는 실행하지 않는다**. 일반 Step 3은 기존 `ViewModel->RefreshCurrentState()` 계약을 유지한다. 기존 Step 3 Wheel의 렌더 당시 SocketName 캡처 대신 현재 `WheelRoleIndex`로 버튼 enable/click에서 actual 이름을 재조회한다.
4. **Browser 선택 복원 보호:** `HandleRefreshVehicles()`의 `RequestListRefresh`/`SetSelection(..., Direct)` 동안 좁은 `bRestoringVehicleSelection` guard를 두고, 지연된 same-target `Direct` 콜백도 재선택하지 않는다. 다른 대상 직접 선택/새 차량 생성은 기존 명시 작업을 유지한다. 소스 확인 결과 `FCFVehicleAuthoringVM::RefreshBrowser()`는 목록 데이터를 갱신할 뿐 current 선택을 Clear하지 않는다. `FCFVehicleBuilderVM::RefreshVehicles()`는 Step 재평가를 하지만 기존 target 선택을 직접 변경하지 않는다.

**대상 코드:** `CFVehicleBuilderTab.h/.cpp`, `CFVehicleBuilderHardpointIntegrity.h/.cpp`, `CFVehicleBuilderHardpointIntegrityTests.cpp`, `CFVehicleAuthoringVMTests.cpp` (모두 `UE/Source/CarFight_ReEditor`의 해당 Public/Private DataAuthoring). Direct consumers와 scoped 테스트만 변경했다.

### 실행·재검증 Evidence

| 검증 | 실행 결과 | 한계/의미 |
| --- | --- | --- |
| UE 5.8 Source Build, `CarFight_ReEditor Win64 Development` | **PASS / exit 0**, 최종 C++ 테스트 추가 포함 `build_job_id=aa6d8272c05e4dc499b8aa0780ba41d8` | 첫 build는 잘못된 헤더 include 때문에 FAIL 후 원인 교정. `06b66161910a4b47b22dc0069e8e0d4a`로 중간 재빌드 PASS, 최종 테스트 추가 뒤에도 PASS. |
| `CF_FQ_060.VSPUX_P0_02.QuickSocketNextName` | **PASS 1/1**, exit 0, failed/missing 0 | 실제 transient Mesh에서 max+1, category 분리/확장, 99→100, invalid·overflow 검사. |
| `CF_FQ_040.VB_P0_09.BuilderShell` | **PASS 1/1**, exit 0, failed/missing 0 | 기존 Step1~8 Slate Tab 생성과 Browser refresh 중 선택 차량/Step/Chassis 보존을 확인하는 직접 영향 회귀가 포함됨. |
| `CF_FQ_047.VBHAI_P0_04.InventoryClassification` | **PASS 1/1**, exit 0, failed/missing 0 | 기존 Recipe 기반 Socket inventory/Hardpoint classification 회귀 보호. |
| Diff/readback + Dirty 범위 | **소스와 문서의 대상 경로만 확인**, 기타 user Dirty 보호 | 최종 Git diff review 및 metadata 동기화 별도 확인 |
| 실제 UE Editor Quick 화면 수동 조작/레이아웃, USER Save | **NOT RUN / P0-04 USER Acceptance 대기** | 실제 배치/폭/스크롤·기존 소켓 편집 버튼 동선·버튼 클릭의 시각적 UX를 수락한 것이 아니다. |

**검증 판정:** P0-02 구현 + 공식 컴파일 + **3개 focused Automation 기술 PASS**. 실제 Slate Quick 화면의 USER 조작·시각 수락은 별도이고 자동 helper 검증으로 대체하지 않는다. 향후 Quick UI-only 배치 변경은 이름/생성/VM semantic diff0를 확인하고 Build·focused test·USER smoke를 반복한다. Git stage/commit/push는 이 작업에서 수행하지 않았다.

**Next:** `VSPUX-P0-03`의 최종 통합·diff 증거를 확인하고, `VSPUX-P0-04`에서 USER가 실제 Editor 화면을 보고 UI만 국소 조정/재검증한다. 증거 없는 실제 Slate 클릭, 동적 메시 편집 성공 또는 USER 수락 PASS를 주장하지 않는다.

## 4.11 VSPUX-P0-03 최종 기술 재검수 — 2026-10-14 (Current Evidence)

**판정: TECHNICAL REVIEW PASS / USER 편집·시각 수락 미실행.** §4.9 구현 계약을 현재 Git Dirty와 실제 C++ 6개 파일에 대조했다. 기존 Step 3 화면·Recipe writer에 Quick UI를 섞지 않고, 기존 StaticMesh 생성 Transaction만 공통 사용한다. Quick은 별도 `QuickSocketPreparationHost`와 Tab-local 대상 메시·위치 분류를 소유하고, 목록 새로고침의 programmatic `Direct` 복원과 사용자 명시 선택을 구분한다.

### 재검수 중 발견하여 교정한 문제

- **P1 UX 상태 오표시:** Quick에 선택된 유효 StaticMesh가 없더라도 Wheel 역할을 `없음`으로 표시하던 문제. `SCFVehicleBuilderTab::GetQuickWheelSocketStatusText(int32)` read-only 상태 getter를 추가하여 **메시 확인 불가 / 실제 소켓 없음 / 실제 소켓 있음** 세 상태로 분리했다. Slate 행은 Construction-time snapshot 대신 getter를 연결해 외부 Socket 수정 후에도 실제 live `FindSocket` 상태를 읽는다.
- **P1 검증 공백:** 기존 `BuilderShell` 검증은 Builder Tab 생성과 VM refresh를 점검하지만 Quick 모드 진입/복귀, Quick vs Step3 host identity, Quick Category·Mesh 상태 보존 및 same-target `Direct`/명시적 row selection을 직접 호출하지 않았다. `CF_FQ_060.VSPUX_P0_03.QuickModeBoundary` Automation을 추가했다. `WITH_DEV_AUTOMATION_TESTS` 조건의 focused test friend만 사용하며 제품용 공개 접근자/별도 테스트 전용 Framework는 만들지 않았다.

### 실제 실행 결과

| 대상 | 결과 | 증명 범위 |
| --- | --- | --- |
| **공식 UE 5.8 Source Editor Win64 Development Build** | **PASS / exit 0**, build job `ad88787677da413aba23034cbd0c507b` | 최종 교정과 새 Automation을 포함한 전체 변경 소스 컴파일·링크 |
| `CarFight.DataAuthoring.CF_FQ_060.VSPUX_P0_02.QuickSocketNextName` | **PASS 1/1 / exit 0** | Category 확장, 독립 번호 max+1, invalid/overflow, 현재 메시에 있는 canonical Socket name |
| `CarFight.DataAuthoring.CF_FQ_060.VSPUX_P0_03.QuickModeBoundary` | **PASS 1/1 / exit 0** | 실제 `SCFVehicleBuilderTab` 생성; Quick/Step3 host 별도, 화면 on/off, Browser refresh 중 Quick mode/Mesh/Category/Step 보존, USER row/new-vehicle 전환, same-target Direct 보호, Wheel 3상태 live 표시. 관련 Quick filter 실행 job `fa9684fad82049689c5f0bf5f0194095`에서 2/2 PASS |
| `CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell` | **PASS 1/1 / exit 0** | 기존 8 Step·Builder tab 생성 및 managed Browser refresh selection/Step/Chassis 회귀 |
| `CarFight.DataAuthoring.CF_FQ_043.VMG_P0_03.Step3HardpointPlanning` | **PASS 1/1 / exit 0** | 기존 Step3 Recipe 기반 Hardpoint 선택·작성 회귀 |
| `CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.InventoryClassification` | **PASS 1/1 / exit 0** | 기존 Hardpoint 인벤토리/소켓 분류 경계 |
| **총 집중 Automation** | **5/5 PASS, failure 0** | 한 UE 프로세스에 Quick 2건, 이후 기존 직접 영향 3건 별도 프로세스 실행. 각각 engine exit 0 |
| Git diff/readback | **PASS / Scope 6 C++ files** | Quick entry, footer/Scroll visibility, Browser guard, UI host identity, Wheel click current role, common transaction diff 재확인; 추가된 SavePackage/SaveAll/Recipe commit/VehicleData Apply 호출 없음 |

### 기술 경계 및 잔여 사용자 검증

- **Quick UI/상태 변경에는 Recipe·VehicleData·Category Registry·Fitting/CCAS writer를 추가하지 않았다.** 기존 `FScopedTransaction` + 실제 `UStaticMeshSocket` 한 개 생성·dirty 처리·StaticMesh Editor 열기 기능을 재사용하며 자동 저장하지 않는다. Step 3 Wheel 이름은 클릭 시 최신 Recipe role binding을 조회한다.
- 신규 Quick focused test는 **실제 Slate Tab 및 handler/visibility를 자동 실행**하지만 **사용자 마우스 클릭·실제 콘텐츠 StaticMesh Editor의 Transform 편집·수동 Save를 수행하지 않는다.** Engine/Transient 테스트가 제품 `.uasset`에 적용되었다고 주장하지 않는다.
- 초기 Quick 레이아웃은 USER 리뷰 전 **시제품**이다. 레이아웃만 바꾸는 후속 조정은 해당 section builder와 UI Slot에 한정하고 socket handler/VM 계약을 변경하지 않는다. USER 검수 후 화면/버튼 동선, 수동 저장, 동일 Socket 중복·Transform 보존을 확인한다.
- 기타 기존 user Dirty(차량 Mesh/Map, Ammo, AssetDump plugin, CF-FQ-059 및 임시 Tool)는 수정·삭제하지 않았다. **Git stage/commit/push 없음.**

**P0-03 Gate: TECHNICAL PASS. NEXT P0-04:** 실제 UE Editor에서 사용자가 Quick 화면을 열고 대표 StaticMesh에 누락 Wheel/HP Socket을 추가·배치·수동 Save·복귀·기존 Step 3 회귀를 검수한다. 레이아웃 수정이 필요하면 semantic 변경 없이 UI만 국소 수정한 후 Build·focused tests를 재실행한다. USER 수락 전 P0-05 Closure나 Systems 승격을 선행하지 않는다.

## 4.12 VSPUX-P0-04 USER Compact 피드백 교정 — 2026-10-14 (Historical: 당시 빌드 잠금)

**사용자 재현:** Compact에서 Quick Wheel Socket 생성은 실제 StaticMesh Editor에 보이지만, Hardpoint `추가 후 편집`은 Quick의 기존 소켓 인벤토리/`현재 소켓 다시 확인`에 새 이름이 나타나면서도 이미 열려 있는 Static Mesh Editor Socket Manager에는 보이지 않음. 바퀴 소켓 생성 시 역할별 적절한 프리뷰 메시를 함께 지정해 달라는 요구.

**원인과 교정:** 기존 `StaticMesh->Sockets.Add(NewSocket)` 직접 배열 변경은 Quick의 live `Sockets` 목록에는 보이지만 이미 열린 에디터의 Socket Manager 갱신을 보장하지 않는다. 공통 생성 함수를 엔진의 `UStaticMesh::AddSocket(NewSocket)`으로 전환하고 곧바로 `FindSocket(SocketName) == NewSocket`를 readback해 false success를 차단했다. 기존 메시의 `IStaticMeshEditor`가 열려 있으면 `UAssetEditorSubsystem::FindEditorForAsset`에서 **동일 메시/StaticMeshEditor identity**를 확인하고 `RefreshTool()` + `SetSelectedSocket(NewSocket)`으로 표시 동기화한다. 에디터를 강제로 닫거나 재오픈하지 않으며 기존 Step 3에도 동일 공통 생성 경로를 사용한다.

**Wheel Preview:** 새로 생성하는 `Wheel_Anchor_FL/FR/RL/RR`에 한해서 각각 다음 오브젝트 경로를 `UStaticMeshSocket::PreviewStaticMesh`로 매핑했다: `Wheel_FL.Wheel_FL`, `Wheel_FR.Wheel_FR`, `Wheel_RL.Wheel_RL`, `Wheel_RR.Wheel_RR` (공통 폴더 `/Game/CarFight/Vehicles/Shared/Tire/`). Git에는 네 `.uasset` 파일이 존재한다. 로드 불가 시 소켓은 생성하되 진단 메시에서 프리뷰 지정 실패를 분리한다. **기존 Wheel Socket에 프리뷰를 소급 적용하지 않고 Hardpoint Socket에 임의 메시를 붙이지 않는다.**

**직접 영향 변경:** `CFVehicleBuilderTab.h/.cpp`, `CarFight_ReEditor.Build.cs`의 Editor-only `StaticMeshEditor` 의존성, `CFVehicleAuthoringVMTests.cpp` 신규 `CF_FQ_060.VSPUX_P0_04.SocketPreviewMapping` focused test. 테스트는 Wheel 4역할 실제 오브젝트 경로·UStaticMesh 로드, transient Chassis의 `UStaticMesh::AddSocket` readback과 Hardpoint 예외를 검사한다. 기존 Recipe/VehicleData/Runtime/소켓 Transform·Asset 저장 로직 변경 없음.

### 검증 Evidence / BLOCKED 판정

- 공식 UE 5.8 Source Editor Development Build job `6c8e81af4a624fb09b35b7a31c065061`: **FAIL / exit 6**. UHT 230초 완료 및 변경된 `CFVehicleBuilderTab.cpp`, `CFVehicleAuthoringVMTests.cpp` 포함 소스 compile 과정에서 관련 C++ error는 관찰되지 않았음. 그러나 **실행 중인 `UnrealEditor.exe`가 `UnrealEditor-NetCore.dll`과 `UnrealEditor-CarFight_ReEditor.dll`을 잠가** 양쪽 링크 모두 LNK1104로 실패. 따라서 **공식 전체 Build PASS 주장 금지**.
- 신규 Preview Mapping focused test: **NOT RUN** (최신 DLL 링크 실패). 기존 v0.1.7 P0-03 자동 테스트 5/5 PASS는 과거 기준 증거이며 교정 후 검증으로 대체 불가.
- Active Editor의 실시간 Socket Manager vs Quick 목록 readback/Compact 수동 Socket 추가·저장: **NOT RUN**. 관리형 UE bridge read 도구는 server policy에 의해 차단됐으므로 실제 에디터 동작 확인을 주장하지 않는다.
- 기존 Compact/다른 차량 Mesh `.uasset`, 테스트 `.uasset`, Map, AssetDump, CF-FQ-059, 기타 Dirty 보호. 에디터 강제 종료·자동 Save·Git stage/commit/push 0회.

**Next (블로커 해제 후):** 사용자가 기존 에디터의 편집·저장 필요 여부를 결정하고 Unreal Editor를 **정상 종료**하면 최신 UE 5.8 공식 Build를 재실행한다. Build PASS 이후 신규 PreviewMapping + QuickModeBoundary + QuickSocketNextName 및 BuilderShell/Step3HardpointPlanning/InventoryClassification 집중 Automation을 재검증한다. 이어 사용자 Compact 실제 UI에서 기존/새 Socket 목록 대조, 프리뷰 mesh 확인, Transform 수동 배치·Save·재로드를 수행한다. USER 수락 전 P0-04 PASS/P0-05 closure 금지.

## 4.13 VSPUX-P0-04 Compiler/Automation Re-validation — 2026-10-14 (Current Evidence)

**판정: 소켓 생성·바퀴 프리뷰 교정 기술 검증 PASS / USER Compact 실제 편집·시각 수락 PENDING.** 사용자 직접 에디터 종료 후, 이전 §4.12의 활성 `UnrealEditor.exe` DLL 잠금이 해제되었다. 고정 공식 UE 5.8 Source Build를 재시도하여 **빌드 PASS**하고 신규 및 직접 영향 Automation을 모두 실행했다. 앞선 §4.12의 LNK1104는 당시 실패 증거로 보존하되 **현재 차단 상태는 해제**되었다.

| 검증 | 실제 결과 |
| --- | --- |
| 공식 UE 5.8 Source Editor `CarFight_ReEditor Win64 Development` Build | **PASS / exit 0**, job `6aa1d40cfbb144d181a989abe908bbe1`. 이전에 잠긴 `UnrealEditor-NetCore.dll`, `UnrealEditor-CarFight_ReEditor.dll`의 링크 및 metadata 작성 성공 |
| `CF_FQ_060.VSPUX_P0_02.QuickSocketNextName` | **PASS 1/1 / exit 0** |
| `CF_FQ_060.VSPUX_P0_03.QuickModeBoundary` | **PASS 1/1 / exit 0** |
| `CF_FQ_060.VSPUX_P0_04.SocketPreviewMapping` | **PASS 1/1 / exit 0**. 네 역할의 프리뷰 오브젝트 경로 및 실제 StaticMesh 로드·transient `AddSocket` readback·Hardpoint 예외 |
| `CF_FQ_040.VB_P0_09.BuilderShell` | **PASS 1/1 / exit 0** |
| `CF_FQ_043.VMG_P0_03.Step3HardpointPlanning` | **PASS 1/1 / exit 0** |
| `CF_FQ_047.VBHAI_P0_04.InventoryClassification` | **PASS 1/1 / exit 0** |
| **총합** | **Automation 6/6 PASS / failure 0 / missing 0 / unexpected 0 / duplicate 0** |

검증 Job 증거: Quick 3건은 `050f31267d60420ea4b8db9c322339e2`, BuilderShell `e655b80e8ea34b81b8c3738995626de7`, Step3HardpointPlanning `aa5a3f5579774fe0994360fe9072f0a5`, InventoryClassification `c1b17654e33e44edb5b67cb651bea487`; 모두 engine exit0, terminal PASS.

**검증 범위의 한계:** transient fixture의 소켓 추가/프리뷰 참조와 기존 Builder 회귀가 PASS된 것이며, 실제 Compact `.uasset`에서 열린 Static Mesh Editor Socket Manager가 `RefreshTool` 이후 새 Hardpoint를 표시하는지는 아직 USER 실제 화면 검증이 필요하다. 기존 사용자 에셋은 자동 수정·저장하지 않았다. **VSPUX-P0-04 USER Acceptance는 여전히 PENDING**이고 P0-05 종료를 진행하지 않는다.

**운영 후속 문제 분리:** 사용자는 원격으로 개발 중 에디터 종료를 요청했고 미저장 폐기도 허용했으나, 현재 MCP의 `carfight.editor.stop` preset은 exact Project `runtime_instance_id`가 필요했고 대상 등록 ID를 확보할 수 없어 실행되지 않았다. 저장/강제 종료를 우회하지 않고 사용자가 직접 에디터를 닫았다. CarFight 기능 결함이 아닌 **관리형 Editor lifecycle/Runtime selector 조회·원격 종료 UX** 문제로 별도 검토한다. 이번 세션에서는 GoPyMCP 구현 변경·Git commit/push 없음.

## 4.14 VSPUX-P0-04 USER Hardpoint Preview 확장 — 2026-10-14 (현재 소스 체크포인트)

**요청:** Quick/기존 Step 3에서 새로 생성하는 **모든 지원 canonical Hardpoint 소켓**(`HP_<Category>_<NN>`)의 `PreviewStaticMesh`를 동일한 `/Game/CarFight/Weapons/Turrets/_Shared/Meshes/Base_Standard.Base_Standard`로 자동 지정한다. 기존 FL/FR/RL/RR 바퀴 프리뷰 매핑은 그대로 유지한다.

**구현:** `SCFVehicleBuilderTab::GetHardpointSocketPreviewMeshPath(SocketName, StandardCategories)`에서 기존 `FCFVehicleBuilderHardpointIntegrity::TryParseCanonicalStandardSocketName`을 이용해 현재 지원 category의 canonical Hardpoint만 분류한다. 공통 `AddChassisSocketAtOriginAndOpenForMesh`는 먼저 기존 Wheel role 프리뷰를 확인하고, 없으면 Hardpoint 공통 프리뷰를 조회해 **새 `UStaticMeshSocket`에만 지정**한다. 소켓 생성·Transaction·기존 Editor 갱신·실제 Socket readback·자동 Save0 계약은 변경하지 않는다. `GetStandardHardpointCategories()` 목록을 사용하여 미래 신규 위치 category도 소스에 하드코딩하지 않는다. 기존 Socket/Transform/Recipe/VehicleData에는 소급 변경하지 않는다.

**테스트 보강:** `CF_FQ_060.VSPUX_P0_04.SocketPreviewMapping`에 현재 7 category + 임의로 신규 지원하는 category, 비지원 category/비canonical 이름/타 소켓 제외, 실제 Base_Standard UStaticMesh 로드와 transient Chassis `AddSocket` 참조 유지 테스트를 추가했다. 요청한 `Base_Standard.uasset`의 Git 추적 파일 존재를 확인했다. 제품 에셋은 쓰거나 저장하지 않았다.

**검증 상태:** **SOURCE APPLIED / Git tracked asset exists / new code Build NOT RUN / modified Automation NOT RUN / USER Compact Editor acceptance PENDING.** 현재 UE Editor Bridge가 Connected이며 기존 사용자의 에디터 수명주기를 강제로 중지하지 않는다. 이전 §4.13의 Build+6/6 PASS는 **이번 Hardpoint Preview 추가 전 기준선**이므로 이 신규 수정에 대한 PASS로 재사용하지 않는다. 공식 UE 5.8 Build에서 새 DLL 링크 성공 후 위 focused test와 기존 Quick/Step3/Inventory 회귀를 재실행하고, 실제 Editor에서 새 Hardpoint 프리뷰·수동 Save·Socket Manager 목록 표시를 USER가 확인해야 한다.

**후속:** 기존 에디터가 사용자의 의도대로 정상 종료된 후 공식 Build → `SocketPreviewMapping` + `QuickSocketNextName`/`QuickModeBoundary` + BuilderShell/Step3HardpointPlanning/InventoryClassification → P0-04 실제 USER 수락. 이 범위에서 Git commit/push 없음.

## 4.15 VSPUX-P0-04 Hardpoint Base_Standard Preview 기술 재검증 — 2026-10-14 (Current Evidence)

**판정: USER-reported Build PASS / 관리형 focused Automation 6/6 PASS / 실제 Compact USER 수락 PENDING.** 사용자가 새 C++ 변경에 대한 UE 5.8 빌드를 직접 수행하고 성공했다고 명시했다. 이 작업에서는 빌드를 별도 재실행하거나 사용자 빌드 로그를 직접 확인하지 않았다. 이후 CarFightMCP_Admin의 관리형 `RunDataAuthoringTests.ps1`가 **현재 로드 가능한 Editor 코드**에서 새 프리뷰 테스트와 직접 영향 범위 테스트를 실행해 결과를 확인했다.

| 검증 | 결과 및 직접 증거 |
| --- | --- |
| UE 5.8 Source Build | **USER 확인 PASS** (빌드 실행·로그 직접 열람은 이 세션에서 NOT RUN) |
| `CF_FQ_060.VSPUX_P0_04.SocketPreviewMapping` | **PASS 1/1, exit0**. `Base_Standard` UStaticMesh 실제 로드, 현행 지원 Hardpoint 7분류 + 미래 추가 분류 공통 프리뷰 매핑, 비canonical/비지원/Wheel 제외, transient Socket `AddSocket` 참조 보존. 독립 실행 job `65364f9c874a431abdd7a8ee78d4d7a5` |
| `CF_FQ_060.VSPUX_P0_02.QuickSocketNextName` | **PASS 1/1, exit0**. 위치 분류별 canonical 다음 이름 |
| `CF_FQ_060.VSPUX_P0_03.QuickModeBoundary` | **PASS 1/1, exit0**. Quick Slate 호스트·모드·선택/새로고침 보존 |
| CF-FQ-060 위 3개 통합 확인 | **PASS 3/3, exit0, failure/missing/unexpected/duplicate 0**, job `bb36da429b6347a3b259187aa09f25ea` |
| `CF_FQ_040.VB_P0_09.BuilderShell` | **PASS 1/1, exit0**, job `f1aa313c9f5d485dabb48ca2784afa67` |
| `CF_FQ_043.VMG_P0_03.Step3HardpointPlanning` | **PASS 1/1, exit0**, job `5a33034fbfb04061a88f5d874d89f732` |
| `CF_FQ_047.VBHAI_P0_04.InventoryClassification` | **PASS 1/1, exit0**, job `c309da67501748e796a588ab26b91891` |

**최신 자동 검증 총계: 서로 다른 테스트 6/6 PASS, 실패·누락 0건.** 독립 Preview 테스트는 통합 3건에 포함되는 동일 테스트이므로 7번째 테스트로 중복 계산하지 않는다. Editor Automation은 제품 Compact 에셋을 저장하지 않는다.

**잔여 USER gate:** 실제 Compact에서 새로 생성한 `HP_<Category>_<NN>`의 `Base_Standard` 프리뷰 표시, Socket Manager가 이미 열린 편집기에서 새 항목을 바로 노출하는지, 위치 수동 조정 및 명시적 저장·재로드를 확인한다. **이미 존재하는 Hardpoint Socket에는 소급 적용하지 않는 정책**이다. 이전 사용자 관찰에서 Editor 탭 재오픈 후 소켓 목록이 나타났으므로, 열린 Socket Manager의 목록 갱신 UX는 실제 화면에서 반드시 재검증한다. USER 수락 전 P0-04 DONE/P0-05 Closure를 선언하지 않는다.

**소유권·Git:** Compact·기타 차량 에셋, Recipe/VehicleData 및 별도 사용자 Dirty 보호. 신규 실제 Asset 저장·Git stage/commit/push 없음.

## 4.16 VSPUX-P0-04 Socket Manager 실시간 목록 갱신 Deferred 결정 — 2026-10-14 (Current)

**USER 최종 결정:** Coupe 실제 차량 메시에서 소켓 생성·뷰포트 프리뷰는 정상이나 이미 열린 Static Mesh Editor의 Socket Manager 목록이 FL 한 행에 멈췄고, 편집 탭을 닫았다 다시 열면 추가된 전체 소켓이 표시된다. Compact에서도 같은 증상을 관찰했다. USER가 **이 표시 편의 결함을 더 깊게 수정하지 않고 Deferred로 분리**하기로 확정했다.

**범위·비용 상한:** Socket Manager 실시간 행 갱신은 핵심 소켓 생성/프리뷰/수동 Save와 별개인 **비차단 UX 편의**이다. 엔진 소스 수정, 커스텀 엔진 패치, 비공개 API나 불안전한 Slate 캐스팅, 별도 Socket Manager 신설, 추가 모듈·상태 체계 및 계속된 엔진 내부 탐색을 요구하지 않는다. 기존 공통 `AddChassisSocketAtOriginAndOpenForMesh()`의 소켓 생성·`RefreshTool()/SetSelectedSocket()` 동작도 이번 Deferred 결정에서 변경하지 않는다.

**현재 USER 작업:** Coupe의 소켓 준비는 USER가 완료했다고 보고했다(완료 보고 기록). 사용자 관찰로 뷰포트에는 추가된 소켓들이 표시됐고, Socket Manager 행만 뒤처졌다. 새 바퀴 프리뷰·Hardpoint `Base_Standard` 매핑은 §4.15 관리형 Automation 6/6 PASS로 기술 검증됐지만, Coupe 개별 소켓의 프리뷰·Transform·수동 Save·재로드 상태를 이 결정만으로 일괄 PASS 판정하지 않는다.

**임시 작업 동선:** 소켓 연속 추가 후 목록만 뒤처지면, 생성된 소켓/프리뷰는 뷰포트에서 확인하고 **Static Mesh Editor 탭을 닫았다가 다시 열어 목록을 재조회**한다. 미저장 사용자 편집은 사용자가 명시적으로 Save 여부를 결정한 후에 탭을 닫는다. 자동 Save, 창 강제 종료, Asset 자동 변경 없음.

**P0-04 판정 변화:** Socket Manager가 열린 상태에서 즉시 새 행을 표시해야 한다는 항목은 **Deferred / USER ACCEPTED LIMITATION / non-blocking**으로 옮긴다. 나머지 필수 USER Acceptance(소켓 올바른 이름/중복 방지, Wheel·Hardpoint 프리뷰, 수동 Transform·Save·재로드, 기존 Step 3 복귀)는 별개로 남아 있다. 따라서 **P0-04 전체 최종 승인·P0-05 종료는 아직 선언하지 않는다.** 다음은 Coupe 완료 결과를 기준으로 남은 필수 UX만 수락 확인하며, 이 화면 갱신 문제를 해결하기 위한 추가 구현은 수행하지 않는다.

**증거·보호:** §4.15의 USER-reported Build PASS / managed Automation 6/6 PASS를 그대로 유지한다(신규 Source 미변경). 기존 차량 .uasset Dirty, 사용자 저장물, Recipe/VehicleData, UE Engine Source, Git stage/commit/push 미변경.

## 4.17 VSPUX-P0-04 Coupe 저장·재로드 USER 확인 — 2026-10-15 (Current)

**USER 직접 확인 PASS:** 사용자 보고에 따르면 실제 Coupe Static Mesh를 다시 로드한 후 작업한 소켓들이 **전부 정상적으로 붙어 있는 것을 확인**했다. 이전 사용자 Coupe 작업 완료·Socket Manager 탭 재열기 후 전체 소켓 표시와 합쳐 **Coupe 작업 결과의 재로드 후 소켓 부착 유지**를 USER 검증 PASS로 기록한다. 이것은 사용자가 수행한 직접 확인이며, 이번 세션에서 시스템이 디스크 에셋을 별도로 읽고 검사한 독립 PASS는 아니다.

**범위:** 사용자 표현 ‘전부 제대로 붙어있음’에 따라 Coupe의 작업 소켓 연결·배치 상태에 대한 사용자 확인을 수락한다. 이전 §4.15의 Wheel 4종과 Hardpoint `Base_Standard` 에셋/프리뷰 경로는 관리형 Automation 6/6 PASS로 검증됐다. 다만 사용자가 Socket Manager 안에서 각 `PreviewStaticMesh` 필드 값까지 개별 확인했다고 별도로 주장하거나, 기존 Builder Step 3 복귀 UX를 이번 재로드 확인에서 테스트했다고 가정하지 않는다.

**현재 Gate:** Coupe **핵심 소켓 저장·재로드 USER PASS**, Socket Manager 목록 즉시 갱신 **Deferred / accepted non-blocking**. 기존 Step 3로 복귀하는 실제 USER 조작은 **별도 직접 확인 미보고**(기존 `QuickModeBoundary` Automation PASS는 유지). 따라서 `VSPUX-P0-04` 전체 USER Acceptance를 이번 확인만으로 완료했다고 선언하지 않고, 현행 계획의 마지막 UI 경계 확인을 남긴다. `VSPUX-P0-05` Closure는 아직 시작하지 않는다. 동일 저장·재로드 검증을 무의미하게 반복시키지 않는다.

**변경 범위:** 소켓 생성 C++, UE 5.8 Engine Source, Coupe 또는 타 차량 .uasset, Recipe/VehicleData/기존 사용자 Dirty는 수정하지 않는다. 이번 턴은 보고를 문서·UDS에 반영하는 작업이며 빌드·Automation 재실행, Git stage/commit/push 없음.

## 5. Gate와 검증

| Gate | 범위 및 판정 기준 | 상태 |
| --- | --- | --- |
| `VSPUX-P0-00` | Fresh Rebaseline, A/B 검토, 보호 조건, 공식 Work 등록 | PASS / Historical |
| `VSPUX-P0-01` | Pre-Implementation Review + USER Category 의미 정정 + Layout-resilient 설계 + P1-01 목록 Refresh callback 격리/P1-02 두 UI host 분리/P2-01 현재 범위 계약 교정·재검수 | PASS / Historical |
| `VSPUX-P0-02` | §4.9 기준 Quick/일반 host 분리, programmatic Browser refresh callback 보호, Tab-local Quick state 및 공통 Socket Handler 최소 Slate prototype 구현 | PASS / Retained checkpoint |
| `VSPUX-P0-03` | §4.11 범위: Quick Wheel 3상태 표시 교정, 실제 Tab mode/host/refresh/regression Automation, Git diff/readback, 공식 UE 5.8 Source Editor Build·5건 focused Automation | FINAL TECHNICAL REVIEW PASS / Previous checkpoint |
| `VSPUX-P0-04` | §4.14 Hardpoint Base_Standard Preview, §4.15 관리형 Automation 고유 6/6 PASS, §4.16 Socket Manager 실시간 목록 Deferred/non-blocking 수락, §4.17 USER Coupe 작업 후 다시 로드해 소켓 전체 부착 정상 확인 PASS. 기존 Builder Step 3 복귀는 Automation PASS이나 USER 직접 UI 확인 미보고 | **TECHNICAL PASS / COUPE SAVE-RELOAD USER PASS / DISPLAY DEFERRED / STEP 3 UX CONFIRMATION PENDING** |
| `VSPUX-P0-05` | Final Audit + 실제 구현 Systems 승격 + UDS closure | Not Started |

필수 focused/affected 시나리오:

1. Recipe/VehicleData가 없는 선택 메시 및 직접 Picker 메시에서 Wheel exact4 이름·live 상태·생성 동작.
2. 같은 소켓 재추가 시 중복0, 기존 Transform 변경0.
3. `HP_Top_01`, `HP_Top_03`이 이미 있을 때 다음 `HP_Top_04`, 이후 `HP_Top_05`; 범주별 번호 독립, 잘못된 이름과 충돌 없음.
4. Quick 모드에서 Recipe/VehicleData/LocationSlot/MountProfile 자동 쓰기0.
5. 후보 목록에 없는 메시 및 이미 다른 차량이 사용하는 공유 메시도 직접 Picker로 선택 가능.
6. managed Recipe custom Wheel Binding/Hardpoint 등록, Step 3 Locked/Complete, 기존 Step 1~8, 신규 차량 생성 회귀.
7. Editor 열기 성공, 수동 Transform/Save, 자동 Save0, 저장 후 실제 소켓 상태 재확인.

기술 PASS는 USER의 실제 화면·저장 동선 수락을 대체하지 않는다. 자동 테스트에서 제품 StaticMesh를 영구 변경하지 않도록 Fixture 또는 임시 자산을 사용한다.

## 6. Changelog / Migration

### v0.1.13 — 2026-10-15

- USER가 Coupe Static Mesh를 다시 로드한 뒤 작업 소켓 전부 정상 부착 상태를 직접 확인했다. 사용자 저장·재로드 결과 PASS로 §4.17에 기록; 관리형 독립 에셋 디스크 readback을 새로 수행했다고 주장하지 않는다.
- Socket Manager 실시간 목록 갱신 Deferred/non-blocking 유지. 새 코드·UE 엔진·에셋 수정 없음, 기존 focused Automation 6/6 PASS 유지.
- 기존 Builder Step 3 복귀는 Automation으로 검증됐지만 USER 직접 재확인 보고는 없으므로 P0-04 전체 최종 수락/P0-05 Closure를 과잉 선언하지 않는다. Git stage/commit/push 없음.

### v0.1.12 — 2026-10-14

- USER가 Coupe 메시 소켓 준비 완료를 보고했지만 열린 Static Mesh Editor의 Socket Manager 목록은 FL만 남고 탭 재열기 후 정상 표시되는 UI 갱신 결함 재현. Compact에서도 동일 증상. 소켓/프리뷰는 뷰포트에 정상 표시됨.
- 비용 대비 효과가 낮아 Socket Manager 실시간 목록 갱신을 Deferred/non-blocking USER 수용 제약으로 확정. 엔진/비공개 API/별도 Socket Manager 수정 금지, 새 C++ 구현 없음. 미저장 편집 보호 후 Editor 탭 닫기·재열기를 수동 우회 절차로 채택.
- P0-04 남은 필수 사용자 수락에 Coupe 프리뷰·Transform·명시 Save/재로드·Step 3 복귀를 보존하고 Socket Manager live-list 즉시 표시 항목만 gate에서 제외. §4.15 USER Build PASS/Automation 6/6 PASS 유지, P0-05 미착수. Git stage/commit/push 없음.

### v0.1.11 — 2026-10-14

- USER가 Hardpoint Preview 변경 포함 UE 5.8 직접 빌드 성공을 보고했다. 빌드 결과는 USER 확인으로 구분하며 별도 build job/log 직접 검증을 주장하지 않는다.
- 관리형 Editor Automation에서 신규 Base_Standard 포함 SocketPreviewMapping 1/1 PASS, CF-FQ-060 3/3 PASS 및 직접 영향 BuilderShell/Step3HardpointPlanning/InventoryClassification 각 1/1 PASS, 고유 테스트 6/6 PASS(엔진 exit0, 실패·누락 0). §4.15에 검증 job 증거와 범위를 기록했다.
- User Compact 기존 소켓 프리뷰 소급적용/자동 저장 없음. 실제 열린 Socket Manager/USER Save 검수는 P0-04 대기, P0-05 진행 안 함. Git stage/commit/push 없음.

### v0.1.10 — 2026-10-14

- USER 요청으로 모든 신규 canonical Hardpoint Socket에 동일 `Base_Standard` PreviewStaticMesh를 적용한다. 현재 7종 및 미래 지원 category는 기존 naming parser/동적 목록을 공통으로 사용하며 Wheel FL/FR/RL/RR 프리뷰는 유지했다.
- `CFVehicleBuilderTab.h/.cpp` 프리뷰 분류·생성 경로와 `CFVehicleAuthoringVMTests.cpp` 실제 메시 로드/7+1 category/비지원·비canonical/Transient AddSocket 검증을 추가했다. Git에 `Base_Standard.uasset` 추적 파일 존재 확인. 기존 Asset/Socket/Transform·Recipe/VehicleData 변경 또는 자동 Save 없음.
- Source 적용 후 UE Editor 실행 중임을 확인했고, DLL 잠금을 유발할 빌드/테스트를 중복 실행하지 않았다. 이번 신규 변경의 공식 UE 5.8 Build·Automation NOT RUN; 이전 §4.13 6/6 PASS는 과거 증거. P0-04 USER Acceptance 미완료, Git stage/commit/push 없음.

### v0.1.9 — 2026-10-14

- 사용자가 CarFight Editor를 종료한 뒤 이전 DLL 잠금이 해제되어 공식 UE 5.8 Source Build 재실행 `6aa1d40cfbb144d181a989abe908bbe1` PASS(exit0). NetCore/CarFight_ReEditor DLL 링크 성공.
- 새로운 Wheel `SocketPreviewMapping`을 포함한 CF-FQ-060 3건, 기존 BuilderShell/Step3HardpointPlanning/InventoryClassification 3건, 집중 Automation 총 6/6 PASS; 실패·누락·예상 외·중복 0.
- §4.12의 당시 BLOCKED는 Historical 이력으로 보존하고 §4.13을 현재 검증 기준으로 명시. 실제 Compact Editor live Socket Manager 표시·수동 Save·USER Acceptance는 아직 수행하지 않아 P0-04 종료나 P0-05 완료로 판정하지 않는다.
- 원격 Editor 종료는 MCP의 exact runtime_instance selector 차단으로 수행되지 않았고 사용자 직접 종료로 빌드 블로커 해소. 원격 lifecycle 사용성 이슈는 별도 다룬다. User Dirty/기타 프로젝트 보호, Git stage/commit/push 없음.

### v0.1.8 — 2026-10-14

- USER Compact 피드백에 따라 새 Wheel Socket 역할별 PreviewStaticMesh 매핑을 추가하고, 공통 Socket 배열 직접 삽입을 UStaticMesh::AddSocket API와 readback으로 교정했다. 열린 동일 StaticMesh Editor에만 RefreshTool/SetSelectedSocket을 호출한다.
- `StaticMeshEditor` Editor-only 모듈 직접 의존성을 추가하고 transient Wheel 프리뷰 4개 Asset load/AddSocket 회귀 테스트를 작성했다. 기존 Socket/Recipe/VehicleData 변경·제품 Asset 자동 Save 없음.
- UE 5.8 공식 Build job 6c8e81af4a624fb09b35b7a31c065061의 소스 컴파일은 진행됐으나 실행 중 UnrealEditor.exe에 의한 NetCore/CarFight_ReEditor DLL 잠금 때문에 LNK1104 / exit6 BLOCKED. 최신 Automation NOT RUN, 사용자 실제 편집 수락 NOT RUN. 이전 v0.1.7 5/5는 현재 교정 성공 증거로 사용하지 않는다.
- User-owned Compact 포함 기존 Dirty 보호, 강제 종료·stage/commit/push 없음. 정상 Editor 종료 후 공식 빌드/테스트 재검증 대기.

### v0.1.7 — 2026-10-14

- VSPUX-P0-03 Final Technical Review PASS. Quick Wheel 미선택 메시 상태를 '없음'으로 오표시하던 문제를 live getter로 교정했고 실제 Tab host/mode/selection/refresh 동작을 확인하는 QuickModeBoundary Automation을 추가했다.
- 공식 UE 5.8 Source Editor Build exit0, Quick focused 2건 + BuilderShell/Step3HardpointPlanning/InventoryClassification 총 5건 PASS, 실패 0건. 변경 Source 6개 diff/readback, Quick와 Recipe Step3 별도 owner, Socket transaction 공유 및 자동 Save/Recipe mutation 신규 경로0을 검수했다.
- Migration: UI read-only 상태 getter와 WITH_DEV_AUTOMATION_TESTS 테스트 접근만 추가, Product schema/Recipe/Target/Runtime 변경·제품 Asset 저장 없음. 레이아웃은 여전히 시제품이며 USER 직접 편집·Save·시각 수락은 P0-04에서 수행한다.
- 다른 작업 Dirty 보호, Git stage/commit/push 없음.

### v0.1.6 — 2026-10-14

- CF-FQ-060 VSPUX-P0-02 독립 Quick Slate UI/상태와 기존 Step3 host 분리, 실제 Socket Transaction 1개 재사용, dynamic Hardpoint 위치 Category/name/overflow guard 및 Wheel click 동적 역할 이름 교정을 구현했다.
- Browser 목록 refresh와 사용자 직접 선택 콜백을 분리하고 기존 managed selection/Step/Chassis 및 Hardpoint inventory 회귀를 Automation으로 검증했다. 공식 UE 5.8 Source Editor Build exit0, focused Automation 3/3 PASS. 최초 빌드 include 오류는 수정 후 정상 재빌드했다.
- Migration: 기존 Recipe/VehicleData/Step1~8/Category store/Runtime Schema 이전 불필요. Quick은 임시 Tab 상태로 기존 StaticMeshSocket만 USER 명시 추가하며 자동 저장하지 않는다. 초기 레이아웃은 P0-04 USER 검수 전 시제품으로 유지한다.
- 최종 사용자 화면/수동 Socket edit·Save는 아직 NOT RUN. 다른 Dirty 보호, stage/commit/push 없음.

### v0.1.5 — 2026-10-14

- P1-01: `HandleRefreshVehicles()`의 programmatic `SetSelection(..., Direct)`와 사용자 직접 선택을 분리하고, `ViewModel->RefreshVehicles()` 내부 상태 변경까지 사전·사후 확인하는 구현/검증 계약을 추가했다.
- P1-02: Quick 전용 UI host와 기존 Step 3 `HardpointPlanningHost`의 화면 소유권을 분리했다. 공통 소켓 검사/Transaction 기능만 재사용하며 Quick Refresh/Step Refresh를 구분한다.
- P2-01: §4.9를 P0-02 필수 UI/동작 범위의 단일 최신 계약으로 추가하고 §4.5/§4.6의 오래된 시안 및 좌표 중심 확장 요구를 Historical로 한정했다. §4.7 category, §4.8 layout 내구성은 유지한다.
- Design Re-review: 문서·실제 `SCFVehicleBuilderTab`, `FCFVehicleBuilderVM` 경로 대조 기준 계약 PASS. 코드는 미구현, 공식 Build/Automation/UE 실제 화면/USER 수락은 모두 Not Run. 기존 Step1~8/Recipe·다른 Dirty 보호.

### v0.1.4 — 2026-10-14

- USER 요청으로 초기 화면 배치를 확정하지 않고 실제 UE Editor 구현 후 USER 검수에서 조정하도록 UX acceptance 순서를 확정했다. Slate UI section/Tab-local state/stable role·category handler/기존 StaticMesh mutation 책임을 분리하여 배치 변경 시 기능 수정이 필요 없도록 한다.
- Render-time 캡처된 SocketName이나 특정 Widget 주소·slot 순서에 의존하지 않으며 클릭 시 live Mesh/role/category/후보를 다시 검증한다. 기존 Step 3 Wheel handler 정합성 P1 교정을 직접 영향 범위에 유지한다.
- 자동 helper 검증과 실제 Slate 버튼/화면의 USER smoke를 구분하고, 레이아웃 수정 시 UI-only diff·컴파일·focused 회귀·화면 동선 점검을 검증 기준으로 추가했다. Source/UE Asset/Build/Automation/PIE/USER 테스트는 아직 Not Run.

### v0.1.3 — 2026-10-14

- USER가 '소켓 위치'는 좌표가 아닌 Top/Front/LeftSide 같은 **Hardpoint category**이며 향후 신규 category가 늘 수 있음을 명시적으로 교정했다. §4.7을 최신 계약으로 추가하고 §4.6의 좌표 중심 해석을 Historical로 구분했다.
- 현재 7개 위치 분류를 영구 고정 목록이 아닌 현재 supported set으로 정의하며 VM category source → Quick 선택 UI → canonical HP_<Category>_<NN> 이름 제안/파서 및 직접 consumers의 의미 일치가 필수임을 명시했다. 추가 category가 자동으로 런타임 게임 의미를 획득하지는 않는다.
- 비요구 범위(좌표 read-only 패널, 임의 Transform UI, 불필요한 분류 Registry/Config, 검색·필터 확장)를 필수 작업에서 배제했다. Source/UE Asset/Build/Automation/USER 테스트는 Not Run.

### v0.1.2 — 2026-10-14

- USER가 Socket의 물리적 위치/향후 종류 확장 가능성을 요구하여 `물리 Transform / 이름 / 게임 Semantic Binding`을 분리했다. 전체 인벤토리, 기타/미인식 소켓의 표시·편집 보존, 동적 Standard category 선택, 다수 위치·번호·이름의 독립성, 미래 소비자 계약 경계를 확정했다.
- 2026-10-13 Design Re-review에서 확인된 P1 3건(기존 Hardpoint 편집, Quick 모드 복귀, Recipe-free Quick 새로고침)의 동작을 §4.6에 정확하게 교정하고 검수했다. 기존 Wheel button construct-time name capture 교정도 P0-02 필수 유지.
- Migration: 기존 Hardpoint category 7종과 4 Wheel role의 제품 계약/저장 구조는 변경하지 않는다. 아직 신규 Runtime/Asset/Build/Automation/PIE 작업을 수행하지 않았다.

### v0.1.1 — 2026-10-13

- VSPUX-P0-01 Fresh Source 기반 Pre-Implementation Contract Review PASS. Quick Tab 상태/메시 소유권/화면 격리, Wheel 동적 이름 선택 전환, HP max+1 및 collision, live mesh error, Recipe-free Save0 계약과 정확한 수정 파일/검증을 확정했다.
- 기존 Construct-time Wheel add handler의 캡처된 SocketName과 동적 Text/Copy/Status가 다른 이름을 가리킬 수 있다는 직접 영향 P1을 확인했다. P0-02에서 바퀴 행 동적 이름 조회를 함께 교정하며 현재는 미구현이다.
- Migration: 새 Backend/VM/Recipe schema 없이 Tab-local 화면 모드+기존 HardpointIntegrity helper만 추가하는 방향으로 제한했다. 기존 Step 3/UDS/CCAS migration 없음; Source/Asset/Build/Automation/PIE 모두 Not Run.

### v0.1.0 — 2026-10-13

- USER의 정식 승격 지시로 소켓 이름 관리 부담을 해결하는 독립 `CF-FQ-060`을 생성.
- A안(기존 Builder 내 독립 Quick Socket Prep)과 필수 Wheel/Hardpoint naming, StaticMesh direct selection, live readback, 사용자 Save 책임, 기존 Recipe state 보호를 문서화.
- Migration: 기존 Builder 8단계/Recipe/VehicleData/CCAS는 그대로 유지한다. Quick 모드의 추가 동작만 구현 대상으로 삼으며 이번 문서 승격은 코드·에셋 마이그레이션이 없다.
