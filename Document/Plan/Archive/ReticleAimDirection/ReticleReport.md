# CarFight — 레티클 시스템 종합 보고서

- Version: 1.0.0
- Date: 2026-07-16
- Status: Active Report / CF-FQ-025 Phase 2 Ready
- Project: CarFight
- Feature ID: `CF-FQ-025 이중 레티클 및 사격방향 시각화`
- Priority: `P0 / 구현순위 1위`
- Scope: 현재 구현된 Command Reticle과 FireFeedback, 구현·검증된 Weapon Preview 데이터, 앞으로 구현할 Weapon Reticle·Ballistic Reticle·Lead Indicator를 하나의 보고서에서 상태별로 구분한다.

---

## 1. 보고서 목적

이 문서는 CarFight의 레티클 시스템을 다음 세 범위로 나누어 정리한다.

```text
Current
= 현재 게임 화면과 런타임에서 동작하며 사용자 PIE 검증까지 끝난 기능

Implemented / Not Yet Presented
= C++ 데이터와 디버그 경로는 구현·검증됐지만 실제 HUD 표시가 아직 연결되지 않은 기능

Planned
= 설계와 구현 순서는 확정됐지만 코드 또는 WBP 작업이 아직 시작되지 않은 기능
```

이 구분은 아래와 같은 오해를 방지하기 위한 것이다.

```text
Weapon Preview World 데이터가 존재한다
≠
Weapon Reticle이 현재 화면에 표시된다
```

현재 구현 기준은 `Document/Systems/UI/AimReticle.md`를 우선한다.
진행 중인 구현 계획과 다음 단계는 `Document/Plan/ReticleAimDirection/ImplementationDesign.md`를 우선한다.
최신 작업 단계는 `Document/ActiveWork.md`의 `CF-FQ-025` 체크포인트를 함께 사용한다.

---

## 2. 경영 요약

현재 CarFight의 레티클 시스템은 **플레이어가 지정한 조준 목표와 실제 발사 판정을 연결하는 기반 기능**까지 완료되어 있다.

현재 화면 중앙의 Reticle은 플레이어의 조준 의도를 나타내는 `Command Reticle`이다.
이 Reticle은 조준 가능 여부뿐 아니라 `Ready`, `Blocked`, `OutOfArc`, `TurretAligning`, `NoWeapon`, `Cooldown`, `FireRejected` 등 조준·발사 상태를 색상과 문구로 표시한다.

`CF-FQ-022`에서 다음 핵심 정렬 계약이 완료됐다.

```text
Command Reticle 목표점
→ Muzzle 기준 요구 방향 계산
→ 터렛이 요구 방향 추적
→ 실제 최종 AimDirection 결정
→ HitScan과 Projectile이 같은 AimDirection 사용
```

따라서 정렬 완료 후에는 Command Reticle 목표점과 실제 탄착이 일치한다.
다만 `bAllowFireWhileAligning=true`인 무기는 터렛 정렬 중에도 현재 총구 방향으로 정상 발사할 수 있으므로, 이 순간에는 화면 중앙 Command Reticle과 실제 발사 방향이 서로 다를 수 있다.

이를 해결하기 위해 `CF-FQ-025`가 진행 중이다.

현재 `CF-FQ-025`의 World Preview 단계는 완료됐다.

```text
- ECFWeaponReticleMode 구현
  - Hidden
  - DirectImpact
  - LaunchDirection
- Weapon Preview World 위치 구현
- Weapon Preview 거리 구현
- DirectImpact의 WeaponHit Trace 공유 구현
- 중력 Projectile의 LaunchDirection 구현
- VehicleDebug 표시 구현
- Editor 빌드 PASS
- Heavy Cannon 사용자 PIE PASS
```

그러나 화면 투영과 WBP 표시는 아직 구현되지 않았다.
현재 다음 작업은 **Phase 2 — Weapon Preview World 위치를 화면 좌표로 투영하고 `Image_WeaponReticle`에 표시하는 것**이다.

최종 방향은 아래와 같다.

```text
현재
Command Reticle + FireFeedback

다음
Command Reticle + Weapon Reticle

후속
Command Reticle + Weapon Reticle + Ballistic Impact Reticle

장기
Command Reticle + Weapon Reticle + Ballistic Reticle + Lead Indicator
```

---

## 3. 레티클 용어 정의

### 3.1 Command Reticle

```text
의미
= 플레이어가 무기에게 향하라고 명령하는 조준 위치

현재 기준 데이터
= FCFVehicleWeaponAimSolution.AimTargetLocation

화면 위치
= 현재 화면 중앙 고정
```

Command Reticle은 현재 `WBP_AimReticle`의 중앙점과 네 방향 브라켓으로 표현된다.

Command Reticle은 실제 총구가 현재 그 위치를 향하고 있다는 뜻이 아니다.
플레이어가 지정한 목표와 현재 조준·발사 상태를 나타내는 명령 UI다.

### 3.2 Weapon Reticle

```text
의미
= 지금 발사가 승인됐을 때 실제로 사용할 최종 발사 방향

기준 데이터
= FCFVehicleWeaponAimSolution.AimDirection

표시 기준
= AimOrigin에서 AimDirection으로 계산한 Weapon Preview World 위치
```

Weapon Reticle은 아직 화면에 표시되지 않는다.
현재는 필요한 World 데이터와 표시 모드가 C++ 및 VehicleDebug에 구현된 상태다.

### 3.3 DirectImpact Reticle

