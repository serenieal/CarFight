# CarFight — 02_Roadmap

> 역할: CarFight 프로젝트의 **현재 해야 할 일 / 우선순위 / 완료 조건 / 진행 순서**를 고정한다.
> 기준 상태 문서: `01_ProjectState.md`
> 상위 방향 문서: `00_Vision.md`
> 문서 버전: v2.28.20


> 마지막 정리(Asia/Seoul): 2026-08-18


---

## 이 로드맵의 성격
이 문서는 아이디어 모음이 아니다.

이 문서의 목적은 아래 4가지다.
- 문서 방향을 먼저 정렬한다.
- 확인된 싱글 실행 / 로컬 Aim / Fire / Reticle 기반 위에 전투 루프를 올린다.
- 기준 차량 1대의 무기 조준, 발사, 피격, 피해, 피드백 흐름을 검증한다.
- 서버/멀티/관리툴 작업을 현재 일정에서 제외해 개발 기간을 단축한다.

---

## 이번 사이클의 최상위 목표
이번 사이클의 목표는 아래 한 문장으로 고정한다.

> **서버/멀티플레이 범위를 현재 일정에서 제외하고, 싱글 플레이 기준 1대 차량의 조준, 발사, 피격, 피해, 피드백이 반복되는 핵심 전투 루프를 검증한다.**

이번 사이클은 2~3개월 일정 단축을 목표로 한다.
서버 권한 전투, Dedicated Server 검증, 2클라 테스트, CFNetSmooth 차량 적용, 서버 런처, 세션/로비는 현재 범위에서 제외한다.

---

## 현재 우선순위 상태 (2026-08-18)


`CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`가 현재 단일 Active다. 사용자가 UI를 프로젝트 최우선 사항으로 재지정했다. 기존 UI-P0-02 Runtime Lifetime USER PASS와 UI-P0-03 Speed/Weapon Cooldown/Target 선택·해제/Ammo·Ripple·Salvo USER PASS, Defense Runtime·Pawn Rebind 기술 Automation은 보존하고 `Defense Production Panel USER Visual → Pawn Rebind USER Visual`에서 재개한다. `CF-FQ-038`은 `DAUTH-P0-08A~M + P0-09~11 Technical PASS / UA-01~02 USER PASS / P0-12 USER PASS 2 / 다음 UA-03` 체크포인트를 보존한 Paused다.


```text
현재 Active: CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
대표 Plan: InGameUIPlan.md v0.58.6
현재 Gate: UI-P0-03 Production Runtime PIE Closure
즉시 재개: Defense Production Panel USER Visual → Pawn Rebind USER Visual
후속 순서: UI-P0-04 AimReticle UISubsystem 통합 → UI-P0-05 TargetSelect Marker 통합
Radar/TargetPanel Sensor Source: Systems/Targeting/SensorContact.md v1.1.0
Paused DAUTH: CF-FQ-038 / DataAuthoringPlan.md v0.2.24 / DataAuthoringRoadmap.md v0.1.32
DAUTH 완료 Gate: DAUTH-P0-11 Technical Validation — Technical PASS
DAUTH Core Technical Validation: PASS
DAUTH Frozen 24.90~24.94 Completeness: PASS
DAUTH 보존 Gate: DAUTH-P0-12 USER Authoring Acceptance — Paused / USER PASS 2 / UA-01~02 PASS / 다음 UA-03

P0-08A: Editor-only Recipe + 5 Profile / Never-Cook / Common Types — Technical PASS
P0-08B: Stable Field Path / Field Value Codec / UCFVehicleData 117 Registry Coverage — Technical PASS
P0-08C: Immutable Snapshot / deterministic fingerprint/hash / RequiredDependencies — Technical PASS
P0-08D: Chassis Socket / Wheel Bounds Asset Snapshot Reader / resolver-relevant fingerprint — Technical PASS
P0-08E: Snapshot-only Pure Resolver / Frozen R0~R16 / deterministic source precedence / Trace·Diff·Stale foundation — Technical PASS
P0-08F: RF_Transient Definition Materializer / Stable-ID arrays / FieldCodec import / existing UCFVDAValidator / readback hash consistency — Technical PASS
P0-08G: Existing Definition lossless Legacy Pin import / Mount hidden serialized partition / group·field Adoption Preview / Recipe-only commit — Technical PASS
P0-08H: TOCTOU-safe FCFVehicleApplyService / transient preflight / dependency-safe exact Diff / AppliedState / atomic rollback / no-auto-save — Technical PASS
P0-08I: Common FCFVehicleAuthoringService / R0~R3 typed contract / prospective Recipe preview / guarded Recipe-only write / shared Apply lane / bounded dedupe — Technical PASS
P0-08J: projection-only Batch Registry / Recipe 7 + Profile 78 numeric allowlist / 117 resolved projection / canonical UTF-8 CSV / baseline manifest / ExportSetHash — Technical PASS
P0-08K: manifest/current Registry validation / canonical CSV parse / 3-way merge / ownership conflict / transient Recipe·Profile prospective Resolver preview / BatchPlanHash — Technical PASS
P0-08L: exact Batch approval / global source TOCTOU preflight / B1 Recipe+B2 shared Profile transaction / rollback / approval invalidation — Technical PASS
P0-08M: fresh R3 BatchApplyPlan / exact B3 approval / global preflight / canonical ApplyService sequence / partial result / no global rollback — Technical PASS
P0-09: single-Vehicle Workspace / Initial Import / Recipe basic intent / Resolve·Diff·Trace·Validation / shared Apply / standard Undo / Raw DA Open — Technical PASS
P0-10: Reference Compare / Assets+Layout semantic migration / Measurement+Adoption / Frozen 4-axis Driving Feel / Stable-ID Mount+Defaults / managed Wizard guard / standard Undo — Technical PASS
P0-11 Core: representative E2E / stale approval invalidation / fresh Resolve·Trace·Diff·Validation / Apply+Undo / Raw Drift fail-closed / Inventory+Fitting consumer non-regression — PASS
Final Build: 98dfceab797942218bd09c844e398ceb / Exit Code 0
Focused P0-11 Automation: b53998e6cacf48a1a554d784b77e013c / 6/6 PASS / 0 FAIL
Focused Result JSON SHA-256: bce3414ec30cb4612209ff1f39f649990140950b73d4136929a3fa8a4be47263
Inventory representative pre-closure evidence: 4dfa10d20f00451abcab209b0db49d40 / 1/1 PASS
Fitting representative pre-closure evidence: 1c66a7139313458f9c5ab68ff03a7b0f / 2/2 PASS
Targeted Automation: 77b45525549b4866995b3d972fb4a9fa / 63/63 PASS / 0 FAIL
Result JSON SHA-256: a4ef815caceb5fb5f413b849393f2d5e8ac1e45929f7f5a3cf1f50347bc0bb1f
Runtime UCFVehicleData / Inventory / Fitting 변경: 0
SCFVDAWizardTab 삭제/대체: 0 / legacy Wizard 병행 유지
Legacy managed-target guard: Layout Capture / Quick Tune Apply / Quick Tune Revert disabled + handler recheck
Vehicle Authoring Workspace: CarFight.VehicleAuthoring / P0-10 parity + Nomad Tab spawn PASS
P0-10 USER Visual/Usability/Driving Feel Acceptance: 미판정 / P0-12까지 보존
DG/DEL Gate: 미개방
Batch main page: 0
Content Asset / Asset Snapshot Reader 변경: 0
Existing UCFVDAValidator source 변경: 0
Raw SetField / direct Target writer / force / skip-validation: 0
Production Target writer: FCFVehicleApplyService only
Automatic Retry / Automatic Save: 0 / 0
Batch Import Session / 3-way Preview: Technical PASS
B1·B2 source commit: Technical PASS / Target mutation 0 / Auto Save 0
B3 Definition Apply: Technical PASS / Target writer FCFVehicleApplyService only / global transaction·rollback 0 / Auto Save 0
Batch UI / file save: 0 / 0
Frozen 26.66~26.69 BatchOperationId/generic envelope: external Batch transport/client 전 follow-up
P0-09/P0-10 USER Visual/Usability/Driving Feel Acceptance: 미판정 / P0-12까지 보존
P0-11 Frozen completeness closure: Handling/Performance Adoption / Shared Profile edit+impact / External Drift 3-way recovery / Mesh-only Candidate+Create Vehicle From Mesh — Technical PASS
P0-12 USER Authoring Acceptance: Paused / UA-01~02 USER PASS / USER PASS 2 / 다음 UA-03
보존 USER Gate: UA-03 Recipe / Shared Profile / Affected Vehicle Impact

CF-FQ-034: Paused / VehicleFittingPlan.md v0.17.0 / FIT-P0-07D USER Driving Feel Comparison
CF-FQ-029: Paused / 기존 LM-P0-06 USER PIE 체크포인트 보존
CF-FQ-037: Done / SCAN-P0-00~07 PASS / `Systems/Targeting/SensorContact.md v1.1.0` Current

CF-FQ-030: Ready / CF-TC-027 Manual PIE Pending
```

CF-FQ-030의 과거 WinError 87 readback 공백은 current AssetDump로 닫았다. 다만 MissileDirectTest World의 전체 Actor label은 current public readback 범위 밖이고 live SceneTools.find_actors도 server policy blocked였으므로 다섯 `MissileTarget_*` label 자체는 독립 PASS로 확대하지 않는다. 실제 정지·측면 이동·선택 변경·목표 파괴·오버슈트·Pool 재사용은 CF-TC-027 Manual PIE에서 확인한다.

CF-FQ-036 완료 뒤에도 다음 체크포인트는 그대로 보호한다.

```text
CF-FQ-015 Paused / VD-P0-04 USER Tuning Pending
CF-FQ-026 Paused / TS-P0-08 USER PIE Pending
CF-FQ-029 Paused / LM-P0-06A Technical PASS / LM-P0-06 USER PIE Pending
CF-FQ-030 Ready / Persisted Asset Technical Verification PASS / CF-TC-027 Manual PIE Pending
CF-FQ-032 Active / Defense·Pawn Rebind USER Visual current / Radar·TargetPanel USER Visual 후속

CF-FQ-034 Paused / FIT-P0-07C Quantitative Mobility Technical PASS / FIT-P0-07D USER Driving Feel·Field UI Pending
CF-FQ-035 Paused / USER Field UI·Mobility Pending
CF-FQ-037 Done / SCAN-P0-00~07 PASS / `Systems/Targeting/SensorContact.md v1.1.0` Current
CF-FQ-038 Paused / DAUTH-P0-08A~M + P0-09 + P0-10 + P0-11 Technical PASS / Frozen 24.90~24.94 Completeness PASS / UA-01~02 USER PASS / P0-12 USER PASS 2 / 다음 UA-03


```

특히 Sensor Technical Acceptance는 다음을 완료로 대체하지 않는다.

```text
CF-FQ-032 Radar Range / Zoom
CF-FQ-032 동적 Radar Blip
CF-FQ-032 Radar / TargetPanel USER Visual
CF-FQ-026 TS-P0-08 USER PIE
```

아래 `2026-08-13` 및 그 이전의 “현재 Active” 문구는 당시 체크포인트를 보존하는 Historical 기록이다. 현재 착수 판단은 이 2026-08-16 섹션, `01_ProjectState.md`, `03_FeatureQueue.md`, `ActiveWork.md`와 Current Systems를 우선한다.

---

## Historical 우선순위 체크포인트 (2026-08-13)

