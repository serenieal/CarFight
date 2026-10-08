# CarFight — 레티클 및 사격방향 확장 설계

- Version: 0.3.4
- Date: 2026-07-22
- Status: Done / User PIE PASS / Official
- Feature ID: `CF-FQ-025 이중 레티클 및 사격방향 시각화`
- Priority: `Completed P0 / Archive Review`
- Scope: 사용자의 조준 레티클과 터렛의 실제 조준 레티클을 분리하고, 투사체 착탄 위치는 Reticle UI와 분리된 후속 3D 표시로 다루는 계약을 정의한다.

---

## Goal

```text
Image_CenterDot을 사용자의 조준 레티클로, Image_WeaponReticle을 터렛이 현재 조준하는 3D 지점의 화면 투영인 터렛 레티클로 고정하고 두 표시를 탄종과 착탄 결과에서 분리한다.
```

## In Scope

- Image_CenterDot 기반 사용자 조준 레티클
- Image_WeaponReticle 기반 탄종 독립 터렛 레티클
- 사용자가 선택한 3D 조준점과 터렛이 현재 조준하는 3D 지점의 분리
- C++ World To Screen 투영과 Optional WBP 바인딩
- 정렬 중 발사 허용/금지, MuzzleBlocked, 이동 상태 회귀 검증
- 후속 3D 착탄 위치 표시와 현재 Reticle UI의 책임 경계 정의

## Out of Scope

- 투사체 착탄 위치의 계산과 3D 표시
- 중력 Projectile Ballistic Solver
- 이동 목표 Lead Indicator
- 자동 락온과 Aim Assist
- 다중 터렛 동시 Reticle
- 네트워크 지연 보정
- 기존 WeaponFire 판정 정책 변경
- 완성형 전투 HUD 전체 재설계

## Constraints

```text
- AimTargetLocation은 Image_CenterDot이 지정한 사용자 조준점 의미를 유지한다.
- Image_WeaponReticle은 CurrentMuzzleDirection 기반 터렛 조준 지점을 표시하며 AimDirection 또는 탄착 결과를 표시하지 않는다.
- 터렛 레티클의 의미와 표시 여부는 HitScan/Projectile, 중력 적용 여부와 무관하다.
- 투사체 착탄 위치는 Reticle enum이나 Image_WeaponReticle에 합치지 않고 후속 3D 표시로 분리한다.
- HitScan과 Projectile은 같은 FireRequest.AimDirection을 사용한다.
- WBP는 Trace, 방향 계산과 발사 가능 판정을 수행하지 않는다.
- 기존 WBP 바인딩과 FireFeedback 상태를 깨지 않는다.
- 기존 DirectImpact/LaunchDirection 분기는 구현 이력으로만 유지하고 최종 터렛 레티클 의미로 사용하지 않는다.
- 신규 또는 수정 파일명은 32자를 넘지 않는다.
- CF-FQ-025 완료 후 확정한 두 Reticle 계약을 회귀 보호하고, 신규 기능은 별도 Feature 범위로 분리한다.
```

