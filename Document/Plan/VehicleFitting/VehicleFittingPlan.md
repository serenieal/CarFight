# Vehicle Fitting Implementation Plan

- Version: 0.18.0
- Date: 2026-09-14
- Status: Done / Historical + Retained Path / Rebaseline Complete
- Feature: `CF-FQ-034 차량 피팅·질량 런타임`
- Priority: Historical P1 Feature / Internal P0 Implementation Scope
- Current Lifecycle: `SUPERSEDED / CLOSE` — 현재 프로젝트의 단일 Active Feature는 `CF-FQ-039` 유지
- Preserved Paused Feature: `CF-FQ-032` — USER Visual checkpoint
- Dependencies: `CF-FQ-031 차량 탄약·재장전 런타임`, `CF-FQ-032 UI Framework`, `CF-FQ-033 차량 방어·손상 런타임`, `CF-FQ-035 인벤토리 Foundation`
- Representative Plan: `Document/Plan/VehicleFitting/VehicleFittingPlan.md`
- Design: `Document/Plan/VehicleFittingDesign.md`
- Roadmap: `Document/Plan/VehicleFittingRoadmap.md`
- Strategic Direction: `Document/ProjectSSOT/CombatPlan/13_Fitting.md`

---

## 2026-09-14 Final Rebaseline Closure

`CF-FQ-034 차량 피팅·질량 런타임`은 과거 `FIT-P0-07D USER Driving Feel Comparison`을 그대로 재개하지 않고 현재 Source와 Current Systems를 기준으로 Rebaseline한 결과 **Done / Historical + Retained Path**로 종료한다.

원래 이 Feature가 소유하던 핵심 요구는 다음이었다.

```text
VehicleData / Fitting selection
→ 결정론적 Fitting Snapshot
→ 총질량 계산과 호환성 검증
→ Initial Sortie / Field Fitting Runtime 적용
→ Weapon / Defense / Ammo / Mass의 일관된 적용·복구
→ 실제 Chaos Mass의 정량 확인
```

이 핵심 기능은 현재 Source에 유지되어 있을 뿐 아니라 당시보다 확장되어 있다. `UCFVehicleFittingComp`는 Prepared/Applied Fitting Snapshot을 소유하고, `ACFVehiclePawn`은 Snapshot의 `TotalVehicleMassKg`를 Physics 등록 전에 Chaos Vehicle Movement에 적용한 뒤 BeginPlay에서 Configured/Actual Mass와 Physics State를 검증한다. 검증된 같은 Snapshot이 Weapon·Defense Runtime Commit의 전제가 되며, `FCFFieldFitCoordinator`는 필드 변경 시 Runtime/Mass/Inventory 완료 경계와 실패 복구를 소유한다.

따라서 Rebaseline 판정은 다음과 같다.

```text
FIT-P0-00~06
= Historical Technical checkpoint preserved

FIT-P0-07A~07C
= Historical Technical PASS / quantitative Mobility evidence preserved

FIT-P0-07D USER Driving Feel Comparison
= Superseded / Not Executed
= USER PASS 아님
= Feature closure blocker 아님

FFIT-P0-05 Field Fitting UI / 16:9·32:9 가독성
= CF-FQ-034 Runtime 완료조건에서 분리
= Inventory/UI successor workflow의 별도 USER 범위
```

`FIT-P0-07D`가 확인하려던 것은 새로운 Fitting 기능 존재 여부가 아니라 이미 기술적으로 측정된 Light / Default / Heavy 질량 차이를 사용자가 어떻게 느끼는지였다. 현재는 `VehicleBuilder.md`의 `Technical Benchmark + USER Feel Pair`, `Vehicle Character / Reference Baseline`, `Measurement Gap / Benchmark Extension Ownership`이 같은 종류의 성능·체감 판정을 더 일반적인 Current 절차로 소유한다.

따라서 과거 세 Fixture를 다시 달려보지 않았고 USER PASS도 추가하지 않는다. 향후 특정 장비 질량이 주행감을 악화시키는지 실제로 튜닝할 필요가 생기면 그것은 `CF-FQ-034` 재개가 아니라 현재 VehicleBuilder Performance Tuning Protocol을 사용하는 **Deferred observational debt**다.

Current owner:

```text
Fitting / Initial & Field Mass Runtime
→ Document/Systems/Vehicles/VehicleRuntime.md v1.3.0

Vehicle mass authoring / validation foundation
→ Document/Systems/Vehicles/VehicleData.md v2.3.0

Mass-related performance tuning workflow
→ Document/Systems/Vehicles/VehicleBuilder.md v1.6.0
```

Lifecycle:

```text
G0 = PASS — 실제 Source의 Fitting/Mass runtime + 기존 FIT-P0-07A~07C evidence + successor coverage 확인
G1 = PASS — 영구 Runtime/Mass 계약을 VehicleRuntime v1.3.0으로 승격, VehicleData/VehicleBuilder existing owner 재사용
G2 = PASS — ActiveWork / ProjectSSOT / Plan Index의 Paused current route 제거
G3 = PASS — VehicleFittingPlan.md retained path로 Historical evidence 보존
G4 = PASS — semantic Historical
G5 = Deferred — physical Archive move optional
```

이번 Rebaseline은 문서/책임 lifecycle 정리이며 새 Product Source/Asset mutation, Build, Automation, PIE 실행은 0이다. 기존 완료 evidence를 새 failure 없이 반복하지 않는다.

---

## 1. 목적

CarFight의 기존 차량·하드포인트·장착 프로파일·장비 프리셋·무기·탄약·방어 데이터를 하나의 검증 가능한 피팅으로 조합하고, 출격 초기 적용과 조건부 필드 장착·해제에 공통으로 사용하는 결정론적인 질량 Snapshot과 차량 런타임 입력으로 변환한다.

이 기능은 기존 `UCFVehicleData`를 피팅 화면에서 직접 수정하는 기능이 아니다.
차량 플랫폼 원본과 플레이어가 선택한 출격 구성을 분리하고, 피팅 적용 실패 시 원본 차량 기본 구성을 훼손하지 않는 구조를 목표로 한다.

```text
VehicleData = 차량 플랫폼과 기본 구성
VehicleFittingData = 저장된 피팅 Template과 Legacy 출격 선택
FieldFittingDraft = 실제 Item Instance를 사용하는 편집 중 선택
VehicleFittingSnapshot = 검증을 통과한 해석 결과
AppliedFittingSnapshot = 현재 차량에 Commit된 읽기 전용 결과
VehicleRuntime = Snapshot을 실제 Pawn과 Chaos Vehicle에 적용
```

이 Plan은 구현 진행 계획이며 `FIT-P0-04` 출격 피팅 Prepare·Commit·Rollback, Weapon·Defense 원자 적용과 AppliedFittingSnapshot 소유까지 실제 소스에 적용됐다. `FIT-P0-05` Initial Sortie Runtime Mass Adapter도 물리 등록 전 질량 설정, BeginPlay 실제 질량 검증, Legacy fallback과 재적용 거부까지 구현·빌드·자동화가 완료됐다. `FIT-P0-07C`에서 공식 Light·Default·Heavy를 fixture별 fresh PIE lifetime으로 분리해 실제 Chaos 가속·제동·조향 수치까지 확보했으며, 남은 Mobility Gate는 사용자가 직접 느끼는 주행감 비교다.
P0 구현과 사용자 PIE를 완료하기 전에는 `Document/Systems/Vehicles/VehicleFitting.md`를 Current System으로 생성하거나 등록하지 않는다.

---

## 2. 조사 결과 요약

### 2.1 현재 차량 플랫폼

현재 `UCFVehicleData`는 다음 실제 런타임 입력을 소유한다.

```text
- 차체·휠 시각 자산
- VehicleMovementConfig
- WheelVisualConfig
- VehicleReferenceConfig
- VehicleDurabilityConfig.MaxHealth
- HardpointSlots
- MountProfiles
- DefaultDefenseData
- DriveStateConfig
```

`FIT-P0-02~03` 완료 범위로 다음 피팅 기반은 실제 소스에 존재한다.

```text
- VehicleData 기준 질량과 최대 허용 총중량
- 별도 VehicleFittingData 출격 프리셋
- MountProfile별 장착 선택 배열
- 구조화된 ValidationIssues와 ValidationState
- 결정론적 ResolvedMounts와 질량 Snapshot
```

Historical v0.17.0 시점에 남아 있다고 기록됐던 사용자-facing 검증 범위:

```text
- FIT-P0-07D: 같은 공식 Light·Default·Heavy의 사용자 직접 주행감 비교
- FIT-P0-06 ViewData를 실제 피팅 화면에 표시하는 `FFIT-P0-05 Field Fitting UI and PIE`
- 16:9·32:9 실제 UI 가독성·진행률·취소 사유 USER 검증
```

`FIT-P0-07C` 정량 계측은 Technical Complete이며, 그 수치만으로 주행감 PASS 또는 추가 Mobility Scalar 필요성을 자동 판정하지 않는다.

현재 완료된 연결 경계:

```text
- INV-P0-04: 실제 ItemInstance → Equipment·Defense Binding → 기존 Fitting Snapshot 변환
- INV-P0-05: 읽기 전용 Inventory Snapshot·Reservation·CompatibilityHint·ChangeSet
- FFIT-P0-01: Combat·Drive·Runtime·Weapon·Launcher·Ammo·Defense·Action·Inventory Permission/Blocker Query
- FFIT-P0-02: Timed Action + Prepared Inventory Reservation + 취소 Rollback
- FFIT-P0-03: 같은 Prepared Transaction의 TimedAction → Atomic Coordinator handoff
- FFIT-P0-04: Weapon·Defense·Chaos Mass Runtime + Inventory Commit 원자 적용·RecoveryFailed·실제 ChaosMassPIE PASS
- UCFVehicleFittingComp v1.4.0: 검증 Snapshot 직접 Prepare + Applied Runtime Checkpoint + 내부 Rollback 성공 여부
- FIT-P0-06: Snapshot 기반 read-only Fitting ViewData·Blueprint Contract Technical PASS
```

### 2.2 현재 하드포인트와 장비

현재 장착 경로는 다음과 같다.

```text
VehicleData.HardpointSlots
← MountProfile.LocationSlotRef
← MountProfile.DefaultEquipmentPresetData
   ├─ DefaultTurretMountData
   └─ DefaultWeaponData
```

현재 검증 가능한 규칙:

```text
- LocationSlotRef가 실제 HardpointSlot을 참조하는지
- EquipmentPresetData.RequiredMountType과 MountProfile.MountType이 호환되는지
- EquipmentPresetData.RequiredWeaponSize가 MountProfile.SizeLimit 이하인지
- WeaponData가 MountType과 SizeLimit을 지원하는지
```

현재 런타임은 `ActiveMountProfileId`로 한 프로파일을 선택하고, `ActiveMountProfileId=None`이면 첫 MountProfile을 사용한다.
장비 프리셋의 동적 교환이나 별도 출격 로드아웃 적용은 구현되어 있지 않다.

### 2.3 현재 질량 데이터

`FIT-P0-02` 완료 후 명시 질량 데이터 계약은 다음과 같다.

```text
VehicleData.BaseVehicleMassKg
VehicleData.MaximumGrossMassKg
TurretMountData.TurretMountWeightKg
WeaponData.WeaponMassKg
VehicleDefenseData.DefenseMassKg
```

기존 VehicleData·WeaponData·VehicleDefenseData의 신규 필드는 모두 `0kg = 미설정` 기본값으로 추가돼 기존 차량 주행·발사·방어 결과를 변경하지 않는다. 피팅에 실제로 연결할 데이터만 명시적인 밸런스 질량을 입력한다.

현재 질량 데이터·후속 범위:

```text
- UCFAmmoData.UnitMassKg와 출격 AmmoMassKg는 CF-FQ-031 Done / Current 계약으로 구현 완료
- EquipmentPresetData 자체 추가 Overhead 질량은 현재 별도 필드가 없으며 필요 시 후속 설계
- Physics State 생성 뒤 다른 피팅 질량의 Field Runtime 재적용·재생성은 FFIT-P0-04 Technical PASS
- 공인 Light·Default·Heavy의 정량 Mobility 계측은 FIT-P0-07C Technical Complete이며 남은 질량 관련 판정은 USER 주행감 비교다
```

초기 출격 질량 적용은 `FIT-P0-05`에서 다음 경로로 구현됐다.

```text
VehicleFittingSnapshot.TotalVehicleMassKg
→ PreRegisterAllComponents에서 UChaosVehicleMovementComponent::Mass 기록
→ Chaos 초기 Physics State 생성
→ BeginPlay에서 VehicleMesh.GetMass() 검증
```

다음 우회 경로는 현재도 의도적으로 사용하지 않는다.

```text
SetMassOverrideInKg
SetMassScale
ChassisMass 임의 반영
PhysicsAsset·BodyInstance 저장 수정
```

`VehicleMesh.GetMass()`는 런타임 검증 증거로만 사용하고 피팅 계산 SSOT로 사용하지 않는다. `VehicleMovementConfig.CenterOfMassOverride`는 중심질량 위치 보정이며 총중량 계산을 대신하지 않는다.

### 2.4 현재 탄약

`CF-FQ-031 차량 탄약·재장전 런타임`은 Done / Current다.

```text
UCFAmmoData.UnitMassKg
MaximumLoadableAmmoCount
VehicleFittingData.InitialSortieAmmoLoads
FCFVehicleFittingSnapshot.InitialSortieAmmoLoads
FCFVehicleFittingSnapshot.AmmoMassKg
```

피팅은 실제 출격 Ammo 선택·수량과 질량을 Snapshot에 포함하지만 Loaded·Reserve·Reload 상태를 중복 소유하지 않는다. FIT-P0-06 ViewData도 `InitialSortieAmmoCount`를 표시하며 `MaximumLoadableAmmoCount`를 현재 수량으로 대체하지 않는다.

### 2.5 현재 방어

`CF-FQ-033`에서 다음 기반이 실제 소스와 테스트 자산에 적용되어 있다.

```text
VehicleData.DefaultDefenseData
→ VehicleDefenseData
→ VehicleDefenseComp
→ Shield
→ 6방향 Armor
→ VehicleHealthComp = Vehicle Integrity
```

`VehicleDefenseData`는 Shield·재생·ArmorType·ArmorResistance·6방향 장갑 설정과 `DefenseMassKg`를 소유한다. `DefenseMassKg`는 피팅 총중량에만 사용되며 현재 Shield·Armor 피해 공식에는 영향을 주지 않는다.
피팅은 방어 공식을 재구현하지 않고, P0에서 하나의 완성된 `VehicleDefenseData` 패키지를 선택하는 역할만 맡는다.

### 2.6 에셋 확인 결과와 제한

2026-08-02 첫 `ue.batchdump_safe` 호출은 자산 로드 전에 WinError 87로 실패했으며 Unreal Asset을 변경하지 않았다. 이후 저장소의 기존 read-only `Tools/RunAssetDetailDump.ps1` 경로로 같은 프로젝트를 재조회했다.

```text
BP_CFVehiclePawn dump job 8273dbea503d4d98b4247508f6d018e6
- /Game/CarFight/Vehicles/Blueprints
- 3/3 Success / Error 0
- Engine 5.8.0

SportsCar dump job a63d09b80c014aeb9b7e83f026e6f79a
- /Game/Vehicles/SportsCar
- 24/24 Success / Error 0
- Engine 5.8.0
```

확인된 사실:

```text
- BP_CFVehiclePawn.RootComponent = VehicleMesh
- VehicleMesh 타입 = USkeletalMeshComponent
- VehicleMesh.SkeletalMesh = /Game/Vehicles/SportsCar/SKM_SportsCar
- VehicleMovementComponent = VehicleMovementComp
- VehicleDriveComp는 Owner의 UChaosWheeledVehicleMovementComponent를 캐시한다.
- SM_Body는 QueryOnly 시각 피격 표면이며 차량 물리 질량 소유자가 아니다.
- /Game/Vehicles/SportsCar/PA_SportsCar PhysicsAsset 자산이 존재한다.
- BP VehicleMesh에 명시적 PhysicsAsset Override 참조는 발견되지 않았다.
```

현재 AssetDump는 SkeletalMesh와 PhysicsAsset Details를 지원하지 않는다. 따라서 `SKM_SportsCar → PA_SportsCar` 직접 연결, Body별 Density·MassScale과 현재 계산 질량 수치는 정적 덤프만으로 확정하지 않는다. 구현 후 `VehicleMesh.GetPhysicsAsset()`과 `VehicleMesh.GetMass()`를 런타임 증거로 사용하며 이 값들은 피팅 설계 SSOT가 아니다.

### 2.7 실제 Chaos 질량 경로 조사 결과

현재 프로젝트의 물리·이동 경로는 다음과 같다.

```text
AWheeledVehiclePawn
└─ VehicleMesh: USkeletalMeshComponent = RootComponent / 실제 물리 바디
   └─ VehicleMovementComp: UChaosWheeledVehicleMovementComponent가 차량 시뮬레이션을 구동

VehicleData.VehicleVisualConfig.ChassisMesh
└─ SM_Body: UStaticMeshComponent = 시각 차체 / QueryOnly 피격 표면
```

현재 `ApplyVehicleMovementConfig()`는 ChassisHeight, Drag, Downforce, CenterOfMassOverride, Engine, Differential과 Steering을 Movement Component에 적용한다. 런타임 Setter도 EngineTorque·Drag·Downforce·DifferentialSplit에만 사용하며, 프로젝트 소스에는 `SetMassOverrideInKg`, `SetMassScale`, `GetMass`, `ChassisMass` 또는 피팅 총중량 적용 코드가 없다.

Runtime Mass Gate 기준 질량 경로는 다음으로 확정한다.

```text
FittingSnapshot.TotalVehicleMassKg = Design SSOT
→ UChaosVehicleMovementComponent::Mass = Chaos 설정 Mirror
→ PreRegisterAllComponents의 Super 호출 전 1회 기록
→ Chaos 초기 Physics State·Vehicle Simulation 생성
→ BeginPlay에서 VehicleMesh.GetMass() = Runtime Evidence 검증
```

첫 Source 구현의 공식 Editor Build에서 UE 5.8 public `Mass` 접근을 컴파일 증명한다. 초기 출격에서는 Physics State 재생성이 필요 없으며, 물리 생성 뒤 다른 질량 적용은 P0에서 거부한다. `VehicleMesh.BodyInstance.SetMassOverrideInKg` 단독 호출은 Movement 시뮬레이션과 물리 바디의 이중 SSOT 위험 때문에 계속 금지한다.

PhysicsAsset의 Body별 Density·MassScale은 기존 자산 기준값으로 유지하며 피팅 적용 때 수정·저장하지 않는다. `CenterOfMassOverride`도 위치 보정 전용으로 유지하고 총중량 대용으로 사용하지 않는다.

---

## 3. P0 목표

P0의 목표는 완성형 차고나 경제 시스템이 아니다.
다음 질문에 일관된 답을 제공하는 최소 출격 피팅 기반이다.

```text
- 이 차량 플랫폼의 어떤 MountProfile에 어떤 EquipmentPreset을 선택했는가?
- 선택한 장비가 하드포인트 타입과 크기 제한에 맞는가?
- 어떤 VehicleDefenseData를 사용할 것인가?
- 어떤 탄종을 몇 발 출격 적재할 것인가?
- 각 선택의 질량은 얼마인가?
- 총중량과 기준 차량 대비 질량 변화는 얼마인가?
- 이 피팅을 적용해도 되는가? 안 된다면 정확한 이유는 무엇인가?
- 적용된 피팅이 차량 가속·제동·선회에 어떤 변화를 주는가?
```

---

## 4. P0 포함 범위

### 4.1 별도 피팅 프리셋

기존 `VehicleData`를 직접 편집하지 않는 별도 `UCFVehicleFittingData` 후보를 둔다.

최소 선택:

```text
- 기준 VehicleData
- MountProfile별 EquipmentPresetData 선택
- VehicleDefenseData 선택 또는 None
- AmmoData별 InitialSortieAmmoCount
- FittingId와 표시 이름
```

### 4.2 기존 하드포인트·마운트 규칙 재사용

P0에서 새로운 하드포인트 체계를 만들지 않는다.

```text
HardpointSlots
+ MountProfiles
+ EquipmentPresetData.CanUseOnMount
+ WeaponData.CanUseOnMount
```

위 현재 계약을 피팅 검증의 기준으로 사용한다.

첫 통합 테스트 차량은 현재 검증된 `Top_01` 계열 한 개 MountProfile만 사용해도 된다.
데이터 구조는 여러 MountProfile을 처리할 수 있어야 하지만, 첫 사용자 PIE에서 네 종류 무기나 다중 슬롯을 동시에 요구하지 않는다.

### 4.3 방어 패키지 선택

P0 방어 선택은 방향별 장갑 수치를 피팅 UI에서 직접 조립하지 않는다.
완성된 `VehicleDefenseData` 하나를 선택한다.

```text
None
또는
VehicleDefenseData 1개
```

Shield·Armor·Penetration·Integrity 계산은 `CF-FQ-033`의 책임을 유지한다.

### 4.4 탄약 적재 선택과 질량

`CF-FQ-031`의 AmmoData 계약을 사용해 탄종별 출격 적재량과 질량을 계산한다.

```text
AmmoMassKg
= Σ(InitialSortieAmmoCount × UnitMassKg)
```

장전량과 예비량을 중복 합산하지 않는다.
탄약 런타임이 준비되지 않은 단계에서는 피팅 정적 선택과 질량 계산만 구현할 수 있지만, 실제 발사 소비 상태를 피팅 컴포넌트가 별도로 소유해서는 안 된다.

### 4.5 질량 스냅샷

P0 스냅샷은 최소 다음 값을 제공한다.

```text
BaseVehicleMassKg
TurretMountMassKg
WeaponMassKg
AmmoMassKg
DefenseMassKg
EquipmentMassKg
TotalVehicleMassKg
PayloadMassKg
PayloadUsageRatio
ValidationState
ValidationIssues
```

P0 총중량 공식:

```text
TotalVehicleMassKg
= BaseVehicleMassKg
+ Σ(TurretMountMassKg)
+ Σ(WeaponMassKg)
+ AmmoMassKg
+ DefenseMassKg
```

연료·배터리·유틸리티 질량은 P0 공식에 임의의 가상값으로 넣지 않는다.
해당 시스템이 구현되기 전에는 0 또는 명시적 미지원 상태로 남긴다.

### 4.6 출격 초기화 적용

검증을 통과한 피팅은 차량 초기화 전에 한 번 해석한다.

권장 순서:

```text
VehicleData 확인
→ FittingData 검증
→ FittingSnapshot 생성
→ 장비·방어·탄약 초기 구성 해석
→ 총중량·기동 보정 계산
→ VehicleRuntime 초기화
→ Weapon·Defense·Ammo Runtime 초기화
```

P0에서는 전투 중 피팅 교환을 허용하지 않는다.

### 4.7 기동성 영향

P0에서 질량이 실제 체감에 영향을 줘야 한다.
최소 영향 대상:

```text
- 가속
- 제동
- 선회 반응
```

최고속도, 충돌 피해, 반동 안정성, 연료 소비는 후속 범위로 남겨도 된다.
정확한 보정 공식과 Chaos 질량 적용 방식은 `FIT-P0-01` 미결정 사항에서 확정한다.

### 4.8 Preview와 Debug

피팅 P0는 완성형 차고 UI보다 먼저 C++ ViewData와 Debug를 제공한다.

```text
- 선택 장비 목록
- 슬롯·하드포인트 호환 상태
- 질량 항목별 Breakdown
- 총중량과 탑재율
- 예상 가속·제동·선회 변화
- 적용 가능 여부
- 실패 사유 목록
```

화면 배치·스타일·레이어는 `CF-FQ-032` UI 프레임워크와 조율하고, 피팅 기능은 판정과 ViewData를 소유한다.

---

## 5. P0 제외 범위

```text
- 게임 내 차고 전체 흐름
- 장비 구매·판매·가격·재화
- 해금·연구·레벨 제한
- SaveGame과 재접속 영구 저장
- 연료 탱크 선택과 실제 연료 소비
- 배터리·발전기·전력망
- 연막·플레어·냉각·수리 등 Utility Runtime
- 방향별 장갑판을 개별 아이템으로 조립
- 전투 상태 또는 차량 이동 중 즉시 장비 Hot Swap
- 장착된 슬롯에 새 장비를 직접 덮어쓰는 교환
- 탄약 발사마다 실시간 Chaos 질량 갱신
- 장착 위치별 중심질량 이동
- 반동 안정성·충돌 피해·연료 소비 공식
- AI 자동 피팅
- 네트워크 권한·복제
```

이 항목을 P0 데이터 구조에 미리 완성형으로 넣지 않는다.

---

## 6. 확정 원칙

```text
1. VehicleData는 차량 플랫폼 원본이다.
2. 피팅 선택은 별도 데이터와 런타임 스냅샷으로 분리한다.
3. MountProfile과 HardpointSlot을 피팅에서 다시 정의하지 않는다.
4. EquipmentPresetData가 TurretMountData와 WeaponData의 단일 장비 조합 소스다.
5. Ammo 수량·재장전 Runtime은 CF-FQ-031이 소유한다.
6. Shield·Armor·Integrity 계산은 CF-FQ-033이 소유한다.
7. 피팅은 각 시스템의 질량 기여 값을 합산하고 검증한다.
8. 검증 실패 시 부분 적용하지 않고 현재 Applied Snapshot을 보존한다.
9. 필드 장착·해제는 비전투, 관련 쿨타임 종료, 차량 정지와 Inventory 예약 성공 조건에서만 시작한다.
10. 장착은 호환되는 빈 슬롯에만 가능하며 교환은 Unequip과 Equip 두 액션으로 수행한다.
11. 시간 액션 완료 전에는 Inventory와 Applied Snapshot을 변경하지 않고 취소 시 예약만 해제한다.
12. 기존 None·MissingOptional 호환 경로를 임의로 제거하지 않는다.
13. C++은 검증·질량·스냅샷·예약 조율·적용을, Blueprint는 화면·진행 표시·연출을 담당한다.
14. 현재 단일 Active CF-FQ-032와 기존 dirty 소스·에셋을 보호한다.
```

---

## 7. FIT-P0-01 Decision Lock

`FIT-D-001~014`는 아래처럼 잠근다. FIT-P0-05 Runtime Mass Gate 완료로 현재 결정 상태는 모두 `Approved`이며, UE 5.8 public 심볼 컴파일은 설계를 다시 여는 결정 Gate가 아니라 구현 검증이다.

| ID | 결정안 | 주요 대안 | 영향 | 결정 상태 |
|---|---|---|---|---|
| `FIT-D-001` | `VehicleData.BaseVehicleMassKg`를 피팅 계산 SSOT로 추가한다. PhysicsAsset 계산 질량은 진단·검증값으로만 사용한다. | PhysicsAsset 자동 계산값을 SSOT로 사용 | Pawn 없는 Preview와 결정론적 Snapshot이 가능해지고 자산 변경 시 불일치를 명시적으로 검출한다. | Approved |
| `FIT-D-002` | 무기 본체 질량은 `WeaponData.WeaponMassKg`가 소유한다. | EquipmentPreset 또는 MountProfile에 포함 | 같은 무기의 Mount 재사용과 Mount·Weapon Breakdown을 보존한다. | Approved |
| `FIT-D-003` | P0 방어 질량은 `VehicleDefenseData.DefenseMassKg` 단일 패키지 값으로 둔다. | Shield·6방향 Armor 질량을 각각 분리 | 현재 완성형 DefenseData 선택 구조와 일치하며 세부 장갑 아이템화는 P1로 미룬다. | Approved |
| `FIT-D-004` | EquipmentPreset 자체 추가 질량은 두지 않고 `TurretMountWeightKg + WeaponMassKg`로 계산한다. | Preset Overhead 질량 추가 | 조합 DataAsset이 중복 질량 SSOT가 되는 것을 막는다. | Approved |
| `FIT-D-005` | 피팅 저장 단위는 별도 `UCFVehicleFittingData` PrimaryDataAsset으로 한다. | VehicleData 직접 수정, SaveGame 단독 저장 | 플랫폼 원본과 출격 선택을 분리하고 향후 SaveGame은 선택 자산 ID만 참조할 수 있다. | Approved |
| `FIT-D-006` | `UCFVehicleFittingComp`가 검증·Resolved Snapshot·Applied Snapshot을 소유하고 Pawn은 초기화 순서만 조정한다. | VehiclePawn 또는 WeaponComp가 직접 소유 | Weapon·Ammo·Defense 책임을 침범하지 않고 원자 적용·Rollback 지점을 만든다. | Approved |
| `FIT-D-007` | `Snapshot.TotalVehicleMassKg`를 `UChaosVehicleMovementComponent::Mass` 설정값으로 물리 컴포넌트 등록 전에 1회 적용한다. Chaos Vehicle의 초기 물리 생성이 VehicleMesh Body 질량으로 전파하게 하며 프로젝트가 `SetMassOverrideInKg`를 직접 호출하지 않는다. | PhysicsAsset 수정, VehicleMesh BodyInstance 단독 Override, BeginPlay 이후 Hot Apply | Design SSOT·Movement 설정값·VehicleMesh 실측값을 분리하면서 초기 출격에서는 Physics State 재생성을 피한다. UE 5.8 첫 구현 빌드에서 public 접근 심볼을 컴파일 증명한다. | Approved |
| `FIT-D-008` | 실제 물리 질량을 1차 효과로 사용한다. 추가 Mobility Adapter는 기본 1.0·비활성으로 두고 M5 측정에서 부족한 축만 데이터 기반 `Pow(1/MassRatio, Exponent)`·Clamp로 보정한다. | 처음부터 Torque·Brake·Steering을 모두 비율 보정 | 질량 효과의 이중 적용을 피하고 Default 피팅 회귀를 보존한다. | Approved |
| `FIT-D-009` | P0 질량은 출격 초기 고정한다. 발사마다 갱신하지 않는다. | 매 발, 임계값, 재장전 시 갱신 | Chaos 재설정과 Ammo Runtime 결합을 피한다. 중량탄 배치 갱신은 P1 후속이다. | Approved |
| `FIT-D-010` | `VehicleData.MaximumGrossMassKg`를 유일한 한도 SSOT로 둔다. Payload 한도는 `MaximumGrossMassKg - BaseVehicleMassKg`로 파생하고 초과는 Error다. | MaximumPayload만 저장, 과적 허용 | UI와 Runtime의 한도 계산이 일치하며 P0 과적 밸런스가 불필요해진다. | Approved |
| `FIT-D-011` | `MissingMountSelectionPolicy`를 명시하고 기본값은 `UseVehicleDefault`로 한다. 빈 장착은 `bEnabled=false` 등 명시 선택으로 구분한다. | 누락을 Empty 또는 Error로 처리 | 기존 VehicleData 기본 장비 호환과 부분 프리셋을 유지하면서 실제 해석 결과를 Snapshot에 남긴다. | Approved |
| `FIT-D-012` | Defense 선택은 `UseVehicleDefault / ExplicitNone / Override` 3상태로 둔다. `ExplicitNone`을 허용한다. | Null 하나로 Default와 None을 함께 표현 | Legacy·경량 빌드를 지원하고 Null 의미의 모호성을 제거한다. | Approved |
| `FIT-D-013` | 첫 테스트 차량은 읽기 전용 `DA_TestSUV`를 원본으로 한 미래 전용 복제 `DA_VehicleFitting_TestSUV`를 사용한다. BP 기본값·TestMap·원본 DataAsset은 수정하지 않는다. | DA_TestSUV 직접 수정, DA_TestSedan 사용 | CF-FQ-029 Launcher 구성을 보존하면서 Light·Default·Heavy 피팅을 동일 플랫폼에서 비교한다. | Approved |
| `FIT-D-014` | 피팅 기능은 Validation·Snapshot·ViewData를 소유하고 `CF-FQ-032`는 LocalPlayer UI Root·화면 전환·표시를 소유한다. | FittingComp가 Widget 수명까지 관리 | 판정과 표현을 분리하고 Debug Panel 기반 선행 검증을 허용한다. | Approved |

