# CarFight Weapon Content Roster Plan

- 문서 버전: v0.1.12
- 작성일: 2026-09-30
- 최근 갱신일: 2026-10-07
- 문서 상태: USER Accepted Production Content Baseline / Production exact8 ACTIVE / Durable VehicleReady exact8 / Canonical Workbook Authority ACTIVE / Publication Catalog exact8 ACTIVE / Flexible Data Role Composition Baseline / End-to-End Vehicle-Ready Contract USER Accepted / CF-FQ-056 PFP-P0-03 USER ACCEPTED COMPLETE
- Project: CarFight
- 대상: 초기 실제 무기 콘텐츠 라인업 / Weapon Content Roster
- 관련 Feature: CF-FQ-058 Content Catalog & Authoring System
- 구현/에셋 변경: First Production Wave generated Production target exact34 + canonical Workbook/provenance + Publication Catalog 활성화
- Product Apply / Save: exact8 durable VehicleReady / Publication exact8 ACTIVE
- Git stage / commit / push: 없음

---

## 1. 목적

이 문서는 CarFight에서 실제 Product 콘텐츠로 제작할 초기 무기 라인업의 게임 디자인 기준선을 정의한다.

목표는 최종 밸런스 숫자를 확정하는 것이 아니라 다음을 명확히 하는 것이다.

- 어떤 Weapon Family(무기 계열)가 필요한가
- 각 Family가 어떤 Gameplay Role(게임플레이 역할)을 담당하는가
- 같은 Family의 Variant(변형)가 어떤 축으로 실제 차이를 가져야 하는가
- 현재 구현으로 바로 콘텐츠화 가능한 범위와 후속 기술이 필요한 범위를 구분한다
- 첫 실제 제작 Wave의 규모를 1인 개발 범위로 제한한다
- 결과를 CF-FQ-058의 Family / Variant / Design Intent 및 향후 CCAS-P0-06 AI Change Set + Batch Planning 입력으로 사용한다

2026-09-30 USER Review에서 Family / Variant / First Production Wave와 Choice 1~5가 승인되었다. 이 승인은 **기획 기준선과 CF-FQ-058 전달 입력에 대한 승인**이며, 당시 DataAsset 생성, Workbook 작성, CCAS 구현, Product Apply / Save를 시작하는 승인이 아니었다.

2026-10-03 USER 결정으로 Production 제작 정책을 교정한다. 기존 `HeavyCannon` / `RocketLauncher`는 prototype/test 성격과 수치가 강하므로 Production anchor로 승격하거나 수치를 개조하지 않고 **Test/Legacy reference**로 유지한다. First Production Wave exact8은 모두 별도 신규 Production Product identity로 제작한다. 기존 두 자산의 검증된 구조·참조 관계는 authoring reference로 재사용할 수 있으나 Product identity와 prototype balance는 승계하지 않는다.

---

## 2. 조사 기준선

### 2.1 Fresh persisted Product 현황

2026-09-30 Fresh AssetDump 기준으로 CFWeaponData는 exact9가 확인되었으며 Product 경로 /Game/CarFight/Weapons/Data/WeaponDefs 아래의 실제 WeaponData는 다음 exact2다.

| Product | EquipmentPreset | WeaponData | 현재 성격 |
| --- | --- | --- | --- |
| Heavy Cannon | HeavyCannon | DA_ProtoTurretCannon | 현재 Product 기준 캐논. Prototype 수치는 provisional balance로 취급 |
| Rocket Launcher | RocketLauncher | DA_RocketLauncher | 현재 Product 기준 비유도 로켓 런처. Persisted 발사 패턴은 Salvo 4발 |

나머지 WeaponData는 Tests 아래의 AmmoIntegration, LauncherRegression, Missile, VehicleDefense fixture다. 이들은 기술 검증 자산이며 Product Roster에 자동 편입하지 않는다.

### 2.2 현재 Product 구성 관계

현재 저장 관계의 핵심은 다음과 같다.

EquipmentPreset
→ TurretMountData
→ WeaponData
→ DefaultProjectileData
→ ProjectileData.DefaultDamageData

WeaponData
→ DefaultAmmoData 또는 AmmoTypeId fallback
→ LauncherFirePatternConfig / LauncherReleaseConfig inline 설정

ProjectileData
→ PropulsionConfig
→ MissileFlightConfig
→ MissileGuideConfig
→ DamageData
→ Projectile Mesh / Trail / Thruster FX

중요:

- Launcher는 현재 별도 필수 DataAsset이라기보다 Runtime Scheduler + WeaponData inline Launcher config 조합이다.
- Missile Guidance도 별도 필수 Guide DataAsset이 아니라 ProjectileData의 inline MissileGuideConfig가 current persistent truth다.
- DamageData는 WeaponData의 직접 child가 아니라 ProjectileData가 소유한다.

### 2.3 Product TurretMount 현황

Product TurretMount는 현재 두 축으로 정리된다.

| Mount | 현재 사용 | 질량 기준 | 초기 재사용 판단 |
| --- | --- | ---: | --- |
| DA_CannonBody | Heavy Cannon | 200 kg | Cannon Family의 초기 공용 mount 후보 |
| DA_RocketBody | Rocket Launcher, Missile test preset에서도 재사용 | 150 kg | Rocket / 초기 Guided Missile 공용 mount 후보 |

Mount 공유는 제작 비용을 줄이기 위한 초기 기준이며 최종 실루엣, muzzle 수, 크기 또는 차량 호환성이 달라지면 Variant별 Mount 분리를 허용한다.

### 2.4 Projectile / Damage / Ammo 현황

Product에 실제 참조되는 대표 Projectile은 다음과 같다.

- DA_HeavyShell
  - 중력 적용
  - 비추진 탄도 Projectile
  - 현재 DA_DamageAsset 참조
- DA_Rocket_PropTest
  - Product RocketLauncher가 실제 참조하는 persisted dependency
  - 이름은 PropTest지만 Product 경로에서 실제 Product Launcher가 사용하고 있으므로 현재 dependency로 취급
  - 중력 + 추진 사용
  - Guidance 미사용
  - 현재 DA_DamageAsset 참조
- DA_Missile_DirectTest
  - Tests/Missile 아래의 fixture
  - TargetActor Guidance 기술 검증용
  - Product Missile로 간주하지 않음

DamageData는 Product 경로에 exact2가 존재한다.

- DA_DamageAsset: ProtoDirectHit, Kinetic, BaseDamage 25의 현재 공용 direct-hit prototype
- DA_DamageArmorPenTest: DamageId 자체가 ArmorPenetration_Test이므로 Roster Product로 자동 승격하지 않는 기술 검증 성격 자산

AmmoData는 exact2가 모두 Tests/AmmoIntegration에 있으며 Product AmmoData는 현재 0이다.

따라서 첫 실제 Product Wave에서는 기존 debug infinite ammo를 그대로 콘텐츠 기준으로 삼지 않고, finite AmmoData를 실제 Product 콘텐츠로 만드는 것을 기본 계획으로 한다. 단 실제 생성은 본 세션 범위 밖이다.

### 2.5 현재 구현된 무기 기술

현재 기반으로 콘텐츠 차별화에 사용할 수 있는 핵심 축:

- Weapon mass
- Weapon size / compatible mount
- Projectile FireMode
- Fire rate
- Max range
- Spread
- Magazine / finite ammo / reserve / reload
- SingleCycle / Ripple / Salvo Launcher pattern
- Projectile count per trigger
- Inter-muzzle delay
- Direct / Angled / Vertical launcher release 계약
- Projectile initial speed / lifetime / gravity
- Propulsion ignition / burn / thrust / propelled speed
- TargetActor guided missile
- Guidance law / turn-rate / lateral-acceleration / response-time 계열
- Lock 기반 Guided Projectile target source
- ArmorPenetration 값과 VehicleDefense 연동 경로
- Projectile interceptability
- Heat / Charge authored field 기반

현재 Roster의 확정 차별화 축으로 사용하지 않는 항목:

- Recoil: 현재 CarFight_Re Source에서 Recoil 소유 데이터/런타임을 확인하지 못함
- Radial Explosion gameplay: DamageData에 폭발 필드는 존재하지만 현재 헤더 계약상 후속 Damage Runtime 후보이며 첫 Wave의 실제 폭발 성능 차별화 축으로 간주하지 않음
- LaserPoint / InertialPoint / DataLink guided weapon: enum/config 존재만으로 Product-ready를 주장하지 않음
- Auto defensive fire / CIWS: 현재 VehicleDefense test weapon과 별개이며 Product 자동 방어무기 Runtime으로 확인되지 않음

### 2.6 Projectile Flight Physics 상태

CF-FQ-056 Projectile Flight Physics는 PFP-P0-02 Technical Implementation PASS를 보존한 채 PFP-P0-03 USER ACCEPTED / COMPLETE로 닫혔다.

Current evidence:

- Cannon gravity-on trajectory USER PASS
- Guided Missile locked-target guidance USER PASS
- BurnedOut transition USER PASS
- Rocket visual re-observation은 USER가 low-priority로 명시적으로 waive했으며 P0-02 Technical PASS에 의존한다.

따라서 Cannon / Rocket / Guided Missile 콘텐츠 설계·authoring은 현재 First Production Wave에서 진행 가능하다. Guided Missile에 기존 `PFP.P0.03.UserTrajectory Pending`을 다시 부여하지 않으며, Test missile asset을 Product로 복사·승격했다고 간주하지 않는 원칙은 유지한다.

---

## 3. Weapon Family 결정

초기 Roster는 Family를 과도하게 늘리지 않고 exact3 Core Family로 시작한다.

### 3.1 Cannon Family

Role:
직사 탄도 화력의 기준 Family. 조준 정확도와 포탄 비행을 사용해 단일 표적을 직접 타격한다.

존재 이유:
CarFight의 가장 이해하기 쉬운 기본 무기로서 Damage / Fire Rate / Projectile Speed / Accuracy / Range / Mass / Ammo trade-off의 기준축을 제공한다.

대표 Gameplay:
- 시야와 조준선을 직접 확보
- 발사 시점과 lead 판단이 중요
- Variant에 따라 순간 화력, 지속 화력, 원거리 정밀성이 분리됨

주요 차별화 축:
- Direct damage
- Armor penetration
- Fire rate
- Projectile speed
- Spread
- Effective range
- Weapon / mount mass
- Magazine / ammo economy
- Reload

현재 제외 축:
- Recoil은 별도 Runtime 소유가 확인될 때까지 Design Note로만 유지

### 3.2 Unguided Rocket Family

