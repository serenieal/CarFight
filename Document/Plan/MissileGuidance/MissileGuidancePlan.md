# Physics-Limited Missile Plan

- Version: 0.6.25
- Date: 2026-09-08
- Status: CF-FQ-030 P0 Complete / Post-Closure Final Audit Correction + Re-review PASS / CF-TC-027 Complete PASS / Low·Normal·High USER Guidance Feel ACCEPTED / Current System `Document/Systems/Combat/MissileGuidance.md v1.0.1` + `Projectile.md v1.9.0` integration boundary / Historical + Retained Path / Technical evidence preserved without replay / GuidancePerformanceDesign v0.1.25
- Feature ID: `CF-FQ-030`
- Planned Test: `CF-TC-027`

---

## 1. 목적

런처에서 분리된 뒤 독립적으로 이동하고, 실제 기동 한계 안에서 목표 추적을 시도하지만 명중을 보장하지 않는 미사일 비행·유도 시스템을 구현한다.

추가로 미사일을 하나의 정답 튜닝으로 고정하지 않고, `UCFProjectileData`마다 관측 성능·Seeker 성능·유도 반응·기동 한계를 다르게 설정해 **성능이 낮은 미사일부터 높은 미사일까지 같은 Runtime 구조에서 데이터로 구성**할 수 있게 한다.

### 1.1 2026-08-02 Direct Runtime 구현 게이트 변경

```text
기존:
LM-P0-06 전체 Launcher PIE 완료
→ MG-P0-01 착수

현재:
Direct Launch Handoff 최소 계약 확인
→ MG-P0-01~04 Direct Release·단일 Muzzle Runtime 구현 가능

후속 통합:
Angled·Vertical Ejection / Carrier Velocity / Ripple·Salvo Missile
→ LM-P0-06 잔여 PIE와 함께 검증
```

이 변경은 Launcher 전체 완료 판정을 앞당기지 않는다.
미사일 첫 Runtime은 `ReleaseMode=Direct`, 단일 Muzzle, `GuideMode=TargetActor`, 발사 순간 Target Snapshot과 `ContinueStraight` 목표 상실 정책으로 제한한다.

### 1.2 Direct Runtime 검증 패키지

다음 전용 검증 자산과 맵은 기존 Rocket·Launcher 원본을 수정하지 않고 복제·격리한다.

```text
/Game/CarFight/Tests/Missile/DA_Missile_DirectTest
/Game/CarFight/Tests/Missile/DA_Missile_DirectWeapon
/Game/CarFight/Tests/Missile/EQ_Missile_DirectTest
/Game/CarFight/Tests/Missile/DA_Missile_TestSUV
/Game/Maps/MissileDirectTest
```

전용 맵 배치:

```text
MissileTarget_Static
MissileTarget_Lateral
MissileTarget_Change
MissileTarget_Destroy
MissileTarget_Overshoot
```

검증 행렬:

```text
1. 정지 표적: 정면 목표에 불필요한 횡기동이 없고 충돌·피해까지 정상 처리
2. 측면 이동 표적: Target Velocity를 읽고 제한된 선회로 추적
3. 목표 변경: 발사 뒤 차량이 다른 타겟을 선택해도 기존 미사일 Target Snapshot 유지
4. 목표 파괴: 유예 시간 뒤 TargetLost를 기록하고 ContinueStraight
5. 오버슈트: Projectile 비차단 타겟을 지나 거리 증가·반대 진행이 되면 Overshoot 기록
6. Pool 재사용: 같은 Projectile Actor 재활성화 시 Flight·Guide·Target 상태가 새 발사 기준으로 초기화
```

검증 도구:

```text
Tools/RunMissileTest.ps1 -Apply
Tools/RunMissileTests.ps1
CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
```

Angled·Vertical·Ripple·Salvo·Laser·Loft·TopAttack은 이 검증 패키지에서 활성화하지 않는다.

### 1.3 2026-09-07 USER Feel 결과와 설계 Rebaseline

USER PIE에서 다음이 확인됐다.

```text
- 측면 이동 Target을 실제로 추적함
- 실제 명중까지 확인됨
- 그러나 Target motion에 대한 반응이 지나치게 예측적이고 완벽하게 보여 USER Feel은 승인하지 않음
- 정지 Target에서 빗나가는 사례를 다시 관찰한 결과, Target이 현재 Seeker 허용각에서 벗어나면 추적이 끊기는 현재 조건과 연관된 정상 Miss 가능성을 확인
```

따라서 현재 단일 USER Gate를 단순 수치 조정으로 닫지 않는다.

기존 목표:

```text
하나의 DirectTest Missile이 물리적으로 자연스럽게 선회하는가
```

1차 교정 목표:

```text
같은 Runtime 구조에서
낮은 성능 / 기준 성능 / 높은 성능 Missile을 Data로 구성할 수 있고,
각 Missile의 관측·Seeker·기동 한계 차이가 실제 Hit/Miss와 추적 체감 차이로 나타나는가
```

### 1.4 2026-09-07 Low USER Feel 2차 Rebaseline — 비싼 것은 복잡하고 싼 것은 단순하게

MG-P0-12A Low transient Variant를 실제 PIE에서 발사한 USER 판정으로 1차 성능축 설계만으로는 충분하지 않음이 확인됐다.

```text
현재 Low / Normal / High
→ 모두 같은 PN 계열의 Target 진행 예측 / 교차 유도
→ 관측주기, 추정응답, 선회율, 횡가속, Seeker 각도만 다름
→ Low도 근본적으로 Target의 가는 길 앞을 잘라 들어가려는 성향 유지
```

USER가 원하는 차이:

```text
저성능 / 구형
→ 매 Guidance update에서 그 순간 관측한 Target 위치를 단순 추적
→ Target의 진행 방향 앞을 적극적으로 예측하지 않음
→ 뒤를 쫓는 궤적과 늦은 반응 자체가 성격이 됨

중간급
→ 제한된 선행점만 사용

고성능
→ Target velocity / 상대운동 / LOS 변화 / closing speed를 이용한 복잡한 교차 유도 허용
```

이를 위해 품질 Enum이 아니라 실제 알고리즘 의미를 가진 데이터 축을 추가한다.

```text
ECFMissileGuidanceLaw
- ProportionalNavigation   // 기존 호환 기본값 / 고급 교차 유도
- PurePursuit              // 현재 관측 위치 단순 추적
- LeadPursuit              // 제한된 선행점 추적
```

PurePursuit 강제 계약:

```text
- TargetVelocityEstimate를 선행 예측에 사용하지 않음
- PN LOS angular velocity / closing speed를 사용하지 않음
- SampledPositionEstimate에서도 predictive EstimatedTargetLocation을 기본 추적점으로 사용하지 않음
- 기본 추적점은 LastObservedTargetLocation
- 관측이 늦으면 그 늦은 위치를 그대로 쫓음
```

새 성능 구조:

```text
Observation Model
× Seeker Model
× Guidance Law
× Guidance Activation
× Physical Performance
```

### 1.5 발사 직후 추적 / 뒤쪽 Target U-turn 요구

USER 추가 요구:

```text
- 발사 후 언제부터 추적을 시작할지 ProjectileData별로 조절 가능
- 어떤 미사일은 거의 발사 직후부터 추적 시작
- Acquisition / Tracking / Reacquisition 각도를 자유롭게 설정
- Target이 발사 방향 뒤쪽에 있어도 넓은 Seeker라면 획득 가능
- 충분한 물리 기동성이 있으면 실제 제한 안에서 크게 꺾여 뒤쪽 Target을 따라감
```

Current Source audit:

```text
MinimumClearanceTimeSeconds = 0.15s
MinimumClearanceDistanceCm = 300cm
IsClearanceSatisfied = Time AND Distance
Direct: Clearance 완료 → GuidedFlight → Guidance Window Open
```

따라서 현재 PIE에서 보이는 "발사 후 약간 직진하고 나서 꺾임"은 Guidance 시작이 Flight Clearance Gate에 묶여 있기 때문이다.

당시 후속 설계에서 아래 소유권 분리를 확정했고, 현재는 MG-P0-12D에서 해당 Guidance Activation 소유권 분리를 구현·검증 완료했다.

```text
Flight Clearance
= 런처 분리 / 점화 / 안전거리 / 비행상태 전환

Guidance Activation
= 발사 후 Seeker Acquiring과 Guidance Command를 언제 허용할지
```

계획 필드:

```text
ECFMissileGuidanceActivationMode
- FollowFlightGuidanceWindow   // 기존 호환 기본값
- Independent

GuidanceActivationDelaySeconds
GuidanceActivationDistanceCm
```

`FollowFlightGuidanceWindow`는 현재 `UCFMissileFlightComp::IsGuidanceWindowOpen()` 결과를 그대로 따르는 호환 경로다. 현재 Direct에서는 Clearance 완료가 GuidedFlight 진입으로 이어져 결과적으로 Window가 열리지만, 미래 Transition/Loft/PitchOver에서도 Clearance와 Guidance Window가 항상 동일하다고 가정하지 않는다.

`Independent`의 exact 조건은 다음과 같다.

```text
DelaySatisfied
= Delay <= 0
  OR FlightSnapshot.ElapsedFlightTimeSeconds >= Delay

DistanceSatisfied
= Distance <= 0
  OR FlightSnapshot.DistanceFromReleaseCm >= Distance

ActivationSatisfied
= DelaySatisfied AND DistanceSatisfied
```

`Independent + Delay=0 + Distance=0`이면 첫 유효 Guidance tick부터 추적을 시작할 수 있다. 한 번 조건이 만족되면 `bGuidanceActivationSatisfied=true`로 해당 activation 동안 latch하고, U-turn으로 Release 지점에 다시 가까워져도 Guidance를 다시 닫지 않는다. latch는 `ResetMissileGuidance()`에서만 초기화한다.

시간·거리 측정값은 `UCFMissileFlightComp`, activation 판정과 latch는 `UCFMissileGuideComp`가 소유한다.

Stateful Seeker의 다음 반각은 각각 독립 0~180deg 데이터로 취급한다.

```text
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
ReacquisitionConeHalfAngleDeg
```

당시 Source의 Acquisition/Reacquisition <= Tracking `Min()` coupling은 MG-P0-12D에서 제거 완료했다. 현재 Stateful Acquisition / Tracking / Reacquisition 반각은 각각 독립적인 0~180deg 값으로 동작한다.

예:

```text
GuidanceActivationMode = Independent
Delay = 0
Distance = 0
Acquisition = 180
Tracking = 180
MaximumTurnRate = 매우 높음
MaximumLateralAcceleration = 매우 높음
```

이면 발사 직후 정반대 Target도 각도상 받아들일 수 있고, 실제 U-turn 속도와 곡률은 물리 한계가 결정한다.

다만 현재 PN은 rear/non-closing geometry에서 ClosingSpeed가 0이 되어 횡가속 명령이 0으로 굳을 수 있으므로 `ProportionalNavigation`은 먼저 물리 제한형 Course Capture를 허용한다.

```text
rear / non-closing Target
→ current Sensor Truth 방향으로 bounded pursuit steering
→ ForwardClosingVelocity > 0 접근 기하 형성
→ ProportionalNavigation 전환
```

정확히 180deg 정반대에서는 좌/우 선회 방향이 수학적으로 모호할 수 있으므로 `FCFProjectileLaunchContext.LaunchTransform`의 Right Vector를 deterministic tie-break로 사용한다. random 선회 방향은 사용하지 않는다. 이 tie-break는 회전 방향만 정하며 Turn/Lateral 물리 한계를 바꾸지 않는다.

또한 Stateful Overshoot는 발사 직후 rear U-turn을 오인하지 않도록 실제 접근이 한 번 성립한 뒤에만 arm한다.

```text
초기 bOvershootArmed = false
거리 감소 AND ForwardClosingVelocity > 0
→ bOvershootArmed = true

bOvershootArmed
AND 거리 증가
AND ForwardClosingVelocity <= 0
→ Overshoot / LostFinal
```

LegacySingleGate의 기존 Overshoot 의미는 호환 경로로 보존한다.
순간이동, Velocity 강제 덮어쓰기, 강제 명중은 계속 금지한다.

정식 상세 설계 Owner:

```text
Document/Plan/MissileGuidance/GuidancePerformanceDesign.md v0.1.25
- MG-P0-00~12A Technical baseline 보존
- MG-P0-12C Guidance Law Runtime = Technical PASS
- MG-P0-12D Guidance Activation + Rear Aspect Runtime = Final Technical PASS after P1 Correction
- MG-P0-12E Guidance Preset DA Rewire = Post-Implementation Mid-review Correction + Re-review PASS
- UCFMissileGuidePresetData = passive data container / Product Low·Normal·High authoring branch 0
- Low/Normal/High 신규 seed owner = CarFight_ReEditor authoring only
- Existing Preset = no-seed / no-save / user tuning preservation + persisted idempotence PASS
- MG-P0-12 Low First USER Feel = Historical REJECTED / corrected Low 재검증 USER ACCEPTED
- Low/Normal/High USER Feel = USER ACCEPTED / 사용자 표현 "얼추 PASS"
- Missile Automation 10/10 PASS
- CF-TC-027 USER Feel = ACCEPTED
- Persisted Guidance Preset root = /Game/Test/CarFight/Missile/FeelPresets
- Low Preset = PurePursuit + Independent 0.25s AND 500cm + Observation 0.08s + GuidanceResponse 0.18s + Turn 35deg/s + Lateral 2000 + Acquisition 45deg / Tracking 60deg + Reacquisition None
- Normal Preset = LeadPursuit + short Independent Activation + free Seeker geometry
- High Preset = ProportionalNavigation + Independent 0/0 + 180deg Seeker + high maneuverability
- DirectTest baseline persisted mutation = 0 / Feel Preset DataAsset 3개 persisted authoring 완료
- Numeric feel tuning owner = Guidance Preset DA; 수치 조정만으로는 C++ rebuild 불필요
- Next Exact Gate = None — CF-FQ-030 Direct P0 Complete; 후속 범위는 별도 Feature/작업으로 명시 착수
```

기존 MG-P0-00~12A Technical PASS evidence는 유지한다.
새 설계 교정은 MG-P0-12B 이후 단계로 추가한다.

### 1.6 2026-09-08 Low USER Feel Reject — 초기 조준 탄도 존중

MG-P0-12E에서 재배선한 첫 Low fixture를 USER가 실제 `MissileDirectTest` PIE에서 발사한 결과 다음 문제가 확인됐다.

```text
USER 발사 조건
- 이동 Target의 앞을 의도적으로 선행 조준
- 초기 발사선 자체는 명중 가능성이 높아 보이는 조건

관찰 결과
- Low Missile이 초기 발사선을 유지하지 않고 Target의 현재/최근 위치 쪽으로 이르게 꺾음
- Target 근처에서 다시 크게 수정하며 오히려 Target을 피하는 것처럼 보이는 Miss 발생
- 저가형의 낮은 명중률이라기보다 잘 만든 초기 조준을 Guidance가 스스로 망치는 체감

USER 판정
- REJECT
```

이 결과만으로 `PurePursuit` 알고리즘 자체를 실패로 판정하지 않는다. 첫 Low fixture는 `PurePursuit`에 빠른 Guidance 개입, 느린 관측/응답, 낮은 기동성, 좁은 Seeker를 동시에 조합해 **초기 수동 선행 조준을 존중하지 못하고 뒤늦은 반복 수정이 과도하게 보일 수 있는 시험 설정**이었다.

Low v1.2.1 교정은 Product Guidance Law를 변경하지 않고 Editor-only transient fixture만 다음처럼 조정한다.

```text
GuidanceLaw = PurePursuit
GuidanceActivationMode = Independent
GuidanceActivationDelaySeconds = 0.25s
GuidanceActivationDistanceCm = 500cm
LeadTimeSeconds = 0
MaxLeadDistanceCm = 0
TargetObservationIntervalSeconds = 0.08s
TargetVelocityEstimateResponseTimeSeconds = 0.25s
GuidanceResponseTimeSeconds = 0.18s
MaximumTurnRateDegPerSec = 35
MaximumLateralAccelerationCmPerSecSq = 2000
AcquisitionConeHalfAngleDeg = 45
TrackingConeHalfAngleDeg = 60
TargetLostGraceTimeSeconds = 0.20s
ReacquisitionMode = None
```

교정 의도:

```text
발사 직후 약 0.25s / 500cm 동안
→ USER가 만든 초기 탄도를 우선 보존

Guidance 활성화 뒤
→ LastObservedTargetLocation만 PurePursuit로 단순 추적
→ Target Velocity 기반 선행 예측 없음
→ PN/Lead 계산 없음

관측/응답/기동/Seeker
→ 너무 둔해서 Target 근처에서 뒤늦게 크게 흔들리는 체감은 완화
→ 단, Reacquisition=None과 PurePursuit를 유지해 저성능 특성 보존
```

Low USER acceptance를 다음처럼 명시한다.

```text
PASS 기대:
- USER가 이동 Target 앞을 적절히 조준해 이미 좋은 충돌 가능 탄도를 만든 경우, Low Guidance가 발사 직후 그 탄도를 즉시 Target 현재 위치 쪽으로 강제로 무너뜨리지 않는다.
- Guidance 활성화 후에는 복잡한 선행예측 없이 현재 사용할 수 있는 마지막 관측 위치를 단순하게 쫓는다.
- 쉬운 기하 또는 좋은 수동 선행 조준에서는 실용적으로 맞을 수 있어야 한다.
- Target이 적극적으로 회피하거나 PurePursuit가 구조적으로 불리한 기하에서는 자연스럽게 놓칠 수 있다.

FAIL:
- 잘 조준된 초기 탄도를 Guidance가 즉시 버려 명중 가능성을 현저히 낮춘다.
- Target 근처에서 지연된 큰 재수정 때문에 반복적으로 Target을 피하는 것처럼 보인다.
- Low라는 이유로 정상적인 쉬운 교전에서도 사실상 맞지 않는 미사일이 된다.
```

기술 검증은 v1.2.1 exact fixture를 대상으로 Official Build와 focused/full Missile regression으로 닫되, 위 체감 조건의 최종 판정은 fresh Editor lifetime에서 USER가 다시 수행한다.

---

## 2. 강제 설계 원칙

```text
1. 발사 순간 런처와 물리적으로 독립한다.
2. 차량의 현재 Target 변경이 기존 미사일에 영향을 주지 않는다.
3. 유도는 명중 보장이 아니다.
4. Actor 위치를 목표 쪽으로 직접 이동하지 않는다.
5. Velocity를 목표 방향으로 즉시 교체하지 않는다.
6. 최대 선회율과 최대 횡가속도를 초과하지 않는다.
7. 실제 충돌에서만 명중과 Damage를 처리한다.
8. 목표 상실, 오버슈트와 수명 종료가 정상 결과다.
9. 명중률 자체를 HitChance 같은 확률값으로 직접 지정하지 않는다.
10. 미사일 성능 차이는 관측·Seeker·Guidance Law·Guidance Activation·기동·추진 데이터의 차이에서 발생시킨다.
11. Low/Normal/High 품질 Enum으로 Runtime 행동을 하드코딩하지 않는다. 대신 PurePursuit / LeadPursuit / ProportionalNavigation처럼 실제 알고리즘 의미를 가진 Guidance Law를 Data로 선택한다.
12. 발사 전 Equipment Lock-On과 발사 후 Missile Seeker Acquisition은 서로 다른 소유권과 상태다.
13. Flight Clearance와 발사 후 Missile Guidance Activation은 독립 개념이다. 기존 호환은 FollowFlightGuidanceWindow로 현재 Flight Guidance Window를 따르고, 신규 Missile은 Independent delay/distance 조건을 사용할 수 있다.
14. Independent activation은 조건 만족 후 해당 activation 동안 latch되며 Release 지점에 다시 가까워져도 다시 닫히지 않는다.
15. Reacquisition이 필요해도 기존 Launch Target Snapshot과 같은 Actor만 다시 받아들이며 다른 Actor로 자동 Retarget하지 않는다.
16. 목표가 뒤쪽이라는 이유만으로 모든 미사일에 자동 U-turn을 강제하지 않는다. 단, 넓은 Seeker 각도·빠른 Guidance Activation·충분한 물리 기동성을 Data가 명시한 미사일은 뒤쪽 Launch Snapshot Target을 획득하고 실제 제한 안에서 Course Capture/U-turn 추적할 수 있다.
17. Stateful Overshoot는 실제 접근이 한 번 성립한 뒤에만 arm한다. rear-aspect 초기 U-turn을 Overshoot로 종결하지 않는다. LegacySingleGate의 기존 Overshoot 의미는 보존한다.
18. 정확히 180deg rear target의 선회 방향 특이점은 LaunchTransform Right Vector 기반 deterministic tie-break를 사용하며 random 선택하지 않는다.
19. Terminal 상태를 명중 보장용 보정 단계로 사용하지 않는다.
```

---

## 3. 데이터 구조

별도 Missile DataAsset보다 기존 `UCFProjectileData`의 중첩 Config를 우선한다.

현재 Foundation 타입:

```text
CFMissileFlightTypes.h
CFMissileGuideTypes.h
CFMissileGuideMath.h / .cpp

FCFMissileFlightConfig
FCFMissileGuideConfig
FCFMissileFlightSnapshot
FCFMissileGuideSnapshot
FCFMissileGuidanceInput
FCFMissileGuidanceCommand
ECFMissileFlightState
ECFMissileGuideMode
ECFMissileLostTargetPolicy
ECFMissileAttackProfile
ECFMissileMissReason
```

기본값:

```text
bUseMissileFlight = false
bUseGuidance = false
```

기존 포탄과 Rocket은 변하지 않는다.

MG-P0-08 이후 계획/구현 타입·필드와 MG-P0-12B Rebaseline 추가 타입:

```text
ECFMissileSeekerModel
- LegacySingleGate
- Stateful

ECFMissileTargetObservationMode
- DirectActorKinematics
- SampledPositionEstimate

ECFMissileGuidanceLaw                  // MG-P0-12B 신규 계획
- ProportionalNavigation               // 기존 동작 호환 기본값
- PurePursuit
- LeadPursuit

ECFMissileGuidanceActivationMode       // MG-P0-12B 설계 확정
- FollowFlightGuidanceWindow            // 기존 동작 호환 기본값
- Independent

ECFMissileSeekerState
- Inactive
- Acquiring
- Tracking
- LostGrace
- Reacquiring
- LostFinal

ECFMissileReacquisitionMode
- None
- ForwardCone
- 후속 범위는 설계 검수 후 확장

TargetObservationIntervalSeconds
TargetVelocityEstimateResponseTimeSeconds
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
ReacquisitionConeHalfAngleDeg
ReacquisitionTimeSeconds
GuidanceActivationDelaySeconds         // MG-P0-12B 신규 계획
GuidanceActivationDistanceCm           // MG-P0-12B 신규 계획
LeadTimeSeconds                         // LeadPursuit 전용 신규 계획
MaxLeadDistanceCm                       // LeadPursuit 전용 신규 계획
```

호환 기본값:

```text
SeekerModel = LegacySingleGate
TargetObservationMode = DirectActorKinematics
```

