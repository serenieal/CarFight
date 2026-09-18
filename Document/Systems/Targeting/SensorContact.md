# SensorContact

- Version: 1.15.0
- Date: 2026-09-18
- Status: Current System / Phase 3B-1 Persistent Knowledge Store + Phase 5 Target Scan Command + Phase 6 HUD Presentation integration / Fresh Technical Validation PASS
- Feature: `CF-FQ-036 차량 센서·Contact Intelligence Runtime` + `CF-FQ-037 차량 스캐너 입력·장비 통합`
- Acceptance: Historical `SEN-P0-00~07 PASS` + `SCAN-P0-00~07 PASS`와 Phase 1~3A Technical PASS를 보존한다. Phase 3B-1에서 valid `TargetEntityId(FGuid)` exact key의 `UCFVehicleSensorComp` private actor-free Persistent Knowledge Store를 추가하고 Identified/DetailedScan 즉시 upsert, Contact Removed 뒤 동일 Entity reassociation, exact `AnalysisCompletionRevision` 복원, authoritative Terminal Record와 stable identity conflict fail-closed를 구현했다. Dynamic Knowledge Domain/Freshness/Rescan은 아직 구현하지 않았다.
- Verification: Phase 3B-1과 Phase 5 Final evidence를 보존한다. Phase 6 latest Source Official UE 5.8 Build `f899ebafdb8d43a88e6788561b4c7a8f` PASS / Exit0, persisted TargetPanel 반영 후 final Build `608d40838ff1410680a0140d630f1e50` PASS다. `CarFight.Targeting.Phase6.HUDPresentationChannels` process `de7c638087374154b8649f22b1175885` 1/1 PASS가 Selection B / Lock A / Scan A 및 Selection Clear 후 Lock/Scan 유지의 실제 Provider 투영을 검증했고, `CarFight.UI.UI_P0_07.TargetKnowledgePanelContract` process `d852293d8eba43b49e12cd20f722a07b` 1/1 PASS가 저장 Production TargetPanel 소비를 검증했다.


---

## 1. 문서 목적

이 문서는 CarFight의 현재 차량 Sensor / Contact Intelligence Runtime이 **무엇을 탐지하고, Contact를 얼마나 기억하며, 플레이어가 대상에 대해 어느 정도의 Knowledge를 획득했는지**를 어떤 구조로 소유하는지 기록한다.

현재 구현 판단은 완료 당시 Plan보다 실제 Source와 이 Current System을 우선한다.

```text
현재 구현 기준
= 실제 C++
→ Document/Systems/Targeting/SensorContact.md
→ 관련 TargetSelect / HUD Current 구현
→ Document/Plan/Archive/SensorContactPlan.md 완료 이력
```

이 시스템의 핵심 목적은 TargetSelect와 별개인 Sensor Runtime을 제공하는 것이다.

```text
TargetSelect
= 무엇을 후보로 보고 무엇을 선택했는가

Sensor / Contact Intelligence
= 무엇을 탐지했고 그 Contact를 얼마나 기억하며 대상에 대해 무엇을 알고 있는가

HUD
= 위 Gameplay Runtime이 공개한 의미 데이터를 읽기 전용으로 표시
```

Sensor가 TargetSelect의 후보 검색이나 선택 수명을 빼앗지 않고, TargetSelect가 Sensor Contact lifecycle이나 Knowledge를 대신 계산하지 않는 것이 현재 구조의 핵심이다.

---

## 2. 현재 구현 범위

현재 SensorContact Current System은 다음 범위를 포함한다.

```text
- 차량별 UCFVehicleSensorComp Runtime
- UCFVehicleSensorData / FCFSensorConfig 정적 설정 계약
- TargetId와 독립된 ContactId
- ContactId·표시 TargetId와 독립된 per-entity `TargetEntityId(FGuid)` foundation
- valid TargetEntityId exact key의 `UCFVehicleSensorComp` private actor-free Persistent Knowledge Store
- Identified / DetailedScan 획득 즉시 Store upsert + Contact Removed 뒤 동일 Entity 새 Contact reassociation
- DetailedScan `AnalysisCompletionRevision` exact 복원, Scan Attempt / completion transition 비복원
- authoritative Destroyed의 최소 Terminal Record와 concrete stable identity conflict Contact-admission fail-closed
- `ApplySensorData / ApplyVehicleBaseSensorData` hot reapply에서는 Store 보존, `InitializeSensorRuntime / ResetSensorRuntime`에서는 Hard clear
- bounded World Actor scanning
- Passive Detection
- Visual fallback Detection
- Active Scan Detection
- Live / LastKnown / Lost / DestroyedHold Contact lifecycle
- Lost 전 same-ID reacquire
- 단일 지정 Target Scan Attempt gain / decay
- Contact Knowledge와 분리된 `FCFSensorScanAttempt` current progress / completion transition
- Detected / Identified / DetailedScan Sensor Knowledge
- VehicleHealth authoritative destruction signal 소비
- actor-free FCFSensorContact / FCFSensorSnapshot
- TargetSelect 선택 상태 + 선택 Contact Knowledge의 `FCFTargetHUDData` read-only 합성
- Selection과 독립된 `FCFSensorSnapshot.ScanAttempt → FCFTargetScanHUDData` HUD 투영
- Lock/Scan 대상 표시 identity 보강을 위한 같은 Refresh Sensor Snapshot Contact lookup
- Sensor Snapshot 기반 Radar Contact ViewData
- Utility Scanner EquipmentPreset / FittingSnapshot의 `ResolvedSensorData` 해석
- `UCFVehicleFittingComp`의 `ResolvedSensorData → ApplySensorData()` 초기 적용·hot reapply·compensation
- `ACFVehiclePawn`의 `IA_ActiveScan` Enhanced Input command binding과 기본 `V` 매핑
- `V` 1회 → 현재 TargetSelect 선택 Actor 하나의 `RequestStartTargetScan()` → `StartTargetScan()` 시작. 선택 대상 없음은 fail-closed
- 지정 Scan Attempt 완료 → DetailedScan Knowledge 보존 + Scanner Runtime 즉시 Idle + Attempt progress 0/inactive
- 활성 중 반복 입력 무연장, 이미 DetailedScan 완료된 동일 Contact의 즉시 재스캔 거부 계약
- `UCFVehicleData.DefaultSensorData` 기반 차량 기본 Sensor Source
- 장착 Scanner override > 차량 기본 SensorData > zero-range Fallback source priority
- Scanner 제거 시 Contact/Knowledge를 지우지 않고 차량 기본 Sensor로 복귀
- 의도적 Sensorless 또는 기본 Sensor 미설정 차량의 안전한 disabled/fallback 처리
- Contact의 최초 DetailedScan Knowledge 획득 이력 `AnalysisCompletionRevision`과 현재 Scan Attempt 완료 전이 `CompletionTransitionRevision`의 독립 계약
- TargetPanel은 현재 Scan Attempt active일 때만 `스캔 XX%`를 표시하고 새 completion transition을 0.75초 `스캔 완료` + 100% feedback으로 한 번 소비한 뒤 Scan UI를 숨김. Sensor Contact가 Unknown으로 바뀌면 남은 completion feedback 시간과 무관하게 즉시 fail-closed
```

현재 범위에 포함하지 않는 항목은 다음과 같다.

```text
- Radar 화면 Range / Zoom 정책
- Radar -1~1 NormalizedPosition 산식
- 실제 동적 Radar Blip Widget 생성·배치
- TargetPanel / Radar 최종 USER Visual 승인
- Sensor power / energy / heat 소비
- 네트워크 복제 / 서버 권한 Sensor
- AI Sensor 소비
- TargetSelect 후보 검색을 Sensor Snapshot으로 교체
- Production Scanner 등급별 최종 밸런스·질량 수치 확정

```

---

## 3. 핵심 Source와 역할

### 3.1 `CFSensorTypes.h`

현재 공개 Sensor 의미 계약을 소유한다.

```text
ECFSensorContactState
FCFSensorConfig
FCFSensorContact
FCFSensorScanAttempt
FCFSensorSnapshot
```

`ECFTargetRelation`, `ECFTargetCategory`, `ECFTargetInfoLevel`은 기존 TargetSelect와 의미가 정확히 같은 범위에서만 재사용한다.

`ECFTargetTrackState`는 TargetSelect의 추적 품질이므로 Sensor Contact lifecycle로 재사용하지 않는다.

### 3.2 `UCFVehicleSensorData`

종류:

```text
UPrimaryDataAsset
```

역할:

```text
FCFSensorConfig SensorConfig 보관
Sensor Config DataValidation
Debug Summary
```

`CF-FQ-036` 완료 당시에는 새 SensorData Content `.uasset`을 생성하거나 저장하지 않았다.

2026-09-16 correction에서 정상 신규 차량이 공통 Basic Sensor를 사용할 수 있도록 `UCFVehicleData.DefaultSensorData`와 Runtime source priority를 추가했고, Production 공통 자산 `/Game/CarFight/Vehicles/Data/Sensor/DA_VehicleSensor_Basic`을 exact1 materialize했다. Fresh persisted AssetDump에서 `UCFVehicleSensorData` exact1과 Passive 20m / Visual 30m / Active 40m / Active Scan 3초 / Update 0.2초 / **Analysis Gain 0.40/s** / Decay 0.15/s / Radar 10·20·40m(기본 20m)를 확인했다. 이 수치는 `09_LockSensorEW.md v0.3.2`의 **P0 기술·플레이테스트 baseline**이며 최종 Scanner/맵 밸런스는 DataAsset 값으로 후속 조정할 수 있다. 이전 `Analysis Gain 0.25/s`는 3초 Scan에서 DetailedScan 1.0에 도달할 수 없어 폐기된 설정이다.

### 3.3 `UCFVehicleSensorComp`

차량별 Sensor Runtime의 실제 owner다.

주요 책임:

```text
- Config 해석
- bounded World scan cursor
- private Actor-backed Runtime Contact
- Passive / Visual / Active Detection
- Contact lifecycle
- 단일 지정 Target Scan Attempt 진행/decay
- Sensor Knowledge 승격
- Scan Attempt 완료 전이와 Scanner Idle 복귀
- VehicleHealth destruction event 소비
- actor-free Snapshot 게시
```

### 3.4 `UCFTargetSelectComp`

Sensor의 하위 시스템이 아니다.

현재 독립 책임:

```text
- 후보 검색
- 직접 선택
- 선택 기록
- 선택 유효성
- 선택 TrackState
- TargetSelect 전용 LOS
- 선택 대상 Destroyed 즉시 clear
```

Sensor가 TargetSelect 후보를 공급하거나 선택 대상을 변경하지 않는다.

### 3.5 `UCFHUDDataProvider`

Gameplay 판정을 새로 계산하는 시스템이 아니라 read-only Adapter다.

```text
TargetSelect
→ 선택 존재 / 선택 유효성 / TrackState

Sensor Snapshot
→ Target Knowledge
→ Contact lifecycle
→ Radar Contact
```

을 하나의 Player-facing HUD ViewData로 변환한다.

