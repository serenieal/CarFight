# CarFight — 01_ProjectState

> 역할: CarFight 프로젝트의 **현재 실제 기준선 / 임시 운영 편차 / 현재 리스크**를 고정한다.
> 공통 규칙 원본: `Document/SSOT/`
> 문서 버전: v2.43.42


> 마지막 정리(Asia/Seoul): 2026-08-18


---

## 먼저 확인할 방향
- 최종 방향은 `00_Vision.md`를 기준으로 본다.
- 이 문서는 그 방향 아래에서 **지금 실제로 굴러가는 상태만** 적는다.

### 현재 공식 Unreal Engine 기준선

```text
Engine Version: Unreal Engine 5.8
Engine Distribution: Source Build
Engine Root: D:\UnrealEngine_Source
Editor Build Entry: D:\Work\CarFight_git\Tools\BuildEditor.bat
Editor Run Entry: D:\Work\CarFight_git\Tools\RunEditor.bat
```

- CarFight의 엔진 호환성, API, 플러그인 지원 여부와 구현 판단은 별도 업그레이드 결정 전까지 **UE 5.8 Source Build**를 기준으로 한다.
- Launcher 설치형 엔진이나 과거 `D:\UE_5.7`, `D:\UE_5.7_Source` 경로는 현재 공식 엔진이 아니다.
- 과거 문서·대화에 남은 UE 5.7 표기는 Historical/폐기 기준이며 현재 엔진 버전이나 구현 근거로 승격하지 않는다.
- 실제 엔진 소스 수준 판단이 필요하면 `D:\UnrealEngine_Source`의 UE 5.8 소스를 최우선 근거로 사용한다.
- `Document/SSOT/UE_SSOT/`는 **UE 5.x 공통 원칙**만 제공하며, 정확한 엔진 버전·배포 형태·엔진 경로·버전 민감 API 판단에서는 이 ProjectSSOT의 **프로젝트별 Engine Baseline**이 우선한다.

### Engine Authority 구조

```text
현재
──────────────────────────────
ProjectSSOT
UE 5.8 Source Build
D:\UnrealEngine_Source
          ↑
          │ Current Authority
          │

과거
──────────────────────────────
EngineSourceBuild/
UE 5.7 Launcher → Source 전환
          ↓
Historical / Superseded
          ↓
Archive
```

- `Document/Plan/EngineSourceBuild/`는 UE 5.7 Launcher → Source Build 전환 당시의 기록이며 현재 Engine Baseline이 아니다.
- 해당 Plan의 과거 경로·명령·판정은 Historical evidence로만 보존하고, 현재 엔진 설정·빌드·플러그인 호환성·API 판단에 사용하지 않는다.
- 향후 엔진 버전이 변경되면 ProjectSSOT의 Engine Baseline을 먼저 갱신하고, 공용 UE SSOT는 특정 프로젝트의 마이너 버전을 자체적으로 고정하지 않는다.

---

## 2026-08-18 현재 프로젝트 상태


### 1. 현재 활성 작업

현재 단일 Active 작업은 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다. 사용자가 UI를 프로젝트 최우선 사항으로 재지정했다. 기존 UI-P0-02 Runtime Lifetime USER PASS와 UI-P0-03 Speed/Weapon Cooldown/Target 선택·해제/Ammo·Ripple·Salvo USER PASS, Defense Runtime·Pawn Rebind 기술 Automation은 보존하고 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`에서 재개한다. `CF-FQ-038`은 `DAUTH-P0-08A~M + P0-09~11 Technical PASS / UA-01~02 USER PASS / P0-12 USER PASS 2 / 다음 UA-03` 체크포인트를 보존한 Paused다.


```text
현재 Active: CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
대표 Plan: Document/Plan/InGameUIPlan.md v0.58.6
현재 Gate: UI-P0-03 Production Runtime PIE Closure
즉시 재개: Defense Production Panel USER Visual → Pawn Rebind USER Visual
후속 준비: UI-P0-04 AimReticle UISubsystem 통합 → UI-P0-05 TargetSelect Marker 통합
Radar/TargetPanel Sensor Source: Document/Systems/Targeting/SensorContact.md v1.1.0
Paused DAUTH: CF-FQ-038 / DataAuthoringPlan.md v0.2.24 / DataAuthoringRoadmap.md v0.1.32
DAUTH 완료 Gate: DAUTH-P0-11 Technical Validation — Technical PASS
DAUTH Core Technical Validation: PASS
DAUTH Frozen 24.90~24.94 Completeness: PASS
DAUTH 보존 Gate: DAUTH-P0-12 USER Authoring Acceptance — Paused / USER PASS 2 / UA-01~02 PASS / 다음 UA-03

P0-08A Technical PASS: Editor-only UCFVehicleRecipeData + 5 flat Profile UDataAsset / Never-Cook / Common Types
P0-08B Technical PASS: Stable Field Path / Field Value Codec / Current UCFVehicleData 117 leaf Registry bidirectional coverage
P0-08C Technical PASS: Immutable Recipe/Profile/Definition/Project Default Snapshot / deterministic fingerprint/hash / RequiredDependencies
P0-08D Technical PASS: Chassis Socket / Wheel Bounds Asset Snapshot Reader / resolver-relevant fingerprint
P0-08E Technical PASS: Snapshot-only Pure Resolver / Frozen R0~R16 / deterministic source precedence / Trace·Diff·Stale foundation
P0-08F Technical PASS: RF_Transient Definition Materializer / Stable-ID arrays / FieldCodec import / existing UCFVDAValidator / readback hash consistency
P0-08G Technical PASS: Existing Definition lossless Legacy Pin import / Mount hidden serialized partition / group·field Adoption Preview / Recipe-only commit
P0-08H Technical PASS: TOCTOU-safe FCFVehicleApplyService / transient preflight / dependency-safe exact Diff / AppliedState / atomic rollback / no-auto-save
P0-08I Technical PASS: Common FCFVehicleAuthoringService / R0~R3 typed contract / prospective Recipe preview / guarded Recipe-only write / shared Apply lane / bounded dedupe
P0-08J Technical PASS: projection-only Batch Column Registry / Recipe 7 + Profile 78 numeric allowlist / 117 resolved projection / canonical UTF-8 CSV / baseline manifest / ExportSetHash
P0-08K Technical PASS: manifest/current Registry verification / canonical CSV parse / 3-way merge / ownership conflict / transient Recipe·Profile prospective Resolver preview / BatchPlanHash / mutation0
P0-08L Technical PASS: exact Batch approval / global source TOCTOU preflight / B1 Recipe+B2 shared Profile atomic source transaction / rollback / approval invalidation
P0-08M Technical PASS: fresh R3 BatchApplyPlan / exact B3 approval / global preflight / canonical ApplyService sequence / partial result / no global rollback
P0-09 Technical PASS: single-Vehicle Workspace / Initial Import / Recipe basic intent / Resolve·Diff·Trace·Validation / shared Apply / standard Undo / Raw DA Open
P0-10 Technical PASS: Reference Compare / Assets+Layout semantic migration / Measurement+Adoption / Frozen 4-axis Driving Feel / Stable-ID Mount+Defaults / managed Wizard guard / standard Undo
P0-11 Core PASS: representative managed E2E / approval invalidation / fresh Resolve·Trace·Diff·Validation / Apply+Undo / Raw Drift fail-closed / applied VehicleData Fitting+Inventory consumption
Official Build: 98dfceab797942218bd09c844e398ceb / Exit Code 0
Focused P0-11 Automation: b53998e6cacf48a1a554d784b77e013c / 6/6 PASS / 0 FAIL
Focused Result JSON SHA-256: bce3414ec30cb4612209ff1f39f649990140950b73d4136929a3fa8a4be47263
Inventory representative pre-closure evidence: 4dfa10d20f00451abcab209b0db49d40 / 1/1 PASS
Fitting representative pre-closure evidence: 1c66a7139313458f9c5ab68ff03a7b0f / 2/2 PASS
Targeted Automation: 77b45525549b4866995b3d972fb4a9fa / 63/63 PASS / 0 FAIL
Result JSON SHA-256: a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f
Reusable Runner: Tools/RunDataAuthoringTests.ps1
Runtime UCFVehicleData contract mutation: 0
Inventory / Fitting consumer mutation: 0
SCFVDAWizardTab deletion/replacement: 0 / legacy tab preserved through P0-11/P0-12 + DG/DEL
Legacy managed-target guard: Layout Capture / Quick Tune Apply / Quick Tune Revert disabled + handler recheck
Vehicle Authoring Workspace: CarFight.VehicleAuthoring / P0-10 parity + tab spawn Automation PASS
P0-10 USER Visual/Usability/Driving Feel Acceptance: 미판정 / P0-12까지 보존
DG/DEL Gate: 미개방
Batch main page: 0
Content Asset mutation: 0
Asset Snapshot Reader mutation: 0
Existing UCFVDAValidator source mutation: 0
Raw SetField / direct Target writer / force / skip-validation: 0
Production Target writer: FCFVehicleApplyService only
Automatic Retry / Automatic Save: 0 / 0
Batch Import Session / 3-way Preview: Technical PASS
B1/B2 source commit: Technical PASS / source package Dirty 가능 / Target mutation 0 / Auto Save 0
B3 Definition Apply orchestration: Technical PASS / Target writer FCFVehicleApplyService only / global transaction·rollback 0 / Auto Save 0
Batch UI / file save: 0 / 0
Frozen 26.66~26.69 BatchOperationId/generic envelope: external Batch transport/client 전 follow-up
P0-09/P0-10 USER Visual/Usability/Driving Feel Acceptance: 미판정 / P0-12까지 보존
P0-11 Frozen completeness closure: Handling/Performance Adoption / Shared Profile edit+impact / External Drift 3-way recovery / Mesh-only Candidate+Create Vehicle From Mesh — Technical PASS
P0-12 USER Authoring Acceptance: Paused / UA-01~02 USER PASS / USER PASS 2 / 다음 UA-03
보존 USER Gate: UA-03 Recipe / Shared Profile / Affected Vehicle Impact

Paused Fitting: CF-FQ-034 / VehicleFittingPlan.md v0.17.0 / FIT-P0-07D USER Driving Feel Comparison
Paused Launcher: CF-FQ-029 / LauncherMissilePlan.md v0.14.0 / LM-P0-06 USER PIE
Done Scanner: CF-FQ-037 / SCAN-P0-00~07 PASS / SensorContact.md v1.1.0 Current / final Build fcf52353d1f5440392d5e1c379f09ee3 PASS

Ready Missile: CF-FQ-030 / CF-TC-027 Manual PIE Pending
기타 보호 상태: CF-FQ-015·026·034·035·038 Paused / CF-FQ-032 Active / CF-FQ-030 Ready / CF-FQ-031·033·008·036·037 Done


```

`CF-FQ-037`은 `SCAN-P0-06 USER PIE Acceptance`와 `SCAN-P0-07 Current System Integration`을 완료해 Done이다. 초기 USER FAIL은 Scanner fixture가 실제 GameMode DefaultPawn이 아닌 배치 Pawn에 연결된 것이 원인이었고, test-only `BP_ScanPlayerPawn` + `BP_ScanGameMode` + `TestMap_ScannerP0` override로 교정했다. USER PIE에서 V 단발 5초 Active Scan, 자동 종료, 활성 중 반복 입력 무연장과 선택 Target의 `???` 해제를 확인했다. USER Acceptance용 임시 관측 코드는 제거했고 final Build `fcf52353d1f5440392d5e1c379f09ee3` PASS다. 현재 구현 owner는 `Document/Systems/Targeting/SensorContact.md v1.1.0`이다.

CF-FQ-037 완료는 Radar Range/Zoom·동적 Blip·TargetPanel/Radar 최종 시각 작업, Sensor energy/heat, Missile/Utility 소비, AI/Network를 완료로 승격하지 않는다. 이 항목들은 각각 기존 CF-FQ-032 또는 별도 후속 Feature에서 다룬다.


`CF-FQ-033`은 Shield, 6방향 독립 Armor, ArmorPenetration, Vehicle Integrity, Shield 재생, Legacy Fallback과 Fitting Defense Commit을 구현하고 DR-P0-00~07, 공식 Editor Build, 전체 Automation과 DR-PIE-00~06 사용자 검증을 완료했다. 현재 구현은 `Document/Systems/Combat/VehicleDefense.md v1.0.0`과 `HitDamage.md v1.1.0`이 소유하며 `CF-FQ-033`은 Done이다.

`CF-FQ-032`는 현재 단일 Active다. UI-P0-02 Runtime Lifetime과 Launcher finite Ammo+Salvo Presentation은 USER PASS이며, Defense 저장 맵 PIE → Provider ViewData와 Pawn Rebind Old Pawn 이벤트 해제는 기술 Automation Success다. 다음 작업은 추가 기술 재구현 없이 `Defense Production Panel USER Visual → Pawn Rebind USER Visual` 두 Gate를 사용자 직접 확인으로 닫는 것이다. 이후에만 기존 Plan 순서대로 UI-P0-04 AimReticle 수명 통합과 UI-P0-05 TargetSelect Marker 통합으로 이동한다. 완료된 Scanner/Sensor Runtime은 후속 TargetPanel/Radar의 read-only source이며 UI가 Detection/Knowledge를 재계산하지 않는다.


`CF-FQ-035`는 TargetSelect 재개에 따라 Paused다. `INV-P0-00~05`, cross-feature `FFIT-P0-01~04`와 `FIT-P0-06 Fitting ViewData and Debug` C++ ViewData·Blueprint Contract Technical PASS, Full CarFight 84/84 증거를 그대로 보존한다. CF-FQ-034의 공식 Light·Default·Heavy 정량 Mobility baseline은 FIT-P0-07C에서 확보됐지만 실제 UI Widget, 16:9·32:9 가독성, Field Fitting USER PIE와 사용자 주행감 검증은 남아 있으므로 INV-P0-06 전체 Done 또는 CF-FQ-035 Done으로 판정하지 않는다.

`CF-FQ-031`은 Done이다. WeaponInstanceId별 Loaded, AmmoId별 Reserve, SingleCycle Commit·Rollback, Ripple·Salvo 전체 유효 발수 예약, FullMagazine Reload, Pause-safe Reload, WeaponPanel `Loaded / MagazineCapacity + label-less Reserve`, Fitting `InitialSortieAmmoLoads`·AmmoMassKg를 구현했고 Heavy·Ripple USER PIE를 완료했다. 현재 구현은 `Document/Systems/Combat/Ammo.md v1.0.0`이 소유하며 완료 Plan은 `Document/Plan/AmmoSystemPlan.md v0.7.0`이다.

`CF-FQ-029`는 Launch Handoff, 가변 Muzzle, SingleCycle, Ripple·Salvo Scheduler, Direct·Angled·Vertical Release와 `LM-P0-05 Launcher Editor Assets` 적용·독립 AssetDump 검증을 완료했으며 현재 단계는 `LM-P0-06 Launcher Integration PIE`다.
`CF-FQ-030`은 `Ready for Manual PIE` 상태다. `MG-P0-01~04` Direct Release Runtime, Missile Flight·Guidance 컴포넌트, TargetActor 제한형 유도, 목표 소실·오버슈트·Pool Reset과 전용 테스트 자산이 적용됐다. 2026-08-17 current AssetDump로 과거 WinError 87 readback 공백을 다시 검증해 Missile test folder 20/20, Maps World 12/12와 4개 계약 DataAsset의 저장값·Vehicle → EquipmentPreset → Weapon → Projectile hard reference chain을 확인했다. MissileDirectTest World package는 33 Actor와 test vehicle/RocketLauncher socket 구성을 확인했지만 전체 Actor label은 public readback 범위 밖이므로 다섯 MissileTarget label 자체를 독립 PASS로 추정하지 않는다. fresh Direct Runtime 보호 회귀 `29d0ef16dd5e4d56945ceae26eec6937`도 1/1 PASS다. Launcher Ripple·Salvo는 첫 발사 순간 `CommandTargetLocation`과 약한 `GuidanceTargetActor`를 함께 Snapshot해 차량의 이후 선택 변경과 같은 Volley의 미사일 목표를 분리한다. 기존 공식 Editor Build `ce88150dfbbc495d91d1e2b8cd7c5225` Exit Code 0과 전체 CarFight Automation `85c1e7285a7946fa8f55182b6247f0f4` 43/43·필수 21/21 Success를 보존한다. 이번 작업은 Source·Content Asset·Blueprint·Map 저장 변경 없이 수행했으며 정지·측면 이동·선택 변경·목표 파괴·오버슈트·Pool 재사용 USER PIE가 남아 있으므로 Done 또는 Systems Current로 판정하지 않는다.
`CF-FQ-019`는 사용자 결정에 따라 Deferred로 이동했으며, CF-FQ-029·030 이후 런처·미사일을 포함한 통합 회귀 범위로 다시 설계한다.
`CF-FQ-028`은 구현, 공식 Editor 빌드, 사용자 PIE와 Systems 승격을 완료해 Done으로 유지한다.
현재 발사체 추진과 지속형 FX의 구현 판단은 `Document/Systems/Combat/Projectile.md`를 우선한다.
`CF-FQ-027`은 Trail-only·Thruster-only·동시 FX, 유효 소켓·Missing Socket Fallback, 종료 Reset, Pool 20발 이상, Ribbon History와 30 FPS 고속 Bounds 사용자 PIE를 완료했으므로 Done이며 `CF-TC-023 PASS`로 유지한다.
`CF-FQ-026`은 현재 Paused다. `TS-P0-00~07 Done`, TS-P0-08 Remote Technical 3/3, LOS Prefilter PASS와 최종 자산 비변경 회귀를 그대로 보존한다. 7도/1200m 범위 체감, 동일 차량 인식 영역, debug Sphere 가시성, 후보 텍스트 겹침과 16:9·32:9 입력·UI 감각은 USER PIE가 가능한 시점까지 Pending이며 새 USER PASS를 추정하지 않는다.

### 프로젝트 전역 사운드 제외 결정

```text
- CarFight는 게임 사운드를 구현하거나 제공하지 않는다.
- SoundWave, SoundCue, MetaSound Source, Sound Attenuation 자산을 제작·도입하지 않는다.
- USoundBase, UAudioComponent와 사운드 재생 함수를 런타임 계약에 추가하지 않는다.
- AudioMixer, Audio Modulation과 별도 오디오 플러그인 의존성을 추가하지 않는다.
- 엔진음, 타이어음, 차량 충돌음, 무기 발사음, 피격음, 파괴음, UI음과 BGM은 기능·품질·테스트 완료 조건에서 제외한다.
- Unreal이 생성한 기본 플랫폼 오디오 설정은 유지할 수 있으나 CarFight 게임 기능으로 해석하지 않는다.
```

아래 과거 상태 기록에 남은 `FX / Audio`, `사운드`, `SFX` 표현은 당시 계획 이력이며 현재 구현 지시로 사용하지 않는다.

### 2. 전투 FX 현재 실제 기준선

현재 확정된 설계 기준:

```text
- 최종 미술 방향: 세미리얼
- 제작 정책: FAB FX 팩 우선 활용 / 직접 신규 제작 최소화
- P0 핵심: Muzzle, 첫 Impact, 최초 Destroyed Niagara FX
- 고정 위치: Mesh Socket 우선
- 충돌 위치: FCFDamageHitContext 월드 Transform
- Projectile 지속 FX: ACFProjectileActor가 소유하고 Pool 반환 전 초기화
- 게임 사운드: 프로젝트 전역 비지원
```

현재 구현·자산 상태:

```text
- UCFCombatFxData와 UCFCombatFxComp C++ 구현 완료
- CarFight_Re.Build.cs Niagara 모듈 의존성 반영 완료
- WeaponData DefaultFireFxData, ProjectileData DefaultImpactFxData와 VehicleData DefaultDestroyedFxData 추가 완료
- ACFVehiclePawn 승인 발사, HitScan Impact, Projectile Impact와 VehicleHealth 최초 파괴 FX 요청 연결 완료
- ACFCombatFxPreviewActor v1.2.0 EditorOnly 도구 구현 완료
- Editor Viewport 자동 반복, Uniform Scale, 최대 수명 자동 정지, Override Niagara 적용과 두 Scale 전달 모드 구현 완료
- UCFCombatFxData.MaximumLifetimeSeconds와 런타임 Loop 강제 제거 안전 퓨즈 구현 완료
- UCFVehicleData.DestroyedFxSocketName 기본값 `FX_Destroyed`와 SM_Body 소켓 우선 파괴 위치 해석 구현 완료
- C++ Foundation Build Job bd5a50bf388049ec84123aec4942ae96 / Exit Code 0
- Editor Preview Tool 최초 Build Job c9682d4a0be24ead95e87a5a7e8b5849 / Exit Code 0
- Rapid Preview Tuning Build Job 89b6d82710af47e89a4c5b3823037d4e / Exit Code 0
- 최신 C++ 변경은 사용자가 직접 Editor 빌드 PASS를 확인했으며 Admin Build Job ID는 없다.
- `/Game/CarFight/FX/Data`의 CombatFxData 3개를 AssetDump로 다시 확인했으며 3개 성공, 0개 실패다.
- Impact P0 Niagara는 `/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_BasicHit`으로 사용자 확정했다.
- NS_BasicHit의 외부 Transform Scale 미반응은 허용하며 현재 자산 크기를 그대로 사용하고 추가 스케일 튜닝을 중단한다.
- 현재 기준 VehicleData는 `/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan`이다.
- 기준 차체 메시 `/Game/CarFight/Vehicles/Meshes/Sedan/Sedan`에 `FX_Destroyed` 소켓을 배치한 뒤 폭발 위치가 해결됐음을 사용자가 확인했다.
- Destroyed 위치는 SM_Body 소켓, SM_Body Bounds 중심, Actor Transform, DamageHitContext 순으로 Fallback한다.
- DA_PoliceCar는 현재 사용하지 않는 폐기 자산이며 신규 FX 연결 대상이 아니다.
- NS_Explossion은 Loop 반복으로 Destroyed P0 후보에서 제외한다.
- SmokeBuilder는 NiagaraSystem 0개와 Cascade ParticleSystem 64개이며 현재 Niagara 전용 P0 계약에는 사용하지 않는다.
- AssetDump 프로필의 DataAsset 프로퍼티 미노출 제한은 유지하며 실제 연결은 사용자 Editor 저장 상태를 우선한다.
- 최종 사용자 PIE에서 승인 발사 Muzzle 1회, 발사 거부 0회, Impact 실제 위치·첫 1회·중복·잔류 없음, Destroyed FX_Destroyed 소켓 위치·최초 1회·추가 피해 중복 없음과 기존 전투 회귀를 모두 PASS했다.
- `CF-TC-021`은 PASS이며 현재 구현 문서는 `Document/Systems/Combat/CombatFx.md`다.
```

따라서 `CF-FQ-024`는 **Done / User PIE PASS / CF-TC-021 PASS**로 판정한다. 현재 구현 기준은 `Document/Systems/Combat/CombatFx.md`로 승격한다.

### 2-1. 투사체 비행 FX 현재 기준선

지속형 Trail·Thruster 런타임은 `Document/Systems/Combat/Projectile.md` Current System에 통합됐다.

```text
- UCFProjectileData v1.7.1
  - ECFProjectileFxAttachMode
  - FCFProjectileAttachedFxSettings
  - TrailFxSettings / ThrusterFxSettings
  - FX_Trail / FX_Exhaust 기본 소켓
  - 소켓·Fallback 공통 RelativeTransform.Scale 독립 FX 배율