따라서 기존 저장 ProjectileData는 재저장 없이 현재 Direct Actor Kinematics와 단일 Seeker Gate 행동을 유지해야 한다.

현재 적용 상태:

```text
- UCFProjectileData.MissileFlightConfig / MissileGuideConfig 기본 비활성 호환 유지
- UCFMissileFlightComp 생성 / Released → Ejection → Clearance → GuidedFlight Direct 상태 연결
- UCFMissileGuideComp 생성 / TargetActor 물리 제한형 Guidance Law Runtime 연결
- Launch Context GuidanceTargetActor 발사 순간 Snapshot 연결
- 차량의 이후 선택 Target 변경과 이미 발사된 미사일 목표 분리
- LegacySingleGate는 기존 Seeker FOV / Lock Break 단일 허용각 Gate를 호환 유지
- DirectActorKinematics는 TargetActor Location / Velocity 직접 관측을 호환 유지
- Stateful Seeker / SampledPositionEstimate는 MG-P0-09~10에서 구현 완료
- Guidance Law 선택은 MG-P0-12C에서 구현 완료: PurePursuit / LeadPursuit / ProportionalNavigation
- PN rear/non-closing Course Capture와 같은 activation의 Course Capture → PN 복귀를 MG-P0-12C에서 구현·검증 완료
- Guidance Activation은 MG-P0-12D에서 FollowFlightGuidanceWindow 호환 경로와 Independent Delay/Distance AND + activation latch로 분리 구현 완료
- Stateful Acquisition / Tracking / Reacquisition effective angle은 MG-P0-12D에서 서로 독립적인 0~180deg clamp로 구현 완료하며 기존 Tracking Min coupling 제거
- Target Lost Grace / ContinueStraight / Overshoot 연결
- Missile은 CurrentVelocityDirection 추진, 기존 Rocket은 FixedLaunchDirection 유지
- ProjectileMovement Sweep·Sub-step·실제 충돌·Damage·Pool 반환 계약 유지
- 전용 MissileTest Projectile·Weapon·Preset·VehicleData·복제 맵 적용 완료
- 기존 Rocket·Launcher·DA_TestSUV·TestMap 원본 SHA-256 실행 전후 동일
- Editor Build Job 09f8071ce6f4431e94c03c310c294c2e / Exit Code 0
- Direct Automation Job 7f7bdbd3a1434b7a895c50a9d7be2404 / Success
- Combat Regression Job 6bf2ebab7f9647578e76f9d339ea8699 / Required 21 of 21 Success
- Asset Apply Job 36e1068edba04489830db07a59fd66b7 / tool_version 1.0.1 / protected_contract_passed=true
- 2026-08-17 Persisted Asset readback은 current ue.batchdump_safe에서 Missile folder 20/20, Maps World 12/12 PASS로 WinError 87 미재현
- 계약 보유 4개 DataAsset은 Vehicle → EquipmentPreset → Weapon → Projectile 저장 참조와 Direct Missile 핵심 설정을 exact readback으로 확인
- /Game/Maps/MissileDirectTest는 저장 World package, 33 Actor와 Missile test vehicle/RocketLauncher socket 구성을 확인
- World Actor 전체 label은 current AssetDump 공개 preview 범위 밖이며 live SceneTools.find_actors도 server policy blocked이므로 5개 MissileTarget_* label 자체를 독립 PASS로 확대하지 않음
- CF-TC-027 Technical 범위는 2026-09-07 Fresh Rebaseline + Persisted DirectTest Damage Integration + Fire→Pool→Persisted ActorClass Review Correction의 결합 증거로 PASS
- Accepted Fire 실행 경계 이후 FireComp→Pool→저장 ProjectileActorClass와 production ProjectileMovement의 실제 Blocking 접촉을 Runtime Automation으로 확인
- CreateNewMap Automation의 swept Hit dispatch 공백은 접촉 판정 이후 테스트 전용 bridge로 production Supplemental Sweep→Impact→Damage를 이어 검증하며, 이를 전체 입력→명중 또는 전 구간 무텔레포트 E2E로 확대하지 않음
- 2026-09-07 USER PIE에서 측면 이동 Target 추적·명중을 확인했으나 과도하게 완벽한 예측성 때문에 USER Feel 미승인
- MG-P0-08 Source/API 계약검수는 GuidancePerformanceDesign v0.1.2에서 PASS
- 구현 직전 독립 설계검수 P0=0 / P1=6 / P2=2를 v0.1.3에서 교정
- v0.1.3 재검수 중 잔여 의미 모호성 P0=0 / P1=2 / P2=1을 추가 확인해 v0.1.4에서 교정
- v0.1.4에서 새로 확정한 HoldTargetLocation이 Pool Reset exact-list에서 누락된 P1=1을 v0.1.5에서 최종 교정
- MG-P0-12C 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS
- Official UE 5.8 Build 8c30576af506485f8e811fc8eb326806 / Succeeded / Exit 0
- Focused MG_P0_12C.GuidanceLawContract 1/1 PASS / 전체 CarFight.Missile 8/8 PASS
- Historical (v0.6.19 당시): 당시 exact Gate는 MG-P0-12 USER Guidance Feel Validation — Corrected Low Revalidation
```

---

## 4. Missile Flight State

```text
Inactive
→ Released
→ Ejection
→ Clearance
→ Ignition
→ Boost
→ Transition
→ GuidedFlight
→ Terminal
→ Impact / Expired
```

### Released

Launch Context를 복사한 직후다.
런처 Transform을 더 이상 조회하지 않는다.

### Ejection

런처가 제공한 초기 Velocity로 이동한다.
Motor와 Guidance는 설정에 따라 비활성일 수 있다.

### Clearance

점화·전환 전에 다음 안전 조건을 검사한다.

```text
MinimumClearanceTimeSeconds
MinimumClearanceDistanceCm
```

### Ignition / Boost

Motor가 점화되고 추진 가속을 생성한다.

### Transition

```text
- Direct 정렬
- Angled Ejection 후 목표 방위 전환
- Vertical Pitch-Over
- Loft 중간 목표 상승
```

### GuidedFlight

정상 유도 단계다.

### Terminal

종말 접근 또는 Top-Attack 하강 단계다.
현재 Direct Runtime에서는 별도 Terminal 전환을 구현하지 않았다.

MG-P0-08~12 Guidance Performance 교정에서 Terminal을 명중률 보정 수단으로 추가하지 않는다.
Terminal은 후속 Attack Profile·종말 센서·종말 유도 목적이 명확해졌을 때 별도 구현한다.

---

## 5. 계획 컴포넌트

### 5.1 UCFMissileFlightComp

책임:

```text
- Flight State
- Ejection 시간·거리
- Clearance
- Transition
- 현재 Flight Guidance Window 상태
- 발사 후 ElapsedFlightTime / DistanceFromRelease 측정
- Attack Profile 단계
- Flight Snapshot
- Pool Reset
```

`UCFMissileFlightComp`는 Independent Guidance Activation의 최종 on/off 결정을 소유하지 않는다. 측정값과 Flight Guidance Window를 제공하고, 실제 Guidance activation mode 판정/latch는 `UCFMissileGuideComp`가 소유한다.

### 5.2 UCFMissileGuideComp

현재 책임:

```text
- Target Actor Snapshot 소비
- 목표 관측
- Guidance Command 계산
- 제한값 적용
- 단일 Seeker 각도 Gate
- Target Lost
- Guide Snapshot
```

MG-P0-08~10 확장 책임:

```text
- LegacySingleGate / Stateful Seeker 모델 선택
- DirectActorKinematics / SampledPositionEstimate 관측 모델 선택
- Acquisition / Tracking / LostGrace / Reacquisition / LostFinal 상태
- 동일 Launch Target Snapshot만 재포착
- 위치 샘플 기반 Target Velocity Estimate
- 관측 사이 Estimated Target State 유지
- 신규 Seeker/Estimator 상태 Pool Reset
```

MG-P0-12B 이후 확장 책임:

```text
- FollowFlightGuidanceWindow / Independent Guidance Activation 판정과 latch
- PurePursuit / LeadPursuit / ProportionalNavigation Strategy 선택
- Law별 GuidanceAimPoint / Guidance Command 입력 구성
- PN rear/non-closing Course Capture와 approach 형성 후 PN 전환
- Stateful bOvershootArmed 관리와 실제 접근 후 Overshoot 종결
- exact 180deg rear steering deterministic tie-break 소비
- 신규 activation/law/overshoot 상태 Pool Reset
```

### 5.3 UCFProjectileMotorComp

기존 Rocket:

```text
FixedLaunchDirection
```

Missile 확장:

```text
CurrentVelocityDirection
또는
CurrentForwardDirection
```

Motor는 목표를 찾지 않는다.
Guide가 횡기동을 만들고 Motor는 현재 추진 방향으로 속력을 제공한다.

---

## 6. Guidance Mode

```text
None
TargetActor
LaserPoint
InertialPoint
DataLink
```

P0 순서:

```text
TargetActor
→ LaserPoint
```

`InertialPoint`와 `DataLink`는 후속 확장이다.

`DataLink`와 `DataLinkAssisted Reacquisition`은 MG-P0-08~12 범위에서 구현하지 않는다.

---

## 7. Target Actor Snapshot

발사 순간:

```text
TargetSelectComp 또는 Weapon TargetUse 평가
→ 유효 Target Actor 확인
→ Launch Context에 복사
→ Missile Actor가 독립 참조 소유
```

발사 후:

```text
차량 Target A → Target B 변경
미사일 A는 기존 Target A 유지
다음 미사일만 Target B 사용
```

약한 참조가 무효화되면 Target Lost 정책으로 전환한다.

MG-P0-09 Reacquisition도 이 계약을 깨지 않는다.

```text
Reacquisition
= 같은 GuidanceTargetActor Snapshot을 다시 Seeker 입력으로 받아들이는 것
≠ 주변 Actor 검색
≠ 차량의 현재 SelectedTarget 재조회
≠ 자동 Retarget
```

---

## 8. Laser Point

Laser Designator는 미사일을 움직이지 않는다.
현재 조사점을 Guidance Input으로 제공한다.

```text
Laser Point 갱신
→ Missile이 관측
→ 허용된 가속도 안에서 추적
```

레이저가 빠르게 이동하거나 끊기면 따라가지 못할 수 있다.

P0 Lost Policy:

```text
ContinueStraight
```

Laser Point는 MG-P0-08~12 TargetActor 성능 Variant 교정 범위에 포함하지 않는다.

---

## 9. 물리 제한형 Guidance

### 9.1 Guidance 입력

```text
MissileLocation
MissileVelocity
TargetLocation
TargetVelocityEstimate
DeltaSeconds
```

### 9.2 Guidance 출력

```text
LateralAccelerationCommand
```

### 9.3 기존 제한값

```text
MaximumTurnRateDegPerSec
MaximumLateralAccelCmPerSecSq
GuidanceResponseTimeSeconds
MinimumGuidanceSpeedCmPerSec
SeekerFieldOfViewDeg
LockBreakAngleDeg
TargetLostGraceTimeSeconds
```

### 9.4 P0 Guidance Law

제한된 비례항법 또는 동등한 시선 변화 기반 방식을 우선한다.

```text
요구 횡가속도 계산
→ MaximumLateralAccel 제한
→ MaximumTurnRate 제한
→ Velocity에 DeltaSeconds 기반 누적
```

PurePursuit는 MG-P0-12B에서 정식 Guidance Law Strategy로 설계 확정됐다. 현재 PN과 대조하기 위한 임시 fallback이 아니라 저성능/구형 미사일의 의도된 단순 추적 알고리즘으로 사용하며, LeadPursuit와 ProportionalNavigation은 더 복잡한 상위 전략으로 분리한다.

```text
PurePursuit
GuidanceAimPoint = LastObservedTargetLocation

LeadPursuit
LeadOffset = FilteredTargetVelocityEstimate * LeadTimeSeconds
LeadOffset magnitude <= MaxLeadDistanceCm
GuidanceAimPoint = LastObservedTargetLocation + LeadOffset

ProportionalNavigation
EstimatedTargetLocation + FilteredTargetVelocityEstimate + LOS/closing speed
rear/non-closing에서는 bounded Course Capture
ForwardClosingVelocity > 0 접근 형성 뒤 PN
```

LeadPursuit는 EstimatedTargetLocation을 기준점으로 다시 더해 이중 예측하지 않는다.

### 9.5 Guidance Performance Variant 원칙

미사일 성능은 하나의 등급값으로 결정하지 않는다.
각 `UCFProjectileData.MissileGuideConfig`가 개별 성능 축을 소유한다.

```text
[관측 성능]
TargetObservationMode
TargetObservationIntervalSeconds
TargetVelocityEstimateResponseTimeSeconds

[Guidance Law]
GuidanceLaw
LeadTimeSeconds
MaxLeadDistanceCm

[Guidance Activation]
GuidanceActivationMode
- FollowFlightGuidanceWindow = 기존 IsGuidanceWindowOpen 호환 기본값
- Independent = Delay/Distance AND 조건 + activation latch
GuidanceActivationDelaySeconds
GuidanceActivationDistanceCm

[유도 반응]
NavigationConstant
GuidanceResponseTimeSeconds

[기동 성능]
MaximumTurnRateDegPerSec
MaximumLateralAccelerationCmPerSecSq
MinimumGuidanceSpeedCmPerSec

[Seeker]
SeekerModel
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
TargetLostGraceTimeSeconds
ReacquisitionMode
ReacquisitionConeHalfAngleDeg
ReacquisitionTimeSeconds

[상실 이후]
LostTargetPolicy
```

`Low / Normal / High`는 예시 Authoring 분류일 수 있지만 Runtime 분기 조건이 아니다.

### 9.6 Target 관측 모델

호환 모드:

```text
DirectActorKinematics
- 현재처럼 TargetActor->GetActorLocation()
- 현재처럼 TargetActor->GetVelocity()
- 기존 Asset 기본 행동
```

신규 모드:

```text
SampledPositionEstimate
- StartMissileGuidance에서 최초 위치 샘플
- TargetObservationIntervalSeconds마다 새 위치 관측
- 위치 차분으로 Raw Target Velocity Estimate 계산
- TargetVelocityEstimateResponseTimeSeconds로 Filtered Velocity Estimate 생성
- 관측 사이에는 LastObservedLocation + FilteredVelocity * ObservationAgeSeconds로 Estimated Target Location 구성
- 매 frame TargetActor->GetVelocity() 정답 소비 금지
```

두 번째 샘플 전에는 Filtered Target Velocity Estimate를 0에서 시작한다.
목표가 급격히 방향을 바꾸면 다음 관측 전까지 기존 추정 오차가 남는 것이 정상이다.

첫 구현에는 랜덤 위치 오차·관측 누락을 넣지 않는다.

### 9.7 Seeker 모델

기존 호환:

```text
SeekerModel = LegacySingleGate
AllowedAngle = min(SeekerFieldOfViewDeg * 0.5, LockBreakAngleDeg)
```

신규 상태형:

```text
SeekerModel = Stateful

Inactive
→ Acquiring
→ Tracking
→ LostGrace
→ Reacquiring (optional)
→ LostFinal
```

Stateful 각도는 모두 **중심선 기준 반각**으로 정의한다.

```text
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
ReacquisitionConeHalfAngleDeg
```

현재 `SeekerFieldOfViewDeg=120`, `LockBreakAngleDeg=85`인 Legacy DirectTest는 실제 허용각이 `min(60,85)=60deg`다.
이 현재 동작을 버그로 취급하지 않고 Legacy 계약으로 보존한다.

Stateful에서는 세 반각을 각각 독립 0~180deg로 설정한다. Acquisition 또는 Reacquisition을 Tracking보다 넓게 둘 수도 있으며, 각 getter는 자기 값만 clamp하고 서로 `Min()`으로 결합하지 않는다. 기존 `LegacySingleGate`의 `min(SeekerFieldOfViewDeg*0.5, LockBreakAngleDeg)` 계약은 그대로 보존한다.

### 9.8 Equipment Lock과 Missile Acquisition 분리

```text
Equipment Lock-On
= 발사 전 Weapon/Equipment 소유
= Lock 진행률 / 완료 / 발사 허용 / HUD

Missile Seeker Acquiring
= 발사 후 UCFMissileGuideComp 소유
= Launch Snapshot Target이 온보드 Seeker 허용각에 들어오는지 확인
```

MG-P0-08~12가 Equipment Lock-On 구현을 대신하지 않는다.

---

## 10. Miss가 발생해야 하는 조건

```text
- 목표가 최소 사거리보다 가까움
- 발사 각도가 나쁨
- 수직발사 후 선회 고도 부족
- 목표가 뒤쪽에 있음
- 목표 측면 기동이 강함
- 미사일 속도가 지나치게 높아 선회 반경이 큼
- 관측 주기가 느려 목표 기동 추정이 늦음
- Target Velocity Estimate가 급기동을 아직 따라가지 못함
- 오버슈트
- Acquisition 실패
- Tracking Cone 이탈
- Reacquisition 실패 또는 미지원
- 장애물 가림
- 목표 파괴
- 연소 종료 후 에너지 부족
- Laser 상실
- LifeTime 종료
```

Automation과 PIE에서 성공 사례만 검증하지 않는다.

정지 Target도 다음을 구분한다.

```text
명확한 정면 충돌 기하 + Seeker 조건 충족
→ 실제 Blocking Hit이 기대됨

나쁜 발사 기하 + 오버슈트 + Tracking 상실
→ ContinueStraight / LifeExpired가 정상 결과일 수 있음
```

따라서 모든 정지 Target 미스를 Product defect로 간주하지 않는다.

---

## 11. Attack Profile

### Direct

초기 발사 방향과 목표 방향이 유사하다.

### Loft

목표보다 높은 중간 지점을 향한 뒤 정상 유도로 전환한다.

### PitchOver

수직 상승 후 제한된 선회로 목표 방위로 전환한다.

### TopAttack

상승 또는 Loft 후 목표 상부로 접근하고 Terminal에서 하강한다.

중간 목표는 Flight State가 제공한다.
Guide는 그 목표를 향해 허용된 기동만 수행한다.

---

## 12. Collision과 Damage

기존 `ACFProjectileActor`의 다음 계약을 유지한다.

```text
- Sweep / Sub-step
- Supplemental Sphere Sweep
- 첫 유효 Impact 한 번
- DamageHitContext
- Pool 반환
```

Guidance가 목표에 가까워졌다는 이유로 Damage를 적용하지 않는다.

MG-P0-08~12는 CollisionRadius, Sweep, Damage와 Pool 반환의 의미를 바꾸지 않는다.

---

## 13. Runtime Debug

현재:

```text
FlightState
GuideMode
AttackProfile
TargetActor
TargetLocation
TargetValid
SeekerAngle
RequestedLateralAccel
AppliedLateralAccel
RequestedTurnRate
AppliedTurnRate
TargetLostTime
MissReason
```

MG-P0-08~10 확장 후보:

```text
SeekerModel
SeekerState
TargetObservationMode
LastObservedTargetLocation
EstimatedTargetLocation
FilteredTargetVelocityEstimate
ObservationAgeSeconds
ReacquisitionElapsedTimeSeconds
```

MG-P0-12B 이후 append-only 진단 후보:

```text
GuidanceLaw
GuidanceActivationMode
bGuidanceActivationSatisfied
GuidanceAimPoint
bCourseCaptureActive
bOvershootArmed
```

단순 관측을 위해 불필요한 Product public getter를 추가하지 않는다.
기존 Snapshot과 Accepted RuntimeRead로 충분한지 먼저 검토한다.

MissReason은 Damage 결과가 아니라 유도 종료·실패 진단 값이다.

---

## 14. 구현 단계

기존 단계:

```text
MG-P0-00 Types / Config — Complete / 기본 비활성 호환 유지
MG-P0-01 Flight State — Complete for Direct / Build·Automation·Persisted Collision Integration PASS
MG-P0-02 Motor Direction Mode — Complete for CurrentVelocityDirection / Rocket 회귀 PASS
MG-P0-03 Target Guidance — Complete for TargetActor / 제한형 Guidance Automation PASS
MG-P0-04 Target Lifetime / Lost — Complete for Snapshot·ContinueStraight·Overshoot / Automation PASS
MG-P0-05 Laser Point — Pending / 제외 범위
MG-P0-06 Attack Profile — Direct만 Complete / Angled·Vertical·Loft·TopAttack Pending
MG-P0-07 Verification — Existing Direct Technical Verification Complete / 2026-09-07 USER Feel 미승인으로 Design Correction 발생
```

2026-09-07 설계 교정 후속 단계:

```text
MG-P0-08 Guidance Performance Variant Contract — Complete / Final Implementation Review PASS
- FCFMissileGuideConfig 신규 성능 축과 effective clamp 확정
- ECFMissileSeekerModel / ECFMissileTargetObservationMode / ECFMissileReacquisitionMode 확정
- Legacy compatibility path 확정
- Stateful Seeker API와 Snapshot-only reacquisition 계약 확정
- Overshoot > Reacquisition 우선순위 확정
- Sampled Estimated Target State 단일 Sensor Truth 확정
- Acquisition/Reacquisition effective cone <= Tracking cone 확정
- Destroyed Target은 grace 이후 Reacquisition 없이 LostFinal 확정
- Guidance Window 닫힘 동안 elapsed만 누적, 새 sample/Seeker 전이 없음 확정
- LostTargetPolicy=Expire는 Guide 진단만 소유하고 실제 Projectile lifetime은 기존 timer 소유
- Snapshot LastObserved / Estimated / VelocityEstimate 의미 확정
- Sampled observer의 sample interval / sample delta / extrapolation은 ObservationAgeSeconds 단일 clock 사용
- HoldLastKnownPoint는 LostFinal 진입 순간 Sensor Truth 위치를 1회 Freeze 후 외삽 금지
- Stateful Acquiring/LostGrace/Reacquiring은 bTargetValid=false / MissReason=None, 최종 LostFinal은 TargetLost
- HoldTargetLocation / bHasHoldTargetLocation Pool Reset 강제
- 첫 Reacquisition Product enum은 None/ForwardCone만 노출
- CFMissileGuideTypes.h v1.1.0 구현: Seeker/Observation/Reacquisition enum + Config + Effective clamp + Snapshot Foundation
- CFMissileFoundationTests.cpp v1.1.0 구현: CarFight.Missile.MG_P0_08.ConfigFoundation
- 기존 GuideComp public signature / serialized enum / shared Projectile integration 보호 확정
- Low/Normal/High Runtime 하드코딩 금지

MG-P0-09 Seeker State Model — Technical PASS / Complete
- ECFMissileSeekerState
- Acquisition / Tracking / LostGrace / Reacquisition / LostFinal
- Acquisition/Tracking angle 분리
- ReacquisitionMode=None/ForwardCone
- 기존 Target Snapshot과 LostTargetPolicy 호환
- 기존 DA 무변경 동작 호환 검증

MG-P0-10 Observation + Velocity Estimator — Technical PASS / Complete
- TargetObservationIntervalSeconds
- 위치 샘플 기반 Target velocity estimate
- TargetVelocityEstimateResponseTimeSeconds
- 관측 사이 EstimatedTargetLocation 외삽
- Sampled mode에서 매 frame TargetActor->GetVelocity() 정답 소비 금지
- deterministic Automation

MG-P0-11 Guidance Variant Verification Matrix — Technical PASS / Complete
- 저성능/기준/고성능 config fixture
- 같은 Target motion에서 반응 차이 증명
- 쉬운 정면 정지 Target 실제 Blocking Hit 가능성 검증
- Tracking cone 이탈 정상 Miss
- Reacquisition None/Enabled 차이
- Turn/Lateral limit 및 Pool Reset 회귀

MG-P0-12A USER Guidance Feel Test Setup — Technical Setup PASS / USER Low Feel Rejected
- 기존 DA_Missile_DirectTest는 regression baseline으로 보존하고 직접 덮어쓰지 않음
- persisted Low/Normal/High 자산을 만들지 않고 저장 DirectTest Projectile/Weapon/Equipment를 PIE Runtime에서 transient 복제
- transient ProjectileData의 MissileGuideConfig만 Variant별로 변경하고 Product Low/Normal/High enum 또는 runtime branch를 추가하지 않음
- 실제 VehicleWeaponComp::InitializeWeaponRuntimeFromFitting 경로로 transient EquipmentPreset 적용
- MissileDirectTest에서 Baseline/Low/Normal/High를 반복 전환하는 Editor-only 콘솔 명령 구현
- Baseline은 저장 EQ_Missile_DirectTest 재적용으로 복원
- 새 Setup Focused Automation 1/1 PASS, CarFight.Missile 전체 7/7 PASS, 공식 Editor Build PASS
- AI-side live PIE 시작은 current lifecycle interpreter identity mismatch + direct StartPIE server policy 차단으로 수행하지 않음
- USER Low 실발사 판정에서 같은 PN 계열에 수치만 다른 Variant 구조를 거부
- 기존 Setup/Build/Automation 7/7은 technical baseline으로 보존
- Next Exact Gate는 MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit

MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit — Design Correction + Re-review PASS / Complete
- 최초 설계검수 P0=2 / P1=5 / P2=3을 교정하고 재검수 P0=0 / P1=0 / P2=0 PASS
- PurePursuit / LeadPursuit / ProportionalNavigation exact semantics와 기존 PN 호환 기본값 확정
- PurePursuit LastObserved-only, LeadPursuit LastObserved + bounded velocity lead, PN Estimated state 소비 계약 확정
- rear/non-closing PN Course Capture → ForwardClosingVelocity>0 → PN 전환 계약 확정
- FollowFlightGuidanceWindow 호환 기본값과 Independent Delay/Distance AND + activation latch 계약 확정
- FlightComp 측정/window 제공 vs GuideComp activation decision/latch 소유권 확정
- Acquisition / Tracking / Reacquisition 0~180deg 독립 geometry와 current Min coupling 제거 확정
- Stateful approach-armed Overshoot와 Legacy overshoot 호환 경계 확정
- exact 180deg LaunchTransform Right Vector deterministic turn-side tie-break 확정
- Snapshot append-only 진단값과 Pool Reset exact-list 확정

MG-P0-12C Guidance Law Runtime — Technical PASS / Complete
- PurePursuit LastObserved-only, LeadPursuit bounded lead, 기존 PN 호환 경로와 rear/non-closing Course Capture 구현 완료
- 같은 activation에서 Course Capture가 실제 Guidance 결과 Velocity를 유지해 접근 기하를 형성한 뒤 PN으로 복귀하는 전환까지 Automation으로 검증
- Focused 1/1 PASS / CarFight.Missile 8/8 PASS / Official UE 5.8 Editor Build PASS
MG-P0-12D Guidance Activation + Rear Aspect Runtime — Final Technical PASS / Complete
- FollowFlightGuidanceWindow 기존 호환 + Independent Delay/Distance AND/latch 구현
- Stateful Acquisition / Tracking / Reacquisition 독립 0~180deg geometry 구현
- approach-armed Overshoot + exact 180deg Launch Right deterministic tie-break 구현
- Focused 1/1 PASS / CarFight.Missile 9/9 PASS / Official UE 5.8 Editor Build PASS
- 최종 재검수 P0=0 / P1=0 / P2=0 PASS
MG-P0-12E Guidance Preset DA Rewire — Post-Implementation Mid-review Correction + Re-review PASS / Complete
- CarFight_Re/Public/CFMissileGuidePresetData.h v1.2.0 + Private/CFMissileGuidePresetData.cpp v1.2.0
- CarFight_ReEditor/Private/CFMissileFeelCommands.cpp v1.5.0
- UCFMissileGuidePresetData는 PresetId/표시 이름/설명/MissileGuideConfig만 저장하는 passive data container이며 PostInitProperties/자산 이름 seed를 소유하지 않음
- Low/Normal/High 신규 seed는 CarFight_ReEditor authoring이 신규 Asset 생성 직후에만 적용하고 existing Asset은 seed/save/dirty 없이 재사용
- Low/Normal/High 수치 하드코딩을 Product Runtime에서 제거하고 /Game/Test/CarFight/Missile/FeelPresets의 Guidance Preset DA를 Runtime transient ProjectileData에 Guidance-only overlay
- Low = PurePursuit + Independent 0.25s / 500cm + Acquisition 45deg / Tracking 60deg / Reacquisition None
- Normal = LeadPursuit + Independent 0.06s / 100cm + Lead 0.18s / 900cm + Acquisition 75deg / Tracking 90deg / Reacquisition 120deg
- High = ProportionalNavigation + Independent 0/0 + Acquisition/Tracking/Reacquisition 180deg + 140deg/s turn
- CarFight.Authoring.MissileFeel.EnsurePresets는 실제 Low/Normal/High를 수정하지 않는 별도 임시 persisted Preset으로 USER 값 저장 → 다른 Variant Ensure 재실행 → package unload/디스크 reload 보존 → 임시 파일 정리까지 idempotence를 검증
- 기존 Guidance Preset AssetDump persisted readback 3/3 PASS 보존 / Fresh Authoring 1/1 PASS / Fresh Focused 1/1 PASS / Fresh CarFight.Missile 10/10 PASS / Official UE 5.8 Editor Build PASS
- DirectTest baseline persisted mutation 0 / Product Low/Normal/High enum·authoring branch 0
- 이후 Low/Normal/High 숫자 체감 튜닝은 Preset DA가 SSOT이며 수치 변경만으로 C++ rebuild 불필요

MG-P0-12 USER Guidance Feel Validation — USER ACCEPTED / Complete
- USER가 동일 MissileDirectTest PIE에서 Low / Normal / High를 모두 직접 확인
- 사용자 최종 표현: "얼추 PASS"
- 현재 P0 체감 품질 기준으로 Low/Normal/High 차이와 자연스러운 Miss 가능성을 수용
- 완벽한 밸런스 고정이 아니라 현재 기능 승인으로 해석하며 후속 수치 튜닝은 Guidance Preset DA에서 비차단으로 가능
- CF-TC-027 USER Feel = ACCEPTED
```

구현 순서:

```text
MG-P0-08
→ 중간 설계/API 검수
→ MG-P0-09
→ Automation
→ MG-P0-10
→ Automation
→ MG-P0-11 통합 Matrix
→ MG-P0-12A USER Feel Test Setup / Low USER rejection
→ MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit
→ MG-P0-12C Guidance Law Runtime
→ MG-P0-12D Activation + Rear Aspect Runtime
→ MG-P0-12E USER Feel Test Setup Rewire
→ USER MG-P0-12
```

---

## 15. CF-TC-027 검증 상태

Automation — 기존 Technical PASS:

```text
- 기본 Guidance 비활성
- Direct Clearance 뒤 GuidedFlight 전환
- 정지 정면 목표 불필요 횡기동 없음
- 측면 이동 Target 위치·Velocity 추적
- Launch Target Snapshot 독립
- 최대 Turn Rate 제한
- 최대 Lateral Accel 제한
- Target 파괴 뒤 TargetLost / ContinueStraight
- Overshoot 진단
- 동일 Projectile Actor Pool 재사용 Reset
```

CF-TC-027 Technical Integration — PASS:

```text
- persisted DA_Missile_DirectTest를 Runtime Automation에서 read-only 로드
- VehicleFireComp::ExecuteAcceptedFireCommand → BuildDirectProjectileLaunchContext → ProjectilePoolComp::AcquireProjectileWithContext 실제 실행
- 실제 Pool Actor Class == persisted ProjectileActorClass 확인
- GuidanceTargetActor Snapshot이 실제 Pool Missile의 MissileGuideComp까지 전달됨을 확인
- production ProjectileMovement가 순간이동 없이 발사점에서 Blocking 접촉 경계까지 실제 이동
- CreateNewMap Automation에서 swept OnComponentHit dispatch가 생략되는 경계는 실제 접촉 판정 뒤 테스트 전용 bridge로만 보완
- bridge 이후 production Supplemental Sweep → ResolveProjectileImpact 진입
- DefaultDamageData → DamageHitContext → VehicleDamageResult 전달
- transient VehicleHealthComp의 Target Integrity 실제 감소와 Pool 반환 확인
- 입력/HandleFireStarted/ValidateFireCommand는 이 시나리오의 Runtime 실행 범위가 아니며 TargetSelect 최초 Snapshot 캡처는 current Source audit 증거를 사용
```

2026-09-07 USER Feel Observation — **NOT ACCEPTED / Design Correction Triggered**:

```text
- USER가 MissileTarget_Lateral 추적을 직접 확인
- USER가 실제 명중을 확인
- 제한 선회 자체는 보였으나 Target motion에 대한 반응이 지나치게 예측적이고 완벽하게 느껴짐
- "명중률이 높다" 자체보다 유도 두뇌가 너무 완벽하게 목표 운동을 아는 느낌이 문제로 판정
- 정지 Target Miss는 재관찰 결과 현재 Seeker 허용각 이탈과 오버슈트 조건에서 정상 Miss가 될 수 있으므로 무조건 충돌 결함으로 확대하지 않음
```

따라서 기존 문구인:

```text
CF-TC-027에 남는 것은 측면 이동 목표 제한 선회 USER Feel 1건
```

은 더 이상 Current next gate가 아니다.
해당 Gate는 USER 관찰로 **미승인 + 설계 교정 입력**이 됐다.

Current CF-TC-027 후속 검증은 MG-P0-08~12를 따른다.

2026-09-07 Fresh Technical Rebaseline — PASS:

```text
- 발사 후 차량 Target 변경 무영향: DirectRuntimeContract + UCFVehicleFireComp 최초 GuidanceTargetActor Snapshot 보존으로 재확인
- 자동 파괴 목표 뒤 직진 유지: DirectRuntimeContract TargetLost / ContinueStraight PASS
- 비차단 목표 통과 뒤 Overshoot: DirectRuntimeContract Overshoot PASS
- 반복 발사 Pool 재사용 상태 오염 없음: 동일 Projectile Actor 재활성화 / Target·Flight Reset PASS
- 제한 선회 수치 계약: 최대 Turn Rate / 최대 Lateral Accel PASS
```

위 Technical PASS를 USER 확인으로 반복하지 않는다.
새 설계 구현에서는 관련 변경 영향 범위의 회귀만 재실행한다.

이번 Direct 검증에서 제외:

```text
- Angled / Vertical
- Ripple / Salvo
- Laser Point
- Loft / PitchOver / TopAttack
```

---

## 16. 검증 체크포인트

### 16.0 2026-08-02 최초 Direct Runtime 체크포인트 — Historical

```text
Build: PASS
- CarFight_ReEditor Win64 Development
- Job 09f8071ce6f4431e94c03c310c294c2e
- Exit Code 0

Direct Automation: PASS
- CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
- Job 7f7bdbd3a1434b7a895c50a9d7be2404
- State Success

Combat Regression: PASS
- Job 6bf2ebab7f9647578e76f9d339ea8699
- Required 21 / 21 Success
- Total Reported 43 / Failed 0

Test Asset Apply: PASS
- Job 36e1068edba04489830db07a59fd66b7
- tool_version 1.0.1
- protected_contract_passed=true
- 기존 Rocket·Launcher·SUV·TestMap 원본 해시 불변

Manual PIE: 당시 Pending
AssetDump Readback at 2026-08-02: Blocked / ue.batchdump_safe WinError 87 — Historical
```

### 16.1 2026-08-17 Persisted Missile Test Asset Technical Verification — PASS

이번 검증은 기존 Missile Runtime과 Content Asset을 재구현·수정하지 않고 저장된 테스트 패키지를 현재 read-only 도구로 독립 확인했다.

```text
Direct Runtime protection Automation: PASS
- CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
- Process 29d0ef16dd5e4d56945ceae26eec6937
- 1 Success / 0 Failure

Missile test asset managed dataset: PASS
- /Game/CarFight/Tests/Missile
- 20 Success / 0 Failure
- dataset_ref adset_v1_6400bb83933bd6f16f353584738d60cd.8d462194002b5b114d85abd8
- dataset_fingerprint c6dacf0a66852431da23fde4f2fe4ffdb4302f2a3256e3438f3fbbd8f74c314b

Persisted DataAsset contract: PASS
- DA_Missile_DirectTest: bUseMissileFlight=true / AttackProfile=Direct / bUseGuidance=true / GuideMode=TargetActor / LostTargetPolicy=ContinueStraight
- DA_Missile_DirectWeapon: Projectile / SingleCycle / ProjectileCountPerTrigger=1 / ReleaseMode=Direct / CarrierVelocityRatio=0 / DA_Missile_DirectTest hard reference
- EQ_Missile_DirectTest: Turret + Large / DA_RocketBody / DA_Missile_DirectWeapon hard reference
- DA_Missile_TestSUV: Top_01 Turret MountProfile → EQ_Missile_DirectTest hard reference

MissileDirectTest World package: PASS within current public readback scope
- /Game/Maps/MissileDirectTest.MissileDirectTest
- map dataset: 12 Success / 0 Failure
- map fingerprint: A4D2716A
- world_actor_count=33
- Missile test vehicle의 RocketLauncherYaw/RocketLauncherPitch와 Muzzle_1~4 socket 저장 구성을 확인

Readback boundary:
- current AssetDump World section은 전체 Actor label을 공개하지 않고 actor_preview 5개만 제공한다.
- AI-owned Editor를 read-only 확인용으로 시작했으나 SceneTools.find_actors는 server policy blocked였다.
- 따라서 MissileTarget_Static/Lateral/Change/Destroy/Overshoot 이름 자체는 이번 독립 readback PASS로 승격하지 않는다.
- 이 label과 실제 시나리오 동작은 CF-TC-027 Manual PIE에서 계속 확인한다.

Mutation / execution boundary:
- Missile Runtime Source 변경 0
- Content Asset / Blueprint / Map 저장 변경 0
- USER PIE 0
- Source 변경이 없으므로 새 Editor Build는 요구하지 않음
- read-only Editor lifetime: start 3c3367e9d8f445bc9357a127a7bd0cda → no-save stop 6849993dc7104fb3a128fc40f35e3682
```

판정: 과거 WinError 87로 비어 있던 **Persisted Missile Test Asset 기술 검증 공백은 현재 도구 기준으로 닫는다.** CF-FQ-030 전체 완료나 CF-TC-027 USER PASS로 확대하지 않는다.

### 16.2 2026-09-07 Post-VPS Fire Handoff Rebaseline — PASS

`CF-FQ-048 VPS-P0-02 Fire Behavior Extraction` 이후 실제 발사 실행 위치가 `ACFVehiclePawn`에서 `UCFVehicleFireComp`로 이동했으므로, 기존 CF-FQ-030 증거를 무조건 재사용하지 않고 현재 Source 계약과 Direct Runtime 보호 회귀를 다시 확인했다.

```text
Current Source contract audit: PASS
- UCFVehicleFireComp::HandleFireStarted가 첫 발사 입력에서 TargetSelectComp의 SelectedTargetActor를 GuidanceTargetActorSnapshot으로 1회 캡처한다.
- BuildDirectProjectileLaunchContext / ExecuteAcceptedFireCommand가 해당 Snapshot을 FCFProjectileLaunchContext.GuidanceTargetActor에 보존한다.
- ProjectilePoolComp AcquireProjectileWithContext 경로로 동일 LaunchContext가 ACFProjectileActor에 전달된다.
- Launcher Ripple/Salvo scheduled shot은 현재 TargetSelect를 다시 읽지 않고 최초 GuidanceTargetActorSnapshot을 재사용한다.
- ACFProjectileActor activation은 MissileFlightComp / MissileGuideComp에 ActiveLaunchContext를 전달하고 deactivation에서 Flight/Guide/Context를 Reset한다.

Fresh Direct Runtime protection: PASS
- Test: CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
- Process Job: d9c8a26520cb44139e2e42d4f7d44ea2
- Editor Exit Code: 0
- State: Success / passed=true
- Result SHA256: d8377419c046aca08a52a2c455514b9e23930cdf57bb0ee6734d0bf688e73ff5

Mutation boundary
- Missile Product Source 변경 0
- Content Asset / Blueprint / Map 저장 변경 0
- 기존 main_game 병렬 dirty 변경 0
- 새 기능 재구현 0
```

판정:

```text
- VPS-P0-02 이후에도 Fire → GuidanceTargetActor → Projectile/Missile handoff 계약은 유지된다.
- Target Snapshot, TargetLost/ContinueStraight, Overshoot, Pool Reset, 제한 선회 수치 계약은 Fresh Technical PASS다.
- CF-TC-027의 과거 Manual PIE 6개를 그대로 USER에게 반복시키지 않는다.
```

당시 next gate 기록은 Historical이다. Current next gate는 §16.5와 §14를 따른다.

### 16.3 2026-09-07 Persisted DirectTest Collision·Damage Integration — PASS / Historical Evidence

`DA_Missile_DirectTest`의 저장 Content 계약을 변경하지 않고 실제 Projectile 충돌·Damage 경로까지 Automation으로 닫았다.

```text
Persisted Asset evidence: PASS
- Asset: /Game/CarFight/Tests/Missile/DA_Missile_DirectTest.DA_Missile_DirectTest
- AssetDump dataset: adset_v1_5be4789f97b7aea39337908e6eeb7b06.15507b6018d08849c6f690a6
- Dataset fingerprint: 0312bfc70a545379c4ac2c586a655af387e2fac6e744fe356362487b678e5d8b
- bUseMissileFlight=true / AttackProfile=Direct
- bUseGuidance=true / GuideMode=TargetActor / LostTargetPolicy=ContinueStraight
- MaximumTurnRateDegPerSec=45
- MaximumLateralAccelerationCmPerSecSq=6000
- bUseSweepCollision=true / bUseSupplementalContinuousSweep=true
- DefaultDamageData=/Game/CarFight/Weapons/Data/DamageDefs/DA_DamageAsset

Test Source:
- CFMissileRuntimeTests.cpp v1.2.0
- 기존 DirectRuntimeContract에 persisted Content Integration 시나리오 1건 추가
- Product Runtime Source 변경 0
- Content Asset / Blueprint / Map 저장 변경 0

Official Editor Build:
- Job de9e8e236d4a4c4aa17a90b5a2167cf1
- CarFight_ReEditor Win64 Development
- Exit Code 0 / PASS

Fresh DirectRuntimeContract:
- Process Job a545e0eab0024591b159aba82da61bf4
- Test CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
- State Success / passed=true / Editor Exit Code 0
- Result SHA256 d0b79868179c482a735e105e0e0629d1575f5b554048fbd34fcca3225972661b

Verified production chain:
- persisted DA_Missile_DirectTest read-only load
- Missile activation + Guidance Target Snapshot
- production Supplemental Sphere Sweep
- ResolveProjectileImpact
- DefaultDamageData → DamageHitContext
- VehicleDamageResult
- Target VehicleHealth Integrity 감소
- DeactivateReason=Hit
```

구현 중 첫 build에서 테스트가 protected `ACFProjectileActor::Tick()`을 직접 호출해 C2248로 실패했다. Product API를 공개하거나 friend seam을 추가하지 않고 Automation `UWorld::Tick()`으로 실제 Actor/Component Tick 경로를 실행하도록 교정했고, 이후 공식 Build와 Automation이 모두 PASS했다. 이 실패는 Product Runtime 결함이 아니다.

판정:

```text
- 이 v1.2.0 체크포인트는 persisted DA의 Supplemental Sweep → Impact → Damage 경로를 검증한 Historical evidence다.
- 이후 독립 Review에서 FireComp/Pool/persisted ProjectileActorClass 및 실제 비행 접촉까지 증명한 것으로 확대 해석한 부분은 과장으로 판정되어 v0.5.3 / §16.4에서 교정했다.
```

### 16.4 2026-09-07 Review Correction / Fire→Pool→Persisted ActorClass — PASS

v0.5.2 독립 검수에서 `CFMissileRuntimeTests.cpp v1.2.0`의 시나리오 7이 base `ACFProjectileActor`를 직접 생성하고 `ActivateProjectileWithContext()`를 직접 호출한 뒤 위치 이동으로 Supplemental Sweep을 유도했으므로, 이를 FireComp/Pool/persisted ActorClass 또는 전 구간 실제 비행 충돌 증거로 확대할 수 없다는 P1 2건 / P2 1건을 확인했다.

교정은 Product Runtime API나 Content를 변경하지 않고 Automation evidence만 강화했다.

```text
Test Source:
- CFMissileRuntimeTests.cpp v1.3.0
- persisted DA_Missile_DirectTest read-only 사용
- transient VehicleData / EquipmentPresetData / WeaponData로 실제 VehicleWeaponComp runtime 구성

Runtime accepted-execution boundary: PASS
- UCFVehicleFireComp::ExecuteAcceptedFireCommand
→ BuildDirectProjectileLaunchContext
→ UCFProjectilePoolComp::AcquireProjectileWithContext
→ persisted ProjectileActorClass Actor 생성·활성화
- 실제 Pool Actor Class == DA_Missile_DirectTest.ProjectileActorClass
- GuidanceTargetActor가 실제 Pool Missile의 MissileGuideComp까지 전달됨

Boundary note:
- 이 시나리오는 ValidateFireCommand 이후의 accepted execute 경계에서 시작한다.
- HandleFireStarted의 TargetSelect 최초 Snapshot 캡처와 ValidateFireCommand 자체는 이 시나리오에서 실행하지 않는다.
- 최초 Snapshot 캡처 / scheduled-shot 재사용은 §16.2 current Source audit evidence가 소유한다.
- 따라서 전체 입력→Damage E2E라고 부르지 않는다.

Production movement contact: PASS
- CreateNewMap Editor Automation의 일반 PIE movement scheduling에 의존하지 않고 실제 UProjectileMovementComponent::TickComponent를 진행
- Start=(0,0,2000)
- Target=(1000,0,2000)
- 21 step 뒤 Blocking=(867,0,2000)
- Target center distance=133cm
- Target radius 120cm + Missile radius 12cm의 이론 접촉 경계 약 132cm와 일치
- Blocking에서 ProjectileMovement Velocity=0
- 위 접촉 판정까지 Projectile Actor 위치 순간이동 0

Automation impact-dispatch boundary:
- CreateNewMap Automation에서는 swept ProjectileMovement가 정확한 Blocking 접촉에서 정지해도 PIE와 같은 OnComponentHit dispatch가 발생하지 않았다.
- DispatchBeginPlay를 보완해도 동일했으므로 Product 결함으로 확대하지 않고 Automation harness 경계로 분리했다.
- 실제 Blocking 접촉을 먼저 독립 PASS한 뒤에만 테스트 전용 위치 bridge를 사용해 production Supplemental Sweep에 Impact를 인계했다.
- bridge 이후 ResolveProjectileImpact → DamageHitContext → VehicleDamageResult → VehicleHealth Integrity 감소 → DeactivateReason=Hit → Pool 반환 PASS
- 이 bridge 때문에 전체 충돌 과정을 '전 구간 무텔레포트 실제 비행 Impact'라고 표현하지 않는다.

Final Official Editor Build:
- Job 31197526b5df47859beb5db28c6230e0
- CarFight_ReEditor Win64 Development
- Exit Code 0 / PASS

Final DirectRuntimeContract:
- Process Job a54742a54b754361a452c33e290d8e48
- Test CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
- State Success / passed=true / Editor Exit Code 0
- Result SHA256 7676eda6ee34865f47709cb3f40fad29c46616a51359f68c7a3efb4f6952bc0b
- Runtime diagnostic: Steps=21 / Blocking X=867 / Target X=1000 / CenterDistance=133
- minimal transient vehicle의 SM_Body 누락 경고 1건은 테스트 실패가 아니며 missile Product defect로 분류하지 않음

Mutation boundary:
- Missile Product Runtime Source 변경 0
- Content Asset / Blueprint / Map 저장 변경 0
- Product public API 확장 0
- 기존 병렬 dirty 임의 수정 0
```

