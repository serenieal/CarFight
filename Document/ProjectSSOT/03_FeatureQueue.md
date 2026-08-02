# CarFight — 03_FeatureQueue

> 문서 버전: v1.48.0
> 작성일(Asia/Seoul): 2026-08-02
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
| `CF-FQ-015` | 차량 데이터 튜닝 패스 | 기준 차량 1대의 Movement/Wheel/DriveState 값을 추적 가능한 데이터 기준으로 정리 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleData.md` 갱신 |
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
| `CF-FQ-031` | 차량 탄약·재장전 런타임 | 현재 출전 차량의 실제 사용 가능 탄약을 무기별 장전량과 탄종별 예비량으로 관리하고, 발사 소비·런처 예약·행동 잠금·재장전·HUD·피팅 중량을 하나의 Runtime 계약으로 연결하기 | `P1` | `Ready` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Ammo.md`, `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/Combat/Launcher.md`, 관련 UI·Vehicle Systems 갱신 |
| `CF-FQ-032` | 인게임 전투 HUD 및 UI 프레임워크 | 인게임 UI를 LocalPlayer 소유 Root와 레이어로 관리하고 완전 Pause, 외부 3인칭 HUD, Radar, 차량 방어·무기 자원·타겟 지식 표시를 구현하면서 향후 전체 게임플로우 확장을 열어두기 | `P1` | `Active` | 필요 | 불필요 | 불필요 | `Document/Systems/UI/InGameHUD.md`, `Document/Systems/UI/UIFlow.md`, 관련 Combat·Targeting·Vehicle Systems 갱신 |
| `CF-FQ-033` | 차량 방어·손상 런타임 | 기존 최소 HitDamage 앞에 쉴드, 6방향 독립 장갑, 관통과 차량 내구도 피해 분배를 추가하고 기존 에셋·Health 이벤트 호환을 유지하는 P0 방어 본체 구축 | `P0` | `Ready` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/VehicleDefense.md`, `Document/Systems/Combat/HitDamage.md` 갱신 |
| `CF-FQ-034` | 차량 피팅·질량 런타임 | VehicleData의 하드포인트·MountProfile과 소유 장비·Ammo·VehicleDefenseData를 검증 Snapshot으로 조합하고, 출격 적용과 비전투·정지·쿨타임 완료 조건의 시간 소모 필드 장착·해제를 원자적으로 연결하기 | `P1` | `Ready` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleFitting.md`, `Document/Systems/Vehicles/VehicleData.md`, `Document/Systems/Vehicles/VehicleRuntime.md`, 관련 Combat·Inventory·UI Systems 갱신 |
| `CF-FQ-035` | 인벤토리 Foundation | 실제 소유 Item Instance, VehicleCargo·Mounted 소유 상태, 접근 조회, Reservation과 Atomic Transfer를 제공해 필드 피팅과 향후 탄약·루팅·보상·저장의 공용 소유권 기반 만들기 | `P1` | `Ready` | 필요 | 불필요 | 불필요 | `Document/Systems/Inventory/InventoryFoundation.md`, 관련 Vehicle·Combat·UI Systems 갱신 |
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
| `CF-FQ-008` | 무장 데이터 정의 | 차량 장착형 터렛/무기 데이터 기준 만들기 | `P2` | `Candidate` | 필요 | 불필요 | 낮음 | `Document/Systems/Data/WeaponData.md` |
| `CF-FQ-009` | 운영 로그 조회 기준 | 서버 전투/스폰/에러 로그를 추적 가능한 형태로 정리 | `Icebox` | `Deferred` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/LogView.md` |
| `CF-FQ-010` | 세션/로비 기초 | Dedicated Server 이후 접속 흐름 확장 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Network/Session.md` |

---

## 6. 현재 최우선 착수 후보

현재 단일 Active 기능은 사용자가 명시적으로 전환한 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다.

