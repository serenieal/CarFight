# UI Asset-First Trial 05 — Handoff Plan

- 문서 버전: v0.8
- 최근 갱신일: 2026-08-27
- 상태: Method Validation Complete / WeaponPanel Production Expansion Cancelled / VehiclePanel Transfer Complete
- 범위: AI-assisted UI Resource 제작 방법론 검증
- Production/UE 적용: 없음

## 1. 전환 이유

Trial 01~04에서 두 극단을 확인했다.

- Generative Concept First: 시각적으로 풍부하지만 실제 Production Source로 역변환할 때 Reality Gap이 큼.
- Production-Proven Primitive Only: 실제 구현 가능성은 높지만 시각적 밀도와 매력이 부족함.

따라서 Trial 05의 가설은 다음이다.

> 완성 Concept을 먼저 만든 뒤 Asset을 뜯어내지 말고, 실제로 사용할 Production Asset을 먼저 만든 뒤 그 Asset만으로 Concept을 합성하면 현실화 가능성과 시각적 매력을 동시에 높일 수 있는가?

## 2. 핵심 Workflow

Style Direction → Asset Inventory → Asset별 Production Method 결정 → 실제 독립 Asset 제작 → 실제 Asset만 사용해 Concept Composite → USER Review → 같은 Asset을 Runtime Packing/Consumer로 전달

Hard Rule: Concept에 보이는 주요 시각 요소는 Concept 이전 또는 동시에 독립 Production Source로 존재해야 한다. 나중에 Flat Concept에서 다시 잘라내거나 가려진 구조를 복원하는 것을 기본 경로로 사용하지 않는다.

## 3. Trial 대상

초기 대상은 Weapon Charge Module을 유지한다.

- 실제 WeaponCharge Runtime/ViewData/Presenter 경로가 Production Proven이다.
- 작은 독립 Module이라 Asset-First workflow 검증에 적합하다.
- Text/ProgressBar 같은 Proven runtime element와 decorative asset을 명확히 분리할 수 있다.
- 기존 VehiclePanel 전체의 복잡한 Frame 문제를 끌고 오지 않는다.

Trial 05는 실제 Production WeaponPanel을 수정하지 않는다.

## 4. Asset Inventory 최종 Lock

Runtime/Primitive — Production Proven / Concept 사용 가능:
- ChargeLabel
- CurrentMaximumReadout
- ChargeProgressBar
- InsufficientStateText
- DarkSurface

Mandatory Structural Asset — 실제 독립 Source 생성 완료:
- FrameBase
  - Source: `URT05_FrameBase.json`
  - Method: Explicit Path JSON CompoundPath / even-odd outer housing + editable corner braces
- InnerRail
  - Source: `URT05_InnerRail.json`
  - Method: Explicit Path JSON StrokePath / edge rail 6개 독립 ID
- AccentStrip
  - Source: `URT05_AccentStrip.json`
  - Method: Explicit Path JSON FilledPath / primary strip 2 + marker 2 독립 ID

Structural Contract Owner:
- `URT05_StructSpec.json v0.2.0`

Decorative Asset — ROI 판정 후 선택 Source 생성:
- CornerDetail
  - Source: `URT05_CornerDetail.json`
  - Raster QA: PASS
- SubtleGlowStrip
  - Source: `URT05_GlowStrip.json`
  - Raster QA: PASS
- SurfaceNoise
  - 상태: Deferred
  - 이유: 68px 높이 대비 readability·packing 비용이 커서 현재 5-Source 조합보다 ROI가 낮음

Lock 규칙:
- Runtime/Primitive 5종 + Mandatory Structural 3종 + 선택 Decorative 2종이 현재 Composite 허용 집합이다.
- SurfaceNoise는 Deferred이며 USER Review 결과가 새 필요성을 만들기 전에는 추가하지 않는다.
- 각 Source Asset은 서로 독립 파일이며 요소 ID 단위로 부분 수정 가능하다.
- Flat Composite에서 역추출하지 않는다.
- AI 생성 사용 시에도 독립 Asset 단위로 생성한다.
- 실제 Asset이 없는 장식은 Concept에 넣지 않는다.
- Glow/Noise가 없어도 Core Identity가 무너지지 않아야 한다.

