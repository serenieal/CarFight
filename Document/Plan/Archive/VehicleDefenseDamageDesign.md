# CarFight — 차량 방어·손상 런타임 설계

- 문서 버전: v0.23.1
- 작성일: 2026-07-30
- 최근 갱신일: 2026-08-22
- 문서 상태: Historical / Retained Path / `CF-FQ-033 Done` / `DR-P0-00~07` Done / `DR-PIE-00~06` USER PASS / Systems Current
- Feature: `CF-FQ-033 차량 방어·손상 런타임`
- 대표 단계: `완료 — VehicleDefense·HitDamage Current System 승격`
- 현재 구현 기준: `Document/Systems/Combat/VehicleDefense.md`, `Document/Systems/Combat/HitDamage.md`
- 장기 전투 방향: `Document/ProjectSSOT/CombatPlan/08_DefenseArmor.md`

---

## 1. 완료 당시 작업 체크포인트

```text
Feature: CF-FQ-033 차량 방어·손상 런타임
Priority: P0 / CarFight 작업 완료 우선순위 1위
Status: Done / DR-P0-00~07 Done / DR-PIE-00~06 USER PASS / Systems Current
Current Task: Completed — CF-FQ-033 Done / Current System 문서 승격 완료
Source Changes: Applied — Common Types / VehicleDefenseData / VehicleDefenseComp / VehicleHealth Integrity Compatibility / Pawn 기본 생성·초기화 / HitScan·Projectile 정식 피해 진입점 / 마지막 전체 피해 결과 캐시 / VehicleDebug Snapshot·Panel / 현재 선택 대상 Target·Defense·Integrity 동적 Panel 섹션 / 유효 DefenseData 초기화 시 VehicleDefenseComp 활성 복구 / Blueprint 이벤트·Debug Automation
Asset Changes: Applied — DA_VehicleDefense_Test / DA_VehicleDefense_TestSUV(DefaultDefenseData·DefaultDestroyedFxData=DA_FX_ProtoVehicleDead 연결) / DA_DamageArmorPenTest(BaseDamage 25·ArmorPenetration 50) / DRAP50 Projectile·Weapon·Preset·Sedan·Pawn·GameMode·M_VehicleDefensePIE_AP50 격리 체인 / M_VehicleDefensePIE_Legacy 원본 비저장 복제 맵(대상 DA_TestSUV·VehicleFittingData=None) / LauncherRegression Ripple Weapon·Preset·Salvo SUV·Ripple SUV·TestMap_DRSalvo·TestMap_DRRipple 격리 체인 / 원본 TestMap·DA_RocketLauncher·RocketLauncher·DA_RocketBody·DA_Rocket_PropTest·DA_TestSUV·BP_CFVehiclePawn·HeavyCannon SHA-256 보호
Latest Build: PASS — 48c0a81e19af4b20a17f628bcc7b723b / CarFight_ReEditor Win64 Development / Exit Code 0 / DR-PIE-06 Salvo·Ripple 격리 에셋·맵 적용 후 / Result Succeeded
Shield Regeneration Runtime Fix: APPLIED — UCFVehicleDefenseComp v1.3.0 / 유효 DefenseData 초기화 시 Activate(true) 후 초기 Tick 정지 / 실제 피해 후 재생 Tick 시작 / 비활성 컴포넌트 자동화 회귀 추가
Last CarFight Build: PASS — 0a642019bd4a4e4e87d96a0d3f4a3a2f / CarFight_ReEditor Win64 Development / Result Succeeded
Defense Automation: PASS — 7b5dce50680f47a99a29b26b02a577d7 / Exit Code 0 / CarFight.Damage 8/8 Success / 실패·경고·미실행 0
Defense Result JSON: UE/Saved/Automation/VehicleDefense/result.json / passed true / SHA-256 f5263e9d009520ef3afe2f68e816c7ebec379f6d052055a17922830822b59626
Combat Automation: PASS — 123e4433f5fd4b2a8ea1c1d991b07498 / Exit Code 0 / 필수 24/24 Success / 전체 46/46 Success / 실패 0 / Salvo·Ripple 격리 에셋·맵 추가 후 Launcher·Projectile·Damage 보호 회귀
Stored Map Two-Pawn PIE Automation: PASS — a4e8048231034105902498f3ec8499f7 / CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE / 플레이어 차량 + 대상 SUV / 대상 SUV 컴포넌트 단일 인스턴스·BeginPlay·Initial Mass·Snapshot Commit·Shield·Armor·Integrity 기준선 검증
Latest Full Regression: PASS — 123e4433f5fd4b2a8ea1c1d991b07498 / 전체 CarFight 46/46 Success / 필수 24/24 Success / 실패 0 / MuzzleSequence·FirePattern·Scheduler·Release·SourceIsolation·Propulsion·Damage RuntimeIntegration 포함
PIE: DR-PIE-00~06 USER PASS / DR-PIE-03 AP0 FRONT 75·INTEGRITY 100 / AP50 FRONT 87.5·INTEGRITY 87.5 / DR-PIE-05 LEGACY YES·INTEGRITY 100→75 / DR-PIE-06 SALVO·RIPPLE·5 VOLLEY·POOL·FX·COLLISION USER PASS
Foundation Build: PASS — 82ab34d50c62451fa74ca17ff00acad4 / Exit Code 0
Foundation Automation Process: PASS — 2887d47843304c4a8cf9a1b2f2b441e1 / CarFight.Damage 6/6
DR-P0-04 Automation Process: PASS — 8952458ad01242768466eaa455daf8a0 / Exit Code 0 / DebugBlueprintContract Success
Result JSON: UE/Saved/Automation/CombatRuntime/result.json / schema 1.0.0 / passed true / required 24/24 / total 46/46 / failed 0
CF-FQ-029 Regression: PASS — Launcher·Launch Context·Muzzle·Scheduler·Release·Pool 격리·추진·요격 필수 회귀 전체 Success
Legacy Cleanup: Deferred — CFWeaponData·CFProjectileData의 별도 정리는 이번 단계 범위 아님
Historical Next Active Feature (2026-08-06): CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
Historical Completion Handoff: Completed — 당시 다음 Active는 CF-FQ-032 UI-P0-02 User PIE
Completion Result: DR-P0-07 전체 PASS → VehicleDefense·HitDamage Systems Current → CF-FQ-033 Done
Historical Next Active Path: Document/Plan/InGameUIPlan.md v0.7.0 → UI-P0-02 User PIE → UI-P0-03 HUD 데이터 계약
```

`DR-P0-00`에서 다음 계약을 확정한다.

```text
- DamageData → DamageHitContext → VehicleDefenseComp → Shield → Directional Armor → VehicleHealthComp 흐름
- VehicleHealthComp를 차량 내구도와 파괴 상태의 기존 소유자로 유지
- ProjectileData.DefaultDamageData 단일 피해 데이터 참조 유지
- 쉴드, 방향별 장갑, 관통과 차량 내구도 피해 분배 공식
- 기존 VehicleData와 ProjectileData가 재저장 없이 동작하는 호환 정책
- HitScan과 Projectile의 동일 결과 계약
- 전용 Automation 테스트 행렬
- CF-FQ-029 dirty 변경과 런처·미사일 자산 보호 범위
```

`DR-P0-03`에서는 CF-FQ-029의 당시 변경을 보존한 채 `ACFVehiclePawn`, `ACFProjectileActor`와 방어 호환 계약의 최소 호출부만 통합했다. `DR-P0-04`에서는 마지막 전체 차량 피해 결과 캐시, Legacy Fallback 캐시, BlueprintPure 조회 API, VehicleDebug Snapshot·Panel 표시와 BlueprintAssignable 이벤트 Reflection Automation을 추가했다. `DR-P0-05`에서는 `DA_VehicleDefense_Test`와 이를 연결한 별도 `DA_VehicleDefense_TestSUV`를 생성했고, 원본 `DA_TestSUV`와 Legacy `DA_TestSedan`은 `DefaultDefenseData=None` 및 기존 Launcher·하드포인트·내구도·메시 값을 그대로 유지했다. `DR-P0-06`의 Defense 전용 Automation과 공식 Build는 PASS했다. 이후 `CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE`에서 플레이어 차량과 대상 SUV가 함께 존재하는 실제 2-Pawn 수명을 재현했다. 대상 SUV의 VehicleFittingComp와 VehicleDefenseComp는 각각 정확히 한 개이고 C++ getter와 같은 인스턴스였으며, `BeginPlay=Yes`, `InitialMass=Verified`, `CommittedSnapshot`, `DefenseReady=Yes`, `Shield=100`, 6방향 Armor 각 100, `Integrity=100`을 확인했다. `CF-FQ-033 ManualPIEProbe`는 대상 SUV가 PreRegister 초기 상태에서 Snapshot 준비로 이동하고 BeginPlay 초기화 직후부터 다음 Tick과 EndPlay 직전까지 같은 컴포넌트 주소로 정상 상태를 유지함을 증명했다. `Uninitialized` 복귀는 PIE 종료의 `EndPlay.AfterReset`에서만 발생했다. 따라서 사용자 Details의 `Uninitialized / None / 0` 표시는 실행 중 런타임 판정 근거로 사용할 수 없으며, DR-PIE-00부터 Details 판정을 폐기하고 `UE/Saved/Logs/CarFight_Re.log`의 ManualPIEProbe를 AI가 직접 판정한다. `DR-P0-06`은 Done이며 현재 작업은 `DR-P0-07 사용자 PIE`다. 사용자 수동 DR-PIE-00은 ManualPIEProbe 기준 초기화·피팅·방어값을 확인해 PASS했다. DR-PIE-01에서는 AP 0·BaseDamage 25 정면 누적 명중으로 Shield 100→0, Front Armor 100→0, Integrity 100→75와 최종 Integrity 0을 확인해 방어층 순서·정면 방향·내구도 고갈을 PASS했다. 최초 Integrity 0에서 폭발이 보이지 않았던 원인은 `DA_VehicleDefense_TestSUV.DefaultDestroyedFxData=None`인 테스트 자산 구성 누락이었고, 사용자가 `DA_FX_ProtoVehicleDead`를 연결한 뒤 실제 파괴 폭발을 확인했다. 사용자는 파괴된 테스트 SUV에 추가 피격을 가한 뒤 두 번째 파괴 폭발이 발생하지 않음을 확인했으므로 DR-PIE-01을 최종 USER PASS로 닫았다. DR-PIE-04의 시간 경과 Shield 재생을 대상 SUV 기준으로 직접 관찰하기 위해 기존 VehicleDebug Panel에 `선택 대상` 동적 Navigation 섹션을 추가했다. 이 섹션은 TargetSelectComp가 선택한 Actor의 표시 정보·추적 상태, VehicleDefenseComp의 Shield·6방향 Armor·재생 상태·남은 지연·마지막 전체 피해 결과와 VehicleHealthComp의 현재·최대 Integrity·파괴 상태를 매 프레임 읽기 전용으로 표시한다. WBP 에셋, TargetSelect 선택 로직, 피해 분배와 Shield 재생 계산은 변경하지 않는다. 사용자의 첫 DR-PIE-04에서 테스트 SUV에 두 발 명중 후 Shield가 50에 고정되고 재생하지 않는 FAIL을 확인했다. `AdvanceShieldRegeneration`은 `VehicleDefenseComp.IsActive()`가 false이면 지연과 재생을 진행하지 않지만 실제 Pawn 초기화는 활성 상태를 명시적으로 복구하지 않았고, 자동화 Fixture만 `Activate(true)`를 선행해 실제 수명 결함을 가리고 있었다. `UCFVehicleDefenseComp v1.3.0`에서 유효 DefenseData 초기화 시 컴포넌트를 명시적으로 활성화하고 즉시 `StopShieldRegeneration`으로 Tick을 꺼 초기 상태를 유지하며, 실제 피해 뒤에만 `RestartShieldRegenerationAfterDamage`가 Tick을 켜도록 수정했다. 자동화는 비활성 컴포넌트 조건을 재현해 초기화 후 활성 복구와 기존 재생 지연·초당 재생·최대값 정지를 함께 검증한다. 공식 Build와 전체 Combat Automation은 PASS했고, 수정 후 사용자 재검증에서 재생 지연 5→0, Shield 50→100, 재생 중 피격 시 지연 5초 재시작을 확인해 DR-PIE-04를 USER PASS로 닫았다. DR-PIE-02에서는 Front 75, Left 70, Right 70, Rear 62.5와 독립 Armor Pool·Integrity 100을 확인해 USER PASS로 닫았다. DR-PIE-03은 원본 AP0 맵과 별도 AP50 Pawn·GameMode·맵 격리 체인의 에셋 적용, AssetDump, 공식 Build와 전체 Combat Automation을 완료했다. 사용자 비교에서 AP0은 Front Armor 75·Integrity 100·Armor 흡수 25·Integrity 피해 0, AP50은 Front Armor 87.5·Integrity 87.5·Armor 흡수 12.5·Integrity 피해 12.5로 계약값과 정확히 일치했다. 동일 BaseDamage 25에서 AP50의 관통률 0.5가 장갑과 Integrity에 절반씩 분배되는 것을 확인해 DR-PIE-03을 USER PASS로 닫았다. DR-PIE-05는 원본 `M_VehicleDefensePIE`, `DA_TestSUV`, 방어 SUV·피팅과 AP0 공격 체인을 재저장하지 않고 `/Game/Maps/M_VehicleDefensePIE_Legacy`만 생성했다. 저장 후 재로드한 대상 `BP_CFVehiclePawn_C_1`은 `VehicleData=DA_TestSUV`, `VehicleFittingData=None`, `AutoPossessPlayer=Disabled`이며 GameMode는 `CFSingleGameMode`를 유지한다. Dry Run `8fbf901a30464338984b235ef981a879`, Apply `923a8e94a7a84ed4b516070b8b070107`, Maps AssetDump 24/24, 공식 Build `c93e71d77e0c45498d66da8b20c0227a`와 전체 Combat Automation `73a7ce48758642d4800c79cfd9c66990` 46/46·필수 24/24가 통과했다. 사용자 Projectile 한 발 결과에서 `Legacy=예`, Shield 0 유지, 6방향 Armor 0 유지, Integrity 100→75와 Integrity 요청/적용 25/25를 확인했다. BaseDamage 25가 방어층 없이 Integrity에 직접 적용되는 Legacy Fallback 계약과 기존 Health 변경 의미가 일치해 DR-PIE-05를 USER PASS로 닫았다. DR-PIE-06은 원본 `TestMap`, `DA_RocketLauncher`, `RocketLauncher`, `DA_RocketBody`, `DA_Rocket_PropTest`, `DA_TestSUV`, `BP_CFVehiclePawn`, `HeavyCannon`을 재저장하지 않고 `/Game/CarFight/Tests/LauncherRegression/` 아래에 Ripple Weapon·Preset과 Salvo·Ripple SUV를 생성하고 `/Game/Maps/TestMap_DRSalvo`, `/Game/Maps/TestMap_DRRipple` 두 격리 맵을 생성했다. Salvo는 원본 4발 동시 패턴을 유지하고 Ripple은 4발·0.15초·최대 동시 처리 1로 분리했으며, 두 맵의 기존 CF-FQ-029 Launcher 슬롯 `BP_CFVehiclePawn_C_1`은 Player0·VehicleFittingData=None으로 재로드됐다. Dry Run `b71e4b9e8242412c8abe31ae13994918`, Apply `f1566e9cc6a647dea7ba0a99b11ee68f`, LauncherRegression AssetDump 4/4, Maps World 8/8, 공식 Build `48c0a81e19af4b20a17f628bcc7b723b`와 전체 Combat Automation `123e4433f5fd4b2a8ea1c1d991b07498` 46/46·필수 24/24가 통과했다. 사용자 DR-PIE-06에서 Salvo 4발 동시, Ripple 4발·0.15초 순차, 두 패턴의 Muzzle 1→4→2→3, Sequence Completed·Accepted 4·Failed 0, 각 5 Volley 이상, Ripple 진행 중 중복 입력 방지, Rocket 추진·Trail·Thruster·Impact FX, 동일 차량 Projectile 상호 비충돌과 일반 차량·월드 충돌을 모두 확인했다. DR-PIE-06을 USER PASS로 닫고 DR-PIE-00~06 전체 PASS에 따라 DR-P0-07을 Done으로 판정했다. `Document/Systems/Combat/VehicleDefense.md v1.0.0`을 Current System으로 신규 작성하고 `HitDamage.md v1.1.0`을 통합 기준으로 갱신했으므로 `CF-FQ-033`은 Done이다. 다음 Active는 기존 우선순위에 따라 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다.

---

## 2. 목적

현재 `CF-FQ-018` 최소 Damage Runtime은 다음 기능을 이미 제공한다.

```text
DamageData.BaseDamage
→ FCFDamageHitContext
→ UCFVehicleHealthComp
→ CurrentHealth 감소
→ 최초 파괴 상태 전환
```

그러나 현재 런타임에는 다음 핵심 방어 기능이 없다.

```text
쉴드
방향별 장갑
장갑 관통
장갑 파괴
쉴드 재생
방어층별 피해 결과
부품 손상 확장 진입점
```

`CF-FQ-033`의 목적은 기존 최소 피해 시스템을 폐기하거나 다시 만드는 것이 아니다.
검증된 HitContext, Projectile Pool, 차량 체력과 파괴 이벤트를 유지하면서 그 앞에 방어 분배 계층을 추가한다.

최종 P0 흐름은 다음과 같다.

```text
WeaponData
→ ProjectileData.DefaultDamageData
→ FCFDamageHitContext
→ UCFVehicleDefenseComp
   → Shield
   → Directional Armor
   → UCFVehicleHealthComp = Vehicle Integrity
→ FCFVehicleDamageResult
→ C++ 상태 이벤트
→ Blueprint HUD·VFX 소비
```

---

## 3. P0 완료 범위

### 3.1 포함

```text
- DamageData 입력 검증
- 자기 피해 금지
- 쉴드 현재값·최대값
- 쉴드 피해 흡수와 파괴
- 쉴드 재생 지연과 초당 재생
- Front / Left / Right / Rear / Top / Bottom 피격 방향 판정
- 방향별 독립 장갑 내구도
- 방향별 피해 배율
- 장갑 저항과 ArmorPenetration 기반 관통 비율
- 장갑 흡수, 관통과 장갑 고갈 초과 피해
- 기존 VehicleHealthComp에 차량 내구도 피해 적용
- 차량 파괴 상태와 기존 파괴 이벤트 유지
- HitScan과 Projectile의 공통 방어 진입점
- 기존 에셋의 Legacy Direct Health Fallback
- 방어층별 결과, Debug와 Blueprint 이벤트
- 전용 Automation 소스와 사용자 PIE 검증표
```

