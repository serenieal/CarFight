# CF-FQ-056 Projectile Flight Physics — 발사체 공통 비행 물리 계획

- 문서 버전: v0.1.7
- 작성일: 2026-09-19 / 최근 갱신일: 2026-10-06 (Asia/Seoul)
- 상태: PFP-P0-03 USER ACCEPTED / CF-FQ-056 PFP COMPLETE — Reticle RESOLVED, Cannon PASS, Guided Missile PASS, BurnedOut PASS, Rocket USER observation WAIVED by USER
- Feature: `CF-FQ-056`
- UDS Work: `wrk_f0c69a42bd174efba0e4c1d2a7b39560`
- Project: `CarFight`, `project_id=carfight`, `repository_role=main_game`, branch `SwitchSourceEngine`
- 공식 엔진: Unreal Engine 5.8 Source Build
- Lifecycle(상태·현재 Phase·정확한 Next)의 권한: `Document/UDS/records/work/wrk_f0c69a42bd174efba0e4c1d2a7b39560/`
- 현재 구현 권한: 실제 Source / 저장 Asset / `Document/Systems/Combat/Projectile.md` / `MissileGuidance.md`

---

## 1. 목적 및 사용자 확정 원칙

로켓/미사일이 연료가 남은 추진 비행 중 아래로 처지는 현상을 계기로 Cannon·Rocket·Guided Missile을 포함하는 **모든 실제 Actor형 발사체**의 비행 모델을 정비한다. **중력을 끄는 수정은 금지**하며, 발사 순간 얻은 초기 속도·비행 중 중력·연소 중 지속 추력·선택적 비행 안정화/유도 제어의 책임을 명확히 나눈다.

| 종류 | 발사 시 속도 | 비행 중 지속 추력 | 중력 | 비행 의도 |
| --- | --- | --- | --- | --- |
| 캐논 탄환 | 포구 이탈 전 가속의 결과로 이미 획득 | 없음 | 기본 적용 | 탄도 비행; 허위 모터 추가 금지 |
| 일반 로켓 | 발사 속도/런처 출구 속도 | 연소 구간에만 있음 | 기본 적용 | 실제 추력 및 물리 한계 안에서 의도한 비행축을 안정화, 유도 표적 추적은 추가하지 않음 |
| 유도미사일 | 발사 속도/런처 출구 속도 | 연소 구간에만 있음 | 기본 적용 | 기존 Guidance가 결정한 상승·수평·하강·목표 추적 방향을 실제 제어 한계 안에서 유지 |
| 비추진 일반 투사체 | 발사 시 속도 | 없음 | 기본 적용 | 탄도/관성 비행 |

- 공통 개념: `실제 가속도 = 중력 + 실제 추진 가속도 + 실제 비행제어 가속도 (+ 향후 별도 승인된 항력)`. 이 식은 개념 계약이며 구체적인 UE 5.8 ProjectileMovement 시뮬레이션·sub-step·Tick 계산은 설계 검수에서 확정한다.
- **연료가 있다는 사실만으로 중력 소거·무조건 직선·무조건 수평·무조건 상승을 강제하지 않는다.** 실제 추력/제어력이 부족하면 물리적으로 경로 이탈/하강 가능; 설계 검수에서 포화 동작 및 사용자 목표와의 타협 기준을 정한다.
- 점화 지연, 연소, 연소 종료 이후는 별도 상태로 검토한다. 연소 종료 후 지속 추력은 0이며 기존 속도와 중력이 유지된다. 미사일의 연소 후 Guidance 제어 지속 여부는 현재 구현 계약을 먼저 읽어 개별 확정한다.
- 실제 Actor가 생성되지 않는 Hitscan 및 가상 ProjectileData에 중력 비행을 강제하지 않는다.
- 기존 `bAffectedByGravity=False` 저장 데이터의 예외/시험 전용 사용 및 필요시 이관은 PFP-P0-00에서 검토한다. 기존 Product DataAsset을 일괄 덮어쓰지 않는다.

## 2. 조사된 현재 기준선 (수정 및 검증 완료 판정 아님)

2026-09-19 읽기 전용 Source·AssetDump 조사:
1. `ACFProjectileActor::ApplyProjectileMovement`가 발사 시 `bAffectedByGravity ? GravityScale : 0`으로 ProjectileMovement 중력 배율을 설정한다.
2. `UCFProjectileMotorComp`는 `IgnitionDelay → Burning → BurnedOut`을 관리하고, Rocket에는 고정 발사 방향, Missile에는 현재 속도 방향으로 가속을 더한다. `ProjectileMotorTests.cpp`의 Rocket 시험은 `bAffectedByGravity=false`로 구성되어 중력·추력 합성을 확인하지 않는다.
3. 저장 `DA_Rocket_PropTest`: 중력 영향 True, 배율 0.35, 점화 지연 0.05초, 연소 1.0초, 추진가속 9000cm/s². 저장 `DA_Missile_DirectTest`의 중력 영향은 False라 동일 시험으로 중력 중 추진 궤적을 증명하지 못한다.
4. 이 관측은 현상과 일치하는 구조적 원인 후보에 대한 **사전 코드·저장 데이터 확인**이다. 실제 PIE 궤적, 현상별 성능 수치, 수정 후 비행 결과는 아직 검증하지 않았다.

기존 완료 `CF-FQ-023` 연속 충돌, `CF-FQ-028` 추진 모터, `CF-FQ-029` Launcher, `CF-FQ-030` Missile Guidance, 기존 Targeting/Guidance Phase 5~7 Accepted 결과는 보존한다. 기존 기능 재구현·Historical Acceptance 무효화 금지.

## 3. 소유권 및 변경 경계

| 영역 | 소유 주체와 해야 할 일 |
| --- | --- |
| 초기 발사 상태 | 기존 Launcher / LaunchContext와 Projectile Actor에서 초기 속도를 한 번 확정; 캐논 추진 에너지는 여기서 표현 |
| 매 시뮬레이션 단계 중력/이동/충돌 | 기존 ProjectileMovement 및 Projectile Actor의 공통 물리·Sweep·sub-step 유지 |
| 추가 지속 추력 | 기존 ProjectileMotorComp의 연소 시간·방향·추력·속력 상한 유지·보정 |
| 비행 안정화·유도 | 기존 MissileFlight/Guide와 필요한 최소 공통 경계; 현재 Velocity 덮어쓰기·중력 이중 적용 여부 조사 |
| 발사체별 설정 | 기존 ProjectileData 중심; 새 설정은 설계 검수에서 필요성이 입증된 경우에만 추가하고 기존 값 이관/툴팁을 명시 |
| 시각효과·수명 | 기존 Thruster FX/Trail/Hit/Pool lifecycle을 물리와 독립적으로 보호 |

## 4. 단계별 승인 게이트

### PFP-P0-00 — Fresh Rebaseline + Pre-Implementation Review (정확한 다음 단계)

- 현재 canonical Checkout·dirty·실제 Source·Systems·대표 Historical Plan·저장 ProjectileData를 좁은 범위로 다시 검토한다.
- Cannon/일반 Projectile/비유도 Rocket/Guided Missile/HitScan의 LaunchContext → ProjectileMovement → Motor → Flight/Guidance → 충돌/종료/Pool까지 데이터 및 Tick/sub-step 경로를 추적한다.
- 연소 중 중력·추력·유도/안정화의 합성 방식, 수평/상향/하향 발사, 저추력 포화, 점화 지연·BurnedOut·목표 상실, 원래 `bAffectedByGravity` 예외값의 보존·전환 정책을 검수한다.
- 실제 API와 속도/가속 소유 컴포넌트를 확정한 Pre-Implementation Design Review를 남긴다. P0 또는 blocking P1이면 구현 HOLD, 교정 뒤 재검수.

### PFP-P0-01 — 공통 계약·구현 경계 Freeze (다음 단계)

- v0.1.2 PFP-P0-00 Design Re-review PASS 계약을 기준으로 실제 C++ 책임 분배, 최소 신규 타입/필드, 함수 경계와 데이터 기본값을 확정한다. 이 단계도 구현 전 설계 단계다.
- 캐논의 초기 발사 속도와 추진 0, 로켓의 제한형 Launch-Axis Stabilization, 미사일의 Guidance + Gravity-Lateral Compensation 단일 제어 예산, 연소 종료 후 전환을 구체 API 수준으로 고정한다.
- 중력 제거·`Velocity.Z=0`·속도 벡터 강제 재정규화로 중력을 소거하는 방식·제어 한도를 우회하는 별도 보정 가속은 금지한다.
- 필요 최소 필드, 기본값, 기존 Product/시험용 DataAsset 이관, Blueprint 툴팁, 구형 세이브/자산 호환과 Scope 보호를 문서화하고 PFP-P0-02 구현 범위를 freeze한다.

### PFP-P0-02 — 최소 구현 + Focused Validation

- 설계 검수에서 승인한 공통 Actor/Motor/Flight/Guide/ProjectileData 변경만 수행; Collision/Hit/Damage/FX/Pool/Launcher/Targeting 기존 Accepted 경로는 보존한다.
- Cannon 탄도, 연소 중 Rocket 수평/상향/하향과 중력·추력 합성, Guided Missile 실제 목표 추적, 점화 지연/연료 소진/최대 속도/저추력, Pool 재사용을 검증하는 좁은 Automation을 만든다.

### PFP-P0-03 — USER trajectory / reticle review

- PFP-P0-02의 공식 UE 5.8 Source Editor Build, gravity-on focused Automation과 직접 영향 회귀 TECHNICAL PASS를 기준선으로 사용하고 같은 기술 검증을 처음부터 반복하지 않는다.
- PIE에서 대표 Cannon / Rocket / Guided Missile / BurnedOut 비행을 확인하고, 실제 궤적의 체감·시각적 품질과 기존 Reticle의 `bAffectedByGravity -> LaunchDirection / DirectImpact` 의미가 새 gravity-on 기본 계약과 일치하는지 USER 검수한다.
- 물리 상태·속도/방향·Dynamics Snapshot 같은 기술 사실은 P0-02 AI Technical evidence를 유지하고, 궤적 체감·조준 표식 의미처럼 사람 판단이 필요한 항목만 USER Acceptance로 분리한다.
- USER 검수에서 실제 blocking 문제가 발견되면 그 문제만 좁게 교정하고 PFP-P0-02 구조를 임의 확장하지 않는다.

