# CarFight — 03_FeatureQueue

> 문서 버전: v1.54.38


> 작성일(Asia/Seoul): 2026-08-18
> 문서 상태: Active
> 역할: CarFight의 **기능 후보 / 착수 판단 / 클라이언트·서버·관리툴 필요성**을 한 곳에서 관리한다.

---

## 1. 목적

이 문서는 CarFight에서 앞으로 개발할 기능 후보를 기능 단위로 정리한다.

이 문서는 상세 설계서가 아니다.
상세 설계는 기능 착수 후 `Document/Plan/<기능명>/`에 작성한다.

이 문서의 목적은 아래 판단을 돕는 것이다.

```text
- 다음에 어떤 기능을 착수할지
- 이 기능이 싱글 / 로컬 플레이 기준으로 검증 가능한지
- 서버 권한 구조가 현재 범위에서 제외되어야 하는지
- 관리툴 또는 임시 운영 도구가 필요한지
- 완료 후 어떤 Systems 문서로 승격할지
```

---

## 2. 문서 위치와 생명주기

CarFight 문서 흐름은 아래 기준으로 본다.

```text
기능 후보
  -> Document/ProjectSSOT/03_FeatureQueue.md

개발 착수 / 상세 계획
  -> Document/Plan/<기능명>/

개발 완료 / 현재 구현 기록
  -> Document/Systems/<분류>/<기능명>.md

오래된 계획 / 완료된 계획 원본
  -> Document/Plan/Archive/
```

중요:
- `FeatureQueue`는 상세 구현 계획을 쓰지 않는다.
- `Plan`은 앞으로 개발할 기능의 상세 계획만 담는다.
- `Systems`는 개발 완료된 기능의 현재 구현 기준만 담는다.

---

## 3. 상태 표기

| 상태 | 의미 |
|---|---|
| `Candidate` | 후보. 아직 착수하지 않음 |
| `Ready` | 설계와 선행 조건이 준비됐지만 현재 단일 Active 작업은 아닌 상태 |
| `Active` | 현재 Plan에서 진행 중 |
| `Paused` | 진행 체크포인트를 보존한 채 다른 주력 작업으로 일시중지 |
| `Blocked` | 선행 조건 때문에 막힘 |
| `Done` | 구현 완료. Systems 문서로 승격됨 |
| `Deferred` | 보류 |
| `Rejected` | 기각 |

---

## 4. 우선순위 표기

| 우선순위 | 의미 | 처리 기준 |
|---|---|---|
| `P0` | 없으면 다음 개발/검증이 막힘 | 최우선 착수 후보 |
| `P1` | 핵심 게임 루프에 필요 | 가까운 사이클에서 착수 |
| `P2` | 생산성/운영/품질 개선 | 반복 비용이 커질 때 착수 |
| `P3` | 편의/폴리싱 | 핵심 루프 이후 |
| `Icebox` | 아이디어 보관 | 당장 계획하지 않음 |

---

## 5. 기능 후보 큐

| ID | 기능 | 목적 | 우선순위 | 상태 | 클라 | 서버 | 관리툴 | 완료 후 Systems 위치 |
|---|---|---|---|---|---|---|---|---|
| `CF-FQ-011` | 싱글 실행 기준선 전환 | 서버 GameMode 전제 없이 PIE 1인 플레이에서 기준 차량 1대를 바로 조작 가능하게 만들기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Config/ProjectRuntimeConfig.md`, `Document/Systems/Vehicles/VehicleRuntime.md` 갱신 |
| `CF-FQ-012` | 1대 차량 주행감 고도화 | `DA_PoliceCar` 기준 전진/후진/조향/브레이크/핸드브레이크 감각을 전투 루프 안에서 조정 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleDrive.md`, `Document/Systems/Vehicles/VehicleSteering.md` 갱신 |
| `CF-FQ-013` | 카메라/로컬 조준 고도화 | 서버 판정 없이 차량 카메라, Local Aim, Reticle 피드백을 싱글 기준으로 정리 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleCamera.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-014` | WheelSync 시각 품질 폴리싱 | 고속 휠 스핀/조향/서스펜션 시각 품질을 기능 FAIL과 품질 후속으로 분리하고 개선 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/WheelSync.md` 갱신 |
| `CF-FQ-015` | 차량 데이터 튜닝 패스 | 기준 차량 1대의 Movement/Wheel/DriveState 값을 추적 가능한 데이터 기준으로 정리 | `P1` | `Paused` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleData.md` 갱신 |
| `CF-FQ-016` | 차량 무기 조준 및 발사 | 기준 차량 1대에서 로컬 조준 상태를 실제 발사 경로로 연결하고 WeaponFire 결과를 기록 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/VehicleDebugPanel.md` 갱신 |
| `CF-FQ-017` | Reticle / FireFeedback UI 구현 | WeaponFire 결과를 Reticle 색상, 상태 문구와 쿨다운 UI로 읽히게 만들기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/FireFeedback.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-018` | 피격 판정 및 피해 처리 | 시각 차체 기반 피격 콜리전 구현 결과를 유지하면서 신뢰 가능한 HitContext를 실제 피해 누적으로 연결하기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/HitDamage.md` |
| `CF-FQ-022` | 조준점·터렛·총구 정렬 | Reticle 월드 목표점과 실제 발사 해를 통합하고, `TurretMountData`별 정렬 중 발사 허용 정책을 선택 가능하게 만들기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-023` | 고속 Projectile 연속 충돌 | Sweep/Sub-stepping과 보조 Sphere Sweep으로 고속 발사체 터널링을 방지하고 신뢰 가능한 HitContext를 보장하기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Projectile.md`, `Document/Systems/Combat/DamageHitContext.md` 갱신 |
| `CF-FQ-025` | 이중 레티클 및 터렛방향 시각화 | Image_CenterDot 조준 레티클과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클을 분리하고 탄종·착탄 위치와 독립적으로 표시 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/UI/AimReticle.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/VehicleDebugPanel.md` 갱신 |
| `CF-FQ-026` | 타겟 선택 시스템 | 직접 조준 우선과 크로스헤어 근접 후보를 기반으로 지속 선택 대상을 만들고 HUD·센서·유틸리티 장비가 공통으로 조회할 기반을 구현 | `P1` | `Paused` | 필요 | 불필요 | 불필요 | `Document/Systems/Targeting/TargetSelect.md`, 관련 UI·장비 Systems 문서 갱신 |
| `CF-FQ-024` | 전투 FX 구현 | 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 Niagara 기반 시각 연출로 1회씩 표현하기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/CombatFx.md`, 기존 Combat Systems 문서 갱신 |
| `CF-FQ-027` | 투사체 비행 FX | Projectile Trail과 추진 화염을 `ACFProjectileActor` Pool 생명주기에 연결하고 메시 소켓 우선·ProjectileData Fallback 부착을 제공 | `P1` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Projectile.md` |
| `CF-FQ-028` | 발사체 추진 시스템 | 비유도 Rocket이 InitialSpeed로 분리된 뒤 점화 지연·고정 방향 가속·연소 종료 후 관성 비행하고 실제 Burning 상태에서만 Thruster FX를 재생하게 만들기 | `P1` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Projectile.md` |
| `CF-FQ-029` | 모듈형 런처 및 발사 인계 | 가변 Muzzle, Single·Ripple·Salvo와 Direct·Angled·Vertical 사출을 공통 Launch Context로 연결하고 기존 직사 Projectile을 호환 유지하기 | `P1` | `Paused` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Launcher.md`, `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/Combat/Projectile.md` 갱신 |
| `CF-FQ-030` | 물리 제한형 미사일 비행·유도 | 런처에서 분리된 미사일이 자체 추진·비행 전환과 제한된 선회율·횡가속도 유도로 목표 추적을 시도하되 명중을 보장하지 않게 만들기 | `P1` | `Ready` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Missile.md`, `Document/Systems/Combat/Projectile.md` 갱신 |
| `CF-FQ-031` | 차량 탄약·재장전 런타임 | 현재 출전 차량의 실제 사용 가능 탄약을 무기별 장전량과 탄종별 예비량으로 관리하고, 발사 소비·런처 예약·행동 잠금·재장전·HUD·피팅 중량을 하나의 Runtime 계약으로 연결하기 | `P1` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Ammo.md v1.0.0` Current / 관련 WeaponFire·Launcher·UI·Vehicle Systems는 후속 실제 변경 시 연계 |
| `CF-FQ-032` | 인게임 전투 HUD 및 UI 프레임워크 | 인게임 UI를 LocalPlayer 소유 Root와 레이어로 관리하고 완전 Pause, 외부 3인칭 HUD, Radar, 차량 방어·무기 자원·타겟 지식 표시를 구현하면서 향후 전체 게임플로우 확장을 열어두기 | `P1` | `Active` | 필요 | 불필요 | 불필요 | `Document/Systems/UI/InGameHUD.md`, `Document/Systems/UI/UIFlow.md`, 관련 Combat·Targeting·Vehicle Systems 갱신 |

| `CF-FQ-033` | 차량 방어·손상 런타임 | 기존 최소 HitDamage 앞에 쉴드, 6방향 독립 장갑, 관통과 차량 내구도 피해 분배를 추가하고 기존 에셋·Health 이벤트 호환을 유지하는 P0 방어 본체 구축 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/VehicleDefense.md`, `Document/Systems/Combat/HitDamage.md` |
| `CF-FQ-034` | 차량 피팅·질량 런타임 | VehicleData의 하드포인트·MountProfile과 소유 장비·Ammo·VehicleDefenseData를 검증 Snapshot으로 조합하고, 출격 적용과 비전투·정지·쿨타임 완료 조건의 시간 소모 필드 장착·해제를 원자적으로 연결하기 | `P1` | `Paused` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleFitting.md`, `Document/Systems/Vehicles/VehicleData.md`, `Document/Systems/Vehicles/VehicleRuntime.md`, 관련 Combat·Inventory·UI Systems 갱신 |
| `CF-FQ-035` | 인벤토리 Foundation | 실제 소유 Item Instance, VehicleCargo·Mounted 소유 상태, 접근 조회, Reservation과 Atomic Transfer를 제공해 필드 피팅과 향후 탄약·루팅·보상·저장의 공용 소유권 기반 만들기 | `P1` | `Paused` | 필요 | 불필요 | 불필요 | `Document/Systems/Inventory/InventoryFoundation.md`, 관련 Vehicle·Combat·UI Systems 갱신 |
| `CF-FQ-036` | 차량 센서·Contact Intelligence Runtime | 실제 Sensor Provider를 구현해 Passive/Active 탐지, Contact 수명, Target Knowledge와 read-only Snapshot을 TargetSelect·HUD·향후 Missile/Utility가 공통 소비할 기반 만들기 | `P1` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Targeting/SensorContact.md v1.1.0` Current owner에 CF-FQ-037 Scanner 통합 반영 |

| `CF-FQ-037` | 차량 스캐너 입력·장비 통합 | 완료된 Sensor Runtime에 실제 Scanner 장비/설정 Source와 플레이어 Active Scan 입력을 연결하고, 차량 초기화·장비 변경에서 검증된 SensorConfig를 적용하는 경로를 만든다 | `P1` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Targeting/SensorContact.md v1.1.0` Current |

| `CF-FQ-038` | 차량 데이터 Authoring 시스템 | VehicleData의 117 leaf를 Raw 직접 입력하지 않고 Editor-only Recipe·5 Profile·Resolver·Source Trace·Diff·Validation·AI/Batch 계약으로 제작·비교·검증·Apply하는 Authoring 기반을 구축하기 | `P2` | `Paused` | 불필요 | 불필요 | 필요 | 완료 후 `Document/Systems/Vehicles/VehicleDataAuthoring.md` 신규 승격 후보 / VehicleData Current System은 실제 완료 전 유지 |

| `CF-FQ-019` | 주행/전투 반복 테스트 | 주행 중 조준/발사/피격/피해와 시각 FX가 반복되는지 PIE 기준으로 검증하되, 런처·미사일 구현 범위가 확정된 뒤 통합 회귀 범위를 다시 설계 | `P1` | `Deferred` | 필요 | 불필요 | 불필요 | `Document/ProjectSSOT/05_TestChecklist.md`, `Document/Systems/Combat/CoreLoop.md` 갱신 |
| `CF-FQ-020` | 조작감/전투 템포/피드백 개선 | 조작감, 발사 리듬, 피격 반응, UI와 시각 피드백을 핵심 루프 기준으로 조정 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/CombatFeel.md` |
| `CF-FQ-021` | 핵심 게임 루프 검증 | 싱글 차량 전투 루프가 다음 개발 단계로 넘어갈 수 있는지 PASS/FAIL 판정 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/CoreLoop.md`, `Document/ProjectSSOT/05_TestChecklist.md` 갱신 |
| `CF-FQ-001` | 서버 권한 발사 요청 | 2클라 환경에서 발사 요청을 서버 권한 구조로 통과시키기 | `Icebox` | `Deferred` | 필요 | 필요 | 불필요 | `Document/Systems/Network/ServerFire.md` 또는 `Document/Systems/Combat/Fire.md` |
| `CF-FQ-002` | 조준/발사 피드백 분리 | 서버 판정과 로컬 조준/이펙트/Reticle 피드백 책임 분리 | `Icebox` | `Deferred` | 필요 | 필요 | 불필요 | 현재는 `CF-FQ-017`의 로컬 FireFeedback으로 대체 |
| `CF-FQ-003` | 체력/대미지 최소 구조 | 초기 장기 확장 후보. 최소 피해는 `CF-FQ-018`, 쉴드·방향별 장갑·차량 내구도 방어 본체는 `CF-FQ-033`으로 이관됐으므로 별도 착수하지 않음 | `P2` | `Deferred` | 필요 | 불필요 | 낮음 | `Document/Systems/Combat/Damage.md` |
| `CF-FQ-004` | 리스폰 최소 구조 | 2클라 전투 테스트 반복 가능 상태 만들기 | `Icebox` | `Deferred` | 필요 | 필요 | 낮음 | `Document/Systems/Network/Respawn.md` |
| `CF-FQ-005` | 전투 결과 기록 | 매치 종료/승패/기본 결과 기록 기반 만들기 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Combat/MatchResult.md` |
| `CF-FQ-006` | 테스트 계정/상태 초기화 도구 | 반복 테스트 준비 비용 줄이기 | `Icebox` | `Deferred` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/TestReset.md` |
| `CF-FQ-007` | 차량 로드아웃 저장 | 차량/무장 장착 상태를 재접속 후 유지 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Data/VehicleLoadout.md` |
| `CF-FQ-008` | 무장 데이터 정의 | 차량 장착형 터렛/무기 데이터 기준 만들기 | `P2` | `Done` | 필요 | 불필요 | 낮음 | `Document/Systems/Combat/WeaponData.md v1.1.0` |
| `CF-FQ-009` | 운영 로그 조회 기준 | 서버 전투/스폰/에러 로그를 추적 가능한 형태로 정리 | `Icebox` | `Deferred` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/LogView.md` |
| `CF-FQ-010` | 세션/로비 기초 | Dedicated Server 이후 접속 흐름 확장 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Network/Session.md` |

---

## 6. 현재 최우선 착수 후보

`CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`가 현재 단일 Active다. `InGameUIPlan.md v0.59.17` 기준 UI-P0-03~05 USER PASS와 UI-P0-06 Stage A+B, RPM, Heat, Applied Fitting Weapon Selection Runtime/HUD source, truthful Weapon Rail Visual Consumer, Player-facing Weapon Select Input, WeaponCharge P0 Runtime + HUD Resource Projection Technical PASS를 보존하고 **UI-P0-06은 current-runtime 기준 Technical Complete**로 닫았다. WeaponCharge는 explicit Maximum/Initial/PerShot/Recovery all-zero 기본값에서 Disabled이고, 활성 시 `UCFVehicleWeaponComp`가 선택 무기별 Current Charge·Game-Time 회복·next-shot 부족 판정·accepted-shot 1회 소비를 소유한다. Pawn은 실제 fire validation에서 `WeaponChargeInsufficient`를 반환하고 accepted result에서 Charge를 소비하며, HUD는 actual `ResourceChannels::WeaponCharge`만 `CHARGE N% / NO CHARGE`로 표시한다. VehicleBattery fallback은 0이다. final Charge Build `2d9f33261d3c427187f75338b8f37f1d` PASS, exact `WeaponChargeRuntimeResourceContract` `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / Result SHA-256 `df53c9732b3135f9eae230dc9f287d646e542179c0464e5b2ef1794b54e8e137`다. closure source readback에서 Pawn Charge precheck/post-consume이 기존 동적 검증된 Heat hook과 같은 실제 fire 함수의 인접 지점임을 확인해 중복 protected-friend dynamic test는 추가하지 않았다. saved WeaponData Charge authoring은 0이다. Rail USER Visual은 representative persisted multi-weapon content가 생길 때까지 content-dependent Deferred이고 artificial fixture 금지를 유지한다. VehicleBattery는 UI-P0-06 미완료 Runtime이 아니라 별도 shared-power Gameplay feature dependency이며 실제 Provider가 없을 때 `Unavailable/Collapsed`가 Current 정답이다. Heat/Charge tuning USER Visual과 Redline authoring/RPM USER Visual도 content-dependent Deferred follow-up으로 분리하며 UI-P0-06 Technical Complete를 차단하지 않는다. formal next Gate는 UI-P0-07 Target Knowledge다.


```text
1. CF-FQ-029 모듈형 런처 및 발사 인계: Paused / `LauncherMissilePlan.md v0.14.0` / LM-P0-06A Technical PASS / Build 6a633eb4cf21481d8ae26ec908d05660 PASS / Launcher 7ab7a286e5484cab845bb0cbfd5ac04e 4/4 + Ammo LauncherLock 1781accd3d9c47a59421fb1d2228e4ab 1/1 PASS / 다음 Gate `LM-P0-06 USER PIE`
2. CF-FQ-037 차량 스캐너 입력·장비 통합: Done / SCAN-P0-00~07 PASS / P0-06 USER PIE PASS / final Build `fcf52353d1f5440392d5e1c379f09ee3` PASS / `Systems/Targeting/SensorContact.md v1.1.0` Current
3. CF-FQ-036 차량 센서·Contact Intelligence Runtime: Done / SEN-P0-00~07 Technical Acceptance PASS / `Systems/Targeting/SensorContact.md v1.1.0` Current owner에 CF-FQ-037 Scanner 통합 반영

```

Current projection update — 2026-08-16: Launcher Runtime을 재구현하지 않고 `RecordShotResult(false)`의 ContinueRemaining/StopSequence를 asset-free Automation으로 검증했다. MuzzleBlocked·Angled/Vertical Ejection·Carrier Velocity USER PIE는 계속 Pending이며 CF-FQ-029 전체 Done으로 확대하지 않는다.

```text
3. CF-FQ-008 무장 데이터 정의: Done / WD-P0-00~03 Done / 기존 검증 보존 / `Systems/Combat/WeaponData.md v1.1.0` Current / UI-P0-06 Heat additive static contract 반영 / Asset 변경 0
4. CF-FQ-015 차량 데이터 튜닝 패스: Paused / VD-P0-00~03 Remote Technical Done / VehicleData 3/3 PASS / VD-P0-04 USER Tuning Pending
5. CF-FQ-026 타겟 선택 시스템: Paused / TS-P0-08 Remote Technical 3/3 + LOS Prefilter PASS / USER PIE Pending
6. CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크: Active / UI-P0-03~05 USER PASS / UI-P0-06 current-runtime Technical Complete — Stage A+B + RPM + Heat + Weapon Selection Runtime/HUD source + truthful Weapon Rail + Weapon Select Input + WeaponCharge Runtime/HUD Technical PASS / Charge Build `2d9f33261d3c427187f75338b8f37f1d` PASS / exact Charge `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS / VehicleBattery는 future shared-power Gameplay dependency·현재 Unavailable/Collapsed / Rail USER Visual·Heat/Charge tuning Visual·Redline/RPM Visual은 content-dependent Deferred / formal next UI-P0-07 Target Knowledge / D1-11 Structure PASS·Art Polish Deferred
7. CF-FQ-029 모듈형 런처 및 발사 인계: Paused / LM-P0-01~05 + LM-P0-06A Technical PASS / LM-P0-06 USER PIE 체크포인트 보존
8. CF-FQ-030 물리 제한형 미사일 비행·유도: Ready for Manual PIE / MG-P0-01~04 Direct Runtime·Test Assets Applied / Persisted Asset Technical Verification PASS / fresh DirectRuntimeContract 1/1 PASS / Manual PIE Pending
9. CF-FQ-031 차량 탄약·재장전 런타임: Done / AMMO-P0-00~08 Done / Heavy·Ripple USER PIE PASS / Ammo Systems Current
10. CF-FQ-034 차량 피팅·질량 런타임: Paused / FIT-P0-07C Quantitative Mobility Technical PASS / Light 1000kg·Default 1570kg·Heavy 1600kg fresh-PIE metrics captured / 재개 FIT-P0-07D USER Driving Feel / Field UI Pending
10-A. CF-FQ-038 차량 데이터 Authoring 시스템: Paused / DataAuthoringPlan.md v0.2.28 / DataAuthoringRoadmap.md v0.1.36 / DAUTH-P0-08A~M + P0-09 + P0-10 + P0-11 Technical PASS / Historical P0-08 baseline 117 Registry·Profile numeric 78 보존 / Current additive schema compatibility 118 Registry·Performance numeric 18·Profile numeric 79 PASS / UA-01~02 USER PASS / P0-12 USER PASS 2 / UA-03 DriveState RequiredProfileMissing checkpoint 보존

11. CF-FQ-033 차량 방어·손상 런타임: Done / DR-P0-00~07 Done / DR-PIE-00~06 USER PASS / Systems Current
12. CF-FQ-028 발사체 추진 시스템: Done / User PIE PASS / CF-TC-024 PASS / Systems Current
13. CF-FQ-027 투사체 비행 FX: Done / User PIE PASS / CF-TC-023 PASS / Systems Current
14. CF-FQ-035 인벤토리 Foundation: Paused / Remote Technical Checkpoint Complete / USER Field UI·Mobility Pending
15. CF-FQ-020 조작감/전투 템포/피드백 개선: Candidate
16. CF-FQ-021 핵심 게임 루프 검증: Candidate
17. CF-FQ-019 주행/전투 반복 테스트: Deferred / 런처·미사일 이후 통합 회귀로 재설계
```

