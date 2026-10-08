# Vehicle Ammo System Design

- Version: 0.1.0
- Date: 2026-07-29
- Status: Ready Design / Source Not Modified
- Feature: `CF-FQ-031` 차량 탄약·재장전 런타임
- Representative Historical Plan: `Document/Plan/Archive/AmmoSystemPlan.md`
- Roadmap: `Document/Plan/AmmoSystemRoadmap.md`
- First TaskSource: `Document/Plan/AmmoTaskSource.md`
- First WorkOrder: `Document/Plan/AmmoWorkOrder.md`

---

## 1. 목적

CarFight 차량이 현재 출격에 실제로 싣고 나온 탄약을 무기별 장전 상태와 차량 탄약고 예비량으로 관리하고, 발사·런처 시퀀스·재장전·무기 교환·인게임 HUD·피팅 중량 계산이 같은 기준을 사용하도록 한다.

이 설계는 다음 질문에 하나의 답을 제공해야 한다.

```text
- 현재 무기에 즉시 발사 가능한 탄약은 몇 발인가?
- 현재 출전 차량에서 앞으로 자유롭게 사용할 수 있는 탄약은 몇 발인가?
- 진행 중인 Ripple·Salvo에 이미 예약된 탄약은 몇 발인가?
- 이번 출격에서 실제 발사한 탄약은 몇 발인가?
- 차량에 남아 있는 실제 전체 탄약은 몇 발인가?
- 재장전이나 무기 교환이 지금 가능한가?
```

---

## 2. 현재 코드 기준

현재 저장소에는 탄약 시스템을 연결할 기반이 일부 존재한다.

```text
UCFWeaponData
- MagazineSize
- ReloadTimeSeconds
- AmmoTypeId

ECFVehicleFireRejectReason
- NoAmmo

ECFVehicleReticleState
- Reloading

UCFLauncherComp
- SingleCycle
- Ripple
- Salvo
- 발사 시퀀스 상태와 성공·실패 집계

ACFVehiclePawn
- ValidateFireCommandInternal
- ExecuteAcceptedFireCommand
- ApplyFireResultInternal

UCFVehicleWeaponComp
- 활성 WeaponData
- 발사 쿨다운
- Muzzle 순환
```

현재 `MagazineSize`, `ReloadTimeSeconds`, `AmmoTypeId`, `NoAmmo`, `Reloading`은 데이터 또는 상태 후보일 뿐 실제 탄약 소비와 재장전 런타임에는 연결되지 않았다.

---

## 3. 확정된 최상위 결정

### 3.1 인게임 수치는 현재 출격 상태만 표시한다

인게임 HUD에서 표시하는 탄약 수치는 피팅 한도나 최대 적재 가능량이 아니다.

```text
인게임 탄약 수치
= 현재 출전한 차량에서 실제로 사용할 수 있는 현재 값
```

피팅 화면에서만 아래 값을 비교한다.

```text
현재 선택 적재량 / 최대 적재 가능량
```

### 3.2 피팅 한도와 현재 사용 가능량을 분리한다

```text
MaximumLoadableAmmoCount
= 피팅 단계에서 적재할 수 있는 최대 한도

InitialSortieAmmoCount
= 출격 시작 시 실제로 적재한 수량

CurrentUsableAmmoCount
= 현재 시점에 신규 행동으로 자유롭게 사용할 수 있는 탄약량
```

`MaximumLoadableAmmoCount`는 기본 인게임 HUD에 표시하지 않는다.

### 3.3 차량 예비 탄약과 무기 장전 탄약을 분리한다

```text
차량 탄약고
= 탄종별 ReserveAmmoCount

장착 무기
= WeaponInstanceId별 LoadedAmmoCount
```

동일 탄종을 사용하는 여러 무기는 차량 예비 탄약 풀을 공유할 수 있지만, 장전 탄약은 각 무기 인스턴스가 독립적으로 가진다.

### 3.4 런처 순차 사격 중 행동을 잠근다

