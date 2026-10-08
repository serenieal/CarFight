# CarFight Builder / Authoring Guide Standard

- 문서 버전: v1.0.0
- 최근 갱신일: 2026-09-17
- 문서 상태: Current Project Standard
- 적용 범위: CarFight Editor의 신규 Builder(제작기), Authoring Guide(제작 가이드), 단계형 제작 Workflow(작업 흐름)
- 기준 사례: `Document/Systems/Vehicles/VehicleBuilder.md`와 현재 `SCFVehicleBuilderTab` 구현

---

## 1. 문서 목적

이 문서는 CarFight에서 새로운 Builder 또는 Authoring Guide를 만들 때 **매번 같은 UX·상태·저장·검증 원칙을 다시 결정하지 않도록 하는 프로젝트 공통 규약**이다.

Builder마다 도메인 데이터와 실제 제작 단계는 달라질 수 있다. 그러나 정상 제작 사용자가 배우는 화면 구조, 단계 진행 방식, 신규/기존 자산 처리, Validation, Apply/Save, 오류 복구, Advanced 편집 경계는 가능한 한 같은 규칙을 사용한다.

이 문서는 거대한 범용 Builder Framework를 강제하는 문서가 아니다. 공통 UX와 Architecture 계약을 먼저 고정하고, 실제 공용 코드 추출은 둘 이상의 구체적인 Builder에서 반복 구현이 확인됐을 때 최소 범위로 수행한다.

현재 Vehicle Builder는 이 규약의 기준 사례다. 기존 Vehicle Builder를 이 문서 때문에 재작성하지 않는다. 신규 Builder와 큰 폭으로 다시 설계하는 기존 Builder가 이 규약을 기본값으로 사용한다.

---

## 2. 핵심 정의

### 2.1 Builder / Authoring Guide

Builder는 **DataAsset 한 종류를 편집하는 화면**이 아니라 사용자가 하나의 실제 제작 업무를 처음부터 끝까지 완료하는 Guided Workflow다.

예:

```text
차량 한 대를 만든다
→ Vehicle Builder

무장 하나를 만든다
→ Weapon Authoring Guide
```

내부적으로 DataAsset이 여러 개 생성되더라도 사용자에게는 하나의 제작 업무로 보여야 한다.

### 2.2 Step / Page

Step은 사용자가 한 시점에 해결할 **하나의 작업 의도 또는 하나의 주요 질문**이다.

페이지 수를 줄이는 것보다 한 페이지의 판단 부담을 줄이는 것을 우선한다.

### 2.3 Embedded Editor Panel

현재 Builder가 만드는 대상의 일부 데이터를 다루는 소형 편집 영역이다. 별도 Builder가 아니다.

예를 들어 Weapon Guide의 Damage, Ammo, Projectile 설정은 내부 구현상 서로 다른 DataAsset이어도 정상 제작 UX에서는 현재 Guide의 Step 또는 Panel로 다룰 수 있다.

### 2.4 Advanced Workspace / Native Editor

전문가용 유지보수, 복구, 특수한 세부 조정에 사용하는 별도 화면이다.

정상적인 신규 제작을 끝내기 위해 Advanced Workspace나 Native DataAsset Editor 진입이 필수라면 Guided Builder의 범위가 불완전한 것으로 본다.

---

## 3. 최상위 설계 원칙

### 3.1 하나의 사용자 업무는 하나의 Builder에서 완료한다

Builder의 분리 기준은 C++ 클래스나 DataAsset 타입이 아니라 **사용자 업무가 바뀌는가**다.

허용되는 예:

```text
Weapon Authoring Guide
→ 무장 제작 완료
→ Equipment Builder
```

`무장 콘텐츠 제작`에서 `완성 장비 프리셋 조립`으로 업무가 바뀌기 때문에 별도 Builder 이동이 가능하다.

기본적으로 금지되는 예:

```text
Weapon Guide
→ Projectile Builder
→ Damage Builder
→ Ammo Builder
```

Projectile, Damage, Ammo가 현재 무장을 완성하기 위한 구성요소라면 현재 Weapon Guide 안에서 생성·선택·검증할 수 있어야 한다.

### 3.2 Builder Chaining 금지

현재 Builder가 만드는 결과물의 **필수 또는 통상적인 하위 구성요소**를 만들기 위해 다른 Builder를 연쇄적으로 열게 하지 않는다.