`CF-FQ-032`는 UI-P0-02 Runtime Lifetime과 UI-P0-03~05 기존 HUD 데이터·수명 이전을 USER PASS로 완료했다. UI-P0-05는 단일 Target Marker, 내부 Actor 이름·중복 의미 Text 0, Offscreen 선택 유지·재진입 단일 복구, Clear/Reselect 회귀까지 사용자 PIE로 확인했다. 현재 formal next Runtime Gate는 `UI-P0-06 차량·무기 HUD`이며 D1-11 Production Structure PASS와 Art Polish Deferred·Non-Blocking 상태를 유지한다. 이후 `UI-P0-07 Target Knowledge → UI-P0-08 Radar` 순서를 따른다. 현재 단일 Active는 계속 `CF-FQ-032`다. `CF-FQ-038`은 DAUTH-P0-12 UA-01~02 USER PASS / USER PASS 2 / 다음 UA-03 체크포인트를 보존한 Paused이며, `CF-FQ-029·026·034·035`도 Paused, `CF-FQ-037`은 Done, `CF-FQ-030`은 Ready, `CF-FQ-031`은 Done, `CF-FQ-019`는 Deferred다.


`CF-FQ-034`는 VehicleData의 HardpointSlots·MountProfiles, EquipmentPresetData, WeaponData, CF-FQ-031 Ammo와 CF-FQ-033 Defense를 하나의 검증·질량 Snapshot으로 해석한다. `FIT-P0-04~07C`, cross-feature `FFIT-P0-01~04`까지 Technical PASS다. `/Game/CarFight/Tests/Fitting`에는 같은 `DA_VehicleDefense_TestSUV` 플랫폼의 Light 1000kg / Default 1570kg / Heavy 1600kg 공식 Fixture가 persisted 상태로 존재하며, FIT-P0-07C는 이를 fixture별 fresh PIE lifetime에서 계측해 0→30 시간 2.349081/2.252550/2.248457초, 제동거리 3.571402/3.657704/3.662367m, Steering 0.5·2초 coast Yaw 16.549116/20.418766/20.597601도를 확보했다. 결과는 단순 질량 순서의 PASS 규칙이 아니며 자동 Mobility Scalar를 만들지 않는다. 남은 범위는 `FIT-P0-07D USER Driving Feel Comparison`, `FFIT-P0-05 Field Fitting UI and PIE`와 16:9·32:9 실제 화면 가독성이다. 기술 결과를 USER PASS로 확대하지 않는다.

`CF-FQ-035`는 TargetSelect 재개에 따라 Paused다. INV-P0-00~05, FFIT-P0-01~04, FIT-P0-06 C++ ViewData·Blueprint Contract와 Full CarFight 84/84 Technical PASS를 그대로 보존한다. 사용자-facing Field UI·USER PIE와 Mobility 검증이 남아 있으므로 M6 전체 Done 또는 CF-FQ-035 Done으로 승격하지 않는다. 상점·가격·재화·월드 루팅·제작·내구도·SaveGame·네트워크는 후속으로 분리한다.

판단 근거:

```text
- FCFProjectileLaunchContext가 Command Target과 Initial Launch State를 분리하고 Pawn → Pool → Projectile Actor 인계를 제공한다.
- TurretMountData의 가변 Muzzle 배열, Legacy 단일 Muzzle fallback과 승인 발사 기반 SingleCycle 진행이 적용됐다.
- UCFWeaponData의 Pattern Data Contract, UCFLauncherComp의 Ripple·Salvo Scheduler와 FCFLauncherReleaseConfig 기반 Direct·Angled·Vertical Release가 적용됐다.
- Scheduler는 첫 입력 순간 Command Target 고정, 다음 Muzzle 재해석, 입력 중복 거부, ContinueRemaining·StopSequence, 차량 파괴·무기·터렛 변경 취소와 두 쿨다운 시작 정책을 처리한다.
- Release Runtime은 실제 Muzzle Transform 방향, EjectionSpeed, CarrierVelocityRatio, Command Target 분리와 실제 사출 방향 안전 검사를 처리한다.
- 공식 Editor Build Job acd575ffac2f4d64bd73194e160a3f62에서 최종 VehiclePawn 컴파일·링크와 Exit Code 0을 확인했으며, 직전 Job 819c61b1a2f247e38768b4205f7e3008에서 신규 ReleaseContract 테스트 소스 컴파일을 확인했다.
- Combat Runtime Automation Job `95bb9b328ed2443baccf0fb59e3a0cab`에서 Launch Handoff·MuzzleSequence·Pattern·Scheduler·Release·SourceIsolation·Propulsion 필수 회귀와 전체 CarFight 23개 테스트가 Success했다.
- LM-P0-05 자산·참조·플레이 진입 경로는 독립 AssetDump와 새 Unreal 프로세스에서 PASS했으며 실제 Muzzle 순서, Ripple 간격, Angled·Vertical 방향, 이동 차량 속도 상속, Pool 부족, MuzzleBlocked와 취소 결과는 CF-TC-025·026 사용자 PIE Pending이다.
- CF-FQ-030의 MG-P0-01~04는 UCFMissileFlightComp·UCFMissileGuideComp, Direct Flight State, CurrentVelocityDirection 추진, TargetActor 제한형 유도, ContinueStraight 목표 소실, Overshoot와 Pool Reset을 실제 Projectile Runtime에 연결했다.
- 첫 발과 Ripple·Salvo 후속 발사는 `FCFLauncherCommandTargetSnapshot`의 같은 CommandTargetLocation·약한 GuidanceTargetActor를 사용하며 현재 TargetSelect를 후속 발사마다 재조회하지 않는다.
- 공식 Editor Build Job `ce88150dfbbc495d91d1e2b8cd7c5225` / Exit Code 0과 Combat Runtime Job `85c1e7285a7946fa8f55182b6247f0f4` / 전체 43/43·필수 21/21 Success를 확인했다.
- 전용 Missile Test 자산은 기존 작업에서 적용된 상태를 유지했으며 이번 통합 안정화에서는 Unreal Asset을 수정하지 않았다. Manual PIE 여섯 시나리오는 Pending이다.
- CF-FQ-028은 비유도 Rocket 추진, 공식 Editor 빌드, 사용자 PIE와 Systems 승격을 완료했다.
- CF-FQ-019는 기존 반복 전투 범위를 그대로 실행하지 않고 Deferred로 유지하며 CF-FQ-029·030 이후 통합 회귀로 다시 설계한다.
- CF-FQ-027은 사용자 PIE 전체 행렬을 완료해 Done / CF-TC-023 PASS다.
- CF-FQ-026은 Remote Technical 3/3 + LOS Prefilter PASS를 보존한 Paused다. 사용자 시각 확인 가능 시 TS-P0-08 USER PIE를 그대로 재개한다.
- CF-FQ-015는 `VehicleDataTuningPlan.md v0.2.0` 기준 VD-P0-00~03 Remote Technical Done이다. 공식 Build `0cffed2f02674b6692d4f8af811d9036` PASS와 `CarFight.VehicleData` `59091b9559db46859c3a521be72396bf` 3/3 PASS를 확인했다. DA_TestSedan/DA_TestSUV는 read-only baseline이며 실제 주행 튜닝값은 USER PIE 전 변경하지 않는다.
- CarFight는 프로젝트 전역에서 게임 사운드를 지원하지 않으며 Audio 기능을 별도 후보로 등록하지 않는다.
```

---

## 6-0. CF-FQ-028 발사체 추진 시스템 완료 기준

대표 문서:

```text
Document/Plan/ProjectilePropulsionPlan.md
```

현재 체크포인트:

```text
PP-P0-00 추진 계약·Rocket/Missile 책임 경계: Done
PP-P0-01 Motor Types와 PropulsionConfig: Applied
PP-P0-02 ProjectileMotorComp 상태 머신: Applied
PP-P0-03 Actor 활성화·비활성화·Pool Reset 연결: Applied
PP-P0-04 가속·최대 속도·BurnedOut 관성 비행: Applied
PP-P0-05 Thruster FX와 Motor Burning 상태 동기화: Applied
PP-P0-06 RuntimeContract Automation: Source Compile PASS / Execution Not Run
PP-P0-07 공식 Editor 빌드: Latest Build Job e8b812bd479549299dd116f9bae8996f / Exit Code 0
PP-P0-08 테스트 Rocket ProjectileData: Done / DA_PFX_ThrusterTest
PP-P0-09 사용자 PIE와 CF-TC-024: Done / PASS
PP-P0-10A Systems 승격: Done / Document/Systems/Combat/Projectile.md v1.4.0
PP-P0-10B 반복 전투 확장 회귀: Deferred / CF-FQ-019 / 런처·미사일 구현 범위 확정 후 재설계
```

P0 계약:

```text
- 기존 ProjectileData는 bUsePropulsion=false 기본값으로 InitialSpeed 고정 비행을 유지한다.
- 추진 Rocket은 InitialSpeed로 분리된 뒤 선택적 IgnitionDelay를 거친다.
- Burning 동안 발사 시 저장한 고정 LaunchDirection으로 가속한다.
- MaximumPropelledSpeed는 추진 상한이며 잘못된 낮은 값으로 InitialSpeed를 강제 감속하지 않는다.
- BurnDuration이 끝나면 BurnedOut으로 전환하고 추가 가속 없이 기존 속도와 중력으로 비행한다.
- BurnedOut은 Projectile Deactivate가 아니다.
- Trail은 Projectile 비행 생명주기를 따르고 Thruster는 실제 Motor Burning 상태만 따른다.
- FX 누락 또는 재생 실패는 추진, 이동, 충돌과 Damage를 바꾸지 않는다.
- Target Homing, Laser Guided, 수동 유도와 목표 상실 정책은 후속 Guidance 기능으로 분리한다.
```

CF-FQ-027 완료 체크포인트:

```text
- UCFProjectileData TrailFxSettings / ThrusterFxSettings 기반은 Current System으로 유지된다.
- ACFProjectileActor의 Socket/Fallback, 종료 Reset과 Pool 재사용 기반을 유지한다.
- Trail=NS_RibbonTrail / Thruster=NS_RocketExhaust_Realistic 사용자 PIE를 완료했다.
- Trail-only / Thruster-only / Trail+Thruster / Missing Socket Fallback: PASS
- Hit·LifeExpired Reset / Pool 20발 이상 / Ribbon History 무잔류 / 30 FPS 고속 Bounds: PASS
- CF-FQ-027 Done / CF-TC-023 PASS
- Current System: Document/Systems/Combat/Projectile.md v1.5.0
- Completed Plan: Document/Plan/ProjectileFlightFxPlan.md v1.0.0
- Automation Source Compile PASS / Execution Not Run — Runner 미노출
```

## 6-1. Reticle / FireFeedback 현재 구현 및 완료 기준

`CF-FQ-017`의 현재 구현 판단과 완료 기록에는 아래 문서를 우선한다.

```text
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/UI/VehicleDebugPanel.md
```

현재 구현 상태:

```text
- WeaponFire는 판정/기록 담당이고, ACFVehiclePawn::BuildFireFeedbackViewData()가 UI용 표시 데이터를 만든다.
- UCFAimReticleWidget은 기본 Aim 상태와 FireFeedback ViewData를 합쳐 최종 Reticle 상태, 텍스트, 색상을 갱신한다.
- WBP_AimReticle에는 중앙점/4방향 브라켓, FireFeedback State/Hint, Cooldown, 전용 OutOfArc 경고가 연결되어 있다.
- FireSuccess는 짧게 성공 색상으로 표시된 뒤 Cooldown으로 전환되고, 쿨다운 종료 후 텍스트가 사라진다.
- OutOfArcWarning은 전용 보조 경고로 표시하며 주 Reticle 상태와 색상을 강제로 덮어쓰지 않는다.
- FirePending은 타입과 표시 문구만 유지하며 현재 동기 발사 구조에서는 사용하지 않는다.
```

최종 사용자 PIE 확인:

```text
- NoWeapon 회색 Reticle, "무기 없음"과 "사용 가능한 무기 없음" 표시 PASS
- AimBlocked 주황 Reticle, "조준 가림"과 "조준선이 막힘" 표시 PASS
- 두 실패 상태의 유지 시간 종료 후 FireFeedback 텍스트 제거 PASS
- 장애물 제거 후 정상 Aim 상태 복귀 PASS
- Ready → FireSuccess → Cooldown → Ready 정상 발사 회귀 PASS
- 판정: CF-FQ-017 Done / CF-TC-014 PASS
```

---

## 6-2. CF-FQ-018 피격/피해 구현 선행 기준

현재 기준 문서:

```text
- Document/Plan/HitDamage/ImplementationDesign.md
- Document/Systems/Combat/DamageHitContext.md
- Document/Systems/Combat/Projectile.md
- Document/Systems/Vehicles/VehicleCoreDecisions.md
```

현재 구현 및 확인 상태:

```text
- WeaponHit = ECC_GameTraceChannel1, Projectile = ECC_GameTraceChannel2로 구현됐다.
- VehicleVisualHit Collision Profile이 구현됐다.
- VehicleMesh는 기존 차량 물리를 유지하면서 WeaponHit / Projectile을 Ignore한다.
- SM_Body는 QueryOnly 상태에서 WeaponHit / Projectile을 Block한다.
- Dummy HitScan과 Projectile HitComponentName이 DamageHitContext에 기록된다.
- Unreal Editor 타깃 빌드가 성공했다.
- 일반 속도 HitScan / Projectile이 SM_Body에서 정상 충돌하는 것을 사용자 PIE에서 확인했다.
```

당시 확인했던 선행 문제와 해결 결과:

```text
- Reticle 월드 목표점과 실제 Muzzle 발사 해 불일치 → CF-FQ-022에서 해결 완료
- Camera Aim Trace와 WeaponHit 표면 기준 차이 → CF-FQ-022에서 단일 Aim Solution 계약으로 정리
- 터렛 추적 중 실제 Muzzle 방향과 요구 방향 차이 → bAllowFireWhileAligning 정책과 Weapon Aim Solution으로 해결
- 고속 Projectile 프레임 사이 터널링 → CF-FQ-023 연속 충돌로 해결 완료
```

현재 판정:

```text
- CF-FQ-022 Done / CF-TC-019 PASS
- CF-FQ-023 Done / CF-TC-020 PASS
- CF-FQ-018 Done / CF-TC-016 PASS
```

Damage Runtime 당시 착수 조건:

```text
- CF-FQ-022 조준점·터렛·총구 정렬 완료
- CF-FQ-023 고속 Projectile 연속 충돌 완료
- 일반 속도와 고속 Projectile 모두 신뢰 가능한 DamageHitContext 생성
- 이후 BaseDamage, 체력 감소, 파괴 상태 구현
```

---

## 6-3. CF-FQ-022 / CF-FQ-023 선행 작업 기준

### 조준점·터렛·총구 정렬

기준 문서:

```text
Document/Plan/AimFireAlignment/ImplementationDesign.md
```

확정 원칙:

```text
- Reticle이 지시하는 월드 위치를 DesiredAimTargetLocation SSOT로 사용한다.
- Camera Aim Trace와 실제 무기 Trace는 WeaponHit 응답 기준을 공유한다.
- 터렛은 Muzzle 위치에서 DesiredAimTargetLocation으로 향하는 요구 발사 방향을 추적한다.
- 현재 Muzzle 방향과 요구 방향의 정렬 오차를 계산한다.
- `UCFTurretMountData.bAllowFireWhileAligning`으로 터렛별 정렬 중 발사 허용 여부를 선택한다.
- 기본값은 `true`이며, true인 터렛은 정렬 중 현재 Muzzle 방향으로 발사하고 false인 터렛은 정렬 완료 전 발사를 거부한다.
- 정렬 완료 후에는 Muzzle → Reticle 목표 방향을 실제 발사 방향으로 사용한다.
- 총구 앞 장애물은 실제 최종 발사 방향 기준 WeaponHit Trace로 검사하며, `MuzzleBlocked`는 정책값과 관계없이 발사를 거부한다.
- OutOfArc와 TurretAligning은 서로 다른 상태로 관리한다.
- Core/Presentation C++, Reticle Recovery Hotfix와 터렛별 정렬 중 발사 정책 구현을 완료했다.
- `BuildEditor.bat` 공식 빌드와 사용자 PIE에서 정책 true/false, 정렬 완료 탄착, `MuzzleBlocked`를 확인했다.
- 현재 판정은 `CF-FQ-022` Done / `CF-TC-019` PASS다.
```

### 고속 Projectile 연속 충돌

기준 문서:

```text
Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md
```

확정 원칙:

```text
- ProjectileMovement Sweep을 ProjectileData 값으로 명시적으로 적용한다.
- 고속/중력 Projectile은 Sub-stepping과 MaxSimulationTimeStep / MaxSimulationIterations를 데이터에서 적용한다.
- P0 안전 기본값은 Sweep=true, Sub-step=true, MaxStep=0.008333, Iterations=8이다.
- 잔여 터널링은 Previous → Current 구간의 CollisionRadius 기반 Sphere Sweep으로 차단한다.
- OnComponentHit과 보조 Sweep은 ResolveProjectileImpact()와 bImpactResolvedThisActivation을 공유한다.
- Pool 재활성화마다 PreviousCollisionLocation과 Impact 처리 상태를 초기화한다.
- Projectile 자신, InstigatorActor, OwnerActor Ignore를 기존 이동과 보조 Sweep 모두에서 유지한다.
- CCD는 주 해결책이 아니라 탄종별 보조 안전장치이며 기본값은 false다.
- 초고속 탄종은 실제 판정을 HitScan으로 분리하고 Projectile은 Tracer로 사용할 수 있다.
- 2026-07-14 기준 연속 충돌 C++ 구현과 사용자 실행 Tools\\BuildEditor.bat 공식 Editor 빌드는 완료됐다.
- 사용자 PIE에서 일반 속도 Projectile 피격, 피격 Actor `BP_CFVehiclePawn_C_1`, 피격 컴포넌트 `SM_Body` 기록을 확인했다.
- HitComponentName 데이터 전달 경로를 Debug Panel의 `피격 컴포넌트` 독립 항목으로 노출했다.
- 사용자 PIE에서 `30 FPS + 기준 InitialSpeed 4배` 차량 집중 발사 테스트를 통과했다.
- 얇은 벽 앞 차량 배치에서 벽 관통 없음, 첫 Blocking Hit, 중복 Impact 없음과 Pool 재사용 정상 동작을 확인했다.
- 위 집중 스트레스 범위를 CF-FQ-023의 P0 완료 기준으로 적용해 상태를 Done으로 전환했다.
- 60 / 120 FPS 전체 조합과 이동 차량 검증은 CF-FQ-019 확장 회귀 범위로 유지한다.
```

---

## 6-4. CF-FQ-024 전투 FX 완료 기준

현재 구현 문서:

```text
Document/Systems/Combat/CombatFx.md
```

완료 Plan:

```text
Document/Plan/CombatFxAudio/ImplementationDesign.md
```

경로의 `CombatFxAudio`는 기존 디렉터리 호환을 위한 레거시 이름이며 현재 구현은 시각 FX만 다룬다.

완료 구현:

```text
- UCFCombatFxData와 UCFCombatFxComp 데이터 기반 런타임
- UCFWeaponData.DefaultFireFxData
- UCFProjectileData.DefaultImpactFxData
- UCFVehicleData.DefaultDestroyedFxData / DestroyedFxSocketName
- 승인된 발사의 실제 Muzzle FX 1회
- HitScan / Projectile 첫 ImpactLocation의 FX 1회
- 최초 Destroyed 전환의 차량별 SM_Body.FX_Destroyed 소켓 FX 1회
- 발사 거부, 중복 Impact와 추가 피해의 FX 중복 방지
- MaximumLifetimeSeconds Loop 잔류 안전 퓨즈
- ACFCombatFxPreviewActor EditorOnly 튜닝 도구
- Impact P0 NS_BasicHit 현재 크기 승인
- FX 자산 미연결 또는 생성 실패에서도 기존 판정 유지
```

최종 검증:

```text
- 공식 Admin Editor 빌드 3건 PASS
- 최신 사용자 직접 Editor 빌드 PASS
- Muzzle 정상 발사 1회 / 발사 거부 0회 PASS
- Impact 실제 위치 / 첫 1회 / 중복·잔류 없음 PASS
- Destroyed FX_Destroyed 위치 / 최초 1회 / 추가 피해 중복·잔류 없음 PASS
- 기존 조준·발사·피격·피해·파괴 회귀 PASS
- 오디오 클래스·모듈·에셋 참조 0개 유지
- CF-TC-021 PASS
- CF-FQ-024 Done
```

프로젝트 전역 제외 범위:

```text
- 모든 게임 사운드와 오디오 런타임
- 물리 표면별 Impact 세분화
- 지속형 Projectile Trail
- 완성형 Geometry Collection 파괴
- 서버 복제와 원격 클라이언트 FX 동기화
- 모든 외부 Niagara에 공통 적용되는 범용 Scale 보장
```

---

## 6-5. CF-FQ-025 이중 레티클 및 사격방향 시각화 기준

대표 Plan:

```text
Document/Plan/ReticleAimDirection/ImplementationDesign.md
```

완료 범위:

```text
- Image_CenterDot을 사용자 조준 레티클로 유지
- FCFVehicleWeaponAimSolution.CurrentMuzzleDirection을 터렛 레티클 방향 기준으로 사용
- 사용자 조준점과 같은 비교 거리의 TurretReticleWorldLocation을 Image_WeaponReticle로 화면 투영
- HitScan/Projectile과 중력 여부에 독립적인 단일 터렛 레티클 계산 적용
- 기존 DirectImpact/LaunchDirection Preview는 UI 소비에서 제외하고 Legacy Debug로만 보존
- 정렬 중 발사 허용/금지, MuzzleBlocked와 기존 FireFeedback 회귀 검증
```

현재 제외 범위:

```text
- 중력 Projectile Ballistic Solver
- 투사체 착탄 위치의 월드 공간 3D 표시
- 이동 목표 Lead Indicator
- 자동 락온과 Aim Assist
- 다중 터렛 동시 Reticle
- 네트워크 지연 보정
- 완성형 전투 HUD 전체 재설계
```

완료 기준:

```text
- Tools\BuildEditor.bat PASS
- Image_CenterDot 기존 조준 동작 유지
- Image_WeaponReticle이 CurrentMuzzleDirection 기반 터렛 조준 지점을 표시
- 정렬 중 두 레티클 분리와 정렬 완료 수렴
- HitScan/Projectile 및 중력 여부에서 동일한 터렛 레티클 의미 유지
- MuzzleBlocked 발사 차단과 터렛 레티클 위치 책임 분리
- Ready / FireSuccess / Cooldown / NoWeapon / AimBlocked 회귀 PASS
- CF-FQ-025 사용자 싱글 PIE PASS
```

