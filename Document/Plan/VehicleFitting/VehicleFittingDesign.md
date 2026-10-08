# Vehicle Fitting Design

- Version: 0.9.0
- Date: 2026-08-04
- Status: Decision Locked / `FIT-P0-04` Done / `FIT-P0-05` Initial Sortie Adapter Code Complete — Build·실제 BP/DataAsset Physics Automation PASS / User PIE·Mobility Pending / `FFIT-P0-00` Field Action Contract Locked / `CF-FQ-035 INV-P0-00~03` Done
- Feature: `CF-FQ-034 차량 피팅·질량 런타임`
- Representative Plan: `Document/Plan/VehicleFitting/VehicleFittingPlan.md`
- Roadmap: `Document/Plan/VehicleFittingRoadmap.md`
- Strategic Direction: `Document/ProjectSSOT/CombatPlan/13_Fitting.md`

---

## 1. 설계 목적

CarFight의 피팅을 기존 차량 데이터의 직접 수정이 아니라 다음 세 계층으로 분리한다.

```text
Platform Definition
= 차량이 원래 제공하는 구조와 기본값

Saved Fitting Template
= 장비 종류와 기본 구성을 저장한 DataAsset 기반 선택

Field Fitting Draft
= 접근 가능한 Inventory의 실제 Item Instance를 사용하는 편집 중 선택

Applied Runtime Snapshot
= 모든 참조·호환·소유권·질량 검증과 Commit을 통과한 실제 적용 결과
```

핵심 목표는 피팅 UI, 차량 런타임, 무기, 탄약, 방어가 서로 다른 계산을 하지 않게 하는 것이다.

---

## 2. 현재 구현 아키텍처

### 2.1 VehicleData

`UCFVehicleData`는 현재 차량 플랫폼 루트다.

```text
UCFVehicleData
├─ VehicleVisualConfig
├─ VehicleLayoutConfig
├─ HardpointSlots
├─ MountProfiles
├─ VehicleMovementConfig
├─ WheelVisualConfig
├─ VehicleReferenceConfig
├─ VehicleDurabilityConfig
├─ DefaultDefenseData
└─ DriveStateConfig
```

현재 VehicleRuntime은 `VehicleData`를 풀어 Movement·Wheel·Drive 설정에 반영한다.

`FIT-P0-02` 완료 후 다음 데이터 계약이 추가됐다.

```text
BaseVehicleMassKg
MaximumGrossMassKg
UCFVehicleFittingData
FCFVehicleFittingSnapshot 공용 구조
```

`FIT-P0-03` 완료 범위로 `UCFVehicleFittingData::BuildFittingSnapshot()`이 실제 Validation과 Resolved Snapshot을 생성한다.

현재 `FIT-P0-04` 완료 범위로 다음 Runtime 계층이 추가됐다.

```text
UCFVehicleFittingComp
- Legacy·Snapshot Runtime 입력 Prepare
- Weapon·Defense Commit과 실패 Rollback
- AppliedFittingSnapshot 읽기 전용 소유
- EndPlay·Reset 수명 정리

VehicleWeaponComp
- VehicleData 기본 EquipmentPresetData Legacy 경로
- Snapshot EquipmentPresetData Override 경로

VehiclePawn
- Fitting Prepare·Commit 뒤 Launcher 연결
- Fitting Commit 성공을 VehicleRuntime Ready 조건에 포함
```

아직 없는 것:

```text
MaximumPayloadMassKg 저장 필드 — GrossMass에서 파생
CF-FQ-031 기반 Ammo 선택·질량
CF-FQ-035 INV-P0-04 기반 Item Instance → Fitting Adapter
Field Fitting Permission·Timed Action·Cancel Runtime
물리 생성 뒤 Field Fitting 질량 재적용·Physics State 재생성 경로
```

### 2.2 Hardpoint와 MountProfile

현재 위치와 장착 규칙은 분리되어 있다.

```text
FCFVehicleHardpointSlot
- LocationSlotId
- LocationCategory
- SocketName
- LocalLocation
- LocalRotation

FCFVehicleMountProfile
- MountProfileId
- LocationSlotRef
- MountType
- SizeLimit
- DefaultEquipmentPresetData
- bExposedModule
```

`LocationSlotRef`가 실제 `HardpointSlots.LocationSlotId`를 참조한다.
피팅은 이 관계를 재정의하지 않고 선택 검증에 사용한다.

### 2.3 EquipmentPreset

현재 장비 조합 SSOT:

```text
UCFEquipmentPresetData
├─ EquipmentId
├─ RequiredMountType
├─ RequiredWeaponSize
├─ DefaultTurretMountData
└─ DefaultWeaponData
```

현재 장비 해석 경로:

```text
MountProfile.DefaultEquipmentPresetData
→ EquipmentPresetData.DefaultTurretMountData
→ EquipmentPresetData.DefaultWeaponData
```

MountProfile에서 TurretMountData나 WeaponData를 직접 fallback하는 레거시 경로는 제거되었다.
피팅도 반드시 EquipmentPresetData를 통해 선택한다.

### 2.4 Weapon과 TurretMount

`UCFTurretMountData`는 실제 장착 메쉬·소켓·회전·발사 정책을 소유하고 `TurretMountWeightKg`를 가진다.
이 질량은 `FCFVehicleFittingSnapshot.TotalVehicleMassKg`에 포함되며 초기 출격 PreRegister 단계에서 차량 Movement Mass에 반영된다.

`UCFWeaponData`는 장착 호환, 발사 모드·속도·사거리, Launcher Pattern·Release, 탄약 후보 필드와 `WeaponMassKg`를 소유한다. `GetEffectiveWeaponMassKg()`는 음수·비유한 값을 0으로 보정하고 Snapshot 총중량에 기여하며, 무기 발사 공식 자체에는 영향을 주지 않는다.

### 2.5 Ammo

현재 WeaponData의 탄약 후보:

```text
MagazineSize
ReloadTimeSeconds
AmmoTypeId
```

`CF-FQ-031` 설계의 향후 소유권:

```text
UCFAmmoData
- AmmoId
- UnitMassKg
- MaximumLoadableAmmoCount

UCFVehicleAmmoComp
- WeaponInstanceId별 LoadedAmmo
- AmmoData별 ReserveAmmo
- Reservation·Commit·Rollback
- Reload
- Runtime Snapshot
```

피팅은 `InitialSortieAmmoCount`를 선택하고 질량을 계산하지만 실제 소비 상태는 소유하지 않는다.

### 2.6 Defense

현재 방어 소유권:

```text
UCFVehicleDefenseData
= Shield·Regeneration·ArmorType·Resistance·6방향 Armor 설정

UCFVehicleDefenseComp
= 방어 런타임 상태와 피해 분배

UCFVehicleHealthComp
= Vehicle Integrity와 파괴 상태
```

피팅은 `VehicleDefenseData`를 선택하고 `DefenseMassKg` 질량 기여를 읽을 뿐, Shield·Armor 공식을 계산하지 않는다. `GetEffectiveDefenseMassKg()`와 DataValidation은 질량 계약만 보호하며 현재 방어 피해 계산을 변경하지 않는다.

### 2.7 VehicleRuntime

현재 초기화:

```text
BeginPlay
→ InitializeVehicleRuntime
→ ApplyVehicleDataConfig
→ ApplyVehicleMovementConfig
→ ApplyVehicleReferenceConfig
→ ApplyVehicleWheelPhysicsConfig
→ ApplyVehicleWheelVisualConfig
→ Drive / WheelSync Ready 판정
```

`FIT-P0-04` 이후 VehicleRuntime은 VehicleData와 선택적 VehicleFittingData를 `UCFVehicleFittingComp`에서 먼저 Prepare하고, 같은 Runtime 입력을 Weapon·Defense에 Commit한 뒤 Launcher를 최종 Weapon Runtime에 연결한다. `FIT-P0-05`는 Game World의 `PreRegisterAllComponents`에서 Snapshot 총중량을 Movement Mass에 1회 기록하고, Physics State 생성 뒤 설정 질량의 Target 일치·VehicleMesh 실제 집계 질량의 Target Coverage·Physics 계약을 검증한 뒤 같은 Snapshot을 Commit한다.

### 2.8 FIT-P0-01 물리 질량 조사

현재 프로젝트와 기존 성공 AssetDump가 확인한 실제 계층:

```text
BP_CFVehiclePawn
└─ VehicleMesh: USkeletalMeshComponent
   - RootComponent
   - 실제 Chaos 차량 물리 바디
   - SkeletalMesh = /Game/Vehicles/SportsCar/SKM_SportsCar

VehicleMovementComp
- VehicleDriveComp가 UChaosWheeledVehicleMovementComponent로 캐시
- 차량 엔진·휠·공력·Chassis 시뮬레이션 소유

SM_Body: UStaticMeshComponent
- VehicleData.ChassisMesh를 표시
- QueryOnly WeaponHit / Projectile / TargetSelect 표면
- 차량 질량 소유자가 아님
```

`ApplyVehicleMovementConfig()`가 현재 적용하는 값:

```text
ChassisHeight
DragCoefficient
DownforceCoefficient
CenterOfMassOverride
EngineSetup
DifferentialSetup
SteeringSetup
```

프로젝트 소스에는 질량을 읽거나 쓰는 `SetMassOverrideInKg`, `SetMassScale`, `GetMass`, `ChassisMass` 경로가 없다. 따라서 현재 차량 질량은 피팅 데이터가 아니라 inherited VehicleMesh·PhysicsAsset와 Chaos VehicleMovement 기본 설정에 맡겨진 상태다.

기준 SkeletalMesh는 확인됐지만 기존 덤프에 PhysicsAsset 직접 참조, Body별 Density·MassScale와 계산 질량은 포함되지 않았다. 승인된 신규 자산 조회도 자산 로드 전에 실패했다.

