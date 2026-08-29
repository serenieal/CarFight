# Ammo

- Version: 1.0.0
- Date: 2026-08-13
- Status: Current System
- Feature: `CF-FQ-031 차량 탄약·재장전 런타임`
- Verification: UE 5.8 Editor Build PASS / CarFight Automation 64 tests·Failed 0 / AMMO-P0-08 Heavy·Ripple USER PIE PASS

---

## 1. 문서 목적

이 문서는 CarFight의 현재 차량 탄약·재장전 런타임이 실제로 어떤 데이터를 소유하고, 출격 피팅의 탄약 수량이 무기별 장전량과 차량 예비량으로 어떻게 구성되며, SingleCycle·Ripple·Salvo 발사와 Reload·HUD가 그 상태를 어떤 계약으로 소비하는지 기록한다.

이 문서는 미래 설계서가 아니다. 현재 구현 판단은 이 문서와 실제 코드·에셋을 우선하며, 완료 당시 계획과 검증 이력은 `Document/Plan/Archive/AmmoSystem/AmmoSystemPlan.md`를 참고한다.

현재 기준은 싱글플레이 로컬 차량 전투다. 서버 권한 탄약 소유권, 복제, 보급·경제와 영구 저장은 이 Current System의 완료 범위가 아니다.

---

## 2. 현재 구현 범위

현재 탄약 시스템의 핵심 타입과 책임은 다음과 같다.

| 구분 | 현재 구현 |
| --- | --- |
| 탄종 정적 데이터 | `UCFAmmoData` |
| 무기 탄약 정적 설정 | `UCFWeaponData` |
| 차량 탄약 Runtime 단일 소유 | `UCFVehicleAmmoComp` |
| 출격 탄약 입력 | `FCFAmmoSortieLoad` |
| 무기별 초기 장전 입력 | `FCFWeaponAmmoInitialization` |
| 무기별 Runtime | `FCFWeaponAmmoRuntime` |
| UI·Debug Snapshot | `FCFAmmoRuntimeSnapshot` |
| 피팅 출격 원본 | `UCFVehicleFittingData.InitialSortieAmmoLoads` |
| 런처 예약 인계 | `ACFVehiclePawn` + `UCFLauncherComp` |
| HUD 변환 | `UCFHUDDataProvider` → `FCFWeaponHUDData` → `UCFHUDPresenter` |

현재 `CFAmmoTypes.h`와 `UCFVehicleAmmoComp`는 v1.3.0, `UCFAmmoData`는 v1.0.0, `UCFWeaponData`의 Ammo 계약은 v1.10.0, `UCFVehicleFittingData`의 출격 탄약 입력은 v1.2.0 기준이다.

---

## 3. 데이터 소유권

### 3.1 AmmoData

`UCFAmmoData`는 탄종의 정적 정의만 소유한다.

```text
AmmoId
AmmoDisplayName
AmmoFamilyId
UnitMassKg
AmmoTags
AmmoIcon
MaximumLoadableAmmoCount
bCanBeResupplied
```

`MaximumLoadableAmmoCount`는 **피팅 상한**이다. 인게임 현재 탄약 수량이나 HUD 숫자로 사용하지 않는다.

`UnitMassKg`는 출격 피팅 질량 계산에 사용한다. 실제 출격 탄약량과 곱해 `AmmoMassKg`에 반영한다.

### 3.2 WeaponData

유한탄 무기는 `UCFWeaponData`에서 다음 정적 설정을 제공한다.

```text
DefaultAmmoData
MagazineSize → Runtime MagazineCapacity
InitialLoadedAmmoCount
AmmoUnitsPerShot
ReloadTimeSeconds
ReloadMode
bAutoReloadWhenEmpty
bAllowPartialReload
bAllowPartialSequence
bUseInfiniteAmmoForDebug
```

기존 WeaponData는 기본적으로 `bUseInfiniteAmmoForDebug=true`, `DefaultAmmoData=None` 호환 경로를 유지한다. 따라서 명시적으로 유한탄 설정을 하지 않은 기존 무기는 CF-FQ-031 도입 때문에 자동으로 탄약 제한을 받지 않는다.

