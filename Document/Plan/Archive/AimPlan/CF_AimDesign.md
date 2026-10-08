# CarFight — CF_AimDesign

> 역할: Aim 시스템의 C++ / BP 구조 설계를 정의한다.
> 문서 버전: v0.4.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Draft / Single Player Planning + Local Fire Rename

---

## 1. 설계 목표

Aim 시스템은 다음 목표를 만족해야 한다.

1. CameraComp와 책임을 분리한다.
2. WeaponComp가 없어도 1차 조준 판정이 가능해야 한다.
3. 미래 WeaponComp, Damage, Lock-On, AI Combat, Destruction과 연결될 수 있어야 한다.
4. Local Aim, Local Fire Validation, Local Hit Result, Feedback을 분리한다.
5. 싱글플레이에서 로컬 전투 판정을 단순하고 검증 가능하게 유지한다.
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
- 로컬 Fire 입력 진입점을 미래 WeaponComp 또는 AimComp로 위임

---

## 4. UCFVehicleAimComp 책임

`UCFVehicleAimComp`는 다음을 담당한다.

- CameraComp에서 Aim Source 읽기
- Local Aim State 계산
- Active Aim Profile 해석
- Weapon Arc 판정
- Reticle State 계산
- Fire Command용 데이터 생성
- 로컬 Fire 검증 보조
- 로컬 Fire Visual State 제공
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
FireBlocked
FireConfirmed
```

### 5.2 ECFVehicleFireRejectReason

로컬 발사 거부 사유다.

후보:

```text
None
VehicleDisabled
NoWeapon
WeaponCooldown
NoAmmo
OutOfWeaponArc
AimBlocked
InvalidAimOrigin
InvalidAimDirection
TraceMiss
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

플레이어 또는 AI의 로컬 조준 상태다.

필드 후보:

- `LocalAimTargetLocation`
- `LocalAimDirection`
- `LocalReticleState`
- `bLocalCanFire`
- `bLocalWithinWeaponArc`
- `bLocalAimBlocked`

### 5.5 FCFVehicleFireValidationState

로컬 발사 검증 상태다.

필드 후보:

- `AimTargetLocation`
- `bWithinWeaponArc`
- `bCanFire`
- `LastFireRejectReason`
- `LastAcceptedFireCommandId`
- `LastRejectedFireCommandId`

주의:

현재 코드 기준 타입명은 `FCFVehicleFireValidationState`다. 과거 `ServerAimState` 표현은 현행 설계 기준에서 사용하지 않는다.

### 5.6 FCFVehicleAimVisualState

로컬 화면에 보여줄 최소 시각 상태다.

필드 후보:

- `AimDirection`
- `AimTargetLocation`
- `bIsFiringVisual`
- `WeaponVisualMode`

### 5.7 FCFVehicleFireCommand

플레이어 또는 AI가 발사 입력 순간에 만드는 로컬 발사 명령이다.

필드 후보:

- `FireRequestId`
- `FireTimeSeconds`
- `AimOrigin`
- `AimDirection`
- `PredictedAimTargetLocation`
- `WeaponGroupId`

주의:

현재 구현에 `FCFVehicleFireRequest` 이름이 남아 있다. 이 이름은 네트워크 요청이 아니라 로컬 Fire Command 데이터 의미로 해석한다.

### 5.8 FCFVehicleFireResult

로컬 전투 흐름이 생성하는 발사 결과다.

필드 후보:

- `FireRequestId`
- `bAccepted`
- `RejectReason`
- `AimTargetLocation`
- `HitLocation`
- `HitNormal`

---

## 6. UCFVehicleAimComp 함수 후보

### 6.1 Public 함수

```text
InitializeAimRuntime()
RefreshLocalAimState(float DeltaSeconds)
GetLocalAimState()
GetFireValidationState()
GetAimVisualState()
GetReticleState()
CanFireLocalPredicted()
BuildFireCommand()
ValidateFireCommand()
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
UpdateAimVisualFromFireResult()
```

---

## 7. Tick / Update 정책

