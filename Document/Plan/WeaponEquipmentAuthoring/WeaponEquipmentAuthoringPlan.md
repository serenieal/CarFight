# CF-FQ-055 Weapon Equipment Authoring Guide Plan

- 문서 버전: v0.2.7
- 최근 갱신일: 2026-09-29
- 문서 상태: Current / WEA-P0-05 Final Technical PASS Preserved / Standalone USER Acceptance Paused / CF-FQ-058 Backend Reuse Input
- Feature: `CF-FQ-055 Weapon Equipment Authoring Guide`
- Work: `wrk_1688f833905941ba8cf87a43cb5945a0`
- Project: `CarFight`
- Project ID: `carfight`
- Repository Role: `main_game`
- Branch: `SwitchSourceEngine`
- Official Engine: Unreal Engine 5.8 Source Build
- 공통 Builder 규약: `Document/Systems/DataManagement/BuilderAuthoringStandard.md v1.0.0`

---

## 0. 2026-09-29 USER 방향 재기준점

WEA-P0-05 Final Technical PASS 자체는 유효하며 재검증하거나 무효화하지 않는다.

USER Acceptance를 진행하면서 사용자가 무기 한 개의 저수준 수치와 child DataAsset 구성을 단계별로 직접 결정하는 Workflow가 1인 개발의 실제 운영 목적과 맞지 않는다는 상위 요구가 확정됐다.

새 방향은 `CF-FQ-058 CarFight Content Catalog & Authoring System`이 소유한다.

```text
사람이 Weapon Guide에서 숫자/DA를 하나씩 제작
→ 기본 사용자 경로에서 중단

USER / AI가 다수 콘텐츠를 기획
→ Authoritative Workbook
→ Generic Content Compiler
→ typed Weapon backend
→ Generated DataAssets
```

따라서 현재 Weapon Guide의 standalone USER Acceptance는 Paused다. 기존 typed validation, multi-asset durable create, provider reuse, partial recovery, Completion Bundle 기술 기반은 폐기하지 않고 CF-FQ-058의 Weapon Provider / Compiler backend 입력으로 보존한다.

향후 사람이 직접 사용하는 Weapon Guide가 필요한지는 CCAS Catalog / AI / Batch Authoring 구현 뒤 fallback 또는 advanced authoring UX 관점에서 재평가한다.

---

## 1. 목적

이 Plan은 CarFight에서 **무장 하나를 실제로 제작하는 정상 사용자 Workflow(작업 흐름)**를 제공하는 `Weapon Equipment Authoring Guide`의 설계, 구현, 검증과 완료 조건을 정의한다.

이 Guide는 `Equipment Builder`를 대체하지 않는다.

```text
Weapon Equipment Authoring Guide
= 무장 콘텐츠 자체를 제작하는 곳

Equipment Builder
= 완성된 TurretMountData + WeaponData 조합을 EquipmentPreset으로 조립·관리·호환성 검토·최종 Apply하는 곳
```

두 화면의 경계는 DataAsset 개수가 아니라 **사용자 업무가 실제로 바뀌는 지점**으로 정의한다.

정상 흐름:

```text
무장을 만든다
→ Weapon Equipment Authoring Guide
→ 무장 제작 완료 Bundle
→ 필요하면 Equipment Builder로 이동
→ EquipmentPreset 조립/관리
```

`CF-FQ-054 Equipment Authoring / Guided Equipment Builder`는 이 Feature가 Acceptance Ready가 되어도 자동 재개하지 않는다. 별도 사용자 지시 전까지 Paused 상태를 유지한다.

---

## 2. WEA-P0-00R 재기준점 결론

### 2.1 재기준점 사유

초기 v0.1.0은 아래 고정 exact8 Step을 전제로 했다.

```text
Basic Info
Mount Structure
Turret Setup
Weapon Behavior
Projectile / Ammo / Damage
Optional Features
Validation
Complete
```

이 구조는 이후 확정된 `BuilderAuthoringStandard.md v1.0.0`과 실제 Runtime Capability를 다시 대조한 결과 그대로 유지하지 않는다.

주요 문제는 다음과 같다.

```text
- Projectile / Damage / Ammo라는 서로 다른 사용자 판단을 한 Page에 결합
- Launcher / Heat / Charge를 하나의 Optional Page에 결합
- 실제 Runtime이 지원하는 Propulsion / Missile Flight / Guidance가 Guided 범위에서 누락
- Guide 밖 Native Editor를 정상 제작 필수 경로로 사용하려는 부분 존재
- 고정 enum index 증가/감소 방식이 Conditional Step과 맞지 않음
```

따라서 v0.2.0부터 **Stable StepId + visible projection + Capability 기반 Conditional Step**으로 재설계한다.

### 2.2 WEA-P0-00R 판정

```text
P0 = 0
blocking P1 = 0
Verdict = PASS
Next = WEA-P0-01 Core Workflow / State / Naming correction
```

기존 Prototype Source는 아직 Product Current System이 아니며 Build Acceptance도 받지 않았다. 따라서 Prototype 구조를 보존하기 위해 새 규약을 약화하지 않고, 현재 accepted 설계에 맞춰 직접 교정한다.

---

## 3. 실제 데이터 소유 관계

Fresh Source와 persisted Product evidence 기준 실제 소유 관계는 다음과 같다.

```text
CFEquipmentPresetData
├─ DefaultTurretMountData -> CFTurretMountData
└─ DefaultWeaponData      -> CFWeaponData
                              ├─ DefaultProjectileData -> CFProjectileData
                              │                           └─ DefaultDamageData -> CFDamageData
                              ├─ DefaultAmmoData       -> CFAmmoData (finite ammo에서 사용)
                              ├─ AmmoTypeId            -> compatibility fallback
                              ├─ LauncherFirePatternConfig / LauncherReleaseConfig
                              ├─ Heat scalar fields
                              └─ Charge scalar fields
```

`CFProjectileData`는 추가로 현재 Runtime Capability를 소유한다.

```text
Projectile movement / actor / collision
PropulsionConfig
MissileFlightConfig
MissileGuideConfig
DefaultDamageData
```

### 3.1 중요한 책임 교정

- `DamageData`는 `WeaponData`의 직접 child가 아니다. `ProjectileData.DefaultDamageData`가 direct owner다.
- HitScan/Laser도 실제 Actor를 생성하지 않는 **virtual ProjectileData**를 사용할 수 있으며 DamageData reference는 이 virtual ProjectileData가 소유한다.
- `Launcher`, `Heat`, `Charge`는 별도 child DataAsset이 아니라 현재 `WeaponData`의 inline authored config다.
- `Propulsion`, `MissileFlight`, `MissileGuidance`는 현재 `ProjectileData`의 inline authored config다.
- `AmmoData`는 finite ammo에서 필요한 별도 DataAsset이다. infinite/debug compatibility에서는 `AmmoTypeId` fallback을 유지할 수 있다.
- player-facing DisplayName은 `EquipmentPresetData`가 소유한다. Weapon Guide는 `SuggestedEquipmentDisplayName`을 completion Bundle에 넣어 다음 업무로 인계한다.

---

## 4. Product 기준선

Fresh persisted AssetDump 기준 `/Game/CarFight/Weapons/Data`는 exact11이었다.

```text
EquipmentPreset exact2
- HeavyCannon
- RocketLauncher

WeaponData exact2
- DA_ProtoTurretCannon
- DA_RocketLauncher

TurretMountData exact2
- DA_CannonBody
- DA_RocketBody

ProjectileData exact3
- DA_HeavyShell
- DA_PFX_ThrusterTest
- DA_Rocket_PropTest

DamageData exact2
- DA_DamageArmorPenTest
- DA_DamageAsset
```