### 3.3 VehicleFittingData

실제 출격 시작 탄약 수량의 단일 원본은 다음이다.

```text
UCFVehicleFittingData.InitialSortieAmmoLoads
```

각 `FCFAmmoSortieLoad.InitialSortieAmmoCount`는 해당 탄종의 **장전+예비 전체 실제 출격 수량**이다.

피팅 Snapshot은 다음을 검증한다.

```text
- finite WeaponData에 유효 AmmoData가 있는가
- AmmoId가 안정적으로 식별되는가
- InitialSortieAmmoCount가 MaximumLoadableAmmoCount를 넘지 않는가
- 무기 초기 장전량을 충족할 수 있는가
- 동일 AmmoId 출격 입력이 중복되지 않는가
- AmmoMassKg = Σ(InitialSortieAmmoCount × UnitMassKg)
```

---

## 4. Runtime 소유 구조

`UCFVehicleAmmoComp`가 차량 한 대의 실제 출격 탄약 상태를 단일 소유한다.

### 4.1 무기별 장전량

```text
WeaponInstanceId → FCFWeaponAmmoRuntime
```

P0에서 `WeaponInstanceId`는 실제 장착 위치를 안정적으로 구분하는 `MountProfileId`를 사용한다.

같은 WeaponData 또는 같은 AmmoData를 사용하는 무기가 여러 개 있어도 `LoadedAmmoCount`는 WeaponInstanceId별로 독립이다.

### 4.2 탄종별 차량 예비량

```text
AmmoId → ReserveAmmoCount
```

같은 탄종을 사용하는 여러 무기는 차량 예비 탄약을 공유한다.

초기화 시 전체 출격 수량에서 각 무기에 배정된 초기 장전량을 제외한 나머지가 해당 AmmoId의 Reserve가 된다.

### 4.3 Snapshot 계산값

`FCFAmmoRuntimeSnapshot`은 내부 Map을 UI나 Debug가 직접 읽지 않도록 계산 완료 상태를 제공한다.

주요 값:

```text
MagazineCapacity
LoadedAmmoCount
ReservedSequenceAmmoCount
ReservedFireTransactionAmmoCount
ReserveAmmoCount
ImmediateUsableAmmoCount
CurrentUsableAmmoCount
CurrentOnboardAmmoCount
ReloadState
ReloadDurationSeconds
RemainingReloadTimeSeconds
PendingReloadAmount
bWeaponActionLocked
WeaponActionLockReason
```

의미 구분:

```text
LoadedAmmoCount
= 현재 무기 탄창에 실제 존재하는 수량

ImmediateUsableAmmoCount
= 현재 예약되지 않아 새 발사 행동에 즉시 사용할 수 있는 장전량

CurrentUsableAmmoCount
= 새 행동 관점에서 사용할 수 있는 장전+예비량

CurrentOnboardAmmoCount
= 같은 AmmoId의 모든 무기 장전량 + 차량 Reserve의 실제 총 보유량
```

`ImmediateUsableAmmoCount`와 `CurrentUsableAmmoCount`는 발사 가능·예약·회귀 판정용 내부 값이다. 현재 WeaponPanel Primary Ammo 숫자로 직접 표시하지 않는다.

---

## 5. SingleCycle 발사 Transaction

유한탄 SingleCycle은 실행 성공과 탄약 소비를 분리한다.

```text
ValidateSingleFireAmmo
→ ReserveSingleFireAmmo
→ 실제 HitScan / Projectile 실행
→ 성공: CommitSingleFireAmmo
→ 실패: RollbackSingleFireAmmo
```

핵심 계약:

```text
- 검증 단계 거부는 탄약을 예약하지 않는다.
- 실행 전에 필요한 탄약을 임시 예약한다.
- 실제 발사가 성립했을 때만 LoadedAmmoCount에서 정확히 한 번 소비한다.
- Projectile 실행 실패는 소비하지 않고 예약만 반환한다.
- Trace Miss는 실제 발사가 실행된 것이므로 탄약을 소비한다.
- Direct Projectile fallback HitScan이 정상 실행되면 소비한다.
```

