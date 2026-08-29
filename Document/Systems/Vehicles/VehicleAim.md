# VehicleAim

- Version: 1.8.1
- Date: 2026-07-21
- Status: Current / P0 Aim Alignment and CF-FQ-025 Turret Reticle User PIE Verified
- Scope: `VehicleCamera`가 만든 조준 결과와 `FCFVehicleWeaponAimSolution`을 로컬 표시/검증/시각 상태로 관리하는 현재 구현

---

## 1. 문서 목적

이 문서는 현재 프로젝트에서 `VehicleAim` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 이 문서에서 `VehicleAim`은 서버 검증/복제 시각화 계층이 아니라, 로컬 조준 상태와 로컬 발사 검증 상태를 분리해 보관하는 차량 조준 중간 계층으로 본다.

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
ServerAimState     -> FireValidationState
RepAimVisualState  -> AimVisualState
WaitingServer      -> FirePending
ServerRejected     -> FireRejected
NoAuthority        -> InvalidLocalState
```

`FireValidationState`는 서버 검증 상태가 아니라, 현재 로컬 Fire Command 처리 결과를 기록하는 상태다.
`AimVisualState`는 복제 시각화 상태가 아니라, 로컬 UI/디버그/후속 이펙트가 읽는 조준 시각화 상태다.

---

## 3. 문서 범위

이 문서에서 말하는 `VehicleAim` 기능은 아래 요소를 묶어서 본다.

- 핵심 컴포넌트: `UCFVehicleAimComp`
- 공용 타입 정의: `CFVehicleAimTypes.h`
- 대표 소유 주체: `ACFVehiclePawn`
- 카메라 상태 공급 주체: `UCFVehicleCameraComp`
- 로컬 표시 소비 주체: `UCFAimReticleWidget`
- 로컬 발사 명령 주체: `ACFVehiclePawn::BuildFireCommand()` 계열 함수
- 로컬 발사 검증 상태: `FCFVehicleFireValidationState`
- 로컬 조준 시각 상태: `FCFVehicleAimVisualState`
- 무기 조준 해 상태: `FCFVehicleWeaponAimSolution`
- 현재 대표 입력/전투 연결 경로:
  - `ACFVehiclePawn::HandleFireStarted()`
  - `ACFVehiclePawn::BuildFireCommand()`
  - `ACFVehiclePawn::ValidateFireCommand()`
  - `ACFVehiclePawn::RunLocalDummyHitScan()`
  - `ACFVehiclePawn::ApplyFireResult()`

즉, 현재 기준 `VehicleAim`은 카메라 조준점만 계산하는 기능이 아니다.
`VehicleCamera`가 만든 조준 결과를 읽고, 로컬 Reticle 표시 상태, 로컬 발사 검증 상태, 로컬 조준 시각 상태, Fire Command 생성 보조까지 묶은 싱글플레이 조준-발사 중간 계층으로 본다.

---

## 4. 현재 실제 역할

현재 구현 기준 `VehicleAim`의 핵심 역할은 **VehicleCamera가 계산한 조준 결과를 차량 기준 AimProfile로 해석하고, Local Aim 상태 / FireValidationState / AimVisualState로 분리해 관리하는 것**이다.

현재 `VehicleAim`은 아래 일을 한다.

1. Aim 런타임 참조를 준비한다.
2. 로컬 제어 Pawn에서 Aim 표시 상태를 계산한다.
3. 로컬 플레이어 기준 Aim 방향과 Reticle 상태를 만든다.
4. 차량 로컬 조준각을 계산한다.
5. 기본 AimProfile의 범위 안/밖을 계산한다.
6. Fire Command 생성을 보조한다.
7. 로컬 발사 검증 결과를 FireValidationState에 반영한다.
8. 로컬 Aim 시각 상태를 AimVisualState에 반영한다.

---

## 5. Aim 런타임 참조 준비

`UCFVehicleAimComp`는 `BeginPlay()`에서 `InitializeAimRuntime()`을 호출한다.

현재 초기화에서 확인하는 참조:

```text
- Owner 차량 Pawn: ACFVehiclePawn
- 차량 카메라 컴포넌트: UCFVehicleCameraComp
```

관련 함수:

```text
- InitializeAimRuntime()
- RefreshAimRuntimeReferences()
- ResolveOwnerVehiclePawn()
- ResolveVehicleCameraComp()
```

현재 동작:

```text
- Owner Actor를 ACFVehiclePawn으로 캐스팅한다.
- Owner Pawn에서 VehicleCameraComp를 가져온다.
- 두 참조가 모두 있으면 bAimRuntimeReady = true로 본다.
- 결과는 LastAimRuntimeSummary 문자열로 남긴다.
```

즉 현재 `VehicleAim`은 `VehicleCamera`를 직접 만들거나 소유하지 않고, 현재 차량 Pawn에 붙어 있는 `VehicleCameraComp`를 찾아 조준 해석에 사용한다.

---

## 6. 로컬 제어 Pawn에서 Aim 표시 계산

`UCFVehicleAimComp::TickComponent()`는 싱글플레이 로컬 Pawn 기준으로 Reticle 상태를 갱신한다.

현재 의미:

```text
- 현재 목표는 싱글플레이 로컬 조준/발사 검증이다.
- Dedicated Server는 현재 구현 목표가 아니며, 방어 코드 수준에서 Local Reticle 계산을 생략한다.
- 로컬 발사 검증 상태는 Fire 입력 처리 흐름에서 별도로 갱신된다.
```

즉 현재 `VehicleAim`에서 Local Aim은 **로컬 플레이어 표시용 상태**이고, 온라인 서버 표시 상태로 취급하지 않는다.

---

## 7. Local Aim 상태 계산

`RefreshLocalAimState()`는 로컬 제어 Pawn에서만 실제 Local Aim 상태를 갱신한다.

현재 처리 흐름:

```text
1. 런타임 참조가 준비되지 않았으면 다시 참조를 찾는다.
2. 참조가 없으면 Reticle을 Hidden으로 둔다.
3. Pawn이 로컬 제어가 아니면 Reticle을 Hidden으로 둔다.
4. VehicleCameraComp->GetCameraRuntimeState()에서 카메라 런타임 상태를 읽는다.
5. 카메라의 AimHitLocation을 목표 위치로 사용한다.
6. Owner 차량 위치에서 목표 위치까지의 방향을 구한다.
7. 목표 위치가 비정상적이면 차량 정면 방향을 fallback으로 사용한다.
8. 월드 조준 방향을 차량 로컬 Yaw/Pitch 각도로 변환한다.
9. DefaultAimProfile의 Yaw/Pitch 범위 안에 있는지 계산한다.
10. 카메라 런타임 상태의 목표 표면 선택용 Blocking Hit 여부를 기록한다.
11. FCFVehicleWeaponAimSolution에서 정책값, 요구 방향, 현재 Muzzle 방향, 실제 최종 방향과 정렬/차단 상태를 읽는다.
12. 유효한 Aim Solution과 최종 방향이 있고 MuzzleBlocked가 아니며 정책상 정렬이 발사를 막지 않으면 Local Aim 기준 발사 가능 예측으로 본다.
13. bLocalWithinWeaponArc는 별도 표시/디버그 값으로 저장한다.
14. 결과를 FCFVehicleLocalAimState에 저장한다.
```

현재 `LocalAimState`에 저장되는 핵심 값:

```text
- LocalAimTargetLocation
- LocalAimDirection
- LocalReticleState
- bLocalCanFire
- bLocalWithinWeaponArc
- bLocalAimBlocked
- bLocalAimTraceHasBlockingHit
```

중요한 기준:

```text
bLocalCanFire = Aim Solution 유효 && 최종 AimDirection 유효 && !MuzzleBlocked && (bAllowFireWhileAligning || !정렬 중)
bLocalWithinWeaponArc = 기준 AimProfile 범위 안/밖 표시값
```

현재 P0 싱글플레이 기준에서 `bLocalWithinWeaponArc`는 **단독 발사 차단 조건이 아니다.**
실제 발사 성공 여부는 `WeaponFire`의 `ValidateFireCommand()` 결과가 최종 기준이다.

즉 현재 Local Aim은 최종 전투 결과가 아니라, 로컬 플레이어가 즉시 Reticle을 표시하기 위한 예측/표시 상태다.

---

## 7-1. 현재 Reticle / Turret / Muzzle 정렬 구현

2026-07-13 사용자 PIE에서 화면 Reticle과 실제 사격 방향 불일치가 확인됐고, 2026-07-14 C++ 구현에서 아래 단일 Aim Solution 구조로 정리됐다.

현재 코드 기준 방향 생성 지점은 아래와 같다.

```text
Camera Aim Target
- FollowCamera 위치에서 카메라 방향으로 WeaponHit Trace
- AimHitLocation 생성