Product AmmoData는 현재 `/Game/CarFight/Weapons/Data`에 존재하지 않았다. AmmoData exact2는 기존 Ammo Integration test namespace에서 확인됐다.

이 Feature의 자동 검증은 Product asset을 fixture로 수정하지 않는다.

---

## 5. Builder 공통 규약 적용

이 Guide는 `BuilderAuthoringStandard.md v1.0.0`을 첫 신규 적용 대상으로 사용한다.

강제 원칙:

```text
- Builder 분리 기준 = DataAsset 종류가 아니라 사용자 업무 단위
- 하위 DataAsset 때문에 다른 Builder를 연쇄 호출하지 않음
- 현재 Runtime이 지원하는 정상 무장 Capability를 하나의 Guide에서 제작 가능하게 함
- 한 Step = 하나의 작업 의도 또는 하나의 주요 질문
- Stable StepId 사용, fixed index semantic 금지
- Common Step + Capability 기반 Conditional Step
- 내부 ObjectPath / PackagePath / Stable ID를 일반 사용자 입력으로 요구하지 않음
- 기존 persistent writer / provider / durable authority 재사용
- Product mutation은 explicit Create/Apply 전 0
- multi-asset partial success를 숨기지 않음
- Final Review는 내부 graph보다 사용자 결과를 우선 표시
```

---

## 6. 사용자 Workflow — Stable Step Model

### 6.1 Stable StepId 전체 집합

Guide가 정의하는 semantic StepId는 다음 exact15다.

```text
IdentityTemplate
MountCompatibility
MountGeometry
FireBehavior
Projectile
Damage
Ammo
Launcher
Propulsion
MissileFlight
Guidance
Heat
Charge
ReviewCreate
Complete
```

이 exact15는 **항상 모두 보이는 UI Page 수가 아니다**.

`VisibleSteps`가 current Draft와 Capability를 기준으로 현재 사용자에게 필요한 Step만 순서대로 projection한다.

### 6.2 공통 Step

#### IdentityTemplate — 기본 정보 / 제작 방식

사용자가 결정:

```text
Asset 기본 이름
Equipment 표시 이름 제안
제작 의도/메모
Template
```

기본 Template 후보:

```text
직사 캐논 (Direct Fire Cannon)
자동화기 (Automatic Gun)
로켓 런처 (Rocket Launcher)
유도미사일 런처 (Guided Missile Launcher)
직접 구성 (Custom)
```

Template은 일반적인 Capability 조합과 초기값을 제안하는 UX 도구다. Runtime에서 가능한 조합을 Template 때문에 금지하지 않는다.

#### MountCompatibility — 장착 호환성

사용자가 결정:

```text
Required Mount Type
Required Weapon Size
```

`None`, Utility 등 무장으로 성립하지 않는 조합은 fail-visible한다.

#### MountGeometry — 터렛 / 장착 구조

```text
새 TurretMountData 생성
또는
기존 TurretMountData 재사용
```

새 생성 핵심 입력:

```text
Base / Yaw / Pitch mesh
Yaw / Pitch pivot socket
Primary / additional muzzle socket
Yaw / Pitch 범위
Yaw / Pitch 회전 속도
Turret mass
```

기존 자산 사용은 read-only reuse가 기본이며 Product 자산을 Guide가 암묵 수정하지 않는다.

#### FireBehavior — 발사 동작 / Capability 선택

핵심 입력:

```text
FireMode
FireRatePerMinute
MaxRange
SpreadDeg
Weapon mass
finite ammo 사용 여부
Launcher 고급 발사 기능 사용 여부
Heat 사용 여부
Charge 사용 여부
```

Projectile FireMode를 선택한 경우 physical Projectile 단계가 보인다.
HitScan/Laser 계열은 physical projectile movement 입력 Page를 숨기되, DamageData를 소유할 virtual ProjectileData는 내부 결과로 계속 생성/재사용한다.

### 6.3 항상 필요한 콘텐츠 Step

#### Damage — 피해 설정

Damage는 독립 Step으로 분리한다.

사용자 선택:

```text
새 DamageData 만들기
또는
기존 DamageData 사용
```

새 DamageData는 Guide 내부 UX에서 exact12 주요 authored field를 구성하고 기존 `CFDADamageProvider`의 typed authoring / reviewed durable authority를 재사용한다.

별도 Damage Builder를 연쇄 호출하지 않는다.

#### Projectile — 발사체 설정

physical Projectile이 필요한 경우에만 Visible Step으로 표시한다.

핵심 입력:

```text
Projectile Actor class
StaticMesh
InitialSpeed
LifeTime
Gravity
Collision radius / sweep
```

Propulsion / Missile Flight / Guidance는 별도 Conditional Step이므로 이 Page에 몰아넣지 않는다.

HitScan/Laser에서는 이 Page를 숨기고 safe virtual ProjectileData를 내부 생성해 Damage reference를 소유하게 한다.

### 6.4 Conditional Step

#### Ammo — 탄약

`bUseFiniteAmmo=true`일 때만 표시한다.

```text
새 AmmoData 만들기
또는
기존 AmmoData 사용
```

새 AmmoData는 Guide 내부 UX에서 exact8 authored field를 구성하고 기존 `CFDAAmmoProvider` typed authoring / reviewed durable authority를 재사용한다.

#### Launcher — 런처 / 다중 발사 패턴

Guide-only Capability flag `bUseLauncher`가 true일 때 표시한다.

이 flag는 Runtime schema에 새 persistent bool을 추가하지 않는다. 사용자에게 advanced launcher configuration Step을 보여줄지 결정하는 Editor-only workflow intent다.

저장 결과는 기존 `WeaponData.LauncherFirePatternConfig`와 `LauncherReleaseConfig`를 사용한다.

#### Propulsion — 자체 추진

`Projectile.PropulsionConfig.bUsePropulsion=true`일 때 표시한다.

핵심 입력:

```text
IgnitionDelaySeconds
BurnDurationSeconds
ThrustAccelerationCmPerSecSq
MaximumPropelledSpeed
```

#### MissileFlight — 미사일 비행

`Projectile.MissileFlightConfig.bUseMissileFlight=true`일 때 표시한다.

핵심 입력:

```text
AttackProfile
MinimumClearanceTimeSeconds
MinimumClearanceDistanceCm
TransitionDurationSeconds
Terminal phase 여부 / 시작 거리
```

#### Guidance — 유도

`Projectile.MissileGuideConfig.bUseGuidance=true`일 때 표시한다.

현재 persistent truth는 `CFProjectileData.MissileGuideConfig` inline config다.
`MissileGuidePresetData`가 존재한다는 이유만으로 존재하지 않는 direct reference를 가정하지 않는다.

핵심 Guided 입력:

```text
GuideMode
GuidanceLaw
GuidanceActivationMode
LostTargetPolicy
MaximumTurnRate
MaximumLateralAcceleration
Guidance response / minimum speed
Seeker model / observation model
필요한 activation / seeker / reacquisition 설정
```

고급 seeker tuning은 같은 Guidance Page의 접힌 Advanced section으로 둘 수 있지만 정상 Guided 제작 범위에서 제거하지 않는다.

#### Heat — 열

`Weapon.bUseHeat=true`일 때 표시한다.

```text
HeatPerShot
MaxHeat
HeatDissipationPerSecond
```

#### Charge — 충전 자원

`Weapon.bUseCharge=true`일 때 표시한다.

```text
MaximumWeaponCharge
InitialWeaponCharge
WeaponChargePerShot
WeaponChargeRecoveryPerSecond
```

### 6.5 Final Step

#### ReviewCreate — 최종 검토 / 생성

fresh current Draft와 선택 Asset truth를 전부 재검증한다.

사용자 우선 표시:

```text
무장 표시 이름 제안
Mount / Size
Fire 방식
활성 Capability
Projectile / Damage / Ammo 구성
생성될 새 Asset
재사용할 기존 Asset
Warning / Blocker
저장 영향 범위
```