### 3.2 P0에서 제외

```text
- 휠·엔진·터렛의 실제 독립 내구도와 기능 저하
- 피격 위치 Damage Zone
- 범위 폭발 대상 검색과 거리 감쇠
- 물리 Impulse 적용
- 도탄과 입사각 유효 장갑 두께
- 지속 피해, 화재, EMP와 상태 이상
- 충돌·낙하·환경 피해 발생기
- 수리, 장갑 복구와 차량 내구도 회복
- 서버 권한, Replication과 네트워크 예측
- 리스폰
```

P0 결과 구조에는 후속 확장 슬롯을 준비하되, 실제 부품 피해와 물리 충격 결과는 `0` 또는 빈 배열로 반환한다.

---

## 4. 현재 구현 기준과 유지 대상

### 4.1 유지하는 현재 계약

| 현재 요소 | 유지 정책 |
| --- | --- |
| `UCFDamageData` | 피해 입력 DataAsset으로 유지한다. |
| `UCFProjectileData.DefaultDamageData` | DamageData의 유일한 직접 참조 슬롯으로 유지한다. |
| `FCFDamageHitContext` | HitScan과 Projectile의 공통 명중 입력으로 유지·확장한다. |
| `FCFDamageApplyResult` | 차량 내구도 적용 결과와 Legacy 결과로 유지한다. 삭제하거나 이름을 바꾸지 않는다. |
| `UCFVehicleHealthComp` | 차량 내구도, 파괴 상태와 기존 이벤트의 소유자로 유지한다. |
| `OnVehicleHealthChanged` | 차량 내구도가 실제로 변경될 때만 발생한다. |
| `OnVehicleDamaged` | 차량 내구도에 실제 피해가 적용될 때만 발생한다. |
| `OnVehicleDestroyed` | 차량 내구도가 최초 0이 될 때 한 번만 발생한다. |
| Projectile 첫 Impact 1회 처리 | 동일 활성화에서 중복 피해를 금지한다. |
| Projectile Pool 반환 결과 복사 | Pool 반환 시 피해를 재계산하지 않는다. |

### 4.2 Legacy로 유지하는 데이터

`WeaponData.BaseDamage`와 `WeaponData.DamageProfileId`는 현재 Legacy 표시용이다.
실제 피해 계산 소유권을 다시 WeaponData로 이동하지 않는다.

```text
실제 DamageData 직접 참조
= ProjectileData.DefaultDamageData
```

`ProjectileData.DamageProfileId`는 `DefaultDamageData`가 없을 때 Debug에 표시할 Fallback ID일 뿐이며 실제 피해를 발생시키지 않는다.

---

## 5. CF-FQ-029 보호 계약

현재 `CF-FQ-029 모듈형 런처 및 발사 인계`는 `LM-P0-06` 사용자 PIE 단계이며 다수의 소스와 에셋이 미커밋 상태다.
`DR-P0-00`과 초기 Foundation은 아래 파일을 수정하지 않는다.

### 5.1 보호 대상 소스

```text
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Private/CFProjectileData.cpp
UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
UE/Source/CarFight_Re/Private/CFWeaponData.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Public/CFLauncherComp.h
UE/Source/CarFight_Re/Private/CFLauncherComp.cpp
UE/Source/CarFight_Re/Public/CFLauncherTypes.h
```

### 5.2 보호 대상 에셋과 Plan

```text
/Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher
/Game/CarFight/Weapons/Data/ProjectileDefs/DA_Rocket_PropTest
/Game/CarFight/Weapons/Data/TurretMounts/DA_RocketBody
/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher
/Game/CarFight/Weapons/Projectiles/Rocket/**
/Game/CarFight/Weapons/Turrets/RocketLauncher/**
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn
/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV
/Game/CarFight/Maps/TestMap
Document/Plan/LauncherMissile*.md
Document/Plan/Launch*.md
Document/Plan/ModularLauncherPlan.md
Document/Plan/MissileGuidancePlan.md
```

### 5.3 단계별 보호 게이트

```text
DR-P0-01~02
= 신규 타입·데이터·방어 컴포넌트와 기존 Damage/Health 파일만 사용
= CF-FQ-029 중첩 파일 수정 금지

DR-P0-03 HitScan·Projectile 통합
= CF-FQ-029 체크포인트 또는 명시적 보호 상태 확인 후 진입
= 대상 파일 현재 전체 내용을 다시 읽고 정확한 최소 호출부만 수정
= Launcher, Release, Projectile Pool, 추진, 요격과 FX 로직 변경 금지
```

기존 dirty 변경을 정리, 되돌리기, 전체 교체하거나 이름을 바꾸지 않는다.

---

## 6. 런타임 아키텍처

### 6.1 소유권

```text
UCFVehicleDefenseData
= 차량별 쉴드·장갑 정적 설정

UCFVehicleDefenseComp
= 쉴드·방향별 장갑 런타임 상태
+ 피해 분배
+ 쉴드 재생
+ 방어 이벤트

UCFVehicleHealthComp
= 차량 내구도 런타임 상태
+ 최초 파괴 전환
+ 기존 Health·Destroyed 이벤트
```

`UCFVehicleDefenseComp`가 `UCFVehicleHealthComp`를 대체하지 않는다.
`VehicleHealthComp.CurrentHealth`는 P0부터 의미상 `Vehicle Integrity`로 해석한다.

### 6.2 공용 피해 진입점

신규 정식 진입점:

```text
UCFVehicleDefenseComp::TryApplyDamageToActor(
    const FCFDamageHitContext& DamageHitContext,
    FCFVehicleDamageResult& OutVehicleDamageResult)
```

HitScan과 Projectile은 최종적으로 이 정적 진입점만 호출한다.

기존 진입점:

```text
UCFVehicleHealthComp::TryApplyDamageToActor()
```

기존 함수는 삭제하지 않고 Legacy Fallback과 기존 Blueprint·테스트 호환을 위해 유지한다.

### 6.3 차량 내구도 명시 피해 함수

방어 계산 후 남은 피해는 `DamageData.BaseDamage`와 다를 수 있다.
따라서 `UCFVehicleHealthComp`에 다음 명시 피해 함수를 추가한다.

```text
ApplyIntegrityDamageFromHitContext(
    const FCFDamageHitContext& DamageHitContext,
    float RequestedIntegrityDamage,
    FCFDamageApplyResult& OutDamageApplyResult)
```

정책:

```text
- 기존 ApplyDamageFromHitContext는 삭제하지 않는다.
- 기존 함수는 DamageData.BaseDamage를 읽어 신규 명시 피해 함수로 전달하는 호환 Wrapper가 된다.
- 체력 변경, 파괴 전환과 기존 세 이벤트는 신규 명시 피해 함수 한 곳에서만 발생한다.
```

---

## 7. 데이터 계약

## 7.1 기존 `UCFDamageData`

기존 필드 의미를 다음처럼 확정한다.

| 필드 | P0 의미 |
| --- | --- |
| `DamageId` | 피해 규칙 식별자 |
| `DamageType` | `Kinetic`, `Explosive`, `Energy` 피해 속성 |
| `BaseDamage` | 방어 분배를 시작할 원본 직접 피해량 |
| `bCanDamageSelf` | 자기 피해 허용 여부 |
| `ArmorPenetration` | 장갑 저항과 같은 단위를 사용하는 관통 기준값 |
| `ModuleDamageScale` | P1 부품 피해 후보 배율. P0 계산에는 사용하지 않음 |
| `bUseRadialDamage` 및 폭발 필드 | P1 범위 피해 후보. P0 직접 명중에서는 사용하지 않음 |
| `ImpulseStrength` | P1 물리 반응 후보. P0에서는 실제 힘을 적용하지 않음 |

### 피해 타입과 관통의 관계

```text
DamageType = 피해 속성
ArmorPenetration = 장갑 우회 능력을 나타내는 연속 수치
```

관통을 별도 피해 타입으로 추가하지 않는다.

예:

```text
일반탄 = Kinetic + 낮은 ArmorPenetration
철갑탄 = Kinetic + 높은 ArmorPenetration
고폭탄 = Explosive + 낮은 ArmorPenetration
에너지 무기 = Energy + 무기별 ArmorPenetration
```

## 7.2 신규 공용 enum

신규 파일 후보:

```text
UE/Source/CarFight_Re/Public/CFDamageRuntimeTypes.h
```

### `ECFDamageDeliveryType`

```text
DirectHit
RadialExplosion
Collision
Environmental
DamageOverTime
Scripted
```

P0 런타임 연결은 `DirectHit`만 완료한다.
나머지는 결과·분기 확장을 위한 계약으로만 선언한다.

### `ECFArmorDirection`

```text
None
Front
Left
Right
Rear
Top
Bottom
```

### `ECFArmorType`

```text
Light
Standard
Heavy
```

`ArmorType`은 분류와 UI용이다.
실제 공식은 DataAsset의 수치 `ArmorResistance`를 사용하며 C++에 Light/Standard/Heavy별 숨은 고정 수치표를 만들지 않는다.

## 7.3 `FCFDirectionalArmorConfig`

각 방향의 정적 장갑 설정이다.

```text
MaximumArmor
DamageMultiplier
```

의미:

| 필드 | 의미 |
| --- | --- |
| `MaximumArmor` | 해당 방향 장갑의 최대 런타임 내구도 |
| `DamageMultiplier` | 쉴드를 통과한 피해에 적용하는 방향별 피해 배율 |

초기 테스트 기본안:

```text
Front DamageMultiplier = 1.0
Left DamageMultiplier = 1.2
Right DamageMultiplier = 1.2
Rear DamageMultiplier = 1.5
Top DamageMultiplier = 1.3
Bottom DamageMultiplier = 1.6
```

이 값은 P0 검증 시작값이며 최종 밸런스 수치가 아니다.

## 7.4 `UCFVehicleDefenseData`

신규 파일 후보:

```text
UE/Source/CarFight_Re/Public/CFVehicleDefenseData.h
UE/Source/CarFight_Re/Private/CFVehicleDefenseData.cpp
```

정적 필드:

```text
DefenseId
bUseShield
MaximumShield
ShieldRegenerationDelaySeconds
ShieldRegenerationPerSecond
ArmorType
ArmorResistance
FrontArmorConfig
LeftArmorConfig
RightArmorConfig
RearArmorConfig
TopArmorConfig
BottomArmorConfig
ShieldComponentDamageScale
ArmorComponentDamageScale
IntegrityComponentDamageScale
```

P0 사용 범위:

```text
실제 사용
= Shield, Regen, ArmorType, ArmorResistance, 6방향 Armor Config

예약 필드
= 방어 단계별 ComponentDamageScale
```

P0에서는 실제 모듈 상태가 없으므로 `DamageAppliedToComponents=0`을 반환한다.
예약 필드는 P1에서 Damage Zone과 모듈 상태가 연결될 때 소비한다.

## 7.5 `UCFVehicleData` 연결

`UCFVehicleData`에 다음 선택 참조를 추가한다.

```text
DefaultDefenseData : UCFVehicleDefenseData
```

정책:

```text
DefaultDefenseData가 유효함
→ Shield·Armor·Integrity 분배 사용

DefaultDefenseData가 None
→ 기존 BaseDamage 직접 Health 적용 Fallback 사용
```

기존 `VehicleDurabilityConfig.MaxHealth`는 유지한다.
차량 내구도 최대값을 DefenseData로 이동하지 않는다.

## 7.6 `FCFVehicleDamageResult`

신규 결과 구조는 한 번의 피해 요청에서 모든 방어층 결과를 보존한다.

필수 필드:

```text
bDamageAccepted
bAppliedToAnyLayer
bUsedLegacyHealthFallback
RejectReason
TargetActor
DamageId
DamageDeliveryType
ArmorDirection
RequestedDamage
DamageAbsorbedByShield
DamageAfterShield
DirectionDamageMultiplier
DirectionalDamage
ArmorPenetration
EffectiveArmorResistance
ArmorPenetrationRatio
DamageAbsorbedByArmor
DamageRequestedForIntegrity
DamageAppliedToIntegrity
IntegrityOverkillDamage
DamageAppliedToComponents
PhysicalImpulseApplied
ShieldBefore
ShieldAfter
ArmorBefore
ArmorAfter
IntegrityBefore
IntegrityAfter
bShieldBrokenThisHit
bArmorBrokenThisHit
bDestroyedThisHit
IntegrityApplyResult
AffectedComponentIds
```

정책:

```text
DamageAppliedToComponents = 0
PhysicalImpulseApplied = 0
AffectedComponentIds = 빈 배열
```

위 세 항목은 DR-P1 전까지 항상 위 값으로 유지한다.

`IntegrityApplyResult`는 기존 `FCFDamageApplyResult`를 그대로 포함해 기존 디버그와 이벤트 결과를 추적할 수 있게 한다.

---

## 8. 런타임 상태 계약

`UCFVehicleDefenseComp`는 다음 상태를 소유한다.

```text
ActiveDefenseData
MaximumShield
CurrentShield
FrontArmor
LeftArmor
RightArmor
RearArmor
TopArmor
BottomArmor
bDefenseInitialized
bShieldRegenerating
RemainingShieldRegenDelaySeconds
```

장갑은 하나의 공유 HP가 아니라 방향별 독립 Pool이다.

```text
정면 장갑 피해가 후면 장갑을 감소시키지 않는다.
후면 장갑 피해가 정면 방어를 약화시키지 않는다.
```

이 구조는 방향 관리가 전투 결과에 영향을 줘야 한다는 CombatPlan 결정을 따른다.

---

## 9. 피해 분배 공식

## 9.1 입력 검증 순서

다음 순서로 거부 조건을 판정한다.

```text
1. bBlockingHit == true
2. HitActor 유효
3. DamageData 유효
4. BaseDamage > 0
5. 자기 피해 정책 허용
6. 대상에 VehicleDefenseComp 또는 VehicleHealthComp 존재
7. 대상 VehicleHealthComp가 아직 파괴되지 않음
```

거부된 요청은 쉴드 재생 지연을 갱신하지 않고 어떤 방어값도 변경하지 않는다.

## 9.2 원본 피해

```text
RequestedDamage = Max(DamageData.BaseDamage, 0)
```

## 9.3 쉴드 처리

쉴드는 방향 배율과 장갑 관통의 영향을 받기 전에 원본 피해를 1:1로 흡수한다.

```text
DamageAbsorbedByShield = Min(CurrentShield, RequestedDamage)
CurrentShield = CurrentShield - DamageAbsorbedByShield
DamageAfterShield = RequestedDamage - DamageAbsorbedByShield
```

쉴드가 모든 피해를 흡수하면 장갑과 차량 내구도 계산을 종료한다.

```text
DamageAfterShield <= 0
→ DamageAbsorbedByArmor = 0
→ DamageAppliedToIntegrity = 0
```

## 9.4 방향 배율

쉴드를 통과한 피해에 피격 방향 배율을 적용한다.

```text
DirectionalDamage
= DamageAfterShield × DirectionDamageMultiplier
```

방향 배율은 `0` 이상으로 제한한다.

## 9.5 장갑 관통 비율

```text
EffectiveArmorResistance = Max(ArmorResistance, 0)
```

장갑 저항이 0 이하이면 남은 피해는 전부 관통한다.

```text
EffectiveArmorResistance <= 0
→ ArmorPenetrationRatio = 1
```

장갑 저항이 유효하면 다음 공식을 사용한다.

```text
ArmorPenetrationRatio
= Clamp(ArmorPenetration / EffectiveArmorResistance, 0, 1)
```

```text
DirectPenetrationDamage
= DirectionalDamage × ArmorPenetrationRatio

ArmorBlockCandidate
= DirectionalDamage - DirectPenetrationDamage
```

## 9.6 방향별 장갑 처리

```text
DamageAbsorbedByArmor
= Min(CurrentDirectionalArmor, ArmorBlockCandidate)

CurrentDirectionalArmor
= CurrentDirectionalArmor - DamageAbsorbedByArmor

ArmorOverflowDamage
= ArmorBlockCandidate - DamageAbsorbedByArmor
```

차량 내구도에 요청할 피해:

```text
DamageRequestedForIntegrity
= DirectPenetrationDamage + ArmorOverflowDamage
```

의미:

```text
관통한 피해
→ 장갑 Pool을 소모하지 않고 차량 내구도로 전달

장갑이 막은 피해
→ 해당 방향 장갑 Pool 소모

장갑 Pool이 부족해 막지 못한 피해
→ 초과분이 차량 내구도로 전달
```

P0에서 도탄, 입사각과 관통탄의 별도 장갑 마모는 계산하지 않는다.

## 9.7 차량 내구도 처리

```text
DamageRequestedForIntegrity > 0
→ VehicleHealthComp.ApplyIntegrityDamageFromHitContext()
```

기존 체력 하한 정책을 유지한다.

```text
DamageAppliedToIntegrity
= Min(DamageRequestedForIntegrity, IntegrityBefore)

IntegrityAfter
= Max(IntegrityBefore - DamageRequestedForIntegrity, 0)

IntegrityOverkillDamage
= Max(DamageRequestedForIntegrity - DamageAppliedToIntegrity, 0)
```

차량 내구도가 최초 0이 되면 기존 `OnVehicleDestroyed`를 정확히 한 번 발생시킨다.

## 9.8 결과 불변식

쉴드 이후 방향 배율이 적용되므로 원본 피해와 이후 피해량은 별도 기준으로 기록한다.

필수 불변식:

```text
RequestedDamage
= DamageAbsorbedByShield + DamageAfterShield

DirectionalDamage
= DamageAbsorbedByArmor + DamageRequestedForIntegrity

DamageRequestedForIntegrity
= DamageAppliedToIntegrity + IntegrityOverkillDamage
  단, 대상 파괴 전 정상 처리 기준
```

부동소수점 비교는 작은 허용 오차를 사용한다.

---

## 10. 피격 방향 판정

CarFight 차량 로컬 축 기준:

```text
Forward = +X
Right = +Y
Up = +Z
```

### 10.1 기준점

```text
1. HitComponentName과 같은 PrimitiveComponent를 찾으면 해당 Bounds.Origin 사용
2. 찾지 못하면 대상 Actor Bounds 중심 사용
```

### 10.2 방향 벡터

```text
WorldOffset = ImpactLocation - DefenseBoundsCenter
LocalOffset = ActorTransform.InverseTransformVectorNoScale(WorldOffset)
```

`LocalOffset`이 거의 0이면 다음 순서로 Fallback한다.

```text
1. ImpactNormal을 차량 로컬 공간으로 변환
2. 유효하지 않으면 -IncomingDirection을 차량 로컬 공간으로 변환
3. 모두 유효하지 않으면 Front
```

