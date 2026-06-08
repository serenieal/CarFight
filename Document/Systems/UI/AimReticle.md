# AimReticle

## 문서 목적
이 문서는 현재 프로젝트에서 `AimReticle` UI 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `AimReticle` 기능은 아래 요소를 묶어서 본다.

- C++ 부모 위젯: `UCFAimReticleWidget`
- 대표 생성 주체: `ACFVehiclePawn`
- Aim 상태 공급 주체: `UCFVehicleAimComp`
- 현재 표시 상태 enum: `ECFVehicleReticleState`
- 현재 표시 데이터: `FCFVehicleLocalAimState`
- 현재 Pawn 설정:
  - `AimReticleWidgetClass`
  - `AimReticleWidgetInstance`
  - `bShowAimReticle`
  - `AimReticleZOrder`

즉, 현재 기준 `AimReticle`은 **화면에 조준점을 그리는 WBP 하나만을 뜻하는 것이 아니라, 차량 Pawn이 로컬 플레이어 조건에서 위젯을 만들고, 위젯이 VehicleAimComp의 Local Aim 상태를 읽어 표시/숨김/텍스트를 갱신하는 UI 표시 흐름 전체**로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `AimReticle`의 핵심 역할은 **로컬 제어 차량 Pawn의 VehicleAimComp에서 Reticle 상태와 발사 가능 예측 값을 읽어, 화면 표시 텍스트와 위젯 가시성을 갱신하는 것**이다.

현재 `AimReticle`은 아래 일을 한다.

### 1. 차량 Pawn 참조를 받아 Reticle 표시 대상을 정한다
`UCFAimReticleWidget::SetVehiclePawnRef()`는 Reticle이 읽을 차량 Pawn 참조를 설정한다.

현재 동작:
- `VehiclePawnRef = InVehiclePawnRef`
- 즉시 `RefreshFromPawn()` 호출
- 즉시 `UpdateReticleVisibility()` 호출

현재 의미:
- Reticle 위젯은 월드 전체를 직접 검색하지 않는다.
- 표시 대상 Pawn을 외부에서 명시적으로 넣어줘야 한다.
- 현재 생성 주체는 `ACFVehiclePawn::CreateAimReticleWidget()`이다.

### 2. Pawn의 VehicleAimComp에서 Local Aim 상태를 읽는다
`UCFAimReticleWidget::RefreshFromPawn()`은 현재 Pawn과 AimComp를 안전하게 확인한 뒤 Local Aim 상태를 읽는다.

현재 처리 흐름:
1. `VehiclePawnRef`가 유효하지 않으면 fallback 상태 `Hidden` 적용
2. Pawn은 있으나 `VehicleAimComp`가 없으면 fallback 상태 `Hidden` 적용
3. `VehicleAimComp->GetLocalAimState()` 호출
4. `LocalAimState.bLocalCanFire`를 `bCachedCanFire`에 저장
5. `VehicleAimComp->GetReticleState()`로 현재 Reticle 상태를 읽음
6. `ApplyReticleState()` 호출
7. `UpdateReticleVisibility()` 호출

즉 현재 `AimReticle`은 **조준 가능 여부를 직접 계산하지 않고, VehicleAimComp가 이미 계산해 둔 Local Aim 상태를 화면 표시용으로 변환하는 얇은 UI 계층**이다.

### 3. Reticle 상태를 캐시하고 TextBlock에 반영한다
`ApplyReticleState()`는 전달받은 `ECFVehicleReticleState`를 `CachedReticleState`에 저장하고 `RefreshTextBlocks()`를 호출한다.

현재 선택적 바인딩 TextBlock:
- `Text_ReticleState`
- `Text_CanFire`
- `Text_ReticleHint`

현재 의미:
- WBP 자식에 해당 TextBlock이 있으면 텍스트를 갱신한다.
- `BindWidgetOptional`이므로 WBP에서 일부 TextBlock이 없어도 C++ 부모 위젯은 동작한다.
- 실제 Reticle 그래픽 배치와 스타일은 WBP 자식이 담당할 수 있다.

