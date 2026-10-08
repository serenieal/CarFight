# Vehicle Builder 사용자 정보 UX Plan

- Version: 0.2.0
- Date: 2026-09-15
- Status: Done / Historical + Retained Path / VBIUX-P0-05B~05E Technical PASS / P0-05F USER Acceptance PASS / P0-06 Current System Promotion + Final Closure Audit PASS
- Feature: `CF-FQ-046 Vehicle Builder 사용자 정보 UX`
- Priority: P2
- Current Active Feature: `CF-FQ-039 Production UI Visual Rework` 유지
- Representative Plan: `Document/Plan/VehicleBuilderInfoUX/VehicleBuilderInfoUXPlan.md`
- Current System Owner: `Document/Systems/Vehicles/VehicleBuilder.md v1.7.0`
- Presentation Implementation Owner: `CarFight_ReEditor / SCFVehicleBuilderTab + bounded Editor-only Presentation Helper`
- Typed State / Backend Authority: 기존 `FCFVehicleBuilderVM`, Recipe, Authoring/Apply/Driving/Catalog 계약 유지

---

## 1. 목적

Guided Vehicle Builder 1~8단계에서 사용자가 개발 내부 용어를 해석하지 않고도 다음 세 가지 질문에 바로 답할 수 있도록 사용자 정보 구조를 정리한다.

1. **이 단계는 무엇을 확인하는 단계인가?**
2. **현재 차량은 어떤 상태인가?**
3. **지금 내가 무엇을 하면 되는가?**

숫자나 기술 설정을 보여줄 때는 값만 노출하지 않고, 사용자가 판단해야 하는 항목에 대해 **값 + 뜻 + 게임에서의 영향**을 한 묶음으로 제공한다.

예:

```text
상향 변속: 5,500 RPM
엔진이 약 5,500 RPM에 도달하면 다음 기어로 변속합니다.

최종 감속비: 3.20
바퀴에 전달되는 힘과 최고속도 성향에 영향을 주는 값입니다.
```

이 Feature는 Vehicle Builder의 계산·저장·Apply 구조를 다시 만드는 작업이 아니다. 기존 기능은 유지하고 **Presentation / User-Facing Information Architecture**만 소유한다.

---

## 2. 발생 배경

2026-09-03 Wagon Guided Builder USER 확인 중 Step 5에서 다음 정보가 기본 상세 화면에 노출됐다.

```text
Persistent Builder receipt
Consumed Claim
EvidenceFingerprint
Engine Curve Hash
private 4 Profile
CLAIM-V60-*
PhysicsDraft.json path
```

이 정보는 기술 진단에는 유효하지만 차량 제작자가 실제 설정을 판단하는 데 필요한 정보가 아니다.

같은 점검에서 Step 6은 전체 내용이 화면 높이를 초과해도 ScrollBox가 없어 하단 정보를 볼 수 없는 레이아웃 문제도 확인됐다. Step 6 스크롤 구조는 선행 UX 교정에서 `FillHeight + SScrollBox`로 수정됐고 official Editor build가 PASS했다.

이후 Step 1~8 Source를 빠르게 검토한 결과 Step 5만의 문제가 아니라 다음 유형이 여러 단계에 반복됨을 확인했다.

- 내부 authority/diagnostic 단어가 사용자 문장에 직접 노출됨
- 수치는 있으나 그 수치가 게임에서 무엇을 의미하는지 설명이 없음
- 완료/Blocked 상태 설명이 구현 계약 중심으로 작성됨
- 실제 사용자가 해야 할 행동과 시스템 내부 상태가 같은 문단에 혼합됨
- 상세 정보와 개발 진단 정보의 레벨이 분리되지 않음

따라서 개별 문자열 수정이 아니라 1~8단계 전체의 정보 계층을 정식 계약으로 관리한다.

---

## 3. 현재 구현 Seed와 보호 경계

### 3.1 이미 존재하는 UX 교정 Seed

정식 Plan 승격 직전 `CFVehicleBuilderTab.cpp/.h`에 다음 USER 피드백 대응이 미커밋 상태로 존재한다.

- Step 표시명 일부 한국어 사용자 명칭으로 변환
- 상단 Provider 표기 제거
- Step 6 전체 `SScrollBox` 전환
- Mount Type/Size/Preset 사용자 명칭 일부 교정
- Step 5 raw 진단 덤프 대신 엔진/변속기/구동/제동/질량 중심 표시를 위한 v1.28.0 초안

이 중 마지막 Step 5 v1.28.0 변경은 **아직 최종 Build/UAT 계약으로 승인된 상태가 아니다.**
VBIUX-P0-00/P0-01에서 전체 정보 원칙을 확정한 뒤 정리·보강한다.

### 3.2 반드시 보호할 병렬 변경

현재 main_game에는 다음 병렬 dirty가 존재한다.

- CF-FQ-041 Runtime Apply
- CF-FQ-044 Runtime Catalog Promotion
- CF-FQ-045 Data Asset Management
- CF-FQ-039 UI Visual SourceArt
- Wagon/Catalog/Weapon Asset의 사용자 또는 병렬 작업 변경
- `CFVehicleBuilderTab.cpp/.h`의 CF-FQ-044 Step 8 Catalog orchestration 변경

VBIUX 구현 전에는 fresh Git diff를 다시 확인하고, **표시 문구/Slate presentation과 무관한 hunk를 수정·정리·되돌리지 않는다.**

### 3.3 기능 계약 불변

이번 Feature에서 변경하지 않는다.

- Builder Step state authority
- Reference Evidence의 실제 검증/결정 로직
- Physics Proposal 생성/계산/commit
- Recipe schema와 semantic owner
- VehicleData Resolve/Final Review/Apply
- Undo/Redo
- Driving benchmark 계산
- USER Driving persistent receipt
- Runtime Catalog Promotion
- Runtime Apply
- auto-save 정책

기존 결과가 사용자에게 어떻게 설명되는지만 바꾼다.

### 3.4 USER 문구 Owner와 typed truth 경계

현재 구현은 `FCFVehicleBuilderVM::Evaluate*Step()`가 state 판정과 함께 개발자용 `Summary/Resolution` 문장을 직접 생성하고 `SCFVehicleBuilderTab`이 이를 그대로 출력하는 부분이 있다.

VBIUX에서는 이 구조를 다음처럼 분리한다.

```text
FCFVehicleBuilderVM
= Step state / count / typed result / backend diagnostic truth owner

Editor-only Presentation
= typed truth를 USER Label / 값 / 의미 / 행동 문장으로 변환

SCFVehicleBuilderTab
= layout / interaction / presentation projection 표시 owner

Raw VM Summary/Resolution / backend Message
= 기술 진단 보존용. USER 기본 문구 authority가 아님
```

VM의 기존 판정 로직과 raw diagnostic 문자열은 삭제하지 않는다.
Presentation에 필요한 typed getter/result가 이미 있으면 그대로 읽고, 부족한 경우에도 **read-only projection/DTO getter만 최소 추가**할 수 있다. 이 Feature를 이유로 Step state 판정, Resolver, Recipe mutation, Apply 의미를 VM에서 재설계하지 않는다.

USER 화면은 가능한 한 raw `Step.Summary`, `Step.Resolution`, `Operation.Message`를 직접 출력하지 않는다. 해당 문자열만 사용할 수 있는 예외 상태는 P0-00 inventory에서 명시하고 P0-01에서 USER recovery message + diagnostic detail로 분리한다.

---

## 4. 정보 계층 계약

### 4.1 Level 0 — 기본 화면

기본 화면에는 다음만 노출한다.

```text
단계 이름
이 단계의 목적
현재 상태
지금 할 일
핵심 경고 또는 완료 메시지
```

사용자는 이 영역만 읽고도 다음 행동을 결정할 수 있어야 한다.

### 4.2 Level 1 — 사용자 상세

사용자가 차량을 판단하기 위해 필요한 실제 데이터만 보여준다.

예:

- 선택된 Chassis / Wheel Mesh
- Wheel Socket 존재 여부와 위치 의미
- Wheelbase / Track
- 엔진 토크 / RPM
- 변속 시점 / 기어 수 / 감속비
- 구동 방식 / 브레이크
- Mount 종류 / 허용 장비 크기
- Final Review의 Before → After
- Driving Test 체크리스트

수치는 가능한 경우 다음 구조로 표시한다.

```text
사용자 이름
현재/제안 값
짧은 의미 설명
필요할 때 게임 내 영향 설명
```

### 4.3 Level 2 — 기술 진단

다음 정보는 기본 화면과 일반 상세에서 제거한다.

- Fingerprint / Hash
- EvidenceId / RecipeId / RunId
- Claim ID
- Proposal correlation
- R0/R1/R2/R3
- mutation0
- provenance
- exact binding
- DefinitionHash
- raw object/package path
- internal enum/state 이름
- implementation-only cache/readback 용어

기술 진단이 실제 운영상 필요하면 별도의 명확한 **진단 정보** 영역에서만 노출한다.
USER UAT에서 해당 정보가 없어도 정상 차량 제작이 가능해야 한다.

### 4.4 기술 용어를 완전히 금지하지 않는다

사용자가 실제 작업에 필요한 Unreal/차량 용어는 유지한다.

예:

- Mesh
- Socket
- Wheelbase
- Track
- RPM
- Torque
- Gear
- Hardpoint
- Mount
- VehicleData

단, 처음 등장하거나 의미가 모호한 경우 사람이 이해할 설명을 붙인다.

```text
Wheelbase 2,950 mm
앞바퀴 축과 뒷바퀴 축 사이 거리입니다. 차량의 크기와 회전 성향 판단에 사용합니다.
```

### 4.5 값 Source Authority 표시 계약

같은 물리 수치라도 source 단계가 다르면 같은 값으로 취급하지 않는다.

Step 5의 최소 구분:

```text
AI Draft
= AI가 제안했지만 아직 승인되지 않은 값

Builder-private Profile
= USER가 AI 제안을 검토·승인해 제작 프로필에 반영된 값

VehicleData
= 현재 Runtime Target에 저장된 값. Step 7 Apply 전후의 최종 대상
```

표시 예:

```text
AI 제안
상향 변속: 5,500 RPM
아직 차량 제작 프로필에는 반영되지 않았습니다.

현재 제작 프로필
상향 변속: 5,500 RPM
검토 승인된 제작용 설정입니다.

Step 7 최종 검토
현재 VehicleData 4,500 → 적용 예정 5,500 RPM
```

USER가 Draft와 이미 적용된 값을 혼동할 수 있는 표현을 금지한다.

### 4.6 단위 / 값 Format 계약

Persistent/Resolver canonical value는 변경하지 않고 **Presentation에서만 USER 단위와 형식으로 변환**한다.

기본 단위:

```text
Engine / Brake Torque = Nm
RPM = RPM
Vehicle Mass = kg
Runtime Speed = km/h
Time = s
UE Socket/Local Transform = cm / deg
실차 제원 길이(Wheelbase/Track 등) = mm 우선
Ratio = 단위 없음, 필요한 소수 자릿수만 표시
Bool / Enum = USER 문장으로 변환
Array / Struct = raw serialized text가 아니라 의미 단위로 구조화
```

동일한 값이라도 사용 맥락이 UE 편집 좌표인지 실차 제원인지 구분한다. 저장값 단위를 임의 변경하지 않는다.

### 4.7 의미와 영향 설명의 정확도 계약

설명은 다음 두 종류를 구분한다.

```text
확정 의미
= 설정 자체가 직접 정의하는 동작
예: 상향 변속 RPM은 해당 RPM 부근에서 다음 기어로 변속하는 기준이다.

일반적 성향
= 다른 설정과 상호작용하는 경향
예: 최종 감속비가 높을수록 일반적으로 구동력이 커지고 최고속 성향은 낮아질 수 있다.
```

다른 필드의 영향을 받는 결과를 단정적인 인과로 표현하지 않는다. 설명 근거가 없는 필드는 추측하지 않고 generic fallback을 사용한다.

