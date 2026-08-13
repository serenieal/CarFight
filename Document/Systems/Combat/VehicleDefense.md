# VehicleDefense

- Version: 1.0.0
- Date: 2026-08-06
- Status: Current System
- Feature: `CF-FQ-033 차량 방어·손상 런타임`
- Verification: `CarFight_ReEditor Win64 Development` Build PASS / Combat Automation 46/46 PASS / DR-PIE-00~06 USER PASS

---

## 1. 문서 목적

이 문서는 CarFight의 현재 차량 방어·손상 런타임이 실제로 어떤 데이터를 소유하고, HitScan 또는 Projectile의 피해를 Shield, 6방향 Armor와 Vehicle Integrity에 어떤 순서와 공식으로 분배하는지 기록한다.

현재 구현 판단은 완료 당시 Plan보다 이 문서를 우선한다.

```text
현재 구현 기준
= 실제 C++와 DataAsset
→ Document/Systems/Combat/VehicleDefense.md
→ Document/Systems/Combat/HitDamage.md
→ 완료 Plan과 과거 체크포인트
```

이 문서는 다음 범위를 현재 구현으로 다룬다.

```text
- VehicleDefenseData 정적 설정
- VehicleDefenseComp 런타임 상태
- Shield 피해 흡수와 자동 재생
- Front / Left / Right / Rear / Top / Bottom 독립 Armor
- 방향별 피해 배율
- ArmorPenetration / ArmorResistance 관통 공식
- Armor 고갈 초과 피해와 Vehicle Integrity 전달
- 기존 VehicleHealthComp·이벤트 호환
- DefaultDefenseData=None Legacy Fallback
- 방어층별 결과·이벤트·VehicleDebug 표시
- Fitting Snapshot과 Initial Mass 이후 Defense Commit
```

부품별 실제 손상, 도탄, 범위 폭발과 네트워크 권한 피해는 현재 범위가 아니다.

---

## 2. 현재 전체 흐름

현재 피해 흐름은 다음과 같다.

```text
WeaponData
→ ProjectileData.DefaultDamageData
→ DamageData
→ FCFDamageHitContext
→ UCFVehicleDefenseComp::TryApplyDamageToActor
   → Shield
   → 피격 방향 Armor
   → UCFVehicleHealthComp = Vehicle Integrity
→ FCFVehicleDamageResult
→ 기존 FCFDamageApplyResult 호환 결과
→ C++ 상태 이벤트 / Blueprint UI·VFX 소비
```

HitScan과 Projectile은 같은 `FCFDamageHitContext`와 같은 방어 진입점을 사용한다.
Projectile Pool 반환 시 피해를 다시 계산하지 않으며 Projectile Actor에 저장된 결과만 Pawn Debug로 복사한다.

---

## 3. 소유권

| 요소 | 현재 소유 책임 |
| --- | --- |
| `UCFDamageData` | BaseDamage, DamageType, 자기 피해 허용, ArmorPenetration과 후속 확장 데이터를 제공한다. |
| `UCFProjectileData.DefaultDamageData` | 실제 DamageData의 유일한 직접 참조 슬롯이다. |
| `FCFDamageHitContext` | Blocking Hit, 대상 Actor·Component, 충돌 위치·노멀·입사 방향과 발사 주체를 전달한다. |
| `UCFVehicleData.DefaultDefenseData` | 차량이 사용할 선택적 `UCFVehicleDefenseData`를 지정한다. `None`이면 Legacy Fallback이다. |
| `UCFVehicleDefenseData` | Shield, 재생, ArmorResistance, 6방향 Armor 설정과 방어 패키지 질량을 제공한다. |
| `UCFVehicleDefenseComp` | 현재 Shield, 6방향 Armor, 관통·피해 분배, 재생 상태와 전체 방어 결과를 소유한다. |
| `UCFVehicleHealthComp` | Vehicle Integrity, 최초 파괴 상태와 기존 Health·Damaged·Destroyed 이벤트를 소유한다. |
| `FCFVehicleDamageResult` | 한 타격의 Shield·Armor·Integrity 전체 계산 결과와 기존 Integrity 결과를 보존한다. |

