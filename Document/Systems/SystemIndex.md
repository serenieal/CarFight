# SystemIndex

- Version: 1.24.0
- Date: 2026-08-24

- Status: Active
- Scope: `Document/Systems/` 하위 문서 위치 안내 색인

---

## 1. 문서 목적

이 문서는 `Document/Systems/` 폴더 안에 있는 시스템 문서들이 각각 **어떤 기능을 설명하는 문서인지** 빠르게 찾기 위한 색인이다.

이 문서는 설계 로드맵이나 개발 순서표가 아니다.
문서를 열기 전에 아래 표에서 필요한 기능을 찾고, 해당 경로의 문서를 확인하면 된다.

---

## 2. 최상위 문서

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/SystemIndex.md` | `Document/Systems/` 하위 문서들이 어디에 있고, 어떤 기능을 다루는지 정리한 색인 문서다. |
| `Document/Systems/SystemTemplate.md` | 새 시스템 문서를 작성할 때 사용하는 기본 템플릿이다. 문서 목적, 범위, 현재 역할, 책임, 비책임, 갱신 조건, 버전 관리를 어떤 형식으로 적을지 정의한다. |

---

## 3. Combat 폴더

현재 Combat 폴더 문서는 **현재 구현된 전투 런타임과 싱글플레이 로컬 전투 피드백 기준**을 기록한다.
직접 피해, Shield, 6방향 독립 Armor, ArmorPenetration, Vehicle Integrity, 최초 파괴 상태와 차량 finite Ammo·Reload는 현재 Systems 범위에 포함한다. 실제 부품별 손상, 도탄, 범위 피해, 완성형 파괴 물리와 서버 권한 전투는 후속 범위다.

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Combat/WeaponData.md` | `CF-FQ-008`에서 완료한 `UCFWeaponData` 정적 무기 DataAsset Current System이다. Identity·Mount·Mass·Fire·TargetUse·Launcher·Ammo·Projectile/FX 정적 설정, DataValidation, fallback과 legacy field 책임 경계를 기록한다. |
| `Document/Systems/Combat/WeaponFire.md` | 싱글플레이 로컬 차량 Pawn에서 Fire 입력을 발사 명령으로 만들고, Weapon Aim Solution을 기준으로 Projectile Actor 또는 Dummy HitScan 경로로 넘기며, 발사 결과와 거부 사유를 Aim / Debug / 후속 UI 피드백이 읽을 수 있게 남기는 현재 발사 기능 문서다. Ammo 수량·Reload 상태는 `UCFVehicleAmmoComp`가 소유한다. |
| `Document/Systems/Combat/Ammo.md` | `CF-FQ-031`에서 완료한 차량 finite Ammo Current System이다. WeaponInstanceId별 Loaded, AmmoId별 Reserve, SingleCycle Commit·Rollback, Ripple·Salvo 전체 예약, FullMagazine Reload, WeaponPanel `Loaded / MagazineCapacity + Reserve`와 출격 Ammo 질량 계약을 기록한다. Heavy·Ripple USER PIE를 완료했다. |
| `Document/Systems/Combat/FireFeedback.md` | `WeaponFire`가 남긴 로컬 발사 성공·실패·쿨다운·NoWeapon·AimBlocked·TurretAligning·MuzzleBlocked 결과를 Reticle 텍스트와 색상으로 표시한다. P0 상태 전환과 피드백 만료를 사용자 PIE로 확인했다. |
| `Document/Systems/Combat/Projectile.md` | `ProjectileData`, 공통 `CFProjectileActor`, `ProjectileMotorComp`와 `ProjectilePoolComp`를 통한 비추진 포탄·비유도 Rocket 이동, 점화·연소·BurnedOut 관성 비행, 지속형 Trail·Thruster 소켓/Fallback·독립 Scale, 종료 Reset·Pool 재사용·고속 Bounds, 고속 연속 충돌과 첫 Impact 피해를 기록한다. `CF-TC-020`, `CF-TC-023`, `CF-TC-024`를 사용자 PIE로 확인했다. |
| `Document/Systems/Combat/DamageHitContext.md` | Dummy HitScan과 Projectile의 시각 차체 Hit 결과, `HitComponentName`, 위치/노멀/입사 방향을 같은 `FCFDamageHitContext` 형식으로 기록한다. 이 Context는 현재 `HitDamage`의 공용 피해 적용 입력으로 사용된다. |
| `Document/Systems/Combat/HitDamage.md` | HitScan·Projectile의 `FCFDamageHitContext`를 정식 `VehicleDefenseComp` 진입점으로 연결하고, Shield·Armor 이후 Vehicle Integrity 적용, Legacy Fallback, 최초 파괴와 기존 `FCFDamageApplyResult` 호환을 기록한다. |
| `Document/Systems/Combat/VehicleDefense.md` | `VehicleDefenseData`와 `VehicleDefenseComp`가 소유하는 Shield, 재생, Front·Left·Right·Rear·Top·Bottom 독립 Armor, 방향 배율, ArmorPenetration, Armor Overflow, Vehicle Integrity 전달과 Fitting Defense Commit을 기록한다. |
| `Document/Systems/Combat/CombatFx.md` | 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 DataAsset 기반 Niagara로 정확히 한 번 표현한다. `NS_BasicHit` Impact, 차량별 `SM_Body.FX_Destroyed` 소켓, 최대 수명 안전 퓨즈와 최종 사용자 PIE PASS를 기록한 현재 전투 FX 문서다. |