---

## 4. Sensor Config 계약

현재 `FCFSensorConfig` 필드는 다음과 같다.

| 필드 | 기본값 | 현재 의미 |
| --- | ---: | --- |
| `PassiveDetectionRangeCm` | `0` | 상시 360도 Passive 거리. 0이면 비활성 |
| `ActiveScanRangeCm` | `0` | broad Active Detection의 장거리 360도 Detection 범위이자 Target Scan의 선택 대상 거리 자격 상한. 두 Operation은 이 Config 값을 공유하지만 Runtime state는 독립. 0이면 둘 다 시작 불가 |
| `VisualDetectionRangeCm` | `0` | Passive 밖에서 직접 가시 대상을 탐지할 후보 범위 |
| `UpdateIntervalSec` | `0.1` | Detection·lifecycle·Analysis 기본 update 간격 |
| `MaxActorScansPerUpdate` | `64` | 한 update에서 검사할 Level Actor 슬롯 CPU 예산, 유효범위 1~4096 |
| `ContactMemoryTimeSec` | `0` | Live 관측 상실 뒤 LastKnown을 유지할 시간 |
| `DestroyedHoldTimeSec` | `0` | 파괴 확정 Contact를 Snapshot에 보존할 시간 |
| `ActiveScanDurationSec` | `0` | broad Active Detection Pulse와 Target Scan Attempt가 시작 시 각각 독립 timer에 복사해 사용하는 기본 지속 시간 |
| `AnalysisGainPerSec` | `0` | 유효 Tactical Analysis의 초당 progress 증가량 |
| `AnalysisDecayPerSec` | `0` | 분석 조건 상실 시 초당 progress 감소량 |
| `IdentifiedThreshold` | `0.5` | Detected → Identified 승격 threshold |
| `DetailedScanThreshold` | `1.0` | Identified → DetailedScan 승격 threshold |

거리와 수명 관련 `0` 기본값은 실제 게임 Sensor tuning이 아직 확정되지 않은 **안전한 비활성 Foundation 값**이다.

`MaxActorScansPerUpdate`는 Sensor 성능이나 출력 세기가 아니라 CPU 작업량 제한이다.

현재 Validation 핵심:

```text
모든 수치는 유한해야 함
UpdateIntervalSec > 0
MaxActorScansPerUpdate = 1~4096
거리·시간·gain/decay는 음수 금지
threshold = 0~1
DetailedScanThreshold > IdentifiedThreshold
ActiveScanRangeCm = 0은 유효한 비활성 값
ActiveScanRangeCm != 0이면 PassiveDetectionRangeCm 이상
AnalysisGainPerSec > 0이면 AnalysisGainPerSec * max(ActiveScanDurationSec - UpdateIntervalSec, 0) >= DetailedScanThreshold
```

Tactical Analysis가 활성인 Sensor는 마지막 scan-expiry update에 의존하지 않고도 정상 Active Scan 1회 안에 DetailedScan까지 완료 가능한 시간 예산을 가져야 한다. `AnalysisGainPerSec=0`인 탐지 전용/zero-range fallback은 이 단발 완료 요구를 적용하지 않는다. 의도적인 multi-pass 분석 장비는 별도 명시 계약이 생기기 전까지 유효한 tuning으로 취급하지 않는다.

---

## 5. 차량 Runtime 연결

`ACFVehiclePawn`은 `UCFVehicleSensorComp`를 기본 Subobject로 소유한다.

현재 초기화는 차량 Runtime 초기화 과정에서 명시적으로 수행한다.
Sensor는 현재 `CoreReady`나 `CombatReady`를 실패시키는 필수 Gate로 사용하지 않는다.

차량 초기화는 Fitting Scanner 적용 전에 `UCFVehicleData.DefaultSensorData`를 `UCFVehicleSensorComp::ApplyVehicleBaseSensorData()`로 먼저 설정한다. Scanner가 장착된 FittingSnapshot은 `ResolvedSensorData`를 통해 `UCFVehicleSensorComp::ApplySensorData()`로 더 높은 우선순위의 override를 적용한다. 초기 출격은 Sensor Runtime 초기화 전에 두 Source를 구성하고, Field Fitting hot reapply는 Contact/Knowledge를 지우는 `InitializeSensorRuntime()` 재호출 없이 non-destructive Apply 경로를 사용한다. Sensor Source가 바뀔 때 진행 중 Target Scan Attempt는 안전하게 종료하며 Current Scan progress를 Contact에 보존하지 않는다.

현재 effective source priority는 다음과 같다.

```text
장착 Scanner SensorData
→ VehicleData.DefaultSensorData
→ FallbackSensorConfig
```

Scanner가 제거되면 유효한 `DefaultSensorData`가 있을 경우 차량 기본 Sensor로 복귀한다. 둘 다 없거나 차량이 의도적으로 Sensorless이면 기존 `FallbackSensorConfig`를 유지한다. Fallback의 기본 거리값은 0이므로 이 경우 Active Scan 시작이 거부되고 불필요한 World Detection Tick을 계속 돌리지 않는다.


---

## 6. bounded World Detection

현재 Sensor는 매 update마다 `TActorIterator`로 World 전체를 처음부터 반복하지 않는다.

다음 persistent cursor를 사용한다.

```text
PassiveScanLevelIndex
PassiveScanActorIndex
PassiveScanCycleSerial
```

흐름:

```text
UWorld::GetLevels()
→ ULevel::Actors[]
→ 이전 update의 cursor 위치에서 계속
→ MaxActorScansPerUpdate만큼 Actor 슬롯 소비
→ 다음 update에서 이어서 진행
```

null Actor 슬롯도 budget을 소비한다.

작은 World에서 한 update의 남은 budget으로 같은 Actor를 여러 번 반복 검사하지 않도록 한 World actor cycle을 완료하면 그 update의 scan을 종료한다.

중요한 의미:

```text
bounded cursor가 이번 update에 Actor를 방문하지 않음
!= 탐지 실패
```

Contact 수명은 bounded cursor 방문 여부가 아니라 실제 탐지 실패와 독립 시간 흐름으로 관리한다.

---

## 7. TargetSelectable 재사용 경계

Sensor 후보는 기존 `ICFTargetSelectable`을 완전히 다른 시스템으로 복제하지 않고 다음 최소 의미만 재사용한다.

```text
IsTargetSelectable
GetTargetSelectionLocation
GetTargetDisplayInfo
```

Sensor용 Context:

```text
ContextId = SensorPassive
bAllowSelfTarget = false
bRequireLineOfSightForNewSelection = false
```

재사용하지 않는 것:

```text
TargetSelect 후보 점수
TargetSelect 선택 상태
TargetSelect TrackState
TargetSelect 전용 LOS 정책
TargetSelect clear 정책
```

Source `FCFTargetDisplayInfo.InformationLevel`은 Actor가 가진 원본 metadata일 뿐 Player Knowledge가 아니다.
따라서 새 Sensor Contact에 직접 복사하지 않는다.

---

## 8. Passive / Visual Detection

현재 baseline Detection은 다음과 같다.

```text
Distance <= PassiveDetectionRangeCm
→ 360도 탐지
→ LOS 불필요

Passive 밖
AND Distance <= VisualDetectionRangeCm
→ ECC_Visibility 직접 가시성 검사
→ 직접 보이면 탐지

둘 다 불충족
→ 탐지 실패
```

Visual direct visibility는 Sensor Owner 위치에서 TargetSelectable 대표 위치까지 `ECC_Visibility`를 사용한다.

```text
blocking hit 없음
→ visible

최초 blocking actor == candidate
→ visible

중간 blocker 존재
→ not visible
```

TargetSelect 전용 `CFCollisionChannels::TargetSelect`를 Sensor가 사용하지 않는다.

---

## 9. Active Scan Detection

Active Scan Runtime은 입력 자산 자체를 소유하지 않는다. 입력 owner는 `ACFVehiclePawn`이며 Sensor Component는 Gameplay command만 받는다.

공개 canonical Runtime command:

```text
StartActiveDetectionPulse()
StartTargetScan(AActor* TargetActor)
CancelTargetScan()
CancelSensorOperations()
IsActiveDetectionPulseRunning()
IsTargetScanRunning()
GetActiveDetectionPulseRemainingSeconds()
```

기존 `StartActiveScan / StartTargetedScan / StopActiveScan`과 구형 상태 Getter는 저장된 Blueprint·기존 C++ caller 호환을 위한 Deprecated compatibility wrapper로만 유지한다. 새 제품 코드와 테스트의 의미 기준은 canonical API다.

현재 플레이어 입력 흐름:

```text
/Game/CarFight/Input/IA_ActiveScan
+ IMC_Vehicle_Default : V
→ ACFVehiclePawn Enhanced Input Started
→ RequestStartTargetScan()
→ 현재 TargetSelect 선택 Actor 확인
→ UCFVehicleSensorComp::StartTargetScan(SelectedTargetActor)
→ broad Active Detection state는 켜지지 않음
→ Target Scan 독립 duration 예산 안에서 선택 Target 1개 분석
→ DetailedScan 완료 시 Target Scan Attempt만 즉시 Idle
```

선택 Target이 없거나 이미 `DetailedScan`까지 완료된 동일 Contact는 새 Target Scan을 시작하지 않고 fail-closed한다. 활성 중 `V` 반복 입력도 남은 시간을 초기화하거나 연장하지 않는다. Phase 5부터 Target Scan 전용 취소는 Sensor `CancelTargetScan()` / Pawn `RequestCancelTargetScan()`을 사용하며, Scan Attempt만 종료하고 동시에 실행 중인 broad Active Detection Pulse는 유지한다. 별도 기본 Stop 키는 추가하지 않았고 `CancelSensorOperations()` / `RequestCancelSensorOperations()`은 시스템·장비 전환용 broad 전체 취소 command로 유지한다.

`StartActiveDetectionPulse()`는 broad Detection Foundation command이며 플레이어 V 입력의 기본 분석 경로가 아니다. 구형 `StartActiveScan()`은 이 canonical command를 호출하는 compatibility wrapper다.

Phase 1 이후 Runtime state authority는 다음처럼 분리된다.

```text
Broad Active Detection Pulse
= bActiveScanRunning (private/internal legacy-named state)
= ActiveScanRemainingSeconds (private/internal legacy-named state)
= IsActiveDetectionPulseRunning()
= GetActiveDetectionPulseRemainingSeconds()

Target Scan Attempt
= bCurrentScanAttemptActive (private runtime)
= CurrentScanAttemptRemainingSeconds (private runtime)
= Snapshot.ScanAttempt.bScanning / Progress01
= IsTargetScanRunning()
```

두 Operation은 동시에 실행할 수 있고, 먼저 시작한 broad Pulse가 만료되어도 진행 중 Target Scan을 자동 종료하지 않는다. 반대로 Target Scan 완료/만료도 broad Pulse를 자동 종료하지 않는다. `CancelTargetScan()` / `RequestCancelTargetScan()`은 Target Scan Attempt만 취소하고 broad Pulse를 유지한다. `CancelSensorOperations()` / `RequestCancelSensorOperations()`은 두 Operation을 함께 취소하는 broad command이며, 구형 `StopActiveScan()` / `RequestStopActiveScan()`은 Deprecated compatibility wrapper다.

