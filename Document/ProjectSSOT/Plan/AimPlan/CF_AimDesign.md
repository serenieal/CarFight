# CarFight — CF_AimDesign

> 역할: Aim 시스템의 C++ / BP 구조 설계를 정의한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Planning

---

## 1. 설계 목표

Aim 시스템은 다음 목표를 만족해야 한다.

1. CameraComp와 책임을 분리한다.
2. WeaponComp가 없어도 1차 조준 판정이 가능해야 한다.
3. 미래 WeaponComp, Damage, Lock-On, AI Combat, Destruction과 연결될 수 있어야 한다.
4. Local Aim, Server Aim, Replicated Aim Visual을 분리한다.
5. 멀티플레이에서 서버 권한 전투 판정을 막지 않는다.
6. BP는 시각 표현과 에셋 연결에 집중한다.

---

## 2. 추천 파일 구조

파일명은 32자 이하를 유지한다.

```text
UE/Source/CarFight_Re/Public/
  CFVehicleAimTypes.h
  CFVehicleAimData.h
  CFVehicleAimComp.h

UE/Source/CarFight_Re/Private/
  CFVehicleAimData.cpp
  CFVehicleAimComp.cpp
```

Reticle C++ 부모가 필요해지면 다음 파일을 사용한다.

```text
UE/Source/CarFight_Re/Public/UI/
  CFAimReticleWidget.h

UE/Source/CarFight_Re/Private/UI/
  CFAimReticleWidget.cpp
```

---

## 3. ACFVehiclePawn 통합 구조

AimComp는 `ACFVehiclePawn`의 컴포넌트로 추가한다.

권장 구조:

```text
ACFVehiclePawn
  ├─ UCFVehicleDriveComp
  ├─ UCFWheelSyncComp
  ├─ UCFVehicleCameraComp
  ├─ UCFVehicleAimComp
  └─ UCFVehicleWeaponComp   // 미래
```

`ACFVehiclePawn`은 다음 역할만 가진다.

- AimComp 생성 / 소유
- Input Action 바인딩
- Debug Snapshot에 Aim 카테고리 연결
- Server RPC 진입점을 미래 WeaponComp 또는 AimComp로 위임

---

## 4. UCFVehicleAimComp 책임

`UCFVehicleAimComp`는 다음을 담당한다.

- CameraComp에서 Aim Source 읽기
- Local Aim State 계산
- Active Aim Profile 해석
- Weapon Arc 판정
- Reticle State 계산
- Server Fire Request용 데이터 생성
- Server Fire 검증 보조
- Replicated Aim Visual State 제공
- VehicleDebug Aim Snapshot 제공

담당하지 않는다.

- 카메라 회전
- 실제 무기 쿨다운 완성 관리
- Damage 적용
- Projectile 완성 예측
- Reticle 애니메이션

---

## 5. 주요 타입 설계

### 5.1 ECFVehicleReticleState

Reticle 표시 상태다.

후보:

```text
Hidden
Ready
Blocked
OutOfArc
NoWeapon
Cooldown
Reloading
WaitingServer
ServerRejected
```

### 5.2 ECFVehicleFireRejectReason

서버 발사 거부 사유다.

후보:

```text
None
NoAuthority
InvalidOwner
VehicleDisabled
NoWeapon
WeaponCooldown
NoAmmo
OutOfWeaponArc
AimBlocked
InvalidAimOrigin
InvalidAimDirection
ServerTraceMiss
```

### 5.3 FCFVehicleAimProfile

무기 또는 조준 그룹의 조준 가능 범위다.

필드 후보:

- `ProfileName`
- `MinYawDeg`
- `MaxYawDeg`
- `MinPitchDeg`
- `MaxPitchDeg`
- `MaxAimDistance`

### 5.4 FCFVehicleLocalAimState

로컬 클라이언트 조준 상태다.

필드 후보:

- `LocalAimTargetLocation`
- `LocalAimDirection`
- `LocalReticleState`
- `bLocalCanFire`
- `bLocalWithinWeaponArc`
- `bLocalAimBlocked`

### 5.5 FCFVehicleServerAimState

서버 검증 조준 상태다.

필드 후보:

- `ServerAimTargetLocation`
- `bServerWithinWeaponArc`
- `bServerCanFire`
- `LastServerRejectReason`
- `LastAcceptedFireRequestId`
- `LastRejectedFireRequestId`

### 5.6 FCFVehicleRepAimVisualState

다른 클라이언트에게 보여줄 최소 시각 상태다.

필드 후보:

- `RepAimDirection`
- `RepAimTargetLocation`
- `bIsFiringVisual`
- `RepWeaponVisualMode`

### 5.7 FCFVehicleFireRequest

클라이언트가 서버에 보내는 발사 요청이다.

필드 후보:

- `FireRequestId`
- `ClientFireTimeSeconds`
- `AimOrigin`
- `AimDirection`
- `PredictedAimTargetLocation`
- `WeaponGroupId`

### 5.8 FCFVehicleFireResult

서버가 Owner Client에 보내는 발사 결과다.

필드 후보:

- `FireRequestId`
- `bAccepted`
- `RejectReason`
- `ServerAimTargetLocation`
- `ServerHitLocation`
- `ServerHitNormal`

---

## 6. UCFVehicleAimComp 함수 후보

### 6.1 Public 함수

```text
InitializeAimRuntime()
RefreshLocalAimState(float DeltaSeconds)
GetLocalAimState()
GetServerAimState()
GetRepAimVisualState()
GetReticleState()
CanFireLocalPredicted()
BuildFireRequest()
ValidateFireRequestOnServer()
BuildDebugAimSnapshot()
```

### 6.2 Protected 함수

```text
ResolveOwnerVehiclePawn()
ResolveVehicleCameraComp()
ResolveActiveAimProfile()
CalculateAimDirectionFromCamera()
CalculateAimAnglesRelativeToVehicle()
IsAimWithinProfile()
BuildReticleState()
UpdateRepAimVisualState()
```

---

## 7. Tick / Update 정책

초기 구현에서는 AimComp가 Tick을 가질 수 있다.

Owner Client에서는 매 프레임 Local Aim State를 갱신한다.

서버에서는 발사 요청 시점에 Fire Request를 검증한다.

다른 클라이언트에서는 Replicated Aim Visual State를 기반으로 시각 보간만 수행한다.

권장 흐름:

```text
Owner Client Tick
  -> CameraRuntimeState 읽기
  -> LocalAimState 갱신
  -> ReticleState 갱신

Server Fire Request
  -> FireRequest 검증
  -> ServerAimState 갱신
  -> FireResult 생성

Simulated Proxy
  -> RepAimVisualState 기반 시각 표시
```

---

## 8. Network 설계 위치

1차에서는 RPC를 `ACFVehiclePawn` 또는 `UCFVehicleAimComp`에 둘 수 있다.

권장 방향은 다음과 같다.

```text
초기:
  ACFVehiclePawn에 ServerRequestFire 더미 RPC 배치

후속:
  UCFVehicleWeaponComp가 생기면 발사 RPC를 WeaponComp로 이동 또는 위임
```

이유:

- 현재 WeaponComp가 없으므로 Pawn에 두는 편이 단순하다.
- 향후 무기 상태가 늘어나면 WeaponComp가 서버 발사 권한을 갖는 편이 자연스럽다.

---

## 9. BP 역할

BP가 담당할 항목:

- `UCFVehicleAimComp` 기본값 연결 확인
- Reticle Widget 배치
- Reticle 색상 / 모양 / 애니메이션
- AimData / WeaponData 에셋 연결
- 테스트용 더미 무기 위치 지정

BP가 담당하지 않을 항목:

- 발사 가능 여부 최종 판정
- 서버 검증
- Damage 적용
- 네트워크 권한 판정
- Weapon Arc 계산 규칙

---

## 10. VehicleDebug 통합

`FCFVehicleDebugSnapshot`에 Aim 카테고리를 추가한다.

후보 구조:

```text
FCFVehicleDebugAim
  LocalAimTargetLocation
  LocalAimDirection
  LocalReticleState
  bLocalCanFire
  bServerCanFire
  LastFireRequestId
  LastFireRejectReason
  RepAimDirection
```

`UCFVehicleDebugPanelWidget`에는 `BuildAimSectionViewData()`를 추가한다.

Navigation 순서는 Camera 다음 또는 Camera와 Weapon 사이가 적절하다.

```text
Overview
Drive
Input
Camera
Aim
Runtime
```

WeaponComp가 생긴 뒤에는 다음 순서도 가능하다.

```text
Overview
Drive
Input
Camera
Aim
Weapon
Network
Runtime
```

---

## 11. DataAsset 설계 방향

초기에는 AimProfile을 AimComp 내부 기본값으로 둔다.

후속으로 다음 DataAsset을 고려한다.

```text
UCFVehicleAimData
  - DefaultAimProfile
  - AimProfileArray

UCFVehicleWeaponData
  - WeaponGroupArray
  - WeaponMountArray
```

`UCFVehicleData`는 직접 모든 전투 데이터를 품기보다 참조 축으로 확장하는 것이 안전하다.

---

## 12. 설계상 금지 사항

- CameraComp 안에 무기 발사 판정을 넣지 않는다.
- Reticle Widget에서 Trace를 직접 수행하지 않는다.
- 클라이언트 Fire Result를 서버가 그대로 믿지 않는다.
- Aim Runtime State 전체를 모든 클라이언트에 매 프레임 복제하지 않는다.
- PoliceCar 전용 하드코딩을 AimComp에 넣지 않는다.
- Damage를 AimComp에서 직접 적용하지 않는다.

---

## 13. 1차 설계 완료 조건

- AimComp 책임이 CameraComp / WeaponComp와 분리되어 있다.
- Local / Server / Replicated 상태가 분리되어 있다.
- Reticle State가 정의되어 있다.
- Fire Request / Result 구조가 정의되어 있다.
- VehicleDebug Aim 연결 방식이 정의되어 있다.
- BP와 C++ 책임 경계가 정의되어 있다.