DesiredAimDirection
- Muzzle 위치에서 AimHitLocation 방향 계산

Turret Aim Direction
- DesiredAimDirection을 계속 추적

Final Fire Direction
- 정렬 중 정책 true이면 CurrentMuzzleDirection
- 정렬 완료이면 DesiredAimDirection
- 정렬 중 정책 false이면 ValidateFireCommand에서 거부
```

각 방향은 의미가 다르지만 하나의 `FCFVehicleWeaponAimSolution`에서 함께 기록한다.

```text
DesiredAimDirection = 요구 방향
CurrentMuzzleDirection = 현재 총구 방향
AimDirection = 실제 최종 발사 방향
```

카메라 Aim Trace, Dummy HitScan, Muzzle obstruction은 모두 `WeaponHit` 응답 기준을 사용한다.

현재 상태 해석:

```text
- LocalAimTargetLocation은 카메라가 선택한 목표점이다.
- DesiredAimDirection은 Muzzle에서 Reticle 목표점으로 향하는 요구 방향이다.
- CurrentMuzzleDirection은 현재 Muzzle Socket X축 방향이다.
- AimDirection은 HitScan과 Projectile이 공유하는 실제 최종 발사 방향이다.
- 정렬 중 정책 true이면 AimDirection은 CurrentMuzzleDirection이다.
- 정렬 완료 후 AimDirection은 DesiredAimDirection이다.
- 정렬 중 정책 false이면 ValidateFireCommand가 TurretAligning 또는 WeaponNotAligned로 거부한다.
- MuzzleBlocked는 실제 최종 AimDirection 경로를 검사하며 정책과 관계없이 거부한다.
```

이 구현의 완료 당시 설계와 검증 evidence는 아래 Historical 문서에 보존한다.

```text
Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md
```

현재 구조 요약:

```text
- Reticle이 지시하는 월드 위치를 DesiredAimTargetLocation SSOT로 사용
- Camera Aim Trace와 실제 무기 Trace의 WeaponHit 기준 통일
- Muzzle → DesiredAimTargetLocation 요구 방향 계산
- 터렛이 요구 방향을 추적
- CurrentMuzzleDirection과 요구 방향의 정렬 오차 계산
- 터렛별 정책에 따라 정렬 중 현재 Muzzle 방향 발사 또는 정렬 완료 전 거부
- 총구 앞 장애물 별도 Trace
- OutOfArc와 TurretAligning 분리
```

---

## 7-2. 조준 레티클과 터렛 레티클 데이터 책임

2026-07-21 확정 기준:

```text
Image_CenterDot
→ 사용자가 화면 내에서 지정하는 조준 레티클
→ Camera Aim Trace
→ AimTargetLocation

