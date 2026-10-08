# CarFight — Aim / Turret / Muzzle 정렬 구현 설계

- Version: 0.5.0
- Date: 2026-07-14
- Status: Completed / Official Build and P0 PIE Passed
- Feature: `CF-FQ-022 조준점·터렛·총구 정렬`
- Priority: P0
- Scope: 화면 Reticle이 지시하는 월드 목표점, 터렛 추적 방향, Muzzle 발사 방향, 실제 HitScan/Projectile 진행 방향을 하나의 기준으로 통합한다.

---

## 현재 작업 체크포인트

- 현재 상태: Aim Solution, 정렬 중 발사 정책, Reticle 상태와 총구 장애물 검사를 구현하고 P0 사용자 PIE까지 통과했다.
- 빌드 상태: `Tools\BuildEditor.bat` PASS
- PIE 상태: 정책 true/false, 정렬 완료 탄착, `MuzzleBlocked` PASS
- 기능 판정: `CF-FQ-022` P0 완료

### 완료한 범위

- Reticle 월드 목표점을 `DesiredAimTargetLocation` 단일 기준으로 연결
- Muzzle 위치에서 목표점으로 향하는 요구 발사 방향과 정렬 오차 계산
- `UCFTurretMountData.bAllowFireWhileAligning` 터렛별 정책 구현
- `bAllowFireWhileAligning=true`에서 정렬 중 `TurretAligning` amber를 유지하며 현재 Muzzle 방향으로 발사
- `bAllowFireWhileAligning=false`에서 정렬 중 발사 거부, 정렬 완료 후 정상 발사 승인
- 정렬 완료 후 Reticle 목표점과 실제 Projectile 탄착 일치 확인
- 정책 true/false 양쪽에서 총구 장애물 발생 시 `MuzzleBlocked`로 발사 차단
- `WeaponNotAligned`가 빨간 FireRejected로 주 Reticle을 덮지 않고 `TurretAligning` 표시를 유지하는 동작 확인
- `MuzzleBlocked`, `OutOfArc`, `TurretAligning` 상태 책임 분리
- Reticle Recovery Hotfix와 Align Fire Policy 공식 Editor 빌드 성공

### 확장 회귀 테스트로 남긴 범위

- 주행·선회 중 고정 목표 추적과 발사
- 5m / 20m / 100m 거리별 정량 탄착 오차 측정
- 터렛 최대 Yaw/Pitch 경계와 차량 Pitch/Roll 변화
- 다중 터렛과 탄도 낙하 발사체

위 항목은 P0 기능 완료를 막는 미완료가 아니라 `CF-FQ-019`와 후속 Ballistic 단계에서 수행할 확장 회귀 범위다.

### 바로 다음 작업

1. `CF-FQ-018` 최소 Damage Runtime의 입력 계약을 확정한다.
2. `BaseDamage`, 차량 체력 감소와 파괴 상태 최소 구조를 구현한다.
3. 주행 중 조준·발사 반복 검증은 `CF-FQ-019`에서 수행한다.

### 관련 코드와 문서 경로

