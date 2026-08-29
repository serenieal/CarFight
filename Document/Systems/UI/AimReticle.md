# AimReticle

- Version: 1.10.0
- Date: 2026-08-22
- Status: Current / P0 Aim·FireFeedback Verified / UI-P0-04 USER PASS / Post-Closure Refresh Optimization Accepted
- Scope: 로컬 Aim/FireFeedback 표시 의미와 UCFUISubsystem 소유 AimReticle 수명·Rebind의 현재 구현 계약

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
- 대표 생성·수명 주체: `UCFUISubsystem`
- Current Widget Class 설정: `UCFUISubsystem::DefaultAimReticleWidgetClass`
- 현재 Layer: `ECFUILayer::HUD`, Local ZOrder 10
- 표시 조건 공급: 현재 Possessed `ACFVehiclePawn::ShouldShowAimReticle()`
- Aim 상태 공급 주체: `UCFVehicleAimComp`
- 현재 표시 상태 enum: `ECFVehicleReticleState`
- 현재 Aim 표시 데이터: `FCFVehicleLocalAimState`
- 현재 FireFeedback 표시 데이터: `FCFVehicleFireFeedbackViewData`
- Legacy Pawn 직렬화 호환 Property/API는 남아 있지만 자동 생성 수명 owner가 아니다.

즉, 현재 기준 `AimReticle`은 화면에 조준점을 그리는 WBP 하나만을 뜻하지 않는다.
LocalPlayer `UCFUISubsystem`이 기존 `WBP_AimReticle`을 HUD Layer에 단일 생성하고 Current Vehicle Pawn으로 Rebind하며, 위젯이 약한 Pawn 참조를 통해 `VehicleAimComp`의 Local Aim 상태와 Pawn FireFeedback 표시 데이터를 읽어 갱신하는 UI 흐름 전체로 본다.

---

## 4. 현재 실제 역할

현재 구현 기준 `AimReticle`의 핵심 역할은 **로컬 제어 차량 Pawn의 `VehicleAimComp`에서 Reticle 상태와 발사 가능 예측 값을 읽어, 화면 표시 텍스트와 위젯 가시성을 갱신하는 것**이다.

현재 `AimReticle`은 아래 일을 한다.

1. 차량 Pawn 참조를 받아 Reticle 표시 대상을 정한다.
2. Pawn의 `VehicleAimComp`에서 Local Aim 상태를 읽는다.
3. Reticle 상태를 캐시하고 선택적 TextBlock에 반영한다.
4. Pawn 표시 조건은 Root Visibility로 따르고, Reticle 상태 기반 Hidden은 RenderOpacity로 표현한다.
5. 매 프레임 자동 갱신을 지원하되 한 Tick에서 Pawn/Aim/FireFeedback cache를 먼저 확정한 뒤 Text·WeaponReticle·Visibility를 각각 한 번만 반영한다.
6. 내부 enum 상태를 한국어 표시 텍스트로 변환한다.

즉 현재 `AimReticle`은 조준 가능 여부나 발사 판정을 직접 계산하지 않는다. `VehicleAimComp`의 기본 Reticle 상태와 `ACFVehiclePawn::BuildFireFeedbackViewData()`의 표시 데이터를 합쳐 화면에 반영하는 UI 계층이다.

### 4.1 현재 구현된 FireFeedback 통합

현재 구현 흐름은 아래와 같다.