Role:
비유도 추진체를 한 발 또는 여러 발의 sequence로 발사해 순간 압박과 공격 패턴 차이를 만드는 Family.

존재 이유:
Cannon과 달리 Projectile propulsion과 Launcher pattern이 플레이를 결정한다. 특히 Standard / Salvo / Ripple은 동일 projectile과 warhead를 공유하면서 firing pattern만으로 의미 있는 Variant 차이를 만들 수 있어 CCAS Family/Variant 검증에 적합하다.

대표 Gameplay:
- Cannon보다 느린 추진체
- 타이밍과 salvo/ripple pattern이 중요
- 높은 탄약 소모와 공격 window 관리

주요 차별화 축:
- Rockets per attack
- Single / Salvo / Ripple pattern
- Launch interval
- Fire rate / sequence cooldown
- Projectile speed / propulsion
- Ammo capacity / reload
- Weapon mass
- Direct-hit warhead strength

현재 제외 축:
- Explosion radius를 실제 gameplay 차별화 축으로 사용하지 않음. Radial damage Runtime readiness가 별도 확인된 뒤 후속 확장.

### 3.3 Guided Missile Family

Role:
Lock된 표적에 대해 비행 중 Guidance를 사용해 명중 가능성을 높이는 고가치 정밀 공격 Family.

존재 이유:
Sensor / Targeting / Lock과 Weapon / Projectile / Guidance를 실제 콘텐츠에서 연결하는 대표 Family다. Cannon/Rocket과 달리 표적 획득 과정 자체가 공격 비용이 된다.

대표 Gameplay:
- Target lock 필요
- 발사 후 guidance performance가 명중 가능성 결정
- 적은 탄수와 긴 공격 준비를 대가로 움직이는 표적에 높은 추적 성능 제공

주요 차별화 축:
- Guidance turn rate
- Lateral acceleration
- Guidance response
- Seeker / acquisition 관련 성능
- Speed / propulsion burn
- Range / lifetime
- Lock requirement
- Ammo count / reload
- Warhead
- Weapon / launcher mass

현재 제한:
- Product Guided Missile은 아직 없음
- PFP-P0-03 USER trajectory review는 USER ACCEPTED / COMPLETE로 해소됨
- 첫 Wave에서는 Standard Guided Missile exact1만 넣어 기술·콘텐츠 연결을 검증하고 LongRange / Agile / Heavy Warhead는 후속으로 둔다

---

## 4. Variant 후보

### 4.1 Cannon

| Variant | Gameplay Role | Design Intent | Standard 대비 강점 | Standard 대비 약점 | 주요 축 | 준비 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| Standard | 범용 직사포 | 모든 비교의 기준형 | 균형 | 특화 없음 | Damage = / FireRate = / Speed = / Mass = | 신규 Product data 필요, backend 준비 |
| Heavy | 중장갑/고가치 표적 | 한 발의 가치와 관통을 높이고 지속 화력·적재성을 희생 | Damage ↑, Penetration ↑ | FireRate ↓, Ammo ↓, Mass ↑ | Damage, penetration, mass, ammo | 신규 Production Product data 필요. 기존 HeavyCannon은 Test/Legacy reference로만 사용 |
| LongRange | 원거리 정밀 사격 | 탄속·정확도·유효사거리를 높이고 근거리 지속화력을 낮춤 | Speed ↑, Accuracy ↑, Range ↑ | FireRate ↓ 또는 Ammo ↓ | projectile speed, spread, range | data 조합으로 구현 가능 |
| Rapid | 경량/중형 표적 지속 압박 | 단발 화력을 낮추고 연사와 지속 압박을 높임 | FireRate ↑ | Damage ↓, Ammo consumption ↑, Spread ↑ 가능 | fire rate, per-shot damage, magazine | Projectile형 Autocannon으로 구현 가능. Recoil 의존 금지 |

### 4.2 Unguided Rocket

| Variant | Gameplay Role | Design Intent | Standard 대비 강점 | Standard 대비 약점 | 주요 축 | 준비 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| Standard | 범용 단발 로켓 | 한 trigger에 1발을 기준으로 탄약과 공격 타이밍을 통제 | Ammo efficiency ↑ | Burst ↓ | projectile count, fire rate | 현재 Launcher config로 구현 가능 |
| Salvo | 순간 집중 화력 | 여러 로켓을 동시에 발사해 짧은 attack window에 화력 집중 | Burst ↑, exposure ↓ | Ammo consumption ↑, miss penalty ↑ | Salvo, simultaneous count | 신규 Production Product data 필요. 기존 RocketLauncher의 Salvo 4발 구조는 Test/Legacy reference로만 사용 |
| Ripple | 순차 압박 | 같은 총 발사수를 시간차로 분산해 움직이는 표적에 연속 압박 | Sustained pressure ↑, correction opportunity ↑ | Instant burst ↓, exposure ↑ | Ripple, interval | Launcher Runtime 준비. 기존 test fixture를 Product로 자동 승격하지 않고 신규 Product Variant로 제작 |
| Heavy | 중화력 로켓 | 더 큰 warhead와 낮은 탄수 | Direct damage ↑ | Ammo ↓, rate ↓, mass ↑ | projectile / damage / mass | Wave 2 후보. 범위폭발을 핵심 정체성으로 삼으려면 radial Damage Runtime 선행 필요 |

### 4.3 Guided Missile

| Variant | Gameplay Role | Design Intent | Standard 대비 강점 | Standard 대비 약점 | 주요 축 | 준비 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| Standard | Lock 기반 범용 정밀타격 | Guided Family 기준형 | Tracking capability | Lock/time cost, low ammo | guidance, lock, propulsion | Wave 1B. Product asset 신규 필요 / PFP-P0-03 USER ACCEPTED COMPLETE / AuthoringReady |
| LongRange | 원거리 정밀타격 | 더 긴 비행·추적 거리와 추진 지속 | Range ↑ | speed/turn/ammo 또는 reload 희생 | lifetime, propulsion, targeting range | Wave 2. Sensor/Lock range와 함께 검증 필요 |
| Agile | 기동 표적 요격 | 선회·횡가속 응답을 높여 회피 표적 추적 | Maneuver ↑ | Range/warhead/ammo trade-off | turn rate, lateral accel, response | Backend knob 존재. Wave 2 USER feel tuning 필요 |
| HeavyWarhead | 고가치 표적 타격 | 추적성보다 warhead를 우선 | Damage ↑ | agility ↓, ammo ↓, mass ↑ | damage, mass, guidance budget | Wave 2. 폭발형 정체성 사용 시 radial runtime 선행 |

---

## 5. 후속 Family 후보와 판정

| 후보 | 판정 | 이유 |
| --- | --- | --- |
| Machine Gun | Wave 2 후보 | HitScan/virtual Projectile 기반은 존재하지만 Cannon Rapid와 첫 Wave 역할이 겹침. 경량 무기·탄약·비주얼 기준이 생긴 뒤 분리 |
| Autocannon | Cannon Rapid로 통합 | 현재는 별도 Family를 만들 만큼 구조적 차이가 필요하지 않음 |
| Shotgun / Scatter | 보류 | Spread 값은 있으나 전용 multi-pellet / scatter semantics가 확인되지 않음. Launcher 다발 발사로 우회 구현하지 않음 |
| Large Single-Shot | Cannon Heavy 확장으로 우선 대응 | 독립 Family보다 Heavy/Siege 계열 Variant가 자연스러움. Super-heavy mount가 필요해질 때 재검토 |
| Defensive Weapon | 보류 | VehicleDefense test fixture는 방어무기 Product가 아님. 자동 위협선정/요격/발사 책임을 별도로 확인해야 함 |
| Special / Utility | 보류 | Damage 외 상태효과, EMP, 지원 효과 등의 명시적 Runtime owner가 정의된 뒤 Family화 |

---

## 6. First Production Wave

첫 Wave는 USER 승인 기준으로 exact8을 확정한다.

목표는 무기 수를 늘리는 것이 아니라 세 가지를 동시에 검증하는 것이다.

1. Cannon에서 stat-based Variant 차이가 실제 gameplay 차이로 보이는가
2. Rocket에서 shared projectile + launcher pattern override가 의미 있는 Family/Variant 모델이 되는가
3. Guided Missile에서 Sensor / Lock / Guidance / Projectile이 실제 Product 콘텐츠로 처음 연결되는가

### 6.1 Wave 1A — 현재 데이터 기반으로 제작 준비 가능한 exact7

| Family | Variant | Role | Design Intent | 현재 구현 가능 | 우선순위 |
| --- | --- | --- | --- | --- | --- |
| Cannon | Standard | 범용 직사포 | 모든 Cannon 비교의 기준형 | 가능 — 신규 Product DA 필요 | Wave 1A |
| Cannon | Heavy | 중장갑/고가치 타격 | 화력·관통 ↑, 연사·적재 ↓ | 가능 — 신규 Production Product DA 제작, 기존 HeavyCannon은 구조 참고만 | Wave 1A |
| Cannon | LongRange | 원거리 정밀 | 탄속·정확도·사거리 ↑ | 가능 — data 중심 | Wave 1A |
| Cannon | Rapid | 지속 압박 | 연사 ↑, 단발 화력·탄 효율 ↓ | 가능 — Projectile형으로 제한 | Wave 1A |
| UnguidedRocket | Standard | 범용 단발 로켓 | 공격 1회당 1발 기준 | 가능 — Launcher config | Wave 1A |
| UnguidedRocket | Salvo | 순간 집중 화력 | 다발 동시 발사 | 가능 — 신규 Production Product DA 제작, 기존 RocketLauncher는 구조 참고만 | Wave 1A |
| UnguidedRocket | Ripple | 순차 압박 | 다발 시간차 발사 | 가능 — Launcher Runtime 준비 | Wave 1A |

### 6.2 Wave 1B — Guided technology USER gate가 충족된 신규 Product exact1

| Family | Variant | Role | Design Intent | 현재 구현 가능 | 우선순위 |
| --- | --- | --- | --- | --- | --- |
| GuidedMissile | Standard | Lock 기반 정밀타격 | Guided Family 기준형 | 가능 — Product Missile 신규 제작 필요 / PFP-P0-03 USER ACCEPTED COMPLETE | Wave 1B |

Wave 1 총계: exact8.

Guided Missile을 Wave 1에서 제거하지 않는 이유는 CF-FQ-058 대량관리 입력이 Cannon/Rocket만으로 끝나면 Guidance inline config, Lock dependency, Projectile flight variation을 실제 Product 콘텐츠로 검증하지 못하기 때문이다.