## Target Files

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
/Game/CarFight/UI/WBP_AimReticle
```

## Verification

```text
- Tools\BuildEditor.bat PASS
- Image_CenterDot이 지정한 3D 조준점을 터렛이 추적
- Image_WeaponReticle이 CurrentMuzzleDirection 기반 터렛 조준 지점을 탄종과 무관하게 표시
- 정렬 완료 시 조준 레티클과 터렛 레티클이 허용 오차 안에서 수렴
- MuzzleBlocked 발사 차단은 유지하되 터렛 레티클을 착탄 위치로 해석하지 않음
- HitScan/Projectile 전환 시 터렛 레티클 의미와 계산 경로가 바뀌지 않음
- 기존 Ready → FireSuccess → Cooldown → Ready 회귀 PASS
- 사용자 싱글 PIE PASS
```

---

## 현재 작업 체크포인트

- 현재 상태: LaunchDirection 사용자 PIE PASS 이후 Phase 2 Weapon Reticle C++ 화면 투영과 Optional WBP 바인딩 지원 빌드 검수가 완료됐다.
- 실행 방식: `TaskSource → Codex 실행 → Browser Diff/Build Review`
- Workflow Compliance: `PASS` — Browser Direct Edit 정책 위반분을 별도 TaskSource와 Codex 계약으로 검토·보정했다.
- 검토·보정 TaskSource: `Document/Plan/ReticleAimDirection/TaskSource_Phase1.md`
- 실행한 Codex 계약: `Document/Plan/ReticleAimDirection/Generated/Final/P1_WeaponPreviewReview.md`
- Codex 실행 결과: `PASS` — 사용자가 2026-07-16 14:08 KST 완료 결과를 전달했다.
- 브라우저 검수: `PASS` — 실제 scoped diff에서 DirectImpact 지원 조건, 공유 WeaponHit Trace, 기존 MuzzleBlocked 판정, Preview 네 필드 할당과 VehicleDebug 네 행을 확인했다.
- 유지한 직접 수정분: `CFVehicleAimTypes.h`의 Preview 필드 네 개는 TaskSource 계약과 일치해 그대로 유지됐다.
- Codex 수정 파일: `CFVehiclePawn.cpp`, `CFVehicleDebugPanelWidget.cpp`.
- 코드 상태: HitScan과 중력 없는 Projectile만 Preview가 유효하며, `CommandPathDistance`와 `PreviewMaxRange` 중 큰 거리로 수행한 단일 WeaponHit Trace를 MuzzleBlocked와 Preview가 공유한다.
- 보호 범위: `AimTargetLocation`, `AimDirection`, HitScan/Projectile 공용 FireRequest 방향, 정렬 중 발사 정책, MuzzleBlocked와 기존 WBP/Reticle 계약이 유지됐다.
- WBP 상태: 이번 Phase에서 변경 없음. `CFAimReticleWidget`과 `WBP_AimReticle`은 기존 dirty 상태를 유지했으며 Codex가 수정하지 않았다.
- 빌드 상태: Codex 보고 `Tools\BuildEditor.bat` PASS. 브라우저 독립 검증 `build_79aa1777becd65e5` PASS, `CarFight_ReEditor Win64 Development` Target Up To Date, Return Code 0.
- 정적 검증: PASS. `git diff --check`는 Codex 보고 기준 PASS이며 현재 Git diff 조회에서도 CRLF 변환 경고만 확인됐다.
- PIE 상태: `Scope Mismatch Confirmed / User Validation Blocked` — 2026-07-16 사용자 PIE에서 Preview 네 값이 계속 무효로 표시됐다.
- 에셋 조사: `HeavyCannon` 프리셋은 `DA_ProtoTurretCannon`을 사용하고, 이 WeaponData는 `FireMode=Projectile`, `DefaultProjectileData=DA_HeavyShell`이다. `DA_HeavyShell.bAffectedByGravity=true`이므로 현재 Phase 1 DirectImpact 조건에서 Preview 무효는 코드 계약상 정상이다.
- 문제 분류: 런타임 계산 고장보다 현재 실사용 무기와 Phase 1 지원 범위가 맞지 않는 설계·검증 범위 문제다. 현재 상태로 Phase 2를 진행하면 Heavy Cannon에서는 Weapon Reticle이 계속 숨겨진다.
- 당시 공식 착수 상태: `CF-FQ-025` P0 Active와 구현순위 1위로 진행했다. 최신 상태는 Done / User PIE PASS다.
- 확정 결정: 사용자는 2026-07-16 중력 Projectile의 Weapon Reticle을 `LaunchDirection`으로 표시하고, 예상 탄착점은 후속 `BallisticImpact`로 분리하는 안을 승인했다.
- 구현 계약: `ECFWeaponReticleMode`는 이번 단계에서 `Hidden / DirectImpact / LaunchDirection`만 도입한다. `BallisticImpact` enum 값은 실제 Ballistic Solver 구현 시 추가한다.
- 실행 TaskSource: `Document/Plan/ReticleAimDirection/TaskSource_LaunchDirection.md`
- 다음 Codex 입력: `Document/Plan/ReticleAimDirection/Generated/Final/P1_LaunchDirection.md`
- LaunchDirection Codex 실행 결과: `PASS` — `CFVehicleAimTypes.h`, `CFVehiclePawn.cpp`, `CFVehicleDebugPanelWidget.cpp` 세 허용 파일에 구현했다.
- LaunchDirection 코드 상태: `ECFWeaponReticleMode`는 `Hidden / DirectImpact / LaunchDirection`만 제공하고, 중력 Projectile은 `LaunchDirection`으로 `AimOrigin + AimDirection * PreviewMaxRange` 위치를 제공한다.
- LaunchDirection 보호 범위: DirectImpact 공유 WeaponHit Trace, `TargetSurfaceTolerance=10.0f`, CommandPathDistance 기준 MuzzleBlocked, 기존 FireRequest AimDirection과 정렬 중 발사 정책을 유지했다.
- LaunchDirection 빌드 상태: `Tools\BuildEditor.bat` PASS, Return Code 0, `CarFight_ReEditor Win64 Development`, Result Succeeded.
- LaunchDirection PIE 상태: `PASS` — 사용자 PIE에서 Heavy Cannon 회전 시 `Mode=LaunchDirection`, `Preview Valid=Yes`, `Preview Blocking Hit=No`, `Preview Distance=10000.0`, `Preview World Location` 갱신을 확인했고, MuzzleBlocked 회귀와 실제 중력탄 초기 발사 방향 일치도 정상 확인했다.
- Phase 2 코드 상태: `PASS / PIE Pending` — `UCFAimReticleWidget`이 `WeaponPreviewWorldLocation`을 `ProjectWorldLocationToWidgetPosition`으로 투영하고, 선택적 `Image_WeaponReticle`이 있으면 화면 안에서만 표시하도록 확장했다.
- Phase 2 빌드 상태: `Tools\BuildEditor.bat` PASS, Return Code 0, `CarFight_ReEditor Win64 Development`, Result Succeeded.
- Phase 2 WBP 상태: `User Validation Pending` — `WBP_AimReticle`에 `Image_WeaponReticle` 바인딩 이미지가 있어야 실제 화면 표시가 보인다. 바인딩이 없으면 기존 Command Reticle과 FireFeedback은 그대로 동작한다.
- Phase 2 초기 PIE 결과: `Partial PASS / 보정 필요` — Optional 바인딩과 화면 표시는 확인했으나 Canvas Slot 앵커 기준, 최대 사거리 깊이의 LaunchDirection 투영 시차와 같은 목표 Actor의 MuzzleBlocked 오판을 확인했다.
- Phase 2 보정 코드 상태: `PASS / PIE Pending` — Canvas Slot 앵커를 좌측 상단으로 고정하고, LaunchDirection Preview를 `AimOrigin + AimDirection * CommandPathDistance`로 변경해 Command 목표와 같은 깊이에서 비교한다.
- MuzzleBlocked 보정 상태: Camera Aim Trace의 `AimTraceHitActor`를 보존하고 총구 Trace가 같은 Actor를 적중하면 목표 표면으로 허용한다. 다른 Actor가 목표보다 앞을 막는 기존 차단은 유지한다.
- Phase 2 보정 빌드 상태: `Tools\BuildEditor.bat` PASS, Return Code 0, `CarFight_ReEditor Win64 Development`, Result Succeeded.
- 2026-07-21 사용자 확정 구조: `Image_CenterDot=조준 레티클`, `Image_WeaponReticle=터렛 레티클`, 투사체 착탄 위치=`Reticle과 분리된 후속 3D 표시`로 고정했다.
- Image_CenterDot 상태: `User PIE PASS` — 사용자가 현재 조준 레티클 작동에 문제가 없음을 확인했다. 이후 구현에서는 변경 대상이 아니라 회귀 보호 대상으로 취급한다.
- Superseded: Weapon Reticle을 `AimDirection`, `DirectImpact`, `LaunchDirection`, 첫 충돌 또는 예상 착탄 위치로 해석하던 이전 계약은 최신 UI 의미 계약에서 대체됐다.
- 터렛 레티클 코드 정렬: `PASS` — `bHasValidTurretReticlePoint`, `TurretReticleWorldLocation`, `TurretReticleDistance`를 추가하고 `Image_WeaponReticle`이 이 값만 소비하도록 변경했다.
- Legacy 처리: 기존 `ECFWeaponReticleMode`와 `WeaponPreviewWorldLocation`은 UI 소비에서 제외하고 과거 구현 확인용 Debug로만 보존했다.
- 빌드 상태: `Tools\BuildEditor.bat` PASS, Return Code 0, `CarFight_ReEditor Win64 Development`, Result Succeeded.
- 최종 사용자 PIE: `PASS` — 사용자가 2026-07-21 계획한 터렛 레티클 구조와 동작이 정상 구현됐음을 확인했다.
- 완료 범위: Image_CenterDot 회귀 보호, CurrentMuzzleDirection 기반 Image_WeaponReticle, 탄종·착탄 위치 분리, Debug 검증과 공식 빌드.
- 다음 설계 주제: Reticle UI와 분리된 투사체 착탄 위치의 월드 공간 3D 표현을 사용자와 논의한다.

### 이번 문서에서 확정한 구조 계약

```text
1. Image_CenterDot은 사용자가 화면 내 원하는 위치를 지정하는 조준 레티클이다.
2. 조준 레티클이 선택한 화면 위치는 Camera Aim Trace를 통해 단일 3D 조준점으로 해석한다.
3. 터렛은 그 3D 조준점을 추적한다.
4. Image_WeaponReticle은 터렛이 현재 조준하는 3D 지점을 사용자 화면에 투영한 터렛 레티클이다.
5. 터렛 레티클은 탄종, FireMode, 중력 적용 여부와 착탄 결과를 입력으로 사용하지 않는다.
6. 투사체 착탄 위치 표시는 Reticle UI와 분리된 월드 공간 3D 표현으로 후속 논의·구현한다.
7. UI가 별도 Trace, 터렛 방향, 탄도 또는 발사 가능 판정을 임의 계산하지 않는다.
8. 기존 WeaponFire 판정과 실제 발사 방향 계약은 UI 의미 변경과 분리해 유지한다.
```

### 바로 다음 작업

```text
1. CF-FQ-025 터렛 레티클 구현은 완료 상태로 유지한다.
2. Image_CenterDot과 Image_WeaponReticle의 확정 의미를 후속 기능에서 변경하지 않는다.
3. 투사체 착탄 위치의 계산 범위, 표시 형태와 수명 정책을 사용자와 별도 논의한다.
4. 착탄 위치 기능은 Reticle enum이나 Image_WeaponReticle에 합치지 않고 별도 Feature로 등록한다.
```

### 사용자 PIE 기대값

```text
- 터렛 레티클 유효 여부: 정상 조준 중 예
- 터렛 레티클 비교 거리: AimOrigin → AimTargetLocation 거리와 동일하며 조준 대상 거리에 따라 변함
- 정렬 중: Image_CenterDot은 사용자 조준 위치를 유지하고 Image_WeaponReticle은 실제 터렛 회전을 따라 지연 이동
- 정렬 완료: 터렛 가동 범위 안에서는 두 레티클이 허용 오차 안에서 겹침
- 바닥/상하 한계 조준: 터렛 Pitch 한계에 걸리면 Image_WeaponReticle이 Image_CenterDot과 떨어져 있어도 정상
- 탄종 변경: HitScan/Projectile 및 중력 여부가 바뀌어도 Image_WeaponReticle의 의미와 계산 경로는 동일
- 착탄 위치: Image_WeaponReticle 위치와 일치할 필요가 없으며 이번 검증 대상이 아님
- Image_CenterDot 및 기존 FireFeedback: 변경 전과 동일
```

---

## 1. 문서 목적

현재 CarFight에는 사용자 조준점, 터렛 추적, Muzzle 기준 요구 방향과 실제 발사 방향을 하나의 `FCFVehicleWeaponAimSolution`으로 연결한 P0 기반이 있다.

현재 구현은 판정 신뢰성 측면에서는 다음을 달성했다.

```text
- 사용자 조준점과 정렬 완료 후 실제 발사 방향 일치
- 정렬 중 발사 허용 정책에 따라 현재 Muzzle 방향으로 발사
- 정렬 중 발사 금지 정책에 따라 정렬 완료 전 발사 거부
- MuzzleBlocked 발생 시 정책과 관계없이 발사 차단
- HitScan과 Projectile의 최종 발사 방향 통일
```

하지만 사용자가 지정한 조준점과 터렛이 현재 바라보는 지점은 서로 다른 UI 책임이다.
정렬 중에는 두 지점이 분리될 수 있으므로 플레이어가 터렛의 실제 추적 상태를 화면에서 직접 읽을 수 있어야 한다.

이 문서의 목적은 다음 질문에 대한 구현 가능한 답을 정하는 것이다.

> 플레이어가 지정한 조준 위치와 터렛이 현재 조준하는 3D 지점을 어떻게 동시에 표시할 것인가?

---

## 2. 상위 설계 근거

CarFight의 장기 조준 설계는 이미 세 가지 조준 요소를 분리하도록 정의한다.

```text
조준 레티클
= Image_CenterDot으로 사용자가 화면에서 직접 지정하는 위치

터렛 레티클
= Image_WeaponReticle로 터렛이 현재 조준하는 3D 지점을 화면에 투영한 위치