### 10.3 지배 축 판정

```text
AbsX = Abs(LocalOffset.X)
AbsY = Abs(LocalOffset.Y)
AbsZ = Abs(LocalOffset.Z)

AbsZ > Max(AbsX, AbsY)
→ Z >= 0 ? Top : Bottom

그 외 AbsX >= AbsY
→ X >= 0 ? Front : Rear

그 외
→ Y >= 0 ? Right : Left
```

동률에서는 `Front/Rear`를 `Left/Right`보다 우선하며, `Top/Bottom`은 Z축이 명확하게 가장 클 때만 선택한다.
이는 경사진 차체 표면 노멀만 사용했을 때 방향이 흔들리는 문제를 줄이기 위한 P0 결정이다.

---

## 11. 쉴드 재생 계약

## 11.1 시작 조건

```text
bUseShield == true
CurrentShield < MaximumShield
ShieldRegenerationPerSecond > 0
유효한 마지막 피해 처리 후 ShieldRegenerationDelaySeconds 경과
```

유효한 피해 요청이 어느 방어층에 실제 값을 감소시키면 재생 지연을 초기화한다.
쉴드가 이미 0이고 장갑 또는 차량 내구도만 피해를 받아도 재생 지연은 다시 시작한다.

거부된 피해는 재생 지연을 초기화하지 않는다.

## 11.2 재생 공식

```text
CurrentShield
= Min(
    CurrentShield + ShieldRegenerationPerSecond × DeltaSeconds,
    MaximumShield)
```

## 11.3 Tick 정책

```text
- 기본 상태에서 Tick 비활성
- 재생 지연 대기 또는 실제 재생 중에만 Tick 활성
- MaximumShield 도달, 재생 불가 설정 또는 컴포넌트 비활성 시 Tick 중지
```

불필요한 상시 Tick을 사용하지 않는다.

---

## 12. 이벤트 계약

### 12.1 신규 Defense 이벤트

```text
OnVehicleDamageResolved
OnShieldChanged
OnShieldBroken
OnArmorChanged
OnArmorBroken
OnShieldRegenerationStarted
OnShieldFullyRestored
```

권장 의미:

| 이벤트 | 발생 조건 |
| --- | --- |
| `OnVehicleDamageResolved` | 유효 피해가 Shield, Armor 또는 Integrity 중 하나 이상에 실제 적용됨 |
| `OnShieldChanged` | CurrentShield가 실제 변경됨 |
| `OnShieldBroken` | ShieldBefore > 0이고 ShieldAfter <= 0 |
| `OnArmorChanged` | 특정 방향 CurrentArmor가 실제 변경됨 |
| `OnArmorBroken` | ArmorBefore > 0이고 ArmorAfter <= 0 |
| `OnShieldRegenerationStarted` | 지연 종료 후 실제 Shield 증가가 시작되는 첫 프레임 |
| `OnShieldFullyRestored` | CurrentShield가 MaximumShield에 도달한 순간 |

### 12.2 기존 Health 이벤트

기존 이벤트 의미는 변경하지 않는다.

```text
Shield만 감소
→ 기존 OnVehicleDamaged 발생하지 않음

Armor만 감소
→ 기존 OnVehicleDamaged 발생하지 않음

Integrity 감소
→ 기존 OnVehicleHealthChanged / OnVehicleDamaged 발생

Integrity 최초 0
→ 기존 OnVehicleDestroyed 발생
```

Blueprint HUD와 VFX는 신규 Defense 이벤트를 사용하고 피해 공식 자체를 Blueprint에 복제하지 않는다.

---

## 13. 호환 정책

## 13.1 기존 차량 에셋

```text
VehicleData.DefaultDefenseData == None
```

이면 기존 동작을 유지한다.

```text
DamageData.BaseDamage
→ VehicleHealthComp 직접 적용
```

기존 차량 DataAsset을 강제로 재저장하지 않는다.

## 13.2 신규 방어 차량

```text
VehicleData.DefaultDefenseData != None
```

이면 신규 3층 방어 계산을 사용한다.

## 13.3 기존 Projectile과 HitScan

DamageData 소유 경로는 바꾸지 않는다.

```text
EquipmentPresetData
→ WeaponData.DefaultProjectileData
→ ProjectileData.DefaultDamageData
→ DamageData
```

HitScan도 ProjectileActorClass가 없는 가상 ProjectileData를 통해 같은 DamageData를 사용한다.

## 13.4 기존 결과와 Blueprint

```text
FCFDamageApplyResult 유지
UCFVehicleHealthComp 함수 유지
기존 Health 이벤트 유지
기존 Debug 필드 유지
```

신규 `FCFVehicleDamageResult`를 추가하고 기존 결과를 내부 Integrity 결과로 포함한다.
기존 Blueprint가 즉시 깨지는 리네이밍이나 시그니처 제거를 하지 않는다.

## 13.5 실패 격리

DefenseData, Shield 또는 Armor 설정이 잘못돼도 다음 기능을 취소하지 않는다.

```text
Projectile 이동
Projectile 충돌
Impact FX
Pool 반환
HitContext 기록
기존 차량 파괴 이벤트
```

유효하지 않은 DefenseData는 `IsDataValid` 경고와 안전 보정을 사용한다.
치명적 설정 오류가 아니라면 Legacy Health Fallback 또는 0 방어층으로 처리한다.

---

## 14. C++ / Blueprint / DataAsset 책임 분리

### C++

```text
- 피해 입력 검증
- 자기 피해 정책
- 피격 방향 판정
- 쉴드·장갑·차량 내구도 분배 공식
- 장갑 관통과 장갑 고갈 처리
- 쉴드 재생 상태
- 파괴 상태 1회 전환
- 결과 구조 작성
- Legacy Fallback
- 이벤트 발생 조건
- 데이터 유효성 검사
- Automation 테스트
```

### Blueprint

```text
- 쉴드 피격 VFX
- 장갑 피격 VFX
- 차량 내구도 피격 VFX
- 카메라 흔들림
- HUD 수치와 방향 표시
- 쉴드 파괴·재생 시각 피드백
- 장갑 파괴 시각 피드백
- 기존 파괴 VFX 연결
```

Blueprint에서 피해 분배 수식, 관통 계산과 파괴 판정을 작성하지 않는다.

### DataAsset

```text
DamageData
= 무기·탄종별 공격 수치

VehicleDefenseData
= 차량별 쉴드·장갑 설정

VehicleData
= DefaultDefenseData 참조 + 기존 VehicleDurabilityConfig
```

---

## 15. 구현 단계

## DR-P0-00 — 설계·계약 확정

```text
상태: Done
산출물: 이 문서
소스·에셋: 변경 없음
```

## DR-P0-01 — 공용 타입과 DefenseData Foundation

```text
상태: Done
Implementation Source: Browser AI Direct
Source Changes: Applied
Asset Changes: Not Applied
Diff Review: PASS
Build: PASS — bdc3e14d513549379d7f0768ad1ed7bd / Exit Code 0
Automation: PASS — CarFight.Damage.DR_P0_01.DataContract / 1 Success / 0 Failed / 0 Not Run
PIE: Not Applicable
```

적용 파일:

```text
신규 UE/Source/CarFight_Re/Public/CFDamageRuntimeTypes.h
신규 UE/Source/CarFight_Re/Public/CFVehicleDefenseData.h
신규 UE/Source/CarFight_Re/Private/CFVehicleDefenseData.cpp
신규 UE/Source/CarFight_Re/Private/CFVehicleDefenseTests.cpp
수정 UE/Source/CarFight_Re/Public/CFVehicleData.h
신규 Tools/RunDefenseDataTests.ps1
```

완료 내용:

```text
- ECFDamageDeliveryType, ECFArmorDirection, ECFArmorType UHT 계약 추가
- FCFDirectionalArmorConfig와 전체 FCFVehicleDamageResult 추가
- UCFVehicleDefenseData 쉴드·재생·장갑 저항·6방향 장갑·P1 예약 배율 추가
- 안전 Getter, 방어 요약과 ValidateDefenseDataContract 추가
- Unreal IsDataValid 기반 DataValidation 오류 보고 추가
- VehicleData.DefaultDefenseData 선택 참조 추가
- 기존 VehicleData의 DefaultDefenseData=None 호환 확인
- P0 방향 배율 Front 1.0 / Left·Right 1.2 / Rear 1.5 / Top 1.3 / Bottom 1.6 기본값 적용
- 공식 Editor 빌드에서 UHT 생성 파일 8개와 신규 테스트 소스 컴파일·링크 PASS
- DataContract Automation 실제 실행 1개 성공, 경고·오류·미실행 0
```

보호 결과:

```text
- CFProjectileActor.* 미수정
- CFProjectileData.* 미수정
- CFProjectilePoolComp.* 미수정
- CFVehiclePawn.* 미수정
- CFVehicleWeaponComp.* 미수정
- CFWeaponData.* 미수정
- CFLauncherComp.*와 CFLauncherTypes.h 미수정
```

Legacy 정리 판정:

```text
- WeaponData.BaseDamage와 WeaponData.DamageProfileId는 런타임 미사용 Legacy 필드다.
- ProjectileData.DamageProfileId는 DamageData 누락 시 Debug fallback ID다.
- 세 필드가 위치한 CFWeaponData.*와 CFProjectileData.*는 현재 CF-FQ-029 dirty 보호 파일이다.
- DR-P0-01에서는 삭제·리네이밍·직렬화 변경을 하지 않았다.
- CF-FQ-029 체크포인트 이후 별도 호환 패치에서 에디터 비노출 UPROPERTY로 전환하고 저장 에셋 마이그레이션을 검증한다.
```

## DR-P0-02 — DefenseComp와 Health 호환 확장

```text
상태: Done
Implementation Source: Browser AI Direct
Source Changes: Applied
Asset Changes: Not Applied
Diff Review: PASS
Final Build: PASS — 82ab34d50c62451fa74ca17ff00acad4 / Exit Code 0
Automation: PASS — CarFight.Damage / 6 Success / 0 Failed / 0 Not Run
PIE: Pending — Pawn·HitScan·Projectile 정식 연결은 DR-P0-03에서 완료됐고 사용자 PIE는 DR-P0-07에서 수행
```

적용 파일:

```text
신규 UE/Source/CarFight_Re/Public/CFVehicleDefenseComp.h
신규 UE/Source/CarFight_Re/Private/CFVehicleDefenseComp.cpp
신규 UE/Source/CarFight_Re/Private/CFVehicleDefenseCompTests.cpp
수정 UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h
수정 UE/Source/CarFight_Re/Private/CFVehicleHealthComp.cpp
수정 Tools/RunDefenseDataTests.ps1
```

완료 내용:

```text
- UCFVehicleDefenseComp가 쉴드와 Front·Left·Right·Rear·Top·Bottom 독립 장갑 Pool을 소유한다.
- Bounds 중심→ImpactLocation 로컬 지배 축과 ImpactNormal·IncomingDirection Fallback을 구현했다.
- Shield → DirectionMultiplier → Penetration → Armor → Integrity 피해 분배 공식을 구현했다.
- ArmorResistance 0 이하 완전 관통, 부분 관통과 장갑 고갈 초과 피해를 구현했다.
- 유효 피해 후 재생 지연을 재시작하고 지연 초과 Delta를 실제 재생에 사용하는 쉴드 재생을 구현했다.
- 재생 대기·진행 중에만 Tick을 켜고 최대 쉴드 도달 시 Tick을 중지한다.
- VehicleHealthComp에 ApplyIntegrityDamageFromHitContext와 Integrity Getter를 추가했다.
- 기존 ApplyDamageFromHitContext, TryApplyDamageToActor, Health Getter·프로퍼티·이벤트 이름을 유지했다.
- DefenseComp 또는 DefenseData가 없으면 기존 BaseDamage 직접 Health 적용 결과를 FCFVehicleDamageResult로 변환한다.
- Defense·Shield·Armor·Regeneration Blueprint 이벤트 계약을 추가했다.
- 부품 피해와 물리 충격 결과는 DR-P1 전까지 0과 빈 배열을 유지한다.
```

Automation 실제 실행:

```text
CarFight.Damage.DR_P0_01.DataContract — Success
CarFight.Damage.DR_P0_02.HealthCompatibility — Success
CarFight.Damage.DR_P0_02.DirectionalArmor — Success
CarFight.Damage.DR_P0_02.ShieldArmorPenetration — Success
CarFight.Damage.DR_P0_02.OverflowLegacyFallback — Success
CarFight.Damage.DR_P0_02.ShieldRegeneration — Success
```

보정 이력:

```text
- 첫 빌드 Job 6daedc1e308345fbbfd8baf9b70123a2는 테스트 보조 함수와 Automation 멤버 이름 충돌로 Exit Code 6이었다.
- 테스트 전용 비교 매크로로 이름 충돌을 제거한 뒤 Build Job c6eee859d6df466080157eac82acae93이 PASS했다.
- 첫 전체 Automation은 5 Success / 1 Fail이었다.
- 실패 원인은 BeginPlay 없는 테스트 월드에서 수동 등록한 DefenseComp가 비활성 상태였기 때문이다.
- Fixture에서 DefenseComp를 명시적으로 Activate한 뒤 최종 Build와 Automation이 PASS했다.
- 실제 런타임 IsActive 안전 조건과 피해·재생 공식을 변경하지 않았다.
```

보호 결과:

```text
- CFProjectileActor.* 미수정
- CFProjectileData.* 미수정
- CFProjectilePoolComp.* 미수정
- CFVehiclePawn.* 미수정
- CFVehicleWeaponComp.* 미수정
- CFWeaponData.* 미수정
- CFLauncherComp.*와 CFLauncherTypes.h 미수정
- 런처·로켓·차량·맵 .uasset 미수정
```

현재 제한:

```text
- ACFVehiclePawn 기본 서브오브젝트로 VehicleDefenseComp를 아직 생성하지 않는다.
- HitScan과 Projectile은 아직 기존 VehicleHealthComp 진입점을 호출한다.
- 따라서 신규 Shield·Armor Runtime은 자동화에서 검증됐지만 실제 플레이 경로에는 연결되지 않았다.
- 정식 통합은 CF-FQ-029 체크포인트를 재확인한 뒤 DR-P0-03에서 최소 호출부만 수정해 완료했다.
```

## DR-P0-03 — HitScan·Projectile 정식 통합

상태: **Done — 2026-07-31**

```text
- ACFVehiclePawn CDO에 VehicleDefenseComp 기본 서브오브젝트 생성 완료
- VehicleHealthComp 초기화 뒤 VehicleDefenseComp.InitializeFromVehicleData 호출 완료
- Runtime Summary의 Defense 상태 Ready / LegacyFallback / Missing 구분 완료
- HitScan 정식 피해 진입점을 UCFVehicleDefenseComp::TryApplyDamageToActor로 전환 완료
- Projectile 첫 유효 Impact 정식 피해 진입점을 UCFVehicleDefenseComp::TryApplyDamageToActor로 전환 완료
- FCFVehicleDamageResult 전체 결과와 기존 FCFDamageApplyResult Integrity 호환 결과 동시 보존 완료
- Projectile 재활성화와 Intercepted 경로의 신규 결과 초기화 완료
- 공식 Editor Build 7c6c3f2e6d3647d4af4e63bf537933c5 / Exit Code 0
- Combat Runtime Automation 95bb9b328ed2443baccf0fb59e3a0cab / 필수 14/14·전체 23/23 Success
- CF-FQ-029 Launch·Pool·추진·동일 차량 격리·다른 차량 요격 회귀 PASS
- Blueprint·DataAsset·맵·Niagara 자산 변경 없음
```

다음 단계는 `DR-P0-04 Debug와 Blueprint 이벤트`다.

선행 게이트:

```text
- CF-FQ-029 현재 dirty 상태 재확인
- Launcher 관련 사용자 변경 보존 가능 여부 확인
- 대상 파일 현재 내용 재읽기
```

수정 후보:

```text
CFVehiclePawn.cpp/.h
CFProjectileActor.cpp/.h
필요 시 Debug 결과 보관 타입
```

금지:

```text
- Launch Context 변경
- Launcher Scheduler 변경
- Projectile Pool 수명 변경
- 추진·유도·요격 변경
- 충돌 Profile 변경
- FX 생명주기 변경
```

완료 조건:

```text
HitScan과 Projectile이 VehicleDefenseComp 진입점 사용
DefaultDefenseData None 차량은 기존 피해 결과 유지
Projectile 첫 Impact 1회와 Pool 결과 복사 유지
공식 Editor Build PASS
```

## DR-P0-04 — Debug와 Blueprint 이벤트 — Done

```text
- VehicleDebug에 Shield·Direction·Armor·Integrity 결과 표시
- 신규 Defense 이벤트 노출
- 기존 Health Debug와 이벤트 유지
- UI는 수치 소비만 하고 계산하지 않음
```

## DR-P0-05 — 테스트 DataAsset 연결 — Done

적용 결과:

```text
- /Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test 생성
- DefenseId=VehicleDefense_Test
- Shield 100 / 재생 지연 5초 / 초당 재생 10
- Standard ArmorResistance 100
- 6방향 Armor 100 / 배율 Front 1.0, Left·Right 1.2, Rear 1.5, Top 1.3, Bottom 1.6
- /Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV 생성
- DA_VehicleDefense_TestSUV.DefaultDefenseData에 DA_VehicleDefense_Test 연결
- DA_TestSUV.DefaultDefenseData=None과 RocketLauncher·하드포인트·내구도·메시 참조 유지
- DA_TestSedan.DefaultDefenseData=None으로 Legacy Integrity Fallback 유지
- /Game/CarFight/Weapons/Data/DamageDefs/DA_DamageArmorPenTest 생성
- DA_DamageArmorPenTest: DamageId=ArmorPenetration_Test / BaseDamage=25 / ArmorPenetration=50 / Kinetic / 자기 피해 금지
- 기존 DA_DamageAsset: BaseDamage=25 / ArmorPenetration=0과 파일 해시 유지
```

검증:

```text
Apply Report: UE/Saved/VehicleDefenseAssetApply/report.json / success=true / protected_contract_passed=true
Final Apply Process: 9c05e8051c75406d8c8ce5a053fd45b7 / Editor Exit Code 0 / wrapper Exit Code 0
Final Apply Report: UE/Saved/VehicleDefenseAssetApply/report.json / tool 1.2.0 / success=true / protected_contract_passed=true / SHA-256 9d5d5d5bfdb29ab2531c2c04cce6b3f5dd98ce161c48902e9c4bb47068b10bc1
Defense AssetDump: assets 2 / succeeded 2 / failed 0
DamageData AssetDump: assets 2 / succeeded 2 / failed 0 / DA_DamageAsset AP 0·DA_DamageArmorPenTest AP 50 확인
DamageData Dump Report: UE/Saved/VehicleDefense/DamageDataFinal/run_report.json
AssetDump Report: UE/Saved/VehicleDefense/DefensePost/run_report.json
DA_TestSUV SHA-256: 1207be26da3f21bda34fb66f950e4717601be1cdee80807085ddb93866219317 전후 동일
DA_TestSedan SHA-256: 8d64cf12b71c1a71c87e2bb269da609ed32159909347a72d86b7846f38da1646 전후 동일
```