## 5. Common Canvas Contract — Locked

- Module Canvas: **720 x 68**
- Origin: Top-Left
- Background: Transparent
- Structural Source Authority: **Explicit Path JSON**
- Derivative Canvas/ViewBox: `0 0 720 68`
- Structural Base Color: White Alpha / tintable
- SVG/PNG: Source가 아니라 JSON에서 생성하는 deterministic derivative
- No baked text
- No baked runtime values
- No filter/glow effect in the mandatory structural layer

직접 `.svg` text write가 Project write allowlist에서 차단되므로 Source authority를 SVG로 억지로 우회하지 않는다. Plan 초안이 허용한 `SVG 또는 explicit path` 중 Explicit Path JSON을 canonical source로 선택했다. `Tools/MakeURT05Sources.py v1.2.0`은 local reference derivative generator로 유지한다. Project bounded execution의 canonical runner는 `Tools/RunURT05Sources.ps1 v1.0.1`이며 Accepted `powershell51` generic-script lane에서 외부 executable 없이 같은 JSON Source 5종을 SVG derivative로 생성·검증한다. 어느 generator도 geometry authority를 소유하지 않으며 authority는 Source JSON 5종에만 있다.

## 6. 현재 진행 상태와 다음 Gate

완료:
1. Asset Inventory 최종 Lock.
2. FrameBase / InnerRail / AccentStrip 제작 방식을 Explicit Path JSON으로 확정.
3. 720×68 Common Canvas Lock.
4. 실제 독립 Source 3개 생성.
5. 각 Source의 독립 파일 / 요소 ID / baked text 0 / baked runtime value 0 readback 확인.
6. JSON 단일 authority → SVG derivative generator 구조로 교정.

Structural Raster QA 결과:
- FrameBase: TECH PASS / readable / generic identity
- InnerRail: TECH PASS / low additional identity gain
- AccentStrip: TECH PASS / directional but sparse
- Overall: **TECH PASS / VISUAL IDENTITY INSUFFICIENT**
- Evidence owner: `URT05_StructQA.json v0.1.0`

Decorative ROI 판정:
- `CornerDetail`: 선택 / Source 생성 / Raster QA PASS
- `SubtleGlowStrip`: 선택 / Source 생성 / Raster QA PASS
- `SurfaceNoise`: Deferred / 68px 높이 대비 readability·packing 비용이 더 큼
- Decorative contract: `URT05_DecoSpec.json v0.2.0`
- Decorative QA: `URT05_DecoQA.json v0.1.0`

Project bounded derivative 결과:
1. GoPyMCP Current contract를 확인해 generic script lane이 `repository_id + runtime=powershell51 + repository-relative .ps1` 계약임을 확정했다.
2. CarFight configured repository ID `main_game`으로 `Tools/RunURT05Sources.ps1 v1.0.1`을 bounded 실행했다.
3. 최종 실행은 exit code 0 / `URT05_RESULT=PASS`.
4. FrameBase / InnerRail / AccentStrip / CornerDetail / GlowStrip SVG 5종 모두 XML parse PASS / 720×68 PASS / baked text 0 / filter 0.
5. Windows PowerShell 5.1 UTF-8-no-BOM 영향으로 최초 generated `<desc>`가 깨지는 결함을 발견해 ASCII-only desc로 교정하고 fresh PASS + readback했다.

Color Hierarchy:
- 기존 Source geometry는 바꾸지 않고 `Dark Metal + Neutral Secondary + Cyan Accent` 계층을 적용했다.
- USER는 grayscale 대비 개선은 인정했으나 최종 Composite에 대해 `나쁘지 않지만 막 좋다고 하기도 어렵고 SO-SO`로 평가했다.
- 따라서 `RecommendedBalance`는 유효한 방향 참고값이지만 Production Visual PASS가 아니다.