---

## 4. Config 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Config/ProjectRuntimeConfig.md` | 프로젝트 시작 맵, 렌더링 기술, 하드웨어 타깃, 입력 백엔드, 축 기본값 같은 런타임 환경 설정을 설명하는 문서다. |

---

## 5. Input 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Input/Input.md` | 차량 입력 자산, 입력 매핑 컨텍스트, 장치 모드, 2D 입력, Legacy 입력 충돌 제어, 최종 차량 주행/카메라 입력 전달 흐름을 설명하는 문서다. |

---

## 6. Maps 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Maps/.gitkeep` | 현재 `Maps` 폴더 유지를 위한 빈 파일이다. 기능 설명 문서는 아직 없다. |

---

## 7. Network 폴더

현재 `Document/Systems/Network/`에는 Current System 문서가 없다. 과거 Dedicated Server / Multiplayer Spawn 구현 기록은 Current Systems에서 내려와 다음 Historical 경로에 보존한다.

```text
Document/ProjectSSOT/Archive/Systems/Network/ServerSpawn.md
```

서버/멀티 작업을 재개할 때는 이 Historical 기록을 그대로 Current로 복원하지 않고 현재 Source·ProjectSSOT를 다시 감사해 새 lifecycle을 연다.

---

## 8. Targeting 폴더

현재 Targeting 폴더는 선택 자체와 독립된 차량 Sensor Contact/Knowledge Current System을 기록한다. TargetSelect의 후보 검색·선택 수명과 Sensor의 Detection·Contact lifecycle·Knowledge 책임은 합치지 않는다.

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Targeting/SensorContact.md` | `CF-FQ-036` Sensor Runtime과 `CF-FQ-037` Scanner 입력·장비 통합을 합친 Current System이다. bounded Passive/Visual/Active Detection, Live·LastKnown·Lost·DestroyedHold, Tactical Analysis·Knowledge, actor-free Snapshot, Utility Scanner Fitting Source, Pawn-owned V Active Scan command와 Target/Radar HUD read-only 소비 경계를 기록한다. Radar Range/Zoom·동적 Blip과 CF-FQ-032 USER Visual은 후속 범위다. |


---

## 9. UI 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/UI/InGameUI.md` | `CF-FQ-032` 인게임 UI Current System이다. LocalPlayer `UCFUISubsystem → UCFUIRootWidget` 8 Layer 수명, Production `UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter` 데이터 경계, HUD·AimReticle·Target Marker·Pause·Radar·Weapon과 VehiclePanel persisted editable 구조의 현재 구현, USER Visual/Feel Deferred 경계를 기록한다. |
| `Document/Systems/UI/AimReticle.md` | `UCFUISubsystem`이 HUD Layer Z10에서 `WBP_AimReticle` singleton 수명과 Current Pawn Rebind를 소유하고, Widget은 Weak Pawn 참조로 `Image_CenterDot` 조준 레티클·`CurrentMuzzleDirection` 기반 `Image_WeaponReticle` 터렛 레티클과 로컬 Aim/FireFeedback 상태를 표시한다. 기존 Aim/FireFeedback USER PIE 의미 계약을 보존한다. |
| `Document/Systems/UI/DisplayTextPolicy.md` | 내부 식별자는 영문으로 유지하고, 화면에 보이는 UI/Debug UI 텍스트는 한국어로 표시한다는 표시 텍스트 정책 문서다. |
| `Document/Systems/UI/VehicleDebugPanel.md` | `VehicleDebug Panel`의 Navigation + Selected Section 구조, TopLevel Section, Camera Debug 편입 상태, 표시 언어 정책을 설명하는 문서다. |

---