`UCFLauncherComp`의 발사 시퀀스가 `Active`인 동안 다음 행동을 허용하지 않는다.

```text
- 수동 재장전
- 자동 재장전 시작
- 활성 무기 교환
- 무기 그룹 교환
- 탄종 교환
- EquipmentPreset 교환
- 신규 발사 시퀀스 시작
```

거부된 재장전이나 무기 교환 요청이 현재 발사 시퀀스를 취소해서는 안 된다.

### 3.5 탄약은 정상 발사 실행 기준으로 소비한다

```text
명중 여부
≠
탄약 소비 여부
```

정상적으로 발사된 HitScan 또는 Projectile은 빗나가거나 요격되어도 탄약을 소비한다.
발사 검증 거부 또는 최종 실행 실패는 탄약을 소비하지 않는다.

---

## 4. 용어와 수량 계약

| 용어 | 권장 변수명 | 의미 | 기본 표시 위치 |
|---|---|---|---|
| 최대 적재 가능 탄약량 | `MaximumLoadableAmmoCount` | 피팅에서 선택 가능한 최대 탄수 | 피팅 화면 |
| 출격 시작 탄약량 | `InitialSortieAmmoCount` | 출격 시작 시 실제 적재한 탄수 | 결과·Debug |
| 최대 장전량 | `MagazineCapacity` | 무기 장전부에 들어가는 최대 탄수 | 상세 HUD·Data |
| 현재 장전량 | `LoadedAmmoCount` | 현재 무기에 물리적으로 장전된 탄수 | 인게임 HUD |
| 시퀀스 예약량 | `ReservedSequenceAmmoCount` | 진행 중 발사 시퀀스에 예약된 장전 탄수 | 시퀀스 HUD·Debug |
| 즉시 자유 사용 가능량 | `ImmediateUsableAmmoCount` | 신규 발사 행동에 사용할 수 있는 현재 장전 탄수 | 인게임 HUD |
| 예비 탄약량 | `ReserveAmmoCount` | 차량 탄약고에 남아 있는 호환 탄수 | 상세 HUD |
| 현재 사용 가능 탄약량 | `CurrentUsableAmmoCount` | 신규 행동으로 자유롭게 사용할 수 있는 장전+예비 탄수 | 인게임 HUD |
| 현재 차량 보유 탄약량 | `CurrentOnboardAmmoCount` | 예약 여부와 무관하게 차량에 실제 남은 장전+예비 탄수 | Debug·상세 자원창 |
| 현재 시퀀스 남은 발사 수 | `PendingSequenceShotCount` | 진행 중 시퀀스에서 앞으로 실행할 발사 횟수 | 시퀀스 HUD |
| 이번 시퀀스 발사량 | `FiredAmmoCountThisSequence` | 현재 시퀀스에서 실제 소비된 탄약 단위 | Debug |
| 이번 출격 발사량 | `FiredAmmoCountThisSortie` | 이번 출격에서 실제 소비된 탄약 단위 | 결과·Debug |

### 4.1 기본 계산식

```text
ImmediateUsableAmmoCount
= Max(LoadedAmmoCount - ReservedSequenceAmmoCount, 0)
```

```text
CurrentUsableAmmoCount
= ImmediateUsableAmmoCount + ReserveAmmoCount
```

```text
CurrentOnboardAmmoCount
= LoadedAmmoCount + ReserveAmmoCount
```

```text
ImmediateFireableShotCount
= ImmediateUsableAmmoCount / AmmoUnitsPerShot
```

```text
RemainingFreeFireableShotCount
= CurrentUsableAmmoCount / AmmoUnitsPerShot
```

정수 나눗셈을 사용하며, 한 번 발사에 필요한 탄약 단위를 충족하지 못하는 잔여 수량은 발사 가능 횟수에 포함하지 않는다.

### 4.2 수량 보존식

```text
CurrentOnboardAmmoCount
= InitialSortieAmmoCount
+ ResuppliedAmmoCountThisSortie
- FiredAmmoCountThisSortie
- DiscardedAmmoCountThisSortie
- LostAmmoCountThisSortie
```

