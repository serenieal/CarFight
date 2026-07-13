# AimReticle

- Version: 1.2.0
- Date: 2026-07-13
- Status: Current / P0 Reticle Implemented / Partial State Verification
- Scope: 로컬 차량 Pawn의 조준 상태와 FireFeedback ViewData를 WBP Reticle의 이미지, 텍스트, 색상으로 표시하는 현재 기준 문서

---

## 1. 문서 목적

이 문서는 현재 프로젝트에서 `AimReticle` UI 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 이 문서에서 `AimReticle`은 서버 상태를 표시하는 UI가 아니라, 로컬 플레이어가 현재 조준 상태와 로컬 발사 결과 피드백을 읽기 위한 UI 계층으로 본다.

---

## 2. 현재 기준

현재 기준은 아래와 같다.

```text
- 서버 권한 발사, 복제, 2클라 검증, 서버 대기 UI는 현재 구현 범위로 보지 않는다.
- 과거 멀티플레이 기준 용어가 남아 있는 경우에는 Legacy / Deferred 흔적으로 본다.
- 현재 문서 기준에서는 로컬 발사 검증과 로컬 피드백 용어를 우선 사용한다.
```

용어 기준:

```text
WaitingServer   -> FirePending
ServerRejected  -> FireRejected
서버 대기        -> 발사 처리 중
서버 거부        -> 발사 거부 또는 발사 조건 미충족
ServerAimState  -> FireValidationState
RepAimVisualState -> AimVisualState
```

`FirePending`은 서버 응답 대기 상태가 아니라, 로컬 Fire 입력 이후 짧게 표시할 수 있는 발사 처리 피드백 상태다.
`FireRejected`는 서버 거부 상태가 아니라, 로컬 발사 검증에서 조건을 만족하지 못했음을 표시하는 상태다.

---

## 3. 문서 범위

이 문서에서 말하는 `AimReticle` 기능은 아래 요소를 묶어서 본다.

- C++ 부모 위젯: `UCFAimReticleWidget`
- 대표 생성 주체: `ACFVehiclePawn`
- Aim 상태 공급 주체: `UCFVehicleAimComp`
- 현재 표시 상태 enum: `ECFVehicleReticleState`
- 현재 Aim 표시 데이터: `FCFVehicleLocalAimState`
- 현재 FireFeedback 표시 데이터: `FCFVehicleFireFeedbackViewData`
- 현재 Pawn 설정:
  - `AimReticleWidgetClass`
  - `AimReticleWidgetInstance`
  - `bShowAimReticle`
  - `AimReticleZOrder`

즉, 현재 기준 `AimReticle`은 화면에 조준점을 그리는 WBP 하나만을 뜻하지 않는다.
차량 Pawn이 로컬 플레이어 조건에서 위젯을 만들고, 위젯이 `VehicleAimComp`의 Local Aim 상태를 읽어 표시/숨김/텍스트를 갱신하는 UI 표시 흐름 전체로 본다.

---

## 4. 현재 실제 역할

현재 구현 기준 `AimReticle`의 핵심 역할은 **로컬 제어 차량 Pawn의 `VehicleAimComp`에서 Reticle 상태와 발사 가능 예측 값을 읽어, 화면 표시 텍스트와 위젯 가시성을 갱신하는 것**이다.

현재 `AimReticle`은 아래 일을 한다.

1. 차량 Pawn 참조를 받아 Reticle 표시 대상을 정한다.
2. Pawn의 `VehicleAimComp`에서 Local Aim 상태를 읽는다.
3. Reticle 상태를 캐시하고 선택적 TextBlock에 반영한다.
4. Reticle 상태에 따라 위젯 가시성을 갱신한다.
5. 매 프레임 자동 갱신을 지원한다.
6. 내부 enum 상태를 한국어 표시 텍스트로 변환한다.

즉 현재 `AimReticle`은 조준 가능 여부나 발사 판정을 직접 계산하지 않는다. `VehicleAimComp`의 기본 Reticle 상태와 `ACFVehiclePawn::BuildFireFeedbackViewData()`의 표시 데이터를 합쳐 화면에 반영하는 UI 계층이다.

### 4.1 현재 구현된 FireFeedback 통합

현재 구현 흐름은 아래와 같다.

