# CarFight Active Work

- 문서 버전: v1.76
- 최근 갱신일: 2026-08-02
- 문서 상태: Current
- 역할: CarFight 게임 프로젝트에서 현재 실제로 진행 중인 작업과 각 대표 체크포인트를 연결하는 세션 복원용 색인

---

## 1. 운영 원칙

이 문서는 `main_game` 저장소의 CarFight 게임 기능만 관리한다.
별도 Git 저장소인 AssetDump와 GoPyMCP의 내부 작업, 릴리스 상태, 체크포인트는 등록하지 않는다.

```text
CarFight ActiveWork = CarFight 게임 기능의 현재 작업 선택
AssetDump ActiveWork = UE/Plugins/ue-assetdump/Documents/ActiveWork.md
GoPyMCP ActiveWork = GoPyMCP/Workspace/docs/ActiveWork.md
```

이 문서는 각 작업의 상세 설계와 구현 상태를 복사하지 않는다.
현재 활성 작업, 마지막 작업 초점, 작업 간 의존 관계와 대표 체크포인트 경로만 유지한다.

```text
ActiveWork = 현재 진행 중인 CarFight 작업을 찾는 세션 복원 색인
대표 Plan = 해당 작업의 상세 체크포인트와 다음 단계
Systems = 현재 구현 기준
ProjectSSOT = 프로젝트 우선순위와 상태 판단 기준
Git 및 실제 코드 = 문서와 충돌할 때 우선하는 최종 확인 대상
```

Candidate, Deferred, Archive 작업은 이 문서에 등록하지 않는다.

---

## 2. 현재 활성 작업

현재 단일 CarFight 활성 작업은 사용자가 명시적으로 전환한 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다.

```text
현재 Active: CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
현재 단계: UI-P0-01A~02 Code Complete / Official Build·Automation PASS / User PIE Pending / UI-P0-03 Not Started
대표 Plan: Document/Plan/InGameUIPlan.md v0.6.0
설계: Document/Plan/InGameUIDesign.md v0.5.0
로드맵: Document/Plan/InGameUIRoadmap.md v0.5.0
Source Changes: Applied — ACFPlayerController / UCFUISubsystem / UCFUIRootWidget / UCFPauseMenuWidget / CFUITypes / VehiclePawn·Drive Input Neutralization / UI Automation / CFSingleGameMode 연결
Asset Changes: None
Build: PASS — 4f2daa07187a41aea7c400386010b2a0 / CarFight_ReEditor Win64 Development / Exit Code 0
Automation: PASS — 6f0a9a5ebde5472b8b784a9103ce484d / 전체 CarFight 32 Success·0 Failed / 필수 회귀 17/17 Success
UI Foundation: Controller 소유 System·UI Context / Pawn 소유 Vehicle Gameplay Context / World별 C++ Root / 8개 Layer / 실제 World Pause / Menu Layer C++ Pause Menu / Continue·Back / Input Mode·Cursor·Focus
Input Neutralization: VehicleMove·Throttle·Steering·Brake·Handbrake·Look·Pressed Key 초기화 / 차량 물리 Velocity 보존
Progress Preservation: Launcher Ripple·Salvo와 Projectile·Motor Runtime을 취소하지 않고 World Pause 동안 정지 후 재개
Legacy Preservation: AimReticle, TargetSelect 생성·수명 경로 변경 없음
PIE: Not Run — 실제 Pause·Focus·입력 잔류·시퀀스·Projectile·Timer 재개 사용자 검증 Pending
다음 단계: UI-P0-03 HUD 데이터 계약 Not Started
이전 Active 체크포인트: CF-FQ-029 LM-P0-06 보존
피팅·인벤토리 보호: CF-FQ-034 FIT-P0-05 Initial Sortie Adapter Code Complete·Build·Automation PASS / Physics PIE·Mobility Pending, CF-FQ-035 INV-P0-04 Done / Field Fitting Coordinator Not Started
교차 기능 잠금: 차량 Gameplay Context는 Pawn 소유 / Controller Gameplay Context는 None 유지 / bVehicleRuntimeReady는 Core 호환 / Combat Ready는 별도 / 공용 통합 파일 단일 소유
일시중지 유지: CF-FQ-026 TargetSelect TS-P0-08 / CF-FQ-029 Launcher LM-P0-06
```

`Document/CodeWorkGate.md` v2.1에 따라 현재 AI 세션이 코드·설정·스크립트를 직접 구현한다. 기존 TaskSource와 WorkOrder는 구현 범위와 보호 기준을 제공하는 참고 문서이며, 최종 Codex YAML은 착수 조건이 아니다. 차량 Gameplay 입력은 Pawn Context가 소유하고 Controller Gameplay Context는 원자 이전 전 `None`으로 유지한다.

사용자 지정 병행 준비 작업 `CF-FQ-033 차량 방어·손상 런타임`은 Ready 상태에서 `DR-P0-00~06`을 완료했다. `DA_VehicleDefense_Test`와 별도 `DA_VehicleDefense_TestSUV`를 생성해 Shield·재생·ArmorResistance·6방향 장갑과 `DefaultDefenseData` 연결을 적용했고, `DA_DamageArmorPenTest`를 BaseDamage 25·ArmorPenetration 50 관통 비교용으로 추가했다. 원본 `DA_TestSUV`와 Legacy `DA_TestSedan`은 `DefaultDefenseData=None` 및 Launcher·하드포인트·내구도·메시 참조와 파일 해시를 그대로 유지했다. 최종 Apply Report tool 1.2.0 `success=true / protected_contract_passed=true`, 방어 자산 AssetDump 2/2와 DamageData AssetDump 2/2를 확인했다. DR-P0-06 재검증에서 Defense Automation Job `7b5dce50680f47a99a29b26b02a577d7`은 8/8 Success, 실패·경고·미실행 0으로 PASS했고 Combat Runtime Job `54340630094a461ea3f7a1056622fd3c`은 필수 15/15·전체 24/24 Success와 Editor Exit Code 0으로 PASS했다. 이전 `WBP_TargetSelect.uasset` 파일 잠금과 `TS_P0_06.TargetHud` 실패는 해소됐다. 독립 AssetDump `ADumpEntityQuery.cpp`의 이전 실패 호출부가 정리된 상태를 확인한 뒤 공식 Build Job `0697e0552a0f4911a93384c5d3a8a928`을 실행했고, `CarFight_ReEditor Win64 Development`가 Exit Code 0과 `Result: Succeeded`로 PASS했다. 따라서 DR-P0-06은 Defense Automation 8/8·Combat Automation 24/24·공식 Build PASS를 모두 충족해 Done이다. `VehicleDefenseDamageDesign.md v0.9.0`에서 DR-P0-07 보호형 사용자 PIE 절차를 준비했다. 현재 TestMap의 Sedan·HeavyCannon / SUV·RocketLauncher 진입, `DA_VehicleDefense_TestSUV` 수동 배치, VehicleDefenseComp 런타임 관찰, BaseDamage 25·AP 0 발별 예상값, AP 50 전용 복제 체인과 CF-FQ-029 원본 에셋 비저장 규칙을 고정했다. 상태는 `PIE Procedure Ready / User Result Pending`이며 실제 사용자 결과 전에는 PASS 처리하지 않는다. 현재 단일 Active는 `CF-FQ-032`이며 CF-FQ-033 전체 Done과 VehicleDefense Systems Current 승격도 수행하지 않았다.

---

## 3. 마지막 작업 초점

- 현재 작업 ID: `CF-FQ-032`
- 작업명: 인게임 전투 HUD 및 UI 프레임워크
- 현재 상태: `Active / UI-P0-01A~02 Code Complete / Build·Automation PASS / User PIE Pending`
- 현재 단계: `UI-P0-03 Not Started`
- 대표 Plan: `Document/Plan/InGameUIPlan.md v0.6.0`
- 설계: `Document/Plan/InGameUIDesign.md v0.5.0`
- 로드맵: `Document/Plan/InGameUIRoadmap.md v0.5.0`
- 적용 소스: `ACFPlayerController / UCFUISubsystem / UCFUIRootWidget / UCFPauseMenuWidget / CFUITypes / CFVehiclePawn / CFVehicleDriveComp / CFUIFoundationTests / CFUIPauseTests / CFSingleGameMode`
- Build: `4f2daa07187a41aea7c400386010b2a0 / Exit Code 0`
- Automation: `6f0a9a5ebde5472b8b784a9103ce484d / 전체 32 Success·0 Failed / 필수 17/17 Success`
- Asset: `변경 없음`
- PIE: `Not Run / 실제 Pause·Focus·입력·Launcher·Projectile·Timer User Pending`
- 보호: `CF-FQ-034 FIT-P0-05 Initial Sortie Adapter와 CF-FQ-035 INV-P0-04 Adapter 완료 상태, 기존 AimReticle·TargetSelect 유지`
- 다음 실행: `사용자 PIE 또는 UI-P0-03 HUD 데이터 계약`

### 이전 작업 체크포인트 — CF-FQ-029

- 현재 작업 ID: `CF-FQ-029`
- 작업명: 모듈형 런처 및 발사 인계
- 현재 상태: `Paused / LM-P0-01~05 구현·빌드·자산·AssetDump 완료 유지 / LM-P0-06 User PIE Checkpoint`
- 현재 단계: `LM-P0-06 Launcher Integration PIE Checkpoint`
- 대표 Plan: `Document/Plan/LauncherMissilePlan.md v0.10.0`
- 설계: `Document/Plan/LauncherMissileDesign.md v0.1.0`
- 로드맵: `Document/Plan/LauncherMissileRoadmap.md v0.5.0`
- 런처 상세: `Document/Plan/ModularLauncherPlan.md v0.5.0`
- 미사일 상세: `Document/Plan/MissileGuidancePlan.md v0.4.0`
- TaskSource: `Document/Plan/LaunchTaskSource.md v0.1.0`
- WorkOrder: `Document/Plan/LaunchWorkOrder.md v0.1.0`
- 확정 계약: `Command Target과 Initial Launch State 분리 / 가변 Muzzle / SingleCycle·Ripple·Salvo / 실패·취소·Volley 쿨다운 / Direct·Angled·Vertical Release / 차량 속도 상속 / 실제 사출 방향 안전 검사 / 발사 후 런처 독립 / 물리 제한형 유도와 정상 Miss`
- 적용 소스: `CFLauncherTypes v1.3.0 / UCFLauncherComp v1.1.0 / ACFVehiclePawn v2.136.0 / UCFVehicleWeaponComp v1.21.0 / UCFWeaponData v1.8.0 / CFLauncherSchedTests v1.1.0 / CFLauncherReleaseTests v1.0.0`
- Build: `ce88150dfbbc495d91d1e2b8cd7c5225 / CarFight_ReEditor Win64 Development / Exit Code 0`
- Automation: `Combat Runtime Job 85c1e7285a7946fa8f55182b6247f0f4 / 전체 CarFight 43/43 Success·0 Failed / 필수 회귀 21/21 Success / Launcher Scheduler 위치·Actor Snapshot·Direct Missile·Rocket·TargetSelect·Pool·Defense·Fitting·UI Pause 보호`
- Assets: `Applied / Independent AssetDump PASS — LauncherPost 10/10, VehiclePost 3/3, MapPost 19/19`
- Play Entry: `BP_CFVehiclePawn CDO → DA_TestSUV → RocketLauncher / TestMap _0 Sedan HeavyCannon 유지 / _1 SUV RocketLauncher`
- PIE: `Partial PASS — 플레이어 진입·Direct Launch Handoff·Ripple·Muzzle 순서·Command Target·중복 입력 방지·SequenceCompleted·Pool·SingleCycle PASS / Salvo Launcher 4 Muzzle 동시 사출 PASS / Damage 정식 런타임 통합 완료·사용자 PIE 재검증 Pending`
- 확정 정책: `같은 차량이 발사한 모든 탄종과 모든 Volley는 ActiveInstigatorActor 기준으로 서로 충돌하지 않음 / 다른 차량 Projectile은 기본 Block이며 요격 가능`
- 수정: `ACFProjectileActor v1.11.0 / UCFProjectilePoolComp v1.5.0 / UCFProjectileData v1.9.0 / ACFVehiclePawn v2.128.0 / CFProjectileCollisionTests v2.0.0`
- 빌드: `940272869b77450797347f76b427faf3 / CarFight_ReEditor Win64 Development / Exit Code 0`
- 사용자 PIE PASS: `Salvo 4발 정상 사출 / 동일 차량 Salvo Projectile의 사출 직후 상호 Impact 없음 / 차량·월드 대상 일반 충돌 유지`
- 보호 회귀: `CarFight.Launcher.LM_P0_03B.SchedulerContract 위치·Actor Snapshot PASS / CarFight.Missile.MG_P0_01_04.DirectRuntimeContract PASS / CarFight.Projectile.LM_P0_06.SourceIsolation PASS — 같은 Volley의 목표 고정, Direct Missile, 기존 Rocket·Pool 격리·요격 계약 보호`
- Deferred 사용자 PIE: `다른 차량 Projectile·Hitscan 요격 / 서로 다른 탄종·Volley 교차 격리 / AI 또는 별도 적 사격 주체가 준비되는 전투 회귀에서 확인`
- Pending: `실제 사출 방향 MuzzleBlocked / AngledEjection / VerticalEjection / CarrierVelocityRatio / 기본값 복구`
- Deferred: `후속 발사 실패 정책 ContinueRemaining·StopSequence — 후속 Muzzle만 실패시키는 전용 배치 또는 테스트 훅 준비 후 검증 / 수동 CancelFireSequence — BlueprintCallable API는 있으나 현재 입력·UI·게임플레이 호출 경로 없음 / OwnerDestroyed·WeaponChanged·TurretMountChanged 취소는 해당 런타임 전환 수단 준비 후 검증`
- 병행 Ready 작업: `CF-FQ-030 물리 제한형 미사일 비행·유도 / Ready for Manual PIE / MG-P0-01~04 Direct Runtime·Test Assets Applied / Build ce88150dfbbc495d91d1e2b8cd7c5225 PASS / Combat Regression 43/43·Required 21/21 PASS / 사용자 Manual PIE Pending`
- 통합 안정화: `한 Volley의 CommandTargetLocation과 GuidanceTargetActor를 첫 발사 순간 함께 약한 Snapshot으로 고정 / 후속 Ripple·Salvo는 현재 TargetSelect를 재조회하지 않음`
- 다음 실행: `LM-P0-06 MuzzleBlocked·Ejection·Carrier Velocity와 Missile Direct 여섯 시나리오 사용자 PIE`