## 10. Vehicles 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Vehicles/VehicleAim.md` | `VehicleCamera`가 만든 조준 결과와 Weapon Aim Solution을 Local 표시·검증 상태로 관리하고, 사용자 조준점과 `CurrentMuzzleDirection` 기반 터렛 레티클 월드 지점을 분리해 제공한다. 정렬 정책, `MuzzleBlocked`와 CF-FQ-025 터렛 레티클을 사용자 PIE로 확인했다. |
| `Document/Systems/Vehicles/VehicleCamera.md` | Look 입력을 차량 기준 누적 조준 상태로 변환하고, 카메라 모드, Aim Profile, 속도, 충돌 상태를 반영해 SpringArm, FOV, AimTrace를 계산/적용하는 차량 카메라 기능 문서다. |
| `Document/Systems/Vehicles/VehicleCoreDecisions.md` | 현재 차량 코어의 유지 결정, 교체 결정, 임시 운영 판단을 기록하는 결정 로그 문서다. 차량 코어 변경 전 확인해야 하는 기준 문서다. |
| `Document/Systems/Vehicles/VehicleData.md` | `UCFVehicleData`의 외형·Layout·Hardpoint·MountProfile·Fitting Mass·Movement·WheelVisual·Reference·Defense/Fx·DriveState 구성과 실제 Pawn 적용 순서를 기록한다. WSA 완료 기준으로 USER-authored Wheel Socket Scale → visual/physics size authority, FL shared fallback Right 예외, hot-reinit full-transform 복원 계약을 포함한다. |
| `Document/Systems/Vehicles/VehicleDrive.md` | 차량 입력을 Chaos Vehicle Movement에 적용하고, 속도/방향/접지/입력 상태를 바탕으로 DriveState를 계산/유지하는 주행 상태 기능 문서다. |
| `Document/Systems/Vehicles/VehiclePawnLegacy.md` | `CFModVehiclePawn / BP_ModularVehicle` 계열을 현재 주력 차량 Pawn이 아닌 레거시 계열로 정리하는 문서다. |
| `Document/Systems/Vehicles/VehicleRuntime.md` | 차량 Pawn이 VehicleData를 실제 주행/휠/Drive 설정에 반영하고 Drive/WheelSync 준비를 검증해 Ready 상태를 관리한다. WSA 완료 기준 Wheel Visual full-transform deterministic reapply, SocketScale/Legacy/Manual 전환, Right fallback handedness 전달을 포함한다. |
| `Document/Systems/Vehicles/VehicleSteering.md` | 게임패드 VehicleMove 2D 입력 방향을 목표 조향으로 해석하고, 제한 속도와 차량 속도 기반 중립 복귀 규칙을 거쳐 실제 조향값을 적용하는 문서다. |
| `Document/Systems/Vehicles/WheelSync.md` | 실제 Movement와 휠 회전 상태를 읽어 각 휠의 조향, 서스펜션, 스핀 시각 입력을 만들고 Anchor/Mesh에 적용한다. WSA 완료 기준 FL-only Right fallback의 per-wheel spin handedness와 absolute/delta local-axis spin 계약을 포함한다. |

---

## 11. 기능별로 찾기

| 찾고 싶은 내용 | 확인할 문서 |
| --- | --- |
| 무기 정적 DataAsset 계약, 장착 호환·질량·Fire·TargetUse·Launcher·Ammo 설정, DataValidation과 legacy/fallback 경계 | `Combat/WeaponData.md` |
| 현재 로컬 발사 명령, WeaponData 해석, 쿨다운, FireOrigin, 발사 결과 기록 | `Combat/WeaponFire.md` |
| 차량 finite Ammo, 무기별 장전량·탄종별 Reserve, Launcher 예약, FullMagazine Reload, WeaponPanel 탄약 표시와 출격 탄약 질량 | `Combat/Ammo.md` |
| 발사 성공/실패/쿨다운/무기 없음 상태를 Reticle, HUD와 시각 VFX로 표시하는 기준 | `Combat/FireFeedback.md` |
| 프로젝트 전역 게임 사운드 비지원 결정과 오디오 도입 금지 기준 | `Document/ProjectSSOT/04_ProjectDecisions.md` |
| Projectile Actor 활성화, 비유도 Rocket 추진 상태, Trail·Thruster 지속형 FX, 독립 FX Scale, 일반·고속 충돌, 첫 Impact 피해와 Pool Reset 기준 | `Combat/Projectile.md` |
| Dummy HitScan / Projectile 시각 차체 HitContext와 HitComponent 기록 | `Combat/DamageHitContext.md` |
| HitScan·Projectile 공용 피해 진입점, Vehicle Integrity 적용, Legacy Fallback과 최초 파괴 상태 | `Combat/HitDamage.md` |
| Shield, 6방향 Armor, 관통·Overflow, 재생과 방어층별 전체 결과 | `Combat/VehicleDefense.md` |
| Muzzle·Impact·Destroyed Niagara의 데이터 연결, 발생 위치, 1회성, 중복 방지와 잔류 안전 계약 | `Combat/CombatFx.md` |
| Reticle 목표점, 터렛 추적, Muzzle 방향을 하나의 Aim Solution으로 통합한 완료 설계·검증 기록 | `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md` |
| Sweep/Sub-stepping/보조 Sphere Sweep을 통한 고속 Projectile 연속 충돌 완료 설계·검증 기록 | `Document/Plan/Archive/ProjectileContinuousCollision/ImplementationDesign.md` |
| 프로젝트 시작 맵, 렌더링, 입력 백엔드 설정 | `Config/ProjectRuntimeConfig.md` |
| 입력 액션, 매핑 컨텍스트, 키보드/게임패드 입력 처리 | `Input/Input.md` |
| 과거 Dedicated Server 접속 후 차량 Pawn 생성과 Possess 기록 | `Document/ProjectSSOT/Archive/Systems/Network/ServerSpawn.md` |
| 차량 Sensor 탐지, ContactId, Live/LastKnown/Lost/DestroyedHold, Tactical Analysis·Knowledge, Scanner Utility 장비/Fitting Source와 V Active Scan 입력, actor-free Snapshot/HUD 소비 경계 | `Targeting/SensorContact.md` |

