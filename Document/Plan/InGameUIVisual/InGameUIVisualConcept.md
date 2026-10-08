# CarFight InGame UI Visual Concept

- 문서 버전: v0.8.1
- 작성일: 2026-08-06
- 최근 갱신일: 2026-08-23
- 문서 상태: Supporting Visual Direction Reference / CF-FQ-032 Historical gate records retained
- 기능 ID: `CF-FQ-032`
- Current Applicability: `CF-FQ-039`에서는 이 문서의 장기 Visual Direction만 Reference로 사용한다. 개별 이미지의 Production Target 승인 범위는 `InGameUIVisualPlan.md` Registry가 별도 소유한다.
- 대표 Plan: `InGameUIVisualPlan.md v0.1.2`
- 상세 구조 설계: `InGameUIDesign.md v0.35.1`
- 제작 규격 초안: `InGameUIStyleSpec.md v0.86.1`
- 1440p Wireframe: `ConceptArt/CFHUDWireframe_1440p.xml`
- Vehicle Panel 상세: `InGameUIVehiclePanelSpec.md v0.20.1`
- Vehicle Panel Wireframe: `ConceptArt/CFVehiclePanel_1440p.xml`
- 로드맵: `InGameUIVisualRoadmap.md v0.1.2`
- 상위 SSOT: `../ProjectSSOT/CombatPlan/14_CombatUI.md`

---

## 1. 목적

이 문서는 CarFight 인게임 HUD와 메뉴가 따라야 할 승인된 시각 콘셉트와 아트 디렉션, 그리고 아직 검증이 필요한 상세 스타일 규격을 정의한다.

기능이 동작하는 것만 확인하는 임시 C++ WidgetTree 외형과, 실제 플레이어가 계속 보게 될 게임 UI의 디자인 기준을 분리한다.

```text
기능 검증용 UI
→ 입력, Pause, Focus, 수명과 데이터 계약 검증

게임용 UI
→ 정보 우선순위, 가독성, 세계관, 상호작용과 화면 일관성까지 충족
```

이 문서의 시각 방향과 디자인 무게, D1-05 Style 기본 Preset과 D1-06 Font·Icon 기본 Preset은 사용자 승인으로 Accepted다. 다만 실제 Style Data·Base Widget·Font·Icon Unreal Asset과 D1-07 인게임 가독성 Static Review는 미완료이므로 `UI-DESIGN-GATE PASS`나 Current System으로 해석하지 않는다.

---

## 2. 상위 확정 전제

다음 항목은 `14_CombatUI.md`와 현재 UI Plan에서 이미 확정된 기반이다.

- 기본 카메라는 외부 3인칭이다.
- 운전석 시점은 사용하지 않는다.
- 포탑 시점은 후속 확장 가능성을 유지한다.
- UI는 장식보다 판독성과 전투 판단을 우선하는 전투용 SF HUD다.
- 미니맵이 아니라 센서 Contact 기반 레이더를 사용한다.
- Shield, Armor와 Vehicle Integrity를 분리한다.
- 선택 타겟 패널은 안정된 고정 영역을 사용한다.
- 미공개 정보는 숨기지 않고 `???`로 표현한다.
- 색상만으로 상태를 전달하지 않는다.
- 게임과 UI 사운드를 사용하지 않으므로 중요한 상태는 화면만으로 이해할 수 있어야 한다.
- 핵심 전투 정보는 중앙 16:9 안전영역에 유지한다.
- CommonUI 도입 여부와 시각 디자인 품질은 별도 Gate로 관리한다.

---

## 3. 콘셉트 정의

### 3.1 공식 콘셉트 이름

```text
차량 탑재형 전술 인터페이스
Vehicle-Borne Tactical Interface
```

### 3.2 한 문장 정의

> CarFight의 UI는 근미래 전투 차량에 탑재된 실용적인 전술 인터페이스로, 군용 장비의 명확성, 현대 차량 계기판의 정돈된 정보 구조와 스타일라이즈드 카툰 렌더링에 어울리는 선명한 평면 그래픽을 결합한다.

### 3.3 핵심 인상

```text
근미래
차량 중심
전술적
기계적
정밀함
절제됨
선명함
빠른 판독
```

### 3.4 디자인 무게

사용자 선택은 `C. 기존 제안 비율 유지`다.

```text
70% 실용적 전투 정보
20% 차량 계기판 정체성
10% 세계관 장식과 스타일
```

적용 원칙:

- 전체 HUD는 실용적 전투 정보와 빠른 판독을 우선한다.
- 속도·주행·차량 상태는 자동차 디지털 계기판의 정돈된 구조를 유지한다.
- 세계관 장식은 정보 의미가 있는 선·형태·전환에만 제한한다.
- 조준·락온·Radar 같은 전투 요소에는 군용 전술성을 국소적으로 강화할 수 있지만 전체 비율을 A 방향으로 변경하지 않는다.
- 장식이 정보보다 먼저 보이거나 전투 상태를 읽는 시간을 늘리면 실패로 판정한다.

---

## 4. 자동차 정체성

CarFight UI는 우주선, 전투기 또는 일반 FPS HUD를 그대로 옮기지 않는다.

자동차 게임의 정체성을 다음 정보와 형태로 유지한다.

