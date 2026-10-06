# Vehicle Targeting Current System

- Version: 1.5.0
- Date: 2026-10-06
- Status: Current / Phase 5 Gameplay Command + Phase 6 HUD Presentation + Phase 7 Guided Weapon Source + Production Lock Input/Retarget Technical PASS
- Design Baseline: `Document/ProjectSSOT/CombatPlan/09A_TargetingSensorArch.md v0.1.16`

---

## 1. 목적

이 문서는 CarFight의 현재 **Vehicle Target Lock Runtime(차량 타겟 락 런타임)** 구현을 기록한다.

Vehicle Targeting은 TargetSelect와 Sensor 사이에 새로 추가된 독립 Gameplay Runtime이다.

```text
TargetSelect
= 플레이어가 무엇을 후보로 보고 선택했는가

Sensor / Contact
= 무엇을 탐지하고 있는가
= 현재 Contact가 Live / LastKnown / Lost / DestroyedHold 중 무엇인가
= 대상에 대해 어떤 Knowledge를 알고 있는가

Vehicle Targeting
= 현재 차량이 어느 Sensor Contact에 Lock을 획득 중인가
= Lock이 완료됐는가
= 현재 Lock Quality가 얼마인가
= Lock이 실제로 끊어졌다면 어떤 이유인가

Fire / Guided Weapon
= Direct Fire는 기존 Aim/Fire 계약 유지
= 실제 Projectile Actor + TargetActor Guidance일 때만 Vehicle Locked Target을 발사 전 Guidance source로 요구
= Selection은 Guidance source가 아님

HUD
= Phase 6부터 actor-free FCFTargetingSnapshot을 독립 FCFTargetLockHUDData로 투영
= 현재 Selection / Target Scan과 별도 Presentation channel 유지

Input
= Phase 5 VehiclePawn command facade 구현 완료
= `/Game/CarFight/Input/IA_TargetLock` Boolean Input Action을 Production 입력으로 연결
= 기본 `IMC_Vehicle_Default`에서 T 키 사용
= T 입력은 현재 Selected Target에 먼저 Lock 요청
= 같은 Target이 이미 Acquiring/Locked면 해당 Lock만 토글 해제
= 다른 유효 Selected Target이면 기존 Lock에서 새 Target Acquiring으로 즉시 교체
= 새 Target이 Live Contact가 아니어 Lock 요청이 거부되면 기존 Lock은 보존
= Selection / Scan과 자동 결합하지 않음
```

핵심 불변식은 다음과 같다.

```text
Selected != Contact
Contact != Locked
Knowledge != Lock
Selected != Guidance Source
Lock == TargetActor Guided Weapon pre-launch source authority
```

Selection 변경만으로 기존 Vehicle Lock이 자동 변경되지 않으며, Sensor Contact가 존재한다는 이유만으로 자동 Lock되지 않는다.

---

## 2. Current Source Authority

현재 Vehicle Targeting Source authority는 다음 파일이다.

