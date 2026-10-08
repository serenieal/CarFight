# CarFight — CF_AimContext

> 역할: 조준 시스템을 CarFight 전체 구조 안에서 어디에 둘지 고정한다.
> 문서 버전: v0.4.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Draft / Single Player Planning + Local Fire Rename

---

## 1. 문서 목적

이 문서는 조준 시스템의 세부 구현 명세가 아니다.

이 문서의 목적은 아래 3가지를 먼저 고정하는 것이다.

1. 조준 시스템이 이미 만들어진 기능과 어떤 관계를 가지는지 정리한다.
2. 조준 시스템이 앞으로 만들 기능과 어떤 관계를 가져야 하는지 정리한다.
3. CarFight의 개발 방향 / 빅픽쳐 / 싱글플레이 전투 흐름 안에서 Aim의 위치를 고정한다.

조준 시스템은 독립 기능이 아니라 `Camera`, `Weapon`, `VehicleDebug`, `Data`, `Damage` 사이의 연결 계층이다.

---

## 2. 프로젝트 빅픽쳐에서 Aim의 위치

CarFight의 최종 방향은 차량 전투와 물리 상호작용 중심 게임이다.

상위 방향은 다음과 같다.

- 차량 전투 중심
- 차량 간 충돌, 밀기, 제어 상실, 복구 같은 물리 상호작용 중시
- 다차종 대응
- 판단 / 상태 / 규칙은 C++ 코어에 배치
- `BP_CFVehiclePawn`는 Thin BP로 유지
- 현재는 `AWheeledVehiclePawn + ChaosWheeledVehicleMovementComponent` 기반 하이브리드 구조
- 장기적으로 `CMVS / Cluster Union / Geometry Collection` 기반 구조 전환 가능성 유지

이 관점에서 Aim은 다음 흐름의 중간 계층이다.

```text
Camera
  -> Aim
  -> Weapon
  -> Projectile / HitScan
  -> Damage
  -> Vehicle Combat
  -> Destruction / Part Damage
```

조준 시스템은 단순한 화면 조준점이 아니라, 카메라 조준과 무기 발사 사이의 해석 계층이다. 현재 기준에서는 플레이어 입력, 로컬 발사 가능 여부, 로컬 HitScan/Projectile 연결 준비, Reticle 피드백을 한 흐름으로 묶는다.

---

## 3. 현재 구현 기능과의 관계

### 3.1 ACFVehiclePawn

현재 기준 플레이 차량 구조는 다음과 같다.

```text
BP_CFVehiclePawn
  -> ACFVehiclePawn
```

`ACFVehiclePawn`은 현재 아래 코어를 소유한다.

- `UCFVehicleDriveComp`
- `UCFWheelSyncComp`
- `UCFVehicleCameraComp`
- `UCFVehicleData`
- Vehicle Input Action 참조
- VehicleDebug Snapshot 제공 함수

Aim 시스템 추가 후 권장 구조는 다음과 같다.

```text
ACFVehiclePawn
  ├─ UCFVehicleDriveComp
  ├─ UCFWheelSyncComp
  ├─ UCFVehicleCameraComp
  ├─ UCFVehicleAimComp
  └─ UCFVehicleWeaponComp   // 미래
```

`ACFVehiclePawn`은 AimComp의 Owner이며, 실제 조준 계산 규칙은 AimComp에 둔다.

### 3.2 UCFVehicleCameraComp

현재 `UCFVehicleCameraComp`는 조준 시스템의 첫 입력 소스다.

현재 CameraComp가 담당하는 영역은 다음과 같다.

- 차량 중심 피벗 기준 카메라 회전
- Look 입력 처리
- Yaw / Pitch Clamp
- SpringArm 충돌 처리
- FOV 보간
- Aim Trace 계산
- `FCFVehicleCameraRuntimeState` 제공

AimComp는 CameraComp에서 다음 정보를 읽는다.

- `AimHitLocation`
- `AimTraceDistance`
- `bAimBlocked`
- `ClampedAimYaw`
- `ClampedAimPitch`
- `ActiveAimProfileName`

단, CameraComp의 Aim Trace는 카메라가 어디를 보고 있는가를 의미한다. AimComp는 이 정보를 이용해 무기가 실제로 그 방향을 쏠 수 있는가를 판단해야 한다.

### 3.3 UCFVehicleData

현재 프로젝트는 차량별 차이를 `UCFVehicleData`로 분리하는 방향이다.