- ACFProjectileActor v1.8.1
  - TrailOriginComponent / TrailNiagaraComponent
  - ThrusterOriginComponent / ThrusterNiagaraComponent
  - MeshSocketWithFallback / ProjectileRelative
  - Missing Socket Fallback Debug
  - Motor Burning 상태 기반 Thruster 활성화
  - InvalidActivation / Manual / Hit / LifeExpired 공용 Reset
  - FinishDeactivatePolicy 이전 Niagara DeactivateImmediate·Reset·Asset 해제
- CFProjectileFlightFxTests.cpp v1.1.0
  - 기본값, 컴포넌트, Fallback, 독립 Scale과 Reset RuntimeContract 소스
```

검증 상태:

```text
Foundation 빌드: 445848ab0f7749fcab8188b164bb1487 / Exit Code 0
Scale 보정 최종 빌드: e8b812bd479549299dd116f9bae8996f / Exit Code 0
Automation 소스 컴파일: PASS
Automation 실행: Not Run — 현재 Admin 도구에 Unreal Automation Runner 없음
Thruster-only·FX_Exhaust·독립 Scale 사용자 PIE: PASS
Trail-only / Trail+Thruster / 유효 소켓 / Missing Socket Fallback 사용자 PIE: PASS
Hit·LifeExpired Reset / Pool 20발 이상 / Ribbon History 무잔류 사용자 PIE: PASS
30 FPS 고속 Bounds·컬링 / Impact·Damage·Pool 회귀 사용자 PIE: PASS
```

현재 해석:

```text
- CF-FQ-027은 Done이며 CF-TC-023은 PASS다.
- 지속형 Trail·Thruster 런타임과 검증된 전체 에디터 행렬은 Projectile Current System의 실제 기준이다.
- Current System: Document/Systems/Combat/Projectile.md v1.5.0
- Completed Plan: Document/Plan/ProjectileFlightFxPlan.md v1.0.0
- Automation 실행은 Runner 미노출로 Not Run이며 소스 컴파일 PASS와 사용자 PIE 결과를 분리해 기록한다.
```

### 2-2. 발사체 추진 Current System

`CF-FQ-028` 비유도 Rocket 추진은 구현, 공식 Editor 빌드, 사용자 PIE와 Systems 승격을 완료했다.

```text
- CFProjectileMotorTypes v1.0.0
  - ECFProjectileMotorState: Inactive / Disabled / IgnitionDelay / Burning / BurnedOut
  - FCFProjectilePropulsionConfig
  - FCFProjectileMotorSnapshot
- UCFProjectileMotorComp v1.0.0
  - 발사 시 고정 LaunchDirection 저장
  - IgnitionDelay와 BurnDuration 결정적 시간 분할
  - ThrustAccelerationCmPerSecSq 기반 Velocity 가속
  - MaximumPropelledSpeed 제한
  - BurnedOut 뒤 추가 가속만 종료하고 Projectile 활성 유지
  - ResetMotor와 활성화 횟수 Debug
- UCFProjectileData v1.7.1
  - PropulsionConfig 추가
  - bUsePropulsion=false 기존 데이터 호환
  - 추진 설정과 Trail·Thruster 독립 Scale Debug 요약
- ACFProjectileActor v1.8.1
  - ProjectileMotorComponent 기본 서브오브젝트
  - ProjectileMovement보다 MotorComp 선행 Tick
  - Activate·Deactivate·Pool Reset 연결
  - Thruster FX를 실제 Burning 상태에만 동기화
  - RelativeTransform.Scale을 소켓·Fallback 공통 독립 FX Scale로 적용
- CFProjectileMotorTests.cpp v1.0.0
  - 기본값, 점화 지연, 잔여 프레임 연소, 속도 상한, BurnedOut 관성, Reset과 기존 포탄 회귀 계약
```

검증 상태:

```text
첫 빌드: 984f0ccab94241148e7b40b9d30a5572 / Exit Code 6
첫 빌드 결함: Automation FVector::Size double과 float 기대값 TestEqual 오버로드 충돌
결함 수정: 기대값을 double로 통일
추진 Foundation 빌드: 2ffd09357e654bb7970a2e379f5ab45f / Exit Code 0
Scale 보정 최종 빌드: e8b812bd479549299dd116f9bae8996f / Exit Code 0
UHT·Motor·Data·Actor·FlightFx Automation 컴파일·링크: PASS
Automation 실행: Not Run — 현재 Admin 도구에 Unreal Automation Runner 없음
Rocket ProjectileData 자산: DA_PFX_ThrusterTest / User Editor 저장
사용자 PIE: PASS / RelativeTransform.Scale 0.2 정상 축소 / CF-TC-024 PASS
```

테스트 Rocket ProjectileData 작성과 사용자 PIE는 완료됐다. 아래 값은 현재 검증된 첫 시작 기준으로 유지한다.

```text
InitialSpeed = 3000 cm/s
bUsePropulsion = true
IgnitionDelaySeconds = 0.05 s
BurnDurationSeconds = 1.0 s
ThrustAccelerationCmPerSecSq = 9000 cm/s²
MaximumPropelledSpeed = 10000 cm/s
LifeTimeSeconds = 4.0 s
GravityScale = 0.35
ThrusterFxSettings = CF-FQ-027 시각 후보 사용
```

Target Homing, Laser Guided와 수동 유도는 이 기준선에 포함하지 않는다. 후속 `UCFProjectileGuidanceComp` 기능이 추진 방향 수정만 소유하도록 분리한다.

현재 판정:

```text
CF-FQ-028: Done
CF-TC-024: PASS
Current System: Document/Systems/Combat/Projectile.md v1.5.0
Completed Plan: Document/Plan/ProjectilePropulsionPlan.md v1.2.0
반복 전투 확장 회귀: CF-FQ-019 Deferred / 런처·미사일 구현 범위 확정 후 재설계
```

### 3. 현재 구현 원칙

```text
- 발사·피격·피해·파괴 판정과 FX 연출을 분리한다.
- FX 자산 누락 또는 생성 실패가 전투 판정을 취소하지 않는다.
- FAB 원본은 직접 수정하지 않고 필요한 자산만 CarFight Adapted 경로로 복제한다.
- Muzzle과 FX_Exhaust 소켓 +X축을 방출 방향으로 통일한다.
- FAB Niagara 축 차이는 RotationOffset으로 보정한다.
- Projectile FX 소켓이 없으면 ProjectileData Fallback Relative Transform을 사용한다.
- Destroyed FX는 차량별 SM_Body.FX_Destroyed 소켓을 우선하고 누락 시 SM_Body Bounds 중심을 사용한다.
- Impact P0는 NS_BasicHit을 현재 크기 그대로 사용하며 범용 Niagara Scale 완성을 다시 열지 않는다.
- P0에서는 Trail 자연 소멸, 별도 FX Pool, 다중 노즐과 표면별 Impact를 제외한다.
- TargetSelect TS-P0-00~07 완료 코드와 에셋은 FX 작업 중 변경하지 않는다.
```

### 4. 다음 작업 선택 기준

현재 단일 Active 작업은 없다. CF-FQ-030 `Persisted Missile Test Asset Technical Verification`은 PASS로 닫혔고, 남은 Ready·Paused 기능의 실제 next gate는 USER PIE 또는 USER Visual/Tuning 확인을 포함한다. 다음 기능은 `ActiveWork.md`와 `03_FeatureQueue.md`에서 사용자가 선택한 뒤에만 Active로 전환한다.

```text
1. CF-FQ-030 Missile — Ready / CF-TC-027 Manual PIE Pending
   - MissileGuidancePlan.md v0.5.0 기준으로 재개한다.
   - MG-P0-00~04 Direct Runtime과 Persisted Asset Technical Verification을 관련 Source/Asset 변경이 없는 한 반복하지 않는다.
   - 정지 목표·측면 이동·발사 후 Target 변경·목표 파괴·Overshoot·Pool 재사용을 사용자가 직접 확인한다.

2. CF-FQ-029 Launcher — Paused / LM-P0-06 USER PIE Pending
   - LauncherMissilePlan.md v0.14.0 기준 MuzzleBlocked·Angled/Vertical Ejection·Carrier Velocity·Direct 기본값 복구만 확인한다.
   - LM-P0-06A Failure Policy Technical Closure는 반복하지 않는다.

3. CF-FQ-037 Scanner — Done / `SensorContact.md v1.1.0` Current
   - SCAN-P0-00~07과 final Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 보존한다.
   - 완료 당시 fixture RCA와 USER Acceptance는 Historical `ScannerIntegrationPlan.md`에서 조회하며 재개 작업으로 취급하지 않는다.

4. 기타 보호된 USER/Ready·Paused 체크포인트

   - CF-FQ-032 Defense·Pawn Rebind USER Visual / Radar·TargetPanel 별도 Pending
   - CF-FQ-026 TS-P0-08 USER PIE
   - CF-FQ-015 VD-P0-04 USER Tuning
   - CF-FQ-034 Mobility·Field UI USER
   - CF-FQ-035 Field UI·Mobility USER

완료 기준선
- CF-FQ-036/037 SensorContact+Scanner는 Done / Current System이며 old SEN/SCAN P0 Gate를 다시 실행하지 않는다.
- Sensor/Scanner 현재 의미는 `Document/Systems/Targeting/SensorContact.md v1.1.0`을 우선한다.

- CF-FQ-030 Persisted Asset 기술 PASS나 다른 Automation 결과를 USER PIE PASS로 확대하지 않는다.
```

`CF-FQ-024`, `CF-FQ-027`과 `CF-FQ-028`은 Done 상태를 유지하며 완료된 전투 FX·비행 FX·추진을 새 기능 안에서 재구현하지 않는다.

### 5. 일시중지·병행 작업 해석

`CF-FQ-029`는 LM-P0-06 체크포인트를 보존한 Paused이고 `CF-FQ-026`은 TS-P0-08 Remote Technical PASS와 USER PIE 체크포인트를 보존한 Paused다.
`CF-FQ-034`는 현재 Active이며 FFIT-P0-01~04, FIT-P0-06 C++ ViewData·Blueprint Contract와 FIT-P0-07B 공식 Fixture 기술 계약까지 PASS다. 같은 SUV 플랫폼의 Light 1000kg / Default 1570kg / Heavy 1600kg 기준은 확정됐고, 남은 범위는 FIT-P0-07C 정량 Mobility 측정·USER 주행감과 FFIT-P0-05 Field UI·PIE다. `CF-FQ-035`는 Remote Technical Complete 체크포인트를 보존한 Paused이며 USER-facing Field UI·PIE가 남아 있다. `CF-FQ-032`는 USER Visual 체크포인트 보존을 위한 Paused 상태다.
다음 착수 판단은 `ActiveWork.md`, `03_FeatureQueue.md`와 각 대표 Plan의 현재 체크포인트를 따르며 과거 Active 기록을 현재 상태로 복원하지 않는다.

---

## 2026-07-21 현재 프로젝트 상태

### 1. 현재 완료된 전투 기준선

현재 싱글 기준 차량의 최소 전투 판정 루프와 조준 표시 기준은 아래 범위까지 완료됐다.

```text
- CF-FQ-016 차량 무기 조준 및 발사: Done
- CF-FQ-017 Reticle / FireFeedback UI: Done
- CF-FQ-018 피격 판정 및 피해 처리: Done
- CF-FQ-022 조준점·터렛·총구 정렬: Done
- CF-FQ-023 고속 Projectile 연속 충돌: Done
- CF-FQ-025 이중 레티클 및 터렛방향 시각화: Done / User PIE PASS
```

현재 검증된 흐름:

```text
차량 주행
→ Image_CenterDot 조준점과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클 표시
→ Reticle 목표와 터렛 / Muzzle 정렬
→ 승인된 HitScan 또는 Projectile 발사
→ 시각 차체 SM_Body 첫 Blocking Hit
→ FCFDamageHitContext 생성
→ BaseDamage 체력 누적 감소
→ 최초 Destroyed 상태 전환
→ Reticle / FireFeedback 상태 표시
```

### 2. 현재 활성 병목

현재 기능 병목은 발사·피격·피해 판정이 아니라 **전투 결과를 실제 화면 효과와 소리로 표현하는 연출 계층 부재**다.

```text
- 발사 성공 Muzzle FX와 발사 사운드 없음
- HitScan / Projectile Impact FX와 Impact 사운드 없음
- 최초 차량 파괴 FX와 파괴 사운드 없음
- ProjectileData.ImpactEffectId는 임시 ID로만 존재
- C++에 Niagara / USoundBase 연출 데이터 계약이 없음
```

따라서 현재 활성 작업은 다음으로 고정한다.

```text
CF-FQ-024 전투 FX 및 사운드 구현
→ 발사·Impact·파괴 연출 P0 사용자 PIE 완료
→ CF-FQ-019 주행·전투 반복 테스트
→ CF-FQ-020 조작감·전투 템포·피드백 개선
→ CF-FQ-021 핵심 게임 루프 검증
```

대표 Plan:

```text
Document/Plan/CombatFxAudio/ImplementationDesign.md
```

### 3. 현재 구현 원칙

```text
- 전투 판정과 연출을 분리한다.
- 발사 승인, 첫 Impact와 최초 파괴 전환은 기존 C++ 판정이 소유한다.
- Niagara, 사운드와 Attenuation 선택은 DataAsset과 Unreal Editor 자산이 소유한다.
- 연출 자산이 없거나 재생에 실패해도 발사·피해·파괴 결과는 유지한다.
- 소스 코드에 특정 FX / Audio 자산 경로를 하드코딩하지 않는다.
- P0에서는 발사, Impact와 파괴의 최소 1회성 연출만 구현한다.
```

### 4. 현재 리스크와 대응

| 리스크 | 내용 | 대응 |
|---|---|---|
| 연출 중복 | Projectile OnHit와 보조 Sweep 또는 추가 피해에서 같은 FX/Sound가 반복될 위험 | 기존 첫 Impact 및 최초 Destroyed 1회 판정을 연출 트리거로 사용 |
| Pool 잔류 | Projectile Pool 재사용 시 Trail 또는 AudioComponent 상태가 남을 위험 | 활성화·반환 시 연출 상태 명시적 초기화 |
| 판정 결합 | 연출 실패가 발사·피해 실패로 이어질 위험 | 연출 요청은 결과 판정 이후 호출하고 null-safe로 처리 |
| 자산 하드코딩 | 임시 자산 경로가 C++에 고정될 위험 | 공용 Combat FX / Audio DataAsset 참조 사용 |
| 과도한 폴리싱 | 완성형 사운드·표면 분기·파괴 연출로 범위가 확산될 위험 | P0 필수 6개 연출을 먼저 완료하고 나머지는 CF-FQ-020으로 이관 |

### 5. 과거 상태 기록 해석

아래 `2026-06-18 현재 프로젝트 상태` 이후 내용은 싱글 전환 당시의 기준과 역사 기록이다.
현재 기능 진행 상태와 우선순위는 이 `2026-07-15 현재 프로젝트 상태`, `02_Roadmap.md`, `03_FeatureQueue.md`를 우선한다.

---

## 2026-06-18 현재 프로젝트 상태

### 1. 프로젝트 운영 상태

CarFight는 현재 **멀티플레이 / Dedicated Server 확장 단계에서 싱글 플레이 1대 차량 고도화 단계로 전환**한다.

현재 전환 이유는 아래와 같다.

```text
1. 서버, 멀티플레이, 2클라 검증 범위를 현재 개발 일정에서 제거한다.
2. 현재 구현된 차량 1대의 조작감과 표현 품질을 클라이언트 사이드에서 고도화한다.
3. 개발 일정을 약 2~3개월 단축한다.
4. 서버/멀티 작업물은 즉시 삭제하지 않고 보류/역사 기록으로 분리한다.
```

현재 개발 방식은 아래처럼 단순화한다.

```text
싱글 차량 기능 후보
→ 로컬 플레이 기준 필요성 판단
→ 현재 차량 코어 영향 판단
→ Plan 작성 또는 바로 구현
→ PIE 기준 검증
→ Systems / TestChecklist 반영
```

2026-06-19 에디터 확인 결과:

```text
기준 차량 조작 / Aim Fire / Reticle UI / VehicleDebug Aim은 싱글플레이 기준에서 정상작동 확인.
이번 세션에서는 기능 추가 없이 싱글플레이 전환 정리를 완료 대상으로 본다.
```

2026-06-19 다음 개발 기준:

```text
싱글 실행 기준선과 로컬 Aim / Fire / Reticle 골격은 선행 기반으로 본다.
다음 개발은 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 게임 루프 검증 순서로 진행한다.
```

주의:
- 현재 확인된 Aim Fire는 로컬 발사 입력/검증 골격 기준이다.
- 실제 차량 무기 시스템, 발사 이펙트/사운드, 피격 판정, 피해 처리는 아직 새 개발 대상으로 본다.

### 2. 문서 구조 상태

2026-06-02 기준 `Document/ProjectSSOT/` 루트는 아래 활성 문서 체계로 정리됐다.

```text
README.md
00_Vision.md
01_ProjectState.md
02_Roadmap.md
03_FeatureQueue.md
04_ProjectDecisions.md
05_TestChecklist.md
Archive/
```

현재 문서 역할은 아래 기준으로 고정한다.

| 위치 | 역할 |
|---|---|
| `Document/ProjectSSOT/` | 프로젝트 판단 기준 |
| `Document/Plan/` | 앞으로 개발할 기능의 상세 계획 |
| `Document/Systems/` | 완료된 기능의 현재 구현 기준 |
| `Document/ProjectSSOT/Archive/` | 역사 기록 / 비활성 문서 |

특히 `Document/Plan/`은 완료 기능의 현재 기준으로 보지 않는다.
완료된 기능은 `Document/Systems/`를 기준으로 본다.

### 3. 현재 구현 기준

현재 차량 코어 기준선은 여전히 아래 조합이다.

```text
BP_CFVehiclePawn
ACFVehiclePawn
UCFVehicleDriveComp
UCFWheelSyncComp
UCFVehicleData
DA_PoliceCar
/Game/Maps/TestMap
```

현재 차량 구조는 최종 목표인 `CMVS / Cluster Union / Geometry Collection` 구조가 아니라,
`ChaosWheeledVehicle` 기반 하이브리드 구조다.

차량 코어 쪽에서는 아래 항목을 현재 유지 코어로 본다.

```text
- VehicleData
- DriveState
- WheelSync
- Thin BP 원칙
```

### 4. 서버 / 네트워크 상태

현재 `Document/Systems/Network/ServerSpawn.md` 기준으로 Dedicated Server 최소 Spawn/Possess 흐름은 Systems에 기록된 상태다.
하지만 2026-06-18 전환 기준에서는 이 흐름을 **현재 착수 대상이 아니라 보류된 과거 구현 기준**으로 본다.

현재 네트워크 구현 기준은 아래 수준으로 본다.

```text
- CarFight_ReServer Target 존재
- ACFMPGameMode 기반 서버 테스트 GameMode 존재
- PostLogin 기반 수동 차량 Spawn/Possess 흐름 존재
- 기본 차량 Pawn fallback 경로는 /Game/CarFight/Vehicles/BP_CFVehiclePawn
- 현재 ServerSpawn은 Dedicated Server 전체 운영 시스템이 아니라 최소 멀티플레이 스폰 흐름이다.
```

현재 해석:
- 위 구현은 삭제 확정이 아니라 `Deferred` 기준으로 보존한다.
- 기본 실행 경로와 이번 로드맵에서는 서버/멀티 검증을 제외한다.
- `CFMPGameMode`, Server Target, CFNetSmooth, 서버 런처, 네트워크 테스트 도구는 현재 착수 판단에서 참고/보류 대상으로만 읽는다.
- 기본 실행 기준선은 `CFSingleGameMode`, 로컬 차량 Pawn, 로컬 Aim/Fire 흐름을 우선한다.
- 이번 세션에서는 새 기능을 추가하지 않고, 싱글플레이 기준과 충돌하는 실행 코드/설정/현재 기준 문서만 정리한다.
- 서버 Target과 보관 문서 삭제 여부는 별도 결정 전까지 보존한다.

### 5. 현재 최우선 개발 후보

현재 최우선 후보는 `03_FeatureQueue.md`의 아래 항목이다.

```text
1. 차량 무기 조준 및 발사 기능 구현
2. 발사 이펙트, 사운드, 조준 UI 구현
3. 피격 판정 및 피해 처리 구현
4. 주행과 전투 흐름 반복 테스트
5. 조작감, 전투 템포, 피드백 개선
6. 핵심 게임 루프 검증
```

현재 기준 차량은 `DA_PoliceCar`이며, 다차종 양산은 장기 목표로만 유지한다.
전투 루프는 서버 권한 구조가 아니라 싱글 로컬 기준으로 먼저 검증한다.

### 6. 현재 하지 않을 것

현재 단계에서 바로 하지 않을 것은 아래와 같다.

```text
- Dedicated Server 고도화
- 2클라 Spawn/Possess 검증
- 이동 복제 / 원격 차량 보간
- 서버 권한 발사 요청
- 서버 런처 / 운영툴 / 관리툴
- 로비 / 세션 / 매치메이킹
- 정식 웹 관리툴부터 만들기
- 인벤토리 / 보상 / 경제 시스템부터 만들기
- 새 차종 양산
- CMVS 최종 구조 전환 구현
- 차량 모델링 파이프라인 확정
- 대규모 서버 프레임워크 선구축
- 무기 종류 대량 추가
- 탄종/장갑/피팅 밸런스 대량 확장
- AI 교전 전체 구현
```

관리툴과 서버 운영 도구는 현재 싱글 차량 고도화 범위 밖이다.

### 7. 현재 리스크

| 리스크 | 내용 | 대응 |
|---|---|---|
| 범위 확산 | 서버 / 멀티 / 관리툴을 다시 끌고 들어올 위험 | 이번 사이클은 싱글 1대 차량으로 제한 |
| 문서 혼동 | Plan과 Systems를 현재 기준처럼 섞어 볼 위험 | 완료 기능은 Systems 우선 |
| 서버 구현 잔재 | `CFMPGameMode`, Server Target, CFNetSmooth를 현재 개발 목표로 오해할 위험 | 현재 착수 기준에서는 참고/보류 대상으로만 표기 |
| 싱글 품질 분산 | 주행/카메라/조준/휠을 동시에 크게 건드릴 위험 | 한 번에 하나의 플레이 감각 축만 검증 |
| 전투 범위 확산 | 무기/탄종/장갑/AI/피팅을 한 번에 열 위험 | 이번 순서는 최소 무기 발사와 피해 루프부터 검증 |
| 피드백 선행 과다 | 이펙트/사운드를 피해 판정 없이 과하게 꾸밀 위험 | 발사 피드백은 판정 흐름을 읽히게 하는 최소 수준으로 시작 |
| 관리툴 조기 개발 | 데이터 구조 불안정 상태에서 관리툴을 갈아엎을 위험 | 현재 일정에서 제외 |
| 차량 코어 품질 후속 | WheelSync 고속 시각 품질 등 폴리싱 항목 잔존 | 기능 FAIL이 아니라 품질 후속으로 분리 |

---

## 현재 실제 기준선
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`
- 현재 기준 테스트 맵: `/Game/Maps/TestMap`

