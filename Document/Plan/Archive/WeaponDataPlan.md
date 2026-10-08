# CF-FQ-008 무장 데이터 정의 Plan

- 문서 버전: v0.3.0
- 최근 갱신일: 2026-08-15
- 문서 상태: Historical + Retained Path
- Feature: `CF-FQ-008 무장 데이터 정의`
- 현재 단계: `WD-P0-00~03 Done / CF-FQ-008 Done`
- Content Asset 변경: 금지 / 대표 WeaponData는 read-only 검증

---

## 1. 목적

CF-FQ-008은 새 Weapon 시스템을 다시 설계하는 작업이 아니다.
현재 `UCFWeaponData`는 WeaponFire, Launcher, Ammo, Fitting, EquipmentPreset가 이미 공통으로 소비하는 정적 무기 정의다.

이번 작업의 목표는 현재 구현을 하나의 **정적 WeaponData 계약**으로 공식화하고, 잘못된 DataAsset이 런타임까지 들어가기 전에 Unreal Data Validation과 Automation으로 검출되도록 만드는 것이다.

```text
UCFWeaponData 현재 필드
→ 정적 데이터 계약
→ 대표 WeaponData read-only 검증
→ Current System 문서
```

WeaponFire의 발사 승인 상태, Ammo의 Loaded/Reserve/Reload 상태, Launcher의 Sequence Runtime 상태는 WeaponData Validator가 중복 소유하지 않는다.

---

## 2. 보호 조건

- `DA_ProtoTurretCannon`, `DA_RocketLauncher`와 기타 WeaponData Content Asset은 read-only 기준 자산이다.
- WeaponData Asset을 자동 수정·저장하지 않는다.
- 기존 Ammo, Launcher, WeaponFire, Fitting Runtime 계약을 재구현하지 않는다.
- `DefaultProjectileData=None`은 현재 Dummy HitScan fallback 계약 때문에 자동 Error로 처리하지 않는다.
- `FireRatePerMinute=0`은 현재 쿨다운 없는 fallback이므로 허용한다.
- `MaxRange=0`은 현재 Weapon/Aim fallback 사거리 계약 때문에 허용한다.
- `WeaponMassKg=0`은 기존 피팅 미설정 질량 호환값으로 허용한다.
- `bUseInfiniteAmmoForDebug=true`에서는 `DefaultAmmoData=None`과 MagazineSize>0 같은 기존 저장 조합을 오류로 승격하지 않는다.
- 실제 런타임 상태·USER 발사 감각을 정적 DataAsset Validation으로 대체하지 않는다.

---

## 3. 현재 UCFWeaponData 역할

현재 주요 데이터 영역:

```text
Identity
- WeaponId

Mount
- WeaponSize
- CompatibleMountTypes

Mass
- WeaponMassKg

Fire
- FireMode
- FireRatePerMinute
- MaxRange
- SpreadDeg

Target Use
- TargetUsePolicy

Launcher
- LauncherFirePatternConfig
- LauncherReleaseConfig

Ammo
- MagazineSize
- ReloadTimeSeconds
- DefaultAmmoData
- InitialLoadedAmmoCount
- AmmoUnitsPerShot
- ReloadMode
- bAutoReloadWhenEmpty
- bAllowPartialReload
- bAllowPartialSequence
- bUseInfiniteAmmoForDebug

FX / Projectile
- DefaultFireFxData
- DefaultProjectileData

Legacy / Reserved
- BaseDamage
- AmmoTypeId
- ProjectileDataId
- DamageProfileId
- HeatPerShot / MaxHeat
```

실제 피해 Data SSOT는 `DefaultProjectileData → DamageData`이며 `BaseDamage / DamageProfileId`는 런타임 Damage SSOT가 아니다.

---

## 4. 현재 소비 경계