```text
ACFVehiclePawn::BuildFireFeedbackViewData()
  -> LastFireResult / RejectReason 읽기
  -> 전체/남은 쿨다운 계산
  -> FireSuccess / Cooldown / FireRejected / NoWeapon / AimBlocked / OutOfArcWarning / TurretAligning 표시 데이터 생성

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

### 4.2 현재 Reticle 의미와 조준 정렬 상태

`Image_CenterDot`은 사용자가 화면 내 원하는 위치를 지정하는 조준 레티클이다.
2026-07-21 사용자 확인에서 현재 `Image_CenterDot` 작동에는 문제가 없었다. 이후 터렛 레티클 코드 정렬에서는 이 경로를 변경하지 않고 회귀 보호한다.
Core 구현에서는 조준 레티클 목표점, Muzzle 기준 요구 발사 방향, HitScan / Projectile 발사 방향이 `FCFVehicleWeaponAimSolution`을 공유하도록 연결됐다.

현재 구현된 방향 기준:

```text
Reticle 목표점 = Camera Aim Trace의 AimHitLocation
요구 발사 방향 = Muzzle 위치 → Reticle 목표점
실제 발사 방향 = Weapon Aim Solution의 AimDirection
```

2026-07-14 사용자 PIE에서는 정렬 완료 후 조준 목표와 실제 탄착이 일치하는 것으로 발사 방향 정합성을 확인했다. 이는 `Image_WeaponReticle`의 착탄점 의미를 정의한 검증이 아니다. 정렬 중에는 정책과 관계없이 `TurretAligning` amber를 유지하고, `bAllowFireWhileAligning=true`는 현재 Muzzle 방향 발사, `false`는 정렬 완료 전 발사 거부로 동작한다. 총구 장애물은 두 정책 모두 `MuzzleBlocked` 표시와 발사 차단으로 확인됐다.

따라서 현재 Reticle은 다음 의미로 해석한다.

```text
현재 계약과 구현:
- Image_CenterDot은 사용자가 화면 위치를 지정하는 조준 레티클
- Image_CenterDot 현재 작동 사용자 확인 PASS / 후속 변경 대상 아님
- 정렬 중은 ECFVehicleReticleState::TurretAligning 주 상태와 기존 FireFeedback DisplayKey 보조 표시로 표현
- 총구 막힘은 MuzzleBlocked -> AimBlocked / Blocked 표시로 표현
- 정렬 완료 후 조준 목표와 실제 발사 방향 정합성 P0 PIE PASS

구현 구조:
- Reticle이 지시하는 월드 위치를 DesiredAimTargetLocation으로 사용
- 터렛과 총구가 같은 목표점을 추적
- 정렬 중과 정렬 완료를 UI 상태로 구분
- 정렬 완료 후 실제 발사
```

상세 완료 설계·검증 evidence:

```text
Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md
```

현재 `TurretAligning` 상태는 `OutOfArc`와 분리한다.

```text
OutOfArc       = 기계적 조준 범위 밖
TurretAligning = 범위 안이지만 총구가 아직 목표에 정렬되지 않음
```

구현상 `ECFVehicleReticleState::TurretAligning` enum을 추가했다.
Core가 정렬 대기 중 LocalReticleState를 TurretAligning으로 반환하고, 기존 `FCFVehicleFireFeedbackViewData.FeedbackDisplayKey = TurretAligning` 보조 표시도 함께 유지한다.

### 4.3 2026-07-21 확정 Reticle 의미 계약

```text
Image_CenterDot
= 조준 레티클(Aim Reticle)
= 사용자가 화면 내 원하는 위치로 지정하는 조준 UI
= Camera Aim Trace를 통해 단일 3D 조준점을 선택

터렛
= 조준 레티클이 선택한 3D 지점을 추적

Image_WeaponReticle
= 터렛 레티클(Turret Reticle)
= 터렛이 현재 조준하는 3D 지점을 사용자 2D 화면에 투영
= CurrentMuzzleDirection 기반
= HitScan/Projectile, 중력 적용 여부와 무관
= 예상 또는 실제 착탄 위치 의미 없음