Asset-only Composite Preview:
- Owner: `URT05_CompositeSpec.json v0.1.3`
- 사용 Source: 실제 독립 Source 5종만 사용.
- Runtime primitive: ChargeLabel / CurrentMaximumReadout / ChargeProgressBar / InsufficientStateText / DarkSurface만 사용.
- Normal 68/100과 Insufficient 0/100 두 상태를 구성.
- 신규 형상 0 / unmapped runtime data 0 / SurfaceNoise 0.
- 첫 Preview에서 NO CHARGE가 lower accent와 겹친 것을 발견해 right-side bar gap의 y=34로 조정했다.
- Production/UE mutation은 0.

다음 Gate:
1. Visual Richness Polish 1회 뒤 USER가 `이 정도면 괜찮다`고 평가해 **Visual Baseline Candidate Accepted**로 HOLD를 해제했다.
2. 이 승인은 final in-game Production Visual PASS가 아니라 Production 구현으로 넘어갈 기준안 승인이다.
3. Production Handoff owner는 `URT05_ProdHandoff.json v0.2.0`이다.
4. Authoring Source 5종은 독립 편집성을 유지하지만 Runtime은 `FrameBase+InnerRail+CornerDetail → Frame 1장`, `AccentStrip+SubtleGlowStrip → Accent 1장`으로 2개 시각 Asset만 패킹한다.
5. DarkSurface/ProgressBar/Text/NO CHARGE는 기존 UMG/Presenter semantic owner를 재사용한다.
6. `Tools/PackURT05Prod.ps1 v1.0.0`으로 `URT05_WeaponResFrame.png` + `URT05_WeaponResAccent.png` 720×68 Runtime Pack 2종을 Project 내부에서 생성 PASS했다.
7. 다음 Gate는 이 두 PNG의 UE Import → HUDVisualData Soft Reference → 기존 WeaponPanel Designer Tree additive 적용 → Presenter label/value/caution semantic 연결이다. 현재 UE Import/Production Asset mutation은 아직 0이다.

**현재 Visual Baseline Candidate는 Production 구현 진입 기준으로 Accepted됐지만 final in-game Production Visual PASS는 아직 아니다.**

## 7. 성공 기준

- Visual Appeal: USER가 실제로 만들고 싶다고 느낄 수준
- Asset Reality: Concept 주요 요소 100% 실제 Source 존재
- Editability: 부분 수정으로 전체 Composite 재생성 불필요
- Reusability: 다른 UI에도 일부 Asset 재사용 가능
- Runtime Suitability: 기존 UMG/Texture/Material consumer로 전달 가능
- Production Surprise: 큰 후발 단순화 0

Visual Appeal이 부족하면 primitive로 무조건 후퇴하지 않는다. 실제 Asset로 만들 수 있는 decorative technique의 Production Proven 범위를 조금씩 넓힌다.

## 8. 이전 Trial에서 보존할 교훈

- Trial 01: 생성형 이미지 결과는 Deterministic Geometry evidence가 아니다.
- Trial 02: 명시적 좌표/Vector source로 부분 수정 가능한 geometry를 만들 수 있다. 미세 좌표 정밀도를 전체 목표로 확대하지 않는다.
- Trial 03: Authoring Layer / partial edit / Runtime Packing은 실용적이지만 풍부한 Concept을 뒤늦게 Source로 만들면 Visual Identity Gap이 커질 수 있다.
- Trial 04: Production Promise를 먼저 정의하면 Reality Gap을 낮출 수 있지만 Proven primitive만으로는 시각적 매력이 부족할 수 있다. Pre-Generation Gate와 Post-Generation Compliance를 분리한다.

## 9. 공용 Gate 연계

- Document/SSOT/Shared/Production_Realizability_Gate.md
- Document/SSOT/Shared/UI_Resource_Gate.md
- Document/SSOT/Shared/P-AI-001.md
- Document/SSOT/Shared/P-UI-001.md

새 교훈은 실제 결과가 나온 뒤에만 Gate/Pattern 보강 여부를 판정한다.

## 10. 보호 범위