### 4.8 오류 / 복구 문구 계약

정상 화면뿐 아니라 실패 Dialog/Status에서도 raw backend diagnostic을 USER가 직접 해석하게 하지 않는다.

오류 표시는 가능하면 다음 3층으로 구성한다.

```text
USER Error Summary
= 무엇이 잘못됐는지 사람이 이해할 문장

USER Recovery Action
= 지금 무엇을 해야 하는지

Diagnostic Detail
= StateChanged / Hash / Path / backend message 등 기술 진단. 접힌 진단 영역 또는 로그용
```

예:

```text
차량 상태가 마지막 확인 이후 변경되었습니다.
'현재 상태 다시 확인'을 누른 뒤 다시 시도하세요.

[진단 정보]
StateChanged / expected ... / actual ...
```

### 4.9 공통 Page Height / Scroll / Overflow 계약

어떤 Step에서도 긴 정보 때문에 `이전 / 다음 / 현재 상태 다시 확인` 같은 핵심 navigation/action에 접근할 수 없어서는 안 된다.

이 계약은 Step 5 물리 설정 상세와 Step 8 진단 정보에만 적용하는 예외 처리가 아니다. **Vehicle Builder Step 1~8 전체의 페이지 본문, 사용자 상세, 진단 정보, Expander를 같은 높이/스크롤 정책으로 관리한다.** Step 5와 Step 8은 현재 재현 사례일 뿐이다.

공통 페이지 구조의 목표는 다음과 같다.

```text
[고정 상단]
단계 이름 / 목적 / 핵심 상태

[가변 콘텐츠 영역]
Step 본문
사용자 상세
Expander로 펼친 추가 정보
진단 정보
→ 이 영역만 남은 높이를 사용하고 세로 스크롤한다.

[고정 하단]
현재 상태 다시 확인 / 이전 / 다음 / Step별 핵심 작업 버튼
→ 긴 콘텐츠가 펼쳐져도 화면 밖으로 밀려나지 않는다.
```

공통 규칙:

- Step 1~8은 root page 자체가 콘텐츠 높이에 따라 무한히 커지는 `AutoHeight` 중심 구조가 되지 않도록 감사한다.
- 각 Step은 가능한 한 **하나의 주 세로 Scroll owner**만 가진다. 기본안은 `FillHeight` 콘텐츠 영역 내부의 단일 `SScrollBox`다.
- 사용자 상세와 `진단 정보` Expander는 펼쳤을 때 페이지 전체 높이를 밀어내지 않고 같은 콘텐츠 Scroll 영역의 길이에 포함된다.
- Expander 내부에 별도의 세로 ScrollBox를 중첩하는 것은 기본 금지한다. 실제로 독립 viewport가 필요한 예외만 허용하고 wheel input trap과 포커스 이동을 별도 검증한다.
- 하단 navigation/action 영역은 주 콘텐츠 Scroll 밖에 둔다. 콘텐츠가 아무리 길어져도 접근 가능해야 한다.
- 상단 핵심 상태 역시 장문의 상세 정보에 밀려 사라지지 않도록 고정 또는 bounded 영역으로 유지한다.
- raw diagnostic의 긴 줄은 USER page 폭을 무한 확장하지 않도록 wrap/copy 가능한 표시 방식을 사용한다. 가로 overflow가 필요하면 세로 Scroll owner와 분리해 명시적으로 처리한다.
- Step별로 임의의 고정 픽셀 높이를 따로 하드코딩하지 않는다. 공통 page shell이 사용 가능한 viewport 높이를 받아 콘텐츠 영역을 배분하는 구조를 우선한다.
- Step 1~8 전체에 대해 `Collapsed / Detail Expanded / Diagnostic Expanded / 최대 동시 Expanded` 상태를 감사한다.
- USER UAT는 실제 화면 높이에서 각 Step의 최하단 콘텐츠까지 스크롤 가능하고, 동시에 하단 작업 UI가 항상 접근 가능한지 확인한다.

---

## 5. Step별 목표

### Step 1 — 차량 / 기준 자료

현재 위험 용어:

- Reference Evidence
- Companion
- Research Draft
- fingerprint
- exact binding

목표 화면:

- 어떤 실존 차량/자료를 기준으로 삼는지
- 기준 자료 준비 완료 여부
- 새 자료를 불러와야 하는지
- 기존 기준을 계속 사용할지

내부 Evidence identity/hash는 진단 정보로 격리한다.

### Step 2 — 차량 Mesh 준비

현재 위험 용어:

- resolve
- AssetSnapshot
- canonical
- blocker
- fallback contract

목표 화면:

- 어떤 Chassis Mesh가 지정됐는지
- 앞/뒤 Wheel Mesh가 무엇인지
- 필수 누락 항목
- FR/RL/RR 미지정 시 FL 재사용의 실제 의미
- Mesh 선택이 차량에 어떤 역할을 하는지

### Step 3 — 소켓 준비 / 이름 설정

Socket 이름은 실제 USER 작업 대상이므로 exact 이름을 유지한다.

목표 화면:

- Wheel Socket 4개가 각각 어느 바퀴인지
- Socket이 존재하는지
- 위치/회전/Scale을 왜 직접 맞춰야 하는지
- Hardpoint가 실제 장비 부착 위치라는 설명
- Wheelbase/Track 값과 의미

`distinct binding`, `Socket-derived`, `canonical` 같은 구현 용어는 기본 UI에서 제거한다.

### Step 4 — 차량 배치 확인

목표 화면:

- 차량 앞/뒤 방향
- Wheelbase
- Front/Rear Track
- Wheel 배치 이상 여부
- 해당 수치가 차량 크기/조향/시각 정렬에 무엇을 의미하는지

값만 나열하지 않는다.

### Step 5 — AI 물리 설정

사용자 상세를 다음 영역으로 구성한다.

```text
[엔진]
[변속기]
[구동]
[조향 / 제동]
[서스펜션]
[차량 기본 / 질량]
```

각 표시값은 반드시 현재 source authority를 함께 구분한다.

```text
승인 전 = AI가 제안한 값
승인 후 = 현재 제작 프로필 값
VehicleData 값 = Step 7 최종 검토에서 별도 표시
```

즉 AI Draft를 읽은 직후 화면에서 `적용할 설정`처럼 확정적으로 표현하지 않고, 승인 후에만 `현재 제작 프로필`로 표시한다. Builder-private Profile과 Target VehicleData가 같은 값이라고 가정하지 않는다.

주요 수치마다 의미를 설명한다.

예:

- 최대 토크 → 엔진이 만들어낼 수 있는 회전력이며 가속 성향에 영향을 준다.
- 최대 RPM → 엔진이 사용할 수 있는 RPM 범위다.
- 상향/하향 변속 RPM → 자동 변속 시점을 직접 정의한다.
- 최종 감속비 → 가속/최고속 성향에 영향을 주는 값이며 다른 기어비·엔진·타이어 설정과 함께 작동한다.
- 브레이크 토크 → 제동력 성향에 영향을 준다.
- 질량 → 가속/제동/충돌/서스펜션 반응에 함께 영향을 준다.

현재 v1.28.0 Seed는 P0-01 계약 확정 후 이 source authority/format 규칙으로 정리한다.

### Step 6 — 게임플레이 설정

목표 화면:

- 장비를 달 위치 수
- 각 위치에 허용되는 장착 방식
- 허용 장비 크기
- 기본 장비 프리셋
- 아직 규칙이 없는 위치
- 사용자가 Socket 위치를 확인해야 하는 경우

`MountIntent`, `Recipe write`, `pending gameplay diff`는 사용자 화면에서 직접 노출하지 않는다.

### Step 7 — 최종 검토

가장 중요한 사용자 정보 Gate다.

각 변경은 다음 형태를 우선한다.

```text
상향 변속 RPM
현재 4,500 → 변경 후 5,500 RPM
이전보다 높은 RPM까지 엔진을 사용한 뒤 다음 기어로 변속합니다.
```

필드 경로 `VehicleMovementConfig.ChangeUpRPM`는 기본 목록의 주표시가 아니다.

필요한 typed presentation 구조:

```text
Operation                 // Set / Add / Remove / Move
UserLabel
BeforeValue
AfterValue
Meaning
EffectSummary
TechnicalFieldPath        // diagnostic only
```

`FCFVehicleFieldDiff::Operation`을 보존해 구조 변경을 단순 Before/After 문자열로 뭉개지 않는다.

표시 예:

```text
[값 변경]
상향 변속 RPM
4,500 → 5,500 RPM
이전보다 높은 RPM까지 사용한 뒤 다음 기어로 변속합니다.

[추가]
장비 장착 위치 추가
Roof_Main 장착 위치가 추가됩니다.

[삭제]
장비 장착 규칙 삭제
Front_Left 위치의 장착 규칙이 제거됩니다.
```

모든 VehicleData field에 거대한 설명 사전을 만들지 않는다. 현재 Guided Builder가 실제 노출하는 주요 변경 필드부터 bounded Editor-only mapping을 만든다.

기존 `FCFVehicleFieldRegistry`는 Resolver/Authoring 기술 SSOT이므로 USER label/unit/meaning/effect metadata를 직접 섞지 않는다. Presentation mapping은 `StableFieldPath`의 canonical pattern을 key로 참조하되 별도 Editor-only owner를 유지한다. Stable-ID array field는 숫자 index에 의존하지 않고 기존 wildcard/stable identity 의미와 호환되는 pattern match를 사용한다.

미등록 필드는 숨기거나 추측해서 설명하지 않는다.

```text
기타 차량 설정 변경
현재값: ...
변경값: ...
이 항목에 대한 사용자 설명은 아직 등록되지 않았습니다.
[진단 정보] 실제 FieldPath 확인 가능
```

Add/Remove/Move의 raw struct/array canonical text도 USER 기본 화면에 그대로 덤프하지 않는다.

### Step 8 — 주행 테스트

기본 화면의 주인공은 benchmark/hash가 아니라 실제 운전 확인이다.

최소 체크리스트:

- 출발 가속이 지나치게 느리거나 급하지 않은가
- 자동 변속 시점이 자연스러운가
- 변속 직후 RPM이 비정상적으로 떨어지지 않는가
- 브레이크가 지나치게 약하거나 강하지 않은가
- 조향이 차종에 어울리는가
- 바퀴/차체가 흔들리거나 튀는 물리 이상이 없는가

이 체크리스트는 **USER 판단을 돕는 non-persistent 안내**다. 각 항목별 체크 상태를 Recipe/VehicleData에 새로 저장하지 않고 Step 8 completion authority를 변경하지 않는다.

```text
주행 체크리스트
= 안내용 / non-persistent

USER 주행 PASS
= 기존 Target VehicleData path + DefinitionHash persistent receipt authority 유지
```

Technical Benchmark는 필요한 경우 사람이 이해할 상태 요약으로 제공한다.

Runtime Catalog 상태도 내부 명칭보다 다음처럼 사용자 의미를 우선한다.

```text
데모 차량 목록 등록됨
데모 차량 목록 미등록 — 주행 승인 뒤 등록할 수 있음
저장되지 않은 변경 있음
```

`DefinitionHash`, `RunId`, receipt identity는 진단 정보로 격리한다.

---

## 6. Presentation Architecture 방향

기본 구현은 기존 Slate C++ 구조를 유지한다.

Blueprint를 새로 도입하지 않는다.

이유:

- Guided Builder가 현재 Editor-only Slate C++로 구현됨
- 표시 state가 `FCFVehicleBuilderVM` 결과와 직접 연결됨
- Blueprint Widget으로 분리하면 기존 Editor Shell과 이중 UI authority가 생김

권장 역할 분리:

```text
FCFVehicleBuilderVM
= 기존 사실/상태/typed result/backend diagnostic owner 유지

SCFVehicleBuilderTab
= Step layout / interaction / presentation projection host

Editor-only Presentation Helper
= typed truth → USER label/value/meaning/effect/recovery action 변환
```