AimTargetLocation
→ 터렛이 추적할 3D 목표점

CurrentMuzzleDirection
→ 터렛이 현재 실제로 조준하는 방향
→ Image_WeaponReticle 터렛 레티클의 3D 지점 계산 기준

AimDirection
→ 정렬 상태와 발사 허용 정책을 반영한 실제 Fire Command 방향
→ 터렛 레티클의 제품 의미 기준이 아님
```

터렛 레티클은 HitScan/Projectile, 중력 적용 여부, 첫 충돌과 착탄 위치에 의존하지 않는다.
투사체 착탄 위치는 VehicleAim Reticle 데이터에 합치지 않고 후속 별도 3D 표시 기능에서 다룬다.

현재 C++은 `bHasValidTurretReticlePoint`, `TurretReticleWorldLocation`, `TurretReticleDistance`를 제공한다.
`TurretReticleWorldLocation`은 `AimOrigin + CurrentMuzzleDirection × TurretReticleDistance`이며, 비교 거리는 `AimOrigin`에서 `AimTargetLocation`까지의 거리다.
UI는 이 터렛 레티클 전용 값만 소비하고 기존 `ECFWeaponReticleMode`와 `WeaponPreviewWorldLocation`은 Legacy Debug로만 보존한다.
공식 에디터 빌드와 2026-07-21 사용자 PIE에서 계획한 터렛 레티클 동작을 확인했다.
완료 당시 세부 기준은 `Document/Plan/Archive/ReticleAimDirection/ImplementationDesign.md` v0.3.2에 보존한다. 현재 구현 판단은 이 Systems 문서와 실제 Source를 우선한다.

---

## 8. 차량 로컬 조준각 계산

### 8.1 AimFireAlignment Core 구현 상태

2026-07-13 Core C++ 구현 이후 현재 기준은 아래와 같다.

```text
- Camera Aim Trace는 WeaponHit 기준을 사용한다.
- UCFVehicleAimComp는 FCFVehicleWeaponAimSolution을 보관한다.
- AimOrigin은 Muzzle 위치를 우선 사용한다.
- DesiredAimDirection은 Muzzle 위치에서 Reticle 목표점으로 향하는 요구 방향이다.
- CurrentMuzzleDirection은 현재 Muzzle Socket X축 방향이다.
- AimDirection은 정책과 정렬 상태를 반영한 실제 최종 발사 방향이다.
- HitScan / Projectile은 같은 AimOrigin / AimDirection / AimTargetLocation을 공유한다.
- CurrentMuzzleDirection과 DesiredAimDirection의 정렬 오차를 WeaponAlignmentErrorDeg로 기록한다.
- TurretAligning, WeaponNotAligned, MuzzleBlocked를 별도 거부 사유로 기록한다.
- VehicleDebug Panel은 Weapon Aim Solution 하위 섹션으로 현재 값을 표시한다.
```

검증 상태:

```text
- Align Fire Policy 포함 Unreal Editor 타깃 빌드 성공
- 싱글 PIE에서 정렬 완료 후 Reticle 목표점과 실제 탄착 일치 PASS
- bAllowFireWhileAligning=true 정렬 중 발사 승인과 CurrentMuzzleDirection 진행 PASS
- bAllowFireWhileAligning=false 정렬 중 거부와 정렬 완료 후 승인 PASS
- TurretAligning amber, WeaponNotAligned 비가림, 정책 양쪽 MuzzleBlocked 발사 차단 PASS
- 주행·거리별 정량 오차와 경계각은 CF-FQ-019 확장 회귀
```

`CalculateAimAnglesRelativeToVehicle()`는 월드 조준 방향을 차량 Actor의 로컬 공간 방향으로 변환한다.

현재 계산 방식:

```text
- OwnerVehiclePawn->GetActorTransform().InverseTransformVectorNoScale(AimDirection)으로 월드 방향을 차량 로컬 방향으로 변환한다.
- 로컬 방향의 Y/X로 Yaw를 계산한다.
- 로컬 방향의 Z / 수평 길이로 Pitch를 계산한다.
```

현재 의미:

```text
- 차량 정면 기준 좌우 조준각은 Yaw로 본다.
- 차량 기준 상하 조준각은 Pitch로 본다.
- 이 값이 DefaultAimProfile 범위 안에 있으면 조준각 내부로 본다.
```

단, 조준각 내부 여부는 현재 기본적으로 **표시/디버그용 상태**다.
이 값을 곧바로 발사 성공/실패 판정으로 해석하지 않는다.

---

## 9. Default Aim Profile 기준

현재 `UCFVehicleAimComp`는 `FCFVehicleAimProfile DefaultAimProfile`을 가진다.

현재 기본 프로필 항목:

```text
- ProfileName
- MinYawDeg
- MaxYawDeg
- MinPitchDeg
- MaxPitchDeg
- MaxAimDistance
```

`IsAimWithinDefaultProfile()`은 현재 Yaw/Pitch가 기본 프로필 범위 안에 있는지만 검사한다.

현재 의미:

```text
- 이 프로필은 아직 무기별/터렛별 완성 튜닝 체계가 아니라, Aim 시스템이 가진 기본 제한값이다.
- 현재 발사 요청의 WeaponGroupId는 기본적으로 DefaultAimProfile.ProfileName을 사용한다.
- 후속 무기 시스템이 들어오면 무기별 AimProfile로 분리될 수 있다.
```

---

## 10. Reticle 상태 생성

`BuildLocalReticleState()`는 현재 Local Aim 조건을 `ECFVehicleReticleState`로 바꾼다.

현재 실제 계산에서 사용하는 상태:

```text
- Hidden
- Blocked
- OutOfArc
- TurretAligning
- Ready
```

현재 코드 기준 판정 순서:

```text
1. Aim 런타임이 준비되지 않으면 Hidden
2. 조준이 막혔으면 Blocked
3. bWithinWeaponArc가 false이면 OutOfArc
4. Weapon Aim Solution이 정렬 대기 중이면 TurretAligning
5. bCanFire가 true이면 Ready
6. 나머지는 Hidden
```

중요한 점은 정상 정렬 대기 상태가 더 이상 `Hidden`으로 떨어지지 않는다는 것이다.
현재 `bCanFire`는 유효한 Weapon Aim Solution과 최종 AimDirection, MuzzleBlocked, `bAllowFireWhileAligning` 정책을 중심으로 계산되며, `bWithinWeaponArc`를 직접 요구하지 않는다. 정렬 중 발사가 가능해도 Reticle은 `TurretAligning`을 유지한다.

따라서 현재 P0 싱글플레이 기준에서는 아래처럼 해석한다.

```text
OutOfArc = 조준각 경고/디버그 상태
OutOfArc != 무조건 발사 불가
```

`ECFVehicleReticleState` enum에는 그 외에도 아래 상태가 정의돼 있다.

```text
- NoWeapon
- Cooldown
- Reloading
- FirePending
- FireRejected
```

현재 의미:

```text
- enum은 후속 무기/탄약/쿨다운/발사 처리 상태까지 고려해 넓게 정의돼 있다.
- 하지만 현재 BuildLocalReticleState() 계산에서 상시 사용하는 상태는 제한적이다.
- NoWeapon, Cooldown, Reloading, FirePending, FireRejected는 weapon fire UI / FireFeedback 확장 단계에서 실제 표시 경로가 정리되어야 한다.
```

---

## 11. Fire Command 데이터 생성

`UCFVehicleAimComp::BuildFireRequest()`는 현재 Local Aim 상태를 기반으로 로컬 발사 명령 데이터를 만든다.

현재 채우는 값:

```text
- FireRequestId
- ClientFireTimeSeconds
- PredictedAimTargetLocation
- AimDirection
- WeaponGroupId
- AimOrigin
```

현재 `AimOrigin`:

```text
- OwnerVehiclePawn이 있으면 차량 Actor 위치를 사용한다.
```

현재 의미:

```text
- 현재 조준 방향과 예측 목표 위치를 로컬 발사 검증에 넘긴다.
- 함수명과 구조체명에는 Request가 남아 있지만, 현재 의미는 네트워크 요청이 아니라 로컬 Fire Command 데이터다.
- 최종 전투 결과는 아직 만들지 않으며, 로컬 검증과 더미 HitScan 확인에 사용한다.
```

---

## 12. 로컬 발사 검증 결과 반영

현재 로컬 발사 결과는 AimComp의 검증 상태와 시각 상태에 반영된다.

관련 함수:

```text
- BuildFireValidationStateFromFireCommand()
- ApplyFireValidationResult()
- UpdateAimVisualFromFireResult()
```

현재 `BuildFireValidationStateFromFireCommand()` 역할:

```text
- 발사 명령의 예측 목표 위치를 검증 상태에 저장한다.
- 요청 조준 방향이 기본 프로필 안에 있는지 계산한다.
- 승인 여부와 거부 사유를 저장한다.
- 승인/거부된 요청 ID를 기록한다.
```

현재 `ApplyFireValidationResult()` 역할:

```text
- 확정 목표 위치를 FireValidationState에 반영한다.
- 승인 여부를 bValidationCanFire에 반영한다.
- 거부 사유와 요청 ID를 기록한다.
- FireResult.RejectReason이 OutOfWeaponArc일 경우 bValidationWithinWeaponArc를 false로 반영할 수 있다.
```

현재 의미:

```text
- Local Aim은 즉시 표시용이다.
- FireValidationState는 발사 검증 결과 기록용이다.
- 두 상태를 분리해 UI 표시와 발사 판정 기록이 서로 덮어쓰지 않게 한다.
```

중요한 현재 기준:

```text
실제 발사 성공 여부 = ACFVehiclePawn::ValidateFireCommand() 결과
LocalAimState.bLocalCanFire = Reticle 표시용 예측값
FireValidationState.bValidationCanFire = 실제 Fire Command 검증 결과 기록값
```

---

## 13. ValidateFireCommand와의 관계

`VehicleAim`은 발사 명령 데이터를 만들고 결과 상태를 보관하지만, 현재 실제 로컬 발사 검증은 `ACFVehiclePawn::ValidateFireCommand()`가 수행한다.

현재 `ValidateFireCommand()`가 검사하는 대표 조건:

```text
- Controller 존재 여부
- VehicleAimComp 존재 여부
- Aim runtime 준비 여부
- AimDirection 유효성
- AimOrigin 유효성
- Pawn 위치와 AimOrigin 사이 거리
- 최신 Weapon Aim Solution의 MuzzleBlocked 여부
- bAllowFireWhileAligning=false일 때 TurretAligning / WeaponNotAligned 여부
- 활성 WeaponData 호환 여부
- 활성 무기 쿨다운 여부
```

현재 기준에서 `ValidateFireCommand()`는 `OutOfArc`를 기본 발사 거부 조건으로 사용하지 않는다.
후속 설계에서 `OutOfArc` 또는 `OutOfWeaponArc`를 실제 발사 거부 조건으로 사용할 경우, 아래 문서를 함께 갱신해야 한다.

```text
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/FireFeedback.md
```

---

## 14. 로컬 Aim 시각 상태 갱신

`AimVisualState`는 발사 결과를 UI/디버그/후속 시각 효과가 읽을 수 있게 보관하는 로컬 상태다.

현재 `UpdateAimVisualFromFireResult()` 동작:

```text
- 발사 요청의 조준 방향을 정규화한다.
- 비정상 방향이면 Local Aim 방향 또는 차량 정면 방향을 fallback으로 사용한다.
- 발사 결과의 목표 위치, 적중 위치, 요청 예측 목표 위치 순서로 시각 목표 위치를 결정한다.
- 발사 승인 여부를 bIsFiringVisual에 넣는다.
- WeaponGroupId를 WeaponVisualMode에 넣는다.
```

현재 의미:

```text
- AimVisualState는 전투 판정용 데이터가 아니다.
- 로컬 UI, 디버그, 후속 발사 이펙트가 조준 방향/목표/발사 시각화를 읽기 위한 최소 상태다.
```

---

## 15. 현재 기준 기능의 성격 정리

현재 구현을 종합하면 `VehicleAim`은 아래 역할을 가진다.

### 15.1 카메라 조준 결과 해석 계층

```text
- VehicleCameraComp의 CameraRuntimeState를 읽는다.
- Aim 목표 위치와 방향을 차량 기준 각도로 변환한다.
- 기본 AimProfile의 조준각 안/밖을 계산한다.
```

### 15.2 로컬 Reticle 상태 공급 계층

```text
- Local Aim 상태를 만든다.
- Reticle 상태와 발사 가능 예측 값을 제공한다.
- 로컬 제어 Pawn이 아니면 표시 상태를 숨긴다.
- bLocalWithinWeaponArc를 표시/디버그용 값으로 제공한다.
```

### 15.3 로컬 발사 검증 상태 기록 계층

```text
- Fire Command 데이터를 만든다.
- 로컬 처리 결과를 FireValidationState에 반영한다.
- 승인/거부 요청 ID와 거부 사유를 기록한다.
```

### 15.4 로컬 시각 상태 계층

```text
- UI/디버그/후속 이펙트가 읽을 최소 조준 방향/목표/발사 시각 상태를 보관한다.
- 실제 전투 판정용이 아니라 시각화용 데이터로 본다.
```

따라서 현재 `VehicleAim`은 단순한 조준점 계산기가 아니라,
**VehicleCamera와 WeaponFire 사이에서 Local 표시, 로컬 검증, 로컬 시각화를 분리해주는 차량 조준 운영 기능**이라고 보는 것이 맞다.

---

## 16. 현재 동작 방식

### 16.1 생성 및 초기화

`UCFVehicleAimComp` 생성자에서 수행하는 일:

```text
- Tick 활성화
- bAimRuntimeReady = false
- LastAimRuntimeSummary = "Constructed"
```

`BeginPlay()`에서 수행하는 일:

```text
- InitializeAimRuntime() 호출
```

### 16.2 Tick 기반 Local Aim 갱신

매 Tick에서 수행하는 일:

```text
- Dedicated Server처럼 로컬 표시가 없는 실행 환경이면 Reticle을 숨기고 종료
- 그 외에는 RefreshLocalAimState() 호출
```

### 16.3 발사 명령 생성 및 처리 연결

현재 발사 명령의 상위 흐름은 `ACFVehiclePawn` 쪽에서 관리한다.

대표 흐름:

```text
1. ACFVehiclePawn::HandleFireStarted()
2. ACFVehiclePawn::BuildFireCommand()
3. VehicleAimComp->BuildFireRequest()
4. ACFVehiclePawn::ValidateFireCommand()
5. ACFVehiclePawn::RunLocalDummyHitScan()
6. ACFVehiclePawn::ApplyFireResult()
7. VehicleAimComp->BuildFireValidationStateFromFireCommand()
8. VehicleAimComp->ApplyFireValidationResult()
9. VehicleAimComp->UpdateAimVisualFromFireResult()
```

현재 구조 해석:

```text
- VehicleAimComp는 발사 명령 데이터를 만들고 Aim 상태를 갱신한다.
- 로컬 발사 검증과 더미 HitScan Trace는 ACFVehiclePawn이 수행한다.
- 현재 경로는 RPC를 거치지 않는 싱글플레이 로컬 처리 흐름이다.
```

---

## 17. 현재 표시 조건 / 실행 조건

현재 `VehicleAim`이 정상 동작하려면 아래 조건이 중요하다.

```text
- Owner가 ACFVehiclePawn이어야 한다.
- Owner Pawn에 VehicleCameraComp가 있어야 한다.
- Local Aim 계산은 OwnerVehiclePawn->IsLocallyControlled()인 경우에만 의미가 있다.
- Dedicated Server에서는 방어 코드상 Local Reticle 계산을 하지 않는다.
- 발사 검증은 현재 싱글플레이 로컬 흐름에서 수행된다.
- AimVisualState는 로컬 발사 결과를 UI/디버그/후속 이펙트가 읽기 위한 상태다.
```

---

## 18. 현재 자산 / 클래스 역할

### `UCFVehicleAimComp`

- 종류: C++ ActorComponent
- 현재 역할: Aim 런타임 참조 준비, Local Aim 계산, Reticle 상태 제공, Fire Command 생성 보조, FireValidationState 갱신, AimVisualState 관리

### `CFVehicleAimTypes.h`

- 종류: C++ 공용 타입
- 현재 역할: Reticle 상태 enum, FireRejectReason enum, AimProfile, Local/FireValidation/AimVisual 상태, FireRequest/FireResult 구조 정의

### `ACFVehiclePawn`

- 종류: C++ Pawn
- 현재 역할: VehicleAimComp 소유, 발사 입력 처리, Fire Command 생성 요청, 로컬 검증, 로컬 더미 HitScan, FireResult 적용

### `UCFVehicleCameraComp`

- 종류: C++ ActorComponent
- 현재 역할: Aim 목표 위치, Aim 가림 여부 등 카메라 런타임 상태 공급

### `UCFAimReticleWidget`

- 종류: C++ UserWidget 부모 클래스
- 현재 역할: VehicleAimComp의 Local Aim/Reticle 상태를 읽어 UI 표시로 변환

---

## 19. 현재 생성 및 연결 구조

현재 연결 구조는 아래와 같다.

```text
ACFVehiclePawn
  -> VehicleCameraComp
  -> VehicleAimComp
      -> OwnerVehiclePawn 캐시
      -> VehicleCameraComp 캐시
      -> LocalAimState 갱신
      -> ReticleState 제공
      -> Fire Command 생성 보조
      -> FireValidationState / AimVisualState 갱신