Active Scan Contact Detection:

```text
bActiveScanRunning
AND Distance <= ActiveScanRangeCm
→ 360도 Contact Detection
→ LOS 불필요
```

따라서 건물 뒤 또는 차량 뒤쪽 대상도 Active range 안이면 Contact 자체는 Live로 유지할 수 있다. 다만 **Knowledge 분석 진행은 현재 지정 Scan Target 하나에만 적용**되며 Detection과 Analysis의 대상 범위를 동일하게 취급하지 않는다.

Active Scan 종료 시 마지막 관측이 Active-only였던 Contact는 `LastKnown`으로 전환한다. Passive/Visual baseline도 유효했던 Contact는 `Live`를 유지한다.

---

## 10. ContactId와 Contact 생성

`ContactId`는 Target의 `TargetId`와 다른 Sensor 전용 식별자다.

현재 Identity 책임은 다음처럼 분리한다.

```text
ContactId
= 현재 Sensor Contact 수명 식별자

KnownTargetId / TargetId
= 식별 이후 공개되는 표시·데이터 Identity
= 차량에서는 VehicleData PrimaryAssetId 기반 타입/모델 Identity

TargetEntityId (FGuid)
= 같은 Gameplay Entity instance lifetime을 식별하는 개체별 Identity
= ContactId나 표시 TargetId를 대체하지 않음
```

`ICFTargetSelectable::GetTargetEntityId()` 기본 구현은 Identity를 추측하지 않고 Invalid Guid를 반환한다. `ACFVehiclePawn`은 같은 Pawn lifetime 동안 한 번 발급한 FGuid를 유지하고, Sensor는 Contact 최초 생성 때 이 값을 `FCFSensorContact.TargetEntityId`에 캡처한다. 이미 유효한 Entity ID를 가진 같은 Contact에서 다른 유효 Entity ID가 관측되면 조용히 덮어쓰지 않고 해당 관측을 fail-closed한다.

Sensor의 TargetSelectable 호출은 기존 정책을 그대로 따른다. 실제 Blueprint override가 있으면 generated `Execute_...`를 사용하고, 그렇지 않은 Native C++ 구현은 virtual `_Implementation()`을 직접 호출한다. 이 정책은 `GetTargetEntityId`에도 동일하게 적용된다.

현재 `TargetEntityId`는 **Persistent Knowledge Store의 exact key**로 사용된다. valid Entity의 Contact가 `Identified` 또는 `DetailedScan`으로 승격되는 순간 `UCFVehicleSensorComp` private Store에도 같은 Knowledge를 즉시 upsert하고, Contact가 `Removed`된 뒤 같은 `TargetEntityId`가 다시 관측되면 stable identity/Terminal 검사를 통과한 경우 새 Contact에 Persistent Fact를 복원한다.

Store Record는 Actor/UObject reference를 갖지 않는 value record다. `TargetEntityId`, stable Source `TargetId/TargetCategory`, 공개 가능한 Known Identity, 최고 Knowledge Tier, DetailedScan의 exact `AnalysisCompletionRevision`, Terminal marker만 보존한다. Detected-only non-terminal 대상은 새 Record 생성을 강제하지 않고, Invalid Guid 대상은 기존 Contact/Scan 호환을 유지하되 Store key를 추측하지 않는다.

Reassociation은 새 Scan completion이 아니다. 새 ContactId를 발급하되 기존 `DetailedScanAnalysisRevision`을 `Contact.AnalysisCompletionRevision`에 exact 복원하고 `NextAnalysisCompletionRevision`, `ScanAttempt.CompletionTransitionRevision`, Scan progress는 증가/복원하지 않는다. 이미 DetailedScan인 복원 Contact의 기본 재스캔 거부 계약도 유지한다.

same Entity의 Store와 현재 Source에서 **양쪽 concrete** `TargetId` 또는 `TargetCategory`가 서로 다르거나 Store가 Terminal이면 새 Contact를 Runtime list에 정상 admit하기 전에 fail-closed한다. `None→valid TargetId`, `Unknown→concrete TargetCategory` 최초 보강은 허용하며 DisplayName/Relation 변화는 Entity conflict 근거로 사용하지 않는다.

authoritative VehicleHealth destruction은 이미 Sensor가 알고 있던 valid Entity Contact에 최소 Terminal Record를 upsert한다. DestroyedHold Contact가 이후 제거돼도 Terminal Store는 남고, 같은 `TargetEntityId`의 재등장은 정상 reassociation으로 수용하지 않는다. 반대로 미관측 pre-destroyed Actor만으로 ghost Contact/Store를 만들지는 않는다.

예:

```text
Contact_000001
Contact_000002
...
```

같은 Component lifetime에서는 serial을 재사용하지 않는다.

새 Contact는 항상 다음 Knowledge에서 시작한다.

```text
InformationLevel = Detected
KnownTargetId = None
KnownDisplayName = empty
```

Source TargetSelectable이 이미 `Identified` metadata를 가지고 있어도 Sensor가 분석을 완료하기 전에는 Player Knowledge로 누출하지 않는다.

---

## 11. Contact lifecycle

현재 Sensor lifecycle은 다음과 같다.

```text
Invalid
  ↓ 최초 탐지
Live
  ↓ 실제 탐지 실패
LastKnown
  ↓ ContactMemoryTimeSec 만료
Lost
  ↓ 최소 한 번 Public Snapshot 게시 후 다음 update
Removed = Snapshot에서 사라짐
```

`Removed`는 enum 상태가 아니다.
Contact가 Snapshot에서 없어지는 것으로 표현한다.

### 11.1 Live

현재 Sensor가 유효하게 관측한 상태다.

재관측 시:

```text
같은 ContactId 유지
같은 TargetEntityId 유지
LastKnownWorldLocation 갱신
LastObservedWorldTimeSeconds 갱신
FreshnessSeconds = 0
Knowledge 유지
```

### 11.2 LastKnown

현재 관측은 실패했지만 마지막 신뢰 정보를 기억한다.

보존:

```text
ContactId
TargetEntityId
KnownTargetId / KnownDisplayName
InformationLevel
AnalysisCompletionRevision
LastKnownWorldLocation
LastObservedWorldTimeSeconds
```

현재 Scan Attempt progress는 Contact lifecycle 보존 대상이 아니다. LastKnown 위치는 Actor의 현재 위치를 추정해 따라가지 않는다.

### 11.3 Lost

ContactMemory가 만료된 상태다.

Lost는 최소 한 번 Actor-free public Snapshot에 포함된다.
그 다음 Sensor update에서 제거된다.

Lost가 제거되기 전에 같은 Actor를 재획득하면 기존 ContactId, TargetEntityId와 획득 Knowledge를 유지한 `Live`로 복귀한다. Lost가 이미 `Removed`된 뒤에는 기존 Runtime Contact가 없으므로 새 ContactId를 발급한다. 이때 새 Contact의 valid `TargetEntityId`가 private Persistent Knowledge Store의 same-Entity Record와 일치하고 identity/terminal 검사를 통과하면 Identified/DetailedScan Knowledge와 exact `AnalysisCompletionRevision`을 복원한다.

### 11.4 weak Actor invalid

weak Actor가 invalid가 됐다는 사실만으로 파괴를 추정하지 않는다.

```text
Live
→ LastKnown
→ Lost
→ Removed

bDestroyedConfirmed = false
```

---

## 12. Tactical Analysis와 Knowledge

Active Scan Detection과 Tactical Analysis는 같은 조건이 아니다.

Contact Detection:

```text
Active range 안
→ LOS 불필요
```

Analysis gain:

```text
현재 Targeted Scan Attempt 활성
AND Contact == 현재 지정 Target Contact
AND Contact == Live
AND Actor 유효
AND TargetSelectable 자격 유효
AND 현재 실제 위치가 Active range 안
AND ECC_Visibility 직접 가시
→ CurrentScanAttemptProgress01에 AnalysisGainPerSec 증가
```

따라서 broad Active Detection으로 여러 Contact가 `Live`가 되더라도 동시에 여러 대상의 Knowledge progress가 오르지 않는다. 건물 뒤 지정 Target은 Contact 자체가 Live일 수 있어도 direct LOS가 없으면 Scan Attempt progress가 감소한다.

Tactical Analysis가 활성인 유효 SensorConfig는 정상 Target이 Active range와 직접 LOS를 Scan 전체에 유지할 경우 Targeted Scan Attempt 1회 안에 `DetailedScanThreshold`까지 도달할 수 있어야 한다. 현재 Basic Sensor는 `Duration 3.0초 / Update 0.2초 / Gain 0.40/s / Detailed 1.0` 조합으로 이 시간 예산 계약을 만족한다.

진행 중 지정 Target의 분석 조건이 유효하지 않을 때:

```text
FCFSensorScanAttempt.Progress01
→ AnalysisDecayPerSec로 서서히 감소
→ Attempt가 계속 살아 있는 동안 즉시 0 reset 안 함
```

사용자가 `CancelSensorOperations()`로 명시 취소하거나 Scan duration이 만료되거나 DetailedScan 완료가 발생하면 **Attempt 자체를 종료**하므로 `bScanning=false`, `TargetContactId=None`, `Progress01=0`으로 즉시 복귀한다. 획득한 Contact Knowledge는 Attempt 종료와 독립적으로 유지된다.

Contact Knowledge 완료 이력과 현재 Scan Attempt 상태는 서로 다른 계약이다.

```text
FCFSensorContact.AnalysisCompletionRevision
= 해당 Contact가 최초 DetailedScan Knowledge를 획득한 영구 이력
= DetailedScan Contact에서는 양수값 유지

FCFSensorScanAttempt
= 현재 시도 중인지, 어느 Contact인지, 현재 progress가 얼마인지 나타내는 일시 상태

FCFSensorScanAttempt.CompletionTransitionRevision
= 새 DetailedScan 완료 전이를 HUD 같은 Presentation consumer가 한 번 감지하기 위한 Component 수명 단조 증가 Revision

FCFSensorScanAttempt.LastCompletedContactId
= 가장 최근 완료 전이가 발생한 ContactId
```

공개 Contact 계약은 `DetailedScan ↔ AnalysisCompletionRevision > 0`을 fail-closed로 요구한다. Detected/Identified Contact는 Revision 0을 유지한다.

Knowledge 승격:

```text
Attempt progress < IdentifiedThreshold
→ Detected
→ KnownTargetId / Name 비공개

Attempt progress >= IdentifiedThreshold
→ Identified
→ 유효한 private Source TargetId 공개 가능
→ DisplayName은 명시 Player-facing source가 있을 때만 공개

Attempt progress >= DetailedScanThreshold
→ DetailedScan
→ Contact AnalysisCompletionRevision 최초 발급
→ ScanAttempt CompletionTransitionRevision 발급
→ Scanner Runtime 즉시 Idle
→ ScanAttempt inactive / progress 0
```

