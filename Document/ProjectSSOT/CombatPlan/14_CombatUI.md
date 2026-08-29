# 14. Combat UI

- 문서 버전: v0.12.0
- 작성일: 2026-06-08
- 최근 갱신일: 2026-08-12
- 문서 상태: Current
- 담당 범위: CarFight 인게임 전투 UI의 장기 정보 구조와 표현 원칙

---

## 1. 목적

이 문서는 CarFight의 인게임 전투 UI가 어떤 정보를 어떤 우선순위와 책임 경계로 보여줘야 하는지 정의한다.

현재 구현 범위는 인게임 전투 UI로 제한한다. 다만 인게임 HUD를 차량 Pawn이 임시로 생성하는 고립된 화면으로 끝내지 않고, 향후 타이틀, 차고, 피팅, 임무 선택, 로딩과 결과 화면을 같은 UI 프레임워크에 추가할 수 있도록 확장 가능성을 유지한다.

```text
현재 구현 범위
= 인게임 HUD + 인게임 메뉴 + 전투 정보 표시

장기 확장 가능 범위
= 타이틀 + 메인 메뉴 + 차고 + 피팅 + 임무 선택 + 로딩 + 결과 화면
```

이 문서는 장기 전투 UI의 SSOT다. CF-FQ-032 완료 당시 구현 순서와 검증 evidence는 `Document/Plan/Archive/InGameUIPlan.md`에 보존돼 있고, 현재 구현은 `Document/Systems/UI/InGameUI.md`와 실제 Source/Asset을 우선한다. 아직 구현되지 않은 항목을 Current System으로 해석하지 않는다.

---

## 2. 현재 확정된 전제

| 항목 | 확정 기준 |
|---|---|
| 현재 플레이 기준 | 싱글플레이 로컬 차량 전투 |
| 기본 카메라 | 외부 3인칭 차량 카메라 |
| 보조 카메라 | 운전석 시점 없음, 포탑 시점은 후속 지원 가능 |
| 탐지 표시 | 지형 미니맵이 아닌 센서 기반 전투 레이더 |
| 차량 방어 표시 | 쉴드, 장갑, 차량 내구도를 분리 표시 |
| 부품 손상 | 쉴드 단계 없음, 장갑 단계 제한적, 차량 내구도 단계 본격 적용 |
| 속도계 | 좌하단 VehiclePanel의 비대칭 실제 Engine RPM Gauge + 큰 3자리 디지털 속도 + 작은 km/h + 단일 Gear Slot |
| 타겟 패널 | 선택 대상이 있는 동안 항상 유지 |
| 미확인 정보 | 필드를 숨기지 않고 `???` 등 명시적 미확인 상태로 표시 |
| 정보 획득 | 가시 식별과 센서 스캔을 별도 정보 경로로 취급 |
| 락온 | 선택과 별도 상태. 현재 별도 Lock Runtime은 없으며 후속 Fire-Control Provider가 실제 상태를 제공할 때만 표시 |
| 시각 방향 | 차량 탑재형 전술 인터페이스, 스타일라이즈드 평면 UI, Cyan 중심 Accent와 C형 70:20:10 균형 |
| HUD 기본 배치 | 16:9 외부 3인칭 TPS 기준: Mission TL, Alert TC, Target TR, Vehicle BL, Radar BC, Weapon BR |
| 오디오 | 프로젝트 전역 정책에 따라 게임 및 UI 사운드를 구현하지 않음 |

---

## 3. 핵심 원칙

### 3.1 UI는 게임플레이 결과를 표시한다

UI는 아래 판정을 직접 수행하지 않는다.

- 피해 계산
- 쉴드, 장갑과 차량 내구도 간 피해 분배
- 부품 손상 판정
- 타겟 후보 정렬
- 가시 식별과 스캔 정보 공개 판정
- 락온 진행과 상실 판정
- 탄약, 배터리, 내부 충전과 열 소비 계산
- 레이더 접촉 생성

권장 데이터 흐름:

```text
게임플레이 시스템
→ 표시 가능한 상태 데이터 생성
→ UI용 View Data 또는 Presenter
→ Widget 표시
```

### 3.2 선택, 식별과 락온을 분리한다

```text
선택 대상
= 플레이어가 지속적으로 확인하고 장비 기준으로 사용할 대상

식별 지식
= 해당 대상에 대해 현재 공개된 정보

장비 락온
= 특정 무기 또는 장비가 사용 조건을 획득한 상태
```

세 상태는 서로 영향을 줄 수 있지만 같은 변수나 같은 Widget 상태로 합치지 않는다.

### 3.3 알 수 없음도 정보다

선택된 타겟의 정보가 없다는 이유로 패널이나 행을 제거하지 않는다.

```text
NAME      ???
TYPE      ???
FACTION   ???
DISTANCE  842 m
SHIELD    ???
ARMOR     ???
VEHICLE   ???
```

이 방식은 레이아웃을 안정적으로 유지하고, 가시 식별과 스캔을 통해 어떤 정보가 새로 공개되었는지 분명하게 보여준다.

### 3.4 전투 중 필요한 정보만 우선한다

전투 HUD는 모든 시스템 값을 한 번에 노출하지 않는다.

- 즉시 판단 정보는 상시 표시한다.
- 상태 변화 정보는 경고 또는 순간 확장으로 표시한다.
- 피팅 수치와 상세 계산은 후속 피팅 화면에서 표시한다.
- 디버그 수치는 Debug Layer로 분리한다.

### 3.5 색상만으로 의미를 전달하지 않는다

상태는 최소 두 가지 이상의 표현을 함께 사용한다.

```text
색상 + 형태
색상 + 텍스트
아이콘 + 게이지
형태 + 점멸 패턴
```

프로젝트 전역에서 사운드를 구현하지 않으므로 중요한 경고도 시각 정보만으로 완전하게 이해할 수 있어야 한다.

### 3.6 승인된 시각 콘셉트

공식 시각 방향은 `차량 탑재형 전술 인터페이스`다.

```text
70% 실용적 전투 정보
20% 차량 디지털 계기판 정체성
10% 세계관 장식과 스타일
```

- 군용 장비의 명확성, 차량 계기판의 정돈된 정보 구조와 스타일라이즈드 카툰 렌더링에 어울리는 평면 그래픽을 결합한다.
- 기본 Accent는 Cyan이며 주의는 Amber, 위험·치명 상태는 Orange Red·Red 계열을 사용한다.
- 조준·락온·Radar는 국소적으로 전술성을 강화할 수 있지만 전체 HUD를 항공기 MFD나 군용 HUD 복제로 만들지 않는다.
- 과도한 네온, 지속 Glitch·Scanline, 의미 없는 원형 장식, 강한 Glass·Blur와 작은 군용 약어 중심 표현을 기본 스타일로 사용하지 않는다.
- 시각 방향의 상세 규격과 제작 체크포인트는 `Document/Plan/InGameUIVisualConcept.md`가 소유한다.
- 이 방향은 Accepted지만 Style Data·Base Widget·Font·Icon·Wireframe과 해상도별 검토가 끝나기 전에는 UI-DESIGN-GATE 전체 PASS나 Current System으로 해석하지 않는다.

### 3.7 승인된 HUD 기본 배치

기본 배치는 외부 3인칭 차량 TPS와 16:9 화면을 기준으로 한다.

```text
좌상단      Mission Summary
상단 중앙   Alert Feed
우상단      Target Panel
좌하단      Vehicle Panel
하단 중앙   Radar Panel
우하단      Weapon Panel
중앙        Reticle·Target Marker 보호영역
```