```

발사 입력 처리 구조는 아래와 같다.

```text
HandleFireStarted
  -> BuildFireCommand
  -> VehicleAimComp.BuildFireRequest
  -> ValidateFireCommand
  -> RunLocalDummyHitScan
  -> ApplyFireResult
  -> VehicleAimComp.BuildFireValidationStateFromFireCommand
  -> VehicleAimComp.ApplyFireValidationResult
  -> VehicleAimComp.UpdateAimVisualFromFireResult
```

---

## 20. 현재 기능 책임

현재 `VehicleAim`의 책임은 아래와 같다.

```text
- Owner Pawn과 VehicleCameraComp 참조를 준비한다.
- CameraRuntimeState의 Aim 목표 위치를 읽는다.
- 차량 로컬 기준 Aim Yaw/Pitch를 계산한다.
- 기본 AimProfile 범위 안/밖을 계산한다.
- 로컬 Reticle 상태를 계산한다.
- 로컬 발사 가능 예측 값을 보관한다.
- bLocalWithinWeaponArc를 표시/디버그용 값으로 보관한다.
- Fire Command 생성을 보조한다.
- Weapon Aim Solution을 보관하고 UI/Debug/WeaponFire 경로에 제공한다.
- 로컬 발사 결과를 FireValidationState에 반영한다.
- 로컬 Aim 시각 상태를 관리한다.
```

---

## 21. 현재 기준 비책임 항목

현재 구현상 `VehicleAim`의 직접 책임이 아닌 것은 아래와 같다.

```text
- 실제 무기 장착/해제 시스템
- 탄약 수량 관리
- 쿨다운/재장전 시간 계산
- 실제 데미지 적용
- 실제 투사체 생성
- 실제 터렛 회전 애니메이션
- 멀티플레이 RPC / 복제 경로
- Dedicated Server에서 UI 생성
- 최종 전투 밸런스 결정
```

현재 `VehicleAim`은 무기 시스템 자체가 아니라, 카메라 조준 결과와 무기 발사 사이의 조준 해석/상태 보관 계층이다.

---

## 22. 현재 문서 기준의 핵심 결론

현재 `VehicleAim` 기능은,

**VehicleCamera가 만든 조준 결과를 Local 표시, 로컬 검증, 로컬 시각화 상태로 나누어 관리하는 차량 조준 중간 계층**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleAim`은 현재 차량의 조준 방향이 기준 AimProfile 안에 있는지, 조준이 막혔는지, 로컬 표시와 로컬 발사 검증에서 어떤 상태로 읽혀야 하는지를 분리해 보관하는 현재 상태 기능이다.