---

## 현재 상태 요약
### 1. 차량 구조
- 현재 차량 구조는 `CMVS`가 아니라 `ChaosWheeledVehicle` 기반 하이브리드 구조다.
- 현재 기준 Pawn은 표준 차량 이동 컴포넌트 위에 `DriveState`, `WheelSync`, `VehicleData`를 올려 사용하는 방식이다.

### 2. BP / C++ 책임 분리
- 현재 기준은 C++ 코어 우선이다.
- `BP_CFVehiclePawn`는 Thin BP로 유지한다.
- 실제 판단 / 상태 / 규칙은 Native 쪽에 두는 방향을 유지한다.

### 3. 데이터 축
- 차량별 차이는 `UCFVehicleData` 축으로 분리하는 방향이 이미 적용되어 있다.
- 현재 실차 기준은 `DA_PoliceCar` 하나다.

### 4. 현재 작업 제약
- 2026-03-27 기준으로 `ue-assetdump` 플러그인 개선 작업 때문에 Unreal Editor 실행이 막혀 있었다.
- 2026-03-30 기준 현재는 에디터 복구가 확인되었고, PIE 기반 런타임 검증을 재개할 수 있다.
- 2026-03-31 기준 `WheelSync` 쪽에서 런타임 spin 경로 재검증이 다시 가능해졌고, 방향 반전 수정까지 반영됐다.
- 2026-03-31 후속 종료 체크 기준 `P0-002 WheelSync`는 PASS로 닫았다.
- 2026-04-01 기준 현재 상태:
  - `Runtime` 디버그 가독성 개선은 완료로 본다.
  - 바퀴 파묻힘은 `Anchor Z`가 아니라 `WheelRadius` 조정으로 해결했다.
  - `ACFVehiclePawn` 디테일 패널 카테고리 정리를 위해 `CarFight|VehiclePawn` 루트 분리가 반영됐다.
  - 후속 메타데이터 정리로 `VehicleDriveComp`는 `Vehicle Drive`, 컴포넌트 참조는 `컴포넌트` 루트로 분리되어 `Vehicle Pawn` 중복 표시가 해소됐다.
  - `CFVehicleData` 내부 `AutoFit / VehicleLayoutConfig / WheelLayout` 레거시 구조는 하드 삭제 완료 상태로 본다.
- 따라서 현재 다음 우선순위는 문서 작업이 아니라 고속 휠 시각 품질 검토다.

---

## 실측 스냅샷 (2026-03-27)
아래 항목은 문서 추정이 아니라 자산 덤프와 현재 코드 기준으로 다시 확인한 값이다.

### 1. `BP_CFVehiclePawn` 실측
- 부모 클래스는 `CFVehiclePawn`이다.
- 요약 기준:
  - 컴포넌트 15개
  - 그래프 2개
  - `EventGraph` 노드 8개
  - `UserConstructionScript` 노드 1개
- 현재 확인된 주요 컴포넌트:
  - `VehicleMesh`
  - `VehicleMovementComp`
  - `VehicleDriveComp`
  - `WheelSyncComp`
  - `SM_Chassis`
  - `Wheel_Anchor_FL / FR / RL / RR`
  - `Wheel_Mesh_FL / FR / RL / RR`
  - `SpringArm`
  - `Camera`
- 현재 클래스 기본값 기준:
  - `VehicleData = DA_PoliceCar`
  - `InputDeviceMode = GamepadOnly`
  - `bEnableDriveStateOnScreenDebug = false`
  - `DriveStateDebugDisplayMode = MultiLine`
  - `bShowDriveStateTransitionSummary = true`
- `EventGraph`는 현재 기준으로 매우 얇고, 주 용도는 `BeginPlay` 시점의 디버그 위젯 생성이다.
- 현재 `VehicleMesh`에는 기존 차량용 스켈레탈 메시가 남아 있다.

### 2. `WheelSyncComp` 초기 실측 (2026-03-27, 역사 기록)
- 아래 항목은 `2026-03-27` 시점의 초기 실측 / 실패 기록이다.
- 현재 운영 기준은 아래 `2-2. WheelSyncComp 운영 메모`를 우선해서 본다.
- 현재 기준 설정:
  - `ExpectedWheelCount = 4`
  - `bAutoPrepareOnBeginPlay = false`
  - `bAutoFindComponentsByName = true`
  - `bEnableApplyTransformsInCpp = true`
  - `bUseSteeringYawDebugPipe = true`
  - `bUseSuspensionZDebugPipe = false`
  - `bUseWheelSpinPitchDebugPipe = false`
  - `bApplySteeringYawInCpp = true`
  - `bApplySuspensionZInCpp = false`
  - `bApplySpinPitchInCpp = false`
- 현재 디버그 / helper / compare 관련 설정:
  - `bDebugMode = false`
  - `bUseRealWheelTransformHelper = false`
  - `bEnableHelperCompareMode = false`
  - 레거시 단일축 테스트 필드는 비활성 상태다.
- 현재 덤프 기준 런타임 준비 상태:
  - `bWheelSyncReady = false`
  - `LastValidationSummary = NotValidated`
- 따라서 현재 문서 기준으로는 "보수적 기본 설정은 확인됨"까지는 적을 수 있지만, "런타임 준비 완료"까지는 아직 적지 않는다.
- 2026-03-30 PIE 기준 확인된 실제 증상:
  - 휠 초기 배치가 비정상이다.
  - 전진 중 바퀴 스핀 시각 반응이 없다.
  - 조향 시 앞바퀴 피벗이 비정상적으로 보인다.
  - 심한 떨림 / 이중 적용은 보이지 않았다.
- 현재 런타임 오버레이에서는 `RuntimeReady = True`, `WheelSync = True`가 표시됐지만, 실제 시각 결과는 FAIL이다.
- 현재 설정 기준으로는 `bUseWheelSpinPitchDebugPipe = false`, `bApplySpinPitchInCpp = false`라 바퀴 스핀 시각 반응이 비활성일 가능성이 크다.

### 2-2. `WheelSyncComp` 운영 메모 (2026-03-31 PIE 기준)
- 현재 런타임 문자열 기준:
  - `SpinOwned = On`
  - `RuntimeSpin = True`
  - `VisualSign = -1.0`
- 현재 `UCFWheelSyncComp` 운영 경로 메모:
  - `CFWheelSyncComp v1.1.8`
  - `bApplySpinPitchInCpp = true`
  - `WheelSpinVisualSign = 1.0`
  - `WheelSpinMeshAxisSign = -1.0`
  - 저속 cap + sign hold + delta local rotation 경로가 활성화된 상태다.
- 현재 판정:
  - 수동 Anchor 기준선에서 `바퀴 위치 PASS`, `조향 피벗 PASS`는 유지된다.
  - 전진 / 후진 시각 회전 방향 반전 문제는 수정 후 눈검사 기준 정상으로 읽힌다.
  - `P0-002 WheelSync`는 종료 체크 기준 PASS로 판정했다.
  - 고속 타이어 효과의 모양새는 아직 개선 여지가 있지만, 현재 기준에서는 기능 FAIL이 아니라 시각 품질 후속 과제다.

### 2-3. 현재 런타임 판정 요약
- `P0-001 기본 주행`: PASS
- `P0-002 WheelSync`: PASS
- `P0-003 DriveState`: PASS
- 현재 기준선에서 가장 큰 남은 문제는 WheelSync 코어 FAIL이 아니라 고속 휠 시각 품질과 레거시 구조 정리다.
- DriveState 코어 자체는 현재 기준선에서 정상적으로 읽힌다.
- 디버그 가독성은 PASS지만 `Runtime` 줄이 너무 길어 보기 편한 형태로 재구성할 필요가 있다.

### 3. `DA_PoliceCar` 실측
- 시각 구성:
  - 차체는 `Combined_Body`
  - 바퀴는 `Wheel_FL / FR / RL / RR`
- 레이아웃:
  - 현재 기준선은 `Wheel_Anchor_*` 수동 배치를 사용한다.
- 레거시 `AutoFit` 필드는 현재 기준 런타임에서 사용하지 않으며, 현재 코드 기준에서는 하드 삭제됐다.
- 현재 코드 정리 기준:
  - `CFVehicleData` 안의 `VehicleLayoutConfig / WheelLayout / AutoFit` 필드는 제거됐다.
  - 현재 운영 기준선은 `수동 Wheel_Anchor 배치 + WheelRadius`만 사용한다.
- `PoliceCar` 수동 기준선 좌표:
    - `FL = (195.069249, -94.740381, 12.036834)`
    - `FR = (195.069249, 94.740381, 12.036834)`
  - `RL = (-162.369103, -94.740381, 12.036834)`
  - `RR = (-162.369103, 94.740381, 12.036834)`
- 물리 기준선:
  - 바퀴 파묻힘 이슈는 `WheelRadius` 조정으로 해결했다.
  - 현재 확인 기준:
    - `FrontWheelRadius = 43`
    - `RearWheelRadius = 43`
- 무브먼트:
  - `bUseMovementOverrides = true`
  - 하지만 현재 덤프 기준 세부 `b_override_*` 플래그는 모두 `false`다.
  - 즉, 오버라이드 게이트는 켜져 있지만 상세 수치 교체는 아직 비활성 상태다.
- 참조형 자산:
  - `FrontWheelClass = BP_Wheel_Front`
  - `RearWheelClass = BP_Wheel_Rear`
- DriveState:
  - `bUseDriveStateOverrides = true`
  - `bEnableDriveStateHysteresis = true`
  - `bUsePerStateHoldTimes = true`
  - `bTreatOppositeThrottleAsBrake = true`
  - `ActiveInputThreshold = 0.05`
  - `IdleEnter/Exit = 0.75 / 1.50`
  - `ReverseEnter/Exit = 1.25 / 0.75`
  - `AirborneMinSpeedThresholdKmh = 3.0`
  - `AirborneVerticalSpeedThresholdCmPerSec = 100.0`

### 4. `TestMap` 실측
- 맵 경로는 `/Game/Maps/TestMap`이다.
- 현재 덤프 기준 액터 수는 11개다.
- 기준 차량으로 `BP_CFVehiclePawn_C_2`가 배치되어 있다.
- `PlayerStart_0`가 함께 존재한다.
- 현재 맵은 매우 작은 기준 맵이며, 기본 주행 / 디버그 / 기준 차량 배치 확인에는 적합하지만 다양한 물리 상황 검증에는 아직 제한적일 수 있다.

---

## 임시 운영 편차
아래 항목은 현재 운영상의 편차이며, 구조 방향 자체를 바꾼 것으로 해석하지 않는다.

### 1. 입력 장치
- 현재 테스트는 게임패드 중심으로 고정되어 있다.
- 키보드 / 게임패드 전환 과정의 오류를 피하기 위한 임시 운영이다.

### 2. 테스트 차량 집중
- 현재 검증은 `DA_PoliceCar` 중심이다.
- 이는 첫 기준선 확보를 위한 선택이며, 최종 차종 구조를 고정한 것은 아니다.

---

## 현재 구조 갭으로 보는 항목
아래 항목은 임시 운영 편차가 아니라 실제 구조 갭이다.

### 1. 최종 구조 미도입
- `CMVS / Cluster Union / Geometry Collection` 기반 구조가 아직 현재 코어에 반영되지 않았다.

### 2. 기존 vehicle root 의존
- 현재 기준 Pawn은 여전히 `ChaosWheeledVehicleMovementComponent` 중심 구조를 사용한다.

### 3. 조립형 차량 파이프라인 미도입
- 데이터에서 차량을 조립하고 재구성하는 최종 파이프라인은 아직 현재 구현 기준선이 아니다.

---

## 현재 리스크
### 1. 코어가 현 구조에 너무 고정될 위험
- `DriveState`와 `WheelSync`가 현 vehicle root 구조에 너무 강하게 묶이면 나중에 구조 전환 비용이 커질 수 있다.

### 2. 테스트 차량이 기준선에서 종착점으로 굳어질 위험
- `DA_PoliceCar` 기준 검증이 길어질수록 다차종 확장 관점이 약해질 수 있다.

### 3. 문서와 구현이 다시 분리될 위험
- 방향 문서와 현재 기준 문서가 다시 섞이면, 어떤 작업이 임시 대응이고 어떤 작업이 구조 전환 준비인지 흐려질 수 있다.

---

## 검증 기준
- 현재 기준선 PASS / FAIL 판정은 `05_TestChecklist.md`를 기준으로 본다.
- 구조 유지 / 교체 판단은 `VehicleCoreDecisions.md`에 남긴다.
- 실제 작업 순서는 `02_Roadmap.md`를 기준으로 진행한다.

---

## 후속 세션에서 먼저 볼 순서
1. `00_Vision.md`
2. `01_ProjectState.md`
3. `02_Roadmap.md`
4. `05_TestChecklist.md`
5. `VehicleCoreDecisions.md`

---

## 현재 카메라 기준선 (2026-04-15 추가)
아래 항목은 현재 카메라 작업의 실제 기준선을 별도 정리한 메모다.
이 섹션은 최종 카메라 철학 전체를 설명하는 문서가 아니라, **현재 프로젝트에서 실제로 붙어 있는 카메라 기준선**만 기록한다.

### 1. 현재 구현 기준
- 현재 카메라 기준 플레이 Pawn은 `BP_CFVehiclePawn`이다.
- 현재 카메라 Native 코어는 `UCFVehicleCameraComp`다.
- 현재 카메라 공용 타입 / 데이터 구조는 아래 파일 기준으로 본다.
  - `CFVehicleCameraTypes.h`
  - `CFVehicleCameraData.h`
  - `CFVehicleCameraComp.h / .cpp`
- 현재 Pawn 입력 연동은 `ACFVehiclePawn`의 `InputAction_Look` 경로를 기준으로 한다.

### 2. 현재 BP 기준 카메라 계층
- 현재 기준 계층은 아래 이름을 기준으로 한다.
  - `CameraPivotRoot`
  - `CameraAimPivot`
  - `CameraBoom`
  - `FollowCamera`
- 현재 방향은 차량 중심 수평 피벗 기준 자유 조준 카메라다.
- 현재 카메라는 일반 레이싱 chase camera보다 차량 기반 3인칭 슈팅 카메라 성격에 가깝다.

