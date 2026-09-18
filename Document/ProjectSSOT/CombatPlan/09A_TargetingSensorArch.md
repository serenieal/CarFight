# 09A. Targeting / Sensor Architecture

- 문서 버전: v0.1.16
- 작성일: 2026-09-17
- 최근 갱신일: 2026-09-18
- 문서 상태: Phase 3B-1 Technical PASS / Phase 4 Vehicle Target Lock Technical PASS / Phase 5 Gameplay Command Technical PASS / Phase 6 HUD Presentation Technical PASS / Phase 7 Guided Weapon Source Technical PASS
- 프로젝트: CarFight
- 상위 기획: `Document/ProjectSSOT/CombatPlan/09_LockSensorEW.md`
- 현재 구현 참고: `Document/Systems/Targeting/SensorContact.md`, `Document/Systems/Targeting/TargetSelect.md`, `Document/Systems/Targeting/VehicleTargeting.md`

---

## 1. 목적

이 문서는 CarFight의 Targeting / Sensor / Scan 계층을 다시 설계한 기준 문서다.

기존 구현은 Sensor Contact, Target Selection, Active Scan, Targeted Scan, Knowledge Progression이 단계적으로 추가되면서 서로 다른 세대의 개념이 `UCFVehicleSensorComp`와 HUD 계약 안에 함께 누적되었다. 특히 Target Scan이 `bActiveScanRunning`을 켜면서 광역 Active Detection까지 함께 시작하는 결합, Contact 내부 Legacy Analysis Progress와 Scan Attempt Progress의 이중 노출, 장비별 Lock ownership과 공용 Sensor 상태의 역할 혼재가 확인되었다. 2026-09-17 Phase 1에서는 **Active Detection Pulse와 Target Scan의 Runtime state 공유를 제거**했고, Phase 2에서는 **Contact/HUD/Radar legacy `AnalysisProgress01` 경로를 제거하고 Sensor/Pawn command naming을 역할 기준 canonical API로 정리**했다. 2026-09-18 Phase 3에서는 **ContactId·표시 TargetId와 독립된 `TargetEntityId(FGuid)` Foundation과 Vehicle/Test Target concrete provider, Sensor Contact 최초 캡처·수명 내 불변 보존**을 구현하고 Fresh Technical PASS를 확보했다. 이어 Phase 3B-1에서 valid `TargetEntityId` exact key의 actor-free Persistent Knowledge Store, Identified/DetailedScan 즉시 upsert, Contact Removed 뒤 same-Entity reassociation, exact `AnalysisCompletionRevision` 복원과 authoritative Terminal Record를 구현하고 Fresh Technical PASS를 확보했다. Dynamic Knowledge Domain/Freshness/Rescan은 Phase 3B-2 후속 범위로 남아 있다.

이번 재설계의 목적은 기능을 삭제하거나 Sensor를 처음부터 다시 만드는 것이 아니다. 현재 유효한 Contact lifecycle, actor-free Snapshot, Knowledge progression, Target Selection, Radar 표시 기반을 보존하면서 **상태와 책임의 경계를 다시 정의하고, 이후 Lock / Scan / EW / Guided Weapon 확장이 서로를 침범하지 않도록 구조를 재정렬**하는 것이다.

이 문서는 전체 Targeting/Sensor 재설계의 설계 기준이며 전체 구현 완료를 의미하지 않는다. Phase 1~3A의 Operation Split, Legacy Progress/API naming, TargetEntityId Foundation, Phase 3B-1 Persistent Knowledge Store/Reassociation/Terminal Record, Phase 4 Vehicle Target Lock Runtime, Phase 5 Selection/Lock/Scan Gameplay Command, Phase 6 HUD Presentation과 Phase 7 Guided Weapon Locked Target source migration은 실제 Source·Production UMG·Systems에 반영되고 Fresh Technical PASS까지 완료됐다. Dynamic Knowledge Domain/Freshness/Rescan은 실제 요구가 생길 때의 Phase 3B-2 후속 범위다.

---

## 2. 설계 원칙

CarFight는 EVE Online의 정보전 구조에서 역할 분리 원칙을 참고하되, 자동차 액션 게임의 직접 조준성과 빠른 전투 흐름을 유지한다.

현재 P0 / P1 CombatPlan의 기준 전투 범위는 **싱글플레이 AI 교전**이다. 이 문서의 P0 Targeting Runtime은 멀티플레이 Replication(복제), 서버 Authority, 팀 공유 Lock을 요구하지 않는다. 네트워크 확장이 실제 범위로 승격되기 전에는 해당 복잡도를 선행 구현하지 않는다.

핵심 불변식은 다음과 같다.

```text
Selected != Contact
Contact != Locked
Locked != Scanning
Scanning != Knowledge
Knowledge != Contact Lifetime
```

하나의 상태 전이가 다른 상태를 암묵적으로 함께 켜거나 완료시키지 않는다.

예를 들어 Target Scan을 시작했다고 해서 Active Detection Pulse가 자동으로 켜지지 않으며, Selection을 바꿨다고 기존 Target Lock이 자동으로 교체되지 않는다. 특정 장비가 여러 기능을 동시에 제공해야 한다면 상위 Gameplay Policy가 독립 Operation을 명시적으로 조합한다.

---

## 3. 6축 책임 모델

Targeting / Sensor / Scan 시스템은 다음 6개 축으로 구분한다.

| 축 | Authority | 역할 | 소유하지 않는 것 |
|---|---|---|---|
| Detection | Sensor Runtime | 월드 대상을 감지하고 Contact 갱신 입력을 제공 | Selection, Target Lock, Target Scan progress |
| Contact | Sensor Runtime | 현재 관측·기억 중인 대상의 추적 상태 | 플레이어 의도, 장비 Lock 상태, 영구 Knowledge |
| Selection | TargetSelect Runtime | 플레이어가 현재 보고 조작하려는 대상 | Detection, Lock, Scan, Knowledge 승격 |
| Target Lock | Vehicle Targeting Runtime | 특정 Contact를 전투 추적 대상으로 확보·유지 | 자동 명중, Scan Knowledge |
| Target Scan | Sensor / Scan Runtime | 특정 Contact 하나의 추가 정보를 분석 | 광역 Detection, Target Lock 생성 |
| Knowledge | Knowledge Runtime / Sensor-owned P0 storage | 이미 획득한 대상 정보 | 현재 Contact lifetime, Scan attempt progress |

이 6축은 서로 영향을 줄 수 있지만 서로의 상태를 대신하지 않는다.

### 3.1 대상 식별자 계층

Contact와 Knowledge를 안전하게 분리하려면 대상 식별자도 역할별로 분리한다.

```text
ContactId
= 현재 Sensor Runtime이 발급한 추적 기록 ID
= Live / LastKnown 재획득 계약에 사용
= 영구 Knowledge 귀속 ID가 아님

TargetEntityId
= 월드에 존재하는 실제 대상 한 개체를 구분하는 안정 Identity
= Persistent Knowledge 재연결 기준
= 서로 다른 실제 차량은 같은 차량 Data를 사용해도 서로 다른 EntityId를 가져야 함
= P0 최소 수명은 현재 전투/World Session의 해당 Gameplay Entity lifetime이며, Save/Load 영속성은 P0 요구가 아님

TargetTypeId / ArchetypeId
= 차량 종류 / DataAsset / Archetype 식별자
= 예: TestSUV 타입이라는 사실
= 실제 개체 한 대를 구분하는 Identity가 아님
```

현재 차량의 `TargetDisplayInfo.TargetId`는 VehicleData의 PrimaryAssetId 기반이므로 동일 VehicleData를 사용하는 여러 차량에서 같을 수 있다. 따라서 현재 `TargetId`를 `TargetEntityId`로 재사용하지 않는다.

Phase 3 Current 구현에서 `TargetEntityId`는 `FGuid` 기반 최소 Identity 계약으로 확정됐다. `ACFVehiclePawn`과 C++ 테스트 Target은 같은 Gameplay Entity lifetime 동안 안정적인 per-instance ID를 제공하고 Sensor Contact는 최초 생성 시 이를 캡처한다. 최소 계약은 **같은 Gameplay Entity lifetime 동안 안정적이고, 동시에 존재하는 서로 다른 개체 사이에서 유일**해야 한다는 것이다. Save/Load를 넘어 동일 ID를 보존하는 영속성은 현재 P0 범위가 아니다. UObject Actor 이름/GetFName/GetName을 안정 Identity fallback으로 사용하지 않는다. Identity 미지원 대상은 Invalid Guid를 정상 호환 상태로 유지하며 Persistent Knowledge를 새 Contact에 추측으로 재연결하지 않고 fail-closed한다.

---

## 4. 전체 흐름

### 4.1 Detection → Contact

```text
World Actor
   ↓ Sensor Detection
Sensor Observation
   ↓
Sensor Contact
   ├─ Live
   ├─ LastKnown
   ├─ Lost
   └─ DestroyedHold
```

Sensor는 대상이 현재 탐지 가능한지 판정하고 Contact를 생성·갱신한다.

Contact는 플레이어 Selection이나 Target Lock을 의미하지 않는다. Radar는 월드 Actor 목록을 직접 그리는 것이 아니라 Sensor Contact를 표시한다.

### 4.2 Selection

```text
Player Input
   ↓
Target Selection
```

Selection은 플레이어의 UI/명령 의도다.

Selection만으로 다음 동작은 자동 발생하지 않는다.

```text
Active Detection 시작 X
Target Lock 시작 X
Target Scan 시작 X
Knowledge 승격 X
```

### 4.3 Target Lock

```text
Selected Actor
+ 유효 Sensor Contact
+ Lock Command
        ↓
Vehicle Targeting Runtime
        ↓
Idle → Acquiring → Locked
```

Target Lock은 차량 공용 Targeting Runtime이 소유한다.

P0는 단일 Locked Target으로 시작한다. Selection과 Lock은 독립이므로 다음 상태가 가능하다.

```text
Selected = Target B
Locked   = Target A
```

플레이어가 다른 대상을 선택해 정보를 확인해도 기존 Lock은 자동으로 바뀌지 않는다.

P0 단일 Lock에서 이미 `Locked(A)` 또는 `Acquiring(A)`인 동안 `RequestLock(B)`를 명시적으로 호출하면 기존 A Lock/Acquire를 종료하고 B를 새 `Acquiring` 대상으로 교체한다. 같은 대상에 대한 중복 요청은 상태를 재시작하지 않고 `AlreadyLocked` 또는 `AlreadyAcquiring` 의미로 fail-visible/no-op 처리한다. `ClearLock()`은 명시적으로 `Idle`로 복귀한다.

### 4.4 Target Scan

```text
Selected Target
+ 유효 Sensor Contact
+ Scan Command
        ↓
Target Scan Attempt
        ↓
Progress
        ↓
Knowledge Update
```

기본 Target Scan은 Target Lock을 요구하지 않는다.

따라서 다음 상태가 가능하다.

```text
Selected = Target B
Locked   = Target A
Scanning = Target B
```

Lock은 전투 추적 대상이고 Scan은 정보 획득 대상이기 때문에 둘을 같은 상태로 취급하지 않는다.

Target Scan은 시작 순간 대상 `ContactId`를 Attempt에 Capture한다. Scan 도중 Selection이 다른 대상으로 바뀌어도 진행 중 Attempt의 대상은 자동 변경되거나 취소되지 않는다. 다른 Target Scan 요청은 P0 단일 Attempt 계약에서 `AlreadyScanning`으로 거부하며, Explicit Cancel / Contact Lost / Destroyed / Runtime invalidation만 Attempt 종료 원인이 된다.

---

## 5. Active Detection Pulse와 Target Scan 분리

### 5.1 Active Detection Pulse

목적은 주변 대상 탐지 능력을 강화하거나 Active Detection Range를 일정 시간 활성화하는 것이다.

```text
RequestActiveDetectionPulse()
        ↓
Active Detection Running
        ↓
주변 Contact 발견 / 갱신
```

Active Detection Pulse는 Target Knowledge를 직접 상승시키지 않는다.

### 5.2 Target Scan / Analysis

목적은 특정 Contact 하나의 Knowledge를 추가 획득하는 것이다.

```text
RequestTargetScan(TargetContact)
        ↓
Scan Attempt
        ↓
Knowledge Gain
```

Target Scan은 광역 Active Detection을 자동 시작하지 않는다.

### 5.3 복합 Scanner 장비

특정 Scanner 장비가 두 기능을 모두 제공하는 것은 허용한다. 다만 내부 구현은 독립 Operation을 명시적으로 조합한다.

```text
Scanner Capability
→ RequestActiveDetectionPulse()
+ RequestTargetScan(SelectedContact)
```

2026-09-17 Phase 1 구현부터 Target Scan은 broad Active Detection state를 켜지 않는다. Phase 2 canonical API는 `StartActiveDetectionPulse() / StartTargetScan() / CancelSensorOperations()`이며 `bActiveScanRunning / ActiveScanRemainingSeconds`는 broad Active Detection Pulse의 내부 legacy-named state로만 남는다. Target Scan은 독립 Attempt state와 duration을 사용한다. 기존 `StartActiveScan() / StartTargetedScan() / StopActiveScan()`과 구형 상태 Getter는 저장된 Blueprint·기존 C++ caller 보호용 Deprecated compatibility wrapper로 유지한다.

### 5.4 동시 실행 계약

Active Detection Pulse와 Target Scan은 서로 다른 Operation이므로 **원칙적으로 동시에 실행 가능**하다.

```text
DetectionOperation = Running
ScanOperation      = Scanning(ContactA)
```

동시 실행 금지는 상태 결합으로 표현하지 않는다. 특정 Scanner가 전력·열·장비 제한 때문에 둘을 동시에 수행할 수 없다면 그것은 별도 Scanner Resource Policy가 판정한다. P0 기본 Runtime 계약은 두 Operation의 독립 실행을 허용한다.

---

## 6. Vehicle Target Lock 설계

### 6.1 Authority

```text
Target Lock Authority
= Vehicle Targeting Runtime

Target Lock Consumers
= Guided Weapon
= EW
= Targeting Assist
= 일부 Scanner / Special Equipment
```

개별 무기나 장비는 공용 Lock을 소유하지 않는다.

장비는 현재 Vehicle Target Lock을 사용할 수 있는지 자신의 Requirement만 판정한다.

### 6.2 P0 Lock State Machine

P0의 지속 상태는 세 개만 둔다.

```text
Idle
  ↓ RequestLock(Contact)
Acquiring
  ↓ Progress == 1.0
Locked
  ↓ 유지 한계 붕괴 / ClearLock
Idle
```

`Broken`은 지속 State가 아니라 **전이 Event**다. Lock이 끊기면 Runtime은 즉시 Idle로 복귀하고 `BreakTransitionRevision`과 `LastBreakReason` 같은 Snapshot event 정보로 HUD/테스트가 exact-once 전이를 관측한다. 한 프레임짜리 Broken State를 만들지 않는다.

### 6.3 Lock Progress와 Lock Quality

`LockProgress`는 아직 Lock을 획득하는 동안의 진행도다.

`LockQuality`는 이미 Locked 상태인 대상을 얼마나 안정적으로 추적 중인지 나타낸다.

```text
Acquiring 70%
→ 엄폐
→ LockProgress 감소 또는 정지

Locked 100% Quality
→ 상대 급기동 / 연막 / 재밍
→ LockQuality 감소
→ 회복 가능 또는 Break Event 후 Idle
```

### 6.4 Lock Requirement

P0에서는 실제 사용 의미가 분명한 두 정책만 둔다.

```text
NoLockRequired
VehicleLockRequired
```

예시:

| 장비 | Requirement |
|---|---|
| 기관총 | NoLockRequired |
| 캐논 | NoLockRequired |
| 무유도 로켓 | NoLockRequired |
| Target Homing Missile | VehicleLockRequired |
| Target 기반 전자전 장비 | VehicleLockRequired |

Direct Fire 무기는 Target Lock 없이 직접 조준·발사할 수 있어야 한다.

`VehicleLockPreferred`는 Lock이 있을 때 무엇이 달라지는지 실제 Consumer 계약이 생기기 전에는 정의하지 않는다. Dumb-fire fallback, 조준 보정 같은 구체적 무기가 생기면 그 기능의 별도 정책으로 추가한다.

### 6.5 Lock Acquisition Mode

Target Lock 획득 방식은 개별 무기가 아니라 Vehicle Targeting Profile / Runtime Policy가 소유한다.

```text
CrosshairHold
TargetCone
SensorTrack
ManualDesignate
```

`LaserGuide`처럼 공용 Vehicle Lock과 성격이 다른 지정 방식은 별도 Guidance / Designation Capability로 둔다.

### 6.6 Contact 상태와 Lock 유지 규칙

P0 기본 규칙은 다음과 같다.