- 중앙은 플레이 차량, 주행 경로, 조준과 타겟 관찰을 위해 비워 둔다.
- Radar는 하단 중앙에 배치하고 Elite Dangerous의 공간 인지 방식을 참고하되 우주선식 자원 UI를 복제하지 않는다.
- Vehicle Panel Visual Layout은 Accepted다. 896×416 가로형으로 좌측 비대칭 실제 Engine RPM Gauge·큰 3자리 디지털 속도·단일 Gear Slot, 우측 왼쪽 전방 Armor 실루엣, 중간 Shield Bar와 최하단 Integrity Bar를 사용한다.
- Armor 상태창은 방향 텍스트를 제거하고 화면 위치를 좌=Front, 상=Right, 우=Rear, 하=Left로 바인딩한다. 상부 Badge는 좌상단, 하부 Badge는 우하단에 둔다.
- 부품 손상 목록은 고정 VehiclePanel에서 제외하며 후속 Alert·Compact Chip·상세 Panel 중 하나로 별도 설계한다.
- Weapon Panel은 선택 무기의 Primary Resource와 Fire State를 강조하고 미지원 Resource Channel과 비선택 Rail을 Collapse하는 Compact 기본 Preset을 사용한다. 실제 Runtime View Data 연결은 후속 UI-P0-03이 소유한다.
- Target Panel은 차량 이미지를 기본 사용하지 않고 `Tracking / Identification / Target Intelligence`를 분리해 실제 확보된 정보만 공개한다. Last Known, Contact Lost와 Destroyed 선택 수명도 Sensor/Gameplay Provider 결과를 따른다.
- P0는 기본 배치를 고정 사용한다. 후속 사용자 배치 커스터마이징을 위해 HUD 모듈은 안정적인 Slot ID와 Layout Profile로 확장 가능해야 한다.
- 이 결정은 위치 배치 Accepted이며 Vehicle·Weapon 내부 디자인, Style Asset과 해상도별 실제 검증 완료를 의미하지 않는다.

---

## 4. 전체 UI 프레임워크 방향

인게임 HUD는 향후 전체 게임플로우를 수용할 공통 루트 안의 하나의 화면으로 설계한다.

```text
UI Root
├─ Game Layer
├─ HUD Layer
├─ Screen Layer
├─ Panel Layer
├─ Menu Layer
├─ Modal Layer
├─ System Layer
└─ Debug Layer
```

| 레이어 | 책임 |
|---|---|
| Game | 월드 위치 기반 타겟·임무·접촉 마커 |
| HUD | 조준, 차량, 무기, 타겟, 레이더 등 상시 전투 정보 |
| Screen | 타이틀, 차고, 피팅, 임무 선택과 결과 같은 주요 전체 화면 |
| Panel | 현재 Screen 또는 InGame 위에 추가되는 상세 정보창 |
| Menu | 일시정지 메뉴와 인게임 설정 |
| Modal | 재시작·종료 등 확인이 필요한 팝업 |
| System | 입력 장치, 저장 상태와 오류 등 시스템 알림 |
| Debug | 개발용 진단 정보 |

현재는 인게임 관련 레이어만 구현한다. 타이틀과 차고 등 전체 게임플로우 화면은 후속 기능으로 남긴다.

### 4.1 구현 소유권과 수명 계약

```text
ACFPlayerController
→ Pawn 수명과 독립된 Pause·UI 공통 입력
→ System / Gameplay / UI Mapping Context 수명
→ Possessed Pawn 변경 통지

UCFUISubsystem
→ LocalPlayer별 UI 상태
→ 현재 World용 UI Root 생성·제거
→ HUD / Screen / Panel / Menu / Modal / Debug 레이어 관리

Presenter / Provider
→ 게임플레이 상태를 플레이어에게 공개 가능한 View Data로 변환

Widget
→ 전달된 View Data 표시
```

추가 확정 원칙:

- 현재 P0는 `UMG + Enhanced Input` 기반으로 구현하고 CommonUI는 후속 전체 게임플로우 단계에서 재검토한다.
- Gameplay Pawn의 기존 Widget 직접 생성은 `LegacyPawnOwned → UISubsystemOwned` 전환 게이트와 회귀 검증을 거쳐 단계적으로 제거한다.
- Subsystem과 Presenter의 Pawn·Actor 참조는 약한 참조를 기본으로 한다.
- `UCFUISubsystem`은 LocalPlayer 수명을 가지지만 실제 UI Root Widget은 World마다 다시 생성한다.
- World 종료 시 이전 Root, PlayerController, Pawn과 Component 이벤트 구독을 모두 해제한다.
- 게임플레이 판정용 Target Data와 플레이어 공개용 Target Knowledge View를 분리한다.
- `Unknown`, `Unavailable`과 실제 0을 명시적으로 구분한다.
- 현재 `UCFVehicleHealthComp` 값은 Vehicle Integrity로만 표시하고, 실제 Runtime이 없는 Shield와 Armor는 `Unavailable`로 표시한다.
- 의미 데이터는 이벤트 중심, 공간 투영은 프레임 중심, 속도·거리·Radar는 제한 주기로 갱신한다.
- 화면 밖 마커는 카메라 View Space 기준으로 방향을 계산한다.
- 시각 경고는 우선순위, 지속 시간과 `AlertKey` 기반 중복 제거를 가진다.

### 4.2 피팅 기능과의 경계

피팅은 전투 UI와 같은 UI Root를 사용할 수 있지만 `CF-FQ-032`가 피팅 기능을 소유하지 않는다.

```text
CF-FQ-032에서 공유
- Screen Layer와 전체 화면 전환 진입점
- 공통 UI 입력·Focus 정책
- 공통 UI Style
- ResourceType과 StatType 같은 표현 타입
- 차량·무기·장갑 Definition Data

별도 피팅 기능에서 소유
- Saved Loadout과 저장
- Fitting Draft
- Validate / Preview / Commit / Cancel
- 하드포인트·장비 호환성
- 중량·차량 성능·자원 적재 계산
- Fitting View Data와 Widget
- Garage Preview Actor
- 출격용 Runtime Snapshot
```

필수 원칙:

- `UCFUISubsystem`은 Loadout이나 피팅 Draft를 소유하지 않는다.
- InGame HUD View Data와 Fitting Preview View Data를 분리한다.
- 피팅 Widget은 Gameplay Pawn을 직접 수정하지 않는다.
- Garage Preview Actor와 Gameplay Pawn을 분리한다.
- P0에서는 전투 중 피팅을 허용하지 않는다.
- CommonUI 도입 여부는 실제 피팅·차고 기획 착수 시 다시 판단한다.
- 피팅 기능 ID와 별도 Plan은 피팅 기획을 시작할 때 부여한다.

---

## 5. 싱글플레이 완전 일시정지

### 5.1 정책

싱글플레이에서 일시정지 메뉴가 열리면 입력만 막는 것이 아니라 게임 월드의 모든 게임플레이 시뮬레이션을 멈춘다.

정지 대상:

- 플레이어와 AI 차량 입력 및 이동
- Chaos 차량 물리와 일반 게임플레이 물리
- AI 판단과 행동
- 무기 발사, 쿨다운과 재장전
- 발사체, 로켓과 미사일 비행
- 터렛 회전과 조준 안정화
- 타겟 탐색, 스캔과 락온 진행
- 쉴드 재생
- 배터리 충전과 소비 진행
- 무기 열 증가·냉각
- 상태 효과와 임무 타이머
- 게임플레이 애니메이션과 FX 시간 진행