```text
EquipmentPresetData
→ WeaponData.CanUseOnMount()

Fitting
→ WeaponData.GetEffectiveWeaponMassKg()

VehicleWeaponComp / WeaponFire
→ FireMode / FireRate / MaxRange / ProjectileData / TargetUsePolicy

Launcher
→ GetEffectiveLauncherFirePatternConfig()
→ GetEffectiveLauncherReleaseConfig()

Ammo Runtime
→ UsesFiniteAmmoRuntime()
→ Magazine / DefaultAmmoData / Reload 정책
```

Validator는 위 Runtime의 현재 상태를 다시 계산하지 않고 저장된 WeaponData 자체의 정합성만 검사한다.

---

## 5. 대표 read-only baseline

### DA_ProtoTurretCannon

ObjectPath:
`/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon.DA_ProtoTurretCannon`

현재 확인값:

```text
WeaponId = Proto_TurretCannon
WeaponSize = Large
CompatibleMountTypes = Turret
WeaponMassKg = 0
FireMode = Projectile
FireRatePerMinute = 120
MaxRange = 10000
SpreadDeg = 0
Launcher Pattern = SingleCycle / 1
Launcher Release = Direct
MagazineSize = 10
ReloadTimeSeconds = 0
DefaultProjectileData = DA_HeavyShell
DefaultFireFxData = DA_FX_ProtoWeaponFire
```

### DA_RocketLauncher

ObjectPath:
`/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher.DA_RocketLauncher`

현재 확인값:

```text
WeaponId = Proto_RocketLauncher
WeaponSize = Large
CompatibleMountTypes = Turret
WeaponMassKg = 0
FireMode = Projectile
FireRatePerMinute = 20
MaxRange = 15000
SpreadDeg = 0
Launcher Pattern = Salvo / 4 / simultaneous 4 / cooldown SequenceCompleted
Launcher Release = Direct
MagazineSize = 4
ReloadTimeSeconds = 0
DefaultProjectileData = DA_Rocket_PropTest
DefaultFireFxData = DA_FX_ProtoWeaponFire
```

현재 AssetDump managed surface가 신규 Ammo 필드 전체를 projection하지 않을 수 있으므로 `DefaultAmmoData / InitialLoaded / bUseInfiniteAmmoForDebug`의 실제 최종값은 최신 C++ Load-only Automation에서 검증한다. 누락된 projection 값을 추정하지 않는다.

---

## 6. 작업 단계

| Task | 이름 | 상태 | 종료 기준 |
| --- | --- | --- | --- |
| `WD-P0-00` | Foundation Audit | Done | 현재 WeaponData schema, 소비자, 대표 자산과 호환 fallback 확인 |
| `WD-P0-01` | Static Data Contract | Technical Done | `ValidateWeaponDataContract` + Unreal DataValidation + `StaticContract` PASS |
| `WD-P0-02` | Representative Assets | Technical Done | 대표 WeaponData 2개 Load-only `RepresentativeAssets` PASS / 저장 0 |
| `WD-P0-03` | Current System Integration | Done | `Systems/Combat/WeaponData.md v1.0.0` 승격 + `WeaponFire.md v1.6.0` stale Ammo 계약 교정 |

---

## 7. WD-P0-01 — Static Data Contract

`UCFWeaponData` 자체에 `ValidateWeaponDataContract()`와 Editor `IsDataValid()`를 추가한다.
별도 병렬 Validator 클래스를 만들지 않는다.

### Error 계약

```text
WeaponId == None
WeaponSize == None
CompatibleMountTypes empty
CompatibleMountTypes에 None 포함
CompatibleMountTypes 중복
WeaponMassKg 음수 또는 비유한
FireRatePerMinute 음수 또는 비유한
MaxRange 음수 또는 비유한
SpreadDeg 음수 또는 비유한
MagazineSize 음수
ReloadTimeSeconds 음수 또는 비유한
InitialLoadedAmmoCount 음수
InitialLoadedAmmoCount > MagazineSize
AmmoUnitsPerShot < 1
DefaultAmmoData가 명시됐지만 AmmoId invalid
bUseInfiniteAmmoForDebug=false인데 유효 DefaultAmmoData 없음
bUseInfiniteAmmoForDebug=false인데 MagazineSize <= 0
HeatPerShot / MaxHeat 음수 또는 비유한
```