## 5. 안전·수용 기준

- 기존 단일 Active `CF-FQ-039`, Ready `CF-FQ-041`·`CF-FQ-055`, Paused `CF-FQ-054` 및 다른 UDS Work 상태를 변경하지 않는다.
- 기존 main_game / 독립 `plan_repo` dirty, 저장 Product Asset과 병렬 작업의 소유 파일을 보호한다.
- raw shell/raw Git, reset/clean/stash/rebase, stage/commit/push, 이미 완료한 기능의 광범위 재검증은 별도 지시 없이 수행하지 않는다.
- PFP-P0-02에서는 Freeze 범위의 C++/Automation만 구현했으며 당시 Product DataAsset·Blueprint·Engine Source는 변경하지 않았다. 이후 PFP-P0-03에서는 전용 `/Game/Test/CarFight/PFP/` Review fixture와 Sensor/Target Lock 운용조건 교정을 통해 USER review를 진행했다. Cannon / Guided Missile / BurnedOut은 USER가 실제 확인해 PASS했고, Rocket의 별도 실제 궤적 관찰은 USER가 현재 중요도 낮음으로 명시적으로 생략 수용했다. Rocket은 관찰 PASS로 기록하지 않고 `USER observation WAIVED / Technical PASS relied upon`으로 닫는다.

## 6. PFP-P0-00 Fresh Rebaseline 및 사전 설계 검수 — 2026-09-20

### 6.1 Fresh authority / 검수 범위

- Project Registry exact Checkout: `checkout_eefe2944e0954bed9270908eb95d8fd0`, branch `SwitchSourceEngine`. `main_game`와 독립 `plan_repo`에 기존 dirty가 있음; Source/Asset/타 Work와 Git mutation 0.
- UDS `CF-FQ-056` canonical Work는 최초 기록 `ver_2dd376a7c4e64be98b13ff504ad2c761` exact1, Ready / PFP-P0-00. 기존 `CF-FQ-039` Active, `CF-FQ-041` Ready, `CF-FQ-054` Paused, `CF-FQ-055` Ready는 그대로 유지.
- 실제 Source: `CFVehicleFireComp.cpp`, `CFLauncherTypes.h`, `CFProjectileActor.cpp`, `CFProjectileMotorComp.cpp`, `CFMissileFlightComp.cpp`, `CFMissileGuideComp.cpp`, `CFProjectileData.h`, 기존 Rocket/Missile/Launch 테스트. 현재 `Projectile.md`, `MissileGuidance.md`, `Launcher.md`는 구현 기준선만 제공하며 신규 기능 구현 완료의 증거로 사용하지 않는다.
- Fresh persisted AssetDump, root `/Game/CarFight`, class `CFProjectileData`: exact5, succeeded5, failed0, dataset `adset_v1_95c4e75fe4bea91c3a01bee8c6107d36.9c23bfc12f75fc8cdd7a6f7b`. 대표 저장값: `DA_HeavyShell` 중력 True/1.0·InitialSpeed 50000·Propulsion false; `DA_Rocket_PropTest` 중력 True/0.35·InitialSpeed 3000·Propulsion true/Delay .05/Burn 1.0/Thrust 9000/MaxSpeed 10000; `DA_Missile_DirectTest` 중력 False/0.35·InitialSpeed 2500·Propulsion true/Delay .05/Burn 4.0/Thrust 3500/MaxSpeed 8000·MissileFlight/Guidance true. 이는 저장 시험 에셋의 사실이며 실제 유저 무장의 참조 관계나 PIE 실행 상태를 증명하지 않는다.

### 6.2 실제 데이터 흐름과 확인된 책임

1. VehicleFire/Launcher: `InitialLaunchVelocity = ResolveInitialLaunchDirection * release speed + inherited carrier velocity`. 캐논/로켓/미사일의 초기 속도는 이미 공통 LaunchContext에서 관리. 실제 초기 Velocity 방향은 발사 방향과 달라질 수 있음.
2. ProjectileActor 활성화: LaunchContext 초기 속도 보존·검증·fallback, ProjectileData `bAffectedByGravity ? max(GravityScale,0) : 0`으로 ProjectileMovement 중력 설정. 비추진 MaxSpeed는 현재 초기 월드 속력, 추진 MaxSpeed는 초기 속력과 추진 최대 속력 중 큰 값.
3. Motor: Rocket `FixedLaunchDirection`, Missile `CurrentVelocityDirection`으로 `Velocity += direction * thrust acceleration * burn fraction`; 추진 후 속도 벡터 크기 clamp. `IgnitionDelay/Burning/BurnedOut` 시간 분할은 Motor Tick, 실제 중력/충돌 sub-step은 ProjectileMovement가 별도 관리.
4. MissileFlight: Released/Ejection/Clearance/GuidedFlight/Terminal과 activation window 관리. 해당 비행 상태 자체가 중력 보상이나 추진 제어력을 생산하지는 않음.
5. MissileGuide: Motor+Flight 이후, Movement 이전 Tick에서 목표 법칙/횡가속 제한/응답 필터 계산; `Velocity + filtered lateral acceleration * DeltaTime`의 방향으로 기존 속력 크기를 유지한 `Velocity`를 직접 기록. 실제 연소 여부를 선행 조건으로 검사하지 않으므로 모터 BurnedOut 이후에도 Guidance가 조건을 만족하면 유도될 수 있음. 현행 `MaximumLateralAcceleration` 및 `MaximumTurnRate` 한도는 있지만, 중력 보상량과 목표기동량이 동일 물리 제어 예산을 소비하는지 명시한 계약 없음.
6. 실제 이동: Motor → Guide → ProjectileMovement prerequisite. Motor/Guide의 프레임 단위 속도 쓰기와 ProjectileMovement 중력·Sweep/sub-step의 계산 단위가 다름. Motor와 Flight 간 상대 Tick 순서는 별도 강제되지 않음. Actor 자체 Tick이 꺼져도 Movement/Motor/Guide는 별도 컴포넌트 생명주기를 가지므로 Actor Tick에 비행 보상을 후부착하는 설계는 금지.
7. 종료/Pool: Projectile Actor는 Guide, Flight, Motor를 Reset하고 Movement를 정지/비활성화. 다음 활성화마다 ProjectileMovement의 중력/초기 속도/속력 상한을 새 ProjectileData 기준으로 재적용함.

### 6.3 Pre-Implementation Review 발견 사항

**P1-1 BLOCKING — Rocket의 중력 대응을 실제 제어 가능한 힘으로 정의하지 못함.**
현재 Rocket은 최초 월드 발사 방향으로만 추력하며 중력으로 생긴 하강 속도를 줄일 수 있는 양의 수직 제어력이 없음. 연소 중 항상 일직선/수평을 보장한다는 요구와 실제 비유도 고정 추력은 양립 불가. 단순 `GravityScale=0`, 중력만 빼고 속도 덮어쓰기, `Velocity.Z=0`, 제한 없는 가상 상향 가속 금지. 교정 계약: 로켓의 의도 비행축(초기 발사축과 차량 속도 상속의 관계 포함), 실제 가능한 thrust-vector/자세/공력 제어 기제의 소유자, 분배할 총 추력·기동 한도, 점화 지연에 누적된 하강 속도의 점진 회복, 포화 시 자연스러운 하강과 사용자 허용 오차를 확정한다. 제어 능력 없는 Rocket은 중력하 탄도·추진 곡선으로 남겨야 한다.

**P1-2 BLOCKING — Missile 중력·추력·유도 합성의 단일 물리적 예산/적용 시간이 미정.**
현재 Missile Motor는 Guide 적용 전 속도 방향으로 추진하며 Guide는 기존 속력 크기를 보존한 새 Velocity 방향을 씀. 이후 Movement가 중력을 적용한다. 중력 보상을 `filtered lateral acceleration`에 단순 가산하면 기존 횡가속/선회 한도 우회, 이중 보정, 에너지원 없는 방향 변환 또는 Motor/Guide/Movement 시간 불일치 가능. 교정 계약: 목표기동과 중력대응이 사용하는 조종면/추력편향의 실제 제어력 예산, 최대 선회율, 속력·추력 분배, sub-step 및 연소 경계 처리, Motor BurnedOut 이후의 유도권한·제어력·FX/상태 전환을 **단일 비행 물리 권한** 관점에서 결정한다. 모든 미사일은 의도된 상향·하향/종말기동을 허용한다.

**P1-3 BLOCKING — 중력 OFF 미사일 저장값과 기존 회귀의 이관·수용 기준 미정.**
현 저장 Missile 테스트 `bAffectedByGravity=false`, `CFMissileRuntimeTests.cpp`와 `CFProjectileMotorTests.cpp`의 임시 Missile/Rocket fixture도 중력 false. 현행 완료 증거를 중력 중 추진 비행의 PASS로 간주할 수 없음. 교정 계약: 실제 Product/시험/HitScan/가상 경로별 물리 적용 대상을 저장 참조 관계로 확인하고, 기본 중력 요구와 이전 에셋 호환·설정 이관·데이터 별 승인 범위·구 테스트 보존 및 신규 중력 ON focused fixture 행렬을 지정한다. 기존 시험용 에셋을 대량 자동 변경하지 않는다.

### 6.4 P0-00 현재 판정과 후속 설계 교정 Gate

```text
PFP-P0-00 Fresh Rebaseline: 완료 (소스·저장 DataAsset·기존 테스트 근거 확보)
Pre-Implementation Review: P0 0 / blocking P1 3 / P2 2 / HOLD
P2-1: Motor 프레임 시간과 Movement sub-step·중력 적용 및 MaxSpeed 속력 cap의 상호작용을 별도 집중 검증할 것.
P2-2: 기존 bAffectedByGravity 기반 조준 레티클 DirectImpact/LaunchDirection 결정이 신규 중력 기본 정책과 일치하는지 사용자 비행 검수 범위에 포함.
Implementation: NOT STARTED / PROHIBITED while blocking P1 open
Next: PFP-P0-00 Contract Correction + Design Re-review — P1-1~3 폐쇄 후 PFP-P0-01 계약 확정에 진입
```

위 P1은 기존 완료 기능의 결함 판정을 소급해 취소하는 것이 아니라 **신규 비행 물리 설계의 착수 차단 요건**이다. Game/Plugin/Engine Source, Product Asset, Git, Build, Automation, PIE mutation·execution은 이번 문서 전용 검수에서 수행하지 않았다.