투사체 착탄 위치 표시
= 탄도 계산 결과를 Reticle UI와 분리해 월드 공간에 3D로 표현하는 후속 기능
```

관련 상위 기준:

```text
Document/ProjectSSOT/CombatPlan/05_AimingSystem.md
Document/ProjectSSOT/CombatPlan/14_CombatUI.md
Document/ProjectSSOT/00_Vision.md
```

현재 Systems와 완료 Plan 기준:

```text
Document/Systems/UI/AimReticle.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/Projectile.md
Document/Plan/AimFireAlignment/ImplementationDesign.md
```

이 문서는 현재 Aim Solution 위에 조준 레티클과 터렛 레티클의 책임을 고정하고, 투사체 착탄 위치 표시를 별도 후속 기능으로 분리하는 세부 구현 계획이다.

---

## 3. 현재 구현 기준선

### 3.1 조준 레티클 (Aim Reticle)

`Image_CenterDot`은 플레이어의 조준 의도를 나타낸다. 사용자는 화면 내 원하는 위치를 이 레티클로 지정한다.

```text
조준 레티클
= Camera Aim Trace가 선택한 AimTargetLocation
= 플레이어가 무기에게 향하라고 명령하는 월드 위치
```

현재 조준 레티클은 아래 상태를 함께 표시한다.

```text
Ready
Blocked
OutOfArc
TurretAligning
NoWeapon
Cooldown
FireRejected
```

### 3.2 Weapon Aim Solution

현재 `FCFVehicleWeaponAimSolution`은 다음 핵심 값을 가진다.

```text
AimOrigin
AimDirection
DesiredAimDirection
CurrentMuzzleDirection
AimTargetLocation
WeaponAlignmentErrorDeg
bTurretAligning
bWeaponNotAligned
bAllowFireWhileAligning
bMuzzleBlocked
```

방향 의미:

```text
DesiredAimDirection
= Muzzle → 조준 레티클 목표점 요구 방향

CurrentMuzzleDirection
= 현재 Muzzle Socket X축 방향

AimDirection
= 지금 발사 입력이 승인됐을 때 실제로 사용할 최종 발사 방향
```

### 3.3 실제 최종 발사 방향 정책

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

발사 정책의 `AimDirection`과 터렛 레티클의 표시 기준은 분리한다.
`Image_WeaponReticle`은 발사 승인 여부나 최종 Fire Command가 아니라 `CurrentMuzzleDirection`으로 표현되는 터렛의 현재 조준 상태를 표시한다.

---

## 4. 현재 사용자 경험 문제

### 4.1 정렬 중 발사 방향을 알기 어렵다

`bAllowFireWhileAligning` 값과 관계없이 조준 레티클은 사용자 목표를, 터렛 레티클은 터렛의 현재 조준 지점을 표시한다.

현재 UI만 보면 플레이어는 다음 차이를 직관적으로 구분하기 어렵다.

```text
사용자가 조준한 위치
vs
터렛이 현재 조준하는 위치
```

### 4.2 발사 실패와 정상적인 무기 지연이 혼동될 수 있다

터렛 각속도나 안정화 지연은 버그나 입력 지연이 아니라 무기 성능의 일부다.
터렛 레티클이 없으면 플레이어는 정상적인 추적 지연을 불공정한 판정으로 느낄 수 있다.

### 4.3 고정·짐벌·터렛 무기의 차이를 UI로 읽기 어렵다

CarFight의 장기 전투 목표는 고정형, 짐벌형, 터렛형 무장의 조작 감각을 구분하는 것이다.
실제 무기 방향을 보여주지 않으면 각속도, 제한각, 안정화라는 밸런스 스탯이 화면에서 충분히 체감되지 않는다.

### 4.4 투사체 착탄 위치는 Reticle UI와 분리해야 한다

현재 ProjectileData는 속도와 중력 설정을 보유한다.
하지만 중력이 적용되는 Projectile은 단순 직선 방향의 끝점과 실제 탄착점이 일치하지 않는다.

따라서 다음 책임을 분리한다.

```text
터렛 레티클
= 터렛이 현재 조준하는 3D 지점의 2D 화면 투영

3D 착탄 위치 표시
= 투사체의 예상 또는 실제 착탄 위치를 월드 공간에 표현하는 별도 기능
```

터렛 레티클을 탄도, 직선 Trace 충돌 또는 예상 착탄점으로 해석해서는 안 된다.

---

## 5. 핵심 용어와 UI 역할

### 5.1 조준 레티클 (Aim Reticle)

```text
위젯: Image_CenterDot
역할: 플레이어가 화면 내에서 지정한 조준 명령 표시
기준: AimTargetLocation
위치: 사용자가 화면 내에서 지정한 위치
주요 상태: Ready / TurretAligning / Blocked / OutOfArc / Cooldown
```

`Image_CenterDot`과 이를 둘러싼 4방향 브라켓은 조준 레티클의 시각 요소로 사용한다.

### 5.2 터렛 레티클 (Turret Reticle)

```text
위젯: Image_WeaponReticle
역할: 터렛이 현재 조준하는 3D 지점 표시
방향 기준: CurrentMuzzleDirection
깊이 기준: 조준 레티클이 선택한 3D 목표점과 같은 비교 깊이
표시 위치: 터렛 조준 3D 지점을 사용자 2D 화면에 투영
탄종 의존성: 없음
착탄 의미: 없음
```

터렛 레티클은 조준 레티클보다 작고 시각적 우선순위가 낮아야 한다.

권장 모양:

```text
- 작은 빈 원 또는 작은 십자
- 중앙이 비어 있어 조준 레티클과 겹쳤을 때 식별 가능
- 정렬 중에는 amber 또는 중립 색상
- 정렬 완료 시 조준 레티클과 겹치며 시각적으로 결합
```

### 5.3 Superseded — Weapon Preview Impact

```text
상태: 최신 터렛 레티클 의미 계약에서 대체됨
이유: 첫 충돌 또는 최대 사거리 끝점은 터렛의 조준 상태와 탄착 결과를 혼합한다.
```

기존 `DirectImpact` Preview는 구현 이력과 Debug 호환을 위해 남을 수 있지만 `Image_WeaponReticle`의 제품 의미로 사용하지 않는다.

### 5.4 후속 3D 착탄 위치 표시

```text
역할: 투사체 방식 탄종의 예상 또는 실제 착탄 위치를 월드 공간에 표시
표현: Reticle UI가 아닌 별도 3D 시각 요소
입력 후보: 초기 속도, 중력, 발사 위치, 목표 위치를 사용한 탄도 해
```

터렛 레티클 완료 후 사용자와 표현 방식과 계산 범위를 별도 논의한 뒤 구현한다.

### 5.5 Lead Indicator

```text
역할: 이동 목표의 미래 위치를 고려한 선행 조준 위치 표시
기준: 목표 속도, 상대 속도, Projectile 비행시간
```

후속 3D 착탄 위치 표시와 대상 예측 계약 확정 이후 별도 기능으로 둔다.

---

## 6. 권장 최종 UX

### 6.1 기본 화면

```text
조준 레티클
= 사용자가 화면 내에서 지정한 위치에 표시

터렛 레티클
= 터렛이 현재 조준하는 3D 지점의 화면상 위치에 표시
```

두 Reticle의 관계가 곧 현재 터렛 정렬 상태를 보여준다.

```text
두 Reticle이 멀리 떨어짐
→ 무기가 아직 따라오지 못함

두 Reticle이 가까워짐
→ 무기가 목표에 접근 중

두 Reticle이 겹침
→ 무기가 명령 방향에 정렬됨
```

### 6.2 정렬 중 발사 허용 터렛

```text
조준 레티클
= 플레이어 목표 유지

터렛 레티클
= CurrentMuzzleDirection 기반 현재 터렛 조준 지점 표시

발사
= 기존 bAllowFireWhileAligning 정책에 따른 AimDirection으로 진행
```

플레이어는 정렬을 기다릴지, 현재 방향으로 선제 발사할지 선택할 수 있다.

### 6.3 정렬 중 발사 금지 터렛

```text
조준 레티클
= 목표 유지

터렛 레티클
= 현재 터렛 조준 지점 표시

발사
= 두 Reticle이 정렬 기준 안에 들어오기 전 거부
```

이 경우 터렛 레티클은 단순 장식이 아니라 남은 추적 상태를 알려주는 핵심 정보다.

### 6.4 MuzzleBlocked

권장 표시:

```text
- 조준 레티클은 기존 Blocked 주황 상태 유지
- 터렛 레티클 위치는 CurrentMuzzleDirection 기반 조준 지점을 유지
- 필요하면 터렛 레티클에 짧은 주황 Pulse 또는 막힘 아이콘 표시
- MuzzleBlocked Trace의 첫 Hit 위치를 터렛 레티클 위치로 사용하지 않음
```

### 6.5 OutOfArc

```text
- 조준 레티클은 기존 OutOfArc 경고 유지
- 터렛 레티클은 현재 유효한 터렛 방향이 있으면 계속 표시
- OutOfArc 자체를 신규 기능에서 자동 발사 차단 조건으로 승격하지 않음
```

### 6.6 정렬 완료

초기 구현 권장:

```text
- 터렛 레티클을 완전히 숨기지 않는다.
- 정렬 완료 시 낮은 Opacity로 조준 레티클 안에 겹쳐 보이게 한다.
- PIE 후 겹침 상태가 지나치게 복잡하면 Fade 정책을 적용한다.
```

항상 표시를 먼저 권장하는 이유:

```text
- 기능 검증이 쉽다.
- 터렛의 현재 조준 방향이 지속적으로 보장되는지 확인할 수 있다.
- 정렬 완료 시 갑자기 사라져 발생하는 Flicker를 피할 수 있다.
```

후속 UX 튜닝 후보:

```text
ShowThresholdDeg = 터렛 레티클을 선명하게 표시할 오차
HideThresholdDeg = 정렬 완료 Fade 기준