하위 데이터가 독립 DataAsset이거나 별도 Durable Writer를 사용한다는 사실은 내부 구현 사정이다. 정상 사용자 Workflow를 분리하는 근거가 아니다.

다른 Builder로 이동하려면 다음 중 하나가 성립해야 한다.

```text
- 사용자 업무 자체가 다음 업무로 전환된다.
- 현재 도메인 소유권 밖의 독립 결과물을 제작한다.
- 명시적인 프로젝트 설계상 별도 업무로 승인돼 있다.
```

### 3.3 도메인 완결성

도메인 Builder는 **현재 CarFight Runtime이 지원하는 해당 도메인의 정상 제작 범위 전체**를 처리할 수 있어야 한다.

예를 들어 Weapon Guide라면 Launcher, Guidance, Heat, Charge 같은 Runtime Capability가 이미 존재하는데 단지 Guide UI가 없다는 이유로 다른 제작기로 보내서는 안 된다.

기능을 만들 수 없는 정당한 사유는 `다른 Builder가 필요함`이 아니라 다음처럼 명확해야 한다.

```text
현재 CarFight Runtime이 이 Capability를 아직 지원하지 않음
```

### 3.4 내부 자산 구조를 사용자 Workflow로 노출하지 않는다

사용자가 만드는 개념과 내부 저장 구조를 구분한다.

```text
사용자 개념
= 120mm 캐논 하나

내부 결과
= WeaponData + TurretMountData + ProjectileData + DamageData + AmmoData + 기타 필요한 DataAsset
```

사용자는 내부 ObjectPath, PackagePath, Stable ID와 DataAsset 연결 순서를 외워야 정상 제작할 수 있어서는 안 된다.

---

## 4. 공통 화면 레이아웃

Vehicle Builder의 현재 Common Page Shell을 CarFight Builder 계열의 기본 레이아웃으로 사용한다.

```text
┌──────────────────────────────────────────────────────────────┐
│ 현재 제작 대상 / 현재 Step / 상태 / 지금 할 일              │  ← 고정 USER Header
├───────────────────┬──────────────────────────────────────────┤
│ 작업 대상          │                                          │
│ 또는 Step 목록     │ 현재 Step 본문                           │
│                   │                                          │
│ ✓ 완료             │ 한 Step의 작업 의도만 다룸              │
│ ● 현재             │                                          │
│ ○ 다음             │ 상세 / 고급 / 진단은 계층화             │
│                   │                                          │
│                   │                         단일 본문 Scroll  │
├───────────────────┴──────────────────────────────────────────┤
│ 현재 Step 핵심 Action                                       │  ← 본문 Scroll 밖
├──────────────────────────────────────────────────────────────┤
│ 이전 │ 현재 상태 다시 확인 │ 다음 │ Advanced                 │  ← 고정 Global Navigation
└──────────────────────────────────────────────────────────────┘
```

### 4.1 고정 Header

Header는 최소한 다음을 사용자가 바로 알 수 있게 한다.

```text
무엇을 만들고 있는가
현재 어느 단계인가
현재 상태는 무엇인가
지금 무엇을 해야 하는가
```

Backend message, Hash, Fingerprint, RunId 같은 진단 정보는 Header의 기본 문장에 넣지 않는다.

### 4.2 Step Navigation

Step 의미는 고정 배열 index가 아니라 **Stable StepId**로 식별한다.

조건에 따라 Step이 삽입·제거되더라도 의미가 index에 결합되지 않아야 한다.

Step 목록은 완료/현재/진행 가능/잠김/문제 상태를 구분해서 보여준다. Step 수가 많으면 Navigation 영역 자체에 독립 Scroll을 둘 수 있지만 본문 Scroll과 역할을 섞지 않는다.

### 4.3 본문 Scroll

현재 Step의 일반 본문, 상세 설명, 접힌 진단 정보는 **하나의 주 세로 Scroll owner**를 사용한다.

독립 viewport가 반드시 필요한 기능이 아니라면 Step 내부에 중첩 Vertical Scroll을 추가하지 않는다.

Step 직접 선택, 이전/다음 이동, 제작 대상 변경 시 새 Page 상단으로 이동한다. `현재 상태 다시 확인`처럼 같은 Page를 새로 평가하는 동작은 사용자가 읽던 위치를 불필요하게 잃지 않도록 Scroll 위치를 유지한다.