기존 런처·로켓 에셋과 원본 VehicleData를 덮어쓰지 않았다.

## DR-P0-06 — Automation 전체 실행

```text
상태: Done — Defense·Combat Automation PASS / Official Build PASS
Defense Automation: PASS — 7b5dce50680f47a99a29b26b02a577d7 / CarFight.Damage 8/8 Success / Exit Code 0
Combat Automation: PASS — 54340630094a461ea3f7a1056622fd3c / 필수 15/15·전체 24/24 Success / Exit Code 0
TargetHud Regression: PASS — TS_P0_06.TargetHud Success / 이전 WBP_TargetSelect 파일 잠금 해소
Official Build: PASS — 0697e0552a0f4911a93384c5d3a8a928 / CarFight_ReEditor Win64 Development / Exit Code 0 / Result Succeeded
Protection: WBP_TargetSelect 직접 편집 없음 / TargetHud Automation SavePackage 성공 / 사전 dirty 자산이므로 바이트 무변경 단정 없음 / CF-FQ-029 Launcher·Pool·추진·요격 소스와 관련 에셋 직접 수정 없음
완료 결과: AssetDump 차단 해소 확인 / 공식 CarFight_ReEditor Win64 Development Build PASS / DR-P0-06 Done
```

## DR-P0-07 — 사용자 PIE

상태: **PIE Procedure Ready / User Result Pending**

### DR-P0-07A — 보호형 테스트 환경 원칙

현재 `TestMap`의 실제 플레이 진입은 다음과 같다.

```text
BP_CFVehiclePawn CDO → DA_TestSUV → RocketLauncher
TestMap BP_CFVehiclePawn_C_0 → DA_TestSedan → HeavyCannon
TestMap BP_CFVehiclePawn_C_1 → DA_TestSUV → RocketLauncher
```

이 경로의 `DA_TestSUV`, `BP_CFVehiclePawn`, `TestMap`, Launcher·Projectile·Pool·추진·요격 에셋은 `CF-FQ-029` 보호 대상이다. 따라서 DR-P0-07에서는 원본을 재저장하지 않는다.

안전한 기본 절차:

```text
1. 원본 TestMap을 직접 수정·저장하지 않는다.
2. 필요하면 Content Browser에서 TestMap을 새 PIE 전용 맵으로 복제한다.
3. 권장 신규 경로: /Game/CarFight/Tests/VehicleDefense/M_VehicleDefensePIE
4. 복제 맵에서만 차량 데이터, Auto Possess와 Debug 표시를 조정한다.
5. 원본 DA_TestSUV, DA_TestSedan, DA_HeavyShell, DA_Rocket_PropTest,
   DA_ProtoTurretCannon, DA_RocketLauncher와 EquipmentPreset은 수정하지 않는다.
6. PIE 종료 시 원본 에셋 저장 대화상자가 나오면 저장하지 않는다.
```

방어 대상 VehicleData:

```text
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV
```

기준값:

```text
Shield 100
Shield Regen Delay 5초
Shield Regen Rate 10/초
ArmorResistance 100
Front 100 × 1.0
Left 100 × 1.2
Right 100 × 1.2
Rear 100 × 1.5
Top 100 × 1.3
Bottom 100 × 1.6
Integrity 100
```

### DR-P0-07B — 권장 플레이 배치

첫 방어 검증은 단발 확인이 쉬운 Sedan·HeavyCannon을 조작 차량으로 사용한다.

복제 맵 설정:

```text
Sedan Actor
- 차량 데이터 (VehicleData): DA_TestSedan 유지
- 자동 빙의 플레이어 (Auto Possess Player): Player 0
- VehicleDebug UI 사용: True
- VehicleDebug Panel 표시 여부: True

SUV Target Actor
- 차량 데이터 (VehicleData): DA_VehicleDefense_TestSUV
- 자동 빙의 플레이어 (Auto Possess Player): Disabled
- 시작 시 정지 상태 유지
```

PIE 시작 직후 발사 전에 다음을 확인한다.

```text
조작 Sedan VehicleDebug
- 활성 ProjectileData와 활성 DamageData가 유효함
- BaseDamage 25 확인
- ArmorPenetration 0 확인

대상 SUV PIE 인스턴스
- 방어 컴포넌트 존재
- DefenseData 초기화=예
- 피해 경로=Shield → Armor → Integrity
- Shield=100/100
- Front·Left·Right·Rear·Top·Bottom Armor=각 100
- Integrity=100
```

조작 차량의 활성 DamageData가 `DA_DamageAsset / BaseDamage 25 / ArmorPenetration 0`과 다르면 수치 검증을 시작하지 않고 실제 표시값을 기록한다.

대상 상태 관찰 방법:

```text
1. 사용자는 Tools/RunEditor.bat로 에디터를 실행하고 M_VehicleDefensePIE에서 PIE를 시작한다.
2. 시작 뒤 최소 2초 동안 사격하지 않고 PIE를 유지한다.
3. 사용자는 World Outliner 또는 Details에서 런타임 필드를 찾지 않는다.
4. 사용자가 PIE 실행 사실을 보고하면 AI가 UE/Saved/Logs/CarFight_Re.log에서
   "CF-FQ-033 ManualPIEProbe"를 검색한다.
5. 대상 SUV의 BeginPlay.AfterInitialize, NextTick, AfterOneSecond 행을 비교한다.
6. 같은 Actor·FittingComp·DefenseComp 주소에서 Verified, CommittedSnapshot,
   DefenseReady=Yes, ActiveDefenseData, Shield·Armor·Integrity 값을 판정한다.
7. PreRegister.BeforePrepare의 Uninitialized와 EndPlay.AfterReset의 Reset은
   정상 수명 경계이므로 실행 중 실패로 판정하지 않는다.
```

에디터 Details의 `InitialMass/FittingRuntime=Uninitialized`, `ActiveDefenseData=None`, 방어값 0 표시는 실행 중 실제 컴포넌트 상태와 일치하지 않는 사례가 확인됐으므로 DR-PIE 판정 근거로 사용하지 않는다.

### DR-P0-07C — AP50 격리 원칙

`DA_DamageArmorPenTest`는 BaseDamage 25·ArmorPenetration 50 비교 데이터이며 원본 Weapon·Projectile에는 연결하지 않는다.

```text
/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageArmorPenTest
BaseDamage 25 / ArmorPenetration 50
```

원본 `DA_HeavyShell`, `DA_ProtoTurretCannon`, `HeavyCannon`, `DA_TestSedan`, `BP_CFVehiclePawn`과 `M_VehicleDefensePIE`를 수정하지 않고 다음 PIE 전용 체인을 적용했다.

```text
/Game/CarFight/Tests/VehicleDefense/DA_HeavyShell_DRAP50
- DA_HeavyShell 복제
- ProjectileId = HeavyShell_DRAP50
- DefaultDamageData = DA_DamageArmorPenTest

/Game/CarFight/Tests/VehicleDefense/DA_ProtoCannon_DRAP50
- DA_ProtoTurretCannon 복제
- WeaponId = ProtoCannon_DRAP50
- DefaultProjectileData = DA_HeavyShell_DRAP50

/Game/CarFight/Tests/VehicleDefense/HeavyCannon_DRAP50
- HeavyCannon 복제
- EquipmentId = HeavyCannon_DRAP50
- DefaultWeaponData = DA_ProtoCannon_DRAP50
- 원본 DefaultTurretMountData = DA_CannonBody 유지

/Game/CarFight/Tests/VehicleDefense/DA_TestSedan_DRAP50
- DA_TestSedan 복제
- MountProfile DefaultEquipmentPresetData = HeavyCannon_DRAP50
- DefaultDefenseData = None 유지

/Game/CarFight/Tests/VehicleDefense/BP_CFVehiclePawn_DRAP50
- BP_CFVehiclePawn 복제
- CDO VehicleData = DA_TestSedan_DRAP50

/Game/CarFight/Tests/VehicleDefense/BP_CFSingleGameMode_DRAP50
- CFSingleGameMode 부모 Blueprint
- ConfiguredDefaultVehiclePawnClass = BP_CFVehiclePawn_DRAP50

/Game/Maps/M_VehicleDefensePIE_AP50
- M_VehicleDefensePIE 복제
- WorldSettings GameMode Override = BP_CFSingleGameMode_DRAP50
- 기존 방어 대상 SUV Actor와 DA_Fit_DefenseTestSUV 연결 유지
```

플레이어 차량은 맵에 배치된 Actor가 아니라 GameMode에서 생성되므로 AP50 Pawn과 GameMode를 함께 격리했다. 전역 `BP_CFVehiclePawn` CDO와 프로젝트 기본 GameMode는 변경하지 않았다.

검증 증거:

```text
Dry Run: 9ad852ec6e9f406888ce1ad801e0289c PASS
Apply: 33923b5739964e94824f64eb3da03b1c PASS
Report: UE/Saved/VehicleDefenseAP50Apply/report.json
AssetDump: VehicleDefense 테스트 폴더 9/9, Maps World 5/5 Success
Build: ad8bace5b6244ceeb8e90b9ede6547da PASS
Combat Automation: 593efc71a9bb40aba4f20e8af3e6243a / 46/46·필수 24/24 Success
```

적용 보고서는 AP50 직접 참조 체인, Pawn CDO, GameMode 기본 Pawn, 맵 GameMode Override와 원본 10개 파일의 SHA-256 전후 일치를 확인했다. 이 에셋은 DR-PIE-03 전용이며 원본 전투 에셋으로 승격하거나 자동 교체하지 않는다.

### DR-P0-07D — 실행 순서

```text
DR-PIE-00 시작 상태 확인
→ DR-PIE-01 Shield → Armor → Integrity
→ DR-PIE-04 Shield Regen
→ 새 PIE 세션에서 DR-PIE-02 방향별 Armor
→ AP50 전용 복제 준비 후 DR-PIE-03 관통 비교
→ DR-PIE-05 Legacy Fallback
→ DR-PIE-06 CF-FQ-029 Launcher·Rocket·Pool 회귀
```

사용자의 실제 결과가 제출되기 전에는 어느 항목도 PASS로 기록하지 않는다.

## DR-P1 — 후속 확장

```text
- Wheel / Engine / Turret Damage Zone
- 실제 DamageAppliedToComponents
- 모듈 4단계 상태와 기능 저하
- Radial Explosion
- Physical Impulse
- Collision / Environmental Damage
- Status Effect
- Repair
```

---

## 16. Automation 테스트 계획

테스트 파일 후보:

```text
UE/Source/CarFight_Re/Private/CFVehicleDefenseTests.cpp
```

테스트는 가능한 한 Transient Actor, Component와 DataAsset을 사용해 `.uasset` 저장값에 의존하지 않는다.

| 테스트 ID | Automation 이름 후보 | 입력 | PASS 기준 |
| --- | --- | --- | --- |
| DR-AUTO-01 | `CarFight.Damage.DR_P0_01.DataContract` | 정상·비정상 DefenseData | Clamp와 DataValidation 결과가 계약과 일치 |
| DR-AUTO-02 | `CarFight.Damage.DR_P0_02.ShieldFullAbsorb` | Shield 100, Damage 25 | Shield 75, Armor·Integrity 변화 0 |
| DR-AUTO-03 | `CarFight.Damage.DR_P0_03.ShieldOverflow` | Shield 10, Damage 25 | Shield 0, DamageAfterShield 15 |
| DR-AUTO-04 | `CarFight.Damage.DR_P0_04.DirectionResolve` | 6방향 ImpactLocation | Front/Left/Right/Rear/Top/Bottom 결정 일치 |
| DR-AUTO-05 | `CarFight.Damage.DR_P0_05.DirectionMultiplier` | 동일 피해·서로 다른 방향 | 각 DirectionalDamage가 설정 배율과 일치 |
| DR-AUTO-06 | `CarFight.Damage.DR_P0_06.NoPenetration` | AP 0, Armor 충분 | 남은 피해를 장갑이 흡수하고 Integrity 0 |
| DR-AUTO-07 | `CarFight.Damage.DR_P0_07.FullPenetration` | AP >= Resistance | Armor 감소 0, DirectionalDamage가 Integrity로 전달 |
| DR-AUTO-08 | `CarFight.Damage.DR_P0_08.PartialPenetration` | AP = Resistance 50% | 관통·차단 분리가 공식과 일치 |
| DR-AUTO-09 | `CarFight.Damage.DR_P0_09.ArmorOverflow` | Armor 부족 | 부족분이 Integrity로 전달 |
| DR-AUTO-10 | `CarFight.Damage.DR_P0_10.IntegrityDestroyOnce` | 치명 피해 2회 | 첫 타격만 DestroyedThisHit와 파괴 이벤트 발생 |
| DR-AUTO-11 | `CarFight.Damage.DR_P0_11.LegacyFallback` | DefaultDefenseData None | 기존 BaseDamage 직접 체력 감소와 결과 유지 |
| DR-AUTO-12 | `CarFight.Damage.DR_P0_12.SelfDamagePolicy` | Instigator == Target | bCanDamageSelf false 거부, true 허용 |
| DR-AUTO-13 | `CarFight.Damage.DR_P0_13.RejectNoMutation` | Missing DamageData 등 | 모든 방어값과 재생 지연 변화 없음 |
| DR-AUTO-14 | `CarFight.Damage.DR_P0_14.ShieldRegeneration` | 피해 후 시간 진행 | 지연 전 0 증가, 지연 후 Rate대로 증가, Max에서 정지 |
| DR-AUTO-15 | `CarFight.Damage.DR_P0_15.ResultInvariant` | 정상·초과 피해 | 결과 불변식이 허용 오차 안에서 성립 |
| DR-AUTO-16 | `CarFight.Damage.DR_P0_16.HitScanProjectileParity` | 동일 Context 핵심값 | 전달 방식과 무관하게 같은 방어 결과 |
| DR-AUTO-17 | `CarFight.Damage.DR_P0_17.ProjectileSingleImpact` | 동일 Projectile 중복 callback | 방어값이 한 번만 감소 |
| DR-AUTO-18 | `CarFight.Damage.DR_P0_18.HealthEventCompatibility` | Shield·Armor·Integrity 각각 피해 | 기존 Health 이벤트는 Integrity 변경 때만 발생 |

현재 실제 맵 수명 보호 회귀:

```text
CarFight.Fitting.FIT_P0_05.DefenseMapPIE
- /Game/Maps/M_VehicleDefensePIE 읽기 전용 로드
- 실제 PIE 시작과 UEDPIE 복제 SUV 식별
- BeginPlay, Initial Mass, Snapshot Commit, Weapon·Defense Ready 검증
- ActiveDefenseData=DA_VehicleDefense_Test 검증
- Shield 100, 6방향 Armor 각 100, Integrity 100 검증

CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE
- 플레이어 제어 차량 1대 이상과 대상 SUV를 함께 생성
- 전체 VehiclePawn 2대 이상과 PlayerControlled 1대 이상 검증
- 대상 SUV VehicleFittingComp·VehicleDefenseComp 각각 1개 검증
- C++ getter와 실제 단일 컴포넌트 인스턴스 동일성 검증
- 2-Pawn 상태에서도 대상 SUV 방어 초기값과 Commit 상태 검증
- 검증 뒤 PIE 종료, 임시 PlayerStart가 필요하면 제거, 맵·에셋 저장 없음
```

### 16.1 고정 수치 예제

부분 관통 테스트 예:

```text
BaseDamage = 40
Shield = 0
DirectionMultiplier = 1.0
ArmorResistance = 100
ArmorPenetration = 50
CurrentArmor = 100
Integrity = 100
```

예상:

```text
ArmorPenetrationRatio = 0.5
DirectPenetrationDamage = 20
ArmorBlockCandidate = 20
DamageAbsorbedByArmor = 20
DamageRequestedForIntegrity = 20
ArmorAfter = 80
IntegrityAfter = 80
```

장갑 초과 테스트 예:

```text
BaseDamage = 40
Shield = 0
DirectionMultiplier = 1.0
ArmorResistance = 100
ArmorPenetration = 0
CurrentArmor = 10
Integrity = 100
```

예상:

```text
DirectPenetrationDamage = 0
ArmorBlockCandidate = 40
DamageAbsorbedByArmor = 10
ArmorOverflowDamage = 30
DamageRequestedForIntegrity = 30
ArmorAfter = 0
IntegrityAfter = 70
```

---

## 17. 사용자 PIE 검증 계획

### DR-PIE-00 시작 상태 게이트

첫 Gate는 Details가 아니라 **실제 수동 PIE 로그**로 판정한다.

```text
로그 파일 = UE/Saved/Logs/CarFight_Re.log
검색 키 = CF-FQ-033 ManualPIEProbe
대상 Phase = BeginPlay.AfterInitialize / NextTick / AfterOneSecond
대상 Actor = VehicleFittingData가 DA_Fit_DefenseTestSUV인 BP_CFVehiclePawn
제외 Phase = PreRegister.BeforePrepare / EndPlay.AfterReset
```

PASS 전제:

```text
대상 SUV VehicleData = DA_VehicleDefense_TestSUV
대상 SUV VehicleFittingData = DA_Fit_DefenseTestSUV
FittingCount = 1 / DefenseCount = 1
InitialMass = Verified
FittingRuntime = CommittedSnapshot
DefenseData 초기화=예
ActiveDefenseData = DA_VehicleDefense_Test
피해 경로=Shield → Armor → Integrity
Shield 100 / 6방향 Armor 각 100 / Integrity 100
조작 무기 DamageData BaseDamage 25 / ArmorPenetration 0
```

자동 기준선 `CarFight.Fitting.FIT_P0_05.DefenseMapPIE`와 `DefenseMapTwoPawnPIE`는 위 대상 SUV 런타임 값을 PASS했다. 사용자 수동 DR-PIE-00도 ManualPIEProbe에서 VehicleData·FittingData, FittingCount 1, DefenseCount 1, InitialMass Verified, CommittedSnapshot, DefenseReady, Shield 100, 6방향 Armor 각 100과 Integrity 100을 확인해 PASS했다. 이후 실행에서는 Details의 `None / 0` 표시를 런타임 실패 근거로 사용하지 않고 같은 실행의 로그와 실제 피해 결과를 판정한다.