교정 과정의 compile/link/automation 실패는 모두 테스트 구현 또는 실행 환경 진단으로 분류한다. protected Pawn facade 직접 호출은 public `UCFVehicleFireComp` 경계 사용으로 교정했고, AI-owned Editor DLL 점유는 ownership-safe no-save stop 뒤 공식 build를 재실행했다. 실제 movement 접촉 뒤 Hit dispatch 공백은 위 Automation boundary로 명시했다.

재검수 판정:

```text
P0 = 0
P1 = 0
P2 = 0

- 기존 P1-1 FireComp→Pool Runtime evidence 공백: Closed
- 기존 P1-2 persisted ProjectileActorClass Runtime evidence 공백: Closed
- 기존 P2-1 teleport 기반 충돌 증거 과장: Closed by evidence separation
- CF-TC-027 Technical은 위 정확한 증거 경계에서 Complete / PASS
```

당시 `다음 exact Gate는 USER 측면 이동 목표 제한 선회 체감 1건`이라는 문장은 Historical이다. 해당 USER Gate는 §16.5에서 미승인으로 관측되어 설계 교정을 열었다.

### 16.5 2026-09-07 USER Guidance Feel → Performance Variant Design Correction — ACCEPTED

USER가 실제 `MissileDirectTest` PIE에서 측면 이동 Target 추적과 명중을 확인했다.

USER 판정:

```text
- 추적한다.
- 명중한다.
- 그러나 지나치게 예측적으로 움직이는 느낌이 있다.
- 단순히 명중률이 높은 것이 아니라 움직임이 비현실적으로 완벽하게 느껴진다.
```

추가 관찰:

```text
- 정지 Target에서 빗나가는 현상을 다시 자세히 보면 Missile이 무조건 Target을 따라가는 구조가 아니다.
- Target이 현재 Seeker 허용각 밖으로 벗어나면 Guidance가 상실될 수 있다.
- Missile이 Target을 추적하며 가속한 뒤 Target을 지나쳐 각도 밖으로 보내면 ContinueStraight/LifeExpired가 될 수 있다.
- 따라서 이 현상을 무조건 Product collision defect 또는 "무조건 재추적해야 함" 요구로 분류하지 않는다.
```

Current Source audit:

```text
- TryResolveTargetObservation은 TargetActor->GetActorLocation() / GetVelocity()를 매 Guidance step 직접 소비
- IsTargetInsideSeekerLimits는 AllowedSeekerAngle = min(Seeker FOV half-angle, LockBreakAngle)
- DA_Missile_DirectTest 저장값 SeekerFieldOfViewDeg=120, LockBreakAngleDeg=85
- Legacy 실제 허용각은 min(60,85)=60deg
- Acquisition과 Tracking 유지각이 별도 상태로 분리돼 있지 않음
```

설계 결정:

```text
1. 하나의 Missile을 "적당히 덜 잘 맞게" 튜닝하는 방향을 폐기한다.
2. 명중률 확률값을 추가하지 않는다.
3. Missile별 관측/추정/Seeker/기동 성능을 Data로 다르게 구성한다.
4. 기존 Asset은 LegacySingleGate + DirectActorKinematics로 현재 행동 보존.
5. 신규 Stateful Seeker에서 Acquisition / Tracking / LostGrace / Reacquisition / LostFinal을 분리.
6. 신규 SampledPositionEstimate에서 관측 주기와 위치 샘플 기반 Velocity Estimate를 사용.
7. Reacquisition은 같은 Launch Target Snapshot만 허용하고 자동 Retarget 금지.
8. Terminal은 강제 명중 보정에 사용하지 않음.
```

상세 설계:

```text
GuidancePerformanceDesign.md v0.1.0
→ 설계검수 P0=0 / P1=1 / P2=0

P1:
- 신규 반각 필드가 기존 Asset에서 자동 파생된다고만 하면 UE 신규 UPROPERTY 저장 여부를 안정적으로 구분할 수 없음

Correction:
- ECFMissileSeekerModel = LegacySingleGate / Stateful
- ECFMissileTargetObservationMode = DirectActorKinematics / SampledPositionEstimate
- 기본값은 모두 Legacy-compatible

GuidancePerformanceDesign.md v0.1.1
→ 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS
```

판정:

```text
- 기존 MG-P0-00~07 Technical baseline: 유지
- 기존 단일 USER Turn Feel Gate: NOT ACCEPTED / Design Correction input으로 종료
- CF-FQ-030: 여전히 Active, Done 아님
- Product Source mutation: 0
- Content Asset mutation: 0
- 이번 단계는 문서·설계 교정이므로 새 Build/Automation 요구 없음
- 당시 Next exact Gate: MG-P0-08 Guidance Performance Variant Contract 구현 착수 전 Source/API 계약검수 — Historical / §16.6~16.8에서 완료
```

### 16.6 2026-09-07 MG-P0-08 Source/API Contract Review — PASS

`GuidancePerformanceDesign.md v0.1.1`을 current C++ Source와 대조해 실제 구현 경계를 검수했다.

Current Source 확인:

```text
- CFMissileGuideMath는 이미 TargetLocation / TargetVelocityEstimate를 순수 입력으로 받으므로 Guidance Law 재설계 불필요
- UCFMissileGuideComp가 Target Actor 관측, Seeker gate, LostGrace, Overshoot와 Guidance response를 소유
- ACFProjectileActor는 기존 StartMissileGuidance / ResetMissileGuidance 경계로 충분
- UCFProjectileData는 FCFMissileGuideConfig 중첩 + GetEffectiveMissileGuideConfig() 경계로 충분
- FCFProjectileLaunchContext는 GuidanceTargetActor Snapshot을 이미 소유하며 별도 Retarget 입력 추가 불필요
- CFMissileRuntimeTests.cpp v1.3.0은 기존 CF-FQ-030 Fire→Pool→persisted ActorClass correction dirty를 보유
```

초기 계약검수:

```text
P0 = 0
P1 = 3
P2 = 2
```

교정 항목:

```text
P1-1 Reacquisition과 Overshoot 우선순위 미정
→ Overshoot를 Reacquisition보다 우선하는 terminal miss로 확정

P1-2 Sampled mode에서 Seeker/Overshoot가 실제 Actor truth를 다시 읽을 여지
→ Estimated Target State를 Seeker/상태전이/PN/Overshoot의 단일 Sensor Truth로 확정

P1-3 public API / serialized enum / shared integration owner 경계 미정
→ UCFMissileGuideComp 기존 public signature와 기존 enum 순서 보존, GuideMath/ProjectileActor/LaunchContext/ProjectileData API no-touch 기본 경계 확정

P2-1 low FPS에서 한 Tick에 synthetic observation 여러 개 생성 가능성
→ 실제 Actor sample은 Guidance Tick당 최대 1회, 누적 실제 elapsed time으로 estimator 갱신

P2-2 기존 dirty RuntimeTests에 신규 상태 테스트 누적 위험
→ Foundation config test + 신규 CFMissileGuideStateTests.cpp로 분리
```

확정 구현 Owner:

```text
변경:
- CFMissileGuideTypes.h
- CFMissileGuideComp.h / .cpp
- CFMissileFoundationTests.cpp
- 신규 CFMissileGuideStateTests.cpp

기본 no-touch:
- CFMissileGuideMath.h / .cpp
- CFProjectileLaunchTypes.h
- CFProjectileActor.h / .cpp
- CFProjectileData public API
- 기존 CFMissileRuntimeTests.cpp v1.3.0 integration evidence
```

호환 기본값:

```text
SeekerModel = LegacySingleGate
TargetObservationMode = DirectActorKinematics
ReacquisitionMode = None
```

교정 후 재검수:

```text
P0 = 0
P1 = 0
P2 = 0

PASS
```

Mutation / validation boundary:

```text
- Product C++ Source mutation 0
- Content Asset / Blueprint / Map mutation 0
- 문서·계약 검수만 수행
- Source 변경이 없으므로 새 Editor Build / Automation 불필요
- 기존 main_game dirty와 CFMissileRuntimeTests.cpp v1.3.0 dirty를 변경하지 않음
```

판정:

```text
MG-P0-08 Source/API Contract Review = Complete / PASS
```

### 16.7 2026-09-07 Implementation-Ready Design Re-review — PASS

`GuidancePerformanceDesign.md v0.1.2`를 current Guide/Flight/Projectile Source와 다시 대조한 구현 직전 독립 설계검수에서 상태 머신과 시간 계약의 해석 공백을 재점검했다.

초기 재검수:

```text
P0 = 0
P1 = 6
P2 = 2

CORRECTION REQUIRED
```

교정 내용:

```text
- Acquisition cone 밖의 유효 Target은 Acquiring 유지, LostGrace 미진입
- Target Actor invalid/destroyed는 grace 이후 Reacquisition 없이 LostFinal
- Effective Acquisition/Reacquisition cone은 Tracking cone 이하
- Start sample 뒤 Guidance Window가 닫혀 있을 때 observation elapsed만 누적
- Window open 시 interval 충족이면 실제 Actor sample을 최대 1회 즉시 취득
- LostTargetPolicy=Expire는 Guide 종료 + LifeExpired 진단만 소유, 실제 Projectile 만료/Pool 반환은 기존 LifeTime timer 소유
- FCFMissileGuideSnapshot 기존 TargetLocation 의미 보존 + LastObserved/Estimated/VelocityEstimate/ObservationAge 등 신규 상태 exact-list 확정
- 첫 ECFMissileReacquisitionMode Product enum은 None/ForwardCone만 노출
- TargetLostGraceTime=0 / ReacquisitionTime=0은 same-step 상태 전이
- 모델별 DataAsset EditCondition 확정
```

추가 Source 확인:

```text
ACFProjectileActor prerequisite:
ProjectileMotor → MissileFlight → MissileGuide → ProjectileMovement

→ Tick architecture 변경 불필요
```

교정 후 재검수:

```text
P0 = 0
P1 = 0
P2 = 0

PASS
```

판정:

```text
GuidancePerformanceDesign.md v0.1.3 = Correction Complete
Final implementation-readiness re-review = §16.8에서 계속
```

Mutation boundary:

```text
- Product C++ Source mutation 0
- Content Asset / Blueprint / Map mutation 0
- 문서 설계 교정만 수행
- 기존 main_game dirty 및 CFMissileRuntimeTests.cpp v1.3.0 dirty 보존
- Source 변경이 없으므로 Build / Automation 불필요
```

### 16.8 2026-09-07 Final Implementation-Readiness Re-review — PASS

v0.1.3 전체 교정본을 다시 읽으면서 구현자가 서로 다른 내부 상태를 만들 수 있는 잔여 의미 모호성을 한 번 더 검수했다.

추가 재검수:

```text
P0 = 0
P1 = 2
P2 = 1
```

추가 교정:

```text
P1-1 Sample interval용 ObservationElapsedTimeSeconds와 extrapolation용 TimeSinceLastObservationSeconds가 동일한 시간 의미를 중복 소유
→ ObservationAgeSeconds 단일 clock으로 통합
→ sample interval 판정 / raw velocity sample delta / EstimatedTargetLocation extrapolation이 같은 시간을 소비

P1-2 HoldLastKnownPoint의 Freeze 위치 출처가 모호
→ LostFinal 진입 순간 Guide가 소비하던 Sensor Truth 위치를 HoldTargetLocation으로 1회 Freeze
→ Direct는 마지막 유효 직접 관측 위치, Sampled는 진입 순간 EstimatedTargetLocation
→ Freeze 이후 estimator velocity로 위치를 계속 움직이지 않음

P2-1 Stateful 임시 상태의 Debug 의미 미정
→ Acquiring / LostGrace / Reacquiring은 bTargetValid=false / MissReason=None
→ target invalid 또는 상태 exhaustion으로 LostFinal이면 TargetLost
→ Overshoot는 Overshoot, LostTargetPolicy=Expire는 LifeExpired
→ Legacy FOV/LockBreak 진단은 기존대로 보존
```

최종 재검수:

```text
P0 = 0
P1 = 0
P2 = 0

PASS
```

판정:

```text
GuidancePerformanceDesign.md v0.1.4 = Final Implementation-Ready
→ Pool reuse state completeness final check = §16.9에서 계속
```

Mutation boundary:

```text
- Product C++ Source mutation 0
- Content Asset / Blueprint / Map mutation 0
- Plan/Design 문서만 교정
- 기존 main_game dirty 및 CFMissileRuntimeTests.cpp v1.3.0 dirty 보존
- Source 변경이 없으므로 Build / Automation 불필요
```

### 16.9 2026-09-07 Pool Reuse State Completeness Final Correction — PASS

v0.1.4에서 `HoldLastKnownPoint`의 고정 지점을 `HoldTargetLocation`으로 새로 확정한 뒤 Pool 재사용 계약을 다시 대조했다.

추가 검수:

```text
P0 = 0
P1 = 1
P2 = 0
```

교정:

```text
P1-1 HoldTargetLocation / bHasHoldTargetLocation이 ResetMissileGuidance exact-list에서 누락
→ HoldTargetLocation = Zero
→ bHasHoldTargetLocation = false
→ 새 activation이 이전 Missile의 Hold point를 재사용하지 않도록 강제
→ Pool-style Reset Automation 요구사항에 동일 검증 추가
```

대표 Plan의 용어 projection도 상세 설계와 정렬했다.

```text
TimeSinceObservation → ObservationAgeSeconds
ReacquisitionTimeSeconds(debug runtime state) → ReacquisitionElapsedTimeSeconds
```

교정 후 최종 재검수:

```text
P0 = 0
P1 = 0
P2 = 0

PASS
```

판정:

```text
GuidancePerformanceDesign.md v0.1.5 = Final Implementation-Ready
MG-P0-08 Types / Config Foundation Gate = OPEN
Next exact Gate = MG-P0-08 Types / Config Foundation Implementation
```

Mutation boundary:

```text
- Product C++ Source mutation 0
- Content Asset / Blueprint / Map mutation 0
- Plan/Design 문서만 교정
- 기존 main_game dirty 및 CFMissileRuntimeTests.cpp v1.3.0 dirty 보존
- Source 변경이 없으므로 Build / Automation 불필요
```

### 16.10 2026-09-07 MG-P0-08 Types / Config Foundation Implementation — AUTOMATION PASS / BUILD RECEIPT PENDING

최종 설계 v0.1.5를 기준으로 MG-P0-08의 Runtime 비활성 Foundation만 구현했다.

Source 변경:

```text
UE/Source/CarFight_Re/Public/CFMissileGuideTypes.h v1.1.0
- ECFMissileSeekerModel
- ECFMissileTargetObservationMode
- ECFMissileSeekerState
- ECFMissileReacquisitionMode
- FCFMissileGuideConfig 신규 성능 축
- 신규 effective clamp / angle invariant
- FCFMissileGuideSnapshot append-only Foundation 확장

UE/Source/CarFight_Re/Private/CFMissileFoundationTests.cpp v1.1.0
- CarFight.Missile.MG_P0_08.ConfigFoundation 추가
```

호환 기본값:

```text
SeekerModel = LegacySingleGate
TargetObservationMode = DirectActorKinematics
ReacquisitionMode = None
```

따라서 MG-P0-08 구현만으로 기존 저장 Missile의 Runtime 추적 행동은 바뀌지 않는다.
`UCFMissileGuideComp`의 Stateful 상태 전이와 Sampled observer/estimator는 아직 구현하지 않았다.

보호 범위:

```text
- CFMissileGuideComp.h/.cpp 변경 0
- CFMissileGuideMath.h/.cpp 변경 0
- CFProjectileLaunchTypes.h 변경 0
- CFProjectileActor.h/.cpp 변경 0
- CFProjectileData public API 변경 0
- 기존 CFMissileRuntimeTests.cpp v1.3.0 변경 0
- Content Asset / Blueprint / Map 저장 변경 0
- 기존 ActiveWork / FeatureQueue / VehiclePanel dirty 임의 수정 0
```

Focused Automation:

```text
Process 02c4f2a145454c36a6f59d0b5032dcba
CarFight.Missile.MG_P0_08.ConfigFoundation
Engine Exit Code 0
Success 1 / Failure 0
```

Affected Missile Regression:

```text
Process 783cd387684a4a1fa364de3535587ee3
Filter CarFight.Missile
Engine Exit Code 0
Success 3 / Failure 0

PASS CarFight.Missile.MG_P0_00.FoundationContract
PASS CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
PASS CarFight.Missile.MG_P0_08.ConfigFoundation
```

참고 실행:

```text
Process ec990362723e4030b625d3036ad22031
- ExactTestNames 3개를 PowerShell positional 인수로 전달한 runner 호출이 테스트 시작 전에 Parameter Binding 실패
- Product / UE Automation failure 아님
- Source/Content mutation 0
- 이후 CarFight.Missile filter 실행으로 동일 3개 테스트를 정상 검증
```

Official Editor Build:

```text
Job b2cfa60513ec4fd6af5467722b697ba1
Preset carfight.editor.development
Engine D:\UnrealEngine_Source
Target CarFight_ReEditor Win64 Development
UHT 진입 확인
```

Later canonical interaction에서 exact Build Job terminal receipt를 회수했다.
`Result: Succeeded / Exit Code 0`이며 UHT 5 generated files, `CFMissileFoundationTests.cpp`, `CFMissileRuntimeTests.cpp`, `UnrealEditor-CarFight_Re.dll` link까지 모두 PASS했다.
Focused 1/1 + Missile regression 3/3과 결합해 MG-P0-08 최종 구현검수를 P0=0 / P1=0 / P2=0 PASS로 닫는다.

현재 판정:

```text
MG-P0-08 Source Implementation = Complete
MG-P0-08 Final Implementation Review = PASS / P0=0 P1=0 P2=0
MG-P0-08 Focused Automation = PASS
Missile affected regression = PASS 3/3
Official Editor Build = PASS / Exit Code 0
MG-P0-09 Seeker State Model = Technical PASS / P0=0 P1=0 P2=0
MG-P0-10 Observation + Velocity Estimator = Technical PASS / P0=0 P1=0 P2=0
MG-P0-11 Guidance Variant Verification Matrix = Final Correction + Re-review PASS / P0=0 P1=0 P2=0
MG-P0-12A USER Guidance Feel Test Setup = Technical Setup PASS / USER Low Feel Rejected
MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit = Design Correction + Re-review PASS / P0=0 P1=0 P2=0 / Complete
MG-P0-12C Guidance Law Runtime = Technical PASS / Complete / P0=0 P1=0 P2=0
MG-P0-12D Guidance Activation + Rear Aspect Runtime = Final Technical PASS / Complete / P0=0 P1=0 P2=0
MG-P0-12E USER Feel Test Setup Rewire = Technical PASS / Complete
MG-P0-12 USER Guidance Feel Validation = Low First Pass Historical REJECTED / corrected Low·Normal·High USER ACCEPTED / Complete
CF-TC-027 = Technical + USER Feel Complete / PASS
CF-FQ-030 = Done / Direct P0 Complete / Current System Promotion Complete / Historical + Retained Path
```

P0 Closure 결과:

```text
Current System owner
- Document/Systems/Combat/MissileGuidance.md v1.0.0
- Document/Systems/Combat/Projectile.md v1.9.0 — 공통 Projectile Actor 통합 경계

P0 완료 범위
- Direct TargetActor Missile Flight / Guidance
- PurePursuit / LeadPursuit / ProportionalNavigation
- Independent Guidance Activation
- Stateful Seeker / Sampled Observation / Reacquisition
- rear-aspect / Overshoot 결정성
- Pool Reset
- passive Guidance Preset authoring/tuning
- Low / Normal / High USER Feel acceptance

P0 비차단 후속 범위
- LaserPoint 실제 Guidance Runtime
- DataLink / InertialPoint 실제 Guidance Runtime
- Angled / Vertical / Loft / TopAttack
- LostTargetPolicy=Expire의 실제 Projectile 종료 연결
- 장비별 Multi-Muzzle / Salvo / 다중 방향 사출 연출 구성
```

MG-P0-12C/12D Product Runtime과 MG-P0-12E 시험 인프라의 Technical PASS는 유지한다. USER Feel도 ACCEPTED이므로 현재 CF-FQ-030 P0 blocker는 0이다. 위 후속 범위는 필요할 때 별도 Feature/작업 lifecycle로 연다.

### 16.11 2026-09-07 MG-P0-09 Stateful Seeker Runtime — TECHNICAL PASS

MG-P0-08의 append-only Config Foundation을 기준으로 `UCFMissileGuideComp`에 발사 후 Stateful Seeker 상태 모델을 구현했다.

Product Source:

```text
UE/Source/CarFight_Re/Public/CFMissileGuideComp.h v1.1.0
UE/Source/CarFight_Re/Private/CFMissileGuideComp.cpp v1.1.0
```

신규 Automation Source:

```text
UE/Source/CarFight_Re/Private/CFMissileGuideStateTests.cpp v1.0.0
CarFight.Missile.MG_P0_09.StatefulSeekerContract
```

구현 상태:

```text
Inactive
→ Guidance Window open 시 Acquiring

Acquiring
→ AcquisitionCone 안이면 Tracking
→ 유효 Target이 cone 밖이면 Acquiring 유지 / Command 없음
→ Target ref invalid면 LostFinal

Tracking
→ TrackingCone 안이면 유지
→ 이탈/관측 상실이면 LostGrace

LostGrace
→ grace 안에 같은 Snapshot Target이 TrackingCone 복귀하면 Tracking
→ invalid Target은 grace 종료 뒤 LostFinal
→ valid Target + None은 LostFinal
→ valid Target + ForwardCone은 Reacquiring

Reacquiring
→ 같은 Launch Snapshot Target만 검사
→ ReacquisitionCone 안이면 Tracking
→ invalid/timeout이면 LostFinal
→ 주변 Actor 자동 Retarget 0

LostFinal
→ ContinueStraight: Guide Tick 종료 / Projectile lifetime 보존
→ HoldLastKnownPoint: LostFinal 순간 고정점 Freeze / Actor Tracking 재개 0
→ Expire: Guide-level LifeExpired 진단 / 실제 Projectile timer·Pool return 소유권 유지

Overshoot
→ Reacquisition보다 우선하는 LostFinal terminal miss
```

Pool-style Reset:

```text
SeekerState = Inactive
PreviousObservedTargetLocation = Zero
LastObservedTargetLocation = Zero
EstimatedTargetLocation = Zero
FilteredTargetVelocityEstimate = Zero
ObservationAgeSeconds = 0
ReacquisitionElapsedTimeSeconds = 0
HoldTargetLocation = Zero
observation/hold validity flags = false
GuidanceTargetActor = null
GuideActivationCount = preserved
```

Focused Automation:

```text
Process 52ecbb245246445aaa75e529e69b07e0
CarFight.Missile.MG_P0_09.StatefulSeekerContract
Engine Exit Code 0
Success 1 / Failure 0
```

Affected Missile Regression:

```text
Process 00f1b39cc8d840d9839397816d802f4c
Filter CarFight.Missile
Engine Exit Code 0
Success 4 / Failure 0

PASS CarFight.Missile.MG_P0_00.FoundationContract
PASS CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
PASS CarFight.Missile.MG_P0_08.ConfigFoundation
PASS CarFight.Missile.MG_P0_09.StatefulSeekerContract
```

Official Build 첫 실행:

```text
Job 5a76c185be894db08345c9c93a4d246c
Exit Code 6
```

이 첫 실패는 `CFMissileGuideComp.cpp`와 신규 `CFMissileGuideStateTests.cpp` compile 뒤 unrelated `CFVehicleVisualComp.cpp` standalone compile에서 기존 AActor include 의존성이 노출된 것이다.
새 `.cpp` 추가로 Adaptive Unity partition이 달라지며 latent IWYU 의존성이 드러난 것으로 분류했다.

Build blocker correction:

```text
UE/Source/CarFight_Re/Private/CFVehicleVisualComp.cpp v1.0.2
- GameFramework/Actor.h 명시 include 1건
- Runtime logic 변경 0
- Blueprint/Public API 변경 0
- Product Asset 변경 0
```

교정 후 공식 Build:

```text
Job 9d495eda12cc42958cf19ce62d3d9ed7
Preset carfight.editor.development
Engine D:\UnrealEngine_Source
Target CarFight_ReEditor Win64 Development
Adaptive non-unity:
- CFVehicleVisualComp.cpp 독립 compile PASS
- CarFight module compile PASS
- UnrealEditor-CarFight_Re.lib link PASS
- UnrealEditor-CarFight_Re.dll link PASS
Result Succeeded
Exit Code 0
```

호환 경계:

```text
- LegacySingleGate default path 변경 없음
- 기존 SeekerFieldOfViewDeg / LockBreakAngleDeg 유지
- 기존 CFMissileRuntimeTests.cpp v1.3.0 수정 0
- Fire→Pool→Launch Target Snapshot 계약 변경 0
- Content Asset / Blueprint / Map 저장 변경 0
- Stateful Technical PASS 범위 = DirectActorKinematics
- SampledPositionEstimate observer/estimator Runtime = MG-P0-10 Pending
- Stateful/Sampled Product DA 적용 = MG-P0-10 Technical PASS 전 금지
```

최종 구현검수:

```text
P0 = 0
P1 = 0
P2 = 0
Result = PASS
```

다음 exact Gate는 `MG-P0-10 Observation + Velocity Estimator`다.

### 16.12 2026-09-07 MG-P0-10 Observation + Velocity Estimator — TECHNICAL PASS

`MG-P0-09 Stateful Seeker Runtime` 위에 관측 성능을 독립 축으로 추가해 `SampledPositionEstimate`가 실제 Target Actor의 완전한 운동 정답을 매 Guidance Tick 소비하지 않도록 구현했다.

Product Source:

```text
UE/Source/CarFight_Re/Public/CFMissileGuideComp.h v1.2.0
UE/Source/CarFight_Re/Private/CFMissileGuideComp.cpp v1.2.0
```

Automation Source:

```text
UE/Source/CarFight_Re/Private/CFMissileGuideStateTests.cpp v1.1.0
CarFight.Missile.MG_P0_10.ObservationEstimatorContract
```

Sampled observation 흐름:

```text
Launch Target Snapshot
→ Target 위치 seed 1회
→ Sampled 초기 VelocityEstimate = Zero
→ ObservationAgeSeconds 누적
→ TargetObservationIntervalSeconds 미도달
   → LastObservedTargetLocation 유지
   → EstimatedTargetLocation 외삽
   → Actor 현재 위치/Velocity로 숨은 보정하지 않음
→ interval 도달
   → 현재 Target 위치를 Guidance Tick당 최대 1회 sample
   → RawVelocity = (NewPosition - LastObservedPosition) / ObservationAgeSeconds
   → TargetVelocityEstimateResponseTimeSeconds로 결정론적 filter
   → LastObserved / Estimated를 새 sample로 commit
   → ObservationAgeSeconds = 0
```

Actor truth 경계:

```text
DirectActorKinematics
- Target Actor 위치 직접 read
- Target Actor GetVelocity 직접 read

SampledPositionEstimate
- 발사 seed와 관측 interval 도달 시점에만 Target Actor 위치 read
- TargetActor->GetVelocity() Guidance 정답 소비 0
- 관측 사이 실제 Target 위치 재조회 0
```

Stateful + Sampled 단일 Sensor Truth:

```text
EstimatedTargetLocation
FilteredTargetVelocityEstimate
→ Acquisition / Tracking angle
→ LostGrace
→ Reacquisition
→ Overshoot
→ PN Guidance Input
```

Guidance Window:

```text
Window closed
→ ObservationAgeSeconds만 누적
→ Actor sample 0
→ Seeker state transition 0

Window open
→ 누적 ObservationAge가 interval을 이미 충족했으면 현재 위치 sample 최대 1회
→ 실제 누적 elapsed time으로 velocity estimate 계산
```

HoldLastKnownPoint:

```text
Sampled LostFinal 진입 순간
→ 현재 Estimated Sensor Truth 위치를 Hold point로 1회 Freeze
→ 이후 Target Actor 이동을 다시 읽지 않음
→ estimator 외삽도 Hold point에 적용하지 않음
```

Focused Automation:

```text
Process b786eb16bdcc46bd947e274b9bf03e53
CarFight.Missile.MG_P0_10.ObservationEstimatorContract
Engine Exit Code 0
Success 1 / Failure 0
```

검증 시나리오:

```text
- 발사 seed에서 Sampled VelocityEstimate Zero
- Target Actor가 -Y GetVelocity를 보고해도 +Y 위치 차분은 +Y velocity estimate 생성
- 0.10s observation interval 전 LastObserved 위치 고정
- interval 도달 시 현재 위치 sample 1회
- 관측 사이 EstimatedTargetLocation 외삽
- 실제 Target을 관측 사이 크게 이동해도 hidden truth를 즉시 따라가지 않음
- 방향 반전 직후 estimator response time 때문에 결정론적 추정 지연 존재
- 0.35s 저 FPS/hitch에서도 한 Guidance Tick에 현재 위치 sample 1회, 실제 elapsed time 사용
- Guidance Window closed 동안 sample 없이 ObservationAge만 누적
- Window open 첫 step에서 overdue sample 최대 1회
- 0.05s / 0.20s observation interval이 같은 Target motion에서 실제 반응 시점 차이를 생성
- LegacySingleGate + SampledPositionEstimate 조합 동작
- Sampled HoldLastKnownPoint fixed-point freeze
- Sampled observer Pool Reset
```

Affected Missile Regression:

```text
Process 9210ce22552840c5bdcf9f7a8af55d81
Filter CarFight.Missile
Engine Exit Code 0
Success 5 / Failure 0

PASS CarFight.Missile.MG_P0_00.FoundationContract
PASS CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
PASS CarFight.Missile.MG_P0_08.ConfigFoundation
PASS CarFight.Missile.MG_P0_09.StatefulSeekerContract
PASS CarFight.Missile.MG_P0_10.ObservationEstimatorContract
```

Official Editor Build:

```text
Job b6c4cd46bba941958abd5c45de009dc0
Preset carfight.editor.development
Engine D:\UnrealEngine_Source
Target CarFight_ReEditor Win64 Development
UHT 1 generated file
CFMissileGuideComp.cpp independent compile PASS
CFMissileGuideStateTests.cpp independent compile PASS
CFMissileRuntimeTests.cpp independent compile PASS
UnrealEditor-CarFight_Re.lib link PASS
UnrealEditor-CarFight_Re.dll link PASS
Result Succeeded
Exit Code 0
```

호환/보호 경계:

```text
- LegacySingleGate + DirectActorKinematics default behavior 유지
- UCFMissileGuideComp public API 변경 0
- CFMissileGuideMath 변경 0
- Projectile Launch/Actor/Data public integration 변경 0
- 기존 CFMissileRuntimeTests.cpp v1.3.0 수정 0
- Content Asset / Blueprint / Map 저장 변경 0
- 기존 DA_Missile_DirectTest를 Sampled로 전환하지 않음
- 랜덤 sensor noise 추가 0
- HitChance / MissProbability 추가 0
```

최종 구현검수:

```text
P0 = 0
P1 = 0
P2 = 0
Result = PASS
```

`CF-TC-027 USER Feel`은 아직 재승인하지 않는다. MG-P0-11에서 성능 variant 검증 행렬을 닫은 뒤 MG-P0-12에서 실제 저성능/기준/고성능 체감을 USER가 다시 판정한다.

다음 exact Gate는 `MG-P0-11 Guidance Variant Verification Matrix`다.

### 16.13 2026-09-07 MG-P0-11 Guidance Variant Verification Matrix — TECHNICAL PASS

MG-P0-08~10에서 만든 데이터 기반 성능축이 실제 동일 Runtime에서 서로 다른 결과를 만드는지, Product Runtime을 추가 수정하지 않고 Automation 행렬로 검증했다.

변경 Source:

```text
UE/Source/CarFight_Re/Private/CFMissileGuideStateTests.cpp v1.2.0
```

Product Runtime 변경:

```text
0
```

저성능 transient fixture:

```text
TargetObservationIntervalSeconds = 0.15
TargetVelocityEstimateResponseTimeSeconds = 0.35
GuidanceResponseTimeSeconds = 0.30
MaximumTurnRateDegPerSec = 25
MaximumLateralAccelerationCmPerSecSq = 1200
AcquisitionConeHalfAngleDeg = 25
TrackingConeHalfAngleDeg = 35
TargetLostGraceTimeSeconds = 0.10
ReacquisitionMode = None
```

기준형 transient fixture:

```text
TargetObservationIntervalSeconds = 0.08
TargetVelocityEstimateResponseTimeSeconds = 0.20
GuidanceResponseTimeSeconds = 0.20
MaximumTurnRateDegPerSec = 50
MaximumLateralAccelerationCmPerSecSq = 6000
AcquisitionConeHalfAngleDeg = 35
TrackingConeHalfAngleDeg = 65
TargetLostGraceTimeSeconds = 0.40
ReacquisitionMode = ForwardCone
ReacquisitionConeHalfAngleDeg = 45
ReacquisitionTimeSeconds = 0.30
```

고성능 transient fixture:

```text
TargetObservationIntervalSeconds = 0.03
TargetVelocityEstimateResponseTimeSeconds = 0.10
GuidanceResponseTimeSeconds = 0.10
MaximumTurnRateDegPerSec = 80
MaximumLateralAccelerationCmPerSecSq = 15000
AcquisitionConeHalfAngleDeg = 50
TrackingConeHalfAngleDeg = 85
TargetLostGraceTimeSeconds = 0.80
ReacquisitionMode = ForwardCone
ReacquisitionConeHalfAngleDeg = 65
ReacquisitionTimeSeconds = 0.60
```

중요한 구조 경계:

```text
- Low / Normal / High는 Automation fixture 이름일 뿐 Product enum이 아니다.
- 동일 UCFMissileGuideComp Runtime이 세 config를 그대로 소비한다.
- HitChance / MissProbability 없음.
- 랜덤 sensor noise 없음.
- 저장 DataAsset 변경 없음.
```

Verification Matrix:

```text
1. 동일 20deg 정지 오프축 Target
   → Low / Normal / High 모두 Tracking 획득
   → Applied Lateral Acceleration: Low < Normal < High
   → Applied Turn Rate: Low < Normal < High

2. Physical Limits
   → 각 Variant Applied Turn Rate <= 자신의 MaximumTurnRate
   → 각 Variant Applied Lateral Acceleration <= 자신의 MaximumLateralAcceleration

3. 동일 50deg Target
   → Low: Tracking35 이탈 + grace 종료 + Reacquisition None → LostFinal / TargetLost
   → Normal: Tracking65 안이므로 Tracking 유지
   → High: Tracking85 안이므로 Tracking 유지

4. Reacquisition None vs ForwardCone
   → 20deg에서 Tracking 성립
   → 80deg로 이탈
   → Low(None): LostFinal
   → Normal(ForwardCone): Reacquiring
   → 같은 Launch Snapshot Target을 40deg로 복귀
   → Low: LostFinal 유지
   → Normal: Tracking 복원

5. Pool-style Reset
   → Low / Normal / High 모두 SeekerState Inactive
   → observation valid false
   → EstimatedTargetLocation zero
   → GuidanceTargetActor null

6. 쉬운 정면 정지 Target 실제 충돌
   → transient actual ACFProjectileActor
   → production UProjectileMovementComponent
   → teleport 없이 +X 실제 이동
   → Blocking으로 Velocity zero
   → collision volume 경계 접촉 PASS
```

정면 Blocking contact 독립 진단:

```text
첫 Focused 실행에서 contact block 자체는 이미 PASS:
Steps = 35
Contact = (871.4, 0.0, 2000.0)
Target = (1000.0, 0.0, 2000.0)
CenterDistance = 128.6cm

최종 교정 실행에서도 이 contact code/assertion은 변경하지 않았고 전체 GuidanceVariantMatrix가 PASS했다.
```

최초 Focused 실행:

```text
Process 03e81128cf974c16bb107c8ef9a1ed31
Result Fail / Exit Code 1
```

실패 분류:

```text
Test Fixture Stimulus Defect / Product Defect 아님

기존 자극:
정면 seed
→ 같은 sample에서 Target을 순간 측면 이동
→ Sampled estimator가 해당 위치 차분을 Target velocity로 정상 추정
→ PN 상대운동이 상쇄
→ 세 Variant Applied Guidance가 모두 0

이 문제는 Product가 같은 config를 구분하지 못한 것이 아니라
비교 시험이 '정지 오프축 반응'과 '순간 위치변화에서 유도되는 속도 추정'을 섞은 것이 원인이다.
```

교정:

```text
세 Variant 모두 Acquisition 가능한 동일 20deg 정지 오프축 Target으로 response 비교
→ 위치 sample 변화 artifact 제거
→ Guidance response / turn / lateral acceleration 성능 차이만 측정
→ Product Source 수정 0
```

교정 후 Focused Automation:

```text
Process 5f07656c176d43e4ba770ee3a67fbbc4
CarFight.Missile.MG_P0_11.GuidanceVariantMatrix
Success 1 / Failure 0
Engine Exit Code 0
```

Affected Missile Regression:

```text
Process 0aa46624f9364f71b226492ed19a92d7
Filter CarFight.Missile
Success 6 / Failure 0
Engine Exit Code 0

PASS CarFight.Missile.MG_P0_00.FoundationContract
PASS CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
PASS CarFight.Missile.MG_P0_08.ConfigFoundation
PASS CarFight.Missile.MG_P0_09.StatefulSeekerContract
PASS CarFight.Missile.MG_P0_10.ObservationEstimatorContract
PASS CarFight.Missile.MG_P0_11.GuidanceVariantMatrix
```

Official UE 5.8 Editor Build after final test correction:

```text
Job 0f75526616124297949ccb22438d07b8
Preset carfight.editor.development
Engine D:\UnrealEngine_Source
Target CarFight_ReEditor Win64 Development
CFMissileGuideStateTests.cpp independent compile PASS
UnrealEditor-CarFight_Re.lib link PASS
UnrealEditor-CarFight_Re.dll link PASS
Result Succeeded
Exit Code 0
```

보호 경계:

```text
- UCFMissileGuideComp Runtime 변경 0
- CFMissileGuideTypes 변경 0
- CFMissileGuideMath 변경 0
- Projectile public integration 변경 0
- 기존 CFMissileRuntimeTests.cpp v1.3.0 변경 0
- Content Asset / DataAsset / Blueprint / Map 변경 0
- 기존 DA_Missile_DirectTest 변경 0
```

최종 구현검수:

```text
P0 = 0
P1 = 0
P2 = 0
Result = PASS
```

`CF-TC-027 USER Feel`은 아직 PASS가 아니다. MG-P0-11은 **성능 Variant를 만들 수 있는 기술 기반과 차이**까지만 검증했으며, 실제 게임 화면에서 그 차이가 자연스럽게 느껴지는지는 USER가 직접 판정해야 한다.

### 16.14 2026-09-07 MG-P0-11 Final Review Correction + Re-review — PASS

MG-P0-11 완료 후 독립 최종검수에서 Technical PASS 자체와 별개로 Acceptance coverage와 다음 USER Gate projection을 다시 검토했다.

초기 최종검수:

```text
P0 = 0
P1 = 3
P2 = 0
Result = CORRECTION REQUIRED
```

P1-1 — 같은 Target motion Variant coverage 부족:

```text
기존 MG-P0-11의 Low < Normal < High 핵심 Guidance response 비교는 20deg 정지 Target이었다.
MG-P0-10에서 fast/slow observer 차이는 이미 검증했지만,
실제 Low/Normal/High fixture가 동일 moving/reversal Target을 처리할 때
관측 주기 + 속도 추정 + Guidance 반응이 함께 갈리는 직접 evidence가 없었다.
```

교정 Source:

```text
UE/Source/CarFight_Re/Private/CFMissileGuideStateTests.cpp v1.2.1
```

추가 Moving/Reversal Matrix:

```text
공통 launch seed = (10000, 0, 0)
→ 모든 Variant 0.01s 진행
→ 같은 Target을 (10000, +600, 0)으로 이동
→ 모든 Variant +0.04s 진행 / 누적 0.05s

Low observation interval 0.15s
→ LastObserved Y = seed 0 유지
→ VelocityEstimate Y = 0 유지

Normal observation interval 0.08s
→ LastObserved Y = seed 0 유지
→ VelocityEstimate Y = 0 유지

High observation interval 0.03s
→ LastObserved Y = +600 관측
→ VelocityEstimate Y > 0

다시 같은 Target을 (10000, -600, 0)으로 반전
→ 모든 Variant +0.04s 진행 / 누적 0.09s

Low
→ 아직 seed Y=0

Normal
→ -600Y 첫 관측

High
→ -600Y 재관측

FilteredTargetVelocityEstimate.Y
→ High < Normal < Low
→ 성능이 높을수록 동일 reversal을 더 빠르게 -Y 방향으로 해석

같은 reversal step의 실제 Applied Guidance
→ Low < Normal < High
→ 세 Variant 모두 자신의 Turn/Lateral 물리 상한 준수
```

교정 후 Focused:

```text
Process a0ff269df8b14c3892c5155e2a24056f
CarFight.Missile.MG_P0_11.GuidanceVariantMatrix
Success 1 / Failure 0
Engine Exit Code 0
```

교정 후 Affected Missile Regression:

```text
Process 5244de95c48d44e6b961cd0c5793392f
Filter CarFight.Missile
Success 6 / Failure 0
Engine Exit Code 0

PASS CarFight.Missile.MG_P0_00.FoundationContract
PASS CarFight.Missile.MG_P0_01_04.DirectRuntimeContract
PASS CarFight.Missile.MG_P0_08.ConfigFoundation
PASS CarFight.Missile.MG_P0_09.StatefulSeekerContract
PASS CarFight.Missile.MG_P0_10.ObservationEstimatorContract
PASS CarFight.Missile.MG_P0_11.GuidanceVariantMatrix
```

교정 Source 공식 UE 5.8 Editor Build:

```text
Job bc116df36570486984d1afcd0f5b7508
Preset carfight.editor.development
Engine D:\UnrealEngine_Source
Target CarFight_ReEditor Win64 Development
CFMissileGuideStateTests.cpp independent compile PASS
UnrealEditor-CarFight_Re.lib link PASS
UnrealEditor-CarFight_Re.dll link PASS
Result Succeeded
Exit Code 0
```

P1-2 — MG-P0-12 Ready 상태 과장:

```text
Low/Normal/High는 현재 Automation transient FCFMissileGuideConfig에만 존재한다.
실제 MissileDirectTest PIE에서 세 Variant를 선택·발사하는 persisted USER 비교 경로는 아직 없다.
따라서 MG-P0-12 USER Validation을 Ready로 표시할 수 없다.
```

현재 UE write capability 확인:

```text
- fresh managed Editor runtime Ready 확인
- DA_Missile_DirectTest referencer chain:
  DA_Missile_DirectTest
  → DA_Missile_DirectWeapon
  → EQ_Missile_DirectTest
  → DA_Missile_TestSUV
- DA_Missile_DirectTest dependency에서 실제 Projectile Blueprint / Rocket Mesh / Damage / Impact FX / Trail / Thruster 참조 확인
- DataAssetTools는 /Game/Test 하위 신규 UCFProjectileData 생성 probe 가능
- ObjectTools property write는 forward-write + readback 뒤 inverse-write로 자동 복구되는 WriteProbe 계약
- therefore persisted Low/Normal/High USER setup을 이 capability로 저장 완료할 수 없음
```

시험 중 생성한 `DA_Missile_Feel_Low`는 저장하지 않았고 AI-owned Editor를 `stopped_discarded_unsaved`로 종료해 persisted Content 변경을 남기지 않았다.

교정:

```text
MG-P0-12A USER Guidance Feel Test Setup을 새 선행 Gate로 분리
→ 기존 DA_Missile_DirectTest regression baseline 보존
→ 동일 Actor/Mesh/Collision/Damage/Flight 기준을 공유하는 Low/Normal/High ProjectileData 준비
→ MissileGuideConfig만 Variant별로 변경
→ 실제 MissileDirectTest PIE 선택/발사 경로 준비
→ persisted readback 확인
→ 그 뒤 MG-P0-12 USER Guidance Feel Validation 시작
```

P1-3 — 상세 설계 문서 stale Current projection:

```text
GuidancePerformanceDesign.md §18~19의 v0.1.9 `현재 판정`이 최신 v0.1.10 상태와 충돌했다.
해당 블록을 Historical MG-P0-10 checkpoint로 명시하고 current owner를 v0.1.11로 전진시켰다.
```

