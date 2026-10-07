# RuntimeApply

- 문서 버전: v1.2.0
- 최근 갱신일: 2026-10-07
- 문서 상태: Current
- 기능 소유: `CF-FQ-041 런타임 콘텐츠 적용 메뉴`
- 대표 구현: `FCFRuntimeVehicleApplyService`, `FCFRuntimeEquipApplyService`, `CFRuntimeApplyWidget`
- 관련 Current System: `VehicleData.md`, `VehicleRuntime.md`, `VehicleInventory.md`, `../Combat/WeaponData.md`

---

## 1. 문서 목적

이 문서는 현재 CarFight의 RuntimeApply가 실제로 어떤 역할을 하고, 어떤 책임을 갖지 않으며, 이후 Garage / Inventory / Field Fitting / Mission Loadout 같은 정식 기능이 어떤 부분을 재사용해야 하는지를 기록한다.

RuntimeApply는 현재 VehicleDebug Panel 안에서 개발·시연용 UI로 노출되지만, **Runtime 적용 Backend 자체는 Debug UI 전용 코드로 취급하지 않는다.**

현재 구조의 핵심 의미는 다음과 같다.

```text
현재 RuntimeApply UI / 향후 Garage / Inventory / Loadout
                ↓
      선택 후보와 소유권 정책 결정
                ↓
       Runtime Apply Application Seam
                ↓
    Fitting Snapshot 전체 상태 구성·검증
                ↓
       Prepare / Commit / Rollback
                ↓
Weapon / Ammo / Launcher / Defense / Sensor / Mass Runtime
```

즉 Frontend와 후보 공급 방식은 바뀔 수 있지만, 이미 존재하는 Vehicle/Fitting Runtime authority를 다시 구현해서는 안 된다.

---

## 2. 현재 책임 분리

### 2.1 현재 RuntimeApply Frontend

현재 `CFRuntimeApplyWidget`은 다음 역할을 맡는다.

```text
- Vehicle 후보는 Runtime Test Catalog에서 표시
- Equipment 후보는 valid Production Publication Catalog를 우선 사용하고, Production Catalog가 없거나 invalid할 때만 Runtime Test Catalog를 fallback으로 사용
- Mount 선택
- 현재 선택과 적용 대상 표시
- 명시적 Apply 요청
- 호환성/결과 메시지 표시
```

현재 Catalog는 개발·시연 후보 목록이며 소유권 시스템이 아니다.

따라서 현재 Frontend는 다음을 수행하지 않는다.

```text
- ItemInstance 소유권 확인
- Inventory Container 변경
- Reservation 생성/해제
- Atomic Transfer
- 장비 구매/판매
- 영구 Loadout 저장
```

이 책임은 `VehicleInventory.md`의 Inventory/Foundation 계약 또는 향후 해당 Product Feature가 소유한다.

### 2.2 Runtime Apply Backend

`FCFRuntimeVehicleApplyService`와 `FCFRuntimeEquipApplyService`는 선택 UI와 분리된 Runtime application seam이다.

Frontend가 어떤 방식으로 후보를 얻었는지와 무관하게 최종 Runtime 적용은 다음 기존 authority를 재사용한다.

```text
VehicleData
→ VehicleFittingData / Fitting Snapshot
→ VehicleFittingComp Prepare / Commit
→ Vehicle runtime reinitialize 또는 dependent runtime refresh
→ 실패 시 recovery
→ final readback
```

향후 Garage/Inventory가 생겨도 이 적용 흐름과 검증을 별도로 복제하지 않는 것을 기본 원칙으로 한다.

---

## 3. Vehicle Runtime Apply 계약

Vehicle 적용은 현재 Pawn을 교체하는 방식이 아니다.

```text
현재 Player VehiclePawn 유지
→ 선택 VehicleData의 transient runtime source 구성
→ 기존 VehicleFittingData source 정리
→ InitializeVehicleRuntime()
→ 성공/실패 readback
→ 실패 시 이전 Vehicle/Fitting 상태 복구
```