정지 중 허용 대상:

- UI Root
- Pause Menu와 Modal
- 메뉴 입력과 포커스 이동
- 메뉴 전환 애니메이션
- 설정 조작

### 5.2 시간 기준

모든 게임플레이 진행 시간은 일시정지의 영향을 받는 게임 시간을 사용한다.

```text
게임플레이 시간
→ World/Game Time과 Pause 영향을 받는 Timer 사용

UI 시간
→ 일시정지 중에도 필요한 UI 애니메이션만 별도 갱신 허용
```

락온, 재장전, 냉각, 쉴드 재생이 실제 시간 기준으로 진행되어 일시정지 중 완료되는 동작을 금지한다.

### 5.3 장기 확장

향후 멀티플레이를 추가할 경우 한 플레이어 메뉴가 월드 전체를 멈추지 않는 별도 정책이 필요하다. 현재 구현은 `SinglePlayerFullPause`만 대상으로 한다.

---

## 6. 카메라별 HUD 표현

### 6.1 외부 3인칭

기본 플레이 화면이다.

필수 정보:

- 플레이어 조준 레티클
- 터렛 또는 무기 실제 조준 레티클
- 차량 진행 방향
- 포탑 방향
- 숫자형 속도
- 레이더
- 차량 방어 상태
- 무기와 자원 상태
- 선택 타겟 정보
- 화면 밖 접촉 방향

차체 전방, 카메라 전방, 포탑 전방과 총구 방향이 다를 수 있음을 전제로 한다.

### 6.2 포탑 시점

포탑 시점은 후속 지원 가능 상태로 둔다.

포탑 시점에서는 다음 정보를 강조한다.

- 정밀 조준 레티클
- 터렛 회전 제한
- 조준 정렬과 안정화
- 유효 거리
- 락온 진행
- 탄약, 충전과 열
- 차체 방향과 포탑 방향 차이

차량 속도와 방어 상태는 축소하되 숨기지 않는다.

운전석 시점은 현재 설계와 구현 범위에 포함하지 않는다.

---

## 7. 화면 배치 원칙

승인된 16:9 외부 3인칭 TPS 기본 배치:

```text
┌────────────────────────────────────────────────────────────┐
│ [임무 목표]              [중요 경고]       [선택 타겟]   │
│                                                            │
│                       [조준 영역]                           │
│                                                            │
│ [차량 상태]              [레이더]          [무기 상태]   │
└────────────────────────────────────────────────────────────┘
```

### 중앙

- 플레이어 Command Reticle
- `CurrentMuzzleDirection` 기반 Turret Reticle
- 선택 Target Marker와 후속 Lock Provider 상태
- 사거리와 조준 불가 상태
- 로컬 공격자의 실제 피해 적용 Hit Confirmation
- 내 차량의 Camera-relative Incoming Damage Direction
- Mission Provider가 명시한 Primary Objective의 World Navigation Marker

### Mission Summary / Objective Navigation

```text
MissionSummary
= Primary Objective의 Text·진행·Timer

Objective World Marker
= Primary Objective가 Navigation Anchor를 가질 때의 공간 방향
```

- 현재 실제 C++에는 Mission/Objective Runtime Provider가 없고 HUD Layout에는 `MissionSummary`와 Stretch `ReticleLayer` Slot만 존재한다. 따라서 실제 Gameplay에서는 Provider 연결 전 MissionSummary와 Objective World Marker를 모두 Collapse하며 Mock는 Visual Prototype 증거로만 사용한다.
- P0 World Navigation은 Mission Provider가 명시한 Primary Objective 1개만 표시한다. `+N OBJECTIVES`를 근거로 UI가 거리순·화면 위치순으로 Secondary Marker를 만들지 않는다.
- Objective World Marker는 Target Selection과 다른 Open Hex Beacon 계열을 사용한다. 기본 Objective 색은 `StateNotice`, Target 선택은 기존 `AccentTactical` 4-Corner Bracket 의미를 유지한다.
- Mission Provider가 정확한 위치를 공개하지 않은 Objective에서 UI가 TargetSelect, Sensor Contact나 Actor 검색으로 실제 Actor 위치를 찾아 Exact Marker를 만들지 않는다. 정확/근사/위치 없음은 Provider가 Player-facing Navigation 정보로 구분한다.
- MissionSummary Primary와 World Marker는 같은 안정적인 Primary Objective Identity를 사용해야 한다. Navigation Anchor가 없는 Objective는 Summary만 표시할 수 있다.
- Objective 거리 표시는 Provider/Presenter가 Player-facing 값으로 제공할 때만 사용하며 근사 위치에서 UI가 정확한 Meter 값을 만들어내지 않는다.
- Selected Target이 같은 Objective Entity이면 두 Full Marker를 중첩하지 않고 기존 Target Bracket에 작은 Objective Badge를 결합한다. 같은 Screen 위치라는 이유만으로 동일 Entity라고 추정하지 않는다.
- 서로 다른 Marker가 겹치면 `Objective < Selected Target < Command/Turret Reticle` 우선순위를 사용하고 Objective 보조 Text를 먼저 축약한다. Edge 충돌에서는 Selected Target의 Ray 교차 위치를 유지하고 Objective만 제한된 Tangential Shift로 간격을 확보한다.
- Objective World Marker가 존재한다는 이유로 Radar Contact를 생성하지 않는다. Radar Sensor Contact와 Mission Navigation은 별도 정보 채널이다.
- Objective 완료·실패·Primary 전환, Player Destroyed 이후 유지 여부와 Mission Result는 Mission/Player/Outcome Provider가 소유한다. Widget Timer·자동 다음 Objective 선택·게임/UI 사운드를 만들지 않는다.
- Objective는 별도 고정 HUD Slot을 요구하지 않고 Game Layer 의미 / 기존 Stretch ReticleLayer 하위 Projection 요소로 확장 가능하게 한다. 이 설계는 현재 D1-11 Production User Preview Gate를 변경하지 않는다.

### 중앙 Reticle Layer Global Declutter

중앙 HUD의 개별 요소는 각각 의미가 달라도 동시에 Full 표시하면 시야를 과도하게 점유할 수 있으므로 **Presentation-only Global Declutter**를 사용한다.

```text
Gameplay State
→ 보존

Projection / Presentation
→ 공간 충돌만 해결
```