Trial 05 USER PASS 전:
- UE Import 0
- Production Widget mutation 0
- Production Texture 교체 0
- VehiclePanel/RadarPanel 실제 Asset 변경 0
- 기존 CF-FQ-039 Frame Master v5 USER Pending 상태 변경 0

이 Trial은 Production Visual Rework의 제작 방법론 검증이며 기존 VPR-P0-01 승인 상태를 자동 변경하지 않는다.

## 11. 현재 재개 지점

> Trial 05는 **방법론 검증 완료**로 닫았다. USER는 Polish 결과를 실제 제작 방식 검증에 충분한 Baseline Candidate로 받아들였고, `Source를 나누는 것 ≠ Runtime Widget/Image를 나누는 것` 원칙까지 확인했다. WeaponPanel Production 확장은 USER 지시에 따라 취소했고 관련 C++ 확장은 원상복구했다. Trial Runtime Pack 2종은 evidence-only로 보존하며 UE Import/WeaponPanel Asset mutation은 0이다. 실제 적용 대상은 현재 Active `CF-FQ-039 VehiclePanel`, owner는 `VT07_AssetFirstPlan.md`다.

## 12. Changelog

### v0.8 - 2026-08-27

- USER가 URT05를 WeaponPanel Production으로 확장하지 말고 현재 Active VehiclePanel에 적용하도록 방향을 교정했다.
- WeaponPanel용 HUDVisualData/Presenter/Test C++ 확장을 전부 원상복구하고 기존 Vehicle/Defense 병렬 변경은 보존했다.
- `URT05_ProdHandoff.json v0.3.0`을 CANCELLED_NOT_APPLIED Historical evidence로 전환했다.
- URT05 Source/Derivative/Runtime Pack은 방법론 evidence로 보존하며 UE Import/Production Asset mutation 0을 확정했다.
- Asset-First 방법은 `SourceArt/UI/HUD/VT/VT07_AssetFirstPlan.md`의 VehiclePanel 작업으로 이전했다.

### v0.7 - 2026-08-27

- `PackURT05Prod.ps1 v1.0.0`을 추가하고 Authoring 5종을 Runtime Frame 1 + Accent 1 PNG로 실제 패킹했다.
- `URT05_WeaponResFrame.png`와 `URT05_WeaponResAccent.png` 모두 720×68 Project Image readback을 확보했다.
- Runtime Texture 수는 2개로 고정하고 Authoring Source 5종 독립 편집성과 Runtime Packing 최소화를 분리했다.
- Production Handoff owner를 `URT05_ProdHandoff.json v0.2.0`으로 전진하고 다음 Gate를 UE Import + additive WeaponPanel implementation으로 이동했다.
- UE Import/Production Asset mutation은 아직 0이다.

### v0.6 - 2026-08-27

- Visual Richness Polish 결과를 USER가 `이 정도면 괜찮다`고 평가해 `Visual Baseline Candidate Accepted`로 HOLD를 해제했다.
- 이 승인을 final Production Visual PASS와 분리하고 실제 게임 적용 후 USER Review를 최종 Gate로 유지했다.
- `URT05_ProdHandoff.json v0.1.0`을 추가해 Authoring 5종 → Runtime 2종 Packing과 C++/UMG 책임을 확정했다.
- FrameBase+InnerRail+CornerDetail은 static Frame 1장, AccentStrip+SubtleGlowStrip은 semantic tint 가능한 Accent 1장으로 패킹하며 5개 Runtime Image 구조를 금지했다.
- 기존 WeaponCharge Runtime/ViewData/ResourcePresentation/ProgressBar/FireState semantic을 재사용하고 별도 Gameplay owner를 만들지 않는다.
- 현재 UE Import/Production mutation은 0이며 다음 Gate는 packed Asset 제작과 additive Production implementation이다.

### v0.5 - 2026-08-27