ShowThresholdDeg > HideThresholdDeg
→ Hysteresis로 경계 Flicker 방지
```

---

## 7. 데이터 흐름 설계

### 7.1 단일 기준 원칙

```text
Image_CenterDot 화면 위치
→ Camera Aim Trace
→ AimTargetLocation
→ 터렛이 AimTargetLocation 추적
→ CurrentMuzzleDirection 기준 터렛 조준 3D 지점 계산
→ World To Screen 투영
→ Image_WeaponReticle 표시
```

금지 구조:

```text
WBP가 Camera Forward 또는 Muzzle Forward를 다시 계산
WBP가 자체 Line Trace 수행
터렛 레티클을 탄종, FireMode, 중력 또는 충돌 Preview로 분기
터렛 레티클에 예상/실제 착탄 의미 부여
UI 표시를 맞추기 위해 FireRequest.AimDirection 변경
```

### 7.2 터렛 조준 3D 지점 계산

탄종 공통 기준:

```text
TurretAimStart = WeaponAimSolution.AimOrigin
TurretAimDirection = WeaponAimSolution.CurrentMuzzleDirection
ComparisonDepth = AimOrigin에서 AimTargetLocation까지의 거리
TurretAimWorldPoint = TurretAimStart + TurretAimDirection * ComparisonDepth
```

이 계산은 HitScan/Projectile, 중력 적용 여부, Trace 첫 충돌과 무관하게 동일하다.
`MuzzleBlocked` 판정용 Trace 결과는 발사 가능 상태에만 사용하고 터렛 레티클 위치를 바꾸지 않는다.

### 7.3 현재 MuzzleBlocked Trace와의 관계

현재 `BuildWeaponAimSolution()`은 Command Target까지의 거리만큼 실제 최종 발사 방향으로 obstruction Trace를 수행한다.

터렛 레티클은 MuzzleBlocked Trace 결과를 위치 입력으로 재사용하지 않는다.
MuzzleBlocked는 발사 차단 판정이고 터렛 레티클은 현재 터렛 조준 상태 표시다.

권장 구현:

```text
BuildWeaponAimSolution 내부에서 발사 차단과 터렛 조준 방향을 각각 확정
→ WeaponFire는 발사 방향과 차단 상태 소비
→ AimReticle은 AimTargetLocation과 CurrentMuzzleDirection 기반 터렛 조준 지점 소비
```

### 7.4 화면 투영

월드 좌표를 화면 좌표로 바꾸는 것은 `UCFAimReticleWidget` C++ 계층에서 수행한다.

```text
WeaponAimSolution의 터렛 조준 World 위치
→ PlayerController.ProjectWorldLocationToScreen
→ Image_WeaponReticle Canvas 위치 갱신
```

WBP Blueprint Graph에 월드 계산이나 Trace 로직을 넣지 않는다.

---

## 8. C++ 데이터 구조 확장안

### 8.1 현재 구조체의 의미 정렬

현재 `FCFVehicleWeaponAimSolution`의 Preview 필드는 기존 구현 이력과 호환을 위해 존재한다.

```text
bHasValidWeaponPreview
bWeaponPreviewHasBlockingHit
WeaponPreviewWorldLocation
WeaponPreviewDistance
```

최신 계약 적용 시 다음 중 하나로 정리한다.

```text
- 기존 필드명을 유지하되 Image_WeaponReticle 소비 경로에서는 탄종/충돌 의미를 제거한다.
- 또는 명확한 TurretAimWorldPoint 계열 필드를 추가하고 기존 Preview 필드는 Legacy Debug로 축소한다.
- 어떤 방식을 택해도 Image_WeaponReticle은 CurrentMuzzleDirection 기반 단일 경로만 소비한다.
```

### 8.2 Superseded — 탄종별 표시 모드

기존 구현에는 다음 enum이 도입돼 있다.

```text
ECFWeaponReticleMode

Hidden
DirectImpact
LaunchDirection
```

현재 상태:

```text
- `DirectImpact`와 `LaunchDirection`은 과거 Preview 구현과 Debug 의미로만 본다.
- Image_WeaponReticle의 터렛 레티클 의미는 이 enum으로 분기하지 않는다.
- 탄종 공통 터렛 레티클에 별도 모드가 필요하지 않으면 enum 의존을 제거한다.
```

`BallisticImpact`를 Reticle enum에 추가하지 않는다. 투사체 착탄 위치는 후속 3D 표시 기능에서 별도 타입과 렌더링 계약으로 논의한다.

### 8.3 파일 배치 권장

초기 구현에서는 기존 파일을 우선 사용한다.

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

표시 전용 타입이 커질 경우에만 신규 파일을 추가한다.

```text
CFVehicleReticleTypes.h
```

파일명은 32자를 넘지 않는다.

---

## 9. C++ / BP / DataAsset 책임 분리

### 9.1 C++ 책임

```text
- 실제 최종 발사 방향 선택
- 조준 레티클 화면 위치를 Camera Aim Trace의 3D 목표점으로 해석
- CurrentMuzzleDirection 기반 터렛 조준 3D 지점 계산
- World To Screen 투영
- 화면 밖 여부와 Clamp 계산
- Reticle 표시 상태와 수치 공급
- Debug Snapshot 제공
- WeaponFire와 UI가 같은 Aim Solution을 소비하되 발사 결과와 터렛 조준 표시 책임을 분리
```

### 9.2 WBP 책임

```text
- Image_CenterDot 조준 레티클과 Image_WeaponReticle 터렛 레티클 이미지 배치
- 크기, 색상, Opacity와 애니메이션
- 조준 레티클과 터렛 레티클의 시각적 계층 구분
- Aligning / Blocked / Ready 표현
- 화면 가장자리 방향 표시의 외형
```

WBP에서 수행하지 않는 것:

```text
- Trace
- 발사 방향 계산
- 각도 오차 계산
- 발사 가능 판정
- Projectile 탄도 계산
- 투사체 착탄 위치 3D 표시
```

### 9.3 DataAsset 책임

첫 구현에서 신규 DataAsset을 만들지 않는다.

기존 데이터 사용:

```text
UCFWeaponData.MaxRange
UCFTurretMountData.StabilizationToleranceDeg
UCFTurretMountData.bAllowFireWhileAligning
UCFProjectileData.InitialSpeed
UCFProjectileData.bAffectedByGravity
UCFProjectileData.GravityScale
```

UI 스타일 값은 우선 `UCFAimReticleWidget`의 EditDefaultsOnly 프로퍼티로 둔다.
무기별 시각 정책이 실제로 필요해진 뒤에 WeaponData 또는 별도 UI DataAsset으로 승격한다.

---

## 10. Widget 확장안

현재 `UCFAimReticleWidget`은 조준 레티클/터렛 레티클 이미지와 FireFeedback 텍스트를 관리한다.

신규 Optional 바인딩 후보:

```text
Image_WeaponReticle
Image_WeaponBlocked
Image_WeaponOffscreen
```

첫 구현 필수:

```text
Image_WeaponReticle
```

후속 선택:

```text
Image_WeaponBlocked
Image_WeaponOffscreen
```

권장 C++ 프로퍼티 후보:

```text
bShowWeaponReticle
WeaponReticleAlignedOpacity
WeaponReticleAligningOpacity
WeaponReticleBlockedOpacity
WeaponReticleScreenPadding
WeaponReticlePositionLerpSpeed
```

모든 WBP 바인딩은 기존 방식처럼 `BindWidgetOptional`을 사용한다.
신규 이미지가 아직 배치되지 않은 WBP에서도 크래시가 발생하지 않아야 한다.

### 10.1 위치 보간 정책

터렛 레티클을 지나치게 느리게 보간하면 실제 터렛 조준 방향과 UI가 어긋난다.

권장 원칙:

```text
- 판정 위치는 매 Tick 즉시 갱신한다.
- 시각 보간은 매우 짧게만 사용한다.
- 발사 순간에는 보간된 위치가 아니라 최신 실제 위치를 사용해 시각 피드백을 보정한다.
```

첫 구현은 보간 없이 정확한 위치를 우선 검증하는 편이 안전하다.
시각 떨림이 확인된 뒤 UI 전용 보간을 추가한다.

---

## 11. 무기 유형별 적용

### 11.1 고정 무기

```text
조준 레티클
= 플레이어가 바라보는 위치

터렛 레티클
= 차체 고정 무기 방향의 실제 위치
```

차체가 목표에 정렬되지 않으면 두 Reticle이 분리된다.
고정 무기의 높은 운전 의존도를 직접 보여준다.

### 11.2 짐벌 무기

```text
조준 레티클
= 플레이어 목표

