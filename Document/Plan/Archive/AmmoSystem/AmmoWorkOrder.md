# AMMO-P0-01 Work Order

- Version: 0.1.0
- Date: 2026-07-29
- Status: Ready for Current AI Direct Implementation
- Feature: `CF-FQ-031` 차량 탄약·재장전 런타임
- Task: `AMMO-P0-01 Ammo Data Contract Foundation`

---

## 1. 실행 목표

기존 발사 결과와 에셋을 변경하지 않고 `UCFAmmoData`, 공용 Ammo 타입과 `UCFWeaponData`의 정적 탄약 설정을 추가한다.

이번 WorkOrder는 AMMO-P0-01 하나만 수행한다.
차량 Runtime, 발사 소비, Launcher 예약, 재장전과 HUD를 구현하지 않는다.

---

## 2. 실행 전 필수 확인

```text
1. main_game Git branch / upstream / dirty 확인
2. plan_repo Git branch / upstream / dirty 확인
3. AGENTS.md 확인
4. Document/CodeWorkGate.md 확인
5. AmmoSystemPlan.md 확인
6. AmmoSystemDesign.md 확인
7. AmmoTaskSource.md 확인
8. CFWeaponData.h / .cpp 현재 버전과 diff 확인
9. CFLauncherTypes와 현재 Launcher dirty가 AMMO-P0-01 범위에 포함되지 않는지 확인
```

기존 dirty가 같은 WeaponData 영역에 있으면 전체 파일 교체를 금지한다.
정확한 최소 패치가 불가능하면 작업을 중단하고 충돌 범위를 보고한다.

---

## 3. Step 1 — Ammo 공용 타입

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFAmmoTypes.h
```

첫 버전:

```text
v1.0.0
```

작업:

```text
- 파일 Header와 Scope
- ECFWeaponReloadMode
- ECFWeaponReloadState
- ECFWeaponActionLockReason
- ECFAmmoTransactionResult
- FCFWeaponAmmoRuntime
- FCFAmmoRuntimeSnapshot
- 안전한 기본값
- 필요한 계산 helper
- 변수·함수 위 한 줄 역할 주석
- Blueprint DisplayName·ToolTip
- Changelog와 Migration
```

주의:

```text
- 실제 상태 Tick을 추가하지 않는다.
- TMap 소유와 Vehicle Component를 만들지 않는다.
- Object 참조는 전방 선언과 TObjectPtr 규칙을 따른다.
- Runtime 구조체가 불필요하게 에셋을 강한 참조하지 않게 검토한다.
```

---

## 4. Step 2 — AmmoData

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Private/CFAmmoData.cpp
```

첫 버전:

```text
v1.0.0
```

필드:

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

함수:

```text
GetEffectiveUnitMassKg
GetEffectiveMaximumLoadableAmmoCount
BuildAmmoSummary
```

유효값:

```text
UnitMassKg >= 0
MaximumLoadableAmmoCount >= 0
```

`AmmoIcon` 타입은 현재 모듈 의존성을 불필요하게 늘리지 않는 UE 자산 타입을 선택한다.
UI 타입 include가 Runtime 모듈에 과도한 의존을 만들면 아이콘 필드를 후속 Task로 미룰 수 있으며, 그 경우 근거를 문서에 기록한다.

---

## 5. Step 3 — WeaponData 정적 설정

수정 후보:

```text
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Private/CFWeaponData.cpp
```

버전:

```text
현재 버전에서 Minor 증가
```

추가 후보:

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

함수 후보:

```text
GetEffectiveMagazineCapacity
GetEffectiveInitialLoadedAmmoCount
GetEffectiveAmmoUnitsPerShot
BuildAmmoConfigSummary
```

호환 규칙:

```text
- MagazineSize 삭제 금지
- ReloadTimeSeconds 삭제 금지
- AmmoTypeId 삭제 금지
- InitialLoadedAmmoCount 기본 -1
- -1은 MagazineSize 전체 장전 의미
- AmmoUnitsPerShot 최소 1
- 초기 장전량은 MagazineSize 안으로 Clamp
- MagazineSize 0은 아직 탄약 Runtime 비활성 호환 의미
- DefaultAmmoData None 안전
```

`BuildWeaponSummary` 수정은 기존 출력 정보를 제거하지 않는 최소 추가만 허용한다.

---

## 6. Step 4 — Contract Automation

신규 파일:

```text
UE/Source/CarFight_Re/Private/CFAmmoContractTests.cpp
```

테스트 이름:

```text
CarFight.Ammo.AMMO_P0_01.Contract
```

검증:

```text
1. Ammo enum 기본값과 문자열 변환
2. Runtime·Snapshot 기본 수량 0
3. UnitMassKg 음수 보정
4. MaximumLoadableAmmoCount 음수 보정
5. AmmoUnitsPerShot 최소 1
6. InitialLoadedAmmoCount -1 해석
7. 초기 장전량 최대치 Clamp
8. MagazineSize 0 호환
9. DefaultAmmoData None 안전
10. AmmoTypeId 보존
11. Launcher Pattern과 ProjectileData 기존 기본값 보존
```

테스트가 실제 `.uasset` 저장을 요구하지 않게 한다.

---

## 7. 변경하지 않을 항목