### 4.4 핵심 Action과 Global Navigation

현재 Step의 핵심 실행 버튼과 `이전 / 상태 다시 확인 / 다음 / Advanced`는 긴 본문 때문에 화면 아래로 밀려 접근할 수 없게 만들지 않는다.

작은 Editor 높이와 긴 한국어 문장에서도 본문 최하단과 Action/Navigation 모두 접근 가능해야 한다.

### 4.5 좁은 화면 대응

긴 한국어 문장, Asset 이름, Validation message는 Wrap 또는 Scroll 가능한 표현을 사용한다.

고정 픽셀 높이 때문에 주요 Action이 잘리거나, 긴 Path가 Widget 폭을 무한 확장하는 구조를 사용하지 않는다.

---

## 5. Step 설계 규약

### 5.1 한 Step은 하나의 작업 의도

한 Page에서 여러 독립 개념을 동시에 판단하게 하지 않는다.

좋은 예:

```text
발사 방식
→ 어떻게 발사되는지만 결정

피해 설정
→ 피해 특성만 결정

탄약 설정
→ 탄약 사용 방식만 결정
```

피해야 할 예:

```text
Projectile / Damage / Ammo
→ 서로 다른 세 개의 판단을 한 Page에 압축
```

### 5.2 Page 수보다 인지 부담을 우선한다

Step 수에 고정 상한을 두지 않는다.

한 Page의 기본 입력은 대략 3~6개의 주요 항목을 목표로 하지만 숫자를 기계적인 제한으로 사용하지 않는다. 서로 다른 개념을 이해해야 하거나 설명 없이 한눈에 판단하기 어려워지면 Page를 나눈다.

### 5.3 Common Step + Conditional Step

모든 대상이 같은 고정 Page 수를 통과하게 만들 필요가 없다.

```text
공통 Step
+ 선택한 Template / Capability에 필요한 조건부 Step
+ Final Review / Complete
```

방식을 기본으로 한다.

예를 들어 Launcher Capability가 없으면 Launcher Step은 나타나지 않아야 한다. Guidance를 사용하면 Guidance Step이 자연스럽게 추가된다.

### 5.4 Template은 길 안내이지 데이터 제약이 아니다

`캐논`, `기관포`, `유도미사일 런처` 같은 Template은 일반적인 Capability 조합과 기본값을 제안하기 위한 제작 보조 수단이다.

Runtime이 허용하는 조합을 Template 분류 때문에 불필요하게 금지하지 않는다. 특수 조합은 `직접 구성` 또는 Advanced Capability 선택으로 열 수 있다.

### 5.5 상위 Step 변경과 downstream invalidation

앞 Step의 선택이 뒤 Step 구성을 바꾸면 기존 downstream 입력을 조용히 유효한 것처럼 유지하지 않는다.

예:

```text
Projectile Weapon → HitScan으로 변경
→ Projectile Flight 전용 Step 제거
→ 기존 Flight 설정은 비활성/폐기 예정 상태로 명확화
→ Damage 등 계속 유효한 정보만 보존
```

의미가 달라진 상태는 Stale 또는 재검토 필요로 표시하고 사용자가 다시 확인하게 한다.

---

## 6. 사용자 정보 계층

Vehicle Builder에서 검증된 정보 우선순위를 공통 사용한다.

```text
사용자가 지금 해야 할 행동
→ 현재 상태
→ 문제가 있으면 해결 방법
→ 판단에 필요한 실제 값과 게임 의미
→ 세부 기술 진단
```

### 6.1 Level 0 — 즉시 행동 정보

기본 화면에서 항상 쉽게 찾을 수 있어야 한다.

```text
Step 이름
Step 목적
현재 상태
지금 할 일
핵심 경고 / 완료 여부
```

### 6.2 Level 1 — 제작 판단 정보

실제 제작 결정을 내리는 데 필요한 내용이다.

```text
Asset 또는 선택 대상
값과 단위
값의 의미
게임에서 어떤 영향을 주는지
현재 값이 Draft인지, 기존 자산인지, 최종 저장값인지
```

### 6.3 Level 2 — 진단 정보

정상 제작 진행에는 필요하지 않은 기술 정보를 별도 접힘 영역에 둔다.