TargetPanel 표시 규칙:

```text
현재 ScanAttempt.bScanning
→ ScanAttempt.Progress01을 `스캔 XX%`로 표시

새 CompletionTransitionRevision 감지
→ 같은 Contact에서 `스캔 완료` + 100%를 약 0.75초 표시

0.75초 경과
→ Scan Text / Bar 숨김
→ DetailedScan Knowledge 자체는 계속 유지
```

따라서 스캔 완료 뒤 UI가 100%에 영구 고정되거나 97% → 94%처럼 역행하지 않는다. 완료 UI는 **짧은 완료 feedback**일 뿐이고, 실제 정보 획득 상태는 `DetailedScan` Knowledge가 소유한다.

Source TargetId가 `None`이면 threshold를 넘었다는 이유만으로 잘못된 Known identity를 만들지 않는다.

차량의 현재 native Identity source는 `ACFVehiclePawn::VehicleData`의 유효한 `PrimaryAssetId.PrimaryAssetName`이다. 이는 Pawn Actor instance 이름과 독립된 차량 타입/모델 Identity이며, 개별 Contact 수명은 계속 `ContactId`가 소유한다. VehicleData가 없거나 PrimaryAssetId가 invalid면 TargetId는 `None`으로 fail-closed한다. 별도 Player-facing 차량 이름 source가 아직 없으므로 차량 `DisplayName`은 Empty를 유지하고 Actor `GetFName()/GetName()`을 TargetId/DisplayName fallback으로 사용하지 않는다.

---

## 13. 차량 파괴와 DestroyedHold

차량 파괴의 authoritative truth는 `UCFVehicleHealthComp`다.

Sensor가 파괴 확정에 사용하는 경로는 다음 두 개뿐이다.

```text
UCFVehicleHealthComp::OnVehicleDestroyed
UCFVehicleHealthComp::IsDestroyed()
```

다음은 Sensor destruction truth가 아니다.

```text
AActor::OnDestroyed
EndPlay
weak Actor invalid
Actor reference 소실
```

기존 Contact의 VehicleHealth 파괴가 확정되면:

```text
Live / LastKnown 등 기존 Contact
→ DestroyedHold

bDestroyedConfirmed = true
```

보존되는 값:

```text
같은 ContactId
KnownTargetId / KnownDisplayName
InformationLevel
AnalysisCompletionRevision
LastKnownWorldLocation
LastObservedWorldTimeSeconds
```

파괴 순간 Actor의 현재 위치나 Impact 위치로 `LastKnownWorldLocation`을 덮어쓰지 않는다.
마지막으로 Sensor가 신뢰했던 위치가 유지된다.

DestroyedHold 보존 시간은 별도 private destruction timestamp를 사용하며 bounded actor cursor와 독립적으로 만료된다.

한 번도 Sensor가 알지 못했던 이미 파괴된 Actor를 발견했다고 새 Destroyed Contact를 생성하지 않는다.

---

## 14. TargetSelect 파괴 수명과의 독립성

TargetSelect는 선택된 대상의 다음 이벤트를 독립적으로 감시한다.

```text
Actor OnDestroyed
Actor OnEndPlay
VehicleHealth OnVehicleDestroyed
```

선택 대상이 파괴되면 TargetSelect는 즉시:

```text
선택 validity false
→ ClearSelectedTarget(Destroyed)
```

를 수행한다.

Sensor는 같은 시점에 기존 Contact를 `DestroyedHold`로 유지할 수 있다.

따라서 현재 정상 결과는 다음과 같다.

```text
Target Panel selection
→ 즉시 해제 가능

Sensor / Radar Contact
→ 같은 ContactId로 DestroyedHold 동안 잠시 유지 가능
```

두 수명을 하나로 합치지 않는다.

---

## 15. Actor-free Public Snapshot 계약

외부 소비자는 `FCFSensorSnapshot`을 기본 공개 계약으로 사용한다.

`FCFSensorContact` 공개 필드:

```text
ContactId
KnownTargetId
KnownDisplayName
TargetCategory
Relation
InformationLevel
AnalysisCompletionRevision
ContactState
LastKnownWorldLocation
LastObservedWorldTimeSeconds
FreshnessSeconds
bDestroyedConfirmed
```

`FCFSensorContact`는 현재 Scan progress를 보유하지 않는다.

`FCFSensorSnapshot` 공개 필드:

```text
Revision
SnapshotWorldTimeSeconds
SensorOriginWorldLocation
SensorForwardWorldDirection
bRuntimeReady
bActiveScanRunning
ScanAttempt
Contacts
```

현재 public Contact/Snapshot에는 다음이 없다.

```text
AActor*
TObjectPtr<AActor>
TWeakObjectPtr<AActor>
UObject*
Gameplay Component pointer
```

Actor 참조와 Source metadata는 `UCFVehicleSensorComp::FCFSensorContactRuntime` private 영역에만 존재한다.

`GetSensorSnapshot()`은 현재 Snapshot 사본을 반환한다.

Public contract 핵심 invariant:

```text
ContactId 유효
ContactId 중복 없음
Contacts 결정 정렬
ScanAttempt.Progress01 = 0~1
ScanAttempt Idle이면 TargetContactId=None / Progress01=0
LastKnown / 시간 값 finite
bDestroyedConfirmed == (ContactState == DestroyedHold)
DetailedScan ↔ AnalysisCompletionRevision > 0
Identified / DetailedScan identity 계약 유효
```

---

## 16. Actor → ContactId association bridge

HUD가 현재 선택 Actor와 Actor-free Snapshot Contact를 연결할 때만 다음 API를 사용할 수 있다.

```text
TryGetContactIdForActor(TargetActor, OutContactId)
```

이 API는 기존 private Runtime Contact를 찾아 **ContactId 하나만** 반환한다.

반환하지 않는 것:

```text
Actor metadata
Actor 현재 위치
Source InformationLevel
private Runtime Contact
Knowledge 원본
```

실제 Player-facing 정보는 반환된 ContactId로 `FCFSensorSnapshot.Contacts`를 다시 찾아 읽는다.

Detected 단계의 `KnownTargetId=None` 계약을 깨지 않으면서 TargetSelect selection과 Sensor Contact를 연결하기 위한 association-only bridge다.

---

## 17. HUD Target ViewData 경계

현재 Target HUD에서 TargetSelect가 제공하는 것은 다음뿐이다.

```text
bHasSelectedTarget
bSelectedTargetValid
TrackState
선택 Actor = ContactId association 용도만
```

Sensor Snapshot이 제공하는 Player Knowledge:

```text
ContactId
KnownTargetId
KnownDisplayName
Relation
Category
InformationLevel
AnalysisCompletionRevision
ContactState
FreshnessSeconds
bDestroyedConfirmed
DistanceMeters 계산용 LastKnownWorldLocation
```

Phase 6부터 현재 Target Scan UI progress는 Contact 또는 현재 Selection이 아니라 독립 `Snapshot.ScanAttempt`에서 `FCFTargetScanHUDData`로 별도 전달한다. Scan Attempt가 A를 캡처한 뒤 Selection이 B로 변경되거나 Clear되어도 실제 Attempt가 살아 있는 동안 Target Scan Presentation은 A를 유지한다.

Production HUD Provider는 다음 경로를 사용하지 않는다.

```text
GetSelectedTargetDisplayInfo()
GetTargetDisplayInfo()
선택 Actor GetActorLocation()
PassiveDetectionRangeCm
VisualDetectionRangeCm
ActiveScanRangeCm
```

따라서 UI가 Actor truth나 Sensor tuning을 사용해 Gameplay Knowledge를 다시 계산하지 않는다.

Phase 6의 Vehicle Target Lock identity 보강도 같은 원칙을 따른다. `FCFTargetingSnapshot.TargetContactId`를 같은 Refresh의 `FCFSensorSnapshot.Contacts`에서 찾아 공개 가능한 `KnownDisplayName`만 `FCFTargetLockHUDData`에 복사하며 `GetLockedTargetActor()`나 Actor 이름을 HUD truth로 사용하지 않는다.

---

## 18. Radar ViewData 경계

Sensor Runtime이 준비된 경우 Radar availability는 다음과 같다.

```text
Snapshot ready + Contact 0개
→ KnownZero

Snapshot ready + Contact 1개 이상
→ Known

Sensor 없음 / Runtime 미준비 / invalid Snapshot
→ Unavailable
```

Radar Contact가 Snapshot에서 받는 현재 정보:

```text
ContactId
Relation
Category
InformationLevel
ContactState
FreshnessSeconds
bDestroyedConfirmed
bSelected
```

Radar Contact는 Target Scan progress를 보유하지 않는다.

`bSelected`는 TargetSelect의 현재 선택 Actor를 ContactId로 association한 결과만 사용한다.
선택 의미 자체는 TargetSelect가 계속 소유한다.

---

## 19. Radar 실제 상대 위치와 NormalizedPosition

HUD Provider는 Actor 현재 위치가 아니라 Snapshot 값만 사용한다.

```text
SensorOriginWorldLocation
SensorForwardWorldDirection
LastKnownWorldLocation
```

으로 다음을 계산한다.

```text
RelativePositionMeters.X
= Sensor 전방 기준 실제 전후 거리 m

RelativePositionMeters.Y
= Sensor 우측 기준 실제 좌우 거리 m

DistanceMeters
= Sensor origin → LastKnown 3D 실제 거리 m
```

현재 Radar 표시 반경과 Zoom 계약은 아직 없다.

따라서 다음과 같이 Sensor 사거리를 Radar UI 반경으로 재해석하지 않는다.

```text
PassiveDetectionRangeCm → Radar Radius  X
VisualDetectionRangeCm  → Radar Radius  X
ActiveScanRangeCm       → Radar Radius  X
```

현재 계약:

```text
RelativePositionMeters = 사용 가능
DistanceMeters = 사용 가능
NormalizedPositionAvailability = Unavailable
```

Radar Range/Zoom과 -1~1 정규화 위치는 `CF-FQ-032` 시각/UI 작업에서 별도로 정의해야 한다.

---

## 20. Radar Widget placeholder 보호

기존 `WBP_CFRadarPanel`의 `CanvasPanel_RadarContacts` 안에는 실제 Sensor Contact가 아니라 디자인 단계의 정적 placeholder Image가 존재한다.

P0-06 이후 Sensor Radar availability가 `Known`이 될 수 있지만, 그 이유만으로 placeholder를 보이면 가짜 Contact가 된다.

따라서 현재 `UCFHUDPresenter`는:

```text
Radar Known / KnownZero
→ 제목 "RADAR"

Radar Unavailable
→ 제목 "RADAR — UNAVAILABLE"

CanvasPanel_RadarContacts
→ 계속 숨김
```

으로 처리한다.

실제 동적 Contact Widget 생성과 위치 배치는 아직 Current System이 아니다.

---

## 21. Debug / 진단 계약

현재 Sensor Component는 다음 read-only 진단을 제공한다.