이 구조는 발사 실행 실패와 실제 Miss를 같은 것으로 취급하지 않는다.

---

## 6. Ripple·Salvo Launcher 예약

Ripple·Salvo는 첫 Projectile 실행 전에 시퀀스 전체 유효 발수를 예약한다.

```text
현재 자유 장전량
+ RequestedShotCount
+ bAllowPartialSequence
→ ReserveLauncherSequenceAmmo
→ 실제 실행 가능한 ReservedShotCount 확정
```

예를 들어 4발 Ripple 요청인데 Loaded가 3이고 부분 시퀀스를 허용하면 실제 Sequence는 3발로 축소된다.

핵심 계약:

```text
- 예약 시점에는 LoadedAmmoCount를 차감하지 않는다.
- 예약된 수량은 ImmediateUsableAmmoCount에서 제외한다.
- 실제 성공한 내부 발사마다 CommitReservedLauncherShot으로 한 발씩 소비한다.
- 실패한 내부 발사는 ReleaseReservedLauncherShot으로 소비 없이 예약만 반환한다.
- Sequence Active 동안 LauncherSequenceActive Action Lock을 유지한다.
- 추가 Sequence 또는 Reload 요청의 거부가 기존 Sequence를 취소하지 않는다.
- Completed·Cancelled·Scheduler 시작 실패에서 미실행 예약을 전부 반환한다.
- Terminal 정리 후 남은 ReservedSequenceAmmoCount는 0이어야 한다.
```

`ACFVehiclePawn`이 첫 발 전에 전체 예약을 만들고 첫 발 성공을 Commit한 뒤, `UCFLauncherComp`에 남은 예약의 WeaponInstanceId를 인계한다. 이후 후속 Ripple·Salvo 발사의 성공·실패와 Terminal 정리를 LauncherComp와 AmmoComp가 함께 처리한다.

---

## 7. Reload Runtime

P0 실제 실행 방식은 `FullMagazine`이다. `PerRound` enum과 데이터 계약은 존재하지만 현재 Runtime 실행은 지원하지 않는다.

### 7.1 Reload 시작

Reload 시작 시 수량을 즉시 이동하지 않는다.

```text
Loaded 유지
Reserve 유지
PendingReloadAmount 계산
ReloadState = Reloading
WeaponActionLockReason = Reloading
```

Reload 중 신규 발사는 `Reloading`으로 거부한다.

### 7.2 Reload 완료

완료 순간 실제 Reserve와 탄창 부족량을 다시 계산해 필요한 수량만 이동한다.

```text
Reserve → Loaded
CurrentOnboardAmmoCount 총량 보존
```

예비 탄약이 부족하고 `bAllowPartialReload=true`이면 가능한 만큼만 이동한다.

### 7.3 자동 Reload

`bAutoReloadWhenEmpty=true`인 유한탄 무기는 탄창이 최소 발사 요구량보다 부족하고 Reserve가 있으면 자동 Reload를 시도한다.

Launcher Sequence가 마지막 탄을 소비한 경우에도 Sequence Terminal에서 Launcher Action Lock을 먼저 해제한 뒤 자동 Reload로 전환한다.

### 7.4 Pause

Reload는 일반 ActorComponent Tick을 사용하고 `TickEvenWhenPaused=false`다. 따라서 싱글플레이 World Pause 동안 Reload 시간이 진행되지 않는다.

---

## 8. 행동 잠금

현재 대표 Action Lock 사유:

```text
None
LauncherSequenceActive
Reloading
WeaponDisabled
VehicleDestroyed
EquipmentChanging
```

현재 실제 P0에서 검증된 핵심은 Launcher Sequence와 Reload 상호 배제다.

```text
Launcher Sequence Active
→ Reload 시작 거부
→ 기존 Sequence 유지

Reloading
→ 신규 Fire 거부
```

장비·탄종 교환 Runtime의 전체 사용자 흐름은 후속 Field Fitting/Inventory 기능에서 확장한다. AmmoComp의 잠금 enum은 그 연결 지점을 보존한다.

---

## 9. HUD 계약

현재 finite Ammo는 다음 경로로 Production HUD에 전달된다.