Aim도 장기적으로는 데이터화되어야 한다.

단, 아직 무기 시스템이 없으므로 1차 구현에서는 과도하게 `UCFVehicleData`를 확장하지 않는다.

권장 단계는 다음과 같다.

```text
Phase 1:
  UCFVehicleAimComp 내부 기본 AimProfile

Phase 2:
  UCFVehicleAimData 또는 UCFWeaponAimData DataAsset 분리

Phase 3:
  UCFVehicleData에서 AimData / WeaponData 참조

Phase 4:
  Weapon / Damage 공통 데이터 검증
```

### 3.4 VehicleDebug

조준 시스템은 반드시 VehicleDebug에 연결해야 한다.

조준 시스템에서 확인해야 하는 값은 다음과 같다.

- Local Aim Target
- Aim Direction
- Weapon Arc 판정
- Reticle State
- Last Fire Reject Reason
- Last Fire Result
- Local HitScan Result

권장 구조는 다음과 같다.

```text
FCFVehicleDebugSnapshot
  ├─ Overview
  ├─ Drive
  ├─ Input
  ├─ Camera
  ├─ Aim
  └─ Runtime
```

CameraDebugPlan과 같은 방식으로 Aim 카테고리를 VehicleDebug Panel에 추가한다.

### 3.5 SteeringPlan

SteeringPlan의 핵심 방향은 차량 이동 입력과 조준 입력을 분리하는 것이다.

현재 구조는 다음과 같다.

```text
왼쪽 스틱:
  차량 이동 / 조향

오른쪽 스틱 또는 마우스:
  카메라 / 조준
```

Aim 시스템은 차량 진행 방향에 종속되면 안 된다.

예를 들어 차량이 왼쪽으로 선회 중이어도, 카메라와 무기는 오른쪽 대상을 조준할 수 있어야 한다.

---

## 4. 앞으로 만들 기능과의 관계

### 4.1 Weapon System

Weapon System은 아직 본격 구현 전이다.

AimComp는 미래의 WeaponComp와 다음 관계를 가진다.

```text
UCFVehicleAimComp
  -> 조준 방향 / 조준 가능 여부 / Reticle State 계산

UCFVehicleWeaponComp
  -> 활성 무기 그룹 / 탄약 / 쿨다운 / 재장전 / 발사 처리
```

싱글플레이에서는 WeaponComp가 로컬 발사 실행, 탄약, 쿨다운, 재장전, 발사 FX 트리거를 담당해야 한다.

### 4.2 Reticle UI

Reticle UI는 계산하지 않고 표시만 한다.

올바른 구조는 다음과 같다.

```text
UCFVehicleAimComp
  -> ECFVehicleReticleState 계산

WBP_AimReticle
  -> ReticleState를 읽어서 시각 표현
```

Reticle은 플레이어가 보는 즉각적인 조준감과 발사 가능 상태를 책임진다.

### 4.3 Damage System

Damage는 싱글플레이 전투 시스템에서 확정한다.

AimComp는 데미지를 직접 적용하지 않는다.

```text
AimComp
  -> 쏠 수 있는 방향 계산

WeaponComp
  -> 발사 실행

Damage System
  -> 로컬 피격 결과 처리
```

### 4.4 Lock-On

Lock-On은 1차 구현 대상이 아니다.

하지만 AimTarget 구조는 Lock-On 확장을 막지 않아야 한다.

미래 AimTargetMode 후보는 다음과 같다.

```text
FreeAim
AssistedAim
LockOn
AssistedCorrected
```

### 4.5 AI Combat

AimComp는 플레이어 입력에 직접 의존하면 안 된다.

플레이어는 CameraComp를 통해 AimTarget을 만들고, AI는 AI Controller 또는 Combat Brain에서 AimTarget을 지정할 수 있어야 한다.

공통 AimComp가 WeaponArc / CanFire 판정을 수행한다.

### 4.6 Destruction / Part Damage

장기적으로 차량 파괴 / 부품 데미지가 들어오면 Aim은 부품 단위 피격 정보까지 열어야 한다.

따라서 AimTarget은 단순 `FVector` 하나로 끝내지 않는다.

미래 확장 필드는 다음과 같다.

- `AimTargetActor`
- `AimTargetComponent`
- `AimHitBoneName`
- `AimHitLocation`
- `AimHitNormal`
- `AimTraceDistance`

---