이 제한 때문에 PhysicsAsset 계산값은 피팅 SSOT로 사용하지 않는다. P0 구조는 명시적 VehicleData 질량을 사용하고, PhysicsAsset·VehicleMesh 실값은 적용 전후 진단과 불일치 검증에만 사용한다.

---

## 3. 문제 정의

현재 기본 장비는 `VehicleData.MountProfiles[].DefaultEquipmentPresetData`에 직접 들어 있다.
이 구조만으로는 다음을 구분할 수 없다.

```text
- 차량 플랫폼이 제공하는 기본 구성
- 플레이어가 선택한 피팅
- 검증을 통과한 출격 결과
- 현재 런타임에서 적용된 구성
```

또한 질량이 각 데이터에 일부 있거나 전혀 없고, 실제 Chaos 질량 적용 경로가 없다.

피팅을 VehicleData에 직접 쓰면 다음 문제가 생긴다.

```text
- 원본 플랫폼 DataAsset이 플레이어 선택마다 변경됨
- 에디터 저장과 런타임 선택이 혼합됨
- 여러 피팅 프리셋을 같은 VehicleData에 저장할 수 없음
- 기본 장비와 플레이어 선택의 fallback 의미가 불명확함
- SaveGame·인벤토리 확장 시 자산과 사용자 상태가 결합됨
- 실패한 부분 적용이 VehicleRuntime에 남을 수 있음
```

따라서 별도 피팅 데이터와 검증된 Snapshot이 필요하다.

---

## 4. 권장 소유 구조

```text
UCFVehicleData
= 차량 플랫폼 구조와 기본 구성

UCFVehicleFittingData
= 출격 전 선택 데이터

UCFVehicleFittingComp
= 검증, 해석, Snapshot, 적용 상태

UCFVehicleWeaponComp
= 실제 활성 무기·터렛·발사 Runtime

UCFVehicleAmmoComp
= 실제 탄약·재장전 Runtime

UCFVehicleDefenseComp
= 실제 Shield·Armor Runtime

ACFVehiclePawn / VehicleRuntime
= 적용 순서와 Chaos Vehicle 연결
```

### 4.1 비소유 원칙

`UCFVehicleFittingComp`는 다음을 소유하지 않는다.

```text
- 발사 쿨다운
- Muzzle 순환
- Launcher Sequence
- 탄약 소비·예약·재장전
- Shield 현재값
- 방향별 Armor 현재값
- Vehicle Integrity
- Projectile·Damage 계산
```

이 값은 기존 Runtime 컴포넌트에 남긴다.

---

## 5. 권장 데이터 모델

### 5.1 UCFVehicleFittingData

`FIT-P0-02` 구현 필드:

```text
FittingId
DisplayName
VehicleData
MountSelections
MissingMountSelectionPolicy
DefenseSelection
FittingTags
```

`AmmoLoadSelections`는 CF-FQ-031의 AmmoData 계약을 재사용하기 위해 아직 추가하지 않았다. 공용 Snapshot에는 후속 연동을 위한 `AmmoMassKg` 0 기본 슬롯만 둔다.

P0에서 아이콘·가격·희귀도·소유 수량은 넣지 않는다.

### 5.2 Mount Selection

권장 구조:

```text
FCFVehicleMountSelection
```

필드:

```text
MountProfileId
EquipmentPresetData
bEnabled
```

해석 규칙:

```text
MountProfileId
→ VehicleData.MountProfiles에서 검색
→ MountProfile.LocationSlotRef 해결
→ EquipmentPresetData 호환 검사
→ TurretMountData와 WeaponData 해석
```

`LocationSlotId`, `MountType`, `SizeLimit`을 선택 데이터에 복사 저장하지 않는다.
원본 VehicleData와 불일치하는 중복 데이터를 피한다.

### 5.3 Ammo Load Selection

`CF-FQ-031` 타입을 최종 재사용한다.
개념 필드:

```text
AmmoData
InitialSortieAmmoCount
```

피팅은 출격 적재 수량을 선택한다.
장전·예비 분배는 Weapon·Ammo Runtime 초기화 정책이 담당한다.

### 5.4 Defense Selection

`FIT-P0-02` 구현 구조:

```text
FCFVehicleDefenseSelection
- SelectionMode
- DefenseData

SelectionMode
- UseVehicleDefault
- ExplicitNone
- Override
```

Override에서만 DefenseData가 필수다. P0에서는 방향별 ArmorData를 별도 장착 아이템 배열로 분해하지 않는다.

---

## 6. 질량 데이터 소유 후보

### 6.1 Base Vehicle Mass

#### 후보 A — VehicleData 명시 질량

```text
VehicleData.BaseVehicleMassKg
```

장점:

```text
- 결정론적인 피팅 계산
- UI와 Runtime 동일 값
- 에셋별 명시적 밸런스 값
- PhysicsAsset 변경 감지 가능
```

위험:

```text
- 실제 Chaos Body 질량과 이중 SSOT 가능
- 메시·PhysicsAsset 변경 후 값 불일치
```

#### 후보 B — 실제 Body 질량 조회

장점:

```text
- 물리 실값 사용
- 별도 입력 불필요
```

위험:

```text
- 에디터·런타임·Cook 결과 차이 가능
- UI Preview에서 Pawn 없이 계산하기 어려움
- 질량 오버라이드와 물리 자산 설정의 의존성이 숨음
```

#### 권장안

```text
VehicleData.BaseVehicleMassKg = 피팅 계산 SSOT
실제 Body 질량 = 런타임 Validation 대상
```

둘이 허용 오차를 벗어나면 경고 또는 적용 실패 정책을 둔다.
최종 승인 전 실제 Chaos 차량 질량 경로를 조사한다.

### 6.2 Turret Mount Mass

현재 `TurretMountData.TurretMountWeightKg`를 사용한다.
필드 이름 변경은 별도 Migration 필요성이 생기기 전까지 하지 않는다.

### 6.3 Weapon Mass

권장:

```text
WeaponData.WeaponMassKg
```

이유:

```text
- 같은 무기를 다른 Mount에 재사용 가능
- EquipmentPresetData가 단순 조합 자산으로 유지
- Mount와 Weapon 질량을 별도 Breakdown 가능
```

### 6.4 Defense Mass

P0 권장:

```text
VehicleDefenseData.DefenseMassKg
```

의미:

```text
Shield Generator
+ Armor Package
+ 관련 방어 장비의 총 패키지 질량
```

P1에서 필요하면 ShieldMass와 방향별 ArmorPlateMass로 분리한다.
P0에서 방향별 장갑 내구도와 질량을 억지로 1:1 환산하지 않는다.

### 6.5 Ammo Mass

```text
AmmoMassKg
= Σ(InitialSortieAmmoCount × AmmoData.UnitMassKg)
```

장전과 예비는 동일 탄약의 위치 구분이므로 피팅 총질량에서 한 번만 계산한다.

### 6.6 EquipmentPreset Mass

P0 권장:

```text
EquipmentPresetMassKg
= TurretMountMassKg + WeaponMassKg
```

EquipmentPresetData 자체에 추가 질량 필드를 두지 않는다.
어댑터·케이블·프레임 등 조합 오버헤드가 실제 디자인에 필요해질 때 P1에서 별도 필드를 검토한다.

---

## 7. 총중량과 탑재 한도

### 7.1 기본 공식

```text
BaseVehicleMassKg
= 차량 플랫폼 기준 질량

PayloadMassKg
= EquipmentMassKg
+ AmmoMassKg
+ DefenseMassKg

TotalVehicleMassKg
= BaseVehicleMassKg + PayloadMassKg
```

### 7.2 한도 후보

#### MaximumPayloadMassKg

```text
PayloadMassKg <= MaximumPayloadMassKg
```

장점: 플레이어가 이해하기 쉽다.

#### MaximumGrossMassKg

```text
TotalVehicleMassKg <= MaximumGrossMassKg
```

장점: 실제 차량 총중량 개념과 가깝다.

둘 다 동시에 SSOT로 두지 않는다.
하나는 계산값 또는 표시값으로 파생한다.

P0 권장안:

```text
VehicleData.MaximumGrossMassKg
```

피팅 UI는 다음을 파생 표시한다.

```text
MaximumPayloadMassKg
= MaximumGrossMassKg - BaseVehicleMassKg
```

최종 결정은 `FIT-D-010`에서 잠근다.

---

## 8. 검증 모델

### 8.1 Validation State

권장 enum:

```text
ECFFittingValidationState
- NotEvaluated
- Valid
- ValidWithWarnings
- Invalid
```

### 8.2 Issue Severity

```text
ECFFittingIssueSeverity
- Info
- Warning
- Error
```

### 8.3 구현 Issue Code

```text
MissingFittingData
MissingFittingId
MissingVehicleData
VehicleDataMismatch
DuplicateMountSelection
MissingMountSelection
UnknownMountProfile
MissingHardpointSlot
MissingEquipmentPreset
IncompleteEquipmentPreset
MountTypeMismatch
WeaponSizeExceeded
WeaponMountIncompatible
MissingDefenseData
InvalidAmmoSelection
AmmoCountExceeded
InvalidMassValue
GrossMassExceeded
MissingMassSource
RuntimeMassMismatch
```

### 8.4 적용 정책

```text
Error 1개 이상
→ 적용 금지
→ VehicleData 기본 구성 보존

Warning만 존재
→ 정책에 따라 적용 가능
→ Snapshot에 Warning 보존

Valid
→ Snapshot 생성 및 적용 가능
```

실패 시 가능한 항목만 부분 적용하지 않는다.
Weapon만 바뀌고 Defense·Mass는 실패하는 상태를 방지한다.