| UI 텍스트를 한국어로 표시하는 기준 | `UI/DisplayTextPolicy.md` |
| 레거시 `WBP_VehicleDebug` 문자열 표시 비교 기준 | `Document/ProjectSSOT/Archive/Systems/UI/VehicleDebug.md` |
| 차량 디버그 패널의 탭/섹션 구조 | `UI/VehicleDebugPanel.md` |
| 인게임 UI Root·Layer 수명, Production HUD 데이터 흐름, Pause, Radar·Target·Weapon UI의 현재 구현 | `UI/InGameUI.md` |
| 조준점/Reticle 표시, 로컬 발사 결과 피드백과 UISubsystem singleton/Rebind | `UI/AimReticle.md` |
| 차량 코어 변경 전 결정 기준 | `Vehicles/VehicleCoreDecisions.md` |
| 차량 DataAsset 구조 | `Vehicles/VehicleData.md` |
| 차량 BeginPlay 준비와 Ready 판정 | `Vehicles/VehicleRuntime.md` |
| 차량 주행 입력 적용과 DriveState | `Vehicles/VehicleDrive.md` |
| 게임패드 2D 조향 해석과 조향 복귀 | `Vehicles/VehicleSteering.md` |
| 바퀴 위치, 조향 피벗, 스핀, 휠 시각 동기화 | `Vehicles/WheelSync.md` |
| 차량 카메라, FOV, AimTrace | `Vehicles/VehicleCamera.md` |
| 차량 조준 상태, 로컬 발사 검증용 Aim 상태, OutOfArc / 조준각 경고 기준 | `Vehicles/VehicleAim.md` |
| 예전 `BP_ModularVehicle` 계열 정리 | `Vehicles/VehiclePawnLegacy.md` |

---

## 12. 문서 추가 시 갱신 규칙

`Document/Systems/` 아래에 새 시스템 문서를 추가하면 이 문서도 함께 갱신한다.

갱신할 위치:

1. 해당 폴더 섹션의 문서 표
2. 필요한 경우 `기능별로 찾기` 표
3. 아래 Changelog

---

## 13. Changelog

### v1.24.0 - 2026-08-24

- `UI/InGameUI.md v1.1.1`의 CF-FQ-039 VehiclePanel persisted editable Structure Readiness 감사를 Current UI owner 설명에 반영했다.
- 신규 기능 승격이 아니라 기존 `WBP_CFArmorSector` 6개 재사용 + BodyMap Canvas + 탑다운 VehicleSilhouette의 실제 persisted 구조를 문서화한 변경이다.
- USER Production Visual PASS와 CF-FQ-039 완료 상태는 변경하지 않았다.

### v1.23.0 - 2026-08-22

- 문서 물리 정리 closure를 반영해 Decommissioned `ServerSpawn`과 레거시 `VehicleDebug`를 Current Systems 색인에서 내리고 `ProjectSSOT/Archive/Systems/` Historical 경로로 연결했다.
- 물리 Archive로 이동된 완료 Plan의 설계·검증 참조를 `Document/Plan/Archive/` 실제 경로로 정상화했다.
- Current 구현 owner, PASS evidence와 USER 판정은 변경하지 않았다.

Migration: 현재 구현은 계속 `Document/Systems/`와 실제 Source/Asset을 우선한다. 과거 완료 설계·검증 또는 내려온 시스템 기준선이 필요할 때만 각 Archive 경로를 선택한다.

### v1.22.0 - 2026-08-22

- CF-FQ-032 post-closure remediation 완료 후 Current owner 버전을 `UI/InGameUI.md v1.1.0`, `UI/AimReticle.md v1.10.0`, `Targeting/SensorContact.md v1.2.0`으로 동기화했다.
- CF-FQ-032 Done과 USER Visual/Zoom Feel Deferred 상태는 변경하지 않았다.

Migration: CF-FQ-032 현재 구현은 최신 Systems와 실제 Source/Asset을 우선한다.

### v1.21.0 - 2026-08-22