### 일시중지 작업

- 작업 ID: `CF-FQ-029`
- 작업명: 모듈형 런처 및 발사 인계
- 중단 위치: `LM-P0-06 Launcher Integration PIE`
- 완료 유지: `LM-P0-01~05 구현·빌드·자산·AssetDump PASS, LM-P0-06 부분 사용자 PIE PASS`
- 체크포인트: `Document/Plan/LauncherMissilePlan.md v0.10.0`

- 작업 ID: `CF-FQ-026`
- 작업명: 타겟 선택 시스템
- 중단 위치: `TS-P0-08 P0 검증과 튜닝`
- 완료 유지: `TS-P0-00~07 Done`, Build Job `0fce6d253d9548dfa0ed39c94501ff47`, TargetSelect Automation 7/7 PASS
- 재개 시 첫 결함: 후보 텍스트와 대상 겹침, 후보 범위 과대, 디버그 원 비가시
- 체크포인트: `Document/Plan/TargetSelectPlan.md`

### 최근 완료 작업

- 작업 ID: `CF-FQ-027`
- 작업명: 투사체 비행 FX
- 완료 상태: `Done / User PIE PASS / CF-TC-023 PASS / Systems Current`
- 완료 기준: Trail-only·Thruster-only·Trail+Thruster, 유효 소켓·Missing Socket Fallback, Hit·LifeExpired Reset, Pool 20발 이상, Ribbon History 무잔류와 30 FPS 고속 Bounds PASS
- 공식 빌드: `445848ab0f7749fcab8188b164bb1487` Foundation PASS / `e8b812bd479549299dd116f9bae8996f` Scale 보정 PASS
- Automation: `PASS — UE/Saved/Automation/CombatRuntime/index.json의 CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract Success 확인 / 실행 수단은 작업 전용 임시 경로 / 공용 재실행 진입점은 정의하지 않음`
- 현재 구현 문서: `Document/Systems/Combat/Projectile.md v1.5.0`
- 완료 체크포인트: `Document/Plan/ProjectileFlightFxPlan.md v1.0.0`

- 작업 ID: `CF-FQ-028`
- 작업명: 발사체 추진 시스템
- 완료 상태: `Done / User PIE PASS / CF-TC-024 PASS / Systems Current`
- 완료 기준: 비유도 Rocket InitialSpeed 분리, IgnitionDelay, Burning 고정 방향 가속, MaximumPropelledSpeed, BurnedOut 관성 비행, Burning 기반 Thruster와 독립 FX Scale
- 최종 빌드: `e8b812bd479549299dd116f9bae8996f / Exit Code 0`
- 현재 구현 문서: `Document/Systems/Combat/Projectile.md`
- 완료 체크포인트: `Document/Plan/ProjectilePropulsionPlan.md`
- 후속 회귀: `CF-FQ-019 Deferred / 런처·미사일 통합 범위 확정 후 재설계`

- 작업 ID: `CF-FQ-024`
- 작업명: 전투 FX
- 완료 상태: `Done / User PIE PASS / CF-TC-021 PASS`
- 완료 기준: `UCFCombatFxData`·`UCFCombatFxComp`, 데이터 기반 Muzzle·Impact·Destroyed FX, `NS_BasicHit` Impact, 차량별 `SM_Body.FX_Destroyed` 위치와 Loop 안전 퓨즈
- 빌드 기준: 공식 Admin Build 3건 PASS + 최신 사용자 직접 Editor 빌드 PASS
- 현재 구현 문서: `Document/Systems/Combat/CombatFx.md`

- 작업 ID: `CF-FQ-025`
- 작업명: 이중 레티클 및 터렛방향 시각화
- 완료 상태: `Done / User PIE PASS`
- 완료 기준: `Image_CenterDot` 조준 레티클과 `CurrentMuzzleDirection` 기반 `Image_WeaponReticle` 터렛 레티클 분리
- 후속 논의: 투사체 착탄 위치를 Reticle UI와 분리한 월드 공간 3D 표현으로 설계
- 완료 체크포인트: `Document/Plan/ReticleAimDirection/ImplementationDesign.md`

사용자가 특정 CarFight 작업을 지정하면 이 마지막 작업 초점보다 사용자가 지정한 작업을 우선한다.
AssetDump 또는 GoPyMCP 작업을 지정하면 이 문서를 사용하지 않고 해당 독립 저장소의 `ActiveWork.md`로 진입한다.

---

## 4. 작업 간 의존 관계

```text
CF-FQ-022 Done
+
CF-FQ-023 Done
→ 신뢰 가능한 Aim Solution과 DamageHitContext 입력 계약 확보
→ CF-FQ-018 Damage Runtime 구현 가능
```

```text
CF-FQ-024 Done
+
CF-FQ-023 Done
→ 일회성 CombatFx와 신뢰 가능한 Projectile Pool 기준 확보
→ CF-FQ-027 투사체 비행 FX Done / 사용자 PIE 전체 행렬 완료 / CF-TC-023 PASS
   +
   CombatPlan Rocket·Missile 분류 결정
   → CF-FQ-028 발사체 추진 시스템 Done
      → PP-P0-00~09 구현·빌드·사용자 PIE 완료
      → CF-TC-024 PASS
      → Document/Systems/Combat/Projectile.md Current System 승격
         +
                  기존 Aim·Turret·FireOrigin·TargetSelect 기반
         → CF-FQ-029 모듈형 런처 및 발사 인계 Paused / LM-P0-06 Checkpoint
            → LM-P0-01~05 구현·빌드·자산 검증 완료 유지
            → LM-P0-06 사용자 PIE 체크포인트 보존
            → CF-FQ-030 물리 제한형 미사일 비행·유도 Ready
→ CF-FQ-019 주행·전투 반복 및 확장 회귀 Deferred / CF-FQ-029·030 구현 범위 확정 후 재설계
```

```text
CF-FQ-017 Done
→ ReticleFireFeedback Plan 완료 및 Archive 검토 대상
```

```text
CF-FQ-016 / 017 / 018 / 022 / 023 / 024 / 025 Done
→ 발사·조준·피격·피해·UI·이중 레티클·전투 FX 기준 확보
→ CF-FQ-024 전투 FX Done / User PIE PASS / CF-TC-021 PASS
   → 데이터 기반 Muzzle·Impact·Destroyed 일회성 FX 완료
   → NS_BasicHit Impact 현재 크기 승인
   → 차량별 SM_Body.FX_Destroyed 소켓 위치 완료
   → Document/Systems/Combat/CombatFx.md Current System 승격
→ CF-FQ-026 타겟 선택 시스템 Paused
   → TS-P0-00~07 Done
   → TS-P0-08 사용자 PIE 검증과 튜닝 체크포인트 보존
→ 프로젝트 전역 사운드 비지원 결정 유지: 게임 오디오 자산·런타임·모듈을 신규 작업에 포함하지 않음
→ CF-FQ-019 주행·전투 반복 테스트 Deferred / 현재 착수 대상 아님 / 런처·미사일 이후 재검토
→ CF-FQ-020 조작감·전투 템포·피드백 개선
→ CF-FQ-021 핵심 게임 루프 검증
```

---

## 5. 세션 복원 규칙

사용자가 CarFight 작업에 대해 `이전 작업 이어서 진행해줘`라고 요청하면 다음 순서로 복원한다.

```text
1. 저장소 루트 AGENTS.md 확인
2. main_game Git 상태 확인
3. Document/Document_Entry.md 확인
4. 이 ActiveWork.md 확인
5. 마지막 작업 초점의 대표 Plan 체크포인트 확인
6. 관련 Systems, ProjectSSOT, 실제 코드와 에셋 확인
7. 완료 범위, 미완료 범위, 빌드 상태, PIE 상태와 다음 단계를 먼저 보고
8. 미완료 단계부터 작업 재개
```

사용자가 작업 ID나 작업명을 지정하면 해당 행의 대표 체크포인트를 선택한다.
체크포인트와 실제 저장소가 다르면 실제 Git 상태와 코드를 우선한다.
중첩 저장소 작업은 해당 저장소의 Git 상태와 독립 문서체계를 사용한다.

---

## 6. 세션 인계 규칙

사용자가 CarFight 세션에서 `새 세션 인계 준비해줘`라고 요청하면 다음을 수행한다.

```text
1. main_game Git 상태 재확인
2. 이번 세션에서 상태가 변경된 CarFight 활성 작업의 대표 Plan 체크포인트 갱신
3. 이 문서의 활성 작업 상태, 의존 관계와 마지막 작업 초점 갱신
4. 문서와 실제 CarFight 코드 상태 교차검증
5. 새 세션에서 사용할 짧은 시작 문구 제공
```

AssetDump와 GoPyMCP 상태를 이 문서에 복사하지 않는다.
장문의 대화 요약을 새 SSOT로 만들지 않는다.
세션별 로그 파일을 누적 생성하지 않는다.

---

## 7. 갱신 조건

다음 중 하나가 CarFight 게임 프로젝트에서 발생했을 때 관련 대표 Plan 체크포인트와 필요한 경우 이 문서를 갱신한다.

- 실제 CarFight 코드나 에셋 패치 적용
- 중요한 설계 결정 확정
- 빌드 성공 또는 실패
- 사용자 PIE 결과 수신
- 오류 원인 또는 차단 요소 확정
- 다음 작업 단계 변경
- CarFight 작업 카테고리 또는 마지막 작업 초점 변경
- 작업이 Active에서 Done, Blocked, Deferred 또는 Archive 대상으로 전환

AssetDump 또는 GoPyMCP 내부 상태 변화만으로는 이 문서를 갱신하지 않는다.
단순 설명, 조사 또는 상태가 변하지 않은 질의응답만으로는 갱신하지 않는다.

---

## 8. Changelog

### v1.76 - 2026-08-02

- Launcher Ripple·Salvo가 CommandTargetLocation만 고정하고 후속 발사마다 현재 TargetSelect의 GuidanceTargetActor를 다시 읽던 교차 기능 결함을 수정했다.
- `FCFLauncherCommandTargetSnapshot`으로 첫 발사 순간 위치와 약한 Actor 참조를 함께 보존하고 첫 Projectile과 모든 후속 Projectile이 같은 값을 사용하도록 연결했다.
- 공식 Editor Build `ce88150dfbbc495d91d1e2b8cd7c5225` / Exit Code 0과 전체 CarFight Automation `85c1e7285a7946fa8f55182b6247f0f4` / 43/43·필수 21/21 Success를 기록했다.
- `CF-FQ-030`을 `MG-P0-01~04 Direct Runtime·Test Assets Applied / Ready for Manual PIE`로 정정하고 Done·Systems Current 승격은 수행하지 않았다.
- Unreal Asset, 현재 단일 Active `CF-FQ-032`, Paused `CF-FQ-029·026`과 기존 사용자 PIE 결과는 변경하지 않았다.

### v1.75 - 2026-08-02

- 검증 결과, 증거, 실행 수단과 공용 재실행 진입점을 분리하는 `CodeWorkGate.md v2.2` 기준을 연결했다.
- `CF-FQ-027`의 Current Automation 상태를 최신 `UE/Saved/Automation/CombatRuntime/index.json` 근거의 `PFX_P0_01.RuntimeContract Success`로 정정했다.
- 해당 실행 수단은 작업 전용 임시 경로였으며 공용 저장소 도구나 영구 재실행 계약으로 승격하지 않았음을 명시했다.
- 과거 Runner 미노출과 Not Run 기록은 해당 날짜·체크포인트의 역사 상태로만 유지하고 현재 실행 가능 여부를 대신하지 않도록 했다.

### v1.74 - 2026-08-02

- 차량 Runtime Ready를 Core와 Combat으로 분리하고 기존 `bVehicleRuntimeReady`를 Core 호환값으로 유지한 상태를 기록했다.
- 차량 Gameplay 입력은 Pawn `DefaultInputMappingContext`, Controller는 System·UI 입력만 소유하도록 현재 소유권을 명시했다.
- `CF-FQ-034 FIT-P0-05` Initial Sortie Adapter와 `CF-FQ-035 INV-P0-04` Fitting Adapter 완료 상태를 보호 체크포인트에 반영했다.
- Inventory Commit과 Runtime Apply를 조율하는 Field Fitting Coordinator는 Not Started로 분리했다.
- 공용 통합 파일 단일 소유 규칙과 `CodeWorkGate.md v2.1`을 현재 작업 규칙으로 연결했다.
- UI-P0-03과 Field Fitting Runtime은 착수하지 않았다.