```text
ACFVehiclePawn::BuildFireFeedbackViewData()
  -> LastFireResult / RejectReason 읽기
  -> 전체/남은 쿨다운 계산
  -> FireSuccess / Cooldown / FireRejected / NoWeapon / AimBlocked / OutOfArcWarning 표시 데이터 생성

UCFAimReticleWidget::RefreshFromPawn()
  -> VehicleAimComp의 BaseReticleState 읽기
  -> Pawn의 FireFeedbackViewData 읽기
  -> ResolveReticleStateFromFireFeedback()로 최종 Reticle 상태 결정
  -> RefreshTextBlocks()와 RefreshVisualStyle()로 텍스트/색상 적용
```

확인된 표시 흐름:

```text
Ready        -> 흰색
FireSuccess  -> 밝은 녹색, 짧은 성공 표시
Cooldown     -> 파란색, 남은 시간 표시
Cooldown 종료 -> FireFeedback State/Hint/Cooldown 텍스트 제거
```

`FirePending`은 타입과 표시 문구는 존재하지만, 현재 동기 발사 처리 구조에서는 활성화 경로가 없다.

---

## 5. 현재 표시 상태

현재 `ECFVehicleReticleState`는 아래 상태를 가진다.

| 상태 | 현재 한국어 표시 | 현재 의미 |
| --- | --- | --- |
| `Hidden` | `조준: 숨김` | Reticle을 표시하지 않음 |
| `Ready` | `조준: 준비` | 로컬 조준 상태가 정상이며 발사 가능 예측 상태 |
| `Blocked` | `조준: 가림` | 카메라/조준선 기준 목표가 가려짐 |
| `OutOfArc` | `조준: 각도 밖` | 현재 조준각이 기준 AimProfile 밖임 |
| `NoWeapon` | `조준: 무기 없음` | 사용할 수 있는 무기 데이터 또는 장착 상태 없음 |
| `Cooldown` | `조준: 재사용 대기` | 활성 무기가 쿨다운 중 |
| `Reloading` | `조준: 재장전` | 재장전 상태. 현재는 데이터 후보/후속 상태 |
| `FirePending` | `조준: 발사 처리 중` | 로컬 발사 입력 후 짧은 처리/피드백 상태 |
| `FireRejected` | `조준: 발사 거부` | 로컬 발사 조건 미충족 |

현재 `GetReticleHintDisplayText()`도 상태별 보조 설명을 한국어로 반환한다.

예:

```text
Ready        -> 목표 조준 가능
Blocked      -> 목표가 가려짐
OutOfArc     -> 무기 조준각 밖
NoWeapon     -> 사용 가능한 무기 없음
Cooldown     -> 무기 대기 중
Reloading    -> 재장전 중
FirePending  -> 발사 처리 대기
FireRejected -> 발사 조건 미충족
Hidden       -> Reticle 숨김
```

---

## 6. OutOfArc 해석 기준

`OutOfArc`는 현재 조준 방향이 기준 `AimProfile` 또는 무기 조준각 범위 밖임을 나타내는 표시/디버그 상태다.

현재 P0 싱글플레이 기준에서는 `OutOfArc` 자체를 단독 발사 차단 조건으로 사용하지 않는다.
실제 발사 성공 여부는 `WeaponFire`의 `ValidateFireCommand` 결과를 기준으로 판단한다.

따라서 UI 구현 시 아래 원칙을 따른다.

```text
OutOfArc = 조준각 경고/디버그 상태
OutOfArc != 무조건 발사 불가
```

후속 설계에서 `OutOfArc`를 실제 발사 거부 사유로 사용하기로 결정하면, `VehicleAim`, `WeaponFire`, `AimReticle`, `FireFeedback` 문서를 함께 갱신해야 한다.

---

## 7. 현재 동작 방식

### 7.1 Pawn 쪽 생성 조건

`ACFVehiclePawn::ShouldShowAimReticle()`이 현재 표시 가능 조건을 판단한다.

현재 조건:

```text
- bShowAimReticle == true
- GetNetMode() != NM_DedicatedServer
- IsLocallyControlled() == true
```

즉 현재 Reticle은 **로컬 제어 Pawn에서만 보이는 Viewport UI**다.
Dedicated Server 조건은 과거 멀티플레이 기능을 활성 범위로 둔다는 뜻이 아니라, UI를 서버 전용 실행 환경에서 만들지 않기 위한 안전 조건으로 본다.