`UCFVehicleDefenseComp`는 `UCFVehicleHealthComp`를 대체하지 않는다.
기존 `MaxHealth`와 `CurrentHealth` 프로퍼티 이름은 직렬화와 Blueprint 호환을 위해 유지하지만 현재 의미는 각각 최대 Vehicle Integrity와 현재 Vehicle Integrity다.

---

## 4. 정적 방어 데이터

`UCFVehicleDefenseData`는 차량별 정적 방어 설정을 제공하는 `PrimaryDataAsset`이다.

### 4.1 식별과 피팅 질량

```text
DefenseId
DefenseMassKg
```

`DefenseMassKg`는 Fitting Snapshot의 총중량에 포함되지만 피해 공식에는 직접 사용하지 않는다.
음수 또는 비유한 값은 유효 질량 Getter에서 0으로 보정한다.

### 4.2 Shield 설정

```text
bUseShield
MaximumShield
ShieldRegenerationDelaySeconds
ShieldRegenerationPerSecond
```

기본 C++ 값:

```text
bUseShield = true
MaximumShield = 100
ShieldRegenerationDelaySeconds = 5초
ShieldRegenerationPerSecond = 10/초
```

`bUseShield=false`이면 유효 최대 Shield는 0이다.
모든 유효 Getter는 음수 값을 0으로 보정한다.

### 4.3 Armor 설정

```text
ArmorType
ArmorResistance
FrontArmorConfig
LeftArmorConfig
RightArmorConfig
RearArmorConfig
TopArmorConfig
BottomArmorConfig
```

각 `FCFDirectionalArmorConfig`는 다음 두 값을 가진다.

```text
MaximumArmor
DamageMultiplier
```

Armor는 여섯 방향이 서로 독립된 현재값을 가진다.
정면 Armor가 감소해도 Left, Right, Rear, Top과 Bottom Armor는 변경되지 않는다.

### 4.4 후속 부품 피해 예약 데이터

```text
ShieldComponentDamageScale
ArmorComponentDamageScale
IntegrityComponentDamageScale
```

이 값들은 DR-P1 부품 손상 확장용 예약 데이터다.
현재 P0 런타임에서는 `DamageAppliedToComponents=0`, `AffectedComponentIds=[]`를 유지한다.

---

## 5. 초기화와 Fitting 연결

### 5.1 차량 기본 컴포넌트

`ACFVehiclePawn`은 `VehicleDefenseComp`와 `VehicleHealthComp`를 기본 서브오브젝트로 생성한다.
Blueprint Pawn에 같은 컴포넌트를 수동으로 중복 추가하지 않는다.

### 5.2 기본 VehicleData 초기화

```text
UCFVehicleDefenseComp::InitializeFromVehicleData
→ VehicleData.DefaultDefenseData 조회
→ 같은 Actor의 VehicleHealthComp 준비
→ InitializeFromDefenseData
```

유효한 `VehicleDefenseData`가 연결되면 다음 상태로 초기화된다.

```text
ActiveDefenseData = 연결된 VehicleDefenseData
CurrentShield = MaximumShield
6방향 CurrentArmor = 각 방향 MaximumArmor
bDefenseInitialized = true
컴포넌트 Active = true
Shield 재생 Tick = 정지
```

초기화 직후에는 Shield가 가득 차 있으므로 Tick을 계속 실행하지 않는다.
실제 피해가 하나 이상의 방어층에 적용된 뒤에만 재생 지연 Tick을 켠다.

### 5.3 같은 DefenseData 재초기화

같은 `ActiveDefenseData`로 다시 초기화할 때는 현재 손상 상태를 새 최대값 범위 안에서 보존한다.
다른 DefenseData로 전환하거나 최초 초기화할 때는 Shield와 6방향 Armor를 새 최대값으로 채운다.

### 5.4 DefenseData 없음

`DefaultDefenseData=None`이면 다음 상태다.