```text
대상
= HitScan
또는
= 중력이 적용되지 않는 Projectile

의미
= 현재 최종 발사 방향에서 처음 만나는 Blocking 표면 또는 최대 사거리 끝점
```

이 모드에서는 Weapon Preview 위치를 실제 예상 충돌 위치로 해석할 수 있다.

### 3.4 LaunchDirection Marker

```text
대상
= 중력이 적용되는 Projectile

의미
= 발사 순간의 실제 초기 방향을 나타내는 표식

월드 위치
= AimOrigin + AimDirection * PreviewMaxRange
```

LaunchDirection은 예상 탄착점이 아니다.
중력, 탄속, 비행시간을 계산하지 않으므로 `Ballistic Impact`로 표현해서는 안 된다.

### 3.5 Ballistic Impact Reticle

```text
의미
= 중력과 초기 속도를 고려한 예상 탄착 위치

상태
= Planned / 미구현
```

Ballistic Solver가 먼저 구현되어야 한다.

### 3.6 Lead Indicator

```text
의미
= 이동 목표를 맞히기 위해 조준해야 하는 미래 위치

상태
= Planned / 장기 후속
```

Lead Indicator는 Weapon Reticle을 대체하지 않는다.
Weapon Reticle은 현재 무기 방향이고, Lead Indicator는 목표의 이동을 고려한 권장 조준 위치다.

---

## 4. 현재 시스템 구성

### 4.1 핵심 C++ 클래스와 구조체

| 항목 | 현재 역할 | 상태 |
| --- | --- | --- |
| `ACFVehiclePawn` | Reticle 위젯 생성·제거, FireFeedback ViewData 생성, Weapon Aim Solution과 Preview 계산 | Current |
| `UCFVehicleAimComp` | Local Aim 상태, Reticle 상태와 Weapon Aim Solution 보관 | Current |
| `UCFAimReticleWidget` | Aim 상태와 FireFeedback을 읽어 현재 Command Reticle의 색상·문구·가시성 갱신 | Current |
| `FCFVehicleWeaponAimSolution` | 요구 방향, 현재 총구 방향, 실제 최종 방향, 정렬·차단 및 Weapon Preview 데이터 보관 | Current + CF-FQ-025 확장 |
| `FCFVehicleLocalAimState` | 로컬 조준 목표, 방향, Reticle 상태와 발사 가능 예측 보관 | Current |
| `FCFVehicleFireFeedbackViewData` | 발사 성공, 쿨다운, 거부 사유와 UI 표시 데이터 보관 | Current |
| `ECFVehicleReticleState` | Command Reticle 상태 정의 | Current |
| `ECFWeaponReticleMode` | Weapon Reticle World 데이터의 의미 정의 | Implemented / HUD 미연결 |

### 4.2 현재 WBP 자산

```text
/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle
```

현재 확인된 주요 Optional 바인딩:

```text
Image_CenterDot
Image_LeftBracket
Image_RightBracket
Image_TopBracket
Image_BottomBracket
Text_FireFeedbackState
Text_FireFeedbackHint
Text_Cooldown
Text_OutOfArcWarning
```

2026-07-16 자산 상세 덤프와 C++ 검색 기준으로 다음 항목은 아직 없다.

```text
Image_WeaponReticle
Image_WeaponBlocked
Image_WeaponOffscreen
```

따라서 현재 WBP는 Command Reticle과 FireFeedback까지만 실제 화면에 표시한다.

---

## 5. 현재 데이터 흐름

### 5.1 Command Reticle과 조준 목표

```text
VehicleCameraComp
→ Camera Aim Trace
→ AimHitLocation
→ AimTargetLocation
→ Command Reticle 목표
```

현재 Command Reticle은 화면 중앙에 고정되어 있고, 카메라 Aim Trace가 선택한 월드 목표 위치를 나타낸다.

### 5.2 요구 방향과 실제 최종 발사 방향

```text
AimOrigin
= Muzzle 위치

DesiredAimDirection
= Muzzle → AimTargetLocation

CurrentMuzzleDirection
= 현재 Muzzle Socket X축 방향

AimDirection
= 현재 발사 정책을 반영한 실제 최종 발사 방향
```

현재 정책:

```text
정렬 완료
→ AimDirection = DesiredAimDirection

정렬 중 + bAllowFireWhileAligning=true
→ AimDirection = CurrentMuzzleDirection

정렬 중 + bAllowFireWhileAligning=false
→ 발사 거부

MuzzleBlocked=true
→ 정책과 관계없이 발사 거부
```

### 5.3 현재 Command Reticle UI 흐름

```text
UCFVehicleAimComp
→ Base Reticle State / Local Aim State

ACFVehiclePawn::BuildFireFeedbackViewData()
→ FireSuccess / Cooldown / NoWeapon / AimBlocked / FireRejected 등

UCFAimReticleWidget::RefreshFromPawn()
→ 두 상태 결합
→ 최종 Command Reticle 상태 결정
→ 텍스트·색상·Opacity 갱신
```

### 5.4 Weapon Preview 데이터 흐름

```text
FCFVehicleWeaponAimSolution.AimDirection
→ 활성 WeaponData / ProjectileData 확인
→ ECFWeaponReticleMode 결정
→ Preview 거리 결정
→ DirectImpact Trace 또는 LaunchDirection 위치 계산
→ FCFVehicleWeaponAimSolution에 결과 저장
→ VehicleDebug 표시
```

현재 여기까지 구현됐다.

아직 구현되지 않은 다음 연결:

```text
WeaponPreviewWorldLocation
→ ProjectWorldLocationToScreen
→ Weapon Reticle 화면 위치
→ Image_WeaponReticle 표시
```

---

## 6. 현재 Command Reticle 상태와 표시 규칙

| 상태 | 현재 의미 | 기본 표현 |
| --- | --- | --- |
| `Hidden` | 표시할 유효한 Local Aim 없음 | RenderOpacity 0 |
| `Ready` | 조준 상태 정상 | 흰색 |
| `Blocked` | 조준선 또는 총구 경로가 막힘 | 주황색 |
| `OutOfArc` | 현재 조준각이 기준 AimProfile 범위 밖 | 보조 노란 경고 |
| `TurretAligning` | 범위 안이지만 터렛·총구가 아직 목표를 추적 중 | amber |
| `NoWeapon` | 사용 가능한 무기 없음 | 회색 |
| `Cooldown` | 활성 무기 재사용 대기 | 파란색 + 남은 시간 |
| `Reloading` | 재장전 상태 후보 | 타입 존재 / 실제 시스템 미연결 |
| `FirePending` | 발사 처리 중 상태 후보 | 타입 존재 / 현재 동기 발사에서는 미사용 |
| `FireRejected` | 발사 조건 미충족 | 빨간색 |

### 6.1 주 Reticle과 보조 피드백 우선순위

```text
1. FireSuccess 유지 시간
2. Cooldown
3. RejectReason별 실패 피드백
4. 기본 Aim 상태
```

예외:

```text
OutOfArcWarning
= 주 Reticle을 강제로 노란색으로 덮지 않는 보조 경고

TurretAligning
= 정렬 중 상태를 amber로 표시

WeaponNotAligned
= 주 Reticle을 무조건 빨간 FireRejected로 덮지 않음

MuzzleBlocked
= AimBlocked와 같은 Blocked / 주황 표시
```

### 6.2 OutOfArc 운영 기준

현재 P0 싱글플레이 기준:

```text
OutOfArc
= 조준각 경고 및 디버그 상태

OutOfArc
≠ 단독 발사 차단 조건
```

실제 발사 성공 여부는 `ValidateFireCommand()` 결과가 최종 기준이다.

---

## 7. 현재 검증 완료 항목

### 7.1 Command Reticle과 FireFeedback

사용자 PIE에서 확인된 항목:

```text
- NoWeapon 회색 Reticle PASS
- 무기 없음 상태·보조 문구 PASS
- AimBlocked 주황 Reticle PASS
- 조준 가림 상태·보조 문구 PASS
- 실패 피드백 유지 시간 종료 후 텍스트 제거 PASS
- 장애물 제거 후 정상 Aim 복귀 PASS
- Ready → FireSuccess → Cooldown → Ready PASS
```

### 7.2 조준점·터렛·총구 정렬

사용자 PIE에서 확인된 항목:

```text
- 정렬 완료 후 Command Reticle 목표점과 실제 탄착 일치 PASS
- bAllowFireWhileAligning=true 정렬 중 현재 Muzzle 방향 발사 PASS
- bAllowFireWhileAligning=false 정렬 중 발사 거부 PASS
- 정렬 완료 후 발사 승인 PASS
- TurretAligning amber 표시 PASS
- WeaponNotAligned가 주 Reticle을 빨간색으로 가리지 않음 PASS
- MuzzleBlocked가 정책과 관계없이 발사 차단 PASS
```

### 7.3 Weapon Preview World 데이터

최신 `Document/ActiveWork.md` v1.15 기준:

```text
- Hidden / DirectImpact / LaunchDirection 모드 구현 PASS
- LaunchDirection World 위치 갱신 PASS
- Preview Valid PASS
- LaunchDirection Blocking Hit=false PASS
- Heavy Cannon Preview Distance=10000 확인 PASS
- MuzzleBlocked 회귀 PASS
- 실제 중력탄 초기 발사 방향 일치 PASS
- Tools\BuildEditor.bat PASS
```

주의:

`Document/Plan/ReticleAimDirection/ImplementationDesign.md` v0.2.5의 체크포인트에는 사용자 검증 중이던 이전 상태가 일부 남아 있다.
이 보고서는 그보다 나중에 갱신된 `Document/ActiveWork.md` v1.15의 사용자 PIE PASS를 최신 상태로 사용한다.

---

## 8. 전체 구현 현황표

| 기능 | 코드 | WBP/HUD | 빌드 | 사용자 PIE | 현재 판정 |
| --- | --- | --- | --- | --- | --- |
| Command Reticle | 완료 | 완료 | PASS | PASS | Current |
| Reticle 상태·색상 | 완료 | 완료 | PASS | PASS | Current |
| FireSuccess / Cooldown | 완료 | 완료 | PASS | PASS | Current |
| NoWeapon / AimBlocked | 완료 | 완료 | PASS | PASS | Current |
| TurretAligning 표시 | 완료 | 완료 | PASS | PASS | Current |
| MuzzleBlocked 표시·차단 | 완료 | 완료 | PASS | PASS | Current |
| AimDirection 단일 발사 기준 | 완료 | 해당 없음 | PASS | PASS | Current |
| DirectImpact Preview World 데이터 | 완료 | 미연결 | PASS | 코드·기반 검증 완료 | Implemented / HUD 미연결 |
| LaunchDirection Preview World 데이터 | 완료 | 미연결 | PASS | PASS | Implemented / HUD 미연결 |
| Weapon Reticle World To Screen | 미구현 | 미구현 | 미검증 | 미검증 | Planned / Phase 2 |
| `Image_WeaponReticle` | C++ 바인딩 미구현 | 자산 미배치 | 미검증 | 미검증 | Planned / Phase 2~3 |
| 화면 밖 Clamp / Chevron | 미구현 | 미구현 | 미검증 | 미검증 | Planned / Phase 4 이후 |
| Ballistic Impact Reticle | 미구현 | 미구현 | 미검증 | 미검증 | Planned / Phase 5 |
| Lead Indicator | 미구현 | 미구현 | 미검증 | 미검증 | Planned / Phase 6 |
| Reloading 실제 연결 | 미구현 | 상태 후보만 존재 | 미검증 | 미검증 | Deferred |
| FirePending 실제 연결 | 미구현 | 상태 후보만 존재 | 미검증 | 미검증 | Deferred |