다만 현재 P0 싱글플레이 기준에서 `bLocalWithinWeaponArc`와 `OutOfArc`는 **표시/디버그 기준**이며, 단독 발사 차단 조건이 아니다.
실제 발사 성공 여부는 `WeaponFire`의 `ValidateFireCommand()` 결과를 기준으로 판단한다.

---

## 23. 현재 문서에서 미확인인 항목

아래는 아직 이 문서에서 확정하지 않은 내용이다.

```text
- 실제 무기 데이터/터렛 데이터가 DefaultAimProfile을 대체하는 최종 경로
- NoWeapon, Cooldown, Reloading, FirePending, FireRejected 상태를 실제로 전환하는 최종 운영 경로
- AimVisualState를 실제 발사 이펙트가 소비하는 최종 경로
- AimProfileOverride 또는 무기별 AimProfile과 VehicleCamera의 연결 정책
- 로컬 발사 검증에서 LocalAimState.bLocalAimBlocked를 계속 참고할지 장기 정책
- OutOfArc / OutOfWeaponArc를 후속 단계에서 실제 발사 거부 조건으로 승격할지 여부
```

---

## 24. 문서 갱신 조건

아래 변경이 생기면 이 문서를 함께 갱신한다.

```text
- UCFVehicleAimComp의 Local Aim 계산 규칙 변경
- ECFVehicleReticleState 상태 전환 규칙 변경
- FCFVehicleAimProfile 구조 변경
- FireRequest/FireResult 구조 변경
- 로컬 발사 검증 책임이 ACFVehiclePawn에서 다른 시스템으로 이동할 때
- AimVisualState 소비 정책 변경
- 무기/터렛 데이터와 AimProfile 연결 방식 변경
- bLocalWithinWeaponArc를 실제 발사 차단 조건으로 사용하기로 할 때
- ValidateFireCommand가 OutOfWeaponArc를 실제 거부 사유로 사용하게 될 때
```