투사체 착탄 위치
= Reticle UI와 분리
= 터렛 레티클 완료 후 별도 논의하는 월드 공간 3D 표시
```

현재 구현 상태:

```text
- Image_WeaponReticle Optional 바인딩과 World To Screen 표시는 구현돼 있다.
- C++은 bHasValidTurretReticlePoint와 TurretReticleWorldLocation을 사용한다.
- TurretReticleWorldLocation은 CurrentMuzzleDirection을 사용자 조준점과 같은 거리까지 연장한 월드 지점이다.
- Image_WeaponReticle은 ECFWeaponReticleMode와 WeaponPreviewWorldLocation을 소비하지 않는다.
- 기존 DirectImpact/LaunchDirection Preview는 Legacy Debug로만 보존한다.
- Tools\BuildEditor.bat PASS 후 2026-07-21 사용자 PIE에서 계획한 터렛 레티클 동작을 확인했다.
- 과거 정렬 완료 후 탄착 일치 검증은 발사 방향 정합성 증거이며 Image_WeaponReticle의 의미 정의가 아니다.
```

완료 당시 세부 구현 기준은 `Document/Plan/Archive/ReticleAimDirection/ImplementationDesign.md` v0.3.2에 보존한다. 현재 구현 판단은 이 Systems 문서와 실제 Source를 우선한다.

---

## 5. 현재 표시 상태

현재 `ECFVehicleReticleState`는 아래 상태를 가진다.

| 상태 | 현재 한국어 표시 | 현재 의미 |
| --- | --- | --- |
| `Hidden` | `조준: 숨김` | Reticle을 표시하지 않음 |
| `Ready` | `조준: 준비` | 로컬 조준 상태가 정상이며 발사 가능 예측 상태 |
| `Blocked` | `조준: 가림` | 카메라/조준선 기준 목표가 가려짐 |
| `OutOfArc` | `조준: 각도 밖` | 현재 조준각이 기준 AimProfile 밖임 |
| `TurretAligning` | `조준: 터렛 정렬 중` | 총구가 조준점을 추적 중임 |
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
TurretAligning -> 총구가 조준점을 추적 중
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

### 7.1 현재 표시 조건

`ACFVehiclePawn::ShouldShowAimReticle()`이 현재 Vehicle Pawn의 Reticle 표시 가능 조건을 판단한다.

현재 조건:

```text
- bShowAimReticle == true
- GetNetMode() != NM_DedicatedServer
- IsLocallyControlled() == true
```

Reticle Widget의 생성 수명은 Pawn이 아니라 `UCFUISubsystem`이 소유한다. UISubsystem은 현재 Possessed Pawn을 `ACFVehiclePawn`으로 해석한 뒤 `ShouldShowAimReticle()` 결과에 따라 Root Visibility를 `HitTestInvisible` 또는 `Collapsed`로 적용한다.

### 7.2 UISubsystem 생성 흐름

`UCFUISubsystem::CreateAimReticleWidget()`의 현재 흐름은 아래와 같다.

1. 기존 AimReticle이 이미 HUD Layer의 올바른 부모에 있으면 새로 만들지 않고 Current Pawn Rebind만 수행
2. 잘못된 기존 인스턴스가 있으면 먼저 정리
3. 현재 LocalPlayer의 `ACFPlayerController`, UI Root와 `DefaultAimReticleWidgetClass` 해석 결과 확인
4. Config의 기존 `WBP_AimReticle` Class로 `CreateWidget<UCFAimReticleWidget>()`
5. `UCFUIRootWidget`의 `ECFUILayer::HUD`에 Local ZOrder 10으로 추가
6. `RebindAimReticleToCurrentPawn()` 수행

현재 의미:

```text
- Reticle 자동 생성·단일 수명은 LocalPlayer UCFUISubsystem이 소유한다.
- Pawn direct AddToViewport 자동 생성 경로는 Current owner가 아니다.
- 기존 Pawn의 AimReticleWidgetClass/API는 저장 Asset 호환 경계로 남아 있을 수 있다.
```

### 7.3 UISubsystem 제거 흐름

`UCFUISubsystem::DestroyAimReticleWidget()`은 현재 아래를 수행한다.

```text
- 인스턴스가 없으면 반환
- SetVehiclePawnRef(nullptr)로 이전 Pawn 참조 먼저 제거
- RemoveFromParent()
- UISubsystem 소유 AimReticleWidget 참조를 nullptr로 정리
```

### 7.4 Possession Rebind 흐름

`UCFUISubsystem::NotifyPossessedPawnChanged()`는 Current Pawn을 갱신한 뒤 `RebindAimReticleToCurrentPawn()`을 호출한다.

```text
CurrentPawn 변경
  -> Cast<ACFVehiclePawn>
  -> AimReticleWidget.SetVehiclePawnRef(CurrentVehiclePawn)
  -> CurrentVehiclePawn && ShouldShowAimReticle()
      ? HitTestInvisible
      : Collapsed