```text
Request Lock 시작
→ Live Contact만 허용

Acquiring 중 Contact가 Live가 아님
→ LockProgress 감소 또는 획득 중단
→ Lost / Destroyed 확정 시 Idle

Locked 중 Live 상실
→ 즉시 자동 성공 유지가 아니라 LockQuality 감소
→ LastKnown 동안 조건부 회복 가능
→ Lost 확정 시 Break Event 후 Idle

DestroyedHold 진입
→ 기존 Lock 즉시 Break Event 후 Idle
```

Contact 재획득은 같은 Contact lifecycle 계약 범위에서 LockQuality 회복 입력이 될 수 있지만, Lost 이후 새 Contact를 과거 Lock에 자동 연결하지 않는다.

### 6.7 Targeting Snapshot 계약

`UCFVehicleTargetingComp`의 Actor/runtime truth를 HUD와 일반 Consumer가 직접 읽지 않도록 actor-free 공개 Snapshot을 둔다.

P0 최소 공개 의미:

```text
State                 = Idle / Acquiring / Locked
TargetContactId       = 현재 Acquire/Lock 대상 ContactId
LockProgress01        = Acquiring 진행률
LockQuality01         = Locked 추적 품질
BreakTransitionRevision
LastBreakReason
```

Runtime 내부는 필요하면 `TWeakObjectPtr<AActor>`를 보유할 수 있지만, HUD는 해당 Actor를 직접 조회해 Lock truth를 만들지 않는다. Sensor와 동일하게 외부 Presentation은 Snapshot과 Contact bridge를 소비한다.

### 6.8 Guided Weapon 인계 계약

P0 Target Homing Missile의 Vehicle Lock과 발사 후 Seeker 책임을 분리한다.

```text
발사 전
VehicleLockRequired 검증
        ↓
Vehicle Targeting Runtime의 Locked Target 확인
        ↓
발사 승인 순간 Locked Target Actor/필요 Target Context Snapshot
        ↓
ProjectileLaunchContext
        ↓
Missile Seeker / Guide Runtime가 발사 후 Target을 독립 소유
```

발사 후 플레이어 Selection 변경, Vehicle Lock 대상 변경 또는 Vehicle Lock 해제는 이미 발사 승인된 P0 Missile의 목표를 자동 변경하지 않는다. 현재 Missile의 발사 순간 `GuidanceTargetActor` Snapshot 구조는 이 원칙에 맞춰 **Source만 Selected Target에서 Locked Target으로 Migration**한다.

지속적인 발사 플랫폼 조명이 필요한 Semi-Active Guidance 등은 P0 공용 계약에 포함하지 않고 별도 Guidance Policy로 확장한다.

### 6.9 Phase 4 Targeting Config 소유권

Phase 4의 `LockProgress01`과 `LockQuality01` 변화율은 Sensor의 Analysis/Contact tuning을 재사용하지 않는다. Sensor는 Contact 관측 상태만 제공하고 Vehicle Targeting Runtime이 Lock 정책을 독립 소유한다.

P0에서는 별도 Targeting DataAsset/Profile 체계를 선행 구현하지 않고 `UCFVehicleTargetingComp`가 작은 `FCFTargetingConfig` fallback 값을 직접 소유한다.

```text
LockAcquireGainPerSec
= Live Contact에서 Acquiring 진행률 증가율

LockAcquireDecayPerSec
= Acquiring 중 Contact가 LastKnown일 때 진행률 감소율

LockQualityRecoveryPerSec
= Locked 대상이 Live로 회복됐을 때 Quality 회복률

LockQualityDecayPerSec
= Locked 대상이 LastKnown일 때 Quality 감소율
```

초기 P0 fallback은 최종 밸런스가 아니라 동작 가능한 Foundation 값이다. 실제 차량/센서/장비별 Targeting Profile 요구가 생기면 이 독립 Config를 Source로 승격하되 Sensor `AnalysisGainPerSec / AnalysisDecayPerSec`에 합치지 않는다.

Phase 4 구현 범위는 `CFTargetingTypes` + `UCFVehicleTargetingComp` + actor-free `FCFTargetingSnapshot` + `ACFVehiclePawn` 기본 서브오브젝트 연결 + focused Automation으로 제한한다. HUD Presentation, Input command wiring, Guided Weapon의 Selected Target → Locked Target source migration은 각각 Phase 5~7 후속으로 남긴다.

2026-09-18 Phase 4 Pre-Implementation Review 결과:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 4 Implementation Ready
```

---

## 7. Knowledge / Persistent Knowledge Store 설계

Contact lifetime과 Knowledge lifetime은 분리한다.

```text
Contact
= 현재 센서가 관측·기억 중인 추적 정보

Persistent Knowledge Store
= 같은 TargetEntityId에 대해 Contact lifetime 밖에서도 유지하는 획득 정보 Authority
```

Contact가 `Lost → Removed`로 사라져도 이미 획득한 모든 정보가 즉시 소멸하지 않는다. 다만 Contact를 다시 만들었다는 사실만으로 과거 정보를 복사하지 않는다. **동일한 유효 `TargetEntityId`가 안정적으로 증명될 때만** 기존 Knowledge Record를 새 Contact에 재연결한다. `ContactId`, `KnownTargetId`, VehicleData 종류가 같다는 이유만으로 서로 다른 실제 개체의 Knowledge를 합치지 않는다.

여기서 `Persistent`는 SaveGame 영속성을 뜻하지 않는다. P0 의미는 **현재 World / Gameplay Entity lifetime 안에서 Contact가 제거·재생성되어도 유지되는 지식**이다. Store 안에는 영구 사실뿐 아니라 마지막으로 관측했던 Dynamic Observation의 역사값도 들어갈 수 있다. 즉 `Persistent Knowledge Store`라는 이름의 `Persistent`는 **Store의 Contact-lifetime 독립성**을 뜻하며 모든 저장 필드가 영원히 Fresh하다는 뜻이 아니다.

### 7.1 Store Authority와 P0 물리 소유권

P0 Persistent Knowledge Store의 논리 Key는 `TargetEntityId` exact1이다.

```text
TargetEntityId(valid)
        ↓
Persistent Knowledge Record
```

Invalid Guid에는 Record를 만들거나 조회하지 않는다. 다음 fallback은 금지한다.

```text
ContactId fallback X
KnownTargetId fallback X
VehicleData / TargetTypeId fallback X
Actor GetName / GetFName fallback X
포인터 주소 fallback X
```

1인 개발 유지보수 비용을 고려해 P0에서는 새 Knowledge Component / World Subsystem을 선행 추가하지 않는다. 우선 `UCFVehicleSensorComp` 내부의 명확히 분리된 Knowledge Storage 책임으로 시작한다.

Persistent Knowledge Record는 Contact/Actor lifetime보다 오래 살아야 하므로 **actor-free / UObject-free value record**로 고정한다. `AActor*`, `TWeakObjectPtr<AActor>`, Component/UObject pointer, raw object reference를 Store Authority에 넣지 않는다. 현재 Target과의 Actor 연결은 기존 private Runtime Contact가 소유하고 Store는 `TargetEntityId`, stable metadata, Knowledge 값과 revision만 보존한다.

P0 Store lifetime과 실제 Runtime API 경계는 다음 exact 계약으로 고정한다.

```text
Contact Removed                              → Record 유지
ApplySensorData / ApplyVehicleBaseSensorData → Record 유지
Scanner 장착/제거 / SensorData hot reapply  → Record 유지
Sensor 일시 비활성                          → Record 유지
ResetSensorRuntime                           → Hard Reset / Record 전체 제거
InitializeSensorRuntime 재호출               → Hard Reinitialize / 기존 Store 전체 제거 후 새 Runtime 시작
EndPlay / Owner teardown / World session 종료 → Record 전체 제거
SaveGame / 다음 플레이 세션                 → 보존 보장 안 함
다른 차량 Sensor Runtime                     → 자동 공유 안 함
```

`InitializeSensorRuntime()`은 Current Source에서 Contact/Operation 상태를 초기화하는 파괴적 재초기화 경로이므로 Phase 3B-1에서도 Knowledge만 예외적으로 살려두지 않는다. 장비 교체나 Scanner hot reapply는 반드시 `ApplySensorData()` / `ApplyVehicleBaseSensorData()` 경로를 사용해 Store를 보존한다.

플레이어가 차량을 바꿔도 지식을 계속 가져가야 하는 요구가 실제 게임 규칙으로 승격되면 그때 Knowledge owner를 Player/Combat Session 계층으로 올린다. 그 요구가 없는 P0에서 Global Knowledge Subsystem을 먼저 만들지 않는다.

Store 자체는 Contact, Radar Blip, Selection candidate를 생성하지 않는다. **Knowledge가 남아 있다는 이유만으로 월드에 없는 대상을 Radar나 TargetSelect에 유령처럼 다시 표시하지 않는다.**

Knowledge Store는 Contact가 제거될 때 한꺼번에 복사하는 archive가 아니다. **Knowledge를 획득하거나 갱신하는 순간 Store도 같은 `TargetEntityId` Record를 즉시 갱신**하는 것을 기본 계약으로 한다. Contact Removed는 Store commit 시점이 아니라 Contact Projection의 수명이 끝나는 사건일 뿐이다. 따라서 DetailedScan 획득 직후 Contact가 비정상적으로 사라져도 이미 획득한 Knowledge가 제거 경로의 성공 여부에 의존하지 않는다.

### 7.2 Knowledge 분류 계약

P0 설계에서는 정보를 다음 네 종류로 구분한다.

| 분류 | 예시 | Contact Removed 후 | 동일 Entity 재획득 시 |
|---|---|---|---|
| Persistent Fact | 안정 Target/Type Identity, 식별된 Vehicle 종류, 안정 TargetCategory, 획득한 최고 Knowledge Tier | 유지 | 즉시 복원 가능 |
| Dynamic Observation | 현재 HP, 현재 방어 상태, 현재 장착/작동 상태, 모듈 손상, Ammo/Heat/Charge, 현재 Relation, 임시 EW/Stealth 상태 | 값+관측시각 보존, `Stale` | 오래된 값으로만 제시, 자동 Fresh 금지 |
| Contact / Tracking State | ContactId, ContactState, LastKnown 위치, 현재 위치/속도, Contact Freshness | Knowledge Store에 저장하지 않음 | 새 Contact가 새로 소유 |
| Operation / Event State | Scan Progress, Scan TargetContactId, Scan CompletionTransitionRevision, Lock Progress/Quality | 저장하지 않음 | 새 Operation에서 새로 시작 |

`bDestroyedConfirmed`는 일반 Dynamic Observation과 다르게 Entity lifetime의 **Terminal Knowledge**로 취급한다. §7.8에서 별도 규칙을 둔다.

Persistent Fact는 “이 개체에 대해 이미 배워서 다시 잊을 필요가 없는 사실”만 포함한다. 장착 장비처럼 게임 중 교체될 수 있는 값은 설령 DetailedScan으로 알아냈더라도 Persistent Fact가 아니라 Dynamic Observation이다.

### 7.3 P0에서 실제 복원할 Knowledge

현재 `FCFSensorContact`와 호환되는 Phase 3B-1 최소 Store Record / 복원 대상은 다음 의미를 가진다.

```text
TargetEntityId                    [Store exact key]
StableTargetId                    [private Source TargetId / TargetType identity]
StableTargetCategory              [해당 Entity lifetime에서 안정 분류]
KnownTargetId                     [획득된 경우의 public projection]
KnownDisplayName                  [표시 복원값, consistency key 아님]
HighestKnowledgeTier              = Detected / Identified / DetailedScan 중 획득 최고 단계
DetailedScanAnalysisRevision      [DetailedScan이면 기존 양수 AnalysisCompletionRevision exact 보존]
TerminalDestroyed                 [authoritative destruction 확인 시 true]
```

`DetailedScanAnalysisRevision`은 단순 bool 이력이 아니다. `FCFSensorContact::IsPublicContractValid()`의 기존 `DetailedScan ↔ AnalysisCompletionRevision > 0` 계약을 만족하기 위해 **DetailedScan 최초 획득 당시의 양수 `AnalysisCompletionRevision` 값을 exact 저장하고 exact 복원**한다. Reassociation에서 새 revision을 발급하거나 `NextAnalysisCompletionRevision` allocator를 증가시키지 않는다.

Phase 3B-1 Record 생성은 필요한 최소 범위로 제한한다.

```text
valid TargetEntityId + Identified 승격
→ Record upsert

valid TargetEntityId + DetailedScan 승격
→ Record upsert + DetailedScanAnalysisRevision exact 저장

valid TargetEntityId + authoritative Destroyed
→ Knowledge 단계와 무관하게 최소 Terminal Record upsert

Detected-only + non-terminal
→ 새 Persistent Record 생성을 요구하지 않음
```

이미 같은 Entity Record가 존재하는 상태에서 새 Contact가 Detected로 재등장한 경우에는 Store lookup/reassociation을 수행한다. 즉 Detected-only 대상을 전부 장기 목록으로 축적하지 않으면서도 이미 획득한 Knowledge는 다시 연결할 수 있다.

`Detected`는 본질적으로 현재 Contact 존재를 나타내므로 Store 단독으로 Radar/Contact를 만들지 않는다. 그러나 과거에 `Identified` 또는 `DetailedScan`까지 획득한 동일 Entity가 새 Contact로 재등장하면 해당 Knowledge Tier를 새 Contact Projection에 다시 적용할 수 있다.

현재 `Relation`은 향후 진영/상태 변화 가능성이 있으므로 Persistent Fact로 고정하지 않는다. 새 Contact의 현재 관측이 Relation을 제공하면 그 Fresh 관측을 사용하고, 과거 Relation은 필요할 경우 Dynamic Observation의 `Stale` 정보로만 남긴다. 향후 절대 변하지 않는 `FactionId`가 별도 계약으로 생기면 그것은 Persistent Fact로 저장할 수 있다.

### 7.4 Dynamic Knowledge Freshness

Dynamic Knowledge는 값을 버리는 대신 **값 + 마지막 유효 관측 시각 + Freshness 상태**로 관리한다.

P0 Freshness 의미는 다음 exact3으로 제한한다.

```text
Unknown
= 이 Domain 값을 획득한 적 없음

Fresh
= 현재 신뢰 가능한 관측/분석으로 갱신됨

Stale
= 과거 값은 알고 있지만 현재 값이라고 보장할 수 없음
```

각 Dynamic Domain은 최소 다음 의미를 가진다.

```text
Value
LastUpdatedWorldTimeSeconds
Freshness = Unknown / Fresh / Stale
```

이 `Freshness`는 현재 `FCFSensorContact.FreshnessSeconds`와 다른 개념이다.

```text
Contact.FreshnessSeconds
= 마지막 Sensor 관측 이후 경과 시간
= Contact tracking/lifetime 정보

Dynamic Knowledge Freshness
= 특정 Knowledge Domain 값이 현재 사실로 신뢰 가능한지 여부
= Knowledge 정보 품질
```

둘을 같은 필드나 같은 만료 로직으로 합치지 않는다.

하나의 전역 `LastUpdatedTime`으로 모든 Dynamic 정보가 동시에 최신인 것처럼 만들지 않는다. HP와 Loadout처럼 갱신 원인이 다를 수 있으므로 **Freshness는 최소 Domain 단위**로 소유한다. 초기 구현에서 Dynamic Domain이 하나뿐이면 한 Record로 시작할 수 있지만 향후 부분 갱신을 막는 전역 timestamp 계약으로 고정하지 않는다.

Dynamic Freshness는 장기 설계 계약으로 유지하되 **Phase 3B-1 구현 범위에는 실제 Dynamic Domain 저장/갱신을 포함하지 않는다.** 현재 Source에는 HP / Loadout / Defense 같은 Sensor Dynamic Knowledge source가 아직 없으며, 범용 Domain framework를 선행 구현하지 않는다.

향후 첫 실제 Dynamic Knowledge 요구가 생기는 Phase 3B-2에서 다음 보수적 정책을 적용한다.

```text
Fresh Dynamic Observation
  ↓ Contact가 Live를 상실
Stale

Contact Removed
→ 모든 current-observation 기반 Dynamic Domain은 최소 Stale

동일 Entity 재획득
→ Stale 값은 Stale 그대로 복원
→ 재획득 자체만으로 Fresh가 되지 않음