- 속도, 전진·후진, 기어와 핸드브레이크는 계기판처럼 명확하게 표시한다.
- 차량 진행 방향과 포탑 방향을 분리한다.
- Shield, Armor와 Vehicle Integrity는 차량 차체 상태로 읽히게 한다.
- 무기 열은 차량 전체 열이 아니라 각 무기 채널의 상태로 표시한다.
- 배터리, 연료와 탄약은 우주선식 통합 에너지로 표현하지 않는다.
- 패널 형태는 항공기 MFD 복제가 아니라 차량용 디지털 클러스터와 견고한 장비 디스플레이를 참고한다.

### 금지되는 정체성 혼합

```text
- 차량 전체를 우주선의 Hull / Energy / Capacitor처럼 표현
- 모든 자원을 하나의 원형 에너지 링으로 통합
- 항공기 조종석의 복잡한 계기와 영문 약어를 그대로 복제
- 일반 FPS의 탄약 숫자와 체력바만 배치하고 차량 상태를 생략
```

---

## 5. 스타일라이즈드 렌더링 호환

월드가 카툰 또는 스타일라이즈드 렌더링으로 표현되더라도 HUD만 지나치게 사실적인 군용 장비처럼 보이지 않게 한다.

권장 방향:

- 텍스처 노이즈보다 단색과 명확한 면 분할을 사용한다.
- 얇은 선을 사용하되 중요한 외곽선은 배경에 묻히지 않게 한다.
- 복잡한 유리 반사, 먼지, 스크래치와 사진 기반 재질을 기본 스타일로 사용하지 않는다.
- 색상 단계와 형태 실루엣을 분명하게 한다.
- 그라디언트는 깊이 또는 값 변화에 필요한 최소 범위에서만 사용한다.
- 월드 외곽선과 경쟁하지 않도록 HUD의 선 밀도를 제한한다.

```text
월드 = 스타일라이즈드 3D 공간
HUD = 선명하고 평면적인 전술 정보층
```

HUD가 월드보다 더 많은 시각 노이즈를 만들면 실패다.

---

## 6. 형태 언어

### 6.1 기본 형태

- 기본 패널은 직사각형을 사용한다.
- 주요 패널은 한두 모서리에만 절삭된 모서리 또는 짧은 사선을 사용한다.
- 모든 모서리를 과도하게 절삭하지 않는다.
- 원과 호는 조준, 락온, 방향, 범위처럼 실제 의미가 있을 때만 사용한다.
- 장식 목적의 원형 회전선과 의미 없는 눈금을 금지한다.

### 6.2 모서리와 반경

1440p 기준 초기 제안:

```text
Small Corner Radius: 2~4 px
Standard Corner Radius: 6 px
Large Menu Corner Radius: 8 px
Chamfer Length: 8~14 px
```

전투 HUD는 작은 반경과 절삭 모서리를 사용하고, Pause와 설정 메뉴는 조금 더 부드러운 반경을 사용할 수 있다.

### 6.3 선 두께

1440p 기준 초기 제안:

```text
Hairline: 1 px
Standard Outline: 2 px
Selected / Focus Outline: 3 px
Critical Accent: 최대 4 px
```

해상도와 DPI Scale에 따라 비율로 확장한다. 1px 이하로 보이는 선은 핵심 정보에 사용하지 않는다.

### 6.4 프레임 밀도

- 모든 정보 블록을 완전한 사각 프레임으로 감싸지 않는다.
- 그룹 구분은 배경 면, 짧은 헤더 선과 여백을 우선한다.
- 중앙 조준 영역에는 완전한 패널을 두지 않는다.
- 화면 주변부의 차량·무기·타겟 정보에만 제한적으로 패널을 사용한다.

---

## 7. 재질과 깊이

### 7.1 기본 재질

```text
어두운 Graphite Surface
+ 제한된 반투명
+ 선명한 텍스트
+ 약한 발광 강조
```

### 7.2 반투명 사용

- 반투명 패널은 정보 그룹 구분에만 사용한다.
- 배경이 복잡한 전투 장면에서도 텍스트가 읽혀야 한다.
- Blur는 기본값으로 사용하지 않는다.
- 전체 HUD에 유리 효과를 적용하지 않는다.
- Pause Menu와 Modal에서만 배경 분리를 위해 제한적으로 어두운 Overlay를 사용한다.

초기 불투명도 제안:

```text
HUD Panel Background: 0.70~0.82
HUD Secondary Surface: 0.45~0.60
Pause Dim Overlay: 0.65~0.78
Decorative Line: 0.45~0.70
```

### 7.3 발광

- 기본 텍스트에는 강한 Glow를 사용하지 않는다.
- 선택, Lock 완료와 Critical 경고에만 제한적으로 사용한다.
- Glow가 글자 획을 뭉개거나 배경을 가리면 제거한다.
- Bloom에 의존하지 않고 색상과 형태만으로도 상태를 읽을 수 있어야 한다.

---

## 8. 색상 시스템

아래 값은 `DA_CFUIStyle` 제작 전 초기 디자인 토큰 제안이다. 실제 월드 배경과 모니터에서 검증 후 조정한다.

### 8.1 중립 색상

| Token | 초기 제안 | 용도 |
|---|---:|---|
| `SurfaceBase` | `#0B1117` | 기본 패널 배경 |
| `SurfaceRaised` | `#111B24` | 선택 패널과 상위 표면 |
| `SurfaceOverlay` | `#17232D` | Modal과 보조 Overlay |
| `LineDefault` | `#577080` | 일반 구분선 |
| `TextPrimary` | `#EAF2F7` | 핵심 수치와 주요 문구 |
| `TextSecondary` | `#A3B2BC` | 레이블과 보조 정보 |
| `TextDisabled` | `#61717C` | 비활성·Unavailable |