### 4. Reticle 상태에 따라 위젯 가시성을 갱신한다
`UpdateReticleVisibility()`는 현재 캐시된 Reticle 상태를 보고 위젯 전체 가시성을 설정한다.

현재 규칙:
- `CachedReticleState != Hidden`이면 `HitTestInvisible`
- `CachedReticleState == Hidden`이면 `Collapsed`

현재 의미:
- Reticle은 마우스/클릭을 막는 인터랙티브 위젯이 아니다.
- 표시 중에도 `HitTestInvisible`로 둔다.
- 숨김 상태에서는 레이아웃에서도 사라지도록 `Collapsed`를 사용한다.

### 5. 매 프레임 자동 갱신을 지원한다
`UCFAimReticleWidget::NativeTick()`은 `bAutoRefreshEveryTick`이 true일 때 매 프레임 `RefreshFromPawn()`과 `UpdateReticleVisibility()`를 호출한다.

현재 기본값:
- `bAutoRefreshEveryTick = true`

현재 의미:
- Aim 상태가 매 프레임 바뀌어도 위젯이 직접 따라갈 수 있다.
- 후속 최적화 단계에서 이벤트 기반 갱신으로 바뀔 수 있지만, 현재는 Tick 기반 자동 갱신 구조다.

### 6. 상태값을 한국어 표시 텍스트로 변환한다
현재 `GetReticleStateDisplayText()`는 내부 enum 상태를 화면 표시용 한국어 텍스트로 변환한다.

현재 매핑:
- `Ready` → `조준: 준비`
- `Blocked` → `조준: 가림`
- `OutOfArc` → `조준: 각도 밖`
- `NoWeapon` → `조준: 무기 없음`
- `Cooldown` → `조준: 재사용 대기`
- `Reloading` → `조준: 재장전`
- `WaitingServer` → `조준: 서버 대기`
- `ServerRejected` → `조준: 서버 거부`
- `Hidden` → `조준: 숨김`

현재 `GetReticleHintDisplayText()`도 상태별 보조 설명을 한국어로 반환한다.

예:
- `Ready` → `목표 조준 가능`
- `Blocked` → `목표가 가려짐`
- `OutOfArc` → `무기 조준각 밖`
- `ServerRejected` → `서버에서 발사 거부`

즉 현재 `AimReticle`은 `DisplayTextPolicy`에 맞게 **내부 enum은 영문 유지, 화면 표시 텍스트는 한국어로 변환**하는 UI 계층이다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `AimReticle`은 아래 역할을 가진다.

1. **Local Aim 표시 UI**
   - `VehicleAimComp`의 Local Aim 상태를 읽는다.
   - Reticle 상태와 발사 가능 여부를 화면에 반영한다.

2. **표시 변환 계층**
   - 내부 enum 상태를 한국어 표시 텍스트로 변환한다.
   - 표시 텍스트와 보조 설명을 분리한다.

3. **안전한 WBP 부모 클래스**
   - 실제 배치/스타일은 WBP 자식이 맡을 수 있다.
   - TextBlock은 `BindWidgetOptional`이므로 없어도 동작한다.

4. **로컬 Pawn 전용 Viewport UI**
   - Pawn이 로컬 제어 상태일 때만 생성/표시된다.
   - Dedicated Server에서는 Viewport UI를 생성하지 않는다.

따라서 현재 `AimReticle`은 단순한 십자선 이미지가 아니라,
**VehicleAimComp가 계산한 로컬 조준 상태를 플레이어가 읽을 수 있는 화면 표시 상태로 바꾸는 Reticle UI 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `AimReticle`은 아래 방식으로 동작한다.

### 1. Pawn 쪽 생성 조건
`ACFVehiclePawn::ShouldShowAimReticle()`이 현재 표시 가능 조건을 판단한다.

현재 조건:
- `bShowAimReticle == true`
- `GetNetMode() != NM_DedicatedServer`
- `IsLocallyControlled() == true`