```text
- CFVehiclePawn
- CFVehicleWeaponComp
- CFLauncherComp
- CFLauncherTypes
- CFProjectileActor
- CFProjectilePoolComp
- CFVehicleAimTypes
- FireFeedback UI
- TargetSelect
- Damage / Health
- Vehicle Runtime 질량
- 모든 .uasset
- Config
- Build scripts
```

컴파일을 위해 Build.cs 수정이 정말 필요하면 이유와 추가 모듈 의존성을 먼저 검토한다.
불필요한 UI 모듈 의존성을 추가하지 않는다.

---

## 8. 코드 품질 체크

```text
[ ] 신규 파일명 32자 이하
[ ] 파일 Header Version / Date / Description / Scope
[ ] 모든 변수 위 역할 주석
[ ] 모든 함수 위 역할 주석
[ ] 직관적인 이름
[ ] Blueprint DisplayName·ToolTip 한글
[ ] Null·음수·Clamp 안전
[ ] 기존 필드 유지
[ ] Changelog
[ ] Migration
[ ] Tick 없음
[ ] 매 프레임 할당 없음
```

---

## 9. Diff 검수

검수 대상:

```text
UE/Source/CarFight_Re/Public/CFAmmoTypes.h
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Private/CFAmmoData.cpp
UE/Source/CarFight_Re/Private/CFAmmoContractTests.cpp
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Private/CFWeaponData.cpp
```

확인:

```text
- 범위 밖 파일 변경 없음
- 기존 dirty 보존
- 기존 WeaponData 필드 삭제 없음
- Launcher·Projectile·Damage 코드 변경 없음
- 전체 파일 교체가 아닌 최소 수정
- 주석·버전 규칙 준수
```

---

## 10. 공식 빌드

반드시 다음 스크립트를 사용한다.

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

PASS 조건:

```text
- UHT
- CarFight_ReEditor Win64 Development
- 신규 Ammo 타입 컴파일
- 신규 Automation 소스 컴파일
- Link
- Exit Code 0
```

다른 엔진 경로를 사용하지 않는다.

---

## 11. Automation

실행 도구가 노출되어 있으면 다음을 실행한다.

```text
CarFight.Ammo.AMMO_P0_01.Contract
```

Runner가 없으면 다음처럼 구분한다.

```text
Automation Source Compile PASS
Execution Not Run — Runner Unavailable
```

실행하지 않은 테스트를 PASS로 기록하지 않는다.

---

## 12. PIE

AMMO-P0-01은 정적 데이터 계약 단계이므로 사용자 PIE를 종료 필수 조건으로 요구하지 않는다.

다만 빌드 후 기존 기준 에셋을 열거나 PIE를 수행했다면 다음 회귀만 관찰한다.

```text
- Heavy Cannon 기존 발사 가능
- RocketLauncher 기존 Ripple 가능
- 기존 WeaponData에 AmmoData가 없어도 NoAmmo가 되지 않음
```

실제 탄약 감소는 아직 발생하면 안 된다.

---

## 13. 완료 보고 형식

```text
Implementation Source: Current AI Direct
Feature: CF-FQ-031
Task: AMMO-P0-01
Source Changes: Applied
Changed Files: [목록]
Diff Review: PASS / FAIL
Build: PASS / FAIL / Job ID
Automation: PASS / Not Run
PIE: Not Required / Optional Regression
Current Behavior: Ammo Runtime Not Connected
Next Task: AMMO-P0-02 Vehicle Ammo Runtime
```

---

## 14. 문서 동기화

AMMO-P0-01 완료 후 최소 갱신:

```text
Document/Plan/Archive/AmmoSystemPlan.md
Document/Plan/AmmoSystemRoadmap.md
```

실제 Active 작업으로 전환됐다면 `Document/ActiveWork.md`와 필요한 ProjectSSOT를 현재 상태에 맞게 갱신한다.
정적 계약만 완료한 상태에서는 Systems 승격을 하지 않는다.

---

## 15. Stop Conditions

```text
- CFWeaponData 현재 dirty와 정확한 최소 패치 충돌
- 기존 저장 필드를 삭제해야만 구현 가능
- AmmoData 아이콘 때문에 과도한 모듈 의존성이 필요
- UHT가 Runtime 구조체의 참조 계약을 허용하지 않음
- 신규 기본값이 기존 무기를 NoAmmo로 변경
- AMMO-P0-02 이상의 Runtime을 같이 넣어야만 테스트 가능
- 공식 빌드 도구가 반복 실패
```

중단 시 추측으로 범위를 확장하지 않는다.

---

## 16. Changelog

### v0.1.0 - 2026-07-29

```text
- AMMO-P0-01 파일별 구현 순서를 작성했다.
- Ammo 타입, AmmoData와 WeaponData 정적 설정의 호환 규칙을 고정했다.
- 변경 금지 범위, 공식 빌드, Automation과 완료 보고 형식을 정의했다.
```

---

## 17. Migration

```text
- 이 WorkOrder는 AMMO-P0-01에만 사용한다.
- 차량 Ammo Component, 발사 소비, Launcher 예약과 Reload는 별도 WorkOrder를 작성한다.
- 기존 WeaponData 필드 삭제·리네이밍은 이 작업 범위가 아니다.
- AMMO-P0-01 완료 후에도 기존 무기는 탄약 비제한 호환 상태를 유지한다.