최종 교정 후 재검수:

```text
P0 = 0
P1 = 0
P2 = 0
Result = PASS
```

최종 경계:

```text
MG-P0-11 = Final Technical PASS
MG-P0-12A = Pending / Next Exact Gate
MG-P0-12 = Not Started / blocked by setup
CF-TC-027 USER Feel = NOT ACCEPTED 유지
Product Runtime correction = 0
Persisted Content mutation = 0
```

다음 exact Gate는 `MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit`이다.

---

### 16.15 2026-09-07 MG-P0-12C Guidance Law Runtime — TECHNICAL PASS

MG-P0-12B에서 확정한 Guidance Law Strategy를 Product Runtime에 연결하고, 중간검수에서 발견한 동일 activation Course Capture→PN 전환 검증 공백을 Test-only로 교정한 뒤 재검수했다.

```text
Product Runtime:
- CFMissileGuideTypes.h v1.2.0
- CFMissileGuideMath.h/.cpp v1.1.0
- CFMissileGuideComp.h/.cpp v1.3.0

Automation:
- CFMissileGuideStateTests.cpp v1.3.1
- CarFight.Missile.MG_P0_12C.GuidanceLawContract
```

확정 Runtime 계약:

```text
PurePursuit
→ LastObservedTargetLocation-only steering
→ predictive EstimatedTargetLocation과 Target velocity 선행 소비 금지

LeadPursuit
→ LastObservedTargetLocation + bounded FilteredTargetVelocityEstimate lead
→ EstimatedTargetLocation 기준의 이중 예측 금지

ProportionalNavigation
→ 기존 Estimated target state 소비 경로 유지
→ rear/non-closing에서 bounded Course Capture
→ 접근 기하가 형성되면 같은 activation에서 PN으로 복귀

공통
→ MaximumTurnRate / MaximumLateralAcceleration / GuidanceResponse / MinimumGuidanceSpeed 제한 유지
→ teleport / instant target-facing velocity replacement / forced hit 없음
```

중간검수 P1 교정:

```text
최초 보강 테스트는 기존 FCFStatefulMissileTestRig::AdvanceGuidance()가 매 step Velocity를 +X로 복원해 실제 선회 누적을 방해하는 Test Fixture Stimulus Defect가 있었다.
Product 결함으로 분류하지 않고 Product Runtime mutation 0으로 유지했다.
CFMissileGuideStateTests.cpp v1.3.1에 AdvanceGuidanceKeepingVelocity()를 추가해 같은 activation의 실제 Guidance 결과 Velocity를 유지하도록 교정했다.
이후 rear Course Capture → 실제 선회 → 접근 기하 형성 → PN 복귀를 동일 activation에서 검증했다.
```

검증 증거:

```text
Official UE 5.8 Editor Build
Job 8c30576af506485f8e811fc8eb326806
Result Succeeded / Exit Code 0
CFMissileGuideStateTests.cpp compile + CarFight lib/dll link + metadata PASS

Focused Automation
Process 62d9858360ab45caacd31924867d2604
CarFight.Missile.MG_P0_12C.GuidanceLawContract
Success 1 / Failure 0 / Missing 0 / Unexpected 0 / Exit Code 0

Affected Missile Regression
Process 43b6343b4cad4ed985698e4c6bd600ea
Filter CarFight.Missile
Success 8 / Failure 0 / Missing 0 / Unexpected 0 / Exit Code 0
```

최종 재검수:

```text
P0 = 0
P1 = 0
P2 = 0
Result = PASS
MG-P0-12C = Technical PASS / Complete
MG-P0-12D = Final Technical PASS / Complete
MG-P0-12E = Ready / Next Exact Gate
CF-TC-027 USER Feel = NOT ACCEPTED 유지
```

MG-P0-12D는 Independent Guidance Activation, free Stateful Seeker geometry, approach-armed Overshoot와 exact 180deg deterministic tie-break의 실제 Runtime 연결과 자동 검증을 완료했다.

---

## 17. 제외 범위

```text
- 실제 공력면 6-DOF 완전 시뮬레이션
- 플레어·재밍
- 근접신관
- 폭발 범위 피해
- DataLink 네트워크
- DataLinkAssisted Reacquisition Runtime
- 서버 권한 유도
- 실제 IR/Radar seeker 물리 모델
- 첫 구현의 랜덤 센서 노이즈
- 명중률 직접 확률값
- Terminal 강제 명중 보정
- Audio
```

---

## 18. Changelog

### v0.6.25 - 2026-09-08

```text
- CF-FQ-030 Post-Closure Final Audit에서 actual Source, Current Systems, FeatureQueue, ActiveWork, Plan Index와 Historical owner를 다시 교차검수했다.
- 최초 판정 P0=0 / P1=3 / P2=0. P1-1은 Current MissileGuidance가 Direct Flight의 실제 Ejection 상태를 생략하고 Impact/Expired를 현재 FlightComp 상태 전이처럼 보이게 기록한 사실 오차였다.
- P1-2는 Tick 그림이 Motor → Flight 상대 순서까지 고정된 것처럼 보였으나 실제 Source는 Motor와 Flight를 각각 Guide prerequisite로 두고 둘 사이의 상대 prerequisite는 두지 않는다는 lifecycle 표현 오차였다.
- P1-3은 Plan Index §2 본문에 v0.6.23 / Ready / P0 Closure Review가 Current 문장으로 남은 stale route였다.
- Current System을 MissileGuidance.md v1.0.1로 교정하고 Plan Index stale current route를 제거했다. Angled/Vertical 다중 사출 연출도 구조상 조합 가능성과 CF-FQ-029 USER PIE Pending을 명확히 분리했다.
- 교정 후 Source↔Systems↔FeatureQueue↔ActiveWork↔Plan/Archive projection 재검수 결과 P0=0 / P1=0 / P2=0 PASS다.
- Product Source/Asset mutation은 0이며 기존 Official Build, Authoring 1/1, Focused 1/1, Missile 10/10, AssetDump 3/3와 USER Feel evidence는 failure/change trigger가 없어 재실행하지 않았다.
```

### v0.6.24 - 2026-09-08

```text
- CF-FQ-030 P0 Closure + Current System Promotion Review를 수행해 기능 blocker 0을 확인하고 Direct P0를 Complete로 닫았다.
- Current System owner를 main_game `Document/Systems/Combat/MissileGuidance.md v1.0.0`으로 승격하고 공통 Projectile Actor 통합 경계는 `Projectile.md v1.9.0`이 소유하도록 분리했다.
- CF-TC-027은 Technical + USER Guidance Feel Complete / PASS다. Low first REJECT는 Historical evidence로 보존하고 corrected Low/Normal/High USER acceptance를 Current 완료 판정으로 유지한다.
- LaserPoint·DataLink/InertialPoint 실제 Runtime, Angled/Vertical/Loft/TopAttack, 실제 Expire lifecycle과 장비별 Multi-Muzzle/Salvo 연출은 P0 비차단 후속 범위로 분리했다.
- MG-P0-12E fresh Build/Authoring/Focused/Missile 10/10과 기존 Guidance Preset AssetDump 3/3은 Source/Asset failure/change evidence가 없어 반복하지 않았다.
- 대표 Plan은 `Historical + Retained Path`로 전환하며 physical move는 수행하지 않는다.
```

### v0.6.23 - 2026-09-08

```text
- USER가 실제 MissileDirectTest PIE에서 persisted Guidance Preset 기반 Low / Normal / High를 모두 직접 확인했고 최종 체감 판정을 "얼추 PASS"로 승인했다.
- 이 판정은 완벽한 최종 밸런스 고정이 아니라 현재 P0 USER Feel acceptance다. 이후 숫자 미세조정은 Guidance Preset DA에서 비차단으로 계속 가능하다.
- MG-P0-12 USER Guidance Feel Validation을 USER ACCEPTED / Complete로 전진했다. 첫 Low REJECT는 Historical evidence로 보존하고 corrected Low 재검증과 Normal/High USER 확인을 모두 PASS로 닫았다.
- CF-TC-027 Technical PASS와 USER Feel ACCEPTED가 모두 성립하므로 CF-TC-027은 현재 검증 범위에서 Complete / PASS다.
- MG-P0-12E Mid-review P0/P1/P2 0 PASS, fresh Build/Authoring/Focused/Missile 10/10과 기존 Guidance Preset AssetDump 3/3은 새 failure/change evidence 없이 반복하지 않았다.
- CF-FQ-030 전체 Done은 아직 선언하지 않는다. MG-P0-05 Laser Point는 현재 Direct P0 제외 범위이고 MG-P0-06 Angled/Vertical/Loft/TopAttack은 후속 범위로 남아 있으므로, 다음 exact Gate는 `CF-FQ-030 P0 Closure + Current System Promotion Review`다.
```

### v0.6.22 - 2026-09-08

```text
- MG-P0-12E Post-Implementation Mid-review에서 P0=0 / P1=3 / P2=1을 확인했다. P1은 load-time 자산 이름 seed에 따른 사용자 튜닝값 보존 위험, Product 모듈의 Low/Normal/High authoring branch, Current/Historical 문서 projection 충돌 3건이었고 P2는 실제 persisted idempotence round-trip 증거 공백 1건이었다.
- `UCFMissileGuidePresetData`를 v1.2.0 passive data container로 교정해 `PostInitProperties`와 자산 이름 기반 seed를 제거했다. Product 모듈은 PresetId/표시 이름/설명/MissileGuideConfig 저장만 소유하며 Asset load-time에 사용자 튜닝값을 재주입하지 않는다.
- Low/Normal/High 신규 seed 책임을 `CarFight_ReEditor`의 `CFMissileFeelCommands.cpp v1.5.0`으로 이동했다. `EnsureMissileFeelPresetAsset`은 existing Asset이면 seed/save/dirty 없이 그대로 반환하고, 실제 신규 생성 직후에만 Editor authoring seed를 적용한다.
- Product `CarFight_Re` Source static audit에서 `MissileFeel_Low / Normal / High`와 `EMissileFeelTestVariant` 실행 흔적 0을 확인했다. Guidance Runtime은 계속 GuidanceLaw/Activation/Seeker/관측/물리 성능 데이터만 소비한다.
- `CarFight.Authoring.MissileFeel.EnsurePresets`에 실제 persisted idempotence round-trip을 추가했다. USER 전용 Low/Normal/High 3개는 수정하지 않고 별도 임시 Preset에 USER sentinel과 C++ 기본값과 같은 값을 저장 → 같은 object path에 다른 Variant seed로 Ensure 재실행 → existing Asset/clean package/USER 값 보존 → package unload → 디스크 재로드 → 동일값 보존 → 임시 파일 제거를 검증한다.
- 첫 신규 idempotence 실행 process `b572202cef824120bbb920ae51c8d0c3`은 값 보존이 아니라 테스트 종료의 임시 `.uasset` 물리 정리 assertion에서만 FAIL했다. Product/authoring 값 보존 결함으로 확대하지 않고 Test Fixture Cleanup Defect로 분류했으며 남은 exact 임시 Asset 1개는 server-owned Trash로 격리했다.
- 정리 경로를 UnrealEd `UPackageTools::UnloadPackages` → 디스크 `LoadPackage` → readback → 재-unload → 파일 삭제로 교정했다. 교정 후 Authoring process `e2e62abaac2148e8806850a137598d03` = `CarFight.Authoring.MissileFeel.EnsurePresets` 1/1 PASS / Failure 0 / Engine Exit 0.
- Fresh MG-P0-12E focused process `f430b8d395fc4bb18db238e9eff8766e` = `CarFight.Missile.MG_P0_12E.FeelSetupRewire` 1/1 PASS / Failure 0 / Engine Exit 0.
- Fresh affected process `2f6b1341bc5c42f988fa13a61d5e8997` = `CarFight.Missile` 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Engine Exit 0.
- Final official UE 5.8 Editor Build `823b88eda5fc42b4b83f41135aa2df0f` = Succeeded / Exit 0. Final `CFMissileFeelCommands.cpp v1.5.0` compile + `UnrealEditor-CarFight_ReEditor.dll` link PASS.
- 실제 Low/Normal/High persisted Preset 3개와 DirectTest persisted baseline은 이번 correction에서 수정하지 않았다. 따라서 v0.6.20 AssetDump 3/3 persisted readback evidence는 failure/change evidence 없이 보존하고 재실행하지 않았다.
- `MissileGuidancePlan / GuidancePerformanceDesign / FeatureQueue / ActiveWork`의 Current projection을 Mid-review closure 기준으로 정렬하고 과거 Low 재검증 Ready 문구는 날짜·버전이 있는 Historical checkpoint로만 보존했다.
- 최종 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. MG-P0-12E Mid-review는 Closed이며 다음 exact Gate는 `MG-P0-12 USER Guidance Feel Validation — Corrected Low DA Revalidation`이다. `CF-TC-027 USER Feel`은 USER 재검증 전까지 NOT ACCEPTED를 유지한다.
```

### v0.6.21 - 2026-09-08

```text
- USER 요청에 따라 새 세션에서 바로 Corrected Low USER PIE로 넘어가지 않고, 방금 완료한 MG-P0-12E Guidance Preset DA Rewire의 Post-Implementation Mid-review를 먼저 수행하도록 재개 Gate를 변경했다.
- v0.6.20의 Technical PASS, persisted Feel Preset 3/3 AssetDump PASS, Authoring 1/1, Focused 1/1, 전체 Missile 10/10, Official Build PASS evidence는 그대로 보존한다.
- 중간검수 목적은 코드/자산 소유권, Preset authoring idempotence, 사용자 튜닝값 보존, Runtime transient overlay 경계, DirectTest baseline 비변경, Product quality enum/branch 0, 문서 Current/Historical 경계를 독립 재검토하는 것이다.
- 새 failure/change evidence가 없으면 기존 Build/Automation/AssetDump를 반복하지 않는다.
- 중간검수에서 P1/P0가 나오면 교정 + 재검수 후 USER PIE로 진행하고, P0/P1 0이면 바로 Corrected Low DA USER Revalidation로 전진한다.
- CF-TC-027 USER Feel은 계속 NOT ACCEPTED다.
```

### v0.6.20 - 2026-09-08

```text
- USER 체감 반복 튜닝에서 C++ 숫자 하드코딩과 재빌드를 제거하기 위해 MG-P0-12E 시험 설정을 persisted Guidance Preset DA 기반으로 전환했다.
- 신규 UCFMissileGuidePresetData v1.1.0은 PresetId / 표시 이름 / 설명 / FCFMissileGuideConfig만 소유한다. Projectile 속도·피해·Collision·FX·ActorClass는 소유하지 않고 기존 DirectTest transient 복제 체인을 보존한다.
- 저장 위치는 /Game/Test/CarFight/Missile/FeelPresets이며 DA_MissileFeel_Low / Normal / High 3개를 생성했다. Low=PurePursuit, Normal=LeadPursuit, High=ProportionalNavigation 구조 계약은 유지한다.
- CFMissileFeelCommands.cpp v1.4.0은 Low/Normal/High float builder를 제거하고 저장 Guidance Preset을 로드해 transient ProjectileData의 MissileGuideConfig에만 overlay한다. 따라서 앞으로 수치 튜닝만 변경할 때 C++ rebuild가 필요하지 않다.
- WriteProbe는 rollback 가능한 시험 mutation만 소유하고 persisted Save를 허용하지 않음을 확인했으며, 이를 우회하지 않았다. 최초 Preset 생성은 명시 실행형 CarFight.Authoring.MissileFeel.EnsurePresets가 UE SavePackage 정식 경로를 사용하고 기존 Preset이 있으면 재사용해 사용자 튜닝값을 덮어쓰지 않는다.
- Official UE 5.8 Editor Build f0e60b97d7844f46b891eb7e18d13709 = Succeeded / Exit 0.
- Authoring process bc4f634ca24c464fb8e823af3b0d59b3 = CarFight.Authoring.MissileFeel.EnsurePresets 1/1 PASS / Exit 0.
- AssetDump dataset adset_v1_af183e2d0c12bdb5db3a0f1c5b775ee9.03130ec4fd3c9b7aaa8b7d0a = Preset 3/3 persisted readback PASS. Low/Normal/High PresetId와 Pure/Lead/PN Guidance Law 및 현재 핵심 수치를 확인했다.
- Focused process af64ddaf4d224bcf9161411e1ed4b0d4 = MG_P0_12E.FeelSetupRewire 1/1 PASS.
- 전체 Missile process 5bf5ae819c96429fb5b3e56c8289c398 = CarFight.Missile 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 최종 코드·설계 재검수 P0=0 / P1=0 / P2=0 PASS. Product Low/Normal/High enum/branch는 0이고 DirectTest baseline persisted 자산은 변경하지 않았다.
- USER Feel 상태는 전진시키지 않는다. Low first-pass REJECTED와 CF-TC-027 NOT ACCEPTED를 유지하며 다음 exact Gate는 MG-P0-12 USER Guidance Feel Validation — Corrected Low DA Revalidation이다.
- GuidancePerformanceDesign current owner를 v0.1.21로 갱신한다.
```

### v0.6.19 - 2026-09-08

```text
- MG-P0-12 USER Guidance Feel Validation의 첫 Low PIE를 USER가 REJECT했다. 이동 Target 앞을 잘 선행 조준한 발사에서도 Low가 초기 탄도를 이르게 버리고 Target 현재/최근 위치 쪽으로 꺾은 뒤 Target 근처에서 다시 크게 수정해, 거의 피하는 것처럼 Miss하는 체감이 확인됐다.
- 이를 단순 명중률 부족으로 보지 않고 "잘 만든 USER 초기 탄도를 Low Guidance가 즉시 망가뜨리지 않아야 한다"를 Low acceptance에 추가했다.
- Product PurePursuit/Guide Runtime은 변경하지 않고 Editor-only CFMissileFeelCommands.cpp를 v1.2.1로 교정했다.
- corrected Low = PurePursuit + Independent 0.25s AND 500cm + Observation 0.08s + VelocityEstimateResponse 0.25s + GuidanceResponse 0.18s + MaximumTurnRate 35deg/s + MaximumLateralAcceleration 2000cm/s^2 + Acquisition 45deg + Tracking 60deg + Grace 0.20s + Reacquisition None다. LeadTime/MaxLead는 0으로 유지한다.
- 목적은 초기 0.25s/500cm 동안 USER가 만든 launch solution을 보존하고, 이후에도 PurePursuit LastObserved-only를 유지하면서 과도한 stale/late correction 체감만 완화하는 것이다. 저가형에 predictive lead나 PN을 되돌리지 않는다.
- Official UE 5.8 Editor Build 7136ad4764b24dde9583ec9771e61b94 = Succeeded / Exit 0. CFMissileFeelCommands.cpp compile + CarFight_ReEditor DLL link PASS.
- Focused process f81f55fadfcf48cebbe6b04b91056c4a = MG_P0_12E.FeelSetupRewire 1/1 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 전체 Missile process 7c93d6f7f52f47f486fe90cefa7d3633 = CarFight.Missile 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- Persisted DirectTest Content/DataAsset/Blueprint/Map mutation은 0이고 Product Low/Normal/High enum/branch도 0이다.
- 현재 Editor/PIE는 v1.2.1 build 전 DLL lifetime이므로 새 Low USER 재시험 증거로 사용할 수 없다. fresh Editor lifetime에서 corrected Low를 다시 적용해 USER 판정한다.
- MG-P0-12E Technical PASS는 유지한다. MG-P0-12 USER Guidance Feel Validation은 Low First Pass REJECTED / Corrected Low Revalidation Ready이며 CF-TC-027 USER Feel은 NOT ACCEPTED를 유지한다.
- GuidancePerformanceDesign current owner를 v0.1.20으로 갱신한다.
```

### v0.6.18 - 2026-09-08

```text
- MG-P0-12E USER Feel Test Setup Rewire를 Technical PASS / Complete로 닫고 다음 exact Gate를 MG-P0-12 USER Guidance Feel Validation으로 전환했다.
- Editor-only CFMissileFeelCommands.cpp를 v1.2.0으로 갱신했다. 저장 DirectTest 자산을 복제한 transient Runtime override 경로는 유지하고 Guidance Law / Activation / Seeker geometry / 기동 축만 재배선했다.
- Low = PurePursuit + FollowFlightGuidanceWindow + 0.15s 관측 + MaximumTurnRate 25deg/s + Acquisition 25deg / Tracking 35deg / Reacquisition None이다.
- Normal = LeadPursuit + Independent 0.06s AND 100cm + LeadTime 0.18s / MaxLead 900cm + Acquisition 75deg / Tracking 90deg / Reacquisition 120deg + MaximumTurnRate 50deg/s다.
- High = ProportionalNavigation + Independent 0/0 + Acquisition/Tracking/Reacquisition 180deg + GuidanceResponse 0.05s + MaximumTurnRate 140deg/s + MaximumLateralAcceleration 15000cm/s^2다.
- 세 fixture는 모두 Stateful + SampledPositionEstimate를 유지하지만 Guidance Law 세 종류가 실제로 서로 다르다. Product Runtime에 Low/Normal/High enum/branch를 추가하지 않았다.
- 기존 CarFight.Missile.MG_P0_12A.FeelCommandSetup은 네 console command와 Editor-only transient 시험 기반 보존만 검증하도록 유지하고, 신규 CarFight.Missile.MG_P0_12E.FeelSetupRewire가 exact Law/Activation/Seeker fixture 행렬을 소유한다.
- Official UE 5.8 Editor Build 108388074e0d40619e969026a596713f = Succeeded / Exit 0. CFMissileFeelCommands.cpp compile + CarFight_ReEditor lib/dll link PASS.
- Focused process 6f14a6adbb524b0db05ce1e39cd5a5d5 = MG_P0_12E.FeelSetupRewire 1/1 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 전체 Missile process 852026f5f3914c1c92220443e059d0ba = CarFight.Missile 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- Persisted DirectTest DataAsset/Blueprint/Map mutation은 0이고 Baseline command는 저장 EQ_Missile_DirectTest를 그대로 재적용한다.
- MG-P0-12E Technical PASS를 USER Feel PASS로 확대하지 않는다. 다음 단계에서 USER가 같은 시험 환경에서 Low/Normal/High의 실제 궤적·반응·rear-aspect 체감을 직접 판정해야 하며 CF-TC-027은 NOT ACCEPTED를 유지한다.
- GuidancePerformanceDesign current owner를 v0.1.19로 갱신한다.
```

### v0.6.17 - 2026-09-07