### 6.5 PFP-P0-00 Contract Correction — 승인 계약

#### 6.5.1 공통 물리 계약

실제 Actor형 Cannon / Rocket / Guided Missile의 공통 기준은 다음과 같이 고정한다.

```text
InitialLaunchVelocity
+ World Gravity * GravityScale
+ bounded physical propulsion
+ bounded physical flight-control acceleration
= actual flight
```

- `InitialLaunchVelocity`는 Launcher가 계산한 사출 속도와 `InheritedCarrierVelocity`를 그대로 포함한다. 비행 제어가 발사 직후 플랫폼 상속 속도를 순간 삭제하지 않는다.
- 중력은 `ProjectileMovement`의 실제 물리항으로 항상 남는다. 중력 때문에 생기는 궤적 편차를 줄이는 경우에도 `GravityScale=0`, `bAffectedByGravity=false`, `Velocity.Z=0` 또는 중력을 계산 결과에서 빼는 방식은 사용하지 않는다.
- `GravityScale`은 게임 밸런스용 양수 배율을 허용한다. PFP 완료 이후 실제 gameplay Actor형 Cannon/Rocket/Missile의 정상 프로필은 `bAffectedByGravity=true`를 요구한다. `false`는 legacy compatibility 또는 명시적 단위시험 isolation에만 남긴다.
- Motor와 Guide가 각각 `ProjectileMovement->Velocity`를 독립적으로 수정하는 다중-writer 구조를 신규 물리 계약으로 유지하지 않는다. PFP 구현에서는 **한 번의 bounded flight-dynamics resolution이 비중력 가속 명령을 합성하고 ProjectileMovement가 중력·이동·충돌을 적용하는 single-authority 경계**를 사용한다.
- Motor burn 시간, steering/control, gravity와 movement는 동일한 bounded simulation step 의미를 가져야 한다. 프레임 단위 Motor/Guide 보정 뒤 서로 다른 ProjectileMovement sub-step이 임의로 따라오는 구조는 신규 acceptance 대상이 아니다.
- `MaximumPropelledSpeed`는 최종 월드 Velocity를 잘라내는 마법 속도 clamp가 아니라 **추진 가속을 더 이상 요구하지 않는 axial propulsion governor** 의미로 전환한다. 중력·플랫폼 속도·하강으로 생긴 월드 속도 성분을 cap 때문에 삭제하지 않는다.

#### 6.5.2 Cannon 계약

Cannon / 비추진 Projectile은 가장 단순한 기준 모델이다.

```text
ControlAuthority = None
Propulsion after release = None
Flight = InitialLaunchVelocity + Gravity
```

- 포신/발사기 안에서 받은 추진 에너지는 `InitialLaunchVelocity`로 이미 표현된 것으로 본다.
- 포구 이탈 뒤 별도 모터·중력 보상·자동 경로 안정화를 부여하지 않는다.
- 차량 속도 상속도 실제 초기 월드 속도의 일부이므로 보존한다.
- 따라서 Cannon은 PFP의 물리 기준(reference)으로 사용한다.

#### 6.5.3 P1-1 교정 — Unguided Rocket Launch-Axis Stabilization

비유도 Rocket의 제어 목적은 **표적 유도**가 아니라 발사 시 정해진 비행축을 연소 중 물리 한도 안에서 안정화하는 것이다.

- 기준축은 `LaunchContext.InitialLaunchDirection`. Target Actor, Reticle 이동, 후속 차량 조준 변경은 기준축을 바꾸지 않는다.
- `InheritedCarrierVelocity` 때문에 실제 초기 Velocity가 기준축과 다를 수 있다. Stabilization은 이를 순간 제거하지 않고 실제 제한된 제어력으로 점진적으로 수렴시킬 수 있다.
- Rocket의 연소 중 총 엔진 가속 크기는 기존 `ThrustAccelerationCmPerSecSq`를 넘지 않는다.
- 안정화는 별도 무료 가속을 더하는 것이 아니라 **엔진 추력 벡터를 제한 각도 안에서 기울이는 방식(thrust-vector control)**으로 표현한다. PFP-P0-01에서 직관적 설정 `MaximumThrustVectorAngleDeg` 또는 동등 최소 필드를 확정한다.
- 필요한 횡가속도 중 중력 대응분은 기준 비행축에 수직인 중력 성분만 대상으로 한다. 기준축 방향과 평행한 중력 성분은 자연스럽게 속력을 늘리거나 줄인다.
- 요구되는 안정화량이 추력 크기·벡터 편향 한도를 초과하면 제어는 포화한다. **포화 후 남는 하강/횡편차는 정상 물리 결과**이며 숨겨서 제거하지 않는다.
- `IgnitionDelay` 동안에는 엔진 추력과 thrust-vector stabilization이 모두 0이므로 중력이 그대로 작용한다. 점화 후 이미 누적된 편차도 제한 안에서만 점진적으로 회복한다.
- `BurnedOut` 이후 비유도 Rocket은 추진·thrust-vector control 0, 기존 Velocity + 중력의 탄도 비행으로 전환한다. 별도 aerodynamic fin model은 이번 P0 범위에 추가하지 않는다.

따라서 P1-1은 “어떤 힘이 중력을 보정하는가”와 “그 힘의 최대치가 무엇인가”가 명확해져 폐쇄한다.

#### 6.5.4 P1-2 교정 — Guided Missile 단일 횡제어 예산

Missile은 추진과 조향을 다음 두 물리 역할로 구분한다.

```text
Axial Propulsion
- Motor burning이 생산
- 현재 추진/비행축을 따라 속력을 증가
- MaximumPropelledSpeed에서 추가 axial thrust 요구를 중단

Lateral Flight Control
- Guidance law가 요구한 목표 기동
- gravity가 비행 방향을 꺾는 횡성분에 대한 compensation
- 둘을 합친 뒤 하나의 MaximumLateralAcceleration + MaximumTurnRate 예산으로 제한
```

중력 벡터 `G`와 현재 유효 비행 접선 단위벡터 `T`에 대해 설계상 횡중력은 다음 의미를 사용한다.

```text
GravityParallel = T * Dot(G, T)
GravityLateral  = G - GravityParallel
GravityCompensationRequest = -GravityLateral
```

이는 중력을 제거하는 계산이 아니다. 실제 `ProjectileMovement`에는 전체 중력 `G`가 그대로 작용하고, Missile이 가진 조향력이 그중 경로를 옆으로 휘게 만드는 성분에 대응하려는 **bounded control request**를 만드는 것이다.

- 최종 control request는 `GuidanceLateralRequest + GravityCompensationRequest`로 합성한 뒤 **한 번만** 기존 `MaximumLateralAcceleration`과 현재 속도 기반 `MaximumTurnRate` 제한을 적용한다.
- 중력 보상용 별도 reserve나 무제한 추가 가속을 만들지 않는다. 목표 기동과 중력 대응은 같은 횡제어 예산을 경쟁해서 사용한다.
- 합성 요구가 한도를 넘으면 vector sum 전체가 제한되며, 결과적으로 목표 추적 오차 또는 중력에 의한 궤적 편차가 남을 수 있다. 이 상태가 정상 saturation이다.
- 상승/하강/Terminal 경로에서도 비행 방향에 평행한 중력 성분은 취소하지 않는다. 따라서 의도적인 상승에서는 중력이 속력을 줄이고, 의도적인 하강에서는 속력을 늘릴 수 있다.
- Guide는 앞으로 **desired lateral acceleration command producer**이며 최종 Velocity writer가 아니다. Motor도 thrust availability producer이며 최종 Velocity writer가 아니다. 실제 non-gravity acceleration 적용은 PFP 단일 flight-dynamics authority가 소유한다.
- Burning 중 axial propulsion과 lateral control이 동시에 존재한다. 연소가 끝나면 axial propulsion은 즉시 0이다.
- 기존 Missile Guidance가 BurnedOut 뒤에도 동작하는 호환성을 유지하기 위해, P0 Missile의 횡제어는 **이상화한 aerodynamic lift/control authority**로 해석한다. 이는 Velocity에 수직인 bounded acceleration이므로 속력 자체를 인위적으로 증가시키지 않으며, 기존 `MinimumGuidanceSpeedCmPerSec`, `MaximumLateralAcceleration`, `MaximumTurnRate` 조건을 모두 만족할 때만 허용한다.
- 향후 “연소 중에만 조향 가능한 Missile”이 필요하면 별도 control-authority mode로 확장한다. 이번 P0에서 기존 Missile의 post-burn guidance를 일괄 제거하지 않는다.

Simulation 계약:
- Motor state transition, axial thrust, guidance command, gravity-lateral control request와 control saturation은 **동일 bounded simulation interval**을 기준으로 해석해야 한다.
- PFP-P0-01은 UE 5.8에서 이 계약을 구현할 single simulation owner/API를 확정한다. Acceptance에서 한 프레임 Motor/Guide 직접 Velocity write + 별도 Movement sub-step 조합은 허용하지 않는다.

따라서 P1-2는 중력·추력·Guidance가 어떤 예산을 공유하고 Burnout 뒤 어떤 물리 권한이 남는지가 명확해져 폐쇄한다.

#### 6.5.5 P1-3 교정 — Gravity OFF legacy / test migration matrix

2026-09-27 fresh persisted AssetDump:
- dataset: `adset_v1_090db68b0bf6ff963882e923d36f8ff1.9236a1fd9e871dd72e2fa31f`
- `CFProjectileData` exact5 / succeeded5 / failed0
- `DA_HeavyShell`: gravity True / 1.0, propulsion false — PFP Cannon 대표 후보
- `DA_Rocket_PropTest`: gravity True / 0.35, propulsion true — PFP Rocket 대표 후보
- `DA_Missile_DirectTest`: gravity **False**, propulsion+flight+guidance true — 기존 Missile compatibility fixture
- `DA_HeavyShell_DRAP50`, `DA_PFX_ThrusterTest`: gravity True이나 각각 특정 Defense/FX 목적의 보조 fixture

