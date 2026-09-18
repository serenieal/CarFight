# Target Select Current System

- Version: 1.0.0
- Date: 2026-09-14
- Status: Current
- Origin: `CF-FQ-026 타겟 선택 시스템`

---

## 1. 목적

이 문서는 CarFight의 **플레이어 타겟 후보 탐색, 선택 상태, 선택 수명과 외부 소비 계약**을 Current System으로 기록한다.

TargetSelect는 Sensor/Scanner의 탐지 상태나 Aim/Fire의 조준 방향을 대신 소유하지 않는다.

```text
TargetSelect
= 무엇을 선택 후보로 볼 것인가
= 현재 후보가 무엇인가
= 플레이어가 무엇을 선택했는가
= 선택 상태가 언제 유지/해제되는가
= 장비가 공용 선택을 어떤 정책으로 읽을 수 있는가

Sensor / Contact
= 무엇을 탐지했는가
= Contact를 얼마나 기억하는가
= 플레이어가 무엇을 알고 있는가

Aim / Fire
= 실제 조준점·Muzzle·발사 방향
= 선택 Target을 소비할 수 있으나 공용 선택 상태를 소유하지 않음

HUD / UI
= TargetSelect와 Sensor 상태를 읽어 표시
= 선택 상태 Authority 아님
```

---

## 2. Current Source Authority

주요 Source authority는 다음과 같다.

```text
UE/Source/CarFight_Re/Public/CFTargetSelectable.h
UE/Source/CarFight_Re/Public/CFTargetPointComp.h
UE/Source/CarFight_Re/Public/CFTargetSelectTypes.h
UE/Source/CarFight_Re/Public/CFTargetUseTypes.h
UE/Source/CarFight_Re/Public/CFTargetSelectData.h
UE/Source/CarFight_Re/Public/CFTargetSelectComp.h
UE/Source/CarFight_Re/Public/CFTargetRegistrySubsystem.h
UE/Source/CarFight_Re/Private/CFTargetSelectComp.cpp
UE/Source/CarFight_Re/Private/CFTargetCandidateSearch.cpp
UE/Source/CarFight_Re/Private/CFTargetRegistrySubsystem.cpp
UE/Source/CarFight_Re/Public/UI/CFTargetSelectWidget.h
UE/Source/CarFight_Re/Private/UI/CFTargetSelectWidget.cpp
UE/Source/CarFight_Re/Private/UI/CFUISubsystem.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

대표 Historical Plan의 상세 Build/Automation/PIE evidence는 `Document/Plan/TargetSelect/TargetSelectPlan.md v0.13.0`이 보존한다.

---

## 3. Targetable / TargetPoint 계약

차량을 포함한 선택 가능 Actor는 `ICFTargetSelectable` 계약을 통해 표시 정보와 Target 위치를 제공한다.

차량 TargetPoint는 `UCFVehiclePawn`의 `TargetPoint` 기본 서브오브젝트를 사용하며 현재 차량 구현은 `SM_Body` Bounds 중심 자동 정렬을 사용한다. Actor 원점이나 임의 Component Bounds를 플레이어 선택 대표점의 암묵적 Authority로 사용하지 않는다.

TargetPoint의 역할은 후보 방향·거리와 UI Projection의 대표 위치 제공이다. Sensor Contact 위치나 Weapon Muzzle 위치를 대체하지 않는다.

---

## 4. Candidate Search와 Registry

`UCFTargetSelectComp`가 현재 후보 탐색과 결정적 정렬을 소유한다.

현재 후보 공급은 `UCFTargetRegistrySubsystem`을 사용한다.

```text
World BeginPlay
→ 기존 World Actor 1회 TargetSelectable bootstrap
→ 이후 Actor spawn / Level add 증분 등록
→ weak reference Registry 유지
→ Candidate Refresh 시 Registry snapshot 수집
→ Direct trace + Targetable 후보 평가
→ 거리 / 반각 / LOS / 분류·태그 정책
→ 결정적 정렬 + CandidateSwitchAdvantageRatio hysteresis
→ Current Candidate
```

과거 `CF-FQ-026` 시점의 20Hz `TActorIterator<AActor>` 전체 World 반복 순회는 Current 구조가 아니다. 전체 Actor 순회는 Registry bootstrap에서 1회 수행되고, 반복 후보 갱신은 Targetable Actor snapshot을 소비한다.

Direct 조준 Trace가 Registry 등록 타이밍과 경쟁해 실제 조준 Actor를 잃지 않도록 direct-hit Actor는 안전하게 후보 입력에 포함할 수 있다.

Target Registry는 **후보 공급 최적화 계층**이며 선택 상태를 소유하거나 Sensor Contact를 생성하지 않는다.

---

## 5. Selection State Authority

`UCFTargetSelectComp`가 공용 선택 상태의 단일 Authority다.

핵심 상태와 명령:

```text
Current Candidate
Selected Target weak reference
Selection Context / TrackState
Confirm Current Candidate
Clear Selected Target
Selected Target lifetime subscriptions
```

선택 입력은 현재 후보가 유효할 때만 선택 대상을 변경한다. 후보가 없으면 기존 선택을 유지한다.

해제 입력은 Manual 사유로 선택만 해제하며 후보를 지우거나 자동 다음 Target을 선택하지 않는다.

선택 대상의 Destroy / EndPlay / VehicleHealth destroyed 상태는 선택 수명에 반영된다. Sensor가 DestroyedHold Contact를 잠시 유지할 수 있어도 TargetSelect의 실제 Actor 선택은 즉시 해제될 수 있다.

---

## 6. Input 계약

현재 Pawn은 다음 persisted Input Action을 기본 로드하고 Enhanced Input의 Started 이벤트에 연결한다.

```text
/Game/CarFight/Input/IA_SelectTarget
/Game/CarFight/Input/IA_ClearTarget
/Game/CarFight/Input/IMC_Vehicle_Default
```

2026-09-14 fresh AssetDump 기준:

```text
IA_SelectTarget
- Boolean
- Pressed Trigger

