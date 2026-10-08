# CarFight Data Authoring Design

- 문서 버전: v0.8.1
- 작성일: 2026-08-17
- 문서 상태: Active / Implementation Contract
- 상위 계획: `Document/Plan/DataAuthoring/DataAuthoringPlan.md`
- 역할: Data Authoring의 상세 데이터 모델, 책임 경계, Vehicle 우선 설계, Migration 기준을 정의한다.

---

## 1. 문서 목적

이 문서는 `DataAuthoringPlan.md`의 방향을 실제 구현 가능한 구조로 구체화한다.

현재 단계에서 중요한 것은 클래스를 먼저 만드는 것이 아니라 다음 질문에 답하는 것이다.

```text
어떤 값은 사람이 결정해야 하는가?
어떤 값은 Profile이 제공해야 하는가?
어떤 값은 다른 값에서 계산할 수 있는가?
어떤 값은 Asset에서 읽을 수 있는가?
어떤 값은 Definition에 확정 저장해야 하는가?
어떤 값은 Fitting이 계산해야 하는가?
어떤 값은 Runtime 상태인가?
어떤 값은 USER 주행감 확인 전 확정하면 안 되는가?
```

---

## 2. 전체 데이터 모델

```text
┌─────────────────────────────────────────────┐
│ AUTHORING                                   │
│                                             │
│ Human / AI Intent                          │
│      ↓                                      │
│ Recipe                                      │
│      ↓                                      │
│ Profiles / Rules / Asset-derived Inputs    │
│      ↓                                      │
│ Resolver                                    │
│      ↓                                      │
│ Resolved Preview + Source Trace + Diff      │
│      ↓                                      │
│ Validator                                   │
└────────────────────┬────────────────────────┘
                     │ Apply
                     ▼
┌─────────────────────────────────────────────┐
│ DEFINITION                                  │
│ Vehicle / Weapon / Equipment / Scanner ... │
└────────────────────┬────────────────────────┘
                     │ Stable Ref / ID
          ┌──────────┴──────────┐
          ▼                     ▼
┌─────────────────┐   ┌──────────────────────┐
│ Inventory       │   │ Fitting              │
│ Instance State  │──▶│ Installed State      │
└─────────────────┘   │ Fitting Derived      │
                      └──────────┬───────────┘
                                 ▼
                         ┌───────────────┐
                         │ Runtime State │
                         └───────────────┘
```

---

## 3. Authoring Field Classification

DAUTH-P0-01에서 VehicleData의 실제 모든 필드를 아래 기준으로 분류한다.

### 3.1 Human Intent

게임 디자이너 또는 사용자가 의미를 결정해야 하는 값.

예:

- 차량 역할
- 차급
- 주행 성향
- 성능 방향
- 장갑 성향
- 무장 레이아웃 의도

Raw Chaos 수치가 Human Intent가 되는 것은 지양한다.

### 3.2 Profile

여러 Definition에서 재사용 가능한 기본 Authoring 정책.

예:

- Mid Sedan Base
- Sport Handling
- Comfort Suspension
- Acceleration-biased Performance

Profile이 실제 UObject인지 Struct인지 Table인지 여부는 P0-02에서 결정한다.

### 3.3 Derived

다른 Authoring 데이터에서 계산 가능한 값.

Derived에는 계산 근거 Source가 있어야 한다.

### 3.4 Asset Derived

Mesh, Socket, Bounds, Metadata 등 Unreal Asset에서 읽을 수 있는 값.

Vehicle의 Layout Capture가 대표 사례다.

### 3.5 Advanced Override

기본 계산 결과를 의도적으로 덮어쓰는 값.

Override에는 최소한 다음 정보가 필요하다.

```text
Override 여부
Override 값
Resolved 값
기본 Source
```

### 3.6 Definition Stored

Runtime과 다른 시스템이 소비하는 확정 정적 데이터.

현재 Vehicle에서는 `UCFVehicleData`가 중심이다.

### 3.7 Fitting Derived

실제 장비 조합 결과로 결정되는 값.

Authoring Resolver가 소유하지 않는다.

### 3.8 Runtime State

실행 중 변하는 값.

Definition이나 Recipe에 보관하지 않는다.

### 3.9 USER Acceptance

자동 계산으로 PASS를 확정할 수 없는 체감 / 시각 / 조작성 결과.

---

## 4. VehicleData Ownership Matrix — DAUTH-P0-01

### 4.1 Scope / 작성 규칙

DAUTH-P0-00 Foundation Audit 결과를 반복하지 않고 Current Source를 기준으로 `UCFVehicleData`에 실제 저장되는 필드를 Field Path 단위로 전수 분류한다.

Current Authority:

```text
UE/Source/CarFight_Re/Public/CFVehicleData.h
UE/Source/CarFight_Re/Public/CFVehicleDriveStateConfig.h
UE/Source/CarFight_Re/Public/CFVehicleWeaponTypes.h
UE/Source/CarFight_Re/Private/CFVehicleData.cpp
UE/Source/CarFight_Re/Private/CFVDAValidator.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/CFVehicleFittingData.cpp
관련 Runtime Consumer
```

포함 범위:

- `UCFVehicleData`의 top-level UPROPERTY
- 위 top-level 필드가 직접 포함하는 `FCFVehicleVisualConfig`
- `FCFVehicleLayoutConfig` / `FCFWheelAnchorPose`
- `FCFVehicleHardpointSlot`
- `FCFVehicleMountProfile`
- `FCFVehicleMovementConfig`
- `FCFVehicleWheelVisualConfig`
- `FCFVehicleReferenceConfig`
- `FCFVehicleDurabilityConfig`
- `FCFVehicleDriveStateConfig`

제외 범위:

- 같은 헤더에 선언되어도 `UCFVehicleData`에 저장되지 않는 `FCFVehicleTurretState`, `FCFVehicleFireOrigin` 같은 Runtime Result/State
- Fitting Snapshot, Inventory Instance, Ammo, Damage 등 다른 시스템이 소유하는 Runtime/Derived 상태

`MountProfiles`의 legacy `UPROPERTY()` 필드는 Editor 편집 노출이 없어도 실제 직렬화 필드이므로 누락하지 않는다.

행 수 감사:

| 구분 | Matrix 행 수 |
|---|---:|
| Top-level aggregate holder | 9 |
| `VehicleVisualConfig` leaf | 5 |
| `VehicleLayoutConfig` leaf | 13 |
| `HardpointSlots[]` leaf | 5 |
| `MountProfiles[]` leaf | 16 |
| Fitting mass top-level leaf | 2 |
| `VehicleMovementConfig` leaf | 47 |
| `WheelVisualConfig` leaf | 8 |
| `VehicleReferenceConfig` leaf | 2 |
| Durability / Defense / FX leaf | 4 |
| `DriveStateConfig` leaf | 15 |
| **합계** | **126** |

분류 표기에서 `Definition Stored`는 최종 Runtime/다른 시스템이 소비하는 확정 정적 값이라는 뜻이며, 그 값이 반드시 사람이 Raw Field로 직접 입력해야 한다는 뜻은 아니다.

### 4.2 Top-level Aggregate Holder Matrix

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| VehicleVisualConfig | UCFVehicleData | Definition Stored (Aggregate) | Leaf fields | Struct C++ defaults | Yes | None direct | Pawn visual apply | Leaf validators | See leaf fields | Do not expose as one opaque Recipe blob. |
| VehicleLayoutConfig | UCFVehicleData | Definition Stored (Aggregate) | Leaf fields / Asset capture | Struct C++ defaults | Yes | None direct | Pawn layout apply | Layout + Socket validators | See leaf fields | Authoring must preview capture before apply. |
| HardpointSlots | UCFVehicleData | Human Intent + Asset Derived + Definition Stored | Slot intent + Chassis sockets | Empty array allowed | Yes | Mount/Fitting location contract | Weapon FireOrigin / Fitting lookup | ValidateHardpointSlots | Conditional — weapon placement | Keep array empty valid; do not auto-invent slots. |
| MountProfiles | UCFVehicleData | Human Intent + Profile + Definition Stored | Mount intent / equipment profile | Empty array allowed; PostLoad may seed legacy Top_01 profile | Yes | Core fitting compatibility/default contract | Weapon/Fitting runtime | ValidateMountProfiles + BuildFittingSnapshot | Conditional — mounted equipment | Separate active editable fields from legacy serialized fields. |
| VehicleMovementConfig | UCFVehicleData | Profile + Derived + Advanced Override + Definition Stored | Driving intent / vehicle context / explicit override | Project C++ baseline + legacy PostLoad migration | Yes | Mass only indirectly through driving; not Fitting owner | Pawn/Chaos Vehicle | ValidateMovementConfig + runtime tests | Yes — driving feel | Never clone all 47 raw fields into Recipe. |
| WheelVisualConfig | UCFVehicleData | Profile + Asset Derived + Advanced Override + Definition Stored | Wheel assets / visual policy | Struct C++ defaults | Yes | None direct | Pawn WheelSync / wheel mesh visual | ValidateWheelVisualConfig | Yes — visual | bUseWheelVisualOverrides is not a runtime gate. |
| VehicleReferenceConfig | UCFVehicleData | Profile + Definition Stored | Wheel class selection / vehicle profile | Explicit class refs required for current vehicles | Yes | None direct | Pawn WheelSetup | ValidateRequiredReferences | Conditional — wheel physics | Candidate to resolve from vehicle/drivetrain profile. |
| VehicleDurabilityConfig | UCFVehicleData | Profile + Advanced Override + Definition Stored | Durability/balance intent | MaxHealth=100 | Yes | None direct | VehicleHealthComp | No direct VDA rule | Yes — combat balance | Keep separate from DefenseData pools. |
| DriveStateConfig | UCFVehicleData | Profile + Advanced Override + Definition Stored | DriveState policy / vehicle-specific override | Struct C++ defaults; override disabled | Yes | None direct | VehicleDriveComp | ValidateDriveStateConfig | Yes — state transition behavior | Runtime gate semantics are real; do not merge with other override flags. |

### 4.3 VehicleVisualConfig

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| VehicleVisualConfig.ChassisMesh | FCFVehicleVisualConfig | Human Intent + Definition Stored | Explicit vehicle asset selection | Required explicit asset; no invented fallback | Yes | Indirect: source for hardpoint/layout contract | Pawn SM_Body visual + layout capture source | ValidateRequiredReferences + ValidateWheelSockets | Yes — chassis visual | Existing value imports as Legacy Explicit Baseline. |
| VehicleVisualConfig.WheelMeshFL | FCFVehicleVisualConfig | Human Intent + Definition Stored | Explicit wheel asset selection | Required baseline wheel mesh | Yes | None direct | Pawn Wheel_Mesh_FL / auto-scale | ValidateRequiredReferences | Yes — wheel visual | Can become profile/asset-set assisted later; preserve explicit legacy. |
| VehicleVisualConfig.WheelMeshFR | FCFVehicleVisualConfig | Human Intent + Definition Stored | Explicit wheel asset selection | Prefer explicit; current validator allows warning if empty | Yes | None direct | Pawn Wheel_Mesh_FR / auto-scale | ValidateRequiredReferences | Yes — wheel visual | Do not silently replace existing asymmetric wheel assets. |
| VehicleVisualConfig.WheelMeshRL | FCFVehicleVisualConfig | Human Intent + Definition Stored | Explicit wheel asset selection | Prefer explicit; current validator allows warning if empty | Yes | None direct | Pawn Wheel_Mesh_RL / auto-scale | ValidateRequiredReferences | Yes — wheel visual | Do not infer from FL during import without preview. |
| VehicleVisualConfig.WheelMeshRR | FCFVehicleVisualConfig | Human Intent + Definition Stored | Explicit wheel asset selection | Prefer explicit; current validator allows warning if empty | Yes | None direct | Pawn Wheel_Mesh_RR / auto-scale | ValidateRequiredReferences | Yes — wheel visual | Do not infer from FL during import without preview. |

### 4.4 VehicleLayoutConfig

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| VehicleLayoutConfig.bUseLayoutOverrides | FCFVehicleLayoutConfig | Derived + Definition Stored | Successful layout authoring/capture state | False for legacy; set True only with resolved layout | Yes | None direct | Pawn gates WheelAnchor apply | ValidateLayoutConfig | Yes — wheel placement | Prefer resolver/apply state, not a normal Human Intent checkbox. |
| VehicleLayoutConfig.BodyWheelSocketFL | FCFVehicleLayoutConfig | Profile + Advanced Override + Definition Stored | Project socket convention + ChassisMesh inspection | Wheel_Anchor_FL | Yes | None direct | Editor capture only | ValidateWheelSockets | No direct; capture result yes | Treat as asset-binding input, not derived pose. |
| VehicleLayoutConfig.BodyWheelSocketFR | FCFVehicleLayoutConfig | Profile + Advanced Override + Definition Stored | Project socket convention + ChassisMesh inspection | Wheel_Anchor_FR | Yes | None direct | Editor capture only | ValidateWheelSockets | No direct; capture result yes | Treat as asset-binding input, not derived pose. |
| VehicleLayoutConfig.BodyWheelSocketRL | FCFVehicleLayoutConfig | Profile + Advanced Override + Definition Stored | Project socket convention + ChassisMesh inspection | Wheel_Anchor_RL | Yes | None direct | Editor capture only | ValidateWheelSockets | No direct; capture result yes | Treat as asset-binding input, not derived pose. |
| VehicleLayoutConfig.BodyWheelSocketRR | FCFVehicleLayoutConfig | Profile + Advanced Override + Definition Stored | Project socket convention + ChassisMesh inspection | Wheel_Anchor_RR | Yes | None direct | Editor capture only | ValidateWheelSockets | No direct; capture result yes | Treat as asset-binding input, not derived pose. |
| VehicleLayoutConfig.WheelAnchorFL.RelativeLocation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | ValidateLayoutConfig (location aggregate only) | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorFL.RelativeRotation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | No direct VDA leaf rule | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorFR.RelativeLocation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | ValidateLayoutConfig (location aggregate only) | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorFR.RelativeRotation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | No direct VDA leaf rule | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorRL.RelativeLocation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | ValidateLayoutConfig (location aggregate only) | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorRL.RelativeRotation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | No direct VDA leaf rule | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorRR.RelativeLocation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | ValidateLayoutConfig (location aggregate only) | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |
| VehicleLayoutConfig.WheelAnchorRR.RelativeRotation | FCFVehicleLayoutConfig / FCFWheelAnchorPose | Asset Derived + Advanced Override + Definition Stored | ChassisMesh wheel socket transform or explicit override | Capture from configured socket; preserve legacy if not regenerated | Yes | None direct | Pawn Wheel_Anchor component apply when bUseLayoutOverrides | No direct VDA leaf rule | Yes — wheel alignment | Import existing value as Legacy Explicit Baseline; regenerate only via preview/diff. |

### 4.5 HardpointSlots[]

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| HardpointSlots[].LocationSlotId | FCFVehicleHardpointSlot | Human Intent + Definition Stored | Weapon layout intent / stable slot naming | Explicit stable ID; empty array valid | Yes | Referenced by MountProfiles and Fitting | Weapon FireOrigin + Fitting slot lookup | ValidateHardpointSlots + ValidateMountProfiles | Conditional — weapon placement | Do not auto-rename existing stable IDs. |
| HardpointSlots[].LocationCategory | FCFVehicleHardpointSlot | Human Intent + Profile + Definition Stored | Semantic placement category | Explicit/profile default; no current runtime requirement | Yes | None current | No current production consumer found | No direct VDA rule | No current runtime; future UX | Keep as semantic metadata; do not treat as authoritative compatibility yet. |
| HardpointSlots[].SocketName | FCFVehicleHardpointSlot | Asset Derived + Advanced Override + Definition Stored | ChassisMesh hardpoint socket selection | None allowed = keep LocalTransform | Yes | Indirect through slot transform | Weapon FireOrigin socket-first + editor capture | ValidateHardpointSlots | Yes — muzzle/hardpoint alignment | Preserve None as intentional manual-transform mode. |
| HardpointSlots[].LocalLocation | FCFVehicleHardpointSlot | Asset Derived + Advanced Override + Definition Stored | ChassisMesh socket transform or manual placement | Capture when SocketName resolves; otherwise preserve | Yes | Indirect through mount location | Weapon FireOrigin fallback / fitting-resolved location ID | ValidateHardpointSlots | Yes — fire origin | Never overwrite manual value when capture socket is absent. |
| HardpointSlots[].LocalRotation | FCFVehicleHardpointSlot | Asset Derived + Advanced Override + Definition Stored | ChassisMesh socket transform or manual placement | Capture when SocketName resolves; otherwise preserve | Yes | Indirect through mount orientation | Weapon FireOrigin orientation fallback | No direct VDA leaf rule | Yes — fire direction/orientation | Add explicit preview before regenerated transform apply. |

### 4.6 MountProfiles[]

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| MountProfiles[].MountProfileId | FCFVehicleMountProfile | Human Intent + Definition Stored | Stable mount-rule identity | Explicit stable ID; existing IDs preserved | Yes | Primary selection key in Fitting | Fitting + VehicleWeaponComp | ValidateMountProfiles + BuildFittingSnapshot | No direct visual; compatibility check | Never auto-rename; source-tracked ID policy needed in P0-02. |
| MountProfiles[].LocationSlotRef | FCFVehicleMountProfile | Human Intent + Definition Stored | Select existing HardpointSlots[].LocationSlotId | Must reference existing slot | Yes | Core hardpoint binding | Fitting + VehicleWeaponComp FireOrigin | ValidateMountProfiles + BuildFittingSnapshot | Yes — mounted position | Resolver may suggest but must not invent missing slot silently. |
| MountProfiles[].MountType | FCFVehicleMountProfile | Human Intent + Profile + Definition Stored | Equipment layout intent / mount profile | Profile/explicit selection | Yes | Compatibility contract copied to resolved mount | Fitting + VehicleWeaponComp behavior | BuildFittingSnapshot / equipment compatibility; VDA no direct leaf rule | Yes — equipment behavior | Keep as Definition compatibility; not Fitting Derived. |
| MountProfiles[].SizeLimit | FCFVehicleMountProfile | Human Intent + Profile + Definition Stored | Vehicle platform/mount capacity intent | Profile/explicit selection | Yes | Core equipment size compatibility | Fitting resolved mount | BuildFittingSnapshot; VDA no direct leaf rule | No feel; balance/fit review | Do not infer from currently installed item. |
| MountProfiles[].DefaultEquipmentPresetData | FCFVehicleMountProfile | Profile + Advanced Override + Definition Stored | Vehicle default loadout/profile | None allowed; explicit default if desired | Yes | Vehicle-default selection source | Fitting + VehicleWeaponComp legacy/default runtime | BuildFittingSnapshot / equipment contract; VDA no direct leaf rule | Conditional — default loadout | Inventory-installed item may override; Authoring owns only Definition default. |
| MountProfiles[].TurretYawMesh | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (None C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].TurretPitchMesh | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (None C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].TurretYawRelativeTransform | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (Identity C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].PitchPivotSocketName | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (PitchPivot C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].TurretPitchRelativeTransform | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (Identity C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].YawTurnRateDegPerSec | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (35 C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].PitchTurnRateDegPerSec | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (20 C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].StabilizationToleranceDeg | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (2 C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].AimSettleTimeSeconds | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (0.35 C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current runtime uses TurretMountData; candidate retirement only after asset migration evidence. |
| MountProfiles[].MountWeightKg | FCFVehicleMountProfile (Legacy Serialized) | Definition Stored | Legacy Imported Value | Preserve existing serialized value; new authoring must not generate (350 C++ default) | Yes — legacy serialized UPROPERTY | None current | No current production consumer found | No current validation owner | No | Legacy serialized only; current fitting mass uses TurretMountData/WeaponData mass; candidate retirement only after asset migration evidence. |
| MountProfiles[].bExposedModule | FCFVehicleMountProfile | Human Intent + Profile + Definition Stored | Module exposure intent | True current default; profile/explicit override candidate | Yes | None current | No current production consumer found; reserved for module damage | No direct VDA rule | Future module-damage acceptance | Keep active semantic field, but do not claim current damage behavior. |

### 4.7 Fitting Mass

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| BaseVehicleMassKg | UCFVehicleData | Human Intent + Profile + Definition Stored | Vehicle platform mass intent / approved balance value | 0 = legacy unset; do not auto-estimate | Yes | Direct base of Fitting Snapshot total mass | Fitting pre-physics mass path when fitting is active | ValidateFittingMassConfig + BuildFittingSnapshot | Conditional — fitted mobility/balance | Never replace with current fitted mass; Fitting Derived remains separate. |
| MaximumGrossMassKg | UCFVehicleData | Profile + Advanced Override + Definition Stored | Vehicle load-capacity policy | 0 = legacy unset; when set must accompany Base mass | Yes | Hard maximum for Fitting Snapshot | Fitting validation / apply gate | ValidateFittingMassConfig + BuildFittingSnapshot | Conditional — loadout/balance | Do not derive from currently installed loadout. |

### 4.8 VehicleMovementConfig

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| VehicleMovementConfig.bUseMovementOverrides | FCFVehicleMovementConfig | Derived + Definition Stored | Whether detailed Wheel/Throttle vehicle-specific values are authored | False legacy; True only when detailed override path is intended | Yes | None direct | Gates Wheel runtime detailed tuning + ThrottleInputScale only | ValidateMovementConfig | Yes — because behavior path changes | Do not interpret as master Movement enable; engine/body values still apply. |
| VehicleMovementConfig.MovementProfileName | FCFVehicleMovementConfig | Profile + Definition Stored | Legacy/profile label | None unless intentionally labeled | Yes | None | Debug/summary only; no physical effect | CompareVehicleData only | No direct | Metadata only; P0-02 may replace/augment with formal source tracking. |
| VehicleMovementConfig.ThrottleInputScale | FCFVehicleMovementConfig | Derived + Advanced Override + Definition Stored | Driving Feel acceleration/throttle intent | 1.0 fallback; resolve from intent/context | Yes | None | Pawn throttle input when bUseMovementOverrides | ValidateMovementConfig + CompareVehicleData | Yes — acceleration feel | Quick Tune currently writes this; import raw value, do not reverse-infer intent as truth. |
| VehicleMovementConfig.FrontWheelMaxSteerAngle | FCFVehicleMovementConfig | Derived + Advanced Override + Definition Stored | Steering agility intent/profile | Profile/resolver baseline | Yes | None | Wheel CDO + direct runtime setter when detailed overrides active | ValidateMovementConfig + CompareVehicleData | Yes — low/mid/high speed steering | Current Quick Tune field. |
| VehicleMovementConfig.FrontWheelMaxBrakeTorque | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Braking response / wheel profile | Profile/resolver baseline | Yes | None | Wheel tuning when detailed overrides active | ValidateMovementConfig (no direct leaf range rule) | Yes — braking/handbrake | Raw technical value; do not require normal user entry. |
| VehicleMovementConfig.RearWheelMaxBrakeTorque | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Braking response / wheel profile | Profile/resolver baseline | Yes | None | Wheel tuning when detailed overrides active | ValidateMovementConfig (no direct leaf range rule) | Yes — braking/handbrake | Raw technical value; do not require normal user entry. |
| VehicleMovementConfig.RearWheelMaxHandBrakeTorque | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Braking response / wheel profile | Profile/resolver baseline | Yes | None | Wheel tuning when detailed overrides active | ValidateMovementConfig (no direct leaf range rule) | Yes — braking/handbrake | Raw technical value; do not require normal user entry. |
| VehicleMovementConfig.FrontWheelRadius | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Wheel/tire asset geometry or explicit physics size | Prefer measured/approved wheel radius; preserve explicit legacy | Yes | None | Wheel physics; also WheelVisual auto-scale target | ValidateMovementConfig + ValidateWheelVisualConfig + CompareVehicleData | Yes — wheel alignment/handling | Strong Asset Derived candidate; must allow explicit physics override. |
| VehicleMovementConfig.RearWheelRadius | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Wheel/tire asset geometry or explicit physics size | Prefer measured/approved wheel radius; preserve explicit legacy | Yes | None | Wheel physics; also WheelVisual auto-scale target | ValidateMovementConfig + ValidateWheelVisualConfig + CompareVehicleData | Yes — wheel alignment/handling | Strong Asset Derived candidate; must allow explicit physics override. |
| VehicleMovementConfig.FrontWheelWidth | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Wheel/tire asset geometry or explicit physics width | Prefer measured/approved width; preserve explicit legacy | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig + CompareVehicleData | Conditional — tire/handling | Asset-derived candidate; runtime immediate setter coverage differs from radius. |
| VehicleMovementConfig.RearWheelWidth | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Wheel/tire asset geometry or explicit physics width | Prefer measured/approved width; preserve explicit legacy | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig + CompareVehicleData | Conditional — tire/handling | Asset-derived candidate; runtime immediate setter coverage differs from radius. |
| VehicleMovementConfig.FrontWheelFrictionForceMultiplier | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Grip Feel / tire profile / vehicle context | Resolver/profile baseline | Yes | None | Wheel friction tuning when detailed overrides active | ValidateMovementConfig (group) + CompareVehicleData | Yes — grip/slip | Current Quick Tune field; USER feel remains pending. |
| VehicleMovementConfig.RearWheelFrictionForceMultiplier | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Grip Feel / tire profile / vehicle context | Resolver/profile baseline | Yes | None | Wheel friction tuning when detailed overrides active | ValidateMovementConfig (group) + CompareVehicleData | Yes — grip/slip | Current Quick Tune field; USER feel remains pending. |
| VehicleMovementConfig.FrontWheelCorneringStiffness | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Grip/handling profile | Resolver/profile baseline | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — cornering feel | Current Quick Tune field; preserve raw legacy on import. |
| VehicleMovementConfig.RearWheelCorneringStiffness | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Grip/handling profile | Resolver/profile baseline | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — cornering feel | Current Quick Tune field; preserve raw legacy on import. |
| VehicleMovementConfig.FrontWheelLoadRatio | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Wheel load/handling model | Vehicle context + handling profile | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — load transfer/handling | Not currently Quick Tune input; should not be hidden by 4-axis migration. |
| VehicleMovementConfig.RearWheelLoadRatio | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Wheel load/handling model | Vehicle context + handling profile | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — load transfer/handling | Not currently Quick Tune input; should not be hidden by 4-axis migration. |
| VehicleMovementConfig.FrontWheelSpringRate | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension Firmness + vehicle mass/context | Context-aware suspension resolver | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (group) + CompareVehicleData | Yes — ride/roll | Current Quick Tune field; same slider value must not imply same raw rate across masses. |
| VehicleMovementConfig.RearWheelSpringRate | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension Firmness + vehicle mass/context | Context-aware suspension resolver | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (group) + CompareVehicleData | Yes — ride/roll | Current Quick Tune field; same slider value must not imply same raw rate across masses. |
| VehicleMovementConfig.FrontWheelSpringPreload | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension Firmness + ride-height context | Context-aware suspension resolver | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — ride height/response | Current Quick Tune field; preserve explicit existing value. |
| VehicleMovementConfig.RearWheelSpringPreload | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension Firmness + ride-height context | Context-aware suspension resolver | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — ride height/response | Current Quick Tune field; preserve explicit existing value. |
| VehicleMovementConfig.FrontWheelSuspensionMaxRaise | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension travel / vehicle geometry profile | Vehicle class + suspension profile | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — bump/air behavior | Not in current Quick Tune; keep Advanced until resolver evidence. |
| VehicleMovementConfig.RearWheelSuspensionMaxRaise | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension travel / vehicle geometry profile | Vehicle class + suspension profile | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — bump/air behavior | Not in current Quick Tune; keep Advanced until resolver evidence. |
| VehicleMovementConfig.FrontWheelSuspensionMaxDrop | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension travel / vehicle geometry profile | Vehicle class + suspension profile | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — bump/air behavior | Not in current Quick Tune; keep Advanced until resolver evidence. |
| VehicleMovementConfig.RearWheelSuspensionMaxDrop | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Suspension travel / vehicle geometry profile | Vehicle class + suspension profile | Yes | None | Wheel CDO tuning when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Yes — bump/air behavior | Not in current Quick Tune; keep Advanced until resolver evidence. |
| VehicleMovementConfig.bFrontWheelAffectedByEngine | FCFVehicleMovementConfig | Derived + Profile + Definition Stored | Drive layout intent | Derive from drivetrain profile; explicit exception allowed | Yes | None | Wheel CDO engine-affect flag when detailed overrides active | ValidateMovementConfig (no direct consistency rule) | Yes — traction behavior | Must be cross-checked with DifferentialType in P0-02 resolver/validation. |
| VehicleMovementConfig.bRearWheelAffectedByEngine | FCFVehicleMovementConfig | Derived + Profile + Definition Stored | Drive layout intent | Derive from drivetrain profile; explicit exception allowed | Yes | None | Wheel CDO engine-affect flag when detailed overrides active | ValidateMovementConfig (no direct consistency rule) | Yes — traction behavior | Must be cross-checked with DifferentialType in P0-02 resolver/validation. |
| VehicleMovementConfig.FrontWheelSweepShape | FCFVehicleMovementConfig | Profile + Advanced Override + Definition Stored | Vehicle physics/contact policy | Project/vehicle profile default | Yes | None | Wheel CDO sweep shape when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Conditional — technical handling | Advanced technical field; not normal Human Intent. |
| VehicleMovementConfig.RearWheelSweepShape | FCFVehicleMovementConfig | Profile + Advanced Override + Definition Stored | Vehicle physics/contact policy | Project/vehicle profile default | Yes | None | Wheel CDO sweep shape when detailed overrides active | ValidateMovementConfig (no direct leaf rule) | Conditional — technical handling | Advanced technical field; not normal Human Intent. |
| VehicleMovementConfig.ChassisHeight | FCFVehicleMovementConfig | Asset Derived + Profile + Advanced Override + Definition Stored | Chassis geometry / physics profile | Asset/context-derived candidate; preserve legacy | Yes | None | Chaos Movement ChassisHeight always applied | ValidateMovementConfig (no direct leaf rule) | Yes — collision/handling | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.DragCoefficient | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Aero/performance profile | Resolver/profile baseline | Yes | None | Chaos Movement DragCoefficient always applied | ValidateMovementConfig (no direct leaf rule) | Yes — acceleration/top-speed | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.DownforceCoefficient | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Aero/handling profile | Resolver/profile baseline | Yes | None | Chaos Movement DownforceCoefficient always applied | ValidateMovementConfig (no direct leaf rule) | Yes — high-speed stability | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.bEnableCenterOfMassOverride | FCFVehicleMovementConfig | Derived + Advanced Override + Definition Stored | Whether explicit COM override is required | False unless deliberate COM override | Yes | None | Chaos Movement COM override enable always applied | ValidateMovementConfig | Yes — rollover/weight transfer | Prefer deriving enable state from explicit override presence. |
| VehicleMovementConfig.CenterOfMassOverride | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Chassis mass model / deliberate handling override | Zero unless approved override | Yes | None | Chaos Movement COM override always applied when enabled | ValidateMovementConfig | Yes — rollover/handling | High-risk Advanced field; keep preview + USER verification. |
| VehicleMovementConfig.EngineMaxTorque | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Performance/acceleration intent | Resolver/profile baseline | Yes | None | Chaos EngineSetup EngineMaxTorque always applied | ValidateMovementConfig + CompareVehicleData | Yes — acceleration | Current Quick Tune field. |
| VehicleMovementConfig.EngineMaxRPM | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Engine/performance profile | Resolver/profile baseline | Yes | None | Chaos EngineSetup EngineMaxRPM always applied | ValidateMovementConfig + CompareVehicleData | Yes — high-speed character | Current Quick Tune field. |
| VehicleMovementConfig.EngineIdleRPM | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Engine profile | Resolver/profile baseline | Yes | None | Chaos EngineSetup EngineIdleRPM always applied | ValidateMovementConfig (group; no direct leaf rule) | Conditional — idle behavior | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.EngineBrakeEffect | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Engine/deceleration profile | Resolver/profile baseline | Yes | None | Chaos EngineSetup EngineBrakeEffect always applied | ValidateMovementConfig (group; no direct leaf rule) | Yes — lift-off behavior | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.EngineRevUpMOI | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Engine response profile | Resolver/profile baseline | Yes | None | Chaos EngineSetup EngineRevUpMOI always applied | ValidateMovementConfig (group; no direct leaf rule) | Yes — throttle response | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.EngineRevDownRate | FCFVehicleMovementConfig | Profile + Derived + Advanced Override + Definition Stored | Engine response profile | Resolver/profile baseline | Yes | None | Chaos EngineSetup EngineRevDownRate always applied | ValidateMovementConfig (group; no direct leaf rule) | Yes — rev decay | Not gated by bUseMovementOverrides. |
| VehicleMovementConfig.DifferentialType | FCFVehicleMovementConfig | Human Intent + Profile + Definition Stored | Drive layout intent | Derive from drivetrain intent/profile | Yes | None | Chaos DifferentialSetup always applied | ValidateMovementConfig (no direct leaf rule) | Yes — traction/drive character | Primary semantic drivetrain field; cross-check wheel engine-affect flags. |
| VehicleMovementConfig.FrontRearSplit | FCFVehicleMovementConfig | Derived + Profile + Advanced Override + Definition Stored | Drive layout + drivetrain profile | Resolver from drivetrain intent | Yes | None | Chaos DifferentialSetup always applied | ValidateMovementConfig (no direct leaf rule) | Yes — traction balance | Meaning depends on DifferentialType; resolve together. |
| VehicleMovementConfig.SteeringType | FCFVehicleMovementConfig | Profile + Advanced Override + Definition Stored | Steering model policy | Project/handling profile | Yes | None | Chaos SteeringSetup always applied | ValidateMovementConfig (no direct leaf rule) | Yes — steering behavior | Technical mode; avoid normal raw user entry. |
| VehicleMovementConfig.SteeringAngleRatio | FCFVehicleMovementConfig | Derived + Advanced Override + Definition Stored | Steering agility intent/profile | Resolver/profile baseline | Yes | None | Chaos SteeringSetup always applied | ValidateMovementConfig + CompareVehicleData | Yes — steering feel | Current Quick Tune field; not gated by bUseMovementOverrides. |
| VehicleMovementConfig.bLegacyWheelFrictionPosition | FCFVehicleMovementConfig | Profile + Advanced Override + Definition Stored | Physics compatibility policy | Preserve current baseline unless deliberate migration | Yes | None | Chaos Movement compatibility flag always applied | ValidateMovementConfig (no direct leaf rule) | Conditional — regression | Treat as technical compatibility field, not gameplay intent. |
| VehicleMovementConfig.FrontWheelAdditionalOffset | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Wheel physics/layout alignment | Zero baseline; derive/override only with evidence | Yes | None | WheelSetup AdditionalOffset always applied | ValidateMovementConfig (no direct leaf rule) | Yes — wheel/physics alignment | Not gated by bUseMovementOverrides; distinguish from visual WheelAnchor pose. |
| VehicleMovementConfig.RearWheelAdditionalOffset | FCFVehicleMovementConfig | Asset Derived + Advanced Override + Definition Stored | Wheel physics/layout alignment | Zero baseline; derive/override only with evidence | Yes | None | WheelSetup AdditionalOffset always applied | ValidateMovementConfig (no direct leaf rule) | Yes — wheel/physics alignment | Not gated by bUseMovementOverrides; distinguish from visual WheelAnchor pose. |

### 4.9 WheelVisualConfig

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| WheelVisualConfig.bUseWheelVisualOverrides | FCFVehicleWheelVisualConfig | Derived + Definition Stored | Authoring readiness/state | False legacy; set only for authored WheelVisual policy | Yes | None | Not a runtime gate; values are still consumed | ValidateWheelVisualConfig | Yes — visual | Do not use as master enable in Resolver. |
| WheelVisualConfig.ExpectedWheelCount | FCFVehicleWheelVisualConfig | Derived + Profile + Advanced Override + Definition Stored | Vehicle wheel layout/profile | 4 default; derive from supported vehicle topology when possible | Yes | None | WheelSync expected count | ValidateWheelVisualConfig | Yes — wheel sync | P0 currently assumes common 4-wheel assets but field remains explicit. |
| WheelVisualConfig.FrontWheelCountForSteering | FCFVehicleWheelVisualConfig | Derived + Profile + Advanced Override + Definition Stored | Steering axle topology | 2 default | Yes | None | WheelSync steering visual count | ValidateWheelVisualConfig | Yes — steering visual | Resolve from steering topology, not arbitrary raw entry. |
| WheelVisualConfig.bAutoScaleWheelMeshToRadius | FCFVehicleWheelVisualConfig | Human Intent + Profile + Definition Stored | Wheel asset normalization policy | False legacy; opt in | Yes | None | Pawn wheel mesh auto-scale | ValidateWheelVisualConfig + CompareVehicleData | Yes — wheel size | Keep explicit because some assets rely on manual scale. |
| WheelVisualConfig.WheelMeshRadiusMeasureMode | FCFVehicleWheelVisualConfig | Asset Derived + Profile + Advanced Override + Definition Stored | Wheel mesh orientation/bounds | AutoMaxXZ | Yes | None | Pawn mesh bounds radius measurement | ValidateWheelVisualConfig (no direct enum rule) | Yes — wheel size | Can be auto-suggested from asset convention, explicit override allowed. |
| WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin | FCFVehicleWheelVisualConfig | Asset Derived + Profile + Advanced Override + Definition Stored | Wheel mesh pivot/bounds quality | True when auto-scale active | Yes | None | Pawn relative-location center correction | ValidateWheelVisualConfig (no direct leaf rule) | Yes — wheel alignment | Do not alter manual placement without diff/visual review. |
| WheelVisualConfig.WheelMeshScaleClampMin | FCFVehicleWheelVisualConfig | Profile + Advanced Override + Definition Stored | Safety policy | 0.25 current baseline | Yes | None | Pawn auto-scale clamp | ValidateWheelVisualConfig + CompareVehicleData | Conditional — abnormal scale | Safety guard, not normal vehicle personality input. |
| WheelVisualConfig.WheelMeshScaleClampMax | FCFVehicleWheelVisualConfig | Profile + Advanced Override + Definition Stored | Safety policy | 4.0 current baseline | Yes | None | Pawn auto-scale clamp | ValidateWheelVisualConfig + CompareVehicleData | Conditional — abnormal scale | Safety guard, not normal vehicle personality input. |

### 4.10 VehicleReferenceConfig

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| VehicleReferenceConfig.FrontWheelClass | FCFVehicleReferenceConfig | Profile + Advanced Override + Definition Stored | Vehicle/drivetrain wheel class selection | Explicit/profile class reference | Yes | None | Pawn WheelSetup front class | ValidateRequiredReferences | Yes — front wheel physics | Prefer profile resolution; preserve explicit legacy class. |
| VehicleReferenceConfig.RearWheelClass | FCFVehicleReferenceConfig | Profile + Advanced Override + Definition Stored | Vehicle/drivetrain wheel class selection | Explicit/profile class reference | Yes | None | Pawn WheelSetup rear class | ValidateRequiredReferences | Yes — rear wheel physics | Prefer profile resolution; preserve explicit legacy class. |

### 4.11 Durability / Defense / FX

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| VehicleDurabilityConfig.MaxHealth | FCFVehicleDurabilityConfig | Profile + Advanced Override + Definition Stored | Vehicle durability/balance intent | 100 current default | Yes | None | VehicleHealthComp max integrity | No direct UCFVDAValidator rule | Yes — combat durability balance | Do not merge with DefenseData shield/armor pools. |
| DefaultDefenseData | UCFVehicleData | Human Intent + Profile + Advanced Override + Definition Stored | Vehicle defense archetype/default loadout | None preserves legacy health-only path | Yes | Default defense may be overridden/removed by Fitting | VehicleDefenseComp / Fitting default | Fitting snapshot/domain contract; no direct VDA rule | Yes — combat balance | Definition default only; installed Defense item remains Fitting/Inventory responsibility. |
| DefaultDestroyedFxData | UCFVehicleData | Profile + Advanced Override + Definition Stored | Vehicle FX style/profile | None allowed | Yes | None | CombatFxComp on destroyed transition | CombatFxData runtime configuration; no direct VDA rule | Yes — destroyed visual | Do not make FX presence a destruction-validity requirement. |
| DestroyedFxSocketName | UCFVehicleData | Asset Derived + Profile + Advanced Override + Definition Stored | ChassisMesh FX socket convention/inspection | FX_Destroyed; runtime bounds fallback if missing | Yes | None | CombatFxComp socket-first destroyed transform | No direct VDA rule; runtime fallback | Yes — destroyed FX placement | Candidate for asset-derived validation/capture, but missing socket is currently valid fallback. |

### 4.12 DriveStateConfig

| Field Path | Current Owner | Authoring Classification | Input Source | Default Strategy | Definition Storage | Fitting Impact | Runtime Consumer | Validation Owner | USER Verification Need | Migration Note |
|---|---|---|---|---|---|---|---|---|---|---|
| DriveStateConfig.bUseDriveStateOverrides | FCFVehicleDriveStateConfig | Derived + Definition Stored | Whether vehicle-specific DriveState profile is authored | False legacy | Yes | None | VehicleDriveComp real apply gate | ValidateDriveStateConfig + CompareVehicleData | Yes — state behavior | Unlike WheelVisual flag, this truly gates config copy. |
| DriveStateConfig.bEnableDriveStateHysteresis | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | DriveState stability policy | True | Yes | None | VehicleDriveComp transition logic | ValidateDriveStateConfig (no direct bool rule) | Yes — state flicker | Keep in DriveState profile, not Driving Feel raw fields. |
| DriveStateConfig.bUsePerStateHoldTimes | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | DriveState stability policy | True | Yes | None | VehicleDriveComp hold-time selection | ValidateDriveStateConfig (group) | Yes — transition timing | If false, per-state values are stored but inactive. |
| DriveStateConfig.DriveStateMinimumHoldTimeSeconds | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | DriveState timing profile | 0.12 | Yes | None | VehicleDriveComp transition hold | ValidateDriveStateConfig (hold-time aggregate) | Yes — transition timing | Do not infer USER PASS from valid non-negative range. |
| DriveStateConfig.IdleStateMinimumHoldTimeSeconds | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | DriveState timing profile | 0.10 | Yes | None | VehicleDriveComp transition hold | ValidateDriveStateConfig (hold-time aggregate) | Yes — transition timing | Do not infer USER PASS from valid non-negative range. |
| DriveStateConfig.ReversingStateMinimumHoldTimeSeconds | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | DriveState timing profile | 0.18 | Yes | None | VehicleDriveComp transition hold | ValidateDriveStateConfig (hold-time aggregate) | Yes — transition timing | Do not infer USER PASS from valid non-negative range. |
| DriveStateConfig.AirborneStateMinimumHoldTimeSeconds | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | DriveState timing profile | 0.08 | Yes | None | VehicleDriveComp transition hold | ValidateDriveStateConfig (hold-time aggregate) | Yes — transition timing | Do not infer USER PASS from valid non-negative range. |
| DriveStateConfig.IdleEnterSpeedThresholdKmh | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Idle hysteresis profile | 0.75 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig | Yes — stop/low-speed state | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.IdleExitSpeedThresholdKmh | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Idle hysteresis profile | 1.5 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig | Yes — stop/low-speed state | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.ReverseEnterSpeedThresholdKmh | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Reverse hysteresis profile | 1.25 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig + CompareVehicleData | Yes — reverse state | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.ReverseExitSpeedThresholdKmh | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Reverse hysteresis profile | 0.75 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig | Yes — reverse state | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.AirborneMinSpeedThresholdKmh | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Airborne detection profile | 3.0 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig + CompareVehicleData | Yes — bump/jump state | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.AirborneVerticalSpeedThresholdCmPerSec | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Airborne detection profile | 100 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig | Yes — bump/jump state | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.ActiveInputThreshold | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Input dead-zone/state policy | 0.05 | Yes | None | VehicleDriveComp state evaluation | ValidateDriveStateConfig | Yes — input/state response | Resolve as one coherent DriveState profile; preserve legacy explicit values. |
| DriveStateConfig.bTreatOppositeThrottleAsBrake | FCFVehicleDriveStateConfig | Profile + Advanced Override + Definition Stored | Input/drive-state interpretation policy | True | Yes | None | VehicleDriveComp state interpretation | ValidateDriveStateConfig (no direct bool rule) | Yes — braking/reverse intent | Do not conflate with actual brake torque or input binding. |

### 4.13 P0-01 Ownership 판정

이번 Matrix로 다음 책임을 확정한다.

1. **Recipe는 `VehicleMovementConfig` 47개 Raw Field의 복제품이 되어서는 안 된다.**
   - 일반 Authoring 입력은 Driving Feel, Drivetrain, Vehicle Context, Asset Selection 같은 상위 의도를 사용한다.
   - Raw Chaos 값은 Profile / Derived / Advanced Override 결과로 Definition에 저장하는 방향이 적합하다.

2. **Asset 선택과 Asset Derived를 구분한다.**
   - `VehicleVisualConfig.*Mesh`는 사람이 고르거나 Profile이 제안할 수 있는 Asset Reference다.
   - Wheel Anchor, Hardpoint Local Transform, Wheel Radius/Width 후보값은 실제 Asset에서 계산 가능한 Asset Derived다.
   - Asset Derived 결과에도 Explicit Override 경로가 필요하다.

3. **Fitting Derived는 Vehicle Authoring이 소유하지 않는다.**
   - `BaseVehicleMassKg`, `MaximumGrossMassKg`, Mount compatibility/default는 Definition Stored다.
   - 실제 Equipment/Ammo/Defense가 합쳐진 `TotalVehicleMassKg`, Installed Set, Resolved Equipment는 Fitting Derived다.

4. **세 종류의 Override flag를 하나의 공통 의미로 합치지 않는다.**
   - `VehicleMovementConfig.bUseMovementOverrides` = Wheel Runtime 상세 튜닝 + `ThrottleInputScale` gate.
   - `WheelVisualConfig.bUseWheelVisualOverrides` = 현재 Runtime gate가 아닌 authoring/readiness 성격.
   - `DriveStateConfig.bUseDriveStateOverrides` = 실제 `VehicleDriveComp` config copy gate.

5. **Current Quick Tune의 소유 범위와 전체 Movement 필드를 분리한다.**
   - 기존 4축 Quick Tune이 직접 쓰는 일부 필드는 Context-aware Resolver migration 대상이다.
   - Brake, wheel geometry, suspension travel, drivetrain, sweep shape, aero, COM 등 나머지 필드는 각각 다른 Source/Default 전략이 필요하다.

6. **Legacy MountProfile 직렬화 필드는 새 Authoring 입력에서 제외한다.**
   - `TurretYawMesh`부터 `MountWeightKg`까지 legacy 필드는 기존 Asset 직렬화 보존만 수행한다.
   - 현재 Runtime/피팅은 `DefaultEquipmentPresetData` 내부 `TurretMountData` / `WeaponData`를 사용한다.
   - 삭제는 P0-01에서 하지 않으며 별도 asset migration evidence 이후에만 검토한다.

7. **현재 저장되지만 production consumer가 없는 필드를 숨겨서 존재하지 않는 것처럼 처리하지 않는다.**
   - `HardpointSlots[].LocationCategory`
   - `MountProfiles[].bExposedModule`
   - 위 필드는 현재 의미 있는 Definition metadata/future contract이므로 Matrix에 보존하되 Current Runtime 효과를 과장하지 않는다.

8. **Validator Coverage와 Authoring Classification은 별개다.**
   - `UCFVDAValidator`가 직접 검사하지 않는 필드가 다수 존재한다.
   - 특히 legacy Mount fields, `LocationCategory`, `bExposedModule`, Durability/Defense/FX 일부, Movement 세부값은 별도 Domain/Resolver validation 또는 future VDA 보강 후보가 된다.
   - P0-01에서는 새 Validator를 구현하지 않는다.

### 4.14 P0-02로 넘기는 Architecture Freeze 입력

P0-01 결과상 P0-02에서 최소 다음 결정을 내려야 한다.

```text
1. Vehicle Recipe의 최소 Human Intent 필드
2. Vehicle Base / Drivetrain / Handling / Performance / DriveState Profile 분할 수
3. Asset Derived Resolver가 계산할 정확한 필드
4. Asset Derived 결과의 Explicit Override 표현
5. bUseMovementOverrides / bUseWheelVisualOverrides / bUseDriveStateOverrides의 Authoring 표현
6. Existing Definition Import에서 Legacy Explicit Baseline으로 보존할 범위
7. MovementProfileName을 정식 Source Tracking과 병행/대체할지 여부
8. legacy MountProfile serialized field retirement 정책
9. LocationCategory / bExposedModule의 P0 Authoring 노출 여부
10. UCFVDAValidator 직접 coverage를 어디까지 확장할지
11. Source Tracking 저장 위치와 Stale 판정
12. Apply Transaction에서 Aggregate Struct 전체가 아니라 Field Diff를 어떻게 보존할지
```

P0-02 전에는 위 판정을 코드 구조로 고정하거나 Source / Content Asset을 변경하지 않는다.

---

## 5. Recipe 설계 원칙

Recipe는 Raw VehicleData 복제품이 아니어야 한다.

좋지 않은 예:

```text
Recipe.EngineMaxTorque
Recipe.EngineMaxRPM
Recipe.SpringRate
Recipe.Friction
Recipe.SteeringAngle
...
```

이런 구조는 VehicleData를 한 번 더 복제할 뿐이다.

권장 예:

```text
Vehicle Archetype
Drive Layout
Handling Character
Performance Bias
Requested Base Mass
Equipment / Armor Intent
Asset Selection
Explicit Overrides
```

Recipe의 목표는 **의도를 최소 필드로 표현하는 것**이다.

---

## 6. Profile 설계 원칙

Profile은 다음 조건을 만족해야 한다.

- 재사용 가능
- Source 추적 가능
- 변경 영향 추적 가능
- Definition과 분리 가능
- 여러 Profile 합성 시 우선순위가 명확함

초기 합성 예:

```text
Vehicle Base Profile
+ Drivetrain Profile
+ Handling Profile
+ Performance Profile
+ Explicit Vehicle Overrides
```

하지만 Profile 수가 지나치게 잘게 분해되어 조합 폭발을 만들지 않도록 P0-02에서 실제 VehicleData 필드 기준으로 최소 구조를 결정한다.

---

## 7. Resolver 설계 원칙

Resolver는 UI와 분리한다.

금지:

```text
Slate Slider 값
→ UI cpp 안에서 FMath::Lerp
→ VehicleData 직접 수정
```

권장:

```text
Authoring Input
→ Resolver
→ FResolvedVehicleAuthoringData
→ Preview
→ Validation
→ Apply Service
```

Resolver는 다음 특성을 가져야 한다.

- Deterministic
- 동일 입력은 동일 결과
- Source 추적 가능
- Preview 가능
- Asset Mutation 없이 계산 가능
- Validation 전 적용하지 않음
- USER 주행감 판정을 포함하지 않음

---

## 8. Source Tracking

최종 값이 어디서 왔는지 보여주는 것이 중요하다.

후보 Source 종류:

```text
Explicit Vehicle Input
Base Profile
Handling Profile
Performance Profile
Asset Derived
Rule Derived
Manual Override
Legacy Imported Value
```

후보 표현:

```text
FieldPath
ResolvedValue
SourceType
SourceId
IsOverride
IsStale
```

이 데이터가 Definition에 영구 저장되어야 하는지, Editor 전용 Metadata로 유지할지는 P0-02 이후 결정한다.

---

## 9. Regeneration / Stale Model

Profile이 바뀌더라도 Definition을 즉시 자동 갱신하지 않는 방식을 기본안으로 한다.

```text
Profile Revision 변경
→ Affected Definition 탐색
→ Stale 표시
→ Regenerate Preview
→ Diff
→ Validate
→ Apply
```

필요 데이터 후보:

```text
LastResolvedProfileRevision
AuthoringRecipeRevision
ResolvedHash
```

단, 저장 필드 추가 자체는 P0-02 이전에 하지 않는다.

---

## 10. Override 모델

Override는 단순히 "현재 값이 Profile과 다름"으로 추정하지 않는 것이 좋다.

가능하면 명시적으로 관리한다.

UI 예:

```text
Steering Angle
Base Source: Responsive Steering Profile = 32°
Override: 36°
Resolved: 36°
```

Override 삭제 시 Profile 결과로 복귀할 수 있어야 한다.

---

## 11. Existing Definition Import

이미 존재하는 VehicleData를 새 Authoring 체계로 가져올 때 모든 값을 역산해 Recipe를 완벽히 복구할 수 있다고 가정하지 않는다.

현재 Quick Tune의 역산처럼 여러 Raw 값을 평균해 의도를 추정하면 원래 의미를 잃을 수 있다.

따라서 Migration 후보:

```text
Existing VehicleData
→ Import as Legacy Explicit Baseline
→ 가능한 필드만 Profile Match 제안
→ 확인되지 않은 값은 Explicit / Legacy Override로 보존
```

새 시스템이 기존 차량 값을 임의로 재해석해 바꾸지 않게 한다.

---

## 12. Vehicle DA Wizard Migration Matrix

### 12.1 Target / Source

현재 기능: 대상 DA / 비교 DA 지정.

처리: 유지 가치 높음.
장기적으로 Data Browser의 Selection + Compare 기능으로 흡수.

### 12.2 Validator

현재 기능: `UCFVDAValidator` 실행.

처리: 그대로 재사용.
새 병렬 Validator를 만들지 않는 것을 기본안으로 한다.

### 12.3 Layout Capture

현재 기능: Chassis Socket에서 Layout을 VehicleData로 캡처.

처리: Asset Derived Authoring Action으로 흡수.

### 12.4 Driving Feel Quick Tune

현재 기능:

```text
4개 Feel Slider
→ 고정 Min/Max Lerp
→ Raw MovementConfig
```

처리:

```text
Feel Intent
→ Vehicle Context-aware Resolver
→ Preview / Diff
→ Apply
```

으로 재설계 후보.

### 12.5 Undo / Revert

현재 기능: `FScopedTransaction`, Target Load 시 기준 Snapshot.

처리: 유지.
새 Apply Service도 Unreal Transaction을 사용해야 한다.

### 12.6 Open Raw DA

처리: 유지.
Advanced / Debug 경로로 제공.

---

## 13. Driving Feel 상세 설계 방향

현재 4개 축은 첫 번째 후보로 보존한다.

```text
Acceleration Feel
Steering Agility
Grip Feel
Suspension Firmness
```

후속 후보:

```text
Braking Response
High-speed Stability
Throttle Response
Weight Transfer Character
```

후속 축은 P0 초기 범위에 자동 추가하지 않는다.

### Context 후보

```text
Base Vehicle Mass
Wheelbase
Track Width
Drive Layout
Wheel / Tire Class
Vehicle Archetype
Current Fitting Mass (Preview only)
```

Current Fitting Mass는 Authoring Input의 소유 데이터가 아니라 Preview / Test Context로만 소비한다.

---

## 14. Inventory Integration Contract

Authoring System에서 생성한 Definition을 Inventory가 참조한다.

Inventory가 보관해야 할 데이터와 Authoring 데이터는 분리한다.

```text
Authoring Recipe
X Inventory 저장 대상 아님

Resolved Definition Ref / Stable ID
O Inventory Instance에서 사용

Instance Durability / Quantity / Ammo State
O Inventory 소유
```

향후 Multiplayer에서도 static Definition 전체를 Instance마다 복제하기보다 Definition Ref + Instance State 구조를 유지하는 방향이 적합하다.

---

## 15. Fitting Integration Contract

Fitting은 다음을 소비할 수 있다.

- Definition Category
- Compatibility
- Hardpoint / Mount Profile
- Static Equipment Mass
- 제한 조건

Fitting은 다음을 Authoring에게 돌려줄 수 있다.

- Installed Set
- Fitted Mass
- Resolved Equipment Configuration

그러나 Authoring은 Fitting State의 owner가 아니다.

테스트 / Preview에서 다음 형태는 허용한다.

```text
Vehicle Authoring Preview
Test Context = Selected Fitting Snapshot
```

이 경우에도 Definition 생성 자체와 Fitting 결과를 혼합 저장하지 않는다.

---

## 16. AI Authoring Interface 후보

AI가 Editor UI를 클릭하는 것과 별개로, 최종적으로 동일 Authoring Contract를 호출할 수 있어야 한다.

후보 operation 개념:

```text
ListDefinitions
ReadDefinition
ListProfiles
BuildRecipePreview
ResolveVehicle
ValidateResolvedVehicle
DiffAgainstDefinition
ApplyResolvedVehicle
```

이 목록은 초기 후보였으며, **v0.7.0부터 실제 P0 AI Authoring operation 의미 Authority는 Section 25**다.
P0-08 구현 시 C++/transport 함수명은 Section 25의 typed operation 분리를 보존하는 범위에서 조정할 수 있다.

원칙:

- UI와 AI가 같은 Resolver를 사용
- AI 전용 계산 로직 금지
- AI 전용 Raw Field 우회 쓰기 금지
- Apply 전 Preview / Validation 가능
- Undo 가능한 Editor Transaction 사용

---

## 17. Data Browser 후보 구조

대량 비교 문제는 단일 Asset Editor가 아니라 Browser 성격의 UI가 필요하다.

후보 컬럼:

```text
Definition ID
Display Name
Type
Status
Base Profile
Mass
Drive Layout
Handling
Validation
Stale
Overrides
```

후보 기능:

- Search
- Filter
- Sort
- Multi Selection
- Compare
- Changed-only
- Validation Error Filter
- Missing Definition / Mesh-only 후보 표시

Mesh-only 차량은 `Unregistered / Visual-only Candidate`로 별도 표기할 수 있다.
실제 등록 규칙은 Foundation Audit 후 결정한다.

---

## 18. Excel / CSV Integration Design

Excel / CSV에 적합한 데이터:

- 반복 Numeric Balance Value
- Display / Economy Value
- Profile 계수
- 비교용 Derived Report

부적합한 데이터:

- UObject hard reference
- 복잡한 nested struct
- Asset-specific layout
- Runtime state

원칙:

```text
Export
→ Edit
→ Import Preview
→ Diff
→ Validate
→ Apply
```

CSV Import가 즉시 Asset을 대량 수정하는 구조는 기본안으로 사용하지 않는다.

---

## 19. Validation Layers

검증은 최소 다음 계층으로 분리할 수 있다.

```text
Authoring Input Validation
Resolver Validation
Definition Validation
Fitting Compatibility Validation
Runtime Technical Validation
USER Acceptance
```

기존 `UCFVDAValidator`는 Definition Validation의 중심으로 유지한다.

각 검증 계층을 섞어 "Technical PASS = 좋은 주행감"으로 해석하지 않는다.

---

## 20. 구현 배분 원칙

현재 예상 C++ / UI 역할:

### C++ Editor / Shared Logic

- Asset enumeration
- Reflection 기반 field access
- Resolver
- Source tracking
- Validation orchestration
- Diff
- Transaction / Apply
- Import / Export

### Slate / Editor UI

- Browser
- Table / Column
- Recipe Form
- Compare
- Driving Feel UI
- Validation Result

Blueprint / Editor Utility Widget는 빠른 프로토타입이 필요한 경우 보조적으로 사용할 수 있지만, 대량 Table / Diff / Reflection / Transaction 핵심 로직은 C++에 두는 방향을 우선 검토한다.

P0 Production UI 기술은 **Section 24에서 `CarFight_ReEditor` C++ Slate로 확정**되었다.
이 Section의 역할 배분 원칙은 유지하되 실제 UX/Slate Authority는 Section 24를 따른다.

---

## 21. DAUTH-P0-02 Authoring Contract Freeze

### 21.1 Freeze 상태

DAUTH-P0-01의 126행 Field Ownership Matrix를 입력으로 다음 P0 Authoring Contract를 동결한다.

```text
DAUTH-P0-02 = Contract Freeze Complete
다음 단계 = DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design
Project 상태 = Working / Pre-Implementation 유지
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
```

P0-02 PASS는 구현 착수 승인이 아니다.
`P0-03~07`은 이 계약을 구현 가능한 상세 구조와 UX/API로 구체화하는 설계 구간이며, 실제 신규 Authoring Source 구현은 `P0-08`부터 시작한다.

### 21.2 Authority / SSOT Contract

Authoring과 Runtime은 서로 다른 종류의 Truth를 소유한다.

```text
Authoring Intent Truth
= Persistent Recipe + Profile Binding + Override + Source Tracking

Resolved Runtime Definition Truth
= UCFVehicleData

Installed / Instance Truth
= Inventory + Fitting

Runtime Mutable State
= Runtime Components
```

규칙:

1. Recipe/Profile은 `UCFVehicleData`의 Raw 복제품이 아니다.
2. `UCFVehicleData`는 계속 Runtime과 Fitting이 소비하는 Canonical Resolved Definition이다.
3. Recipe/Profile/Source Tracking은 Runtime 소비를 요구하지 않는 Authoring 전용 데이터다.
4. Inventory/Fitting은 Recipe/Profile을 알 필요가 없으며 기존 `UCFVehicleData` 계약만 소비한다.
5. Authoring Metadata를 넣기 위해 P0에서 `UCFVehicleData` Runtime 구조를 확장하지 않는다.

따라서 이 구조는 SSOT 중복이 아니다.
Recipe/Profile은 **왜 그 값인지**를 소유하고, VehicleData는 **현재 적용된 값이 무엇인지**를 소유한다.

### 21.3 Vehicle Recipe Contract

Vehicle Recipe는 **영구 보존되는 Authoring Record**로 사용한다.

P0-03에서 실제 UObject / UStruct / DataAsset 타입을 결정하지만 다음 의미 계약은 변경하지 않는다.

Recipe가 소유하는 최소 그룹:

```text
Authoring Identity / Target Binding
Vehicle Archetype Intent
Vehicle Asset Selection
5개 Profile Binding
Driving Feel Intent 4축
선택적 Semantic Vehicle Input
Hardpoint / Mount Intent
Default Equipment / Defense / Destroyed FX Intent
Wheel Visual Policy
DriveState Mode
Explicit Advanced Leaf Overrides
Import / Legacy Adoption State
```

현재 보존할 Driving Feel Intent는 다음 4개뿐이다.

```text
Acceleration Feel
Steering Agility
Grip Feel
Suspension Firmness
```

Braking Response, High-speed Stability, Weight Transfer 등 추가 축은 P0-03 설계 근거 없이 Recipe 필수 입력으로 늘리지 않는다.

Recipe에 직접 복제하지 않는 대표 데이터:

```text
VehicleMovementConfig 47개 Raw Field 전체
Wheel Anchor Transform
Hardpoint Local Transform
Fitted TotalVehicleMassKg
Installed Equipment Set
Inventory Instance State
Runtime Health / Shield / Ammo / Drive State
legacy MountProfile serialized field
```

차량의 Base/Gross Mass, Drive Layout, Hardpoint/Mount 구조, Default Equipment/Defense/FX처럼 실제 의미가 사용자의 의도인 값은 Recipe의 Semantic Input 또는 Profile Selection으로 표현할 수 있다.
정확한 C++ 필드 형태는 P0-03에서 결정한다.

### 21.4 Profile Contract — P0 5 Domain Fixed

P0 Vehicle Authoring의 재사용 Profile Domain은 **5개로 동결**한다.

| Profile Domain | 주 책임 |
|---|---|
| Vehicle Base Profile | 차량 플랫폼 공통 정책, 질량/내구도/기본 참조·안전 정책 등 다른 Domain이나 Asset Derived가 소유하지 않는 플랫폼 기본값 |
| Drivetrain Profile | 구동 방식, Differential, Front/Rear Split, 구동 휠 정책, Wheel Class 계열 |
| Handling Profile | Steering, Grip, Wheel Load, Suspension, Brake/Handbrake, Wheel Contact, COM 관련 기본 정책 |
| Performance Profile | Throttle, Engine 성능/응답, Drag/Downforce 등 성능 계열 |
| DriveState Profile | DriveState hysteresis, hold time, state threshold, input interpretation |

Profile 규칙:

1. 한 Recipe는 위 5개 Domain Binding을 가진다. UI가 기본 Profile을 자동 선택할 수는 있지만 Source Trace에는 실제 선택 Profile이 남아야 한다.
2. P0 Profile은 **flat composition**을 기본으로 하며 Profile→Profile 상속 트리를 만들지 않는다.
3. 하나의 VehicleData leaf field는 최대 하나의 Profile Domain을 Primary Owner로 가진다.
4. 같은 field를 여러 Profile이 덮는 implicit last-write-wins는 금지한다.
5. Asset 선택/Asset Derived/Hardpoint 구조처럼 Profile이 적합하지 않은 field는 Non-Profile Source로 둔다.
6. Profile은 Definition을 직접 수정하지 않고 Resolver input만 제공한다.
7. Profile은 persistent + revisioned Authoring Source여야 한다. 실제 저장 클래스는 P0-03에서 결정한다.

Profile 수를 더 잘게 쪼개는 것은 P0 범위에서 금지한다. 실제 Vehicle MVP에서 5 Domain이 부족하다는 증거가 있을 때만 후속 개정한다.

### 21.5 Resolver Contract

Resolver는 UI/AI와 Definition Mutation 사이의 유일한 계산 경계다.

입력:

```text
Vehicle Recipe
5개 Profile Snapshot
Asset-derived Input Snapshot
Project Compatibility Defaults
Existing Definition Snapshot (Diff용)
Legacy Pinned Baseline (Import 차량만)
Optional Fitting Snapshot (Preview/Test Context only)
```

`Optional Fitting Snapshot`은 주행 Preview를 위한 Context일 뿐 Definition 값을 생성하는 Authoring Source가 아니다.

출력:

```text
Resolved Vehicle Definition Snapshot
Per-field Source Trace
Field-level Diff against current UCFVehicleData
Authoring / Resolver Validation Result
Definition Validation Result
Source Signature / Resolved Hash
Stale / External Drift 정보
```

Resolver 규칙:

- Pure Preview가 가능해야 한다.
- Resolve 중 Asset을 수정하지 않는다.
- Resolve 중 Target `UCFVehicleData`를 수정하지 않는다.
- 같은 입력 + 같은 Resolver Contract Revision은 같은 결과를 반환해야 한다.
- USER 주행감/시각 PASS를 계산 결과로 만들지 않는다.
- Fitting 계산이나 Inventory 권한/Instance 계산을 복제하지 않는다.

실제 출력 Struct 이름과 module placement는 P0-03에서 정한다.

### 21.6 Field Source / Precedence Contract

모든 resolved leaf field는 허용된 Source 하나를 최종 owner로 가져야 한다.

P0 Source 종류:

```text
Project Compatibility Default
Vehicle Base Profile
Drivetrain Profile
Handling Profile
Performance Profile
DriveState Profile
Rule Derived
Asset Derived
Recipe Explicit Semantic Input
Legacy Imported Pinned Baseline
Advanced Leaf Override
Legacy Serialized Passthrough
```

일반 resolution precedence:

```text
1. Project Compatibility Default
2. 해당 Field의 Primary Profile
3. Rule Derived / Asset Derived
4. Recipe Explicit Semantic Input
5. Legacy Imported Pinned Baseline      [Import 차량만]
6. Advanced Leaf Override               [최고 우선순위]
```

단, 모든 Source가 모든 field에 허용되는 것은 아니다.
Field Ownership Matrix와 P0-03 Field Resolver Map이 허용 Source를 제한한다.

동일 precedence의 두 Source가 같은 field를 동시에 소유하려 하면 Resolver Error로 처리하고, 배열 순서나 호출 순서에 따른 implicit last-write-wins는 사용하지 않는다.

Legacy Pinned Baseline은 기존 Definition 보존을 위해 일반 Profile/Derived 결과보다 우선한다.
사용자가 해당 field/group을 새 Authoring Source로 **Adopt**하기 전에는 Profile 변경이 기존 차량 값을 조용히 덮지 못한다.

### 21.7 Source Tracking Storage Contract

Source Tracking은 persistent Authoring Metadata이며 **VehicleData Runtime field에 저장하지 않는다.**

Recipe와 함께 저장되거나 Recipe가 소유하는 Authoring-side record로 관리한다.
정확한 UObject 형태는 P0-03에서 결정한다.

Per-field Trace 최소 정보:

```text
Stable Field Path
Source Type
Source ID
Source Revision / Fingerprint
IsAdvancedOverride
IsLegacyPinned
Last Applied Value Hash
```

Applied Provenance 최소 정보:

```text
Recipe Revision
Profile IDs + Profile Revisions
Relevant Asset Fingerprints
Resolver Contract Revision
Last Applied Source Signature
Last Applied Resolved Definition Hash
```

Preview 중에는 실제 Resolved Value와 Shadowed/Base Source를 추가로 보여줄 수 있다.

배열 field의 Stable Field Path는 index를 identity로 사용하지 않는다.

예:

```text
HardpointSlots[LocationSlotId=Top_01].LocalLocation
MountProfiles[MountProfileId=RoofTurret_MediumOrLarge].SizeLimit
```

Array reorder가 Source Tracking identity를 깨뜨려서는 안 된다.

### 21.8 Override Contract

Override는 두 층으로 분리한다.

```text
Semantic Intent Change
= Recipe의 정상 Authoring 입력 변경

Advanced Leaf Override
= Resolved Raw leaf field를 명시적으로 덮어쓰는 예외 경로
```

Advanced Override 규칙:

1. leaf field 단위만 허용한다.
2. `VehicleMovementConfig` 전체 같은 aggregate override는 금지한다.
3. 배열은 index가 아니라 Stable ID selector를 사용한다.
4. Override에는 Base Source와 Base Resolved Value를 함께 표시할 수 있어야 한다.
5. Override를 해제하면 현재 Profile/Rule/Asset Source 결과로 되돌아간다.
6. Profile과 값이 다르다는 이유만으로 Override라고 추정하지 않는다.
7. Existing Definition의 Legacy Pin은 Advanced Override와 별도 Source Type이다.
8. legacy MountProfile serialized field에는 새 Advanced Override를 만들지 않는다.

### 21.9 Existing Override Flag Authoring Contract

기존 flag의 Runtime 의미가 다르므로 Authoring에서 같은 추상화로 묶지 않는다.

#### `VehicleLayoutConfig.bUseLayoutOverrides`

새 managed Vehicle에서는 유효한 Layout이 Resolve/Adopt되었는지를 Resolver가 결정하는 derived output으로 취급한다.
Legacy Import는 기존 값을 pin한다.

#### `VehicleMovementConfig.bUseMovementOverrides`

새 managed Vehicle에서는 gated Wheel Runtime detail / `ThrottleInputScale`을 Resolver가 실제 소유할 때 True가 되는 derived output으로 취급한다.
전체 Movement master switch로 노출하지 않는다.
Legacy Import는 기존 값을 pin한다.

#### `WheelVisualConfig.bUseWheelVisualOverrides`

현재 Runtime gate가 아니므로 기능 On/Off 입력으로 사용하지 않는다.
새 managed Vehicle에서 WheelVisual authoring block이 정식 Resolve/Adopt되었음을 나타내는 readiness output으로만 유지한다.
Legacy Import는 기존 값을 보존한다.

#### `DriveStateConfig.bUseDriveStateOverrides`

이 flag는 실제 Runtime gate이므로 Recipe 수준에서 다음 semantic mode로 표현한다.

```text
Project Default DriveState
Vehicle-specific DriveState
```

`Project Default`이면 Resolver output은 False,
`Vehicle-specific`이면 선택된 DriveState Profile을 Resolve하고 True를 출력한다.

실제 enum/type 이름은 P0-03에서 결정한다.

### 21.10 Asset Derived Contract

P0에서 Asset Derived를 자동화할 범위를 보수적으로 제한한다.

#### Authoritative Asset Derived — P0 허용

```text
VehicleLayoutConfig.WheelAnchor*.RelativeLocation
VehicleLayoutConfig.WheelAnchor*.RelativeRotation
HardpointSlots[].LocalLocation      [SocketName이 명시되고 실제 소켓이 있을 때]
HardpointSlots[].LocalRotation      [SocketName이 명시되고 실제 소켓이 있을 때]
```

현재 Layout Capture와 동일한 Chassis Socket 근거가 있는 항목이다.

#### Measurement-assisted Candidate — Preview 필요

```text
VehicleMovementConfig.FrontWheelRadius
VehicleMovementConfig.RearWheelRadius
VehicleMovementConfig.FrontWheelWidth
VehicleMovementConfig.RearWheelWidth
WheelVisualConfig.WheelMeshRadiusMeasureMode 제안
```

Mesh Bounds 측정은 **후보값**을 만들 수 있지만 P0에서는 기존 Definition을 silent auto-apply하지 않는다.
정확한 width axis/mesh convention과 측정 rule은 P0-03에서 결정하고 Preview/Diff를 거친다.

#### P0 자동 Asset Derived에서 제외

```text
VehicleMovementConfig.ChassisHeight
VehicleMovementConfig.CenterOfMassOverride
VehicleMovementConfig.FrontWheelAdditionalOffset
VehicleMovementConfig.RearWheelAdditionalOffset
DestroyedFxSocketName의 임의 자동 선택
```

이 값들은 Mesh만 보고 안전하게 정답을 정하기 어렵다.
`DestroyedFxSocketName`은 현재 `FX_Destroyed` default + Runtime fallback 계약을 유지하며 존재 여부를 Validation/Preview로 보여주는 방향을 사용한다.

### 21.11 Existing Definition Import Contract

기존 `UCFVehicleData`는 새 Authoring System 도입 때문에 자동 변환하지 않는다.

Import는 명시적 Authoring Action이다.

```text
Unmanaged Existing Definition
→ Read-only Snapshot
→ Legacy Imported Recipe 생성
→ Current Definition leaf values를 Legacy Pinned Baseline으로 보존
→ Profile Match Suggestion 생성 가능
→ Preview
→ 사용자가 field/group을 Adopt
→ Partially Managed
→ 모든 필요한 Source가 채택되면 Managed
```

규칙:

1. Import 자체는 Target VehicleData를 수정하지 않는다.
2. 기존 값의 원래 Intent를 역산했다고 주장하지 않는다.
3. Current Quick Tune Slider 역산 결과는 authoritative Intent로 사용하지 않는다.
4. Profile과 현재 값이 우연히 같아도 자동으로 Profile-owned로 전환하지 않는다. Match는 Suggestion일 뿐이다.
5. Legacy Pin은 사용자가 Adopt하기 전까지 기존 값을 보존한다.
6. legacy MountProfile serialized field는 `Legacy Serialized Passthrough`로 보존하고 새 Authoring UI의 정상 입력 대상에서 제외한다.
7. 일부 field만 Adopt한 상태를 허용한다. 전체 차량을 한 번에 재생성하도록 강제하지 않는다.

### 21.12 Stale / Regenerate / External Drift Contract

Authoring Source 변경은 Definition을 자동 수정하지 않는다.

```text
Source 변경
→ Stale 판정
→ Regenerate Preview
→ Field Diff
→ Validation
→ Explicit Apply
```

Stale Source Signature에는 최소 다음이 참여한다.

```text
Recipe Revision
사용 중인 Profile Revision
관련 Asset Fingerprint
Resolver Contract Revision
Legacy Pin / Override State
```

Stale은 dependency 기반으로 field 단위 추적 가능해야 한다.

예:

- Handling Profile 변경 → Handling-owned field만 stale.
- ChassisMesh socket 변경 → 해당 Asset Derived Layout field만 stale.
- Legacy Pinned field → Profile 변경만으로 stale/overwrite되지 않음.
- Resolver Contract Revision 변경 → 해당 Resolver-owned field를 재검토 대상으로 표시.

Authoring Apply 이후 누군가 Raw DA Editor에서 `UCFVehicleData`를 직접 수정해 `Last Applied Resolved Definition Hash`와 달라지면 `External Drift`로 판정한다.
External Drift를 Source Tracking에 자동 흡수하지 않는다.
사용자는 Re-import/Rebase 또는 Authoring 결과 재적용 중 하나를 선택해야 한다.

### 21.13 Definition Identity / AssetManager Contract

P0에서는 `UCFVehicleData`에 새 Stable Definition ID field를 추가하지 않는다.

```text
Runtime Definition Identity
= 현재 UPrimaryDataAsset / Unreal Asset identity 유지

Authoring Recipe Identity
= 별도 Authoring identity
```

Recipe가 Target Definition을 참조하는 실제 Soft/Object/PrimaryAsset 방식은 P0-03에서 정한다.

P0에서 금지:

- rename-independent ID 필요성을 추정해 VehicleData에 ID field 추가
- 기존 AssetManager 계약 변경
- Inventory/Fitting Definition ID 체계와 Authoring Recipe ID 통합

실제 rename-independent cross-system stable ID 요구가 확인되면 별도 migration으로 다룬다.

Data Browser Index가 필요하면 **재생성 가능한 cache**만 허용하며 Authoring SSOT로 사용하지 않는다.

### 21.14 Apply Transaction Contract

Apply는 Resolver와 분리된 명시적 mutation 단계다.

표준 순서:

```text
Resolve Preview
→ Field Diff
→ Authoring / Resolver Validation
→ UCFVDAValidator Definition Validation
→ Apply Eligibility 판정
→ Begin Single Logical Editor Transaction
→ Target Definition + Authoring Applied-State Modify
→ Changed Field만 적용
→ Revalidate mutated candidate
→ PASS: Source Tracking Applied Snapshot 갱신 + Package Dirty + Commit
→ FAIL: Pre-Apply Snapshot 복원 + Abort
```

규칙:

1. Apply 전 Preview/Diff를 만들 수 있어야 한다.
2. `Error` 또는 적용에 필요한 unresolved `Blocked`가 있으면 Apply를 막는다.
3. Warning/Info는 USER PASS로 해석하지 않으며 UI에 계속 노출한다.
4. Aggregate Struct 전체를 이유 없이 초기화/복사하지 않고 field diff 기반으로 적용한다.
5. Array mutation은 Stable ID 기준 add/update/remove diff로 표현한다.
6. 한 Apply는 하나의 `FScopedTransaction`으로 Undo 가능해야 한다.
7. Apply는 Package Dirty까지만 수행하고 자동 Save를 강제하지 않는다.
8. Target Definition 변경과 Source Tracking Applied Snapshot 갱신은 하나의 논리적 원자 Apply로 취급한다. Post-apply validation이 실패하면 pre-apply Definition/Authoring state를 복원해 서로 다른 revision이 남지 않게 한다.
9. UI와 AI는 같은 Apply Service를 사용한다.
10. 기존 Raw DA Editor는 Advanced/Debug escape hatch로 유지하지만, 직접 수정 후에는 External Drift가 발생할 수 있음을 표시한다.

### 21.15 Validation Ownership Freeze

Validation은 하나의 거대 Validator로 합치지 않는다.

```text
Recipe Input Validation
→ Recipe 필수 semantic input / stable ID / source binding

Resolver Validation
→ source conflict / dependency / cross-profile / derived consistency

UCFVDAValidator
→ Resolved UCFVehicleData 자체의 Definition invariant

Domain Validator / Fitting Validation
→ Equipment / Defense / Ammo / Fitting compatibility와 domain contract

Runtime Technical Validation
→ 실제 적용 경로와 automation / PIE

USER Acceptance
→ 주행감 / 시각 / 조작성 / 제작 UX
```

P0-01에서 확인한 `UCFVDAValidator` coverage gap을 이유로 병렬 Vehicle Validator를 만들지 않는다.
필요한 Definition invariant만 기존 `UCFVDAValidator`에 후속 보강하는 것이 원칙이다.
Domain asset 내부 규칙은 해당 Domain Validator에 남긴다.

### 21.16 P0 Authoring Exposure Freeze

P0 UX 상세 배치는 P0-05에서 결정하지만 노출 수준은 다음처럼 동결한다.

```text
HardpointSlots[].LocationCategory
= Optional Semantic Metadata
= Advanced/Hardpoint authoring에서 볼 수 있음
= P0 필수 입력 아님

MountProfiles[].bExposedModule
= Advanced / Future Module Damage Metadata
= 현재 Runtime 효과 없음 명시
= P0 일반 Vehicle creation 필수 입력 아님

legacy MountProfile serialized fields
= Normal Authoring UI 숨김
= Import/Diagnostics에서 read-only provenance 확인만 허용
```

### 21.17 AI Authoring Boundary Freeze

AI는 사람과 같은 Contract를 사용한다.

```text
Natural Language / Structured Request
→ Recipe Semantic Intent
→ same Profiles
→ same Resolver
→ same Diff / Validation
→ same Apply Service
```

AI 전용 Raw VehicleData mutation path, AI 전용 Profile bypass, AI 전용 hidden default는 금지한다.
Advanced Leaf Override가 필요하면 어떤 field를 왜 override하는지 Preview/Source Trace에 남겨야 한다.

### 21.18 P0-01 Section 4.14 — 12개 입력 최종 판정

| Freeze Input | P0-02 결정 |
|---|---|
| 1. Vehicle Recipe 최소 Human Intent | Persistent Recipe 사용. Asset 선택, 5 Profile Binding, 4 Driving Feel 축, Semantic mass/drive/mount/default-data intent, Advanced Override를 최소 그룹으로 사용 |
| 2. Profile 분할 수 | Vehicle Base / Drivetrain / Handling / Performance / DriveState **5 Domain 고정**, P0 Profile inheritance 금지 |
| 3. Asset Derived 정확 범위 | Layout/Hardpoint Socket Transform은 authoritative. Wheel radius/width는 measurement-assisted preview. COM/ChassisHeight/AdditionalOffset/FX socket 자동 선택 제외 |
| 4. Asset Derived Override | Stable Field Path 기반 Advanced Leaf Override. Base Asset Source와 Override를 동시에 추적 |
| 5. Existing Override Flag 표현 | Layout/Movement/WheelVisual은 derived/readiness 의미에 맞춰 처리. DriveState는 Project Default vs Vehicle-specific semantic mode |
| 6. Existing Definition Import | 전체 기존 leaf를 Legacy Pinned Baseline으로 보존하고 명시적 Adopt 방식 사용. 자동 역산/자동 Profile ownership 금지 |
| 7. MovementProfileName | Current debug/legacy metadata로 보존. 새 Source Tracking의 SSOT로 사용하지 않으며 제거/대체는 P0에서 하지 않음 |
| 8. legacy MountProfile retirement | 새 Authoring 입력 제외 + Legacy Serialized Passthrough. 실제 삭제는 별도 Asset migration evidence 후 |
| 9. LocationCategory / bExposedModule 노출 | 둘 다 P0 필수 입력 아님. Optional/Advanced metadata로 유지 |
| 10. UCFVDAValidator coverage | 기존 Validator를 Definition validation 중심으로 유지. 필요한 invariant만 증분 보강; domain validator 복제 금지 |
| 11. Source Tracking / Stale | Recipe-side persistent provenance + revisions/fingerprints/hash. Profile/Asset/Resolver 변경은 Stale만 표시하고 자동 mutation 금지 |
| 12. Apply Field Diff | Preview → Validate → 단일 Transaction → changed leaf/stable-array diff 적용 → revalidate. 자동 Save 금지 |

### 21.19 Architecture Gate 판정

P0-02의 Freeze 질문에 다음과 같이 답할 수 있다.

```text
새 Vehicle에서 사용자가 입력하는 것
= Asset / Vehicle 의미 / Profile 선택 / 4축 Feel / 구조적 Mount intent / 필요한 명시 Override

AI가 입력하는 것
= 사용자와 동일한 Recipe Semantic Intent

시스템이 계산하는 것
= Profile 합성, Rule/Asset Derived, Raw Definition field, Source Trace, Diff, Stale

최종 VehicleData에 저장되는 것
= 현재 Runtime/Fitting이 소비하는 resolved static Definition 값

왜 특정 값인지 추적하는 방법
= Recipe-side Per-field Source Trace + Applied Provenance

Profile이 바뀔 때
= Stale → Regenerate Preview → Diff → Validate → Explicit Apply

Inventory / Fitting 영향
= 없음. 기존 UCFVehicleData contract를 계속 소비
```

따라서 **DAUTH-P0-02 Authoring Contract Freeze는 PASS**다.

---

## 22. DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design

### 22.1 P0-03 상태와 범위

`DataAuthoringDesign.md v0.3.0` Section 21의 Frozen Contract를 변경하지 않고 실제 구현 가능한 C++ / Editor 설계로 구체화한다.

```text
DAUTH-P0-03 = Detailed Design Complete
다음 단계 = DAUTH-P0-04 VDA Wizard Migration Design
Project 상태 = Working / Pre-Implementation 유지
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
```

P0-03은 다음을 새로 결정하는 단계가 아니다.

```text
Recipe가 필요한가?                 → P0-02에서 Yes로 동결됨
Profile을 몇 개로 나누는가?        → 5 Domain으로 동결됨
Profile inheritance를 쓰는가?       → P0에서는 금지됨
UCFVehicleData를 대체하는가?         → No
Inventory/Fitting을 수정하는가?      → No
기존 Definition을 자동 재해석하는가? → No
```

이번 단계는 위 계약을 P0-08에서 그대로 코드로 옮길 수 있도록 **타입, 데이터 흐름, Field Registry, Resolver Rule, Hash/Stale, Diff, Apply 경계**까지 고정한다.

### 22.2 구현 Module 경계

Data Authoring 구현은 기존 `CarFight_ReEditor` 모듈을 재사용한다.

```text
CarFight_Re Runtime Module
└─ 기존 UCFVehicleData / Validator / Runtime Consumer
   └─ Authoring 의존성 추가 없음

CarFight_ReEditor Module
├─ Persistent Recipe UDataAsset
├─ 5개 Profile UDataAsset
├─ Field Registry / Reflection Codec
├─ Asset Snapshot Reader
├─ Resolver
├─ Source Trace / Stale
├─ Diff
├─ Import / Adoption
├─ Apply Transaction
└─ 후속 Slate / AI Entry Point
```

이 배분을 사용하는 이유:

1. Recipe/Profile은 packaged Runtime에서 필요하지 않은 Editor Authoring 데이터다.
2. Resolver, Reflection, `FScopedTransaction`, Asset inspection은 Editor 책임이다.
3. Runtime Module이 `UnrealEd`, Slate, Authoring Metadata에 의존하지 않게 유지할 수 있다.
4. 기존 `CarFight_ReEditor.Build.cs`는 이미 `CarFight_Re`, `AssetRegistry`, `ContentBrowser`, `Slate`, `UnrealEd` 의존성을 가진다.
5. 새 Runtime module 또는 새 AssetManager 계약을 만들 필요가 없다.

Persistent Recipe/Profile Asset은 `UPrimaryDataAsset`이 아니라 **Editor-only `UDataAsset`**을 사용한다.
P0-02에서 AssetManager 변경을 금지했으므로 Authoring asset을 Primary Asset 계약에 넣지 않는다.

향후 실제 Content Asset을 만들기 전 P0-08에서 Authoring asset package가 cook 대상에 들어가지 않는 **명시적 Never-Cook 보호**를 함께 구현/검증해야 한다. `CarFight_ReEditor` 클래스라는 이유만으로 임의의 `/Game` 폴더가 자동으로 안전하게 cook 제외된다고 가정하지 않는다.

### 22.3 P0-08 예상 파일 배치

실제 파일 생성은 P0-08 이후이며 P0-03에서는 경로와 책임만 고정한다.

```text
UE/Source/CarFight_ReEditor/
├─ Public/DataAuthoring/
│  ├─ CFVehicleAuthoringTypes.h
│  ├─ CFVehicleRecipeData.h
│  ├─ CFVehicleProfileTypes.h
│  ├─ CFVehicleBaseProfile.h
│  ├─ CFDrivetrainProfile.h
│  ├─ CFHandlingProfile.h
│  ├─ CFPerformanceProfile.h
│  ├─ CFDriveStateProfile.h
│  ├─ CFVehicleResolver.h
│  ├─ CFVehicleApplyService.h
│  ├─ CFVehicleImportService.h
│  └─ CFVehicleStaleService.h
│
└─ Private/DataAuthoring/
   ├─ CFVehicleFieldRegistry.h/.cpp
   ├─ CFVehicleFieldCodec.h/.cpp
   ├─ CFVehicleAssetReader.h/.cpp
   ├─ CFVehicleResolver.cpp
   ├─ CFVehicleApplyService.cpp
   ├─ CFVehicleImportService.cpp
   ├─ CFVehicleStaleService.cpp
   └─ *Tests.cpp
```

`FieldRegistry`, `FieldCodec`, `AssetReader`는 UI/외부 호출자가 직접 사용하지 않는 내부 구현으로 `Private`에 둔다.
Resolver / Apply / Import / Stale의 요청·결과 계약은 P0-06 AI Authoring에서도 재사용해야 하므로 `Public` contract로 둔다.

### 22.4 공용 Authoring Type Set

P0-08 구현 시 다음 타입을 기준으로 한다.
이름은 모두 32자를 넘지 않으며 실제 구현 단계에서 무분별한 리네이밍을 하지 않는다.

```text
ECFVehicleProfileDomain
ECFVehicleSourceType
ECFVehicleResolveRule
ECFVehicleAdoptGroup
ECFVehicleDriveStateMode
ECFAuthoringInputMode
ECFAssetIntentMode
ECFWheelVisualIntentMode
ECFVehicleManageState
ECFVehicleDiffOp
ECFVehicleStaleReason

FCFVehicleProfileMeta
FCFFeelResponse
FCFMassScaleRule
FCFVehicleAssetIntent
FCFVehicleProfileBindings
FCFVehicleFeelIntent
FCFVehicleMassIntent
FCFVehicleDurabilityIntent
FCFHardpointIntent
FCFMountIntent
FCFVehicleDefaultIntent
FCFWheelVisualIntent
FCFVehicleAssetAdoption
FCFVehicleBaseProfileData
FCFDrivetrainProfileData
FCFHandlingProfileData
FCFPerformanceProfileData
FCFDriveStateProfileData
FCFVehicleProfileSource
FCFVehicleProfileSnapshotSet
FCFVehicleRecipeSnapshot
FCFVehicleDefinitionSnapshot
FCFVehicleFieldEntry
FCFVehicleResolvedField
FCFVehicleMeasurementProposal
FCFVehicleFieldPath
FCFVehicleFieldValue
FCFVehicleFieldOverride
FCFVehicleSourceLayer
FCFVehicleSourceTrace
FCFVehicleFieldDiff
FCFVehicleStaleField
FCFVehicleStaleReport
FCFVehicleAppliedTrace
FCFVehicleAppliedState
FCFVehicleImportState
FCFVehicleAssetSnapshot
FCFVehicleResolveRequest
FCFVehicleResolveResult
FCFVehicleApplyRequest
FCFVehicleApplyResult
```

위 목록은 P0 public contract에서 서로 참조하는 타입을 누락 없이 나열한 기준 목록이다. `FCFVehicleFieldDescriptor`, Registry/Codec 내부 helper처럼 외부 contract가 아닌 타입은 `Private/DataAuthoring` 구현에 남긴다.

UI 표시용 텍스트와 실제 stable key를 분리한다.
예를 들어 Profile Display Name이 바뀌어도 field identity가 바뀌지 않아야 한다.

### 22.5 Persistent Vehicle Recipe — `UCFVehicleRecipeData`

`UCFVehicleRecipeData : UDataAsset`을 P0 Vehicle Authoring Intent SSOT로 사용한다.

예상 필드 그룹:

```text
FGuid RecipeId
TSoftObjectPtr<UCFVehicleData> TargetVehicleData
FName VehicleArchetypeId

FCFVehicleAssetIntent AssetIntent
FCFVehicleProfileBindings ProfileBindings
FCFVehicleFeelIntent DrivingFeelIntent
FCFVehicleMassIntent MassIntent
FCFVehicleDurabilityIntent DurabilityIntent

TArray<FCFHardpointIntent> HardpointIntents
TArray<FCFMountIntent> MountIntents

FCFVehicleDefaultIntent DefaultDataIntent
FCFWheelVisualIntent WheelVisualIntent
ECFVehicleDriveStateMode DriveStateMode

FCFVehicleAssetAdoption AssetAdoption
TArray<FCFVehicleFieldOverride> AdvancedOverrides
FCFVehicleImportState ImportState
FCFVehicleAppliedState AppliedState

int32 AuthoringRevision
```

#### Recipe Identity

`RecipeId`는 Authoring record 자체의 identity다.
Target `UCFVehicleData` identity와 통합하지 않는다.

- 신규 Recipe 생성 시 새 `FGuid`를 부여한다.
- Recipe 복제 기능을 만들 때는 복제본에 새 Guid를 부여한다.
- Target VehicleData rename-independent ID를 새로 만들지는 않는다.
- Profile Source identity는 별도 Guid를 강제하지 않고 Authoring Asset의 Soft Object Path를 사용한다.

#### Target Binding

`TargetVehicleData`는 `TSoftObjectPtr<UCFVehicleData>`로 둔다.

```text
Resolver Preview
- Target 있음  → current Definition diff / drift 비교 가능
- Target 없음  → 신규 Definition 후보 Preview 가능

Apply
- Target 없음  → P0에서는 Apply Blocked
- Target 있음  → 기존 Apply Service 경로 사용
```

새 `UCFVehicleData` asset 생성 UX는 P0-05/P0-09에서 설계·구현한다.
Resolver 자체가 asset package를 생성하지 않는다.

#### Vehicle Asset Intent

`FCFVehicleAssetIntent`는 다음 **선택값과 binding 이름**만 보관한다.
Asset에서 파생되는 Transform은 저장하지 않는다.

```text
TSoftObjectPtr<UStaticMesh> ChassisMesh
TSoftObjectPtr<UStaticMesh> WheelMeshFL
TSoftObjectPtr<UStaticMesh> WheelMeshFR
TSoftObjectPtr<UStaticMesh> WheelMeshRL
TSoftObjectPtr<UStaticMesh> WheelMeshRR

FName BodyWheelSocketFL
FName BodyWheelSocketFR
FName BodyWheelSocketRL
FName BodyWheelSocketRR
```

Wheel Socket 기본 이름은 현재 Project compatibility convention인:

```text
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR
```

을 사용한다.

#### Driving Feel Intent

`FCFVehicleFeelIntent`는 Frozen 4축만 가진다.

```text
float AccelerationFeel     // 0..1
float SteeringAgility      // 0..1
float GripFeel             // 0..1
float SuspensionFirmness   // 0..1
```

각 값은 Semantic Intent이며 Raw Movement value가 아니다.
Current Quick Tune raw 값을 평균해 이 값을 authoritative하게 역산하지 않는다.

#### Mass Intent

Base/Gross Mass는 정확한 semantic numeric value가 의미 있으므로 Recipe에서 Profile default와 explicit input을 구분한다.

```text
ECFAuthoringInputMode BaseMassMode      // Profile / Explicit
float ExplicitBaseMassKg

ECFAuthoringInputMode GrossMassMode     // Profile / Explicit
float ExplicitGrossMassKg
```

`Fitted TotalVehicleMassKg`는 여기에 넣지 않는다.

#### Durability Intent

`VehicleDurabilityConfig.MaxHealth`도 raw technical value가 아니라 차량 내구도에 대한 명시적 balance input이 될 수 있으므로 Profile default와 explicit input을 구분한다.

```text
ECFAuthoringInputMode MaxHealthMode     // Profile / Explicit
float ExplicitMaxHealth
```

이 값은 `DefaultDefenseData`의 Shield/Armor pool과 합치지 않는다.
`MaxHealth`는 계속 Vehicle durability static definition이고 DefenseData는 별도 defense domain asset이다.

#### Profile Bindings

`FCFVehicleProfileBindings`는 5개 Domain을 이름이나 문자열로 간접 매칭하지 않고 typed soft reference로 가진다.

```text
TSoftObjectPtr<UCFVehicleBaseProfile> VehicleBaseProfile
TSoftObjectPtr<UCFDrivetrainProfile> DrivetrainProfile
TSoftObjectPtr<UCFHandlingProfile> HandlingProfile
TSoftObjectPtr<UCFPerformanceProfile> PerformanceProfile
TSoftObjectPtr<UCFDriveStateProfile> DriveStateProfile
```

Resolver Snapshot Builder가 이 reference를 resolve한 뒤 실제 UObject를 Pure Resolver에 넘기지 않고 typed snapshot으로 복사한다.

#### Default Asset Intent

Defense / Destroyed FX는 `None` 자체도 의미 있는 선택이므로 단순 null만으로 source를 추정하지 않는다.

`ECFAssetIntentMode`:

```text
UseProfile
ExplicitAsset
ExplicitNone
```

을 사용한다.

따라서 `FCFVehicleDefaultIntent`는 최소 다음을 가진다.

```text
DefenseMode
DefaultDefenseData

DestroyedFxMode
DefaultDestroyedFxData

DestroyedFxSocketName
```

`DestroyedFxSocketName`은 자동 Asset Derived로 선택하지 않으며 기본 convention `FX_Destroyed`를 유지한다.

#### Hardpoint Intent

`FCFHardpointIntent`는 정확히 semantic/binding 입력만 가진다.

```text
FName LocationSlotId
FName LocationCategory
FName SocketName
```

`LocalLocation`, `LocalRotation`은 Recipe에 일반 필드로 복제하지 않는다.

- Socket이 있으면 Asset Derived.
- Manual placement가 필요하면 Advanced Leaf Override.
- Legacy import에서는 Legacy Pin으로 보존.

`LocationSlotId`는 stable identity이므로 Advanced Override로 이름을 바꾸지 않는다.
ID rename은 Recipe semantic operation이며 참조 `LocationSlotRef`까지 함께 검증해야 한다.

#### Mount Intent

`FCFMountIntent`는 Current active Mount 계약만 가진다.

```text
FName MountProfileId
FName LocationSlotRef
ECFVehicleMountType MountType
ECFVehicleWeaponSize SizeLimit
TSoftObjectPtr<UCFEquipmentPresetData> DefaultEquipmentPresetData
bool bExposedModule
```

legacy inline Turret field 10개는 이 struct에 넣지 않는다.

`MountProfileId`도 stable identity이므로 Advanced Override 대상이 아니다.

#### Wheel Visual Intent

Normal Authoring에서 raw flag를 직접 노출하지 않고 `ECFWheelVisualIntentMode` semantic mode를 사용한다.

```text
UseProfilePolicy
ManualMeshScale
AutoScaleToPhysicsRadius
```

실제 `bAutoScaleWheelMeshToRadius`, readiness flag 등은 Resolver가 출력한다.

#### DriveState Mode

`ECFVehicleDriveStateMode`는 Frozen Contract 그대로 두 상태만 사용한다.

```text
ProjectDefault
VehicleSpecific
```

`VehicleSpecific`일 때만 DriveState Profile이 effective source가 되고 Resolver가 `bUseDriveStateOverrides=true`를 출력한다.

### 22.6 Profile 공통 Metadata와 Typed Data

5개 Profile은 서로 상속하지 않는다.
공통 C++ base Profile UObject도 만들지 않고 `FCFVehicleProfileMeta` value struct를 각 Profile이 구성(composition)으로 보유한다.

```text
FCFVehicleProfileMeta
├─ FText DisplayName
├─ FString Description
└─ int32 AuthoringRevision
```

Source identity는 Profile Asset의 `FSoftObjectPath`다.

`AuthoringRevision`은 사람이 임의로 맞춰 쓰는 유일한 stale 근거가 아니다.
각 resolve에서 실제 profile payload를 canonical hash하여 **Profile Fingerprint**를 다시 계산한다.
Raw Details 편집으로 Revision 증가를 우회하더라도 Fingerprint 변경으로 stale을 검출할 수 있어야 한다.

각 Profile Asset은 공통 Meta와 Domain별 typed data struct를 가진다.

```text
UCFVehicleBaseProfile
= Meta + FCFVehicleBaseProfileData

UCFDrivetrainProfile
= Meta + FCFDrivetrainProfileData

UCFHandlingProfile
= Meta + FCFHandlingProfileData

UCFPerformanceProfile
= Meta + FCFPerformanceProfileData

UCFDriveStateProfile
= Meta + FCFDriveStateProfileData
```

이 typed data struct를 그대로 Snapshot Builder의 copy source로 사용한다. Resolver Core가 `UDataAsset` property를 직접 읽거나 domain별 값을 generic string에서 다시 해석하지 않는다.

### 22.7 Feel Response와 Context Scale

Current Wizard의 hard-coded Min/Max `FMath::Lerp`를 Profile 데이터로 이동하기 위해 다음 공용 규칙을 사용한다.

#### `FCFFeelResponse`

```text
float LowValue
float NeutralValue
float HighValue
```

Resolve 규칙:

```text
Feel = Clamp(Intent, 0, 1)

0.0 ~ 0.5
= Lerp(LowValue, NeutralValue, Feel * 2)

0.5 ~ 1.0
= Lerp(NeutralValue, HighValue, (Feel - 0.5) * 2)
```

이 방식은 Curve Asset을 추가하지 않고도 0 / 0.5 / 1.0 기준을 Profile에서 명시할 수 있으며 deterministic하다.

#### `FCFMassScaleRule`

같은 Feel 값이 질량이 다른 차량에 무조건 같은 Raw 값을 만들지 않도록 Profile이 명시적으로 mass context scale을 선택할 수 있다.

```text
bool bEnabled
float ReferenceMassKg
float MassExponent
```

계산:

```text
MassScale = 1                              // disabled
MassScale = Pow(BaseMass / ReferenceMass,
                MassExponent)             // enabled
```

규칙:

- `bEnabled=false`이면 hidden scaling 없음.
- `bEnabled=true`인데 BaseVehicleMassKg 또는 ReferenceMassKg가 유효하지 않으면 Resolver `Blocked`.
- 임의 Min/Max clamp를 새로 만들지 않는다.
- 실제 Profile이 어떤 exponent를 사용할지는 Profile content/balance 값이며 P0-03에서 임의 수치를 생성하지 않는다.
- USER Driving Feel PASS는 별도 P0-12다.

P0에서 이 scale을 사용할 수 있는 대표 대상:

- Acceleration Feel → `EngineMaxTorque`
- Suspension Firmness → Front/Rear `SpringRate`, `SpringPreload`

다른 field에 mass scale을 추가하려면 해당 Profile rule에 명시적으로 설정해야 한다.

### 22.8 Vehicle Base Profile

`UCFVehicleBaseProfile : UDataAsset`

Primary baseline fields:

```text
BaseVehicleMassKg
MaximumGrossMassKg
VehicleDurabilityConfig.MaxHealth
VehicleMovementConfig.ChassisHeight

DefaultDefenseData
DefaultDestroyedFxData

WheelVisualConfig.ExpectedWheelCount
WheelVisualConfig.FrontWheelCountForSteering
WheelVisualConfig.bAutoScaleWheelMeshToRadius default policy
WheelVisualConfig.WheelMeshRadiusMeasureMode default
WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin
WheelVisualConfig.WheelMeshScaleClampMin
WheelVisualConfig.WheelMeshScaleClampMax
```

Recipe explicit Mass/DefaultData/WheelVisual intent가 있으면 Frozen precedence에 따라 Profile baseline보다 우선한다.

`DestroyedFxSocketName`, wheel socket naming, legacy friction compatibility는 Vehicle Base Profile에 숨겨 넣지 않고 Project Compatibility Default 또는 Recipe binding으로 남긴다.

### 22.9 Drivetrain Profile

`UCFDrivetrainProfile : UDataAsset`

```text
VehicleMovementConfig.DifferentialType
VehicleMovementConfig.FrontRearSplit
VehicleMovementConfig.bFrontWheelAffectedByEngine
VehicleMovementConfig.bRearWheelAffectedByEngine

VehicleReferenceConfig.FrontWheelClass
VehicleReferenceConfig.RearWheelClass
```

Wheel Class는 Authoring Asset에서 `TSoftClassPtr<UChaosVehicleWheel>`로 저장하고 Resolve 시 Definition에 필요한 class reference로 materialize한다.

Resolver Validation은 최소 다음 일관성을 검사한다.

```text
DifferentialType
↔ FrontRearSplit
↔ Front/Rear bAffectedByEngine
```

단, 실제 허용 조합/게임 밸런스 임계값을 P0-03에서 임의로 새로 만들지 않는다.

### 22.10 Handling Profile

`UCFHandlingProfile : UDataAsset`

#### Steering Feel Response

```text
FrontWheelMaxSteerAngleByFeel : FCFFeelResponse
SteeringAngleRatioByFeel      : FCFFeelResponse
```

#### Grip Feel Response

```text
FrontWheelFrictionByFeel      : FCFFeelResponse
RearWheelFrictionByFeel       : FCFFeelResponse
FrontCorneringByFeel          : FCFFeelResponse
RearCorneringByFeel           : FCFFeelResponse
```

#### Suspension Feel Response

```text
FrontSpringRateByFeel         : FCFFeelResponse
RearSpringRateByFeel          : FCFFeelResponse
FrontSpringPreloadByFeel      : FCFFeelResponse
RearSpringPreloadByFeel       : FCFFeelResponse
SuspensionMassScale           : FCFMassScaleRule
```

#### Direct Handling Baseline

```text
FrontWheelMaxBrakeTorque
RearWheelMaxBrakeTorque
RearWheelMaxHandBrakeTorque
FrontWheelLoadRatio
RearWheelLoadRatio
FrontWheelSuspensionMaxRaise
RearWheelSuspensionMaxRaise
FrontWheelSuspensionMaxDrop
RearWheelSuspensionMaxDrop
FrontWheelSweepShape
RearWheelSweepShape
SteeringType
```

Wheel Radius/Width, COM vector, AdditionalOffset은 Handling Profile에 넣지 않는다.
각각 Asset Measurement / Advanced Override 경계를 유지한다.

### 22.11 Performance Profile

`UCFPerformanceProfile : UDataAsset`

#### Acceleration Feel Response

```text
EngineMaxTorqueByFeel     : FCFFeelResponse
EngineMaxRPMByFeel        : FCFFeelResponse
ThrottleInputScaleByFeel  : FCFFeelResponse
TorqueMassScale           : FCFMassScaleRule
```

#### Direct Performance Baseline

```text
EngineIdleRPM
EngineBrakeEffect
EngineRevUpMOI
EngineRevDownRate
DragCoefficient
DownforceCoefficient
```

`TorqueMassScale`은 Profile이 명시적으로 enable했을 때만 `EngineMaxTorque` response 결과에 적용한다.
RPM과 ThrottleInputScale에는 동일 scale을 자동 전파하지 않는다.

### 22.12 DriveState Profile

`UCFDriveStateProfile : UDataAsset`

`FCFVehicleDriveStateConfig` 전체를 그대로 넣지 않는다.
그 struct에는 Runtime gate인 `bUseDriveStateOverrides`가 포함되어 있기 때문이다.

Profile에는 나머지 14 behavior field만 보관한다.

```text
bEnableDriveStateHysteresis
bUsePerStateHoldTimes
DriveStateMinimumHoldTimeSeconds
IdleStateMinimumHoldTimeSeconds
ReversingStateMinimumHoldTimeSeconds
AirborneStateMinimumHoldTimeSeconds
IdleEnterSpeedThresholdKmh
IdleExitSpeedThresholdKmh
ReverseEnterSpeedThresholdKmh
ReverseExitSpeedThresholdKmh
AirborneMinSpeedThresholdKmh
AirborneVerticalSpeedThresholdCmPerSec
ActiveInputThreshold
bTreatOppositeThrottleAsBrake
```

`bUseDriveStateOverrides`는 Recipe `DriveStateMode`로부터 Derived한다.

### 22.13 Immutable Resolve Snapshot

Resolver가 live UObject를 여기저기 직접 읽으면서 계산하지 않도록 UI/Asset read와 Pure Resolve를 분리한다.

```text
Editor UObject / Asset
        ↓ Snapshot Builder
Immutable Resolve Request
        ↓ Pure Resolver
Resolved Field Set
```

#### Recipe Snapshot — `FCFVehicleRecipeSnapshot`

Recipe UObject에서 Resolve에 필요한 Authoring Intent와 Import/Applied baseline을 value copy한다.
Snapshot 생성 뒤 Resolver Core는 `UCFVehicleRecipeData`를 다시 읽지 않는다.

#### Profile Source — `FCFVehicleProfileSource`

```text
FSoftObjectPath SourceObjectPath
int32 AuthoringRevision
FString ProfileFingerprint
```

#### Profile Snapshot Set — `FCFVehicleProfileSnapshotSet`

5개 Domain 각각에 `FCFVehicleProfileSource + Domain typed data` pair를 보관한다.

```text
BaseSource       + FCFVehicleBaseProfileData
DrivetrainSource + FCFDrivetrainProfileData
HandlingSource   + FCFHandlingProfileData
PerformanceSource+ FCFPerformanceProfileData
DriveStateSource + FCFDriveStateProfileData
```

`DriveStateMode=ProjectDefault`이면 DriveState pair는 non-effective/optional snapshot으로 둘 수 있다.

#### Definition Snapshot — `FCFVehicleDefinitionSnapshot`

Current Target 또는 Project Default `UCFVehicleData`를 Registry로 읽은 결과다.

```text
FCFVehicleFieldEntry
= FCFVehicleFieldPath + FCFVehicleFieldValue

TArray<FCFVehicleFieldEntry> SortedFields
FString DefinitionHash
```

Raw Definition Snapshot에는 Source ownership을 넣지 않는다. Source는 Resolver가 붙이는 정보이므로 `FCFVehicleResolvedField`와 분리한다.

Target snapshot과 Project Default snapshot은 같은 codec/ordering을 사용해 hash 의미가 달라지지 않게 한다.

#### Asset Snapshot — `FCFVehicleAssetSnapshot`

Resolver가 필요한 asset 사실만 보관한다.

```text
Chassis Object Path
사용하는 Chassis Socket별 Relative Transform

각 WheelMesh Object Path
Bounds Origin
Bounds Extent

각 resolver-relevant Asset Fingerprint
```

Asset Fingerprint는 package 전체의 무관한 변경까지 stale 처리하기보다 **Resolver가 실제 소비하는 추출 결과**를 canonical hash한다.

예:

```text
Chassis Layout Fingerprint
= Chassis Object Path
+ 사용 Wheel/Hardpoint Socket Name
+ 각 Socket Transform

Wheel Measure Fingerprint
= Wheel Object Path
+ Bounds Origin
+ Bounds Extent
```

Material만 변경되고 Socket/Bounds가 같다면 Layout/Measurement field를 불필요하게 stale 처리하지 않는다.

### 22.14 Stable Field Path — `FCFVehicleFieldPath`

Source Trace / Override / Legacy Pin / Diff의 identity에 raw FString 한 줄만 사용하지 않는다.
구조화된 key를 저장하고 UI/로그에서 canonical string으로 변환한다.

예상 구조:

```text
TArray<FName> PropertyChain

FName CollectionPropertyName
FName SelectorKeyPropertyName
FName SelectorKeyValue
```

Scalar/Nested 예:

```text
PropertyChain
= [VehicleMovementConfig, EngineMaxTorque]

CollectionPropertyName = None
```

Array 예:

```text
CollectionPropertyName   = HardpointSlots
SelectorKeyPropertyName  = LocationSlotId
SelectorKeyValue         = Top_01
PropertyChain            = [LocalLocation]
```

Canonical 표시:

```text
VehicleMovementConfig.EngineMaxTorque
HardpointSlots[LocationSlotId=Top_01].LocalLocation
MountProfiles[MountProfileId=RoofTurret_MediumOrLarge].SizeLimit
```

규칙:

1. Array index는 identity로 사용하지 않는다.
2. `LocationSlotId`, `MountProfileId`가 current stable selector다.
3. Selector ID rename은 leaf override가 아니라 semantic collection edit다.
4. Canonical string은 표시/정렬/Hash input으로 사용하고 실제 property access는 구조화된 path를 사용한다.

### 22.15 Generic Field Value — `FCFVehicleFieldValue`

117개 leaf type마다 별도 override struct를 만들지 않는다.
Legacy Pin / Advanced Override / Diff에 필요한 generic value는 Reflection-safe canonical value로 저장한다.

```text
FString PropertyTypeSignature
FString CanonicalValueText
```

`PropertyTypeSignature`에는 단순 `FloatProperty`만 기록하지 않고 필요하면 다음 type identity까지 포함한다.

```text
Struct path
Enum path
Object class path
Class constraint
```

`CFVehicleFieldCodec` 책임:

```text
FProperty value → CanonicalValueText
CanonicalValueText → checked FProperty value
Type Signature 검증
Value Hash 생성
Soft/Object/Class reference path normalize
```

구현은 Unreal Reflection의 `FProperty` Export/Import 경로를 사용하되 typed UI가 `CanonicalValueText`를 직접 편집하게 만들지 않는다.

이 방식의 목적:

- float/bool/int/FName/enum/Vector/Rotator/Transform/reference를 한 diff 모델에서 처리
- hidden legacy UPROPERTY도 보존
- 새로운 `FInstancedPropertyBag` 의존성을 P0에서 불필요하게 추가하지 않음
- Apply 전 실제 property type mismatch를 Blocked로 판정

### 22.16 Vehicle Field Registry

`FCFVehicleFieldRegistry`는 P0-01 117 leaf field와 P0-03 Resolver rule을 연결하는 정적 C++ Registry다.
Top-level aggregate holder 9개는 leaf diff 대상이 아니므로 Registry expected leaf count는 **117**이다.

각 `FCFVehicleFieldDescriptor`는 최소 다음을 가진다.

```text
Stable Path Pattern
Property Chain
Collection Selector Rule
Primary Profile Domain
Resolve Rule
Allowed Source Mask
Adoption Group
Advanced Override 허용 여부
Identity Field 여부
Legacy Serialized 여부
Required Dependency 목록
```

#### Coverage Self-Test

P0-08 구현 시 자동 테스트에서 Reflection으로 Current `UCFVehicleData` leaf를 다시 discover해 Registry와 양방향 비교한다.

```text
Current Definition leaf found but no Registry descriptor
→ FAIL

Registry descriptor but source field removed/renamed
→ FAIL

Duplicate descriptor
→ FAIL

Expected Current Coverage
= 117 leaf
```

이 테스트는 향후 `UCFVehicleData` 필드 추가 시 Data Authoring이 조용히 누락되는 것을 막는 핵심 유지보수 Gate다.

### 22.17 Field Resolver Map — 117 Leaf Coverage

아래 Map은 P0-01 Matrix를 반복하는 표가 아니라 **각 current leaf가 P0 Resolver에서 어떤 Rule로 생성되는지**를 고정한다.

`Override` 열은 신규 Advanced Leaf Override 허용 여부다.
Legacy Pin은 Existing Definition Import에서 별도 precedence로 모든 보존 대상에 적용될 수 있다.

| Field Path / Group | Count | Primary Resolver Rule | Main Dependency | Adoption Group | Override |
|---|---:|---|---|---|---|
| `VehicleVisualConfig.{ChassisMesh,WheelMeshFL,WheelMeshFR,WheelMeshRL,WheelMeshRR}` | 5 | Recipe Asset Intent | Recipe asset refs | VisualAssets | No — normal semantic edit |
| `VehicleLayoutConfig.bUseLayoutOverrides` | 1 | Derived Gate | adopted/resolved layout state | Layout | No |
| `VehicleLayoutConfig.BodyWheelSocket{FL,FR,RL,RR}` | 4 | Recipe Binding + Project Default | Recipe socket names | Layout | Yes |
| `VehicleLayoutConfig.WheelAnchor{FL,FR,RL,RR}.{RelativeLocation,RelativeRotation}` | 8 | Asset Socket Derived | Chassis snapshot + socket binding | Layout | Yes |
| `HardpointSlots[].LocationSlotId` | 1/element | Recipe Semantic Identity | Hardpoint intent | Hardpoints | No |
| `HardpointSlots[].LocationCategory` | 1/element | Recipe Semantic | Hardpoint intent | Hardpoints | No |
| `HardpointSlots[].SocketName` | 1/element | Recipe Asset Binding | Hardpoint intent | Hardpoints | No |
| `HardpointSlots[].LocalLocation` | 1/element | Asset Socket Derived | Chassis socket snapshot | Hardpoints | Yes |
| `HardpointSlots[].LocalRotation` | 1/element | Asset Socket Derived | Chassis socket snapshot | Hardpoints | Yes |
| `MountProfiles[].MountProfileId` | 1/element | Recipe Semantic Identity | Mount intent | Mounts | No |
| `MountProfiles[].LocationSlotRef` | 1/element | Recipe Semantic | Mount + Hardpoint intent | Mounts | No |
| `MountProfiles[].{MountType,SizeLimit,DefaultEquipmentPresetData}` | 3/element | Recipe Semantic | Mount intent | Mounts | Yes |
| `MountProfiles[].{TurretYawMesh,TurretPitchMesh,TurretYawRelativeTransform,PitchPivotSocketName,TurretPitchRelativeTransform,YawTurnRateDegPerSec,PitchTurnRateDegPerSec,StabilizationToleranceDeg,AimSettleTimeSeconds,MountWeightKg}` | 10/element | Legacy Serialized Passthrough | Import snapshot only | LegacyTechnical | No |
| `MountProfiles[].bExposedModule` | 1/element | Recipe Optional Semantic | Mount intent | Mounts | Yes |
| `BaseVehicleMassKg` | 1 | Base Profile → Recipe Explicit | Base profile / Mass intent | MassDurability | No — explicit semantic input exists |
| `MaximumGrossMassKg` | 1 | Base Profile → Recipe Explicit | Base profile / Mass intent | MassDurability | Yes |
| `VehicleMovementConfig.bUseMovementOverrides` | 1 | Derived Gate | effective Wheel detail / Throttle source | DerivedState | No |
| `VehicleMovementConfig.MovementProfileName` | 1 | Compatibility Default / Legacy Pin | Current default or imported value | LegacyTechnical | No |
| `VehicleMovementConfig.{ThrottleInputScale,EngineMaxTorque,EngineMaxRPM}` | 3 | Acceleration Feel Derived | Performance profile + Recipe feel + optional mass context | Performance | Yes |
| `VehicleMovementConfig.{FrontWheelMaxSteerAngle,SteeringAngleRatio}` | 2 | Steering Feel Derived | Handling profile + Recipe feel | Handling | Yes |
| `VehicleMovementConfig.{FrontWheelMaxBrakeTorque,RearWheelMaxBrakeTorque,RearWheelMaxHandBrakeTorque}` | 3 | Handling Profile Direct | Handling profile | Handling | Yes |
| `VehicleMovementConfig.{FrontWheelRadius,RearWheelRadius,FrontWheelWidth,RearWheelWidth}` | 4 | Asset Measurement Proposal → Explicit Adoption | Wheel snapshot bounds | WheelGeometry | Yes |
| `VehicleMovementConfig.{FrontWheelFrictionForceMultiplier,RearWheelFrictionForceMultiplier,FrontWheelCorneringStiffness,RearWheelCorneringStiffness}` | 4 | Grip Feel Derived | Handling profile + Recipe feel | Handling | Yes |
| `VehicleMovementConfig.{FrontWheelLoadRatio,RearWheelLoadRatio}` | 2 | Handling Profile Direct | Handling profile | Handling | Yes |
| `VehicleMovementConfig.{FrontWheelSpringRate,RearWheelSpringRate,FrontWheelSpringPreload,RearWheelSpringPreload}` | 4 | Suspension Feel Derived | Handling profile + Recipe feel + optional mass context | Handling | Yes |
| `VehicleMovementConfig.{FrontWheelSuspensionMaxRaise,RearWheelSuspensionMaxRaise,FrontWheelSuspensionMaxDrop,RearWheelSuspensionMaxDrop}` | 4 | Handling Profile Direct | Handling profile | Handling | Yes |
| `VehicleMovementConfig.{bFrontWheelAffectedByEngine,bRearWheelAffectedByEngine}` | 2 | Drivetrain Profile | Drivetrain profile | Drivetrain | Yes — exceptional technical override |
| `VehicleMovementConfig.{FrontWheelSweepShape,RearWheelSweepShape}` | 2 | Handling Profile Direct | Handling profile | Handling | Yes |
| `VehicleMovementConfig.ChassisHeight` | 1 | Vehicle Base Profile | Base profile | MassDurability | Yes |
| `VehicleMovementConfig.{DragCoefficient,DownforceCoefficient}` | 2 | Performance Profile Direct | Performance profile | Performance | Yes |
| `VehicleMovementConfig.bEnableCenterOfMassOverride` | 1 | Derived Gate | effective CenterOfMassOverride source | TechnicalHandling | No |
| `VehicleMovementConfig.CenterOfMassOverride` | 1 | Compatibility Default / Advanced Only | Explicit override or Legacy Pin | TechnicalHandling | Yes |
| `VehicleMovementConfig.{EngineIdleRPM,EngineBrakeEffect,EngineRevUpMOI,EngineRevDownRate}` | 4 | Performance Profile Direct | Performance profile | Performance | Yes |
| `VehicleMovementConfig.DifferentialType` | 1 | Drivetrain Profile | Drivetrain profile | Drivetrain | Yes |
| `VehicleMovementConfig.FrontRearSplit` | 1 | Drivetrain Profile | Drivetrain profile | Drivetrain | Yes |
| `VehicleMovementConfig.SteeringType` | 1 | Handling Profile Direct | Handling profile | Handling | Yes |
| `VehicleMovementConfig.bLegacyWheelFrictionPosition` | 1 | Project Compatibility Default | Current C++ compatibility default | LegacyTechnical | Yes |
| `VehicleMovementConfig.{FrontWheelAdditionalOffset,RearWheelAdditionalOffset}` | 2 | Compatibility Default / Advanced Only | Explicit override or Legacy Pin | TechnicalHandling | Yes |
| `WheelVisualConfig.bUseWheelVisualOverrides` | 1 | Derived Readiness | effective WheelVisual adoption | WheelVisual | No |
| `WheelVisualConfig.{ExpectedWheelCount,FrontWheelCountForSteering}` | 2 | Vehicle Base Profile | Base profile | WheelVisual | Yes |
| `WheelVisualConfig.bAutoScaleWheelMeshToRadius` | 1 | Recipe WheelVisual Policy | Recipe intent + Base default | WheelVisual | No |
| `WheelVisualConfig.WheelMeshRadiusMeasureMode` | 1 | Base Profile + Measurement Suggestion | Base profile / Wheel snapshot | WheelGeometry | Yes |
| `WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin` | 1 | Vehicle Base Profile | Base profile | WheelVisual | Yes |
| `WheelVisualConfig.{WheelMeshScaleClampMin,WheelMeshScaleClampMax}` | 2 | Vehicle Base Profile | Base profile | WheelVisual | Yes |
| `VehicleReferenceConfig.{FrontWheelClass,RearWheelClass}` | 2 | Drivetrain Profile | Drivetrain profile | Drivetrain | Yes |
| `VehicleDurabilityConfig.MaxHealth` | 1 | Base Profile → Recipe Explicit | Base profile / Durability intent | MassDurability | Yes |
| `DefaultDefenseData` | 1 | Base Profile → Recipe Asset Intent | Base profile / ExplicitAsset / ExplicitNone | DefenseFx | Yes |
| `DefaultDestroyedFxData` | 1 | Base Profile → Recipe Asset Intent | Base profile / ExplicitAsset / ExplicitNone | DefenseFx | Yes |
| `DestroyedFxSocketName` | 1 | Project Default → Recipe Semantic | `FX_Destroyed` / Recipe | DefenseFx | Yes |
| `DriveStateConfig.bUseDriveStateOverrides` | 1 | Derived Gate | Recipe DriveStateMode | DriveState | No |
| `DriveStateConfig` 나머지 14 behavior leaf | 14 | DriveState Profile | DriveState profile when VehicleSpecific | DriveState | Yes |

Coverage audit:

```text
VehicleVisualConfig        5
VehicleLayoutConfig       13
HardpointSlots             5 leaf patterns
MountProfiles             16 leaf patterns
Fitting Mass               2
VehicleMovementConfig     47
WheelVisualConfig          8
VehicleReferenceConfig     2
Durability / Defense / FX  4
DriveStateConfig          15
----------------------------
Total                    117 leaf patterns
```

Array row의 Count는 element당 leaf pattern 수이며 Registry Coverage는 selector wildcard pattern 기준으로 117을 계산한다.
실제 Recipe에 Hardpoint/Mount element가 여러 개 있어도 schema coverage count 자체는 변하지 않는다.

### 22.18 Asset Measurement Adoption

Wheel Radius/Width는 P0-02에서 authoritative auto-apply가 아니라 measurement-assisted candidate로 동결했다.
따라서 Resolver Result에는 **resolved field**와 별도로 `MeasurementProposals`를 제공한다.

```text
Wheel Mesh Bounds
→ Measurement Proposal
→ Preview 표시
→ 사용자/AI가 Use Measured Value를 명시적으로 선택
→ Recipe AssetAdoption에 fingerprint와 선택 기록
→ 다음 Resolve부터 effective Asset Derived Source
```

`FCFVehicleAssetAdoption` 최소 정보:

```text
bUseMeasuredFrontRadius
bUseMeasuredRearRadius
bUseMeasuredFrontWidth
bUseMeasuredRearWidth
bUseSuggestedRadiusMeasureMode

각 accepted measurement의 Asset Fingerprint
```

Accepted fingerprint와 현재 Wheel Snapshot fingerprint가 달라지면 해당 field를 Stale로 판정한다.

신규 managed 차량에서 WheelGeometry가 Profile이나 hidden fallback으로 조용히 확정되는 것을 피한다.
사용자는 measured candidate를 Adopt하거나 Advanced Override를 사용하거나 compatibility default 사용을 명시적으로 확인해야 한다.

### 22.19 Advanced Leaf Override

`FCFVehicleFieldOverride`:

```text
FCFVehicleFieldPath FieldPath
FCFVehicleFieldValue OverrideValue
FString Reason
```

규칙:

1. Registry의 `bAdvancedOverrideAllowed=true` field만 등록 가능.
2. `LocationSlotId`, `MountProfileId` 같은 identity field는 override 금지.
3. aggregate path 금지.
4. legacy Mount serialized field override 금지.
5. type signature mismatch는 저장/resolve 시 Error.
6. duplicate override path는 Error.
7. UI/AI는 Reason을 기록할 수 있어야 한다.
8. Override 삭제 시 현재 Base/Profile/Rule/Asset result가 다시 effective value가 된다.

Advanced override가 effective한 동안 underlying Profile이 바뀌어도 target value가 바뀔 필요는 없다.
다만 UI에서 `Shadow Source Changed`를 표시해 override 해제 시 base가 달라졌음을 알 수 있어야 한다.

### 22.20 Existing Definition Import / Legacy Pin

Import Service는 Current `UCFVehicleData` 117 leaf snapshot을 Field Registry로 읽는다.

```text
UCFVehicleData
→ Definition Snapshot
→ Recipe 생성
→ Legacy Pin Set
→ lossless semantic candidate copy
→ optional Profile Match Suggestion
```

#### Legacy Pin Storage

`FCFVehicleImportState` 최소 정보:

```text
ECFVehicleManageState ManageState
FString ImportedDefinitionHash
TArray<FCFVehicleFieldOverride> LegacyPinnedFields
TArray<FCFVehicleFieldOverride> LegacySerializedFields
TSet<ECFVehicleAdoptGroup> AdoptedGroups
```

`LegacySerializedFields`는 MountProfile legacy 10개 전용이며 normal Adopt 대상이 아니다.

#### Lossless Semantic Candidate Copy

다음처럼 현재 raw 값 자체가 이미 semantic 값인 것은 Recipe candidate로 그대로 복사할 수 있다.

```text
Asset references
Wheel socket binding names
Hardpoint ID / Category / Socket
Mount active fields
Base/Gross Mass
Default Defense / FX refs
DestroyedFxSocketName
DriveState override mode
```

하지만 copy한 candidate가 있다는 이유만으로 source ownership을 자동 전환하지 않는다.
Legacy Pin이 더 높은 precedence로 기존 값을 계속 보호한다.

#### 금지되는 Inverse Inference

```text
Movement raw values → 4 Driving Feel을 authoritative하게 역산
Movement raw values → Handling/Performance Profile 자동 선택
현재 값이 Profile과 동일 → 해당 Profile ownership 자동 확정
```

위는 모두 금지한다.

### 22.21 Adoption Transaction

Legacy 차량은 field 또는 group 단위로 새 Authoring Source를 Adopt할 수 있다.

P0 표준 사용자 Adoption group:

```text
VisualAssets
Layout
Hardpoints
Mounts
MassDurability
DefenseFx
Drivetrain
Handling
Performance
WheelGeometry
WheelVisual
DriveState
TechnicalHandling
LegacyTechnical
```

`DerivedState`는 `bUseMovementOverrides`처럼 여러 Authoring source의 상태에서 계산되는 내부 Registry group이며 사용자에게 독립 Adopt 대상으로 노출하지 않는다. Derived field의 Legacy Pin 해제 여부는 해당 소유 source/group의 Adoption과 Resolver gate rule로 결정한다.

`LegacyTechnical`의 hidden serialized fields는 일반 Adopt에서 제외한다.

Adopt 흐름:

```text
현재 Legacy Pin 유지 Resolve
→ 선택 group의 Pin을 가상으로 제외한 Preview
→ Source / Diff / Validation 확인
→ Adopt 승인
→ Recipe에서 해당 Legacy Pin 제거
→ AdoptedGroups 갱신
→ Recipe Revision 증가
→ Recipe Package Dirty
```

중요:

- **Adopt 자체는 Target VehicleData를 수정하지 않는다.**
- Adopt 뒤 Definition은 Stale 상태가 될 수 있다.
- 실제 Definition 변경은 별도 Apply를 눌러야 한다.
- Recipe authoring mutation도 `FScopedTransaction`으로 Undo 가능하게 구현한다.

### 22.22 Resolver Input — `FCFVehicleResolveRequest`

Resolver Core는 live Slate 상태를 직접 읽지 않는다.

```text
FCFVehicleResolveRequest
├─ FCFVehicleRecipeSnapshot Recipe
├─ FCFVehicleProfileSnapshotSet Profiles
├─ FCFVehicleDefinitionSnapshot ProjectDefaults
├─ FCFVehicleAssetSnapshot Assets
├─ FCFVehicleDefinitionSnapshot CurrentDefinition [optional]
├─ Optional Fitting Preview Context
└─ ResolverContractRevision
```

`Project Default Definition Snapshot`은 별도 숫자표를 중복 관리하지 않는다.
현재 `UCFVehicleData` C++ defaults를 transient/default object에서 Field Registry로 읽어 117 leaf snapshot을 만든다.

따라서 C++ 기본값이 Authoring compatibility baseline의 단일 근거다.

Optional Fitting Context는 Resolve value source가 아니다.
UI Preview/technical context 계산만 소비할 수 있고 Source Trace winner가 될 수 없다.

### 22.23 Resolver Output — `FCFVehicleResolveResult`

```text
FCFVehicleResolveResult
├─ ResolveStatus
├─ Sorted ResolvedFields
├─ PreviewSourceTrace
├─ MeasurementProposals
├─ FieldDiff
├─ RecipeValidation
├─ ResolverValidation
├─ DefinitionValidation
├─ StaleReport
├─ SourceSignature
├─ ResolvedDefinitionHash
└─ ResolverContractRevision
```

`ResolvedFields`는 `UCFVehicleData` aggregate copy가 아니라 `FCFVehicleResolvedField`의 sorted leaf set이다.

```text
FCFVehicleResolvedField
├─ FCFVehicleFieldPath FieldPath
├─ FCFVehicleFieldValue Value
└─ Effective Source reference/index
```

Wheel measurement 후보는 `FCFVehicleMeasurementProposal`로 별도 보관한다.

```text
FieldPath
MeasuredCandidateValue
AssetFingerprint
MeasurementRuleId
```

따라서 proposal은 adoption 전에는 ResolvedFields effective value를 조용히 바꾸지 않는다.

이 구조로 같은 데이터가 Source Trace, Diff, Apply, Hash에 동일하게 사용된다.

Definition Validation이 필요할 때만 Field Codec으로 transient `UCFVehicleData` candidate를 materialize하여 기존 `UCFVDAValidator`에 전달한다.

### 22.24 Resolver Stage Order

P0 Resolver 순서는 고정한다.

```text
R0  Request / Recipe / Binding validation
R1  Current C++ Project Default Snapshot 생성
R2  5 Profile Snapshot 적용
R3  Recipe Semantic Input 적용
R4  Driving Feel Rule Derived 적용
R5  Authoritative Asset Socket Derived 계산
R6  Wheel Measurement Proposal 계산
R7  Accepted Asset Measurement만 effective source로 적용
R8  Legacy Pinned Baseline 적용 [import only]
R9  Legacy Serialized Passthrough 적용 [import only]
R10 Advanced Leaf Override 적용
R11 Derived Gate / Readiness flag 계산
R12 Cross-field Resolver Validation
R13 Source Trace / Hash 생성
R14 Current Definition과 Field Diff 생성
R15 transient UCFVehicleData materialize + UCFVDAValidator
R16 Stale / External Drift Report 생성
```

주의:

- Legacy Pin이 존재하면 R2~R7의 lower source는 preview의 shadow source로 보일 수 있지만 effective source가 되지 않는다.
- Advanced Override가 있으면 R10이 최종 effective value다.
- Gate field는 R11에서 raw bool을 새로 덮어쓰는 방식이 아니라 **Derived candidate layer를 계산**한다. 해당 gate field 자체에 R8 Legacy Pin이 존재하면 Legacy Pin이 계속 effective source이며 R11 candidate는 shadow layer로만 남는다.
- 따라서 R11의 실행 순서가 Frozen precedence의 `Legacy Pin > Derived` 의미를 뒤집지 않는다.

### 22.25 Derived Flag 세부 Rule

#### Layout Flag

```text
bUseLayoutOverrides = true
```

조건:

- 4 Wheel Anchor의 location/rotation 8개 leaf가 모두 **Project Compatibility Default가 아닌 유효 Authoring Source**로 resolve됨.
- 허용 source는 Asset Derived / Advanced Leaf Override / Legacy Pin이며 단순 C++ default zero pose는 resolved layout으로 세지 않는다.
- New/Adopted Layout에서 blocking socket error가 없음.

Legacy Pin이 flag 자체를 보호하는 import 상태에서는 pin이 우선한다.

#### Movement Flag

`bUseMovementOverrides`는 전체 Movement flag가 아니다.

New/Managed Resolve에서는 다음 Wheel detail/Throttle authored path가 effective할 때 true로 출력한다.

```text
ThrottleInputScale
Front/Rear wheel detailed runtime tuning fields
```

Legacy Pin이 flag를 소유하면 pin 우선.

#### WheelVisual Flag

`bUseWheelVisualOverrides`는 Runtime gate로 사용하지 않는다.

```text
WheelVisual Adopted/Managed
→ true readiness output

Legacy Pin
→ imported value preserved
```

#### DriveState Flag

```text
DriveStateMode = ProjectDefault
→ bUseDriveStateOverrides = false

DriveStateMode = VehicleSpecific
+ valid DriveState Profile
→ bUseDriveStateOverrides = true
```

### 22.26 Source Trace

Preview는 winning source 하나만 보여주지 않고 source stack을 설명할 수 있어야 한다.

`FCFVehicleSourceLayer`:

```text
SourceType
SourceId
SourceRevision
SourceFingerprint
ValueHash
bEffective
```

`FCFVehicleSourceTrace`:

```text
FieldPath
TArray<FCFVehicleSourceLayer> Layers
EffectiveLayerIndex
EffectiveSourceSignature
ShadowSourceSignature
```

예:

```text
VehicleMovementConfig.EngineMaxTorque

Project Default       750
Performance Profile   820
Acceleration Rule     905
Legacy Pin            812   ← Effective
```

또는:

```text
Performance Profile   820
Acceleration Rule     905
Advanced Override     930   ← Effective
```

Persistent `AppliedState`에는 전체 verbose stack을 그대로 중복 보존할 필요는 없다.
Stale 판정에 필요한 최소 Applied Trace를 저장한다.

```text
FieldPath
Effective Source Type / ID
Effective Source Signature
Shadow Source Signature
Last Applied Value Hash
```

### 22.27 Recipe / Profile / Asset Fingerprint

Revision 숫자만 stale truth로 사용하지 않는다.

#### Revision Lifecycle

`AuthoringRevision`은 사람이 변경 이력을 읽기 위한 sequence이며 stale truth 자체가 아니다.

- Recipe/Profile의 Authoring payload를 정상 UI/Service로 변경할 때 logical edit당 1회 증가시킨다.
- Raw Details 편집도 `PostEditChangeProperty` 경로에서 revision 증가 대상으로 처리한다.
- Recipe의 `AppliedState` 갱신은 Authoring Intent 변경이 아니므로 `AuthoringRevision`을 증가시키지 않는다.
- Profile `DisplayName` / `Description` 같은 resolver-nonsemantic metadata 변경은 payload revision 증가 대상에서 제외할 수 있다.
- Undo/Redo는 Unreal Transaction이 저장한 revision 값까지 함께 복원해야 한다.
- Revision 누락이나 수동 편집 가능성을 고려해 실제 stale 판정은 아래 Fingerprint를 Authority로 사용한다.

#### Source Signature

Field-level effective source signature는 표시용 revision 숫자만으로 만들지 않는다.
최소 canonical input은 다음이다.

```text
Stable Field Path
Source Type
Source Identity
Source Payload Fingerprint
Resolver Rule Identity
Resolver Contract Revision
해당 field가 실제 소비한 dependency fingerprint
```

`AuthoringRevision`은 UI/diagnostic trace에 함께 기록할 수 있지만 signature의 유일한 변화 근거가 아니다.
DisplayName/Description만 바뀌었는데 resolver payload가 동일하면 Effective Stale을 만들지 않는다.

#### Recipe Fingerprint

Recipe canonical hash에는 Authoring Intent만 포함한다.

포함:

```text
Asset Intent
Profile Bindings
Feel Intent
Mass Intent
Hardpoint / Mount Intent
Default Data Intent
WheelVisual Intent
DriveState Mode
Asset Adoption
Legacy Pin state
Advanced Override state
```

제외:

```text
AppliedState
Last Applied Hash
UI transient selection
Validation report cache
```

AppliedState를 Recipe Fingerprint에 포함하면 Apply할 때마다 자기 자신 때문에 stale이 생기므로 금지한다.

#### Profile Fingerprint

Profile Metadata의 표시 설명보다 **Resolver가 실제 소비하는 payload**를 hash한다.
DisplayName/Description 변경만으로 차량 물리값을 stale 처리하지 않는다.

#### Asset Fingerprint

Section 22.13의 resolver-relevant extracted facts만 hash한다.

#### Hash Algorithm

Hash의 보안성이 게임 규칙은 아니므로 특정 암호 알고리즘을 Architecture 의미로 고정하지 않는다.
P0-08에서는 UE에서 안정적으로 사용할 수 있는 deterministic digest 하나를 선택하고 `ResolverContractRevision`과 함께 버전 관리한다.

중요한 것은 **canonical input set과 ordering**이다.

### 22.28 Stale / Shadow Stale / External Drift

세 상태를 분리한다.

#### Effective Stale

현재 effective source signature가 Last Applied signature와 다름.

예:

```text
Handling Profile 변경
Recipe Feel 변경
Accepted Wheel Asset bounds 변경
Resolver Contract Revision 변경
```

#### Shadow Source Changed

Legacy Pin 또는 Advanced Override가 effective라 target 결과는 그대로지만 그 아래 source가 변경된 상태.

```text
Legacy Pin = effective
Handling Profile 변경
→ Effective Stale 아님
→ Shadow Source Changed = true
```

이 상태는 Apply를 요구하지 않지만 Pin/Override 해제 시 결과가 달라질 수 있음을 알려준다.

#### External Drift

Target `UCFVehicleData` current value가 마지막으로 신뢰한 Definition baseline과 달라진 상태다.

Baseline 선택:

```text
AppliedState가 유효함
→ AppliedState의 field/value hash 사용

Initial Import 후 아직 Apply 전
→ ImportState.ImportedDefinitionHash + Legacy Pin value hash 사용
```

따라서 Import 직후 누군가 Raw DA를 수정해도 첫 Apply까지 조용히 지나가지 않는다.

대표 원인:

```text
Raw DA Editor 직접 수정
외부 Editor tool 수정
다른 스크립트가 Target 수정
```

Resolver는 External Drift를 Recipe에 자동 흡수하지 않는다.

### 22.29 `FCFVehicleStaleReport`

```text
bool bHasEffectiveStale
bool bHasShadowSourceChange
bool bHasExternalDrift

TArray<FCFVehicleStaleField> Fields
```

`FCFVehicleStaleField` 최소 정보:

```text
FieldPath
StaleReason flags
LastAppliedSourceSignature
CurrentSourceSignature
LastAppliedValueHash
CurrentTargetValueHash
```

Stale reason 후보:

```text
RecipeChanged
ProfileChanged
AssetChanged
ResolverChanged
LegacyPinChanged
OverrideChanged
ShadowSourceChanged
ExternalDefinitionChanged
```

Field-level report를 만들기 때문에 Handling Profile 변경을 모든 117 field stale로 표시하지 않는다.

### 22.30 Field Diff Model

`FCFVehicleFieldDiff`는 aggregate struct replacement가 아니라 leaf/stable-array change를 표현한다.

`ECFVehicleDiffOp`:

```text
SetLeaf
AddArrayElement
RemoveArrayElement
MoveArrayElement
```

#### SetLeaf

```text
FieldPath
BeforeValue
AfterValue
Resolved Source Trace
```

#### AddArrayElement

Array element를 opaque struct blob으로 넣지 않는다.

```text
Collection + Stable Selector
TArray<LeafPath + AfterValue>
DesiredOrder
```

#### RemoveArrayElement

```text
Collection + Stable Selector
Before leaf snapshot
```

#### MoveArrayElement

Stable ID는 유지하고 display/serialized order만 명시적으로 바꿀 때 사용한다.
Array reorder가 Source identity 변경으로 취급되지 않는다.

Identity rename은 `SetLeaf`로 처리하지 않는다.

```text
LocationSlotId Old → New
= old element Remove + new element Add
+ LocationSlotRef dependency validation
```

### 22.31 Diff Determinism / Array Apply Order

Diff 출력 정렬도 deterministic해야 한다.

Dependency-safe 기본 순서:

```text
Remove MountProfiles
Remove HardpointSlots

Add HardpointSlots
Add MountProfiles

Set scalar / nested leaf
Set array element leaf
Move array order
```

이 순서는 `MountProfiles[].LocationSlotRef`가 Hardpoint stable ID를 참조하는 현재 계약을 보호한다.

Current Target의 기존 array order를 Import Recipe가 그대로 보존하며, 사용자가 order를 명시적으로 바꾸지 않으면 불필요한 Move diff를 만들지 않는다.

### 22.32 Definition Materialization

Resolver의 sorted leaf set을 실제 `UCFVehicleData` candidate로 만드는 책임은 Resolver Core가 아니라 Field Codec/Materializer가 가진다.

```text
Create transient UCFVehicleData
→ Current C++ defaults 확보
→ ResolvedFields를 Registry path별로 set
→ Arrays는 stable selector 기준 생성
→ transient candidate 완성
```

Import Legacy Serialized field도 reflection UPROPERTY이므로 candidate에 복원할 수 있어야 한다.

이 candidate는:

- Preview validation
- `UCFVDAValidator`
- resolved hash readback test

에 사용한다.
실제 Content Asset mutation은 하지 않는다.

### 22.33 Apply Request — TOCTOU 보호

사용자가 Preview를 본 뒤 Profile/Recipe/Target이 바뀐 상태에서 오래된 Diff를 적용하지 않도록 Apply Request에 precondition을 둔다.

`FCFVehicleApplyRequest` 최소 정보:

```text
Recipe
Target VehicleData
Resolved Diff
ExpectedRecipeFingerprint
ExpectedSourceSignature
ExpectedTargetDefinitionHash
ExpectedResolvedDefinitionHash
ExpectedResolverContractRevision
```

Apply 직전에 현재 값을 다시 계산해 하나라도 다르면:

```text
Apply Blocked = PreviewOutOfDate
```

으로 중단하고 새 Resolve Preview를 요구한다.

자동 retry하거나 오래된 Diff를 강제로 적용하지 않는다.

### 22.34 Apply Service — `FCFVehicleApplyService`

Apply는 `CarFight_ReEditor` C++ service 하나가 소유한다.
Slate와 AI는 이 service 외의 Raw VehicleData write path를 사용하지 않는다.

표준 구현 순서:

```text
A0 ApplyRequest precondition 검사
A1 Resolved Result Error/Blocked 검사
A2 Current Target을 transient duplicate하고 exact Diff 적용
A3 transient candidate UCFVDAValidator 실행
A4 PASS이면 FScopedTransaction 시작
A5 TargetVehicleData->Modify()
A6 Recipe->Modify()
A7 Pre-Apply Target / Recipe applied-state snapshot 확보
A8 Target에 dependency-safe Field Diff 적용
A9 Target readback → ExpectedResolvedDefinitionHash 비교
A10 actual Target UCFVDAValidator 재실행
A11 PASS이면 Recipe AppliedState 갱신
A12 Target + Recipe Package Dirty
A13 PostEditChange / UI refresh notification
A14 transaction 정상 종료
```

실패:

```text
A8~A10 중 실패
→ Target pre-apply snapshot 복원
→ Recipe applied-state 복원
→ transaction cancel/abort
→ partial success 금지
```

자동 Save는 하지 않는다.

### 22.35 Applied State

`FCFVehicleAppliedState` 최소 정보:

```text
int32 AppliedRecipeRevision
FString AppliedRecipeFingerprint
FString AppliedSourceSignature
FString AppliedDefinitionHash
int32 ResolverContractRevision

TArray<FCFVehicleAppliedTrace> FieldTraces
```

각 field trace:

```text
FieldPath
EffectiveSourceType
EffectiveSourceId
EffectiveSourceSignature
ShadowSourceSignature
LastAppliedValueHash
```

Profile 별 revision/path를 별도 duplicated table로 다시 저장할 필요는 없지만 UI 요약을 위해 Applied Source Summary를 추가할 수 있다.
Field trace가 stale 판정의 Authority다.

### 22.36 External Raw DA Edit 처리

기존 Raw DA Editor는 계속 허용한다.
다만 새 managed Recipe가 있는 Target을 Raw Editor에서 수정하면 Source Tracking과 Definition이 분리될 수 있다.

다음 open/refresh 시:

```text
Current Target Snapshot
vs AppliedState Field Hash
→ External Drift report
```

사용자 선택:

```text
1. Re-Import / Rebase
   Raw 변경을 새 Legacy Pin/candidate로 가져옴

2. Re-Apply Authoring
   Recipe/Profile Resolver 결과로 Raw 변경을 덮음

3. Cancel
   아무 것도 수정하지 않음
```

Raw 변경을 자동으로 `AdvancedOverride`로 변환하는 것은 금지한다.

### 22.37 Import / Rebase 차이

#### Initial Import

managed Recipe가 없는 Existing Definition에서 Recipe를 최초 생성한다.

#### Rebase

이미 Recipe가 있으나 External Drift가 발생한 Definition의 current raw 값을 다시 Authoring baseline 후보로 읽는다.

Rebase도 즉시 source ownership을 바꾸지 않는다.
변경 field를:

```text
New Legacy Pin으로 보존할지
기존 Authoring Source를 유지하고 버릴지
Advanced Override로 명시적으로 승격할지
```

Preview에서 선택하게 한다.

P0-04/P0-05에서 실제 UX를 설계한다.

### 22.38 Validation Contract 구체화

#### Recipe Validation

```text
Target binding format
Managed source가 필요한 Profile binding 존재 여부
Duplicate Hardpoint LocationSlotId
Duplicate MountProfileId
Mount LocationSlotRef 존재
Duplicate Advanced Override path
Override type signature
identity field override 금지
Asset intent mode / reference 일관성
DriveStateMode / Profile binding 일관성
```

Profile binding 요구는 상태를 고려한다.

- New/fully Managed Vehicle에서는 Vehicle Base / Drivetrain / Handling / Performance binding을 요구한다.
- DriveState Profile은 `DriveStateMode=VehicleSpecific`일 때만 effective binding을 요구한다.
- Legacy/Partially Managed Vehicle은 아직 Legacy Pin이 소유하는 group의 Profile binding이 없어도 Import 자체를 Block하지 않는다. 해당 group을 Adopt하려는 Preview 시점에는 새 owner source가 반드시 준비되어 있어야 한다.

#### Resolver Validation

```text
동일 field source conflict
missing required source
Drivetrain cross-field consistency
Asset Socket missing
Accepted measurement fingerprint mismatch
MassScale enabled but mass context missing
Derived gate precondition
Legacy Pin / Adopt state conflict
```

#### Definition Validation

기존 `UCFVDAValidator`를 materialized transient candidate에 사용한다.

새 병렬 `VehicleAuthoringValidator`로 `UCFVDAValidator` 규칙을 복제하지 않는다.

### 22.39 Resolver Contract Revision

Field mapping이나 Resolver 수학이 바뀌면 Profile/Recipe revision만으로 stale을 설명할 수 없다.
따라서 코드에 별도 `ResolverContractRevision`을 둔다.

P0 최초 구현 revision:

```text
ResolverContractRevision = 1
```

증가 대상 예:

```text
Feel mapping 공식 변경
Mass context 적용 공식 변경
Field Resolver ownership 변경
Asset measurement 공식 변경
Derived gate rule 변경
Canonical field serialization 의미 변경
```

증가하지 않는 예:

```text
Slate 배치 변경
텍스트/Tooltip 변경
Validation message 문구 변경
성능 최적화만 하고 결과가 동일
```

Revision 증가 시 해당 resolver-owned field는 stale 후보가 되며 자동 Apply하지 않는다.

### 22.40 C++ / Blueprint / Slate 배분

P0 핵심 Authoring은 C++로 구현한다.

#### C++

```text
Recipe/Profile schema
Field Registry
Reflection Codec
Asset Snapshot Reader
Resolver
Source Trace
Hash/Stale
Diff
Import/Adoption
Apply Transaction
Automation Test
```

#### Slate

P0-05 이후:

```text
Recipe form
Profile selection
Source trace visualization
Measurement proposal
Diff
Stale/Drift
Validation
Apply/Undo
```

#### Blueprint

P0 핵심 계약에는 Blueprint가 필요하지 않다.

Blueprint/EUW는:

- 일회성 prototype
- read-only presentation
- 특정 Editor UX experiment

정도로만 사용할 수 있으며 Resolver/Apply의 별도 BP 구현을 만들지 않는다.

이 배분은 현재 1인 개발에서 계산/Mutation 로직이 Slate와 BP에 분산되는 것을 막고 AI도 같은 C++ contract를 호출할 수 있게 한다.

### 22.41 P0-08 Automation 설계 입력

P0-03에서 최소 다음 자동 테스트 요구를 미리 고정한다.

```text
1. FieldRegistry_Coverage117
2. FieldPath_ArrayReorderStable
3. FieldCodec_RoundTrip_AllTypes
4. Resolver_DeterministicSameInput
5. Resolver_ProfileSingleOwner
6. Resolver_FeelResponseEndpoints
7. Resolver_LegacyPinWins
8. Resolver_AdvancedOverrideWins
9. Resolver_ShadowChangeNotEffectiveStale
10. Resolver_AssetFingerprintStale
11. Resolver_ExternalDriftDetect
12. Diff_NoAggregateStructWrite
13. Diff_ArrayStableIdAddRemove
14. Apply_PreviewOutOfDateBlocked
15. Apply_TransactionUndo
16. Apply_PostValidateRollback
17. Import_NoDefinitionMutation
18. Import_NoQuickTuneIntentInference
19. Fitting_NoAuthoringDependency
20. Runtime_UCFVehicleDataContractUnchanged
```

USER Driving Feel/Visual Acceptance는 이 Automation 목록에 포함하지 않는다.

### 22.42 P0-03 완료 판정

Roadmap의 P0-03 완료 조건을 다음처럼 충족한다.

```text
C++ 클래스 후보와 책임이 겹치지 않음
→ Recipe/Profile asset / Pure Resolver / Registry-Codec / Apply 분리

UI 없이 Resolver 단독 테스트 가능
→ Immutable ResolveRequest + snapshot 구조

Asset Mutation 없이 Preview 가능
→ AssetReader snapshot 후 pure field resolve

동일 입력 Deterministic 결과 가능
→ stable field path + canonical field value + fixed stage order
```

추가로:

```text
117 leaf Registry coverage 계약 확보
Legacy Pin / Adopt / Rebase 구체화
Stale / Shadow Stale / External Drift 분리
TOCTOU-safe Apply precondition 확보
Array Stable-ID diff/apply 정의
P0-08 구현 파일 경계 확정
```

따라서 **DAUTH-P0-03 Vehicle Recipe / Profile / Resolver Design은 Complete / P0-04 Ready**로 판정한다.

---

## 23. DAUTH-P0-04 VDA Wizard Migration Design

### 23.1 P0-04 상태와 범위

P0-04는 `DataAuthoringDesign.md v0.4.0` Section 22의 Recipe / Profile / Resolver / Diff / Apply 계약을 변경하지 않고, Current `SCFVDAWizardTab`의 기능을 새 Authoring Workspace로 어떻게 이전할지 고정한다.

```text
DAUTH-P0-04 = Migration Design Complete
다음 단계 = DAUTH-P0-05 Vehicle Authoring UX Design
Project 상태 = Working / Pre-Implementation 유지
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
```

Current Source Authority:

```text
CarFight_ReEditor/Public/CFVDAWizardTab.h
CarFight_ReEditor/Private/CFVDAWizardTab.cpp
CarFight_Re/Public/CFVDAValidator.h
CarFight_Re/Private/CFVDAValidator.cpp
CarFight_Re/Public/CFVehicleData.h
CarFight_Re/Private/CFVehicleData.cpp
```

이번 단계에서는 위 Source를 수정하지 않는다.

### 23.2 Migration 판정 용어

각 기능의 판정은 다음 의미로 사용한다.

```text
Reuse As-Is
= 현재 핵심 구현/계약을 의미 변경 없이 새 Workspace에서도 직접 재사용 가능

Refactor and Reuse
= 사용자 기능과 유효한 계산/Editor primitive는 보존하되
  상태 소유자 또는 호출 경계를 P0-03 서비스로 이동

Replace
= 현재 구현 의미가 P0-03 계약과 충돌하여
  새 Resolver/Diff/Apply 구현이 기능의 Authority를 대체

Retire After Parity
= 신규 경로가 기술/사용자 parity를 증명할 때까지 Current 기능을 유지하고
  Gate 통과 뒤 Deprecated/Hidden/Delete 대상으로 전환
```

하나의 사용자 기능 안에서도 내부 구현은 서로 다른 lifecycle을 가질 수 있다.
예를 들어 Layout Capture의 **socket extraction 의미는 Refactor and Reuse**지만 `UCFVehicleData::CaptureLayoutFromChassisSockets()`라는 direct-mutation entry는 **Retire After Parity**다.

### 23.3 Current Wizard 실제 책임 감사

Current `SCFVDAWizardTab`은 하나의 Slate class 안에서 다음 책임을 동시에 수행한다.

```text
Selection / Object Path Load
Validation Orchestration
Reference Compare
Layout Direct Mutation
Driving Feel UI State
Driving Feel Raw Calculation
Driving Feel Reverse Inference
Quick Tune Direct Mutation
Ad-hoc Revert Snapshot
Transaction Ownership
Report Presentation
Raw Asset Editor Open
```

이 구조는 작은 P0 도구로는 동작하지만 새 Authoring Contract에서는 다음 세 문제가 있다.

1. UI class가 계산과 Asset mutation을 직접 소유한다.
2. `Source DA` reference compare가 `UCFVDAValidator` validation report에 합쳐져 Apply eligibility와 비교 의미가 섞일 수 있다.
3. Quick Tune의 raw inverse와 load-time revert snapshot은 Persistent Recipe / Source Tracking / External Drift를 설명하지 못한다.

따라서 새 Workspace는 **Selection / Presentation만 Slate**, 계산/validation orchestration/mutation은 P0-03 service로 분리한다.

### 23.4 기능별 최종 Migration Map

| Current 기능 | Current 구현 | 판정 | 새 Authority | Legacy 처리 |
|---|---|---|---|---|
| Target 선택 | Content Browser 선택 + Path TextBox + `TWeakObjectPtr<UCFVehicleData>` | Refactor and Reuse | Workspace Selection + Recipe `TargetVehicleData` binding | 기존 선택 UX는 parity까지 유지 |
| Source 선택 | optional Source DA 선택/clear | Refactor and Reuse | Read-only Reference Compare selection | Source는 Resolver input owner가 아님 |
| Object Path 직접 Load | `FSoftObjectPath::TryLoad()` | Retire After Parity | typed asset picker / Data Browser selection + soft binding | Advanced fallback 필요 여부는 P0-05 결정 |
| Compare | `UCFVDAValidator::CompareVehicleData()` 일부 field threshold 비교 | Replace | 117-leaf Registry 기반 Reference Compare | legacy compare는 parity 후 Workspace에서 사용 중단 |
| `UCFVDAValidator` | `ValidateVehicleData(Target, Source)` | Reuse As-Is | Definition Validation | 새 Workspace는 candidate validation에 Source=null 사용 |
| Validator Report UI | row/list/detail/report text | Refactor and Reuse | Unified Validation presentation adapter | 기존 UI는 parity까지 유지 |
| Layout Capture | `CaptureLayoutFromChassisSockets()` direct write | Refactor and Reuse | Asset Snapshot Reader + Resolver Asset Derived | direct-mutation entry는 parity 후 retire |
| Driving Feel 4축 | Slate float state 4개 | Refactor and Reuse | Recipe `FCFVehicleFeelIntent` | 4축 의미/표시는 보존 |
| Sedan/SUV/Sports/Heavy preset | 4축 hard-coded semantic 값 | Refactor and Reuse | Recipe 4축을 바꾸는 UI convenience command | 별도 Source/Profile로 승격 금지 |
| Driving Feel Raw Mapping | Wizard cpp의 hard-coded Min/Max `FMath::Lerp` | Replace | Handling/Performance Profile + Pure Resolver | legacy constants는 migration parity 후 제거 |
| Driving Feel Reverse Inference | raw 13값 평균 → 4 slider | Retire After Parity | Existing Definition Import는 Legacy Pin; authoritative inverse 없음 | 새 Workspace에서 authoritative intent 생성 금지 |
| Quick Tune Preview | 13 raw Movement text | Replace | `FCFVehicleResolveResult` + Source Trace + Field Diff + Validation | old preview는 legacy Wizard에만 유지 |
| Quick Tune Apply | `ApplyDrivingFeelValuesToData()` direct write | Replace | `FCFVehicleApplyService` | managed target direct write 금지 |
| Layout Apply | Wizard transaction + DA method direct write | Replace | Resolve Preview → Diff → Apply Service | managed target direct write 금지 |
| `FScopedTransaction` | feature handler별 transaction | Reuse As-Is | Apply Service / Recipe mutation service | primitive는 유지, ownership만 이동 |
| Quick Tune Revert | Target load-time raw snapshot 재적용 | Retire After Parity | Unreal Undo + source-aware re-resolve/rebase | 신규 persistent revert source를 만들지 않음 |
| Raw DA Open | `UAssetEditorSubsystem::OpenEditorForAsset()` | Reuse As-Is | Advanced / Debug action | managed asset direct edit는 External Drift 가능 표시 |
| Report Clipboard Copy | `FPlatformApplicationMisc::ClipboardCopy()` | Reuse As-Is | Validation/Diff report export convenience | SSOT가 아님 |

### 23.5 Target Selection Migration

Current Target selection은 다음 일을 한 번에 한다.

```text
Content Browser 선택
→ UCFVehicleData cast
→ Target weak pointer 저장
→ Path TextBox 동기화
→ Revert snapshot 캡처
→ Raw Movement에서 4축 slider 역산
```

새 Workspace에서는 selection이 Authoring mutation을 유발하지 않는다.

표준 흐름:

```text
Vehicle Definition 선택
→ 연결 Recipe 탐색
   ├─ Recipe 있음  → Recipe + Target binding load
   └─ Recipe 없음  → Unmanaged Existing Definition 상태
→ Current Definition Snapshot
→ Stale / Drift 확인
→ 화면 상태 갱신
```

Target 선택 자체로 다음을 하면 안 된다.

- Recipe 자동 생성
- Existing Definition 자동 Import
- Quick Tune inverse를 Recipe intent로 저장
- Target Asset 수정
- Package Dirty

Existing Definition Import는 별도 명시 Action이다.

### 23.6 Source Selection / Reference Compare Migration

Current `SourceVehicleData`는 `ValidateVehicleData(Target, Source)`에 전달되어 validation과 reference comparison이 하나의 report로 합쳐진다.

새 Workspace에서는 Source를 두 역할로 분리한다.

```text
Current Target Definition
= Resolver / Diff / Apply precondition 대상

Reference Vehicle
= 사람이 비교를 위해 선택하는 read-only reference
```

Reference Vehicle은 다음의 Source가 아니다.

- Recipe Source
- Profile Source
- Resolver precedence source
- Legacy Pin Source
- Apply value source

즉 Source DA를 선택했다고 Target 값이 Source 값을 상속하지 않는다.

### 23.7 Compare Migration — Validation과 분리

Current `CompareVehicleData()`는 일부 주요 field만 비교하고 각 field에 hard-coded difference threshold를 사용한다.

대표 범위:

```text
MovementProfileName
bUseMovementOverrides
Torque / RPM / Throttle
Steer / Wheel Radius / Width
Friction / Spring
COM gate
WheelVisual 일부
Base/Gross Mass
DriveState 일부
```

이 기능은 **전체 Definition diff가 아니며 117 leaf coverage를 보장하지 않는다.**
또 차이가 크다는 이유로 Warning을 내는 threshold는 Authoring source/diff semantics와 동일하지 않다.

따라서 새 Workspace의 Compare는 두 종류로 분리한다.

#### A. Authoring Diff — Apply Authority

```text
Resolved Result
vs
Current Target Definition Snapshot
```

- Field Registry 117 leaf coverage.
- Stable-ID array diff.
- exact before/after value.
- Source Trace 포함.
- Apply Service의 실제 mutation input.
- 임의 threshold로 차이를 숨기지 않음.

#### B. Reference Compare — Read-only 제작 보조

```text
Selected A Definition/Resolved Snapshot
vs
Selected B Definition/Resolved Snapshot
```

- 동일 Field Registry / Field Codec 사용.
- 117 leaf 기준 Changed / Same 필터 가능.
- Stable-ID array identity 사용.
- Apply eligibility와 무관.
- Warning/Error severity를 단순 값 차이에서 생성하지 않음.

Current `CompareVehicleData()`의 threshold 비교 메시지는 migration 동안 legacy diagnostic으로만 남긴다.
새 Workspace가 이를 Authoring Diff로 호출해서는 안 된다.

### 23.8 `UCFVDAValidator` Migration

`UCFVDAValidator` 자체는 **Reuse As-Is**다.

P0-03에서 정한 것처럼 materialized transient candidate의 Definition invariant 검증에 계속 사용한다.

새 Authoring Validation 흐름:

```text
Recipe Validation
→ Resolver Validation
→ transient UCFVehicleData candidate
→ UCFVDAValidator::ValidateVehicleData(Candidate, nullptr)
→ Definition Validation Result
```

여기서 `SourceVehicleData=nullptr`를 사용하는 이유는 Reference Compare를 Definition validity와 분리하기 위해서다.

Current overload/signature를 바꿀 필요는 없다.

`UCFVDAValidator`의 향후 field invariant 보강은 별도 Definition validation 개선이며 P0-04 Migration을 이유로 병렬 Validator를 만들지 않는다.

### 23.9 Validator UI Migration

Current Wizard의 다음 presentation 기능은 재사용 가치가 있다.

```text
Severity Summary
Result Row
Selected Row Detail
Field Path
Message
Recommended Action
Clipboard Report
```

하지만 새 Workspace는 다음 validation source를 동시에 보여줘야 한다.

```text
Recipe Validation
Resolver Validation
Definition Validation
Stale / External Drift
```

따라서 Current `FCFVDARow` / report text 형식 자체를 SSOT로 재사용하지 않고, 새 공통 presentation row로 Refactor한다.

최소 표시 항목:

```text
Layer
Severity
Stable Field Path 또는 Validator FieldPath
Message
Recommended Action
Apply Blocking 여부
```

Current Validator의 string `FieldPath`가 P0-03 `FCFVehicleFieldPath` Registry와 일치하면 UI adapter가 structured path로 연결한다.
Registry에 없는 Validator diagnostic path는 validation-only text path로 보존하고 억지로 fake field identity를 만들지 않는다.

### 23.10 Layout Capture Migration

Current `UCFVehicleData::CaptureLayoutFromChassisSockets()`는 다음을 수행한다.

```text
ChassisMesh socket read
→ 4 WheelAnchor pose 계산
→ 4개 모두 실패 없는지 검사
→ Modify()
→ bUseLayoutOverrides=true
→ socket name + 4 pose 직접 저장
→ Hardpoint SocketName이 있는 slot은 LocalLocation/Rotation 직접 저장
→ MarkPackageDirty()
→ notification
```

또한 Current 정책은:

- 4 Wheel socket 중 하나라도 없으면 wheel layout 전체 적용 중단.
- Hardpoint `SocketName=None`이면 기존 manual transform 유지.
- Hardpoint socket missing은 wheel capture 성공 자체를 실패로 만들지 않고 warning으로 취급.

이 의미는 보존한다.

새 흐름:

```text
Recipe Asset/Socket Intent
→ FCFVehicleAssetReader
→ FCFVehicleAssetSnapshot
→ Resolver R5 Asset Socket Derived
→ Layout/Hardpoint Source Trace
→ Field Diff
→ Validation
→ FCFVehicleApplyService
```

#### 재사용할 것

- Current wheel socket fallback convention.
- StaticMesh socket → RelativeLocation / RelativeRotation extraction 의미.
- 4 wheel socket atomic validity 의미.
- Hardpoint optional capture / warning 의미.

#### 재사용하지 않을 것

- Asset read와 `UCFVehicleData` write가 결합된 함수 경계.
- `Modify()` / `MarkPackageDirty()`를 Capture 함수가 직접 소유하는 구조.
- Capture 자체가 즉시 `bUseLayoutOverrides=true`를 쓰는 구조.

P0-10 구현 시 pure socket extraction helper를 분리해 Current CallInEditor path와 새 AssetReader가 같은 extraction 의미를 공유하는 것을 우선한다.
필요하다면 `CarFight_Re`의 `#if WITH_EDITOR` pure helper로 추출할 수 있지만 `CarFight_ReEditor`에 대한 역방향 의존은 절대 만들지 않는다.

Current `CaptureLayoutFromChassisSockets()` direct-mutation entry는 새 Workspace parity 전까지 삭제하지 않는다.

### 23.11 Managed Target Legacy Write Guard

P0-10 이후 managed Recipe가 연결된 Target을 Legacy Wizard에서 직접 수정하면 Source Tracking / AppliedState를 우회한다.
따라서 P0-10 migration 구현에는 **모든 Legacy Wizard write action**에 Editor-side managed-target guard가 필요하다.

차단 대상:

```text
Layout Capture
Driving Feel Quick Tune Apply
Driving Feel Quick Tune Revert
향후 legacy Wizard에 남아 있는 다른 direct Target mutation action
```

표준 흐름:

```text
Legacy Wizard에서 Target 선택
→ Target을 가리키는 managed Recipe 탐색
   ├─ 없음 → legacy direct write 임시 허용
   └─ 있음 → legacy write button Block
              "Data Authoring에서 Preview/Apply하세요"
```

Read-only action은 계속 허용할 수 있다.

```text
Validate
Reference Compare
Open Raw DA
Copy Report
```

`UCFVehicleData` Runtime class에 Recipe reference를 추가하지 않는다.
Guard는 `CarFight_ReEditor`에서 Recipe Asset을 조회한다.

Raw Asset Editor 직접 편집과 `CaptureLayoutFromChassisSockets()` CallInEditor 같은 **의도적으로 남긴 Advanced/Legacy escape hatch**는 Wizard guard와 구분한다.
이 경로의 out-of-band mutation은 다음 Workspace refresh에서 External Drift로 확실히 검출해야 한다.
최종 deprecation 단계에서는 Layout CallInEditor entry의 노출 축소/제거를 별도 Source migration으로 검토한다.

### 23.12 Driving Feel 4축 Migration

Current 4축 의미는 그대로 유지한다.

```text
Acceleration Feel
Steering Agility
Grip Feel
Suspension Firmness
```

그러나 Slate transient float가 Authority가 되지 않는다.

새 흐름:

```text
Slider edit
→ Recipe FCFVehicleFeelIntent 변경
→ Recipe AuthoringRevision / Fingerprint 변경
→ Resolve Preview
→ Handling / Performance Profile response
→ Raw VehicleData field result
```

Slider 이동만으로 Target Definition은 수정되지 않는다.
Recipe edit 자체는 `FScopedTransaction`으로 Undo 가능해야 한다.

### 23.13 Named Driving Feel Preset Migration

Current exact semantic preset은:

```text
Sedan  = 0.50 / 0.50 / 0.55 / 0.45
SUV    = 0.38 / 0.34 / 0.50 / 0.38
Sports = 0.85 / 0.78 / 0.82 / 0.78
Heavy  = 0.28 / 0.25 / 0.45 / 0.55
```

이 값은 **Profile Source가 아니라 4개 Recipe semantic input을 한 번에 입력하는 UI convenience**로만 해석한다.

Migration 규칙:

1. P0-10 parity까지 exact 4축 값을 보존한다.
2. Preset click은 Recipe 4축만 수정한다.
3. `SourceType=SedanPreset` 같은 새 Source layer를 만들지 않는다.
4. Vehicle Base/Handling/Performance 5 Domain Profile 구조와 합치거나 여섯 번째 Profile로 만들지 않는다.
5. P0-05 UX에서 최종 버튼 노출 여부/이름은 바꿀 수 있지만 semantic shortcut 기능 손실 없이 대체 위치를 제공한다.
6. Preset 값이 좋은 주행감을 보장한다고 기록하지 않는다.

### 23.14 Driving Feel Raw Mapping Migration

Current `BuildDrivingFeelValues()`는 Wizard cpp 내부 hard-coded Min/Max를 `FMath::Lerp`하여 13개 raw value를 만든다.

```text
bUseMovementOverrides
EngineMaxTorque
EngineMaxRPM
ThrottleInputScale
FrontWheelMaxSteerAngle
SteeringAngleRatio
Front/Rear Friction
Front/Rear CorneringStiffness
Front/Rear SpringRate
Front/Rear SpringPreload
```

새 Workspace에서는 이 계산을 호출하지 않는다.
P0-03의:

```text
FCFFeelResponse
Handling Profile
Performance Profile
optional FCFMassScaleRule
Pure Resolver
```

가 대체 Authority다.

Current hard-coded Min/Max는 P0-10 parity 검증의 **legacy behavior reference**일 뿐 새 Profile 값으로 자동 복사하거나 정답으로 승격하지 않는다.
실제 Profile content authoring은 P0-08/P0-09 implementation에서 별도 근거로 생성한다.

### 23.15 Driving Feel Reverse Inference Retirement

Current Target 선택/검사 시:

```text
Raw Movement 13값
→ 각 legacy Min/Max Normalize
→ 평균
→ 4 slider 값
```

을 수행한다.

이 기능은 P0-03 Existing Definition Import 계약과 충돌한다.

새 Workspace 규칙:

```text
Existing Definition에 Recipe 없음
→ 4축 값 "Unknown / Not Authored"
→ Legacy Pin으로 raw 값 보존
→ optional 참고용 legacy estimate를 표시할 수는 있음
→ estimate를 Recipe에 자동 저장 금지
```

따라서 authoritative reverse inference는 **Retire After Parity**다.

P0-05 UX에서 참고 estimate를 보여주더라도 반드시 `추정값 / Source Unknown`으로 표시하고 Adopt 동작과 분리한다.

### 23.16 Preview Migration

Current Quick Tune Preview는 계산된 13 raw value의 텍스트 목록이며:

- Current Target과의 exact diff가 아님.
- Source Trace 없음.
- 117 field 영향 없음.
- Stale/Drift 없음.
- Apply precondition hash 없음.

따라서 완전히 Replace한다.

새 Preview 최소 구성:

```text
Recipe Intent Summary
Resolved Field Changes
Before / After
Source Trace
Measurement Proposal
Legacy Pin / Override 표시
Recipe/Resolver/Definition Validation
Stale / Shadow Source Changed / External Drift
Apply Blocking Reason
```

Preview는 `FCFVehicleResolveResult` 하나에서 파생하며 UI가 raw 계산을 새로 하지 않는다.

### 23.17 Apply Migration

Current Wizard의 두 mutation lane:

```text
Layout
→ FScopedTransaction
→ Target Modify
→ CaptureLayoutFromChassisSockets direct mutation

Driving Feel
→ FScopedTransaction
→ Target Modify
→ ApplyDrivingFeelValuesToData direct mutation
→ MarkPackageDirty
```

둘 다 새 Workspace에서는 금지한다.

새 단일 mutation lane:

```text
Recipe / Asset / Profile edit
→ Resolve
→ Diff
→ Validate
→ FCFVehicleApplyRequest precondition
→ FCFVehicleApplyService
→ Single logical FScopedTransaction
→ Target + AppliedState atomic update
→ Revalidate
```

Slate button handler는 `ApplyService` 호출 외에 `UCFVehicleData` field를 직접 set하지 않는다.

### 23.18 `FScopedTransaction` Migration

`FScopedTransaction` 자체는 **Reuse As-Is**다.

다만 ownership을 UI handler에서 service로 이동한다.

```text
Recipe Semantic Edit
→ Recipe mutation service / editor command transaction

Legacy Adopt / Rebase choice
→ Recipe mutation transaction

Definition Apply
→ FCFVehicleApplyService transaction
```

금지:

- Slate와 ApplyService가 같은 Definition mutation을 각각 별도 transaction으로 감싸기.
- Apply 성공 뒤 ad-hoc Revert snapshot을 또 SSOT처럼 보존하기.

Undo/Redo 시 Recipe revision / AppliedState도 transaction과 함께 일관되게 복원되어야 한다.

### 23.19 Revert Migration

Current Quick Tune Revert는 Target을 선택/검사한 순간의 13 raw value를 메모리에 저장했다가 다시 직접 쓰는 기능이다.

한계:

- Editor 재시작 후 보존되지 않음.
- Capture 뒤 외부 Raw edit가 있으면 snapshot freshness를 설명하지 못함.
- Recipe/Profile Source를 복원하지 못함.
- Quick Tune이 건드린 13개 field만 되돌림.

새 Workspace에서 별도 `RevertSnapshot` SSOT는 만들지 않는다.

대체 기능:

```text
즉시 직전 edit/apply
→ Unreal Undo

Unapplied Recipe edit 취소
→ Recipe transaction Undo / UI discard

External Drift
→ Rebase / Re-Apply / Cancel

Legacy Pin 해제 전 상태 확인
→ Adoption Preview Cancel
```

따라서 Current Quick Tune Revert button은 새 Workspace parity 후 retire한다.

### 23.20 Raw DA Open Migration

`UAssetEditorSubsystem::OpenEditorForAsset()`는 그대로 재사용한다.

새 Workspace에서 위치:

```text
Advanced / Debug
→ Open Raw Vehicle Data
```

Managed Target이면 열기 전/상단에 다음 의미를 알려야 한다.

```text
직접 수정은 허용됨
하지만 Recipe Source Tracking 밖의 변경이므로 External Drift가 발생할 수 있음
```

Raw DA Open 자체를 금지하지 않는다.
P0-03 escape hatch 계약을 유지한다.

### 23.21 Current Wizard Freeze 범위

P0-04 완료 후 P0-10 migration 착수 전까지 `SCFVDAWizardTab`은 **Legacy Current Tool**로 Freeze한다.

허용:

- 명백한 crash/data-loss bug fix.
- 현재 Runtime/Validator schema 변화에 따른 필수 compatibility fix.
- 기존 기능을 보존하기 위한 최소 correction.

금지:

- 새 Quick Tune axis 추가.
- 새로운 hard-coded balance threshold/preset 추가.
- 새 direct `UCFVehicleData` mutation 기능 추가.
- 새 Authoring Source 개념을 Wizard 내부에 독자 구현.
- P0-03 Resolver와 별개인 두 번째 Recipe/Profile 계산 경로 추가.
- managed Target에 대한 새 bypass write 기능 추가.

Current legacy values는 migration reference로 보존하되 새 Architecture의 기준값이라고 해석하지 않는다.

### 23.22 단계별 Coexistence Plan

#### P0-08 Implementation Foundation

```text
새 Authoring Core 구현
Old Wizard = 그대로 유지
Old Wizard write path = 아직 Current legacy path
신규 Core가 Old Wizard에 의존하지 않음
```

#### P0-09 Vehicle Authoring MVP

새 Workspace가 우선 확보할 parity:

```text
Target selection
Recipe/Import status
Resolve Preview
117 Field Diff
Source Trace
Definition Validation
Apply/Undo
Raw DA Open
```

이 단계에서도 old Wizard를 삭제하지 않는다.

#### P0-10 Existing Wizard Migration

다음 migration을 완료한다.

```text
Reference Compare
Layout Asset Derived
Driving Feel 4축
Named semantic preset shortcut
Unified Validation presentation
Legacy managed-target guard
```

P0-10 완료 시 old Wizard는 `Deprecated` 후보가 된다.

### 23.23 Managed / Unmanaged Coexistence

Migration 중에는 두 상태가 공존한다.

```text
Unmanaged VehicleData
- Recipe 없음
- Legacy Wizard 사용 가능
- Raw DA direct edit 가능

Managed / Partially Managed VehicleData
- Recipe 있음
- Data Authoring Workspace가 정상 authoring path
- Legacy Wizard write action은 Block 대상
- Raw DA direct edit는 External Drift escape hatch
```

Recipe 존재 여부만으로 Existing Definition을 자동 Import하거나 값을 바꾸지 않는다.
Recipe 생성은 explicit Import/Create action의 결과여야 한다.

### 23.24 Deprecated Gate

기존 Wizard를 메뉴에서 Deprecated/Legacy로 내리거나 기본 진입에서 숨기기 위한 Gate:

```text
DG1 P0-09에서 Target / Preview / 117 Diff / Validator / Apply / Undo / Raw Open parity
DG2 P0-10에서 Reference Compare / Layout / Driving Feel / preset shortcut parity
DG3 managed Target의 Legacy Wizard direct write는 차단되고, Raw DA/CallInEditor escape-hatch mutation은 External Drift로 확실히 검출됨
DG4 Old Wizard 없이 새 Vehicle과 Existing Imported Vehicle의 기술 workflow를 끝까지 수행 가능
DG5 Automation에서 old/new 결과의 의도된 migration 차이가 문서화됨
```

DG1~DG5 전에는 기존 메뉴 진입을 제거하지 않는다.

Deprecated 상태에서는:

- 이름/배너에 `Legacy Vehicle DA Wizard` 표시 가능.
- managed Target write button은 비활성화.
- Unmanaged legacy vehicle을 위한 임시 read/diagnostic path는 남길 수 있음.
- 새 기능은 추가하지 않음.

### 23.25 Wizard Delete Gate

`SCFVDAWizardTab` Source를 실제 삭제할 수 있는 조건은 Deprecated Gate보다 강하다.

```text
DEL1 DG1~DG5 전부 PASS
DEL2 P0-11 Technical Validation PASS
DEL3 Inventory / Fitting / Vehicle Runtime non-regression PASS
DEL4 기존 Wizard가 제공하던 Target/Compare/Validator/Layout/Feel/Preview/Undo/Raw Open 기능의 replacement route 존재
DEL5 USER P0-12 Authoring Acceptance에서 새 workflow 사용 가능 확인
DEL6 Current Source/메뉴/문서에서 SCFVDAWizardTab 직접 참조를 inventory하고 필수 caller 0 확인
DEL7 old Wizard 삭제가 Existing VehicleData 자동 migration을 요구하지 않음
```

DEL1~DEL7 이전에는 물리 삭제 금지다.

중요:

- Wizard UI 삭제와 `UCFVehicleData::CaptureLayoutFromChassisSockets()` CallInEditor entry 삭제는 별도 Gate다.
- Raw DA escape hatch를 유지해야 한다면 Wizard 삭제 후에도 VehicleData Editor 자체는 계속 존재한다.
- Layout CallInEditor entry는 새 AssetReader parity, caller inventory, External Drift 정책 확인 뒤 별도 migration에서 제거/Deprecated할 수 있다.

### 23.26 P0-10 Source Migration 예상 범위

실제 Source 변경은 P0-10에서 수행하며 P0-04에서는 설계만 고정한다.

예상 방향:

```text
SCFVDAWizardTab
- 신규 기능 추가 금지
- managed target guard 추가
- Deprecated banner / redirect 가능

CFVehicleData socket capture
- pure extraction 부분 분리 검토
- existing direct mutation wrapper는 parity까지 유지

New Data Authoring Workspace
- Recipe selection/edit
- Reference Compare
- Resolver Preview
- Validation adapter
- Layout proposal/adoption
- Driving Feel
- Apply Service
- Raw DA Open
```

Current `FCFDrivingFeelValues`, hard-coded Quick Tune constants, reverse inference helper는 new Workspace가 사용하지 않는다.
삭제는 P0-10 parity 이후 old Wizard lifecycle에 맞춰 수행한다.

### 23.27 P0-04 완료 판정

Roadmap P0-04 완료 조건:

```text
기존 기능 손실 없이 새 위치가 정의됨
→ Target/Source/Compare/Validator/Layout/Feel/Preview/Undo/Revert/Raw Open 전부 Migration Map 보유

기존 Wizard 삭제 시점이 명확함
→ Deprecated Gate DG1~DG5 + Delete Gate DEL1~DEL7 고정

Migration 전까지 기존 코드 Freeze 범위가 명확함
→ Section 23.21 Legacy Current Tool Freeze 확정
```

추가로 다음 경계를 고정했다.

```text
Definition Validation과 Reference Compare 분리
Managed Target legacy direct write 차단
Layout socket extraction 의미 보존 + direct mutation shell retirement
4 Driving Feel semantic 축 보존 + raw mapping/reverse inference retirement
FScopedTransaction primitive 재사용 + Apply ownership 중앙화
Raw DA Open 유지 + External Drift escape hatch
```

따라서 **DAUTH-P0-04 VDA Wizard Migration Design은 Complete / P0-05 Ready**로 판정한다.

---

## 24. DAUTH-P0-05 Vehicle Authoring UX Design

### 24.1 P0-05 상태와 범위

P0-05는 Section 22의 P0-03 Core 계약과 Section 23의 P0-04 Wizard Migration 판정을 변경하지 않고, 실제 1인 개발 일상 제작에서 사용할 Vehicle Authoring Workspace의 화면 구조와 사용자 행동 계약을 고정한다.

```text
DAUTH-P0-05 = UX Design Complete
다음 단계 = DAUTH-P0-06 AI Authoring Contract
Project 상태 = Working / Pre-Implementation 유지
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
```

이번 단계는 UI mockup의 시각 스타일을 확정하는 단계가 아니다.
다음의 **정보 구조 / 상태 / 행동 / 안전 경계**를 구현 가능한 Slate UX 계약으로 확정한다.

```text
Vehicle Browser
New Vehicle
Existing Definition Import
Managed / Unmanaged 상태
Recipe / Profile 편집
Assets / Layout
Driving Feel
Hardpoint / Mount / Defaults
Measurement Proposal / Adoption
Compare / Diff
Source Trace
Validation
Stale / External Drift
Apply / Undo
Advanced Override
Raw DA Escape Hatch
```

### 24.2 P0 Production UI 기술 선택

P0 Production Authoring UI는 **C++ Slate**를 사용한다.

```text
Production Workspace
= CarFight_ReEditor C++ Slate

Core calculation / mutation
= Section 22 C++ service

Blueprint / EUW
= production Authoring Authority로 사용하지 않음
```

표준 Unreal asset picker, list/table, splitter, text/numeric control 같은 Editor widget은 적극 재사용한다.
그러나 Generic Raw `IDetailsView`를 Vehicle Authoring의 메인 화면으로 사용하지 않는다.
Raw Details를 다시 노출하면 현재 입력 문제를 그대로 재현하기 때문이다.

Profile처럼 typed property 수가 많은 보조 편집 화면에서는 Unreal Property Editor의 typed control을 내부 구현에 활용할 수 있지만, 사용자에게는 Domain별 그룹/설명/Source 의미가 정리된 화면을 제공한다.

### 24.3 Workspace 전체 구조

Vehicle Authoring은 별도 Nomad Tab / Workspace 하나로 제공한다.
기본 구조는 3-pane + bottom action bar다.

```text
┌─────────────────────────────────────────────────────────────────────┐
│ Vehicle Authoring Header / Global Actions                          │
├───────────────┬──────────────────────────────┬──────────────────────┤
│ Vehicle       │ Main Authoring View          │ Context Pane         │
│ Browser       │                              │                      │
│               │ Overview                     │ Changes              │
│ Search        │ Recipe & Profiles            │ Source Trace         │
│ Filters       │ Assets & Layout              │ Issues               │
│ Vehicle List  │ Driving Feel                 │ Sync                 │
│               │ Mounts & Defaults            │                      │
│               │ Compare                      │                      │
│               │ Validation                   │                      │
│               │ Advanced                     │                      │
├───────────────┴──────────────────────────────┴──────────────────────┤
│ Preview Freshness / Pending Changes / Apply Eligibility / Apply    │
└─────────────────────────────────────────────────────────────────────┘
```

Pane은 `SSplitter` 기반으로 사용자가 폭을 조절할 수 있게 한다.
고정 pixel 폭을 Architecture contract로 만들지 않는다.

핵심 UX 원칙:

1. 왼쪽은 **어떤 Vehicle을 작업하는지** 소유한다.
2. 가운데는 **무엇을 의도하는지 입력**한다.
3. 오른쪽은 **그 입력이 어떤 결과/Source/문제를 만드는지** 항상 보여준다.
4. 하단은 **Definition에 실제 적용 가능한지**와 Apply action만 소유한다.

117개 Raw leaf를 한 화면에 모두 노출하지 않는다.

### 24.4 Header 구조

Header에는 현재 selection과 전역 상태를 짧게 유지한다.

```text
[+ 새 Vehicle]
[기존 Definition 가져오기]
[Browser 새로고침]

Selected Vehicle Display Name
Definition Asset Path
Recipe Asset Path [있을 때]

Management State
Sync State
Validation State
Pending Diff Count
```

Header badge는 색상만으로 의미를 전달하지 않는다.
항상 icon + text를 같이 사용한다.

예:

```text
관리: Unmanaged
동기화: Stale 8
검증: Warning 2
변경: 12 Pending
```

### 24.5 Transient Workspace View State

UI가 상태를 쉽게 표현하기 위해 transient ViewModel state를 가질 수 있다.
이 상태는 Recipe/Definition의 새 SSOT가 아니다.

후보 view enum:

```text
ECFWorkspaceEntityKind
- None
- MeshCandidate
- VehicleDefinition

ECFWorkspaceManageView
- Unmanaged
- LegacyImported
- PartiallyManaged
- Managed

ECFWorkspaceSyncView
- NoBaseline
- InSync
- EffectiveStale
- ShadowChanged
- ExternalDrift
- PreviewOutOfDate

ECFWorkspaceValidView
- NotEvaluated
- Valid
- Warning
- Blocked
```

실제 source type / manage state authority는 Section 22의 Recipe/Resolver data다.
View enum은 화면 표시를 위한 derived state일 뿐 persistent metadata로 중복 저장하지 않는다.

### 24.6 관리 상태와 동기화 상태를 분리한다

한 개의 거대한 Status enum으로 모든 경우를 합치지 않는다.

예:

```text
Management = Partially Managed
Sync       = Effective Stale
Validation = Warning
```

가 동시에 가능하다.

#### Management

```text
Unmanaged
= Target UCFVehicleData는 있지만 Recipe가 없음

Legacy Imported
= Import Recipe가 있고 아직 일반 Authoring Source Adopt가 시작되지 않음

Partially Managed
= 일부 Adoption Group은 새 Source 소유, 일부는 Legacy Pin 소유

Managed
= 일반 Authoring 대상 field/group이 새 Source로 관리됨
  Legacy Serialized Passthrough가 남는 것은 Managed 판정을 막지 않음
```

#### Sync

```text
In Sync
= Current Definition과 AppliedState가 일치

Effective Stale
= effective Authoring Source가 Last Applied와 달라짐

Shadow Changed
= Pin/Override 아래 source만 바뀌어 current output은 동일

External Drift
= Raw Definition이 trusted baseline과 다름

Preview Out Of Date
= 화면에 보이는 Resolve Result가 현재 Recipe/Profile/Target과 맞지 않음
```

이 분리를 통해 `Managed = 정상`, `Unmanaged = 오류`처럼 단순화하지 않는다.

### 24.7 Vehicle Browser

왼쪽 Browser는 일상 진입점이다.
Browser index는 Section 21 계약대로 **재생성 가능한 cache/view**일 뿐 SSOT가 아니다.

기본 row kind:

```text
Managed Definition
Partially Managed Definition
Legacy Imported Definition
Unmanaged VehicleData
Mesh-only / Unregistered Candidate
```

기본 표시 column:

```text
Vehicle
Kind
Management
Sync
Validation
Override Count
```

좁은 화면에서는 상태 column을 하나의 compact status cell로 접을 수 있다.

추가 정보는 hover/detail에 표시한다.

```text
Recipe path
5 Profile binding summary
Base mass
Drive layout
Legacy Pin count
Stale field count
Validation issue count
```

### 24.8 Browser Search / Filter / Sort

P0 Browser 필수 기능:

```text
Search
- Vehicle / asset name
- Recipe name
- Profile name

Filter
- Managed
- Partially Managed
- Legacy Imported
- Unmanaged
- Stale
- External Drift
- Validation Error / Blocked
- Has Advanced Override
- Mesh-only Candidate

Sort
- Name
- Management
- Sync severity
- Validation severity
- Stale count
- Override count
```

Multi-selection은 P0-05에서 **Reference Compare용 read-only selection**까지만 허용한다.
Bulk Authoring / Multi Apply는 P0-07 Batch 계약 전에는 추가하지 않는다.

### 24.9 Browser Refresh 동작

`Browser 새로고침`은 다음만 수행한다.

```text
Asset Registry / Recipe relation 재조회
Derived status cache 재계산
Mesh-only candidate view 재구성
```

다음을 하지 않는다.

- Asset Import
- Recipe 자동 생성
- Definition 수정
- Stale Definition 자동 Apply
- Package Save

Asset Registry change event로 자연스럽게 갱신할 수 있지만 항상 수동 Refresh action도 제공한다.

### 24.10 Main Navigation 확정

가운데 Main Authoring View의 P0 navigation은 다음 8개로 고정한다.

```text
1. Overview
2. Recipe & Profiles
3. Assets & Layout
4. Driving Feel
5. Mounts & Defaults
6. Compare
7. Validation
8. Advanced
```

`Source Trace`, `Pending Diff`, `Stale`를 각각 별도 Main page로 만들지 않는다.
이 정보는 사용자가 어느 page에서 작업하든 오른쪽 Context Pane에서 즉시 확인해야 한다.

`Fitting`은 Authoring owner가 아니므로 독립 편집 page로 만들지 않는다.
필요한 read-only Fitting Preview Context는 Overview의 보조 card로 제공한다.

### 24.11 Overview Page

Overview는 Vehicle을 선택했을 때 첫 화면이다.

구성:

```text
Identity
Management Summary
Sync Summary
Validation Summary
Pending Change Summary
Profile Binding Summary
Needs Attention
Read-only Fitting Context
Recent Authoring Status
```

#### Needs Attention

가장 중요한 작업을 priority 순으로 보여준다.

```text
1. Error / Apply Blocker
2. External Drift
3. Missing Required Profile / Asset
4. Effective Stale
5. Unreviewed Measurement Proposal
6. Legacy Pinned group ready for adoption
7. Warning
8. Shadow Source Changed
```

각 item에는:

```text
문제 요약
관련 Domain / Field
Go To action
추천 행동
```

을 제공한다.

### 24.12 Overview의 Fitting Context

Fitting은 read-only context만 제공한다.

예:

```text
Base Vehicle Mass
Maximum Gross Mass
Optional Selected Fitting Snapshot
Fitted Mass [context가 있을 때]
```

표시에는 반드시:

```text
Preview Context Only
Authoring Source 아님
```

을 명시한다.

Authoring Workspace에서 Installed Set이나 Fitting total mass를 수정하지 않는다.

### 24.13 New Vehicle 진입

Browser/Header의 `+ 새 Vehicle`은 Raw `UCFVehicleData` Details를 열지 않고 **New Vehicle Setup** dialog를 연다.

Mesh-only Candidate에서 실행하면 `Create Vehicle From Mesh`로 같은 dialog를 열되 Chassis Mesh만 prefill한다.

Setup 항목:

```text
Vehicle Display / Asset Name
Definition Destination Folder
Recipe Asset Name
Recipe Destination Folder
Optional Chassis Mesh
Optional Wheel Mesh Selection
5 Profile Binding
```

P0-05는 새로운 balance 수치 default를 만들지 않는다.
Profile이나 required semantic input이 비어 있어도 Record 자체 생성은 허용할 수 있지만 생성 후:

```text
Management = Managed
Validation = Blocked / Setup Incomplete
Apply = Disabled
```

로 명확하게 표시한다.

여기서 `Managed`는 **Legacy Pin이 아닌 새 Authoring 구조가 이 Recipe의 정상 ownership 경로라는 뜻**이지 `Setup Complete`나 `Valid`를 뜻하지 않는다. 완성도는 Validation 축에서 별도로 표현한다.

### 24.14 New Vehicle Record 생성 계약

`Create Vehicle Records`는 Asset creation 전용 Editor action이다.
Resolver의 Apply와 분리한다.

표준 결과:

```text
New UCFVehicleData
+ New UCFVehicleRecipeData
+ Recipe.TargetVehicleData binding
```

생성 직후 Resolver 결과를 Definition에 자동 Apply하지 않는다.
Target은 현재 C++ Definition defaults를 가진 새 Definition이고, Recipe intent를 실제 Definition에 반영하려면 정상 Preview → Apply를 사용한다.

Creation UX 규칙:

1. Definition/Recipe package path를 먼저 validate한다.
2. 기존 Asset을 암묵적으로 덮어쓰지 않는다.
3. 두 record 중 하나가 생성되지 못한 경우 half-bound 상태로 정상 완료했다고 표시하지 않는다.
4. 생성된 package를 자동 Save하지 않는다.
5. 생성 후 Workspace에서 Definition/Recipe dirty state를 보여준다.

정확한 failure rollback 구현은 P0-09에서 고정한다.

### 24.15 New Vehicle 정상 작업 흐름

```text
Mesh Candidate 또는 + New Vehicle
→ Create Vehicle Records
→ Overview: Setup Incomplete 확인
→ Recipe & Profiles
→ Assets & Layout
→ Measurement Proposal 검토
→ Driving Feel / Mount / Defaults 입력
→ Resolve Preview
→ Validation
→ Pending Diff 확인
→ Explicit Apply
→ Definition / Recipe Package Dirty
→ 사용자가 일반 UE Save workflow로 저장
```

`Create`와 `Apply`를 한 버튼으로 합치지 않는다.

### 24.16 Unmanaged Existing Definition UX

Recipe가 없는 기존 `UCFVehicleData`를 선택하면 모든 Authoring field를 editable form으로 즉시 보여주지 않는다.

Overview 상단:

```text
상태: Unmanaged Existing Definition

이 VehicleData는 아직 Recipe/Source Tracking으로 관리되지 않습니다.
현재 값은 읽고 검사할 수 있지만 Authoring Intent로 자동 역산하지 않습니다.
```

허용 action:

```text
Validate Current Definition
Reference Compare
Open Raw DA
Import Into Authoring
```

금지 action:

```text
Apply Authoring
Adopt Group
Advanced Override 생성
Driving Feel을 raw 값에서 authoritative하게 생성
```

### 24.17 Existing Definition Import Dialog

`Import Into Authoring`은 별도 reviewed dialog를 사용한다.

단계:

```text
1. Target 확인
2. Recipe destination 선택
3. Current Definition snapshot / hash 확인
4. Legacy Pin summary 확인
5. Optional Profile Match Suggestion 확인
6. Import 결과 설명
7. Confirm
```

Confirm 문구가 분명해야 한다.

```text
이 작업은 Recipe를 생성하고 현재 Definition 값을 Legacy Pin으로 기록합니다.
Target VehicleData 값은 변경하지 않습니다.
```

Legacy serialized Mount field는 일반 pin summary와 분리해:

```text
Legacy Serialized Passthrough
```

로 접힌 diagnostics section에서만 표시한다.

### 24.18 Profile Match Suggestion UX

Import 중 Profile Match는 suggestion일 뿐이다.

표시 예:

```text
Handling Profile Candidate: Handling_Sedan
Match: <matched> / <eligible managed fields>
Status: Suggestion Only
```

금지:

- Match가 높다는 이유로 자동 Adopt.
- 현재 raw values를 Driving Feel로 역산하여 Profile match score에 authoritative하게 사용.
- `같은 값 = 이 Profile이 원래 Source`라고 표시.

사용자가 suggestion을 binding에 선택해도 기존 Legacy Pin은 그대로 남는다.
실제 ownership 변경은 별도 Adoption이다.

### 24.19 Import 완료 후 상태

Initial Import 직후 기본 상태:

```text
Management = Legacy Imported
Target Definition = Import 전과 동일
Legacy Pin = Current field values 보존
Recipe = 생성됨
Definition Apply = 없음
```

Overview에는 다음을 크게 표시한다.

```text
Legacy Values Protected
0개 또는 현재 Adopt된 Group 수
```

이 상태는 오류가 아니다.

### 24.20 Adoption Status Matrix

`Recipe & Profiles` page에는 Adoption Group matrix를 제공한다.

row 예:

```text
Visual Assets
Layout
Hardpoints
Mounts
Mass / Durability
Defense / FX
Drivetrain
Handling
Performance
Wheel Geometry
Wheel Visual
DriveState
Technical Handling
```

상태:

```text
Pinned
Mixed
Managed
Not Applicable
Blocked
```

`LegacyTechnical`은 normal adoption row로 노출하지 않고 Advanced diagnostics에서 read-only로 표시한다.

### 24.21 Group Adoption UX

Primary workflow는 field-by-field가 아니라 **group adoption**이다.

Pinned row action:

```text
[Preview Adoption]
```

Preview:

```text
Current Effective Value / Source
vs
Legacy Pin을 제외했을 때 New Source Value / Source

Changed Fields
Validation
Missing Source
```

`Preview Adoption`은 Recipe를 수정하지 않는다.

모든 prerequisite가 준비되면:

```text
[Adopt Handling]
```

같은 명시 action을 제공한다.

Adopt 결과:

```text
Legacy Pin 제거
AdoptedGroups 갱신
Recipe Revision 증가
Recipe Package Dirty
Target VehicleData 변경 없음
Definition Sync = 필요하면 Effective Stale
```

따라서 `Adopt`와 `Apply`는 항상 별도 행동이다.

### 24.22 Field-level Adoption

Field-level adoption은 일반 Main form에 먼저 노출하지 않는다.

접근 경로:

```text
Source Trace
→ Legacy Pin field 선택
→ Advanced Action
→ Preview This Field Adoption
```

사용 목적:

- group 전체를 아직 넘길 수 없는 exceptional field.
- migration/debug 상황.

Identity field나 Legacy Serialized Passthrough는 기존 계약에 따라 제한한다.

### 24.23 Recipe & Profiles Page 구조

Page section:

```text
Recipe Identity
Vehicle Archetype
5 Profile Bindings
Mass / Durability Intent
DriveState Mode
Default Defense / FX Intent
WheelVisual Semantic Policy
Adoption Status Matrix
```

Raw resolved Movement field는 여기서 편집하지 않는다.

### 24.24 Semantic Input Control Pattern

Profile default와 explicit value를 선택할 수 있는 field는 두 단계 control을 사용한다.

예:

```text
Base Mass
Source: [Use Base Profile ▼]
Value: <resolved profile value> kg [read-only resolved preview]

또는

Source: [Explicit Value ▼]
Value: [        ] kg
```

정확한 값은 current data/profile에서 읽으며 P0-05에서 신규 숫자를 만들지 않는다.

Defense / Destroyed FX는:

```text
Use Profile
Explicit Asset
Explicit None
```

을 명시적으로 선택한다.
Null object reference만 보고 `None 의도`를 추정하지 않는다.

### 24.25 5 Profile Binding UI

5개 Domain은 각각 독립 card로 표시한다.

```text
Vehicle Base
Drivetrain
Handling
Performance
DriveState
```

각 card:

```text
Bound Profile Asset
Authoring Revision
Payload Fingerprint short display
Current Vehicle에서 effective인지
Pinned 때문에 shadowed인지
Resolve issue count

[Choose Profile]
[Open Profile]
```

`DriveStateMode=ProjectDefault`일 때 DriveState Profile은 binding이 있어도 `Non-effective`로 표시할 수 있다.

### 24.26 Shared Profile 편집 경계

Vehicle page에서 Shared Profile payload를 inline으로 직접 고치지 않는다.
실수로 여러 차량을 동시에 stale시키는 것을 줄이기 위해 `Open Profile`을 명시적으로 눌러 별도 Profile Editor route로 들어간다.

Header/breadcrumb 예:

```text
Vehicle Authoring
> DA_TestSedan
> Handling Profile: Handling_Sedan
```

Profile Editor 상단에는:

```text
Shared Authoring Source
이 Profile 변경은 이를 사용하는 다른 Recipe도 Stale 상태로 만들 수 있습니다.
```

을 표시한다.

Browser cache로 usage를 계산할 수 있으면 `Used by N Vehicles`를 보조 정보로 보여줄 수 있지만 cache가 없다는 이유로 edit를 잘못 허용/차단하지 않는다.

### 24.27 Profile Editor 동작

Profile payload 수정은:

```text
Typed Domain Control
→ Profile UObject transaction edit
→ AuthoringRevision / Fingerprint 변화
→ Profile Package Dirty
→ Selected Vehicle Preview Invalidate
→ Resolver Refresh
```

이다.

Definition을 자동 Apply하지 않는다.

Profile copy가 필요하면 일반 Unreal duplicate 흐름 또는 `Duplicate Profile` convenience를 사용할 수 있다.
복제 후 current Recipe에 binding을 바꾸는 동작은 별도 명시 action으로 둔다.
Profile duplication과 rebind를 숨은 한 동작으로 처리하지 않는다.

### 24.28 Assets & Layout Page

Page section:

```text
Vehicle Mesh Assets
Wheel Mesh Assets
Wheel Socket Bindings
Derived Wheel Anchors
Wheel Measurement Proposals
Hardpoint Slots / Socket Bindings
Derived Hardpoint Transforms
Asset Validation
```

핵심 원칙:

```text
선택할 수 있는 값 = editable
Asset에서 계산된 값 = read-only derived preview
Target Definition 값 = diff/reference로 표시
```

### 24.29 Asset Picker UX

Chassis/Wheel은 typed asset picker를 사용한다.

각 row:

```text
Asset
Source = Recipe Asset Intent
Asset Status
Resolver-relevant fingerprint 상태
[Locate in Content Browser]
[Open Asset]
```

Asset을 바꾸는 순간 Target Definition은 수정하지 않는다.
Recipe가 수정되고 Preview가 invalidated된다.

### 24.30 Wheel Socket Binding UX

4개 Wheel socket row:

```text
FL | Socket Name | Found/Missing | Derived Location | Derived Rotation
FR | ...
RL | ...
RR | ...
```

Current fallback convention이 사용 중이면:

```text
Using Project Default: Wheel_Anchor_FL
```

같이 명확하게 표시한다.

4개 중 하나라도 missing이면 Layout section top에 Blocking issue를 표시한다.

```text
Layout Resolve Blocked
4 Wheel Anchor socket이 모두 필요합니다.
```

`Capture Layout` 버튼으로 Target에 직접 쓰는 기능은 새 Workspace에 만들지 않는다.

### 24.31 Derived Wheel Anchor 표시

Derived anchor transform은 normal 상태에서 read-only다.

row:

```text
Field
Socket
Derived Value
Effective Source
Current Target Value
Change Status
```

Manual correction이 필요한 경우:

```text
Advanced Override...
```

로 이동한다.

Normal Layout page에서 Transform raw numeric edit box를 기본 제공하지 않는다.

### 24.32 Wheel Measurement Proposal UX

Measurement-assisted field는 dedicated table을 사용한다.

```text
Field
Current Effective Value
Measured Candidate
Difference
Measure Rule
Asset Fingerprint Status
Decision
```

상태:

```text
Not Reviewed
Accepted
Asset Changed / Stale Measurement
Using Compatibility Default
Advanced Override
```

Action:

```text
[Use Measured Value]
[Confirm Compatibility Default]
[Set Manual Override...]
```

`Use Measured Value`:

```text
Recipe AssetAdoption 갱신
해당 Asset fingerprint 기록
Recipe transaction
Preview refresh
Target mutation 없음
```

`Confirm Compatibility Default`는 새 Source Type을 만들지 않는다.
`WheelGeometry`의 explicit reviewed/adoption metadata로 기록하고 effective value source는 계속 Project Compatibility Default다.
정확한 storage는 Section 22의 existing adoption metadata 범위 안에서 구현한다.

`Set Manual Override...`는 Advanced Override dialog로 이동한다.

### 24.33 Stale Measurement UX

이전에 accepted한 Wheel measurement의 fingerprint가 바뀌면:

```text
Measured Value Stale
```

을 표시하고 자동으로 새 candidate를 Target에 적용하지 않는다.

사용자 선택:

```text
Review New Measurement
Keep Current Authored Decision
Switch to Manual Override
```

새 measurement를 다시 accept하면 Recipe adoption metadata가 갱신되고 정상 Stale/Preview/Apply 흐름을 따른다.

### 24.34 Hardpoint Slot UX

Hardpoint table:

```text
LocationSlotId
LocationCategory
SocketName
Socket Status
Derived Local Location
Derived Local Rotation
Mount Reference Count
```

`LocalLocation/Rotation`은 Socket-derived이면 read-only다.
Manual field는 Advanced Override로 이동한다.

Stable ID를 normal text cell에서 즉시 편집하지 않는다.

Action:

```text
[Add Hardpoint]
[Rename Slot ID...]
[Remove Hardpoint...]
```

### 24.35 Hardpoint stable ID 보호

`Rename Slot ID...`는 semantic operation이다.
Dialog에서:

```text
Old ID
New ID
Dependent Mount LocationSlotRef 목록
Legacy Pin / Override dependency
Result Preview
```

를 보여준다.

관련 참조를 안전하게 함께 갱신할 수 없는 경우 rename을 Block한다.

`Remove Hardpoint`도 dependent Mount가 있으면 default cascade delete를 하지 않는다.
먼저 Mount를 reassign/remove하도록 안내한다.

### 24.36 Driving Feel Page

화면의 중심은 4개 semantic control이다.

```text
가속감
조향 민첩성
접지감
서스펜션 단단함
```

각 control:

```text
0~100% 표시
Slider
Current Recipe Intent
Effective Raw Output 요약
Owning Profile
Source Status
```

Raw output 전체를 slider 아래에 모두 펼치지 않는다.
필요한 자세한 raw field는 오른쪽 Changes / Source Trace에서 본다.

### 24.37 Driving Feel interaction transaction

Slider drag를 수십 개 transaction으로 만들지 않는다.

UX contract:

```text
Mouse/keyboard interaction 시작
→ 하나의 logical Recipe edit transaction
→ value 변경
→ interaction commit
→ transaction 종료
→ Preview refresh
```

실제 Slate event 연결 방식은 P0-10에서 구현한다.

Preview를 위해 Slider tick마다 Target Definition을 수정하는 것은 금지한다.

### 24.38 Legacy Pin과 Driving Feel

Legacy Imported 차량의 Handling/Performance field가 아직 pinned여도 future Recipe Feel Intent를 미리 입력할 수 있다.

단 각 slider에 상태를 표시한다.

예:

```text
가속감 <현재 입력>%
Status: Shadowed by Legacy Pin
Performance group을 Adopt하기 전에는 Target 결과에 영향을 주지 않습니다.
```

즉 Control을 무조건 disable하지 않는다.
사용자가 원하는 future intent를 먼저 설계한 뒤 Adoption Preview로 결과를 볼 수 있게 한다.

### 24.39 Driving Feel Preset UX

Current semantic shortcut은 유지한다.

```text
[Sedan]
[SUV]
[Sports]
[Heavy]
```

버튼에는:

```text
4축 값 일괄 입력
Target Definition은 아직 변경되지 않음
```

이라는 tooltip/보조 설명을 제공한다.

P0-04의 exact 4축 값은 migration parity reference로 유지한다.
Preset을 Profile 또는 Source Type으로 표시하지 않는다.

### 24.40 Existing Definition의 Feel 표시

Unmanaged Existing Definition이나 아직 Feel Intent가 작성되지 않은 Import Recipe에는:

```text
Driving Feel: Not Authored
```

를 기본 표시한다.

Legacy estimate를 제공할 경우:

```text
Legacy Estimate: <estimate>%
Source: Unknown / Derived for Reference Only
[Do not adopt automatically]
```

처럼 authority가 아님을 명확히 한다.

### 24.41 Mounts & Defaults Page

Page section:

```text
Mount Profiles
Default Equipment Preset
Default Defense
Destroyed FX
Destroyed FX Socket
Optional bExposedModule metadata
```

Hardpoint geometry는 Assets & Layout에서 편집하고, Mount는 그 Hardpoint `LocationSlotRef`를 선택한다.

Mount row:

```text
MountProfileId
LocationSlotRef
MountType
SizeLimit
DefaultEquipmentPresetData
bExposedModule
Status
```

legacy serialized turret field는 normal row에 표시하지 않는다.

### 24.42 Mount stable ID 보호

`MountProfileId`도 normal inline rename을 하지 않는다.

```text
[Rename Mount ID...]
```

으로 dependency preview를 거친다.
Legacy Pin / Advanced trace가 old stable path를 사용 중이면 안전한 migration 가능 여부를 검사한다.

무조건 string만 바꿔 Source Tracking을 끊는 행동은 금지한다.

### 24.43 Default Defense / FX UX

각 default ref는 Source mode를 함께 보여준다.

예:

```text
Default Defense
Mode: Use Base Profile / Explicit Asset / Explicit None
Value: ...
Effective Source: Vehicle Base Profile
```

Destroyed FX Socket은 Project Default `FX_Destroyed`를 쓸 때도:

```text
Project Compatibility Default
```

badge를 보여준다.

Asset socket을 임의 자동 선택하는 버튼은 P0에서 제공하지 않는다.

### 24.44 Compare Page — 2 Mode

Compare Page는 mode를 명확히 분리한다.

```text
[Pending Authoring Changes]
[Reference Vehicle Compare]
```

같은 table component를 사용할 수 있지만 의미를 섞지 않는다.

### 24.45 Pending Authoring Changes Mode

Authority:

```text
FCFVehicleResolveResult.FieldDiff
```

기본 filter:

```text
Changed Only = On
```

column:

```text
Operation
Domain / Group
Stable Field Path
Before
After
Effective Source
Issue
```

Array operation은:

```text
Add
Remove
Move
Set Leaf
```

를 명확히 표시한다.

이 table이 Apply 대상의 실제 diff다.

### 24.46 Reference Vehicle Compare Mode

A/B selection:

```text
A = Current Vehicle / Resolved Preview 선택
B = Browser에서 고른 Reference Vehicle
```

column:

```text
Stable Field Path
A Value
B Value
Same / Different
A Source [resolved snapshot이면]
B Source [resolved snapshot이면]
```

단순 차이에 Warning/Error severity를 붙이지 않는다.

Action:

```text
Changed Only
Domain Filter
Source Filter
Swap A/B
Locate Field
```

`Copy From Reference` 같은 direct mutation action은 P0-05에서 만들지 않는다.

### 24.47 오른쪽 Context Pane

Context Pane은 main page와 독립적으로 현재 선택 field/result를 설명한다.

고정 tab:

```text
Changes
Source Trace
Issues
Sync
```

main center table/field에서 항목을 선택하면 같은 Stable Field Path를 context selection으로 사용한다.

### 24.48 Context — Changes

선택 field의:

```text
Current Target Value
Resolved Value
Diff Operation
Pending / No Change
```

을 표시한다.

field를 선택하지 않았으면 전체 summary:

```text
Set Leaf N
Array Add N
Array Remove N
Array Move N
```

을 보여준다.

### 24.49 Context — Source Trace

Source stack은 낮은 precedence → 높은 precedence 순으로 보여준다.

예:

```text
Project Compatibility Default
Owning Profile Domain
Rule / Asset Derived
Recipe Explicit
Legacy Pin
Advanced Override
```

각 layer:

```text
Source Type
Source ID / Asset
Source Revision
Fingerprint short form
Resolved Value
Effective / Shadowed
```

Effective layer는 반드시 text badge로 표시한다.

click action:

```text
Profile → Open Profile
Asset → Locate/Open Asset
Legacy Pin → Adoption Preview
Advanced Override → Open Override
```

### 24.50 Source Trace의 편집 제한

Source Trace는 raw editor가 아니다.

허용 action:

```text
Preview Adoption
Remove Advanced Override
Open Source Asset
Go To Authoring Control
```

금지:

- CanonicalValueText 직접 편집.
- Source Type 임의 변경.
- `Effective` checkbox 같은 manual precedence control.

Source winner는 Resolver가 결정한다.

### 24.51 Context — Issues

현재 field 또는 전체 Vehicle의 issue를 다음 layer로 표시한다.

```text
Recipe
Resolver
Definition
Sync / Drift
```

각 row:

```text
Layer
Severity
Field Path
Message
Recommended Action
Apply Blocking
[Go To]
```

### 24.52 Validation Severity UX

표시 의미:

```text
Error
= 입력/계약이 잘못되어 resolve/apply가 성립하지 않음

Blocked
= dependency/prerequisite가 없어 현재 단계 진행 불가

Warning
= 적용은 가능하지만 검토가 필요

Info
= 설명/참고
```

Apply rule:

```text
Error > 0   → Disabled
Blocked > 0 → Disabled
Warning > 0 → Enabled 가능, warning count 노출
Info         → Apply 영향 없음
```

`USER Driving Feel 미확인` 같은 사항을 Error로 만들어 Technical Apply 자체를 막지 않는다.
USER Acceptance 상태는 별도다.

### 24.53 Validation Page

Validation Page는 Context Issues의 전체-screen 작업판이다.

filter:

```text
Layer
Severity
Apply Blocking Only
Domain
Field Path
```

row click:

```text
→ owning Main Page로 이동
→ 관련 control highlight
→ Context Source/Issue 동기화
```

오류를 toast 한 번 띄우고 사라지게 하지 않는다.
Persistent issue row가 Authority presentation이다.

### 24.54 Inline Validation

사용자가 입력 중인 field 바로 아래에도 가장 중요한 issue 1개를 inline 표시할 수 있다.

예:

```text
Handling Profile
[None]
! 이 Group을 Adopt하려면 Handling Profile이 필요합니다.
```

전체 진단은 Validation Page/Issues에 남는다.
Inline message만 보고 report가 사라지지 않는다.

### 24.55 Context — Sync

Sync tab은 다음을 분리해 보여준다.

```text
Effective Stale
Shadow Source Changed
External Drift
Preview Freshness
Applied Definition Hash
Current Definition Hash
Resolver Contract Revision
```

Hash는 full raw string을 기본 화면에 길게 노출하지 않고 short form + Copy diagnostics action을 사용할 수 있다.

### 24.56 Effective Stale UX

표시:

```text
Stale: 8 fields
Reason: Handling Profile Changed
Target Definition은 마지막 Apply 상태를 유지 중입니다.
```

Action:

```text
[Refresh Preview]
[Show Stale Fields]
```

Refresh는 Target을 수정하지 않는다.
Fresh Resolve 이후 일반 Apply workflow로 넘어간다.

### 24.57 Shadow Source Changed UX

예:

```text
Shadow Source Changed: 4 fields
Effective value는 Legacy Pin / Advanced Override 때문에 변하지 않았습니다.
```

Apply를 요구하지 않는다.

Action:

```text
[Review Shadow Sources]
```

이 화면에서 Pin/Override를 즉시 자동 해제하지 않는다.

### 24.58 External Drift Global Banner

External Drift는 눈에 잘 띄는 persistent banner를 Header/Overview에 표시한다.

```text
External Drift Detected
Target VehicleData가 마지막으로 적용/가져온 baseline과 다릅니다.
정상 Apply 전에 Drift를 먼저 검토하세요.
```

Primary action:

```text
[Review Drift]
```

Secondary:

```text
[Open Raw DA]
```

일반 `Apply` 버튼은 unresolved External Drift 상태에서 disabled다.

### 24.59 External Drift Review

Drift review table은 3개 값을 함께 보여준다.

```text
Field
Last Trusted / Applied
Current Raw Definition
Current Authoring Resolve
```

사용자 결정 후보:

```text
Keep Authoring
= Raw 변경을 Source로 가져오지 않음.
  다음 explicit Re-Apply에서 Authoring result가 Target을 덮을 수 있음.

Preserve Raw As Legacy Pin
= Rebase를 통해 Raw 값을 Authoring baseline으로 보존.

Promote Raw To Advanced Override
= Registry에서 허용된 field만 가능.
  Reason 입력 필요.

Cancel
= 아무 것도 수정하지 않음.
```

### 24.60 External Drift 결정은 즉시 Target write가 아니다

Drift Review의 `Keep Authoring`, `Preserve Raw`, `Advanced Override`는 Authoring review/rebase decision이다.
Target을 다시 쓰는 것은 최종 Apply에서만 수행한다.

단순히 Drift table을 열어봤다는 사실만으로 일반 Apply를 활성화하지 않는다.

`Keep Authoring`을 명시적으로 선택한 경우에만 transient UI review decision을 만들 수 있다.
이 decision은 최소 다음 current state에 묶인다.

```text
Current Target Definition Hash
Current Recipe Fingerprint
Current Source Signature
Current Resolver Contract Revision
```

이 중 하나라도 바뀌면 review decision은 즉시 무효다.
이 transient decision은 새 Source/Persistent Metadata가 아니며 ApplyService contract를 대체하지 않는다.

이 상태에서만 Bottom Bar가:

```text
[Apply Reviewed Authoring Result]
```

를 제공할 수 있다.
실제 write는 동일 `FCFVehicleApplyService`와 current Target hash precondition을 사용한다.

`Re-Apply Authoring` shortcut이 있더라도:

```text
Review Drift
→ Keep Authoring 또는 Rebase/Override 결정
→ Fresh Resolve
→ Drift/Pending Diff 확인
→ Validation
→ Explicit Apply
```

를 건너뛰지 않는다.

### 24.61 Preview Freshness UX

Bottom Action Bar에 항상 다음 중 하나를 표시한다.

```text
Preview: Fresh
Preview: Needs Refresh
Preview: Resolving
Preview: Blocked
Preview: Out Of Date
```

Recipe/Profile/Asset input이 변경되면 current result를 숨기지 않고:

```text
Out Of Date
```

watermark/badge를 붙여 사용자가 이전 결과라는 것을 알 수 있게 한다.

오래된 preview에서는 Apply를 비활성화한다.

### 24.62 Auto Preview Refresh 정책

Pure Resolve는 안전하므로 사용자 click을 매번 요구하지 않는다.

기본 UX:

```text
Discrete control commit
→ Preview invalidated
→ auto refresh request
```

다만:

- Slider drag 중 매 tick마다 expensive full resolve를 강제하지 않는다.
- Slider interaction commit 시 full resolve한다.
- Asset load가 필요하거나 resolver가 busy이면 `Needs Refresh`를 먼저 표시한다.
- 항상 manual `Refresh Preview` 버튼을 제공한다.

Auto Preview는 Target Definition mutation을 의미하지 않는다.

### 24.63 Recipe / Profile edit와 Definition Apply를 구분

이 UX에서 매우 중요한 차이:

```text
Recipe/Profile Edit
= Authoring Intent asset 자체는 즉시 transaction edit됨
= Recipe/Profile Package Dirty 가능
= Target Definition은 그대로

Apply
= Fresh Resolve Result를 Target UCFVehicleData에 동기화
= Target + Recipe AppliedState Package Dirty
```

따라서 Header에 package 상태를 분리해 표시한다.

예:

```text
Recipe: Modified / Unsaved
Profile: Modified / Unsaved
Definition: In Sync

또는

Recipe: Modified / Unsaved
Definition: 12 Pending Changes
```

`Apply`를 누르면 Recipe 자체가 disk save된다고 오해하게 만들지 않는다.

### 24.64 자동 Save 금지 UX

Workspace는 Apply 뒤:

```text
Applied successfully.
VehicleData와 Recipe package가 Modified 상태입니다.
```

라고 표시할 수 있다.

자동 `Save All`을 호출하지 않는다.
일반 Unreal Editor save workflow를 사용한다.

P0에서 별도 강제-save button을 핵심 Authoring workflow에 넣지 않는다.

### 24.65 Bottom Action Bar

항상 보이는 상태:

```text
Preview Freshness
Pending Diff Count
Error / Blocked / Warning Count
External Drift 여부
Target 여부
```

Action:

```text
[Refresh Preview]
[Apply N Changes]
```

조건부:

```text
[Review Drift]
[Show Blocking Issues]
```

### 24.66 Apply 활성 조건

`Apply N Changes`는 다음이 모두 만족될 때만 활성화한다.

```text
Recipe 존재
Target VehicleData 존재
Preview Fresh
Resolved Diff 존재
Recipe Validation Error=0
Resolver Validation Error/Blocked=0
Definition Validation Error/Blocked=0
External Drift 없음
  또는 current-hash-bound Keep Authoring review decision이 유효한 Reviewed Apply mode
ApplyRequest precondition 생성 가능
```

Diff가 0이면:

```text
No Changes To Apply
```

로 disabled한다.

### 24.67 Warning이 있는 Apply

Warning만 있고 Error/Blocked가 없으면 Apply를 허용한다.

Button:

```text
Apply 8 Changes · 2 Warnings
```

click 시 compact warning review를 먼저 보여줄 수 있다.

```text
Target
Change count
Warnings
Array Add/Remove count
No Auto Save
```

매번 큰 modal을 띄워 일상 작업을 방해하지 않는다.

### 24.68 Destructive-looking Diff confirmation

다음은 clean Apply라도 한 번 더 명확히 보인다.

```text
Array Element Remove
Stable ID Rename 결과의 Remove/Add
Reference asset ExplicitNone로 변경
```

Apply diff table과 action bar에서 distinct marker를 사용한다.

데이터 삭제 위험이 있는 array remove가 hidden diff로 처리되지 않게 한다.

### 24.69 Apply Success UX

Success result:

```text
Applied N changes
Validation: PASS / Warning count
Definition Hash updated
Recipe AppliedState updated
Packages: Modified / Unsaved
```

자동으로 Save하거나 USER feel PASS를 표시하지 않는다.

직전 Editor transaction이 이 Workspace Apply임을 확실히 추적할 수 있을 때만 optional:

```text
[Undo Last Apply]
```

을 보여줄 수 있다.
그 외에는 Unreal 표준 Undo (`Ctrl+Z`)를 안내한다.

### 24.70 Apply Failure UX

Failure는 toast만 띄우지 않는다.

persistent result:

```text
Apply Failed
Stage
Reason
Rollback Result
Target Current Hash
Recommended Action
```

예:

```text
Preview Out Of Date
→ Refresh Preview

Post Apply Validation Failed
→ Rollback complete
→ Open Validation
```

rollback 실패 같은 비정상 상태는 Error로 높이고 Target을 재읽어 External Drift/unknown state를 다시 판정한다.

### 24.71 Undo UX

Undo 대상은 서로 다르다.

```text
Recipe edit
Profile edit
Adoption
Override add/remove
Definition Apply
```

각 logical action은 하나의 `FScopedTransaction` 단위다.

Undo 후:

```text
Recipe/Profile/Definition snapshot refresh
Preview invalidation
Stale/Drift re-evaluation
```

을 수행한다.

Current Quick Tune의 별도 memory Revert button은 새 Workspace에 만들지 않는다.

### 24.72 Advanced Page 구조

Advanced는 정상 작업에서 자주 들어갈 필요가 없는 기능을 모은다.

```text
Advanced Leaf Overrides
Legacy Pin Inspector
Legacy Serialized Passthrough
Resolver Diagnostics
Field Source Table
Raw VehicleData Escape Hatch
```

Raw 117 field editable mirror를 만들지 않는다.

### 24.73 Advanced Override 생성

Primary action:

```text
[Add Advanced Override]
```

Dialog:

```text
Searchable allowed field
Stable Field Path
Base Source
Base Resolved Value
Typed Override Value
Reason
Expected Result
```

Field Registry에서 `bAdvancedOverrideAllowed=false`인 field는 selection 목록에 나타나지 않는다.

UI는 `CanonicalValueText` raw string을 직접 사용자에게 편집시키지 않고 property type에 맞는 typed control을 사용한다.

### 24.74 Override Reason UX

P0 UI에서는 유지보수를 위해 `Reason` 입력을 **필수**로 한다.

예:

```text
Wheel mesh bounds convention does not match physical tire radius.
```

AI Contract의 최소 requirement는 P0-06에서 별도 설계하지만 UI에서 이유 없는 새 override를 만들지 않는다.

### 24.75 Override 표시

Override가 있는 normal field control에는 작게:

```text
Advanced Override Active
```

을 표시한다.

right Source Trace:

```text
Base: Handling Profile = ...
Override: ... [Effective]
Reason: ...
```

Action:

```text
[Edit Override]
[Remove Override]
```

Remove하면 즉시 underlying current source result가 preview된다.
Target은 Apply 전까지 바뀌지 않는다.

### 24.76 Legacy Pin Inspector

Advanced Page에서 current pin을 filter/search할 수 있다.

```text
Field Path
Pinned Value
Shadow Source
Shadow Value
Adoption Group
```

Normal workflow에는 group adoption을 권장한다.
Pin table에서 mass unpin 같은 일괄 raw checkbox UI를 만들지 않는다.

### 24.77 Raw DA Escape Hatch

`Open Raw VehicleData`는 Advanced Page에서 유지한다.

Unmanaged:

```text
[Open Raw VehicleData]
```

Managed/Partially Managed:

```text
! Advanced Escape Hatch
Raw edit는 Recipe Source Tracking을 우회하며 External Drift를 만들 수 있습니다.
[Cancel]
[Open Raw VehicleData Anyway]
```

Raw editor를 열었다는 사실 자체는 Drift가 아니다.
실제 값 변경이 trusted baseline과 다를 때 Drift다.

### 24.78 Raw Editor에서 돌아왔을 때

Workspace는 가능한 Editor asset change notification 또는 focus refresh에서 Current Target Snapshot을 다시 검사한다.

```text
No Change
→ 그대로

Changed
→ External Drift
```

항상 manual:

```text
[Refresh Current Definition]
```

도 제공한다.

Raw edit를 자동 Advanced Override로 흡수하지 않는다.

### 24.79 오류/경고 표현 원칙

P0 UI는 오류 처리에서 다음을 지킨다.

1. **문제 위치를 보여준다.**
2. **왜 막혔는지 말한다.**
3. **무엇을 해야 하는지 action을 제공한다.**
4. **Target이 수정됐는지 아닌지 명확히 한다.**
5. **Toast만으로 중요한 상태를 끝내지 않는다.**

Bad:

```text
Resolve Failed
```

Good:

```text
Layout Resolve Blocked
VehicleLayoutConfig.BodyWheelSocketFR에 대응하는 Chassis socket을 찾지 못했습니다.
Target VehicleData는 변경되지 않았습니다.
[Assets & Layout으로 이동]
```

### 24.80 Modal 사용 제한

Modal/dialog는 다음처럼 ownership 또는 destructive meaning이 큰 행동에만 사용한다.

```text
New Vehicle record creation
Existing Definition Initial Import
External Drift Rebase commit
Stable ID rename / destructive remove
Managed Vehicle Raw DA escape hatch warning
```

일반 field edit, Preview refresh, Warning list 확인마다 modal을 띄우지 않는다.

### 24.81 Field Label / Raw Path 표시

Normal UI:

```text
한국어 사용자 의미 이름
```

을 먼저 보여준다.

예:

```text
기준 차량 질량
전륜 최대 조향각
앞왼쪽 바퀴 소켓
```

Stable Field Path는:

- Diff
- Source Trace
- Validation detail
- Advanced

에서 secondary technical identity로 보여준다.

즉 초보 작업자는 Raw path를 외우지 않아도 되고, debugging할 때는 정확한 field identity를 확인할 수 있다.

### 24.82 Navigation / Go To Field

Issue/Diff/Source Trace row에는 가능한 경우 `Go To` action을 제공한다.

Registry descriptor가 normal Authoring control을 알고 있으면:

```text
VehicleMovementConfig.EngineMaxTorque
→ Driving Feel Page
→ Acceleration section
```

처럼 이동한다.

Advanced-only field면:

```text
→ Advanced Override / Source Inspector
```

로 이동한다.

### 24.83 Selection 전환 안전성

Recipe/Profile form은 UObject에 transaction edit를 즉시 반영하므로 일반적인 웹 form처럼 별도 `Save Form` buffer를 만들지 않는다.

다만 focus 중인 text/numeric field에 commit되지 않은 edit가 있으면 Selection 전환 전에:

```text
Commit valid edit
또는
Cancel invalid edit
```

를 수행한다.

Selection을 바꾼다고 Recipe/Profile package를 자동 Save하지 않는다.

### 24.84 Read-only / Editable 시각 구분

같은 숫자라도 역할을 구분한다.

```text
Editable Semantic Input
→ normal editable control

Derived Value
→ read-only value + Source badge

Current Target Value
→ comparison style

Legacy Pin
→ locked/pinned badge

Advanced Override
→ warning/advanced badge
```

색상만으로 구분하지 않는다.

### 24.85 ViewModel / UI state 경계

P0-09 구현 시 `FCFVehicleAuthoringVM` 같은 transient ViewModel을 사용할 수 있다.

책임:

```text
Current Browser Selection
Current Recipe / Target refs
Current Resolve Result
Reference Compare selection
UI filter state
Current field selection
Preview freshness
Issue presentation rows
```

비책임:

```text
Authoring source truth
Definition resolved truth
Field precedence
Persistent Legacy Pin
Persistent Override
```

ViewModel을 저장하지 않는다.

### 24.86 예상 Slate 파일 경계

실제 파일 생성은 P0-08/P0-09이며 P0-05에서는 책임만 고정한다.

```text
CarFight_ReEditor/
├─ Public/DataAuthoring/
│  ├─ CFVehicleAuthoringTab.h
│  └─ CFVehicleAuthoringVM.h
│
└─ Private/DataAuthoring/
   ├─ CFVehicleAuthoringTab.cpp
   ├─ CFVehicleAuthoringVM.cpp
   ├─ CFVehicleBrowserView.h/.cpp
   ├─ CFVehicleEditorView.h/.cpp
   ├─ CFVehicleContextView.h/.cpp
   ├─ CFVehicleDiffView.h/.cpp
   └─ CFVehicleIssueView.h/.cpp
```

처음부터 page 하나마다 별도 class/file을 과도하게 만들 필요는 없다.
`CFVehicleEditorView` 내부 section이 커져 독립 responsibility가 확인될 때 분리한다.
1인 개발 기준으로 조기 class explosion을 피한다.

### 24.87 UI Event Pipeline

일반 Recipe edit:

```text
User Control
→ Recipe Transaction Mutation
→ Mark Recipe Package Dirty
→ Invalidate Preview
→ Snapshot Builder
→ Pure Resolver
→ ViewModel ResolveResult 교체
→ Context / Diff / Issues refresh
```

Definition Apply:

```text
Apply Button
→ Current Fresh ResolveResult
→ ApplyRequest
→ FCFVehicleApplyService
→ Result
→ Target/Recipe snapshot refresh
→ UI refresh
```

Slate가 별도 raw calculation/mutation을 수행하지 않는다.

### 24.88 Reference Compare Event Pipeline

```text
Select Current Vehicle
→ Select Reference Vehicle
→ Snapshot both
→ Field Registry Compare
→ Read-only Compare Result
```

Compare 결과가 Recipe를 바꾸거나 `ApplyRequest`를 생성하지 않는다.

### 24.89 New Vehicle 일상 흐름 예시

```text
1. Browser에서 CityCar Mesh Candidate 선택
2. Create Vehicle From Mesh
3. Definition/Recipe 이름·경로 확인
4. Records 생성
5. Overview에서 missing Profile 확인
6. 5 Profile binding 설정
7. Assets & Layout에서 socket 상태 확인
8. Wheel measurement proposal 검토
9. Driving Feel 4축 입력
10. Hardpoint / Mount / Default 설정
11. Validation blocker 해결
12. Pending Diff 검토
13. Apply
14. UE 표준 Save
```

이 흐름에서 Raw `UCFVehicleData` Details를 열 필요가 없어야 한다.

### 24.90 Existing Vehicle Import 흐름 예시

```text
1. Browser에서 Unmanaged DA_TestSedan 선택
2. Current Definition Validate
3. Import Into Authoring
4. Legacy Pin summary 확인
5. Recipe 생성
6. Profile 후보/Binding 선택
7. Handling Adoption Preview
8. Desired Driving Feel 설정
9. Handling Adopt
10. Performance Adoption Preview / Adopt
11. Effective Stale 확인
12. Diff / Validation
13. Apply
14. 다른 group을 필요에 따라 점진적으로 Adopt
```

전체 117 field를 한 번에 새 시스템으로 강제 전환하지 않는다.

### 24.91 Managed Vehicle 수정 흐름 예시

```text
1. Browser에서 Managed Vehicle 선택
2. In Sync 확인
3. Recipe 또는 Shared Profile 수정
4. Preview Invalidate → Resolve
5. Stale/Pending Changes 확인
6. Source Trace로 원인 확인
7. Validation
8. Apply
9. UE 표준 Save
```

Profile 변경이 여러 Vehicle에 영향을 줄 수 있지만 선택 Vehicle만 자동 Apply하지 않는다.
다른 dependent Recipe는 Browser에서 Stale로 표시될 수 있다.

### 24.92 External Drift 복구 흐름 예시

```text
1. Managed Vehicle 선택
2. External Drift banner
3. Review Drift
4. Last Applied / Current Raw / Current Authoring 비교
5. field/group별 Keep Authoring / Rebase / Advanced Override 결정
6. Recipe transaction 반영
7. Fresh Resolve
8. Diff / Validation
9. Explicit Apply
```

Drift를 발견했다고 Raw 값을 조용히 Recipe에 흡수하지 않는다.

### 24.93 Profile 변경 영향 UX

Shared Profile 수정 완료 후 현재 Vehicle에는:

```text
Profile Changed
Preview refreshed
Definition has N pending changes
```

을 보여준다.

Browser cache가 다른 dependent Recipe를 발견하면:

```text
Affected Vehicles: N
```

을 보여줄 수 있다.

이 정보는 navigation aid이며 dependency cache가 SSOT가 아니다.

### 24.94 Data Browser와 Mesh-only Candidate

Mesh-only Candidate는 Definition이 아니므로:

```text
Validation
Apply
Import Existing
```

을 제공하지 않는다.

Primary action:

```text
Create Vehicle From Mesh
```

Candidate row는 chassis asset 사실만 의미하며 차급/Profile/물리값을 자동 추론하지 않는다.

### 24.95 No hidden mutation 원칙

다음 action은 Target Definition을 수정하지 않는다.

```text
Vehicle selection
Profile selection
Recipe edit
Driving Feel slider
Preset click
Asset picker change
Layout socket binding change
Measurement proposal review
Adoption Preview
Reference Compare
Validation
Source Trace
Refresh Preview
```

Target mutation action은 P0 정상 workflow에서 **Apply 하나**다.

New Vehicle record creation은 새 asset 생성 action이며 기존 Target mutation과는 별개다.

### 24.96 No hidden intent 원칙

UI는 다음을 자동으로 Authoring Intent로 만들지 않는다.

```text
Raw VehicleData → Driving Feel
Raw value → Profile ownership
Reference Vehicle → Target Source
Mesh bounds → accepted Wheel physics value
External Raw edit → Advanced Override
```

Suggestion/Estimate는 항상 label로 구분한다.

### 24.97 P0-09 MVP UI 최소 범위와 P0-10 확장

P0-04 Migration 단계와 맞춘다.

#### P0-09 MVP

```text
Vehicle Browser / Target selection
Management status
Initial Import
Overview
Recipe basic view
Resolve Preview
Pending Diff
Source Trace
Definition Validation
Apply / Undo
Raw DA Open
```

#### P0-10 parity expansion

```text
Reference Compare
Full Assets & Layout
Measurement Adoption
Driving Feel 4축 + presets
Mount/Default migration UI
Unified Validation navigation
Managed-target Legacy Wizard guard
```

P0-05의 전체 UX가 모두 P0-09 첫 구현에 들어가야 하는 것은 아니다.

### 24.98 P0-11 Technical Validation에 넘길 UI 계약

Technical validation은 최소 다음 UI behavior를 검증해야 한다.

```text
Unmanaged selection does not mutate
Initial Import does not mutate Target
Recipe edit invalidates preview
Old preview cannot Apply
Error/Blocked disables Apply
Warning-only can Apply
External Drift disables normal Apply
Adopt mutates Recipe only
Measurement accept mutates Recipe only
Apply mutates exact Diff only
Undo restores Recipe/Definition/AppliedState coherently
Raw DA edit becomes External Drift
Reference Compare never creates Apply mutation
Managed Legacy Wizard write is blocked after P0-10
```

### 24.99 USER Acceptance에 남기는 항목

P0-05 설계 완료는 다음을 PASS로 추정하지 않는다.

```text
화면이 실제로 편한가
Raw Field 탐색 시간이 충분히 줄었는가
4축 Driving Feel이 이해하기 쉬운가
Source Trace가 과도하게 복잡하지 않은가
Import/Adoption이 귀찮지 않은가
Diff/Validation 정보량이 적절한가
실제 주행감이 좋은가
```

이는 P0-12 USER Authoring Acceptance에서 판정한다.

### 24.100 P0-05 완료 판정

Roadmap 완료 조건:

```text
새 Vehicle 생성 흐름을 처음부터 끝까지 설명 가능
→ Sections 24.13~15 / 24.89

기존 Vehicle 수정 흐름 설명 가능
→ Initial Import / Adoption / Managed Edit / Drift Recovery 정의

Raw DA Editor가 필요한 경우가 구분됨
→ Advanced Escape Hatch로 제한, External Drift 경계 명시

Apply 전 Diff / Validation 확인 가능
→ Right Context + Compare + Validation + Bottom Apply Gate 확정
```

추가로:

```text
C++ Slate Production UI 선택 확정
3-pane + bottom action bar Workspace 확정
Management / Sync / Validation 상태 분리
117 Raw leaf 직접 편집 중심 UI 배제
Shared Profile editing 경계 확정
Measurement proposal/adoption UX 확정
Legacy Pin group adoption UX 확정
External Drift 3-way review UX 확정
Recipe edit와 Definition Apply / Save 의미 분리
Advanced Override typed/reason-required UX 확정
P0-09 MVP와 P0-10 parity UI 범위 분리
```

따라서 **DAUTH-P0-05 Vehicle Authoring UX Design은 Complete / P0-06 Ready**로 판정한다.

---

## 25. DAUTH-P0-06 AI Authoring Contract

### 25.1 P0-06 상태와 범위

P0-06은 Section 22의 Core, Section 23의 Wizard Migration, Section 24의 사용자 UX 계약을 변경하지 않고 **AI가 동일 Authoring Core를 호출하는 typed client contract**를 고정한다.

```text
DAUTH-P0-06 = AI Authoring Contract Complete
다음 단계 = DAUTH-P0-07 Batch / Excel / CSV Contract
Project 상태 = Working / Pre-Implementation 유지
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
```

P0-06의 핵심 원칙:

```text
AI는 Authoring System의 별도 Owner가 아니다.
AI는 C++ Authoring Service의 한 Client다.
UI와 AI는 같은 Snapshot / Resolver / Diff / Validation / Apply를 사용한다.
AI 전용 Raw VehicleData mutation path는 존재하지 않는다.
AI의 persistent write는 typed semantic operation 또는 Registry-approved Advanced Override뿐이다.
Target UCFVehicleData write는 FCFVehicleApplyService 하나뿐이다.
```

### 25.2 AI Contract가 해결해야 하는 문제

자연어 요청을 바로 Raw DA field write로 연결하면 다음 문제가 생긴다.

```text
사용자가 의도하지 않은 balance 숫자 생성
Profile / Recipe / Legacy Pin ownership 우회
Source Trace 손실
117 leaf 중 일부만 ad-hoc write
Stale Preview Apply
External Drift 덮어쓰기
Shared Profile 영향 범위 은폐
retry로 중복 mutation
AI와 UI 결과 불일치
```

따라서 P0 AI는 반드시:

```text
Read Context
→ Semantic Intent Plan
→ Prospective Preview
→ User-readable Diff / Validation
→ Approved Authoring Mutation
→ Fresh Resolve
→ Approved Definition Apply
→ Revalidate
```

의 경계를 사용한다.

### 25.3 Module / Dependency 경계

P0-06 구현도 `CarFight_ReEditor` 안에 둔다.
Runtime module은 AI를 알지 않는다.

```text
CarFight_Re Runtime
└─ UCFVehicleData / current runtime consumers
   └─ AI dependency 0

CarFight_ReEditor
├─ Section 22 Authoring Core
├─ Section 24 Slate Workspace
└─ AI Authoring Facade / typed requests
```

AI adapter가 향후 MCP/API/tool surface로 노출되더라도 adapter는 thin transport다.
Resolver / validation / mutation rule을 adapter에 복제하지 않는다.

### 25.4 공통 Facade 책임

P0-08 이후 구현 후보:

```text
FCFVehicleAuthoringService
```

이 service는 새로운 Source Truth가 아니라 기존 Section 22 service를 묶는 Editor facade다.

책임:

```text
Vehicle / Recipe / Profile context query
Immutable snapshot 구성
Prospective semantic change preview
Recipe transaction mutation orchestration
Initial Import orchestration
Adoption / Measurement / Override command orchestration
Resolve / Trace / Diff / Validation query
External Drift review orchestration
Apply Request 생성
FCFVehicleApplyService 호출
Typed result / error 반환
```

비책임:

```text
Field precedence 자체
Resolver 수학
Definition validation 규칙 복제
Fitting 계산
Runtime state mutation
Package Save
AI 자연어 모델 추론
```

### 25.5 구현 파일 후보

실제 파일 생성은 P0-08 이후다.

```text
CarFight_ReEditor/
├─ Public/DataAuthoring/
│  ├─ CFVehicleAuthoringService.h
│  └─ CFVehicleAIContract.h
│
└─ Private/DataAuthoring/
   ├─ CFVehicleAuthoringService.cpp
   └─ CFVehicleAIContract.cpp
```

파일/타입 폭증을 피한다.
operation마다 별도 UObject/class를 만들지 않고 request/result struct와 service method 중심으로 시작한다.

### 25.6 Caller는 권한이 아니다

공통 execution context는 caller identity와 authorization을 분리한다.

후보:

```text
FCFAuthoringCallContext
- ClientOperationId
- CallerKind
- ApprovalClass
- ApprovalScopeHash
```

`CallerKind` 예:

```text
SlateUI
AI
Automation
```

AI라는 이유만으로 더 강한 write 권한을 얻지 않는다.
Slate라는 이유만으로 validation/precondition을 우회하지도 않는다.

### 25.7 Operation Risk Class

P0 operation은 네 등급으로 분리한다.

```text
R0 Read Only
R1 Authoring Record Write
R2 Ownership / Exceptional Authoring Write
R3 Definition Apply
```

#### R0 Read Only

```text
Vehicle / Recipe / Profile 조회
Resolver Preview
Diff / Source Trace / Validation
Reference Compare
Measurement Proposal 조회
Adoption Preview
Drift Review
```

Persistent mutation 0.

#### R1 Authoring Record Write

정상 semantic intent를 Recipe에 기록한다.

```text
Profile Binding
Driving Feel
Asset Intent
Mass / Durability Intent
DriveState Mode
Default Defense / FX Intent
Hardpoint / Mount semantic edit
```

Target Definition mutation 0.

#### R2 Ownership / Exceptional Write

의미나 ownership을 바꾸므로 더 강한 review가 필요하다.

```text
New Vehicle record creation
Existing Definition Initial Import
Adopt Group / Field
Measurement Accept / Compatibility acknowledgement
Stable ID Rename / Remove
Advanced Override add/edit/remove
External Drift Rebase / Raw preservation decision
```

#### R3 Definition Apply

```text
Resolved Diff → Target UCFVehicleData
```

오직 `FCFVehicleApplyService`로 실행한다.

### 25.8 Approval 원칙

`ECFAuthoringApprovalClass` 최소 값은 다음 의미로 고정한다.

```text
None
= R0 read-only 또는 mutation 없는 prospective preview

AuthoringWrite
= reviewed R1 normal Recipe semantic write

OwnershipWrite
= reviewed R2 ownership / exceptional Authoring write

DefinitionApply
= reviewed R3 Target Definition Apply
```

operation risk와 approval class가 맞지 않으면 `ApprovalScopeMismatch`로 Block한다.
더 높은 class를 보냈다는 이유만으로 다른 종류의 operation을 자동 허용하지 않는다.
예를 들어 `DefinitionApply` approval을 `AdvancedOverride` 작성 권한으로 재사용하지 않는다.

Approval은 transport/UI의 문구가 아니라 **어떤 exact mutation을 허용했는지**를 식별해야 한다.

P0 AI contract의 approval은 transient execution authorization이다.
Recipe/VehicleData에 persistent security token을 저장하지 않는다.

최소 approval binding:

```text
Operation Name
Target Definition identity [있을 때]
Recipe identity [있을 때]
Proposed Payload Hash
Expected State Fingerprint
```

R3 Apply는 추가로:

```text
Resolved Definition Hash
Diff Hash
Target Definition Hash
Source Signature
Resolver Contract Revision
```

에 binding한다.

이 중 하나라도 달라지면 approval을 재사용하지 않는다.

### 25.9 무엇을 사용자 승인으로 볼 수 있는가

사용자가 현재 요청에서 exact scope와 값을 명시한 경우 해당 R1 operation의 authorization으로 사용할 수 있다.

예:

```text
"DA_TestSedan의 Handling Profile을 Handling_Sedan으로 바꿔"
```

처럼 Target / operation / desired value가 명확한 요청이다.

반면:

```text
"좀 더 민첩하게 해줘"
```

는 exact `SteeringAgility` 값이나 Profile을 자동 생성할 권한이 아니다.
AI는 기존 Profile/현재 값/available preset을 읽고 **구체적인 proposal**을 만든 뒤 승인받아야 한다.

R2와 R3는 persistent ownership 또는 Definition 결과를 바꾸므로 **reviewed exact action approval**을 요구한다.

### 25.10 Apply Approval은 Preview 이후다

AI가 `Apply` 권한을 작업 시작 시점의 포괄적 문장으로 미리 소비하지 않는다.

표준:

```text
Fresh Resolve
→ exact Diff / Warning / Blocker / Target 설명
→ approval scope 생성
→ user approves reviewed result
→ Apply
```

사용자가 미리 "다 적용해"라고 말했더라도 Preview가 아직 존재하지 않아 Diff/ResolvedHash가 정해지지 않았다면 R3 approval scope를 만들 수 없다.

자동 Apply는 P0 기본 계약에 없다.

### 25.11 Read-before-Write 계약

모든 AI write는 fresh context에 기반해야 한다.

최소 순서:

```text
ReadVehicleContext
→ expected fingerprint 확보
→ prospective preview 또는 exact desired state 결정
→ write request
```

R1/R2 request에는 최소:

```text
ExpectedRecipeFingerprint
ExpectedTargetDefinitionHash [target 관련 operation]
```

를 요구한다.

Profile/Asset dependency가 실제 결과에 참여하면 해당 source fingerprint도 prospective preview에 포함된다.

### 25.12 공통 Request Context

후보 struct:

```text
FCFAuthoringRequestContext

FString ClientOperationId
ECFAuthoringCallerKind CallerKind
ECFAuthoringApprovalClass ApprovalClass
FString ApprovalScopeHash
FString ExpectedRecipeFingerprint
FString ExpectedTargetDefinitionHash
int32 ExpectedResolverContractRevision
```

모든 field가 모든 operation에서 required인 것은 아니다.
operation schema가 required precondition을 별도로 고정한다.

`ClientOperationId`는 persistent Recipe identity가 아니라 **한 client operation의 idempotency key**다.

### 25.13 공통 Result Envelope

모든 AI operation은 성공/실패만 bool로 반환하지 않는다.

후보:

```text
FCFAuthoringOpResult

OperationStatus
ErrorCode
Message
ClientOperationId
bRecipeChanged
bTargetChanged
bProfileChanged
bCreatedAssets
bPackageDirty
bRetryAllowed
AuthoringActionId
CurrentRecipeFingerprint
CurrentTargetDefinitionHash
CurrentSourceSignature
ResolverContractRevision
ValidationSummary
```

read-only operation은 mutation flag가 모두 false여야 한다.

### 25.14 Mutation Footprint는 항상 반환한다

AI는 결과를 보고 다음을 확실히 말할 수 있어야 한다.

```text
Recipe만 바뀌었는가
Target VehicleData가 바뀌었는가
새 Asset이 생겼는가
Shared Profile이 바뀌었는가
Package가 Dirty가 되었는가
Save가 수행되었는가
```

P0 AI contract에서 `SavePerformed`는 항상 false다.
자동 Save operation을 제공하지 않는다.

### 25.15 Operation Status

최소 상태:

```text
Succeeded
NoChange
Blocked
Conflict
FailedRolledBack
FailedUnknownState
```

`NoChange`는 실패가 아니다.
요청 desired state가 이미 만족된 경우 transaction을 만들지 않고 반환할 수 있다.

`FailedUnknownState`는 rollback 결과를 신뢰할 수 없을 때만 사용하고 즉시 fresh Target/Recipe read를 요구한다.

### 25.16 Error Code 범주

P0 최소 typed error:

```text
TargetNotFound
RecipeNotFound
ProfileNotFound
WrongAssetType
UnmanagedRequired
ManagedRequired
StateChanged
RecipeFingerprintMismatch
TargetHashMismatch
ResolverRevisionMismatch
ApprovalRequired
ApprovalScopeMismatch
OperationIdConflict
InvalidSemanticInput
MissingRequiredSource
FieldNotAuthorable
OverrideNotAllowed
StableIdConflict
DependencyConflict
MeasurementNotFound
MeasurementFingerprintStale
AdoptionBlocked
ExternalDriftUnresolved
PreviewOutOfDate
ValidationBlocked
ApplyPreconditionFailed
ApplyValidationFailed
ApplyRollbackFailed
UnsupportedOperation
```

문자열 parsing으로 error type을 추측하지 않는다.

### 25.17 Retry 정책

R0 read는 필요하면 fresh request로 반복할 수 있다.

R1/R2/R3 write는 **automatic retry 금지**다.

```text
StateChanged / FingerprintMismatch
→ fresh read
→ 새 preview
→ 필요 시 새 approval
```

을 사용한다.

Timeout/transport error에서 operation이 실제 실행됐는지 불명확하면 `ClientOperationId` 상태를 조회하거나 fresh object state를 읽기 전 동일 write를 blind retry하지 않는다.

### 25.18 Idempotency 기본 규칙

모든 mutation은 toggle/increment보다 **desired state**로 표현한다.

Good:

```text
SetDrivingFeel(AccelerationFeel = X)
BindProfile(Handling = ProfileAsset)
SetDriveStateMode(ProjectDefault)
```

Bad:

```text
IncreaseAcceleration
ToggleDriveStateOverride
NextProfile
```

같은 desired state request가 이미 성립하면 `NoChange`다.

### 25.19 ClientOperationId dedupe

`FCFVehicleAuthoringService`는 Editor lifetime의 bounded transient dedupe cache를 가질 수 있다.

규칙:

```text
같은 ClientOperationId + 같은 Request Hash
→ 이미 terminal result가 있으면 mutation 반복 금지
→ 기존 terminal result 또는 AlreadyCompleted 의미 반환

같은 ClientOperationId + 다른 Request Hash
→ OperationIdConflict
```

이 cache는 Authoring SSOT가 아니며 Recipe에 저장하지 않는다.
Editor restart 후에는 fresh state read + 새 operation id를 사용한다.

### 25.20 Apply idempotency

R3 Apply는 Section 22의 precondition이 Authority다.

Apply 직전:

```text
Recipe fingerprint
Source signature
Target definition hash
Resolved definition hash
Resolver revision
```

을 다시 확인한다.

Target이 이미 ExpectedResolvedDefinitionHash이고 AppliedState도 현재 source를 반영한다면:

```text
NoChange / Already Applied
```

로 처리하고 두 번째 mutation transaction을 만들지 않는다.

### 25.21 AI Read Operation Catalog

P0 R0 typed query 후보:

```text
ListVehicles
ReadVehicleContext
ListProfiles
ReadProfile
PreviewCreateVehicleRecords
PreviewImportDefinition
ResolveVehiclePreview
ReadPendingDiff
ReadSourceTrace
ReadValidation
PreviewAdoption
ReadMeasurementProposals
ReviewExternalDrift
CompareReferenceVehicles
```

이 operation 이름은 P0-08 C++ method/tool naming에서 소폭 조정할 수 있지만 의미 분리는 유지한다.

### 25.22 `ListVehicles`

입력 후보:

```text
SearchText
ManagementFilter
SyncFilter
ValidationFilter
bIncludeMeshCandidates
```

출력:

```text
Vehicle / Candidate identity
Definition path [있을 때]
Recipe path [있을 때]
Management
Sync
Validation summary
Override count
```

Asset을 수정하지 않는다.

### 25.23 `ReadVehicleContext`

AI write 전에 사용하는 authoritative context read다.

출력 최소:

```text
Entity Kind
Definition identity / path
Recipe identity / path
Management state
Recipe fingerprint
Current Definition hash
Applied Definition hash [있을 때]
Source signature [있을 때]
Resolver Contract Revision
5 Profile bindings
Legacy Pin summary
Advanced Override summary
Stale / Drift summary
Validation summary
```

Raw 117 field 전체가 필요하면 별도 bounded field query를 사용하고 기본 context를 불필요하게 거대하게 만들지 않는다.

### 25.24 `ListProfiles` / `ReadProfile`

AI는 기존 5 Domain Profile을 검색/검토할 수 있다.

`ListProfiles` 출력:

```text
Domain
Profile asset path
Display name
Authoring revision
Fingerprint
Optional usage summary
```

`ReadProfile`:

```text
Typed domain payload
Feel response [해당 Domain]
Direct values
Mass scale rule [있을 때]
Fingerprint
```

P0-06 AI scope에서 **Shared Profile payload mutation operation은 노출하지 않는다.**
AI는 existing Profile을 read/bind한다.
Shared Profile 자체의 AI 편집은 실제 필요가 확인되면 별도 contract로 연다.

#### Create / Import Preview

두 persistent asset/ownership operation은 실행 전 별도 R0 preview를 가진다.

```text
PreviewCreateVehicleRecords
PreviewImportDefinition
```

`PreviewCreateVehicleRecords` 최소 반환:

```text
Definition package path availability
Recipe package path availability
Proposed Target binding
Seed Asset Intent / Profile Binding summary
Expected created asset count
Target Existing Asset overwrite = No
Auto Apply = No
Auto Save = No
ProposalHash
```

`PreviewImportDefinition` 최소 반환:

```text
Target Definition identity
Current Definition Hash
Expected Unmanaged state
Recipe destination availability
Legacy Pin group/field summary
Legacy Serialized Passthrough summary
Optional Profile Suggestions
Target Mutation = No
ProposalHash
```

R2 `CreateVehicleRecords` / `ImportVehicleDefinition`은 해당 preview의 `ProposalHash`와 current expected state에 approval을 binding한다.

### 25.25 `ResolveVehiclePreview`

기존 Recipe를 resolve하는 read-only operation이다.

입력:

```text
Recipe identity
Optional Current Definition context
Optional Fitting Preview context
```

출력은 Section 22 `FCFVehicleResolveResult`를 presentation-safe 형태로 노출한다.

```text
Resolve status
Resolved hash
Source signature
Pending diff
Measurement proposals
Validation layers
Stale report
```

Target mutation 0.

### 25.26 Prospective Recipe Change Preview

AI는 Recipe를 먼저 영구 수정한 뒤에야 결과를 보는 방식만 사용할 필요가 없다.
안전한 자연어 authoring을 위해 **transient prospective preview**를 제공한다.

후보 operation:

```text
PreviewRecipeChange
```

흐름:

```text
Current Recipe Snapshot
→ typed semantic command를 transient copy에 적용
→ 동일 Snapshot Builder / Resolver
→ prospective Diff / Trace / Validation
→ persistent mutation 0
```

이것은 두 번째 Resolver가 아니다.
Section 22의 동일 Resolver input을 사용한다.

### 25.27 Prospective Change Result

최소 반환:

```text
ProposalHash
ExpectedRecipeFingerprint
ProspectiveResolvedHash
ProspectiveSourceSignature
ProspectiveDiff
Validation
Warnings / Blockers
Affected Adoption / Pin state
```

이 `ProposalHash`를 이후 exact R1/R2 approval scope에 사용할 수 있다.

### 25.28 Semantic Write Operation Catalog

P0 R1 후보:

```text
BindVehicleProfile
SetVehicleArchetype
SetVehicleAssetIntent
SetDrivingFeel
SetMassIntent
SetDurabilityIntent
SetDefaultDataIntent
SetWheelVisualIntent
SetDriveStateMode
UpsertHardpointIntent
UpsertMountIntent
```

각 operation은 **자신의 typed semantic payload**만 받는다.

범용:

```text
SetField("VehicleMovementConfig.EngineMaxTorque", "1234")
```

같은 API는 만들지 않는다.

### 25.29 `BindVehicleProfile`

입력:

```text
Recipe identity
Profile Domain
Profile asset identity
ExpectedRecipeFingerprint
ClientOperationId
Authorization
```

검사:

```text
정확한 Domain class인가
asset 존재
Recipe state 허용
```

결과:

```text
Recipe transaction mutation
Recipe revision/fingerprint 갱신
Preview invalidation
Target mutation 0
```

### 25.30 `SetDrivingFeel`

입력은 정확히 Section 22의 네 semantic axis다.

```text
AccelerationFeel
SteeringAgility
GripFeel
SuspensionFirmness
```

일부 axis만 변경하는 typed patch를 허용한다.

```text
입력에 포함된 axis = exact desired value로 Set
입력에서 omitted axis = 변경하지 않음
```

service는 `ExpectedRecipeFingerprint`를 확인한 뒤 current Recipe에 patch를 적용하므로 omitted 값을 stale client snapshot으로 덮지 않는다.

허용 범위는 0..1이다.
값이 범위를 벗어나면 clamp해서 조용히 고치지 않고 `InvalidSemanticInput`을 반환한다.

AI가 자연어에서 임의의 정확한 수치를 발명하지 않는다.

### 25.31 Named Preset 사용

Sedan/SUV/Sports/Heavy shortcut은 Section 23/24에서 보존한 UI convenience다.

AI가 사용하려면:

```text
SetDrivingFeelPreset(PresetId)
```

처럼 typed preset identity를 사용할 수 있다.

이 operation은 내부적으로 current migration-parity exact 4축 semantic input을 적용할 뿐 새로운 SourceType/Profile을 만들지 않는다.

사용자가 해당 preset을 선택하거나 명시적으로 승인하지 않은 상태에서 AI가 차량 형상만 보고 preset을 자동 선택하지 않는다.

### 25.32 Asset Intent Write

`SetVehicleAssetIntent`는 다음처럼 typed reference를 사용한다.

```text
Chassis Mesh
Wheel Mesh FL/FR/RL/RR
Wheel Socket Binding FL/FR/RL/RR
```

Asset reference type을 검증한다.

Mesh를 선택했다는 이유로:

```text
차급
Profile
Driving Feel
Wheel measurement acceptance
```

를 자동 추론/commit하지 않는다.

### 25.33 Hardpoint / Mount AI Write

AI는 stable ID 기반 semantic operation만 사용한다.

```text
UpsertHardpointIntent(LocationSlotId, ...)
RenameHardpointId(OldId, NewId)
RemoveHardpoint(LocationSlotId)

UpsertMountIntent(MountProfileId, ...)
RenameMountId(OldId, NewId)
RemoveMount(MountProfileId)
```

Rename/Remove는 R2다.
Dependency Preview와 explicit approval 없이 실행하지 않는다.

Array index 기반:

```text
HardpointSlots[2]
```

mutation은 금지한다.

### 25.34 New Vehicle Record Creation

후보 operation:

```text
PreviewCreateVehicleRecords   [R0]
CreateVehicleRecords          [R2]
```

R2 실행 전에 matching `ProposalHash`의 create preview가 필요하다.

입력 최소:

```text
Definition Asset Name / Package Path
Recipe Asset Name / Package Path
Optional typed Asset Intent seed
Optional 5 Profile Bindings
ClientOperationId
Approval
```

전제:

```text
두 package path 사용 가능
기존 asset 덮어쓰기 없음
```

결과:

```text
New UCFVehicleData
New UCFVehicleRecipeData
Target binding
Package Dirty
Auto Save 0
Auto Apply 0
```

half-created 상태를 성공으로 반환하지 않는다.
정확한 creation rollback은 P0-09 implementation validation 대상이다.

### 25.35 Existing Definition Initial Import

후보 operation:

```text
PreviewImportDefinition   [R0]
ImportVehicleDefinition   [R2]
```

R2 실행 전에 matching `ProposalHash`의 import preview가 필요하다.

반드시 선행:

```text
ReadVehicleContext = Unmanaged
Read Current Definition Snapshot
PreviewImportDefinition
```

입력:

```text
Target Definition
Recipe package path
Expected Target Definition Hash
Expected Unmanaged state
Approval
```

결과:

```text
Recipe 생성
Legacy Pin 생성
Legacy Serialized Passthrough 보존
Target mutation 0
Auto Adopt 0
Auto Apply 0
```

### 25.36 Import Preview Result

`PreviewImportDefinition`이 AI에게 보여줘야 하는 정보:

```text
Target
Current Definition Hash
Recipe destination
Legacy Pin field/group summary
Legacy Serialized field summary
Optional Profile Suggestions
Target Mutation = No
```

Profile suggestion을 import request payload의 ownership source로 자동 채택하지 않는다.

### 25.37 Adoption Operation

후보:

```text
PreviewAdoption
AdoptGroup
AdoptField
```

`PreviewAdoption`은 R0.
`AdoptGroup/Field`는 R2.

Adopt request에:

```text
ExpectedRecipeFingerprint
Adoption Group / Stable Field Path
Preview ProposalHash
Approval
```

를 요구한다.

Adopt 결과는 Recipe만 바뀌고 Target Definition은 바뀌지 않는다.

### 25.38 AI는 Group Adoption 우선

일반 AI workflow에서도 group adoption을 우선한다.

```text
Handling
Performance
Drivetrain
...
```

Field adoption은 migration exception일 때만 사용한다.

AI가 "관리형으로 전환" 같은 포괄 문구를 받고 117 field의 Legacy Pin을 한 번에 자동 해제하지 않는다.
각 group의 replacement source readiness와 diff를 확인한다.

### 25.39 Measurement Operation

후보:

```text
ReadMeasurementProposals
AcceptMeasurement
ConfirmCompatibilityDefault
```

`ReadMeasurementProposals`는 R0.
나머지는 R2.

Accept request:

```text
Field Path
Measured Candidate Value Hash
Asset Fingerprint
Measurement Rule Id
ExpectedRecipeFingerprint
Approval
```

fingerprint가 바뀌면 `MeasurementFingerprintStale`로 Block한다.
새 measurement를 자동 accept하지 않는다.

### 25.40 Compatibility Default 확인

`ConfirmCompatibilityDefault`는 새 SourceType을 만들지 않는다.

AI는:

```text
Measured value를 쓰지 않고 현재 Project Compatibility Default를 명시적으로 유지한다
```

는 reviewed decision을 Recipe adoption metadata에 기록한다.

정확한 numeric default를 AI가 자체 복사본으로 보관하지 않는다.

### 25.41 Advanced Override Operation

후보:

```text
AddAdvancedOverride
UpdateAdvancedOverride
RemoveAdvancedOverride
```

모두 R2다.

입력:

```text
Structured Stable Field Path
Typed Field Value
Reason
Expected Base Source Signature
ExpectedRecipeFingerprint
Approval
```

`Reason`은 AI contract에서도 필수다.

### 25.42 Advanced Override 허용 검사

AI는 Registry를 우회하지 않는다.

```text
Field descriptor 없음
→ FieldNotAuthorable

bAdvancedOverrideAllowed=false
→ OverrideNotAllowed

Identity / aggregate / hidden legacy
→ OverrideNotAllowed
```

CanonicalValueText raw string을 모델이 직접 만들어 mutation payload로 넘기는 것은 금지한다.
Transport serialization이 문자열이라도 adapter/service가 typed field signature를 검증한 뒤 Field Codec으로 변환한다.

### 25.43 External Drift Read

후보:

```text
ReviewExternalDrift
```

R0다.

출력:

```text
Drift Review Id
Target Hash
Recipe Fingerprint
Source Signature
Resolver Revision
Field rows:
  Last Trusted
  Current Raw
  Current Authoring Resolve
  Allowed Decisions
```

Drift를 읽는 것만으로 review decision을 commit하지 않는다.

### 25.44 External Drift Decision

후보 R2 operation:

```text
CommitDriftDecision
```

field/group별 decision:

```text
KeepAuthoring
PreserveRawAsLegacyPin
PromoteRawToAdvancedOverride
```

`Cancel`은 mutation operation이 아니다.

request는 Section 24와 동일하게 current state에 binding한다.

```text
Drift Review Id
Current Target Hash
Recipe Fingerprint
Source Signature
Resolver Revision
Decision payload
Approval
```

### 25.45 `KeepAuthoring`의 의미

`KeepAuthoring`은 Recipe source를 새로 만들지 않는다.

해당 current-hash-bound drift review lifetime에서:

```text
Raw drift를 source로 흡수하지 않고
현재 Authoring Resolve를 다음 reviewed Apply에서 사용하겠다
```

는 transient decision이다.

Target/Recipe/Source/Resolver state가 바뀌면 decision은 무효다.

### 25.46 `PreserveRawAsLegacyPin`

이 decision은 Rebase 성격의 Recipe mutation이다.

```text
Current Raw value
→ New Legacy Pin candidate
```

으로 보존한다.

Target은 이미 그 Raw 값을 가지고 있으므로 operation 자체는 Target을 쓰지 않는다.
Fresh Resolve / Diff / Validation을 다시 수행한다.

### 25.47 `PromoteRawToAdvancedOverride`

Registry에서 override가 허용된 field만 가능하다.

AI는 반드시 Reason을 제공하고 exact Raw typed value와 Current Target hash에 binding한다.

허용되지 않은 Raw drift를 보존하기 위해 Registry rule을 자동 약화하지 않는다.

### 25.48 Validation Operation

후보:

```text
ReadValidation
```

결과는 Section 24 UX와 같은 layer를 유지한다.

```text
Recipe
Resolver
Definition
Sync / Drift
```

각 issue:

```text
Severity
Stable Field Path [있을 때]
Message
Recommended Action
Apply Blocking
```

AI가 Warning을 Error로 임의 승격하거나 Technical PASS를 USER Acceptance PASS로 확대하지 않는다.

### 25.49 Source Trace Operation

후보:

```text
ReadSourceTrace
```

입력:

```text
Recipe / Resolve Result identity
Optional Stable Field Paths
```

출력:

```text
Ordered Source Layers
Effective Source
Shadow Sources
Value Hash / typed presentation
Source fingerprint
```

AI는 Trace를 읽어 원인을 설명할 수 있지만 `Effective Source`를 직접 지정하지 않는다.

### 25.50 Pending Diff Operation

후보:

```text
ReadPendingDiff
```

결과는 `FCFVehicleResolveResult.FieldDiff`와 동일 authority를 사용한다.

```text
SetLeaf
AddArrayElement
RemoveArrayElement
MoveArrayElement
```

AI가 자체 비교로 Apply payload를 재구성하지 않는다.

### 25.51 Reference Compare

후보:

```text
CompareReferenceVehicles
```

R0다.

Reference Compare 결과를:

```text
CopyFromReference
ApplyReference
```

같은 mutation shortcut으로 연결하지 않는다.

사용자가 Reference와 같은 설정을 원하면 AI는 해당 의미를 Recipe/Profile typed intent로 다시 표현하고 prospective preview를 만든다.

### 25.52 R3 Apply Operation

AI-facing 후보:

```text
ApplyResolvedVehicle
```

이것은 새로운 writer가 아니다.
내부 구현은 반드시:

```text
FCFVehicleAuthoringService
→ FCFVehicleApplyRequest 구성
→ FCFVehicleApplyService
```

다.

AI adapter에서 reflection/raw UObject set을 하지 않는다.

### 25.53 Apply Request Required Evidence

R3 request 최소:

```text
Target Definition identity
Recipe identity
ClientOperationId
Approval Scope
ExpectedRecipeFingerprint
ExpectedSourceSignature
ExpectedTargetDefinitionHash
ExpectedResolvedDefinitionHash
ExpectedDiffHash
ExpectedResolverContractRevision
```

External Drift Reviewed Apply면 추가:

```text
Valid KeepAuthoring Drift Review Decision Id
```

를 요구한다.

### 25.54 Apply 전 서버측 재검사

AI가 위 hash를 보내도 신뢰만 하지 않는다.
service가 현재 UObject/snapshot을 다시 읽는다.

```text
모두 일치
→ ApplyService

하나라도 불일치
→ PreviewOutOfDate / StateChanged
→ mutation 0
```

AI가 old preview를 강제로 적용하는 `force=true` 옵션은 P0에 없다.

### 25.55 Apply 결과

성공:

```text
Succeeded
bTargetChanged=true [실제 diff > 0]
bRecipeChanged=true [AppliedState update]
AppliedFieldCount
ValidationSummary
NewTargetHash
NewRecipeFingerprint
PackageDirty=true
SavePerformed=false
```

이미 동일 결과:

```text
NoChange
bTargetChanged=false
```

실패:

```text
FailedRolledBack
또는
FailedUnknownState
```

partial success를 정상 성공으로 보고하지 않는다.

### 25.56 No Auto Save

AI contract에 다음 operation을 P0에서 제공하지 않는다.

```text
SaveVehicle
SaveRecipe
SaveAllAuthoring
```

Apply는 package를 Dirty로 만들 수 있지만 disk save는 별도 사용자/Editor workflow다.

AI가 "적용 완료"를 "저장 완료"라고 보고하지 않는다.

### 25.57 AI Transaction / Undo 계약

모든 R1/R2/R3 persistent mutation은 logical `FScopedTransaction`을 사용한다.

AI 전용 global blind Undo operation은 P0에서 제공하지 않는다.
이유:

```text
Editor Transaction Stack에는 사용자 작업도 섞일 수 있음
AI가 Ctrl+Z 성격의 global undo를 호출하면 사용자 작업을 되돌릴 위험
```

AI가 수행한 operation은 Undo 가능한 transaction으로 남고 사용자는 Editor 표준 Undo를 사용할 수 있다.
Apply 실패 rollback은 Undo stack에 의존하지 않고 `FCFVehicleApplyService`가 자체 atomic rollback한다.

향후 AI-owned top-of-stack action을 안전하게 식별하는 근거가 필요해지면 별도 bounded Undo contract를 추가한다.

### 25.58 Natural Language → Typed Intent Mapping

자연어 해석은 C++ service 책임이 아니다.
AI layer가 다음 중 하나로만 mapping한다.

```text
Existing Profile identity
Existing named preset identity
Explicit user-provided semantic value
Explicit asset identity
Explicit stable semantic ID
Registry-approved override + typed value + reason
```

grounding이 없는 balance 숫자를 만들어 commit하지 않는다.

### 25.59 Relative Natural Language 요청

예:

```text
"조향을 좀 더 민첩하게"
```

현재 값만 보고 AI가 임의로 `+0.1` 같은 수치를 commit하지 않는다.

허용 흐름:

```text
Current Intent / available Profiles / Presets 읽기
→ 구체적인 exact proposal 생성
→ Prospective Preview
→ 사용자에게 결과 제시
→ approval
→ SetDrivingFeel / BindProfile
```

P0에 자연어 adjective → numeric delta table을 숨은 balance rule로 만들지 않는다.

### 25.60 AI Profile 선택

AI는 Profile을 추천할 수 있으나 추천 근거를 표시한다.

근거 예:

```text
사용자가 명시한 Profile 이름
현재 Vehicle Archetype intent
기존 Authoring convention
reference comparison
```

하지만 Match/Suggestion을 Source ownership으로 자동 승격하지 않는다.
Profile bind와 Legacy group adoption은 별도 operation이다.

### 25.61 AI Shared Profile Write 금지

P0-06에서 AI는 Shared Profile payload를 직접 수정하지 않는다.

금지 이유:

```text
한 Profile 변경이 여러 Vehicle Recipe를 stale시킬 수 있음
P0-06의 요청 범위는 Profile 선택/검토
Batch/Cross-vehicle 영향 계약은 P0-07 이후 검토 가능
```

사용자가 Shared Profile 수정을 요청하면 P0 AI surface는:

```text
Read Profile
Affected Usage Summary [가능하면]
변경 필요 보고
```

까지만 수행하고 typed shared-profile writer 부재를 명시한다.

### 25.62 AI Raw VehicleData Write 금지

다음은 어떤 approval이 있어도 P0 AI operation으로 제공하지 않는다.

```text
Set UObject property by path
Patch UCFVehicleData raw struct
Call Legacy Wizard Apply
Call CaptureLayoutFromChassisSockets direct mutation
Edit hidden legacy mount field
Raw DA editor automation write
```

정말 exceptional raw result가 필요하면 Registry-approved `Advanced Override`로 Authoring intent를 기록한 뒤 정상 Apply한다.

### 25.63 AI Raw Recipe Metadata Write 금지

다음 persistent metadata를 AI가 직접 patch하지 않는다.

```text
AppliedState
SourceTrace
ImportState internal hashes
Legacy Pin array raw representation
Recipe fingerprint
AuthoringRevision
```

이 값은 service operation의 결과로만 갱신된다.

### 25.64 AI Derived Field Write 금지

다음과 같은 derived output을 intent처럼 set하지 않는다.

```text
bUseLayoutOverrides
bUseMovementOverrides
bUseWheelVisualOverrides
DriveState gate raw flag
Wheel Anchor transforms [normal path]
Socket-derived Hardpoint transforms [normal path]
```

필요한 semantic source를 바꾸면 Resolver가 output을 결정한다.

### 25.65 AI Measurement 자동 수락 금지

Mesh bounds 측정 성공은 approval이 아니다.

AI는 candidate를 설명/비교할 수 있지만:

```text
AcceptMeasurement
```

R2 approval 없이 Recipe adoption metadata를 바꾸지 않는다.

### 25.66 AI Legacy Pin 자동 해제 금지

Profile이 현재 value와 같아도:

```text
Legacy Pin → Profile ownership
```

을 자동 수행하지 않는다.

Adoption Preview와 R2 approval을 거친다.

### 25.67 AI External Drift 자동 흡수 금지

External Drift 발견 시:

```text
Raw → Legacy Pin
Raw → Advanced Override
```

를 자동 수행하지 않는다.

ReviewExternalDrift 결과를 먼저 제시하고 exact decision approval을 받는다.

### 25.68 AI Validation 우회 금지

P0에는:

```text
IgnoreValidation
ForceApply
ApplyDespiteError
SkipPrecondition
```

옵션이 없다.

Warning-only Apply는 Section 24와 동일하게 허용 가능하지만 approval summary에 warning count를 포함한다.

### 25.69 AI USER Acceptance 추정 금지

다음을 AI technical result로 만들지 않는다.

```text
Driving Feel PASS
조작감 좋음
UI 사용성 PASS
Visual Layout PASS
```

Resolver/Validation 성공은 Technical Authoring Result일 뿐이다.

### 25.70 AI와 UI parity rule

같은:

```text
Recipe Snapshot
Profile Snapshots
Asset Snapshot
Project Default Snapshot
Target Snapshot
Resolver Contract Revision
```

을 사용하면 UI와 AI의 Resolve hash가 같아야 한다.

같은 ApplyRequest precondition이면 UI와 AI가 다른 Definition 결과를 만들면 안 된다.

### 25.71 UI와 AI가 공유해야 하는 서비스

공유:

```text
Snapshot Builder
FCFVehicleFieldRegistry
FCFVehicleFieldCodec
FCFVehicleResolver
Import Service
Adoption / Recipe mutation service
Measurement decision service
Stale / Drift service
Validation orchestration
FCFVehicleApplyService
```

분리 가능한 것:

```text
Slate presentation
AI natural-language interpretation
Transport serialization
Human-readable response formatting
```

### 25.72 P0 AI Standard Workflow — New Vehicle

```text
1. ListVehicles / identify Mesh Candidate
2. Read candidate context
3. Propose Definition/Recipe paths + Profile bindings
4. CreateVehicleRecords Preview
5. R2 approval
6. CreateVehicleRecords
7. ReadVehicleContext
8. PreviewRecipeChange for semantic intents
9. R1/R2 approved Recipe mutations
10. ResolveVehiclePreview
11. Diff / Trace / Validation report
12. R3 approval
13. ApplyResolvedVehicle
14. Revalidate / report Package Dirty, SavePerformed=false
```

### 25.73 P0 AI Standard Workflow — Existing Import

```text
1. ReadVehicleContext
2. confirm Unmanaged
3. Import Preview
4. show Legacy Pin / Target mutation 0
5. R2 approval
6. ImportVehicleDefinition
7. Read context
8. List/Read Profiles
9. Bind Profile intent as approved
10. PreviewAdoption
11. R2 Adopt approval
12. Adopt Group
13. Resolve / Diff / Validate
14. R3 approval
15. Apply
```

전체 field를 한 번에 auto-adopt하지 않는다.

### 25.74 P0 AI Standard Workflow — Managed Edit

```text
1. ReadVehicleContext
2. verify no unresolved drift
3. Read relevant Profile / Trace
4. Build exact semantic proposal
5. PreviewRecipeChange
6. present prospective Diff / Validation
7. R1/R2 approval
8. Commit Recipe mutation
9. Fresh Resolve
10. verify result
11. R3 approval
12. Apply
13. re-read / revalidate
```

### 25.75 P0 AI Standard Workflow — Drift Recovery

```text
1. ReadVehicleContext
2. External Drift 확인
3. ReviewExternalDrift
4. present 3-way rows
5. exact decision approval
6. CommitDriftDecision [필요한 경우]
7. Fresh Read / Resolve
8. Diff / Validation
9. Reviewed Apply approval
10. Apply via same ApplyService
```

`KeepAuthoring`이면 Step 6은 persistent Recipe mutation 없이 transient reviewed decision만 만든다.

### 25.76 Approval Summary Format

AI가 approval을 요청할 때 최소 다음을 사용자에게 설명한다.

R1/R2:

```text
Target / Recipe
Operation
Authoring values/ownership change
Target VehicleData mutation 여부
Expected Diff summary [prospective preview가 있을 때]
Warnings / Blockers
Save 여부
```

R3:

```text
Target
Changed field count
Array Add/Remove/Move count
Warnings
Resolved Hash short form
External Drift reviewed mode 여부
Target mutation = Yes
Auto Save = No
```

### 25.77 Approval 없이 가능한 AI 행동

기본적으로 R0는 approval 없이 수행할 수 있다.

```text
검색
조회
Preview
Diff
Trace
Validation
Compare
Drift read
Measurement proposal read
Adoption preview
```

단, 대량 read가 별도 tool/runtime policy를 요구하면 transport가 자체 bounded query rule을 적용할 수 있다.
그 transport rule이 Authoring ownership 계약을 바꾸지는 않는다.

### 25.78 Approval Scope 재사용 금지 조건

다음이면 approval invalid:

```text
Target selection 변경
Recipe fingerprint 변경
Target hash 변경
Profile/Asset source signature 변경
Resolver revision 변경
Proposal payload 변경
Diff hash 변경 [Apply]
External Drift Review state 변경
```

AI는 예전 approval을 새 상태에 끼워 맞추지 않는다.

### 25.79 Concurrent / Editor Change 안전성

Snapshot 이후 사용자가 UI에서 값을 바꾸거나 Asset이 reload될 수 있다.

모든 write commit은 Editor/Game thread의 실제 current UObject state를 다시 검사한다.

```text
Expected state != Current
→ StateChanged
→ mutation 0
```

AI가 "방금 읽었으니 그대로일 것"이라고 가정하지 않는다.

### 25.80 Read Scope와 token 절약

AI가 매번 117 field/full trace/full validation을 전부 읽을 필요는 없다.

표준:

```text
ReadVehicleContext summary
→ 관련 Domain / Field만 bounded query
→ Resolve 결과에서는 changed/issue field 우선
```

하지만 Apply approval 전에는 exact complete `FieldDiff`와 validation blocking state가 service 내부에 존재해야 한다.
사용자 보고는 요약할 수 있어도 Apply authority 자체를 축약하지 않는다.

### 25.81 Audit / Diagnostics

P0 AI operation result에 diagnostic metadata를 남길 수 있다.

```text
ClientOperationId
CallerKind=AI
Operation name
AuthoringActionId
Before/After fingerprints
Mutation footprint
```

이것은 Source Trace/Recipe intent를 대체하지 않는다.
별도 장기 audit log 저장 여부는 P0-08 구현에서 결정할 수 있다.

### 25.82 No AI-only Source Type

다음을 새 Source Type으로 만들지 않는다.

```text
AI Generated
AI Suggested
ChatGPT
MCP
```

AI는 사람이 UI에서 선택할 수 있는 동일 source semantics를 기록한다.

```text
Profile
Recipe Explicit
Asset Derived acceptance
Legacy Pin
Advanced Override
```

누가 입력했는지는 optional audit caller metadata 문제이지 Field precedence 문제가 아니다.

### 25.83 No AI-only Recipe fields

P0에서는:

```text
AIConfidence
AIExplanation
AILastPrompt
AIModelName
```

같은 field를 Recipe authoring truth에 추가하지 않는다.

필요한 설명은 Override Reason, operation diagnostics, external work log에서 처리한다.

### 25.84 P0-07 Batch와의 경계

P0-06 operation은 기본적으로 **single Vehicle / single Recipe scope**다.

```text
List / Compare = 여러 Vehicle read 가능
Write / Adopt / Apply = 한 Vehicle target
```

여러 Vehicle에 같은 변경을 반복 적용하는 Batch orchestration은 P0-07에서 별도 계약을 정한다.

AI가 `for each vehicle` loop로 사실상 hidden bulk Apply를 만들어 P0-07 Gate를 우회하지 않는다.

### 25.85 P0-08 구현 시 추가 C++ 타입 후보

최소 후보:

```text
ECFAuthoringCallerKind
ECFAuthoringApprovalClass
ECFAuthoringOpStatus
ECFAuthoringErrorCode

FCFAuthoringCallContext
FCFAuthoringOpResult
FCFAuthoringProposal
FCFDriftReview
FCFDriftDecision
```

기존 Section 22 type을 복제하지 않는다.
Resolve/Diff/Trace/Validation payload는 기존 타입을 그대로 참조하거나 presentation DTO만 얇게 둔다.

### 25.86 P0-08 Automation에 추가할 AI Contract 입력

Section 22의 20개 test에 추가 후보:

```text
AI_ReadOperation_NoMutation
AI_ProspectivePreview_NoMutation
AI_Write_ExpectedFingerprintRequired
AI_Write_StateChangedBlocks
AI_Write_DuplicateIdNoDoubleMutation
AI_Write_SameDesiredStateNoChange
AI_Import_TargetUnchanged
AI_Adopt_TargetUnchanged
AI_Measurement_StaleFingerprintBlocked
AI_Override_RegistryDeniedBlocked
AI_Drift_ReadNoDecision
AI_Drift_OldReviewRejected
AI_Apply_ApprovalScopeMismatchBlocked
AI_Apply_UsesSharedApplyService
AI_Apply_NoAutoSave
AI_Apply_DuplicateNoSecondWrite
AI_NoRawVehicleDataWriter
AI_UIResolverParity
AI_UIApplyParity
AI_BatchWriteNotExposedP0_06
```

정확한 test class/file 배치는 P0-08에서 기존 automation 구조와 함께 결정한다.

### 25.87 Technical Validation에 넘길 Acceptance

P0-11에서는 최소 다음을 검증한다.

```text
AI read path는 asset mutation 0
Prospective preview는 Recipe/Target mutation 0
R1 write는 Recipe만 변경
R2 import/adopt/measurement/override가 계약한 object만 변경
R3만 Target Definition을 변경
AI와 UI 동일 input의 Resolve hash 동일
AI와 UI Apply result 동일
stale preview / stale approval / stale drift review 모두 block
write automatic retry 0
No Auto Save
Typed error와 mutation footprint 정확
```

### 25.88 USER Acceptance에 남기는 것

P0-06 설계 완료로 다음을 추정하지 않는다.

```text
AI가 자연어를 항상 올바른 semantic intent로 이해한다
AI 추천 Profile이 좋은 선택이다
AI가 제안한 Driving Feel이 사용자 취향에 맞다
AI workflow가 수동 UI보다 실제로 빠르다
```

이는 실제 사용과 P0-12 USER Acceptance에서 판단한다.

### 25.89 P0-06 완료 판정

Roadmap 완료 조건:

```text
UI와 AI가 동일 Resolver 사용
→ shared Snapshot/Resolver service + no AI-only resolver

AI 전용 Raw Mutation 로직 없음
→ typed semantic commands + ApplyService single target writer

사용자 자연어 요청을 Authoring Intent로 매핑 가능
→ Profile/Preset/Explicit Semantic/Asset/Stable-ID/Override typed mapping 계약
```

추가로:

```text
R0~R3 operation risk class 확정
Preview-bound approval contract 확정
Read-before-write / expected fingerprint contract 확정
ClientOperationId idempotency와 no blind retry 확정
Typed common result/error/mutation footprint 확정
Prospective Recipe Change Preview 확정
Import / Adoption / Measurement / Override / Drift typed operations 확정
Shared Profile AI write P0 제외
AI global blind Undo 금지 + transaction undoability 유지
No AI-only Source/Recipe metadata 확정
Single-Vehicle write scope로 P0-07 Batch 경계 보호
P0-08 automation 20개 AI 추가 입력 확정
```

따라서 **DAUTH-P0-06 AI Authoring Contract는 Complete / P0-07 Ready**로 판정한다.

---

## 26. DAUTH-P0-07 Batch / Excel / CSV Contract

### 26.1 P0-07 상태와 범위

P0-07은 Section 22~25의 Core / Migration / UX / AI 계약을 변경하지 않고, **여러 Vehicle과 Shared Profile의 반복 Numeric Balance 작업을 안전하게 외부 표에서 수행하는 Batch orchestration contract**를 고정한다.

```text
DAUTH-P0-07 = Batch / Excel / CSV Contract Complete
다음 단계 = DAUTH-P0-08 Implementation Foundation
Project 상태 = Working / Pre-Implementation 유지
Source 변경 = 0
Content Asset 변경 = 0
Runtime 변경 = 0
Plan Index / FeatureQueue / ActiveWork 정식 등록 = 하지 않음
```

P0-07의 핵심 원칙:

```text
Unreal Authoring Asset이 SSOT다.
Spreadsheet는 Export Snapshot + Edit Staging이다.
Spreadsheet 자체는 Source Type이 아니다.
Resolved VehicleData Raw Field는 Spreadsheet write target이 아니다.
Import는 Authoring Source mutation과 Definition Apply를 분리한다.
Batch Apply는 single-Vehicle FCFVehicleApplyService를 우회하지 않는다.
```

### 26.2 P0에서 Batch가 실제로 필요한 이유

현재 1인 개발에서 외부 표가 주는 가장 큰 이점은 다음 두 범위다.

```text
A. 여러 Vehicle의 semantic numeric intent를 같은 표에서 비교 / 수정
B. Shared Profile numeric payload를 같은 Domain 안에서 비교 / 조정
```

반대로 다음은 표가 편해 보여도 Source ownership / identity / derived 의미가 더 중요하다.

```text
Asset Reference
Layout Transform
Hardpoint / Mount 구조
Stable ID Rename / Remove
Measurement Accept
Legacy Adoption
External Drift Decision
Advanced Override 생성
Resolved Raw VehicleData leaf 직접 편집
```

따라서 P0 Batch는 **Numeric Balance에 집중하고 구조 편집은 UI/AI typed operation에 남긴다.**

### 26.3 Excel과 CSV의 P0 역할

P0의 canonical machine interchange는 **UTF-8 CSV + Batch Manifest**로 고정한다.

```text
Canonical Import / Export
= .csv + adjacent .cfbatch.json manifest

Excel 사용
= exported CSV를 Excel에서 열고 편집한 뒤
  CSV UTF-8 형식으로 저장하여 Import
```

P0에서는 native `.xlsx` reader/writer를 구현 필수로 만들지 않는다.
이유:

```text
Unreal Editor 안에 별도 XLSX dependency / parser를 추가하지 않아도 됨
CSV schema / diff / validation contract를 먼저 검증 가능
Excel을 실제 사용자 편집기로 계속 사용할 수 있음
1인 개발에서 파일 포맷 구현 비용을 Authoring Core보다 먼저 늘리지 않음
```

실제 P0-12 사용성에서 CSV↔Excel friction이 크다는 근거가 생기면 native `.xlsx` wrapper를 후속 UX 개선으로 추가할 수 있다.
그 경우에도 Batch canonical model과 import safety 계약은 바꾸지 않는다.

### 26.4 Spreadsheet는 SSOT가 아니다

Authority:

```text
Recipe / Profile Authoring Truth
= Unreal Editor-only Authoring Assets

Resolved Definition Truth
= UCFVehicleData

Spreadsheet
= 특정 시점 Unreal state의 외부 작업 복사본
```

Spreadsheet 변경은 Import Preview를 통과하기 전까지 아무 의미가 없다.

다음을 하지 않는다.

```text
Excel 파일을 Runtime에서 읽기
CSV를 Runtime Definition Source로 참조
Recipe에 CSV file path를 Source로 저장
Profile이 Spreadsheet row를 live-link
Editor 시작 시 CSV 자동 동기화
```

### 26.5 No Spreadsheet Source Type

새 Field Source Type:

```text
Excel
CSV
Spreadsheet
BatchImport
```

을 만들지 않는다.

CSV에서 Recipe Driving Feel을 바꾸면 최종 source 의미는 여전히:

```text
Recipe Explicit Semantic Input
```

이다.

Profile numeric 값을 바꾸면 source 의미는 해당:

```text
Vehicle Base Profile
Drivetrain Profile
Handling Profile
Performance Profile
DriveState Profile
```

이다.

Spreadsheet는 **누가/어떻게 값을 입력했는지에 대한 transport**일 뿐 precedence layer가 아니다.

### 26.6 P0 Dataset Kind

P0 Batch Dataset은 네 종류로 고정한다.

```text
VehicleSummaryReport      [Read Only]
ResolvedFieldReport       [Read Only]
RecipeNumericEdit         [Editable Import]
ProfileNumericEdit        [Editable Import]
```

한 CSV는 정확히 한 Dataset Kind를 가진다.

Recipe와 Profile edit row를 한 CSV에 섞지 않는다.
Read-only Report CSV를 edit import template처럼 사용하지 않는다.

### 26.7 `VehicleSummaryReport`

목적:

```text
여러 Vehicle을 한 행씩 비교
```

한 row = 한 Vehicle Definition / Recipe context.

대표 read-only column:

```text
Vehicle identity
Definition path
Recipe path
Management state
Sync state
Validation summary
Bound Profile summary
Base / Gross Mass summary
Driving Feel intent summary
Drive layout summary
Override count
Legacy Pin count
Stale count
```

정확한 balance value column은 Registry/Recipe schema에서 projection한다.
이 Report는 다시 Import하지 않는다.

### 26.8 `ResolvedFieldReport`

목적:

```text
여러 Vehicle의 exact 117 leaf 결과 / Source를 분석
```

P0에서는 long-form을 기본으로 한다.

```text
Vehicle
Stable Field Path
Current Target Value
Resolved Value
Effective Source Type
Effective Source Id
Diff Status
Validation Status
```

Report schema는 Registry의 **117 leaf pattern 전부를 coverage**해야 한다.

```text
Scalar leaf pattern
→ Vehicle당 한 resolved row

Array leaf pattern
→ 실제 Stable-ID element마다 resolved row로 expand
```

따라서 Hardpoint/Mount element 수에 따라 실제 `ResolvedFieldReport` row 수는 117보다 많아질 수 있으며, `117`을 고정 row count로 해석하지 않는다.
Array field는 Section 22의 Stable Selector canonical path를 사용한다.

이 Report는 **완전 read-only**다.
숫자 cell을 Excel에서 바꾸어도 Import 대상이 되지 않는다.

### 26.9 Resolved Raw Field를 Import하지 않는 이유

`ResolvedFieldReport`의 숫자를 다시 Import하면:

```text
어느 Profile을 고쳐야 하는지
Recipe Explicit 값인지
Asset Derived인지
Legacy Pin인지
Advanced Override인지
Derived gate인지
```

가 사라진다.

따라서:

```text
Resolved VehicleData field를 외부에서 바꾸고 싶다
→ Source Trace 확인
→ owning Recipe/Profile semantic input을 편집
→ exceptional case는 UI/AI Advanced Override
```

를 사용한다.

P0에 `RawResolvedNumericImport`는 없다.

### 26.10 `RecipeNumericEdit`

목적:

```text
여러 Vehicle Recipe의 반복 numeric semantic intent를 한 표에서 편집
```

한 row = 한 Recipe / Target Vehicle.

Writable candidate는 **Section 21 Matrix + Section 22 Recipe schema상 Recipe가 직접 소유하는 scalar numeric semantic input**으로 제한한다.

P0 최소 확정 writable:

```text
Driving Feel 4 axis
```

추가로 Mass / Durability 등 Recipe struct 안의 scalar numeric semantic field는 P0-08 Batch Column Registry가 다음을 모두 확인할 때만 writable로 승격한다.

```text
Field Ownership Matrix = Recipe Semantic Input
Scalar numeric type
Stable semantic meaning
Asset / Runtime / Fitting state 아님
Identity / array key 아님
Derived output 아님
External edit가 ownership change를 요구하지 않음
```

P0-07에서 새 numeric field/default를 발명하지 않는다.

### 26.11 Recipe Source Mode 변경은 Batch에서 하지 않는다

일부 Recipe semantic block은:

```text
Use Profile
Explicit Value
Explicit None
```

같은 source mode를 가질 수 있다.

P0 `RecipeNumericEdit`는 **기존 source mode를 바꾸지 않는다.**

예:

```text
Base Mass가 현재 Explicit Value
→ numeric value edit 가능 [Batch descriptor 허용 시]

Base Mass가 현재 Use Profile
→ Spreadsheet numeric cell은 read-only / unavailable
→ Batch에서 Profile→Explicit ownership 전환 금지
```

Source ownership 변경은 Section 24/25 UI/AI semantic operation에서 수행한다.

### 26.12 Legacy Pin 아래 Recipe Intent

Legacy Imported / Partially Managed Vehicle에서 Driving Feel 같은 future intent는 Section 24와 동일하게 Batch edit할 수 있다.

```text
Recipe numeric edit = 가능
Effective Source = Legacy Pin
Definition Diff = 0 또는 일부 shadowed
Batch Preview Status = ShadowOnly
```

즉 Spreadsheet가 Legacy Pin을 해제하지 않는다.

### 26.13 `ProfileNumericEdit`

목적:

```text
같은 Domain Shared Profile의 numeric balance payload를 비교 / 조정
```

한 file은 한 Profile Domain만 포함한다.

예:

```text
ProfileNumericEdit.VehicleBase
ProfileNumericEdit.Drivetrain
ProfileNumericEdit.Handling
ProfileNumericEdit.Performance
ProfileNumericEdit.DriveState
```

서로 다른 Domain의 typed payload를 한 wide table에 억지로 합치지 않는다.

### 26.14 Profile Batch Write는 P0-07에서 별도 허용한다

Section 25에서는 AI single-operation Shared Profile payload write를 P0-06에서 제외했다.
P0-07은 실제 Numeric Balance 필요성이 명확하므로 **reviewed Batch Profile Numeric Edit**만 별도 계약으로 연다.

이것은 Section 25의 AI generic profile writer를 새로 여는 것이 아니다.

```text
Single Profile arbitrary AI write
= 계속 P0-06 surface에 없음

Reviewed ProfileNumericEdit Batch
= P0-07 Batch Service를 통해 허용
```

Shared Profile 변경은 dependency 영향이 크므로 Recipe batch보다 강한 affected-Vehicle preview를 요구한다.

### 26.15 Profile writable field 범위

Writable:

```text
Profile typed payload의 scalar numeric value
FCFFeelResponse Low / Neutral / High numeric payload
opt-in numeric MassScale rule parameter [해당 Profile schema가 소유할 때]
기타 Field Registry / Profile schema가 numeric batch editable로 명시한 값
```

P0 Spreadsheet에서 writable 아님:

```text
Profile asset identity / path
Profile Domain
Display metadata
Revision / Fingerprint
Object Reference
Enum
Bool
Array / Map / Set
Nested identity struct
```

Nested struct 안의 numeric leaf는 Batch descriptor가 **독립 scalar semantic leaf**로 명시한 경우에만 flattened column으로 노출할 수 있다.

### 26.16 Profile 변경의 affected Vehicle scope

Profile batch preview는 해당 Profile을 binding한 Recipe를 모두 찾는다.

```text
Changed Profile
→ Bound Recipe inventory
→ 각 Recipe에서 Profile이 effective / shadowed인지 확인
→ prospective Resolve
→ affected Vehicle Diff / Validation 집계
```

Legacy Pin / Advanced Override 때문에 Profile이 shadowed인 Vehicle도 usage 목록에는 남기되:

```text
Effective Change = 0
Shadow Source Changed = Yes
```

로 구분한다.

### 26.17 Profile Batch Commit Validation Gate

Shared Profile은 여러 Vehicle에 영향을 줄 수 있으므로 Batch Profile commit 전 **모든 binding Recipe의 dependency inventory를 완성**하고, 그중 Profile 변경이 effective output에 참여하는 Recipe를 prospective resolve한다.

다음이면 Profile Batch Authoring Commit을 Block한다.

```text
Profile typed input invalid
Profile schema violation
Dependent Recipe inventory 불완전
Effective / Shadowed 분류 불가
Profile 변경이 effective인 Recipe resolve 불가
Profile 변경으로 effective affected Vehicle에 Error / Blocked Definition state 발생
```

Warning-only는 review summary를 포함하고 commit 가능하다.

Profile이 Legacy Pin / Advanced Override 때문에 완전히 shadowed되어 **Prospective Effective Change = 0**인 Vehicle은 usage/audit 목록에는 남기되, unrelated 기존 Definition Error만으로 Profile commit을 막지 않는다.
다만 shadow 상태 판정 자체를 신뢰할 수 없으면 affected scope incomplete로 Block한다.

### 26.18 Asset Reference P0 정책

Asset Reference는 Spreadsheet에서 **read-only export만 허용**한다.

예:

```text
Chassis Mesh
Wheel Mesh
Default Defense Data
Destroyed FX Data
Default Equipment Preset
```

이유:

```text
path typo / rename
asset class 검증
Content Browser 탐색 필요
Reference 선택 자체가 semantic operation
```

Asset 선택은 Section 24 typed picker 또는 Section 25 typed AI operation을 사용한다.

### 26.19 Stable ID / Array P0 정책

다음은 Spreadsheet write 금지다.

```text
Hardpoint LocationSlotId
MountProfileId
LocationSlotRef
Hardpoint / Mount Add
Hardpoint / Mount Remove
Stable ID Rename
Array order
```

Read-only Report에서는 Stable ID를 표시할 수 있다.

Identity mutation은 dependency preview가 필요한 Section 24/25 R2 operation으로 유지한다.

### 26.20 Complex Struct P0 정책

Spreadsheet writable 대상에서 제외:

```text
FTransform
FVector
FRotator
FGameplayTag container류
Object complex struct
Array / Map / Set
Nested identity-bearing struct
```

단순 struct 안의 scalar numeric leaf가 독립 semantic meaning을 가지며 Batch descriptor에 등록된 경우만 flattened numeric column을 허용한다.

### 26.21 Asset Derived / Measurement 정책

다음 값은 Spreadsheet에서 직접 edit/import하지 않는다.

```text
Wheel Anchor Location / Rotation
Socket-derived Hardpoint Transform
Measured Wheel Radius / Width candidate
Asset Fingerprint
Measurement adoption state
```

Measurement candidate는 Report로 export할 수 있지만 acceptance는 Section 24/25의 explicit reviewed decision을 사용한다.

### 26.22 Advanced Override 정책

Advanced Override는 Spreadsheet에서 생성/수정/삭제하지 않는다.

Report에는 다음을 read-only로 포함할 수 있다.

```text
Override Active
Override Value
Base Source
Reason
```

Raw numeric cell을 바꿔 Override를 암묵적으로 만드는 것을 금지한다.

### 26.23 Batch Column Registry

P0-08에서 `FCFBatchColumnRegistry` 같은 projection registry를 둘 수 있다.

각 descriptor 최소:

```text
ColumnId
DatasetKind
DisplayLabel
ValueType
Unit
Access = ReadOnly / Editable
AuthoringOwner
TypedMutationKind
Property / Semantic Target
BlankPolicy
ValidationMetadata
```

이 Registry는 새 Source Truth가 아니다.

```text
Resolved report columns
→ FCFVehicleFieldRegistry 117 projection

Recipe edit columns
→ Recipe schema / semantic operation projection

Profile edit columns
→ typed Profile schema projection
```

수동으로 VehicleData raw numeric table을 복제하지 않는다.

### 26.24 Stable Column ID

CSV header는 localized display name이 아니라 stable technical `ColumnId`를 사용한다.

예시 형식:

```text
__cf_target
__cf_recipe
Recipe.DrivingFeel.Acceleration
Recipe.DrivingFeel.SteeringAgility
```

실제 exact ColumnId 문자열은 P0-08 registry 구현에서 확정하지만 다음 규칙을 지킨다.

```text
rename 가능한 사용자 label을 identity로 사용하지 않음
Spreadsheet column order를 identity로 사용하지 않음
localized Korean label을 import key로 사용하지 않음
```

Excel 사용자에게 보여줄 한국어 label / 설명 / unit은 manifest metadata 또는 별도 안내 row/view로 제공할 수 있다.

### 26.25 Reserved Metadata Column

`__cf_` prefix는 importer-owned metadata로 예약한다.

Recipe edit row 최소 후보:

```text
__cf_row_id
__cf_target
__cf_recipe
__cf_export_recipe_fingerprint
__cf_export_target_hash
__cf_schema_id
__cf_schema_revision
```

Profile edit row:

```text
__cf_row_id
__cf_profile
__cf_profile_domain
__cf_export_profile_fingerprint
__cf_schema_id
__cf_schema_revision
```

Reserved column은 read-only다.
수정되면 importer가 identity 변경으로 받아들이지 않고 error를 낸다.

### 26.26 Manifest 역할

CSV 옆의 `.cfbatch.json`은 export snapshot contract를 보존한다.

최소:

```text
BatchExportId
DatasetKind
SchemaId
SchemaRevision
ResolverContractRevision
Exported Row Identities
Editable Column Descriptors
Baseline canonical values
Baseline object fingerprints
Baseline ownership/source mode
ExportSetHash
```

Manifest는 Unreal SSOT가 아니다.
Import에서 3-way comparison을 수행하기 위한 immutable export baseline evidence다.

### 26.27 Manifest 변경을 신뢰하지 않는다

Manifest는 보안 서명 토큰이 아니다.
사용자가 실수로 수정하거나 손상할 수 있다.

따라서 Import는 Manifest 내용만 믿고 mutation하지 않는다.

항상:

```text
Manifest schema 검증
CSV↔Manifest row/column 일치 검증
Current Unreal Asset 재조회
Current fingerprint / ownership 확인
Typed validation
```

을 수행한다.

Manifest가 손상되었거나 누락되면 Import를 Block하고 새 Export를 요구한다.

### 26.28 CSV Canonical Value 규칙

P0 canonical CSV:

```text
UTF-8
comma-delimited
header = stable ColumnId
numeric decimal = invariant '.'
unit text를 numeric cell에 포함하지 않음
NaN / Infinity 금지
```

quoted string escaping은 표준 CSV 규칙을 따른다.

P0 editable column은 numeric 중심이므로 locale-dependent display string을 canonical value로 사용하지 않는다.

### 26.29 Excel Formula / Macro 정책

P0 canonical import는 CSV이므로 formula engine을 신뢰하지 않는다.

editable numeric cell에 다음이 들어오면 값으로 평가하지 않고 parse error다.

```text
=SUM(...)
=A2*1.1
external workbook reference
macro expression
```

사용자는 Excel 안에서 계산을 보조할 수 있지만 **Import할 최종 editable cell은 plain numeric value**여야 한다.

native `.xlsm` / macro import는 P0에 없다.

### 26.30 Blank Cell 의미

Editable column에서 blank는:

```text
No Change
```

다.

blank를:

```text
0
None
Clear
Reset to Profile
```

로 해석하지 않는다.

Optional semantic value를 clear하거나 Source Mode를 바꾸는 작업은 P0 Spreadsheet에서 하지 않고 UI/AI typed operation을 사용한다.

### 26.31 Read-only Cell 수정 처리

Editable template 안의 read-only/reserved cell이 export baseline과 달라지면 해당 row를 silent ignore하지 않는다.

```text
ReadOnlyColumnModified
```

로 Block한다.

사용자가 수정한 값이 실제로 적용되지 않는다고 오해하는 것을 막기 위해서다.

### 26.32 Duplicate Row 정책

동일 Recipe/Profile identity가 CSV에 두 번 나오면:

```text
DuplicateRowIdentity
```

로 file preview를 Block한다.

아래 row가 위 row를 덮는 last-write-wins는 사용하지 않는다.

### 26.33 Row Order 의미 없음

CSV row order는 identity도 precedence도 아니다.

Batch plan은 deterministic하게 canonical asset path / row id 기준으로 정렬한다.

사용자가 Excel에서 정렬해도 결과 semantics는 달라지지 않는다.

### 26.34 Export Baseline과 3-way Import

Import는 단순히:

```text
CSV value != Current Unreal value
```

만 비교하지 않는다.

각 editable cell에:

```text
Export Baseline
Edited Spreadsheet Value
Current Unreal Authoring Value
```

3개를 비교한다.

### 26.35 3-way Merge 규칙

#### Case A — 현재 Unreal이 baseline 그대로

```text
Current == Export Baseline
Edited != Baseline
→ Safe Candidate
```

#### Case B — 사용자는 cell을 안 바꿈

```text
Edited == Export Baseline
Current != Baseline
→ No Spreadsheet Change
→ Current Unreal 유지
```

#### Case C — 같은 최종 값으로 수렴

```text
Current != Baseline
Edited == Current
→ NoChange
```

#### Case D — 양쪽 모두 다르게 수정

```text
Current != Baseline
Edited != Baseline
Edited != Current
→ ConcurrentEditConflict
```

Spreadsheet가 조용히 Unreal 변경을 덮지 않는다.

### 26.36 Ownership Changed Conflict

Numeric 값이 같더라도 Export 이후 Authoring ownership/source mode가 바뀌면 editable eligibility가 달라질 수 있다.

예:

```text
Export: Mass = Recipe Explicit
Current: Mass = Use Profile
```

이 경우 CSV numeric edit는:

```text
OwnershipChangedSinceExport
```

로 Block한다.

자동으로 Source Mode를 되돌리지 않는다.

### 26.37 Fingerprint 역할

Row-level fingerprint는 빠른 freshness signal이다.

```text
Recipe fingerprint unchanged
→ direct cell compare 가능

Recipe fingerprint changed
→ 3-way cell merge 검사
```

Row fingerprint mismatch 자체로 무조건 전체 row를 실패시키지는 않는다.
관련 없는 Recipe field가 UI에서 바뀌었을 수도 있기 때문이다.

다만 commit 직전에는 **fresh preview에서 계산한 current fingerprint**를 exact precondition으로 다시 사용한다.

### 26.38 Batch Import Session

CSV를 읽는 것만으로 Asset을 수정하지 않는다.

후보 transient object/model:

```text
FCFBatchImportSession
```

보유:

```text
Dataset kind
Manifest / schema
Parsed rows
Current Unreal snapshots
3-way merge result
Prospective authoring patches
Prospective resolve summaries
Conflicts / validation
BatchPlanHash
```

이 Session은 persistent Authoring Asset이 아니다.
Editor restart 후에는 CSV와 current Unreal state로 다시 생성한다.

### 26.39 Batch Row Status

최소:

```text
Unchanged
Candidate
ShadowOnly
Warning
Blocked
Conflict
ExternalDrift
```

하나의 row가 여러 issue를 가질 수 있으므로 status 외에 issue list도 보존한다.

### 26.40 Recipe Batch Prospective Preview

`RecipeNumericEdit` Import Preview:

```text
CSV parse
→ 3-way merge
→ current Recipe Snapshot에 candidate numeric patch transient 적용
→ same Resolver
→ prospective Source Trace
→ prospective Definition Diff
→ Validation
→ per-row status
```

persistent Recipe / Target mutation 0.

### 26.41 Profile Batch Prospective Preview

`ProfileNumericEdit` Import Preview:

```text
CSV parse
→ Profile 3-way merge
→ transient Profile payload patch
→ affected Recipe inventory
→ 각 Recipe를 transient changed Profile Snapshot으로 resolve
→ per-Vehicle Diff / Trace / Validation
→ Profile Batch Plan
```

Profile/Recipe/Target mutation 0.

### 26.42 Batch Preview Summary

사용자에게 최소:

```text
Dataset Kind
Edited row count
Candidate / NoChange / ShadowOnly / Warning / Blocked / Conflict count
Affected Recipe/Profile count
Affected Vehicle count
Prospective Definition changed field count
Array structural change count [P0 numeric edit에서는 정상적으로 0]
External Drift count
Validation Error/Blocked/Warning count
```

을 제공한다.

### 26.43 Per-Row Review

각 row:

```text
Target / Recipe or Profile
Changed editable cells
Before / After
Authoring owner
Prospective effective impact
Shadow reason [있을 때]
Vehicle Definition Diff count
Validation
Conflict reason
```

Shared Profile row는 affected Vehicle list를 drill-down할 수 있어야 한다.

### 26.44 Spreadsheet Import는 바로 Apply가 아니다

매우 중요한 고정 흐름:

```text
External CSV
→ Parse / Schema / 3-way
→ Prospective Batch Preview
→ Batch Authoring Approval
→ Commit Authoring Source Changes
→ Fresh Unreal Read
→ Fresh Resolve All Affected Vehicles
→ Build Batch Definition Apply Plan
→ Separate Batch Apply Approval
→ per-Vehicle FCFVehicleApplyService
```

CSV Import와 VehicleData Apply를 한 버튼/transaction으로 합치지 않는다.

### 26.45 왜 Apply Preview를 다시 만드는가

Authoring Source Commit 후:

```text
Recipe fingerprint
Profile fingerprint
Source signature
Resolved hash
```

가 바뀐다.

따라서 CSV Import 전에 본 prospective Diff를 R3 Apply authorization으로 그대로 재사용하면 Section 25의 stale approval 금지와 충돌한다.

Source Commit 후 **fresh Resolve + fresh Batch Apply Plan**이 필수다.

### 26.46 Batch Operation Class

P0-07 Batch-level operation을 다음으로 구분한다.

```text
B0 Batch Read / Export / Preview
B1 Recipe Numeric Authoring Commit
B2 Shared Profile Numeric Commit
B3 Batch Definition Apply Orchestration
```

B3가 새 Definition writer를 의미하지 않는다.
각 item은 기존 single-Vehicle R3 `FCFVehicleApplyService`다.

### 26.47 B1 Recipe Numeric Commit

한 `RecipeNumericEdit` Batch Plan의 candidate Recipe patch를 commit한다.

전제:

```text
BatchPlanHash 일치
모든 included row current precondition 일치
Conflict 0
ReadOnlyColumnModified 0
Input / Recipe typed validation Blocker 0
```

Target VehicleData는 변경하지 않는다.

### 26.48 B1 Atomicity

B1은 **하나의 logical Editor Transaction에서 all-or-nothing**으로 처리한다.

표준:

```text
모든 Recipe precondition 선검사
→ 하나라도 stale이면 mutation 0
→ FScopedTransaction
→ changed Recipe 전부 Modify
→ deterministic order로 typed patch
→ 각 Recipe post-check
→ 실패 시 touched Recipe 전부 pre-batch snapshot 복원
→ transaction cancel
```

B1 성공 후 Recipe package들은 Dirty가 될 수 있으나 Auto Save는 하지 않는다.

### 26.49 B2 Shared Profile Numeric Commit

B2도 한 Dataset / Domain Batch Plan 단위의 reviewed transaction이다.

전제:

```text
모든 Profile current fingerprint 일치
Affected Recipe inventory 재확인
Prospective affected-Vehicle validation gate 통과
BatchPlanHash 일치
```

하나라도 다르면 mutation 0 + 새 Preview를 요구한다.

### 26.50 B2 Atomicity

B2는 Profile Authoring Source 변경 자체에 대해 all-or-nothing이다.

```text
모든 Profile precondition 선검사
→ FScopedTransaction
→ changed Profile 전부 Modify
→ typed numeric patch
→ Profile fingerprint 갱신
→ post-check
→ 실패 시 전체 Profile source mutation rollback
```

성공 후 dependent Recipe의 Definition은 자동 Apply하지 않는다.

```text
Profile Package = Dirty
Dependent Vehicle = Effective Stale 또는 Shadow Source Changed
```

로 표시한다.

### 26.51 Authoring Commit approval binding

B1/B2 approval 최소:

```text
Dataset Kind
Schema Id / Revision
BatchExportId
BatchPlanHash
Included Row Id set
Per-row Proposed Patch Hash
Per-row Current Fingerprint
Conflict / Warning Summary
```

사용자가 일부 row를 제외하려면 기존 approval을 축소해서 재사용하지 않는다.
새 selection으로 새 Batch Plan / Hash를 만든다.

### 26.52 Authoring Commit 중 External Drift

External Drift는 Target Definition과 Authoring Source의 불일치다.
Recipe numeric intent 자체의 edit를 반드시 막는 상태는 아니다.

따라서 B1에서:

```text
External Drift row
→ Recipe semantic patch 자체가 3-way safe하면 Commit candidate 가능
→ 하지만 Definition Apply Eligible = No
→ Drift warning을 명확하게 표시
```

B2 Profile commit도 Profile Source 자체는 commit할 수 있지만 affected Vehicle의 Drift는 이후 Definition Apply를 Block한다.

Spreadsheet가 Drift를 해결하거나 흡수하지 않는다.

### 26.53 Stale / Shadow 처리

`Effective Stale`은 Batch Source edit의 blocker가 아니다.
Fresh prospective resolve가 current state를 기준으로 계산한다.

`Shadow Source Changed`도 blocker가 아니다.

단 결과는:

```text
Effective Definition impact
Shadow-only impact
```

를 분리해서 보여준다.

### 26.54 Batch Definition Apply Plan

B1/B2 Commit이 끝나면 affected Vehicle을 새로 읽고 fresh Resolve한다.

후보:

```text
FCFBatchDefinitionApplyPlan
```

각 item:

```text
Target identity
Recipe identity
ExpectedRecipeFingerprint
ExpectedSourceSignature
ExpectedTargetDefinitionHash
ExpectedResolvedDefinitionHash
ExpectedDiffHash
ExpectedResolverContractRevision
Validation Summary
Warning Summary
```

즉 Section 25 R3 evidence의 collection이다.

### 26.55 Batch Apply 대상 선택

Apply Plan에는 기본적으로:

```text
Fresh Preview
Diff > 0
Error / Blocked = 0
External Drift = 0
```

인 item만 eligible하다.

NoChange / ShadowOnly item은 Apply하지 않는다.

External Drift가 있는 item은 P0 Batch에서 bulk drift decision으로 해결하지 않는다.
Section 24/25 per-Vehicle Drift Review를 먼저 완료한 뒤 새 Batch Apply Plan을 만든다.

### 26.56 External Drift bulk resolution 금지

P0 Batch에 다음을 만들지 않는다.

```text
KeepAuthoringForAllDriftedVehicles
RebaseAllRawValues
PromoteAllDriftToOverrides
```

Drift는 raw out-of-band edit 의미가 서로 다를 수 있으므로 per-Vehicle review를 유지한다.

### 26.57 B3 Batch Apply Approval

B3 approval은 exact Apply Plan에 binding한다.

최소:

```text
BatchApplyPlanHash
Ordered Target Set
Per-Target R3 evidence hash
Total Vehicle Count
Total Field Diff Count
Warnings
Array structural diff count
Auto Save = No
```

CSV Import 단계의 B1/B2 approval을 B3에 재사용하지 않는다.

### 26.58 B3 Global Preflight

첫 Vehicle을 수정하기 전에 **모든 eligible item의 current precondition을 다시 검사**한다.

```text
하나라도 mismatch
→ Batch Apply 전체 시작 안 함
→ Applied Vehicle = 0
→ 새 Resolve / Plan 필요
```

이 preflight는 가능한 stale partial-start를 줄인다.

### 26.59 B3 실행 순서

Global preflight PASS 후 deterministic order로 실행한다.

기본 order:

```text
Canonical Target Asset Path ascending
```

각 Vehicle은:

```text
FCFVehicleApplyService
```

를 정확히 한 번 호출한다.

### 26.60 B3 Atomicity — Per Vehicle Only

Batch Definition Apply는 **global atomic을 보장하지 않는다.**

Authority:

```text
각 Vehicle Apply = atomic
전체 Batch = sequence of atomic Vehicle Applies
```

이유:

```text
Section 22 ApplyService가 한 Target + Recipe transaction owner
nested/global transaction으로 기존 ApplyService를 우회하지 않음
이미 성공한 Vehicle을 inverse-write로 자동 rollback하는 위험 회피
```

### 26.61 Mid-Batch Failure

실행 도중 한 Vehicle이 실패하면 P0 기본 정책은:

```text
Stop On First Failure
```

이다.

결과:

```text
Applied
Failed
NotStarted
```

를 exact target list로 반환한다.

이미 성공한 Vehicle을 자동 rollback하지 않는다.
남은 Vehicle을 자동 continue하지 않는다.

### 26.62 Partial Failure Recovery

부분 성공 후:

```text
1. 전체 affected context fresh read
2. 성공 Vehicle = 이미 Applied / NoChange로 재분류
3. 실패 원인 해결
4. NotStarted + Failed 대상의 새 Batch Apply Plan 생성
5. 새 approval
6. 다시 Apply
```

동일 BatchApplyPlanHash를 blind retry하지 않는다.

### 26.63 왜 Global Rollback을 하지 않는가

이미 성공한 Vehicle을 되돌리려면:

```text
inverse diff 재구성
새 Target hash와 충돌 가능
Recipe AppliedState 복원
Validator 재실행
다른 Editor edit와 충돌
```

이 필요하다.

P0에서는 이를 자동화하지 않고 per-Vehicle atomic guarantee와 정확한 partial result를 우선한다.

### 26.64 Batch Apply Undo

각 `FCFVehicleApplyService` 성공은 기존 Editor transaction으로 Undo 가능하다.

하지만 P0에서:

```text
Undo Entire Batch
```

라는 별도 global blind operation은 제공하지 않는다.

Batch 도중 사용자/Editor transaction stack과 섞일 수 있으므로 안전한 batch-owned grouped undo 근거가 생길 때만 후속 검토한다.

### 26.65 No Auto Save

B1/B2/B3 모두:

```text
Package Dirty 가능
Auto Save = 0
```

이다.

Batch UI/AI가 `Batch Applied`를 `Saved`라고 보고하지 않는다.

### 26.66 Batch Idempotency

후보 id:

```text
BatchOperationId
```

B1/B2:

```text
BatchOperationId + BatchPlanHash
```

B3:

```text
BatchOperationId + BatchApplyPlanHash
```

를 transient dedupe key로 사용한다.

같은 id + 같은 hash의 terminal result가 있으면 동일 mutation을 반복하지 않는다.
같은 id + 다른 hash는 `BatchOperationIdConflict`다.

### 26.67 Batch Write Retry

B1/B2/B3 모두 automatic retry 금지다.

Transport timeout에서 실행 여부가 불명확하면:

```text
BatchOperationId result 조회
또는
fresh Unreal object state read
```

전에는 동일 write를 재실행하지 않는다.

### 26.68 Batch Error Code 후보

P0 추가 typed error:

```text
BatchManifestMissing
BatchManifestInvalid
BatchSchemaMismatch
BatchDatasetNotImportable
BatchColumnUnknown
BatchReadOnlyColumnModified
BatchDuplicateRowIdentity
BatchCellParseError
BatchOwnershipChanged
BatchConcurrentEditConflict
BatchPlanOutOfDate
BatchApprovalRequired
BatchApprovalScopeMismatch
BatchAffectedScopeIncomplete
BatchValidationBlocked
BatchGlobalPreflightFailed
BatchOperationIdConflict
BatchApplyPartialFailure
```

per-Vehicle error는 Section 25 `ECFAuthoringErrorCode`를 그대로 nested result에 사용할 수 있다.

### 26.69 Batch Result Envelope

후보:

```text
FCFBatchOpResult

BatchOperationId
BatchStatus
BatchErrorCode
BatchPlanHash
DatasetKind
TotalRows
ChangedRows
NoChangeRows
ShadowOnlyRows
WarningRows
BlockedRows
ConflictRows
bAuthoringSourcesChanged
bAnyTargetChanged
bPackageDirty
SavePerformed=false
PerRowResults
PerVehicleApplyResults
```

`BatchApplyPartialFailure`에서는 Applied/Failed/NotStarted target을 반드시 구분한다.

### 26.70 Batch UI 구조 후보

Section 24 Workspace에 별도 `Batch` main page를 P0-09 필수로 넣지 않는다.
P0-08 foundation 이후 실제 필요 시 Nomad Tab 또는 Workspace secondary page로 구현할 수 있다.

필수 workflow component:

```text
Export
Import File
Import Preview Summary
Row Review Table
Conflict Filter
Affected Vehicle Drill-down
Commit Authoring Changes
Build Definition Apply Plan
Apply Review
Apply Batch
Export Review Report
```

UI 위치는 구현 ergonomics 문제이고 Batch contract Authority는 본 Section이다.

### 26.71 Import Preview Filter

최소:

```text
Changed Only
Conflict
Blocked
Warning
Shadow Only
External Drift
Profile / Domain
Vehicle
Column
```

사용자가 문제가 있는 row를 바로 찾을 수 있어야 한다.

### 26.72 사용자 수정과 Batch selection

사용자가 conflict row를 제외하고 나머지만 진행하려면:

```text
Exclude Rows
→ New Included Row Set
→ Rebuild Preview
→ New BatchPlanHash
→ New Approval
```

이다.

Block된 row를 UI에서 check 해제했다고 기존 approval로 나머지를 실행하지 않는다.

### 26.73 Export 시 Filter와 Selection

Export 자체는 read-only이므로 Vehicle Browser selection/filter 결과를 사용해 범위를 만들 수 있다.

Manifest에는 실제 exported row identity set을 기록한다.

Search/filter 조건 문자열 자체를 Authoring SSOT로 저장하지 않는다.

### 26.74 Export File 이름은 identity가 아니다

CSV 파일명이나 folder name은 Batch target identity가 아니다.

Identity는 Manifest + row metadata의 Unreal asset identity를 사용한다.

파일을 rename해도 adjacent manifest association을 명시적으로 유지하면 읽을 수 있지만, 잘못된 Manifest와 pair되면 import를 Block한다.

### 26.75 Asset Rename / Move 이후 Import

Export 이후 Recipe/Profile/Definition Asset path가 rename/move되면 P0 import가 예전 path를 자동 추측해서 찾지 않는다.

```text
TargetNotFound / BatchIdentityMoved
```

로 새 Export를 요구하는 방향을 기본으로 한다.

rename-independent Authoring/Runtime ID를 P0-07 때문에 새로 도입하지 않는다.

### 26.76 Validation Layer 보존

Batch에서도 Section 22/24 validation layer를 섞지 않는다.

```text
Batch Schema / Parse
Authoring Input / Recipe / Profile
Resolver
Definition
Sync / Drift
Runtime Technical [후속]
USER Acceptance [별도]
```

Batch Preview PASS를 Driving Feel PASS로 해석하지 않는다.

### 26.77 Batch numeric range 검증

Importer가 임의 balance threshold를 만들지 않는다.

Range/constraint는:

```text
기존 semantic contract
UPROPERTY / typed schema metadata
Profile / Recipe validation
Resolver validation
```

에서 가져온다.

근거가 없는 min/max를 Batch용으로 새로 만들지 않는다.

### 26.78 Unit 처리

Unit은 column metadata다.

```text
kg
rpm
degree
seconds
normalized 0..1
```

같은 unit label은 manifest/UI에서 설명한다.

numeric cell에:

```text
"1570 kg"
"35 deg"
```

처럼 unit 문자열을 넣지 않는다.

정확한 값 범위는 기존 contract/schema가 Authority다.

### 26.79 Precision / Round Trip

Export는 해당 numeric type을 lossless하게 round-trip할 수 있는 canonical precision을 사용한다.

Import는 Field Codec / typed parser로 실제 property type에 변환하고 readback canonical value를 다시 비교한다.

Spreadsheet display rounding을 실제 authoring value로 조용히 적용하지 않는다.

### 26.80 CSV row deletion 의미

Spreadsheet에서 row를 삭제해도:

```text
Delete Vehicle
Delete Recipe
Unbind Profile
```

을 의미하지 않는다.

Manifest row가 CSV에서 빠지면:

```text
Excluded / No Change
```

로 처리할 수 있다.

단 Import Preview에서 `Rows Missing From Export`로 count를 보여 사용자가 범위가 줄었음을 알게 한다.

Asset deletion operation은 P0 Batch에 없다.

### 26.81 새 row 추가 의미

사용자가 CSV에 임의 새 row를 추가해 새 Vehicle/Profile을 생성하지 않는다.

Manifest에 없는 row는:

```text
UnknownBatchRow
```

로 Block한다.

New Vehicle 생성은 Section 24/25 reviewed operation을 사용한다.

### 26.82 Column 추가 / 삭제

Unknown column:

```text
BatchColumnUnknown
```

로 처리한다.

Editable known column을 파일에서 삭제하는 것은 해당 field의 `No Change`로 볼 수 있지만 Preview에서 Missing Editable Column을 표시할 수 있다.

Reserved identity/schema column 누락은 file Block이다.

### 26.83 Batch Report Export

Preview/Apply 결과는 다시 read-only CSV report로 export할 수 있다.

대표:

```text
BatchPreviewReport
BatchApplyResultReport
```

이 report는 audit/convenience용이며 다시 Import하지 않는다.

### 26.84 Batch Audit Metadata

Operation result에는:

```text
BatchOperationId
CallerKind
DatasetKind
BatchPlanHash
Included Row Id set hash
Before/After object fingerprints
Mutation footprint
Per-target Apply result
```

를 diagnostics로 남길 수 있다.

이것은 Recipe/Profile Source Trace를 대체하지 않는다.

### 26.85 AI Batch Orchestration의 위치

AI는 P0-07부터 Batch Service의 client가 될 수 있다.

```text
Excel/CSV UI ─┐
AI Batch      ─┼→ FCFVehicleBatchService
Automation    ─┘
```

AI가 Section 25 single-Vehicle write operation을 자체 `for each` loop로 반복하여 hidden batch를 만들지 않는다.

### 26.86 AI는 CSV 자체를 SSOT처럼 생성하지 않는다

AI Batch의 권장 입력은 CSV text 조작보다 **같은 canonical Batch Table model**이다.

```text
ExportBatchDataset
→ FCFBatchTable
→ AI proposes typed cell edits
→ same PreviewBatchImport path
```

CSV는 사람이 Excel로 편집하기 위한 serialization이다.

AI가 manifest 없이 Vehicle 이름/field 이름만 추측해 새 spreadsheet를 만들어 바로 commit하지 않는다.

### 26.87 AI Batch Approval

AI도 B1/B2/B3 approval을 동일하게 사용한다.

AI가 사용자에게 보여줄 최소:

B1/B2:

```text
Dataset Kind
Changed source objects
Changed cells
Affected Vehicle count
Conflicts / Blockers / Warnings
Target VehicleData mutation = No
Auto Save = No
```

B3:

```text
Target Vehicle count
Total field diff count
Warnings
External Drift excluded count
Per-target Apply evidence summary
Target mutation = Yes
Global atomic = No
Stop On First Failure
Auto Save = No
```

### 26.88 AI Conflict 자동 해소 금지

AI는:

```text
ConcurrentEditConflict
OwnershipChangedSinceExport
External Drift
Unknown Row / Column
```

을 임의로 skip/overwrite하지 않는다.

사용자가 conflict row를 제외하라고 승인하면 새 Batch Plan을 만든다.

### 26.89 AI Profile Batch 변경

Section 25에서 금지한 arbitrary Shared Profile writer는 계속 없다.

AI가 Shared Profile numeric 값을 바꾸려면:

```text
ProfileNumericEdit Batch model
→ affected Vehicle preview
→ B2 approval
→ FCFVehicleBatchService
```

를 사용한다.

따라서 AI와 Excel 모두 같은 cross-vehicle impact gate를 통과한다.

### 26.90 AI Batch Apply

AI는 B3에서:

```text
FCFBatchDefinitionApplyPlan
```

을 사용한다.

각 target mutation은 여전히:

```text
FCFVehicleApplyService
```

다.

AI adapter에 `ApplyAllVehiclesRaw` 같은 별도 mutation primitive를 만들지 않는다.

### 26.91 Implementation Service 후보

P0-08 이후 후보:

```text
FCFVehicleBatchService
FCFBatchColumnRegistry
FCFBatchTable
FCFBatchManifest
FCFBatchImportSession
FCFBatchAuthoringPlan
FCFBatchDefinitionApplyPlan
FCFBatchOpResult
```

구현 파일 후보:

```text
Public/DataAuthoring/CFVehicleBatchService.h
Public/DataAuthoring/CFVehicleBatchTypes.h
Private/DataAuthoring/CFVehicleBatchService.cpp
Private/DataAuthoring/CFBatchCSV.cpp
```

native XLSX file layer는 P0 구현 필수에 넣지 않는다.

### 26.92 C++ / Slate / AI 역할

#### C++

```text
Column schema
CSV parse / serialize
Manifest
3-way merge
Current-state fingerprint checks
Prospective batch resolve
Conflict classification
Authoring commit transaction
Batch Apply orchestration
Typed results
```

#### Slate

```text
Export / Import UX
Preview summary
Row/conflict table
Affected Vehicle drill-down
Approval actions
Result presentation
```

#### AI

```text
Batch dataset read
typed numeric proposal
review summary
same Batch Service invocation
```

Blueprint에 별도 CSV mutation path를 만들지 않는다.

### 26.93 P0-08 Automation 입력 — Batch

최소 추가 후보:

```text
Batch_Export_NoMutation
Batch_ResolvedReport_NotImportable
Batch_RecipeWritableColumns_Allowlist
Batch_ProfileWritableColumns_Allowlist
Batch_ReservedColumnModifiedBlocked
Batch_DuplicateRowBlocked
Batch_UnknownRowBlocked
Batch_BlankMeansNoChange
Batch_ThreeWay_UnrelatedChangeSafe
Batch_ThreeWay_ConflictBlocked
Batch_OwnershipChangedBlocked
Batch_RecipePreview_NoMutation
Batch_ProfilePreview_NoMutation
Batch_ProfilePreview_AllDependentsClassified
Batch_B1_AllOrNothing
Batch_B2_AllOrNothing
Batch_SourceCommit_NoTargetMutation
Batch_SourceCommit_NoAutoSave
Batch_ApplyPlan_FreshAfterCommit
Batch_Apply_GlobalPreflightNoMutationOnMismatch
Batch_Apply_UsesPerVehicleApplyService
Batch_Apply_StopOnFirstFailure
Batch_Apply_PartialResultExact
Batch_Apply_NoAutoRollbackPriorSuccess
Batch_Apply_NoAutoSave
Batch_OperationId_NoDoubleMutation
Batch_AI_NoHiddenSingleVehicleLoop
Batch_Profile_AI_UsesBatchGate
Batch_Drift_BulkResolutionNotExposed
Batch_CSV_FormulaRejected
```

P0-08에서 기존 Core/AI automation과 함께 실제 test file 구조를 결정한다.

### 26.94 P0-11 Technical Validation에 넘길 Acceptance

```text
Spreadsheet import가 Unreal SSOT를 대체하지 않음
Read-only report import 불가
Writable column allowlist 밖 write 차단
3-way merge가 concurrent Unreal edit를 덮지 않음
B1/B2 source commit all-or-nothing
B1/B2 Target mutation 0
Source commit 후 fresh Apply Plan 필수
B3 global preflight mismatch 시 mutation 0
B3 per-Vehicle ApplyService 사용
mid-batch failure 시 prior success / failed / not-started 정확
automatic retry 0
automatic global rollback 0
automatic save 0
External Drift bulk resolution 0
AI와 Slate Batch Preview / Apply plan parity
```

### 26.95 USER Acceptance에 남기는 것

P0-07 설계 완료로 다음을 추정하지 않는다.

```text
Excel 편집이 실제로 충분히 편하다
CSV 저장 workflow가 번거롭지 않다
wide Recipe table column 구성이 적절하다
Profile numeric table이 이해하기 쉽다
Batch conflict UX가 과도하게 복잡하지 않다
대량 balance 작업이 실제로 빨라진다
```

이는 P0-12에서 실제 사용으로 확인한다.
Native XLSX 필요성도 이 근거로 판단할 수 있다.

### 26.96 P0-07 완료 판정

Roadmap 완료 조건:

```text
Excel과 Unreal 중 SSOT가 중복되지 않음
→ Unreal Recipe/Profile이 Authoring SSOT, CSV는 staging snapshot

Import가 Preview 없이 직접 대량 Mutation하지 않음
→ 3-way Preview → Authoring Commit → Fresh Resolve → separate Batch Apply Plan

Vehicle P0에서 실제 필요한 Numeric Batch 범위가 근거로 확인됨
→ Recipe semantic numeric + Shared Profile numeric만 writable
→ Raw Resolved / Asset / Layout / Stable-ID / Complex Struct는 read-only
```

추가로:

```text
CSV + manifest를 P0 canonical interchange로 확정
Excel은 CSV editor로 지원, native XLSX는 P0 필수에서 제외
4 Dataset Kind 확정
Column Registry / stable ColumnId / metadata contract 확정
3-way merge / ownership conflict contract 확정
B1 Recipe source atomic commit 확정
B2 Shared Profile source atomic commit + affected Vehicle gate 확정
B3 per-Vehicle atomic / batch non-atomic Apply 확정
Global preflight + stop-on-first-failure + exact partial result 확정
External Drift bulk resolution 금지
Batch approval / idempotency / no blind retry 확정
AI Batch가 동일 FCFVehicleBatchService를 사용하도록 확정
P0-08 Batch automation 30개 후보 확정
```

따라서 **DAUTH-P0-07 Batch / Excel / CSV Contract는 Complete / P0-08 Ready**로 판정한다.

---

## 27. Changelog

### v0.8.1 - 2026-08-17

- `CF-FQ-038`의 DAUTH-P0-08 실제 구현 착수에 맞춰 문서 lifecycle을 `Active / Implementation Contract`로 전환했다.
- Section 21~26의 Frozen Contract / Resolver / Migration / UX / AI / Batch 의미는 변경하지 않았다.
- P0-08A Editor-only Recipe + 5 Profile / Never-Cook / common types와 P0-08B Stable Field Path / Field Value Codec / 117 Registry coverage가 official Build 및 targeted Automation으로 Technical PASS임을 implementation checkpoint로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting 소비 경로, `SCFVDAWizardTab`, Content Asset 변경은 0이다.
- 다음 구현 slice는 `DAUTH-P0-08C Immutable Snapshot Foundation`이다.

### v0.8.0 - 2026-08-17

- DAUTH-P0-07 Batch / Excel / CSV Contract를 완료.
- Unreal Recipe/Profile을 Authoring SSOT로 유지하고 Spreadsheet를 Export Snapshot + Edit Staging으로 고정.
- P0 canonical interchange를 UTF-8 CSV + `.cfbatch.json` Manifest로 확정하고 Excel은 CSV 편집기로 지원하며 native XLSX parser/writer는 P0 필수에서 제외.
- VehicleSummaryReport / ResolvedFieldReport read-only, RecipeNumericEdit / ProfileNumericEdit editable의 4 Dataset Kind를 확정.
- Resolved VehicleData raw leaf, Asset Reference, Stable ID/Array, Complex Struct, Asset Derived/Measurement, Advanced Override를 Spreadsheet write 범위에서 제외.
- Recipe semantic scalar numeric과 Shared Profile typed scalar numeric만 Batch Column Registry allowlist를 통해 external edit 가능하도록 고정.
- Shared Profile numeric Batch write를 P0-07 reviewed Batch gate로 열되 Section 25 arbitrary single-profile AI writer는 계속 금지.
- stable ColumnId, reserved metadata, Manifest baseline, canonical numeric/blank/formula/read-only cell 규칙을 확정.
- Export Baseline / Edited Value / Current Unreal의 3-way merge와 ownership-changed conflict를 고정.
- `FCFBatchImportSession`, prospective Recipe/Profile Batch Preview, affected Vehicle resolve/validation을 설계.
- Spreadsheet Import와 Definition Apply를 분리하고 Authoring Source Commit 후 fresh Resolve + 별도 Batch Definition Apply Plan을 필수화.
- B1 Recipe Numeric Commit과 B2 Profile Numeric Commit은 source object에 대해 all-or-nothing transaction, Target mutation 0으로 고정.
- B3는 global preflight 후 canonical target order로 기존 `FCFVehicleApplyService`를 per-Vehicle 호출하며 전체 Batch global atomic은 보장하지 않는 것으로 확정.
- mid-batch failure는 Stop On First Failure, prior success 자동 rollback 없음, Failed/Applied/NotStarted exact result, 새 Plan 없이 blind retry 금지로 고정.
- External Drift bulk resolution, Auto Save, global blind Batch Undo를 금지.
- Batch approval/idempotency/typed error/result와 AI Batch orchestration을 고정하고 hidden single-Vehicle loop를 금지.
- P0-08 Batch automation 추가 검증 30개 후보를 정의.
- DAUTH-P0-07 Complete / P0-08 Ready를 판정.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.7.0 - 2026-08-17

- DAUTH-P0-06 AI Authoring Contract를 완료.
- AI를 별도 Authoring owner가 아닌 공통 C++ Authoring Service의 typed client로 고정하고 UI/AI가 Snapshot/Resolver/Diff/Validation/Apply를 공유하도록 확정.
- `FCFVehicleAuthoringService` facade와 AI contract type/file 후보를 정의하되 Section 22 Core 책임을 복제하지 않도록 경계를 고정.
- R0 Read Only / R1 Authoring Record Write / R2 Ownership·Exceptional Write / R3 Definition Apply risk class를 확정.
- exact payload/state hash에 binding되는 transient approval과 Preview 이후 R3 Apply approval 원칙을 정의.
- Read-before-write, Expected Recipe/Target/Resolver precondition, typed result/error/mutation footprint를 구체화.
- desired-state mutation, `ClientOperationId` transient dedupe, no blind write retry, Apply idempotency를 확정.
- List/Read/Resolve/Diff/Trace/Validation/Compare/Drift read operation과 prospective `PreviewRecipeChange`를 설계.
- Profile Binding, Driving Feel, Asset Intent, Mass/Durability/Default/DriveState, Hardpoint/Mount semantic typed write operation을 정의하고 generic Raw `SetField(path,value)` API를 금지.
- New Vehicle record creation, Existing Definition Initial Import, Group/Field Adoption, Measurement decision, Advanced Override, External Drift decision typed contract를 구체화.
- `ApplyResolvedVehicle`는 AI writer가 아니라 `FCFVehicleApplyService`의 동일 Apply lane을 호출하도록 고정하고 force/skip-validation/no-auto-save를 금지.
- AI shared Profile payload write를 P0-06에서 제외하고 existing Profile read/bind까지만 허용.
- AI Raw VehicleData/Recipe internal metadata/derived flag write, auto measurement acceptance, auto Legacy unpin, auto Drift absorption, USER Acceptance 추정을 명시적으로 금지.
- AI global blind Undo를 P0에서 노출하지 않고 모든 mutation의 transaction undoability와 ApplyService atomic rollback을 유지.
- AI-only Source Type / Recipe field를 금지하고 caller metadata를 precedence와 분리.
- P0-06 write scope를 single Vehicle로 제한해 P0-07 Batch Gate 우회를 금지.
- P0-08 automation에 AI Contract 추가 검증 20개 후보를 고정.
- DAUTH-P0-06 Complete / DAUTH-P0-07 Ready를 판정.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.6.0 - 2026-08-17

- DAUTH-P0-05 Vehicle Authoring UX Design을 완료.
- P0 Production UI를 `CarFight_ReEditor` C++ Slate Workspace로 확정하고 Generic Raw Details 중심 UX를 배제.
- Vehicle Browser + Main Authoring View + persistent Context Pane + Bottom Apply Bar의 3-pane 구조를 확정.
- Management / Sync / Validation을 독립 상태 축으로 분리하고 Unmanaged / Legacy Imported / Partially Managed / Managed, In Sync / Stale / Shadow Changed / External Drift / Preview Out Of Date 표현을 구체화.
- Vehicle Browser의 managed/unmanaged/mesh-candidate row, search/filter/sort와 read-only multi-select compare 범위를 확정.
- New Vehicle record creation, Mesh Candidate → Vehicle creation, Existing Definition Initial Import와 Legacy Pin summary workflow를 상세화.
- Recipe/Profile binding, Shared Profile editor 경계, group/field Adoption, semantic input control UX를 확정.
- Assets/Layout의 typed asset picker, Wheel Socket/Derived Anchor read-only 표현, Wheel measurement proposal/adoption/stale measurement UX를 확정.
- Hardpoint stable ID rename/remove dependency guard, Driving Feel 4축/preset/shadowed pin UX, Mount/Default editor 구조를 확정.
- Pending Authoring Diff와 Reference Vehicle Compare를 두 mode로 분리하고 117-leaf Registry 기반 table contract를 고정.
- 오른쪽 Context Pane의 Changes / Source Trace / Issues / Sync 계약과 source stack/navigation 행동을 확정.
- Error/Blocked/Warning/Info, inline + persistent validation, `Go To` navigation, External Drift 3-way review UX를 구체화.
- Recipe/Profile edit, Preview freshness, Definition Apply, Package Save 의미를 분리하고 no-auto-save UI를 확정.
- Apply 활성 조건, warning apply, destructive diff marker, success/failure/rollback/Undo UX를 고정.
- Advanced Override typed editor + Reason required, Legacy Pin Inspector, managed Raw DA escape-hatch warning과 External Drift recovery 흐름을 확정.
- P0-09 MVP UI 범위와 P0-10 Wizard parity expansion을 분리.
- DAUTH-P0-05 Complete / DAUTH-P0-06 Ready를 판정.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.5.0 - 2026-08-17

- DAUTH-P0-04 VDA Wizard Migration Design을 완료.
- Current `SCFVDAWizardTab`, `UCFVDAValidator`, `UCFVehicleData::CaptureLayoutFromChassisSockets()` 실제 Source를 기준으로 기능별 Migration 판정을 확정.
- Target/Source selection은 Refactor and Reuse, Reference Compare는 117-leaf Registry 기반 기능으로 Replace, `UCFVDAValidator`는 Definition Validation에 Reuse As-Is로 판정.
- 새 Workspace에서 `ValidateVehicleData(Candidate, nullptr)`를 사용해 Definition Validation과 Reference Compare를 분리하도록 고정.
- Layout Capture의 socket extraction/partial-warning 의미는 보존하되 direct Target mutation shell은 Asset Snapshot → Resolver → Diff → Apply Service로 이전하도록 확정.
- Driving Feel 4축과 Sedan/SUV/Sports/Heavy semantic shortcut은 보존하되 hard-coded raw mapping과 authoritative reverse inference는 새 Workspace에서 제거하도록 확정.
- Quick Tune Preview/Apply는 `FCFVehicleResolveResult` / `FCFVehicleApplyService`로 Replace하고 `FScopedTransaction` primitive는 중앙 service에서 재사용하도록 확정.
- load-time Quick Tune Revert는 Unreal Undo / Recipe transaction / Rebase 흐름으로 대체 후 retire하도록 확정.
- Raw DA Open과 Clipboard report convenience는 유지하고 managed raw edit는 External Drift escape hatch로 처리.
- P0-08~10 coexistence plan, managed/unmanaged split, Current Wizard Freeze, Deprecated Gate DG1~DG5, Delete Gate DEL1~DEL7을 고정.
- DAUTH-P0-04 Complete / DAUTH-P0-05 Ready를 판정.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.4.0 - 2026-08-17

- DAUTH-P0-03 Vehicle Recipe / Profile / Resolver 상세 설계를 완료.
- 기존 `CarFight_ReEditor` 재사용, Runtime Authoring 의존성 0, Editor-only `UDataAsset` Recipe/Profile 구조를 확정.
- `UCFVehicleRecipeData`의 Identity / Target / Asset Intent / Profile Binding / 4축 Feel / Mass / Hardpoint / Mount / Default Data / WheelVisual / DriveState / Override / Import / AppliedState 구조를 고정.
- Vehicle Base / Drivetrain / Handling / Performance / DriveState 5 flat Profile의 실제 field 책임과 Feel Response 구조를 구체화.
- `FCFFeelResponse`, opt-in `FCFMassScaleRule`을 정의해 hard-coded Quick Tune Lerp를 Profile-authored deterministic rule로 이전하는 설계를 확정.
- immutable Profile/Asset Snapshot과 resolver-relevant asset fingerprint 방식을 확정.
- 구조화된 Stable Field Path, Reflection canonical Field Value Codec, 117 leaf Field Registry와 coverage self-test를 설계.
- 117 leaf 전체의 Field Resolver Map을 Resolver Rule / dependency / Adoption Group / Advanced Override 허용 기준으로 작성.
- Wheel Radius/Width measurement proposal → explicit adoption 계약을 구체화.
- Legacy Pin / lossless semantic candidate / 금지 inverse inference / group adoption / rebase 계약을 구체화.
- Resolver Request/Result와 R0~R16 fixed stage order, Derived flag rule, Source Trace stack을 설계.
- Effective Stale / Shadow Source Changed / External Drift를 분리하고 field-level stale report를 정의.
- Stable-ID array Field Diff와 dependency-safe apply ordering을 정의.
- TOCTOU precondition을 가진 `FCFVehicleApplyService`의 atomic transaction / rollback / no-auto-save 계약을 구체화.
- ResolverContractRevision과 P0-08 automation 20개 입력을 고정.
- DAUTH-P0-03 Complete / DAUTH-P0-04 Ready를 판정.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.3.0 - 2026-08-17

- DAUTH-P0-02 Authoring Contract Freeze를 완료.
- Authoring Intent Truth(Recipe/Profile/Source Tracking)와 Runtime Canonical Definition(`UCFVehicleData`)의 SSOT 경계를 동결.
- Vehicle Recipe를 persistent Authoring Record로 확정하고 Raw VehicleData 복제 금지를 고정.
- P0 Profile을 Vehicle Base / Drivetrain / Handling / Performance / DriveState 5 Domain으로 고정하고 Profile inheritance와 implicit last-write-wins를 금지.
- Resolver input/output, field source precedence, conflict policy와 optional Fitting preview-context 비소유 계약을 확정.
- Source Tracking을 Recipe-side persistent metadata로 확정하고 Stable Field Path / Source Revision / Applied Hash 계약을 정의.
- Semantic Intent와 Advanced Leaf Override를 분리하고 Stable-ID array override를 확정.
- Layout / Movement / WheelVisual / DriveState flag의 서로 다른 Authoring 표현을 고정.
- P0 Asset Derived 자동 범위를 Layout/Hardpoint Socket Transform 중심으로 제한하고 wheel radius/width는 measurement-assisted preview로 분리.
- Existing Definition을 Legacy Pinned Baseline으로 import하고 field/group Adopt를 통해 점진적으로 managed 상태로 전환하는 계약을 확정.
- Stale / Regenerate / External Drift, Definition Identity, Data Browser cache, Apply Transaction, Validation ownership 계약을 확정.
- Section 4.14의 12개 Architecture Freeze 입력을 모두 판정하고 DAUTH-P0-02 PASS / P0-03 Ready를 기록.
- Source / Content Asset / Runtime / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.2.0 - 2026-08-17

- DAUTH-P0-01 Vehicle Field Ownership Matrix를 Current Source 기준으로 완료.
- `UCFVehicleData` top-level aggregate holder 9개와 실제 leaf field 117개, 총 126행을 전수 분류.
- 각 Field Path에 Current Owner / Authoring Classification / Input Source / Default Strategy / Definition Storage / Fitting Impact / Runtime Consumer / Validation Owner / USER Verification Need / Migration Note를 기록.
- `VehicleMovementConfig` 47개 leaf field를 Quick Tune 소유 범위와 분리하고 Asset Derived / Drivetrain / Suspension / Aero / Advanced 기술 필드로 재분류.
- `FCFVehicleMountProfile`의 Editor 비노출 legacy serialized field 10개를 누락 없이 기록하고 신규 Authoring 입력 제외 대상으로 판정.
- `LocationCategory`, `bExposedModule`처럼 현재 production consumer가 없는 저장 필드를 명시적으로 보존.
- 세 Override flag의 서로 다른 Current Runtime 의미를 Matrix에 고정.
- P0-02 Authoring Contract Freeze에 넘길 12개 Architecture 결정 입력을 정리.
- Source / Content Asset / Plan Index / FeatureQueue / ActiveWork는 변경하지 않음.

### v0.1.0 - 2026-08-17

- Data Authoring 상세 설계 초안 작성.
- Authoring Field Classification 9종을 정의.
- VehicleData Field Ownership Matrix 작성 규칙을 정의.
- Recipe / Profile / Resolver / Source Tracking / Stale / Override 설계 원칙을 기록.
- 기존 Vehicle DA Wizard Migration Matrix를 작성.
- Inventory / Fitting / AI / Data Browser / Excel 연계의 책임 경계를 명시.
- P0-02 Architecture Freeze에서 확정해야 할 미결 항목을 분리.

---

## 28. Migration

- v0.8.1은 P0-08 implementation lifecycle 전환만 기록하며 Section 21~26의 설계 의미와 precedence를 변경하지 않는다. P0-08A/B PASS를 보존하고 다음 구현은 P0-08C Snapshot Foundation에서 시작한다.
- v0.8.0부터 Section 26은 DAUTH-P0-07 Batch / Excel / CSV Contract Authority다.
- P0 Batch의 canonical external interchange는 UTF-8 CSV + Batch Manifest이며 native XLSX support는 P0 구현 필수가 아니다.
- Spreadsheet는 Authoring Source/SSOT가 아니며 Runtime에서 읽지 않는다.
- ResolvedFieldReport와 VehicleSummaryReport는 read-only이고 다시 Import하지 않는다.
- external writable 범위는 Batch Column Registry가 허용한 Recipe semantic scalar numeric과 Shared Profile typed scalar numeric으로 제한한다.
- Shared Profile numeric write는 P0-07 Batch gate에서만 허용하며 Section 25 arbitrary single-profile AI writer 금지는 유지한다.
- B1/B2 Authoring Source Commit은 Target Definition을 쓰지 않고 source object에 대해 all-or-nothing transaction을 사용한다.
- Source Commit 후 fresh Resolve + 별도 `FCFBatchDefinitionApplyPlan` 없이 Target Definition을 Batch Apply하지 않는다.
- B3는 single-Vehicle `FCFVehicleApplyService`를 그대로 사용하며 per-Vehicle atomic / batch non-atomic, global preflight, stop-on-first-failure를 적용한다.
- External Drift bulk resolution, automatic retry, automatic prior-success rollback, Auto Save, global blind Batch Undo를 만들지 않는다.
- AI Batch는 `FCFVehicleBatchService`를 사용하며 Section 25 single-Vehicle write를 hidden loop로 반복하지 않는다.
- v0.7.0부터 Section 25는 DAUTH-P0-06 AI Authoring Contract Authority다.
- P0 AI는 `CarFight_ReEditor` 공통 Authoring Service의 client이며 Runtime module에 AI dependency를 추가하지 않는다.
- AI-facing transport는 typed operation adapter만 제공하고 Resolver/validation/mutation rule을 복제하지 않는다.
- AI write는 single Vehicle scope, desired-state, expected fingerprint, no automatic retry를 사용한다. Multi-Vehicle write orchestration은 P0-07 이전에 추가하지 않는다.
- AI Definition Apply는 Section 22 `FCFVehicleApplyService` 이외의 writer를 만들지 않으며 Preview/approval hash가 stale하면 mutation 없이 Block한다.
- AI는 Shared Profile payload를 P0-06에서 수정하지 않고 existing Profile read/bind만 수행한다.
- AI Raw VehicleData/Recipe internal metadata/derived output write API와 force/skip-validation/auto-save/global blind undo API를 만들지 않는다.
- AI-specific Source Type이나 Recipe provenance field를 추가하지 않으며 caller identity는 optional audit metadata로만 취급한다.
- v0.6.0부터 Section 24는 DAUTH-P0-05 Vehicle Authoring UX Authority다.
- P0 Production Authoring UI는 `CarFight_ReEditor` C++ Slate를 기본 경로로 사용하고 Generic Raw `IDetailsView`는 메인 Vehicle Authoring UI로 사용하지 않는다.
- P0-09는 Section 24.97의 MVP UI를 우선 구현하고, P0-10에서 Layout/Driving Feel/Reference Compare 등 full Wizard parity를 확장한다.
- UI ViewModel은 transient presentation state이며 Recipe/Definition/Source Tracking의 새 SSOT가 아니다.
- Recipe/Profile edit는 authoring asset transaction이며 Definition Apply와 별개다. Apply는 package save가 아니며 automatic Save를 수행하지 않는다.
- Existing Definition Initial Import, group/field Adoption, Measurement accept, Drift Rebase는 Target mutation과 분리하며 실제 Target write는 Apply Service만 수행한다.
- managed Vehicle Raw DA editor는 Advanced escape hatch로 유지하고 out-of-band mutation은 External Drift로 처리한다.
- Multi-select는 P0-07 전까지 Reference Compare 같은 read-only workflow에만 사용하며 Batch Apply를 추가하지 않는다.
- v0.5.0부터 Section 23은 DAUTH-P0-04 VDA Wizard Migration Authority다.
- 신규 Authoring Workspace는 Current `SCFVDAWizardTab`의 직접 DA mutation 함수를 호출하지 않는다.
- `UCFVDAValidator`는 계속 Definition validator로 재사용하되 Reference Compare는 새 Registry-based compare로 분리한다.
- P0-10 parity 전에는 `SCFVDAWizardTab`과 `UCFVehicleData::CaptureLayoutFromChassisSockets()`를 삭제하지 않는다.
- P0-04 이후 Current Wizard에는 crash/data-loss/schema compatibility correction 외 신규 기능을 추가하지 않는다.
- managed Target의 정상 write path는 P0-10 이후 Data Authoring Workspace → Resolver/Diff/Apply Service 하나로 수렴시킨다.
- Wizard UI 물리 삭제는 DG1~DG5, DEL1~DEL7을 모두 충족한 뒤에만 허용한다.
- v0.4.0부터 Section 22는 DAUTH-P0-03의 구현 상세 Authority이며 Section 21 Frozen Contract를 변경하지 않고 구체화한다.
- P0-08 구현 시 Recipe/Profile/Resolver 핵심 타입은 `CarFight_ReEditor`에 두며 `CarFight_Re` Runtime module에 Authoring dependency를 추가하지 않는다.
- Recipe/Profile은 Editor-only `UDataAsset`을 사용하고 Primary Asset / AssetManager 계약을 새로 만들지 않는다.
- 첫 실제 Authoring Content Asset 생성 전에 explicit Never-Cook 보호와 packaging regression을 P0-08에서 검증한다.
- P0-08 Field Registry는 current `UCFVehicleData` 117 leaf와 양방향 coverage test를 가져야 하며 source field가 추가/삭제되면 조용히 통과하면 안 된다.
- Current Quick Tune raw inverse는 기존 차량 Recipe intent migration에 사용하지 않으며 P0-04에서 기능 migration만 설계한다.
- Existing Definition import는 Legacy Pin을 유지하고 Adopt 자체는 Recipe만 수정하며 Target `UCFVehicleData` mutation은 별도 Apply에서만 수행한다.
- Source/Content Asset은 P0-03에서 변경하지 않았으며 실제 class/file/content 생성은 P0-08 이후다.
- v0.3.0부터 Section 21은 DAUTH-P0-02의 Frozen Authoring Contract이며 P0-03~07 상세 설계는 이 경계를 변경하지 않고 구체화한다.
- P0-02 PASS는 신규 Recipe/Profile/Resolver Source 구현 승인이 아니다. 구현은 Roadmap의 P0-08 Gate 이후다.
- 기존 `UCFVehicleData`는 Runtime Canonical Definition으로 유지하며 Authoring provenance를 넣기 위한 Runtime field를 P0에서 추가하지 않는다.
- Existing Definition은 자동 변환하지 않고 명시적 Import 시 Legacy Pinned Baseline으로 보존한다.
- Profile/Asset/Resolver 변경은 Definition을 자동 갱신하지 않고 Stale → Preview/Diff → Explicit Apply를 사용한다.
- v0.2.0부터 Section 4의 126행 Matrix는 DAUTH-P0-01 Current Ownership 결과이며, v0.1.0의 예시 분류 표를 대체한다.
- Section 4 Matrix는 Current field responsibility의 Authority이고, Section 21 Frozen Contract가 그 Matrix를 P0 Authoring 구조로 해석하는 Architecture 계약이다.
- 현재 VehicleData 값을 Recipe로 자동 역산하여 덮어쓰지 않는다.
- Existing Definition의 확인되지 않은 Source는 명시적 Import에서 `Legacy Imported Pinned Baseline`으로 보존한다.
- `FCFVehicleMountProfile` legacy serialized field는 새 Authoring 입력에서 제외하되 P0-01에서 Source나 Asset을 삭제/변환하지 않는다.
- 기존 Vehicle DA Wizard는 Migration 완료 전 유지한다.
- 기존 `UCFVDAValidator`를 대체하는 병렬 검증기를 선제 생성하지 않는다.
- Plan Index / FeatureQueue / ActiveWork에는 아직 Data Authoring을 정식 등록하지 않는다.