명시적 `[무장 생성]` Action 전 Product mutation은 0이어야 한다.

#### Complete — 완료

완료 Bundle을 사용자에게 보여준다.

```text
SuggestedEquipmentDisplayName
RequiredMountType
RequiredWeaponSize
TurretMountDataPath
WeaponDataPath
ProjectileDataPath
DamageDataPath
AmmoDataPath 또는 AmmoTypeId fallback
DeterministicBundleFingerprint
```

`Equipment Builder 열기`는 navigation-only 기본 계약이다. 정확한 context injection은 CF-FQ-054 resume 시 별도 explicit Handoff Contract로 검토한다.

---

## 7. Visible Step projection 규칙

Visible Step은 current Draft에서 매번 deterministic하게 계산한다.

기본 순서:

```text
IdentityTemplate
MountCompatibility
MountGeometry
FireBehavior
[Projectile: physical Projectile일 때]
Damage
[Ammo: finite ammo일 때]
[Launcher: bUseLauncher]
[Propulsion: bUsePropulsion]
[MissileFlight: bUseMissileFlight]
[Guidance: bUseGuidance]
[Heat: bUseHeat]
[Charge: bUseCharge]
ReviewCreate
Complete
```

현재 Step이 capability 변경으로 projection에서 제거되면 다음 규칙을 사용한다.

```text
- 현재 의미가 계속 유효한 가장 가까운 이전/다음 visible Step으로 안전하게 이동
- 제거된 Step의 값을 persistent truth처럼 유지하지 않음
- 다시 활성화했을 때 transient draft를 복구할지는 해당 field의 semantic 동일성 기준으로 판단
- 의미가 달라진 downstream receipt/validation은 stale 처리
```

UI 숫자는 `현재 visible index / visible total`로 표시할 수 있지만 semantic authority는 Stable StepId다.

---

## 8. Template 기본안

Template은 runtime hard class가 아니다.

### Direct Fire Cannon

```text
FireMode = Projectile
Launcher advanced = false
Finite Ammo = optional/default false
Propulsion = false
Missile Flight = false
Guidance = false
Heat = false
Charge = false
```

### Automatic Gun

```text
FireMode = Projectile
higher FireRate proposal
Launcher advanced = false
Finite Ammo = optional/default true candidate
Propulsion = false
Missile Flight = false
Guidance = false
```

### Rocket Launcher

```text
FireMode = Projectile
Launcher advanced = true
Propulsion = true
Missile Flight = false
Guidance = false
```

### Guided Missile Launcher

```text
FireMode = Projectile
Launcher advanced = true
Propulsion = true
Missile Flight = true
Guidance = true
GuideMode default candidate = TargetActor
```

### Custom

현재 Capability intent를 사용자가 직접 구성한다.

Template 변경은 이미 입력한 값을 조용히 덮어쓰지 않는다. UI 단계에서 변경 영향 Preview/confirmation을 제공한다.

---

## 9. Persistent Authority / Writer 경계

### 9.1 Guide local exact3 Create authority

현재 별도 typed DataAuthoring provider가 없는 다음 exact3은 Guide-local typed payload/materializer/extractor/fingerprint를 사용하되 existing `CFDADurableCore::ApplyTypedTarget` sequencing을 재사용한다.

```text
CFTurretMountData
CFProjectileData
CFWeaponData
```

범위는 P0에서 **Create-only**다.
기존 Product exact3 자산을 선택해 Update하지 않는다.

### 9.2 Existing provider exact2 reuse

```text
CFAmmoData
CFDamageData
```

새 Ammo/Damage를 Guide에서 만들더라도 자체 두 번째 writer를 만들지 않는다.

Guide Draft를 provider payload로 변환한 뒤 existing provider semantic fingerprint/serializer/reviewed durable mutation authority를 재사용한다.

```text
CFDAAmmoProvider / CFDAAmmoProviderImpl
CFDADamageProvider / CFDADamageProviderImpl
```

Provider 전체 JSON/Staging UI를 사용자에게 노출할 필요는 없다. Guide가 typed payload를 만들고 provider contract를 호출한다.

### 9.3 EquipmentPreset

`CFEquipmentPresetData`는 이 Guide가 생성/수정하지 않는다.
Equipment Builder의 기존 authority를 보존한다.

---

## 10. Reference 연결 규칙

최종 신규 무장 생성 시 연결은 다음 순서를 만족해야 한다.

```text
DamageData durable ready
AmmoData durable ready (finite일 때)
TurretMountData durable ready
ProjectileData durable ready + DefaultDamageData exact
WeaponData durable ready
  + DefaultProjectileData exact
  + finite일 때 DefaultAmmoData exact
```

실제 mutation 전에 가능한 모든 target path collision과 selected existing child validity를 preflight한다.

### Existing Weapon 재사용

기존 WeaponData는 Guide가 Update하지 않는다.

따라서 existing Weapon을 재사용하는 경우 다음은 exact consistency를 요구한다.

```text
Guide selected Projectile == existing Weapon.DefaultProjectileData
finite이면 Guide selected Ammo == existing Weapon.DefaultAmmoData
Guide FireMode / Size / Mount compatibility == persisted Weapon truth
Projectile selected Damage == persisted Projectile.DefaultDamageData
```

사용자가 existing Weapon을 선택한 뒤 다른 Projectile/Ammo/Damage를 선택하면 `기존 Weapon을 수정해 주는 것`으로 오해하지 않도록 fail-closed한다.

---

## 11. Naming / Identity

일반 사용자는 `BaseAssetName`과 표시 이름 제안만 입력한다.

Guide가 deterministic identity를 만든다.

```text
Turret:
/Game/CarFight/Weapons/Data/TurretMounts/DA_<Base>_Mount.DA_<Base>_Mount

Weapon:
/Game/CarFight/Weapons/Data/WeaponDefs/DA_<Base>_Weapon.DA_<Base>_Weapon

Projectile:
/Game/CarFight/Weapons/Data/ProjectileDefs/DA_<Base>_Projectile.DA_<Base>_Projectile

Damage:
/Game/CarFight/Weapons/Data/DamageDefs/DA_<Base>_Damage.DA_<Base>_Damage

Ammo:
/Game/CarFight/Weapons/Data/AmmoDefs/DA_<Base>_Ammo.DA_<Base>_Ammo
```

실제 Damage/Ammo Product root가 프로젝트 규약과 충돌하는 fresh evidence가 발견되면 구현 전에 Plan을 교정한다.

규칙:

```text
silent sanitize 금지
auto suffix 금지
overwrite 금지
collision fail-visible
고급 path override는 P0 기본 범위 밖
```

---

## 12. Multi-Asset Create / Partial Failure

최종 Create는 완전한 원자 transaction이라고 가장하지 않는다.

기본 sequencing:

```text
0. 모든 target / existing reference / capability 전체 preflight
1. Damage create/reuse 확정
2. finite Ammo create/reuse 확정
3. Turret create/reuse 확정
4. Projectile create/reuse 확정
5. Weapon create/reuse 확정
6. exact durable readback
7. Result Bundle 생성
```

중간 실패 시 이미 DurableApplied된 새 child를 자동 삭제/rollback하지 않는다.

같은 Guide session은 session-created exact path를 기억하고 안전한 retry에서 재사용한다.

사용자에게 다음을 구분해서 보여준다.

```text
이미 생성 완료
아직 생성되지 않음
실패한 단계
다음 재시도 범위
```

blind retry와 Product Delete는 사용하지 않는다.

---

## 13. Validation 계약

### IdentityTemplate

```text
BaseAssetName valid
SuggestedDisplayName non-empty
naming path valid
```

### MountCompatibility

```text
weapon-compatible MountType
WeaponSize != None
```