`CF-FQ-031 차량 탄약·재장전 런타임`은 AMMO-P0-00~08, Heavy·Ripple USER PIE와 Ammo Current System 승격까지 완료해 Done이다. 현재 단일 Active는 다시 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`이며 UI-P0-03 Production Runtime PIE의 남은 Launcher 전체 Presentation·Defense·Pawn Rebind 검증을 계속한다.

```text
1. CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크: Active / UI-P0-02 USER PASS / UI-P0-03 Source·Build·Automation PASS / Production Runtime PIE Partial USER PASS / Launcher 전체 Presentation·Defense·Pawn Rebind Pending
2. CF-FQ-029 모듈형 런처 및 발사 인계: Paused / LM-P0-06 체크포인트 보존
3. CF-FQ-030 물리 제한형 미사일 비행·유도: Ready for Manual PIE / MG-P0-01~04 Runtime Applied
4. CF-FQ-031 차량 탄약·재장전 런타임: Done / AMMO-P0-00~08 Done / Heavy·Ripple USER PIE PASS / Ammo Systems Current
5. CF-FQ-034 차량 피팅·질량 런타임: Ready / FIT-P0-05 Code·Build·Automation PASS / Physics PIE Pending
6. CF-FQ-035 인벤토리 Foundation: Ready / INV-P0-00~04 Done / Field Fitting Coordinator Pending
7. CF-FQ-033 차량 방어·손상 런타임: Done / DR-P0-00~07 Done / DR-PIE-00~06 USER PASS / Systems Current
8. CF-FQ-028 발사체 추진 시스템: Done / User PIE PASS / CF-TC-024 PASS / Systems Current
9. CF-FQ-027 투사체 비행 FX: Done / User PIE PASS / CF-TC-023 PASS / Systems Current
10. CF-FQ-026 타겟 선택 시스템: Paused / TS-P0-08 체크포인트 보존
11. CF-FQ-020 조작감, 전투 템포, 피드백 개선: Candidate
12. CF-FQ-021 핵심 게임 루프 검증: Candidate
13. CF-FQ-019 주행과 전투 흐름 반복 테스트: Deferred / 런처·미사일 이후 재설계
```

현재 작업은 `CF-FQ-032`다. UI-P0-02는 전체 USER PASS했고 UI-P0-03의 Source·Build·Automation과 일부 Production Runtime PIE도 PASS했다. CF-FQ-031 완료로 finite Ammo·Reload 실제 Snapshot까지 WeaponPanel에 연결됐다. 다음 순서는 `Launcher Presentation 전체 회귀(Salvo + terminal Cooldown→READY) → Defense 실제 변화 표시 → Pawn Rebind → 전체 USER PASS 후 UI-P0-03 완료 판정`으로 고정한다. 완료된 Ammo·Defense 계산은 UI에서 읽기 전용으로 소비하고 재구현하지 않는다.

발사체 추진 완료 체크포인트:

```text
- 완료 Feature: CF-FQ-028 Done
- 완료 Test: CF-TC-024 PASS
- Current System: Document/Systems/Combat/Projectile.md v1.5.0
- 완료 Plan: Document/Plan/ProjectilePropulsionPlan.md v1.2.0
- CFProjectileMotorTypes v1.0.0
- UCFProjectileMotorComp v1.0.0
- UCFProjectileData v1.7.1 / PropulsionConfig·독립 FX Scale
- ACFProjectileActor v1.8.1 / MotorComp·Burning 기반 Thruster·Pool Reset
- InitialSpeed 분리 → IgnitionDelay → Burning 고정 방향 가속 → BurnedOut 관성·중력 비행
- 최종 빌드: e8b812bd479549299dd116f9bae8996f / Exit Code 0
- Automation: RuntimeContract 소스 컴파일 PASS / 실행 Not Run
- 사용자 PIE: DA_PFX_ThrusterTest 추진·FX_Exhaust·Scale 0.2 PASS
- 반복 전투 확장 회귀: CF-FQ-019 Deferred / 런처·미사일 구현 범위 확정 후 재설계
```

투사체 비행 FX 완료 체크포인트:

```text
- 완료 Feature: CF-FQ-027 Done
- 완료 Test: CF-TC-023 PASS
- Current System: Document/Systems/Combat/Projectile.md v1.5.0
- 완료 Plan: Document/Plan/ProjectileFlightFxPlan.md v1.0.0
- Trail-only / Thruster-only / Trail+Thruster: PASS
- 유효 소켓 / Missing Socket Fallback: PASS
- Hit·LifeExpired Reset / Pool 20발 이상 / Ribbon History 무잔류: PASS
- 30 FPS 고속 Bounds·컬링과 Impact·Damage·Pool 회귀: PASS
- Automation: RuntimeContract 소스 컴파일 PASS / 실행 Not Run — Runner 미노출
```

`CF-FQ-024`의 일회성 Muzzle·Impact·Destroyed Current System은 다시 열지 않는다.
`CF-FQ-027`과 `CF-FQ-028`의 검증된 지속형 FX·비유도 Rocket 추진 기준은 `Systems/Combat/Projectile.md`를 우선한다.
`CF-FQ-019`는 기존 반복 전투 범위를 그대로 실행하지 않고 Deferred로 이동한다. `CF-FQ-029`와 `CF-FQ-030` 이후 런처·미사일을 포함한 새 통합 회귀 범위로 다시 설계한다.
`CF-FQ-026`의 완료 코드와 Automation 결과는 Paused 체크포인트로 유지한다.

---

## 현재 진행 상태 (2026-07-15)

### 완료된 기준선

- `ProjectSSOT` 루트 정리는 완료됐다.
- `Document/ProjectSSOT/`는 `README + 00~05 + Archive` 구조로 정리됐다.
- 완료 기능과 현재 구현 기준은 `Document/Systems/`를 우선 기준으로 본다.
- 차량 코어 기준선은 `BP_CFVehiclePawn / ACFVehiclePawn / UCFVehicleDriveComp / UCFWheelSyncComp / UCFVehicleData / DA_PoliceCar` 조합으로 유지한다.
- 차량 코어의 기본 주행 / WheelSync / DriveState는 과거 P0 기준에서 PASS로 본다.
- `VehicleCamera`, Local Aim, Fire, Reticle 골격은 현재 싱글 전투 루프의 선행 기반으로 유지한다.
- `Document/Systems/Network/ServerSpawn.md` 기준 Dedicated Server 최소 Spawn/Possess 흐름은 존재하지만 현재 일정에서는 보류한다.

### 현재 진행 체크포인트

2026-07-15 기준 완료 상태:

```text
- C1 차량 무기 조준 및 발사: 완료
- C2-A Reticle / FireFeedback UI: 완료
- C3 피격 판정 및 피해 처리: 완료
- 조준점·터렛·총구 정렬과 고속 Projectile 신뢰성: 완료
```

현재 남은 사이클 과제:

```text
1. 완료된 주행·조준·발사·피격·피해·전투 FX를 반복 전투에서 함께 회귀 검증한다.
2. 전체 FPS·속도 조합과 이동 차량 대상 반복 전투를 확인한다.
3. 전투 템포와 피드백 문제를 기능 차단과 품질 개선으로 분리한다.
4. 핵심 게임 루프가 다음 개발 단계로 넘어갈 수 있는지 최종 판정한다.
```

프로젝트 전역 사운드 정책:

```text
- CarFight는 게임 사운드를 구현하지 않는다.
- SoundWave, SoundCue, MetaSound Source, Sound Attenuation 자산을 제작·도입하지 않는다.
- USoundBase, UAudioComponent와 사운드 재생 코드를 추가하지 않는다.
- 엔진음, 타이어음, 충돌음, 무기음, 피격음, 파괴음, UI음과 BGM은 완료 조건에서 제외한다.
- Unreal의 기본 플랫폼 오디오 설정은 엔진 생성 설정으로 남길 수 있으나 게임 기능으로 간주하지 않는다.
```

따라서 현재 진행 순서는 아래로 갱신한다.

```text
완료: C2-B / CF-FQ-024 전투 FX
완료: PFX / CF-FQ-027 투사체 비행 FX
완료: PROP / CF-FQ-028 발사체 추진 시스템
완료: DMG / CF-FQ-033 차량 방어·손상 런타임 / DR-P0-00~07 / DR-PIE-00~06 / Systems Current
현재 Active: UI / CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크
현재 단계: UI-P0-03 Production Runtime PIE Partial USER PASS
다음 경로: Launcher 전체 Presentation(Salvo·terminal Cooldown→READY) → Defense 실제 변화 → Pawn Rebind → 전체 USER PASS 후 UI-P0-03 완료 판정
완료 Current: AMMO / CF-FQ-031 / AMMO-P0-00~08 / Heavy·Ripple USER PIE / Systems/Combat/Ammo.md v1.0.0
실패 경로: 남은 UI Runtime 검증의 실제 결함만 최소 수정 → 공식 Build·전체 Automation → 해당 영향 PIE 재실행
보호 Paused: LCH / CF-FQ-029 / LM-P0-06
보호 Paused: TGT / CF-FQ-026 / TS-P0-08
보호 Ready: MSL / CF-FQ-030 / Manual PIE
보호 Ready: FIT / CF-FQ-034 / Physics PIE
보호 Ready: INV / CF-FQ-035 / Field Fitting Coordinator
후속 Candidate: C5 / CF-FQ-020 조작감, 전투 템포, 피드백 개선
후속 Candidate: C6 / CF-FQ-021 핵심 게임 루프 검증
후순위 Deferred: C4 / CF-FQ-019 주행과 전투 흐름 반복 테스트
```

완료된 발사체·FX·데미지 기준과 현재 런처·미사일·피팅 체크포인트는 UI-P0-02 검증의 회귀 보호 대상으로 유지한다. `CF-FQ-019`는 Deferred이며 현재 후보 순서에서 착수 대상으로 해석하지 않는다.

### 이번 사이클에서 하지 않을 것

```text
- Dedicated Server 고도화
- 2클라 Spawn/Possess 검증
- 이동 복제 / 원격 차량 보간
- 서버 권한 발사 요청
- CFNetSmooth 차량 적용
- 서버 런처 / 서버 운영 도구
- 로비 / 세션 / 매치메이킹
- 정식 웹 관리툴 선구축
- 대규모 서버 프레임워크 선구축
- 인벤토리 / 보상 / 경제 시스템 선구축
- 새 차종 양산
- CMVS 최종 구조 전환 구현
- 차량 모델링 파이프라인 확정
```

관리툴은 현재 싱글 전투 루프 검증 범위에서 제외한다.

---

## 이번 사이클의 절대 기준

### 먼저 끝내야 하는 것

1. 확인된 싱글 실행 / 기준 차량 조작 / 로컬 Aim / Fire / Reticle 기반을 유지한다.
2. 기준 차량 1대에 최소 무기 조준 / 발사 경로를 구현한다.
3. 발사 성공/불가/쿨다운 같은 상태를 시각 이펙트와 조준 UI로 읽히게 한다.
4. 피격 판정과 피해 처리를 붙여 전투 결과가 남게 한다.
5. 주행 중 조준/발사/피격/피해가 반복되는 테스트 루프를 만든다.
6. 조작감, 전투 템포, 피드백을 한 번에 크게 바꾸지 않고 관찰 가능한 단위로 조정한다.
7. 서버/멀티 관련 Plan은 현재 착수 대상에서 제외하거나 보류 표시한다.

### 아직 하지 않을 것

- 서버 Target 삭제
- `CFMPGameMode` 하드 삭제
- CFNetSmooth 서브모듈 제거
- 서버 관련 Plan 폴더 대량 이동
- 정식 웹 관리툴 구현
- 인벤토리 / 보상 / 경제 시스템 구현
- 탄종/무기 밸런스 대량 설계
- 무기 종류 대량 추가
- 장갑 방향 / 피팅 / 열 / 탄약 경제 전체 구현
- AI 교전 전체 구현
- 새 차종 양산
- CMVS 최종 구조 전환 구현
- 차량 모델링 파이프라인 확정
- 서버 프레임워크를 게임 기능 없이 먼저 크게 만드는 작업

주의:
- 2026-06-19 이전 전환 세션은 기능 추가 없이 싱글플레이 전환 정리만 다뤘다.
- 현재 신규 개발은 싱글 전투 루프 검증 순서로 진행한다.
- 서버 Target, CFMPGameMode, CFNetSmooth, 서버 관련 보관 문서는 삭제하지 않고 Deferred 대상으로 보존한다.

---

## 전체 진행 순서
1. **S0 — ProjectSSOT 싱글 전환**: 완료
2. **S0-1 — 멀티플레이 선로 비활성화**: 완료
3. **S1 — 싱글 실행 기준선 전환**: 완료
4. **C1 — 차량 무기 조준 및 발사 기능 구현**: 완료
5. **C2-A — Reticle / FireFeedback UI 구현**: 완료
6. **C3 — 피격 판정 및 피해 처리 구현**: 완료
7. **C2-B — 전투 FX 구현**: 완료 / User PIE PASS / CF-TC-021 PASS
8. **PFX — CF-FQ-027 투사체 비행 FX**: 완료 / User PIE PASS / CF-TC-023 PASS / Systems Current
9. **PROP — CF-FQ-028 발사체 추진 시스템**: 완료 / User PIE PASS / CF-TC-024 PASS / Systems Current
10. **DMG — CF-FQ-033 차량 방어·손상 런타임**: Active / 작업 완료 최우선 / DR-P0-00~06 Done / DR-P0-07 User PIE
11. **UI — CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크**: Paused / UI-P0-01A~02 Code Complete / User PIE·UI-P0-03 Pending
12. **LCH — CF-FQ-029 모듈형 런처 및 발사 인계**: Paused / LM-P0-06 Checkpoint
13. **MSL — CF-FQ-030 물리 제한형 미사일 비행·유도**: Ready for Manual PIE / MG-P0-01~04 Runtime Applied
14. **FIT — CF-FQ-034 차량 피팅·질량 런타임**: Ready / Physics PIE Pending
15. **INV — CF-FQ-035 인벤토리 Foundation**: Ready / Field Fitting Coordinator Pending
16. **C5 — 조작감 / 전투 템포 / 피드백 개선**: Candidate
17. **C6 — 핵심 게임 루프 검증**: Candidate
18. **C4 — 주행과 전투 흐름 반복 테스트**: Deferred / 런처·미사일 이후 재설계

현재 Active 단계는 DMG / CF-FQ-033이다. 먼저 DR-P0-07을 완료하고, 실패가 있으면 해당 데미지 결함을 최우선 수정한다. DR-PIE-01~06 전체 PASS 뒤 VehicleDefense·HitDamage Systems를 동기화하고 CF-FQ-033을 Done으로 전환한 다음 UI / CF-FQ-032 체크포인트로 복귀한다.

---

## S0 — ProjectSSOT 싱글 전환
### 목표
활성 판단 문서가 모두 싱글 플레이 1대 차량 기준과 이후 로컬 전투 루프 검증 방향을 가리키게 한다.

### 완료 조건
- [x] `00_Vision.md`가 싱글 전환 목표를 명시한다.
- [x] `01_ProjectState.md`가 서버/멀티를 보류 상태로 해석한다.
- [x] `02_Roadmap.md`가 S0, S0-1, S1, C1~C6 순서로 재정렬된다.
- [x] `03_FeatureQueue.md`의 P0가 싱글 전투 루프 작업으로 바뀐다.
- [x] `04_ProjectDecisions.md`에 전환 결정이 기록된다.
- [x] `05_TestChecklist.md`의 Network 항목이 현재 기준 `N/A`로 내려간다.

---

## S0-1 — 멀티플레이 선로 비활성화
### 목표
서버/멀티 관련 문서가 현재 싱글 작업의 활성 계획처럼 읽히지 않게 분리한다.

### 해야 할 일
- [x] `Document/Plan/Archive/MP_ServerPlan`, `ServerUpgradePlan`, `VehicleNetSyncPlan`, `CFNetSmoothPlan`을 보류 계획으로 분류
- [x] `Document/Plan/README.md`와 `Document/Plan/Archive/README.md`에 활성/보관 기준 문서화
- [x] 코드, Target, 플러그인, 배치 파일은 이번 전환 범위에서 삭제/분류하지 않는다고 명시

### 완료 조건
- [x] 현재 착수 계획에서 서버/멀티 문서 항목이 제거되어 있다.
- [x] 남기는 서버 관련 문서는 보류 이유가 문서화되어 있다.
- [x] 서버 관련 코드/툴 파일은 현상 유지하고, 이번 세션에서 삭제/분류하지 않는다는 기준이 문서화되어 있다.

---

## S1 — 싱글 실행 기준선 전환
### 목표
에디터 실행과 PIE 1인 플레이가 서버 GameMode 전제 없이 기준 차량 1대를 바로 조작하는 상태가 되게 한다.

### 해야 할 일
- [x] `DefaultEngine.ini`의 기본 GameMode 연결 확인
- [x] 싱글 기준 GameMode 또는 기존 배치 차량 점유 방식 결정
- `/Game/Maps/TestMap`에서 기준 차량 1대 조작 루프 확인

### 완료 조건
- [x] `Tools\RunEditor.bat` 실행 후 에디터가 열린다.
- [x] PIE 1인 플레이에서 `BP_CFVehiclePawn`을 바로 조작할 수 있다.
- [x] 서버 창, 2클라 창, 네트워크 접속 없이 검증 가능하다.

---

## C1 — 차량 무기 조준 및 발사 기능 구현
### 목표
기준 차량 1대에서 로컬 조준 상태를 실제 발사 경로로 연결한다.

### 범위
- 첫 구현은 단일 기준 차량과 최소 기준 무기 1개로 제한한다.
- 서버 권한 발사, 복제, PvP 판정은 현재 범위에서 제외한다.
- 무기 종류 대량 설계보다 `조준 입력 -> 발사 가능 판단 -> 발사 결과` 경로를 우선한다.

### 해야 할 일
- 현재 `VehicleCamera / VehicleAim / AimReticle` 기반에서 발사 방향과 발사 원점을 추적한다.
- Fire 입력이 성공/실패 사유를 남기도록 정리한다.
- 최소 기준 무기의 쿨다운 또는 연사 제한 위치를 정한다.
- 발사 방식은 상세 Plan에서 확정하되, ProjectSSOT 기준에서는 로컬 발사 루프 존재 여부만 판정한다.

### 완료 조건
- [x] PIE 1인 플레이에서 기준 차량이 조준 방향으로 발사 요청을 만든다.
- [x] 발사 성공 / 발사 불가 / 쿨다운 상태를 구분할 수 있다.
- [x] 발사 원점, 발사 방향, 현재 조준 상태를 디버그 또는 UI로 확인할 수 있다.
- [x] 서버 창, 2클라 창, 네트워크 접속 없이 검증 가능하다.

현재 판정:

```text
CF-FQ-016 Done
CF-TC-013은 전체 확장 회귀 전까지 PARTIAL 유지
```

---

## C2 — 전투 피드백 구현

C2는 UI와 실제 월드 연출의 생명주기를 분리한다.

### C2-A — Reticle / FireFeedback UI

상태: 완료

완료 범위:

```text
- Ready / FireSuccess / Cooldown 상태와 색상
- NoWeapon 회색 표시
- AimBlocked 주황 표시
- TurretAligning amber와 MuzzleBlocked 표시
- 피드백 유지 시간 종료와 정상 상태 복귀
```

현재 판정:

```text
CF-FQ-017 Done
CF-TC-014 PASS
```

### C2-B — 전투 FX

상태: `CF-FQ-024 Done / User PIE PASS / CF-TC-021 PASS`

현재 구현 기준:

```text
Document/Systems/Combat/CombatFx.md
```

목표:
- 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 Niagara 기반 시각 연출로 즉시 이해하게 만든다.

범위:
- 판정과 연출을 분리한다.
- 소스 코드에 특정 Niagara 자산 경로를 하드코딩하지 않는다.
- 기준 무기·발사체·차량의 P0 시각 연출을 우선한다.
- 게임 사운드 자산, 런타임과 오디오 모듈은 프로젝트 전역 범위에서 제외한다.
- 상세 기준은 `Document/Plan/CombatFxAudio/ImplementationDesign.md`를 따른다.
- `CombatFxAudio`는 경로 호환용 레거시 디렉터리명이며 현재 범위에 Audio는 포함하지 않는다.

완료 범위:
- 실제 Muzzle 위치의 발사 Niagara 연결
- `FCFDamageHitContext.ImpactLocation / ImpactNormal` 기반 Impact Niagara 연결
- `SM_Body.FX_Destroyed` 소켓 기반 최초 Destroyed Niagara 연결
- 발사 거부, 중복 Impact와 파괴 후 추가 피해의 FX 중복 방지
- WeaponData / ProjectileData / VehicleData의 CombatFxData 참조
- MaximumLifetimeSeconds Loop 잔류 안전 퓨즈
- Editor Preview Actor와 DataAsset 튜닝 흐름
- 게임 오디오 클래스·모듈·에셋 참조 0개 유지

완료 조건:
- [x] 승인된 발사 1회당 Muzzle FX가 1회 발생한다.
- [x] 발사 거부 상태에서는 발사 FX가 발생하지 않는다.
- [x] HitScan과 Projectile의 첫 Impact에서 FX가 1회 발생한다.
- [x] 최초 차량 파괴 전환에서 `FX_Destroyed` 소켓 위치의 파괴 FX가 1회 발생한다.
- [x] 중복 충돌, 추가 피해와 반복 전투에서 Niagara 중복·잔류가 없다.
- [x] FX 자산 누락 또는 생성 실패가 기존 발사·피해·파괴 판정을 변경하지 않는 null-safe 계약이 구현됐다.
- [x] 게임 오디오 자산·런타임·모듈 참조가 0개다.
- [x] Editor 빌드와 사용자 PIE에서 `CF-TC-021`을 통과했다.

---

## C3 — 피격 판정 및 피해 처리 구현
### 목표
발사 결과가 피격 판정과 피해 처리로 이어져 전투 결과가 남게 한다.

### 범위
- 첫 구현은 최소 목표물 또는 기준 차량 간 피격 판정으로 제한한다.
- 장갑 방향, 모듈 손상, 탄종 상성, 파괴 연출 전체 구현은 현재 범위에서 제외한다.

### 해야 할 일
- 발사 결과가 맞았는지 판단하는 최소 피격 경로를 정한다.
- 피해량 적용 대상과 생존 상태 위치를 정한다.
- 피해 처리 실패 시 원인을 Aim / Fire / Hit / Damage로 분리해 기록한다.
- 완료 후 Systems 승격 후보 문서 경로를 정한다.

### 완료 조건
- [x] 발사 결과가 목표물 또는 차량에 피격으로 기록된다.
- [x] 피해량이 누적되고 현재 체력 또는 파괴 상태를 확인할 수 있다.
- [x] 빗나감 / 피격 / 피해 적용 실패를 구분할 수 있다.
- [x] 서버 없이 로컬 테스트에서 반복 검증 가능하다.

현재 판정:

```text
CF-FQ-018 Done
CF-TC-015 / CF-TC-016 / CF-TC-020 PASS
```

---

## C4 — 주행과 전투 흐름 반복 테스트
### 목표
주행, 조준, 발사, 피격, 피해와 전투 시각 FX가 한 흐름으로 반복되는지 확인한다.

### 범위
- 테스트는 `DA_PoliceCar`와 현재 기준 테스트 맵을 우선 사용한다.
- 새 맵, 새 차종, AI 교전 전체 구현은 핵심 루프가 닫힌 뒤 검토한다.

### 해야 할 일
- 정지 상태, 저속, 중속, 회전 중 발사 테스트를 나눈다.
- 발사 중 주행 조작이 끊기지 않는지 확인한다.
- 휠 시각 품질, 카메라 흔들림, Reticle 가독성, 발사·Impact·파괴 FX를 함께 관찰한다.
- 발사 1회당 FX 횟수, Projectile Pool 재사용과 Niagara 잔류를 확인한다.
- 실패를 Core / Data / Asset / Input / UI / FX / Quality / DocMismatch로 분류한다.

### 완료 조건
- [ ] 주행 중 조준/발사 입력이 끊기지 않는다.
- [ ] 같은 루프를 10회 이상 반복해도 입력, UI, 판정과 FX 상태가 꼬이지 않는다.
- [ ] 발사·Impact·파괴 연출의 중복과 Projectile Pool 잔류가 없다.
- [ ] 조작감 문제와 전투 판정·연출 문제를 분리해 기록할 수 있다.
- [ ] 반복 테스트 결과가 `05_TestChecklist.md`에 반영된다.

---

## C5 — 조작감 / 전투 템포 / 피드백 개선
### 목표
핵심 전투 루프가 기능적으로 도는 상태에서 조작감과 템포를 조정한다.

### 범위
- 조정은 한 번에 하나의 축만 바꾼다.
- 차량 주행값, 무기 발사 템포, 조준 UI와 시각 이펙트를 동시에 크게 바꾸지 않는다.

### 해야 할 일
- 조작감: 주행, 조향, 카메라, 조준이 서로 방해하는지 확인한다.
- 전투 템포: 발사 간격, 쿨다운, 피격 반응이 너무 빠르거나 느린지 확인한다.
- 피드백: 플레이어가 왜 맞았고 왜 못 맞혔는지 읽을 수 있는지 확인한다.
- 튜닝값이 `VehicleData`, 무기 데이터, UI 설정, C++ 기본값 중 어디에 속하는지 기록한다.

### 완료 조건
- [ ] 조작감 문제가 기능 FAIL인지 튜닝 문제인지 분리된다.
- [ ] 전투 템포 조정값의 위치와 이유가 기록된다.
- [ ] 발사/피격/피해 피드백이 플레이어 관점에서 납득 가능하다.
- [ ] 남은 품질 문제는 핵심 루프 차단 여부와 함께 기록된다.

---

## C6 — 핵심 게임 루프 검증
### 목표
현재 싱글 차량 전투 루프가 다음 개발 단계로 넘어갈 수 있는지 판정한다.

### PASS 기준
- [ ] 기준 차량 1대를 조작할 수 있다.
- [ ] 조준 방향을 만들 수 있다.
- [ ] 발사 입력이 성공/실패 상태로 판정된다.
- [ ] 발사 피드백이 화면과 UI로 읽힌다.
- [ ] 피격과 피해가 결과로 남는다.
- [ ] 주행과 전투를 반복해도 상태가 꼬이지 않는다.
- [ ] 남은 문제를 기능 차단 / 품질 후속 / 장기 확장으로 분리할 수 있다.

### 완료 후 처리
- 완료된 기능은 `Document/Systems/Combat/` 또는 관련 Systems 문서로 승격한다.
- 진행 중 상세 계획은 `Document/Plan/CombatPlan/` 또는 별도 기능 Plan으로 정리한다.
- 완료 판정 결과는 `05_TestChecklist.md`와 `04_ProjectDecisions.md`에 반영한다.

---

## 기존 기준선 기록 (2026-06-02 이전)
아래 D0/P0/P1/P2/P3 기록은 기존 차량 코어 기준선과 판단 근거를 보존하기 위한 참고 기록이다.
2026-06-19 이후의 실제 진행 순서는 위 S0, S0-1, S1, C1~C6를 우선한다.

---

## D0 — 문서 방향 정렬
### 목표
최종 방향, 현재 기준선, 구조 갭, 작업 순서를 문서상에서 서로 충돌하지 않게 맞춘다.

### 해야 할 일
- `00_Vision.md`에 최종 방향 / 현재 기준선 / 구조 갭 / 유지할 것 / 교체할 것을 고정한다.
- `01_ProjectState.md`에 현재 실제 상태와 임시 운영 편차만 남긴다.
- `05_TestChecklist.md`에 현재 검증 기준을 통합한다.
- `VehicleCoreDecisions.md`에 유지 구조와 교체 구조 결정을 적는다.
- 중복 문서는 루트에서 내리고 `Archive/`로 이동한다.

### 완료 조건
- [ ] 최종 방향 문장이 문서마다 충돌하지 않는다.
- [ ] 현재 기준선 문장이 문서마다 충돌하지 않는다.
- [ ] `GamepadOnly` 같은 운영 편차와 구조 갭이 분리돼 있다.
- [ ] 루트 활성 문서가 최소 집합으로 줄어 있다.

---

## P0 — 현재 기준선 실측
### 목표
현재 프로젝트가 어떤 자산 / 값 / 맵 / 루틴 위에서 굴러가는지 실제 기준으로 닫는다.

### 해야 할 일
- `BP_CFVehiclePawn` 실제 구성 확인
- `DA_PoliceCar` 실제 값 확인
- `TestMap` 현재 검증 루프 확인
- `WheelSync` 현재 운용 방식 확인
- `DriveState` 디버그 표시 확인

### 완료 조건
- [ ] `BP_CFVehiclePawn` 실제 구성표가 있다.
- [ ] `DA_PoliceCar` 실제 설정표가 있다.
- [ ] `TestMap` 검증 루프가 적혀 있다.
- [ ] WheelSync 현재 설정과 미사용 대상을 분리했다.
- [ ] 실측 결과가 `01_ProjectState.md`와 `05_TestChecklist.md`에 반영돼 있다.

### 현재 상태 메모
- 위 항목 중 문서 / 자산 / 코드 실측으로 확인 가능한 부분은 반영 완료
- 남은 항목은 PIE 런타임 PASS / FAIL 검증이다.
- 현재는 에디터 복구가 확인되었으므로 이 검증을 바로 재개한다.
- 현재 런타임 진행 결과:
  - `P0-001`: PASS
  - `P0-002`: PASS
  - `P0-003`: PASS
  - 다음 작업 대상은 `P0-002` 후속 정리, 특히 디버그 가독성 개선과 Anchor `Z` 미세 보정이다.

### P0-002 역사 기록 — 수정 전 확정 절차
아래 항목은 `P0-002`를 처음 분해하던 시점의 기록이다.
현재 운영 기준과 우선순위는 문서 상단 `현재 진행 상태`를 우선해서 본다.

직접 수정에 들어가기 전에 아래를 먼저 닫는다.

1. `확정 사실`
   - `ApplyVehicleLayoutConfig()`는 현재 `Wheel_Mesh_*`만 재배치한다.
   - `WheelSync`의 조향 회전은 `Wheel_Anchor_*` 기준으로 적용된다.
   - 당시 기본 설정에서는 스핀 적용 경로가 꺼져 있었다.
2. `강한 가설`
   - 초기 배치 FAIL과 조향 피벗 FAIL은 `Anchor`와 `Mesh`의 기준 분리에서 같이 발생했을 가능성이 높다.
3. `수정 전 결정`
   - 최종 배치 기준을 `Wheel_Anchor_*` 중심 구조로 둘지 먼저 확정한다.
   - `Wheel_Mesh_*`는 Anchor 자식 기준 시각 표현 전용으로 둘지 먼저 확정한다.
   - 스핀 값을 임시 DebugPipe로 볼지, 실제 휠 회전값 경로로 볼지 먼저 정한다.
4. `수정 순서`
   - 배치 기준 정리
   - 조향 피벗 정상화
   - 스핀 시각 경로 복구
   - 디버그 UX 정리

---

## P1 — 공통 코어 검증
### 목표
현재 코어가 특정 테스트 차량 전용 임시 구조가 아니라, 다음 차종에도 옮길 수 있는 공통 코어인지 검증한다.

### 해야 할 일
- `05_TestChecklist.md`의 DriveState / WheelSync 기준으로 검증한다.
- 실패 항목을 코어 문제와 튜닝 문제로 분리한다.
- 공통 규칙이 아닌 현재 구조 종속 처리 후보를 기록한다.

### 완료 조건
- [ ] DriveState 기본 흐름 PASS / FAIL이 정리되어 있다.
- [ ] WheelSync 공통 규칙 관점의 문제점이 정리되어 있다.
- [ ] 코어 문제와 차량별 수치 문제가 분리되어 있다.

---

## P2 — 첫 차량 기준값 정리
### 목표
`DA_PoliceCar`를 현재 기준선 차량으로서 안정화하되, 최종 구조 대체물처럼 굳지 않게 정리한다.

### 해야 할 일
- `05_TestChecklist.md`에 정리한 시작값 기준으로 DriveState를 조정한다.
- 현재 차량 전용 조정과 공통 코어 문제를 구분한다.
- 이후 다른 차량에도 옮길 수 있는 항목과 아닌 항목을 구분한다.

### 완료 조건
- [ ] 현재 시작값 표가 정리되어 있다.
- [ ] 차량 전용 조정 항목이 분리되어 있다.
- [ ] 공통 코어를 덮는 식의 튜닝이 금지되어 있다.

---

## P3 — 구조 전환 설계 입력값 고정
### 목표
실제 구조 전환 작업 전에, 무엇을 유지하고 무엇을 교체할지 문서와 결정 로그로 먼저 고정한다.

### 해야 할 일
- `VehicleCoreDecisions.md`에 유지 코어와 교체 구조를 확정한다.
- 현재 하이브리드 구조에서 전환 시 영향을 받을 지점을 정리한다.
- 최종 구조 전환을 별도 UE 작업으로 시작할 준비를 마친다.

### 완료 조건
- [ ] 유지할 코어 목록이 고정되어 있다.
- [ ] 교체할 구조 목록이 고정되어 있다.
- [ ] 다음 UE 작업이 "무엇을 바꾸는 작업인지" 문서상에서 명확하다.

---

## 후속 착수 메모 — Camera Baseline (2026-04-15 추가)
이 섹션은 기존 로드맵을 뒤집는 새 상위 목표가 아니라,
현재 기준선 코어 정리 이후 실제로 착수한 **카메라 기준선 작업**이 어디에 붙는지 설명하는 보강 메모다.

### 현재 판정
- 현재 카메라 작업은 `차량 전투 규칙 구현`이나 `최종 카메라/HUD 확장` 단계로 보지 않는다.
- 현재 작업의 성격은 **차량 기준선 위에 기본 카메라 시스템 뼈대와 입력 경로를 먼저 고정하는 보조 기준선 확보 작업**이다.
- 따라서 이 작업은 로드맵 상위 우선순위를 바꾸기보다, 이후 HUD / 무기 연동 / 카메라 확장을 안전하게 붙이기 위한 준비 단계로 본다.

### 현재 완료 기준
- `UCFVehicleCameraComp` 기반 카메라 코어 추가
- `ACFVehiclePawn`의 Look 입력 연동 추가
- `BP_CFVehiclePawn` 카메라 계층 기준선 정리
- `IA_LookAround` 2D 입력 기준선 확인
- Yaw / Pitch 기본 회전 확인
- 빌드 성공 확인

### 현재 남은 작업
- 카메라 감각 튜닝
- 충돌 감각 튜닝
- HUD / 조준점 최소 버전
- 무기 시스템 기반 Aim Profile 실제 연동
- Reverse / Airborne / Destroyed 확장 모드 정리

### 해석 규칙
- 현재 카메라 작업은 어디까지나 **기본 카메라 기준선 확보**다.
- `차량 전투 규칙 구현`, `HUD 확장`, `무기별 카메라 모드 완성`은 아직 상위 로드맵 기준에서 후속 단계로 남는다.
- 세부 구현 기준은 `Document/Plan/CameraPlan/` 문서군에서 본다.

---

## 변경 이력

### v2.28.20 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-02 Browser·Mesh-only·Create·Existing Import`을 사용자 직접 확인으로 USER PASS 처리했다.
- Paused 상태는 유지하고 USER checkpoint만 `PASS 2 / 다음 UA-03 Recipe·Shared Profile·Affected Vehicle Impact`로 전진시켰다.
- 대표 Plan/Roadmap projection은 `DataAuthoringPlan.md v0.2.24 / DataAuthoringRoadmap.md v0.1.32`다. DG/DEL/Wizard deletion은 계속 미개방이다.

