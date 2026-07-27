# HitDamage

- Version: 1.0.1
- Date: 2026-07-24
- Status: Current System
- Feature: `CF-FQ-018 피격 판정 및 피해 처리`
- Verification: `CarFight_ReEditor Win64 Development` Build PASS / User Single PIE PASS

---

## 1. 문서 목적

이 문서는 CarFight의 현재 최소 피해 처리 시스템이 실제로 어떤 데이터를 입력받고, 차량 체력을 어떻게 감소시키며, 파괴 상태와 Blueprint 이벤트를 어떻게 남기는지 기록한다.

이 문서는 장갑, 부위별 피해, 폭발 피해와 완성형 파괴 연출을 설명하는 미래 설계 문서가 아니다.
현재 구현과 P0 사용자 PIE에서 확인된 최소 Damage Runtime만 현재 기준으로 다룬다.

---

## 2. 현재 범위

현재 HitDamage 시스템은 다음 요소를 하나의 흐름으로 본다.

```text
DamageData
+ ProjectileData.DefaultDamageData
+ FCFDamageHitContext
+ FCFDamageApplyResult
+ UCFVehicleHealthComp
+ ACFVehiclePawn HitScan / Projectile 연결
```

현재 기준 핵심 데이터와 클래스:

| 요소 | 현재 역할 |
| --- | --- |
| `UCFDamageData` | 기본 피해량과 자기 피해 허용 정책 공급 |
| `UCFProjectileData.DefaultDamageData` | Projectile 또는 가상 HitScan이 사용할 DamageData 직접 참조 |
| `FCFDamageHitContext` | 피격 Actor, Component, 위치, 노멀, 입사 방향과 발사 주체 기록 |
| `FCFDamageApplyResult` | 피해 적용 여부, 거부 사유, 요청/적용 피해량과 체력 전후 값 기록 |
| `UCFVehicleHealthComp` | 차량 최대 체력, 현재 체력과 파괴 상태 소유 |
| `ACFVehiclePawn` | HitScan과 Projectile 결과를 공용 피해 적용 진입점으로 연결 |
| `ACFProjectileActor` | 첫 유효 Impact에서 피해를 한 번 적용하고 결과를 Pool 반환 시 전달 |

---

## 3. 현재 데이터 기준

### 3.1 차량 내구도

`UCFVehicleData`의 `VehicleDurabilityConfig`가 차량별 기본 최대 체력을 공급한다.

현재 기본값:

```text
MaxHealth = 100
```

런타임에서는 `UCFVehicleHealthComp`가 다음 값을 소유한다.

```text
MaxHealth
CurrentHealth
bHealthInitialized
bVehicleDestroyed
```

### 3.2 피해 데이터

현재 기준 DamageData 자산:

```text
/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset
```

확인된 값:

```text
DamageId = ProtoDirectHit
DamageType = Kinetic
BaseDamage = 25
bCanDamageSelf = false
ArmorPenetration = 0
ImpulseStrength = 0
```

현재 `BaseDamage`만 최소 차량 체력 감소에 사용한다.
`ArmorPenetration`, 범위 피해, 모듈 피해 배율과 충격 힘은 데이터 필드로 존재하지만 현재 P0 체력 계산에는 적용하지 않는다.

### 3.3 ProjectileData 연결

현재 기준 ProjectileData 자산:

```text
/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell
```

현재 연결:

```text
DefaultDamageData
→ /Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset
```

DamageData 직접 참조 슬롯은 `ProjectileData.DefaultDamageData`가 소유한다.
WeaponData와 Projectile Actor가 별도의 DamageData 복사본을 소유하지 않는다.

---

## 4. 현재 피해 적용 흐름

### 4.1 HitScan

```text
Fire 명령 승인
→ WeaponHit Trace Blocking Hit
→ FCFDamageHitContext 생성
→ UCFVehicleHealthComp::TryApplyDamageToActor()
→ 대상 VehicleHealthComp 검색
→ DamageData.BaseDamage 적용
→ FCFDamageApplyResult 기록
```

HitScan은 Projectile과 동일한 `FCFDamageHitContext`와 공용 피해 적용 진입점을 사용한다.

### 4.2 Projectile

```text
Projectile 활성화
→ 첫 유효 Blocking Impact
→ FCFDamageHitContext 생성
→ UCFVehicleHealthComp::TryApplyDamageToActor()
→ DamageData.BaseDamage 적용
→ FCFDamageApplyResult 저장
→ Projectile 비활성화
→ Pool 반환 Callback에서 저장 결과 전달
```