---

## 9. CF-FQ-025 현재 구현 내용

### 9.1 Weapon Reticle 표시 모드

현재 구현된 enum:

```text
ECFWeaponReticleMode

Hidden
DirectImpact
LaunchDirection
```

의미:

```text
Hidden
= 화면에 투영할 유효한 Weapon Preview 데이터 없음

DirectImpact
= HitScan 또는 중력 없는 Projectile의 첫 충돌 또는 최대 사거리 끝점

LaunchDirection
= 중력 Projectile의 실제 초기 발사 방향 표식
```

현재 `BallisticImpact`와 `LeadPoint`는 enum에 미리 추가하지 않는다.
실제 Solver와 대상 예측 데이터가 구현될 때 추가한다.

### 9.2 Weapon Preview 데이터

현재 `FCFVehicleWeaponAimSolution`에서 사용하는 Preview 정보:

```text
WeaponReticleMode
bHasValidWeaponPreview
bWeaponPreviewHasBlockingHit
WeaponPreviewWorldLocation
WeaponPreviewDistance
```

### 9.3 Preview 거리 기준

권장·현재 계약:

```text
1. 활성 WeaponData의 MaxRange 우선
2. 유효하지 않으면 AimProfile MaxAimDistance 사용
3. 둘 다 유효하지 않으면 Hidden 처리
```

### 9.4 DirectImpact 계산

```text
PreviewStart
= AimOrigin

PreviewDirection
= AimDirection

PreviewEnd
= AimOrigin + AimDirection * PreviewMaxRange

TraceChannel
= WeaponHit
```

Blocking Hit이 있으면:

```text
WeaponPreviewWorldLocation = ImpactPoint
bWeaponPreviewHasBlockingHit = true
```

Blocking Hit이 없으면:

```text
WeaponPreviewWorldLocation = PreviewEnd
bWeaponPreviewHasBlockingHit = false
```

### 9.5 LaunchDirection 계산

```text
WeaponPreviewWorldLocation
= AimOrigin + AimDirection * PreviewMaxRange

bWeaponPreviewHasBlockingHit
= false
```

MuzzleBlocked는 LaunchDirection의 Blocking Hit 값으로 표현하지 않는다.
기존 `bMuzzleBlocked`를 별도로 사용한다.

### 9.6 MuzzleBlocked와 Preview Trace 공유

DirectImpact Preview는 기존 총구 장애물 판정과 충돌 기준이 달라지지 않도록 같은 `WeaponHit` Trace 결과를 공유한다.

보호 원칙:

```text
- 서로 다른 Trace Channel 금지
- 서로 다른 Ignore 목록 금지
- 서로 다른 AimDirection 금지
- UI 전용 Trace 추가 금지
- Preview를 맞추기 위한 FireRequest 방향 변경 금지
```

---

## 10. 구현 예정 — Phase 2 Weapon Reticle 화면 투영

### 10.1 목표

```text
WeaponPreviewWorldLocation을 실제 HUD 좌표로 투영하고,
현재 최종 발사 방향을 별도의 Weapon Reticle로 표시한다.
```

### 10.2 C++ 구현 대상

주요 대상:

```text
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

필요 시 읽기 경로 보강:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

### 10.3 권장 데이터 흐름

```text
UCFAimReticleWidget::RefreshFromPawn()
→ VehiclePawnRef에서 최신 Weapon Aim Solution 읽기
→ bHasValidWeaponPreview 확인
→ WeaponReticleMode 확인
→ WeaponPreviewWorldLocation 읽기
→ PlayerController.ProjectWorldLocationToScreen 호출
→ 화면 안/뒤쪽 여부 판단
→ Weapon Reticle 화면 위치 캐시
→ Optional Image 위치와 가시성 갱신
```

### 10.4 권장 C++ 상태 데이터

아래 이름은 구현 계약 작성 시 최종 검토한다.

```text
bCachedWeaponReticleVisible
CachedWeaponReticleScreenPosition
CachedWeaponReticleMode
bCachedWeaponReticleOnScreen
bCachedWeaponReticleBlocked
```

Blueprint에서 읽을 필요가 있는 값은 `BlueprintReadOnly`와 명확한 ToolTip을 제공한다.

예상 ToolTip 기준:

```text
CachedWeaponReticleScreenPosition
= 현재 실제 최종 발사 방향의 Weapon Preview 월드 위치를 화면 좌표로 변환한 결과입니다.