### v2.28.19 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-12 UA-01 Workspace First Impression`을 사용자 직접 확인으로 USER PASS 처리했다.
- CF-FQ-038 Paused는 유지하고 USER checkpoint만 `PASS 1 / 다음 UA-02 Browser·Mesh-only·Create·Existing Import`로 전진시켰다.
- 대표 Plan/Roadmap projection은 `v0.2.20 / v0.1.28`이다. DG/DEL/Wizard deletion은 계속 미개방이다.

### v2.28.18 - 2026-08-18

- 사용자 우선순위 변경에 따라 `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`를 현재 단일 Active로 복원했다.
- 즉시 재개 지점은 `InGameUIPlan.md v0.58.6 / UI-P0-03 Defense Production Panel USER Visual → Pawn Rebind USER Visual`이며 기존 UI-P0-02/03 기술·USER PASS는 관련 결함이 없는 한 반복하지 않는다.
- `CF-FQ-038`은 P0-08A~M/P0-09~11 Technical PASS와 `DAUTH-P0-12 USER PASS 0 / UA-01`을 보존한 Paused로 전환했다. DG/DEL Gate는 미개방 상태를 유지한다.
- `CF-FQ-037 Scanner`의 `SensorContact.md v1.1.0`은 TargetPanel/Radar 후속 UI가 소비할 Current Sensor source이며 Radar Range/Zoom·동적 Blip·USER Visual은 계속 Pending이다.

