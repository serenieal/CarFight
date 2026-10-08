# CarFight InGame UI Vehicle Panel Specification

- 문서 버전: v0.20.6
- 작성일: 2026-08-07
- 최근 갱신일: 2026-08-25
- 문서 상태: Supporting Vehicle Visual/Semantic Contract / CF-FQ-032 Historical implementation checkpoints retained
- 기능 ID: `CF-FQ-032`
- Current Applicability: VehiclePanel의 의미·시각 계약만 supporting reference로 사용한다. 현재 Gear/RPM/Defense Runtime과 UMG 구현 상태는 `Systems/UI/InGameUI.md` 및 실제 Source가 우선하며 CF-FQ-039 작업 상태는 `InGameUIVisualPlan.md`가 소유한다.
- 대표 Plan: `InGameUIVisualPlan.md v0.1.19`
- 시각 콘셉트: `InGameUIVisualConcept.md v0.8.1`
- 전체 Style 규격: `InGameUIStyleSpec.md v0.86.1`
- 패널 Wireframe: `ConceptArt/CFVehiclePanel_1440p.xml`
- 현재 방어 구현: `../Systems/Combat/VehicleDefense.md`

---

## 1. 목적

이 문서는 승인된 16:9 외부 3인칭 차량 TPS HUD에서 좌하단 `VehiclePanel`이 표시할 정보의 우선순위, 896×416 내부 배치, 상태 표현과 View Data 계약을 정의한다.

이번 범위:

```text
- 속도
- 주행 방향과 선택적 실제 기어
- 주행 상태
- 핸드브레이크와 즉시 주행 경고
- Shield
- Front / Left / Right / Rear / Top / Bottom Armor
- Vehicle Integrity
- 부품 손상 요약 확장 규칙
```

이번 범위가 아닌 것:

```text
- 무기창
- Radar
- Target Panel
- 실제 UMG Asset 제작
- C++ View Data 구현
- 부품 손상 Gameplay Runtime 구현
- 사용자 HUD 위치 편집 기능
```

이 문서의 좌하단 VehiclePanel 시각 구조는 사용자 승인으로 Accepted다. 다만 실제 C++ View Data, UMG Asset, 애니메이션, 1080p·1440p 가독성 검증은 미완료이므로 `D1-VEHICLE-PANEL Implementation Complete`, Current System 또는 `UI-DESIGN-GATE PASS`로 해석하지 않는다.

---

## 2. 현재 Runtime과 설계 경계

### 2.1 현재 실제로 존재하는 데이터

`UCFVehicleDriveComp`:

```text
CurrentSpeedKmh
ForwardSpeedKmh
CurrentDriveState
bIsGrounded
ThrottleInput
SteeringInput
BrakeInput
bHandbrakePressed
```

현재 Drive State:

```text
Idle
Accelerating
Coasting
Braking
Reversing
Airborne
Disabled
```

`UCFVehicleDefenseComp`와 `UCFVehicleHealthComp`:

```text
Current / Maximum Shield
Shield Ratio
Shield Regenerating
Remaining Shield Regeneration Delay
Front / Left / Right / Rear / Top / Bottom Current Armor
Front / Left / Right / Rear / Top / Bottom Maximum Armor
각 방향 Armor Ratio
Current / Maximum Vehicle Integrity
Destroyed
마지막 피격 Armor Direction
Shield Broken / Armor Broken 이벤트
```

### 2.2 현재 존재하지 않는 데이터

```text
- 실제 부품별 내구도와 기능 저하 Runtime
- Boost Runtime
- Armor 수리와 Integrity 회복 Runtime
```

Historical note: 과거에는 실제 Gear HUD 계약이 없었지만 Current Source에서는 `ChaosWheeledVehicleMovementComponent::GetCurrentGear()`를 `UCFHUDDataProvider`가 읽어 `R / N / 실제 전진 Gear 숫자`로 ViewData에 제공한다.

표시 원칙:

- 없는 Gameplay 데이터를 UI가 추정하거나 계산하지 않는다.
- 부품 손상 Runtime이 없으면 부품 영역 자체를 표시하지 않는다.
- Gear Slot은 실제 Transmission 상태만 표시한다. 실제 Gear Data가 없으면 전진 기어 단수를 추정하지 않는다.
- Shield 또는 특정 방향 Armor가 유효하게 장착되지 않은 경우 `0% 파괴`가 아니라 `미장착`으로 구분한다.
- Legacy Fallback과 내부 에셋 이름은 플레이어 HUD에 노출하지 않는다.

---

## 3. 정보 우선순위

### 3.1 우선순위 계층

| 우선순위 | 정보 | 이유 |
|---:|---|---|
| 1 | Vehicle Integrity Critical·Destroyed, Drive Disabled | 즉시 생존과 조작 가능 여부를 결정 |
| 2 | 현재 속도, 현재 Gear Slot, Braking·Airborne·Handbrake | 현재 차량 조작과 이동 판단에 직접 필요 |
| 3 | Shield 상태와 최근 피격 Armor 방향 | 다음 피격 위험과 교전 지속 판단에 필요 |
| 4 | 6방향 Armor 전체 상태 | 차량 방향 선택과 약한 면 보호 판단에 필요 |
| 5 | 손상된 부품 요약 | 기능 저하 원인을 설명하지만 정상 상태에서는 상시 점유 불필요 |

### 3.2 시각적 무게

```text
속도 숫자
→ 항상 가장 큰 상시 수치

Vehicle Integrity Critical / Drive Disabled
→ 상태 발생 시 속도보다 강한 경고 Accent 허용

Shield / 최근 피격 Armor
→ 즉시 변화가 보이되 중앙 전투 시야를 빼앗지 않음

6방향 Armor
→ 위치가 고정된 도형으로 비교

부품 손상
→ 고정 VehiclePanel에서는 제외
→ 후속 Alert 또는 확장 상세 Panel에서만 표시
```

---

## 4. 패널 크기와 내부 좌표

기준:

```text
VehiclePanel Slot: 896 × 416
전체 HUD 위치: X 64 / Y 960
Anchor: Bottom Left
Panel Padding: 20
```

패널 로컬 좌표:

| 영역 | X | Y | W | H |
|---|---:|---:|---:|---:|
| `SpeedGauge` | 20 | 16 | 422 | 272 |
| `ArmorBodyMap` | 454 | 16 | 422 | 272 |
| `ShieldRow` | 20 | 296 | 856 | 44 |
| `IntegrityRow` | 20 | 348 | 856 | 52 |

상단 두 Module의 현재 Composition 계약은 **50:50**이다. 좌우 내부 폭 `856`에서 Module 사이 Gap `12`를 제외한 `844`를 정확히 반으로 나눠 `422 + 12 + 422`로 사용한다. 이는 2026-08-24 USER가 `CAND-VEH-MASTER-01`을 검토한 뒤 기존 35~40:55~60 비율을 교체해 명시 승인한 값이다.

### 4.1 고정 크기 원칙

- 외부 Panel은 가로형 `896×416`을 기본으로 하며 상태 변화로 늘어나거나 줄어들지 않는다.
- 상단은 좌측 계기판과 우측 장갑 상태창이 **동일한 422px 폭(50:50)**으로 나란히 배치되고 둘 사이 기본 Gap은 12px다.
- Shield는 계기판·장갑 상태창과 Vehicle Integrity 사이의 전체 폭 Bar다.
- Vehicle Integrity는 패널 최하단의 전체 폭 Bar다.
- 부품 손상 목록은 고정 VehiclePanel 안에 넣지 않는다.
- Shield 미장착과 실제 기어 미지원 상태에서도 주요 영역 위치는 바뀌지 않는다.

---

## 5. RPM Gauge + 디지털 속도계

### 5.1 채택 형태

좌측 `SpeedGauge`는 자동차 디지털 계기판을 연상시키는 **비대칭 RPM Gauge + 디지털 속도 숫자** 조합을 사용한다.

```text
좌측 세로 구간 ↑ → 넓게 휘는 전환 구간 → 상단 긴 수평 구간 = Engine RPM Gauge / Tick
+ 중앙·좌측의 큰 숫자                          = Vehicle Speed
+ 큰 속도 숫자 오른편의 작은 km/h 단위
+ 현재 변속 상태를 표시하는 단일 Gear Slot
```

중요한 의미 경계:

- 큰 디지털 숫자는 차량 속도만 표시한다.
- RPM Gauge와 Tick은 속도를 중복 표시하지 않고 **실제 Engine RPM**을 표시한다.
- RPM 값이 아직 HUD Provider에 연결되지 않은 구현 단계에서는 임의 값이나 속도 비율로 가짜 RPM을 만들어 움직이지 않는다.
- RPM Gauge의 **시각 스케일과 Red Zone 위치는 모든 차량에서 고정**한다.
- 차량마다 실제 `RedlineStartRPM`과 `MaximumEngineRPM`이 달라도 Presenter가 이를 HUD용 비율로 변환해, 같은 운전 의미가 항상 같은 게이지 위치에 나타나게 한다.
- 따라서 Gauge Tick은 절대 RPM 숫자를 직접 고정 표기하지 않는다. 실제 RPM 수치는 차량별 Gameplay Data로 유지하고, HUD Gauge는 운전자가 현재 엔진 회전 상태와 Red Zone 진입 여부를 즉시 읽는 데 집중한다.
- **Red Zone은 고정 시각 스케일의 85% 지점에서 시작**한다.
- `85%`는 모든 차량의 절대 최적 변속 RPM을 강제하는 값이 아니라 각 차량의 실제 Redline 구간 진입을 공통 HUD 위치로 표현하는 시각 계약이다.
- 실제 최적 변속 시점은 차량별 엔진 출력 특성·기어비·변속 로직이 소유하며 HUD가 85%에서 변속을 강제 판정하지 않는다.
- RPM Gauge Tick은 **총 21개**로 고정한다.
- 0%~100%를 5% 간격으로 나누며, 10% 간격 Tick은 Major, 그 사이 5% Tick은 Minor로 표현한다.
- 85% Tick은 Red Zone 시작 경계이므로 일반 Minor보다 강하게 구분한다.
- Tick 주변에는 절대 RPM 숫자를 표시하지 않는다.
- 작은 정적 `RPM` 레이블은 게이지 의미를 빠르게 읽기 위한 보조 표기로 허용한다. `TACHO`, `x1000`, 절대 RPM 숫자 눈금은 표시하지 않는다. 실제 의미는 게이지 동작·21 Tick·Red Zone이 주로 전달한다.
- 게이지 전체는 반원형이 아니다. **좌측 아래에서 세로로 위쪽을 향해 올라온 뒤, 반원 일부처럼 충분히 부드럽게 위쪽으로 휘어 전환하고, 상단의 긴 수평 직선 구간으로 이어지는 비대칭 형태**를 사용한다.
- 곡선 전환부가 아래로 처지거나 수평 구간이 세로 구간보다 아래에 놓이는 형태로 뒤집히지 않게 한다.
- 곡선 전환부를 단순한 직각 Rounded Corner처럼 보이게 만들지 않는다. 곡선 자체가 명확히 읽히되 SpeedGauge 내부 공간을 과도하게 점유하지 않게 한다.
- 현재 `SpeedGauge 422×272` 기준 곡선 반경의 초기 작업 범위는 약 **52~64px**로 둔다. 과거 320px 폭에서 사용한 40~48px은 Historical baseline이며 Current exact 계약이 아니다. 최종 곡률은 `CAND-VEH-MASTER-01`의 좌측 세로→부드러운 곡선→상단 수평 silhouette를 우선해 Slot/Art 단계에서 미세 조정한다.
- 현재 Production의 `SpeedArc*` 명칭은 기존 구현 이름이며, 이 디자인 결정 자체가 즉시 Runtime Field나 Unreal Asset Rename을 수행했다는 의미는 아니다.

### 5.2 속도 값

```text
DisplaySpeedKmh = Clamp(round(CurrentSpeedKmh), 0, 999)
DisplayFormat = 항상 3자리 정수
```

규칙:

- 표시 범위는 `000`~`999`다.
- `0 → 000`, `7 → 007`, `76 → 076`, `120 → 120`으로 표시한다.
- 실제 값이 999를 초과해도 HUD에는 `999`로 표시하고 `999+`는 사용하지 않는다.
- 속도 숫자는 RPM 게이지가 감싸는 **안쪽 여백의 주 시각 요소**로 크게 배치한다.
- 속도 숫자는 SpeedGauge 안에서 **좌측 정렬**을 유지하되, 게이지 안쪽 공간을 충분히 활용해 RPM 그래픽보다 먼저 읽히게 한다.
- 정확한 Font Size와 X/Y 좌표는 최종 시안에서 조정하되, 3자리 `999`까지 동일한 영역 안에서 흔들림 없이 들어가야 한다.
- `km/h` 단위는 속도 숫자보다 작게 하고 **큰 속도 숫자의 오른편에 가깝게 배치**한다.
- `km/h`는 속도 숫자의 보조 단위이므로 숫자보다 시각적으로 약하게 유지하고 Gear Slot과 겹치지 않게 한다.
- 소수점은 Debug Layer에서만 사용한다.

Typography 초기값:

```text
Speed Number: 64 / Bold / Tabular Number
Speed Unit: 16 / Medium
Gear Slot: Speed Number보다 작고 Speed Unit과 같은 보조 정보 계층
```

### 5.3 단일 Gear Slot

기존의 세로 `D / N / R` 3개 고정 표시는 사용하지 않는다. SpeedGauge 안에는 **현재 변속 상태 하나만 표시하는 단일 Gear Slot**을 둔다.

표시 규칙:

```text
후진 기어   → R
중립 기어   → N
전진 기어   → 실제 현재 기어 단수 숫자
예: 1 / 2 / 3 / 4 ...
```

- 전진 상태에서 `D`를 표시하지 않고 **실제 현재 기어 단수**를 표시한다.
- `R`, `N`, 전진 기어 숫자는 모두 같은 한 자리 Gear Slot을 공유한다.
- Gear Slot은 주행 방향 추정치가 아니라 실제 Transmission 상태를 표시해야 한다.
- Current Source에서는 Chaos Vehicle Movement의 실제 Current Gear를 Provider/ViewData에 연결해 사용한다.
- 속도나 DriveState를 이용해 가짜 `1`, `2`, `3` 등을 추정하지 않는 원칙은 유지한다.
- 현재 디자인은 단일 문자/숫자 슬롯을 전제로 한다. 향후 10단 이상처럼 두 자리 Gear 표기가 실제로 필요해지면 임의로 잘라 표시하지 않고 Gear Slot 폭 계약을 별도 재검토한다.

### 5.4 주행 상태와 핸드브레이크

주행 상태는 계기판 내부의 작은 상태 Chip 또는 Icon으로만 조건부 표시한다.

| Runtime 상태 | 기본 표시 | 시각 강도 |
|---|---|---|
| Idle | 생략 또는 `정지` | 낮음 |
| Accelerating | 작은 Cyan Segment | 낮음 |
| Coasting | 기본 상태, 문구 생략 가능 | 낮음 |
| Braking | Amber Brake Icon | 중간 |
| Reversing | `R` 강조 | 중간 |
| Airborne | 접지 끊김 Icon | Amber |
| Disabled | `구동 불가` | Critical |

- `ThrottleInput`, `SteeringInput`, `BrakeInput` 원시 수치는 일반 HUD에 표시하지 않는다.
- 핸드브레이크는 활성일 때만 Amber `P` Brake Icon으로 표시한다.
- 상태 문구가 RPM Gauge 안쪽의 속도 판독을 방해하지 않게 한다.

---

## 6. Shield Row

### 6.1 기본 형태와 위치

Shield는 상단의 속도계·장갑 상태창과 최하단 Vehicle Integrity 사이에 배치한다.

```text
위: SpeedGauge + ArmorBodyMap
중간: ShieldRow
아래: IntegrityRow
```

형태:

```text
전체 폭 연속형 Energy Bar
행 높이: 44
Fill 높이: 12~14
Label: 쉴드 / SHIELD
Bar 중앙 값: Current/Maximum
표시 예: 999/999
```

- Shield 수치는 Energy Bar의 가로·세로 중앙에 TextBlock으로 겹쳐 표시한다.
- 숫자 사이에는 공백을 넣지 않고 `현재/최대` 형식으로 표시한다.
- `999/999` 길이를 기준으로 안정적인 표시 폭을 확보해 값 변화 시 중앙 정렬이 흔들리지 않게 한다.
- Bar Fill과 중앙 숫자는 서로 다른 Widget 책임으로 유지한다.

예:

```text
SHIELD  ━━━━━━━━━━━ 078/100 ━━━━━━━━━━━
```

### 6.2 상태 표현