새 유효 관측 / 명시 Dynamic Rescan 결과
→ 해당 Domain만 Fresh로 갱신
→ LastUpdatedWorldTimeSeconds 갱신
```

Current `StartTargetScan()`은 이미 `DetailedScan` Contact 재스캔을 거부하는 계약이다. Phase 3B-1은 이 동작을 변경하지 않는다. Dynamic 정보 갱신이 실제로 필요해지면 **기존 Initial Target Scan과 별개의 Rescan / Refresh Analysis 계약**을 먼저 설계한 뒤 Phase 3B-2에서 추가한다.

향후 특정 Domain에 “10초까지 Fresh” 같은 TTL이 필요하면 Domain policy로 추가한다. P0에서는 TTL DataAsset이나 복잡한 decay framework를 선행 도입하지 않고 **Live 상실 시 stale**를 안전 기준으로 사용한다.

Stale 값은 UI에서 `마지막 확인`, `오래된 정보`처럼 표현할 수 있지만 다음 Authority로 사용하지 않는다.

```text
발사 허용 판정 X
Target Lock 성공 판정 X
현재 HP 기반 자동 전술 판정 X
현재 장비 사용 가능 판정 X
```

### 7.5 Contact Removed → 동일 Entity 재획득 Reassociation

동일 Entity 재획득 순서는 Phase 3B-1에서 다음으로 고정한다. **Store conflict / Terminal 검사는 새 Contact를 public/runtime list에 최종 admit하기 전에 수행**한다.

```text
1. Candidate의 TargetEntityId + Source identity metadata 관측
2. TargetEntityId가 valid면 Knowledge Store exact lookup
3. 기존 Record가 있으면 stable identity consistency 검증
4. TerminalDestroyed 여부 확인
5. conflict / terminal이면 candidate admission과 Knowledge merge를 fail-closed
6. 통과하면 새 Sensor Contact 생성/확정
7. Persistent Fact + HighestKnowledgeTier 복원
8. DetailedScan이면 기존 DetailedScanAnalysisRevision exact 복원
9. Contact-specific tracking state는 새 Contact 값 유지
10. ScanAttempt / completion transition은 복원하지 않음
```

`fail-closed`는 기존 terminal/conflicting Record를 덮어쓰거나 새 Live Contact로 정상 수용하지 않는다는 뜻이다. 단순 DisplayName/Relation 변화는 이 경로를 차단하지 않는다.

Phase 3B-1의 stable identity consistency key는 exact하게 다음만 사용한다.

```text
Primary key        = TargetEntityId exact match
Stable identity    = private Source TargetId / TargetTypeId
Stable category    = TargetCategory
```

단, Current `FCFTargetDisplayInfo`는 `TargetId=None`, `TargetCategory=Unknown`을 허용하므로 미확정 값은 충돌 근거가 아니다.

```text
Stored TargetId=None + Observed valid TargetId
→ 같은 Entity Record에 stable identity 최초 채움 허용

Stored TargetCategory=Unknown + Observed concrete category
→ 같은 Entity Record에 stable category 최초 채움 허용

Stored/Observed TargetId 둘 다 valid이고 서로 다름
→ identity conflict / fail-closed

Stored/Observed TargetCategory 둘 다 concrete이고 서로 다름
→ identity conflict / fail-closed
```

한 번 concrete stable value가 확정된 뒤에는 자동 변경하지 않는다. `KnownDisplayName`, `Relation`, ContactId, LastKnown 위치는 consistency key가 아니다. DisplayName은 localization/UI 명칭 변경 가능성이 있고 Relation은 동적으로 바뀔 수 있으므로 mismatch만으로 Entity identity conflict를 발생시키지 않는다.

Phase 3B-2가 실제 Dynamic Domain을 추가한 뒤에만 stale historical value 연결과 domain별 Fresh 갱신 단계를 이 순서 뒤에 확장한다.

다음 예시는 **Phase 3B-2 Dynamic Domain까지 확장된 뒤의 장기 결과 형태**를 설명한다. Phase 3B-1 실제 구현에서는 HP/Stale 필드를 만들지 않고 Persistent Fact 복원 부분만 적용한다.

```text
과거:
Contact_A
Entity = E1
DetailedScan 획득
HP = 43% @ t=100 (Fresh)

Contact_A Removed

나중:
Contact_B
Entity = E1

복원:
KnowledgeTier = DetailedScan
Known Identity = 유지
HP = 43% / Stale / LastUpdated=t=100
ScanAttempt.Progress = 0
ContactId = Contact_B
```

즉 **“상세 스캔한 적이 있는 대상”이라는 사실은 기억하지만, 당시 HP가 지금도 43%라고 주장하지 않는다.**

### 7.6 Compatibility Projection 규칙

현재 사용 중인 단계형 `InformationLevel`은 제거하지 않는다.

```text
Detected
Identified
DetailedScan
```

Persistent Knowledge Store 구현 뒤에는 이 값을 다음 의미의 호환 Projection으로 본다.

```text
Detected
= 현재 Contact가 있으나 영구 식별 Knowledge가 없음

Identified
= 동일 Entity에 대해 식별 Knowledge를 이미 획득함

DetailedScan
= 동일 Entity에 대해 Detailed Knowledge 획득 이력이 있음
```

따라서 새 Contact가 같은 Entity로 재식별되면 `Identified / DetailedScan`은 즉시 복원될 수 있다. 단, `DetailedScan`은 **“모든 Dynamic 값이 현재 Fresh하다”는 뜻이 아니다.** Dynamic Domain별 Freshness를 별도로 확인해야 한다.

현재 `AnalysisCompletionRevision`은 DetailedScan Knowledge의 durable acquisition history 의미를 유지한다. Reassociation은 기존 획득 이력을 복원할 뿐 새 Scan completion으로 취급하지 않는다. P0 구현에서 이 Revision의 **숫자 자체는 현재 Sensor Runtime 내부의 단조 증가 식별자**이므로 전역 Entity revision으로 승격하지 않는다.

Phase 3B-1에서는 Store Record가 최초 DetailedScan 때 받은 양수 `AnalysisCompletionRevision` 값을 `DetailedScanAnalysisRevision`으로 exact 보존하고 동일 값으로 복원한다.

```text
Reassociation
→ Contact.AnalysisCompletionRevision = Store.DetailedScanAnalysisRevision
→ NextAnalysisCompletionRevision allocator 증가 금지
→ 새 AnalysisCompletionRevision 발급 금지
→ ScanAttempt.CompletionTransitionRevision 증가 금지
→ `스캔 완료` transient HUD feedback 재발행 금지
→ ScanAttempt.Progress01 = 0
```

같은 Runtime 안에서 과거 revision 값을 복원해도 allocator는 이미 그보다 큰 다음 값을 소유해야 한다. Store 복원 코드가 allocator를 역행시키거나 재계산하지 않는다.

### 7.7 Merge / Conflict 정책

같은 `TargetEntityId` Record에 새 정보가 들어올 때는 정보 종류에 따라 다르게 처리한다.

```text
Persistent Fact
→ 이미 확정된 안정 Identity와 같은 값: 유지
→ 더 높은 Knowledge Tier: 단조 증가
→ 같은 EntityId에서 서로 모순되는 안정 Target/Type Identity: 자동 overwrite 금지 / fail-visible

Dynamic Observation
→ 새 유효 관측이 이전 값보다 최신: 새 값 + timestamp로 갱신
→ 관측 상실: 값 유지 + Stale
```

특히 같은 `TargetEntityId`에서 **양쪽 모두 concrete인** private Source `TargetId / TargetTypeId` 또는 안정 `TargetCategory`가 서로 다르면 단순 데이터 갱신으로 보지 않는다. 이는 Entity ID 재사용이나 provider 계약 위반 가능성이 있으므로 **새 Contact admission과 과거 Persistent Knowledge merge를 fail-closed**한다. 반대로 `None → valid TargetId`, `Unknown → concrete TargetCategory`는 최초 안정 metadata 보강으로 허용한다.

반대로 `KnownDisplayName` 또는 `Relation`만 달라진 것은 identity conflict가 아니다. DisplayName은 최신 표시 metadata로 갱신할 수 있고 Relation은 Dynamic Observation 정책이 생긴 뒤 별도로 관리한다.

### 7.8 Destroyed / Terminal Knowledge

대상 파괴가 authoritative하게 확인되고 Contact가 valid `TargetEntityId`를 가지고 있으면 **기존 Knowledge Record 존재 여부와 무관하게 최소 Terminal Record를 upsert**한다.

```text
Destroyed confirmed + valid TargetEntityId
→ Store record 없음: 최소 Record 생성
→ Store record 있음: 기존 Persistent Fact 유지
→ TerminalDestroyed = true
→ Phase 3B-2 Dynamic Observation이 있으면 Stale
```

따라서 `Identified` 이전에 파괴된 대상도 같은 EntityId가 다시 살아서 나타나는 Identity 재사용을 검출할 수 있다. Invalid `TargetEntityId` 대상에는 Terminal Record를 추측 생성하지 않는다.

`DestroyedHold`는 여전히 Contact lifecycle 상태이며 Knowledge Store 상태로 옮기지 않는다. Terminal marker는 “이 Entity lifetime이 파괴로 끝났음”을 의미하는 Knowledge다.

같은 `TargetEntityId`가 Destroyed confirmed 이후 다시 Live Entity로 나타나는 것은 P0 정상 Reassociation이 아니다.

```text
Same TargetEntityId after terminal destruction
→ 자동 부활/Knowledge 재연결 금지
→ Identity provider / lifecycle contract violation으로 fail-visible
```

정상 Respawn은 새 Gameplay Entity이므로 새 `TargetEntityId`를 받아야 한다. 향후 부활/재생 같은 게임 규칙이 실제로 생기면 별도 명시 정책으로 확장한다.

### 7.9 Store와 Contact / Radar / HUD 경계

Persistent Knowledge Store는 현재 월드 관측 목록이 아니다.

```text
Store에 Record 존재
≠ 현재 Contact 존재
≠ Radar Blip 존재
≠ Selection 가능
≠ Target Lock 가능
```

Radar는 계속 `FCFSensorSnapshot.Contacts`만 소비한다. TargetSelect도 Store를 후보 목록처럼 직접 사용하지 않는다.

새 Contact가 같은 Entity로 재연결되면 그 Contact의 Knowledge Projection을 통해 HUD가 Persistent Fact와 Stale Dynamic 정보를 표시한다. 향후 HP/Loadout 같은 Dynamic Domain을 실제 UI에 노출할 때는 Actor 포인터나 raw private Store를 UMG에 직접 노출하지 않고 actor-free Knowledge View/Snapshot을 통해 전달한다.

### 7.10 P0에서 하지 않는 것

Persistent Knowledge Store Foundation에 다음 범위를 선행 포함하지 않는다.

```text
SaveGame 영속 저장
게임 재실행 뒤 Knowledge 복원
멀티플레이 Replication
팀/AI 간 공유 Intel
다른 차량 Sensor Runtime 간 자동 Knowledge 공유
Store Record가 Contact/Radar를 직접 생성하는 기능
Knowledge용 신규 DataAsset 계층
임의 LRU/TTL로 Persistent Fact 삭제
```

현재 전투/World Session 규모에서는 Persistent Fact Record를 owner lifetime 동안 유지한다. 실제 메모리 압력이 확인되기 전에는 획득한 지식을 임의 LRU로 삭제하는 복잡도를 추가하지 않는다.

Phase 3B-1의 물리 구현은 `UCFVehicleSensorComp` private storage로 제한하되 **구체 컨테이너/struct 이름 자체는 설계에서 강제하지 않는다.** 구현 시 `TMap<FGuid, ...KnowledgeRecord...>` 형태가 자연스럽지만, 별도 Global Subsystem / Component / 범용 Domain Registry로 확장하는 것은 실제 요구가 생기기 전 금지한다.

---

## 8. Scan Attempt와 Knowledge 분리

Scan Attempt는 현재 진행 중인 작업 상태다.

```text
Idle
→ Scanning(TargetContactId, Progress)
→ Completed / Cancelled / Expired
→ Idle
```

Scan Attempt Progress는 transient state다.

```text
ScanAttempt.Progress01
```

중단·만료·완료 뒤에는 Attempt Progress가 0으로 돌아가도 이미 획득한 Knowledge는 별도로 보존된다.

P0 단일 Scan Attempt 규칙:

```text
Selection 변경                  → 현재 Attempt 계속
동일 대상 Scan 재요청           → AlreadyScanning / no-op
다른 대상 Scan 요청             → AlreadyScanning / 거부
Explicit Cancel                 → Idle + Progress 0
Contact Lost / Destroyed        → Attempt 종료 + Progress 0
LOS / Range 일시 상실           → 기존 계약처럼 Progress decay 가능
조건 재획득                     → 같은 Attempt에서 Progress 재상승 가능
DetailedScan 완료               → 완료 transition 기록 후 Idle + Progress 0
```

Phase 2부터 `FCFSensorContact.AnalysisProgress01`은 제거됐다. 현재 Scan progress의 public Authority는 `FCFSensorSnapshot::ScanAttempt.Progress01`이며 Target/Radar HUD에도 Contact 기반 progress passthrough를 두지 않는다.

---

## 9. Runtime 책임 배치

논리 책임을 6축으로 나눈다고 해서 Component를 6개로 쪼개지는 않는다.

1인 개발 유지보수 비용을 고려해 P0 물리 구조는 다음 정도로 유지한다.

```text
UCFVehicleSensorComp
├─ Detection
├─ Contact Lifetime
├─ Target Scan
└─ Knowledge P0 Storage / Snapshot

UCFTargetSelectComp
└─ Selection

UCFVehicleTargetingComp   [신규]
└─ Vehicle Target Lock
```

`UCFVehicleSensorComp` 내부에서도 Detection / Contact / Scan / Knowledge 함수와 상태는 명확히 분리하지만, 책임 분리만을 이유로 작은 Component를 무분별하게 추가하지 않는다.

---

## 10. C++ / DataAsset / Blueprint 역할

### 10.1 C++ Authority

C++는 다음을 소유한다.

```text
Sensor Contact lifecycle
Detection evaluation
Target Lock state machine
LockProgress / LockQuality
Target Scan state machine
Knowledge transition
실패 사유
Snapshot
장비 Lock Requirement 판정
자동 테스트
```

### 10.2 Data / Tuning 소유권

P0에서는 Targeting을 위해 새 전용 DataAsset 계층을 먼저 만들지 않는다.

```text
Sensor 관련 수치
→ 기존 VehicleSensorData / Scanner source 계약 유지

Targeting 관련 수치
→ 우선 FCFTargetingConfig 값 구조체
→ VehicleData의 기본 Targeting Config에서 공급

장비 Lock Requirement
→ 실제 Lock 소비 장비 데이터에서 소유
```

`FCFTargetingConfig` P0 후보 값:

```text
Lock Range
Lock Acquire Time
Lock Progress Decay
Lock Quality Decay / Recovery
Targeting Acquisition Mode
```

실제 장비별 Targeting override 요구가 생기기 전에는 `TargetingDataAsset` 같은 새 자산 타입을 선행 도입하지 않는다. 이후 반복 사용·독립 제작 필요성이 확인되면 별도 DataAsset으로 승격할 수 있다.

### 10.3 Blueprint / UMG

Blueprint / UMG는 Presentation과 조립에 집중한다.

```text
Selection 표시
Locking / Locked 표시
Lock 게이지
Scan 진행 표시
Knowledge 공개 UI
실패 피드백
애니메이션 / 시각 연출
```

Blueprint가 Lock/Scan gameplay Authority를 직접 소유하지 않는다.

---

## 11. HUD Presentation 계약

HUD에서는 다음 상태를 서로 다른 Presentation Channel로 본다.

```text
Selection
Target Lock
Target Scan
Knowledge
```

예를 들어 같은 대상에 대해 다음 UI가 동시에 존재할 수 있다.

```text
Selected Marker
Locking Ring / Locked Marker
Scanning Progress
Identified / Detailed Information
```

다른 대상을 선택한 상태에서도 기존 Locked Target 표시가 유지될 수 있다.

Scan 완료 후 일시적인 `스캔 완료` 피드백과 Persistent Knowledge 표시는 별도 lifecycle로 관리한다.

### 11.1 Phase 6 ViewData Source 계약

Phase 6는 Gameplay 상태를 새로 만들지 않고 Phase 5까지 확정된 독립 Runtime Snapshot을 HUD용 읽기 전용 ViewData로 투영한다.

```text
Selection + 선택 대상 Knowledge
→ TargetSelect + 선택 Actor→ContactId association + FCFSensorSnapshot Contact
→ 기존 FCFTargetHUDData

Target Lock
→ FCFTargetingSnapshot
→ 독립 FCFTargetLockHUDData

