# Missile Guidance

- Version: 1.0.1
- Date: 2026-09-08
- Status: Current System / CF-FQ-030 P0 Complete / CF-TC-027 PASS / Final Audit Correction PASS
- Feature: `CF-FQ-030 물리 제한형 미사일 비행·유도`
- Validation: Technical PASS + USER Guidance Feel ACCEPTED
- Scope: Direct Release 기반 TargetActor 미사일의 발사 순간 목표 Snapshot, 비행 상태, 물리 제한형 Guidance, Seeker/관측/재포착, Guidance Law, 독립 Guidance Activation과 Pool 재사용 안전성의 현재 구현 기준

---

## 1. 문서 목적

이 문서는 CarFight에서 현재 완료된 **Direct TargetActor 물리 제한형 미사일 비행·유도**의 Current System을 기록한다.

상세 구현 과정, 설계 교정 이력, Build/Automation Job과 USER Feel 반복 시험의 전체 근거는 대표 Historical Plan이 보존한다.

```text
Document/Plan/MissileGuidance/MissileGuidancePlan.md
Document/Plan/MissileGuidance/GuidancePerformanceDesign.md
```

현재 구현 판단에서는 실제 Source와 이 문서를 우선하고, Plan은 완료 당시 evidence와 후속 범위 확인에 사용한다.

---

## 2. 현재 P0 완료 범위

현재 P0에서 완료된 범위는 다음과 같다.

```text
- Direct Release 미사일의 Released → Ejection → Clearance → GuidedFlight 비행 상태
- 발사 순간 GuidanceTargetActor Snapshot
- 발사 후 차량의 현재 선택 Target 변경과 독립된 기존 미사일 Target 유지
- TargetActor Guidance
- 물리 제한형 선회율·횡가속도·응답시간 적용
- PurePursuit / LeadPursuit / ProportionalNavigation Guidance Law
- FollowFlightGuidanceWindow / Independent Guidance Activation
- Independent Delay + Distance AND 조건과 activation latch
- LegacySingleGate / Stateful Seeker
- Acquisition / Tracking / LostGrace / Reacquiring / LostFinal 상태
- DirectActorKinematics / SampledPositionEstimate 관측
- 위치 샘플 기반 Target 속도 추정과 관측 사이 외삽
- ContinueStraight 목표 상실 경로
- HoldLastKnownPoint 상태형 LostFinal 경로
- ForwardCone 재포착 시 최초 Launch Target Snapshot과 같은 Actor만 재포착
- rear/non-closing PN Course Capture
- Stateful approach-armed Overshoot
- exact rear 방향의 결정적 Launch Right tie-break
- Impact/LifeExpired/Pool 재사용 시 Flight·Guidance Runtime Reset
- Low / Normal / High USER Guidance Feel acceptance
```

현재 USER 판정은 Low / Normal / High를 실제 `MissileDirectTest` PIE에서 확인한 **P0 체감 승인**이다. 사용자 표현은 `얼추 PASS`이며, 이는 유도 수치가 영구 동결됐다는 의미가 아니다.

---

## 3. 현재 책임 구조

### 3.1 ProjectileData

`UCFProjectileData`는 미사일의 저장 Runtime 설정 owner다.

```text
MissileFlightConfig
MissileGuideConfig
```

기존 ProjectileData에서 미사일 기능이 비활성이면 일반 Projectile·Rocket의 기존 이동을 변경하지 않는다.

### 3.2 공통 Projectile Actor

`ACFProjectileActor`는 공통 발사체 생명주기에 Missile Flight와 Guidance를 연결한다.

현재 주요 컴포넌트 순서는 다음과 같다.

```text
ProjectileMotorComp ─┐
                     ├→ MissileGuideComp → ProjectileMovementComponent
MissileFlightComp ───┘
```

`MissileGuideComp`는 Motor와 Flight를 모두 Tick prerequisite로 두고 두 컴포넌트 이후에 Guidance를 계산한다. `ProjectileMovementComponent`는 `MissileGuideComp`를 Tick prerequisite로 둬 Guidance 결과 Velocity로 실제 이동한다. 현재 Source는 Motor와 Flight 사이의 상대 실행 순서를 별도 prerequisite로 고정하지 않는다.