재장전은 차량 내부 위치 이동이므로 `CurrentOnboardAmmoCount`를 변경하지 않는다.

---

## 5. 식별자와 데이터 소유권

### 5.1 WeaponId만으로 런타임을 식별하지 않는다

동일한 `WeaponData`를 사용하는 무기가 여러 개 장착될 수 있으므로 장전 상태 키는 `WeaponInstanceId`를 사용한다.

P0 권장 기본값:

```text
WeaponInstanceId = MountProfileId
```

향후 하나의 MountProfile에 여러 무기 인스턴스가 들어가면 장착 슬롯과 인스턴스 번호를 조합한다.

### 5.2 권장 소유 구조

```text
ACFVehiclePawn
├─ UCFVehicleWeaponComp
│  └─ 활성 무기·장착·발사 원점·쿨다운 해석
├─ UCFLauncherComp
│  └─ Ripple·Salvo 시퀀스와 발사 예약 진행
└─ UCFVehicleAmmoComp
   ├─ 탄종별 차량 예비 탄약
   ├─ WeaponInstanceId별 장전 상태
   ├─ 탄약 예약·소비·반환
   ├─ 재장전 상태
   ├─ 행동 잠금 조회
   ├─ 출격 통계
   └─ UI·Debug Snapshot
```

`UCFVehicleWeaponComp`에 모든 탄약 상태를 추가하지 않는다.
탄약 수량과 재장전 상태의 단일 소유자는 `UCFVehicleAmmoComp`다.

---

## 6. 신규 파일과 타입

모든 신규 파일명은 32자를 넘지 않는다.

```text
UE/Source/CarFight_Re/Public/CFAmmoTypes.h
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Private/CFAmmoData.cpp
UE/Source/CarFight_Re/Public/CFVehicleAmmoComp.h
UE/Source/CarFight_Re/Private/CFVehicleAmmoComp.cpp
UE/Source/CarFight_Re/Private/CFAmmoContractTests.cpp
UE/Source/CarFight_Re/Private/CFAmmoRuntimeTests.cpp
```

### 6.1 `UCFAmmoData`

권장 필드:

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

P0에서는 `ProjectileData`와 `DamageData`의 현재 소유 경로를 변경하지 않는다.

```text
WeaponData.DefaultProjectileData
→ ProjectileData.DefaultDamageData
```

`AmmoData`는 탄종 식별, 중량, 피팅 한도와 UI 메타데이터를 우선 소유한다.
탄종별 Projectile 교체는 후속 단계에서 별도 매핑 구조로 확장한다.

### 6.2 `UCFWeaponData` 탄약 설정

기존 저장 필드를 무분별하게 리네이밍하지 않는다.

```text
MagazineSize
ReloadTimeSeconds
AmmoTypeId
```

신규 권장 필드:

```text
DefaultAmmoData
InitialLoadedAmmoCount
AmmoUnitsPerShot
ReloadMode
bAutoReloadWhenEmpty
bAllowPartialReload
bAllowPartialSequence
bUseInfiniteAmmoForDebug
```

`MagazineSize`는 기존 에셋 호환을 위해 유지하며 런타임에서는 `MagazineCapacity` 의미로 해석하는 유효값 Getter를 제공한다.

### 6.3 공용 enum

```text
ECFWeaponReloadMode
- FullMagazine
- PerRound

ECFWeaponReloadState
- NotInitialized
- Ready
- Reloading
- Empty
- NoReserveAmmo
- Disabled

ECFWeaponActionLockReason
- None
- LauncherSequenceActive
- Reloading
- WeaponDisabled
- VehicleDestroyed
- EquipmentChanging

ECFAmmoTransactionResult
- Accepted
- MissingAmmoData
- MissingWeaponRuntime
- InvalidAmmoAmount
- NotEnoughLoadedAmmo
- SequenceAlreadyActive
- Reloading
- ActionLocked
- ExecutionFailed
```