### 8.2 핵심 강조색

| Token | 초기 제안 | 의미 |
|---|---:|---|
| `AccentTactical` | `#55C7E8` | 선택, 시스템 강조, 조준 관련 정상 상태 |
| `StateNotice` | `#72C7D9` | 정보 공개와 일반 알림 |
| `StateCaution` | `#F1B84B` | 진행 중, 불안정, 주의 |
| `StateDanger` | `#FF714D` | 손상, 실패 위험, 즉각 대응 필요 |
| `StateCritical` | `#FF3F46` | 치명적 상태와 긴급 경고 |
| `StateDisabled` | `#65727B` | 비활성, 파괴, 사용 불가 |

### 8.3 방어 계층 색상

| 계층 | 기본 색상 | 추가 구분 |
|---|---:|---|
| Shield | `#59D5E6` | 육각 또는 에너지 아이콘, 얇은 연속 바 |
| Armor | `#D6A448` | 방패·판재 아이콘, 분절형 바 |
| Vehicle Integrity | `#F0644F` | 차체 아이콘, 굵은 연속 바 |

색상만으로 계층을 구분하지 않는다. 라벨, 아이콘과 게이지 형태를 함께 사용한다.

### 8.4 관계 색상

| 관계 | 기본 색상 |
|---|---:|
| Friendly | `#53A9FF` |
| Neutral | `#D5C777` |
| Hostile | `#FF644F` |
| Unknown | `#A7ADB3` |
| Selected | 관계색 유지 + `AccentTactical` 외곽선 |

선택 상태 때문에 적대·아군 관계색이 사라지지 않게 한다.

### 8.5 색상 금지 규칙

- 정상 상태를 모든 영역에서 초록색으로 통일하지 않는다.
- 적대와 Critical을 동일한 표현 하나로 합치지 않는다.
- 빨강과 초록만으로 성공·실패를 구분하지 않는다.
- Rainbow Gradient와 다수의 네온 Accent를 사용하지 않는다.
- 같은 색이 관계, 피해 계층과 입력 Focus를 동시에 의미하지 않게 한다.

---

## 9. Typography

### 9.1 방향

- 한글은 정돈된 산세리프 계열을 사용한다.
- 수치 정보는 Tabular Number 또는 Monospaced Number를 지원하는 폰트를 우선한다.
- 너무 좁고 장식적인 군용 스텐실 폰트를 본문에 사용하지 않는다.
- 영문 대문자는 짧은 상태명과 장비명에 제한적으로 사용한다.
- 한글 문장을 억지로 영문 약어로 변환하지 않는다.

### 9.2 기준 크기

1440p 기준 초기 제안:

| Token | 크기 | 용도 |
|---|---:|---|
| `DisplayNumberLarge` | 44~52 px | 속도와 핵심 수치 |
| `DisplayNumberMedium` | 28~34 px | 탄약·거리·주 방어 수치 |
| `Heading` | 24~28 px | Pause 제목과 주요 패널 제목 |
| `Body` | 18~20 px | 일반 메뉴와 상태 문구 |
| `Label` | 15~17 px | HUD 레이블과 단위 |
| `Caption` | 13~15 px | 보조 설명과 입력 힌트 |

최소 글자 크기는 실제 1440p 화면에서 사용자가 정상 거리에서 읽을 수 있는지 검증한다. 32:9라고 해서 글자를 화면 끝으로 보내거나 작게 만들지 않는다.

### 9.3 숫자 규칙

- 속도와 거리 숫자는 자리 이동이 적은 고정 폭 표현을 우선한다.
- 단위는 수치보다 작게 표시한다.
- 불필요한 소수점은 제거한다.
- 퍼센트와 절대값을 동시에 표시할 때 정보 우선순위를 정한다.
- `0`, `Unknown`, `Unavailable`을 서로 다른 텍스트로 표현한다.

```text
Known Zero → 0
Unknown → ???
Unavailable → N/A 또는 비활성 라벨
```

---

## 10. 간격과 크기 시스템

### 10.1 Spacing Grid

4px 기반 간격을 사용한다.

```text
4 / 8 / 12 / 16 / 24 / 32 / 48
```

- 같은 그룹 내부: 4~8
- 관련 행 간격: 8~12
- 그룹 사이: 16~24
- 주요 패널 외곽: 24~32

### 10.2 상호작용 크기

1440p 기준 초기 제안:

```text
최소 클릭·Focus 영역: 48 × 48 px
일반 메뉴 버튼 높이: 56~64 px
주요 Pause 버튼 높이: 64~72 px
주요 Pause 버튼 폭: 320~380 px
```

텍스트가 보이더라도 클릭 영역이 작아 버튼으로 인식되지 않는 구조를 금지한다.

---

## 11. Iconography

### 11.1 D1-06 승인 스타일

기본 Icon Style은 `Solid Core + Tactical Cut`으로 확정한다.

```text
Base Grid = 24 × 24
Default IconSmall = 16
Default IconMedium = 20
Default IconLarge = 28
Weapon Compact = 18
Weapon Selected = 20
```

