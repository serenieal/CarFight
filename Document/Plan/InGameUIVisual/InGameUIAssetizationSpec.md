# CarFight InGame UI Assetization Spec

- 문서 버전: v0.16.2
- 작성일: 2026-08-07
- 최근 갱신일: 2026-08-24
- 문서 상태: Historical Assetization Evidence + Current Production Structure Reference
- 기능 ID: `CF-FQ-032`
- Current Applicability: D1-09~12의 구조·Migration·Fallback 계약만 CF-FQ-039 supporting reference로 재사용한다. 과거 ART/UI-P0 진행 수치와 Scaffold/Build/Apply 절차는 Historical이며 CF-FQ-039의 Visual 제작 방식으로 재사용하지 않는다. 현재 구현 상태는 `Systems/UI/InGameUI.md`, 현재 Visual 작업과 유일한 Production 제작 순서는 `InGameUIVisualPlan.md`의 Target Decomposition Pipeline이 우선한다.
- 상위 Plan: `InGameUIVisualPlan.md v0.1.10`
- Style SSOT: `InGameUIStyleSpec.md v0.86.1`
- Static Review SSOT: `InGameUIStaticReview.md`
- 현재 기준 화면: `1920×1080 / 16:9 / SR-1080-16 USER PASS`

---

## 1. 목적과 작업 경계

이 문서는 `UI-DESIGN-GATE`의 D1-09A부터 D1-12까지 당시 실행했던 파일, 클래스, Unreal Asset, 기본값, Widget Tree, 검증과 중단 조건을 Historical evidence와 구조 Reference로 보존한다.

> **CF-FQ-039 Visual Production Lock:** 이 문서의 과거 `Prototype/Scaffold → Apply → Art 보정` 절차를 Current Visual 제작 경로로 재실행하지 않는다. CF-FQ-039은 `Production Visual Target Lock → Logical Breakdown → WBP Composition Skeleton → Slot Contract Review → Physical Art Breakdown → UMG Assembly` 순서만 사용한다. 이 문서는 WBP/Asset 구조·Fallback·Migration 판단에만 참고하며 Visual authority를 소유하지 않는다.

이번 준비 단계의 허용 범위:

```text
허용
- Font 라이선스와 Unreal Font API 외부 근거 확인
- 실제 CarFight 소스의 현재 구조 확인
- D1-09A~D1-12 구현 계약 문서화
- Plan / Roadmap / ActiveWork 체크포인트 동기화

금지
- 추가 C++ Source 수정
- Config 수정
- Unreal Asset 생성·수정
- Build·Automation 실행
- PIE 실행
- UI-P0-03~09 기능 구현
- 1440p·21:9·32:9 Layout Profile 구현
- commit·push
```

2026-08-10 D1-08V·D1-09A·D1-09B·D1-10은 PASS 상태를 유지한다. D1-11은 네 번의 사용자 Preview에서 FAIL했고 v1.x flattened `Canvas_Mock_*` + Border Mosaic 제작 경로는 active path에서 폐기했다. Production 구조는 Root 1개 + 기능 Panel 6개 + 의미 Element 2개 + HUD Visual DataAsset 1개로 실제 자산화됐다. Root Canvas는 D1-07 승인 Slot 배치만 담당하고, Panel/Element 내부 시각은 TextBlock, Image/Brush, ProgressBar와 의미 단위 하위 Widget이 소유한다. `UCFUIStyleData v1.2.0`은 기존 D1-09B Semantic Icon 18개를 Semantic ID로 해석하며 별도 Icon DataAsset을 만들지 않는다. `UCFHUDVisualData v1.0.0`은 Vehicle Silhouette·6방향 Armor Plate·Speed Arc·Radar Frame/Sweep·Selected Target/Weapon Frame 같은 HUD 전용 비Semantic 아트 교체 슬롯만 소유한다. Production Bridge/Assetization 자동 검증과 기존 보호 회귀 PASS 증거는 유지한다. 2026-08-12 Pre-Preview Contract Review에서 확인한 `Text_Gear="D"`, 6방향 Armor Current/Maximum Text, `ProgressBar_SpeedFallback`·Cooldown Semantic Speed Arc Fallback은 `CFUIHUDProdEditorBridge v1.1.0`에서 D1-11 범위로 최소 교정했다. SpeedGauge는 `Text_Gear="N"`의 단일 Gear Slot, 실제 RPM 값과 독립된 고정 21 Tick 비대칭 Tachometer Scale, 85%부터 Red Zone, 실제 HUDVisualData `SpeedArcTrack`만 허용하는 `Image_RPMTrackArt`로 재구성했으며 가짜 RPM/Speed 비율은 제거했다. ArmorBodyMap은 6방향 Current/Maximum Text를 모두 제거하고 각 Plate/Badge 우측에 `BottomToTop` 세로 `ProgressBar_Armor*`를 추가했다. 공식 UE 5.8 Build `9416a5877d8e47a5a80ac552d5e7e76d`, Production Apply `a3fe8f3981f542c483d607d7e88eca93`, Saved Readback `9972afbf11df44a7aac5d731e9010a3a`가 Exit 0으로 PASS했고 Readback은 exact 10 Asset·Root 1·Panel 6·Element 2·HUD Visual Data 1, Border Mosaic 0, Gameplay Cast 0, Runtime Binding 0, D1-12 0, UI-P0-03 0을 유지했다. Fresh AssetDump dataset `adset_v1_b17fbe9392114d7f155e6c74ad48f141.4ee866a52a57c78493666817`은 WidgetBlueprint 9/9 Success, SpeedGauge의 21 Tick·Gear N·구형 Fallback 제거와 ArmorBodyMap의 6개 세로 Bar·Armor Text 제거, Root 7 Slot·ReticleLayer 보존을 독립 확인했다. 2026-08-13 Structure-first 기준으로 Fifth Preview의 D1-07 외곽 배치·중앙 시야 구조 sanity를 승인 증거로 유지하고, `CFUIHUDProdEditorBridge v1.2.0`에서 `SpeedArcTrack=None`/Load 실패 시 `Image_RPMTrackArt=Collapsed`와 Resource/Visibility Validator를 적용했다. 공식 Build `cef0c064cd434848b1fcb94459fbccb5`, Production Apply `f1c32cab41b544d6b27709315029bb04`, 최종 Saved Readback `c0bdf6a72df3412da78057406e843c0c`, Fresh AssetDump dataset `adset_v1_aa5d0dfdce87eb6a015641240dbc3cdd.cf961b240264a26f8e54bd19`가 모두 PASS했다. Readback `ApplyUIHUDProduction.py v1.1.0`은 exact 10 Asset·Root 1·Panel 6·Element 2·HUD Visual Data 1, Semantic/Image Structure PASS, Border Mosaic 0, Gameplay Cast 0, Runtime Binding 0, UI-P0-03 0, D1-12 0, `d1_11_structure_pass=true`, `d1_11_pass=true`, `d1_11_art_user_approval=Pending`을 기록했다. 따라서 **D1-11 Production Structure Gate = PASS**로 닫고 다음 단계는 D1-11-ART-01 P1 HUD Source Art 사용자 검토다. 사용자 승인 전 Import/`DA_CFHUDVisual_Default` 연결은 하지 않으며 D1-12와 UI-P0-03은 시작하지 않는다.

---

## 2. 외부 근거 확정

### 2.1 Pretendard 라이선스

검증 원본:

```text
공식 저장소: orioncactus/pretendard
LICENSE: SIL Open Font License 1.1
Reserved Font Name:
- Pretendard
- Source
- Inter
- M PLUS 1
```

판정:

```text
Commercial Game Bundling = ALLOWED
Modification = ALLOWED WITH OFL CONDITIONS
Redistribution = ALLOWED WITH OFL CONDITIONS
Standalone Font Sale = NOT ALLOWED
Copyright Notice + OFL Text on Redistribution = REQUIRED
```

CarFight 방침:

- 공식 배포본의 TTF/OTF를 수정하지 않고 사용한다.
- Glyph 삭제·추가, Subset 재생성, Font Name 변경과 Variable Font 재가공을 D1 범위에서 하지 않는다.
- Unreal Import와 Cook은 게임 번들/임베드 경로로 취급하되, 원본 저작권 고지와 OFL 1.1 전문을 프로젝트 배포 고지에 포함한다.
- 실제 Font 파일을 수정하게 되는 미래 작업은 Reserved Font Name 조건을 다시 검토한다.

### 2.2 IBM Plex Mono 라이선스

검증 원본:

```text
공식 저장소: IBM/plex
LICENSE.txt: SIL Open Font License 1.1
Reserved Font Name: Plex
```

판정:

```text
Commercial Game Bundling = ALLOWED
Modification = ALLOWED WITH OFL CONDITIONS
Redistribution = ALLOWED WITH OFL CONDITIONS
Standalone Font Sale = NOT ALLOWED
Copyright Notice + OFL Text on Redistribution = REQUIRED
```

CarFight 방침:

- 공식 IBM Plex Mono TTF/OTF를 수정 없이 사용한다.
- D1에서는 원본 Typeface 이름과 Font Face를 그대로 사용한다.

### 2.3 CarFight 배포 고지 계약

D1-09B에서 Font Asset을 실제 프로젝트에 넣을 때 아래 원본 라이선스 파일을 같이 저장한다.

```text
UE/ThirdPartyNotices/Fonts/Pretendard-OFL-1.1.txt
UE/ThirdPartyNotices/Fonts/IBMPlex-OFL-1.1.txt
```

규칙:

- 파일에는 각 공식 배포본의 Copyright Notice와 OFL 1.1 전문을 그대로 보존한다.
- Font 원본 저장소에서 가져온 라이선스 전문을 임의 요약본으로 대체하지 않는다.
- Shipping 패키지에서 사용자가 고지 내용을 확인할 수 있는 최종 경로는 Release/Packaging 단계에서 검증한다.
- `UI-DESIGN-GATE`에서는 **라이선스 사용 적격성 확인과 프로젝트 내 원본 고지 보존**까지 요구한다.
- 실제 Shipping 빌드의 고지 파일 포함 여부는 UI 디자인 기능이 아니라 배포 검증 책임으로 남긴다.

License Eligibility Review:

```text
Pretendard = PASS
IBM Plex Mono = PASS
Shipping Notice Delivery = FUTURE PACKAGING VALIDATION
```

---

## 3. Unreal Font Asset 계약

### 3.1 현재 Unreal 기준 타입

Epic 최신 문서 기준:

```text
UFont
- Slate / UMG / Canvas가 사용하는 Font Object
- Runtime Cached일 때 Composite Font를 소유

UFontFace
- 원본 TTF/OTF payload를 보관
- UFont의 Typeface가 참조

FSlateFontInfo
- Slate/UMG에서 실제 Font Object + Size + Typeface Name을 결합
- UObject Font Object를 받는 생성 경로가 존재
```

따라서 CarFight Style Data의 최상위 Font 참조는 `UFont`로 고정한다.
`UFontFace`를 개별 Widget에 직접 전달하지 않는다.

### 3.2 Font Import 기본 규격

Unreal Editor 경로:

```text
콘텐츠 브라우저(Content Browser)
→ 임포트(Import)
→ 공식 TTF/OTF 선택
→ Font Face + Composite Font 생성
→ 폰트 에디터(Font Editor)에서 Runtime Composite 구성 확인
```

Font Face 기본 설정:

```text
Hinting = Default
Loading Policy = Lazy Load
Layout Method = Metrics
Distance Field = Off by default
```

예외:

- 실제 1080p에서 글자 상하가 잘리는 경우에만 `Layout Method = Bounding Box`를 비교 검토한다.
- 최초 표시 Hitch가 실제 측정될 때만 `Inline`을 검토한다.
- `Stream`은 전투 중 새 글리프 파일 I/O 가능성이 있으므로 P0 기본값으로 사용하지 않는다.
- Font Asset은 `Runtime Cached Composite Font`를 사용하고 Offline Font Atlas를 신규 기본안으로 사용하지 않는다.

### 3.3 실제 Font Asset Allowlist

경로:

```text
/Game/CarFight/UI/Fonts/
```

Composite Font:

```text
Font_CFUI
Font_CFNumeric
```

Pretendard Font Face:

```text
FF_Pretendard_Regular
FF_Pretendard_Medium
FF_Pretendard_SemiBold
FF_Pretendard_Bold
```