P0 실제 재장전은 `FullMagazine`만 구현하고 `PerRound`는 데이터 계약 또는 후속 범위로 유지할 수 있다.

---

## 7. 런타임 구조체

### 7.1 무기별 탄약 런타임

```text
FCFWeaponAmmoRuntime
```

권장 필드:

```text
WeaponInstanceId
WeaponData
CurrentAmmoData
MagazineCapacity
LoadedAmmoCount
ReservedSequenceAmmoCount
ReloadState
ReloadElapsedSeconds
ReloadDurationSeconds
PendingReloadAmount
FiredAmmoCountThisSortie
FiredAmmoCountThisSequence
ReloadedAmmoCountThisSortie
NoAmmoRejectCount
```

### 7.2 UI·Debug Snapshot

```text
FCFAmmoRuntimeSnapshot
```

필수 필드:

```text
WeaponInstanceId
AmmoId
MagazineCapacity
LoadedAmmoCount
ReservedSequenceAmmoCount
ImmediateUsableAmmoCount
ReserveAmmoCount
CurrentUsableAmmoCount
CurrentOnboardAmmoCount
ImmediateFireableShotCount
RemainingFreeFireableShotCount
PendingSequenceShotCount
FiredAmmoCountThisSequence
FiredAmmoCountThisSortie
ReloadState
RemainingReloadTimeSeconds
bWeaponActionLocked
WeaponActionLockReason
```

UI는 내부 Map을 직접 읽지 않고 Snapshot 또는 변경 이벤트를 사용한다.

---

## 8. 인게임 표시 계약

### 8.1 일반 무기 HUD

기본 인게임 HUD는 현재 선택된 무기와 탄종 기준으로 표시한다.

```text
20 | 120
```

```text
왼쪽 20
= ImmediateUsableAmmoCount

오른쪽 120
= CurrentUsableAmmoCount
```

시퀀스 예약이 없는 일반 상태에서는 다음과 같다.

```text
LoadedAmmoCount = 20
ReservedSequenceAmmoCount = 0
ReserveAmmoCount = 100
ImmediateUsableAmmoCount = 20
CurrentUsableAmmoCount = 120
```

상세 HUD 또는 전술 자원창에서는 장전·예비·현재 보유를 분리할 수 있다.

```text
장전       20
예비      100
사용 가능 120
현재 보유 120
```

### 8.2 차량 전체 탄종 표시

서로 교환할 수 없는 탄종을 단순 합산한 숫자를 기본 HUD에 표시하지 않는다.

```text
30mm AP       80
30mm HE       40
IR Missile     6
HE Rocket     12
```

기본 전투 HUD는 현재 선택 무기와 현재 탄종만 크게 표시하고, 전체 탄종 목록은 상세 자원창에서 표시한다.

### 8.3 런처 순차 사격 중

순차 사격 중에는 일반 탄약 숫자보다 발사 시퀀스 상태를 우선 표시한다.

```text
RIPPLE 2 / 6
남은 발사 4
자유 사용 가능 4
```

예:

```text
LoadedAmmoCount = 4
ReservedSequenceAmmoCount = 4
ReserveAmmoCount = 4
ImmediateUsableAmmoCount = 0
CurrentUsableAmmoCount = 4
CurrentOnboardAmmoCount = 8
PendingSequenceShotCount = 4
```

진행 중 시퀀스에 예약된 4발은 차량에 남아 있지만 신규 행동에 자유롭게 사용할 수 없으므로 `CurrentUsableAmmoCount`에서는 제외한다.

### 8.4 피팅 화면

피팅 화면에서만 다음을 비교한다.

```text
현재 선택 적재량 / 최대 적재 가능량
탄약 중량
차량 총중량
예상 전체 발사 횟수
예상 완전 탄창 또는 Salvo 횟수
```

예:

```text
탄약 적재 120 / 180
탄약 중량 96kg
```

---

## 9. 발사 탄약 트랜잭션

발사 입력 순간 바로 영구 차감하지 않는다.

권장 흐름:

```text
1. Fire Command 생성
2. 조준·무기·행동 잠금 검증
3. 장전 탄약 존재 검증
4. 탄약 소비 예약
5. HitScan 또는 Projectile 실행
6. 실행 성공 시 소비 확정
7. 실행 실패 시 예약 반환
8. FireResult·UI·Debug 갱신
```

### 9.1 검증 위치

```text
ACFVehiclePawn::ValidateFireCommandInternal
→ 재장전·행동 잠금·장전 탄약 검증
```

### 9.2 실행 위치

```text
ACFVehiclePawn::ExecuteAcceptedFireCommand
→ 탄약 소비 예약
→ Projectile 또는 HitScan 실행
→ Commit 또는 Rollback
```

### 9.3 결과 반영 위치

```text
ACFVehiclePawn::ApplyFireResultInternal
→ UI·FX·Debug·이벤트 반영
```

`ApplyFireResultInternal`에서 처음 탄약을 차감하지 않는다.
Salvo가 같은 프레임에 여러 발을 처리할 때 중복 소비 검증이 늦어지는 것을 방지해야 한다.

### 9.4 탄약 소비 판정

| 상황 | 탄약 소비 |
|---|---|
| Projectile 정상 발사 후 빗나감 | 소비 |
| Projectile 정상 발사 후 요격 | 소비 |
| HitScan 정상 발사 후 Trace Miss | 소비 |
| 총구 막힘으로 검증 거부 | 소비 안 함 |
| 쿨다운으로 검증 거부 | 소비 안 함 |
| 재장전 중 거부 | 소비 안 함 |
| 장전 탄약 부족 | 소비 안 함 |
| Projectile 실행 실패 후 전체 취소 | 예약 반환 |
| Direct Projectile 실패 후 Dummy HitScan fallback 성공 | 소비 |
| Ripple 일부 성공 | 성공한 발수만 소비 |

---

## 10. 런처 시퀀스 예약과 행동 잠금

### 10.1 시퀀스 시작 전 유효 발수 결정

```text
RequestedProjectileCount
ImmediateFireableShotCount
bAllowPartialSequence
```

부분 시퀀스 허용 시:

```text
EffectiveProjectileCount
= Min(RequestedProjectileCount, ImmediateFireableShotCount)
```

부분 시퀀스 비허용 시 필요한 전체 발수를 충족하지 못하면 첫 발 전에 `NoAmmo`로 거부한다.

CarFight P0 기본값은 부분 시퀀스 허용이다.

### 10.2 예약 처리

시퀀스가 실제로 시작되기 전에 다음 수량을 예약한다.

```text
ReservedSequenceAmmoCount
= EffectiveProjectileCount * AmmoUnitsPerShot
```

첫 발을 포함한 전체 시퀀스 수량을 예약해야 한다.
예약 성공 전에는 시퀀스를 `Active`로 전환하지 않는다.

### 10.3 각 발사 성공

```text
LoadedAmmoCount -= AmmoUnitsPerShot
ReservedSequenceAmmoCount -= AmmoUnitsPerShot
FiredAmmoCountThisSequence += AmmoUnitsPerShot
FiredAmmoCountThisSortie += AmmoUnitsPerShot
```

### 10.4 한 발 실행 실패

`ContinueRemaining` 정책:

```text
ReservedSequenceAmmoCount -= AmmoUnitsPerShot
LoadedAmmoCount는 감소하지 않음
남은 발사 계속
```

`StopSequence` 정책:

```text
실패한 발과 미실행 발의 예약을 모두 해제
LoadedAmmoCount는 실제 성공 발사분만 감소
시퀀스 Cancelled
```

### 10.5 시퀀스 완료

```text
ReservedSequenceAmmoCount = 0
행동 잠금 해제
자동 재장전 조건 평가
무기 교환 허용
```

시퀀스 마지막 Projectile 결과가 기록되고 `Completed`가 확정된 뒤 잠금을 해제한다.

### 10.6 사용자 요청으로 시퀀스를 취소하지 않는다

다음 흐름은 금지한다.