```

`UCFAimReticleWidget::VehiclePawnRef`는 v1.9.0부터 약한 참조이므로 UISubsystem보다 짧은 Pawn lifetime을 소유하지 않는다.

현재 의미:

```text
- Pawn 표시 조건이 바뀌거나 Possession이 바뀌면 UISubsystem이 Reticle Root Visibility와 Source Pawn을 함께 Rebind한다.
- 위젯 내부의 상태 기반 Hidden은 RenderOpacity 0으로 표현하고, Root Visibility는 UISubsystem의 Current Pawn 표시 조건이 담당한다.
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
  -> RenderOpacity 갱신
```

---

## 8. 현재 표시 조건 / 실행 조건

현재 `AimReticle`이 실제로 보이려면 아래 조건이 모두 중요하다.

```text
- ACFVehiclePawn::bShowAimReticle이 true여야 한다.
- Pawn이 Dedicated Server 환경이 아니어야 한다.
- Pawn이 로컬 제어 상태여야 한다.
- UISubsystem의 `DefaultAimReticleWidgetClass`가 현재 `WBP_AimReticle`로 해석되어야 한다.
- LocalPlayer의 `ACFPlayerController`와 UI Root가 유효해야 한다.
- VehicleAimComp가 있어야 의미 있는 상태를 읽을 수 있다.
- Pawn 표시 조건이 true이고 VehicleAimComp가 Hidden이 아닌 Reticle 상태를 반환해야 위젯 RenderOpacity가 1로 복구된다.
```

조건 해석:

```text
- UISubsystem/Root/Class 생성 전제가 없으면 Widget을 만들지 않는다.
- Current Pawn 표시 조건이 맞지 않으면 기존 singleton Widget을 Collapsed한다.
- AimComp 조건이 맞지 않으면 위젯은 안전하게 RenderOpacity 0인 Hidden 상태로 동작한다.
- WBP에 TextBlock이 없어도 위젯 자체는 동작할 수 있다.
```

---

## 9. 현재 자산 / 클래스 역할

### `UCFAimReticleWidget`

- 종류: C++ UserWidget 부모 클래스
- 현재 역할: Pawn/AimComp 상태, Weapon Aim Solution과 FireFeedback ViewData 읽기, 최종 Reticle 상태 결정, Optional 이미지/TextBlock 갱신, World To Screen 투영, 상태별 색상과 RenderOpacity 적용

### `UCFUISubsystem`

- 종류: LocalPlayer C++ Subsystem
- 현재 역할: `WBP_AimReticle` 단일 생성·HUD Layer Z10 연결, Current Pawn Rebind, World/Controller 수명 정리와 Root Visibility 적용

### `ACFVehiclePawn`

- 종류: C++ Pawn
- 현재 역할: `ShouldShowAimReticle()` 표시 조건과 `BuildFireFeedbackViewData()` UI 표시 데이터 제공. Legacy AimReticle Class/API는 직렬화 호환 경계이며 자동 생성 수명 owner가 아님

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
  - `Image_WeaponReticle`
  - `Text_FireFeedbackState`
  - `Text_FireFeedbackHint`
  - `Text_Cooldown`
  - `Text_OutOfArcWarning`
- 모든 바인딩은 누락 시 크래시가 발생하지 않는 Optional 구조다.

---

## 10. 현재 생성 및 연결 구조

현재 생성 및 연결 구조는 아래와 같다.

```text
ACFPlayerController::BeginPlay
  -> LocalPlayer UCFUISubsystem 등록
      -> UCFUIRootWidget
          -> HUD Layer
              -> WBP_CFInGameHUD Z0
              -> WBP_AimReticle Z10

Possession 변경
  -> UCFUISubsystem::NotifyPossessedPawnChanged
  -> RebindAimReticleToCurrentPawn
      -> UCFAimReticleWidget::SetVehiclePawnRef(CurrentVehiclePawn)
      -> ACFVehiclePawn::ShouldShowAimReticle()로 Root Visibility 결정

ACFVehiclePawn
  -> BuildFireFeedbackViewData
      -> LastFireResult / RejectReason
      -> VehicleWeaponComp Cooldown
      -> VehicleAimComp LocalAimState
      -> FCFVehicleFireFeedbackViewData