Target Scan
→ FCFSensorSnapshot.ScanAttempt
→ 독립 FCFTargetScanHUDData
```

`FCFTargetHUDData`는 현재 선택 대상의 Selection/Knowledge 표시만 소유한다. 진행 중 Scan Attempt와 Vehicle Target Lock을 선택 대상에 종속된 필드로 합치지 않는다.

Phase 6 HUD는 최소 다음 동시 상태를 손실 없이 표현해야 한다.

```text
Selected = B
Locked/Acquiring = A
Scanning = C
Knowledge = 각 Contact가 이미 획득한 Persistent Fact
```

따라서 다음 결합은 금지한다.

```text
SelectedContactId == Lock TargetContactId일 때만 Lock 표시
SelectedContactId == ScanAttempt.TargetContactId일 때만 Scan 표시
Selection Clear → Lock/Scan HUD 강제 Clear
DetailedScan Knowledge → Scan progress 100% 영구 표시
GetLockedTargetActor() / Scan Target Actor를 HUD truth로 직접 소비
```

Lock과 Scan ViewData가 Player-facing identity를 보강할 때는 각각의 `TargetContactId`로 동일 Refresh의 `FCFSensorSnapshot.Contacts`를 조회한다. Actor metadata, Actor 현재 위치 또는 내부 Gameplay Actor bridge를 표시 truth로 사용하지 않는다. Contact가 존재하지만 Identity가 아직 공개되지 않았으면 일반적인 미식별 표시를 사용하며 내부 ContactId를 Player-facing 이름으로 노출하지 않는다.

### 11.2 Target Lock Presentation lifecycle

Target Lock 채널은 `FCFTargetingSnapshot`의 다음 값을 직접 투영한다.

```text
State
TargetContactId
LockProgress01
LockQuality01
BreakTransitionRevision
LastBreakReason
```

표시 의미:

```text
Idle
→ 지속 Lock 상태 표시 없음

Acquiring
→ Locking 표시 + LockProgress01

Locked
→ Locked 표시 + LockQuality01

새 BreakTransitionRevision
→ 동일 Revision을 중복 소비하지 않는 짧은 Presentation-only Break feedback
```

`ClearLock()`은 Break가 아니므로 새 Break feedback을 만들지 않는다. Break 당시 Target Actor를 복원하기 위해 새 Actor 포인터나 stale TargetContactId를 Snapshot에 추가하지 않는다.

### 11.3 Target Scan Presentation lifecycle

Target Scan 채널은 현재 Selection이 아니라 `FCFSensorSnapshot.ScanAttempt`를 Authority로 사용한다.

```text
ScanAttempt.bScanning
ScanAttempt.TargetContactId
ScanAttempt.Progress01
ScanAttempt.CompletionTransitionRevision
ScanAttempt.LastCompletedContactId
```

진행 중 Attempt는 Selection이 다른 대상으로 바뀌거나 Selection이 Clear되어도 실제 Attempt가 계속되는 동안 HUD에 유지된다. 새 completion transition은 `LastCompletedContactId` 기준으로 짧은 `스캔 완료` feedback을 한 번 소비하며, Persistent DetailedScan Knowledge와 별도 lifecycle을 유지한다.

### 11.4 Production HUD sink 경계

Phase 6는 기존 Production `WBP_CFTargetPanel`의 Selection/Knowledge 표시를 보존한다. Target Lock과 Target Scan은 같은 Panel 안에 배치하더라도 **서로 다른 명시적 Widget sink와 별도 ViewData 입력**을 사용해야 한다.

P0 최소 sink:

```text
Text_TargetLock
ProgressBar_TargetLock
Text_TargetScan
ProgressBar_TargetScan
```

`Text_TargetArmor` 같은 다른 의미의 기존 Mock/예약 Row를 Lock 표시로 재사용하지 않는다. Root HUD의 전체 Panel layout을 불필요하게 재설계하지 않으며, 시각 품질·간격·가독성은 Technical Validation과 별도로 USER PIE 검증 대상으로 남길 수 있다.

---

## 12. 실패 사유 계약

Lock과 Scan 실패는 단순 bool 반환만으로 끝내지 않고 UI가 소비할 수 있는 명시 결과로 확장한다.

P0 후보 실패/결과 사유:

```text
NoSelection
NoSensorContact
OutOfRange
NoLineOfSight
InvalidTarget
SensorUnavailable
AlreadyScanning
AlreadyDetailed
AlreadyLocked
AlreadyAcquiring
LockUnavailable
LockBroken
ContactLost
TargetDestroyed
```

실패 결과 Enum 이름과 공개 API는 해당 기능 구현 Phase에서 기존 naming contract와 충돌 여부를 검토한 뒤 확정한다. Phase 2의 Sensor command naming은 이미 canonical API로 확정됐다.

---

## 13. 기존 구현에서 보존할 것

이번 재설계는 다음 기반을 폐기하지 않는다.

```text
Sensor ContactId
Live / LastKnown / Lost / DestroyedHold lifecycle
actor-free FCFSensorSnapshot
Target Actor → ContactId bridge
Detected / Identified / DetailedScan P0 projection
AnalysisCompletionRevision의 durable knowledge history 의미
Scan completion transition의 presentation event 의미
Radar가 Sensor Contact를 표시하는 구조
Target Selection의 독립 Runtime
```

이 기반은 새 책임 모델에 맞춰 위치와 사용처를 재정렬한다.

---

## 14. 기존 구현에서 Migration할 것

현재 Migration 상태는 다음과 같다.

```text
[Phase 1 완료]
1. Target Scan → bActiveScanRunning implicit coupling 제거
2. broad Active Detection과 Target Scan의 공용 runtime state 분리

[Phase 2 완료]
3. Sensor command를 StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations로 canonical 정리
4. 전체 Sensor update canonical 이름을 RunSensorUpdate로 정리하고 RunPassiveDetectionUpdate는 compatibility wrapper로 축소
5. FCFSensorContact.AnalysisProgress01 legacy progress 제거
6. HUD/Radar legacy AnalysisProgress01 경로 제거
7. Pawn facade를 RequestStartTargetScan / RequestCancelSensorOperations로 canonical 정리

[Phase 3 완료]
8. TargetEntityId(FGuid) 최소 Identity 계약 + Vehicle/Test Target concrete provider 추가
9. Sensor Contact 최초 생성 시 Entity ID 캡처 + 같은 Contact lifetime 불변성 보존
10. Identity 미지원 Invalid Guid fail-closed 호환 유지

[후속 Migration]
11. Phase 3B-1 TargetEntityId keyed Persistent Knowledge Store + Persistent Fact Reassociation + Terminal Record 구현 [완료 / Fresh Technical PASS]
12. Phase 3B-2 실제 Dynamic Knowledge 요구가 생기면 Domain Freshness + 별도 Rescan/Refresh 계약 추가
13. 장비별 Lock ownership을 가정하는 과거 기획/코드 → Vehicle Target Lock Authority로 수렴
14. VehicleFireComp의 SelectedTargetActor → GuidanceTargetActor source를 Vehicle Locked Target으로 전환
```

무분별한 리네이밍과 대규모 클래스 분할은 금지한다. 먼저 상태와 Authority를 분리하고, 의미가 안정된 뒤 필요한 이름만 교정한다.

---

## 15. 구현 Migration 순서

이 문서 확정 후 구현은 다음 순서를 기본으로 한다.

```text
Phase 1. Active Detection / Target Scan state 분리 [구현 완료]
Phase 2. Legacy Scan Progress 제거 + Sensor/Pawn API naming responsibility 정리 [구현 완료 / Fresh Technical PASS]
Phase 3A. TargetEntityId 최소 Identity Foundation [구현 완료 / Fresh Technical PASS]
Phase 3B-1. Persistent Knowledge Store + 동일 Entity Persistent Fact Reassociation + Terminal Record [구현 완료 / Fresh Technical PASS]
Phase 3B-2. 첫 실제 Dynamic Knowledge Domain + Unknown/Fresh/Stale + 별도 Rescan/Refresh 계약 [실제 요구 발생 시 후속]
Phase 4. Vehicle Target Lock Runtime + actor-free Snapshot 추가 [구현 완료 / Fresh Technical PASS]
Phase 5. Selection / Lock / Scan Gameplay Command 경계 정리 [Fresh Technical PASS]
Phase 6. HUD Presentation 채널 분리 [Technical PASS]
Phase 7. Guided Weapon target source를 Selected Target → Vehicle Locked Target으로 Migration [Technical PASS]
Phase 8. Lock Quality / Contact lifecycle / 엄폐 / 횡이동 / 연막 대응
Phase 9. Knowledge Domain 확장 / Freshness 정책 고도화
Phase 10. Fresh Build + focused Automation + PIE validation
```

각 Phase는 현재 플레이 가능한 기존 기능을 가능한 범위에서 보존하고, 한 Phase가 검증되기 전에 다음 책임까지 한꺼번에 이동하지 않는다.

### 15.1 Phase 5 Selection / Lock / Scan Gameplay Command 계약

Phase 5의 목적은 Selection, Vehicle Target Lock, Target Scan을 하나의 상태로 합치는 것이 아니라 **플레이어/Gameplay 호출자가 각 독립 Runtime에 명시적 명령을 보낼 수 있는 얇은 Command Boundary를 고정하는 것**이다.

P0 조정 계층은 새 Manager/Subsystem을 만들지 않고 현재 `ACFVehiclePawn`의 Gameplay facade를 사용한다.

```text
TargetSelectComp
= Selection Authority

VehicleTargetingComp
= Vehicle Target Lock Authority

VehicleSensorComp
= Target Scan / Detection / Knowledge Authority

VehiclePawn
= 현재 Selection을 명령 시점에 한 번 읽어
  필요한 독립 Runtime에 명시 Command만 전달하는 얇은 Gameplay facade
```

Phase 5 canonical command 의미는 다음과 같다.

```text
ConfirmCurrentTargetCandidate()
→ Selection만 변경
→ Lock/Scan 자동 시작·교체 없음

ClearSelectedTargetManually()
→ Selection만 해제
→ 기존 Lock/Acquiring과 진행 중 Target Scan을 자동 해제하지 않음

RequestLockSelectedTarget()
→ 명령 시점 Selected Actor를 exact once 읽음
→ VehicleTargetingComp::RequestLock(SelectedActor)로 전달
→ 선택 대상이 없거나 무효하면 기존 ECFTargetLockRequestResult::InvalidTarget 의미를 사용
→ 새 Command 전용 Result enum/type을 추가하지 않음
→ 이후 Selection 변경은 이미 시작된 Acquire/Lock 대상에 영향 없음

RequestClearTargetLock()
→ VehicleTargetingComp::ClearLock()만 호출
→ Selection/Target Scan/Active Detection을 변경하지 않음
→ 기존 Phase 4와 동일하게 manual clear는 Break가 아님

RequestStartTargetScan()
→ 현재 구현처럼 명령 시점 Selected Actor를 exact once 읽음
→ VehicleSensorComp::StartTargetScan(SelectedActor)로 전달
→ Sensor가 시작 시 Actor + exact ContactId를 자체 Attempt state에 capture
→ 이후 Selection 변경은 진행 중 Scan Attempt target을 바꾸지 않음

RequestCancelTargetScan()
→ 진행 중 Target Scan Attempt만 명시 취소
→ broad Active Detection Pulse는 유지
→ Selection과 Vehicle Target Lock도 유지
```

`RequestCancelSensorOperations()`는 Phase 5 이후에도 **broad system/equipment/compatibility cancel**로 유지한다.

```text
RequestCancelSensorOperations()
→ Active Detection Pulse + Target Scan Attempt 전체 취소
→ Target Scan 전용 사용자 명령으로 재해석하지 않음
```

Sensor 내부에는 Target Scan만 종료하는 최소 `CancelTargetScan()` command를 추가한다. 이 command는 현재 Scan Attempt가 없으면 false를 반환하고, 활성 Attempt가 있으면 기존 `MarkCurrentScanTargetLastKnownIfNeeded()` + `ResetCurrentScanAttempt()` 의미를 재사용해 Attempt actor/contact/progress만 정리한다. `bActiveScanRunning`과 `ActiveScanRemainingSeconds`는 변경하지 않는다.

Phase 5 P0에서 자동 연쇄 명령은 금지한다.

```text
Select ≠ Lock
Select ≠ Scan
Lock ≠ Scan

Selection 변경/해제
→ Lock 자동 교체/해제 금지
→ Scan 자동 retarget/cancel 금지

Lock clear
→ Selection/Scan 변경 금지

Scan cancel
→ Selection/Lock/Active Detection 변경 금지
```

기존 `InputAction_StartActiveScan`과 `/Game/CarFight/Input/IA_ActiveScan`의 **Asset 이름은 호환성을 위해 유지**한다. 다만 현재 V 입력의 실제 의미는 이미 Selected Target의 Target Scan이므로 C++ Property ToolTip/주석이 옛 broad Active Scan 의미를 말하는 부분은 Phase 5 구현에서 현재 의미로 교정한다.

Phase 5에서는 **새 Lock InputAction, 새 물리 키 매핑, HUD 표시, Guided Weapon source migration을 추가하지 않는다.** Command API와 Runtime boundary만 먼저 검증하고, 실제 추가 입력 배선은 명시적으로 별도 범위가 열릴 때 다룬다. Phase 6 HUD, Phase 7 Guided Weapon source migration을 선행하지 않는다.

### 15.2 Phase 7 Guided Weapon Locked Target Source 계약

Phase 7의 목적은 기존 Direct Fire/Aim/Selection 계약을 Lock 중심으로 재작성하는 것이 아니라, **실제 TargetActor Guidance가 활성인 Guided Projectile의 GuidanceTargetActor source만 Selected Target에서 Vehicle Locked Target으로 이동**하는 것이다.

Guidance target requirement는 다음 exact 조건으로 한정한다.

```text
ProjectileData 존재
AND Effective MissileGuideConfig.IsGuidanceEnabled() == true
AND GuideMode == TargetActor
→ Vehicle Locked Target 필수

그 외
- HitScan
- 비유도 Projectile / Rocket
- Guidance disabled
- TargetActor 이외 GuideMode
→ Vehicle Lock을 새 발사 전제조건으로 추가하지 않음
→ 기존 Aim / Fire / Selection 계약 유지
```

첫 발사 순간 source resolution은 다음 순서를 사용한다.

```text
VehicleFireComp
→ active ProjectileData의 Guidance requirement 판정
→ TargetActor Guidance가 아니면 GuidanceTargetActor = None 허용
→ TargetActor Guidance이면 VehicleTargetingComp 확인
→ Targeting Runtime Ready 확인
→ Targeting Snapshot State == Locked 확인
→ Snapshot public contract + internal Actor bridge 유효 확인
→ Locked Target Actor exact-once capture
```

다음 상태는 TargetActor Guided Weapon 첫 발사를 fail-closed한다.

```text
VehicleTargetingComp 없음
Targeting Runtime 미준비
Idle
Acquiring
Locked Snapshot public contract invalid
Locked Actor weak reference invalid / Actor being destroyed
Guidance target snapshot 없음
```

발사 거부는 Projectile acquire, SingleCycle Ammo reserve/commit, Ripple/Salvo sequence ammo reserve보다 먼저 확정해야 한다. Lock이 없는데 미사일만 발사하고 Guidance가 `TargetLost/GuidanceDisabled`로 뒤늦게 실패하는 fail-open 동작은 허용하지 않는다.

Ripple/Salvo는 기존 Launcher command snapshot 계약을 유지한다.

```text
첫 승인 발
→ Locked Target Actor exact-once capture
→ Volley 공통 GuidanceTargetActor snapshot으로 보존

Volley 진행 중
Selection 변경 = retarget 금지
Vehicle Lock 변경/해제 = 이미 시작된 Volley 자동 retarget 금지
새 Lock(B) = 기존 Volley(A)에 영향 없음

단, 저장된 weak GuidanceTargetActor가 실제 invalid가 되면
후속 TargetActor-guided shot은 fail-closed
```

`CommandTargetLocation`은 기존 Aim/Launcher command point 의미를 유지한다. Phase 7은 이를 Locked Actor 현재 위치로 바꾸지 않는다. 따라서 초기 사출 방향, MuzzleBlocked, Direct/Angled/Vertical release와 Reticle/Aim 계약은 재설계하지 않는다.

`UCFMissileGuideComp`의 발사 후 Target Lost/Seeker/Reacquisition 정책도 변경하지 않는다. Vehicle Lock은 **발사 전 source admission authority**이고, 이미 발사된 Missile은 기존 LaunchContext의 Target Snapshot과 자체 Seeker/LostTargetPolicy를 계속 사용한다.

Phase 7에서 금지하는 것:

```text
- TargetSelect를 Lock Authority로 재해석
- Direct Fire에 Lock 요구 추가
- Sensor/Lock/HUD Phase 1~6 재구현
- MissileGuide seeker/law/flight 재설계
- Volley 도중 Selection 또는 새 Lock으로 자동 retarget
- CommandTargetLocation을 Locked Actor 위치로 강제 교체
- 새 Targeting Manager/Subsystem 추가
```

---

## 16. Current / Design 경계

이 문서의 내용은 **Design Baseline**이다.

현재 실제 Source는 Phase 1 Operation Split, Phase 2 Legacy Progress/API naming 정리, Phase 3A TargetEntityId Foundation, Phase 3B-1 Persistent Knowledge Store/Reassociation/Terminal Record, Phase 4 Vehicle Target Lock Runtime, Phase 5 Selection/Lock/Scan Gameplay Command Boundary, Phase 6 HUD Presentation과 Phase 7 Guided Weapon Locked Target source migration까지 반영됐다. Phase 3B-2 Dynamic Knowledge Domain/Freshness/Rescan은 아직 Current 구현이 아니며 실제 요구가 생기기 전에는 선행 구현하지 않는다.

```text
[Phase 1 반영 완료]
Target Scan과 broad Active Detection이 서로 다른 Runtime state를 사용
Target Scan 시작만으로 bActiveScanRunning이 켜지지 않음
두 Operation은 독립적으로 동시에 실행 가능
각 Operation의 duration/만료가 서로를 자동 종료하지 않음