```text
무기 교환 요청
→ 시퀀스 취소
→ 무기 교환
```

올바른 흐름:

```text
무기 교환 요청
→ LauncherSequenceActive로 거부
→ 기존 시퀀스 계속
```

### 10.7 시스템 강제 취소

다음 상황은 강제 취소를 허용한다.

```text
- 차량 파괴
- Owner 제거
- 무기 파괴 또는 비활성화
- 장비 데이터 무효화
- 월드 종료
- 런타임 실행 불가
```

강제 취소 시 이미 발사한 탄약은 소비 상태를 유지하고 미실행 예약만 해제한다.

---

## 11. 재장전

### 11.1 P0 기본 방식

```text
ReloadMode = FullMagazine
```

재장전 시작 시 계산:

```text
PendingReloadAmount
= Min(MagazineCapacity - LoadedAmmoCount, ReserveAmmoCount)
```

탄약 이동은 재장전 시작 순간이 아니라 완료 순간에 수행한다.

```text
LoadedAmmoCount += PendingReloadAmount
ReserveAmmoCount -= PendingReloadAmount
```

### 11.2 재장전 요청 검증 순서

```text
1. 차량과 무기 유효성
2. LauncherSequenceActive 여부
3. 이미 Reloading인지
4. MagazineCapacity보다 적게 장전됐는지
5. 호환 ReserveAmmoCount가 있는지
6. 재장전 시작
```

시퀀스가 `Active`이면 즉시 거부하고 현재 시퀀스는 계속 진행한다.

### 11.3 자동 재장전

기본값:

```text
bAutoReloadWhenEmpty = true
```

자동 재장전은 다음 조건을 모두 만족할 때 시작한다.

```text
LauncherSequenceState != Active
ReservedSequenceAmmoCount == 0
LoadedAmmoCount < AmmoUnitsPerShot
ReserveAmmoCount > 0
현재 무기 유효
```

시퀀스가 끝나는 프레임에는 `Completed` 또는 `Cancelled` 확정과 예약 해제를 먼저 완료한 뒤 자동 재장전을 평가한다.

### 11.4 쿨다운과 재장전

`FireRatePerMinute` 기반 쿨다운과 재장전 시간은 병렬 상태다.

```text
다음 발사 가능
= 쿨다운 완료
AND 재장전 완료
AND 행동 잠금 없음
AND 필요한 장전 탄약 존재
```

---

## 12. 무기와 탄종 교환

P0에서 실제 무기 교환 UI가 아직 없더라도 공용 검증 API는 행동 잠금 계약을 제공해야 한다.

```text
CanChangeWeapon
CanChangeWeaponGroup
CanChangeAmmoType
CanChangeEquipmentPreset
```

모든 함수는 동일한 `ECFWeaponActionLockReason`을 반환할 수 있어야 한다.

`LauncherSequenceActive`일 때 교환 요청은 거부하며 현재 시퀀스를 취소하지 않는다.
`Reloading` 중 무기 교환 정책은 후속 확장 가능하지만 P0 기본값은 거부다.

---

## 13. 이벤트와 UI 갱신

권장 이벤트:

```text
OnAmmoRuntimeChanged
OnWeaponReloadStateChanged
OnWeaponActionLockChanged
OnAmmoDepleted
OnReloadStarted
OnReloadCompleted
OnReloadCancelled
OnSequenceAmmoReserved
OnSequenceAmmoReleased
```

이벤트는 실제 상태 변경 때만 발생한다.
매 Tick 동일 Snapshot을 반복 브로드캐스트하지 않는다.

Blueprint 공개 함수와 변수에는 한국어 `DisplayName`과 `ToolTip`을 작성한다.

---

## 14. C++과 Blueprint 책임

### C++

```text
- 탄약 데이터 유효값 해석
- 탄종별 예비 탄약 Map
- 무기 인스턴스별 장전 상태
- 수량 계산
- 발사 예약·Commit·Rollback
- 런처 시퀀스 예약
- 행동 잠금
- 재장전 상태와 시간
- 이벤트와 Snapshot
- 자동화 테스트
```

