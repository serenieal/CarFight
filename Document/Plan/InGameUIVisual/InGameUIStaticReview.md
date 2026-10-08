# CarFight InGame UI Static Review

- 문서 버전: v0.6.0
- 작성일: 2026-08-07
- 최근 갱신일: 2026-08-07
- 문서 상태: D1-07 Phase 1 SR-1080-16 USER PASS / 1440p·21:9·32:9 Deferred Expansion / Runtime Capture Not Run / UI-DESIGN-GATE Assetization Pending
- 기능 ID: `CF-FQ-032`
- 대표 Plan: `InGameUIPlan.md`
- Style SSOT: `InGameUIStyleSpec.md v0.14.0`
- Visual Concept: `InGameUIVisualConcept.md v0.8.0`
- Vehicle Panel: `InGameUIVehiclePanelSpec.md v0.2.0`
- Weapon Panel: `InGameUIWeaponPanelSpec.md v0.5.0`

---

## 1. 목적

D1-01~06에서 사용자 승인된 CarFight UI 기본 Preset을 우선 `1920×1080 / 16:9` 한 가지 기준으로 완성도 있게 검토한다.

P0 UI 제작과 D1-07 Phase 1의 실제 승인 기준은 `SR-1080-16` 하나로 제한한다. 2560×1440·3440×1440·5120×1440은 현재 Gate 통과 조건에서 제외하고, 1080p 기본 HUD가 확정된 뒤 Layout Profile 확장 단계에서 검토한다.

이 문서와 Mockup은 **시각 검토용 Mock View Data**다. 실제 Gameplay Runtime, UMG 구현, Font Import, Icon Asset 또는 PIE 증거가 아니다.

검토 기본 Preset:

```text
Camera = 외부 3인칭 차량 TPS
UI Concept = Vehicle-Borne Tactical Interface
Density = HUD Standard / Weapon Compact
UIFontFamily = Pretendard
NumericFontFamily = IBM Plex Mono
IconStyle = Solid Core + Tactical Cut
Persistent HUD = 중앙 16:9 Canvas 유지
```

---

## 2. Static Mockup 산출물

| Review ID | Viewport | Mockup | 현재 역할 |
|---|---:|---|---|
| `SR-1080-16` | 1920×1080 | `ConceptArt/CFHUD_SR1080_16.xml` | **Phase 1 USER PASS** |
| `SR-1440-16` | 2560×1440 | `ConceptArt/CFHUD_SR1440_16.xml` | Deferred Expansion Reference |
| `SR-1440-21` | 3440×1440 | `ConceptArt/CFHUD_SR1440_21.xml` | Deferred Expansion Reference |
| `SR-1440-32` | 5120×1440 | `ConceptArt/CFHUD_SR1440_32.xml` | Deferred Expansion Reference |

이미 준비된 1440p·울트라와이드 Mockup은 삭제하지 않고 후속 확장 참고자료로 보존한다. 현재 D1-07 PASS/FAIL 판정에는 포함하지 않는다.

대표 Mockup 상태는 `Combat Busy`다.

```text
- Target 선택 + Hostile 관계색
- Target 정보 일부 Unknown
- Lock Segment 진행 중
- Vehicle Shield·6방향 Armor·Integrity 표시
- Front Armor Caution
- Radar에 Friendly·Neutral·Hostile·Unknown Contact
- Weapon Primary + Secondary 2개 + Rail 3개
- Warning 2개
```

Mock 숫자는 시각 검토를 위한 값이며 실제 Runtime 구현 상태를 의미하지 않는다.

---

## 3. Phase 1 1080p 구조 검토와 후속 확장 참고

### SR-1080-16 — 1920×1080

Geometry는 1440p 기준 0.75를 사용한다.

실효 Typography 목표:

| Role | 최소 화면 크기 |
|---|---:|
| Heading | 22 |
| Body | 16 |
| Label | 14 |
| Caption | 12 |
| Speed | 38 |
| Weapon Primary | 30 |
| Small Value | 14 |
| Tile Text | 12 |

배치 계산:

```text
MissionPanel = X 24~399 / Y 24~111
AlertPanel   = X 690~1242 / Y 24~105
TargetPanel  = X 1584~1896 / Y 24~256.5
VehiclePanel = X 24~696 / Y 744~1056
RadarPanel   = X 817.5~1102.5 / Y 786~1056
WeaponPanel  = X 1626~1896 / Y 865.5~1056

Mission ↔ Alert gap = 291
Alert ↔ Target gap  = 342
Vehicle ↔ Radar gap = 121.5
Radar ↔ Weapon gap  = 523.5
외곽 Margin          = 24
```

### Deferred — SR-1440-16 / 2560×1440

```text
VehiclePanel = X 64~960 / Y 960~1376
RadarPanel   = X 1090~1470 / Y 1016~1376
WeaponPanel  = X 2136~2496 / Y 1122~1376
TargetPanel  = X 2080~2496 / Y 64~374

Vehicle ↔ Radar gap = 130
외곽 Margin          = 64
Weapon actual size   = 360×254
Weapon Maximum Slot  = 464×360
```

### Deferred — SR-1440-21 / 3440×1440

```text
Central HUD Canvas = X 440~3000
왼쪽 확장 월드      = X 0~440
오른쪽 확장 월드   = X 3000~3440
```

Persistent HUD는 `SR-1440-16` 중앙 화면을 그대로 재사용한다. 추가 가로폭 때문에 Vehicle·Target·Weapon을 물리 화면 끝으로 이동하지 않는다.

### Deferred — SR-1440-32 / 5120×1440

```text
Central HUD Canvas = X 1280~3840
왼쪽 확장 월드      = X 0~1280
오른쪽 확장 월드   = X 3840~5120
```

Persistent HUD의 시선 이동 거리는 16:9와 동일하다. 추가 공간 때문에 Radar Contact 수, Weapon Rail 수 또는 Alert 수를 늘리지 않는다.

---

## 4. 사용자 PASS / FAIL 체크리스트

각 항목은 `PASS / FAIL / 수정 필요` 중 하나로 기록한다.

### 4.1 전역 시선 흐름

- [ ] 중앙 Reticle 주변이 충분히 비어 있다.
- [ ] HUD보다 월드와 적 차량이 먼저 보인다.
- [ ] Mission → Alert → Target이 상단에서 서로 경쟁하지 않는다.
- [ ] Vehicle·Radar·Weapon이 하단에서 서로 중첩되지 않는다.
- [ ] 고정 HUD 패널이 약 24px 외곽 Margin까지 바깥쪽에 배치되어 중앙 전투 시야를 불필요하게 점유하지 않는다.
- [ ] Cyan Accent가 선택·조준·시스템 강조에만 제한되어 있다.
- [ ] Amber·Red가 실제 Warning·Damage 의미에만 사용된다.

### 4.2 VehiclePanel

- [ ] 속도 `076`이 가장 먼저 읽힌다.
- [ ] D/N/R 중 현재 방향 D가 명확하다.
- [ ] 차량 전방이 화면 왼쪽이라는 Armor Map 방향이 직관적이다.
- [ ] Front/Right/Rear/Left Plate 값을 방향 텍스트 없이 구분할 수 있다.
- [ ] Armor 창 좌상단 Top Badge와 우하단 Bottom Badge로 6방향 Armor가 빠짐없이 보인다.
- [ ] Shield는 Energy Field 계층으로 읽힌다.
- [ ] Armor는 판재 계층으로 읽힌다.
- [ ] Integrity는 차체 생존 계층으로 읽힌다.
- [ ] Shield·Armor·Integrity가 색상만 다른 동일 Bar 세 개처럼 보이지 않는다.

### 4.3 WeaponPanel Compact

- [ ] 1440 Design Unit `360×254`가 1080p에서 실효 약 `270×190.5`로 유지되고, 최대 Slot 실효 `348×270` 전체를 불필요하게 채우지 않는다.
- [ ] 선택 무기가 Rail보다 확실히 강하게 보인다.
- [ ] Primary 숫자 `20 | 120`이 한 카드의 최상위 수치로 읽힌다.
- [ ] Heat와 Cooldown 두 Secondary가 과밀하지 않다.
- [ ] Fire State `재사용 대기`가 즉시 읽힌다.
- [ ] Rail Tile 3개가 1080p 실효 약 `84×51`에서도 서로 구분된다.
- [ ] 1080p에서 Weapon Tile 텍스트와 상태가 뭉개지지 않는다.
- [ ] 예전 432×224 + 136×96 넓은 Wireframe 느낌으로 되돌아가지 않았다.

