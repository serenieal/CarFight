# CarFight — CF_AimSpec

> 역할: CarFight 조준 시스템이 제공해야 할 기능 명세를 정의한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Planning

---

## 1. 목적

CarFight의 Aim 시스템은 차량 기반 카메라 조준과 무기 발사 사이의 해석 계층이다.

Aim 시스템은 다음 질문에 답해야 한다.

1. 현재 플레이어 또는 AI가 어디를 조준하고 있는가?
2. 현재 활성 무기 그룹이 그 방향을 조준할 수 있는가?
3. 로컬 클라이언트 기준으로 Reticle은 어떤 상태인가?
4. 서버 기준으로 발사가 가능한가?
5. 다른 클라이언트에게 어떤 조준 시각 상태를 보여줄 것인가?

---

## 2. 범위

### 2.1 포함 범위

1차 Aim 시스템은 다음을 포함한다.

- Local Aim Target 계산
- Local Aim Direction 계산
- Weapon Arc 판정
- Reticle State 계산
- Fire Request 데이터 구조
- Fire Result 데이터 구조
- Server Fire 검증 구조
- Replicated Aim Visual State 구조
- VehicleDebug Aim 표시 기준
- HitScan 더미 발사 검증 기준

### 2.2 제외 범위

1차 Aim 시스템은 다음을 제외한다.

- 완성형 무기 시스템
- 완성형 데미지 시스템
- Projectile 예측
- Lock-On
- AI Combat 완성 구현
- 부품 파괴 / Geometry Collection 연동
- 대규모 네트워크 최적화

---

## 3. 핵심 개념

### 3.1 Camera Aim

Camera Aim은 카메라가 보고 있는 방향이다.

현재 기준으로 `UCFVehicleCameraComp`가 담당한다.

Camera Aim은 다음 값을 제공한다.

- 카메라 기준 조준 방향
- 카메라 Trace 결과
- AimHitLocation
- 카메라 Yaw / Pitch
- 카메라 Aim Profile Clamp 상태

### 3.2 Aim Target

Aim Target은 조준 시스템이 현재 목표로 보는 월드 위치 또는 대상이다.

초기에는 CameraComp의 `AimHitLocation`을 사용한다.

미래 확장 후보:

- `AimTargetActor`
- `AimTargetComponent`
- `AimHitBoneName`
- `AimHitLocation`
- `AimHitNormal`
- `AimTargetMode`

### 3.3 Weapon Arc

Weapon Arc는 현재 활성 무기 그룹이 실제로 조준/발사할 수 있는 각도 범위다.

Camera Aim 범위와 Weapon Arc는 다를 수 있다.

예:

```text
카메라 Yaw:
  -60도 ~ +60도

전방 고정 기관총 Yaw:
  -20도 ~ +20도
```

이 경우 플레이어는 볼 수 있지만 쏠 수 없는 구간이 존재한다.

### 3.4 Reticle State

Reticle State는 소유 클라이언트에게 보여줄 조준점 상태다.

후보 enum:

- `Hidden`
- `Ready`
- `Blocked`
- `OutOfArc`
- `NoWeapon`
- `Cooldown`
- `Reloading`
- `WaitingServer`
- `ServerRejected`

### 3.5 Fire Request

Fire Request는 클라이언트가 서버에 보내는 발사 요청이다.

Fire Request는 결과가 아니다. 서버가 검증해야 하는 입력이다.

### 3.6 Fire Result

Fire Result는 서버가 발사 요청을 처리한 결과다.

Owner Client의 UI 보정과 Debug에 사용한다.

---

## 4. 기능 요구사항

### 4.1 Local Aim 계산

AimComp는 로컬에서 다음을 계산해야 한다.

- Local Aim Target Location
- Local Aim Direction
- Local Within Weapon Arc 여부
- Local Aim Blocked 여부
- Local CanFire 예측
- Local Reticle State

이 계산은 소유 클라이언트에서 즉시 반응해야 한다.

### 4.2 Server Aim 검증

서버는 Fire Request를 받을 때 다음을 검증해야 한다.

- 요청 소유권
- Pawn 유효성
- Pawn 전투 가능 상태
- 무기 그룹 유효성
- 쿨다운 상태
- 탄약 상태
- 조준 방향 유효성
- Weapon Arc 유효성
- AimOrigin 유효성
- 서버 Trace 결과

### 4.3 Replicated Aim Visual

Aim 시스템은 다른 클라이언트에게 필요한 최소 조준 시각 상태를 제공해야 한다.

복제 후보:

- Aim Direction
- Aim Target Location
- Firing Visual Flag
- Weapon Visual Mode

이 정보는 전투 판정용이 아니라 시각 표현용이다.

### 4.4 VehicleDebug Aim

Aim 시스템은 VehicleDebug에 다음 정보를 제공해야 한다.

- Local Aim Target
- Local Aim Direction
- Local Reticle State
- Local CanFire
- Server CanFire
- Server Reject Reason
- Last Fire Request ID
- Last Fire Result
- Replicated Aim Direction

### 4.5 Reticle UI

Reticle UI는 계산하지 않는다.

Reticle UI는 AimComp가 제공하는 Reticle State를 읽고 시각 표현만 담당한다.

---

## 5. 데이터 요구사항

### 5.1 Aim Profile

Aim Profile은 조준 가능 각도와 거리 제한을 가진다.

초기 필드 후보:

- `ProfileName`
- `MinYawDeg`
- `MaxYawDeg`
- `MinPitchDeg`
- `MaxPitchDeg`
- `MaxAimDistance`

### 5.2 Fire Request

필드 후보:

- `FireRequestId`
- `ClientFireTimeSeconds`
- `AimOrigin`
- `AimDirection`
- `PredictedAimTargetLocation`
- `WeaponGroupId`

### 5.3 Fire Result

필드 후보:

- `FireRequestId`
- `bAccepted`
- `RejectReason`
- `ServerAimTargetLocation`
- `HitActor`
- `HitLocation`
- `HitNormal`

초기에는 HitActor를 Debug 문자열이나 약한 참조 방식으로 제한할 수 있다.

---

## 6. 네트워크 요구사항

### 6.1 서버 권한

다음은 서버 권한이다.

- 발사 승인
- HitScan 최종 Trace
- Projectile Spawn
- Damage 적용
- 부품 파괴 적용

### 6.2 클라이언트 예측

다음은 클라이언트 예측이 가능하다.

- Reticle 표시
- Local CanFire 예측
- Muzzle Flash 예측
- Camera Shake 예측

### 6.3 보정

서버가 발사 요청을 거부하면 Owner Client는 Reticle 또는 Debug에서 거부 상태를 확인할 수 있어야 한다.

---

## 7. 실패 / 거부 사유

Fire Reject Reason 후보:

- `None`
- `NoAuthority`
- `InvalidOwner`
- `VehicleDisabled`
- `NoWeapon`
- `WeaponCooldown`
- `NoAmmo`
- `OutOfWeaponArc`
- `AimBlocked`
- `InvalidAimOrigin`
- `InvalidAimDirection`
- `ServerTraceMiss`

---

## 8. 완료 조건

1차 Aim Spec 완료 조건은 다음과 같다.

- Local Aim State 구조가 정의되어 있다.
- Server Aim State 구조가 정의되어 있다.
- Replicated Aim Visual State 구조가 정의되어 있다.
- Fire Request / Result 구조가 정의되어 있다.
- Reticle State가 정의되어 있다.
- 서버 검증 기준이 정의되어 있다.
- VehicleDebug Aim 표시 항목이 정의되어 있다.
- 1차 구현에서 제외할 범위가 명시되어 있다.