---

## 6-6. CF-FQ-026 타겟 선택 시스템 구현 기준

기준 문서:

```text
- Document/Design/TargetSelect.md
- Document/Plan/TargetSelectPlan.md
- Document/Plan/TargetSelectRoadmap.md
- Document/Plan/TargetSelectWorkOrder.md
```

현재 구현 상태:

```text
- TS-P0-00 구조 조사와 기존 차량·카메라·입력·충돌 경계 복구 완료
- TS-P0-01 핵심 상태·인터페이스·설정 DataAsset·Pawn 기본 통합 완료
- TS-P0-02 TargetPoint → SM_Body Bounds → Actor 위치 Fallback 완료
- TS-P0-03 직접 조준 우선·화면 근접도·거리·안정 키 후보 정렬 완료
- TS-P0-04 파괴·EndPlay·가림 유예·추적 거리 선택 수명 완료
- TS-P0-05 IA_SelectTarget / IA_ClearTarget과 IMC_Vehicle_Default 입력 연결 완료
- TS-P0-06 UCFTargetSelectWidget / WBP_TargetSelect 후보·선택 HUD 완료
- TS-P0-07 TargetUsePolicy와 VehicleWeaponComp 읽기 전용 장비 평가 연동 완료
- 기존 TS-P0-00~07 Done과 과거 TargetSelect Automation 7/7 PASS 증거를 보존
- 2026-08-15 공식 Build `2aa5462fbd5445379416423210054fec` PASS
- 최종 current-source 공식 Build `4b4554ab8f7e455abf0c0538ae10a664` PASS, TS-P0-08 `3d8bb0ebbeb5427f954cc7621b432408` 3/3 PASS
- `SettingsPath`로 표준 BP CDO 실제 source가 `FallbackTargetSelectConfig`, `TargetSelectData=None`임을 확정: Direct 2000m / Proximity 1200m / 7도 / 20Hz / Occlusion 1.5초 / Switch 15%
- TargetPoint CDO는 Use=True / AutoAlign=True / PreferredBounds=SM_Body / Offset=0
- `SingleTargetBoundary`는 ±6.9도 수락·±7.1도 거부가 16:9·32:9에서 좌우 대칭
- 런타임 검색 진단을 추가하고 proximity 밖 비-Direct Targetable Actor의 LOS Trace만 생략하는 의미 보존 사전필터 적용
- Transient 구조 샘플: WorldScanned=11 / Input=3 / VisibilityTraces=1 / PrefilterSkipped=2 / TotalTraces=2 / SearchMs=0.1780 — 성능 PASS로 해석하지 않음
- 최종 자산 비변경 TS-P0-01·02·03·04·07 회귀 각 1/1 PASS
- dirty WBP_TargetSelect 보호 때문에 자산 저장 가능성이 있는 TS-P0-05·06 및 전체 suite는 이번 세션 미실행
- 전체 Actor 20Hz 순회는 남은 scalability 항목이며 대표 workload 없이 Registry/Spatial Query로 임의 전환하지 않음
- TS-P0-08은 Paused이며 USER PIE 통합 검증과 튜닝은 미완료
```

현재 체크포인트:

```text
현재 위치: TS-P0-08 P0 통합 검증과 튜닝 / Paused / USER PIE Pending
보호 USER 결함: 후보 텍스트와 대상 겹침, 후보 범위 과대 체감, debug Sphere 실제 비가시, 동일 차량 대표 위치/인식 범위 재검증
원격 기술 증거: 단일 대상 7도 경계 좌우 대칭 + TargetPoint·CandidateRanking·SelectionLifetime·EquipmentQuery PASS
다음 조치: 대표 workload 없이는 20Hz full-world scan 구조 교체 보류 → USER 단일 차량 범위 재검증 → HUD·debug 시각성·입력/화면비 튜닝
```

P0 완료 전 제외 범위:

```text
- 자동 조준과 선택 대상 방향으로 직접 조준 무기 보정
- 다중 타겟과 다중 락온
- 센서 기반 상세 정보 공개
- 장비별 락온·해킹·견인 획득 로직
- 부위 선택
- 서버 복제와 원격 클라이언트 동기화
```

---

## 7. 기능을 Plan으로 승격하는 기준

기능을 `Document/Plan/<기능명>/`로 승격하려면 아래 조건을 만족해야 한다.

```text
- 목적이 한 문장으로 설명된다.
- 클라이언트 / 서버 / 관리툴 필요성이 1차로 판단됐다.
- 완료 후 Systems 문서 위치가 정해졌다.
- 이번 범위에서 제외할 항목이 정해졌다.
- 선행 Systems 문서가 무엇인지 확인됐다.
```

승격 시 `Plan` 폴더에는 아래 문서 구성을 권장한다.

```text
Document/Plan/<기능명>/README.md
Document/Plan/<기능명>/Scope.md
Document/Plan/<기능명>/CurrentState.md
Document/Plan/<기능명>/Design.md
Document/Plan/<기능명>/TaskList.md
Document/Plan/<기능명>/TestPlan.md
Document/Plan/<기능명>/DecisionLog.md
```

단, 기능 규모가 작으면 `README.md`, `TaskList.md`, `TestPlan.md`만으로 줄일 수 있다.

---

## 8. 관리툴 후보 큐

관리툴은 현재 싱글 차량 고도화 범위에서 제외한다.
아래 항목은 장기 서버/운영 재개 시 다시 검토할 보류 후보로만 유지한다.

| ID | 후보 | 발생 조건 | 임시 대체 | 정식화 시점 | 상태 |
|---|---|---|---|---|---|
| `CF-ADM-001` | 테스트 계정/상태 초기화 | 같은 테스트 세팅을 3회 이상 반복 | 콘솔 명령 / 임시 서버 명령 | 계정/로드아웃/인벤토리 도입 후 | `Candidate` |
| `CF-ADM-002` | 유저 차량 상태 조회 | 차량 소유/로드아웃 저장 도입 | 로그 / DB 직접 조회 | 로드아웃 서버 저장 후 | `Candidate` |
| `CF-ADM-003` | 보상 지급/회수 | 전투 결과와 보상 도입 | 수동 DB 수정 | MatchResult/RewardLog 도입 후 | `Candidate` |
| `CF-ADM-004` | 전투 로그 조회 | 서버 판정 전투 도입 | 로그 파일 검색 | MatchResult 저장 후 | `Candidate` |
| `CF-ADM-005` | 서버 상태 확인 | 장시간 Dedicated Server 테스트 반복 | 콘솔 로그 확인 | 테스트 서버 상시 운용 전 | `Candidate` |

현재 관리툴 원칙:

```text
1. 이번 싱글 차량 고도화 사이클에서는 관리툴을 만들지 않는다.
2. 서버/운영 기능이 재개될 때만 콘솔 명령 / CLI / 임시 서버 명령부터 검토한다.
3. 데이터 구조가 안정화된 뒤 정식 관리툴로 승격한다.
4. 관리툴 기능도 완료되면 Document/Systems/Admin/ 아래에 기록한다.
```

---

## 9. 기능 큐 갱신 조건

아래 상황이 발생하면 이 문서를 갱신한다.

```text
- 새 기능 후보가 생김
- 기능 우선순위가 바뀜
- 기능이 Plan으로 승격됨
- 기능이 완료되어 Systems로 승격됨
- 관리툴 후보가 생김
- 기능이 보류/기각됨
```

---

## 10. 문서 버전 관리

- 현재 문서 버전: `v1.54.38`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 기능 큐 운영 방식 자체 변경 |
| Minor | 기능 후보/관리툴 후보/승격 기준 추가 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 11. 체인지로그

### v1.54.38 - 2026-08-19

- UI-P0-06 중간점검에서 원래 Roadmap 종료조건을 재확인했다. 현재 Runtime이 제공하는 차량/무기 HUD 채널을 Production 경로에 유지하고 Provider 없는 채널을 추정 없이 Unavailable/Collapsed로 두는 것이 완료조건이며, VehicleBattery Gameplay Runtime 신규 구현은 UI-P0-06 필수 범위가 아니다.
- VehicleBattery를 future shared-power Gameplay dependency로 분리하고 UI-P0-06을 current-runtime 기준 `Technical Complete`로 전환했다. Rail USER Visual, Heat/Charge saved tuning USER Visual, authoritative Redline/RPM USER Visual은 실제 content가 준비될 때 수행하는 Deferred follow-up으로 남긴다.
- formal current Gate를 `UI-P0-07 Target Knowledge`로 전진시켰다. 완료된 UI-P0-06 Stage A/B·RPM·Heat·Weapon Selection·Rail·Input·WeaponCharge 증거는 관련 결함 없이 반복하지 않는다.
- `CFHUDDataProvider.h v1.7.0`, `CFHUDPresenter.h v1.15.0` Current 주석을 실제 상태에 맞췄다. 실행 코드·Asset·Battery·Heat/Charge tuning·Redline mutation은 0이다.

Migration: 대표 Plan은 `InGameUIPlan.md v0.59.17`을 사용한다. VehicleBattery는 향후 별도 Gameplay power feature가 구현될 때 기존 `VehicleBattery` ResourceChannel에 연결하고, 그 전에는 UI-P0-07~09 진행을 막지 않는다.

### v1.54.37 - 2026-08-19

- `CF-FQ-032 / UI-P0-06 WeaponCharge`를 Technical PASS로 반영했다. WeaponCharge는 explicit Maximum/Initial/PerShot/Recovery가 유효할 때만 활성이고 existing all-zero WeaponData는 Disabled 호환이다. 선택 무기별 Charge 상태와 Game-Time 회복은 WeaponComp가 소유하며 Pawn fire path는 충전 부족 `WeaponChargeInsufficient`와 accepted-shot 1회 소비를 사용한다.
- HUD는 actual `ResourceChannels::WeaponCharge`만 소비해 `CHARGE N%`와 `NO CHARGE`를 Compact 계약에 투영한다. VehicleBattery/정적 설정/Cooldown fallback은 0이고 Secondary 최대2 및 `Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY` 우선순위를 유지한다.
- final Official Build `2d9f33261d3c427187f75338b8f37f1d` PASS, exact `WeaponChargeRuntimeResourceContract` `8bf4e80d9f1b4c88b0fd557053b27810` 1/1 PASS, Result SHA-256 `df53c9732b3135f9eae230dc9f287d646e542179c0464e5b2ef1794b54e8e137`다. closure source readback에서 Charge precheck/post-consume이 기존 Heat 동적 검증 경계와 같은 Pawn fire 함수의 바로 인접 hook임을 확인해 별도 중복 friend test는 추가하지 않았다.
- saved WeaponData Charge authoring, VehicleBattery, Heat tuning, Redline authoring, Rail USER Visual 상태 변경은 0이다. Rail USER Visual Deferred와 artificial 2무기 fixture 금지는 유지하며 현재 남은 실제 Gameplay Runtime은 VehicleBattery다.

Migration: 대표 Plan은 `InGameUIPlan.md v0.59.16`을 사용한다. WeaponCharge Technical PASS는 관련 결함 없이 반복하지 않고 실제 Charge 무기 tuning은 네 explicit 값을 함께 결정한다. VehicleBattery는 별도 shared-power Runtime으로 남기며 Charge를 Battery로 재해석하지 않는다.

### v1.54.36 - 2026-08-19

- `CF-FQ-032 / UI-P0-06` Player-facing Weapon Select Input을 Technical PASS로 갱신했다. `IA_SelectWeapon` Axis1D + `IMC_Vehicle_Default` 숫자 1~9 direct ordinal mapping을 사용하고 Mouse Wheel Radar 예약/게임패드 미지정을 보존한다. final Build `dec0757b745342b4b90ff5f17b761eb0`, persisted IA fp `1A8A0402`, IMC fp `52D2D868` / 47→56 mapping, exact `WeaponSelectInputContract` `daeaf649669d48c79746a65cc44b1b89` 1/1 PASS다.
- Rail USER Visual은 existing persisted representative가 모두 1무기라 Deferred로 전환했다. artificial fixture는 Production Weapon mass0 후 mass-valid test-only preset을 찾았지만 finite-ammo loadout 작성까지 필요해 사용자 중단 기준에 따라 확장하지 않았다. 실패 실행에서 생성된 fixture asset은 cleanup error0으로 삭제됐다.
- USER Visual PASS는 추정하지 않는다. representative persisted multi-weapon content가 실제 생기면 그때 1↔2 Header/Rail swap과 내부 ID 노출0을 USER Gate로 다시 연다.

Migration: 대표 Plan은 `InGameUIPlan.md v0.59.15`을 사용한다. Weapon Select Input Technical PASS는 반복하지 않고 Rail USER Visual은 representative multi-weapon content prerequisite를 만족할 때만 재개한다.

### v1.54.35 - 2026-08-19

- `CF-FQ-032 / UI-P0-06` Weapon Rail Visual Consumer를 Technical PASS로 전진시켰다. pre-apply persisted `WBP_CFWeaponPanel` fingerprint `4A873878`에서 Rail이 Turret/Ammo/Reload semantic Image 3개뿐이고 실제 `SelectableWeapons` source와 구조가 불일치함을 확인했다.
- 선택 무기는 기존 Header/Selected Card에만 유지하고 비선택 weapon만 Provider fixed order로 표시하는 truthful Text Rail을 적용했다. 0개 숨김, 1~3개 `원본 순번 + DisplayName`, 4+ `앞2 + +N`, 이름 부재 `WEAPON`이며 internal MountProfileId/WeaponId/AssetName fallback과 fake icon/resource summary는 0이다.
- Official Build `941ea18597b3483594be8de3317e68fd` PASS, targeted `WeaponPanelOnly` apply `0617f7cbed97427a942c7a6f59936e22` exact asset1 / other Production0, fresh AssetDump saved fingerprint `26CFB205`, exact `WeaponRailVisualContract` `7cc9ddd5b2204725bb3ec45c200d649b` 1/1 PASS를 확보했다.
- Weapon Selection Runtime/HUD source와 Heat/RPM/Stage A+B/기존 USER PASS는 반복하지 않았다. Battery/Charge, Heat tuning, Redline authoring은 변경하지 않았고 Rail USER Visual과 실제 Weapon Select Input Mapping은 Pending이다.

Migration: `InGameUIPlan.md v0.59.14`을 현재 UI 상세 기준으로 사용한다. truthful Text Rail Technical PASS는 관련 결함 없이 반복하지 않으며 정식 per-weapon icon/source가 생기기 전 name-only Rail을 유지한다.

### v1.54.34 - 2026-08-18

- `CF-FQ-032 / UI-P0-06` Player-facing WeaponGroup source를 fresh 감사해 Applied Fitting Snapshot의 weapon-bearing `ResolvedMounts` 고정 순서를 실제 source로 확정했다. 새 Group ID를 만들지 않고 fixed display order + SelectedWeaponIndex로 계약했으며 내부 MountProfileId는 Runtime identity로만 남겨 HUD 노출을 금지했다.
- Fitting→WeaponComp 선택 목록, per-weapon Cooldown/Heat 독립 상태, Pawn 안전 전환, HUD DisplayName-only 목록을 구현했다. Launcher active 전환은 기존 WeaponChanged cancel cleanup을 사용하고 현재 single active Turret Visual도 같은 선택으로 갱신한다.
- final Official Build `62ffb62f69524bb18c3a9f11bb9f58f1` PASS, exact `WeaponSelectionRuntimeContract` process `ff23cf6ac04e4b2c8cf0084b24173cdb` 1/1 PASS / Failure 0, Result SHA-256 `0ffa955f626d0de3f5ebf7b2ac594d7f8b039d268fe6efb2ab0730abfe77618c`다.
- Production WeaponRail 기존 세 Image는 Turret/Ammo/Reload 의미라 실제 weapon slot으로 재해석하지 않고 Collapsed를 유지했다. Asset/Input mutation과 USER Visual PASS 추가는 0이다. 남은 actual Gameplay Runtime은 VehicleBattery·WeaponCharge다.

Migration: Weapon Selection Runtime/HUD source PASS는 관련 결함 없이 반복하지 않는다. 다음 WeaponPanel slice는 실제 selection ViewData를 소비할 Rail Visual Consumer를 fresh persisted 구조 기준으로 설계하며 현재 의미 아이콘이나 내부 ID를 무기 슬롯으로 위장하지 않는다.

### v1.54.33 - 2026-08-18

- `CF-FQ-032 / UI-P0-06` 남은 WeaponGroup·VehicleBattery·WeaponCharge·Heat Runtime을 fresh 감사하고 Heat를 가장 독립적인 다음 slice로 선정했다. WeaponGroup은 player-facing list/index 부재, VehicleBattery는 shared power Runtime 부재, WeaponCharge는 실제 Runtime/정적 입력 부재를 유지한다.
- `HeatDissipationPerSecond` 기본 0 + `FCFWeaponHeatRuntime`을 추가하고 실제 승인 발사 1회당 Heat 1회 누적, 자연 냉각, MaxHeat 과열과 next-shot-headroom 회복을 구현했다. existing saved WeaponData는 새 냉각값 0으로 Disabled여서 현재 동작이 자동 변경되지 않는다.
- HUD는 실제 WeaponComp Heat만 `ResourceChannels::Heat` Percent로 전달하며 Compact Projection은 Heat Secondary와 `Reload > NoAmmo > Overheated > Cooldown/READY` FireState를 사용한다. Launcher Primary + Ammo/Heat Secondary 최대 2와 기존 LauncherSequenceRevision lifecycle은 유지한다.
- final Official Build `0ecfed49ab3a4f41b349fc707c44e5f2` PASS, exact `HeatRuntimeResourceContract` `c736d1a6d6134a798a4b84452750e3d6` 1/1 PASS / Failure 0, Result SHA-256 `1abf0dc7a4788d6543c7933abef38433849ae57d569229cfab220f14937abad5`다. Content Asset mutation과 Heat tuning authoring, USER Visual PASS는 0이다.
- RPM/Stage A/B/UI-P0-03~05 완료 evidence는 반복하지 않았고 `CF-FQ-038` Paused 상태도 변경하지 않았다. UI-P0-06 전체는 계속 In Progress다.

### v1.54.32 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 RPM Gauge Production Visual Binding`을 Technical PASS로 반영했다. 기존 저장 `WBP_CFSpeedGauge`의 21 Tick 구조를 그대로 사용하고 Presenter가 explicit Redline/Maximum mapping 결과만 Runtime Percent에 적용한다.
- Redline 0/Unavailable/invalid는 `EngineMaxRPM` 기반 fallback 없이 21 Tick fill=0으로 fail-closed reset한다. persisted representative VehicleData의 Idle/Max는 Sedan 900/6500, TestSUV·DefenseSUV 900/6020이며 authoritative Redline source가 없어 모두 0 Unconfigured를 유지했고 Asset mutation은 0이다.
- 직전 test-only C4458 이름 충돌을 교정한 final Official Build `5ff6d404dfa44c61a85e698e27052db5` PASS, exact `CarFight.UI.UI_P0_06.RpmGaugeVisualBindingContract` process `7d32d08b7e554e9991fb64e80d472d78` 1/1 PASS / Failure 0, Result SHA-256 `0054abd391216065f732e274b00bc054478c5ab168608523504b614c860fc6d4`다.
- Stage A/B와 UI-P0-03~05 USER PASS는 반복하지 않았다. UI-P0-06 전체 Done이나 새 USER Visual PASS로 확대하지 않으며 실제 차량 Redline authoring·RPM USER Visual과 WeaponGroup/Battery/Charge/Heat Runtime은 계속 Pending이다.

### v1.54.31 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 RPM Gauge explicit Redline upstream contract`를 Technical PASS로 반영했다. `EngineMaxRPM`은 실제 Chaos 물리 상한, `RedlineStartRPM`은 별도 authored field이며 0은 미설정이다. `ChangeUpRPM`은 Current CarFight Source에 존재하지 않는다.
- Runtime source를 Current RPM=Chaos `GetEngineRotationSpeed()`, Redline/Maximum=VehicleData explicit authored value로 분리하고 Presenter는 실제 Redline을 0.85, EngineMaxRPM을 1.0 화면 위치로 mapping한다. Production SpeedGauge visual binding과 실제 차량 Redline 값 authoring은 Pending이다.
- Official Build `745dba430bcc47c2925c841a0c5a6686` PASS, 신규 exact `RpmGaugePresentationContract` `f805050484af47b8a2653cbb8ae2f639` 1/1 PASS, `RpmGaugeRuntimeSourceContract` `af1d9d010c2c433382819698a6dc7eeb` 1/1 PASS다.
- `RedlineStartRPM` additive leaf로 Current DAUTH compatibility surface는 118 Registry / Performance numeric 18 / 5 Profile numeric 79가 됐다. P0-08의 original 117/78 evidence는 Historical로 보존하고 bounded Registry/Batch compatibility 3건은 각 1/1 PASS했다. `CF-FQ-038` USER Acceptance는 Paused 상태 그대로다.
- Stage A 6/6, Stage B Dynamic Resource Visual, UI-P0-03~05 USER PASS는 반복하지 않았다.

### v1.54.30 - 2026-08-18

- `CF-FQ-032 / UI-P0-06 Dynamic Resource Visual Stage A` Presenter Projection을 Technical PASS로 반영했다. raw `ResourceChannels` 직접 0~N 렌더링을 금지하고 `Primary 최대 1 + Secondary 최대 2 + FireState 최대 1` Compact Presentation 계약을 고정했다.
- ReserveAmmo는 Header owner를 유지하고 현재 Runtime 없는 VehicleBattery·WeaponCharge·Heat는 생성·추정하지 않는다. Production Asset 변경은 0이며 다음 UI-only 범위는 Stage B Production Visual Container다.
- final Official Build `4c043a73e3054baab5f725b18cd7de6c` PASS, focused `CarFight.UI.UI_P0_06` process `73189888eb8042628db1b9499691c17c` 6/6 PASS / Failure 0, Result SHA-256 `39be662e7125ef4972574a75b433119283b7a279fbab56e511b140aafd6c70fe`를 Current evidence로 연결했다.
- `CF-FQ-038` Paused checkpoint는 더 최신 `DataAuthoringPlan.md v0.2.25 / DataAuthoringRoadmap.md v0.1.33 / USER PASS 2 / UA-03 setup 진행 중`을 보존한다.