터렛 레티클
= 제한각과 각속도로 보정된 현재 짐벌 방향
```

짐벌 제한각 밖에서는 터렛 레티클이 조준 레티클을 더 이상 따라가지 못하는 것을 보여줘야 한다.

### 11.3 터렛 무기

```text
조준 레티클
= 목표 명령

터렛 레티클
= 실제 터렛/총구 방향
```

현재 구현과 가장 직접적으로 연결되는 첫 적용 대상이다.

### 11.4 락온 무기

터렛 레티클만으로 락온을 표현하지 않는다.

```text
터렛 레티클
= 발사관 또는 현재 발사 방향

Lock Indicator
= 목표 선택과 락온 진행
```

락온 UI는 별도 시스템이다.

### 11.5 수동 유도 무기

발사 전 터렛 레티클과 발사 후 유도 조준 UI를 분리한다.
이번 범위에서 제외한다.

---

## 12. 중력 Projectile 단계 분리

### 12.1 Superseded — DirectImpact와 LaunchDirection Reticle 분기

아래 분기는 기존 구현 이력이다.

```text
HitScan
또는
ProjectileData.bAffectedByGravity == false
```

중력 Projectile은 `LaunchDirection`으로 분류했다.

```text
LaunchDirection World Point
= AimOrigin + AimDirection * PreviewDistance
```

- 최신 계약에서는 Image_WeaponReticle을 이 분기로 제어하지 않는다.
- 터렛 레티클은 모든 탄종에서 CurrentMuzzleDirection 기반으로 동일하게 표시한다.
- DirectImpact/LaunchDirection은 코드 정렬 전 Legacy Preview/Debug 상태다.

### 12.2 투사체 착탄 위치 3D 표시 후속 입력

```text
MuzzleWorldLocation
DesiredAimTargetLocation
ProjectileInitialSpeed
GravityScale
WorldGravity
```

후속 출력 후보:

```text
bHasBallisticSolution
BallisticLaunchDirection
BallisticFlightTimeSeconds
BallisticImpactLocation
bUsesHighArcSolution
```

### 12.3 투사체 착탄 표시 정책

```text
조준 레티클
= 플레이어가 지정한 목표점

터렛 레티클
= 터렛이 현재 조준하는 3D 지점의 화면 투영

3D 착탄 위치 표시
= 예상 또는 실제 착탄 위치의 월드 공간 표현
```

투사체 무기에서는 세 위치가 서로 다를 수 있다.
3D 착탄 위치 표시는 터렛 레티클 완료 후 사용자 논의를 거쳐 별도 기능으로 착수한다.

---

## 13. Lead Indicator 후속 단계

Lead Indicator는 조준 레티클이나 터렛 레티클의 대체물이 아니다.

```text
조준 레티클
= 현재 플레이어 입력

터렛 레티클
= 현재 무기 방향

Lead Indicator
= 움직이는 목표를 맞히기 위해 조준해야 하는 미래 위치
```

필요 입력:

```text
TargetWorldLocation
TargetVelocity
ShooterVelocity
ProjectileInitialSpeed
Gravity
```

착수 조건:

```text
- 대상 선택 또는 락온 데이터가 존재함
- Projectile 비행시간 계산이 신뢰 가능함
- 조준 레티클과 터렛 레티클 구현이 먼저 완료됨
- 투사체 착탄 위치 3D 표시의 입력 계약이 별도로 확정됨
```

---

## 14. 화면 밖 처리

Weapon Preview 위치가 화면 밖이거나 카메라 뒤에 있을 수 있다.

단계별 권장:

### Phase 1

```text
- 화면 안의 터렛 레티클만 표시
- 화면 밖이면 숨김
- Debug Panel에서 OnScreen 여부 확인
```

### Phase 2

```text
- 화면 가장자리로 Clamp
- Weapon 방향을 나타내는 작은 Chevron 표시
- 카메라 뒤쪽이면 별도 BehindCamera 상태 사용
```

첫 구현에 화면 가장자리 UI까지 묶으면 검증 범위가 커진다.
따라서 Phase 1에서 실제 방향 데이터의 정확성을 먼저 확인한다.

---

## 15. Debug 확장안

VehicleDebug의 Weapon Aim Solution 섹션에 아래 항목을 추가한다.

```text
Legacy Weapon Reticle Mode
Weapon Preview Valid
Weapon Preview Blocking Hit
Weapon Preview World Location
Weapon Preview Distance
Turret Reticle On Screen
Turret Reticle Screen Position
```

Debug Draw 후보:

```text
Command Target Line
= AimOrigin → AimTargetLocation

Final Fire Line
= AimOrigin → WeaponPreviewWorldLocation