### 7.2 Pawn 쪽 생성 흐름

`ACFVehiclePawn::CreateAimReticleWidget()`는 현재 아래 순서로 동작한다.

1. 이미 `AimReticleWidgetInstance`가 있으면 `RefreshAimReticleWidget()` 후 기존 인스턴스 반환
2. `ShouldShowAimReticle()`이 false면 생성하지 않음
3. `AimReticleWidgetClass`가 없으면 생성하지 않음
4. Controller를 `APlayerController`로 캐스팅
5. `CreateWidget<UCFAimReticleWidget>()` 호출
6. 생성된 위젯을 `AimReticleWidgetInstance`에 저장
7. `SetVehiclePawnRef(this)` 호출
8. `AddToViewport(AimReticleZOrder)` 호출
9. `RefreshAimReticleWidget()` 호출

현재 의미:

```text
- Reticle 생성은 Pawn이 주도한다.
- 위젯은 생성 직후 표시 대상 Pawn을 받는다.
- Viewport ZOrder는 AimReticleZOrder로 조정한다.
```

### 7.3 Pawn 쪽 제거 흐름

`ACFVehiclePawn::DestroyAimReticleWidget()`은 현재 아래를 수행한다.

```text
- 위젯 인스턴스가 없으면 아무것도 하지 않음
- 있으면 RemoveFromParent() 호출
- AimReticleWidgetInstance = nullptr
```

### 7.4 Pawn 쪽 갱신 흐름

`ACFVehiclePawn::RefreshAimReticleWidget()`은 현재 아래를 수행한다.

```text
- 인스턴스가 없으면 반환
- SetVehiclePawnRef(this) 호출
- ShouldShowAimReticle() 결과에 따라 위젯 가시성을 HitTestInvisible 또는 Collapsed로 설정
```

현재 의미:

```text
- Pawn 표시 조건이 바뀌면 Reticle도 함께 숨김 처리될 수 있다.
- 위젯 내부의 상태 기반 숨김과 Pawn의 표시 조건 기반 숨김이 모두 존재한다.
```

### 7.5 위젯 내부 갱신 흐름

`UCFAimReticleWidget` 내부 갱신 흐름은 아래와 같다.

```text
NativeConstruct
  -> RefreshFromPawn
  -> UpdateReticleVisibility

NativeTick
  -> bAutoRefreshEveryTick 확인
  -> RefreshFromPawn
  -> UpdateReticleVisibility

RefreshFromPawn
  -> VehiclePawnRef 확인
  -> VehicleAimComp 확인
  -> LocalAimState / BaseReticleState 읽기
  -> VehiclePawnRef.BuildFireFeedbackViewData() 읽기
  -> ResolveReticleStateFromFireFeedback()로 최종 상태 결정
  -> ApplyFireFeedbackViewData() / ApplyReticleState() 호출
  -> RefreshTextBlocks() / RefreshVisualStyle() 갱신
  -> Visibility 갱신
```

---

## 8. 현재 표시 조건 / 실행 조건

현재 `AimReticle`이 실제로 보이려면 아래 조건이 모두 중요하다.

```text
- ACFVehiclePawn::bShowAimReticle이 true여야 한다.
- Pawn이 Dedicated Server 환경이 아니어야 한다.
- Pawn이 로컬 제어 상태여야 한다.
- AimReticleWidgetClass가 설정되어 있어야 한다.
- Pawn의 Controller가 APlayerController여야 한다.
- VehicleAimComp가 있어야 의미 있는 상태를 읽을 수 있다.
- VehicleAimComp가 Hidden이 아닌 Reticle 상태를 반환해야 위젯이 보인다.
```

조건 해석:

```text
- Pawn 조건이 맞지 않으면 위젯이 생성되지 않거나 Collapsed된다.
- AimComp 조건이 맞지 않으면 위젯은 안전하게 Hidden 상태로 동작한다.
- WBP에 TextBlock이 없어도 위젯 자체는 동작할 수 있다.
```

---

## 9. 현재 자산 / 클래스 역할

### `UCFAimReticleWidget`