### 3. 현재 입력 기준
- 현재 Look 입력 자산 기준은 `IA_LookAround`다.
- 현재 `IA_LookAround`는 `Axis2D` 기준으로 사용한다.
- 현재 `IMC_Vehicle_Default`에서는 마우스 2D 입력과 게임패드 오른쪽 썸스틱 2D축 입력이 정상 동작 기준선이다.
- `ACFVehiclePawn` 생성자 내부 입력 자산 경로는 하드 고정값이 아니라 fallback로만 남기고, BP/파생 클래스 지정값을 우선한다.

### 4. 현재 확인 완료 항목
- 빌드 성공 확인 완료
- 카메라 Yaw 회전 확인 완료
- 카메라 Pitch 회전 확인 완료
- SpringArm 기반 외부 3인칭 거리 계산 경로 반영 완료
- FOV 반영 경로 반영 완료
- Aim Trace 계산 경로 반영 완료

### 5. 현재 범위 한계
- 현재는 기본 카메라 움직임과 뼈대가 구현된 상태다.
- 아직 HUD / 조준점 / 진행 방향 힌트는 미구현이다.
- 아직 무기 시스템의 실제 Aim Profile 공급과 무기별 제한각 연동은 미구현이다.
- 따라서 현재 카메라 기준선은 **기본 회전과 시스템 골격 확보 단계**로 본다.

---

## 변경 이력

### v2.43.42 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-02 Browser·Mesh-only·Create·Existing Import`을 사용자 직접 확인으로 USER PASS 처리했다.
- Paused 상태는 유지하면서 보존 checkpoint를 `USER PASS 2 / 다음 UA-03 Recipe·Shared Profile·Affected Vehicle Impact`로 전진시켰다.
- 대표 Plan/Roadmap projection을 `DataAuthoringPlan.md v0.2.24 / DataAuthoringRoadmap.md v0.1.32`로 갱신했다. DG/DEL/Wizard deletion은 계속 미개방이다.

### v2.43.41 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-01 Workspace First Impression`을 사용자 직접 재확인으로 USER PASS 처리했다.
- Paused 상태는 유지하되 보존 체크포인트를 `USER PASS 1 / 다음 UA-02 Browser·Mesh-only·Create·Existing Import`로 전진시켰다.
- 대표 Plan/Roadmap projection을 `DataAuthoringPlan.md v0.2.20 / DataAuthoringRoadmap.md v0.1.28`로 갱신했다. DG/DEL/Wizard deletion은 계속 미개방이다.

### v2.43.40 - 2026-08-18

- 사용자 우선순위 변경에 따라 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`를 현재 단일 Active로 복원했다.
- 즉시 재개 지점은 `InGameUIPlan.md v0.58.6 / UI-P0-03 Defense Production Panel USER Visual → Pawn Rebind USER Visual`이며 기존 UI-P0-02/03 기술·USER PASS는 반복하지 않는다.
- `CF-FQ-038`은 P0-08A~M/P0-09~11 Technical PASS와 `DAUTH-P0-12 USER PASS 0 / UA-01` 상태를 보존한 Paused로 전환했다. DG/DEL Gate는 계속 미개방이다.
- `CF-FQ-037 Scanner` 완료로 `SensorContact.md v1.1.0`이 TargetPanel/Radar 후속 UI의 Current Sensor source가 됐지만 Radar Range/Zoom·동적 Blip·USER Visual은 아직 Pending이다.

Migration: 새 세션은 CF-FQ-032 UI를 우선한다. DAUTH는 `DataAuthoringPlan.md v0.2.17 / DataAuthoringRoadmap.md v0.1.25 / P0-12 UA-01`에서 보존하며 사용자가 재지정하기 전 자동 재개하지 않는다.

### v2.43.39 - 2026-08-18


- `CF-FQ-038 / DAUTH-P0-12 USER Authoring Acceptance`를 실제 사용자 검증 단계로 착수했다.
- 대표 Plan `DataAuthoringPlan.md v0.2.16`, Roadmap `DataAuthoringRoadmap.md v0.1.24`의 UA-01~08을 current USER checklist로 연결했다.
- P0-08A~M/P0-09~11 Technical PASS는 반복하지 않으며 현재 USER PASS는 0이다.
- fresh Editor Ready는 확보했지만 Slate UI live inspection은 current GoPyMCP server policy에 의해 차단되므로 UI/사용성 PASS는 사용자 직접 판정만 사용한다.
- DG/DEL Gate와 `SCFVDAWizardTab` 삭제는 P0-12 완료 전까지 미개방 상태를 유지한다.

### v2.43.38 - 2026-08-18

- `CF-FQ-037 / SCAN-P0-06 USER PIE Acceptance`를 사용자 확인으로 PASS 처리하고 `SCAN-P0-07 Current System Integration`까지 완료해 Scanner를 Done으로 전환했다.
- 초기 USER FAIL은 Scanner fixture가 실제 GameMode DefaultPawn이 아니라 배치 Pawn에 연결된 것이 원인이었으며, test-only `BP_ScanPlayerPawn` + `BP_ScanGameMode` + `TestMap_ScannerP0` override로 교정했다.
- USER PIE에서 V 단발 5초 Active Scan, 자동 종료, 활성 중 반복 입력 무연장과 선택 Target의 `???` 해제를 확인했다. exact scanner-less False/0은 USER 관측으로 꾸미지 않고 기존 기술 회귀 evidence로 보호한다.
- USER Acceptance용 임시 관측 코드는 제거했고 final Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 closure evidence로 고정했다. 현재 구현 owner는 `Systems/Targeting/SensorContact.md v1.1.0`이다.
- Radar Range/Zoom·동적 Blip·Radar/TargetPanel USER Visual, Sensor energy/heat·AI/Network는 완료 범위에 포함하지 않았다.

### v2.43.37 - 2026-08-18


- `CF-FQ-038 / DAUTH-P0-11 Frozen UX Completeness Closure`를 완료해 P0-11 overall을 Technical PASS로 반영했다.
- Handling/Performance Adoption, Shared Profile B2 edit+affected impact, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh를 Common Authoring facade와 existing Core 재사용으로 구현했다.
- Drift prospective duplicate의 RecipeId 재발급으로 approval hash가 비결정적이던 defect를 원본 RecipeId 복원으로 교정했으며 approval binding은 약화하지 않았다.
- final Build `98dfceab797942218bd09c844e398ceb` PASS, focused P0-11 `b53998e6cacf48a1a554d784b77e013c` 6/6 PASS, full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS를 확보했다.
- full result JSON SHA-256은 `a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f`다.
- Runtime `UCFVehicleData`, Inventory/Fitting production source, Content Asset, Wizard 삭제, DG/DEL은 변경/개방하지 않았다. P0-12는 dependency상 Ready지만 이번 작업에서는 착수하지 않았다.

### v2.43.36 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-11` core technical validation을 PASS로 확인했다. representative E2E와 applied VehicleData Inventory/Fitting consumer regression을 추가했고 full Data Authoring 59/59 PASS를 확보했다.
- `AppliedState.AppliedDefinitionHash`가 full Current Definition hash가 아니라 Resolver-owned projection의 `ResolvedDefinitionHash`라는 P0-08H 계약을 재확인했으며 production hash 의미는 변경하지 않았다.
- Frozen 24.90~24.94 전체 workflow audit에서 normal Workspace Handling/Performance Adoption, Shared Profile edit/impact, Drift 3-way recovery, Mesh-only Create flow가 미구현임을 확인했다.
- P0-08/P0-09/P0-10 scoped Technical PASS는 유지하지만 P0-11 overall은 In Progress, P0-12는 Not Ready로 유지한다. Wizard 삭제/DG/DEL도 열지 않았다.
- 다음 착수는 `DataAuthoringPlan.md v0.2.14 / DataAuthoringRoadmap.md v0.1.22 / DAUTH-P0-11 Frozen UX Completeness Closure`다.

Migration: Section 14/24.98 검증을 반복하지 않고 확인된 24.90~24.94 production gap을 먼저 닫는다.

### v2.43.35 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-10 Existing Wizard Migration`을 Technical PASS로 Current 기준선에 반영했다.
- 새 Workspace에 Reference Compare, Assets/Layout semantic migration, Measurement/Adoption review, Frozen 4축 Driving Feel/preset, Stable-ID Mount/Defaults와 standard Undo를 parity했다.
- legacy Wizard는 managed Target의 세 변경 동작을 비활성화하되 read-only 기능과 unmanaged path를 유지한다. Wizard 삭제/DG/DEL은 아직 아니다.
- final Build `03fa78af4e124a3db33893ca8bfe436d` PASS, focused P0-10 4/4, full Data Authoring 57/57 PASS, result SHA `90c01eda8b1df96f03d55feb1e3000f99db16f8a717baa697f5c15cdb9f6884b`를 확인했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset과 Batch main page는 변경하지 않았다. USER UX/Driving Feel PASS는 P0-12까지 미판정이다.
- 다음 Gate는 `DAUTH-P0-11 Technical Validation`이다.

Migration: P0-08A~M/P0-09/P0-10을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.21 / DAUTH-P0-11`에서 재개한다.

### v2.43.34 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-09 Vehicle Authoring MVP`를 Technical PASS로 current 기준선에 반영했다.
- 새 `CarFight.VehicleAuthoring` Nomad Workspace에 Vehicle Browser/Selection, Initial Import, Recipe basic intent, Resolver Preview, Pending Diff, Source Trace, Validation, shared Apply/standard Undo, Raw DA Open을 구현했다.
- UI/ViewModel은 Common Authoring facade만 사용하고 Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset을 변경하지 않았다. 기존 `CarFight.VehicleDAWizard` / `SCFVDAWizardTab`도 그대로 보존했다.
- final Build `76fc476c8ccb4daf895a3b567fe0c992` PASS, focused P0-09 6/6, full Data Authoring 53/53 PASS, result SHA `697230dbce1c9a6281a0e56c4aa292544d3becd28b701a25285a487a50592654`를 확인했다.
- Apply/Undo는 existing `FCFVehicleApplyService` transaction + Unreal standard Undo를 실제 Automation으로 검증했고 stale approval은 Target mutation0으로 차단한다.
- P0-09 Technical PASS를 P0-12 USER 시각/사용성 PASS로 확대하지 않는다.
- 다음 Gate는 `DAUTH-P0-10 Existing Wizard Migration`이다. P0-10 parity 전 Wizard 삭제 금지를 유지한다.

Migration: P0-08A~M/P0-09를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.20 / DAUTH-P0-10`에서 재개한다.

### v2.43.33 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08M`을 Technical PASS로 current 기준선에 반영하고 `DAUTH-P0-08 Implementation Foundation` 전체를 Technical PASS로 닫았다.
- per-target fresh R3 evidence, exact B3 approval, all-target global preflight, canonical TargetPath 순서의 `FCFVehicleApplyService` exact-once 호출과 Stop-On-First-Failure partial result를 구현했다.
- NoChange/ShadowOnly/ExternalDrift/Resolve Error·Blocked/Validation Blocked는 ineligible이다. Batch global atomic/rollback, already-applied rollback, retry/save는 0이다.
- official Build `10027d18360e48f399a5b439f280c24c` PASS, focused M `a49d21b6b9b94d648142d5a555a79c95` 4/4 PASS, targeted `cb2ae69e9c834657a553fe52c00f5a96` 47/47 PASS, result SHA `472ef5bb8f5fdeb46ebd1b7e68f1fc5a66acba065828ff025167b475aaf25595`를 확인했다.
- Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader 보호 범위를 유지했고 UI/file save는 0이다.
- Frozen 26.66~26.69 idempotency/generic envelope는 M의 dependency가 아니므로 첫 실제 external Batch transport/client integration 전 follow-up으로 보존한다.
- `CF-FQ-038`은 계속 Active이며 다음 Gate는 `DAUTH-P0-09 Vehicle Authoring MVP`다.

Migration: P0-08A~M과 P0-08 Foundation을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.19 / DAUTH-P0-09`에서 재개한다.

### v2.43.32 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08L`을 Technical PASS로 current 기준선에 반영했다.
- exact Batch approval binding, all-source global TOCTOU preflight, B1 Recipe/B2 shared Profile one-transaction source commit, all-or-nothing rollback과 success 후 approval invalidation을 구현했다.
- official Build `2e2923f31d5a4f3fa703f3b7623c0741` PASS, targeted process `3853d42dfb344bfd8eb6e516903a8d78` 43/43 PASS, result JSON SHA-256 `eef2051ec4b3ccb5dce10b8b76761299885d18a8bf7f2c9db42cc2967320abcf`를 확인했다.
- External Drift는 source commit blocker가 아니며 B1/B2는 Target을 변경하지 않는다. Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader 보호 범위를 유지했고 B3/UI/file save도 0이다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 Frozen 26.54~26.65의 `DAUTH-P0-08M B3 Batch Definition Apply Foundation`이다.

Migration: P0-08A~L을 반복하지 않고 `DataAuthoringRoadmap.md v0.1.18 / P0-08M`에서 재개한다. `P0-08M`은 Frozen B3 구간에 새로 붙인 implementation checkpoint label이다.

### v2.43.31 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08K`를 Technical PASS로 현재 기준선에 반영했다.
- current Unreal reread 기반 Batch 3-way preview와 ownership conflict, transient Recipe/Profile prospective Resolver preview, deterministic BatchPlanHash를 구현했다.
- official Build `a3d37d4c8520442d8ceb09a72eb6a68f` PASS와 targeted process `7d2db5a9baa54dd9abf25857138d3994` 37/37 PASS, result JSON SHA-256 `bbdf497d472e5ec2fbe4458e075907897ca79b2ac984d43ba3290cff3c55eeec`를 확인했다.
- persistent Batch source/Target mutation, B1/B2 source commit, B3 Apply, UI, file save는 아직 0이며 Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader 보호 범위를 유지했다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 Frozen Section 26.44~26.53의 `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E/F/G/H/I/J/K를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.17 / P0-08L`에서 재개한다. Section 26.54+ B3는 별도 후속이다.

### v2.43.30 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08J`를 Technical PASS로 현재 기준선에 반영했다.
- existing Recipe/Profile typed schema와 117 Field Registry projection 기반 Batch Column Registry, Recipe 7 + Profile 78 numeric allowlist, reserved `__cf_`, stable ColumnId를 구현했다.
- canonical BOM-less UTF-8 CSV와 immutable-style `.cfbatch.json` baseline manifest/ExportSetHash를 구현하고 source-mode baseline/manifest consistency를 보존했다.
- official Build `3249098c1b99487a8fd173694573bd5e` PASS와 targeted process `534486583a1d4689bc5137505a03f806` 32/32 PASS, result JSON SHA-256 `84fc15de1ad23b4e13891d91db09fa3668a49c900a438dec17b70d818a45d36f`를 확인했다.
- Spreadsheet/CSV는 새 Source Type/SSOT가 아니며 Import Session/B1·B2/B3/UI/file save/persistent Batch mutation은 아직 0이다.
- 보호 Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader 범위를 유지했다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E/F/G/H/I/J를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.16 / P0-08K`에서 재개한다.

### v2.43.29 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- 공통 `FCFVehicleAuthoringService`는 기존 Snapshot/AssetReader/Pure Resolver/ApplyService를 orchestration하며 R0 read, prospective R1 preview, guarded Recipe semantic write와 R3 shared Apply entry를 제공한다.
- exact R1/R3 approval scope, expected Recipe/Target/revision, ExpectedDiffHash, typed result/error/mutation footprint와 bounded ClientOperationId dedupe를 적용했고 automatic retry/save는 0이다.
- Section 22.27 Frozen RecipeFingerprint 밖의 `VehicleArchetypeId`는 hash 계약을 변경하지 않고 operation-specific desired-state equality로 NoChange/commit을 판정하도록 교정했다.
- source audit에서 Raw SetField, FieldCodec ImportValue, direct Target mutation, force/skip-validation, SavePackage는 0이고 `FCFVehicleApplyService::Apply` actual call은 1개다.
- official Build `252fdbd097f943379f2a1e2a942bedef` PASS와 targeted process `f2011a206c964fadb269d13c4dba3c86` 27/27 PASS, result JSON SHA-256 `b9bfea5be21f1bcc521e0bcfa8b8a43c017e18a4dec65a208bfd57a68805bfbf`를 확인했다.
- Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader/UI/CSV/Batch 보호 범위를 유지했다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E/F/G/H/I를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.15 / P0-08J`에서 재개한다.

### v2.43.28 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08H Apply Transaction Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- `FCFVehicleApplyService`에 five TOCTOU precondition, fresh Resolve/Diff revalidation, transient duplicate preflight, dependency-safe Stable-ID/leaf write를 중앙화했다.
- persistent Apply 전후로 Resolver-owned Target readback hash와 existing `UCFVDAValidator`를 검사하고 성공 시 `FCFVehicleAppliedState` field traces를 갱신한다.
- transaction 실패는 Target transient backup, Recipe AppliedState, package dirty state를 자체 복원하고 transaction cancel + full Target hash 재검증으로 partial success를 차단한다.
- official Build `b88831b71797415fac9dc0a23d12c39e` PASS와 targeted process `6f261c0b76f04d0faa6b9e855865818e` 23/23 PASS를 확인했다.
- production SavePackage 호출 0, `FCFVehicleApplyService` 외 product Target writer 추가 0이며 Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader와 UI/CSV/Batch는 변경하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E/F/G/H를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.14 / P0-08I`에서 재개한다.

### v2.43.27 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08G Existing Definition Import / Adoption Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- Existing Definition exact field 전체를 lossless Legacy baseline으로 보존하고 Mount hidden serialized field를 별도 passthrough로 분리하며 direct semantic candidate copy를 Recipe에 연결했다.
- Initial Import ownership은 Legacy Pin이 유지하고 Movement raw → Driving Feel/Profile inverse inference 및 automatic Profile binding은 금지 상태를 유지한다.
- group/field Adoption Preview는 virtual pin release만 수행하고 approved Commit은 fresh Recipe fingerprint precondition 아래 Recipe-only transaction으로 처리한다.
- official Build `b201d87cd13f4987a0907e08c8f00a6c` PASS와 targeted process `3b52a251ab244096b78bb872a06c0069` 20/20 PASS를 확인했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader, existing Validator 변경은 0이고 Target Definition mutation / Apply/UI/CSV도 구현하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 `DAUTH-P0-08H Apply Transaction Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E/F/G를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.13 / P0-08H`에서 재개한다.

### v2.43.26 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08F Definition Materializer / Validation Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- transient `UCFVehicleData` materialization, Stable-ID Hardpoint/Mount reconstruction, FieldCodec checked import, Resolver-owned readback hash와 기존 `UCFVDAValidator` DefinitionValidation orchestration을 구현했다.
- R15은 더 이상 Deferred가 아니며 materialization 자체 실패는 Error, Definition validation Error/Blocked는 preview-preserving Blocked로 분리한다.
- official Build `e5d925c6531e4ae6ac58aa356e4078eb` PASS와 targeted process `4b21d25b780c4a398de7a1b47c595c5e` 17/17 PASS를 확인했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset, Asset Reader와 기존 Runtime Validator source 변경은 0이며 Apply/UI/CSV는 구현하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress이며 다음 Gate는 `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`이다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E/F를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.12 / P0-08G`에서 재개한다.

### v2.43.25 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08E Pure Resolver Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- Snapshot-only Request/Result, Frozen R0~R16, deterministic precedence, Measurement Proposal/Adoption, Source Trace/Hash, R14 Diff, R16 Stale/Drift foundation을 구현했다.
- R15 transient Definition Materializer + existing `UCFVDAValidator` integration은 다음 `DAUTH-P0-08F Definition Materializer / Validation Foundation`으로 남겼다.
- official Build `db202f0797af41fe86a859609cb4dfd9` PASS, targeted process `93d445f78cbe4eaabca1478eb6d27078` 15/15 PASS를 확인했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset과 Asset Snapshot Reader 변경은 0이며 Apply / UI / CSV도 이번 Gate에서 구현하지 않았다.
- `DAUTH-P0-08`은 계속 In Progress다.