Migration: 다음 주력 작업은 CF-FQ-032 UI다. DAUTH는 `DataAuthoringPlan.md v0.2.17 / DataAuthoringRoadmap.md v0.1.25 / P0-12 UA-01`에서 보존하고 사용자 재지정 전 자동 재개하지 않는다.

### v2.28.17 - 2026-08-18


- `CF-FQ-038 / DAUTH-P0-12 USER Authoring Acceptance`를 In Progress로 착수했다.
- `DataAuthoringPlan.md v0.2.16` / `DataAuthoringRoadmap.md v0.1.24`의 UA-01~08을 USER 검증 순서로 사용하며 current checkpoint는 UA-01 Workspace First Impression이다.
- P0-08A~M/P0-09~11 Technical PASS를 반복하지 않고 현재 USER PASS는 0으로 유지한다.
- DG/DEL Gate와 Wizard 삭제는 P0-12 USER 결과 전까지 열지 않는다.

### v2.28.16 - 2026-08-18

- `CF-FQ-037 Scanner`를 `SCAN-P0-06 USER PIE PASS` + `SCAN-P0-07 Current System Integration Done`으로 닫아 Paused 재개 후보에서 제거했다.
- final production Build `fcf52353d1f5440392d5e1c379f09ee3` PASS와 `Systems/Targeting/SensorContact.md v1.1.0` Current 승격을 closure 기준으로 연결했다.
- Scanner 완료는 CF-FQ-032 Radar Range/Zoom·동적 Blip·USER Visual이나 Sensor energy/heat·AI/Network를 완료로 확대하지 않는다.

### v2.28.15 - 2026-08-18


- `CF-FQ-038 / DAUTH-P0-11 Frozen UX Completeness Closure`를 완료해 P0-11 overall을 Technical PASS로 올렸다.
- Handling/Performance Adoption, Shared Profile B2 edit+impact, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh를 existing Core 재사용으로 구현했다.
- Drift prospective duplicate의 RecipeId 재발급으로 approval hash가 비결정적이던 defect를 원본 RecipeId 복원으로 교정하고 approval binding은 유지했다.
- final Build `98dfceab797942218bd09c844e398ceb` PASS, focused P0-11 `b53998e6cacf48a1a554d784b77e013c` 6/6 PASS, full Data Authoring `77b45525549b4866995b3d972fb4a9fa` 63/63 PASS를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting production source, Content Asset, Wizard 삭제, DG/DEL은 변경/개방하지 않았다. P0-12는 dependency상 Ready지만 이번 작업에서 시작하지 않았다.

### v2.28.14 - 2026-08-18

- `DAUTH-P0-11` representative E2E와 applied VehicleData Inventory/Fitting consumer regression을 추가해 Core Technical Validation PASS를 확보했다.
- final Build `ae0e92c60a094be09b036ae12b106717` PASS, focused P0-11 2/2, Inventory 1/1, Fitting 2/2, full Data Authoring 59/59 PASS를 current evidence로 연결했다.
- Frozen 24.90~24.94 전체 workflow audit에서 Handling/Performance Adoption UI, normal Shared Profile edit/impact, External Drift 3-way recovery, Mesh-only Candidate/Create Vehicle From Mesh가 미구현임을 확인했다.
- P0-08/P0-09/P0-10 Technical PASS는 보존하되 P0-11 overall은 In Progress, P0-12는 Not Ready다. Wizard 삭제와 DG/DEL Gate도 열지 않는다.
- 다음 착수는 `DataAuthoringRoadmap.md v0.1.22 / DAUTH-P0-11 Frozen UX Completeness Closure`다.

Migration: 기존 Section 14/24.98 PASS 증거를 반복하지 않고 Frozen 24.90~24.94 production gap closure를 우선한다.

### v2.28.13 - 2026-08-18