초기 구현에서는 AimComp가 Tick을 가질 수 있다.

플레이어 Pawn에서는 매 프레임 Local Aim State를 갱신한다.

발사 입력 시점에는 Fire Command를 만들고 로컬 검증을 수행한다.

Fire Visual State는 Reticle, Debug, FX 연결에 필요한 표시 상태만 가진다.

권장 흐름:

```text
Player Tick
  -> CameraRuntimeState 읽기
  -> LocalAimState 갱신
  -> ReticleState 갱신

Local Fire Command
  -> FireCommand 검증
  -> FireValidationState 갱신
  -> FireResult 생성

Local Feedback
  -> AimVisualState 기반 시각 표시
```

---

## 8. 싱글플레이 Fire 설계 위치

1차에서는 Fire 입력 처리를 `ACFVehiclePawn` 또는 `UCFVehicleAimComp`에 둘 수 있다.

권장 방향은 다음과 같다.

```text
초기:
  ACFVehiclePawn에 HandleFireStarted / LocalFireCommand 배치

후속:
  UCFVehicleWeaponComp가 생기면 발사 실행을 WeaponComp로 이동 또는 위임
```

이유:

- 현재 WeaponComp가 없으므로 Pawn에 두는 편이 단순하다.
- 향후 무기 상태가 늘어나면 WeaponComp가 탄약, 쿨다운, 발사 실행을 갖는 편이 자연스럽다.

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
- Damage 적용
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
  bLocalFireAccepted
  LastFireCommandId
  LastFireRejectReason
  LocalHitLocation
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
- UI 표시 상태를 전투 판정 결과처럼 사용하지 않는다.
- Aim Runtime State를 네트워크 복제 전제로 설계하지 않는다.
- PoliceCar 전용 하드코딩을 AimComp에 넣지 않는다.
- Damage를 AimComp에서 직접 적용하지 않는다.

---

## 13. 1차 설계 완료 조건

- AimComp 책임이 CameraComp / WeaponComp와 분리되어 있다.
- Local Aim / Fire Validation / Fire Visual 상태가 분리되어 있다.
- Reticle State가 정의되어 있다.
- Fire Command / Result 구조가 정의되어 있다.
- VehicleDebug Aim 연결 방식이 정의되어 있다.
- BP와 C++ 책임 경계가 정의되어 있다.

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 설계 목표를 멀티플레이 권한/복제 대비에서 싱글플레이 로컬 전투 흐름으로 전환했다.
  - `ServerAimState`, `RepAimVisualState`, `FireRequest`는 신규 설계 기준에서 각각 `FireValidationState`, `FireVisualState`, `FireCommand` 의미로 재정의했다.
  - Network 설계 섹션을 싱글플레이 Fire 설계 위치로 바꿨다.
- v0.4.0 / 2026-06-19
  - 현재 코드에 적용된 `FCFVehicleFireValidationState`, `FCFVehicleAimVisualState`, `GetAimVisualState()`, `UpdateAimVisualFromFireResult()` 명칭을 반영했다.
  - `FCFVehicleFireRequest`는 남은 이름이지만 현재 의미가 로컬 Fire Command 데이터임을 명시했다.

## 마이그레이션 지침

- Pawn 단위 서버/RPC Fire 경로와 Aim 상태 이름은 현재 코드에서 로컬 기준으로 정리됐다.
- 새 작업지시서에서는 `FireValidationState`, `AimVisualState`, `BuildFireCommand`, `ValidateFireCommand`, `RunLocalDummyHitScan`, `ApplyFireResult` 기준 이름을 사용한다.
- 남은 `FCFVehicleFireRequest` 이름을 바꿀 때는 별도 리네이밍 작업으로 분리하고, BP 저장값과 빌드를 함께 확인한다.
- BP와 UI는 여전히 표시 전용으로 유지하고, 발사 가능 여부 계산은 C++ Aim/Weapon 계층에 둔다.

## Change Note

- 2026-06-19: 과거 설계상 “문서 전환만으로 리네이밍하지 않는다”는 내용을 현재 적용 완료 상태에 맞게 축소했다.
