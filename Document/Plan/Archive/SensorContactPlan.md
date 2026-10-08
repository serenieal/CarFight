# CF-FQ-036 차량 센서·Contact Intelligence Runtime Plan

- 문서 버전: v0.9.0
- 최근 갱신일: 2026-08-15
- 문서 상태: Historical + Retained Path / SEN-P0-07 Technical Acceptance PASS
- Feature: `CF-FQ-036 차량 센서·Contact Intelligence Runtime`
- 현재 단계: `SEN-P0-00~07 Complete / CF-FQ-036 Done`
- 구현 상태: Current owner = `Document/Systems/Targeting/SensorContact.md v1.0.0` / 이 Plan은 구현·검증·Acceptance evidence를 보존하는 완료 기록
- 대표 목표: 실제 Sensor Runtime이 탐지한 Contact와 플레이어가 획득한 Target Knowledge를 하나의 읽기 가능한 Snapshot으로 제공

---

## 1. 목적

현재 CarFight에는 `TargetSelect`, Radar/Target HUD ViewData와 `Detected / Identified / DetailedScan` 정보 단계가 존재하지만 **실제 Sensor Runtime Provider는 없다.**

현재 HUD 계약도 Sensor Provider가 없으면 Radar를 `Unavailable`로 유지하고 Target 후보를 가짜 Radar Contact로 만들지 않도록 명시한다.

CF-FQ-036의 목적은 이 빈 계층을 실제 Gameplay Runtime으로 구현하는 것이다.

```text
World Actor / Targetable Actor
→ Vehicle Sensor Runtime
→ Contact Runtime + Player Knowledge
→ Read-only Sensor Snapshot
→ TargetSelect / HUD / Missile / Utility / 향후 AI·Network 소비
```

핵심 책임 분리:

```text
TargetSelect
= 무엇을 선택했는가

Sensor / Contact Intelligence
= 무엇을 탐지했고 그 대상에 대해 현재 얼마나 알고 있는가

HUD
= 위 Runtime이 공개한 의미 데이터를 표시
```

TargetSelect의 선택 수명과 후보 정렬을 Sensor가 빼앗지 않는다.

---

## 2. 현재 확인된 선행 기반

### TargetSelect

현재 `CFTargetSelectTypes.h`에는 이미 다음 공용 의미 타입이 있다.

```text
ECFTargetRelation
- Unknown
- Friendly
- Neutral
- Hostile

ECFTargetInfoLevel
- None
- Detected
- Identified
- DetailedScan

ECFTargetTrackState
- Invalid
- Visible
- Occluded
- Estimated
- SignalLost
```

`ECFTargetTrackState`는 현재 TargetSelect의 **추적 품질** 의미다.
Sensor Contact 수명인 `Live / LastKnown / Lost / DestroyedHold`와 같은 enum으로 합치지 않는다.

### HUD

현재 `CFHUDViewData`에는 이미 다음 준비 계약이 있다.

```text
FCFRadarContactHUDData
FCFRadarHUDData
FCFTargetHUDData.InformationLevel
FCFTargetHUDData.TrackState
```

현재 `CFHUDDataProvider`는 실제 Sensor Provider가 없으므로 Radar를 `Unavailable`로 유지한다.

CF-FQ-036은 이 빈 Runtime Provider를 만드는 기능이며, UI가 Gameplay Actor를 직접 검색하도록 만들지 않는다.

---

## 3. 사용자 확정 설계 결정

현재까지 승인된 Sensor/Radar/Target Intelligence 의미 계약은 다음과 같다.

### Passive / Active

```text
Passive Tracking
= 근거리 기본 탐지
= 상시 사용 가능한 낮은 비용의 Contact 유지 기반

Active Scan
= Scanner 성능에 따라 Passive보다 더 먼 거리까지 탐지·분석
= 전방향 탐지
= 상시 Sweep이 아니라 Scan 실행 중에만 Sweep 의미 사용
```

### 직접 가시 대상

플레이어가 실제로 명확하게 볼 수 있는 가시 범위의 대상은 Sensor 성능 수치 때문에 존재 자체를 놓치는 방향으로 만들지 않는다.
정확한 Visual Detection Range·LOS 판정·성능 비용은 `SEN-P0-00~02`에서 현재 월드 규모와 Targetable 계약을 확인한 뒤 잠근다.

### Contact lifecycle

```text
Live Contact
→ Last Known
→ Contact Lost
```

대상이 잠깐 범위를 벗어나거나 건물 뒤로 숨었다고 즉시 모든 정보를 삭제하지 않는다.

`LastKnown`은 대상의 현재 실제 위치를 추정해 진실처럼 제공하는 상태가 아니다.
마지막으로 신뢰 가능했던 위치·정보의 시점과 freshness를 보존하는 상태다.

### Tactical Analysis

Active Scan / Tactical Analysis 도중 대상이 잠시 범위를 벗어나거나 가려지면:

```text
진행률 즉시 초기화 금지
→ 서서히 감소
→ 다시 유효해지면 남은 진행률에서 재개
```

정확한 증가율·감소율·승격 threshold는 Data/Config로 분리한다.

### Information Level

기존 공용 enum을 우선 재사용한다.

```text
Detected
→ Identified
→ DetailedScan
```

새 계층이 필요해도 기존 의미와 중복 enum을 만들기 전에 현재 소비자를 먼저 감사한다.

### Destroyed Contact

파괴된 대상은 같은 프레임에 즉시 사라지는 Contact로 만들지 않는다.
Target Panel/Radar가 파괴 확인 결과를 잠시 표현할 수 있도록 `DestroyedHold` 계층을 설계하되, **실제 유지시간과 선택 해제 순서는 기존 TargetSelect 파괴 수명과 충돌하지 않게 `SEN-P0-00`에서 확정**한다.

---

## 4. 권장 C++ / BP 책임 분리

### C++ Runtime

권장 중심 타입:

```text
UCFVehicleSensorComp
FCFSensorContact
FCFSensorSnapshot
FCFSensorConfig 또는 UCFVehicleSensorData
```

최종 이름은 `SEN-P0-00` source audit 후 기존 명명과 충돌 여부를 확인하고 잠근다.
신규 파일명·클래스명은 32자 제한을 지킨다.

C++ 책임:

```text
- Sensor 활성/비활성 수명
- Passive Contact 탐지
- Active Scan 요청과 진행 상태
- Contact 안정 ID와 Actor 약한 참조
- 관계·분류·InformationLevel 공개
- Live / LastKnown / Lost / DestroyedHold 수명
- LastKnown 위치와 freshness
- Tactical Analysis progress 증가·감소
- Snapshot 생성
- 결정적 Automation 계약
- 미래 Authority/Replication 경계를 막지 않는 데이터 소유권
```

### DataAsset / Config

```text
- Passive Detection Range
- Active Scan Range
- Visual Detection Range 후보
- Scan Duration / Analysis Rate
- Analysis Decay Rate
- Contact Memory Time
- Destroyed Hold Time
- Update Interval
- 필요 시 Contact category별 modifier
```

수치는 코드 상수로 박지 않고 조정 가능한 데이터 계층으로 분리한다.

### Blueprint / UI

```text
- Radar Blip 시각
- Active Scan Sweep 시각
- Contact 관계 아이콘·색
- Last Known 표현
- Scan Progress 표현
- Target Intelligence 표시 애니메이션
```

Blueprint Widget이 World Actor를 직접 검색하거나 Sensor 판정을 재계산하지 않는다.

---

## 5. P0 작업 단계

| Task | 이름 | 상태 | 핵심 종료 기준 |
| --- | --- | --- | --- |
| `SEN-P0-00` | Foundation Audit | Done | 기존 TargetSelect/HUD/Vehicle/Collision/World 검색 경계와 재사용 타입 확정 |
| `SEN-P0-01` | Contact Data Contract | Technical Done | Contact ID·ContactState·Knowledge·Snapshot·Config 계약 + asset-free Automation 3/3 PASS |
| `SEN-P0-02` | Passive Detection | Technical Done | 근거리 전방향 탐지 + 직접 가시 대상 최소 계약 + bounded cursor + 같은 Actor 중복 Contact 금지 + Sensor 전체 6/6 PASS |
| `SEN-P0-03` | Contact Lifetime | Technical Done | Live → LastKnown → Lost → 제거, frozen LastKnown 위치, update 독립 Freshness/Memory, Lost 1회 Snapshot, Lost 전 same-ID reacquire + Sensor 전체 8/8 PASS |
| `SEN-P0-04` | Active Scan Analysis | Technical Done | Active command API·장거리 전방향 Detection·Analysis gain/decay·Knowledge 승격 / 공식 Build PASS + Sensor 11/11 PASS |
| `SEN-P0-05` | Destroyed Contact | Technical Done | VehicleHealth authoritative 파괴 확정 → 기존 Contact DestroyedHold / same-ID·Knowledge·frozen last-known 보존 / 독립 hold timer / Build + Sensor 13/13 PASS |
| `SEN-P0-06` | Public Snapshot Integration | Technical Done | TargetSelect 선택/TrackState 소유권 유지 + actor-free Sensor Snapshot Knowledge/Radar ViewData 연결 + Gameplay 의미 재계산 없음 + Build/Sensor/UI contract PASS |
| `SEN-P0-07` | Technical Acceptance | PASS / Done | 기존 final Build·Sensor 14/14·UI asset-free 회귀 재사용 + 책임 경계·actor-free contract 최종 정적 감사 PASS + `Systems/Targeting/SensorContact.md v1.0.0` Current 승격 |

P0의 주 목적은 **Runtime과 기술 검증**이다.
Radar/TargetPanel 최종 시각 USER PASS는 CF-FQ-032의 기존 USER Visual gate와 분리한다.

---

## 6. SEN-P0-00 — Foundation Audit

새 세션에서 가장 먼저 수행할 작업이다.

### 읽어야 할 현재 기준

```text
Document/ActiveWork.md
Document/ProjectSSOT/01_ProjectState.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/Plan/SensorContactPlan.md
Document/Plan/TargetSelectPlan.md
Document/Plan/InGameUIDesign.md
Document/Systems/SystemIndex.md

UE/Source/CarFight_Re/Public/CFTargetSelectTypes.h
UE/Source/CarFight_Re/Public/CFTargetSelectComp.h
UE/Source/CarFight_Re/Private/CFTargetCandidateSearch.cpp
UE/Source/CarFight_Re/Public/UI/CFHUDViewData.h
UE/Source/CarFight_Re/Public/UI/CFHUDDataProvider.h
ACFVehiclePawn 관련 컴포넌트 생성·수명 코드
```

### 반드시 판정할 것