공통 Actor는 활성화·충돌·Impact·Lifetime·Pool 반환을 계속 소유한다. Guidance 알고리즘 세부 책임은 `UCFMissileGuideComp`가 소유한다.

### 3.3 Launch Context

`FCFProjectileLaunchContext`는 런처가 발사 순간 확정한 값을 Projectile에 복사한다.

미사일에 중요한 값:

```text
LaunchTransform
InitialLaunchDirection
InitialLaunchVelocity
CommandTargetLocation
GuidanceTargetActor
ReleaseMode
FireRequestId
WeaponGroupId
```

`GuidanceTargetActor`는 발사 순간 Snapshot이다. 발사 후 차량이 다른 Target을 선택해도 이미 발사된 미사일은 자동으로 새 Target으로 바뀌지 않는다.

---

## 4. Missile Flight

현재 P0 Flight owner는 `UCFMissileFlightComp`다.

Direct Release 기준 상태 흐름:

```text
Inactive
→ Released
→ Ejection
→ Clearance
→ GuidedFlight
```

현재 Direct Runtime에서 `UCFMissileFlightComp`가 `Impact` 또는 `Expired` 상태로 직접 전환하지는 않는다. 실제 Hit/LifeExpired 종료는 `ACFProjectileActor`의 Projectile lifecycle이 소유하고, 비활성화 시 `ResetMissileFlight()`가 Flight 상태를 `Inactive`로 되돌린다. `Impact`/`Expired` enum 항목은 존재하지만 현재 Direct 상태 머신의 실행 상태로 확대 해석하지 않는다.

FlightComp는 다음을 측정·보존한다.

```text
ReleaseLocation
ElapsedFlightTimeSeconds
DistanceFromReleaseCm
CurrentFlightState
Guidance Window 상태
```

`FollowFlightGuidanceWindow` Guidance는 이 Flight 상태가 연 Guidance Window를 따른다.

`Independent` Guidance는 FlightComp가 측정값만 제공하고, Guidance 활성 여부와 latch는 GuideComp가 소유한다.

---

## 5. Guidance Activation

`ECFMissileGuidanceActivationMode`의 현재 두 경로는 다음과 같다.

### FollowFlightGuidanceWindow

기존 호환 경로다.

```text
FlightComp Guidance Window 닫힘
→ Guidance 미적용

FlightComp Guidance Window 열림
→ Guidance 허용
```

### Independent

발사 후 시간과 분리 거리를 독립 조건으로 사용한다.

```text
ElapsedFlightTimeSeconds >= GuidanceActivationDelaySeconds
AND
DistanceFromReleaseCm >= GuidanceActivationDistanceCm
→ Guidance Activation
```

두 조건을 모두 만족해야 한다. 한 번 만족한 activation은 이후 U-turn 등으로 분리 거리가 다시 줄어도 닫히지 않는다.

이 구조 때문에 향후 장비에서 사출 직후 일정 시간/거리 동안 초기 사출 탄도를 유지한 뒤 유도를 시작하는 연출도 같은 시스템으로 구성할 수 있다. 장비별 Salvo/사출 패턴 자체는 이 문서의 P0 책임이 아니다.

---

## 6. Guidance Law

### PurePursuit

마지막 실제 관측 위치를 직접 추적한다.

```text
AimPoint = LastObservedTargetLocation
```

목표 속도 선행 예측을 사용하지 않는다.

### LeadPursuit

마지막 실제 관측 위치에 필터된 Target 속도 기반의 제한된 선행량을 더한다.

```text
LeadOffset = FilteredTargetVelocityEstimate * LeadTimeSeconds
LeadOffset 크기 <= MaxLeadDistanceCm
AimPoint = LastObservedTargetLocation + LeadOffset
```

### ProportionalNavigation

상대 운동과 LOS 변화에 기반한 비례항법을 사용한다.

rear/non-closing 기하에서는 즉시 비정상 PN 명령을 만들지 않고 물리 제한형 Course Capture로 접근 기하를 먼저 만든 뒤 PN으로 복귀한다.