Shield 재생 상태는 별도 `재생 대기`, `재생 중`, `쉴드 소진` 문구로 표시하지 않고 **Bar Fill의 현재 우측 끝점에 붙는 3단 Chevron 진행 표시**로 통합한다.

| 상태 | 표시 |
|---|---|
| Full | Bar 중앙 `Current/Maximum` 값과 Bar만 표시, 상태 문구 없음 |
| Damaged / Delay 1 | 현재 Fill 우측 끝점에 `▶` 표시 = 재생 대기 |
| Damaged / Delay 2 | 현재 Fill 우측 끝점에 `▶▶` 표시 = 재생 대기 |
| Regenerating | 현재 Fill 우측 끝점에 `▶▶▶` 표시 = 재생 중 |
| Empty | 별도 `쉴드 소진` 문구 없음. Bar Fill 0 상태만 유지 |
| Not Installed | `쉴드 미장착`, Bar Fill 없음 |
| Unavailable | `쉴드 정보 없음`, Muted Warning Icon |

- `▶`, `▶▶`, `▶▶▶`는 Shield Bar와 별개 위치에 고정하지 않고 **현재 Fill의 우측 끝점에 붙어서 이동**한다.
- Chevron 수가 3개 미만인 동안은 재생 준비 단계이며 실제 Shield 값은 증가시키지 않는다.
- `▶▶▶`가 완성된 시점부터 Regenerating 상태로 판정하고 실제 Shield Fill 증가와 함께 Chevron 묶음도 우측으로 이동한다.
- 중앙 `Current/Maximum` 숫자는 Chevron과 독립적으로 Bar 중앙 위치를 유지한다.

### 6.3 Motion

- 피해 직후 120ms 밝기 감소와 180ms 실제 값 보간을 사용한다.
- 재생 대기 동안 Chevron은 `▶` → `▶▶` → `▶▶▶` 순서로 한 개씩 채워진다.
- `▶▶▶` 완성 후에는 세 Chevron을 유지한 채 Shield Fill 끝점과 함께 이동한다.
- 별도의 Broken/Empty 강조 애니메이션이나 `쉴드 소진` 문구는 사용하지 않는다.
- Full 상태에서 지속 Glow나 반복 애니메이션을 사용하지 않는다.

---

## 7. 6방향 Armor 상태창

### 7.1 전체 원칙

Armor를 하나의 전체 퍼센트로 합치지 않는다.

```text
Front
Left
Right
Rear
Top
Bottom
```

각 방향은 Runtime 내부에서는 다음 값을 독립 유지한다.

```text
CurrentArmor
MaximumArmor
NormalizedArmor
DisplayState
```

하지만 일반 HUD에서는 `CurrentArmor / MaximumArmor`, `80%` 같은 숫자 텍스트를 표시하지 않는다. 각 방향 Armor의 남은 상태는 **NormalizedArmor 비율을 별도 세로 비율 Bar로 표현**한다. 방향은 각 Sector의 **방향 Icon과 Runtime instance 의미**로 구분하며 화면상 상대 위치는 USER가 자유롭게 배치한다. Production 화면에는 방향 Text를 표시하지 않는다.

6방향 Armor 표시는 공통 기계식 Plate + 별도 방향 Icon + 실제 Armor 비율 Bar를 조립하는 구조를 사용한다.

- `Image_ArmorPlate`는 모든 방향이 공유하는 공통 Plate/Frame Texture를 사용한다.
- 방향 표시는 Plate에 baked하지 않고 별도 `Image_DirectionIcon`이 담당한다.
- Production 방향 Icon Texture는 **2종만** 제공한다: `Arrow`와 `Chevron2`. 두 Source Texture의 canonical 방향은 화면 오른쪽 `→`이며 각 Sector 인스턴스에서 Designer가 회전해 사용한다.
- Front/Right/Rear/Left/Top/Bottom별 완성 Texture 6장을 Production 계약으로 만들지 않는다.
- Plate와 Sector의 화면 위치·크기·Icon 종류·Icon 회전은 UMG Designer가 소유한다. Runtime 방향 의미는 화면 위치가 아니라 기존 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` instance name과 Presenter binding이 소유한다.
- Armor Plate에는 상시 수치 TextBlock을 두지 않는다.
- 세로 Bar는 `NormalizedArmor 0.0~1.0`을 직접 소비하며, 기본 Fill 방향은 아래에서 위로 채워지는 방식으로 한다.
- 비율 표시는 별도 `ProgressBar_Armor`가 담당하며 Plate/Icon Texture에 Fill 값을 굽지 않는다.
- Plate/Icon/Bar의 정확한 간격·두께·위치는 Designer에서 최종 조정한다.

### 7.2 차량 실루엣 방향

Armor 상태창의 차량 실루엣은 **차종별 VehicleData identity에 따라 교체**되는 탑다운(Top-down) HUD 이미지다. 각 차량용 Source는 화면 왼쪽을 차량 전방으로 정규화해 제작하며 월드/카메라 회전에 따라 돌리지 않는다.

```text
VehiclePawn.VehicleData
→ HUD Visual의 VehicleData별 Silhouette catalog 조회
→ 해당 차량 전용 top-down left-facing silhouette
→ 미등록 시 기존 단일 VehicleSilhouette fallback
```

현재 `UCFVehicleData` schema/DAUTH Registry를 UI 이유로 확장하지 않는다. 차종별 HUD 실루엣 매핑은 `UCFHUDVisualData`가 HUD 전용 catalog로 소유하는 방향을 사용하며 기존 단일 `VehicleSilhouette` 필드는 migration 동안 fallback으로 보존한다.

- Front/Right/Rear/Left/Top/Bottom의 Runtime 의미는 항상 구분 가능해야 하며 Production 기본 표시는 **Icon-only**다.
- 방향 Icon은 `Arrow` / `Chevron2` 2종 Source를 재사용하고 각 Sector 인스턴스에서 Designer가 회전한다. 방향별 완성 Texture 6종은 Current Production 계약이 아니다.
- `Text_Direction`은 정상 Production 표시가 아니라 Icon 미설정/오류 시 fallback/debug 확인용으로만 남긴다.
- 차량 실루엣과 6개 Sector의 정확한 픽셀 위치·크기는 UMG Designer가 소유한다. 화면상 위치로 ArmorDirection을 추론하지 않는다.
- 6방향 모두 동일한 `WBP_CFArmorSector` visual family를 재사용하고, Runtime 의미는 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` instance name과 Presenter binding으로 고정한다.
- 구현 시 Designer 배치와 실제 ArmorDirection Enum을 분리해 명시적인 바인딩을 유지한다.

### 7.3 6방향 Designer 배치

Front/Right/Rear/Left/Top/Bottom의 **화면 위치는 더 이상 Spec에서 고정하지 않는다.** USER가 `WBP_CFArmorBodyMap` Designer에서 차량 실루엣과 여섯 Sector의 최종 위치를 직접 조정한다.

- `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` 이름은 Runtime 의미 계약이므로 유지한다.
- Screen 좌/우/상/하 위치는 의미 판정 입력이 아니다. 어떤 위치로 옮겨도 Presenter는 instance name 기준으로 같은 Armor Ratio를 전달한다.
- 각 Sector의 Icon 종류(`Arrow` 또는 `Chevron2`)와 회전도 Designer 소유다.
- Top/Bottom도 다른 4방향과 같은 `WBP_CFArmorSector` 구조를 사용하고 숫자 수치를 표시하지 않는다.
- 6개 Sector 모두 `ProgressBar_Armor`를 사용하며 `NormalizedArmor 0.0~1.0`, 기본 아래→위 Fill 계약은 유지한다.

### 7.4 Editable Production UMG 구조

현재 canonical 구조는 별도 `WBP_CFArmorSlot`을 새로 만들지 않고 기존 재사용 `WBP_CFArmorSector`를 공통 Armor Slot으로 사용한다.

```text
WBP_CFArmorBodyMap
└─ CanvasPanel_Root
   ├─ Image_VehicleSilhouette          // 탑다운 좌향 차량
   ├─ WBP_ArmorFront  : WBP_CFArmorSector
   ├─ WBP_ArmorRight  : WBP_CFArmorSector
   ├─ WBP_ArmorRear   : WBP_CFArmorSector
   ├─ WBP_ArmorLeft   : WBP_CFArmorSector
   ├─ WBP_ArmorTop    : WBP_CFArmorSector
   └─ WBP_ArmorBottom : WBP_CFArmorSector

WBP_CFArmorSector
└─ 공통 의미 구조
   ├─ Image_ArmorPlate             // 공통 mechanical Plate
   ├─ Image_DirectionIcon          // NEW / Arrow 또는 Chevron2 + Designer rotation
   ├─ Text_Direction               // fallback/debug only
   └─ ProgressBar_Armor            // runtime ratio sink
```