---

## 25. 문서 버전 관리

- 현재 문서 버전: `1.6.0`
- 문서 상태: `Current / P0 Aim Alignment PIE Verified`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준

- `Major`
  - 기능 해석 자체가 바뀌는 수준의 대규모 재작성
  - Aim이 무기 시스템 전체 문서로 확장되거나 분리될 때
- `Minor`
  - 새로운 Aim 상태, 발사 검증 항목, 로컬 시각 상태 항목이 추가될 때
  - 무기/터렛 데이터와 실제 연결될 때
  - `bLocalWithinWeaponArc`나 `OutOfArc`의 운영 정책이 바뀔 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

---

## 26. Migration

### v1.5.0 -> v1.6.0

```text
- `bAllowFireWhileAligning=true/false` 양쪽의 정렬 중 발사 정책을 사용자 PIE 결과로 확정한다.
- 정렬 완료 후 Reticle 목표점과 실제 탄착 일치를 P0 검증 기준에 포함한다.
- TurretAligning amber, WeaponNotAligned 비가림과 MuzzleBlocked 발사 차단을 현재 동작으로 사용한다.
- 주행·거리별 정량 오차와 경계각 검증은 CF-FQ-019 확장 회귀로 관리한다.
- 코드와 DataAsset은 변경하지 않는다.
```