```text
1. CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크: Active / UI-P0-01A~02 Code Complete·Build·Automation PASS / User PIE Pending / UI-P0-03 Not Started
2. CF-FQ-029 모듈형 런처 및 발사 인계: Paused / LM-P0-01~05 완료 유지 / LM-P0-06 사용자 PIE 체크포인트 보존
3. CF-FQ-030 물리 제한형 미사일 비행·유도: Ready for Manual PIE / MG-P0-01~04 Direct Runtime·Test Assets Applied / Build·Automation PASS / Manual PIE Pending
4. CF-FQ-031 차량 탄약·재장전 런타임: Ready / AMMO-P0-00 Documentation Done / AMMO-P0-01 Code Entry Ready
5. CF-FQ-033 차량 방어·손상 런타임: Ready / DR-P0-00~06 Done / DR-P0-07 User Result Pending
6. CF-FQ-034 차량 피팅·질량 런타임: Ready / FIT-P0-05 Initial Sortie Adapter Code Complete·Build·Automation PASS / Physics PIE·Mobility Pending / FFIT-P0-00 Field Action Contract Done
7. CF-FQ-035 인벤토리 Foundation: Ready / INV-P0-00~04 Done / Adapter Code·Automation·Build PASS / Field Fitting Coordinator Not Started
8. CF-FQ-028 발사체 추진 시스템: Done / User PIE PASS / CF-TC-024 PASS / Systems Current
9. CF-FQ-027 투사체 비행 FX: Done / User PIE PASS / CF-TC-023 PASS / Systems Current
10. CF-FQ-026 타겟 선택 시스템: Paused / TS-P0-08 재개 가능
11. CF-FQ-020 조작감/전투 템포/피드백 개선: Candidate
12. CF-FQ-021 핵심 게임 루프 검증: Candidate
13. CF-FQ-019 주행/전투 반복 테스트: Deferred / 런처·미사일 이후 통합 회귀로 재설계
```

`CF-FQ-029`는 Launch Handoff, 가변 Muzzle·SingleCycle, Pattern Data, Ripple·Salvo Scheduler와 Direct·Angled·Vertical Release의 C++ 적용·공식 Editor 빌드, `LM-P0-05 Launcher Editor Assets` 적용과 독립 AssetDump 검증을 완료했다. 현재 단계는 `LM-P0-06 Launcher Integration PIE`이며 `CF-TC-025·026`은 TODO다. `CF-FQ-030`은 Ready 상태에서 `MG-P0-01~04` Direct Release·단일 Muzzle Runtime과 격리 테스트 자산을 적용했고, TargetActor 제한형 유도·목표 소실·오버슈트·Pool Reset을 자동 검증했다. Launcher Ripple·Salvo는 첫 발사 순간 Command Target 위치와 약한 Guidance Target Actor를 함께 Snapshot하므로 이후 TargetSelect 변경이 같은 Volley의 후속 미사일 목표를 바꾸지 않는다. 정지·측면 이동·선택 변경·목표 파괴·오버슈트·Pool 재사용 사용자 PIE 전에는 Done 또는 Systems Current로 승격하지 않는다. `CF-FQ-031`은 현재 출격 차량의 사용 가능 탄약, 무기별 장전량, 탄종별 차량 예비량, 런처 시퀀스 예약과 재장전·교환 행동 잠금을 다루는 문서 준비 완료 Ready 기능이며, 사용자가 실제 착수를 선택하기 전에는 Source Not Modified 상태를 유지한다. `CF-FQ-033`은 `VehicleDefenseDamageDesign.md v0.9.0`에서 `DR-P0-00~06`을 완료하고 `DR-P0-07` 보호형 사용자 PIE 절차를 준비했다. `DA_VehicleDefense_Test`에 Shield·재생·ArmorResistance·6방향 장갑 기준값을 저장하고, 원본 `DA_TestSUV`를 직접 수정하지 않는 별도 `DA_VehicleDefense_TestSUV`에 `DefaultDefenseData`를 연결했다. 또한 `DA_DamageArmorPenTest`를 BaseDamage 25·ArmorPenetration 50으로 생성해 기존 `DA_DamageAsset`의 BaseDamage 25·ArmorPenetration 0과 비교 가능한 관통 테스트 기준을 준비했다. 원본 `DA_TestSUV`와 `DA_TestSedan`은 `DefaultDefenseData=None`, Launcher·하드포인트·내구도·메시 참조와 SHA-256을 유지했다. DR-P0-06 Defense Automation Job `7b5dce50680f47a99a29b26b02a577d7`은 8/8 Success, Combat Runtime Job `54340630094a461ea3f7a1056622fd3c`은 필수 15/15·전체 24/24 Success와 Editor Exit Code 0으로 PASS했다. 이전 `WBP_TargetSelect.uasset` 파일 잠금과 `TS_P0_06.TargetHud` 실패는 해소됐다. 독립 AssetDump `ADumpEntityQuery.cpp`의 이전 실패 호출부가 정리된 상태를 확인한 뒤 공식 Build Job `0697e0552a0f4911a93384c5d3a8a928`을 실행했고, `CarFight_ReEditor Win64 Development`가 Exit Code 0과 `Result: Succeeded`로 PASS했다. 따라서 DR-P0-06은 Done이다. DR-P0-07은 원본 TestMap·CF-FQ-029 전투 에셋 비저장, `DA_VehicleDefense_TestSUV` 수동 배치, AP 0 발별 수치와 AP 50 PIE 전용 복제 체인을 포함하는 Procedure Ready / User Result Pending 상태다. 실제 사용자 결과 전에는 PASS 처리하지 않는다. 현재 단일 Active는 `CF-FQ-032`이며 `CF-FQ-029`는 LM-P0-06 체크포인트를 보존한 Paused 상태, `CF-FQ-019`는 Deferred다.

