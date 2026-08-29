# WeaponData

- Version: 1.1.0
- Date: 2026-08-18
- Status: Current System / UI-P0-06 Weapon Heat Runtime Additive Integration
- Feature: `CF-FQ-008 무장 데이터 정의` + `CF-FQ-032 UI-P0-06 Weapon Heat Runtime 소비 확장`
- Verification: 기존 CF-FQ-008 검증 보존 / Heat final UE 5.8 Editor Build `0ecfed49ab3a4f41b349fc707c44e5f2` PASS / exact `CarFight.UI.UI_P0_06.HeatRuntimeResourceContract` `c736d1a6d6134a798a4b84452750e3d6` 1/1 PASS / WeaponData Content Asset mutation 0

---

## 1. 문서 목적

이 문서는 CarFight의 현재 `UCFWeaponData`가 차량 장착 무기의 **정적 데이터 SSOT**로서 어떤 값을 소유하고, Fitting·WeaponFire·Launcher·Ammo·TargetUse·Projectile/FX 계층이 그 값을 어떤 경계로 소비하는지 기록한다.

이 문서는 미래 무기 설계서가 아니다. 현재 구현 판단은 실제 `UCFWeaponData` 코드와 이 Current System을 우선하며, CF-FQ-008의 작업 과정과 검증 이력은 `Document/Plan/Archive/WeaponDataPlan.md`를 Historical 기록으로 참고한다.

핵심 원칙은 다음과 같다.

```text
WeaponData
= 무기 한 종류의 정적 정의

WeaponData
!= 현재 장전량 / Reserve / Reload 진행 상태
!= Launcher Sequence 진행 상태
!= 발사 승인·거부 Runtime 상태
!= 실제 Damage Runtime 상태
```

---

## 2. 현재 구현 범위

현재 WeaponData 계약의 핵심 타입은 `UCFWeaponData`다.

| 구분 | 현재 구현 |
| --- | --- |
| 정적 무기 정의 | `UCFWeaponData` |
| 장착 호환 판정 | `SupportsMountType`, `SupportsWeaponSize`, `CanUseOnMount` |
| 피팅 질량 입력 | `WeaponMassKg`, `GetEffectiveWeaponMassKg` |
| 발사 정적 입력 | `FireMode`, `FireRatePerMinute`, `MaxRange`, `SpreadDeg` |
| 대상 사용 정적 정책 | `TargetUsePolicy` |
| Launcher 정적 입력 | `LauncherFirePatternConfig`, `LauncherReleaseConfig` |
| Ammo 정적 입력 | Magazine·AmmoData·Reload·부분 처리·무한탄 호환 설정 |
| Heat 정적 입력 | `HeatPerShot`, `MaxHeat`, `HeatDissipationPerSecond`, `UsesWeaponHeatRuntime` |
| Projectile / FX 참조 | `DefaultProjectileData`, `DefaultFireFxData` |
| 정적 계약 검증 | `ValidateWeaponDataContract`, Editor `IsDataValid` |
| Debug 요약 | `BuildWeaponSummary`, `BuildAmmoConfigSummary`, Launcher 요약 함수 |

`UCFWeaponData`는 `UPrimaryDataAsset`이며 EquipmentPresetData에서 참조되는 차량 장착 무기의 정적 정의다.

---

## 3. 정적 데이터 소유권

### 3.1 Identity

```text
WeaponId
```

`WeaponId`는 로그, Debug와 저장 데이터에서 무기를 안정적으로 식별하는 이름이다.
정적 계약상 `None`은 허용하지 않는다.

### 3.2 Mount

```text
WeaponSize
CompatibleMountTypes
```

현재 장착 판정은 다음 helper를 사용한다.

```text
SupportsMountType()
SupportsWeaponSize()
CanUseOnMount()
```

정적 계약상 다음은 Invalid다.

```text
WeaponSize == None
CompatibleMountTypes empty
CompatibleMountTypes에 None 포함
CompatibleMountTypes 중복
```

실제 어떤 하드포인트와 MountProfile에 장착되는지는 Fitting / EquipmentPreset 계층이 결정한다. WeaponData는 차량의 현재 장착 상태를 소유하지 않는다.

### 3.3 Mass

```text
WeaponMassKg
GetEffectiveWeaponMassKg()
```

`WeaponMassKg`는 무기 본체가 피팅 총중량에 기여하는 정적 질량이다.