```text
Hash
Fingerprint
RunId
Internal Object Path
Exact Binding
Backend raw message
세부 provenance
```

`기술 상세 보기 (진단용)` 같은 명확한 라벨을 사용한다.

---

## 7. 표시 언어와 용어

사용자가 읽는 기본 UI는 한국어를 우선한다.

Unreal 또는 도메인에서 실제로 사용하는 고유 작업 용어는 필요할 때 한국어와 영어를 함께 표시할 수 있다.

```text
장착 위치(Hardpoint)
소켓(Socket)
발사체(Projectile)
유도(Guidance)
```

내부 C++ 타입명, enum 이름, raw backend 상태 코드는 기본 UI에 그대로 노출하지 않는다.

버튼은 `삭제`, `적용`처럼 의미가 넓은 단어만 쓰지 말고 실제 영향 범위를 드러낸다.

```text
장착 위치만 삭제
최종 적용 및 저장
기존 자산 사용
새로 만들기
```

---

## 8. 신규 생성 / 기존 자산 재사용 패턴

하위 구성요소를 새로 만들 수도 있고 기존 자산을 재사용할 수도 있다면 같은 선택 패턴을 사용한다.

```text
이 항목을 어떻게 구성할까요?

● 새로 만들기
○ 기존 자산 사용
```

### 8.1 새로 만들기

현재 Step 안에서 정상 제작에 필요한 핵심 필드를 입력한다.

고급 필드는 접힌 Advanced 영역으로 둔다. 최종 저장은 해당 타입의 기존 Authoring/Durable authority가 있다면 반드시 재사용한다.

### 8.2 기존 자산 사용

Object Picker 또는 도메인에 맞는 선택 UI를 제공하고, 선택된 자산의 핵심 의미를 read-only 요약으로 보여준다.

단순히 Path만 표시해서 사용자가 내용을 기억해야 하게 만들지 않는다.

### 8.3 기존 자산의 수정

`기존 자산 사용`은 기본적으로 **선택 / 읽기 / 검증 / 재사용** 의미다.

기존 자산을 수정하는 기능까지 제공하려면 해당 Builder의 명시적인 Edit Contract가 있어야 한다. 아무 경고 없이 선택한 Product Asset을 수정·저장하지 않는다.

전문가용 세부 수정은 Native DataAsset Editor 또는 Advanced Workspace를 열 수 있지만 정상적인 신규 제작이 이 경로에 의존해서는 안 된다.

---

## 9. Naming / Identity 규약

사용자에게 내부 Identity 관리 부담을 기본 Workflow로 넘기지 않는다.

### 9.1 기본 입력

사용자는 가능한 한 사람이 이해하는 이름 또는 제작 대상 이름만 정한다.

Builder가 다음을 deterministic하게 파생한다.

```text
Asset 이름
Package 경로
Object 경로
내부 Stable ID가 필요한 경우 그 값
```

### 9.2 경로 직접 입력 최소화

ObjectPath, PackagePath, 내부 ID를 정상 제작 Page의 필수 입력으로 두지 않는다.

특수한 경로가 필요한 경우 `고급 Asset 경로 설정` 같은 접힌 Advanced override로 제공한다.

### 9.3 Silent sanitize 금지

입력 이름이 유효하지 않으면 조용히 다른 문자열로 바꿔 저장하지 않는다.

문제를 즉시 설명하고 사용자가 고치게 한다.

### 9.4 Collision은 fail-visible

이미 같은 이름의 자산이 있으면 덮어쓰거나 자동 숫자 suffix로 의미를 바꾸지 않는다.

기존 자산 재사용 또는 새 이름 선택을 명시적으로 요구한다.

### 9.5 생성 이후 Stable Identity

생성된 Asset의 stable identity를 downstream convenience 때문에 조용히 rename하지 않는다.

Rename/Migration이 필요하면 별도 명시적 계약으로 다룬다.

---

## 10. ViewModel / Backend / Persistent Authority

### 10.1 UI

Slate UI는 표시와 USER action 수집을 담당한다.

Persistent truth를 별도 복제해 두 번째 권한으로 만들지 않는다.

### 10.2 ViewModel

ViewModel은 transient draft, Step 상태, 현재 Page projection, Validation orchestration을 소유할 수 있다.

다음 원칙을 유지한다.