Current Muzzle Line
= AimOrigin → CurrentMuzzleDirection
```

Debug 색상은 구현 시 기존 VehicleDebug 색상 규칙을 따른다.
실제 사용자 HUD 색상과 Debug Draw 색상을 같은 데이터로 강제하지 않는다.

---

## 16. 구현 단계

### Phase 0 — 기준선 회귀 확인

```text
1. 현재 Image_CenterDot 조준 레티클 / FireFeedback 회귀 확인
2. 정책 true/false의 AimDirection 의미 재확인
3. MuzzleBlocked Trace의 거리와 Ignore 목록 확인
4. HitScan / Projectile이 FireRequest.AimDirection을 공유하는지 재확인
```

완료 조건:

```text
기존 CF-FQ-022 계약을 변경하지 않고 신규 데이터를 추가할 수 있음
```

### Phase 1 — Legacy Weapon Preview World 데이터

```text
1. FCFVehicleWeaponAimSolution에 Preview 필드 추가
2. BuildWeaponAimSolution에서 실제 최종 발사 방향 Preview 계산
3. 활성 무기 MaxRange 사용
4. WeaponHit Trace와 현재 Ignore 정책 재사용
5. 직선 사격만 DirectImpact로 분류
6. VehicleDebug에 World 데이터 표시
```

완료 조건:

```text
Debug Line의 끝점과 실제 HitScan/중력 없는 Projectile 첫 충돌이 일치
```

이 Phase는 완료된 구현 이력이다. 최신 터렛 레티클 계약은 이 Preview의 탄종/충돌 의미를 소비하지 않는다.

### Phase 2 — 터렛 레티클 C++ 표시 연결

```text
1. UCFAimReticleWidget에서 Weapon Aim Solution 읽기
2. CurrentMuzzleDirection 기반 터렛 조준 3D 지점을 화면 좌표로 투영
3. Image_WeaponReticle Optional 바인딩 연결
4. 표시/숨김과 상태별 Opacity 적용
5. 기존 Image_CenterDot 조준 레티클과 FireFeedback 상태 유지
6. FireMode와 중력 여부에 따른 UI 위치 분기 제거
```

완료 조건:

```text
Image_WeaponReticle이 탄종과 무관하게 터렛의 현재 조준 지점에 표시
```

### Phase 3 — WBP 시각 디자인

```text
1. 작은 빈 원 또는 십자 형태 제작
2. 조준 레티클보다 낮은 시각 우선순위 적용
3. TurretAligning / Ready / Blocked 시각 구분
4. 겹침 상태 가독성 확인
5. 해상도와 DPI Scale 확인
```

완료 조건:

```text
플레이어가 두 Reticle의 역할을 설명 없이 구분 가능
```

### Phase 4 — 이동 및 화면 경계 회귀

```text
1. 직진/선회 중 고정 목표 추적
2. 카메라와 차량 동시 회전
3. 가까운 목표와 먼 목표
4. 화면 가장자리 조준
5. 카메라 FOV 변경
6. 차량 Pitch/Roll 변화
```

완료 조건:

```text
CurrentMuzzleDirection 기반 터렛 조준 지점과 Image_WeaponReticle이 허용 오차 안에서 지속 일치
```

### Phase 5 — 후속 3D 착탄 위치 표시

```text
1. 터렛 레티클 완료 후 사용자와 표현 방식 논의
2. 투사체 예상/실제 착탄 데이터 범위 확정
3. 월드 공간 3D 표시 방식 설계
4. 필요한 경우 중력 Projectile용 탄도 해 계산
5. 해가 없는 경우의 3D 표시 상태 정의
```

Phase 2~4 터렛 레티클 완료 후 별도 Feature로 착수한다. Reticle enum이나 Image_WeaponReticle에 합치지 않는다.

### Phase 6 — Lead Indicator

```text
1. 대상 속도와 상대 속도 확보
2. 비행시간 기반 미래 위치 계산
3. Lead Indicator 표시
4. 락온/센서 품질과 연결
```

3D 착탄 위치 표시와 대상 선택 시스템 이후 착수한다.

---

## 17. 예상 수정 파일

### C++ 수정 후보

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

### UE 에셋 수정 후보

```text
/Game/CarFight/UI/WBP_AimReticle
```

### 구현 완료 후 갱신할 Systems 문서

```text
Document/Systems/UI/AimReticle.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/UI/VehicleDebugPanel.md
```

Ballistic 단계에서는 추가로 갱신한다.

```text
Document/Systems/Combat/Projectile.md
```

---

## 18. 보호 범위

이번 기능 구현에서 아래 계약을 임의로 변경하지 않는다.

```text
- AimTargetLocation은 Image_CenterDot 조준 레티클 목표점이다.
- Image_WeaponReticle은 CurrentMuzzleDirection 기반 터렛 조준 지점이다.
- 터렛 레티클은 탄종과 착탄 위치에 의존하지 않는다.
- AimDirection은 실제 최종 발사 방향이다.
- HitScan과 Projectile은 같은 FireRequest.AimDirection을 사용한다.
- bAllowFireWhileAligning=true이면 정렬 중 현재 Muzzle 방향으로 발사한다.
- bAllowFireWhileAligning=false이면 정렬 완료 전 발사를 거부한다.
- MuzzleBlocked는 정책과 관계없이 발사를 거부한다.
- OutOfArc는 현재 단독 발사 차단 조건이 아니다.
- AimReticle은 판정을 계산하지 않고 결과를 표시한다.
- 투사체 착탄 위치는 후속 별도 3D 표시에서 다룬다.
- 기존 WBP 바인딩 이름과 FireFeedback 상태는 유지한다.
```

또한 이번 첫 구현에서 아래를 함께 개발하지 않는다.

```text
- Ballistic Solver
- Lead Indicator
- 자동 락온
- Aim Assist
- 다중 터렛 동시 Reticle
- 네트워크 지연 보정
- 완성형 HUD 전체 재설계
```

---

## 19. 테스트 계획

### 19.1 정지 상태

```text
- 5m / 20m / 100m 목표
- 화면 중앙 / 좌우 가장자리
- 높은 목표 / 낮은 목표
- 터렛 최대 Yaw/Pitch 근처
```

확인:

```text
조준 레티클 = 플레이어가 지정한 목표
터렛 레티클 = CurrentMuzzleDirection 기반 터렛 조준 지점
발사 결과/착탄 위치 = 두 Reticle의 제품 의미와 별도
```

### 19.2 정렬 중 발사 허용

```text
1. Reticle을 빠르게 좌우로 이동
2. 터렛이 따라오는 중 발사
3. CurrentMuzzleDirection Debug와 터렛 레티클 비교
4. 터렛 정렬 후 두 Reticle 겹침 확인
```

### 19.3 정렬 중 발사 금지

```text
1. 두 Reticle이 분리된 상태에서 발사
2. TurretAligning 거부 확인
3. 두 Reticle이 겹친 뒤 발사 승인 확인
```

### 19.4 장애물

```text
- 카메라는 볼 수 있지만 Muzzle 앞이 막힌 낮은 벽
- 차량 차체 가까이 배치된 장애물
- 목표 앞 다른 차량
```

확인:

```text
- 터렛 레티클은 기존 터렛 조준 위치 유지
- 조준 레티클은 Blocked 상태
- 발사 거부
```

### 19.5 이동 상태

```text
- 직진 중 고정 목표
- 선회 중 고정 목표
- 카메라 회전과 차체 회전 동시 수행
- 차량 Pitch/Roll 변화
```

### 19.6 무기 실행 방식

```text
- Dummy HitScan
- Projectile Actor
- Projectile Pool 재사용
```

터렛 레티클은 Dummy HitScan, 중력/무중력 Projectile 모두 같은 방식으로 검증한다.

---

## 20. PASS 기준

```text
1. Image_CenterDot 조준 레티클과 Image_WeaponReticle 터렛 레티클의 의미가 코드와 UI에서 분리된다.
2. 터렛 레티클은 FCFVehicleWeaponAimSolution.CurrentMuzzleDirection을 기준으로 한다.
3. 터렛 레티클은 HitScan/Projectile과 중력 여부에 따라 위치 계산 경로가 바뀌지 않는다.
4. 정렬 완료 시 터렛 레티클이 조준 레티클에 수렴한다.
5. 정렬 중 발사 금지 정책의 발사 거부가 유지된다.
6. MuzzleBlocked 발사 차단이 유지된다.
7. 투사체 착탄 위치가 터렛 레티클에 섞이지 않는다.
8. 기존 Ready / FireSuccess / Cooldown / NoWeapon / AimBlocked 표시가 회귀하지 않는다.
9. Optional WBP 바인딩 누락 시 크래시가 없다.
10. Tools\BuildEditor.bat가 성공한다.
11. 사용자 싱글 PIE 확인을 통과한다.
```

정량 오차 기준은 구현 착수 시 화면 픽셀과 월드 거리 두 기준으로 확정한다.

---

## 21. 리스크와 대응

### 리스크 1 — 터렛 레티클과 착탄 위치 의미 혼동

원인 후보:

```text
- 기존 DirectImpact/LaunchDirection 코드와 Debug 명칭
- MuzzleBlocked 첫 Hit 위치
- 중력 Projectile의 낙하
- HitScan의 즉시 충돌
```

대응:

```text
- 터렛 레티클은 CurrentMuzzleDirection 기반 단일 계산으로 통일
- 탄종과 충돌 Preview 분기를 UI 소비 경로에서 제거
- 투사체 착탄 위치는 별도 3D 표시 기능으로 분리
```

### 리스크 2 — Reticle 두 개로 화면이 복잡해짐

대응:

```text
- 터렛 레티클 크기와 Opacity를 낮춤
- 상태 텍스트를 추가하지 않고 위치 정보 중심으로 표현
- 겹침 시 Fade 정책을 사용자 PIE로 비교
```

### 리스크 3 — UI 보간으로 실제 방향과 어긋남

대응:

```text
- 첫 구현은 보간 없음
- 필요 시 매우 짧은 시각 보간만 적용
- 판정과 발사에는 보간값을 사용하지 않음
```

### 리스크 4 — MuzzleBlocked Trace가 터렛 레티클 위치를 오염

대응:

```text
- MuzzleBlocked Trace는 발사 차단에만 사용
- 첫 Blocking Hit 위치를 Image_WeaponReticle 위치로 사용하지 않음
```

### 리스크 5 — 완료 작업과 후속 Active 작업 간 우선순위 혼동

`CF-FQ-025` 완료 후에도 과거 Active 기록만 읽으면 세션 복원 시 작업 초점이 혼동될 수 있다.

대응:

```text
- CF-FQ-025는 Done / User PIE PASS로 고정
- CF-FQ-024를 기존 순서상 구현순위 1위 Active 작업으로 복원
- 투사체 착탄 위치 3D 표시는 사용자 논의 후 별도 Feature로 등록
```

---

## 22. 미결정 항목

### 제품/UX 결정

```text
- 터렛 레티클 항상 표시 vs 정렬 중에만 표시
- 정렬 완료 시 유지 vs Fade Out
- 화면 밖 터렛 레티클을 숨김 vs 가장자리 Clamp
- MuzzleBlocked 전용 아이콘 필요 여부
- 터렛 레티클 색상을 상태색과 공유할지 별도 중립색을 사용할지
```

### 기술 결정

해결됨:

```text
- bHasValidTurretReticlePoint / TurretReticleWorldLocation / TurretReticleDistance 전용 필드를 추가
- 기존 Preview 필드는 Legacy Debug로 보존
```

이번 단계에서 확정:

```text
- Image_CenterDot은 조준 레티클이다.
- Image_CenterDot의 현재 작동은 사용자 확인 PASS이며 후속 코드 정렬에서 회귀 보호한다.
- Image_WeaponReticle은 CurrentMuzzleDirection 기반 터렛 레티클이다.
- 터렛 레티클은 탄종과 착탄 위치에 의존하지 않는다.
- 투사체 착탄 위치는 후속 별도 3D 표시로 구현한다.
```

### 일정 결정

```text
- CF-FQ-025 Phase 0~4와 사용자 PIE 완료
- CF-FQ-024를 기존 순서상 구현순위 1위 Active 작업으로 복원
```

공식 구현 순서:

```text
완료. CF-FQ-025 Phase 0~4 구현 및 사용자 PIE
1. CF-FQ-024 전투 FX 구현 재개
2. CF-FQ-019 주행/전투 반복 테스트
3. CF-FQ-020 조작감/전투 템포/피드백 개선
4. CF-FQ-021 핵심 게임 루프 검증
```

---

## 23. 공식 등록 상태

```text
- Feature ID: CF-FQ-025
- Priority: P0
- Status: Done / User PIE PASS
- 완료일: 2026-07-21
- 대표 Plan: Document/Plan/ReticleAimDirection/ImplementationDesign.md
- 완료 구현 범위: Image_CenterDot 조준 레티클 / Image_WeaponReticle 터렛 레티클 의미 통일과 Phase 2~4 검증
```

공식 등록 문서:

```text
Document/ProjectSSOT/03_FeatureQueue.md
Document/Plan/README.md
Document/ActiveWork.md
```

완료 증거:

```text
- CurrentMuzzleDirection 기반 터렛 레티클 전용 데이터 구현
- Image_WeaponReticle Legacy Preview 의존 제거
- Tools\BuildEditor.bat PASS
- 사용자 PIE PASS
```

---

## 24. Migration

### v0.3.4 완료 Plan 생명주기 적용 안내

```text
- CF-FQ-025는 완료 상태를 유지하며 현재 활성 구현 우선순위에서 제외한다.
- 신규 세션의 활성 작업 복원은 CF-FQ-024 CombatFxAudio Plan을 사용한다.
- 이 문서는 완료 당시 설계와 검증 체크포인트 보존용이며 현재 구현 판단은 관련 Systems 문서를 우선한다.
```

### v0.3.3 사용자 PIE 완료 적용 안내

```text
- CF-FQ-025는 Done / User PIE PASS로 해석한다.
- Image_CenterDot과 Image_WeaponReticle의 현재 계약은 완료된 Systems 기준으로 보호한다.
- 투사체 착탄 위치 3D 표시는 CF-FQ-025 완료 범위에 합치지 않고 별도 후속 Feature로 논의한다.
```

### v0.3.2 터렛 레티클 코드 정렬 적용 안내

```text
- Image_WeaponReticle은 bHasValidTurretReticlePoint와 TurretReticleWorldLocation만 소비한다.
- TurretReticleWorldLocation은 AimOrigin + CurrentMuzzleDirection × TurretReticleDistance로 계산한다.
- TurretReticleDistance는 AimOrigin에서 AimTargetLocation까지의 거리다.
- 기존 DirectImpact/LaunchDirection Preview 필드는 삭제하지 않고 Legacy Debug로만 보존한다.
- Blueprint 위젯 이름과 기존 함수 시그니처는 변경하지 않는다.
```

### v0.3.1 Image_CenterDot 검증 상태 적용 안내

```text
- Image_CenterDot의 현재 조준 동작은 사용자 확인 PASS로 해석한다.
- 후속 터렛 레티클 코드 정렬에서 Image_CenterDot 입력과 Camera Aim Trace 경로를 변경하지 않는다.
- Image_CenterDot은 신규 조사·구현 범위가 아니라 회귀 보호 범위다.
```

### v0.3.0 조준/터렛 레티클 구조 확정 적용 안내

```text
- Command Reticle은 최신 제품 용어에서 조준 레티클(Aim Reticle)로 통일하며 실제 위젯은 Image_CenterDot이다.
- Weapon Reticle은 최신 제품 용어에서 터렛 레티클(Turret Reticle)로 통일하며 실제 위젯은 Image_WeaponReticle이다.
- Image_WeaponReticle은 AimDirection, DirectImpact, LaunchDirection, 첫 Blocking Hit 또는 착탄 위치를 표시하지 않는다.
- 터렛 레티클은 CurrentMuzzleDirection 기반 터렛 조준 3D 지점을 모든 탄종에서 동일하게 화면 투영한다.
- 기존 ECFWeaponReticleMode와 Preview 필드는 코드 정렬 전 Legacy 구현/Debug 상태로 해석한다.
- BallisticImpact를 Reticle enum에 추가하지 않으며 투사체 착탄 위치는 후속 별도 3D 표시 기능으로 논의한다.
- 과거 v0.2.x Migration과 Changelog는 구현 이력으로 보존하되 최신 UI 의미 판단에는 v0.3.0을 우선한다.
```

### v0.2.4 LaunchDirection 확정 적용 안내

```text
- 중력 Projectile은 Weapon Reticle 데이터에서 Hidden이 아니라 LaunchDirection으로 해석한다.
- LaunchDirection은 AimOrigin + AimDirection × PreviewDistance로 만든 초기 발사 방향 표식이며 예상 탄착점이 아니다.
- LaunchDirection의 bWeaponPreviewHasBlockingHit는 항상 false로 해석하고, 총구 가림은 기존 bMuzzleBlocked를 별도로 사용한다.
- ECFWeaponReticleMode에는 이번 단계에서 Hidden / DirectImpact / LaunchDirection만 추가한다.
- BallisticImpact와 LeadPoint는 실제 Solver 및 대상 예측 구현 전에는 enum과 런타임 결과에 미리 추가하지 않는다.
- 다음 코드 실행 입력은 Generated/Final/P1_LaunchDirection.md이며, Codex 실행과 브라우저 검수 전에는 구현 완료로 해석하지 않는다.
```

### v0.2.3 실사용 무기 범위 불일치 적용 안내

```text
- 현재 Heavy Cannon의 Preview 무효 결과를 런타임 고장으로 해석하지 않는다.
- HeavyCannon → DA_ProtoTurretCannon → DA_HeavyShell 연결에서 FireMode=Projectile, bAffectedByGravity=true임을 기준으로 한다.
- DA_HeavyShell의 중력 설정을 DirectImpact 테스트 통과 목적으로 변경하지 않는다.
- Phase 2 진입 전에 중력 Projectile의 LaunchDirection 표시 계약을 확정하거나 별도 직선 사격 테스트 에셋으로 DirectImpact 구현을 검증한다.
- 실사용 Heavy Cannon에서 Weapon Reticle을 제공하려면 LaunchDirection 또는 BallisticImpact 지원이 필요하다.
```

### v0.2.2 Phase 1 코드 검수 적용 안내

```text
- Phase 1 World Preview 데이터와 VehicleDebug 표시는 Codex 실행 및 브라우저 코드·빌드 검수를 통과한 상태로 해석한다.
- 사용자 PIE 전까지 Phase 1은 코드 검수 PASS / 사용자 검증 Pending 상태로 유지한다.
- HitScan과 중력 없는 Projectile Preview 일치, 중력 Projectile Preview 무효, MuzzleBlocked와 정렬 정책 회귀를 PIE에서 확인한다.
- Phase 1 PIE PASS 후에만 Phase 2 Weapon Reticle C++ 투영 TaskSource와 Codex 계약을 작성한다.
```

### v0.2.1 정책 위반 복구 적용 안내

```text
- 기존 Browser Direct Edit 변경을 임의로 되돌리거나 Codex 작업으로 소급 기록하지 않는다.
- 후속 Phase 1 코드는 TaskSource_Phase1.md와 P1_WeaponPreviewReview.md를 사용한 Codex 검토·보정으로만 진행한다.
- Codex 실행 전 상태는 Workflow Compliance FAIL, Codex Not Run, Browser Review Pending으로 해석한다.
- 부분 데이터 필드 빌드 PASS는 Phase 1 기능 완료나 사용자 PIE PASS로 승격하지 않는다.
- 오염된 자동 생성 P1-T1 산출물은 CarFight 코드 실행 입력에서 제외한다.
```

### v0.2.0 공식 승격 적용 안내

```text
- CF-FQ-025를 P0 Active와 구현순위 1위로 해석한다.
- 신규 CarFight 구현 세션은 ReticleAimDirection Plan을 우선 복원한다.
- CF-FQ-024는 취소하지 않고 구현순위 2위 Active 작업으로 유지한다.
- 새 Weapon Reticle은 구현 및 사용자 PIE 완료 전까지 Current System으로 해석하지 않는다.
- AimDirection과 AimTargetLocation의 기존 의미는 변경하지 않는다.
- 기존 Command Reticle과 FireFeedback 동작을 회귀 보호한다.
```

---

## 25. Changelog

### v0.3.4 - 2026-07-22

```text
- 완료된 CF-FQ-025가 활성 구현순위 1위처럼 읽히지 않도록 Priority를 Completed P0 / Archive Review로 변경했다.
- 현재 활성 작업 복원은 CF-FQ-024 CombatFxAudio Plan을 사용하도록 생명주기 안내를 추가했다.
- 기능 설계와 사용자 PIE 완료 결과는 변경하지 않았다.
```

### v0.3.3 - 2026-07-21

```text
- 사용자의 계획 일치 확인을 최종 PIE PASS로 기록했다.
- CF-FQ-025 터렛 레티클 범위를 Done으로 전환했다.
- 다음 설계 주제를 별도 월드 공간 3D 착탄 위치 표시 논의로 전환했다.
```

### v0.3.2 - 2026-07-21

```text
- 탄종 독립 터렛 레티클 전용 유효성, 월드 위치와 비교 거리 필드를 추가했다.
- Image_WeaponReticle의 Legacy Weapon Preview 소비를 제거했다.
- Debug Panel에 터렛 레티클 세 필드를 추가하고 기존 Preview 행을 Legacy로 구분했다.
- Tools\BuildEditor.bat 성공과 사용자 PIE 기대값을 기록했다.
```

### v0.3.1 - 2026-07-21

```text
- Image_CenterDot의 현재 작동을 사용자 확인 PASS로 기록했다.
- 화면 이동 입력 계약 확인을 후속 작업에서 제거했다.
- Image_CenterDot을 터렛 레티클 코드 정렬의 회귀 보호 범위로 전환했다.
```

### v0.3.0 - 2026-07-21

```text
- Image_CenterDot을 사용자가 화면 내 원하는 위치를 지정하는 조준 레티클로 확정했다.
- 터렛이 조준 레티클의 3D 목표점을 추적하도록 책임을 고정했다.
- Image_WeaponReticle을 CurrentMuzzleDirection 기반 터렛 조준 3D 지점의 2D 화면 투영으로 확정했다.
- 터렛 레티클을 탄종, FireMode, 중력, 첫 충돌과 착탄 위치에서 분리했다.
- 투사체 착탄 위치는 Reticle UI와 분리된 후속 월드 공간 3D 표시로 이관했다.
- 이전 DirectImpact/LaunchDirection Weapon Reticle 계약을 Superseded로 표시하고 구현 정렬을 다음 작업으로 전환했다.
```

### v0.2.7 - 2026-07-20

```text
- Image_WeaponReticle 사용자 PIE에서 화면 표시를 확인하고 위치 보정 이슈를 기록했다.
- Canvas Slot 좌측 상단 앵커를 런타임에 고정해 화면 좌표 중복 오프셋을 제거했다.
- LaunchDirection Preview 깊이를 PreviewMaxRange에서 CommandPathDistance로 변경해 정렬 완료 시 Command 목표와 같은 점을 투영하도록 보정했다.
- Camera Aim Trace의 목표 Actor를 보존하고 같은 Actor를 맞힌 총구 Trace는 목표 표면으로 허용하도록 MuzzleBlocked 오판을 보정했다.
- Tools\BuildEditor.bat PASS와 세 항목 사용자 PIE 재검증 대기를 기록했다.
```

### v0.2.6 - 2026-07-16

```text
- Phase 2 C++ 화면 투영 구현 결과를 반영했다.
- UCFAimReticleWidget에 Image_WeaponReticle Optional 바인딩, WeaponReticleColor, WeaponReticleOpacity를 추가했다.
- WeaponPreviewWorldLocation을 ProjectWorldLocationToWidgetPosition으로 투영하고 화면 안에 있을 때만 표시하도록 기록했다.
- Image_WeaponReticle 누락 시 기존 Command Reticle과 FireFeedback이 유지되는 안전 동작을 기록했다.
- Tools\BuildEditor.bat PASS와 WBP/PIE Pending 상태를 분리했다.
```

### v0.2.5 - 2026-07-16

```text
- LaunchDirection Codex 실행 결과를 반영했다.
- ECFWeaponReticleMode Hidden / DirectImpact / LaunchDirection 추가와 FCFVehicleWeaponAimSolution.WeaponReticleMode 추가를 기록했다.
- 중력 Projectile LaunchDirection 위치 계산, DirectImpact Trace 확장 조건 분리, MuzzleBlocked 10.0 허용값 보호를 기록했다.
- VehicleDebug의 Weapon Reticle Mode 행과 FireOrigin 요약 문자열의 WeaponReticleMode 값을 기록했다.
- Tools\BuildEditor.bat PASS와 사용자 Heavy Cannon PIE Pending 상태를 분리했다.
- Heavy Cannon 회전 PIE에서 LaunchDirection 모드, Preview Valid, Blocking Hit false, 거리 10000.0, 월드 위치 갱신 부분 확인을 기록했다.
- Heavy Cannon MuzzleBlocked 회귀와 실제 중력탄 초기 발사 방향 일치 사용자 확인을 반영해 LaunchDirection PIE PASS로 전환했다.
```

### v0.2.4 - 2026-07-16

```text
- 사용자 승인에 따라 중력 Projectile의 Weapon Reticle을 LaunchDirection으로 공식 확정
- ECFWeaponReticleMode의 이번 구현 범위를 Hidden / DirectImpact / LaunchDirection으로 확정
- LaunchDirection World Point를 AimOrigin + AimDirection × PreviewDistance로 정의
- LaunchDirection은 Blocking Hit와 예상 탄착점을 의미하지 않으며 bWeaponPreviewHasBlockingHit=false로 정의
- BallisticImpact와 LeadPoint는 실제 Solver 및 대상 예측 구현 시 추가하도록 연기
- TaskSource_LaunchDirection.md 신규 작성
- 최종 Codex 입력 Generated/Final/P1_LaunchDirection.md 생성 및 품질 게이트 통과
- 다음 작업을 Codex 실행, 세 허용 파일 scoped diff, Editor 빌드와 Heavy Cannon PIE 검수로 변경
```

### v0.2.3 - 2026-07-16

```text
- 사용자 PIE에서 Weapon Preview 네 값이 항상 무효로 표시된 현상 기록
- HeavyCannon 프리셋, DA_ProtoTurretCannon과 DA_HeavyShell의 실제 에셋 연결 및 설정 조사
- 현재 실사용 무기가 FireMode=Projectile, bAffectedByGravity=true인 중력 발사체임을 확인
- 현상을 코드 계산 고장이 아니라 DirectImpact 전용 Phase 1과 실사용 무기의 범위 불일치로 재분류
- 현재 상태로 Phase 2를 진행하면 Heavy Cannon의 Weapon Reticle이 계속 숨겨짐을 차단 사유로 등록
- 중력 설정 임시 변경 대신 LaunchDirection 표시와 후속 BallisticImpact 분리를 권장안으로 등록
- 다음 작업을 LaunchDirection 계약 확정 및 Codex 보정 준비로 변경
```

### v0.2.2 - 2026-07-16

```text
- Phase 1 Weapon Preview World 데이터의 Codex 보정 결과와 브라우저 scoped diff 검수를 PASS로 반영
- HitScan 및 중력 없는 Projectile DirectImpact 조건, 단일 WeaponHit Trace 공유와 기존 MuzzleBlocked 10.0 거리 허용값 보존 확인
- Preview 네 필드 할당과 VehicleDebug Preview 네 행 추가 확인
- 독립 Editor 빌드 build_79aa1777becd65e5 PASS, Target Up To Date와 Return Code 0 기록
- Phase 1 상태를 코드·빌드 검수 PASS / 사용자 PIE Pending으로 전환
- 다음 작업을 HitScan·Projectile Preview 일치, 중력 Projectile Preview 무효와 기존 발사 정책 회귀 PIE로 변경
- ActiveWork v1.11과 Phase 1 사용자 검증 상태를 동기화
```

### v0.2.1 - 2026-07-16

```text
- Browser Direct Edit 정책 위반과 부분 코드 상태를 현재 작업 체크포인트에 기록
- 당시 TaskSource Missing, 기존 계약 미실행, Workflow Compliance FAIL 상태 명시
- Phase 0 기준선 및 부분 필드 빌드 증거와 PIE Pending을 분리 기록
- Codex 검토·보정용 TaskSource_Phase1.md와 P1_WeaponPreviewReview.md 경로 등록
- GoPyMCP 테스트 대상이 혼입된 자동 생성 P1-T1 산출물을 실행 입력에서 제외
- 바로 다음 작업을 Codex 실행, 브라우저 diff·빌드 검수와 사용자 PIE 순서로 변경
```

### v0.2.0 - 2026-07-16

```text
- CF-FQ-025를 P0 Active 공식 기능과 구현순위 1위로 승격
- CF-FQ-024를 취소하지 않고 구현순위 2위 Active 작업으로 조정
- Phase 0~4 DirectImpact Weapon Reticle을 첫 구현 범위로 확정
- 공식 등록 상태, 병행 Active 작업 우선순위와 Migration 추가
- 다음 작업을 Phase 0 회귀 확인과 Phase 1 Codex 작업지시서 생성으로 변경
```

### v0.1.2 - 2026-07-16

```text
- Plan 검사기가 Scope 코드펜스를 항목으로 오인하지 않도록 In Scope와 Out of Scope를 일반 목록으로 변경
- 기능 범위 내용과 우선순위는 변경하지 않음
```

### v0.1.1 - 2026-07-16

```text
- Plan 자동 검사에서 인식할 Goal / In Scope / Out of Scope / Constraints / Target Files / Verification 표준 섹션 추가
- 기존 조준 계약, 첫 구현 범위와 대상 파일을 문서 상단에서 즉시 확인하도록 보강
- 기능 설계와 구현 계약의 내용은 v0.1.0과 동일하게 유지
```

### v0.1.0 - 2026-07-16

```text
- 현재 AimFireAlignment, AimReticle, VehicleAim, WeaponFire와 Projectile 구현 기준 조사
- CombatPlan의 플레이어 조준점 / 무기 실제 조준점 / 예측 조준점 분리 원칙 반영
- Command Reticle과 Weapon Reticle의 이중 Reticle 구조 권장
- FCFVehicleWeaponAimSolution.AimDirection 기반 Weapon Preview 설계
- C++ 계산, WBP 표현과 DataAsset 튜닝 책임 분리
- 직선 사격, Ballistic Reticle과 Lead Indicator의 단계적 구현 계획 작성
- 테스트, PASS 기준, 보호 범위와 미결정 항목 정의
```

---

## 26. 확인 근거

- 확인 일시: 2026-07-16
- 관련 코드:
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
- 관련 현재 문서:
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/Combat/WeaponFire.md`
  - `Document/Systems/Combat/Projectile.md`
- 관련 설계 문서:
  - `Document/Plan/AimFireAlignment/ImplementationDesign.md`
  - `Document/ProjectSSOT/CombatPlan/05_AimingSystem.md`
  - `Document/ProjectSSOT/CombatPlan/14_CombatUI.md`
  - `Document/ProjectSSOT/00_Vision.md`