### v1.54.29 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-02 Browser·Mesh-only·Create·Existing Import`을 사용자 직접 확인으로 USER PASS 처리했다.
- Feature는 Paused를 유지하면서 보존 USER checkpoint를 `PASS 2 / 다음 UA-03 Recipe·Shared Profile·Affected Vehicle Impact`로 전진시켰다.
- 대표 Plan/Roadmap projection을 `DataAuthoringPlan.md v0.2.24 / DataAuthoringRoadmap.md v0.1.32`로 갱신했다. DG/DEL/Wizard deletion은 계속 미개방이다.

### v1.54.22 - 2026-08-18

- `CF-FQ-032 / UI-P0-05 TargetSelect Marker Integration` USER Visual을 완료했다. 선택 Marker 정확히 1개, 내부 Actor 이름·거리·Track 중복 Text 0, 화면 밖 숨김+선택 유지, 화면 재진입 단일 Marker 복구, Clear/Reselect 단일 Marker 복구를 사용자 PIE로 PASS했다.
- 기존 Official Build `242dfee7186d46aaa98e3c24b25c0d09` PASS와 focused Automation `3834e606f2b54efb9ac5c09ea410b4f4` 1/1 PASS를 결합해 UI-P0-03~05 기존 HUD 데이터·수명 이전을 USER PASS로 닫았다.
- formal dependency에 따라 현재 Gate를 `UI-DESIGN-GATE D1-11 Structure PASS·Art Polish Deferred Non-Blocking → UI-P0-06 Vehicle·Weapon HUD`로 이동했다. `UI-P0-07 Target Knowledge → UI-P0-08 Radar`는 후속 순서를 유지한다.
- `CF-FQ-038`은 최신 `DataAuthoringPlan.md v0.2.20 / DataAuthoringRoadmap.md v0.1.28 / UA-01 USER PASS / USER PASS 1 / 다음 UA-02` Paused 상태를 변경하지 않았다.

Migration: 현재 CF-FQ-032 상세 기준은 `InGameUIPlan.md v0.58.9 / InGameUIRoadmap.md v0.26.3`이며 다음 작업은 UI-P0-06 착수 감사다. UI-P0-02~05 완료 검증은 관련 결함이 없는 한 반복하지 않는다.

### v1.54.21 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-01 Workspace First Impression`을 사용자 직접 재확인으로 USER PASS 처리했다.
- Feature 상태는 Paused를 유지하고 보존 USER checkpoint만 `PASS 1 / 다음 UA-02 Browser·Mesh-only·Create·Existing Import`로 전진시켰다.
- 대표 Plan/Roadmap은 `DataAuthoringPlan.md v0.2.20 / DataAuthoringRoadmap.md v0.1.28`이다. DG/DEL/Wizard deletion은 미개방이다.

### v1.54.20 - 2026-08-18

- `CF-FQ-032`의 UI-P0-04 AimReticle UISubsystem 통합을 USER PASS로 반영했다.
- UI-P0-05 TargetSelect Marker는 Source·Official Build·focused Automation PASS / USER Visual Pending으로 Current projection을 이동했다.
- `CF-FQ-038` DAUTH와 다른 Paused/Ready/Done 상태는 변경하지 않았다.

Migration: 현재 CF-FQ-032 Gate는 UI-P0-05 USER Visual이다. PASS 전 후속 TargetPanel/Radar 완료를 추정하지 않는다.

### v1.54.19 - 2026-08-18

- `CF-FQ-032 / UI-P0-03` Defense Production Panel과 Pawn Rebind USER Visual을 사용자 직접 화면 확인으로 PASS했다. Old Defense SUV 추가 피해·파괴 중 Current Pawn/HUD 무변화까지 확인해 Old Pawn Event Isolation도 USER PASS했다.
- UI-P0-03을 USER PASS로 닫고 현재 Gate를 `UI-P0-04 AimReticle UISubsystem 통합`으로 이동했다.
- UI-P0-04는 `UCFUISubsystem v1.6.0` HUD Layer 단일 Reticle 소유·Current Pawn Rebind·Cleanup, `CFVehiclePawn v2.148.0` direct Reticle CreateWidget/AddToViewport 자동 경로 제거, `DefaultAimReticleWidgetClass` Config와 focused Foundation contract 보강까지 Source Applied다. Official Build·Automation·USER Visual은 아직 Pending이다.
- `CF-FQ-038`은 `DataAuthoringPlan.md v0.2.17 / DataAuthoringRoadmap.md v0.1.25 / P0-12 UA-01 / USER PASS 0` Paused 상태를 유지한다.

Migration: 현재 주력은 `InGameUIPlan.md v0.58.7 / InGameUIRoadmap.md v0.26.1 / UI-P0-04`다. UI-P0-03은 관련 결함이 없는 한 반복하지 않고 UI-P0-04 PASS 전 UI-P0-05로 넘어가지 않는다.

### v1.54.18 - 2026-08-18

- 사용자 우선순위 변경에 따라 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`를 Paused에서 현재 단일 Active로 복원했다.
- UI는 `InGameUIPlan.md v0.58.6 / UI-P0-03 Defense Production Panel USER Visual → Pawn Rebind USER Visual`부터 재개하며 기존 UI-P0-02/03 기술·USER PASS를 반복하지 않는다.
- `CF-FQ-038`은 P0-08A~M/P0-09~11 Technical PASS와 `DAUTH-P0-12 USER PASS 0 / UA-01`을 보존한 Paused로 전환했다. DG/DEL Gate는 미개방 상태를 유지한다.
- `CF-FQ-037 Scanner` Done과 `SensorContact.md v1.1.0` Current를 TargetPanel/Radar 후속 UI의 Sensor source로 연결하되 Radar Range/Zoom·동적 Blip·USER Visual을 완료로 승격하지 않는다.

Migration: 다음 CarFight 주력 작업은 CF-FQ-032 UI다. DAUTH는 UI 우선 작업이 끝나거나 사용자가 명시적으로 재개하기 전 자동 Active로 올리지 않는다.

### v1.54.17 - 2026-08-18


- `CF-FQ-038 / DAUTH-P0-12 USER Authoring Acceptance`를 In Progress로 착수하고 `DataAuthoringPlan.md v0.2.16` / `DataAuthoringRoadmap.md v0.1.24`의 UA-01~08을 current USER Gate로 연결했다.
- P0-08A~M/P0-09~11 Technical PASS는 반복하지 않으며 USER PASS는 사용자 화면·입력·주행 판단 전까지 0이다.
- DG/DEL Gate와 Wizard physical deletion은 P0-12 완료 전까지 미개방이다.

### v1.54.16 - 2026-08-18

- `CF-FQ-037 차량 스캐너 입력·장비 통합`을 Paused에서 Done으로 전환했다. `SCAN-P0-00~07 PASS`, P0-06 USER PIE PASS, final Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 closure evidence로 사용한다.
- 현재 구현 owner를 `Document/Systems/Targeting/SensorContact.md v1.1.0`으로 연결하고 Scanner를 새 Active 후보에서 제거했다.
- Radar/TargetPanel 시각, Sensor energy/heat, AI/Network와 Missile/Utility 소비는 별도 후속 범위로 유지한다.

### v1.54.15 - 2026-08-18


- `CF-FQ-038 / DAUTH-P0-11 Frozen UX Completeness Closure`를 완료해 P0-11 overall을 Technical PASS로 갱신했다.
- Handling/Performance Adoption, Shared Profile B2 edit+affected Vehicle impact, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh normal Workspace production gap을 닫았다.
- final Build `98dfceab797942218bd09c844e398ceb` PASS, focused P0-11 `b53998e6cacf48a1a554d784b77e013c` 6/6 PASS, full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting production source, Content Asset, Wizard 삭제, DG/DEL은 변경/개방하지 않았다. `CF-FQ-038`은 P0-12 USER Acceptance가 남아 있어 Active를 유지하며 P0-12 자체는 이번 작업에서 시작하지 않았다.

### v1.54.14 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-11` Core Technical Validation PASS를 반영했다. representative Authoring E2E와 applied VehicleData Inventory/Fitting consumer regression을 추가하고 full Data Authoring 59/59 PASS를 확보했다.
- Frozen 24.90~24.94 전체 workflow audit에서 normal Workspace Handling/Performance Adoption, Shared Profile edit/impact, Drift 3-way recovery, Mesh-only Candidate/Create flow가 미구현임을 확인했다.
- 따라서 CF-FQ-038은 계속 Active, P0-11 overall은 In Progress / Completeness Blocked이며 P0-12는 Not Ready다.
- P0-08A~M/P0-09/P0-10 Technical PASS와 기존 Wizard 보존 상태는 유지한다. DG/DEL Gate는 열지 않았다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.14`, 다음 착수를 `DAUTH-P0-11 Frozen UX Completeness Closure`로 갱신했다.

Migration: P0-11 Core E2E/consumer 검증을 반복하지 않고 명시된 Frozen production gap을 우선 구현·검증한다.

### v1.54.13 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-10 Existing Wizard Migration`을 Technical PASS로 반영했다.
- 새 Vehicle Authoring Workspace에 Reference Compare, Assets/Layout semantic authoring, Measurement/Adoption, Frozen 4축 Driving Feel/preset, Stable-ID Mount/Defaults와 standard Undo를 parity했다.
- legacy `CarFight.VehicleDAWizard` / `SCFVDAWizardTab`은 유지하며 managed Recipe Target의 Layout Capture / Quick Tune Apply / Quick Tune Revert만 비활성화했다. DG/DEL Gate는 미개방이다.
- final Build `03fa78af4e124a3db33893ca8bfe436d` PASS, focused P0-10 4/4, full Data Authoring 57/57 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset, Batch main page는 변경하지 않았고 P0-10 USER UX/Driving Feel PASS는 미판정이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.13`, 다음 Gate를 `DAUTH-P0-11 Technical Validation`로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A~M/P0-09/P0-10을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.21 / P0-11`에서 시작한다. Wizard 삭제는 P0-11/P0-12 + DG/DEL Gate 전까지 금지한다.

### v1.54.12 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-09 Vehicle Authoring MVP`를 Technical PASS로 반영했다.
- single-Vehicle `CarFight.VehicleAuthoring` Workspace와 transient ViewModel을 추가하고 Initial Import, Resolve/Diff/Trace/Validation, shared Apply/standard Undo, Raw DA Open을 기술 검증했다.
- legacy `CarFight.VehicleDAWizard` / `SCFVDAWizardTab`은 P0-10 parity 전까지 그대로 유지한다. Batch main page는 만들지 않았다.
- final Build `76fc476c8ccb4daf895a3b567fe0c992` PASS, focused P0-09 6/6, full Data Authoring 53/53 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset은 변경하지 않았고 P0-09 USER Visual/Usability PASS는 미판정이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.12`, 다음 Gate를 `DAUTH-P0-10 Existing Wizard Migration`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A~M/P0-09를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.20 / P0-10`에서 시작한다. Wizard 삭제는 별도 DG/DEL Gate 전까지 금지한다.

### v1.54.11 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08M B3 Batch Definition Apply Foundation`을 Technical PASS로 반영하고 `DAUTH-P0-08 Implementation Foundation` 전체를 Technical PASS로 닫았다.
- fresh R3 evidence, exact B3 approval, all-target global preflight, canonical TargetPath 순서의 per-Vehicle `FCFVehicleApplyService` 호출과 Stop-On-First-Failure partial result를 구현했다.
- final Build `10027d18360e48f399a5b439f280c24c` PASS, focused M `a49d21b6b9b94d648142d5a555a79c95` 4/4 PASS, targeted `cb2ae69e9c834657a553fe52c00f5a96` 47/47 PASS를 current evidence로 연결했다.
- Target mutation authority는 `FCFVehicleApplyService` 하나로 유지하고 batch global transaction/rollback, already-applied rollback, retry/save는 0이다. Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Validator를 변경하지 않았다.
- Frozen 26.66~26.69 BatchOperationId/generic envelope는 첫 실제 external Batch transport/client integration 전 follow-up으로 보존한다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.11`, 다음 Gate를 `DAUTH-P0-09 Vehicle Authoring MVP`로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A~M과 P0-08 Foundation을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.19 / DAUTH-P0-09`에서 시작한다.

### v1.54.10 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`을 Technical PASS로 current feature projection에 반영했다.
- exact Batch approval binding, global source preflight, B1 Recipe/B2 shared Profile one-transaction source commit, all-or-nothing rollback과 approval invalidation을 구현했다.
- final Build `2e2923f31d5a4f3fa703f3b7623c0741` PASS와 targeted process `3853d42dfb344bfd8eb6e516903a8d78` 43/43 PASS를 current evidence로 연결했다.
- B1/B2는 source만 변경하며 Target mutation/Auto Save/B3/UI/file save는 아직 0이다. Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Validator를 변경하지 않았다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.10`, 다음 Gate를 `DAUTH-P0-08M B3 Batch Definition Apply Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A~L을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.18 / P0-08M`에서 시작한다. P0-08M은 Frozen 26.54~26.65 B3 구간에 새로 부여한 implementation checkpoint label이다.

### v1.54.9 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`을 Technical PASS로 current feature projection에 반영했다.
- manifest/current Registry verification, canonical CSV parse, current Unreal 3-way classification과 ownership conflict, transient Recipe/Profile prospective preview, deterministic BatchPlanHash를 구현했다.
- official Build `a3d37d4c8520442d8ceb09a72eb6a68f` PASS와 targeted process `7d2db5a9baa54dd9abf25857138d3994` 37/37 PASS를 current evidence로 연결했다.
- persistent Recipe/Profile/Target mutation은 0이며 Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Validator를 변경하지 않았다. B1/B2/B3/UI/file save도 아직 0이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.9`, 다음 Gate를 `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E/F/G/H/I/J/K를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.17 / P0-08L`에서 시작한다. B3는 Section 26.54+ 별도 후속이다.

### v1.54.8 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`을 Technical PASS로 current feature projection에 반영했다.
- existing Recipe/Profile typed schema와 117 Field Registry를 projection하는 Batch Registry, Recipe 7 + Profile 78 numeric allowlist, reserved `__cf_`, stable ColumnId를 구현했다.
- canonical BOM-less UTF-8 CSV와 `.cfbatch.json` baseline manifest/ExportSetHash foundation을 구현하고 persistent Recipe/Profile/Target mutation0을 확인했다.
- official Build `3249098c1b99487a8fd173694573bd5e` PASS와 targeted process `534486583a1d4689bc5137505a03f806` 32/32 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Validator를 변경하지 않았으며 Import Session/B1·B2/B3/UI/file save도 0이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.8`, 다음 Gate를 `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E/F/G/H/I/J를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.16 / P0-08K`에서 시작한다.

### v1.54.7 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`을 Technical PASS로 current feature projection에 반영했다.
- `FCFVehicleAuthoringService`와 typed AI contract로 R0~R3 risk/approval/result boundary를 구현하고 UI/AI가 기존 Snapshot/Resolver/Apply Core를 같은 경로로 호출할 기반을 만들었다.
- prospective `PreviewRecipeChange`는 persistent mutation 0이며 R1 commit은 exact AuthoringWrite/expected-state/proposal scope 아래 Recipe만 transaction mutation한다. bounded ClientOperationId dedupe는 duplicate write를 다시 실행하지 않는다.
- R3 `ApplyResolvedVehicle`는 ExpectedDiffHash와 DefinitionApply approval을 확인한 뒤 existing `FCFVehicleApplyService`를 actual 1회 호출한다. Raw SetField/direct Target writer/force/skip-validation/auto-retry/auto-save는 0이다.
- `VehicleArchetypeId`는 Frozen RecipeFingerprint에 임의 편입하지 않고 operation-specific typed desired-state equality로 commit/NoChange를 처리했다.
- official Build `252fdbd097f943379f2a1e2a942bedef` PASS와 targeted process `f2011a206c964fadb269d13c4dba3c86` 27/27 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Runtime Validator는 변경하지 않았고 UI/CSV/Batch도 0이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.7`, 다음 Gate를 `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E/F/G/H/I를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.15 / P0-08J`에서 시작한다.

### v1.54.6 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08H Apply Transaction Foundation`을 Technical PASS로 current feature projection에 반영했다.
- `FCFVehicleApplyService` 하나에 frozen TOCTOU precondition, fresh Resolve/Diff, transient preflight, dependency-safe exact Target write, readback/Validator, AppliedState와 rollback을 중앙화했다.
- actual Target mutation 후 controlled failure Automation까지 포함해 Target hash/stable arrays/Recipe AppliedState/dirty state rollback을 검증했으며 no-auto-save를 유지한다.
- official Build `b88831b71797415fac9dc0a23d12c39e` PASS와 targeted process `6f261c0b76f04d0faa6b9e855865818e` 23/23 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Runtime Validator는 변경하지 않았고 UI/CSV/Batch도 0이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.6`, 다음 Gate를 `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E/F/G/H를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.14 / P0-08I`에서 시작한다.

### v1.54.5 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08G Existing Definition Import / Adoption Foundation`을 Technical PASS로 current feature projection에 반영했다.
- Existing Definition exact field 전체를 lossless Legacy baseline으로 보존하고 Mount hidden serialized 10 leaf를 별도 passthrough로 분리하며 direct semantic candidate copy를 구현했다.
- Movement raw → Driving Feel/Profile inverse inference와 automatic Profile binding을 금지 상태로 유지하고 group/field Adoption Preview와 Recipe-only Commit을 fresh fingerprint precondition 아래 구현했다.
- official Build `b201d87cd13f4987a0907e08c8f00a6c` PASS와 targeted process `3b52a251ab244096b78bb872a06c0069` 20/20 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Runtime Validator는 변경하지 않았고 Target Definition mutation / Apply/UI/CSV도 0이다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.5`, 다음 Gate를 `DAUTH-P0-08H Apply Transaction Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E/F/G를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.13 / P0-08H`에서 시작한다.

### v1.54.4 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08F Definition Materializer / Validation Foundation`을 Technical PASS로 current feature projection에 반영했다.
- RF_Transient candidate, Stable-ID array reconstruction, FieldCodec import, existing UCFVDAValidator, DefinitionValidation과 Resolver-owned readback hash consistency를 구현했다.
- official Build `e5d925c6531e4ae6ac58aa356e4078eb` PASS와 targeted process `4b21d25b780c4a398de7a1b47c595c5e` 17/17 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Runtime Validator는 변경하지 않았고 Apply/UI/CSV는 구현하지 않았다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.4`, 다음 Gate를 `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E/F를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.12 / P0-08G`에서 시작한다.

### v1.54.3 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08E Pure Resolver Foundation`을 Technical PASS로 current feature projection에 반영했다.
- Frozen R0~R16 stage identity, deterministic source precedence, Proposal/Adoption, Source Trace/Hash, R14 Diff와 R16 Stale/Drift foundation을 실제 C++로 구현했다.
- official Build `db202f0797af41fe86a859609cb4dfd9` PASS와 targeted process `93d445f78cbe4eaabca1478eb6d27078` 15/15 PASS를 current evidence로 연결했다.
- R15 Materializer/Validator는 다음 P0-08F로 남겼으며 Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, Apply/UI/CSV는 변경하지 않았다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.3`, 다음 Gate를 `DAUTH-P0-08F Definition Materializer / Validation Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D/E를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.11 / P0-08F`에서 시작한다.

### v1.54.2 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08D Asset Snapshot Reader Foundation`을 Technical PASS로 current feature projection에 반영했다.
- Chassis requested socket relative transform facts, Wheel local bounds와 Section 22.27 resolver-relevant asset fingerprints를 구현했다.
- official Build `780d30c4a44f48feb179f7f6da5480e1` PASS와 targeted process `ab2cd64308484ce0ae0e24cb1143ca33` 9/9 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset은 변경하지 않았고 Resolver / Apply / UI / CSV는 아직 구현하지 않았다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.2`, 다음 Gate를 `DAUTH-P0-08E Pure Resolver Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C/D를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.10 / P0-08E`에서 시작한다.

### v1.54.1 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08C Immutable Snapshot Foundation`을 Technical PASS로 current feature projection에 반영했다.
- Recipe / 5 Profile / Definition / Project Compatibility Default Snapshot, Registry-expanded Stable-ID field entry, deterministic fingerprint/hash와 Section 22.17 `RequiredDependencies`를 구현했다.
- final official Build `6c81b2149de44d00a99554a44d2135aa` PASS와 targeted process `340050d2c911445da3634ebb92acc014` 8/8 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset은 변경하지 않았고 Asset Snapshot Reader / Resolver / Apply / UI / CSV는 아직 구현하지 않았다.
- 대표 Plan을 `DataAuthoringPlan.md v0.2.1`, 다음 Gate를 `DAUTH-P0-08D Asset Snapshot Reader Foundation`으로 갱신했다.

Migration: CF-FQ-038 재개 시 P0-08A/B/C를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.9 / P0-08D`에서 시작한다.

### v1.54.0 - 2026-08-17

- 사용자 요청에 따라 `CF-FQ-038 차량 데이터 Authoring 시스템`을 P2 / Active로 정식 등록했다.
- 대표 Plan은 `DataAuthoringPlan.md v0.2.0`, 현재 구현 Gate는 `DAUTH-P0-08 Implementation Foundation`이다.
- `DAUTH-P0-08A/B`에서 Editor-only Recipe + 5 Profile, `IsEditorOnly()` Never-Cook, common types, Stable Field Path / Field Value Codec와 Current `UCFVehicleData` 117 leaf Registry 양방향 coverage를 구현했다.
- final official Build `5c745a31d19b448d9b1049877bc877ca` PASS와 targeted process `ab9de3e8c8cd410a8de0641178690dff` 4/4 PASS를 현재 기술 evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting 소비 경로, `SCFVDAWizardTab`, Content Asset은 변경하지 않았다.
- `CF-FQ-034`는 Done으로 확대하지 않고 `FIT-P0-07D USER Driving Feel Comparison` 체크포인트를 보존한 Paused로 전환했다.
- 다음 CF-FQ-038 Gate는 `DAUTH-P0-08C Immutable Snapshot Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B를 반복하지 않고 DataAuthoringRoadmap의 P0-08C에서 재개한다. CF-FQ-034는 VehicleFittingPlan v0.17.0 / FIT-P0-07D에서 재개한다.

### v1.53.12 - 2026-08-17