UCFAimReticleWidget
  -> Weak VehiclePawnRef.GetVehicleAimComp
  -> BaseReticleState / LocalAimState
  -> VehiclePawnRef.BuildFireFeedbackViewData
  -> CachedFireFeedbackViewData
  -> ResolveReticleStateFromFireFeedback
  -> CachedReticleState / bCachedCanFire
  -> RefreshTextBlocks / RefreshVisualStyle / RenderOpacity 갱신
```

전체 UI Root·Layer 수명과 Production HUD 경계는 `Document/Systems/UI/InGameUI.md`가 상위 Current owner다.

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
| `RejectReason == TurretAligning` | `OutOfArcWarning` + `FeedbackDisplayKey=TurretAligning` | 기본 상태 유지, 전용 보조 경고 | amber |
| `RejectReason == WeaponNotAligned` | 없음 또는 Debug 전용 | 기본 상태 유지, 빨간 주 Reticle 덮어쓰기 없음 | 기본 Aim 색상 |
| `RejectReason == MuzzleBlocked` | `AimBlocked` | `Blocked`로 덮어쓰기 | 주황색 |
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
- TurretAligning은 OutOfArcWarning과 같은 전용 보조 표시 경로를 사용하지만 문구는 `정렬 중`, 색상은 `TurretAligningReticleColor`를 사용한다.
- WeaponNotAligned는 주 Reticle을 빨간 FireRejected로 덮지 않는다.
- MuzzleBlocked는 AimBlocked와 같은 Blocked / 주황 표시로 처리한다.
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
- Hidden 상태에서는 Root Visibility를 바꾸지 않고 RenderOpacity 0으로 숨긴다.
- Optional WBP 위젯이 없어도 크래시 없이 동작한다.
- Image_CenterDot은 조준 레티클, Image_WeaponReticle은 터렛 레티클이라는 제품 의미를 지킨다.
- 터렛 레티클을 탄종이나 착탄 결과에 따라 다른 UI 의미로 바꾸지 않는다.
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
- 투사체 예상/실제 착탄 위치 계산과 월드 공간 3D 표시
```

현재 `AimReticle`은 계산기가 아니라, **이미 계산된 Local Aim 상태와 로컬 발사 결과 피드백을 화면에 읽기 쉽게 표시하는 UI 계층**이다.

---

## 14. 현재 문서 기준의 핵심 결론

현재 `AimReticle` 기능은,

**로컬 차량 Pawn의 VehicleAimComp 기본 상태와 FireFeedback ViewData를 결합해 Reticle 이미지, 한국어 상태 문구, 쿨다운 시간, 보조 경고, 상태별 색상으로 표시하는 현재 UI 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `AimReticle`은 현재 로컬 플레이어가 조준 상태와 로컬 발사 피드백을 즉시 읽을 수 있도록 Viewport UI로 표시해주는 기능이다.

---

## 15. 현재 확인 및 미확정 항목

2026-07-15 사용자 PIE 확인:

```text
- NoWeapon 회색 Reticle과 무기 없음 안내 문구 PASS
- 일반 AimBlocked 주황 Reticle과 조준 가림 안내 문구 PASS
- NoWeapon / AimBlocked 피드백 유지 시간 종료 후 텍스트 제거 PASS
- 장애물 제거 후 정상 Aim 상태 복귀 PASS
- Ready → FireSuccess → Cooldown → Ready 정상 발사 회귀 PASS
```

아래는 아직 이 문서에서 확정하지 않은 내용이다.

```text
- Reloading 상태를 실제 탄약/재장전 시스템과 연결할지 여부
- FirePending을 비동기/충전 무기 도입 시 활성화할지 여부
- Common UI PrimaryLayout 스택에 편입할지, 현재처럼 Viewport 직접 추가를 유지할지
- 터렛 레티클의 정렬 완료 Fade와 화면 밖 Clamp 정책
```

확정된 항목:

```text
- Image_CenterDot은 사용자의 조준 레티클이다.
- Image_WeaponReticle은 탄종 독립 터렛 레티클이다.
- 투사체 착탄 위치는 Reticle이 아닌 후속 3D 표시다.
```