### v1.4.1 -> v1.5.0

```text
- Weapon Aim Solution은 bAllowFireWhileAligning, DesiredAimDirection, CurrentMuzzleDirection, 실제 최종 AimDirection을 구분한다.
- bLocalCanFire는 정책 true이면 정렬 중에도 true가 될 수 있지만 LocalReticleState는 TurretAligning을 유지한다.
- MuzzleBlocked는 실제 최종 발사 방향 경로를 기준으로 정책과 관계없이 발사를 차단한다.
- `Tools/BuildEditor.bat`는 성공했고 PIE는 Pending이다.
```

### v1.4.0 -> v1.4.1

```text
- TurretAligning은 ECFVehicleReticleState enum 주 상태로 추가됐다.
- BuildLocalReticleState는 Runtime 미준비, MuzzleBlocked, OutOfArc, TurretAligning, Ready, Hidden 순서로 해석한다.
- 정상 정렬 대기 경로는 Hidden이 아니라 TurretAligning을 반환해야 한다.
- Reticle Recovery Hotfix 빌드는 성공했지만 PIE 검증 전이므로 PIE PASS로 기록하지 않는다.
```

### v1.3.0 -> v1.4.0

```text
- Camera Aim Trace는 WeaponHit 기준으로 해석한다.
- 발사 경로는 가능한 경우 FCFVehicleWeaponAimSolution의 AimOrigin / AimDirection / AimTargetLocation을 사용한다.
- TurretAligning, WeaponNotAligned, MuzzleBlocked는 OutOfArc / AimBlocked와 원인을 구분해 기록한다.
- VehicleDebug Panel에서 Weapon Aim Solution 섹션으로 현재 조준 해를 확인한다.
- C++ 빌드는 완료됐지만 PIE에서 Reticle 목표점과 실제 탄착 일치를 별도로 검증해야 한다.
```

### v1.2.0 -> v1.3.0

```text
- 현재 Reticle 목표와 실제 Muzzle 발사 방향이 일치하지 않는 상태를 현행 한계로 기록한다.
- 현재 Camera Aim Trace는 ECC_Visibility, 실제 무기 Trace는 WeaponHit을 사용한다.
- 현재 LocalAimDirection은 차량 Actor 위치, 터렛 방향은 TurretYawPivot 위치, 최종 발사 방향은 Muzzle Socket X축을 기준으로 한다.
- 조준 정렬 구현 전까지 이 방향들을 동일한 Aim Solution으로 간주하지 않는다.
- 완료 당시 목표 구조와 코드 변경 기준은 `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md`에 보존한다.
- 현재 코드와 데이터 마이그레이션은 이 문서 갱신에서 수행하지 않는다.
```

### v1.1.0 -> v1.2.0

```text
- bWithinWeaponArc && !bAimBlocked이면 로컬 발사 가능으로 본다는 설명을 제거한다.
- 현재 기준 bLocalCanFire는 조준 방향 유효성과 bAimBlocked를 중심으로 계산되는 표시용 예측값으로 본다.
- bLocalWithinWeaponArc는 표시/디버그용 값으로 해석한다.
- OutOfArc는 현재 P0 싱글플레이 기준에서 단독 발사 차단 조건이 아니라 조준각 경고/디버그 상태로 해석한다.
- 실제 발사 성공 여부는 WeaponFire의 ValidateFireCommand() 결과를 기준으로 판단한다.
```

### v1.0.0 -> v1.1.0

```text
- 서버/RPC 기준 설명을 싱글플레이 로컬 Fire Command 기준으로 정정했다.
- FireValidationState, AimVisualState, FirePending, FireRejected 명칭을 현재 코드 기준으로 반영했다.
- 멀티플레이 RPC / 복제 설명을 현재 책임에서 제외하고 과거/미래 온라인 전환 후보로 분리했다.
```

---

## 27. Changelog

### v1.8.1 - 2026-07-21

```text
- CurrentMuzzleDirection 기반 터렛 레티클을 사용자 PIE PASS로 확정했다.
- 탄종과 착탄 위치에 독립적인 터렛 레티클 데이터 계약을 현재 검증 완료 상태로 전환했다.
```

### v1.8.0 - 2026-07-21