bCachedWeaponReticleOnScreen
= Weapon Preview 위치가 현재 카메라 앞쪽이며 화면 영역 안에 투영됐는지 나타냅니다.
```

### 10.5 Optional WBP 바인딩

Phase 2 필수:

```text
Image_WeaponReticle
```

후속 선택:

```text
Image_WeaponBlocked
Image_WeaponOffscreen
```

모든 바인딩은 `BindWidgetOptional`을 사용한다.
WBP가 아직 갱신되지 않았거나 선택 위젯이 누락돼도 크래시가 발생하지 않아야 한다.

### 10.6 화면 위치 적용 방식

권장 구조:

```text
CanvasPanelSlot
→ SetPosition(ScreenPosition - ReticleHalfSize)
```

주의:

```text
- Viewport 절대 좌표와 DPI Scale 좌표를 혼동하지 않는다.
- ProjectWorldLocationToScreen의 PlayerViewportRelative 정책을 명시한다.
- Canvas 기준 좌표로 변환할 때 Geometry Scale을 고려한다.
- 화면 중앙 Command Reticle의 위치를 움직이지 않는다.
```

### 10.7 첫 구현의 표시 정책

```text
- Preview Valid=false → 숨김
- 투영 실패 → 숨김
- 카메라 뒤쪽 → 숨김
- 화면 밖 → 첫 구현에서는 숨김
- 화면 안 → 표시
- 보간 없음
```

첫 구현에서 보간을 넣지 않는 이유:

```text
Weapon Reticle의 핵심 목적은 실제 발사 방향의 정확한 표시다.
보간이 강하면 UI가 실제 AimDirection보다 늦게 움직여 잘못된 방향을 보여줄 수 있다.
```

---

## 11. 구현 예정 — Phase 3 WBP 시각 디자인

### 11.1 시각적 역할 분리

```text
Command Reticle
= 플레이어 입력과 전투 상태의 주 UI

Weapon Reticle
= 현재 실제 발사 방향을 보조하는 저우선순위 UI
```

Weapon Reticle은 Command Reticle보다 작고 약하게 보여야 한다.

### 11.2 권장 형태

```text
- 작은 빈 원
또는
- 중심이 비어 있는 작은 십자
```

Command Reticle과 겹쳤을 때 두 요소가 같은 위치에 있다는 사실을 읽을 수 있어야 한다.

### 11.3 권장 상태 표현

| 상태 | Weapon Reticle 권장 표현 |
| --- | --- |
| `TurretAligning` | amber 또는 중립색, 비교적 높은 Opacity |
| 정렬 완료 | Command Reticle 안에 겹침, 낮은 Opacity |
| `MuzzleBlocked` | 주황색 또는 짧은 Pulse |
| `DirectImpact` | 빈 원 또는 작은 십자 |
| `LaunchDirection` | DirectImpact와 다른 내부 표시 또는 선형 마커 후보 |
| 데이터 무효 | 숨김 |

### 11.4 권장 조절 프로퍼티

```text
bShowWeaponReticle
WeaponReticleAlignedOpacity
WeaponReticleAligningOpacity
WeaponReticleBlockedOpacity
WeaponReticleScreenPadding
WeaponReticlePositionLerpSpeed
```

첫 구현에서는 `WeaponReticlePositionLerpSpeed`를 사용하지 않거나 즉시 갱신으로 둔다.

### 11.5 정렬 완료 시 표시 정책

첫 검증에서는 항상 표시를 권장한다.

```text
이유
- 실제 방향 검증이 쉽다.
- 정렬 완료 순간의 Flicker를 피할 수 있다.
- Command Reticle과 정확히 겹치는지 확인할 수 있다.
```

사용자 PIE 후 다음을 비교한다.

```text
A. 항상 낮은 Opacity로 유지
B. 정렬 완료 후 Fade Out
C. 일정 오차 이하에서 축소
```

---

## 12. 구현 예정 — Phase 4 이동·해상도·화면 경계 회귀

### 12.1 이동 회귀

```text
- 정지 차량 + 정지 목표
- 직진 차량 + 정지 목표
- 선회 차량 + 정지 목표
- 차량과 카메라 동시 회전
- 차량 Pitch 변화
- 차량 Roll 변화
```

### 12.2 거리 회귀

```text
- 5m
- 20m
- 100m
- 최대 사거리 근처
```

### 12.3 화면 영역 회귀

```text
- 화면 중앙
- 좌우 가장자리
- 상하 가장자리
- 카메라 뒤쪽
- 투영 실패 위치
```

### 12.4 해상도와 DPI 회귀

```text
- 일반 16:9
- 21:9
- 5120×1440급 초광폭
- Windowed / Fullscreen
- UI DPI Scale 변화
```

### 12.5 화면 밖 후속 정책

첫 구현:

```text
화면 밖이면 숨김
```

후속 후보:

```text
- 화면 가장자리 Clamp
- 발사 방향 Chevron
- BehindCamera 전용 상태
```

화면 밖 Clamp는 방향 데이터 정확성 검증이 끝난 뒤 추가한다.

---

## 13. 구현 예정 — Phase 5 Ballistic Impact Reticle

### 13.1 착수 조건

```text
- Command Reticle과 Weapon Reticle 역할이 사용자에게 명확함
- Phase 2~4 사용자 PIE PASS
- ProjectileData의 초기 속도와 중력 설정이 안정적임
- Projectile 실제 이동과 Solver가 같은 물리 기준을 사용함
```

### 13.2 Solver 입력 후보

```text
MuzzleWorldLocation
DesiredAimTargetLocation
ProjectileInitialSpeed
GravityScale
WorldGravity
ShooterVelocity
```

### 13.3 Solver 출력 후보

```text
bHasBallisticSolution
BallisticLaunchDirection
BallisticFlightTimeSeconds
BallisticImpactLocation
bUsesHighArcSolution
```

### 13.4 탄도 UI 역할

```text
Command Reticle
= 플레이어가 맞히고 싶은 위치