IBM Plex Mono Font Face:

```text
FF_IBMPlexMono_Regular
FF_IBMPlexMono_Medium
FF_IBMPlexMono_SemiBold
FF_IBMPlexMono_Bold
```

Composite Typeface Name은 두 Font 모두 정확히 다음 4개로 통일한다.

```text
Regular
Medium
SemiBold
Bold
```

이 이름이 `ECFUIFontWeight`와 직접 대응한다.

### 3.4 Variable Font 정책

D1 P0에서는 Variable Font를 사용하지 않는다.

이유:

- 현재 필요한 Weight는 네 단계로 고정돼 있다.
- Static Face는 Asset·Cook·Typeface Name 검증이 단순하다.
- Variable Font 축 범위와 UE 버전별 처리 차이를 이번 Gate의 새 변수로 만들 필요가 없다.

향후 Font Asset 수를 줄일 실익이 확인될 때 별도 최적화로 검토한다.

---

## 4. D1-08V — 실행 진입 Gate

D1-09A보다 반드시 먼저 실행한다.

```text
1. Unreal Editor 종료 확인
2. D:\Work\CarFight_git\Tools\BuildEditor.bat
3. CarFight.UI.D1_08.StyleDataContract Automation
```

PASS:

```text
Official Editor Build Exit Code = 0
StyleDataContract = Success
```

FAIL이면 D1-09A를 시작하지 않는다.

2026-08-10 Actual Evidence:

```text
Official Editor Build = PASS
Build Job = 0345aafbcc9341deb60b44791c528f84
Exit Code = 0

Automation Runner = Tools/RunCombatRuntimeTests.ps1
Process Job = 65308f5602454c42927fd238d8f412c3
CarFight.UI.D1_08.StyleDataContract = Success
Warnings = 0
Errors = 0
전체 CarFight = 47/47 Success
필수 회귀 = 24/24 Success
```

판정: `D1-08V = PASS`.

---

## 5. D1-09A — Font Binding / Layout / Density C++

### 5.1 정확한 Source Allowlist

신규:

```text
UE/Source/CarFight_Re/Public/UI/CFHUDLayoutData.h
UE/Source/CarFight_Re/Private/UI/CFHUDLayoutData.cpp
UE/Source/CarFight_Re/Public/UI/CFUIDensityData.h
UE/Source/CarFight_Re/Private/UI/CFUIDensityData.cpp
UE/Source/CarFight_Re/Private/CFUILayoutDataTests.cpp
```

수정 허용:

```text
UE/Source/CarFight_Re/Public/UI/CFUIStyleData.h
UE/Source/CarFight_Re/Private/UI/CFUIStyleData.cpp
UE/Source/CarFight_Re/Public/UI/CFUISubsystem.h
UE/Source/CarFight_Re/Private/UI/CFUISubsystem.cpp
UE/Source/CarFight_Re/CarFight_Re.Build.cs
```

`Build.cs`는 실제 필요한 신규 Module이 있을 때만 수정한다.
현재 `Engine`, `UMG`, `Slate`, `SlateCore`가 이미 존재하므로 Font 기능만을 이유로 신규 Module을 추가하지 않는다.

### 5.2 Font Style 타입

`CFUIStyleData.h`에 다음을 추가한다.

```text
ECFUIFontFamilyRole
- UI
- Numeric

FCFUIFontAssets
- UObject hard reference가 아니라 강타입 UFont hard reference 사용
- UIFontAsset : TObjectPtr<UFont>
- NumericFontAsset : TObjectPtr<UFont>
- RegularTypefaceName : FName = Regular
- MediumTypefaceName : FName = Medium
- SemiBoldTypefaceName : FName = SemiBold
- BoldTypefaceName : FName = Bold
```

Hard Reference 이유:

```text
UCFUISubsystem
→ DA_CFUIStyle_Default 하나를 Soft Reference로 1회 Load
→ DA_CFUIStyle_Default가 항상 필요한 Font_CFUI / Font_CFNumeric을 Hard Reference
→ Widget은 Font 경로를 Load하지 않음
```

Font 자체를 다시 Soft Reference로 중첩하면 Widget 사용 직전에 추가 Load 상태를 관리해야 하므로 P0에서는 사용하지 않는다.

`UCFUIStyleData` 추가 함수 후보:

```text
ResolveFontAsset(FontFamilyRole) -> UFont*
ResolveTypefaceName(FontWeight) -> FName
ResolveSlateFontInfo(FontFamilyRole, TypographyRole, TypographyScale, MinimumEffectiveFontSize) -> FSlateFontInfo
```

`ResolveSlateFontInfo`의 Font Size 규칙:

```text
ScaledSize = BaseTypographySize × TypographyScale
EffectiveSize = Max(ScaledSize, MinimumEffectiveFontSize)
```

Font Object가 없으면 Engine 기본 Font를 안전 Fallback으로 사용하되 `ValidateStyleData`는 실제 DA에서 Null Font를 검출할 수 있게 한다.

### 5.3 HUD Layout 타입

`ECFHUDSlotId`:

```text
MissionSummary
AlertFeed
TargetPanel
VehiclePanel
RadarPanel
WeaponPanel
ReticleLayer
```

`FCFUITypographyFloor`:

```text
DisplayXL
DisplayL
ValueM
ValueS
HeadingL
HeadingM
Body
Label
Caption
```

`FCFHUDSlotLayout`:

```text
SlotId
AnchorMinimum
AnchorMaximum
Alignment
PixelOffset
DesiredSize
RenderScale
ZOrder
bVisibleByDefault
```

`UCFHUDLayoutData : UPrimaryDataAsset`:

```text
ProfileId
ProfileVersion
ReferenceViewportSize
GeometryScale
TypographyScale
MinimumEffectiveFontSizes
SlotLayouts
```

역할:

- `PixelOffset`, `DesiredSize`는 해당 `ReferenceViewportSize`에서의 **최종 Screen-space 값**이다.
- `GeometryScale`은 Density/Shape/Icon 같은 내부 Design Token을 해상도에 맞게 조정한다.
- `TypographyScale`은 Typography Token 크기 조정에 사용한다.
- `MinimumEffectiveFontSizes`가 1080p 글자 하한을 보호한다.
- `ReticleLayer`는 Full Stretch이며 World Projection 좌표를 HUD 중앙 Canvas에 Clamp하지 않는다.

### 5.4 1080p Profile Scale

`DA_CFHUDLayout_1080_16`:

```text
ReferenceViewportSize = 1920 × 1080
GeometryScale = 0.75
TypographyScale = 0.75
```

Minimum Effective Font Size:

```text
DisplayXL = 38
DisplayL = 32
ValueM = 24
ValueS = 14
HeadingL = 22
HeadingM = 18
Body = 16
Label = 14
Caption = 12
```

안전 하한:

- `MinimumHitSize=48`은 GeometryScale을 곱하지 않는다.
- `OutlineHairline`은 최종 화면에서 1px 아래로 내리지 않는다.
- Critical 정보는 Scale을 이유로 제거하지 않는다.

### 5.5 Density 타입

`ECFUICaptionPolicy`:

```text
CoreOnly
Contextual
Expanded
```

`FCFUIDensityTokens`:

```text
PanelPadding
PanelInnerGap
SectionGap
InfoRowGap
HeaderHeight
InfoRowHeight
InfoRowHeightLarge
ChipHeight
IconSmall
IconMedium
IconLarge
PrimaryValueScale
SecondaryTextScale
OutlineScale
CaptionPolicy
```

`UCFUIDensityData : UPrimaryDataAsset`:

```text
Preset : ECFUIDensityPreset
Tokens : FCFUIDensityTokens
```

Density Data는 Gameplay 의미를 소유하지 않는다.

### 5.6 UCFUISubsystem Config 계약

P0에서는 별도 `UDeveloperSettings`를 추가하지 않는다.
기존 `UCFUISubsystem`에 최소 Config 계약을 둔다.

Class:

```text
UCLASS(BlueprintType, Config=Game)
UCFUISubsystem
```

Config Soft Reference:

```text
DefaultStyleDataAsset
DefaultDensityDataAsset
DefaultHUDLayoutDataAsset
```

타입:

```text
TSoftObjectPtr<UCFUIStyleData>
TSoftObjectPtr<UCFUIDensityData>
TSoftObjectPtr<UCFHUDLayoutData>
```

Runtime Cache:

```text
ResolvedStyleData
ResolvedDensityData
ResolvedHUDLayoutData
```

Subsystem 책임:

```text
Initialize / RegisterPlayerController
→ Config Soft Reference 해석
→ 한 번 Load
→ 유효성 검사
→ Getter로 Child Widget에 전달
```

개별 Widget은 `/Game/...` 문자열을 알지 못한다.

Config 누락:

- Style = D1-08 Native Safe Fallback
- Density = Native Standard Density Fallback
- Layout = 현재 16:9 최소 Native Fallback 또는 HUD Prototype 비표시

Layout의 Native Fallback은 Gameplay 좌표를 새로 추정하지 않고 D1-07 승인 1080p 값만 사용한다.

### 5.7 D1-09A Automation

추가 Test Prefix:

```text
CarFight.UI.D1_09A.LayoutDataContract
CarFight.UI.D1_09A.DensityDataContract
CarFight.UI.D1_09A.FontBindingContract
CarFight.UI.D1_09A.SubsystemFallbackContract
```

필수 검증:

- Slot ID 중복 검출
- Required 7 Slot 누락 검출
- Reference Viewport 0 이하 검출
- GeometryScale / TypographyScale 0 이하 검출
- Anchor 0~1 범위 검증
- DesiredSize 음수 검출
- 1080 Font Floor 기본값 검증
- Density Token 음수 검출
- MinimumHitSize 비스케일 규칙 검증
- Font Weight → Typeface Name 매핑 검증
- Config Asset Null일 때 Native Fallback 유효

D1-09A PASS 전에는 DataAsset을 만들지 않는다.

### 5.8 D1-09A 실제 구현·검증 결과 — 2026-08-10

구현 결과:

```text
CFUIStyleData v1.1.0
- ECFUIFontFamilyRole = UI / Numeric
- FCFUIFontAssets = UI/Numeric UFont Hard Reference + 4 Typeface Name
- ResolveFontAsset / ResolveTypefaceName / ResolveSlateFontInfo
- Font Asset 누락 시 Engine Core Style Font Fallback

CFHUDLayoutData v1.0.0
- 7 Required HUD Slot
- D1-07 USER PASS 1920×1080 Native Fallback
- GeometryScale 0.75 / TypographyScale 0.75
- Role별 1080p Font Floor
- Slot/Anchor/Scale/Size 검증

CFUIDensityData v1.0.0
- Compact / Standard / Expanded Native Preset
- Caption Policy
- Geometry·Scale 검증

CFUISubsystem v1.4.0
- Config=Game
- Style / Density / HUD Layout TSoftObjectPtr 진입점
- Subsystem 수명에서 1회 Load·Validate·Cache
- Config 누락·검증 실패 시 Native CDO Fallback

Build.cs
- 변경 없음
- 기존 Engine / UMG / Slate / SlateCore 의존성으로 충분
```

검증 중 발견된 두 오류는 제품 Runtime 결함이 아니라 UE 5.8 Automation Fixture 결함이었다.

```text
Fixture v1.0.1
- TArray 내부 요소를 같은 TArray Add 인수로 직접 사용해 Self-reference Assert 발생
- 지역 값 복사 후 Add하도록 수정

Fixture v1.0.3
- ULocalPlayerSubsystem / ULocalPlayer ClassWithin 계층 위반 Ensure 발생
- GEngine → Transient ULocalPlayer → UCFUISubsystem의 실제 Outer 계층으로 수정
```

최종 Evidence:

```text
Official Editor Build = PASS
Build Job = 9d9aefb3803e4556bd4559afded0f4ed
Exit Code = 0

Automation Runner = Tools/RunCombatRuntimeTests.ps1
Process Job = 18bea9b5952e4444b618e776cb2e2571
Editor Exit Code = 0
전체 CarFight = 51/51 Success
필수 회귀 = 24/24 Success
실패 = 0

CarFight.UI.D1_09A.LayoutDataContract = Success / Warning 0 / Error 0
CarFight.UI.D1_09A.DensityDataContract = Success / Warning 0 / Error 0
CarFight.UI.D1_09A.FontBindingContract = Success / Warning 0 / Error 0
CarFight.UI.D1_09A.SubsystemFallbackContract = Success / Warning 0 / Error 0
```