- `UE/Source/CarFight_Re/Public/CFTurretMountData.h`
- `UE/Source/CarFight_Re/Private/CFTurretMountData.cpp`
- `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `Document/Systems/Vehicles/VehicleAim.md`
- `Document/Systems/Combat/WeaponFire.md`
- `Document/Systems/UI/AimReticle.md`

### 보호 범위

- 무기 피격 Collision Channel과 `VehicleVisualHit` Profile을 이 작업에서 다시 설계하지 않는다.
- 실제 피해 누적은 `CF-FQ-018` 범위로 유지한다.
- 사용자 PIE 확인 전에는 PASS, Done 또는 Completed로 기록하지 않는다.

---

## 1. 목적

현재 CarFight에서는 화면 Reticle이 지시하는 카메라 조준점과 실제 탄환 진행 방향이 완전히 일치하지 않을 수 있다.

목표 동작은 다음과 같다.

> 플레이어의 Reticle이 가리키는 월드 지점을 단일 조준 목표로 사용하고, 터렛과 총구가 그 지점을 향하도록 추적한 뒤 실제 HitScan과 Projectile도 동일한 발사 해를 사용한다.

이 작업은 실제 피해 처리 전에 완료해야 한다.
피격 콜리전이 정확해도 조준점과 실제 사격 방향이 다르면 전투 결과를 신뢰할 수 없기 때문이다.

---

## 2. 구현 전 확인 흐름

### 2.1 카메라 조준점

구현 전 `UCFVehicleCameraComp::UpdateAimTrace()`는 아래 기준으로 카메라 조준점을 만들었다.

```text
TraceStart     = FollowCamera 위치
TraceDirection = 카메라 현재 조준 방향
TraceChannel   = ECC_Visibility
AimHitLocation = 충돌 위치 또는 최대 거리 끝점
```

### 2.2 Local Aim 방향

구현 전 `UCFVehicleAimComp::RefreshLocalAimState()`는 다음 방향을 `LocalAimDirection`으로 저장했다.

```text
OwnerVehiclePawn Actor 위치
→ CameraRuntimeState.AimHitLocation
```

즉, 카메라 위치가 아니라 차량 Actor 중심에서 목표점으로 향하는 방향이다.

### 2.3 터렛 시각 추적

구현 전 `ACFVehiclePawn::ResolveTurretAimWorldDirection()`은 다음 방향을 우선 사용했다.

```text
TurretYawPivot 위치
→ LocalAimTargetLocation
```

이 방향을 `VehicleWeaponComp->UpdateTurretState()`에 전달해 Yaw/Pitch 시각 피벗을 회전한다.

### 2.4 실제 발사 방향

구현 전 `ACFVehiclePawn::BuildFireCommand()`는 `TryBuildMuzzleFireOrigin()` 성공 시 최종 발사 방향을 다음 값으로 교체했다.

```text
Muzzle Socket의 월드 X축
```

따라서 구현 전에는 아래 방향들이 서로 달랐다.

```text
카메라 위치       → Reticle 목표점
차량 Actor 중심   → Reticle 목표점
터렛 Yaw Pivot    → Reticle 목표점
Muzzle Socket X축 → 실제 탄환 진행 방향
```

---

## 3. 구현 배경 문제

### 3.1 시차 오차

카메라와 총구는 서로 다른 위치에 있다.
카메라 Ray와 총구 Forward Ray를 평행하게 사용하면 가까운 거리에서 큰 시차가 발생한다.

### 3.2 회전 지연 오차

터렛은 회전 속도 제한으로 목표 방향을 추적하지만, 실제 발사는 현재 Muzzle Socket X축을 즉시 사용한다.
터렛이 아직 목표에 정렬되지 않았다면 Reticle 목표점과 탄환 방향이 다르다.

### 3.3 충돌 채널 불일치

구현 전 카메라 Aim Trace는 `ECC_Visibility`, 실제 무기 HitScan은 `WeaponHit`을 사용했다.
같은 월드 위치에서도 채널 응답 차이로 서로 다른 목표점을 얻을 수 있다.

### 3.4 총구 앞 장애물 미분리

카메라는 목표물을 볼 수 있어도 총구 앞에는 차량 차체, 벽, 낮은 장애물이 있을 수 있다.
카메라 Trace만으로 발사를 허용하면 탄환이 Reticle과 다른 가까운 표면에 충돌하거나 총구가 장애물을 관통한 것처럼 보일 수 있다.

---

## 4. 확정 구조 결정

### 4.1 Reticle 월드 목표점을 조준 SSOT로 사용

다음 의미의 값을 조준 시스템의 단일 기준으로 사용한다.

```cpp
FVector DesiredAimTargetLocation;
```

이 값은 화면 Reticle이 지시하는 월드 위치다.

아래 시스템은 모두 이 값을 기준으로 파생 계산한다.

```text
- 터렛 목표 Yaw/Pitch
- 총구 요구 발사 방향
- 무기 정렬 오차
- HitScan 방향
- Projectile 발사 방향
- 총구 앞 장애물 검사
- Reticle Ready / Aligning 상태
```

### 4.2 카메라 Aim Trace와 무기 Trace 기준 통일

P0에서는 카메라 Aim Trace도 `WeaponHit` Trace Channel을 사용한다.

```text
Camera Aim Trace = WeaponHit
Dummy HitScan    = WeaponHit
Muzzle Obstruction Trace = WeaponHit
```

이렇게 해서 Reticle이 선택한 표면과 실제 무기가 충돌하는 표면을 같은 응답표로 해석한다.

장기적으로 UI 전용 Aim 채널이 필요해지면 별도 채널을 만들 수 있지만, 그 경우에도 `WeaponHit`과 충돌 응답표를 명시적으로 동기화해야 한다.

### 4.3 총구 기준 요구 발사 방향

중력 없는 직선 발사체의 요구 방향은 다음과 같이 계산한다.

```cpp
DesiredLaunchDirection =
    (DesiredAimTargetLocation - MuzzleWorldLocation).GetSafeNormal();