### MountGeometry

```text
reuse = clean persisted exact type
create = muzzle/range/rate/mass validity + target absent
```

### FireBehavior

```text
finite numeric fire values
native UCFWeaponData contract probe
SupportsMountType
WeaponSize exact
```

### Projectile

physical projectile일 때:

```text
Actor class required
speed/lifetime/collision finite
reuse type/clean persisted
create target absent
```

HitScan/Laser virtual projectile는 movement UI를 요구하지 않되 내부 target collision과 Damage 연결 가능성은 full validation에서 확인한다.

### Damage

```text
reuse exact clean UCFDamageData
or new provider payload valid
existing Projectile reuse 시 persisted DefaultDamageData exact consistency
```

### Ammo

finite일 때:

```text
reuse exact clean UCFAmmoData + IsAmmoDataValid
or new provider payload valid
existing Weapon reuse 시 persisted DefaultAmmoData exact consistency
```

### Launcher

`bUseLauncher=true`일 때 launcher count/release numeric 의미를 검증한다.

### Propulsion

`bUsePropulsion=true`일 때:

```text
IgnitionDelay >= 0 finite
BurnDuration >= 0 finite
ThrustAcceleration >= 0 finite
MaximumPropelledSpeed > 0 finite
```

### MissileFlight

`bUseMissileFlight=true`일 때 raw authored values가 runtime clamp에 의존하지 않고 valid 범위인지 fail-visible한다.

### Guidance

`bUseGuidance=true`일 때:

```text
GuideMode != None
GuidanceLaw valid
Activation valid
turn/lateral/response/min speed finite
seeker/observation/reacquisition 조건부 값 valid
```

### Heat / Charge

기존 Runtime numeric contract를 그대로 검증한다.

### ReviewCreate

모든 visible Step + 내부 virtual projectile + final reference graph를 fresh 재검증한다.

---

## 14. Editor UX 구조

Vehicle Builder와 공통 규약을 따른다.

```text
고정 USER Header
→ Step Navigation
→ 현재 Page 단일 main vertical Scroll
→ 현재 Step 핵심 Action
→ 이전 / 현재 상태 다시 확인 / 다음 / Advanced
```

기본 정보 계층:

```text
Level 0 = 목적 / 상태 / 지금 할 일 / blocker
Level 1 = 실제 Asset / 값 / 단위 / 게임 영향
Level 2 = object path / fingerprint / raw diagnostic
```

Advanced는 고급 값과 진단을 위한 접힌 영역이며 normal creation의 누락 기능을 대신하지 않는다.

---

## 15. 구현 구조

Editor-only C++:

```text
Public/WeaponAuthoring/CFWeaponGuideTypes.h
Public/WeaponAuthoring/CFWeaponGuideVM.h
Public/WeaponAuthoring/CFWeaponGuideTab.h

Private/WeaponAuthoring/CFWeaponGuideVM.cpp
Private/WeaponAuthoring/CFWeaponGuideTab.cpp
Private/WeaponAuthoring/CFWeaponGuideTests.cpp
```

새 파일명/클래스명은 32자 제한을 지킨다.

Blueprint Runtime schema를 이 Guide 때문에 추가하지 않는다.

### C++ 책임

```text
Stable Step projection
Draft state
Template proposal
validation
naming
persistent authority orchestration
durable retry/recovery
completion bundle
Slate shell
```

### Blueprint 책임

이 Feature의 Editor workflow authority는 기본적으로 C++가 소유한다.
Runtime 콘텐츠 표현이나 실제 발사체 Blueprint asset 제작 자체는 기존 Blueprint/Asset 책임을 유지한다.

---

## 16. 현재 Prototype 교정 범위

v1.0.0 Prototype은 다음 기반을 재사용할 수 있다.

```text
exact3 local durable create seam
naming helpers
existing child clean/persisted validation
partial durable session recovery
completion fingerprint
```

다음은 v0.2.0 계약에 맞춰 교정한다.

```text
fixed exact8 enum/navigation → Stable StepId + VisibleSteps
PayloadChain → Projectile / Damage / Ammo 분리
OptionalFeatures → Launcher / Propulsion / MissileFlight / Guidance / Heat / Charge 분리
Projectile payload → PropulsionConfig + MissileFlightConfig + MissileGuideConfig 보존
existing Weapon child consistency guard 추가
bUseLauncher Editor workflow intent 추가
Template intent 추가
Damage/Ammo create/reuse Draft + existing provider orchestration 추가
```

---

## 17. 단계 계획

### WEA-P0-00 — Fresh Rebaseline / Initial Contract Freeze

상태: Historical PASS

초기 schema/asset/authority audit를 완료했다.

### WEA-P0-00R — Builder Standard Rebaseline / Contract Correction

상태: PASS

완료:

```text
BuilderAuthoringStandard v1.0.0 적용
fixed8 폐기 결정
Runtime Propulsion/MissileFlight/Guidance 재감사
Ammo/Damage existing typed provider 확인
Stable Step/Conditional Step 계약 freeze
```

### WEA-P0-01 — Core Workflow / State / Naming Correction

상태: **Technical PASS — 2026-09-18**

구현 완료:

```text
Stable StepId exact15
VisibleSteps deterministic projection
Template intent exact5
Conditional capability topology
기존 naming/collision guard 보존
Projectile payload에 Propulsion/MissileFlight/Guidance durable roundtrip 추가
existing Weapon↔Projectile/Ammo, Projectile↔Damage exact consistency guard
partial durable 이후 BaseName/Template 변경 차단
Damage/Ammo 신규 Create는 existing provider 연결 전 fail-visible
```

Fresh evidence:

```text
Official UE 5.8 Build:
- b735382d4dd140be937d1f0cc722b307 PASS

Focused Automation compile Build:
- 25dad746a4334e739ce06fc3d5c759de PASS

WEA-P0-01 exact5 same-process Automation:
- process 62a7eb4246e54d4d82543526beb8396c
- VisibleSteps PASS
- TemplateTopology PASS
- Naming PASS
- CapabilityValidation PASS
- ProviderBoundary PASS
- success 5 / failure 0 / missing 0 / unexpected 0 / duplicate 0

Fresh persisted Product evidence:
- /Game/CarFight/Weapons/Data exact11 / failed0
- dataset adset_v1_e9fce8adc116043287d91cc99eea0a6a.97c4262e1fd83c9a12f7bde3
- dataset fingerprint 41efdc643f815bec2d7a52fe44151ad85d4236e3075477b3f8f931f321af66ba
- /Game/CarFight/Tests/WeaponGuide exact0
- Git status에 UE/Content/CarFight/Weapons/Data mutation 0
```

판정:

```text
P0 = 0
blocking P1 = 0
Verdict = PASS
Next = WEA-P0-02 Guided Draft UX / Child Authoring
```

### WEA-P0-02 — Guided Draft UX / Child Authoring

상태: **Technical PASS — 2026-09-18**

구현 완료:

```text
CarFight.WeaponGuide 독립 Native Slate Nomad Tab
Stable Step rail + 현재 Page 단일 main Scroll
Identity / Mount / Fire / Projectile / Damage / Ammo 분리 Page
Launcher / Propulsion / MissileFlight / Guidance / Heat / Charge conditional Page
기존 Turret/Weapon/Projectile/Damage/Ammo persisted asset read-only reuse picker
Damage CreateNew Draft → existing CFDADamageProvider ReviewedMutationReady mapping
Ammo CreateNew Draft → existing CFDAAmmoProvider ReviewedMutationReady mapping
Damage/Ammo typed payload → semantic fingerprint → serializer → parser mutation0 round-trip
Damage radial normal authoring fields + Ammo Texture2D Icon normal authoring
Review Page에서 provider preview와 validation 사용자 요약
WEA-P0-02 durable entry는 신규 Damage/Ammo 포함 시 WEA-P0-03 boundary로 mutation 전에 fail-visible
CF-FQ-054 Equipment Builder state/business logic mutation 0
```