### 7.1 Gate 책임 재분리

기존 문서는 정확한 Engine Mass API와 PhysicsAsset 기준값을 `FIT-P0-02` 전체의 선행 Gate로 묶었다. 실제 소스 범위를 다시 분해한 결과 `FIT-P0-02`는 공용 타입, DataAsset과 정적 질량 필드만 추가하며 Chaos Vehicle API, Physics State, Vehicle Simulation 또는 VehicleMesh 물리 상태를 호출하지 않는다.

따라서 Gate를 다음처럼 분리한다.

```text
FIT-P0-02 Data Contract Gate
- 기존 런타임 기본값 호환
- 데이터 소유권과 직렬화 계약
- UHT·Editor Build
- DataContract Automation

FIT-P0-05 Runtime Mass Gate
- UChaosWheeledVehicleMovementComponent Mass 필드·Setter 정확한 심볼
- Mass 적용 시점
- Physics State 또는 Vehicle Simulation 재생성 필요 여부
- BP_CFVehiclePawn.VehicleMesh 연결 PhysicsAsset과 현재 계산 질량
- 적용 후 Movement 질량과 VehicleMesh 실제 질량 허용 오차
```

`FIT-P0-02`를 Engine API 증거 부족으로 차단한 과거 판정은 Gate 범위를 과도하게 앞당긴 것으로 정정한다. 런타임 질량 증거가 없다는 사실은 유지하지만 데이터 계약 구현을 차단하지 않는다.

### 7.2 FIT-P0-02 Data Contract Gate 결과 — PASS

2026-08-01 다음 데이터 계약을 구현하고 검증했다.

```text
- CFFittingTypes.h
- CFVehicleFittingData.h / .cpp
- CFFittingContractTests.cpp
- VehicleData.BaseVehicleMassKg
- VehicleData.MaximumGrossMassKg
- WeaponData.WeaponMassKg + 안전 Getter
- VehicleDefenseData.DefenseMassKg + 안전 Getter·DataValidation
- Defense 선택 3상태
- 누락 Mount 선택 정책
- 검증 상태·심각도·문제 코드
- 결정론적 FittingSnapshot 공용 구조
```

검증 결과:

```text
Official Editor Build: PASS
CarFight.Fitting.FIT_P0_02.DataContract: Success
Automation Warnings: 0
Automation Errors: 0
전체 CarFight 필수 회귀: 15/15 Success
Blueprint·DataAsset·Map 변경: 없음
VehiclePawn·Chaos 질량 적용 변경: 없음
```

### 7.3 FIT-P0-05 Runtime Mass Gate — Locked

2026-08-02 읽기 전용 조사로 다음 사실을 확인했다.

```text
Runtime Engine: Unreal Engine 5.8.0
물리 Root: AWheeledVehiclePawn 상속 VehicleMesh / USkeletalMeshComponent
Movement: VehicleMovementComp / UChaosWheeledVehicleMovementComponent 계열
VehicleMesh SkeletalMesh: /Game/Vehicles/SportsCar/SKM_SportsCar
같은 자산 폴더의 PhysicsAsset: /Game/Vehicles/SportsCar/PA_SportsCar
SM_Body: QueryOnly 시각 피격 표면 / 질량 적용 대상 아님
현재 ApplyVehicleMovementConfig: ChassisHeight·Drag·Downforce·COM·Engine·Diff·Steering만 적용
현재 Fitting Prepare: Super::BeginPlay 이후 InitializeVehicleRuntime 안에서 실행
프로젝트 소스의 Mass API 사용: 없음
```

AssetDump 증거:

```text
BP read-only dump job 8273dbea503d4d98b4247508f6d018e6
- 3/3 Success / Error 0
- VehicleMesh와 VehicleMovementComp 실제 컴포넌트 참조 확인

SportsCar read-only dump job a63d09b80c014aeb9b7e83f026e6f79a
- 24/24 Success / Error 0
- SKM_SportsCar와 PA_SportsCar 자산 존재 확인
- SkeletalMesh·PhysicsAsset Details는 현재 AssetDump 지원 범위 밖이므로 직접 연결과 계산 질량 수치는 미확인
```

Gate 결정:

```text
Design Mass SSOT
= FCFVehicleFittingSnapshot.TotalVehicleMassKg

Chaos Configuration Mirror
= UChaosVehicleMovementComponent::Mass

Runtime Evidence
= VehicleMesh.GetMass() + VehicleMesh.GetPhysicsAsset() 경로

초기 적용 시점
= ACFVehiclePawn::PreRegisterAllComponents에서 Super 호출 전
= Snapshot의 질량 부분만 순수 Prepare하고 Movement Mass에 1회 기록

BeginPlay Commit
= Physics State 생성 뒤 VehicleMesh 실제 질량 검증
= 검증 성공 뒤에만 같은 Cached Snapshot의 Weapon·Defense Commit 허용

SetMassOverrideInKg
= 일반 Body 저수준 경로로 취급
= 프로젝트 초기 출격 코드에서는 직접 호출 금지
= Movement Mass와 분리된 Body-only Override 금지

Physics State·Vehicle Simulation 재생성
= 물리 등록 전 초기 출격 적용에는 불필요
= Physics State 생성 뒤 다른 질량으로 변경할 때는 필요 가능성이 높으므로 FIT-P0-05 P0에서 변경 요청 자체를 거부
= 실제 필드 재적용과 재생성 계약은 FFIT-P0-04로 분리
```

실제 질량 비교 기준:

```text
TargetMassKg = Cached Snapshot.TotalVehicleMassKg
ConfiguredMassKg = VehicleMovementComponent.Mass
ActualMassKg = VehicleMesh.GetMass()
ToleranceKg = Max(1.0kg, TargetMassKg × 0.01)

Configured와 Actual이 Target 허용 오차 안
+ VehicleMesh Physics State 생성
+ VehicleMesh 물리 시뮬레이션 활성
+ PhysicsAsset 유효
= Initial Runtime Mass Commit 가능
```

실패 정책:

```text
FittingData 없음
→ Movement Mass 무수정 / 기존 Legacy 질량 유지

Snapshot Invalid 또는 TargetMass 비정상
→ 물리 등록 전 무수정 / Weapon·Defense도 Legacy 경로

PreRegister 적용 실패
→ 캐시한 기존 Movement Mass를 Super 호출 전에 복원 / Legacy 경로

Post-Physics 실제 질량 불일치
→ Weapon·Defense Snapshot Commit 금지
→ VehicleRuntime Ready 실패
→ 안전하지 않은 자동 Recreate·Body Override로 Legacy를 가장하지 않음
→ 새 Pawn 생성 또는 후속 명시적 재생성 경로가 필요
```

이 Gate는 초기 출격 1회 적용 구조를 잠갔다. UE 5.8 public `Mass` 접근은 첫 Source 구현의 공식 Editor Build로 컴파일 증명해야 하지만, 이는 적용 계층을 다시 선택하는 설계 미결정이 아니다.

---

## 8. 소스 현황과 후속 후보

파일과 클래스 이름은 모두 32자 이하로 유지한다.

구현 완료:

```text
UE/Source/CarFight_Re/Public/CFFittingTypes.h
UE/Source/CarFight_Re/Public/CFVehicleFittingData.h
UE/Source/CarFight_Re/Private/CFVehicleFittingData.cpp
UE/Source/CarFight_Re/Public/CFVehicleFittingComp.h
UE/Source/CarFight_Re/Private/CFVehicleFittingComp.cpp
UE/Source/CarFight_Re/Private/CFFittingContractTests.cpp
UE/Source/CarFight_Re/Private/CFFittingSnapshotTests.cpp
UE/Source/CarFight_Re/Private/CFFittingRuntimeTests.cpp
```