이 경로는 Guided Vehicle Builder의 same-Pawn runtime 적용 계약과 같은 Vehicle Runtime authority를 사용한다.

Vehicle 적용이 성공했다고 해서 Inventory ownership이 변경되는 것은 아니다.

---

## 4. Equipment Runtime Apply 계약

장비 적용은 WeaponData 포인터를 직접 갈아끼우는 작업이 아니다.

정식 경로는 다음과 같다.

```text
Target MountProfileId
+ EquipmentPresetData
        ↓
완전한 transient VehicleFittingData 후보
        ↓
BuildFittingSnapshot()
        ↓
호환성 / 총질량 / 장비 데이터 전체 검증
        ↓
Prepare / Commit
        ↓
필요 시 Chaos Mass hot reapply
        ↓
Weapon / Ammo / Launcher / TurretVisual / Combat runtime refresh
        ↓
final readback
```

검증 또는 Commit이 실패하면 이전 Runtime/Fitting 상태를 보존하거나 recovery한다.

---

## 5. Complete Mount State 계약

### 5.1 왜 필요한가

Vehicle Builder가 만든 차량은 여러 Mount를 가질 수 있고, 일부 Mount가 비어 있는 것이 정상이다.

예:

```text
DA_Vehicle_Wagon
├─ Mount_Top_01   : 장비 없음
└─ Mount_Front_01 : 장비 없음
```

`Mount 존재 + 장비 없음`은 데이터 누락 오류가 아니라 **정상적인 Empty Mount 상태**다.

### 5.2 Legacy → Snapshot 최초 승격

Snapshot이 아직 없는 Legacy Runtime에서 첫 장비를 적용할 때 RuntimeApply는 VehicleData의 모든 Mount를 빠짐없이 materialize한다.

각 Mount는 다음 세 상태 중 하나로 명시된다.

```text
1. DefaultEquipmentPresetData 있음
   → 기본 장비가 활성 선택으로 materialize

2. DefaultEquipmentPresetData 없음
   → ExplicitEmpty 선택으로 materialize

3. USER가 변경할 Target Mount
   → 선택한 EquipmentPresetData로 교체
```

모든 Mount를 명시한 뒤 `MissingMountSelectionPolicy`는 `TreatAsError`로 전환한다.

이 시점 이후 Mount 선택이 누락되면 그것은 정상 Empty가 아니라 **RuntimeApply 후보 구성 결함**으로 fail-closed 처리한다.

### 5.3 기존 Snapshot 보존

현재 Pawn에 같은 VehicleData의 `VehicleFittingData`가 존재하더라도 그것만으로 기존 선택을 authoritative state로 간주하지 않는다.

기존 선택을 복제해 보존하려면 실제 `VehicleFittingComp`에 Applied Fitting Snapshot이 있어야 한다.

```text
VehicleFittingData 존재
+ 같은 VehicleData
+ Applied Snapshot 존재
= 현재 Snapshot 선택 상태 보존 가능
```

Applied Snapshot이 없다면 Legacy 상태로 보고 VehicleData 전체 Mount를 다시 완전 materialize한다.

---

## 6. 검증 계층

RuntimeApply의 검증은 한 단계가 아니다.

```text
1. Candidate authorization
   - Vehicle: Runtime Test Catalog exact membership
   - Equipment: Production Publication Catalog exact published membership 우선
   - Production Catalog absent/invalid 시에만 legacy Runtime Test Catalog Equipment membership fallback

2. Mount 구조 호환성
   - MountType / SizeLimit / Equipment 요구 조건

3. Full Fitting Snapshot validation
   - 모든 Mount 해석
   - Equipment data completeness
   - 중복/누락
   - Gross Mass 등

4. Runtime Prepare / Commit

5. dependent runtime refresh / readback

6. 실패 시 recovery / rollback
```

UI에서 `현재 Mount에 장착 가능`이 표시되어도 전체 Fitting Snapshot의 총질량 검증에서 거부될 수 있다. 두 판정은 서로 다른 계층이며 둘 다 정상적으로 유지해야 한다.

