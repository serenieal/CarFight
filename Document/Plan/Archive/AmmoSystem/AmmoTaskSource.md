# AMMO-P0-01 TaskSource

- Version: 0.1.0
- Date: 2026-07-29
- Status: Ready for Direct Implementation
- Feature: `CF-FQ-031` 차량 탄약·재장전 런타임
- Task: `AMMO-P0-01 Ammo Data Contract Foundation`
- Representative Historical Plan: `Document/Plan/Archive/AmmoSystemPlan.md`
- Design: `Document/Plan/AmmoSystemDesign.md`

---

## 1. 작업 목적

현재 WeaponFire, Launcher, Projectile, Damage와 사용자 에셋의 실제 발사 결과를 변경하지 않으면서 탄약 시스템이 사용할 정적 데이터와 공용 타입 계약을 추가한다.

```text
이번 Task
= AmmoData + Ammo 공용 타입 + WeaponData 정적 설정

이번 Task가 아닌 것
= 차량 탄약 수량 Runtime + 발사 소비 + 재장전 + HUD
```

---

## 2. 현재 코드 근거

```text
UCFWeaponData
- MagazineSize
- ReloadTimeSeconds
- AmmoTypeId
- LauncherFirePatternConfig
- DefaultProjectileData

ECFVehicleFireRejectReason
- NoAmmo

ECFVehicleReticleState
- Reloading

UCFLauncherComp
- Active Sequence Runtime

ACFVehiclePawn
- 현재 탄약 검증 없음
```

기존 `MagazineSize`, `ReloadTimeSeconds`, `AmmoTypeId`는 저장 에셋과 Debug 호환을 위해 유지해야 한다.

---

## 3. Locked Decisions

```text
- 인게임 수치는 현재 출격 차량의 현재 사용 가능량이다.
- 피팅 최대 적재 가능량은 별도 값이다.
- UCFVehicleAmmoComp가 후속 Runtime의 단일 소유자다.
- 장전 상태 키는 WeaponInstanceId이며 WeaponId만 사용하지 않는다.
- 차량 예비 탄약은 AmmoData별 공유다.
- 런처 Active 시퀀스 중 재장전·무기·탄종·장비 교환을 금지한다.
- 교환 거부가 시퀀스를 취소하지 않는다.
- 발사 성공 단위 Commit과 실패 Rollback을 후속 Task에서 사용한다.
- 기존 DamageData와 ProjectileData 소유 경로를 변경하지 않는다.
```

---

## 4. 이번 Task 범위

### 신규 파일

```text
UE/Source/CarFight_Re/Public/CFAmmoTypes.h
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Private/CFAmmoData.cpp
UE/Source/CarFight_Re/Private/CFAmmoContractTests.cpp
```

### 수정 후보

```text
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Private/CFWeaponData.cpp
```

실제 수정 전에 현재 diff와 파일 버전을 다시 읽는다.

---

## 5. `CFAmmoTypes.h` 계약

계획 enum:

```text
ECFWeaponReloadMode
ECFWeaponReloadState
ECFWeaponActionLockReason
ECFAmmoTransactionResult
```

계획 구조체:

```text
FCFWeaponAmmoRuntime
FCFAmmoRuntimeSnapshot
```

이번 Task에서는 구조체 기본값, 계산용 최소 helper와 Reflection 계약을 정의할 수 있다.
실제 Map 소유, Tick과 상태 전이는 `AMMO-P0-02` 이후다.

### 필수 기본값

```text
수량 = 0
인덱스 = INDEX_NONE 또는 안전한 기본값
ReloadState = NotInitialized
ActionLockReason = None
TransactionResult = Accepted 또는 안전한 미실행 기본값
Object 참조 = nullptr
Name = NAME_None
시간 = 0
```

음수 수량을 유효 상태로 저장하지 않는다.

---

## 6. `UCFAmmoData` 계약

필수 필드:

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

필수 함수 후보:

```text
GetEffectiveUnitMassKg
GetEffectiveMaximumLoadableAmmoCount
BuildAmmoSummary
```

Blueprint 공개 항목에는 한국어 `DisplayName`과 `ToolTip`을 작성한다.

### P0 비책임

```text
- ProjectileData 직접 소유
- DamageData 직접 소유
- 탄종별 탄도 배율
- 탄약고 위치
- 유폭
```

---

## 7. `UCFWeaponData` 확장

신규 설정 후보:

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

### 호환 규칙

```text
- MagazineSize는 유지한다.
- ReloadTimeSeconds는 유지한다.
- AmmoTypeId는 유지한다.
- DefaultAmmoData가 비어 있으면 AmmoTypeId를 Debug fallback으로 표시할 수 있다.
- InitialLoadedAmmoCount < 0이면 MagazineSize 전체 장전 의미로 해석한다.
- AmmoUnitsPerShot는 최소 1로 보정한다.
- MagazineSize == 0인 기존 에셋은 탄약 Runtime 연결 전까지 기존 무제한 발사 호환을 유지한다.
```