Migration: CF-FQ-038은 P0-08A/B/C/D/E를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.11 / P0-08F`에서 재개한다.

### v2.43.24 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08D Asset Snapshot Reader Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- Chassis requested socket transform facts, Wheel local bounds와 resolver-relevant fingerprint를 live UObject와 분리한 Asset Snapshot으로 구현했다.
- official Build `780d30c4a44f48feb179f7f6da5480e1` PASS, targeted process `ab2cd64308484ce0ae0e24cb1143ca33` 9/9 PASS를 확인했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset 변경은 0이고 Resolver / Apply / UI / CSV도 이번 Gate에서 구현하지 않았다.
- 다음 Gate를 `DAUTH-P0-08E Pure Resolver Foundation`으로 이동했다.

Migration: CF-FQ-038은 P0-08A/B/C/D를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.10 / P0-08E`에서 재개한다.

### v2.43.23 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08C Immutable Snapshot Foundation`을 Technical PASS로 현재 기준선에 반영했다.
- Recipe / 5 Profile / Current Definition / Project Compatibility Default Snapshot, Stable-ID exact field expansion과 deterministic fingerprint/hash를 구현했다.
- Frozen Section 22.17 `RequiredDependencies`를 Registry에 채우고 final official Build `6c81b2149de44d00a99554a44d2135aa` PASS, targeted process `340050d2c911445da3634ebb92acc014` 8/8 PASS를 확인했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset 변경은 0이고 Asset Snapshot Reader / Resolver / Apply / UI / CSV도 이번 Gate에서 구현하지 않았다.
- 다음 Gate를 `DAUTH-P0-08D Asset Snapshot Reader Foundation`으로 이동했다.

Migration: CF-FQ-038은 P0-08A/B/C를 반복하지 않고 `DataAuthoringRoadmap.md v0.1.9 / P0-08D`에서 재개한다.

### v2.43.22 - 2026-08-17

- `CF-FQ-038 차량 데이터 Authoring 시스템`을 현재 단일 Active로 반영하고 대표 Plan을 `DataAuthoringPlan.md v0.2.0`으로 연결했다.
- `DAUTH-P0-08A/B` 첫 Foundation slice는 official Build `5c745a31d19b448d9b1049877bc877ca` PASS와 targeted Automation `ab9de3e8c8cd410a8de0641178690dff` 4/4 PASS로 Technical PASS다.
- Editor-only Recipe + 5 Profile, Never-Cook, Stable Field Path / Field Value Codec, `UCFVehicleData` 117 Registry coverage가 구현됐다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset은 변경하지 않았다.
- `CF-FQ-034`는 `FIT-P0-07D USER Driving Feel Comparison`을 보존한 Paused로 이동했고 USER 주행감 PASS를 추정하지 않았다.
- 다음 Gate는 `DAUTH-P0-08C Immutable Snapshot Foundation`이다.

### v2.43.21 - 2026-08-17

- CF-FQ-034 `FIT-P0-07C Quantitative Mobility Measurement`을 Technical Complete로 현재 기준선에 반영했다.
- 같은 공식 Light/Default/Heavy를 fixture별 fresh PIE lifetime에서 계측해 0→30, 30→0.75km/h 제동거리와 Steering 0.5·2초 coast 누적 Yaw baseline을 확보했다.
- final official Build `bb04d56e0cd24647b2ef6fcbc7f2bd77` PASS, Quantitative process `63bbeaf0202041b79dac8f32f5447923` Success / Metric 3/3을 현재 evidence로 연결했다.
- 가속·coast-steering 결과가 단순 질량 순서로 단조 변화하지 않았지만 임의 합격 기준·자동 Mobility Scalar·새 질량값을 추가하지 않았다.
- USER 주행감은 Pending으로 유지하고 다음 Gate를 `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D USER Driving Feel Comparison`으로 이동했다.

Migration: CF-FQ-034는 FIT-P0-07A~07C와 기존 Fitting 23/23을 반복하지 않는다. 같은 공식 세 Fixture를 직접 주행 비교한 뒤에만 PhysicalMassOnly 유지 또는 후속 Mobility Adapter 필요성을 판단한다.

### v2.43.20 - 2026-08-17

- CF-FQ-034 `FIT-P0-07B Heavy Payload Resolution + Official Fixture Preparation`을 Technical Complete로 현재 기준선에 반영했다.
- 같은 `DA_VehicleDefense_TestSUV` 플랫폼에서 Light 1000kg / Default 1570kg / Heavy 1600kg 공식 Fixture 3종의 persisted 계약을 고정했다.
- `verify_existing` process `c33a36b21a8f4079b38bba7c51e12864`, fresh AssetDump dataset `adset_v1_b60bd590730faab54910f1bbc42fdac4.fe0d1a67bac9a357bc81385a`, post-label `OfficialMobilityFixtures` `5ab277165e9d4aef9e05b779106c808f` 1/1 PASS를 현재 증거로 연결했다.
- Production 질량값을 새로 만들거나 변경하지 않았고 USER PIE·USER 주행감은 Pending으로 유지했다.
- 다음 원격 기술 Gate를 `VehicleFittingPlan.md v0.16.0 / FIT-P0-07C Quantitative Mobility Measurement`로 이동했다.

Migration: CF-FQ-034는 FIT-P0-07A~07B를 반복하지 않고 `FIT-P0-07C`에서 공인 세 Fixture를 그대로 사용한다. 사용자 주행감과 Field UI는 기술 계측과 분리한다.

### v2.43.19 - 2026-08-17

- 사용자 선택에 따라 CF-FQ-034를 현재 단일 Active로 전환했다.
- FIT-P0-07A Fixture Readiness Audit에서 차량 자산을 Visual-only / VehicleData baseline / Fitting-ready technical platform으로 분류했다.
- DA_TestSedan·DA_TestSUV는 Base/Gross 0kg라 현재 공식 Fitting Fixture로 사용하지 않고, `DA_VehicleDefense_TestSUV` 1000/2500kg를 동일 플랫폼 기준으로 고정했다.
- Light 1000kg과 Default 1570kg은 임의 질량 없이 준비 가능하며 Heavy는 same-platform persisted payload 확인을 FIT-P0-07B로 남겼다.
- Source·Content Asset·Build·USER PIE는 변경하거나 실행하지 않았다.

Migration: CF-FQ-034는 `VehicleFittingPlan.md v0.15.0 / FIT-P0-07B`에서 재개한다. 메시-only 차량은 VehicleData 계약이 생기기 전 Mobility Fixture 대상에서 제외한다.

### v2.43.18 - 2026-08-17

- CF-FQ-029 LM-P0-06A Technical PASS 이후 남아 있던 stale Active projection을 교정하고 현재 단일 Active를 없음으로 동기화했다.
- CF-FQ-030 Persisted Missile Test Asset Technical Verification PASS와 `MissileGuidancePlan.md v0.5.0`을 현재 상태에 반영했다.
- fresh DirectRuntimeContract `29d0ef16dd5e4d56945ceae26eec6937` 1/1, Missile folder AssetDump 20/20, Maps World 12/12 PASS와 4개 계약 DataAsset 저장 참조 체인을 기록했다.
- MissileDirectTest는 33 Actor와 Missile test vehicle/RocketLauncher socket을 확인했지만 전체 Actor label은 current public readback 범위 밖이므로 다섯 MissileTarget label 자체를 PASS로 확대하지 않았다.
- Runtime Source·Content Asset·Blueprint·Map 저장 변경과 USER PIE는 0이며 CF-FQ-030은 Ready / CF-TC-027 Manual PIE Pending을 유지한다.

Migration: CF-FQ-030 재개 시 `MissileGuidancePlan.md v0.5.0 / CF-TC-027 Manual PIE`에서 시작한다. CF-FQ-029는 `LauncherMissilePlan.md v0.14.0 / LM-P0-06 USER PIE`, CF-FQ-037은 `ScannerIntegrationPlan.md v0.7.1 / SCAN-P0-06 USER PIE`를 보존한다.

### v2.43.17 - 2026-08-16

- 현재 단일 Active를 CF-FQ-037 Scanner에서 CF-FQ-029 Launcher로 전환했다.
- CF-FQ-029의 next gate를 `LM-P0-06A Failure Policy Technical Closure`로 고정하고 asset-free Automation 범위와 USER PIE 보호 경계를 기록했다.
- CF-FQ-037은 `SCAN-P0-06 USER PIE` 체크포인트와 P0-06A fixture readiness를 보존한 Paused로 유지한다.

Migration: 새 세션은 `LauncherMissilePlan.md v0.13.0`에서 시작한다. Scanner는 화요일 USER PIE 가능 시 `ScannerIntegrationPlan.md v0.7.1`에서 복원한다.

### v2.43.16 - 2026-08-16

- CF-FQ-037 Current projection을 `ScannerIntegrationPlan.md v0.7.1`로 동기화했다.
- P0-06A Scanner 전용 fixture readiness 완료와 USER PIE 미실행 상태를 현재 프로젝트 기준선에 반영했다.
- P0-06은 실제 `V` 입력·5초 timed scan·반복 입력 무연장·scanner-less safe reject·TargetSelect/HUD 기본 비회귀 사용자 확인만 남긴다.
- P0-04/05 기술 증거는 Source 변경이 없는 한 반복하지 않고 P0-07 자동 승격도 계속 금지한다.

Migration: CF-FQ-037 재개 시 `ScannerIntegrationPlan.md v0.7.1`의 P0-06 USER PIE에서 복원한다.

### v2.43.15 - 2026-08-16

- CF-FQ-037 Current projection을 `ScannerIntegrationPlan.md v0.7.0 / SCAN-P0-06 USER PIE Acceptance`로 교정했다.
- `SCAN-P0-00~05` Technical Done, P0-04 final Build/회귀와 P0-05 fresh TargetSelect 보호 회귀를 현재 상태에 연결했다.
- 사용자 직접 PIE 전에는 P0-06 USER PASS와 P0-07 Current System Integration을 자동 승격하지 않도록 재개 기준을 교정했다.

Migration: CF-FQ-037 재개 시 더 이상 P0-00 Foundation Audit을 다시 시작하지 않고 `ScannerIntegrationPlan.md v0.7.0`의 P0-06에서 복원한다.
- v2.43.13 (2026-08-15)
  - `CF-FQ-036 SEN-P0-07 Technical Acceptance`를 최종 Source 책임 경계 감사 결과 PASS로 판정했다.
  - public `FCFSensorContact / FCFSensorSnapshot` actor/UObject pointer 0, TargetSelect→Sensor 의존 없음, Sensor→TargetSelect 선택 mutation 없음, HUD의 TargetSelectable 원본 metadata·Actor 현재 위치·Sensor range 재판정 경로 없음과 VehicleHealth Destroyed 즉시 clear/독립 DestroyedHold를 확인했다.
  - 기존 official Build `7dff9da7aaa24e76b0871348762c9d93` PASS, `CarFight.Sensor` `85d008613a104df9b7107795995c6ac5` 14/14 PASS와 UI asset-free 1/1+1/1 PASS를 Acceptance evidence로 재사용했다. Source 의미 변경이 없어 Build/Automation을 반복하지 않았다.
  - `Document/Systems/Targeting/SensorContact.md v1.0.0`을 Current System으로 승격하고 `CF-FQ-036`을 Done으로 전환했다.
  - 현재 단일 Active를 비웠으며 Paused/Ready 기능은 사용자 선택 없이 자동 승격하지 않는다.
  - CF-FQ-032 Radar/TargetPanel USER Visual, Radar Range/Zoom·동적 Blip, CF-FQ-026 TS-P0-08 USER PIE와 다른 USER Pending은 완료로 추정하지 않았다.

- v2.43.12 (2026-08-15)
  - CF-FQ-036 `SEN-P0-06 Public Snapshot Integration`을 Technical Done으로 현재 상태에 반영했다.
  - TargetSelect는 선택 존재·유효성·TrackState owner를 유지하고 Target Knowledge/Radar Contact는 actor-free `FCFSensorSnapshot`을 Player-facing source로 사용하는 HUD Provider/ViewData 연결을 적용했다.
  - Detected identity 비누출을 보존하는 Actor→ContactId read-only bridge와 Snapshot 기반 상대 위치·거리 ViewData를 추가했으며 Radar Range/Zoom/normalized 위치는 임의 추정하지 않는다.
  - 기존 Radar 정적 placeholder Canvas는 실제 Sensor Blip이 아니므로 계속 숨기고 Blueprint/Content Asset은 변경하지 않았다.
  - 최종 official Build `7dff9da7aaa24e76b0871348762c9d93` PASS와 `CarFight.Sensor` Process `85d008613a104df9b7107795995c6ac5` 14/14 PASS를 최신 기술 기준선으로 갱신했다.
  - 관련 UI asset-free `ViewDataAvailability` `67d114255ec943bb8ded2642a40a581c`, `ProviderPawnRebind` `395c93df5fb947a1a979ca8fa0c74665` 각각 1/1 PASS를 보호 회귀로 기록했다.
  - `SEN-P0-06`을 Technical Done으로 승격하고 대표 Plan을 `SensorContactPlan.md v0.8.0`, current gate를 `SEN-P0-07 Technical Acceptance`로 이동했다.
  - CF-FQ-032/026 USER Pending, WBP_TargetSelect, Content Asset과 Project Config는 변경하지 않았다.

- v2.43.11 (2026-08-15)
  - CF-FQ-036 `SEN-P0-05 Destroyed Contact`의 destruction truth owner를 `UCFVehicleHealthComp`로 확정하고 Sensor가 `OnVehicleDestroyed / IsDestroyed`만 독립 소비하는 현재 구현을 반영했다.
  - 같은 ContactId·Knowledge·AnalysisProgress·마지막 신뢰 위치를 보존한 DestroyedHold와 bounded cursor 독립 `DestroyedHoldTimeSec` 만료를 현재 Sensor lifecycle에 추가했다.
  - weak Actor invalid/Actor Destroy 자체는 파괴 확정이 아니며 기존 LastKnown→Lost를 유지하고, 미관측 pre-destroyed Actor에서 새 Destroyed Contact를 만들지 않는 계약을 기록했다.
  - official Build `954dc0ab844b40f48f2674f2a0ae672a` PASS와 `CarFight.Sensor` Process `4ba274cc90814e5b90d40c33820b3bd0` 13/13 PASS를 최신 기술 기준선으로 갱신했다.
  - `SEN-P0-05`를 Technical Done으로 승격하고 대표 Plan을 `SensorContactPlan.md v0.7.0`, current gate를 `SEN-P0-06 Public Snapshot Integration`으로 이동했다.
  - TargetSelect의 Destroyed 즉시 clear, HUD, CFVehiclePawn, Project Config, Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.

- v2.43.10 (2026-08-15)
  - 사용자 Editor 종료 후 official UE 5.8 Build `786e47df68fe43f9a924c9c6fbd86726`이 NetCore/CarFight_Re DLL Link까지 Exit Code 0으로 PASS했다.
  - 새 linked binary에서 `CarFight.Sensor` Process `6fcb3d2dbd5f42489febfff6040611af`가 11 Success / 0 Failure로 종료됐다.
  - P0-01~03 기존 8개와 P0-04 ActiveRange/AnalysisDecay/AnalysisProgress 3개가 모두 PASS했고 TargetSelect channel/HUD/selection/InputAction/DestroyedHold 금지 경계도 0으로 재확인했다.
  - `SEN-P0-04 Active Scan Analysis`를 Technical Done으로 승격하고 대표 Plan을 `SensorContactPlan.md v0.6.0`, current gate를 `SEN-P0-05 Destroyed Contact`로 이동했다.
  - TargetSelect/HUD/CFVehiclePawn/Project Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.

- v2.43.9 (2026-08-15)
  - CF-FQ-036 `SEN-P0-04 Active Scan Analysis` Input/Scanner owner 감사를 완료하고 Sensor를 InputAction owner가 아닌 Active command·Detection·Analysis·Knowledge owner로 고정했다.
  - `StartActiveScan / StopActiveScan`, ActiveScanRange 장거리 전방향 Contact Detection, ECC_Visibility 기반 Tactical Analysis gain, invalid 상태 decay와 Identified/DetailedScan Sensor Knowledge 승격 Source를 적용했다.
  - `CFSensorActiveScanTests.cpp`에 ActiveRange / AnalysisProgress / AnalysisDecay 3개 asset-free Automation을 작성했다.
    - 현재 정확한 Source official Build `d1cd746d5f3340e1bb1cfe43885cf5e2`에서 UHT와 CFSensorTypes/ActiveScanTests/VehicleSensorComp/VehicleSensorData 및 기존 Sensor C++ Compile은 PASS했으나 실행 중 `UnrealEditor.exe`가 DLL을 점유해 Link `LNK1104`로 종료됐다.
  - 현재 Editor는 이번 세션 AI-owned 증거가 없어 자동 종료하지 않았으며 P0-04 Automation도 새 binary에서 실행하지 않았다. 따라서 `SEN-P0-04`를 Technical Done으로 승격하지 않는다.
  - TargetSelect/HUD/CFVehiclePawn/Project Config/Content Asset/DestroyedHold와 기존 USER Pending 체크포인트는 변경하지 않았고 current gate를 `SEN-P0-04 Final Build + Sensor Regression`으로 유지한다.

- v2.43.8 (2026-08-15)
  - CF-FQ-036 `SEN-P0-03 Contact Lifetime`을 Technical Done으로 갱신했다.
  - 탐지 상실 즉시 제거를 `Live → LastKnown → Lost → Removed`로 교체하고 LastKnown 위치·마지막 관측 시각을 마지막 신뢰 값으로 고정했다.
  - `FreshnessSeconds / ContactMemoryTimeSec`을 bounded Actor cursor와 분리해 매 Sensor update 진행하며 Lost는 최소 한 Snapshot에 노출한 뒤 다음 update에서 제거하도록 했다.
  - Lost 게시 전 동일 Actor 재획득 시 기존 ContactId, Sensor Knowledge와 AnalysisProgress를 유지하는 Live 복귀 계약을 기록했다.
    - 최종 `CFVehicleSensorComp.h v1.2.1` formatting-only 정리 후 정확한 Source에서 Build `fdd231a10ff648839b14b6a2468b295a` PASS와 `CarFight.Sensor` `45c96b20b2984984b753ff69c9b49d3f` 8/8 PASS를 최신 기술 증거로 기록했다.
  - TargetSelect/HUD/Project Config/Content Asset/DestroyedHold/Active Scan과 기존 USER Pending 체크포인트는 변경하지 않았고 다음 Gate를 `SEN-P0-04 Active Scan Analysis`로 이동했다.

- v2.43.7 (2026-08-15)
  - CF-FQ-036 `SEN-P0-02 Passive Detection`을 Technical Done으로 갱신했다.
  - Sensor private weak Actor Runtime Contact, Level Actor persistent cursor와 `MaxActorScansPerUpdate` bounded budget을 현재 구현으로 반영했다.
  - Passive 범위는 전방향·LOS 불필요, Passive 밖 Visual fallback은 `ECC_Visibility` 직접 가시성으로 판정하며 TargetSelect 전용 Trace Channel은 사용하지 않는다.
  - 동일 Actor 재관측에서 ContactId를 유지하고 Source Identified를 public Sensor Knowledge로 복사하지 않는 Detected 시작 계약을 기록했다.
  - 최종 Build `0e5a272bd3f047cf97574914cb9fce92` PASS와 `CarFight.Sensor` `288b8cc1c5224f4bbf57fbf7b23654cd` 6/6 PASS를 기술 증거로 기록했다.
  - TargetSelect/HUD/Project Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았고 다음 Gate를 `SEN-P0-03 Contact Lifetime`으로 이동했다.

- v2.43.6 (2026-08-15)
  - CF-FQ-036 `SEN-P0-00 Foundation Audit`과 `SEN-P0-01 Contact Data Contract`를 Technical Done으로 갱신했다.
  - 독립 `ContactId`, `ECFSensorContactState`, Actor-free `FCFSensorContact/FCFSensorSnapshot`, `FCFSensorConfig/UCFVehicleSensorData`와 `UCFVehicleSensorComp` Foundation 적용을 현재 상태로 반영했다.
  - 공식 UE 5.8 Build `e8c46e25c4aa464da7f9f07730dcba1a` PASS와 `CarFight.Sensor.SEN_P0_01` 3/3 PASS를 기술 증거로 기록했다.
  - TargetSelect/HUD/Collision Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았고 다음 Gate를 `SEN-P0-02 Passive Detection`으로 이동했다.