```text
ActiveDefenseData = None
Shield = 0
6방향 Armor = 0
bDefenseInitialized = false
Legacy Health Fallback 사용
```

기존 차량 에셋은 재저장하지 않아도 기존 BaseDamage 직접 Integrity 피해 동작을 유지한다.

### 5.5 Fitting Snapshot Commit

Fitting이 연결된 차량은 다음 순서로 같은 Snapshot을 적용한다.

```text
Fitting Snapshot 준비
→ Configured Initial Mass와 실제 Physics Mass Coverage 검증
→ Weapon Commit
→ Defense Commit
→ VehicleDefenseComp.ActiveDefenseData 연결
```

현재 질량 검증은 설정 질량의 정확 비교를 유지하면서 PhysicsAsset 집계 질량이 Target 이상을 덮는지 Coverage로 판정한다.
실제 방어 테스트 SUV에서는 다음 결과를 확인했다.

```text
Configured Mass = 1570kg
Actual Physics Mass = 1837.377kg
Initial Mass = Verified
Snapshot = CommittedSnapshot
WeaponReady = Yes
DefenseReady = Yes
ActiveDefenseData = DA_VehicleDefense_Test
```

---

## 6. 피해 입력 검증

정식 진입점:

```text
UCFVehicleDefenseComp::TryApplyDamageToActor(
    const FCFDamageHitContext& DamageHitContext,
    FCFVehicleDamageResult& OutVehicleDamageResult)
```

다음 조건을 검증한다.

```text
- Blocking Hit이어야 한다.
- HitActor가 유효해야 한다.
- DamageData가 연결돼야 한다.
- BaseDamage가 0보다 커야 한다.
- Instigator와 HitActor가 같고 bCanDamageSelf=false이면 거부한다.
- DefenseComp 소유 Actor와 HitActor가 같아야 한다.
- VehicleHealthComp가 존재해야 한다.
- 대상 Vehicle Integrity가 남아 있고 파괴 상태가 아니어야 한다.
```

거부 시 `FCFVehicleDamageResult.RejectReason`에 기존 공용 `ECFDamageApplyRejectReason`을 기록한다.
이미 파괴된 차량의 후속 피해는 `TargetDestroyed`로 거부한다.

---

## 7. 6방향 Armor 판정

기본 좌표 계약:

```text
차량 로컬 +X = Front
차량 로컬 -X = Rear
차량 로컬 +Y = Right
차량 로컬 -Y = Left
차량 로컬 +Z = Top
차량 로컬 -Z = Bottom
```

방향 판정은 피격 Component 또는 Actor Bounds 중심에서 ImpactLocation으로 향하는 오프셋을 차량 로컬 공간으로 변환하고 절대값이 가장 큰 지배 축을 선택한다.

```text
LocalOffset = VehicleTransform.InverseTransformPosition(ImpactLocation - DefenseCenter)
DominantAxis = max(abs(X), abs(Y), abs(Z))
```

오프셋이 사실상 0이면 안전 기본값으로 Front를 사용한다.

---

## 8. 피해 분배 공식

### 8.1 원본 피해

```text
RequestedDamage = max(DamageData.BaseDamage, 0)
```

### 8.2 Shield

Shield는 방향 배율과 관통 계산 전에 원본 피해를 1:1로 흡수한다.

```text
DamageAbsorbedByShield = min(CurrentShield, RequestedDamage)
CurrentShield = max(CurrentShield - DamageAbsorbedByShield, 0)
DamageAfterShield = max(RequestedDamage - DamageAbsorbedByShield, 0)
```

Shield가 피해 전 0보다 크고 이번 타격으로 0이 되면 `bShieldBrokenThisHit=true`다.

### 8.3 방향 피해 배율

Shield를 통과한 피해에 피격 방향 설정의 배율을 적용한다.

```text
DirectionalDamage = DamageAfterShield × DirectionDamageMultiplier
```

### 8.4 관통 비율