---

## 16. 문서 갱신 조건

아래 변경이 생기면 이 문서를 함께 갱신한다.

```text
- UCFAimReticleWidget의 TextBlock 구성 변경
- GetReticleStateDisplayText() 표시 문구 변경
- GetReticleHintDisplayText() 표시 문구 변경
- UpdateReticleVisibility() 가시성 정책 변경
- `UCFUISubsystem::CreateAimReticleWidget()` 생성·단일 수명·Rebind 흐름 변경
- `DefaultAimReticleWidgetClass` 또는 HUD Layer/ZOrder 변경
- `ShouldShowAimReticle()` 표시 조건 변경
- AimReticle의 상위 UI Root/Layer 소유권 변경
- 실제 WBP 자산명과 구조가 확정될 때
- Reticle이 FireValidationState, LastFireResult, Cooldown 정보를 직접 읽도록 확장될 때
- FireFeedback 문서가 신설되어 Reticle과 피드백 책임이 분리될 때
```

---

## 17. 문서 버전 관리

- 현재 문서 버전: `1.10.0`
- 문서 상태: `Current / P0 Aim·FireFeedback Verified / UI-P0-04 UISubsystem Singleton·Pawn Rebind USER PASS`
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

### v1.9.0 -> v1.10.0

```text
- post-closure code review에서 확인한 AimReticle per-frame 중복 refresh만 교정한다.
- 자동 Pawn Refresh는 FireFeedback/Reticle cache를 한 번에 갱신한 뒤 RefreshTextBlocks, RefreshWeaponReticle, UpdateReticleVisibility를 각각 한 번 수행한다.
- SetVehiclePawnRef, NativeConstruct, NativeTick에서 RefreshFromPawn 이후 Visibility를 중복 호출하지 않는다.
- 외부 Blueprint/C++ ApplyReticleState / ApplyFireFeedbackViewData API의 의미와 기존 USER PASS는 변경하지 않는다.
- Widget Asset/Layout mutation은 0이다.
```

### v1.8.1 -> v1.9.0

```text
- AimReticle 자동 생성·단일 수명 owner를 ACFVehiclePawn이 아니라 LocalPlayer UCFUISubsystem Current 계약으로 교정한다.
- WBP_AimReticle은 UCFUISubsystem의 DefaultAimReticleWidgetClass에서 생성되어 HUD Layer Z10에 존재한다.
- Possession 변경은 UCFUISubsystem::NotifyPossessedPawnChanged -> RebindAimReticleToCurrentPawn 경로를 사용하고 UCFAimReticleWidget은 Weak VehiclePawnRef로 이전 Pawn lifetime을 소유하지 않는다.
- ACFVehiclePawn의 Legacy AimReticle Class/API는 저장 Asset 호환 경계로 남길 수 있으나 자동 생성 owner로 해석하지 않는다.
- Aim/FireFeedback/Turret Reticle 의미와 기존 USER PASS는 변경하지 않는다.
- 이번 문서 교정은 Product Source/Config/Content Asset을 변경하지 않는다.
```

### v1.5.0 -> v1.6.0

```text
- NoWeapon와 일반 AimBlocked의 실제 색상, 상태·보조 문구와 피드백 만료를 Current 동작으로 확정한다.
- 장애물 제거 후 정상 Aim 복귀와 정상 발사 상태 전환을 Current 동작으로 사용한다.
- CF-FQ-017 Done / CF-TC-014 PASS를 반영한다.
- 코드와 WBP 자산 마이그레이션은 필요하지 않다.
```

### v1.4.1 -> v1.5.0

```text
- 정렬 완료 후 Reticle 목표점과 실제 탄착 일치를 사용자 PIE 결과로 확정한다.
- TurretAligning amber와 WeaponNotAligned 비가림을 현재 Reticle 동작으로 사용한다.
- 정책 true/false 양쪽의 MuzzleBlocked 발사 차단과 Blocked 표시를 현재 기준에 포함한다.
- NoWeapon과 일반 AimBlocked 별도 UI 회귀는 CF-FQ-017에 남긴다.
- 코드와 WBP 자산은 변경하지 않는다.
```