### 허용 계약

```text
WeaponMassKg = 0
FireRatePerMinute = 0
MaxRange = 0
DefaultProjectileData = None
DefaultFireFxData = None
bUseInfiniteAmmoForDebug=true + DefaultAmmoData=None
bUseInfiniteAmmoForDebug=true + MagazineSize>0
HitScan + 저장된 non-Direct LauncherReleaseConfig
```

위 값은 현재 Runtime의 명시적 fallback·호환 계약을 따른다.

LauncherFirePatternConfig와 LauncherReleaseConfig의 clamp/fallback은 기존 `GetEffective...()`가 소유한다. WD-P0-01에서 Scheduler/Release Runtime 정책을 중복 구현하지 않는다.

---

## 8. WD-P0-02 — Representative Assets

Automation에서 두 대표 WeaponData를 Load-only로 읽는다.

검증:

```text
ValidateWeaponDataContract PASS
WeaponId
Mount compatibility
FireMode
대표 FireRate
Launcher Pattern
DefaultProjectileData 유효 참조
현재 UsesFiniteAmmoRuntime 결과 기록
```

에셋 Compile/Save/Fixup을 하지 않는다.

---

## 9. WD-P0-03 — Current System Integration

완료된 Current 문서:
`Document/Systems/Combat/WeaponData.md v1.0.0`

승격 내용:

```text
WeaponData static SSOT
Mount / Mass / Fire / TargetUse / Launcher / Ammo / Projectile·FX 경계
legacy / reserved field 비책임
DataValidation Invalid·허용 계약
대표 WeaponData baseline
Build·Automation 증거
```

`Document/Systems/Combat/WeaponFire.md`는 v1.6.0으로 교정했다. 과거 `MagazineSize / ReloadTimeSeconds` Runtime 미구현 문구를 제거하고, 이 값들은 WeaponData의 정적 입력이며 실제 Loaded·Reserve·Reservation·Reload Runtime은 `UCFVehicleAmmoComp`가 소유한다고 현재 CF-FQ-031 계약에 맞췄다.

원래 Plan에서 예정한 `Document/Systems/Data/WeaponData.md` 경로는 실제 Systems 구조에 별도 `Data/` 분류가 없음을 확인한 뒤 사용하지 않았다. 기존 Combat 소비자 문서와 같은 도메인에 두기 위해 최종 Current owner를 `Document/Systems/Combat/WeaponData.md`로 확정했다.

---

## 10. 완료 기준

```text
WD-P0-01 PASS
WD-P0-02 PASS
WD-P0-03 문서 정합성 완료
공식 UE 5.8 Editor Build PASS
CarFight.WeaponData targeted PASS
WeaponData Content Asset mutation = 0
```

현재 검증 증거:

```text
첫 Build: f3cd9332aa854248979f164053a7e4c4 / Exit 6
- 원인: CFWeaponDataTests.cpp의 TNumericLimits<float>::Infinity() 테스트 API 사용 오류
- CFWeaponData.cpp Runtime/DataValidation 본체 결함 아님

교정 후 최종 Build: 53e2dbaf04a3401b8ed89c906b308cdc / PASS / Exit 0
CarFight.WeaponData: eefe58aa87d74dcaa79a1764a5f7611b / 2/2 Success / 0 Fail
- WD-P0-01 StaticContract PASS
- WD-P0-02 RepresentativeAssets PASS
WeaponData Content Asset mutation = 0
```

CF-FQ-008은 정적 데이터 정의 기능이며 WD-P0-00~03, 공식 Build, targeted Automation과 Current System 승격을 모두 만족했으므로 **Done**으로 판정한다. 별도 USER PIE는 요구하지 않는다. 이 완료 판정은 WeaponFire·Launcher·Ammo·UI·TargetSelect 등 다른 Runtime Feature의 기존 USER Pending을 새 PASS로 승격하지 않는다.