Migration/validation 계약:
1. 기존 `DA_Missile_DirectTest`와 중력 false transient test fixture는 **자동 수정하지 않는다**. 기존 MG/Launcher/Collision 계약을 격리 검증하는 legacy compatibility evidence로 보존한다.
2. 중력 false fixture의 PASS를 PFP 비행물리 acceptance 근거로 사용하지 않는다.
3. PFP 구현 단계에서는 gravity-on Missile 대표 fixture를 별도 생성하거나, 명시적으로 승인된 기존 fixture만 목적을 변경한다. 병렬 Accepted 자산을 조용히 덮어쓰지 않는다.
4. 최종 gameplay Actor형 Cannon/Rocket/Guided Missile은 `bAffectedByGravity=true`를 acceptance 조건으로 한다. `GravityScale`은 0보다 큰 게임플레이 배율을 허용한다.
5. HitScan과 Actor 비행이 없는 가상/표시용 ProjectileData는 위 gameplay gravity admission 대상이 아니다.
6. 신규 focused validation matrix는 최소 다음을 포함한다.
   - Cannon gravity-on / propulsion-off / control-off 탄도
   - Rocket gravity-on / Burning stabilization / control saturation / IgnitionDelay / BurnedOut ballistic
   - Missile gravity-on / level·climb·descent guidance / shared control saturation / BurnedOut guidance / below-minimum-speed control disable
   - 동일 조건의 서로 다른 frame delta/sub-step에서 허용 오차 내 궤적 일관성
   - Pool 재사용에서 gravity/control/motor runtime state 오염 0
7. 기존 gravity-off focused tests는 삭제하지 않고 isolation test로 유지한다.

따라서 P1-3은 production acceptance와 legacy isolation evidence를 명확히 분리하고 migration 방법을 정해 폐쇄한다.

### 6.6 Design Re-review

교정 계약을 기존 Source/저장 데이터/완료 기능 보호 조건과 다시 대조했다.

| Finding | Re-review |
| --- | --- |
| P1-1 Rocket 실제 제어력 미정 | **RESOLVED** — Launch-Axis Stabilization을 bounded thrust-vector control로 정의. 총 엔진 가속 초과 금지, ignition/burnout/saturation 의미 고정. |
| P1-2 Missile 합성 예산·시간 미정 | **RESOLVED** — Guidance + gravity-lateral compensation을 하나의 lateral-control budget에서 제한하고 axial propulsion과 분리. post-burn control의 물리 해석과 single simulation authority 고정. |
| P1-3 gravity-off 저장값/시험 이관 미정 | **RESOLVED** — legacy isolation 유지, gameplay gravity-on admission, 신규 gravity-on validation matrix와 자산 mutation 경계 고정. |
| P2-1 Motor frame vs Movement sub-step/MaxSpeed 상호작용 | **OPEN / non-blocking** — PFP-P0-01 single simulation owner/API freeze와 P0-02 frame/sub-step validation에서 폐쇄. |
| P2-2 gravity 기반 Reticle mode 의미 | **OPEN / non-blocking** — 비행물리 core를 차단하지 않으며 P0-03 USER trajectory/reticle 검수에 포함. |

```text
PFP-P0-00 Contract Correction: COMPLETE
Design Re-review: P0 0 / blocking P1 0 / P2 2 non-blocking
Verdict: TECHNICAL DESIGN PASS
Implementation: NOT STARTED
Next: PFP-P0-01 — Common Flight Physics Contract / C++ Responsibility / Minimal Data API Freeze
```

PFP-P0-01에서는 이 설계를 다시 바꾸는 것이 아니라, 위 계약을 C++ 책임과 최소 데이터 필드로 정확히 매핑하고 구현 범위를 freeze한다. 기존 `CF-FQ-023/028/029/030` 및 Targeting/Guidance Accepted 결과는 유지한다.

---

## 7. PFP-P0-01 Common Flight Physics / C++ Responsibility / Minimal Data API Freeze — 2026-09-27

### 7.1 Freeze 목표와 선택한 구현 축

PFP-P0-00 v0.1.2의 물리 계약을 구현 가능한 최소 C++ 경계로 고정한다. 이번 단계는 설계 Freeze이며 Source/Asset 구현은 수행하지 않는다.

UE 5.8의 기존 `UProjectileMovementComponent`는 `AddForce()`로 다음 Movement Tick의 pending force를 누적하고, `ComputeAcceleration()` 및 `ComputeVelocity()` 경로에서 이를 실제 Velocity 적분에 반영한다. 따라서 별도 custom ProjectileMovement subclass를 추가하지 않고 현재 MovementComponent를 **최종 적분·중력·Sub-step·Sweep/Collision 단일 authority**로 유지한다.

CarFight는 새 `UCFProjectileDynamicsComp` 하나를 추가해 Motor/Guidance가 만든 비중력 acceleration request를 합성·제한한 뒤 `ProjectileMovementComponent->AddForce()`에 단 한 번 전달한다.

```text
Launcher / LaunchContext
        ↓
MotorComp ----------- propulsion state/request
MissileFlightComp --- flight state/window
MissileGuideComp ---- target/guidance request
        ↓
UCFProjectileDynamicsComp
- rocket launch-axis stabilization
- missile gravity-lateral + guidance shared budget
- axial propulsion governor
- saturation / debug snapshot
        ↓  AddForce(non-gravity acceleration)
UProjectileMovementComponent
- full gravity
- velocity integration
- Sub-step
- Sweep / Collision
```

금지:
- 새 custom Movement subclass 도입
- Motor/Guide/Dynamics의 직접 `Velocity =` 또는 `Velocity +=`
- Blueprint Tick에서 Velocity/Gravity 보정
- Dynamics 외 CarFight runtime code의 ProjectileMovement `AddForce()` 사용
- gravity compensation 전용 무제한 acceleration channel

### 7.2 신규 C++ 소유 클래스

#### 신규 파일

```text
UE/Source/CarFight_Re/Public/CFProjectileDynamicsTypes.h
UE/Source/CarFight_Re/Public/CFProjectileDynamicsComp.h
UE/Source/CarFight_Re/Private/CFProjectileDynamicsComp.cpp
```

신규 클래스:

```cpp
UCFProjectileDynamicsComp
```

책임:
1. 한 World Tick의 MotorStep과 Guidance request를 읽는다.
2. 현재 ProjectileMovement의 실제 Velocity와 `GetGravityZ()` 기준 중력을 읽는다.
3. Ballistic / Rocket / GuidedMissile 모드를 결정한다.
4. Rocket은 총 engine acceleration 안에서 Launch-Axis TVC를 계산한다.
5. Missile은 Guidance request와 gravity-lateral compensation을 하나의 lateral-control budget에서 합성한다.
6. 최종 non-gravity acceleration vector를 정확히 한 번 `ProjectileMovementComponent->AddForce()`로 queue한다.
7. 최종 적용/포화 결과를 `FCFProjectileDynamicsSnapshot`에 기록한다.
8. Pool Reset 시 pending force를 정리하고 모든 runtime filter/snapshot을 기본값으로 되돌린다.

`UCFProjectileDynamicsComp`는 Blueprint에서 읽을 수 있는 상태 조회만 제공하고 BlueprintSpawnable authority로 노출하지 않는다. Projectile Actor가 정확히 하나를 기본 subobject로 소유한다.

### 7.3 Dynamics 공개 API Freeze

P0-02 구현 시 다음 의미의 API를 사용한다. 세부 const/reference 표기는 구현 중 컴파일 정합성에 맞춰도 되지만 함수 책임과 데이터 방향은 변경하지 않는다.

```cpp
void StartProjectileDynamics(
    const UCFProjectileData* InProjectileData,
    const FCFProjectileLaunchContext& InLaunchContext,
    UProjectileMovementComponent* InProjectileMovementComponent,
    UCFProjectileMotorComp* InProjectileMotorComponent,
    UCFMissileFlightComp* InMissileFlightComponent,
    UCFMissileGuideComp* InMissileGuideComponent);

void ResetProjectileDynamics();

FCFProjectileDynamicsSnapshot GetProjectileDynamicsSnapshot() const;

void AdvanceDynamicsForAutomation(float DeltaTime);
```

Runtime `TickComponent()`와 Automation 진입점은 같은 `AdvanceDynamicsSimulation(DeltaTime)`을 호출한다.

내부 책임 함수는 다음 의미로 분리한다.

```text
ResolveDynamicsMode()
ResolveWorldGravityAcceleration()
ResolveAxialPropulsionAcceleration()
ResolveRocketStabilizedThrust()
ResolveMissileLateralControl()
QueueNonGravityAcceleration()
RefreshDynamicsSnapshot()
```

이름은 위 의미를 유지하며 32자 파일/클래스 규칙을 지킨다.

### 7.4 Dynamics mode Freeze

신규 `ECFProjectileDynamicsMode`:

```text
Ballistic
Rocket
GuidedMissile
```

판정 우선순위:

1. `MissileFlightConfig.bUseMissileFlight == true`이면 `GuidedMissile`.
2. 그 외 `PropulsionConfig.bUsePropulsion == true`이면 `Rocket`.
3. 나머지는 `Ballistic`.

`GuidedMissile`이지만 현재 Guidance Command가 닫혀 있거나 Target이 무효인 경우에도 dynamics mode 자체는 Missile로 유지한다. Guidance request만 0이 될 수 있으며, 비행 안정화를 위한 gravity-lateral request는 유효 control authority가 있을 때 별도로 계산한다.

### 7.5 MotorComp 책임 교정 Freeze

기존 `UCFProjectileMotorComp`는 **Velocity writer에서 ignition/burn state + propulsion step producer로 변경**한다.

유지:
- `Inactive / Disabled / IgnitionDelay / Burning / BurnedOut`
- 점화 지연·연소 시간 상태 전이
- `OnMotorStateChanged`
- 기존 Blueprint 조회와 Debug summary
- `StartMotor`, `StartMotorWithDirectionMode`의 기존 호출 호환

제거할 runtime 책임:
- `ApplyThrustForDuration()`의 직접 `ProjectileMovement->Velocity` 변경
- 모터 자체의 최종 speed clamp

신규 값 타입 `FCFProjectileMotorStep`은 최소 다음 값을 소유한다.

```text
StepDeltaSeconds
AppliedBurnDurationSeconds
AppliedBurnFraction
ThrustAccelerationCmPerSecSq
MaximumPropelledSpeedCmPerSec
MotorStateBeforeStep
MotorStateAfterStep
bHasPropulsionImpulse
```