### v1.73 - 2026-08-01

- `CF-FQ-032 UI-P0-02` 실제 싱글플레이 World Pause와 Pause Menu 수명을 Code Complete로 기록했다.
- 차량 Drive·Look·Pressed Key 입력 중립화와 Launcher Ripple·Salvo 상태 보존 정책을 기록했다.
- `UCFPauseMenuWidget` Menu 레이어 단일 인스턴스, Continue·Pause·Back 해제와 Focus 기반을 기록했다.
- 최종 공식 Editor Build `4f2daa07187a41aea7c400386010b2a0` / Exit Code 0을 기록했다.
- 최종 Automation `6f0a9a5ebde5472b8b784a9103ce484d` / 전체 32/32·필수 17/17 Success를 기록했다.
- 피팅 `FIT-P0-03` Compatibility·MassSnapshot과 Launcher·Projectile·Propulsion 보호 회귀 PASS를 유지했다.
- 사용자 PIE와 Systems 승격은 수행하지 않았고 다음 코드 단계는 `UI-P0-03 Not Started`로 남겼다.

### v1.72 - 2026-08-01

- 사용자 지정에 따라 현재 단일 Active 작업을 `CF-FQ-032`로 전환했다.
- `UI-P0-01A~01B` C++ 기반, 공식 Editor Build와 전체 CarFight Automation 29/29 PASS를 기록했다.
- 기존 Pawn Gameplay 입력과 AimReticle·TargetSelect를 보존하고 Unreal Asset 변경이 없음을 기록했다.
- 이전 `CF-FQ-029 LM-P0-06` 체크포인트를 일시중지 작업으로 보존했다.
- `CF-FQ-034 FIT-P0-03` 소스·문서·에셋 미변경 보호 상태를 기록했다.
- 다음 실행을 `UI-P0-02 완전 Pause`로 지정했으며 사용자 PIE와 Systems 승격은 수행하지 않았다.

### v1.71 - 2026-07-31

- CF-FQ-033 DR-P0-07 사용자 PIE 실행 절차를 `VehicleDefenseDamageDesign.md v0.9.0`에 준비했다.
- 원본 TestMap·BP_CFVehiclePawn·DA_TestSUV와 Launcher·Projectile·Pool·추진·요격 에셋을 재저장하지 않는 복제 맵 기준을 기록했다.
- Sedan·HeavyCannon AP 0 조작 차량, `DA_VehicleDefense_TestSUV` 방어 대상, VehicleDefenseComp 런타임 관찰과 발별 예상값을 고정했다.
- AP 50 DamageData가 자동 연결되지 않는 상태와 PIE 전용 복제 체인 원칙을 기록했다.
- 상태는 PIE Procedure Ready / User Result Pending으로 유지하고 DR-P0-07 PASS, CF-FQ-033 Done과 Systems Current 승격은 수행하지 않았다.
- 코드·에셋·Build·Automation은 변경하거나 반복하지 않았으며 현재 단일 Active CF-FQ-029 / LM-P0-06을 유지했다.

### v1.70 - 2026-07-31

- CF-FQ-033 DR-P0-06 공식 Editor Build를 단독 재검증했다.
- AssetDump `ADumpEntityQuery.cpp`의 이전 실패 호출부가 `MakeEntityQueryStringArray` 기반으로 정리된 현재 상태를 확인했다.
- 공식 Build Job `0697e0552a0f4911a93384c5d3a8a928`에서 `CarFight_ReEditor Win64 Development`가 Exit Code 0, `Result: Succeeded`로 통과했다.
- 기존 Defense Automation 8/8과 전체 Combat Automation 필수 15/15·전체 24/24 PASS 증거를 유지하고 반복 실행하지 않았다.
- CF-FQ-033은 Ready / DR-P0-00~06 Done으로 갱신하고 다음 단계를 DR-P0-07 사용자 PIE로 이동했다.
- 현재 단일 Active CF-FQ-029 / LM-P0-06을 유지하고 DR-P0-07·전체 Done·Systems Current 승격은 수행하지 않았다.

### v1.69 - 2026-07-31

- CF-FQ-033 DR-P0-06 공식 Editor Build와 전체 Automation을 재검증했다.
- Defense Automation Job `7b5dce50680f47a99a29b26b02a577d7`에서 CarFight.Damage 8/8 Success와 Exit Code 0을 확인했다.
- Combat Runtime Job `54340630094a461ea3f7a1056622fd3c`에서 필수 15/15·전체 24/24 Success, 실패 0과 Editor Exit Code 0을 확인했다.
- `WBP_TargetSelect.uasset` 파일 잠금 및 `TS_P0_06.TargetHud` 실패가 해소됐음을 기록했다.
- 공식 Build Job `15d79dff7c6c4e8680bde3eb4052b27a`는 독립 AssetDump `ADumpEntityQuery.cpp` 컴파일 오류로 Exit Code 6을 재현했다.
- CF-FQ-033은 Ready / DR-P0-06 Automation PASS·Build Blocked로 유지하고 현재 단일 Active CF-FQ-029 / LM-P0-06, DR-P0-07 Pending, 전체 Done·Systems Current 미승격 상태를 보존했다.

### v1.68 - 2026-07-31

- CF-FQ-033 DR-P0-05 테스트 DataAsset 연결을 완료했다.
- `DA_VehicleDefense_Test`와 `DA_VehicleDefense_TestSUV` 생성·연결, 원본 SUV·Legacy Sedan 보호 해시와 독립 AssetDump 2/2 Success를 기록했다.
- `DA_DamageArmorPenTest` BaseDamage 25·ArmorPenetration 50 생성, 기존 `DA_DamageAsset` AP 0 보호와 DamageData AssetDump 2/2 Success를 기록했다.
- Defense Automation 8/8 Success를 확인했다.
- DR-P0-06 공식 Build는 AssetDump dirty 컴파일 오류, 전체 Combat 회귀는 WBP_TargetSelect 파일 잠금으로 차단돼 완료 처리하지 않았다.
- CF-FQ-033은 Ready, 현재 단일 Active는 CF-FQ-029 / LM-P0-06으로 유지하고 DR-P0-07·전체 Done·Systems Current는 수행하지 않았다.

### v1.67 - 2026-07-31

- CF-FQ-033 DR-P0-04 Debug와 Blueprint 이벤트 완료, 공식 Editor Build와 Combat Runtime 필수 15/15·전체 24/24 Success를 기록했다.
- 마지막 전체 차량 피해 결과 캐시, Legacy Fallback 캐시, BlueprintPure 조회 API와 VehicleDebug 방어 섹션 적용 상태를 반영했다.
- CF-FQ-033은 Ready, 다음 단계는 DR-P0-05로 유지하고 현재 단일 Active CF-FQ-029 / LM-P0-06은 변경하지 않았다.
- DR-P0-05·DR-P0-07 완료, CF-FQ-033 전체 Done과 Systems Current 승격은 수행하지 않았다.

### v1.66 - 2026-07-31

- 사용자 지정 병행 작업 `CF-FQ-033 DR-P0-03`의 Pawn·HitScan·Projectile 정식 방어 통합을 Done으로 기록했다.
- `VehicleDefenseComp` 기본 생성, VehicleData 초기화, Ready·LegacyFallback·Missing 런타임 상태를 기록했다.
- HitScan과 Projectile 첫 Impact가 `UCFVehicleDefenseComp::TryApplyDamageToActor`를 사용하고 기존 Health Debug·Pool 호환 결과를 유지함을 기록했다.
- 공식 Editor Build Job `7c6c3f2e6d3647d4af4e63bf537933c5` / Exit Code 0을 기록했다.
- Combat Runtime Automation Job `95bb9b328ed2443baccf0fb59e3a0cab` / 필수 14개·전체 23개 Success / 실패 0을 기록했다.
- `CF-FQ-029` Launcher·Launch Context·Pool·추진·동일 차량 격리·다른 차량 요격 보호 회귀 PASS를 기록했다.
- 에셋과 런처·로켓 설정은 수정하지 않았고 현재 단일 Active를 `CF-FQ-029 / LM-P0-06`으로 유지했다.
- `CF-FQ-033` 다음 단계를 `DR-P0-04 Debug와 Blueprint 이벤트`로 이동했으며 전체 Done·Systems Current·사용자 PIE 완료 처리는 하지 않았다.

### v1.65 - 2026-07-31

- 사용자 지정 병행 작업으로 `CF-FQ-033 DR-P0-02` VehicleDefenseComp와 VehicleHealthComp 호환 확장을 구현 완료했다.
- 쉴드, 6방향 독립 장갑, 방향 판정, 장갑 관통·초과 피해, 차량 내구도 분배와 쉴드 재생을 적용했다.
- 기존 Health API·프로퍼티·이벤트 이름을 유지하고 명시 Integrity 피해 API와 Legacy Fallback을 추가했다.
- 첫 빌드 테스트 이름 충돌과 첫 Automation Fixture 비활성 문제를 보정했다.
- 최종 Editor Build Job `82ab34d50c62451fa74ca17ff00acad4` / Exit Code 0을 기록했다.
- `CarFight.Damage` 전체 Automation 6 Success / 0 Failed / 0 Not Run을 기록했다.
- CF-FQ-029 Launcher·Projectile·Weapon·Pawn 보호 파일과 에셋은 수정하지 않았다.
- CF-FQ-033은 `Ready / DR-P0-03 Protected Integration Gate`이며 현재 단일 Active는 `CF-FQ-029 / LM-P0-06`으로 유지했다.

### v1.64 - 2026-07-31

- 사용자 지정 병행 작업으로 `CF-FQ-033 DR-P0-01` Foundation을 구현 완료했다.
- `CFDamageRuntimeTypes`, `UCFVehicleDefenseData`, `VehicleData.DefaultDefenseData`와 DataContract Automation을 추가했다.
- 공식 Editor Build Job `bdc3e14d513549379d7f0768ad1ed7bd` / Exit Code 0과 Automation 1 Success / 0 Failed를 기록했다.
- CF-FQ-029 Launcher·Projectile·Weapon·Pawn 보호 파일과 에셋은 수정하지 않았다.
- Legacy 피해 필드 숨김은 CF-FQ-029 보호 범위와 충돌하므로 후속 직렬화 호환 패치로 이관했다.
- 현재 단일 Active는 `CF-FQ-029 / LM-P0-06`으로 유지하고 CF-FQ-033은 `Ready / DR-P0-02 Ready`로 기록했다.

### v1.63 - 2026-07-30

- 사용자 결정에 따라 후속 발사 실패 정책 `ContinueRemaining·StopSequence` 검증을 현재 LM-P0-06 즉시 완료 조건에서 제외했다.
- 해당 정책은 후속 Muzzle만 의도적으로 실패시키는 전용 배치 또는 테스트 훅이 준비된 뒤 검증하도록 Deferred했다.
- 현재 즉시 Pending은 실제 사출 방향 MuzzleBlocked, AngledEjection, VerticalEjection, CarrierVelocityRatio와 기본값 복구로 축소했다.
- 코드와 에셋은 변경하지 않았으며 문서 상태만 정정했다.

### v1.62 - 2026-07-30

- 실제 코드를 재검토해 `CancelFireSequence()`는 BlueprintCallable API로 존재하지만 현재 플레이어 입력, UI와 게임플레이 이벤트에서 호출되지 않음을 확인했다.
- 기존 `ContinueRemaining·취소` 검증 표현을 후속 발사 실패 정책인 `ContinueRemaining·StopSequence`와 수동 취소로 분리했다.
- `StopSequence`는 후속 예약 발사가 실패할 때 `Cancelled / ShotFailed`로 전환되는 자동 중단이며 직접 취소 입력이 아니다.
- 수동 취소와 OwnerDestroyed·WeaponChanged·TurretMountChanged 취소는 현재 상태 전환 수단이 없어 후속 검증으로 Deferred했다.
- LM-P0-06의 즉시 Pending은 후속 발사 실패 정책, MuzzleBlocked, Ejection, Carrier Velocity와 기본값 복구로 정정했다.

### v1.61 - 2026-07-30

- 사용자 PIE에서 Salvo 설정의 다중 발사가 정상적으로 진행되고 사출 직후 동일 차량 Projectile끼리 상호 Impact되는 기존 결함이 재현되지 않음을 확인했다.
- Projectile의 차량·월드 대상 일반 충돌 동작도 유지됨을 확인했다.
- 다른 차량 Projectile·Hitscan 요격은 현재 플레이어 외 사격 주체가 없어 미검증으로 남기고, AI 전투 주체 또는 별도 적 사격 환경이 준비되는 단계로 Deferred했다.
- 요격 미검증을 현재 런처 통합 PIE의 즉시 차단 조건에서 제외하고 다음 실행을 ContinueRemaining·취소와 Ejection·Carrier Velocity 검증으로 이동했다.
- `CF-TC-026`은 나머지 런처 검증이 남아 있으므로 PARTIAL을 유지한다.

### v1.60 - 2026-07-30