```text
1. Sensor가 탐지할 대상의 최소 인터페이스를 ICFTargetSelectable로 재사용할 수 있는가?
2. 안정 ContactId를 TargetId와 동일하게 써도 되는가, 별도 ID가 필요한가?
3. ECFTargetInfoLevel / ECFTargetRelation을 그대로 재사용할 범위는 어디까지인가?
4. ContactState는 어떤 신규 enum이 필요한가?
5. 기존 TargetSelect 20Hz full-world scan을 Sensor 구현과 바로 합칠지, P0에서는 독립 유지할지?
6. Collision/LOS Trace Channel을 TargetSelect와 공유해도 의미 충돌이 없는가?
7. ACFVehiclePawn 기본 서브오브젝트로 SensorComp를 두는 것이 현재 초기화 순서에 안전한가?
8. Snapshot 소비자가 Actor pointer 없이도 UI/향후 네트워크에 필요한 정보를 읽을 수 있는가?
9. DestroyedHold와 TargetSelect의 현재 Destroyed 즉시 clear 계약을 어떻게 분리할 것인가?
10. CF-FQ-032 Source gate를 침범하지 않고 Sensor Runtime을 독립 기술 완료할 수 있는 경계는 어디인가?
```

### SEN-P0-00 종료 산출물

- 기존 구현 재사용/비재사용 표
- C++ 파일·타입 최종 후보
- Contact lifecycle state diagram
- P0 Config field 목록
- Automation matrix
- 다음 `SEN-P0-01` exact 변경 범위

`SEN-P0-00`에서는 Source·Config·Content Asset을 수정하지 않는다.

### SEN-P0-00 확정 판정

```text
- ICFTargetSelectable은 대상 자격·Source Metadata·대표 위치에만 제한 재사용한다.
- ContactId는 TargetId와 분리하고 Sensor Runtime이 독립 발급한다.
- ECFTargetRelation / ECFTargetCategory / ECFTargetInfoLevel은 의미가 같은 공개 데이터에 재사용한다.
- ECFTargetTrackState는 TargetSelect 추적 품질이므로 Sensor Contact lifecycle에 재사용하지 않는다.
- 신규 ECFSensorContactState는 Invalid / Live / LastKnown / Lost / DestroyedHold로 분리한다.
- TargetSelect의 20Hz full-world 후보 검색과 Sensor Detection은 P0에서 독립 유지한다.
- CFCollisionChannels::TargetSelect는 선택 가능 표면 전용이므로 일반 Sensor LOS 채널로 공유하지 않는다.
- UCFVehicleSensorComp는 ACFVehiclePawn 기본 서브오브젝트로 소유하되 기존 CoreReady/CombatReady를 차단하지 않는다.
- 공개 Sensor Snapshot은 Actor/UObject pointer를 포함하지 않는다.
- TargetSelect의 Destroyed 즉시 clear와 Sensor DestroyedHold는 독립 수명으로 유지한다.
- HUD Provider 연결은 SEN-P0-06 이전까지 수행하지 않는다.
```

## 6-1. SEN-P0-01 — Contact Data Contract 완료 체크포인트

### 적용 Source

```text
신규 Public
- CFSensorTypes.h
- CFVehicleSensorData.h
- CFVehicleSensorComp.h

신규 Private
- CFSensorTypes.cpp
- CFVehicleSensorData.cpp
- CFVehicleSensorComp.cpp
- CFSensorContractTests.cpp

최소 통합
- CFVehiclePawn.h
- CFVehiclePawn.cpp
```

### 확정 계약

```text
ECFSensorContactState
- Invalid
- Live
- LastKnown
- Lost
- DestroyedHold

FCFSensorContact
- ContactId: Sensor가 발급하는 독립 ID
- KnownTargetId: Identified 이상에서만 공개 가능한 TargetId
- KnownDisplayName
- ECFTargetCategory
- ECFTargetRelation
- ECFTargetInfoLevel
- ECFSensorContactState
- LastKnownWorldLocation
- LastObservedWorldTimeSeconds
- FreshnessSeconds
- AnalysisProgress01
- bDestroyedConfirmed
- Actor/UObject pointer 0
- ECFTargetTrackState 0

FCFSensorSnapshot
- Revision
- SnapshotWorldTimeSeconds
- SensorOriginWorldLocation
- SensorForwardWorldDirection
- bRuntimeReady
- bActiveScanRunning
- Actor-free Contacts[]
- ContactId 결정 정렬 / 중복 금지
```

`ContactId`는 같은 `UCFVehicleSensorComp` 수명에서 `Contact_000001` 형식의 증가 serial로 발급하며 `ResetSensorRuntime()` 후에도 serial을 재사용하지 않는다. 실제 Actor와 Contact의 약한 참조 매핑은 Passive Detection이 시작되는 `SEN-P0-02`의 private Runtime 상태로 추가하며 공개 Snapshot에 노출하지 않는다.

`FCFSensorConfig`와 `UCFVehicleSensorData` 클래스는 조정 가능한 데이터 계층을 준비했지만 **이번 단계에서 Content DataAsset은 생성하지 않았다.** Passive/Active/Visual 거리와 Contact 수명 기본값 `0`은 실제 게임 수치를 임의 확정하지 않기 위한 Foundation 비활성 값이며, `UpdateIntervalSec=0.1`, 정보 단계 임계값의 구조 유효성만 계약화했다.

### Vehicle Runtime 연결

`ACFVehiclePawn`은 `VehicleSensorComp`를 C++ 기본 서브오브젝트로 소유하고 `InitializeVehicleRuntime()`에서 Sensor Foundation을 독립 초기화한다.

```text
기존 CoreReady gate 변경 없음
기존 CombatReady gate 변경 없음
TargetSelect 선택 상태 접근/변경 없음
HUD Provider 연결 없음
World Actor 검색 없음
LOS Trace 없음
Active Scan 없음
```

따라서 기존 `BP_CFVehiclePawn` 계열은 Content Asset 저장 없이 컴포넌트를 상속하며, Sensor Foundation 실패가 현재 차량 주행·전투 준비 상태를 새로 차단하지 않는다.

### 검증

```text
Official UE 5.8 Editor Build
- Build Job: e8c46e25c4aa464da7f9f07730dcba1a
- Result: PASS / Exit Code 0

CarFight.Sensor.SEN_P0_01
- DataContract: PASS
- RuntimeContract: PASS
- PawnOwnership: PASS
- 합계: 3 PASS / 0 FAIL
- Process Job: 9a00e71e09fe421c8ea8ea544aabeff1
```

Automation은 기존 작업 전용 PowerShell runner의 `-TestFilter` 기능을 **이번 실행 수단으로만 1회 재사용**했다. 해당 runner를 Sensor 공용 도구로 승격하거나 수정하지 않았고, 검증 산출물은 `Saved/`에만 생성됐다.

첫 공식 Build는 Automation의 incomplete forward declaration 비교 한 줄 때문에 실패했고, TargetSelect 구현 변경 없이 테스트 파일 include만 보강했다. 두 번째 Build의 한 줄 patch payload 오염도 즉시 복구했으며, 위 최종 Build Job이 실제 승인 기준이다. Production Sensor Source 자체의 의미 변경을 요구한 실패는 없었다.

### 보호 결과

```text
TargetSelect Source 의미 변경 0
HUD Source 변경 0
DefaultEngine.ini 변경 0
CFCollisionChannels.h 변경 0
Content Asset 변경 0
WBP_TargetSelect.uasset 저장/덮어쓰기 0
기존 USER PASS/Pending 상태 변경 0
```

## 6-2. SEN-P0-02 — Passive Detection 완료 체크포인트

### 적용 Source

```text
수정
- CFSensorTypes.h / .cpp
- CFVehicleSensorData.h / .cpp
- CFVehicleSensorComp.h / .cpp

신규
- CFSensorPassiveTests.cpp

변경하지 않음
- CFVehiclePawn.h / .cpp
- TargetSelect Source
- HUD Source
- DefaultEngine.ini
- CFCollisionChannels.h
- Content Asset
```

### private Runtime Contact

`UCFVehicleSensorComp` 내부에만 다음 Runtime Contact를 둔다.

```text
FCFSensorContactRuntime
- TWeakObjectPtr<AActor> TargetActor
- FCFTargetDisplayInfo SourceDisplayInfo
- FCFSensorContact PublicContact
```

Actor 약한 참조와 Actor Truth 성격의 Source Metadata는 private Runtime에만 존재한다. 공개 `FCFSensorSnapshot`에는 `PublicContact`만 복사하므로 SEN-P0-01의 Actor-free 계약을 유지한다.

`ICFTargetSelectable` 재사용 범위는 다음으로 고정한다.

```text
재사용
- IsTargetSelectable: 대상 자격
- GetTargetSelectionLocation: 대표 위치
- GetTargetDisplayInfo.TargetCategory
- GetTargetDisplayInfo.Relation
- 원본 TargetId / DisplayName / AttributeTags는 private Source Metadata로 보관 가능

Sensor Knowledge로 자동 재사용 금지
- GetTargetDisplayInfo.InformationLevel
- GetTargetTrackState
```

기존 차량과 테스트 Target은 Source `InformationLevel=Identified`를 반환할 수 있지만 새 Sensor Contact는 반드시 `Detected`에서 시작한다. 따라서 Detected 상태에서 `KnownTargetId=None`, `KnownDisplayName=Empty`를 유지한다.

### bounded Passive world scan

TargetSelect 20Hz full-world Actor scan을 재사용하거나 합치지 않았다. Sensor는 `UWorld::GetLevels()`의 `ULevel::Actors[]`를 persistent cursor로 순회한다.

```text
PassiveScanLevelIndex
PassiveScanActorIndex
PassiveScanCycleSerial
```

한 update의 종료 조건은 다음 둘 중 먼저 도달한 것이다.

```text
1. MaxActorScansPerUpdate 슬롯을 소비
또는
2. 현재 World Level Actor 배열을 한 바퀴 완료
```

따라서 작은 World에서 budget이 남아도 같은 Actor를 같은 update에서 다시 스캔하지 않고, 큰 World에서는 다음 update가 이전 cursor 다음 슬롯부터 이어서 검사한다.

`FCFSensorConfig.MaxActorScansPerUpdate` 기본값은 `64`, 유효 범위는 `1~4096`이다. 이 값은 Scanner 성능이나 탐지 거리가 아니라 **한 update의 CPU 작업량 상한**이다.

### Passive / Visual Detection 의미

```text
PassiveDetectionRangeCm 안
→ 전방향 탐지
→ LOS 불필요

Passive 범위 밖 + VisualDetectionRangeCm 안
→ 직접 가시성 확인
→ ECC_Visibility LineTrace
→ blocker 없거나 첫 Blocking Hit Actor가 대상이면 탐지

둘 다 밖
→ 미탐지
```

Visual fallback은 `ECC_Visibility`만 사용한다. `CFCollisionChannels::TargetSelect`, `ECC_GameTraceChannel3`, TargetSelect 후보 검색/선택 상태는 사용하지 않는다.

별도 Sensor Collision Channel은 아직 추가하지 않았다. P0-02의 최소 직접 가시 계약은 일반 Visibility query로 충분했으므로 `DefaultEngine.ini`와 `CFCollisionChannels.h`는 변경하지 않았다.

### Contact 생성·중복 방지

같은 Actor는 `FindRuntimeContactIndexByActor()`로 먼저 조회한다.

```text
첫 탐지
→ 새 ContactId 1회 발급
→ Live Contact 생성

같은 Actor 재관측
→ 기존 Runtime Contact 갱신
→ ContactId 유지
→ Contact 수 증가 없음
```