제작 원칙:

- 단색 또는 최대 두 단계 명도를 사용한다.
- 외부 실루엣을 강하고 단순하게 유지하고 내부 Detail은 0~2개 수준으로 제한한다.
- 작은 Chamfer·Notch·절개로 차량 탑재형 전술 장비의 기계적 느낌을 부여한다.
- 실루엣만으로 의미를 구분할 수 있어야 하며 색상만으로 의미를 전달하지 않는다.
- 16~20 크기에서 내부 선이 뭉개지지 않아야 한다.
- 동일 카테고리는 동일한 선 두께와 시점 규칙을 사용한다.
- Gradient, Drop Shadow, 장식용 Glow와 의미 없는 보조 Glyph를 기본 Icon에 넣지 않는다.

### 11.2 Semantic Icon Set

```text
Vehicle
Shield
Armor
Integrity
Engine
Steering
Turret
Ammo
Battery
Charge
Heat
Cooldown
Reload
Target
Lock
RadarContact
Warning
Critical
```

Gameplay·Presenter·Widget은 실제 Texture/Vector 경로 대신 Semantic Icon ID를 사용한다. 실제 Brush·Vector·Texture는 교체 가능한 Icon Set Data가 소유한다.

### 11.3 방어 Icon 승인 형태

```text
Shield
→ Energy Field / Hex·Arc 실루엣

Armor
→ Chamfered Plate / 장갑 판재 실루엣

Integrity
→ Vehicle Chassis / 차체 구조 실루엣
```

Shield를 중세 방패 모양으로 만들지 않는다. 색을 제거해도 Shield·Armor·Integrity 세 계층을 형태로 구분할 수 있어야 한다.

### 11.4 Weapon Icon 승인 형태

Weapon Icon은 작은 크기 판독성을 위해 기본적으로 Side/Profile Silhouette를 사용한다.

```text
Ballistic
→ Barrel + Receiver

Rail / Energy
→ 긴 직선 Body + Energy Gap

Launcher
→ 다중 Tube / Pod

Defense Device
→ Projector / Field Generator

Utility
→ Sensor / Pulse Device
```

무기마다 임의의 화풍을 사용하지 않고 동일한 Base Grid·Stroke·Notch 규칙을 따른다.

### 11.5 Target·Lock 승인 형태

```text
Selected Target
→ 기존 관계색 Marker 유지
→ Cyan Outer Corner / Bracket 추가

Lock Progress
→ Target 중심을 감싸는 2~4개의 독립 Arc 또는 Corner Segment
→ 진행에 따라 Segment가 닫힘
→ 완료 시 1회 Snap 후 안정 상태
```

Hostile을 선택했다고 관계색 Red를 Cyan으로 대체하지 않는다. 선택 상태와 관계 상태를 동시에 읽을 수 있어야 한다.

### 11.6 Radar Contact 승인 기본 Preset

| 상태 | 기본 Symbol |
|---|---|
| Friendly | Circle |
| Neutral | Square |
| Hostile | Diamond |
| Unknown | Hollow Diamond 또는 `?` Notch |
| Selected | 기존 Symbol + Cyan Outer Bracket |
| Last Known | 점선 또는 낮은 Alpha |
| Off-screen | 방향 Chevron |

Radar Symbol은 D1-07 Static Review에서 1080p·1440p 판독성을 확인해 Icon Set Data 값만 보정할 수 있다.

### 11.7 외부 Icon 패키지 정책

- 라이선스가 불명확한 외부 아이콘을 최종 자산으로 사용하지 않는다.
- 외부 패키지를 참고하거나 원본으로 사용하더라도 CarFight Semantic Icon Set의 시점·실루엣·Stroke 규칙에 맞춰 재가공한다.
- Icon Set 전체 교체가 Gameplay·Presenter·View Data 변경을 요구하면 설계 위반이다.

---

## 12. 모션과 애니메이션

### 12.1 기본 원칙

- 상태 변화가 있을 때만 애니메이션을 사용한다.
- 지속적인 Scanline, Glitch, Noise와 의미 없는 회전을 금지한다.
- 전투 중 UI 애니메이션이 조준과 타겟 추적을 방해하지 않게 한다.
- Pause 중 메뉴 애니메이션은 실제 World Pause와 독립적으로 실행 가능해야 한다.

### 12.2 초기 시간 기준

| Motion | 시간 |
|---|---:|
| Hover / Focus 전환 | 80~120 ms |
| Pressed 반응 | 60~100 ms |
| 작은 HUD 상태 전환 | 120~180 ms |
| 패널 등장·퇴장 | 160~240 ms |
| 경고 Pulse 1회 | 300~450 ms |
| 게이지 보간 | 120~250 ms |

### 12.3 경고

- Notice는 1회 강조 후 안정 상태로 돌아간다.
- Warning은 제한된 Pulse를 사용한다.
- Critical만 반복 Pulse를 허용한다.
- 반복 Pulse는 1~2 Hz를 넘지 않는다.
- 화면 전체 Flash는 플레이어 즉시 사망 또는 극단적 상황 외에는 사용하지 않는다.
- 점멸과 함께 아이콘, 문구와 형태 변화가 있어야 한다.

---

## 13. 화면 정보 밀도

### 13.1 중앙 조준 영역

```text
가장 비워 두어야 하는 영역
```

허용:

- 플레이어 조준 레티클
- 무기 실제 조준 레티클
- Lock 진행
- 조준 불가와 사거리 상태
- 짧은 즉시 경고

금지:

- 큰 패널
- 장시간 유지되는 설명 문구
- 차량 전체 상태 목록
- 장식 프레임

### 13.2 중앙 인접 영역

- 선택 대상 마커
- Lock 실패 이유
- 피격 방향
- 화면 밖 접촉 방향

짧고 즉시 읽히는 형태만 사용한다.

### 13.3 좌측 하단

- 내 차량 정보
- 속도와 기어
- Shield와 Vehicle Integrity
- 상·하·전·후·좌·우 Armor
- 주행 상태와 손상 부품 요약

상세 초안은 `InGameUIVehiclePanelSpec.md`가 소유한다. 속도와 D/R/N은 상단, Shield는 얇은 연속 Bar, Front·Left·Right·Rear Armor는 Top View Body Map, Top·Bottom Armor는 우측 별도 행, Integrity는 최하단 굵은 Bar로 구성한다. 부품 손상은 실제 Provider가 생긴 뒤 손상된 항목만 표시한다.

### 13.4 하단 중앙

- 센서 Contact 기반 전투 레이더
- Heading Up
- 플레이 차량 기준 방향과 선택 Target 강조

Elite Dangerous의 중앙 하단 공간 인지 방식을 참고하되 우주선식 자원 UI는 복제하지 않는다.

### 13.5 우측 하단

- 내 무기 정보
- 현재 선택 무기 강조
- 비선택 무기 축약 목록
- 탄약·충전·열·쿨다운·재장전 상태

무기창의 정확한 정보 밀도와 자원 채널은 별도 상세 설계에서 확정한다.

### 13.6 우측 상단

- 선택 타겟 패널
- 차량 이미지 없이 Compact 정보로 시작
- 식별·스캔 진행에 따른 점진 정보 공개
- Lock 상태

### 13.7 상단

- 좌상단: 즉시 필요한 임무 목표
- 상단 중앙: 시스템 알림과 Critical 경고

상단을 일반 로그 영역으로 사용하지 않는다. 중앙 조준 영역은 외부 3인칭 TPS 시야를 위해 비워 둔다.

---

## 14. HUD 요소별 콘셉트

### 14.1 Aim Reticle

- 중앙 점과 무기 실제 조준점을 분리한다.
- 선은 짧고 명확하게 유지한다.
- 정상 상태는 얇은 `AccentTactical` 계열을 사용한다.
- 조준 불가 시 색상뿐 아니라 선 분리, 끊김 또는 금지 기호를 사용한다.
- 발사 성공 애니메이션은 짧고 중앙 시야를 덮지 않는다.
- 쿨다운 진행을 레티클 전체 원형 Progress로 고정하지 않고 무기 특성에 따라 보조 표시할 수 있다.

### 14.2 Lock Indicator

- 선택 마커와 Lock 진행을 같은 도형 하나로 합치지 않는다.
- Lock 진행은 Target 중심을 감싸는 2~4개의 Arc 또는 Corner Segment를 사용한다.
- 완료 시 1회 Snap 또는 밝기 강조 후 안정 상태로 전환한다.
- 실패 이유는 짧은 문구와 상태 아이콘으로 표시한다.

### 14.3 Vehicle Defense

- Shield는 얇은 연속 Bar와 재생 대기·재생 중·소진 상태를 사용한다.
- Front·Left·Right·Rear Armor는 차량 Top View Body Map의 고정 Plate로 표시한다.
- Top·Bottom Armor는 Body Map과 겹치지 않는 우측 별도 행으로 표시한다.
- 각 방향 Armor는 전체 평균이 아니라 `현재/최대` 값을 독립 표시한다.
- Integrity는 패널 최하단의 가장 굵은 연속 Bar를 사용한다.
- Stable·Caution·Critical·Broken은 색상과 함께 Plate 파손 형태·X·문구를 사용한다.
- 부품 상태는 실제 Runtime이 제공된 뒤 손상된 항목만 최대 3개 표시한다.
- 세 계층을 동일한 모양의 색만 다른 체력바 세 개로 만들지 않는다.

### 14.4 Speed Cluster

- 속도 숫자를 가장 크게 표시한다.
- 단위와 기어는 보조 크기로 둔다.
- 원형 아날로그 속도계를 기본으로 사용하지 않는다.
- 차량 조작 상태는 작은 아이콘과 짧은 레이블로 표시한다.

### 14.5 Weapon Panel

- 무기 이름과 그룹을 상단에 둔다.
- Resource Channel은 같은 행 규칙을 재사용한다.
- 탄약, 충전, 열과 쿨다운을 동일 색상·동일 게이지로 표현하지 않는다.
- 사격 불가 이유를 아이콘과 짧은 문구로 표시한다.

### 14.6 Target Panel

- 패널 크기와 행 위치를 고정한다.
- 정보가 공개되어도 행이 추가되며 전체 패널이 흔들리는 구조를 피한다.
- `???`, 추정값과 확인값의 표현 차이를 명확하게 한다.
- 대상 내부 Actor 이름과 디버그 문자열을 플레이어 UI에 노출하지 않는다.

### 14.7 Radar