편집 소유권:
- `WBP_CFArmorBodyMap`의 차량 실루엣과 6개 Sector 인스턴스는 각각 독립 Canvas Slot을 사용하며 **Position / Size / Alignment / 간격을 Designer에서 자유 조정**한다.
- Front/Right/Rear/Left/Top/Bottom Runtime 의미는 Widget instance name과 Presenter binding이 소유하므로 재배치 과정에서 이름을 서로 바꾸지 않는다.
- `WBP_CFArmorSector` 내부의 Plate/Direction/Ratio 의미 구조는 공통 family로 유지하되 내부 비율·간격·Brush는 Designer/Art 작업에서 조정할 수 있다.
- 새로운 common slot Widget을 중복 생성하지 않는다. 구조적 필요가 새로 생기기 전 `WBP_CFArmorSector`가 canonical reusable Armor element다.

현재 구현과 migration 경계:
- persisted 구현은 아직 `Image_ArmorPlate` 하나에 방향별 Texture를 넣는 P2 Scaffold다. 이번 USER 결정으로 이 구조는 **호환 기반**으로만 남고 Current Production 목표는 공통 Plate + 별도 `Image_DirectionIcon` 2종이다.
- 다음 WBP migration에서 `Image_DirectionIcon`을 additive 추가하고 각 Sector 인스턴스가 `Arrow/Chevron2 + rotation`을 Designer에서 설정할 수 있게 한다. 기존 6개 instance name과 `ProgressBar_Armor`는 유지한다.
- 기존 방향별 `ArmorPlates.Front~Bottom` DataAsset 필드는 migration이 끝날 때까지 compatibility fallback으로 보존하고 즉시 삭제/rename하지 않는다.
- `Text_Direction`은 Icon 미설정 시에만 fallback Label로 표시한다.
- `SetArmorPercent()`의 Runtime Armor Ratio/Visibility 계약과 `ProgressBar_Armor` sink는 유지한다. Plate/Icon 상태색 적용 방식은 WBP/C++ migration slice에서 함께 교정한다.
- 이 동작은 Gameplay Defense 상태를 재판정하지 않으며 Presenter가 전달한 Ratio만 표시한다.

### 7.5 상태 단계

UI 상태는 Gameplay 판정이 아니라 표시용 비율 단계다.

| 상태 | 조건 | 표현 |
|---|---:|---|
| Stable | Ratio > 0.60 | 낮은 명도의 Armor Amber Plate |
| Caution | 0.30 < Ratio ≤ 0.60 | 밝은 Amber + 작은 주의 Notch |
| Critical | 0 < Ratio ≤ 0.30 | Orange Red + 갈라진 Plate Icon |
| Broken | Ratio = 0, Maximum > 0 | **Current technical implementation:** Critical tint + empty Bar. **Production target:** 어두운 Plate + Red X/갈라진 Plate 계열 Art는 후속 Visual refinement에서 USER 승인 후 추가 |
| Not Installed | Maximum = 0, 데이터 유효 | Empty Frame + `—` |
| Unavailable | Provider 상태 없음 | 점선 Frame + `?` |

### 7.6 최근 피격 방향

- `OnArmorChanged` 또는 마지막 Damage Result의 Armor Direction을 사용한다.
- 최근 피격 방향 Plate 외곽선만 350ms 강조한다.
- 같은 방향 연속 피격은 강조 시간을 갱신한다.
- 전체 Armor Map을 Flash하지 않는다.
- Shield가 피해를 전부 흡수해 Armor가 변하지 않은 경우 Armor 방향을 거짓으로 강조하지 않는다.

### 7.7 Armor Broken

- 해당 방향 Plate는 위치를 유지한다.
- Red X 또는 분리된 판재 형태를 고정 표시한다.
- `Armor Broken` Alert는 상단 Alert Feed에서 1회 전달할 수 있지만, 지속 상태는 VehiclePanel Plate가 소유한다.

---

## 8. Vehicle Integrity

### 8.1 기본 형태

IntegrityRow는 ShieldRow와 같은 읽기 규칙을 사용한다.

```text
전체 폭 연속형 Integrity Bar
Label: 차량 / INTEGRITY
Bar 중앙 값: Current/Maximum
표시 예: 999/999
```

- Integrity 수치는 Bar의 가로·세로 중앙에 TextBlock으로 겹쳐 표시한다.
- 숫자 사이에는 공백을 넣지 않고 `현재/최대` 형식으로 표시한다.
- `999/999` 길이를 기준으로 안정적인 표시 폭을 확보한다.
- Bar Fill과 중앙 숫자는 서로 다른 Widget 책임으로 유지한다.
- ShieldRow와 시각 문법은 통일하되, 색상과 Critical 상태 강조는 Integrity 의미에 맞게 유지할 수 있다.

예:

```text
INTEGRITY  ━━━━━━━━━━━ 091/100 ━━━━━━━━━━━
```

### 8.2 상태 단계

| 상태 | 조건 | 표현 |
|---|---:|---|
| Stable | Ratio > 0.60 | 밝은 Neutral Fill + Integrity Accent Label |
| Caution | 0.30 < Ratio ≤ 0.60 | Amber Fill |
| Critical | 0 < Ratio ≤ 0.30 | Red Fill + 700ms 낮은 Pulse |
| Destroyed | Ratio = 0 | Bar Fill 0 유지. 차량 파괴 Gameplay 상태 표시는 별도 시스템이 소유할 수 있다. |
| Unavailable | Health Provider 없음 | Muted Bar + `정보 없음` |

Integrity는 생존의 최종 계층이므로 Critical 진입 시 패널 외곽선 일부에도 1회 Red Accent를 허용한다.

### 8.3 수리 상태 표현

수리 상태는 Shield 재생과 동일한 3단 Chevron 문법을 사용한다.

| 상태 | 표시 |
|---|---|
| Repair Delay 1 | 현재 Fill 우측 끝점에 `▶` 표시 = 수리 대기 |
| Repair Delay 2 | 현재 Fill 우측 끝점에 `▶▶` 표시 = 수리 대기 |
| Repairing | 현재 Fill 우측 끝점에 `▶▶▶` 표시 = 수리 중 |

- `▶`, `▶▶`, `▶▶▶`는 Integrity Bar의 현재 Fill 우측 끝점에 붙어서 이동한다.
- Chevron이 3개 미만인 동안은 수리 준비 단계이며 실제 Integrity 값은 증가시키지 않는다.
- `▶▶▶`가 완성된 시점부터 실제 수리를 시작하고 Fill 증가와 함께 Chevron 묶음도 우측으로 이동한다.
- 별도 `수리 대기`, `수리 중` 텍스트는 기본 HUD에 표시하지 않는다.
- 중앙 `Current/Maximum` 숫자는 Chevron과 독립적으로 Bar 중앙을 유지한다.

### 8.4 피해·수리 보간

- 피해 시 실제 현재값은 즉시 적용한다.
- 시각 Fill은 최대 180ms로 따라간다.
- 수리 중에도 실제 Integrity 값과 시각 Fill을 분리해 필요한 경우 짧게 보간할 수 있다.
- 선택적으로 이전값을 나타내는 얇은 Damage Trail을 350ms 유지할 수 있다.
- Trail은 실제 피해량을 계산하지 않고 Presenter가 제공한 이전 Ratio만 사용한다.

---

## 9. 부품 손상 표시 경계

### 9.1 고정 VehiclePanel에서 제외

사용자가 승인한 좌하단 고정 패널은 다음 네 영역만 유지한다.

```text
SpeedGauge
ArmorBodyMap
ShieldRow
IntegrityRow
```

부품 손상 목록은 정보 밀도를 높이고 Armor Map 공간을 줄이므로 고정 VehiclePanel 안에 표시하지 않는다.

### 9.2 현재 구현 경계

현재 P0에는 실제 부품별 손상 Runtime이 없다.

따라서:

```text
- 부품 정상이라는 가짜 목록 표시 금지
- 임의 퍼센트 표시 금지
- Debug Component 이름 표시 금지
- Damage Result 예약 필드를 실제 부품 손상으로 해석 금지
```