### 4.4 Target·Lock·Radar

- [ ] Hostile Red와 Selected Cyan이 동시에 구분된다.
- [ ] Target 선택 Marker와 Lock Segment가 서로 다른 의미로 보인다.
- [ ] `???`가 실제 0이나 N/A처럼 보이지 않는다.
- [ ] Radar Friendly Circle, Neutral Square, Hostile Diamond, Unknown Hollow Diamond가 1080p에서 약 16~18px급 실효 크기로 구분된다.
- [ ] 선택 Hostile Radar Contact가 Red Diamond를 유지하면서 별도 Cyan Outer Bracket으로 구분된다.

### 4.5 Font·Icon

- [ ] 한글 본문은 Pretendard 방향의 정돈된 산세리프로 읽힌다.
- [ ] 숫자는 IBM Plex Mono 방향의 고정폭 계기 수치로 읽힌다.
- [ ] 한글과 숫자 Font가 서로 이질적으로 싸우지 않는다.
- [ ] 16~20px급 Solid Core + Tactical Cut Icon이 내부선 없이 읽힌다.
- [ ] Shield·Armor·Integrity 실루엣을 색 없이도 구분할 수 있다.
- [ ] Weapon 아이콘이 Side/Profile Silhouette로 읽힌다.

### 4.6 후속 21:9·32:9 확장 체크리스트 — 현재 Gate 비차단

아래 항목은 1080p 16:9 기본 HUD 승인 뒤 Layout Profile 확장 단계에서 사용한다.

- [ ] Persistent HUD가 중앙 16:9 영역에 유지된다.
- [ ] 좌우 확장 공간이 과도하게 비어 보여도 HUD를 화면 끝으로 옮기지 않는다.
- [ ] 중앙 Reticle과 Persistent HUD의 시선 이동 거리가 16:9와 같다.
- [ ] World Marker만 전체 Viewport Projection을 사용할 수 있다는 구분이 자연스럽다.
- [ ] 32:9에서 HUD가 중앙에 몰려 있어도 전투 판단에 문제가 없다.

---

## 5. 필수 상태 세트

현재 Phase 1에서는 `SR-1080-16`의 `Combat Busy`를 대표 화면으로 먼저 검토한다.

1080p 기본 HUD가 안정되면 같은 1920×1080 기준에서 아래 상태를 추가 확인한다.

| State ID | 상태 | 검토 포인트 |
|---|---|---|
| `SR-A` | Normal Driving | Target·Alert 없음 / Weapon 최소 상태 / 여백 |
| `SR-B` | Combat Busy | 현재 `SR-1080-16` 대표 Mockup / 최고 일반 정보 밀도 |
| `SR-C` | Critical | Integrity Critical / Weapon Block / Critical Alert |
| `SR-D` | Sparse Provider | 미지원 Weapon 채널·Rail Collapse / 빈 Spacer 없음 |
| `SR-E` | Long Text | 긴 한글 이름 / 4자리 수치 / 9.9초 포맷 |

`SR-A/C/D/E`는 사용자 검토 중 실제 문제가 의심되는 항목을 우선해 추가 Mockup으로 확장할 수 있다. 이는 D1-07 결과 기록용 산출물이며 Source·Unreal Asset 구현을 요구하지 않는다.

---

## 6. 전역 FAIL 조건

다음 중 하나라도 확인되면 해당 Review ID는 FAIL이다.

- 핵심 텍스트·숫자 잘림
- 한 줄 항목이 두 줄로 바뀌어 Panel 높이가 흔들림
- Reticle Protection 침범
- Vehicle·Radar·Weapon·Target 상호 중첩
- `Unknown / Unavailable / KnownZero` 의미 혼동
- 상태 의미와 무관한 Cyan·Amber·Red 장식 남용
- 1080p 유효 글자 하한 미달
- 후속 21:9·32:9 확장 시 Persistent HUD가 물리 화면 끝으로 이동

- Collapsed 대상의 고정 Spacer 잔존
- WeaponPanel이 승인 Compact 크기보다 불필요하게 확대
- Radar Contact Symbol이 1080p에서 형태로 구분되지 않음
- Font 조합 때문에 숫자·한글 Baseline과 시각 무게가 불안정함

---

## 7. 현재 판정

