# HitDamage

- Version: 1.1.0
- Date: 2026-08-06
- Status: Current System
- Features: `CF-FQ-018 피격 판정 및 피해 처리`, `CF-FQ-033 차량 방어·손상 런타임`
- Verification: `CarFight_ReEditor Win64 Development` Build PASS / Combat Automation 46/46 PASS / DR-PIE-00~06 USER PASS

---

## 1. 문서 목적

이 문서는 CarFight의 HitScan과 Projectile 명중 결과가 공용 HitContext를 통해 차량 방어 계층과 Vehicle Integrity에 정확히 한 번 적용되는 현재 피해 처리 계약을 기록한다.

`CF-FQ-018`의 최소 BaseDamage·Health Runtime은 유지되며, `CF-FQ-033`이 그 앞에 Shield와 6방향 Armor 분배 계층을 추가했다.

현재 구현은 더 이상 모든 차량에 BaseDamage를 바로 체력으로 적용하지 않는다.

```text
DefenseData가 있는 차량
DamageData
→ DamageHitContext
→ VehicleDefenseComp
→ Shield
→ 6방향 Armor
→ VehicleHealthComp = Vehicle Integrity

DefenseData가 없는 차량
DamageData
→ DamageHitContext
→ Legacy Health Fallback
→ VehicleHealthComp = Vehicle Integrity
```

Shield·Armor의 상세 공식과 이벤트는 `Document/Systems/Combat/VehicleDefense.md`를 우선한다.

---

## 2. 현재 범위

현재 HitDamage 시스템은 다음 요소를 하나의 흐름으로 본다.

```text
UCFDamageData
UCFProjectileData.DefaultDamageData
FCFDamageHitContext
FCFVehicleDamageResult
FCFDamageApplyResult
UCFVehicleDefenseComp
UCFVehicleHealthComp
ACFVehiclePawn HitScan / Projectile 연결
ACFProjectileActor 첫 Impact·Pool 결과 보존
```

| 요소 | 현재 역할 |
| --- | --- |
| `UCFDamageData` | BaseDamage, DamageType, 자기 피해 정책과 ArmorPenetration을 공급한다. |
| `UCFProjectileData.DefaultDamageData` | Projectile과 가상 HitScan이 사용할 DamageData 직접 참조를 소유한다. |
| `FCFDamageHitContext` | 대상 Actor·Component, 위치, 노멀, 입사 방향과 발사 주체를 기록한다. |
| `UCFVehicleDefenseComp` | 정식 피해 진입점, Shield·Armor·Integrity 분배와 Legacy Fallback을 처리한다. |
| `FCFVehicleDamageResult` | 한 타격의 모든 방어층 결과를 기록한다. |
| `UCFVehicleHealthComp` | Vehicle Integrity, 최초 파괴 상태와 기존 Health 이벤트를 소유한다. |
| `FCFDamageApplyResult` | Integrity 적용 결과와 기존 Debug·Pool 호환 결과를 유지한다. |
| `ACFProjectileActor` | 한 활성화의 첫 유효 Impact만 해결하고 결과를 Pool 반환 뒤까지 보존한다. |

---

## 3. DamageData 기준

실제 DamageData 직접 참조 소유 위치:

```text
ProjectileData.DefaultDamageData
```

WeaponData나 Projectile Actor가 별도 DamageData 복사본을 소유하지 않는다.

현재 주요 필드:

```text
DamageId
DamageType
BaseDamage
bCanDamageSelf
ArmorPenetration
ModuleDamageScale
bUseRadialDamage와 범위 피해 설정
ImpulseStrength
```

P0 직접 명중에서 실제 사용하는 값:

```text
BaseDamage
bCanDamageSelf
ArmorPenetration
DamageId
DamageType = 결과 속성·확장 기준
```

`ModuleDamageScale`, 범위 피해와 Impulse는 현재 결과 구조에 예약돼 있지만 실제 부품·범위·물리 적용은 수행하지 않는다.

기준 AP0 자산:

```text
/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset
BaseDamage = 25
ArmorPenetration = 0
```

기준 AP50 자산:

```text
/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageArmorPenTest
BaseDamage = 25
ArmorPenetration = 50
```

---

## 4. 공용 피해 진입점

현재 HitScan과 Projectile의 정식 피해 적용 진입점:

```text
UCFVehicleDefenseComp::TryApplyDamageToActor(
    const FCFDamageHitContext& DamageHitContext,
    FCFVehicleDamageResult& OutVehicleDamageResult)
```

처리 순서:

```text
HitContext 공통 검증
→ HitActor의 VehicleDefenseComp 검색
→ 있으면 ApplyDamageFromHitContext
   → ActiveDefenseData가 있으면 Shield·Armor·Integrity
   → ActiveDefenseData=None이면 Legacy Health Fallback
→ VehicleDefenseComp 자체가 없으면 VehicleHealthComp 직접 Fallback
```

기존 함수는 호환을 위해 유지한다.

```text
UCFVehicleHealthComp::ApplyDamageFromHitContext
UCFVehicleHealthComp::TryApplyDamageToActor
```

새 코드가 방어 계층을 우회해야 하는 특별한 이유가 없다면 기존 Health 정적 진입점을 직접 호출하지 않는다.

---

## 5. HitScan 처리

현재 HitScan 흐름:

```text
Fire 명령 승인
→ WeaponHit Trace Blocking Hit
→ FCFDamageHitContext 생성
→ UCFVehicleDefenseComp::TryApplyDamageToActor
→ FCFVehicleDamageResult 저장
→ BuildIntegrityCompatibilityResult
→ 기존 Fire·Damage Debug 갱신
```

HitScan은 Projectile과 같은 HitContext와 방어 계산을 사용한다.
방어 데이터가 있는 같은 대상에 같은 DamageData와 같은 HitContext를 입력하면 Projectile과 같은 Shield·Armor·Integrity 결과를 만든다.

---

## 6. Projectile 처리

현재 Projectile 흐름:

```text
Projectile 활성화
→ 이동·보조 연속 Sphere Sweep
→ 첫 유효 Blocking Impact
→ FCFDamageHitContext 생성
→ UCFVehicleDefenseComp::TryApplyDamageToActor
→ FCFVehicleDamageResult와 Integrity 호환 결과 저장
→ Impact FX 요청
→ Projectile 비활성화
→ Pool 반환
```

한 활성화에서 피해와 Impact는 첫 유효 충돌 한 번만 해결한다.

```text
OnComponentHit
+ 보조 연속 Sphere Sweep
→ 공용 ResolveProjectileImpact
→ bImpactResolvedThisActivation으로 중복 차단
```

Pool 반환 Callback은 피해를 다시 적용하지 않는다.
저장된 `FCFVehicleDamageResult`와 `FCFDamageApplyResult`만 Pawn Debug로 복사한다.

---

## 7. 입력 검증과 거부

공통 검증 기준:

```text
- Blocking Hit
- 유효 HitActor
- 유효 DamageData
- BaseDamage > 0
- 자기 피해 정책
- 유효 VehicleHealthComp
- 대상 미파괴 상태
```

현재 주요 거부 사유:

| 거부 사유 | 의미 |
| --- | --- |
| `NoBlockingHit` | Blocking Hit이 아니다. |
| `MissingHitActor` | 대상 Actor가 없다. |
| `MissingDamageData` | DamageData가 연결되지 않았다. |
| `NonPositiveDamage` | BaseDamage 또는 명시 Integrity 피해가 0 이하이다. |
| `SelfDamageBlocked` | 공격자와 대상이 같고 자기 피해가 금지됐다. |
| `TargetMismatch` | DefenseComp 소유 Actor와 HitActor가 다르다. |
| `MissingHealthComponent` | 차량 내구도를 소유할 HealthComp가 없다. |
| `TargetDestroyed` | 대상 차량이 이미 파괴 상태다. |

월드 지형과 비차량 Actor는 충돌·Impact FX 대상이 될 수 있지만 VehicleHealthComp가 없으므로 차량 Integrity를 변경하지 않는다.

---

## 8. Vehicle Integrity