Guided Missile의 기존 PFP-P0-03 USER Gate는 이미 통과했다. Wave 1A / 1B 구분은 현재 기술 blocker가 아니라 제작 순서와 Family 구분을 위한 Roster identity로 유지하며, actual Production Product는 새 corrected Review Package와 별도 USER Cutover Acceptance 경계를 따른다.

---

## 7. 상대 밸런스 기준선

정확한 final 숫자는 이번 문서에서 확정하지 않는다.

### 7.1 Cannon relative intent

| Stat | Standard | Heavy | LongRange | Rapid |
| --- | --- | --- | --- | --- |
| Damage | Medium | High | Medium | Low |
| Fire Rate | Medium | Low | Medium-Low | High |
| Projectile Speed | Medium | Medium | High | Medium |
| Accuracy / Spread | Medium | Medium | High | Medium-Low |
| Range | Medium | Medium | High | Medium-Low |
| Weapon Mass | Medium | High | Medium-High | Medium |
| Ammo Efficiency | Medium | Low-Medium | Medium | Low |
| Reload burden | Medium | High | Medium-High | Medium-High |

### 7.2 Rocket relative intent

| Stat | Standard | Salvo | Ripple |
| --- | --- | --- | --- |
| Rockets per attack | 1 | High | High |
| Instant burst | Low-Medium | High | Medium |
| Sequence duration | Short | Short | Long |
| Ammo consumption per full attack | Low | High | High |
| Exposure while attacking | Low | Low | High |
| Projectile / warhead | Shared candidate | Shared candidate | Shared candidate |
| Launcher pattern | SingleCycle | Salvo | Ripple |

이 구조에서는 Rocket Standard / Salvo / Ripple의 projectile, DamageData, AmmoData를 가능한 한 공유하고 WeaponData의 Launcher pattern과 ammo use에서 Variant 차이를 만든다.

### 7.3 Guided Missile Standard

첫 Variant는 Family 기준점만 만든다.

- Damage: Medium
- Guidance: Medium
- Maneuverability: Medium
- Range: Medium
- Ammo count: Low
- Reload burden: High
- Lock dependency: Required

LongRange / Agile / HeavyWarhead의 정확한 상대값은 Standard의 USER gameplay 검수 후 Wave 2에서 확정한다.

---

## 8. DataAsset 공유 / 분리 원칙

### 8.1 공통 원칙

Family 공유는 무조건적인 복제가 아니라 의미가 같은 데이터만 공유한다.

Variant가 Design Intent를 위해 값이 달라져야 하는 경우 별도 DataAsset을 만든다.

현재 First Wave에서 확인된 `EquipmentPreset / TurretMount / Weapon / Projectile / Damage / Ammo` exact6은 **현재 제작에 필요한 초기 Data Role 집합**이며 고정된 영구 schema가 아니다. 실제 Production 제작 중 별도 책임이 필요한 DataAsset이 발견되면 Role을 추가할 수 있고, 반대로 별도 DataAsset이 불필요해져 inline config 또는 다른 DataAsset에 흡수되면 해당 Role을 제외할 수 있어야 한다.

#### 8.1.1 Flexible Data Role Composition

Product의 DataAsset 구성을 고정 column 목록으로 표현하지 않고 `Product × Role` binding 집합으로 다룬다.

각 binding은 최소 다음 의미를 가진다.

- `RoleId`: Product 안에서의 stable semantic role
- `ContentType / AssetClass`: binding 대상 DataAsset 종류
- `Requirement`: Required / Optional / Conditional
- `Cardinality`: Single / Multiple
- `Ownership`: Shared / VariantOwned
- `ParentRole`: 참조 그래프상 상위 Role
- `ActivationCondition`: 해당 Role이 필요한 조건
- `AuthoringProviderId`: 생성/갱신/검증을 담당하는 typed authoring provider
- `BoundContentKey / BoundAsset`: 실제 연결된 logical content 또는 persisted asset
- `ManagementState / Lifecycle`: Managed / ExternalReadOnly 및 Active / Deprecated / Retired 계열 상태

현재 Wave 1의 초기 Role projection은 다음과 같다.

| RoleId | 현재 대표 DA | 기본 Requirement | 기본 Ownership |
| --- | --- | --- | --- |
| EquipmentPreset | UCFEquipmentPresetData | Required | VariantOwned |
| Mount | UCFTurretMountData | Required | Shared 가능 |
| WeaponDefinition | UCFWeaponData | Required | VariantOwned |
| Projectile | UCFProjectileData | FireMode에 따라 Conditional | Shared 또는 VariantOwned |
| Damage | UCFDamageData | Projectile topology에 따라 Conditional | Shared 또는 VariantOwned |
| Ammo | UCFAmmoData | finite ammo policy에 따라 Conditional | Shared 또는 VariantOwned |

이 표는 현재 First Wave의 **초기 binding 예시**이며 허용 가능한 Role의 폐쇄 목록이 아니다.

향후 `SeekerData`, `ExplosionData`, `RecoilData` 같은 별도 DataAsset이 실제 Runtime/Authoring 책임으로 도입되면 기존 exact6을 재설계하지 않고 새 Role descriptor와 Provider를 추가해 Product binding에 포함할 수 있어야 한다. 반대로 Guidance, Heat, Charge처럼 inline config가 적합한 항목은 별도 DA를 억지로 만들지 않는다.

Role을 제외하는 경우에도 binding 제거와 `.uasset` 물리 삭제를 동일 동작으로 취급하지 않는다. 기존 asset은 migration/review 없이 자동 삭제하지 않으며 CCAS의 `Retire != physical delete` 원칙을 유지한다.

Workbook/CCAS 표현 역시 `Preset/Mount/Weapon/Projectile/Damage/Ammo`를 영구 고정 column으로 강제하지 않고, canonical composition은 row-oriented Role Binding으로 확장 가능해야 한다. 자주 쓰는 Role은 USER-facing 비교 화면에서 column으로 projection할 수 있다.

### 8.2 EquipmentPreset

원칙:
- Variant별 separate EquipmentPreset
- 각 Variant가 독립적인 장비 identity를 가져야 Catalog / Fitting / Compare가 명확해짐

공유:
- 없음을 기본으로 함

### 8.3 TurretMountData

Cannon:
- Wave 1 초기에는 DA_CannonBody 공유 가능
- Mount mass / geometry / muzzle / articulation이 Variant identity를 침범하면 분리

Rocket:
- Standard / Salvo / Ripple은 DA_RocketBody 공유 우선
- Heavy Rocket은 향후 별도 mount 후보

Guided Missile:
- Standard는 초기 제작 비용 절감을 위해 DA_RocketBody 공유 후보
- Guided 전용 pod / launcher가 필요해질 때 분리

### 8.4 WeaponData

원칙:
- 모든 Variant별 separate WeaponData
- Family / Variant 차이를 가장 직접적으로 표현하는 top-level gameplay record

주요 Variant-owned 항목:
- FireRate
- MaxRange
- Spread
- WeaponMass
- Ammo / Magazine / Reload
- Launcher pattern
- TargetUsePolicy
- DefaultProjectileData

### 8.5 ProjectileData

Cannon:
- Standard / Heavy / LongRange / Rapid는 projectile speed, damage reference, ballistic role이 달라질 가능성이 높으므로 별도 ProjectileData를 기본으로 함
- 동일 shell을 의도적으로 공유하는 경우에만 재사용

Rocket:
- Standard / Salvo / Ripple은 동일 unguided Rocket ProjectileData 공유를 우선
- Launcher pattern 차이를 WeaponData에서 표현

Guided Missile:
- 별도 Guided ProjectileData 필수
- MissileFlight / MissileGuide inline config가 Variant 성능의 핵심

### 8.6 AmmoData

첫 Wave에서는 Product AmmoData를 실제로 도입하는 것을 목표로 한다.

- 현재 Product AmmoData exact0
- 기존 HeavyCannon / RocketLauncher의 infinite-debug 상태를 최종 콘텐츠 기준으로 삼지 않음
- Rocket Standard / Salvo / Ripple은 동일 rocket ammo 공유를 우선
- Cannon은 projectile/탄종 identity에 따라 공용 또는 Variant별 분리
- Guided Missile은 guided missile ammo를 별도 소유하는 것을 기본으로 함

### 8.7 DamageData

Cannon:
- Heavy / Rapid처럼 per-shot damage 또는 penetration intent가 달라지면 별도 DamageData
- Standard / LongRange가 동일 warhead를 의도하면 공유 가능

Rocket:
- Standard / Salvo / Ripple은 같은 direct-hit warhead를 공유하는 것이 우선
- 현재 radial explosion은 first-wave gameplay authority로 사용하지 않음

Guided:
- Standard Guided가 unguided Rocket과 동일 warhead를 의도하면 DamageData 공유 가능
- hit probability와 raw damage budget을 별도로 조정하기로 결정하면 분리

### 8.8 Production exact8 Data Role Binding Matrix — AI Technical Baseline 2026-10-06

이 Matrix는 Flexible Data Role Composition 계약을 실제 First Production Wave exact8에 적용한 current AI Technical Production composition baseline이다.

중요한 전제:

- 기존 `HeavyCannon`, `RocketLauncher`, `DA_CannonBody`, `DA_RocketBody`, prototype Projectile/Damage/Ammo는 **구조와 수치 참고 evidence일 뿐 Production binding target으로 직접 사용하지 않는다.**
- 따라서 이번 Matrix의 `BindShared`는 기존 Test/Legacy asset 재사용이 아니라 **이번 Production Wave에서 새로 만든 shared Production DA를 다른 Variant가 함께 참조한다는 뜻**이다.
- current exact8에서 기존 Test/Legacy DA를 직접 `ReuseExisting`하는 binding은 exact0이다.
- `CreateNewShared`는 해당 shared DA를 최초 1회 생성하는 owner binding, `BindShared`는 같은 신규 shared DA를 참조하는 후속 binding이다.
- shared DA의 actual gameplay payload는 `(ContentTypeId + ContentId)` canonical authored row exact1만 소유한다. `BindShared` consumer Product는 관계만 선언하며 shared target의 수치를 local override하지 않는다.
- 같은 shared target에 `CreateNewShared` owner가 exact1이 아니거나, AssetClass / Provider / BoundContentId가 충돌하거나, consumer별 desired fingerprint가 갈라지면 mutation 전에 fail-closed한다.
- exact8은 모두 Projectile weapon이므로 현재 Wave에서는 `Projectile`, `Damage`, `Ammo` Role도 실제 Product composition상 Required로 취급한다. 단 일반 CCAS schema에서는 FireMode/finite-ammo 정책에 따라 Conditional일 수 있다.
- Guided Missile Standard의 historical PFP-P0-03 conditional gate는 CF-FQ-056 USER ACCEPTED / COMPLETE로 충족됐다. Current exact8에서는 Guided Product를 구성하는 DA Role exact6을 Required로 유지하고 Product readiness는 AuthoringReady다.