### v1.4.0 -> v1.4.1

```text
- 기존 WBP_AimReticle 바인딩 이름은 변경하지 않는다.
- TurretAligning은 ECFVehicleReticleState enum 주 상태와 기존 FireFeedback DisplayKey 보조 표시를 함께 지원한다.
- UCFAimReticleWidget의 상태 기반 Hidden은 Root Visibility를 Collapsed로 바꾸지 않고 RenderOpacity 0으로 표현한다.
- Root Visibility는 ACFVehiclePawn::RefreshAimReticleWidget()의 ShouldShowAimReticle() 정책이 계속 담당한다.
- Reticle Recovery Hotfix 빌드는 성공했지만 PIE 검증 전이므로 PIE PASS로 기록하지 않는다.
```

### v1.3.0 -> v1.4.0

```text
- 기존 WBP_AimReticle 바인딩 이름은 변경하지 않는다.
- TurretAligning은 v1.4.1부터 ReticleState enum 주 상태와 FireFeedback DisplayKey 보조 표시를 함께 지원한다.
- 새 TurretAligningReticleColor 프로퍼티는 C++ 기본 amber 값을 사용하며 WBP에서 필요 시 조정할 수 있다.
- WeaponNotAligned는 빨간 FireRejected로 주 Reticle을 덮지 않는 정책을 유지한다.
- MuzzleBlocked는 기존 AimBlocked / Blocked 표시 경로를 사용한다.
- C++ 빌드는 성공했지만 PIE PASS는 별도 검증 전까지 기록하지 않는다.
```

### v1.2.0 -> v1.3.0

```text
- 현재 Reticle은 Aim/FireFeedback 표시 UI로 정상 동작하지만 실제 Muzzle 탄착점을 보장하지 않는 상태로 기록한다.
- Reticle 목표와 실제 발사 방향 통합 전까지 Ready 표시를 총구 정렬 완료와 같은 의미로 확장 해석하지 않는다.
- v1.4.1에서 TurretAligning 상태를 ECFVehicleReticleState enum에 추가했다.
- OutOfArc는 기계적 조준 범위 밖, TurretAligning은 범위 안에서의 회전 지연으로 분리한다.
- 완료 당시 목표 구조와 UI 변경 기준은 `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md`에 보존한다.
- 이 문서 갱신에서는 WBP와 C++를 변경하지 않는다.
```

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

### v1.10.0 - 2026-08-22

```text
- UCFAimReticleWidget.cpp v1.10.0의 자동 Pawn Refresh hot-path를 Current 문서에 반영했다.
- 한 Tick의 중복 Text/Visibility 갱신을 제거하고 최종 cache 확정 뒤 단일 visual refresh를 수행한다.
- 외부 Apply API, UISubsystem singleton/Rebind, Weak Pawn, Aim/FireFeedback/Turret 의미 계약은 그대로 유지한다.
- final Official Build bd3640c616794cd7a54cc8a8afa4b021 PASS와 CarFight.UI broad c7bbf0bf32c54d248c7cd3e122d700bc 44/44 PASS를 회귀 evidence로 사용한다.
```

### v1.9.0 - 2026-08-22

```text
- UI-P0-04 이후 Current AimReticle 생성·수명 owner를 UCFUISubsystem으로 교정했다.
- WBP_AimReticle의 HUD Layer Z10 singleton, Current Pawn Rebind, Weak VehiclePawnRef와 World cleanup 경계를 기록했다.
- ACFVehiclePawn은 ShouldShowAimReticle과 BuildFireFeedbackViewData source를 제공하고 Legacy AimReticle 직렬화/API는 호환 경계임을 명시했다.
- 기존 Aim/FireFeedback/Turret Reticle USER PASS와 의미 계약은 변경하지 않았다.
- 상위 인게임 UI Current owner로 Document/Systems/UI/InGameUI.md를 연결했다.
```

### v1.8.1 - 2026-07-21

```text
- CF-FQ-025 터렛 레티클 구현을 사용자 PIE PASS로 확정했다.
- Image_CenterDot과 탄종 독립 Image_WeaponReticle을 현재 검증 완료 UI 계약으로 전환했다.
```