```text
- v0.6.16 closure 후 최종 재검수에서 P1 1건을 발견했다. exact 180deg rear Launch Right tie-break가 PN Course Capture에만 적용돼 PurePursuit / LeadPursuit가 정확한 정후방 Target에서 lateral steering 0으로 남을 수 있는 law coverage gap이었다.
- Product를 CFMissileGuideComp.h/.cpp v1.4.1로 교정해 BuildBoundedPursuitCommandWithRearTieBreak() 공통 helper를 추가했다. PurePursuit / LeadPursuit / PN Course Capture가 모두 같은 deterministic Launch Right tie-break를 사용한다.
- tie-break는 실제 Target Snapshot과 GuidanceAimPoint를 바꾸지 않고 계산용 Pursuit 입력에 0.001 Launch Right steering bias만 추가한다. 180deg 방향오차 크기와 기존 turn-rate / lateral-acceleration / response 물리 제한은 유지하며 random, 위치 순간이동, Velocity 강제 덮어쓰기, 강제 명중은 추가하지 않는다.
- CFMissileGuideStateTests.cpp v1.4.1에 PurePursuit / LeadPursuit exact-rear 직접 회귀를 추가했다. Tracking, Launch Right +Y 실제 선회, SeekerAngle 180deg, 사실상 180deg 요청회전량, 물리 상한, 실제 rear Debug AimPoint 보존을 검증한다.
- 교정 후 Official UE 5.8 Editor Build 0370b20c40bf45f4a5103d9ef8c807ce = Succeeded / Exit 0.
- 교정 후 Focused process 9a601179fb3a407cb8002ee832c266f0 = MG_P0_12D.GuidanceActivationRearAspectContract 1/1 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 교정 후 전체 Missile process d0b9c2d62f254950bd3af6dabf39dde7 = CarFight.Missile 9/9 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 마지막 Source 변경은 CFMissileGuideStateTests.cpp의 테스트 함수 설명 주석을 v1.4.1 의미로 맞춘 comment-only correction이며 동작 변경은 없다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. v0.6.16의 v1.4.0 Build/Automation evidence는 pre-correction Historical checkpoint로 보존하고 Current 최종 증거는 v1.4.1 fresh Build/Automation으로 대체한다.
- Persisted Content/DataAsset/Blueprint/Map mutation은 0이며 MG-P0-12A Technical Setup과 DirectTest baseline을 보존한다.
- MG-P0-12D = Final Technical PASS / Complete, 다음 exact Gate = MG-P0-12E USER Feel Test Setup Rewire, CF-TC-027 USER Feel = NOT ACCEPTED를 유지한다.
- GuidancePerformanceDesign current owner를 v0.1.18로 갱신한다.
- 교정 후 재검수에서 §1.5의 과거 `현재 Source` / `후속 설계` 현재형 표현을 P2 stale projection으로 확인해, 당시 설계 경위와 MG-P0-12D 현재 구현 완료 상태를 명확히 구분하도록 문구를 교정했다.
```

### v0.6.16 - 2026-09-07

```text
- MG-P0-12D Guidance Activation + Rear Aspect Runtime을 Final Technical PASS / Complete로 닫고 다음 exact Gate를 MG-P0-12E USER Feel Test Setup Rewire로 전환했다.
- FollowFlightGuidanceWindow 기본 호환과 Independent Guidance Activation을 구현했다. Independent는 FlightSnapshot elapsed time / release distance를 AND로 판정하고 한 번 만족하면 activation 동안 latch하며 ResetMissileGuidance에서 초기화한다.
- Stateful Acquisition / Tracking / Reacquisition 반각의 기존 Tracking Min coupling을 제거하고 각각 독립적인 0~180deg clamp를 적용했다. LegacySingleGate 의미는 보존한다.
- Stateful approach-armed Overshoot를 구현해 실제 접근이 성립하기 전 rear Course Capture/U-turn의 거리 증가를 Overshoot로 종결하지 않으며, 실제 접근 후 거리 증가 + non-closing에서 정상 Overshoot를 유지한다.
- exact 180deg rear target은 LaunchContext.LaunchTransform Right Vector를 deterministic tie-break로 사용하며 기존 turn-rate / lateral-acceleration 물리 한계 안에서 선회한다.
- Guide Snapshot에 GuidanceActivationMode / bGuidanceActivationSatisfied / bOvershootArmed를 append-only로 연결하고 activation-local Pool Reset을 검증했다.
- Product Source: CFMissileGuideTypes.h v1.3.0 / CFMissileGuideComp.h/.cpp v1.4.0. Affected tests: CFMissileFoundationTests.cpp v1.3.0 / CFMissileGuideStateTests.cpp v1.4.0.
- Official UE 5.8 Editor Build 16a2f3c957414cb29f18cb812bf4e807 = Succeeded / Exit 0.
- Focused process 723d04a116c54944973be5a6c498de28 = MG_P0_12D.GuidanceActivationRearAspectContract 1/1 PASS / Failure 0 / Exit 0.
- 전체 Missile process 66d2fa64d1314308bd8cc1ea637cdeca = 9/9 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 최종 코드·계약 재검수 P0=0 / P1=0 / P2=0 PASS. Persisted Content/DataAsset/Blueprint/Map mutation은 0이며 MG-P0-12A Technical Setup과 DirectTest regression baseline을 보존했다.
- CF-TC-027 USER Feel은 실제 USER 재검증 전까지 NOT ACCEPTED를 유지한다.
- GuidancePerformanceDesign current owner를 v0.1.17로 갱신한다.
```

### v0.6.15 - 2026-09-07

```text
- MG-P0-12C Guidance Law Runtime을 Technical PASS / Complete로 닫고 다음 exact Gate를 MG-P0-12D Guidance Activation + Rear Aspect Runtime으로 전환했다.
- PurePursuit LastObserved-only, LeadPursuit bounded lead, 기존 PN 호환 경로와 rear/non-closing Course Capture Runtime을 구현·검증했다.
- 중간검수 P1인 동일 activation Course Capture→PN 전환 검증 공백을 CFMissileGuideStateTests.cpp v1.3.1 Test-only 교정으로 보강했다. 기존 helper가 매 step +X Velocity를 복원하던 Test Fixture Stimulus Defect를 Product 결함과 분리했다.
- Official Build 8c30576af506485f8e811fc8eb326806 = Succeeded / Exit 0.
- Focused process 62d9858360ab45caacd31924867d2604 = MG_P0_12C.GuidanceLawContract 1/1 PASS / Exit 0.
- 전체 Missile process 43b6343b4cad4ed985698e4c6bd600ea = 8/8 PASS / Failure 0 / Missing 0 / Unexpected 0 / Exit 0.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. Product Runtime correction은 0이며 CF-TC-027 USER Feel은 미승인 상태를 유지한다.
- GuidancePerformanceDesign current owner를 v0.1.16으로 갱신한다.
```

### v0.6.14 - 2026-09-07

```text
- MG-P0-12B 최초 설계검수 P0=2 / P1=5 / P2=3을 GuidancePerformanceDesign v0.1.15로 교정하고 대표 Plan에 동기화했다.
- rear/non-closing PN에서 ClosingSpeed=0으로 command가 0이 되는 문제를 bounded Course Capture로 교정했다. Target 방향으로 물리 제한형 선회를 먼저 수행하고 ForwardClosingVelocity>0 접근 기하가 성립하면 PN으로 전환한다.
- Stateful Overshoot는 bOvershootArmed=false로 시작하고 실제 거리 감소 + ForwardClosingVelocity>0 접근이 한 번 성립한 뒤에만 arm한다. rear-aspect 초기 U-turn을 Overshoot로 종결하지 않으며 LegacySingleGate의 기존 Overshoot는 보존한다.
- GuidanceActivationMode의 호환 이름을 AfterFlightClearance에서 FollowFlightGuidanceWindow로 교정해 실제 UCFMissileFlightComp::IsGuidanceWindowOpen() 의미와 일치시켰다.
- Independent activation은 FlightSnapshot elapsed time / release distance의 AND 조건을 사용하고 한 번 만족하면 activation 동안 latch하며 ResetMissileGuidance에서만 초기화한다. FlightComp는 측정값/window, GuideComp는 판정/latch를 소유한다.
- Sampled Sensor State의 소비를 Law별로 분리했다. PurePursuit=LastObserved, LeadPursuit=LastObserved+bounded filtered velocity lead, PN=Estimated state를 사용한다.
- LeadPursuit exact formula와 DataAsset EditCondition/finite clamp를 확정하고 EstimatedTargetLocation을 기준점으로 다시 더하는 이중 예측을 금지했다.
- Stateful Acquisition/Tracking/Reacquisition 반각은 각각 독립 0~180deg clamp로 확정해 current Min coupling 제거를 구현 요구로 고정했다.
- 정확히 180deg rear target의 회전 방향 특이점은 LaunchContext.LaunchTransform Right Vector deterministic tie-break로 고정하고 random 선택을 금지했다.
- Snapshot append-only 진단값 GuidanceLaw / GuidanceActivationMode / bGuidanceActivationSatisfied / GuidanceAimPoint / bCourseCaptureActive / bOvershootArmed와 activation-local Pool Reset 요구를 확정했다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. Product Source/Content mutation은 0이며 MG-P0-12B는 Complete, 다음 exact Gate는 MG-P0-12C Guidance Law Runtime이다.
- GuidancePerformanceDesign current owner를 v0.1.15로 갱신한다.
```

### v0.6.13 - 2026-09-07

```text
- MG-P0-12A Low USER PIE에서 같은 PN 계열에 성능 수치만 다른 Low/Normal/High 구조가 원하는 행동 차이를 만들지 못한다는 USER Feel rejection을 기록했다.
- USER 핵심 원칙을 "비싼 것은 복잡하고 싼 것은 단순하게"로 명문화했다.
- ECFMissileGuidanceLaw 계획을 추가하고 PurePursuit / LeadPursuit / ProportionalNavigation을 실제 알고리즘 의미의 데이터 축으로 분리했다. Low/Normal/High 품질 enum은 만들지 않는다.
- PurePursuit는 현재 관측 위치만 단순 추적하며 Target velocity estimate, PN LOS/closing-speed 및 Sampled observer의 predictive EstimatedTargetLocation을 선행 예측에 사용하지 않는 계약으로 설계한다.
- USER 추가 요구인 발사 직후 추적과 뒤쪽 Target 급선회를 지원하기 위해 Guidance Activation을 Flight Clearance와 분리하는 ECFMissileGuidanceActivationMode + Delay/Distance 설계를 추가했다.
- Current source의 MinimumClearanceTimeSeconds=0.15s AND MinimumClearanceDistanceCm=300cm가 Direct GuidedFlight/GuidanceWindow 시작을 지연시키는 구조임을 source audit으로 확인했다.
- AfterFlightClearance를 기존 호환 기본값으로, Independent + Delay0 + Distance0에서는 first valid Guidance tick부터 Seeker Acquiring/Tracking이 가능하도록 계획했다.
- Acquisition / Tracking / Reacquisition 반각을 각각 독립 0~180deg 데이터로 두고 current Acquisition/Reacquisition <= Tracking Min coupling을 제거/재설계 대상으로 지정했다.
- 넓은 180deg Seeker + 즉시 Activation + 높은 물리 기동성을 가진 ProjectileData는 발사 방향 뒤쪽의 같은 Launch Snapshot Target도 획득하고 실제 Turn/Lateral limit 안에서 크게 U-turn할 수 있도록 허용한다. 자동 U-turn, 순간이동, 강제 명중은 계속 금지한다.
- MG-P0-12A technical setup, 공식 Build와 기존 Missile 7/7 Automation은 기술 baseline으로 보존하지만 USER Feel은 미승인 상태를 유지한다.
- 다음 exact Gate를 MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit으로 전환한다.
- GuidancePerformanceDesign current owner를 v0.1.14로 갱신한다.
```

### v0.6.12 - 2026-09-07

```text
- MG-P0-12A USER 시험 helper를 Product Runtime 모듈에서 분리해 UE/Source/CarFight_ReEditor/Private/CFMissileFeelCommands.cpp v1.1.1로 이동했다.
- 이동은 ownership correction이며 Baseline/Low/Normal/High command 계약, transient Projectile/Weapon/Equipment clone 경로, 실제 VehicleWeaponComp::InitializeWeaponRuntimeFromFitting 사용과 Variant 수치는 변경하지 않았다.
- CarFight_ReEditor가 CarFight_Re를 기존 dependency로 갖고 있으므로 Product Missile Runtime API를 그대로 소비하고 packaged Runtime에는 시험 helper가 포함되지 않는다.
- 모듈 이동 후 공식 UE 5.8 Build acadfea728224ebe9ffdbeb3e8ece08d = CFMissileFeelCommands.cpp Editor-module compile + UnrealEditor-CarFight_ReEditor.dll link / Exit 0 PASS.
- 모듈 이동 후 Focused process 89c440f6ff034fe48f3aa243082db89b = CarFight.Missile.MG_P0_12A.FeelCommandSetup 1/1 PASS / Exit 0.
- 모듈 이동 후 전체 CarFight.Missile process 9731fccffc194204b5ac31c408e5472b = 7/7 PASS / Failure 0 / Missing 0 / Unexpected 0 / Exit 0.
- MG-P0-00~11 technical baseline과 MG-P0-12A setup technical contract가 모두 보존됐다.
- Content Asset mutation은 0이며 DA_Missile_DirectTest와 DirectTest Vehicle/Weapon/Equipment/Map은 계속 regression baseline으로 보존한다.
- MG-P0-12A technical setup은 최종 PASS. USER가 MissileDirectTest PIE에서 CarFight.MissileFeel.Baseline / Low / Normal / High를 실제 전환하고 발사해 Runtime switch를 확인하는 단계만 남았다.
- 다음 exact Gate는 MG-P0-12A USER PIE Runtime Switch Validation이다. 이것이 PASS한 뒤 MG-P0-12 USER Guidance Feel Validation을 시작한다.
```

### v0.6.11 - 2026-09-07

```text
- MG-P0-12A USER 시험 경로를 persisted Low/Normal/High DataAsset 3벌이 아니라 Editor-only transient Runtime override로 구현했다.
- 당시 신규 C++(pre-module-move): UE/Source/CarFight_Re/Private/CFMissileFeelCommands.cpp v1.1.0. Current source는 v0.6.12에서 UE/Source/CarFight_ReEditor/Private/CFMissileFeelCommands.cpp v1.1.1로 이동했다.
- 저장 DA_Missile_TestSUV / EQ_Missile_DirectTest / WeaponData / ProjectileData를 기준으로 사용하며 DirectTest Content Asset mutation은 0이다.
- Low/Normal/High 선택 시 ProjectileData→WeaponData→EquipmentPresetData를 VehiclePawn Outer의 RF_Transient UObject로 복제하고 MissileGuideConfig만 변경한다.
- 실제 UCFVehicleWeaponComp::InitializeWeaponRuntimeFromFitting을 호출하고 ActiveWeaponData/ActiveProjectileData pointer readback 실패 시 저장 Baseline으로 즉시 복원한다.
- 콘솔 명령: CarFight.MissileFeel.Baseline / CarFight.MissileFeel.Low / CarFight.MissileFeel.Normal / CarFight.MissileFeel.High.
- 첫 구현 v1.0.0은 UE 5.8에 없는 FAutoConsoleCommandWithArgs 사용으로 공식 Build Exit 6 실패. existing source의 FAutoConsoleCommand + FConsoleCommandDelegate 계약으로 v1.0.1 교정 후 Build PASS.
- v1.1.0에서 CarFight.Missile.MG_P0_12A.FeelCommandSetup을 추가해 네 콘솔 명령 등록 및 Low/Normal/High exact config를 Editor process에서 검증했다.
- 최종 공식 UE 5.8 Build de0111d1155945b1a28e962c8fdf9e2e PASS / Exit 0.
- Focused process 876d4dd231b44c418d19dd72486644a5 = 1/1 PASS / Exit 0.
- CarFight.Missile affected regression process 74e7dc8034084fa1a7ab7566bb596252 = 7/7 PASS / Exit 0.
- AI-owned fresh Editor Ready까지는 확인했으나 canonical carfight.pie.start process 1ca5fb9f1dd949299fd3673e0776dc50가 Managed UE Bridge interpreter identity mismatch로 PIE 시작 전에 실패했다.
- direct UE EditorApp StartPIE도 server policy가 차단하므로 lifecycle 정책을 우회하지 않았다. 이는 Product/Setup 코드 실패가 아니라 AI-side live PIE execution blocker다.
- 따라서 MG-P0-12A는 Implementation + Offline Technical PASS이며, 실제 USER PIE에서 Baseline/Low/Normal/High Runtime 전환과 발사를 확인하는 live switch validation이 남았다.
- MG-P0-12 USER Guidance Feel Validation은 이 live switch validation 뒤 시작한다.
```

### v0.6.10 - 2026-09-07

```text
- MG-P0-11 독립 최종검수에서 P0=0 / P1=3 / P2=0을 확인했다.
- P1-1 same Target motion coverage를 CFMissileGuideStateTests.cpp v1.2.1의 동일 +Y 이동→-Y 반전 Low/Normal/High 행렬로 교정했다.
- 누적 0.05s에서 High만 +600Y를 먼저 관측하고 누적 0.09s reversal에서 Low는 seed에 남는 반면 Normal/High는 -600Y를 관측하는 실제 observation cadence 차이를 검증했다.
- reversal 직후 FilteredTargetVelocityEstimate는 High < Normal < Low 순으로 더 빠르게 -Y 변화에 반응하고 실제 Applied Guidance도 Low < Normal < High 차이를 유지했다.
- 교정 후 Focused a0ff269df8b14c3892c5155e2a24056f 1/1 PASS / Engine Exit 0, 전체 CarFight.Missile 5244de95c48d44e6b961cd0c5793392f 6/6 PASS / Engine Exit 0을 확인했다.
- 공식 Build bc116df36570486984d1afcd0f5b7508은 CFMissileGuideStateTests.cpp independent compile 및 CarFight lib/dll link까지 Result Succeeded / Exit 0 PASS했다.
- P1-2에 따라 MG-P0-12 USER Guidance Feel Validation의 Ready 표기를 철회하고 실제 Low/Normal/High PIE 비교 경로를 만드는 MG-P0-12A USER Guidance Feel Test Setup을 선행 Gate로 추가했다.
- UE WriteProbe에서 DirectTest dependency/referencer chain을 읽어 실제 테스트 자산 구성에 필요한 Projectile Actor/Mesh/Damage/FX 기준은 확보했지만, property mutation은 forward/readback/inverse 자동 복구 계약이라 persisted setup을 저장할 수 없음을 확인했다.
- 시험 생성 DA_Missile_Feel_Low는 저장하지 않고 AI-owned Editor no-save 종료로 폐기해 persisted Content mutation을 0으로 유지했다.
- P1-3 stale Current projection을 GuidancePerformanceDesign v0.1.11에서 Historical MG-P0-10 checkpoint로 교정했다.
- 최종 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. MG-P0-11은 최종 Technical PASS이며 다음 exact Gate는 MG-P0-12A다.
```

### v0.6.9 - 2026-09-07

```text
- MG-P0-11 Guidance Variant Verification Matrix를 CFMissileGuideStateTests.cpp v1.2.0으로 추가했다.
- Product Low/Normal/High 분기 없이 transient FCFMissileGuideConfig 세 개만 사용해 동일 Runtime의 데이터 기반 차이를 검증했다.
- 동일 20deg 정지 오프축 engagement에서 Applied Guidance와 Turn Rate가 Low < Normal < High로 갈리고 각자 물리 상한을 지킴을 확인했다.
- 동일 50deg 기하에서 Low는 정상 Tracking 상실/LostFinal, Normal/High는 Tracking 유지 결과를 확인했다.
- Reacquisition=None과 ForwardCone의 LostFinal 유지 / Reacquiring→Tracking 복원 차이를 같은 Launch Snapshot Target으로 검증했다.
- Low/Normal/High Pool-style Reset에서 Seeker/observer/Target 상태 완전 초기화를 확인했다.
- 쉬운 정면 정지 Target은 actual ACFProjectileActor production ProjectileMovement가 teleport 없이 실제 Blocking contact까지 도달함을 확인했다. 첫 실행 독립 진단은 35 steps / center distance 128.6cm였고 최종 PASS 실행에서도 동일 contact assertion을 유지했다.
- 최초 Focused 03e81128cf974c16bb107c8ef9a1ed31의 실패는 순간 Target 위치변화가 Sampled velocity estimate를 함께 만들어 PN 입력을 상쇄한 Test Fixture Stimulus Defect로 분류했다. Product defect 및 Product source correction은 0이다.
- 비교 자극을 동일 20deg 정지 오프축 Target으로 교정했고 Focused 5f07656c176d43e4ba770ee3a67fbbc4가 1/1 PASS / Engine Exit 0을 기록했다.
- CarFight.Missile 전체 0aa46624f9364f71b226492ed19a92d7이 6/6 PASS / Engine Exit 0을 기록했다.
- 최종 공식 UE 5.8 Build 0f75526616124297949ccb22438d07b8은 CFMissileGuideStateTests.cpp independent compile 및 CarFight lib/dll link까지 Result Succeeded / Exit Code 0 PASS했다.
- Product Runtime / Config type / 기존 RuntimeTests / Content Asset / Blueprint / Map mutation은 0이다.
- 최종 구현검수 P0=0 / P1=0 / P2=0 PASS. CF-TC-027 USER Feel은 아직 미승인이다.
- GuidancePerformanceDesign current owner를 v0.1.10으로 갱신하고 다음 exact Gate를 MG-P0-12 USER Guidance Feel Validation으로 전환한다.
```

### v0.6.8 - 2026-09-07

```text
- MG-P0-10 Observation + Velocity Estimator를 CFMissileGuideComp.h/.cpp v1.2.0으로 구현했다.
- SampledPositionEstimate가 Target 위치를 interval 기반으로 샘플하고 Actor GetVelocity 정답을 사용하지 않으며 위치 차분 + 실제 ObservationAgeSeconds로 Target Velocity를 추정하도록 했다.
- 관측 사이 EstimatedTargetLocation 외삽, velocity estimate response filter와 Stateful Seeker/Overshoot/PN 단일 Sensor Truth 경로를 연결했다.
- Guidance Window closed elapsed-only, Window open overdue single sample, low-FPS single-sample/actual-elapsed 계약을 구현했다.
- Sampled HoldLastKnownPoint는 LostFinal 순간 Estimated Sensor Truth를 fixed point로 Freeze하고 이후 Target Actor 추적/외삽을 재개하지 않도록 했다.
- CFMissileGuideStateTests.cpp v1.1.0에 MG_P0_10.ObservationEstimatorContract를 추가했다.
- Focused MG_P0_10 1/1 PASS: process b786eb16bdcc46bd947e274b9bf03e53 / Engine Exit 0.
- CarFight.Missile 전체 5/5 PASS: process 9210ce22552840c5bdcf9f7a8af55d81 / Engine Exit 0.
- 공식 UE 5.8 Build b6c4cd46bba941958abd5c45de009dc0은 UHT와 GuideComp/StateTests/RuntimeTests compile, CarFight lib/dll link까지 Result Succeeded / Exit Code 0 PASS했다.
- 기존 Legacy+Direct 기본 행동, GuideComp public API, GuideMath, Projectile integration, RuntimeTests, Content Asset을 보존했다.
- 기존 DA_Missile_DirectTest는 아직 LegacySingleGate + DirectActorKinematics이며 이번 단계에서 Product DA를 Sampled로 바꾸지 않았다.
- 최종 구현검수 P0=0 / P1=0 / P2=0 PASS. CF-TC-027 USER Feel은 아직 미승인이다.
- GuidancePerformanceDesign current owner를 v0.1.9로 갱신하고 다음 exact Gate를 MG-P0-11 Guidance Variant Verification Matrix로 전환한다.
```