- 원형 또는 둥근 정사각형 영역을 사용할 수 있다.
- Heading Up 방향을 기본으로 한다.
- 지형 텍스처를 표시하지 않는다.
- Contact는 관계, 식별, 선택과 마지막 탐지 상태를 형태와 외곽선으로 함께 구분한다.
- 선택 타겟은 크기 증가보다 외곽선과 중심 표식을 우선한다.

### 14.8 Alert

- Alert는 `Notice`, `Warning`, `Critical` 위계를 사용한다.
- 같은 `AlertKey`를 반복해서 쌓지 않는다.
- 최대 동시 표시 개수를 제한한다.
- Critical은 상단 중앙 또는 중앙 인접 영역에서 짧게 강조하고, 지속 상태는 주변 상태 패널로 이동한다.

### 14.9 Pause Menu

- Pause Menu는 전투 HUD보다 안정적이고 큰 형태를 사용한다.
- 가장 중요한 기본 동작을 한 개의 Primary Button으로 명확히 보여준다.
- 버튼은 배경 면, 테두리, 문구, Hover·Focus 상태와 충분한 클릭 영역을 가진다.
- 입력 힌트는 버튼 아래 보조 텍스트로 표시한다.
- Pause Overlay는 HUD를 파괴하지 않고 시각적으로 뒤로 밀어낸다.

---

## 15. 메뉴 상호작용 상태

모든 상호작용 Widget은 최소 다음 상태를 구분한다.

```text
Normal
Hover
Focused
Pressed
Disabled
```

### 상태 표현 초안

| 상태 | 표현 |
|---|---|
| Normal | 어두운 Surface + 기본 Outline |
| Hover | Surface 밝기 증가 + 약한 Accent Line |
| Focused | 3px Accent Outline + 방향 Marker 또는 Icon |
| Pressed | 짧은 명도 감소 또는 0.98 Scale 반응 |
| Disabled | 낮은 대비 + Disabled Icon·Text, Hover 없음 |

게임패드 Focus는 마우스 Hover와 구분되지만 동일한 디자인 언어를 사용한다.

---

## 16. 해상도와 안전영역

### 16.1 기준 캔버스

디자인 기준 해상도는 `2560×1440`으로 둔다.

- 1920×1080은 축소 검증한다.
- 3440×1440과 5120×1440은 중앙 16:9 핵심 영역을 유지한다.
- 핵심 HUD를 울트라와이드 화면 끝에 고정하지 않는다.

1440p 기준 중앙 핵심 영역:

```text
2560 × 1440 중앙 영역
```

외곽 영역은 비필수 정보, Debug와 보조 로그에만 사용한다.

### 16.2 Safe Margin

초기 제안:

```text
화면 외곽 최소 Safe Margin: 48 px
핵심 HUD 내부 Margin: 64~96 px
중앙 조준 보호 영역: 화면 폭 32~40%, 화면 높이 28~36%
```

정확한 값은 실제 차량 크기와 카메라 FOV에서 조정한다.

---

## 17. 금지 스타일

다음 표현은 CarFight 기본 UI 콘셉트로 사용하지 않는다.

```text
- 과도한 사이버펑크 네온
- Rainbow Accent와 다색 발광
- 화면 전체를 덮는 홀로그램 장식
- 지속 Glitch, Scanline, Noise와 CRT 왜곡
- 의미 없이 회전하는 원과 눈금
- 모바일 RPG식 두꺼운 장식 프레임
- 사진 기반 금속·스크래치 텍스처 남용
- 지나치게 작은 군용 계기판 글자
- 영문 약어만 가득한 항공기 MFD 복제
- 모든 상태를 원형 Progress Ring으로 표현
- 모든 패널에 Blur와 강한 Glass 효과 적용
- 경고 상태를 빠른 화면 전체 점멸로만 표현
```

---

## 18. 구현 구조 매핑

### C++

- View Data
- 상태 Enum
- 정보 공개 상태
- Alert 우선순위
- Presenter / Provider
- 스타일 토큰을 소비할 안정된 API

### Blueprint / UMG

- `WBP_CFUIRoot`
- 레이아웃과 Safe Zone
- `WBP_CFButtonBase`
- `WBP_CFPanelBase`
- `WBP_CFStatusBar`
- `WBP_CFInfoRow`
- `WBP_CFAlertItem`
- 상태별 애니메이션
- Style Data 적용

### Style Data

초기 후보:

```text
DA_CFUIStyle
```

권장 그룹:

```text
Color Tokens
Typography Tokens
Spacing Tokens
Shape Tokens
Motion Tokens
HUD Element Styles
Menu Element Styles
```

Widget마다 색상, 폰트 크기, 여백과 전환 시간을 개별 하드코딩하지 않는다.

---

## 19. UI-DESIGN-GATE 종료 조건

다음 조건을 모두 충족해야 Gate를 PASS할 수 있다.

1. 공식 콘셉트 한 문장이 사용자 승인을 받는다.
2. 핵심 키워드와 금지 스타일이 승인된다.
3. 색상 토큰의 의미와 초기 팔레트가 승인된다.
4. Typography 위계와 기준 해상도가 확정된다.
5. Panel, Button, Status Bar와 Info Row의 공통 형태가 결정된다.
6. Reticle, Vehicle Defense, Weapon, Target, Radar와 Alert의 화면 우선순위가 정해진다.
7. 16:9·21:9·32:9 배치 규칙이 확정된다.
8. 최소 한 장의 HUD Wireframe 또는 Style Board가 검토된다.
9. Pause Menu의 Primary Button이 설명 없이 상호작용 요소로 식별된다.
10. 디자인 품질과 CommonUI 도입 여부가 별도 결정으로 유지된다.