### 9.3 후속 확장 후보

실제 부품 손상 Provider가 구현되면 다음 중 하나를 별도 설계한다.

```text
A. 상단 Alert Feed의 순간 경고
B. VehiclePanel 인접 Compact Damage Chip
C. 입력으로 여는 차량 상세 Panel
```

지속 목록을 기본 VehiclePanel 안에 다시 추가하려면 별도 사용자 승인이 필요하다. 부품 손상 판정은 Gameplay Runtime이 소유하고 UI는 결과만 표시한다.

---

## 10. 패널 상태 전환과 경고

### 10.1 패널 전체가 강하게 반응하는 조건

```text
Vehicle Integrity Critical 진입
Drive Disabled 진입
Destroyed 전환
```

Armor 하나가 감소하거나 Shield가 일부 감소한 것만으로 패널 전체를 Flash하지 않는다.

### 10.2 지속 경고와 순간 경고 분리

```text
순간 경고
→ 상단 Alert Feed 또는 1회 Panel Accent

지속 상태
→ Shield Row, Armor Plate + 세로 Bar, Integrity Row
```

### 10.3 애니메이션 상한

- 일반 값 변화: 120~180ms
- Armor Hit Plate: 350ms
- Shield 재생 Chevron 단계 전환: 시각적으로 구분되되 과도한 Flash 금지
- Critical Integrity Pulse: 700ms
- 반복 Pulse는 Critical Integrity와 Drive Disabled에만 제한

---

## 11. 권장 View Data 계약

향후 UI-P0-03에서 다음 표현용 구조를 권장한다.

```text
FCFVehiclePanelViewData
FCFVehicleDriveViewData
FCFDefenseLayerViewData
FCFDirectionalArmorViewData
FCFVehiclePartStatusViewData
```

### 11.1 FCFVehiclePanelViewData

```text
bVehicleDataAvailable
Drive
Shield
FrontArmor
LeftArmor
RightArmor
RearArmor
TopArmor
BottomArmor
Integrity
DamagedParts
bComponentDamageSupported
LastChangedArmorDirection
LastArmorHitGameTimeSeconds
PanelAlertState
```

### 11.2 FCFVehicleDriveViewData

```text
SpeedKmh
EngineRPM
DriveDirection
DriveState
bHandbrakeActive
bGrounded
bTransmissionGearAvailable
TransmissionGearText
```

`EngineRPM`은 실제 Gameplay RPM 값이다. HUD Presenter는 차량별 `RedlineStartRPM`과 `MaximumEngineRPM`을 기준으로 이를 **고정 시각 스케일용 RPM 비율**로 변환한다. 차량별 절대 RPM 차이를 Widget이 직접 해석하지 않는다.

고정 스케일 계약:

```text
실제 차량 RPM 특성
- EngineRPM
- RedlineStartRPM
- MaximumEngineRPM

Presenter
→ 차량별 실제 값을 HUD용 TachoDisplayRatio로 변환

HUD
- TachoDisplayRatio = 0.0 ~ 1.0
- Red Zone의 시각적 시작 위치 = 0.85
- 실제 RedlineStartRPM 도달 시 = TachoDisplayRatio 0.85 위치에 도달
- MaximumEngineRPM 도달 시 = Gauge 끝 1.0에 도달
```

Red Zone 시작 비율은 `0.85`로 고정한다.

Tick 계약:

```text
총 Tick = 21
간격 = 5%
Major Tick = 0%, 10%, 20% ... 100% / 11개
Minor Tick = 5%, 15%, 25% ... 95% / 10개
85% Tick = Red Zone 시작 경계 강조
절대 RPM 숫자 = 표시하지 않음
작은 `RPM` 레이블 = 표시 허용
TACHO/x1000 레이블 = 표시하지 않음
```

### 11.3 FCFDefenseLayerViewData

```text
CurrentValue
MaximumValue
NormalizedValue
DisplayState
PrimaryText
SecondaryText
bRegenerating
RemainingRegenerationDelaySeconds
RecoveryChevronCount
bRecoveryActive
```

### 11.4 FCFDirectionalArmorViewData

```text
ArmorDirection
CurrentValue
MaximumValue
NormalizedValue
DisplayState
bRecentlyHit
bBroken
```

### 11.5 Display State

```text
Known
NotInstalled
Unavailable
```

`Known` 안에서 Ratio 기반 Stable·Caution·Critical·Broken을 계산하는 것은 Presenter의 표현 변환 책임으로 둔다. Widget이 `Current / Maximum`을 다시 계산하지 않는다.

---

## 12. 데이터 갱신과 소유권

### 이벤트 기반

```text
Drive State 변경
Handbrake 상태 변경
Shield 변경·파괴·재생 시작·완전 회복
방향 Armor 변경·파괴
Vehicle Integrity 변경·파괴
부품 상태 변경
```

### 제한 주기 기반

```text
Speed: 20~30 Hz Snapshot
Shield 재생 수치: 10~20 Hz 또는 이벤트 결과 보간
```

### Widget 책임 금지

```text
Widget Tick
→ Get Player Pawn
→ Cast
→ DriveComp / DefenseComp / HealthComp 직접 조회
```

권장 흐름:

```text
Vehicle HUD Presenter
→ 현재 Pawn의 Drive·Defense·Health 이벤트 구독
→ FCFVehiclePanelViewData 생성
→ WBP_CFVehicleHUD에 전달
```

Pawn 교체 또는 World 종료 시 기존 이벤트 구독을 반드시 해제한다.

---

## 13. 권장 UMG 구조

```text
WBP_CFVehicleHUD
└─ WBP_CFPanelBase
      └─ VerticalBox
      ├─ UpperCluster
      │  └─ HorizontalBox
      │     ├─ WBP_CFSpeedGauge
      │     │  ├─ TachoGauge
      │     │  ├─ SpeedNumber
      │     │  ├─ SpeedUnit
      │     │  ├─ GearSlot
      │     │  └─ ConditionalDriveIcons
      │     └─ WBP_CFArmorBodyMap
      │        ├─ LeftFacingVehicleSilhouette
      │        ├─ FrontPlate_Left + VerticalBar_Right
      │        ├─ RightPlate_Top + VerticalBar_Right
      │        ├─ RearPlate_Right + VerticalBar_Right
      │        ├─ LeftPlate_Bottom + VerticalBar_Right
      │        ├─ TopArmorBadge_TopLeft + VerticalBar_Right
      │        └─ BottomArmorBadge_BottomRight + VerticalBar_Right
      ├─ WBP_CFStatusBar_Shield
      │  ├─ CenterValueText
      │  └─ RecoveryChevronAtFillEnd
      └─ WBP_CFStatusBar_Integrity
         ├─ CenterValueText
         └─ RepairChevronAtFillEnd
```

신규 후보 이름은 모두 32자를 넘지 않는다.

### C++과 Blueprint 배분

```text
C++
- View Data 구조체
- Provider·Presenter
- Display State와 우선순위
- 이벤트 구독 수명
- 숫자 정규화와 안전값

Blueprint / UMG
- 896×416 레이아웃
- Armor Plate 이미지와 우측 세로 Bar 배치
- 색상·Icon·Typography
- Shield/Integrity 중앙 수치와 Fill 끝 Chevron 배치
- 값 보간과 상태 애니메이션
- 1080p·1440p DPI 대응
```

---

## 14. 해상도 대응

### 2560×1440

```text
896 × 416
```

### 1920×1080

```text
기본 Scale 0.75
실제 크기 약 672 × 312
```

1080p 검토 시:

- 6방향 Armor의 세로 Bar가 서로 구분되고 남은 비율을 읽을 수 있는지 확인한다.
- Shield/Integrity 중앙 `999/999` 수치와 Fill 끝 Chevron이 겹치지 않는지 확인한다.
- Speed `000`, `km/h`, Gear Slot의 정보 계층이 유지되는지 확인한다.
- 패널 크기를 줄이기보다 내부 Caption과 장식선을 먼저 줄인다.

### 21:9·32:9

- 중앙 16:9 Canvas의 좌하단 Anchor를 유지한다.
- 물리 화면의 최좌측 끝으로 이동하지 않는다.
- 후속 사용자 Layout Override도 기본 Profile 좌표를 기준으로 한다.

---

## 15. 검증 기준

### 3초 판독

플레이어가 3초 이내에 다음을 읽어야 한다.