Fresh evidence:

```text
Final Official UE 5.8 Build:
- build b5d8649e8c5448df9d76371bfb2955c8 PASS
- CFWeaponGuideTests.cpp / CFWeaponGuideTab.cpp fresh compile + CarFight_ReEditor DLL link PASS

Final WEA exact5 same-process Automation:
- process 102ed9a51f1e4b1383f90dad59fd3bb1
- CapabilityValidation PASS
- Naming PASS
- TemplateTopology PASS
- VisibleSteps PASS
- ProviderDraftMapping PASS
- success 5 / failure 0 / missing 0 / unexpected 0 / duplicate 0
- ProviderDraftMapping은 non-null Ammo Texture2D Icon과 Damage radial non-default field를 포함

Fresh persisted Product evidence:
- /Game/CarFight/Weapons/Data exact11 / failed0
- dataset adset_v1_0fb4b4d61649489a8cecf347bedc517a.f7d5b3266202b69b7ca79fbb
- dataset fingerprint e98e3f9acf76b7d5dfa1d68e19d5cca4fa3435bd1d46a31a1a318876003e76e4
- /Game/CarFight/Tests/WeaponGuide exact0 / failed0
- Git status에 UE/Content/CarFight/Weapons/Data mutation 0

Correction evidence:
- 최초 P0-02 exact5에서 ProviderDraftMapping 마지막 P0-03 boundary assertion 1건 실패
- 원인: durable entry가 sibling incomplete Step validation을 먼저 반환
- 교정: 신규 Damage/Ammo durable boundary를 전체 validation보다 먼저 fail-visible
- 교정 후 Official Build PASS + exact5 5/5 PASS
```

판정:

```text
P0 = 0
blocking P1 = 0
Product weapon asset mutation = 0
fixture residue = 0
Verdict = PASS
Next = WEA-P0-03 Multi-Asset Durable Create / Recovery
CF-FQ-054 = Paused 유지
```

### WEA-P0-03 — Multi-Asset Durable Create / Recovery

상태: **Technical PASS — 2026-09-18**

구현 완료:

```text
explicit [무장 생성] Action 전 Product mutation 0
Damage CreateNew → existing CFDADamageProvider operation table → shared CFDADurableCore
finite Ammo CreateNew → existing CFDAAmmoProvider operation table → shared CFDADurableCore
Turret / Projectile / Weapon exact3 → Guide-local typed payload + shared CFDADurableCore
전체 순서 = Damage → Ammo → Turret → Projectile → Weapon
mutation 전 fresh ValidateAll + provider Create Preview + target absence preflight
partial durable 성공 child를 session exact path로 기억하고 자동 rollback/delete 금지
same-session retry에서 Damage/Ammo provider semantic fingerprint 재검증
same-session retry에서 Turret/Projectile/Weapon exact3 persisted semantic fingerprint 재검증
stale Draft/reference가 durable child와 달라지면 fail-closed
최종 persisted graph readback:
- Projectile.DefaultDamageData + DamageProfileId exact
- Weapon.DefaultProjectileData + ProjectileDataId exact
- finite Weapon.DefaultAmmoData + AmmoTypeId exact
graph readback 통과 뒤에만 completion Bundle + deterministic fingerprint 확정
Complete Page에서 exact child path + Bundle fingerprint 사용자 표시
EquipmentPreset mutation 0 / CF-FQ-054 lifecycle mutation 0
```

Fresh evidence:

```text
Final Official UE 5.8 Build:
- build 28196840a5214813a82d54931f69866d PASS
- CFWeaponGuideVM.cpp / CFWeaponGuideTests.cpp fresh compile
- CarFight_ReEditor DLL link PASS

Final WEA exact7 same-process Automation:
- process 895e656c48164d639ac326d8e6f50c75
- P001.VisibleSteps PASS
- P001.TemplateTopology PASS
- P001.Naming PASS
- P001.CapabilityValidation PASS
- P002.ProviderDraftMapping PASS
- P003.DurableCreateGraph PASS
- P003.PartialRecovery PASS
- success 7 / failure 0 / missing 0 / unexpected 0 / duplicate 0

P003.DurableCreateGraph:
- disposable exact5 Damage/Ammo/Turret/Projectile/Weapon durable Create
- persisted Projectile→Damage / Weapon→Projectile / Weapon→Ammo exact readback
- completion Bundle deterministic fingerprint 확정
- teardown residue exact0

P003.PartialRecovery:
- Damage + Ammo + Turret confirmed durable 직후 deterministic interruption
- 이미 저장한 Turret과 다른 Draft를 의도적으로 만들면 stale retry 차단
- 원래 Draft로 복원 후 같은 ViewModel에서 confirmed child 재사용
- 남은 Projectile / Weapon 생성 + final graph readback 완료
- teardown residue exact0

Fresh persisted Product evidence:
- /Game/CarFight/Weapons/Data exact11 / failed0
- dataset adset_v1_4e033c724c92dc98c229182b4a83a51a.1da953f3e0e3854c11743cf1
- dataset fingerprint 35fb3c7a1d9edf177cd911eae671ed0f9ae8b93b94e6beff78bc7d3dbb7a39bf
- /Game/CarFight/Tests/WeaponGuide exact0 / failed0
- Git status: UE/Content/CarFight/Weapons/Data mutation 0
- Git status: UE/Content/CarFight/Tests/WeaponGuide residue/mutation 0
```

Post-Implementation Review correction:

```text
발견:
- 최초 P0-03 구현은 session-created exact3(Turret/Projectile/Weapon) retry에서 path/type/clean만 재검증
- partial durable 이후 사용자가 뒤로 이동해 Draft 값을 바꾸면 stale exact3를 재사용할 가능성 존재

교정:
- exact3도 current Draft + resolved child reference로 desired typed fingerprint 재계산
- persisted session-created exact3 fingerprint와 exact 비교
- mismatch는 mutation 전에 fail-closed
- PartialRecovery 테스트를 Turret durable 이후 Draft 변조까지 검증하도록 강화
- 교정 후 Official Build PASS + exact7 7/7 PASS
```

판정:

```text
P0 = 0
blocking P1 = 0
Product weapon asset mutation = 0
fixture residue = 0
CF-FQ-054 = Paused 유지
Verdict = PASS
Next = WEA-P0-04 Completion Bundle / Equipment Builder Integration Contract
```

### WEA-P0-04 — Completion Bundle / Equipment Builder Integration Contract

상태: **Technical PASS — 2026-09-18**

구현 완료:

```text
P0-03 FCFWeaponGuideResultBundle exact 의미와 DeterministicBundleFingerprint 보존
completion summary/handoff 직전 ResultBundle.bComplete fail-closed
현재 Draft/reference truth로 completion Bundle fresh 재계산
fresh DeterministicBundleFingerprint == P0-03 확정 fingerprint exact 검증
persisted exact reference graph 재검증
USER completion summary:
- SuggestedEquipmentDisplayName
- Required MountType / WeaponSize
- FireMode
- 활성 Launcher / Propulsion / MissileFlight / Guidance / Heat / Charge
- Turret / Damage / Ammo / Projectile / Weapon 신규 생성·기존 재사용 의미
- Equipment Builder 다음 작업 안내
기술 세부 영역:
- exact child object path
- DeterministicBundleFingerprint
FCFWeaponGuideEquipmentHandoff navigation-only 계약:
- TargetTabId = CarFight.EquipmentBuilder
- SourceBundleFingerprint = completion Bundle fingerprint
- bNavigationOnly = true
- bInjectContext = false
Complete Page에 [장비 제작 가이드 열기 (이동만)] Action 추가
Action은 FGlobalTabmanager::TryInvokeTab(TargetTabId)만 실행
Equipment Builder VM/Draft/context API 호출 0
WeaponAuthoring → EquipmentAuthoring/CFEquipmentBuilder include 0
P0-03 DeterministicBundleFingerprint는 그대로 보존하고, 별도 P0-04 CompletionSemanticFingerprint가 persisted exact5 + FireMode/Capability/child mode 의미 변경을 stale handoff로 차단
CF-FQ-054 source/business state/lifecycle mutation 0
```