```text
Stable StepId 사용
fixed index에 의미 결합 금지
fresh authoritative read로 상태 재평가
stale transient approval 자동 부활 금지
persistent schema의 독립 복제 authority가 되지 않음
```

### 10.3 Persistent Authority

기존 타입에 검증된 Writer, Typed Provider, Apply Service, Durable Core가 있으면 Builder가 새 Writer를 만들지 않고 재사용한다.

UI를 통합한다는 이유로 저장 Authority까지 중복 구현하지 않는다.

```text
사용자 Workflow 통합
≠ Persistent Writer 통합
```

### 10.4 공용 Framework 과잉 일반화 금지

새 Builder 하나를 만들기 위해 범용 Step Engine, 범용 Writer, 범용 Schema Framework를 먼저 만들지 않는다.

같은 패턴이 둘 이상의 실제 Builder에서 반복되고 공용화 이득이 명확할 때 다음과 같은 좁은 UI component/helper부터 추출한다.

```text
Step Navigation
Common Header
Validation Summary
Asset Picker Row
Advanced Disclosure
공통 Page Shell
```

도메인 Business State와 Persistent Authority를 하나의 거대한 Generic Builder Framework로 합치지 않는다.

---

## 11. Validation 규약

### 11.1 Step-local Validation

가능한 오류는 해당 Step에서 바로 알려준다.

마지막 Review까지 진행한 뒤 초반 입력 오류를 대량으로 처음 발견하는 구조를 피한다.

### 11.2 Blocker와 Warning 구분

```text
Blocker
= 이 상태로 다음 단계 또는 최종 생성이 의미상 안전하지 않음

Warning
= 생성은 가능하지만 사용자가 알아야 할 비차단 상태
```

Blocker를 단순 경고색으로만 표시하고 Next를 허용하지 않는다.

### 11.3 오류 문장 순서

오류/복구 UI는 다음 순서를 기본으로 한다.

```text
무엇이 잘못됐는지
→ 지금 무엇을 해야 하는지
→ 필요할 때만 기술 진단
```

### 11.4 Final Validation

Final Review는 모든 Step의 current authoritative truth를 fresh하게 다시 확인한다.

최종 검토는 새로운 문제를 처음 발견하는 주된 장소가 아니라 **이미 단계별로 검증한 결과를 교차 확인하고 최종 결과를 보여주는 장소**여야 한다.

### 11.5 Runtime Capability Validation

사용자가 구성한 Capability 조합을 현재 Runtime이 실제 지원하는지 검증한다.

Runtime 미지원 조합은 다른 Builder로 보내지 않고 정확한 미지원 사유를 표시한다.

---

## 12. Preview / Apply / Save 규약

### 12.1 명시적 Mutation

사용자가 값을 입력하거나 Page를 이동했다는 이유만으로 Product Asset을 자동 변경하거나 저장하지 않는다.

가능하면 다음 흐름을 사용한다.

```text
Draft
→ Validation
→ Preview / Review
→ USER explicit Create / Apply
→ Durable readback
```

도메인상 중간 저장이 필요한 경우에도 해당 Action의 영향 범위를 UI에서 명확히 표시한다.

### 12.2 저장 범위를 명확히 표시

`저장` 버튼은 무엇을 저장하는지 분명해야 한다.

`Save All`에 의존하지 않는다. 현재 Action이 저장하는 exact Asset/Package 범위를 정하고 사용자에게 의미를 설명한다.

### 12.3 최종 Create / Apply 전에 전체 Preflight

여러 Asset을 만드는 Builder는 실제 첫 mutation 전에 가능한 범위에서 다음을 전부 검사한다.

```text
대상 이름/경로 collision
필요 클래스/타입
필수 참조
Capability compatibility
기존 선택 자산의 유효성
현재 저장 Authority readiness
```

### 12.4 Multi-Asset 생성은 Atomic이라고 가정하지 않는다

여러 Package 저장을 완전한 단일 transaction처럼 가장하지 않는다.

중간 Asset 저장 성공 뒤 다음 Asset 저장이 실패하면 이미 성공한 persistent write를 숨기거나 blind rollback/retry하지 않는다.

다음 정보를 정확히 보여준다.

```text
성공한 Asset
실패한 Asset
현재 재시도 가능한 단계
사용자가 해야 할 복구 행동
```