세 Law는 모두 같은 물리 제한 계약을 사용한다.

```text
MaximumTurnRateDegPerSec
MaximumLateralAccelerationCmPerSecSq
GuidanceResponseTimeSeconds
MinimumGuidanceSpeedCmPerSec
```

미사일 위치 순간이동, Target 방향으로 Velocity 강제 덮어쓰기, 강제 명중은 사용하지 않는다.

---

## 7. Seeker와 Target 관측

### LegacySingleGate

기존 저장 DataAsset 호환을 위한 단일 각도 판정 경로다.

```text
SeekerFieldOfViewDeg
LockBreakAngleDeg
```

### Stateful

현재 상태형 Seeker는 다음 상태를 사용한다.

```text
Inactive
Acquiring
Tracking
LostGrace
Reacquiring
LostFinal
```

각 geometry는 독립 반각이다.

```text
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
ReacquisitionConeHalfAngleDeg
```

각 값은 0~180도 범위에서 서로를 암묵적으로 축소하지 않는다.

### DirectActorKinematics

Target Actor의 현재 위치와 Velocity를 직접 읽는 기존 호환 관측이다.

### SampledPositionEstimate

Target Actor 위치만 설정된 간격으로 관측하고, 위치 차분으로 Target Velocity를 추정한다.

```text
TargetObservationIntervalSeconds
TargetVelocityEstimateResponseTimeSeconds
LastObservedTargetLocation
EstimatedTargetLocation
FilteredTargetVelocityEstimate
ObservationAgeSeconds
```

Sampled 경로는 매 프레임 `TargetActor->GetVelocity()`를 정답처럼 Guidance 입력에 사용하지 않는다.

---

## 8. 목표 상실과 재포착

현재 P0의 우선 Target은 발사 순간 `GuidanceTargetActor` Snapshot이다.

지원되는 주요 정책:

```text
ContinueStraight
HoldLastKnownPoint
```

`ForwardCone` 재포착은 주변 Actor를 자동 검색하지 않는다. 발사 순간 Snapshot과 같은 Actor만 재포착할 수 있다.

현재 `Expire` enum/config는 존재하지만 **Guidance가 실제 Projectile lifetime 종료를 요청하는 Product lifecycle 연결은 후속 범위**다. Projectile 실제 lifetime은 기존 Projectile timer/lifecycle이 소유한다.

Target Actor가 파괴되거나 무효화된 경우 약한 참조 수명을 연장하지 않는다.

---

## 9. Overshoot와 rear-aspect

Stateful Overshoot는 발사 직후 rear U-turn을 Target 통과로 오인하지 않도록 실제 접근이 한 번 성립한 뒤에만 arm한다.

```text
거리 감소
AND ForwardClosingVelocity > 0
→ Overshoot Armed

Armed 이후 거리 증가
AND ForwardClosingVelocity <= 0
→ Overshoot / LostFinal
```

exact 180도 rear Target처럼 좌우 방향이 수학적으로 모호한 경우 발사 순간 `LaunchRightVector`를 결정적 tie-break로 사용한다.

---

## 10. Pool 재사용 계약

Projectile 비활성화/Pool 반환 시 이전 미사일 Runtime 상태가 다음 발사에 남지 않아야 한다.

Reset 대상에는 다음 계열이 포함된다.

```text
GuidanceTargetActor
Flight State / Flight elapsed time / release distance
Seeker State
LastObserved / Estimated Target state
Filtered Target Velocity estimate
Lost / Reacquisition elapsed time
Hold target
Guidance AimPoint
Course Capture
Guidance Activation latch
Overshoot armed state
Current Guidance Command / Snapshot
```

누적 activation count처럼 진단 목적 누적값은 해당 컴포넌트 계약에 따라 유지할 수 있으나, 다음 활성화의 동작 입력으로 이전 Target/Guidance 상태를 재사용하지 않는다.

---

## 11. Guidance Preset DataAsset

`UCFMissileGuidePresetData`는 `FCFMissileGuideConfig` 한 덩어리를 저장하는 **passive authoring/tuning container**다.