```text
EffectiveArmorResistance = max(ArmorResistance, 0)

if EffectiveArmorResistance <= 0:
    ArmorPenetrationRatio = 1
else:
    ArmorPenetrationRatio = clamp(
        max(ArmorPenetration, 0) / EffectiveArmorResistance,
        0,
        1)
```

관통 비율은 피해 타입이 아니다.
`DamageType`은 Kinetic, Explosive, Energy 속성을 유지하고 `ArmorPenetration`은 Armor Pool을 우회하는 연속 수치다.

### 8.5 Armor와 직접 관통

```text
DirectPenetrationDamage = DirectionalDamage × ArmorPenetrationRatio
ArmorBlockCandidate = max(DirectionalDamage - DirectPenetrationDamage, 0)
DamageAbsorbedByArmor = min(CurrentDirectionalArmor, ArmorBlockCandidate)
CurrentDirectionalArmor = max(CurrentDirectionalArmor - DamageAbsorbedByArmor, 0)
ArmorOverflowDamage = max(ArmorBlockCandidate - DamageAbsorbedByArmor, 0)
```

Armor가 피해 전 0보다 크고 이번 타격으로 0이 되면 `bArmorBrokenThisHit=true`다.

### 8.6 Vehicle Integrity

```text
DamageRequestedForIntegrity
= DirectPenetrationDamage + ArmorOverflowDamage
```

이 값이 0보다 크면 `VehicleHealthComp.ApplyIntegrityDamageFromHitContext`로 전달한다.

```text
DamageAppliedToIntegrity
= min(DamageRequestedForIntegrity, CurrentIntegrity)

IntegrityAfter
= max(IntegrityBefore - DamageAppliedToIntegrity, 0)

IntegrityOverkillDamage
= max(DamageRequestedForIntegrity - DamageAppliedToIntegrity, 0)
```

Integrity가 처음 0이 되면 기존 `OnVehicleDestroyed`가 한 번 발생한다.
파괴된 차량에 다시 피해를 적용해도 두 번째 파괴 이벤트나 Destroyed FX가 발생하지 않는다.

---

## 9. Shield 자동 재생

하나 이상의 계층이 실제 감소하면 다음을 수행한다.

```text
RestartShieldRegenerationAfterDamage
→ RemainingDelay = ShieldRegenerationDelaySeconds
→ bShieldRegenerating = false
→ Tick 활성화
```

Tick은 먼저 지연 시간을 소비한다.
같은 프레임에 지연이 끝나고 DeltaTime이 남으면 남은 시간만큼 즉시 Shield 재생에 사용한다.

```text
RegeneratedShield
= ShieldRegenerationPerSecond × RegenerationDeltaSeconds

CurrentShield
= min(CurrentShield + RegeneratedShield, MaximumShield)
```

실제 Shield 증가가 시작되는 첫 프레임에 `OnShieldRegenerationStarted`가 발생한다.
최대값에 도달하면 `OnShieldFullyRestored`가 발생하고 Tick을 끈다.

재생 중 다시 유효 피해를 받으면 재생을 중단하고 지연을 처음부터 다시 시작한다.

현재 사용자 검증 기준:

```text
피해 후 남은 지연 5 → 0
Shield 50 → 100
재생 중 재피격 → 지연 5초로 재시작
```

---

## 10. Legacy Fallback

다음 경우 기존 `VehicleHealthComp` 직접 피해 경로를 사용한다.

```text
- HitActor에 VehicleDefenseComp가 없음
- VehicleDefenseComp는 있으나 ActiveDefenseData=None
- VehicleData.DefaultDefenseData=None
```

Fallback 결과:

```text
bUsedLegacyHealthFallback = true
Shield = 0
6방향 Armor = 0
DamageRequestedForIntegrity = BaseDamage
DamageAppliedToIntegrity = 기존 Health 결과의 AppliedDamage
```

기준 BaseDamage 25 한 발의 사용자 검증 결과:

```text
Legacy = 예
Shield = 0 유지
Front / Left / Right / Rear / Top / Bottom Armor = 0 유지
Integrity 100 → 75
Integrity 요청/적용 = 25 / 25
```