#### 8.8.1 Matrix

| Product | EquipmentPreset | Mount | WeaponDefinition | Projectile | Damage | Ammo |
| --- | --- | --- | --- | --- | --- | --- |
| `Cannon_Standard` | CreateNew / VariantOwned | **CreateNewShared `CannonMount_W1`** | CreateNew / VariantOwned | CreateNew / VariantOwned | **CreateNewShared `CannonStdDamage_W1`** | **CreateNewShared `CannonStdAmmo_W1`** |
| `Cannon_Heavy` | CreateNew / VariantOwned | BindShared `CannonMount_W1` | CreateNew / VariantOwned | CreateNew / VariantOwned | CreateNew / VariantOwned | CreateNew / VariantOwned |
| `Cannon_LongRange` | CreateNew / VariantOwned | BindShared `CannonMount_W1` | CreateNew / VariantOwned | CreateNew / VariantOwned | BindShared `CannonStdDamage_W1` | BindShared `CannonStdAmmo_W1` |
| `Cannon_Rapid` | CreateNew / VariantOwned | BindShared `CannonMount_W1` | CreateNew / VariantOwned | CreateNew / VariantOwned | CreateNew / VariantOwned | CreateNew / VariantOwned |
| `Rocket_Standard` | CreateNew / VariantOwned | **CreateNewShared `RocketPodMount_W1`** | CreateNew / VariantOwned | **CreateNewShared `RocketProjectile_W1`** | **CreateNewShared `RocketDamage_W1`** | **CreateNewShared `RocketAmmo_W1`** |
| `Rocket_Salvo` | CreateNew / VariantOwned | BindShared `RocketPodMount_W1` | CreateNew / VariantOwned | BindShared `RocketProjectile_W1` | BindShared `RocketDamage_W1` | BindShared `RocketAmmo_W1` |
| `Rocket_Ripple` | CreateNew / VariantOwned | BindShared `RocketPodMount_W1` | CreateNew / VariantOwned | BindShared `RocketProjectile_W1` | BindShared `RocketDamage_W1` | BindShared `RocketAmmo_W1` |
| `GuidedMissile_Standard` | CreateNew / VariantOwned | BindShared `RocketPodMount_W1` / initial | CreateNew / VariantOwned | CreateNew / VariantOwned | CreateNew / VariantOwned | CreateNew / VariantOwned |

위 `*_W1` 명칭은 **Matrix 안에서 공유 topology의 역사적 working key를 명확히 식별하기 위한 표기**다. 실제 Unreal asset filename / canonical ContentKey는 Section 8.12에서 이미 freeze된 naming을 사용하며 `*_W1` 문자열 자체를 Production identity로 강제하지 않는다.

#### 8.8.2 공유 판단 근거

Cannon:

- four Variant 모두 동일 기본 turret geometry를 사용할 수 있으므로 Wave 1에서는 Production `CannonMount` exact1을 새로 만들고 공유한다.
- Standard / LongRange는 direct-hit raw damage identity를 같게 두고 LongRange 차이는 Projectile speed / spread / lifetime / range와 WeaponData 쪽에서 표현한다. 따라서 DamageData를 공유한다.
- AmmoData는 `AmmoId / AmmoFamily / UnitMassKg / MaximumLoadableAmmoCount / ResupplyPolicy` 의미를 가지므로, Standard / LongRange는 동일 shell family를 의도하는 초기 기준으로 공유한다. LongRange의 magazine/reload 부담 차이는 WeaponData가 소유한다.
- Heavy는 높은 per-shot damage / penetration / 중량화된 탄종을 의도하므로 Damage와 Ammo를 분리한다.
- Rapid는 낮은 per-shot damage와 높은 소비량을 가지는 별도 탄종 identity가 자연스러우므로 Damage와 Ammo를 분리한다.
- Cannon Projectile는 탄속/탄도 역할이 Variant identity에 직접 영향을 주므로 exact4 모두 분리한다.

Rocket:

- Standard / Salvo / Ripple의 정체성 차이는 Projectile/warhead가 아니라 `WeaponData.LauncherFirePatternConfig`의 SingleCycle / Salvo / Ripple, count, interval이다.
- 따라서 Mount / Projectile / Damage / Ammo는 신규 Production exact1씩 공유하고 WeaponDefinition / EquipmentPreset만 Variant별로 분리한다.
- `MuzzleSocketNames`는 승인 발사마다 다음 muzzle index로 순환하고 `SingleCycle`은 입력당 유효 Projectile 수가 항상 1이다. 따라서 다중 muzzle Rocket pod를 공유해도 Standard가 강제로 Salvo가 되지 않으며, Salvo/Ripple도 같은 muzzle 배열을 순차적으로 소비할 수 있다.

Guided Missile:

- 첫 Standard는 Production `RocketPodMount_W1`을 초기 공유한다. 유도 여부와 target lock은 Mount가 아니라 Weapon/Projectile guidance 경로가 소유하므로 현재 Runtime 계약상 분리가 필수는 아니다.
- Guided Projectile은 Guidance/propulsion/flight 값이 핵심 identity이므로 Rocket Projectile과 공유하지 않는다.
- Guided Damage/Ammo도 첫 Product에서는 독립시킨다. 초기 수치가 Rocket과 우연히 같더라도 hit probability, ammo economy, future HeavyWarhead 분화 가능성을 고려해 semantic ownership을 분리한다.
- 향후 Seeker 전용 pod, 다른 muzzle geometry, 별도 ejection profile 등이 필요해지면 Mount Role binding만 분리하면 되며 Product identity를 재설계하지 않는다.

### 8.9 Matrix 결과 — unique Production DA exact34

현재 Matrix 기준 exact8은 Role binding으로는 `8 Products × 6 Roles = exact48`이다. 그러나 Shared binding을 반영하면 실제 신규 Production DA는 exact34다.

| DA Role | 신규 unique DA 수 | 산출 근거 |
| --- | ---: | --- |
| EquipmentPreset | 8 | Variant별 exact1 |
| Mount | 2 | Cannon shared exact1 + Rocket/Guided shared exact1 |
| WeaponDefinition | 8 | Variant별 exact1 |
| Projectile | 6 | Cannon exact4 + Rocket shared exact1 + Guided exact1 |
| Damage | 5 | Cannon exact3 + Rocket shared exact1 + Guided exact1 |
| Ammo | 5 | Cannon exact3 + Rocket shared exact1 + Guided exact1 |
| **합계** | **34** | Production exact8 current composition candidate |

이 `exact34`는 final immutable 수량이 아니다. 제작 중 Role이 추가/제외되거나 visual/muzzle/warhead identity가 분리되어야 한다는 evidence가 나오면 Data Role Composition 계약에 따라 Matrix를 갱신한다. 다만 변경은 해당 Role binding과 dependency만 수정하고 Generic CCAS Core를 다시 설계하지 않는다.

### 8.10 End-to-End Vehicle-Ready 적용 규칙 — USER Accepted

이 Roster의 Product는 `DA가 생성됐다`는 이유만으로 Production 준비 완료가 아니다. exact8 각각이 `ContentAuthoringSystemPlan.md v0.1.25`의 End-to-End Vehicle-Ready Contract와 Product-level Provisioning/Publication correction을 만족해야 한다.

Roster 관점의 고정 조건:

- Workbook의 Product/Role/Gameplay/Resource binding만으로 필요한 Production DA graph를 생성 또는 갱신할 수 있어야 한다.
- 생성 뒤 `EquipmentPreset → Mount + Weapon → Projectile/Ammo → Damage` required reference chain이 실제 persisted asset에서 완성되어야 한다.
- Test/Legacy DA는 reference evidence로만 사용하며 Production output이 prototype 자산 수동 수정에 의존하지 않는다.
- 생성된 EquipmentPreset은 별도의 사용자 수동 Catalog 등록 없이 기술검증을 통과한 뒤 CCAS generated `UCFProdEquipCatalogData` publication catalog에 자동 등록되어야 한다.
- Production publication 전에는 기존 `FCFRuntimeEquipApplyService::ApplyEquipmentRuntime` direct seam으로 compatible mount에 실제 적용해 technical proof를 완료한다.
- generated publication catalog의 membership만 Production 사용자 노출/선택 authority이며 RuntimeTestCatalog와 Inventory ownership 목록은 이 책임을 대신하지 않는다.
- finite weapon은 Provider-owned `EquipmentPresetAmmoLoads`에서 Product+Ammo Role별 explicit `DefaultSortieAmmoCount`를 authoring하고 이를 existing Fitting `InitialSortieAmmoLoads`로 projection한다. `MaximumLoadableAmmoCount`를 현재 탄약량으로 자동 사용하지 않는다.
- existing Fitting에 같은 AmmoId가 계속 필요하면 유효한 현재 수량을 보존하고, 새 AmmoId가 필요할 때 Product default를 추가한다. candidate 전체는 기존 `BuildFittingSnapshot`의 finite-ammo/질량 검증을 그대로 통과해야 한다.
- 차량에 적용한 뒤 Weapon runtime / Ammo runtime / Projectile-Damage reference resolution까지 기술적으로 초기화돼야 한다.
- shared Production DA를 수정할 때 이미 publish된 consumer가 있으면 해당 consumer를 mutation 전에 publication catalog에서 durable withdraw하고 impact closure 전체를 재검증한 뒤 PASS Product만 republish한다.
- Guided Missile Standard도 technical VehicleReady를 달성해야 한다. CF-FQ-056 PFP-P0-03은 이미 USER ACCEPTED / COMPLETE이므로 current Product authoring/cutover에 별도 Pending gate를 다시 만들지 않는다.
- Required Role 또는 Resource가 부족하면 생성 전에 fail-closed하거나 incomplete staged state로 남기며, 해당 Product를 Production discovery에 publish하지 않는다.

사용자가 exact34 내부 구조를 모두 직접 검수할 필요는 없다. Matrix 공유/분리와 실제 DA 수 최적화는 AI Technical responsibility이며, **사용자 승인 기준은 최종 결과가 Workbook에서 차량 적용까지 끊김 없이 작동하는가**이다.

### 8.11 Initial Production Balance Baseline — AI Technical Frozen 2026-10-06