### DR-PIE-01 방어층 순서

AP 0, BaseDamage 25의 단발 공격을 대상 정면에 9회 적용한다. 각 발은 쉴드 재생이 시작되기 전인 5초 이내 간격으로 진행한다.

예상:

```text
1발: Shield 100→75 / Front Armor 100 / Integrity 100
2발: Shield 75→50 / Front Armor 100 / Integrity 100
3발: Shield 50→25 / Front Armor 100 / Integrity 100
4발: Shield 25→0 / Front Armor 100 / Integrity 100
5발: Shield 0 / Front Armor 100→75 / Integrity 100
6발: Front Armor 75→50 / Integrity 100
7발: Front Armor 50→25 / Integrity 100
8발: Front Armor 25→0 / Integrity 100
9발: Front Armor 0 / Integrity 100→75
10발: Integrity 75→50
11발: Integrity 50→25
12발: Integrity 25→0 / 이번 타격 차량 파괴=True
13발: 추가 Integrity 감소 없음 / 두 번째 파괴 전환 없음
```

PASS:

```text
Shield가 남은 동안 Armor·Integrity 변화 없음
Shield 0 이후 정면 Armor만 감소
정면 Armor 0 이후 Integrity 감소
마지막 결과 방향=Front
최초 Integrity 0 타격에서만 DestroyedThisHit와 파괴 FX·이벤트 발생
파괴 후 추가 타격에서 두 번째 파괴 전환이나 FX가 발생하지 않음
```

현재 무기가 Ripple처럼 입력 한 번에 여러 발을 발사하면 ‘클릭 수’가 아니라 실제 명중 수를 기준으로 기록한다.

현재 사용자 결과:

```text
DR-PIE-01 방어층 순서·파괴 전환·중복 파괴 방지 = USER PASS
- 4회 누적 명중: Shield 100→0 / Front Armor 100 / Integrity 100
- 9회 누적 명중: Shield 0 / Front Armor 0 / Integrity 75
- 후속 파괴 세션: Integrity 0 도달 확인
- 다른 5방향 Armor 유지 확인
- DA_VehicleDefense_TestSUV에 DA_FX_ProtoVehicleDead 연결 후 최초 파괴 폭발 사용자 확인
- 파괴된 테스트 SUV에 추가 피격 후 두 번째 파괴 폭발이 발생하지 않음을 사용자 확인
- OnVehicleDestroyed 1회와 DestroyedThisHit 1회 계약은 기존 Automation PASS 증거 유지
```

DR-PIE-01, DR-PIE-02와 수정 후 DR-PIE-04는 최종 수동 PASS로 닫았으며 다음 순서는 DR-PIE-03 관통 비교다.

### DR-PIE-02 방향별 장갑

새 PIE 세션에서 쉴드를 0으로 만든 뒤, Front·Left·Right·Rear에 AP 0 / BaseDamage 25를 한 발씩 적용한다.

각 방향을 독립 초기 상태에서 비교하는 것이 원칙이다. 가장 정확한 방법은 방향별로 PIE를 재시작하거나 `차량 방어 완전 초기화` 후 쉴드만 다시 제거하는 것이다.

한 발 예상:

```text
Front: 해당 Armor 100→75.0 / Armor 흡수 25.0 / Integrity 피해 0
Left:  해당 Armor 100→70.0 / Armor 흡수 30.0 / Integrity 피해 0
Right: 해당 Armor 100→70.0 / Armor 흡수 30.0 / Integrity 피해 0
Rear:  해당 Armor 100→62.5 / Armor 흡수 37.5 / Integrity 피해 0
```

PASS:

```text
마지막 결과 방향이 실제 피격면과 일치
피격 방향 Armor만 감소
다른 방향 Armor Pool은 유지
방향 배율이 Front 1.0 / Left·Right 1.2 / Rear 1.5 결과와 일치
```

Top·Bottom 사용자 조준은 P0 필수 수동 행렬에서 제외하며 Automation 방향 판정 PASS 증거를 유지한다.

사용자 검증 결과:

```text
Front 75 / Left 70 / Right 70 / Rear 62.5
각 방향에서 해당 Armor만 감소
다른 방향 Armor 100 유지
Integrity 100 유지
DR-PIE-02 USER PASS
```

### DR-PIE-03 관통 비교

AP 0과 AP 50은 서로 다른 격리 맵의 새 PIE에서 비교한다.

```text
AP 0 맵: /Game/Maps/M_VehicleDefensePIE
AP 50 맵: /Game/Maps/M_VehicleDefensePIE_AP50
```

각 맵에서 Shield 100을 정면 네 발로 0으로 만든 뒤, 재생이 시작되기 전에 정면 다섯 번째 한 발을 Armor 비교탄으로 적용한다.

동일한 초기 상태에서 Front Armor에 다음 두 DamageData를 각각 한 발 적용한다.

```text
AP 0:  DA_DamageAsset / BaseDamage 25 / ArmorPenetration 0
AP 50: DA_DamageArmorPenTest / BaseDamage 25 / ArmorPenetration 50
ArmorResistance 100
Shield 0
Front Armor 100
Integrity 100
```

예상:

```text
AP 0
- ArmorPenetrationRatio 0.0
- Front Armor 100→75
- Integrity 100 유지

AP 50
- ArmorPenetrationRatio 0.5
- Front Armor 100→87.5
- Integrity 100→87.5
- Armor 흡수 12.5
- Integrity 요청·적용 12.5
```

PASS:

```text
AP 50에서 Armor 감소량은 더 작고 Integrity 피해는 더 큼
AP 0에서는 충분한 Armor가 모든 피해를 흡수
두 결과의 BaseDamage는 동일한 25
```

사용자 검증 결과:

```text
AP0: Front Armor 75 / Integrity 100 / Armor 흡수 25 / Integrity 피해 0
AP50: Front Armor 87.5 / Integrity 87.5 / Armor 흡수 12.5 / Integrity 피해 12.5
동일 BaseDamage 25 확인
AP50 관통률 0.5 분배 확인
DR-PIE-03 USER PASS
```

### DR-PIE-04 쉴드 재생

새 PIE 세션에서 AP 0 한 발로 Shield를 100→75로 만든 뒤 사격을 중지한다.

예상:

```text
피격 직후~5초 이전: Shield 75 유지
5초 경과 뒤: 초당 약 10씩 증가
재생 시작 약 1초 후: Shield 약 85
최종: Shield 100에서 정지
재생 중 재피격: 재생 중지 후 지연 5초 재시작
```

PASS:

```text
지연 중 증가 없음
지연 종료 후 증가
재피격 시 지연 재시작
최대 100 초과 없음
```

프레임 경계 때문에 관찰값에 작은 오차는 허용하되 장시간 선행 재생 또는 100 초과는 허용하지 않는다.

사용자 재검증 결과:

```text
남은 재생 지연 5→0 확인
Shield 50→100 재생 확인
재생 중 피격 시 지연 5초 재시작 확인
DR-PIE-04 USER PASS
```

### DR-PIE-05 Legacy 호환

사용자 PIE 전용 맵은 `/Game/Maps/M_VehicleDefensePIE_Legacy`다. 이 맵은 원본 `M_VehicleDefensePIE`를 복제한 뒤 비소유 대상 `BP_CFVehiclePawn_C_1`만 다음 상태로 저장했다.

```text
VehicleData=/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV
VehicleFittingData=None
AutoPossessPlayer=Disabled
DefaultDefenseData=None
Maximum Integrity=100
WorldSettings.DefaultGameMode=/Script/CarFight_Re.CFSingleGameMode
```

플레이어는 기존 GameMode가 생성하는 `DA_TestSedan → HeavyCannon → DA_ProtoTurretCannon → DA_HeavyShell → DA_DamageAsset` AP0 Projectile 체인을 그대로 사용한다. `DA_DamageAsset`은 BaseDamage 25, ArmorPenetration 0이다.

사용자 절차:

```text
1. Tools/RunEditor.bat로 현재 빌드의 에디터를 실행한다.
2. /Game/Maps/M_VehicleDefensePIE_Legacy를 연다.
3. 새 PIE를 시작하고 배치된 SUV를 선택 대상으로 확정한다.
4. VehicleDebug → 선택 대상 → 대상 방어에서 초기값을 확인한다.
5. 기존 Heavy Cannon Projectile 한 발을 대상 SUV에 명중시킨다.
6. 같은 대상 방어·내구도와 마지막 전체 피해 결과를 확인한다.
```

한 발 기대값:

```text
방어 데이터=LegacyFallback / 초기화되지 않음
Shield=0/0 유지
Front·Left·Right·Rear·Top·Bottom Armor=0 유지
Integrity=100→75
마지막 결과 Legacy=예
요청 피해=25
Shield 흡수=0
Armor 흡수=0
Integrity 요청/적용=25/25
파괴=아니오
```

PASS:

```text
피해 경로=Projectile Legacy Integrity Fallback
Shield·Armor 값 0 유지
BaseDamage 25가 Integrity에 직접 적용
기존 Health 변경·Damage 이벤트 의미 유지
원본 에셋 재저장 요구 없음
```

기존 Destroyed 이벤트 의미는 필요하면 같은 PIE 또는 새 PIE에서 총 네 발로 Integrity를 0까지 낮춰 최초 파괴 이벤트·폭발이 한 번만 발생하고, 파괴 후 추가 피격에서 두 번째 파괴 이벤트·폭발이 없는지 확인한다. 한 발 결과가 정확하고 기존 DR-PIE-01의 최초 파괴·추가 피격 이벤트 USER PASS를 그대로 회귀 증거로 사용할 수 있으면 중복 파괴 검증은 선택 사항이다.

현재 에셋에서 명시적 HitScan 장비가 확인되지 않았으므로 새 HitScan 에셋을 만들지 않는다. Projectile Legacy Fallback은 사용자 PIE로 검증하고 HitScan·Projectile 동일 결과는 `CarFight.Damage.DR_P0_03.RuntimeIntegration`과 전체 Combat Automation PASS 증거로 유지한다.

사용자 검증 결과:

```text
Legacy=예
Shield 0 유지
Armor 6방향 0 유지
Integrity 100→75
Integrity 요청/적용 25/25
DR-PIE-05 USER PASS
```

### DR-PIE-06 CF-FQ-029 회귀

원본 `TestMap`과 Launcher·Projectile 에셋을 재저장하지 않고 다음 두 격리 맵에서 검증한다.

```text
Salvo: /Game/Maps/TestMap_DRSalvo
Ripple: /Game/Maps/TestMap_DRRipple
Player Vehicle Actor: BP_CFVehiclePawn_C_1 / AutoPossessPlayer=Player0
VehicleFittingData=None
GameMode=/Script/CarFight_Re.CFSingleGameMode
```

격리 체인:

```text
Salvo:
DA_TestSUV_DRSalvo
→ /Game/CarFight/Weapons/Data/EquipmentPresets/RocketLauncher
→ DA_RocketLauncher
→ DA_Rocket_PropTest

Ripple:
DA_TestSUV_DRRipple
→ RocketLauncher_DRRipple
→ DA_RocketLauncher_DRRipple
→ DA_Rocket_PropTest
```

저장 계약:

```text
Salvo: Pattern=Salvo / ProjectileCount=4 / MaximumSimultaneous=4
Ripple: Pattern=Ripple / ProjectileCount=4 / InterMuzzleDelay=0.15s / MaximumSimultaneous=1
공통 Muzzle 순서: Muzzle_1 → Muzzle_4 → Muzzle_2 → Muzzle_3
공통 정책: ContinueRemaining / SequenceCompleted / Direct
Projectile: InitialSpeed 3000 / Ignition 0.05s / Burn 1.0s / Acceleration 9000 / MaxSpeed 10000
Trail=NS_RibbonTrail / Thruster=NS_RocketExhaust_Realistic / Impact=DA_FX_ProtoShellImpact
```

사용자 절차:

```text
1. Tools/RunEditor.bat로 현재 빌드의 에디터를 실행한다.
2. TestMap_DRSalvo를 열고 새 PIE를 시작한다.
3. 기존 발사 입력 한 번으로 4발이 같은 Salvo 묶음으로 사출되는지 확인한다.
4. 창(Window) → 개발자 도구(Developer Tools) → 출력 로그(Output Log)에서 VehicleWeaponFireOrigin을 필터링한다.
5. Muzzle_1 → Muzzle_4 → Muzzle_2 → Muzzle_3 로그와 MuzzleCount=4를 확인한다.
6. PIE World Outliner의 Player SUV에서 LauncherComp를 선택하고 런처 시퀀스 요약을 확인한다.
7. 최소 5 Volley를 반복하고 각 Volley가 4발 완결·누락 없음·잔류 없음인지 확인한다.
8. PIE를 종료하고 TestMap_DRRipple에서 같은 절차를 반복한다.
9. Ripple 한 입력에서 약 0.15초 간격으로 4발이 순차 사출되고 진행 중 추가 입력이 중복 시퀀스를 만들지 않는지 확인한다.
10. 두 맵에서 Rocket 추진·Trail·Thruster·Impact FX, 같은 차량 Projectile 상호 비충돌, 일반 차량·월드 충돌을 확인한다.
```

완료 시 LauncherComp `런처 시퀀스 요약` 기대값:

```text
State=Completed
Cancel=None
Pattern=Salvo 또는 Ripple
Total=4
Attempted=4
Accepted=4
Failed=0
Remaining=0
Pending=0
CooldownApplied=True
```

사용자 검증 결과:

```text
Salvo 4발 동시 정상
Salvo Muzzle 1→4→2→3 정상
Salvo Sequence Completed / Accepted 4 / Failed 0
Salvo 5 Volley 이상 정상

Ripple 4발 0.15초 순차 정상
Ripple Muzzle 1→4→2→3 정상
Ripple Sequence Completed / Accepted 4 / Failed 0
Ripple 진행 중 중복 입력 방지 정상
Ripple 5 Volley 이상 정상

추진 정상
Trail 정상
Thruster 정상
Impact FX 정상
동일 차량 Projectile 상호 Impact 없음
일반 차량 충돌 정상
월드 충돌 정상
DR-PIE-06 USER PASS
```

DR-PIE-00~06이 모두 USER PASS이므로 DR-P0-07 전체를 PASS 처리하고 CF-FQ-033을 Done으로 판정한다.

---

## 18. 완료 판정

`CF-FQ-033` P0는 다음이 모두 충족되어야 Done으로 판정한다.

```text
- Shield → Directional Armor → Integrity 순서 구현
- 6방향 독립 Armor Pool 구현
- 방향 배율과 관통 공식 구현
- 쉴드 재생 구현
- HitScan·Projectile 공통 Defense 진입점 구현
- DefaultDefenseData None Legacy Fallback 구현
- 기존 Health·Destroyed 이벤트 호환
- Projectile 첫 Impact 1회와 Pool 회귀 유지
- 전용 Automation PASS
- 공식 CarFight Editor Build PASS
- 사용자 PIE DR-PIE-01~06 PASS
- Systems/Combat/VehicleDefense.md Current System 승격
- Systems/Combat/HitDamage.md 호환 관계 갱신
```

모든 완료 조건이 실제 코드·에셋, 공식 Build, 전체 Automation, 사용자 DR-PIE-00~06과 Current System 승격으로 충족됐다. `CF-FQ-033`은 Done이다.

---

## 19. 확정 결정 요약

```text
1. VehicleHealthComp를 삭제하지 않는다.
2. VehicleHealthComp의 Health는 Vehicle Integrity로 사용한다.
3. VehicleDefenseComp가 Shield와 방향별 Armor를 소유한다.
4. DamageData 직접 참조는 ProjectileData.DefaultDamageData 하나다.
5. DamageType은 Kinetic / Explosive / Energy를 유지한다.
6. ArmorPenetration은 피해 타입이 아니라 장갑 관통 수치다.
7. Armor는 Front / Left / Right / Rear / Top / Bottom 독립 Pool이다.
8. 방향 판정은 Bounds 중심→ImpactLocation 로컬 지배 축을 기본으로 한다.
9. P0 관통은 ArmorPenetration / ArmorResistance의 결정적 선형 공식이다.
10. P0에서 도탄, 입사각 유효 두께와 관통탄 장갑 마모를 추가하지 않는다.
11. 쉴드는 방향 배율과 관통 전에 원본 피해를 1:1 흡수한다.
12. DefaultDefenseData가 없으면 기존 직접 Health 피해를 유지한다.
13. 기존 FCFDamageApplyResult와 Health 이벤트를 유지한다.
14. 부품 피해, 범위 피해와 Impulse는 DR-P1이다.
15. CF-FQ-029의 dirty 소스·에셋을 초기 Foundation에서 건드리지 않는다.
```

---

## 20. 관련 경로

```text
Document/Systems/Combat/VehicleDefense.md
Document/Systems/Combat/HitDamage.md
Document/Systems/Combat/DamageHitContext.md
Document/Systems/Combat/Projectile.md
Document/ProjectSSOT/CombatPlan/08_DefenseArmor.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/Plan/LauncherMissilePlan.md
UE/Source/CarFight_Re/Public/CFDamageData.h
UE/Source/CarFight_Re/Public/CFDamageTypes.h
UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h
UE/Source/CarFight_Re/Private/CFVehicleHealthComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleData.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

---

## 21. Changelog

### v0.23.1 - 2026-08-22

- 문서 정상화 Health Check에서 완료 문서 상단의 `Current Active Feature: CF-FQ-032`와 old Next Active를 2026-08-06 완료 당시 Historical handoff로 명시했다.
- CF-FQ-033 완료 evidence와 당시 CF-FQ-032 전환 사실은 그대로 보존했다. 현재 Active/next feature 판단은 main_game FeatureQueue/ActiveWork를 우선한다.

Migration: 이 문서는 Historical / Retained Path다. `Current`/`Next`라는 과거 표현은 현재 작업 지시로 사용하지 않는다.

### v0.23.0 - 2026-08-06

- 사용자 DR-PIE-06에서 Salvo 4발 동시, Muzzle 1→4→2→3, Sequence Completed·Accepted 4·Failed 0과 5 Volley 이상 반복을 확인했다.
- Ripple 4발·0.15초 순차, Muzzle 1→4→2→3, Sequence Completed·Accepted 4·Failed 0, 진행 중 중복 입력 방지와 5 Volley 이상 반복을 확인했다.
- Rocket 추진·Trail·Thruster·Impact FX, 동일 차량 Projectile 상호 비충돌과 일반 차량·월드 충돌을 모두 확인해 DR-PIE-06을 USER PASS로 승격했다.
- DR-PIE-00~06 전체 USER PASS에 따라 DR-P0-07을 Done으로 판정했다.
- `Document/Systems/Combat/VehicleDefense.md v1.0.0`을 Current System으로 신규 작성하고 `HitDamage.md v1.1.0`과 `SystemIndex.md v1.15.0`을 통합 기준으로 갱신했다.
- `CF-FQ-033 차량 방어·손상 런타임`을 Done으로 전환하고 다음 Active를 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`로 복원했다.
- 이번 마감은 사용자 PIE 결과와 문서 동기화만 수행했으며 코드·에셋·빌드·Automation을 다시 변경하거나 실행하지 않았다.
- 기존 미커밋 변경을 보호했고 commit·push는 수행하지 않았다.

