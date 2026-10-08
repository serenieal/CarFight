# CarFight — CF_AimSpec

> 역할: CarFight 조준 시스템이 제공해야 할 기능 명세를 정의한다.
> 문서 버전: v0.4.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Draft / Single Player Planning + Local Fire Rename

---

## 1. 목적

CarFight의 Aim 시스템은 차량 기반 카메라 조준과 무기 발사 사이의 해석 계층이다.

Aim 시스템은 다음 질문에 답해야 한다.

1. 현재 플레이어 또는 AI가 어디를 조준하고 있는가?
2. 현재 활성 무기 그룹이 그 방향을 조준할 수 있는가?
3. 플레이어 화면 기준으로 Reticle은 어떤 상태인가?
4. 싱글플레이 로컬 전투 흐름에서 발사가 가능한가?
5. 로컬 HitScan / Projectile / Damage 연결에 어떤 결과 데이터를 넘길 것인가?

---

## 2. 범위

### 2.1 포함 범위

1차 Aim 시스템은 다음을 포함한다.

- Local Aim Target 계산
- Local Aim Direction 계산
- Weapon Arc 판정
- Reticle State 계산
- Fire Command 데이터 구조
- Fire Result 데이터 구조
- 로컬 Fire 검증 구조
- VehicleDebug Aim 표시 기준
- 로컬 HitScan 더미 발사 검증 기준

### 2.2 제외 범위

1차 Aim 시스템은 다음을 제외한다.

- 완성형 무기 시스템
- 완성형 데미지 시스템
- Projectile 예측
- Lock-On
- AI Combat 완성 구현
- 부품 파괴 / Geometry Collection 연동
- 멀티플레이 RPC / 복제 / Dedicated Server 검증

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

Reticle State는 플레이어에게 보여줄 조준점 상태다.

후보 enum:

- `Hidden`
- `Ready`
- `Blocked`
- `OutOfArc`
- `NoWeapon`
- `Cooldown`
- `Reloading`
- `FireBlocked`
- `FireConfirmed`

### 3.5 Fire Command

Fire Command는 플레이어 또는 AI가 발사 입력 순간에 만든 로컬 발사 명령이다.

Fire Command는 결과가 아니다. Aim / Weapon / Combat 계층이 검증해야 하는 입력이다.

주의:

기존 코드나 이전 문서에 `Fire Request` 이름이 남아 있으면, 싱글플레이 전환 완료 전까지는 같은 개념의 레거시 명칭으로 취급한다.

### 3.6 Fire Result

Fire Result는 로컬 전투 흐름이 발사 명령을 처리한 결과다.

Reticle, VehicleDebug, Hit Marker, FX 연결에 사용한다.

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

이 계산은 플레이어 입력에 즉시 반응해야 한다.

### 4.2 Local Fire 검증

로컬 전투 흐름은 Fire Command를 처리할 때 다음을 검증해야 한다.

- Pawn 유효성
- Pawn 전투 가능 상태
- 무기 그룹 유효성
- 쿨다운 상태
- 탄약 상태
- 조준 방향 유효성
- Weapon Arc 유효성
- AimOrigin 유효성
- 로컬 Trace 결과

### 4.3 Local Fire Visual

Aim 시스템은 로컬 화면과 디버그에 필요한 최소 발사 시각 상태를 제공해야 한다.

표시 후보:

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
- Local Fire Reject Reason
- Last Fire Command ID
- Last Fire Result
- Local Hit Location

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

### 5.2 Fire Command

필드 후보:

- `FireRequestId`
- `FireTimeSeconds`
- `AimOrigin`
- `AimDirection`
- `PredictedAimTargetLocation`
- `WeaponGroupId`

### 5.3 Fire Result

필드 후보:

- `FireRequestId`
- `bAccepted`
- `RejectReason`
- `AimTargetLocation`
- `HitActor`
- `HitLocation`
- `HitNormal`

초기에는 HitActor를 Debug 문자열이나 약한 참조 방식으로 제한할 수 있다.

---

## 6. 싱글플레이 판정 요구사항

### 6.1 로컬 전투 판정

다음은 싱글플레이 로컬 전투 흐름에서 확정한다.

- 발사 승인
- HitScan 최종 Trace
- Projectile Spawn
- Damage 적용
- 부품 파괴 적용

### 6.2 즉시 피드백

다음은 입력 직후 바로 표시할 수 있다.

- Reticle 표시
- Local CanFire 예측
- Muzzle Flash 예측
- Camera Shake 예측

### 6.3 실패 피드백

로컬 검증이 발사를 거부하면 Reticle 또는 Debug에서 거부 상태를 확인할 수 있어야 한다.

---

## 7. 실패 / 거부 사유

Fire Reject Reason 후보:

- `None`
- `VehicleDisabled`
- `NoWeapon`
- `WeaponCooldown`
- `NoAmmo`
- `OutOfWeaponArc`
- `AimBlocked`
- `InvalidAimOrigin`
- `InvalidAimDirection`
- `TraceMiss`

---

## 8. 완료 조건

1차 Aim Spec 완료 조건은 다음과 같다.

- Local Aim State 구조가 정의되어 있다.
- Local Fire Validation 구조가 정의되어 있다.
- Fire Command / Result 구조가 정의되어 있다.
- Reticle State가 정의되어 있다.
- 로컬 발사 검증 기준이 정의되어 있다.
- VehicleDebug Aim 표시 항목이 정의되어 있다.
- 1차 구현에서 제외할 범위가 명시되어 있다.

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 서버 요청/서버 검증/복제 시각 상태 중심 명세를 싱글플레이 로컬 판정 기준으로 전환했다.
  - `Fire Request`는 레거시 명칭으로 남기고, 문서상 의미를 `Fire Command`로 재정의했다.
  - 완료 조건에서 RPC, 복제, Dedicated Server 전제를 제거했다.
- v0.4.0 / 2026-06-19
  - Reticle 상태 이름을 현재 코드의 `FirePending`, `FireRejected`, `InvalidLocalState`, `TraceMiss` 기준으로 정리했다.
  - Pawn 단위 서버/RPC Fire 경로 제거 후 남은 `FCFVehicleFireRequest` 의미를 로컬 Fire Command 데이터로 고정했다.

## 마이그레이션 지침

- 새 문서와 신규 구현 지시는 `Fire Command`, `Local Fire Validation`, `Local Hit Result` 용어를 우선 사용한다.
- 현재 남은 `FCFVehicleFireRequest` 이름은 네트워크 요청이 아니라 로컬 Fire Command 데이터로 해석한다.
- Reticle 상태는 현재 코드 기준 `FirePending`, `FireRejected`, `InvalidLocalState`, `TraceMiss`를 사용한다.

## Change Note

- 2026-06-19: 과거 enum 후보인 `WaitingServer`, `ServerRejected` 안내를 현재 적용된 명칭으로 교체했다.