- Declutter는 Target Selection, Lock, Mission Primary, Objective Lifecycle, Damage Result, Knowledge와 Fire 가능 상태를 변경하지 않는다.
- P0 중앙 의미 집합은 Command 1, Turret 1, Selected Target 최대 1, Primary Objective 최대 1, Target 부착 Lock 최대 1, Immediate Fire State 1, Hit Marker State 1, Incoming Direction 최대 4 Bucket으로 제한한다.
- Target Candidate·Radar Contact·Secondary Objective·Multi-lock·Floating Damage Number를 중앙 World Marker로 자동 증식시키지 않는다.
- 중앙 보호 기준은 고정 Viewport 중심이 아니라 **현재 Command Reticle Screen Position**이다. 기본 Aim Core 반경 56, Combat Ring 반경 136을 사용한다.
- Aim Core에서는 자유로운 보조 Text를 두지 않고 Command/Turret/Hit 핵심 Geometry와 실제 같은 위치의 Selected Target·Lock Open Geometry만 허용한다. Objective는 ShapeOnly가 기본이다.
- Command Reticle 아래 `144×24` Status Lane은 Immediate Fire State 최대 1줄 전용이며 Target Distance, Objective Distance와 Lock Text를 같은 위치에 쌓지 않는다.
- Selected Target 주변 Auxiliary Text도 최대 1줄로 제한하고 `Lock Failure/Blocked → Lock Progress/Locked → Target State → Distance` 순으로 우선한다.
- Primary Objective는 Target보다 낮은 중앙 시각 우선순위를 가지며 충돌 시 Distance부터 제거하고 `Full → Compact → ShapeOnly` 순으로 축약한다.
- 같은 Target Entity인 Objective는 두 Full Marker 대신 기존 Target Bracket + 작은 Objective Badge를 사용한다.
- Hit Confirmation과 Incoming Damage는 높은 Draw Order의 짧은 **Transient Overlay**이며 발생할 때마다 Persistent Target/Objective Marker를 이동시키거나 Full/Compact 상태를 Reflow하지 않는다.
- Incoming Arc 최대 4개는 방향 정보를 유지하되 `TOP/BOTTOM` Micro Label은 동시에 최대 1개만 표시한다.
- Persistent Marker 충돌은 `Provider 검증 → Projection → Stable Identity Merge → Safe Region → Target 고정 → Objective 제한 이동 → Text Budget → Transient Overlay`의 결정적 순서로 처리한다.
- Edge Band는 Selected Target 최대 1 + Primary Objective 최대 1을 기준으로 하고 Target 위치를 고정한 채 Objective만 기본 40 간격·최대 56 Tangential Shift를 허용한다.
- 충돌 진입/해제 기본 Padding은 8/16으로 달리해 경계 떨림을 막고, 보조 정보 전환은 약 0.10초 Opacity Fade만 허용한다. Scale Pop·위치 Spring·Sound는 사용하지 않는다.
- UI-P0-03 이후 공통 Projection/Presentation Resolver가 한 프레임의 Screen Bounds와 Safe Region을 해결하고 각 Widget은 다른 Widget, Pawn, Mission Actor와 Damage Component를 Tick에서 직접 조회하지 않는다.
- 현재 Mission Provider와 Lock Provider 부재는 그대로 유지하며 Declutter를 구현하기 위해 가짜 Runtime을 만들지 않는다.
- 이 통합 설계는 D1-11 New Production 1920×1080 User Preview Pending / D1-11 NOT PASS 상태와 Production Asset을 변경하지 않는다.

### 좌측 하단 VehiclePanel

```text
896×416 가로형
좌측: 비대칭 실제 Engine RPM Gauge + 큰 3자리 디지털 속도 + 작은 km/h + 단일 Gear Slot
우측: 왼쪽 전방 차량 실루엣의 6방향 Armor 상태창
중간: Shield 전체 폭 Bar
최하단: Vehicle Integrity 전체 폭 Bar
```

### 하단 중앙

- 센서 Contact 기반 Radar
- Heading Up
- 지형 미니맵 미사용

### 우측 하단

- 현재 선택 무기 강조
- 비선택 무기 축약
- 탄약, 충전, 열, 쿨다운과 재장전 중 필요한 채널

### 우측 상단

- 선택 타겟 정보
- 차량 이미지 미사용
- 식별과 스캔 진척에 따른 점진 정보 공개
- 락온 상태

---

## 8. 차량 방어 HUD

차량 방어는 다음 세 계층을 분리 표시한다.

```text
Shield
→ 6방향 Armor
→ Vehicle Integrity
```

### Shield

- 계기판·Armor Cluster와 Integrity 사이의 전체 폭 연속 Bar다.
- 현재/최대, 재생 대기·재생 중·소진·미장착을 구분한다.

### 6방향 Armor

Armor 차량 실루엣은 화면 왼쪽을 전방으로 고정한다.

```text
실루엣 왼쪽   = Front
실루엣 위     = Right
실루엣 오른쪽 = Rear
실루엣 아래   = Left
상부 Badge     = Armor 창 좌상단
하부 Badge     = Armor 창 우하단
```

- 전면·후면·좌측·우측 방향 텍스트와 각 Plate의 Current/Maximum 숫자는 기본 표시하지 않는다.
- 각 Plate/Badge 우측의 독립 세로 Bar로 해당 방향 `NormalizedArmor`를 표시한다.
- Top은 좌상단 Badge, Bottom은 우하단 Badge를 사용하며 6방향 의미를 평균 Armor 하나로 합치지 않는다.
- 차량 월드 회전이나 카메라 방향으로 HUD 실루엣을 회전하지 않는다.

### Vehicle Integrity

- 패널 최하단 전체 폭의 가장 굵은 Bar다.
- 현재/최대, Critical과 Destroyed 상태를 표시한다.

### 피격 Feedback 책임

```text
Incoming Damage Direction
→ 공격이 어느 Camera-relative 방향에서 왔는지

VehiclePanel
→ Shield / 실제 ArmorDirection / Integrity 중 어느 방어층이 감소했는지

Alert Feed
→ Shield Broken, Integrity Critical처럼 Provider가 즉시 대응 사건으로 승격한 경우만
```

- 실제 방향의 원본은 `DamageHitContext.IncomingDirection`, 실제 방어층 결과는 `VehicleDamageResult`가 소유한다.
- `FireSuccess`는 발사 승인 상태이며 Hit Confirmation이 아니다. Hit Confirmation은 로컬 공격의 `VehicleDamageResult.bAppliedToAnyLayer`가 실제 방어층 감소를 확인한 경우만 발생한다.
- Shield/Armor-only Hit도 정상 적중이므로 Integrity 호환 결과만으로 Hit Marker를 판정하지 않는다.
- 기본 HUD에는 Floating Damage Number를 사용하지 않는다.
- 일반 Hit마다 Alert Feed 메시지를 만들지 않는다.

### Player Integrity Critical / Destroyed / Result 경계

```text
Integrity Critical
= 아직 생존 중인 Player-facing 위험 상태

Player Vehicle Destroyed
= VehicleHealth가 차량 생존 종료를 확정한 상태

Defeat / Respawn / Spectate / Result
= 상위 Player·Mission·Session Flow가 결정하는 Outcome
```

- `Critical`, `Destroyed`, `Result`를 하나의 상태로 합치지 않는다.
- 현재 `VehicleHealthComp`의 Destroyed는 `bDestroyed`와 최초 `OnVehicleDestroyed` 사건을 소유하지만 차량 입력, Chaos 물리, 메쉬, Possession, Respawn과 Result Screen을 자동 처리하지 않는다.
- 현재 `LauncherComp`는 Owner Destroyed 시 진행 중 Ripple·Salvo를 취소하며 `CombatFxComp`는 Destroyed FX를 1회 재생한다.
- 반면 현재 새 Fire 입력과 Drive 입력에는 Health Destroyed를 공통 차단하는 Player Lifecycle 계약이 없으므로, HUD만 Reticle·Weapon을 숨겨 조작 불가처럼 보이는 구현을 금지한다.
- 실제 Terminal HUD 구현은 상위 Gameplay의 Combat/Vehicle Availability 상태와 같은 사건에서 전환해야 한다. 이 연결은 D1-11 Visual Preview가 아니라 후속 UI-P0-03 Runtime Integration 범위다.
- Destroyed Terminal에서 VehiclePanel은 `DESTROYED`와 실제 최종 방어 상태를 보여줄 수 있지만 Shield·Armor·속도·RPM·Gear를 파괴 사실만으로 임의 0으로 만들지 않는다.
- 실제 Combat Interaction이 종료되면 Reticle, Lock, Target Marker, Weapon과 Radar는 짧게 종료하고 VehiclePanel Terminal 상태를 Outcome 전환 전까지 유지할 수 있다.
- `INTEGRITY CRITICAL` Alert는 Destroyed에서 종료하며 기본 `VEHICLE DESTROYED` Alert를 별도로 중복하지 않는다.
- `VEHICLE DESTROYED`는 `DEFEAT`, `GAME OVER`, `MISSION FAILED`와 동의어가 아니다.
- 미래 Result Screen은 `Screen` Layer를 사용하되 Result Title·Action·World Pause 여부는 상위 Outcome Flow가 소유한다. `SetPrimaryScreenWidget` 자체를 Game Over/Pause 판정으로 사용하지 않는다.
- 현재 Camera 타입에 Destroyed/Spectate 상태는 존재하지만 Health Destroyed와 자동 연결돼 있지 않으므로 UI 문서가 현재 동작으로 주장하지 않는다.