- 사용자 확정에 따라 같은 차량이 발사한 모든 탄종과 모든 Volley를 하나의 Projectile 충돌 격리 범위로 고정했다.
- 서로 다른 차량의 Projectile은 기본 Block을 유지하고, `bCanBeIntercepted=true`인 Projectile은 적 Projectile·Hitscan 유효 적중 한 번으로 Intercepted 처리하도록 구현했다.
- 이전 v1.10.0의 전역 Projectile Ignore와 전역 Impact Guard는 폐기하고 차량별 양방향 Ignore 등록·Pool 반환 정리로 교체했다.
- `ACFProjectileActor v1.11.0`, `UCFProjectilePoolComp v1.5.0`, `UCFProjectileData v1.9.0`, `ACFVehiclePawn v2.128.0`과 `CFProjectileCollisionTests v2.0.0`을 적용했다.
- 첫 빌드 Job `363ccb2d040a45dbb4e980c49631310b`의 FVector 형식 오류를 보정하고 최종 Job `940272869b77450797347f76b427faf3`에서 Exit Code 0을 확인했다.
- CF-TC-026은 PARTIAL을 유지하고 다음 사용자 PIE를 동일 차량 격리·다른 차량 요격 검증으로 전환했다.

### v1.59 - 2026-07-30

- 사용자 PIE에서 Salvo 런처가 한 입력에 네 Muzzle의 Projectile을 동시 사출하는 것 자체는 정상임을 확인했다.
- 네 Projectile이 사출 직후 서로 Blocking Hit을 발생시켜 Impact 처리되는 결함을 확인했다.
- 원인은 Projectile CollisionComponent가 Projectile Object Type을 사용하면서 자기 Projectile 채널까지 Block하던 응답표였다.
- `ACFProjectileActor v1.10.0`에서 Projectile 채널 Ignore와 Impact 단계 Projectile Actor·Object Channel Guard를 적용했다.
- `CFProjectileCollisionTests.cpp v1.0.0`을 추가하고 공식 Editor Build Job `c96631773b2543b0b480e82da5d63e7c` / Exit Code 0을 확인했다.
- CF-TC-026은 PARTIAL을 유지하고 바로 다음 검증을 Salvo 상호 충돌 수정 재확인으로 제한했다.

### v1.58 - 2026-07-30

- 사용자 PIE에서 CF-TC-025의 플레이어 Launcher 진입, Direct Launch Handoff, 기존 Rocket 추진, 발사 후 런처 독립과 Pool 5 Volley 이상 재사용을 PASS했다.
- Damage는 현재 보강 미완료로 적용되지 않아 별도 차단 항목으로 기록하고 런처 실패로 판정하지 않았다.
- CF-TC-026의 Direct Ripple 4발·0.15초, Muzzle 1→4→2→3, 고정 Command Target, 입력 중복 방지와 SequenceCompleted 쿨다운을 PASS했다.
- SingleCycle 입력당 1발, Muzzle 순환 1→4→2→3→1, 거부 시 Projectile·FX 없음과 Muzzle 인덱스 유지를 PASS했다.
- CF-TC-025와 CF-TC-026은 전체 범위 미완료로 PARTIAL을 유지하고 다음 사용자 PIE를 Salvo로 이동했다.

### v1.57 - 2026-07-30

- 사용자 PIE에서 `CF-TC-023`의 Trail-only, Thruster-only, Trail+Thruster 전체 조합을 PASS했다.
- 유효 소켓·Missing Socket Fallback, Hit·LifeExpired Reset, 20발 이상 Pool 재사용과 Ribbon History 무잔류를 PASS했다.
- 30 FPS 고속 Bounds·컬링과 기존 Impact·Damage·Pool 회귀를 PASS했다.
- `CF-FQ-027`을 Paused에서 Done으로 전환하고 `CF-TC-023 PASS`와 Projectile Systems Current를 연결했다.
- 현재 단일 Active `CF-FQ-029 / LM-P0-06`과 Ready·Deferred 작업 상태는 변경하지 않았다.
- 2026-07-30 당시 체크포인트에서는 작업 범위 안의 승인된 실행 경로를 준비하지 않아 Automation 소스 컴파일 PASS / 실행 Not Run 상태로 기록했다.

### v1.56 - 2026-07-29

- 사용자 요청에 따라 `CF-FQ-030 MG-P0-00`의 비활성 Foundation만 선행 구현했다.
- Missile Flight·Guidance 타입, Config·Snapshot, 제한형 비례항법 순수 수학과 Foundation Automation 소스를 추가했다.
- `UCFProjectileData v1.8.0`에 기본 비활성 `MissileFlightConfig`와 `MissileGuideConfig`를 추가했다.
- Editor Build Job `8c4f952fa58f499b9145b95caa1ff926`에서 UHT·신규 수학·테스트·ProjectileData 컴파일·링크와 Exit Code 0을 확인했다.
- `ACFProjectileActor`, `UCFProjectileMotorComp`, Launcher, TargetSelect와 `.uasset`은 변경하지 않았고 Runtime Not Connected 상태를 유지했다.
- 현재 단일 Active는 계속 `CF-FQ-029 / LM-P0-06`이며 `CF-FQ-030`은 Ready, `MG-P0-01`은 사용자 PIE 선행 상태로 유지했다.

### v1.55 - 2026-07-29

- `LM-P0-05 Launcher Editor Assets`를 Assets Applied / Independent AssetDump PASS로 종료했다.
- `DA_RocketBody`, `DA_RocketLauncher`, `RocketLauncher` Preset, `DA_TestSUV`, `TestMap`과 `BP_CFVehiclePawn` CDO의 실제 플레이 진입 경로를 연결했다.
- LauncherPost 10/10, VehiclePost 3/3, MapPost 19/19와 신규 Unreal 프로세스 CDO 재검증을 기록했다.
- `LM-P0-06 Launcher Integration PIE`와 `CF-TC-025·026`을 현재 사용자 검증 단계로 이동했다.
- 사용자 PIE 전에는 CF-FQ-029를 Done 또는 Systems Current로 판정하지 않는다.

### v1.54 - 2026-07-29

- `LM-P0-04 Angled·Vertical Ejection`의 데이터·수학 계약과 Pawn Runtime 연결을 적용했다.
- `FCFLauncherReleaseConfig`, WeaponData·WeaponComp Release Getter, Release-aware Launch Context와 실제 사출 방향 안전 검사를 연결했다.
- Direct 기본값은 기존 AimDirection·ProjectileData.InitialSpeed·CarrierVelocityRatio 0 결과를 유지한다.
- `CFLauncherReleaseTests.cpp`를 추가하고 공식 Editor Build Job `acd575ffac2f4d64bd73194e160a3f62`에서 Exit Code 0을 확인했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run, Editor PIE는 Pending으로 유지했다.
- 현재 단계를 `LM-P0-05 Launcher Editor Assets`로 이동했다.

### v1.52 - 2026-07-28

- `LM-P0-01 Projectile Launch Handoff Foundation`의 C++ 구현을 적용했다.
- `CFProjectileLaunchTypes.h`와 `CFProjectileLaunchTests.cpp`를 추가하고 Pawn → Pool → Projectile Actor Context 전달을 연결했다.
- 기존 `ActivateProjectile`과 `AcquireProjectile`은 Direct Launch Context Adapter로 유지해 기존 InitialSpeed 직사 결과를 보존했다.
- 공식 Editor Build Job `056397a4b43d49c9845c9cc296b23b9c`에서 UHT·신규 테스트 컴파일·링크와 Exit Code 0을 확인했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run으로 분리하고 사용자 PIE를 Pending으로 유지했다.
- Multi-Muzzle, Ripple·Salvo, Angled·Vertical Ejection과 Guidance는 구현하지 않았다.

### v1.51 - 2026-07-28

- 사용자 결정에 따라 `CF-FQ-019 주행·전투 반복 테스트`를 가까운 후속 Candidate에서 `Deferred`로 이동했다.
- `CF-FQ-019`는 취소하지 않으며 `CF-FQ-029`와 `CF-FQ-030`의 구현 범위가 확정된 뒤 런처·미사일을 포함한 통합 회귀로 다시 설계한다.
- 현재 Active `CF-FQ-029`와 선행 대기 `CF-FQ-030`의 상태는 변경하지 않았다.
- 소스, 에셋, 빌드와 PIE 상태는 변경하지 않았다.

### v1.50 - 2026-07-28

- 사용자 요청에 따라 `CF-FQ-029 모듈형 런처 및 발사 인계`를 현재 단일 Active 작업으로 등록했다.
- `CF-FQ-030 물리 제한형 미사일 비행·유도`를 CF-FQ-029 Launch Handoff 선행의 Ready 작업으로 분리했다.
- LauncherMissilePlan, Design, Roadmap, ModularLauncherPlan, MissileGuidancePlan, LaunchTaskSource와 LaunchWorkOrder를 생성했다.
- Direct·Angled·Vertical 사출, ProjectileMovement 기반 Ejection, 발사 후 런처 독립과 유도 명중 비보장 계약을 확정했다.
- LM-P0-00을 Done, LM-P0-01을 Code Entry Ready로 기록하고 Source Changes / Build / Automation / PIE를 아직 미실행 상태로 구분했다.
- `CF-FQ-019`, `CF-FQ-027`과 `CF-FQ-026`의 기존 상태를 유지했다.

### v1.49 - 2026-07-28

- `CF-FQ-027` 보존 체크포인트를 실제 현재 상태에 맞게 갱신했다.
- 지속형 FX 런타임과 독립 FX Scale은 `Document/Systems/Combat/Projectile.md v1.4.0`에 반영됐음을 기록했다.
- `DA_PFX_ThrusterTest`의 Rocket Thruster·FX_Exhaust·Scale 0.2 사용자 PIE PASS를 완료 유지 항목으로 추가했다.
- 별도 `CF-TC-023` 전체 검증은 Trail-only·동시 FX·Missing Socket·Pool 10회 이상 등이 남아 있어 Pending으로 유지했다.
- `CF-FQ-027`을 Paused로 유지하고 사용자가 명시적으로 선택할 때 미완료 전체 행렬부터 재개하도록 정정했다.
- `CF-FQ-028 Done`, 현재 Active 없음과 `CF-FQ-019 Candidate / Not Started` 상태는 변경하지 않았다.

### v1.48 - 2026-07-28

- `CF-FQ-028 발사체 추진 시스템`을 `Document/Systems/Combat/Projectile.md v1.4.0` Current System으로 승격했다.
- `CF-FQ-028`을 Done / User PIE PASS / `CF-TC-024 PASS`로 종료했다.
- `ProjectilePropulsionPlan.md v1.2.0`을 완료 이력 문서로 전환했다.
- 현재 자동 선택된 Active 작업을 비웠다.
- `CF-FQ-019 주행·전투 반복 테스트`를 다음 Candidate / Not Started로 유지하고 자동 착수하지 않았다.
- `CF-FQ-027`은 런타임 기반이 Projectile Current System에 반영됐지만 별도 전체 체크리스트와 `CF-TC-023` 미완료로 Paused 상태를 유지했다.
- `CF-FQ-026`의 `TS-P0-08` Paused 체크포인트를 변경하지 않았다.

### v1.47 - 2026-07-28

- `DA_PFX_ThrusterTest` 사용자 PIE에서 추진과 Thruster FX 기본 동작은 정상이고 Scale만 반영되지 않는 결함을 확인했다.
- 유효 `FX_Exhaust` 소켓 경로가 FX Origin Scale을 `1,1,1`로 덮어쓰던 원인을 보정했다.
- `UCFProjectileData v1.7.1`, `ACFProjectileActor v1.8.1`, `CFProjectileFlightFxTests.cpp v1.1.0`으로 갱신했다.
- 최종 Build Job `e8b812bd479549299dd116f9bae8996f`에서 UHT·컴파일·링크와 Exit Code 0을 확인했다.
- 사용자 재검증에서 `RelativeTransform.Scale=0.2` 적용 시 추진 화염이 정상 축소됨을 확인했다.
- `PP-P0-08`, `PP-P0-09`를 Done으로 전환하고 `CF-TC-024 PASS`를 등록했다.
- 다음 단계를 `PP-P0-10 Systems 승격과 반복 전투 회귀`로 이동했다.

### v1.46 - 2026-07-27

- 사용자 확정에 따라 차량 부스터가 아닌 로켓·미사일용 실제 발사체 추진 기능을 `CF-FQ-028`로 등록하고 단일 Active로 전환했다.
- `CFProjectileMotorTypes v1.0.0`, `UCFProjectileMotorComp v1.0.0`, `UCFProjectileData v1.7.0`, `ACFProjectileActor v1.8.0`과 RuntimeContract Automation 소스를 적용했다.
- InitialSpeed 분리, 점화 지연, 고정 발사 방향 가속, 최대 추진 속도, BurnedOut 관성·중력 비행과 Pool Reset 계약을 구현했다.
- Thruster Niagara를 Projectile 전체 활성 시간이 아니라 실제 Motor `Burning` 상태에만 동기화했다.
- 첫 Build Job `984f0ccab94241148e7b40b9d30a5572`의 Automation double-float 비교 오버로드 오류를 수정하고 최종 형식 정리 후 Build Job `2ffd09357e654bb7970a2e379f5ab45f`에서 Exit Code 0을 확인했다.
- `CF-FQ-027`은 C++ Foundation을 유지한 채 Editor 후보 연결 단계에서 Paused로 보존했으며, 다음 실행을 테스트 Rocket ProjectileData와 사용자 PIE로 이동했다.

### v1.45 - 2026-07-27