현재 `Evaluate*Step()`가 만드는 raw `Summary/Resolution`은 diagnostic compatibility로 보존할 수 있지만 정상 USER 문구의 SSOT로 사용하지 않는다.

Presentation에 필요한 사실이 기존 public getter/result에 없을 때는 먼저 read-only DTO projection으로 노출할 수 있는지 검토한다. UI를 쉽게 만들기 위해 backend semantic state를 새로 계산하거나 duplicated authority를 만들지 않는다.

P0-01에서 helper를 실제 별도 파일로 만들지, Tab 내부 bounded helper로 충분한지 먼저 판단한다. 단순 문자열 교체를 위해 불필요한 framework를 만들지 않는다.

Step 7의 반복 field presentation은 별도 Editor-only typed descriptor/helper가 적합한 경우 이를 사용하되 `FCFVehicleFieldRegistry`와 소유권을 분리한다.

---

## 7. VBIUX-P0-00 Full User-Facing Information Audit 결과

### 7.1 감사 범위와 판정

2026-09-03 current Source 기준으로 다음 USER 노출 경로를 read-only 전수 감사했다.

~~~text
SCFVehicleBuilderTab
- 상단 Step frame
- Step 1~8 detail panel
- Button label / Tooltip
- Confirmation dialog
- LastStatusText
- Runtime Catalog status

FCFVehicleBuilderVM
- EvaluateIdentityReferenceStep
- EvaluateMeshPrepStep
- EvaluateSocketGuideStep
- EvaluateLayoutCaptureStep
- EvaluatePhysicsProposalStep
- EvaluateGameplaySetupStep
- EvaluateFinalReviewStep
- EvaluateDrivingTestStep
- BuildReferenceSummary
- BuildGameplayGuidanceSummary
- BuildFinalReviewSummary
- BuildDrivingTestSummary
~~~

감사 판정:

- **VBIUX-P0-00 PASS**
- 1~8 Step 모두 주요 USER surface와 source owner를 식별했다.
- 정상 화면뿐 아니라 Tooltip / Confirmation / LastStatus / backend failure 직통 경로까지 inventory에 포함했다.
- Step 5/6의 선행 UX Seed는 유지하되 P0-01 계약 확정 전 추가 구현하지 않는다.
- backend semantic 변경, Asset mutation, Editor restart, Build/Automation은 이번 Audit에서 수행하지 않았다.
- exact next Gate는 **VBIUX-P0-01 Presentation Contract Design Review**다.

### 7.2 공통 Surface Inventory

| Surface | Source Owner / Authority | Current 문제 | 목표 Level / P0-01 처리 |
| --- | --- | --- | --- |
| 상단 Step 목적 설명 | SCFVehicleBuilderTab::GetCurrentStepSummaryText + FCFVehicleBuilderStepView::Summary | Step 5/6만 USER 문구이고 Step 1~4/7~8은 raw VM Summary를 그대로 표시 | Level 0. 1~8 모두 StepId별 USER purpose/status projection 사용 |
| 상단 지금 할 일 | GetCurrentStepResolutionText + raw Step.Resolution | Step 5/6만 USER action 문구이고 나머지는 backend contract 문장 노출 | Level 0. 상태별 USER next action으로 변환 |
| Step state 문자열 | Step.State | 상태 자체는 필요하지만 raw Summary에도 [Ready]류 suffix가 중복 삽입됨 | Level 0. 상태 badge/text는 유지, raw summary prefix는 USER 표시에서 소비하지 않음 |
| Builder subtitle | SCFVehicleBuilderTab 고정 LOCTEXT | Reference / Physics Proposal / Gameplay / Final Review / Driving 혼용 | Level 0. 제작 흐름을 한국어 중심으로 설명 |
| LastStatusText | Tab handler + backend Error/Result.Message/Operation.Message | 성공/실패 뒤 raw backend message, path, hash, typed lane 용어가 직접 노출됨 | Level 0 USER 결과 + 복구 행동. raw message는 Level 2 diagnostic으로 분리 |
| Tooltip | Tab 고정 LOCTEXT | mutation0, R0~R3, exact, Recipe-only, ProposalHash, DefinitionHash가 다수 노출 | Level 0/1. 무엇을 하며 무엇을 바꾸지 않는지만 기본 tooltip에 남김 |
| Confirmation Dialog | Step 1/5/7/8 handler | USER 승인보다 hash/path/transaction contract가 더 큰 비중 | Level 1. 실제 변경/비변경 범위와 USER 판단을 먼저 표시, Level 2 진단은 분리 |
| 긴 Step layout | Slate slot/ScrollBox | Step 6/7은 scroll 보강됨. Step 8은 긴 summary+status+buttons인데 root AutoHeight, Step 5도 상세 확장 시 길어질 수 있음 | P0-01에서 Step 5/8 bounded scroll과 action 고정 여부 확정 |

### 7.3 Step별 USER Surface Inventory

| Step | Source Owner / Source Authority | Current USER surface / leak | 목표 Level / 목표 표현 |
| --- | --- | --- | --- |
| **1. 차량 / 기준 자료** | BuildReferenceSummary, EvaluateIdentityReferenceStep, Reference/Companion handlers / Evidence + Research Draft + Recipe | Reference Evidence / Companion, Draft 절대경로, Canonical Claim/Conflict/Unknown, EvidenceFingerprint, private Profile, exact binding, review token, R1/R2, ResolvedDefinitionHash가 본문·Tooltip·Dialog에 노출 | Level 0: 어떤 실차를 기준으로 하는지/준비 완료 여부/다음 행동. Level 1: 제조사·모델·연식·트림·파워트레인·변속기, 자료/미해결 항목 수. Level 2: Evidence path/id/fingerprint, Claim ID, hash, receipt/token/R1/R2 |
| **2. 차량 Mesh 준비** | EvaluateMeshPrepStep, Mesh picker/commit / Recipe AssetIntent + AssetSnapshot | picker와 FL fallback은 유용하나 상단 raw Summary에 Resolve/AssetSnapshot/canonical/bounds/non-finite/runtime fallback 노출. Tooltip/상태는 typed AssetIntent Recipe-only 사용 | Level 0: 차체/필수 휠 준비 여부와 누락 항목. Level 1: 선택 Mesh와 FL 재사용 의미, 필요한 규격을 사람이 읽는 문장. Level 2: AssetSnapshot/resolve/canonical/bounds diagnostic |
| **3. 소켓 준비 / 이름 설정** | EvaluateSocketGuideStep, Hardpoint panel / Chassis Socket truth + Recipe Hardpoint intent | exact Socket 이름·공유 StaticMesh 경고·축 안내는 USER 필수. 반면 distinct binding, Socket-derived, stable ID, migration, LocationSlotId, max-used+1, Recipe Hardpoint/Mount가 섞임 | Level 0: Wheel Socket 4개 존재/누락, 장착 위치 계획과 다음 행동. Level 1: 실제 Socket 이름, shared Mesh 경고, Wheelbase/Track, 축, Hardpoint 위치. Level 2: distinct/stable identity/migration/backend count |
| **4. 차량 배치 확인** | EvaluateLayoutCaptureStep / current Socket facts + Target VehicleLayoutConfig | **전용 detail panel이 없음.** 상단 raw Summary/Resolution만 보여 ChassisLayoutFingerprint, bUseLayoutOverrides, NotCaptured, persisted/current exact mismatch, evaluator 문구가 사실상 유일한 설명 | Level 0: 현재 Wheel 배치가 Socket과 일치하는지, 아직 VehicleData 반영 전인지, 다음 행동. Level 1: Wheelbase/Front Track/Rear Track 및 위치 이상 요약. Level 2: fingerprint/override path/exact mismatch diagnostic |
| **5. AI 물리 설정** | Tab v1.28 Seed + Physics Draft + Builder-private Profile + Target VehicleData | 상단 흐름은 개선됐으나 상세는 loaded AI Draft만 표시하면서 title이 적용할 물리 설정 보기라 authority 혼동 가능. 값은 있으나 필드별 의미/영향 설명과 서스펜션 상세가 부족. Blocked 문구는 존재하지 않는 기술 상세 보기 label을 참조. 승인 Dialog는 Claim/Fingerprint/Profile hash/Transmission ProposalHash/ResolvedDefinitionHash/R1을 대량 노출 | Level 0: AI 제안 불러오기→검토→승인 흐름. Level 1: AI 제안 vs 현재 제작 프로필 명시, 엔진/변속/구동/조향·제동/서스펜션/질량의 값+뜻+영향. VehicleData는 Step 7에서 별도. Level 2: Claim/hash/profile fingerprint/transaction diagnostic |
| **6. 게임플레이 설정** | GetGameplaySetupUserSummaryText, Mount panel, BuildGameplayGuidanceSummary / Recipe Hardpoint+Mount + Gameplay Guidance | Step 6 root scroll과 기본 USER 문구는 개선됨. 남은 leak은 Legacy 모드의 Recipe Mount/custom·multi-Mount/Standard 1:1, draft status의 MountType/SizeLimit/EquipmentPreset, 진단 상세의 R0/RelatedFieldPath/PendingGameplayDiff. 현재 USER summary의 Hardpoint 수 - Mount 수 계산은 custom/multi-Mount에서 authoritative blocker로 사용하면 안 됨 | Level 0: 어느 장착 위치에 어떤 규칙이 부족한지. Level 1: 위치별 장착 방식/허용 크기/기본 프리셋. Level 2: Recipe/MountIntent/R0/field path/pending diff. Missing count는 typed Guidance authority 우선 |
| **7. 최종 검토** | BuildFinalReviewSummary, FCFVehicleFieldDiff, Apply/Undo handlers / FinalReviewResult | **가장 큰 developer dump.** Warning/Blocker/External Drift, Guarded Undo, DiffHash, provenance/Claim/Fingerprint, Transmission ProposalHash/Ratio/Overall/Retention, DefinitionApply hash/revision, raw canonical FieldPath/value, R0/R3가 모두 일반 상세과 확인 Dialog에 노출 | Level 0: 변경사항 수/문제 여부/적용 필요/다음 행동. Level 1: Set/Add/Remove/Move typed row + USER label + 단위 변환 Before/After + 의미/영향. Level 2: provenance/hash/field path/resolver revision/transaction id/R0/R3 |
| **8. 주행 테스트** | BuildDrivingTestSummary, benchmark result, Driving receipt, Runtime Catalog status / saved Target + benchmark + acceptance | Saved-state gate, Recipe Dirty, RunId, TargetHash, schema/runner identity가 기본 summary와 confirmation에 노출. 유용한 benchmark 수치와 체크리스트는 이미 있으나 기술 identity가 섞임. Runtime Catalog도 package clean/validation message 노출. root panel은 긴데 AutoHeight라 overflow 위험 | Level 0: 저장 필요 여부→벤치마크→PIE 적용→직접 주행→USER PASS 순서와 non-persistent checklist. Level 1: 0-50/0-100/최고속/RPM/제동/회전반경 등 수치+의미. Catalog는 등록/미등록/저장 필요만 표시. Level 2: RunId/DefinitionHash/schema/runner/receipt/catalog validation detail |

### 7.4 USER 기본 화면에서 차단해야 할 Direct-display 경로

P0-01은 최소 다음 direct-display 경로를 명시적으로 교체하거나 Level 2로 격리해야 한다.

~~~text
1. GetCurrentStepSummaryText()
   default -> Step.Summary
   영향: Step 1~4 / 7~8 raw VM diagnostic 노출

2. GetCurrentStepResolutionText()
   default -> Step.Resolution
   영향: Step 1~4 / 7~8 backend contract/action 문장 노출

3. GetReferenceSummaryText()
   -> BuildReferenceSummary()
   영향: Claim/Fingerprint/path/private Profile 노출

4. GetFinalReviewSummaryText()
   -> BuildFinalReviewSummary()
   영향: hash/provenance/raw FieldDiff/R0/R3 전체 노출