IMC_Vehicle_Default
- IA_SelectTarget ← MiddleMouseButton
- IA_SelectTarget ← Gamepad_RightThumbstick
- IA_ClearTarget ← RightMouseButton
- IA_ClearTarget ← Gamepad_FaceButton_Right
```

Input은 `ACFVehiclePawn::SetupPlayerInputComponent()`에서 TargetSelect 명령으로 전달된다. Input Action 자체가 선택 상태를 소유하지 않는다.

---

## 7. Sensor / Scanner 경계

Current `SensorContact.md`의 경계를 유지한다.

```text
TargetSelect Candidate / Selected Actor
≠ Sensor Contact List
≠ Scanner Detection Snapshot
```

Sensor/Scanner는 TargetSelect 후보를 공급하거나 선택을 변경하지 않는다.

Radar/HUD의 `bSelected` 표시는 TargetSelect가 소유한 현재 선택 Actor를 Sensor Contact와 연관해 보여주는 read-only association이다.

Sensor Contact가 Lost/DestroyedHold 상태를 유지하는 동안에도 실제 TargetSelect 선택 상태와 Actor lifetime은 독립적으로 변할 수 있다.

---

## 8. AimReticle / VehicleAim / Fire 경계

TargetSelect는 공용 선택 Target을 제공하지만 현재 직접 조준의 AimOrigin, AimDirection, Muzzle direction과 Fire validation을 자동으로 덮어쓰지 않는다.

Vehicle Aim/Camera/AimReticle은 조준·예측·발사 피드백을 소유한다.

Weapon/Equipment는 `FCFTargetUseRequest`를 통해 현재 선택 대상을 **읽기 전용 평가**할 수 있다. 호환 대상, 관계/태그, TrackState, 장비 준비, 거리 조건은 선택 존재·유효성과 분리해서 평가한다.

현재 Fire/Launcher 경로는 발사 순간 Selected Target Actor를 snapshot으로 받아 유도 Projectile/후속 Launcher Command에 전달할 수 있다. 이 소비가 공용 TargetSelect 상태의 ownership을 Fire/Launcher로 옮기지는 않는다.

---

## 9. UI 표시 경계

Current UI lifetime은 `UCFUISubsystem`이 소유한다.

`WBP_TargetSelect`는 현재 Game Layer의 Target World Marker 역할이며 Possess 변경 시 현재 Pawn의 TargetSelect에 rebind된다.

2026-09-14 fresh AssetDump에서 persisted `WBP_TargetSelect` WidgetBlueprint와 TargetSelect marker widget tree가 존재함을 확인했다. 저장 자산에는 Historical TextBlock 구조가 남아 있지만 Current C++ `UCFTargetSelectWidget`은 후보/선택 semantic text를 runtime에서 비우고 숨긴다.

현재 의미 텍스트와 상태 정보는 Production TargetPanel/HUD 경로가 담당하고 TargetSelect World Marker는 화면 위치·후보/선택 marker와 화면 밖 selected edge marker를 담당한다.

따라서 과거 `TS-P0-08`의 "후보 텍스트가 차량과 겹침" 관찰은 Current marker architecture에 그대로 적용되는 결함으로 보지 않는다. 그렇다고 과거 USER FAIL을 사후 PASS로 변경하지도 않는다.

---

## 10. Current 기본 튜닝 의미

Historical evidence 기준 표준 fallback 설정은 다음과 같다.

```text
DirectSelectMaxDistance = 2000m
ProximitySelectMaxDistance = 1200m
ProximityHalfAngle = 7deg
CandidateRefreshInterval = 0.05s / 20Hz
OcclusionGrace = 1.5s
CandidateSwitchAdvantageRatio = 0.15
AutoDebug = true
DebugSphereRadius = 20cm
```

이 값이 **기술적으로 연결돼 있다**는 사실과 **사용자에게 좋은 조작감이다**라는 평가는 분리한다.

특히 7° / 1200m 후보 범위, Candidate hysteresis와 debug Sphere 가시성은 필요 시 USER 튜닝/진단 범위로 다시 평가한다. 현재 문서는 이를 USER PASS로 주장하지 않는다.

---

## 11. Historical TS-P0-08 Rebaseline

2026-09-14 Rebaseline 판정:

```text
TS-P0-00~07
= Historical Technical PASS 보존