현재 호환 계약:

```text
WeaponMassKg = 0
→ 유효
→ 기존 미설정 질량 호환값
```

음수 또는 비유한 값은 DataValidation Invalid다.
실제 차량 총 질량 계산과 Runtime 재적용은 Fitting 계층이 소유한다.

### 3.4 Fire

```text
FireMode
FireRatePerMinute
MaxRange
SpreadDeg
```

현재 helper:

```text
GetEffectiveFireRatePerMinute()
GetFireIntervalSeconds()
```

`GetFireIntervalSeconds()`는 유효 FireRate가 0 이하이면 0초를 반환하고, 그 외에는 `60 / RPM`으로 실제 발사 간격을 만든다.

현재 호환 계약:

```text
FireRatePerMinute = 0
→ 쿨다운 없는 현재 fallback으로 허용

MaxRange = 0
→ 현재 Weapon/Aim 사거리 fallback을 위해 허용

SpreadDeg = 0
→ 탄퍼짐 없음
```

음수 또는 비유한 FireRate·MaxRange·Spread는 Invalid다.

### 3.5 Target Use

```text
TargetUsePolicy
```

`TargetUsePolicy`는 선택된 대상에 이 무기를 사용할 수 있는지 평가할 정적 정책 입력이다.

WeaponData는 TargetSelect의 후보 탐색, 선택 수명, HUD 표시를 소유하지 않는다. 실제 대상 선택 상태는 TargetSelect 계층이 소유하고, WeaponData는 장비별 사용 가능성 평가에 필요한 정적 정책만 제공한다.

### 3.6 Launcher

```text
LauncherFirePatternConfig
LauncherReleaseConfig
```

현재 helper:

```text
GetEffectiveLauncherFirePatternConfig()
BuildLauncherFirePatternSummary()
GetEffectiveLauncherReleaseConfig()
BuildLauncherReleaseSummary()
```

`LauncherFirePatternConfig`는 SingleCycle·Ripple·Salvo의 입력당 발사 수, Ripple 간격, Salvo 동시 처리 한도와 실패·쿨다운 시작 정책을 정의한다.

`LauncherReleaseConfig`는 Direct·AngledEjection·VerticalEjection의 분리 방향, 속력, 차량 속도 상속과 안전 검사 거리를 정의한다.

현재 호환 계약:

```text
기존 WeaponData
→ SingleCycle / 1발 기본값

HitScan + 저장된 non-Direct Release
→ 저장 자체는 Invalid로 만들지 않음
→ GetEffectiveLauncherReleaseConfig()가 Direct / CarrierVelocityRatio 0으로 보정
```

실제 Ripple·Salvo Sequence 진행, 후속 발사 Scheduler, 취소와 terminal lifecycle은 Launcher Runtime이 소유한다. WeaponData Validator가 Scheduler 상태를 재구현하지 않는다.

### 3.7 Ammo 정적 설정

현재 WeaponData가 소유하는 Ammo 설정은 다음과 같다.

```text
MagazineSize
ReloadTimeSeconds
DefaultAmmoData
InitialLoadedAmmoCount
AmmoUnitsPerShot
ReloadMode
bAutoReloadWhenEmpty
bAllowPartialReload
bAllowPartialSequence
bUseInfiniteAmmoForDebug
```

현재 helper:

```text
GetEffectiveMagazineCapacity()
GetEffectiveInitialLoadedAmmoCount()
GetEffectiveAmmoUnitsPerShot()
GetEffectiveReloadTimeSeconds()
UsesFiniteAmmoRuntime()
BuildAmmoConfigSummary()
```

`UsesFiniteAmmoRuntime()`은 다음 조건을 모두 만족할 때만 True다.

```text
bUseInfiniteAmmoForDebug == false
DefaultAmmoData 유효
DefaultAmmoData.AmmoId 유효
MagazineSize > 0
```

기존 WeaponData 호환 계약:

```text
bUseInfiniteAmmoForDebug = true
DefaultAmmoData = None
→ 기존 무한탄 호환 경로 유지
→ Invalid 아님
```

유한탄 모드에서는 유효한 `DefaultAmmoData`와 `MagazineSize > 0`이 필수다.

중요한 책임 경계:

```text
WeaponData
= MagazineCapacity, 초기 장전 설정, 발사당 소비량, Reload 시간·정책의 정적 원본

UCFVehicleAmmoComp
= WeaponInstanceId별 Loaded
+ AmmoId별 Reserve
+ 발사 예약/Commit/Rollback
+ Reload 진행 상태
+ Action Lock
```

따라서 `MagazineSize`와 `ReloadTimeSeconds`는 현재 단순 예약 필드가 아니라 CF-FQ-031 Ammo Runtime이 소비하는 정적 입력이다. 실제 현재 장전량이나 Reload 진행률은 WeaponData에 저장하지 않는다.

P0 Runtime은 `FullMagazine` Reload를 실행한다. `PerRound` enum은 후속 확장 계약으로만 존재한다.

### 3.8 Heat 정적 설정

현재 WeaponData가 소유하는 Heat 입력은 다음과 같다.

```text
HeatPerShot
MaxHeat
HeatDissipationPerSecond
```

`UsesWeaponHeatRuntime()`은 세 값이 모두 유한한 양수일 때만 True다. 하나라도 0이면 Heat Runtime은 비활성이고 기존 무기 발사 동작을 유지한다.

```text
HeatPerShot > 0
MaxHeat > 0
HeatDissipationPerSecond > 0
→ Heat Runtime Enabled

그 외
→ Disabled / 기존 발사 호환
```

책임 경계는 다음과 같다.

```text
WeaponData
= 발사당 Heat / 최대 Heat / 초당 자연 냉각량의 정적 원본

FCFWeaponHeatRuntime + UCFVehicleWeaponComp
= 현재 Heat / 자연 냉각 / 과열 / 재사용 가능 상태
```

과열 회복 임계값은 별도 숨은 percentage를 만들지 않고 `max(MaxHeat - HeatPerShot, 0)`으로 계산한다. 즉 다음 표준 한 발이 MaxHeat를 넘지 않을 만큼 냉각됐을 때 재사용 가능 상태로 돌아간다.

현재 저장 WeaponData는 새 `HeatDissipationPerSecond`가 기본 0이므로 별도 authoring 전 Heat Runtime이 자동 활성화되지 않는다. UI-P0-06 Heat slice에서 Content Asset 값은 변경하지 않았다.

### 3.9 FX / Projectile

```text
DefaultFireFxData
DefaultProjectileData
```

`DefaultFireFxData`는 승인된 발사 위치와 방향에서 사용할 발사 Niagara 데이터 참조다. 비어 있어도 발사 판정은 유지한다.

`DefaultProjectileData`는 이 무기가 기본으로 사용할 ProjectileData 참조다. 비어 있으면 기존 Dummy HitScan fallback을 유지하므로 현재 정적 계약에서 `None`은 Invalid가 아니다.

실제 피해 데이터 SSOT는 다음 경로다.

```text
WeaponData.DefaultProjectileData
→ ProjectileData.DefaultDamageData
```

WeaponData가 실제 Damage 계산을 소유하지 않는다.

### 3.10 Legacy / Reserved

현재 다음 필드는 호환 또는 후속 확장을 위해 남아 있다.

```text
BaseDamage
AmmoTypeId
ProjectileDataId
DamageProfileId
private CooldownSeconds
bMigratedCooldownSecondsToFireRate
```

현재 의미:

```text
BaseDamage / DamageProfileId
→ DamageData 분리 전 레거시 확인값
→ 실제 Damage SSOT 아님

ProjectileDataId
→ 기존 에셋 호환·로그 확인용
→ 실제 ProjectileData는 DefaultProjectileData 직접 참조 우선

AmmoTypeId
→ AmmoData 분리 이전 호환 ID
→ 현재 finite Ammo 정적 참조는 DefaultAmmoData 우선

CooldownSeconds
→ 과거 저장값 마이그레이션 전용 private 필드
→ PostLoad에서 FireRatePerMinute로 1회 변환
```

Heat 정적 입력의 실제 Runtime 소비 기준은 위 `3.8 Heat 정적 설정`을 따른다. 세 값은 음수·비유한이면 Invalid이며, 0은 backward-compatible Disabled 설정으로 허용한다.

---

## 4. 현재 소비 경계