```text
현재 속도
전진·후진 여부
가장 위험한 방어 계층
가장 약한 Armor 방향
핸드브레이크 또는 구동 불가 여부
```

### 1초 반응

다음은 1초 이내에 인지돼야 한다.

```text
Shield Broken
특정 방향 Armor Broken
Vehicle Integrity Critical
Drive Disabled
Airborne
```

### 상태 조합 검토

```text
Shield Full / Delay(▶·▶▶) / Regenerating(▶▶▶) / Empty / NotInstalled
각 Armor Stable / Caution / Critical / Broken / NotInstalled
Integrity Stable / Caution / Critical / Destroyed / Repair Delay(▶·▶▶) / Repairing(▶▶▶)
Gear Slot: R / N / 실제 전진 기어 숫자
Braking + Handbrake
Airborne
Drive Disabled
왼쪽 전방 실루엣 기준 4방향 Armor 바인딩
상부 좌상단 / 하부 우하단 Badge
```

### 실제 데이터 경계 검토

- Legacy Fallback을 플레이어에게 기술 용어로 표시하지 않는다.
- MaximumArmor가 0인 방향을 Broken으로 표시하지 않는다.
- 현재 부품 Runtime이 없을 때 부품 목록이 나타나지 않는다.
- 실제 Gear Data가 없을 때 가짜 기어 숫자가 나타나지 않는다.
- 실제 RPM 값이 연결되지 않은 상태에서 속도나 임의 애니메이션으로 가짜 RPM을 표시하지 않는다.
- Widget이 피해 계산이나 Armor 방향 판정을 다시 수행하지 않는다.

---

## 16. 사용자 승인 상태

### Visual Layout Accepted

```text
1. 좌하단 가로형 VehiclePanel
2. 좌측 SpeedGauge는 **좌측 세로→부드러운 곡선→상단 긴 수평으로 이어지는 비대칭 RPM Gauge + 큰 디지털 속도 숫자** 조합
3. 속도는 `000`~`999`, 3자리, 좌측 정렬, 999 초과도 `999`; 작은 `km/h`와 그 오른쪽 Gear Slot을 속도 숫자보다 작게 표시
4. 단일 Gear Slot에서 후진 `R`, 중립 `N`, 전진은 실제 현재 기어 단수 숫자 표시
5. 우측 ArmorBodyMap 정중앙에 왼쪽을 바라보는 차량 실루엣 배치
6. Front/Right/Rear/Left는 차량을 둘러싸는 독립 Armor Plate, Top은 좌상단 Badge, Bottom은 우하단 Badge
7. Armor 방향 텍스트와 수치 텍스트는 제거하고 6방향 모두 Plate/Badge 우측 세로 Bar로 `NormalizedArmor`를 표시
8. Armor 바인딩: 좌=Front / 상=Right / 우=Rear / 하=Left, Top=좌상단 / Bottom=우하단
9. Shield Bar를 상단 Cluster와 Integrity 사이에 배치하고 중앙에 `999/999`; Fill 끝 `▶`·`▶▶`=재생 대기, `▶▶▶`=재생 중
10. Vehicle Integrity를 최하단 전체 폭 Bar로 배치하고 중앙 `999/999`; Fill 끝 Chevron으로 수리 대기/수리 중을 Shield와 동일하게 표시
11. 부품 손상 목록을 고정 패널에서 제외
```

현재 상태:

```text
D1-VEHICLE-PANEL Visual Layout: Accepted
D1-VEHICLE-PANEL Runtime·UMG Implementation: Pending
D1-WEAPON-PANEL: Pending / Not Touched
UI-DESIGN-GATE: Not Passed
```

---

## 17. Changelog

### v0.20.6 - 2026-08-25