### v0.6.7 - 2026-09-07

```text
- MG-P0-09 Stateful Seeker Runtime을 CFMissileGuideComp.h/.cpp v1.1.0으로 구현했다.
- Acquisition/Tracking/LostGrace/Reacquiring/LostFinal과 분리 반각, None/ForwardCone 재포착, same Launch Snapshot Target-only 계약을 연결했다.
- Destroyed Target 재포착 금지, zero grace/reacquisition time same-step transition, Overshoot 우선, HoldLastKnownPoint fixed-point semantics와 Reset을 구현했다.
- 신규 CFMissileGuideStateTests.cpp v1.0.0 / StatefulSeekerContract를 추가하고 Focused 1/1 PASS를 확인했다.
- CarFight.Missile 전체 4/4 PASS로 기존 Foundation/DirectRuntime/ConfigFoundation 회귀가 없음을 확인했다.
- 첫 공식 Build 5a76c185be894db08345c9c93a4d246c은 unrelated CFVehicleVisualComp.cpp latent standalone include dependency 때문에 Exit 6으로 실패했다.
- CFVehicleVisualComp.cpp v1.0.2에 GameFramework/Actor.h 명시 include만 추가해 non-unity build blocker를 교정했으며 Runtime/API 변경은 없다.
- 교정 후 공식 Build 9d495eda12cc42958cf19ce62d3d9ed7은 CFVehicleVisualComp.cpp standalone compile 및 CarFight DLL link까지 Result Succeeded / Exit Code 0 PASS했다.
- MG-P0-09 최종 구현검수 P0=0 / P1=0 / P2=0 PASS.
- Stateful Technical PASS는 DirectActorKinematics 범위이며 SampledPositionEstimate estimator는 MG-P0-10으로 유지했다.
- 다음 exact Gate를 MG-P0-10 Observation + Velocity Estimator 구현으로 전환한다.
```

### v0.6.6 - 2026-09-07

```text
- 기존 공식 Build Job b2cfa60513ec4fd6af5467722b697ba1 terminal receipt를 회수해 Result Succeeded / Exit Code 0을 확인했다.
- UHT 5 generated files, CFMissileFoundationTests.cpp / CFMissileRuntimeTests.cpp compile, UnrealEditor-CarFight_Re.dll link까지 PASS했다.
- Focused MG_P0_08.ConfigFoundation 1/1 PASS와 CarFight.Missile 3/3 PASS를 결합해 MG-P0-08 최종 구현검수를 P0=0 / P1=0 / P2=0 PASS로 닫았다.
- GuidancePerformanceDesign current owner를 v0.1.7로 갱신했다.
- 다음 exact Gate를 MG-P0-09 Stateful Seeker State Model 구현으로 전환한다.
```

### v0.6.5 - 2026-09-07

```text
- MG-P0-08 Types / Config Foundation을 CFMissileGuideTypes.h v1.1.0과 CFMissileFoundationTests.cpp v1.1.0으로 구현했다.
- 신규 Seeker/Observation/Reacquisition enum, 데이터 기반 성능 Config, effective clamp와 Snapshot Foundation을 append-only로 추가했다.
- 기존 저장 Missile 호환을 위해 LegacySingleGate + DirectActorKinematics + Reacquisition None 기본값을 유지했다.
- UCFMissileGuideComp Runtime은 변경하지 않아 Stateful/Sampled Runtime 동작은 아직 MG-P0-09~10 범위로 남겼다.
- Focused MG_P0_08.ConfigFoundation 1/1 PASS와 CarFight.Missile 전체 3/3 PASS를 확인했다.
- 3-test ExactTestNames runner 시도 1회는 PowerShell positional parameter binding에서 UE 실행 전 실패했고 Product failure로 분류하지 않았다. 이후 filter run으로 3개를 정상 검증했다.
- Official Editor Build Job b2cfa60513ec4fd6af5467722b697ba1은 D:\UnrealEngine_Source와 UHT 진입을 확인했으나 Browser build observation budget 소진으로 terminal Exit Code receipt는 Pending이다.
- 새 테스트가 실제 UE process에서 실행되어 신규 모듈 compile/link/load는 실증됐지만 canonical Build Job을 terminal PASS로 과장하지 않는다.
- 다음 Gate는 기존 Build Job terminal receipt 회수 → MG-P0-08 최종 구현 검수 → MG-P0-09다.
```

### v0.6.4 - 2026-09-07

```text
- GuidancePerformanceDesign v0.1.4에서 HoldLastKnownPoint용 HoldTargetLocation을 확정한 뒤 Pool Reset exact-list 누락 P1 1건을 최종 확인했다.
- GuidancePerformanceDesign을 v0.1.5로 올려 HoldTargetLocation=Zero / bHasHoldTargetLocation=false Reset을 강제하고 Pool-style Reset Automation 요구에도 포함했다.
- 대표 Plan의 Sampled observation 용어를 ObservationAgeSeconds 단일 clock으로 정렬하고 Runtime Debug 후보의 ReacquisitionTimeSeconds를 ReacquisitionElapsedTimeSeconds로 교정했다.
- 최종 재검수 P0=0 / P1=0 / P2=0 PASS. Product Source/Content mutation 0이며 MG-P0-08 Types / Config Foundation 구현 Gate를 최종 OPEN했다.
```

### v0.6.3 - 2026-09-07

```text
- GuidancePerformanceDesign v0.1.3 교정본의 최종 구현 준비 재검수에서 잔여 의미 모호성 P0=0 / P1=2 / P2=1을 추가 확인했다.
- Sampled observer의 sample interval용 시간과 extrapolation용 시간을 ObservationAgeSeconds 단일 clock으로 통합해 estimator 시간축 불일치를 제거했다.
- HoldLastKnownPoint는 LostFinal 진입 순간 Sensor Truth 위치를 1회 Freeze하고 이후 estimator 외삽을 하지 않도록 exact semantics를 확정했다.
- Stateful Acquiring/LostGrace/Reacquiring 임시 상태는 bTargetValid=false / MissReason=None, 최종 LostFinal은 TargetLost로 debug 의미를 확정했다.
- LegacySingleGate의 SeekerFieldOfViewExceeded / LockBreakAngleExceeded 진단은 보존하고 Stateful 때문에 기존 MissReason enum을 확장하지 않는다.
- GuidancePerformanceDesign을 v0.1.4로 승격하고 최종 재검수 P0=0 / P1=0 / P2=0 PASS를 기록했다.
- Product Source/Content mutation 0. 다음 exact Gate는 MG-P0-08 Types / Config Foundation 구현이다.
```

### v0.6.2 - 2026-09-07

```text
- GuidancePerformanceDesign v0.1.2를 current Source와 다시 대조한 구현 직전 독립 설계검수에서 P0=0 / P1=6 / P2=2를 확인했다.
- Acquiring 상태 문구 충돌, Destroyed Target Reacquisition, angle invariant, observation timing, Expire ownership, Snapshot 의미, future enum 노출, zero-time/EditCondition 공백을 v0.1.3에서 교정했다.
- Acquisition/Reacquisition effective cone을 Tracking cone 이하로 제한하고 Target Actor invalid/destroyed는 grace 이후 Reacquisition 없이 LostFinal로 고정했다.
- Guidance Window 닫힘 동안 observation elapsed만 누적하고 새 Actor sample/Seeker 전이는 하지 않으며 Window open 시 interval 충족이면 최대 1회 즉시 sample하도록 했다.
- LostTargetPolicy=Expire는 Guide-level LifeExpired 진단만 소유하고 실제 Projectile lifetime/Pool return은 기존 ACFProjectileActor timer가 소유하도록 no-touch 경계를 유지했다.
- FCFMissileGuideSnapshot의 기존 TargetLocation 의미를 보존하면서 신규 observer/seeker 상태 exact-list를 확정했다.
- 첫 Reacquisition Product enum은 None/ForwardCone만 노출하고 zero-time same-step 전이와 DataAsset EditCondition을 확정했다.
- ACFProjectileActor의 Motor→Flight→Guide→ProjectileMovement prerequisite를 확인해 Tick architecture 변경이 필요 없음을 재확인했다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. Product Source/Content mutation 0이며 다음 exact Gate는 MG-P0-08 Types / Config Foundation 구현이다.
```

### v0.6.1 - 2026-09-07

```text
- MG-P0-08 Source/API 계약검수를 current CFMissileGuideTypes / GuideComp / GuideMath / ProjectileData / ProjectileActor / LaunchContext / Automation Source와 대조했다.
- 초기 검수 P0=0 / P1=3 / P2=2를 확인하고 GuidancePerformanceDesign을 v0.1.2로 교정했다.
- Overshoot를 Reacquisition보다 우선하는 terminal miss로 고정하고 목표를 지나친 뒤 U-turn 재추적하는 경로를 금지했다.
- SampledPositionEstimate의 Estimated Target State를 Seeker/상태전이/PN/Overshoot가 공통 소비하는 단일 Sensor Truth로 고정했다.
- 실제 Actor observation은 Guidance Tick당 최대 1회로 제한하고 누적 elapsed time으로 velocity estimator를 갱신하도록 했다.
- UCFMissileGuideComp 기존 public signature, 기존 serialized enum 순서, GuideMath/ProjectileActor/LaunchContext/ProjectileData public integration 경계를 보존했다.
- 신규 상태/Estimator Automation은 CFMissileGuideStateTests.cpp로 분리해 현재 dirty CFMissileRuntimeTests.cpp v1.3.0 integration evidence를 보호하도록 했다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. Product Source/Content mutation 0이며 새 Build/Automation은 수행하지 않았다.
- 다음 exact Gate는 MG-P0-08 Types / Config Foundation 구현이다.
```

### v0.6.0 - 2026-09-07

```text
- CF-TC-027 USER PIE에서 MissileTarget_Lateral 추적과 실제 명중을 확인했으나, Target motion에 대한 반응이 지나치게 예측적이고 완벽하게 느껴져 USER Feel을 승인하지 않았다.
- 정지 Target 미스를 재관찰해 현재 Missile이 무조건 재추적하는 구조가 아니며 Seeker 허용각 이탈·오버슈트가 정상 Miss 원인이 될 수 있음을 Current 설계에 반영했다.
- 하나의 DirectTest Missile 수치 조정 대신 저성능부터 고성능까지 UCFProjectileData로 구성하는 Guidance Performance Variant 방향으로 대표 Plan을 교정했다.
- HitChance/MissProbability 같은 직접 명중률 확률값을 금지하고 관측·Seeker·Guidance·기동·추진 성능과 교전 기하에서 결과가 나오도록 고정했다.
- GuidancePerformanceDesign.md v0.1.1을 상세 설계 Owner로 추가했다.
- ECFMissileSeekerModel LegacySingleGate/Stateful, ECFMissileTargetObservationMode DirectActorKinematics/SampledPositionEstimate의 명시적 호환 모델 설계를 채택했다.
- Stateful Seeker의 Acquisition / Tracking / LostGrace / Reacquisition / LostFinal과 반각 기반 Acquisition/Tracking/Reacquisition Cone을 설계했다.
- SampledPositionEstimate의 관측 주기, 위치 차분 Target Velocity Estimate, 추정 응답 시간과 관측 사이 Estimated Target Location 계약을 설계했다.
- Reacquisition은 같은 Launch Target Snapshot만 허용하고 주변 Actor 자동 Retarget을 금지했다.
- 기존 DA는 기본 Legacy 모델로 재저장 없이 현재 동작을 보존하도록 Migration을 고정했다.
- 상세 설계 초안 검수 P1 1건(신규 필드 저장 여부 추측 기반 호환)을 명시적 모델 선택으로 교정하고 v0.1.1 재검수 P0=0 / P1=0 / P2=0 PASS를 확인했다.
- 기존 MG-P0-00~07 Technical PASS를 보존하고 MG-P0-08~12를 새 후속 단계로 추가했다.
- 이번 변경은 Plan/Design 문서만 수정하며 Product Source, Content Asset, Blueprint, Map mutation은 0이다.
```

### v0.5.3 - 2026-09-07

```text
- v0.5.2 독립 검수에서 FireComp→Pool Runtime evidence, persisted ProjectileActorClass runtime evidence 공백과 v1.2.0 teleport sweep 증거 과장을 P1 2건 / P2 1건으로 확인했다.
- CFMissileRuntimeTests.cpp를 v1.3.0으로 강화해 UCFVehicleFireComp::ExecuteAcceptedFireCommand → BuildDirectProjectileLaunchContext → ProjectilePoolComp::AcquireProjectileWithContext를 실제 실행했다.
- Pool이 DA_Missile_DirectTest의 persisted ProjectileActorClass를 실제 생성했고 GuidanceTargetActor가 실제 MissileGuideComp까지 전달됨을 확인했다.
- production ProjectileMovement가 순간이동 없이 21 step 동안 X=0→867cm 이동해 X=1000cm Target의 center distance 133cm Blocking 접촉 경계에서 정지함을 확인했다.
- CreateNewMap Automation의 swept OnComponentHit dispatch 공백은 실제 접촉 판정 이후 테스트 전용 bridge로만 보완하고 production Supplemental Sweep→Impact→Damage→Pool return을 검증했다.
- 이 Automation을 전체 입력→Damage E2E 또는 전 구간 무텔레포트 Impact 증거로 확대하지 않는 경계를 명시했다.
- 최종 공식 Build 31197526b5df47859beb5db28c6230e0 Exit Code 0 PASS, DirectRuntimeContract a54742a54b754361a452c33e290d8e48 Success / Exit Code 0 PASS, result SHA256 7676eda6ee34865f47709cb3f40fad29c46616a51359f68c7a3efb4f6952bc0b.
- 재검수 P0=0 / P1=0 / P2=0. Product Runtime Source / Content mutation 0. 당시 next exact Gate는 USER 측면 이동 목표 제한 선회 체감 1건이었다.
```

### v0.5.2 - 2026-09-07

```text
- CF-TC-027의 persisted DirectTest 실제 충돌·Damage 통합을 CFMissileRuntimeTests.cpp v1.2.0으로 추가했다.
- DA_Missile_DirectTest의 현재 저장 계약을 AssetDump로 확인하고 저장 에셋을 수정하지 않은 채 Automation에서 read-only 로드했다.
- production Supplemental Sweep → ResolveProjectileImpact → DamageHitContext → VehicleDamageResult → VehicleHealth Integrity 감소를 실제로 검증했다.
- 첫 compile에서 protected ACFProjectileActor::Tick 직접 호출 C2248을 확인했고 Product API 확장 없이 UWorld::Tick으로 교정했다.
- 공식 Editor Build de9e8e236d4a4c4aa17a90b5a2167cf1 Exit Code 0 PASS, DirectRuntimeContract a545e0eab0024591b159aba82da61bf4 Success / Exit Code 0 PASS를 확인했다.
- CF-TC-027 Technical 범위를 Complete/PASS로 전진하고 당시 USER Gate를 측면 이동 목표 제한 선회 체감 1건으로 축소했다.
- Product Runtime Source 및 Content Asset mutation은 0이며 기존 병렬 dirty는 건드리지 않았다.
```

### v0.5.1 - 2026-09-07

```text
- CF-FQ-048 VPS-P0-02 이후 CF-FQ-030 Current Source + Contract rebaseline을 수행했다.
- UCFVehicleFireComp의 최초 GuidanceTargetActor Snapshot, LaunchContext 전달, Launcher scheduled-shot snapshot 재사용과 ACFProjectileActor Missile activation/reset 경계를 current Source에서 확인했다.
- current UE 5.8에서 DirectRuntimeContract Job d9c8a26520cb44139e2e42d4f7d44ea2를 fresh 실행해 Success / Exit Code 0을 확인했다.
- 기존 Manual PIE 6개를 Current validation policy에 맞춰 Technical과 USER Feel로 재분류했다. Snapshot/Lost/Overshoot/Pool/선회 제한은 Technical PASS를 보존하며 반복 USER 확인을 금지한다.
- 당시 다음 exact Gate는 persisted MissileDirectTest 실제 Fire→Missile→충돌·Damage Content Integration AI Technical PIE였고 USER Gate는 측면 이동 목표 제한 선회 체감만 남겼다.
- Product Source/Content mutation은 0이고 기존 병렬 dirty는 건드리지 않았다.
```

### v0.5.0 - 2026-08-17

```text
- 과거 ue.batchdump_safe WinError 87 때문에 빠졌던 Persisted Missile Test Asset 독립 검증을 current AssetDump로 재수행해 기술 공백을 닫았다.
- Missile test folder fresh managed dataset 20/20과 Maps World dataset 12/12를 확인했고, 4개 계약 DataAsset의 저장값·hard reference chain을 PASS로 판정했다.
- DA_Missile_DirectTest의 Direct/TargetActor/ContinueStraight, DirectWeapon의 SingleCycle 1발/Direct release, EquipmentPreset과 TestSUV 연결을 저장 결과에서 확인했다.
- MissileDirectTest World package는 33 Actor와 Missile test vehicle/RocketLauncher socket 구성을 확인했다.
- Actor 전체 label은 current public readback에 노출되지 않고 SceneTools.find_actors가 server policy blocked이므로 다섯 MissileTarget label 자체는 독립 PASS로 확대하지 않았다.
- fresh DirectRuntimeContract Process 29d0ef16dd5e4d56945ceae26eec6937 1/1 PASS를 보호 회귀로 추가했다.
- Runtime Source·Content Asset·Blueprint·Map 저장 변경과 USER PIE는 0이며 새 Build는 수행하지 않았다.
```

### v0.2.0 - 2026-07-29

```text
- MG-P0-00 비활성 Foundation으로 Flight·Guidance 타입, Config, Snapshot과 순수 수학 계약을 추가했다.
- UCFProjectileData v1.8.0에 MissileFlightConfig와 MissileGuideConfig를 기본 비활성으로 연결했다.
- 제한형 비례항법이 최대 횡가속도와 최대 선회율을 동시에 적용하고 Actor·Velocity를 직접 변경하지 않도록 구현했다.
- CFMissileFoundationTests.cpp를 추가하고 Editor Build Job 8c4f952fa58f499b9145b95caa1ff926 / Exit Code 0을 확인했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run으로 기록했다.
- CF-FQ-030은 Ready를 유지하며 MG-P0-01 런타임은 LM-P0-06 사용자 PIE 뒤로 차단했다.
```

### v0.1.0 - 2026-07-28

```text
- Missile Flight와 Guidance를 Launcher에서 분리했다.
- 발사 후 독립 이동과 Target Snapshot 계약을 확정했다.
- Flight State, Guidance Mode, Lost Policy와 Attack Profile을 설계했다.
- 물리 제한형 횡가속도·선회율과 명중 비보장 원칙을 정의했다.
- Target Actor, Laser Point와 실제 충돌 Damage 경계를 고정했다.
```

---

## 19. Migration

```text
- 새 세션은 MG-P0-00~04 Direct Runtime, 2026-08-17 Persisted Asset Technical Verification, 2026-09-07 Post-VPS Fire Handoff Rebaseline, §16.3 Historical Damage Integration과 §16.4 Fire→Pool→Persisted ActorClass Review Correction PASS를 관련 failure/change evidence 없이 다시 구현하거나 반복하지 않는다.
- §16.4 Automation은 accepted Fire 실행 이후 Runtime 경계와 실제 ProjectileMovement Blocking 접촉을 증명한다. HandleFireStarted/ValidateFireCommand 실행 및 전 구간 무텔레포트 Impact까지 증명한 것으로 확대하지 않는다.
- 2026-09-07 기존 단일 USER Turn Feel Gate는 §16.5 USER 관찰에서 미승인됐고 Guidance Performance Design Correction을 여는 근거가 됐다. 이를 Pending 1건으로 되돌리지 않는다.
- Current 상세 설계 Owner는 GuidancePerformanceDesign.md v0.1.25이며 USER Low first-pass Historical rejection, corrected Low/Normal/High USER acceptance, MG-P0-12C/12D Product Runtime 계약과 MG-P0-12E passive Guidance Preset + Editor-only authoring + persisted idempotence 시험 계약을 소유한다.
- Current 최신 기술 증거는 Official UE 5.8 Build `823b88eda5fc42b4b83f41135aa2df0f` PASS, Authoring `e2e62abaac2148e8806850a137598d03` 1/1 PASS, 기존 AssetDump Guidance Preset 3/3 persisted PASS 보존, MG-P0-12E focused `f430b8d395fc4bb18db238e9eff8766e` 1/1 PASS, 전체 CarFight.Missile `2f6b1341bc5c42f988fa13a61d5e8997` 10/10 PASS다.
- Low/Normal/High의 숫자 체감 튜닝은 /Game/Test/CarFight/Missile/FeelPresets의 DA가 Current SSOT다. `UCFMissileGuidePresetData`는 passive container이고 신규 seed는 Editor authoring만 소유한다. Guidance Law/Config schema/Runtime 알고리즘 변경이 아니라 수치만 조정하는 경우 C++ rebuild를 요구하지 않는다.
- CF-TC-027은 2026-09-08 USER 직접 Low/Normal/High 확인으로 Technical + USER Complete / PASS다. CF-FQ-030 Direct P0는 Current System 승격까지 완료됐으며 현재 exact Gate는 없다. 후속 Guidance Mode/Profile/장비 연출은 별도 Feature/작업으로 연다.
- 구현 시 CFMissileGuideMath / CFProjectileLaunchTypes / CFProjectileActor / UCFProjectileData public API는 관련 compile 필요가 없는 한 기본 no-touch로 보호한다.
- 신규 상태형/Estimator 세부 Automation은 CFMissileGuideStateTests.cpp를 기본 owner로 사용해 기존 CFMissileRuntimeTests.cpp v1.3.0 correction evidence와 분리한다.
- 신규 코드 구현 시 SeekerModel 기본값은 LegacySingleGate, TargetObservationMode 기본값은 DirectActorKinematics로 두어 기존 저장 DA 행동을 보존한다.
- 기존 SeekerFieldOfViewDeg / LockBreakAngleDeg는 첫 구현에서 삭제·리네임하지 않는다.
- Stateful 모델만 Acquisition/Tracking/Reacquisition Half Angle을 사용한다.
- SampledPositionEstimate가 검증되기 전에 DirectActorKinematics 경로를 제거하지 않는다.
- Reacquisition은 기존 GuidanceTargetActor Snapshot과 같은 Actor만 허용하며 자동 Retarget하지 않는다.
- DA_Missile_DirectTest / DirectWeapon / EQ_Missile_DirectTest / DA_Missile_TestSUV 저장 계약은 관련 Source 또는 해당 Asset이 변경되지 않는 한 기존 PASS evidence를 재사용한다.
- 기존 ProjectileData 기본 비활성 호환, Rocket FixedLaunchDirection, ACFProjectileActor 충돌·Damage·Pool 계약을 유지한다.
- Angled·Vertical·Ripple·Salvo·Laser·Loft·PitchOver·TopAttack은 MG-P0-08~12 Direct TargetActor 성능 Variant 범위에 포함하지 않는다.
- MG-P0-08~10 구현에서 새 Seeker State / observation sample / velocity estimator / HoldTargetLocation / hold-valid 상태는 Pool 재사용 시 완전히 Reset돼야 한다.
```