### 부품 손상

- 고정 VehiclePanel 목록에서는 제외한다.
- 실제 부품 Runtime이 구현된 뒤 Alert, 인접 Compact Chip 또는 별도 차량 상세 Panel 중 하나로 설계한다.

정확한 피해 분배와 부품 손상 판정은 `08_DefenseArmor.md`와 현재 VehicleDefense Runtime이 소유한다.

---

## 9. RPM Gauge + 디지털 속도계

좌하단 VehiclePanel의 좌측 `SpeedGauge`는 자동차 디지털 계기판을 연상시키는 **비대칭 Engine RPM Gauge + 디지털 속도 숫자** 조합을 사용한다.

```text
좌측 세로 Gauge
→ 부드러운 곡선 전환
→ 상단 긴 수평 Gauge

+ 큰 3자리 속도 숫자
+ 작은 km/h 단위
+ 단일 Gear Slot
```

- Gauge는 SpeedKmh를 중복 표현하는 Speed Arc가 아니라 **실제 Engine RPM**을 표시한다.
- 실제 RPM Provider가 연결되지 않은 구현 단계에서는 속도 비율이나 임의 Timer로 가짜 RPM을 생성하지 않는다.
- Gauge의 시각 스케일은 차량마다 공통 문법을 사용하며 Red Zone 시작은 기본 시각 스케일 85%, Tick은 21개다. 실제 차량별 RPM 값과 변속 판단은 Gameplay Data가 소유한다.
- `RPM`, `TACHO`, `x1000` 절대 수치 Label은 기본 HUD에 표시하지 않는다.
- 큰 속도 숫자는 차량 속도만 표시한다.
- Gear Slot은 `R`, `N`, 실제 Provider가 제공하는 전진 기어 단수를 표시한다. Gear Data가 없으면 전진 단수를 추정 생성하지 않는다.
- Handbrake·Braking·Airborne·Drive Disabled는 실제 Provider 상태가 있을 때만 조건부 Compact 상태로 표시한다.

---

## 10. 레이더

레이더는 지형을 그리는 미니맵이 아니다.

```text
월드 Actor 직접 검색
≠ 레이더

센서가 만든 Contact 데이터
→ 레이더 표시
```

기본 표시 정보:

- 플레이어 차량 중심과 진행 방향
- 센서에 감지된 접촉
- 선택된 타겟
- 아군, 적대, 중립과 미식별 접촉
- 탐지 범위
- 상대 방향과 거리
- 필요한 경우 높이 차이
- 마지막 탐지 상태

기본 방위 표현은 차량 진행 방향을 위로 두는 `Heading Up` 방식을 우선 검토한다.

레이더 접촉이 존재해도 대상의 이름, 진영과 방어 상태가 모두 공개되는 것은 아니다. 레이더 위치 정보와 타겟 지식은 분리한다.

---

## 11. 타겟 정보와 식별

### 11.1 선택 타겟 패널

선택 대상이 있는 동안 패널은 항상 표시한다. 공개되지 않은 필드는 `???`로 유지한다.

선택 대상이 없을 때는 같은 영역에서 `NO TARGET`을 표시해 전체 레이아웃 이동을 막는다.

### 11.2 가시 식별

가시 식별은 외형을 직접 관측해 얻는 정보다.

공개 가능 예:

- 차량 또는 구조물 여부
- 대략적인 크기와 유형
- 외부 표식과 진영 추정
- 노출된 무장
- 외부 손상, 연기와 화재

거리, 화면상 크기, 시야, 관측 시간과 광학 성능에 영향을 받을 수 있다.

### 11.3 센서 스캔

센서 스캔은 기술적 분석으로 얻는 정보다.

공개 가능 예:

- 정확한 식별명과 진영
- 쉴드, 장갑과 차량 내구도
- 내부 부품 상태
- 무장 구성
- 전자 신호와 상태 이상
- 약점 정보

### 11.4 분야별 지식

하나의 전역 식별 단계만으로 모든 필드 공개를 결정하지 않는다.

```text
Identity Knowledge
Affiliation Knowledge
Defense Knowledge
Weapon Knowledge
Component Knowledge
Threat Knowledge
```

P0 구현은 단순 단계형으로 시작할 수 있지만, UI 데이터 계약은 분야별 공개 상태로 확장 가능해야 한다.

---

## 12. 선택과 락온 UI

타겟 선택과 장비 락온은 분리한다.

```text
Selected Target
= 정보 패널과 장비 평가의 기준

Locked Target
= 후속 Fire-Control Runtime이 실제 획득을 확정한 대상
```

현재 실제 소스에는 별도 Lock Runtime이 확인되지 않으므로 HUD가 `Selected`, Reticle 정렬, ID2 또는 Scanner Analysis를 근거로 Lock Progress를 생성하지 않는다.

후속 Lock/Fire-Control 기능은 Crosshair Hold, Target Cone, Sensor Track, Manual Designate, Laser Guide, No Lock Required 같은 획득 방식을 선택할 수 있다. 정확한 획득 방식과 조건은 해당 Gameplay 기능이 소유한다.

UI는 실제 Provider가 존재할 때만 다음 Player-facing 결과를 표시한다.

- Lock Supported 여부
- Lock State
- 실제 제공되는 경우 Lock Progress
- Failure Reason

Lock 완료는 Cooldown, Ammo, MuzzleBlocked, Turret 정렬 정책 같은 실제 Fire Validation을 우회하지 않는다.

---

## 13. 무기 자원 표시

탄약과 에너지를 하나의 게임플레이 자원으로 합치지 않는다.

서로 다른 자원 예:

- 탄창 탄약
- 차량 예비 탄약
- 차량 배터리
- 무기 내부 충전
- 무기 열
- 재사용 대기시간
- 재장전 진행

다만 HUD는 공통 표시 채널 구조를 사용한다.

```text
Resource Type
Current Value
Maximum Value
Normalized Value
Display Text
Resource State
```

예:

```text
AUTOCANNON
32 / 120
HEAT 18%

PULSE LASER
CHARGE 74%
HEAT 42%

GUIDED MISSILE
4 / 8
LOCK 67%
```

차량 배터리는 센서, 락온, 안정화와 능동 방어의 전장 전력이다. 에너지 무기가 차량 배터리를 직접 소비하는지는 해당 무기 설계에서 명시적으로 결정한다. 무기 내부 충전은 차량 배터리와 다른 자원일 수 있다.