`CF-FQ-034`는 VehicleData의 HardpointSlots·MountProfiles, EquipmentPresetData, WeaponData, CF-FQ-031 Ammo와 CF-FQ-033 Defense를 하나의 검증·질량 Snapshot으로 해석한다. `FIT-P0-04`에서 Legacy·Snapshot Prepare, Weapon·Defense 원자 Commit, 실패 Rollback과 `AppliedFittingSnapshot` 소유를 구현했다. `FIT-P0-05`에서는 PreRegister 초기 질량 기록, BeginPlay 실제 VehicleMesh 질량 검증과 같은 Snapshot의 Weapon·Defense Commit을 구현했고 공식 Build와 전체 Automation을 통과했다. 남은 범위는 실제 Physics PIE, Light·Default·Heavy Mobility 측정, Ammo 질량과 Physics State 생성 뒤 다른 질량의 Field Runtime 재적용이다. `FFIT-P0-00`의 비전투·쿨타임 종료·차량 정지·Inventory 예약 성공 계약과 빈 슬롯 전용 Equip·분리된 Unequip/Equip 규칙은 유지한다.

`CF-FQ-035`는 Item Definition·Instance, VehicleCargo·Mounted 소유 상태, 접근 조회, Reservation, Atomic Transfer와 `INV-P0-04 Fitting Adapter`까지 완료했다. Adapter는 실제 ItemInstance를 Equipment·Defense Binding과 기존 Fitting Snapshot으로 결정론적으로 변환하며 Inventory Container, Reservation과 차량 Runtime을 변경하지 않는다. 다음 미구현 경계는 읽기 전용 ViewData·Events와, Field Fitting에서 Prepared Inventory Transaction·Runtime Apply·보상 Rollback을 조율하는 Coordinator다. 상점·가격·재화·월드 루팅·제작·내구도·SaveGame·네트워크는 후속으로 분리한다.

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
- CF-FQ-026의 TS-P0-08 Paused 체크포인트는 변경하지 않는다.
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
- Build Job 0fce6d253d9548dfa0ed39c94501ff47에서 Editor 빌드와 TargetSelect Automation 7/7 PASS
- TS-P0-08 사용자 PIE 통합 검증과 튜닝은 Paused
```

현재 체크포인트:

```text
재개 위치: TS-P0-08 P0 통합 검증과 튜닝
첫 결함: 후보 텍스트와 대상 겹침, 후보 범위 과대, 디버그 원 비가시
보존 증거: TS-P0-00~07 Done / TargetSelect Automation 7/7 PASS
다음 조치: 단일 대상 범위 검증 → 다중 후보 히스테리시스 분리 → HUD·디버그·성능 튜닝
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

- 현재 문서 버전: `v1.48.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 기능 큐 운영 방식 자체 변경 |
| Minor | 기능 후보/관리툴 후보/승격 기준 추가 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 11. 체인지로그

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