```text
UE/Source/CarFight_Re/Public/CFTargetingTypes.h
UE/Source/CarFight_Re/Private/CFTargetingTypes.cpp
UE/Source/CarFight_Re/Public/CFVehicleTargetingComp.h
UE/Source/CarFight_Re/Private/CFVehicleTargetingComp.cpp
UE/Source/CarFight_Re/Private/CFTargetingTests.cpp
UE/Source/CarFight_Re/Private/CFTargetInputTests.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

Sensor Contact source authority는 `Targeting/SensorContact.md`, Selection source authority는 `Targeting/TargetSelect.md`가 계속 별도로 소유한다.

---

## 3. Vehicle Targeting Authority

`UCFVehicleTargetingComp`가 한 차량의 단일 Vehicle Target Lock 상태를 소유한다.

`ACFVehiclePawn`은 다음 기본 서브오브젝트를 서로 독립적으로 가진다.

```text
TargetSelectComp
VehicleSensorComp
VehicleTargetingComp
```

각 Component가 다른 Component의 상태를 대신 소유하지 않는다.

Vehicle Targeting의 P0 지속 상태는 exact3이다.

```text
Idle
Acquiring
Locked
```

### Idle

현재 Acquire/Lock 대상이 없다.

```text
TargetContactId = None
LockProgress01 = 0
LockQuality01 = 0
```

과거 실제 Lock Break가 있었다면 `BreakTransitionRevision`과 `LastBreakReason`은 Idle에서도 보존될 수 있다.

### Acquiring

현재 Sensor의 exact ContactId 하나를 대상으로 Lock을 획득 중이다.

```text
TargetContactId = valid
0 <= LockProgress01 < 1
LockQuality01 = 0
```

### Locked

Lock 획득이 완료된 상태다.

```text
TargetContactId = valid
LockProgress01 = 1
LockQuality01 > 0
```

---

## 4. RequestLock 계약

`RequestLock(AActor* TargetActor)`은 임의 Actor를 바로 Lock하지 않는다.

현재 요청 순서는 다음과 같다.

```text
Targeting Runtime Ready 확인
→ TargetActor valid 확인
→ Vehicle Sensor Runtime Ready 확인
→ Actor → current ContactId bridge 확인
→ fresh Sensor Snapshot 획득
→ exact ContactId Contact 확인
→ ContactState == Live 확인
→ 같은 Contact 중복 요청 여부 확인
→ 기존 active target clear
→ 새 Contact를 Acquiring으로 시작
```

따라서 **현재 Sensor가 Live로 관측 중인 Contact만 새 Lock 요청 대상이 될 수 있다.**

같은 Contact의 중복 요청은 진행률을 재시작하지 않는다.

```text
같은 Contact + Acquiring
→ AlreadyAcquiring

같은 Contact + Locked
→ AlreadyLocked
```

다른 유효 Live Contact에 새 RequestLock이 들어오면 단일 Vehicle Lock 대상이 새 Contact로 교체된다.

RequestLock의 명시적 결과는 다음 enum으로 공개된다.

```text
None
Accepted
InvalidTarget
RuntimeNotReady
SensorUnavailable
NoSensorContact
ContactNotLive
AlreadyAcquiring
AlreadyLocked
```

---

## 5. Lock 진행과 Contact lifecycle

Vehicle Targeting은 Sensor Snapshot을 읽지만 Sensor Contact lifecycle을 변경하지 않는다.

### Acquiring + Live

`LockAcquireGainPerSec`만큼 `LockProgress01`이 증가한다.

1.0에 도달하면:

```text
State = Locked
LockProgress01 = 1
LockQuality01 = 1
```

### Acquiring + LastKnown

`LockAcquireDecayPerSec`만큼 진행률이 감소한다.

이 상태는 아직 Lock Break가 아니다. Acquiring은 아직 완성된 Lock이 아니므로 Lost/Destroyed 등으로 취소돼도 Break transition을 발행하지 않는다.

### Locked + Live

`LockQualityRecoveryPerSec`만큼 Quality가 회복된다.

### Locked + LastKnown

`LockQualityDecayPerSec`만큼 Quality가 감소한다.

Quality가 0에 도달하면:

```text
BreakReason = QualityDepleted
→ Idle
```

### Locked + Lost

즉시:

```text
BreakReason = ContactLost
→ Idle
```

### Locked + DestroyedHold

즉시:

```text
BreakReason = TargetDestroyed
→ Idle
```

Sensor Runtime 자체를 사용할 수 없게 된 상태에서 기존 Lock을 유지하고 있었다면:

```text
BreakReason = SensorUnavailable
→ Idle
```

---

## 6. Lock Break exact-once 계약

실제 `Locked` 상태가 끊어질 때만 `BreakTransitionRevision`이 1 증가한다.

```text
BreakTransitionRevision
= 실제 Lock Break event의 단조 증가 revision