---

## 20. 현재 미결정 항목

다음 항목은 D1-06 승인 뒤에도 후속 검증이 필요하다.

- Pretendard·IBM Plex Mono의 실제 배포 라이선스와 게임 패키지 재배포 조건 최종 확인
- Unreal Font Asset Import와 Fallback Font 구성
- Solid Core + Tactical Cut Icon의 실제 Vector/Texture 제작 포맷과 제작 파이프라인
- 인게임 색상 최종 보정
- Reticle의 정확한 실루엣
- 월드 카툰 렌더링 강도에 따른 HUD 외곽선 강도
- UI Material과 Post Process 영향
- D1-07 실제 Screen Mockup과 해상도별 판독성

이 항목은 사용자 검토, Wireframe과 인게임 캡처 비교를 거쳐 확정한다.

---

## 21. 사용자 검토 상태

### 시각 방향 승인 완료

```text
1. 차량 탑재형 전술 인터페이스: 승인
2. 스타일라이즈드 카툰 렌더링에 어울리는 평면 UI: 승인
3. Cyan 중심 Accent + Amber·Red 상태색: 승인
4. 디자인 무게 C / 70:20:10 균형: 승인
```

### UI-DESIGN-GATE 현재 제작 체크포인트

```text
InGameUIStyleSpec.md v0.10.0: D1-05 Style + D1-06 Font·Icon User Accepted
CFHUDWireframe_1440p.xml: TPS 16:9 Placement Accepted
CFVehiclePanel_1440p.xml: Vehicle Panel Accepted Layout
InGameUIWeaponPanelSpec.md v0.5.0: Weapon Compact Token User Accepted
Accepted Slots: Mission TL / Alert TC / Target TR / Vehicle BL / Radar BC / Weapon BR
Font: Pretendard + IBM Plex Mono Default Preset
Icon: Solid Core + Tactical Cut / Semantic Icon Set
DA_CFUIStyle·Font·Icon Unreal Asset: Not Started
Base Widget Unreal Asset: Not Started
D1-07 1080p·1440p·21:9·32:9 Static Review: Not Run
```

시각 방향은 Accepted지만 위 산출물과 검증이 완료되기 전에는 `UI-DESIGN-GATE PASS`로 승격하지 않는다.

---

## 22. Changelog

### v0.8.0 - 2026-08-07

- 사용자가 D1-06 Font·Icon 추천안을 전부 승인했다.
- 기본 UI Font를 Pretendard, 숫자·기술 수치 Font를 IBM Plex Mono로 확정하고 Font Family Role을 Style Data에서 교체 가능하도록 했다.
- Icon Style을 `Solid Core + Tactical Cut`, 24×24 Base Grid와 16/20/28 기본 크기, Weapon 18/20으로 확정했다.
- Shield=Energy Field, Armor=Chamfered Plate, Integrity=Vehicle Chassis, Weapon=Side/Profile Silhouette를 Semantic Icon 기본 형태로 승인했다.
- Selected Target은 관계색 Marker + Cyan Outer Bracket, Lock은 독립 Arc/Corner Segment, Radar는 관계색 + 기하 Symbol 조합으로 승인했다.
- 외부 Icon 패키지는 직접 고정 참조하지 않고 Semantic Icon Set 규칙에 맞춰 재가공·교체할 수 있도록 했다.
- 실제 Font·Icon Unreal Asset 생성, 라이선스 최종 확인과 D1-07 Static Review는 수행하지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.7.0 - 2026-08-07

- 사용자가 좌하단 VehiclePanel의 가로형 최종 Visual Layout을 승인했다.
- 원호형 속도계, 세로 D/N/R, 왼쪽 전방 차량 Armor 실루엣과 방향별 값 배치를 확정했다.
- 상부 좌상단·하부 우하단 Badge, Shield 중간 Bar와 Integrity 최하단 Bar를 확정했다.
- 부품 손상 목록은 고정 VehiclePanel에서 제외했다.
- 다음 디자인 상세 단계를 우하단 D1-WEAPON-PANEL로 이동했다.
- 실제 UMG·Runtime·해상도 검증과 UI-DESIGN-GATE PASS는 수행하지 않았다.

### v0.6.0 - 2026-08-07

- `InGameUIVehiclePanelSpec.md v0.1.0`과 `CFVehiclePanel_1440p.xml`을 VehiclePanel 상세 초안으로 연결했다.
- 속도·D/R/N·Drive State·Handbrake, Shield, 6방향 Armor, Integrity와 조건부 부품 손상의 시각 우선순위를 정의했다.
- Front·Left·Right·Rear Armor는 Top View Body Map, Top·Bottom은 우측 별도 행으로 분리했다.
- 현재 실제 Gear 번호와 부품 손상 Runtime이 없으므로 가짜 값을 표시하지 않는 경계를 추가했다.
- Vehicle Panel은 Draft / User Review Pending이며 Weapon Panel은 다루지 않았다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.5.0 - 2026-08-07