FIT-P0-04 통합 수정:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
Tools/RunCombatRuntimeTests.ps1
```

수정 후보는 결정 잠금 후 확정한다.

```text
CFVehicleData.h / .cpp
CFWeaponData.h / .cpp
CFTurretMountData.h / .cpp
CFVehicleDefenseData.h / .cpp
CFVehiclePawn.h / .cpp
CFVehicleWeaponComp.h / .cpp
CFVehicleAmmoComp.h / .cpp — CF-FQ-031 구현 이후
```

---

## 9. 작업 패키지

| Task ID | 작업명 | 주 책임 | 선행 | 상태 |
|---|---|---|---|---|
| `FIT-P0-00` | 실제 구현 조사와 문서 준비 | 분석·문서 | 없음 | Done |
| `FIT-P0-01` | 질량·소유권·적용 정책 결정 잠금 | 설계 | P0-00 | Done |
| `FIT-P0-02` | Fitting Data Contract Foundation | C++ Data | P0-01 | Done — Build·DataContract PASS |
| `FIT-P0-03` | Compatibility Validation and Snapshot | C++ Runtime | P0-02 | Done — Build·Automation PASS |
| `FFIT-P0-00` | Field Fitting Action Contract | Design | FIT-P0-03 | Done — Documentation |
| `FIT-P0-04` | Sortie Fitting Apply Runtime | C++ Component | P0-03 | Done — Build·Automation PASS |
| `FIT-P0-05` | Vehicle Mass and Mobility Adapter | C++ Vehicle | P0-03~04 | In Progress — Initial Sortie Adapter Build·Automation PASS / Physics PIE·Mobility Pending |
| `FIT-P0-06` | Fitting ViewData and Debug | C++/UI/BP | P0-03~05 | Technical Done — C++ ViewData·Blueprint Contract PASS / UI readability Pending |
| `FFIT-P0-01` | Permission and Blocker Query | C++ Runtime | FFIT-P0-00 | Done — Code·Build·Automation PASS |
| `FFIT-P0-02` | Timed Action and Reservation | C++ Runtime | INV-P0-03, FFIT-P0-01 | Done — Code·Build·Automation PASS |
| `FFIT-P0-03` | Atomic Equip and Unequip | C++ Runtime | FIT-P0-04, INV-P0-04, FFIT-P0-02 | Technical Done — TimedAction→Coordinator same Transaction handoff PASS / mass-changing Runtime remains P0-04 |
| `FFIT-P0-04` | Field Runtime Mass Reapply | C++ Vehicle | FIT-P0-05, FFIT-P0-03 | Technical Done — Chaos Movement Mass Reapply·Physics State recreation·RecoveryFailed·saved-map PIE PASS |
| `FFIT-P0-05` | Field Fitting UI and PIE | C++/UI/BP | FIT-P0-06, FFIT-P0-04, UI Framework | Not Started / Blocked by FIT-P0-06 + USER UI availability |
| `FIT-P0-07` | Test DataAssets and Protected PIE Setup | Data/Editor | P0-02~06, FFIT-P0-05 | Not Started |
| `FIT-P0-08` | Integrated Verification and Tuning | QA/Data | P0-04~07, FFIT-P0-05 | Not Started |

---

## 10. FIT-P0-00 — Done

완료 범위:

```text
- CombatPlan/13_Fitting.md 기존 방향 조사
- VehicleData·VehicleRuntime 실제 소스 조사
- HardpointSlots·MountProfiles 조사
- EquipmentPresetData·TurretMountData·WeaponData 연결 조사
- WeaponData 탄약 후보 필드와 CF-FQ-031 계획 조사
- VehicleDefenseData·VehicleDefenseComp와 CF-FQ-033 상태 조사
- 명시 질량 필드와 Chaos 질량 적용 경로 검색
- P0 포함·제외 범위 분리
- 미결정 사항 목록 작성
- 별도 Feature ID, Plan, Design, Roadmap 생성
```

미수행:

```text
- C++ 수정
- Blueprint 수정
- .uasset 수정·재저장
- 빌드
- Automation
- PIE
```

---

## 11. FIT-P0-01 — Decision Lock

### 상태

```text
Done / 2026-08-01
```

### 조사 증거

```text
- 물리 RootComponent는 상속된 VehicleMesh USkeletalMeshComponent다.
- 기존 성공 덤프의 VehicleMesh.SkeletalMesh는 /Game/Vehicles/SportsCar/SKM_SportsCar다.
- SM_Body는 QueryOnly 시각 피격 표면으로 질량 소유자가 아니다.
- VehicleDriveComp는 UChaosWheeledVehicleMovementComponent를 캐시한다.
- ApplyVehicleMovementConfig에는 질량 적용이 없고 CenterOfMassOverride는 위치 보정뿐이다.
- 프로젝트 소스에 SetMassOverrideInKg, SetMassScale, GetMass와 ChassisMass 사용이 없다.
- 승인된 신규 자산 조회가 자산 로드 전에 실패해 PhysicsAsset·계산 질량 수치는 미확인이다.
```

### 완료 결정

```text
- FIT-D-001~014 모두 Approved
- VehicleData 명시 BaseVehicleMassKg와 MaximumGrossMassKg를 피팅 SSOT로 사용
- WeaponData와 VehicleDefenseData가 각 질량을 소유
- EquipmentPreset 추가 오버헤드 없음
- VehicleFittingData + VehicleFittingComp + 결정론적 Snapshot 구조
- Chaos VehicleMovement 공식 Mass 경로에 초기화 전 1회 적용
- BodyInstance 단독 질량 Override와 PhysicsAsset 수정 금지
- 실제 질량을 우선하고 Mobility Adapter는 기본 비활성
- 출격 초기 고정 탄약 질량
- 누락 Mount는 Vehicle 기본값, 빈 장착은 명시 선택
- Defense는 Default / ExplicitNone / Override 3상태
- DA_TestSUV 원본을 보호한 전용 피팅 테스트 복제 사용
- ViewData와 UI Framework 책임 분리
```

### 종료 판정

```text
- 중복 질량 SSOT 없음: PASS
- 첫 테스트 플랫폼과 보호 방식 확정: PASS
- FIT-P0-02 필드·타입 방향 확정: PASS
- 초기 출격 Mass SSOT·적용 수명·검증 기준: Runtime Mass Gate Locked
- PhysicsAsset 직접 연결·계산 질량: Runtime GetPhysicsAsset·GetMass 증거로 확인
- Source·Blueprint·DataAsset·Map 변경: 없음
```

다음 단계는 `FIT-P0-03 Compatibility Validation and Snapshot`이다.

---

## 12. FIT-P0-02 — Data Contract Foundation

### 상태

```text
Done / Source Applied / Build·Automation PASS / 2026-08-01
```

### 구현 결과

```text
- UCFVehicleFittingData PrimaryDataAsset
- FCFVehicleMountSelection
- FCFVehicleDefenseSelection
- FCFVehicleFittingSnapshot
- FCFFittingValidationIssue
- ECFFittingValidationState / Severity / IssueCode
- ECFMissingMountPolicy
- ECFDefenseSelectionMode
- VehicleData BaseVehicleMassKg / MaximumGrossMassKg
- WeaponData WeaponMassKg / GetEffectiveWeaponMassKg
- VehicleDefenseData DefenseMassKg / GetEffectiveDefenseMassKg
- DataValidation과 BuildVehicleFittingSummary
```

Ammo 선택 타입은 `CF-FQ-031` 계약을 중복 생성하지 않기 위해 이번 단계에서 추가하지 않았다. Snapshot의 `AmmoMassKg` 슬롯만 0 기본값으로 보존한다.

### 호환 결과

```text
- 신규 질량 필드 기본값 0kg: 기존 차량 주행·발사·방어 변화 없음
- FittingData 미지정: 기존 VehicleData 기본 장비·방어 경로 유지
- MountProfile.DefaultEquipmentPresetData 유지
- DefenseData None Legacy Fallback 유지
- VehiclePawn·Blueprint·DataAsset·Map 변경 없음
```

### 검증 결과

```text
Official Editor Build: PASS
CarFight.Fitting.FIT_P0_02.DataContract: Success / Warnings 0 / Errors 0
CarFight 필수 전투 회귀: 15/15 Success
PIE: Not Applicable — 데이터 계약 단계
```

---

## 13. FIT-P0-03 — Validation and Snapshot

### 상태

```text
Done / Source Applied / Official Editor Build PASS / Automation PASS / 2026-08-01
```

### 구현 결과

```text
UCFVehicleFittingData.BuildFittingSnapshot()
- Pawn·VehicleMovement 없는 const 순수 해석
- VehicleData.MountProfiles 순서로 ResolvedMounts 생성
- MountSelections 배열 순서와 무관한 결정론적 결과
- Duplicate·Unknown·Missing MountSelection 구조화 검증
- LocationSlotRef → HardpointSlots 해석
- EquipmentPreset 완성 여부와 MountType·WeaponSize 검증
- WeaponData MountType·Size 호환 검증
- UseVehicleDefault / TreatAsEmpty / TreatAsError 누락 정책
- ExplicitEmpty와 MissingPolicyEmpty 선택 소스 구분
- UseVehicleDefault / ExplicitNone / Override 방어 3상태 보존
- Base·TurretMount·Weapon·Defense 질량 합산
- Payload·Total·PayloadUsageRatio·GrossMassUsageRatio 계산
- MissingMassSource·InvalidMassValue·GrossMassExceeded 검증
- Error 존재 시 ValidationState=Invalid
```

Ammo 선택과 질량은 `CF-FQ-031` 계약 전까지 중복 구현하지 않고 `AmmoMassKg=0`으로 유지한다.

### Automation 결과

```text
CarFight.Fitting.FIT_P0_03.Compatibility: Success / Warnings 0 / Errors 0
CarFight.Fitting.FIT_P0_03.MassSnapshot: Success / Warnings 0 / Errors 0
전체 CarFight Automation: 27 Success / Failed 0
필수 회귀 목록: 17/17 Success
PIE: Not Applicable — Pawn 없는 순수 계산 단계
```

### 공식 빌드 상태

```text
- UHT와 신규 피팅 소스 컴파일: PASS
- UnrealEditor-CarFight_Re.dll 링크: PASS
- 후속 CF-FQ-032 공식 Editor Build 4f2daa07187a41aea7c400386010b2a0: Exit Code 0
- 최신 Combat Runtime Automation 6f0a9a5ebde5472b8b784a9103ce484d: 32/32 Success
- FIT-P0-02 DataContract, FIT-P0-03 Compatibility, MassSnapshot: Warnings 0 / Errors 0
```

이 후속 공식 빌드가 현재 피팅 소스를 포함해 통과했으므로 이전 NetCore DLL 파일 잠금은 해소된 과거 차단으로 기록하고 `FIT-P0-03`을 Done으로 전환한다.

### 실패 정책

```text
- Error가 하나라도 있으면 Snapshot 적용 불가
- 비호환 선택도 참조와 유효 질량 Breakdown은 보존하지만 ValidationState는 Invalid
- 명시적 빈 장착과 누락 정책 빈 장착은 정상 선택으로 구분
- VehicleData 기본 방어 None과 ExplicitNone은 Legacy 정상 구성
- 선택된 실제 장비·방어의 0kg 질량은 MissingMassSource Error
- 실패해도 VehicleData, EquipmentPresetData와 하위 DataAsset을 수정하지 않음
```

---

## 13A. FFIT-P0-00 — Field Fitting Action Contract

### 상태

```text
Done — Documentation / Source·Asset Not Modified / 2026-08-01
```

### 허용 조건

필드 피팅 화면은 World Pause를 사용하지 않는다. 실제 Equip·Unequip 액션은 다음 조건을 모두 만족할 때만 시작한다.

```text
- 비전투 상태
- 모든 관련 무기·런처·재장전·방어·Utility 쿨타임 종료
- 진행 중 발사 시퀀스·재장전·수리·다른 시간 액션 없음
- 차량이 데이터 기반 정지 속도 한도 이하
- 정지 상태를 요구 시간 동안 유지
- VehicleRuntime Ready
- 접근 가능한 Inventory Container 존재
- Item·Destination Capacity·Mount Slot 예약 성공
```

쿨타임과 행동 잠금은 각 시스템이 공통 Blocker Query 계약으로 제공한다. Fitting UI가 WeaponComp·LauncherComp 내부 변수를 직접 조회하거나 쿨타임을 재계산하지 않는다.

### Equip 규칙

```text
- 대상 MountProfile은 Empty여야 함
- EquipmentPreset의 MountType·WeaponSize와 WeaponData 호환 PASS
- 실제 ItemInstance가 접근 가능한 Inventory에 존재
- 같은 ItemInstance가 다른 Slot에 장착·예약되지 않음
- 장착된 슬롯에 새 장비를 직접 덮어쓰지 않음
- 교환은 Unequip 완료 후 Equip을 별도 수행
```

### Unequip 규칙

```text
- 대상 MountProfile에 실제 ItemInstance가 장착되어 있어야 함
- 반환할 VehicleCargo Capacity 예약 성공
- 해제 완료 전까지 기존 장비와 Applied Snapshot 유지
```

### 시간 액션과 취소

```text
Start
→ Item·Slot·Capacity Reservation
→ RequiredDurationSeconds 동안 조건 감시
→ 완료 직전 최종 조건 재검증
→ Inventory Transaction과 Runtime Apply 원자 Commit
```

취소 사유 후보:

```text
VehicleMoved
CombatStarted
CooldownOrActionStarted
DamageReceived
VehicleDestroyed
RuntimeNotReady
SlotStateChanged
InventoryChanged
ReservationLost
ScreenClosed
UserCancelled
```

취소 시:

```text
- 실제 Inventory 이동 없음
- 실제 장비·방어·질량 변경 없음
- 기존 AppliedFittingSnapshot 유지
- 모든 Reservation 해제
- Action 상태와 취소 사유만 기록
```

화면은 취소 후 유지할 수 있으며 조건이 다시 충족되면 사용자가 새 액션을 시작한다.

### 의존 관계

```text
FFIT-P0-01 Permission Query
→ 현재 Combat·Drive·Weapon·Launcher·Ammo·Defense 상태 조회 계약

FFIT-P0-02 Timed Action
→ CF-FQ-035 INV-P0-03 Reservation·Atomic Transfer 필요

FFIT-P0-03 Atomic Equip / Unequip
→ FIT-P0-04 Applied Snapshot Runtime
→ CF-FQ-035 INV-P0-04 Fitting Adapter

FFIT-P0-04 Field Mass Reapply
→ FIT-P0-05 Runtime Mass Gate와 재초기화 계약

FFIT-P0-05 UI / PIE
→ FIT-P0-06 ViewData
→ CF-FQ-032 Screen·Panel·Modal·System Layer
```

---

## 14. FIT-P0-04 — Sortie Apply Runtime

### 상태

```text
Done / Source Applied / Official Editor Build PASS / Automation PASS / 2026-08-01
```

### 구현 결과

```text
UCFVehicleFittingComp
- VehicleFittingData 없음 → VehicleData Legacy 입력 Prepare
- 유효 VehicleFittingSnapshot → Weapon·Defense Runtime 입력 Prepare
- 무효 Snapshot·VehicleData 불일치 → Adapter 무호출·현재 Applied 상태 보존
- Weapon → Defense 순서 Commit
- 어느 단계 실패든 직전 Applied 입력 또는 Legacy 입력으로 전체 복원
- AppliedFittingSnapshot과 Legacy·Snapshot 적용 상태 소유
- 명시적 Prepared Rollback
- 재초기화 시 이전 Snapshot 중복 누적 없음
- EndPlay·Reset에서 Transient Prepared·Applied 상태 정리

Weapon 통합
- VehicleData 기본 EquipmentPresetData Legacy 경로 유지
- Snapshot EquipmentPresetData Override 해석 계층 추가
- Commit된 장비 프리셋을 터렛 Runtime 캐시와 시각 구성에 사용

Defense 통합
- VehicleData.DefaultDefenseData Legacy 경로 유지
- Snapshot UseVehicleDefault / ExplicitNone / Override 최종 DefenseData 입력 적용
```

Ammo, Runtime Mass, Inventory Adapter와 Field Equip·Unequip은 추가하지 않았다.

### 원자 적용 경계

```text
PrepareSortieFitting
→ Snapshot 검증과 입력 생성만 수행
→ Weapon·Defense 무변경

CommitPreparedSortieFitting
→ Weapon 적용
→ Defense 적용
→ 모두 성공한 경우에만 AppliedFittingSnapshot 교체

실패
→ 직전 Applied 입력 또는 Legacy 입력으로 Weapon·Defense 복원
→ 후보 Prepared 입력 폐기
→ 기존 AppliedFittingSnapshot 유지

RollbackPreparedSortieFitting
→ 하위 Runtime 무호출
→ Prepared 입력만 폐기
```

### Pawn 초기화 순서

```text
Health 초기화
→ Fitting Prepare
→ Weapon·Defense Commit
→ Commit된 Weapon 기준 터렛 시각 적용
→ Launcher를 최종 Weapon Runtime에 연결
→ Fitting Commit 성공을 VehicleRuntime Ready 조건에 포함
```

### Pawn 없는 Automation

```text
CarFight.Fitting.FIT_P0_04.RuntimeApply: Success
CarFight.Fitting.FIT_P0_04.AtomicBoundary: Success
Warnings: 0
Errors: 0
```

### 공식 검증

```text
첫 Build c24f71d6ab6944bbb0da7a1ae527672f
- 기존 Debug Summary의 지역 변수명 잔류 1건으로 Exit Code 6
- 신규 FittingComp·RuntimeTests·Pawn 통합은 컴파일됨

최종 Build 52b5b5572e2a4c739771dbd711a284d3
- CarFight_ReEditor Win64 Development
- Exit Code 0 / Result Succeeded

전체 Automation 370cf40d1e8041f9ac1bdca8121b3e32
- 전체 CarFight 40/40 Success
- 필수 회귀 19/19 Success
- Failed 0 / Missing 0
```

### 보호 결과

```text
- Inventory INV-P0-00~03 소스 무수정
- INV-P0-04 미착수
- Runtime Mass·Ammo 임시 타입 미구현
- UI·Blueprint·DataAsset·PhysicsAsset·Map·SaveGame 무수정
- CF-FQ-032 Active 유지
```

---

## 15. FIT-P0-05 — Mass and Mobility Adapter

### 상태

```text
Initial Sortie Adapter Code Complete / Official Build PASS / Automation PASS / 2026-08-02
Actual VehicleMesh Physics PIE: Pending
Light·Default·Heavy Mobility Measurement: Pending
```

### 목표

피팅 총중량을 실제 차량 물리와 최소 기동성 변화에 연결한다.

### 초기 출격 적용 계약

```text
1. PreRegisterAllComponents / Super 이전
2. VehicleData·VehicleFittingData에서 결정론적 Snapshot Prepare
3. Legacy Movement Mass 캐시
4. 유효 Snapshot의 TotalVehicleMassKg를 MovementComponent.Mass에 1회 기록
5. Super::PreRegisterAllComponents 실행
6. Chaos Vehicle이 Physics State와 Vehicle Simulation 생성
7. BeginPlay에서 Configured Mass·VehicleMesh.GetMass·PhysicsAsset 검증
8. 검증 성공 뒤 같은 Cached Snapshot으로 FIT-P0-04 Weapon·Defense Commit
9. AppliedFittingSnapshot과 Applied Runtime Mass 상태 동시 확정
```

Snapshot을 PreRegister와 BeginPlay에서 서로 따로 계산해 다른 결과를 사용할 수 없다. 실제 구현은 `UCFVehicleFittingComp`가 PreRegister에서 만든 전체 Snapshot과 Target Mass를 캐시하고, BeginPlay의 FIT-P0-04 Weapon·Defense Commit이 같은 Prepared 입력을 소비한다.

### 적용 후 재초기화 계약

```text
같은 Snapshot·같은 Target Mass
→ 검증만 재실행 / Mass 재작성·Physics 재생성 없음

Physics State 생성 뒤 다른 Target Mass
→ FIT-P0-05 P0에서 거부
→ RuntimeMassReapplyUnsupported 또는 동등한 구조화 실패

Respawn·새 Pawn
→ 새 수명에서 PreRegister 초기 적용 재실행

EndPlay
→ Prepared·Applied Mass 캐시만 정리
→ 파괴 중인 Physics State에 Mass 복원·Recreate 호출 금지
```

### SetMassOverrideInKg 판정

```text
- 초기 출격의 프로젝트 직접 적용 API로 채택하지 않음
- VehicleMesh Body만 바꾸면 Movement 설정값과 Simulation 입력이 분리될 수 있음
- PhysicsAsset·BodyInstance 저장 수정과 MassScale 우회도 금지
- 엔진 내부 Chaos 초기화가 Movement Mass를 Body에 전파하도록 둠
- 후속 Field Mass Reapply에서 필요성이 생기면 Movement와 Body를 함께 다루는 전용 Adapter·재생성 테스트로 별도 승인
```

### 구현 결과

```text
UCFVehicleFittingComp v1.1.0
- ECFInitialMassState
- PrepareInitialSortieFitting
- RecordInitialMassBeforePhysics
- VerifyInitialMassAfterPhysics
- Legacy fallback
- 같은 질량 Verify Only
- 다른 질량 ReapplyRejected
- EndPlay·Reset Mass 상태 정리

ACFVehiclePawn v2.133.0
- PreRegisterAllComponents / Super 이전 Snapshot Target 적용
- UChaosVehicleMovementComponent::Mass 직접 설정
- BeginPlay VehicleMesh.GetMass·GetPhysicsAsset·Physics State 검증
- Mass 검증 성공 뒤 Cached Snapshot Weapon·Defense Commit

금지 경로 미사용
- SetMassOverrideInKg 호출 없음
- Body-only Override 없음
- PhysicsAsset·MassScale 수정 없음
- RecreatePhysicsState·Simulation Reset 없음
```

### 공식 검증

```text
첫 공식 Build dce20226611944f9b323f94b573d4a74
- CarFight_ReEditor Win64 Development
- Exit Code 0
- UE 5.8 UChaosVehicleMovementComponent::Mass public 접근·PreRegister override 컴파일·링크 PASS

