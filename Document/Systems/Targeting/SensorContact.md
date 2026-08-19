# SensorContact

- Version: 1.1.0
- Date: 2026-08-18
- Status: Current System
- Feature: `CF-FQ-036 차량 센서·Contact Intelligence Runtime` + `CF-FQ-037 차량 스캐너 입력·장비 통합`
- Acceptance: `SEN-P0-00~07 PASS` + `SCAN-P0-00~07 PASS`
- Verification: 최종 UE 5.8 Editor Build `fcf52353d1f5440392d5e1c379f09ee3` PASS / Scanner Fitting 1/1 + Fitting 22/22 + Sensor 14/14 + Inventory 12/12 + TargetSelect RuntimeContract 1/1 PASS / SCAN-P0-06 USER PIE PASS


---

## 1. 문서 목적

이 문서는 CarFight의 현재 차량 Sensor / Contact Intelligence Runtime이 **무엇을 탐지하고, Contact를 얼마나 기억하며, 플레이어가 대상에 대해 어느 정도의 Knowledge를 획득했는지**를 어떤 구조로 소유하는지 기록한다.

현재 구현 판단은 완료 당시 Plan보다 실제 Source와 이 Current System을 우선한다.

```text
현재 구현 기준
= 실제 C++
→ Document/Systems/Targeting/SensorContact.md
→ 관련 TargetSelect / HUD Current 구현
→ Document/Plan/SensorContactPlan.md 완료 이력
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
- bounded World Actor scanning
- Passive Detection
- Visual fallback Detection
- Active Scan Detection
- Live / LastKnown / Lost / DestroyedHold Contact lifecycle
- Lost 전 same-ID reacquire
- Tactical Analysis gain / decay
- Detected / Identified / DetailedScan Sensor Knowledge
- VehicleHealth authoritative destruction signal 소비
- actor-free FCFSensorContact / FCFSensorSnapshot
- TargetSelect 선택 상태와 Sensor Knowledge의 HUD read-only 합성
- Sensor Snapshot 기반 Radar Contact ViewData
- Utility Scanner EquipmentPreset / FittingSnapshot의 `ResolvedSensorData` 해석
- `UCFVehicleFittingComp`의 `ResolvedSensorData → ApplySensorData()` 초기 적용·hot reapply·compensation
- `ACFVehiclePawn`의 `IA_ActiveScan` Enhanced Input command binding과 기본 `V` 매핑
- `V` 1회 → `ActiveScanDurationSec` 실행 → 자동 종료, 활성 중 반복 입력 무연장 계약
- scanner-less 차량의 안전한 disabled/fallback 처리
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

현재 클래스 계약만 구현돼 있고 `CF-FQ-036`에서는 새 SensorData Content `.uasset`을 생성하거나 저장하지 않았다.

### 3.3 `UCFVehicleSensorComp`

차량별 Sensor Runtime의 실제 owner다.

주요 책임:

```text
- Config 해석
- bounded World scan cursor
- private Actor-backed Runtime Contact
- Passive / Visual / Active Detection
- Contact lifecycle
- Tactical Analysis
- Sensor Knowledge 승격
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
| `ActiveScanRangeCm` | `0` | Active Scan 실행 중 장거리 360도 Detection 범위. 0이면 Active Scan 시작 비활성 |
| `VisualDetectionRangeCm` | `0` | Passive 밖에서 직접 가시 대상을 탐지할 후보 범위 |
| `UpdateIntervalSec` | `0.1` | Detection·lifecycle·Analysis 기본 update 간격 |
| `MaxActorScansPerUpdate` | `64` | 한 update에서 검사할 Level Actor 슬롯 CPU 예산, 유효범위 1~4096 |
| `ContactMemoryTimeSec` | `0` | Live 관측 상실 뒤 LastKnown을 유지할 시간 |
| `DestroyedHoldTimeSec` | `0` | 파괴 확정 Contact를 Snapshot에 보존할 시간 |
| `ActiveScanDurationSec` | `0` | 한 Active Scan의 지속 시간 |
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
DetailedScanThreshold >= IdentifiedThreshold
ActiveScanRangeCm = 0은 유효한 비활성 값
ActiveScanRangeCm != 0이면 PassiveDetectionRangeCm 이상
```

---

## 5. 차량 Runtime 연결

`ACFVehiclePawn`은 `UCFVehicleSensorComp`를 기본 Subobject로 소유한다.

현재 초기화는 차량 Runtime 초기화 과정에서 명시적으로 수행한다.
Sensor는 현재 `CoreReady`나 `CombatReady`를 실패시키는 필수 Gate로 사용하지 않는다.

Scanner가 장착된 FittingSnapshot은 `ResolvedSensorData`를 통해 `UCFVehicleSensorComp::ApplySensorData()`로 적용된다. 초기 출격은 Sensor Runtime 초기화 전 Source를 선택하고, Field Fitting hot reapply는 Contact/Knowledge/Analysis를 지우는 `InitializeSensorRuntime()` 재호출 없이 non-destructive Apply 경로를 사용한다.

SensorData가 없거나 유효하지 않은 scanner-less 차량은 Component의 안전한 `FallbackSensorConfig`를 유지한다. 기본 거리값이 0이므로 Sensor range가 없는 차량에서는 Active Scan 시작이 거부되고 불필요한 World Detection Tick을 계속 돌리지 않는다.


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

공개 Runtime command:

```text
StartActiveScan()
StopActiveScan()
IsActiveScanRunning()
GetActiveScanRemainingSeconds()
```

현재 입력 흐름:

```text
/Game/CarFight/Input/IA_ActiveScan
+ IMC_Vehicle_Default : V
→ ACFVehiclePawn Enhanced Input Started
→ RequestStartActiveScan()
→ UCFVehicleSensorComp::StartActiveScan()
→ ActiveScanDurationSec 동안 실행
→ 자동 종료
```

활성 중 `V` 반복 입력은 남은 시간을 초기화하거나 연장하지 않는다. 별도 기본 Stop 키는 없고 `RequestStopActiveScan()`은 시스템·장비 전환용 명시적 command로만 유지한다.


Active Scan Contact Detection:

```text
bActiveScanRunning
AND Distance <= ActiveScanRangeCm
→ 360도 Contact Detection
→ LOS 불필요
```

따라서 건물 뒤 또는 차량 뒤쪽 대상도 Active range 안이면 Contact 자체는 Live로 유지할 수 있다.

Active Scan 종료 시 마지막 관측이 Active-only였던 Contact는 `LastKnown`으로 전환한다.
Passive/Visual baseline도 유효했던 Contact는 `Live`를 유지한다.

---

## 10. ContactId와 Contact 생성

`ContactId`는 Target의 `TargetId`와 다른 Sensor 전용 식별자다.

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
LastKnownWorldLocation 갱신
LastObservedWorldTimeSeconds 갱신
FreshnessSeconds = 0
Knowledge 유지
AnalysisProgress 유지
```