### v1.8.0 - 2026-07-21

```text
- Image_WeaponReticle을 CurrentMuzzleDirection 기반 TurretReticleWorldLocation 소비 경로로 전환했다.
- 탄종별 ECFWeaponReticleMode와 WeaponPreviewWorldLocation의 UI 의존을 제거했다.
- 공식 에디터 빌드 PASS와 사용자 PIE 대기 상태를 기록했다.
```

### v1.7.1 - 2026-07-21

```text
- Image_CenterDot의 현재 작동을 사용자 확인 PASS로 기록했다.
- Image_CenterDot 입력 계약 확인을 후속 작업에서 제거하고 회귀 보호 대상으로 전환했다.
```

### v1.7.0 - 2026-07-21

```text
- Image_CenterDot을 사용자가 화면 내 위치를 지정하는 조준 레티클로 확정했다.
- Image_WeaponReticle을 CurrentMuzzleDirection 기반 터렛 조준 3D 지점의 화면 투영으로 확정했다.
- 터렛 레티클을 HitScan/Projectile, 중력 여부와 착탄 위치에서 분리했다.
- 투사체 착탄 위치를 Reticle UI가 아닌 후속 월드 공간 3D 표시로 이관했다.
- 현재 DirectImpact/LaunchDirection Preview 소비 코드는 새 계약에 맞춘 구현 정렬 대기 상태로 기록했다.
```

### v1.6.0 - 2026-07-15

```text
- NoWeapon 회색 Reticle, AimBlocked 주황 Reticle과 상태·보조 문구를 사용자 PIE로 확인했다.
- 두 실패 상태의 피드백 만료, 장애물 제거 후 정상 Aim 복귀와 정상 발사 회귀를 확인했다.
- CF-FQ-017 Done / CF-TC-014 PASS를 현재 Reticle 상태에 반영했다.
```

### v1.5.0 - 2026-07-14

```text
- CF-FQ-022 P0 사용자 PIE 결과를 Reticle 현재 상태에 반영했다.
- 정렬 완료 탄착 일치, TurretAligning amber, WeaponNotAligned 비가림과 MuzzleBlocked 표시를 PASS로 기록했다.
- PIE Pending 상태를 P0 Aim Alignment Reticle Verified로 변경했다.
- NoWeapon과 일반 AimBlocked 별도 UI 검증은 CF-FQ-017에 유지했다.
```

### v1.4.1 - 2026-07-13

```text
- Reticle Recovery Hotfix로 TurretAligning ReticleState enum 표시 기준 반영
- 상태 기반 Hidden이 Root Visibility 대신 RenderOpacity를 사용하도록 변경된 현재 기준 반영
- BuildEditor.bat 성공과 PIE Pending 상태를 기록
```

### v1.4.0 - 2026-07-13

```text
- TurretAligning 표시를 정렬 중 문구와 amber 보조 색상으로 기록
- WeaponNotAligned가 빨간 FireRejected로 주 Reticle을 덮지 않는 정책 기록
- MuzzleBlocked를 AimBlocked / Blocked 주황 표시 경로로 기록
- AimFireAlignment C++ 빌드 완료와 PIE Pending 상태를 분리 기록
- 당시 기준으로 FireFeedback DisplayKey 기반 TurretAligning 보조 표시 구현 기준 추가
```

### v1.3.0 - 2026-07-13

```text
- Reticle 목표점과 실제 Muzzle 발사 방향 불일치 사용자 PIE 결과 반영
- 현재 Reticle을 조준 의도/피드백 표시 UI로 유지하되 실제 탄착 보장은 미완료로 명시
- Command Reticle, TurretAligning, Weapon Reticle 후보 의미 정리
- OutOfArc와 TurretAligning 분리 기준 추가
- `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md` Historical evidence 연결
```

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

- 확인 일시: `2026-08-22`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
  - `UE/Source/CarFight_Re/Public/UI/CFUISubsystem.h`
  - `UE/Source/CarFight_Re/Private/UI/CFUISubsystem.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle` 자산 상세 덤프
  - 2026-07-13 사용자 PIE 확인 결과
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/Combat/WeaponFire.md`