Weapon Reticle
= 현재 총구의 실제 초기 발사 방향

Ballistic Impact Reticle
= 현재 설정에서 예상되는 탄착 위치
```

이 세 요소는 중력 무기에서 서로 다른 위치일 수 있다.

### 13.5 금지 사항

```text
- LaunchDirection을 예상 탄착점이라고 표시하지 않는다.
- UI에서 탄도 방정식을 별도로 계산하지 않는다.
- Solver 방향과 실제 Projectile 초기 방향을 따로 유지하지 않는다.
- 해가 없는 상태를 임의의 직선 표시로 위장하지 않는다.
```

---

## 14. 구현 예정 — Phase 6 Lead Indicator

### 14.1 착수 조건

```text
- 목표 선택 또는 락온 데이터가 존재함
- 대상 위치와 속도를 신뢰할 수 있음
- Projectile 비행시간 계산이 신뢰 가능함
- Ballistic 또는 DirectImpact 비행 모델이 확정됨
```

### 14.2 필요한 입력

```text
TargetWorldLocation
TargetVelocity
ShooterVelocity
ProjectileInitialSpeed
Gravity
```

### 14.3 역할 분리

```text
Command Reticle
= 현재 플레이어 입력

Weapon Reticle
= 현재 실제 무기 방향