```text
Phase 1 Mockup Preparation = PASS
- `SR-1080-16` 1920×1080 / 16:9 기준 준비 완료
- D1-05·06 Preset 반영
- 1080p Typography 하한 반영

Phase 1 AI Structural Review = PASS AFTER MOCKUP FIX
- Mission·Alert·Target 상단 패널 상호 중첩 없음
- Vehicle·Radar·Weapon 하단 패널 상호 중첩 없음
- 사용자 피드백에 따라 고정 HUD 패널 크기는 유지하고 Phase 1 외곽 Margin을 약 24px로 축소해 화면 가장자리 쪽으로 재배치
- Weapon Content 실효 약 270×190.5 / Maximum Slot 실효 348×270 이내
- VehiclePanel Top·Bottom Armor Badge 누락 보정 완료
- OutlineHairline 1px 화면 하한 보정 완료
- Radar Contact Symbol을 실효 약 16~18px급으로 보정
- 선택 Hostile Radar Contact에 관계색을 보존하는 Cyan Outer Bracket 복구

Phase 1 Visual User Review = USER PASS
- 2026-08-07 사용자 승인: 외곽 재배치 수정본을 현재 기본안으로 채택

D1-07 Phase 1 = PASS
1440p·21:9·32:9 Review = DEFERRED EXPANSION / CURRENT GATE NON-BLOCKING
Runtime Capture = NOT RUN
UI-DESIGN-GATE = NOT PASSED / ASSETIZATION PENDING
```

AI Structural Review의 PASS는 좌표·계약·정보 밀도 정합성 검토 결과다. 실제 게임 화면에서의 시각적 취향·판독성에 대한 사용자 PASS를 대신하지 않는다.

---

## 8. 사용자 결과 기록

```text
SR-1080-16: USER PASS
승인일: 2026-08-07
승인 대상: 외곽 재배치 수정본
1080p 추가 수정 요청: 없음

후속 확장:
- SR-1440-16 / SR-1440-21 / SR-1440-32 = Deferred
```

---

## 9. Changelog

### v0.6.0 - 2026-08-07

- 사용자가 외곽 재배치가 반영된 `SR-1080-16` 수정본을 현재 기본안으로 승인했다.
- Phase 1 Visual User Review와 D1-07 Phase 1을 USER PASS / PASS로 기록했다.
- 같은 1920×1080의 SR-A·C·D·E 추가 Mockup은 현재 승인에 필수적이지 않은 선택 검토로 유지했다.
- `SR-1440-16`·`SR-1440-21`·`SR-1440-32`는 후속 Layout Profile 확장으로 Deferred하며 현재 Gate를 차단하지 않는다.
- Runtime Capture는 실행하지 않았고, UI-DESIGN-GATE 전체는 실제 Font·Icon·Style Data·Base Widget 자산화가 남아 있어 Assetization Pending으로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.5.0 - 2026-08-07

- 사용자 시각 피드백에 따라 고정 HUD 패널이 화면 중앙 쪽으로 들어와 보이던 배치를 수정했다.
- `SR-1080-16`에서 패널 크기와 내부 정보 밀도는 유지하고 Mission·Vehicle은 좌측, Target·Weapon은 우측, Alert는 상단, Radar는 하단 방향으로 각각 바깥쪽 재배치했다.
- Phase 1 외곽 Margin을 기존 약 48px에서 약 24px로 줄였고, Vehicle↔Radar 간격은 121.5px, Radar↔Weapon 간격은 523.5px로 유지되어 패널 중첩이 없음을 좌표상 확인했다.
- 24px 외곽 Margin은 1920×1080 Phase 1 전용 Layout Override이며, 1440p Design Unit `SafeMargin=64`와 1440p·21:9·32:9 Deferred Reference는 이번 변경에서 수정하지 않았다.
- 사용자 Visual Review는 재배치 Mockup 기준으로 계속 Pending이며 Runtime Capture와 UI-DESIGN-GATE는 Not Run/Not Passed를 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.4.0 - 2026-08-07