LastBreakReason
= 가장 최근 BreakTransitionRevision의 원인
```

현재 Break 사유는 다음 exact4다.

```text
ContactLost
TargetDestroyed
SensorUnavailable
QualityDepleted
```

`ClearLock()`은 사용자가 명시적으로 현재 Acquire/Lock을 해제하는 command이며 **Lock Break event가 아니다.**

따라서 수동 Clear에서는:

```text
State → Idle
BreakTransitionRevision 유지
LastBreakReason 유지
```

HUD가 Phase 6에서 Break feedback을 추가할 경우 이 Revision을 이용해 같은 Break를 중복 소비하지 않는 구조로 연결할 수 있다.

---

## 7. ContactId와 자동 재연결 금지

Vehicle Lock은 현재 Sensor Contact lifetime의 exact `ContactId`를 기준으로 추적한다.

Sensor의 Persistent Knowledge Store는 같은 `TargetEntityId`가 Lost→Removed 뒤 재발견되면 과거 Knowledge를 새 Contact에 reassociate할 수 있다.

그러나 Vehicle Lock은 그 동작을 따라 자동 복구하지 않는다.

```text
기존 Locked Contact_A
→ Contact_A Lost/Removed
→ Lock Break / Idle
→ 같은 Entity가 Contact_B로 재발견

결과:
Knowledge reassociation 가능
Vehicle Lock 자동 reassociation 금지
```

새 Contact에 다시 Lock하려면 새 Gameplay command를 통해 `RequestLock()`을 명시적으로 호출해야 한다.

이 경계는 **Knowledge persistence와 Lock authority를 분리**하기 위한 의도된 계약이다.

---

## 8. Targeting Config

Phase 4는 별도 Targeting DataAsset/Profile 체계를 선행 구현하지 않는다.

`UCFVehicleTargetingComp`가 작은 `FCFTargetingConfig` fallback을 직접 소유한다.

현재 P0 기본값:

```text
LockAcquireGainPerSec       = 1.0
LockAcquireDecayPerSec      = 0.5
LockQualityRecoveryPerSec   = 1.0
LockQualityDecayPerSec      = 0.5
```

이 값은 최종 밸런스가 아니라 Runtime Foundation을 위한 provisional fallback이다.

중요한 경계:

```text
Sensor AnalysisGain/Decay
!=
Vehicle Target Lock Gain/Decay
```

향후 차량/센서/장비별 Targeting Profile이 실제로 필요해지면 `FCFTargetingConfig`의 source만 승격한다. Sensor Analysis tuning과 합치지 않는다.

---

## 9. Actor-free Targeting Snapshot

HUD와 일반 Consumer가 읽는 공개 상태는 `FCFTargetingSnapshot`이다.

현재 필드:

```text
State
TargetContactId
LockProgress01
LockQuality01
BreakTransitionRevision
LastBreakReason
```

Snapshot은 Actor/UObject 포인터를 포함하지 않는다.

`GetLockedTargetActor()`는 Phase 7 TargetActor Guided Weapon 같은 내부 Gameplay Consumer용 bridge다. Runtime Ready + Locked + valid public Snapshot + valid non-destroying Actor를 모두 만족할 때만 Actor를 반환하며 HUD truth로는 사용하지 않는다.

현재 Snapshot의 상태별 계약은 `IsPublicContractValid()`로 검증한다.

---

## 10. VehiclePawn 연결

`ACFVehiclePawn`은 `VehicleTargetingComp`를 기본 서브오브젝트로 생성한다.

Blueprint에는 다음 getter가 공개된다.

```text
차량 타겟팅 컴포넌트 반환
(GetVehicleTargetingComp)
```

Phase 4의 기본 Component ownership 위에 Phase 5 command facade, Phase 6 HUD Presentation, Phase 7 Guided Weapon source consumption이 순차적으로 연결됐다.

현재 아직 없는 연결:

```text
- 플레이어 전용 새 Lock Input Action / 물리 key
- 고급 조준/거리/기동 기반 Lock Acquisition Policy
```

따라서 Component가 존재한다는 사실을 완성형 플레이어 Lock UX 구현으로 확대 해석하면 안 된다.

---

## 11. TargetSelect / Sensor / Fire와의 경계

### TargetSelect

TargetSelect는 계속 Selected Target authority다.

Selection 변경은 VehicleTargetingComp 상태를 자동 변경하지 않는다.

### Sensor

Sensor는 Contact state와 actor-free Sensor Snapshot authority다.

VehicleTargetingComp는 Sensor Snapshot을 읽을 뿐 Contact 생성·삭제·Knowledge progression을 변경하지 않는다.

### Direct Fire

Direct Fire는 현재 Vehicle Lock 없이 기존 Aim/Fire 계약으로 계속 동작한다.

### Guided Weapon

Phase 7부터 실제 Projectile Actor 실행 경로이면서 `MissileGuideConfig.IsGuidanceEnabled() == true`이고 `GuideMode == TargetActor`인 경우에만 Vehicle Lock이 발사 전 Guidance source authority가 된다.

```text
VehicleFireComp
→ VehicleTargetingComp::GetLockedTargetActor()
→ 첫 승인 발사 순간 GuidanceTargetActor snapshot
→ Launcher / Projectile LaunchContext
→ MissileGuideComp
```

`Idle / Acquiring / RuntimeNotReady / invalid Locked Actor`에서는 `GuidanceTargetUnavailable`으로 Projectile acquire와 Ammo reservation 전에 fail-closed한다. 반대로 HitScan, 비유도 Projectile, TargetActor 이외 GuideMode는 Vehicle Lock을 새 발사 전제조건으로 사용하지 않는다.

Ripple/Salvo는 첫 승인 발사에서 캡처한 GuidanceTargetActor를 Volley 전체에 유지한다. 이후 Selection 변경이나 새 Lock은 진행 중 Volley를 자동 retarget하지 않는다.

---

## 12. 현재 미구현 범위

다음은 Phase 6 Technical PASS 이후에도 Current System에 포함되지 않는다.

```text
- Phase 3B-2 Dynamic Knowledge Domain
- Knowledge Freshness / TTL
- Rescan / Refresh API
- 새 Lock Input Action / 물리 key mapping
- CrosshairHold / TargetCone / LOS / 거리 / 목표 기동 기반 실제 Lock Acquisition Policy
- 엄폐/횡이동/연막을 반영한 고급 Lock Quality
- 차량/센서/장비별 Targeting DataAsset/Profile
- Multiplayer replication / server authority / shared lock
```

이 항목은 각각 `09A_TargetingSensorArch.md`의 후속 Phase에서 다룬다.

---

## 13. Fresh Technical Validation

Phase 4 초기 구현 후 source review에서 RequestLock의 validation order에 blocking P1 1건을 발견했다.

문제:

```text
같은 Contact의 AlreadyLocked / AlreadyAcquiring 판정
→ fresh Contact Live 검증보다 먼저 수행
```

교정:

```text
fresh Sensor Snapshot
→ Contact 존재
→ Contact Live
→ 중복 Lock 판정
```

교정 후 최종 검증:

```text
Official UE 5.8 Build
Job = 5dd08ae80e5d4ea0884d1d5e914e5c71
CarFight_ReEditor Win64 Development
PASS / ExitCode 0