같은 Builder session에서 안전하게 exact retry할 수 있다면 이미 성공한 child를 재사용한다. 기존 Product Asset을 임의 삭제해서 원자성을 흉내 내지 않는다.

### 12.5 Durable 완료 판정

API 호출 성공만으로 완료 처리하지 않는다.

가능한 범위에서 persistent 존재, dirty 상태, exact reference/readback을 확인한 뒤 Complete로 전진한다.

---

## 13. Final Review / Complete 규약

### 13.1 Final Review

최종 화면은 내부 DataAsset graph보다 사용자가 만든 결과를 먼저 보여준다.

```text
무엇을 만들었는가
핵심 제작 특성
어떤 기능이 활성화됐는가
호환성 상태
남은 Warning
생성/적용 시 바뀌는 범위
```

내부 Asset 목록과 exact path는 필요하면 Level 1 또는 진단 정보에서 확인할 수 있게 한다.

### 13.2 명시적 완료 Action

실제 mutation이 발생한다면 `[무장 생성]`, `[최종 적용 및 저장]`처럼 결과가 분명한 버튼을 사용한다.

`완료`처럼 저장·적용 의미가 모호한 문구만 사용하지 않는다.

### 13.3 Completion Result

완료 뒤 downstream 업무가 소비할 결과가 있으면 stable Result/Handoff 계약으로 제공한다.

Builder 간 직접 mutable business state 공유를 기본값으로 사용하지 않는다.

### 13.4 Cross-Builder 이동

다음 Builder로 이동할 수는 있지만 기본 계약은 navigation-only다.

정확한 context handoff가 필요하면 별도 명시적 Handoff Contract를 설계하고 identity/freshness를 검증한다. 단순 편의를 위해 두 Builder가 같은 mutable ViewModel을 공유하지 않는다.

---

## 14. Advanced 편집 경계

Advanced Workspace 또는 Native Asset Editor는 다음 용도로 유지할 수 있다.

```text
고급 필드 직접 수정
기존 자산 유지보수
Legacy/compatibility 확인
External Drift 복구
전문가용 진단
Guided Workflow 밖의 특수 작업
```

반대로 다음은 Advanced로 떠넘기지 않는다.

```text
정상적인 신규 대상 제작에 항상 필요한 데이터
도메인의 일반적인 Capability
Builder가 정상 지원한다고 선언한 기능의 필수 설정
```

Guided Builder 기본 화면은 초보 사용자가 내부 데이터 구조를 몰라도 정상 결과를 만들 수 있어야 한다.

---

## 15. 사용자 상태와 재평가

### 15.1 Fresh truth 우선

Step 상태는 가능하면 current authoritative asset/service truth를 다시 읽어 평가한다.

오래된 cached UI 상태만으로 Complete/Blocked를 주장하지 않는다.

### 15.2 Refresh 의미 통일

`현재 상태 다시 확인`은 persistent mutation을 만들지 않는 재평가 Action을 기본 의미로 한다.

사용자 Action을 실행한 뒤 UI refresh만 실패했다면 실제 mutation 성공과 presentation refresh 실패를 구분한다.

### 15.3 Builder 종료 / 재진입

Transient draft를 자동으로 persistent truth처럼 복원하지 않는다.

길고 복잡한 제작 흐름에서 Draft persistence가 필요하다면 별도 명시적 Draft owner와 freshness 계약을 설계한다. 단순 UI cache를 durable 작업 기록으로 간주하지 않는다.

---

## 16. C++ / Blueprint 책임

CarFight Editor Builder의 Workflow shell, Step state, Validation orchestration, Asset writer 연계, durable guard는 기본적으로 C++ Editor Module에서 소유한다.

Blueprint는 Runtime 콘텐츠 표현, 개별 게임 오브젝트 구성, 시각적 콘텐츠 authoring처럼 Blueprint가 실제 장점을 가지는 영역에 사용한다.

Builder를 빠르게 만들기 위해 핵심 저장/검증 Authority를 Blueprint에 임시로 중복 구현하지 않는다.

Blueprint에 노출되는 설정은 한글 Tooltip을 제공해 현재 Step에서 왜 필요한 값인지 이해할 수 있게 한다.

---

## 17. 자동화 / USER 검증 기준

Builder Technical Validation은 최소한 다음 축을 검토한다.