P0-02에서 대상이 탐지 조건을 잃으면 Contact를 즉시 제거하는 것은 **임시 정책**이다. 다음 `SEN-P0-03`에서 이 제거 지점을 `Live → LastKnown → Lost` lifecycle로 교체한다.

### Config 의미 교정

`ActiveScanRangeCm=0`은 Active Scan이 아직 비활성인 유효 상태로 허용한다. 0이 아닌 Active Scan 값은 Passive 거리 이상이어야 한다.

Production fallback의 `PassiveDetectionRangeCm / VisualDetectionRangeCm` 기본값은 계속 `0`이다. 즉 **P0-02는 Passive Detection 기술 Runtime을 구현한 단계이지 실제 게임 탐지 거리 수치를 확정하거나 Content DataAsset을 생성한 단계가 아니다.** 실제 튜닝은 별도 명시 범위에서 수행한다.

### asset-free Automation

```text
Official UE 5.8 Editor Build
- Build Job: 0e5a272bd3f047cf97574914cb9fce92
- Result: PASS / Exit Code 0

CarFight.Sensor
- SEN_P0_01.DataContract: PASS
- SEN_P0_01.PawnOwnership: PASS
- SEN_P0_01.RuntimeContract: PASS
- SEN_P0_02.PassiveRange: PASS
- SEN_P0_02.VisualFallback: PASS
- SEN_P0_02.BoundedScan: PASS
- 합계: 6 PASS / 0 FAIL
- Process Job: 288b8cc1c5224f4bbf57fbf7b23654cd
```

P0-02 신규 Automation은 transient Editor World와 기존 C++ `ACFMissileTestTarget`만 사용했다. 프로젝트 Content Asset을 생성·수정·저장하지 않았다.

검증 범위:

```text
PassiveRange
- 자기 Sensor Owner 제외
- Passive 범위 안 탐지
- Passive 범위 밖 미탐지
- Source Identified → Public Detected 비누출
- KnownTargetId / KnownDisplayName 미공개

VisualFallback
- Passive 밖 / Visual 안 직접 가시 Target 탐지
- ECC_Visibility blocker 뒤 Target 미탐지
- TargetSelect Trace Channel 불사용

BoundedScan
- update당 Actor 슬롯 budget 준수
- persistent cursor로 전체 Actor 공정 순회
- 큰 budget도 update당 World 1회 초과 순회 금지
- 동일 Actor 2회 전체 순회 후 Contact 수 증가 없음
- 동일 Actor ContactId 집합 유지
- Snapshot duplicate ContactId 없음
```

### 보호 결과

```text
TargetSelect Source 변경 0
HUD Source 변경 0
CFVehiclePawn P0-02 변경 0
DefaultEngine.ini 변경 0
CFCollisionChannels.h 변경 0
Content Asset 변경/저장 0
WBP_TargetSelect.uasset 저장/덮어쓰기 0
기존 USER PASS/Pending 상태 변경 0
```

## 6-3. SEN-P0-03 — Contact Lifetime 완료 체크포인트

### 적용 Source

```text
수정
- CFVehicleSensorComp.h
- CFVehicleSensorComp.cpp

신규
- CFSensorLifetimeTests.cpp

변경하지 않음
- CFSensorTypes.h / .cpp
- CFVehicleSensorData.h / .cpp
- CFVehiclePawn.h / .cpp
- TargetSelect Source
- HUD Source
- DefaultEngine.ini / CFCollisionChannels.h
- Content Asset
```

P0-01에서 이미 `ECFSensorContactState`, `LastKnownWorldLocation`, `LastObservedWorldTimeSeconds`, `FreshnessSeconds`와 `ContactMemoryTimeSec` 계약이 준비돼 있었으므로 공개 Sensor 타입이나 Config field를 새로 늘리지 않았다.

### Contact lifecycle

P0-02의 미탐지 즉시 제거를 다음 수명으로 교체했다.

```text
유효 관측
→ Live
→ LastKnownWorldLocation = 현재 신뢰 관측 위치
→ LastObservedWorldTimeSeconds = 현재 World time
→ FreshnessSeconds = 0

실제 후보 평가에서 탐지 실패
→ Live → LastKnown
→ LastKnownWorldLocation 고정
→ LastObservedWorldTimeSeconds 고정
→ 기존 ContactId / Sensor Knowledge / AnalysisProgress 유지

매 Sensor update
→ FreshnessSeconds = 현재 update time - LastObservedWorldTimeSeconds
→ bounded Actor cursor 재방문과 독립적으로 진행

FreshnessSeconds >= ContactMemoryTimeSec
→ LastKnown → Lost

Lost 최초 Snapshot 게시
→ bLostSnapshotPublished = true

다음 Sensor update
→ Runtime Contact 제거
→ 다음 Snapshot에서 사라짐
```

`LastKnownWorldLocation`은 LastKnown/Lost 동안 실제 Actor가 이동해도 따라가지 않는다. 현재 실제 위치 추정이나 외삽을 하지 않고 마지막 신뢰 관측 위치만 유지한다.

### bounded cursor와 lifetime 분리

`RunPassiveDetectionUpdate()`는 bounded scan 전에 모든 기존 Runtime Contact에 `AdvanceContactLifetimes()`를 실행한다.

```text
Sensor update 시작
→ 기존 Contact 전체 freshness/lifetime 진행
→ 이번 update의 bounded Actor subset만 후보 재평가
→ 재획득/신규 탐지 반영
→ Actor-free Snapshot 게시
```

따라서 Actor가 `MaxActorScansPerUpdate` 때문에 이번 update의 cursor 범위에 들어오지 않았다는 사실 자체는 탐지 실패가 아니다. 실제로 해당 Actor를 재평가해 범위/가시성/자격 실패를 확인했을 때만 Live→LastKnown으로 전환하며, 한번 LastKnown에 들어간 뒤의 freshness와 ContactMemory는 cursor 재방문 없이 계속 진행한다.

private weak Actor가 invalid가 된 Live Contact도 cursor 재방문을 기다리지 않고 lifecycle update에서 LastKnown으로 전환한다. `ContactMemoryTimeSec=0`이어도 같은 update에서 LastKnown을 건너뛰어 즉시 Lost가 되지 않도록 최소 한 lifecycle 단계는 분리한다.

### Lost 최소 1 Snapshot

private Runtime Contact에 다음 flag만 추가했다.

```text
bLostSnapshotPublished
```

Lost로 전환된 update에서는 Contact를 제거하지 않는다. `PublishRuntimeSnapshot()`이 Lost Contact를 공개 Snapshot에 복사한 뒤 flag를 세우고, **그 다음 Sensor update 시작 시** 해당 Runtime Contact를 제거한다.

따라서 소비자는 Lost 상태를 최소 한 Snapshot revision에서 항상 관측할 수 있다.

### Lost 전 same-ID reacquire

LastKnown 또는 아직 공개되지 않은 Lost Contact와 같은 Actor가 다시 유효 탐지되면 기존 Runtime Contact를 재사용한다.

```text
기존 ContactId 유지
기존 InformationLevel 유지
기존 KnownTargetId / KnownDisplayName 유지
기존 AnalysisProgress01 유지
ContactState → Live
LastKnownWorldLocation → 새 신뢰 관측 위치
LastObservedWorldTimeSeconds → 새 관측 시각
FreshnessSeconds → 0
bLostSnapshotPublished → false
```

즉 P0-04에서 획득할 Sensor Knowledge가 이후 LastKnown/reacquire 과정에서 임의 초기화되지 않는 기반을 먼저 고정했다. 이미 Lost가 공개되고 다음 update cleanup까지 끝난 뒤 새로 탐지되는 경우는 제거된 Runtime Contact이므로 새 Contact lifecycle로 시작한다.

### DestroyedHold / Active Scan 경계

P0-03 일반 lifetime은 `DestroyedHold` Contact를 진행하거나 생성하지 않는다. `DestroyedHold`는 `SEN-P0-05`가 소유한다.

`bActiveScanRunning=true` 전환, Active Scan 거리 판정과 Analysis progress 증가·감소도 P0-03에서 구현하지 않았다. 이는 `SEN-P0-04` 책임이다.

### asset-free Automation

```text
Official UE 5.8 Editor Build
- Build Job: fdd231a10ff648839b14b6a2468b295a
- Result: PASS / Exit Code 0

CarFight.Sensor
- SEN_P0_01.DataContract: PASS
- SEN_P0_01.PawnOwnership: PASS
- SEN_P0_01.RuntimeContract: PASS
- SEN_P0_02.PassiveRange: PASS
- SEN_P0_02.VisualFallback: PASS
- SEN_P0_02.BoundedScan: PASS
- SEN_P0_03.ContactLifetime: PASS
- SEN_P0_03.Reacquire: PASS
- 합계: 8 PASS / 0 FAIL
- Process Job: 45c96b20b2984984b753ff69c9b49d3f
```

`CFSensorLifetimeTests.cpp`는 transient Editor World, plain Actor Sensor Owner와 기존 C++ `ACFMissileTestTarget`만 사용한다. 프로젝트 Content Asset을 생성·수정·저장하지 않는다.

검증 범위:

```text
ContactLifetime
- 최초 Live Contact 생성
- 실제 범위 이탈 평가 → LastKnown
- LastKnownWorldLocation / LastObservedWorldTimeSeconds 고정
- cursor 재방문 없이 FreshnessSeconds 0.25초 진행
- ContactMemoryTimeSec 1.0초 만료 → Lost
- Lost 최소 1 Actor-free Snapshot 노출
- 같은 ContactId 유지
- 다음 lifecycle update에서 제거

Reacquire
- LastKnown 중 동일 Actor 재획득
- Runtime Contact 수 증가 없음
- ContactId 유지
- 기존 Identified Knowledge 유지
- KnownTargetId / KnownDisplayName 유지
- AnalysisProgress01 유지
- FreshnessSeconds 0 복귀
- 새 신뢰 관측 위치로 LastKnownWorldLocation 갱신
```

Automation 시간은 실제 sleep을 사용하지 않고 private lifecycle 함수에 명시적인 World time을 전달해 결정적으로 검증했다. Production Runtime은 실제 `UWorld::GetTimeSeconds()`를 사용한다.

### 보호 결과

```text
TargetSelect Source 변경 0
HUD Source 변경 0
CFVehiclePawn 변경 0
CFSensorTypes / VehicleSensorData 변경 0
DefaultEngine.ini / CFCollisionChannels.h 변경 0
Content Asset 변경/저장 0
DestroyedHold transition 추가 0
Active Scan 시작/진행 추가 0
WBP_TargetSelect.uasset 저장/덮어쓰기 0
기존 USER PASS/Pending 상태 변경 0
```

## 6-4. SEN-P0-04 — Active Scan Analysis Source 체크포인트

### 입력 / Scanner 요청 owner 감사

현재 Source에는 Scanner 전용 `InputAction`, `StartScan`, `StopScan` 구현이 없다.
기존 입력 소유권은 다음과 같다.