5. GetDrivingTestSummaryText()
   -> BuildDrivingTestSummary()
   영향: RunId/TargetHash/saved-state diagnostic 노출

6. LastStatusText handlers
   -> Error / Result.Message / Operation.Message 직접 append
   영향: 성공·실패 후 backend diagnostic 재노출

7. Step 1/5/7/8 FMessageDialog
   -> raw summary/hash/path/transaction identity 직접 삽입

8. Runtime Catalog status
   -> Promotion service validation/message 직접 사용자 문자열에 삽입
~~~

GetGameplaySetupSummaryText -> BuildGameplayGuidanceSummary는 이미 접힌 기술 상세 보기 (진단용)에 있으므로 Level 2 owner로 유지할 수 있다. 단, 일반 사용자 행동을 이 진단 텍스트에 의존시키지 않는다.

### 7.5 P0-01에 넘길 Exact Correction List

P0-01은 구현 전에 다음 설계 결정을 exact하게 닫는다.

1. **공통 Step Presentation API**
   - Step 1~8 Purpose / StatusSummary / NextAction을 raw Step.Summary/Resolution과 분리한다.
   - 필요한 typed fact가 없을 때만 VM read-only projection/getter를 최소 추가한다.

2. **USER Error/Status Translator**
   - LastStatusText와 confirmation 실패 경로를 UserSummary / RecoveryAction / DiagnosticDetail로 분리한다.
   - backend Operation.Message는 기본 문자열에 이어붙이지 않는다.

3. **Step 1 Reference Presentation**
   - Primary vehicle identity와 source/problem count typed projection을 정의한다.
   - Evidence path/hash/Claim/receipt/token은 Level 2로 이동한다.

4. **Step 2/3/4 Asset·Socket·Layout Presentation**
   - Asset 상태와 Socket 상태를 사용자 문장으로 투영한다.
   - Step 4 전용 Level 1 presentation을 추가하되 Step 3과 Wheelbase/Track authority를 중복 계산하지 않는다.
   - 실차 제원 표시는 mm, UE Socket 좌표는 cm 규칙을 적용한다.

5. **Step 5 Physics Source Authority**
   - AI 제안과 현재 제작 프로필을 각각 어떤 typed payload에서 읽을지 확정한다.
   - 승인 완료 후 Draft 재로드를 강요하지 않고 current approved profile을 read-only로 설명할 수 있는 projection을 검토한다.
   - 엔진/변속/구동/조향·제동/서스펜션/질량의 설명 descriptor를 확정한다.

6. **Step 6 Gameplay Presentation**
   - current Guidance result를 USER authority로 사용하고 단순 HardpointNum - MountNum 계산은 advisory 이상으로 승격하지 않는다.
   - Legacy/custom/multi-Mount 표현을 기존 구조 보존 의미로 번역한다.

7. **Step 7 Typed Diff Presenter**
   - Editor-only field presentation descriptor와 formatter를 정의한다.
   - Set/Add/Remove/Move, scalar/enum/bool/struct/array, unknown field fallback을 모두 계약한다.
   - FCFVehicleFieldRegistry Resolver metadata는 변경하지 않는다.

8. **Step 8 Driving / Catalog Presentation**
   - benchmark metrics와 USER checklist를 기술 identity에서 분리한다.
   - checklist는 non-persistent 유지.
   - Catalog는 등록됨 / 미등록 / 저장 필요 / 확인 실패의 USER 상태를 우선한다.
   - Step 8 root scroll 또는 bounded detail + fixed action 구조를 확정한다.

9. **Layout / Overflow 공통 규칙**
   - Step 5와 Step 8의 실제 최대 본문을 기준으로 bounded scroll 범위를 정한다.
   - 하단 이전 / 현재 상태 다시 확인 / 다음과 Step별 핵심 action이 항상 접근 가능해야 한다.

10. **Validation Seam**
    - Complete / Ready / Locked / Stale / Blocked
    - Step 7 Set/Add/Remove/Move + unknown field
    - USER error/recovery
    - Step 5 Draft/Profile authority
    - Step 8 overflow/checklist non-persistence
    를 focused fixture 대상으로 고정한다.

### 7.6 P0-00 Closure

~~~text
Audit implementation mutation = 0
Runtime/Asset mutation = 0
Editor restart = 0
Build/Automation = Not Required (document/read-only audit)
Parallel dirty protection = 유지
Next = VBIUX-P0-01 Presentation Contract Design Review
~~~

---

## 8. Gate 계획

### VBIUX-P0-00 — Full User-Facing Information Audit

목표:

- Step 1~8에서 USER에게 노출되는 Text/Tooltip/Dialog/Status/Error/Confirmation surface를 전수 분류
- `SCFVehicleBuilderTab` 고정 문구와 `FCFVehicleBuilderVM::Evaluate*Step()` raw Summary/Resolution source를 함께 추적
- 각 항목을 Level 0 / Level 1 / Level 2로 분류
- 내부 용어 leak 목록 작성
- 값은 있으나 의미 설명이 없는 항목 목록 작성
- 각 값의 source authority(Draft/Profile/VehicleData/Recipe/Asset/Benchmark) 기록
- raw backend `Operation.Message`가 USER에게 직접 노출되는 failure surface 확인
- scroll/overflow/readability 문제 확인

구현:

- 새 기능 구현 0
- 현재 Step 5 v1.28 Seed를 추가 확장하지 않음
- 필요하면 문서만 교정

PASS 조건:

- 1~8 Step 각각의 사용자 정보 inventory가 존재
- 모든 주요 surface에 `Source Owner / Source Authority / 현재 문구 / 목표 Level / 목표 표현`이 기록됨
- Technical-required와 diagnostic-only가 구분됨
- raw VM/backend message direct-display 예외가 식별됨
- P0-01에서 사용할 명확한 교정 목록이 확정됨

### VBIUX-P0-01 — Presentation Contract Design Review

**판정: PASS / Design Re-review P0-P1 0**

P0-00 inventory와 current typed Source를 다시 대조해 구현 전에 아래 Presentation 계약을 확정했다. 이번 Gate는 설계/문서 전용이며 C++/Asset/Runtime mutation은 수행하지 않는다.

#### P0-01A. 구현 Owner와 파일 경계

VBIUX 구현은 다음 3층으로 고정한다.

~~~text
FCFVehicleBuilderVM
= 기존 Step state / typed truth / backend diagnostic owner
= VBIUX 때문에 semantic decision을 새로 계산하지 않음
= 필요한 경우 read-only typed projection만 additive 노출

FCFVehicleBuilderPresentation
= 신규 Editor-private pure presentation helper
= typed truth → USER label/value/unit/meaning/effect/error/recovery 변환
= UObject mutation / disk write / Save / Apply / GEditor side effect 없음

SCFVehicleBuilderTab
= Slate layout / button / dialog / current step orchestration owner
= Level 0/1 USER UI와 collapsed Level 2 diagnostic host
~~~

신규 helper 파일은 32자 미만 이름을 유지한다.

~~~text
UE/Source/CarFight_ReEditor/Private/DataAuthoring/
- CFVehicleBuilderPresentation.h
- CFVehicleBuilderPresentation.cpp
~~~

Public module API로 승격하지 않는다. FCFVehicleFieldRegistry에는 USER label/meaning/unit metadata를 추가하지 않는다.

VM에는 필요한 경우 다음 범위의 read-only seam만 허용한다.

~~~text
ReadCurrentAssetSnapshot(...)
ReadCurrentPrivateProfilePayload(...)
ReadCurrentLayoutFacts(...)
~~~

BuildCurrentPrivateProfilePayload()의 existing owner/load/validation 의미를 재구현하지 않고 public read-only wrapper로 재사용한다. Layout fact는 Step 3/4 USER 화면이 같은 Wheel geometry 계산을 공유하도록 한 곳에서 계산하고 presentation에서 다시 별도 수학 authority를 만들지 않는다.

#### P0-01B. Level 0 공통 Step Frame

모든 Step은 상단 고정 영역에서 같은 순서로 표시한다.

~~~text
단계 이름 + 사용자 상태

이 단계의 목적
현재 상태
지금 할 일
~~~

상태 표기는 다음 USER 용어를 사용한다.

| Backend State | USER 표기 |
| --- | --- |
| Unavailable | 사용할 수 없음 |
| Locked | 이전 단계 필요 |
| Ready | 확인 / 작업 필요 |
| Complete | 완료 |
| Blocked | 진행 불가 |
| Stale | 다시 확인 필요 |

GetCurrentStepSummaryText()와 GetCurrentStepResolutionText()의 default raw Step.Summary/Resolution direct display는 제거한다. raw 문자열은 삭제하지 않고 Level 2 diagnostic에서만 읽는다.

#### P0-01C. Level 1 / Level 2 분리

Level 1은 사용자가 차량을 제작·판단하기 위한 실제 정보만 표시한다.

Level 2는 각 Step 하단의 공통 collapsed 진단 정보 영역으로 통합한다.

Level 2에는 필요할 때 다음을 보존한다.

~~~text
raw Step.Summary / Step.Resolution
BuildReferenceSummary
BuildPhysicsProposalSummary
BuildGameplayGuidanceSummary
BuildFinalReviewSummary
BuildDrivingTestSummary
Last backend diagnostic message
FieldPath / Hash / Fingerprint / RunId / Proposal / transaction identity
~~~

USER 정상 흐름이나 버튼 선택이 Level 2 내용을 읽어야만 진행 가능해서는 안 된다.

기존 Step 5의 적용할 물리 설정 보기는 Level 1 영역으로 재정의한다. 별도 Level 2 diagnostic과 혼동하지 않는다.

#### P0-01D. 사용자 Terminology Dictionary

| 내부 용어 | 기본 USER 표현 |
| --- | --- |
| Reference Evidence | 기준 자료 |
| Research Draft | AI 기준 자료 제안 |
| Companion | 기준 자료용 제작 보조 데이터 / 기본 화면에서는 가능하면 직접 노출하지 않음 |
| Builder-private Profile | 차량 전용 제작 프로필 |
| Recipe | 제작 기록 |
| Target VehicleData / Definition | 현재 차량 데이터 / VehicleData |
| Resolve / Resolved | 제작 설정을 반영해 계산된 값 |
| Apply / DefinitionApply | 차량 데이터에 반영 |
| Pending Diff | 적용 예정 변경 |
| External Drift | 마지막 적용 뒤 차량 데이터가 따로 변경됨 |
| Dirty | 저장되지 않은 변경 있음 / 저장 필요 |
| Hardpoint | 장비 장착 위치 (Hardpoint) — 첫 노출 후 Hardpoint 허용 |
| Mount / MountIntent | 장착 규칙 |
| Runtime Catalog | 데모 차량 목록 |
| Technical Benchmark | 기술 주행 측정 / 최초 설명 뒤 벤치마크 허용 |
| exact / mutation0 / R0~R3 / receipt / provenance | Level 2 진단 전용 |

Mesh, Socket, Wheelbase, Track, RPM, Torque, Gear, VehicleData는 실제 USER 작업 개념이므로 유지하되 처음 등장할 때 의미를 붙인다.

#### P0-01E. Step 1~4 Source / Presentation 계약

**Step 1**

Source priority:

~~~text
Persistent Reference Evidence 존재
→ 현재 기준 자료

Evidence 없음 + loaded ResearchDraft 존재
→ AI 기준 자료 제안

둘 다 없음
→ 기준 자료 준비 필요
~~~

Level 1에는 Primary 제조사/모델/연식/트림/파워트레인/변속기, 참고 Source 수, 해결이 필요한 conflict/unknown 유무만 표시한다. Claim ID, Evidence path/fingerprint, private Profile path는 Level 2로 이동한다.

**Step 2**

Recipe AssetIntent와 fresh AssetSnapshot을 source로 사용한다. Chassis/FL/FR/RL/RR은 full object path 대신 Asset 이름을 기본 표시한다.

