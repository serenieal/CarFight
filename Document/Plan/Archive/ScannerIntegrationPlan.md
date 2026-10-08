# Scanner Integration Plan

- 문서 버전: v0.8.0
- 작성일: 2026-08-16
- 최근 갱신일: 2026-08-18
- 문서 상태: Completed / Historical + Retained Path
- Feature: `CF-FQ-037 차량 스캐너 입력·장비 통합` — Done
- 최종 Gate: `SCAN-P0-07 Current System Integration` — Done
- 현재 구현 기준: `Document/Systems/Targeting/SensorContact.md v1.1.0`


---

## 1. 목적

이 Plan은 `CF-FQ-036 차량 센서·Contact Intelligence Runtime`에서 완료한 Sensor Runtime을 다시 설계하지 않고, 실제 플레이 차량의 **Scanner 장비 데이터와 플레이어 입력**에 연결하는 후속 Feature를 관리한다.

현재 Sensor는 이미 다음 Gameplay Runtime을 제공한다.

```text
Passive Detection
Active Scan Detection
Live / LastKnown / Lost / DestroyedHold
Tactical Analysis gain / decay
Detected / Identified / DetailedScan Knowledge
actor-free FCFSensorSnapshot
Target / Radar HUD ViewData read-only 연결
```

하지만 현재 Current System에서는 아래 항목이 의도적으로 후속 범위로 남아 있다.

```text
Scanner InputAction / 실제 키 입력
실제 Scanner 장비 성능 Source
차량 장비 상태에서 SensorConfig를 해석하는 경로
장비 변경과 Sensor Runtime 재적용 경로
실제 SensorData Content Asset 튜닝값
```

따라서 이번 Feature의 목적은 다음 데이터 흐름을 확립하는 것이다.

```text
차량에 적용된 Scanner 장비 또는 승인된 Scanner 설정 Source
        ↓
검증된 Sensor Config 해석
        ↓
UCFVehicleSensorComp Runtime 적용
        ↓
플레이어 Scanner 입력
        ↓
StartActiveScan / StopActiveScan command
        ↓
기존 CF-FQ-036 Detection / Analysis / Knowledge Runtime
```

---

## 2. 시작 기준선

### 2.1 완료된 Current System

`CF-FQ-036`은 Done이며 현재 구현은 다음 문서가 소유한다.

```text
Document/Systems/Targeting/SensorContact.md v1.0.0
```

대표 완료 증거:

```text
SEN-P0-00~07 Technical Acceptance PASS
Official UE 5.8 Build: 7dff9da7aaa24e76b0871348762c9d93 PASS
CarFight.Sensor: 85d008613a104df9b7107795995c6ac5 / 14/14 PASS
UI asset-free Provider regression: 1/1 + 1/1 PASS
```

이 증거는 이번 Feature의 시작 기준선이다. `CF-FQ-037` 착수를 이유로 기존 SEN-P0 Gate를 재실행하거나 CF-FQ-036을 다시 Active로 열지 않는다.

### 2.2 현재 확정된 책임 경계

```text
UCFVehicleSensorComp
= Detection / Contact lifecycle / Analysis / Knowledge Runtime owner

ACFVehiclePawn 또는 별도 Input Adapter
= Enhanced Input 입력을 Gameplay command로 변환하는 owner

Scanner 장비 데이터 계층
= Sensor Runtime이 소비할 설정 Source를 제공하는 owner

Vehicle Fitting / Inventory
= 실제 장비 소유·장착 구조가 Scanner를 표현할 수 있는지에 대한 통합 owner

TargetSelect
= 후보 검색 / 선택 / TrackState owner

HUD
= Sensor Snapshot read-only 표시
```

고정 금지사항:

```text
Sensor Component가 InputAction을 직접 Bind하지 않는다.
TargetSelect가 Scanner 입력이나 SensorConfig를 소유하지 않는다.
HUD가 Scanner 성능이나 Detection 결과를 재계산하지 않는다.
장비 계층이 Contact lifecycle이나 Knowledge 계산을 복제하지 않는다.
```

---

## 3. 이번 Feature 범위

### 포함

```text
1. 기존 Enhanced Input 구조와 Scanner command owner 감사
2. 현재 VehicleData / Fitting / Inventory 구조에서 Scanner 장비를 표현할 수 있는지 감사
3. 기존 UCFVehicleSensorData / FCFSensorConfig 재사용 가능성 감사
4. Scanner 장비 또는 승인된 설정 Source → SensorConfig 해석 계약
5. 차량 Runtime 초기화 시 Sensor Config 적용 경로
6. 런타임 장비 변경을 지원해야 하는 경우 안전한 Sensor Config 재적용 경로
7. 플레이어 입력 → StartActiveScan / StopActiveScan 연결
8. 입력 반복·중복·비활성 Scanner 상태의 안전한 처리
9. 기존 Sensor/TargetSelect/HUD/Fitting 의미 보존 Automation
10. 공식 UE 5.8 Editor Build와 가능한 Technical Acceptance
11. 실제 Scanner 입력과 탐지 차이에 대한 USER PIE를 기술 검증과 분리
```

### 제외

```text
CF-FQ-032 Radar Range / Zoom
CF-FQ-032 동적 Radar Blip Widget
CF-FQ-032 TargetPanel / Radar USER Visual
Sensor power / energy / heat 소비
Missile / Utility가 Sensor Contact를 실제 Gameplay target source로 소비하는 기능
AI Sensor 소비
TargetSelect 검색을 Sensor Snapshot으로 교체
Network Replication / Server-authoritative Sensor
Scanner 밸런스 대량 튜닝
새 차량·새 무기 양산
```

위 제외 범위는 `CF-FQ-037` 완료 조건에 포함하지 않는다.

---

## 4. 핵심 설계 원칙

### 4.1 기존 Sensor Runtime을 재사용한다

`UCFVehicleSensorComp`의 다음 command API와 의미를 우선 재사용한다.

```text
StartActiveScan()
StopActiveScan()
IsActiveScanRunning()
GetActiveScanRemainingSeconds()
```

첫 감사에서 실제 Source 시그니처를 다시 확인한 뒤 필요한 최소 연결만 추가한다.

### 4.2 Scanner 장비 전용 데이터 구조를 먼저 만들지 않는다

현재 `UCFVehicleSensorData`, `FCFSensorConfig`, VehicleData, Fitting/Inventory 관련 구조가 이미 존재한다.

따라서 `SCAN-P0-00`에서는 먼저 아래를 판정한다.

```text
A. UCFVehicleSensorData 자체를 Scanner 정적 데이터로 재사용 가능한가
B. 기존 Fitting의 장비 정의가 UCFVehicleSensorData를 참조할 수 있는가
C. VehicleData에 기본 Scanner Source를 두어야 하는가
D. 런타임 Fitting Snapshot에서 Scanner를 해석할 수 있는가
```

감사 전에는 `UCFScannerData`, `UCFScannerEquipmentData`와 같은 병렬 DataAsset 클래스를 새로 만들지 않는다.

### 4.3 장비 성능과 CPU scan budget을 섞지 않는다

`MaxActorScansPerUpdate`는 CPU 작업 예산이다.
Scanner의 탐지 성능이나 고급 장비 등급을 표현하는 플레이어-facing 성능값으로 사용하지 않는다.

Scanner 장비 성능은 우선 기존 `FCFSensorConfig`의 Gameplay 의미 필드로 해석한다.

```text
PassiveDetectionRangeCm
ActiveScanRangeCm
VisualDetectionRangeCm
ContactMemoryTimeSec
DestroyedHoldTimeSec
ActiveScanDurationSec
AnalysisGainPerSec
AnalysisDecayPerSec
IdentifiedThreshold
DetailedScanThreshold
```

실제 밸런스 수치는 이번 Feature 기술 계약과 분리하며 근거 없는 수치를 임의 확정하지 않는다.

### 4.4 Config 재적용은 Contact 의미를 함부로 지우지 않는다

런타임 장비 교체가 지원되는 구조라면 Scanner Config 변경 시 다음을 별도 판단해야 한다.

```text
현재 Active Scan을 중단할지
기존 Live / LastKnown Contact를 유지할지
AnalysisProgress / 획득 Knowledge를 유지할지
range 감소로 즉시 Contact를 제거하지 않고 기존 lifecycle을 따를지
```

`SCAN-P0-00`에서 현재 Sensor 초기화 API와 Fitting commit lifecycle을 확인하기 전에는 이 정책을 추정하지 않는다.

---

## 5. 작업 체크포인트