아래 수치는 final balance가 아니라 **게임에 즉시 넣고 exact8의 역할 차이와 End-to-End VehicleReady를 검증하기 위한 첫 Production tuning baseline**이다. 이후 USER feel/playtest에서 수치는 Workbook으로 재튜닝할 수 있어야 하며, 재튜닝 때문에 DA topology나 Product identity를 다시 설계하지 않는다.

현재 기본 Vehicle durability가 `MaxHealth=100`인 Runtime scale을 기준으로 direct-hit lethality가 지나치게 one-shot 중심이 되지 않도록 잡는다. 첫 Wave에서는 실제 Runtime 효력이 확인된 `BaseDamage`, `FireRatePerMinute`, `InitialSpeed`, `MaxRange`, `SpreadDeg`, finite Ammo, Reload, Launcher pattern, propulsion/guidance를 주요 balance authority로 사용한다. `ArmorPenetration`, radial explosion, ModuleDamage 등은 현재 first-wave 핵심 lethality를 결정하는 값으로 사용하지 않는다.

#### 8.11.1 Cannon exact4

| Product | BaseDamage | RPM | InitialSpeed cm/s | MaxRange cm | Spread deg | Magazine / Initial | Reload s | WeaponMass kg | Ammo Max | Default Sortie | Ammo Unit kg | Projectile Life s |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `Cannon_Standard` | 25 | 60 | 9000 | 12000 | 0.35 | 8 / 8 | 3.0 | 650 | 40 | 40 | 12 | 2.0 |
| `Cannon_Heavy` | 45 | 30 | 7500 | 10000 | 0.50 | 4 / 4 | 4.5 | 950 | 20 | 20 | 22 | 2.0 |
| `Cannon_LongRange` | 25 | 45 | 14000 | 18000 | 0.10 | 6 / 6 | 3.5 | 750 | 40 | 40 | 12 | 2.0 |
| `Cannon_Rapid` | 8 | 240 | 8500 | 9000 | 0.85 | 24 / 24 | 3.8 | 700 | 120 | 120 | 5 | 1.6 |

공통:

- `FireMode = Projectile`
- `LauncherFirePattern = SingleCycle`
- `ProjectileCountPerTrigger = 1`
- `AmmoUnitsPerShot = 1`
- `bUseInfiniteAmmoForDebug = false`
- `bAffectedByGravity = true`, `GravityScale = 1.0`
- Cannon Projectile는 `bUsePropulsion = false`, Guidance/MissileFlight disabled
- Standard / LongRange는 `Damage=25`와 동일 Ammo family를 공유한다. LongRange 역할 차이는 탄속/정확도/사거리/연사에서 만든다.
- Heavy는 높은 per-shot damage와 무거운 탄약, 낮은 RPM/탄창으로 분리한다.
- Rapid는 `8 damage × 240 RPM`의 높은 sustained-pressure를 주되 spread, 짧은 사거리, 작은 per-shot damage와 높은 소비량으로 차별화한다.
- Cannon exact4의 `DefaultSortieAmmoCount`는 각각 40 / 20 / 40 / 120으로 explicit freeze한다. 첫 Wave에서는 Ammo Max와 같은 숫자를 의도적으로 사용하지만 Runtime에서 Max를 현재 수량으로 자동 대입하지 않는다.

#### 8.11.2 Unguided Rocket exact3

Shared Rocket Projectile baseline:

```text
InitialSpeed = 2200 cm/s
LifeTimeSeconds = 4.0
bAffectedByGravity = true
GravityScale = 1.0
bUsePropulsion = true
IgnitionDelaySeconds = 0.05
BurnDurationSeconds = 1.40
ThrustAccelerationCmPerSecSq = 7000
MaximumPropelledSpeed = 7000 cm/s
bUseLaunchAxisStabilization = true
MaximumThrustVectorAngleDeg = 12
LaunchAxisStabilizationResponseTimeSeconds = 0.25
MissileFlight = disabled
Guidance = disabled
```

`12 deg` TVC와 launch-axis stabilization은 active burn 중 gravity 때문에 Rocket이 단순 free-fall처럼 꺾이는 것을 줄이기 위한 first-wave baseline이며, 중력을 끄는 방식은 사용하지 않는다.

Shared Rocket Damage / Ammo:

```text
BaseDamage = 25
AmmoFamilyId = Rocket_Standard_Warhead
UnitMassKg = 18
MaximumLoadableAmmoCount = 32
bCanBeResupplied = true
```

| Product | Pattern | Count | Simultaneous | Ripple Delay | RPM | MaxRange cm | Spread deg | Magazine / Initial | Default Sortie | Reload s | WeaponMass kg |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `Rocket_Standard` | SingleCycle | 1 | 1 | 0 | 30 | 14000 | 0.35 | 6 / 6 | 32 | 4.0 | 500 |
| `Rocket_Salvo` | Salvo | 4 | 4 | 0 | 12 | 14000 | 0.55 | 8 / 8 | 32 | 5.0 | 600 |
| `Rocket_Ripple` | Ripple | 4 | 1 | 0.18 s | 15 | 14000 | 0.45 | 8 / 8 | 32 | 5.0 | 575 |

공통:

- `AmmoUnitsPerShot = 1`은 Projectile 한 발당 소비량이다.
- current Ammo runtime은 launcher sequence 시작 시 실제 Projectile 수만큼 shot을 예약하고 각 accepted Projectile마다 `AmmoUnitsPerShot`을 소비한다. 따라서 Salvo/Ripple 4발은 정상 완료 시 4 ammo units를 소비한다.
- `bUseInfiniteAmmoForDebug = false`.
- `DefaultSortieAmmoCount = 32`를 Standard / Salvo / Ripple 모두 explicit Product apply baseline으로 사용한다. 값이 우연히 `MaximumLoadableAmmoCount`와 같지만 Runtime이 Max 값을 현재 수량으로 추론하는 것은 금지한다.
- direct-hit warhead만 first-wave balance authority로 사용한다.

#### 8.11.3 Guided Missile Standard

```text
BaseDamage = 40
FireRatePerMinute = 15
InitialSpeed = 1800 cm/s
MaxRange = 18000 cm
SpreadDeg = 0.10
MagazineSize = 2
InitialLoadedAmmoCount = 2
AmmoUnitsPerShot = 1
ReloadTimeSeconds = 6.0
WeaponMassKg = 650
Ammo UnitMassKg = 25
Ammo MaximumLoadableAmmoCount = 10
DefaultSortieAmmoCount = 10
LifeTimeSeconds = 5.0
bAffectedByGravity = true
GravityScale = 1.0
```

Propulsion:

```text
bUsePropulsion = true
IgnitionDelaySeconds = 0.15
BurnDurationSeconds = 2.0
ThrustAccelerationCmPerSecSq = 8500
MaximumPropelledSpeed = 8000 cm/s
bUseLaunchAxisStabilization = false
```

Missile flight / Guidance:

```text
bUseMissileFlight = true
AttackProfile = Direct
MinimumClearanceTimeSeconds = 0.15
MinimumClearanceDistanceCm = 300
TransitionDurationSeconds = 0.35
bUseTerminalPhase = false

bUseGuidance = true
GuideMode = TargetActor
LostTargetPolicy = ContinueStraight
GuidanceLaw = ProportionalNavigation
NavigationConstant = 3.0
MaximumTurnRateDegPerSec = 70
MaximumLateralAccelerationCmPerSecSq = 9000
GuidanceResponseTimeSeconds = 0.15
MinimumGuidanceSpeedCmPerSec = 800
SeekerModel = LegacySingleGate
TargetObservationMode = DirectActorKinematics
GuidanceActivationMode = FollowFlightGuidanceWindow
SeekerFieldOfViewDeg = 60
LockBreakAngleDeg = 85
TargetLostGraceTimeSeconds = 0.25
```

Guided는 Rocket과 같은 direct-hit 값에 묶지 않고 40 damage 독립 warhead로 시작한다. CF-FQ-056에서 PFP-P0-03 USER trajectory/guidance acceptance가 완료됐으므로 current Production authoring에서는 별도 Pending gate를 유지하지 않는다.

### 8.12 Canonical Production Identity / Asset Naming Freeze

First Wave Product logical identity는 다음 exact8을 유지한다.

```text
Cannon_Standard
Cannon_Heavy
Cannon_LongRange
Cannon_Rapid
Rocket_Standard
Rocket_Salvo
Rocket_Ripple
GuidedMissile_Standard
```

Weapon Product identity rule:

```text
CCAS Weapon ContentId == UCFWeaponData.WeaponId == UCFEquipmentPresetData.EquipmentId
```

즉 `Cannon_Standard` Product의 세 값은 모두 `Cannon_Standard`이다. 현재 Catalog UI가 `EquipmentId`에서 별도의 `weapon_*` token을 생성하는 projection은 Production canonical identity로 승격하지 않으며 Production Provider/Catalog 연결 구현에서 위 identity rule로 교정한다.

Child logical IDs는 역할을 명확히 드러내며 shared Role은 shared ID를 가진다.

```text
Mount_Cannon
Mount_RocketPod
Projectile_Cannon_Standard
Projectile_Cannon_Heavy
Projectile_Cannon_LongRange
Projectile_Cannon_Rapid
Projectile_Rocket
Projectile_GuidedMissile_Standard
Damage_Cannon_Standard       # LongRange도 공유
Damage_Cannon_Heavy
Damage_Cannon_Rapid
Damage_Rocket
Damage_GuidedMissile_Standard
Ammo_Cannon_Standard         # LongRange도 공유
Ammo_Cannon_Heavy
Ammo_Cannon_Rapid
Ammo_Rocket
Ammo_GuidedMissile_Standard
```

Generated asset physical root는 Test/Legacy와 분리된 `/Game/CarFight/Weapons/Data/Production/`을 사용하고 Role별 subfolder를 둔다. deterministic filename pattern은 다음을 사용한다.

```text
EquipmentPresets/DA_Equip_<ProductId>
WeaponDefs/DA_Weapon_<ProductId>
TurretMounts/DA_<LogicalMountId>
Projectiles/DA_<LogicalProjectileId>
Damage/DA_<LogicalDamageId>
Ammo/DA_<LogicalAmmoId>
```

최초 생성 path는 pin하며 normal Workbook edit로 physical rename/move하지 않는다.

---

## 9. CCAS 입력 기준

CF-FQ-058에 넘길 Roster 입력은 숫자 목록만이 아니라 의미와 공유 관계를 함께 보존해야 한다.

### 9.1 Core metadata

모든 Weapon Variant에 최소 다음을 전달한다.