- v2.43.5 (2026-08-15)
  - 사용자 결정에 따라 `CF-FQ-036 차량 센서·Contact Intelligence Runtime`을 새 단일 Active로 전환했다.
  - 대표 Plan `SensorContactPlan.md v0.1.0`과 첫 Gate `SEN-P0-00 Foundation Audit`을 등록했다.
  - 실제 Sensor Runtime Provider 부재, TargetSelect=선택 / Sensor=탐지·지식 / HUD=표시 경계를 현재 상태로 기록했다.
  - 인계 준비에서는 문서만 변경하고 Source·Config·Content Asset·Build·PIE는 건드리지 않았다.

- v2.43.4 (2026-08-15)
  - CF-FQ-008 WD-P0-03 Current System Integration을 완료하고 `Document/Systems/Combat/WeaponData.md v1.0.0`을 Current System으로 승격했다.
  - `WeaponFire.md v1.6.0`의 Ammo 도입 전 Magazine/Reload 미구현 설명을 CF-FQ-031의 현재 Runtime 책임 경계로 교정했다.
  - CF-FQ-008을 Done으로 전환하고 `WeaponDataPlan.md v0.3.0`을 Historical + Retained Path로 내렸다.
  - 다음 Feature를 자동 Active로 선택하지 않아 현재 단일 Active는 없다. 기존 USER Pending 체크포인트는 그대로 보존한다.

- v2.43.3 (2026-08-15)
  - CF-FQ-008 WD-P0-01 Static Data Contract와 WD-P0-02 Representative Assets를 Technical Done으로 갱신했다.
  - 최종 Build `53e2dbaf04a3401b8ed89c906b308cdc` PASS와 `CarFight.WeaponData` `eefe58aa87d74dcaa79a1764a5f7611b` 2/2 PASS를 기록했다.
  - WeaponData Content Asset mutation은 0이며 WD-P0-03 Current System Integration은 Next로 유지한다.

- v2.43.2 (2026-08-15)
  - CF-FQ-015를 VD-P0-00~03 Remote Technical Done / VD-P0-04 USER Tuning Pending 체크포인트가 보존된 Paused로 전환했다.
  - `CF-FQ-008 무장 데이터 정의`를 단일 Active로 승격하고 `WeaponDataPlan.md v0.1.0`을 대표 Plan으로 등록했다.
  - 기존 UCFWeaponData를 재설계하지 않고 정적 DataValidation과 대표 자산 Load-only 검증부터 진행하도록 고정했다.

- v2.43.1 (2026-08-15)
  - CF-FQ-015 VD-P0-00~03을 Remote Technical Done으로 갱신했다.
  - 공식 Build `0cffed2f02674b6692d4f8af811d9036` PASS와 `CarFight.VehicleData` `59091b9559db46859c3a521be72396bf` 3/3 PASS를 기록했다.
  - VehicleData Current System 문서를 v2.0.0으로 갱신하고 실제 Content Asset 값은 변경하지 않았음을 명시했다.
  - 남은 VD-P0-04 USER Tuning은 사용자 PIE 가능 시점까지 보존한다.

- v2.43.0 (2026-08-15)
  - 사용자 선택에 따라 CF-FQ-015 차량 데이터 튜닝 패스를 단일 Active로 전환하고 CF-FQ-026 TargetSelect는 기존 TS-P0-08 USER PIE 체크포인트가 보존된 Paused로 이동했다.
  - VehicleDataTuningPlan v0.1.0과 VD-P0-00 Foundation Audit 완료를 반영했다.
  - 실제 차량 튜닝값은 USER PIE 전 변경하지 않고 VD-P0-01 Validator Contract부터 원격 진행한다.

- v2.42.9 (2026-08-15)
  - TS-P0-08 최신 Build 806a8dba54434dc38b1b83baa310c50c PASS와 targeted 3/3 PASS를 반영했다.
  - 저장 BP CDO의 실제 fallback 설정 경로와 TargetPoint SM_Body 자동 정렬을 확정했다.
  - 런타임 검색 진단과 의미 보존 LOS 사전필터를 반영했으며 20Hz 전체 Actor 순회는 대표 workload 전 구조 교체를 보류했다.
  - 기존 USER 체크포인트는 변경하지 않았다.

- v2.42.7 (2026-08-15)
  - `FIT-P0-06 Fitting ViewData and Debug`의 C++ ViewData·Blueprint Contract Technical PASS를 반영했다.
  - 공식 Build `ad3452d3add74785bbdb41c667dce728`, FIT-P0-06 1/1, Fitting 22/22, Inventory 12/12, Full CarFight 84/84 Success를 기록했다.
  - CF-FQ-035/034의 현재 원격 기술 선행 Gate를 완료 상태로 정리하고 남은 범위를 Field UI·실제 화면 가독성·공인 Light·Default·Heavy Mobility·USER 실제 사용 검증으로 한정했다.
  - 사용자 선택 없이 다음 Feature를 자동 Active 전환하지 않도록 다음 작업 선택 기준을 갱신했다.

- v2.42.6 (2026-08-15)
  - cross-feature `FFIT-P0-01~04` Permission·Timed Action·Atomic completion·Chaos Field Mass Reapply를 Technical PASS로 반영했다.
  - 실제 M_VehicleDefensePIE에서 Configured/Actual Mass +50kg 반영과 원복, Physics State·Transform·Runtime Ready 보존을 확인했다.
  - 최종 Build `cd7207084dfd49c4a28b607d8577307a`, FFIT-P0-04 4/4, Fitting 21/21, Inventory 12/12, Full CarFight 83/83 Success를 기록했다.
  - 다음 원격 기술 Gate를 `FIT-P0-06 Fitting ViewData and Debug`로 이동하고 USER-facing Field UI·PIE는 Pending으로 보존했다.

- v2.42.4 (2026-08-14)
  - 사용자가 직접 USER Visual을 수행할 수 없는 기간 동안 기술 개발을 계속하기로 결정해 `CF-FQ-032`를 시각 확인 체크포인트가 보존된 Paused로 전환하고 `CF-FQ-035`를 단일 Active로 전환했다.
  - `INV-P0-05 / M5`의 읽기 전용 Inventory Snapshot·Reservation 표시·외부 Fitting CompatibilityHint·의미 ChangeSet을 구현 완료했다.
  - 공식 Build `30503595ba074c63ba8a6bb87f7a2645`, Inventory 9/9, 전체 CarFight 68/68 Success를 현재 기술 증거로 반영했다.
  - 현재 next gate를 `INV-P0-06 / M6 Integration Verification`과 Field Fitting Coordinator 기술 통합으로 변경했다.
  - CF-FQ-032 Defense Production Panel/Pawn Rebind USER Visual은 취소·완료 처리하지 않고 동일 체크포인트를 보호한다.

- v2.42.3 (2026-08-14)
  - `CFHUDDataTests v1.6.0`으로 실제 Defense Runtime→Provider ViewData, 저장 `M_VehicleDefensePIE` 실제 PIE 복제 Defense SUV→Provider ViewData와 Pawn Rebind Old Pawn 이벤트 해제를 자동 회귀화했다.
  - 최종 공식 UE 5.8 Editor Build `681c91810da34066bb398ad1b0989f1e` Exit Code 0과 targeted `CarFight.UI.UI_P0_03` Automation `f6b08a970bbf4ec282893e86e92e72ac` 5/5 Success·0 Fail을 현재 기술 증거로 반영했다.
  - 사용자 직접 화면 확인이 없으므로 Defense와 Pawn Rebind를 USER PASS로 승격하지 않고 next gate를 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`로 좁혔다.
  - 대표 Plan을 `Document/Plan/InGameUIPlan.md v0.58.5`로 동기화했으며 UI-P0-03과 CF-FQ-032는 계속 Active다.

- v2.42.0 (2026-08-13)
  - `CF-FQ-032` 현재 projection을 대표 `InGameUIPlan.md v0.58.0`과 동기화했다.
  - Salvo 전용 Hold v1.5.0·v1.6.0 폐기와 HeavyCannon·Ripple·Salvo 공통 Weapon/Launcher Presentation lifecycle 교정 완료를 반영했다.
  - 공식 Editor Build `6978e029bfaa48c7addfc9acd87484b1` PASS와 전체 Automation `00ee23a01aa7480cabc557f312780ae8` 64/64 PASS·필수 32/32 Success를 현재 기술 검증 기준으로 반영했다.
  - 현재 next gate를 `/Game/Maps/TestMap_DRSalvo` USER PIE로 고정하고 Launcher USER PASS 후 Defense 실제 변화 → Pawn Rebind 순서를 유지했다.

- v2.41.0 (2026-08-13)
  - TestMap_AmmoRipple의 시작 3/4+Reserve4, 실제 3발 Partial Ripple, Loaded 감소, 2초 Auto Reload, 두 번째 4발 Ripple과 최종 NO AMMO를 사용자 전 항목 PASS로 반영했다.
  - `AMMO-P0-08`과 `CF-FQ-031`을 Done으로 전환하고 `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System으로 등록했다.
  - 현재 단일 Active를 `CF-FQ-032`로 복귀시키고 UI-P0-03의 다음 Gate를 Launcher 전체 Presentation(Salvo·terminal Cooldown→READY), Defense 실제 변화, Pawn Rebind로 고정했다.
  - finite Ammo는 더 이상 UI Provider 부재 항목이 아니며 실제 VehicleAmmoComp Snapshot을 WeaponPanel이 소비한다고 현재 기준선을 교정했다.

- v2.40.0 (2026-08-08)
  - Engine Authority를 `ProjectSSOT → UE 5.8 Source Build → D:\UnrealEngine_Source`로 명시했다.
  - `EngineSourceBuild/`의 UE 5.7 Launcher → Source 전환 기록을 Historical / Superseded → Archive 경계로 분리했다.
  - 공용 `Document/SSOT/UE_SSOT/`는 UE 5.x 공통 원칙만 소유하고, 정확한 버전·배포 형태·경로·버전 민감 API는 프로젝트별 Engine Baseline을 우선하도록 고정했다.

- v2.39.0 (2026-08-08)
  - CarFight의 현재 공식 엔진을 **Unreal Engine 5.8 Source Build**로 명시했다.
  - 공식 엔진 루트를 `D:\UnrealEngine_Source`, 빌드·실행 진입점을 `Tools\BuildEditor.bat` / `Tools\RunEditor.bat`로 고정했다.
  - 과거 UE 5.7 표기는 Historical/폐기 기준이며 현재 호환성·API·구현 판단에 사용하지 않도록 명시했다.
  - 엔진 소스 수준 판단에서는 실제 `D:\UnrealEngine_Source` UE 5.8 소스를 최우선 근거로 사용하도록 고정했다.

- v2.37.0 (2026-08-06)
  - 사용자 DR-PIE-06에서 Salvo·Ripple, Muzzle 순서, Sequence 4/4, 각 5 Volley, 중복 입력 방지, 추진·Trail·Thruster·Impact FX와 충돌 격리를 모두 PASS했다.
  - DR-PIE-00~06 전체 USER PASS와 DR-P0-07 완료에 따라 `CF-FQ-033 차량 방어·손상 런타임`을 Done으로 전환했다.
  - `Document/Systems/Combat/VehicleDefense.md v1.0.0`을 신규 Current System으로 등록하고 `HitDamage.md v1.1.0`과 `SystemIndex.md v1.15.0`을 통합 기준으로 갱신했다.
  - 데미지 시스템 완료 후 UI로 복귀한다는 기존 우선순위에 따라 `CF-FQ-032`를 현재 단일 Active로 복원했다.
  - 다음 실행을 UI-P0-02 Pause·Focus·Launcher·Projectile·Timer 사용자 PIE로 고정하고 PASS 뒤 UI-P0-03 HUD 데이터 계약으로 진행하도록 했다.
  - 코드·에셋·빌드·Automation은 다시 변경하거나 실행하지 않았고 기존 미커밋 변경과 체크포인트를 보호했다.

- v2.36.0 (2026-08-03)
  - 사용자 결정에 따라 작업 완료 최우선 사항을 `CF-FQ-033 차량 방어·손상 런타임`, 즉 데미지 시스템으로 재정렬했다.
  - `CF-FQ-033`을 Active로 전환하고 `DR-P0-07 사용자 PIE → 실패 결함 우선 수정 → 전체 PASS → Systems 승격 → Done` 완료 경로를 고정했다.
  - 데미지 검증을 직접 차단하는 문제만 예외적으로 선행 처리하고 해소 즉시 CF-FQ-033으로 복귀하도록 했다.
  - `CF-FQ-032`는 UI-P0-01A~02 완료 상태와 User PIE·UI-P0-03 체크포인트를 보존한 Paused 상태로 전환했다.
  - 런처·미사일·TargetSelect·피팅·인벤토리의 기존 구현 상태와 검증 결과는 변경하지 않았다.

- v2.35.0 (2026-08-02)
  - `CF-FQ-030 MG-P0-01~04` Direct Missile Runtime과 격리 테스트 자산 적용 상태를 현재 기준선에 동기화했다.
  - Launcher Volley의 위치·Actor 목표를 첫 발사 순간 함께 보존하도록 수정하고 이후 TargetSelect 변경과 후속 미사일 목표를 분리했다.
  - 공식 Editor Build `ce88150dfbbc495d91d1e2b8cd7c5225` / Exit Code 0과 전체 Automation `85c1e7285a7946fa8f55182b6247f0f4` / 43/43·필수 21/21 Success를 기록했다.
  - `CF-FQ-030`은 Ready for Manual PIE로 유지하고 Done·Systems Current 승격과 Unreal Asset 변경은 수행하지 않았다.
  - 현재 단일 Active `CF-FQ-032`, Paused `CF-FQ-029·026`과 사용자 PIE Pending 상태를 유지했다.

- v2.34.0 (2026-08-02)
  - 차량 Runtime 준비 상태를 Core와 Combat으로 분리하고 기존 `bVehicleRuntimeReady`를 Core 호환값으로 유지했다.
  - 차량 Gameplay 입력은 Pawn `DefaultInputMappingContext`, Controller는 System·UI 입력을 소유하는 현재 경계를 고정했다.
  - `CF-FQ-034 FIT-P0-05` Initial Sortie Adapter와 `CF-FQ-035 INV-P0-04` Fitting Adapter 완료 상태를 현재 기준선에 동기화했다.
  - Inventory Commit과 Runtime Apply를 원자 조율하는 Field Fitting Coordinator는 미구현 상태로 분리했다.
  - 현재 단일 Active를 `CF-FQ-032`로 유지하고 UI-P0-03과 Field Fitting Runtime은 시작하지 않았다.

- v2.33.0 (2026-08-01)
  - `CF-FQ-032 UI-P0-02` 싱글플레이 완전 Pause C++ 기반을 완료했다.
  - Drive·Look·Pressed Key 입력 중립화, C++ Pause Menu와 실제 World Pause를 기록했다.
  - Launcher·Projectile·Motor·World Timer 진행 정지 구조 회귀를 기록했다.
  - 공식 Editor Build와 전체 Automation 32/32 PASS를 기록했다.
  - 사용자 PIE, UI-P0-03과 Systems 승격은 미수행 상태로 유지했다.
  - `CF-FQ-029 LM-P0-06`과 `CF-FQ-034 FIT-P0-03` 보호 상태를 보존했다.

- v2.32.0 (2026-08-01)
  - 현재 단일 Active 작업을 사용자 지정에 따라 `CF-FQ-032`로 전환했다.
  - `UI-P0-01A~01B` C++ 기반, 공식 Editor Build와 전체 Automation 29/29 PASS를 기록했다.
  - 기존 Pawn 입력·AimReticle·TargetSelect와 Unreal Asset이 변경되지 않았음을 기록했다.
  - `CF-FQ-029 LM-P0-06` 체크포인트와 `CF-FQ-034 FIT-P0-03` 보호 상태를 보존했다.
  - `UI-P0-02`, 사용자 PIE와 Systems 승격은 미수행 상태로 남겼다.

- v2.25.0 (2026-07-30)
  - CF-TC-023 투사체 비행 FX 사용자 PIE 전체 행렬 PASS를 반영했다.
  - Trail-only, Thruster-only, Trail+Thruster, 유효 소켓과 Missing Socket Fallback을 PASS 처리했다.
  - Hit·LifeExpired Reset, Pool 20발 이상, Ribbon History 무잔류와 30 FPS 고속 Bounds를 PASS 처리했다.
  - CF-FQ-027을 Paused에서 Done으로 전환하고 Document/Systems/Combat/Projectile.md v1.5.0을 Current System으로 연결했다.
  - Automation은 소스 컴파일 PASS / 실행 Not Run 상태와 Runner 미노출 사실을 유지했다.
  - 현재 Active CF-FQ-029 LM-P0-06과 다른 Ready·Deferred 상태는 변경하지 않았다.

- v2.24.0 (2026-07-29)
  - CF-FQ-030 Ready 상태 안에서 MG-P0-00 비활성 Foundation 타입·Config·Guidance 수학을 적용했다.
  - UCFProjectileData v1.8.0의 두 Missile Config는 기본 비활성이며 기존 Projectile·Rocket 런타임에 연결하지 않았다.
  - Editor Build Job 8c4f952fa58f499b9145b95caa1ff926 / Exit Code 0과 Automation Source Compile PASS를 기록했다.
  - 현재 Active는 CF-FQ-029 LM-P0-06을 유지하고 MG-P0-01은 해당 사용자 PIE 뒤로 차단했다.

- v2.23.0 (2026-07-29)
  - LM-P0-05 Launcher Editor Assets를 적용하고 독립 AssetDump와 CDO 재검증을 완료했다.
  - 플레이어 기본 BP_CFVehiclePawn은 DA_TestSUV + RocketLauncher로 시작하고 TestMap에는 Sedan + HeavyCannon과 SUV + RocketLauncher 표적을 유지한다.
  - LM-P0-06 CF-TC-025·026 사용자 PIE를 현재 단계로 이동했다.
  - 사용자 결과 전에는 CF-FQ-029 Done 또는 Systems 승격을 기록하지 않는다.

- v2.20.0 (2026-07-28)
  - `CF-FQ-029 모듈형 런처 및 발사 인계`를 현재 Active 작업으로 동기화했다.
  - `CF-FQ-030 물리 제한형 미사일 비행·유도`를 CF-FQ-029 선행의 Ready 작업으로 동기화했다.
  - 사용자 결정에 따라 `CF-FQ-019 주행·전투 반복 테스트`를 Candidate에서 Deferred로 이동했다.
  - CF-FQ-019는 CF-FQ-029·030 이후 런처·미사일 통합 회귀 범위로 다시 설계하도록 기록했다.
  - 소스, 에셋, 빌드와 PIE 상태는 변경하지 않았다.

- v2.19.0 (2026-07-28)
  - `CF-FQ-028 발사체 추진 시스템`을 `Document/Systems/Combat/Projectile.md v1.4.0` Current System으로 승격했다.
  - `CF-FQ-028`을 Done / User PIE PASS / `CF-TC-024 PASS`로 종료했다.
  - Projectile Current System에 비유도 Rocket 추진, Burning 기반 Thruster, Trail·Thruster 소켓/Fallback, 독립 FX Scale과 Pool Reset을 통합했다.
  - `ProjectilePropulsionPlan.md v1.2.0`을 완료 이력 문서로 전환했다.
  - 현재 자동 선택된 Active 작업을 비우고 `CF-FQ-019`를 Candidate / Not Started로 유지했다.
  - `CF-FQ-027`은 런타임 기반이 Current System에 반영됐지만 별도 전체 체크리스트와 `CF-TC-023` 미완료로 Paused 상태를 유지했다.
  - 이번 승격 작업에서는 반복 전투 회귀를 수행하거나 완료 처리하지 않았다.

- v2.18.0 (2026-07-28)
  - `DA_PFX_ThrusterTest` 사용자 PIE에서 추진과 Thruster FX는 정상이고 Scale만 반영되지 않는 결함을 확인했다.
  - 유효 `FX_Exhaust` 소켓 경로가 FX Origin Scale을 `1,1,1`로 덮어쓰던 원인을 보정했다.
  - `UCFProjectileData v1.7.1`, `ACFProjectileActor v1.8.1`, `CFProjectileFlightFxTests.cpp v1.1.0`을 현재 기준선으로 갱신했다.
  - 최종 Build Job `e8b812bd479549299dd116f9bae8996f`에서 UHT·컴파일·링크 Exit Code 0을 확인했다.
  - 사용자 재검증에서 `RelativeTransform.Scale=0.2` 적용 시 추진 화염이 정상 축소됨을 확인했다.
  - `PP-P0-08`, `PP-P0-09`를 Done으로 전환하고 `CF-TC-024 PASS`를 등록했다.
  - 다음 단계를 `PP-P0-10 Systems 승격과 반복 전투 회귀`로 이동했다.