CarFight.Targeting
Job = d91049be29514b27b3f57397c39d69d5
3 / 3 PASS
Failure 0
- TGT_P0_01.TypesContract
- TGT_P0_02.RuntimeState
- TGT_P0_03.PawnFoundation

CarFight.TargetSelect regression
Job = d05c2385a69642ad9d5f58f1a6f8c6ff
11 / 11 PASS
Failure 0

CarFight.Sensor regression
Job = cb265d1da2724940a672c46342b41adf
17 / 17 PASS
Failure 0
```

Post-Implementation Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 4 Technical PASS
```

Automation 실행 중 `Tools/RunUIAutomation.ps1`의 허용 prefix는 일시 확장 후 원문으로 복원했으며 최종 runner diff는 0이다.

### 13.1 Mid-review Correction / Final Re-review

Phase 4 완료 후 독립 중간검수에서 다음 잔여 사항을 확인했다.

```text
P0 = 0
blocking P1 = 1
P2 = 1
Verdict = HOLD / Correction Required
```

blocking P1은 `CarFight.Targeting` 기존 exact3가 상태 머신 내부를 private seed로 검증했지만, 실제 Gameplay 진입점인 public `RequestLock(AActor*)`를 한 번도 호출하지 않아 다음 계약의 acceptance evidence가 없었던 문제다.