최종 공식 Build b7a4e52743c34fa2844ebe94925c6a0d
- Exit Code 0 / Result Succeeded

최종 Automation b45311c3d9784635ac01e1bdb1cb8b07
- CarFight.Fitting.FIT_P0_05.InitialMass: Success
- 전체 CarFight 41/41 Success
- 필수 회귀 20/20 Success
- Failed 0 / Missing 0
```

첫 Automation `7c68e7c632df430799bece286e77c235`는 구현 실패가 아니라 13.6kg 부동소수점 비교 단언의 과도한 정밀도로 새 테스트 1건만 실패했다. 비교 허용치를 0.001kg로 명시한 뒤 최종 빌드와 전체 회귀가 통과했다.

### 남은 최소 결과

```text
- 실제 VehicleMesh Physics State에서 Target·Configured·Actual 질량 일치 PIE
- 적용된 총중량 Debug 확인
- 경량·기준·중량 피팅의 가속 차이
- 제동거리 차이
- 조향 반응 차이
- 원본 기준 피팅의 주행 회귀
```

### 금지

```text
- Tick마다 VehicleMovement 전체 설정 재작성
- 질량과 무관한 엔진·휠 튜닝을 피팅 Task에서 재설계
- CenterOfMassOverride를 총중량 대용으로 사용
- 동일 효과를 Chaos 질량과 임의 Scalar에 중복 적용
```

---

## 16. FIT-P0-06 — ViewData and Debug

### 상태

```text
Technical Done — C++ ViewData·Blueprint Contract PASS
UI Widget 적용: Not Started
16:9·32:9 실제 가독성: USER Pending
```

### 구현 결과

```text
Source:
- UE/Source/CarFight_Re/Public/CFFittingViewData.h v1.0.0
- UE/Source/CarFight_Re/Private/CFFittingViewData.cpp v1.0.0

Automation:
- UE/Source/CarFight_Re/Private/CFFittingViewTests.cpp v1.0.0
- CarFight.Fitting.FIT_P0_06.BlueprintContract
```

ViewData는 `FCFVehicleFittingSnapshot`의 최종 결과를 다시 계산하지 않고 그대로 투영한다.

```text
- VehicleData / FittingId / ValidationState / ValidationIssues
- ResolvedMounts 순서와 최종 Equipment·Mount·Weapon 참조
- Resolved Defense 선택과 DefenseMassKg
- InitialSortieAmmoLoads 실제 출격 Count·AmmoData
- Base / Equipment / Ammo / Defense / Payload / Total / MaximumGross 질량 7행
- PayloadUsageRatio / GrossMassUsageRatio
- bCanApply = Snapshot.IsValid()
```

VehicleData에는 현재 정식 플레이어 표시 이름 필드가 없으므로 `VehicleDisplayName`은 P0 Debug에서 Asset 이름 fallback을 사용하고 `bVehicleDisplayNameUsesAssetName=true`로 명시한다.

Mobility Preview는 숨은 보정값을 만들지 않는다.

```text
PreviewMode = PhysicalMassOnly
MassRatio = TotalVehicleMassKg / BaseVehicleMassKg
AccelerationScale = 1.0
BrakingScale = 1.0
SteeringResponseScale = 1.0
bAdditionalAdapterApplied = false
```

축 Scale 1.0은 실제 질량 효과가 없다는 뜻이 아니라 추가 인위적 Mobility Adapter가 아직 비활성이라는 뜻이다. 실제 Chaos 질량은 FIT-P0-05·FFIT-P0-04 경로에서 계속 물리에 반영된다.

### 검증

```text
Official Build: `ad3452d3add74785bbdb41c667dce728` / Exit 0
FIT-P0-06 Targeted: `0c7ee328d7ef4f659d66d857fecdab2a` / 1/1 Success / 0 Fail
Targeted SHA-256: `fdc034d0715ece5147bf2fe60f2bd046bdc08ff3f7c8b89ff2bebd3585d802c2`
Fitting Regression: `e5140ca056444ea694abb4f73861c935` / 22/22 Success / 0 Fail
Inventory Regression: `c76221a75d9447dab1b03cc274d43a35` / 12/12 Success / 0 Fail
Full CarFight: `59aae9f0ac204e13a957dbbfe8cdbaec` / 84/84 Success / 0 Fail
Full SHA-256: `4e49080b401ada72e243e8bd1a98e58e9bdd9d3af4471a03b78f7d7ca277c139`
```

BlueprintContract는 핵심 Property의 `BlueprintVisible + BlueprintReadOnly`, Mount·Ammo·Issue 순서 보존, Snapshot 질량 값 그대로 복사, `InitialSortieAmmoCount`와 `MaximumLoadableAmmoCount` 분리, PhysicalMassOnly Mobility와 Invalid Snapshot Issue 표시를 검증했다.

### 남은 시각 Gate

실제 Widget/Screen이 아직 없으므로 `16:9·32:9 기본 가독성`, Field Fitting 진행률·취소 사유와 UI 조합은 `FFIT-P0-05 Field Fitting UI and PIE`에서 검증한다. 이 USER Gate를 Technical PASS로 대체하지 않는다.

---

## 17. FIT-P0-07 — Test Assets and PIE Setup

### 목표

원본 전투 에셋을 보호하는 별도 테스트 피팅 조합을 만든다.

권장 테스트 조합:

```text
Fitting_Default
- 현재 기본 장비·방어·탄약

Fitting_Light
- 가벼운 장비 또는 Defense None
- 낮은 탄약 적재

Fitting_Heavy
- 무거운 Mount·Weapon
- Defense 패키지
- 높은 탄약 적재
```

실제 이름과 자산은 FIT-P0-01에서 확정한다.

---

## 17.1 FIT-P0-07A — Light / Default / Heavy Fixture Readiness Audit

### 2026-08-17 판정

현재 자산을 이름으로 추정하지 않고 Asset class와 저장된 `UCFVehicleData` / 질량 계약으로 분류했다.

```text
Visual-only 차량 메시
- /Game/CarFight/Vehicles/Meshes/CityCar/CityCar
- /Game/CarFight/Vehicles/Meshes/Compact/Compact
- /Game/CarFight/Vehicles/Meshes/Coupe/Coupe
- /Game/CarFight/Vehicles/Meshes/Pickup/Pickup
- /Game/CarFight/Vehicles/Meshes/SubCompact/SubCompact
- /Game/CarFight/Vehicles/Meshes/Van/Van
- /Game/CarFight/Vehicles/Meshes/Wagon/Wagon
- 위 자산은 현재 `/Game/CarFight/Vehicles` AssetDump에서 StaticMesh로만 확인됐으며 대응 CFVehicleData는 발견되지 않았다.
- 외형 후보로는 보존하지만 Light/Default/Heavy Mobility Fixture의 차량 플랫폼으로 사용하지 않는다.

VehicleData baseline / Fitting-ready 아님
- DA_TestSedan: BaseVehicleMassKg=0 / MaximumGrossMassKg=0
- DA_TestSUV: BaseVehicleMassKg=0 / MaximumGrossMassKg=0
- 둘 다 실제 Sedan/SUV 메시·Wheel·MountProfile을 가진 현재 VehicleData baseline이지만 0kg 질량은 Fitting 계약에서 MissingMassSource 성격의 미설정 값이므로 공식 Fixture로 직접 사용하지 않는다.

Fitting-ready technical platform
- DA_VehicleDefense_TestSUV: BaseVehicleMassKg=1000 / MaximumGrossMassKg=2500
- 동일 SUV 시각 자산 사용
- DefaultDefenseData=DA_VehicleDefense_Test / DefenseMassKg=100
- 단일 RoofTurret_MediumOrLarge / Top_01 구조
- 기존 DA_Fit_DefenseTestSUV가 이 VehicleData를 참조한다.
- Preset_DefenseTest → DA_RocketBody 350kg + DA_Wpn_DefenseTest 120kg
- 기존 저장 피팅의 총중량은 1000 + 350 + 120 + 100 = 1570kg이며 기존 Chaos Runtime evidence와 일치한다.
```

### 공식 3단계 Fixture readiness

```text
Light — Ready to Prepare
- 같은 DA_VehicleDefense_TestSUV 사용
- RoofTurret_MediumOrLarge를 ExplicitEmpty
- DefenseSelection=ExplicitNone
- AmmoLoad 없음
- 새 질량값을 만들지 않고 기존 BaseVehicleMassKg만 사용
- 예상 Snapshot TotalVehicleMassKg=1000kg

Default — Ready / Existing Technical Baseline
- 기존 DA_Fit_DefenseTestSUV 계약 재사용
- Equipment 470kg + Defense 100kg
- 예상/기검증 Snapshot TotalVehicleMassKg=1570kg

Heavy — Not Yet Promoted
- 같은 DA_VehicleDefense_TestSUV 플랫폼에서 1570kg보다 크고 2500kg 이하인 실제 저장 Mass Source 조합이어야 한다.
- AmmoIntegration의 DA_Fit_HeavyFinite는 별도 DA_Veh_HeavyFinite를 사용하므로 그대로 Mobility Heavy로 승격하면 차량 플랫폼 차이가 섞인다.
- 기존 HeavyFinite payload를 같은 SUV 플랫폼에 재조합할 수 있는지는 저장된 Mount/Weapon/Ammo 질량과 Snapshot 호환을 별도 확인한 뒤 결정한다.
- 임의 ballast, 임의 WeaponMassKg, 임의 DefenseMassKg, 임의 Vehicle mass는 만들지 않는다.
```

판정:

```text
FIT-P0-07A Fixture Readiness Audit = Complete
차량 메시-only 구분 = 가능 / Visual-only로 격리
Light 준비 가능 = Yes
Default 준비 가능 = Yes
Heavy 준비 가능 = Pending same-platform persisted payload resolution
Content Asset mutation = 0
Source mutation = 0
Build / USER PIE = 0
```

다음 기술 Gate는 `FIT-P0-07B Heavy Payload Resolution → Official Light/Default/Heavy Fixture Preparation`이다. Heavy가 저장된 기존 질량 Source만으로 성립하지 않으면 임의 수치를 만들지 않고 정확한 데이터 blocker로 남긴다.

### Changelog v0.15.0

- 현재 AssetDump로 Vehicle 계층을 Visual-only / VehicleData baseline / Fitting-ready technical platform으로 분류했다.
- CityCar·Compact·Coupe·Pickup·SubCompact·Van·Wagon을 StaticMesh-only Visual 후보로 격리했다.
- DA_TestSedan·DA_TestSUV는 VehicleData이지만 Base/Gross 0kg라 공식 Fitting Fixture 대상에서 제외했다.
- DA_VehicleDefense_TestSUV의 1000/2500kg 계약과 기존 1570kg Defense fitting chain을 persisted asset에서 재확인했다.
- 같은 플랫폼의 Light=1000kg, Default=1570kg 후보는 임의 질량 없이 준비 가능하다고 판정했다.
- Heavy는 동일 플랫폼의 persisted payload가 1570kg 초과임을 확인하기 전 공식 Fixture로 승격하지 않는다.
- Source·Content Asset·Build·USER PIE 변경/실행은 0이다.

Migration: 새 세션은 `FIT-P0-07A`를 반복하지 않는다. `FIT-P0-07B Heavy Payload Resolution`에서 같은 DA_VehicleDefense_TestSUV 플랫폼을 유지하고 저장된 기존 질량 Source만 사용한다. 메시-only 차량은 VehicleData 제작 전까지 Mobility Fixture 후보에서 제외한다.

---

## 17.2 FIT-P0-07B — Heavy Payload Resolution and Official Fixture Preparation

### 2026-08-17 Technical Closure

Heavy 후보를 별도 차량 플랫폼 이름으로 승격하지 않고, 저장된 `HeavyFinite` payload의 실제 Mount·Weapon·Ammo 계약을 `DA_VehicleDefense_TestSUV`에 재조합해 동일 플랫폼 비교를 성립시켰다.

```text
공통 VehicleData
- /Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV
- BaseVehicleMassKg = 1000kg
- MaximumGrossMassKg = 2500kg
- DefaultDefenseData = DA_VehicleDefense_Test / DefenseMassKg = 100kg
- MountProfile = RoofTurret_MediumOrLarge

Heavy persisted payload
- DA_Mount_HeavyFinite / TurretMountWeightKg = 350kg
- DA_Wpn_HeavyFinite / WeaponMassKg = 120kg / Turret 호환 / Large
- Preset_HeavyFinite / RequiredMountType = Turret / RequiredWeaponSize = Large
- DA_Ammo_HeavyFinite / UnitMassKg = 2kg
- InitialSortieAmmoCount = 15
- AmmoMassKg = 30kg
```

공식 Fixture는 Production DataAsset의 질량값을 바꾸지 않고 별도 테스트 경로에 저장했다.

```text
/Game/CarFight/Tests/Fitting/DA_Fit_MobilityLight
- FittingId = Mobility_Light
- DisplayName = 모빌리티 경량 피팅
- same VehicleData
- RoofTurret_MediumOrLarge = ExplicitEmpty
- DefenseSelection = ExplicitNone
- Ammo = 없음
- Snapshot = Base 1000 + Equipment 0 + Ammo 0 + Defense 0 = 1000kg

/Game/CarFight/Tests/Fitting/DA_Fit_MobilityDefault
- FittingId = Mobility_Default
- DisplayName = 모빌리티 기본 피팅
- same VehicleData
- Preset_DefenseTest
- DefenseSelection = UseVehicleDefault
- Snapshot = Base 1000 + Equipment 470 + Ammo 0 + Defense 100 = 1570kg