- USER 결정으로 Armor 표시를 방향별 완성 Plate 6종에서 **공통 Plate + 별도 방향 Icon 2종(`Arrow`, `Chevron2`)** 조립 구조로 변경했다. 두 Icon은 canonical `→` Source를 제공하고 각 `WBP_CFArmorSector` 인스턴스의 Icon 종류·회전·Position/Size는 UMG Designer가 소유한다.
- Front/Right/Rear/Left/Top/Bottom의 화면 위치 고정 계약을 해제했다. Runtime 의미는 화면 좌표가 아니라 기존 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom` instance name과 Presenter binding이 계속 소유한다.
- 중앙 차량 실루엣은 전역 1장이 아니라 **현재 VehicleData identity별로 교체**되는 HUD 이미지로 변경했다. `UCFVehicleData`/DAUTH schema 확장은 피하고 `UCFHUDVisualData`의 차종별 silhouette catalog + 기존 단일 `VehicleSilhouette` fallback 방향을 채택했다.
- canonical reusable Widget은 계속 `WBP_CFArmorSector`이며 새 `WBP_CFArmorSlot`을 만들지 않는다. 다음 migration 목표 내부 구조는 `Image_ArmorPlate + Image_DirectionIcon + Text_Direction fallback + ProgressBar_Armor`다.
- 대표 Plan 포인터를 `InGameUIVisualPlan.md v0.1.19`로 동기화했다. 아직 WBP/C++/DataAsset runtime mutation은 수행하지 않았다.

### v0.20.5 - 2026-08-24

- USER 결정으로 Armor 방향 표기를 Production에서 **Icon-only**로 확정했다. Front `←`, Right `↑`, Rear `→`, Left `↓`, Top `⌃⌃`, Bottom `⌄⌄` 의미를 유지하고 `Text_Direction`은 missing-texture fallback/debug 전용으로 제한했다.
- RPM 표기는 작은 `RPM` 레이블만 허용하고 `TACHO`, `x1000`, 절대 숫자 눈금은 금지했다. 기존 21 Tick / 85% Red Zone / 실제 RPM→HUD Ratio 의미 계약은 유지한다.
- Shield/Integrity의 Chevron은 Master Candidate의 기계식 모양 언어를 사용하되 기존 `▶/▶▶/▶▶▶` recovery/repair 상태와 Fill-end 이동 의미를 유지한다. 고정 장식 Chevron으로 바꾸지 않는다.
- 대표 Plan 포인터를 `InGameUIVisualPlan.md v0.1.13`으로 동기화했다. Runtime/Gameplay 계산은 변경하지 않았다.

### v0.20.4 - 2026-08-24

- USER가 `CAND-VEH-MASTER-01` 기준 상단 `Speed/RPM : Armor` 비율을 **50:50**으로 명시 승인했다. 기존 35~40:55~60 composition은 Current에서 폐기하고 Historical로만 보존한다.
- 896×416 VehiclePanel의 좌우 내부폭 856, Module Gap 12를 기준으로 `SpeedGauge 20,16 / 422×272`, `ArmorBodyMap 454,16 / 422×272`를 Current Composition 좌표로 고정했다.
- SpeedGauge 폭 증가에 맞춰 과거 320px 기준 RPM 곡선 반경 40~48px을 Historical baseline으로 내리고 Current 초기 작업 범위를 52~64px로 갱신했다. 최종 곡률은 Master Candidate silhouette가 우선한다.
- Runtime source, Gear/RPM 의미, Armor 6방향 의미와 Shield/Integrity 전체폭 구조는 변경하지 않았다. `VT-VEH-01` 전체 승격은 별도 USER 승인 전까지 Pending이다.

### v0.20.3 - 2026-08-24

- `WBP_CFArmorSector`의 현재 Production Presentation 계약을 Texture/Icon 우선 + `Text_Direction` fallback으로 명시했다.
- 기존 P2 6방향 Texture family와 `SetArmorPercent()` 기반 Plate/세로 Bar 동시 상태색 적용을 Current 구현으로 반영했다.
- Armor Ratio Presentation 단계는 Stable/Caution/Critical을 기술 구현했고, 0% Broken의 Red X/갈라진 Plate Art는 아직 USER Visual refinement Pending임을 명확히 분리했다.
- Runtime Defense 계산, 6방향 instance name과 Designer Layout ownership은 변경하지 않았다.

### v0.20.2 - 2026-08-24

- CF-FQ-039 USER 피드백으로 중앙 차량을 명시적인 **탑다운 좌향 silhouette**로 고정했다.
- 6방향 Armor는 위치만으로 추측하게 두지 않고 Direction Indicator(Icon 또는 짧은 Label)로 Front/Right/Rear/Left/Top/Bottom을 즉시 구분하도록 보강했다.
- 현재 persisted 구현을 반영해 `WBP_CFArmorBodyMap Canvas + VehicleSilhouette + WBP_CFArmorSector 6개`를 canonical editable production 구조로 문서화했다.
- 사용자 요청의 신규 `WBP_CFArmorSlot`은 기존 `WBP_CFArmorSector`와 중복이므로 생성하지 않고 기존 Sector를 common Armor slot으로 유지했다.
- 6개 Sector와 VehicleSilhouette의 정확한 pixel Position/Size는 Designer ownership이며 Runtime 의미 instance name은 보존하도록 경계를 고정했다.

### v0.20.0 - 2026-08-11

- VehiclePanel 전체 조합 검토를 수행하고 SpeedGauge·ArmorBodyMap·ShieldRow·IntegrityRow 간 시각 구조 충돌이 없음을 확인했다.
- 현재-state 문서의 `RPM 원호`, `Arc Label`, `560×416`, Armor 수치 표기, 부품 손상 행 등 이전 설계 흔적을 최신 사용자 결정에 맞게 정리했다.
- 권장 UMG 구조를 단일 Gear Slot, 6방향 Armor 우측 세로 Bar, Shield/Integrity 중앙 `999/999`, Fill 끝 Chevron 구조로 갱신했다.
- 1080p 검토 항목과 사용자 승인 상태를 현재 Armor/Shield/Integrity 설계에 맞게 갱신했다.
- View Data에 `RecoveryChevronCount`, `bRecoveryActive` 표시용 필드를 추가해 Shield 재생과 향후 Integrity 수리의 공통 Chevron 표현 경계를 명시했다.
- 과거 Changelog의 당시 설계 기록은 역사 증거로 보존했다.

### v0.19.0 - 2026-08-11

- 사용자 결정으로 IntegrityRow의 기본 시각 문법을 ShieldRow와 통일했다.
- Integrity Bar 중앙에 `Current/Maximum`을 `999/999` 형식으로 표시하도록 확정했다.
- 수리 대기는 `▶`, `▶▶`, 수리 중은 `▶▶▶`로 표시하며 Chevron 묶음은 현재 Fill 우측 끝점에 붙어 이동한다.
- `▶▶▶` 완성 전에는 실제 Integrity가 증가하지 않고, 완성 후 실제 수리와 Fill 증가가 시작된다.
- 별도 `수리 대기`, `수리 중` 텍스트는 기본 HUD에서 사용하지 않는다.
- Integrity 0일 때 Bar는 0 Fill을 유지하고, 차량 파괴 Gameplay 상태의 별도 표현은 다른 시스템이 소유할 수 있도록 분리했다.

### v0.18.0 - 2026-08-11

- 사용자 결정으로 Shield 재생 상태를 Fill 우측 끝점에 붙는 3단 Chevron 진행 표시로 통합했다.
- `▶`, `▶▶`는 재생 대기, `▶▶▶` 완성은 재생 중을 의미한다.
- Regenerating 중 `▶▶▶` 묶음은 현재 Shield Fill의 우측 끝점에 계속 붙어서 Fill 증가와 함께 이동한다.
- 별도 `재생 대기`, `재생 중`, `쉴드 소진` 상태 문구와 소진 전용 표현은 제거했다.
- 중앙 `Current/Maximum` 숫자는 Chevron과 무관하게 Bar 중앙을 유지한다.

### v0.17.0 - 2026-08-11

- 사용자 결정으로 Shield 수치를 Energy Bar 중앙에 `Current/Maximum` 형식으로 표시하도록 확정했다.
- 대표 표시 폭은 `999/999` 기준이며 숫자 사이 공백 없이 중앙 정렬한다.
- Shield Fill과 중앙 숫자는 별도 Widget 책임으로 유지해 Bar 비율과 텍스트를 독립적으로 갱신한다.

### v0.16.0 - 2026-08-11

- 사용자 결정으로 Top·Bottom Armor도 4방향 Armor와 동일하게 Badge 우측에 세로 비율 Bar를 배치하도록 확정했다.
- 6방향 Armor 전체가 `NormalizedArmor` 기반 세로 Bar와 아래→위 Fill 규칙을 공유하도록 통일했다.
- 숫자·퍼센트 텍스트 미표시 원칙은 그대로 유지한다.

### v0.15.0 - 2026-08-11

- 사용자 결정으로 Front·Right·Rear·Left Armor의 비율 표시는 Plate 내부 Fill이 아니라 **Plate 우측의 세로 Bar**로 변경했다.
- Plate 그래픽은 `Image + Texture2D`, 남은 비율은 별도 세로 `ProgressBar`가 담당하도록 책임을 분리했다.
- 세로 Bar는 `NormalizedArmor`를 사용하며 기본 Fill 방향은 아래→위로 정의했다.
- Top·Bottom은 숫자 미표시 원칙만 유지하고, Badge 비율 Bar의 정확한 위치는 후속 시안 결정으로 남겼다.

### v0.14.0 - 2026-08-11

- 사용자 결정으로 6방향 Armor의 `현재/최대`, 퍼센트 숫자 등 상시 수치 표기를 제거했다.
- Front·Right·Rear·Left Plate와 Top·Bottom Badge는 `NormalizedArmor`를 그래픽 자체의 Fill 비율로만 표현하도록 변경했다.
- 비직사각형 Armor Plate에는 단순 ProgressBar를 겹치지 않고 `Image + UI Material`의 비율 파라미터로 Fill을 제어하는 방식을 기본안으로 정했다.
- Runtime의 CurrentArmor와 MaximumArmor 데이터는 계산·상태 판정용으로 유지하되 일반 HUD 텍스트에는 노출하지 않는다.

### v0.13.0 - 2026-08-11

- 사용자 결정으로 Top Armor Badge는 ArmorBodyMap 좌상단, Bottom Armor Badge는 ArmorBodyMap 우하단으로 최종 확정했다.
- 기존 보조 Badge 방식은 유지하고 위치만 사용자 확정 상태로 승격했다.
- Front·Right·Rear·Left 4방향 Plate와 중앙 차량 실루엣 배치는 변경하지 않았다.

### v0.12.0 - 2026-08-11

- 사용자 결정으로 Front·Right·Rear·Left의 4개 주요 Armor 표시를 단순 Bar가 아닌 독립 Armor Plate 형태로 확정했다.
- Plate는 중앙 차량 실루엣을 둘러싸되 차량을 가리지 않고, 방향 관계 자체로 Armor 면을 읽게 한다.
- Plate 그래픽은 방향별 `Image + Texture2D`를 기본으로 하고 Border 조각을 쌓아 모양을 만드는 방식은 사용하지 않는다.
- Armor 수치 TextBlock과 Plate 그래픽 자산을 분리해 WBP에서 위치·간격·세부 외형을 조정할 수 있게 했다.

### v0.11.0 - 2026-08-11

- 사용자 결정으로 ArmorBodyMap의 차량 실루엣을 정중앙에 배치하도록 확정했다.
- Front·Right·Rear·Left Plate는 중앙 실루엣을 둘러싸는 고정 위치 관계를 유지한다.
- 차량 전방은 기존대로 화면 왼쪽을 유지하며 월드·카메라 회전에 따라 실루엣을 회전시키지 않는다.

### v0.10.0 - 2026-08-11

- 사용자 결정으로 기존 세로 `D/N/R` 3개 고정 표시를 폐기하고 단일 Gear Slot으로 변경했다.
- 후진은 `R`, 중립은 `N`, 전진은 실제 현재 기어 단수 숫자를 표시하도록 확정했다.
- 전진 시 `D`는 사용하지 않으며 Gear Slot은 실제 Transmission 상태만 소비한다.
- 현재 HUD 공용 Gear Data가 확인되지 않은 상태에서는 가짜 기어 단수를 추정하지 않고 실제 Provider 연결을 구현 과제로 남겼다.
- `km/h` 위치도 직전 사용자 결정에 맞춰 속도 숫자 아래가 아니라 큰 속도 숫자의 오른편으로 교정했다.

### v0.9.0 - 2026-08-11

- 사용자 결정으로 디지털 속도 숫자를 RPM 게이지가 감싸는 안쪽 공간에 크게 배치하도록 확정했다.
- 속도 숫자는 좌측 정렬을 유지하고, RPM 게이지보다 시각적으로 우선해 가장 먼저 읽히는 상시 수치로 유지한다.
- `km/h`는 큰 속도 숫자 바로 아래에 작게 좌측 정렬한다.
- 정확한 Font Size와 X/Y 좌표는 최종 시안에서 조정하되 `000`~`999` 3자리 영역은 고정한다.

### v0.8.0 - 2026-08-11

- RPM 게이지 방향 해석 오류를 교정했다.
- 좌우 배치는 유지하고, 상하 방향만 바로잡아 `좌측 아래 세로 시작 → 위로 상승 → 넓은 곡선 전환 → 상단 긴 수평 구간`으로 명시했다.
- 곡선이 아래로 처지거나 수평 구간이 낮게 내려오는 뒤집힌 형태를 금지했다.
- 기존 21 Tick, 85% Red Zone, 숫자·레이블 없음, 곡선 반경 40~48px 기준은 유지했다.

### v0.7.0 - 2026-08-11

- 사용자 결정으로 RPM 게이지 전체 형태를 반원형에서 비대칭 연속 경로로 구체화했다.
- 경로는 `좌측 세로 구간 → 넓게 휘는 곡선 전환부 → 상단 긴 수평 직선 구간`으로 정의했다.
- 전환부는 단순 90° Rounded Corner가 아니라 반원 일부처럼 충분히 부드럽게 휘도록 했다.
- `SpeedGauge 320×272` 기준 곡선 반경은 초기 시안 약 `40~48px`, 기본 상한 `48px`로 두고 최종 시안에서 미세조정 가능하도록 했다.
- RPM 21 Tick, 85% Red Zone, 숫자·RPM 레이블 없음 계약은 유지했다.

### v0.6.0 - 2026-08-11

- 사용자 결정으로 RPM 원호 Tick을 총 `21개`로 확정했다.
- 0%~100%를 5% 간격으로 나누고 10% 간격은 Major Tick, 중간 5%는 Minor Tick으로 정의했다.
- 85% Tick은 Red Zone 시작 경계로 별도 강조한다.
- RPM 게이지에는 절대 RPM 숫자를 표시하지 않는다.
- `RPM`, `TACHO`, `x1000` 같은 별도 레이블도 표시하지 않고, 원호 동작과 Red Zone 자체로 Tachometer 의미를 전달한다.

### v0.5.0 - 2026-08-11

- 사용자 결정으로 RPM 고정 스케일의 Red Zone 시작 위치를 `85%`로 확정했다.
- `85%`는 공통 HUD의 고회전 경고 구간 시작점이며 모든 차량의 절대 최적 변속 시점을 강제하는 값이 아님을 명시했다.
- 실제 변속 판정은 차량별 엔진 출력 특성·기어비·Transmission/Drive Runtime이 소유하고 HUD는 실제 Redline 구간을 공통 위치로 시각화한다.
- 남은 RPM 원호 미결정 사항은 Tick 구성이다.

### v0.4.0 - 2026-08-11

- 사용자 결정으로 RPM 원호의 시각 스케일을 차량 공통 고정 방식으로 확정했다.
- 차량별 실제 RPM 범위가 달라도 `RedlineStartRPM`과 `MaximumEngineRPM`을 Presenter에서 HUD용 비율로 변환해 동일한 운전 의미가 항상 같은 게이지 위치에 오도록 했다.
- 실제 Redline 진입 시 모든 차량에서 같은 시각적 Red Zone 시작 위치에 도달하고, 실제 Maximum RPM에서 원호 끝에 도달하도록 계약했다.
- 고정 원호 Tick에 차량별 절대 RPM 숫자를 직접 박지 않도록 했다.
- Red Zone의 정확한 시작 비율과 Tick 개수는 다음 SpeedGauge 세부 결정으로 남겼다.

### v0.3.0 - 2026-08-11

- 사용자 결정으로 SpeedGauge의 원호 의미를 `SpeedKmh` 보조 표시에서 **실제 Engine RPM Tachometer**로 변경했다.
- 큰 디지털 숫자는 속도 전용으로 유지하고 `000`~`999` 3자리, 좌측 정렬, 999 초과도 `999`로 Clamp하도록 확정했다.
- `km/h`는 속도 숫자 아래에 작게 좌측 정렬하도록 확정했다.
- RPM Gauge가 속도를 중복 표시하지 않도록 정보 책임을 분리했다.
- 실제 RPM 값이 연결되지 않은 동안 속도 비율이나 임의 애니메이션으로 가짜 RPM을 표시하지 않도록 했다.
- RPM 최대 Scale, Redline, 차량별 가변 범위와 Tick 구성은 후속 SpeedGauge 세부 설계의 미결정 사항으로 남겼다.
- 현재 Production의 `SpeedArc*` 구현 이름과 생성 자산은 이번 디자인 결정만으로 Rename하지 않았다.

### v0.2.0 - 2026-08-07

- 사용자가 좌하단 VehiclePanel의 최종 시각 구조를 승인했다.
- 패널을 560×416 세로형 초안에서 896×416 가로형 구조로 교체했다.
- 좌측에 원호형 디지털 속도계, 큰 숫자 속도와 세로 D/N/R 표시를 확정했다.
- 참고 계기판의 형태만 채택하고 실제 Runtime이 없는 RPM 표기는 금지했다.
- Armor 차량 실루엣이 왼쪽을 바라보도록 고정하고 화면 위치를 좌=Front, 상=Right, 우=Rear, 하=Left Armor에 바인딩했다.
- 전면·후면·좌측·우측 텍스트를 제거하고 Armor Plate에는 현재/최대 값만 표시하도록 했다.
- 상부 Badge를 Armor 창 좌상단, 하부 Badge를 우하단에 배치했다.
- Shield를 계기판·Armor Cluster와 Vehicle Integrity 사이의 전체 폭 Bar로 확정했다.
- Vehicle Integrity를 최하단 전체 폭 Bar로 확정했다.
- 부품 손상 목록을 고정 VehiclePanel에서 제외하고 후속 Alert·Compact Chip·상세 Panel 후보로 이관했다.
- Visual Layout만 Accepted이며 실제 C++·UMG·해상도 검증은 Pending으로 유지했다.
- 무기창, Source·Unreal Asset·Build·Automation·commit·push는 변경하지 않았다.

### v0.1.0 - 2026-08-07

- 16:9 TPS 좌하단 VehiclePanel의 560×416 상세 규격을 신규 작성했다.
- 속도, D/R/N, Drive State와 Handbrake의 우선순위와 표시 규칙을 정의했다.
- Shield 연속 Bar, Front·Left·Right·Rear Body Map, Top·Bottom 별도 행과 Vehicle Integrity 굵은 Bar를 정의했다.
- 6방향 Armor를 단일 퍼센트로 통합하지 않고 각 방향 Current/Maximum을 고정 위치에서 표시하도록 했다.
- 현재 부품 손상 Runtime과 실제 Gear HUD 계약이 없다는 구현 경계를 반영했다.
- 손상 부품만 최대 3개 표시하는 후속 확장 규칙을 정의했다.
- View Data, Presenter, UMG 책임과 1080p·1440p 검증 기준을 정의했다.
- 무기창, Source·Unreal Asset·Build·Automation·commit·push는 변경하지 않았다.

---

## 18. Migration

- 기존 승인 HUD Slot 위치는 변경하지 않는다.
- 이전 560×416 VehiclePanel 초안과 우측 부품 목록 구조는 폐기한다.
- 신규 구현은 896×416 가로형 구조, 비대칭 RPM Gauge + 디지털 속도계, 중앙 왼쪽 전방 차량 실루엣과 6방향 Armor 우측 세로 Bar, Shield 중간 Bar와 Integrity 최하단 Bar를 사용한다.
- UI-P0-03 구현 시 Drive·Defense·Health Runtime을 읽기 전용으로 변환하고 Widget이 직접 Component를 조회하지 않는다.
- 현재 HUD 공용 실제 Gear Data 계약은 미확인 상태다. Gear Slot 구현 전 Transmission의 실제 기어 상태를 Provider/ViewData에 연결하고, 연결 전에는 가짜 전진 기어 숫자를 표시하지 않는다.
- RPM Gauge는 실제 Engine RPM 값만 소비한다. 실제 RPM Provider 연결 여부는 구현 시 확인하며, 연결 전에는 가짜 RPM을 만들지 않는다.
- 부품 손상 Provider가 구현되어도 고정 VehiclePanel 목록으로 자동 추가하지 않는다.
- VehicleDefense의 Shield·6방향 Armor·Integrity 계산과 이벤트를 UI에 복제하지 않는다.
- Visual Layout Accepted와 실제 UMG Asset·Runtime 구현·해상도 검증 완료를 구분한다.