---

## 9. Snapshot 설계

권장 구조:

```text
FCFVehicleFittingSnapshot
```

Identity:

```text
FittingId
VehicleData
ValidationState
IsValid() 파생 판정
```

Resolved Mounts:

```text
ResolvedMounts[]
- MountProfileId
- LocationSlotId
- MountType
- SizeLimit
- EquipmentPresetData
- TurretMountData
- WeaponData
- MountMassKg
- WeaponMassKg
```

Defense:

```text
ResolvedDefenseSelectionMode
ResolvedDefenseData
DefenseMassKg
```

`ResolvedDefenseSelectionMode`는 `UseVehicleDefault`에서 기본 방어가 None인 경우와 `ExplicitNone`을 구분한다.

Ammo:

```text
ResolvedAmmoLoads[]
- AmmoData
- InitialSortieAmmoCount
- UnitMassKg
- TotalAmmoMassKg
```

Mass:

```text
BaseVehicleMassKg
EquipmentMassKg
AmmoMassKg
DefenseMassKg
PayloadMassKg
TotalVehicleMassKg
MaximumGrossMassKg
PayloadUsageRatio
GrossMassUsageRatio
```

Mobility Preview:

```text
AccelerationScale
BrakingScale
SteeringResponseScale
TopSpeedScale — P0 선택
```

Diagnostics:

```text
ValidationIssues
Summary
```

UI와 실제 Runtime 적용은 같은 Snapshot을 사용한다.
UI가 질량을 다시 계산하지 않는다.

---

## 10. 피팅 해석 흐름

```text
BuildFittingSnapshot(FittingData)
```

실제 `FIT-P0-03` 해석 순서:

```text
1. Snapshot Identity와 Defense 선택 방식 초기화
2. FittingId·VehicleData·BaseMass·GrossMass 검증
3. HardpointSlot ID와 MountProfile ID 집합 생성
4. MountSelections를 유일 선택·중복·Unknown으로 분류
5. 중복·Unknown 문제를 이름 오름차순으로 기록
6. VehicleData.MountProfiles 원본 순서로 ResolvedMounts 생성
7. LocationSlotRef → HardpointSlot 검증
8. FittingOverride 또는 MissingMountSelectionPolicy 해석
9. ExplicitEmpty와 MissingPolicyEmpty 구분
10. EquipmentPreset 완성·MountType·Size 검증
11. Weapon MountType·Size 호환 검증
12. TurretMount·Weapon 질량 검증·합산
13. Defense 3상태와 DefenseMass 해석
14. AmmoMassKg=0 유지
15. Payload·Total·사용률 계산
16. MaximumGrossMass 초과 검증
17. 문제 심각도로 ValidationState 확정
```

결정론성 기준:

```text
- ResolvedMounts 순서 = VehicleData.MountProfiles 순서
- Duplicate·Unknown Selection 문제 순서 = MountProfileId 이름 오름차순
- MountSelections 배열 저장 순서는 결과와 문제 순서를 변경하지 않음
```

Mobility Preview와 표시용 Summary/ViewData는 FIT-P0-05~06 범위다.

---

## 11. 누락 MountSelection 정책

후보:

```text
UseVehicleDefault
TreatAsEmpty
TreatAsError
```

권장 P0 기본:

```text
UseVehicleDefault
```

이유:

```text
- 기존 VehicleData 기본 장비와 호환
- 부분 피팅 프리셋 작성 가능
- 기존 에셋이 FittingData 미지정일 때 동작 유지
```

단, `UseVehicleDefault`가 암묵적으로 작동하면 UI와 Runtime이 다르게 보일 수 있으므로 Snapshot에 실제 해석된 기본 장비를 반드시 기록한다.

빈 장착을 명시하려면 단순 누락과 구분되는 `bEnabled=false` 또는 명시 Empty 선택을 사용한다.

---

## 11A. Field Fitting Action 설계

### 11A.1 역할 분리

```text
Inventory Foundation
= 실제 Item Instance 소유권, Container, Reservation, Atomic Transfer

Vehicle Fitting
= 빈 슬롯·호환 판정, 시간 액션, Snapshot, Runtime Apply Coordinator

Combat·Drive·Weapon·Ammo·Defense
= 현재 액션을 차단하는 상태와 이유 제공

UI Framework
= Screen·Panel·Modal·System 표시, Focus와 입력 모드
```

### 11A.2 액션 시작 조건

다음 조건을 모두 만족해야 한다.

```text
CombatState = NonCombat
RelevantCooldowns = Complete
ConflictingActions = None
VehicleSpeed <= StationarySpeedThresholdKmh
StationaryDuration >= RequiredStationaryDurationSeconds
VehicleRuntime = Ready
InventoryAccess = Accessible
Reservation = Success
```

`StationarySpeedThresholdKmh`, `RequiredStationaryDurationSeconds`, Equip·Unequip 시간은 데이터 기반 설정으로 둔다. Chaos 미세 진동 때문에 속도 `0.0` 정확 비교를 사용하지 않는다.

### 11A.3 공통 Blocker Query

피팅 컴포넌트가 모든 하위 컴포넌트의 내부 변수를 하드코딩하지 않는다.

```text
Combat Provider
→ CombatActive

Drive Provider
→ VehicleMoving

Weapon / Launcher Provider
→ CooldownActive, FireSequenceActive

Ammo Provider
→ ReloadActive, AmmoTransferActive

Defense / Utility Provider
→ CooldownActive, RepairActive
```

각 Provider가 구조화된 BlockReason을 반환하고 Field Fitting Coordinator가 최종 Permission을 만든다.

### 11A.4 Equip 규칙

```text
- 대상 MountProfile은 Empty
- Item Instance는 접근 가능한 Container에 존재
- Reservation되지 않은 실제 소유 Item
- MountType·WeaponSize·Weapon 호환 PASS
- MaximumGrossMass 검증 PASS
```

장착된 슬롯에 새 아이템을 직접 덮어쓰지 않는다.

```text
Replace
= Unequip Action 완료
→ 빈 슬롯 확인
→ Equip Action 새로 시작
```

### 11A.5 Unequip 규칙

```text
- 대상 MountProfile에 실제 Item Instance가 장착됨
- 반환 Destination Container 접근 가능
- Destination Slot·Mass Capacity 예약 성공
- Action 완료 전 기존 장비 Runtime 유지
```

### 11A.6 시간 액션 상태

```text
Idle
→ Preparing
→ InProgress
→ Completing
→ Completed

Preparing / InProgress / Completing
→ Cancelled 또는 Failed 가능
```

액션 시작 시 실제 아이템을 이동하거나 장비를 제거하지 않고 Reservation만 만든다.

### 11A.7 취소 조건

```text
VehicleMoved
CombatStarted
CooldownOrConflictingActionStarted
DamageReceived
VehicleDestroyed
RuntimeNotReady
SlotStateChanged
InventoryChanged
ReservationLost
ScreenClosed
UserCancelled
```

취소 결과:

```text
Inventory 위치 무변경
Mounted 장비 무변경
Applied Snapshot 무변경
질량 무변경
Reservation 전부 해제
CancelReason 기록
```

### 11A.8 완료와 원자 적용

완료 순간 다음을 하나의 조정 흐름으로 처리한다.

```text
1. 모든 Permission과 Reservation 최종 재검증
2. Candidate Draft로 Snapshot 생성
3. Inventory Transaction Prepare
4. Vehicle Runtime Apply Prepare
5. Runtime Apply Commit
6. Inventory Transaction Commit
7. 새 Applied Snapshot 확정
8. Reservation 해제와 완료 이벤트
```

어느 단계든 실패하면 기존 Applied Snapshot과 기존 Inventory 소유 상태로 Rollback한다. Weapon만 교체되고 Inventory 이동이나 질량 적용이 실패하는 부분 상태를 허용하지 않는다.

### 11A.9 World와 UI

필드 피팅 화면은 게임을 Pause하지 않는다.

```text
- World Tick과 전투 상태는 계속 진행
- 차량 이동 또는 공격으로 조건이 깨지면 액션 취소
- 취소 후 화면은 유지 가능
- 실제 화면 수명·Focus·입력 모드는 CF-FQ-032가 소유
```

---

## 12. 런타임 적용 순서

`FIT-P0-04` 구현 순서:

```text
ACFVehiclePawn::InitializeVehicleRuntime

1. VehicleData 기반 기존 Config·Movement·Wheel 초기화
2. Health Runtime 초기화
3. VehicleFittingComp.PrepareSortieFitting
   - VehicleFittingData 없음: Legacy 입력
   - VehicleFittingData 있음: BuildFittingSnapshot과 VehicleData 일치 검증
4. VehicleFittingComp.CommitPreparedSortieFittingToVehicle
   - Weapon Runtime 입력 적용
   - Defense Runtime 입력 적용
   - 모두 성공할 때만 AppliedFittingSnapshot 교체
5. Commit된 Weapon 캐시로 Turret Visual 재적용
6. LauncherComp를 최종 Weapon Runtime에 연결
7. Drive·WheelSync·Health·Defense Component·Fitting Commit Ready 판정
```

적용 정책:

```text
FittingData 미지정
→ VehicleData 기본 Weapon·Defense Legacy 입력 Commit
→ AppliedFittingSnapshot 없음

FittingData 지정·Snapshot Valid
→ Snapshot 장비·방어 입력 Commit
→ AppliedFittingSnapshot 고정

Snapshot Invalid 또는 VehicleData 불일치
→ Weapon·Defense Adapter 무호출
→ 기존 Applied 상태 보존
→ VehicleRuntime Ready 실패

Weapon 또는 Defense Commit 실패
→ 직전 Applied 입력이 있으면 해당 입력으로 전체 복원
→ 첫 초기화면 VehicleData Legacy 입력으로 전체 복원
→ 후보 AppliedFittingSnapshot 미확정
```