- `CF-FQ-038 / DAUTH-P0-10 Existing Wizard Migration`을 Technical PASS로 Roadmap에 반영했다.
- Assets/Layout semantic authoring, Wheel Measurement/Adoption, Frozen 4축 Driving Feel/preset, read-only Reference Compare, Stable-ID Mount/Defaults와 standard Undo를 새 Workspace에 parity했다.
- legacy Wizard는 managed Target의 변경 동작만 비활성화하고 read-only 기능과 unmanaged path를 유지한다. Wizard deletion/DG/DEL은 열지 않았다.
- final Build `03fa78af4e124a3db33893ca8bfe436d` PASS, focused P0-10 4/4, full Data Authoring 57/57 PASS, JSON SHA `90c01eda8b1df96f03d55feb1e3000f99db16f8a717baa697f5c15cdb9f6884b`를 current evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, Content Asset, Batch main page는 변경하지 않았고 USER UX/Driving Feel PASS는 P0-12에 남겼다.
- 다음 Gate는 `DAUTH-P0-11 Technical Validation`이다.

Migration: `DataAuthoringPlan.md v0.2.13 / DataAuthoringRoadmap.md v0.1.21`의 P0-11에서 재개하고 P0-08A~M/P0-09/P0-10을 반복하지 않는다.

### v2.28.12 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-09 Vehicle Authoring MVP`를 Technical PASS로 Roadmap에 반영했다.
- `CarFight.VehicleAuthoring` Nomad Workspace에 Browser/Selection, Initial Import, Recipe basic intent, Resolver Preview, Pending Diff, Source Trace, Validation, shared Apply/standard Undo, Raw DA Open을 구현했다.
- UI/ViewModel은 `FCFVehicleAuthoringService` facade만 사용하며 Target writer는 계속 `FCFVehicleApplyService` 하나다. Runtime/Inventory/Fitting/Content Asset은 변경하지 않았다.
- legacy `CarFight.VehicleDAWizard` / `SCFVDAWizardTab`을 그대로 유지하고 `Workspace.TabRegistration`으로 두 spawner 공존 및 신규 Workspace actual spawn을 검증했다.
- final Build `76fc476c8ccb4daf895a3b567fe0c992` PASS, focused P0-09 6/6, full Data Authoring 53/53 PASS, JSON SHA `697230dbce1c9a6281a0e56c4aa292544d3becd28b701a25285a487a50592654`를 current evidence로 연결했다.
- P0-09 Technical PASS를 USER Visual/Usability PASS로 확대하지 않으며, 다음 Gate는 `DAUTH-P0-10 Existing Wizard Migration`이다.

Migration: `DataAuthoringPlan.md v0.2.12 / DataAuthoringRoadmap.md v0.1.20`의 P0-10에서 재개하고 P0-08A~M/P0-09를 반복하지 않는다. Wizard deletion은 DG/DEL Gate 전까지 금지한다.

### v2.28.11 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08M`을 Technical PASS로 Roadmap에 반영하고 `DAUTH-P0-08 Implementation Foundation` 전체를 Technical PASS로 닫았다.
- per-target fresh R3 evidence, exact B3 approval, all-target global preflight, canonical TargetPath 순서의 `FCFVehicleApplyService` exact-once 호출과 Stop-On-First-Failure partial result를 구현했다.
- NoChange/ShadowOnly/ExternalDrift/Resolve Error·Blocked/Validation Blocked는 ineligible이다. Batch global atomic/rollback, already-applied rollback, retry/save는 0이다.
- final Build `10027d18360e48f399a5b439f280c24c` PASS, focused M `a49d21b6b9b94d648142d5a555a79c95` 4/4 PASS, targeted `cb2ae69e9c834657a553fe52c00f5a96` 47/47 PASS, result SHA `472ef5bb8f5fdeb46ebd1b7e68f1fc5a66acba065828ff025167b475aaf25595`를 current evidence로 연결했다.
- Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader 보호 범위를 유지했고 Batch UI/file save는 0이다.
- Frozen 26.66~26.69 idempotency/generic envelope는 M의 dependency가 아니므로 첫 실제 external Batch transport/client integration 전 follow-up으로 보존한다.
- 다음 Gate를 `DAUTH-P0-09 Vehicle Authoring MVP`로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.11 / DataAuthoringRoadmap.md v0.1.19`의 `DAUTH-P0-09`에서 재개한다. P0-08A~M과 P0-08 Foundation을 반복하지 않는다.

### v2.28.10 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08L`을 Technical PASS로 Roadmap에 반영했다.
- exact Batch approval, all-source global TOCTOU preflight, B1 Recipe/B2 shared Profile atomic source transaction, rollback과 success 후 old approval invalidation을 구현했다.
- External Drift/ShadowOnly를 source commit blocker로 승격하지 않고 B1 Recipe validation과 B2 affected Recipe inventory/validation을 Frozen 경계대로 fresh 재검사한다.
- official Build `2e2923f31d5a4f3fa703f3b7623c0741` PASS와 targeted process `3853d42dfb344bfd8eb6e516903a8d78` 43/43 PASS를 current evidence로 연결했다.
- B1/B2는 source package Dirty만 허용하고 Target mutation/Auto Save/B3/UI/file save는 0이다. Runtime/Inventory/Fitting/Wizard/Content/Validator/Asset Reader 보호 범위를 유지했다.
- `DAUTH-P0-08`은 In Progress이며 다음 checkpoint는 Frozen Section 26.54~26.65의 `DAUTH-P0-08M B3 Batch Definition Apply Foundation`이다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.10 / DataAuthoringRoadmap.md v0.1.18`에서 P0-08M부터 재개한다. P0-08M은 Frozen B3 구간에 새로 부여한 implementation checkpoint label이다.

### v2.28.9 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08K`를 Technical PASS로 Roadmap에 반영했다.
- current Unreal reread 기반 canonical Batch Import 3-way preview, duplicate/read-only guard, ownership conflict, transient Recipe/Profile prospective Resolver preview와 deterministic BatchPlanHash를 구현했다.
- official Build `a3d37d4c8520442d8ceb09a72eb6a68f` PASS와 targeted process `7d2db5a9baa54dd9abf25857138d3994` 37/37 PASS를 current evidence로 연결했다.
- persistent Batch source/Target mutation은 0이고 Runtime·Inventory·Fitting·Legacy Wizard·Content·Validator·Asset Reader 보호 범위를 유지했다. B1/B2 source commit, B3 Apply, UI/file save는 열지 않았다.
- `DAUTH-P0-08`은 In Progress이며 다음 checkpoint는 Frozen Section 26.44~26.53의 `DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation`이다. Section 26.54+ B3는 별도 후속이다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.9 / DataAuthoringRoadmap.md v0.1.17`에서 P0-08L부터 재개한다.

### v2.28.8 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08J`를 Technical PASS로 Roadmap에 반영했다.
- projection-only Batch Registry, Recipe/Profile numeric allowlist, 117 resolved projection과 reserved stable ColumnId metadata를 구현했다.
- canonical BOM-less UTF-8 CSV와 immutable-style `.cfbatch.json` export baseline/ExportSetHash를 구현하고 order determinism/manifest consistency/persistent mutation0을 검증했다.
- official Build `3249098c1b99487a8fd173694573bd5e` PASS와 targeted process `534486583a1d4689bc5137505a03f806` 32/32 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content·Validator·Asset Reader 보호 범위를 유지하고 Import Session/B1·B2/B3/UI/file save는 0으로 유지했다.
- `DAUTH-P0-08`은 In Progress이며 다음 checkpoint를 `DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.8 / DataAuthoringRoadmap.md v0.1.16`에서 P0-08K부터 재개한다.

### v2.28.7 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`을 Technical PASS로 Roadmap에 반영했다.
- `FCFVehicleAuthoringService`는 existing Snapshot/AssetReader/Pure Resolver/ApplyService authority를 재사용하는 single-Vehicle facade이며 R0 read, prospective R1 preview/commit과 R3 shared Apply entry를 제공한다.
- exact approval scope, expected Recipe/Target/revision, ExpectedDiffHash, typed result/error/mutation footprint와 bounded ClientOperationId dedupe를 구현했고 Raw SetField/direct Target write/force/skip-validation/auto-retry/auto-save는 0이다.
- Frozen Section 22.27 RecipeFingerprint에서 `VehicleArchetypeId`를 추가하지 않고 typed desired-state equality로 non-resolver semantic operation을 처리해 기존 Core 계약을 보존했다.
- official Build `252fdbd097f943379f2a1e2a942bedef` PASS와 targeted process `f2011a206c964fadb269d13c4dba3c86` 27/27 PASS, result JSON SHA-256 `b9bfea5be21f1bcc521e0bcfa8b8a43c017e18a4dec65a208bfd57a68805bfbf`를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset·Asset Reader·existing Validator 보호 범위를 유지하고 UI/CSV/Batch는 0으로 유지했다.
- `DAUTH-P0-08`은 In Progress를 유지하며 다음 checkpoint를 `DAUTH-P0-08J Batch Column Registry / Canonical Export Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.7 / DataAuthoringRoadmap.md v0.1.15`에서 P0-08J부터 재개한다.

### v2.28.6 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08H Apply Transaction Foundation`을 Technical PASS로 Roadmap에 반영했다.
- central `FCFVehicleApplyService`가 TOCTOU precondition, fresh Resolver/Diff, transient preflight, dependency-safe stable-array/leaf mutation, Target readback/Validator와 AppliedState를 소유한다.
- successful Apply는 Target+Recipe transaction 및 Package Dirty까지만 수행하고 automatic Save는 0이다. A8~A10 실패는 자체 rollback + transaction cancel + full Target hash 재검증으로 partial success를 차단한다.
- official Build `b88831b71797415fac9dc0a23d12c39e` PASS와 targeted process `6f261c0b76f04d0faa6b9e855865818e` 23/23 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset·Asset Reader·existing Validator 보호 범위를 유지하고 UI/CSV/Batch는 0으로 유지했다.
- `DAUTH-P0-08`은 In Progress를 유지하며 다음 checkpoint를 `DAUTH-P0-08I Common Authoring Service / AI Typed Contract Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.6 / DataAuthoringRoadmap.md v0.1.14`에서 P0-08I부터 재개한다.

### v2.28.5 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08G Existing Definition Import / Adoption Foundation`을 Technical PASS로 Roadmap에 반영했다.
- Current Definition exact field 전체를 normal Legacy Pin과 Mount hidden LegacySerialized passthrough로 lossless partition하고 direct semantic candidate copy를 Recipe migration state에 연결했다.
- Movement raw 값에서 Driving Feel/Profile을 authoritative하게 역산하거나 자동 binding하지 않으며 Initial Import source ownership은 Legacy Pin이 유지한다.
- group/field Adoption Preview는 persistent mutation 없이 selected Pin만 virtual release하고, approved Commit은 fresh Recipe fingerprint precondition 아래 Recipe-only transaction으로 수행한다.
- official Build `b201d87cd13f4987a0907e08c8f00a6c` PASS와 targeted process `3b52a251ab244096b78bb872a06c0069` 20/20 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset·Asset Reader·existing Validator 보호 범위를 유지하고 Target Definition mutation / Apply / UI / CSV는 0으로 유지했다.
- `DAUTH-P0-08`은 In Progress를 유지하며 다음 checkpoint를 `DAUTH-P0-08H Apply Transaction Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.5 / DataAuthoringRoadmap.md v0.1.13`에서 P0-08H부터 재개한다.

### v2.28.4 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08F Definition Materializer / Validation Foundation`을 Technical PASS로 Roadmap에 반영했다.
- Resolver R15에 transient `UCFVehicleData` materialization, Stable-ID Hardpoint/Mount reconstruction, FieldCodec checked import, existing `UCFVDAValidator`와 readback hash consistency를 연결했다.
- Definition Error/Blocked는 Apply eligibility Blocked로, materialization/codec/hash 자체 오류는 Resolver Error로 분리한다.
- official Build `e5d925c6531e4ae6ac58aa356e4078eb` PASS와 targeted process `4b21d25b780c4a398de7a1b47c595c5e` 17/17 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset·Asset Reader·기존 Runtime Validator 보호 범위를 유지하고 Apply / UI / CSV는 후속으로 남겼다.
- `DAUTH-P0-08`은 In Progress를 유지하며 다음 checkpoint를 `DAUTH-P0-08G Existing Definition Import / Adoption Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.4 / DataAuthoringRoadmap.md v0.1.12`에서 P0-08G부터 재개한다.

### v2.28.3 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08E Pure Resolver Foundation`을 Technical PASS로 Roadmap에 반영했다.
- Snapshot-only Request/Result, Frozen R0~R16, stage-independent deterministic precedence, Measurement Proposal/Adoption, Source Trace/Hash, R14 Diff, R16 Stale/Drift foundation을 구현했다.
- R15 Definition Materializer + `UCFVDAValidator` integration은 다음 `DAUTH-P0-08F`로 분리했으며 `DAUTH-P0-08`은 In Progress를 유지한다.
- official Build `db202f0797af41fe86a859609cb4dfd9` PASS와 targeted process `93d445f78cbe4eaabca1478eb6d27078` 15/15 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset·Asset Reader 보호 범위를 유지하고 Apply / UI / CSV를 후속으로 남겼다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.3 / DataAuthoringRoadmap.md v0.1.11`에서 P0-08F부터 재개한다.

### v2.28.2 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08D Asset Snapshot Reader Foundation`을 Technical PASS로 Roadmap에 반영했다.
- Chassis requested socket facts, 4개 Wheel StaticMesh local bounds와 resolver-relevant fingerprints를 immutable Asset Snapshot으로 구현했다.
- official Build `780d30c4a44f48feb179f7f6da5480e1` PASS와 targeted process `ab2cd64308484ce0ae0e24cb1143ca33` 9/9 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset 보호 범위를 유지하고 Resolver / Apply / UI / CSV를 후속으로 남겼다.
- 다음 구현 Gate를 `DAUTH-P0-08E Pure Resolver Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.2 / DataAuthoringRoadmap.md v0.1.10`에서 P0-08E부터 재개한다.