| Gate | 상태 | 목적 | 대표 완료 조건 |
| --- | --- | --- | --- |
| `SCAN-P0-00 Foundation Audit` | **Technical Done** | 기존 Input·SensorData·VehicleData·Fitting 소유권 감사 | Source mutation 0 / 실제 재사용 경로와 최소 변경 파일 확정 |
| `SCAN-P0-01 Scanner Data Contract` | **Technical Done** | Scanner 장비/설정 Source와 SensorConfig 해석 계약 확정 | 중복 DataAsset 없이 명확한 Source/Validation 계약 |
| `SCAN-P0-02 Runtime Config Apply` | **Technical Done** | 차량 초기화·필요 시 장비 변경에서 SensorConfig 적용 | 기존 Sensor lifecycle 의미 보존 + invalid/fallback 안전 경로 |
| `SCAN-P0-03 Input Command Integration` | **Technical Done** | Enhanced Input caller → Sensor Start/Stop command 연결 | Sensor 직접 Input bind 금지 + 중복 입력 안전 처리 |
| `SCAN-P0-04 Fitting Integration` | **Technical Done** | 현재 Fitting/Inventory 구조에서 Scanner 장착 상태 연결 | 기존 FFIT/Inventory checkpoint 의미 비파괴 + 원자적 적용 경계 |
| `SCAN-P0-05 Technical Validation` | **Technical Done** | 신규 계약 Automation + 기존 Sensor/TargetSelect/Fitting 회귀 + 공식 Build | 관련 기술 회귀 PASS / USER PIE와 분리 |
| `SCAN-P0-06 USER PIE Acceptance` | **USER Done** | 실제 Scanner 조작과 Passive/Active 체감 확인 | 사용자가 직접 확인한 항목만 USER PASS |
| `SCAN-P0-07 Current System Integration` | **Done** | 완료 범위를 Current Knowledge로 승격 | SensorContact 및 필요한 Systems/ProjectSSOT 갱신 후 CF-FQ-037 Done 판정 |


---

## 6. 완료 Gate 기록 — SCAN-P0-00 Foundation Audit

### 목적

새 데이터 클래스나 InputAction을 만들기 전에 현재 저장소가 이미 제공하는 경로를 확인하고, 가장 작은 구현 구조를 확정한다.

### 읽기 대상

최소한 다음 Current Source와 문서를 확인한다.

```text
Document/Systems/Targeting/SensorContact.md
UE/Source/CarFight_Re/Public/CFSensorTypes.h
UE/Source/CarFight_Re/Public/CFVehicleSensorData.h
UE/Source/CarFight_Re/Private/CFVehicleSensorData.cpp
UE/Source/CarFight_Re/Public/CFVehicleSensorComp.h
UE/Source/CarFight_Re/Private/CFVehicleSensorComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

Fitting/VehicleData/Input 관련 파일은 이름을 추정해 무작정 확대하지 않고, 위 Source에서 실제 참조를 확인하거나 bounded search로 정확한 owner를 찾은 뒤 필요한 파일만 추가한다.

### 감사 질문

```text
Q1. 현재 Enhanced Input binding owner는 정확히 어디인가?
Q2. Scanner 입력을 기존 Pawn binding에 추가하는 것이 맞는가, 별도 Input Adapter가 이미 존재하는가?
Q3. UCFVehicleSensorData가 실제 Scanner equipment static data 역할까지 담당해도 책임이 깨지지 않는가?
Q4. 현재 VehicleData에 SensorData 참조가 이미 존재하는가?
Q5. 현재 Fitting Snapshot/EquipmentPreset 구조가 비무기 Utility/Scanner를 표현할 수 있는가?
Q6. 장착 Scanner 변경 후 Runtime config를 다시 적용할 공식 hook이 존재하는가?
Q7. Config 재적용 시 기존 Contact/Knowledge/Active Scan에 어떤 현재 의미가 이미 코드에 존재하는가?
Q8. Scanner가 없는 차량의 현재 fallback 0-range 동작을 그대로 유지할 수 있는가?
Q9. 어떤 신규 Automation이 가장 작은 의미 회귀를 증명하는가?
```

### 산출물

`SCAN-P0-00` 종료 시 다음을 대표 Plan에 기록한다.

```text
1. 입력 owner와 정확한 호출 흐름
2. Scanner static/runtime data owner
3. Fitting 연계 가능 여부
4. 재사용할 기존 타입과 새 타입이 정말 필요한 지점
5. 수정 허용 파일 목록
6. 보호 파일/기능 목록
7. Build/Automation 계획
8. 다음 Gate SCAN-P0-01의 정확한 구현 범위
```

### 금지

```text
Source 수정
Config 수정
Content Asset 수정
Blueprint 저장
InputAction 생성
Build
PIE
USER PASS 추정
```

이번 Gate는 read-only 구조 감사다.

### 6.1 감사 결과 — 2026-08-16

`SCAN-P0-00`은 Source·Config·Content Asset·Blueprint·InputAction·Build·PIE mutation 없이 **Technical Done**으로 판정한다.

#### Q1~Q2. Enhanced Input owner와 Scanner command caller

현재 Gameplay Enhanced Input의 실제 owner는 `ACFVehiclePawn::SetupPlayerInputComponent()`다. `DefaultInputMappingContext` 등록도 Pawn의 `RegisterDefaultInputMappingContext()`가 소유하며 Fire / SelectTarget / ClearTarget 역시 Pawn이 입력을 받아 Gameplay command로 변환한다.

```text
Enhanced Input
→ ACFVehiclePawn::SetupPlayerInputComponent
→ Pawn input handler
→ Gameplay component command
```

별도 Scanner Input Adapter는 현재 Source에서 확인되지 않았다. 따라서 후속 `SCAN-P0-03`의 최소 구조는 기존 Pawn binding 패턴을 재사용하는 것이며, `UCFVehicleSensorComp`가 InputAction을 직접 Bind하지 않는다. 이번 Audit에서는 InputAction asset을 생성하지 않았다.

#### Q3. Scanner static/runtime data owner

`UCFVehicleSensorData`는 이미 `FCFSensorConfig`와 DataValidation을 소유하는 정적 Sensor 설정 DataAsset이다. Scanner 탐지 성능의 정적 payload로 **재사용 가능**하며 `UCFScannerData`, `UCFScannerEquipmentData` 같은 병렬 DataAsset 클래스는 현재 필요하지 않다.

다만 `UCFVehicleSensorData` 자체는 Inventory 장비 identity가 아니다. EquipmentId, 장착 호환, Item Instance 소유권은 기존 Equipment/Fitting/Inventory 계층이 계속 소유해야 한다.

```text
UCFVehicleSensorData
= Scanner/Sensor 정적 성능 payload

UCFEquipmentPresetData + Fitting/Inventory
= 장비 identity / 장착 / 실제 Item ownership

UCFVehicleSensorComp
= 적용된 config를 소비하는 Detection / Contact / Analysis / Knowledge Runtime
```

#### Q4. VehicleData 기본 Scanner Source

현재 `UCFVehicleData`에는 `SensorData` 또는 Scanner 전용 참조가 없다. Scanner가 장착 장비라면 VehicleData에 새 기본 Sensor 슬롯을 선제 추가할 근거도 아직 없다.

P0 기본 방향은 **장비 Source가 없으면 현재 `UCFVehicleSensorComp::FallbackSensorConfig`의 0-range 안전 동작을 유지**하는 것이다. VehicleData 기본 Scanner는 실제 게임 설계 요구가 생길 때 별도 판단한다.

#### Q5. Fitting / Inventory가 현재 Scanner를 표현할 수 있는가

부분적으로만 가능하다.

```text
존재함
- ECFVehicleMountType::Utility
- VehicleData.MountProfiles
- UCFEquipmentItemData → UCFEquipmentPresetData
- Inventory Fit Adapter → 기존 Fitting Snapshot