Motor Tick은 한 프레임에서 실제 Burning에 해당한 시간만 `AppliedBurnDurationSeconds`로 기록한다.

```text
AppliedBurnFraction =
    AppliedBurnDurationSeconds / StepDeltaSeconds
```

Dynamics는 이를 사용해 프레임 전체에 전달할 impulse-equivalent thrust acceleration을 만든다. Ignition/Burnout 경계 프레임에서도 총 `DeltaV`는 기존 실제 burn duration과 일치하도록 한다.

기존 `ECFProjectileThrustDirectionMode`는 저장/Blueprint 호환 때문에 삭제하지 않는다. PFP 이후 물리 방향 최종 authority는 Dynamics이며 이 enum은 Motor의 source-intent/debug 정보로만 사용한다.

### 7.6 Rocket 최소 Data API Freeze

기존 `FCFProjectilePropulsionConfig`에 다음 **세 필드만** 추가한다.

```cpp
bool bUseLaunchAxisStabilization = false;
float MaximumThrustVectorAngleDeg = 0.0f;
float LaunchAxisStabilizationResponseTimeSeconds = 0.25f;
```

의미:
- `bUseLaunchAxisStabilization`: 비유도 Rocket이 연소 중 Launch Axis 안정화를 사용할지 여부. 기본 False로 기존 저장 데이터 자동 행동 변경을 막는다.
- `MaximumThrustVectorAngleDeg`: 엔진 총 추력 벡터를 `InitialLaunchDirection`에서 벗어나게 할 수 있는 최대 각도. 구현 Clamp 범위는 0~45deg로 고정한다. 0이면 안정화 가속을 만들 수 없다.
- `LaunchAxisStabilizationResponseTimeSeconds`: Launch Axis에 수직인 속도 오차를 얼마나 빠르게 줄일지 정하는 응답 시간. 구현 Clamp 범위는 0.01~10s. 기본 0.25s는 기능 비활성 기본값에서는 동작에 영향 없음.

Blueprint/DataAsset 표시명은 각각:
- `발사축 안정화 사용 (bUseLaunchAxisStabilization)`
- `최대 추력 벡터 각도 deg (MaximumThrustVectorAngleDeg)`
- `발사축 안정화 응답 시간 초 (LaunchAxisStabilizationResponseTimeSeconds)`

Rocket에서 Target Actor/Reticle/Guidance는 이 세 설정에 영향을 주지 않는다.

### 7.7 Rocket 수학 Freeze

기준축:

```text
LaunchAxis = Normalize(LaunchContext.InitialLaunchDirection)
```

현재 Velocity의 발사축 수직 오차:

```text
OffAxisVelocity =
    Velocity - LaunchAxis * Dot(Velocity, LaunchAxis)
```

실제 중력의 발사축 수직 성분:

```text
GravityLateral =
    Gravity - LaunchAxis * Dot(Gravity, LaunchAxis)
```

안정화 요구:

```text
VelocityRecoveryRequest =
    -OffAxisVelocity / StabilizationResponseTime

RocketLateralRequest =
    VelocityRecoveryRequest - GravityLateral
```

단, 이 값은 별도 가속 채널이 아니다. Burning에서 사용할 수 있는 기존 `ThrustAccelerationCmPerSecSq`의 방향을 바꾸기 위한 request다.

제어 한계:

```text
MaxTvcLateralAcceleration =
    ThrustAcceleration * sin(MaximumThrustVectorAngle)
```

RocketLateralRequest를 위 한도로 제한하고, 남은 thrust magnitude만 LaunchAxis 방향 추진에 사용한다. 최종 engine acceleration magnitude는 항상 기존 `ThrustAccelerationCmPerSecSq` 이하이다.

`MaximumPropelledSpeed`는 더 이상 최종 world Velocity hard clamp가 아니다.

- LaunchAxis 방향 속도가 설정값보다 낮으면 기존 total thrust capacity 안에서 axial propulsion과 TVC stabilization을 함께 사용한다.
- 설정값 이상이면 추가 axial thrust를 0으로 governor한다.
- PFP-P0-02 구현 재검수에서 TVC가 같은 엔진 thrust vector의 axial 성분으로 governor를 우회할 수 있는 모순을 확인했으므로, axial governor가 활성인 frame에는 Rocket engine thrust vector 전체를 0으로 두고 TVC도 적용하지 않는다. 별도 lateral force channel을 만들지 않는다.
- 중력·하강·플랫폼 상속으로 world speed가 상한을 넘더라도 `Velocity` hard clamp로 잘라내지 않는다.

IgnitionDelay / BurnedOut에서는 Rocket TVC request도 0이다.

### 7.8 Missile Guidance → Dynamics handoff Freeze

`UCFMissileGuideComp`는 Target 관측·Seeker·Guidance Law와 **guidance request 생산자**로 유지한다.

PFP 이후 GuideComp가 하지 않는 일:
- ProjectileMovement Velocity 직접 변경
- Gravity compensation
- 최종 shared lateral-control budget 적용

기존 `FCFMissileGuidanceCommand.RequestedLateralAccelerationCmPerSecSq`를 Dynamics handoff의 authoritative Guidance request로 사용한다.

기존 `AppliedLateralAccelerationCmPerSecSq`와 `AppliedTurnRateDegPerSec` 필드는 Blueprint/API 호환 때문에 삭제·rename하지 않는다. 기존 pure guidance math가 계산한 **guidance-only legacy diagnostic**으로 유지하며, 실제 PFP 최종 물리 적용값은 Dynamics snapshot이 authority다.

기존 `ApplyGuidanceCommand()`는 P0-02에서 직접 Velocity 적용 책임을 제거하고 `UpdateGuidanceRequestState()` 의미로 교체한다. Guidance observation / lost policy / response state는 GuideComp가 계속 소유한다.

### 7.9 Missile shared control math Freeze

Dynamics는 시작 시 ProjectileData에서 복사한 effective MissileGuideConfig를 사용한다.

현재 유효 비행 접선:

```text
FlightTangent = Normalize(CurrentVelocity)
```

0/invalid Velocity면 LaunchAxis를 fallback한다.

중력:

```text
Gravity = FVector(0, 0, ProjectileMovement->GetGravityZ())
```

횡중력:

```text
GravityLateral =
    Gravity - FlightTangent * Dot(Gravity, FlightTangent)
```

Guide가 만든 request도 FlightTangent에 수직으로 다시 투영해 hidden axial energy를 제거한다.

```text
GuidanceLateral =
    GuidanceRequest
    - FlightTangent * Dot(GuidanceRequest, FlightTangent)
```

Guidance response filter는 Dynamics가 기존 `GuidanceResponseTimeSeconds`를 사용해 runtime state로 유지한다.

```text
FilteredGuidanceRequest =
    Lerp(PreviousFilteredRequest,
         GuidanceLateral,
         Clamp(DeltaTime / ResponseTime, 0, 1))
```

최종 횡제어 요구:

```text
CombinedLateralRequest =
    FilteredGuidanceRequest
    - GravityLateral
```

속도 기반 선회율 한도:

```text
MaxByTurnRate =
    Speed * Radians(MaximumTurnRateDegPerSec)

EffectiveMaxLateral =
    Min(MaximumLateralAccelerationCmPerSecSq,
        MaxByTurnRate)
```

`CombinedLateralRequest`를 `EffectiveMaxLateral`로 한 번만 제한한다.

따라서 Guidance와 중력 대응은 같은 control budget을 경쟁해서 사용한다.

### 7.10 Missile activation / burnout Freeze

- `bUseMissileFlight=true`이고 현재 속도가 `MinimumGuidanceSpeedCmPerSec` 이상이면 lateral flight-control authority 후보가 된다.
- Guidance window/Target이 유효하면 Guidance request + gravity compensation을 합성한다.
- Guidance window가 닫혀 있거나 Target request가 없는 순간에도 MissileFlight가 활성이고 속도 조건이 충족되면 `GuidanceRequest=0`으로 두고 gravity-lateral stabilization은 계속 허용한다. 이는 clearance / ContinueStraight에서 연료가 남은 미사일이 단순 중력 낙하하는 것을 막기 위한 **같은 제한형 flight-control authority**다.
- 속도가 MinimumGuidanceSpeed 아래면 lateral authority 전체를 0으로 하고 중력이 그대로 궤적을 지배한다.
- Burning 동안 axial propulsion은 MotorStep에서 온다.
- BurnedOut 이후 axial propulsion은 0. 기존 호환 계약대로 충분한 속도에서는 bounded aerodynamic lateral control은 유지할 수 있다.
- Terminal 상승/하강 경로의 `GravityParallel`은 취소하지 않는다.

### 7.11 AddForce / Sub-step interval Freeze

PFP의 control command interval은 **ProjectileMovement의 outer component Tick DeltaTime**으로 고정한다.

- Motor/Guide/Dynamics는 같은 PrePhysics frame의 DeltaTime으로 request를 계산한다.
- Dynamics는 최종 non-gravity acceleration을 한 번 `AddForce()`로 queue한다.
- ProjectileMovement는 그 pending force와 full gravity를 자체 `ComputeAcceleration()/ComputeVelocity()` 경로에서 적분하며 기존 Sub-step/Sweep을 그대로 사용한다.
- Motor의 ignition/burn 경계가 frame 중간에 있으면 `AppliedBurnFraction`으로 frame-average acceleration을 만들어 총 impulse를 보존한다.
- frame 내부에서 ignition 순간 위치까지 정확히 분할하는 별도 custom integrator는 이번 P0에 만들지 않는다.
- 서로 다른 frame delta와 `MaxSimulationTimeStep`에서 궤적이 허용 오차 내 수렴하는지는 PFP-P0-02 focused test acceptance로 강제한다.

이 계약으로 기존 P2-1 "Motor frame vs Movement sub-step"의 **설계 소유권은 RESOLVED**한다. 남는 것은 구현 검증이다.

### 7.12 Actor wiring / Tick ordering Freeze

`ACFProjectileActor`에 정확히 하나의 `UCFProjectileDynamicsComp`를 기본 subobject로 추가한다.

PrePhysics dependency:

```text
MotorComp ------┐
MissileFlight --┼→ MissileGuideComp
                │
MotorComp -------┐
MissileGuideComp ├→ ProjectileDynamicsComp
MissileFlight ---┘
                      ↓
              ProjectileMovement
```