### v2.28.1 - 2026-08-17

- `CF-FQ-038 / DAUTH-P0-08C Immutable Snapshot Foundation`을 Technical PASS로 Roadmap에 반영했다.
- Recipe / 5 Profile / Definition / Project Compatibility Default Snapshot과 Registry-expanded Stable-ID field entries, deterministic fingerprint/hash, `RequiredDependencies`를 구현했다.
- final official Build `6c81b2149de44d00a99554a44d2135aa` PASS와 targeted process `340050d2c911445da3634ebb92acc014` 8/8 PASS를 current evidence로 연결했다.
- Runtime·Inventory·Fitting·Legacy Wizard·Content Asset 보호 범위를 유지하고 Asset Snapshot Reader / Resolver / Apply / UI / CSV를 후속으로 남겼다.
- 다음 구현 Gate를 `DAUTH-P0-08D Asset Snapshot Reader Foundation`으로 이동했다.

Migration: CF-FQ-038은 `DataAuthoringPlan.md v0.2.1 / DataAuthoringRoadmap.md v0.1.9`에서 P0-08D부터 재개한다.

### v2.28.0 - 2026-08-17

- `CF-FQ-038 차량 데이터 Authoring 시스템`을 현재 단일 Active로 정식 반영했다.
- `DAUTH-P0-08A/B`에서 Editor-only Recipe + 5 Profile, Never-Cook, Stable Field Path / Field Value Codec와 `UCFVehicleData` 117 Registry 양방향 coverage를 구현했다.
- official Build `5c745a31d19b448d9b1049877bc877ca` PASS와 targeted Automation `ab9de3e8c8cd410a8de0641178690dff` 4/4 PASS를 현재 Gate evidence로 연결했다.
- Runtime `UCFVehicleData`, Inventory/Fitting, `SCFVDAWizardTab`, Content Asset은 변경하지 않았다.
- `CF-FQ-034`는 `FIT-P0-07D USER Driving Feel Comparison`을 보존한 Paused로 전환했다.
- 다음 구현 Gate를 `DAUTH-P0-08C Immutable Snapshot Foundation`으로 고정했다.

### v2.27.7 - 2026-08-17

- CF-FQ-034 `FIT-P0-07C Quantitative Mobility Measurement`을 Technical Complete로 Roadmap에 반영했다.
- Light/Default/Heavy를 fixture별 fresh PIE에서 같은 프로토콜로 계측하고 가속·제동거리·coast-steering baseline을 확보했다.
- final Build `bb04d56e0cd24647b2ef6fcbc7f2bd77` PASS와 Quantitative process `63bbeaf0202041b79dac8f32f5447923` Success / Metric 3/3을 보호 증거로 연결했다.
- 가속과 coast-steering의 비단조 결과를 자동 FAIL이나 Scalar 튜닝으로 바꾸지 않고 PhysicalMassOnly를 유지했다.
- 다음 순서를 `FIT-P0-07D USER Driving Feel Comparison`으로 이동했으며 USER Mobility·Field UI·PIE는 Pending이다.

Migration: CF-FQ-034 다음 작업은 `VehicleFittingPlan.md v0.17.0 / FIT-P0-07D`다. FIT-P0-07A~07C와 기존 Fitting 23/23을 반복하지 않고 같은 공식 Fixture의 사용자 직접 체감만 확인한다.

### v2.27.6 - 2026-08-17

- CF-FQ-034 `FIT-P0-07B Heavy Payload Resolution + Official Fixture Preparation`을 Technical Complete로 반영했다.
- 같은 SUV 플랫폼의 Light 1000kg / Default 1570kg / Heavy 1600kg 공식 Fixture와 persisted DisplayName·참조를 현재 Roadmap 기준으로 고정했다.
- post-label `CarFight.Fitting.FIT_P0_07.OfficialMobilityFixtures` 1/1 PASS를 보호 증거로 연결했다.
- 다음 원격 기술 순서를 `FIT-P0-07C Quantitative Mobility Measurement`로 이동했고 USER 주행감·Field UI·PIE는 Pending을 유지했다.

Migration: CF-FQ-034 다음 작업은 `VehicleFittingPlan.md v0.16.0 / FIT-P0-07C`다. FIT-P0-07A~07B와 기존 Build/Fitting 23/23을 관련 Source/Asset 변경 없이 반복하지 않는다.

### v2.27.5 - 2026-08-17

- 사용자 선택에 따라 CF-FQ-034 차량 피팅·질량 런타임을 현재 단일 Active로 전환했다.
- FIT-P0-07A Fixture Readiness Audit에서 메시-only 차량을 실제 VehicleData/Fitting platform과 분리했다.
- DA_TestSedan·DA_TestSUV는 Base/Gross 0kg라 공식 Fixture에서 제외하고 `DA_VehicleDefense_TestSUV` 1000/2500kg를 동일 플랫폼 기준으로 고정했다.
- Light 1000kg과 기존 Default 1570kg은 임의 질량 없이 준비 가능하며 Heavy same-platform persisted payload 확인을 FIT-P0-07B로 이동했다.
- Source·Content Asset·Build·USER PIE는 변경하거나 실행하지 않았다.

Migration: 다음 CF-FQ-034 작업은 `VehicleFittingPlan.md v0.15.0 / FIT-P0-07B Heavy Payload Resolution`이다. 기존 기술 PASS와 FIT-P0-07A AssetDump 분류는 반복하지 않는다.

### v2.27.4 - 2026-08-17

- CF-FQ-029를 stale Active projection에서 Paused / LM-P0-06 USER PIE Pending으로 교정하고 현재 단일 Active를 비웠다.
- CF-FQ-030 Persisted Missile Test Asset Technical Verification PASS와 `MissileGuidancePlan.md v0.5.0`을 현재 우선순위에 반영했다.
- fresh DirectRuntimeContract `29d0ef16dd5e4d56945ceae26eec6937` 1/1 PASS, Missile folder AssetDump 20/20, Maps World 12/12와 4개 계약 DataAsset 저장 참조 체인을 기록했다.
- MissileDirectTest World는 33 Actor와 Missile test vehicle/RocketLauncher socket을 확인했지만 전체 Actor label은 public readback 범위 밖이므로 다섯 MissileTarget label 자체를 독립 PASS로 확대하지 않았다.
- Runtime Source·Content Asset·Blueprint·Map 저장 변경과 USER PIE는 0이며 CF-FQ-030은 Ready / Manual PIE Pending을 유지한다.

Migration: 다음 Missile 작업은 `MissileGuidancePlan.md v0.5.0 / CF-TC-027 Manual PIE`에서 시작한다. CF-FQ-029와 CF-FQ-037의 USER PIE 체크포인트도 기존 위치를 그대로 보존한다.

### v2.27.3 - 2026-08-16

- 현재 단일 Active를 CF-FQ-029 Launcher로 전환하고 `LM-P0-06A Failure Policy Technical Closure`를 원격 기술 next gate로 등록했다.
- ContinueRemaining/StopSequence는 asset-free Automation으로 검증하고 Manual Cancel/Ammo cleanup 기존 회귀를 보호한다.
- CF-FQ-037 Scanner는 P0-06A readiness가 완료된 `SCAN-P0-06 USER PIE Pending` 상태로 Paused 보존한다.
- 양쪽 USER PIE 결과를 기술 Automation으로 추정하지 않는다.

Migration: 새 세션은 Launcher Failure Policy 기술 Closure를 먼저 수행한다. 화요일 Scanner USER PIE 가능 시 Scanner 체크포인트를 별도 재개한다.

### v2.27.2 - 2026-08-16

- CF-FQ-037 대표 Plan projection을 `ScannerIntegrationPlan.md v0.7.1`로 동기화했다.
- P0-06A Scanner 전용 PIE fixture readiness 완료를 현재 Roadmap에 반영했다.
- 실제 남은 Gate는 `V` 단발 입력, 5초 timed scan 자동 종료, 반복 입력 무연장, scanner-less safe reject와 TargetSelect/HUD 기본 비회귀 USER 확인이다.
- 검증용 Sensor fixture 수치를 게임 밸런스 확정값으로 승격하지 않고 P0-07 자동 이동을 계속 금지한다.

Migration: P0-06A 준비 작업은 반복하지 않는다. 사용자 직접 PIE가 가능해지면 `ScannerIntegrationPlan.md v0.7.1`의 P0-06 체크리스트만 수행한다.

### v2.27.1 - 2026-08-16

- CF-FQ-037 현재 우선순위 projection을 `ScannerIntegrationPlan.md v0.7.0 / SCAN-P0-06 USER PIE Acceptance`로 교정했다.
- P0-00~05 Technical Done과 P0-04 final Build·회귀, P0-05 fresh TargetSelect 보호 회귀를 현재 기술 기준선으로 반영했다.
- Foundation Audit 재시작 문구를 제거하고, Source 변경이 없는 동안 기존 기술 evidence를 반복하지 않으며 사용자 직접 PIE만 남도록 정리했다.

Migration: CF-FQ-037의 현재 순서는 `P0-06 USER PIE → PASS 시 P0-07 Current System Integration`이며 P0-00~05는 재실행 대상이 아니다.

### v2.27.0 - 2026-08-16

- 사용자 선택에 따라 `CF-FQ-037 차량 스캐너 입력·장비 통합`을 현재 단일 Active로 등록했다.
- 대표 Plan `ScannerIntegrationPlan.md v0.1.0`과 `SCAN-P0-00~07` 진행 경로를 현재 우선순위에 연결했다.
- 첫 Gate는 Source/Asset mutation 없는 Foundation Audit이며 기존 Input owner, UCFVehicleSensorData/FCFSensorConfig, VehicleData와 Fitting/Inventory 재사용 가능성을 먼저 판정한다.
- CF-FQ-036 SensorContact는 Done / Current System으로 유지하며 old SEN-P0 Gate를 재개하지 않는다.
- Radar/TargetPanel, Sensor energy/heat, Missile/Utility 소비, AI/Network는 CF-FQ-037 범위에서 제외했다.
- 기존 USER Pending과 dirty work를 보호하며 이번 인계 준비에서는 Source·Asset·Build·Automation·PIE를 실행하지 않았다.

### v2.26.0 - 2026-08-15

- `CF-FQ-036 SEN-P0-07 Technical Acceptance`를 PASS로 종료하고 `Document/Systems/Targeting/SensorContact.md v1.0.0` Current System 승격을 반영했다.
- 기존 Build `7dff9da7aaa24e76b0871348762c9d93` PASS, Sensor 14/14와 UI asset-free 1/1+1/1 PASS를 Acceptance evidence로 재사용했으며 Source 의미 변경이 없어 Build/Automation을 반복하지 않았다.
- 현재 단일 Active를 비우고 다음 작업은 FeatureQueue에서 사용자 선택으로 정하도록 변경했다. Paused/Ready 기능은 자동 승격하지 않는다.
- `SensorContactPlan.md`는 Historical + Retained Path 완료 기록으로 내리고 현재 구현 판단은 SensorContact Current System을 우선한다.
- CF-FQ-032 Radar/TargetPanel USER Visual, Radar Range/Zoom·동적 Blip, CF-FQ-026 TS-P0-08 USER PIE와 다른 USER Pending은 그대로 보존했다.

### v2.24.0 - 2026-08-15

- 사용자 결정에 따라 `CF-FQ-036 차량 센서·Contact Intelligence Runtime`을 현재 단일 Active로 등록했다.
- 대표 Plan `SensorContactPlan.md v0.1.0`과 `SEN-P0-00~07` 원격 기술 진행 순서를 현재 Roadmap 상단에 추가했다.
- 첫 Gate는 Source mutation 없는 Foundation Audit이며 TargetSelect=선택 / Sensor=탐지·지식 / HUD=표시 책임 분리를 고정했다.
- 기존 2026-08-13 이하의 “현재 Active” 표현은 Historical 체크포인트로 명시하고 현재 판단에서 제외했다.
- 기존 USER Pending 기능과 dirty 자산을 보호하며 이번 인계 준비에서는 Source·Asset·Build·PIE를 변경하지 않았다.

### v2.23.0 - 2026-08-13