/Game/CarFight/Tests/Fitting/DA_Fit_MobilityHeavy
- FittingId = Mobility_Heavy
- DisplayName = 모빌리티 중량 피팅
- same VehicleData
- Preset_HeavyFinite
- DA_Ammo_HeavyFinite × 15
- DefenseSelection = UseVehicleDefault
- Snapshot = Base 1000 + Equipment 470 + Ammo 30 + Defense 100 = 1600kg
```

공식 질량 순서:

```text
Light 1000kg < Default 1570kg < Heavy 1600kg <= MaximumGrossMass 2500kg
```

임의 ballast, 임의 WeaponMassKg, 임의 DefenseMassKg, 임의 Vehicle mass를 추가하지 않았다.

### 구현·저장·검증 증거

```text
Fixture create process: a00514602d8741ff8a7df8aaefcbb6b4 / Exit 0
Official Editor Build: 7e18b91c7171466fbe1b548527ab0b5a / Exit 0
CarFight.Fitting regression: 7fab5f60587d489f8ba231720180143a / 23/23 Success / 0 Fail
verify_existing + DisplayName save: c33a36b21a8f4079b38bba7c51e12864 / Exit 0
Fresh persisted AssetDump dataset: adset_v1_b60bd590730faab54910f1bbc42fdac4.fe0d1a67bac9a357bc81385a
Post-label OfficialMobilityFixtures: 5ab277165e9d4aef9e05b779106c808f / 1/1 Success / 0 Fail
```

`CarFight.Fitting.FIT_P0_07.OfficialMobilityFixtures`는 저장된 세 Fixture를 직접 로드해 동일 VehicleData, 각 질량 Breakdown, Light ExplicitEmpty·ExplicitNone, Heavy 15발×2kg와 Gross 한도를 보호한다. 이 테스트는 USER 주행감 판정을 대신하지 않는다.

판정:

```text
FIT-P0-07B Heavy Payload Resolution = Technical Complete
Official Light / Default / Heavy Fixture Preparation = Technical Complete
Persisted DisplayName / reference readback = PASS
Production mass value mutation = 0
USER PIE / USER driving feel PASS = 0
```

다음 기술 Gate는 `FIT-P0-07C Quantitative Mobility Measurement`다. 세 공인 Fixture를 그대로 사용해 가속·제동거리·조향 반응을 동일 조건으로 계측하되, 임의 합격 임계값을 먼저 만들지 않는다. 정량 결과와 사용자가 느끼는 주행감은 별도 Gate로 유지한다.

---

## 17.3 FIT-P0-07C — Quantitative Mobility Measurement

### 2026-08-17 Technical Closure

공식 Light / Default / Heavy Fixture의 질량값이나 Production VehicleData를 변경하지 않고 실제 Chaos Vehicle에서 동일 측정 프로토콜로 가속·제동·조향 수치를 확보했다.

측정 조건은 **비교를 위한 동일 입력 조건**이며 PASS 임계값이 아니다.

```text
공통 Map: /Game/Maps/M_VehicleDefensePIE
공통 VehicleData: DA_VehicleDefense_TestSUV
가속: 정지 → 30km/h / Throttle 1.0
제동: 약 30km/h → 기존 DriveState IdleEnterSpeedThresholdKmh 0.75km/h / Brake 1.0
조향: 약 30km/h 시작 / Throttle 0 / Steering 0.5 / 2.0초 누적 Yaw
Fixture 격리: Light / Default / Heavy 각각 Map reload + fresh PIE lifetime
Persistent Asset Save: 0
Production Mass Value Mutation: 0
Ordering Assertion: 없음
USER Driving Feel Assertion: 없음
```

같은 Chaos Vehicle instance 안에서 Light 측정 뒤 Default로 질량만 바꾸는 첫 Harness는 drivetrain/physics 상태가 다음 Fixture에 오염되는 현상을 보였다. 최종 Harness는 각 Fixture를 fresh PIE lifetime으로 분리해 cross-fixture 상태 재사용을 제거했다. 비소유 기준 차량은 현재 PIE PlayerController가 계측 동안만 임시 Possess하고, Pawn의 사용자 조향 Tick만 일시 중지하며 Chaos Movement Component Tick은 유지한다. 종료 시 해당 PIE lifetime의 원래 Possession을 복원하고 PIE를 종료한다.

### 최종 정량 결과

| 항목 | Light | Default | Heavy |
|---|---:|---:|---:|
| Configured Mass | 1000.000 kg | 1570.000 kg | 1600.000 kg |
| VehicleMesh Actual Mass | 1267.377 kg | 1837.377 kg | 1867.377 kg |
| 0→30 km/h 시간 | 2.349081 s | 2.252550 s | 2.248457 s |
| 0→30 km/h 거리 | 9.308084 m | 8.849235 m | 8.835980 m |
| 제동 시작 속도 | 30.088120 km/h | 30.072287 km/h | 30.057823 km/h |
| 30→0.75 km/h 제동 시간 | 0.794304 s | 0.821693 s | 0.833656 s |
| 30→0.75 km/h 제동거리 | 3.571402 m | 3.657704 m | 3.662367 m |
| 조향 시작 속도 | 30.135790 km/h | 30.072977 km/h | 30.130573 km/h |
| Steering 0.5 / 2s 누적 Yaw | 16.549116° | 20.418766° | 20.597601° |
| 조향 2s 종료 속도 | 3.358729 km/h | 8.196785 km/h | 8.488779 km/h |

Actual Mass는 세 Fixture 모두 Configured Mass 대비 정확히 `+267.377kg`의 PhysicsAsset/body 집계 오버헤드를 유지해 기존 Chaos Mass evidence와 일치한다.

### 결과 해석 경계

```text
제동
- 질량 증가에 따라 제동 시간·거리가 소폭 증가했다.
- Light 3.571402m → Default 3.657704m → Heavy 3.662367m.

가속
- 단순한 "가벼울수록 0→30이 빠르다" 순서는 나오지 않았다.
- Default/Heavy가 Light보다 약 0.10초 빨랐다.
- 이 결과를 자동 FAIL이나 튜닝 지시로 바꾸지 않는다.

조향
- 2초 누적 Yaw는 Default/Heavy가 더 컸다.
- 하지만 이 측정은 constant-speed steering test가 아니라 Throttle 0 상태의 coast-steering 통합 응답이다.
- 2초 종료 속도도 Light 3.36km/h, Default 8.20km/h, Heavy 8.49km/h로 크게 달라 Yaw 값에는 속도 유지 특성이 함께 섞여 있다.
- 따라서 "Heavy가 조향성이 더 좋다"로 해석하지 않는다.
```

현재 결과는 **실제 PhysicalMassOnly Chaos 응답의 기술 기준선**이다. 예상과 다른 비단조 결과가 존재하지만 사용자 체감과 게임 목표를 확인하기 전에 `AccelerationScale`, `BrakingScale`, `SteeringResponseScale` 또는 새 질량값을 자동 도입하지 않는다.

### 최종 Evidence

```text
Test Source: UE/Source/CarFight_Re/Private/CFMobilityMeasureTests.cpp v1.2.0
Task-local Runner: Tools/RunMobilityMeasure.ps1 v1.0.0
Final Official Editor Build: bb04d56e0cd24647b2ef6fcbc7f2bd77 / Exit 0
Final Quantitative Process: 63bbeaf0202041b79dac8f32f5447923 / Exit 0
Automation: CarFight.Fitting.FIT_P0_07C.QuantitativeMobility / Success
Metric Count: 3
Result JSON SHA-256: 048ba88a46ae9c07d765a2d7f7c17ec1b7090464f6846369b125892d39ddda4b
ordering_asserted=false
user_driving_feel_asserted=false
Content Asset / Map Save: 0
USER PIE PASS 추가: 0
```

판정:

```text
FIT-P0-07C Quantitative Mobility Measurement = Technical Complete
PhysicalMassOnly quantitative baseline = Captured
Mobility behavior acceptance = Not Judged
USER driving feel = Pending
Automatic Mobility Scalar/Tuning = 금지 / 미적용
```

Historical note: v0.17.0 당시 다음 Gate는 `FIT-P0-07D USER Driving Feel Comparison`이었다. 이 USER 비교는 실제 수행해 PASS한 기록이 없으며, 2026-09-14 Rebaseline에서 Current VehicleBuilder Performance Tuning Protocol로 책임이 대체되어 **Superseded / Not Executed**로 종료했다. 기존 quantitative result를 USER 체감 PASS로 확대하지 않는다.

---

## 18. FIT-P0-08 — Integrated Verification

### 계획 Automation

```text
CarFight.Fitting.FIT_P0_02.DataContract
CarFight.Fitting.FIT_P0_03.Compatibility
CarFight.Fitting.FIT_P0_03.MassSnapshot
CarFight.Fitting.FIT_P0_04.RuntimeApply
CarFight.Fitting.FIT_P0_04.AtomicBoundary
CarFight.Fitting.FIT_P0_05.InitialMass — 구현·Success
CarFight.Fitting.FIT_P0_05.MobilityAdapter — 후속
CarFight.Fitting.FIT_P0_06.BlueprintContract
CarFight.Fitting.FIT_P0_07.OfficialMobilityFixtures — 1/1 Success
```

### 사용자 PIE

```text
- 기본 피팅이 기존 장비·방어·주행을 유지
- 비호환 EquipmentPreset 적용 거부와 이유 표시
- Light / Default / Heavy 총중량 순서 확인
- Light / Default / Heavy 가속 차이
- Light / Default / Heavy 제동거리 차이
- Light / Default / Heavy 조향 반응 차이
- 장비 선택이 실제 WeaponData·TurretMountData에 반영
- Defense 선택이 실제 VehicleDefenseComp 초기화에 반영
- Ammo 선택이 출격 탄약과 질량에 한 번만 반영
- Launcher·Projectile·Damage·Pool·Defense 회귀
```

### 완료 조건

```text
- 공식 Editor 빌드 PASS
- Fitting Automation PASS
- 관련 Ammo·Defense·Launcher 회귀 PASS
- 사용자 PIE PASS
- VehicleFitting Current System 작성
- FeatureQueue Done 전환
```

---

## 19. 보호 범위

```text
- CF-FQ-029 Launcher·Missile의 현재 dirty 소스·에셋을 임의 정리하지 않는다.
- CF-FQ-031 Ammo의 수량·재장전 소유권을 피팅으로 이동하지 않는다.
- CF-FQ-033 Defense의 피해 공식을 피팅에 복제하지 않는다.
- ProjectileData.DefaultDamageData 경로를 변경하지 않는다.
- MountProfile의 legacy 제거 완료 경로를 되돌리지 않는다.
- VehicleData 원본을 플레이어 선택 저장소로 사용하지 않는다.
- 사용자 미커밋 .uasset을 임의 재저장하지 않는다.
- 게임 Audio를 추가하지 않는다.
- 사용자 요청 없이 commit·push·reset·checkout·stash를 수행하지 않는다.
```

---

## 20. 완료 후 문서 승격

Historical 신규 Current System 후보였던 별도 `VehicleFitting.md`는 만들지 않는다. Rebaseline 결과 기존 `VehicleRuntime.md`가 실제 런타임 owner이고 `VehicleData.md` / `VehicleBuilder.md`가 각각 데이터와 튜닝 책임을 이미 소유하므로, Current 계약을 기존 owner에 최소 승격한다.

갱신 후보:

```text
Document/Systems/Vehicles/VehicleData.md
Document/Systems/Vehicles/VehicleRuntime.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/Ammo.md
Document/Systems/Combat/VehicleDefense.md
Document/Systems/UI/InGameHUD.md
Document/Systems/UI/VehicleDebugPanel.md
```

Historical note: 당시에는 사용자 PIE 전 `CF-FQ-034 Done`을 금지했다. 2026-09-14 closure는 USER PIE를 PASS로 간주한 것이 아니라, 실제 기능 본체와 영구 계약이 Current Systems에 이미 흡수되고 남은 USER feel gate의 책임이 successor workflow로 대체됐음을 근거로 한 `SUPERSEDED / CLOSE`다.

---

## 21. 다음 작업

```text
원격 기술 체크포인트
→ FIT-P0-06 C++ ViewData·Blueprint Contract Technical Done
→ FFIT-P0-01~04 Technical Done / actual ChaosMassPIE PASS
→ FIT-P0-07A Fixture Readiness Audit Complete
→ FIT-P0-07B Official Light·Default·Heavy Fixture Technical Complete
→ FIT-P0-07C Quantitative Mobility Measurement Technical Complete

Historical v0.17.0 next gate
→ FIT-P0-07D USER Driving Feel Comparison
→ Superseded / Not Executed / USER PASS 아님
→ 필요 시 VehicleBuilder Performance Tuning Protocol의 USER Feel 범위로 새 관찰