- `CF-FQ-032 / UI-P0-11 Systems Promotion`을 반영해 `Document/Systems/UI/InGameUI.md v1.1.0`을 인게임 UI Current System으로 신규 등록했다.
- LocalPlayer `UCFUISubsystem → UCFUIRootWidget` 수명, Production `UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter` 경계, 실제 Pause·HUD·Radar·Target·Weapon 계약과 UI-P0-10 AI Runtime Technical Validation을 Current 지식으로 승격했다.
- `AimReticle.md v1.10.0`의 생성·수명 owner를 실제 Source에 맞춰 `UCFUISubsystem` HUD Layer singleton + Current Pawn Rebind로 교정했다. 기존 Aim/FireFeedback/Turret Reticle USER PASS 의미는 변경하지 않았다.
- UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART 잔여 Visual은 Deferred/Pending으로 유지하며 Current Systems 등록을 USER Visual PASS로 확대하지 않는다.

Migration: CF-FQ-032의 현재 구현 판단은 `UI/InGameUI.md`와 실제 Source/Asset을 우선한다. `UI/AimReticle.md`는 Aim/FireFeedback 상세 의미 owner로 유지하며 과거 Pawn direct AddToViewport 설명을 Current 생성 경로로 사용하지 않는다.

### v1.20.0 - 2026-08-18

- `CF-FQ-037 SCAN-P0-00~07` 완료를 반영해 `SensorContact.md v1.1.0`을 Scanner Utility 장비/Fitting Source와 Pawn-owned V Active Scan 입력까지 포함하는 Current System으로 갱신했다.
- SCAN-P0-06 USER PIE에서 5초 timed scan, 반복 입력 무연장과 Target Knowledge `???` 해제를 확인했고, 임시 관측 코드를 제거한 최종 Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 closure evidence로 연결했다.
- Radar Range/Zoom·동적 Blip·CF-FQ-032 USER Visual과 Sensor energy/heat·AI/Network는 완료 범위에 포함하지 않았다.

### v1.19.0 - 2026-08-15


- `Document/Systems/Targeting/SensorContact.md v1.0.0`을 `CF-FQ-036 SEN-P0-00~07` Technical Acceptance PASS의 Current System으로 신규 등록했다.
- Sensor의 bounded Passive/Visual/Active Detection, Contact lifecycle, Tactical Analysis·Knowledge, VehicleHealth authoritative DestroyedHold와 actor-free Snapshot 계약을 Targeting 색인에 추가했다.
- TargetSelect는 후보 검색·선택·TrackState owner로 유지하고 HUD는 Sensor Snapshot을 read-only 소비하며 Radar Range/Zoom·동적 Blip·CF-FQ-032 USER Visual은 완료로 해석하지 않는 책임 경계를 명시했다.
- 현재 기술 기준선은 Build `7dff9da7aaa24e76b0871348762c9d93` PASS, `CarFight.Sensor` `85d008613a104df9b7107795995c6ac5` 14/14 PASS다.

### v1.18.0 - 2026-08-15

- `Document/Systems/Combat/WeaponData.md v1.0.0`을 CF-FQ-008 Current System으로 신규 등록했다.
- WeaponData의 정적 SSOT, DataValidation, Launcher·Ammo·Projectile/Fitting 소비 경계와 legacy/fallback 계약을 Combat 색인에 추가했다.
- WeaponFire가 Ammo Runtime 상태를 직접 소유하지 않고 `UCFVehicleAmmoComp`가 Loaded·Reserve·Reload를 소유하는 현재 책임 경계를 색인 설명에 반영했다.

### v1.17.0 - 2026-08-15

- `Document/Systems/Vehicles/VehicleData.md`를 v2.0.0으로 전면 교정해 DA_PoliceCar 중심 구형 설명을 현재 `DA_TestSedan / DA_TestSUV` 기준으로 갱신했다.
- Layout·Hardpoint·MountProfile·Fitting Mass까지 확장된 현재 VehicleData 계약과 `bUseMovementOverrides` 실제 런타임 의미를 반영했다.
- CF-FQ-015 VD-P0-00~03의 Build·VehicleData Automation 3/3 Technical PASS를 기록하되 실제 주행감 USER 튜닝은 미완료 상태로 유지했다.

### v1.16.0 - 2026-08-13