## 5. 싱글플레이 전투 흐름

Aim은 싱글플레이에서 다음 4개 흐름으로 나눈다.

```text
Local Aim
  -> 플레이어 입력과 카메라 기준 조준 방향 계산

Local Fire Validation
  -> 무기 조준각 / 쿨다운 / 탄약 / 차단 여부 확인

Local Hit Result
  -> HitScan 또는 Projectile 초기 결과 생성

Local Feedback
  -> Reticle / Debug / FX / 사운드 표시
```

전투 결과는 현재 로컬 런타임에서 확정한다.

플레이어가 발사 버튼을 누르면 AimComp와 미래 WeaponComp가 같은 프레임 기준으로 발사 가능 여부를 확인한다.

```text
Player Input:
  Fire 입력 발생

Aim / Weapon:
  현재 조준 방향과 무기 상태로 발사 가능 여부 검증

Combat:
  HitScan / Projectile / Damage / FX로 결과 반영
```

---

## 6. 설계 원칙

- CameraComp와 AimComp를 분리한다.
- 조준 계산과 발사 실행을 분리한다.
- Local Aim, Fire Validation, Hit Result, Feedback을 분리한다.
- Aim Runtime State를 네트워크 복제 전제로 설계하지 않는다.
- Weapon System이 없어도 AimComp는 독립적으로 조준 가능 여부를 계산할 수 있어야 한다.
- 발사 입력은 조준각, 차단 여부, 무기 상태를 통과한 뒤에만 결과를 만든다.
- Debug 없이 조준 시스템을 구현하지 않는다.
- 현재 PoliceCar 기준으로 테스트하되, PoliceCar 전용 구조로 만들지 않는다.
- 장기적인 차량 root 구조 전환을 막는 의존성을 만들지 않는다.

---

## 7. 1차 구현 범위

포함한다.

- `UCFVehicleAimComp`
- Local Aim State
- Local Fire Validation State
- Fire Command / Fire Result 구조
- Reticle State
- VehicleDebug Aim 카테고리
- 로컬 Fire 입력 처리
- 로컬 HitScan Trace 더미

제외한다.

- 완성형 Weapon System
- 완성형 Damage System
- Projectile 예측
- Lock-On
- AI Combat
- 부품 파괴 연동
- 멀티플레이 RPC / 복제 / Dedicated Server 검증

---

## 8. 후속 문서 연결

- 과거 네트워크 세부 원칙은 `CF_AimNet.md`에 Deprecated 기록으로 둔다.
- 기능 명세는 `CF_AimSpec.md`에서 다룬다.
- C++ / BP 구조는 `CF_AimDesign.md`에서 다룬다.
- 구현 순서는 `CF_AimTasks.md`에서 다룬다.
- 검증 기준은 `CF_AimVerify.md`에서 다룬다.

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 멀티플레이 관계 섹션을 싱글플레이 전투 흐름으로 교체했다.
  - 서버 권한, 클라이언트 요청, 복제 시각 상태를 현재 구현 기준에서 제외했다.
  - Weapon / Damage 관계를 로컬 발사와 로컬 피격 결과 기준으로 재정의했다.
- v0.4.0 / 2026-06-19
  - Pawn 단위 서버/RPC Fire 경로 제거 후 현재 코드 기준 이름을 반영했다.
  - 남은 `FCFVehicleFireRequest` 계열 이름은 로컬 Fire Command 데이터 의미로 해석하도록 정리했다.

## 마이그레이션 지침

- 신규 구현은 `Local Aim -> Local Fire Validation -> Local Hit Result -> Local Feedback` 흐름을 따른다.
- 현재 코드 기준에서 `ServerAim`, `RepAimVisual`, `ServerRequestFire` 계열 핵심 경로는 제거 또는 로컬 명칭으로 정리됐다.
- 신규 문서에는 `FireValidationState`, `AimVisualState`, `BuildFireCommand`, `ValidateFireCommand`, `RunLocalDummyHitScan`, `ApplyFireResult` 기준 이름을 사용한다.
- 남은 `FCFVehicleFireRequest` 이름을 바꿀 때는 C++ API, BP 저장값, 에디터 검증 범위를 확인한 뒤 별도 작업으로 진행한다.

## Change Note

- 2026-06-19: “즉시 변경하지 말 것”으로 남아 있던 과거 리네이밍 유예 문장을 현재 적용 완료 상태로 갱신했다.