즉 현재 Reticle은 **로컬 제어 Pawn에서만 보이는 Viewport UI**다.

### 2. Pawn 쪽 생성 흐름
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
- Reticle 생성은 Pawn이 주도한다.
- 위젯은 생성 직후 표시 대상 Pawn을 받는다.
- Viewport ZOrder는 `AimReticleZOrder`로 조정한다.

### 3. Pawn 쪽 제거 흐름
`ACFVehiclePawn::DestroyAimReticleWidget()`은 현재 아래를 수행한다.

- 위젯 인스턴스가 없으면 아무것도 하지 않음
- 있으면 `RemoveFromParent()` 호출
- `AimReticleWidgetInstance = nullptr`

### 4. Pawn 쪽 갱신 흐름
`ACFVehiclePawn::RefreshAimReticleWidget()`은 현재 아래를 수행한다.

- 인스턴스가 없으면 반환
- `SetVehiclePawnRef(this)` 호출
- `ShouldShowAimReticle()` 결과에 따라 위젯 가시성을 `HitTestInvisible` 또는 `Collapsed`로 설정

현재 의미:
- Pawn 표시 조건이 바뀌면 Reticle도 함께 숨김 처리될 수 있다.
- 위젯 내부의 상태 기반 숨김과 Pawn의 표시 조건 기반 숨김이 모두 존재한다.

### 5. 위젯 내부 갱신 흐름
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
  -> LocalAimState 읽기
  -> ReticleState 읽기
  -> TextBlock 갱신
  -> Visibility 갱신
```

## 현재 표시 조건 / 실행 조건
현재 `AimReticle`이 실제로 보이려면 아래 조건이 모두 중요하다.

- `ACFVehiclePawn::bShowAimReticle`이 true여야 한다.
- Pawn이 Dedicated Server 환경이 아니어야 한다.
- Pawn이 로컬 제어 상태여야 한다.
- `AimReticleWidgetClass`가 설정되어 있어야 한다.
- Pawn의 Controller가 `APlayerController`여야 한다.
- `VehicleAimComp`가 있어야 의미 있는 상태를 읽을 수 있다.
- `VehicleAimComp`가 `Hidden`이 아닌 Reticle 상태를 반환해야 위젯이 보인다.

조건 해석:
- Pawn 조건이 맞지 않으면 위젯이 생성되지 않거나 `Collapsed`된다.
- AimComp 조건이 맞지 않으면 위젯은 안전하게 `Hidden` 상태로 동작한다.
- WBP에 TextBlock이 없어도 위젯 자체는 동작할 수 있다.

## 현재 자산 / 클래스 역할

### `UCFAimReticleWidget`
- 종류: C++ UserWidget 부모 클래스
- 현재 역할: Pawn/AimComp 상태 읽기, Reticle 상태 캐시, 선택적 텍스트 갱신, 가시성 갱신

### `ACFVehiclePawn`
- 종류: C++ Pawn
- 현재 역할: Reticle 위젯 클래스 보유, 로컬 조건 판정, 위젯 생성/제거/갱신, Viewport 추가

### `UCFVehicleAimComp`
- 종류: C++ ActorComponent
- 현재 역할: Local Aim 상태와 Reticle 상태 공급

### `ECFVehicleReticleState`
- 종류: C++ enum
- 현재 역할: Reticle이 표시할 상태 종류 정의

### `FCFVehicleLocalAimState`
- 종류: C++ struct
- 현재 역할: 로컬 조준 목표, 조준 방향, 발사 가능 예측, 조준각 내부 여부, 조준 가림 여부 제공

### WBP 자식 위젯
- 종류: Widget Blueprint
- 현재 역할: 실제 Reticle 배치/스타일 담당
- 현재 문서 기준 정확한 WBP 자산명과 배치 구성은 미확인이다.

## 현재 생성 및 연결 구조
현재 생성 및 연결 구조는 아래와 같다.

```text
ACFVehiclePawn
  -> ShouldShowAimReticle
  -> CreateAimReticleWidget
      -> CreateWidget<UCFAimReticleWidget>
      -> SetVehiclePawnRef(this)
      -> AddToViewport(AimReticleZOrder)
      -> RefreshAimReticleWidget