[Phase 2 반영 완료]
FCFSensorContact.AnalysisProgress01 제거
Target/Radar HUD의 legacy AnalysisProgress passthrough 제거
StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations canonical command 추가
RequestStartTargetScan / RequestCancelSensorOperations Pawn facade 추가
RunSensorUpdate canonical update 이름 적용
구형 ActiveScan/TargetedScan/RunPassiveDetectionUpdate 이름은 Deprecated 또는 private compatibility wrapper로만 유지

[Phase 3A 반영 완료]
FCFSensorContact.TargetEntityId(FGuid) 추가
ICFTargetSelectable GetTargetEntityId fail-closed 계약 추가
VehiclePawn / C++ Test Target per-instance provider 추가
Sensor Contact 최초 Entity ID 캡처 + Contact lifetime 불변 보존
Native C++ / Blueprint TargetSelectable resolver 정책에 Entity ID 통합

[Phase 3B-1 반영 완료]
Persistent Knowledge Store / Persistent Fact Reassociation / Terminal Record 구현 완료 / Fresh Technical PASS

[Phase 4 반영 완료]
UCFVehicleTargetingComp 단일 Vehicle Target Lock Authority 추가
Idle / Acquiring / Locked exact3 상태 추가
Live Contact-only RequestLock + independent Targeting Config 추가
actor-free FCFTargetingSnapshot + LockProgress/Quality + Break revision/reason 추가
Lost/Destroyed/SensorUnavailable/QualityDepleted Break와 새 ContactId 자동 Lock reassociation 금지
VehiclePawn 기본 VehicleTargetingComp ownership 추가

[Current 반영 상태 / 잔여 후속]
Phase 3B-2 Dynamic Domain / Freshness / Rescan 계약은 실제 요구 발생 시 후속
Phase 5 Selection / Lock / Scan Gameplay Command는 Fresh Technical PASS
Phase 6 HUD Presentation은 Technical PASS
Phase 7 Guided Weapon target source migration은 Technical PASS
```

현재 구현 완료 범위의 Current System truth는 다음 Systems 문서와 실제 Source가 소유한다.

```text
Document/Systems/Targeting/SensorContact.md
Document/Systems/Targeting/TargetSelect.md
Document/Systems/Targeting/VehicleTargeting.md
실제 C++ Source
```

이 문서는 Architecture Design Baseline과 Phase별 acceptance evidence를 소유하며, 실제 Current 동작 판단은 위 Systems 문서와 C++ Source를 우선한다. Phase 상태는 문서 갱신만이 아니라 해당 Phase의 Build/Automation evidence와 함께 판단한다.

---

## 17. Technical Validation 기준

재설계 구현이 끝났다고 판단하려면 최소 다음이 모두 필요하다.

```text
1. Detection / Selection / Lock / Scan state가 서로 독립적으로 전이함
2. Target Scan 시작만으로 broad Active Detection state가 켜지지 않음
3. Active Detection 시작만으로 Target Knowledge가 상승하지 않음
4. Selection 변경만으로 기존 Vehicle Target Lock이 교체되지 않음
5. Target Scan 완료/중단 후 Attempt Progress는 0으로 복귀함
6. 획득 Knowledge는 Attempt lifecycle과 독립적으로 유지됨
7. Direct Fire는 Vehicle Lock 없이 정상 동작함
8. Guided Weapon은 요구 정책에 따라 Vehicle Lock을 소비함
9. HUD가 Selection / Lock / Scan / Knowledge를 독립 표현함
10. 기존 Contact lifecycle / Radar / Target Selection regression이 없음
11. Official UE 5.8 Build PASS
12. focused Sensor / Scanner / Targeting / HUD / Guided Weapon Automation PASS
13. 필요한 PIE runtime invariant PASS
14. 동일 VehicleData를 쓰는 서로 다른 차량의 Knowledge가 서로 오염되지 않음
15. Selection 변경이 진행 중 Scan Attempt target을 자동 변경하지 않음
16. Active Detection과 Target Scan이 독립적으로 동시 실행 가능함
17. Locked(A) 상태에서 명시적 RequestLock(B)가 정의된 단일 Lock 교체 규칙대로 동작함
18. Lock Break가 지속 Broken State 없이 transition revision/reason으로 관측 가능함
19. Guided Weapon 발사 전 source는 Vehicle Locked Target이며 발사 후 Missile target은 Vehicle Selection/Lock 변경과 독립임
20. HUD/일반 Consumer가 Targeting Actor truth가 아닌 Targeting Snapshot을 소비함
21. Contact Removed 뒤 동일 valid TargetEntityId 재획득 시 Identified/DetailedScan Persistent Knowledge가 복원됨
22. 서로 다른 TargetEntityId 또는 Invalid Guid 대상에는 과거 Knowledge가 재연결되지 않음
23. 동일 VehicleData/TargetTypeId를 공유해도 서로 다른 Entity 사이 Knowledge 오염이 없음
24. Reassociation이 ScanAttempt Progress 또는 CompletionTransitionRevision을 재생성하지 않음
25. Phase 3B-1에서 DetailedScan Knowledge 복원 후에도 기존 StartTargetScan의 AlreadyDetailed / 재스캔 거부 계약이 유지됨
26. DetailedScan Reassociation은 Store에 저장된 기존 양수 AnalysisCompletionRevision을 exact 복원하며 NextAnalysisCompletionRevision / CompletionTransitionRevision을 증가시키지 않음
27. ResetSensorRuntime / InitializeSensorRuntime 재호출 / EndPlay은 Store를 clear하고 ApplySensorData / ApplyVehicleBaseSensorData는 Store를 보존함
28. 같은 TargetEntityId라도 양쪽 concrete private Source TargetId/TargetTypeId 또는 안정 TargetCategory가 충돌하면 새 Contact admission과 과거 Knowledge merge를 fail-closed함
29. None→valid TargetId / Unknown→concrete TargetCategory 최초 보강은 허용하고 KnownDisplayName 또는 Relation 변화만으로 Entity identity conflict를 발생시키지 않음
30. valid TargetEntityId 대상이 authoritative Destroyed 확인되면 기존 Knowledge가 없어도 최소 Terminal Record가 생성됨
31. terminal destroyed Entity의 같은 TargetEntityId 재등장을 정상 Reassociation으로 수용하지 않음
32. Store Record만 존재하는 대상이 Radar Contact/Selection candidate/Lock target으로 유령 생성되지 않음
33. Phase 3B-1은 실제 HP/Loadout/Defense Dynamic Domain framework나 Rescan API를 선행 구현하지 않음
34. Phase 3B-2에서 실제 Dynamic Domain이 생긴 뒤 Contact Live 상실/Removed → Stale, 재획득만으로 Fresh 금지, 명시 Refresh/Rescan으로 해당 Domain만 Fresh 갱신을 검증함
35. Persistent Knowledge Record는 Actor/UObject reference가 없는 actor-free value record이며 Contact/Actor invalidation 뒤에도 Knowledge Authority가 유효함
36. Detected-only non-terminal 대상은 새 Store Record 생성을 강제하지 않고 Identified/DetailedScan/Terminal에서만 최소 upsert하되 기존 Record 재획득은 정상 reassociation함
37. Pawn `RequestLockSelectedTarget()`은 명령 시점 Selected Actor를 한 번만 읽고 Targeting `RequestLock()`에 전달하며 이후 Selection 변경으로 대상이 자동 교체되지 않음
38. Selected Actor가 없거나 무효한 Lock command는 새 별도 상태를 만들지 않고 `InvalidTarget`으로 fail-closed하며 기존 Acquire/Lock을 자동 변경하지 않음
39. `ClearSelectedTargetManually()`은 Selection만 해제하고 기존 Acquire/Lock 및 진행 중 Target Scan Attempt를 자동 해제하지 않음
40. `RequestClearTargetLock()`은 Lock만 수동 해제하고 Selection/Target Scan/Active Detection을 변경하지 않으며 Break revision을 증가시키지 않음
41. `RequestStartTargetScan()`은 시작 시 Selected Actor를 Sensor에 전달하고 Sensor가 exact Actor + ContactId를 capture해 이후 Selection 변경으로 Scan target이 바뀌지 않음
42. `RequestCancelTargetScan()` / Sensor `CancelTargetScan()`은 Target Scan Attempt만 종료하고 동시에 실행 중인 broad Active Detection Pulse를 유지함
43. Scan-only cancel은 Selection/Vehicle Lock을 변경하지 않고 broad `RequestCancelSensorOperations()`는 별도 system/equipment/compatibility 전체 취소 의미를 유지함
44. Phase 5 P0는 새 Lock InputAction/key, HUD Presentation, Guided Weapon source migration을 추가하지 않음
```

시각 품질, 조작감, Lock feedback와 Scan feedback의 체감은 USER Validation 대상으로 남긴다.

### 17.1 Pre-Implementation Design Re-review

2026-09-17 교정 후 재검수 결과:

```text
P0 = 0
blocking P1 = 0
P2 = 2 non-blocking
Verdict = PASS / Implementation Ready
```

비차단 P2는 다음 구현 단계에서 해당 Phase 안에서 확정한다.

```text
P2-1. TargetEntityId의 concrete C++ type / provider / injection point
      → 의미·수명·유일성 계약은 확정됨
      → Phase 3에서 현재 Vehicle/Pawn 생성 구조에 맞춰 최소 구현

P2-2. StartActiveScan / StopActiveScan / RunPassiveDetectionUpdate 등 legacy naming
      → Phase 2에서 canonical API + compatibility wrapper 방식으로 RESOLVED
      → 저장된 Blueprint/기존 caller를 깨지 않으면서 신규 코드의 의미 기준을 역할명 API로 전환
```

2026-09-18 Current 기준으로 기존 P2 exact2는 모두 해소됐다.

```text
P2-1 TargetEntityId concrete carrier/provider
→ Phase 3A에서 FGuid + VehiclePawn/TestTarget provider + Sensor capture로 RESOLVED
→ Official UE 5.8 Build `52ef16043f994685bac161b6dac118d8` PASS
→ `CarFight.Sensor` `ba870c3a79964323b388fd45a3702e23` 16/16 PASS

P2-2 legacy Scan API naming
→ Phase 2 canonical API + compatibility wrapper로 RESOLVED
```

v0.1.5 Persistent Knowledge Store 설계는 2026-09-18 Pre-Implementation Review에서 `P0 0 / blocking P1 6 / P2 2`로 HOLD됐다. v0.1.6에서는 해당 P1/P2를 다음처럼 교정했다.

```text
P1-1 DetailedScan 재스캔 충돌
→ 3B-1은 Persistent Fact 복원만 수행하고 기존 AlreadyDetailed 재스캔 금지 유지
→ Dynamic Refresh는 별도 3B-2 Rescan/Refresh 계약으로 분리

P1-2 AnalysisCompletionRevision 복원 불명확
→ DetailedScanAnalysisRevision exact 저장/복원, allocator 및 completion transition 증가 금지

P1-3 Store Reset lifetime 불명확
→ ResetSensorRuntime / InitializeSensorRuntime 재호출 / EndPlay clear
→ ApplySensorData / ApplyVehicleBaseSensorData preserve

P1-4 stable identity consistency key 불명확
→ TargetEntityId + private Source TargetId/TargetTypeId + 안정 TargetCategory exact
→ KnownDisplayName / Relation은 consistency key에서 제외

P1-5 Terminal Record 생성 조건 불명확
→ valid TargetEntityId + authoritative destruction이면 미식별 대상도 최소 Terminal Record upsert

P1-6 Dynamic framework 선행 위험
→ 3B-1 Persistent Store/Reassociation/Terminal과 3B-2 실제 Dynamic Domain/Freshness를 분리

P2-1 KnownDisplayName identity 오용 가능성
→ 표시 복원값으로만 유지하고 consistency key에서 제외

P2-2 물리 Store 타입 조기 고정
→ UCFVehicleSensorComp private storage만 고정, 구체 struct/container 이름은 구현에서 최소 형태로 결정
```

교정 후에는 §7, Current Source의 `StartTargetScan / ResetSensorRuntime / InitializeSensorRuntime / ApplySensorData / AnalysisCompletionRevision` 계약, `FCFSensorContact::IsPublicContractValid()`, `FCFTargetDisplayInfo`의 `TargetId=None / TargetCategory=Unknown` 허용 계약을 다시 교차검증했다.

2026-09-18 v0.1.6 Correction 후 최종 Design Re-review 결과:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 3B-1 Implementation Ready
```

재검수에서 추가로 발견한 두 해석 공백도 같은 v0.1.6 안에서 닫았다.

```text
1. Persistent Knowledge Record lifetime
→ actor-free / UObject-free value record로 고정
→ Contact/Actor invalidation과 Knowledge Authority를 물리적으로 분리

2. Detected-only Record 생성 범위
→ non-terminal Detected-only는 새 Persistent Record 생성을 강제하지 않음
→ Identified / DetailedScan / valid-Entity authoritative Terminal에서 최소 upsert
→ 기존 Record가 있는 Entity의 Detected 재획득은 정상 reassociation
```

따라서 Phase 3B-1 구현은 §7.1~7.10과 Technical Validation 21~33, 35~36을 기준으로 시작할 수 있다. Dynamic Domain / Freshness / Rescan의 실제 구현과 검증 34는 Phase 3B-2까지 시작하지 않는다.

### 17.2 Phase 3B-1 Post-Implementation Re-review

2026-09-18 현재 dirty checkpoint의 Phase 3B-1 구현을 v0.1.6 Design Re-review PASS 기준으로 fresh 재검수했다.

- Persistent Knowledge Store는 actor-free / UObject-free value record이며 valid `TargetEntityId`만 exact key로 사용한다.
- Identified/DetailedScan 승격은 Store upsert 성공 뒤 Contact와 revision allocator를 확정해 partial mutation과 revision 중복/역행을 만들지 않는다.
- Contact Removed 뒤 same-Entity reassociation은 새 `ContactId`를 발급하되 Persistent Fact와 exact `AnalysisCompletionRevision`만 복원하고 Scan Attempt, completion transition, allocator를 재발행·증가시키지 않는다.
- concrete `TargetId`/`TargetCategory` conflict와 Terminal Entity 재등장은 Contact admission 전에 fail-closed하며 기존 Store identity를 덮어쓰지 않는다.
- `ApplySensorData` / `ApplyVehicleBaseSensorData` hot reapply는 Store를 보존하고 `InitializeSensorRuntime` / `ResetSensorRuntime` hard lifecycle은 Store를 clear한다.
- Invalid Guid 대상은 기존 Detection/Scan 호환을 유지하면서 Store를 만들지 않으며, 미관측 pre-destroyed 대상도 ghost Contact/Record를 만들지 않는다.
- Dynamic Knowledge Domain, Knowledge Freshness/TTL, Rescan/Refresh API와 Phase 3B-2 구현은 추가하지 않았다.

Fresh Technical Validation:

```text
Official UE 5.8 Build
= c135ce8fa9414f2d93f6ae98f6294339
= CarFight_ReEditor Win64 Development
= PASS / ExitCode 0

CarFight.Sensor focused Automation
= be546de596414eee93bdfb5d638f8051
= 17 / 17 PASS
= Failure 0 / EngineExitCode 0
= includes CarFight.Sensor.SEN_P0_03.PersistentKnowledge
```

Post-Implementation Re-review 결과:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 3B-1 Technical PASS
```

Phase 3B-2 Dynamic Knowledge/Freshness/Rescan은 이번 acceptance에 포함하지 않으며 미착수 상태를 유지한다.

### 17.3 Phase 4 Vehicle Target Lock Post-Implementation Re-review

2026-09-18 Phase 4는 §6 Vehicle Target Lock Authority와 §6.9 Targeting Config ownership을 기준으로 구현했다.

Current 구현:

```text
UCFVehicleTargetingComp
= 차량 단위 단일 Lock Authority
= Idle / Acquiring / Locked exact3
= Live Sensor Contact-only RequestLock
= LastKnown acquire/quality decay + Live quality recovery
= ContactLost / TargetDestroyed / SensorUnavailable / QualityDepleted Break
= exact-once BreakTransitionRevision + LastBreakReason

FCFTargetingConfig
= Sensor Analysis tuning과 분리된 P0 fallback tuning