```text
ACFPlayerController
= Pause/Back 같은 Pawn 비종속 System·UI 입력

ACFVehiclePawn
= Drive / Look / Fire / SelectTarget / ClearTarget 같은 차량 Gameplay Enhanced Input

UCFVehicleSensorComp
= Sensor Runtime 상태·탐지·Contact·Knowledge owner
```

따라서 SEN-P0-04에서는 Sensor가 Enhanced Input 자산이나 키 바인딩을 직접 소유하지 않는다.

```text
ACFVehiclePawn 또는 향후 Input Adapter
→ Scanner InputAction 수신
→ StartActiveScan() / StopActiveScan() 명령 전달

UCFVehicleSensorComp
→ Active Scan 실행 상태
→ 남은 시간
→ Active range Detection
→ Tactical Analysis
→ Sensor Knowledge
```

공용 dirty 파일인 `CFVehiclePawn.h/.cpp`는 이번 단계에서 수정하지 않았다. 실제 InputAction 연결은 후속 공용 통합 Gate에서 수행할 수 있도록 명령 API만 먼저 고정했다.

### 적용 Source

```text
수정
- CFSensorTypes.h v1.2.1
- CFVehicleSensorComp.h v1.3.0
- CFVehicleSensorComp.cpp v1.3.0

신규
- CFSensorActiveScanTests.cpp v1.0.0

변경하지 않음
- CFVehiclePawn.h / .cpp
- TargetSelect Source
- HUD Source
- DefaultEngine.ini / CFCollisionChannels.h
- VehicleSensorData schema
- Content Asset
```

`CFSensorTypes.h v1.2.0`은 새 field를 추가하지 않고 P0-01부터 준비돼 있던 Active/Analysis 공개 계약의 ToolTip·Migration만 현재 Runtime 의미와 동기화했으며, 최종 v1.2.1은 의미 변경 없는 formatting-only 정리다.

### Active Scan command API

Sensor Runtime에 다음 명령/조회 API를 추가했다.

```text
StartActiveScan()
StopActiveScan()
IsActiveScanRunning()
GetActiveScanRemainingSeconds()
```

`StartActiveScan()`은 Runtime Ready, 유효 `ActiveScanRangeCm`, 양수 `ActiveScanDurationSec` 조건을 만족할 때만 시작한다. 중복 Start는 거부한다.

`StopActiveScan()`은 실행 상태와 남은 시간을 종료하지만 `AnalysisProgress01`을 즉시 0으로 만들지 않는다.

`FCFSensorSnapshot.bActiveScanRunning`은 더 이상 Foundation 고정 False가 아니라 실제 Sensor Runtime 상태를 게시한다.

### Active Scan Detection 의미

```text
PassiveDetectionRangeCm 안
→ 기존 Passive 전방향 Detection

Passive 밖 + VisualDetectionRangeCm 안 + ECC_Visibility 직접 가시
→ 기존 Visual fallback Detection

Active Scan 실행 중 + ActiveScanRangeCm 안
→ 장거리 전방향 Active Contact Detection
→ LOS는 Contact 생성 조건이 아님
```

따라서 차량 뒤쪽의 대상도 Active Scan range 안이면 Contact로 탐지할 수 있다.

Active Scan 신호와 Tactical Analysis를 같은 의미로 합치지 않았다.
건물 뒤 대상은 Active signal로 Contact가 될 수 있지만 분석 진척을 얻지는 못한다.

### Tactical Analysis gain 조건

`AnalysisProgress01`은 bounded Actor cursor가 해당 Actor를 이번 update에 다시 방문했는지와 분리해 **현재 Runtime Contact 목록을 Sensor update마다 직접 진행**한다.

Gain 조건은 다음을 모두 만족해야 한다.

```text
Active Scan 실행 중
+ ContactState == Live
+ 같은 private Actor가 유효
+ ICFTargetSelectable 자격 유효
+ 현재 실제 대표 위치가 ActiveScanRangeCm 안
+ Owner → Target 대표 위치 ECC_Visibility 직접 가시
```

유효하면:

```text
AnalysisProgress01
+= AnalysisGainPerSec * ActiveValidTime
```

Active Scan이 한 Sensor update 중간에 만료되면 남은 비활성 시간은 같은 update에서 decay 대상으로 계산한다.

### decay / 재개 계약

다음 상태에서는 즉시 reset하지 않는다.

```text
가림
Active range 이탈
LastKnown
Active Scan 중단 또는 만료
```

대신:

```text
AnalysisProgress01
-= AnalysisDecayPerSec * InvalidTime
```

으로 0~1 범위에서 서서히 감소한다.

Lost 전에 같은 Actor가 다시 유효해지면 P0-03의 기존 Runtime Contact를 재사용하므로:

```text
같은 ContactId
기존 Sensor Knowledge
남아 있는 AnalysisProgress01
```

을 유지한 채 Live로 돌아오고 남은 progress에서 분석을 재개한다.

### InformationLevel / Known Target Knowledge

Source `FCFTargetDisplayInfo.InformationLevel`은 여전히 Actor truth이며 Sensor Knowledge로 복사하지 않는다.
새 Contact는 Source가 Identified라고 보고하더라도 `Detected`에서 시작한다.

Sensor가 직접 획득한 `AnalysisProgress01`이 Config threshold를 통과할 때만 단방향 승격한다.

```text
AnalysisProgress01 >= IdentifiedThreshold
→ Detected → Identified
→ KnownTargetId 공개
→ KnownDisplayName 공개

AnalysisProgress01 >= DetailedScanThreshold
→ Identified/Detected → DetailedScan
→ KnownTargetId / KnownDisplayName 유지
```

progress가 이후 decay로 threshold 아래로 내려가도 이미 획득한 `InformationLevel`과 Known Target Knowledge는 강등하지 않는다.

### P0-03 lifecycle 보존

Active-only 관측 Contact는 마지막 관측이 Passive/Visual baseline으로도 유지 가능한지를 private flag로 기억한다.

```text
bBaselineDetectionValidAtLastObservation
```

Active Scan 종료/만료 시:

```text
baseline Passive/Visual도 유효했던 Live Contact
→ Live 유지

Active Scan 때문에만 Live였던 Contact
→ LastKnown
```

LastKnown 이후 ContactMemory→Lost→최소 1 Snapshot→제거와 Lost 전 same-ID reacquire는 SEN-P0-03 계약을 그대로 사용한다.

### asset-free Automation Source

신규 `CFSensorActiveScanTests.cpp`에 다음 3개 테스트를 작성했다.

```text
CarFight.Sensor.SEN_P0_04.ActiveRange
- Active 시작 전 Passive만 탐지
- Active 실행 중 차량 뒤쪽 장거리 Target 전방향 탐지
- Active range 밖 미탐지
- Snapshot.bActiveScanRunning
- 중복 Start 거부
- duration 자동 만료
- baseline Contact Live 유지 / active-only Contact LastKnown

CarFight.Sensor.SEN_P0_04.AnalysisProgress
- 직접 가시 Active Contact Analysis 증가
- threshold 전 Detected / KnownTargetId·Name 비공개
- Source Identified 직접 복사 금지
- Identified threshold 승격 + Known 정보 공개
- DetailedScan threshold 승격
- 같은 ContactId 유지
- decay 뒤 획득 Knowledge 비강등

CarFight.Sensor.SEN_P0_04.AnalysisDecay
- ECC_Visibility 가림 중 Active Contact는 Live 유지
- 가림에서 progress 즉시 reset 없이 decay
- 가림 해제 뒤 남은 progress에서 증가 재개
- Active range 이탈 → LastKnown + decay
- Lost 전 재획득 → 같은 ContactId + 남은 progress 재개
- StopActiveScan 자체는 progress 즉시 reset하지 않음
- 다음 Sensor update에서 decay
```

테스트는 transient Editor World, plain Actor Sensor Owner, 기존 C++ `ACFMissileTestTarget`, transient `UBoxComponent` Visibility blocker만 사용한다. Content Asset을 생성·수정·저장하지 않는다.

### 최종 검증 — Technical Done

사용자가 실행 중이던 Editor를 정상 종료한 뒤 같은 exact Source에서 공식 UE 5.8 Editor Build를 재실행했다.

```text
Build Job: 786e47df68fe43f9a924c9c6fbd86726
Result: SUCCEEDED / Exit Code 0
Duration: 12.451s

Link:
- UnrealEditor-NetCore.dll PASS
- UnrealEditor-CarFight_Re.dll PASS
```

직전 exact-source Build `d1cd746d5f3340e1bb1cfe43885cf5e2`에서 이미 UHT와 P0-04 포함 Sensor C++ compile을 확인했고, 이번 Build에서 이전 DLL lock이 해소된 상태로 최종 Link와 metadata write까지 완료했다.

새 linked binary 기준 `CarFight.Sensor` 전체 회귀:

```text
Process Job: 6fcb3d2dbd5f42489febfff6040611af
Execution: Tools/RunInvAutomation.ps1 -TestFilter CarFight.Sensor
Engine Exit Code: 0
Success: 11
Failure: 0

PASS CarFight.Sensor.SEN_P0_01.DataContract
PASS CarFight.Sensor.SEN_P0_01.PawnOwnership
PASS CarFight.Sensor.SEN_P0_01.RuntimeContract
PASS CarFight.Sensor.SEN_P0_02.BoundedScan
PASS CarFight.Sensor.SEN_P0_02.PassiveRange
PASS CarFight.Sensor.SEN_P0_02.VisualFallback
PASS CarFight.Sensor.SEN_P0_03.ContactLifetime
PASS CarFight.Sensor.SEN_P0_03.Reacquire
PASS CarFight.Sensor.SEN_P0_04.ActiveRange
PASS CarFight.Sensor.SEN_P0_04.AnalysisDecay
PASS CarFight.Sensor.SEN_P0_04.AnalysisProgress
```

따라서 현재 판정은:

```text
P0-04 Source Implemented: YES
Official linked Editor binary: PASS
CarFight.Sensor regression: 11/11 PASS
SEN-P0-04 Technical Done: YES
SEN-P0-05 진입 가능: YES
```

### 보호 결과

최종 정적 감사:

```text
ECC_GameTraceChannel3 사용 0
CFCollisionChannels::TargetSelect 사용 0
CFHUDDataProvider 사용 0
SetSelectedTarget 사용 0
RefreshCurrentCandidate 사용 0
Scanner InputAction 신규 구현 0
DestroyedHold 상태 전환 추가 0
Source InformationLevel → Public Knowledge 직접 대입 0
CFVehiclePawn 변경 0
TargetSelect/HUD 변경 0
Project Config 변경 0
Content Asset 변경/저장 0
WBP_TargetSelect.uasset 저장/덮어쓰기 0
USER PASS 추정 0
```

### SEN-P0-05 Destroyed Contact — Technical Done

#### 파괴 확정 owner 감사

현재 차량 파괴 truth의 authoritative owner는 `UCFVehicleHealthComp`다.

```text
VehicleDefense / Damage Runtime
→ UCFVehicleHealthComp::ApplyIntegrityDamageFromHitContext
→ CurrentHealth <= 0 최초 전환
→ bDestroyed = true
→ OnVehicleDestroyed(FCFDamageHitContext) 1회 Broadcast
```