`UCFVehicleHealthComp`의 기존 프로퍼티 이름:

```text
MaxHealth
CurrentHealth
bHealthInitialized
bDestroyed
```

현재 의미:

```text
MaxHealth = Maximum Vehicle Integrity
CurrentHealth = Current Vehicle Integrity
bDestroyed = Integrity가 처음 0이 된 상태
```

명시 피해 함수:

```text
ApplyIntegrityDamageFromHitContext(
    const FCFDamageHitContext& DamageHitContext,
    float RequestedIntegrityDamage,
    FCFDamageApplyResult& OutDamageApplyResult)
```

적용식:

```text
AppliedDamage = clamp(RequestedIntegrityDamage, 0, CurrentIntegrity)
CurrentIntegrity = CurrentIntegrity - AppliedDamage
```

기존 `ApplyDamageFromHitContext`는 `DamageData.BaseDamage`를 읽어 명시 Integrity 함수로 전달하는 호환 Wrapper다.

---

## 9. 파괴 상태

Integrity가 0 이하가 되는 첫 타격에서 다음 상태가 한 번만 발생한다.

```text
bDestroyed = true
FCFDamageApplyResult.bDestroyedThisHit = true
FCFVehicleDamageResult.bDestroyedThisHit = true
OnVehicleDestroyed 발생
```

이미 파괴된 차량의 후속 피해는 `TargetDestroyed`로 거부한다.

```text
- Integrity 추가 감소 없음
- OnVehicleDestroyed 재발생 없음
- Destroyed FX 두 번째 발생 없음
```

파괴 상태는 현재 생존 판정과 이벤트를 소유한다.
차량 입력 정지, Chaos 파괴 물리, 메시 교체와 리스폰은 자동 수행하지 않는다.

---

## 10. Legacy Fallback

`VehicleData.DefaultDefenseData=None`인 기존 차량은 다음 흐름을 사용한다.

```text
BaseDamage
→ VehicleHealthComp.ApplyDamageFromHitContext
→ Vehicle Integrity 직접 감소
```

전체 결과에는 다음을 기록한다.

```text
bUsedLegacyHealthFallback = true
Shield 피해 = 0
Armor 피해 = 0
DamageRequestedForIntegrity = 기존 RequestedDamage
DamageAppliedToIntegrity = 기존 AppliedDamage
IntegrityApplyResult = 기존 FCFDamageApplyResult
```

이 경로는 기존 VehicleData와 ProjectileData를 강제로 재저장하지 않기 위한 정식 호환 계약이다.

---

## 11. 기존 결과 호환

`FCFVehicleDamageResult`가 정식 전체 결과다.
기존 UI·Debug·Pool은 계속 `FCFDamageApplyResult`를 읽을 수 있다.

```text
UCFVehicleDefenseComp::BuildIntegrityCompatibilityResult
```

동작:

```text
- 실제 Integrity 피해가 있으면 VehicleHealthComp 결과를 그대로 반환한다.
- Legacy Fallback이면 기존 Health 결과를 그대로 반환한다.
- Shield 또는 Armor만 감소했으면 Integrity 전후 값이 같은 호환 결과를 생성한다.
```

따라서 기존 Debug가 Shield·Armor 타격을 차량 Integrity 감소로 잘못 표시하지 않는다.

---

## 12. 이벤트

`UCFVehicleHealthComp`의 기존 Blueprint 이벤트는 유지된다.

```text
OnVehicleHealthChanged
OnVehicleDamaged
OnVehicleDestroyed
```

발생 조건:

```text
OnVehicleHealthChanged
= Vehicle Integrity가 실제 변경될 때

OnVehicleDamaged
= Shield·Armor를 통과한 피해가 Integrity에 실제 적용될 때

OnVehicleDestroyed
= Integrity가 처음 0이 될 때 한 번
```

Shield와 Armor만 감소한 타격에서는 Health 이벤트가 발생하지 않는다.
방어 계층 이벤트는 `VehicleDefenseComp`가 별도로 제공한다.

---

## 13. Debug

현재 Debug는 두 결과를 구분한다.