```text
Stable StepId와 navigation
조건부 Step 삽입/제거와 downstream stale 처리
Page별 Blocker/Warning
신규 생성 / 기존 자산 재사용
Naming / collision / wrong type
no-auto-mutation / no-auto-save
Preview / explicit Apply
Multi-Asset partial failure / recovery
fresh persisted readback
작은 Editor 높이 / 긴 콘텐츠 / Action 접근성
테스트용 disposable asset residue exact0
Product Asset 비의도 mutation0
```

UX, 읽기 쉬움, 한 Page의 판단 부담, 버튼 의미, 제작 흐름의 자연스러움은 Technical Automation만으로 최종 PASS 처리하지 않는다. 대표 Workflow를 USER가 직접 확인하는 Acceptance를 별도로 둔다.

테스트는 가능한 한 `/Game/CarFight/Tests/...` 같은 disposable namespace를 사용하고 종료 뒤 residue exact0을 확인한다. Product Asset을 테스트 fixture처럼 변경·저장하지 않는다.

---

## 18. 신규 Builder 설계 시 고정 결정표

아래 항목은 새 Builder를 시작할 때 매번 처음부터 논쟁하지 않고 이 문서의 기본값을 사용한다.

| 결정 항목 | CarFight 기본값 |
| --- | --- |
| Builder 분리 기준 | DataAsset 타입이 아니라 사용자 업무 단위 |
| 하위 DataAsset 제작 | 현재 Builder 내부 Step/Panel에서 처리 |
| 다른 Builder 호출 | 업무가 실제로 바뀔 때만 허용 |
| Step 수 | 고정 상한 없음 |
| 한 Step 범위 | 하나의 작업 의도 / 주요 질문 |
| Step identity | Stable StepId, fixed index 의존 금지 |
| 특수 기능 | Capability 기반 Conditional Step |
| Template | 기본 Capability 조합 제안, hard class 제약 아님 |
| 본문 Scroll | Page당 주 Vertical Scroll exact1 기본 |
| Action / Navigation | 본문 Scroll 밖 고정 접근 |
| 사용자 정보 | Level 0 → Level 1 → Level 2 진단 |
| 기본 UI 언어 | 한국어 우선, 실제 기술 용어만 영어 병기 |
| 내부 ID/Path | 자동 파생, normal flow 직접 입력 금지 |
| Advanced Path override | 접힌 고급 설정에서만 |
| invalid name | silent sanitize 금지, fail-visible |
| collision | overwrite/자동 suffix 금지, explicit 해결 |
| 기존 자산 | 선택/읽기/검증/재사용이 기본 |
| 기존 자산 수정 | explicit Edit Contract가 있을 때만 |
| Persistent writer | 기존 검증된 authority 재사용 |
| Draft | transient 기본, 별도 persistence는 명시 계약 필요 |
| Mutation | 명시적 USER action 전 Product mutation0 |
| Save | exact scope 명시, Save All 의존 금지 |
| Multi-Asset 실패 | partial success를 숨기지 않고 exact recovery 제공 |
| Final Review | fresh 전체 검증 + 사용자 결과 중심 요약 |
| Advanced Workspace | 전문가 유지보수/복구용, normal creation 필수 경로 금지 |
| 공용 코드 추출 | 실제 2개 이상 consumer에서 반복 확인 뒤 최소 추출 |
| USER Acceptance | Technical PASS와 별도 |

---

## 19. 신규 Builder 착수 체크리스트

구현 전에 아래 질문에 답할 수 있어야 한다.

1. 사용자가 최종적으로 무엇 하나를 완성하려는가?
2. 그 업무에 포함되는 하위 데이터는 무엇이며 Builder 밖으로 나가야 할 정당한 이유가 있는가?
3. 현재 Runtime이 지원하는 해당 도메인 Capability 전체 목록은 무엇인가?
4. Common Step과 Conditional Step은 무엇인가?
5. 각 Step이 하나의 작업 의도만 가지는가?
6. 한 Page의 주요 입력과 Advanced 입력은 어떻게 나뉘는가?
7. 신규 생성과 기존 자산 재사용을 각각 어떻게 처리하는가?
8. 실제 persistent writer/validation authority는 무엇을 재사용하는가?
9. 사용자 이름에서 내부 Asset identity를 어떻게 deterministic하게 파생하는가?
10. Step-local Blocker와 Warning은 무엇인가?
11. 앞 Step 변경 시 어떤 downstream state가 Stale이 되는가?
12. 실제 Product mutation은 어느 explicit Action에서 처음 발생하는가?
13. 여러 Asset 생성 중 partial failure가 나면 어떤 exact recovery를 제공하는가?
14. Final Review가 보여줄 사용자 결과와 내부 진단 정보는 무엇인가?
15. 다음 Builder로 넘기는 경우 실제로 사용자 업무가 바뀌는가?
16. Advanced Workspace가 normal creation의 누락 기능을 대신하고 있지는 않은가?
17. Technical Automation과 USER Acceptance를 각각 무엇으로 판정할 것인가?