```text
GetLastSensorRuntimeSummary()
GetLastPassiveScanActorCount()
GetLastPassiveVisibilityTraceCount()
GetLastActiveAnalysisTraceCount()
GetPassiveScanCycleSerial()
```

이 값은 bounded scan과 LOS 비용을 확인하기 위한 진단 정보다.
Gameplay Sensor 성능 수치로 사용하지 않는다.

---

## 22. 현재 검증 기준선

### 22.1 Official UE 5.8 Build

```text
Build Job: 7dff9da7aaa24e76b0871348762c9d93
Result: PASS
Exit Code: 0
```

해당 Build에서 현재 Sensor/HUD integration Source가 UHT, Compile, Link를 통과했다.

### 22.2 Sensor 전체 Automation

```text
Process: 85d008613a104df9b7107795995c6ac5
Filter: CarFight.Sensor
Success: 14
Failure: 0
Engine Exit Code: 0
```

검증 범위:

```text
SEN-P0-01 DataContract / PawnOwnership / RuntimeContract
SEN-P0-02 PassiveRange / VisualFallback / BoundedScan
SEN-P0-03 ContactLifetime / Reacquire
SEN-P0-04 ActiveRange / AnalysisProgress / AnalysisDecay
SEN-P0-05 DestroyedHold / InvalidActor
SEN-P0-06 HUDSnapshot
```

### 22.3 HUD Provider asset-free 보호 회귀

```text
CarFight.UI.UI_P0_03.ViewDataAvailability
Process: 67d114255ec943bb8ded2642a40a581c
1/1 PASS

CarFight.UI.UI_P0_03.ProviderPawnRebind
Process: 395c93df5fb947a1a979ca8fa0c74665
1/1 PASS
```

### 22.4 SEN-P0-07 final static audit

최종 Source 감사에서 다음을 확인했다.

```text
Public FCFSensorContact / FCFSensorSnapshot Actor/UObject pointer = 0
TargetSelect → Sensor dependency = 없음
Sensor → TargetSelect selection mutation = 없음
Sensor LOS = ECC_Visibility
TargetSelect LOS = CFCollisionChannels::TargetSelect
Sensor Actor OnDestroyed / OnEndPlay destruction truth 사용 = 없음
TargetSelect VehicleHealth Destroyed 즉시 clear = 유지
HUD Provider GetSelectedTargetDisplayInfo 사용 = 0
HUD Provider GetTargetDisplayInfo 사용 = 0
HUD Provider 선택 Actor GetActorLocation 사용 = 0
HUD Provider Sensor range로 Radar scale 계산 = 0
HUD Provider SetSelectedTarget / 후보 refresh = 0
```

P0-06 최종 Build 이후 Source 의미 변경 없이 SEN-P0-07은 read-only audit와 문서 승격만 수행했다.
따라서 Technical Acceptance를 위해 Build/Automation을 습관적으로 재실행하지 않았다.

---

## 23. Technical Acceptance 판정

`SEN-P0-07` 판정:

```text
Sensor Foundation                     PASS
Actor-free public data contract       PASS
bounded Passive / Visual Detection    PASS
Contact lifecycle                     PASS
Active Scan Detection                 PASS
Tactical Analysis / Knowledge         PASS
DestroyedHold                         PASS
TargetSelect ownership isolation      PASS
HUD read-only Snapshot integration    PASS
P0 regression                         PASS
Content Asset mutation protection     PASS
USER checkpoint protection            PASS
```

결론:

```text
CF-FQ-036 / SEN-P0-00~07
P0 Technical Acceptance = PASS
```

`CF-FQ-036`은 Sensor Runtime 기술 기능으로 Done 처리할 수 있다.

이 판정은 다음 USER 작업을 대신하지 않는다.

```text
CF-FQ-032 Radar / TargetPanel USER Visual
Radar Range / Zoom
동적 Radar Blip
CF-FQ-026 TS-P0-08 USER PIE
```

---

## 24. 현재 비책임 / 후속 확장 지점

### Scanner Input / Equipment Integration

`CF-FQ-037`에서 Current로 승격됐다.

```text
입력 owner = ACFVehiclePawn
Runtime owner = UCFVehicleSensorComp
장비/Fitting adapter = UCFVehicleFittingComp
정적 Scanner payload = UCFVehicleSensorData
```

Scanner test fixture에서 `V` 단발 입력, 5초 timed Active Scan 자동 종료, 활성 중 반복 입력 무연장과 Target Knowledge 승격(`???` 해제)을 USER PIE로 확인했다. scanner-less safe reject는 기존 Scanner Runtime/Fitting 기술 회귀로 보호한다.

### Sensor tuning Content Asset

`UCFVehicleSensorData`와 Scanner test fixture는 Current 검증 경로지만, Production Scanner 등급별 최종 게임플레이 밸런스·질량 수치는 아직 확정하지 않았다.


### Radar 시각화

실제 Range/Zoom, clipping, edge indication, blip icon/scale/declutter는 CF-FQ-032 책임이다.

### TargetSelect 통합

현재 TargetSelect full-world candidate search를 Sensor Snapshot 기반 후보 검색으로 교체하지 않는다.
필요성이 실제 성능/게임플레이 evidence로 확인될 때 별도 Feature에서 다룬다.

### Network

현재 Snapshot은 향후 Network 소비를 고려해 Actor-free로 설계됐지만 replication 자체는 구현하지 않았다.

### AI / Utility / Missile

현재 public Snapshot은 향후 소비 가능한 기반이지만 AI, Utility, Missile이 Contact를 실제 gameplay source로 사용하도록 연결하는 작업은 별도 Feature 책임이다.

---

## 25. 문서 갱신 조건

다음 변경이 생기면 이 문서를 함께 갱신한다.

```text
- FCFSensorConfig 의미나 Validation 변경
- FCFSensorContact / FCFSensorSnapshot 공개 필드 변경
- Passive / Visual / Active Detection 의미 변경
- Contact lifecycle 변경
- Knowledge threshold 또는 promotion 변경
- destruction truth owner 변경
- TargetSelect와 Sensor 책임 경계 변경
- HUD가 Snapshot을 소비하는 방식 변경
- Radar Range / Zoom이 Current System으로 확정
- SensorData 대표 Content Asset과 실제 tuning이 Current로 확정
- Network replication 또는 AI/Utility 소비가 Sensor 핵심 계약을 바꿈
```

---

## 26. Migration

### v1.12.0 -> v1.13.0

- valid `TargetEntityId`를 가진 Contact가 `Identified` 또는 `DetailedScan`으로 승격되면 `UCFVehicleSensorComp` private actor-free Persistent Knowledge Store에 즉시 같은 Knowledge를 upsert한다. Invalid Guid 대상은 기존 Scan 호환을 유지하지만 Store key를 추측 생성하지 않는다.
- `Removed` 뒤 동일 Entity가 다시 관측되면 Store identity/Terminal 검사를 **새 Runtime Contact admission 전에** 수행한다. 통과하면 새 ContactId를 발급하고 Persistent Fact를 복원하며, concrete stable TargetId/TargetCategory 충돌 또는 Terminal Entity 재등장은 fail-closed한다.
- DetailedScan reassociation은 Store에 보존한 기존 양수 `AnalysisCompletionRevision`을 exact 복원하고 `NextAnalysisCompletionRevision`, `CompletionTransitionRevision`, Scan Attempt progress를 증가/복원하지 않는다.
- `ApplySensorData()`와 `ApplyVehicleBaseSensorData()` hot reapply는 Store를 보존한다. `InitializeSensorRuntime()` 재호출과 `ResetSensorRuntime()`은 Hard Reinitialize/Reset으로 Store를 비운다.
- authoritative VehicleHealth destruction은 이미 Sensor가 알고 있던 valid Entity Contact에 최소 Terminal Record를 upsert하며 DestroyedHold Contact가 제거된 뒤에도 Store를 유지한다. 미관측 pre-destroyed Actor만으로 ghost Store를 만들지 않는다.
- Phase 3B-1은 Persistent Fact와 Terminal만 구현한다. HP/Loadout/Defense 같은 Dynamic Knowledge Domain, Unknown/Fresh/Stale, TTL, Rescan/Refresh API는 Phase 3B-2까지 추가하지 않는다.

### v1.8.0 -> v1.9.0

- 플레이어 V 입력은 broad `StartActiveScan()`이 아니라 현재 TargetSelect의 선택 Actor 하나를 `StartTargetedScan()`으로 전달한다. 선택 없음, invalid Target, 이미 DetailedScan 완료된 동일 Contact는 fail-closed한다.
- 현재 진행 상태는 Contact의 legacy `AnalysisProgress01`이 아니라 `FCFSensorScanAttempt.Progress01`이 소유한다. LOS/범위 이탈 중에는 Attempt progress가 decay할 수 있지만 Stop/만료/완료로 Attempt가 끝나면 즉시 inactive + progress 0이다.
- 최초 `DetailedScan` Knowledge 이력 `AnalysisCompletionRevision`은 Contact에 유지하고, 별도 `CompletionTransitionRevision + LastCompletedContactId`를 ScanAttempt에 두어 완료 Presentation을 1회 전이로 전달한다.
- Target HUD는 active Attempt 동안만 `스캔 XX%`를 표시하고, 새 completion transition은 약 0.75초 `스캔 완료` + 100%로 표시한 뒤 Scan UI를 숨긴다. DetailedScan Knowledge와 완료 UI 수명은 분리된다.
- v1.9.0 fresh full Build는 unrelated untracked `EquipmentAuthoring` compile error와 실행 중 Editor DLL 점유 때문에 Exit6로 차단됐고, 새 binary가 생성되지 않아 focused Automation은 실행하지 않았다. Historical v1.8.0 PASS를 새 구현의 PASS로 재사용하지 않는다.

### v1.7.0 -> v1.8.0

- `AnalysisProgress01`과 최초 DetailedScan 완료 이력을 분리하기 위해 `AnalysisCompletionRevision`을 추가했다.
- 공개 `FCFSensorContact` 계약은 `DetailedScan ↔ AnalysisCompletionRevision > 0`을 요구하며 Detected/Identified는 Revision 0을 유지한다.
- v1.8.0 당시 Target HUD는 DetailedScan 완료 후 `스캔 완료` + 100%를 유지했으나, 이 terminal latch 정책은 v1.9.0에서 폐기됐다.
- Player-facing 차량 DisplayName source는 추가하지 않았다. 내부 Actor/Asset 이름 fallback 금지와 Vehicle Authoring ownership 경계를 유지한다.

### v1.6.0 -> v1.7.0