- `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System으로 신규 등록했다.
- `CF-FQ-031`의 AMMO-P0-00~08, 공식 Build·Automation과 Heavy·Ripple USER PIE PASS를 완료 근거로 반영했다.
- Vehicle finite Ammo의 WeaponInstanceId별 Loaded, AmmoId별 Reserve, Launcher Sequence 예약, FullMagazine Reload, WeaponPanel 표시와 Fitting Ammo 질량을 Combat 현재 범위에 추가했다.
- Launcher 전체 완료 여부는 별도 `CF-FQ-029`가 소유하므로 Ammo 승격과 함께 Launcher를 자동 완료 처리하지 않는다.

### v1.15.0 - 2026-08-06

- `Document/Systems/Combat/VehicleDefense.md v1.0.0`을 Current System으로 신규 등록했다.
- Combat 현재 범위를 Shield, 6방향 독립 Armor, ArmorPenetration과 Vehicle Integrity까지 확장해 실제 구현과 일치시켰다.
- `HitDamage.md v1.1.0`의 정식 VehicleDefense 진입점, Legacy Fallback과 기존 Health·Pool 호환 관계를 색인에 반영했다.
- 공식 Build `48c0a81e19af4b20a17f628bcc7b723b`, Combat Automation 46/46과 DR-PIE-00~06 USER PASS를 `CF-FQ-033 Done` 근거로 등록했다.

### v1.14.0 - 2026-07-30

- `Document/Systems/Combat/Projectile.md`를 v1.5.0으로 갱신해 `CF-FQ-027 투사체 비행 FX` 완료 상태를 반영했다.
- Trail-only, Thruster-only, Trail+Thruster, 유효 소켓·Missing Socket Fallback 사용자 PIE PASS를 색인에 추가했다.
- Hit·LifeExpired Reset, Pool 20발 이상, Ribbon History 무잔류와 30 FPS 고속 Bounds PASS를 반영했다.
- `CF-FQ-027 Done / CF-TC-023 PASS`를 Current System으로 등록했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run이며 Runner 미노출 상태를 유지했다.

### v1.13.0 - 2026-07-28

- `Document/Systems/Combat/Projectile.md`를 v1.4.0으로 갱신해 `CF-FQ-028 발사체 추진 시스템`을 기존 Projectile Current System에 통합했다.
- `UCFProjectileMotorComp`, 점화 지연, Burning 고정 방향 가속, MaximumPropelledSpeed와 BurnedOut 관성 비행을 색인 설명에 추가했다.
- Trail·Thruster Origin/Niagara, 메시 소켓/Fallback, Pool Reset과 `RelativeTransform.Scale` 독립 FX Scale 계약을 추가했다.
- 최종 Build Job `e8b812bd479549299dd116f9bae8996f` Exit Code 0과 사용자 PIE Scale 0.2 정상 동작을 반영했다.
- `CF-FQ-028 Done / CF-TC-024 PASS`를 등록하고 반복 전투 확장 회귀를 `CF-FQ-019`로 이관했다.
- `CF-FQ-027` 별도 전체 체크리스트와 `CF-TC-023`은 자동 완료로 해석하지 않도록 유지했다.

### v1.12.0 - 2026-07-27

- `Document/Systems/Combat/CombatFx.md`를 Current System으로 등록했다.
- `CF-FQ-024 Done`, `CF-TC-021 PASS`와 최종 사용자 PIE 전체 PASS를 반영했다.
- `NS_BasicHit` Impact 현재 크기 승인과 차량별 `SM_Body.FX_Destroyed` 소켓 위치 기준을 색인에 추가했다.
- 기능별 찾기 표에 데이터 기반 Muzzle·Impact·Destroyed Niagara 런타임 문서를 연결했다.

### v1.11.0 - 2026-07-22

- `CF-FQ-025` Done과 `CF-TC-022` 사용자 PIE PASS를 반영했다.
- `AimReticle` 색인에 Image_CenterDot 조준 레티클과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클 책임을 추가했다.
- `VehicleAim` 색인에 터렛 레티클 월드 지점 제공과 기존 정렬·MuzzleBlocked 회귀 검증 상태를 반영했다.
- 기능별 현재 구현 설명을 이중 레티클 완료 기준에 맞게 갱신했다.

### v1.10.0 - 2026-07-15

- `CF-FQ-017` Done과 `CF-TC-014` 사용자 PIE PASS를 반영했다.
- `FireFeedback` 색인에 NoWeapon, AimBlocked, 피드백 만료 검증 완료를 추가했다.
- `AimReticle` 색인에 실패 상태 색상과 정상 상태 복귀 검증 완료를 추가했다.

### v1.9.0 - 2026-07-15

- `Document/Systems/Combat/HitDamage.md`를 Current System으로 등록했다.
- `CF-FQ-018` 최소 Damage Runtime의 공식 빌드와 사용자 PIE PASS를 반영했다.
- DamageHitContext를 실제 피해 적용 입력으로 사용하는 현재 관계를 색인에 반영했다.
- 장갑·모듈 피해와 완성형 파괴 연출은 후속 범위로 유지했다.

### v1.8.0 - 2026-07-14

- `VehicleAim`, `AimReticle`, `Projectile`, `DamageHitContext` 색인 설명을 현재 P0 사용자 PIE 완료 상태로 갱신했다.
- `CF-FQ-022` 정렬 중 발사 정책 true/false, 정렬 완료 탄착과 `MuzzleBlocked` 검증 완료를 반영했다.
- `CF-FQ-023` 고속 Projectile 집중 스트레스 검증 완료를 반영했다.
- 실제 HP 차감과 파괴 상태는 여전히 `CF-FQ-018` 미구현 범위로 유지했다.

### v1.7.0 - 2026-07-14

- Projectile 고속 연속 충돌 상태를 코드 구현·공식 Editor 빌드 완료, 사용자 PIE 검증 Pending으로 정정했다.
- `DamageHitContext` 설명에 고속 Projectile 경로 연결과 사용자 PIE 신뢰성 검증 대기를 분리해 반영했다.
- 오래된 `고속 터널링 미완료 한계` 표현을 제거하고 구현 상태와 검증 상태를 구분했다.
- 실제 피해 적용은 여전히 후속 범위임을 유지했다.

### v1.6.0 - 2026-07-13

- `WeaponFire.md`, `FireFeedback.md`, `AimReticle.md`, `VehicleAim.md` 설명을 AimFireAlignment 빌드 완료 / PIE Pending 상태로 갱신했다.
- Weapon Aim Solution, TurretAligning, MuzzleBlocked 표시 연결 상태를 색인 설명에 반영했다.

### v1.5.0 - 2026-07-13

- `VehicleAim.md`와 `AimReticle.md` 설명에 Reticle 목표와 Muzzle 발사 방향 정렬 한계를 반영했다.
- `Projectile.md`와 `DamageHitContext.md` 설명을 시각 차체 피격 완료 / 고속 Projectile 부분 완료 상태로 갱신했다.
- `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md`를 조준 해 통합 설계의 Historical evidence로 연결했다.
- `Document/Plan/Archive/ProjectileContinuousCollision/ImplementationDesign.md`를 고속 Projectile 연속 충돌 설계의 Historical evidence로 연결했다.
- `HitDamage` Plan 설명을 시각 피격 구현 완료 이후 Damage Runtime 대기 상태로 정정했다.

### v1.4.0 - 2026-07-13

- `DamageHitContext.md` 설명에 현재 차량 피격이 `VehicleMesh` Physics Asset 기준이라는 한계를 반영했다.
- 시각 차체 기반 피격 콜리전 분리와 피해 처리 완료 설계 기록 `Document/Plan/Archive/HitDamage/ImplementationDesign.md`를 Historical evidence로 연결했다.
- `SM_Body` 기반 무기 피격 전환은 아직 Systems 완료 기능이 아니라 Plan 단계임을 유지했다.

### v1.3.0 - 2026-07-09

- `Document/Systems/Combat/FireFeedback.md` 신규 문서를 Combat 폴더 색인에 추가했다.
- 기능별 찾기 표에 발사 성공/실패/쿨다운/무기 없음 상태를 표시하는 기준 문서로 `Combat/FireFeedback.md`를 추가했다.
- `Vehicles/VehicleAim.md` 설명에서 서버 검증 / 복제 시각화 표현을 제거하고 로컬 발사 검증 상태 / 로컬 시각화 상태 기준으로 정정했다.
- `UI/AimReticle.md`, `Combat/WeaponFire.md` 설명을 Reticle / FireFeedback 교통정리 이후 기준에 맞춰 보강했다.

### v1.2.0 - 2026-07-09

- `Document/Systems/Combat/` 섹션을 추가했다.
- 현재 구현된 전투 기능 문서 `WeaponFire.md`, `Projectile.md`, `DamageHitContext.md`를 색인에 등록했다.
- 아직 구현 기준으로 확인되지 않은 HP 차감, 파괴, 장갑/모듈 피해, 서버 권한 전투는 Combat Systems 현재 범위에 포함하지 않는다고 명시했다.

### v1.1.0 - 2026-06-19

- 싱글 플레이 1대 차량 고도화 전환 기준에 맞춰 Network 폴더를 Deferred 기록으로 표시했다.
- 기능별 찾기 표에서 서버 Spawn/Possess 항목이 현재 활성 작업처럼 보이지 않게 정리했다.

### v1.0.0 - 2026-06-02

- `Document/Systems/` 하위 문서 위치 안내 색인 문서로 최초 작성했다.
- 각 폴더별 문서 경로와 문서가 다루는 기능을 표로 정리했다.
- 기능별로 어떤 문서를 열면 되는지 찾기 표를 추가했다.

---

## 14. Migration

### v1.19.0 적용 안내

- `CF-FQ-036` 완료 이후 Sensor/Contact 현재 구현은 `Targeting/SensorContact.md v1.0.0`과 실제 Source를 우선한다.
- `Document/Plan/Archive/SensorContactPlan.md`는 완료 당시 설계·검증 evidence를 보존하는 Historical + Archived Path로 읽는다.
- TargetSelect 후보 검색·선택 수명과 Sensor Contact lifecycle·Knowledge를 합치지 않는다.
- Radar Range/Zoom·NormalizedPosition·동적 Blip 및 CF-FQ-032 Target/Radar USER Visual은 이 Current System 승격으로 자동 완료되지 않는다.
- CF-FQ-026 TS-P0-08 USER PIE도 별도 Pending 상태를 유지한다.

### v1.18.0 적용 안내

- `CF-FQ-008` 완료 이후 WeaponData 현재 구현은 `Combat/WeaponData.md`와 실제 `UCFWeaponData` 코드를 우선한다.
- `Document/Plan/Archive/WeaponDataPlan.md`는 완료 당시 설계·검증 기록인 Historical + Archived Path로 읽는다.
- `MagazineSize`와 `ReloadTimeSeconds`는 현재 Ammo Runtime의 정적 입력이며, Loaded·Reserve·Reload 진행 상태는 `UCFVehicleAmmoComp`가 소유한다.
- `HeatPerShot / MaxHeat`는 현재 과열 Runtime 완료를 의미하지 않는다.

### v1.15.0 적용 안내

- 차량 방어·손상 현재 구현은 `Combat/VehicleDefense.md`와 `Combat/HitDamage.md`를 함께 우선한다.
- `VehicleDefenseDamageDesign.md`는 완료 당시 설계·검증 기록이며 Current System을 대체하지 않는다.
- `VehicleData.DefaultDefenseData=None`은 오류가 아니라 기존 차량을 위한 정식 Legacy Fallback으로 읽는다.
- 실제 부품 손상, 도탄, 범위 피해와 서버 권한 피해는 Current System으로 간주하지 않는다.

### v1.14.0 적용 안내

- `CF-FQ-027`의 현재 구현 판단은 `Document/Systems/Combat/Projectile.md v1.5.0`을 우선한다.
- `Document/Plan/Archive/ProjectileFlightFx/ProjectileFlightFxPlan.md v1.0.0`은 완료 당시 설계·빌드·사용자 PIE 기록으로 유지한다.
- `CF-TC-023`은 PASS이며 CF-FQ-027을 Active 또는 Paused로 복원하지 않는다.
- Automation 실행은 Runner 미노출로 Not Run 상태를 유지한다.

### v1.13.0 적용 안내

- `CF-FQ-028`의 현재 구현 판단은 `Document/Systems/Combat/Projectile.md`를 우선한다.
- `Document/Plan/Archive/ProjectilePropulsionPlan.md`는 완료 당시 설계·빌드·PIE 체크포인트로 유지한다.
- `CF-TC-024`는 PASS이며 비유도 Rocket 추진과 Burning 기반 Thruster는 Current System이다.
- 반복 전투 확장 회귀는 `CF-FQ-019`가 소유하며 자동 착수하지 않는다.
- `CF-FQ-027`과 `CF-TC-023`의 별도 전체 Trail·Fallback·Pool 검증은 Paused 상태를 유지한다.

### v1.12.0 적용 안내

- `CF-FQ-024`의 현재 구현 판단은 `Document/Systems/Combat/CombatFx.md`를 우선한다.
- `Document/Plan/Archive/CombatFxAudio/ImplementationDesign.md`는 완료 당시 설계와 검증 기록으로 유지한다.
- `CF-TC-021`은 PASS이며 CombatFx는 P0 사용자 PIE 완료된 Current System이다.

### v1.10.0 적용 안내

- `CF-FQ-017`의 현재 구현 판단은 `Document/Systems/Combat/FireFeedback.md`와 `Document/Systems/UI/AimReticle.md`를 우선한다.
- `Document/Plan/Archive/ReticleFireFeedback/ImplementationDesign.md`는 완료 당시 설계와 검증 체크포인트 보존용으로 유지한다.
- NoWeapon, AimBlocked와 정상 발사 회귀는 P0 사용자 PIE 완료 상태로 읽는다.

### v1.9.0 적용 안내

- `CF-FQ-018`의 현재 구현 판단은 `Document/Systems/Combat/HitDamage.md`를 우선한다.
- `Document/Plan/Archive/HitDamage/ImplementationDesign.md`는 완료 당시 설계와 검증 체크포인트 보존용으로 유지한다.
- 최소 Damage Runtime은 Current System이지만 파괴 시 입력·물리 정지와 장갑·모듈 피해는 구현된 것으로 간주하지 않는다.

### v1.8.0 적용 안내

- `CF-FQ-022`와 `CF-FQ-023`은 P0 사용자 PIE까지 완료된 Current System 기준으로 읽는다.
- AimFireAlignment와 ProjectileContinuousCollision Plan은 완료 체크포인트 보존용이며 현재 구현 판단은 관련 Systems 문서를 우선한다.
- 실제 `BaseDamage`, 체력 감소와 파괴 상태의 완료 당시 설계 evidence는 `Document/Plan/Archive/HitDamage/ImplementationDesign.md`의 `CF-FQ-018` 기록에 보존한다.

### v1.7.0 적용 안내

- 기존 Systems 문서 경로와 책임은 변경하지 않는다.
- 고속 연속 충돌 코드와 공식 Editor 빌드 완료 사실은 색인에 반영하되, 사용자 PIE 검증 전에는 검증 완료된 Current System 동작으로 승격하지 않는다.
- 일반 속도 `SM_Body` 충돌의 기존 사용자 PIE 확인 결과는 유지한다.
- 고속 Projectile 신뢰성 검증과 실제 피해 적용은 계속 Pending으로 구분한다.