- CF-FQ-034 `FIT-P0-07C Quantitative Mobility Measurement`을 Technical Complete로 반영했다.
- Light/Default/Heavy 공식 Fixture를 각각 fresh PIE lifetime에서 동일 프로토콜로 계측해 실제 Chaos Mobility baseline 3종을 확보했다.
- final official Build `bb04d56e0cd24647b2ef6fcbc7f2bd77` PASS와 process `63bbeaf0202041b79dac8f32f5447923` Success / Metric 3/3을 현재 기술 evidence로 연결했다.
- 가속·coast-steering의 비단조 결과에 임의 합격 기준이나 자동 Mobility Scalar를 추가하지 않고 `ordering_asserted=false`, USER 주행감 Pending을 유지했다.
- CF-FQ-034는 계속 Active이며 다음 Gate는 `FIT-P0-07D USER Driving Feel Comparison`이다. Field UI·PIE도 Pending으로 유지한다.

Migration: CF-FQ-034 재개 시 FIT-P0-07A~07C와 기존 Fitting 23/23을 반복하지 않는다. `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D`에서 같은 공식 Fixture의 사용자 직접 주행감만 비교한다.

### v1.53.11 - 2026-08-17

- CF-FQ-034 `FIT-P0-07B Heavy Payload Resolution + Official Fixture Preparation`을 Technical Complete로 반영했다.
- 같은 `DA_VehicleDefense_TestSUV` 플랫폼의 Light 1000kg / Default 1570kg / Heavy 1600kg 공식 Fixture 3종과 persisted 사용자 표시명·참조를 고정했다.
- `verify_existing` PASS, fresh AssetDump persisted readback와 post-label `OfficialMobilityFixtures` 1/1 PASS를 현재 기술 evidence로 연결했다.
- CF-FQ-034는 계속 Active이며 다음 Gate는 `FIT-P0-07C Quantitative Mobility Measurement`다. USER Mobility·Field UI·PIE는 Pending으로 유지한다.

### v1.53.10 - 2026-08-17

- 사용자 선택에 따라 CF-FQ-034 차량 피팅·질량 런타임을 Ready에서 Active로 전환했다.
- `VehicleFittingPlan.md v0.15.0 / FIT-P0-07A Fixture Readiness Audit` 완료를 반영했다.
- current AssetDump에서 CityCar·Compact·Coupe·Pickup·SubCompact·Van·Wagon을 StaticMesh-only Visual 후보로, DA_TestSedan·DA_TestSUV를 Base/Gross 0kg VehicleData baseline으로 분리했다.
- `DA_VehicleDefense_TestSUV`는 Base 1000kg / Gross 2500kg의 Fitting-ready technical platform이고 기존 `DA_Fit_DefenseTestSUV` 1570kg baseline이 저장 계약과 일치함을 확인했다.
- 같은 플랫폼에서 Light 1000kg과 Default 1570kg은 임의 질량 없이 준비 가능하며 Heavy는 1570kg 초과 persisted payload 확인을 다음 Gate로 유지한다.
- Source·Content Asset·Build·USER PIE 변경/실행은 0이다.

### v1.53.9 - 2026-08-17

- CF-FQ-030 Persisted Missile Test Asset Technical Verification을 PASS로 반영하고 대표 Plan을 `MissileGuidancePlan.md v0.5.0`으로 갱신했다.
- fresh `CarFight.Missile.MG_P0_01_04.DirectRuntimeContract` Process `29d0ef16dd5e4d56945ceae26eec6937` 1/1 PASS를 보호 회귀로 기록했다.
- current AssetDump에서 Missile test folder 20/20, Maps World 12/12를 성공했고 4개 계약 DataAsset의 저장값과 Vehicle → EquipmentPreset → Weapon → Projectile hard reference chain을 확인했다.
- MissileDirectTest World package는 33 Actor와 Missile test vehicle/RocketLauncher socket 구성을 확인했으나 전체 Actor label은 public readback 범위 밖이므로 다섯 MissileTarget label 자체를 독립 PASS로 추정하지 않았다.
- Runtime Source·Content Asset·Blueprint·Map 저장 변경과 USER PIE는 0이며 Source 변경이 없어 새 Build는 수행하지 않았다.
- CF-FQ-030은 Done으로 승격하지 않고 Ready / CF-TC-027 Manual PIE Pending을 유지하며 현재 단일 Active는 없다.

### v1.53.8 - 2026-08-16

- CF-FQ-029 `LM-P0-06A Failure Policy Technical Closure`를 Technical PASS로 반영했다.
- 최종 closure 공식 Build `6a633eb4cf21481d8ae26ec908d05660` PASS, `CarFight.Launcher` Process `7ab7a286e5484cab845bb0cbfd5ac04e` 4/4 PASS와 `AMMO_P0_04.LauncherLock` Process `1781accd3d9c47a59421fb1d2228e4ab` 1/1 PASS를 closure evidence로 연결했다.
- Launcher Runtime은 재구현하지 않았고 `CFLauncherSchedTests.cpp v1.2.0`에 StopSequence terminal 이후 추가 Dispatch 불가 보호 assert만 보강했다.
- CF-FQ-029 상태를 Active에서 Paused로 전환하고 `LauncherMissilePlan.md v0.14.0 / LM-P0-06 USER PIE`를 재개 체크포인트로 고정했다.
- MuzzleBlocked·Angled/Vertical Ejection·Carrier Velocity USER PIE와 CF-FQ-037 `SCAN-P0-06 USER PIE`는 Pending을 유지하며 현재 단일 Active는 비웠다.

### v1.53.7 - 2026-08-16

- 사용자 선택에 따라 CF-FQ-037을 `SCAN-P0-06 USER PIE` 체크포인트가 보존된 Paused로 전환하고 CF-FQ-029를 단일 Active로 승격했다.
- CF-FQ-029의 현재 기술 Gate를 `LauncherMissilePlan.md v0.13.0 / LM-P0-06A Failure Policy Technical Closure`로 고정했다.
- `ContinueRemaining / StopSequence`는 기존 Launcher Sequence Runtime을 재구현하지 않고 asset-free Automation으로 검증하며, 기존 Manual Cancel + Ammo Reservation cleanup은 보호 회귀로 유지한다.
- MuzzleBlocked·Ejection·Carrier Velocity USER PIE와 CF-FQ-037 USER PIE는 완료로 추정하지 않는다.

### v1.53.6 - 2026-08-16

- CF-FQ-037 대표 Plan을 `ScannerIntegrationPlan.md v0.7.1`로 동기화했다.
- `P0-06A` Scanner 전용 PIE fixture readiness 완료를 FeatureQueue current projection에 반영했다.
- CF-FQ-037은 계속 Active이며 `SCAN-P0-06 USER PIE Acceptance`와 USER PIE PASS 0 상태를 유지한다.
- P0-04/05 Technical evidence는 반복하지 않고 미구현 Field Fitting USER UI를 CF-FQ-037 P0-06 완료 조건으로 승격하지 않는다.

### v1.53.5 - 2026-08-16

- `CF-FQ-037 SCAN-P0-05 Technical Validation`을 Technical Done으로 반영하고 대표 Plan을 `ScannerIntegrationPlan.md v0.7.0`, 현재 Gate를 `SCAN-P0-06 USER PIE Acceptance`로 갱신했다.
- P0-04 final Build `20ca14bdc6094650bc11209f4ce320fa`, FittingIntegration 1/1, CarFight.Fitting 22/22, CarFight.Sensor 14/14, CarFight.Inventory 12/12 PASS를 current-source Acceptance evidence로 반복 없이 재사용했다.
- fresh asset-free `CarFight.TargetSelect.TS_P0_01.RuntimeContract` process `19077228b82f4c37a1113c51fe18eaa7` 1/1 PASS를 추가했다.
- 정적 감사에서 Scanner Enhanced Input은 Pawn만 소유하고 Sensor/Fitting에는 Input bind가 없으며 TargetSelect에는 SensorData·SensorConfig·ActiveScan·ApplySensorRuntime 소유권 침범이 없음을 확인했다.
- TargetSelect Input/HUD 저장 가능 테스트는 dirty Content 보호를 위해 실행하지 않았으며 해당 Asset/USER Gate를 완료로 추정하지 않는다.
- P0-06은 실제 `V` 입력, timed Active Scan, 반복 입력, scanner-less 처리, 장비 변경 후 성능 반영과 Contact/Knowledge 보존의 사용자 직접 PIE만 소유한다.

### v1.53.4 - 2026-08-16

- `CF-FQ-037 SCAN-P0-04 Fitting Integration`을 Technical Done으로 반영하고 대표 Plan을 `ScannerIntegrationPlan.md v0.6.0`, 현재 Gate를 `SCAN-P0-05 Technical Validation`으로 갱신했다.
- `ResolvedSensorData → ApplySensorData()`를 기존 Fitting Runtime Commit/Checkpoint/Compensation의 Sensor participant로 통합하고 초기 Source 선택, Runtime Ready hot reapply, scanner-less Fallback, Legacy Source 보존과 실패 복원을 확정했다.
- Scanner Utility mount가 Weapon 후보로 오인되지 않도록 실제 WeaponData가 있는 Resolved Mount만 Weapon Runtime 후보로 선택하는 경계를 반영했다.
- final official Build `20ca14bdc6094650bc11209f4ce320fa` PASS와 FittingIntegration 1/1, CarFight.Fitting 22/22, CarFight.Sensor 14/14, CarFight.Inventory 12/12 PASS를 P0-04 closure evidence로 연결했다.
- P0-05는 위 검증을 반복하지 않고 현재 Source의 asset-free TargetSelect 보호 회귀와 Scanner/Sensor/Fitting/TargetSelect 책임 경계 Technical Acceptance를 수행한다.
- USER PIE와 실제 `V` 조작감·탐지 체감은 P0-06에 유지하며 기존 Paused/Ready/USER Pending 상태를 완료로 추정하지 않는다.

### v1.53.3 - 2026-08-16

- `CF-FQ-037 SCAN-P0-03 Input Command Integration`을 Technical Done으로 반영하고 대표 Plan을 `ScannerIntegrationPlan.md v0.5.0`, 현재 Gate를 `SCAN-P0-04 Fitting Integration`으로 갱신했다.
- P0 Scanner 입력을 `/Game/CarFight/Input/IA_ActiveScan` Boolean + Pressed와 `IMC_Vehicle_Default`의 `V` 단일 매핑으로 확정했다. 1회 입력 후 `ActiveScanDurationSec` 동안 실행되고 자동 종료하며 별도 Stop 키는 만들지 않았다.
- `ACFVehiclePawn`이 Enhanced Input bind와 Sensor Gameplay command 변환을 소유하고 Sensor Component 직접 Input bind를 금지하는 책임 경계를 유지했다.
- official Build `b84e8dabcb784b30a005fb120af5cf4d` PASS와 process `4d9a1e5ad12f416db18b1a069694c241` PASS, InputAsset 1/1·InputCommand 1/1·Sensor 14/14·TargetSelect RuntimeContract 1/1을 P0-03 closure evidence로 연결했다.
- Persisted AssetDump에서 IA_ActiveScan Boolean/Pressed와 `IA_ActiveScan <- V` 저장 상태를 재확인했다.
- USER PIE PASS는 추가하지 않았고 실제 Scanner 조작감은 P0-06에 유지한다. CF-FQ-037 전체는 P0-04~07이 남아 있으므로 `Active`를 유지한다.

### v1.53.2 - 2026-08-16

- `CF-FQ-037 SCAN-P0-02 Runtime Config Apply`를 Technical Done으로 반영하고 대표 Plan을 `ScannerIntegrationPlan.md v0.4.0`, 현재 Gate를 `SCAN-P0-03 Input Command Integration`으로 갱신했다.
- `UCFVehicleSensorComp::ApplySensorData()`의 non-destructive hot reapply, Applied Config 사본, invalid 원자 거부, scanner-less fallback, Contact/Knowledge/Analysis 보존과 range 감소 lifecycle reconcile을 현재 기술 계약으로 확정했다.
- official Build `d670926be7c4498dbd15b12601f8505b` PASS, `CarFight.Scanner.SCAN_P0_02.ConfigApply` 1/1 PASS와 `CarFight.Sensor` 14/14 PASS를 P0-02 closure evidence로 연결했다.
- Content Asset·Blueprint·InputAction·Scanner tuning·PIE mutation과 USER PASS 추가는 0이며 실제 FittingSnapshot/Field Fitting Sensor 통합은 P0-04에 유지한다.
- CF-FQ-037은 기능 전체가 아직 진행 중이므로 `Active`를 유지하고 기존 Paused/Ready/USER Pending 기능 상태를 변경하지 않았다.

### v1.53.1 - 2026-08-16

- `CF-FQ-037 SCAN-P0-00 Foundation Audit`을 Source·Asset·Build·PIE mutation 0으로 Technical Done 처리했다.
- Enhanced Input owner는 `ACFVehiclePawn`, Scanner 정적 성능 payload는 기존 `UCFVehicleSensorData`, Sensor Runtime owner는 `UCFVehicleSensorComp`로 확정했다.
- `UCFVehicleData`에는 SensorData 참조가 없고, `ECFVehicleMountType::Utility`는 존재하지만 현재 `EquipmentPresetData/FittingSnapshot`이 Turret+Weapon 중심이어서 Scanner를 그대로 표현할 수 없음을 확인했다.
- 새 Scanner DataAsset을 만들지 않고 기존 `UCFEquipmentPresetData → FittingSnapshot` 경로가 SensorData payload를 운반하도록 최소 확장하는 `SCAN-P0-01 Scanner Data Contract`를 다음 Gate로 지정했다.
- hot reapply와 InputAction 연결은 각각 P0-02/P0-03 이후로 분리하고 기존 USER Pending/dirty work를 그대로 보호했다.

### v1.53.0 - 2026-08-16

- 사용자 선택에 따라 `CF-FQ-037 차량 스캐너 입력·장비 통합`을 P1 / Active로 신규 등록했다.
- 대표 Plan은 `ScannerIntegrationPlan.md v0.1.0`, 첫 Gate는 `SCAN-P0-00 Foundation Audit`이다.
- 완료된 CF-FQ-036 SensorContact Runtime을 재설계하지 않고 Scanner 장비/설정 Source와 플레이어 입력 command를 연결하는 별도 후속 Feature로 분리했다.
- Foundation Audit 전에는 새 Scanner DataAsset 클래스, 임의 Sensor tuning 값, InputAction asset을 만들지 않는다.
- Radar/TargetPanel, Sensor energy/heat, Missile/Utility 소비, AI/Network는 이번 Feature 범위에서 제외했다.
- 기존 CF-FQ-015/026/029/032/035 Paused, CF-FQ-030/034 Ready와 모든 USER Pending 체크포인트를 보존했다.

### v1.52.17 - 2026-08-15

- `CF-FQ-036 SEN-P0-07 Technical Acceptance` PASS와 `Document/Systems/Targeting/SensorContact.md v1.0.0` Current System 승격을 FeatureQueue에 반영했다.
- CF-FQ-036 상태를 Active에서 Done으로 전환하고 현재 단일 Active를 비웠다. Paused/Ready 기능은 사용자 선택 없이 자동 승격하지 않는다.
- Acceptance는 기존 Build `7dff9da7aaa24e76b0871348762c9d93` PASS, Sensor `85d008613a104df9b7107795995c6ac5` 14/14 PASS와 UI asset-free 1/1+1/1 PASS를 재사용했으며 Source 의미 변경이 없어 반복 실행하지 않았다.
- Sensor/TargetSelect/HUD 책임 경계, actor-free public contract, Contact lifecycle·Knowledge·DestroyedHold·Snapshot integration 정적 감사가 PASS였음을 완료 기준으로 기록했다.
- CF-FQ-032 Radar/TargetPanel USER Visual, Radar Range/Zoom·동적 Blip, CF-FQ-026 TS-P0-08 USER PIE와 다른 USER Pending 체크포인트는 완료로 추정하지 않았다.
- 대표 완료 Plan은 `SensorContactPlan.md v0.9.0` Historical + Retained Path로 보존한다.

### v1.52.16 - 2026-08-15

- `CF-FQ-036 SEN-P0-06 Public Snapshot Integration` Technical Done 상태를 현재 우선순위 projection에 동기화했다.
- TargetSelect가 선택·TrackState owner를 유지한 채 Target Knowledge/Radar Contact는 actor-free `FCFSensorSnapshot`을 read-only source로 소비하도록 HUD Provider/ViewData 경계를 연결했다.
- Detected identity 비누출을 위한 Actor→ContactId association-only bridge, Snapshot 기반 상대 위치·거리, Radar normalized position Unavailable 계약을 반영했다.
- 정적 Radar placeholder Canvas는 실제 Sensor Contact로 노출하지 않으며 Blueprint/Content Asset과 기존 CF-FQ-032 USER Visual gate는 변경하지 않았다.
- 최종 Build `7dff9da7aaa24e76b0871348762c9d93` PASS와 `CarFight.Sensor` `85d008613a104df9b7107795995c6ac5` 14/14 PASS를 현재 기술 증거로 연결했다.
- 관련 UI asset-free 보호 회귀 `67d114255ec943bb8ded2642a40a581c`, `395c93df5fb947a1a979ca8fa0c74665` 각각 1/1 PASS를 기록했다.
- 대표 Plan을 `SensorContactPlan.md v0.8.0`, 다음 Gate를 `SEN-P0-07 Technical Acceptance`로 갱신했다.
- 문서 버전 관리 섹션도 v1.52.16으로 동기화했다.

### v1.52.15 - 2026-08-15

- `CF-FQ-036 SEN-P0-01~05` Technical Done 상태를 현재 우선순위 projection에 동기화했다.
- P0-05에서 `UCFVehicleHealthComp::OnVehicleDestroyed / IsDestroyed`를 authoritative destruction truth로 소비하는 Sensor 독립 DestroyedHold를 구현하고 TargetSelect의 Destroyed 즉시 clear를 유지했다.
- official Build `954dc0ab844b40f48f2674f2a0ae672a` PASS와 `CarFight.Sensor` Process `4ba274cc90814e5b90d40c33820b3bd0` 13/13 PASS를 현재 기술 증거로 연결했다.
- 대표 Plan을 `SensorContactPlan.md v0.7.0`, 다음 Gate를 `SEN-P0-06 Public Snapshot Integration`으로 갱신했다.
- TargetSelect/HUD/Content Asset/Project Config와 기존 USER Pending 상태는 변경하지 않았다.
- 문서 버전 관리 섹션의 stale current version 표기도 v1.52.15로 교정했다.

### v1.52.14 - 2026-08-15

- 사용자 결정에 따라 `CF-FQ-036 차량 센서·Contact Intelligence Runtime`을 P1 / Active로 신규 등록했다.
- 대표 Plan은 `SensorContactPlan.md v0.1.0`, 첫 Gate는 `SEN-P0-00 Foundation Audit`이며 이번 인계 준비에서는 구현을 시작하지 않는다.
- TargetSelect·UI USER Pending과 기존 dirty 자산을 보호하면서 실제 Sensor Runtime Provider를 별도 Gameplay owner로 만드는 방향을 고정했다.

### v1.52.13 - 2026-08-15

- CF-FQ-008 WD-P0-03 Current System Integration을 완료하고 기능 상태를 Done으로 전환했다.
- `Document/Systems/Combat/WeaponData.md v1.0.0`을 현재 구현 owner로 등록하고 `WeaponFire.md v1.6.0`의 Ammo 도입 전 stale 설명을 교정했다.
- WeaponData Content Asset mutation 0, 최종 Build PASS와 targeted 2/2 PASS 증거를 유지한다.
- 현재 단일 Active를 비우고 다음 Feature는 사용자 선택 전 자동 승격하지 않는다.
- CF-FQ-026 상세 섹션에 남아 있던 stale `TS-P0-08 Active` 표기 2곳을 상단 상태와 동일한 `Paused / USER PIE Pending`으로 교정했으며 USER PASS는 추가하지 않았다.

### v1.52.12 - 2026-08-15

- CF-FQ-008 WD-P0-01 Static Data Contract와 WD-P0-02 Representative Assets를 Technical Done으로 반영했다.
- 최종 Build `53e2dbaf04a3401b8ed89c906b308cdc` PASS와 `CarFight.WeaponData` `eefe58aa87d74dcaa79a1764a5f7611b` 2/2 PASS를 기록했다.
- 대표 WeaponData는 Load-only로 검증했고 Content Asset mutation은 0이다. 다음 Gate는 WD-P0-03 Current System Integration이다.

### v1.52.11 - 2026-08-15

- CF-FQ-015를 VD-P0-00~03 Remote Technical Done / VD-P0-04 USER Tuning Pending 체크포인트가 보존된 Paused로 전환했다.
- `CF-FQ-008 무장 데이터 정의`를 단일 Active로 승격하고 WD-P0-01 Static Data Contract를 next gate로 등록했다.
- 기존 WeaponData Content Asset과 Runtime 상태를 수정하지 않고 정적 DataValidation부터 진행하도록 고정했다.

### v1.52.10 - 2026-08-15

- CF-FQ-015 VD-P0-00~03 Remote Technical Done, Build PASS와 VehicleData Automation 3/3 PASS를 반영했다.
- VehicleData Current System 문서를 v2.0.0으로 갱신하고 UCFVDAValidator, Representative Compare, 실제 Pawn Runtime Apply 계약을 현재 구현과 맞췄다.
- VehicleData Content Asset 값은 변경하지 않았으며 남은 VD-P0-04 USER Tuning은 PIE 가능 시점까지 Pending으로 보존한다.

### v1.52.9 - 2026-08-15

- 사용자가 PIE를 직접 확인할 수 없는 기간의 원격 작업으로 `CF-FQ-015 차량 데이터 튜닝 패스`를 단일 Active로 승격했다.
- `CF-FQ-026 TargetSelect`는 TS-P0-08 Remote Technical 3/3 + LOS Prefilter PASS와 기존 USER 체크포인트를 보존한 Paused로 전환했다.
- CF-FQ-015의 첫 Gate를 `VD-P0-01 Validator Contract`로 고정하고 DA_TestSedan/DA_TestSUV 실제 튜닝값 변경은 USER PIE 전 금지했다.

### v1.52.8 - 2026-08-15

- TS-P0-08 최신 Build PASS와 SearchDiagnostics·SettingsPath·SingleTargetBoundary 3/3 PASS를 반영했다.
- 저장된 BP CDO가 실제 fallback 7도·1200m·15% 설정과 SM_Body TargetPoint 자동 정렬을 사용하는 것을 확정했다.
- 런타임 검색 진단과 후보 의미를 보존하는 LOS 사전필터를 적용해 테스트 조건에서 Input 3개를 유지하면서 LOS 2건을 생략했다.
- 작은 Transient 검색 ms는 성능 PASS로 해석하지 않고 20Hz 전체 Actor 순회는 대표 workload 전 구조 교체를 보류한다.
- 기존 USER 체크포인트와 dirty WBP 보호는 변경하지 않았다.