- 종류: C++ UserWidget 부모 클래스
- 현재 역할: Pawn/AimComp 상태와 FireFeedback ViewData 읽기, 최종 Reticle 상태 결정, Optional 이미지/TextBlock 갱신, 상태별 색상과 가시성 적용

### `ACFVehiclePawn`

- 종류: C++ Pawn
- 현재 역할: Reticle 위젯 생성/제거/갱신, Viewport 추가, `BuildFireFeedbackViewData()`를 통한 UI 표시 데이터 제공

### `UCFVehicleAimComp`

- 종류: C++ ActorComponent
- 현재 역할: Local Aim 상태와 Reticle 상태 공급

### `ECFVehicleReticleState`

- 종류: C++ enum
- 현재 역할: Reticle이 표시할 상태 종류 정의

### `FCFVehicleLocalAimState`

- 종류: C++ struct
- 현재 역할: 로컬 조준 목표, 조준 방향, 발사 가능 예측, 조준각 내부 여부, 조준 가림 여부 제공

### `WBP_AimReticle`

- 자산 경로: `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle`
- 부모 클래스: `UCFAimReticleWidget`
- 현재 역할: 실제 Reticle 이미지/텍스트 배치와 C++에서 계산한 상태별 색상 표시
- 주요 Optional 바인딩:
  - `Image_CenterDot`
  - `Image_LeftBracket`
  - `Image_RightBracket`
  - `Image_TopBracket`
  - `Image_BottomBracket`
  - `Text_FireFeedbackState`
  - `Text_FireFeedbackHint`
  - `Text_Cooldown`
  - `Text_OutOfArcWarning`
- 모든 바인딩은 누락 시 크래시가 발생하지 않는 Optional 구조다.

---

## 10. 현재 생성 및 연결 구조

현재 생성 및 연결 구조는 아래와 같다.

```text
ACFVehiclePawn
  -> ShouldShowAimReticle
  -> CreateAimReticleWidget
      -> CreateWidget<UCFAimReticleWidget>
      -> SetVehiclePawnRef(this)
      -> AddToViewport(AimReticleZOrder)
      -> RefreshAimReticleWidget
  -> BuildFireFeedbackViewData
      -> LastFireResult / RejectReason
      -> VehicleWeaponComp Cooldown
      -> VehicleAimComp LocalAimState
      -> FCFVehicleFireFeedbackViewData

UCFAimReticleWidget
  -> VehiclePawnRef.GetVehicleAimComp
  -> BaseReticleState / LocalAimState
  -> VehiclePawnRef.BuildFireFeedbackViewData
  -> CachedFireFeedbackViewData
  -> ResolveReticleStateFromFireFeedback
  -> CachedReticleState / bCachedCanFire
  -> RefreshTextBlocks / RefreshVisualStyle / Visibility 갱신
```

---

## 11. WeaponFire / FireFeedback 현재 연결

현재 `AimReticle`은 `VehicleAimComp`의 기본 상태와 Pawn이 생성한 `FCFVehicleFireFeedbackViewData`를 함께 읽는다.

실제 표시 매핑:

| 입력 결과 | FireFeedback 상태 | Reticle 처리 | 기본 색상 |
| --- | --- | --- | --- |
| `LastFireResult.bAccepted == true` | `FireSuccess` | 기본 Aim 상태 유지, 성공 색상 우선 | 밝은 녹색 |
| 남은 쿨다운 > 0 | `Cooldown` | `Cooldown`으로 덮어쓰기 | 파란색 |
| `RejectReason == NoWeapon` | `NoWeapon` | `NoWeapon`으로 덮어쓰기 | 회색 |
| `RejectReason == AimBlocked` | `AimBlocked` | `Blocked`로 덮어쓰기 | 주황색 |
| `RejectReason == OutOfWeaponArc` | `OutOfArcWarning` | 기본 상태 유지, 전용 보조 경고 | 노란색 경고 |
| 그 외 거부 사유 | `FireRejected` | `FireRejected`로 덮어쓰기 | 빨간색 |

현재 우선순위:

```text
1. 발사 성공 유지 시간 안이면 FireSuccess
2. 성공 표시 종료 후 남은 쿨다운이 있으면 Cooldown
3. 성공 결과이며 쿨다운도 끝났으면 None
4. 실패 결과는 RejectReason에 따라 짧은 실패 피드백 표시
```

OutOfArc 전용 경고 정책:

```text
- Text_OutOfArcWarning이 있고 현재 FeedbackState가 실제 OutOfArcWarning이면 전용 경고만 표시한다.
- 전용 위젯이 없으면 일반 FireFeedback State/Hint를 fallback으로 사용한다.
- 조준각 밖이어도 FireSuccess / Cooldown / FireRejected가 현재 피드백이면 해당 일반 문구를 숨기지 않는다.
- OutOfArcWarning은 주 Reticle 상태와 주 Reticle 색상을 강제로 노란색으로 바꾸지 않는다.
```

판정과 피드백의 책임 분리는 유지된다. `WeaponFire`가 결과를 기록하고, `AimReticle`은 결과를 화면에 표시할 뿐 판정 결과를 변경하지 않는다.

---

## 12. 현재 기능 책임

현재 `AimReticle`의 책임은 아래와 같다.

```text
- 로컬 제어 Pawn의 Reticle UI 생성 조건을 따른다.
- VehicleAimComp에서 기본 Reticle 상태와 Local Aim 정보를 읽는다.
- Pawn의 FireFeedback ViewData를 읽어 최종 Reticle 상태를 결정한다.
- 상태별 한국어 State/Hint/Cooldown/OutOfArc 경고 텍스트를 표시한다.
- Reticle 이미지 5개와 피드백 텍스트에 상태별 색상을 적용한다.
- FireSuccess / OutOfArcWarning처럼 주 상태를 덮어쓰지 않는 보조 표시 정책을 지킨다.
- Hidden 상태에서는 위젯을 숨긴다.
- Optional WBP 위젯이 없어도 크래시 없이 동작한다.
```

---

## 13. 현재 기준 비책임 항목

현재 구현상 `AimReticle`의 직접 책임이 아닌 것은 아래와 같다.

```text
- Aim 방향 계산
- Aim Trace 계산
- 조준각 판정
- 네트워크/서버 발사 검증
- 탄약/쿨다운/재장전 상태 계산
- 실제 크로스헤어 이미지/머티리얼 스타일 확정
- Common UI 레이어 관리
- 멀티플레이 원격 플레이어 Reticle 표시
- 전투 판정
```

현재 `AimReticle`은 계산기가 아니라, **이미 계산된 Local Aim 상태와 로컬 발사 결과 피드백을 화면에 읽기 쉽게 표시하는 UI 계층**이다.

---

## 14. 현재 문서 기준의 핵심 결론

현재 `AimReticle` 기능은,

**로컬 차량 Pawn의 VehicleAimComp 기본 상태와 FireFeedback ViewData를 결합해 Reticle 이미지, 한국어 상태 문구, 쿨다운 시간, 보조 경고, 상태별 색상으로 표시하는 현재 UI 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `AimReticle`은 현재 로컬 플레이어가 조준 상태와 로컬 발사 피드백을 즉시 읽을 수 있도록 Viewport UI로 표시해주는 기능이다.

---

## 15. 현재 문서에서 미확인인 항목

아래는 아직 이 문서에서 확정하지 않은 내용이다.

```text
- NoWeapon 실제 PIE 표시와 회색 상태
- AimBlocked 실제 PIE 표시와 주황 상태
- NoWeapon / AimBlocked 피드백 유지 시간 종료 후 텍스트 제거
- Reloading 상태를 실제 탄약/재장전 시스템과 연결할지 여부
- FirePending을 비동기/충전 무기 도입 시 활성화할지 여부
- Common UI PrimaryLayout 스택에 편입할지, 현재처럼 Viewport 직접 추가를 유지할지
- Reticle을 중앙 고정으로 둘지, AimHitLocation 기반 화면 좌표로 이동시킬지 장기 정책
```

---

## 16. 문서 갱신 조건

아래 변경이 생기면 이 문서를 함께 갱신한다.

```text
- UCFAimReticleWidget의 TextBlock 구성 변경
- GetReticleStateDisplayText() 표시 문구 변경
- GetReticleHintDisplayText() 표시 문구 변경
- UpdateReticleVisibility() 가시성 정책 변경
- ACFVehiclePawn::CreateAimReticleWidget() 생성 흐름 변경
- ShouldShowAimReticle() 표시 조건 변경
- Reticle이 Common UI 레이어로 이동할 때
- 실제 WBP 자산명과 구조가 확정될 때
- Reticle이 FireValidationState, LastFireResult, Cooldown 정보를 직접 읽도록 확장될 때
- FireFeedback 문서가 신설되어 Reticle과 피드백 책임이 분리될 때
```