### Blueprint / DataAsset

```text
- 실제 AmmoData 자산 값
- WeaponData 탄약 설정
- 출격 초기 적재량
- 무기 HUD 배치와 스타일
- 재장전 애니메이션과 시각 피드백
- LowAmmo·NoAmmo 표시
- 피팅 화면 수치 표시
```

게임 Audio는 프로젝트 정책상 구현하지 않는다.

---

## 15. 탄약 중량

```text
AmmoMassKg
= AmmoCount * UnitMassKg
```

```text
TotalAmmoMassKg
= 모든 장전 탄약 중량
+ 모든 예비 탄약 중량
```

장전과 예비를 중복 계산하지 않는다.

P0는 출격 시작 총중량에 반드시 반영한다.
런타임 질량 재적용은 탄종과 발사 빈도에 따라 배치 갱신할 수 있다.

```text
- 대구경 포탄·미사일: 발사마다 갱신 가능
- 소구경 연사: 누적 질량 또는 수량 임계값 이후 갱신 가능
```

정확한 Chaos 질량 재적용 정책은 `AMMO-P0-07`에서 현재 VehicleRuntime과 교차검증한다.

---

## 16. P0 제외 범위

```text
- 개별 탄창 아이템 인벤토리
- 약실과 탄창 분리
- 탄띠 물리
- 탄 걸림
- 탄피 회수
- 탄약 부피와 탄약고 공간 배치
- 탄약고 위치별 손상
- 탄약고 유폭
- 탄종 혼합 장전 순서
- 아군 탄약 전달
- 서버 권한과 복제
- 영구 경제와 구매 비용
```

---

## 17. 핵심 통합 검증

```text
- 정상 단발만 1회 소비
- 쿨다운·총구 막힘·정렬 실패는 소비 없음
- HitScan Miss는 소비
- Projectile 실행 실패는 Rollback
- Direct fallback 성공은 소비
- Ripple·Salvo는 성공한 각 Projectile만 소비
- 부분 Salvo는 현재 장전량만큼만 시작
- 시퀀스 중 재장전·무기 교환·탄종 교환 거부
- 거부 요청이 시퀀스를 취소하지 않음
- 시퀀스 완료 후 자동 재장전
- 재장전은 차량 전체 탄약량을 변경하지 않음
- 동일 탄종 무기 두 개의 장전량은 독립
- 예비 탄약은 공유
- 차량 파괴 취소 시 미발사 예약 반환
- Pool 재사용에서 이전 탄약 트랜잭션 잔류 없음
```

---

## 18. Changelog

### v0.1.0 - 2026-07-29

```text
- 현재 출격 차량의 사용 가능 탄약을 인게임 표시 기준으로 확정했다.
- 피팅 최대 적재량과 인게임 현재 사용 가능량을 분리했다.
- 차량 예비 탄약, 무기별 장전량과 시퀀스 예약량의 소유권을 정의했다.
- 런처 순차 사격 중 재장전·무기·탄종·장비 교환 금지를 잠갔다.
- 발사 탄약 트랜잭션, 재장전, UI Snapshot과 중량 연계를 설계했다.
```

---

## 19. Migration

```text
- 기존 UCFWeaponData.MagazineSize, ReloadTimeSeconds와 AmmoTypeId는 삭제하거나 즉시 리네이밍하지 않는다.
- AmmoTypeId는 DefaultAmmoData 전환 기간 동안 레거시 식별자와 Debug fallback으로 유지한다.
- 기존 WeaponFire는 탄약 컴포넌트가 없을 때 무한탄처럼 동작하는 임시 호환 경로를 단계적으로 유지할 수 있다.
- 호환 경로 제거는 모든 기준 차량에 Ammo Runtime 초기화와 사용자 PIE가 완료된 뒤 별도 결정한다.
- 기존 Launcher Sequence와 Projectile Pool의 성공·실패 판정 순서를 변경하지 않는다.