### v1.52.7 - 2026-08-15

- 사용자 선택에 따라 CF-FQ-026 TargetSelect / TS-P0-08을 단일 Active로 재개하고 CF-FQ-035는 USER Field UI·Mobility 체크포인트가 보존된 Paused로 전환했다.
- TargetSelect 공식 Build PASS, TS-P0-08 SingleTargetBoundary 1/1과 자산 비변경 TS-P0-01·02·03·04·07 각 1/1 PASS를 반영했다.
- 자동 후보 debug Sphere의 표시 수명·Debug 반경 계약을 보강했지만 후보 게임플레이 수치는 변경하지 않았다.
- 기존 USER PASS·FAIL·재검증 항목은 그대로 보존하고 새 USER PASS를 추정하지 않았다.

### v1.52.6 - 2026-08-15

- `FIT-P0-06 Fitting ViewData and Debug` C++ ViewData·Blueprint Contract Technical PASS와 Full CarFight 84/84 Success를 반영했다.
- CF-FQ-035/034 내부의 현재 원격 기술 선행 Gate를 완료 상태로 정리했다.
- 남은 Fitting/Inventory 범위를 FFIT-P0-05 Field UI·PIE, 16:9·32:9 실제 화면 가독성, 공인 Light·Default·Heavy Mobility·USER 실제 사용 검증으로 한정했다.
- CF-FQ-035 Active는 유지하되 다음 원격 Feature 전환은 사용자 선택 없이 자동 수행하지 않는다.

### v1.52.5 - 2026-08-15

- cross-feature `FFIT-P0-01~04` Technical Integration과 실제 Chaos Field Mass Reapply PASS를 반영했다.
- 최종 Build `cd7207084dfd49c4a28b607d8577307a`, FFIT-P0-04 4/4, Fitting 21/21, Inventory 12/12, Full CarFight 83/83 Success를 기록했다.
- 다음 원격 기술 Gate를 `FIT-P0-06 Fitting ViewData and Debug`로 이동했다.
- CF-FQ-034는 Ready 상태를 유지하고 CF-FQ-035 Active의 cross-feature dependency로 기술 작업을 계속한다. USER Mobility·Field UI·PIE는 Pending이다.

### v1.52.4 - 2026-08-14

- `CF-FQ-035 INV-P0-06` Coordinator Foundation의 원자 completion·보상 Rollback을 Technical PASS로 기록했다.
- 최종 Build `e2ef556b64484a09ba8b3544c62344e3`, M6 3/3, Inventory 12/12, Full CarFight 71/71 Success를 반영했다.
- M6 전체 Done이나 formal FFIT-P0-03 완료로 승격하지 않는다. mass-changing Field Runtime은 FFIT-P0-04 전까지 명시 거부한다.
- current next gate를 cross-feature `CF-FQ-034 FFIT-P0-01 Permission and Blocker Query`로 이동했다.
- CF-FQ-032 USER Visual Paused 체크포인트는 계속 보호한다.

### v1.52.3 - 2026-08-14

- 사용자가 직접 USER Visual을 수행할 수 없는 기간 동안 기술 개발을 계속하기로 결정해 `CF-FQ-032`를 시각 확인 체크포인트가 보존된 Paused로 전환하고 `CF-FQ-035`를 단일 Active로 전환했다.
- `CF-FQ-035 INV-P0-05 / M5`의 읽기 전용 Inventory Snapshot·Reservation 표시·Fitting Hint 합성·의미 ChangeSet을 완료했다.
- 공식 Build `30503595ba074c63ba8a6bb87f7a2645`, Inventory 9/9, 전체 CarFight 68/68 Success를 반영했다.
- 다음 Gate는 `INV-P0-06 / M6 Integration Verification`과 Field Fitting Coordinator 기술 통합이다.
- `CF-FQ-032` Defense/Pawn Rebind USER Visual은 취소되거나 완료 처리되지 않았으며 사용자 직접 확인 가능 시 같은 체크포인트에서 재개한다.

### v1.52.2 - 2026-08-14

- `CF-FQ-032`은 계속 Active이며 완료 상태로 승격하지 않았다.
- `CFHUDDataTests v1.6.0` 보강과 최종 공식 Build `681c91810da34066bb398ad1b0989f1e` Exit Code 0을 반영했다.
- targeted `CarFight.UI.UI_P0_03` Automation `f6b08a970bbf4ec282893e86e92e72ac`에서 5/5 Success·0 Fail을 확인해 실제 Defense 저장 맵 PIE→Provider ViewData와 Pawn Rebind Old Pawn 이벤트 해제를 기술적으로 검증했다.
- 사용자가 직접 화면을 확인하지 않았으므로 Defense/Pawn Rebind는 USER Visual Pending으로 유지하며 다음 Gate를 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`로 고정했다.
- UI-P0-03 USER Gate 완료 전 Systems 승격과 UI-P0-04 Source 착수는 하지 않는다.

### v1.52.0 - 2026-08-13

- TestMap_AmmoRipple의 시작 3/4+Reserve4, 실제 3발 Partial Ripple, Auto Reload, 두 번째 4발 Ripple과 최종 NO AMMO를 사용자 전 항목 PASS로 반영했다.
- `CF-FQ-031`을 Ready에서 Done으로 전환하고 `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System 완료 위치로 등록했다.
- 현재 단일 Active는 `CF-FQ-032`이며 UI-P0-03의 남은 Launcher 전체 Presentation, Defense 실제 변화와 Pawn Rebind를 다음 검증으로 고정했다.
- finite Ammo는 실제 UI Runtime 연결이 완료됐으므로 더 이상 미구현 HUD Provider 항목으로 취급하지 않는다.
- CF-FQ-029 Launcher 전체 기능은 Paused 상태를 유지하며 Ammo 완료로 자동 Done 처리하지 않는다.

### v1.51.0 - 2026-08-06

- CF-FQ-032 UI-P0-02의 Pause UI 상호작용 범위를 사용자 Standalone PASS로 기록했다.
- `AddToViewport(100)`, 계속하기 마우스 클릭과 Enter·Escape 해제 경로를 현재 구현 기준으로 반영했다.
- 사용자 직접 Editor 빌드와 Combat Automation `595e0c1ca0f640b69f63964bdfd22277` 전체 46/46·필수 24/24 Success를 기록했다.
- 남은 Gamepad·입력 유지 잔류·Launcher·Projectile·Timer·Root 수명 검증 때문에 CF-FQ-032는 Active / UI-P0-02 Partial USER PASS로 유지하고 UI-P0-03은 시작하지 않았다.

### v1.50.0 - 2026-08-06

- 사용자 DR-PIE-06의 Salvo·Ripple·Pool·FX·충돌 PASS와 DR-P0-07 완료를 반영했다.
- `CF-FQ-033 차량 방어·손상 런타임`을 Active에서 Done으로 전환하고 VehicleDefense·HitDamage Current System을 완료 위치로 등록했다.
- 데미지 시스템 완료 후 UI 재개 우선순위에 따라 `CF-FQ-032`를 Paused에서 Active로 전환했다.
- 현재 실행을 UI-P0-02 실제 Pause·Focus·Launcher·Projectile·Timer 사용자 PIE로 고정하고 PASS 뒤 UI-P0-03으로 진행하도록 했다.
- CF-FQ-029·026 Paused, CF-FQ-030·031·034·035 Ready와 CF-FQ-019 Deferred 상태를 유지했다.
- 코드·에셋·빌드·Automation은 재실행하지 않았고 기존 미커밋 변경을 보호했다.

### v1.49.0 - 2026-08-03

- 사용자 결정에 따라 작업 완료 최우선 기능을 `CF-FQ-033 차량 방어·손상 런타임`, 즉 데미지 시스템으로 재정렬했다.
- `CF-FQ-033`을 Ready에서 Active로, `CF-FQ-032`를 Active에서 Paused로 전환했다.
- 현재 실행 순서를 DR-P0-07 사용자 PIE, 실패 데미지 결함 우선 수정, 공식 Build·전체 Automation, 영향 PIE 재검증, Systems 승격과 Done으로 고정했다.
- DR-P0-07을 직접 차단하는 문제만 예외 선행으로 허용하고 해소 즉시 데미지 시스템으로 복귀하도록 했다.
- 런처·미사일·TargetSelect·피팅·인벤토리의 기존 상태와 검증 결과는 변경하지 않았다.

### v1.48.0 - 2026-08-02

- `CF-FQ-030` 상태를 MG-P0-00 비활성 Foundation에서 `MG-P0-01~04 Direct Runtime·Test Assets Applied / Ready for Manual PIE`로 동기화했다.
- Launcher Ripple·Salvo의 CommandTargetLocation과 GuidanceTargetActor를 첫 발사 순간 함께 Snapshot해 후속 선택 변경과 같은 Volley의 목표를 분리했다.
- 공식 Editor Build `ce88150dfbbc495d91d1e2b8cd7c5225` / Exit Code 0과 전체 CarFight Automation `85c1e7285a7946fa8f55182b6247f0f4` / 43/43·필수 21/21 Success를 기록했다.
- Direct Missile·기존 Rocket·TargetSelect·Projectile Pool·Defense·Fitting·UI Pause 회귀가 같은 실행에서 통과했다.
- `CF-FQ-030`은 Ready, `CF-FQ-032`는 Active, `CF-FQ-029·026`은 Paused를 유지했으며 Unreal Asset과 Systems 승격은 수행하지 않았다.

### v1.47.0 - 2026-08-02

- `CF-FQ-034`를 FIT-P0-05 Initial Sortie Adapter Code Complete·Build·Automation PASS / Physics PIE·Mobility Pending으로 동기화했다.
- `CF-FQ-035`를 INV-P0-00~04 Done으로 동기화하고 순수 Fitting Adapter 완료와 Field Fitting Coordinator 미구현 상태를 분리했다.
- 차량 Runtime Ready의 Core·Combat 분리와 Pawn·Controller 입력 Context 소유권을 교차 기능 통합 기준으로 기록했다.
- `CF-FQ-032` 단일 Active, `CF-FQ-029`·`CF-FQ-026` Paused 상태를 유지했다.
- UI-P0-03, Field Fitting Runtime, Ammo와 Missile Runtime은 새로 착수하지 않았다.

### v1.46.0 - 2026-08-01

- `CF-FQ-034 FIT-P0-04 Sortie Fitting Apply Runtime`을 Done으로 전환했다.
- `UCFVehicleFittingComp`의 Legacy·Snapshot Prepare, Weapon·Defense Commit, 실패 Rollback과 `AppliedFittingSnapshot` 소유를 구현했다.
- 무효 Snapshot은 하위 Runtime Adapter를 호출하지 않고, 부분 Commit 실패는 직전 Applied 또는 VehicleData Legacy 입력으로 복원한다.
- 최종 공식 Editor Build `52b5b5572e2a4c739771dbd711a284d3`가 Exit Code 0으로 PASS했다.
- 전체 CarFight Automation은 40/40, 필수 회귀는 신규 FIT-P0-04 두 건을 포함해 19/19 Success였다.
- 다음 단계를 `FIT-P0-05 Runtime Mass Gate Ready`로 이동했다.
- `CF-FQ-032` 단일 Active와 Inventory `INV-P0-00~03 Done / INV-P0-04 Not Started`를 유지했다.
- Runtime Mass, Ammo, Inventory Adapter, Field Fitting, UI, Blueprint와 Unreal Asset은 변경하지 않았다.

### v1.45.0 - 2026-08-01

- `CF-FQ-035` 실제 저장소 상태를 `INV-P0-00~03 Done / INV-P0-04 Not Started`로 동기화했다.
- `INV-P0-01` Item Identity·Definition과 `INV-P0-02` VehicleCargo·MountedEquipment Container·Capacity·Access Query 완료 상태를 반영했다.
- `INV-P0-03` Reservation·Atomic Transfer는 이번 요청 이전에 존재한 구현으로 보호했으며 수정하거나 재구현하지 않았다.
- 공식 Editor Build `d46dd6fca34e44618ec51fa32f4250eb`가 Exit Code 0으로 PASS했다.
- 전체 CarFight Automation은 38/38 Success, 기존 필수 회귀는 17/17 Success였고 `INV-P0-02.Container`와 `INV-P0-02.AccessQuery`는 Warnings 0 / Errors 0이었다.
- `CF-FQ-032` 단일 Active 체크포인트를 유지했으며 C++, Blueprint와 Unreal Asset은 수정하지 않았다.

### v1.44.0 - 2026-08-01

- `CF-FQ-035 인벤토리 Foundation`을 P1 Ready 기능으로 등록했다.
- Inventory P0를 Item Instance, VehicleCargo·Mounted 소유 상태, 접근 조회, Reservation과 Atomic Transfer로 제한했다.
- `CF-FQ-034`에 Field Fitting 하위 트랙을 추가하고 비전투·쿨타임 종료·차량 정지·예약 성공 Gate를 확정했다.
- Equip은 호환되는 빈 슬롯에만 허용하고 교환은 Unequip과 Equip 두 시간 액션으로 분리했다.
- 이동·전투·피해·상태 변경·예약 손실·화면 종료 취소 시 Inventory와 Applied Snapshot을 원상 유지하도록 계약을 잠갔다.
- FIT-P0-03의 후속 공식 Editor Build와 전체 Automation 32/32 PASS를 반영해 Done으로 전환했다.
- Source, Blueprint와 Unreal Asset은 수정하지 않았다.

### v1.43.0 - 2026-08-01

- `CF-FQ-032 UI-P0-02` 실제 싱글플레이 World Pause, 입력 잔류 제거와 C++ Pause Menu 수명을 Code Complete로 기록했다.
- 최종 공식 Editor Build `4f2daa07187a41aea7c400386010b2a0`와 전체 Automation `6f0a9a5ebde5472b8b784a9103ce484d` 32/32 PASS를 기록했다.
- Launcher·Projectile·Propulsion과 `CF-FQ-034 FIT-P0-03` 보호 회귀 Success를 유지했다.
- 사용자 PIE는 Pending이며 `UI-P0-03`은 시작하지 않았다.
- `CF-FQ-032`은 Active로 유지하고 Done 또는 Systems Current로 승격하지 않았다.

### v1.42.0 - 2026-08-01

- 현재 단일 Active 기능을 사용자 지정에 따라 `CF-FQ-032`로 전환했다.
- `UI-P0-01A~01B` Code Complete, 공식 Editor Build와 전체 CarFight Automation 29/29 PASS를 기록했다.
- `CF-FQ-029`는 `LM-P0-06` 체크포인트를 보존한 Paused 상태로 전환했다.
- `CF-FQ-034 FIT-P0-03`는 이번 UI 작업에서 소스·문서·에셋 미변경 상태로 보존했다.
- 다음 단계는 `UI-P0-02 완전 Pause`이며 사용자 PIE와 Systems 승격은 아직 수행하지 않았다.

### v1.41.0 - 2026-07-31

- `CF-FQ-034 차량 피팅·질량 런타임`을 P1 Ready 기능으로 등록했다.
- `VehicleFittingPlan.md`, `VehicleFittingDesign.md`, `VehicleFittingRoadmap.md`와 갱신된 `CombatPlan/13_Fitting.md`를 기준 문서로 연결했다.
- 현재 VehicleData·HardpointSlots·MountProfiles·EquipmentPresetData·TurretMountData·WeaponData·Ammo 계획·VehicleDefenseData·질량 적용 상태를 조사했다.
- `FIT-P0-00`을 Done으로 기록하고 다음 단계를 `FIT-P0-01` 질량·데이터 소유권·Chaos 적용 정책 결정 잠금으로 지정했다.
- 피팅 P0를 별도 출격 데이터, 장착 호환 검증, 방어·탄약 선택, 질량 Snapshot, 출격 초기 적용과 가속·제동·선회 변화 검증으로 제한했다.
- 연료·배터리·Utility·경제·영구 저장·전투 중 교환과 실시간 탄약 질량 갱신은 P0에서 제외했다.
- 직접 AssetDump 재조회는 WinError 87로 실패했으므로 신규 에셋 검증 PASS를 기록하지 않았다.
- Source·Blueprint·Unreal Asset은 수정하지 않았고 현재 단일 Active `CF-FQ-029`와 기존 기능 상태를 유지했다.

### v1.40.0 - 2026-07-31

- CF-FQ-033 DR-P0-07 보호형 사용자 PIE 절차를 `VehicleDefenseDamageDesign.md v0.9.0`에 준비했다.
- 원본 TestMap·Pawn·Launcher·Projectile·Pool·추진·요격 에셋 보호, 복제 맵 배치와 VehicleDefenseComp 관찰 기준을 추가했다.
- BaseDamage 25·AP 0의 방어층·방향·재생 예상값과 AP 50 비교용 PIE 전용 복제 체인을 고정했다.
- CF-FQ-033은 Ready / DR-P0-07 Procedure Ready·User Result Pending으로 유지했다.
- 사용자 PIE PASS, CF-FQ-033 Done과 VehicleDefense Systems Current 승격은 수행하지 않았다.

### v1.39.0 - 2026-07-31

- CF-FQ-033 DR-P0-06 공식 Editor Build를 단독 재검증했다.
- AssetDump `ADumpEntityQuery.cpp`의 이전 실패 호출부가 `MakeEntityQueryStringArray` 기반으로 정리된 현재 상태를 확인했다.
- 공식 Build Job `0697e0552a0f4911a93384c5d3a8a928`에서 `CarFight_ReEditor Win64 Development`가 Exit Code 0, `Result: Succeeded`로 통과했다.
- 기존 Defense Automation 8/8과 전체 Combat Automation 필수 15/15·전체 24/24 PASS 증거를 유지하고 반복 실행하지 않았다.
- CF-FQ-033은 Ready / DR-P0-00~06 Done으로 갱신하고 다음 단계를 DR-P0-07 사용자 PIE로 이동했다.
- 현재 단일 Active CF-FQ-029를 유지하고 DR-P0-07·전체 Done·Systems Current 승격은 수행하지 않았다.

### v1.38.0 - 2026-07-31

- CF-FQ-033 DR-P0-06 공식 Editor Build와 전체 Automation을 재검증했다.
- Defense Automation Job `7b5dce50680f47a99a29b26b02a577d7`에서 CarFight.Damage 8/8 Success와 Exit Code 0을 확인했다.
- Combat Runtime Job `54340630094a461ea3f7a1056622fd3c`에서 필수 15/15·전체 24/24 Success, 실패 0과 Editor Exit Code 0을 확인했다.
- 이전 `WBP_TargetSelect.uasset` 파일 잠금과 `TS_P0_06.TargetHud` 실패가 해소됐음을 반영했다.
- 공식 Build Job `15d79dff7c6c4e8680bde3eb4052b27a`는 독립 AssetDump `ADumpEntityQuery.cpp` 컴파일 오류로 Exit Code 6을 재현했다.
- CF-FQ-033은 Ready / DR-P0-06 Automation PASS·Official Build Blocked로 유지하고 DR-P0-07·전체 Done·Systems Current는 수행하지 않았다.

### v1.37.0 - 2026-07-31

- CF-FQ-033 DR-P0-05 테스트 VehicleDefenseData와 별도 테스트 VehicleData 생성·연결을 완료했다.
- 원본 DA_TestSUV·DA_TestSedan 보호, Apply Report 보호 계약과 방어 자산 AssetDump 2/2 Success를 반영했다.
- `DA_DamageArmorPenTest` AP 50 생성, 기존 `DA_DamageAsset` AP 0·SHA 보호와 DamageData AssetDump 2/2 Success를 반영했다.
- Defense Automation 8/8 Success를 기록했다.
- DR-P0-06 공식 Build와 전체 회귀는 독립 AssetDump 컴파일 오류 및 WBP_TargetSelect 파일 잠금으로 차단되어 완료 처리하지 않았다.
- CF-FQ-033 Ready와 CF-FQ-029 단일 Active를 유지하고 DR-P0-07·전체 Done·Systems Current는 수행하지 않았다.

### v1.36.0 - 2026-07-31

- CF-FQ-033 DR-P0-04 Debug·Blueprint Contract 적용과 공식 Editor Build PASS를 기록했다.
- Combat Runtime 필수 15/15·전체 24/24 Success, DR-P0-04 DebugBlueprintContract와 CF-FQ-029 보호 회귀 PASS를 반영했다.
- CF-FQ-033은 Ready를 유지하고 다음 단계를 DR-P0-05 테스트 DataAsset 연결로 이동했다.
- DR-P0-05·DR-P0-07 완료, CF-FQ-033 전체 Done과 Systems Current 승격은 수행하지 않았다.

### v1.35.0 - 2026-07-31

- `CF-FQ-033 DR-P0-03`의 VehicleDefenseComp 기본 생성·초기화와 HitScan·Projectile 정식 방어 진입점 통합을 Done으로 기록했다.
- 기존 Vehicle Health Debug와 Projectile Pool용 Integrity 결과 호환, DefaultDefenseData None Legacy Fallback 유지를 기록했다.
- 공식 Editor Build Job `7c6c3f2e6d3647d4af4e63bf537933c5` / Exit Code 0을 기록했다.
- Combat Runtime Automation Job `95bb9b328ed2443baccf0fb59e3a0cab` / 필수 14개·전체 23개 Success / 실패 0을 기록했다.
- CF-FQ-029 Launcher·Launch Context·Pool·추진·동일 차량 격리·다른 차량 요격 보호 회귀 PASS와 관련 에셋 무변경을 기록했다.
- CF-FQ-033은 Ready를 유지하고 다음 단계를 `DR-P0-04 Debug와 Blueprint 이벤트`로 이동했다.
- CF-FQ-033 전체 Done, DR-P0-05·DR-P0-07 완료와 Systems Current 승격은 수행하지 않았다.

### v1.34.0 - 2026-07-31