FCFTargetingSnapshot
= actor-free public state
= State / TargetContactId / LockProgress01 / LockQuality01
= BreakTransitionRevision / LastBreakReason

ACFVehiclePawn
= VehicleTargetingComp 기본 서브오브젝트 ownership
```

Post-Implementation Review 중 blocking P1 1건을 발견했다. 초기 `RequestLock()`은 같은 Contact의 `AlreadyLocked / AlreadyAcquiring` 판정을 fresh Sensor Snapshot의 Live 검증보다 먼저 수행하고 있었다. 이를 `fresh Snapshot → exact Contact → Live 확인 → duplicate Lock 판정` 순서로 교정해 RequestLock의 Live-only 계약을 stale window에서도 보존했다.

교정 후 Fresh Technical Validation:

```text
Official UE 5.8 Build
= 5dd08ae80e5d4ea0884d1d5e914e5c71
= CarFight_ReEditor Win64 Development
= PASS / ExitCode 0

CarFight.Targeting
= d91049be29514b27b3f57397c39d69d5
= 3 / 3 PASS
= Failure 0

CarFight.TargetSelect regression
= d05c2385a69642ad9d5f58f1a6f8c6ff
= 11 / 11 PASS
= Failure 0

CarFight.Sensor regression
= cb265d1da2724940a672c46342b41adf
= 17 / 17 PASS
= Failure 0
```

Automation 실행을 위해 일시 확장했던 `Tools/RunUIAutomation.ps1` filter allowlist는 원문으로 복원했고 final runner diff는 0이다.

Post-Implementation Re-review 결과:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 4 Technical PASS
```

Phase 4는 Input/HUD/Guided Weapon source migration을 포함하지 않는다. `VehicleFireComp`는 Phase 7 전까지 기존 Selected Target source를 유지하며, 다음 실제 구현 단계는 Phase 5 Selection / Lock / Scan Gameplay Command 경계 정리다. Phase 3B-2는 실제 Dynamic Knowledge 요구가 생기기 전까지 계속 미착수로 둔다.

### 17.4 Phase 4 Mid-review Correction / Final Re-review

Phase 4 Current 승격 뒤 독립 중간검수에서 구현 계약과 acceptance evidence를 다시 대조했다.

중간검수 판정:

```text
P0 = 0
blocking P1 = 1
P2 = 1
Verdict = HOLD / Correction Required
```

blocking P1은 기존 `CarFight.Targeting` exact3가 private state seed + `AdvanceTargetingState()`로 내부 상태 전이는 검증했지만, 실제 외부 진입점인 public `RequestLock(AActor*)`를 직접 호출하지 않았다는 검증 공백이다. 특히 이전 구현 review에서 실제 blocking defect가 `RequestLock()`의 fresh Live validation order에서 발견됐으므로 이 public path를 우회한 acceptance로 Phase 5에 진입하지 않기로 했다.

교정은 Runtime 알고리즘을 변경하지 않고 테스트 seam/evidence만 보강했다.

```text
UCFVehicleSensorComp
→ FCFTargetingRequestLockTest exact friend 추가
→ Actor→ContactId private bridge + public Snapshot fixture 구성용

UCFVehicleTargetingComp
→ FCFTargetingRequestLockTest exact friend 추가
→ public RequestLock 이후 상태 진행을 결정적으로 검증하기 위한 test-only seam

CarFight.Targeting.TGT_P0_04.RequestLockPublicPath
→ Live A RequestLock = Accepted
→ Acquiring A duplicate = AlreadyAcquiring / progress 유지
→ Locked A duplicate = AlreadyLocked
→ Locked A + Live B RequestLock = B Acquiring / progress 0
→ explicit A→B replacement = BreakTransitionRevision 비증가
→ LastKnown C = ContactNotLive
→ no-contact Actor = NoSensorContact
→ nullptr = InvalidTarget
→ 거부 후 기존 B Acquire 유지
```

이로써 Technical Validation #17 `Locked(A) 상태에서 명시적 RequestLock(B)가 정의된 단일 Lock 교체 규칙대로 동작함`을 실제 public API 경로로 직접 검증한다.

P2는 상위 설계에 존재하는 `CrosshairHold / TargetCone / LOS / 거리 / 목표 기동` 기반 실제 Lock Acquisition Policy와 Phase 4 Foundation의 경계가 Current 문서에서 충분히 명확하지 않았던 문제다. Phase 4 Current 범위는 **Live Sensor Contact + FCFTargetingConfig fallback gain/decay 기반 Lock Runtime Foundation**까지이며, 실제 조준 영역·LOS·거리·기동 기반 획득/품질 정책은 후속 Gameplay/Targeting 정책으로 남긴다. 이를 `VehicleTargeting.md v1.0.1`의 미구현 범위에 명시했다.

Fresh Correction Validation:

```text
Official UE 5.8 Build
= 767de4c7b3a244a98631d1ac0bf5c873
= CarFight_ReEditor Win64 Development
= PASS / ExitCode 0

CarFight.Targeting
= adaac5891c564259b3b7b09e49689773
= 4 / 4 PASS
= Failure 0
= includes TGT_P0_04.RequestLockPublicPath

CarFight.TargetSelect regression
= 1dcd5872717744028ae8e2bb98ba2a26
= 11 / 11 PASS
= Failure 0

CarFight.Sensor regression
= 1050686421b14fef986816df4eab14eb
= 17 / 17 PASS
= Failure 0
```

검증용 `Tools/RunUIAutomation.ps1` allowlist 임시 확장은 검증 직후 원문으로 복원했고 final Git diff는 exact0이다.

Final Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 4 Technical PASS Restored
```

Phase 5는 이 correction closure 뒤의 다음 단계이며 이번 교정 범위에서는 시작하지 않았다.

### 17.5 Phase 5 Gameplay Command Pre-Implementation Review

2026-09-18 `09A v0.1.10`, `VehicleTargeting.md v1.0.1`과 Current `TargetSelect / Sensor / VehiclePawn` Source를 fresh 교차검증했다.

초기 Pre-Implementation Review:

```text
P0 = 0
blocking P1 = 3
P2 = 2
Verdict = HOLD / Contract Correction Required
```

발견 사항:

```text
P1-1. VehicleTargetingComp::RequestLock(AActor*)는 존재하지만
      현재 Selected Target을 소비하는 Pawn Gameplay command가 없음.

P1-2. 현재 RequestCancelSensorOperations()는
      broad Active Detection + Target Scan을 함께 취소함.
      이를 Target Scan 전용 command로 재사용하면 Phase 1 Operation Split을 다시 결합함.

P1-3. Selection 변경/해제와 이미 시작된 Lock/Scan의 독립 의미가
      Current 구현에는 부분적으로 성립하지만 Phase 5 acceptance contract로 고정되지 않음.

P2-1. InputAction_StartActiveScan Property ToolTip 일부가
      현재 V 입력의 Selected Target Scan 의미가 아니라 옛 broad Active Scan 의미를 설명함.

P2-2. Phase 5에서 새 Lock InputAction/key까지 추가해야 하는지 경계가 불명확하면
      Input Asset/Mapping 작업이 Command boundary보다 앞서 범위를 확장할 위험이 있음.
```

Correction으로 §15.1과 Technical Validation 37~44를 추가해 다음을 고정했다.

```text
- 새 Command Manager/Subsystem 추가 금지
- VehiclePawn을 얇은 Gameplay command facade로 유지
- RequestLockSelectedTarget / RequestClearTargetLock 추가
- 기존 Lock Result enum을 재사용하고 No Selection은 InvalidTarget fail-closed
- RequestStartTargetScan 기존 선택 snapshot 의미 유지
- Sensor CancelTargetScan + Pawn RequestCancelTargetScan 추가
- broad RequestCancelSensorOperations 의미 보존
- Selection / Lock / Scan / Active Detection 사이 implicit coupling 금지
- 기존 IA_ActiveScan Asset rename 금지, stale C++ ToolTip만 현재 의미로 교정
- 새 Lock InputAction/key, HUD, Guided Weapon migration은 Phase 5 P0에서 제외
```

Correction 후 Current Source와 다시 대조한 결과 이 계약은 기존 Component ownership을 재작성하지 않고 최소 확장으로 구현 가능하다. 특히 Scan Attempt는 이미 `CurrentScanTargetActor + CurrentScanTargetContactId`를 시작 시 capture하고, VehicleTargeting은 Selection event를 구독하지 않으므로 새 자동 동기화 계층이 필요하지 않다.

Design Re-review 결과:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 5 Implementation Ready
```

Phase 5 구현은 `CFVehiclePawn.h/.cpp`, `CFVehicleSensorComp.h/.cpp`의 최소 command surface와 focused Automation만 대상으로 한다. Phase 4 Targeting state machine 자체, Input Asset/Mapping, HUD, `CFVehicleFireComp` Guided target source는 변경하지 않는다.

---

### 17.6 Phase 5 Gameplay Command Post-Implementation Re-review

v0.1.11 Design Re-review PASS의 최소 command 계약을 그대로 구현했다.

Current Source:

```text
ACFVehiclePawn
- RequestLockSelectedTarget()
- RequestClearTargetLock()
- RequestStartTargetScan() (기존 canonical facade 유지)
- RequestCancelTargetScan()

UCFVehicleSensorComp
- CancelTargetScan()
- CancelSensorOperations() broad 의미 보존
```

핵심 Current 계약:

```text
Selection 변경/해제
!= Lock 자동 교체/해제
!= Scan 자동 retarget/cancel

RequestLockSelectedTarget
= command 시점 Selected Actor exact-once read
= 기존 ECFTargetLockRequestResult 재사용
= Selection 없음 → InvalidTarget

RequestClearTargetLock
= Lock-only manual clear
= non-break

RequestStartTargetScan
= command 시점 Selected Actor 전달
= Sensor Actor + ContactId capture

RequestCancelTargetScan
= Target Scan Attempt only cancel
= broad Active Detection Pulse 유지
= Selection/Lock 유지

RequestCancelSensorOperations
= broad system/equipment/compatibility cancel 유지
```

구현 범위는 `CFVehiclePawn.h/.cpp`, `CFVehicleSensorComp.h/.cpp`, `CFTargetingTests.cpp`의 최소 확장이다. `UCFVehicleTargetingComp` Phase 4 state machine 자체는 변경하지 않았고 새 Manager/Subsystem, Lock InputAction/key, HUD, Guided Weapon source migration도 추가하지 않았다.

Focused Automation에 `CarFight.Targeting.TGT_P0_05.GameplayCommandBoundary`를 추가했다. public API만 사용해 Target A를 Selection→Scan→Lock한 뒤 Selection을 B로 변경하고 다시 해제해도 기존 Lock/Scan ContactId가 A에 유지되는지, Lock-only clear가 Break revision/Scan/Detection을 변경하지 않는지, Selection 없는 Lock이 InvalidTarget인지, Scan-only cancel이 broad Active Detection을 유지하는지 검증했다.

Fresh Technical Validation:

```text
Official UE 5.8 Build (runtime implementation)
= 53d88fe964f74d8cbb91f39f42a3e369
= PASS / ExitCode 0

CarFight.Targeting
= 2ef26d0d70cb4f45ba57e1d0008273c7
= 5 / 5 PASS
= includes TGT_P0_05.GameplayCommandBoundary

CarFight.TargetSelect regression
= 096cf68401564c7d8fe7a9cfd3730bb0
= 11 / 11 PASS

CarFight.Sensor regression
= 6f5d671908944caa9a4e7f779fa4c9a3
= 17 / 17 PASS

Post-review P2 comment correction final source build
= 8c3d5e3eb4d94be88492d4b55e6c6577
= PASS / ExitCode 0
```

Post-Implementation Review에서 blocking runtime defect는 없었다. `CFVehiclePawn.cpp` Migration의 옛 `V → ActiveScanDurationSec` 설명 1건만 P2 stale documentation으로 발견해 `V → Selected Target RequestStartTargetScan`, legacy Stop은 broad cancel, Scan-only cancel은 `RequestCancelTargetScan`으로 교정했다. Runtime 의미 변경은 없고 최종 build로 source/binary 기준을 다시 맞췄다.

Automation 검증 동안만 `Tools/RunUIAutomation.ps1` allowlist에 Targeting/TargetSelect를 임시 추가했고 실행 뒤 원문으로 복원해 final runner diff exact0을 확인했다.

Final Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 5 Technical PASS
```

Phase 6 HUD Presentation과 Phase 7 Guided Weapon source migration은 모두 Fresh Technical Validation까지 완료됐다. TargetActor Guided Projectile은 발사 전 Vehicle Locked Target만 Guidance source로 admission하며 Direct Fire/HitScan/비유도 Projectile의 기존 계약은 유지한다.

### 17.7 Phase 6 HUD Presentation Pre-Implementation Review / Correction Re-review

Phase 5 Technical PASS 기준선과 현재 HUD Source + fresh Production Widget AssetDump를 교차검수했다.

초기 Review:

```text
P0 = 0
blocking P1 = 3
P2 = 0
Verdict = HOLD / Contract Correction Required
```

확인한 blocking P1:

```text
P1-1. 기존 FCFTargetHUDData가 현재 Selection 중심이라 Vehicle Lock을 같은 Target 채널에 추가하면 Selected B / Locked A를 표현할 수 없음.
P1-2. 기존 Scan HUD가 SelectedContactId == ScanAttempt.TargetContactId일 때만 progress/completion을 전달해, Phase 5에서 계속 살아 있는 Scan Attempt가 Selection 변경 시 Presentation에서 사라짐.
P1-3. fresh AssetDump 기준 Production WBP_CFTargetPanel에는 Selection/Knowledge/Scan sink만 있고 Lock 전용 sink가 없음.
```

Correction:

```text
- 기존 FCFTargetHUDData는 Selection + 선택 Contact Knowledge 전용으로 유지.
- FCFTargetLockHUDData를 별도 추가하고 FCFTargetingSnapshot을 actor-free source로 사용.
- FCFTargetScanHUDData를 별도 추가하고 FCFSensorSnapshot.ScanAttempt를 Selection과 무관한 source로 사용.
- Lock/Scan target의 표시 identity는 각 ContactId를 같은 Sensor Snapshot에서 찾아 보강하며 Actor truth를 읽지 않음.
- Production WBP_CFTargetPanel에는 의미가 명시된 Text_TargetLock / ProgressBar_TargetLock sink를 additive 추가하고 기존 Text_TargetScan / ProgressBar_TargetScan은 새 Scan 채널이 소유.
- Text_TargetArmor를 Lock 표시로 재사용하지 않음.
- Phase 7 VehicleFireComp / GuidanceTarget source migration은 변경 0.
```

Correction Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 6 Implementation Ready
```

### 17.8 Phase 6 HUD Presentation Implementation / Fresh Technical Validation / Final Re-review

Design Re-review PASS 계약을 그대로 구현했다. 기존 `FCFTargetHUDData`는 Selection + 선택 Contact Knowledge 전용으로 유지하고, `FCFTargetLockHUDData`와 `FCFTargetScanHUDData`를 `FCFInGameUIViewData`의 독립 채널로 추가했다.

Current Source 경계:

```text
Selection + selected Knowledge
→ UCFHUDDataProvider::FillTargetViewData
→ FCFTargetHUDData

Vehicle Target Lock
→ FCFTargetingSnapshot
→ UCFHUDDataProvider::FillTargetLockViewData
→ FCFTargetLockHUDData

Target Scan
→ FCFSensorSnapshot.ScanAttempt
→ UCFHUDDataProvider::FillTargetScanViewData
→ FCFTargetScanHUDData
```

`UCFHUDPresenter`는 `ApplyTargetViewData / ApplyTargetLockViewData / ApplyTargetScanViewData`를 별도 경로로 사용한다. Selection Clear는 Lock/Scan 표시를 강제 Clear하지 않으며 Lock Break와 Scan Completion은 각각 `BreakTransitionRevision` / `CompletionTransitionRevision`으로 동일 전이 중복 재생을 막는다.

Production UMG는 기존 `WBP_CFTargetPanel` Designer Tree를 rebuild하지 않고 exact additive migration을 적용했다.

```text
Pre-migration fresh AssetDump
TargetPanel fingerprint = 2EFE9807
Widget count = 11

Post-migration fresh AssetDump
TargetPanel fingerprint = 8991232E
Widget count = 13

Added exact2
- Text_TargetLock [TextBlock]
- ProgressBar_TargetLock [ProgressBar]

Existing Selection / Knowledge / Scan widgets preserved
Text_TargetArmor reuse = 0
Other Production HUD asset mutation = 0
```

Production apply report:

```text
mode = target_phase6_apply
success = true
exact_mutated_asset_count = 1
exact_mutated_asset = /Game/CarFight/UI/HUD/Panels/WBP_CFTargetPanel
target_panel_tree_rebuilt = false
new_target_lock_semantic_widgets_additive_only = true
phase5_runtime_mutated = false
phase7_guided_weapon_mutated = false
```