### 11.2 LastKnown

현재 관측은 실패했지만 마지막 신뢰 정보를 기억한다.

보존:

```text
ContactId
KnownTargetId / KnownDisplayName
InformationLevel
AnalysisProgress01
LastKnownWorldLocation
LastObservedWorldTimeSeconds
```

LastKnown 위치는 Actor의 현재 위치를 추정해 따라가지 않는다.

### 11.3 Lost

ContactMemory가 만료된 상태다.

Lost는 최소 한 번 Actor-free public Snapshot에 포함된다.
그 다음 Sensor update에서 제거된다.

Lost가 제거되기 전에 같은 Actor를 재획득하면 기존 ContactId와 Knowledge/AnalysisProgress를 유지한 `Live`로 복귀한다.

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
Active Scan 실행 중
AND Contact == Live
AND Actor 유효
AND TargetSelectable 자격 유효
AND 현재 실제 위치가 Active range 안
AND ECC_Visibility 직접 가시
→ AnalysisGainPerSec 증가
```

따라서 건물 뒤 Contact는 Active Scan 덕분에 `Live`일 수 있지만 Tactical Analysis는 증가하지 않는다.

분석 조건이 유효하지 않을 때:

```text
AnalysisProgress01
→ AnalysisDecayPerSec로 서서히 감소
→ 즉시 0 reset 안 함
```

획득한 Knowledge는 progress가 감소해도 강등하지 않는다.

Knowledge 승격:

```text
progress < IdentifiedThreshold
→ Detected
→ KnownTargetId / Name 비공개