Mass·Mobility와 Ammo는 이 순서에 아직 포함하지 않으며 각각 FIT-P0-05와 CF-FQ-031 연동 이후 추가한다.

---

## 13. 장비 적용 방식

현재 `MountProfile.DefaultEquipmentPresetData`는 VehicleData 구조체 내부 값이다.
피팅을 적용하기 위해 VehicleData 원본 배열을 변경해서는 안 된다.

구현 방식:

```text
InitializeWeaponRuntimeForActiveProfile
→ VehicleData MountProfile.DefaultEquipmentPresetData Legacy 경로

InitializeWeaponRuntimeFromFitting
→ Snapshot이 해석한 EquipmentPresetData Override
→ ExplicitEmpty면 Override=None

ResolveActiveEquipmentPresetData
→ Runtime Override 사용 중이면 Snapshot 입력
→ 아니면 VehicleData DefaultEquipmentPresetData
```

`FindActiveMountProfile()`은 VehicleData의 MountType·SizeLimit·LocationSlotRef 구조를 계속 읽고, 실제 EquipmentPresetData만 Runtime Override 계층에서 분리한다. Commit된 장비 선택은 WeaponData·TurretMountData 캐시와 Pawn의 터렛 시각 구성에 동일하게 사용된다.

장착 타입·크기·하드포인트 위치는 VehicleData MountProfile에서 읽는다.

---

## 14. Defense 적용 방식

현재:

```text
VehicleData.DefaultDefenseData
```

Decision Lock 이후 Defense 선택은 Null 하나로 표현하지 않고 다음 3상태를 사용한다.

```text
UseVehicleDefault
→ VehicleData.DefaultDefenseData 사용

ExplicitNone
→ 방어 패키지 없음 / Legacy Integrity 경로

Override
→ FittingData.SelectedDefenseData 사용
```

VehicleFittingComp의 Defense Runtime 입력은 Snapshot이 최종 해석한 DefenseData 하나 또는 명시 None을 `InitializeFromDefenseData`에 전달한다. FittingData가 없으면 기존 `InitializeFromVehicleData`를 사용한다.

```text
UseVehicleDefault
→ Snapshot.ResolvedDefenseData

ExplicitNone
→ InitializeFromDefenseData(nullptr)
→ Defense Runtime 비활성 / Legacy Integrity 경로

Override
→ Snapshot.ResolvedDefenseData
```

피팅 컴포넌트는 쉴드·장갑 현재값과 피해 공식을 소유하지 않는다.

---

## 15. Ammo 적용 방식

`CF-FQ-031` 구현 후 권장:

```text
FittingSnapshot.ResolvedAmmoLoads
→ VehicleAmmoComp.InitializeSortieAmmo
→ WeaponInstanceId별 초기 장전
→ AmmoData별 예비량
```

질량 계산은 `InitialSortieAmmoCount` 전체를 한 번 합산한다.
장전과 예비의 분배는 질량을 변경하지 않는다.

Ammo Runtime 구현 전 FIT-P0 Foundation을 시작할 경우:

```text
- AmmoLoad 구조를 CF-FQ-031 타입에 종속시키지 않고 임시 중복 생성하지 않는다.
- Ammo integration Task를 dependency gate로 유지한다.
- AmmoMassKg는 0 또는 MissingDependency Warning으로 명시한다.
```

---

## 16. Chaos 질량 적용 결정

현재 코드에는 피팅 질량 적용 경로가 없다. FIT-P0-05 Runtime Mass Gate에서 적용 소유권과 수명을 다음처럼 잠갔다.

```text
Design SSOT
= FittingSnapshot.TotalVehicleMassKg

Chaos Configuration Mirror
= UChaosVehicleMovementComponent::Mass

Runtime Evidence
= VehicleMesh.GetMass()
+ VehicleMesh.GetPhysicsAsset()

초기 적용
= ACFVehiclePawn::PreRegisterAllComponents
→ Super::PreRegisterAllComponents 호출 전 1회 Mass 기록
→ Chaos Vehicle 초기 물리 생성에 위임

검증·Commit
= BeginPlay에서 VehicleMesh 실제 질량 검증
→ 성공한 같은 Cached Snapshot만 Weapon·Defense Commit
```

금지 경로:

```text
- VehicleMesh.BodyInstance의 SetMassOverrideInKg만 단독 호출
- PhysicsAsset Body 질량·Density·MassScale 수정 또는 저장
- CenterOfMassOverride를 총중량으로 사용
- Tick마다 질량 또는 VehicleMovement 전체 설정 재작성
- 실제 질량과 Mobility Scalar에 같은 효과를 무조건 중복 적용
```

UE 5.8 Runtime Mass Gate 결과, 초기 출격은 물리 등록 전 `Mass` 설정값을 기록하므로 Physics State·Vehicle Simulation 재생성을 수행하지 않는다. 현재 `Super::BeginPlay()` 이후 실행되는 `InitializeVehicleRuntime`는 무재생성 초기 질량 적용 시점으로는 늦으므로 질량 Prepare만 PreRegister 단계로 앞당긴다. Physics State 생성 뒤 다른 질량으로 바꾸는 요청은 FIT-P0-05 P0에서 거부하고 실제 재생성은 FFIT-P0-04로 분리한다. 첫 구현 Editor Build는 public `Mass` 접근 심볼을 컴파일 증명해야 하지만 설계 선택은 다시 열지 않는다.

실제 질량은 물리 관성·충돌·휠 하중의 기반이다. Mobility Adapter는 실제 질량만으로 목표 체감이 부족한 축에만 후속 적용하며 기본값은 1.0·비활성이다.

---

## 17. Mobility 공식 결정

P0는 실제 Chaos 질량을 1차 기동 효과로 사용한다. Snapshot은 다음 Preview 입력을 항상 계산하지만 추가 보정은 기본 비활성이다.

```text
MassRatio
= TotalVehicleMassKg / BaseVehicleMassKg
```

M5 측정에서 물리 질량만으로 특정 축의 차이가 부족할 때만 아래 공식을 데이터 옵션으로 활성화한다.

```text
AxisScale
= Clamp(Pow(1 / MassRatio, AxisExponent), AxisScaleMin, AxisScaleMax)
```

축 후보:

```text
AccelerationScale
BrakingScale
SteeringResponseScale
```

결정 원칙:

```text
- 기본 Scale은 모두 1.0
- 실제 질량 적용 전 추가 보정 금지
- 실제 질량만으로 목표 차이가 나오면 Adapter 비활성 유지
- 모든 Exponent·Clamp는 DataAsset에 노출
- 숨은 C++ 매직넘버 금지
- 기준 피팅은 정확히 1.0
- 질량 0·음수·NaN은 Validation Error
- 최고속도 직접 보정은 P0 제외
```

이 결정은 가속·제동·선회를 반드시 인위적으로 낮춘다는 뜻이 아니라, 필요할 경우 사용할 단일 선택 공식만 잠근 것이다.

---

## 18. 탄약 소모와 질량 갱신

후보:

```text
A. 매 발사마다 즉시 질량 갱신
B. 일정 질량 임계값마다 배치 갱신
C. 재장전·보급·탄종 교환 시 갱신
D. 출격 초기 질량 고정
```

P0 권장:

```text
D. 출격 초기 질량 고정
```

이유:

```text
- Ammo Runtime과 Chaos 재적용 복잡도 분리
- 대다수 탄약의 소량 질량 변화가 프레임별 물리 재설정을 정당화하지 않음
- 피팅의 핵심 감각 검증에 집중
```

P1에서 중량탄·미사일처럼 탄약 질량 변화가 큰 빌드만 B 또는 C를 검토한다.

---

## 19. Fitting ViewData

권장 구조:

```text
FCFVehicleFittingViewData
```

필드:

```text
FittingId
VehicleDisplayName
ValidationState
bCanApply
MountRows
DefenseRow
AmmoRows
MassRows
TotalVehicleMassKg
MaximumGrossMassKg
GrossMassUsageRatio
AccelerationScale
BrakingScale
SteeringResponseScale
IssueRows
```

Blueprint는 ViewData를 표시만 한다.
장착 호환이나 질량 공식을 WBP 그래프에 복제하지 않는다.

---

## 20. P0 UI 상태

최소 상태:

```text
Valid
ValidWithWarnings
Invalid
Applied
NotApplied
```

필수 메시지 예:

```text
- Top_01에 선택한 장비가 Turret Mount를 지원하지 않습니다.
- 선택한 무기 크기 Large가 슬롯 제한 Medium을 초과합니다.
- 탄약 적재량이 최대 적재량을 초과합니다.
- 총중량이 최대 허용 총중량을 120kg 초과합니다.
- 무기 질량 데이터가 없어 피팅을 적용할 수 없습니다.
```

`???` 같은 일반 표시보다 구체적 실패 사유를 우선한다.

---

## 21. P0 테스트 데이터 원칙

원본 전투 자산을 수정하지 않는 별도 테스트 자산을 사용한다.

필수 세트:

```text
Default
Light
Heavy
InvalidMountType
InvalidWeaponSize
OverGrossMass
MissingMassData
```

첫 차량은 다중 슬롯보다 한 개 `Top_01` 장착으로 검증한다.
다중 MountProfile Automation은 순수 데이터 테스트로 먼저 보장할 수 있다.

---

## 22. 자동화 설계

### Data Contract

```text
- 기본값 호환
- 음수·NaN 질량 보정 또는 Error
- ID·Summary
- 직렬화 필드 존재
```

### Compatibility