저장 필드:

```text
PresetId
PresetDisplayName
PresetDescription
MissileGuideConfig
```

이 타입은 다음을 하지 않는다.

```text
- 자산 이름으로 Low/Normal/High를 판정하지 않는다.
- PostInitProperties에서 seed하지 않는다.
- 로드 시 사용자 저장 튜닝값을 덮어쓰지 않는다.
- Projectile 속도·피해·Collision·FX를 소유하지 않는다.
```

Low/Normal/High 신규 seed가 필요한 시험 저작은 `CarFight_ReEditor` authoring 경로가 신규 자산 생성 시에만 수행한다.

현재 Low/Normal/High는 Product Runtime의 고정 품질 enum이 아니다. 체감 비교와 후속 장비 저작에 사용할 데이터 기반 Guidance Preset이다.

수치만 조정하는 경우 Guidance Law/schema/Product Runtime 코드가 바뀌지 않으므로 C++ rebuild를 요구하지 않는다.

---

## 12. 현재 검증 상태

`CF-TC-027`은 현재 범위에서 Complete / PASS다.

```text
Technical Validation = PASS
USER Guidance Feel = ACCEPTED
```

최신 완료 evidence:

```text
Official UE 5.8 Editor Build
- Job 823b88eda5fc42b4b83f41135aa2df0f
- PASS / Exit Code 0

Guidance Preset authoring/idempotence
- e2e62abaac2148e8806850a137598d03
- 1/1 PASS

MG-P0-12E focused
- f430b8d395fc4bb18db238e9eff8766e
- 1/1 PASS

CarFight.Missile
- 2f6b1341bc5c42f988fa13a61d5e8997
- 10/10 PASS

Persisted Guidance Preset
- 기존 AssetDump 3/3 PASS evidence 보존

USER Feel
- Low / Normal / High 직접 PIE 확인
- USER 표현: "얼추 PASS"
```

새 failure/change evidence가 없으면 이 완료 evidence를 Systems 문서 갱신만을 이유로 반복하지 않는다.

---

## 13. 현재 비책임 / 후속 범위

다음은 CF-FQ-030 Direct P0 완료를 막지 않는 후속 범위다.

```text
- LaserPoint 실제 Guidance Runtime
- DataLink 실제 Guidance Runtime
- InertialPoint 실제 Guidance Runtime
- Angled / Vertical Attack Profile 완성
- Loft / TopAttack 공격 프로파일
- Guidance LostTargetPolicy=Expire의 실제 Projectile 종료 요청
- 주변 Actor 자동 Retarget
- 무기/장비별 Multi-Muzzle·Salvo·사출 방향 연출 구성
- 네트워크 복제와 서버 권한 미사일 Guidance
```

특히 여러 방향으로 동시 사출된 다수 미사일이 같은 Target으로 수렴하는 장면은 Launcher의 Multi-Muzzle/Salvo, 공통 Volley Target Snapshot, Angled/Vertical Release와 이 Guidance 시스템을 **구조상 조합할 수 있는 후속 장비 저작 범위**다. 다만 Angled/Vertical Release의 실제 통합 USER PIE는 `CF-FQ-029`에 남아 있으므로 이 문장은 해당 사출 모드의 USER acceptance를 뜻하지 않는다. CF-FQ-030 P0 완료 범위에 추가 구현하지 않는다.

---

## 14. 연관 Current Systems

```text
Document/Systems/Combat/Projectile.md
- 공통 Projectile Actor, 이동, 충돌, Impact, Pool과 Missile 컴포넌트 통합 경계

Document/Systems/Combat/WeaponFire.md
- 발사 명령과 Projectile 활성화 진입 경계

Document/Systems/Combat/WeaponData.md
- 무기 정적 Launcher/Projectile 참조 설정

Document/Systems/Vehicles/VehicleAim.md
- 발사 전 Aim/Target 방향 계산 경계
```

Launcher의 Multi-Muzzle/Ripple/Salvo와 Angled/Vertical Release는 CF-FQ-029가 별도 lifecycle로 관리하며, 현재 미완료 USER PIE 범위를 이 문서의 Missile Guidance 완료로 확대하지 않는다.