---

## 14. 전투용 SF HUD 시각 원칙

- 얇고 명확한 선을 사용한다.
- 반투명 패널은 정보 그룹 구분에만 사용한다.
- 숫자와 기호의 판독성을 장식보다 우선한다.
- 상태 변화가 있을 때만 애니메이션을 사용한다.
- 조준 중앙 영역을 불필요한 장식으로 덮지 않는다.
- 의미 없는 회전선, 지속 노이즈와 과도한 왜곡을 사용하지 않는다.
- 쉴드, 장갑, 차량 내구도는 색상뿐 아니라 라벨과 형태로 구분한다.
- 락온 완료, 정보 공개, 부품 손상은 형태와 텍스트 변화로도 전달한다.

---

## 15. 해상도와 안전영역

핵심 전투 정보는 중앙 16:9 기준 안전영역 안에 유지한다.

중앙 안전영역:

- 조준 UI
- 타겟 패널
- 차량 핵심 상태
- 현재 무기
- 숫자형 속도
- 주요 경고

외곽 확장 가능 영역:

- 상세 로그
- 디버그 패널
- 비필수 임무 정보
- 후속 보조 센서

검증 해상도:

- 1920×1080
- 2560×1440
- 3440×1440
- 5120×1440

32:9 화면에서도 차량 상태와 무기 정보가 화면 양 끝으로 지나치게 멀어지지 않아야 한다.

---

## 16. 갱신 방식

이벤트 기반 갱신:

- 무기 변경
- 탄약과 자원 변경
- 타겟 변경
- 식별 정보 공개
- 방어 계층과 부품 상태 변경
- Damage Resolved / Shield·Armor Break / Integrity Damage / Destroyed 전환
- 로컬 공격자의 실제 Hit Confirmation
- 경고 생성과 해제

프레임 기반 갱신:

- 조준 레티클 투영
- 타겟 마커와 화면 밖 방향 투영
- 살아 있는 Damage Event의 World Direction → 현재 Camera-relative 방향 투영

제한 주기 갱신 초기 기준:

```text
SpeedKmh: 20~30 Hz
Target Distance: 10~20 Hz
Radar Contact: 10~20 Hz
```

같은 텍스트, 색상과 아이콘은 값이 변경될 때만 다시 적용한다. 모든 Widget이 매 프레임 Pawn을 조회하고 Blueprint Cast를 반복하는 구조를 금지한다.

---

## 17. P0 UI 범위

```text
01. LocalPlayer 소유 UI Root와 레이어
02. 싱글플레이 완전 일시정지 메뉴
03. 현재 외부 3인칭 AimReticle 보존·통합
04. 숫자형 속도계
05. Shield / Armor / Vehicle Integrity 표시 계약
06. 손상 부품 경고 영역
07. 현재 무기와 공통 자원 채널
08. 선택 타겟 고정 패널과 ??? 표시
09. 가시 식별 / 스캔 공개 상태 표시
10. 센서 Contact 기반 레이더
11. 후속 Lock Provider용 시각 계약 — 현재 Runtime 부재 시 Collapse
12. Incoming Damage Direction / 실제 피해 적용 Hit Confirmation 시각 계약
13. 16:9 / 21:9 / 32:9 안전영역 검증
```

시스템이 아직 구현되지 않은 Shield, Radar, Knowledge와 Lock-on 데이터는 동일한 최종 UI 계약 형태의 Mock 데이터로 먼저 검증할 수 있다. Mock을 실제 구현 완료로 해석하지 않는다.

---

## 18. P1 이후 범위

- 포탑 전용 시점 HUD
- 다중 타겟과 다중 락온
- 센서 교란과 스텔스
- 정밀 부품 분석과 약점 표시
- 방향별 장갑 상세 표시
- 레이더 높이·신뢰도·마지막 탐지 확장
- 무기별 전용 조준 화면
- 타겟 비교와 전술 상세 패널
- 타이틀, 차고, 피팅, 임무 선택과 결과 화면
- 멀티플레이 메뉴 전용 Pause 정책

---

## 19. 구현 영향

| 영역 | 필요한 방향 |
|---|---|
| UI Framework | LocalPlayer 소유 Root, 레이어와 화면 전환 |
| Game Flow | 싱글플레이 완전 Pause 정책 |
| Camera | 외부 3인칭 기본, 후속 포탑 시점 표시 모드 |
| Damage | Shield → Armor → Vehicle Integrity와 부품 손상 결과 |
| TargetSelect | 지속 선택 대상과 변경 이벤트 제공 |
| Sensor | Radar Contact와 탐지 신뢰도 제공 |
| Target Knowledge | 가시 식별·스캔·필드별 공개 상태 제공 |
| Lock-on | 선택과 독립된 장비별 진행·완료·실패 상태 제공 |
| Weapon | 탄약, 배터리, 충전, 열 등 독립 자원 상태 제공 |
| Presentation | Widget은 View Data를 표시하고 게임 규칙을 계산하지 않음 |

---

## 20. 결정안

```text
CarFight Combat UI v0.5.0

현재는 인게임 UI만 구현한다.
인게임 HUD는 향후 전체 게임플로우를 수용할 LocalPlayer UI Root의 한 화면으로 설계한다.

싱글플레이 Pause는 입력뿐 아니라 게임 시뮬레이션 전체를 멈춘다.
기본 플레이 카메라는 외부 3인칭이며 운전석 시점은 사용하지 않는다.
포탑 시점은 후속 지원 가능성을 유지한다.

미니맵 대신 센서 Contact 기반 전투 레이더를 사용한다.
차량 방어는 Shield, Armor, Vehicle Integrity를 분리 표시하고 부품 상태는 별도 계층으로 표시한다.

선택 타겟 패널은 항상 안정적으로 유지한다.
미공개 필드는 ???로 표시한다.
가시 식별과 센서 스캔은 서로 다른 정보 획득 경로다.

선택과 락온은 별도 상태다.
현재 별도 Lock Runtime이 없으므로 기본 락온 획득 방식을 선행 확정하지 않는다. 후속 Fire-Control Provider가 실제 획득 방식과 진행·완료·실패 상태를 소유한다.

탄약, 차량 배터리, 무기 내부 충전과 열은 게임플레이에서 분리한다.
HUD 표현만 공통 자원 채널 구조로 통합한다.

속도계는 숫자형이다.
전체 시각 방향은 차량 탑재형 전술 인터페이스다.
실용적 전투 정보 70%, 차량 디지털 계기판 정체성 20%, 세계관 장식 10%의 균형을 사용한다.
스타일라이즈드 평면 UI와 Cyan 중심 Accent, Amber·Red 상태색을 사용한다.

Pause·UI 공통 입력은 ACFPlayerController가 소유한다.
UI 화면 상태는 UCFUISubsystem이 관리하고 Root Widget은 World마다 새로 생성한다.
기존 Pawn 소유 HUD는 전환 게이트와 회귀 검증을 거쳐 단계적으로 이전한다.
게임플레이 실제 정보와 플레이어 공개 정보를 분리한다.
Unknown, Unavailable과 실제 0을 명시적으로 구분한다.

피팅은 같은 UI Root와 공통 스타일을 사용할 수 있지만 별도 기능과 별도 Plan이 소유한다.
UCFUISubsystem은 Loadout을 소유하지 않으며 InGame HUD와 Fitting View Data는 분리한다.
```

---

## 21. Changelog

### v0.12.0 - 2026-08-12