```

Muzzle Socket X축은 현재 총신이 실제로 향하는 방향을 측정하는 데 사용한다.
요구 발사 방향을 결정하는 최종 기준으로 사용하지 않는다.

### 4.4 터렛은 요구 발사 방향을 추적

터렛 Yaw/Pitch 목표는 `DesiredLaunchDirection`을 터렛 기준 로컬 각도로 변환해 계산한다.

```text
DesiredAimTargetLocation
→ MuzzleWorldLocation 기준 요구 방향
→ Turret Mount 기준 Yaw/Pitch
→ 회전 속도 제한으로 CurrentYaw/CurrentPitch 추적
```

### 4.5 실제 발사 전 정렬 판정

다음 상태를 명시적으로 계산한다.

```text
DesiredLaunchDirection = 총구에서 Reticle 목표점으로 향하는 요구 방향
CurrentMuzzleDirection = Muzzle Socket 현재 X축
AimAlignmentErrorDegrees = 두 방향의 각도 차이
bWeaponAlignedToAim = 오차가 허용 범위 이하인지 여부
```

터렛별 발사 정책:

```text
- UCFTurretMountData에 bAllowFireWhileAligning 스위치를 둔다.
- 기본값은 true로 하여 기존 정렬 중 발사 감각과의 호환을 우선한다.
- bAllowFireWhileAligning == false이면 bTurretAligning 또는 bWeaponNotAligned 상태에서 발사를 거부한다.
- bAllowFireWhileAligning == true이면 정렬 중에도 발사를 허용한다.
- 정렬 중 허용된 발사는 Reticle 목표 방향으로 꺾지 않고 현재 Muzzle Socket X축 방향으로 발사한다.
- 정렬 완료 후에는 Muzzle에서 Reticle 목표점으로 향하는 DesiredLaunchDirection을 사용한다.
- MuzzleBlocked는 스위치 값과 관계없이 항상 발사를 거부한다.
- 정렬 중 Reticle은 발사 가능 여부와 별개로 TurretAligning 상태와 amber 표현을 유지한다.
```

현재 구현 상태:

```text
- UCFTurretMountData.bAllowFireWhileAligning과 기본값 true가 C++에 반영됐다.
- FCFVehicleWeaponAimSolution은 DesiredAimDirection, CurrentMuzzleDirection, 실제 최종 AimDirection을 분리한다.
- 정책 true의 정렬 중 발사는 CurrentMuzzleDirection, 정렬 완료 후 발사는 DesiredAimDirection을 사용한다.
- 정책 false의 TurretAligning / WeaponNotAligned는 ValidateFireCommand에서 거부한다.
- 실제 최종 발사 경로 기준 MuzzleBlocked 검사는 정책과 관계없이 항상 거부한다.
- Tools/BuildEditor.bat는 성공했으며 사용자 PIE 확인 전까지 PIE PASS로 기록하지 않는다.
```

정렬 허용 오차의 초기 권장 범위:

```text
기본값 후보: 0.5도
기관총 계열: 0.5 ~ 1.0도
일반 포 계열: 0.2 ~ 0.5도
정밀 무기: 0.1 ~ 0.2도
```

최종 수치는 Weapon/Turret 데이터 튜닝 항목으로 둔다.

### 4.6 총구 앞 장애물 검사

카메라 목표점이 결정된 뒤 실제 최종 발사 방향을 기준으로 총구 앞 Trace를 수행한다.

```text
정렬 완료
→ FinalFireDirection = DesiredLaunchDirection