```text
Live Contact → Accepted
같은 Acquiring target 재요청 → AlreadyAcquiring / progress 유지
같은 Locked target 재요청 → AlreadyLocked
Locked(A) → RequestLock(B) → B Acquiring으로 명시 교체
LastKnown → ContactNotLive
Sensor Contact 없음 → NoSensorContact
invalid target → InvalidTarget
명시 A→B replacement → BreakTransitionRevision 비증가
```

교정은 Product Runtime 의미를 바꾸지 않았다. `UCFVehicleSensorComp`와 `UCFVehicleTargetingComp`에 `FCFTargetingRequestLockTest` exact test friend만 추가하고, `TGT_P0_04.RequestLockPublicPath`가 실제 Actor→ContactId bridge + fresh Sensor Snapshot + public `RequestLock()` 경로를 통과하도록 focused Automation을 보강했다.

P2는 상위 Targeting 설계의 `CrosshairHold / TargetCone / LOS / 거리 / 목표 기동` 기반 실제 Lock Acquisition Policy가 Phase 4 Current 범위로 오해될 수 있던 문서 경계다. Phase 4 Current는 **Live Contact + 독립 fallback gain/decay 기반 Lock Runtime Foundation**까지만 소유하며, 실제 조준/거리/기동 기반 획득 정책은 후속 Gameplay/Targeting 정책 범위로 남긴다고 명시했다.

중간검수 교정 후 Fresh Validation:

```text
Official UE 5.8 Build
Job = 767de4c7b3a244a98631d1ac0bf5c873
CarFight_ReEditor Win64 Development
PASS / ExitCode 0

CarFight.Targeting
Job = adaac5891c564259b3b7b09e49689773
4 / 4 PASS
Failure 0
- TGT_P0_01.TypesContract
- TGT_P0_02.RuntimeState
- TGT_P0_03.PawnFoundation
- TGT_P0_04.RequestLockPublicPath

CarFight.TargetSelect regression
Job = 1dcd5872717744028ae8e2bb98ba2a26
11 / 11 PASS
Failure 0

CarFight.Sensor regression
Job = 1050686421b14fef986816df4eab14eb
17 / 17 PASS
Failure 0
```

최종 재검수:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 4 Technical PASS Restored
```

검증 동안만 임시 확장한 `Tools/RunUIAutomation.ps1` allowlist는 원문으로 복원했으며 최종 Git diff는 exact 0이다.

### 13.2 Phase 6 HUD Presentation Technical Validation

Phase 6에서는 Vehicle Targeting Runtime을 재작성하지 않고 공개 `FCFTargetingSnapshot`을 HUD read-only source로 사용한다.

```text
FCFTargetingSnapshot
→ UCFHUDDataProvider::FillTargetLockViewData()
→ FCFTargetLockHUDData
→ UCFHUDPresenter::ApplyTargetLockViewData()
→ WBP_CFTargetPanel.Text_TargetLock / ProgressBar_TargetLock
```

Lock HUD는 Selection Target ViewData와 분리되어 `Selected B / Acquiring or Locked A`를 표현할 수 있다. Lock target identity 보강은 `TargetContactId`로 같은 Refresh의 Sensor Snapshot Contact를 찾으며 `GetLockedTargetActor()`를 HUD truth로 사용하지 않는다.

`BreakTransitionRevision`은 실제 Lock Break feedback의 exact-once Presentation key이며 `ClearLock()` 같은 manual non-break clear는 새 Break feedback을 만들지 않는다.

Production `WBP_CFTargetPanel`에는 기존 Designer Tree를 rebuild하지 않고 `Text_TargetLock / ProgressBar_TargetLock` exact2만 additive 추가됐다. fresh AssetDump에서 Widget count `11 → 13`과 기존 Selection/Knowledge/Scan Widget 보존을 확인했다.

Fresh Validation:

```text
Official UE 5.8 Build = f899ebafdb8d43a88e6788561b4c7a8f PASS
Persisted TargetPanel 후 final Build = 608d40838ff1410680a0140d630f1e50 PASS
CarFight.Targeting.Phase6.HUDPresentationChannels = 1/1 PASS
CarFight.UI.UI_P0_07.TargetKnowledgePanelContract = 1/1 PASS
```

Phase 7 `CFLauncherComp / CFMissileGuideComp` 경로는 worktree diff exact0이며 Locked Target을 Guidance source로 사용하는 migration은 아직 수행하지 않았다.

---

## 14. 다음 단계

Phase 3B-2는 실제 Dynamic Knowledge Domain 요구가 없으므로 계속 미착수 상태로 유지한다.

Phase 5에서 현재 구현된 Gameplay command 경계는 다음과 같다.

```text
VehiclePawn::RequestLockSelectedTarget()
→ 명령 시점 Selected Actor exact-once read
→ VehicleTargetingComp::RequestLock()