```text
- 유효 MountType·Size
- Unknown MountProfile
- Missing Hardpoint
- Incomplete EquipmentPreset
- Weapon incompatibility
- 중복 Selection
```

### Mass Snapshot

```text
- Mount+Weapon 합산
- Ammo 수량×단위질량
- Defense 합산
- 장전·예비 중복 없음
- GrossMass 한도
- 여러 Mount 합산
```

### Runtime

```text
CarFight.Fitting.FIT_P0_04.RuntimeApply
- Fitting 미지정 Legacy Prepare·Commit
- Valid Snapshot Weapon·Defense 적용
- AppliedFittingSnapshot 소유
- 같은 Snapshot 재초기화 중 중복 상태 없음
- Reset 안전

CarFight.Fitting.FIT_P0_04.AtomicBoundary
- Invalid Snapshot Adapter 무호출
- Defense 단계 실패 시 직전 Weapon·Defense 입력 복원
- 기존 AppliedFittingSnapshot 보존
- 명시적 Prepared Rollback은 하위 Runtime 무호출
```

### Regression

```text
- Launcher
- WeaponFire
- Projectile Pool
- Damage
- VehicleDefense
- Ammo
- VehicleRuntime
```

---

## 23. FIT-P0-01 Decision Record

### 23.1 질량 SSOT와 물리 적용

```text
Design SSOT
= VehicleData.BaseVehicleMassKg
+ Snapshot의 장비·탄약·방어 질량

Runtime Apply SSOT
= Snapshot.TotalVehicleMassKg
→ Chaos Wheeled VehicleMovement 공식 Mass 경로

Diagnostic Only
= VehicleMesh / PhysicsAsset 계산 질량
```

대안이었던 PhysicsAsset 자동 계산 SSOT는 Pawn 없는 Preview가 불안정하고 자산 변경에 따라 피팅 결과가 숨게 되므로 채택하지 않는다. BodyInstance만 직접 Override하는 방식도 Movement 시뮬레이션과 물리 바디의 기준 불일치 위험 때문에 채택하지 않는다.

### 23.2 데이터 소유권

```text
VehicleData.BaseVehicleMassKg
VehicleData.MaximumGrossMassKg
TurretMountData.TurretMountWeightKg
WeaponData.WeaponMassKg
VehicleDefenseData.DefenseMassKg
AmmoData.UnitMassKg
```

EquipmentPresetData는 조합만 소유하며 별도 Overhead 질량을 갖지 않는다.

### 23.3 Fitting Data와 Runtime

```text
UCFVehicleFittingData
= 출격 선택 PrimaryDataAsset

UCFVehicleFittingComp
= Validation, Resolved Snapshot, Applied Snapshot

ACFVehiclePawn
= 초기화 순서 조정
```

VehicleData 원본, Weapon Runtime, Ammo Runtime과 Defense Runtime의 현재 상태를 피팅 컴포넌트로 이동하지 않는다.

### 23.4 한도와 과적

```text
MaximumGrossMassKg = 유일한 저장 한도
MaximumPayloadMassKg = MaximumGrossMassKg - BaseVehicleMassKg 파생값
TotalVehicleMassKg > MaximumGrossMassKg = Validation Error / 적용 금지
```

P0에서는 과적 페널티 빌드를 지원하지 않는다.

### 23.5 Mount Fallback과 빈 장착

```text
MissingMountSelectionPolicy 기본값 = UseVehicleDefault
누락 = VehicleData.DefaultEquipmentPresetData
명시 빈 장착 = bEnabled=false 또는 명시 Empty 선택
```

Snapshot은 최종 장비가 피팅 선택인지 Vehicle 기본값인지 Source를 기록한다.

### 23.6 Defense 선택

```text
UseVehicleDefault
ExplicitNone
Override
```

Null 하나로 Default와 None을 동시에 표현하지 않는다.

### 23.7 탄약 질량

P0는 출격 초기 `InitialSortieAmmoCount × UnitMassKg`를 한 번 계산해 고정한다. 장전량과 예비량을 중복 합산하지 않으며 매 발사 질량 갱신은 P1로 보류한다.

### 23.8 Mobility

실제 Chaos 질량이 우선이다. 선택적 Adapter는 M5 측정 뒤 필요한 축에만 `Pow(1/MassRatio, Exponent)`·Clamp 공식을 사용하며 기본 비활성이다.

### 23.9 테스트 플랫폼

미래 테스트 자산은 읽기 전용 `DA_TestSUV`를 원본으로 하는 `DA_VehicleFitting_TestSUV` 복제다. 현재 BP 기본값, TestMap, DA_TestSUV와 CF-FQ-029 Launcher 자산은 수정하지 않는다.

### 23.10 UI 책임

피팅은 Validation·Snapshot·ViewData를 소유한다. CF-FQ-032는 UI Root, 화면 수명과 표시를 소유한다. UI Framework 전에도 Debug Panel로 같은 ViewData를 검증할 수 있다.

### 23.11 Gate 책임 분리

피팅 데이터 계약과 실제 물리 질량 적용은 서로 다른 단계다.

```text
FIT-P0-02
= 타입, DataAsset, 질량 필드, 기본값 호환, DataValidation
= Chaos Vehicle·PhysicsAsset 접근 없음

FIT-P0-03
= Pawn 없는 Compatibility Validation과 결정론적 Mass Snapshot
= Chaos Vehicle·PhysicsAsset 접근 없음

FIT-P0-05
= Snapshot.TotalVehicleMassKg의 실제 Chaos Vehicle 적용
= Engine Mass API·PhysicsAsset 증거 필수
```

따라서 엔진 API와 PhysicsAsset 기준값은 `FIT-P0-05 Runtime Mass Gate`로 이동한다. 이 Gate는 FIT-D 결정을 다시 여는 절차가 아니며 안전한 적용 계약을 확인하는 증거 Gate다.

### 23.12 FIT-P0-02 구현 계약

실제 구현된 공용 타입:

```text
ECFFittingValidationState
ECFFittingIssueSeverity
ECFFittingIssueCode
ECFMissingMountPolicy
ECFDefenseSelectionMode
ECFFittingSelectionSource
FCFFittingValidationIssue
FCFVehicleMountSelection
FCFVehicleDefenseSelection
FCFResolvedFittingMount
FCFVehicleFittingSnapshot
```

실제 구현된 DataAsset 계약:

```text
UCFVehicleFittingData
- FittingId / DisplayName / VehicleData
- MountSelections
- MissingMountSelectionPolicy
- DefenseSelection
- FittingTags
- ValidateFittingDataContract
- BuildVehicleFittingSummary
- Editor DataValidation
```

실제 구현된 질량 소유권:

```text
VehicleData.BaseVehicleMassKg
VehicleData.MaximumGrossMassKg
TurretMountData.TurretMountWeightKg — 기존 필드 재사용
WeaponData.WeaponMassKg
VehicleDefenseData.DefenseMassKg
```

### 23.13 호환성과 검증 결과

```text
- 신규 질량 필드 기본값 0kg = 미설정
- 기존 차량 주행·발사·방어 결과 변화 없음
- FittingData 미지정 Legacy 경로 유지
- Ammo 선택 타입 중복 생성 없음
- VehiclePawn·Blueprint·DataAsset·Map 변경 없음
- Official Editor Build PASS
- CarFight.Fitting.FIT_P0_02.DataContract Success
- Automation Warnings 0 / Errors 0
- 전체 CarFight 필수 전투 회귀 15/15 Success
```

### 23.14 FIT-P0-03 구현 결과

```text
- UCFVehicleFittingData::BuildFittingSnapshot const 순수 함수 구현
- VehicleData MountProfile 순서 기반 결정론적 ResolvedMounts
- Hardpoint, EquipmentPreset, Weapon 호환 ValidationIssues
- MissingMountSelectionPolicy 3종과 명시적 빈 장착 해석
- Defense 선택 3상태와 ResolvedDefenseSelectionMode 보존
- Base·Mount·Weapon·Defense 질량과 GrossMass 검증
- Compatibility Automation Success / Warnings 0 / Errors 0
- MassSnapshot Automation Success / Warnings 0 / Errors 0
- 전체 Automation 27 Success / Failed 0
- 필수 회귀 17/17 Success
```

최종 빌드 검증:

```text
- 신규 피팅 소스와 테스트 개별 컴파일 PASS
- UnrealEditor-CarFight_Re.dll 링크 PASS
- 후속 공식 Editor Build 4f2daa07187a41aea7c400386010b2a0: Exit Code 0
- FIT-P0-04 최종 공식 Editor Build 52b5b5572e2a4c739771dbd711a284d3: Exit Code 0
- 과거 UnrealEditor-NetCore.dll 파일 점유 차단은 해소된 이력
```

따라서 FIT-P0-03은 Done이며 FIT-P0-04 Runtime 적용도 구현·빌드·Automation 완료 상태다.

### 23.15 FIT-P0-04 구현 결과

```text
UCFVehicleFittingComp
- ECFFittingRuntimeApplyState
- FCFFittingWeaponRuntimeInput
- FCFFittingDefenseRuntimeInput
- FCFFittingSortieRuntimeInput
- ICFFittingRuntimeApplyAdapter
- PrepareSortieFitting
- CommitPreparedSortieFitting
- CommitPreparedSortieFittingToVehicle
- RollbackPreparedSortieFitting
- ResetFittingRuntimeState
- AppliedFittingSnapshot
```

원자성:

```text
- Prepare는 Snapshot 검증과 Runtime 입력 생성만 수행
- Invalid Snapshot은 하위 Adapter 호출 없음
- Commit은 Weapon → Defense 순서
- 모두 성공한 경우에만 Applied Snapshot 교체
- 부분 실패는 직전 Applied 또는 Legacy 입력으로 Weapon·Defense 전체 복원
- EndPlay과 Reset은 Prepared·Applied Transient 상태를 정리
```