Sensor는 새 파괴 판정을 만들지 않고 이 기존 truth만 소비한다.

```text
Primary signal: UCFVehicleHealthComp::OnVehicleDestroyed
Race-safe state check: UCFVehicleHealthComp::IsDestroyed()
금지: AActor::OnDestroyed / EndPlay / weak Actor invalid만으로 파괴 추정
```

TargetSelect도 이미 같은 `OnVehicleDestroyed`를 독립 구독해 선택을 즉시 `Destroyed` 사유로 clear한다. P0-05는 TargetSelect 코드를 수정하지 않았으며, **TargetSelect 선택 수명과 Sensor Contact 수명은 계속 독립**이다.

#### Sensor DestroyedHold 전환

기존 Sensor Runtime Contact가 만들어질 때 해당 Actor의 `UCFVehicleHealthComp`가 있으면 `OnVehicleDestroyed`에 독립 구독한다.

파괴 이벤트 또는 race-safe `IsDestroyed()` 확인이 들어오면 **이미 존재하는 Contact만** 다음 상태로 전환한다.

```text
Live / LastKnown / Lost 게시 전 기존 Contact
→ DestroyedHold
→ bDestroyedConfirmed = true
→ ContactId 유지
→ InformationLevel / KnownTargetId / KnownDisplayName 유지
→ AnalysisProgress01 유지
→ LastKnownWorldLocation 유지
→ LastObservedWorldTimeSeconds 유지
```

파괴 순간의 실제 Actor 위치나 Damage Impact 위치로 `LastKnownWorldLocation`을 덮어쓰지 않는다. 이 값은 마지막 Sensor 신뢰 관측 위치다.

Sensor가 한 번도 알지 못한 Actor가 이미 `IsDestroyed()==true`인 상태로 처음 bounded scan에 들어온 경우에는 새 Destroyed Contact를 생성하지 않는다. P0-05는 **기존 Contact의 파괴 확정 수명**만 소유한다.

#### DestroyedHold 보존 시간

private Runtime에 `DestroyedConfirmedWorldTimeSeconds`를 별도로 저장한다.

```text
DestroyedHoldElapsed
= CurrentSensorUpdateWorldTime - DestroyedConfirmedWorldTimeSeconds
```

이 경과 시간은 Actor bounded cursor 재방문 여부와 무관하게 `AdvanceContactLifetimes()`에서 매 Sensor update 진행한다.

`DestroyedHoldTimeSec` 만료 시 Contact를 제거하고 VehicleHealth event binding도 정리한다. Passive/Active 탐지 작업이 없어도 기존 DestroyedHold Contact가 있으면 Tick을 유지해 만료가 진행된다.

Production fallback의 `DestroyedHoldTimeSec=0`은 게임 튜닝 확정을 뜻하지 않는다. 0인 경우 파괴 이벤트 handler가 즉시 DestroyedHold Snapshot을 한 번 게시할 수 있지만 다음 lifetime update에서 만료 제거된다. 실제 hold 체감 수치는 후속 데이터 튜닝 범위다.

#### weak Actor invalid 보호

VehicleHealth 파괴 확정 없이 Actor가 `Destroy()`되어 weak reference가 invalid가 되는 경우는 기존 P0-03 의미를 그대로 사용한다.

```text
Live + weak invalid
→ LastKnown
→ ContactMemoryTimeSec
→ Lost
→ 제거

bDestroyedConfirmed = false
```

따라서 Actor lifecycle 종료와 Gameplay 차량 파괴를 혼동하지 않는다.

#### Source / 테스트

```text
CFVehicleSensorComp.h/.cpp v1.4.0
CFSensorDestroyedTests.cpp v1.0.0
```

신규 asset-free Automation:

```text
CarFight.Sensor.SEN_P0_05.DestroyedHold
- 실제 VehicleHealth 피해 경로로 최초 파괴 확정
- event 즉시 DestroyedHold Snapshot
- 같은 ContactId / Knowledge / AnalysisProgress 보존
- 마지막 신뢰 위치·관측 시각 보존
- bounded cursor 독립 hold 진행
- DestroyedHoldTimeSec 만료 제거
- 파괴된 Actor 재평가로 Live 복귀 금지

CarFight.Sensor.SEN_P0_05.InvalidActor
- Health signal 없는 Actor Destroy → LastKnown
- bDestroyedConfirmed=false
- 같은 ContactId 유지
- Sensor가 모르는 상태에서 먼저 파괴된 Actor는 새 Destroyed Contact 생성 금지
```

#### 최종 검증

```text
Official UE 5.8 Build
Job: 954dc0ab844b40f48f2674f2a0ae672a
Result: PASS / Exit Code 0
- UHT PASS / 3 generated files
- CFSensorDestroyedTests.cpp compile PASS
- CFVehicleSensorComp.cpp compile PASS
- 기존 Sensor tests compile PASS
- UnrealEditor-CarFight_Re.dll Link PASS

CarFight.Sensor Regression
Process: 4ba274cc90814e5b90d40c33820b3bd0
Engine Exit Code: 0
Success: 13
Failure: 0
- P0-01~04 기존 11개 PASS
- P0-05 DestroyedHold PASS
- P0-05 InvalidActor PASS
```

최종 금지 경계 감사:

```text
OnDestroyed.Add 0
OnEndPlay.Add 0
ECC_GameTraceChannel3 0
CFCollisionChannels::TargetSelect 0
CFHUDDataProvider 0
SetSelectedTarget 0
RefreshCurrentCandidate 0
InputAction_ActiveScan 0
CFVehiclePawn 변경 0
TargetSelect/HUD 변경 0
Project Config 변경 0
Content Asset 변경/저장 0
USER PASS 추정 0
```

### SEN-P0-06 Public Snapshot Integration — Technical Done

#### 기존 소비 경계 감사

P0-06 시작 시 실제 Source 기준 책임은 다음과 같았다.

```text
TargetSelect
- 후보 검색·선택·선택 해제 owner
- HasSelectedTarget / IsSelectedTargetValid
- GetSelectedTargetTrackState
- GetSelectedTargetActor

Sensor
- Detection / Contact lifecycle / Player Knowledge owner
- FCFSensorSnapshot actor-free public contract

HUD Provider
- Runtime → Player-facing ViewData adapter
- 기존 Target 정보는 TargetSelect DisplayInfo를 직접 소비
- 기존 Radar는 Sensor Provider 미연결로 Unavailable
```

P0-06에서는 TargetSelect의 선택 owner를 유지하면서 **Target Knowledge와 Radar Contact 의미 source를 FCFSensorSnapshot으로 이동**했다.

최종 경계:

```text
TargetSelect
→ 선택 존재 / 선택 유효성 / TrackState
→ 선택 Actor는 ContactId association에만 사용

Sensor
→ FCFSensorSnapshot
→ Relation / Category / InformationLevel
→ KnownTargetId / KnownDisplayName
→ ContactState / Freshness / AnalysisProgress / DestroyedConfirmed
→ LastKnownWorldLocation

HUD Data Provider
→ TargetSelect 선택 상태 + Sensor Snapshot Knowledge를 read-only 합성
→ Radar Contact를 Snapshot에서 ViewData로 변환
→ Detection / lifecycle / Knowledge를 재판정하지 않음

HUD Presenter / Widget
→ ViewData 표시만 담당
```

#### 선택 Actor → ContactId read-only bridge

Detected 단계에서는 `KnownTargetId`가 의도적으로 `None`이므로 TargetSelect의 선택 Actor를 Sensor Snapshot Contact와 TargetId로 연결할 수 없다.

이를 위해 `UCFVehicleSensorComp`에 다음 **association-only** API를 추가했다.

```text
TryGetContactIdForActor(TargetActor, OutContactId)
```

이 API는 이미 존재하는 private Runtime Contact에서 **ContactId만 반환**한다.

```text
공개하지 않음:
- Actor metadata
- 현재 Actor 위치
- Source InformationLevel
- private Runtime Contact
- Knowledge 원본
```

HUD Provider는 이 ContactId로 `FCFSensorSnapshot.Contacts`를 다시 찾고, 실제 Player-facing 정보는 전부 Snapshot에서 읽는다.

#### Target HUD Snapshot 소비

`FCFTargetHUDData`에 Sensor association/Knowledge 상태를 추가했다.

```text
SensorContactAvailability
ContactId
ContactState
FreshnessSeconds
AnalysisProgress01
bDestroyedConfirmed
```

기존 `TargetId / DisplayName / Relation / Category / InformationLevel / DistanceMeters`도 P0-06부터 Sensor Snapshot 의미를 사용한다.

```text
TargetSelect source:
- bHasSelectedTarget
- bSelectedTargetValid
- TrackState

Sensor Snapshot source:
- TargetId = KnownTargetId
- DisplayName = KnownDisplayName
- Relation / Category
- InformationLevel
- ContactState / Freshness / AnalysisProgress / DestroyedConfirmed
- Distance = Snapshot SensorOrigin → LastKnownWorldLocation
```

선택은 존재하지만 현재 Sensor Contact가 없으면 선택 자체는 유지하되 `SensorContactAvailability=Unknown`으로 남고 TargetSelectable 원본 identity를 HUD로 누출하지 않는다.

Detected 단계에서는 Source Actor가 이미 Identified metadata를 가지고 있어도 HUD는 Sensor Snapshot의 Detected 상태와 숨겨진 Identity를 그대로 사용한다.

#### Radar Snapshot 소비

Sensor Runtime이 준비됐으면 Radar availability는 다음과 같다.

```text
Snapshot ready + Contact 0
→ KnownZero

Snapshot ready + Contact 1+
→ Known

Sensor missing / Runtime not ready / invalid Snapshot
→ Unavailable
```

각 `FCFRadarContactHUDData`는 Snapshot에서 다음을 받는다.

```text
ContactId
Relation / Category / InformationLevel
ContactState
FreshnessSeconds
AnalysisProgress01
bDestroyedConfirmed
bSelected
```

좌표는 Snapshot에 이미 공개된 데이터만 사용한다.

```text
SensorOriginWorldLocation
SensorForwardWorldDirection
LastKnownWorldLocation
→ Forward(+X) / Right(+Y) RelativePositionMeters
→ DistanceMeters
```

Actor의 현재 위치를 HUD가 직접 읽지 않는다.

#### Radar Range / Zoom 비추정

현재 Source에는 Radar 표시 반경 또는 Zoom을 `NormalizedPosition [-1,1]`로 변환할 명시 계약이 없다.

따라서 Sensor의 Passive/Visual/Active 사거리를 UI Radar 반경으로 임의 사용하지 않았다.

```text
RelativePositionMeters = Known
DistanceMeters = Known
NormalizedPositionAvailability = Unavailable
NormalizedPosition = 기본값 유지
```

Radar Range/Zoom 계약과 실제 Blip 배치 정책은 CF-FQ-032 시각 작업에서 별도 확정한다.

#### 정적 Radar placeholder 보호

기존 `WBP_CFRadarPanel`의 `CanvasPanel_RadarContacts`에는 실제 Sensor Contact가 아니라 디자인용 정적 placeholder Image가 있다.