Historical cross-feature USER / UI gate
→ FFIT-P0-05 Field Fitting UI and PIE
→ 16:9·32:9 가독성·진행률·취소 사유 USER 검증
→ CF-FQ-034 Runtime closure blocker가 아니라 Inventory/UI successor workflow가 소유
```

공식 Fixture와 정량 Mobility baseline은 Historical technical evidence로 보존한다. 정량 결과만으로 질량 차이의 게임성 적합성을 판정하거나 숨은 Mobility Scalar를 추가하지 않는 계약도 유지한다. 다만 현재는 이 미수행 USER 비교를 CF-FQ-034의 필수 closure blocker로 보지 않으며, 필요 시 successor tuning workflow에서 다시 관찰한다.

---

## 22. Changelog

### v0.18.0 - 2026-09-14

- `CF-FQ-034`를 current Source/System 기준으로 Rebaseline해 `SUPERSEDED / CLOSE`, Done / Historical + Retained Path로 전환했다.
- Fitting Snapshot, Initial/Field Runtime Apply, 실제 Chaos Mass 적용·검증과 rollback이 현재 Source에 유지·확장된 것을 확인하고 영구 Runtime/Mass 계약을 main_game `VehicleRuntime.md v1.3.0`으로 승격했다. 정적 Mass authoring/validation은 `VehicleData.md v2.3.0`, 향후 feel/tuning은 `VehicleBuilder.md v1.6.0`이 소유한다.
- FIT-P0-07A~07C quantitative evidence는 반복하지 않고 Historical Technical PASS로 보존했다. FIT-P0-07D는 USER PASS가 아니라 `Superseded / Not Executed`이며 향후 필요 시 Deferred observational debt로 새 tuning workflow에서 다룬다.
- Field Fitting UI·16:9/32:9 가독성은 CF-FQ-034 Runtime 완료조건에서 분리해 Inventory/UI successor 범위로 남겼다.
- Source/Asset mutation 0, Build/Automation/PIE 재실행 0, Product 값 변경 0이다. G0~G4 PASS / G5 Deferred다.

Migration: `VehicleFittingPlan.md v0.18.0`은 Historical evidence owner다. 새 작업에서 `FIT-P0-07D`를 current next gate로 재개하지 말고 Fitting/Mass runtime은 main_game `VehicleRuntime.md v1.3.0`, 성능/체감 튜닝은 `VehicleBuilder.md v1.6.0`을 기준으로 판단한다.

### v0.17.0 - 2026-08-17

- `FIT-P0-07C Quantitative Mobility Measurement`를 Technical Complete로 닫았다.
- 기존 공식 Light 1000kg / Default 1570kg / Heavy 1600kg와 `/Game/Maps/M_VehicleDefensePIE`를 그대로 사용하고 Fixture별 fresh PIE lifetime에서 동일 입력 조건을 적용했다.
- 최종 공식 Build `bb04d56e0cd24647b2ef6fcbc7f2bd77` PASS, `CarFight.Fitting.FIT_P0_07C.QuantitativeMobility` process `63bbeaf0202041b79dac8f32f5447923` Success, Metric 3/3을 확인했다.
- 0→30 시간은 2.349081 / 2.252550 / 2.248457초, 제동거리는 3.571402 / 3.657704 / 3.662367m, coast-steering 2초 누적 Yaw는 16.549116 / 20.418766 / 20.597601도로 측정됐다.
- Actual Mass는 Configured Mass 대비 세 Fixture 모두 +267.377kg로 기존 PhysicsAsset/body 집계 오버헤드를 유지했다.
- 가속과 coast-steering이 단순 질량 순서에 따라 단조 변화하지 않았지만, 임의 기대 순서를 PASS/FAIL 조건으로 추가하지 않았다.
- `ordering_asserted=false`, `user_driving_feel_asserted=false`를 보존하고 Production 질량값·Content Asset·Map을 변경하지 않았다.
- 자동 Mobility Scalar/튜닝은 추가하지 않고 다음 Gate를 `FIT-P0-07D USER Driving Feel Comparison`으로 이동했다.

Migration: 새 세션은 FIT-P0-07A~07C와 기존 Fitting 23/23을 반복하지 않는다. `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D`에서 같은 공식 세 Fixture를 사용해 사용자 직접 주행감만 비교한다. 정량 결과만으로 `AccelerationScale`, `BrakingScale`, `SteeringResponseScale` 또는 질량값을 변경하지 않는다.

### v0.16.0 - 2026-08-17

- `FIT-P0-07B Heavy Payload Resolution`을 Technical Complete로 닫았다.
- HeavyFinite의 저장 Mount 350kg, Weapon 120kg, Ammo 2kg×15를 같은 `DA_VehicleDefense_TestSUV`에 재조합해 Heavy 1600kg를 임의 질량 없이 성립시켰다.
- `/Game/CarFight/Tests/Fitting`에 Light 1000kg / Default 1570kg / Heavy 1600kg 공식 Fixture 3종을 저장하고 사용자 표시명을 모빌리티 경량/기본/중량 피팅으로 고정했다.
- 공식 Build `7e18b91c7171466fbe1b548527ab0b5a` PASS, Fitting 23/23 PASS와 저장 자산 직접 로드 `OfficialMobilityFixtures` post-label 1/1 PASS를 확인했다.
- Fresh AssetDump `adset_v1_b60bd590730faab54910f1bbc42fdac4.fe0d1a67bac9a357bc81385a`에서 세 DisplayName과 VehicleData·Preset·Ammo hard reference를 persisted readback했다.
- Production 질량값은 변경하지 않았고 USER PIE·USER 주행감 PASS는 추가하지 않았다.
- 다음 기술 Gate를 `FIT-P0-07C Quantitative Mobility Measurement`로 이동했다.

Migration: 새 세션은 `FIT-P0-07A~07B`와 기존 Build/Fitting 23/23을 반복하지 않는다. `VehicleFittingPlan.md v0.16.0 / FIT-P0-07C`에서 공인 세 Fixture를 그대로 사용해 정량 Mobility 측정을 시작한다. 사용자 주행감은 별도 USER Gate로 유지한다.

### v0.14.0 - 2026-08-15

- `FIT-P0-06 Fitting ViewData and Debug`의 C++ ViewData·Blueprint Contract를 Technical Done으로 전환했다.
- `FCFVehicleFittingViewData`와 Mount·Defense·Ammo·Mass·Mobility 행, `FCFFittingViewBuilder`를 추가했다. 기존 FittingSnapshot의 호환·질량·Validation 결과를 다시 계산하지 않고 결정론적으로 투영한다.
- VehicleData에 정식 플레이어 표시 이름이 아직 없어 P0 Debug `VehicleDisplayName`은 Asset 이름 fallback임을 명시하는 플래그를 함께 제공한다.
- Mobility Preview는 실제 Chaos 질량을 1차 효과로 유지하고 추가 Adapter는 비활성인 `PhysicalMassOnly`로 표시한다. MassRatio 외 별도 숨은 보정 공식을 만들지 않았다.
- Ammo ViewData는 실제 `InitialSortieAmmoCount`를 표시하고 `MaximumLoadableAmmoCount`를 현재 수량으로 오용하지 않는 계약을 자동화했다.
- 공식 Build `ad3452d3add74785bbdb41c667dce728` PASS, FIT-P0-06 `0c7ee328d7ef4f659d66d857fecdab2a` 1/1, Fitting `e5140ca056444ea694abb4f73861c935` 22/22, Inventory `c76221a75d9447dab1b03cc274d43a35` 12/12, Full CarFight `59aae9f0ac204e13a957dbbfe8cdbaec` 84/84 Success·0 Fail을 확인했다. Full SHA-256은 `4e49080b401ada72e243e8bd1a98e58e9bdd9d3af4471a03b78f7d7ca277c139`이다.
- 실제 UI Widget이 아직 없으므로 16:9·32:9 가독성은 USER PASS로 승격하지 않고 FFIT-P0-05와 함께 Pending으로 보존한다.
- 저장소에 공인 Light·Default·Heavy 전용 피팅 Fixture가 없으므로 임의 질량값으로 Mobility 완료를 만들지 않는다. 남은 Mobility는 정식 Fixture 기준 확정·정량 측정·USER 주행감 검증이다.

### v0.13.0 - 2026-08-15

- `FFIT-P0-04 Field Runtime Mass Reapply`를 Technical Done으로 전환했다.
- `ICFFieldFitMassRuntime`과 `FCFChaosVehicleMassRuntime`을 추가해 mass-changing 후보가 Weapon·Defense Runtime과 같은 completion transaction 안에서 Chaos Movement Mass를 재적용하도록 했다.
- Body-only `SetMassOverrideInKg`, MassScale, PhysicsAsset 저장 수정은 사용하지 않고 기존 SSOT대로 `FittingSnapshot.TotalVehicleMassKg → UChaosWheeledVehicleMovementComponent::Mass → RecreatePhysicsState → VehicleMesh.GetMass()` 경로를 유지한다.
- 후보 Runtime/Mass Apply 실패와 Inventory Commit 실패에서 이전 Applied Runtime과 이전 질량을 함께 복구한다. Weapon·Defense 내부 Rollback 또는 Mass 복구가 실패하면 단순 RuntimeCommitFailed가 아니라 `RecoveryFailed`로 승격한다.
- 실제 `/Game/Maps/M_VehicleDefensePIE` PIE에서 `1570.000kg configured / 1837.377kg actual → 1620.000 / 1887.377 → 1570.000 / 1837.377`을 확인해 PhysicsAsset 집계 오버헤드 267.377kg을 보존한 +50kg 적용·원복이 PASS했다.
- 최종 공식 Build `cd7207084dfd49c4a28b607d8577307a` PASS, FFIT-P0-04 `3c720a7453414e2798b78403bb2ba03b` 4/4, Fitting `b6968e3a18f04a86bc9ac421b0d37c7a` 21/21, Inventory `0c1e2ac86be14ab48cf863fe218e8f2a` 12/12, Full CarFight `f6a3a32df74a4091b158df8251352972` 83/83 Success·0 Fail을 확인했다. Full SHA-256은 `9390cc8458c31a340367ddcf6f091da5e16ed99977d8e5c056b078b2612d8505`다.
- FFIT-P0-04 Technical Done은 mass-changing completion의 C++/실제 Chaos 기술 경로 완료를 뜻하며 CF-FQ-034 전체 USER Mobility·Field UI 완료를 뜻하지 않는다.
- 다음 원격 기술 Gate는 `FIT-P0-06 Fitting ViewData and Debug`다. `FFIT-P0-05 Field Fitting UI and PIE`는 FIT-P0-06과 사용자 UI 확인 가능 조건 이후 진행한다.

### v0.12.0 - 2026-08-15

- `FFIT-P0-02 Timed Action and Reservation`을 Technical Done으로 전환했다. Permission PASS 시 단일 Inventory `PrepareTransfer`만 수행하고 Item/Runtime은 변경하지 않으며, 조건 변화·피해·파괴·슬롯/Inventory 변화·화면 닫힘·사용자 취소에서 Reservation을 Rollback한다.
- RequiredDuration 도달 시 `Completing`으로 전환하되 같은 Transaction은 `Prepared`로 유지해 completion 계층이 이어받는다.
- P0-02 Build `8ef1458bdd2c4a29956c5a83702d710e`, targeted `bc468049fbd847bab839629c3bf05978` 3/3, Fitting 14/14, Inventory 12/12, Full CarFight `6ec302015e43408387bb07fdb8b559b5` 76/76 Success를 확인했다. Full SHA-256은 `e5692811791b4b6983e0a05befdf77571d6bba43dc13764e7455d77692e9d234`다.
- `FFIT-P0-03 Atomic Equip and Unequip` formal handoff를 구현했다. `TimedAction::Completing`이 시작 때 소유한 동일 Prepared Transaction을 `FCFFieldFitCoordinator`에 넘기고 `AlreadyPrepared`를 정상 handoff로 처리한다.
- Equip·Unequip 성공은 Action Completed + Inventory Committed로 끝나며 Runtime 실패는 Coordinator Rollback 뒤 Action Failed, Reservation 유실은 Coordinator 호출 전에 `ReservationLost`로 차단한다.
- P0-03 Build `7a1a774cc0a64493975dc49f55dc4e09`, targeted `0aca2e1b958f40649fb648008cc4cc89` 3/3, Fitting `fef1565b21e046818a942e838bb82e12` 17/17, Inventory `511c4bdba5dd4671930255864a4ae8ae` 12/12, Full CarFight `cb3095863968406e89af99b38c69c9c8` 79/79 Success를 확인했다. Full SHA-256은 `3a3b8472dd8e19c1648e4e6df038ae50befd3e5829925d8a9402623fa4c62786`다.
- P0-03 Technical Done은 현재 same-mass Runtime 경계까지의 원자 completion 완료를 뜻한다. 실제 질량이 바뀌는 일반 Equip/Unequip은 `FFIT-P0-04 Field Runtime Mass Reapply` 전까지 계속 명시 거부한다.
- 다음 formal gate는 `FFIT-P0-04 Field Runtime Mass Reapply`다.

### v0.11.0 - 2026-08-15

- `FFIT-P0-01 Permission and Blocker Query`를 순수 C++ Provider-state 합성 Query로 구현 완료했다.
- `FCFFieldFitPermissionInput → FCFFieldFitPermissionQuery → FCFFieldFitPermissionResult` 구조로 Combat·Drive·VehicleRuntime·Weapon·Launcher·Ammo·Defense·Action·Inventory blocker를 결정론적으로 수집한다.
- 이동 중에는 `VehicleMoving`만, 정지 임계값 이하지만 유지시간 부족일 때만 `StationaryDurationInsufficient`를 반환해 Drive blocker 중복을 피한다.
- 정지 임계값·요구 유지시간은 호출자가 데이터 기반 값으로 공급하며 Query 내부에 게임 튜닝 상수를 고정하지 않는다.
- Reservation 생성·성공 여부는 의도적으로 FFIT-P0-02가 소유하며 FFIT-P0-01은 Runtime/Inventory 상태를 변경하지 않는다.
- 공식 Build `0a42d6369a194a07b1dba2fcc1e98286` PASS, targeted `6c914031a3e6474f9781e6c6816a7071` 2/2, Fitting `1c74d0756e574cb2813c667ade605269` 11/11, Inventory `326dab8d98164daea089360c657943e5` 12/12, Full CarFight `b36b42c06a4949e89ba2c95c44a22809` 73/73 Success·0 Fail을 확인했다.
- Full CarFight result SHA-256은 `c714ff261333353b44380396d27a7c6fe024ea6f256372114af526914624aa91`이다.
- 다음 formal gate는 `FFIT-P0-02 Timed Action and Reservation`이다.

### v0.10.0 - 2026-08-14

- `CF-FQ-035 INV-P0-06`에서 Field completion Coordinator Foundation이 구현·검증된 현재 상태를 반영했다.
- `FCFFieldFitCoordinator`의 Inventory Prepare → Runtime Prepare/Commit → Inventory Commit과 실패 보상 Rollback은 Technical PASS지만 정식 `FFIT-P0-03` 전체 완료로 승격하지 않는다.
- `UCFVehicleFittingComp v1.3.0`의 검증 Snapshot 직접 Prepare와 Applied Runtime Checkpoint 복원 계약을 반영했다.
- 현재 실제 Runtime Adapter는 Field Mass Reapply 전까지 0.01kg 이내 same-mass 후보만 허용하며 질량 변경 후보는 명시 거부한다.
- Inventory 선행 `INV-P0-03~05`가 완료됐으므로 stale `Blocked by CF-FQ-035` 표기를 제거하고 formal next를 `FFIT-P0-01 Permission and Blocker Query`로 고정했다.
- 최종 Build `e2ef556b64484a09ba8b3544c62344e3`, M6 3/3, Inventory 12/12, Full CarFight 71/71 Success를 현재 기술 증거로 기록했다.

### v0.9.0 - 2026-08-02

```text
- CF-FQ-035 INV-P0-04 Fitting Inventory Adapter의 Code·Automation·Build PASS 상태를 현재 의존성에 동기화했다.
- INV-P0-04의 순수 ItemInstance→Binding→FittingSnapshot 변환과 미구현 Field Fitting Coordinator를 분리했다.
- FIT-P0-05 Initial Sortie Mass 적용 코드가 이미 존재하므로 현재 질량 격차를 Field Runtime Mass Reapply로 한정했다.
- bVehicleCoreRuntimeReady와 bVehicleCombatRuntimeReady의 분리, Pawn Gameplay Context 소유권을 Field Permission Query 선행 계약에 연결했다.
- 다음 작업을 Physics PIE·Mobility 측정과 Permission Query·Coordinator 순서로 정정했다.
- Source, Blueprint, DataAsset, PhysicsAsset, Map과 SaveGame은 수정하지 않았다.
```

### v0.8.0 - 2026-08-02

```text
- FIT-P0-05 Initial Sortie Runtime Mass Adapter를 구현했다.
- UCFVehicleFittingComp에 Initial Mass 상태, Cached Snapshot Target, Legacy fallback, Verify Only, ReapplyRejected와 실제 질량 검증을 추가했다.
- ACFVehiclePawn의 PreRegisterAllComponents에서 Super 호출 전 UChaosVehicleMovementComponent::Mass를 1회 기록하고 BeginPlay에서 VehicleMesh.GetMass·GetPhysicsAsset·Physics State를 검증한다.
- 질량 검증 성공 뒤에만 같은 Cached Snapshot의 Weapon·Defense Commit을 허용한다.
- UE 5.8 Mass public 접근과 PreRegister 수명은 공식 Build dce20226611944f9b323f94b573d4a74, 최종 Build b7a4e52743c34fa2844ebe94925c6a0d에서 Exit Code 0으로 증명했다.
- CarFight.Fitting.FIT_P0_05.InitialMass를 추가하고 최종 전체 CarFight 41/41, 필수 회귀 20/20 Success를 확인했다.
- SetMassOverrideInKg, Body-only Override, PhysicsAsset·MassScale 수정, Hot Recreate와 Field Mass Reapply는 구현하지 않았다.
- Blueprint, DataAsset, PhysicsAsset, Map, SaveGame, UI, Ammo와 Inventory Adapter는 수정하지 않았다.
- CF-FQ-032 Active와 Inventory INV-P0-00~03 / INV-P0-04 Not Started를 유지했다.
- 실제 VehicleMesh Physics PIE와 Mobility 측정이 남아 있어 FIT-P0-05 전체를 Done으로 전환하지 않았다.
```

### v0.7.0 - 2026-08-02

```text
- FIT-P0-05 Runtime Mass Gate를 조사하고 초기 출격 질량 계약을 잠갔다.
- Design SSOT를 Snapshot.TotalVehicleMassKg, Chaos 설정 Mirror를 UChaosVehicleMovementComponent::Mass, 실제 증거를 VehicleMesh.GetMass로 분리했다.
- 현재 BeginPlay 피팅 Prepare는 무재생성 적용에 늦으므로 PreRegisterAllComponents의 Super 호출 전 질량 Prepare·기록을 확정했다.
- 물리 등록 전 초기 적용은 Physics State·Vehicle Simulation 재생성이 필요 없고, 생성 후 다른 질량 적용은 P0에서 거부하도록 결정했다.
- SetMassOverrideInKg, PhysicsAsset 수정과 VehicleMesh Body-only Override를 초기 출격 경로에서 금지했다.
- 실제 질량 허용 오차를 Max(1kg, TargetMass의 1%)로 잠갔다.
- Legacy·PreRegister 실패·Post-Physics 불일치·재초기화·EndPlay 정책을 구분했다.
- BP_CFVehiclePawn read-only AssetDump 3/3과 SportsCar 폴더 24/24 Success를 기록했다.
- SKM_SportsCar와 PA_SportsCar 자산 존재를 확인했지만 SkeletalMesh→PhysicsAsset 직접 연결과 계산 질량은 AssetDump 클래스 지원 한계로 미확인임을 유지했다.
- Source, Blueprint, DataAsset, PhysicsAsset과 Map은 수정하지 않았다.
- CF-FQ-032 Active와 Inventory INV-P0-00~03 / INV-P0-04 Not Started를 유지했다.
```

### v0.6.0 - 2026-08-01

```text
- FIT-P0-04 Sortie Fitting Apply Runtime을 구현 완료했다.
- UCFVehicleFittingComp에 Legacy·Snapshot Prepare, Weapon·Defense Commit, 실패 Rollback과 AppliedFittingSnapshot 수명을 추가했다.
- 무효 Snapshot은 하위 Adapter를 호출하지 않고 기존 Applied 상태를 보존한다.
- VehicleWeaponComp에 Snapshot EquipmentPresetData Override를 추가하고 VehicleData 기본 경로를 유지했다.
- VehiclePawn 초기화 순서를 Fitting Prepare·Commit 뒤 Launcher 연결 순서로 변경했다.
- Pawn 없는 RuntimeApply·AtomicBoundary Automation 2/2를 추가했다.
- 최종 공식 Editor Build 52b5b5572e2a4c739771dbd711a284d3 / Exit Code 0을 확인했다.
- 전체 CarFight 40/40, 필수 회귀 19/19 Success를 확인했다.
- Runtime Mass, Ammo, Inventory Adapter, Field Fitting, UI와 Unreal Asset은 변경하지 않았다.
- CF-FQ-032 Active와 Inventory INV-P0-00~03을 보호했다.
```

### v0.5.0 - 2026-08-01

```text
- CF-FQ-035 Inventory Foundation을 별도 기능 의존성으로 연결했다.
- 필드 피팅을 CF-FQ-034 내부 FFIT-P0 하위 트랙으로 추가했다.
- 비전투·전체 쿨타임 종료·차량 정지·Inventory 예약 조건을 Field Fitting 시작 Gate로 잠갔다.
- Equip은 호환되는 빈 슬롯에만 허용하고 교환은 Unequip과 Equip 두 액션으로 분리했다.
- 시간 액션 중에는 Reservation만 유지하며 완료 전 실제 Inventory·Applied Snapshot을 변경하지 않는 계약을 추가했다.
- 이동·전투 진입·피해·상태 변경·예약 손실·화면 종료 취소와 원상 유지 규칙을 추가했다.
- 후속 CF-FQ-032 공식 Editor Build와 Automation 32/32 PASS를 근거로 FIT-P0-03을 Done으로 전환했다.
- Source, Blueprint와 Unreal Asset은 수정하지 않았다.
```

### v0.4.0 - 2026-08-01

```text
- FIT-P0-03 Pawn 없는 BuildFittingSnapshot 순수 해석을 구현했다.
- MountProfile·Hardpoint, EquipmentPreset·Weapon 호환, 누락 선택 정책과 방어 3상태를 결정론적으로 해석했다.
- Base·Mount·Weapon·Defense 질량, 사용률과 MaximumGrossMass 초과 검증을 구현했다.
- Compatibility와 MassSnapshot Automation이 경고·오류 없이 Success했으며 전체 27개 Automation과 필수 회귀 17/17을 통과했다.
- 신규 피팅 소스 컴파일과 UnrealEditor-CarFight_Re.dll 링크는 통과했다.
- 기존 UnrealEditor-Cmd.exe 두 프로세스의 UnrealEditor-NetCore.dll 점유로 공식 Editor 전체 타깃 링크는 Blocked 상태다.
- VehiclePawn, VehicleMovement, Chaos 질량 적용, Blueprint, DataAsset, PhysicsAsset과 Map은 수정하지 않았다.
- 공식 빌드 PASS 전에는 FIT-P0-03을 Done 또는 FIT-P0-04를 Ready로 전환하지 않는다.
```

### v0.3.0 - 2026-08-01

```text
- FIT-P0-02 데이터 계약이 Chaos 런타임 API와 무관함을 확인하고 Engine·PhysicsAsset Gate를 FIT-P0-05로 이동했다.
- CFFittingTypes, VehicleFittingData, 공용 Snapshot·검증 타입과 Base·Weapon·Defense 질량 필드를 구현했다.
- 기존 데이터의 신규 질량 기본값을 0kg로 유지해 런타임 호환을 보존했다.
- 공식 Editor Build와 CarFight.Fitting.FIT_P0_02.DataContract를 PASS했다.
- 전체 CarFight 필수 전투 회귀 15/15 Success를 확인했다.
- Blueprint, DataAsset, Map과 VehiclePawn·Chaos 질량 적용 코드는 변경하지 않았다.
- FIT-P0-02를 Done, FIT-P0-03을 Ready로 전환했다.
```

### v0.2.3 - 2026-08-01

```text
- 피팅 문서에서 외부 도구 작업 ID, 설정, 런타임 상태와 복구 절차를 제거했다.
- 승인된 읽기 경로의 성공 여부, 미확인 증거와 FIT-P0-02 Gate 판정만 남겼다.
- 정확한 Engine Mass API, PhysicsAsset과 계산 질량 증거가 없어 Gate Blocked 상태는 유지했다.
- CarFight Source, Blueprint, DataAsset과 Map은 수정하지 않았다.
```

### v0.2.2 - 2026-08-01

```text
- FIT-P0-02 Gate 해제를 위해 승인된 엔진·자산 읽기 경로를 재확인했다.
- 정확한 Engine Mass API, PhysicsAsset과 계산 질량을 확인하지 못해 Gate Blocked를 유지했다.
- CarFight Source, Blueprint, DataAsset과 Map은 수정하지 않았다.
```

### v0.2.1 - 2026-08-01

```text
- FIT-P0-02 착수 전 Engine Mass API·PhysicsAsset Baseline Gate를 수행했다.
- 정확한 Mass API, PhysicsAsset 참조와 계산 질량 증거가 없어 FIT-P0-02를 Blocked로 전환했다.
- Gate 정책에 따라 Fitting C++ 타입·DataAsset 계약·질량 필드·테스트를 구현하지 않았다.
- VehiclePawn, Blueprint, DataAsset과 Map은 수정하지 않았다.
```

### v0.2.0 - 2026-08-01

```text
- FIT-P0-01 Decision Lock을 완료하고 FIT-D-001~014를 확정했다.
- 실제 물리 루트 VehicleMesh, SKM_SportsCar와 VehicleMovementComp 경로를 기존 성공 덤프와 C++로 교차 확인했다.
- SM_Body가 QueryOnly 시각 피격 표면이며 질량 소유자가 아님을 기록했다.
- VehicleData 명시 질량을 피팅 SSOT로, Chaos VehicleMovement 공식 Mass 경로를 단일 런타임 적용점으로 확정했다.
- BodyInstance 단독 Mass Override, PhysicsAsset 수정과 CenterOfMassOverride의 총중량 대용을 금지했다.
- Fresh AssetDump 차단과 정확한 엔진 Mass API·PhysicsAsset 수치 확인을 FIT-P0-02 사전 Gate로 분리했다.
- FIT-P0-02를 Ready로 전환했으며 Source와 Unreal Asset은 수정하지 않았다.
```

### v0.1.0 - 2026-07-31

```text
- CF-FQ-034 차량 피팅·질량 런타임 대표 Plan을 생성했다.
- 현재 차량·하드포인트·장비·탄약·방어·질량 구현을 조사했다.
- FIT-P0-00~08 작업 패키지와 선행 관계를 정의했다.
- 기존 CombatPlan의 넓은 P0를 실제 구현 가능한 출격 피팅·질량 범위로 축소했다.
- 연료·배터리·유틸리티·경제·영구 저장·전투 중 교환은 후속으로 분리했다.
- Source와 Unreal Asset은 수정하지 않았다.
```

---

## 23. Migration

- v0.11.0부터 FFIT-P0-01은 Technical Done이다. 후속 Field Fitting Action은 직접 하위 컴포넌트를 재판정하지 않고 이 Permission Result를 시작/취소 판단 입력으로 사용한다.
- Reservation은 FFIT-P0-02가 시작 시 `PrepareTransfer`, 취소·실패 시 `RollbackTransfer`로 소유하며 FFIT-P0-01에 역으로 넣지 않는다.
- v0.10.0부터 CF-FQ-035 M6 Coordinator Foundation은 재사용 가능한 completion 하위 계층으로 인정하지만, Timed Action 없이 `FFIT-P0-03 Done`으로 해석하지 않는다.
- Field Fitting의 formal sequence는 `FFIT-P0-01 → FFIT-P0-02 → FFIT-P0-03 → FFIT-P0-04`를 유지한다.
- `FCFFieldFitRuntimeAdapter`의 same-mass gate는 임시 물리 우회가 아니라 FFIT-P0-04 전 안전 경계다. 질량 변경 후보를 무시하거나 기존 질량으로 강제 적용하지 않는다.
- CF-FQ-032는 USER Visual checkpoint를 보존한 Paused 상태이며 현재 Active는 CF-FQ-035다.



```text
- v0.9.0부터 INV-P0-04 Adapter는 완료된 기반이며 Field Fitting에서 같은 ItemInstance 검증·Binding·Snapshot 변환을 재구현하지 않는다.
- INV-P0-04 Adapter는 순수 변환 계층이며 Inventory Commit, Vehicle Runtime Apply와 보상 Rollback을 수행하지 않는다.
- Field Fitting은 별도 Coordinator가 INV-P0-03 Prepared Transaction과 FIT-P0-04 Runtime Apply를 조율해야 한다.
- Field Permission Query는 차량 기본 가용성에 bVehicleCoreRuntimeReady를 사용하고, 전투 장비 교환 차단에는 bVehicleCombatRuntimeReady와 Launcher·Weapon 상태를 함께 확인한다.
- 차량 Gameplay 입력은 Pawn DefaultInputMappingContext가 소유하며 Controller Gameplay Context 원자 이전은 별도 UI 입력 마이그레이션으로 유지한다.
- FIT-P0-05는 물리 생성 전 초기 출격 질량만 지원하며 Physics State 생성 뒤 다른 질량 재적용은 여전히 미지원이다.
- v0.8.0부터 FIT-P0-05 초기 출격 질량 계약은 실제 UCFVehicleFittingComp·ACFVehiclePawn 구현 기준이다.
- PreRegisterAllComponents는 게임 World와 bAutoInitializeOnBeginPlay 조건에서만 Snapshot Target을 Movement Mass에 기록한다.
- BeginPlay는 PreRegister의 Prepared 입력을 재사용하고 실제 Mass·Physics 계약을 통과한 뒤에만 Weapon·Defense를 Commit한다.
- 초기 Invalid Snapshot은 기존 Movement Mass와 VehicleData Legacy Weapon·Defense 입력으로 fallback한다.
- Snapshot 질량이 이미 구성된 Pawn에서 다른 Target 또는 Legacy 전환은 ReapplyRejected로 거부한다.
- 같은 Target은 Mass를 다시 쓰지 않고 Verify Only로 처리한다.
- 공식 Build로 UE 5.8 UChaosVehicleMovementComponent::Mass public 접근이 증명됐으므로 BodyInstance 우회는 추가하지 않는다.
- 실제 VehicleMesh Physics PIE와 기동 측정 전에는 FIT-P0-05 전체를 Done으로 해석하지 않는다.
- v0.7.0부터 FIT-P0-05 초기 출격 질량은 PreRegisterAllComponents에서 Super 호출 전에 Movement Mass에 1회 적용한다.
- 현재 BeginPlay의 FIT-P0-04 Prepare 결과와 별도 Snapshot을 만들지 말고 PreRegister에서 만든 같은 Cached Snapshot을 재사용한다.
- FittingData 미지정·Invalid Snapshot은 Movement Mass를 건드리지 않고 기존 Legacy 물리 질량을 유지한다.
- Physics State 생성 뒤 다른 질량으로 재초기화하는 요청은 FIT-P0-05에서 거부하며 FFIT-P0-04 전에는 자동 Recreate를 추가하지 않는다.
- EndPlay은 Mass 캐시만 정리하고 Physics State 복원이나 재생성을 수행하지 않는다.
- VehicleMesh.GetPhysicsAsset과 GetMass는 Runtime 검증 증거이며 피팅 계산 SSOT가 아니다.
- 초기 출격 코드에서 SetMassOverrideInKg, MassScale, PhysicsAsset 수정과 Body-only Override를 사용하지 않는다.
- UE 5.8의 UChaosVehicleMovementComponent::Mass public 접근은 첫 구현 Editor Build에서 컴파일 증명한다.
- v0.6.0부터 FIT-P0-04는 공식 Editor Build와 전체 Automation을 포함해 Done이다.
- 출격 Runtime의 AppliedFittingSnapshot은 UCFVehicleFittingComp가 소유하며 Weapon·Defense는 같은 Prepared 입력을 Commit한다.
- FittingData 미지정 차량은 VehicleData 기본 Weapon·Defense Legacy 경로를 유지한다.
- 무효 Snapshot은 하위 Runtime을 호출하지 않고, Commit 중 실패는 직전 Applied 또는 Legacy 입력으로 복원한다.
- FIT-P0-05만 Runtime Mass Gate를 열며 FIT-P0-04는 Chaos 질량을 적용하지 않는다.
- v0.5.0부터 FIT-P0-03은 후속 공식 Editor Build와 Automation PASS를 포함해 Done이다.
- FIT-P0-04 Sortie Apply Runtime은 기존 FittingData 기반으로 구현할 수 있으나 Field Fitting의 실제 Item 이동을 대신하지 않는다.
- Field Fitting Runtime은 CF-FQ-035 INV-P0-03 Reservation·Atomic Transfer 전에는 시작하지 않는다.
- 실제 ItemInstance 기반 장착은 INV-P0-04 Fitting Adapter 전에는 구현하지 않는다.
- 필드 질량 재적용은 Runtime Mass Gate 잠금만으로 시작하지 않으며 FIT-P0-05 초기 적용 구현·검증과 FFIT-P0-03 완료 뒤 별도 재생성 계약으로 진행한다.
- 기존 VehicleFittingData UObject 참조는 Template·Legacy 입력으로 유지하며 실제 소유권 증거로 간주하지 않는다.
- 장착된 슬롯의 직접 교체를 허용하지 않고 Unequip과 Equip 두 액션을 사용한다.
- BuildFittingSnapshot은 VehicleData.MountProfiles 순서를 결과 SSOT로 사용하며 MountSelections 배열 순서에 의존하지 않는다.
- ExplicitEmpty, MissingPolicyEmpty와 VehicleDefault를 서로 다른 SelectionSource로 유지한다.
- VehicleData 기본 방어 None과 ExplicitNone은 모두 방어 질량 0의 정상 구성이나 ResolvedDefenseSelectionMode로 의미를 구분한다.
- 선택된 장비·방어의 0kg 질량은 미설정으로 보고 적용을 거부한다.
- Ammo 선택 타입은 CF-FQ-031 계약을 재사용하며 피팅 전용 임시 중복 타입을 만들지 않는다.
- Runtime Mass Gate는 초기 적용 API·수명·검증 기준을 잠갔으며 VehicleMesh PhysicsAsset 경로와 실제 질량은 FIT-P0-05 Runtime 검증에서 수집한다.
- PhysicsAsset 계산 질량을 피팅 SSOT로 승격하거나 VehicleMesh BodyInstance만 단독 Override하지 않는다.
- FIT-P0-04에서 VehiclePawn과 VehicleWeaponComp는 출격 Snapshot 적용 통합에 필요한 범위만 수정했다. VehicleMovement, Blueprint, DataAsset, PhysicsAsset과 Map은 변경하지 않았다.
- 이 문서는 현재 Active CF-FQ-029 Launcher Plan을 대체하지 않는다.
```