부족함
- UCFEquipmentPresetData는 TurretMountData + WeaponData만 참조
- HasCompleteEquipmentData()는 두 참조를 모두 필수로 판정
- UCFVehicleFittingData도 이 완성 조건을 계약 검증과 Snapshot 생성에서 강제
- FCFResolvedFittingMount는 TurretMountData / WeaponData만 해석
- 현재 Snapshot에 SensorData 참조가 없음
```

따라서 `Utility` enum만 이용해 Scanner 프리셋을 만드는 것은 현재 계약상 불가능하다. `SCAN-P0-01`은 기존 EquipmentPreset/Fitting 경로가 `UCFVehicleSensorData`를 운반할 수 있도록 **최소 확장**해야 한다.

Inventory의 `UCFEquipmentItemData`는 이미 `UCFEquipmentPresetData`를 강타입 참조하므로, Scanner payload가 EquipmentPreset에 추가된다면 Inventory Item Definition에 병렬 SensorData 참조를 중복 추가하지 않는 방향을 우선한다.

#### Q6. 초기 적용과 장비 변경 Runtime hook

초기 차량 Runtime에는 다음 hook이 이미 존재한다.

```text
ACFVehiclePawn::InitializeVehicleRuntime
→ Fitting Prepare / Mass Verify / Fitting Commit
→ UCFVehicleSensorComp::InitializeSensorRuntime
```

하지만 현재 Sensor 초기화는 Applied Fitting Snapshot에서 Scanner Source를 해석하지 않는다. Field Fitting의 정식 원자 transaction도 현재 `Weapon + Defense + optional Mass`만 Commit/Compensation 대상으로 다루며 Sensor는 participant가 아니다.

따라서 **초기 적용 위치는 존재하지만 Scanner Source 연결은 없음**, **장비 변경용 공식 Sensor reapply hook도 없음**으로 판정한다.

#### Q7. 현재 Config 재적용 의미

`UCFVehicleSensorComp::InitializeSensorRuntime()`을 장비 교체 후 그대로 다시 호출하는 것은 안전한 hot reapply가 아니다. 현재 구현은 초기화할 때 다음 Runtime state를 비운다.

```text
RuntimeContacts.Reset()
Active Scan 중지 / Remaining 0
Actor cursor reset
Contact에 보존되던 Analysis / Knowledge도 Contact와 함께 소멸
```

따라서 `SCAN-P0-02`에서 Config 재적용 API와 의미를 별도로 설계한다. 기존 Contact/Knowledge를 지울지에 대한 정책을 Audit에서 임의 확정하지 않으며, 단순 `InitializeSensorRuntime()` 재호출을 장비 변경 해법으로 사용하지 않는다.

#### Q8. Scanner가 없는 차량의 fallback

현재 `FallbackSensorConfig`는 거리와 Active Scan 값이 0인 상태에서도 Config 계약 자체는 유효하다. 이 경우 Sensor Runtime은 준비될 수 있지만 Passive/Visual 탐지 tick이 불필요하게 활성화되지 않고 Active Scan도 시작되지 않는다.

이 **0-range scanner-less fallback**은 기존 차량 호환 경계로 보존한다. 감사 중 임의 tuning 값은 만들지 않았다.

#### Q9. 최소 Automation 방향

P0-01~05 기술 검증은 실제 Content Asset 생성 없이 Transient UObject/구조체를 우선 사용한다.

```text
SCAN-P0-01
- Utility Scanner EquipmentPreset의 SensorData payload 검증
- 기존 Turret+Weapon preset validity 비회귀
- FittingSnapshot에서 SensorData가 VehicleData.MountProfiles 순서대로 결정론적으로 보존되는지 검증
- scanner-less mount / invalid payload의 명시적 validation

SCAN-P0-02
- scanner-less 0-range fallback
- valid SensorData config 적용
- invalid source의 안전 처리
- config 재적용 시 확정된 Contact/Knowledge/ActiveScan 정책 회귀

SCAN-P0-03
- 실제 InputAction asset을 테스트 fixture로 만들지 않고 Gameplay command wrapper와 Sensor Start/Stop 의미를 우선 자동화

SCAN-P0-04~05
- Field Fitting에 Sensor가 transaction participant로 추가될 경우 Commit/Compensation 원자성
- 기존 CarFight.Sensor / Fitting / Inventory / TargetSelect 영향 회귀
```

### 6.2 재사용 타입과 최소 변경 경계

`SCAN-P0-01`에서 우선 재사용한다.

```text
UCFVehicleSensorData
FCFSensorConfig
UCFEquipmentPresetData
FCFResolvedFittingMount
FCFVehicleFittingSnapshot
UCFVehicleFittingData
UCFEquipmentItemData → EquipmentPresetData 기존 연결
```

새 Scanner DataAsset 클래스와 새 Inventory Scanner Definition 클래스는 만들지 않는다.

P0-01의 최소 Source 후보:

```text
UE/Source/CarFight_Re/Public/CFEquipmentPresetData.h
UE/Source/CarFight_Re/Private/CFEquipmentPresetData.cpp
UE/Source/CarFight_Re/Public/CFFittingTypes.h
UE/Source/CarFight_Re/Private/CFVehicleFittingData.cpp
신규 asset-free targeted test 1개가 필요하면 32자 이하 이름 사용
```

현재 감사 기준으로 아래 파일은 P0-01에서 **수정 불필요 우선**이다.

```text
CFInventoryItemData.h/.cpp
CFInventoryFitAdapter.h/.cpp
CFVehicleSensorComp.h/.cpp
CFVehiclePawn.h/.cpp
UCFVehicleData 관련 Source
Content Asset / Blueprint / InputAction
```

실제 구현 중 새로운 책임이 확인되지 않는 한 위 범위를 확대하지 않는다.

### 6.3 SCAN-P0-01의 정확한 구현 범위

다음 Gate에서는 다음 정적 계약만 확정한다.

```text
1. UCFEquipmentPresetData가 선택적 UCFVehicleSensorData payload를 표현한다.
2. 기존 Turret+Weapon preset은 현재 validity와 호환성을 유지한다.
3. Utility Scanner preset은 Turret/Weapon을 억지로 요구하지 않는 mount-aware validation을 갖는다.
4. FCFResolvedFittingMount / FittingSnapshot이 해석된 SensorData 참조를 결정론적으로 보존한다.
5. Inventory는 기존 EquipmentItemData → EquipmentPresetData 경로를 그대로 재사용한다.
6. 복수 Scanner payload가 동시에 해석될 경우 정책은 first-wins로 임의 처리하지 않고 P0-01 validation 계약에서 명시적으로 결정한다.
7. 실제 Sensor Runtime 적용, hot reapply, InputAction binding은 각각 P0-02/P0-03 이후로 남긴다.
8. Scanner 질량·등급·탐지 수치 등 근거 없는 gameplay tuning은 추가하지 않는다.
```

`SCAN-P0-01` 완료 전에도 새 Scanner Content Asset은 필요하지 않으며 Transient DataAsset으로 계약을 검증할 수 있다.

### 6.4 SCAN-P0-01 Closure — 2026-08-16

`SCAN-P0-01 Scanner Data Contract`는 **Technical Done**으로 종료한다.

구현 계약:

```text
- UCFVehicleSensorData / FCFSensorConfig를 Scanner 정적 성능 payload로 재사용
- UCFEquipmentPresetData는 무장 패키지와 Utility Scanner 패키지를 상호 배타적으로 표현
- Utility Scanner는 DefaultSensorData만 사용하고 RequiredWeaponSize=None을 요구
- 기존 TurretMountData + WeaponData 무장 패키지 호환 유지
- FCFResolvedFittingMount.SensorData와 FCFVehicleFittingSnapshot.ResolvedSensorData 추가
- Scanner Source 0개 = null 허용
- Scanner Source 1개 = top-level ResolvedSensorData로 결정론적 해석
- Scanner Source 2개 이상 = MultipleSensorSources 오류 / first-wins 금지 / top-level null
- invalid SensorConfig는 장착·Fitting validation에서 거부
- Scanner 질량·등급·탐지 수치 등 근거 없는 gameplay tuning 추가 0
- Inventory는 기존 EquipmentItemData → EquipmentPresetData 경로 그대로 유지
```

검증:

```text
Official UE 5.8 Editor Build
= aecd7ab0a1c54bd2b68327b292d117e7 PASS / Exit 0