VehiclePawn::RequestClearTargetLock()
→ Lock-only manual clear / non-break

VehiclePawn::RequestStartTargetScan()
→ 명령 시점 Selected Actor read
→ Sensor가 Actor + ContactId capture

VehiclePawn::RequestCancelTargetScan()
→ Target Scan Attempt only cancel
→ Active Detection / Selection / Lock 유지
```

Selection 변경 또는 수동 해제는 이미 시작된 Acquire/Lock과 Scan Attempt를 자동 교체·취소하지 않는다. 새 Manager/Subsystem은 추가하지 않았고 Pawn은 얇은 Gameplay facade만 담당한다.

Phase 7 Guided Weapon source migration까지 Technical PASS로 완료됐다. TargetActor Guided Projectile은 발사 전 Vehicle Locked Target을 요구하며, Direct Fire/HitScan/비유도 Projectile은 기존 계약을 유지한다. `IA_TargetLock`의 T 입력은 현재 Selected Target에 먼저 `RequestLockSelectedTarget()`을 시도한다. 같은 대상의 `AlreadyAcquiring/AlreadyLocked`만 `RequestClearTargetLock()`으로 토글 해제하고, 다른 유효 대상의 `Accepted`는 `VehicleTargetingComp::RequestLock()`의 기존 replacement 계약으로 즉시 새 Acquiring을 시작한다. 새 대상 요청이 `NoSensorContact/ContactNotLive` 등으로 실패하면 기존 Lock을 지우지 않는다. Selection / Lock / Scan 독립 계약은 유지한다. Lock admission과 유지가 Sensor Live Contact를 요구하므로 현재 Basic Sensor 운용거리는 `SensorContact.md`의 600/800/1200m baseline을 따른다.

---

## 15. Changelog

### v1.5.0 - 2026-10-06

- USER 검수에서 첫 Lock 후 다른 Target을 선택해도 T 입력이 기존 Lock을 무조건 clear하여 즉시 재타겟할 수 없던 입력 의미 결함을 확인했다.
- T 입력을 `현재 Selected Target RequestLock 우선`으로 교정했다. 같은 대상 중복 결과(`AlreadyAcquiring/AlreadyLocked`)만 Lock-only clear로 토글하고, 다른 유효 Selected Target은 기존 `RequestLock()` replacement 계약으로 즉시 Acquiring을 전환한다. 새 대상 Lock 거부 시 기존 Lock은 보존한다.
- 동시에 Vehicle Lock이 의존하는 Basic Sensor Live Contact의 기존 20/30/40m 운용거리 불일치를 `SensorContact.md v1.16.0`의 600/800/1200m baseline으로 교정했다.
- Official UE 5.8 Editor Build `5b2e29e0f60842a8a9746a4f7ede9cf4` PASS, `CarFight.Targeting.TGT_P0_05.GameplayCommandBoundary` 1/1 PASS, `CarFight.Sensor.SEN_P0_04.BasicSensorSingleScanCompletion` 1/1 PASS, `CarFight.Missile.MG_P0_01_04.DirectRuntimeContract` PASS를 확보했다. Sensor 수치 이관용 일회성 migration Automation은 persisted 반영 확인 후 제거했다.

Migration: T는 같은 대상 Lock 토글과 다른 대상 retarget을 구분한다. Selection 변경 자체는 여전히 Lock을 자동 변경하지 않으며, 사용자가 T를 눌렀을 때만 현재 Selected Target을 새 Lock 대상으로 소비한다.

### v1.4.0 - 2026-10-02

- Phase 7 Guided Weapon이 Vehicle Locked Target을 필수로 소비하지만 실제 Production Lock 입력이 없던 integration gap을 교정했다.
- `/Game/CarFight/Input/IA_TargetLock` Boolean Input Action을 추가하고 `IMC_Vehicle_Default`의 T 키에 매핑했다.
- `ACFVehiclePawn`은 `InputAction_TargetLock`을 별도로 로드·바인딩하며, Idle에서는 `RequestLockSelectedTarget()`, Acquiring/Locked에서는 `RequestClearTargetLock()`을 호출한다. Selection과 Scan은 변경하지 않는다.
- Official UE 5.8 Editor Build job `b7a9530ca06940109becfabefbd45392` PASS, `CarFight.TargetSelect.TS_P0_05.InputIntegration` 1/1 PASS, `CarFight.Targeting.TGT_P0_05.GameplayCommandBoundary` 1/1 PASS, `CarFight.Missile.MG_P0_01_04.DirectRuntimeContract` PASS를 확보했다.
- fresh AssetDump `adset_v1_c531d0a394f4edbf36b1f0a3c5fc04db.7982219f5326cff56114dd64`에서 `IA_TargetLock` Boolean/Pressed Trigger와 `IMC_Vehicle_Default`의 `IA_TargetLock <- T` persisted mapping을 확인했다.

### v1.3.0 - 2026-09-18

- Phase 7에서 TargetActor Guided Projectile의 Guidance source를 Selected Target에서 Vehicle Locked Target으로 전환했다.
- `GetLockedTargetActor()`는 Runtime Ready + Locked + valid public Snapshot + valid non-destroying Actor를 모두 만족할 때만 내부 Gameplay bridge를 반환한다.
- `VehicleFireComp`는 실제 Projectile Actor + TargetActor Guidance에만 Lock을 요구하고, `GuidanceTargetUnavailable`을 Projectile acquire/Ammo reservation 전에 fail-closed한다.
- `Selected B / Locked A` 실제 발사에서 MissileGuideComp가 A를 GuidanceTargetActor로 받은 것을 focused Automation으로 검증했다. Acquiring은 발사 차단, HitScan/Guidance-disabled Projectile은 무Lock 호환을 유지한다.
- Ripple/Salvo는 첫 승인 발사의 Guidance snapshot을 유지하고 이후 Selection/새 Lock 변경으로 자동 retarget하지 않는다.
- Final Official UE 5.8 Build `052b1fc8d0414f6187013655dd19f44d` PASS, Phase 7 exact `731a047740c244e096e43c1f048243a1` PASS, Launcher Scheduler `1e0ad2dafbee477e83e8bbd07583c951` PASS, Direct Missile Runtime `fe4e25d60dcb4bfaa752325555bbd362` PASS를 확보했다.

### v1.2.0 - 2026-09-18

- Phase 6에서 actor-free `FCFTargetingSnapshot`을 독립 `FCFTargetLockHUDData`로 투영하고 Selection/Scan과 분리된 Target Lock HUD Presentation을 Current로 승격했다.
- `UCFHUDPresenter::ApplyTargetLockViewData()`가 Acquiring progress / Locked quality와 `BreakTransitionRevision` 기반 일시 Break feedback을 Production TargetPanel의 전용 Lock sink에 적용한다.
- `WBP_CFTargetPanel` 기존 Designer Tree는 rebuild하지 않고 `Text_TargetLock / ProgressBar_TargetLock` exact2만 additive 추가했으며 fresh AssetDump Widget 11→13을 확인했다.
- Official UE 5.8 Build `f899ebafdb8d43a88e6788561b4c7a8f` 및 final up-to-date Build `608d40838ff1410680a0140d630f1e50` PASS, Phase 6 Provider/Production HUD focused Automation 각 1/1 PASS를 확보했다.
- Phase 7 Guided Weapon source migration은 수행하지 않았고 관련 Launcher/MissileGuide worktree diff exact0을 확인했다.

### v1.1.0 - 2026-09-18

- Phase 5에서 새 Manager 없이 `ACFVehiclePawn`을 Selection / Lock / Scan의 얇은 Gameplay command facade로 확장했다.
- `RequestLockSelectedTarget()`은 요청 순간의 Selected Actor만 읽어 기존 `VehicleTargetingComp::RequestLock()`에 전달하고, Selection 변경/해제로 Acquire/Lock을 자동 교체·해제하지 않는다. Selection 없음은 기존 `InvalidTarget` 의미로 fail-closed한다.
- `RequestClearTargetLock()`은 Lock만 수동 해제하며 Selection/Scan/Detection을 변경하지 않고 Break revision도 증가시키지 않는다.
- 기존 `RequestStartTargetScan()`은 시작 순간 Selected Actor를 Sensor에 전달하고 Sensor가 Actor + ContactId를 capture하므로 이후 Selection 변경을 추종하지 않는다.
- `RequestCancelTargetScan()` + Sensor `CancelTargetScan()`을 추가해 Scan Attempt만 취소하고 broad Active Detection Pulse를 유지한다. broad `RequestCancelSensorOperations()` 의미는 보존했다.
- 기존 `IA_ActiveScan` 이름/V 매핑은 자산 호환을 위해 유지하고 C++ ToolTip/Migration만 현재 Selected Target Scan 의미로 교정했다. 새 Lock InputAction/key, HUD, Guided Weapon source migration은 추가하지 않았다.
- 구현 재검수 중 stale Active Scan Migration 주석 P2 1건을 교정한 뒤 final Official UE 5.8 Build `8c3d5e3eb4d94be88492d4b55e6c6577` PASS, Targeting `5/5` (`2ef26d0d70cb4f45ba57e1d0008273c7`), TargetSelect `11/11` (`096cf68401564c7d8fe7a9cfd3730bb0`), Sensor `17/17` (`6f5d671908944caa9a4e7f779fa4c9a3`) PASS로 `P0 0 / blocking P1 0 / P2 0`, Phase 5 Technical PASS를 확정했다.

### v1.0.1 - 2026-09-18

- Phase 4 독립 중간검수에서 public `RequestLock()` acceptance coverage 누락을 `blocking P1 1`, 실제 Lock Acquisition Policy Current/후속 경계 불명확을 `P2 1`로 확인하고 HOLD했다.
- Product Runtime 의미 변경 없이 Sensor/Targeting exact test friend와 `TGT_P0_04.RequestLockPublicPath`를 추가해 Live 수락, 동일 대상 중복 no-reset, Locked duplicate, A→B 명시 교체, LastKnown/NoContact/Invalid 거부와 replacement non-break를 실제 public API 경로로 검증했다.
- 실제 `CrosshairHold / TargetCone / LOS / 거리 / 목표 기동` 기반 Lock Acquisition Policy는 Phase 4 Current가 아니라 후속 범위임을 명시했다.
- Official UE 5.8 Build `767de4c7b3a244a98631d1ac0bf5c873` PASS, Targeting `4/4`, TargetSelect `11/11`, Sensor `17/17` PASS 후 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 4 Technical PASS Restored`로 닫았다.
- Automation runner의 임시 allowlist 확장은 검증 후 원문 복원했고 final diff exact0을 확인했다.

### v1.0.0 - 2026-09-18

- Phase 4 Vehicle Target Lock Runtime을 신규 Current System으로 등록했다.
- `Idle / Acquiring / Locked` exact3 상태, Live Contact-only RequestLock, LastKnown acquire/quality decay, Live quality recovery, Lost/Destroyed/SensorUnavailable/QualityDepleted Break와 exact-once Break revision 계약을 기록했다.
- Sensor tuning과 분리된 `FCFTargetingConfig` fallback, actor-free `FCFTargetingSnapshot`, Pawn-owned `UCFVehicleTargetingComp` ownership을 Current로 승격했다.
- Lost 뒤 같은 Entity의 새 ContactId에 Lock을 자동 reassociate하지 않는 경계와 Knowledge persistence와 Lock lifetime의 분리를 명시했다.
- RequestLock Live-contact validation order blocking P1 교정 후 Official UE 5.8 Build PASS, Targeting 3/3, TargetSelect 11/11, Sensor 17/17 PASS를 Fresh Technical Validation으로 기록했다.
- Input/HUD/Guided Weapon migration과 Phase 3B-2 Dynamic Knowledge/Freshness/Rescan은 후속 범위로 보존했다.