FR/RL/RR 미지정은 다음 의미로 설명한다.

~~~text
해당 위치에 별도 Wheel Mesh를 지정하지 않으면 앞왼쪽 Wheel Mesh를 재사용합니다.
~~~

resolve, canonical, bounds, non-finite, AssetSnapshot은 기본 UI에서 사용하지 않는다.

**Step 3**

exact Socket 이름은 USER가 실제 편집해야 하므로 Level 1에서 유지한다. Wheel 4-role 존재 상태, shared StaticMesh 경고, 축 방향, Hardpoint 실제 위치 의미를 표시한다.

Step 3/4 Wheel geometry는 동일 typed fact source를 공유한다.

~~~text
Wheelbase = front axle midpoint X - rear axle midpoint X 절대값
Front Track = FR.Y - FL.Y 절대값
Rear Track = RR.Y - RL.Y 절대값
~~~

VM/current Asset truth의 cm를 USER 실차 제원 표시에만 mm로 변환한다. Socket 좌표 자체는 cm를 유지한다.

**Step 4**

전용 Level 1 panel을 추가한다.

~~~text
차량 전방 기준
Wheelbase
Front Track
Rear Track
Wheel 배치 이상 여부
현재 VehicleData 배치와 Socket 일치 여부
아직 미반영이면 Step 7에서 반영 예정이라는 설명
~~~

Ready / NotCaptured는 정상 deferred-Apply 상태임을 명확히 설명한다.

Stale은 현재 Builder에 존재하지 않는 recovery 기능을 UI 문구로 만들어내지 않는다. 현재 상태 재확인 뒤에도 계속 Stale이면 기존 managed authoring 경로에서 배치 차이를 점검하도록 안내하고, 실제 유효 recovery lane이 없다고 확인될 경우 별도 기능 결함으로 승격한다.

#### P0-01F. Step 5 Physics Source Authority 계약

Step 5는 화면 상단에 source badge를 반드시 표시한다.

~~~text
AI 제안
= loaded PhysicsDraft.ProfilePayload
= 아직 제작 프로필 승인 전

현재 제작 프로필
= current Recipe에 owner로 binding된 private 4 Profile typed payload
= USER가 승인한 persistent 제작 설정

VehicleData
= Step 5에서 current profile과 같다고 가정하지 않음
= Step 7 Before → After에서만 최종 적용 대상 표시
~~~

표시 source 선택 규칙:

1. Step Complete + current private payload valid → 현재 제작 프로필을 기본 표시
2. 아직 승인 전 + loaded Draft 존재 → AI 제안
3. persistent receipt/profile이 stale이면 현재 제작 프로필 — 다시 확인 필요
4. loaded Draft가 남아 있어도 Complete 상태에서 Draft를 승인된 current 값처럼 주표시하지 않음

FCFFeelResponse는 하나의 적용값으로 축약하지 않는다.

~~~text
가속 성향별 최대 토크
낮음 / 중립 / 높음

설명:
차량의 가속 성향 설정에 따라 이 범위 안에서 실제 값이 결정됩니다.
실제 VehicleData에 계산될 값은 Step 7에서 확인합니다.
~~~

즉 NeutralValue를 임의로 현재 적용값으로 표시하지 않는다.

Level 1 Physics 영역은 다음으로 고정한다.

~~~text
엔진
변속기
구동
조향 / 제동
서스펜션
차량 기본 / 질량
~~~

핵심 필드만 P0에서 노출하고 모든 Profile field를 한 화면에 덤프하지 않는다. 각 row는 값 + 의미 + 필요한 경우 일반적 영향을 갖는다.

#### P0-01G. Step 6 Gameplay 계약

FCFBuilderGameplayGuidanceResult.Items, SocketGuidance, BlockedCount, NeedsReviewCount, bCanCompleteGameplayStep가 USER 상태 authority다.

현재 Seed의 단순 HardpointIntents.Num() - MountIntents.Num() 계산은 custom/multi-Mount 구조에서 진실이 아닐 수 있으므로 blocker/완료 authority로 사용하지 않는다.

Level 1은 다음만 표시한다.

~~~text
장착 위치
장착 방식
허용 장비 크기
기본 장비 프리셋
Socket 수동 확인 필요 여부
완료 / 확인 필요 / 진행 불가
~~~

Legacy/custom/multi-Mount는 기존 차량의 장착 구조를 그대로 보존 중으로 설명한다. MountIntent, Recipe write, R0, RelatedFieldPath, pending gameplay diff는 Level 2다.

#### P0-01H. Step 7 Typed Diff Presenter 계약

Step 7은 신규 Editor-private descriptor를 사용하되 Resolver Registry와 별도 owner로 둔다.

기본 row 구조:

~~~text
Operation
UserLabel
BeforeValue
AfterValue
Meaning
EffectSummary
TechnicalFieldPath   // Level 2 only
bKnownField
~~~

Operation USER 이름:

| ECFVehicleDiffOp | USER 표현 |
| --- | --- |
| SetLeaf | 값 변경 |
| AddArrayElement | 추가 |
| RemoveArrayElement | 삭제 |
| MoveArrayElement | 순서 변경 |

Stable array pattern match는 numeric/index 문자열에 의존하지 않는다. FCFVehicleFieldPath의 collection/property chain/selector-key 이름을 비교하고 SelectorKeyValue만 wildcard로 취급한다.

Hardpoint/Mount 같은 stable collection은 selector 단위로 group한다.

~~~text
AddArrayElement + 같은 selector의 SetLeaf 여러 개
→ 사용자에게 장비 장착 위치 추가 한 묶음으로 표시

RemoveArrayElement
→ 해당 stable identity 삭제 한 묶음

기존 selector의 개별 SetLeaf
→ 실제 변경 field만 row 표시
~~~

따라서 신규 array element 한 개를 여러 raw leaf 변경으로 중복 표시하지 않는다.

MoveArrayElement는 현재 ApplyService 지원 여부와 별개로 USER에게 순서 변경 감지로 표시한다. Apply 가능 여부는 기존 FinalReviewResult가 계속 authority다.

Descriptor에는 Resolver semantics를 넣지 않고 USER metadata만 둔다.

~~~text
CanonicalPattern
UserLabel
ValueFormat
Meaning
EffectSummary
~~~

초기 mapping은 Guided Builder에서 실제 빈도가 높은 엔진/변속/구동/조향/제동/서스펜션/질량/Wheel/Hardpoint/Mount field로 bounded한다.

Unknown field fallback은 raw canonical dump를 Level 1에 다시 노출하지 않는다.

~~~text
기타 차량 설정 변경
설명되지 않은 차량 설정이 변경됩니다.
세부 기술 값은 진단 정보에서 확인할 수 있습니다.
~~~

단순 primitive이며 안전한 formatter가 명확한 경우에만 Before/After를 함께 표시할 수 있다. struct/array/object의 긴 canonical text는 Level 2 전용이다.

#### P0-01I. 단위 / Value Formatter 계약

Persistent canonical value는 변경하지 않고 presentation에서만 변환한다.

| 종류 | USER format |
| --- | --- |
| Torque | 1,234 Nm |
| RPM | 5,500 RPM |
| Mass | 1,886 kg |
| Runtime Speed | 123.4 km/h |
| Time | 0.40초 또는 측정값은 필요 시 3자리 |
| UE local distance | 123.4 cm |
| Wheelbase / Track | cm source → 1,234 mm |
| Angle | 12.5° |
| Ratio | 의미 있는 소수 2~4자리 |
| Bool | 사용 / 사용하지 않음, 있음 / 없음 등 field별 USER 문장 |
| Enum | internal enum token 금지, USER label mapping |
| Object/Class | full path 대신 Asset/Class display name 우선 |
| Vector/Rotator | USER 작업상 필요한 경우만 cm/deg 구성요소로 분리 |
| Atomic Struct / Array | 전용 formatter가 없으면 raw text를 Level 1에 출력하지 않음 |

FinalRatio 같은 값의 영향은 다른 조건이 같을 때라는 조건을 붙여 일반적 성향으로만 설명한다.

#### P0-01J. USER Error / Recovery 계약

문자열 parsing으로 오류를 분류하지 않는다.

FCFAuthoringOpResult가 있는 경로는 stable ECFAuthoringErrorCode와 ECFAuthoringOpStatus를 사용한다.

공통 분류:

~~~text
StateChanged / PreviewOutOfDate /
RecipeFingerprintMismatch / TargetHashMismatch /
ResolverRevisionMismatch / ApprovalScopeMismatch
→ 마지막 확인 뒤 상태가 변경됨
→ 현재 상태 다시 확인 후 다시 검토

TargetNotFound / RecipeNotFound / ProfileNotFound /
MissingRequiredSource
→ 필요한 제작 데이터가 없음
→ 해당 이전 단계/선택 상태 확인

ValidationBlocked / ApplyValidationFailed /
InvalidSemanticInput / DependencyConflict / StableIdConflict
→ 현재 설정에 해결해야 할 문제가 있음
→ 표시된 문제를 수정하고 다시 확인

UnsupportedOperation
→ 현재 Guided Builder에서 처리할 수 없는 변경
→ 자동 우회하지 않고 기존 고급 Authoring 경로 또는 별도 교정 필요

FailedUnknownState / ApplyRollbackFailed / InternalError
→ 작업 결과를 안전하게 확정할 수 없음
→ 자동 재시도 금지, current state 재확인 + 진단 정보 확인
~~~

VM API가 FString OutError만 반환하는 경로는 context-specific USER summary/recovery를 고정하고 raw OutError는 LastDiagnosticText에만 저장한다. 오류 문자열 내용으로 분기하지 않는다.

Tab의 transient presentation state는 최소 다음처럼 분리한다.

~~~text
LastStatusText
= USER summary + recovery

LastDiagnosticText
= raw backend diagnostic
~~~

Confirmation FMessageDialog에는 collapsed diagnostic UI가 없으므로 Hash/Path/Proposal/TransactionId를 다시 넣지 않는다. 승인 Dialog는 실제 변경 범위와 USER 판단만 표시하고 기술 진단은 main Step의 진단 정보에서 확인한다.

#### P0-01K. Step 8 Driving / Catalog 계약

Step 8 기본 순서를 다음 USER 흐름으로 고정한다.

~~~text
1. 저장 필요 여부 확인
2. 기술 주행 측정 실행
3. PIE에 선택 차량 적용
4. 직접 주행
5. USER 주행 PASS
6. 데모 차량 목록 등록 상태 확인
~~~

Dirty는 저장 필요로 표시한다. 어떤 package 저장이 실제 action prerequisite인지는 existing backend gate를 그대로 따른다.

Benchmark Level 1 수치는 다음을 유지한다.

~~~text
0→50
0→100
관측 최고 속도
최고 RPM / 당시 기어
100→저속 제동 시간/거리
회전반경
회전 측정 평균 속도
~~~

각 값은 측정 결과이며 자동 PASS/FAIL 기준이 아님을 명시한다.

주행 체크리스트는 transient UI 안내이며 Recipe/VehicleData/receipt에 항목별 상태를 저장하지 않는다.

USER Driving confirmation 기본 화면에서는 RunId/DefinitionHash를 제거한다. 기존 persistent Target-bound acceptance authority는 backend에서 그대로 유지하며 진단 정보에서만 identity를 볼 수 있다.

Runtime Catalog는 다음 USER 상태를 우선한다.

~~~text
데모 차량 목록 등록됨
데모 차량 목록 미등록
데모 차량 목록에 저장되지 않은 변경 있음
데모 차량 목록 상태를 확인할 수 없음
~~~

Promotion service raw validation/message는 Level 2다.

#### P0-01L. Scroll / Action 접근 계약

Global 이전 / 현재 상태 다시 확인 / 다음 navigation은 긴 detail scroll과 독립된 고정 영역을 유지한다.

Step별 규칙:

~~~text
Step 5
- Level 1 Physics detail = single FillHeight SScrollBox
- AI 불러오기 / 검토 후 반영 action = scroll 밖 고정

Step 6
- 기존 single ScrollBox 유지
- card 내부 편집 button은 content와 함께 scroll
- nested ScrollBox 추가 금지

Step 7
- human-readable diff/detail = single FillHeight SScrollBox
- Apply / Undo action = scroll 밖 고정

Step 8
- saved state / metric / checklist / Catalog detail = single FillHeight SScrollBox
- benchmark / PIE 적용 / USER PASS / Catalog retry = scroll 밖 고정
~~~

Level 2 diagnostic은 해당 Step의 single scroll 안에 collapsed 상태로 둔다.

#### P0-01M. 설계감사 교정 결과

초기 P0-01 방향을 current Source와 재대조하며 다음 P1 위험을 교정했다.

1. Presentation helper가 UObjects를 직접 load하거나 VM authority를 복제할 위험 → pure formatter + minimal read-only VM seam으로 제한
2. FCFFeelResponse.NeutralValue를 현재 적용값처럼 보일 위험 → Low/Neutral/High authored response로 그대로 표시
3. 오류를 backend FString parsing으로 번역할 위험 → ECFAuthoringErrorCode/Status 우선, FString-only는 context fallback
4. Unknown Field fallback이 raw canonical dump를 다시 만들 위험 → Level 1 generic summary, raw value Level 2
5. FMessageDialog에 Level 2까지 밀어 넣을 위험 → confirmation은 USER scope만, 진단은 main panel
6. Step 6 missing Mount 단순 산술이 custom/multi-Mount에서 틀릴 위험 → GameplayGuidanceResult authority로 교정
7. Step 7 AddArrayElement + child SetLeaf를 여러 변경으로 중복 표시할 위험 → stable selector grouping 계약 추가
8. Step 4 Stale에 존재하지 않는 자동 recovery를 안내할 위험 → 실제 existing route만 안내하고 기능 gap은 별도 승격

**Re-review 결과 P0 0 / P1 0. 구현 진입 가능.**

**Next exact Gate: VBIUX-P0-02 Step 1~4 Implementation**

### VBIUX-P0-02 — Step 1~4 Implementation

범위:

- Reference
- Mesh
- Socket
- Layout

조건:

- backend mutation 0
- 기존 Step 3 Socket workflow와 CF-FQ-043 완료 계약 보존
- exact Socket 이름은 유지

### VBIUX-P0-03 — Step 5~6 Implementation

범위:

- Physics
- Gameplay Setup
- 현재 v1.28 Seed 정규화
- Step 6 scroll UX 유지

조건:

- Physics 계산/commit 변경 0
- Mount Recipe write/remove 계약 변경 0

### VBIUX-P0-04 — Step 7~8 Implementation

범위:

- Final Review Set/Add/Remove/Move human-readable typed presentation
- 주요 field Before→After + meaning/effect/unit formatting
- unknown-field generic fallback
- Driving checklist
- Benchmark/Catalog 사용자 용어
- USER error/recovery + diagnostic detail 분리
- diagnostic identity 격리

조건:

- `FCFVehicleFieldRegistry` Resolver metadata 의미 변경 0
- Final Review Apply/Undo 변경 0
- Driving checklist persistent state 추가 0
- USER Driving receipt 변경 0
- Runtime Catalog promotion 변경 0

### VBIUX-P0-05 — Post-047 Integrated Correction + Regression + USER Acceptance

기존 2026-09-03 official Editor build PASS와 focused/affected Automation 8/8 PASS는 **CF-FQ-047 변경 전 Technical baseline evidence**로 보존한다. CF-FQ-047이 같은 `SCFVehicleBuilderTab` Step 3/5와 Presentation surface를 변경했고, USER Acceptance 중 공통 page overflow 문제가 추가 발견됐으므로 해당 PASS를 최종 closure evidence로 재사용하지 않는다.

#### VBIUX-P0-05A — CF-FQ-047 Completion Gate

착수 조건:

- CF-FQ-047 USER Acceptance와 필요한 교정이 모두 끝났을 것
- CF-FQ-047의 `SCFVehicleBuilderTab` / Hardpoint / Mount / Physics receipt 관련 기능 변경이 최종 동결됐을 것
- 047의 USER/Technical PASS와 문서 cleanup이 완료됐을 것

047 완료 전에는 046의 page layout 구현, build, Automation 재실행과 USER UAT를 시작하지 않는다.

#### VBIUX-P0-05B — Step 1~8 Common Page Layout Audit

Step 1~8 전체를 다음 항목으로 표 형태 감사한다.

```text
Step
root height policy
primary vertical Scroll owner
본문 container
사용자 상세 container
진단 정보 container
Expander expanded behavior
하단 action 위치
nested vertical Scroll 존재 여부
최대 콘텐츠 재현 상태
```

필수 재현 사례:

- Step 5 `물리 설정 상세보기` 최대 확장
- Step 8 `진단 정보` 최대 확장
- Step 1~8 각각의 Detail/Diagnostic/Expander 최대 동시 확장

Step 5와 Step 8만 수정 대상으로 좁히는 것을 금지한다.

#### VBIUX-P0-05C — Common Page Shell Contract + Design Review

감사 결과를 바탕으로 Step 1~8이 공유할 Slate page shell을 확정한다.

기본 구조:

```text
VerticalBox / root FillHeight
├─ 상단 핵심 정보            AutoHeight 또는 bounded
├─ 콘텐츠 영역              FillHeight(1.0)
│  └─ 단일 SScrollBox
│     ├─ 본문
│     ├─ 사용자 상세
│     ├─ Expander body
│     └─ 진단 정보
└─ 하단 작업 영역            AutoHeight / Scroll 밖
```

설계 원칙:

- 공통 helper 또는 공통 builder function을 우선 검토해 Step별 복사/편차를 줄인다.
- 단, 공통화 자체를 이유로 Step semantics, VM state authority, Hardpoint/Mount integrity, Physics receipt, Recipe/Apply/Save 로직을 옮기거나 재설계하지 않는다.
- 기존 Step별 action enable/disable 조건과 버튼 동작은 그대로 유지한다.
- Step별 고정 높이 magic number를 대량 추가하는 방식은 사용하지 않는다.
- 기존 Step 6/7 scroll 보강도 새 공통 shell과 중복/충돌하는지 재평가하고, 동일 정책으로 정리한다.

Design Review는 최소 다음을 확인한다.

- Step 1~8 모두 동일한 root height model을 사용 가능한가
- Expander를 펼쳐도 footer action이 밀리지 않는가
- nested vertical scrolling 없이 Detail/Diagnostic을 읽을 수 있는가
- 작은 viewport에서도 footer가 살아 있는가
- 047 기능 owner와 presentation-only owner 경계가 유지되는가

#### VBIUX-P0-05D — Step 1~8 Common Layout Implementation

범위:

- Step 1~8 page root / content / footer layout 교정
- 사용자 상세 / 진단 정보 / Expander expanded body를 공통 Scroll 영역에 수용
- Step별 중복 layout wrapper를 공통 helper로 줄일 수 있으면 bounded extraction
- 기존 문구/typed presentation은 이번 layout 교정에 필요한 범위 외에는 재작성하지 않음

금지:

- CF-FQ-047 Hardpoint integrity 변경
- Mount/Physics receipt validation 변경
- BuilderVM semantic 변경
- Recipe/VehicleData/Asset mutation 의미 변경
- auto-save 정책 변경
- USER Driving acceptance authority 변경

#### VBIUX-P0-05E — Post-047 Technical Revalidation

Technical:

- official Editor build
- Builder focused Automation
- VBIUX focused/affected Automation 전체 재실행
- CF-FQ-047 affected regression 중 실제 touched contract 재검증
- Complete / Ready / Locked / Stale / Blocked presentation fixture
- Step 7 Set/Add/Remove/Move + unknown-field fallback fixture
- failure path에서 USER recovery와 diagnostic detail 분리 fixture
- Step 1~8 common page shell 정적/구조 검증
- no-auto-save 유지
- backend semantic mutation diff 0 확인
- 동일 입력에서 기존 Step state / Proposal / Recipe / Apply / Driving acceptance / Catalog behavior 동일 확인

#### VBIUX-P0-05F — USER Acceptance

1. 각 Step을 보고 목적을 이해할 수 있다.
2. 현재 문제가 무엇인지 이해할 수 있다.
3. 다음에 누르거나 수정할 것이 무엇인지 이해할 수 있다.
4. 주요 숫자의 단위와 의미를 이해할 수 있다.
5. AI 제안값 / 현재 제작 프로필 / 현재 VehicleData를 혼동하지 않는다.
6. 일반 사용 중 hash/claim/receipt 같은 내부 진단 용어를 해석할 필요가 없다.
7. 대표 오류 화면에서 원인과 다음 복구 행동을 알 수 있다.
8. Step 1~8 어느 페이지에서도 상세/진단/Expander를 길게 펼친 뒤 전체 콘텐츠를 스크롤해 읽을 수 있다.
9. Step 1~8 어느 페이지에서도 긴 콘텐츠 때문에 하단 작업 UI가 화면 밖으로 밀려 접근 불가능해지지 않는다.
10. Step 8 체크리스트가 안내라는 점과 최종 `USER 주행 PASS`의 의미를 이해할 수 있다.

USER UAT는 정상 Wagon 1~8 E2E에 더해 대표 incomplete/error 상태 1~2개만 직접 확인한다. 모든 상태 조합은 Automation fixture가 담당한다.

### VBIUX-P0-06 — Current System Promotion

USER PASS 후:

- `Systems/Vehicles/VehicleBuilder.md`에 User-Facing Information Architecture 계약 승격
- FeatureQueue Done 전환
- ActiveWork / Plan Index current route cleanup
- 대표 Plan Historical 전환 판단

---

## 9. 구현 운영 전략

이번 Feature는 Editor를 단계마다 재시작하지 않는다.

기본 순서:

```text
P0-00 전체 감사
→ P0-01 설계 확정
→ P0-02~04 source batch implementation
→ pre-CF-FQ-047 P0-05 Technical baseline PASS 보존
→ CF-FQ-047 USER Acceptance / 교정 / 최종 동결 완료 대기
→ P0-05B Step 1~8 Common Page Layout Audit
→ P0-05C Common Page Shell Design Review
→ P0-05D Step 1~8 Layout Implementation
→ code review
→ Editor DLL lock이 실제 있을 때만 USER/managed lifecycle로 한 번 종료
→ P0-05E official Editor build + focused/affected Automation 재검증
→ fresh Editor
→ P0-05F 1~8 Step USER UAT
```

Source-only 감사/설계 중에는 편의를 위해 Editor를 종료하지 않는다.

현재 실행 중 Editor가 user-owned/unknown이면 build를 위해 자동 종료하지 않는다.

---

## 10. 검증 불변조건

다음은 UX가 좋아져도 반드시 그대로여야 한다.

```text
같은 차량 입력
→ 같은 Builder state
→ 같은 Proposal/Recipe/VehicleData result
→ 같은 Apply/Undo result
→ 같은 Driving acceptance authority
→ 같은 Runtime Catalog behavior
```

UI 문구 변경 때문에 정상 backend 결과가 바뀌면 VBIUX failure다.

Step 8 체크리스트는 UI 안내만 추가하며 Recipe/VehicleData/receipt에 새로운 persistent completion field를 만들지 않는다.

또한 사용자 화면에서 정보를 숨겼다는 이유로 diagnostic 자체를 삭제하지 않는다. 진단 데이터와 raw backend message는 backend/test/debug evidence로 보존한다.

---

## 11. Scope Out

이번 P0에서 하지 않는다.

- VehicleData 전체 필드 설명 사전 구축
- 모든 Unreal 기술 용어 제거
- BuilderVM 대규모 리팩터링
- Recipe schema migration
- DataAsset Manager와 통합
- Blueprint UI 재작성
- Editor style/theme 리디자인
- 자동 AI 설명 생성
- Localization framework 신규 구축
- Tooltip만으로 모든 설명 해결
- Runtime HUD 변경