Fresh Technical Validation:

```text
Official UE 5.8 Build after latest Source correction
Job = f899ebafdb8d43a88e6788561b4c7a8f
PASS / ExitCode 0

Persisted TargetPanel 반영 후 Official UE 5.8 Build
Job = 608d40838ff1410680a0140d630f1e50
PASS / ExitCode 0 / Target up to date

CarFight.Targeting.Phase6.HUDPresentationChannels
Process = de7c638087374154b8649f22b1175885
1 / 1 PASS / Failure 0

CarFight.UI.UI_P0_07.TargetKnowledgePanelContract
Process = d852293d8eba43b49e12cd20f722a07b
1 / 1 PASS / Failure 0
```

첫 build 시 신규 Provider 회귀가 기존 private Sensor deterministic seam을 사용하면서 exact test friend가 빠져 컴파일 실패했으나, Product Runtime/Public API를 늘리지 않고 `FCFPhase6HUDChannelsTest` exact friend 하나만 추가해 교정했다. 교정 후 build와 두 focused Automation은 모두 PASS했다.

Final Technical Review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 6 Technical PASS
```

Phase 5 Runtime state machine/command 의미는 재구현하지 않았다. `CFLauncherComp` / `CFMissileGuideComp` Phase 7 관련 경로의 final worktree diff는 exact0이며 Guided Weapon source migration은 이 시점까지 미착수 상태였다.

### 17.9 Phase 7 Guided Weapon Locked Target Source Implementation / Fresh Technical Validation / Final Re-review

Phase 6 Technical PASS를 기준으로 기존 Selected Target 기반 `GuidanceTargetActor` source를 fresh rebaseline했다.

Pre-Implementation Review 초기 결과:

```text
P0 = 0
blocking P1 = 3
P2 = 1
Verdict = HOLD / Contract Correction Required
```

주요 문제:

```text
P1-1. VehicleFireComp 메인 발사 경로가 TargetSelect의 Selected Actor를 Guidance source로 직접 캡처.
P1-2. compatibility single-shot 경로도 Selected Actor를 직접 Guidance source로 사용.
P1-3. TargetActor Guidance가 유효 Locked Target 없이도 Projectile을 발사한 뒤 MissileGuide에서 늦게 실패할 수 있는 fail-open 경계.
P2-1. Launcher GuidanceTargetActor 주석/ToolTip이 Selected Target 의미로 고정돼 Phase 7 source authority와 불일치.
```

Correction contract:

```text
실제 Projectile Actor 실행
AND Guidance enabled
AND GuideMode == TargetActor
→ Vehicle Locked Target 필수

HitScan / 비유도 Projectile / Guidance disabled / 다른 GuideMode
→ 새 Lock 요구 없음

첫 승인 발사
→ VehicleTargetingComp::GetLockedTargetActor() exact-once capture
→ Projectile acquire / Ammo reservation 이전 fail-closed

Ripple / Salvo
→ 첫 승인 발사의 Guidance snapshot 유지
→ 이후 Selection/새 Lock으로 자동 retarget 금지
```

Correction Re-review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 7 Implementation Ready
```

Current Source 구현:

```text
VehicleFireComp
→ actual Projectile path + TargetActor Guidance requirement 판정
→ VehicleTargetingComp Runtime Ready 확인
→ GetLockedTargetActor()
→ GuidanceTargetActor snapshot
→ Launcher / Projectile LaunchContext
→ MissileGuideComp
```

`GetLockedTargetActor()`는 Runtime Ready + Locked + valid public Targeting Snapshot + valid non-destroying Actor를 모두 만족할 때만 내부 Gameplay Actor bridge를 반환한다. HUD는 계속 actor-free `FCFTargetingSnapshot`만 사용한다.

새 `ECFVehicleFireRejectReason::GuidanceTargetUnavailable`은 enum 끝에 append했고 TargetActor Guided Projectile의 pre-launch admission 실패에만 사용한다. `CommandTargetLocation`과 Aim Direction은 기존 Aim/Launcher 의미를 유지하며 Locked Actor 위치로 강제 교체하지 않는다.

Post-Implementation Review에서 blocking P1 1건을 추가 발견했다. Guidance requirement helper가 ProjectileData의 Guide 설정만 보면 HitScan 무기에 연결된 ProjectileData까지 Lock 요구로 오염시킬 수 있으므로 `ShouldUseProjectileActorFire()`를 선행 조건으로 추가했다. 첫 focused 재검증 실패는 Product defect가 아니라 테스트가 WeaponData FireMode만 바꾸고 `VehicleWeaponComp` runtime cache를 재초기화하지 않은 fixture 문제로 확정했으며, 실제 Runtime 경로와 동일하게 재초기화하도록 테스트를 교정했다.

Fresh Technical Validation:

```text
Initial Phase 7 Source Build
Job = 6e24e8f00a8347a1bde1494a6c70463e
PASS / ExitCode 0

Post-review correction Build
Job = 1a677db2ca1e445583b04a9652171059
PASS / ExitCode 0

Final fixture-aligned Official UE 5.8 Build
Job = 052b1fc8d0414f6187013655dd19f44d
PASS / ExitCode 0

CarFight.Targeting.Phase7.GuidedWeaponLockedTargetSource
Process = 731a047740c244e096e43c1f048243a1
1 / 1 PASS

CarFight.Launcher.LM_P0_03B.SchedulerContract
Process = 1e0ad2dafbee477e83e8bbd07583c951
1 / 1 PASS

CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
Process = fe4e25d60dcb4bfaa752325555bbd362
1 / 1 PASS
```

Acceptance 결과:

```text
Selected = B
Locked = A
TargetActor Guided launch
→ MissileGuideComp.GuidanceTargetActor = A

Acquiring / No valid Lock
→ GuidanceTargetUnavailable
→ Projectile acquire = 0

HitScan / Guidance-disabled Projectile
→ Vehicle Lock 없이 기존 발사 계약 유지
```

`CFMissileGuideComp / CFMissileFlightComp` source mutation은 0이며 Seeker, Guidance Law, Target Lost/Reacquisition, Flight state는 재설계하지 않았다. Product Content Asset mutation도 Phase 7 범위에서 0이고 검증용 `RunMissileTests.ps1`은 원문으로 복원해 final diff exact0이다.

Final Technical Review:

```text
P0 = 0
blocking P1 = 0
P2 = 0
Verdict = PASS / Phase 7 Technical PASS
```

Phase 1~6은 재구현하지 않았다.

---

## 18. Changelog

### v0.1.16 - 2026-09-18

- Phase 7 Guided Weapon source migration을 구현·검증해 TargetActor Guided Projectile의 Guidance source를 Selected Target에서 Vehicle Locked Target으로 전환했다.
- 실제 Projectile Actor + TargetActor Guidance에만 Lock을 요구하고 HitScan/비유도 Projectile/다른 GuideMode는 기존 발사 계약을 유지했다.
- `GuidanceTargetUnavailable`을 Projectile acquire/Ammo reservation 이전 fail-closed 사유로 추가하고 `GetLockedTargetActor()` bridge를 Runtime Ready + Locked + valid Snapshot/Actor 조건으로 강화했다.
- `Selected B / Locked A` 실제 launch에서 MissileGuideComp target A, Acquiring 발사 차단, HitScan/Guidance-disabled 무Lock 호환을 focused Automation으로 검증했다.
- Launcher Volley snapshot은 첫 승인 발사의 Guidance source를 유지하고 Selection/새 Lock 변경으로 retarget하지 않는 기존 계약을 보존했다.
- Final Build `052b1fc8d0414f6187013655dd19f44d` PASS, Phase7 `731a047740c244e096e43c1f048243a1` 1/1, Launcher `1e0ad2dafbee477e83e8bbd07583c951` 1/1, Direct Missile `fe4e25d60dcb4bfaa752325555bbd362` 1/1 PASS로 닫았다.
- Final Re-review를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 7 Technical PASS`로 확정했다. Phase 1~6은 재구현하지 않았다.

### v0.1.15 - 2026-09-18

- Phase 7 Pre-Implementation Review에서 Selected Target direct guidance source exact2, pre-launch guidance admission 부재, Launcher stale Selected wording을 확인해 `P0 0 / blocking P1 3 / P2 1`, HOLD로 판정했다.
- actual Projectile + TargetActor Guidance에만 Vehicle Locked Target을 요구하고, pre-launch fail-closed와 Volley first-shot snapshot 유지 계약을 확정했다.
- Correction Re-review를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 7 Implementation Ready`로 닫았다.

### v0.1.14 - 2026-09-18

- Phase 6 HUD Presentation을 독립 `Target / TargetLock / TargetScan` ViewData 채널과 Presenter 경로로 구현하고 Selection 변경/해제와 Lock/Scan Presentation을 분리했다.
- 저장 Production `WBP_CFTargetPanel`은 기존 Tree를 rebuild하지 않고 `Text_TargetLock / ProgressBar_TargetLock` exact2만 additive 추가했다. fresh AssetDump에서 Widget 11→13, 기존 Selection/Knowledge/Scan 보존과 `Text_TargetArmor` 비재사용을 확인했다.
- Phase 6 전용 Production apply는 TargetPanel exact1만 저장했고 다른 Production HUD mutation 0을 구조화 report로 확인했다.
- 최신 Source Official UE 5.8 Build `f899ebafdb8d43a88e6788561b4c7a8f` PASS, persisted Asset 반영 후 final Build `608d40838ff1410680a0140d630f1e50` PASS를 확보했다.
- `CarFight.Targeting.Phase6.HUDPresentationChannels` 1/1 PASS와 `CarFight.UI.UI_P0_07.TargetKnowledgePanelContract` 1/1 PASS로 Runtime→Provider 독립 채널 및 Production TargetPanel 소비를 focused 검증했다.
- Final Re-review를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 6 Technical PASS`로 닫았다. Phase 5는 재구현하지 않았고 Phase 7 Guided Weapon source migration은 미착수로 유지한다.

### v0.1.13 - 2026-09-18

- Phase 6 HUD Presentation Pre-Implementation Review를 Current HUD Source와 fresh Production AssetDump 기준으로 수행해 초기 `P0 0 / blocking P1 3 / P2 0`, HOLD를 확인했다.
- Selection 중심 `FCFTargetHUDData`에 Lock을 합칠 경우 `Selected B / Locked A`를 표현할 수 없는 문제, Scan HUD가 Selection 일치 조건 때문에 Selection 변경 후 진행 Attempt를 숨기는 문제, Production TargetPanel의 Lock sink 부재를 blocking P1으로 분류했다.
- 기존 Target은 Selection + 선택 Contact Knowledge 전용으로 유지하고 `TargetLock`과 `TargetScan`을 독립 ViewData 채널로 분리했다. Lock은 `FCFTargetingSnapshot`, Scan은 `FCFSensorSnapshot.ScanAttempt`만 Authority로 사용하도록 계약을 고정했다.
- Lock/Scan의 Player-facing identity 보강은 각 ContactId와 같은 Refresh의 Sensor Snapshot Contact로만 수행하며 Actor truth와 내부 ContactId 노출을 금지했다.
- Production `WBP_CFTargetPanel`에는 명시적 Lock sink를 additive 추가하고 기존 Scan sink는 독립 Scan ViewData가 소유하도록 고정했다. `Text_TargetArmor` 재사용은 금지했다.
- Correction Re-review를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 6 Implementation Ready`로 확정했다. Phase 7 Guided Weapon source migration은 미착수로 유지한다.

### v0.1.12 - 2026-09-18

- Phase 5 Selection / Lock / Scan Gameplay Command Boundary를 `ACFVehiclePawn`의 얇은 facade와 Sensor scan-only cancel로 구현했다. 새 Manager/Subsystem은 추가하지 않았다.
- `RequestLockSelectedTarget / RequestClearTargetLock / RequestCancelTargetScan`을 추가하고 기존 `RequestStartTargetScan`과 함께 Selection/Lock/Scan/Detection 사이 implicit coupling 금지 계약을 Current Source로 승격했다.
- Sensor `CancelTargetScan()`은 Target Scan Attempt만 취소하고 broad Active Detection Pulse를 유지하며, broad `CancelSensorOperations()` 의미는 변경하지 않았다.
- 기존 `/Game/CarFight/Input/IA_ActiveScan` 자산 이름과 V 매핑은 보존하고 C++ 설명만 현재 Target Scan 의미로 교정했다. 새 Lock InputAction/key, HUD, Guided Weapon source migration은 수행하지 않았다.
- Official UE 5.8 runtime build `53d88fe964f74d8cbb91f39f42a3e369` PASS, Targeting `5/5`, TargetSelect `11/11`, Sensor `17/17` PASS를 확보했다. Post-review P2 stale comment 교정 뒤 final source build `8c3d5e3eb4d94be88492d4b55e6c6577` PASS로 기준을 맞췄다.
- 최종 재검수를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 5 Technical PASS`로 닫았다. 다음 실제 단계는 Phase 6 HUD Presentation이며 Phase 7 Guided Weapon migration은 미착수다.

### v0.1.11 - 2026-09-18

- Phase 5 Selection / Lock / Scan Gameplay Command Pre-Implementation Review를 Current `TargetSelect / Sensor / VehicleTargeting / VehiclePawn` Source 기준으로 수행해 초기 `P0 0 / blocking P1 3 / P2 2`, HOLD를 확인했다.
- Pawn의 Selected Target→Lock command 부재, Scan-only cancel 부재로 인한 Detection/Scan 재결합 위험, Selection 변경과 기존 Lock/Scan 독립 acceptance 미고정을 blocking P1으로 분류했다.
- `InputAction_StartActiveScan` stale ToolTip과 새 Lock 물리 입력의 범위 불명확을 P2로 분류했다.
- 새 Manager 없이 VehiclePawn을 얇은 Gameplay facade로 유지하고 `RequestLockSelectedTarget / RequestClearTargetLock / RequestCancelTargetScan`과 Sensor `CancelTargetScan` 최소 command 계약을 확정했다.
- No Selection Lock 요청은 기존 `InvalidTarget`을 재사용하고 새 Result type을 만들지 않으며, Selection/Lock/Scan/Active Detection 사이 implicit coupling을 금지했다.
- `RequestCancelSensorOperations()`는 broad system/equipment/compatibility 전체 취소로 보존하고 Target Scan 전용 command로 재해석하지 않도록 했다.
- 기존 `/Game/CarFight/Input/IA_ActiveScan` 자산 이름은 보존하며 stale C++ ToolTip만 현재 Selected Target Scan 의미로 교정하도록 했다. 새 Lock InputAction/key, HUD, Guided Weapon source migration은 Phase 5 P0에서 제외했다.
- Correction 후 재검수를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 5 Implementation Ready`로 확정했다. Source 구현은 아직 시작하지 않았다.

### v0.1.10 - 2026-09-18

- Phase 4 독립 중간검수에서 public `RequestLock()` 경로 acceptance 누락을 `blocking P1 1`, 실제 조준/거리/기동 기반 Lock Acquisition Policy의 Current/후속 경계 불명확을 `P2 1`로 확인해 일시 HOLD했다.
- Product Runtime 알고리즘 변경 없이 `FCFTargetingRequestLockTest` exact test friends와 `TGT_P0_04.RequestLockPublicPath`를 추가해 Live 수락, same-target duplicate no-reset, A→B replacement, non-Live/NoContact/Invalid 거부와 replacement non-break를 실제 public API 경로로 검증했다.
- Technical Validation #17의 단일 Lock 명시 교체 규칙을 public `RequestLock()` evidence로 직접 닫았다.
- Phase 4 Current는 Live Contact + independent fallback gain/decay Lock Runtime Foundation까지이며 `CrosshairHold / TargetCone / LOS / 거리 / 목표 기동` 기반 실제 Lock Acquisition Policy는 후속 범위임을 명시했다.
- Official UE 5.8 Build `767de4c7b3a244a98631d1ac0bf5c873` PASS, Targeting `4/4`, TargetSelect `11/11`, Sensor `17/17` PASS 후 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 4 Technical PASS Restored`로 재확정했다.
- 검증용 Automation runner allowlist는 원문으로 복원했고 final diff exact0을 확인했다. Phase 5는 시작하지 않았다.

### v0.1.9 - 2026-09-18

- Phase 4 Vehicle Target Lock Runtime을 Current Source와 신규 `Document/Systems/Targeting/VehicleTargeting.md v1.0.0`에 승격했다.
- `UCFVehicleTargetingComp`, `FCFTargetingConfig`, actor-free `FCFTargetingSnapshot`, VehiclePawn 기본 Targeting Component와 Idle/Acquiring/Locked exact3 state를 구현 완료 상태로 반영했다.
- RequestLock은 fresh Sensor Snapshot에서 exact Contact가 현재 Live임을 검증한 뒤에만 새 Acquire 또는 same-Contact duplicate 결과를 반환하도록 고정했다.
- LastKnown acquire/quality decay, Live quality recovery, Lost/Destroyed/SensorUnavailable/QualityDepleted Break, exact-once Break revision과 Lost 뒤 새 ContactId 자동 Lock reassociation 금지를 Current 계약으로 기록했다.
- Post-Implementation Review에서 발견한 RequestLock validation-order blocking P1을 교정한 뒤 Official UE 5.8 Build `5dd08ae80e5d4ea0884d1d5e914e5c71` PASS, Targeting `3/3`, TargetSelect `11/11`, Sensor `17/17` PASS를 fresh evidence로 확보했다.
- 최종 재검수를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 4 Technical PASS`로 닫았다.
- Phase 3B-2는 실제 Dynamic Knowledge 요구가 생길 때까지 미착수로 유지하고, 다음 실제 단계는 Phase 5 Selection / Lock / Scan Gameplay Command 경계 정리로 고정했다. HUD와 Guided Weapon source migration은 Phase 6~7 후속으로 유지한다.