정렬 미완료 + bAllowFireWhileAligning == true
→ FinalFireDirection = CurrentMuzzleDirection

TraceStart = MuzzleWorldLocation
TraceEnd   = MuzzleWorldLocation + FinalFireDirection * 검사 거리
TraceChannel = WeaponHit
```

`bAllowFireWhileAligning == false`이고 정렬이 끝나지 않은 경우에는 정렬 거부가 우선이며 실제 발사를 실행하지 않는다.

목표점보다 가까운 Blocking Hit이 존재하면 다음으로 처리한다.

```text
bMuzzleLineBlocked = true
발사 불가
Reticle = AimBlocked 또는 전용 MuzzleBlocked 표시
```

카메라가 볼 수 있는 목표와 총구가 실제로 쏠 수 있는 목표를 분리해 검증한다.

---

## 5. Reticle 의미

현재 주 Reticle은 플레이어가 원하는 목표점을 나타내는 **Command Reticle**로 본다.

```text
Command Reticle
= DesiredAimTargetLocation을 지정하는 화면 조준점
```

터렛 정렬 중에도 Command Reticle은 목표 위치에 유지한다.

P0 UI 상태 후보:

```text
Ready          = 총구 경로가 열려 있고 터렛 정렬 완료
TurretAligning = 목표는 유효하지만 터렛 정렬 중
Blocked        = 총구 앞 또는 조준 경로 차단
Cooldown       = 무기 재사용 대기
NoWeapon       = 무기 없음
```

`TurretAligning`을 기존 `OutOfArc`에 합치지 않는다.

```text
OutOfArc       = 터렛/무기의 기계적 조준 범위 밖
TurretAligning = 범위 안이지만 현재 회전이 아직 목표에 도달하지 않음
```

향후 필요하면 현재 총구가 실제로 향하는 지점을 보여주는 **Weapon Reticle**을 보조 UI로 추가할 수 있다.
P0 필수 범위는 Command Reticle과 정렬 상태 표시까지다.

---

## 6. 중력 발사체 정책

현재 P0 정렬 구현은 중력 없는 직선 Projectile과 HitScan을 우선한다.

중력이 적용되는 발사체는 단순히 목표점 방향으로 발사하면 낙하 때문에 목표 아래에 도달한다.
후속 Ballistic 단계에서는 다음 입력으로 탄도 발사 해를 계산한다.

```text
MuzzleWorldLocation
DesiredAimTargetLocation
ProjectileInitialSpeed
GravityScale
WorldGravity
```

그 결과 얻은 `BallisticLaunchDirection`을 터렛 요구 방향과 실제 Projectile 초기 속도에 함께 사용한다.

```text
Reticle = 목표 지점
터렛/총구 = 탄도 해에 따라 목표보다 위쪽 방향
Projectile = 곡선 비행 후 목표 지점 도달
```

Ballistic Solver는 이번 P0 직선 정렬 작업 범위에서 제외한다.

---

## 7. 데이터 구조 후보

기존 타입에 다음 의미의 값을 추가하는 방향을 우선한다.

### Local Aim / Aim Solution

```text
DesiredAimTargetLocation
DesiredLaunchDirection
CurrentMuzzleDirection
AimAlignmentErrorDegrees
bWeaponAlignedToAim
bMuzzleLineBlocked
MuzzleBlockingHitLocation
```

### 터렛 데이터

```text
UCFTurretMountData.bAllowFireWhileAligning
```

확정 정책:

```text
- 정책 소유자는 WeaponData가 아니라 UCFTurretMountData다.
- 기본값은 true다.
- true이면 정렬 중 현재 Muzzle 방향 발사를 허용한다.
- false이면 정렬 완료 전 발사를 거부한다.
- 정렬 허용 오차는 기존 UCFTurretMountData.StabilizationToleranceDeg를 재사용한다.
```

현재 C++에 `bAllowFireWhileAligning`과 기본값 true가 반영됐으며, 기존 DataAsset 인스턴스는 저장하지 않았다.

### Reticle 상태

새 상태 후보:

```text
TurretAligning
```

정렬 상태를 별도 FireRejectReason으로 기록할 경우 이름 후보:

```text
WeaponNotAligned
```

최종 enum 변경은 Codex 작업지시서 작성 시 현재 타입 사용처를 확인한 뒤 확정한다.

---

## 8. 구현 순서

### Phase 1 — 현재 조준 해 Debug 분리

```text
1. Camera Trace Start/Direction/Target 기록
2. DesiredAimTargetLocation 기록
3. MuzzleWorldLocation 기록
4. CurrentMuzzleDirection 기록
5. DesiredLaunchDirection 기록
6. AimAlignmentErrorDegrees 기록
7. Muzzle obstruction 결과 기록
```

### Phase 2 — 카메라 Aim Trace 채널 통일

```text
1. UCFVehicleCameraComp가 CFCollisionChannels::WeaponHit 사용
2. Owner 차량 Ignore 유지
3. SM_Body와 월드 장애물 응답 확인
4. 기존 Camera Boom 충돌 채널은 변경하지 않음
```

### Phase 3 — Muzzle 기준 요구 방향 계산

```text
1. DesiredAimTargetLocation 유효성 검사
2. Muzzle Socket 위치 확보
3. Muzzle → Target 방향 계산
4. TurretState 목표 방향으로 전달
5. 현재 Muzzle 방향과 정렬 오차 계산
```

### Phase 4 — 발사 검증 연결

```text
1. UCFTurretMountData.bAllowFireWhileAligning 정책 읽기
2. 정렬 오차 허용 범위 검사
3. 정책 false일 때 TurretAligning / WeaponNotAligned 거부
4. 실제 최종 발사 방향 기준 총구 앞 WeaponHit Trace 검사
5. MuzzleBlocked는 정책과 관계없이 거부
6. Reticle 상태와 FireFeedback 표시 연결
```

### Phase 5 — 실제 발사 해 통일

```text
1. 정렬 완료 시 FinalFireDirection = DesiredLaunchDirection
2. 정렬 미완료 + 정책 true 시 FinalFireDirection = CurrentMuzzleDirection
3. HitScan과 Projectile은 같은 FinalFireDirection 사용
4. FireRequest.AimDirection은 실제 최종 발사 방향을 기록
5. Debug Line으로 Camera Ray, Desired Ray, Current Muzzle Ray를 구분 표시
```

### Phase 6 — PIE 검증

```text
1. 정책 true에서 터렛 회전 중 발사 승인과 현재 Muzzle 방향 탄착 확인
2. 정책 false에서 터렛 회전 중 발사 거부 확인
3. 두 정책 모두 TurretAligning amber 표시 유지 확인
4. 정렬 완료 후 Reticle 목표 방향 발사 확인
5. 총구 앞 낮은 벽에서 두 정책 모두 발사 차단 확인
6. 가까운 목표와 먼 목표에서 정렬 완료 후 Reticle/탄착 일치
7. 차량 회전/주행 중 목표 추적과 상태 복귀 확인
```

---

## 9. C++ / BP 책임 분리

### C++

```text
- Reticle 목표점 월드 계산
- Camera WeaponHit Trace
- Muzzle 기준 요구 발사 방향 계산
- 터렛 목표각과 현재각 비교
- 정렬 오차와 발사 가능 여부 계산
- 총구 앞 장애물 Trace
- FireRequest / FireResult / RejectReason 기록
- HitScan / Projectile 발사 방향 통일
- Debug 데이터와 로그 제공
```

### BP / WBP

```text
- Reticle 이미지 배치와 시각 스타일
- TurretAligning / Ready / Blocked 상태 표시
- 정렬 오차에 따른 애니메이션 또는 색상 표현
- Muzzle, YawPivot, PitchPivot 소켓/컴포넌트 구성 유지
- 계산과 발사 판정 로직은 BP에 추가하지 않음
```

### DataAsset

```text
- 터렛 Yaw/Pitch 속도
- 기계적 조준 범위
- 정렬 허용 오차
- 정렬 중 발사 허용 여부 (`bAllowFireWhileAligning`, 기본값 `true`)
- 후속 Ballistic 관련 발사체 속도/중력 값
```

---

## 10. 완료 기준

```text
1. Reticle 목표점이 단일 DesiredAimTargetLocation으로 기록된다.
2. Camera Aim Trace와 실제 무기 Trace가 같은 WeaponHit 응답 기준을 사용한다.
3. 터렛이 Muzzle → DesiredAimTargetLocation 방향을 추적한다.
4. CurrentMuzzleDirection과 DesiredLaunchDirection의 오차가 Debug에 표시된다.
5. `bAllowFireWhileAligning=false`인 터렛은 정렬 오차가 허용 범위보다 크면 발사되지 않는다.
6. `bAllowFireWhileAligning=true`인 터렛은 정렬 중 현재 Muzzle 방향으로 발사하고, 정렬 완료 후 HitScan과 Projectile이 Reticle 목표점을 향한다.
7. 카메라와 총구 위치 차이가 있어도 가까운 목표에서 탄착이 목표점으로 수렴한다.
8. 총구 앞 장애물이 있으면 발사가 차단된다.
9. OutOfArc와 TurretAligning이 의미상 분리된다.
10. 주행 중 카메라, 터렛, Reticle 갱신이 끊기지 않는다.
11. Unreal Editor 타깃 빌드가 성공한다.
12. 싱글 PIE 검증을 통과한다.
```

---

## 11. 테스트 기준

### 정지 상태

```text
- 5m / 20m / 100m 목표에서 Reticle과 탄착 비교
- 화면 좌우 가장자리 조준
- 높은 목표 / 낮은 목표 조준
- 터렛 최대 Yaw/Pitch 근처 조준
```

### 이동 상태

```text
- 직진 중 고정 목표 조준
- 선회 중 고정 목표 조준
- 차량 Pitch/Roll 변화 중 목표 추적
- 카메라 회전과 차량 회전을 동시에 수행
```

### 장애물

```text
- 카메라는 볼 수 있지만 총구 앞이 막힌 낮은 벽
- 차량 차체 가까이에서 발사
- 목표 앞에 다른 차량 또는 월드 장애물 배치
```

### PASS 판단

```text
- Reticle 목표와 실제 탄착이 허용 오차 안에서 일치
- `bAllowFireWhileAligning=true`이면 정렬 중 현재 Muzzle 방향으로 발사되고, `false`이면 발사가 거부된다.
- 두 정책 모두 정렬 중 Reticle은 `TurretAligning` 상태로 유지된다.
- 총구 막힘이 AimBlocked와 구분 가능
- 발사 후 DamageHitContext 방향과 목표점이 일관됨
```

---

## 12. 이번 범위에서 제외

```text
- 탄도 낙하 보정 Solver
- 목표 이동량 선행 조준
- 자동 락온
- 네트워크 지연 보정
- 다중 터렛 동시 조준
- 휠/터렛 부위 피해
- 실제 HP 감소
- 완성형 Weapon Reticle 보조 UI
- 조준 보정 Aim Assist
```

---

## 13. 튜닝 및 미확정 항목

```text
- 기본 FireAlignmentToleranceDegrees 최종값
- 기존 DataAsset에서 `bAllowFireWhileAligning` true/false 양쪽 PIE 확인
- TurretAligning 전용 Reticle 상태 enum은 추가 완료. 기존 FireFeedback DisplayKey 보조 표시도 유지
- 총구 막힘을 AimBlocked에 합칠지 MuzzleBlocked로 분리할지
- 직선 Projectile 이후 Ballistic Solver 착수 시점
- Weapon Reticle 보조 UI 도입 시점
```

구조 원칙은 확정이며 위 항목은 구현 시 데이터와 UI 범위를 조정하는 튜닝 사항이다.

---

## 14. Migration

### v0.4.1 -> v0.5.0

```text
- `bAllowFireWhileAligning=true` 정렬 중 발사와 현재 Muzzle 방향 진행을 사용자 PIE PASS로 기록했다.
- `bAllowFireWhileAligning=false` 정렬 중 발사 거부와 정렬 완료 후 승인을 사용자 PIE PASS로 기록했다.
- 정책 true/false 양쪽에서 `MuzzleBlocked`가 발사를 차단하는 동작을 확인했다.
- 정렬 완료 후 Reticle 목표점과 실제 탄착 일치를 확인해 CF-FQ-022를 P0 완료로 판정했다.
- 주행·거리별 정량 오차·경계각 검증은 CF-FQ-019 확장 회귀로 이관했다.
- 코드와 DataAsset 기본값은 변경하지 않았다.
```

### v0.3.1 -> v0.4.0

```text
- 문서 상단에 다중 작업 세션 복원용 현재 작업 체크포인트를 추가했다.
- 기존 Aim Solution, 정렬 중 발사 정책, 코드와 에셋 동작은 변경하지 않았다.
- 새 세션은 체크포인트와 실제 Git 상태·코드를 교차검증한 뒤 PIE Pending 단계부터 재개한다.
```

### v0.3.0 -> v0.3.1

```text
- `UCFTurretMountData.bAllowFireWhileAligning=true` 기본값과 한국어 DisplayName/ToolTip을 C++에 반영했다.
- `FCFVehicleWeaponAimSolution.AimDirection`은 실제 최종 발사 방향이며, 요구 방향과 현재 Muzzle 방향은 별도 필드로 분리했다.
- HitScan과 Projectile은 동일한 `FireRequest.AimDirection`을 사용한다.
- 기존 DataAsset 인스턴스와 WBP/.uasset은 이번 작업에서 저장하거나 변경하지 않았다.
- `Tools/BuildEditor.bat`는 성공했고 PIE는 사용자 확인 전까지 Pending이다.
```

### v0.2.1 -> v0.3.0

```text
- 정렬 중 발사 정책을 `UCFTurretMountData.bAllowFireWhileAligning`으로 소유한다.
- 기본값은 true로 하며, 기존 DataAsset 인스턴스는 코드 기본값을 사용하므로 이번 문서 단계에서 `.uasset`을 저장하지 않는다.
- true인 터렛은 정렬 중 현재 Muzzle 방향으로 발사하고, false인 터렛은 기존처럼 정렬 완료 전 발사를 거부한다.
- MuzzleBlocked와 TurretAligning Reticle 표현은 정책값과 관계없이 기존 의미를 유지한다.
- 현재는 설계 및 Codex 작업지시서만 완료됐으며 코드/빌드/PIE 완료로 기록하지 않는다.
```

### v0.1.0 -> v0.2.0

```text
- Core C++ 구현은 완료됐고 Unreal Editor 타깃 빌드는 성공했다.
- Presentation C++ 연결은 완료됐고 기존 WBP 자산/바인딩 이름은 변경하지 않았다.
- Reticle Recovery Hotfix에서 TurretAligning은 ReticleState enum 주 상태로 추가했고, 기존 FireFeedback DisplayKey 보조 표시도 유지한다.
- WeaponNotAligned는 빨간 FireRejected로 주 Reticle을 덮지 않는다.
- MuzzleBlocked는 AimBlocked / Blocked 표시 경로를 사용한다.
- Reticle Recovery Hotfix 빌드는 에디터 종료 후 재실행해 성공했다.
- PIE 검증은 아직 Pending이므로 완료 기준 12번은 PASS 처리하지 않는다.
```

### 문서 생성 시점

```text
- 현재 코드, BP, WBP, DataAsset은 변경하지 않는다.
- 기존 Reticle과 FireFeedback 표시를 유지한다.
- 구현 시 Camera Aim Trace 채널, FireRequest AimDirection 의미, Reticle 상태가 변경될 수 있다.
- 기존 OutOfArc가 정렬 중 상태로 오용되지 않도록 신규 상태 또는 플래그로 분리한다.
- 실제 enum/구조체 변경이 생기면 BP 재컴파일과 관련 Systems 문서 갱신 지침을 별도로 작성한다.
```

---

## 15. Changelog

### v0.5.0 - 2026-07-14

```text
- 정책 true/false 정렬 중 발사 동작과 정렬 완료 탄착을 사용자 PIE PASS로 기록했다.
- 두 정책 모두 MuzzleBlocked 발사 차단과 Reticle 상태 유지가 정상임을 기록했다.
- CF-FQ-022를 P0 Completed로 판정했다.
- 수행하지 않은 주행·거리별 정량 검증은 CF-FQ-019 확장 회귀로 이관했다.
```

### v0.4.0 - 2026-07-14

```text
- 다중 작업 세션 복원을 위한 표준 현재 작업 체크포인트 추가
- 코드·공식 빌드 PASS와 사용자 PIE Pending 상태를 상단에서 즉시 확인하도록 정리
- 완료 범위, 미검증 항목, 다음 작업, 관련 경로와 보호 범위를 명시
```

### v0.3.1 - 2026-07-14

```text
- Align Fire Policy C++ 구현과 `BuildEditor.bat` 성공 결과를 반영했다.
- 요구 방향, 현재 Muzzle 방향, 실제 최종 발사 방향의 런타임 의미를 현재 코드 기준으로 확정했다.
- MuzzleBlocked와 TurretAligning 표시 정책을 유지하고 PIE Pending 상태를 기록했다.
```

### v0.3.0 - 2026-07-14

```text
- `UCFTurretMountData.bAllowFireWhileAligning` 터렛별 정책을 확정했다.
- 기본값 true, true일 때 정렬 중 현재 Muzzle 방향 발사, false일 때 정렬 완료 전 발사 거부 기준을 기록했다.
- MuzzleBlocked는 항상 거부하고 TurretAligning Reticle은 발사 허용 여부와 분리해 유지하는 원칙을 기록했다.
- Codex 작업지시서 생성 완료, 코드/빌드/PIE는 Pending으로 구분했다.
```

### v0.2.1 - 2026-07-13

```text
- Reticle Recovery Hotfix 코드 완료 상태 반영
- TurretAligning을 ECFVehicleReticleState enum 주 상태로 추가한 현재 기준 반영
- UCFAimReticleWidget의 상태 기반 Hidden이 Root Visibility 대신 RenderOpacity를 사용하도록 변경한 기준 반영
- Reticle Recovery Hotfix 빌드 성공과 PIE Pending 상태 기록
```

### v0.2.0 - 2026-07-13

```text
- AimFireAlignment Core / Presentation C++ 빌드 완료 상태 반영
- TurretAligning / WeaponNotAligned / MuzzleBlocked UI/Debug 연결 상태 반영
- PIE Pending 상태를 명시해 Build PASS와 PIE PASS를 분리
```

### v0.1.0 - 2026-07-13

```text
- 카메라 목표점, 차량 중심 방향, 터렛 방향, Muzzle Socket X축이 서로 다른 현재 구조 기록
- Reticle 월드 목표점을 DesiredAimTargetLocation SSOT로 사용하는 구조 결정
- Muzzle → Target 요구 방향, 터렛 정렬 오차, 정렬 완료 후 발사 정책 정의
- Camera / HitScan / Muzzle Trace의 WeaponHit 채널 통일 방향 정의
- 총구 앞 장애물 검사와 TurretAligning 상태 분리 기준 정의
- 직선 Projectile 우선, 중력 탄도 Solver 후속 정책 정의
```

---

## 16. 확인 근거

- 확인 일시: 2026-07-14
- 확인 대상:
  - `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`
  - `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Combat/WeaponFire.md`
  - 2026-07-13 사용자 PIE 관찰: Reticle 목표와 실제 발사 방향 불일치