---

## 12. 완료 정의

CF-FQ-046 P0는 다음이 모두 만족될 때 완료한다.

- 1~8 Step 사용자 정보 감사 완료
- 공통 Presentation 계약 PASS
- 주요 내부 진단 용어 기본 화면 제거
- 주요 수치에 source authority / USER 단위 / 뜻/영향 설명 추가
- 확정 의미와 일반적 성향 문구 구분
- Step 7 Set/Add/Remove/Move human-readable review + unknown-field fallback
- 오류 상태 USER summary/recovery + diagnostic detail 분리
- Step 8 non-persistent 실제 운전 체크리스트
- Step 1~8 공통 page height/scroll/Expander 정책 적용
- 모든 Step에서 상세/진단/Expander 최대 확장 시 콘텐츠 scroll 가능
- 모든 Step에서 하단 핵심 action이 긴 콘텐츠에 밀리지 않고 접근 가능
- 긴 Step scroll/overflow 및 핵심 action 접근 UAT PASS
- official Editor build PASS
- focused/affected Automation PASS
- no-auto-save / no-backend-semantic-change 확인
- USER가 1~8 Step을 실제 확인하고 이해 가능 판정

---

## 13. 현재 체크포인트

2026-09-03 승격 시점:

- USER가 Step 5 raw technical dump를 이해할 수 없다고 명시 피드백
- Step 6 overflow/scroll 문제는 선행 교정 + official Editor build PASS
- Step 5 v1.28 human-readable physics source Seed는 작성됐지만 아직 final build/UAT 전
- Step 1~8 quick Source scan에서 유사 internal terminology leak가 여러 Step에 존재함을 확인
- CF-FQ-039 Active 유지
- CF-FQ-041/044/045 Ready와 기존 dirty 변경 보호
- `VBIUX-P0-00 Full User-Facing Information Audit`은 1~8 Step + Tooltip/Dialog/LastStatus/Error surface 전수 감사로 PASS
- 가장 큰 공통 leak는 Step 1~4/7~8의 raw `Step.Summary/Resolution` default projection, Step 7 full developer dump, Step 8 technical identity + overflow 위험으로 확정
- `VBIUX-P0-01 Presentation Contract Design Review` PASS / Re-review P0-P1 0
- Editor-private `CFVehicleBuilderPresentation` helper + minimal VM read-only seam, common Level0/Level2, Step 5 source authority, Step 7 stable-selector diff grouping, typed error translation, Step 5/7/8 scroll/action contract 확정
- `VBIUX-P0-02 Step 1~4` source implementation 완료: Reference/Mesh/Socket/Layout 기본 화면을 USER 목적/상태/행동 + typed detail + collapsed `진단 정보` 구조로 전환
- `VBIUX-P0-03 Step 5~6` source implementation 완료: AI Draft↔승인 Profile source authority 분리, 물리 수치 `값+뜻+영향`, Mount/Gameplay USER 용어, Step 6 scroll/diagnostic 분리 적용
- `VBIUX-P0-04 Step 7~8` source implementation 완료: Before→After field descriptor, Set/Add/Remove/Move grouping, unknown fallback, 기술 주행 측정/직접 주행 checklist, 데모 차량 목록 USER 용어, Step 7/8 diagnostic identity 격리 적용
- P0-04 static terminology audit에서 기본 UI string literal의 `DefinitionApply`, `ProposalHash`, `DefinitionHash`, `RunId`, `Runtime Demo Catalog`, `USER Driving`, `Technical Benchmark`, `mutation0`, `exact`, `managed Recipe`, `private` 노출 0 확인. `EvidenceFingerprint`는 collapsed diagnostic 문자열에만 보존
- `CFVBPresentTests.cpp` + `Tools/RunVBIUXTests.ps1`로 P0-05 PresentationStates / FinalReviewDiff / DrivingSummary와 기존 affected Step 1/5/7/8 exact regression fixture 준비
- USER가 Editor를 정상 종료한 뒤 official UE 5.8 `CarFight_ReEditor Win64 Development` build를 실행해 **PASS**. build job `3c9330dc7ee64fff9ab8de24808f66f4`, exit 0, 9.085초. 기존 Tripo3DUEBridge→Interchange dependency warning만 유지
- `Tools/RunVBIUXTests.ps1` focused/affected regression을 실행해 **8/8 PASS**, process job `6ad5ecd8d662488383e9eaa1c6f5f558`, exit 0, `VBIUX_FOCUSED_AUTOMATION=PASS`
- 신규 Presentation 3종 `PresentationStates / FinalReviewDiff / DrivingSummary` PASS와 기존 affected `BuilderShell / BuilderStep1Reference / BuilderStep5Physics / BuilderStep7FinalReview / BuilderStep8Driving` PASS 확인
- Automation 동안 Product Asset Save/PIE mutation은 수행하지 않았고 테스트 runner는 exact filter별 terminal `SUCCESS_COUNT=1 / FAILURE_COUNT=0`을 반환
- 2026-09-03 P0-05 Technical Validation PASS는 CF-FQ-047 변경 전 baseline evidence로 보존한다. 이후 047이 같은 Step 3/5와 Presentation surface를 변경했으므로 최종 closure 전 재검증이 필요하다.
- 2026-09-04 CF-FQ-047 USER Acceptance 중 Step 5 물리 설정 상세와 Step 8 진단 정보에서 긴 콘텐츠 확장 시 page height/scroll 불일치가 재현됐다. 두 화면을 개별 수정하지 않고 Step 1~8 전체 page body/detail/diagnostic/Expander를 공통 양식으로 감사·교정하는 후속 Gate를 추가했다.
- CF-FQ-047은 아직 완료되지 않았으므로 046 구현은 시작하지 않는다. 047 최종 동결 후 `P0-05B Audit → P0-05C Design Review → P0-05D Implementation → P0-05E Technical Revalidation → P0-05F USER Acceptance` 순서로 진행한다.
- **Historical Dependency Gate (2026-09-04): `CF-FQ-047 완료 및 관련 Vehicle Builder 변경 동결` — 현재 충족됨**

### 13.1 2026-09-15 post-CF-FQ-047 fresh rebaseline / P0-05B~05E 결과

- 실제 `SCFVehicleBuilderTab` 기준 fresh layout audit에서 Step 1/2/4/5의 page scroll 부재, Step 3/6의 개별 vertical Scroll, Step 7/8의 `430/380` 고정 높이 nested Scroll이 하나의 Common Page Shell 계약을 사용하지 않는 **A. Common Shell 문제**로 확인됐다. 긴 Step 5/8 detail에서 하단 접근을 막을 수 있으므로 **E. 실제 기능 blocker** 성격도 함께 가졌다.
- CF-FQ-047 Hardpoint/Mount/Physics/Step 7 durable save/Step 8 Driving readiness, CF-FQ-038 Data Authoring backend, CF-FQ-015 Performance Tuning은 owner 밖으로 분류해 재설계·중복 구현하지 않았다.
- `CFVehicleBuilderTab.cpp v1.39.0` / `CFVehicleBuilderTab.h v1.29.0`에서 Step 1~8 body/detail/diagnostic을 **단일 FillHeight `CommonPageScrollBox`**로 통합하고 Step 3/6 local vertical Scroll과 Step 7/8 `HeightOverride(430/380)`을 제거했다. Step 5/7/8 핵심 action과 Global Navigation은 Scroll 밖에 유지하고 기존 handler/enable/save/apply authority를 그대로 재사용한다.
- Step 직접 선택, 이전/다음, 차량 target 변경과 신규 차량 진입 시 `ScrollToStart()`로 공통 Scroll을 새 page 상단으로 복귀시킨다. 현재 상태 새로고침은 사용자가 읽던 위치를 유지한다.
- 작은 폭에서 긴 한국어 action label이 밀리는 위험을 줄이기 위해 Step 1 action과 Step 5/7/8 고정 action, Global Navigation을 wrapping/compact layout으로 교정했다. 새로운 대형 UI framework나 Builder rewrite는 추가하지 않았다.
- fresh static audit에서 Vehicle Builder page vertical `SScrollBox`는 common owner exact1, `HeightOverride` exact0이며 Step 5/7/8 body와 fixed action은 같은 Step visibility를 사용한다.
- Official UE 5.8 Editor Build job `a383a2177c8445c8aa18d7f15edcacef`는 exit 0 **PASS**했다. `CFVehicleBuilderTab.cpp` non-unity compile과 `UnrealEditor-CarFight_ReEditor.dll` link까지 완료됐다.
- `Tools/RunVBIUXTests.ps1` process `27362090dd734973b950842b2f8f5123`은 exact 8/8 **PASS**했다. 신규 Presentation 3종 `PresentationStates / FinalReviewDiff / DrivingSummary`와 affected Builder 5종 `BuilderShell / BuilderStep1Reference / BuilderStep5Physics / BuilderStep7FinalReview / BuilderStep8Driving`이 모두 `SUCCESS_COUNT=1 / FAILURE_COUNT=0`으로 종료됐다.
- 독립 fixture 재검수에서 `PresentationStates`는 USER recovery와 backend fallback 격리, `FinalReviewDiff`는 Set/Add/Remove/Move + grouped child + unknown fallback, `DrivingSummary`는 USER metric/checklist와 RunId/DefinitionHash 비노출을 직접 검증한다.
- Post-Implementation Mid-review 판정은 **P0 0 / blocking P1 0 / non-blocking P2 0 — Technical PASS**다. Product Asset 자동 Save/Save All과 047/038/015 backend semantic mutation은 수행하지 않았다.
- 작은 Editor 높이, DPI scaling, 긴 한국어/Asset path/Validation message, diagnostic expand 상태의 실제 읽기 편함과 action 접근성은 사람 판단이 필요하므로 Technical PASS로 대체하지 않는다.
- **VBIUX-P0-05F USER Acceptance: PASS (2026-09-15)**
- USER는 실제 Vehicle Builder Step 1~8에서 작은 창 높이, common scroll, overflow, 긴 한국어/경로/진단 표시, Step 5/7/8 fixed action 접근성, Step 전환 시 scroll-to-start 동작을 확인하고 PASS를 승인했다.
- **VBIUX-P0-06 Current System Promotion / Final Closure Audit: PASS (2026-09-15)**
- G0 구현/Build/Automation/USER Acceptance = PASS
- G1 Current Knowledge → `Document/Systems/Vehicles/VehicleBuilder.md v1.7.0` + `Document/Systems/SystemIndex.md v1.47.0` 승격 = PASS
- G2 ActiveWork / Plan Index / FeatureQueue의 stale Ready route 제거 = PASS
- G3 대표 Plan 탐색 경로 보존 = PASS
- G4 semantic Historical = PASS
- G5 physical move = Deferred / Not Required. 본 문서는 `VehicleBuilderInfoUX/VehicleBuilderInfoUXPlan.md` 경로를 유지하는 Historical + Retained Path다.
- **Current exact next Gate: 없음. CF-FQ-046 closure complete.**

---

## 14. Changelog

### v0.2.0 - 2026-09-15

- `VBIUX-P0-06 Current System Promotion / Final Closure Audit`을 완료했다.
- Step 1~8 User-Facing Information Architecture와 Common Page Shell/scroll/overflow 계약을 `VehicleBuilder.md v1.7.0`으로 승격했고 `SystemIndex.md v1.47.0`에 Current owner를 반영했다.
- G0~G4를 PASS하고 CF-FQ-046을 Done → Historical + Retained Path로 전환했다. G5 physical move는 링크/dirty 위험을 만들 이유가 없어 Deferred한다.
- ActiveWork/Plan Index/FeatureQueue의 Ready current route를 제거하고 Archive Index에 retained Historical 탐색 경로를 남긴다.
- Product Asset/Recipe/Profile mutation, backend authority 재설계, Save All/자동 Save는 수행하지 않았다. USER UAT 중 별도로 dirty가 된 `DA_Recipe_Wagon.uasset`은 closure 범위에서 변경·정리하지 않았다.