Asset-free Automation
= d014a29d723f4831a6c00e1795a392d6 PASS
= CarFight.Scanner.SCAN_P0_01.DataContract
= CarFight.Fitting.FIT_P0_02.DataContract
= CarFight.Fitting.FIT_P0_03.Compatibility
= CarFight.Fitting.FIT_P0_03.MassSnapshot
```

Automation은 작업 전용 self-cleaning runner로 실행했으며 실행 후 runner 파일은 남기지 않았다. 공용 검증 도구로 승격하지 않는다.

Content Asset / Blueprint / InputAction / PIE 변경은 0이며 USER PASS를 추가하지 않았다. `CFEquipmentPresetData.h`와 `CFFittingTypes.h`의 P0-01 도입 formatting은 정규화했다. `CFVehicleFittingData.cpp`에는 기존 기능 의미와 Build/Automation 결과에 영향을 주지 않는 일부 들여쓰기 cosmetic debt가 남아 있으며 P0-01 Technical Acceptance 차단 사유로 보지 않는다.

---

## 7. 보호 체크포인트

이번 Feature 전체에서 다음 상태를 자동 승격하거나 다시 검증 완료로 추정하지 않는다.

```text
CF-FQ-036 SensorContact = Done / Current System 유지
CF-FQ-032 = Paused / Defense·Pawn Rebind USER Visual Pending
CF-FQ-032 Radar Range/Zoom·동적 Blip·Radar/TargetPanel USER Visual = 별도 Pending
CF-FQ-026 = Paused / TS-P0-08 USER PIE Pending
CF-FQ-015 = Paused / VD-P0-04 USER Tuning Pending
CF-FQ-029 = Paused / LM-P0-06 USER PIE
CF-FQ-030 = Ready / Manual PIE Pending
CF-FQ-034 = Ready / Mobility·Field UI USER Pending
CF-FQ-035 = Paused / USER Field UI·Mobility Pending
```

기존 dirty 보호:

```text
UE/Content/CarFight/UI/WBP_TargetSelect.uasset
UE/Source/CarFight_Re/Private/CFHUDDataTests.cpp
기타 작업 시작 전 존재하던 모든 dirty/untracked 사용자·기능 작업
```

공용 통합 파일인 `CFVehiclePawn.h/.cpp`, `CFVehicleData.h/.cpp`, `CFVehicleFittingComp.h/.cpp`를 수정해야 한다면 현재 내용을 fresh read하고 이번 Feature에 필요한 최소 anchor만 변경한다.

---

## 8. 검증 원칙

### C++ 변경이 발생하면

```text
1. Git diff 검수
2. 공식 UE 5.8 Editor Build
3. 신규 Scanner targeted Automation
4. 기존 CarFight.Sensor 회귀
5. 영향받은 Fitting/Input/TargetSelect 관련 targeted 회귀
6. USER PIE가 필요한 항목은 별도 Pending 유지
```

공식 Build entry:

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

Editor가 실행 중일 경우 process 존재만으로 종료하지 않고 CodeWorkGate의 binary interference + ownership 규칙을 따른다.

### USER PIE에서만 판정할 것

```text
실제 입력 키/버튼 체감
Passive와 Active 탐지 범위 체감
Active Scan 시작·종료 피드백 체감
Scanner 장비 변경 시 플레이 감각
최종 HUD/Radar 시각 적합성
```

마지막 항목의 Radar/TargetPanel 시각 자체는 CF-FQ-032 소유이며 CF-FQ-037 Done 조건으로 끌어오지 않는다.

---

## 9. SCAN-P0-02 Closure — 2026-08-16

`SCAN-P0-02 Runtime Config Apply`는 구현과 기술 검증을 완료해 **Technical Done**으로 종료한다.

구현 Source:

```text
UE/Source/CarFight_Re/Public/CFVehicleSensorComp.h v1.6.0
UE/Source/CarFight_Re/Private/CFVehicleSensorComp.cpp v1.6.0
UE/Source/CarFight_Re/Private/CFScannerRuntimeTests.cpp v1.0.0
```

확정 Runtime 계약:

```text
- UCFVehicleSensorComp::ApplySensorData(UCFVehicleSensorData*)를 non-destructive apply/reapply API로 추가
- Runtime Ready 전 valid SensorData/null fallback 적용은 Source만 선택하고 암묵적으로 InitializeSensorRuntime()을 호출하지 않음
- explicit invalid SensorData 또는 invalid fallback은 기존 Source/Runtime/Snapshot을 바꾸지 않고 false로 원자 거부
- Runtime Ready 후에는 AppliedSensorConfig 사본을 실제 Runtime config로 소비해 Source UObject 직접 변경이 Apply를 우회하지 못함
- hot reapply에서 InitializeSensorRuntime()을 재호출하지 않아 RuntimeContacts, ContactId, 획득 Knowledge, AnalysisProgress, LastKnown/DestroyedHold 의미를 보존
- Passive/Visual 탐지 능력 감소 시 기존 Live Contact를 즉시 삭제하지 않고 LastKnown lifecycle로 넘김
- Active range 감소 시 active-only Live Contact를 LastKnown으로 넘기되 baseline Contact는 불필요하게 내리지 않음
- 새 Config가 Active Scan을 지원하지 않으면 실행 중 Scan을 안전하게 중단하고 Remaining=0
- 새 Config가 Active Scan을 계속 지원해도 Remaining은 새 duration으로 clamp할 뿐 장비 변경 때문에 증가시키지 않음
- 적용 뒤 Passive work / Active running / 남은 Contact lifecycle 필요 여부로 Tick을 재계산하고 fresh Snapshot을 게시
- scanner-less null은 유효한 0-range Fallback으로 적용하며 남은 Contact를 즉시 삭제하지 않음
```

검증:

```text
Official UE 5.8 Editor Build
= d670926be7c4498dbd15b12601f8505b PASS

P0-02 targeted Automation
= CarFight.Scanner.SCAN_P0_02.ConfigApply 1/1 PASS

Current Sensor regression
= CarFight.Sensor 14/14 PASS / Failure 0
= result: UE/Saved/CarFight/ScannerP002ValidationResult.json
= result sha256: 7b5f908a60fda04b5f9a1f9ef23c15cceae47e622fad8d4cbba99d5a9ceb4be9
= validation finalizer process: 76b2577325a64cf2a3a0b64069474abe PASS
```

검증 실행 중 Automation은 14/14 Success와 `Automation Test Queue Empty`, `TestExit status 0`까지 정상 완료했고 CarFight `UnrealEditor.exe`도 종료됐다. 이전 task-local outer runner가 `Start-Process -Wait`의 process-tree wait에서 남는 실행 인프라 문제는 테스트 결과와 분리했으며, read-only finalizer에서 `remaining_carfight_editor_count=0`을 재확인했다. 강제 종료, Save, Product Asset mutation은 사용하지 않았다.

P0-02에서 Content Asset, Blueprint, InputAction, Scanner tuning과 PIE 변경은 0이며 USER PASS를 추가하지 않았다. `FCFVehicleFittingSnapshot.ResolvedSensorData`를 실제 Vehicle Sensor에 연결하고 Field Fitting transaction participant로 통합하는 책임은 계속 `SCAN-P0-04`에 남긴다.

---

## 10. SCAN-P0-03 Closure — 2026-08-16

`SCAN-P0-03 Input Command Integration`은 실제 Enhanced Input 자산과 저장된 Mapping Context까지 연결하고 기술 검증을 완료해 **Technical Done**으로 종료한다.

구현 Source:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h v2.147.0
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp v2.147.0
UE/Source/CarFight_Re/Private/CFScannerInputTests.cpp v1.1.0
```

P0 입력 계약:

```text
InputAction = /Game/CarFight/Input/IA_ActiveScan.IA_ActiveScan
ValueType = Boolean
ConsumeInput = true
Trigger = InputTriggerPressed
Keyboard default = V
동작 = V 1회 입력 → StartActiveScan() → ActiveScanDurationSec 동안 실행 → Sensor Runtime이 자동 종료
별도 Stop 키 = 없음
```

책임 경계:

```text
ACFVehiclePawn
= Enhanced Input Bind owner
= IA_ActiveScan Started를 RequestStartActiveScan() Gameplay command로 변환

UCFVehicleSensorComp
= StartActiveScan()/StopActiveScan() Runtime owner
= InputAction 직접 Bind 금지

RequestStartActiveScan()/RequestStopActiveScan()
= SensorData Apply/재적용 금지
= InitializeSensorRuntime()/ResetSensorRuntime 호출 금지
= Contact/Knowledge/Analysis 초기화 금지

RequestStopActiveScan()
= P0 기본 키에는 연결하지 않음
= 장비 교체·상태 전환·향후 명시적 취소를 위한 Gameplay command로 유지
```

실제 Content 변경:

```text
신규: UE/Content/CarFight/Input/IA_ActiveScan.uasset
수정: UE/Content/CarFight/Input/IMC_Vehicle_Default.uasset

IMC_Vehicle_Default mapping count
= 46 → 47

신규 mapping
= IA_ActiveScan <- V
```

Persisted AssetDump 재검증:

```text
IA_ActiveScan
= InputAction
= boolean
= consume_input true
= InputTriggerPressed 1개

IMC_Vehicle_Default
= mapping_count 47
= IA_ActiveScan / V / key_valid=true
```

`IA_ActiveScan` dump의 `LastValue` unsupported warning은 `InputTriggerPressed` 내부 `FInputActionValue` 관측 표현에 대한 AssetDump warning이며 InputAction 생성·저장·Automation PASS를 변경하지 않는다.

검증:

```text
Official UE 5.8 Editor Build
= b84e8dabcb784b30a005fb120af5cf4d PASS / Exit 0

P0-03 Actual Input Integration Validation
= process 4d9a1e5ad12f416db18b1a069694c241 PASS / Exit 0
= CarFight.Scanner.SCAN_P0_03.InputAsset 1/1 PASS
= CarFight.Scanner.SCAN_P0_03.InputCommand 1/1 PASS
= CarFight.Sensor 14/14 PASS
= CarFight.TargetSelect.TS_P0_01.RuntimeContract 1/1 PASS

Result
= UE/Saved/CarFight/ScannerP003AssetValidationResult.json
= sha256 f55a0d06e38a8dbb5d5de3bda1cb8be0f70cd629197dec6bffb9dc0e0e9e1f57
```

Blueprint·DataAsset·Scanner tuning 변경은 0이다. InputAction/IMC는 P0-03이 소유한 실제 입력 통합 범위로 명시적으로 저장했다. live Editor를 시작하거나 종료하지 않았으며 headless Automation/AssetDump만 사용했다.

실제 `V` 키 조작감과 Active Scan 시작·종료 피드백 체감은 사용자가 직접 확인하지 않았으므로 **USER PASS를 추가하지 않는다**. 이는 `SCAN-P0-06 USER PIE Acceptance`에서 별도 판정한다.

### 다음 세션 시작점

현재 Gate는 `SCAN-P0-04 Fitting Integration`이다.

P0-04는 P0-01의 `FCFVehicleFittingSnapshot.ResolvedSensorData`를 P0-02의 `UCFVehicleSensorComp::ApplySensorData()`에 실제 연결하고, 필요 시 Field Fitting transaction에 Sensor participant를 추가하는 통합 범위만 소유한다.

P0-04 기본 계약:

```text
1. 초기 차량 Runtime에서 검증된 Applied Fitting Snapshot의 ResolvedSensorData를 VehicleSensorComp에 적용한다.
2. Scanner가 없는 Snapshot은 P0-02의 scanner-less null/fallback 의미를 사용한다.
3. invalid Sensor Source는 기존 적용 Sensor Runtime을 훼손하지 않고 원자적으로 거부한다.
4. Field Fitting에서 Scanner 장착·해제가 허용되면 Sensor apply/reapply를 기존 Weapon/Defense/Mass transaction과 같은 성공·보상 경계 안에 둔다.
5. 장비 변경 때문에 InitializeSensorRuntime()을 hot reapply 경로로 호출하지 않는다.
6. P0-02의 ContactId/Knowledge/Analysis/LastKnown/DestroyedHold 보존 의미를 그대로 유지한다.
7. P0-03의 IA_ActiveScan/V 입력 계약을 변경하지 않는다.
8. 기존 FFIT/Inventory checkpoint와 USER Pending을 완료로 추정하지 않는다.
```

---

## 11. SCAN-P0-04 Closure — 2026-08-16

`SCAN-P0-04 Fitting Integration`은 P0-01의 `FCFVehicleFittingSnapshot.ResolvedSensorData`를 P0-02의 `UCFVehicleSensorComp::ApplySensorData()`에 실제 연결하고, Field Fitting의 기존 원자 Commit/Checkpoint/Compensation 경계에 Sensor participant를 포함해 **Technical Done**으로 종료한다.

구현 Source:

```text
UE/Source/CarFight_Re/Public/CFVehicleFittingComp.h v1.5.0
UE/Source/CarFight_Re/Private/CFVehicleFittingComp.cpp v1.5.0
UE/Source/CarFight_Re/Private/CFScannerFittingTests.cpp v1.2.0
```

확정 Runtime 흐름:

```text
FCFVehicleFittingSnapshot.ResolvedSensorData
→ FCFFittingSensorRuntimeInput
→ UCFVehicleFittingComp atomic Runtime input
→ ICFFittingRuntimeApplyAdapter::ApplySensorRuntime()
→ UCFVehicleSensorComp::ApplySensorData()
```

초기 출격에서는 Fitting Commit이 Sensor Runtime 초기화보다 먼저 Source만 선택하고, 이후 기존 `InitializeSensorRuntime()`이 선택된 Source로 Runtime을 시작한다. Field Fitting에서는 Runtime Ready 상태에서 `ApplySensorData()` hot reapply만 사용하므로 `InitializeSensorRuntime()`을 재호출하지 않는다.

Field Fitting 원자성:

```text
Weapon → Defense → Sensor Commit
Sensor 실패 → 직전 Weapon·Defense·Sensor Runtime 복원
후속 Inventory Commit 실패 → Captured Runtime Checkpoint로 Sensor 포함 Compensation
scanner-less Snapshot → ResolvedSensorData=null → ApplySensorData(nullptr) Fallback
Legacy VehicleData 경로 → 기존 Sensor Source 보존
```

또한 Scanner Utility mount가 `ResolvedMounts[0]`에 위치할 때 Weapon Runtime 후보로 오인될 수 있던 경계를 교정해, 실제 `WeaponData`가 있는 Resolved Mount만 Weapon 후보로 선택하도록 했다.

검증:

```text
Official UE 5.8 Editor Build
= 20ca14bdc6094650bc11209f4ce320fa PASS / Exit 0

SCAN-P0-04 targeted
= CarFight.Scanner.SCAN_P0_04.FittingIntegration 1/1 PASS
= process a1e696eb8dd64ee7a46b1b3f4c36cb39 PASS

Fitting regression
= CarFight.Fitting 22/22 PASS
= process ff4f50ecf3654400a444ab77fe291946 PASS

Sensor regression
= CarFight.Sensor 14/14 PASS
= process ae7c73907d6c420e9c84a4279f11f90c PASS

Inventory protection regression
= CarFight.Inventory 12/12 PASS
= process 25a211085364499bad50fcdb35ee1b15 PASS
```

첫 Build `5b603482221140e0971e58a97b286590`은 `TObjectPtr<UCFWeaponData>`를 `IsValid(const UObject*)`에 직접 전달한 단일 compile error로 실패했고, `.Get() != nullptr` 교정 후 최종 Build가 PASS했다. 이 실패는 Runtime/설계 실패로 확대하지 않는다.

P0-04에서는 Content Asset, Blueprint, InputAction, Scanner tuning과 USER PIE를 변경하지 않았다. P0-03의 `IA_ActiveScan=V` 단발 입력 계약과 CF-FQ-032/026/015/029/035 USER Pending/Paused 체크포인트를 보존한다.

### SCAN-P0-05 시작점

P0-05는 새 Source 기능을 추가하는 Gate가 아니라 **현재 정확한 Source의 기술 Acceptance**다. P0-04 최종 Build와 신규 FittingIntegration, Fitting/Sensor/Inventory 회귀는 같은 Source에서 이미 PASS했으므로 반복하지 않는다.

남은 필수 기술 검증은 다음과 같다.

```text
1. TargetSelect asset-free 보호 회귀를 현재 Source에서 실행한다.
2. P0-01~04 Scanner 계약과 Sensor/Fitting/TargetSelect 책임 경계를 정적 감사한다.
3. Build 20ca14bdc6094650bc11209f4ce320fa와 기존 P0-04 회귀를 P0-05 Acceptance evidence로 재사용한다.
4. Asset 저장 가능성이 있는 TargetSelect Input/HUD 테스트는 보호 중인 dirty Content 때문에 실행하지 않는다.
5. USER PIE, 실제 V 키 체감, 실제 탐지 거리/피드백은 P0-06에 남긴다.
```

---

## 12. SCAN-P0-05 Closure — 2026-08-16

`SCAN-P0-05 Technical Validation`은 P0-04와 동일한 최종 Source에 대해 신규 Scanner/Fitting 계약, 기존 Sensor/Fitting/Inventory 보호 회귀, TargetSelect 책임 경계와 공식 Build를 종합 감사해 **Technical Done**으로 종료한다.

재사용한 current-source evidence:

```text
Official UE 5.8 Editor Build
= 20ca14bdc6094650bc11209f4ce320fa PASS / Exit 0

Scanner/Fitting contract
= CarFight.Scanner.SCAN_P0_04.FittingIntegration 1/1 PASS

Fitting regression
= CarFight.Fitting 22/22 PASS

Sensor regression
= CarFight.Sensor 14/14 PASS

Inventory protection regression
= CarFight.Inventory 12/12 PASS
```

위 항목은 P0-04에서 같은 Source로 이미 PASS했으므로 P0-05에서 반복 실행하지 않았다.

P0-05 fresh TargetSelect 보호 회귀:

```text
CarFight.TargetSelect.TS_P0_01.RuntimeContract
= 1/1 PASS
= process 19077228b82f4c37a1113c51fe18eaa7
= Engine Exit 0 / Failure 0
```

TargetSelect 전체 prefix는 실행하지 않았다. `TS_P0_05.InputIntegration`과 `TS_P0_06.TargetHud`는 저장 가능한 Input/HUD Asset 테스트 경로를 포함하므로 현재 dirty Content 보호를 위해 제외했다. 이는 TargetSelect USER/Asset Gate를 완료로 추정하는 의미가 아니다.

정적 책임 경계 감사:

```text
ACFVehiclePawn
= IA_ActiveScan Enhanced Input Bind의 유일한 Scanner 입력 caller

UCFVehicleSensorComp
= Start/Stop Active Scan, SensorConfig, Contact lifecycle/Analysis/Knowledge owner
= EnhancedInput / BindAction / UInputAction 참조 0

UCFVehicleFittingComp
= ResolvedSensorData → ApplySensorData adapter와 Commit/Restore owner
= Scanner Input bind 0

TargetSelect Source
= SensorData 참조 0
= ApplySensorData 참조 0
= InputAction_StartActiveScan 참조 0
= StartActiveScan 참조 0
= FCFSensorConfig 참조 0
= ApplySensorRuntime 참조 0
```

판정:

```text
P0-01 Scanner Data Contract: 유지
P0-02 non-destructive Config Apply: 유지
P0-03 Pawn Input ownership: 유지
P0-04 Fitting atomic integration: 유지
Sensor/TargetSelect 책임 분리: 유지
Content/Blueprint/Input asset mutation: 0
USER PIE PASS 추가: 0
Technical Acceptance: PASS
```

### SCAN-P0-06 시작점

P0-06은 기술 Automation이 아니라 사용자 직접 PIE Acceptance다. 다음 항목은 실제 화면과 조작을 사용자가 확인한 경우에만 PASS 처리한다.

```text
1. /Game/Maps/TestMap_ScannerP0의 실제 차량에서 V 1회 입력으로 Active Scan이 시작되는가
2. VehicleSensorComp Details의 "Active Scan 실행 상태 / 남은 시간"이 5초 동안 진행되고 자동으로 False / 0으로 종료되는가
3. 실행 중 V 반복 입력이 남은 시간을 5초로 다시 늘리거나 중복 실행하지 않는가
4. Scanner가 없는 기존 /Game/Maps/TestMap_AmmoRipple에서는 V 입력이 안전하게 거부되고 Active Scan 상태가 False / 0을 유지하는가
5. Scanner fixture 차량에서 센서 런타임 준비 상태와 현재 센서 Snapshot이 유효하며, scanner-less 기준과 달리 Active Scan을 실제 시작할 수 있는가
6. 기존 TargetSelect 입력과 현재 HUD 기본 동작이 Scanner 입력 추가 때문에 깨지지 않는가
```