- Family
- Variant
- Gameplay Role
- Design Intent
- Relative Intent
- Base Content / comparison target
- Production Wave
- Readiness
- Technology Dependency
- Shared Data policy
- Variant-owned Data policy
- Data Role Binding set
- Role별 Required / Optional / Conditional
- Role별 Shared / VariantOwned
- Role별 Authoring Provider / binding target
- Role topology migration state

CCAS의 BaseContentId는 비교/디자인 기준으로 사용하고 implicit inheritance owner로 오해하지 않는다.

### 9.2 대량 변경에 적합한 항목

Weapon:
- Fire Rate
- Range
- Spread
- Weapon Mass
- Magazine Size
- Reload
- finite ammo policy
- Launcher Fire Pattern
- Projectile count
- launch interval
- target-use metadata

Projectile:
- Initial Speed
- Lifetime
- Gravity Scale
- Propulsion burn / thrust / max propelled speed
- Guidance turn rate / lateral acceleration / response
- Guidance activation 성능
- collision / interceptability 중 gameplay-authored scalar

Ammo:
- Ammo Family
- Unit Mass
- Maximum load
- Resupply policy

Damage:
- Base Damage
- Damage Type
- Armor Penetration
- Module damage scalar 등 현재 실제 Runtime 의미가 확인된 field

### 9.3 Resource 제작과 Workbook authoring의 경계

Mesh / Niagara / Sound / Animation 같은 binary resource 자체의 **제작**은 CCAS Workbook이 하지 않는다. 그러나 이미 존재하는 resource를 Production Product에 연결하는 것은 Workbook 정상 authoring 범위다.

따라서 아래 값은 사용자가 UE DataAsset을 열어 수동 연결하는 것이 아니라 `Resources` / `ResourceBindings`의 stable `ResourceId`와 semantic binding으로 Workbook에서 선택·참조해야 한다.

- Static Mesh / Skeletal Mesh resource
- Muzzle / exhaust socket semantic binding
- Turret visual resource와 socket role
- Niagara / impact / muzzle / thruster FX
- Sound / animation resource
- Ammo icon 등 presentation resource

고급 TargetUsePolicy / gameplay tag, Missile seeker/observation/loss-policy, attack profile, 새로운 mount type처럼 domain 의미가 복잡한 값도 **지원되는 기존 타입이면 Workbook typed field로 표현**한다. 새로운 Runtime 기능 자체를 설계해야 하는 경우에만 별도 feature work로 분리한다.

radial explosion gameplay field는 Runtime ownership이 명확해지기 전 first-wave balance authority로 취급하지 않는다.

### 9.4 CCAS 검증에 좋은 First Wave 사례

Cannon Family:
- 같은 Family에서 ProjectileData / DamageData가 Variant별로 달라지는 사례
- absolute value와 base-relative delta 비교 검증

Rocket Family:
- EquipmentPreset / WeaponData는 Variant별 분리
- Mount / Projectile / Damage / Ammo는 공유
- Launcher pattern만 Variant override
- Family default + Variant override 검증에 가장 좋은 사례

Guided Missile:
- ProjectileData inline Guidance config
- Lock / targeting dependency
- 다른 Family와 mount 또는 warhead 일부 공유
- optional config / dependency graph 검증

---

## 10. First Wave 제작 시 데이터 목표

아래 topology는 2026-10-06 기준 First Wave의 **초기 Role Binding projection**이다. 제작 중 새로운 DA 역할이 필요해지거나 기존 역할이 불필요해지는 경우 Section 8.1.1의 Flexible Data Role Composition 계약에 따라 추가/제외할 수 있으며, 이 표 자체를 고정 schema로 취급하지 않는다.

Wave 1A의 목표 데이터 topology:

Cannon Standard
→ EquipmentPreset separate
→ Cannon Mount shared candidate
→ WeaponData separate
→ Projectile separate
→ Ammo Product
→ Damage Standard

Cannon Heavy
→ 신규 Production EquipmentPreset / WeaponData identity
→ Projectile / Damage를 Heavy intent에 맞춰 Production 기준으로 구성
→ 기존 HeavyCannon의 작동 구조와 참조 관계는 authoring reference로만 사용
→ 기존 prototype numeric 값과 Product identity는 Production 기준으로 승계하지 않음

Cannon LongRange
→ EquipmentPreset / Weapon / Projectile separate
→ Cannon Mount shared candidate
→ Standard Damage 공유 여부 USER 결정

Cannon Rapid
→ EquipmentPreset / Weapon / Projectile separate
→ finite Ammo 필수
→ 낮은 per-shot damage와 높은 consumption으로 차별화
→ Recoil 없이도 identity가 성립해야 함

Rocket Standard / Salvo / Ripple
→ EquipmentPreset separate
→ WeaponData separate
→ Rocket Mount shared
→ Rocket Projectile shared
→ Rocket Ammo shared
→ Rocket Damage shared
→ Launcher pattern / count / interval이 Variant 차이

Guided Missile Standard
→ EquipmentPreset separate
→ Rocket Mount initial reuse candidate
→ Guided WeaponData separate
→ Guided Projectile separate
→ Guided Ammo separate
→ Damage 공유/분리는 warhead intent에 따라 USER 결정
→ TargetActor Lock + Guidance + CF-FQ-056 PFP-P0-03 USER ACCEPTED evidence 재사용

---

## 11. USER 승인 확정사항

### 11.0 2026-10-03 Production Product 정책 교정 — Current

USER 승인:
- First Production Wave exact8은 모두 별도 신규 Production Product identity로 제작한다.
- 기존 `HeavyCannon`은 Cannon Heavy Production anchor로 승격하지 않고 Test/Legacy reference로 유지한다.
- 기존 `RocketLauncher`는 Unguided Rocket Salvo Production anchor로 승격하지 않고 Test/Legacy reference로 유지한다.
- 기존 자산의 검증된 구조, dependency, firing pattern은 신규 Product 제작 시 참고할 수 있다.
- 기존 prototype 수치, debug ammo 상태, Product identity는 Production 기준으로 그대로 승계하지 않는다.
- Guided Missile Standard는 신규 Production Product로 함께 준비하며, 기존 PFP-P0-03 USER trajectory gate는 CF-FQ-056 USER ACCEPTED / COMPLETE로 충족됐다. Current authoring readiness는 AuthoringReady다.

아래 2026-09-30 Choice 1~5는 당시 USER Accepted planning evidence로 보존한다. 다만 Choice 1의 `HeavyCannon Production anchor`와 Choice 2의 `RocketLauncher Salvo Production anchor` 부분은 위 2026-10-03 Current 정책이 대체한다. Choice 3~5는 그대로 유효하다.

### 11.1 2026-09-30 USER 승인 — Historical Baseline

2026-09-30 USER Review에서 Choice 1~5를 제안안 그대로 승인했다. 아래 항목은 당시 CF-FQ-058에 전달한 기획 기준선이다.

### Choice 1 — Cannon Family의 기준형

USER 승인:
- Standard Cannon을 Family Base로 신규 구성한다.
- 기존 HeavyCannon은 기존 Product identity를 유지하면서 Heavy Variant anchor로 분류한다.
- 기존 HeavyCannon의 현재 prototype 수치를 Standard 기준값으로 재해석하지 않는다.

### Choice 2 — Current RocketLauncher의 의미

Fresh persisted data에서 current Product RocketLauncher는 Salvo 4발이다.

USER 승인:
- 기존 Product RocketLauncher를 Salvo Variant anchor로 사용한다.
- Standard와 Ripple을 신규 Product Variant로 계획한다.
- Test/AmmoIntegration의 Salvo/Ripple fixture는 Product 재사용 근거가 아니라 참고 evidence로만 사용한다.

### Choice 3 — First Wave Guided Missile 포함 여부

USER 승인:
- First Wave exact8에 Standard Guided Missile을 포함한다.
- Guided Missile은 Wave 1B conditional gate로 분리한다.
- PFP-P0-03 USER trajectory review 전에는 Product Accepted로 닫지 않는다.
- 따라서 First Wave의 Roster 포함과 Product Acceptance readiness는 서로 분리한다.

### Choice 4 — Finite Ammo를 첫 Product Wave의 필수 기준으로 할지

USER 승인:
- Wave 1 실제 Product 콘텐츠는 Product AmmoData + finite ammo를 기본 기준으로 한다.
- 현재 Product의 bUseInfiniteAmmoForDebug는 prototype compatibility로 취급한다.
- Ammo capacity / reload / ammo mass를 실제 Variant gameplay 차별화 축으로 사용한다.

### Choice 5 — Rocket의 범위폭발

현재 radial authored field는 존재하지만 first-wave runtime balance authority로 보기 어렵다.

USER 승인:
- Wave 1 Rocket Standard / Salvo / Ripple은 direct-hit warhead 기준으로 시작한다.
- Radial Damage Runtime이 Product-ready로 확인된 뒤 Heavy Rocket 또는 Explosive Rocket Variant를 후속 검토한다.

---

## 12. 미확정 / 후속 기술 의존

- Cannon Recoil gameplay Runtime / data ownership
- Rocket / Missile radial explosion Runtime의 실제 Product-ready 상태
- LongRange Guided Missile과 Sensor / Lock effective range의 결합 기준
- Missile Agile Variant의 실제 feel tuning
- Defensive auto-target / intercept weapon Runtime
- Scatter / multi-pellet 전용 semantics
- Utility / EMP / status effect 계열의 effect ownership
- exact8 visual resource / sound / VFX의 실제 ResourceId 선택 및 Production binding
- final feel/playtest 수치 튜닝 — initial Production balance baseline은 Section 8.11에서 frozen

---

## 13. 후속 작업 계획

2026-10-06 End-to-End Vehicle-Ready Contract까지 USER 승인되었다. exact8 Matrix 세부 공유/분리는 AI Technical responsibility로 관리하며 사용자가 exact34를 개별 검수하는 Gate를 두지 않는다. CCAS-P0-01~07의 완료된 기술 기준선은 다시 열지 않고 **Production exact8 제작과 실제 차량 적용 가능 상태를 함께 만든다.**