- CF-FQ-032 중앙 Reticle Layer Global Declutter / Marker Density / 중앙 시야 보호 원칙을 장기 Combat UI 계약에 동기화했다.
- Declutter를 Presentation-only 공간 해결로 정의해 Target Selection, Lock, Mission Primary, Objective Lifecycle, Damage Result, Knowledge와 Fire 가능 상태를 변경하지 않도록 했다.
- P0 중앙 의미 집합을 Command 1 / Turret 1 / Selected Target 1 / Primary Objective 1 / Lock 1 / Immediate State 1 / Hit State 1 / Incoming 최대 4 Bucket으로 제한하고 Candidate·Secondary Objective·Multi-lock·Floating Number 증식을 금지했다.
- Command 위치 기준 Aim Core 56 / Combat Ring 136 / Status Lane 144×24를 연결하고 중앙 Text를 별도 Budget으로 관리하도록 했다.
- Target 보조 Text 최대 1줄, Objective Full→Compact→ShapeOnly 축약, 같은 Entity의 Target+Objective Badge 결합을 장기 원칙으로 반영했다.
- Hit/Incoming을 transient overlay로 정의해 연사·연속 피격이 Persistent Marker를 이동·Reflow하지 않도록 했다.
- Stable Identity Merge, Safe Region, Target 고정, Objective 제한 이동, Text Budget, Transient Overlay 순의 공통 Resolver 파이프라인을 기록했다.
- Edge Marker는 Target 고정 + Objective 40 간격·최대 56 Shift를 유지하고 충돌 Hysteresis 8/16과 0.10초 보조 Fade를 사용하도록 했다.
- UI-P0-03 이후 공통 Projection/Presentation Resolver가 공간 해결을 소유하고 Widget별 Tick 상호조회와 가짜 Mission/Lock Runtime 생성을 금지했다.
- 결정문에 남아 있던 `기본 락온은 유효 조준 영역 유지형` 문구를 제거하고 현재 Lock Runtime 부재·후속 Fire-Control Provider 권한과 정합화했다.
- D1-11 User Preview Gate, Source·Config·Unreal Asset과 기존 검증 상태는 변경하지 않았다.

### v0.11.0 - 2026-08-12

- CF-FQ-032 Objective World Marker / Mission Navigation의 장기 Combat UI 책임 경계를 추가했다.
- MissionSummary는 Primary Objective의 Text·Progress·Timer, Objective World Marker는 Navigation Anchor의 공간 방향을 소유하도록 분리했다.
- 현재 Mission/Objective Runtime Provider 부재를 유지해 실제 Gameplay에서 Provider 연결 전 Summary/Marker를 Collapse하고 Mock는 Visual Prototype으로만 취급하도록 했다.
- P0 Navigation은 Provider가 명시한 Primary Objective 1개만 표시하고 `+N OBJECTIVES`에서 Secondary World Marker를 자동 생성하지 않도록 했다.
- Objective Open Hex / StateNotice와 Target 4-Corner / AccentTactical 시각 의미를 분리하고, 정확/근사 위치 공개 수준을 Provider 권한으로 남겼다.
- 같은 Mission Objective가 Selected Target이면 Target Bracket + 작은 Objective Badge로 결합하고 Screen 위치만으로 Entity를 병합하지 않도록 했다.
- Marker 충돌 우선순위를 `Objective < Selected Target < Reticle`, Edge 충돌은 Target 고정 + Objective Tangential Shift로 정리했다.
- Mission Objective가 Radar Sensor Contact를 자동 생성하지 않도록 Mission Navigation과 Radar를 분리했다.
- 완료·실패·Player Destroyed·Result 수명은 상위 Mission/Player/Outcome Flow에 유지하고 새 HUD Slot, Source·Asset 변경과 D1-11 Gate 변경은 만들지 않았다.

### v0.10.0 - 2026-08-12

- CF-FQ-032 Player Integrity Critical → Vehicle Destroyed → 미래 Outcome/Result 책임 경계를 장기 Combat UI Current 원칙에 동기화했다.
- `Critical != Destroyed`, `Destroyed != Defeat/Result`를 명시하고 VehicleHealth의 파괴 사건이 Mission/Game Outcome을 직접 결정하지 않도록 했다.
- 현재 VehicleHealth는 Destroyed 상태·이벤트만 소유하고 Launcher의 OwnerDestroyed 취소와 CombatFx의 1회 Destroyed FX 외에 입력·물리·Respawn·Result를 자동 처리하지 않는 실제 Runtime 경계를 반영했다.
- 현재 Fire/Drive에 공통 Destroyed 차단이 없는 상태에서 HUD만 비활성화하는 가짜 조작불가 표현을 금지하고 실제 Combat/Vehicle Availability와 Terminal HUD 전환을 함께 연결하도록 했다.
- VehiclePanel Terminal은 실제 최종 방어/주행 값을 보존하고 `DESTROYED`를 추가하며 Critical Alert는 종료하도록 했다.
- 미래 Result Screen은 UISubsystem Screen Layer를 사용할 수 있지만 Outcome Title, Action, World Pause와 Respawn/Spectate 선택은 상위 Player/Mission/Session Flow가 소유하도록 분리했다.
- 상단 전제의 Lock 항목도 현재 별도 Lock Runtime 부재와 후속 Fire-Control Provider 계약에 맞게 교정했다.
- D1-11 User Preview Gate, D1-12와 UI-P0-03 구현 상태는 변경하지 않았고 Source·Unreal Asset은 수정하지 않았다.

### v0.9.0 - 2026-08-12

- CF-FQ-032 Incoming Damage Direction, Hit Confirmation과 Local Damage Feedback 상세 설계의 장기 Combat UI 원칙을 Current 본문에 최소 동기화했다.
- 중앙 전투 HUD에서 Command Reticle, CurrentMuzzleDirection 기반 Turret Reticle, 선택 Target/미래 Lock, Outgoing Hit Confirmation과 Incoming Damage Direction의 의미를 분리했다.
- `FireSuccess=발사 승인`, `Hit Confirmation=실제 VehicleDamageResult 방어층 감소`로 구분해 기존 FireFeedback 계약을 보호했다.
- DamageHitContext의 실제 입사 방향과 VehicleDamageResult의 실제 방어층 결과를 Damage Feedback의 Source of Truth로 고정하고 Shield/Armor-only Hit를 Integrity 호환 결과만으로 판정하지 않도록 했다.
- VehiclePanel은 실제 감소 방어층, 중앙 Damage Direction은 공격 방위, Alert는 Break/Critical 사건만 맡도록 중복 책임을 정리했다.
- 기본 Floating Damage Number를 사용하지 않는 원칙을 추가했다.
- 현재 Lock Runtime 부재를 반영해 기존 `선택 타겟 조준 유지 → Lock Progress`를 Current 기본 동작처럼 서술하던 부분을 미래 Fire-Control Provider 계약으로 교정했다.
- Source·Unreal Asset·Gameplay Runtime은 수정하지 않았다.

### v0.8.0 - 2026-08-12