- v2.17.0 (2026-07-27)
  - 로켓·미사일용 실제 자체 추진 기반을 CF-FQ-028로 등록하고 단일 Active로 전환했다.
  - CFProjectileMotorTypes v1.0.0, UCFProjectileMotorComp v1.0.0, UCFProjectileData v1.7.0, ACFProjectileActor v1.8.0과 RuntimeContract 소스를 현재 C++ 기준선으로 등록했다.
  - InitialSpeed 분리, IgnitionDelay, 고정 LaunchDirection 가속, 최대 추진 속도, BurnedOut 관성·중력 비행과 Pool Reset 계약을 기록했다.
  - Thruster FX가 실제 Motor Burning 상태에서만 재생되는 기준을 추가했다.
  - 첫 Build Job 984f0ccab94241148e7b40b9d30a5572의 Automation 타입 오류를 수정하고, 최종 형식 정리 후 Build Job 2ffd09357e654bb7970a2e379f5ab45f Exit Code 0을 공식 증거로 등록했다.
  - CF-FQ-027은 Editor 자산 연결 단계에서 Paused로 보존하고 다음 실행을 테스트 Rocket ProjectileData와 사용자 PIE로 이동했다.

- v2.16.0 (2026-07-27)
  - PFX-P0-05 자산 조사에서 Rocket Thruster Niagara 18/18, Stylized Attacks Niagara 14/14와 DA_HeavyShell 1/1 AssetDump 성공을 확인했다.
  - Trail 기술 1차 후보를 NS_RibbonTrail, Thruster 1차 후보를 NS_RocketExhaust_Realistic으로 기록했다.
  - AssetDump 기본 프로필의 Niagara 내부 설정과 DataAsset 실제 값 미노출 제한을 반영해 자산 연결 완료가 아닌 Editor 시각·저장 Pending으로 판정했다.
  - DA_HeavyShell Trail-only와 DA_PFX_ThrusterTest·DA_PFX_BothTest, FX_Trail·FX_Exhaust 소켓 및 Fallback 설정을 다음 Editor 작업으로 고정했다.

- v2.15.0 (2026-07-27)
  - CF-FQ-027의 ProjectileData 비행 FX 계약과 ProjectileActor Trail·Thruster 생명주기를 현재 AI 세션이 직접 구현했다.
  - UCFProjectileData v1.6.0, ACFProjectileActor v1.7.0과 CFProjectileFlightFxTests.cpp v1.0.0을 현재 C++ 기준선으로 등록했다.
  - 소켓 우선·Fallback, MissingSystem 무해 처리와 Pool 반환 전 Niagara Reset을 구현 상태로 반영했다.
      - 최초 빌드 include 오류와 Blueprint 툴팁을 보정한 뒤 Build Job 445848ab0f7749fcab8188b164bb1487 Exit Code 0을 최종 공식 증거로 등록했다.
  - 현재 Admin 표면의 Automation Runner 부재로 테스트 실행은 미수행이며 소스 컴파일 PASS와 실행 Not Run을 분리 기록했다.
  - 다음 작업을 PFX-P0-05 Editor Niagara·DataAsset·소켓 연결과 사용자 PIE 준비로 변경했다.

- v2.14.0 (2026-07-27)
  - CarFight 코드 작업 기본 방침을 별도 Codex 위임에서 현재 AI 세션의 직접 구현으로 변경했다.
  - CF-FQ-027의 코드 게이트를 `Ready — Direct Implementation`으로 전환했다.
  - TaskSource와 WorkOrder를 구현 참고 자료로 유지하고 최종 Codex YAML을 착수 조건에서 제거했다.
  - 현재 사실 상태와 일시중지 작업 해석에서 활성 작업·실행 주체 표현을 직접 구현 기준으로 정정했다.

- v2.13.0 (2026-07-27)
  - 사용자 선택에 따라 `CF-FQ-027 투사체 비행 FX`를 현재 단일 Active 작업으로 등록했다.
  - PFX-P0-00에서 현재 Projectile·CombatFx 코드와 CF-PDL-0010 결정을 복원하고 지속형 FX 설계를 확정했다.
  - 대표 Plan, 로드맵, TaskSource와 사람이 검토 가능한 WorkOrder 초안을 생성했다.
  - ACFProjectileActor 소유, FX_Trail·FX_Exhaust 소켓 우선, ProjectileData Fallback과 Pool 반환 전 Reset을 현재 준비 기준선으로 추가했다.
  - 계획 테스트 ID CF-TC-023을 예약하고 실제 소스·에셋·빌드·PIE는 시작하지 않았음을 구분했다.
  - plan.* 품질·증거 게이트 미노출로 최종 Codex YAML Missing과 코드 게이트 차단 상태를 기록했다.
  - CF-FQ-019를 후속 Candidate로 유지하고 CF-FQ-026 TS-P0-08 Paused 상태를 보존했다.

- v2.12.0 (2026-07-27)
  - 최종 사용자 PIE에서 Muzzle 정상 발사 1회와 거부 0회, Impact 위치·1회성·중복·잔류 없음, Destroyed 소켓 위치·최초 1회·추가 피해 중복 없음과 기존 전투 회귀가 모두 PASS했다.
  - `CF-FQ-024`를 Done으로 전환하고 `CF-TC-021 PASS`를 현재 기준선에 등록했다.
  - `Document/Systems/Combat/CombatFx.md`를 Current System으로 승격했다.
  - 현재 자동 선택된 Active 작업을 비우고 `CF-FQ-019`를 착수 가능한 Candidate로 유지했다.
  - `CF-FQ-026`은 `TS-P0-08 Paused` 상태를 그대로 보존했다.

- v2.11.0 (2026-07-27)
  - Impact P0 Niagara를 NS_BasicHit으로 사용자 확정하고 외부 Transform Scale 미반응을 허용해 추가 튜닝을 종료했다.
  - NS_Impact_1, Adapted Niagara와 공용 User Scale 연결 작업을 P0에서 재개하지 않도록 고정했다.
  - UCFCombatFxData v1.2.0 Scale 전달 모드와 UCFVehicleData.DestroyedFxSocketName 구현 상태를 현재 기준선에 반영했다.
  - Sedan 차체의 FX_Destroyed 소켓 적용 후 폭발 위치 해결 사용자 결과를 기록했다.
  - 사용자의 직접 Editor 빌드 PASS와 CombatFxData AssetDump 3/3 성공을 기록했다.
  - 현재 상태를 Phase 4 Asset Selection·Destroyed Socket Integration Complete / Final PIE Pending으로 변경했다.

- v2.10.0 (2026-07-25)
  - ACFCombatFxPreviewActor v1.1.0의 자동 반복, Uniform Scale, 최대 수명 자동 정지와 Override Niagara 적용을 현재 기준선에 반영했다.
  - Build Job 89b6d82710af47e89a4c5b3823037d4e의 UHT·컴파일·링크 성공과 Exit Code 0을 등록했다.
  - NS_AOE_Explosion_1을 수동 Loop·세미리얼 검토 대기 후보로 고정했다.
  - SmokeBuilder NiagaraSystem 0개와 Cascade ParticleSystem 64개를 확인하고 현재 Niagara P0 계약에서 제외했다.
  - 현재 AssetDump 프로필로는 DataAsset 참조 프로퍼티 값을 검증할 수 없음을 기록했다.
  - 현재 상태를 Phase 3 Rapid Preview Tuning Build PASS / Manual Review·PIE Pending으로 변경했다.

- v2.9.0 (2026-07-24)
  - ACFCombatFxPreviewActor EditorOnly 도구 구현과 Build Job c9682d4a0be24ead95e87a5a7e8b5849 Exit Code 0을 현재 기준선에 반영했다.
  - UCFCombatFxData.MaximumLifetimeSeconds와 런타임 Loop 강제 제거 안전 퓨즈를 기록했다.
  - `/Game/CarFight/FX/Data`의 CombatFxData 3개가 이미 존재하며 AssetDump 3/3 성공임을 반영했다.
  - 현재 기준 VehicleData를 DA_TestSedan으로 정정하고 DA_PoliceCar를 신규 FX 연결 대상에서 제외했다.
  - NS_Explossion을 Loop 반복으로 Destroyed P0 후보에서 제외했다.
  - 현재 상태를 Phase 2 Editor Preview Tool Build PASS / DataAsset Tuning·PIE Pending으로 변경했다.

- v2.8.0 (2026-07-24)
  - CF-FQ-024 공용 Combat FX C++ 기반과 Niagara 모듈 의존성 구현 완료를 현재 기준선에 반영했다.
  - 승인 발사, HitScan·Projectile 첫 Impact와 최초 차량 파괴의 데이터 기반 FX 요청 연결을 기록했다.
  - Build Job bd5a50bf388049ec84123aec4942ae96의 UHT·Editor 빌드 성공과 Exit Code 0을 공식 증거로 등록했다.
  - 현재 상태를 Phase 1 C++ Foundation Build PASS / DataAsset·PIE Pending으로 변경했다.
  - 다음 작업을 CombatFxData 3개 생성, 기존 데이터 연결과 사용자 PIE로 고정했다.

- v2.7.0 (2026-07-24)
  - 사용자 우선순위 변경에 따라 `CF-FQ-024 전투 FX`를 현재 단일 Active 작업으로 승격했다.
  - 현재 단계를 `Phase 0 FAB FX 자산 반입과 후보 선별`로 고정했다.
  - `CF-FQ-026`은 `TS-P0-00~07 Done / TS-P0-08 Paused`로 보존했다.
  - 전투 FX 현재 실제 기준선, 구현 원칙과 바로 다음 자산 조사 작업을 갱신했다.
  - 반입된 FAB 콘텐츠는 실제 Niagara 목록과 의존성을 확인하기 전까지 후보 확정으로 해석하지 않도록 했다.

- v2.6.0 (2026-07-24)
  - CarFight 프로젝트 전역 게임 사운드 비지원 결정을 현재 실제 기준선에 추가했다.
  - CF-FQ-024를 전투 FX 전용 Ready 작업으로 변경하고 CombatFxAudio 경로를 레거시 이름으로 고정했다.
  - Sound 자산, Audio 런타임, 오디오 모듈과 청각 테스트 완료 조건을 신규 작업에서 금지했다.
  - 과거 상태 기록의 Audio 표현은 역사 기록이며 현재 구현 지시가 아니라는 해석 규칙을 추가했다.

- v2.5.0 (2026-07-23)
  - 현재 단일 활성 작업을 `CF-FQ-026 타겟 선택 시스템`으로 전환했다.
  - `CF-FQ-024`는 취소하지 않고 구현 재개 가능한 Ready 상태로 보존했다.
  - TS-P0-01 C++ 계약과 Editor 빌드 PASS, Blueprint·PIE Pending 상태를 현재 실제 기준선으로 추가했다.
  - 바로 다음 작업을 TS-P0-00 조사 결과 복구와 TS-P0-01 검증으로 고정했다.

- v2.4.0 (2026-07-22)
  - CF-FQ-025 이중 레티클 및 터렛방향 시각화의 Done / User PIE PASS를 현재 완료 기준선에 추가했다.
  - Image_CenterDot 조준 레티클과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클의 책임 분리를 검증된 흐름에 반영했다.
  - 현재 활성 병목과 다음 순서는 CF-FQ-024 전투 FX / Audio 기준을 유지했다.

- v2.3.0 (2026-07-15)
  - 완료된 조준·발사·Reticle/UI·피격·피해·고속 Projectile 기준선을 현재 상태로 추가했다.
  - 현재 병목을 실제 전투 FX / Audio 부재로 정리하고 CF-FQ-024를 활성 작업으로 지정했다.
  - 현재 진행 순서를 CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021로 갱신했다.
  - 판정과 연출 분리, 데이터 기반 자산 참조, 중복·Pool 잔류 방지 원칙과 리스크를 추가했다.

- v2.2.0 (2026-06-19)
  - 다음 개발 기준을 싱글 로컬 전투 루프 구현/검증 순서로 갱신했다.
  - 현재 확인된 Aim Fire/Reticle 골격과 앞으로 구현할 차량 무기/피격/피해 시스템을 분리했다.
  - 전투 범위 확산과 피드백 선행 과다 리스크를 추가했다.

- v2.1.3 (2026-06-19)
  - 사용자 에디터 정상작동 확인 결과를 현재 실제 기준선에 반영했다.
  - 이번 세션 완료 기준을 기능 추가가 아니라 싱글플레이 전환 정리 완료로 고정했다.

- v2.1.2 (2026-06-19)
  - 현재 세션 목표를 기능 추가 없이 싱글플레이 전환 정리로 고정했다.
  - 기본 실행 기준선을 `CFSingleGameMode`와 로컬 Aim/Fire 흐름으로 해석하도록 명시했다.
  - 서버 Target/보관 문서는 삭제하지 않고 Deferred 대상으로 보존한다고 정리했다.

- v2.1.1 (2026-06-19)
  - 이번 세션 범위를 코드/툴 삭제가 아니라 싱글플레이 진행 선로 전환으로 정정했다.
  - 서버 관련 구현물은 현상 유지하고, 현재 착수 판단에서는 참고/보류 대상으로만 읽도록 명시했다.

- v2.1.0 (2026-06-18)
  - 현재 프로젝트 상태를 서버 권한 전투 Vertical Slice 준비 단계에서 싱글 플레이 1대 차량 고도화 단계로 전환했다.
  - Dedicated Server / 2클라 / 이동 복제 / 서버 런처 / 세션 / 관리툴을 현재 일정 제외 항목으로 명시했다.
  - 서버/네트워크 구현은 삭제 확정이 아니라 `Deferred` 기준의 보존 대상으로 분리했다.
  - 현재 최우선 개발 후보를 싱글 실행 기준선, 주행감, 카메라, 로컬 Aim, WheelSync 품질, 차량 데이터 튜닝으로 재정렬했다.

- v1.1.0 (2026-04-15)
  - 문서 버전 / 마지막 정리 날짜를 갱신했다.
  - 현재 카메라 기준선 섹션을 추가했다.

---

## Migration

### v2.43.14 적용 안내

```text
- 현재 단일 Active는 CF-FQ-037 차량 스캐너 입력·장비 통합이며 대표 Plan은 Document/Plan/ScannerIntegrationPlan.md v0.1.0이다.
- 첫 Gate는 SCAN-P0-00 Foundation Audit으로 Source·Config·Content Asset·Build·PIE mutation 없이 실제 Input/Data/Fitting owner를 감사한다.
- CF-FQ-036은 Done / Current System을 유지하며 Sensor 현재 의미는 Document/Systems/Targeting/SensorContact.md v1.0.0을 우선한다.
- Scanner Input은 Sensor가 직접 Bind하지 않고 Pawn 또는 확인된 Input Adapter가 StartActiveScan/StopActiveScan command를 호출하는 구조를 유지한다.
- 감사 전에는 새 Scanner DataAsset 클래스, 임의 Sensor tuning 값, InputAction asset을 선제 생성하지 않는다.
- Radar Range/Zoom·동적 Blip·Radar/TargetPanel USER Visual은 CF-FQ-032 별도 Pending이며 CF-FQ-037 완료 조건이 아니다.
- 기존 CF-FQ-015/026/029/032/035 Paused와 CF-FQ-030/034 Ready USER 체크포인트를 그대로 보존한다.
```

### v2.43.13 적용 안내

```text
- CF-FQ-036은 Done이며 Sensor/Contact 현재 구현은 Document/Systems/Targeting/SensorContact.md v1.0.0과 실제 Source를 우선한다.
- Document/Plan/SensorContactPlan.md v0.9.0은 완료 당시 설계·검증 evidence를 보존하는 Historical + Retained Path다.
- 현재 단일 Active는 없으며 Paused/Ready 기능은 사용자가 선택하기 전까지 자동 착수하지 않는다.
- TargetSelect는 후보 검색·선택·TrackState owner, Sensor는 Detection·Contact lifecycle·Knowledge owner, HUD는 read-only Snapshot adapter 책임을 유지한다.
- public Sensor Snapshot은 Actor/UObject pointer-free 계약을 유지한다.
- TargetSelect Destroyed 즉시 clear와 Sensor DestroyedHold는 독립 수명으로 유지한다.
- Radar Range/Zoom·NormalizedPosition·동적 Blip 및 CF-FQ-032 Radar/TargetPanel USER Visual은 별도 Pending이다.
- CF-FQ-026 TS-P0-08 USER PIE와 다른 USER Pending 체크포인트도 CF-FQ-036 Done으로 승격되지 않는다.
```

### v2.43.12 적용 안내

```text
- 현재 단일 Active는 CF-FQ-036이며 대표 Plan은 Document/Plan/SensorContactPlan.md v0.8.0이다.
- SEN-P0-00~06은 Technical Done이며 다음 착수점은 SEN-P0-07 Technical Acceptance다.
- P0-06 최종 기술 기준선은 Build 7dff9da7aaa24e76b0871348762c9d93 PASS와 CarFight.Sensor 14/14 PASS다.
- TargetSelect는 후보 검색·선택 수명·TrackState owner를 유지하고 Sensor는 Detection·Contact lifecycle·Knowledge owner를 유지한다.
- Target/Radar Player-facing Knowledge는 actor-free FCFSensorSnapshot을 source로 하며 선택 Actor는 ContactId association에만 사용한다.
- HUD는 Actor metadata·현재 위치·Sensor range로 Detection/Knowledge/Radar scale을 재계산하지 않는다.
- Radar Range/Zoom 계약 전에는 실제 RelativePositionMeters/DistanceMeters만 사용하고 NormalizedPosition은 Unavailable이다.
- 기존 CanvasPanel_RadarContacts 정적 placeholder는 실제 Sensor Contact가 아니므로 계속 숨긴다. 동적 Radar Blip과 시각 USER PASS는 CF-FQ-032 범위다.
- TargetSelect Destroyed 즉시 clear와 Sensor DestroyedHold Radar 보존은 독립 수명으로 유지된다.
- SEN-P0-07은 현재 Build/Sensor/UI asset-free 증거를 우선 재사용해 P0 완료 기준과 SensorContact Current System 승격 범위를 판정한다.
- WBP_TargetSelect/Content Asset/Project Config와 기존 USER Pending 체크포인트는 SEN-P0-06에서 변경하지 않았다.
```

### v2.43.11 적용 안내

```text
- 현재 단일 Active는 CF-FQ-036이며 대표 Plan은 Document/Plan/SensorContactPlan.md v0.7.0이다.
- SEN-P0-00~05는 Technical Done이며 다음 착수점은 SEN-P0-06 Public Snapshot Integration이다.
- P0-05 최종 기술 기준선은 Build 954dc0ab844b40f48f2674f2a0ae672a PASS와 CarFight.Sensor 13/13 PASS다.
- 차량 파괴 truth는 UCFVehicleHealthComp::OnVehicleDestroyed / IsDestroyed가 소유하고 Sensor는 기존 Contact에서만 이를 소비한다.
- Actor Destroy/EndPlay/weak invalid만으로 DestroyedHold를 만들지 않으며 이런 경우 기존 LastKnown→Lost 수명을 유지한다.
- DestroyedHold는 ContactId·획득 Knowledge·AnalysisProgress·마지막 신뢰 위치를 보존하고 DestroyedHoldTimeSec을 bounded cursor와 독립적으로 진행한다.
- TargetSelect의 Destroyed 즉시 clear는 그대로 유지하며 Sensor DestroyedHold와 독립이다.
- SEN-P0-06은 FCFSensorSnapshot을 actor-free read-only source로 소비 계층에 연결하되 선택/UI Gameplay 판정을 중복 소유하지 않아야 한다.
- TargetSelect/HUD/Project Config/Content Asset과 기존 USER Pending 체크포인트는 SEN-P0-05에서 변경하지 않았다.
```

### v2.43.10 적용 안내