Legacy Fallback은 오래된 차량 에셋을 강제 재저장하지 않기 위한 정식 호환 경로다.

---

## 11. 전체 피해 결과

`FCFVehicleDamageResult`는 한 타격의 전체 결과를 보존한다.

주요 필드:

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
ShieldBefore / ShieldAfter
ArmorBefore / ArmorAfter
IntegrityBefore / IntegrityAfter
bShieldBrokenThisHit
bArmorBrokenThisHit
bDestroyedThisHit
IntegrityApplyResult
```

부품 피해와 물리 Impulse 필드는 후속 확장 전까지 0 또는 빈 배열이다.

`BuildIntegrityCompatibilityResult`는 기존 VehicleDebug와 Projectile Pool 경로가 계속 `FCFDamageApplyResult`를 사용할 수 있게 한다.
Shield 또는 Armor만 감소한 경우에도 Integrity가 변하지 않았다는 호환 결과를 명시적으로 만든다.

---

## 12. Blueprint 이벤트

`UCFVehicleDefenseComp`가 제공하는 이벤트:

```text
OnVehicleDamageResolved
OnShieldChanged
OnShieldBroken
OnArmorChanged
OnArmorBroken
OnShieldRegenerationStarted
OnShieldFullyRestored
```

`UCFVehicleHealthComp`가 유지하는 기존 이벤트:

```text
OnVehicleHealthChanged
OnVehicleDamaged
OnVehicleDestroyed
```

이벤트 책임:

```text
C++
- 피해 검증과 분배
- Shield·Armor·Integrity 상태 변경
- 재생 상태
- 최초 파괴 전환
- 결과 캐시와 이벤트 발생 조건