### v0.1.8 - 2026-09-15

- `VBIUX-P0-05F USER Acceptance`를 USER 직접 확인으로 PASS 처리했다.
- Step 1~8의 작은 viewport, 단일 common scroll, overflow, 긴 한국어/경로/diagnostic, Step 5/7/8 fixed action 접근성, Step 전환 scroll reset에서 사용자 차단 문제 없음이 승인됐다.
- CF-FQ-046은 아직 Ready를 유지하며 정확한 다음 Gate를 `VBIUX-P0-06 Current System Promotion / Final Closure Audit`로 전진했다. P0-06을 이번 PASS 응답에서 자동 시작하거나 Done/History로 승격하지 않는다.

### v0.1.7 - 2026-09-15

- CF-FQ-047 완료 후 current `VehicleBuilder.md v1.6.0`과 `SCFVehicleBuilderTab`을 fresh rebaseline해 P0-05B Common Page Layout Audit와 P0-05C Design Review를 완료했다.
- Step 1~8 body/detail/diagnostic을 단일 FillHeight `CommonPageScrollBox`로 통합하고 Step 3/6 local Scroll, Step 7/8 `HeightOverride(430/380)`을 제거했다. Step 5/7/8 핵심 action과 Global Navigation은 Scroll 밖에 유지하고 기존 backend callback/enable/save/apply authority를 보존했다.
- Official UE 5.8 Build PASS + focused/affected exact8 8/8 PASS를 fresh 확보했고 Post-Implementation Mid-review를 P0 0 / blocking P1 0 / P2 0 Technical PASS로 닫았다.
- USER 시각·읽기 편함·작은 viewport/DPI·긴 텍스트 실제 접근성은 아직 승인하지 않았으며 exact next를 `VBIUX-P0-05F USER Acceptance`로 전진했다. USER PASS 전 P0-06 Current System Promotion과 Done 승격은 금지한다.

### v0.1.6 - 2026-09-04

- CF-FQ-047 USER Acceptance 중 확인된 Step 5 물리 설정 상세/Step 8 진단 정보 overflow를 두 화면의 개별 버그로 처리하지 않고, Step 1~8 전체 page body/detail/diagnostic/Expander의 공통 height/scroll 정책 문제로 승격했다.
- 공통 page shell을 `고정 상단 핵심 정보 + FillHeight 단일 세로 Scroll 콘텐츠 영역 + 고정 하단 action`으로 설계하는 방향을 4.9 계약에 추가하고 nested vertical ScrollBox와 Step별 고정 높이 magic number를 기본 금지했다.
- P0-05를 post-047 integrated correction gate로 확장해 `P0-05A 047 Completion Gate → P0-05B Step 1~8 Audit → P0-05C Common Page Shell Design Review → P0-05D Implementation → P0-05E Technical Revalidation → P0-05F USER Acceptance` 순서를 고정했다.
- 기존 2026-09-03 official Editor build PASS + focused/affected Automation 8/8 PASS는 pre-CF-FQ-047 baseline evidence로 보존하되 최종 closure evidence로 재사용하지 않는다.
- CF-FQ-047이 아직 완료되지 않았으므로 이번 변경은 Plan 예약만 수행하며 046 source/build/Automation/Editor/User UAT는 시작하지 않는다.

### v0.1.5 - 2026-09-03

- USER가 Editor를 정상 종료한 뒤 official UE 5.8 Editor build를 실행해 `CarFight_ReEditor Win64 Development` **PASS**를 확인했다. build job `3c9330dc7ee64fff9ab8de24808f66f4`, exit 0이며 기존 Tripo3DUEBridge의 Interchange dependency warning 외 신규 build blocker는 없다.
- `RunVBIUXTests.ps1`을 실행해 VBIUX 신규 Presentation fixture 3건과 기존 affected Builder Step regression 5건, 합계 **8/8 PASS**를 확인했다. process job `6ad5ecd8d662488383e9eaa1c6f5f558`, exit 0, final marker `VBIUX_FOCUSED_AUTOMATION=PASS`다.
- `PresentationStates`는 Complete/Ready/Locked/Stale/Blocked recovery와 backend fallback 격리, `FinalReviewDiff`는 Set/Add/Remove/Move/structural child grouping/unknown fallback, `DrivingSummary`는 metric/checklist와 RunId/DefinitionHash 비노출을 직접 검증했다.
- 기존 `BuilderShell`, `BuilderStep1Reference`, `BuilderStep5Physics`, `BuilderStep7FinalReview`, `BuilderStep8Driving` affected regression도 모두 PASS하여 UX presentation 변경 뒤 기존 Builder 동작 계약이 유지됨을 확인했다.
- P0-05 Technical Validation을 PASS로 전환했다. 남은 Gate는 fresh Editor의 정상 Wagon 1~8 Step + 대표 incomplete/error 상태 USER UAT이며, USER PASS 전에는 P0-06 Current System Promotion으로 넘어가지 않는다.

### v0.1.4 - 2026-09-03

- `VBIUX-P0-02~04` source batch implementation과 static audit을 완료했다. Step 1~8 기본 화면을 USER 목적/상태/다음 행동 중심으로 정리하고 raw backend/hash/path/identity는 collapsed `진단 정보`에 격리했다.
- Step 5는 AI Draft와 승인 완료 Builder-private Profile을 구분하고 엔진/변속기/구동/조향·제동/서스펜션/질량을 `값 + 뜻 + 주행 영향`으로 표시한다. 실제 VehicleData 값은 Step 7에서 재확인한다.
- Step 7은 authoritative `FCFBuilderFinalReviewResult::FieldDiff`를 Set/Add/Remove/Move 보존 상태로 USER field descriptor에 투영하고 Hardpoint/Mount structural row를 grouping한다. 설명되지 않은 field는 raw path를 노출하지 않고 generic fallback + diagnostic 경로로 처리한다.
- Step 8은 saved-state, 0→50/100, 최고속도, RPM/기어, 제동, 회전반경과 non-persistent 직접 주행 checklist를 표시하며 기술 측정값이 자동 합격/불합격이 아님을 명시한다. USER PASS/Catalog backend authority는 유지하고 기본 표기는 `주행 테스트 통과` / `데모 차량 목록`으로 전환했다.
- P0-05용 `CFVBPresentTests.cpp`와 `RunVBIUXTests.ps1`을 추가해 state recovery, structural diff/unknown fallback, metric/checklist/identity isolation + existing affected Builder Step 회귀를 exact filter로 준비했다.
- 현재 Editor lifetime이 connected 상태라 자동 종료하지 않았다. official Editor build와 focused/affected Automation은 아직 미실행이며 next Gate는 P0-05 Technical Validation이다.

### v0.1.3 - 2026-09-03

- `VBIUX-P0-01 Presentation Contract Design Review`을 완료하고 current Source와 재검수해 **P0/P1 0 PASS**로 닫았다.
- 구현 owner를 `FCFVehicleBuilderVM typed truth` / Editor-private `FCFVehicleBuilderPresentation pure formatter` / `SCFVehicleBuilderTab Slate host` 3층으로 확정하고 VM 변경은 current Asset/Profile/Layout fact의 read-only seam만 허용했다.
- 1~8 공통 Level 0 `목적/현재 상태/지금 할 일`, collapsed Level 2 diagnostic, terminology dictionary, 단위/value formatter, stable `ECFAuthoringErrorCode` 기반 recovery contract를 확정했다.
- Step 5는 AI Draft/승인된 private Profile/VehicleData source authority를 분리하고 FeelResponse neutral을 current 값으로 축약하지 않도록 고정했다.
- Step 7은 Editor-only field descriptor, stable selector wildcard match, Add/Remove structural grouping, unknown-field safe fallback을 확정했으며 `FCFVehicleFieldRegistry`는 변경하지 않는다.
- Step 8은 metric/checklist/Catalog를 diagnostic identity와 분리하고 Step 5/7/8 single-scroll + action-fixed layout을 확정했다. next Gate는 `VBIUX-P0-02 Step 1~4 Implementation`이다.

### v0.1.2 - 2026-09-03

- `VBIUX-P0-00 Full User-Facing Information Audit`을 read-only로 완료하고 1~8 Step의 Text/Tooltip/Dialog/Status/Error/Confirmation surface와 source authority를 inventory화했다.
- Step 1~4/7~8의 top frame이 raw `Step.Summary/Resolution`을 직접 사용하고, Step 1 `BuildReferenceSummary`, Step 7 `BuildFinalReviewSummary`, Step 8 `BuildDrivingTestSummary`, 공통 `LastStatusText`/confirmation이 backend diagnostic을 USER에게 다시 노출하는 direct-display 경로를 확정했다.
- Step 4는 전용 detail presentation 부재, Step 5는 Draft↔approved Profile authority 혼동과 의미 설명 부족, Step 6은 Legacy/custom 표현과 advisory count 경계, Step 7은 raw canonical FieldDiff, Step 8은 RunId/DefinitionHash와 root overflow 위험을 P0-01 exact correction list로 넘겼다.
- Audit 동안 C++/Asset/Runtime mutation, Editor restart, Build/Automation은 0이며 next Gate를 `VBIUX-P0-01 Presentation Contract Design Review`로 전진했다.

### v0.1.1 - 2026-09-03

- Correction 후 current `VehicleBuilder.md v1.2.0`, `SCFVehicleBuilderTab`, `FCFVehicleBuilderVM`, `FCFVehicleFieldDiff`, `FCFVehicleFieldRegistry` 계약과 재대조했고 **Design Re-review P0/P1 0 PASS**로 판정했다. 기존 기능 authority를 재설계하지 않고 VBIUX-P0-00 감사로 진입 가능한 상태다.
- 설계검수 P1 7건과 P2 보강을 반영했다. VM typed truth/backend diagnostic과 USER Presentation owner를 분리하고 raw `Summary/Resolution/Operation.Message`를 기본 USER authority로 사용하지 않도록 계약했다.
- Step 5는 AI Draft / 승인된 Builder-private Profile / Target VehicleData source authority를 분리하고 Presentation-only 단위/format 규칙을 추가했다.
- `값 + 의미 + 영향` 설명에서 설정이 직접 정의하는 확정 의미와 다른 필드에 의존하는 일반적 성향을 구분하도록 했다.
- Step 7은 `FCFVehicleFieldDiff` Set/Add/Remove/Move를 보존하는 typed presentation, Editor-only field mapping, unknown-field generic fallback을 추가하고 Resolver `FCFVehicleFieldRegistry`와 USER metadata ownership을 분리했다.
- 정상/실패 surface 모두 USER Error Summary + Recovery Action + Diagnostic Detail 계층을 적용하고, Step 8 체크리스트는 non-persistent 안내로 고정해 기존 USER Driving receipt authority를 보호했다.
- 공통 scroll/overflow contract와 Complete/Ready/Locked/Stale/Blocked + structural diff Automation matrix를 P0-05에 추가했다.

### v0.1.0 - 2026-09-03

- USER 승인으로 `CF-FQ-046 Vehicle Builder 사용자 정보 UX` 정식 Plan을 신규 생성했다.
- Step 5 raw diagnostic dump와 Step 6 overflow 피드백을 직접 발생 근거로 기록했다.
- 1~8 Step 전체를 Level 0 기본 / Level 1 사용자 상세 / Level 2 기술 진단으로 분리하는 정보 계층을 기본 방향으로 고정했다.
- 주요 수치는 `값 + 의미 + 게임 영향` 방식으로 표시하고, Step 7은 Before→After human-readable review, Step 8은 실제 주행 체크리스트를 중심으로 재구성하도록 했다.
- backend/Recipe/Apply/Driving/Catalog authority는 변경하지 않으며 next Gate를 `VBIUX-P0-00 Full User-Facing Information Audit`로 설정했다.