TS-P0-08 historical remote technical evidence
= 기존 Build / 3/3 / safe regressions 보존

20Hz whole-world TActorIterator structural debt
= Successor implementation으로 해결
= Current TargetRegistry snapshot 사용

동일 차량 TargetPoint 위치/인식 영역 USER 재검증
= 코드·Automation 교정은 Historical Technical evidence로 보존
= USER Inconclusive / 새 USER PASS 없음

후보 범위 7° / 1200m 체감
= Deferred USER tuning debt

Debug Sphere 실제 가시성
= Deferred developer observational debt

후보 텍스트 차량 겹침
= successor UI architecture가 역할을 대체
= Historical USER issue를 PASS로 재작성하지 않음

16:9 / 32:9 실제 UI·입력 감각
= old TS-P0-08 USER workflow는 Superseded / Not Executed
= Current UISubsystem + marker/Production TargetPanel workflow가 필요 시 USER 검증 owner
```

따라서 `CF-FQ-026`의 old `TS-P0-08 USER PIE`를 그대로 다시 실행하는 것은 현재 UI/후보 공급 구조를 검증하는 최적 경로가 아니다.

향후 실제 TargetSelect UX 튜닝이 필요할 때는 Historical Feature를 재개하지 않고 Current System 기준으로 정확한 관찰 항목을 새 lifecycle에서 정의한다.

---

## 12. Deferred / 비책임

다음 항목은 Current TargetSelect의 완료를 막는 residual implementation으로 취급하지 않는다.

```text
- 7° / 1200m가 실제 조작감에 적절한지 USER tuning
- Target debug Sphere의 개발자 체감 가시성
- 새로운 Production HUD visual polish
- Sensor contact quality / scan profile tuning
- 관계·Faction gameplay 확장
- 다중 선택 / 부위 선택 / lock-on gameplay 확장
- 대표 대규모 맵에서 Registry 후보 규모 성능 profiling
```

대표 workload가 정해진 뒤 성능 예산이 필요하면 Current Registry 경로를 기준으로 별도 profiling한다. 과거 작은 Transient World 샘플을 Current 성능 PASS로 확대하지 않는다.

---

## 13. Changelog

### v1.0.0 - 2026-09-14

- `CF-FQ-026 Target Select` Rebaseline closure에서 실제 Source의 Targetable/TargetPoint, Registry candidate supply, TargetSelect selection authority, Input, equipment read-only query와 UI marker 경계를 Current System으로 승격했다.
- 과거 20Hz 전체 World `TActorIterator` 후보 수집은 `UCFTargetRegistrySubsystem`의 1회 bootstrap + 증분 등록 + snapshot 조회로 successor-resolved 되었음을 기록했다.
- Sensor/Scanner는 Contact/Detection owner이며 TargetSelect 선택 상태를 소유하거나 변경하지 않는 경계를 고정했다.
- `WBP_TargetSelect` persisted asset과 입력 자산을 fresh AssetDump로 확인했고, current C++ marker-only/Production TargetPanel 분리를 반영했다.
- old `TS-P0-08 USER PIE`는 USER PASS가 아닌 `Superseded / Not Executed`이며 동일 차량 체감·7°/1200m tuning·debug sphere visibility 등은 USER Inconclusive/Deferred observational debt로 분리했다.
- Source/Asset mutation 0, Build/Automation/PIE 재실행 0이다.

Migration: Target selection의 현재 판단은 이 문서와 실제 Source를 우선한다. Sensor/Scanner 완료를 TargetSelect 선택 ownership 대체로 해석하지 말고, Historical `TargetSelectPlan`의 old `TS-P0-08 USER PIE`를 current next gate로 자동 재개하지 않는다.