Blueprint
- HUD 표시
- Shield·Armor·피격·파괴 VFX
- 카메라·차량 흔들림
- 파괴 이후의 추가 연출 또는 입력 정책
```

피해량 계산과 생존 판정을 Blueprint에 중복 구현하지 않는다.
CarFight의 게임 사운드 비지원 결정에 따라 방어·피격·파괴 사운드는 완료 범위가 아니다.

---

## 13. Debug와 UI 조회

VehicleDebug는 로컬 플레이어 차량과 현재 선택 대상을 읽기 전용으로 표시한다.

현재 선택 대상 방어 섹션에서 확인 가능한 값:

```text
DefenseReady
ActiveDefenseData
Current / Maximum Shield
Front / Left / Right / Rear / Top / Bottom Armor
Shield Regenerating
Remaining Shield Regeneration Delay
Current / Maximum Integrity
Destroyed
Last Vehicle Damage Result
Legacy Fallback
Armor Direction
Penetration Ratio
Shield Absorbed
Armor Absorbed
Integrity Requested / Applied
```

PIE 중 Details Panel의 에디터 원본 Actor 값은 실제 `UEDPIE` 복제 Actor의 런타임 상태가 아닐 수 있다.
초기화 판정은 PIE World Actor, VehicleDebug 또는 `ManualPIEProbe` 로그를 사용한다.

---

## 14. 현재 테스트 자산

방어 기준 자산:

```text
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_Test
/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV
/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset
/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageArmorPenTest
```

주요 검증 맵:

```text
/Game/Maps/M_VehicleDefensePIE
/Game/Maps/M_VehicleDefensePIE_AP50
/Game/Maps/M_VehicleDefensePIE_Legacy
/Game/Maps/TestMap_DRSalvo
/Game/Maps/TestMap_DRRipple
```

AP50과 Launcher Regression은 원본 전투 자산을 다시 저장하지 않는 격리 체인이다.
이 자산들은 현재 기능 검증용이며 실제 게임 밸런스의 최종값으로 해석하지 않는다.

---

## 15. 현재 검증 상태

### 15.1 공식 빌드

```text
Build Job: 48c0a81e19af4b20a17f628bcc7b723b
Target: CarFight_ReEditor
Platform: Win64
Configuration: Development
Engine: D:\UnrealEngine_Source
Exit Code: 0
Result: Succeeded
```

### 15.2 자동화

```text
Combat Automation Job: 123e4433f5fd4b2a8ea1c1d991b07498
Required: 24/24 Success
Total: 46/46 Success
Failed: 0
```

방어 관련 필수 테스트:

```text
CarFight.Damage.DR_P0_01.DataContract
CarFight.Damage.DR_P0_02.HealthCompatibility
CarFight.Damage.DR_P0_02.DirectionalArmor
CarFight.Damage.DR_P0_02.ShieldArmorPenetration
CarFight.Damage.DR_P0_02.OverflowLegacyFallback
CarFight.Damage.DR_P0_02.ShieldRegeneration
CarFight.Damage.DR_P0_03.RuntimeIntegration
CarFight.Damage.DR_P0_04.DebugBlueprintContract
```

Fitting·Defense 통합 필수 테스트:

```text
CarFight.Fitting.FIT_P0_05.InitialMass
CarFight.Fitting.FIT_P0_05.DefensePIEPipeline
CarFight.Fitting.FIT_P0_05.DefenseMapPIE
CarFight.Fitting.FIT_P0_05.DefenseMapTwoPawnPIE
```

### 15.3 사용자 PIE

```text
DR-PIE-00: 방어 초기화·Fitting Commit PASS
DR-PIE-01: Shield → Front Armor → Integrity·최초 파괴·중복 파괴 없음 PASS
DR-PIE-02: Front 75 / Left 70 / Right 70 / Rear 62.5·독립 Armor PASS
DR-PIE-03: AP0·AP50 관통 비교 PASS
DR-PIE-04: 5초 지연·초당 재생·재피격 지연 Reset PASS
DR-PIE-05: DefaultDefenseData=None Legacy Fallback PASS
DR-PIE-06: Launcher·Projectile·Pool·FX·충돌 회귀 PASS
```

DR-PIE-06 세부 결과:

```text
Salvo 4발 동시·Muzzle 1→4→2→3·Completed·Accepted 4·Failed 0
Salvo 5 Volley 이상 정상
Ripple 4발 0.15초 순차·Muzzle 1→4→2→3·Completed·Accepted 4·Failed 0
Ripple 진행 중 중복 입력 방지·5 Volley 이상 정상
Rocket 추진·Trail·Thruster·Impact FX 정상
동일 차량 Projectile 상호 Impact 없음
일반 차량·월드 충돌 정상
```

판정:

```text
CF-FQ-033 Done
DR-P0-00~07 Done
DR-PIE-00~06 USER PASS
VehicleDefense Current System
```

---

## 16. 현재 비책임 항목

```text
- 실제 휠·엔진·터렛·센서 부품별 내구도와 기능 저하
- 입사각 기반 유효 장갑 두께
- 도탄과 탄자 변형
- 범위 폭발 대상 검색과 거리 감쇠
- 충돌·낙하·환경 피해 발생기
- 지속 피해, 화재, EMP와 상태 이상
- 실제 물리 Impulse 적용
- Armor 수리와 Integrity 회복
- Shield 용량·재생의 에너지 시스템 연동
- 서버 권한 피해, Replication과 예측
- 리스폰
- 완성형 방향별 Armor HUD
```

이 항목들은 현재 구조의 결과 필드와 이벤트를 확장해서 구현하며 P0 계산을 Blueprint에서 우회하지 않는다.

---

## 17. 현재 책임 분리

### C++

```text
- 방어 데이터 검증과 초기화
- Fitting Defense Commit
- HitContext 검증
- 6방향 판정
- Shield·Armor·관통·Integrity 계산
- Shield 재생과 Tick 수명
- Legacy Fallback
- 결과 캐시와 이벤트
- 기존 Health·Pool·Debug 호환
```

### DataAsset

```text
- 차량별 Shield·재생·ArmorResistance
- 방향별 MaximumArmor·DamageMultiplier
- 방어 패키지 질량
- ProjectileData의 DamageData 선택
```

### Blueprint / UMG

```text
- 방어·내구도 표시
- 상태별 VFX와 애니메이션
- 레이아웃·색상·아이콘
- 이벤트 소비
```

---

## 18. 관련 경로

```text
UE/Source/CarFight_Re/Public/CFDamageRuntimeTypes.h
UE/Source/CarFight_Re/Public/CFVehicleDefenseData.h
UE/Source/CarFight_Re/Public/CFVehicleDefenseComp.h
UE/Source/CarFight_Re/Private/CFVehicleDefenseComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h
UE/Source/CarFight_Re/Private/CFVehicleHealthComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleData.h
UE/Source/CarFight_Re/Public/CFDamageData.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
Document/Systems/Combat/HitDamage.md
Document/Systems/Combat/DamageHitContext.md
Document/Systems/Combat/Projectile.md
Document/Plan/VehicleDefenseDamageDesign.md
```

---

## 19. 문서 갱신 조건

다음 변경이 생기면 이 문서를 함께 갱신한다.

```text
- 피해 분배 순서 또는 공식 변경
- Armor 방향 판정 변경
- Shield 재생 정책 변경
- VehicleDefenseData 필드 변경
- VehicleDefenseComp 이벤트·결과 변경
- VehicleHealthComp 소유권 또는 파괴 의미 변경
- Legacy Fallback 폐기 또는 변경
- Fitting Defense Commit 경로 변경
- 부품 피해·범위 피해·Impulse가 Current System으로 승격
- 서버 권한 피해나 Replication 도입
```

---

## 20. 문서 버전 관리

- 현재 문서 버전: `1.0.0`
- 문서 상태: `Current System`

### Changelog

#### v1.0.0 - 2026-08-06

- CF-FQ-033 차량 방어·손상 런타임의 Current System 문서를 신규 작성했다.
- Shield, 6방향 독립 Armor, 방향 배율, ArmorPenetration과 Armor Overflow 공식을 기록했다.
- Vehicle Integrity와 기존 Health 프로퍼티·이벤트 호환을 기록했다.
- Shield 재생 지연·초당 재생·재피격 Reset과 컴포넌트 활성 수명을 기록했다.
- DefaultDefenseData=None Legacy Fallback과 Fitting Snapshot Defense Commit을 기록했다.
- 공식 Build, 전체 Automation과 DR-PIE-00~06 USER PASS를 완료 증거로 등록했다.
- CF-FQ-033 Done과 DR-P0-00~07 Done을 Current 판정으로 기록했다.

### Migration

#### Initial Current System 적용 안내

- CF-FQ-033의 현재 구현 판단은 이 문서와 `HitDamage.md`를 우선한다.
- `VehicleDefenseDamageDesign.md`는 완료 당시 설계·검증·자산 보호 기록으로 유지한다.
- 기존 VehicleData의 `DefaultDefenseData=None`은 오류가 아니라 정식 Legacy Fallback이다.
- 방어를 사용할 차량만 `DefaultDefenseData` 또는 Fitting Snapshot의 DefenseData를 명시적으로 연결한다.
- 기존 `CurrentHealth`, `MaxHealth`와 Health 이벤트 이름은 그대로 사용하지만 의미상 Vehicle Integrity로 읽는다.
- Shield·Armor·Integrity 계산을 Blueprint에 복제하지 않는다.
- 기존 테스트용 격리 에셋과 맵을 제품 밸런스 기본값으로 자동 채택하지 않는다.

---

## 21. 마지막 확인 기준

- 확인 일시: 2026-08-06
- 확인 근거:
  - 실제 C++ 소스와 DataAsset
  - Build Job `48c0a81e19af4b20a17f628bcc7b723b`
  - Combat Automation Job `123e4433f5fd4b2a8ea1c1d991b07498`
  - Defense Automation `7b5dce50680f47a99a29b26b02a577d7`
  - Stored Map Two-Pawn PIE `a4e8048231034105902498f3ec8499f7`
  - 사용자 DR-PIE-00~06 결과