| 소비자 | WeaponData에서 읽는 현재 책임 | WeaponData가 소유하지 않는 상태 |
| --- | --- | --- |
| `UCFEquipmentPresetData` / Fitting | 장착 타입·크기 호환, 무기 참조 | 실제 장착 Instance·Inventory 소유권 |
| Fitting Mass | `GetEffectiveWeaponMassKg()` | 차량 최종 질량 적용·Physics 재생성 |
| `UCFVehicleWeaponComp` / WeaponFire | FireMode, FireRate, Range, Projectile, TargetUse와 Heat 정적 입력 | Fire 승인·거부 결과, 현재 Cooldown·CurrentHeat·Overheated 상태 |
| `UCFLauncherComp` | 유효 FirePattern / Release 설정 | Sequence 진행, Scheduler, 취소·완료 상태 |
| `UCFVehicleAmmoComp` | Magazine, AmmoData, InitialLoaded, UnitsPerShot, Reload 정책 | Loaded·Reserve·Reservation·Reload Runtime |
| TargetUse 평가 | `TargetUsePolicy` | TargetSelect 후보·선택 수명 |
| Projectile / Damage | `DefaultProjectileData` | 비행, 충돌, Damage 계산과 적용 |
| Combat FX | `DefaultFireFxData` | FX 재생 lifecycle과 실패 처리 |

정적 DataAsset과 Runtime owner를 분리하는 것이 현재 계약의 핵심이다.

---

## 5. DataValidation 계약

`UCFWeaponData::ValidateWeaponDataContract()`가 Automation과 정적 계약 검증을 공유하고, Editor에서는 `IsDataValid()`가 같은 오류를 Unreal Data Validation에 전달한다.

### 5.1 Invalid 계약

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
DefaultAmmoData가 지정됐지만 AmmoId invalid
finite ammo인데 유효 DefaultAmmoData 없음
finite ammo인데 MagazineSize <= 0
HeatPerShot 음수 또는 비유한
MaxHeat 음수 또는 비유한
HeatDissipationPerSecond 음수 또는 비유한
```

### 5.2 명시적으로 허용하는 호환값

```text
WeaponMassKg = 0
FireRatePerMinute = 0
MaxRange = 0
DefaultProjectileData = None
DefaultFireFxData = None
bUseInfiniteAmmoForDebug = true + DefaultAmmoData = None
bUseInfiniteAmmoForDebug = true + MagazineSize > 0
HitScan + 저장된 non-Direct LauncherReleaseConfig
```

이 허용값은 기존 Runtime fallback을 보존하기 위한 현재 계약이다. Validator가 “더 엄격해 보인다”는 이유만으로 기존 안전 fallback을 오류로 승격하지 않는다.

Launcher Config의 clamp/fallback은 기존 `GetEffective...()` 함수가 소유한다. DataValidation에서 Launcher Scheduler나 Release Runtime을 중복 구현하지 않는다.

---

## 6. 대표 WeaponData 기준선

CF-FQ-008은 기존 Content Asset을 수정하지 않고 다음 두 대표 자산을 Load-only로 검증했다.

### 6.1 DA_ProtoTurretCannon

ObjectPath:
`/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon.DA_ProtoTurretCannon`

검증된 대표 값:

```text
WeaponId = Proto_TurretCannon
WeaponSize = Large
CompatibleMountTypes = Turret
WeaponMassKg = 0
FireMode = Projectile
FireRatePerMinute = 120
MaxRange = 10000
SpreadDeg = 0
Launcher = SingleCycle / 1
Release = Direct
MagazineSize = 10
ReloadTimeSeconds = 0
DefaultProjectileData = DA_HeavyShell
DefaultFireFxData = DA_FX_ProtoWeaponFire
```

### 6.2 DA_RocketLauncher

ObjectPath:
`/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher.DA_RocketLauncher`

검증된 대표 값:

```text
WeaponId = Proto_RocketLauncher
WeaponSize = Large
CompatibleMountTypes = Turret
WeaponMassKg = 0
FireMode = Projectile
FireRatePerMinute = 20
MaxRange = 15000
SpreadDeg = 0
Launcher = Salvo / 4 / simultaneous 4 / cooldown SequenceCompleted
Release = Direct
MagazineSize = 4
ReloadTimeSeconds = 0
DefaultProjectileData = DA_Rocket_PropTest
DefaultFireFxData = DA_FX_ProtoWeaponFire
```

신규 Ammo 세부 필드는 AssetDump projection 누락값을 추정하지 않았다. 최종 정적 계약 유효 여부와 `UsesFiniteAmmoRuntime()` 판정은 C++ Load-only Automation 결과를 기준으로 한다.

---

## 7. 검증 증거

CF-FQ-008 최종 기술 검증:

```text
Official UE 5.8 Editor Build
Job: 53e2dbaf04a3401b8ed89c906b308cdc
Result: PASS / Exit 0