```text
- 현재 단일 Active는 CF-FQ-036이며 대표 Plan은 Document/Plan/SensorContactPlan.md v0.6.0이다.
- SEN-P0-00~04는 Technical Done이며 다음 착수점은 SEN-P0-05 Destroyed Contact다.
- P0-04 최종 기술 기준선은 Build 786e47df68fe43f9a924c9c6fbd86726 PASS와 CarFight.Sensor 11/11 PASS다.
- Active Scan은 실행 중 ActiveScanRangeCm의 장거리 전방향 Contact Detection이며 Tactical Analysis gain은 ECC_Visibility 직접 가시를 추가 요구한다.
- Analysis progress는 비유효 시 서서히 감소하고 이미 획득한 Identified/DetailedScan Knowledge는 강등하지 않는다.
- SEN-P0-05는 TargetSelect의 Destroyed 즉시 clear를 변경하지 않고 Sensor 독립 DestroyedHold 수명만 추가해야 한다.
- TargetSelect/HUD/Project Config/Content Asset과 기존 USER Pending 체크포인트는 SEN-P0-04에서 변경하지 않았다.
```

### v2.43.8 적용 안내

```text
- 현재 단일 Active는 CF-FQ-036이며 대표 Plan은 Document/Plan/SensorContactPlan.md v0.4.0이다.
- SEN-P0-00~03은 Technical Done이며 다음 착수점은 SEN-P0-04 Active Scan Analysis다.
- 현재 Contact lifecycle은 Live → LastKnown → Lost → Removed이며 LastKnownWorldLocation은 마지막 신뢰 관측 위치로 고정한다.
- FreshnessSeconds와 ContactMemoryTimeSec은 bounded Actor cursor 재방문과 독립적으로 Sensor update마다 진행한다.
- Lost는 최소 한 Actor-free Snapshot에 게시한 뒤 다음 update에서 제거되고, 게시 전 동일 Actor 재획득은 기존 ContactId와 Sensor Knowledge를 유지한다.
- Production Sensor 수치 기본값은 실제 게임 튜닝 확정을 의미하지 않으며 Content DataAsset mutation은 없었다.
- DestroyedHold는 SEN-P0-05, Active Scan/Analysis는 SEN-P0-04, HUD/TargetSelect 소비 통합은 SEN-P0-06 책임이다.
- TargetSelect/HUD/Project Config/Content Asset과 기존 USER Pending 체크포인트는 SEN-P0-03에서 변경하지 않았다.
```

### v2.43.7 적용 안내

```text
- 현재 단일 Active는 CF-FQ-036이며 대표 Plan은 Document/Plan/SensorContactPlan.md v0.3.0이다.
- SEN-P0-00~02는 Technical Done이며 다음 착수점은 SEN-P0-03 Contact Lifetime이다.
- Passive Detection은 private weak Actor Runtime Contact와 bounded Level Actor cursor를 사용하고 공개 Snapshot은 계속 Actor-free다.
- Production fallback Passive/Visual 거리는 아직 0이므로 P0-02 Technical Done을 실제 게임 탐지 수치 확정으로 해석하지 않는다.
- 탐지 상실 즉시 Contact 제거는 P0-02 임시 정책이며 SEN-P0-03에서 Live→LastKnown→Lost로 교체한다.
- Sensor Visual LOS는 ECC_Visibility를 사용하고 TargetSelect 전용 Trace Channel을 재사용하지 않는다.
- TargetSelect/HUD/Project Config/Content Asset과 기존 USER Pending 체크포인트는 SEN-P0-02에서 변경하지 않았다.
```

### v2.43.6 적용 안내

```text
- 현재 단일 Active는 CF-FQ-036이며 대표 Plan은 Document/Plan/SensorContactPlan.md v0.2.0이다.
- SEN-P0-00과 SEN-P0-01은 Technical Done이며 다음 착수점은 SEN-P0-02 Passive Detection이다.
- 현재 UCFVehicleSensorComp의 bRuntimeReady는 Sensor Foundation Config 초기화 의미이며 실제 Passive/Active Contact 탐지가 동작한다는 뜻이 아니다.
- 공개 Sensor Snapshot은 Actor/UObject pointer를 포함하지 않고 TargetSelect 선택 상태와 독립이다.
- TargetSelect/HUD/Collision Config/Content Asset과 기존 USER Pending 체크포인트는 SEN-P0-01에서 변경하지 않았다.
```

### v2.42.4 적용 안내

```text
- 현재 단일 Active는 CF-FQ-035이며 대표 Plan은 Document/Plan/InventoryFoundationPlan.md v0.6.0이다.
- INV-P0-05는 완료됐으므로 다음 착수점은 INV-P0-06 Field Fitting Coordinator다.
- CF-FQ-032는 Paused지만 기술 실패가 아니다. 사용자가 화면을 직접 확인할 수 있을 때 Defense Production Panel USER Visual → Pawn Rebind USER Visual부터 재개한다.
- CF-FQ-032의 이미 완료된 technical Automation과 Launcher USER PASS는 관련 Source 변경이 없다면 재실행하지 않는다.
- Inventory M5의 작업 전용 Automation runner는 공용 Tool로 자동 승격하지 않는다.
```

### v2.42.3 적용 안내

```text
- CF-FQ-032 상세 technical evidence와 next gate owner는 Document/Plan/InGameUIPlan.md v0.58.5다.
- Defense 저장 맵 PIE Automation과 Pawn Rebind Technical Automation은 Success지만 사용자 시각 확인을 대신하지 않는다.
- 다음 검증은 Defense Production Panel USER Visual → Pawn Rebind USER Visual 두 항목만 수행한다.
- 완료된 targeted Automation은 관련 Source가 다시 바뀌지 않는 한 사용자 시각 확인을 위해 반복하지 않는다.
- Tools/RunUIAutomation.ps1은 이번 검증의 작업 전용 Execution Method이며 공용 저장소 Tool로 자동 승격하지 않는다.
```

### v2.42.0 적용 안내

```text
- CF-FQ-032 상세 작업 상태 owner는 Document/Plan/InGameUIPlan.md v0.58.0이다.
- 공통 Weapon/Launcher Presentation lifecycle 교정, 공식 Editor Build와 전체 Automation 64/64 PASS는 완료됐으므로 반복하지 않는다.
- 다음 Gate는 /Game/Maps/TestMap_DRSalvo USER PIE이며 SALVO 진행 → SALVO 4 / 4 → Cooldown → READY를 사용자 확인한다.
- Launcher USER PASS 전에는 Defense 실제 변화와 Pawn Rebind로 넘어가지 않는다.
- CF-FQ-031은 Done이며 Heavy·Ripple Ammo USER PIE를 반복하지 않는다.
- UI-P0-03 또는 CF-FQ-032 전체 완료는 Defense 실제 변화와 Pawn Rebind까지 USER PASS한 뒤에만 판정한다.
```

### v2.41.0 적용 안내

```text
- CF-FQ-031은 Done이며 AMMO-P0-00~08과 Heavy·Ripple USER PIE를 반복하지 않는다.
- Ammo 현재 구현은 Document/Systems/Combat/Ammo.md v1.0.0을 우선한다.
- 현재 Active는 CF-FQ-032 UI-P0-03이다.
- 다음 사용자 검증은 Launcher 전체 Presentation(Salvo·terminal Cooldown→READY) → Defense 실제 변화 → Pawn Rebind 순서다.
- 위 범위 전체 USER PASS 전에는 UI-P0-03 또는 CF-FQ-032를 Done 처리하지 않는다.
- CF-FQ-029 Launcher 전체 기능은 별도 Paused 체크포인트를 유지하며 Ammo 완료로 자동 Done 처리하지 않는다.
```

### v2.40.0 적용 안내

- CarFight의 현재 Engine Authority는 `ProjectSSOT → UE 5.8 Source Build → D:\UnrealEngine_Source` 순서로 해석한다.
- `Document/Plan/EngineSourceBuild/`는 현재 Plan이나 Engine Baseline으로 복원하지 않고 Historical / Superseded 기록으로만 읽는다.
- 공용 UE SSOT에서 특정 마이너 버전이 언급되더라도 CarFight의 실제 버전 판단에는 ProjectSSOT Engine Baseline을 우선한다.
- 버전 민감 API·플러그인·엔진 동작은 가능하면 실제 `D:\UnrealEngine_Source` 소스를 최종 근거로 검증한다.

### v2.39.0 적용 안내

- CarFight 관련 새 세션과 도구 작업은 `UE 5.8 + Source Build + D:\UnrealEngine_Source`를 하나의 공식 엔진 기준으로 사용한다.
- 과거 UE 5.7 문구는 현재 버전 확인 근거로 사용하지 않으며, 명시적 엔진 업그레이드 결정 없이 다른 버전으로 자동 전환하지 않는다.
- 빌드와 에디터 실행 진입점은 기존 `Tools\BuildEditor.bat`, `Tools\RunEditor.bat`를 그대로 유지하므로 코드·에셋 마이그레이션은 없다.

### v2.38.0 적용 안내

- UI-P0-02의 Pause UI 상호작용 범위는 사용자 Standalone PASS다.
- UI Root는 Legacy Pawn HUD와 같은 `AddToViewport` 계층의 ZOrder 100을 사용한다.
- 최신 사용자 직접 Editor 빌드와 Combat Automation `595e0c1ca0f640b69f63964bdfd22277` 전체 46/46·필수 24/24 Success를 기준으로 한다.
- Gamepad, 입력 유지 잔류, Ripple·Salvo, Projectile·추진·World Timer와 Root 수명 사용자 PIE가 남아 있으므로 UI-P0-02 전체 완료나 UI-P0-03 착수로 판정하지 않는다.

### v2.37.0 적용 안내

```text
- 현재 Active 작업은 CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크다.
- 대표 Plan은 Document/Plan/InGameUIPlan.md v0.7.0이다.
- UI-P0-01A~02 코드·Build·Automation은 완료됐으므로 UI-P0-02 사용자 PIE부터 재개한다.
- UI-P0-02 PASS 뒤 UI-P0-03 Vehicle·Weapon·Defense·Target·Radar·Alert HUD 데이터 계약으로 진행한다.
- CF-FQ-033은 Done이며 현재 구현은 VehicleDefense.md와 HitDamage.md를 우선한다.
- UI는 Shield·Armor·관통·Integrity 계산을 읽기 전용으로 소비하고 재구현하지 않는다.
- CF-FQ-029·026 Paused와 CF-FQ-030·031·034·035 Ready 체크포인트를 유지한다.
```

### v2.20.0 적용 안내

```text
- 현재 Active 작업은 CF-FQ-029 모듈형 런처 및 발사 인계다.
- 첫 코드 진입점은 LM-P0-01 Projectile Launch Handoff Foundation이다.
- CF-FQ-030은 CF-FQ-029 선행의 Ready 상태다.
- CF-FQ-019는 Deferred이며 현재 Candidate로 복원하지 않는다.
- 기존 CF-FQ-019 테스트 기록은 런처·미사일 통합 회귀 재설계의 참고 입력으로 유지한다.
```

### v2.19.0 적용 안내

```text
- CF-FQ-028은 Done / CF-TC-024 PASS이며 Active로 복원하지 않는다.
- 발사체 추진과 지속형 FX의 현재 구현 판단은 Document/Systems/Combat/Projectile.md를 우선한다.
- ProjectilePropulsionPlan.md는 완료 이력으로 유지한다.
- 현재 자동 선택된 Active 작업은 없다.
- CF-FQ-019는 다음 Candidate지만 사용자가 선택하기 전에는 착수하지 않는다.
- CF-FQ-027 별도 전체 체크리스트와 CF-TC-023은 Paused 상태를 유지한다.
- 이번 Systems 승격은 반복 전투 회귀 완료를 의미하지 않는다.
```

### v2.17.0 적용 안내

```text
- 현재 Active 작업은 CF-FQ-028 발사체 추진 시스템이다.
- 대표 체크포인트는 Document/Plan/ProjectilePropulsionPlan.md다.
- PP-P0-00~07 C++ Foundation과 공식 Editor 빌드는 완료됐으므로 같은 코드를 다시 작성하지 않는다.
- 다음 실행은 Editor 테스트 Rocket ProjectileData 생성, PropulsionConfig와 FX_Exhaust 저장, 사용자 PIE다.
- 기존 ProjectileData는 bUsePropulsion=false 기본값으로 기존 속도 비행을 유지한다.
- CF-FQ-027은 Editor 자산 연결 단계에서 Paused, CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
- 사용자 PIE PASS 전에는 Projectile Propulsion을 Systems Current로 승격하거나 CF-TC-024 PASS로 판정하지 않는다.
```

### v2.14.0 적용 안내

```text
- 새 CarFight 코드 작업은 CodeWorkGate.md v2.0에 따라 현재 AI 세션이 직접 구현한다.
- 현재 Active 작업은 CF-FQ-027 투사체 비행 FX다.
- PFX-P0-00 설계 준비는 완료됐으므로 PFX-P0-01 ProjectileData 계약부터 시작한다.
- TaskSource와 WorkOrder는 구현 참고 자료이며 최종 Codex YAML을 기다리지 않는다.
- 구현 후 Git diff, 공식 Editor 빌드, Automation과 사용자 PIE를 단계별로 검수한다.
- CF-FQ-024 Done과 CF-FQ-026 TS-P0-08 Paused 상태는 유지한다.
```

### v2.13.0 적용 안내 — 폐기됨, v2.14.0이 대체

> 아래 내용은 당시 실행 기준 보존용이며 현재 코드 작업 판단에는 사용하지 않는다.

```text
- 새 세션은 CF-FQ-027과 Document/Plan/ProjectileFlightFxPlan.md를 현재 Active 기준으로 복원한다.
- PFX-P0-00 설계·TaskSource·WorkOrder 초안 준비는 완료됐으므로 반복하지 않는다.
- 현재 최종 Codex YAML은 Missing이며 plan.* 품질·증거 게이트 전에는 소스 변경을 시작하지 않는다.
- CF-FQ-024는 Done 상태와 Current CombatFx System을 유지한다.
- CF-FQ-019는 CF-FQ-027 완료 후 후속 Candidate다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v2.12.0 적용 안내

```text
- CF-FQ-024는 Done / User PIE PASS / CF-TC-021 PASS다.
- 전투 FX 현재 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- CombatFxAudio Plan은 완료 당시 설계·자산 선택·튜닝 기록으로 유지한다.
- NS_BasicHit Scale 미반응 분석과 NS_Impact_1 튜닝을 P0 작업으로 다시 열지 않는다.
- 현재 Active 작업은 없으며 CF-FQ-019는 사용자 선택 시에만 Active로 전환한다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v2.11.0 적용 안내

```text
- 새 세션은 Impact P0 자산을 NS_BasicHit으로 복원하며 Scale 미반응 분석을 다시 열지 않는다.
- DA_FX_ProtoShellImpact는 FxScale 1,1,1 기준으로 두고 현재 Niagara 내부 크기를 그대로 사용한다.
- Destroyed FX는 각 차량 SM_Body StaticMesh의 FX_Destroyed 소켓 위치를 사용한다.
- 소켓 누락 시 SM_Body Bounds 중심 Fallback을 허용하며 위치 오프셋 공통값으로 차량 차이를 해결하지 않는다.
- 다음 작업은 후보 조사나 Preview 튜닝이 아니라 최종 통합 PIE 한 번이다.
- PIE PASS 후에만 CF-FQ-024를 Done으로 전환하고 Document/Systems/Combat/CombatFx.md를 생성한다.
```

### v2.10.0 적용 안내

```text
- 새 세션은 ACFCombatFxPreviewActor v1.1.0과 Build Job 89b6d82710af47e89a4c5b3823037d4e PASS 상태로 복원한다.
- FX 크기는 PreviewUniformScale과 자동 반복 재생으로 먼저 조정한다.
- Override 후보 확정 시 DataAsset 적용 버튼으로 Niagara와 튜닝값을 함께 반영할 수 있다.
- NS_AOE_Explosion_1은 수동 확인 전까지 최종 Destroyed 후보로 해석하지 않는다.
- SmokeBuilder Cascade 자산을 위해 현재 Niagara 계약을 확장하지 않는다.
- 실제 FX 참조 연결은 Editor Details에서 확인하고 저장 후 최종 PIE를 진행한다.
```

### v2.9.0 적용 안내

```text
- 새 세션은 ACFCombatFxPreviewActor와 MaximumLifetimeSeconds 안전 퓨즈가 구현·빌드 완료된 상태로 복원한다.
- 공식 Preview Tool 빌드 증거는 Build Job c9682d4a0be24ead95e87a5a7e8b5849 / Exit Code 0이다.
- /Game/CarFight/FX/Data의 기존 CombatFxData 3개를 재생성하지 않는다.
- FX 크기·회전·위치 보정은 Preview Actor에서 먼저 수행하고 DataAsset에 적용·저장한다.
- 현재 기준 VehicleData는 DA_TestSedan이며 DA_PoliceCar를 신규 FX 연결 대상으로 사용하지 않는다.
- NS_Explossion은 Loop 부적합 후보이므로 비Loop Destroyed 후보로 교체한다.
- MaximumLifetimeSeconds는 영구 잔류 방지 안전 퓨즈이며 Loop 자산 채택 근거가 아니다.
- 사용자 PIE PASS 전에는 CF-FQ-024를 Done 또는 Current System으로 승격하지 않는다.
```

### v2.8.0 적용 안내

```text
- 새 세션은 Combat FX C++와 Editor 빌드가 완료된 상태로 복원한다.
- 공식 성공 증거는 Build Job bd5a50bf388049ec84123aec4942ae96 / Exit Code 0이다.
- 다음 작업은 CombatFxData 3개 생성과 DA_ProtoTurretCannon, DA_HeavyShell, 현재 기준 DA_PoliceCar 참조 연결이다.
- FX DataAsset 또는 Niagara 미연결 상태는 기존 전투 판정 실패가 아니다.
- 사용자 PIE PASS 전에는 CF-FQ-024를 Done 또는 Current System으로 승격하지 않는다.
```

### v2.7.0 적용 안내

```text
- 새 세션은 CF-FQ-024와 CombatFxAudio/ImplementationDesign.md를 우선 복원한다.
- 첫 작업은 AssetPreparationChecklist.md에 따라 반입된 FAB 콘텐츠를 조사하는 Phase 0이다.
- CF-FQ-026은 TS-P0-08 Paused이며 FX 작업 중 TargetSelect 구현과 검증 증거를 정리하거나 되돌리지 않는다.
- FAB Niagara 후보가 확정되기 전에는 Combat FX C++ 구현과 DataAsset 생성을 시작하지 않는다.
- 게임 오디오 자산·클래스·모듈과 청각 테스트를 추가하지 않는다.
```

### v2.6.0 적용 안내

```text
- 새 세션은 CF-FQ-026 TargetSelect를 우선 복원한다.
- CF-FQ-024는 사운드를 제외한 전투 FX 전용 Ready 기능이다.
- CombatFxAudio 경로명은 유지하지만 UCFCombatFxData와 UCFCombatFxComp 같은 FX 전용 명칭을 사용한다.
- 게임 오디오 자산·클래스·모듈과 테스트 조건을 추가하지 않는다.
```

### v2.5.0 적용 안내

```text
- 새 세션은 CF-FQ-026과 Document/Plan/TargetSelectPlan.md를 우선 복원한다.
- TS-P0-01 소스와 빌드 성공을 P0 기능 완료나 Systems 승격으로 해석하지 않는다.
- 기존 전투 FX / Audio 계획과 완료 전투 Systems는 삭제하거나 재작성하지 않는다.
- CF-FQ-024는 TargetSelect P0 진행 후 재개 가능한 Ready 작업이다.
```

### v2.4.0 적용 안내

```text
- CF-FQ-025는 Done / User PIE PASS이며 현재 구현 판단은 AimReticle, VehicleAim과 VehicleDebugPanel Systems 문서를 우선한다.
- ReticleAimDirection Plan은 완료 당시 설계 기록이며 현재 활성 작업 복원에는 사용하지 않는다.
- 현재 활성 작업과 신규 구현 시작점은 계속 CF-FQ-024 CombatFxAudio Plan이다.
```

### v2.3.0 적용 안내

```text
- 2026-06-18 이하 상태는 과거 싱글 전환 기록으로 유지한다.
- 현재 진행 판단은 2026-07-15 상태 섹션과 CF-FQ-024 대표 Plan을 우선한다.
- 완료된 발사·피격·피해 Systems는 유지하며 실제 FX / Audio가 이미 구현된 것으로 해석하지 않는다.
- CF-FQ-024 완료 전에는 신규 CombatFxAudio Systems 문서를 Current로 승격하지 않는다.
```