progress >= IdentifiedThreshold
→ Identified
→ private Source TargetId / DisplayName 공개 가능

progress >= DetailedScanThreshold
→ DetailedScan
```

Source TargetId가 `None`이면 threshold를 넘었다는 이유만으로 잘못된 Known identity를 만들지 않는다.

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
AnalysisProgress01
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
ContactState
LastKnownWorldLocation
LastObservedWorldTimeSeconds
FreshnessSeconds
AnalysisProgress01
bDestroyedConfirmed
```

`FCFSensorSnapshot` 공개 필드:

```text
Revision
SnapshotWorldTimeSeconds
SensorOriginWorldLocation
SensorForwardWorldDirection
bRuntimeReady
bActiveScanRunning
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
AnalysisProgress01 = 0~1
LastKnown / 시간 값 finite
bDestroyedConfirmed == (ContactState == DestroyedHold)
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
ContactState
FreshnessSeconds
AnalysisProgress01
bDestroyedConfirmed
DistanceMeters 계산용 LastKnownWorldLocation
```

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
AnalysisProgress01
bDestroyedConfirmed
bSelected
```

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

- 완료 이후 Sensor의 현재 구현을 판단할 때 `Document/Plan/SensorContactPlan.md`의 old next gate보다 이 Current System과 실제 Source를 우선한다.
- `SensorContactPlan.md`는 완료 당시 설계·검증 evidence를 보존하는 Historical + Retained Path 문서로 유지한다.
- TargetSelect의 기존 후보 검색·선택 수명은 그대로 유지한다.
- CF-FQ-032의 Radar/TargetPanel USER Visual과 CF-FQ-026 TS-P0-08 USER PIE는 CF-FQ-036/037 Done으로 자동 승격되지 않는다.
- `ScannerIntegrationPlan.md`는 CF-FQ-037 완료 당시 fixture RCA·USER Acceptance·P0-07 승격 evidence를 보존하는 Historical + Retained Path 문서다.

- Radar normalized position을 임의의 Sensor range로 계산하지 않는다.
- Sensor public Snapshot에 Actor/UObject pointer를 추가하지 않는다. 그런 요구가 생기면 public contract를 확장하기 전에 별도 구조 검토가 필요하다.

---

## 27. Changelog

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

- 확인 일시: `2026-08-18`

- 현재 Source:
  - `UE/Source/CarFight_Re/Public/CFSensorTypes.h v1.2.1`
  - `UE/Source/CarFight_Re/Public/CFVehicleSensorData.h v1.1.0`
  - `UE/Source/CarFight_Re/Public/CFVehicleSensorComp.h v1.5.0`
  - `UE/Source/CarFight_Re/Private/CFVehicleSensorComp.cpp v1.5.0`
  - `UE/Source/CarFight_Re/Private/UI/CFHUDDataProvider.cpp v1.6.0`
  - `UE/Source/CarFight_Re/Public/UI/CFHUDViewData.h v1.4.0`
- 완료 이력: `Document/Plan/SensorContactPlan.md`
- Current System: `Document/Systems/Targeting/SensorContact.md`