CarFight.WeaponData targeted Automation
Process: eefe58aa87d74dcaa79a1764a5f7611b
Success: 2
Failure: 0

PASS: CarFight.WeaponData.WD_P0_01.StaticContract
PASS: CarFight.WeaponData.WD_P0_02.RepresentativeAssets

Representative WeaponData
= Load-only
Compile = 0
Save = 0
Fixup = 0
Content Asset mutation = 0
```

첫 Build `f3cd9332aa854248979f164053a7e4c4`의 Exit 6은 `CFWeaponDataTests.cpp`에서 존재하지 않는 `TNumericLimits<float>::Infinity()`를 사용한 테스트 코드 오류였다. `std::numeric_limits<float>::infinity()`로 교정한 뒤 최종 Build가 PASS했으며 WeaponData Runtime/DataValidation 본체 결함으로 판정하지 않는다.

CF-FQ-008은 정적 DataAsset 계약 기능이므로 별도 USER PIE를 완료 조건으로 요구하지 않는다. 이 판정은 기존 WeaponFire·Launcher·Ammo·UI USER 체크포인트를 새 USER PASS로 승격한다는 의미가 아니다.

---

## 8. 현재 기능 책임

`WeaponData`의 현재 책임은 다음과 같다.

```text
- 무기의 안정 Identity를 제공한다.
- 장착 타입과 크기 호환 조건을 제공한다.
- 피팅에 사용할 무기 본체 질량을 제공한다.
- 발사 모드·RPM·Range·Spread 정적 설정을 제공한다.
- TargetUse 정적 정책을 제공한다.
- Launcher Pattern·Release 정적 설정과 안전한 유효값을 제공한다.
- finite Ammo의 Magazine·AmmoData·초기 장전·발사당 소비·Reload 정책을 제공한다.
- 기본 Projectile과 발사 FX 참조를 제공한다.
- 잘못된 정적 설정을 DataValidation에서 차단한다.
- 기존 에셋 fallback과 레거시 저장값 마이그레이션을 보존한다.
```

---

## 9. 현재 기준 비책임 항목

`WeaponData`는 다음 Runtime 상태를 직접 소유하지 않는다.

```text
- 현재 Loaded / Reserve 탄약 수량
- SingleCycle·Ripple·Salvo 탄약 Reservation/Commit/Rollback
- Reload 진행률과 Action Lock
- Launcher Sequence 진행·취소·완료 상태
- Fire 승인·거부 결과와 현재 Cooldown 상태
- TargetSelect 후보 탐색과 선택 수명
- Projectile 비행·충돌·Pool lifecycle
- Shield·Armor·Integrity Damage 계산
- 실제 발사 FX lifecycle
- 과열 누적·냉각 Runtime
- 서버 권한·복제 상태
```

각 상태는 현재 해당 Runtime owner가 계속 소유한다.

---

## 10. 현재 문서 기준 핵심 결론

현재 `UCFWeaponData`는 **차량 장착 무기의 Identity·장착 호환·질량·발사·대상 사용·Launcher·Ammo·Heat·Projectile/FX를 한 곳에 정의하되, 실제 전투 Runtime 상태는 각 전용 컴포넌트에 남기는 정적 무기 DataAsset SSOT**다.

가장 중요한 책임 경계는 다음 한 줄이다.

> WeaponData는 “무기가 무엇이고 어떤 정적 규칙을 가지는가”를 정의하며, “현재 게임에서 그 무기가 어떤 상태인가”는 소유하지 않는다.

---

## 11. 문서 갱신 조건

다음 변경이 생기면 이 문서를 갱신한다.

```text
- UCFWeaponData 필드 추가·삭제·의미 변경
- 장착 호환 또는 질량 Getter 계약 변경
- FireRate / Range / Spread fallback 변경
- Launcher Pattern / Release 정적 계약 변경
- Ammo static config 또는 UsesFiniteAmmoRuntime 조건 변경
- Heat static config 또는 UsesWeaponHeatRuntime 조건 변경
- DefaultProjectileData / DamageData 소유 경계 변경
- DefaultFireFxData 책임 변경
- ValidateWeaponDataContract / IsDataValid 규칙 변경
- 레거시 필드 제거 또는 migration 방식 변경
```

Runtime 상태 머신만 변경되고 WeaponData의 정적 입력 의미가 바뀌지 않는 경우에는 해당 Runtime Systems 문서를 우선 갱신한다.

---

## 12. 버전 관리

- 현재 문서 버전: `1.1.0`
- 문서 상태: `Current System`

### v1.1.0 - 2026-08-18

- `HeatDissipationPerSecond`와 `UsesWeaponHeatRuntime()`을 Current WeaponData 정적 계약에 추가했다. `HeatPerShot / MaxHeat / HeatDissipationPerSecond`가 모두 유한한 양수일 때만 실제 Heat Runtime이 활성화된다.
- 현재 Heat 누적·자연 냉각·과열 상태는 WeaponData가 아니라 `FCFWeaponHeatRuntime + UCFVehicleWeaponComp`가 소유한다. 회복은 임의 percentage가 아니라 다음 표준 한 발의 headroom인 `max(MaxHeat - HeatPerShot, 0)` 기준이다.
- 기존 저장 WeaponData는 새 냉각값 기본 0으로 Heat가 Disabled이므로 현재 발사 결과가 바뀌지 않는다. Content Asset mutation은 0이다.
- UI-P0-06 final Build `0ecfed49ab3a4f41b349fc707c44e5f2` PASS와 exact `HeatRuntimeResourceContract` `c736d1a6d6134a798a4b84452750e3d6` 1/1 PASS를 additive Current evidence로 연결했다. CF-FQ-008 Done 상태는 다시 열지 않는다.

### v1.0.0 - 2026-08-15

- `CF-FQ-008 무장 데이터 정의` 완료 범위를 Current System으로 최초 승격했다.
- UCFWeaponData의 Identity·Mount·Mass·Fire·TargetUse·Launcher·Ammo·Projectile/FX 정적 소유권을 공식화했다.
- `ValidateWeaponDataContract + IsDataValid`의 Invalid/허용 계약을 현재 기준으로 기록했다.
- `DA_ProtoTurretCannon`, `DA_RocketLauncher` Load-only 대표 검증과 Build·Automation 증거를 기록했다.
- WeaponData와 WeaponFire·Launcher·Ammo·Fitting·Projectile/Damage Runtime의 책임 경계를 명시했다.

---

## 13. Migration

```text
- CF-FQ-008 완료 이후 WeaponData 현재 구현 판단은 이 문서와 실제 UCFWeaponData 코드를 우선한다.
- `Document/Plan/Archive/WeaponDataPlan.md`는 완료 당시 설계·검증 이력을 보존하는 Historical + Archived Path 문서로 읽는다.
- 기존 WeaponData Content Asset을 새 Validator에 맞춘다는 이유만으로 자동 수정·저장하지 않는다.
- FireRate=0, MaxRange=0, WeaponMass=0, DefaultProjectileData=None과 infinite-ammo 호환은 현재 허용 계약을 유지한다.
- MagazineSize와 ReloadTimeSeconds는 현재 CF-FQ-031 Ammo Runtime의 정적 입력이며 “미구현 예약 필드”로 해석하지 않는다.
- HeatPerShot / MaxHeat / HeatDissipationPerSecond는 현재 Heat Runtime의 정적 입력이다. 세 값이 모두 양수일 때만 활성화하며 현재 Heat·과열 상태 자체는 WeaponData에 저장하지 않는다.
```

---

## 14. 마지막 확인 기준

  - 기존 CF-FQ-008 Build `53e2dbaf04a3401b8ed89c906b308cdc`
  - 기존 CF-FQ-008 Automation `eefe58aa87d74dcaa79a1764a5f7611b`
  - Heat final Build `0ecfed49ab3a4f41b349fc707c44e5f2`
  - Heat exact Automation `c736d1a6d6134a798a4b84452750e3d6` / Result SHA-256 `1abf0dc7a4788d6543c7933abef38433849ae57d569229cfab220f14937abad5`
  - `Document/Systems/Combat/Ammo.md v1.0.0`
  - `Document/Systems/Combat/WeaponFire.md`
  - `Document/Plan/Archive/WeaponDataPlan.md v0.2.0`
  - Build `53e2dbaf04a3401b8ed89c906b308cdc`
  - Automation `eefe58aa87d74dcaa79a1764a5f7611b`