Sensor Radar가 `Known`으로 바뀌었다는 이유만으로 이 Canvas를 보이면 가짜 Contact가 표시되므로 `UCFHUDPresenter`는 P0-06에서도 해당 Canvas를 항상 숨긴다.

```text
Radar title
- Known / KnownZero → RADAR
- Unavailable → RADAR — UNAVAILABLE

CanvasPanel_RadarContacts
- 항상 Hidden/Collapsed 처리
- 동적 Sensor Contact 렌더링으로 해석하지 않음
```

Blueprint/Widget Asset은 수정·저장하지 않았다.

#### DestroyedHold와 TargetSelect 독립 수명 유지

신규 `CarFight.Sensor.SEN_P0_06.HUDSnapshot`은 실제 VehicleHealth 피해 경로로 다음을 함께 검증했다.

```text
VehicleHealth Destroyed
→ TargetSelect: 선택 즉시 clear
→ HUD Target: KnownZero

동시에
→ Sensor: 같은 ContactId DestroyedHold 유지
→ Radar: DestroyedHold Contact 유지
→ Knowledge / AnalysisProgress / last-known relative position 유지
→ bSelected=false
```

따라서 P0-05의 독립 수명 계약이 Public Snapshot Integration 뒤에도 유지된다.

#### Source

```text
CFVehicleSensorComp.h/.cpp v1.5.0
CFHUDViewData.h v1.4.0
CFHUDDataProvider.h v1.5.0
CFHUDDataProvider.cpp v1.6.0
CFHUDPresenter.cpp v1.8.0
CFSensorHUDTests.cpp v1.0.0 신규
```

`CFVehiclePawn`, `CFTargetSelectComp`, TargetSelect 후보 검색 Source와 Content Asset은 변경하지 않았다.

#### 최종 검증

최종 formatting까지 반영한 정확한 Source 기준:

```text
Official UE 5.8 Build
Job: 7dff9da7aaa24e76b0871348762c9d93
Result: PASS / Exit Code 0
- UHT PASS
- CFSensorHUDTests.cpp compile PASS
- CFHUDDataProvider.cpp compile PASS
- CFHUDPresenter.cpp compile PASS
- existing CFHUDDataTests.cpp compile PASS
- UnrealEditor-CarFight_Re.dll Link PASS
```

최종 Sensor 전체 회귀:

```text
Process: 85d008613a104df9b7107795995c6ac5
Engine Exit Code: 0
Success: 14
Failure: 0

P0-01 3/3 PASS
P0-02 3/3 PASS
P0-03 2/2 PASS
P0-04 3/3 PASS
P0-05 2/2 PASS
P0-06 HUDSnapshot 1/1 PASS
```

HUD Provider 관련 asset-free 보호 회귀:

```text
CarFight.UI.UI_P0_03.ViewDataAvailability
Process: 67d114255ec943bb8ded2642a40a581c
1/1 PASS

CarFight.UI.UI_P0_03.ProviderPawnRebind
Process: 395c93df5fb947a1a979ca8fa0c74665
1/1 PASS
```

두 UI 회귀 뒤의 변경은 header 주석/들여쓰기 formatting-only이며 최종 Build와 Sensor 14/14에서 다시 컴파일·링크됐다.

#### 최종 책임 경계 감사

Production HUD Provider/Presenter 기준:

```text
GetSelectedTargetDisplayInfo 0
GetTargetDisplayInfo 0
GetActorLocation 0
PassiveDetectionRangeCm 0
ActiveScanRangeCm 0
VisualDetectionRangeCm 0
SetSelectedTarget 0
SetCurrentCandidate 0
```

`SetSelectedTarget`은 신규 Automation에서 **TargetSelect가 계속 선택 owner임을 검증하기 위해서만** 사용한다.

Sensor 내부의 기존 `GetTargetDisplayInfo`는 P0-02부터 Source Metadata를 private로 획득하는 Sensor 책임이며 HUD가 직접 읽는 경로가 아니다.

추가 보호:

```text
CFVehiclePawn Source 변경 0
TargetSelect Source 변경 0
WBP_TargetSelect 변경/저장 0
기타 HUD Blueprint Asset 변경/저장 0
Content Asset 변경/저장 0
Project Config 변경 0
P0-04 Active Scan 의미 변경 0
P0-05 DestroyedHold 의미 변경 0
CF-FQ-032 USER Visual PASS 추정 0
TS-P0-08 USER PASS 추정 0
```

### SEN-P0-07 Technical Acceptance — PASS

P0-06 이후 Sensor/HUD Source 의미 변경이 없으므로 `SEN-P0-07`은 Source를 재구현하거나 Build/Automation을 습관적으로 반복하지 않고 기존 final evidence와 current Source 정적 감사를 결합해 판정했다.

재사용한 최종 evidence:

```text
Official UE 5.8 Build
7dff9da7aaa24e76b0871348762c9d93
PASS / Exit Code 0

CarFight.Sensor
85d008613a104df9b7107795995c6ac5
14 Success / 0 Failure

UI asset-free protection
ViewDataAvailability 67d114255ec943bb8ded2642a40a581c 1/1 PASS
ProviderPawnRebind 395c93df5fb947a1a979ca8fa0c74665 1/1 PASS
```

최종 Source 감사 결과:

```text
Public FCFSensorContact / FCFSensorSnapshot Actor·UObject pointer = 0
TargetSelect → Sensor runtime dependency = 없음
Sensor → TargetSelect selection mutation = 없음
Sensor LOS = ECC_Visibility
TargetSelect LOS = CFCollisionChannels::TargetSelect
Sensor weak invalid / Actor EndPlay를 destruction truth로 사용 = 없음
VehicleHealth authoritative destruction signal/state = 유지
TargetSelect Destroyed 즉시 clear = 유지
Sensor same-ID DestroyedHold = 유지
HUD GetSelectedTargetDisplayInfo = 0
HUD GetTargetDisplayInfo = 0
HUD selected Actor GetActorLocation = 0
HUD Sensor range 기반 Radar scale 재계산 = 0
HUD SetSelectedTarget / candidate refresh = 0
Radar NormalizedPosition 추정 = 없음 / Unavailable 유지
```

P0 완료 기준 판정:

```text
Sensor Runtime owner                         PASS
Passive / Visual / Active Detection          PASS
bounded scan                                 PASS
Live / LastKnown / Lost / Reacquire          PASS
Tactical Analysis / Knowledge                PASS
DestroyedHold                                PASS
actor-free public Snapshot                   PASS
TargetSelect ownership isolation             PASS
HUD read-only Snapshot integration           PASS
official Build                               PASS
Sensor targeted regression                   PASS
related UI asset-free regression             PASS
Content Asset mutation protection            PASS
USER checkpoint protection                   PASS
```

결론:

```text
SEN-P0-07 Technical Acceptance = PASS
CF-FQ-036 SEN-P0-00~07 = Complete
CF-FQ-036 = Done
Current System = Document/Systems/Targeting/SensorContact.md v1.0.0
```

이 완료는 다음을 대신하지 않는다.

```text
CF-FQ-032 Radar / TargetPanel USER Visual
CF-FQ-032 Radar Range / Zoom
CF-FQ-032 동적 Radar Blip
CF-FQ-026 TS-P0-08 USER PIE
```

### 현재 next gate

이 Plan에는 더 이상 current next gate가 없다.

새 Sensor 기능 확장이 필요하면 현재 `Document/Systems/Targeting/SensorContact.md`와 FeatureQueue를 먼저 읽고 별도 lifecycle로 착수한다. 이 완료 Plan의 과거 SEN-P0 gate를 현재 작업으로 재사용하지 않는다.

---

## 7. TargetSelect 보호 계약

CF-FQ-026은 현재:

```text
TS-P0-00~07 Done
TS-P0-08 Remote Technical 3/3 + LOS Prefilter PASS
USER PIE Pending
```

따라서 CF-FQ-036 P0에서 다음을 금지한다.

```text
- TargetSelect 후보 정렬 의미 변경
- 기존 7° / 1200m USER 체감 값을 Sensor 값으로 몰래 대체
- 선택·해제 입력 변경
- 선택 수명 자동 다음 타겟 추가
- TS-P0-08 USER Pending을 Sensor Automation으로 PASS 승격
- 선행 dirty WBP_TargetSelect.uasset 덮어쓰기·저장
```

초기 Sensor는 TargetSelect의 대체 검색기가 아니라 **독립 Contact/Knowledge owner**로 시작한다.
TargetSelect가 Sensor Contact를 후보 source로 사용할지는 대표 workload와 TS-P0-08 종료 후 별도 통합 결정을 한다.

---

## 8. CF-FQ-032 UI 보호 계약

현재 CF-FQ-032는 Defense Production Panel과 Pawn Rebind의 USER Visual Gate 때문에 Paused다.

CF-FQ-036에서 허용:

```text
- Sensor Snapshot C++ 계약
- HUD가 후속으로 읽을 수 있는 Adapter/ViewData 설계
- asset-free Provider contract test가 UI gate를 변경하지 않는 범위의 기술 검증
```

CF-FQ-036에서 금지:

```text
- RadarPanel 최종 USER PASS 추정
- TargetPanel 최종 시각 완료 처리
- UI-P0-03 Done 승격
- UI-P0-04 Source 작업 자동 시작
- 기존 HUD Blueprint Asset 시각 수정·저장
```

Production RadarPanel에 실제 Contact를 표시하는 시각 Gate는 CF-FQ-032 재개 시 USER 확인과 함께 닫는다.

---

## 9. 성능·미래 멀티플레이 원칙

### P0 성능

Sensor 도입이 기존 TargetSelect의 full-world scan을 하나 더 복제하는 구조가 되지 않도록 한다.
다만 `SEN-P0-00`에서 근거 없이 대규모 Spatial Index를 선구축하지 않는다.

원칙:

```text
대표 대상 규모 확인
→ 단순 bounded 검색으로 충분하면 단순 구조
→ 실제 profile/Automation 근거가 생기면 spatial registration/index 승격
```

### 미래 Network

현재는 싱글플레이 Runtime이지만 데이터 소유권은 다음 확장을 막지 않는다.

```text
Server World Truth
→ Player/Vehicle Sensor Knowledge
→ Replicated Contact Snapshot
→ Client HUD
```

클라이언트 HUD가 월드의 모든 Actor 실제 상태를 직접 읽는 구조를 만들지 않는다.

P0에서 Replication 자체는 구현하지 않는다.

---

## 10. 검증 원칙

USER 화면 확인 없이 기술적으로 검증 가능한 항목을 우선한다.

Automation 후보:

```text
- Passive range 안 Target → Contact 생성
- range 밖 → 생성 금지
- direct visible 최소 탐지 계약
- Live → LastKnown → Lost
- Lost 전 reacquire → 같은 Contact 복귀
- Analysis progress 증가
- 가림/범위 이탈 → 즉시 reset 없이 decay
- 재진입 → 남은 progress부터 재개
- Detected → Identified → DetailedScan 승격
- Destroyed → DestroyedHold → 제거
- invalid Actor / EndPlay 안전 정리
- Pause/World cleanup 수명
- Snapshot 안정 정렬과 duplicate Contact 없음
```

