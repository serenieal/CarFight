# CarFight — 01_ProjectState

> 역할: CarFight 프로젝트의 **현재 실제 기준선 / 임시 운영 편차 / 현재 리스크**를 고정한다.
> 공통 규칙 원본: `Document/SSOT/`
> 문서 버전: v2.35.0
> 마지막 정리(Asia/Seoul): 2026-08-02


---

## 먼저 확인할 방향
- 최종 방향은 `00_Vision.md`를 기준으로 본다.
- 이 문서는 그 방향 아래에서 **지금 실제로 굴러가는 상태만** 적는다.

---

## 2026-08-02 현재 프로젝트 상태

### 1. 현재 활성 작업

현재 단일 CarFight 활성 작업은 사용자가 명시적으로 전환한 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다.

```text
현재 Active: CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
현재 단계: UI-P0-01A~02 Code Complete / Official Build·Automation PASS / User PIE Pending / UI-P0-03 Not Started
대표 Plan: Document/Plan/InGameUIPlan.md v0.6.0
Source Changes: Applied — PlayerController·LocalPlayer UI Subsystem·World별 C++ Root·8개 레이어·C++ Pause Menu·차량 입력 중립화·Runtime Ready 분리·Input Context 소유권 명시·Automation·GameMode 연결
Asset Changes: None
Official Editor Build: 4f2daa07187a41aea7c400386010b2a0 / Exit Code 0
Automation: 6f0a9a5ebde5472b8b784a9103ce484d / 전체 32 Success·0 Failed / 필수 17/17 Success
Pause Runtime: 실제 World Pause / Continue·Back / Drive·Look·Pressed Key 중립화 / Launcher·Projectile·Timer 상태 보존
Legacy Preservation: 기존 AimReticle·TargetSelect 변경 없음
PIE: Not Run / 실제 Pause·Focus·입력 잔류·진행 재개 User Pending
다음 단계: UI-P0-03 HUD 데이터 계약 Not Started
이전 Active: CF-FQ-029 LM-P0-06 / Paused Checkpoint Preserved
피팅·인벤토리 보호: CF-FQ-034 FIT-P0-05 Initial Sortie Adapter Code Complete·Build·Automation PASS / Physics PIE·Mobility Pending, CF-FQ-035 INV-P0-04 Done / Field Fitting Coordinator Not Started
Runtime Ready: bVehicleRuntimeReady=Vehicle Core 호환 / bVehicleCombatRuntimeReady=Aim·Weapon·Launcher·TargetSelect 포함
Input Context: 차량 Gameplay=Pawn DefaultInputMappingContext 소유 / Controller GameplayInputMappingContext=None 유지
일시중지 유지: CF-FQ-026 TargetSelect TS-P0-08
```

`UI-P0-01A`는 Pawn과 독립된 Pause·Back 요청, Controller 소유 System·UI Context, Pawn 소유 Vehicle Gameplay Context, Possession 통지와 Input Mode·Cursor·Focus 기반을 제공한다.
`UI-P0-01B`는 `UCFUISubsystem`과 World별 `UCFUIRootWidget`을 통해 Game·HUD·Screen·Panel·Menu·Modal·System·Debug 레이어를 제공한다.
`UI-P0-02`는 실제 World Pause, 차량 입력 잔류 제거, C++ Pause Menu와 Launcher·Projectile·Timer 진행 정지 구조를 제공한다.
다만 실제 입력 장치, Focus, 물리·시퀀스 재개 사용자 PIE가 남아 있으므로 Current System 또는 `CF-FQ-032 Done`으로 판정하지 않는다.