- `DA_VehicleSensor_Basic.AnalysisGainPerSec`는 `0.25/s`에서 `0.40/s`로 교정했다. 기존 0.25/s는 `3초 × 0.25/s < DetailedScanThreshold 1.0`이므로 단발 Active Scan 완료 계약을 만족하지 못한다.
- `AnalysisGainPerSec > 0`인 SensorConfig/DataAsset은 `Gain * max(Duration - UpdateInterval, 0) >= DetailedScanThreshold` cross-field invariant를 만족해야 한다. 단일 필드 범위가 정상이어도 이 관계가 깨지면 invalid로 fail-closed한다.
- scanner-less zero-range fallback은 Detection/Analysis가 모두 비활성인 `gain=0` 의미를 유지하며 유효하다.
- 의도적인 multi-pass 분석을 원하면 숫자 조합으로 우회하지 않고 별도 gameplay mode/계약을 먼저 정의한다.

### v1.3.0 -> v1.4.0

- 신규 정상 Vehicle Builder record는 `UseProjectBasicSensor`로 시작하고 DefinitionApply 결과가 canonical `DA_VehicleSensor_Basic`을 `VehicleData.DefaultSensorData`에 반영한다.
- 기존 `DefaultSensorData=None` Definition은 Import 시 `ExplicitNone`으로 보존되며 Basic Sensor를 자동 지급하지 않는다.
- Scanner 장착은 Vehicle Basic보다 우선하고 Scanner 제거 시 Vehicle Basic으로 복귀한다. 둘 다 없을 때만 zero-range fallback을 사용한다.
- 명시된 Vehicle base SensorData가 invalid면 Runtime 초기화를 fail-closed하며 이전 base source를 조용히 유지하지 않는다.

### v1.1.0 -> v1.2.0

- CF-FQ-032 post-closure code review에서 차량 Target Identity의 Actor-name fallback을 제거했다.
- `ACFVehiclePawn`은 유효한 `VehicleData.PrimaryAssetId.PrimaryAssetName`만 안정 TargetId로 제공하고 VehicleData/PrimaryAssetId가 없으면 None으로 fail-closed한다.
- Player-facing 차량 DisplayName은 명시 source 전까지 Empty를 유지한다. Sensor는 Empty DisplayName을 내부 Actor 이름으로 보충하지 않는다.
- `ContactId`와 TargetId의 독립 책임, Detected 비공개, Identified public-contract valid KnownTargetId 조건은 유지한다.
- final Sensor broad 14/14와 HUDSnapshot 1/1 PASS로 회귀를 확인했다.



- 완료 이후 Sensor의 현재 구현을 판단할 때 `Document/Plan/Archive/SensorContactPlan.md`의 old next gate보다 이 Current System과 실제 Source를 우선한다.
- `Document/Plan/Archive/SensorContactPlan.md`는 완료 당시 설계·검증 evidence를 보존하는 Historical + Archived Path 문서로 유지한다.
- TargetSelect의 기존 후보 검색·선택 수명은 그대로 유지한다.
- CF-FQ-032의 Radar/TargetPanel USER Visual과 CF-FQ-026 TS-P0-08 USER PIE는 CF-FQ-036/037 Done으로 자동 승격되지 않는다.
- `Document/Plan/Archive/ScannerIntegrationPlan.md`는 CF-FQ-037 완료 당시 fixture RCA·USER Acceptance·P0-07 승격 evidence를 보존하는 Historical + Archived Path 문서다.

- Radar normalized position을 임의의 Sensor range로 계산하지 않는다.
- Sensor public Snapshot에 Actor/UObject pointer를 추가하지 않는다. 그런 요구가 생기면 public contract를 확장하기 전에 별도 구조 검토가 필요하다.

---

## 27. Changelog

### v1.15.0 - 2026-09-18

- Phase 6 HUD Presentation에서 현재 Scan Attempt를 Selection-matched `FCFTargetHUDData`에서 분리하고 `FCFSensorSnapshot.ScanAttempt → FCFTargetScanHUDData` 독립 채널로 Current 계약을 갱신했다.
- Scan A 진행 중 Selection이 B로 변경되거나 Clear되어도 실제 Scan Attempt가 살아 있는 동안 A의 progress/identity가 유지되며, 완료 전이는 `CompletionTransitionRevision + LastCompletedContactId`로 transient feedback만 제공한다.
- Vehicle Target Lock 표시 identity도 `TargetContactId → 같은 Refresh Sensor Snapshot Contact` 경로만 사용하며 Actor truth나 내부 ContactId를 Player-facing 이름으로 노출하지 않는다.
- Phase 6 Provider focused Automation `CarFight.Targeting.Phase6.HUDPresentationChannels` 1/1 PASS와 Production `UI_P0_07.TargetKnowledgePanelContract` 1/1 PASS를 확보했다.
- latest Source Official UE 5.8 Build `f899ebafdb8d43a88e6788561b4c7a8f`와 persisted TargetPanel 후 final Build `608d40838ff1410680a0140d630f1e50`이 모두 PASS했다.

Migration: HUD의 현재 Target Scan 상태는 Selection Target ViewData에서 추정하지 않는다. `FCFTargetScanHUDData`를 독립 소비하고 Persistent `DetailedScan/AnalysisCompletionRevision`은 현재 Scan 진행·완료 feedback과 별개로 유지한다.

### v1.14.0 - 2026-09-18

- Phase 5 Selection / Lock / Scan Gameplay Command boundary를 반영해 `UCFVehicleSensorComp::CancelTargetScan()`을 canonical Target Scan-only 취소 API로 추가했다.
- `CancelTargetScan()`은 활성 Scan Attempt의 Actor/ContactId/progress만 정리하고 broad Active Detection Pulse의 `bActiveScanRunning / ActiveScanRemainingSeconds`를 변경하지 않는다. broad Pulse가 실행 중이면 Target Scan-only Contact를 LastKnown으로 강등하지 않는다.
- Pawn `RequestCancelTargetScan()`을 Scan-only facade로 사용하고 기존 `CancelSensorOperations / RequestCancelSensorOperations`는 Active Detection + Target Scan을 함께 끄는 system/equipment/compatibility broad cancel 의미로 유지한다.
- 기존 `IA_ActiveScan` 자산 이름과 V 매핑은 호환성을 위해 유지하며 V의 canonical 의미는 현재 Selected Target 하나의 `RequestStartTargetScan()`이다. 새 Stop/Lock InputAction이나 키는 추가하지 않았다.
- Phase 5 final Source 기준 Official UE 5.8 Build `8c3d5e3eb4d94be88492d4b55e6c6577` PASS, Targeting `5/5`, TargetSelect `11/11`, Sensor `17/17` PASS를 확인했다. 검증용 Automation runner의 임시 allowlist는 원복되어 final diff exact0이다.

Migration: 새 Gameplay 코드는 Target Scan만 취소해야 할 때 `CancelTargetScan / RequestCancelTargetScan`을 사용한다. `CancelSensorOperations / RequestCancelSensorOperations`를 Scan-only UX 의미로 재사용하지 않는다. Selection 변경/해제는 이미 시작된 Target Scan의 Actor+ContactId capture를 자동 변경하지 않는다.

### v1.13.0 - 2026-09-18

- `UCFVehicleSensorComp` private storage에 valid `TargetEntityId` exact key의 actor-free / UObject-free Persistent Knowledge Store를 추가했다.
- `Identified`와 `DetailedScan` 승격을 Contact와 Store의 같은 전이에서 즉시 upsert하도록 하고, DetailedScan Store Record는 양수 `AnalysisCompletionRevision`을 exact 보존한다.
- Contact가 Lost Snapshot 게시 뒤 Removed되어 Runtime Contact가 없어져도 Store는 유지되며, same Entity 재관측 시 새 ContactId에 Known Identity / 최고 Knowledge Tier / exact DetailedScan revision을 복원한다.
- reassociation은 Scan Attempt progress, completion transition, allocator를 복원·증가시키지 않으며 복원된 DetailedScan Contact의 기존 재스캔 거부 계약을 유지한다.
- Store의 stable identity는 `TargetEntityId + concrete Source TargetId + concrete TargetCategory`로 검증한다. `None→valid`, `Unknown→concrete` 최초 보강은 허용하고 concrete mismatch 또는 Terminal Entity 재등장은 Contact admission 전에 fail-closed한다.
- authoritative Destroyed 기존 Contact는 prior Knowledge 유무와 관계없이 valid Entity에 최소 Terminal Record를 upsert하고 DestroyedHold 제거 뒤에도 보존한다. 미관측 pre-destroyed Actor는 Store를 만들지 않는다.
- Store lifetime은 `ApplySensorData / ApplyVehicleBaseSensorData` hot reapply에서 보존, `InitializeSensorRuntime / ResetSensorRuntime / EndPlay` Hard lifecycle 경계에서 clear로 확정했다.
- Dynamic Knowledge Domain/Freshness/TTL/Rescan은 구현하지 않고 Phase 3B-2 후속으로 남겼다.
- Final Official UE 5.8 Build `bf50dcf60f0c41a5b8549f1222ef0ce2` PASS / Exit0. Final `CarFight.Sensor` process `20a62b05b1a54d95a21a3f4f05cd598e` 17/17 PASS / Failure0 / EngineExitCode0이며 신규 `CarFight.Sensor.SEN_P0_03.PersistentKnowledge`를 포함한다.
- Automation 실행을 위해 clean `Tools/RunUIAutomation.ps1`의 기본 filter를 일시 `CarFight.Sensor`로 바꿨다가 실행 직후 원문으로 복원했으며 최종 runner worktree diff는 0이다.

Migration: 기존 `ContactId`, 표시/데이터 `TargetId`, Sensor Snapshot/HUD 소비 계약은 유지한다. Persistent Store는 private Runtime Authority이며 UI/Radar/TargetSelect가 Store를 직접 열거하지 않는다. Dynamic Knowledge/Rescan Consumer를 현재 구현으로 가정하지 않는다.

### v1.12.0 - 2026-09-18

- `ICFTargetSelectable`에 Gameplay Entity lifetime용 `GetTargetEntityId()` 계약을 추가하고 기본 구현은 Invalid Guid fail-closed로 유지했다.
- `ACFVehiclePawn`과 C++ 테스트 Target은 per-instance FGuid를 같은 Actor lifetime 동안 안정적으로 제공하며, `FCFSensorContact.TargetEntityId`는 Contact 최초 생성 때 캡처되어 Live/LastKnown/Lost/재획득 동안 유지된다.
- 신규 `Execute_GetTargetEntityId()`가 Native C++ override 대신 기본 Invalid Guid로 떨어지는 focused failure를 확인해, 기존 Sensor의 Blueprint 실제 override 우선 / Native `_Implementation()` 직접 호출 resolver 정책에 Entity ID를 추가했다.
- Official UE 5.8 Build `52ef16043f994685bac161b6dac118d8` PASS / Exit0, final `CarFight.Sensor` process `ba870c3a79964323b388fd45a3702e23` 16/16 PASS로 교정 후 회귀를 닫았다.
- v1.12.0 당시에는 Entity Identity foundation까지만 Current였고, Removed Contact 이후 persistent Knowledge store/reassociation은 아직 미구현이었다. 해당 범위는 v1.13.0 Phase 3B-1에서 구현·검증 완료됐다.