판정: `D1-09A = PASS`.

다음 허용 단계: `D1-09B` 실제 Font·Semantic Icon·Style/Density/1080 Layout DataAsset과 Config 연결. D1-10 이후 단계로 건너뛰지 않는다.

---

## 6. D1-09B — 실제 DataAsset / Font / Icon

### 6.1 DataAsset Allowlist

```text
/Game/CarFight/UI/Style/DA_CFUIStyle_Default

/Game/CarFight/UI/Density/DA_CFUIDensity_Compact
/Game/CarFight/UI/Density/DA_CFUIDensity_Standard
/Game/CarFight/UI/Density/DA_CFUIDensity_Expanded

/Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16
```

금지:

```text
DA_CFHUDLayout_1440_16
DA_CFHUDLayout_21x9
DA_CFHUDLayout_32x9
Custom User Layout Asset
```

후속 해상도는 Deferred Expansion이다.

### 6.2 DA_CFHUDLayout_1080_16 값

| Slot | Anchor | Alignment | Pixel Offset | Desired Size | ZOrder |
|---|---|---|---|---|---:|
| MissionSummary | Top Left | `(0,0)` | `(24,24)` | `375×87` | 10 |
| AlertFeed | Top Center | `(0.5,0)` | `(+6,24)` | `552×81` | 40 |
| TargetPanel | Top Right | `(1,0)` | `(-24,24)` | `312×232.5` | 10 |
| VehiclePanel | Bottom Left | `(0,1)` | `(24,-24)` | `672×312` | 10 |
| RadarPanel | Bottom Center | `(0.5,1)` | `(0,-24)` | `285×270` | 10 |
| WeaponPanel | Bottom Right | `(1,1)` | `(-24,-24)` | `270×190.5` | 10 |
| ReticleLayer | Full Stretch | `(0,0)` | `(0,0)` | Viewport Fill | 30 |

`AlertFeed +6px`은 USER PASS Mockup의 실제 중심 위치를 보존한다.

### 6.3 Density 초기값

#### Compact

```text
PanelPadding = 12
PanelInnerGap = 8
SectionGap = 8
InfoRowGap = 4
HeaderHeight = 36
InfoRowHeight = 28
InfoRowHeightLarge = 36
ChipHeight = 22
IconSmall / Medium / Large = 16 / 20 / 28
PrimaryValueScale = 0.86
SecondaryTextScale = 0.90
OutlineScale = 0.75
CaptionPolicy = CoreOnly
```

#### Standard

```text
24 / 12 / 16 / 8
48 / 36 / 44 / 28
20 / 24 / 32
1.00 / 1.00 / 1.00
CaptionPolicy = Contextual
```

#### Expanded

```text
32 / 16 / 24 / 12
56 / 44 / 52 / 32
24 / 32 / 40
1.12 / 1.08 / 1.00
CaptionPolicy = Expanded
```

### 6.4 Semantic Icon P0 Asset 규격

P0에서는 외부 Icon Pack을 그대로 고정하지 않는다.
CarFight가 소유하는 단색 원본으로 생성한다.

Source 권장:

```text
128 × 128 transparent PNG
white monochrome shape
alpha background
Texture Group = UI
Compression = UserInterface2D 계열
Tint = Style Data가 런타임 적용
```

최소 Semantic ID:

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

Asset 이름 예:

```text
T_UII_Vehicle
T_UII_Shield
T_UII_Armor
T_UII_Integrity
...
```

`RadarContact`의 Friendly/Neutral/Hostile/Unknown 구분은 관계색 + 기하 Symbol 계약을 유지하며 선택 상태는 Cyan Outer Bracket을 별도 표현한다.

### 6.5 DefaultGame.ini 예정 값

D1-09B 구현 시 실제 클래스 섹션명은 UHT 생성 결과를 확인한 뒤 적용한다.
의도 계약은 다음과 같다.

```text
DefaultStyleDataAsset = /Game/CarFight/UI/Style/DA_CFUIStyle_Default
DefaultDensityDataAsset = /Game/CarFight/UI/Density/DA_CFUIDensity_Standard
DefaultHUDLayoutDataAsset = /Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16
```

문자열 경로는 Config 한 곳에서만 소유한다.
Widget C++과 Blueprint Graph에 복제하지 않는다.

### 6.6 D1-09B 실제 구현·검증 결과 — 2026-08-10

생성 결과:

```text
FontFace = 8
- FF_Pretendard_Regular / Medium / SemiBold / Bold
- FF_IBMPlexMono_Regular / Medium / SemiBold / Bold

Runtime Composite UFont = 2
- Font_CFUI
- Font_CFNumeric

Semantic Icon Texture = 18
- Vehicle / Shield / Armor / Integrity / Engine / Steering / Turret
- Ammo / Battery / Charge / Heat / Cooldown / Reload
- Target / Lock / RadarContact / Warning / Critical

DataAsset = 5
- DA_CFUIStyle_Default
- DA_CFUIDensity_Compact
- DA_CFUIDensity_Standard
- DA_CFUIDensity_Expanded
- DA_CFHUDLayout_1080_16

Font Notice = 2
- UE/ThirdPartyNotices/Fonts/Pretendard-OFL-1.1.txt
- UE/ThirdPartyNotices/Fonts/IBMPlex-OFL-1.1.txt

Config = UE/Config/DefaultGame.ini
- DefaultStyleDataAsset -> DA_CFUIStyle_Default
- DefaultDensityDataAsset -> DA_CFUIDensity_Standard
- DefaultHUDLayoutDataAsset -> DA_CFHUDLayout_1080_16
```

Font 원본은 Pretendard 공식 `v1.3.9` 정적 Font와 IBM 공식 Plex Mono 정적 TTF를 사용했다. Semantic Icon 18종은 외부 Icon Pack을 가져오지 않고 CarFight 소유 128×128 투명 단색 원본으로 생성했다.

실행 Evidence:

```text
DryRun
- Process = ab53b08384df4e3b9b1a4d83354c4a63
- existing_targets_before = []
- forbidden_assets_found = []
- official Font/OFL source + icon source + CompositeFont syntax = PASS

Apply
- Process = 7611bbc6382546198fb9ea37dbe9b771
- Full Editor -ExecutePythonScript
- Editor Exit Code = 0
- asset_contract_passed = true
- license_contract_passed = true

Readback
- Process = d09097cc7781470ba9fd4261828a2c14
- asset_contract_passed = true
- license_contract_passed = true
- config_contract_passed = true
- d1_09b_pass = true

Final Full Regression
- Process = e73311a38be043d48d512ab90afd258f
- Editor Exit Code = 0
- 전체 CarFight = 51/51 Success
- 필수 회귀 = 24/24 Success
- Failed = 0
```

첫 Apply `9c190db655ce4fdc97fc0fac54647832`는 `PythonScript Commandlet`에서 `UFontFace::PostEditChangeProperty → FSlateApplication::Get()` Assert가 발생해 중단됐다. Font 데이터 결함으로 우회하지 않고 Apply 실행기만 Slate가 준비되는 Full Editor `-ExecutePythonScript`로 변경해 정상 생성했다.

전체 회귀의 첫 두 실행 `2f1db9b7bb2e42ffb8fa4284bd01eba0`, `548e96d098724f03bf875a2a25d26a6c`은 `WBP_TargetSelect.uasset`을 UnrealEditor PID 16096이 잡은 Windows Error Code 32 파일 잠금 때문에 `TargetHud` 1건만 실패했다. 해당 Asset·테스트를 수정하지 않았으며 잠금 해제와 UnrealEditor 잔류 0을 확인한 뒤 재실행한 최종 회귀가 51/51 PASS했다.

금지 범위 확인:

```text
DA_CFHUDLayout_1440_16 = 생성 안 함
DA_CFHUDLayout_21x9 = 생성 안 함
DA_CFHUDLayout_32x9 = 생성 안 함
Custom User Layout = 생성 안 함
Base Widget = 생성 안 함
WBP_CFInGameHUD = 생성 안 함
Pause Visual Migration = 시작 안 함
```

판정: `D1-09B = PASS`.

다음 허용 단계: `D1-10`. `D1-10`은 아직 `Not Started`이며 이번 단계에서 자동 착수하지 않는다.

---

## 7. D1-10 — Base Widget Production

D1-10은 `C++ Visual Context Bridge`와 `Blueprint Visual Asset` 두 단계로 나눈다.

### 7.0 실행 체크포인트 — 2026-08-10

```text
D1-10A
- UCFStyledWidgetBase / UCFButtonBaseWidget / CFUIBaseWidgetTests 구현
- Official Editor Build = PASS
  - Job 322535f4c9ab4b74843125afb0a8a970 / Exit 0
- CarFight.UI.D1_10A.VisualContextContract = Success
- CarFight.UI.D1_10A.ButtonBridgeContract = Success

D1-10B
- CFUIBaseEditorBridge = v1.2.0
- ApplyUIBaseWidgets.py = v1.2.1
- RunUIBaseWidgets.ps1 = v1.1.1
- Official Editor Build / DLL Link = PASS
  - Job ca38c51a4e264ca79e8fa018eba4584f / Exit 0
- Probe = PASS
  - Process ac14d231ec2248a29147187402061748
- DryRun = PASS
  - Process 83b87a0b61fd40beb676d827de16ce61
  - mutable target = 정확한 /Game/CarFight/UI/Base 5종만
- Apply = PASS
  - Process 2b5959aca44443829e6f58fd68ad1f76
  - created 5 / compiled 5 / saved 5
- Saved Readback = PASS
  - Process 93d974ce5c854c6da46ee5cbb87942d6
- Independent AssetDump = PASS
  - /Game/CarFight/UI/Base WidgetBlueprint 5/5 Success
- Full Protected Regression = PASS
  - Process 3c8fcd4a7438478db83eefbbd0719de0
  - total 53/53 Success / failed 0 / required 24/24 Success
  - TargetHud / D1-10A ButtonBridgeContract / VisualContextContract = Success
```

UE 5.8 Python Reflection에서 `UWidgetBlueprint::WidgetTree`는 직접 비노출이고, `bool ReturnValue + FString& OutFailureReason` 형태의 UFUNCTION은 Python에서 Out FString만 반환하는 실제 마샬링을 확인했다. 기존 상세 실패 사유 함수는 보존하고 Python 자동화용 bool-only `BuildBaseWidgetTreeResult` / `ValidateBaseWidgetTreeResult` 래퍼를 추가해 이 차이를 최소 격리했다.

D1-10B Apply는 아래 정확한 5개 Asset만 생성했다. 기존 D1-09B Asset과 TargetSelect/AimReticle 등 보호 범위는 수정하지 않았다.

```text
/Game/CarFight/UI/Base/WBP_CFButtonBase
/Game/CarFight/UI/Base/WBP_CFPanelBase
/Game/CarFight/UI/Base/WBP_CFStatusBar
/Game/CarFight/UI/Base/WBP_CFInfoRow
/Game/CarFight/UI/Base/WBP_CFAlertItem
```

종료 판정: `D1-10 = PASS`. D1-11은 `Prepared / Not Started`이며 이번 작업에서는 착수하지 않았다.

### 7.1 D1-10A C++ Visual Context Bridge

신규 후보:

```text
Public/UI/CFStyledWidgetBase.h
Private/UI/CFStyledWidgetBase.cpp
Public/UI/CFButtonBaseWidget.h
Private/UI/CFButtonBaseWidget.cpp
Private/CFUIBaseWidgetTests.cpp
```

#### UCFStyledWidgetBase

부모:

```text
UUserWidget
```

책임:

```text
StyleData 참조 보관
DensityData 참조 보관
GeometryScale 보관
TypographyScale 보관
Typography Floor 보관
Style Context 중복 적용 방지
Blueprint Visual Refresh Event 호출
```

비책임:

```text
Gameplay Actor 탐색
Gameplay 값 계산
Widget 위치 결정
DataAsset 직접 Load
```

공개 함수 후보:

```text
SetUIVisualContext(...)
RefreshUIVisualStyle()
GetResolvedGeometryScale()
ResolveScaledSpacing(...)
ResolveTypographyFont(...)
```

Blueprint Event:

```text
OnUIVisualStyleChanged
```

#### UCFButtonBaseWidget

부모:

```text
UCFStyledWidgetBase
```

필수 Bind Widget:

```text
InteractionButton : UButton
```

책임:

- Click/Hover/Press/Release를 안정적인 이벤트로 변환
- Keyboard/Gamepad Focus Path 진입·이탈 감지
- Focus와 Hover를 별개 Visual State로 전달
- 실제 `UButton*` 반환
- 외부 Focus 요청 처리

공개 계약 후보:

```text
GetInteractionButton()
FocusInteractionButton()
SetButtonEnabled()
OnActivated
OnButtonVisualStateChanged
```

Pause 판정과 World Pause는 절대 소유하지 않는다.

### 7.2 D1-10B Blueprint Asset Allowlist

```text
/Game/CarFight/UI/Base/WBP_CFButtonBase
/Game/CarFight/UI/Base/WBP_CFPanelBase
/Game/CarFight/UI/Base/WBP_CFStatusBar
/Game/CarFight/UI/Base/WBP_CFInfoRow
/Game/CarFight/UI/Base/WBP_CFAlertItem
```

### 7.3 WBP_CFButtonBase

Parent:

```text
CFButtonBaseWidget
```

Widget Tree:

```text
SizeBox_Root
└─ Button_Interaction
   └─ Overlay_Visual
      ├─ Border_Background
      ├─ Border_FocusOutline
      ├─ HorizontalBox_Content
      │  ├─ Image_Icon
      │  ├─ Text_Label
      │  ├─ Spacer_Fill
      │  └─ Text_InputHint
      └─ Border_StateMarker
```

외부 변수:

```text
ButtonText
SemanticIconId
InputHintText
ButtonVariant
bShowIcon
bShowInputHint
bOverrideMinimumSize
OverrideMinimumSize
```

상태:

```text
Normal
Hover
Focused
Pressed
Disabled
```

Focus는 Hover와 합치지 않는다.

Animation 원칙:

- UMG Animation Track은 1초 Normalized 길이로 제작할 수 있다.
- 실제 Token Duration은 Play Speed로 맞춘다.
- Idle Loop 금지.

### 7.4 WBP_CFPanelBase

Parent:

```text
CFStyledWidgetBase
```

Widget Tree:

```text
Overlay_Root
├─ Border_Background
├─ Border_Outline
├─ Border_AccentMarker
└─ VerticalBox_Layout
   ├─ SizeBox_Header
   │  └─ HorizontalBox_Header
   │     ├─ Image_HeaderIcon
   │     ├─ Text_Header
   │     └─ NamedSlot_HeaderExtra
   └─ NamedSlot_Content
```

외부 변수:

```text
PanelTitle
PanelVariant
SemanticHeaderIconId
bShowHeader
bShowAccentMarker
bOverrideContentPadding
ContentPaddingOverride
```

Gameplay 조회 금지.

### 7.5 WBP_CFStatusBar

Parent:

```text
CFStyledWidgetBase
```

Widget Tree:

```text
SizeBox_Root
└─ Overlay_Bar
   ├─ Border_Track
   ├─ ProgressBar_Continuous
   ├─ HorizontalBox_ArmorSegments
   ├─ Text_PrimaryValue
   └─ Text_SecondaryValue
```

Input:

```text
NormalizedValue
PrimaryText
SecondaryText
BarVariant
DisplayState
bShowNumericValue
bShowLabel
```

규칙:

- `Current/Maximum` 계산 금지.
- Presenter가 제공한 `NormalizedValue` 사용.
- Armor 10 Segment는 Visual 계산만 허용.
- Unavailable = `N/A`.
- KnownZero = 정상 0.

### 7.6 WBP_CFInfoRow

Parent:

```text
CFStyledWidgetBase
```

Widget Tree:

```text
HorizontalBox_Root
├─ SizeBox_Label
│  └─ Text_Label
├─ SizeBox_Value
│  └─ Text_Value
├─ Spacer_Fill
└─ Image_StateIcon
```

Input:

```text
LabelText
ValueText
KnowledgeState
SemanticStateIconId
bShowStateIcon
bReserveValueWidth
```

`Unknown`, `Estimated`, `Known`, `Unavailable`, `KnownZero`, `NotApplicable`을 섞지 않는다.

### 7.7 WBP_CFAlertItem

Parent:

```text
CFStyledWidgetBase
```

Widget Tree:

```text
SizeBox_Root
└─ Overlay_Root
   ├─ Border_Background
   ├─ Border_AccentLine
   └─ HorizontalBox_Content
      ├─ Image_Severity
      └─ Text_Display
```

Input:

```text
AlertKey
Severity
DisplayText
SemanticAlertIconId
Duration
bPersistent
```

- 자체 Dedup 금지.
- Critical만 반복 Pulse 허용.

### 7.8 D1-10 검증

```text
C++ Bridge Build PASS
Base Widget Contract Automation PASS
5개 Widget Blueprint Compile PASS
InteractionButton Bind PASS
Style/Density Context 외부 전달 PASS
Gameplay Cast 0
DataAsset 직접 Load 0
Collapsed Spacer 잔류 0
```

실제 종료 Evidence:

```text
Official Build ca38c51a4e264ca79e8fa018eba4584f = PASS
Probe ac14d231ec2248a29147187402061748 = PASS
DryRun 83b87a0b61fd40beb676d827de16ce61 = PASS
Apply 2b5959aca44443829e6f58fd68ad1f76 = PASS
Readback 93d974ce5c854c6da46ee5cbb87942d6 = PASS
Exact WidgetTree 5/5 = PASS
Native Parent 5/5 = PASS
Button_Interaction BindWidget = PASS
Style/Density External Context 5/5 = PASS
Gameplay Cast = 0
DataAsset Direct Load = 0
Collapsed Spacer Residue = 0
Graph CallFunction = 0
AssetDump /Game/CarFight/UI/Base WidgetBlueprint = 5/5 Success
Protected Regression 3c8fcd4a7438478db83eefbbd0719de0 = 53/53 Success / Required 24/24 / Failed 0
D1-10 = PASS
D1-11 = Not Started
```

---

## 8. D1-11 — Production WBP_CFInGameHUD Visual Prototype

### 8.1 역할

실제 Gameplay HUD가 아니다.
D1-07 승인 Mockup의 Layout·Semantic 의미를 **Production UMG 제작 구조**로 재현하는 Visual Contract 검증용이다. 이전 v1.x flattened Border Mock은 Historical evidence로만 보존하며 현재 제작 경로에는 사용하지 않는다.

### 8.1A 실행 체크포인트 — 2026-08-10

```text
D1-11 Target
- /Game/CarFight/UI/HUD/WBP_CFInGameHUD 정확히 1개
- Parent = CFStyledWidgetBase
- D1-12 Asset = 0
- UI-P0-03 View Data = 0

Editor Tool
- CFUIHUDEditorBridge v1.3.0
- ApplyUIHUDPrototype.py v1.3.0
- RunUIHUDPrototype.ps1 v1.0.0

User Preview History
- First 1920×1080 Preview = FAIL / 내부 축소·단순화
- Second User Re-preview = FAIL / Vehicle Arc·Silhouette 부족 + `NamedSlot_*` 잔상
- Third User Re-preview = FAIL / 잔상·전체 Slot은 개선, Vehicle Armor 의미별 위치 불일치 + Arc/Silhouette 판독성 부족
- Fourth User Re-preview = FAIL / 시각 근접도는 개선됐지만 Border/Canvas 절대좌표 Mosaic 방식 자체가 Production UI 구조로 부적합

v1.3 Source Build
- 4e455496158f4c9bb25ee9851bb65718 = PASS / Exit 0

v1.3 DryRun
- abf8def22ad54d69a174671f926b2b9c = PASS
- mutable asset count = 1
- action = rebuild_exact

v1.3 Rebuild Apply
- 3dcf10ee81fc4e1b938849ff1260bc5d = PASS
- same WBP_CFInGameHUD reused / WidgetTree rebuilt / compiled / saved
- Vehicle Armor semantic position/value validator = PASS

Concurrent BuildId Interruption
- AssetDump concurrent work changed UE module BuildId while commandlet validation was starting
- historical failures reached plugin load before HUD Python/Validator
- no HUD failure classification

BuildId Resync Official Build
- 608a05d40bab49e7abfed3073ee1914b = PASS / Exit 0
- changed AssetDump module compiled and linked with current CarFight_ReEditor target

Recovered Saved Readback
- e8a74175779c41a3a6a9186bcbf6f087 = PASS
- tool_version = 1.3.0
- Native Parent / Visual Fidelity Contract = PASS
- D1-11 final user gate remains Pending

Independent AssetDump
- /Game/CarFight/UI/HUD WidgetBlueprint = 1/1 Success
- Failed = 0

Protected Regression
- 01dc9e734d4946208d5de53ccc3f39fa
- 53/53 Success
- Required 24/24 Success
- Failed 0

Production Rework Current
- UCFUIStyleData v1.2.0 / UCFHUDVisualData v1.0.0 / CFUIStyleDataTests v1.1.0
- CFUIHUDProdEditorBridge v1.0.0
- ApplyUIHUDProduction.py v1.0.0 / RunUIHUDProduction.ps1 v1.0.0
- Foundation Link Build 2e63b4a2ee77448a95bd645ac23b47cb = PASS / Exit 0
- Production Bridge Build 5653c410a2674232aa5477f12437c91d = PASS / UHT·Compile·DLL Link / Exit 0
- Probe ac83c6588cdb49dca292e0653774d08b = PASS
- DryRun 3e3ff6fa152942fca0f555083779632c = PASS / exact mutable asset count 10
- Apply 3c7dfff73b4042f895e98989c825c9d1 = PASS / create 9 + reuse root 1 / WidgetBlueprint 9/9 Compile / save 10
- Saved Readback eac37017a42f4a46989c98691d3fdb9f = PASS
- Border Mosaic residue = 0 / Gameplay Cast = 0 / Runtime Event Binding = 0
- Independent AssetDump = WidgetBlueprint 9/9 + HUD 전체 10/10 Success / Failed 0
- Protected Regression 05faa8326c7848a5b1ca18525b59540c = 53/53 / Required 24/24 / Failed 0
- New Production 1920x1080 User Preview = FIFTH PREVIEW FAIL
```

첫 Apply `d1f181a339334f519a636ad71c35e576`은 신규 Widget Blueprint에 UE가 자동 생성하는 미연결 기본 `K2Node_Event` 3개를 Runtime Logic으로 과잉 판정해 Save 전 중단됐다. 해당 실행에서 HUD Asset은 디스크에 남지 않았다. Validator는 기본 Event Node가 모든 Pin 미연결일 때만 허용하고 Event 외 Node 또는 연결된 Pin은 계속 FAIL하도록 정밀화했다.

첫 1920×1080 사용자 Preview와 v1.1.0 이후 두 번째 User Re-preview는 모두 FAIL했다. 두 번째 FAIL에서 확인된 Vehicle Arc/Silhouette와 중첩 Base Widget Designer Placeholder 문제를 v1.2.0으로 보정했고 자동/정적 범위는 PASS다. v1.2.0 수정 후 세 번째 1920×1080 사용자 Preview는 아직 수행하지 않았으므로 이 체크포인트만으로 D1-11을 PASS 처리하지 않는다.

### 8.2 Assets

```text
Root
- /Game/CarFight/UI/HUD/WBP_CFInGameHUD

Panels
- /Game/CarFight/UI/HUD/Panels/WBP_CFMissionPanel
- /Game/CarFight/UI/HUD/Panels/WBP_CFAlertFeed
- /Game/CarFight/UI/HUD/Panels/WBP_CFTargetPanel
- /Game/CarFight/UI/HUD/Panels/WBP_CFVehiclePanel
- /Game/CarFight/UI/HUD/Panels/WBP_CFRadarPanel
- /Game/CarFight/UI/HUD/Panels/WBP_CFWeaponPanel

Elements
- /Game/CarFight/UI/HUD/Elements/WBP_CFSpeedGauge
- /Game/CarFight/UI/HUD/Elements/WBP_CFArmorBodyMap

Visual Data
- /Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default
```

### 8.3 Production Widget Tree