```text
- CF-FQ-033 DR-P0-02 VehicleDefenseComp와 VehicleHealthComp 호환 확장을 구현 완료했다.
- 쉴드, 6방향 독립 장갑, 방향 판정, 관통·장갑 초과 피해, Integrity 분배와 쉴드 재생을 구현했다.
- 기존 Health API·프로퍼티·이벤트 이름을 유지하고 명시 Integrity 피해 API와 Legacy Fallback을 추가했다.
- 첫 빌드의 테스트 이름 충돌과 첫 Automation의 테스트 Fixture 비활성 문제를 보정했다.
- 최종 Editor Build Job 82ab34d50c62451fa74ca17ff00acad4 / Exit Code 0을 확인했다.
- CarFight.Damage 전체 6개 Automation을 실제 실행해 6 Success / 0 Failed / 0 Not Run을 확인했다.
- CF-FQ-029 Launcher·Projectile·Weapon·Pawn 보호 파일과 관련 에셋은 수정하지 않았다.
- CF-FQ-033을 Ready / DR-P0-03 Protected Integration Gate로 이동하고 현재 단일 Active CF-FQ-029를 유지했다.
```

### v1.33.0 - 2026-07-31

```text
- CF-FQ-033 DR-P0-01 공용 타입·VehicleDefenseData Foundation을 구현 완료했다.
- CFDamageRuntimeTypes, VehicleDefenseData, VehicleDefenseDataContract Automation과 VehicleData.DefaultDefenseData 선택 참조를 추가했다.
- 기존 VehicleData는 DefaultDefenseData=None으로 현재 BaseDamage 직접 Health Fallback을 유지한다.
- 공식 Editor Build Job bdc3e14d513549379d7f0768ad1ed7bd / Exit Code 0을 확인했다.
- CarFight.Damage.DR_P0_01.DataContract 실제 실행 결과 1 Success / 0 Failed / 0 Not Run을 확인했다.
- CF-FQ-029 Launcher·Projectile·Weapon·Pawn 보호 파일과 에셋은 수정하지 않았다.
- Legacy 피해 필드 숨김은 해당 필드가 CF-FQ-029 보호 파일에 있으므로 직렬화 호환 패치로 이관했다.
- CF-FQ-033은 Ready / DR-P0-02 Ready이며 현재 단일 Active는 계속 CF-FQ-029다.
```

### v1.32.0 - 2026-07-30

```text
- CF-FQ-033 차량 방어·손상 런타임을 P0 Ready 기능으로 등록했다.
- VehicleDefenseDamageDesign.md v0.1.0에서 DR-P0-00 데이터 계약, 쉴드→6방향 장갑→차량 내구도 피해 분배 공식과 Legacy Fallback을 확정했다.
- 기존 UCFVehicleHealthComp를 Vehicle Integrity와 파괴 상태 소유자로 유지하고 신규 VehicleDefenseComp가 앞단 방어층을 처리하는 구조를 확정했다.
- CF-FQ-029 Launcher·Missile dirty 변경 보호 범위와 DR-P0-01~03 단계별 통합 게이트를 고정했다.
- 전용 Automation 18개와 사용자 PIE 검증 계획을 확정했으며 소스·에셋은 수정하지 않았다.
- 현재 단일 Active CF-FQ-029와 CF-FQ-030~032 Ready, CF-FQ-019 Deferred, CF-FQ-026 Paused 상태는 변경하지 않았다.
```

### v1.31.0 - 2026-07-30

```text
- CF-TC-023 투사체 비행 FX 사용자 PIE 전체 행렬 PASS를 기록했다.
- Trail-only, Thruster-only, Trail+Thruster, 소켓·Fallback, 종료 Reset, Pool 20발 이상과 30 FPS 고속 Bounds를 완료했다.
- CF-FQ-027 상태를 Paused에서 Done으로 변경하고 Projectile Current System v1.5.0에 연결했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run 상태와 Runner 미노출 사실을 유지했다.
- CF-FQ-029 Active, CF-FQ-030~032 Ready, CF-FQ-019 Deferred와 CF-FQ-026 Paused 상태는 변경하지 않았다.
```

### v1.29.0 - 2026-07-29

```text
- 사용자 확정 요구에 따라 CF-FQ-031 차량 탄약·재장전 런타임을 P1 Ready 기능으로 등록했다.
- 인게임 탄약 수치를 피팅 최대 적재 가능량이 아니라 현재 출전 차량의 자유 사용 가능 탄약량으로 분리했다.
- 무기별 장전량, 탄종별 차량 예비량, 런처 시퀀스 예약, 발사 Commit·Rollback과 재장전 계약을 확정했다.
- Launcher Sequence Active 중 재장전, 무기·무기 그룹·탄종·EquipmentPreset 교환과 신규 시퀀스 시작을 거부하며, 거부 요청이 기존 시퀀스를 취소하지 않도록 잠갔다.
- AmmoSystemPlan, Design, Roadmap, TaskSource와 첫 WorkOrder를 준비했으며 소스·에셋은 수정하지 않았다.
- 현재 단일 Active CF-FQ-029와 Ready CF-FQ-030의 상태를 유지했다.
```

### v1.28.0 - 2026-07-29

```text
- CF-FQ-030 Ready 상태 안에서 MG-P0-00 비활성 Foundation 타입·Config·Guidance 수학을 적용했다.
- UCFProjectileData v1.8.0은 두 Missile Config를 기본 비활성으로 보유하며 기존 런타임을 변경하지 않는다.
- Editor Build Job 8c4f952fa58f499b9145b95caa1ff926 / Exit Code 0과 Automation Source Compile PASS를 기록했다.
- CF-FQ-030은 Ready, CF-FQ-029는 Active를 유지하고 MG-P0-01은 LM-P0-06 뒤로 차단했다.
```

### v1.27.0 - 2026-07-29

```text
- CF-FQ-029의 LM-P0-05 Launcher Editor Assets 적용과 독립 AssetDump 검증을 완료했다.
- DA_RocketLauncher와 RocketLauncher Preset을 분리하고 기본 플레이 Pawn을 DA_TestSUV + RocketLauncher로 연결했다.
- LM-P0-06 CF-TC-025·026 사용자 PIE를 현재 Active 단계로 이동했다.
- CF-FQ-030은 LM-P0-06과 CF-FQ-029 완료 판정 선행의 Ready 상태를 유지했다.
```

### v1.24.0 - 2026-07-28

```text
- 사용자 결정에 따라 CF-FQ-019 주행/전투 반복 테스트를 Candidate에서 Deferred로 변경했다.
- CF-FQ-019는 취소하지 않고 CF-FQ-029와 CF-FQ-030 이후 런처·미사일을 포함한 통합 회귀 범위로 다시 설계하도록 기록했다.
- 현재 Active CF-FQ-029와 Ready CF-FQ-030의 상태와 순서는 유지했다.
- 기존 CF-FQ-019 과거 이력은 변경하지 않았다.
```

### v1.23.0 - 2026-07-28

```text
- CF-FQ-029 모듈형 런처 및 발사 인계를 P1 Active 기능으로 등록했다.
- CF-FQ-030 물리 제한형 미사일 비행·유도를 P1 Ready 기능으로 등록하고 CF-FQ-029 Launch Handoff를 선행 조건으로 고정했다.
- LauncherMissilePlan, LauncherMissileDesign, LauncherMissileRoadmap, ModularLauncherPlan과 MissileGuidancePlan을 공식 준비 문서로 연결했다.
- Ejection 기본 이동을 ProjectileMovement 기반으로 결정하고 Direct·Angled·Vertical 사출을 단계적으로 구현하도록 정리했다.
- 미사일은 발사 후 런처와 독립하며 유도는 명중을 보장하지 않고 실제 충돌에서만 Damage를 처리하도록 강제 계약을 기록했다.
- 첫 코드 작업을 LM-P0-01 Projectile Launch Handoff Foundation으로 제한하고 현재 Source Not Modified / Code Entry Ready 상태를 등록했다.
- CF-FQ-019, CF-FQ-027과 CF-FQ-026의 기존 상태는 유지했다.
```

### v1.22.0 - 2026-07-28

```text
- CF-FQ-028 발사체 추진 시스템을 Active에서 Done으로 전환했다.
- 사용자 PIE PASS와 CF-TC-024 PASS를 등록했다.
- Document/Systems/Combat/Projectile.md v1.4.0을 현재 구현 기준으로 승격했다.
- ProjectilePropulsionPlan.md v1.2.0을 완료 이력으로 전환했다.
- 현재 Active 기능을 비웠다.
- CF-FQ-019를 다음 Candidate / Not Started로 유지하고 자동 착수하지 않았다.
- CF-FQ-027은 런타임 기반이 Current System에 반영됐지만 별도 전체 체크리스트와 CF-TC-023 미완료로 Paused 상태를 유지했다.
- 이번 승격 작업에서 반복 전투 회귀를 수행하거나 완료 처리하지 않았다.
```

### v1.21.0 - 2026-07-27

```text
- 사용자가 승인한 로켓·미사일용 실제 자체 추진 기능을 CF-FQ-028 발사체 추진 시스템으로 추가하고 P1 Active로 전환했다.
- CF-FQ-027은 Trail·Thruster C++ Foundation과 공식 빌드를 유지한 채 Editor 자산 연결 단계의 Paused 상태로 변경했다.
- CFProjectileMotorTypes, UCFProjectileMotorComp, ProjectileData PropulsionConfig와 ProjectileActor Motor·Thruster 상태 연결을 현재 체크포인트로 등록했다.
- InitialSpeed 분리, IgnitionDelay, Burning 고정 방향 가속, MaximumPropelledSpeed와 BurnedOut 관성·중력 비행 계약을 확정했다.
- 첫 빌드 Automation 타입 오류 수정과 최종 형식 정리 후 Build Job 2ffd09357e654bb7970a2e379f5ab45f Exit Code 0을 공식 증거로 기록했다.
- 계획 테스트 ID CF-TC-024를 예약하고 사용자 PIE 전에는 Done 또는 Systems Current로 승격하지 않도록 했다.
- CF-FQ-019를 CF-FQ-028 사용자 PIE 후속 반복 전투 Candidate로 이동했다.
```

### v1.20.0 - 2026-07-27

```text
- CarFight 코드 작업 기본 방침을 Browser AI Direct 구현으로 변경했다.
- CF-FQ-027의 코드 게이트를 Ready — Direct Implementation으로 전환했다.
- 최종 Codex YAML과 plan.* 품질 게이트를 실제 소스 착수 조건에서 제거했다.
- PFX-P0-01 ProjectileData 계약을 현재 AI 세션의 다음 직접 구현 단계로 확정했다.
- TaskSource와 WorkOrder는 구현 범위와 보호 기준을 제공하는 참고 자료로 유지했다.
```

### v1.19.0 - 2026-07-27

```text
- 사용자 선택에 따라 CF-FQ-027 투사체 비행 FX를 P1 Active 기능으로 등록했다.
- 완료된 CF-FQ-024 일회성 CombatFx와 지속형 Projectile Flight FX 책임을 분리했다.
- PFX-P0-00 대표 Plan, 로드맵, TaskSource와 사람이 검토 가능한 WorkOrder 초안 준비 상태를 기록했다.
- ACFProjectileActor 소유, FX_Trail·FX_Exhaust 소켓 우선, ProjectileData Fallback과 Pool 반환 전 Reset 계약을 등록했다.
- 계획 테스트 ID CF-TC-023을 예약하되 기능 완료 전 TestChecklist Current 회귀 항목에는 추가하지 않았다.
- plan.* 품질·증거 게이트가 현재 Admin 표면에 없어 최종 Codex YAML Missing과 Source Not Modified 상태를 분리 기록했다.
- CF-FQ-019를 CF-FQ-027 완료 후 후속 Candidate로 유지하고 CF-FQ-026 TS-P0-08 Paused 상태를 보존했다.
```

### v1.18.0 - 2026-07-27

```text
- 최종 사용자 PIE 전체 PASS를 반영해 CF-FQ-024를 Active에서 Done으로 전환했다.
- CF-TC-021 PASS와 Document/Systems/Combat/CombatFx.md Current System 승격을 등록했다.
- NS_BasicHit Impact 현재 크기 승인과 차량별 SM_Body.FX_Destroyed 위치 완료를 기록했다.
- 현재 Active 기능을 비우고 CF-FQ-019를 착수 가능한 다음 Candidate로 재정렬했다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지했다.
```

### v1.17.0 - 2026-07-24

```text
- 사용자 결정에 따라 CF-FQ-024 전투 FX를 Ready에서 Active로 전환했다.
- CF-FQ-026 타겟 선택 시스템은 TS-P0-08 체크포인트를 보존한 Paused 상태로 변경했다.
- 상태 표기에 Paused를 추가하고 기능 큐와 현재 최우선 순서를 갱신했다.
- 현재 FX 단계를 FAB 콘텐츠 조사와 Muzzle·Impact·Destroyed 후보 선별 Phase 0으로 기록했다.
- TargetSelect TS-P0-00~07 완료 상태와 Automation 7/7 PASS는 회귀 보호 기준으로 유지했다.
```

### v1.16.0 - 2026-07-24

```text
- 사용자 결정에 따라 CarFight 프로젝트 전역에서 게임 사운드를 지원하지 않는 정책을 확정했다.
- CF-FQ-024를 전투 FX 및 사운드 구현에서 전투 FX 전용 구현으로 변경했다.
- 대표 Plan을 Document/Plan/CombatFxAudio/ImplementationDesign.md로 유지하되 시각 FX 전용으로 해석했다.
- CF-FQ-019와 CF-FQ-020의 Audio·사운드 완료 조건을 시각 FX와 UI 피드백 기준으로 정리했다.
- SoundWave, SoundCue, MetaSound, Sound Attenuation, USoundBase, UAudioComponent와 오디오 모듈 도입을 프로젝트 범위에서 제외했다.
```

### v1.15.0 - 2026-07-23

```text
- CF-FQ-026 타겟 선택 시스템을 P1 Active 기능으로 등록했다.
- TS-P0-01 C++ 계약과 Editor 빌드 PASS, Blueprint·PIE Pending 상태를 현재 구현 체크포인트로 기록했다.
- CF-FQ-024 전투 FX 및 사운드 구현은 취소하지 않고 Ready로 전환했다.
- 현재 최우선 순서를 CF-FQ-026 → CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021로 변경했다.
- TargetSelect 기획·Plan·로드맵·작업지시 규격과 P0 제외 범위를 FeatureQueue에 연결했다.
```

### v1.14.0 - 2026-07-21

```text
- CF-FQ-025를 CurrentMuzzleDirection 기반 탄종 독립 터렛 레티클 기능으로 정정했다.
- 공식 에디터 빌드와 사용자 PIE PASS를 근거로 CF-FQ-025를 Done으로 전환했다.
- CF-FQ-024를 현재 구현순위 1위로 복원했다.
- 투사체 착탄 위치 3D 표시는 별도 후속 Feature 논의 대상으로 분리했다.
```

### v1.13.0 - 2026-07-16

```text
- CF-FQ-025 이중 레티클 및 사격방향 시각화를 P0 Active 기능으로 공식 등록했다.
- 현재 구현순위를 CF-FQ-025 → CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021 순서로 변경했다.
- CF-FQ-024는 취소하지 않고 구현순위 2위 Active 작업으로 유지했다.
- ReticleAimDirection 대표 Plan, 첫 구현 범위, 제외 범위와 사용자 PIE 완료 기준을 등록했다.
- 실제 사격 방향이 명확해진 뒤 전투 FX와 사운드를 연결하도록 개발 순서를 조정했다.
```

### v1.12.0 - 2026-07-15

```text
- CF-FQ-024 전투 FX 및 사운드 구현을 P0 Active 기능으로 추가했다.
- 완료된 CF-FQ-017의 범위를 Reticle / FireFeedback UI로 명확히 분리했다.
- 현재 우선순위를 CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021 순서로 갱신했다.
- CombatFxAudio 대표 Plan과 P0 완료 기준을 등록했다.
- CF-FQ-022의 오래된 PIE Pending 문구를 현재 Done / PASS 상태로 정정했다.
```

### v1.11.0 - 2026-07-15

```text
- CF-FQ-017 NoWeapon, AimBlocked와 정상 발사 회귀 사용자 PIE PASS를 기록했다.
- CF-FQ-017 상태를 Active에서 Done으로 변경했다.
- CF-TC-014 발사 피드백/UI PASS와 관련 Systems 현재 상태를 반영했다.
- 다음 후보 CF-FQ-019는 Candidate로 유지하고 자동 착수하지 않았다.
```

### v1.10.0 - 2026-07-15

```text
- CF-FQ-018 최소 Damage Runtime 사용자 PIE PASS를 기록했다.
- CF-FQ-018 상태를 Active에서 Done으로 변경했다.
- 현재 구현 기준을 Document/Systems/Combat/HitDamage.md로 승격했다.
- 최우선 착수 후보를 CF-FQ-017 최종 상태 검증과 CF-FQ-019 반복 회귀 순서로 갱신했다.
```

### v1.9.6 - 2026-07-14

```text
- CF-FQ-022 사용자 PIE에서 정렬 중 발사 정책 true/false, 정렬 완료 탄착과 MuzzleBlocked를 PASS 처리했다.
- CF-FQ-022 상태를 Active에서 Done으로 변경했다.
- CF-FQ-018 Damage Runtime의 사전 조건이 모두 완료된 것으로 정리했다.
- 주행·거리별 정량 조준 검증은 CF-FQ-019 확장 회귀로 이관했다.
```

### v1.9.5 - 2026-07-14

```text
- 사용자 PIE 축약 스트레스 테스트 통과를 기록했다.
- 30 FPS + 기준 속도 4배 차량 집중 발사, 얇은 벽 첫 Blocking Hit, 중복 Impact와 Pool 재사용 검증을 PASS 처리했다.
- CF-FQ-023 상태를 Active에서 Done으로 변경했다.
- 전체 FPS·속도 매트릭스와 이동 차량 검증은 CF-FQ-019 확장 회귀로 이관했다.
- CF-FQ-018의 남은 직접 선행 조건을 CF-FQ-022 사용자 PIE 완료로 정리했다.
```

### v1.9.4 - 2026-07-14

```text
- 사용자 PIE에서 피격 Actor BP_CFVehiclePawn_C_1과 피격 컴포넌트 SM_Body 기대값 출력을 확인했다.
- CF-FQ-023의 일반 속도 Visual HitContext 경로를 PIE PASS로 기록했다.
- CF-FQ-023은 Active 상태를 유지하고 고속·저프레임·얇은 충돌체 검증만 Pending으로 남겼다.
- 고속 PIE 전체 확인 전에는 Done 또는 전체 PASS로 판정하지 않는 기준을 유지했다.
```

### v1.9.3 - 2026-07-14

```text
- 사용자 실행 Tools\\BuildEditor.bat에서 피격 컴포넌트 독립 표시 보강분 공식 Editor 빌드 성공을 기록했다.
- CF-FQ-023은 Active 상태를 유지하고 피격 컴포넌트 SM_Body 확인과 고속 PIE만 Pending으로 남겼다.
- 사용자 PIE 전체 확인 전에는 Done 또는 PASS로 판정하지 않는 기준을 유지했다.
```

### v1.9.2 - 2026-07-14

```text
- 사용자 PIE에서 일반 속도 Projectile 피격과 피격 Actor BP_CFVehiclePawn_C_1 기록을 확인했다.
- HitComponentName 데이터 경로는 존재하지만 Debug Panel에 독립 표시가 없던 상태를 확인했다.
- VehicleDebug Panel의 Damage HitContext에 피격 컴포넌트 독립 표시 행을 추가했다.
- 표시 보강분 공식 빌드, SM_Body 재확인과 고속 PIE는 Pending으로 유지했다.
```

### v1.9.1 - 2026-07-14

```text
- 사용자 실행 Tools\\BuildEditor.bat에서 CF-FQ-023 공식 Editor 빌드 성공을 기록했다.
- CF-FQ-023은 Active 상태를 유지하고 사용자 PIE 고속 검증만 Pending으로 남겼다.
- 사용자 PIE 확인 전에는 Done 또는 PASS로 판정하지 않는 기준을 유지했다.
```

### v1.9.0 - 2026-07-14

```text
- CF-FQ-023 상태를 Ready에서 Active로 변경했다.
- UCFProjectileData 연속 충돌 설정과 ACFProjectileActor 보조 Sphere Sweep C++ 구현 완료 범위를 기록했다.
- Sweep=true, Sub-step=true, MaxStep=0.008333, Iterations=8, SupplementalSweep=true, CCD=false 안전 기본값을 기록했다.
- OnComponentHit과 보조 Sweep의 ResolveProjectileImpact 통합, 활성화별 중복 방지, Pool 상태 초기화를 기록했다.
- 기존 Projectile 채널, 시각 차체 충돌, HitComponentName, DamageHitContext 전달 경로 유지 상태를 기록했다.
- Tools\\BuildEditor.bat 공식 빌드와 사용자 PIE 검증은 Pending으로 유지했다.
- CF-FQ-022 정렬 중 발사 정책의 코드·빌드 완료와 PIE Pending 상태가 충돌하지 않도록 문장을 정정했다.
```

### v1.8.1 - 2026-07-14

```text
- CF-FQ-022 Align Fire Policy C++ 구현과 `BuildEditor.bat` 성공을 기록했다.
- CF-FQ-022는 사용자 PIE 검증 전이므로 Active / PIE Pending을 유지했다.
- WBP와 DataAsset 인스턴스는 이번 작업에서 변경하지 않았다.
```

### v1.8.0 - 2026-07-14

```text
- CF-FQ-022에 `UCFTurretMountData.bAllowFireWhileAligning` 터렛별 정책을 추가했다.
- 기본값 true, 정렬 중 허용 시 현재 Muzzle 방향 발사, 비허용 시 정렬 완료 전 거부 기준을 기록했다.
- MuzzleBlocked는 항상 거부하고 TurretAligning Reticle은 발사 허용 여부와 분리해 유지하도록 기록했다.
- 설계와 Codex 작업지시서 작성은 완료됐지만 코드/빌드/PIE는 Pending으로 유지했다.
```

### v1.7.1 - 2026-07-13

```text
- Reticle Recovery Hotfix 코드 완료와 BuildEditor.bat 성공 상태 기록
- 에디터 종료 후 UnrealEditor-CarFight_Re.dll 링크가 성공했음을 기록
- CF-FQ-022는 Active / PIE Pending 유지
```

### v1.7.0 - 2026-07-13

```text
- CF-FQ-022 상태를 Ready에서 Active로 변경
- AimFireAlignment Core/Presentation C++ 빌드 완료 상태와 PIE Pending 상태를 분리 기록
- TurretAligning / WeaponNotAligned / MuzzleBlocked 표시 연결이 완료됐지만 PIE PASS는 아직 기록하지 않음
- 실제 피해 처리와 고속 Projectile 작업은 CF-FQ-022 PIE 검증 이후 진행하도록 우선순위 설명 갱신
```

### v1.6.0 - 2026-07-13