---

## 7. Wagon 2-Mount 사례

2026-09-14 기준 USER-approved Wagon baseline은 Hardpoint/Mount 2/2다.

```text
Mount_Top_01
Mount_Front_01
```

발견된 회귀는 Legacy→Snapshot 첫 장비 적용 후보가 Target Mount만 명시하고 나머지 Mount를 `UseVehicleDefault`에 맡기면서 발생했다.

`Mount_Front_01`에 기본 장비가 없는 정상 상태가 다음처럼 잘못 해석됐다.

```text
정상 Empty Mount
→ 누락 selection
→ UseVehicleDefault
→ DefaultEquipment 없음
→ MissingEquipmentPreset
→ 전체 Snapshot ValidationFailed
```

현재 교정은 이를 다음으로 바꾼다.

```text
Mount_Top_01   → 선택 장비
Mount_Front_01 → ExplicitEmpty
Missing policy → TreatAsError
```

2026-10-07 CCAS Production exact8 cutover 이후 Wagon Top Mount는 `Cannon_Standard`와 `Rocket_Standard`의 MountType/SizeLimit 구조 호환성 자체는 통과한다. 그러나 canonical Workbook이 소유하는 current Production 질량과 explicit sortie ammo를 모두 포함하면 현재 Wagon `MaximumGrossMassKg=2350`을 초과한다.

```text
Cannon_Standard on Wagon
- candidate total = 3216kg
- Wagon max gross = 2350kg
- result = ValidationFailed / mutation0

Rocket_Standard on Wagon
- candidate total = 3112kg
- Wagon max gross = 2350kg
- result = ValidationFailed / mutation0
```

따라서 현재 Current truth는 "Mount 구조 호환 = 장착 성공"이 아니다. RuntimeApply는 published Product를 정상 discovery/authorization한 뒤 Full Fitting Snapshot의 GrossMass에서 fail-closed하며, 기존 Legacy Runtime/Fitting source/active Mount를 보존한다. Product balance나 Wagon gross limit을 테스트 통과 목적으로 낮추거나 올리지 않는다.

2026-09-14 Prototype HeavyCannon/RocketLauncher가 Provisional Gameplay Balance에서 Wagon Top Mount에 성공했던 결과는 Historical evidence로 보존한다. 현재 Production exact8에서는 canonical Workbook의 Product/Ammo 질량과 strict GrossMass validator가 우선한다.

---

## 8. Inventory / Garage 후속 연동 원칙

향후 정식 Inventory/Garage는 RuntimeApply UI를 그대로 재사용할 필요가 없다.

대신 다음 분리를 유지한다.

```text
Garage / Inventory
= 후보 목록, 소유권, Reservation, 비용, Transaction 정책

Runtime Apply Backend
= 선택 결과를 Vehicle/Fitting Runtime에 안전하게 적용
```

따라서 향후 정식 시스템에서 다음과 같은 별도 구현을 만들지 않는다.

```text
RuntimeApply용 장착 알고리즘
Inventory용 장착 알고리즘
Garage용 장착 알고리즘
Mission Loadout용 장착 알고리즘
```

소유권/Transaction 앞단은 달라질 수 있지만 Fitting Snapshot 생성·검증·Prepare/Commit/Recovery와 dependent runtime refresh는 공통 authority를 재사용한다.

---

## 9. 현재 비책임 / 남은 범위

현재 다음 항목은 아직 RuntimeApply 완료 범위가 아니다.

```text
- 정식 Inventory/Garage USER UI
- Equipment Empty/Default를 직접 선택하는 정식 UI action
- ItemInstance ownership / Reservation / Atomic Transfer
- 영구 Loadout SaveGame
- 서버 권한 / Replication 장비 교체
- RTA-P0-06 Packaged Demo E2E
```

현재 Frontend가 Catalog 기반이라는 사실을 Backend의 영구 후보 공급 계약으로 확대하지 않는다.