- PFX-P0-05 자산 조사에서 RocketThrusterExhaustFX 18/18, Stylized Attacks Niagara 14/14와 DA_HeavyShell 1/1 AssetDump 성공을 확인했다.
- Trail 기술 1차 후보를 `NS_RibbonTrail`, Thruster 1차 후보를 `NS_RocketExhaust_Realistic`으로 좁혔다.
- 현재 AssetDump가 Niagara Loop·Local Space·Spawn Per Unit·Bounds와 DataAsset 실제 저장값을 노출하지 않아 시각 승인과 .uasset 저장은 Editor Pending으로 유지했다.
- DA_HeavyShell Trail-only와 테스트용 `DA_PFX_ThrusterTest`, `DA_PFX_BothTest`, FX_Trail·FX_Exhaust 소켓/Fallback 설정 절차를 대표 Plan에 고정했다.

### v1.44 - 2026-07-27

- CF-FQ-027의 PFX-P0-01~03 C++ 구현을 현재 AI 세션이 직접 적용했다.
- `UCFProjectileData v1.6.0`에 비행 FX 공용 설정과 Trail·Thruster 슬롯을 추가했다.
- `ACFProjectileActor v1.7.0`에 두 Origin/Niagara 컴포넌트, 소켓 Fallback과 Pool 반환 전 Reset을 구현했다.
- `CFProjectileFlightFxTests.cpp v1.0.0` RuntimeContract 자동화 소스를 추가했다.
- 최초 Build Job `604e6b11db87486fa37cc09b14907e18`의 NiagaraComponent include 오류를 보정했다.
- Blueprint 툴팁 정렬 후 최종 Build Job `445848ab0f7749fcab8188b164bb1487`에서 UHT·신규 테스트 컴파일·링크와 Exit Code 0을 확인했다.
- 2026-07-27 당시 체크포인트에서는 작업 범위 안의 승인된 Unreal Automation 실행 경로를 준비하지 않아 자동화 실행을 Not Run으로 기록하고, 다음 초점을 PFX-P0-05 Editor 자산 연결로 이동했다.

### v1.43 - 2026-07-27

- CarFight 코드 작업 기본 방침을 `Browser AI Direct`로 변경했다.
- TaskSource와 WorkOrder를 구현 참고 자료로 유지하고 최종 Codex YAML을 착수 조건에서 제거했다.
- `CF-FQ-027`의 코드 게이트를 `Ready — Direct Implementation`으로 전환했다.
- 다음 실행을 `PFX-P0-01 ProjectileData 계약 직접 구현`으로 확정했다.
- 작업 의존 관계에 남아 있던 최종 품질 게이트 대기 문구를 직접 구현 기준으로 정정했다.

### v1.42 - 2026-07-27

- 사용자 선택에 따라 `CF-FQ-027 투사체 비행 FX`를 단일 Active 작업으로 등록했다.
- `PFX-P0-00`에서 현재 Projectile·CombatFx 코드와 프로젝트 결정을 복원하고 지속형 Trail·추진 FX 구조를 확정했다.
- `ProjectileFlightFxPlan.md`, `ProjectileFlightFxRoadmap.md`, `ProjectileFxTaskSource.md`, `ProjectileFxWorkOrder.md`를 대표 준비 문서로 연결했다.
- `ACFProjectileActor` 소유, `FX_Trail`·`FX_Exhaust` 소켓 우선, ProjectileData Fallback과 Pool 반환 전 Reset을 구현 계약으로 고정했다.
- 현재 Admin 표면에 `plan.*` 품질·증거 게이트가 없어 최종 Codex YAML은 Missing이며 실제 소스·에셋은 변경하지 않았음을 기록했다.
- `CF-FQ-026`은 TS-P0-08 Paused, `CF-FQ-019`는 후속 Candidate 상태를 유지했다.

### v1.41 - 2026-07-27

- 최종 사용자 PIE에서 Muzzle 정상 발사 1회, 발사 거부 0회, Impact 위치·1회성·중복·잔류 없음, Destroyed 소켓 위치·최초 1회·추가 피해 중복 없음과 기존 전투 회귀가 모두 PASS했다.
- `CF-FQ-024`를 `Done / User PIE PASS`로 전환하고 `CF-TC-021 PASS`를 등록했다.
- 현재 구현 기준을 `Document/Systems/Combat/CombatFx.md`로 승격했다.
- 활성 작업을 비우고 `CF-FQ-019`를 착수 가능한 다음 Candidate로 표시하되 자동 Active 전환하지 않았다.
- `CF-FQ-026`은 `TS-P0-08 Paused` 상태를 그대로 유지했다.

### v1.40 - 2026-07-27

- Impact P0 Niagara를 `NS_BasicHit`으로 사용자 확정하고 외부 Transform Scale 미반응을 허용해 추가 튜닝을 종료했다.
- `NS_Impact_1`, Adapted Niagara와 공용 User Scale 연결 작업은 P0에서 재개하지 않도록 고정했다.
- `UCFVehicleData.DestroyedFxSocketName=FX_Destroyed`와 `SM_Body` 소켓 우선 파괴 위치 구조를 반영했다.
- Sedan 차체 메시의 `FX_Destroyed` 소켓 적용 후 폭발 위치 해결 사용자 결과를 기록했다.
- 사용자의 직접 Editor 빌드 PASS를 기록하되 Admin Build Job ID가 없는 사용자 검증으로 구분했다.
- 현재 단계를 후보 튜닝에서 최종 통합 PIE 단일 게이트로 이동했다.

### v1.39 - 2026-07-25

- `ACFCombatFxPreviewActor`를 v1.1.0으로 확장해 Editor Viewport 자동 반복, Uniform Scale, 최대 수명 자동 정지와 Override Niagara DataAsset 적용을 추가했다.
- Build Job `89b6d82710af47e89a4c5b3823037d4e`에서 UHT·컴파일·링크가 성공하고 Exit Code 0임을 확인했다.
- 현재 Niagara 팩의 Explosion 명명 후보가 `NS_Explossion`과 `NS_AOE_Explosion_1` 두 개뿐임을 확인했다.
- `NS_Explossion`은 계속 제외하고 `NS_AOE_Explosion_1`은 수동 Loop·세미리얼 검토 대상으로 유지했다.
- SmokeBuilder는 NiagaraSystem 0개, Cascade ParticleSystem 64개이며 현재 Niagara 전용 P0 계약에는 채택하지 않았다.
- 현재 AssetDump 프로필로는 DataAsset 프로퍼티 참조값을 확인할 수 없어 기존 FX 연결은 Editor 확인이 필요함을 기록했다.

### v1.38 - 2026-07-24

- `ACFCombatFxPreviewActor` EditorOnly 도구를 추가해 PIE 없이 Niagara Scale·Rotation·Location을 반복 조정할 수 있도록 했다.
- `UCFCombatFxData.MaximumLifetimeSeconds`와 런타임 강제 제거 안전 퓨즈를 추가했다.
- Build Job `c9682d4a0be24ead95e87a5a7e8b5849`에서 UHT와 CarFight_ReEditor Win64 Development 빌드가 Exit Code 0으로 성공했다.
- `/Game/CarFight/FX/Data`의 CombatFxData 3개가 이미 존재하며 AssetDump 3/3 성공임을 반영했다.
- 현재 기준 VehicleData를 `DA_TestSedan`으로 정정하고 `DA_PoliceCar`를 새 FX 연결 대상에서 제외했다.
- `NS_Explossion`을 Loop 부적합 Destroyed 후보로 제외하고 비Loop 대체 후보 확정을 다음 작업으로 지정했다.

### v1.37 - 2026-07-24

- `UCFCombatFxData`와 `UCFCombatFxComp`를 추가하고 Niagara 모듈 의존성을 연결했다.
- 승인된 발사, HitScan 첫 Impact, Projectile 첫 Impact와 최초 차량 파괴 이벤트를 데이터 기반 일회성 FX 요청으로 연결했다.
- `UCFWeaponData.DefaultFireFxData`, `UCFProjectileData.DefaultImpactFxData`, `UCFVehicleData.DefaultDestroyedFxData`를 추가했다.
- FX 데이터 또는 Niagara가 비어 있어도 기존 발사·충돌·피해·파괴 판정을 유지하도록 null 안전 경로를 고정했다.
- Build Job `bd5a50bf388049ec84123aec4942ae96`에서 UHT와 CarFight_ReEditor Win64 Development 빌드가 Exit Code 0으로 성공했다.
- 현재 초점을 CombatFxData 3개 생성, 기존 DataAsset 연결과 사용자 PIE 시각 검토로 이동했다.

### v1.36 - 2026-07-24

- `/Game/RocketThrusterExhaustFX` NiagaraSystem 18개와 `/Game/sA_Megapack_v1` NiagaraSystem 96개를 AssetDump로 확인했다.
- 두 팩 합계 114개가 모두 Dump 성공하고 실패 0개임을 기록했다.
- Muzzle, Impact, Explosion, Bullet/Rocket Trail과 Rocket Exhaust 이름 기반 예비 후보를 추출했다.
- 현재 단계를 자산 반입 대기에서 `Asset Inventory Complete / Visual Review Pending`으로 이동했다.
- 자산 이름만으로 최종 미술 선택을 확정하지 않고 Unreal Editor 시각 검토를 다음 작업으로 지정했다.

### v1.35 - 2026-07-24

- 사용자 우선순위 결정에 따라 `CF-FQ-026`을 TS-P0-08 체크포인트에서 Paused로 전환했다.
- 최신 TargetSelect PIE 결함인 후보 텍스트 겹침, 후보 범위 과대와 디버그 원 비가시를 재개 항목으로 보존했다.
- `CF-FQ-024 전투 FX`를 공식 단일 활성 작업으로 전환했다.
- 현재 FX 단계를 `Phase 0 FAB FX 팩 1개 반입 → AssetDump → Muzzle·Impact·Destroyed 후보 확정`으로 지정했다.
- TargetSelect의 TS-P0-00~07 완료 상태와 전체 Automation 7/7 PASS 증거는 유지한다.

### v1.34 - 2026-07-24

- TS-P0-08 사용자 PIE에서 정지 상태 동일 차량 인스턴스의 후보 대표 위치 불일치를 확인했다.
- `UCFTargetPointComp`에 지정 PrimitiveComponent Bounds 중심 자동 정렬과 로컬 오프셋을 추가했다.
- 차량 TargetPoint를 `SM_Body` 기준으로 활성화하고 차체 메시 적용 후 정렬하도록 연결했다.
- Build Job `0fce6d253d9548dfa0ed39c94501ff47`에서 Editor 빌드와 TargetSelect 전체 7개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- 현재 다음 단계를 사용자 PIE 위치·범위 재검증으로 갱신했다.

### v1.33 - 2026-07-24

- `CFTargetUseTypes.h`에 장비 타겟 사용 정책, 평가 요청, 단계별 결과와 11개 실패 사유 enum을 추가했다.
- `UCFTargetSelectComp`에 선택 상태를 변경하지 않는 `EvaluateSelectedTargetForUse`와 디버그 요약 API를 추가했다.
- 평가 결과는 장비 준비, 선택 존재, 선택 유효성, 대상 호환성, 거리 충족과 최종 사용 가능 여부를 각각 보존한다.
- `UCFWeaponData`에 대상 분류·관계·필수·제외 태그·추적 상태를 지정하는 `TargetUsePolicy`를 추가했으며 빈 정책은 기존 에셋 호환을 위해 제한 없음으로 해석한다.
- `UCFVehicleWeaponComp`를 첫 장비 소비 예제로 연결해 선택 변경·해제·유효성·추적 상태 이벤트와 0.10초 저빈도 Tick으로 활성 무기 평가 캐시를 갱신하고, 사거리 진입·이탈처럼 실제 사용 상태가 바뀔 때 변경 이벤트를 발생시키도록 했다.
- 장비 평가 실패는 공용 선택을 해제하거나 별도 획득 상태를 만들지 않으며 FireOrigin과 직접 조준 발사 방향을 변경하지 않는다.
- 최종 Build Job `b7c2eb766e984c8ea6f5d3d847cee1f6`에서 UHT와 Editor 빌드 성공, 이동 중 사거리 이탈·복귀를 포함한 TS-P0-01~07 전체 7개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- TS-P0-07을 Done으로 전환하고 현재 작업 초점을 TS-P0-08 P0 통합 검증과 튜닝으로 이동했다.

### v1.32 - 2026-07-24

- `UCFTargetSelectWidget` C++ 부모와 `/Game/CarFight/UI/WBP_TargetSelect` Blueprint 위젯을 추가했다.
- C++는 TargetSelectComp 이벤트 구독, 후보·선택 상태 캐시, 월드 위치 화면 투영과 거리·관계·추적 상태 표시를 담당하고 Blueprint는 8개 위젯 배치와 스타일을 담당하도록 분리했다.
- 후보 `◇`, 선택 `▣`, 가림 선택 `▧` 형태를 사용해 색상 없이도 후보·선택·가림 상태를 구분하도록 했다.
- 같은 Actor가 후보와 선택인 경우 후보 마커를 숨기고, 화면 밖에서는 마커만 숨기며 선택 상태를 유지하도록 했다.
- `ACFVehiclePawn`에 TargetSelect HUD 클래스·인스턴스·표시 토글·ZOrder 20과 BeginPlay·Input 준비·EndPlay 수명을 연결했다.
- 최종 Build Job `26ef764a7de94b9ab46b221fcbc47805`에서 Editor 빌드와 TS-P0-01~06 전체 6개 Automation이 성공하고 실패 0, 경고 0, 오류 0, Exit Code 0임을 확인했다.
- UI AssetDump에서 WidgetBlueprint 7/7 성공과 WBP_TargetSelect의 C++ 부모 및 8개 위젯 트리를 확인했다.
- TS-P0-06을 Done으로 전환하고 현재 작업 초점을 TS-P0-07 장비 조회 연동으로 이동했다.