- USER가 Asset-only Composite를 `나쁘지 않지만 막 좋다고 하기도 어려운 SO-SO`로 평가해 USER HOLD로 기록했다.
- 해당 피드백을 Production Visual PASS로 확대하지 않았다.
- 다음 Gate를 새 독립 레이어 추가가 아니라 기존 Source 내부 Visual Richness Polish 1회로 한정했다.
- 우선 개선 대상을 FrameBase 내부 명암/재질 계층, AccentStrip 국소 강조 차이, Insufficient semantic warning contrast로 고정했다.
- 1회 Polish 뒤에도 SO-SO면 `Realizable but Visual Appeal Limited`로 Trial을 종료하고 Production 적용을 강행하지 않는 Exit Rule을 추가했다.
- SurfaceNoise Deferred, CF-FQ-039 Frame Master v5 USER Pending, Production/UE mutation 0을 유지했다.

### v0.4 - 2026-08-27

- GoPyMCP generic script Current contract와 configured repository `main_game`을 확인해 Project bounded derivative 실행 경로를 복구했다.
- `RunURT05Sources.ps1 v1.0.1`로 Source JSON 5종 → SVG 5종 generation/validation PASS를 확보했다.
- Windows PowerShell 5.1 UTF-8-no-BOM의 generated desc corruption을 발견·교정하고 fresh readback PASS했다.
- `RecommendedBalance` Color Hierarchy를 임시 기준으로 고정하고 USER가 기존 grayscale보다 개선된 방향으로 평가한 상태를 기록했다.
- `URT05_CompositeSpec.json v0.1.1`에 실제 5 Source + Production Proven Runtime primitive만 사용한 Normal / Insufficient Composite를 정의했다.
- InsufficientText의 lower accent overlap을 조정했으며 현재 Gate를 USER Visual Review로 이동했다.
- SurfaceNoise Deferred, CF-FQ-039 Frame Master v5 USER Pending, Production/UE mutation 0을 유지했다.

### v0.3 - 2026-08-27

- exact Structural Source 3종을 720×68 raster로 검증해 TECH PASS / Visual Identity Insufficient로 판정했다.
- ROI 기준으로 `CornerDetail`과 `SubtleGlowStrip`만 실제 독립 Source로 생성하고 individual raster QA PASS했다.
- `SurfaceNoise`는 68px readability 대비 낮은 ROI로 Deferred했다.
- `MakeURT05Sources.py`를 v1.2.0으로 전진해 Structural 3 + Decorative 2 총 5종 derivative 대상으로 확장했다.
- Project `process.run` generic-script는 connector argument schema 단계에서 거부되어 Project bounded derivative generation은 Pending으로 남겼다.
- local exact-source derivative/raster QA를 Project bounded execution PASS로 확대하지 않았고, Concept Composite는 계속 생성하지 않았다.
- Production/UE mutation 0과 CF-FQ-039 Frame Master v5 USER Pending을 보존했다.

### v0.2 - 2026-08-27

- Weapon Charge Module Asset Inventory를 최종 Lock했다.
- Common Canvas를 720×68 / Top-Left / Transparent / White-Alpha tintable로 확정했다.
- `FrameBase / InnerRail / AccentStrip`을 각각 독립 Explicit Path JSON Source로 생성했다.
- direct `.svg` write 차단에 대응해 SVG/PNG를 Source가 아닌 deterministic derivative로 재분류했다.
- `URT05_StructSpec.json v0.2.0`과 `MakeURT05Sources.py v1.1.0`으로 단일 geometry authority를 고정했다.
- Concept Composite는 생성하지 않았고 다음 Gate를 Structural derivative Visual QA로 이동했다.
- Production/UE mutation 0과 CF-FQ-039 Frame Master v5 USER Pending 상태를 보존했다.

### v0.1 - 2026-08-27

- Trial 01~04의 결과를 바탕으로 Asset-First UI Concept Trial을 신규 정의했다.
- Concept-first 역추출 대신 실제 독립 Asset → Concept Composite 순서로 전환했다.
- Weapon Charge Module을 첫 대상, FrameBase/InnerRail/AccentStrip을 첫 Structural Source 후보로 고정했다.
- USER PASS 전 Production/UE mutation 0과 기존 CF-FQ-039 Frame Master 상태 보존을 명시했다.