---

## 10. 주요 Source / Test

```text
UE/Source/CarFight_Re/Public/CFRuntimeEquipApply.h
UE/Source/CarFight_Re/Private/CFRuntimeEquipApply.cpp
UE/Source/CarFight_Re/Private/CFRuntimeVehicleApply.cpp
UE/Source/CarFight_Re/Private/CFVehicleFittingData.cpp
UE/Source/CarFight_Re/Private/UI/CFRuntimeApplyPIETests.cpp
UE/Source/CarFight_Re/Private/CFRuntimeEquipApplyTests.cpp
Tools/RunRuntimeApplyTests.ps1
```

---

## 11. 현재 검증 상태

2026-09-14 Prototype Weapon Provisional Gameplay Balance + Multi-Mount Current 기준:

```text
Persisted Product Asset
- DA_CannonBody.TurretMountWeightKg = 200kg
- DA_ProtoTurretCannon.WeaponMassKg = 100kg
- DA_RocketBody.TurretMountWeightKg = 150kg
- DA_RocketLauncher.WeaponMassKg = 80kg
- Fresh AssetDump /Game/CarFight/Weapons/Data = 11/11 PASS

Official UE 5.8 Editor Build
- PASS
- Job: 8c957a50fe594132b3448d6f0358eed2

RuntimeApply Automation
- PASS
- Process Job: 12eb4517709249cd9d5a8b0e5a4d25a6
- EngineExitCode=0
- Success=18 / Failure=0
- Result SHA-256: f54303f90e1491a216324439322102a73484b46dde2467760e58ac5ff28ecb8a
- `CF_FQ_058.ProductionCatalogAuthorization`: published Product exact8 authorization + unpublished fail-closed PASS
- `CF_FQ_058.ProductionCatalogDiscovery`: RuntimeApply Equipment option exact8이 Production Publication Catalog를 우선 source로 사용함을 PASS
- `CF_FQ_047.WagonMountEquipmentPIE`: Production Cannon_Standard/Rocket_Standard는 Mount 호환이지만 current Wagon GrossMass 초과로 ValidationFailed + mutation0 PASS
- `RTA_P0_05.PIEE2E`: Production Cannon_Standard/Rocket_Standard 반복 apply와 Applied Snapshot TotalVehicleMassKg↔Chaos configured mass readback PASS

Fitting Mobility Fixture 파생 검증
- Process Job: 4d34130bf6c749b19ae294c879ab3ac9
- PASS / Exit 0
- Product DA_RocketBody를 공유하는 Default Fixture = Equipment 270kg / Total 1370kg
- 격리 Heavy Fixture = Equipment 470kg / Ammo 30kg / Defense 100kg / Total 1600kg 유지
```

Historical RuntimeApply regression과 현재 validation을 구분한다. Multi-Mount Empty-State correction의 과거 16/16 PASS와 Prototype Provisional Balance 시절 Wagon 장착 성공은 Historical evidence로 보존한다. 현재 Product 기준은 canonical Workbook/Production Publication Catalog exact8을 사용한 위 fresh 18/18 PASS가 우선한다.

---

## 12. Changelog

### v1.2.0 - 2026-10-07

- CF-FQ-058 managed cutover 이후 RuntimeApply Equipment discovery/authorization의 normal source를 generated Production Publication Catalog exact8로 승격했다. Vehicle 후보 authority는 기존 Runtime Test Catalog를 유지한다.
- `UCFProdEquipCatalogData` runtime load + published Equipment exact lookup, `FCFRuntimeEquipApplyService::ApplyPublishedEquipment`, `CFRuntimeApplyWidget` Production-first/fallback 경계를 Current 계약으로 기록했다.
- legacy RuntimeTestCatalog Equipment membership은 Production Catalog absent/invalid일 때만 fallback하며 기존 `ApplyCatalogEquipment` 계약은 제거하지 않는다.
- current Wagon은 Production `Cannon_Standard`/`Rocket_Standard`와 Mount 구조상 호환되지만 explicit sortie ammo 포함 3216kg/3112kg로 2350kg gross limit을 초과하므로 ValidationFailed + mutation0가 정상 Current behavior다. Product/Vehicle balance는 테스트를 위해 수정하지 않았다.
- RTA-P0-05의 고정 1400kg 기대값을 제거하고 Applied Snapshot `TotalVehicleMassKg`와 Chaos configured mass exact 동기화를 검증하도록 갱신했다.
- Official UE 5.8 Build `97030b7aba1348d4b1688c34afc19954` PASS, RuntimeApply `12eb4517709249cd9d5a8b0e5a4d25a6` 18/18 PASS, Result SHA-256 `f54303f90e1491a216324439322102a73484b46dde2467760e58ac5ff28ecb8a`.