필요한 Source 변경 뒤:

```text
Official UE 5.8 Editor Build
→ CarFight.Sensor targeted Automation
→ TargetSelect 의미 영향이 있으면 관련 기존 Automation
→ HUD Provider 의미 영향이 있으면 asset-free UI contract test
```

사용자가 직접 확인하지 않은 Radar/TargetPanel 시각 결과는 USER PASS로 기록하지 않는다.

---

## 11. 현재 보호 체크포인트

```text
CF-FQ-008 WeaponData = Done / Current
CF-FQ-015 VehicleData = Paused / VD-P0-04 USER Tuning Pending
CF-FQ-026 TargetSelect = Paused / TS-P0-08 USER PIE Pending
CF-FQ-029 Launcher = Paused / LM-P0-06 USER PIE
CF-FQ-030 Missile = Ready / Manual PIE Pending
CF-FQ-032 InGame UI = Paused / Defense·Pawn Rebind USER Visual Pending
CF-FQ-034 Fitting = Ready / Mobility·Field UI USER Pending
CF-FQ-035 Inventory = Paused / USER Field UI·Mobility Pending
```

CF-FQ-036 Automation 또는 기술 완료가 위 USER 체크포인트를 대신하지 않는다.

---

## 12. 완료 기준

CF-FQ-036 P0 완료는 다음을 모두 만족해야 한다.

```text
- Sensor/Contact Runtime owner 존재
- Passive Detection 동작
- Active Scan / Tactical Analysis 동작
- Live / LastKnown / Lost lifecycle 동작
- InformationLevel 승격 동작
- Destroyed Contact lifecycle 동작
- read-only Snapshot 제공
- TargetSelect 선택 소유권 침범 없음
- UI Gameplay 판정 중복 없음
- 공식 UE 5.8 Build PASS
- Sensor targeted Automation PASS
- 필요한 기존 회귀 PASS
- Content Asset mutation은 명시적 승인 없이는 0
```

최종 Radar/TargetPanel 시각 USER PASS는 CF-FQ-032의 별도 사용자 검증 범위다.

---

## 13. Changelog

### v0.9.0 - 2026-08-15

- `SEN-P0-07 Technical Acceptance`를 기존 final Build `7dff9da7aaa24e76b0871348762c9d93` PASS, `CarFight.Sensor` `85d008613a104df9b7107795995c6ac5` 14/14 PASS와 UI asset-free 1/1+1/1 PASS를 우선 evidence로 사용해 PASS 판정했다.
- P0-06 이후 Source 의미 변경이 없어 Source 재구현, official Build와 Automation 반복 실행은 하지 않고 current Source read-only audit로 Acceptance를 보강했다.
- actor-free public contract, bounded Detection, lifecycle·Knowledge·DestroyedHold, TargetSelect 선택 소유권 격리와 HUD read-only Snapshot integration이 모두 현재 Source와 일치함을 확인했다.
- `Document/Systems/Targeting/SensorContact.md v1.0.0`을 Current System으로 승격하고 CF-FQ-036을 Done으로 종료했다.
- 이 Plan을 `Historical + Retained Path`로 전환하고 current next gate를 제거했다.
- CF-FQ-032 Radar/TargetPanel USER Visual, Radar Range/Zoom·동적 Blip, CF-FQ-026 TS-P0-08 USER PIE와 다른 USER Pending은 완료로 추정하지 않았다.

### v0.8.0 - 2026-08-15

- `SEN-P0-06 Public Snapshot Integration`을 구현해 TargetSelect가 선택 존재·유효성·TrackState를 유지하고 Target Knowledge와 Radar Contact는 actor-free `FCFSensorSnapshot`만 의미 source로 사용하도록 경계를 고정했다.
- Detected 단계에서 KnownTargetId가 숨겨지는 계약을 보존하면서 선택 Actor를 Snapshot Contact와 연결할 수 있도록 `UCFVehicleSensorComp::TryGetContactIdForActor` read-only ContactId bridge를 추가했다. Actor metadata나 private Runtime data는 공개하지 않는다.
- `FCFTargetHUDData`와 `FCFRadarContactHUDData`에 Sensor ContactId, lifecycle, freshness, analysis, destruction과 실제 상대 위치·거리 ViewData를 추가하고 HUD Provider가 Gameplay 판정을 재계산하지 않도록 구현했다.
- 현재 Radar Range/Zoom 계약이 없으므로 Sensor 사거리로 NormalizedPosition을 추정하지 않고 `NormalizedPositionAvailability=Unavailable`을 명시했다.
- 기존 Radar Contact Canvas의 정적 placeholder가 실제 Sensor Contact처럼 노출되지 않도록 Presenter에서 계속 숨기며 Blueprint/Widget Asset은 수정하지 않았다.
- 신규 `CFSensorHUDTests.cpp v1.0.0`에서 Detected identity 비누출, Identified 공개, TargetSelect selection owner, 실제 VehicleHealth Destroyed 즉시 clear와 Sensor DestroyedHold Radar 보존을 asset-free로 검증했다.
- 최종 official Build `7dff9da7aaa24e76b0871348762c9d93` PASS와 `CarFight.Sensor` Process `85d008613a104df9b7107795995c6ac5` 14/14 PASS를 기록했다.
- `CarFight.UI.UI_P0_03.ViewDataAvailability` Process `67d114255ec943bb8ded2642a40a581c` 1/1과 `ProviderPawnRebind` Process `395c93df5fb947a1a979ca8fa0c74665` 1/1 PASS를 관련 Provider 보호 증거로 기록했다.
- TargetSelect/CFVehiclePawn/Project Config/Content Asset/WBP_TargetSelect와 CF-FQ-032 USER Visual gate를 변경하지 않았다.
- `SEN-P0-06`을 Technical Done으로 승격하고 다음 Gate를 `SEN-P0-07 Technical Acceptance`로 이동했다.

### v0.7.0 - 2026-08-15

- `SEN-P0-05 Destroyed Contact`의 authoritative destruction owner를 감사해 `UCFVehicleHealthComp::OnVehicleDestroyed`와 `IsDestroyed()`만 Sensor 파괴 확정 source로 사용하도록 고정했다.
- 기존 Sensor Contact가 VehicleHealth 파괴 이벤트를 독립 구독하고, 최초 파괴 확정 시 같은 ContactId·Knowledge·AnalysisProgress·마지막 신뢰 위치를 보존한 `DestroyedHold`로 즉시 전환하도록 구현했다.
- `DestroyedHoldTimeSec`을 bounded Actor cursor와 분리된 private destruction timestamp로 진행하고 만료 시 Contact와 event binding을 정리하도록 구현했다.
- Actor Destroy/EndPlay/weak invalid만으로 파괴를 추정하지 않고 P0-03 LastKnown→Lost 경로를 유지했으며, Sensor가 한 번도 알지 못한 pre-destroyed Actor에서 새 Destroyed Contact를 생성하지 않도록 고정했다.
- `CFSensorDestroyedTests.cpp v1.0.0`에 DestroyedHold와 InvalidActor asset-free Automation 2개를 추가했다.
- official Build `954dc0ab844b40f48f2674f2a0ae672a` PASS와 `CarFight.Sensor` Process `4ba274cc90814e5b90d40c33820b3bd0` 13/13 PASS를 기록했다.
- TargetSelect의 기존 Destroyed 즉시 clear, HUD, Content Asset, Project Config와 P0-04 Active Scan/Analysis 의미를 변경하지 않았다.
- `SEN-P0-05`를 Technical Done으로 승격하고 다음 Gate를 `SEN-P0-06 Public Snapshot Integration`으로 이동했다.

### v0.6.0 - 2026-08-15

- 사용자가 실행 중 Editor를 정상 종료한 뒤 official Build `786e47df68fe43f9a924c9c6fbd86726`을 재실행해 `UnrealEditor-NetCore.dll`과 `UnrealEditor-CarFight_Re.dll` 최종 Link까지 Exit Code 0으로 PASS했다.
- 새 linked binary에서 `Tools/RunInvAutomation.ps1 -TestFilter CarFight.Sensor`를 실행했고 Process Job `6fcb3d2dbd5f42489febfff6040611af`가 Engine Exit 0 / 11 Success / 0 Failure로 종료됐다.
- P0-01~03 기존 8개와 P0-04 `ActiveRange / AnalysisDecay / AnalysisProgress` 3개가 모두 PASS해 기존 ContactId/lifecycle/reacquire 회귀가 없음을 확인했다.
- 최종 금지 경계 감사에서도 TargetSelect channel/HUD/selection/InputAction/DestroyedHold coupling 0을 확인했다.
- `SEN-P0-04 Active Scan Analysis`를 Technical Done으로 승격하고 다음 Gate를 `SEN-P0-05 Destroyed Contact`로 이동했다.
- TargetSelect/HUD/CFVehiclePawn/Project Config/Content Asset과 기존 USER Pending 체크포인트는 변경하지 않았다.

### v0.5.0 - 2026-08-15

- `SEN-P0-04 Active Scan Analysis`의 Input/Scanner owner를 감사해 Sensor는 InputAction을 소유하지 않고 `StartActiveScan / StopActiveScan` Runtime command API만 소유하도록 고정했다.
- `ActiveScanRangeCm`을 실행 중 장거리 전방향 Contact Detection에만 사용하고 Active signal Detection과 ECC_Visibility가 필요한 Tactical Analysis gain을 분리했다.
- `AnalysisProgress01`의 gain/decay, 즉시 reset 금지, 재유효화 시 남은 progress 재개와 Identified/DetailedScan threshold 기반 Knowledge 승격을 Source에 구현했다.
- Identified 이상에서만 KnownTargetId/KnownDisplayName을 공개하고 Source InformationLevel을 Sensor Knowledge로 직접 복사하지 않도록 유지했다.
- P0-03 ContactId/lifecycle/reacquire를 보존하고 Active-only Contact가 Scan 종료 시 LastKnown으로 전환되는 private baseline 관측 계약을 추가했다.
- `CFSensorActiveScanTests.cpp`에 ActiveRange, AnalysisProgress, AnalysisDecay 3개 asset-free Automation을 추가했다.
- 현재 정확한 Source official Build `d1cd746d5f3340e1bb1cfe43885cf5e2`에서 UHT와 CFSensorTypes/ActiveScanTests/VehicleSensorComp/VehicleSensorData 및 기존 Sensor C++ compile은 모두 통과했으나 실행 중 `UnrealEditor.exe`가 NetCore/CarFight DLL을 점유해 Link `LNK1104`로 종료됐다.
- 현재 Editor는 Bridge connected이며 이번 세션 AI-owned 증거가 없어 강제 종료하지 않았다. 따라서 P0-04는 Source Implemented / Final Validation Blocked이며 Technical Done으로 승격하지 않았다.
- TargetSelect/HUD/CFVehiclePawn/Project Config/Content Asset/DestroyedHold와 기존 USER Pending 체크포인트를 변경하지 않았다.

### v0.4.0 - 2026-08-15