P0-04에서 이미 자동 검증한 Runtime Ready hot reapply의 ContactId/Knowledge/Analysis 보존과 Field Fitting compensation은 기술 Acceptance로 유지한다. 현재 실제 Field Fitting production caller/UI는 `CF-FQ-034/035` USER Pending 범위이므로, **CF-FQ-037 P0-06 USER PASS를 아직 미구현인 Field UI를 통한 Scanner 교체에 종속시키지 않는다.** 향후 실제 Field UI가 연결되면 해당 기능의 USER Gate에서 Scanner participant까지 함께 재확인한다.

Radar Range/Zoom·동적 Blip·TargetPanel/Radar USER Visual은 CF-FQ-032 범위이므로 P0-06 PASS 조건에 포함하지 않는다.

### SCAN-P0-06A PIE Fixture Readiness — 2026-08-16

사용자 조건상 **2026-08-18까지 직접 PIE 확인이 불가능**하므로 USER PASS는 추가하지 않고, 화요일에 즉시 검증할 수 있는 전용 Scanner fixture만 준비했다.

생성된 test-only Content:

```text
/Game/CarFight/Tests/Scanner/DA_Sensor_ScanP0
/Game/CarFight/Tests/Scanner/Preset_ScannerP0
/Game/CarFight/Tests/Scanner/DA_Vehicle_ScanP0
/Game/CarFight/Tests/Scanner/DA_Fit_ScannerP0
/Game/Maps/TestMap_ScannerP0
```

Fixture 설계:

```text
DA_Sensor_ScanP0
- PassiveDetectionRangeCm = 3000
- ActiveScanRangeCm = 6000
- VisualDetectionRangeCm = 2000
- UpdateIntervalSec = 0.1
- MaxActorScansPerUpdate = 64
- ContactMemoryTimeSec = 10
- DestroyedHoldTimeSec = 1
- ActiveScanDurationSec = 5
- AnalysisGainPerSec = 0.5
- AnalysisDecayPerSec = 0.2
- IdentifiedThreshold = 0.5
- DetailedScanThreshold = 1.0

Preset_ScannerP0
- RequiredMountType = Utility
- RequiredWeaponSize = None
- DefaultTurretMountData = null
- DefaultWeaponData = null
- DefaultSensorData = DA_Sensor_ScanP0

DA_Vehicle_ScanP0
- 기존 valid-mass DA_VehicleDefense_TestSUV 복제
- BaseVehicleMassKg = 1000 / MaximumGrossMassKg = 2500
- 기존 Top_02 hardpoint를 ScannerPrimary Utility MountProfile로 재사용
- Production VehicleData는 변경하지 않음

DA_Fit_ScannerP0
- RoofTurret_MediumOrLarge = explicit disabled
- ScannerPrimary = Preset_ScannerP0 enabled
- MissingMountSelectionPolicy = TreatAsError
- InitialSortieAmmoLoads = empty

TestMap_ScannerP0
- TestMap_AmmoRipple 복제
- 단일 BP_CFVehiclePawn의 VehicleData/FittingData만 Scanner fixture로 교체
- 원본 TestMap_AmmoRipple은 scanner-less 비교 기준으로 보존
```

위 Sensor 수치는 `CFScannerRuntimeTests`에서 이미 사용한 기술 fixture 값을 Content로 승격한 것이며 **게임 밸런스·최종 Scanner 등급 수치가 아니다.** Scanner 장비 질량은 여전히 새로 정의하지 않는다. `CFVehicleMountProfile.MountWeightKg=350`은 기존 구조에 남은 hidden legacy serialized field이며 현재 Scanner EquipmentMass/FittingSnapshot 질량 source로 사용하지 않는다.

생성/검증 evidence:

```text
Fixture commandlet
= process 452d943b3d4043849f1a0b063076f8dc PASS / Exit 0
= 신규 Content 5개 생성 / production asset mutation 0

Correction/validation first attempt
= process 5b3201445c2847ff807211d71950e16f FAIL
= hidden legacy MountWeightKg Python edit 거부에서 중단 / functional fixture failure 아님 / 저장 전 중단

Corrected validation
= process 20ddd7ac28954f858563fd9758b41623 PASS / Exit 0
= SensorConfig valid=True
= FittingSnapshot ValidationState=VALID
= ResolvedSensorData=DA_Sensor_ScanP0
= ResolvedMountCount=2
= TotalVehicleMassKg=1100
= TestMap_ScannerP0 player vehicle binding exact 1

Fresh persisted AssetDump
= dataset adset_v1_56089c60aee0c30aa6267ea67f157bf0.88247d80ba58c230d08df105
= Scanner DataAsset 4/4 PASS
= Preset Utility/None + SensorData-only PASS
= Fitting TreatAsError + Turret disabled + Scanner enabled PASS
```

`TotalVehicleMassKg=1100`은 Scanner 질량 추가가 아니라 복제한 TestSUV의 Base 1000kg + 기존 기본 Defense 100kg 결과다.

P0-06A는 **Readiness만 완료**했다. 실제 V 키 입력, Active Scan 5초 실행/자동 종료, 반복 입력 무연장, scanner-less 거부와 TargetSelect/HUD 비회귀는 2026-08-18 이후 사용자가 직접 확인하기 전까지 Pending이다.

### SCAN-P0-06 USER PIE Acceptance — 2026-08-18

첫 USER PIE에서는 사용자가 `V`를 여러 번 누르고 TargetSelect도 시도했지만 화면상 변화가 없다고 보고해 **USER FAIL**로 판정했다. 이 실패를 PASS로 추정하지 않고 fixture와 실제 player spawn 경로를 다시 감사했다.

RCA 결과:

```text
초기 TestMap_ScannerP0
- Scanner Data/Fitting은 저장된 배치 BP_CFVehiclePawn에 적용돼 있었음
- 배치 Pawn 두 대 모두 AutoPossessPlayer는 Player0이 아니었음
- CFSingleGameMode는 Unreal 기본 흐름으로 별도 DefaultPawn을 spawn/possess함
- 따라서 "맵에 Scanner fixture 차량 exact 1" 검증은 실제 Player0 Scanner binding을 증명하지 못했음
```

첫 AutoPossess 기반 교정 commandlet은 Player0 exact-1 preflight가 성립하지 않아 process `60eed26a657a4290bd33c5db40702663`에서 **fail-closed / mutation 0 / save 0**으로 중단했다. 이 결과를 근거로 배치 Pawn possession을 바꾸는 방향은 폐기했다.

최종 fixture 교정은 production GameMode/Pawn을 바꾸지 않고 test-only player spawn 경로를 만들었다.

```text
/Game/CarFight/Tests/Scanner/BP_ScanPlayerPawn
- BP_CFVehiclePawn 상속
- VehicleData = DA_Vehicle_ScanP0
- VehicleFittingData = DA_Fit_ScannerP0

/Game/CarFight/Tests/Scanner/BP_ScanGameMode
- GameModeBase 상속
- DefaultPawnClass = BP_ScanPlayerPawn
- PlayerControllerClass = CFPlayerController

/Game/Maps/TestMap_ScannerP0
- WorldSettings GameMode Override = BP_ScanGameMode
```

교정 commandlet process `e2025c0155b543e4aa96ad667c2a887d`는 PlayerPawn CDO Scanner readback, GameMode DefaultPawn readback, map GameMode override save/reload를 모두 PASS했다. Production asset/source mutation 0, 기존 배치 차량 mutation 0이다.

USER Acceptance를 위해 실제 화면에 이미 존재하는 VehicleDebugPanel의 Overview Last Transition 본문에 **test-only 임시 관측 행**을 붙여 `Scanner 준비 / Active Scan / 남은 시간`을 직접 보이게 했다. 이 관측 코드는 Sensor 상태를 읽기만 했으며 Acceptance 완료 후 제거했다.

사용자 직접 확인 결과:

```text
PASS — Scanner 준비 상태가 화면에 표시됨
PASS — V 1회 입력으로 Active Scan 시작
PASS — 남은 시간이 약 5초에서 0초까지 감소
PASS — 약 5초 후 자동 종료
PASS — Active Scan 중 V 반복 입력이 남은 시간을 5초로 reset/연장하지 않음
PASS — Target 선택 기능 유지
PASS — 선택 Target의 "???"가 Active Scan 후 해제됨
       → 입력/타이머뿐 아니라 Analysis/Knowledge 승격 경로까지 USER-visible 동작 확인
```

`TestMap_AmmoRipple`에서는 Scanner 관측 행 자체를 Scanner-map-only 임시 UI가 표시하지 않았으므로 **USER가 scanner-less exact `False / 0`을 직접 관측했다고 기록하지 않는다.** scanner-less 안전 거부는 P0-03~05의 Scanner Runtime/Fitting 기술 회귀 evidence로 보호하며 USER 증거를 꾸며내지 않는다.