Migration: 기존 `ContactId`와 VehicleData 기반 `TargetId` 소비자는 변경하지 않는다. Entity 재식별이 필요한 신규 경로만 `TargetEntityId`를 사용하며, Identity 미지원 Target의 Invalid Guid도 정상 호환 상태로 유지한다.

### v1.11.0 - 2026-09-17

- Phase 2 Fresh Technical Validation PASS: Official UE 5.8 Build `966325479f6e4a05ab75fa350822fa45` Exit0, `CarFight.Sensor` 16/16 PASS, `CarFight.UI.UI_P0_07` 2/2 PASS.
- Phase 2에서 `FCFSensorContact.AnalysisProgress01`을 공개 계약에서 제거하고 Target/Radar HUD ViewData의 동일 legacy passthrough를 함께 제거했다. 현재 Scan progress Authority는 `FCFSensorSnapshot.ScanAttempt.Progress01` 단일 경로다.
- Contact lifecycle은 `ContactId + Knowledge + AnalysisCompletionRevision`을 보존하며 Scan Attempt progress를 Contact 수명에 저장하지 않는다.
- canonical Sensor API를 `StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations`와 역할 명확한 상태 Getter로 정리했다. 구형 `StartActiveScan / StartTargetedScan / StopActiveScan` 계열은 저장된 Blueprint·기존 C++ caller 보호용 Deprecated compatibility wrapper로 유지한다.
- 전체 Sensor update의 canonical 내부 이름을 `RunSensorUpdate()`로 정리하고 `RunPassiveDetectionUpdate()`는 기존 Automation/private caller 호환 wrapper로 축소했다.
- Pawn Gameplay facade를 `RequestStartTargetScan / RequestCancelSensorOperations`로 정리했다. 저장된 `IA_ActiveScan` Content Asset 이름은 호환을 위해 변경하지 않았다.
- Reflection 회귀에서 Contact/Target HUD/Radar HUD의 `AnalysisProgress01` Property 부재를 검증하도록 추가했고 기존 lifecycle/HUD/Scanner test fixture를 새 Knowledge/ScanAttempt Authority에 맞췄다.
- Phase 2 최종 상태는 fresh Build `966325479f6e4a05ab75fa350822fa45`, Sensor 16/16, UI_P0_07 2/2의 최종 코드 이후 증거로 Technical PASS다.

Migration: 새 제품 코드와 신규 Blueprint는 canonical API를 사용한다. 현재 Scan 진행률은 `Snapshot.ScanAttempt.Progress01`, 영구 정보는 `Contact.InformationLevel/AnalysisCompletionRevision`, 완료 Presentation 전이는 `ScanAttempt.CompletionTransitionRevision`으로 읽는다. 구형 API wrapper는 호환 기간 동안만 유지하며 신규 호출을 추가하지 않는다.

### v1.10.0 - 2026-09-17

- Phase 1에서 broad Active Detection Pulse와 Target Scan Attempt의 Runtime state를 분리했다. `bActiveScanRunning / ActiveScanRemainingSeconds`는 broad Detection 전용이며 Target Scan은 독립 Attempt active/duration state를 사용한다.
- `StartTargetedScan()`이 더 이상 broad Active Detection을 암묵적으로 켜지 않도록 교정했고, 선택 Target 하나만 Active range 자격으로 관측·분석한다.
- broad Active Detection과 Target Scan의 동시 실행을 허용하고 각 duration 만료와 DetailedScan completion이 상대 Operation을 자동 종료하지 않도록 분리했다.
- `StopActiveScan()`은 기존 caller 호환을 위해 현재 두 Operation을 함께 명시 취소하는 compatibility command로 유지했다. 이 시점에 남았던 API naming 정리는 v1.11.0 Phase 2에서 canonical API + Deprecated wrapper 구조로 완료했다.
- Fresh Official UE 5.8 Build `53ccf44c56ac4d1dbdd65e76dfcc2e97` PASS / Exit0, `CarFight.Sensor.SEN_P0_04` `d5b25f65bda641b5ba66ad6a0ca5ef0d` 5/5 PASS, `CarFight.Scanner.SCAN_P0_03.InputCommand` `8b8d056f7ee2441bb8b7dd7915d1e9ce` 1/1 PASS를 확인했다.

Migration: v1.10.0 당시에는 Target Scan 실행 여부를 broad Active 상태로 추정하지 않고 `IsTargetedScanRunning()`과 `Snapshot.ScanAttempt`로 분리했다. 이때 남아 있던 `Contact.AnalysisProgress01`과 legacy function naming은 v1.11.0에서 제거/compatibility migration 완료됐다.

### v1.9.0 - 2026-09-17

- Sensor 분석 경로를 broad Active Scan과 분리해 **현재 선택 Target 1개만** Knowledge scan을 진행하는 `StartTargetedScan()` 계약으로 교정했다.
- `FCFSensorScanAttempt`를 공개 Snapshot에 추가해 현재 대상, active 여부, progress, 최근 completion transition을 Contact Knowledge와 분리했다.
- DetailedScan 완료 시 Scanner Runtime이 즉시 Idle로 복귀하고 Attempt progress가 0이 되도록 변경했으며, 이미 완료된 동일 Contact의 즉시 재스캔은 거부한다.
- HUD Provider/Presenter는 current ScanAttempt progress만 진행 UI로 사용하고 새 completion transition을 약 0.75초 `스캔 완료` + 100% feedback으로 한 번 표시한 뒤 숨긴다. DetailedScan Knowledge는 그대로 유지한다.
- Sensor/Input/HUD Automation source를 새 계약에 맞춰 교정했다.
- Fresh Official Build `799a5dc7ca42455589df7fbf9905ee76`은 UHT 13 generated files와 runtime module compile/lib 단계까지 진행했지만 unrelated untracked `EquipmentAuthoring` compile error와 실행 중 Editor의 runtime DLL lock이 함께 발생해 Exit6로 종료됐다. 따라서 v1.9.0 fresh Automation은 아직 미실행이며 Technical Acceptance는 Pending이다.

Migration: `Contact.AnalysisProgress01`을 현재 Scanner UI progress authority로 사용하지 않는다. 진행 상태는 `Snapshot.ScanAttempt.Progress01`, 영구 정보는 `Contact.InformationLevel/AnalysisCompletionRevision`, 완료 알림은 `ScanAttempt.CompletionTransitionRevision`으로 분리한다.

### v1.8.0 - 2026-09-16

- `DetailedScan` 최초 완료를 raw `AnalysisProgress01`과 분리하는 `AnalysisCompletionRevision`을 `FCFSensorContact` 공개 계약에 추가했다. 같은 Component 수명에서 최초 완료마다 단조 증가 양수 Revision을 한 번 발급하며 raw progress decay 이후에도 유지한다.
- `FCFSensorContact::IsPublicContractValid()`는 `DetailedScan ↔ AnalysisCompletionRevision > 0`, Detected/Identified ↔ Revision 0 일관성을 fail-closed로 검증한다.
- Target HUD Provider가 완료 Revision을 ViewData로 전달하고 Presenter는 완료 전 `스캔 XX%`, 완료 후 `스캔 완료` + 100% Bar를 유지한다. 완료 전에 조건이 끊긴 경우의 기존 decay 의미는 변경하지 않았다.
- 차량 Player-facing DisplayName source는 이번 slice에서 추가하지 않았다. Actor instance 이름과 VehicleData Asset 이름 fallback 금지를 유지해 CF-FQ-046 Vehicle Authoring ownership과 충돌하지 않는다.
- fresh persisted AssetDump `adset_v1_d6b3d8267c658bd0a856de77952d8663.a3f327c8be153f255487c151`에서 canonical Basic Sensor `Duration 3.0 / Gain 0.40/s / Decay 0.15/s / Detailed 1.0`과 기존 range/radar 값을 재확인했다.
- Official UE 5.8 Build `416e3c1490d34a8eb0429dcb46824b2b` PASS / `CarFight.Sensor.SEN_P0_04` `122e0b4ca17d418285bec7d01c6b706d` 4/4 PASS / `CarFight.Sensor` `bb4f6b37fe2140178b997e79d2155a0d` 15/15 PASS / `CarFight.UI.UI_P0_07.TargetKnowledgePanelContract` `151164a0304147ef9f49720b7b51f555` 1/1 PASS를 확인했다.

Migration: 완료 여부는 `AnalysisProgress01 == 1` 추정이 아니라 `DetailedScan + AnalysisCompletionRevision > 0` 의미를 사용한다. 완료 전 interrupted progress decay와 완료 뒤 terminal UI latch를 구분한다.

### v1.7.0 - 2026-09-16

- USER PIE에서 Basic Sensor Active Scan이 약 72~75%까지만 상승한 뒤 decay하는 현상을 확인했고, `ActiveScanDurationSec=3.0 × AnalysisGainPerSec=0.25 < DetailedScanThreshold=1.0`인 상호모순 tuning을 원인으로 확정했다.
- canonical `DA_VehicleSensor_Basic`의 `AnalysisGainPerSec`를 `0.40/s`로 교정했다. fresh persisted AssetDump `adset_v1_56062aa1d39bf71e73d5c6a2aab8b52f.f8bb85da2d99f05473bfd023`에서 0.40, Passive 20m / Visual 30m / Active 40m / Duration 3초 / Radar 10·20·40m(기본 20m) 보존을 다시 확인했다.
- `FCFSensorConfig`와 `UCFVehicleSensorData` validation에 단발 Active Scan 완료 cross-field invariant를 추가해 같은 종류의 잘못된 tuning이 개별 필드 유효성만으로 통과하지 못하게 했다.
- 실제 production `DA_VehicleSensor_Basic`을 직접 로드하는 `CarFight.Sensor.SEN_P0_04.BasicSensorSingleScanCompletion`을 추가해 정상 range/LOS 조건에서 Scan 만료 전 100% DetailedScan 도달, Scan 종료 뒤 progress decay 가능, 획득 DetailedScan Knowledge 비강등을 검증했다.
- final diff review에서 `KINDA_SMALL_NUMBER` 이하의 극소 양수 gain이 분석 비활성처럼 취급될 수 있는 우회 가능성을 추가 발견했다. 분석 비활성 의미를 **exact gain 0**으로 고정하고 모든 유한 양수 gain을 단발 완료 예산 검증 대상으로 교정했으며, 극소 양수 regression test를 추가했다.
- 최종 교정 후 Official UE 5.8 Build `e2216c2e595641ec955aef946bbe926f` PASS / Exit0, Basic behavioral exact1 `749d2325d9c9495ba4a19f87c459fff6` 1/1 PASS, `CarFight.Sensor` `03e7b29bfa3245139b776c6b7a329af8` 15/15 PASS, `CarFight.Scanner` `e66799c3cad44c62a374a1dec69699e8` 5/5 PASS를 canonical runner로 재확인했다.
- Scanner final regression 첫 실행에서 `SCAN_P0_03.InputCommand`가 새 invariant에 의해 실패했으나 제품 결함이 아니라 scanner-less fixture가 range/duration만 0으로 내리고 이전 gain 0.5를 남긴 테스트 모순이었다. fixture를 실제 zero-range 의미인 gain0/decay0으로 교정한 뒤 Scanner 5/5 PASS로 종료했다.
- 이 behavioral Automation은 transient Editor World에서 제품 Runtime을 직접 검증한 Technical evidence다. 별도 live PIE RuntimeRead는 GoPyMCP infrastructure 상태 때문에 이번 correction에서 수행하지 않았으며 USER visual/feel confirmation과 구분한다.