---

## 15. 회귀 보호 기준

Missile Guidance 관련 Source/Asset 변경 시 최소 보호 기준:

```text
- bUseMissileFlight=false / bUseGuidance=false 기존 Projectile·Rocket 행동 유지
- Direct TargetActor launch snapshot 유지
- 발사 후 차량 Target 변경이 기존 미사일 Target을 바꾸지 않음
- 물리 제한 선회율·횡가속·응답시간 유지
- Guidance Law별 Sensor Truth 의미 유지
- SampledPositionEstimate가 Actor Velocity 정답을 소비하지 않음
- Stateful Seeker 상태/각도 의미 유지
- 재포착은 동일 Launch Target만 허용
- Independent Delay+Distance AND 및 latch 유지
- rear-aspect / Overshoot 결정성 유지
- Impact/LifeExpired/Pool Reset 뒤 이전 Target/Guide state 오염 없음
- Guidance Preset load-time user tuning overwrite 없음
```

---

## 16. 문서 갱신 조건

다음이 바뀌면 이 문서를 갱신한다.

```text
- MissileFlightConfig / MissileGuideConfig schema 또는 의미
- Flight State와 Guidance Activation 책임 경계
- Guidance Law 알고리즘
- 물리 제한 계산 정책
- Seeker/Observation/Reacquisition 상태 의미
- Target Snapshot 또는 Retarget 정책
- Pool Reset exact runtime 계약
- Guidance Preset DataAsset의 Product/Authoring 책임
- Direct P0 후속 범위가 실제 Current Runtime으로 승격될 때
```

---

## 17. Changelog

### v1.0.1 - 2026-09-08

```text
- CF-FQ-030 post-closure Final Audit에서 확인된 Current 문서 사실 오차를 Source 기준으로 교정했다.
- Direct Flight 실제 상태를 Released → Ejection → Clearance → GuidedFlight로 교정하고, Hit/LifeExpired 종료는 Projectile Actor lifecycle이 소유하며 FlightComp가 Impact/Expired 상태로 직접 전환하지 않는 경계를 명시했다.
- Tick ordering을 실제 prerequisite graph로 교정했다. Motor와 Flight는 모두 Guide의 prerequisite이며 둘 사이의 상대 순서는 고정하지 않고, ProjectileMovement는 Guide를 prerequisite로 둔다.
- 향후 다중 방향 Salvo 수렴 연출은 구조상 조합 가능성으로만 기록하고 CF-FQ-029 Angled/Vertical USER PIE Pending을 명시했다.
- Product Source/Asset mutation은 0이며 기존 Build/Automation/AssetDump/USER Feel evidence는 failure/change trigger가 없어 재실행하지 않았다.
```

### v1.0.0 - 2026-09-08

```text
- CF-FQ-030 Direct TargetActor 물리 제한형 Missile Flight/Guidance를 Current System으로 최초 승격했다.
- MG-P0-00~12E Technical 완료와 MG-P0-12 Low/Normal/High USER Guidance Feel ACCEPTED를 현재 계약으로 정리했다.
- Flight/Guide/Projectile/LaunchContext/Guidance Preset 책임 경계를 기록했다.
- LaserPoint, Angled/Vertical/Loft/TopAttack, 실제 Expire와 장비별 Salvo 연출을 비차단 후속 범위로 분리했다.
- 기존 Technical Build/Automation/AssetDump evidence는 새 failure/change evidence 없이 반복하지 않는다.
```

### Migration

```text
- CF-FQ-030의 현재 구현 판단은 이 문서를 우선한다.
- MissileGuidancePlan과 GuidancePerformanceDesign은 완료 당시 상세 설계·검증 evidence owner로 보존한다.
- Projectile.md의 공통 Actor/충돌/Pool 책임은 유지하며 Guidance 세부 계약은 이 문서가 소유한다.
- 후속 Guidance Mode나 Attack Profile이 구현될 때 CF-FQ-030 P0를 자동 재오픈하지 않고 해당 범위의 별도 Feature/작업 lifecycle을 연다.
```