### v1.1.0 - 2026-09-14

- Prototype 무기 질량을 `Provisional Gameplay Balance`로 조정한 현재 Product 상태를 RuntimeApply Current 계약에 동기화했다.
- Wagon Top Mount에서 Product HeavyCannon 300kg과 RocketLauncher 230kg이 모두 정상 적용되며 비대상 Front Mount는 `ExplicitEmpty`로 보존되는 실제 PIE regression으로 갱신했다.
- 이전 350+120kg placeholder의 Wagon GrossMass 거부와 transient lightweight 성공 경로는 Historical evidence로 내리고, 현재 strict GrossMass validator 자체는 그대로 유지됨을 명시했다.
- fresh AssetDump 11/11, UE 5.8 Build `8c957a50fe594132b3448d6f0358eed2` PASS, RuntimeApply `54dbac4be35b45ce914e915002867561` 16/16 PASS와 Fitting Mobility Fixture 파생 검증 PASS를 기록했다.

### v1.0.0 - 2026-09-14

- RuntimeApply의 현재 구현을 독립 Current System으로 승격했다.
- 현재 VehicleDebug RuntimeApply UI는 non-owning Catalog frontend, Runtime Apply service는 향후 Garage/Inventory에서도 재사용할 application seam으로 역할을 분리했다.
- Legacy→Snapshot 첫 장비 적용의 Complete Mount State 계약을 추가했다.
- 기본 장비 없는 Mount를 `ExplicitEmpty`로 materialize하고 전체 Mount 구성 뒤 누락은 `TreatAsError`로 fail-closed하는 정책을 기록했다.
- 기존 Fitting 선택 보존은 실제 Applied Snapshot이 있을 때만 허용하는 authority 경계를 기록했다.
- Wagon 2-Mount에서 Empty Mount 오류와 실제 GrossMass 검증을 분리했다.
- Official UE 5.8 Build PASS 뒤 fresh RuntimeApply Automation 16/16 PASS를 확보해 Multi-Mount remediation Technical PASS로 닫았다.

## 13. Migration

- 기존 문서에서 RuntimeApply를 단순 Debug/Demo 기능으로만 해석한 표현은 **현재 UI frontend의 용도**로 제한해서 읽는다.
- Runtime 적용 Backend의 현재 계약은 이 문서와 실제 Source를 우선한다.
- 향후 Garage/Inventory/Fitting frontend는 소유권·후보 선택 정책만 추가하고 기존 Runtime Apply application seam과 Fitting Runtime authority를 중복 구현하지 않는다.
- Historical `RTA-P0-05` USER PASS와 기존 regression PASS는 보존한다. 2026-09-14 Multi-Mount remediation과 Provisional Weapon Mass 후속 regression은 각각의 fresh evidence로 구분하며 다음 Product Gate는 기존 `RTA-P0-06 Packaged Demo`다.
- Prototype 무기 질량은 정식 현실 제원이 아니라 현재 개발용 게임플레이 밸런스다. 향후 Small/Medium/Large 무기군 정식 밸런스가 갖춰질 때 Product DataAsset 값을 재조정하되 RuntimeApply/Fitting의 GrossMass 검증 코드는 완화하지 않는다.