```text
FCFVehicleDamageResult
= Shield·Armor·관통·Integrity 전체 결과

FCFDamageApplyResult
= Vehicle Integrity 호환 결과
```

주요 확인값:

```text
DamageAccepted
AppliedToAnyLayer
LegacyFallback
RejectReason
DamageId
ArmorDirection
RequestedDamage
ShieldAbsorbed
ArmorPenetrationRatio
ArmorAbsorbed
IntegrityRequested
IntegrityApplied
ShieldBefore / After
ArmorBefore / After
IntegrityBefore / After
DestroyedThisHit
HitActor
HitComponentName
```

VehicleDebug의 `선택 대상` 섹션은 현재 선택된 차량의 Shield, 6방향 Armor, 재생 지연, Integrity와 마지막 전체 피해 결과를 읽기 전용으로 표시한다.

---

## 14. 현재 검증 상태

### 공식 빌드

```text
Build Job: 48c0a81e19af4b20a17f628bcc7b723b
CarFight_ReEditor Win64 Development
Exit Code 0
Result Succeeded
```

### 전체 자동화

```text
Combat Job: 123e4433f5fd4b2a8ea1c1d991b07498
Required 24/24 Success
Total 46/46 Success
Failed 0
```

핵심 Damage 테스트:

```text
DataContract
HealthCompatibility
DirectionalArmor
ShieldArmorPenetration
OverflowLegacyFallback
ShieldRegeneration
RuntimeIntegration
DebugBlueprintContract
```

### 사용자 PIE

```text
Shield → Armor → Integrity 순서 PASS
6방향 독립 Armor PASS
AP0 / AP50 관통 분배 PASS
Shield 지연·재생·재피격 Reset PASS
Legacy Integrity Fallback PASS
최초 파괴·후속 중복 파괴 방지 PASS
Launcher·Projectile·Pool·FX·충돌 통합 회귀 PASS
```

판정:

```text
CF-FQ-018 Done 유지
CF-FQ-033 Done
HitDamage Current System
VehicleDefense Current System
```

---

## 15. 현재 책임

```text
- HitScan과 Projectile 피격을 같은 HitContext로 변환
- 정식 VehicleDefense 진입점 호출
- DamageData와 대상 유효성 검증
- Shield·Armor 이후 남은 Integrity 피해 전달
- Legacy 차량의 BaseDamage 직접 Integrity 호환
- Vehicle Integrity와 최초 파괴 상태 소유
- Projectile 첫 Impact 피해 1회 보장
- Pool 반환 뒤 결과 보존
- Blueprint가 사용할 변경·피격·파괴 이벤트 제공
- Debug용 전체 결과와 호환 결과 기록
```

---

## 16. 현재 비책임 항목

```text
- 도탄과 입사각 유효 장갑 두께
- 실제 부품별 독립 체력과 기능 저하
- 범위 폭발 대상 검색·거리 감쇠
- 지속 피해와 화재
- 충돌·낙하·환경 피해 발생기
- 물리 Impulse 적용
- 서버 권한 피해와 Replication
- 리스폰
- 파괴 시 입력·Chaos 물리 자동 정지
- 완성형 파괴 메시 전환
```

Shield·Armor의 데이터와 계산은 `VehicleDefense.md`가 소유한다.

---

## 17. 책임 분리

### C++

```text
- HitContext와 DamageData 검증
- 정식 Defense 진입점 선택
- Integrity 적용과 파괴 상태
- 첫 Impact 중복 방지
- 전체·호환 결과 생성
- 이벤트 발생 조건
```

### DataAsset

```text
- ProjectileData.DefaultDamageData 연결
- BaseDamage와 ArmorPenetration
- VehicleData.DefaultDefenseData 선택
```

### Blueprint

```text
- 피해·방어 HUD
- 피격·파괴 VFX와 애니메이션
- 이벤트 기반 표시
```

피해 계산과 생존 판정은 Blueprint에 중복 작성하지 않는다.
게임 사운드는 프로젝트 완료 범위에 포함하지 않는다.

---

## 18. 관련 경로