```text
WBP_CFInGameHUD
CanvasPanel_Root                           # 화면 Slot 배치 전용
├─ SizeBox_Slot_MissionSummary
│  └─ WBP_CFMissionPanel
├─ SizeBox_Slot_AlertFeed
│  └─ WBP_CFAlertFeed
├─ SizeBox_Slot_TargetPanel
│  └─ WBP_CFTargetPanel
├─ SizeBox_Slot_VehiclePanel
│  └─ WBP_CFVehiclePanel
│     ├─ WBP_CFSpeedGauge
│     └─ WBP_CFArmorBodyMap
├─ SizeBox_Slot_RadarPanel
│  └─ WBP_CFRadarPanel
├─ SizeBox_Slot_WeaponPanel
│  └─ WBP_CFWeaponPanel
└─ CanvasPanel_Slot_ReticleLayer
```

Panel 제작 규칙:
- Panel은 실제 Surface/Container 목적의 `Border_Surface` **정확히 1개**만 허용한다.
- `WBP_CFSpeedGauge`, `WBP_CFArmorBodyMap` Element는 Border로 도형을 그리지 않으며 Border 0을 유지한다.
- 아이콘·차량·Armor·Radar Contact·Weapon 상태 시각은 `Image`를 사용한다.
- 연속 상태는 `ProgressBar`, 문자 정보는 `TextBlock`을 사용한다.
- `CanvasPanel`은 Root Slot 배치, Armor Body Map의 방향별 공간 배치, Radar Contact의 2D 공간 배치처럼 좌표 의미가 실제로 필요한 곳만 허용한다.
- `Mock`, `Mosaic` 이름과 작은 Border 조각 기반 도형 합성은 Validator에서 금지한다.
- 모든 Root Slot의 Canvas 위치와 Size는 `DA_CFHUDLayout_1080_16`에서만 가져온다.

Visual Asset 정책:
- Semantic Icon 18종은 `DA_CFUIStyle_Default.IconSet`을 그대로 재사용한다.
- Vehicle Silhouette, 6방향 Armor Plate, Speed Arc, Radar Frame/Sweep, Selected Target/Weapon 전용 아트는 `DA_CFHUDVisual_Default`가 교체 지점을 소유한다.
- 현재 전용 아트가 비어 있는 슬롯은 Production Preview에서 Semantic Texture를 Fallback Brush로 사용할 수 있다. 실제 전용 아트 연결 시 Widget Tree를 재작성하지 않는다.

### 8.4 Screen-space Scaling 원칙

```text
Slot Position / Desired Size
→ Layout Data의 1920×1080 최종 Screen-space 값

Panel 내부 Padding / Gap / Header / Icon / Outline
→ Density·Style Token × GeometryScale 0.75

Typography
→ Style Typography × TypographyScale 0.75
→ Role별 Minimum Effective Font Size Clamp

MinimumHitSize
→ Scale하지 않음
```

이 구조를 사용하면 `ReticleLayer`와 향후 World Target Marker가 전체 Viewport Projection을 그대로 사용할 수 있다.

### 8.5 Mock Data

D1-07과 같은 Mock 데이터를 사용한다.
새 Gameplay 정보를 만들지 않는다.

```text
Mission: 호송 차량 보호 / 적 3
Alert: Front Armor 30% / Ripple 2/4
Target: Hostile / 842m / Identity ??? / Armor ??? / Scan 35%
Vehicle: 076 km/h / N / 현재 승인 방어값
Weapon: Heavy Cannon / Ammo 20|120 / Heat 38% / Cooldown 0.8s
```

### 8.6 금지

```text
ACFVehiclePawn Cast
UCFVehicleDefenseComp 조회
WeaponFire / Launcher 조회
TargetSelect 조회
Radar Actor Scan
UI-P0-03 View Data 구현
Runtime Event Binding
21:9 / 32:9 Profile 생성
```

### 8.7 D1-11 PASS — Production Structure Gate

2026-08-13 사용자 결정에 따라 D1-11의 PASS 책임을 **최종 미술 완성도 판정이 아니라 Production UMG 내부 구조와 의미 계약 판정**으로 조정한다. 이미지의 최종 형태, 세부 간격, 픽셀 단위 위치와 시각 폴리시는 D1-11-ART 및 후속 Unreal Editor 수동 조정에서 다룰 수 있으며 D1-11 Structure PASS를 단독 차단하지 않는다.

D1-11 Structure PASS 필수 조건:

- exact Production Asset 경계가 Root 1 + Panel 6 + Element 2 + HUD Visual Data 1을 유지한다.
- `WBP_CFInGameHUD`가 D1-07 기반 여섯 의미 Panel Slot + Full Stretch ReticleLayer의 정확한 7 Slot 구조를 유지한다.
- Root는 Slot 배치만, Panel/Element는 의미 단위 Widget 구조를 소유하며 `Canvas_Mock_*`, Border Mosaic과 작은 Border 도형 합성이 없다.
- Gameplay Cast, Runtime Event Binding, 직접 Gameplay 판정, UI-P0-03 View Data와 D1-12 Asset이 0이다.
- SpeedGauge가 21 RPM Tick, 85% Red Zone 의미, 3자리 속도, `km/h`, 단일 Gear Slot을 유지하고 `D`·가짜 RPM/Speed Fallback을 사용하지 않는다.
- 전용 `SpeedArcTrack` Texture가 없거나 Load 실패하면 `Image_RPMTrackArt` 교체 슬롯은 유지하되 `Collapsed`여서 빈/default Brush가 렌더링되지 않는다.
- ArmorBodyMap이 Vehicle Silhouette 교체 슬롯, 6방향 Plate/Badge 교체 슬롯, 각 방향 우측 세로 Armor Bar를 유지하고 `Text_Armor*` 숫자/방향 Text를 재도입하지 않는다.
- 1920×1080 화면에서 D1-07 외곽 배치와 중앙 전투 시야가 구조적으로 붕괴하지 않는다. Fifth Preview에서 이 항목은 확인됐다.
- 공식 UE 5.8 Build, Production Apply, Saved Readback과 Fresh AssetDump가 모두 PASS한다.

D1-11 Structure PASS를 **차단하지 않는 후속 조정 항목**:

- Vehicle Silhouette·Armor Plate·Speed Arc·Target Bracket 등 전용 HUD Art의 최종 모양과 스타일.
- Panel 내부의 세부 정렬, 간격, 이미지 크기와 픽셀 단위 미세 위치.
- Placeholder/Fallback Icon의 미적 완성도 자체. 단, 빈 Brush 사각형처럼 잘못된 의미 또는 예상치 못한 면을 생성하는 렌더링 결함은 차단한다.
- 최종 이미지 Import·`DA_CFHUDVisual_Default` 연결과 그 후의 미술 폴리시. 이는 D1-11-ART 사용자 승인 절차를 따른다.

현재 판정:

```text
First 1920×1080 Designer/User Preview = FAIL
Second 1920×1080 Designer/User Re-preview = FAIL
Third 1920×1080 Designer/User Re-preview = FAIL
v1.3 Vehicle Semantic Visual Fix Build / DryRun / Rebuild Apply / Compile / Saved Readback = PASS
Vehicle Armor Semantic Position/Value Contract = PASS
Hidden Base Widget Designer Placeholder Contract = PASS
AssetDump = PASS
Gameplay Query / Runtime Binding = 0
Protected Regression = PASS
Fourth 1920×1080 Designer/User Re-preview = FAIL
Production Visual Composition Rework Build/Assetization/Readback/AssetDump/Regression = PASS
Fifth Production 1920×1080 Preview = Structure Sanity ACCEPTED / Art Polish Deferred
Empty RPM Track Fix + Validator = PASS
Official Build / Production Apply / Saved Readback / Fresh AssetDump = PASS
D1-11 Production Structure Gate = PASS
D1-11-ART-01 User Visual Review = READY / PENDING
D1-12 = NOT STARTED
```

첫 Preview FAIL은 Slot 내부 Visual Fidelity, 두 번째 Preview FAIL은 Vehicle Arc/Silhouette와 중첩 Base Widget Designer Placeholder, 세 번째 Preview FAIL은 Armor 의미별 위치와 Arc/Silhouette 판독성, 네 번째 Preview FAIL은 Border Mosaic 제작 구조 자체가 핵심이었다. Fifth Preview에서는 D1-07 외곽 배치·중앙 시야·Production 의미 단위 구조는 화면에서도 유지됐고, 실제 Structure 차단 결함은 `SpeedArcTrack=None`인데 가시 상태의 `Image_RPMTrackArt`가 빈/default Brush Tint 사각형을 만든 것이었다. `CFUIHUDProdEditorBridge v1.2.0`은 Texture 미해석 시 해당 슬롯을 `Collapsed` 처리하고 Resource/Visibility Validator를 추가했다. 에디터 종료 후 공식 Build `cef0c064cd434848b1fcb94459fbccb5` Exit 0, Apply `f1c32cab41b544d6b27709315029bb04` Exit 0, 최종 Readback `c0bdf6a72df3412da78057406e843c0c` Exit 0, Fresh AssetDump 9/9 Success를 확인했다. AssetDump는 `Image_RPMTrackArt visibility=Collapsed`, RPM Tick 00~20, `076 / km/h / N`, ArmorBodyMap 6방향 세로 Bar, Root 여섯 Panel + ReticleLayer를 저장 자산에서 독립 확인했다. 따라서 D1-11 Structure Gate는 PASS이며 전용 HUD Art 최종 형태와 세부 배치는 D1-11-ART 사용자 검토로 넘긴다.

---

## 9. D1-12 — Pause Menu Visual Migration

### 9.1 현재 실제 제약

현재 런타임은:

```text
UCFUISubsystem::CreatePauseMenuWidget()
→ UCFPauseMenuWidget::StaticClass() 직접 생성

UCFPauseMenuWidget::NativeOnInitialized()
→ EnsurePauseMenuTree()
→ Native WidgetTree 직접 구성
```

따라서 WBP를 만들기만 해서는 적용되지 않는다.

### 9.2 WBP Asset

```text
/Game/CarFight/UI/Menu/WBP_CFPauseMenu
```

Parent:

```text
CFPauseMenuWidget
```

Widget Tree:

```text
CanvasPanel_Root
├─ Border_DimOverlay
└─ SizeBox_MenuPanel
   └─ WBP_CFPanelBase
      └─ VerticalBox_MenuContent
         ├─ Text_PauseTitle
         ├─ WBP_CFButtonBase_Continue
         └─ Text_ContinueHint
```

Style:

```text
DimOverlay
SurfaceOverlay
HeadingL
Primary Button
Standard Density
```

### 9.3 UCFPauseMenuWidget 최소 변경

기존 공개 함수 시그니처는 보존한다.

추가/변경 계약:

```text
Designer Tree 존재 + Continue Base Widget Bind 성공
→ Designer Tree 사용
→ Native Tree 생성 금지

Designer Tree 없음 또는 Bind 실패
→ 기존 EnsurePauseMenuTree Native Fallback 사용
```

새 Optional Bind 후보:

```text
ContinueButtonWidget : UCFButtonBaseWidget / BindWidgetOptional
```

기존 함수:

```text
GetContinueButton()
- Designer Tree면 ContinueButtonWidget->GetInteractionButton()
- Native면 기존 ContinueButton 반환

FocusDefaultButton()
- Designer Tree면 ContinueButtonWidget Focus
- Native면 기존 UButton Focus
```

Continue Activate:

```text
WBP_CFButtonBase OnActivated
→ UCFPauseMenuWidget 내부 Continue Handler
→ 기존 OnContinueRequested Broadcast
```

Pause 해제 판정은 계속 `UCFUISubsystem`이 소유한다.

### 9.4 UCFUISubsystem 최소 변경

D1-12에서 Config에 추가:

```text
PauseMenuWidgetClass : TSoftClassPtr<UCFPauseMenuWidget>
```

해석:

```text
Subsystem 초기화 시 1회 Resolve
→ 유효한 WBP_CFPauseMenu Class면 Cache
→ 실패/Null이면 UCFPauseMenuWidget::StaticClass() Cache
```

`CreatePauseMenuWidget()`은 Cache된 Class만 사용한다.
Pause 버튼을 누르는 순간 새 Soft Load를 하지 않는다.

### 9.5 D1-12 보호 범위

변경 금지:

- `APlayerController::SetPause` 경로
- 차량 Drive / Look 입력 중립화
- Pressed Key Flush
- GameAndUI / GameOnly 복귀
- Ripple·Salvo Sequence Runtime
- Projectile / Motor 상태
- World Timer 진행
- Menu 단일 인스턴스 수명
- Back / Continue 의미

### 9.6 D1-12 PASS

자동:

```text
기존 Pause Automation 전부 Success
Pause Visual Class Resolve Contract Success
Native Fallback Contract Success
Blueprint Continue Bridge Contract Success
```

사용자 PIE:

```text
Pause 진입
→ 새 Visual 표시
→ Continue Mouse 동작
→ Enter 동작
→ Escape/Back 동작
→ Gamepad Focus 확인
→ 반복 Pause 중 Widget 중복 없음
→ Pause 중 World 완전 정지 유지
```

사용자 확인 전 D1-12를 USER PASS로 기록하지 않는다.

---

## 10. 전체 검증 행렬

| 단계 | 자동/정적 | 에디터/사용자 | PASS |
|---|---|---|---|
| D1-08V | Official Build + StyleDataContract | 없음 | 둘 다 Success |
| D1-09A | Layout/Density/Font/Subsystem Contract | 없음 | Compile/UHT + Contract Success |
| D1-09B | Asset Class/Reference Readback | DataAsset·Font Editor 확인 | Style·Density·Layout·Font·Icon 일치 |
| D1-10A | C++ Bridge Build/Automation | 없음 | Context + Focus Bridge Success |
| D1-10B | Widget BP Compile/Readback | Designer State Preview | 5종 Compile / Cast 0 / Direct Load 0 |
| D1-11 | Production exact Asset/7 Slot/Semantic Child/Border·Graph·Fallback/SpeedGauge·Armor 계약 + Official Build·Apply·Readback·AssetDump | 1920×1080에서 외곽 배치·중앙 시야 구조 붕괴만 확인. 미술·세부 배치는 D1-11-ART/후속 Editor 조정 | Structure 계약과 자동 검증 전체 PASS + 화면 구조 sanity PASS |
| D1-12 | Pause Regression + Class/Fallback | Pause/Continue/Focus PIE | 기존 동작 회귀 0 |

---

## 11. 전역 중단 조건

다음 중 하나라도 발생하면 다음 단계로 진행하지 않는다.

```text
D1-08V Build / Automation FAIL
D1-09A UHT / Compile FAIL
Font Asset Type이 현재 D:\UnrealEngine_Source와 불일치
Font LICENSE 원본 고지 확보 실패
Layout Required Slot 누락/중복
1080p Profile이 D1-07 승인 위치와 불일치
Base Widget Blueprint Compile FAIL
Gameplay Cast 또는 직접 Gameplay 판정 발견
Widget 내부 콘텐츠 경로 직접 Load 발견
D1-11에서 실제 HUD Runtime 구현으로 범위 확장
Pause Visual 적용 후 Pause·Continue·Focus 회귀
```

문제 발생 시 실패한 단계만 최소 수정한다.
다음 단계로 우회하거나 별도 기능을 시작하지 않는다.

---

## 12. 보호 범위

- 기존 미커밋 Source와 Unreal Asset을 덮어쓰지 않는다.
- `WBP_TargetSelect`, AimReticle, 기존 TargetSelect 선택 상태는 D1에서 수정하지 않는다.
- VehicleDefense·Damage·Fitting·Launcher·Projectile 계산은 UI에서 수정하지 않는다.
- `UI-P0-03` View Data는 D1-11 Mock Prototype을 이유로 선행 구현하지 않는다.
- 1440p·21:9·32:9는 Deferred Expansion으로 유지한다.
- CommonUI는 도입하지 않는다.
- commit·push는 사용자 명시 요청 전 수행하지 않는다.

---

## 13. Changelog

### v0.16.2 - 2026-08-24

- CF-FQ-039 USER 결정에 따라 이 문서의 과거 D1-11 Prototype/Scaffold/Apply 절차를 Current Production Visual 제작 방식으로 재사용하지 않도록 Applicability를 명시했다.
- Visual 제작의 유일한 Current 경로를 `InGameUIVisualPlan.md v0.1.10`의 Target Decomposition Pipeline으로 연결했다. 이 문서는 구조·Migration·Fallback Historical evidence만 supporting reference로 유지한다.
- 과거 WBP/Primitive/Scaffold 결과는 Runtime/구조 evidence로 보존할 수 있으나 새 Visual authority로 역승격하지 않는다.

### v0.16.0 - 2026-08-13

- 사용자 종료 후 공식 UE 5.8 Editor Build `cef0c064cd434848b1fcb94459fbccb5`가 제품 DLL Link까지 Exit 0으로 PASS했다.
- Production Apply `f1c32cab41b544d6b27709315029bb04`와 Saved Readback `3b10248701f94c9a9b490c1aaa0d5e13`가 Exit 0 PASS했고 exact 10 Asset·Root 1·Panel 6·Element 2·HUD Visual Data 1, Semantic/Image Structure PASS, Border Mosaic·Gameplay Cast·Runtime Binding·D1-12·UI-P0-03 0을 유지했다.
- Structure-first 사용자 결정과 공식 보고 계약을 일치시키기 위해 `ApplyUIHUDProduction.py v1.1.0`으로 갱신했다. `preview_1920x1080_user_gate=StructureSanityAccepted`, `d1_11_structure_pass=true`, `d1_11_pass=true`, `d1_11_art_user_approval=Pending`을 분리 기록한다.
- 최종 Saved Readback `c0bdf6a72df3412da78057406e843c0c`가 새 보고 계약으로 Exit 0 PASS했다.
- Fresh AssetDump dataset `adset_v1_aa5d0dfdce87eb6a015641240dbc3cdd.cf961b240264a26f8e54bd19`은 WidgetBlueprint 9/9 Success를 기록했다. `WBP_CFSpeedGauge.Image_RPMTrackArt`가 `visibility=Collapsed`이고 21 RPM Tick·Gear N을 유지하며, ArmorBodyMap 6방향 세로 Bar와 Root 7 Slot 의미 구조를 재확인했다.
- 위 증거와 Fifth Preview 구조 sanity 확인을 합쳐 **D1-11 Production Structure Gate를 PASS**로 닫았다. 이미지 형태·세부 간격·픽셀 폴리시는 별도 D1-11-ART 사용자 검토 책임이다.
- 다음 단계는 D1-11-ART-01 P1 HUD Source Art 사용자 Visual Review다. 기존 Source 9/9 Technical PASS는 유지하지만 UserVisualApproval은 아직 Pending이며, 승인 전 Unreal Import·`DA_CFHUDVisual_Default` 연결은 0으로 유지한다.
- D1-12와 UI-P0-03은 시작하지 않았고 기존 미커밋 변경·unrelated 에셋을 보호했으며 commit·push는 수행하지 않았다.

### v0.15.0 - 2026-08-13

- 사용자 결정에 따라 D1-11 PASS 기준을 최종 미적 완성도 중심에서 **Production 내부 구조·의미 단위·안전 Fallback 중심 Structure Gate**로 조정했다. 이미지 형상·세부 간격·픽셀 배치는 D1-11-ART와 후속 사용자 Editor 조정으로 분리한다.
- Fifth Preview에서 구조적으로 확인된 D1-07 외곽 배치·중앙 시야·Root/Panel/Element 분리는 Structure sanity PASS 증거로 유지한다. Placeholder 아트 자체는 더 이상 D1-11 Structure 단독 차단 사유가 아니다.
- `CFUIHUDProdEditorBridge v1.2.0` Source에서 `SpeedArcTrack`이 None이거나 Load 실패일 때 `Image_RPMTrackArt`를 `Collapsed`로 저장하도록 수정하고, Brush Resource 유무와 Visibility의 일치 여부를 Validator에 추가했다.
- 공식 Build `d7f30ed2ced04f0a8c4387b0d73c65f7`에서 UHT와 `CFUIHUDProdEditorBridge.cpp` Compile은 PASS했으나 실행 중인 `UnrealEditor.exe`가 `UnrealEditor-CarFight_Re.dll`을 잠가 Link가 LNK1104 / Exit 6으로 중단됐다. 코드 Compile FAIL로 분류하지 않는다.
- 현재 열린 Asset은 `/Game/CarFight/UI/HUD/WBP_CFInGameHUD` 하나이며 UE `AssetTools.is_dirty=false`를 확인했다. 사용자의 unrelated 저장 상태를 임의 추정하거나 Editor를 강제 종료하지 않았다.
- 따라서 `D1-11 = NOT PASS`를 유지한다. Editor 종료 후 공식 Build → Production Apply → Saved Readback → Fresh AssetDump를 PASS해야 Structure Gate를 닫는다.
- D1-11-ART-01 Source Art는 사용자 승인 전 Import/연결하지 않으며 D1-12·UI-P0-03은 시작하지 않았다.

### v0.14.0 - 2026-08-13

- 사용자가 새 Production `WBP_CFInGameHUD`의 1920×1080 Designer Preview를 제공해 D1-11 다섯 번째 User Preview를 실제 화면 기준으로 검토했다.
- D1-07 승인 외곽 배치, 중앙 전투 시야 보호, Root 7 Slot과 의미 단위 Panel 구조는 화면에서도 유지됨을 확인했다.
- VehiclePanel SpeedGauge에서 `SpeedArcTrack=None` 상태의 `Image_RPMTrackArt`가 숨겨지지 않고 AccentTactical Tint가 적용된 큰 청록색 사각형으로 렌더링되는 Visual FAIL을 확인했다. 21 RPM Tick과 `076 / km/h / N` 자체는 존재하지만 배경 사각형이 Tachometer 판독성을 훼손한다.
- `DA_CFHUDVisual_Default`의 VehicleSilhouette·ArmorPlates·SpeedArcTrack 등 전용 HUD Art가 아직 None이므로 Vehicle/Armor가 Semantic Fallback Placeholder로 보이며, 현재 화면은 Production 최종 시각 승인 대상으로 보기 부족하다고 판정했다.
- 따라서 다섯 번째 Production 1920×1080 Preview는 FAIL이며 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.
- 다음 D1-11 수정 우선순위는 전용 RPM Track Texture가 없을 때 `Image_RPMTrackArt`를 그리지 않도록 안전 처리하고, 승인된 HUD Visual Art 연결 전후를 분리해 다시 Preview하는 것이다.
- 이번 Preview 판정에서는 Source·Config·Unreal Asset을 수정하지 않았고 기존 미커밋 변경을 보호했으며 commit·push는 수행하지 않았다.

### v0.13.0 - 2026-08-12