### v1.31 - 2026-07-24

- `ACFVehiclePawn`에 `InputAction_SelectTarget`, `InputAction_ClearTarget`, 현재 후보 선택 확정과 Manual 선택 해제 API를 추가했다.
- 선택 입력은 현재 후보가 있을 때만 선택을 변경하고, 후보가 없으면 기존 선택을 유지하도록 연결했다.
- 해제 입력은 선택만 `Manual` 사유로 해제하고 현재 후보 유지와 자동 다음 타겟 금지 정책을 보존했다.
- `/Game/CarFight/Input/IA_SelectTarget`, `/Game/CarFight/Input/IA_ClearTarget`을 생성하고 `IMC_Vehicle_Default`에 가운데 마우스·오른쪽 스틱 클릭 선택, 오른쪽 마우스·게임패드 FaceButton Right 해제를 저장했다.
- Build Job `efdc298d3108451ca60a9906a7612333`에서 Editor 빌드와 TS-P0-01~05 전체 5개 Automation이 성공하고 실패 0, 경고 0, 오류 0, Exit Code 0임을 확인했다.
- Input 폴더 AssetDump에서 15개 에셋 전체 성공과 신규 Action·Mapping Context 참조를 확인했다.
- TS-P0-05를 Done으로 전환하고 현재 작업 초점을 TS-P0-06 후보 및 선택 HUD로 이동했다.

### v1.30 - 2026-07-24

- Automation 월드에서 BeginPlay 이전 Actor의 `Destroy()`가 `OnEndPlay`만으로 즉시 전달되지 않는 경계를 확인했다.
- 선택 대상에 `AActor.OnDestroyed`를 추가 구독하고 `OnEndPlay`는 스트리밍·레벨 제거 경로로 유지했다.
- Build Job `bb63b77b7a174afda89762fd0d2d712c`에서 UHT와 `CarFight_ReEditor Win64 Development` 컴파일 성공을 확인했다.
- `TS-P0-01 RuntimeContract`, `TS-P0-02 TargetPoint`, `TS-P0-03 CandidateRanking`, `TS-P0-04 SelectionLifetime` 전체 4개 Automation이 성공하고 실패 0, 경고 0, 오류 0, Exit Code 0임을 확인했다.
- TS-P0-04를 Done으로 전환하고 현재 작업 초점을 TS-P0-05 입력과 플레이어 연결로 이동했다.

### v1.29 - 2026-07-24

- TS-P0-03의 TargetSelect 전용 Trace, 카메라 Aim 기반 후보 수집, 화면 근접도·월드 거리·안정 키의 결정적 정렬과 후보 전환 안정화를 구현 완료로 기록했다.
- Build Job `6b359cbce86449dea5cf35daacd15ed5`에서 `TS-P0-01~03` 전체 3개 Automation 성공과 Exit Code 0을 TS-P0-03 공식 완료 증거로 등록했다.
- TS-P0-04의 선택 대상 `OnEndPlay`·`VehicleHealth.OnVehicleDestroyed` 구독, 가림 유예, 추적 거리 이탈과 시스템 비활성화 해제 구현을 기록했다.
- Build Job `7204a1766a084306aedd3d274017b091`에서 UHT와 `CarFight_ReEditor Win64 Development` 컴파일은 성공했지만 전체 Automation은 4개 중 2 성공 / 2 실패, 경고 0 / 오류 7, 최종 Exit Code 255임을 확인했다.
- 실패 범위는 `TS-P0-01 RuntimeContract`와 `TS-P0-04 SelectionLifetime`의 Actor `Destroy()` 직후 자동 해제·해제 사유·이벤트 기대이며, `TS-P0-02 TargetPoint`와 `TS-P0-03 CandidateRanking`은 Success다.
- 현재 작업 초점을 TS-P0-04 선택 수명 Automation 수정과 TS-P0-01 회귀 복구로 이동했다.

### v1.28 - 2026-07-24

- 저장된 Build Job `c79b3418a3b2405b97ad52df7fa57bb0` 로그에서 `CarFight_ReEditor Win64 Development` 빌드 성공과 TargetSelect 전체 Automation Exit Code 0을 최종 확인.
- `UE/Saved/Automation/TargetSelect/index.json` 기준 전체 2개 테스트 성공, 실패 0, 경고 0, 오류 0을 확인.
- 최종 Blueprint 프로퍼티 메타데이터 정리 후 Build Job `a1fdf3bafd374f38affc4e14773f7915`에서 UHT 생성 파일 2개와 Editor 빌드 Exit Code 0을 추가 확인.
- one-shot 테스트 marker가 실행 전에 제거된 상태를 확인하고 TS-P0-02 완료 증거와 문서 날짜·Build Job ID를 실제 결과에 맞게 정정.
- 현재 작업 초점은 TS-P0-03 후보 탐색과 정렬을 유지.

### v1.27 - 2026-07-24

- `UCFTargetPointComp`와 위치 출처·해석 결과 구조를 추가하고 TargetPoint, Actor Bounds 중심, Actor 위치 순서의 공용 Fallback을 구현.
- `ACFVehiclePawn`에 기본 `TargetPoint` 서브오브젝트를 연결하고 기존 차량 호환을 위해 `bUseAsTargetPoint=false`를 기본값으로 유지.
- `CarFight.TargetSelect.TS_P0_02.TargetPoint` 자동화 테스트와 기존 TS-P0-01 회귀 테스트를 모두 PASS.
- 최종 Editor 빌드 `c79b3418a3b2405b97ad52df7fa57bb0` Exit Code 0을 기록하고 TS-P0-02를 Done으로 전환.
- 현재 작업 초점을 TS-P0-03 후보 탐색과 정렬로 이동.

### v1.26 - 2026-07-23

- Native C++ `ICFTargetSelectable` 구현은 직접 `_Implementation`을 호출하고 실제 Blueprint 재정의만 `Execute_` 경로로 전달하도록 안전 디스패치를 보강.
- `CarFight.TargetSelect.TS_P0_01.RuntimeContract`에서 입력 거부, 분류·관계·태그 필터, 후보·선택 이벤트 중복 억제, 유효성·추적 상태, DataAsset Fallback과 약한 참조 무효화를 자동 검증.
- `CarFight_ReEditor Win64 Development` 빌드와 Automation Test를 Exit Code 0으로 완료하고 TS-P0-01을 Done으로 전환.
- 현재 작업 초점을 TS-P0-02 타겟 포인트 기반으로 이동.

### v1.25 - 2026-07-23

- `ACFVehiclePawn`에 `TargetSelectComp` 기본 서브오브젝트와 `ICFTargetSelectable` 차량 기본 계약을 연결.
- Vehicle 분류, Unknown 관계, Identified 정보 단계, Bounds 중심 위치와 파괴 상태 기반 선택 가능·추적 상태를 추가.
- UnrealHeaderTool 생성 파일 갱신과 `CarFight_ReEditor Win64 Development` 빌드 PASS를 기록.
- 현재 초점을 Blueprint 에셋 노출과 상태 컴포넌트 런타임 계약 검증으로 전환.

### v1.24 - 2026-07-23

- `Document/Plan/TargetSelectInvestigation.md`를 TS-P0-00 공식 조사 산출물로 연결.
- 실제 Pawn, Camera Aim, Enhanced Input, Aim Reticle, VehicleHealth, 충돌과 장비 경계를 확인하고 TS-P0-00을 Done으로 전환.
- 현재 단계를 TS-P0-01 Blueprint 계약 검증과 `ACFVehiclePawn` 최소 통합 보정으로 갱신.
- 바로 다음 작업에서 후보 Trace, 입력, HUD를 섞지 않도록 범위를 고정.

### v1.23 - 2026-07-23

- 사용자 결정에 따라 `CF-FQ-026 타겟 선택 시스템`을 단일 공식 활성 작업으로 등록.
- 실제 코드와 빌드 상태를 기준으로 현재 단계를 `TS-P0-01 코드·Editor 빌드 PASS / Blueprint·PIE 검증 Pending`으로 기록.
- `CF-FQ-024 전투 FX 및 사운드 구현`은 취소하거나 Deferred로 내리지 않고 `Ready` 상태의 후속 작업으로 보존.
- 세션 복원 대표 체크포인트를 `Document/Plan/TargetSelectPlan.md`로 전환.

### v1.22 - 2026-07-22

- 마지막 작업 초점을 완료된 `CF-FQ-025`에서 현재 활성 작업 `CF-FQ-024`로 전환.
- `CombatFxAudio/ImplementationDesign.md`를 세션 복원 대표 체크포인트로 고정.
- `CF-FQ-025`는 최근 완료 작업과 별도 3D 착탄 위치 후속 논의로 보존.

### v1.21 - 2026-07-21

- `CF-FQ-025` 터렛 레티클 구현을 사용자 PIE PASS와 Done으로 전환.
- `CF-FQ-025`를 현재 활성 작업 목록에서 제거하고 `CF-FQ-024`를 기존 순서상 구현순위 1위로 복원.
- 다음 논의 주제를 Reticle과 분리된 투사체 착탄 위치 3D 표현으로 기록.

### v1.20 - 2026-07-21

- Weapon Aim Solution에 터렛 레티클 유효성, 월드 위치와 비교 거리를 추가.
- `Image_WeaponReticle`의 Legacy DirectImpact/LaunchDirection Preview 소비를 제거.
- 기존 Preview 필드는 Legacy Debug로 보존하고 Debug Panel에 신규 터렛 레티클 세 필드를 추가.
- `Tools\BuildEditor.bat` PASS, 다음 단계를 사용자 PIE 검증으로 전환.

### v1.19 - 2026-07-21

- `Image_CenterDot`의 현재 작동을 사용자 확인 PASS로 기록.
- 조준 레티클 입력 계약 확인을 다음 작업에서 제거하고 회귀 보호 대상으로 전환.
- 바로 다음 작업을 `Image_WeaponReticle`의 `CurrentMuzzleDirection` 기반 코드 정렬로 한정.

### v1.18 - 2026-07-21

- `Image_CenterDot`을 사용자의 화면 위치 지정용 조준 레티클로 확정.
- 터렛은 조준 레티클이 선택한 3D 지점을 추적하도록 책임 고정.
- `Image_WeaponReticle`을 탄종 독립 `CurrentMuzzleDirection` 기반 터렛 레티클로 확정.
- 투사체 착탄 위치를 Reticle UI와 분리된 후속 3D 표시로 이관.
- 다음 작업을 기존 DirectImpact/LaunchDirection Preview 소비 경로의 코드 정렬로 변경.

### v1.17 - 2026-07-20

- `Image_WeaponReticle` 사용자 PIE 화면 표시와 오른쪽 구석 좌표 문제 확인을 반영.
- Canvas Slot 앵커를 좌측 상단으로 고정하고 LaunchDirection을 Command 목표 깊이로 표시하도록 보정.
- Camera Aim Trace와 총구 Trace가 같은 Actor를 맞히면 목표 표면으로 허용해 MuzzleBlocked 오판을 보정.
- `Tools\BuildEditor.bat` 성공과 Return Code 0을 기록하고 사용자 PIE 재검증으로 전환.

### v1.16 - 2026-07-16

- Phase 2 C++ 화면 투영 구현 결과를 반영.
- `UCFAimReticleWidget`이 `WeaponPreviewWorldLocation`을 화면 좌표로 투영하고 Optional `Image_WeaponReticle`을 화면 안에서만 표시하도록 확장.
- `Tools\BuildEditor.bat` 성공과 Return Code 0을 기록.
- 현재 단계를 WBP 바인딩 및 사용자 PIE 확인 대기로 전환.

### v1.15 - 2026-07-16

- Heavy Cannon 사용자 PIE에서 MuzzleBlocked 회귀와 실제 중력탄 초기 발사 방향 일치가 정상임을 반영.
- `CF-FQ-025` LaunchDirection 단계를 코드, 빌드, PIE PASS로 전환.
- 바로 다음 작업을 Phase 2 World To Screen과 WBP Weapon Reticle 표시 계약 준비로 변경.

### v1.14 - 2026-07-16

- `CF-FQ-025` LaunchDirection Codex 실행 결과를 반영.
- `ECFWeaponReticleMode`와 `FCFVehicleWeaponAimSolution.WeaponReticleMode` 추가, 중력 Projectile의 LaunchDirection 데이터 제공, VehicleDebug Mode 행 추가를 완료 범위로 기록.
- `Tools\BuildEditor.bat` 성공과 Return Code 0을 기록.
- 현재 단계를 Codex 실행 대기에서 Heavy Cannon 사용자 PIE 대기로 전환.
- 사용자 PIE에서 Heavy Cannon 회전 시 LaunchDirection 모드, Preview Valid, Blocking Hit false, 거리 10000.0, 월드 위치 갱신을 부분 확인으로 기록.
- Phase 2 World To Screen과 WBP Weapon Reticle 표시는 사용자 PIE PASS 이후 진행하도록 유지.

### v1.13 - 2026-07-16

- 중력 Projectile의 Weapon Reticle을 `LaunchDirection`으로 표시하고 예상 탄착점은 후속 `BallisticImpact`로 분리하는 사용자 결정을 반영.
- `Hidden / DirectImpact / LaunchDirection` 모드 계약과 LaunchDirection의 비충돌·비탄착 의미를 확정.
- 상세 TaskSource `ReticleAimDirection/TaskSource_LaunchDirection.md`를 생성.
- 최종 Codex 입력 `ReticleAimDirection/Generated/Final/P1_LaunchDirection.md`를 생성.
- 현재 상태를 LaunchDirection 결정 대기에서 설계·Codex 계약 준비 완료와 Codex 실행 대기로 전환.
- 대표 Plan v0.2.4의 구현 계약 및 다음 검수 단계로 연결.