Projectile은 한 활성화에서 첫 유효 Impact만 처리한다.
`OnComponentHit`, 보조 연속 Sphere Sweep과 Pool 반환 Callback이 같은 피해를 중복 적용하지 않는다.

Pool 반환 경로는 피해를 다시 계산하거나 적용하지 않고 Projectile Actor에 저장된 `FCFDamageApplyResult`만 Pawn의 Debug 상태로 복사한다.

---

## 5. 차량 체력과 파괴 상태

유효 피해가 들어오면 현재 체력은 다음 식으로 감소한다.

```text
AppliedDamage = Clamp(BaseDamage, 0, CurrentHealth)
CurrentHealth = CurrentHealth - AppliedDamage
```

현재 기본값에서는 다음 누적 흐름이 발생한다.

```text
100 → 75 → 50 → 25 → 0
```

체력이 0 이하가 되는 첫 타격에서 다음 상태가 한 번만 발생한다.

```text
bVehicleDestroyed = true
bTargetDestroyedThisHit = true
OnVehicleDestroyed 발생
```

이미 파괴된 차량에 대한 후속 피해 요청은 체력을 다시 감소시키지 않고 `TargetDestroyed`로 거부한다.
`OnVehicleDestroyed`도 다시 발생하지 않는다.

현재 파괴 상태는 생존 판정과 이벤트 상태만 담당한다.
차량 입력 중지, Chaos 물리 전환과 완성형 파괴 연출은 아직 자동으로 수행하지 않는다.

---

## 6. 피해 적용 거부 사유

현재 `FCFDamageApplyResult`는 피해가 적용되지 않은 이유를 명시적으로 기록한다.

| 거부 사유 | 현재 의미 |
| --- | --- |
| `InvalidHitContext` | 피격 문맥이 유효하지 않음 |
| `NoDamageData` | 사용할 DamageData가 연결되지 않음 |
| `InvalidDamageAmount` | 적용할 피해량이 0 이하이거나 유효하지 않음 |
| `InvalidTarget` | 피해 대상 Actor가 유효하지 않음 |
| `NoHealthComponent` | 대상에 `UCFVehicleHealthComp`가 없음 |
| `SelfDamageBlocked` | 공격자와 대상이 같고 `bCanDamageSelf=false`임 |
| `TargetDestroyed` | 대상 차량이 이미 파괴 상태임 |

월드 지형, 벽과 비차량 Actor에 충돌하더라도 차량 Health Component가 없으면 차량 체력을 변경하지 않는다.

---

## 7. Blueprint 이벤트

`UCFVehicleHealthComp`는 다음 Blueprint 이벤트를 제공한다.

```text
OnVehicleHealthChanged
OnVehicleDamaged
OnVehicleDestroyed
```

현재 책임 분리:

### C++

```text
- 피해 요청 유효성 검증
- 자기 피해 정책 판정
- BaseDamage 적용
- 현재 체력 감소
- 파괴 상태 1회 전환
- 피해 적용 결과와 거부 사유 기록
```

### Blueprint

```text
- 피격 VFX
- 카메라 또는 차량 흔들림
- 체력 UI 표시
- 파괴 VFX와 애니메이션
- 파괴 후 입력·물리 정지 연출 연결
```

프로젝트 결정 `CF-PDL-0009`에 따라 피격음과 파괴음을 포함한 게임 사운드는 Blueprint 책임에도 포함하지 않는다.

피해량 계산과 생존 판정 로직은 Blueprint에 중복 작성하지 않는다.

---

## 8. Debug 확인

VehicleDebug Panel의 Weapon 또는 Damage 관련 표시에서 다음 결과를 확인할 수 있다.

```text
DamageApplied
RejectReason
RequestedDamage
AppliedDamage
PreviousHealth
CurrentHealth
TargetDestroyedThisHit
HitActor
HitComponentName
```

현재 차량 차체 명중의 P0 기준 `HitComponentName`은 `SM_Body` 또는 승인된 시각 피격 컴포넌트다.

---

## 9. 현재 검증 상태

공식 빌드:

```text
Target: CarFight_ReEditor
Platform: Win64
Configuration: Development
Engine: D:\UnrealEngine_Source
Result: PASS
```

사용자 싱글 PIE:

```text
- DamageData가 연결된 발사체의 차량 피해 적용 정상
- BaseDamage 기준 체력 누적 감소 정상
- 체력 0 이하 파괴 상태 전환 정상
- 파괴 이후 추가 피해 거부 정상
- 파괴 이벤트 중복 발생 없음
- 판정: CF-TC-016 PASS / CF-FQ-018 Done
```