Migration: Basic Sensor 0.25/s는 폐기값이다. Analysis가 활성인 Sensor는 단발 Scan 완료 cross-field invariant를 만족해야 하며 multi-pass가 필요하면 별도 명시 계약을 먼저 추가한다.

### v1.6.0 - 2026-09-16

- `DA_TestSUV`와 `DA_VehicleDefense_TestSUV`의 기존 Authoring blocker를 우회하지 않고 교정했다. 기존 `DriveStateConfig` 14개 behavior field를 각 Recipe-private `CFDriveStateProfile`로 exact-copy해 `VehicleSpecific` ownership을 성립시키고, stale `HP_Top_02` socket 의존은 기존 `Top_02` LocalLocation/LocalRotation을 보존한 manual slot(`SocketName=None`)으로 전환했다.
- 두 Target 모두 reviewed Initial Import → Profile bind/adoption → Hardpoint/Sensor field adoption → R3 DefinitionApply 경로로 migration했다. 최종 Target diff는 exact2 `DefaultSensorData` + `HardpointSlots[Top_02].SocketName`만 허용했고 persisted AssetDump에서 canonical `DA_VehicleSensor_Basic`, `Top_02.SocketName=None`, 기존 transform 보존을 확인했다.
- 신규 persisted sidecar는 `DA_Recipe_TestSUV + DA_Profile_TestSUV_DS`와 `DA_Recipe_DefenseSUV + DA_Profile_DefenseSUV_DS` exact4다. 기존 `DA_Recipe_TestSedan`과 pre-existing `DA_Recipe_Wagon`은 보존했다.
- 단발성 SUV migration commandlet/runner는 persisted 검증 후 repository에서 격리 제거했고, 제거 뒤 Official UE 5.8 Build `dbef9176e89a4f849995bc26c40ed8c1` PASS / Exit0를 확인했다.
- fresh Automation은 `CarFight.Scanner` 5/5 PASS, `CarFight.DataAuthoring.DAUTH_P0_08` 47/47 PASS, `CF_FQ_042.VBCUX_P0_02.RecordCreation` 1/1 PASS다.
- live Editor는 managed Ready까지 확인했지만 PIE 시작은 GoPyMCP `carfight.pie.start`의 managed Bridge interpreter identity mismatch로 fail-closed됐고, UE MCP tool schema pagination도 첫 page 이후 cursor invalidation이 확인됐다. 따라서 이번 correction의 live PIE RuntimeRead는 **Product blocker가 아니라 infrastructure blocker로 미검증** 상태다.

Migration: 기존 Sensorless 차량을 일괄 변환하지 않는다. 이번 reviewed migration은 `DA_TestSedan / DA_TestSUV / DA_VehicleDefense_TestSUV`에 한정되며 Wagon과 테스트/Legacy 자산은 별도 결정 전 기존 의미를 유지한다.

### v1.5.0 - 2026-09-16

- 기존 VehicleData migration audit를 fresh persisted AssetDump 기준으로 수행했다. `DA_TestSedan`은 Vehicle Authoring Preview/Approval/DefinitionApply 정식 경로로 canonical `DA_VehicleSensor_Basic`을 `DefaultSensorData`에 저장했고 fresh readback PASS다.
- `DA_TestSUV`와 `DA_VehicleDefense_TestSUV`는 Sensor diff 자체는 `DefaultSensorData` exact1이지만 기존 Managed Authoring blocker `RequiredProfileMissing(DriveState)` + `HardpointSocketMissing(Top_02 / HP_Top_02)` 때문에 DefinitionApply를 우회하지 않고 HOLD했다. 두 Target은 fresh readback에서 `DefaultSensorData=None`을 유지한다.
- `DA_Vehicle_Wagon`은 작업 전부터 존재한 `DA_Recipe_Wagon` 병렬 dirty를 보호하기 위해 migration mutation 0으로 제외했고 `DefaultSensorData=None`을 유지한다. `/Tests/**`, `/_Legacy/**`도 migration 대상이 아니다.
- failed TestSUV/DefenseSUV audit가 만든 임시 Recipe는 저장되지 않았고 fresh Authoring Recipe inventory는 `DA_Recipe_TestSedan + DA_Recipe_Wagon` exact2다.
- 단발성 Basic Sensor migration commandlet/runner는 audit 완료 후 repository에서 격리 제거했으며, 제거 뒤 Official UE 5.8 Build `945148c284e04fdea6e9301e66ae764c` PASS / Exit0로 최종 source 상태를 확인했다.

Migration: 기존 `DefaultSensorData=None`을 일괄 Basic Sensor로 승격하지 않는다. 정상 Managed Authoring validation을 통과하는 차량만 reviewed migration하며, 기존 unrelated blocker를 Sensor 작업 명목으로 우회하거나 Raw VehicleData direct edit하지 않는다.

### v1.4.0 - 2026-09-16

- Vehicle Builder 정상 신규 차량의 Basic Sensor baseline 누락을 `VehicleData.DefaultSensorData` + `Scanner override > Vehicle Basic > zero-range Fallback` source priority로 Current System에 반영했다.
- Production `DA_VehicleSensor_Basic` exact1을 materialize하고 fresh persisted AssetDump에서 Passive 20m / Visual 30m / Active 40m / Active Scan 3초 / Radar 10·20·40m 값을 확인했다.
- invalid Vehicle base SensorData는 Runtime 재초기화에서 이전 source를 조용히 유지하지 않고 fail-closed한다.
- Official UE 5.8 Build `8b557312ea864753b4e3ea467de59080` PASS와 correction exact6 process `c5a21f7f44e6465db4d626bb48e58d06` 6/6 PASS를 확인했다.

### Maintenance - 2026-09-02

- 이미 물리 Archive된 CF-FQ-037 Scanner Historical Plan의 stale Retained Path 표기를 `Document/Plan/Archive/ScannerIntegrationPlan.md` Archived Path로 교정했다. Sensor Runtime 계약과 v1.2.0 내용은 변경하지 않았다.

### v1.2.0 - 2026-08-22

- Vehicle Target Identity의 Current source를 `VehicleData.PrimaryAssetId.PrimaryAssetName`으로 명시하고 Actor `GetFName/GetName` fallback 금지와 DisplayName fail-closed 계약을 추가했다.
- Identified Sensor public contract의 유효 KnownTargetId 요구를 유지하면서 Player-facing 내부 UObject 이름 누출을 제거했다.
- CF-FQ-032 remediation final Build `bd3640c616794cd7a54cc8a8afa4b021`, `CarFight.Sensor` broad `dc4ba42db0fc477aa82112a903b2054b` 14/14, HUDSnapshot `10825286b6d14994a31baa0295486aec` 1/1 PASS를 최신 관련 회귀 evidence로 연결했다.

### v1.1.0 - 2026-08-18

- `CF-FQ-037 SCAN-P0-00~07`을 완료해 Scanner Utility Equipment/Fitting Source, non-destructive SensorData Apply, Pawn-owned `IA_ActiveScan`/V command와 scanner-less fallback을 Current System에 통합했다.
- 초기 P0-06 USER FAIL의 원인이 배치 fixture 차량과 실제 GameMode DefaultPawn의 불일치임을 확인하고 test-only `BP_ScanPlayerPawn` + `BP_ScanGameMode` + `TestMap_ScannerP0` override로 교정했다.
- USER PIE에서 Scanner 준비 상태, V 단발 5초 Active Scan, 자동 종료, 활성 중 V 반복 무연장과 선택 Target의 `???` 해제를 확인해 Detection뿐 아니라 Analysis/Knowledge 경로까지 실제 동작함을 확인했다.
- scanner-less safe reject는 기존 Scanner/Fitting 기술 회귀를 Current 보호 evidence로 유지하며 USER가 관측하지 않은 False/0 값을 새 USER PASS로 꾸며 기록하지 않는다.
- USER Acceptance용 임시 DebugPanel 관측 코드는 제거했고 최종 production Build `fcf52353d1f5440392d5e1c379f09ee3` PASS로 closure했다.
- Radar Range/Zoom·동적 Blip·TargetPanel/Radar USER Visual, Sensor energy/heat, AI/Network와 Missile/Utility 소비는 후속 범위로 유지한다.

### v1.0.0 - 2026-08-15


- `CF-FQ-036 SEN-P0-00~07` Technical Acceptance PASS를 기준으로 SensorContact Current System을 최초 작성했다.
- bounded Passive/Visual/Active Detection, Live/LastKnown/Lost/DestroyedHold, Tactical Analysis와 Sensor Knowledge를 현재 구현으로 기록했다.
- VehicleHealth authoritative destruction truth, TargetSelect 즉시 clear와 Sensor DestroyedHold 독립 수명을 기록했다.
- actor-free `FCFSensorContact / FCFSensorSnapshot`, Actor→ContactId association-only bridge와 HUD Target/Radar read-only integration을 기록했다.
- Radar Range/Zoom/NormalizedPosition과 동적 Blip, Scanner Input, 대표 SensorData tuning, Network/AI 소비를 현재 비책임으로 분리했다.
- Build `7dff9da7aaa24e76b0871348762c9d93`, Sensor 14/14와 UI asset-free 보호 회귀를 Technical Acceptance evidence로 고정했다.

---

## 28. 마지막 확인 기준

- 확인 일시: `2026-09-18`

- 현재 Source:
  - `UE/Source/CarFight_Re/Public/CFSensorTypes.h v1.8.0`
  - `UE/Source/CarFight_Re/Private/CFSensorTypes.cpp v1.5.0`
  - `UE/Source/CarFight_Re/Public/CFVehicleSensorData.h v1.3.0`
  - `UE/Source/CarFight_Re/Private/CFVehicleSensorData.cpp v1.3.1`
  - `UE/Source/CarFight_Re/Public/CFVehicleSensorComp.h v1.13.0`
  - `UE/Source/CarFight_Re/Private/CFVehicleSensorComp.cpp v1.15.0`
  - `UE/Source/CarFight_Re/Public/CFTargetSelectable.h v1.2.0`
  - `UE/Source/CarFight_Re/Private/CFTargetSelectable.cpp v1.3.0`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h v2.174.0`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp v2.174.0`
  - `UE/Source/CarFight_Re/Private/UI/CFHUDDataProvider.cpp v1.20.0`
  - `UE/Source/CarFight_Re/Public/UI/CFHUDViewData.h v1.18.0`
  - `UE/Source/CarFight_Re/Private/UI/CFHUDPresenter.cpp` (transient completion feedback 소비)
- 완료 이력: `Document/Plan/Archive/SensorContactPlan.md`
- Current System: `Document/Systems/Targeting/SensorContact.md`