Fresh evidence:

```text
Final Official UE 5.8 Build:
- build d393cb0ec5d246d288deb99594b2b328 PASS
- updated CFWeaponGuideTests.cpp fresh compile
- CarFight_ReEditor DLL link PASS

Final WEA exact8 same-process Automation:
- process 68bdfbb06d5c46dfb04a5c60ec78ff28
- P001.VisibleSteps PASS
- P001.TemplateTopology PASS
- P001.Naming PASS
- P001.CapabilityValidation PASS
- P002.ProviderDraftMapping PASS
- P003.DurableCreateGraph PASS
- P003.PartialRecovery PASS
- P004.CompletionHandoff PASS
- success 8 / failure 0 / missing 0 / unexpected 0 / duplicate 0

P004.CompletionHandoff:
- completion 전 summary/handoff fail-closed
- CarFight.EquipmentBuilder Nomad Tab spawner 등록 확인
- exact5 durable completion 뒤 USER summary 생성
- navigation-only=true / context injection=false exact
- SourceBundleFingerprint == P0-03 DeterministicBundleFingerprint exact
- summary/handoff 생성 전후 completion fingerprint 불변
- 완료 뒤 Draft 변조 시 stale fingerprint mismatch 차단
- Draft 복원 후 navigation-only handoff 재성립
- teardown fixture residue exact0

Fresh persisted Product evidence:
- /Game/CarFight/Weapons/Data exact11 / failed0
- dataset adset_v1_2e6238cc95d32f4d9f024c020d2503bf.590deeae22bf3548b3496fad
- dataset fingerprint 3133996c4b106a120ddf576ff70d2b88b8ca206e9b7f7f892c26d15cccbcf714
- /Game/CarFight/Tests/WeaponGuide exact0 / failed0
- Git status: UE/Content/CarFight/Weapons/Data mutation 0
- Git status: UE/Content/CarFight/Tests/WeaponGuide residue/mutation 0
```

Post-Implementation Review:

```text
P0 = 0
blocking P1 = 0
WeaponAuthoring → EquipmentAuthoring business coupling = 0
context injection = false
Product weapon asset mutation = 0
fixture residue = 0
CF-FQ-054 = Paused 유지
Verdict = PASS
Next = WEA-P0-05 Final Technical + USER Acceptance
```

#### WEA-P0-04 Mid-review Correction + Re-review — 2026-09-19

Mid-review 최초 판정:

```text
P0 = 0
blocking P1 = 1
P2 = 2
Verdict = HOLD / Correction Required
```

발견 사항:

```text
P1:
- P0-03 DeterministicBundleFingerprint는 DisplayName/Mount/Size/child path/AmmoTypeId 중심이어서
  USER completion summary가 표시하는 FireMode / Launcher / Propulsion / MissileFlight / Guidance / Heat / Charge 전체 의미를 보호하지 못했다.
- ValidateDurableReferenceGraph도 reference/logical ID 중심이어서 persisted child semantic 전체 변경을 다시 고정하지 못했다.

P2-1:
- Complete Page button IsEnabled가 BuildEquipmentBuilderHandoff를 반복 호출해 asset load/reference validation을 UI attribute 평가마다 수행할 수 있었다.

P2-2:
- completion integrity 실패 시에도 이전 ResultBundle path/fingerprint 기술 세부 정보가 계속 표시돼 stale 확정값처럼 보일 수 있었다.
```

교정:

```text
- P0-03 DeterministicBundleFingerprint contract는 변경하지 않았다.
- 별도 session-local CompletionSemanticFingerprint를 추가했다.
- CompletionSemanticFingerprint 입력:
  - P0-03 DeterministicBundleFingerprint
  - persisted Turret/Damage/Ammo/Projectile/Weapon typed semantic fingerprint
  - child CreateNew/ReuseExisting mode
  - FireMode / finite ammo
  - Launcher / Propulsion / MissileFlight / Guidance / Heat / Charge 활성 의미
- CreateNew child는 current Draft desired typed semantic fingerprint와 persisted typed fingerprint exact 일치를 요구한다.
- summary/handoff 직전 CompletionSemanticFingerprint를 fresh 재계산하고 completion 당시 snapshot과 exact 비교한다.
- Existing WeaponData reuse 시 persisted Launcher config와 Heat/Charge authored 값을 Draft projection에 동기화한다.
- Complete Page 진입 시 authoritative summary/integrity 검증 결과를 bCompletionHandoffReady에 1회 cache한다.
- button IsEnabled는 bCompletionHandoffReady만 읽고, 실제 click에서는 BuildEquipmentBuilderHandoff authoritative validation을 다시 수행한다.
- integrity 실패 시 이전 exact path/fingerprint 기술 세부 정보를 숨긴다.
```

강화된 P004 회귀:

```text
- stale SuggestedDisplayName 차단
- stale FireMode 차단
- stale Launcher intent 차단
- stale Guidance capability 차단
- 위 semantic stale 검증 중에도 P0-03 DeterministicBundleFingerprint 값 자체는 불변
- 원래 semantic 복원 후 navigation-only handoff 재성립
```

Fresh correction evidence:

```text
Final Official UE 5.8 Build:
- d424ec35a246459f92133f72f3b9e7f8 PASS

Final exact8 same-process Automation:
- db8df53581d4469c9e04377bc2d356f7
- success 8 / failure 0 / missing 0 / unexpected 0 / duplicate 0
- P004.CompletionHandoff PASS including FireMode / Launcher / Guidance stale semantic regression

Fresh Product AssetDump:
- /Game/CarFight/Weapons/Data exact11 / failed0
- dataset adset_v1_7c8efa5e133f7770fffad2f3e5943d20.5fd25c0b93b01d55ba212ad6
- dataset fingerprint 7c01478f9bf2a6830409dcf9a26a4ffa4992046f9a8a82c9102bb73cac8c03db
- selection fingerprint 55ea7d15ae77fdf8efddf8a915143d61921fe37151fd0f51534fa958d5dd33f3

Fresh fixture:
- /Game/CarFight/Tests/WeaponGuide exact0 / failed0
- Product Weapon asset Git mutation 0
- WeaponGuide fixture Git mutation/residue 0
```