- D1-11 Pre-Preview Contract FAIL 세 항목만 최소 교정했다. `CFUIHUDProdEditorBridge`를 v1.1.0으로 갱신하고 Runtime/ViewData·D1-12·UI-P0-03은 변경하지 않았다.
- `WBP_CFSpeedGauge` 생성 계약에서 전진 `D`, `ProgressBar_SpeedFallback`, Cooldown Semantic 기반 `Image_SpeedArcArt`를 제거했다. Designer Preview는 `Gear=N`, 고정 21 Tick, 85% Red Zone, 좌측 세로→곡선→상단 수평 비대칭 RPM Scale을 사용하며 실제 RPM Provider가 없는 동안 현재 RPM 비율을 만들지 않는다.
- `WBP_CFArmorBodyMap` 생성 계약에서 여섯 `Text_Armor*` Current/Maximum Text를 제거하고 각 Plate/Badge 우측에 `ProgressBar_ArmorFront/Right/Rear/Left/Top/Bottom` 세로 Bar를 추가했다. Bar는 `BottomToTop` Fill과 D1 Visual Mock `NormalizedArmor`만 사용한다.
- Validator가 SpeedGauge의 Gear `D`·구형 SpeedFallback·구형 SpeedArcArt 회귀, 21 Tick 누락과 Armor 6 Bar 누락·`Text_Armor*` 재도입을 즉시 FAIL하도록 보강했다.
- 공식 Build Job `9416a5877d8e47a5a80ac552d5e7e76d`가 `CFUIHUDProdEditorBridge.cpp` Compile과 `UnrealEditor-CarFight_Re.dll` Link를 포함해 Exit 0 PASS했다.
- Production Apply `a3fe8f3981f542c483d607d7e88eca93`와 독립 Saved Readback `9972afbf11df44a7aac5d731e9010a3a`가 Exit 0 PASS했다. exact 10 Asset, Semantic/Image Structure PASS, Border Mosaic·Gameplay Cast·Runtime Binding·D1-12·UI-P0-03는 0을 유지했다.
- Fresh AssetDump dataset `adset_v1_b17fbe9392114d7f155e6c74ad48f141.4ee866a52a57c78493666817`은 `/Game/CarFight/UI/HUD` WidgetBlueprint 9/9 Success를 기록했다. SpeedGauge는 `Image_RPMTrackArt` + RPM Tick 00~20 + `076 / km/h / N`, ArmorBodyMap은 Vehicle Silhouette + 6 Plate + 6 세로 Bar만 보유하고 Root는 기존 7 Slot과 ReticleLayer를 유지한다.
- Pre-Preview Contract FAIL은 교정 완료로 닫고 새 1920×1080 Designer/User Preview Gate를 READY·Pending으로 전환했다. 실제 User Visual PASS 전 `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.
- 기존 미커밋 변경과 unrelated 에셋은 보호했고 commit·push는 수행하지 않았다.

### v0.12.0 - 2026-08-12

- D1-11 새 Production 1920×1080 Designer/User Preview Gate를 재개하기 전에 실제 저장 Asset을 fresh AssetDump와 공식 Production Readback으로 다시 검증했다.
- `/Game/CarFight/UI/HUD` WidgetBlueprint 9/9를 읽고 `WBP_CFInGameHUD` Root가 D1-07 LayoutData 기반 여섯 Panel Slot + Full Stretch ReticleLayer 구조를 유지하며 Production 의미 단위 Widget Composition과 Border Mosaic 0을 확인했다.
- 공식 `Tools/RunUIHUDProduction.ps1 -Readback` Process `3d301f3b3d4c4066a626b6081e010e6e`가 Exit 0으로 PASS했다. 보고서는 exact mutable asset 10, Root 1, Panel 6, Element 2, HUD Visual Data 1, Semantic Child Structure PASS, Image/Brush Structure PASS, Border Mosaic 0, Gameplay Cast 0, Runtime Binding 0, UI-P0-03 0, D1-12 0을 기록했다.
- 같은 보고서가 `preview_1920x1080_user_gate=Pending`, `d1_11_pass=false`를 유지하므로 자동 Readback만으로 User Gate를 PASS 처리하지 않았다.
- 최신 `InGameUIVehiclePanelSpec.md v0.20.0`과 저장 Designer Tree를 대조한 Pre-Preview Contract Review에서 세 가지 차단 요소를 확정했다: `Text_Gear="D"`는 전진 시 실제 기어 숫자만 사용한다는 단일 Gear Slot 계약 위반, 여섯 `Text_Armor*` Current/Max는 Armor 수치 Text 제거 + Plate/Badge 우측 세로 `NormalizedArmor` Bar 계약 위반, `ProgressBar_SpeedFallback` 및 Cooldown Semantic Texture 기반 Speed Arc Fallback은 실제 Engine RPM Tachometer와 가짜 RPM 금지 계약 위반이다.
- 따라서 D1-07 승인 Root 배치·Production 의미 단위 구조·ReticleLayer 중앙 보호영역은 Structural PASS를 유지하되, 현재 저장 Production Designer 콘텐츠는 **Pre-Preview Contract FAIL**로 판정했다. 새 1920×1080 User Preview는 교정 전 Blocked이며 `D1-11 = NOT PASS`다.
- 이번 Gate에서는 Source·Config·Unreal Asset을 수정하지 않았고 D1-12와 UI-P0-03을 시작하지 않았다. 기존 미커밋 변경과 에셋을 보호했으며 commit·push는 수행하지 않았다.

### v0.11.0 - 2026-08-10

- D1-11 Border Mosaic 폐기 결정에 따라 Production UMG 구조를 실제 구현했다. Root 1 + 기능 Panel 6 + 의미 Element 2 + `DA_CFHUDVisual_Default` 1의 exact 10 Asset 계약이며 Root Canvas는 D1-07 Slot 배치만 소유한다.
- `UCFUIStyleData v1.2.0`에 Semantic ID→Soft Icon Asset Resolver와 무효/중복 Icon 검증을 추가하고 `UCFHUDVisualData v1.0.0`에 Vehicle Silhouette·6방향 Armor Plate·Speed Arc·Radar·Selected Target/Weapon 전용 아트 Soft Reference를 분리했다. Semantic Icon 18개는 기존 `DA_CFUIStyle_Default.IconSet`을 그대로 재사용한다.
- `CFUIHUDProdEditorBridge v1.0.0`은 Panel Border 1·Element Border 0, 실제 Image 슬롯, TextBlock·ProgressBar, VehiclePanel의 `WBP_CFSpeedGauge`·`WBP_CFArmorBodyMap` 중첩 Class, Root 7 Slot과 Runtime Graph 0을 검증한다. 기존 `CFUIHUDEditorBridge v1.x`는 Historical Mock 재현용으로만 보존한다.
- 공식 Build `2e63b4a2ee77448a95bd645ac23b47cb`, Production Bridge Build `5653c410a2674232aa5477f12437c91d`, Probe `ac83c6588cdb49dca292e0653774d08b`, DryRun `3e3ff6fa152942fca0f555083779632c`, Apply `3c7dfff73b4042f895e98989c825c9d1`, Saved Readback `eac37017a42f4a46989c98691d3fdb9f`가 모두 PASS했다.
- Apply는 신규 9 Asset 생성 + 기존 `WBP_CFInGameHUD` 동일 Asset 재구성, WidgetBlueprint 9/9 Compile, 전체 10 Save를 완료했다. Border Mosaic 0, Gameplay Cast 0, Runtime Event Binding 0, D1-12 Asset 0, UI-P0-03 ViewData 0이다.
- 독립 AssetDump는 `/Game/CarFight/UI/HUD` WidgetBlueprint 9/9와 전체 HUD Asset 10/10 Success·실패 0을 확인했다. 보호 회귀 `05faa8326c7848a5b1ca18525b59540c`는 전체 CarFight 53/53·필수 24/24 Success다.
- 새 Production 1920×1080 Designer/User Preview가 Pending이므로 `D1-11 = NOT PASS`, D1-12 = Prepared·Not Started를 유지한다.

### v0.10.0 - 2026-08-10

- 네 번째 1920×1080 User Re-preview에서 v1.3.0의 배치와 의미는 가까워졌지만 `Canvas_Mock_VehiclePanel` 등에서 Border를 다수 배치해 차체·바퀴·Armor Plate·Speed Arc·장식선을 만드는 구조가 Production UI 의도와 다르다는 사용자 피드백으로 FAIL 판정했다.
- 기존 `InGameUIVehiclePanelSpec`은 이미 `WBP_CFSpeedGauge`, `WBP_CFArmorBodyMap` 의미 단위 UMG 구조와 Blueprint/UMG 소유의 Armor Plate 도형·Icon·Typography를 요구한다. 현재 flattened Border Mock은 이 구조를 따르지 못한 것으로 판정한다.
- D1-11 Visual Composition Rework 원칙을 추가했다: Text=TextBlock, 정적/교체 가능 시각 자산=Image/Brush, 스타일화된 동적 도형·게이지=UI Material, 선형 상태값=ProgressBar 또는 UI Material, Border=실제 Panel Surface/Container에만 사용. Border를 작은 조각으로 배치해 그림을 그리는 방식은 금지한다.
- v1.3.0 Build·Readback·AssetDump·53/53 회귀는 Layout/Semantic 안전성 역사 증거로 보존하지만 사용자 Visual Gate를 대체하지 않는다. `D1-11 = NOT PASS`, D1-12·UI-P0-03 = Not Started를 유지한다.

### v0.9.0 - 2026-08-10

- 세 번째 D1-11 1920×1080 User Re-preview에서 `NamedSlot_*` 잔상 제거와 전체 Slot 배치는 통과했지만 VehiclePanel의 Armor 의미별 위치가 D1-07 계약과 불일치하고 Speed Arc·왼쪽 전방 차량 Silhouette의 판독성이 부족해 USER FAIL로 판정했다.
- `CFUIHUDEditorBridge v1.3.0`은 Armor 위치·값을 `좌 Front=30/100 / 상 Right=70/100 / 우 Rear=60/80 / 하 Left=63/100 / 좌상단 Top=84/100 / 우하단 Bottom=72/100`으로 정확히 고정하고 자동 Validator가 의미별 Text 값과 Canvas Rect를 함께 확인한다. Speed Arc는 조밀한 Segment 열로, 차체는 Nose Shoulder·Tail·Cabin Outline·Wheel로 왼쪽 전방 방향 판독성을 강화했다.
- v1.3 Source Build `4e455496158f4c9bb25ee9851bb65718`, DryRun `abf8def22ad54d69a174671f926b2b9c`, Rebuild Apply `3dcf10ee81fc4e1b938849ff1260bc5d`를 PASS했다. 병행 AssetDump 작업 중 Engine Module Manifest BuildId가 달라져 Commandlet 검증이 HUD Python 실행 전에 일시 Blocked됐지만, 공식 Build `608a05d40bab49e7abfed3073ee1914b`에서 변경된 AssetDump 모듈까지 Compile·Link해 정합성을 복구했다.
- 복구 후 Saved Readback `e8a74175779c41a3a6a9186bcbf6f087`, HUD AssetDump WidgetBlueprint 1/1·Failed 0, 보호 회귀 `01dc9e734d4946208d5de53ccc3f39fa` 53/53·필수 24/24를 PASS했다.
- D1-11 전용 Source에서 Gameplay Actor/Component 조회, 직접 Asset Load, Runtime Dynamic Binding은 0이며 D1-12·UI-P0-03은 시작하지 않았다. 네 번째 사용자 Preview 전까지 `D1-11 = NOT PASS`를 유지한다.

### v0.8.0 - 2026-08-10

- 두 번째 D1-11 1920×1080 User Re-preview에서 Slot·Target·Radar Contact·Weapon Compact는 개선됐지만 VehiclePanel의 D1-07 승인 좌측 원호형 Speed Gauge와 왼쪽 전방 차량 Silhouette가 없고 Radar·Target·Weapon 위에 중첩 D1-10 Base Widget `NamedSlot_*` Designer Placeholder가 남아 있어 USER FAIL로 판정했다.
- `CFUIHUDEditorBridge v1.2.0`은 Speed Arc 정적 Segment와 Nose·Cabin·Wheel 기반 왼쪽 전방 차량 Silhouette, 위치 기반 Armor 값 계약을 추가했다. 중첩 D1-10 Base Widget과 Target InfoRow는 Class 재사용 증거를 보존하면서 Collapsed·RenderOpacity 0·화면 밖 Canvas Storage로 이동해 Designer Placeholder 회귀를 막는다.
- `ApplyUIHUDPrototype.py v1.2.0`은 동일 `WBP_CFInGameHUD` 단일 Asset 재구축 경계를 유지하고 세 번째 User Re-preview Pending 상태를 보고한다.
- 공식 Build `69132f9359744251a2eeb8b55dec5322`, DryRun `ea3666f226e94ecdadc57774c001a27f`, Rebuild Apply `27682e43292b4121b24172119f84faa2`, Saved Readback `47e3be11d32c4784bc21b39bdd0924f4`, HUD AssetDump 1/1과 보호 회귀 `c4f6d70abcb74270bf70d18595922558` 53/53·필수 24/24를 PASS했다.
- Gameplay Actor/Component 조회, 직접 Asset Load, Runtime Binding, D1-12 Asset, UI-P0-03 View Data는 계속 0이며 세 번째 사용자 Preview 전까지 `D1-11 = NOT PASS`를 유지한다.

### v0.7.0 - 2026-08-10

- 첫 D1-11 1920×1080 사용자 Designer Screenshot에서 Root Slot bounds는 대체로 맞았지만 실제 콘텐츠가 Slot 좌상단에 축소되고 Vehicle·Radar·Weapon·Target 내부 Visual이 D1-07 `SR-1080-16`보다 지나치게 단순해 Visual Preview FAIL로 판정했다.
- `CFUIHUDEditorBridge v1.1.0`에서 모든 Mock Surface를 Overlay Fill + Panel-local Canvas 구조로 바꾸고 Visual Fidelity Validator를 추가했다. Vehicle은 Speed/DNR·6방향 Armor·Shield·Integrity, Radar는 공간형 Contact Field와 Selected Bracket, Weapon은 Compact 상태·Rail 3개, Target은 전체 폭 Info/Scan 구조를 갖는다.
- `ApplyUIHUDPrototype.py v1.1.0`은 기존 Target의 Native Parent를 보호하면서 같은 `WBP_CFInGameHUD` Asset의 WidgetTree만 재구축한다. D1-09B Style/Density/Layout과 D1-10 Base Widget Asset은 수정하지 않는다.
- 공식 Build `f28ff1c21e5d4e34824fc43e0f1f0cfc`, DryRun `71f2015ca8a94ea993475e94b689dce2`, Rebuild Apply `c531a16ea2ee48f6adc2c290ce50f302`, Saved Readback `8560bcbda6204ad0a106abe360e4dbed`, 독립 AssetDump 1/1과 보호 회귀 `10baf383a2204fb680dc65a91534c4d2` 53/53·필수 24/24를 PASS했다.
- Gameplay Actor/Component 조회, 직접 Asset Load, Runtime Binding, D1-12 Asset, UI-P0-03 View Data는 계속 0이다.
- 자동 Visual Fidelity Fix는 PASS지만 수정 후 사용자 1920×1080 재Preview가 Pending이므로 `D1-11 = NOT PASS`, D1-12 = Not Started를 유지한다.

### v0.6.0 - 2026-08-10

- D1-11만 재개해 Editor 전용 `CFUIHUDEditorBridge`, `ApplyUIHUDPrototype.py`, `RunUIHUDPrototype.ps1`을 추가하고 `/Game/CarFight/UI/HUD/WBP_CFInGameHUD` 한 개만 생성했다.
- D1-09B Style·Standard/Compact Density·1080 Layout과 D1-10 Panel/Alert/InfoRow Base를 읽기 전용 입력으로 사용하고, 정확한 7-Slot·D1-07 Mock·Typography Floor·Weapon Compact·중앙 시야 보호를 정적 검증하도록 했다.
- Probe `a72fac6ee3ae4e199041773f2711a446`, DryRun `c020e0f2f39144e5a595c1aab31aed51`, Apply `1831d3f2ce8740e6aff814662d1cf421`, 최종 Saved Readback `2b283ba7bd2842b483bccabafcbf7bc7`을 PASS했다.
- 첫 Apply `d1f181a339334f519a636ad71c35e576`은 UE가 새 Widget Blueprint에 자동 생성하는 미연결 기본 EventGraph 3노드를 Runtime Logic으로 과잉 판정해 Save 전 중단됐다. Validator는 기본 `K2Node_Event`가 모든 Pin 미연결일 때만 허용하고 Event 외 Node 또는 연결 Pin은 계속 FAIL하도록 정밀화했다.
- 최종 공식 Build `e79da92637754ce280a956e7624fc6c6`은 `CFUIHUDEditorBridge v1.0.3` Compile·제품 DLL Link Exit 0이다.
- 독립 AssetDump는 `/Game/CarFight/UI/HUD` WidgetBlueprint 정확히 1/1 Success, Parent `CFStyledWidgetBase`, Root `CanvasPanel_Root`, WidgetTree 67, Reference 0, Widget Binding 0, Animation 0을 확인했다.
- 최종 보호 회귀 `fd7b421ba5ba4963ae0d4a19f3273906`은 53/53 Success·필수 24/24 Success·실패 0이다.
- D1-11 자동/정적 범위는 PASS했지만 1920×1080 Designer/User Preview가 Pending이므로 `D1-11 = NOT PASS`다. Style/Density 변경만으로 전체 표현이 함께 갱신되는지도 Preview에서 확인해야 한다.
- D1-12와 UI-P0-03은 Not Started를 유지했고 기존 D1-09B/D1-10 및 unrelated dirty 자산을 보호했다.

### v0.5.0 - 2026-08-10

- D1-10B 재개 후 공식 Editor Build Job `ca38c51a4e264ca79e8fa018eba4584f`에서 `CFUIBaseEditorBridge` UHT·Compile·제품 DLL Link를 Exit 0으로 확인했다.
- UE 5.8 Python의 Bool + Out FString 마샬링에서 Bool ReturnValue가 직접 노출되지 않는 동작을 확인하고, 기존 상세 실패 API를 보존한 bool-only Build/Validate 래퍼로 최소 보정했다.
- Probe `ac14d231ec2248a29147187402061748`, DryRun `83b87a0b61fd40beb676d827de16ce61`, Apply `2b5959aca44443829e6f58fd68ad1f76`, 저장 후 별도 Readback `93d974ce5c854c6da46ee5cbb87942d6`을 PASS했다.
- Allowlist의 정확한 Base Widget 5종만 생성했고 5/5 Compile·정확 WidgetTree·Native Parent·`Button_Interaction` BindWidget·Style/Density 외부 Context를 검증했다. Gameplay Cast 0, DataAsset 직접 Load 0, Collapsed Spacer 0, Graph CallFunction 0이다.
- 독립 AssetDump에서 `/Game/CarFight/UI/Base` WidgetBlueprint 5/5 Success를 확인했다.
- 전체 보호 회귀 Process `3c8fcd4a7438478db83eefbbd0719de0`은 53/53 Success·필수 24/24 Success·실패 0이며 기존 TargetHud와 D1-10A 두 Contract도 Success다.
- `D1-10 = PASS`로 닫고 `D1-11 = Prepared / Not Started`를 유지했다. 이번 작업에서는 D1-11을 시작하지 않았다.
- 기존 미커밋 변경과 D1-09B 자산을 보호했고 commit·push·reset·checkout·stash·clean은 수행하지 않았다.

### v0.4.0 - 2026-08-10

- D1-10A `UCFStyledWidgetBase`, `UCFButtonBaseWidget`, `CFUIBaseWidgetTests`를 구현하고 공식 Build Job `322535f4c9ab4b74843125afb0a8a970` PASS를 기록했다.
- `CarFight.UI.D1_10A.VisualContextContract`, `CarFight.UI.D1_10A.ButtonBridgeContract`가 Automation에서 Success임을 확인했다.
- UE 5.8 Python Reflection으로 WidgetBlueprint Factory·Blueprint 변수·Compile API는 공개되지만 `UWidgetBlueprint::WidgetTree`가 직접 공개되지 않음을 확인했다.
- 정확한 Designer Tree 생성용 최소 `CFUIBaseEditorBridge` Source를 추가했으며 C++ Compile은 성공했으나 열린 Editor의 `UnrealEditor-CarFight_Re.dll` 점유로 공식 Link Job `ccb393ff7d894089bf958de9b9c5064b`가 LNK1104로 중단됐다.
- 열린 일반 Editor는 임의 종료하지 않았고 `/Game/CarFight/UI/Base/` 5종 Asset을 생성하지 않았다.
- D1-10은 NOT PASS, D1-10B는 Blocked, D1-11은 Prepared/Not Started로 유지했다.
- commit·push는 수행하지 않았다.

### v0.3.0 - 2026-08-10

- D1-09B FontFace 8개, Runtime Composite UFont 2개, Semantic Icon 18개, Style/Density/1080 Layout DataAsset 5개와 Font Notice 2개를 실제 생성했다.
- `UE/Config/DefaultGame.ini`에 `CFUISubsystem` 기본 Style·Standard Density·1080 Layout Soft Reference 3개를 연결했다.
- `ApplyUIAssetization.py v1.1.1`과 `RunUIAssetization.ps1 v1.0.2`로 DryRun → Full Editor Apply → 새 프로세스 Readback을 수행했다.
- Readback Process `d09097cc7781470ba9fd4261828a2c14`에서 Asset·License·Config Contract와 `d1_09b_pass=true`를 확인했다.
- 전체 회귀 중 `WBP_TargetSelect.uasset` Windows Error Code 32 잠금으로 `TargetHud` 1건이 두 번 실패했으나 TargetSelect를 수정하지 않았고, 잠금 해제 후 Process `e73311a38be043d48d512ab90afd258f`에서 51/51·필수 24/24 Success를 확인했다.
- 1440p·21:9·32:9 Layout, Base Widget, HUD Prototype, Pause Visual은 생성·착수하지 않았다.
- D1-09B를 PASS로 닫고 D1-10을 Not Started / Next로 유지했다.
- commit·push는 수행하지 않았다.

### v0.2.0 - 2026-08-10

- D1-08V 공식 Editor Build Job `0345aafbcc9341deb60b44791c528f84`와 `StyleDataContract`를 포함한 47/47 전체 Automation PASS를 기록했다.
- D1-09A Font Binding·HUD Layout·Density 타입과 `UCFUISubsystem` Config Soft Reference·Native Fallback을 구현했다.
- `CFHUDLayoutData`는 D1-07 USER PASS 1920×1080 7-Slot과 Geometry/Typography Scale·Font Floor를 Native Fallback으로 보존한다.
- `CFUIDensityData`는 Compact·Standard·Expanded 승인값을 Native Preset으로 제공한다.
- `CFUIStyleData v1.1.0`은 UI/Numeric `UFont` Hard Reference와 Weight→Typeface·Slate Font 해석을 추가했다.
- 중간 실패 두 건은 TArray Self-reference와 ClassWithin Outer 계층을 잘못 구성한 Automation Fixture 문제였으며 제품 Runtime 결함 없이 테스트 v1.0.3에서 수정했다.
- 최종 Build Job `9d9aefb3803e4556bd4559afded0f4ed` PASS, Automation Process `18bea9b5952e4444b618e776cb2e2571` 51/51·필수 24/24 Success, D1-09A 네 Contract 모두 Warning 0 / Error 0을 확인했다.
- D1-09B Unreal Asset·Config는 아직 시작하지 않았고 commit·push도 수행하지 않았다.

### v0.1.0 - 2026-08-07

- Pretendard와 IBM Plex Mono의 공식 OFL 1.1 라이선스를 검토해 상용 게임 번들 사용 적격성을 PASS로 확정했다.
- Font 재배포 시 Copyright Notice와 OFL 전문 보존 요구, Reserved Font Name 조건과 Shipping 고지 후속 검증을 기록했다.
- Epic 최신 Font 문서를 기준으로 `UFont` Runtime Composite + `UFontFace` Static Face + `FSlateFontInfo` 해석 구조를 확정했다.
- D1 P0 Font는 Static TTF/OTF 4 Weight, Runtime Cached Composite Font, Hinting Default, Loading Policy Lazy Load, Layout Metrics를 기본값으로 확정했다.
- D1-09A의 정확한 C++ Allowlist, Font Hard Reference, Layout/Density 타입, 1080p Geometry/Typography Scale, Subsystem Config Soft Reference와 Automation 계약을 작성했다.
- D1-09B의 Style·Density·1080 Layout·Font·Semantic Icon Asset Allowlist와 라이선스 고지 경로를 작성했다.
- D1-10을 C++ Visual Context Bridge와 Base Widget 5종 제작으로 분리하고 Widget Tree·입력·상태·책임을 고정했다.
- D1-11을 기능 없는 1920×1080 Visual Prototype으로 제한하고 Screen-space Layout과 내부 Token Scale 규칙을 확정했다.
- D1-12의 Designer Tree / Native Fallback, Pause Class Soft Reference, Continue Focus Bridge와 기존 Pause Runtime 보호 범위를 확정했다.
- Source·Config·Unreal Asset·Build·Automation·PIE·commit·push는 수행하지 않았다.

---

## 14. Migration

- D1-09~12 구현 준비 상세는 이 문서를 우선 사용하고 `InGameUIStyleSpec.md`는 공통 Style Token과 디자인 규칙 SSOT 역할을 유지한다.
- Font Asset은 `UFont` Composite Font 두 개를 Style Data가 Hard Reference하고, Style Data 자체는 `UCFUISubsystem`이 Config Soft Reference로 한 번 해석하는 구조를 사용한다.
- 1080p HUD Slot은 기존 D1-07 USER PASS Screen-space 값을 유지하며 내부 Token만 `GeometryScale=0.75`, Typography는 `TypographyScale=0.75 + Effective Floor`로 해석한다.
- Base Widget의 시각 적용은 C++ Context Bridge + Blueprint Visual 역할로 분리한다.
- Pause Menu는 Native Fallback을 제거하지 않고 WBP가 정상일 때만 Visual Class를 교체한다.