```text
- CF-FQ-018을 Ready에서 Active로 변경하고 시각 차체 기반 피격 구현/PIE 확인 결과 반영
- CF-FQ-022 조준점·터렛·총구 정렬 P0 Ready 기능 추가
- CF-FQ-023 고속 Projectile 연속 충돌 P0 Ready 기능 추가
- Reticle 목표와 실제 Muzzle 발사 방향 불일치, 고속 Projectile 터널링을 Damage Runtime 선행 문제로 등록
- Document/Plan/AimFireAlignment/ImplementationDesign.md와 Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md를 기준 문서로 연결
- 실제 피해 처리보다 두 사격 신뢰성 작업을 먼저 수행하도록 우선순위 재정렬
```

### v1.5.0 - 2026-07-13

```text
- CF-FQ-018을 Candidate에서 Ready로 변경
- 현재 차량 무기 피격이 VehicleMesh Physics Asset 기준이고 SM_Body는 NoCollision이라는 확인 결과 반영
- 실제 피해 처리 전에 VehicleMesh 물리 충돌과 SM_Body 무기 피격 Query를 분리한다는 선행 조건 추가
- Document/Plan/HitDamage/ImplementationDesign.md를 CF-FQ-018 현재 설계 기준으로 등록
- Collision Channel/Profile 이름과 차량별 StaticMesh Collision 상태를 코드 작업 전 미확정 항목으로 분리
```

### v1.4.0 - 2026-07-13

```text
- CF-FQ-017 발사 피드백 구현 상태를 Ready에서 Active로 변경
- Reticle 이미지/텍스트 상태별 색상과 FireSuccess → Cooldown → 종료 흐름의 구현 및 PIE 확인 결과 반영
- 전용 OutOfArc 경고의 중복 방지, 종료 조건, 다른 FireFeedback 비가림 조건 확인 결과 반영
- NoWeapon / AimBlocked 실제 PIE 검증을 Done 전 남은 조건으로 명시
- 다음 우선순위를 CF-FQ-017 최종 검증 후 CF-FQ-018 착수 순서로 갱신
```

### v1.3.0 - 2026-07-09

```text
- CF-FQ-016 차량 무기 조준 및 발사를 Done으로 조정
- CF-FQ-017 발사 피드백 구현을 Ready로 조정
- 현재 최우선 착수 후보를 FireFeedback / Reticle 구현 기준으로 재정렬
- FireFeedback.md 신규 Systems 문서 생성 상태를 완료 후 Systems 위치와 착수 기준에 반영
- CF-FQ-002 조준/발사 피드백 분리의 현재 대체 기준을 CF-FQ-017 로컬 FireFeedback으로 갱신
- Reticle / FireFeedback 착수 기준 섹션을 추가
- OutOfArc / FirePending / FireRejected / Cooldown / NoWeapon 관련 현재 기준을 기능 큐에 반영
```

### v1.2.0 - 2026-06-19

```text
- CF-FQ-016 ~ CF-FQ-021 싱글 로컬 전투 루프 개발 후보를 추가
- 현재 최우선 착수 후보를 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증 순서로 재정렬
- CF-FQ-011 싱글 실행 기준선 전환과 CF-FQ-013 카메라/로컬 조준 고도화를 완료 기반으로 표시
- 체력/대미지 장기 확장 항목은 유지하되, 최소 피해 처리는 CF-FQ-018에서 먼저 다루도록 분리
```

### v1.1.0 - 2026-06-18

```text
- 기능 후보 큐의 P0를 서버 권한 발사 요청에서 싱글 실행 기준선 / 1대 차량 주행감 / 카메라·로컬 조준 고도화로 변경
- 서버 권한 발사, 리스폰, 세션, 서버 로그, 관리툴 후보를 Deferred 또는 Icebox로 조정
- CF-FQ-011 ~ CF-FQ-015 싱글 차량 고도화 후보 추가
- 관리툴 후보 큐를 현재 범위 밖 장기 보류 후보로 재해석
```

### v1.0.0 - 2026-06-02

```text
- FeatureQueue 문서 최초 작성
- 기능 후보 큐 작성
- Plan 승격 기준 정의
- Systems 승격 위치 개념 정의
- 관리툴 후보 큐 추가
```

---

## 12. Migration

### v1.41.0 적용 안내

- `CF-FQ-034` 기획·구현 세션은 `Document/Plan/VehicleFittingPlan.md`에서 시작한다.
- `FIT-P0-00` 조사와 Plan·Design·Roadmap 작성은 완료됐으므로 반복하지 않는다.
- 다음 단계 `FIT-P0-01`에서는 차체 기준 질량 SSOT, Weapon·Defense 질량 필드, 최대 총중량, Missing MountSelection, Chaos 질량 적용과 기동 보정 공식을 먼저 잠근다.
- VehicleData는 플랫폼 원본, VehicleFittingData는 출격 선택, Applied Fitting Snapshot은 검증된 결과로 분리한다.
- CF-FQ-031의 탄약 수량·재장전과 CF-FQ-033의 Shield·Armor·Integrity 소유권을 피팅으로 이동하지 않는다.
- FittingData 미지정 기존 차량은 현재 VehicleData 기본 EquipmentPreset과 Defense fallback을 유지한다.
- 직접 AssetDump 재조회 실패는 신규 에셋 검증 PASS가 아니며 소스·에셋 착수 전에 대상 VehicleData·EquipmentPresetData를 다시 확인한다.
- 현재 단일 Active는 계속 `CF-FQ-029`이며 사용자가 별도 구현 착수를 선택하기 전에는 CF-FQ-034를 Active로 전환하지 않는다.

### v1.40.0 적용 안내

- CF-FQ-033 재개 시 `VehicleDefenseDamageDesign.md v0.9.0`의 DR-PIE-00 시작 상태 게이트부터 진행한다.
- DR-P0-00~06 구현·Build·Automation을 반복하지 않는다.
- 사용자 결과는 DR-PIE-01~06별 PASS / FAIL / BLOCKED로 기록하며 전체 행렬 전에는 기능을 완료 처리하지 않는다.
- 현재 단일 Active CF-FQ-029와 관련 dirty 소스·에셋을 계속 보호한다.

### v1.39.0 적용 안내

- CF-FQ-033 DR-P0-00~06 구현·테스트 자산·Automation·공식 Editor Build는 완료 상태이므로 반복하지 않는다.
- 최신 공식 Build 기준은 Job `0697e0552a0f4911a93384c5d3a8a928`, `CarFight_ReEditor Win64 Development`, Exit Code 0이다.
- 다음 단계는 DR-P0-07 사용자 PIE이며 사용자 확인 전에는 PASS 또는 완료로 기록하지 않는다.
- DR-P0-07 완료 전에는 CF-FQ-033 전체 Done과 `Document/Systems/Combat/VehicleDefense.md` Current 승격을 수행하지 않는다.
- 현재 단일 Active CF-FQ-029와 Launcher·Pool·추진·요격 dirty 변경 및 관련 에셋을 계속 보호한다.

### v1.38.0 적용 안내

- CF-FQ-033 DR-P0-00~05 구현·테스트 자산과 DR-P0-06 Defense·Combat Automation은 반복하지 않는다.
- 전체 CarFight Automation 기준은 필수 15/15·전체 24/24 Success이며 `WBP_TargetSelect.uasset` 잠금 차단은 해소됐다.
- DR-P0-06의 남은 차단은 독립 AssetDump `ADumpEntityQuery.cpp` 컴파일 오류에 의한 공식 Editor Build 실패다.
- CarFight 작업에서는 AssetDump 소스를 임의 수정하지 않고 독립 저장소의 해소 결과를 확인한 뒤 공식 Build만 재검증한다.
- 공식 Build PASS 전에는 DR-P0-06을 Done으로 전환하지 않는다.
- DR-P0-07 사용자 PIE, CF-FQ-033 전체 Done과 Systems Current 승격은 아직 수행하지 않는다.

### v1.37.0 적용 안내

- CF-FQ-033 DR-P0-00~05 구현·테스트 자산 생성은 반복하지 않는다.
- 정식 방어 테스트 VehicleData는 `DA_VehicleDefense_TestSUV`, Legacy 회귀 VehicleData는 `DA_TestSedan`이다.
- 다음 단계는 DR-P0-06 재검증이며 AssetDump compile blocker와 WBP_TargetSelect 파일 잠금을 먼저 해소한다.
- 공식 Build PASS와 전체 CarFight 24/24 Success 전에는 DR-P0-06 완료로 전환하지 않는다.
- DR-P0-07 사용자 PIE, CF-FQ-033 전체 Done과 Systems Current 승격은 아직 수행하지 않는다.

### v1.36.0 적용 안내

- CF-FQ-033 DR-P0-00~04는 구현·공식 Build·Combat Runtime Automation 완료 상태이므로 반복하지 않는다.
- 다음 단계는 DR-P0-05 테스트 VehicleDefenseData 생성과 VehicleData.DefaultDefenseData 연결이다.
- 사용자 PIE는 DR-P0-07까지 Pending이며 CF-FQ-033 전체 Done과 Systems Current 승격은 아직 수행하지 않는다.
- 현재 단일 Active는 CF-FQ-029 / LM-P0-06이며 Launcher·Pool·추진·요격 dirty 변경과 관련 에셋을 계속 보호한다.

### v1.35.0 적용 안내

- CF-FQ-033 DR-P0-00~03은 적용·공식 Build·Combat Runtime Automation 완료 상태이므로 반복 구현하지 않는다.
- HitScan과 Projectile의 정식 피해 진입점은 `UCFVehicleDefenseComp::TryApplyDamageToActor`다.
- 기존 Health Debug와 Projectile Pool은 `BuildIntegrityCompatibilityResult`가 생성한 Integrity 결과를 계속 사용한다.
- DefaultDefenseData가 None인 기존 차량은 VehicleHealthComp Legacy Fallback으로 동작한다.
- CF-FQ-033은 아직 Ready이며 전체 Done 또는 Systems Current가 아니다.
- 다음 구현 단계는 `DR-P0-04 Debug와 Blueprint 이벤트`다.

### v1.34.0 적용 안내

```text
- CF-FQ-033 DR-P0-00~03 Runtime Foundation·정식 통합은 적용·공식 Build·Combat Runtime Automation 완료 상태이므로 반복 구현하지 않는다.
- VehicleHealthComp의 기존 Health 함수·프로퍼티·이벤트 이름은 Blueprint와 직렬화 호환을 위해 유지한다.
- 당시 v1.34.0 기준으로 VehicleDefenseComp는 ACFVehiclePawn 기본 컴포넌트와 HitScan·Projectile 실제 호출부에 연결되지 않았으며, 현재 상태는 v1.35.0 적용 안내를 따른다.
- 기존 플레이 경로는 DR-P0-03부터 VehicleDefenseComp 정식 진입점을 사용하고 DefenseData가 없으면 VehicleHealthComp Legacy Fallback을 유지한다.
- DR-P0-03은 CF-FQ-029 dirty 파일과 체크포인트를 재확인한 뒤 최소 호출부만 수정해 완료했으며, 다음 단계는 DR-P0-04다.
- Launcher, Launch Context, Projectile Pool, 추진, 유도, 요격, 충돌 Profile과 FX 수명을 변경하지 않는다.
- 방어 DataAsset 생성과 차량 연결은 DR-P0-05 전까지 수행하지 않는다.
- 현재 단일 Active 작업은 CF-FQ-029 LM-P0-06으로 유지한다.
```

### v1.33.0 적용 안내

```text
- CF-FQ-033 DR-P0-01 Foundation은 적용·빌드·자동 테스트 완료 상태이므로 반복 구현하지 않는다.
- 다음 단계는 DR-P0-02 VehicleDefenseComp와 VehicleHealthComp 호환 확장이다.
- DR-P0-02에서도 CFProjectileActor, CFProjectileData, CFProjectilePoolComp, CFVehiclePawn, CFVehicleWeaponComp, CFWeaponData와 Launcher 파일을 수정하지 않는다.
- 기존 VehicleData의 DefaultDefenseData는 None이며 신규 방어 에셋을 연결하기 전까지 기존 피해 결과를 유지한다.
- Legacy 피해 필드는 CF-FQ-029 보호 해제 또는 체크포인트 확보 후 에디터 비노출과 직렬화 호환을 함께 검증한다.
- 현재 단일 Active 작업은 CF-FQ-029 LM-P0-06으로 유지한다.
```

### v1.32.0 적용 안내

```text
- CF-FQ-033은 P0 Ready이며 현재 Active 또는 Current System이 아니다.
- 대표 설계 문서는 Document/Plan/VehicleDefenseDamageDesign.md v0.1.0이다.
- DR-P0-00 설계·계약은 완료됐으므로 구현 세션에서 피해 소유권과 공식을 임의로 다시 설계하지 않는다.
- 기존 UCFVehicleHealthComp는 Vehicle Integrity와 최초 파괴 상태 소유자로 유지한다.
- 기존 차량은 DefaultDefenseData=None Legacy Fallback으로 BaseDamage 직접 Health 동작을 유지한다.
- DR-P0-01~02는 CF-FQ-029 중첩 파일을 수정하지 않으며, HitScan·Projectile 통합은 DR-P0-03 보호 게이트 이후 수행한다.
- CF-FQ-003의 방어 본체 범위는 CF-FQ-033으로 이관됐으므로 별도 기능으로 착수하지 않는다.
- 현재 세션 복원 우선순위는 계속 CF-FQ-029 LM-P0-06 사용자 PIE다.
```

### v1.29.0 적용 안내

```text
- CF-FQ-031 구현 세션은 Document/Plan/AmmoSystemPlan.md를 대표 진입점으로 사용한다.
- 첫 코드 작업은 AMMO-P0-01 Ammo Data Contract Foundation이며 실제 차량 탄약 Runtime과 발사 소비를 함께 구현하지 않는다.
- 인게임 CurrentUsableAmmoCount는 현재 출전 차량에서 신규 행동으로 자유롭게 사용할 수 있는 장전·예비 탄약이며, 진행 중 시퀀스 예약량은 별도 표시한다.
- Launcher Sequence Active 중 재장전과 무기·탄종·장비 교환 요청은 거부하고 현재 시퀀스를 계속 진행한다.
- 문서 준비 완료만으로 CF-FQ-031을 Active, Done 또는 Current System으로 해석하지 않는다.
- 현재 세션 복원 우선순위는 계속 CF-FQ-029 LM-P0-06이다.
```

### v1.27.0 적용 안내

```text
- 새 세션은 LM-P0-01~05 구현·자산 적용을 반복하지 않고 LM-P0-06 CF-TC-025·026 사용자 PIE에서 시작한다.
- AssetDump PASS는 저장값과 참조 검증이며 실제 발사 동작 PASS를 대신하지 않는다.
- Angled·Vertical 임시 검증 뒤 DA_RocketLauncher를 Direct / EjectionSpeed 0 / CarrierVelocityRatio 0으로 복구한다.
- CF-FQ-029는 사용자 통합 PIE 전까지 Active를 유지하고 CF-FQ-030은 Ready를 유지한다.
```

### v1.24.0 적용 안내

```text
- CF-FQ-019는 Deferred이며 현재 최우선 또는 후속 Candidate로 복원하지 않는다.
- 기존 반복 전투 테스트 항목은 삭제하지 않고 런처·미사일 통합 회귀를 다시 설계할 때 입력으로 사용한다.
- 현재 Active는 CF-FQ-029이며 다음 코드 작업은 LM-P0-01이다.
- CF-FQ-030은 CF-FQ-029 Launch Handoff 선행의 Ready 상태를 유지한다.
```

### v1.23.0 적용 안내

```text
- 현재 Active 기능은 CF-FQ-029 모듈형 런처 및 발사 인계다.
- 대표 Plan은 Document/Plan/LauncherMissilePlan.md다.
- LM-P0-00 조사·설계·문서 준비는 완료됐으므로 반복하지 않는다.
- 다음 작업은 LM-P0-01 Projectile Launch Handoff Foundation 직접 구현이다.
- 기존 단일 Muzzle 직사 발사와 비유도 Rocket 추진을 회귀 보호한다.
- CF-FQ-030은 CF-FQ-029 Launch Handoff와 Direct Launcher 기반 검증 전에는 코드 착수하지 않는다.
- CF-FQ-019, CF-FQ-027과 CF-FQ-026의 기존 Candidate·Paused 상태를 유지한다.
```

### v1.22.0 적용 안내

```text
- CF-FQ-028은 Done / CF-TC-024 PASS이며 Active로 복원하지 않는다.
- 현재 구현 판단은 Document/Systems/Combat/Projectile.md를 우선한다.
- ProjectilePropulsionPlan.md는 완료 이력이다.
- 현재 Active 기능은 없다.
- CF-FQ-019는 다음 Candidate지만 사용자가 선택하기 전에는 착수하지 않는다.
- CF-FQ-027과 CF-FQ-026의 Paused 상태를 유지한다.
```

### v1.21.0 적용 안내

```text
- 새 세션은 CF-FQ-028과 Document/Plan/ProjectilePropulsionPlan.md를 현재 Active 기능으로 복원한다.
- PP-P0-00~07 C++ Foundation과 공식 Editor 빌드는 완료됐으므로 반복 구현하지 않는다.
- 다음 실행은 Editor 테스트 Rocket ProjectileData 생성, PropulsionConfig·FX_Exhaust 저장과 사용자 PIE다.
- 기존 ProjectileData는 bUsePropulsion=false 기본값으로 기존 포탄 이동을 유지한다.
- CF-FQ-027은 Editor 자산 연결 단계 Paused, CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
- CF-TC-024 PASS와 Projectile Systems 승격은 사용자 PIE 완료 후에만 기록한다.
```

### v1.20.0 적용 안내

```text
- 새 세션은 CF-FQ-027과 Document/Plan/ProjectileFlightFxPlan.md를 현재 Active 기능으로 복원한다.
- CodeWorkGate.md v2.0에 따라 현재 AI 세션이 PFX-P0-01부터 직접 구현한다.
- TaskSource와 WorkOrder는 구현 참고 자료이며 최종 Codex YAML 또는 plan.* 품질 게이트를 기다리지 않는다.
- 구현 후 Git diff, 공식 Editor 빌드, Automation과 사용자 PIE를 검수한다.
- CF-FQ-024는 Done, CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v1.19.0 적용 안내 — 폐기됨, v1.20.0이 대체

> 아래 내용은 당시 실행 기준 보존용이며 현재 코드 작업 판단에는 사용하지 않는다.

```text
- 새 세션은 CF-FQ-027과 Document/Plan/ProjectileFlightFxPlan.md를 현재 Active 기능으로 복원한다.
- PFX-P0-00 조사·설계를 반복하지 않고 TaskSource와 WorkOrder 상태부터 확인한다.
- 현재 최종 Codex YAML은 Missing이며 plan.* 품질·증거 게이트 전에는 실제 소스 변경을 시작하지 않는다.
- CF-FQ-024는 Done 상태를 유지하고 Muzzle·Impact·Destroyed 일회성 FX를 다시 구현하지 않는다.
- CF-FQ-019는 CF-FQ-027 완료 후 반복 전투 후속 Candidate다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v1.18.0 적용 안내

```text
- CF-FQ-024는 Done / User PIE PASS / CF-TC-021 PASS다.
- 현재 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- CombatFxAudio Plan은 완료 이력이며 Active 구현 문서로 사용하지 않는다.
- 현재 Active 기능은 없고 CF-FQ-019는 사용자가 선택할 때만 Active로 전환한다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v1.17.0 적용 안내

```text
- 새 세션은 CF-FQ-024와 Document/Plan/CombatFxAudio/ImplementationDesign.md를 우선 복원한다.
- 첫 실행은 AssetPreparationChecklist.md 기준 FAB Niagara 자산 조사다.
- CF-FQ-026은 TS-P0-08 Paused이며 FX 작업 중 TargetSelect 코드·에셋·검증 결과를 변경하지 않는다.
- CF-FQ-024 완료 전에는 CombatFx를 Current System으로 승격하지 않는다.
- 게임 사운드 자산, 런타임, 모듈과 테스트 완료 조건을 추가하지 않는다.
```

### v1.16.0 적용 안내

```text
- CF-FQ-024는 동일 ID를 유지하지만 의미는 전투 FX 전용 기능이다.
- 신규 구현은 `Document/Plan/CombatFxAudio/ImplementationDesign.md`를 사용한다.
- `CombatFxAudio` 디렉터리명은 레거시 경로이며 문서 내용과 구현 범위는 시각 FX 전용으로 해석한다.
- 게임 사운드 자산, 런타임, 모듈과 테스트 완료 조건을 새 기능에 추가하지 않는다.
- Unreal의 기본 플랫폼 오디오 설정은 엔진 생성 설정으로 남길 수 있으나 게임 기능 구현으로 해석하지 않는다.
```

### v1.14.0 적용 안내

```text
- CF-FQ-025는 Done / User PIE PASS로 해석한다.
- Weapon Reticle은 실제 최종 AimDirection이나 착탄 Preview가 아니라 CurrentMuzzleDirection 기반 터렛 레티클이다.
- 기존 DirectImpact/LaunchDirection 기록은 Legacy 구현 이력으로만 해석한다.
- 신규 구현 우선순위는 CF-FQ-024부터 재개한다.
```

### v1.13.0 적용 안내

```text
- CF-FQ-025를 현재 P0 Active 구현순위 1위로 해석한다.
- 신규 구현 세션은 Document/Plan/ReticleAimDirection/ImplementationDesign.md를 우선 복원한다.
- CF-FQ-024는 취소하지 않고 P0 Active 구현순위 2위로 유지한다.
- CF-FQ-025 Phase 0~4와 사용자 PIE 완료 또는 사용자 명시적 전환 후 CF-FQ-024를 재개한다.
- CF-FQ-025 구현 완료 전에는 Weapon Reticle과 Weapon Preview를 Current System으로 해석하지 않는다.
- 기존 AimDirection, AimTargetLocation, FireFeedback와 MuzzleBlocked 판정 계약은 변경하지 않는다.
```

### v1.12.0 적용 안내

```text
- CF-FQ-017은 Reticle / FireFeedback UI 완료 기능으로 유지한다.
- 실제 Niagara와 공간 사운드 구현은 CF-FQ-024에서 진행한다.
- CF-FQ-019 반복 테스트는 CF-FQ-024 사용자 PIE 완료 뒤 착수한다.
- CF-FQ-024 구현 완료 전에는 CombatFxAudio를 Current System으로 해석하지 않는다.
```