---

## 11. Changelog

### v0.3.0 - 2026-08-15

- WD-P0-03 Current System Integration을 완료하고 `Document/Systems/Combat/WeaponData.md v1.0.0`을 Current System으로 승격했다.
- 원래 예정한 `Systems/Data/WeaponData.md` 대신 기존 Combat 도메인 문서 구조와 일치하는 `Systems/Combat/WeaponData.md`를 실제 owner로 확정했다.
- `Document/Systems/Combat/WeaponFire.md v1.6.0`의 Magazine/Reload 미구현 설명을 CF-FQ-031 Ammo Current Runtime 책임 경계로 교정했다.
- CF-FQ-008을 Done으로 판정하고 이 Plan을 `Historical + Retained Path`로 전환했다. 물리 Archive 이동은 수행하지 않는다.
- WD-P0-03은 문서 전용 변경이므로 이미 PASS한 Build `53e2dbaf04a3401b8ed89c906b308cdc`와 `CarFight.WeaponData` 2/2를 반복 실행하지 않았다.
- 기존 WeaponData Content Asset mutation은 계속 0이며 다른 Feature의 USER Pending을 새 PASS로 추정하지 않았다.

### v0.2.0 - 2026-08-15

- WD-P0-01 Static Data Contract를 Technical Done으로 전환했다. `UCFWeaponData::ValidateWeaponDataContract()`와 Unreal `IsDataValid()`를 추가하고 기존 Runtime fallback은 유지했다.
- WD-P0-02 Representative Assets를 Technical Done으로 전환했다. `DA_ProtoTurretCannon`과 `DA_RocketLauncher`를 Load-only로 검증했으며 저장·Fixup은 수행하지 않았다.
- 첫 Build `f3cd9332aa854248979f164053a7e4c4`는 테스트의 Infinity API 사용 오류로 실패했고, `std::numeric_limits<float>::infinity()`로 교정 후 Build `53e2dbaf04a3401b8ed89c906b308cdc` PASS를 확인했다.
- `CarFight.WeaponData` Automation `eefe58aa87d74dcaa79a1764a5f7611b`는 2/2 Success / 0 Fail이다.
- WD-P0-03 Current System Integration은 아직 시작하지 않았으며 다음 단계로 유지한다.

### v0.1.0 - 2026-08-15

- CF-FQ-008을 현재 구현을 재설계하지 않고 UCFWeaponData 정적 계약 공식화 작업으로 재정의했다.
- WeaponFire, Launcher, Ammo, Fitting, EquipmentPreset 소비 경계를 fresh 감사했다.
- DA_ProtoTurretCannon과 DA_RocketLauncher를 read-only baseline으로 확정했다.
- DefenseData와 유사한 `Validate...Contract + IsDataValid` 패턴을 WD-P0-01 기본안으로 선정했다.

---

## 12. Migration

- v0.3.0부터 현재 구현 owner는 `Document/Systems/Combat/WeaponData.md v1.0.0`과 실제 `UCFWeaponData` 코드다. 이 Plan의 과거 next-action은 현재 작업 지시로 사용하지 않는다.
- 이 파일은 물리 경로를 유지하는 `Historical + Retained Path`다. Historical 탐색은 `Document/Plan/Archive/README.md`를 사용한다.
- 기존 UCFWeaponData 필드와 Runtime getter를 유지한다.
- Launcher/Ammo/WeaponFire Runtime 정책을 WeaponData Validator로 이동하지 않는다.
- 기존 WeaponData Content Asset은 WD-P0-01~03에서 수정하지 않는다.
- CF-FQ-015는 VD-P0-00~03 Remote Technical Done / VD-P0-04 USER Tuning Pending 체크포인트로 보존한다.
- CF-FQ-026 TS-P0-08 USER PIE 체크포인트도 그대로 보존한다.