- `CF-FQ-031`의 AMMO-P0-00~08, Heavy·Ripple USER PIE를 완료하고 `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System으로 승격했다.
- CF-FQ-031을 Ready에서 Done으로 이동하고 현재 단일 Active를 `CF-FQ-032`로 복귀시켰다.
- UI-P0-02 전체 USER PASS와 UI-P0-03 Source·Build·Automation·부분 Production PIE 상태를 반영했다.
- CF-FQ-032의 다음 순서를 Launcher 전체 Presentation(Salvo·terminal Cooldown→READY) → Defense 실제 변화 → Pawn Rebind로 고정했다.
- Ammo USER PIE는 완료 증거로 보존하고 다음 UI 검증에서 반복하지 않는다.

### v2.22.0 - 2026-08-06

- UI-P0-02 사용자 Standalone에서 Pause UI 상호작용 범위를 PASS했다.
- Root 계층·마우스·Enter 결함을 `AddToViewport(100)`과 Controller 확인 Fallback으로 최소 수정했다.
- 사용자 직접 Editor 빌드와 Combat Automation `595e0c1ca0f640b69f63964bdfd22277` 전체 46/46·필수 24/24 Success를 반영했다.
- 남은 Gamepad·입력 유지 잔류·Launcher·Projectile·Timer·Root 수명 사용자 PIE 뒤에만 UI-P0-03으로 진행하도록 순서를 유지했다.

### v2.21.0 - 2026-08-06

- `CF-FQ-033`의 DR-PIE-06 Salvo·Ripple·Pool·FX·충돌 사용자 PASS와 DR-P0-07 전체 완료를 반영했다.
- `CF-FQ-033`을 Done으로 전환하고 VehicleDefense·HitDamage를 Current System 기준으로 등록했다.
- 데미지 시스템 완료 뒤 UI로 복귀한다는 기존 우선순위에 따라 `CF-FQ-032`를 현재 단일 Active로 복원했다.
- 다음 순서를 UI-P0-02 실제 Pause·Focus·Launcher·Projectile·Timer 사용자 PIE와 PASS 뒤 UI-P0-03 HUD 데이터 계약으로 고정했다.
- CF-FQ-029·026 Paused와 CF-FQ-030·031·034·035 Ready 체크포인트를 유지했다.
- 코드·에셋·빌드·Automation은 재실행하지 않았고 기존 미커밋 변경을 보호했다.

### v2.20.0 - 2026-08-03

```text
- 사용자 결정에 따라 작업 완료 최우선 사항을 데미지 시스템으로 재정렬했다.
- CF-FQ-033 차량 방어·손상 런타임을 단일 Active로 전환하고 DR-P0-07 사용자 PIE를 현재 단계로 고정했다.
- 실패한 데미지 결함 우선 수정, 공식 Build·전체 Automation, 영향 PIE 재검증과 Systems 승격까지를 하나의 완료 경로로 묶었다.
- 데미지 검증을 직접 차단하는 문제만 예외 선행으로 허용하고 차단 해소 즉시 CF-FQ-033으로 복귀하도록 했다.
- CF-FQ-032는 완료된 UI Foundation을 보존한 Paused 상태로 전환하고 런처·미사일·피팅·인벤토리 체크포인트를 후순위 보호했다.
```

### v2.19.0 - 2026-07-30

```text
- CF-TC-023 투사체 비행 FX 사용자 PIE 전체 행렬을 PASS 처리했다.
- Trail-only, Thruster-only, Trail+Thruster와 유효 소켓·Missing Socket Fallback을 완료했다.
- Hit·LifeExpired Reset, Pool 20발 이상, Ribbon History 무잔류와 30 FPS 고속 Bounds 회귀를 완료했다.
- CF-FQ-027을 Paused에서 Done으로 전환하고 Projectile Current System v1.5.0에 연결했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run 상태와 Runner 미노출 사실을 유지했다.
- 현재 Active CF-FQ-029 LM-P0-06과 CF-FQ-030~032 Ready, CF-FQ-019 Deferred 상태는 변경하지 않았다.
```

### v2.17.0 - 2026-07-29

```text
- CF-FQ-030 Ready 상태 안에서 MG-P0-00 비활성 Foundation Code Applied와 Editor Build PASS를 반영했다.
- Flight·Guidance Config 기본 비활성, 제한형 비례항법 순수 수학과 Runtime Not Connected 경계를 기록했다.
- 현재 Active는 CF-FQ-029 LM-P0-06을 유지하고 MG-P0-01 Flight State는 해당 사용자 PIE 뒤로 유지했다.
```

### v2.16.0 - 2026-07-29

```text
- LM-P0-05 Launcher Editor Assets, 실제 플레이 진입 경로와 독립 AssetDump 검증 완료를 반영했다.
- LM-P0-06 CF-TC-025·026 사용자 PIE를 현재 Active 단계로 이동했다.
- CF-FQ-030은 LM-P0-06과 CF-FQ-029 완료 판정 이후 착수하는 Ready 작업으로 유지했다.
- 사용자 PIE 전에는 CF-FQ-029 또는 Launcher Systems를 완료 처리하지 않는다.
```
- v2.13.0 (2026-07-28)
  - 사용자 결정에 따라 `CF-FQ-019 주행·전투 반복 테스트`를 가까운 후속 Candidate에서 Deferred로 이동했다.
  - 기존 반복 전투 체크리스트는 폐기하지 않고 `CF-FQ-029`와 `CF-FQ-030` 이후 런처·미사일을 포함한 통합 회귀 범위로 다시 설계하도록 고정했다.
  - 현재 Active `CF-FQ-029`와 Ready `CF-FQ-030`의 순서는 유지했다.
  - 소스, 에셋, 빌드와 PIE 상태는 변경하지 않았다.

- v2.12.0 (2026-07-28)
  - `CF-FQ-029 모듈형 런처 및 발사 인계`를 현재 Active 단계로 등록했다.
  - `CF-FQ-030 물리 제한형 미사일 비행·유도`를 CF-FQ-029 선행 조건을 가진 Ready 단계로 등록했다.
  - LauncherMissile Plan·Design·Roadmap과 런처·미사일 상세 Plan을 생성했다.
  - Direct·Angled·Vertical 사출, Single·Ripple·Salvo와 ProjectileMovement 기반 Ejection 순서를 확정했다.
  - 발사 후 런처 독립, Target Snapshot, 물리 제한형 유도와 명중 비보장 원칙을 고정했다.
  - 첫 코드 단계를 `LM-P0-01 Projectile Launch Handoff Foundation`으로 제한하고 현재 Source Not Modified / Code Entry Ready로 기록했다.
  - `CF-FQ-019`, `CF-FQ-027`과 `CF-FQ-026`의 기존 상태는 유지했다.

- v2.11.0 (2026-07-28)
  - `CF-FQ-028 발사체 추진 시스템`을 Done / User PIE PASS / `CF-TC-024 PASS`로 종료했다.
  - `Document/Systems/Combat/Projectile.md v1.4.0`을 비유도 Rocket 추진과 지속형 Projectile FX의 Current System으로 승격했다.
  - `ProjectilePropulsionPlan.md v1.2.0`을 완료 이력 문서로 전환했다.
  - 현재 Active 단계를 비웠다.
  - `CF-FQ-019`를 다음 Candidate / Not Started로 유지하고 자동 착수하지 않았다.
  - `CF-FQ-027`은 런타임 기반이 Current System에 반영됐지만 별도 전체 체크리스트와 `CF-TC-023` 미완료로 Paused 상태를 유지했다.
  - 이번 승격 작업에서는 반복 전투 회귀를 수행하거나 완료 처리하지 않았다.

- v2.10.0 (2026-07-27)
  - 사용자가 승인한 비유도 Rocket 실제 추진 설계를 `CF-FQ-028 발사체 추진 시스템`으로 등록하고 단일 Active 단계로 전환했다.
  - CFProjectileMotorTypes, UCFProjectileMotorComp, ProjectileData PropulsionConfig와 ProjectileActor Motor·Thruster 상태 연결을 PP-P0-01~05 완료 기준으로 등록했다.
  - InitialSpeed 분리, IgnitionDelay, 고정 LaunchDirection 가속, 최대 추진 속도와 BurnedOut 관성·중력 비행을 구현 순서에 추가했다.
  - 첫 Build Job 984f0ccab94241148e7b40b9d30a5572의 Automation 타입 오류를 수정하고, 최종 형식 정리 후 Build Job 2ffd09357e654bb7970a2e379f5ab45f Exit Code 0을 공식 증거로 기록했다.
  - CF-FQ-027은 C++ Foundation을 유지한 채 Editor 자산 연결 단계에서 Paused로 보존하고, CF-FQ-019를 CF-FQ-028 사용자 PIE 후속으로 이동했다.

- v2.9.0 (2026-07-27)
  - CarFight 코드 작업 기본 방침을 별도 Codex 위임에서 현재 AI 세션의 직접 구현으로 변경했다.
  - CF-FQ-027의 Plan Work-Order Generation 차단 상태를 해제하고 PFX-P0-01 직접 구현을 다음 단계로 지정했다.
  - TaskSource와 WorkOrder는 구현 참고 자료로 유지하고 최종 Codex YAML을 착수 조건에서 제거했다.
  - 직접 구현 후 Git diff, 공식 빌드, Automation과 사용자 PIE 검증 순서는 유지했다.

- v2.8.0 (2026-07-27)
  - 사용자 선택에 따라 CF-FQ-027 투사체 비행 FX를 현재 단일 Active 단계로 등록했다.
  - PFX-P0-00에서 대표 Plan, 로드맵, TaskSource와 사람이 검토 가능한 WorkOrder 초안을 준비했다.
  - ACFProjectileActor 소유, FX_Trail·FX_Exhaust 소켓 우선, ProjectileData Fallback과 Pool 반환 전 Reset을 구현 순서로 고정했다.
  - 계획 테스트 ID CF-TC-023과 PFX-P0-01~07 코드·빌드·자산·PIE·Systems 승격 순서를 추가했다.
  - 현재 Admin 표면에 plan.* 품질·증거 게이트가 없어 최종 Codex YAML Missing과 Source Not Modified 상태를 기록했다.
  - CF-FQ-019를 CF-FQ-027 완료 후 후속 Candidate로 이동하고 CF-FQ-026 TS-P0-08 Paused 상태를 유지했다.

- v2.7.0 (2026-07-27)
  - `CF-FQ-024` 최종 사용자 PIE 전체 PASS와 `CF-TC-021 PASS`를 반영했다.
  - C2-B 전투 FX를 완료 상태로 전환하고 `Document/Systems/Combat/CombatFx.md`를 Current System으로 연결했다.
  - Impact `NS_BasicHit` 현재 크기 승인과 차량별 `FX_Destroyed` 소켓 위치 완료를 기록했다.
  - C4 / `CF-FQ-019`를 선행 조건이 해제된 Candidate로 표시하되 자동 Active 전환하지 않았다.
  - `CF-FQ-026`은 TS-P0-08 Paused 상태를 그대로 유지했다.

- v2.6.0 (2026-07-24)
  - 사용자 결정에 따라 `CF-FQ-024 전투 FX`를 현재 단일 Active 단계로 승격했다.
  - 현재 실행 순서를 FAB 콘텐츠 조사 → Muzzle·Impact·Destroyed 후보 확정 → 공용 FX 계약 구현으로 변경했다.
  - `CF-FQ-026`을 `TS-P0-08 Paused`로 보존하고 완료 코드와 Automation 결과를 회귀 보호 대상으로 유지했다.
  - C2-B 상태와 전체 진행 순서를 Active / Phase 0 기준으로 갱신했다.

- v2.4.0 (2026-07-23)
  - 사용자 결정에 따라 CF-FQ-026 타겟 선택 시스템을 현재 Active 단계로 삽입했다.
  - TS-P0-01 소스 및 Editor 빌드 PASS, Blueprint·PIE Pending 체크포인트를 기록했다.
  - CF-FQ-024는 취소하지 않고 Ready / CF-FQ-026 이후 재개 상태로 변경했다.
  - 기존 전투 사이클의 완료 조건과 후속 순서는 유지했다.

- v2.3.0 (2026-07-15)
  - 완료된 C1, C2-A, C3와 조준·Projectile 신뢰성 작업을 현재 진행 체크포인트에 반영했다.
  - 기존 C2를 Reticle/UI 완료 단계 C2-A와 실제 Niagara/Audio 활성 단계 C2-B로 분리했다.
  - CF-FQ-024를 현재 Active로 지정하고 이후 순서를 C4, C5, C6로 재정렬했다.
  - C4 반복 테스트에 FX/Audio 중복, Pool 잔류와 10회 반복 검증을 추가했다.

- v2.2.0 (2026-06-19)
  - 현재 신규 개발 순서를 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증으로 재정렬했다.
  - 기존 S2~S4의 주행감, 카메라/로컬 Aim, WheelSync 품질 항목은 전투 반복 테스트와 피드백 개선 단계 안에서 함께 검증하도록 통합했다.
  - 서버/멀티 제외 원칙은 유지하되, 다음 목표를 싱글 로컬 전투 루프 검증으로 명확히 했다.

- v2.1.4 (2026-06-19)
  - 사용자 에디터 정상작동 확인을 반영해 S1 싱글 실행 기준선과 S3 로컬 Aim/Reticle 전환 완료 조건을 체크했다.
  - Standalone/패키지 검증은 기능 추가가 아니라 후속 검증 후보로 남겼다.

- v2.1.3 (2026-06-19)
  - 이번 세션 목표를 기능 추가 없이 싱글플레이 전환 정리로 고정했다.
  - `CFSingleGameMode` 기본 실행 기준과 로컬 Aim/Fire 전환 완료분을 로드맵에 반영했다.
  - 서버 Target/CFMPGameMode/CFNetSmooth/보관 문서는 삭제하지 않고 Deferred로 보존한다고 정리했다.

- v2.1.2 (2026-06-19)
  - 이번 세션의 목적을 코드/툴 정리가 아니라 멀티플레이 진행 선로를 싱글플레이 진행 선로로 바꾸는 문서 전환으로 축소했다.
  - 서버/멀티 문서 보류 정리를 개발 전 선행 단계인 S0-1로 이동했다.
  - 서버 관련 코드, Target, 플러그인, 배치 파일은 현상 유지하며 이번 세션에서 삭제/분류하지 않는다고 명시했다.

- v2.1.1 (2026-06-19)
  - 서버/멀티 보류 정리 중 문서 Archive 분리 완료분을 체크했다.
  - Plan 루트/Archive README 신설에 맞춰 보류 문서 경로를 `Document/Plan/Archive/` 기준으로 정정했다.
  - 서버 관련 코드/툴 파일 판단은 이번 문서 전환 세션 범위 밖으로 분리했다.

- v2.1.0 (2026-06-18)
  - 이번 사이클 최상위 목표를 서버 권한 Combat Vertical Slice에서 싱글 플레이 1대 차량 고도화로 전환했다.
  - 싱글 전환 진행 순서를 추가하고 서버/멀티/CFNetSmooth/서버 런처/세션/관리툴을 현재 범위 제외로 명시했다.
  - 기존 D0/P0/P1/P2/P3 기록은 2026-06-02 이전 기준선 참고 기록으로 분리했다.
  - 싱글 실행 기준선, 주행감, 카메라/로컬 Aim/Reticle, WheelSync 품질, 서버/멀티 보류 정리를 다음 작업 흐름으로 고정했다.
  - S0 ProjectSSOT 싱글 전환 완료 조건을 완료 상태로 표시했다.
  - CameraPlan 참조 경로를 현재 `Document/Plan/CameraPlan/` 기준으로 정정했다.

- v1.1.0 (2026-04-15)
  - 문서 버전 / 마지막 정리 날짜를 갱신했다.
  - 기존 로드맵을 바꾸지 않는 범위에서 Camera Baseline 후속 착수 메모를 추가했다.

---

## Migration

### v2.26.0 적용 안내

```text
- CF-FQ-036은 Done이며 현재 구현은 Document/Systems/Targeting/SensorContact.md v1.0.0을 우선한다.
- SensorContactPlan.md v0.9.0은 Historical + Retained Path 완료 기록이며 old SEN-P0 next gate를 재실행하지 않는다.
- 현재 단일 Active는 없다. 다음 Feature는 사용자가 FeatureQueue에서 선택한 뒤에만 Active로 전환한다.
- TargetSelect=후보 검색·선택·TrackState, Sensor=Detection·Contact lifecycle·Knowledge, HUD=read-only 표시 책임을 유지한다.
- Radar Range/Zoom·NormalizedPosition·동적 Blip과 CF-FQ-032 Radar/TargetPanel USER Visual은 별도 Pending이다.
- CF-FQ-026 TS-P0-08 USER PIE와 다른 Paused/Ready USER Gate도 그대로 유지한다.
```

### v2.24.0 적용 안내

```text
- 현재 Active는 CF-FQ-036 차량 센서·Contact Intelligence Runtime이다.
- 새 세션은 Document/Plan/SensorContactPlan.md v0.1.0의 SEN-P0-00 Foundation Audit부터 시작한다.
- SEN-P0-00에서는 Source·Config·Content Asset을 수정하지 않고 현재 TargetSelect/HUD/Vehicle/Collision 경계와 재사용 타입을 먼저 확정한다.
- CF-FQ-026 TS-P0-08과 CF-FQ-032 USER Visual은 그대로 Pending이며 Sensor 기술 검증으로 PASS 승격하지 않는다.
- 아래 과거 migration의 Active 표기는 해당 날짜의 역사 상태다.
```

### v2.23.0 적용 안내

```text
- CF-FQ-031은 Done이며 Document/Systems/Combat/Ammo.md v1.0.0이 현재 구현을 소유한다.
- 현재 Active는 CF-FQ-032 UI-P0-03이다.
- Ammo Heavy·Ripple USER PIE는 반복하지 않는다.
- 다음 검증은 Launcher 전체 Presentation(Salvo·terminal Cooldown→READY) → Defense 실제 변화 → Pawn Rebind다.
- 이 범위 전체 USER PASS 전에는 UI-P0-03 또는 CF-FQ-032를 Done 처리하지 않는다.
- CF-FQ-029·026 Paused, CF-FQ-030·034·035 Ready 체크포인트는 유지한다.
```

### v2.21.0 적용 안내

```text
- 현재 Active는 CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크다.
- UI-P0-01A~02는 Code Complete·Build·Automation PASS이므로 UI-P0-02 사용자 PIE부터 시작한다.
- UI-P0-02 PASS 뒤 UI-P0-03 Vehicle·Weapon·Defense·Target·Radar·Alert HUD 데이터 계약으로 진행한다.
- CF-FQ-033은 Done이며 VehicleDefense.md와 HitDamage.md가 현재 구현을 소유한다.
- Defense HUD는 실제 Runtime을 읽기만 하고 피해 계산을 UI에서 재작성하지 않는다.
- 기존 Launcher·Missile·Fitting·Inventory·TargetSelect 체크포인트는 보호한다.
```

### v2.20.0 적용 안내

```text
- 새 세션의 현재 Active는 CF-FQ-033 차량 방어·손상 런타임이다.
- 대표 Plan은 Document/Plan/VehicleDefenseDamageDesign.md v0.10.0이다.
- DR-P0-00~06은 완료 상태이므로 반복 구현하지 않고 DR-P0-07 DR-PIE-00 시작 Gate부터 진행한다.
- 데미지 PIE 실패는 다른 신규 기능보다 먼저 수정하며 전체 PASS 후 VehicleDefense·HitDamage Systems 승격과 CF-FQ-033 Done을 수행한다.
- CF-FQ-032, CF-FQ-029와 CF-FQ-026은 Paused 체크포인트를 유지한다.
```

### v2.19.0 적용 안내

```text
- CF-FQ-027은 Done / CF-TC-023 PASS이며 Active 또는 Paused로 복원하지 않는다.
- 투사체 비행 FX의 현재 구현 판단은 Document/Systems/Combat/Projectile.md v1.5.0을 우선한다.
- ProjectileFlightFxPlan.md v1.0.0은 완료 당시 구현·빌드·사용자 PIE 기록으로 유지한다.
- 현재 단일 Active는 CF-FQ-029 LM-P0-06이며 이번 완료 처리로 런처 우선순위를 변경하지 않는다.
- Automation 실행은 Runner 미노출로 Not Run 상태를 유지한다.
```

### v2.13.0 적용 안내

```text
- CF-FQ-019는 Deferred이며 현재 후속 Candidate로 복원하지 않는다.
- 기존 CF-FQ-019 테스트 항목은 삭제하지 않고 런처·미사일 구현 뒤 새 통합 회귀 범위의 입력으로 사용한다.
- 현재 코드 진입점은 LM-P0-01 Projectile Launch Handoff Foundation이다.
- CF-FQ-029 Active와 CF-FQ-030 Ready 상태는 유지한다.
```

### v2.12.0 적용 안내

```text
- 현재 Active 단계는 LCH / CF-FQ-029 모듈형 런처 및 발사 인계다.
- 대표 Plan은 Document/Plan/LauncherMissilePlan.md다.
- LM-P0-00 조사·설계·문서 준비는 완료됐으므로 반복하지 않는다.
- 다음 작업은 LM-P0-01 Projectile Launch Handoff Foundation 직접 구현이다.
- 첫 Task에서는 기존 직사 발사의 위치·방향·속도와 Rocket FixedThrustDirection을 회귀 보호한다.
- CF-FQ-030은 CF-FQ-029 Launch Handoff와 Direct Launcher 검증 전에는 코드 착수하지 않는다.
- CF-FQ-019, CF-FQ-027과 CF-FQ-026의 기존 Candidate·Paused 상태를 유지한다.
```

### v2.11.0 적용 안내

```text
- CF-FQ-028은 Done / CF-TC-024 PASS이며 Active로 복원하지 않는다.
- 현재 구현 판단은 Document/Systems/Combat/Projectile.md를 우선한다.
- ProjectilePropulsionPlan.md는 완료 이력으로 유지한다.
- 현재 Active 단계는 없다.
- CF-FQ-019는 다음 Candidate지만 사용자가 선택하기 전에는 착수하지 않는다.
- CF-FQ-027과 CF-FQ-026의 Paused 체크포인트를 유지한다.
```

### v2.10.0 적용 안내

```text
- 현재 Active 단계는 PROP / CF-FQ-028 발사체 추진 시스템이다.
- 대표 Plan은 Document/Plan/ProjectilePropulsionPlan.md다.
- PP-P0-00~07 C++ Foundation과 공식 Editor 빌드는 완료됐으므로 반복 구현하지 않는다.
- 다음 작업은 Editor 테스트 Rocket ProjectileData 생성, PropulsionConfig·FX_Exhaust 저장과 사용자 PIE다.
- 기존 ProjectileData는 bUsePropulsion=false 기본값으로 기존 InitialSpeed 비행을 유지한다.
- CF-FQ-027은 Editor 자산 연결 단계 Paused, CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
- CF-FQ-019는 CF-FQ-028 사용자 PIE 완료 후 착수하는 반복 전투 Candidate다.
```

### v2.9.0 적용 안내

```text
- 현재 Active 단계는 CF-FQ-027 투사체 비행 FX다.
- 새 세션은 CodeWorkGate.md v2.0에 따라 현재 AI 세션이 코드 작업을 직접 수행한다.
- PFX-P0-00 조사·설계는 완료됐으므로 반복하지 않는다.
- PFX-P0-01 ProjectileData 계약부터 직접 구현한다.
- TaskSource와 WorkOrder는 구현 참고 자료이며 최종 Codex YAML을 기다리지 않는다.
- CF-FQ-024는 Done 상태를 유지하고 일회성 CombatFx를 다시 구현하지 않는다.
- CF-FQ-019는 CF-FQ-027 완료 후 반복 전투 Candidate다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v2.8.0 적용 안내 — 폐기됨, v2.9.0이 대체