필수 Getter 후보:

```text
GetEffectiveMagazineCapacity
GetEffectiveInitialLoadedAmmoCount
GetEffectiveAmmoUnitsPerShot
BuildAmmoConfigSummary
```

기존 `BuildWeaponSummary`에 신규 값을 추가하더라도 기존 필드를 삭제하지 않는다.

---

## 8. 변경 금지 범위

```text
- CFVehiclePawn 발사 함수 변경 금지
- CFVehicleWeaponComp Runtime 변경 금지
- CFLauncherComp 변경 금지
- CFLauncherTypes 변경 금지
- CFProjectileActor·Pool 변경 금지
- Damage·Health 변경 금지
- TargetSelect 변경 금지
- UI 변경 금지
- .uasset 변경 금지
- Config 변경 금지
- 게임 Audio 추가 금지
```

include 또는 forward declaration이 필요한 최소 변경만 허용한다.

---

## 9. 코드 품질

```text
- 신규 파일명 32자 이하
- 파일 Header에 Version, Date, Description, Scope
- 모든 변수 바로 위 역할 주석
- 모든 함수 바로 위 역할 주석
- Blueprint DisplayName·ToolTip 한글
- 직관적인 변수와 함수명
- Changelog와 Migration
- Clamp와 Null 안전
- 기존 API 삭제 없음
- Tick 없음
- 매 프레임 할당 없음
```

---

## 10. Automation 계약

테스트 이름:

```text
CarFight.Ammo.AMMO_P0_01.Contract
```

검증:

```text
- AmmoData 기본값 안전
- UnitMassKg 음수 보정
- MaximumLoadableAmmoCount 음수 보정
- Ammo Runtime·Snapshot 기본 수량 0
- ActionLock 기본 None
- AmmoUnitsPerShot 0 또는 음수 입력은 1
- InitialLoadedAmmoCount -1은 MagazineSize로 해석
- InitialLoadedAmmoCount가 MagazineSize를 넘으면 Clamp
- MagazineSize 0 기존 호환 정책
- DefaultAmmoData None 안전
- AmmoTypeId 레거시 보존
- 기존 LauncherFirePatternConfig와 ProjectileData 참조 보존
```

Automation이 DataAsset 생성 없이 순수 타입과 CDO 기준으로 실행 가능하도록 구성한다.

---

## 11. 완료 조건

```text
- 신규 Ammo 타입과 DataAsset 컴파일
- WeaponData 정적 설정과 유효값 Getter 적용
- 기존 WeaponData 에셋 호환
- scoped Git diff 검수 PASS
- Tools\BuildEditor.bat PASS
- Automation PASS 또는 Runner Unavailable 명시
- 실제 발사 수량은 아직 변경되지 않음
- 대표 Plan과 Roadmap 체크포인트 갱신
```

---

## 12. 실패 조건

```text
- 기존 WeaponData 에셋 로드 실패
- 기존 Heavy Cannon 또는 RocketLauncher 발사 결과 변경
- MagazineSize·ReloadTimeSeconds·AmmoTypeId 삭제 또는 리네이밍
- ProjectileData·DamageData 소유권 변경
- .uasset 재저장 필요
- unrelated dirty 파일 변경
- AMMO-P0-02 이상의 Runtime 구현 혼입
```

---

## 13. 검증 후 보고 형식

```text
Implementation Source: Current AI Direct
Feature: CF-FQ-031
Task: AMMO-P0-01
Source Changes: Applied / Not Applied
Diff Review: PASS / FAIL
Build: PASS / FAIL
Automation: PASS / Not Run
PIE: Not Required for Contract / Pending
Next Task: AMMO-P0-02
```

---

## 14. Changelog

### v0.1.0 - 2026-07-29

```text
- AMMO-P0-01의 정적 데이터 계약 범위를 정의했다.
- 신규 Ammo 타입과 WeaponData 확장 후보를 잠갔다.
- 기존 발사 Runtime과 에셋을 변경하지 않는 보호 범위를 정의했다.
- Contract Automation과 완료·실패 조건을 작성했다.
```

---

## 15. Migration

```text
- 기존 WeaponData는 DefaultAmmoData가 없어도 로드 가능해야 한다.
- AmmoTypeId는 전환 기간 동안 삭제하지 않는다.
- 실제 탄약 수량 제한은 AMMO-P0-02~03 전까지 활성화하지 않는다.
- 신규 기본값 때문에 기존 무기가 NoAmmo가 되지 않도록 호환 경로를 유지한다.