- `SEN-P0-03 Contact Lifetime`을 Technical Done으로 완료했다.
- P0-02의 미탐지 즉시 제거를 `Live → LastKnown → Lost → Removed`로 교체하고 마지막 신뢰 위치·관측 시각을 LastKnown/Lost 동안 고정했다.
- `FreshnessSeconds`와 `ContactMemoryTimeSec` 수명을 bounded Actor cursor와 분리해 매 Sensor update 진행하도록 구현했다.
- Lost Contact를 최소 한 Actor-free Snapshot에 게시한 뒤 다음 update에서 제거하는 private publication flag를 추가했다.
- Lost 게시 전 동일 Actor 재획득 시 기존 ContactId, InformationLevel, KnownTargetId/Name와 AnalysisProgress를 보존한 Live 복귀를 고정했다.
- 최종 `CFVehicleSensorComp.h v1.2.1`은 의미 변경 없는 formatting-only 정리이며, 그 정확한 Source 상태에서 공식 UE 5.8 Build `fdd231a10ff648839b14b6a2468b295a` PASS와 `CarFight.Sensor` `45c96b20b2984984b753ff69c9b49d3f` 8/8 PASS를 기록했다.
- TargetSelect/HUD/Project Config/Content Asset/DestroyedHold/Active Scan과 기존 USER 체크포인트를 변경하지 않았다.
- 다음 Gate를 `SEN-P0-04 Active Scan Analysis`로 이동했다.

### v0.3.0 - 2026-08-15

- `SEN-P0-02 Passive Detection`을 Technical Done으로 완료했다.
- private weak Actor Runtime Contact와 Actor-free Snapshot 경계를 유지하고 `ICFTargetSelectable`은 자격·대표 위치·Source Metadata에만 제한 재사용했다.
- Level Actor persistent cursor와 `MaxActorScansPerUpdate` bounded budget으로 TargetSelect 20Hz full-world scan과 독립된 Passive 검색을 구현했다.
- Passive 근거리 전방향 탐지는 LOS를 요구하지 않고, Passive 밖 Visual fallback만 `ECC_Visibility` 직접 가시성으로 확인하도록 고정했다.
- 동일 Actor 재관측 시 기존 ContactId를 유지하고 Source `InformationLevel=Identified`가 Sensor Knowledge로 새지 않도록 새 Contact를 `Detected`에서 시작시켰다.
- 최종 공식 UE 5.8 Build `0e5a272bd3f047cf97574914cb9fce92` PASS와 `CarFight.Sensor` `288b8cc1c5224f4bbf57fbf7b23654cd` 6/6 PASS를 기록했다.
- TargetSelect/HUD/Project Config/Content Asset과 기존 USER 체크포인트를 변경하지 않았다.
- 다음 Gate를 `SEN-P0-03 Contact Lifetime`으로 이동했다.

### v0.2.0 - 2026-08-15

- `SEN-P0-00 Foundation Audit`을 완료하고 TargetSelect/Sensor/HUD/Collision/Vehicle 경계와 재사용 타입을 확정했다.
- `SEN-P0-01 Contact Data Contract`를 Technical Done으로 완료해 독립 ContactId, `ECFSensorContactState`, Actor-free `FCFSensorContact/FCFSensorSnapshot`, `FCFSensorConfig/UCFVehicleSensorData`와 `UCFVehicleSensorComp` Foundation을 추가했다.
- `ACFVehiclePawn` 기본 서브오브젝트와 독립 초기화 경계를 연결하되 기존 CoreReady/CombatReady를 변경하지 않았다.
- 공식 UE 5.8 Build `e8c46e25c4aa464da7f9f07730dcba1a` PASS와 `CarFight.Sensor.SEN_P0_01` 3/3 PASS를 기록했다.
- TargetSelect/HUD/Collision Config/Content Asset과 기존 USER 체크포인트를 변경하지 않았다.
- 다음 Gate를 `SEN-P0-02 Passive Detection`으로 이동했다.

### v0.1.0 - 2026-08-15

- CF-FQ-036 차량 센서·Contact Intelligence Runtime을 신규 Active Plan으로 준비했다.
- 현재 실제 Sensor Runtime Provider 부재와 준비된 Radar/Target ViewData 계약을 선행 근거로 기록했다.
- TargetSelect=선택, Sensor=탐지·지식, HUD=표시의 책임 분리를 고정했다.
- Passive 근거리, Active Scan 장거리·전방향, 가시 대상 탐지, Tactical Analysis decay, Live→LastKnown→Lost와 DestroyedHold 방향을 사용자 승인 설계로 기록했다.
- TS-P0-08, CF-FQ-032 USER Visual과 기존 dirty WBP_TargetSelect 보호 조건을 명시했다.
- 새 세션 첫 Gate를 Source mutation 없는 `SEN-P0-00 Foundation Audit`으로 고정했다.

---

## 14. Migration

- v0.9.0부터 이 문서는 `Historical + Retained Path` 완료 기록이다. Sensor의 현재 구현 판단은 실제 Source와 `Document/Systems/Targeting/SensorContact.md v1.0.0`을 우선한다.
- CF-FQ-036은 Done이며 이 Plan의 old SEN-P0 next gate를 현재 착수 작업으로 사용하지 않는다.
- CF-FQ-036을 다시 확장할 필요가 생기면 현재 FeatureQueue/System을 fresh audit한 뒤 새 lifecycle 또는 명시적 후속 Feature를 만든다.
- 현재 단일 Active는 CF-FQ-036 완료로 자동 대체되지 않으며 Paused/Ready 기능은 사용자 선택 없이 승격하지 않는다.
- CF-FQ-032 Radar/TargetPanel USER Visual, Radar Range/Zoom·동적 Blip과 CF-FQ-026 TS-P0-08 USER PIE는 별도 Pending이다.
- 이 Plan은 CF-FQ-032의 Radar/Target UI 설계를 Gameplay Sensor Runtime으로 옮겨 적는 문서가 아니다. UI 의미와 Gameplay 판정 소유권을 분리한다.
- 기존 `ECFTargetRelation`, `ECFTargetInfoLevel`, `ECFTargetTrackState`를 무조건 복제하지 않는다. SEN-P0-00에서 의미가 같을 때 재사용한다.
- 기존 TargetSelect 후보 검색을 CF-FQ-036 착수와 동시에 제거·교체하지 않는다.
- 기존 UI `Radar = Unavailable` 계약은 실제 Sensor Snapshot이 연결되기 전까지 안전 fallback으로 유지한다.
- v0.4.0부터 탐지 상실 즉시 제거는 폐기됐으며 현재 Contact lifecycle은 `Live → LastKnown → Lost → Removed`다.
- `LastKnownWorldLocation`은 LastKnown/Lost에서 실제 Actor를 따라가지 않고 마지막 신뢰 관측 위치만 보존한다. 현재 위치 추정·외삽으로 해석하지 않는다.
- `FreshnessSeconds`와 `ContactMemoryTimeSec`은 bounded scan cursor와 독립적으로 Sensor update마다 진행한다. Actor가 이번 bounded subset에 없다는 이유만으로 LastKnown으로 바꾸지 않는다.
- Lost는 최소 한 Snapshot에 게시한 뒤 다음 update에서 제거된다. Lost 게시 전 재획득은 기존 ContactId와 Sensor Knowledge를 유지한다.
- Production fallback의 Passive/Visual/Active 거리와 lifecycle 시간 기본값이 실제 게임 튜닝 확정을 뜻하지 않는다. Content DataAsset 생성·수치 확정은 별도 명시 범위다.
- Sensor Visual fallback은 일반 `ECC_Visibility`를 사용하며 TargetSelect 전용 Trace Channel을 재사용하지 않는다.
- v0.5.0부터 Active Scan 입력 자산/키 바인딩과 Sensor Runtime owner를 분리한다. Sensor는 Start/Stop command와 실행 상태·탐지·분석·Knowledge만 소유한다.
- Active Scan Contact Detection은 실행 중 Active range 안에서 전방향이며 LOS를 요구하지 않는다. Tactical Analysis gain은 Live + Active range + ECC_Visibility 직접 가시 조건을 추가로 요구한다.
- Analysis progress가 decay로 threshold 아래가 되어도 이미 획득한 InformationLevel/Known Target Knowledge는 강등하지 않는다.
- v0.6.0부터 P0-04 최종 기준은 official Build `786e47df68fe43f9a924c9c6fbd86726` PASS와 `CarFight.Sensor` Process `6fcb3d2dbd5f42489febfff6040611af` 11/11 PASS다. 이전 DLL lock Build `d1cd746d5f3340e1bb1cfe43885cf5e2`는 중간 blocked evidence로만 유지한다.
- v0.7.0부터 차량 파괴 확정은 `UCFVehicleHealthComp::OnVehicleDestroyed` 이벤트와 `IsDestroyed()` 상태만 사용한다. Actor Destroy/EndPlay/weak invalid 자체는 Sensor destruction truth가 아니다.
- DestroyedHold는 기존 Sensor Contact에만 적용하고 새 pre-destroyed Contact를 생성하지 않는다. ContactId·Knowledge·AnalysisProgress·마지막 신뢰 위치를 보존하며 별도 destruction timestamp로 `DestroyedHoldTimeSec`을 진행한다.
- TargetSelect의 Destroyed 즉시 clear와 Sensor DestroyedHold는 독립 수명으로 유지한다. HUD/TargetSelect의 read-only Snapshot 소비 연결은 SEN-P0-06 책임이다.
- P0-05 최종 기준은 official Build `954dc0ab844b40f48f2674f2a0ae672a` PASS와 `CarFight.Sensor` Process `4ba274cc90814e5b90d40c33820b3bd0` 13/13 PASS다.
- v0.8.0부터 Target Knowledge와 Radar Contact의 UI 의미 source는 actor-free `FCFSensorSnapshot`이다. TargetSelect는 선택 존재·유효성·TrackState owner로 유지한다.
- 선택 Actor는 `TryGetContactIdForActor`로 ContactId association에만 사용하고 HUD는 Actor metadata·현재 위치·Sensor private Runtime Contact를 직접 읽지 않는다.
- Radar Range/Zoom 계약이 없는 동안 RelativePositionMeters/DistanceMeters만 Snapshot에서 계산하고 NormalizedPosition은 Unavailable로 유지한다. Passive/Visual/Active Sensor range를 Radar 화면 반경으로 임의 재사용하지 않는다.
- 기존 `CanvasPanel_RadarContacts` 정적 placeholder는 실제 Sensor Blip이 아니므로 P0-06에서도 숨긴다. 동적 Contact 시각화와 USER PASS는 CF-FQ-032 책임이다.
- P0-06 최종 기준은 official Build `7dff9da7aaa24e76b0871348762c9d93` PASS와 `CarFight.Sensor` Process `85d008613a104df9b7107795995c6ac5` 14/14 PASS다. UI Provider 보호 회귀는 `67d114255ec943bb8ded2642a40a581c`, `395c93df5fb947a1a979ca8fa0c74665` 각각 1/1 PASS다.
- 기존 TargetSelect/CFVehiclePawn/Project Config/Content Asset과 USER Pending 체크포인트는 SEN-P0-06 Technical Done에서도 변경하지 않았다.