이 결과로 `SCAN-P0-06 USER PIE Acceptance`는 **USER Done**이다.

### SCAN-P0-07 Current System Integration — 2026-08-18

P0-06 USER PASS 후 임시 Acceptance 관측 코드를 제거하고 production Source를 정리했다.

최종 closure evidence:

```text
Final production Build
= fcf52353d1f5440392d5e1c379f09ee3 PASS / Exit 0
= CFVehiclePawn.cpp compile PASS
= CFVehicleDebugPanelWidget.cpp compile PASS
= UnrealEditor-CarFight_Re.dll link PASS

Current System
= Document/Systems/Targeting/SensorContact.md v1.1.0
= Document/Systems/SystemIndex.md v1.20.0

Current route cleanup
= ActiveWork에서 CF-FQ-037 Paused row 제거 / Done projection
= ProjectState / Roadmap / FeatureQueue Done 동기화
= Plan Index의 Paused Scanner route 제거
= Archive Index에 Historical + Retained Path 등록
```

`CF-FQ-037`은 `SCAN-P0-00~07` 완료로 **Done**이다. Scanner 현재 구현 판단은 `SensorContact.md v1.1.0`과 실제 Source/Asset을 우선한다. 이 문서는 fixture RCA와 완료 당시 Build/Automation/USER Acceptance를 보존하는 Historical evidence다.

완료에서 제외된 항목은 그대로 유지한다.

```text
CF-FQ-032 Radar Range / Zoom
CF-FQ-032 동적 Radar Blip
CF-FQ-032 Radar / TargetPanel USER Visual
Sensor energy / heat
Missile / Utility의 실제 Sensor Contact 소비
AI Sensor 소비
Network / server-authoritative Sensor
Production Scanner 등급별 최종 밸런스
```

위 항목은 CF-FQ-037을 다시 여는 자동 사유가 아니며 필요 시 별도 Feature/lifecycle로 진행한다.

---

## 13. Changelog / Migration


### v0.8.0 - 2026-08-18

- `SCAN-P0-06 USER PIE Acceptance`를 USER Done으로, `SCAN-P0-07 Current System Integration`을 Done으로 닫아 `CF-FQ-037`을 완료했다.
- 초기 USER FAIL에서 저장된 배치 Scanner fixture가 실제 GameMode DefaultPawn/Player0이 아니었던 문제를 RCA하고 test-only `BP_ScanPlayerPawn` + `BP_ScanGameMode` + `TestMap_ScannerP0` override 구조로 교정했다.
- 첫 AutoPossess 교정은 process `60eed26a657a4290bd33c5db40702663`에서 mutation0/save0 fail-closed였고, 최종 fixture 교정 process `e2025c0155b543e4aa96ad667c2a887d`가 CDO/GameMode/map override readback PASS했다.
- USER가 V 단발 5초 Active Scan, 자동 종료, 활성 중 반복 입력 무연장, TargetSelect 유지와 선택 Target의 `???` 해제를 직접 확인했다. scanner-less exact False/0은 USER 관측으로 꾸미지 않고 기존 기술 회귀 evidence로 보호한다.
- Acceptance용 임시 VehicleDebugPanel 관측 코드를 제거한 뒤 final production Build `fcf52353d1f5440392d5e1c379f09ee3` PASS를 확보했다.
- 현재 구현을 `SensorContact.md v1.1.0`과 `SystemIndex.md v1.20.0`으로 승격하고 ProjectSSOT/ActiveWork/Plan Index current route를 정리했다.
- 이 Plan은 물리 이동 없이 `Completed / Historical + Retained Path`로 보존한다. Radar/TargetPanel 시각, Sensor energy/heat, AI/Network와 Missile/Utility 소비는 후속 범위다.

Migration: CF-FQ-037을 현재 작업으로 재개하지 않는다. Scanner의 현재 구현은 `Document/Systems/Targeting/SensorContact.md v1.1.0`을 우선한다. 완료 당시 fixture RCA·Build·USER Acceptance가 필요할 때만 이 Historical Plan을 읽고, 새 범위는 별도 Feature/lifecycle로 정의한다.

### v0.7.1 - 2026-08-16


- ProjectState/Roadmap/Plan Index의 stale `v0.1.0 / P0-00` Current projection을 실제 `v0.7.x / P0-06` 상태로 교정한 뒤 P0-06 전용 PIE fixture readiness를 준비했다.
- `/Game/CarFight/Tests/Scanner`에 SensorData/Preset/VehicleData/FittingData 4종과 `/Game/Maps/TestMap_ScannerP0` 전용 맵을 test-only로 생성했다.
- Sensor fixture 수치는 기존 `CFScannerRuntimeTests`의 3000/6000/2000cm·5s 계약을 재사용했으며 게임 밸런스 확정으로 승격하지 않았다.
- Corrected validation process `20ddd7ac28954f858563fd9758b41623`에서 SensorConfig valid, FittingSnapshot VALID, ResolvedSensorData, ResolvedMount 2개, map vehicle binding exact 1을 확인했다.
- Fresh AssetDump dataset `adset_v1_56089c60aee0c30aa6267ea67f157bf0.88247d80ba58c230d08df105`에서 Scanner DataAsset 4/4 persisted readback을 PASS했다.
- 실제 Field Fitting production caller/UI는 CF-FQ-034/035 USER Pending이므로 P0-04 hot reapply/compensation Technical PASS는 유지하되 CF-FQ-037 P0-06 USER PASS를 아직 미구현 Field UI Scanner 교체에 종속시키지 않도록 Acceptance 범위를 교정했다.
- 사용자 조건상 2026-08-18까지 PIE 검사가 불가능하므로 P0-06 USER PASS와 P0-07 승격은 계속 금지한다.

Migration: 2026-08-18 이후 `/Game/Maps/TestMap_ScannerP0`과 scanner-less `/Game/Maps/TestMap_AmmoRipple`을 비교해 V 단발 입력, 5초 timed scan, 반복 입력 무연장, safe reject와 TargetSelect/HUD 기본 비회귀만 USER 확인한다. P0-04/05 Build·Automation은 Source 변경이 없는 한 반복하지 않는다.

### v0.7.0 - 2026-08-16

- `SCAN-P0-05 Technical Validation`을 Technical Done으로 종료하고 현재 Gate를 `SCAN-P0-06 USER PIE Acceptance`로 이동했다.
- P0-04 final Build `20ca14bdc6094650bc11209f4ce320fa`, FittingIntegration 1/1, Fitting 22/22, Sensor 14/14, Inventory 12/12 PASS를 같은 current Source의 Acceptance evidence로 재사용하고 반복하지 않았다.
- 현재 Source에서 asset-free `CarFight.TargetSelect.TS_P0_01.RuntimeContract`를 fresh 실행해 process `19077228b82f4c37a1113c51fe18eaa7` 1/1 PASS를 확인했다.
- TargetSelect Input/HUD 저장 가능 테스트는 dirty Content 보호 때문에 의도적으로 제외했고 USER/Asset PASS로 확대하지 않았다.
- 정적 감사에서 Scanner Enhanced Input은 Pawn만 소유하고 Sensor/Fitting에는 Input bind가 없으며, TargetSelect Source에는 SensorData·SensorConfig·ActiveScan·ApplySensorRuntime 소유권 침범이 없음을 확인했다.
- P0-06은 실제 `V` 입력, timed Active Scan, 반복 입력, scanner-less 안전 처리, 장비 변경 후 성능 적용과 Contact/Knowledge 보존을 사용자 직접 PIE로 검증한다.

Migration: P0-06부터는 USER PIE 증거가 필요하다. 기술 PASS만으로 USER PASS를 추정하지 않으며 CF-FQ-032 Radar/TargetPanel 시각 Gate는 Scanner Acceptance와 분리한다.

### v0.6.0 - 2026-08-16

- `SCAN-P0-04 Fitting Integration`을 Technical Done으로 종료하고 현재 Gate를 `SCAN-P0-05 Technical Validation`으로 이동했다.
- `FCFFittingSensorRuntimeInput`과 `ICFFittingRuntimeApplyAdapter::ApplySensorRuntime()`을 도입해 `ResolvedSensorData → ApplySensorData()`를 기존 Weapon·Defense Runtime Commit/Checkpoint/Compensation에 통합했다.
- 초기 출격은 Sensor Runtime 초기화 전 Source 선택, Field Fitting은 Runtime Ready non-destructive hot reapply로 분리해 `InitializeSensorRuntime()` 재호출을 금지했다.
- scanner-less null/Fallback, Legacy Source 보존, Sensor Commit 실패 복원과 Inventory 실패 compensation을 기존 원자 경계 안에서 확정했다.
- Scanner-first Snapshot에서 Utility Scanner mount를 Weapon으로 오인하지 않도록 실제 `WeaponData`가 있는 mount만 Weapon Runtime 후보로 선택하도록 교정했다.
- 최종 official Build `20ca14bdc6094650bc11209f4ce320fa` PASS, FittingIntegration 1/1, CarFight.Fitting 22/22, CarFight.Sensor 14/14, CarFight.Inventory 12/12 PASS를 closure evidence로 기록했다.
- P0-05에서는 위 증거를 반복하지 않고 현재 Source의 asset-free TargetSelect 보호 회귀와 정적 책임 경계 감사를 추가한다. USER PIE는 P0-06에 유지한다.