- 기본 플레이 시점을 외부 3인칭 차량 TPS, 기본 UI 화면비를 16:9로 확정했다.
- HUD 위치를 좌상단 Mission, 상단 중앙 Alert, 우상단 Target, 좌하단 Vehicle, 하단 중앙 Radar, 우하단 Weapon으로 승인했다.
- Target Panel은 차량 이미지 없이 Compact하게 시작하고 Scan 진척에 따라 정보를 공개하도록 유지했다.
- 내 차량 정보와 무기창은 위치만 승인하고 내부 정보 구조를 각각 별도 상세 설계 Pending으로 분리했다.
- 향후 사용자 HUD 배치 커스터마이징을 고려하되 현재 P0에서는 기본 Layout Profile만 사용하도록 경계를 추가했다.
- UI-DESIGN-GATE 전체 PASS, Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.4.0 - 2026-08-06

- `InGameUIStyleSpec.md v0.1.0`을 생성해 DA_CFUIStyle 상세 Token과 Base Widget 5종의 제작 규격을 분리했다.
- `ConceptArt/CFHUDWireframe_1440p.xml`에 2560×1440 좌표 기반 HUD Wireframe 초안을 작성했다.
- 좌측 Vehicle Defense·Radar, 중앙 Reticle Protection·Speed, 우측 Target·Weapon 정보 Rail을 정의했다.
- Radar는 Rounded Square Panel 안의 Circular Plot Area 조합을 초안으로 제안했다.
- Style Spec과 Wireframe은 사용자 검토 Pending이며 Unreal Asset·실제 화면 검토가 없으므로 UI-DESIGN-GATE PASS는 보류했다.
- CommonUI 판단은 다루지 않았고 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.3.0 - 2026-08-06

- 사용자가 디자인 무게 `C. 기존 70:20:10 균형 유지`를 승인했다.
- 실용적 전투 정보 70%, 차량 계기판 정체성 20%, 세계관 장식 10%를 공식 아트 디렉션 비율로 확정했다.
- 조준·락온·Radar에는 전술성을 국소 강화할 수 있지만 전체 UI를 군용 HUD 중심으로 변경하지 않도록 경계를 명시했다.
- 콘셉트·평면 스타일·Palette·디자인 무게를 Visual Direction Accepted로 승격했다.
- Style Data, Base Widget, Font·Icon, Wireframe과 해상도별 검증은 Pending이므로 UI-DESIGN-GATE 전체 PASS는 보류했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.2.0 - 2026-08-06

- 사용자가 `차량 탑재형 전술 인터페이스` 콘셉트를 승인했다.
- 스타일라이즈드 카툰 렌더링과 어울리는 평면 UI 방향을 승인 상태로 기록했다.
- Cyan 중심 Accent와 Amber·Red 상태색 방향을 승인 상태로 기록했다.
- 디자인 무게 A·B·C 선택은 응답에서 확정되지 않아 Pending으로 유지했다.
- 기존 70:20:10 비율을 승인된 값으로 오인하지 않도록 디자인 무게 절을 미결정 선택지로 변경했다.
- UI-DESIGN-GATE와 상위 SSOT Accepted 승격은 디자인 무게 선택, Wireframe과 Style 검토 전까지 금지한다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.1.0 - 2026-08-06

- CarFight 인게임 UI의 첫 구체적 시각 콘셉트 Working Draft를 작성했다.
- 공식 콘셉트 후보를 `차량 탑재형 전술 인터페이스`로 정의했다.
- 근미래 차량·전술·스타일라이즈드 렌더링의 결합 방향을 제안했다.
- 형태, 재질, 색상, Typography, 간격, Icon, Motion과 화면 밀도 규칙을 정의했다.
- Reticle, Lock, Vehicle Defense, Speed, Weapon, Target, Radar, Alert와 Pause Menu별 디자인 원칙을 추가했다.
- 16:9·21:9·32:9 Safe Zone과 UI-DESIGN-GATE 종료 조건을 정의했다.
- 실제 Font, Icon, 최종 Palette와 Wireframe은 사용자 검토 Pending으로 남겼다.
- 문서만 작성했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

---

## 23. Migration

- v0.8.0부터 D1-06 Font·Icon 기본 Preset은 User Accepted다. 실제 Font 파일과 Icon Brush 경로는 Widget에 직접 고정하지 않고 Style/Icon Set Data가 소유한다.
- 기본 Font Family는 Pretendard + IBM Plex Mono이며 실제 Asset Import 전 라이선스·재배포 조건을 최종 확인한다.
- Icon은 Semantic ID와 교체 가능한 Icon Set을 사용하고 Shield·Armor·Integrity·Weapon·Target·Lock·Radar의 승인 형태를 기본 Preset으로 적용한다.
- D1-07 Static Review에서 판독성 문제가 생기면 Semantic 의미를 유지한 채 Font/Icon Data만 보정한다.
- 이 문서의 시각 방향은 Accepted지만 실제 Unreal Asset과 Static Review 없이 기존 UI 에셋에 일괄 적용하지 않는다.
- UI-P0-03~05 데이터·수명 작업은 기존 시각을 보존한 채 진행할 수 있다.
- UI-P0-06 신규 HUD 제작 전에 승인된 콘셉트와 `DA_CFUIStyle` 계약을 사용한다.
- 현재 C++ Pause Menu는 기능 검증 기준으로 유지하며 최종 Pause Menu Blueprint 제작 시 본 문서의 버튼·패널·Focus 규칙을 적용한다.
- CommonUI 도입 여부는 `UI-COMMONUI-GATE`가 별도로 소유한다.