기존 `ProjectileMovementComponent->AddTickPrerequisiteComponent(MotorComp/GuideComp)` 직접 연결은 Dynamics 중심으로 단순화한다.

Activation:
1. LaunchContext / ProjectileData 안전 복사.
2. ProjectileMovement 중력·충돌·초기 Velocity 설정.
3. Motor / MissileFlight / MissileGuide 시작.
4. ProjectileDynamics 시작.
5. Movement는 Dynamics가 queue한 force를 다음 Movement Tick에서 적분.

Deactivation / Pool:
1. `ProjectileDynamicsComp->ResetProjectileDynamics()`.
2. Dynamics가 `ProjectileMovement->ClearPendingForce(true)`를 호출해 이전 activation force residue를 제거.
3. Guide / Flight / Motor Reset.
4. Movement Stop/Deactivate.
5. 기존 FX/Collision/Context Pool Reset 유지.

### 7.13 Dynamics Snapshot Freeze

신규 `FCFProjectileDynamicsSnapshot`은 최소 다음 진단값을 가진다.

```text
DynamicsMode
WorldGravityAcceleration
AppliedBurnFraction
RequestedAxialAcceleration
AppliedAxialAcceleration
RocketStabilizationRequest
GuidanceLateralRequest
GravityLateralCompensationRequest
CombinedLateralControlRequest
AppliedLateralControlAcceleration
QueuedNonGravityAcceleration
bAxialGovernorActive
bRocketStabilizationActive
bLateralControlSaturated
```

모든 값은 Debug/Automation 읽기 전용이다. 게임 로직이 Snapshot 값을 다시 물리에 입력하지 않는다.

### 7.14 C++ / Blueprint 책임 Freeze

C++:
- 모든 비행 수학
- 중력 분해
- Motor state/burn fraction
- TVC 한도
- Missile shared lateral budget
- AddForce queue
- saturation
- Pool reset
- Automation

Blueprint / DataAsset:
- 기존 ProjectileData 작성
- Rocket stabilization on/off
- TVC 최대 각도
- 안정화 응답 시간
- 기존 Missile 최대 횡가속/선회율/응답 시간 튜닝
- FX/표현

Blueprint 금지:
- Event Tick 기반 Velocity 보정
- Set Velocity로 중력 취소
- Gravity Scale runtime toggle
- 별도 중력 보상 force 추가

### 7.15 P0-02 구현 범위 Freeze

신규:
- `CFProjectileDynamicsTypes.h`
- `CFProjectileDynamicsComp.h/.cpp`
- `CFProjectileDynamicsTests.cpp`

수정 가능 소유 파일:
- `CFProjectileActor.h/.cpp`
- `CFProjectileMotorTypes.h`
- `CFProjectileMotorComp.h/.cpp`
- `CFMissileGuideComp.h/.cpp`
- 필요한 최소 `CFMissileGuideTypes.h` 주석/Debug 의미
- focused tests directly affected by direct Velocity-write removal

기본적으로 수정하지 않는 영역:
- Launcher release math
- Projectile collision/damage/interception
- Missile seeker/law target-observation algorithms
- Vehicle Targeting/Sensor/HUD
- VehicleBuilder/EquipmentAuthoring/Data Authoring
- Engine Source

Asset mutation:
- 기존 `DA_Missile_DirectTest` gravity-off compatibility fixture는 그대로 보존.
- PFP 전용 gravity-on Missile fixture를 신규로 두는 방향을 우선한다.
- `DA_Rocket_PropTest` stabilization 값 변경은 P0-02에서 명시적 fixture 목적 아래 수행하고 다른 Product ProjectileData에 전파하지 않는다.

### 7.16 PFP-P0-01 Technical Review

| Review item | 판정 |
| --- | --- |
| 중력 제거 없이 공통 통합 권한 존재 | PASS — ProjectileMovement gravity 유지 + Dynamics 단일 non-gravity request |
| 다중 Velocity writer 제거 경로 | PASS — Motor/Guide는 request producer, Dynamics AddForce handoff만 허용 |
| Rocket 실제 제어력 한도 | PASS — 기존 total thrust 안의 bounded TVC, 3개 최소 data field |
| Missile Guidance + gravity shared budget | PASS — single combined lateral clamp, turn-rate limit 포함 |
| Burnout semantics | PASS — Rocket ballistic, Missile axial thrust 0 + speed-dependent bounded lateral control |
| 기존 Data/Test 호환 | PASS — gravity-off legacy isolation 유지, 별도 gravity-on acceptance |
| 기존 Collision/Sub-step 보존 | PASS — custom Movement 없이 UE ProjectileMovement 유지 |
| P2-1 frame/sub-step 책임 | RESOLVED at design — AddForce outer-tick command + impulse-preserving MotorStep; P0-02 수치 검증 필요 |
| P2-2 Reticle 의미 | OPEN / non-blocking — P0-03 USER trajectory/reticle 검수 유지 |

```text
PFP-P0-01 Contract/API Freeze: COMPLETE
Technical Review: P0 0 / blocking P1 0 / P2 1 non-blocking
Verdict: TECHNICAL CONTRACT PASS
Implementation: NOT STARTED
Next: PFP-P0-02 — Minimal C++ Implementation + Focused Validation
```

P0-02는 위 파일/책임/API 범위를 벗어나 구조를 재설계하지 않는다. 새 blocker가 실제 구현 중 발견되면 해당 blocker만 Plan에 기록하고 HOLD한다.

### 7.17 PFP-P0-02 Minimal C++ Implementation + Focused Validation — 2026-09-27

구현 결과:
- `UCFProjectileDynamicsComp`와 Dynamics Types를 신규 추가하고 `ACFProjectileActor`가 정확히 하나의 기본 subobject로 소유하도록 연결했다.
- CarFight runtime의 `ProjectileMovement->AddForce()` 사용은 Dynamics exact1로 수렴했다. `ClearPendingForce(true)`도 Dynamics Reset exact1이며 Pool 재활성화 force residue를 차단한다.
- `UCFProjectileMotorComp`는 직접 Velocity writer / world-speed clamp를 제거하고 `FCFProjectileMotorStep` producer로 전환했다. Ignition/Burnout가 한 outer frame에 걸쳐도 `AppliedBurnFraction`으로 impulse를 보존한다.
- `UCFMissileGuideComp`는 Target observation / Seeker / Guidance law / Lost policy와 `RequestedLateralAccelerationCmPerSecSq` request를 유지하되 Projectile Velocity 직접 변경, gravity compensation, 최종 shared lateral clamp를 제거했다. 기존 Applied 필드는 pure-guidance diagnostic 호환으로 유지한다.
- Rocket exact3 설정 필드를 추가했고 기본값은 False / 0deg / 0.25s라 기존 저장 ProjectileData를 자동 변경하지 않는다.
- `ProjectileMovementComponent->MaxSpeed=0`으로 world-speed hard clamp를 제거하고 `MaximumPropelledSpeed`는 Dynamics axial governor로만 해석한다.
- P0-02 Mid-review에서 Rocket axial governor 활성 중 TVC가 axial component를 다시 만들 수 있는 모순을 발견해 같은 엔진 thrust vector 전체를 0으로 제한하고 `RocketAxialGovernor` 회귀를 추가했다. 새 제어 채널이나 중력 OFF 방식은 추가하지 않았다.

최종 검증:
- Official UE 5.8 Source Editor Build `d18d4a6ee4574192bb87acf7b4819ff3`: PASS / Exit 0.
- PFP gravity-on focused Automation `047aa290d0cb4cbcbd2e60aa5cf9b1c9`: exact12 / 12 PASS / failure0 / missing0 / unexpected0 / duplicate0 / Engine Exit 0.
- focused exact12: Cannon gravity-on ballistic, Rocket gravity-on stabilization, Rocket saturation, Rocket axial governor, IgnitionDelay, BurnedOut ballistic, Missile level/climb/descent, Missile shared lateral saturation, Missile BurnedOut guidance, minimum guidance speed, frame-delta/sub-step consistency, Pool residue 0.
- 직접 영향 회귀: Projectile Propulsion `677aac224a7742eeb6a2ce26cdff6e8f` PASS; Projectile Launch `30ccc89170064567ac14547e05200f7d` PASS; Projectile FlightFX `9994084741e54082b974881d2d3d409a` PASS; Projectile SourceIsolation `47f9c51844314869974aaf4046e77b4e` PASS.
- Missile 직접 영향: Direct Runtime 전용 runner `74478513abf7460691f227a77c08ece0` PASS / Editor Exit 0 / report Success; ObservationEstimator `bfbbcaf5c51f419ca5248ae7cd0536e6` PASS; GuidanceVariantMatrix `8a64453dae574681b1929709910a6d37` PASS; GuidanceLawContract `c4ad36a1c8d44feab2f7cb7b5a1fe177` PASS; GuidanceActivationRearAspectContract `ce766cf76b264cc8bfc73d1aad879f19` PASS.
- grouped generic Missile runner에서 개별 terminal marker가 Success여도 Editor process exit가 비0인 실행이 있어 해당 grouped run은 acceptance evidence에서 제외했다. 위의 개별 Exit-0 및 전용 runner evidence만 최종 판정에 사용한다.

보호 결과:
- `DA_Missile_DirectTest`를 포함한 Product ProjectileData / Blueprint / Engine Source mutation 0.
- Launcher release math, collision/damage/interception, seeker/guidance law/target observation 알고리즘, Vehicle Targeting/Sensor/HUD, VehicleBuilder/EquipmentAuthoring/Data Authoring 변경 0.
- 기존 병렬 dirty를 정리·삭제·되돌리지 않았고 stage / commit / push 0.
- `CFMissileGuideStateTests.cpp`에는 PFP handoff에 맞춘 병렬 test-only dirty가 추가로 관측되어 현재 내용을 덮어쓰지 않고 보존했다. Product runtime authority 변경으로 확대 해석하지 않는다.

Post-Implementation Mid-review:

| Review item | 판정 |
| --- | --- |
| ProjectileMovement full gravity 유지 | PASS — gravity-on focused acceptance, gravity toggle/Velocity.Z hack 없음 |
| non-gravity acceleration 단일 권한 | PASS — runtime AddForce exact1 = Dynamics |
| Motor/Guide direct Velocity writer 제거 | PASS — 비행 중 Motor/Guide writer 0 |
| Rocket bounded TVC + axial governor | PASS — total thrust bound + saturation + governor 우회 차단 |
| Missile Guidance + GravityLateral shared budget | PASS — 단일 combined clamp + turn-rate limit |
| Burnout / minimum-speed semantics | PASS — Rocket ballistic, Missile bounded post-burn lateral, 저속 authority 0 |
| frame delta / ProjectileMovement sub-step | PASS — focused consistency 허용오차 내 |
| Pool reuse | PASS — Dynamics reset + pending force clear + residue0 |
| 기존 직접 영향 회귀 | PASS — 위 Exit-0 evidence |
| P2 Reticle 의미 | OPEN / non-blocking — PFP-P0-03 USER trajectory/reticle review |

```text
PFP-P0-02 Minimal C++ Implementation + Focused Validation: COMPLETE
Post-Implementation Mid-review: P0 0 / blocking P1 0 / P2 1 non-blocking
Verdict: TECHNICAL IMPLEMENTATION PASS
USER trajectory / reticle Acceptance: NOT STARTED
Next: PFP-P0-03 — USER trajectory / reticle review
```

### 7.18 PFP-P0-03 Reticle Technical Review + USER Trajectory Gate Audit — 2026-09-27

#### Reticle 의미 기술 검수

현재 Source와 persisted WidgetBlueprint를 다시 교차검수했다.

- `CFVehicleFireComp.cpp`에는 Legacy Weapon Preview용 `bAffectedByGravity ? LaunchDirection : DirectImpact` 생산 분기가 남아 있다.
- 그러나 현재 `Image_WeaponReticle` 제품 UI는 `ECFWeaponReticleMode` / `WeaponPreviewWorldLocation`을 소비하지 않는다.
- `UCFAimReticleWidget`은 `bHasValidTurretReticlePoint`와 `TurretReticleWorldLocation`만 소비하며, `TurretReticleWorldLocation`은 `CurrentMuzzleDirection` 기반의 터렛 조준 방향점이다.
- Source 검색에서 `WeaponReticleMode`, `bHasValidWeaponPreview`, `WeaponPreviewWorldLocation`의 현재 UI 소비자는 제품 Reticle이 아니라 `CFVehicleDebugPanelWidget`의 Legacy Debug 표시뿐이다.
- Fresh persisted WidgetBlueprint AssetDump dataset `adset_v1_5fe6aa9daf8ba8cc99537ba3bdf6cc44.89b243181cfc24fdc49872f2`은 exact22 / succeeded22 / failed0. `WBP_AimReticle`과 `WBP_CFInGameHUD`의 `bp_search_index`에도 Legacy Weapon Preview 변수 read/write가 없고, `Image_WeaponReticle`은 저장 Designer에 그대로 존재한다.
- Current Systems `UI/AimReticle.md`와 `Vehicles/VehicleAim.md`도 이미 `Image_WeaponReticle = CurrentMuzzleDirection 기반 Turret Reticle`, `DirectImpact/LaunchDirection Preview = Legacy Debug`로 기록돼 있다.

판정:

```text
PFP-P0-02 P2 Reticle 의미: RESOLVED
Current product HUD reticle mismatch: NOT FOUND
Product code / Blueprint correction required: NO
Reticle USER re-acceptance required solely for PFP: NO
```

과거 `bAffectedByGravity -> LaunchDirection / DirectImpact` 분기는 삭제할 이유가 확인되지 않았으며, 현재 제품 UI 의미를 결정하지 않는 Legacy Debug 진단값으로 보존한다.

#### USER trajectory 검수 자산 준비 상태

Fresh persisted ProjectileData AssetDump:
- dataset `adset_v1_bbad965c172cd7c373559038bfa24dd3.9d859db04589625eab327c74`
- exact5 / succeeded5 / failed0

대표 저장 상태:
- `DA_HeavyShell`: gravity True / GravityScale 1.0 / InitialSpeed 50000 / propulsion False. Cannon USER trajectory 후보로 사용 가능.
- `DA_Rocket_PropTest`: gravity True / GravityScale 0.35 / propulsion True / Delay .05 / Burn 1.0 / Thrust 9000 / MaxSpeed 10000이지만 `bUseLaunchAxisStabilization=false`, `MaximumThrustVectorAngleDeg=0`. 따라서 새 PFP Rocket stabilization USER 검수 자산으로는 부적합.
- `DA_Missile_DirectTest`: MissileFlight/Guidance/propulsion은 기존 Direct Missile 계약을 유지하지만 `bAffectedByGravity=false`. 따라서 gravity-on Guided Missile USER 검수 자산으로는 부적합.

Fresh live referencer 확인:
- `DA_Rocket_PropTest -> DA_RocketLauncher -> EquipmentPresets/RocketLauncher ->` Product/Defense/Regression 차량·레시피로 확장된다. 기존 Rocket asset 직접 수정은 좁은 PFP USER fixture 변경이 아니다.
- `DA_Missile_DirectTest -> DA_Missile_DirectWeapon -> EQ_Missile_DirectTest -> DA_Missile_TestSUV -> /Game/Maps/MissileDirectTest`. 기존 CF-FQ-030 legacy gravity-off fixture를 직접 바꾸면 Historical/Regression baseline을 훼손한다.

P0-02 focused evidence에서 review seed로 재사용 가능한 값:
- Rocket: Launch-Axis Stabilization True / Maximum TVC 30deg / response 0.25s가 gravity-on bounded TVC 경로에서 검증됨.
- Missile: gravity-on + TargetActor Guidance request와 GravityLateral shared budget 경로가 transient focused data로 검증됨.
- 이 값은 Technical test envelope이며 Product tuning 승인값으로 확대하지 않는다.

#### PFP 전용 Review DataAsset 제작 시도

승인된 UE write surface에서 Product/legacy asset을 건드리지 않고 전용 Review fixture를 만들 수 있는지 확인했다.

- `AssetTools.duplicate`: server policy blocked.
- 정식 `DataAssetTools.create`: `/Game/Test/` 하위에서 명시 승인 시 허용됨.
- `/Game/Test/CarFight/PFP/DA_PFP_RocketReview`를 메모리상 생성했으나 저장 전 상태에서 `ObjectTools.set_properties`가 scalar-only 요청까지 forward/inverse complete success contract를 만족하지 못해 전부 rollback.
- 생성된 빈 Review asset은 저장하지 않았다.
- 이 세션이 시작한 AI-owned Editor를 `carfight.editor.stop_ai_owned`로 종료해 `status=stopped_discarded_unsaved`, `save_requested=false`를 확인했고 Git status에 `UE/Content/Test` residue는 없다.
- 기존 Product/legacy DataAsset mutation = 0.

따라서 현재 승인된 UE write surface만으로는 안전한 persisted PFP Review fixture를 완성하지 못했다. raw filesystem/uasset 조작이나 임시 우회 스크립트로 안전 경계를 우회하지 않는다.

#### Managed PIE start 상태

- Editor 자체는 canonical `Tools/RunEditor.ps1`로 기동돼 UE MCP read-only 연결까지 Ready를 확인했다.
- Direct `EditorAppToolset.StartPIE` write는 server policy에 의해 차단됨.
- 공식 managed preset `carfight.pie.start` job `2d02e1f8ceee4ff9803e293131da2726`은 PIE 시작 전에 `Managed UE Bridge interpreter identity mismatch; PIE lifecycle not started.`로 실패했다.
- 따라서 Cannon / Rocket / Guided Missile / BurnedOut 실제 PIE trajectory는 이번 세션에서 실행되지 않았고 USER PASS를 주장하지 않는다.
- 이 실패는 PFP runtime code failure evidence가 아니라 PIE lifecycle infrastructure blocker다.

현재 PFP-P0-03 판정:

| 항목 | 판정 |
| --- | --- |
| Reticle `bAffectedByGravity -> LaunchDirection/DirectImpact` 의미 | RESOLVED — Legacy Debug only, current product HUD 영향 없음 |
| Cannon gravity-on USER review readiness | READY — persisted `DA_HeavyShell` 사용 가능 |
| Rocket PFP USER review readiness | BLOCKED — persisted Rocket은 stabilization OFF; 전용 fixture 저장 경로 미확보 |
| Guided Missile gravity-on USER review readiness | BLOCKED — legacy persisted missile은 gravity OFF; 전용 fixture 저장 경로 미확보 |
| BurnedOut USER review readiness | BLOCKED with Rocket fixture |
| Managed PIE execution | BLOCKED — interpreter identity mismatch before PIE |
| PFP-P0-02 Technical PASS | PRESERVED / NOT REOPENED |
| USER trajectory Acceptance | NOT RUN |

```text
PFP-P0-03 Reticle Technical Review: COMPLETE / RESOLVED
PFP-P0-03 USER Trajectory Review: NOT RUN
Blocker class: Validation Fixture + PIE Lifecycle Infrastructure
Product implementation regression: NOT ESTABLISHED
Next: recover an approved review-fixture authoring path and managed PIE start, then run USER trajectory review only.
Do not replay PFP-P0-00~02 unless the trajectory review reveals a concrete PFP runtime defect.
```

### 7.19 PFP-P0-03 USER Trajectory Review Current Checkpoint — 2026-10-06

PFP-P0-02 Technical PASS와 7.18 Reticle Technical RESOLVED를 보존한 상태에서 실제 USER trajectory review를 진행했다.

USER 확인 결과:

| 항목 | USER 판정 | 비고 |
| --- | --- | --- |
| Cannon gravity-on trajectory | **PASS** | `DA_HeavyShell` 대표 탄도 비행 확인 |
| Guided Missile gravity-on / TargetActor guidance | **PASS** | Vehicle Target Lock 확보 후 의도적으로 표적과 어긋난 방향으로 발사해도 실제 Lock 대상 방향으로 유도 선회하는 것을 USER 확인 |
| BurnedOut transition | **PASS** | 연료 소진 후 추진이 끝나고 중력에 의해 자연스럽게 추락하는 전환 확인 |
| Rocket gravity-on / launch-axis stabilization | **USER OBSERVATION WAIVED** | 별도 실제 궤적 관찰은 USER가 현재 중요도 낮음으로 생략 수용. PFP-P0-02 Technical PASS를 유지하며 관찰 PASS로 허위 승격하지 않음 |