### v0.22.0 - 2026-08-06

- DR-PIE-06을 원본 `TestMap`과 Launcher·Projectile 에셋을 재저장하지 않는 Salvo·Ripple 격리 맵 방식으로 준비했다.
- `/Game/CarFight/Tests/LauncherRegression/`에 `DA_RocketLauncher_DRRipple`, `RocketLauncher_DRRipple`, `DA_TestSUV_DRSalvo`, `DA_TestSUV_DRRipple`을 생성했다.
- `/Game/Maps/TestMap_DRSalvo`와 `/Game/Maps/TestMap_DRRipple`을 생성하고 기존 CF-FQ-029 Launcher 슬롯 `BP_CFVehiclePawn_C_1`을 Player0·VehicleFittingData=None으로 연결했다.
- Salvo는 원본 4발 동시 설정을 유지하고 Ripple은 4발·0.15초·최대 동시 처리 1로 설정했으며 공통 Muzzle_1→4→2→3, ContinueRemaining, SequenceCompleted, Direct와 원본 Rocket Projectile 참조를 유지했다.
- `ApplyLauncherRegressionTest.py v1.0.0`과 `RunLauncherRegressionTest.ps1 v1.0.0`의 최종 Dry Run `b71e4b9e8242412c8abe31ae13994918`과 Apply `f1566e9cc6a647dea7ba0a99b11ee68f`가 Exit Code 0으로 통과했다.
- 원본 TestMap·Weapon·Preset·Turret·Projectile·Vehicle·Pawn·HeavyCannon 8개 파일의 SHA-256 전후 일치를 확인했다.
- AssetDump는 LauncherRegression 4/4, Maps World 8/8 Success였으며 두 World의 SUV·RocketLauncher PitchMesh와 Muzzle_1~4 소켓을 확인했다.
- 공식 Build `48c0a81e19af4b20a17f628bcc7b723b`와 Combat Automation `123e4433f5fd4b2a8ea1c1d991b07498`이 Exit Code 0, 전체 46/46·필수 24/24 Success·실패 0으로 통과했다.
- MuzzleSequence·FirePattern·Scheduler·Release·SourceIsolation·Propulsion과 Damage RuntimeIntegration 필수 회귀가 모두 Success다.
- DR-PIE-06은 사용자 Salvo·Ripple·Pool·FX·충돌 결과 Pending이며 PASS 또는 DR-P0-07 완료 판정을 수행하지 않았다.
- 기존 미커밋 변경과 원본 에셋을 보호했고 commit·push는 수행하지 않았다.

### v0.21.0 - 2026-08-06

- 사용자 DR-PIE-05에서 마지막 피해 결과 `Legacy=예`를 확인했다.
- Shield 0과 Front·Left·Right·Rear·Top·Bottom Armor 0이 피격 전후 그대로 유지됨을 확인했다.
- BaseDamage 25 한 발이 Armor 분배 없이 Integrity에 직접 적용돼 Integrity 100→75와 Integrity 요청/적용 25/25가 일치함을 확인했다.
- 기존 Projectile 경로가 `DefaultDefenseData=None` 차량에서 Legacy Integrity Fallback으로 연결되는 것을 확인해 DR-PIE-05를 USER PASS로 승격했다.
- 명시적 HitScan 테스트 에셋은 추가하지 않고 기존 RuntimeIntegration·Combat Automation PASS를 HitScan·Projectile 동등성 증거로 유지했다.
- 다음 실행을 마지막 사용자 게이트인 DR-PIE-06 CF-FQ-029 Launcher·Projectile 회귀로 이동했다.
- 코드·에셋·빌드·Automation은 추가 변경하거나 재실행하지 않았고 기존 미커밋 변경과 원본 에셋을 보호했으며 commit·push도 수행하지 않았다.

### v0.20.0 - 2026-08-06

- 원본 `M_VehicleDefensePIE`를 재저장하지 않고 `/Game/Maps/M_VehicleDefensePIE_Legacy`만 생성하는 DR-PIE-05 격리 경로를 추가했다.
- 저장 후 재로드한 대상 `BP_CFVehiclePawn_C_1`이 `DA_TestSUV`, `VehicleFittingData=None`, `AutoPossessPlayer=Disabled`를 유지하고 맵 GameMode가 `CFSingleGameMode`로 보존됨을 확인했다.
- `DA_TestSUV.DefaultDefenseData=None`, Maximum Integrity 100과 `DA_DamageAsset` BaseDamage 25·ArmorPenetration 0 계약을 확인했다.
- Dry Run `8fbf901a30464338984b235ef981a879`과 Apply `923a8e94a7a84ed4b516070b8b070107`이 Exit Code 0으로 통과했고, 원본 맵·차량·피팅·Pawn·AP0 공격 체인 10개 파일의 SHA-256 전후 일치를 확인했다.
- Maps AssetDump 24/24 Success와 `M_VehicleDefensePIE_Legacy` World 존재·대상 Pawn 직렬화를 독립 확인했다. Actor의 VehicleData·Fitting 값은 Apply 후 저장·재로드 보고서로 교차검증했다.
- 공식 Build `c93e71d77e0c45498d66da8b20c0227a`와 Combat Automation `73a7ce48758642d4800c79cfd9c66990`이 Exit Code 0, 전체 46/46·필수 24/24 Success·실패 0으로 통과했다.
- DR-PIE-05는 사용자 Projectile 한 발의 Legacy Integrity Fallback 결과 Pending이며 PASS 처리하지 않았다.
- 기존 미커밋 변경과 원본 에셋을 보호했고 commit·push는 수행하지 않았다.

### v0.19.0 - 2026-08-06

- 사용자 DR-PIE-03 AP0 결과에서 Front Armor 75, Integrity 100, Armor 흡수 25, Integrity 피해 0을 확인했다.
- 사용자 DR-PIE-03 AP50 결과에서 Front Armor 87.5, Integrity 87.5, Armor 흡수 12.5, Integrity 피해 12.5를 확인했다.
- 동일 BaseDamage 25에서 AP50의 장갑 감소량이 더 작고 피해 절반이 Integrity에 적용돼 ArmorPenetrationRatio 0.5 계약을 검증했다.
- AP50 전용 Projectile→Weapon→Preset→VehicleData→Pawn→GameMode→Map 경로가 실제 사용자 PIE에서도 올바른 DamageData를 전달한 것을 확인했다.
- DR-PIE-03을 USER PASS로 승격하고 다음 실행을 DR-PIE-05 Legacy 호환으로 이동했다.
- 코드·에셋·빌드·Automation은 추가 변경하거나 재실행하지 않았고 기존 미커밋 변경과 원본 에셋을 보호했으며 commit·push도 수행하지 않았다.

### v0.18.0 - 2026-08-06

- DR-PIE-03용 AP50 Projectile·Weapon·Preset·Sedan DataAsset 체인과 전용 Pawn Blueprint, GameMode Blueprint, `M_VehicleDefensePIE_AP50` 격리 맵을 생성했다.
- 플레이어가 맵 배치 Actor가 아니라 `CFSingleGameMode`에서 생성되는 구조를 확인해 AP50 Pawn CDO와 맵별 GameMode Override를 사용하도록 설계를 보정했다.
- `ApplyDefenseAP50Test.py v1.1.0`과 `RunDefenseAP50Test.ps1 v1.1.0`의 Dry Run `9ad852ec6e9f406888ce1ad801e0289c`와 Apply `33923b5739964e94824f64eb3da03b1c`가 성공했다.
- 적용 보고서에서 직접 참조 체인, Pawn CDO, GameMode 기본 Pawn, 맵 Override와 원본 10개 에셋 SHA-256 전후 일치를 확인했다.
- AssetDump는 VehicleDefense 테스트 폴더 9/9, Maps World 5/5 Success이며 AP50 DamageData→Projectile→Weapon→Preset→VehicleData→Pawn→GameMode 참조를 독립 확인했다.
- 공식 Build `ad8bace5b6244ceeb8e90b9ede6547da`와 Combat Automation `593efc71a9bb40aba4f20e8af3e6243a`가 Exit Code 0, 전체 46/46·필수 24/24 Success·실패 0으로 통과했다.
- DR-PIE-03은 AP0 원본 맵과 AP50 격리 맵의 사용자 비교 Pending이며 PASS 처리하지 않았다.
- 기존 미커밋 변경과 원본 전투 에셋을 보호했고 commit·push는 수행하지 않았다.

### v0.17.0 - 2026-08-06

- DR-PIE-02 사용자 검증에서 Front Armor 75, Left 70, Right 70, Rear 62.5를 확인했다.
- 각 방향에서 피격 방향 Armor만 감소하고 다른 방향 Armor가 100을 유지하는 것을 확인했다.
- 네 방향 모두 Integrity가 100을 유지해 Armor가 피해를 전부 흡수하는 것을 확인했다.
- DR-PIE-02를 USER PASS로 승격하고 다음 실행을 DR-PIE-03 AP 0·AP 50 관통 비교로 이동했다.
- 코드·에셋·빌드·Automation은 추가 변경하거나 재실행하지 않았고 commit·push도 수행하지 않았다.

### v0.16.0 - 2026-08-06

- 수정 후 DR-PIE-04 사용자 재검증에서 남은 재생 지연이 5→0으로 감소하는 것을 확인했다.
- Shield가 50→100으로 재생되고 최대값 100에서 정지하는 것을 확인했다.
- 재생 중 추가 피격 시 재생이 중단되고 재생 지연이 다시 5초로 초기화되는 것을 확인했다.
- DR-PIE-04를 USER PASS로 승격하고 다음 실행을 DR-PIE-02 방향별 장갑으로 이동했다.
- 코드·에셋·빌드·Automation은 추가 변경하거나 재실행하지 않았고 commit·push도 수행하지 않았다.

### v0.15.0 - 2026-08-06

- 사용자의 첫 DR-PIE-04에서 두 발 명중 후 Shield 50 고정·재생 없음 FAIL을 기록했다.
- 실제 재생 함수의 `IsActive()` Gate와 실제 Pawn 초기화의 활성 상태 미보장, 자동화 Fixture의 `Activate(true)` 선행 차이를 원인으로 확정했다.
- `UCFVehicleDefenseComp v1.3.0`에서 유효 DefenseData 초기화 시 활성 상태를 복구하고 초기 Tick은 정지, 실제 피해 뒤에만 재생 Tick을 켜도록 수정했다.
- ShieldRegeneration 자동화에 비활성 컴포넌트 재현·활성 복구 검증을 추가했다.
- 최종 공식 Build `c3a867d4233843f9b3df8116f332de65`와 Combat Automation `e798effcb81943748b27940702c5c095`가 Exit Code 0, 전체 46/46·필수 24/24 Success로 통과했다.
- DR-PIE-04는 수정 후 사용자 재검증 Pending으로 유지하며 PASS 처리하지 않았다.
- 기존 미커밋 변경과 에셋을 보호했고 commit·push는 수행하지 않았다.

### v0.14.0 - 2026-08-05

- 사용자가 파괴된 테스트 SUV에 추가 피격을 가한 뒤 두 번째 파괴 폭발이 발생하지 않음을 확인해 DR-PIE-01을 최종 USER PASS로 닫았다.
- DR-PIE-04 대상 Shield 재생 관찰을 위해 VehicleDebug Snapshot에 현재 선택 대상의 TargetSelect 표시·추적, Defense·Integrity 상태를 추가했다.
- VehicleDebug Panel에 `선택 대상` Navigation 섹션과 대상 방어·내구도·추적 수명 하위 섹션을 C++ 동적 ViewData로 추가했으며 WBP 에셋은 수정하지 않았다.
- Panel 갱신은 전체 VehicleDebug Snapshot을 프레임당 한 번만 읽도록 정리해 선택 대상 컴포넌트 검색의 반복을 방지했다.
- 공식 Build `bcf0a935bdb64dd5a8a3f999a8832e30`은 전체 성공했다. 최종 주석·Migration 정리 뒤 재빌드는 CarFight 소스 컴파일과 모듈 DLL 링크를 통과했으나 기존 `UnrealEditor-Cmd.exe`의 `UnrealEditor-NetCore.dll` 점유로 전체 preset 링크가 차단됐다.
- 파일 점유 상태에서 중복 Commandlet 실행을 시작하지 않아 변경 후 Automation은 미실행이며, 다음 단계는 점유 해소 후 Build·Automation 재검증과 DR-PIE-04다.
- 기존 미커밋 변경과 에셋을 보호했고 commit·push는 수행하지 않았다.

### v0.13.0 - 2026-08-05

- 사용자 수동 DR-PIE-00에서 ManualPIEProbe 기준 InitialMass Verified, CommittedSnapshot, DefenseReady, Shield 100, 6방향 Armor 각 100과 Integrity 100을 확인해 PASS했다.
- DR-PIE-01 AP 0·BaseDamage 25 정면 누적 명중에서 Shield 100→0, Front Armor 100→0, Integrity 100→75와 최종 Integrity 0을 확인해 방어층 순서·정면 방향·내구도 고갈을 PASS했다.
- Integrity 0에서도 폭발이 보이지 않은 원인을 `DA_VehicleDefense_TestSUV.DefaultDestroyedFxData=None`인 테스트 자산 구성 누락으로 확정했다.
- 사용자가 `DA_VehicleDefense_TestSUV.DefaultDestroyedFxData`에 `DA_FX_ProtoVehicleDead`를 연결하고 실제 파괴 폭발을 확인했다.
- DR-PIE-01의 남은 수동 항목을 파괴 후 추가 피격의 두 번째 폭발 없음 확인으로 제한하고, 이후 실행 순서를 DR-PIE-04 → DR-PIE-02 → DR-PIE-03 → DR-PIE-05 → DR-PIE-06으로 고정했다.
- 문서 체크포인트만 갱신했으며 추가 코드 수정·빌드·Automation·commit·push는 수행하지 않았다.

### v0.12.0 - 2026-08-04

- 사용자 수동 PIE와 같은 플레이어 차량 + 대상 SUV 2-Pawn 환경을 재현하는 `CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE`를 추가했다.
- 대상 SUV의 VehicleFittingComp와 VehicleDefenseComp가 각각 한 개이고 C++ getter와 같은 인스턴스임을 검증해 중복 컴포넌트 가능성을 제외했다.
- `CF-FQ-033 ManualPIEProbe`로 PreRegister, BeginPlay 초기화 전후, 다음 Tick, 1초 후와 EndPlay 전후의 실제 컴포넌트 주소와 피팅·방어 상태를 기록하도록 했다.
- 대상 SUV는 BeginPlay 초기화 직후부터 EndPlay 직전까지 `Verified / CommittedSnapshot / DefenseReady=Yes / ActiveDefenseData=DA_VehicleDefense_Test / Shield 100 / 6방향 Armor 각 100 / Integrity 100`을 유지했고 `Uninitialized` 복귀는 PIE 종료 Reset에서만 발생했다.
- Details의 `Uninitialized / None / 0` 표시는 실행 중 실제 컴포넌트 상태와 불일치하므로 DR-PIE-00의 관찰 방법을 Details에서 `CarFight_Re.log` ManualPIEProbe 직접 판정으로 변경했다.
- 공식 Editor Build `22528e83dfff4d6fa92c06dc34397198`, 2-Pawn PIE Process `a4e8048231034105902498f3ec8499f7`, 전체 CarFight Automation `2abca3c385114209890ffb8a820ac283` / 46/46·필수 24/24 Success를 기록했다.
- 사용자 DR-PIE-00 수동 로그 확인과 DR-PIE-01~06은 아직 Pending이며 CF-FQ-033 Done 또는 Systems Current 승격은 수행하지 않았다.

### v0.11.0 - 2026-08-04

- 사용자 PIE에서 대상 SUV가 `InitialMass/FittingRuntime=Uninitialized`, `ActiveDefenseData=None`, 방어값 0으로 보였던 상태를 실제 저장 맵 PIE 수명으로 재검증했다.
- `CarFight.Fitting.FIT_P0_05.DefenseMapPIE`를 추가해 `M_VehicleDefensePIE`를 실제 PIE로 열고 `UEDPIE` 복제 SUV의 BeginPlay, Initial Mass, Snapshot Commit, ActiveDefenseData와 Shield·6방향 Armor·Integrity 초기값을 검증했다.
- 실제 PIE 복제 SUV는 `BeginPlay=Yes`, `InitialMass=Verified`, `CommittedSnapshot`, `DefenseReady=Yes`, `Shield=100`, 6방향 Armor 각 100, `Integrity=100`으로 PASS했다.
- 월드 아웃라이너에서 에디터 원본 Actor와 PIE 복제 Actor를 구분하도록 `UEDPIE_0_M_VehicleDefensePIE` 경로 확인을 DR-PIE-00 필수 Gate로 추가했다.
- 공식 Editor Build `70015f640d124c97b22403f97c0717cb`, 단일 맵 PIE Process `994ac65e0df942a99974f917c7306f40`, 전체 CarFight Automation `b699a2ddcc824d7eb553674e7b681c07` / 45/45·필수 23/23 Success를 기록했다.
- 사용자 DR-PIE-01~06은 아직 Pending이며 CF-FQ-033 Done 또는 Systems Current 승격은 수행하지 않았다.

### v0.10.0 - 2026-08-03

- 사용자 결정에 따라 `CF-FQ-033 차량 방어·손상 런타임`을 CarFight 작업 완료 최우선 사항으로 전환했다.
- 문서 상태를 Ready Plan에서 Active Plan으로 변경하고 현재 실행을 DR-P0-07 사용자 PIE로 고정했다.
- DR-PIE 실패 시 해당 데미지 결함을 다른 신규 기능보다 먼저 수정하고 공식 Build·전체 Automation·영향 PIE를 재검증하도록 했다.
- DR-P0-07 전체 PASS 뒤 VehicleDefense·HitDamage Systems 동기화와 CF-FQ-033 Done을 수행하는 완료 경로를 명시했다.
- 데미지 PIE를 직접 막는 빌드·에셋·입력·테스트 환경 결함만 예외 선행으로 허용하고 해소 즉시 이 Plan으로 복귀하도록 했다.
- CF-FQ-032 UI와 런처·미사일·피팅·인벤토리·TargetSelect 체크포인트는 후순위로 보존했다.