- D1-06 Radar 승인 Preset과 `SR-1080-16` Combat Busy를 재대조했다.
- Friendly Circle·Neutral Square가 0.75 Geometry 축소 후 약 12px로 작아지는 문제를 24 Design Unit 기준, 실효 약 18px급으로 보정했다.
- 선택된 Hostile Contact가 관계색 Red Diamond를 유지하면서 별도 Cyan Outer Bracket을 갖도록 복구해 Selected와 Relation 의미를 동시에 읽게 했다.
- Hostile·Unknown도 같은 24 Design Unit 계열 footprint로 정리해 관계별 Symbol 크기 편차를 줄였다.
- 사용자 Visual Review, Runtime Capture와 UI-DESIGN-GATE는 계속 Pending/Not Passed로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.3.0 - 2026-08-07

- `CFHUD_SR1080_16.xml`을 중심으로 1920×1080 / 16:9 HUD 배치와 Combat Busy 정보 밀도를 구조 검토했다.
- Mission·Alert·Target과 Vehicle·Radar·Weapon 사이에 고정 패널 중첩이 없고 외곽 Margin 48, Weapon 실효 Content 약 270×190.5가 승인 Compact 범위 안임을 확인했다.
- 승인된 VehiclePanel 6방향 Armor 계약과 달리 빠져 있던 Top·Bottom Armor Badge를 각각 Armor 창 좌상단·우하단에 복구했다.
- 1440 Design Unit 1 Hairline이 Geometry 0.75에서 0.75px이 되던 Mockup 표현을 화면 실효 약 1px이 되도록 보정했다.
- AI 구조 검토는 수정 후 PASS로 기록하되 실제 사용자 시각 Review, Runtime Capture와 UI-DESIGN-GATE는 Pending/Not Passed로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.2.0 - 2026-08-07

- 사용자 결정에 따라 D1-07 Phase 1과 P0 HUD 제작 기준을 `1920×1080 / 16:9` 중심으로 축소했다.
- `SR-1080-16`만 현재 사용자 PASS/FAIL Gate로 유지하고 2560×1440·3440×1440·5120×1440 검토는 후속 Layout Profile 확장으로 Deferred했다.
- 이미 생성한 1440p·울트라와이드 Mockup은 삭제하지 않고 확장 참고자료로 보존하되 현재 Gate 통과 조건에서는 제외했다.
- 확장 가능성은 Layout Data·Density Data·Anchor·Safe Zone·World Projection 계약으로 계속 보존한다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.1.0 - 2026-08-07

- D1-07 해상도 Static Review 전용 문서를 신규 작성했다.
- SR-1080-16, SR-1440-16, SR-1440-21, SR-1440-32 대표 Combat Busy Mockup 경로를 등록했다.
- 1080p Typography 하한, 1440p 배치 좌표, 21:9·32:9 중앙 Canvas 좌표를 검토 기준으로 기록했다.
- Vehicle·Weapon·Target·Lock·Radar·Font·Icon·울트라와이드 PASS/FAIL 체크리스트를 작성했다.
- Normal/Busy/Critical/Sparse/Long Text 필수 상태 세트와 전역 FAIL 조건을 연결했다.
- Mockup Preparation만 PASS로 기록하고 Visual User Review·Runtime Capture·UI-DESIGN-GATE는 Pending/Not Passed로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

---

## 10. Migration

- D1-07 Phase 1 시각 검토의 승인 기준은 `CFHUD_SR1080_16.xml`이며 2026-08-07 사용자 USER PASS를 기록했다.
- `SR-1080-16`의 고정 HUD는 1440p 기본 `SafeMargin=64`를 단순 0.75 축소하지 않고 Phase 1 전용 `32 Design Unit = 화면 실효 약 24px` 외곽 배치 Override를 사용한다. 후속 1440p·21:9·32:9 Layout Profile에는 자동 전파하지 않고 별도 확장 Review에서 재판정한다.
- 기존 Wireframe·VehiclePanel·WeaponPanel XML과 이미 만든 1440p·21:9·32:9 SR Mockup은 역사적·확장 참고자료로 보존하고 삭제하거나 덮어쓰지 않는다.
- 후속 해상도 확장 시 별도 Layout Profile을 추가하되 Gameplay·Presenter·View Data 계약은 변경하지 않는다.
- 21:9·32:9 Persistent HUD 중앙 고정과 World Marker 전체 Viewport Projection 계약은 미래 확장 요구로 유지한다.
- 실제 Runtime 구현 증거가 필요한 단계에서는 Static Mockup을 PIE Screenshot으로 오인하지 않는다.