### v1.12 - 2026-07-16

- 사용자 PIE에서 실사용 Heavy Cannon의 Weapon Preview가 계속 무효로 표시된 결과를 반영.
- HeavyCannon이 중력 Projectile을 사용해 DirectImpact 전용 Phase 1 범위 밖임을 확인.
- 상태를 단순 사용자 PIE 대기에서 실사용 무기 범위 불일치와 LaunchDirection 결정 대기로 변경.
- 다음 작업을 중력탄의 초기 발사 방향 표시와 후속 BallisticImpact 분리 결정으로 전환.
- 대표 Plan v0.2.3의 에셋 조사와 차단 상태에 연결.

### v1.11 - 2026-07-16

- `CF-FQ-025` Phase 1 Codex 실행 결과와 브라우저 scoped diff 검수를 PASS로 반영.
- 독립 Editor 빌드 `build_79aa1777becd65e5` 성공과 Return Code 0을 기록.
- 현재 단계를 사용자 PIE 대기로 전환하고 HitScan·Projectile Preview 및 기존 발사 정책 회귀 확인을 다음 작업으로 지정.
- 대표 Plan v0.2.2의 검수 증거와 다음 단계로 연결.

### v1.10 - 2026-07-16

- `CF-FQ-025`의 현재 단계를 Phase 1 Codex 검토·보정 계약 준비 완료와 Codex 실행 대기로 갱신.
- 마지막 작업의 다음 실행 입력을 `ReticleAimDirection/Generated/Final/P1_WeaponPreviewReview.md`로 지정.
- Codex 실행 후 실제 scoped diff와 `Tools\BuildEditor.bat` 결과를 브라우저 AI가 검수하도록 다음 단계를 갱신.
- 세부 정책 위반 상태와 빌드·PIE 증거는 대표 Plan v0.2.1에서 관리하도록 연결.

### v1.9 - 2026-07-16

- `CF-FQ-025` 이중 레티클 및 사격방향 시각화를 P0 Active 구현순위 1위로 등록.
- 마지막 작업 초점과 세션 복원 대표 체크포인트를 `ReticleAimDirection/ImplementationDesign.md`로 전환.
- `CF-FQ-024`는 취소하지 않고 P0 Active 구현순위 2위로 유지.
- 진행 순서를 `CF-FQ-025 → CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021`로 갱신.

### v1.8 - 2026-07-15

- `CF-FQ-024` 전투 FX 및 사운드 구현을 새 활성 작업으로 등록.
- 발사·Impact·차량 파괴 연출의 대표 Plan을 `CombatFxAudio/ImplementationDesign.md`로 연결.
- 다음 진행 순서를 `CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021`로 갱신.

### v1.7 - 2026-07-15

- `CF-FQ-017`의 NoWeapon, AimBlocked와 정상 발사 회귀 사용자 PIE PASS를 반영.
- `CF-FQ-017`을 활성 작업에서 제거하고 현재 활성 작업 없음으로 전환.
- `CF-FQ-019`는 자동 착수하지 않고 Candidate 상태를 유지.

### v1.6 - 2026-07-15

- `CF-FQ-018` 사용자 PIE PASS와 Done 판정을 반영해 활성 작업 목록에서 제거.
- 마지막 작업 초점을 `CF-FQ-017`의 `NoWeapon` / `AimBlocked` 최종 검증으로 전환.
- `CF-FQ-019`가 Damage Runtime 완료 이후 착수 가능한 확장 회귀 후보임을 기록.

### v1.5 - 2026-07-14

- `CF-FQ-018` 최소 Damage Runtime 코드와 공식 Editor 빌드 완료 상태를 반영.
- 사용자 PIE 전이므로 기능은 Active로 유지하고 피해 누적·파괴 전환 검증을 다음 작업으로 지정.
- HitScan과 Projectile 공용 피해 적용, Projectile 첫 Impact 1회 적용과 Pool 결과 복사 구조를 복원 기준으로 기록.

### v1.4 - 2026-07-14

- `CF-FQ-022`와 `CF-FQ-023`의 Done / P0 사용자 PIE 완료 상태를 반영해 활성 작업 목록에서 제거.
- 마지막 작업 초점을 `CF-FQ-018 최소 Damage Runtime`으로 전환.
- `CF-FQ-018`의 선행 조건 완료와 바로 다음 작업을 DamageData·체력 소유 구조·피해 인터페이스 확정으로 기록.
- 완료된 Aim/Projectile 작업은 대표 Plan과 Systems에서 조회하고 ActiveWork에는 복사하지 않도록 정리.

### v1.3 - 2026-07-14

- CarFight ActiveWork의 관리 범위를 `main_game` 게임 기능으로 제한.
- 잘못 등록된 `CF-TOOL-ADUMP` 활성 작업과 AssetDump 마지막 작업 초점 및 의존 관계 제거.
- 마지막 작업 초점을 기존 CarFight 작업인 `CF-FQ-023`으로 복원.
- AssetDump와 GoPyMCP 작업은 각 독립 저장소의 `ActiveWork.md`로 진입하도록 경계 추가.

### v1.2 - 2026-07-14

- `CF-TOOL-ADUMP`을 현재 활성 도구 작업으로 등록하고 `AssetDumpPlan/README.md`에 연결했다.
- 이 등록은 독립 저장소 경계 원칙에 어긋나 v1.3에서 철회했다.

### v1.1 - 2026-07-14

- CF-FQ-022, 023, 018, 017 대표 Plan에 표준 `현재 작업 체크포인트`를 적용.
- CF-FQ-023 공식 Editor 빌드 성공 상태를 반영하고 다음 작업을 사용자 PIE 검증으로 변경.
- CF-FQ-018의 선행 조건을 코드 구현이 아니라 CF-FQ-022·023 사용자 PIE 검증으로 정정.

### v1.0 - 2026-07-14

- 다중 작업 세션 복원을 위한 ActiveWork 색인 최초 작성.
- CF-FQ-017, 018, 022, 023 활성 작업과 대표 Plan 연결.
- 마지막 작업 초점을 CF-FQ-023으로 기록.
- 짧은 세션 인계 및 복원 명령의 처리 규칙 추가.

---

## 9. Migration

### v1.75 적용 안내

- 새 세션은 과거 Runner Unavailable 또는 Not Run 문구를 현재의 영구적인 실행 불가 상태로 복원하지 않는다.
- `CF-FQ-027` Automation의 현재 증거는 `UE/Saved/Automation/CombatRuntime/index.json`과 `result.json`이며 `PFX_P0_01.RuntimeContract`는 Success다.
- 당시 사용한 작업 전용 임시 실행 경로를 공용 도구, Git 등록 대상 또는 다음 세션의 고정 재실행 경로로 가정하지 않는다.
- 재실행이 필요하면 현재 도구 표면, 저장소 상태, 작업 허용 범위와 보호 범위를 다시 확인한다.

### v1.51 적용 안내

- `CF-FQ-019`를 다음 착수 Candidate로 복원하지 않는다.
- `CF-FQ-019`는 `Deferred`이며 `CF-FQ-029`와 `CF-FQ-030`의 구현·검증 범위가 정리된 뒤 통합 회귀 범위를 다시 작성한다.
- 현재 코드 진입점은 계속 `LM-P0-01 Projectile Launch Handoff Foundation`이다.
- 기존 `CF-FQ-019` 관련 과거 Changelog와 Migration은 당시 이력으로만 읽는다.

### v1.48 적용 안내

- 새 세션에서 `CF-FQ-028`을 Active로 복원하지 않는다.
- 발사체 추진과 지속형 FX의 현재 구현 판단은 `Document/Systems/Combat/Projectile.md`를 우선한다.
- `ProjectilePropulsionPlan.md`는 완료 이력이며 현재 런타임 SSOT가 아니다.
- 현재 자동 선택된 Active 작업은 없다.
- `CF-FQ-019`는 다음 Candidate지만 사용자가 선택하기 전에는 착수하지 않는다.
- `CF-FQ-027`과 `CF-FQ-026`의 Paused 상태를 유지한다.

### v1.41 적용 안내

- `CF-FQ-024`의 현재 구현 판단은 `Document/Systems/Combat/CombatFx.md`를 우선한다.
- `Document/Plan/CombatFxAudio/ImplementationDesign.md`는 완료 당시 설계·튜닝·검증 기록으로 유지한다.
- `CF-TC-021`은 PASS이며 Muzzle·Impact·Destroyed의 P0 1회성 시각 연출을 완료 기준으로 사용한다.
- 현재 자동 선택된 Active 작업은 없으며 `CF-FQ-019`는 사용자가 선택할 때만 Active로 전환한다.
- `CF-FQ-026`의 TS-P0-08 체크포인트는 계속 Paused로 보존한다.

### v1.40 적용 안내

- 새 세션은 Impact P0 자산을 `NS_BasicHit`으로 복원하며 Scale 미반응 분석을 다시 열지 않는다.
- `DA_FX_ProtoShellImpact`는 `FxScale=1,1,1` 기준으로 두고 Niagara 내부 크기를 그대로 사용한다.
- Destroyed 위치는 각 차량 `SM_Body` StaticMesh의 `FX_Destroyed` 소켓으로 지정한다.
- 소켓이 없으면 `SM_Body` Bounds 중심 Fallback을 허용한다.
- 다음 작업은 후보 조사나 Preview 튜닝이 아니라 최종 통합 PIE 한 번이다.
- PIE PASS 전에는 `CF-FQ-024`를 Done 또는 Current System으로 승격하지 않는다.

### v1.39 적용 안내

- 새 세션은 `ACFCombatFxPreviewActor v1.1.0`과 Build Job `89b6d82710af47e89a4c5b3823037d4e` PASS 상태로 복원한다.
- Scale은 우선 `PreviewUniformScale` 하나로 조정하고 일회성 FX는 자동 반복으로 관찰한다.
- Override 후보 확정 시 DataAsset 적용 버튼으로 Niagara와 튜닝값을 함께 반영할 수 있다.
- `NS_AOE_Explosion_1`은 수동 확인 전까지 비Loop 또는 최종 Destroyed 후보로 해석하지 않는다.
- SmokeBuilder Cascade 자산을 위해 현재 Niagara 계약을 확장하지 않는다.
- 기존 FX 참조값은 Editor Details에서 확인하고 DataAsset 저장 후 최종 PIE를 진행한다.

### v1.38 적용 안내

- 새 세션에서 `ACFCombatFxPreviewActor`와 `MaximumLifetimeSeconds` 안전 퓨즈를 다시 구현하지 않는다.
- 공식 Preview Tool 빌드 증거는 Build Job `c9682d4a0be24ead95e87a5a7e8b5849`과 Exit Code 0이다.
- `/Game/CarFight/FX/Data`의 기존 CombatFxData 3개를 재생성하지 않는다.
- FX 튜닝은 Preview Actor에서 먼저 수행하고 최종값을 DataAsset에 적용·저장한 뒤 PIE로 발생 위치와 횟수만 검증한다.
- 현재 기준 VehicleData는 `DA_TestSedan`이며 `DA_PoliceCar`를 수정하지 않는다.
- `NS_Explossion`은 Loop 부적합 후보이며 비Loop Destroyed 후보를 별도로 선택한다.
- 사용자 PIE 전에는 CF-FQ-024를 Done 또는 Current System으로 승격하지 않는다.

### v1.37 적용 안내

- 새 세션에서 Combat FX C++ 기반을 다시 구현하지 않는다.
- 공식 빌드 증거는 Build Job `bd5a50bf388049ec84123aec4942ae96`과 Exit Code 0이다.
- 다음 작업은 `/Game/CarFight/FX/Data` 아래 CombatFxData 3개 생성과 기존 Weapon·Projectile·Vehicle DataAsset 연결이다.
- DataAsset 또는 Niagara가 비어 있는 상태는 전투 판정 실패가 아니라 FX 미연결 상태다.
- 사용자 PIE 전에는 CF-FQ-024를 Done 또는 Current System으로 승격하지 않는다.

### v1.36 적용 안내

- 새 세션에서 FAB 팩 추가와 Niagara 목록 조사를 반복하지 않는다.
- 현재 확인 수는 RocketThrusterExhaustFX 18개, sA_Megapack_v1 96개, 합계 114개다.
- 다음 작업은 예비 후보의 Unreal Editor 시각·공간·생명주기 검토다.
- 최종 후보 확정 전에는 Combat FX C++ 구현과 DataAsset 연결을 시작하지 않는다.

### v1.35 적용 안내

- 새 세션은 `CF-FQ-024`와 `Document/Plan/CombatFxAudio/ImplementationDesign.md`를 우선 복원한다.
- 첫 실행은 FAB FX 팩 1개가 프로젝트에 반입됐는지 확인하고, 반입됐다면 AssetDump로 Niagara 자산과 의존성을 조사한다.
- `CF-FQ-026`은 취소나 폐기가 아니라 TS-P0-08 Paused 상태이며 FX 작업 중 코드를 정리하거나 되돌리지 않는다.
- FX 구현에는 게임 사운드, Sound 자산 참조와 Audio 런타임을 포함하지 않는다.

### v1.33 적용 안내