전체 FPS·속도 매트릭스, 이동 차량 대상과 주행 중 반복 전투 검증은 `CF-FQ-019` 확장 회귀 범위다.

---

## 10. 현재 책임

현재 HitDamage 시스템의 책임:

- HitScan과 Projectile 피격을 동일 피해 입력으로 변환
- DamageData의 최소 직접 피해를 차량 체력에 적용
- 차량 체력과 파괴 상태 소유
- 자기 피해 금지와 무효 대상 거부
- 한 Impact당 피해 1회 보장
- Blueprint가 사용할 체력 변경·피해·파괴 이벤트 제공
- Debug가 읽을 수 있는 피해 적용 결과 기록

---

## 11. 현재 비책임 항목

다음은 현재 HitDamage 시스템의 완료 범위가 아니다.

```text
- 장갑 두께와 관통 계산
- 입사각과 도탄
- 휠·터렛·엔진 등 부위별 독립 체력
- 범위 폭발 피해
- 지속 피해와 화재
- 피해 저항과 상태 이상
- 서버 권한 피해와 복제
- 리스폰
- 파괴 시 입력/Chaos 물리 자동 정지
- 완성형 파괴 VFX
- Geometry Collection 파괴 전환
```

---

## 12. 핵심 결론

현재 HitDamage는,

**신뢰 가능한 시각 차체 HitContext를 DamageData의 BaseDamage와 차량 런타임 체력·파괴 상태로 정확히 한 번 연결하는 싱글플레이 최소 피해 처리 시스템**이다.

> HitDamage는 현재 HitScan과 Projectile이 공유하는 피해 적용 계약, 차량 체력 소유권과 파괴 상태 1회 전환을 제공한다.

---

## 13. 문서 갱신 조건

다음 변경이 생기면 이 문서를 함께 갱신한다.

- `UCFVehicleHealthComp`의 체력 또는 파괴 상태 소유권 변경
- `FCFDamageApplyResult` 필드나 거부 사유 변경
- `TryApplyDamageToActor()` 공용 피해 적용 계약 변경
- DamageData 직접 참조 소유 위치 변경
- HitScan/Projectile 피해 적용 경로 변경
- 자기 피해 정책 변경
- 파괴 상태가 입력·물리·연출을 직접 제어하도록 확장
- 장갑·모듈·범위 피해가 현재 시스템 책임으로 승격

---

## 14. 문서 버전 관리

- 현재 문서 버전: `1.0.1`
- 문서 상태: `Current System`

### Changelog

#### v1.0.1 - 2026-07-24

- HitDamage 비책임 항목에서 SFX를 제거하고 완성형 파괴 표현을 VFX 전용으로 정리
- 게임 사운드 비지원 결정 CF-PDL-0009와 현재 Blueprint 책임 설명을 정합화

#### v1.0.0 - 2026-07-15

- `CF-FQ-018` 최소 Damage Runtime의 현재 구현 문서 최초 작성
- DamageData, DamageHitContext, DamageApplyResult와 VehicleHealthComp 소유권 기록
- HitScan/Projectile 공용 피해 적용과 Projectile 중복 차감 방지 흐름 기록
- 사용자 싱글 PIE 통과와 `CF-TC-016 PASS` 기록

### Migration

#### v1.0.1 적용 안내

- 코드, DamageData와 VehicleHealthComp 런타임은 변경하지 않는다.
- 파괴 후속 표현은 VFX와 애니메이션만 사용한다.
- 피격음과 파괴음을 포함한 게임 사운드는 Blueprint 책임이나 후속 기능으로 추가하지 않는다.

#### Initial Current System 적용 안내

- 현재 구현 판단은 `Document/Plan/HitDamage/ImplementationDesign.md`보다 이 문서를 우선한다.
- HitDamage Plan은 완료 당시 설계와 검증 체크포인트 보존용으로 유지한다.
- 기존 VehicleData는 `VehicleDurabilityConfig.MaxHealth=100` C++ 기본값을 사용할 수 있다.
- 기준 ProjectileData는 `DefaultDamageData`를 직접 참조해야 한다.
- 파괴 상태가 입력 또는 Chaos 물리를 자동 중지한다고 가정하지 않는다.

---

## 15. 마지막 확인 기준

- 확인 일시: 2026-07-15
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFDamageData.h`
  - `UE/Source/CarFight_Re/Public/CFDamageTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleHealthComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
  - `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
  - `/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell`
  - `/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset`
  - 사용자 싱글 PIE 정상 동작 확인