```text
- Weapon Aim Solution에 탄종 독립 터렛 레티클 유효성, 월드 위치와 비교 거리를 추가했다.
- CurrentMuzzleDirection과 AimTargetLocation 깊이를 사용하는 계산 계약을 현재 구현으로 기록했다.
- Legacy Weapon Preview와 Image_WeaponReticle 제품 의미를 분리했다.
- 공식 에디터 빌드 PASS와 사용자 PIE 대기 상태를 기록했다.
```

### v1.7.0 - 2026-07-21

```text
- Image_CenterDot 조준 레티클이 AimTargetLocation을 선택하고 터렛이 이를 추적하는 책임을 확정했다.
- Image_WeaponReticle 터렛 레티클의 기준을 AimDirection이 아닌 CurrentMuzzleDirection으로 확정했다.
- 터렛 레티클을 탄종과 착탄 위치에서 분리하고 투사체 착탄 위치를 후속 3D 표시로 이관했다.
- 기존 DirectImpact/LaunchDirection Preview 소비 경로를 구현 정렬 대기로 기록했다.
```

### v1.6.0 - 2026-07-14

```text
- CF-FQ-022 P0 사용자 PIE 통과를 현재 VehicleAim 상태에 반영했다.
- 정책 true/false, 정렬 완료 탄착, TurretAligning·WeaponNotAligned·MuzzleBlocked 검증을 기록했다.
- PIE Pending 상태를 P0 Aim Alignment Verified로 변경했다.
- 확장 조준 회귀는 CF-FQ-019로 이관했다.
```

### v1.5.0 - 2026-07-14

```text
- 터렛별 정렬 중 발사 허용 정책과 Aim Solution 방향 3종의 현재 구현을 반영했다.
- bLocalCanFire와 TurretAligning Reticle을 분리한 상태 계산 기준을 기록했다.
- Align Fire Policy 빌드 완료와 PIE Pending을 기록했다.
```

### v1.4.1 - 2026-07-13

```text
- Reticle Recovery Hotfix로 LocalReticleState TurretAligning 복구 기준 반영
- BuildLocalReticleState 우선순위와 bLocalCanFire 해석을 현재 코드 기준으로 정정
- BuildEditor.bat 성공과 PIE Pending 상태를 기록
```

### v1.4.0 - 2026-07-13

```text
- FCFVehicleWeaponAimSolution 기반 AimOrigin / AimDirection / AimTargetLocation 공유 구조 반영
- Camera Aim Trace가 WeaponHit 기준을 사용한다는 현재 구현 반영
- TurretAligning / WeaponNotAligned / MuzzleBlocked 거부 사유와 Debug 확인 기준 추가
- AimFireAlignment C++ 빌드 완료와 PIE Pending 상태를 분리 기록
```

### v1.3.0 - 2026-07-13

```text
- Reticle 목표점과 실제 Muzzle 발사 방향이 일치하지 않는 사용자 PIE 확인 결과 반영
- Camera, Vehicle Actor, Turret Pivot, Muzzle Socket이 서로 다른 방향 기준을 사용하는 현재 구조 기록
- Camera Aim Trace ECC_Visibility와 실제 WeaponHit Trace 채널 불일치 기록
- DesiredAimTargetLocation SSOT, Muzzle 기준 요구 방향, 터렛 정렬 오차, 총구 장애물 검사를 후속 설계로 연결
- `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md` Historical evidence 연결
```

### v1.2.0 - 2026-07-09

```text
- bLocalWithinWeaponArc를 현재 P0 싱글플레이 기준에서 표시/디버그용 상태로 명시
- bLocalCanFire와 실제 발사 성공 여부를 분리해 설명
- BuildLocalReticleState()의 현재 코드 기준 판정 순서를 정정
- OutOfArc가 단독 발사 차단 조건이 아님을 명시
- ValidateFireCommand()가 현재 OutOfArc를 기본 거부 조건으로 사용하지 않는다는 기준 추가
- AimReticle / WeaponFire / FireFeedback와 함께 갱신해야 하는 OutOfArc 정책 변경 조건 추가
```

### v1.1.0 - 2026-06-19

```text
- 현재 Aim 발사 경로를 서버/RPC 기준에서 싱글플레이 로컬 Fire Command 기준으로 정정했다.
- FireValidationState, AimVisualState, FirePending, FireRejected 명칭을 현재 코드 기준으로 반영했다.
- 멀티플레이 RPC / 복제 설명을 현재 책임에서 제외하고 과거/미래 온라인 전환 후보로 분리했다.
```

### v1.0.0 - 2026-06-02

```text
- VehicleAim 시스템 문서 최초 작성
- UCFVehicleAimComp, CFVehicleAimTypes, ACFVehiclePawn 발사 요청 흐름 기준으로 현재 기능 범위 정리
- Local Aim / Server Aim / Rep Aim Visual 책임 분리 기록
- 현재 Reticle 상태 계산 범위와 미확정 상태 기록
```

---

## 28. 마지막 확인 기준

- 확인 일시: `2026-07-14`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
  - `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Combat/WeaponFire.md`

---

## 29. Change Note

```text
- 2026-07-21: 과거 정렬 완료 후 탄착 일치 기록은 발사 방향 정합성 증거로 유지하되 터렛 레티클의 의미 정의와 분리했다.
- 2026-06-19: 현재 코드에서 제거/변경된 서버 RPC 명칭을 현재 상태 문서의 현행 설명에서 제외했다.
- 2026-07-09: OutOfArc / bLocalWithinWeaponArc를 발사 차단 기준으로 오해하지 않도록 현재 P0 싱글플레이 기준으로 재정리했다.
- 과거 멀티플레이 설계 기록은 Document/Plan/Archive/AimPlan/CF_AimNet.md와 이전 체인지로그에 남긴다.
```