### v0.9.0 - 2026-07-31

- DR-P0-07을 `PIE Procedure Ready / User Result Pending` 상태로 구체화했다.
- 현재 TestMap의 Sedan·HeavyCannon / SUV·RocketLauncher 진입과 `DA_VehicleDefense_TestSUV`가 자동 배치되지 않은 사실을 분리했다.
- CF-FQ-029 원본 맵·Pawn·Launcher·Projectile·Pool·추진·요격 에셋을 재저장하지 않는 복제 맵 기반 보호 절차를 추가했다.
- VehicleDefenseComp 런타임 Details와 마지막 전체 피해 결과를 사용한 관찰 방법을 추가했다.
- BaseDamage 25·AP 0 기준 Shield 100→Armor 100→Integrity 100의 발별 예상값을 고정했다.
- Front·Left·Right·Rear 방향 배율별 한 발 예상 장갑 감소량을 추가했다.
- AP 50 비교 에셋이 자동 연결되지 않음을 명시하고 원본 대신 PIE 전용 Projectile·Weapon·Preset·VehicleData 복제 체인을 사용하도록 제한했다.
- 쉴드 5초 지연·초당 10 재생, Legacy Fallback과 CF-FQ-029 회귀의 실제 PASS 기준을 확장했다.
- 읽기 전용 `ue.batchdump_safe` 재검증은 Windows `WinError 87`로 실행되지 않았으며 기존 Apply Report와 AssetDump 2/2 PASS 증거를 사용했다.
- 구현·Build·Automation·PIE는 실행하지 않았고 DR-P0-07 PASS, CF-FQ-033 Done과 Systems Current 승격을 수행하지 않았다.

### v0.8.0 - 2026-07-31

- 독립 AssetDump `ADumpEntityQuery.cpp`의 이전 `MakeStringArray` 호출부가 `MakeEntityQueryStringArray`로 정리된 현재 소스를 확인했다.
- 공식 Build Job `0697e0552a0f4911a93384c5d3a8a928`에서 `CarFight_ReEditor Win64 Development`가 Exit Code 0, `Result: Succeeded`로 통과했다.
- UBT는 현재 Target이 up to date임을 판정했으며 AssetDump 컴파일 차단이 실제 Consumer Editor Build에서 더 이상 재현되지 않았다.
- 기존 Defense Automation 8/8과 전체 Combat Automation 필수 15/15·전체 24/24 PASS 증거를 유지하고 재실행하지 않았다.
- `DR-P0-06`을 Done으로 전환하고 다음 단계를 `DR-P0-07 사용자 PIE`로 이동했다.
- AssetDump 소스, `WBP_TargetSelect`, CF-FQ-029 Launcher·Pool·추진·요격 dirty 변경과 관련 에셋은 직접 수정하지 않았다.
- `DR-P0-07`, CF-FQ-033 전체 Done과 `Document/Systems/Combat/VehicleDefense.md` Current 승격은 수행하지 않았다.

### v0.7.0 - 2026-07-31

- DR-P0-06 공식 Editor Build와 전체 Automation을 재검증했다.
- Defense Automation Process `7b5dce50680f47a99a29b26b02a577d7`에서 CarFight.Damage 8/8 Success, 실패·경고·미실행 0과 Exit Code 0을 확인했다.
- Combat Runtime Process `54340630094a461ea3f7a1056622fd3c`에서 필수 15/15·전체 24/24 Success, 실패 0과 Editor Exit Code 0을 확인했다.
- 이전 `WBP_TargetSelect.uasset` 파일 잠금과 `TS_P0_06.TargetHud` 실패가 해소됐음을 확인했다.
- 공식 Build Job `15d79dff7c6c4e8680bde3eb4052b27a`는 독립 AssetDump dirty `ADumpEntityQuery.cpp`의 MakeStringArray·SetArrayField 컴파일 오류로 Exit Code 6을 재현했다.
- AssetDump 소스와 CF-FQ-029 Launcher·Pool·추진·요격 변경 및 관련 에셋은 수정하지 않았다. `WBP_TargetSelect`는 직접 편집하지 않았으나 TargetHud Automation이 SavePackage에 성공했고 사전부터 dirty였으므로 바이트 무변경으로 단정하지 않는다.
- DR-P0-06은 Automation PASS / Build Blocked 상태로 유지하고 DR-P0-07·CF-FQ-033 전체 Done·Systems Current 승격은 수행하지 않았다.

### v0.6.0 - 2026-07-31

- DR-P0-05 테스트 DataAsset 연결을 완료했다.
- `DA_VehicleDefense_Test`에 Shield·재생·ArmorResistance·6방향 장갑 명시값을 저장했다.
- `DA_DamageArmorPenTest`를 BaseDamage 25·ArmorPenetration 50으로 생성하고 기존 `DA_DamageAsset`의 BaseDamage 25·ArmorPenetration 0과 SHA-256을 보존했다.
- 잠긴 원본 `DA_TestSUV`를 직접 저장하지 않고 별도 `DA_VehicleDefense_TestSUV`를 복제해 `DefaultDefenseData`만 연결했다.
- `DA_TestSUV`와 `DA_TestSedan`의 파일 해시와 보호 필드를 전후 비교해 CF-FQ-029 및 Legacy Fallback 보존을 확인했다.
- Apply Report `success=true / protected_contract_passed=true`와 독립 AssetDump 2/2 Success를 기록했다.
- Defense Automation은 8/8 Success였다.
- 공식 Build는 독립 AssetDump dirty 소스 컴파일 오류로 차단됐고, 전체 Combat 회귀는 필수 15/15 Success이나 잠긴 WBP_TargetSelect로 전체 1건 실패해 DR-P0-06은 완료 처리하지 않았다.
- DR-P0-07 사용자 PIE, CF-FQ-033 전체 Done과 Systems Current 승격은 수행하지 않았다.

### v0.5.0 - 2026-07-31

- DR-P0-04 Debug와 Blueprint 이벤트를 완료했다.
- `CFVehicleDefenseComp`에 정상 방어·Legacy Fallback 마지막 전체 피해 결과 캐시와 BlueprintPure 조회 API를 적용했다.
- `FCFVehicleDebugWeapon`, `ACFVehiclePawn::GetVehicleDebugSnapshot`과 VehicleDebug Panel에 방어 준비·Fallback·현재 Shield·6방향 Armor·마지막 전체 결과 표시를 연결했다.
- `CarFight.Damage.DR_P0_04.DebugBlueprintContract`에서 BlueprintAssignable 이벤트 7종, BlueprintPure Debug API, 정상·Legacy 캐시와 Reset 수명을 검증했다.
- 공식 Editor Build `0a642019bd4a4e4e87d96a0d3f4a3a2f` Result Succeeded와 Combat Runtime Process `8952458ad01242768466eaa455daf8a0` 필수 15/15·전체 24/24 Success를 기록했다.
- CF-FQ-029 Launcher·Muzzle·Scheduler·Release·Pool 격리·추진·요격 회귀를 보호하고 에셋은 수정하지 않았다.
- 다음 단계를 DR-P0-05 테스트 DataAsset 연결로 이동했다.

### v0.4.0 - 2026-07-31

- `DR-P0-03 HitScan·Projectile 정식 통합`을 Done으로 전환했다.
- `ACFVehiclePawn`의 VehicleDefenseComp 기본 생성·VehicleData 초기화·Defense 런타임 요약을 기록했다.
- HitScan과 Projectile 첫 Impact가 `UCFVehicleDefenseComp::TryApplyDamageToActor`를 정식 진입점으로 사용하도록 통합된 상태를 기록했다.
- 기존 Vehicle Health Debug와 Projectile Pool이 `BuildIntegrityCompatibilityResult` 결과를 계속 사용하는 호환 계약을 기록했다.
- 공식 Editor Build Job `7c6c3f2e6d3647d4af4e63bf537933c5` / Exit Code 0을 기록했다.
- Combat Runtime Automation Job `95bb9b328ed2443baccf0fb59e3a0cab` / 필수 14개·전체 23개 Success / 실패 0을 기록했다.
- CF-FQ-029 Launcher·Pool·추진·동일 차량 격리·다른 차량 요격 회귀 PASS와 자산 무변경을 기록했다.
- 다음 단계를 `DR-P0-04 Debug와 Blueprint 이벤트`로 이동했다.

### v0.3.0 - 2026-07-31

```text
- DR-P0-02 VehicleDefenseComp와 VehicleHealthComp 호환 확장을 구현 완료했다.
- Shield, 6방향 독립 Armor, 방향 판정, 선형 관통, Armor Overflow와 Integrity 분배를 구현했다.
- 유효 피해 후 지연 재시작, 잔여 Delta 재생과 최대값 Tick 중지를 포함한 Shield Regeneration을 구현했다.
- 기존 Health API·프로퍼티·이벤트 이름을 유지하고 명시 Integrity 피해 API를 추가했다.
- DefenseComp 또는 DefenseData가 없는 차량의 Legacy BaseDamage 직접 Health Fallback을 유지했다.
- 첫 빌드의 테스트 이름 충돌과 첫 Automation의 테스트 Fixture 비활성 문제를 보정했다.
- 최종 Editor Build Job 82ab34d50c62451fa74ca17ff00acad4 / Exit Code 0을 확인했다.
- CarFight.Damage 전체 6개 Automation을 실제 실행해 6 Success / 0 Failed / 0 Not Run을 확인했다.
- CF-FQ-029 보호 파일과 에셋은 수정하지 않았다.
- 다음 단계를 DR-P0-04 Debug와 Blueprint 이벤트로 이동했다.
```

### v0.2.0 - 2026-07-31

```text
- DR-P0-01 공용 타입·VehicleDefenseData Foundation을 구현 완료했다.
- CFDamageRuntimeTypes에 피해 전달 방식, 6방향 장갑, 장갑 분류, 방향 설정과 전체 VehicleDamageResult를 추가했다.
- VehicleDefenseData에 쉴드·재생·장갑 저항·방향별 장갑·부품 피해 예약 배율과 DataValidation을 추가했다.
- VehicleData.DefaultDefenseData를 선택 참조로 추가하고 기존 None Legacy Health Fallback을 유지했다.
- 공식 Editor Build Job bdc3e14d513549379d7f0768ad1ed7bd / Exit Code 0을 확인했다.
- CarFight.Damage.DR_P0_01.DataContract를 실제 실행해 1 Success / 0 Failed / 0 Not Run을 확인했다.
- CF-FQ-029 보호 파일과 에셋은 수정하지 않았다.
- Legacy 피해 필드 숨김은 해당 필드가 CF-FQ-029 보호 파일에 있으므로 직렬화 호환 패치로 이관했다.
- 다음 단계를 DR-P0-02 DefenseComp와 VehicleHealthComp 호환 확장으로 이동했다.
```

### v0.1.0 - 2026-07-30

```text
- CF-FQ-033 차량 방어·손상 런타임의 DR-P0-00 설계 문서를 신규 작성했다.
- 기존 VehicleHealthComp를 Vehicle Integrity와 파괴 상태 소유자로 유지하는 호환 구조를 확정했다.
- VehicleDefenseData, VehicleDefenseComp와 VehicleDamageResult 데이터 계약을 확정했다.
- Shield → Directional Armor → Integrity 피해 분배 순서와 P0 관통 공식을 확정했다.
- 6방향 독립 Armor Pool과 Bounds 중심 기반 방향 판정 규칙을 확정했다.
- 쉴드 재생 지연·초당 재생과 이벤트 계약을 확정했다.
- DefaultDefenseData None 차량의 Legacy Direct Health Fallback을 확정했다.
- DR-P0-01~07 구현·검증 단계와 18개 Automation 테스트 계획을 정의했다.
- CF-FQ-029 Launcher·Missile dirty 변경 보호 범위와 DR-P0-03 통합 게이트를 고정했다.
- 소스, Blueprint, DataAsset과 맵은 수정하지 않았다.
```

---

## 22. Migration

### v0.23.0 적용 안내

- `CF-FQ-033`의 현재 구현 판단은 `Document/Systems/Combat/VehicleDefense.md`와 `HitDamage.md`를 우선한다.
- 이 문서는 Completed Plan으로 유지하며 현재 계산식·소유권을 Systems보다 우선하지 않는다.
- DR-PIE-00~06은 완료됐으므로 관련 코드·데이터·피팅·Launcher·Projectile 변경이 영향을 줄 때만 회귀로 재실행한다.
- 기존 `DefaultDefenseData=None` 차량은 정식 Legacy Fallback으로 유지한다.
- 다음 Active는 `Document/Plan/InGameUIPlan.md v0.7.0`이며 UI-P0-02 사용자 PIE부터 재개한다.
- UI-P0-02 완료 뒤 UI-P0-03 Vehicle·Weapon·Defense·Target·Radar·Alert HUD 데이터 계약으로 진행한다.
- 완료 당시 격리 테스트 에셋과 맵은 검증 증거로 유지하되 제품 밸런스 자산으로 자동 승격하지 않는다.

### v0.22.0 적용 안내

- 기존 에디터가 이전 빌드로 실행 중이면 종료하고 `Tools/RunEditor.bat`로 다시 실행한다.
- Salvo 검증은 `/Game/Maps/TestMap_DRSalvo`, Ripple 검증은 `/Game/Maps/TestMap_DRRipple`에서 각각 새 PIE로 수행한다.
- 두 맵에서 Player0는 위치 `(1725, 48, 0)`의 SUV·RocketLauncher이며 다른 Sedan은 자동 빙의가 해제된 상태다.
- 출력 로그에서 `VehicleWeaponFireOrigin`을 필터링해 Muzzle_1→Muzzle_4→Muzzle_2→Muzzle_3와 MuzzleCount=4를 확인한다.
- PIE World Outliner에서 Player SUV의 `LauncherComp`를 선택해 `런처 시퀀스 요약 (LastLauncherSequenceSummary)`이 Completed, Accepted 4, Failed 0, Remaining 0, Pending 0, CooldownApplied True인지 확인한다.
- Salvo는 한 입력에서 4발이 같은 묶음으로 사출돼야 하고 Ripple은 약 0.15초 간격으로 4발이 순차 사출돼야 한다.
- Ripple 진행 중 추가 발사 입력이 두 번째 시퀀스를 만들지 않아야 한다.
- 각 맵에서 최소 5 Volley를 반복해 매 Volley 4발 완결, Projectile 누락·잔류·Pool 누수 없음과 같은 차량 Projectile 상호 비충돌을 확인한다.
- Rocket의 점화 후 가속, Trail, 연소 중 Thruster, 차량·월드 충돌 시 Impact FX를 확인한다.
- 원본 `TestMap`, `DA_RocketLauncher`, `RocketLauncher`, `DA_RocketBody`, `DA_Rocket_PropTest`, `DA_TestSUV`, `BP_CFVehiclePawn`, `HeavyCannon`은 수정하거나 재저장하지 않는다.
- 결과가 모두 일치하면 DR-PIE-06 USER PASS와 DR-P0-07 전체 PASS를 기록하고 VehicleDefense·HitDamage Systems 동기화와 CF-FQ-033 Done 전환을 진행한다.

### v0.21.0 적용 안내

- DR-PIE-05는 사용자 확인으로 PASS 처리하며 `M_VehicleDefensePIE_Legacy` 검증을 반복하지 않는다.
- 다음은 DR-PIE-06 CF-FQ-029 Launcher·Projectile 회귀다.
- 원본 `TestMap` 또는 기존 CF-FQ-029 사용자 검증 경로에서 RocketLauncher의 Ripple과 Salvo를 실행한다.
- 발사 순서·Muzzle 순서·SequenceCompleted, Projectile 사출·이동·일반 차량 및 월드 충돌, Rocket 추진·Trail·Thruster와 Impact FX가 기존 결과를 유지해야 한다.
- Projectile Pool은 최소 5 Volley 이상 반복해 Projectile 누락·잔류·Pool 누수가 없는지 확인한다.
- 같은 차량에서 발사된 Projectile끼리 사출 직후 상호 Impact하지 않아야 한다.
- 피해 통합 뒤 Launcher 시퀀스 취소, Muzzle 순서 변경과 비정상적인 조기 Impact가 발생하면 DR-PIE-06 FAIL로 기록한다.
- DR-PIE-06까지 USER PASS이면 DR-P0-07 전체 PASS 판정 후 VehicleDefense·HitDamage Systems 동기화와 CF-FQ-033 Done 전환을 진행한다.

### v0.20.0 적용 안내

- `Tools/RunEditor.bat`로 현재 빌드의 에디터를 실행하고 `/Game/Maps/M_VehicleDefensePIE_Legacy`를 연다.
- 원본 `M_VehicleDefensePIE`, `DA_TestSUV`, `DA_VehicleDefense_TestSUV`, `DA_Fit_DefenseTestSUV`와 AP0 무기 체인은 수정하거나 재저장하지 않는다.
- 새 PIE에서 배치된 SUV를 선택하고 VehicleDebug `선택 대상 → 대상 방어`의 초기 Shield 0/0, 6방향 Armor 0, Integrity 100을 확인한다.
- 기존 Heavy Cannon Projectile 한 발을 명중시킨다.
- 마지막 전체 피해 결과가 Legacy=예, 요청 25, Shield 흡수 0, Armor 흡수 0, Integrity 요청/적용 25/25인지 확인한다.
- Integrity는 100→75가 되어야 하며 Shield와 6방향 Armor는 계속 0이어야 한다.
- 명시적 HitScan 장비는 새로 만들지 않고 기존 RuntimeIntegration·Combat Automation PASS를 HitScan·Projectile 동등성 증거로 사용한다.
- 결과가 일치하면 DR-PIE-05를 USER PASS로 기록하고 DR-PIE-06 CF-FQ-029 회귀로 진행한다.

### v0.19.0 적용 안내

- DR-PIE-03은 사용자 확인으로 PASS 처리하며 AP0·AP50 비교를 반복하지 않는다.
- 다음은 DR-PIE-05 Legacy 호환이다.
- `DefaultDefenseData=None`인 `DA_TestSedan` 또는 `DA_TestSUV`를 대상으로 새 PIE를 시작한다.
- 기존 Projectile 한 발을 명중시키고 VehicleDebug에서 피해 경로가 `Legacy Integrity Fallback`인지 확인한다.
- Shield와 6방향 Armor는 모두 0을 유지해야 한다.
- BaseDamage 25가 Armor 분배 없이 Integrity에 직접 적용돼 Integrity가 100→75가 되어야 한다.
- 기존 Health 변경·Damage 이벤트는 Integrity가 실제 감소할 때 발생하고, Integrity 0의 최초 Destroyed 이벤트 의미도 유지되어야 한다.
- 명시적 HitScan 테스트 장비가 현재 에셋에 없으면 새 에셋을 만들지 않고 Projectile Legacy Fallback 사용자 PIE와 기존 HitScan·Projectile Automation PASS 증거로 판정한다.
- DR-PIE-05 PASS 뒤 DR-PIE-06 CF-FQ-029 회귀로 진행한다.