```text
UCFVehicleAmmoComp
→ FCFAmmoRuntimeSnapshot
→ UCFHUDDataProvider
→ FCFWeaponHUDData
→ UCFHUDPresenter
→ WBP_CFWeaponPanel
```

Widget이 Pawn 또는 AmmoComp를 직접 Cast해 현재 수량을 계산하지 않는다.

### 9.1 기본 탄약 표시

현재 WeaponPanel 기본 계약:

```text
Primary: LoadedAmmoCount / MagazineCapacity
Reserve: 우상단 label-less ReserveAmmoCount
```

예:

```text
5 / 10     Reserve 10
0 / 10     Reserve 0
```

실제 UI에서는 Reserve에 `RESERVE` 같은 라벨을 붙이지 않고 작은 숫자만 표시한다.

KnownZero인 Reserve `0`도 숨기지 않는다.

### 9.2 표시하지 않는 값

다음은 현재 Primary Ammo 숫자가 아니다.

```text
ImmediateUsableAmmoCount
CurrentUsableAmmoCount
MaximumLoadableAmmoCount
```

특히 `MaximumLoadableAmmoCount`는 피팅 상한이므로 HUD 현재 탄약으로 사용하면 안 된다.

### 9.3 Reload와 Launcher 표시 우선순위

WeaponPanel 상태는 현재 다음 의미 우선순위를 사용한다.

```text
Launcher Sequence Active
→ RIPPLE/SALVO N / Total + Progress

Sequence가 아니고 Reloading
→ RELOAD Remaining / Duration

총 보유량 0
→ NO AMMO

그 외
→ Cooldown / READY
```

정상 Ripple·Salvo Sequence는 전역 AlertFeed가 아니라 WeaponPanel이 소유한다.

---

## 10. 출격 질량과 Fitting 연결

탄약 질량은 출격 Snapshot에서 계산한다.

```text
AmmoMassKg
= Σ(InitialSortieAmmoCount × AmmoData.UnitMassKg)
```

이 값은 차량 피팅 총중량에 포함된다.

검증용 저장 체인 기준:

```text
Heavy
InitialSortieAmmoCount = 15
UnitMassKg = 2 kg
AmmoMassKg = 30 kg
TotalVehicleMassKg = 1500 kg

Ripple Rocket
InitialSortieAmmoCount = 7
UnitMassKg = 5 kg
AmmoMassKg = 35 kg
TotalVehicleMassKg = 1505 kg
```

인게임에서 탄약을 발사했다고 차량 Chaos 질량을 매 발 실시간 재계산하는 기능은 현재 P0 범위가 아니다. P0 질량은 **출격 초기 피팅 질량**이다.

---

## 11. 호환성

현재 시스템은 기존 콘텐츠를 강제 변환하지 않는다.

```text
DefaultAmmoData = None
또는
bUseInfiniteAmmoForDebug = true
```

이면 기존 무한탄 호환 경로를 유지한다.

기존 다음 필드는 삭제하거나 리네이밍하지 않았다.

```text
MagazineSize
ReloadTimeSeconds
AmmoTypeId
```

`MagazineSize`와 `ReloadTimeSeconds`는 신규 Runtime의 호환 입력으로 계속 사용한다.

ProjectileData와 DamageData 소유권도 AmmoData로 이동하지 않았다. Ammo는 **수량·예약·재장전**을 소유하고, Projectile·Damage는 기존 전투 시스템 책임을 유지한다.

---

## 12. 검증 완료 기준

### 12.1 공식 빌드·자동화

최종 제품 코드 검증 기준:

```text
Official UE 5.8 Editor Build
- 2230e755e05844d1b1f7c0729e226d3a
- Compile / lib / DLL / Metadata
- Exit 0

Full Regression
- 4c68c30619554fd8985a83bfe06c5e04
- Required Ammo·Damage·Fitting·Launcher·Projectile·Missile 32/32 Success
- Total CarFight 64
- Failed 0
- AMMO_P0_01.Contract ~ AMMO_P0_08.AssetChain Success
```

후속 문서화 직전 공식 빌드 `6b15b6ebd9b94262ac1a6b25d97bc381`도 Target up-to-date / Exit 0으로 확인됐다.