1. 이 v0.1.9를 Weapon Content Roster Current Production Content baseline으로 사용
2. Section 8.8의 exact48 Role binding / unique Production DA exact34를 AI Technical baseline으로 사용하되 VehicleReady를 위해 필요한 Role 변경은 해당 binding 범위에서 허용
3. Section 8.11 initial Production balance baseline과 Section 8.12 canonical identity / asset naming을 Typed Production input으로 사용
4. Workbook이 exact8 Role / Gameplay / Resource input을 충분히 표현하도록 `ContentAuthoringSystemPlan.md`의 frozen Authoring Schema를 구현
5. 기존 `HeavyCannon` / `RocketLauncher` 및 prototype child DA는 Test/Legacy reference로 유지하고 Production binding target으로 직접 사용하지 않음
6. 기존 Weapon Guide / typed authoring provider / Equipment Builder / CFDADurableCore를 재사용하는 `Typed Production Provisioning` bridge를 구현해 absent Product와 child DA를 Workbook에서 생성 가능하게 함
7. Product-level durable progress를 `Prepared → [VisibilityWithdrawn if needed] → ChildAssetsPersisted → ProductGraphVerified → RuntimeVerified → Published → VehicleReady`로 추적하고 partial durable creation은 `RecoveryRequired`로 fail-visible 처리
8. direct Product Update 또는 shared target Update가 published Product에 영향을 주면 canonical reference/dependency graph에서 reverse consumer closure를 계산해 모든 impacted Product를 어떤 durable mutation보다 먼저 generated Catalog에서 withdraw하고, 재검증 PASS Product만 republish
9. Resume 시 persisted target fingerprint가 동일 transaction desired fingerprint와 exact 일치하는 target만 idempotent skip하고 mismatch/drift는 자동 덮어쓰기 없이 Block
10. finite Product의 explicit `DefaultSortieAmmoCount`를 resolved `InitialSortieAmmoLoads`로 구성하고 기존 Fitting authority의 finite-ammo/질량 validation을 통과
11. generated EquipmentPreset과 child DA graph를 persist/readback 검증한 뒤 publication 없이 compatible Vehicle Mount에 기존 `FCFRuntimeEquipApplyService` 경로로 EquipmentPreset + resolved sortie ammo loads를 direct technical apply → Weapon/Ammo/Projectile-Damage initialization technical PASS를 확인
12. technical PASS Product만 CCAS generated `UCFProdEquipCatalogData`에 durable publish하고 Production discovery에서 동일 canonical identity + ProductGraphFingerprint로 재조회. incomplete/failed Product는 publish하지 않음
13. Production Weapon `ICFContentProvider`와 P0-07 reviewed cutover를 연결해 Product fingerprint / Workbook / Catalog / Provider / Role topology freshness를 검수
14. Guided Missile Standard는 CF-FQ-056 PFP-P0-03 USER ACCEPTED / COMPLETE evidence를 satisfied prerequisite로 사용하며 technical VehicleReady까지 같은 제작 wave에서 닫는다.
15. Workbook promotion / managed authority cutover는 fresh Production Review Package에 대한 별도 USER 승인 전까지 수행하지 않음

---

## 14. Acceptance 기준

이 Roster의 USER Acceptance는 다음 질문에 모두 답할 수 있을 때 성립한다.

- 각 Weapon Family의 존재 이유가 명확한가
- 같은 Family Variant끼리 실제 플레이 차이가 있는가
- Variant 이름만 다르고 역할이 중복되지 않는가
- 현재 기술로 가능한 것과 후속 기술이 필요한 것이 분리되어 있는가
- First Wave가 1인 개발 규모에서 감당 가능한가
- Rocket처럼 shared data + Variant override 사례가 존재하는가
- Cannon처럼 Variant-owned Projectile / Damage 사례가 존재하는가
- Guided Missile처럼 optional tech dependency 사례가 존재하는가
- CCAS가 Family / Variant / Design Intent / sharing / dependency를 검증하기에 충분한 Variety가 있는가
- 향후 Vehicle 종류가 늘어도 Mount / Size / Compatibility 확장이 가능한가

현재 판정:

Weapon Content Roster v0.1.12: CURRENT FIRST PRODUCTION WAVE BASELINE.
CF-FQ-058 Production exact8: ACTIVE / VEHICLEREADY / PUBLISHED.
Production Product policy: exact8 NEW PRODUCT preserved.
Production DA Composition policy: FLEXIBLE DATA ROLE BINDING / exact6 initial projection preserved.
End-to-End Vehicle-Ready Contract: USER ACCEPTED / cutover technical proof PASS.
Production exact8 Role Binding Matrix: Current exact48 bindings / unique Production target exact34 / direct Test-Legacy binding exact0.
Existing HeavyCannon / RocketLauncher and prototype child DAs: TEST/LEGACY REFERENCE ONLY.
Initial Production balance + canonical identity/naming: Current first-wave baseline; future changes use Workbook Change Set / Review.
Canonical Workbook: `Authoring/Content/CarFight_Content.xlsx` ACTIVE sole authoring authority for managed exact42.
Production Publication Catalog: exact8 ACTIVE.
Managed Authority Cutover: APPLIED / VERIFIED.
Next exact gate: CCAS-P0-08 USER final workflow acceptance.

---

## 15. Changelog

### v0.1.12 — 2026-10-07

- P0-08 technical Current promotion을 반영해 First Production Wave를 candidate/handoff가 아니라 Current active Product baseline으로 정리했다.
- post-cutover fresh evidence는 P0-04 5/5, P0-05 5/5, P0-06 7/7, P0-07 25/25, Weapon Guide 8/8, read-only durable cutover verification 1/1 PASS다.
- exact8 / exact48 / exact34 / approved balance/readiness 자체는 변경하지 않았다.
- 향후 Product 값 변경은 pre-cutover bootstrap builder가 아니라 active canonical Workbook Change Set → Review → Provisioning → Runtime proof → Publication 경로를 사용한다.
- exact next는 CCAS-P0-08 USER final workflow acceptance다. Git stage/commit/push는 수행하지 않았다.

### v0.1.11 — 2026-10-07

- USER Cutover Acceptance 뒤 First Production Wave exact8 managed cutover가 실제 적용됐다.
- `Cannon_Standard / Cannon_Heavy / Cannon_LongRange / Cannon_Rapid / Rocket_Standard / Rocket_Salvo / Rocket_Ripple / GuidedMissile_Standard` exact8은 canonical Workbook authority 아래 durable `VehicleReady` Product로 활성화됐다.
- exact8 Role Binding exact48 / unique Production target exact34 topology와 approved balance/readiness는 Review Package identity 그대로 보존됐다.
- generated Production Publication Catalog published membership exact8이 활성화됐고 `Authoring/Content/CarFight_Content.xlsx`가 managed manifest의 sole Current authoring authority다.
- 이 Roster는 더 이상 pre-cutover candidate 목록이 아니라 First Production Wave Current Product baseline이다. 이후 값 변경은 bootstrap builder가 아니라 canonical Workbook Change Set / Review / Provisioning 경로를 사용한다.
- stage/commit/push는 수행하지 않았다.

### v0.1.10 — 2026-10-07

- CF-FQ-058 affected Weapon Guide regression에서 disabled Trail/Thruster FX slot의 비활성 하위 field normalization mismatch가 확인되어 backend semantic fingerprint가 교정됐다.
- Roster의 Product/Role/balance/readiness 의미는 변경하지 않았으며 exact8 / exact48 / unique exact34 topology를 그대로 유지한다.
- Current Production review reference를 `ContentAuthoringSystemPlan.md v0.1.30`으로 전진시켰다. 실제 Production DA 생성, Workbook promotion, Publication mutation, managed authority cutover는 여전히 수행하지 않았다.

### v0.1.9 — 2026-10-07

- canonical CF-FQ-056 UDS의 `PFP-P0-03 USER ACCEPTED / COMPLETE / Closed`를 Current Roster truth로 반영했다.
- `GuidedMissile_Standard`는 Wave1B identity를 유지하지만 current readiness를 `AuthoringReady`로 전환하고 `PFP.P0.03.UserTrajectory Pending` dependency를 제거했다.
- Guided Missile의 historical 2026-09-30/10-03 Conditional 결정과 PFP gate는 당시 evidence/changelog로 보존하되 Current authoring/cutover blocker로 재사용하지 않는다.
- First Production Wave exact8, exact48 Role Binding, unique Production target exact34, current balance/naming/resource topology는 그대로 유지한다.
- CF-FQ-058 v0.1.30 corrected Review Package가 actual typed execution manifest exact8/exact34와 결속되므로 Roster는 그 corrected package의 Current Production input으로 사용한다.
- actual Production DA 생성, canonical Workbook promotion, Publication Catalog mutation, managed authority cutover는 이 문서 동기화에서 수행하지 않았다.

### v0.1.8 — 2026-10-06

- v0.1.7 re-review에서 발견된 finite ammo sortie input과 shared-target published impact closure를 exact8 소비 기준에 추가했다.
- exact8 모두 finite ammo이므로 `DefaultSortieAmmoCount`를 explicit Product apply field로 freeze했다: Cannon 40/20/40/120, Rocket exact3 각 32, Guided Missile 10.
- 이 값들이 first-wave에서 Ammo Max와 같더라도 `MaximumLoadableAmmoCount`를 현재 탄약량으로 추론하는 것은 금지하고 Workbook authored value → `InitialSortieAmmoLoads` projection을 사용한다.
- pre-publication Runtime proof는 기존 `FCFRuntimeEquipApplyService`를 최소 확장해 resolved sortie ammo loads를 같은 Fitting authority에 전달하도록 고정했다.
- direct Product Update 또는 shared Production DA Update가 published Product에 영향을 주면 mutation 전 impacted catalog membership을 durable withdraw하고 reverse consumer closure 전체의 ProductGraphFingerprint / Runtime proof를 재검증한 뒤 PASS Product만 republish하도록 고정했다.
- Fresh correction re-review 결과: P0 exact0 / blocking P1 exact0 / correction-local P2 exact0. exact8 ResourceId/semantic binding final values는 실제 batch authoring 준비 입력으로 남지만 bridge 설계 blocker가 아니다.
- 실제 C++/Config/Asset 구현, exact34 생성, Workbook promotion, managed authority cutover는 수행하지 않았다.

### v0.1.7 — 2026-10-06