### v0.18.0 적용 안내

- `Tools/RunEditor.bat`로 에디터를 실행한다.
- AP 0 검증은 `/Game/Maps/M_VehicleDefensePIE`에서 새 PIE를 시작한다.
- 대상 SUV를 선택하고 정면에 빠르게 네 발을 명중시켜 Shield를 0으로 만든 뒤, 재생 전에 다섯 번째 한 발을 명중시킨다.
- VehicleDebug `선택 대상 → 대상 방어`에서 Front Armor 75, Integrity 100, Armor 흡수 25, Integrity 적용 0을 확인하고 PIE를 종료한다.
- AP 50 검증은 `/Game/Maps/M_VehicleDefensePIE_AP50`에서 새 PIE를 시작한다.
- VehicleDebug Weapon에서 `DA_HeavyShell_DRAP50`과 `DA_DamageArmorPenTest`가 활성 경로인지 먼저 확인한다.
- 같은 방식으로 정면 다섯 발을 명중시킨 뒤 Front Armor 87.5, Integrity 87.5, Armor 흡수 12.5, Integrity 적용 12.5를 확인한다.
- AP50 맵에서 플레이어가 Sedan·Heavy Cannon이 아니거나 활성 DamageData가 `DA_DamageArmorPenTest`가 아니면 사격하지 않고 BLOCKED로 보고한다.
- 두 결과가 일치하면 DR-PIE-03을 PASS 처리하고 DR-PIE-05 → DR-PIE-06 순서로 진행한다.
- 원본 `M_VehicleDefensePIE`, `BP_CFVehiclePawn`, `DA_TestSedan`, HeavyCannon 계열 에셋은 수정하거나 재저장하지 않는다.

### v0.17.0 적용 안내

- DR-PIE-02는 사용자 확인으로 PASS 처리하며 반복하지 않는다.
- 다음은 DR-PIE-03 AP 0·AP 50 관통 비교다.
- AP 0과 AP 50은 반드시 각각 새 PIE 또는 방어 완전 초기화 상태에서 비교한다.
- 공통 초기 상태는 Shield 0, Front Armor 100, Integrity 100이다.
- AP 0 / BaseDamage 25 한 발은 Front Armor 75, Integrity 100이 예상값이다.
- AP 50 / BaseDamage 25 한 발은 Front Armor 87.5, Integrity 87.5가 예상값이다.
- AP 50에서 Armor 감소량은 더 작고 Integrity 피해는 더 커야 하며, 두 공격의 BaseDamage는 동일한 25여야 한다.
- DR-PIE-03 PASS 뒤 DR-PIE-05 → DR-PIE-06 순서로 진행한다.

### v0.16.0 적용 안내

- DR-PIE-04는 수정 후 사용자 확인으로 PASS 처리하며 반복하지 않는다.
- 다음은 DR-PIE-02 방향별 장갑이다.
- Front·Left·Right·Rear는 각각 독립 초기 상태에서 비교한다. 가장 안전한 방법은 방향마다 새 PIE를 시작하는 것이다.
- 각 PIE에서 먼저 Shield를 0으로 만든 뒤 AP 0 / BaseDamage 25 한 발을 지정 방향에 명중시킨다.
- 예상 Armor는 Front 75.0, Left 70.0, Right 70.0, Rear 62.5이며 Integrity는 100을 유지해야 한다.
- 피격 방향 Armor만 감소하고 다른 방향 Armor는 100을 유지해야 한다.
- DR-PIE-02 PASS 뒤 DR-PIE-03 → DR-PIE-05 → DR-PIE-06 순서로 진행한다.

### v0.15.0 적용 안내

- `Tools/RunEditor.bat`로 새 에디터를 실행하고 `M_VehicleDefensePIE`에서 새 PIE를 시작한다.
- 테스트 SUV를 선택 대상으로 확정하고 두 발 명중해 Shield 50을 만든다.
- VehicleDebug의 `선택 대상 → 대상 방어`에서 남은 재생 지연이 약 5초부터 0까지 감소하는지 확인한다.
- 지연 종료 뒤 Shield가 초당 약 10 증가해 100에서 정지하고 재생 상태가 종료되는지 확인한다.
- 지연 중 추가 피격 시 남은 재생 지연이 다시 약 5초로 초기화되는지도 확인한다.
- 수정 후 DR-PIE-04 PASS가 확인되기 전에는 DR-PIE-02로 넘어가지 않는다.
- DR-PIE-04 PASS 뒤 DR-PIE-02 → DR-PIE-03 → DR-PIE-05 → DR-PIE-06 순서로 진행한다.

### v0.14.0 적용 안내

- 기존 에디터 또는 Commandlet을 종료해 `UnrealEditor-NetCore.dll` 파일 점유를 해소한 뒤 공식 Editor Build와 `Tools/RunCombatRuntimeTests.ps1`을 재검증한다.
- 에디터를 새 빌드로 다시 실행하고 `M_VehicleDefensePIE`에서 테스트 SUV를 현재 선택 대상으로 확정한다.
- VehicleDebug Panel의 `선택 대상` Navigation을 열어 대상 방어에서 Shield·6방향 Armor·재생 상태·남은 재생 지연을, 대상 내구도에서 현재·최대 Integrity와 파괴 여부를 확인한다.
- DR-PIE-04는 새 PIE에서 한 발 명중 후 Shield 감소, 약 5초 재생 지연 감소, 초당 10 재생, 최대 100 정지를 선택 대상 섹션으로 판정한다.
- Target 섹션은 C++ 동적 ViewData이므로 WBP 에셋이나 Blueprint 그래프를 수정하지 않는다.
- DR-PIE-04 PASS 뒤 DR-PIE-02 → DR-PIE-03 → DR-PIE-05 → DR-PIE-06 순서로 진행한다.
- 전체 DR-PIE 행렬 PASS 전에는 CF-FQ-033을 Done 처리하거나 Systems Current로 승격하지 않는다.

### v0.13.0 적용 안내

- 새 세션은 DR-PIE-00과 DR-PIE-01 방어층 수치를 반복하지 않는다.
- 먼저 `DA_VehicleDefense_TestSUV`가 `DA_FX_ProtoVehicleDead`를 유지하는 상태에서 테스트 SUV를 파괴하고, 폭발 후 추가 1발에 두 번째 폭발이 없는지만 확인한다.
- 해당 결과가 PASS이면 새 PIE를 시작해 DR-PIE-04 쉴드 재생을 진행한다.
- 이후 DR-PIE-02 방향별 장갑, DR-PIE-03 AP 0/AP 50 비교, DR-PIE-05 Legacy Fallback, DR-PIE-06 CF-FQ-029 회귀 순서로 진행한다.
- 에디터 Details의 VehicleDefense Runtime `None / 0` 표시는 실제 런타임 판정 근거로 사용하지 않는다.
- DR-PIE-01~06 전체 PASS 전에는 CF-FQ-033을 Done 처리하거나 Systems Current로 승격하지 않는다.

### v0.12.0 적용 안내

- 사용자에게 World Outliner와 Details에서 런타임 수치를 찾게 하지 않는다.
- 사용자는 `Tools/RunEditor.bat → M_VehicleDefensePIE → PIE 시작 → 2초 유지`까지만 수행하고 실행 사실을 보고한다.
- AI는 해당 실행의 `UE/Saved/Logs/CarFight_Re.log`에서 `CF-FQ-033 ManualPIEProbe`를 직접 읽어 DR-PIE-00을 판정한다.
- BeginPlay.AfterInitialize, NextTick과 AfterOneSecond를 사용하고 PreRegister.BeforePrepare와 EndPlay.AfterReset은 정상 수명 경계로 제외한다.
- 사용자 수동 DR-PIE-00과 DR-PIE-01~06 전체 PASS 전에는 CF-FQ-033을 Done 처리하지 않는다.

### v0.11.0 적용 안내

- 사용자 PIE에서 Details를 읽기 전에 월드 아웃라이너를 PIE World로 전환하고 선택 Actor 경로가 `/Game/Maps/UEDPIE_0_M_VehicleDefensePIE...`인지 확인한다.
- 에디터 원본 `/Game/Maps/M_VehicleDefensePIE...` Actor의 `Uninitialized / None / 0` 표시는 런타임 판정에 사용하지 않는다.
- DR-PIE-00의 실제 저장 맵 자동 기준선은 PASS 상태이므로 런타임 초기화 코드를 다시 수정하지 않고 사용자 선택 Gate부터 재개한다.
- 다음 사용자 단계는 DR-PIE-00 수동 확인 뒤 DR-PIE-01 Shield → Armor → Integrity이며, DR-PIE-01~06 전체 PASS 전에는 CF-FQ-033을 Done 처리하지 않는다.

### v0.10.0 적용 안내

- 이 문서는 현재 CarFight 단일 Active Plan이며 작업 완료 우선순위 1위다.
- DR-P0-00~06 구현·테스트 자산·Build·Automation은 완료 상태이므로 반복하지 않는다.
- `DR-PIE-00 시작 상태 Gate`부터 진행하고 DR-PIE-01~06을 각각 PASS / FAIL / BLOCKED로 기록한다.
- FAIL이면 해당 데미지 결함을 먼저 수정하고 공식 Editor Build, 전체 CarFight Automation과 영향받은 PIE를 다시 수행한다.
- 전체 PASS 뒤 `Document/Systems/Combat/VehicleDefense.md`와 `HitDamage.md`를 동기화하고 CF-FQ-033을 Done으로 전환한다.
- DR-P0-07을 직접 차단하는 문제만 예외적으로 먼저 처리하며, 차단 해소 뒤 즉시 이 Plan으로 복귀한다.

### v0.9.0 적용 안내

- 새 세션은 DR-P0-00~06을 반복하지 않고 DR-PIE-00 시작 상태 게이트부터 진행한다.
- 첫 사용자 실행은 AP 0 Sedan·HeavyCannon → DA_VehicleDefense_TestSUV 대상 경로를 우선한다.
- 원본 TestMap과 CF-FQ-029 전투 에셋을 수정·저장하지 않으며 필요 시 `M_VehicleDefensePIE` 복제 맵을 사용한다.
- `DA_DamageArmorPenTest`는 자동 연결되지 않으므로 AP 50 검증 전 PIE 전용 복제 체인을 준비한다.
- 사용자 결과는 DR-PIE-01~06별 PASS / FAIL / BLOCKED로 기록하며 전체 행렬 전에는 DR-P0-07을 완료 처리하지 않는다.
- CF-FQ-033 전체 Done과 `Document/Systems/Combat/VehicleDefense.md` Current 승격은 DR-P0-07 전체 PASS 후 별도 단계에서 처리한다.

### v0.8.0 적용 안내

- DR-P0-00~06 구현·테스트 자산·Automation·공식 Editor Build는 완료 상태이므로 반복하지 않는다.
- 최신 공식 Build 기준은 Job `0697e0552a0f4911a93384c5d3a8a928`, `CarFight_ReEditor Win64 Development`, Exit Code 0이다.
- 다음 단계는 `DR-P0-07 사용자 PIE`이며 문서 또는 Automation 결과만으로 사용자 PIE를 PASS 처리하지 않는다.
- DR-P0-07 완료 전에는 CF-FQ-033 전체 Done과 `Document/Systems/Combat/VehicleDefense.md` Current 승격을 수행하지 않는다.
- 현재 단일 Active `CF-FQ-029 / LM-P0-06`과 Launcher·Pool·추진·요격 dirty 변경 및 관련 에셋을 계속 보호한다.

### v0.7.0 적용 안내

- DR-P0-00~05 구현과 DR-P0-06 Defense·Combat Automation은 PASS 상태이므로 반복하지 않는다.
- `WBP_TargetSelect.uasset` 파일 잠금 차단은 해소됐으며 전체 CarFight Automation 기준은 24/24 Success다.
- 남은 DR-P0-06 차단은 독립 AssetDump `ADumpEntityQuery.cpp` 컴파일 오류에 의한 공식 Editor Build 실패 한 건이다.
- CarFight 작업에서 AssetDump 소스를 임의 수정하지 않고 독립 저장소에서 차단이 해소된 뒤 공식 Build만 재검증한다.
- 공식 Build PASS 전에는 DR-P0-06을 Done으로 전환하지 않는다.
- DR-P0-07 사용자 PIE, CF-FQ-033 전체 Done과 Systems Current 승격은 아직 수행하지 않는다.

### v0.6.0 적용 안내

- DR-P0-00~05의 구현과 테스트 자산 생성은 반복하지 않는다.
- `DA_VehicleDefense_TestSUV`가 정식 방어 테스트 VehicleData이고, 원본 `DA_TestSUV`와 `DA_TestSedan`은 `DefaultDefenseData=None` 보호 기준이다.
- 무관통 비교 기준은 기존 `DA_DamageAsset`의 BaseDamage 25·ArmorPenetration 0, 부분 관통 비교 기준은 `DA_DamageArmorPenTest`의 BaseDamage 25·ArmorPenetration 50이다.
- 다음 단계는 DR-P0-06 재검증이며, AssetDump 컴파일 오류와 `WBP_TargetSelect.uasset` 파일 잠금을 먼저 해소해야 한다.
- DR-P0-06 공식 Build와 전체 CarFight 24/24 Success 전에는 완료 처리하지 않는다.
- DR-P0-07 사용자 PIE, CF-FQ-033 전체 Done과 Systems Current 승격은 아직 수행하지 않는다.

### v0.5.0 적용 안내

- DR-P0-00~04의 C++ 구현과 Automation은 반복하지 않는다.
- 다음 단계는 DR-P0-05 테스트 `VehicleDefenseData` 생성·`VehicleData.DefaultDefenseData` 연결이다.
- 사용자 PIE는 DR-P0-07까지 Pending이며 현재 문서만으로 PASS 처리하지 않는다.
- CF-FQ-033 전체 Done과 Systems Current 승격은 DR-P0-05~07 및 최종 회귀 완료 전 수행하지 않는다.
- 현재 단일 Active `CF-FQ-029 / LM-P0-06`과 기존 dirty 에셋을 계속 보호한다.

### v0.4.0 적용 안내

- `DR-P0-03` 소스 통합과 공식 Build·Automation은 완료 상태이므로 반복 구현하지 않는다.
- HitScan과 Projectile의 정식 피해 진입점은 `UCFVehicleDefenseComp::TryApplyDamageToActor`다.
- 기존 `FCFDamageApplyResult` 소비자는 `BuildIntegrityCompatibilityResult`를 통해 계속 호환한다.
- `DefaultDefenseData=None`은 실패가 아니라 기존 VehicleHealth 직접 피해를 사용하는 Legacy Fallback이다.
- `CF-FQ-033` 전체 Done, `DR-P0-05`, `DR-P0-07`, Systems Current 승격은 아직 수행하지 않는다.
- 다음 작업은 `DR-P0-04 Debug와 Blueprint 이벤트`다.

### v0.3.0 적용 안내

```text
- DR-P0-02의 VehicleDefenseComp, Health Integrity 호환 API와 6개 Automation은 완료됐으므로 반복 구현하지 않는다.
- 기존 ApplyDamageFromHitContext, TryApplyDamageToActor, MaxHealth, CurrentHealth와 Health 이벤트 이름을 계속 유지한다.
- 신규 VehicleDefenseComp는 아직 ACFVehiclePawn 기본 컴포넌트가 아니며 실제 Projectile·HitScan 경로에 연결되지 않았다.
- 기존 플레이 결과는 DR-P0-03부터 VehicleDefenseComp 정식 진입점을 사용하며 DefenseData가 없으면 VehicleHealthComp Legacy Fallback을 유지한다.
- DR-P0-03은 CF-FQ-029 dirty 파일과 현재 체크포인트를 다시 읽은 뒤 최소 호출부만 수정해 완료했다.
- Launcher Scheduler, Launch Context, Projectile Pool, 추진, 유도, 요격, 충돌 Profile과 FX 생명주기를 변경하지 않는다.
- 신규 DefenseData 에셋 생성과 VehicleData 연결은 DR-P0-05 사용자 Editor 단계까지 수행하지 않는다.
- CF-FQ-033을 Done 또는 Current System으로 승격하지 않는다.
```

### v0.2.0 적용 안내

```text
- 기존 VehicleData는 DefaultDefenseData=None이므로 현재 BaseDamage 직접 Health 동작을 유지한다.
- 신규 VehicleDefenseData 에셋은 아직 생성하거나 기존 차량에 연결하지 않는다.
- DR-P0-02는 신규 VehicleDefenseComp와 기존 VehicleHealthComp 호환 확장만 수행하고 CF-FQ-029 보호 파일을 계속 수정하지 않는다.
- DR-P0-01에서 추가한 FCFVehicleDamageResult는 아직 HitScan·Projectile 런타임에서 소비하지 않는다.
- WeaponData·ProjectileData Legacy 피해 필드는 이번 단계에서 삭제하거나 이름을 바꾸지 않았다.
- Legacy 숨김은 CF-FQ-029 체크포인트 후 기존 .uasset 직렬화 보존, 에디터 비노출과 디버그 요약 제거를 함께 검증하는 별도 패치로 수행한다.
```

### v0.1.0 적용 안내

```text
- 기존 차량은 DefaultDefenseData가 없으므로 현재 BaseDamage 직접 Health 동작을 유지한다.
- 기존 VehicleData, ProjectileData와 DamageData를 이번 문서 단계에서 재저장하지 않는다.
- WeaponData의 LegacyBaseDamage와 LegacyDamageProfileId를 실제 피해 소유권으로 복원하지 않는다.
- VehicleDurabilityConfig.MaxHealth를 유지하고 의미를 Vehicle Integrity 최대값으로 확장한다.
- UCFVehicleHealthComp와 FCFDamageApplyResult를 삭제하거나 이름을 변경하지 않는다.
- 신규 Defense Runtime은 DR-P0-01~02에서 CF-FQ-029 중첩 파일 없이 구현한다.
- HitScan·Projectile 정식 호출 교체는 DR-P0-03에서 CF-FQ-029 보호 게이트, 공식 Build와 회귀 Automation을 통과해 완료했다.
- P0 구현과 검증 완료 전에는 Systems Current 또는 CF-FQ-033 Done으로 승격하지 않는다.
```