```text
Document/Systems/Combat/VehicleDefense.md
Document/Systems/Combat/DamageHitContext.md
Document/Systems/Combat/Projectile.md
UE/Source/CarFight_Re/Public/CFDamageData.h
UE/Source/CarFight_Re/Public/CFDamageTypes.h
UE/Source/CarFight_Re/Public/CFDamageRuntimeTypes.h
UE/Source/CarFight_Re/Public/CFVehicleDefenseComp.h
UE/Source/CarFight_Re/Private/CFVehicleDefenseComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h
UE/Source/CarFight_Re/Private/CFVehicleHealthComp.cpp
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

---

## 19. 문서 갱신 조건

```text
- 공용 피해 진입점 변경
- HitContext 또는 DamageData 직접 참조 위치 변경
- Vehicle Integrity 소유권 변경
- 파괴 상태·이벤트 의미 변경
- Legacy Fallback 변경 또는 폐기
- Projectile 첫 Impact·Pool 결과 수명 변경
- Shield·Armor 결과와 기존 FCFDamageApplyResult 호환 변경
- 부품·범위·환경 피해가 Current System으로 승격
- 서버 권한 피해 도입
```

---

## 20. 문서 버전 관리

- 현재 문서 버전: `1.1.0`
- 문서 상태: `Current System`

### Changelog

#### v1.1.0 - 2026-08-06

- CF-FQ-033 VehicleDefense 통합 이후의 정식 HitDamage 흐름으로 문서를 갱신했다.
- `VehicleDefenseComp::TryApplyDamageToActor`를 HitScan·Projectile 공용 진입점으로 기록했다.
- 기존 Health 프로퍼티를 Vehicle Integrity 의미로 유지하는 호환 계약을 기록했다.
- Shield·Armor만 감소한 결과와 기존 `FCFDamageApplyResult`의 호환 변환을 기록했다.
- DefaultDefenseData=None Legacy Fallback과 최초 파괴·중복 파괴 방지 계약을 기록했다.
- 공식 Build, 전체 Automation과 DR-PIE-00~06 USER PASS를 반영했다.

#### v1.0.1 - 2026-07-24

- HitDamage 비책임 항목에서 SFX를 제거하고 완성형 파괴 표현을 VFX 전용으로 정리했다.
- 게임 사운드 비지원 결정 CF-PDL-0009와 현재 Blueprint 책임 설명을 정합화했다.

#### v1.0.0 - 2026-07-15

- CF-FQ-018 최소 Damage Runtime의 Current System 문서를 최초 작성했다.
- DamageData, DamageHitContext, DamageApplyResult와 VehicleHealthComp 소유권을 기록했다.
- HitScan·Projectile 공용 피해 적용과 Projectile 중복 차감 방지를 기록했다.

### Migration

#### v1.1.0 적용 안내

- 신규 피해 호출부는 `VehicleHealthComp::TryApplyDamageToActor`가 아니라 `VehicleDefenseComp::TryApplyDamageToActor`를 사용한다.
- 기존 Health 함수와 이벤트 이름은 삭제하거나 변경하지 않는다.
- `MaxHealth`와 `CurrentHealth`는 의미상 Maximum·Current Vehicle Integrity다.
- Shield와 Armor의 상세 계산은 `VehicleDefense.md`를 사용한다.
- DefenseData가 없는 기존 차량은 정식 Legacy Fallback으로 계속 동작한다.
- 완료 Plan보다 이 Current System 문서를 우선한다.

#### v1.0.1 적용 안내

- 파괴 후속 표현은 VFX와 애니메이션만 사용한다.
- 피격음과 파괴음을 포함한 게임 사운드는 Blueprint 책임이나 후속 기능으로 추가하지 않는다.

---

## 21. 마지막 확인 기준

- 확인 일시: 2026-08-06
- 확인 근거:
  - 실제 Damage·Defense·Health·Pawn·Projectile C++ 소스
  - Build Job `48c0a81e19af4b20a17f628bcc7b723b`
  - Combat Automation Job `123e4433f5fd4b2a8ea1c1d991b07498`
  - 사용자 DR-PIE-00~06 결과