- CCAS Production bridge 설계검수의 blocking P1 exact4를 Roster 소비 기준에도 반영했다.
- Shared Production DA는 canonical authored row exact1 / `CreateNewShared` owner exact1 / `BindShared` consumer 1..N의 single-writer 계약을 사용하고 consumer-local payload override를 금지했다.
- Product-level provisioning state를 `Prepared → ChildAssetsPersisted → ProductGraphVerified → RuntimeVerified → Published → VehicleReady`로 고정하고 partial durable creation을 `RecoveryRequired`로 분류했다.
- Resume는 persisted fingerprint가 동일 transaction desired fingerprint와 exact 일치하는 target만 skip하며 mismatch/drift에서 자동 overwrite/rebase하지 않도록 했다.
- Production publication owner를 수동 RuntimeTestCatalog/Inventory와 분리한 CCAS generated `UCFProdEquipCatalogData`로 고정했다.
- exact8 각 Product는 generated catalog에 publish되기 전에 `FCFRuntimeEquipApplyService::ApplyEquipmentRuntime` direct seam으로 compatible mount Runtime technical proof를 통과해야 한다.
- publish는 Runtime/Weapon/Ammo/Projectile technical proof 이후 마지막 visibility mutation이며 generated Catalog durable readback + 동일 canonical identity 재조회까지 성공해야 VehicleReady다.
- Matrix의 `*_W1`은 historical working key로 재분류하고 Section 8.12의 frozen canonical naming을 구현 authority로 명시했다.
- 실제 C++/Config/Asset 구현, exact34 생성, Workbook promotion, managed authority cutover는 이 문서 교정에서 수행하지 않았다.

### v0.1.6 — 2026-10-06

- current C++ Runtime/Data contract를 기준으로 exact8 initial Production balance baseline을 동결했다.
- 기본 Vehicle MaxHealth 100 scale에서 one-shot 중심을 피하도록 Cannon Standard 25, Heavy 45, LongRange 25, Rapid 8 damage와 각 RPM/탄속/사거리/탄창/재장전을 설정했다.
- Rocket Standard/Salvo/Ripple은 shared Projectile/Damage/Ammo를 유지하면서 SingleCycle/Salvo4/Ripple4 패턴과 trigger cadence로 차별화했다.
- Ammo runtime이 launcher sequence에서 Projectile별로 `AmmoUnitsPerShot`을 예약·소비하는 실제 계약을 반영했다.
- unguided Rocket은 gravity를 끄지 않고 propulsion + launch-axis stabilization으로 active-burn 궤적을 유지하는 baseline을 설정했다.
- Guided Missile Standard는 independent Damage/Ammo와 TargetActor + PN guidance baseline을 설정하고 PFP-P0-03 USER trajectory gate를 최종 gameplay acceptance에 유지했다.
- first-wave balance는 현재 실제 Runtime 효력이 확인된 BaseDamage/RPM/속도/사거리/Spread/Ammo/Reload/Launcher/Propulsion/Guidance를 authority로 삼고 radial/module/penetration 효과에 의존하지 않도록 했다.
- canonical Weapon identity를 `CCAS ContentId == WeaponId == EquipmentId` exact8로 고정하고 current `weapon_*` Catalog projection을 Production identity로 승격하지 않도록 했다.
- child logical ID와 `/Game/CarFight/Weapons/Data/Production/` deterministic generated asset naming/path 규칙을 동결했다.
- binary Resource 제작과 Workbook binding을 분리해, 기존 Mesh/FX/Sound 등의 선택·연결은 ResourceId/Semantic Binding으로 Workbook에서 수행하도록 교정했다.
- next gate를 frozen Workbook schema 구현 → Typed Production Provisioning + Production discovery/apply bridge → exact8 batch authoring으로 전진시켰다.

### v0.1.5 — 2026-10-06

- USER가 `Workbook 결과물이 즉시 차량 적용 가능해야 한다`는 전제조건을 승인하여 Roster 최상위 완료 조건으로 반영했다.
- exact8 Matrix 세부 검수를 USER Gate에서 제거하고 AI Technical responsibility로 전환했다. USER acceptance는 Workbook→Product→Vehicle Apply end-to-end 결과 중심으로 둔다.
- exact8 모두 Workbook logical input만으로 Production DA graph create/update가 가능해야 하며 수동 UE DA 교정을 정상 workflow에서 금지했다.
- 생성된 EquipmentPreset은 수동 RuntimeTestCatalog 등록 없이 Production discovery/apply 경로에 publish되어야 함을 명시했다.
- VehicleReady는 persisted reference graph, typed validation/readback, production discovery, compatible mount selection, Runtime Equip Apply, Weapon/Ammo/Projectile-Damage initialization까지 포함하도록 했다.
- Guided Missile Standard도 technical VehicleReady는 요구하고 PFP-P0-03은 최종 gameplay Product Acceptance에만 남겼다.
- 후속 순서를 balance baseline → naming/schema freeze → Typed Production Provisioning + Production discovery/apply bridge → batch authoring으로 교정했다.

### v0.1.4 — 2026-10-06

- exact8 First Production Wave에 Flexible Data Role Composition을 실제 적용한 `Data Role Binding Matrix`를 추가했다.
- Production exact8은 exact48 Role binding이며 current 공유 기준에서 unique 신규 Production DA exact34로 산출했다.
- 기존 HeavyCannon/RocketLauncher와 prototype child DA의 direct Production reuse를 exact0으로 고정하고, 공유는 신규 Production shared DA 내부에서만 수행하도록 했다.
- Cannon은 Mount exact1 공유, Projectile exact4 분리, Damage/Ammo는 Standard+LongRange 공유 및 Heavy/Rapid 분리로 구성했다.
- Rocket Standard/Salvo/Ripple은 신규 Production Mount/Projectile/Damage/Ammo exact1씩을 공유하고 EquipmentPreset/WeaponDefinition만 Variant별 분리하도록 했다.
- Guided Missile Standard는 RocketPod Mount만 initial 공유하고 Weapon/Projectile/Damage/Ammo는 독립하도록 했다.
- Launcher runtime의 SingleCycle exact1 발사 및 승인 발사 기반 muzzle 순환 계약을 근거로 Rocket family의 multi-muzzle Mount 공유 가능성을 기록했다.
- Matrix logical working key와 final Unreal asset filename/canonical ContentKey freeze를 분리했다.
- next gate를 USER Matrix Review → Production balance baseline → canonical naming freeze → batch authoring으로 전진시켰다.

### v0.1.3 — 2026-10-06

- USER 결정으로 Production Weapon의 DataAsset 구성을 고정 exact6 schema가 아니라 extensible Data Role Binding 집합으로 다루는 기준을 추가했다.
- 현재 `EquipmentPreset / Mount / WeaponDefinition / Projectile / Damage / Ammo`를 Wave 1 초기 Role projection으로 정의하고 허용 Role의 폐쇄 목록이 아님을 명시했다.
- Role binding에 Requirement, Cardinality, Ownership, ParentRole, ActivationCondition, AuthoringProvider, binding target, management/lifecycle 의미를 포함하도록 했다.
- 신규 `SeekerData` 등 DA 역할이 필요해질 경우 새 Role + Provider + Product binding으로 확장하고 Generic Core 재설계를 요구하지 않도록 했다.
- 기존 Role 제거/흡수 시 binding lifecycle migration과 물리 asset 삭제를 분리하고 `Retire != physical delete` 원칙을 유지했다.
- Workbook/CCAS composition은 고정 DA column보다 row-oriented Role Binding을 canonical 확장 모델로 사용하도록 했다.
- Section 10 exact8 topology를 고정 schema가 아닌 현재 초기 projection으로 재분류했다.
- 후속 순서를 `exact8 Data Role Binding Matrix → 실제 DA 수/dependency 확정 → balance baseline → batch authoring`으로 갱신했다.

### v0.1.2 — 2026-10-03

- USER 결정으로 First Production Wave exact8을 모두 신규 Production Product identity로 제작하는 정책으로 교정했다.
- 기존 `HeavyCannon` / `RocketLauncher`의 Production anchor 승격을 취소하고 Test/Legacy reference로 유지하도록 했다.
- 기존 prototype/test 자산은 작동 구조와 dependency reference로만 재사용하며 prototype 수치, debug 상태, Product identity는 Production 기준으로 승계하지 않도록 고정했다.
- `Cannon_Standard`, `Cannon_Heavy`, `Cannon_LongRange`, `Cannon_Rapid`, `Rocket_Standard`, `Rocket_Salvo`, `Rocket_Ripple`, `GuidedMissile_Standard` exact8을 신규 Production identity 대상으로 고정했다.
- Wave 1A exact7과 Guided Missile Wave 1B exact1을 제작 준비에서는 병렬화하되 Guided Product Acceptance의 PFP-P0-03 USER trajectory gate는 유지했다.
- CCAS Production Provider / canonical Workbook / fresh Review Package 준비를 후속 경로로 갱신하고 실제 Product mutation / Workbook promotion / authority cutover는 별도 USER 승인 전까지 금지했다.
- 2026-09-30 Choice 1~5는 Historical evidence로 보존하되 Choice 1/2의 Production anchor 결정만 Current 정책으로 supersede했다.

### v0.1.1 — 2026-09-30

- USER Review에서 Choice 1~5를 제안안 그대로 승인했다.
- Cannon Standard를 Family Base로, 기존 HeavyCannon을 Heavy Variant Product anchor로 확정했다.
- 기존 Product RocketLauncher를 Salvo Variant anchor로 확정하고 Standard / Ripple 신규 Product Variant 계획을 승인했다.
- Standard Guided Missile을 Wave 1B conditional exact1로 포함하되 PFP-P0-03 USER trajectory gate를 Product Acceptance 선행조건으로 유지했다.
- Wave 1의 Product AmmoData + finite ammo 기준을 승인했다.
- Wave 1 Rocket은 direct-hit warhead 기준으로 확정하고 radial explosion은 후속 Runtime readiness 이후로 보류했다.
- CF-FQ-058 전달 필드와 소비 순서를 확정했으며 CCAS-P0-05 USER PASS 전 P0-06 batch authoring을 열지 않도록 경계를 유지했다.
- DataAsset / Workbook / CCAS 구현 / Product Apply / Save는 시작하지 않았다.

### v0.1.0 — 2026-09-30

- Fresh repository / Systems / Plan / AssetDump 기준으로 Weapon Product와 test fixture를 재분리했다.
- 초기 Core Family를 Cannon / Unguided Rocket / Guided Missile exact3로 제안했다.
- First Production Wave를 Cannon4 + Rocket3 + GuidedMissile1 exact8로 제안했다.
- 기존 HeavyCannon을 Heavy Variant anchor, persisted Salvo RocketLauncher를 Salvo Variant anchor로 분류했다.
- Product AmmoData exact0을 반영해 finite Ammo를 Wave 1 목표로 제안했다.
- Recoil 미확인과 radial Damage Runtime 제약을 명시해 존재하지 않는 차별화 축을 확정값처럼 사용하지 않도록 했다.
- CF-FQ-058 Family / Variant / Design Intent 및 CCAS-P0-06 Batch Planning 입력 기준을 정의했다.