검증:

```text
Official Editor Build 52b5b5572e2a4c739771dbd711a284d3: Exit Code 0
CarFight.Fitting.FIT_P0_04.RuntimeApply: Success
CarFight.Fitting.FIT_P0_04.AtomicBoundary: Success
전체 CarFight Automation: 40/40 Success
필수 회귀: 19/19 Success
```

제외 범위인 Runtime Mass, Ammo 임시 타입, Inventory Adapter, Field Fitting, UI와 Unreal Asset은 변경하지 않았다.

### 23.16 FIT-P0-05 Runtime Mass Gate 결과

#### 조사 증거

```text
Engine Runtime: 5.8.0
BP_CFVehiclePawn Parent: /Script/CarFight_Re.CFVehiclePawn
Physics Root: VehicleMesh / USkeletalMeshComponent
Movement: VehicleMovementComp / UChaosWheeledVehicleMovementComponent 계열
VehicleMesh SkeletalMesh: /Game/Vehicles/SportsCar/SKM_SportsCar
동일 폴더 PhysicsAsset 자산: /Game/Vehicles/SportsCar/PA_SportsCar
SM_Body: QueryOnly 시각 피격 표면
```

읽기 전용 덤프:

```text
8273dbea503d4d98b4247508f6d018e6
- /Game/CarFight/Vehicles/Blueprints
- 3/3 Success
- BP_CFVehiclePawn의 VehicleMesh·VehicleMovementComp 참조 확인

a63d09b80c014aeb9b7e83f026e6f79a
- /Game/Vehicles/SportsCar
- 24/24 Success
- SKM_SportsCar와 PA_SportsCar 자산 존재 확인
```

현재 AssetDump는 SkeletalMesh와 PhysicsAsset의 Details를 지원하지 않으므로 SKM→PA 직접 연결, Body별 Density·MassScale와 계산 질량 수치는 정적 덤프로 확정하지 않는다. BP 컴포넌트에 명시적 PhysicsAsset Override 참조는 발견되지 않았으며 실제 런타임에서는 `VehicleMesh.GetPhysicsAsset()` 결과를 증거로 사용한다.

#### 질량 SSOT

```text
피팅 계산 SSOT
= FCFVehicleFittingSnapshot.TotalVehicleMassKg

Chaos 설정 Mirror
= UChaosVehicleMovementComponent::Mass

실제 Runtime 증거
= VehicleMesh.GetMass()

PhysicsAsset
= 충돌 형상·관성 계산 원본과 진단 경로
= 피팅 질량 SSOT 아님
```

`VehicleData.BaseVehicleMassKg`는 Snapshot의 플랫폼 질량 입력이며 실제 VehicleMesh 계산 질량을 자동 복사하지 않는다.

#### 적용 수명

```text
PreRegisterAllComponents / Super 이전
1. VehicleData·VehicleFittingData에서 전체 Snapshot을 순수 계산
2. 기존 Movement Mass 캐시
3. Snapshot Valid와 TargetMass 유효성 확인
4. MovementComponent.Mass에 TargetMass 1회 기록
5. Prepared Snapshot과 TargetMass 캐시

Component Registration
6. Super::PreRegisterAllComponents
7. Chaos Vehicle Physics State·Simulation 생성

BeginPlay
8. Movement Mass·VehicleMesh.GetMass·PhysicsAsset·Physics State 검증
9. 검증 성공 시 같은 Cached Snapshot으로 Weapon·Defense Commit
10. AppliedFittingSnapshot과 Applied Mass 상태 확정
```

PreRegister와 BeginPlay에서 Snapshot을 독립적으로 두 번 계산해 서로 다른 선택을 적용하지 않는다.

#### SetMassOverrideInKg 판정

```text
가능한 저수준 Primitive Body API
≠ CarFight 초기 출격 Mass SSOT

초기 출격 프로젝트 코드 직접 호출: 금지
VehicleMesh Body-only Override: 금지
Movement Mass와 Body Mass 이중 독립 기록: 금지
PhysicsAsset 저장 수정·MassScale 우회: 금지
```

Chaos 초기화가 Movement `Mass` 설정값을 실제 Body에 전파하도록 둔다. 후속 필드 재적용에서 이 API가 필요하면 Movement 설정, Body 실제값, Physics State와 Vehicle Simulation을 함께 다루는 별도 Adapter·재생성 테스트를 먼저 통과해야 한다.

#### Physics State와 Simulation 재생성

```text
초기 출격 / 물리 등록 전 적용
→ 재생성 불필요

같은 TargetMass 재초기화
→ 재적용 없이 검증만 수행

물리 생성 뒤 다른 TargetMass
→ FIT-P0-05에서 거부
→ 자동 RecreatePhysicsState·Simulation Reset 없음
→ FFIT-P0-04 후속 범위
```

따라서 초기 출격 FIT-P0-05는 정확한 Hot Recreate 함수 이름에 의존하지 않는다.

#### 실제 질량 검증

```text
TargetMassKg = CachedSnapshot.TotalVehicleMassKg
ConfiguredMassKg = VehicleMovementComponent.Mass
ActualMassKg = VehicleMesh.GetMass()
ToleranceKg = Max(1.0, TargetMassKg × 0.01)
```

Commit 조건:

```text
- Target·Configured·Actual 모두 유한하고 0보다 큼
- Configured가 Target 허용 오차 안 — Snapshot 설정값 전파 정확성
- Actual + Tolerance >= Target — VehicleMesh 실제 집계 질량이 Target을 충분히 포함
- Actual이 Target보다 큰 양의 오버헤드는 PhysicsAsset 보조 Body 집계 증거로 허용
- VehicleMesh Physics State 생성
- VehicleMesh Simulate Physics 활성
- VehicleMesh.GetPhysicsAsset() 유효
```

2026-08-04 실제 SUV 회귀에서 `Target=1570kg`, `Configured=1570kg`, `Actual=1837.377kg`이 관찰됐다. Legacy 1500kg에서도 Actual이 1767.377kg으로 같은 `+267.377kg` 차이를 보여, Snapshot 계산 오류가 아니라 현재 PhysicsAsset 구성에서 반복되는 실제 집계 질량 오버헤드로 판정했다. 이 값의 엔진 내부 세부 구성은 별도 PhysicsAsset 분석 전까지 추정으로 확대하지 않으며, P0 검증은 설정값 정확 일치와 실제 질량 하한 Coverage만 잠근다.

#### Legacy·실패 정책

```text
VehicleFittingData 없음
→ Movement Mass 무수정
→ 기존 VehicleData Weapon·Defense와 기존 Chaos 질량 유지

Snapshot Invalid
→ Movement Mass 무수정
→ Weapon·Defense Legacy 경로

PreRegister 기록 실패
→ Super 호출 전 기존 Movement Mass 복원
→ Legacy 경로

Post-Physics 설정 질량 불일치 또는 실제 집계 질량 Target 부족
→ Snapshot Weapon·Defense Commit 금지
→ VehicleRuntime Ready 실패
→ Body Override나 자동 재생성으로 Legacy 복원을 가장하지 않음
```

Actual이 Target보다 큰 양의 오버헤드는 실패가 아니다. Configured가 Target과 다르거나 Actual이 허용 오차보다 크게 부족한 경우는 물리 생성 뒤 안전한 Rollback API가 필요한 비정상 상태이므로 P0에서 조용히 계속 주행하지 않는다.

#### 재초기화와 EndPlay

```text
같은 Snapshot·질량
→ Idempotent Verify Only

다른 Snapshot·질량 / Physics State 존재
→ 요청 거부 / Respawn 필요

EndPlay
→ Prepared·Applied Mass 상태 캐시만 정리
→ 파괴 중인 Body에 Mass 복원·Physics 재생성 호출 없음
```

#### 구현 판정

```text
Initial Sortie Runtime Mass Contract: Locked and Implemented
FIT-P0-05 Initial Sortie Source: Code Complete
Official Editor Build: PASS
Pawn-less Initial Mass Automation: PASS
Actual BP/DataAsset Game World Physics Automation: PASS
User PIE: Pending
Mobility Measurement: Pending
Field Runtime Mass Reapply: Blocked / FFIT-P0-04
PhysicsAsset 편집: Not Required / Forbidden in P0
```

실제 구현:

```text
UCFVehicleFittingComp v1.2.0
- ECFInitialMassState
- PrepareInitialSortieFitting
- CalculateInitialMassToleranceKg
- RecordInitialMassBeforePhysics
- VerifyInitialMassAfterPhysics
- Movement 설정 질량의 Snapshot Target 정확 비교
- VehicleMesh 실제 집계 질량의 Target 하한 Coverage와 양의 오버헤드 허용
- FallbackPreparedInitialMassToLegacy
- 같은 Target Verify Only
- 다른 Target·Legacy 전환 ReapplyRejected
- ResetFittingRuntimeState Mass 캐시 정리

ACFVehiclePawn v2.133.0
- PreRegisterAllComponents / Super 이전 Target Mass 적용
- UChaosVehicleMovementComponent::Mass 직접 읽기·쓰기
- BeginPlay VehicleMesh.GetMass·GetPhysicsAsset·IsPhysicsStateCreated·IsSimulatingPhysics 검증
- 검증 성공 뒤 같은 Cached Snapshot Weapon·Defense Commit
```

공식 검증:

```text
Official Editor Build f4af0979dfbb40268c77d83254e65b4c
- CarFight_ReEditor Win64 Development
- Exit Code 0
- UCFVehicleFittingComp v1.2.0과 실제 BP/DataAsset 회귀 컴파일·링크 증명

Targeted Runtime 82bf351952b54b4e8cef17908a7b07af
- CarFight.Fitting.FIT_P0_05.DefensePIEPipeline: Success
- Snapshot Valid / TotalMass 1570kg
- InitialMass Verified / CommittedSnapshot
- Configured 1570kg / Actual 1837.377kg / ActualCoverage Passed / Overhead 267.377kg
- WeaponReady Yes / DefenseReady Yes / ActiveDefenseData DA_VehicleDefense_Test

Combat Runtime 57b649a3e3264007aae4b61ce56df0ca
- 전체 CarFight 44/44 Success
- 필수 회귀 22/22 Success
- Failed 0 / Missing 0
- Editor Exit Code 0 / Process Exit Code 0
```

따라서 BodyInstance 우회는 필요하지 않다. 실제 BP/DataAsset Game World 자동화에서 질량 설정·Physics State·Weapon·Defense Commit은 확인됐으며, 사용자가 확인하는 보호형 PIE와 질량에 따른 기동 차이는 남은 검증 단계다.

---

## 24. 설계 결론

CarFight 피팅 P0의 핵심은 많은 장비 종류가 아니다.
현재 검증된 데이터 체계를 훼손하지 않고 다음 변환을 신뢰 가능하게 만드는 것이다.

```text
Vehicle Platform
+ Owned Item Instances
+ Mount Equipment Choices
+ Defense Package
+ Ammo Load
→ Compatibility·Ownership Validation
→ Deterministic Mass Snapshot
→ Sortie Apply 또는 조건부 Field Timed Action
→ Atomic Runtime·Inventory Commit
→ Observable Mobility Change
```

Inventory Foundation은 필드 피팅을 위해 최소 범위로 선행 구축한다. 상점, 루팅, 경제와 영구 저장은 이 기반 이후 별도 기능으로 확장한다.

---

## 25. Changelog

### v0.9.0 - 2026-08-04

```text
- 실제 BP_CFVehiclePawn·DA_VehicleDefense_TestSUV·DA_Fit_DefenseTestSUV Game World 회귀를 추가해 Fitting Snapshot → Initial Mass → Weapon Commit → Defense Commit → ActiveDefenseData 전체 경로를 검증했다.
- Snapshot·Movement 설정 질량은 1570kg으로 정상이나 VehicleMesh.GetMass가 1837.377kg을 반환해 기존 정확 일치 Gate가 Snapshot Commit을 차단하던 원인을 확정했다.
- Legacy 1500kg에서도 동일한 +267.377kg 차이가 반복돼 실제 집계 질량의 양의 오버헤드를 허용하고 Target보다 허용 오차 이상 부족한 경우만 실패하는 Coverage 계약으로 수정했다.
- UCFVehicleFittingComp v1.2.0, CFFittingRuntimeTests v1.3.0과 Combat 필수 회귀 22개를 현재 구현에 반영했다.
- 공식 Editor Build PASS, DefensePIEPipeline PASS, 전체 CarFight 44/44·필수 22/22 Success를 기록했다.
- SetMassOverrideInKg, Body-only Override, PhysicsAsset·MassScale 수정, Hot Recreate와 Unreal Asset 수정은 수행하지 않았다.
- 사용자 보호형 PIE와 Mobility 측정은 Pending이며 FIT-P0-05 전체 Done 판정은 보류했다.
```

### v0.8.0 - 2026-08-02

```text
- FIT-P0-05 Initial Sortie Runtime Mass Adapter의 실제 C++ 구조와 검증 결과를 반영했다.
- UCFVehicleFittingComp가 Cached Snapshot, Target Mass, Legacy fallback, Verify Only, ReapplyRejected와 실제 질량 검증 상태를 소유한다.
- ACFVehiclePawn은 PreRegisterAllComponents의 Super 호출 전에 Movement Mass를 설정하고 BeginPlay에서 VehicleMesh 실제 물리 계약을 검증한다.
- Initial Mass 검증 성공 뒤에만 동일 Prepared Snapshot의 Weapon·Defense Commit이 진행되도록 FIT-P0-04 경계를 확장했다.
- UE 5.8 UChaosVehicleMovementComponent::Mass public 접근과 PreRegister 수명을 공식 Build 두 건으로 증명했다.
- CarFight.Fitting.FIT_P0_05.InitialMass와 최종 전체 CarFight 41/41, 필수 회귀 20/20 Success를 기록했다.
- 첫 Automation의 단일 실패는 13.6kg 테스트 비교 정밀도 문제였으며 0.001kg 비교 허용치 명시 후 최종 PASS했다.
- SetMassOverrideInKg, Body-only Override, PhysicsAsset·MassScale 수정, Hot Recreate, Field Mass Reapply와 Mobility Scalar는 추가하지 않았다.
- Blueprint, DataAsset, PhysicsAsset, Map, SaveGame, UI, Ammo와 Inventory Adapter는 변경하지 않았다.
- 실제 Physics PIE와 기동 측정이 남아 있어 FIT-P0-05 전체 Done 판정은 보류했다.
```

### v0.7.0 - 2026-08-02

```text
- FIT-P0-05 Runtime Mass Gate를 조사해 초기 출격 질량 SSOT·적용 수명·검증 계약을 잠갔다.
- Snapshot.TotalVehicleMassKg, UChaosVehicleMovementComponent::Mass와 VehicleMesh.GetMass의 역할을 각각 Design SSOT·Configuration Mirror·Runtime Evidence로 분리했다.
- 현재 BeginPlay 적용은 늦으므로 PreRegisterAllComponents의 Super 호출 전 질량 Prepare와 1회 기록을 확정했다.
- 물리 등록 전 적용은 Physics State·Vehicle Simulation 재생성이 불필요하고, 생성 후 다른 질량 적용은 FIT-P0-05에서 거부하도록 했다.
- SetMassOverrideInKg 직접 호출, Body-only Override, PhysicsAsset·MassScale 수정과 자동 Hot Recreate를 P0에서 금지했다.
- 실제 질량 허용 오차, Legacy fallback, 비정상 Post-Physics mismatch, 같은 질량 재초기화와 EndPlay 계약을 추가했다.
- BP_CFVehiclePawn 3/3과 SportsCar 폴더 24/24 read-only AssetDump 증거를 기록했다.
- SKM_SportsCar·PA_SportsCar 자산 존재는 확인했지만 직접 연결과 계산 질량은 AssetDump 지원 한계로 Runtime 검증 항목으로 남겼다.
- Source와 Unreal Asset은 수정하지 않고 CF-FQ-032 Active와 Inventory 상태를 보존했다.
```

### v0.6.0 - 2026-08-01

```text
- UCFVehicleFittingComp의 Legacy·Snapshot Prepare, Weapon·Defense Commit과 실패 Rollback 설계를 실제 구현에 맞게 확정했다.
- AppliedFittingSnapshot을 성공 Commit 뒤에만 교체하고 무효 Snapshot에서는 Adapter를 호출하지 않는 원자 경계를 기록했다.
- VehicleWeaponComp의 Runtime EquipmentPresetData Override와 VehicleData Legacy Fallback을 구현 상태로 반영했다.
- VehiclePawn 초기화에서 Fitting Commit 뒤 터렛 시각 적용과 Launcher 연결 순서를 확정했다.
- RuntimeApply·AtomicBoundary Pawn 없는 Automation과 전체 40/40, 필수 19/19 Success를 기록했다.
- 공식 Editor Build 52b5b5572e2a4c739771dbd711a284d3 / Exit Code 0을 기록했다.
- FIT-P0-05 Runtime Mass Gate와 금지된 우회는 변경하지 않았다.
```

### v0.5.0 - 2026-08-01

```text
- Saved Fitting Template, Field Fitting Draft와 Applied Snapshot 계층을 분리했다.
- CF-FQ-035 Inventory Foundation의 Item Instance·Container·Reservation·Atomic Transfer 의존성을 추가했다.
- 비전투·전체 관련 쿨타임 종료·차량 정지·Runtime Ready·예약 성공을 Field Action 시작 조건으로 잠갔다.
- Equip은 호환되는 빈 슬롯에만 허용하고 Replace를 Unequip과 Equip 두 액션으로 분리했다.
- 시간 액션 동안 Reservation만 만들고 완료 순간 Runtime과 Inventory를 원자 Commit하는 계약을 정의했다.
- 이동·전투·피해·상태 변화·예약 손실·화면 종료 취소와 원상 유지 정책을 추가했다.
- 후속 공식 Editor Build와 전체 Automation PASS를 반영해 FIT-P0-03을 Done으로 해석한다.
```

### v0.4.0 - 2026-08-01

```text
- FIT-P0-03 BuildFittingSnapshot의 실제 결정론적 해석 순서와 소유 경계를 반영했다.
- MissingFittingId·MissingMountSelection, MissingPolicyEmpty와 ResolvedDefenseSelectionMode 계약을 추가했다.
- VehicleData MountProfile 순서, 이름 정렬 문제 순서와 MountSelections 배열 순서 독립성을 잠갔다.
- Mount·Hardpoint·EquipmentPreset·Weapon 호환, 방어 3상태와 질량·GrossMass 검증 결과를 기록했다.
- 두 FIT-P0-03 Automation과 전체 27개 Automation, 필수 회귀 17/17 Success를 기록했다.
- 신규 피팅 소스 컴파일과 CarFight 모듈 링크는 통과했으나 NetCore DLL 외부 잠금으로 공식 Editor 전체 타깃은 Blocked다.
- FIT-P0-05 Runtime Mass Gate와 금지된 우회는 변경하지 않았다.
```

### v0.3.0 - 2026-08-01