Migration: P0-05는 Source 기능 추가가 아니라 Technical Acceptance다. P0-04 Build/Fitting/Sensor/Inventory 검증을 재실행하지 않고 TargetSelect asset-free regression과 정적 통합 감사만 추가한다.

### v0.5.0 - 2026-08-16

- `SCAN-P0-03 Input Command Integration`을 Technical Done으로 종료했다.
- `ACFVehiclePawn v2.147.0`에서 Enhanced Input owner를 유지하며 `IA_ActiveScan` Started를 Sensor `StartActiveScan()` Gameplay command로 연결했다. Sensor Component 직접 Input bind는 추가하지 않았다.
- P0 실제 입력 의미를 `V 1회 입력 → ActiveScanDurationSec 실행 → 자동 종료`로 확정했고 별도 Stop 키는 만들지 않았다. `RequestStopActiveScan()`은 시스템·장비 전환용 command로 유지한다.
- `/Game/CarFight/Input/IA_ActiveScan`을 Boolean + ConsumeInput + InputTriggerPressed로 생성하고 기존 `IMC_Vehicle_Default`에 `IA_ActiveScan <- V` 매핑을 저장했다.
- Persisted AssetDump에서 IA_ActiveScan Boolean/Pressed와 IMC mapping 47개 중 V 매핑을 재확인했다.
- official Build `b84e8dabcb784b30a005fb120af5cf4d` PASS와 validation process `4d9a1e5ad12f416db18b1a069694c241` PASS를 기록했다. InputAsset 1/1, InputCommand 1/1, Sensor 14/14, TargetSelect RuntimeContract 1/1이 모두 PASS다.
- Blueprint·DataAsset·Scanner tuning 변경은 0이며 USER PIE PASS는 추가하지 않았다. 실제 V 키 조작감은 `SCAN-P0-06`에서 사용자 직접 확인한다.
- 다음 Gate를 `SCAN-P0-04 Fitting Integration`으로 이동하고 실제 `ResolvedSensorData → VehicleSensorComp` 및 Field Fitting Sensor participant 통합을 P0-04에 유지한다.

Migration: P0-04는 P0-01의 ResolvedSensorData를 P0-02 ApplySensorData에 연결한다. P0-03의 IA_ActiveScan/V 단발 입력 계약을 변경하거나 Sensor Component에 직접 입력 책임을 추가하지 않는다.

### v0.4.0 - 2026-08-16

- `SCAN-P0-02 Runtime Config Apply`를 Technical Done으로 종료했다.
- `CFVehicleSensorComp v1.6.0`에 non-destructive `ApplySensorData()`와 Applied Config 사본을 도입해 invalid 원자 거부, scanner-less fallback, Contact/Knowledge/Analysis 보존, range 감소 lifecycle reconcile과 Active Scan remaining 비증가를 확정했다.
- `CFScannerRuntimeTests.cpp v1.0.0`의 `CarFight.Scanner.SCAN_P0_02.ConfigApply` 1/1 PASS, official Build `d670926be7c4498dbd15b12601f8505b` PASS와 `CarFight.Sensor` 14/14 PASS를 closure evidence로 기록했다.
- Sensor 회귀는 `TestExit status 0` 뒤 main Editor가 종료됐지만 task-local outer `Start-Process -Wait`가 process-tree wait에 남는 실행 인프라 현상을 분리했고, read-only finalizer `76b2577325a64cf2a3a0b64069474abe`에서 Editor 0과 14/14 결과를 확정했다.
- Content Asset·Blueprint·InputAction·Scanner tuning·PIE mutation과 USER PASS 추가는 0이다.
- 다음 Gate를 `SCAN-P0-03 Input Command Integration`으로 이동하며 실제 FittingSnapshot/Field Fitting Sensor 통합은 `SCAN-P0-04`에 유지한다.

Migration: P0-03은 `ACFVehiclePawn`의 기존 Enhanced Input owner 구조에서 Sensor Start/Stop command만 연결한다. P0-02의 Applied Config/non-destructive reapply 의미를 입력 계층에서 우회하지 않으며 P0-04 전에는 FittingSnapshot Source를 Vehicle Sensor에 연결하지 않는다.

### v0.3.0 - 2026-08-16

- `SCAN-P0-01 Scanner Data Contract`를 Technical Done으로 종료했다.
- 기존 `UCFVehicleSensorData`를 Utility Scanner payload로 재사용하고 EquipmentPreset/FittingSnapshot의 mount-aware SensorData 계약과 복수 Scanner 명시적 거부를 적용했다.
- 공식 Build `aecd7ab0a1c54bd2b68327b292d117e7` PASS와 asset-free Automation `d014a29d723f4831a6c00e1795a392d6` PASS를 closure evidence로 기록했다.
- Scanner 질량·튜닝·Content Asset·InputAction·PIE를 추가하지 않았고 기존 Inventory 경로와 USER Pending을 보존했다.
- 다음 Gate를 `SCAN-P0-02 Runtime Config Apply`로 이동하고 hot reapply에서 Contact/Knowledge를 지우는 `InitializeSensorRuntime()` 재호출을 금지했다.
- P0-02는 Sensor Component 내부 Config Apply 의미만 소유하고 실제 FittingSnapshot 연결/Field Fitting transaction은 P0-04에 유지한다.

Migration: P0-01의 정적 `ResolvedSensorData` 계약을 이후 Gate의 입력으로 사용한다. P0-02에서는 non-destructive apply/reapply API를 먼저 확립하며 P0-04 전에는 VehiclePawn/FittingComp에 Scanner 장착 Source를 연결하지 않는다.

### v0.2.0 - 2026-08-16

- `SCAN-P0-00 Foundation Audit`을 Source/Asset/Build/PIE mutation 0으로 Technical Done 처리했다.
- Enhanced Input owner를 `ACFVehiclePawn`, Scanner 정적 성능 payload를 기존 `UCFVehicleSensorData`, Detection/Contact/Analysis/Knowledge Runtime owner를 `UCFVehicleSensorComp`로 확정했다.
- `UCFVehicleData`에는 SensorData 참조가 없고 scanner-less 차량은 기존 0-range Fallback을 유지하는 것이 현재 최소 호환 경계임을 확인했다.
- `ECFVehicleMountType::Utility`는 존재하지만 `UCFEquipmentPresetData::HasCompleteEquipmentData()`와 `UCFVehicleFittingData`가 TurretMountData+WeaponData를 강제해 Scanner Utility preset을 현재 계약 그대로는 표현할 수 없음을 확인했다.
- P0-01은 새 Scanner DataAsset 없이 기존 EquipmentPresetData와 FittingSnapshot이 `UCFVehicleSensorData` payload를 운반하도록 최소 확장하는 범위로 고정했다.
- 초기 `InitializeVehicleRuntime()`의 Sensor init hook은 존재하지만 Field Fitting transaction에는 Sensor participant가 없고 `InitializeSensorRuntime()` 재호출은 Contact/Knowledge/ActiveScan을 초기화하므로 hot reapply로 사용하지 않는다고 기록했다.
- 기존 USER Pending/dirty work를 보존하고 다음 Gate를 `SCAN-P0-01 Scanner Data Contract`로 이동했다.

Migration: P0-00은 read-only 감사 완료다. P0-01은 정적 장비/Fitting 데이터 계약만 소유하며 Runtime reapply는 P0-02, InputAction 연결은 P0-03 이후에 진행한다.

### v0.1.0 - 2026-08-16

- 사용자 선택에 따라 `CF-FQ-037 차량 스캐너 입력·장비 통합`의 대표 Plan을 신규 생성했다.
- 완료된 `CF-FQ-036 SensorContact`를 재개하지 않고 Current System으로 소비하는 별도 후속 Feature로 분리했다.
- 첫 Gate를 Source/Asset mutation 없는 `SCAN-P0-00 Foundation Audit`으로 고정했다.
- Scanner 장비 데이터 구조를 선제적으로 만들지 않고 기존 `UCFVehicleSensorData`, VehicleData, Fitting/Inventory 구조의 재사용 가능성을 먼저 감사하도록 했다.
- Sensor는 Gameplay Runtime owner, Pawn/Input Adapter는 입력 owner라는 기존 책임 경계를 유지했다.
- Radar/TargetPanel, 에너지·열, Missile/Utility 소비, AI/Network는 이번 Feature 범위에서 제외했다.
- 기존 USER Pending과 dirty work를 모두 보호했다.

Migration: `CF-FQ-036`의 현재 Sensor 의미는 `Document/Systems/Targeting/SensorContact.md v1.0.0`을 계속 우선한다. 이 Plan은 Scanner 입력·장비 연결의 앞으로 할 작업만 소유한다.