Lead Indicator
= 이동 목표를 맞히기 위한 권장 조준 위치
```

Lead Indicator는 자동 조준이 아니다.
플레이어에게 선행 조준 정보를 제공하는 UI다.

---

## 15. C++ / Blueprint / DataAsset 책임 분리

### 15.1 C++ 책임

```text
- AimTargetLocation 생성·보관
- DesiredAimDirection 계산
- CurrentMuzzleDirection 읽기
- 최종 AimDirection 결정
- 정렬 오차 계산
- MuzzleBlocked 판정
- WeaponReticleMode 결정
- Weapon Preview World 위치 계산
- World To Screen 투영
- 화면 안/밖과 카메라 앞/뒤 판정
- UI가 사용할 상태와 위치 데이터 제공
- VehicleDebug 데이터 제공
```

### 15.2 WBP 책임

```text
- Command Reticle 이미지 배치
- Weapon Reticle 이미지 배치
- 크기, 색상, Opacity와 애니메이션
- 겹침 상태 가독성
- 화면 가장자리 아이콘의 외형
- 해상도·DPI에 따른 레이아웃
```

WBP 금지 책임:

```text
- Line Trace 또는 Sweep
- AimDirection 계산
- 총구 방향 계산
- 발사 가능 판정
- 정렬 오차 계산
- Ballistic Solver
- Lead 계산
```

### 15.3 DataAsset 책임

현재 사용 데이터:

```text
UCFWeaponData.MaxRange
UCFTurretMountData.StabilizationToleranceDeg
UCFTurretMountData.bAllowFireWhileAligning
UCFProjectileData.InitialSpeed
UCFProjectileData.bAffectedByGravity
UCFProjectileData.GravityScale
```

첫 Weapon Reticle 구현에서는 신규 DataAsset을 만들지 않는다.
UI 스타일은 우선 `UCFAimReticleWidget`의 `EditDefaultsOnly` 프로퍼티로 둔다.
무기별 UI 정책이 실제로 필요해진 뒤 별도 UI DataAsset 승격을 검토한다.

---

## 16. 예상 수정 파일

### 16.1 Phase 2~3

```text
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
/Game/CarFight/UI/WBP_AimReticle
```

필요 시:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

### 16.2 구현 완료 후 갱신할 Systems 문서

```text
Document/Systems/UI/AimReticle.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/UI/VehicleDebugPanel.md
```

Ballistic 단계:

```text
Document/Systems/Combat/Projectile.md
```

신규 또는 수정 파일명은 32자를 넘지 않는다.

---

## 17. Phase 2~4 검증 계획

### 17.1 정렬 중 발사 허용

```text
1. Reticle을 빠르게 좌우로 이동한다.
2. 터렛이 따라오는 중 발사한다.
3. Weapon Reticle과 실제 발사 방향을 비교한다.
4. 정렬 완료 시 Command Reticle과 겹치는지 확인한다.
```

PASS:

```text
- 탄착 또는 초기 발사 방향이 Weapon Reticle과 일치
- Command Reticle은 플레이어 목표를 계속 유지
```

### 17.2 정렬 중 발사 금지

```text
1. 두 Reticle이 분리된 상태에서 발사한다.
2. TurretAligning 거부를 확인한다.
3. 두 Reticle이 정렬된 뒤 발사한다.
```

PASS:

```text
- 정렬 전 발사 거부 유지
- 정렬 후 발사 승인
- 승인된 발사 방향과 Weapon Reticle 일치
```

### 17.3 MuzzleBlocked

테스트 배치:

```text
- 카메라는 목표를 볼 수 있으나 총구 앞은 낮은 벽으로 막힘
- 차량 차체에 가까운 장애물
- 목표 앞 다른 차량
```

PASS:

```text
- 기존 MuzzleBlocked 발사 차단 유지
- Command Reticle Blocked 표시 유지
- Weapon Reticle이 정의된 Blocked 표현을 사용
- Preview 표시 때문에 발사 판정이 변경되지 않음
```

### 17.4 DirectImpact

```text
- Dummy HitScan
- 중력 없는 Projectile
- Blocking Hit 있음
- Blocking Hit 없음
```

PASS:

```text
Weapon Reticle 위치
= 실제 첫 충돌 또는 최대 사거리 끝점
```

### 17.5 LaunchDirection

```text
- Heavy Cannon
- 중력 Projectile
- 터렛 정렬 중
- 터렛 정렬 완료
```

PASS:

```text
- 표시 모드 LaunchDirection
- 실제 초기 발사 방향과 일치
- 예상 탄착점으로 오인되는 표현 없음
```

### 17.6 UI 회귀

```text
- Ready
- FireSuccess
- Cooldown
- NoWeapon
- AimBlocked
- OutOfArcWarning
- TurretAligning
```

PASS:

```text
기존 Command Reticle의 색상, 문구와 전환이 변하지 않음
```

---

## 18. CF-FQ-025 완료 기준

```text
1. Command Reticle과 Weapon Reticle의 의미가 코드와 UI에서 분리된다.
2. Weapon Reticle은 FCFVehicleWeaponAimSolution.AimDirection을 단일 기준으로 사용한다.
3. Weapon Reticle 위치 계산을 위해 WBP가 별도 Trace를 수행하지 않는다.
4. 정렬 중 발사 허용 시 실제 발사 방향과 Weapon Reticle이 일치한다.
5. 정렬 완료 시 Weapon Reticle이 Command Reticle에 수렴한다.
6. 정렬 중 발사 금지 정책이 유지된다.
7. MuzzleBlocked 발사 차단이 유지된다.
8. DirectImpact의 Preview와 첫 충돌이 일치한다.
9. LaunchDirection이 실제 초기 발사 방향과 일치한다.
10. 기존 FireFeedback 회귀가 없다.
11. Optional WBP 바인딩 누락 시 크래시가 없다.
12. Tools\BuildEditor.bat가 성공한다.
13. 사용자 싱글 PIE를 통과한다.
```

Phase 2~4와 사용자 PIE가 끝나기 전에는 `CF-FQ-025 Done` 또는 `Current System`으로 승격하지 않는다.

---

## 19. 보호 범위

이번 구현에서 변경하지 않는 계약:

```text
- AimTargetLocation은 Command Reticle 목표점이다.
- AimDirection은 실제 최종 발사 방향이다.
- HitScan과 Projectile은 같은 FireRequest.AimDirection을 사용한다.
- bAllowFireWhileAligning=true이면 정렬 중 현재 Muzzle 방향으로 발사한다.
- bAllowFireWhileAligning=false이면 정렬 완료 전 발사를 거부한다.
- MuzzleBlocked는 정책과 관계없이 발사를 거부한다.
- OutOfArc는 현재 단독 발사 차단 조건이 아니다.
- AimReticle은 판정을 계산하지 않고 결과를 표시한다.
- 기존 FireFeedback 상태와 WBP 바인딩 이름을 유지한다.
```

이번 단계에서 함께 구현하지 않는 항목:

```text
- Ballistic Solver
- Ballistic Impact Reticle
- Lead Indicator
- 자동 락온
- Aim Assist
- 다중 터렛 동시 Reticle
- 네트워크 지연 보정
- 완성형 전투 HUD 전체 재설계
```

---

## 20. 주요 리스크와 대응

### 20.1 DPI와 화면 좌표 불일치

위험:

```text
World To Screen 결과와 Canvas 위치가 다른 스케일을 사용해 Weapon Reticle이 어긋남
```

대응:

```text
- PlayerViewportRelative 사용 여부 명시
- Canvas Geometry와 DPI Scale 확인
- 초광폭 해상도 포함 테스트
```

### 20.2 UI 보간으로 실제 방향 지연

위험:

```text
Weapon Reticle이 보기에는 부드럽지만 실제 발사 방향보다 늦게 움직임
```

대응:

```text
- 첫 구현 보간 없음
- 후속 보간은 시각 계층에서만 적용
- 발사 판정에 보간값 사용 금지
```

### 20.3 Reticle 두 개로 화면 복잡도 증가

대응:

```text
- Weapon Reticle 크기를 작게 유지
- 낮은 Opacity 사용
- 추가 텍스트보다 위치 정보 중심
- 정렬 완료 Fade 여부를 사용자 PIE로 결정
```

### 20.4 DirectImpact와 Projectile 실제 충돌 차이

원인 후보:

```text
- Projectile Collision Radius
- Sphere Sweep과 Line Trace 차이
- 이동 대상
- 중력
```

대응:

```text
- 중력 Projectile은 LaunchDirection으로 분리
- 필요 시 Projectile CollisionRadius 기반 Sweep Preview를 후속 도입
- 이동 목표는 Lead 단계로 분리
```

### 20.5 문서 상태 불일치

현재 확인:

```text
ActiveWork v1.15
= LaunchDirection 사용자 PIE PASS