> 아래 내용은 당시 실행 기준 보존용이며 현재 코드 작업 판단에는 사용하지 않는다.

```text
- 현재 Active 단계는 CF-FQ-027 투사체 비행 FX다.
- 새 세션은 Document/Plan/ProjectileFlightFxPlan.md의 PFX-P0-00 체크포인트를 우선 복원한다.
- PFX-P0-00 조사·설계는 완료됐으므로 반복하지 않는다.
- 최종 Codex YAML은 Missing이며 plan.* 품질·증거 게이트 전에는 소스 변경을 시작하지 않는다.
- CF-FQ-024는 Done 상태를 유지하고 일회성 CombatFx를 다시 구현하지 않는다.
- CF-FQ-019는 CF-FQ-027 완료 후 반복 전투 Candidate다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v2.7.0 적용 안내

```text
- C2-B / CF-FQ-024는 Done / User PIE PASS / CF-TC-021 PASS다.
- 현재 전투 FX 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- CombatFxAudio Plan은 완료 이력으로 유지하며 Active 실행 문서로 사용하지 않는다.
- C4 / CF-FQ-019는 착수 가능하지만 사용자가 선택할 때만 Active로 전환한다.
- CF-FQ-026은 TS-P0-08 Paused 체크포인트를 유지한다.
```

### v2.6.0 적용 안내

```text
- 현재 Active 단계는 C2-B / CF-FQ-024 전투 FX다.
- 첫 단계는 반입된 FAB 콘텐츠의 NiagaraSystem과 의존성을 확인하는 Phase 0이다.
- CF-FQ-026은 TS-P0-08 Paused이며 FX 작업 중 TargetSelect 구현을 변경하거나 되돌리지 않는다.
- 후보 FX가 확정되기 전에는 공용 C++ 런타임 구현으로 넘어가지 않는다.
- C4 반복 전투 테스트는 CF-FQ-024 사용자 PIE 완료 뒤 착수한다.
```

### v2.5.0 적용 안내

```text
- C2-B와 CF-FQ-024는 Niagara 기반 전투 FX 전용 단계로 해석한다.
- CombatFxAudio는 레거시 경로명이며 Sound 자산이나 Audio 런타임을 뜻하지 않는다.
- C4~C6의 완료 판정은 화면, UI, 시각 FX와 게임 판정만으로 수행한다.
- 게임 오디오 에셋·클래스·모듈을 새 작업에 추가하지 않는다.
```

### v2.3.0 적용 안내

```text
- 기존 C2의 Reticle/UI 완료 결과는 C2-A로 유지한다.
- 실제 Niagara와 공간 사운드 구현은 C2-B / CF-FQ-024로 진행한다.
- C4 반복 전투 테스트는 C2-B 사용자 PIE 완료 전에는 착수하지 않는다.
- 완료된 C1과 C3의 판정 기준은 관련 Systems와 TestChecklist PASS 상태를 유지한다.
- 현재 활성 상세 계획은 Document/Plan/CombatFxAudio/ImplementationDesign.md를 사용한다.
```