### 12.2 Heavy USER PIE

`/Game/Maps/TestMap_AmmoHeavy`

```text
시작: 5 / 10 + Reserve 10
실제 발사마다 Loaded 감소
빈 탄창 자동 Reload 2초
Reload 완료 시 Reserve→Loaded 이동
완전 소진: 0 / 10 + Reserve 0
Reserve KnownZero `0` 가시성
```

전 항목 USER PASS.

### 12.3 Ripple USER PIE

`/Game/Maps/TestMap_AmmoRipple`

```text
시작: 3 / 4 + Reserve 4
첫 Trigger: 4발 요청 → 실제 3발 Partial Ripple
WeaponPanel: RIPPLE N / 3
Commit: Loaded 3 → 2 → 1 → 0
Reload 전 Reserve 4 유지
Sequence Terminal → 2초 Auto Reload
Reload 완료: 4 / 4 + Reserve 0
두 번째 Trigger: 실제 4발 Ripple
최종: 0 / 4 + Reserve 0 + NO AMMO
정상 Sequence는 AlertFeed에 표시하지 않음
```

2026-08-13 15:15 KST 사용자 확인으로 전 항목 USER PASS.

따라서 `AMMO-P0-08 = Done`, `CF-FQ-031 = Done`이며 이 문서를 Current System으로 승격한다.

---

## 13. 현재 범위 밖

다음 항목은 CF-FQ-031 완료를 의미하지 않는다.

```text
- PerRound 실제 재장전
- 수동 Reload 입력 매핑
- 탄종 교환 UI와 실제 교환 Runtime
- Field Fitting 중 탄약 적재 변경
- 인게임 실시간 탄약 질량에 따른 Chaos 질량 재계산
- 보급·정비·상점·가격·경제
- 영구 저장·SaveGame
- 서버 권한 탄약 소유권
- 멀티플레이 복제·예측·보정
- AI 탄약 정책
```

이 항목들은 필요한 상위 기능이 착수될 때 별도 Plan에서 확장한다.

---

## 14. 관련 Current System

```text
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/Projectile.md
Document/Systems/Combat/HitDamage.md
Document/Systems/Combat/VehicleDefense.md
Document/Systems/UI/AimReticle.md
Document/Systems/UI/VehicleDebugPanel.md
Document/Systems/Vehicles/VehicleRuntime.md
```

Launcher 자체는 아직 `CF-FQ-029 LM-P0-06` 체크포인트가 남아 있으므로 Launcher 전체를 이 Ammo 완료와 함께 자동으로 Done 처리하지 않는다.

---

## 15. Changelog

### v1.0.0 - 2026-08-13

- `CF-FQ-031`의 Ammo Data, Vehicle Ammo Runtime, SingleCycle Transaction, Launcher Sequence 예약, FullMagazine Reload, HUD와 Fitting Mass 연결을 Current System으로 신규 승격했다.
- WeaponInstanceId별 Loaded 독립, AmmoId별 Reserve 공유와 출격 `InitialSortieAmmoLoads` 단일 원본 계약을 기록했다.
- WeaponPanel의 `Loaded / MagazineCapacity + label-less Reserve` 계약과 KnownZero `0` 표시를 기록했다.
- Heavy와 Ripple 격리 Production USER PIE 전 항목 PASS를 완료 근거로 기록했다.
- `AMMO-P0-08`과 `CF-FQ-031`을 Done으로 판정했다.

---

## 16. Migration

- CF-FQ-031의 현재 구현 판단은 이 문서와 실제 코드를 우선한다.
- `Document/Plan/Archive/AmmoSystem/AmmoSystemPlan.md`는 완료 당시 구현·검증 체크포인트를 보존하는 Historical + Archived Path이며 Current System을 대체하지 않는다.
- 기존 무한탄 WeaponData는 명시적인 finite Ammo 설정이 없으면 계속 기존 호환 동작을 유지한다.
- `MaximumLoadableAmmoCount`를 인게임 현재 탄약으로 사용하지 않는다.
- Launcher 전체 기능의 완료 여부는 별도 `CF-FQ-029` 체크포인트를 따른다.