ImplementationDesign v0.2.5 일부 체크포인트
= 사용자 검증 진행 중인 이전 문구 잔존
```

대응:

```text
- 구현 착수 전 대표 Plan 체크포인트를 최신 PASS 상태로 동기화
- 이 보고서는 최신 ActiveWork 상태를 사용
```

---

## 21. 남은 제품·UX 결정

Phase 2 데이터 정확성 검증 후 사용자 PIE로 정할 항목:

```text
- Weapon Reticle 항상 표시 vs 정렬 중에만 표시
- 정렬 완료 시 유지 vs Fade Out
- DirectImpact와 LaunchDirection을 같은 모양으로 표시할지 여부
- MuzzleBlocked 전용 아이콘 필요 여부
- 화면 밖 숨김 vs 가장자리 Clamp
- 상태색 공유 vs Weapon Reticle 전용 중립색
- 정렬 완료 판정에 Hysteresis를 적용할지 여부
```

현재 기술 결정:

```text
- Weapon Reticle은 AimDirection을 기준으로 한다.
- Preview 데이터는 FCFVehicleWeaponAimSolution에 유지한다.
- 중력 Projectile은 LaunchDirection으로 표시한다.
- 예상 탄착은 후속 BallisticImpact로 분리한다.
- WBP는 계산하지 않고 표현만 담당한다.
```

---

## 22. 권장 구현 순서

```text
1. 대표 Plan의 LaunchDirection 사용자 PIE 상태를 PASS로 동기화한다.
2. Phase 2 TaskSource와 최종 Codex 작업지시서를 준비한다.
3. UCFAimReticleWidget에 Weapon Preview 화면 투영을 구현한다.
4. Image_WeaponReticle Optional 바인딩을 추가한다.
5. WBP_AimReticle에 최소 시각 자산을 배치한다.
6. 보간 없이 실제 방향 정확성을 검증한다.
7. DirectImpact와 LaunchDirection을 각각 PIE 검증한다.
8. 정렬 허용·금지와 MuzzleBlocked 회귀를 검증한다.
9. 초광폭·DPI·화면 경계 회귀를 수행한다.
10. 시각 복잡도를 평가해 Fade 또는 Clamp 정책을 결정한다.
11. CF-FQ-025 완료 후 Systems 문서를 갱신한다.
12. 이후 CF-FQ-024 전투 FX 구현으로 복귀한다.
```

---

## 23. 결론

CarFight의 현재 레티클은 단순한 화면 중앙 조준점이 아니다.
현재 시스템은 플레이어의 조준 의도, 터렛 정렬 상태, 총구 장애물, 무기 유무, 쿨다운과 발사 결과를 통합해 표시하는 `Command Reticle + FireFeedback` 구조다.

조준 판정과 실제 발사 방향의 기반은 이미 신뢰 가능한 상태다.
`FCFVehicleWeaponAimSolution.AimDirection`이 HitScan과 Projectile의 실제 최종 발사 방향을 통일하며, 정렬 완료 탄착과 정렬 중 발사 정책도 사용자 PIE로 검증됐다.

`CF-FQ-025`의 World Preview 데이터도 구현과 검증을 통과했다.
현재 남은 핵심은 새로운 방향 계산이 아니라, 이미 확정된 실제 방향 데이터를 정확하게 화면에 표시하는 것이다.

따라서 다음 구현 목표는 명확하다.

> 현재 중앙 Command Reticle을 유지하면서, 실제 최종 발사 방향의 Weapon Preview 위치를 별도의 Weapon Reticle로 화면에 표시한다.

Ballistic Impact Reticle과 Lead Indicator는 이 이중 레티클 구조가 검증된 뒤 독립 단계로 확장한다.

---

## 24. Migration

### v1.0.0 적용 안내

```text
- 이 문서는 기존 Current System 문서인 Document/Systems/UI/AimReticle.md를 대체하지 않는다.
- 현재 구현 판정은 계속 Systems 문서를 우선한다.
- 미구현 단계와 구현 예정 기능은 이 보고서와 대표 Plan에서 관리한다.
- Weapon Preview World 데이터는 구현됐지만 Weapon Reticle HUD는 아직 미구현으로 해석한다.
- LaunchDirection은 예상 탄착점이 아니라 중력 Projectile의 실제 초기 발사 방향 표식이다.
- Phase 2~4와 사용자 PIE 완료 전에는 CF-FQ-025를 Done으로 해석하지 않는다.
```

---

## 25. Changelog

### v1.0.0 - 2026-07-16

```text
- 현재 Command Reticle, FireFeedback와 AimFireAlignment 구현 상태를 종합했다.
- Weapon Preview World 데이터와 ECFWeaponReticleMode 구현 상태를 실제 코드 검색 기준으로 반영했다.
- WBP_AimReticle 자산에 Image_WeaponReticle이 아직 없음을 자산 덤프와 코드 검색 기준으로 기록했다.
- ActiveWork v1.15의 Heavy Cannon LaunchDirection, MuzzleBlocked와 초기 발사 방향 사용자 PIE PASS를 최신 상태로 반영했다.
- Phase 2 World To Screen, Phase 3 WBP 디자인, Phase 4 회귀, Phase 5 Ballistic과 Phase 6 Lead 구현 계획을 정리했다.
- C++ / WBP / DataAsset 책임과 보호 범위, 테스트, 완료 기준과 리스크를 정의했다.
```

---

## 26. 확인 근거

### 현재 구현 문서

```text
Document/Systems/UI/AimReticle.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/FireFeedback.md
Document/Systems/Combat/Projectile.md
Document/Systems/UI/VehicleDebugPanel.md
```

### 현재 계획과 우선순위

```text
Document/Plan/ReticleAimDirection/ImplementationDesign.md
Document/ActiveWork.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/ProjectSSOT/CombatPlan/05_AimingSystem.md
Document/ProjectSSOT/CombatPlan/14_CombatUI.md
```

### 실제 코드

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

### UE 자산

```text
/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle
```

- 마지막 확인 일시: `2026-07-16`
- 현재 기준 브랜치: `Document/Plan` 저장소 `main`