이 질문에 답하지 못한 상태에서는 UI 구현보다 설계 보강을 우선한다.

---

## 20. 규약 예외 처리

특정 Builder가 이 규약과 다른 동작이 반드시 필요하면 해당 Feature의 대표 Plan에 다음을 명시한다.

```text
어떤 기본 규약을 벗어나는가
왜 필요한가
사용자에게 어떤 차이가 생기는가
저장/검증/복구 위험은 무엇인가
어떻게 검증할 것인가
```

예외를 코드 구현만으로 암묵적으로 만들지 않는다.

---

## 21. 현재 적용 기준

### Vehicle Builder

현재 `VehicleBuilder.md`의 User-Facing Information Architecture, Common Page Shell, single body Scroll, fixed Action/Navigation, Stable StepId, Advanced Workspace 경계를 이 규약의 기준 사례로 사용한다.

이 문서 도입만을 이유로 현재 Vehicle Builder를 재작성하거나 기존 PASS를 재검증하지 않는다.

### Equipment Builder

기존 구현은 현재 Feature 계약을 보존한다. 이후 큰 UX 재설계 또는 새 제작 흐름 추가 시 본 규약을 기본 검토 기준으로 사용한다.

### Weapon Equipment Authoring Guide

현재 설계 중인 Weapon Guide는 이 규약을 첫 신규 적용 대상으로 본다.

특히 다음을 강제 설계 기준으로 사용한다.

```text
무장이라면 현재 Runtime이 지원하는 범위 전체를 하나의 Guide에서 제작
Damage / Ammo / Projectile / Launcher / Guidance 등을 별도 Builder 연쇄로 분리하지 않음
Page 수를 줄이기보다 Step-by-Step 인지 부담 최소화
무장 Template + Capability 기반 Conditional Step 사용
기존 persistent authoring authority는 재사용하되 UX는 하나의 Guide로 통합
```

기존에 작성된 고정 8-Step 초안이나 구현 prototype은 이 규약보다 우선하지 않는다. Step 구조를 재설계한 뒤 구현을 계속한다.

---

## 22. Changelog

### v1.0.0 - 2026-09-17

- Vehicle Builder의 검증된 User-Facing Information Architecture와 Common Page Shell을 CarFight Builder 공통 기준으로 승격했다.
- 사용자 업무 단위 Builder, Builder Chaining 금지, 도메인 완결성, Capability 기반 Conditional Step, 한 Page 하나의 작업 의도 원칙을 추가했다.
- 신규/기존 Asset 패턴, Naming/Identity, writer authority 재사용, explicit Preview/Apply/Save, Multi-Asset partial failure/recovery, Final Review/Handoff와 Advanced 경계를 공통 규약으로 정의했다.
- 신규 Builder 설계 때 반복 결정을 줄이기 위한 고정 결정표와 착수 체크리스트를 추가했다.
- 현재 Weapon Equipment Authoring Guide의 고정 8-Step prototype보다 본 규약에 따른 UX 재설계를 우선하도록 적용 기준을 명시했다.

---

## 23. Migration

- 이 문서 도입으로 기존 Vehicle Builder, Equipment Builder 또는 Product Asset을 자동 변경하지 않는다.
- 신규 Builder는 v1.0.0을 기본 설계 계약으로 사용한다.
- 기존 Builder는 큰 UX 재설계 또는 관련 Feature가 명시적으로 범위를 열 때 본 규약으로 수렴한다.
- Weapon Equipment Authoring Guide의 기존 미검증 prototype은 폐기/유지 여부를 별도 설계 재검수에서 판정하며, 이 문서 생성 자체가 Source 삭제·Build·Asset mutation을 승인하지 않는다.