- 선택 상태는 계속 `UCFTargetSelectComp`가 소유하며 무기·유틸리티 장비는 `FCFTargetUseRequest`로 읽기 전용 평가만 수행한다.
- 장비 사용 불가와 선택 대상 없음은 별도 상태다. `EquipmentUnavailable` 결과에서도 실제 `bHasSelectedTarget`과 `bSelectedTargetValid`을 유지한다.
- `TargetUsePolicy`의 허용 배열이 비어 있으면 제한 없음, `MaxUseDistanceCm <= 0`이면 거리 제한 없음으로 해석한다.
- 기존 WeaponData 에셋은 정책을 설정하지 않아도 유효한 선택 대상을 기존처럼 허용하므로 재저장이 필수는 아니다.
- WeaponComp의 대상 평가 이벤트는 UI·유틸리티 피드백에 사용할 수 있지만 TargetSelectComp 선택을 변경하거나 장비 획득 상태를 소유하지 않는다.
- 직접 조준 무기의 FireOrigin, AimSolution과 발사 방향은 선택 대상 평가와 독립적으로 유지한다.
- TS-P0-08에서는 실제 PIE로 입력부터 HUD·장비 평가·대상 파괴까지 통합 흐름을 확인한다.

### v1.32 적용 안내

- `WBP_TargetSelect`는 `WBP_AimReticle`과 별도 Viewport 위젯이며 Aim Reticle의 레티클·발사 피드백 책임을 변경하지 않는다.
- 기존 `BP_CFVehiclePawn`은 C++ 부모에서 `TargetSelectWidgetClass`, `bShowTargetSelectHud=true`, `TargetSelectHudZOrder=20`을 상속한다.
- 후보와 선택 위치는 TargetSelectComp와 공용 TargetPoint 결과를 사용하고 위젯 Blueprint에서 검색·선택 수명을 다시 계산하지 않는다.
- P0 화면 밖 대상은 마커를 숨기되 선택을 해제하지 않으며 방향 표시는 TS-P1-02 범위로 유지한다.
- TS-P0-07은 HUD를 수정하지 않고 선택 대상 조회와 장비 호환성 계약만 연결한다.
- 실제 화면 크기·울트라와이드·UI 스케일·고속 이동 감각은 TS-P0-08 통합 PIE에서 최종 확인한다.

### v1.31 적용 안내

- 기본 선택 입력은 `MiddleMouseButton`과 `Gamepad_RightThumbstick`, 기본 해제 입력은 `RightMouseButton`과 `Gamepad_FaceButton_Right`다.
- 기존 `BP_CFVehiclePawn`은 C++ 부모의 신규 Input Action 프로퍼티와 `SetupPlayerInputComponent` 바인딩을 상속하며 별도 Blueprint 이벤트 그래프 구현이 필요하지 않다.
- 후보가 없을 때 선택 입력은 기존 선택을 유지하며, 해제 입력은 현재 후보를 지우거나 자동 선택하지 않는다.
- TS-P0-05는 Done이므로 TS-P0-06에서 입력 처리나 선택 상태를 재구현하지 않고 TargetSelectComp 이벤트와 조회 API만 소비한다.
- 실사용 키 감각, UI 포커스, 재시작과 Possess 변경은 TS-P0-08 P0 통합 PIE에서 최종 확인한다.

### v1.30 적용 안내

- 선택 대상의 명시적 파괴는 `OnDestroyed`, 스트리밍·레벨 제거와 일반 수명 종료는 `OnEndPlay`, 차량 전투 파괴 상태는 `VehicleHealth.OnVehicleDestroyed`로 처리한다.
- 세 경로 중 먼저 도착한 이벤트가 선택을 해제하며 `bHasSelectedTarget` 검사로 후속 중복 이벤트를 무시한다.
- TS-P0-04는 Done이므로 후속 Task에서 선택 수명, 가림 유예와 자동 다음 타겟 금지 정책을 다시 구현하지 않는다.
- 다음 구현은 TS-P0-05 입력 연결에 한정하고 HUD와 장비 연동은 각각 TS-P0-06과 TS-P0-07 범위에 유지한다.

### v1.29 적용 안내

- TS-P0-03은 Done이며 공식 성공 증거는 Build Job `6b359cbce86449dea5cf35daacd15ed5`와 당시 전체 3개 TargetSelect Automation PASS다.
- Build Job `7204a1766a084306aedd3d274017b091`은 Editor 컴파일 성공과 Automation 실패를 분리해 해석한다. 통합 프리셋 최종 상태는 Exit Code 255이므로 TS-P0-04를 Done으로 처리하지 않는다.
- 현재 실패는 Actor `Destroy()` 호출 직후 같은 테스트 프레임에서 `OnEndPlay` 기반 자동 해제를 기대한 검증에 한정되며, 수정 전 TS-P0-05 입력 작업으로 넘어가지 않는다.
- 다음 작업은 기존 수명 구독 설계를 폐기하지 않고 EndPlay 전달 시점 또는 Automation 월드 진행 절차를 보정한 뒤 `CarFight.TargetSelect` 전체 회귀를 재실행한다.

### v1.28 적용 안내

- TS-P0-02의 공식 검증 증거는 Build Job `c79b3418a3b2405b97ad52df7fa57bb0`과 `UE/Saved/Automation/TargetSelect/index.json`이다.
- TS-P0-01과 TS-P0-02 자동화는 모두 PASS이므로 후속 Task에서 동일 계약과 Fallback을 다시 구현하지 않는다.
- 다음 구현은 TS-P0-03 후보 탐색과 정렬에 한정하며 입력과 HUD는 각각 TS-P0-05와 TS-P0-06 범위에 유지한다.

### v1.27 적용 안내

- 기존 차량은 상속된 `TargetPoint` 컴포넌트를 자동 보유하지만 `bUseAsTargetPoint=false`이므로 기존 Bounds 중심 선택 위치를 유지한다.
- 차량별 명시 위치가 필요할 때 Blueprint에서 `TargetPoint` 상대 위치를 조정하고 `bUseAsTargetPoint`를 활성화한다.
- 위치 해석 순서는 `TargetPoint → Actor Bounds 중심 → Actor Location`으로 고정한다.
- TS-P0-03은 이 공용 위치 결과를 사용하며 타겟 포인트 책임을 다시 구현하지 않는다.

### v1.26 적용 안내

- TS-P0-01은 기술 검증 완료 상태이며 동일 계약·필터·이벤트 테스트를 수동으로 반복하지 않는다.
- 사용자 PIE는 입력·후보 검색·HUD가 연결되는 후속 P0 통합 검증에서 수행하며 TS-P0-01 단독 종료 게이트로 유지하지 않는다.
- 다음 구현은 TS-P0-02의 타겟 포인트와 Fallback에 한정하고 후보 Trace, 입력과 HUD를 섞지 않는다.
- Native C++ 대상과 Blueprint 재정의 대상의 인터페이스 디스패치 경계를 유지한다.

### v1.25 적용 안내

- 기존 `BP_CFVehiclePawn`에는 `TargetSelectComp`를 수동 추가하지 않고 C++ 부모에서 상속된 기본 서브오브젝트를 사용한다.
- 차량은 기본적으로 `ICFTargetSelectable`을 구현하므로 Blueprint에 같은 인터페이스를 중복 추가하지 않는다.
- 현재 단계에서는 후보 Trace, 입력과 HUD가 없으며 Blueprint 에셋·런타임 계약 검증 완료 전 TS-P0-01을 Done으로 해석하지 않는다.

### v1.24 적용 안내

- 새 세션은 `TargetSelectInvestigation.md`의 TS-P0-00 확정 결정을 먼저 읽은 뒤 `TargetSelectPlan.md`의 TS-P0-01을 복원한다.
- 직접 선택 기준은 `VehicleCameraComp`의 카메라 Aim이며 `WeaponAimSolution`과 터렛 방향이 아니다.
- TS-P0-01은 Blueprint 계약 검증과 Pawn 최소 통합만 수행하고 후보 Trace, 입력 에셋과 HUD는 후속 Task에 유지한다.
- TS-P0-00은 Done이므로 같은 저장소 구조 조사를 처음부터 반복하지 않는다.

### v1.23 적용 안내

- 새 세션은 `CF-FQ-026 타겟 선택 시스템`과 `Document/Plan/TargetSelectPlan.md`를 우선 복원한다.
- `TS-P0-01`은 소스 작성과 Editor 빌드는 통과했지만 Blueprint 노출 및 사용자 PIE 확인 전이므로 Done으로 해석하지 않는다.
- 다음 작업은 신규 계약을 다시 작성하는 것이 아니라 TS-P0-00 조사 결과 복구와 TS-P0-01 검증 완료다.
- `CF-FQ-024`는 폐기된 작업이 아니며 타겟 선택 P0 진행 후 재개 가능한 Ready 작업으로 유지한다.

### v1.13 적용 안내

- 새 세션은 `CF-FQ-025`의 LaunchDirection 설계가 확정됐고 Codex 실행 계약까지 준비된 상태로 복원한다.
- 실행 입력은 `Document/Plan/ReticleAimDirection/Generated/Final/P1_LaunchDirection.md`다.
- 중력 Projectile은 LaunchDirection으로 실제 초기 발사 방향을 제공하되 Blocking Hit와 예상 탄착점을 의미하지 않는다.
- 예상 탄착점은 후속 Ballistic Solver의 BallisticImpact 단계로 유지한다.
- Codex 실행과 Browser scoped diff·Editor 빌드 검수 전에는 LaunchDirection 코드 구현 완료로 해석하지 않는다.
- 검수 후 사용자 PIE에서 Heavy Cannon의 Mode=LaunchDirection, Preview Valid=Yes, Blocking Hit=No를 확인한다.

### v1.12 적용 안내

- 새 세션은 `CF-FQ-025` DirectImpact 코드와 빌드는 검수됐지만 실사용 Heavy Cannon이 중력 Projectile이라 기능 검증이 차단된 상태로 복원한다.
- 현재 Heavy Cannon의 Preview 무효 결과를 단순 코드 고장으로 판단하거나 `DA_HeavyShell.bAffectedByGravity`를 임시 변경하지 않는다.
- Phase 2 UI 작업 전에 `LaunchDirection` 표시를 도입할지 확정한다.
- 권장 구조는 중력 Projectile에서 실제 초기 발사 방향을 Weapon Reticle로 표시하고 예상 탄착점은 후속 `BallisticImpact`로 분리하는 것이다.
- 사용자 결정 후 TaskSource와 Codex 계약을 새로 작성한다.

### v1.11 적용 안내

- 새 세션은 `CF-FQ-025` Phase 1 코드와 빌드가 검수 PASS이고 사용자 PIE만 Pending인 상태로 복원한다.
- Preview World 데이터는 코드에 존재하지만 사용자 PIE 완료 전까지 Phase 1 최종 완료로 해석하지 않는다.
- 다음 구현 TaskSource를 만들기 전에 HitScan, 중력 없는 Projectile, 중력 Projectile 무효, MuzzleBlocked와 정렬 정책 회귀를 확인한다.
- Phase 1 PIE PASS 후에만 Phase 2 Weapon Reticle C++ 투영 작업으로 진행한다.

### v1.10 적용 안내

- 새 세션은 `CF-FQ-025` 대표 Plan의 정책 위반 복구 체크포인트를 먼저 읽는다.
- Phase 1의 실행 입력은 `Document/Plan/ReticleAimDirection/Generated/Final/P1_WeaponPreviewReview.md`이며, 상세 요구사항은 연결된 `TaskSource_Phase1.md`를 기준으로 한다.
- Codex 실행과 브라우저 diff·빌드 검수가 끝나기 전에는 Phase 1 완료 또는 Current System으로 해석하지 않는다.
- 현재 연결에 Codex 실행 도구가 없으면 소스 직접 수정 예외를 자동 적용하지 않고 `Codex 실행 대기` 상태를 유지한다.

### v1.9 적용 안내

- 신규 CarFight 구현 세션은 `CF-FQ-025`와 `Document/Plan/ReticleAimDirection/ImplementationDesign.md`를 우선 복원한다.
- `CF-FQ-024`는 삭제하거나 보류 상태로 바꾸지 않고 구현순위 2위 Active 작업으로 유지한다.
- `CF-FQ-025` Phase 0~4와 사용자 PIE 완료 또는 사용자 명시적 전환 전에는 신규 구현 초점을 `CF-FQ-025`에 둔다.
- Weapon Reticle과 Weapon Preview는 구현 및 사용자 PIE 완료 전까지 Current System으로 해석하지 않는다.
- 현재 완료 구현 판단은 계속 관련 `Document/Systems/` 문서를 우선한다.

### 기존 적용 안내

- `CF-TOOL-ADUMP`과 `Document/Plan/AssetDumpPlan/`은 CarFight 활성 작업 체계에서 제거한다.
- AssetDump 상태와 릴리스 판정은 `UE/Plugins/ue-assetdump/Documents/`에서만 관리한다.
- GoPyMCP 상태와 내부 로드맵은 `GoPyMCP/Workspace/docs/`에서만 관리한다.
- CarFight가 도구의 공개 계약에 의존하는 경우 의존성만 CarFight 문서에 기록하고 도구 내부 진행 상태는 복사하지 않는다.
- 기존 FeatureQueue, Plan Index, Systems Index의 역할은 변경하지 않는다.
- 새 세션에서는 장문의 인계 프롬프트 대신 `이전 작업 이어서 진행해줘`를 사용할 수 있다.
- 세션 이동 전에는 `새 세션 인계 준비해줘`를 사용할 수 있다.