`CF-FQ-029`는 Launch Handoff, 가변 Muzzle, SingleCycle, Ripple·Salvo Scheduler, Direct·Angled·Vertical Release와 `LM-P0-05 Launcher Editor Assets` 적용·독립 AssetDump 검증을 완료했으며 현재 단계는 `LM-P0-06 Launcher Integration PIE`다.
`CF-FQ-030`은 `Ready for Manual PIE` 상태다. `MG-P0-01~04` Direct Release·단일 Muzzle Runtime, Missile Flight·Guidance 컴포넌트, TargetActor 제한형 유도, 목표 소실·오버슈트·Pool Reset과 전용 테스트 자산이 적용됐다. Launcher Ripple·Salvo는 첫 발사 순간 `CommandTargetLocation`과 약한 `GuidanceTargetActor`를 함께 Snapshot해 차량의 이후 선택 변경과 같은 Volley의 미사일 목표를 분리한다. 공식 Editor Build `ce88150dfbbc495d91d1e2b8cd7c5225`가 Exit Code 0으로 통과했고 전체 CarFight Automation `85c1e7285a7946fa8f55182b6247f0f4`는 43/43, 필수 회귀는 21/21 Success였다. 정지·측면 이동·선택 변경·목표 파괴·오버슈트·Pool 재사용 사용자 PIE가 남아 있으므로 Done 또는 Systems Current로 판정하지 않는다.
`CF-FQ-019`는 사용자 결정에 따라 Deferred로 이동했으며, CF-FQ-029·030 이후 런처·미사일을 포함한 통합 회귀 범위로 다시 설계한다.
`CF-FQ-028`은 구현, 공식 Editor 빌드, 사용자 PIE와 Systems 승격을 완료해 Done으로 유지한다.
현재 발사체 추진과 지속형 FX의 구현 판단은 `Document/Systems/Combat/Projectile.md`를 우선한다.
`CF-FQ-027`은 Trail-only·Thruster-only·동시 FX, 유효 소켓·Missing Socket Fallback, 종료 Reset, Pool 20발 이상, Ribbon History와 30 FPS 고속 Bounds 사용자 PIE를 완료했으므로 Done이며 `CF-TC-023 PASS`로 유지한다.
`CF-FQ-026`은 `TS-P0-00~07 Done / TS-P0-08 P0 검증과 튜닝 Paused` 상태로 보존한다.

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

현재 단일 Active 작업은 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`다.

```text
현재 Active: CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
- 완료 유지: UI-P0-01A~02 Code Complete / Official Build·Automation PASS
- 사용자 검증: 실제 Pause·Focus·입력 잔류·Launcher·Projectile·Timer 재개 PIE Pending
- 다음 코드 단계: UI-P0-03 HUD 데이터 계약 Not Started
- Runtime Ready: bVehicleCoreRuntimeReady와 bVehicleCombatRuntimeReady 분리
- 입력 소유권: Pawn Vehicle Gameplay Context / Controller System·UI Context

보호된 Ready·Paused 작업
- CF-FQ-034: FIT-P0-05 Initial Sortie Adapter Code·Build·Automation PASS / Physics PIE·Mobility Pending
- CF-FQ-035: INV-P0-00~04 Done / Field Fitting Coordinator Not Started
- CF-FQ-029: LM-P0-06 체크포인트 보존 / Paused
- CF-FQ-026: TS-P0-08 체크포인트 보존 / Paused
- CF-FQ-030·031·033: 기존 Ready 상태 유지
- CF-FQ-019: Deferred / 런처·미사일 이후 통합 회귀 재설계
```

`CF-FQ-024`, `CF-FQ-027`과 `CF-FQ-028`은 Done 상태를 유지하며 완료된 전투 FX·비행 FX·추진을 새 기능 안에서 재구현하지 않는다.

### 5. 일시중지·병행 작업 해석

`CF-FQ-029`와 `CF-FQ-026`은 취소된 작업이 아니며 각각 LM-P0-06과 TS-P0-08 체크포인트를 보존한다.
`CF-FQ-034`와 `CF-FQ-035`는 Ready 기능으로서 완료된 Adapter 기반을 보호하지만, 현재 Active를 전환하거나 Field Fitting Runtime을 자동 착수하지 않는다.
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