### v0.1.8 - 2026-09-18

- Phase 3B-2는 실제 Dynamic Knowledge Domain 요구가 아직 없으므로 미착수 상태를 유지하고 구현 순서상 다음 실제 단계인 Phase 4 Vehicle Target Lock Runtime을 Pre-Implementation Review했다.
- Lock 진행/유지 tuning을 Sensor Analysis/Contact tuning과 분리하고 `UCFVehicleTargetingComp` 소유의 최소 `FCFTargetingConfig` fallback으로 고정했다.
- P0 Phase 4 범위를 상태/명령/Snapshot foundation과 Pawn 기본 서브오브젝트 연결, focused Automation으로 제한했다. HUD/Input/Guided Weapon source migration은 Phase 5~7 후속으로 보존했다.
- Pre-Implementation Review를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 4 Implementation Ready`로 확정했다.

### v0.1.7 - 2026-09-18

- v0.1.6 Design Re-review PASS를 기준으로 Phase 3B-1 Persistent Knowledge Store 구현을 Current Source와 Systems에 동기화했다.
- actor-free Store, Identified/DetailedScan 즉시 upsert, Contact Removed 뒤 same-Entity reassociation, exact DetailedScan revision 복원, Terminal Record와 stable identity conflict fail-closed를 구현 완료 상태로 승격했다.
- Official UE 5.8 Build `c135ce8fa9414f2d93f6ae98f6294339` PASS / Exit0와 `CarFight.Sensor` `be546de596414eee93bdfb5d638f8051` 17/17 PASS / Failure0를 fresh evidence로 기록했다.
- Post-Implementation Re-review를 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 3B-1 Technical PASS`로 닫았다.
- Dynamic Knowledge Domain/Freshness/TTL/Rescan 및 Phase 3B-2는 미구현·미착수 상태로 유지했다.

### v0.1.6 - 2026-09-18

- Persistent Knowledge Store Pre-Implementation Review의 `P0 0 / blocking P1 6 / P2 2 / HOLD`를 반영했다.
- Phase 3B를 `3B-1 Persistent Fact Store/Reassociation/Terminal`과 `3B-2 실제 Dynamic Domain/Freshness/Rescan`으로 분리해 기존 DetailedScan 재스캔 금지 계약과 충돌하지 않도록 교정했다.
- Store lifetime을 Current API 기준으로 exact 고정했다. `ResetSensorRuntime`, `InitializeSensorRuntime` 재호출, `EndPlay`은 Store clear이며 `ApplySensorData`, `ApplyVehicleBaseSensorData`는 Store preserve다.
- DetailedScan Store Record가 기존 양수 `AnalysisCompletionRevision` 값을 `DetailedScanAnalysisRevision`으로 exact 저장/복원하도록 하고 Reassociation 중 allocator/Scan completion transition 증가를 금지했다.
- stable identity consistency key를 `TargetEntityId + private Source TargetId/TargetTypeId + 안정 TargetCategory`로 제한하고 `KnownDisplayName / Relation`을 conflict key에서 제외했다. Current DisplayInfo가 허용하는 `TargetId=None / TargetCategory=Unknown`은 미확정 상태로 취급해 `None→valid`, `Unknown→concrete` 최초 보강을 허용하고 양쪽 concrete mismatch만 충돌로 판정한다.
- Store conflict / Terminal 판정은 새 Contact를 정상 admission하기 전에 수행해 identity 위반 대상이 과거 Knowledge를 오염시키거나 정상 Live Contact로 조용히 수용되지 않도록 fail-closed 순서를 확정했다.
- valid TargetEntityId의 authoritative destruction은 기존 Knowledge 유무와 상관없이 최소 Terminal Record를 upsert하도록 명시했다.
- HP/Loadout/Defense 등 실제 Dynamic Knowledge source가 없는 Current 상태에서 범용 Domain Registry/TTL/Rescan framework를 선행 구현하지 않도록 했다.
- P0 물리 owner는 `UCFVehicleSensorComp` private storage로 유지하되 구체 container/record type 이름은 구현 시 최소 형태로 결정하도록 해 과도한 사전 추상화를 피했다.
- Store Record를 actor-free / UObject-free value record로 고정해 Contact/Actor lifetime과 Knowledge lifetime의 독립성을 물리 데이터 계약에서도 보장했다.
- Detected-only non-terminal 대상은 새 Persistent Record 생성을 강제하지 않고 Identified/DetailedScan/Terminal에서만 최소 upsert하도록 해 불필요한 장기 Record 누적을 피하면서 기존 Record 재획득은 유지했다.
- Technical Validation 25~36을 correction 계약에 맞춰 재작성하고 Phase 3B-1 검증과 후속 3B-2 Dynamic 검증을 분리했다.
- Correction 후 Current Source/Types와 재교차검증해 `P0 0 / blocking P1 0 / P2 0`, `PASS / Phase 3B-1 Implementation Ready`를 확정했다.

### v0.1.5 - 2026-09-18

- Phase 3A `TargetEntityId(FGuid)` Foundation의 실제 구현/검증 상태를 Design 문서에 동기화했다. VehiclePawn/Test Target provider, Sensor Contact 최초 캡처와 lifetime 불변 보존, native/Blueprint resolver 교정을 Current 완료 상태로 기록했다.
- Persistent Knowledge Store의 P0 Authority를 valid `TargetEntityId` exact key로 고정하고 Invalid Guid / ContactId / TargetId / VehicleData fallback 재연결을 금지했다.
- P0 물리 owner를 별도 Global Subsystem이 아니라 `UCFVehicleSensorComp` 내부의 분리된 Knowledge Storage 책임으로 시작하도록 제한했다. SaveGame, cross-vehicle 공유, replication을 선행 구현 범위에서 제외했다.
- Knowledge를 Persistent Fact / Dynamic Observation / Contact Tracking / Operation State exact4로 분류하고, Dynamic Freshness를 Unknown / Fresh / Stale exact3으로 정의했다. Dynamic Knowledge Freshness는 Contact의 `FreshnessSeconds`와 다른 정보 품질 계약으로 분리했다.
- Store는 Contact Removed 시점에 archive하는 구조가 아니라 Knowledge 획득/갱신 순간 same-Entity Record를 즉시 갱신하는 Authority라고 명시했다. `Persistent`는 Store의 Contact-lifetime 독립성을 뜻하며 Dynamic 값의 영구 Fresh를 뜻하지 않는다.
- Contact Removed → 동일 Entity 재획득 시 Persistent Fact와 최고 Knowledge Tier는 복원하되 Dynamic 값은 Stale로만 유지하고 재획득 자체로 Fresh 승격하지 않는 Reassociation 계약을 확정했다.
- `DetailedScan` 호환 Projection은 “과거 상세 Knowledge 획득 이력”을 뜻하며 모든 Dynamic 정보의 현재성을 보증하지 않는다고 명시했다. Reassociation은 `AnalysisCompletionRevision`의 durable 의미만 보존하고 Scan completion transition/HUD 완료 feedback을 재발행하지 않는다.
- same EntityId stable identity conflict와 terminal destroyed Entity 재등장은 자동 merge하지 않고 fail-visible하도록 설계했다.
- Store Record 자체는 Contact/Radar/Selection을 생성하지 않는다고 명시해 ghost target 생성을 차단했다.
- 구현 Migration 순서를 Phase 3A Identity 완료 → Phase 3B Persistent Knowledge Store/Freshness로 세분화하고 Technical Validation 21~28을 추가했다.

### v0.1.4 - 2026-09-17

- Phase 2에서 `FCFSensorContact.AnalysisProgress01`과 Target/Radar HUD legacy progress passthrough를 제거해 `Snapshot.ScanAttempt.Progress01`을 Current Scan progress 단일 Authority로 고정했다.
- Sensor canonical command를 `StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations`로, Pawn facade를 `RequestStartTargetScan / RequestCancelSensorOperations`로 정리했다.
- `RunSensorUpdate()`를 전체 Sensor update canonical 이름으로 적용하고 구형 `RunPassiveDetectionUpdate()`는 compatibility wrapper로 축소했다.
- 저장된 Blueprint와 기존 C++ caller 보호를 위해 구형 ActiveScan/TargetedScan API는 Deprecated compatibility wrapper로 유지하며 무분별한 자산 rename은 수행하지 않았다.
- Current / Design 경계와 Migration 순서를 갱신해 Phase 1/2 완료와 TargetEntityId/Vehicle Target Lock/Guided Weapon source의 후속 범위를 분리했다.
- Phase 2는 최종 코드 기준 Official UE 5.8 Build `966325479f6e4a05ab75fa350822fa45` PASS, `CarFight.Sensor` 16/16 PASS, `CarFight.UI.UI_P0_07` 2/2 PASS로 Fresh Technical PASS다. UI 회귀에서 발견된 Contact Unknown completion feedback 잔존 결함도 같은 Phase에서 교정 후 재검증했다.

### v0.1.3 - 2026-09-17

- Phase 1 Active Detection / Target Scan Operation Split 구현을 Current Source에 반영했다.
- `bActiveScanRunning / ActiveScanRemainingSeconds`를 broad Active Detection 전용 state로 고정하고 Target Scan에 독립 duration/attempt state를 부여했다.
- `StartTargetedScan()`의 broad Active Detection implicit coupling을 제거하고 두 Operation의 동시 실행·독립 만료를 Automation으로 검증했다.
- Current / Design 경계를 갱신해 완료된 Operation Split과 아직 남은 Legacy Progress, naming, Target Lock, TargetEntityId migration을 분리했다.

### v0.1.2 - 2026-09-17

- v0.1.1 Correction 후 현재 Sensor / Scan / TargetSelect / HUD / Guided Missile fire path와 재교차검증했다.
- 재검수 결과 `P0 0 / blocking P1 0 / P2 2 non-blocking`, Verdict `PASS / Implementation Ready`를 확정했다.
- `TargetEntityId` P0 수명을 Save/Load 영속 ID가 아니라 현재 Gameplay Entity lifetime의 안정·유일 Identity로 제한해 불필요한 persistence architecture를 배제했다.
- 남은 P2를 `TargetEntityId concrete carrier/provider`와 legacy Scan API naming migration exact2로 한정하고 각 구현 Phase의 후속 항목으로 명시했다.
- 문서 상태를 `Design Re-review PASS / Implementation Ready`로 승격했다.

### v0.1.1 - 2026-09-17

- Pre-Implementation Design Review의 P0/P1 지적을 반영해 `ContactId / TargetEntityId / TargetTypeId` 식별자 계층을 분리하고 Knowledge 재연결 Authority를 안정 `TargetEntityId`로 고정했다.
- 현재 VehicleData PrimaryAssetId 기반 `TargetId`는 타입 식별 성격이므로 실제 개체 Entity Identity로 재사용하지 않는다고 명시했다.
- P0 단일 Lock의 동일/다른 대상 재요청, 명시 해제와 Lock 교체 규칙을 확정했다.
- `Broken`을 지속 State에서 제거하고 break revision/reason을 가진 transition event로 정리했다.
- `VehicleLockPreferred`를 P0에서 제거하고 `NoLockRequired / VehicleLockRequired` exact2로 축소했다.
- Contact Live/LastKnown/Lost/DestroyedHold와 Lock Acquire/Quality/Break 관계를 명문화했다.
- Vehicle Targeting actor-free Snapshot 계약을 추가했다.
- Target Homing Missile의 발사 전 Vehicle Lock 소비, 발사 순간 Target Snapshot, 발사 후 Missile Seeker 독립 ownership을 확정했다.
- Target Scan이 시작 시 ContactId를 Capture하고 Selection 변경과 독립적으로 계속되는 P0 단일 Attempt 계약을 확정했다.
- Active Detection Pulse와 Target Scan의 동시 실행을 기본 허용하고 장비 자원 제한과 Runtime 상태 분리를 구분했다.
- Targeting tuning은 P0에서 `FCFTargetingConfig` + VehicleData 기본값으로 시작하고 새 Targeting DataAsset을 선행 도입하지 않도록 했다.
- 현재 CombatPlan의 싱글플레이 AI 기준을 Architecture 범위에도 명시하고 네트워크 선행 구현을 제외했다.
- Migration 및 Technical Validation 기준을 신규 Identity, Snapshot, Missile handoff 계약에 맞춰 강화했다.

### v0.1.0 - 2026-09-17

- Sensor / Target / Scan 누적 구현을 재정비하기 위한 독립 Targeting / Sensor Architecture 기준 문서를 신설했다.
- `Detection / Contact / Selection / Target Lock / Target Scan / Knowledge` 6축 책임 모델을 확정했다.
- Active Detection Pulse와 Target Scan을 서로 독립된 Operation으로 정의했다.
- Target Lock Authority를 개별 장비가 아닌 차량 공용 `Vehicle Targeting Runtime`으로 정의했다.
- `LockProgress`와 `LockQuality`, Lock Requirement, Acquisition Mode의 소유권을 분리했다.
- Contact lifetime과 Knowledge lifetime, Persistent / Dynamic Knowledge Freshness를 분리했다.
- Scan Attempt Progress와 Knowledge를 별도 상태로 고정하고 `FCFSensorContact.AnalysisProgress01`을 Legacy Migration 대상으로 지정했다.
- P0 물리 구조를 `UCFVehicleSensorComp + UCFTargetSelectComp + UCFVehicleTargetingComp` 중심으로 정리했다.
- C++ / DataAsset / Blueprint 역할, HUD Presentation 채널, 실패 사유 계약, Migration 순서와 Technical Validation 기준을 명문화했다.

---

## 19. Migration

- 이 문서 도입만으로 기존 Source나 Systems 문서가 자동 변경된 것으로 간주하지 않는다.
- `UCFVehicleSensorComp`, `FCFSensorSnapshot`, `FCFSensorContact`의 Phase 1~3B-1 변경과 Phase 5 Scan-only command boundary는 실제 Source와 `SensorContact.md v1.14.0`에 반영됐다. Vehicle Target Lock 및 Phase 5 Pawn Gameplay facade는 `VehicleTargeting.md v1.1.0`이 Current owner다. 이후 Migration은 실제 Dynamic Knowledge/Freshness, Phase 6 HUD, Phase 7 Guided Weapon source 등 미구현 책임에 한정한다.
- 기존 Targeted Scan correction에서 확보한 Contact lifecycle, Knowledge durability, Scan completion transition과 HUD transient completion feedback은 가능한 범위에서 보존한다.
- 현재 VehicleData PrimaryAssetId 기반 `TargetDisplayInfo.TargetId`는 `TargetEntityId`로 자동 승격하지 않는다. Phase 3B Knowledge Reassociation은 valid `TargetEntityId` exact match에서만 허용하고 Invalid Guid 또는 type/display identity fallback은 fail-closed한다.
- Phase 3B-1 Persistent Knowledge Store는 Current 구현이며 Contact Removed 뒤 valid same-Entity Knowledge reassociation까지 동작한다. Store Record는 계속 Radar/TargetSelect source가 아니며 Dynamic 정보는 별도 Freshness 계약 전까지 Current 값처럼 노출하지 않는다.
- 현재 VehicleFireComp / Launcher가 SelectedTargetActor를 GuidanceTargetActor로 Snapshot하는 경로는 Vehicle Target Lock 구현 이후 Locked Target source로 단계적으로 Migration한다. 발사 후 MissileGuide의 독립 target ownership은 보존한다.
- `UCFVehicleTargetingComp`와 Phase 5 Pawn Gameplay facade는 Current 구현이다. 다만 새 Lock InputAction/key, HUD Presentation과 Guided Weapon Locked Target source는 아직 미구현으로 유지한다.
- 구현 후 Systems 문서를 갱신할 때 이 Design Baseline과 실제 Source가 일치하는지 fresh review하고, 일치한 항목만 Current System으로 승격한다.