UCFAimReticleWidget
  -> VehiclePawnRef
  -> VehiclePawnRef.GetVehicleAimComp
  -> VehicleAimComp.GetLocalAimState
  -> VehicleAimComp.GetReticleState
  -> CachedReticleState / bCachedCanFire
  -> TextBlock / Visibility 갱신
```

## 현재 기능 책임
현재 `AimReticle`의 책임은 아래와 같다.

- 로컬 제어 Pawn의 Reticle UI 생성 조건을 따른다.
- `VehicleAimComp`에서 현재 Reticle 상태를 읽는다.
- Local Aim의 발사 가능 예측 값을 읽는다.
- 내부 enum 상태를 한국어 표시 텍스트로 변환한다.
- 상태별 보조 설명 텍스트를 제공한다.
- `Hidden` 상태에서는 위젯을 숨긴다.
- WBP 자식에서 TextBlock이 없어도 크래시 없이 동작한다.

## 현재 기준 비책임 항목
현재 구현상 `AimReticle`의 직접 책임이 아닌 것은 아래와 같다.

- Aim 방향 계산
- Aim Trace 계산
- 조준각 판정
- 서버 발사 검증
- 탄약/쿨다운/재장전 상태 계산
- 실제 크로스헤어 이미지/머티리얼 스타일 확정
- Common UI 레이어 관리
- 멀티플레이 원격 플레이어 Reticle 표시
- 전투 판정

현재 `AimReticle`은 계산기가 아니라, **이미 계산된 Local Aim 상태를 화면에 읽기 쉽게 표시하는 UI 계층**이다.

## 현재 문서 기준의 핵심 결론
현재 `AimReticle` 기능은,

**로컬 차량 Pawn의 VehicleAimComp에서 Reticle 상태를 읽어 화면 표시 텍스트와 위젯 가시성으로 변환하는 현재 상태 UI 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `AimReticle`은 현재 로컬 플레이어가 조준 상태를 즉시 읽을 수 있도록 VehicleAimComp의 Local Aim 상태를 Viewport UI로 표시해주는 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- 실제 WBP 자식 위젯 자산명
- 실제 Reticle 이미지/머티리얼/애니메이션 구성
- `AimReticleWidgetClass`가 어떤 BP 자산으로 설정되어 있는지
- Common UI PrimaryLayout 스택에 편입할지, 현재처럼 Viewport 직접 추가를 유지할지
- `WaitingServer`, `ServerRejected`, `Cooldown`, `Reloading`, `NoWeapon` 상태가 실제 UI에서 전환되는 최종 경로
- Reticle을 중앙 고정으로 둘지, AimHitLocation 기반 화면 좌표로 이동시킬지 장기 정책

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `UCFAimReticleWidget`의 TextBlock 구성 변경
- `GetReticleStateDisplayText()` 표시 문구 변경
- `GetReticleHintDisplayText()` 표시 문구 변경
- `UpdateReticleVisibility()` 가시성 정책 변경
- `ACFVehiclePawn::CreateAimReticleWidget()` 생성 흐름 변경
- `ShouldShowAimReticle()` 표시 조건 변경
- Reticle이 Common UI 레이어로 이동할 때
- 실제 WBP 자산명과 구조가 확정될 때

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
- 문서 상태: `Initial`
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
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.0 - 2026-06-02
- `AimReticle` UI 문서 최초 작성
- `UCFAimReticleWidget`와 `ACFVehiclePawn`의 현재 Reticle 생성/표시 흐름 정리
- Reticle 상태별 한국어 표시 텍스트와 현재 미확정 UI 항목 기록

## 마지막 확인 기준
- 확인 일시: `2026-06-02`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