- CF-FQ-032 상세 설계 동기화 검토에서 Current 본문에 남아 있던 구형 VehiclePanel 표현을 최신 Accepted 계약으로 교정했다.
- `원호형 Speed Scale + 세로 D/N/R` 현재 요약을 비대칭 실제 Engine RPM Gauge, 큰 3자리 디지털 속도, 작은 km/h와 단일 Gear Slot 구조로 교체했다.
- RPM Gauge는 속도 중복 표시가 아니라 실제 Engine RPM을 표시하며 Provider가 없을 때 가짜 RPM을 생성하지 않는 경계를 명시했다.
- VehiclePanel 6방향 Armor는 방향/Current-Max 숫자 대신 각 Plate/Badge 우측 `NormalizedArmor` 세로 Bar를 사용하는 최신 계약으로 동기화했다.
- WeaponPanel과 TargetPanel의 Current 요약을 Compact Resource Channel 및 `Tracking / Identification / Target Intelligence` 분리 계약에 맞춰 갱신했다.
- 과거 v0.7.0 Changelog의 당시 원호형 속도계·세로 D/N/R 결정은 역사 이력으로 보존하고 현재 본문만 교정했다.
- Source·Unreal Asset·Gameplay Runtime은 수정하지 않았다.

### v0.7.0 - 2026-08-07

- 좌하단 VehiclePanel Visual Layout을 Accepted로 기록했다.
- 896×416 가로형, 원호형 속도계, 왼쪽 전방 Armor 실루엣과 세로 D/N/R을 확정했다.
- Armor 위치를 좌=Front, 상=Right, 우=Rear, 하=Left로 고정하고 상부 좌상단·하부 우하단 Badge를 채택했다.
- Shield를 상단 Cluster와 Integrity 사이, Integrity를 최하단 전체 폭 Bar로 확정했다.
- 부품 손상 목록을 고정 VehiclePanel에서 제외했다.
- 다음 상세 설계를 D1-WEAPON-PANEL로 이동했다.

### v0.6.0 - 2026-08-07

- 외부 3인칭 차량 TPS와 16:9를 HUD 기본 배치 기준으로 확정했다.
- Mission 좌상단, Alert 상단 중앙, Target 우상단, Vehicle 좌하단, Radar 하단 중앙, Weapon 우하단 배치를 채택했다.
- Vehicle Panel과 Weapon Panel은 위치 승인과 내부 상세 검토를 분리했다.
- Target Panel의 이미지 미사용과 Scan 기반 점진 정보 공개 원칙을 배치 계약에 반영했다.
- 후속 사용자 HUD 배치 커스터마이징을 위한 Slot ID·Layout Profile 확장 가능성을 기록했다.
- 위치 배치 Accepted와 UI-DESIGN-GATE 전체 PASS를 분리했다.

### v0.5.0 - 2026-08-06

- `차량 탑재형 전술 인터페이스`를 CarFight 전투 UI의 승인된 시각 콘셉트로 기록했다.
- 스타일라이즈드 평면 UI, Cyan 중심 Accent와 Amber·Red 상태색을 채택했다.
- 디자인 무게를 실용적 전투 정보 70%, 차량 계기판 정체성 20%, 세계관 장식 10%로 확정했다.
- 조준·락온·Radar의 국소 전술성 강화와 전체 HUD의 차량 중심 정체성 유지 경계를 추가했다.
- 시각 방향 Accepted와 UI-DESIGN-GATE 전체 PASS를 분리하고 Style Asset·Wireframe·해상도 검토를 후속 작업으로 유지했다.
- CommonUI 도입 판단은 별도 Gate로 유지했다.

### v0.4.0 - 2026-07-31

- 주요 전체 화면을 위한 Screen Layer를 공통 UI Root에 추가했다.
- 피팅 구현을 `CF-FQ-032`에서 제외하고 별도 기능·별도 Plan이 소유하도록 확정했다.
- UI Subsystem과 Loadout, InGame HUD와 Fitting View Data, Gameplay Pawn과 Garage Preview Actor의 경계를 분리했다.
- 피팅의 Draft·검증·적용, 계산, 저장과 Runtime Snapshot은 후속 피팅 기능이 소유하도록 기록했다.
- CommonUI 도입 여부와 피팅 기능 ID는 실제 피팅 기획 착수 시 결정하도록 남겼다.

### v0.3.0 - 2026-07-31

- Pause·UI 공통 입력을 소유하는 최소 `ACFPlayerController`를 채택했다.
- LocalPlayer 수명의 `UCFUISubsystem`과 World 수명의 UI Root Widget을 분리했다.
- 현재 P0는 UMG 기반으로 유지하고 CommonUI는 후속 전체 게임플로우 단계에서 재검토하도록 확정했다.
- 기존 Pawn 소유 AimReticle·TargetSelect HUD의 단계적 소유권 이전 계약을 추가했다.
- Target Knowledge, `Unknown`·`Unavailable`·실제 0의 공개 상태 계약을 추가했다.
- 이벤트·프레임·제한 주기 갱신, View Space 화면 밖 투영과 Alert 중복 제거 원칙을 추가했다.

### v0.2.0 - 2026-07-30

- 현재 구현 범위를 인게임 UI로 제한하면서 전체 게임플로우 확장 가능한 UI Root와 레이어 원칙을 추가했다.
- 싱글플레이 완전 일시정지 정책을 확정했다.
- 외부 3인칭 기본 카메라와 후속 포탑 시점, 운전석 시점 제외를 반영했다.
- 미니맵 대신 센서 Contact 기반 레이더를 채택했다.
- Shield / Armor / Vehicle Integrity와 부품 상태 표시를 분리했다.
- 선택 타겟 패널 상시 유지, `???`, 가시 식별과 센서 스캔 분리를 추가했다.
- 선택과 락온 상태 분리, 기본 조준 유지형과 무기별 획득 방식 확장을 반영했다.
- 탄약, 배터리, 내부 충전과 열은 분리하고 UI 채널만 공통화하는 기준을 추가했다.
- 숫자형 속도계, 전투용 SF HUD, 울트라와이드 안전영역을 확정했다.
- 프로젝트 전역 사운드 비지원 정책에 따라 UI 경고가 시각 정보만으로 완결되도록 정리했다.

### v0.1 - 2026-06-08

- 전투 UI 정보 구조 초안 작성
- P0 전투 UI와 피팅 UI 범위 정의

---

## 22. Migration

- 기존 `AimReticle`과 `FireFeedback` Current System의 의미와 사용자 검증 결과는 유지한다.
- 기존 Pawn 소유 AimReticle은 `UI-P0-04`, TargetSelect HUD는 `UI-P0-05` 전까지 Current System으로 유지한다.
- 새 Root 구현 시 기존 Widget을 즉시 삭제하지 않고 `LegacyPawnOwned`와 `UISubsystemOwned` 전환 게이트를 사용한다.
- `CF-FQ-032` 착수는 Root가 아니라 `UI-P0-01A ACFPlayerController·입력 기반`부터 시작한다.
- 기존 Pawn의 Mapping Context 등록과 Widget 생성 경로는 해당 전환 단계가 완료되기 전까지 임의 삭제하지 않는다.
- `UCFUISubsystem`이 LocalPlayer와 함께 유지되어도 이전 World의 Root와 이벤트 구독은 보존하지 않는다.
- Shield, Radar, Target Knowledge, Full Pause와 공통 자원 채널은 아직 Current System이 아니다.
- Screen Layer와 공통 표현 타입을 준비해도 피팅 기능 구현 완료로 해석하지 않는다.
- Loadout, 피팅 계산, 저장, Garage Preview, Fitting Widget과 Runtime Snapshot은 별도 피팅 Plan이 생성되기 전까지 `CF-FQ-032`에 추가하지 않는다.
- 실제 구현과 사용자 PIE 완료 후 관련 `Document/Systems/` 문서로 승격한다.