Correction Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
P0-03 DeterministicBundleFingerprint contract mutation = 0
P0-04 completion semantic fail-closed = PASS
navigation-only / context injection false = PASS
Product weapon asset mutation = 0
fixture residue = 0
CF-FQ-054 = Paused 유지
Verdict = PASS / RE-ACCEPTED
Next = WEA-P0-05 Final Technical + USER Acceptance
```

### WEA-P0-05 — Final Technical + USER Acceptance

목표:

```text
Official Build
focused WeaponGuide automation
affected authoring regression
Product mutation0
fixture residue exact0
대표 Cannon / Rocket / Guided Missile / HitScan workflow USER Acceptance
```

Final Technical 결과:

```text
Official UE 5.8 Build = PASS
Build Job = 8599723f22c243af9ff0a29c0430163e
WeaponGuide focused = exact8 / 8 PASS
WeaponGuide Process = 1d3581a0911b4ffca51a3fd73a6d8dac
Ammo affected regression = exact17 / 17 PASS
Ammo Process = 6f6fd8f79c8345c3a35442f76630d288
Damage affected regression = exact14 / 14 PASS
Damage Process = 290442c973684084ac05dcc01257ff1a
Bounded affected total = exact39 / 39 PASS
Product /Game/CarFight/Weapons/Data = exact11 / failed0
Product dataset = adset_v1_71742f0133da8471dcf7da92b79cc766.d22d9e6d5da1c28e0f076b24
Product dataset fingerprint = a56ad7944e284558d0a605f0f206e7e2c86927f31a46251b95f5db0204203748
Fixture /Game/CarFight/Tests/WeaponGuide = exact0 / failed0
Product + fixture Git diff = 0
Final implementation re-review = P0 0 / blocking P1 0
Technical Verdict = PASS
USER Acceptance = Pending
CF-FQ-054 = Paused 유지
```

USER Acceptance는 Automation이나 코드 검증으로 대체하지 않는다. 실제 Editor에서 직사 캐논 / 비유도 로켓 런처 / TargetActor 유도미사일 런처 / HitScan 또는 virtual ProjectileData 4종을 USER가 확인하기 전 CF-FQ-055를 Complete로 종료하지 않는다.

---

## 18. 자동화 검증 계획

최소 축:

```text
VisibleSteps common-only projection
finite Ammo conditional insertion
Launcher conditional insertion
Propulsion conditional insertion
MissileFlight conditional insertion
Guidance conditional insertion
Heat conditional insertion
Charge conditional insertion
current Step removal recovery
Template suggestion does not become hard runtime restriction
naming deterministic
invalid name fail-visible
collision fail-visible
existing child exact type/clean persisted
existing Weapon↔Projectile exact mismatch blocked
existing Projectile↔Damage exact mismatch blocked
finite existing Weapon↔Ammo exact mismatch blocked
Projectile payload Propulsion roundtrip
Projectile payload MissileFlight roundtrip
Projectile payload MissileGuide roundtrip
Damage provider new/reuse
Ammo provider new/reuse
multi-asset partial durable retry
completion bundle deterministic
Product mutation0
fixture residue exact0
```

---

## 19. 대표 USER Acceptance

Technical PASS와 별도로 최소 다음 실제 제작 흐름을 사용자 확인한다.

```text
1. 직사 캐논
2. 비유도 로켓 런처
3. TargetActor 유도미사일 런처
4. HitScan 또는 virtual ProjectileData 경로
```

확인 항목:

```text
한 Page에 너무 많은 판단이 몰리지 않는가
조건부 Step이 자연스럽게 나타나고 사라지는가
내부 ID/Path를 몰라도 제작 가능한가
새 child와 기존 child 재사용의 의미가 분명한가
최종 생성 범위를 이해할 수 있는가
Equipment Builder로 넘어갈 때 업무 전환이 자연스러운가
```

---

## 20. 보호 범위

다음은 명시적 별도 범위가 열리기 전 변경하지 않는다.

```text
GoPyMCP 본체
기존 Product weapon assets
기존 EquipmentPreset durable authority
CF-FQ-054 accepted technical evidence
CF-FQ-054 lifecycle resume
기존 Ammo/Damage provider semantic contract
기존 Missile runtime gameplay semantics
```

자동 테스트는 disposable namespace만 사용한다.

---

## 21. Changelog

### v0.2.7 - 2026-09-29

- USER Acceptance 중 개별 무기 수치 입력 중심 Guide보다 전체 무장 Family/Variant/라인업을 AI와 대량 운영하는 구조가 핵심 요구임을 확정했다.
- WEA-P0-05 Final Technical PASS와 exact39/39, Product/fixture mutation0 등 기존 technical evidence는 그대로 보존한다.
- standalone USER Acceptance는 Paused로 전환하고 Weapon Guide의 typed validation / durable create / partial recovery / existing provider reuse를 `CF-FQ-058 CarFight Content Catalog & Authoring System`의 Weapon backend reuse input으로 재분류했다.
- WEA UX 자체를 추가 polish하지 않으며, CCAS 구현 뒤 fallback/advanced direct authoring 필요성을 다시 판단한다.
- 현재 Product DA authority나 Runtime 계약은 이 문서 전환만으로 변경하지 않는다.

### v0.2.6 - 2026-09-19

- WEA-P0-05 Final Technical을 fresh restore 기준으로 수행했다. CF-FQ-055 canonical DAG unique head exact1은 `ver_5e9c8a12d4f7481ba73c260fd5e61490`, CF-FQ-054 canonical head는 `ver_6059f52cab81416a97557b33671ecdc1`이며 Paused 상태를 재확인했다.
- Official UE 5.8 Build job `8599723f22c243af9ff0a29c0430163e`가 exit code 0으로 PASS했다.
- WeaponGuide exact8 process `1d3581a0911b4ffca51a3fd73a6d8dac`, Ammo affected exact17 process `6f6fd8f79c8345c3a35442f76630d288`, Damage affected exact14 process `290442c973684084ac05dcc01257ff1a`가 모두 failure/missing/unexpected/duplicate 0으로 총 exact39/39 PASS했다.
- fresh AssetDump에서 Product `/Game/CarFight/Weapons/Data` exact11 / failed0, fixture `/Game/CarFight/Tests/WeaponGuide` exact0 / failed0을 확인했고 두 경로 Git diff는 0이었다.
- final implementation re-review에서 CompletionSemanticFingerprint fresh revalidation, Complete Page cache-only IsEnabled, click-time authoritative handoff validation, navigation-only Equipment Builder boundary와 direct API coupling 0을 재확인했다.
- Final Technical 판정은 `P0 0 / blocking P1 0 / PASS`다. USER Acceptance는 실제 Editor 확인 전까지 Pending이며 CF-FQ-055는 WEA-P0-05에 유지한다.
- CF-FQ-054는 explicit USER resume 전까지 Paused를 유지하며 context injection / Draft autofill / EquipmentPreset Apply/Save를 수행하지 않는다.

### v0.2.5 - 2026-09-19

- WEA-P0-04 중간검수에서 completion USER summary semantic이 P0-03 Bundle fingerprint 범위를 넘어서는 P1 1건과 UI revalidation/stale detail P2 2건을 발견했다.
- P0-03 `DeterministicBundleFingerprint`를 변경하지 않고 별도 `CompletionSemanticFingerprint`를 추가해 persisted exact5 fingerprint와 FireMode/Launcher/Propulsion/MissileFlight/Guidance/Heat/Charge/child mode 의미를 고정했다.
- CreateNew child는 current Draft desired typed fingerprint와 persisted typed fingerprint exact 일치를 요구하고 summary/handoff 직전 semantic fingerprint를 fresh 재계산한다.
- Complete Page button의 `IsEnabled`는 page-entry validation cache만 읽고 click 직전 authoritative handoff validation을 다시 수행하도록 교정했다.
- integrity 실패 시 이전 exact path/fingerprint 기술 세부 정보를 숨기도록 교정했다.
- 강화된 P004에서 stale FireMode/Launcher/Guidance 차단과 P0-03 Bundle fingerprint 불변을 검증했다.
- Final Official UE 5.8 Build `d424ec35a246459f92133f72f3b9e7f8` PASS와 exact8 process `db8df53581d4469c9e04377bc2d356f7` 8/8 PASS를 확정했다.
- fresh Product Weapons/Data exact11 / failed0, WeaponGuide fixture exact0 / failed0, Product/fixture Git mutation0을 확인했다.
- Correction Re-review `P0 0 / blocking P1 0 / P2 0`, Verdict `PASS / RE-ACCEPTED`. WEA-P0-05 Ready를 유지하고 CF-FQ-054는 Paused 상태를 유지한다.

### v0.2.4 - 2026-09-18

- WEA-P0-04에서 P0-03 completion Bundle과 DeterministicBundleFingerprint를 다시 계산·검증하는 fail-closed integrity gate를 추가했다.
- USER completion summary에 표시 이름 제안, 장착 요구, FireMode, 활성 Capability, child 생성/재사용 의미와 다음 작업을 사용자 우선으로 표시했다.
- `FCFWeaponGuideEquipmentHandoff`를 navigation-only 계약으로 추가하고 `TargetTabId=CarFight.EquipmentBuilder`, `bNavigationOnly=true`, `bInjectContext=false`를 고정했다.
- Complete Page의 `장비 제작 가이드 열기 (이동만)` Action은 registered Nomad Tab만 열며 Equipment Builder VM/Draft/context에는 값을 주입하지 않는다.
- P004 Automation에서 완료 전 handoff 차단, registered spawner, fingerprint 불변, stale Draft handoff 차단과 복원 후 재성립을 검증했다.
- Final Official UE 5.8 Build `d393cb0ec5d246d288deb99594b2b328` PASS와 exact8 process `68bdfbb06d5c46dfb04a5c60ec78ff28` 8/8 PASS를 확정했다.
- fresh AssetDump에서 Product Weapons/Data exact11 / failed0, WeaponGuide fixture exact0 / failed0과 Git Product/fixture mutation0을 확인했다.
- WEA-P0-04 Technical PASS를 확정하고 lifecycle next를 WEA-P0-05 Final Technical + USER Acceptance로 전진한다. CF-FQ-054는 explicit USER resume 전까지 Paused 상태를 유지한다.

### v0.2.3 - 2026-09-18

- WEA-P0-03에서 Damage/Ammo 신규 child를 별도 writer 없이 기존 CFDADamageProvider/CFDAAmmoProvider operation table과 shared CFDADurableCore durable authority에 연결했다.
- Turret/Projectile/Weapon exact3와 provider exact2를 Damage → Ammo → Turret → Projectile → Weapon 순서의 multi-asset durable Create로 통합했다.
- mutation 전 fresh provider Create Preview/target absence preflight와 final persisted reference graph readback을 추가했다.
- partial durable 성공 child를 session exact path로 보존하고 자동 rollback/delete 없이 same-session retry에서 semantic fingerprint까지 재검증하도록 했다.
- Post-Implementation Review에서 exact3 retry의 stale Draft 재사용 가능성을 발견해 Turret/Projectile/Weapon persisted fingerprint exact 비교로 교정했다.
- `P003.DurableCreateGraph`와 강화된 `P003.PartialRecovery`를 포함한 exact7 process `895e656c48164d639ac326d8e6f50c75`가 7/7 PASS했다.
- Final Official UE 5.8 Build `28196840a5214813a82d54931f69866d` PASS를 확정했다.
- fresh AssetDump에서 Product Weapons/Data exact11 / failed0, WeaponGuide fixture exact0 / failed0과 Git Product/fixture mutation0을 확인했다.
- WEA-P0-03 Technical PASS를 확정하고 lifecycle next를 WEA-P0-04 Completion Bundle / Equipment Builder Integration Contract로 전진한다. CF-FQ-054는 Paused 상태를 유지한다.

### v0.2.2 - 2026-09-18

- WEA-P0-02 Guided Draft Native Slate UX를 구현하고 Stable Step rail, 단일 main Scroll, 분리/조건부 Page 구조를 실제 Editor 탭에 연결했다.
- 신규 Damage/Ammo Draft를 별도 writer 없이 existing CFDADamageProvider/CFDAAmmoProvider의 ReviewedMutationReady typed payload/fingerprint/serializer/parser 계약에 mutation0으로 연결했다.
- Damage radial normal authoring field와 Ammo Texture2D Icon을 포함해 provider mapping payload coverage를 보강했다.
- P0-02 durable entry가 incomplete sibling validation보다 먼저 WEA-P0-03 boundary로 fail-visible하도록 교정했다.
- Final Official UE 5.8 Build b5d8649e8c5448df9d76371bfb2955c8 PASS와 exact5 process 102ed9a51f1e4b1383f90dad59fd3bb1 5/5 PASS를 확정했다.
- fresh AssetDump에서 Product Weapons/Data exact11 / failed0, WeaponGuide fixture exact0 / failed0과 Git Product mutation0을 확인했다.
- WEA-P0-02 Technical PASS를 확정하고 lifecycle next를 WEA-P0-03 Multi-Asset Durable Create / Recovery로 전진한다. CF-FQ-054는 Paused 상태를 유지한다.

### v0.2.1 - 2026-09-18

- WEA-P0-01에서 fixed exact8 Prototype을 Stable StepId exact15 + VisibleSteps projection으로 실제 교정했다.
- Projectile durable payload에 PropulsionConfig / MissileFlightConfig / MissileGuideConfig roundtrip을 추가했다.
- existing Weapon↔Projectile/Ammo, Projectile↔Damage reference mismatch를 fail-closed하고 partial durable 이후 identity/template 변경을 차단했다.
- `CFWeaponGuideTests.cpp` exact5 focused Automation과 `RunWeaponGuideP01Tests.ps1` same-process runner를 추가했다.
- Official UE 5.8 Build 2회 PASS, focused exact5/5 PASS, Product exact11 유지, WeaponGuide fixture exact0을 근거로 WEA-P0-01 Technical PASS를 확정했다.
- lifecycle next를 WEA-P0-02 Guided Draft UX / Child Authoring으로 전진하되 CF-FQ-054 Paused 상태는 유지한다.

### v0.2.0 - 2026-09-17

- `BuilderAuthoringStandard.md v1.0.0` 적용으로 초기 fixed exact8 Wizard를 Stable StepId exact15 + current Capability 기반 VisibleSteps 구조로 재설계했다.
- Projectile/Ammo/Damage 결합 Page를 분리하고 Launcher/Heat/Charge 단일 Optional Page를 개별 Conditional Step으로 분리했다.
- fresh Runtime audit에서 확인한 `PropulsionConfig`, `MissileFlightConfig`, `MissileGuideConfig`를 정상 Guided authoring 범위에 포함했다.
- `MissileGuidePresetData` direct reference를 가정하지 않고 current persistent truth인 `ProjectileData.MissileGuideConfig` inline authoring을 사용하도록 고정했다.
- DamageData/AmmoData는 별도 Builder 연쇄 대신 Guide 내부 create/reuse UX를 제공하고 existing `CFDADamageProvider` / `CFDAAmmoProvider` writer authority를 재사용하도록 교정했다.
- existing Weapon 재사용 시 Projectile/Ammo, existing Projectile 재사용 시 Damage reference exact consistency를 fail-closed하도록 계약을 추가했다.
- WEA-P0-00R Builder Standard Rebaseline을 PASS로 추가하고 WEA-P0-01을 Core Workflow/State/Naming Correction으로 재정의했다.

### v0.1.0 - 2026-09-17

- 최초 Weapon Equipment Authoring Guide 독립 Feature 계약을 작성했다.
- initial exact8 Wizard와 Turret/Weapon/Projectile exact3 Create authority, Equipment Builder handoff boundary를 정의했다.

---

## 22. Migration

- v0.2.0부터 v0.1.0의 고정 exact8 Step 번호와 `PayloadChain`, `OptionalFeatures` 의미는 현재 구현 기준이 아니다.
- 아직 Build Acceptance 전인 `CFWeaponGuideTypes/VM` v1.0.0 Prototype은 v0.2.0 설계에 맞춰 직접 교정하며, 이를 기존 Current System 호환성 파괴로 간주하지 않는다.
- 기존 Product Asset resave는 요구하지 않는다.
- 기존 Ammo/Damage Provider/DACE history는 변경하지 않는다.
- CF-FQ-054는 자동 재개하지 않는다.
- `Document/ActiveWork.md`는 frozen authority0이므로 이 작업으로 수정하지 않는다.