---

## 17. 문서 버전 관리

- 현재 문서 버전: `1.2.0`
- 문서 상태: `Current / P0 Reticle Implemented / Partial State Verification`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준

- `Major`
  - Reticle 표시 구조가 Viewport 직접 추가에서 Common UI 레이어 구조로 바뀔 때
  - Reticle이 단순 상태 표시에서 전투 HUD 전체로 확장될 때
- `Minor`
  - 새로운 표시 상태, 텍스트, WBP 구조, 입력 조건이 추가될 때
  - 실제 WBP 자산 연결 상태가 확정될 때
  - Reticle이 로컬 발사 결과 피드백을 표시하도록 확장될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

---

## 18. Migration

### v1.1.0 -> v1.2.0

```text
- WBP 자산은 /Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle을 현재 기준으로 사용한다.
- 신규 Reticle 이미지와 FireFeedback TextBlock은 BindWidgetOptional 구조를 사용한다.
- Ready / FireSuccess / Cooldown / FireRejected / NoWeapon / AimBlocked / OutOfArcWarning 상태별 색상은 C++ 프로퍼티로 조정한다.
- FireSuccess는 쿨다운보다 먼저 짧게 표시하고, 이후 Cooldown으로 전환한다.
- Text_OutOfArcWarning은 실제 OutOfArcWarning 피드백에서만 일반 State/Hint를 대체한다.
- 기존 WBP 바인딩 이름을 유지하면 추가 데이터 마이그레이션은 필요하지 않다.
```

### v1.0.0 -> v1.1.0

```text
- WaitingServer 표현은 FirePending으로 교체한다.
- ServerRejected 표현은 FireRejected로 교체한다.
- 서버 대기 / 서버 거부 UI 표현은 현재 싱글플레이 기준 문서에서 사용하지 않는다.
- FirePending은 로컬 발사 처리 피드백 상태로 해석한다.
- FireRejected는 로컬 발사 조건 미충족 상태로 해석한다.
- OutOfArc는 현재 P0 싱글플레이 기준에서 단독 발사 차단 조건이 아니라 조준각 경고/디버그 상태로 해석한다.
```

---

## 19. Changelog

### v1.2.0 - 2026-07-13

```text
- ACFVehiclePawn::BuildFireFeedbackViewData()와 UCFAimReticleWidget의 실제 통합 흐름 반영
- WBP_AimReticle 자산 경로와 Optional 이미지/TextBlock 바인딩 구성 확정
- FireSuccess → Cooldown → 종료 우선순위와 텍스트 종료 동작 반영
- 상태별 색상 규칙과 RefreshVisualStyle() 역할 반영
- 전용 OutOfArc 경고의 중복 방지, fallback, 다른 FireFeedback 비가림 정책 반영
- FirePending 현재 미사용과 NoWeapon / AimBlocked 남은 PIE 검증 항목 명시
```

### v1.1.0 - 2026-07-09

```text
- 싱글플레이 로컬 전투 기준 문구 추가
- WaitingServer / ServerRejected 서버 상태 표현을 FirePending / FireRejected 기준으로 정리
- FirePending / FireRejected의 로컬 발사 피드백 의미 명시
- OutOfArc를 단독 발사 차단 조건이 아닌 조준각 경고/디버그 상태로 정리
- WeaponFire / FireFeedback 확장 기준과 RejectReason 표시 매핑 후보 추가
- 미확인 항목과 문서 갱신 조건을 weapon fire UI 작업 기준으로 정리
```

### v1.0.0 - 2026-06-02

```text
- AimReticle UI 문서 최초 작성
- UCFAimReticleWidget와 ACFVehiclePawn의 현재 Reticle 생성/표시 흐름 정리
- Reticle 상태별 한국어 표시 텍스트와 현재 미확정 UI 항목 기록
```

---

## 20. 마지막 확인 기준

- 확인 일시: `2026-07-13`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
    - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle` 자산 상세 덤프
  - 2026-07-13 사용자 PIE 확인 결과
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/Combat/WeaponFire.md`