Guided Missile 검수 과정에서 PFP core와 별개인 선행 운용조건 문제 exact2를 확인·교정했다.

1. Production Basic Sensor가 Passive 20m / Visual 30m / Active 40m여서 600m Guided Missile 사거리보다 Live Contact 범위가 지나치게 짧았고, 조금만 멀어져도 Vehicle Lock이 Break되거나 신규 Lock이 거부됐다. 현재 `DA_VehicleSensor_Basic`은 Passive 600m / Visual 800m / Active 1200m, Radar 300/600/1200m(기본 600m)로 교정됐다.
2. T Lock 입력이 기존 Lock 상태에서 무조건 clear하던 의미를 교정해, 같은 Selected Target은 토글 해제하고 다른 유효 Selected Target은 기존 `RequestLock()` replacement 계약으로 즉시 재획득하도록 했다. 새 Target Lock 요청이 실패하면 기존 Lock은 보존한다.

위 교정 후 USER가 Vehicle Target Lock 유지와 Guided Missile 실제 유도 선회를 모두 확인했다. 따라서 이전의 "locked target인데 straight flight"는 현재 PFP Guidance Runtime FAIL로 판정하지 않는다.

차량선택 HUD의 Lock 대상 표현은 USER가 현재 부족하다고 평가했으나 **PFP 비행물리 Acceptance blocker가 아니다.** 이 항목은 별도 Vehicle Selection HUD 세션에서 간단한 Presentation 보강 작업으로 분리하며, PFP-P0-03에서는 HUD 확장을 수행하지 않는다.

현재 판정:

```text
PFP-P0-02 Technical Implementation: PASS / PRESERVED
PFP-P0-03 Reticle Technical Review: COMPLETE / RESOLVED
Cannon USER trajectory: PASS
Guided Missile USER trajectory: PASS
BurnedOut USER transition: PASS
Rocket USER trajectory: OBSERVATION WAIVED BY USER / TECHNICAL PASS RELIED UPON
Vehicle Selection HUD feedback: DEFERRED / NON-BLOCKING / separate HUD session
PFP-P0-03 USER Acceptance: COMPLETE
CF-FQ-056 Projectile Flight Physics: COMPLETE
Next: none in PFP. Reopen only if a concrete projectile-flight regression is observed.
```

### 7.20 PFP-P0-03 USER Acceptance Closure — 2026-10-06

USER는 남은 Rocket gravity-on / launch-axis stabilization 실제 궤적 관찰을 현재 중요도 낮음으로 판단해 별도 확인 없이 수용하고 다음 작업으로 넘어가기로 결정했다.

이 결정은 Rocket을 `USER observed PASS`로 소급 기록하는 것이 아니다. Rocket은 PFP-P0-02의 gravity-on/stabilization/saturation/axial-governor Technical PASS를 근거로 유지하고, PFP-P0-03의 별도 관찰 항목만 `USER OBSERVATION WAIVED`로 닫는다.

따라서 PFP-P0-03은 USER ACCEPTED로 종료하며 CF-FQ-056 Projectile Flight Physics는 현재 범위에서 COMPLETE다. 이후 실제 플레이에서 구체적인 Rocket/Projectile Flight regression이 확인될 경우에만 해당 결함 범위로 재오픈한다.

---

## 8. Changelog

### v0.1.7 — 2026-10-06
- USER가 남은 Rocket 실제 궤적 관찰을 현재 중요도 낮음으로 명시적으로 생략 수용했다.
- Rocket을 관찰 PASS로 허위 기록하지 않고 `USER OBSERVATION WAIVED / PFP-P0-02 Technical PASS relied upon`으로 구분했다.
- PFP-P0-03 USER Acceptance를 COMPLETE로 닫고 CF-FQ-056 Projectile Flight Physics를 현재 범위에서 COMPLETE로 정리했다.
- 이후 재오픈 조건은 구체적인 Projectile Flight regression 관측으로 제한한다.

### v0.1.6 — 2026-10-06
- PFP-P0-03 USER trajectory review를 실제 진행해 Cannon gravity-on, Guided Missile TargetActor guidance, BurnedOut transition을 USER PASS로 기록했다.
- Guided Missile 검수 중 확인된 Basic Sensor 운용거리 불일치와 T Lock retarget 입력 의미를 별도 Targeting/Sensor 책임 범위에서 교정했고, 교정 후 USER가 Lock 유지와 실제 유도 선회를 확인했다.
- Vehicle Selection HUD의 Lock 대상 표현 부족은 PFP blocker가 아닌 Presentation 후속으로 분리해 별도 HUD 세션으로 이관했다. PFP에서는 HUD mutation을 수행하지 않는다.
- 당시 남은 USER 항목은 Rocket gravity-on / launch-axis stabilization 대표 trajectory review exact1이었으며, v0.1.7에서 USER가 별도 관찰을 생략 수용해 `USER OBSERVATION WAIVED`로 종료했다.

### v0.1.5 — 2026-09-27
- PFP-P0-03 Reticle 의미를 fresh Source + persisted WidgetBlueprint + Current Systems로 재검수해 기존 `bAffectedByGravity -> LaunchDirection/DirectImpact`가 current product HUD가 아닌 Legacy Debug임을 확정했다. PFP-P0-02의 Reticle P2를 RESOLVED로 닫았고 Product Reticle code/Blueprint mutation은 0이다.
- Fresh persisted ProjectileData exact5에서 Cannon은 USER review 준비 완료, Rocket은 stabilization OFF, Missile은 gravity OFF임을 확인했다. 기존 Rocket/Missile fixture의 referencer 범위를 확인해 기존 자산을 직접 변경하지 않았다.
- 전용 `/Game/Test/` Review DataAsset 제작은 생성 후 property write 완전성 계약에서 rollback됐고 unsaved asset은 AI-owned Editor stop으로 폐기했다. 저장 신규 asset과 Product/legacy asset mutation은 0이다.
- managed `carfight.pie.start`가 PIE 시작 전 UE Bridge interpreter identity mismatch로 실패해 USER trajectory는 NOT RUN으로 유지한다. PFP-P0-02 Technical PASS는 보존하며 다음은 review fixture authoring + managed PIE lifecycle 복구 후 USER trajectory review만 재개한다.

### v0.1.4 — 2026-09-27
- PFP-P0-02 최소 C++ 구현을 완료했다. `UCFProjectileDynamicsComp`를 sole non-gravity AddForce authority로 추가하고 Motor는 impulse-preserving step producer, Guide는 guidance request producer로 전환했으며 ProjectileMovement full gravity/sub-step/sweep/collision을 유지했다.
- Official UE 5.8 Build PASS, gravity-on focused exact12 12/12 PASS와 Projectile/Missile 직접 영향 Exit-0 회귀 PASS를 확보했다. Product DataAsset/Blueprint/Engine Source mutation은 0이고 `DA_Missile_DirectTest`는 legacy gravity-off fixture로 보존했다.
- Mid-review에서 Rocket axial governor와 TVC의 모순을 최소 교정해 governor 활성 시 같은 엔진 thrust vector 전체를 0으로 제한하고 회귀를 추가했다. Post-Implementation Mid-review P0 0 / blocking P1 0 / P2 1 non-blocking, TECHNICAL IMPLEMENTATION PASS. 다음은 PFP-P0-03 USER trajectory/reticle review다.

### v0.1.3 — 2026-09-27
- PFP-P0-01에서 새 `UCFProjectileDynamicsComp`를 단일 CarFight non-gravity acceleration 합성 권한으로 Freeze했다. 기존 `UProjectileMovementComponent`는 중력·Velocity integration·Sub-step·Sweep/Collision authority로 보존하며 Dynamics만 `AddForce()`를 사용한다.
- MotorComp는 propulsion step producer, MissileGuideComp는 guidance request producer로 책임을 축소하고 직접 Velocity write를 제거하는 P0-02 구현 경계를 확정했다. Rocket 최소 신규 필드는 `bUseLaunchAxisStabilization`, `MaximumThrustVectorAngleDeg`, `LaunchAxisStabilizationResponseTimeSeconds` exact3으로 고정했다.
- Rocket TVC 수학, Missile shared lateral-control 수학, Burnout, AddForce outer-tick/sub-step 의미, Pool pending-force reset, C++/BP 책임과 P0-02 파일 범위를 Freeze. Review P0 0 / blocking P1 0 / P2 1 non-blocking, TECHNICAL CONTRACT PASS. 구현·Build·Automation·PIE는 아직 미수행.

### v0.1.2 — 2026-09-27
- PFP-P0-00 blocking P1 3건 Contract Correction + Design Re-review 완료. Rocket은 bounded thrust-vector Launch-Axis Stabilization, Missile은 Guidance + gravity-lateral compensation 단일 횡제어 예산, gravity-off 기존 Missile/테스트는 legacy isolation으로 분리했다.
- Fresh ProjectileData AssetDump exact5로 기존 저장 상태를 재확인하고 gameplay Actor형 발사체의 gravity-on admission 및 신규 focused validation matrix를 확정했다.
- Re-review 결과 P0 0 / blocking P1 0 / P2 2 non-blocking, Technical Design PASS. 구현·Source/Asset mutation·Build/Automation/PIE/Git mutation은 수행하지 않았으며 다음은 PFP-P0-01 API/책임 Freeze다.

### v0.1.1 — 2026-09-20
- Fresh Source/Current Systems/기존 focused test 및 persisted ProjectileData exact5 재기준점 조사 완료. Rocket 물리적 제어력, Missile 단일 제어 예산·sub-step·burnout, 중력 비활성 기존 미사일 데이터의 이관·검증을 blocking P1 3건으로 분리해 PFP-P0-00 설계 HOLD로 기록했다.
- 구현/테스트/PIE/Asset 변경 0, 기존 Accepted 결과와 Active/Ready/Paused Work 유지. 다음은 PFP-P0-00 Contract Correction + Design Re-review.

### v0.1.0 — 2026-09-19
- 사용자 승인에 따라 모든 실제 발사체의 초기 속도·중력·추력·선택적 비행제어를 정비할 신규 `CF-FQ-056` 대표 Plan 등록.
- 중력 OFF 임시처방 배제, 캐논/로켓/미사일 역할 분리 및 PFP-P0-00~03 Gate, 기존 Work·기능 보호 정의.
- 상태: Promoted / Design Review Pending / Implementation Not Started.