```text
- 데이터 계약과 실제 Chaos 질량 적용의 Gate 책임을 분리했다.
- FIT-P0-02 공용 enum·구조체, VehicleFittingData와 Base·Weapon·Defense 질량 계약을 구현 상태로 반영했다.
- Defense 3상태, Missing Mount 정책과 결정론적 Snapshot 공용 구조를 실제 타입명에 맞췄다.
- 신규 질량 0kg 기본값의 Legacy 호환과 Ammo 타입 보류 원칙을 기록했다.
- 공식 Editor Build, Fitting DataContract와 전체 필수 전투 회귀 PASS를 기록했다.
- Engine Mass API·PhysicsAsset 증거는 FIT-P0-05 Runtime Mass Gate로 이동했다.
```

### v0.2.3 - 2026-08-01

```text
- 피팅 설계에서 외부 도구 작업 ID, 설정, 런타임 상태와 복구 절차를 제거했다.
- 엔진·자산 증거, 금지된 우회와 Gate 판정만 남겼다.
- Gate Blocked와 Decision Lock을 유지하고 코드·에셋은 수정하지 않았다.
```

### v0.2.2 - 2026-08-01

```text
- 승인된 엔진·자산 읽기 경로를 재확인했다.
- 필요한 증거를 확보하지 못해 Gate Blocked와 Decision Lock을 유지했다.
- 코드·에셋은 수정하지 않았다.
```

### v0.2.1 - 2026-08-01

```text
- FIT-P0-02 Pre-Implementation Gate 실행 결과를 기록했다.
- 정확한 엔진 Mass API와 PhysicsAsset 계산 질량이 없어 데이터 계약 구현을 시작하지 않는다고 확정했다.
- Decision Lock은 유지하고 Source·Blueprint·DataAsset·Map은 수정하지 않았다.
```

### v0.2.0 - 2026-08-01

```text
- FIT-P0-01 Decision Lock과 FIT-D-001~014의 최종 결정을 반영했다.
- BP_CFVehiclePawn의 물리 Root VehicleMesh, SKM_SportsCar와 VehicleMovementComp 실제 연결을 기존 성공 덤프에서 확인했다.
- SM_Body를 QueryOnly 시각 피격 표면으로 분리하고 PhysicsAsset 질량과의 책임 경계를 기록했다.
- VehicleData 명시 질량 → FittingSnapshot → Chaos VehicleMovement Mass 단일 적용 구조를 확정했다.
- BodyInstance 단독 Mass Override, PhysicsAsset 수정, 질량 효과 중복 보정을 금지했다.
- Defense 3상태, Mount Fallback, GrossMass 한도, 출격 고정 탄약 질량과 전용 TestSUV 복제 정책을 잠갔다.
- 정확한 엔진 API·PhysicsAsset 수치는 FIT-P0-02 사전 증거 Gate로 이관했다.
- Source와 Unreal Asset은 수정하지 않았다.
```

### v0.1.0 - 2026-07-31

```text
- 현재 VehicleData·Hardpoint·MountProfile·EquipmentPreset·Weapon·Ammo·Defense·VehicleRuntime 구조를 기준으로 피팅 설계를 작성했다.
- 별도 FittingData와 Applied Snapshot 소유 구조를 제안했다.
- 질량 소유 후보, 총중량 공식, 검증 모델, 적용 순서와 ViewData를 정의했다.
- 미결정 질량 SSOT·Chaos 적용·기동 공식·탄약 질량 갱신 정책을 분리했다.
- Source와 Asset은 수정하지 않았다.
```

---

## 26. Migration

```text
- v0.9.0부터 Initial Sortie Mass 검증은 UCFVehicleFittingComp v1.2.0의 설정 질량 Target 일치와 실제 VehicleMesh 집계 질량 Target Coverage 계약을 기준으로 한다.
- 실제 VehicleMesh 질량이 Target보다 큰 양의 오버헤드는 허용하지만, Configured가 Target과 다르거나 Actual이 허용 오차보다 크게 부족하면 Snapshot Commit을 거부한다.
- `CarFight.Fitting.FIT_P0_05.DefensePIEPipeline`은 실제 BP/DataAsset Game World 자동화 증거이며 사용자 보호형 PIE와 Mobility PASS를 대신하지 않는다.
- v0.8.0부터 Initial Sortie Mass 설계는 UCFVehicleFittingComp와 ACFVehiclePawn의 PreRegister 초기 적용 구현을 기준으로 한다.
- PreRegister는 게임 World와 자동 초기화 조건에서만 전체 Snapshot을 준비하고 유효 Target Mass를 Movement Component에 기록한다.
- BeginPlay는 기존 Prepared 입력이 있으면 재계산하지 않고 실제 Physics 계약 검증 뒤 Weapon·Defense Commit에 사용한다.
- 초기 Invalid Snapshot은 Legacy Runtime 입력으로 다시 준비하며 Movement Mass는 수정하지 않는다.
- 같은 Target Mass는 Verify Only, 다른 Target 또는 Snapshot 질량 적용 뒤 Legacy 전환은 ReapplyRejected다.
- Legacy 경로의 Mass 검증은 기존 Chaos 질량을 변경하지 않는 no-op 성공으로 처리한다.
- Mass 검증 실패 시 Snapshot Weapon·Defense Commit과 VehicleRuntime Ready를 허용하지 않는다.
- 공식 Build에서 UE 5.8 public Mass 접근을 증명했으므로 SetMassOverrideInKg·Body-only Override를 추가하지 않는다.
- 실제 VehicleMesh Physics PIE와 Mobility 측정 전에는 FIT-P0-05 전체를 Done으로 해석하지 않는다.
- v0.7.0부터 초기 출격 Runtime Mass는 PreRegisterAllComponents의 Super 호출 전 Movement Mass에 1회 기록한다.
- PreRegister에서 만든 전체 Cached Snapshot을 BeginPlay Weapon·Defense Commit이 그대로 재사용해야 한다.
- Snapshot 없는 Legacy 차량과 Invalid Snapshot은 Movement Mass를 수정하지 않는다.
- VehicleMesh.GetMass와 GetPhysicsAsset은 런타임 증거이며 VehicleData.BaseVehicleMassKg를 대체하지 않는다.
- 같은 질량 재초기화는 Verify Only이고 다른 질량은 Respawn 또는 FFIT-P0-04 전까지 거부한다.
- EndPlay은 Mass 상태 캐시만 정리하며 Body 복원과 Physics State 재생성을 수행하지 않는다.
- SetMassOverrideInKg, Body-only Override, PhysicsAsset·MassScale 저장 수정은 FIT-P0-05 초기 출격 구현에 사용하지 않는다.
- UE 5.8 public Mass 심볼은 첫 Editor Build에서 검증하고 실패하면 우회하지 않고 Gate를 Blocked로 되돌린다.
- v0.6.0부터 FIT-P0-04의 Applied Runtime Snapshot과 Weapon·Defense 원자 적용 계약이 실제 구현 기준이다.
- FittingData 미지정 차량은 VehicleData Legacy 입력을 Commit하며 AppliedFittingSnapshot을 생성하지 않는다.
- Snapshot Invalid에서는 하위 Runtime을 호출하지 않고, Commit 실패에서는 직전 Applied 또는 Legacy 입력으로 복원한다.
- Runtime Mass와 Ammo는 FIT-P0-04 적용 입력에 포함하지 않으며 후속 Gate·기능 타입을 기다린다.
- v0.5.0부터 FIT-P0-03은 공식 Editor Build와 Automation 증거를 포함해 Done이다.
- 기존 VehicleFittingData는 Saved Template·Legacy 입력으로 유지하고 실제 소유 Item Instance와 동일시하지 않는다.
- Field Fitting Draft는 CF-FQ-035 INV-P0-04 Adapter가 해석한 실제 ItemInstanceId를 사용한다.
- INV-P0-03 전에는 시간 액션 Reservation·Transfer를 임시 배열로 구현하지 않는다.
- Equip은 Occupied Slot을 직접 교체하지 않고 Unequip 완료 후 별도 Equip을 수행한다.
- 필드 Runtime Mass 재적용은 Runtime Mass Gate 잠금만으로 시작하지 않으며 FIT-P0-05 초기 적용 검증과 FFIT-P0-03 완료 뒤 별도 Physics 재생성 계약으로 진행한다.
- BuildFittingSnapshot은 MountSelections 순서가 아니라 VehicleData.MountProfiles 순서를 사용한다.
- ExplicitEmpty와 MissingPolicyEmpty는 EquipmentPresetData가 없어도 정상 빈 장착이며 SelectionSource를 구분한다.
- UseVehicleDefault의 방어 None과 ExplicitNone은 ResolvedDefenseSelectionMode로 구분한다.
- 선택된 실제 TurretMount, Weapon과 Defense의 0kg 질량은 MissingMassSource Error다.
- Ammo 선택 타입은 CF-FQ-031의 실제 계약을 재사용하며 AmmoMassKg는 연동 전 0이다.
- Runtime Mass Gate는 초기 적용 API·수명·검증 기준을 잠갔고 PhysicsAsset 직접 경로와 실제 질량은 FIT-P0-05 Runtime 검증에서 수집한다.
- API 이름이나 PhysicsAsset 질량을 문서의 권장 구조만으로 추측하지 않는다.
- v0.2.0부터 FIT-D-001~014는 Decision Locked 상태다.
- VehicleData 명시 질량과 Chaos VehicleMovement 적용 경로를 사용하며 PhysicsAsset이나 BodyInstance 단독 Override로 대체하지 않는다.
- CF-FQ-031 Ammo와 CF-FQ-033 Defense의 런타임 소유권을 유지한다.
- 피팅 P0 완료 전에는 이 문서를 Current System으로 해석하지 않는다.
```