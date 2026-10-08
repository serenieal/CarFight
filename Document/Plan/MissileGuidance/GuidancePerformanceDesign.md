# Missile Guidance Performance Variants + Seeker State Model Design

- Version: 0.1.25
- Date: 2026-09-08
- Status: CF-FQ-030 Direct P0 Complete / Post-Closure Final Audit Correction + Re-review PASS / CF-TC-027 Technical + USER Feel PASS / Low·Normal·High USER PIE 얼추 PASS / MG-P0-12E Technical evidence preserved / Current owner main_game `Document/Systems/Combat/MissileGuidance.md v1.0.1` / Historical Design Evidence + Retained Path
- Owner Feature: `CF-FQ-030 Physics-Limited Missile Guidance`
- Representative Plan: `MissileGuidancePlan.md`

---

## 1. 목적

CarFight의 미사일 유도를 하나의 정답 튜닝으로 고정하지 않고, `UCFProjectileData`마다 서로 다른 유도 성능을 구성할 수 있는 데이터 기반 시스템으로 확장한다.

목표는 미사일마다 다음 차이를 실제 비행 결과로 만들 수 있게 하는 것이다.

```text
- 어떤 Guidance Law로 목표를 따라가는가
- 목표의 현재 위치만 단순 추적하는가, 제한된 선행점을 노리는가, 비례항법으로 교차점을 만드는가
- 목표를 얼마나 자주 관측하는가
- 목표 속도를 얼마나 빨리/정확하게 추정하는가
- 발사 후 언제부터 Seeker/Guidance가 실제 추적을 시작하는가
- 유도 명령에 얼마나 빠르게 반응하는가
- 얼마나 큰 선회율과 횡가속을 낼 수 있는가
- 어느 각도에서 목표를 처음 받아들일 수 있는가
- 추적 중 목표가 얼마나 옆이나 뒤로 벗어나도 유지할 수 있는가
- 목표를 잃은 뒤 얼마 동안 마지막 정보를 유지하는가
- 목표를 잃은 뒤 재포착을 시도하는가
- 재포착 가능한 탐색 각도가 얼마나 넓은가
```

명중률 자체를 직접 데이터로 지정하지 않는다.

```text
금지:
HitChance = 0.35
MissProbability = 0.20

허용:
관측 성능 + Seeker 성능 + 기동 성능 + 추진 성능 + 교전 기하
→ 결과적으로 서로 다른 명중 가능성
```

따라서 저가형 미사일은 쉬운 목표에도 기동 변화에 늦게 반응해 놓칠 수 있고, 고급 미사일은 같은 상황에서 더 빠르게 관측·추정·추적하지만 여전히 물리 한계와 Seeker 한계 때문에 명중을 보장하지 않는다.

---

## 2. 2026-09-07 USER Feedback에서 확인한 설계 교정 이유

현재 `DA_Missile_DirectTest`는 측면 이동 목표를 실제로 추적하고 명중할 수 있다.
하지만 USER PIE에서 다음 체감 문제가 확인됐다.

```text
1. 측면 이동 목표에 대한 반응이 지나치게 예측적이고 완벽하게 느껴진다.
2. 미사일이 목표를 지나치거나 목표가 현재 Seeker 허용각 밖으로 벗어나면 더 이상 추적하지 않는다.
3. 두 번째 현상은 반드시 버그가 아니다. 물리 제한형 유도에서 추적각 상실과 오버슈트는 정상적인 Miss 원인이 될 수 있다.
4. 따라서 문제를 "무조건 더 잘 맞게 만들기"로 교정하면 안 된다.
5. 하나의 미사일을 정답값으로 만드는 대신, 낮은 성능부터 높은 성능까지 구성 가능한 유도 성능 축이 필요하다.
```

Current Source에서 확인한 단순화 지점:

```text
- TargetActor Guide는 매 Guidance step에서 TargetActor->GetActorLocation()을 직접 읽는다.
- TargetActor Guide는 매 Guidance step에서 TargetActor->GetVelocity()를 직접 읽는다.
- 관측 갱신 주기와 속도 추정 지연이 별도로 없다.
- 현재 Seeker 판정은 min(SeekerFieldOfViewDeg * 0.5, LockBreakAngleDeg)을 매 step 동일하게 적용한다.
- 따라서 Acquisition과 이미 성립한 Tracking의 유지 한계가 분리되지 않는다.
```

현재 구현은 잘못된 결과를 임의로 만드는 코드가 아니라 **P0용 단순화 모델**이다.
이번 교정은 해당 단순 모델을 데이터 기반 품질 차이가 가능한 상태 모델로 확장한다.

### 2.1 2026-09-07 USER Low Feel 판정 — Guidance Law 자체가 성능축이어야 함

MG-P0-12A의 Low transient Variant를 실제 PIE에서 발사한 USER 판정으로 다음 문제가 확인됐다.

```text
- Low / Normal / High가 관측 주기, 추정 응답, 선회율, 횡가속, Seeker 각도만 다르고 핵심 Guidance Law는 모두 같은 PN 계열이다.
- 그래서 Low도 근본적으로 Target의 이동 경로를 예측하고 앞을 잘라 들어가는 성향을 유지한다.
- USER가 원하는 저성능 미사일은 "같은 똑똑한 알고리즘을 느리고 약하게 돌리는 미사일"이 아니다.
- 저성능 미사일은 매 Guidance update에서 그 순간 관측한 Target 위치 자체를 단순하게 쫓는 행동이 더 자연스럽다.
- 고성능 미사일은 Target velocity / 상대 운동 / LOS 변화 등을 이용해 Target의 진행 앞을 잘라 들어가는 복잡한 유도를 사용해도 된다.
```

따라서 기존 설계의 다음 문장은 의미를 수정한다.

```text
기존 의도:
낮은 성능과 높은 성능 미사일은 같은 Runtime 코드에서 Data 수치만 달라야 한다.

교정 의도:
낮은 성능과 높은 성능 미사일은 같은 Runtime Framework를 공유하되,
Data가 선택한 Guidance Law Strategy 자체도 서로 다를 수 있어야 한다.
```

품질 등급 `Low/Normal/High`를 Runtime enum으로 만들지는 않는다.
대신 실제 유도 알고리즘의 의미를 나타내는 독립 Strategy를 데이터로 선택한다.

계획 Enum:

```text
ECFMissileGuidanceLaw
- ProportionalNavigation   // 기존 동작 호환 기본값
- PurePursuit              // 현재 관측 위치만 단순 추적
- LeadPursuit              // 제한된 선행점 추적
```

각 Guidance Law의 계약:

```text
PurePursuit
- Target의 "현재 사용할 수 있는 위치"만 향한다.
- TargetVelocityEstimate를 선행 예측에 사용하지 않는다.
- LOS angular velocity / closing speed 기반 교차 유도를 사용하지 않는다.
- SampledPositionEstimate와 조합할 경우 기본 입력 위치는 LastObservedTargetLocation이다.
- 즉 관측 사이에 velocity extrapolation으로 앞을 예측한 EstimatedTargetLocation을 몰래 사용하지 않는다.
- 결과적으로 뒤를 쫓고, 급기동에 늦고, 자연스럽게 놓칠 수 있다.

LeadPursuit
- LastObservedTargetLocation을 기준점으로 사용한다.
- LeadOffset = FilteredTargetVelocityEstimate * LeadTimeSeconds로 계산한다.
- LeadOffset 크기는 MaxLeadDistanceCm로 제한한다.
- GuidanceAimPoint = LastObservedTargetLocation + BoundedLeadOffset으로 만든다.
- EstimatedTargetLocation을 기준점으로 다시 더해 이중 예측하지 않는다.
- 완전한 PN보다 단순하지만 PurePursuit보다 앞을 본다.

ProportionalNavigation
- 현재 구현 계열의 고급 유도법이다.
- Estimated Target State, Target velocity estimate, LOS 변화, closing speed를 이용해 교차 경로를 만든다.
- 단, Target이 뒤쪽이거나 아직 접근 기하가 형성되지 않아 PN ClosingSpeed가 0인 구간에서는 PN 명령이 0으로 굳지 않도록 물리 제한형 Course Capture를 먼저 허용한다.
- Course Capture는 현재 Sensor Truth 방향을 향한 Pursuit steering으로 기수만 돌리고, ForwardClosingVelocity > 0인 접근 기하가 형성되면 PN으로 전환한다.
- Course Capture도 MaximumTurnRateDegPerSec / MaximumLateralAccelerationCmPerSecSq / GuidanceResponseTimeSeconds를 그대로 지키며 위치·Velocity 순간이동이나 강제 명중을 하지 않는다.
- 기존 저장 Asset 호환을 위해 신규 GuidanceLaw의 C++ 기본값은 ProportionalNavigation으로 둔다.
```

중요한 독립축:

```text
Observation Model
× Seeker Model
× Guidance Law
× Guidance Activation
× Physical Performance
```

따라서 예시는 다음과 같이 구성할 수 있다.

```text
구형/저가형
SampledPositionEstimate + Stateful + PurePursuit + 느린 관측 + 낮은 기동성

보급형
SampledPositionEstimate + Stateful + LeadPursuit + 중간 관측 + 중간 기동성

고급형
Fast SampledPositionEstimate + Stateful + ProportionalNavigation + 높은 기동성
```

### 2.2 발사 후 Guidance 시작 시점과 360°에 가까운 Seeker 형상 요구

USER가 추가로 원하는 비행 형태:

```text
- 발사 후 한동안 무조건 직진하다가 꺾이는 미사일만 존재하면 안 된다.
- 어떤 미사일은 발사 거의 직후 Guidance가 시작돼야 한다.
- Launch Target Snapshot이 발사 방향의 뒤쪽에 있어도 넓은 Seeker를 가진 미사일은 즉시 그 Target을 획득할 수 있어야 한다.
- 충분한 MaximumTurnRate / MaximumLateralAcceleration이 설정돼 있으면 실제 물리 제한 안에서 크게 휘어 뒤쪽 Target을 따라갈 수 있어야 한다.
```

현재 Source에서 확인한 원인:

```text
FCFMissileFlightConfig.MinimumClearanceTimeSeconds = 0.15s
FCFMissileFlightConfig.MinimumClearanceDistanceCm = 300cm

IsClearanceSatisfied()
= 시간 조건 AND 거리 조건

Direct Flight:
Clearance 충족
→ GuidedFlight
→ IsGuidanceWindowOpen = true
```

즉 현재는 **Flight Clearance와 Seeker/Guidance 시작 시점이 같은 Gate**라서 0.15s + 300cm가 사실상 초기 직진 구간을 만든다.

교정 원칙:

```text
Flight Clearance
= 런처 분리, 점화/전환, 발사체 안전거리 같은 비행 상태 소유권

Guidance Activation
= 발사 후 Seeker가 Acquiring을 시작하고 Guidance Command를 허용하는 시점

두 개를 독립 설정할 수 있어야 한다.
```

계획 Enum/필드:

```text
ECFMissileGuidanceActivationMode
- FollowFlightGuidanceWindow   // 기존 동작 호환 기본값
- Independent

GuidanceActivationDelaySeconds
GuidanceActivationDistanceCm
```

의미:

```text
FollowFlightGuidanceWindow
- 현재 Source의 UCFMissileFlightComp::IsGuidanceWindowOpen() 결과를 그대로 따른다.
- 현재 Direct에서는 Clearance 완료 → GuidedFlight가 결과적으로 Guidance Window를 열지만, 미래 Transition/Loft/PitchOver에서는 Clearance 완료와 Guidance Window Open이 동일하다고 가정하지 않는다.
- 기존 저장 Asset 호환 기본값이다.

Independent
- Flight Clearance / Flight Guidance Window와 별도로 발사 후 경과시간·Release 기준 거리로 Guidance 시작을 결정한다.
- Delay=0, Distance=0이면 발사 후 첫 유효 Guidance tick부터 Acquiring/Tracking을 시작할 수 있다.
- 시간만 쓰고 싶으면 Distance=0, 거리만 쓰고 싶으면 Delay=0으로 설정할 수 있다.

DelaySatisfied
= GuidanceActivationDelaySeconds <= 0
  OR FlightSnapshot.ElapsedFlightTimeSeconds >= GuidanceActivationDelaySeconds

DistanceSatisfied
= GuidanceActivationDistanceCm <= 0
  OR FlightSnapshot.DistanceFromReleaseCm >= GuidanceActivationDistanceCm

IndependentActivationSatisfied
= DelaySatisfied AND DistanceSatisfied

- IndependentActivationSatisfied가 한 번 True가 되면 해당 activation 동안 bGuidanceActivationSatisfied=True로 latch한다.
- U-turn 등으로 Release 지점과 다시 가까워져도 Guidance를 다시 닫지 않는다.
- latch는 ResetMissileGuidance에서만 False로 초기화한다.
- 시간·거리 측정값의 owner는 UCFMissileFlightComp, activation 판정과 latch owner는 UCFMissileGuideComp다.
```

Seeker 각도는 Stateful에서 다음 세 값이 **서로 독립적인 0~180° 중심선 반각**이어야 한다.

```text
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
ReacquisitionConeHalfAngleDeg
```

MG-P0-12D에서 `GetEffectiveAcquisitionConeHalfAngleDeg()`와 `GetEffectiveReacquisitionConeHalfAngleDeg()`의 Tracking 반각 `Min()` coupling을 제거했다. 현재 Stateful Acquisition / Tracking / Reacquisition 반각은 각각 독립적으로 0~180°만 안전 보정하며 서로를 암묵적으로 축소하지 않는다.

```text
허용 예:
Acquisition = 180°
Tracking    = 180°
Reacquire   = 180°

→ 발사 방향 정반대 Target도 각도만으로는 획득/유지 가능
→ 실제 U-turn 속도와 곡률은 MaximumTurnRate / MaximumLateralAcceleration이 결정
→ 위치/Velocity 순간이동이나 강제 명중은 여전히 금지
```

따라서 기존 원칙인 "뒤쪽 Target이라고 무조건 U-turn하지 않는다"는 다음처럼 교정한다.

```text
금지:
모든 미사일이 Target 상실 또는 뒤쪽 Target을 자동으로 180° U-turn해 강제 재추적

허용:
해당 ProjectileData가 넓은 Acquisition/Tracking/Reacquisition 각도와
빠른 Guidance Activation, 충분한 물리 기동성을 명시한 경우
Launch Snapshot Target을 실제 물리 제한 안에서 크게 선회해 추적
```

정확히 180° 정반대 Target은 좌/우 회전 방향이 수학적으로 모호할 수 있으므로 deterministic tie-break를 둔다.

```text
일반 rear-aspect
→ Target 방향의 실제 lateral steering 성분 사용

거의 정확히 180°이고 lateral steering 성분을 안정적으로 만들 수 없음
→ FCFProjectileLaunchContext.LaunchTransform의 Right Vector를 stable turn-side 기준으로 사용
→ random 선택 금지
→ 이 기준은 회전 방향만 결정하며 선회율/횡가속 한계는 그대로 적용
```

### 2.3 2026-09-08 Low USER Feel Reject — 초기 탄도 존중 계약

MG-P0-12E v1.2.0 Low fixture를 실제 `MissileDirectTest` PIE에서 시험한 USER가 다음 체감 문제를 확인했다.

```text
- 이동 Target 앞을 적절히 선행 조준해 초기 발사선 자체가 좋아 보이는 조건으로 발사
- Low Missile이 초기 발사선을 빠르게 버리고 Target의 현재/최근 관측 위치 쪽으로 꺾음
- Target 근처에서 다시 크게 수정하며 거의 Target을 피하는 듯한 Miss 발생
- 단순히 "저가형이라 명중률이 낮다"가 아니라 USER가 만든 좋은 launch solution을 Guidance가 스스로 망치는 느낌
```

USER 판정은 `REJECT`다.

이 결과는 `PurePursuit` 자체를 제거할 근거로 확대하지 않는다. v1.2.0 Low는 단순 추적 알고리즘에 다음 약점을 동시에 겹쳤다.

```text
FollowFlightGuidanceWindow 기반 비교적 이른 Guidance 개입
Observation 0.15s
VelocityEstimateResponse 0.35s
GuidanceResponse 0.30s
MaximumTurnRate 25deg/s
MaximumLateralAcceleration 1200cm/s^2
Acquisition 25deg
Tracking 35deg
```

따라서 Current Low 설계는 **단순 알고리즘과 실용성**을 분리한다.

```text
단순함
= PurePursuit 유지
= GuidanceAimPoint는 LastObservedTargetLocation
= LeadTime 0 / MaxLead 0
= PN/Lead prediction 사용 금지

실용성
= 잘 만든 USER 초기 조준을 짧게 보존
= 관측/응답이 지나치게 늦어 Target 근처에서 뒤늦은 큰 재수정을 반복하지 않게 완화
= 저가형이라도 쉬운 기하에서는 실제로 맞을 수 있는 Seeker/기동 성능 확보
```

Current corrected Low v1.2.1 fixture:

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

Low USER acceptance:

```text
PASS
- 이동 Target 앞을 USER가 적절히 선행 조준한 경우 발사 직후 좋은 초기 탄도를 Guidance가 즉시 현재 Target 위치 쪽으로 무너뜨리지 않는다.
- 짧은 초기 탄도 보존 뒤에는 LastObservedTargetLocation만 단순 추적한다.
- 쉬운 기하 또는 좋은 수동 선행 조준에서는 실용적으로 명중할 수 있다.
- 적극 회피 Target, 큰 기하 변화와 PurePursuit 구조상 불리한 상황에서는 자연스럽게 Miss할 수 있다.

FAIL
- 좋은 초기 탄도를 Guidance가 즉시 버려 명중 가능성을 현저히 낮춘다.
- Target 근처에서 늦은 큰 재수정 때문에 반복적으로 Target을 피하는 것처럼 보인다.
- 저가형이라는 이유로 쉬운 교전에서도 사실상 명중할 수 없다.
```

이 계약은 HitChance나 강제 명중을 추가하라는 의미가 아니다. Low가 **잘 조준한 발사의 관성적 가치**를 보존하면서도 활성화 뒤에는 여전히 단순 PurePursuit로 행동해야 한다는 USER Feel 기준이다.

---

## 3. 강제 설계 원칙

```text
1. 명중률을 직접 확률값으로 만들지 않는다.
2. 낮은 성능과 높은 성능 미사일은 같은 Runtime Framework를 공유하되 Data가 Guidance Law Strategy와 개별 성능값을 선택해야 한다.
3. 미사일별 차이는 기존 UCFProjectileData의 MissileGuideConfig를 우선 확장한다.
4. 단순 품질 등급 Enum(Low/Medium/High)으로 Runtime 행동을 하드코딩하지 않는다.
5. 품질 등급 이름이 필요하면 Authoring Preset 또는 예시 DA에서만 사용하고 실제 Runtime은 개별 수치를 소비한다.
6. 미사일은 TargetActor의 완벽한 월드 속도를 매 frame 정답으로 소비하는 방식에서 선택적으로 벗어날 수 있어야 한다.
7. 기존 저장 Asset은 별도 재저장 없이 기존 Direct Actor Kinematics + Single Gate 행동을 유지해야 한다.
8. 랜덤 오차는 첫 구현에 넣지 않는다. 먼저 결정론적 관측 주기와 속도 추정 지연으로 품질 차이를 만든다.
9. Seeker의 최초 획득, 추적 유지, 상실 유예와 재포착을 서로 다른 상태로 취급한다.
10. 발사 후 Seeker Acquisition은 발사 전 Equipment Lock-On과 절대 같은 상태가 아니다.
11. 목표가 뒤쪽이라는 이유만으로 모든 미사일에 U-turn을 강제하지 않는다. 단, ProjectileData가 넓은 Seeker 각도·빠른 Guidance Activation·충분한 물리 기동성을 명시하면 Launch Snapshot Target을 뒤쪽에서도 획득하고 실제 제한 안에서 U-turn 추적할 수 있어야 한다.
12. Reacquisition은 기존 Launch Target Snapshot과 같은 Target만 다시 받아들이며 다른 Actor로 자동 Retarget하지 않는다.
13. 목표 상실, 오버슈트와 수명 종료는 정상 결과로 유지한다.
14. Terminal Guidance는 명중률 보정용 마법 단계로 사용하지 않는다.
```

---

## 4. 소유권 경계

### 4.1 발사 전 Equipment Lock-On

소유자:

```text
Weapon / Equipment Lock system
```

책임:

```text
- 어떤 Target을 Lock 후보로 삼는가
- Lock 진행 시간
- Lock 완료 여부
- 발사 허용 여부
- HUD Lock 표시
```

이번 설계에서 구현하지 않는다.

### 4.2 발사 순간 Target Snapshot

기존 계약 유지:

```text
TargetSelect / Weapon target use
→ GuidanceTargetActorSnapshot
→ FCFProjectileLaunchContext.GuidanceTargetActor
→ 발사된 Missile 독립 참조
```

차량이 발사 후 다른 Target을 선택해도 이미 발사된 Missile은 바뀌지 않는다.

### 4.3 발사 후 Missile Seeker

소유자:

```text
UCFMissileGuideComp
```

책임:

```text
- Snapshot Target을 Seeker가 현재 받아들일 수 있는지 판정
- 관측 샘플 갱신
- 목표 속도 추정
- Tracking 유지
- Lost Grace
- 선택적 Reacquisition
- Guidance Command 입력 생성
```

---

## 5. 기존 Asset 호환을 위한 명시적 모델 선택

새 필드가 저장됐는지 여부를 추측해서 Legacy/New 행동을 나누지 않는다.
UE 직렬화에서 신규 UPROPERTY는 기존 Asset에도 C++ 기본값으로 존재할 수 있으므로 **모델 선택 필드를 명시적으로 둔다.**

### 5.1 Seeker 모델

계획 Enum:

```text
ECFMissileSeekerModel

LegacySingleGate
Stateful
```

기본값:

```text
LegacySingleGate
```

의미:

```text
LegacySingleGate
= 현재 min(SeekerFieldOfViewDeg * 0.5, LockBreakAngleDeg) 판정을 그대로 유지

Stateful
= Acquisition / Tracking / LostGrace / Reacquisition / LostFinal 상태 모델 사용
```

기존 저장 Asset은 신규 필드를 재저장하지 않아도 `LegacySingleGate` 기본값으로 현재 행동을 유지한다.

### 5.2 Target 관측 모델

계획 Enum:

```text
ECFMissileTargetObservationMode

DirectActorKinematics
SampledPositionEstimate
```

기본값:

```text
DirectActorKinematics
```

의미:

```text
DirectActorKinematics
= 현재처럼 TargetActor Location / Velocity를 직접 읽는 Legacy-compatible 경로

SampledPositionEstimate
= 일정 주기로 위치를 관측하고 위치 변화로 Target Velocity를 추정하는 신규 성능 경로
```

따라서 이번 교정은 기존 미사일을 일괄적으로 약화시키지 않는다.
새 미사일 DA 또는 명시적으로 교정한 DA부터 `Stateful + SampledPositionEstimate`를 선택한다.

---

## 6. Guidance 성능 축

기존 `FCFMissileGuideConfig`를 중심으로 확장한다.
Product Missile Runtime을 위한 별도 품질등급/Performance DataAsset은 새로 만들지 않는다.

다만 MG-P0-12 USER 체감 반복 튜닝은 C++ 숫자 하드코딩을 제거하기 위해 **시험/저작 전용 `UCFMissileGuidePresetData`**를 사용한다. 이 Preset은 `FCFMissileGuideConfig`만 소유하고 Projectile 속도·피해·Collision·FX·ActorClass는 소유하지 않는다. 실제 Product Projectile의 최종 Runtime 계약은 계속 `UCFProjectileData.MissileGuideConfig`가 소유한다.

### 6.0 신규 Guidance Law / Activation 축

기존 관측·Seeker·기동 수치에 더해 다음을 독립 데이터축으로 추가한다.

```text
GuidanceLaw
GuidanceActivationMode
GuidanceActivationDelaySeconds
GuidanceActivationDistanceCm
LeadTimeSeconds                  // LeadPursuit에서만 사용
MaxLeadDistanceCm                // LeadPursuit에서만 사용
```

강제 소비 규칙:

```text
PurePursuit
→ GuidanceAimPoint = LastObservedTargetLocation
→ Target velocity estimate로 선행점 생성 금지
→ Sampled observer의 EstimatedTargetLocation을 pursuit aim point로 몰래 사용하지 않음

LeadPursuit
→ LeadOffset = FilteredTargetVelocityEstimate * LeadTimeSeconds
→ LeadOffset magnitude <= MaxLeadDistanceCm
→ GuidanceAimPoint = LastObservedTargetLocation + LeadOffset

ProportionalNavigation
→ Estimated Target State + LOS/closing-speed 기반 PN 소비
→ ClosingSpeed가 0이고 Target이 유효한 rear/non-closing 기하에서는 bounded Course Capture로 접근 기하를 먼저 형성
→ ForwardClosingVelocity > 0이 되면 PN으로 전환

GuidanceActivationMode=Independent
→ Flight Guidance Window가 닫혀 있어도 delay/distance 조건을 만족하면 Guide가 Acquiring/Tracking을 시작할 수 있어야 함
→ 만족 상태는 activation 동안 latch되며 다시 닫히지 않음
→ launcher collision/clearance 위험은 실제 Flight/Collision 결과가 책임짐
```

DataAsset authoring 조건:

```text
NavigationConstant
→ GuidanceLaw == ProportionalNavigation

LeadTimeSeconds / MaxLeadDistanceCm
→ GuidanceLaw == LeadPursuit

GuidanceActivationDelaySeconds / GuidanceActivationDistanceCm
→ GuidanceActivationMode == Independent
```

MG-P0-12 USER Feel 시험 Preset authoring 계약:

```text
Class = UCFMissileGuidePresetData
Root = /Game/Test/CarFight/Missile/FeelPresets
Low = DA_MissileFeel_Low
Normal = DA_MissileFeel_Normal
High = DA_MissileFeel_High

Preset owns:
- PresetId
- PresetDisplayName
- PresetDescription
- MissileGuideConfig

Preset does not own:
- Projectile InitialSpeed / Motor / Damage / Collision / FX / ActorClass
- Product Low/Normal/High enum/branch

Runtime Feel command:
Persisted DirectTest Projectile/Weapon/Equipment transient duplicate
+ selected Preset.MissileGuideConfig only
→ VehicleWeaponComp::InitializeWeaponRuntimeFromFitting

Numeric feel tuning:
- Preset DA를 수정·저장
- C++ rebuild 불필요

C++ 변경이 다시 필요한 경우:
- Guidance Law/Config schema 추가·변경
- Runtime 알고리즘/상태전이 변경
- Preset class/authoring contract 자체 변경
```

최초 3개 Preset 생성은 명시 실행형 `CarFight.Authoring.MissileFeel.EnsurePresets`가 UE `SavePackage` 정식 경로로 수행한다. `UCFMissileGuidePresetData` 자체는 `PresetId / 표시 이름 / 설명 / MissileGuideConfig`만 보관하는 **passive data container**이며 `PostInitProperties`, 자산 이름 판별, Low/Normal/High seed를 소유하지 않는다. Low/Normal/High 신규 seed는 `CarFight_ReEditor`의 authoring 경로가 **새 Asset을 실제 생성한 직후에만** 적용한다. 기존 Preset이 이미 존재하면 seed/save 경로에 진입하지 않고 저장된 사용자 튜닝값을 그대로 재사용한다. 따라서 Asset load-time에는 Product DataAsset 클래스가 사용자 값을 재주입하거나 기본값으로 되돌리는 경로가 없다. WriteProbe의 rollback-only mutation 경계를 persisted authoring 권한으로 확대하지 않는다.

Idempotence 검증은 실제 Low/Normal/High 3개를 시험용으로 수정하지 않는다. `CarFight.Authoring.MissileFeel.EnsurePresets`가 별도 임시 persisted Preset을 생성하고 USER sentinel 값과 C++ 기본값과 같은 값까지 저장한 뒤, 같은 경로에 의도적으로 다른 Variant seed를 요청하는 Ensure를 다시 수행한다. 이때 existing Asset 재사용, package clean, USER 값 보존을 확인하고 `ReloadPackage`로 디스크 package를 다시 읽어 동일값이 유지됨을 검증한 뒤 임시 `.uasset`/sidecar를 정리한다.

신규 float는 finite 검사와 안전 clamp를 갖는다. Stateful Seeker angle은 각 필드별 0~180°만 clamp하고 서로를 `Min()`으로 강제 결합하지 않는다.

Guide Snapshot은 기존 필드를 깨지 않고 다음 진단값을 append-only로 노출하는 것을 구현 목표로 한다.

```text
GuidanceLaw
GuidanceActivationMode
bGuidanceActivationSatisfied
GuidanceAimPoint
bCourseCaptureActive
bOvershootArmed
```

### 6.1 이미 존재하는 물리/유도 성능 축

```text
NavigationConstant
MaximumTurnRateDegPerSec
MaximumLateralAccelerationCmPerSecSq
GuidanceResponseTimeSeconds
MinimumGuidanceSpeedCmPerSec
TargetLostGraceTimeSeconds
LostTargetPolicy
```

이 값들은 계속 사용한다.

### 6.2 신규 관측 성능 축

계획 필드:

```text
TargetObservationMode
TargetObservationIntervalSeconds
TargetVelocityEstimateResponseTimeSeconds
```

#### TargetObservationIntervalSeconds

의미:

```text
SampledPositionEstimate에서 Missile Seeker가 Target의 새 위치 샘플을 얻는 간격
```

예시:

```text
저가형: 0.15s
보통:   0.08s
고급형: 0.03s
```

초기 구현에서는 랜덤 누락이나 위치 노이즈를 추가하지 않는다.
같은 입력이면 같은 결과가 나오도록 결정론적으로 유지한다.

#### TargetVelocityEstimateResponseTimeSeconds

의미:

```text
연속 위치 샘플의 변화로 계산한 Raw Velocity Estimate가
실제 Guidance 입력 속도 추정값에 얼마나 빠르게 반영되는가
```

이 값은 `GuidanceResponseTimeSeconds`와 다르다.

```text
TargetVelocityEstimateResponseTimeSeconds
= 목표 움직임을 얼마나 빨리 이해하는가

GuidanceResponseTimeSeconds
= 계산된 조향 명령에 미사일이 얼마나 빨리 반응하는가
```

---

## 7. Sampled Position Observation 계약

### 7.1 최초 샘플

`StartMissileGuidance()`에서 Target이 유효하면 최초 위치 샘플을 하나 저장한다.

```text
PreviousObservedTargetLocation = Current Target Location
LastObservedTargetLocation = Current Target Location
FilteredTargetVelocityEstimate = Zero
```

두 번째 유효 샘플이 오기 전에는 미래 속도를 알고 있다고 가정하지 않는다.

### 7.2 후속 샘플

관측 타이머가 `TargetObservationIntervalSeconds`에 도달했을 때만 실제 TargetActor 위치를 다시 읽는다.

```text
RawVelocityEstimate
= (NewObservedLocation - PreviousObservedLocation) / ObservationAgeSeconds
```

한 Guidance Tick의 `DeltaTime`이 관측 간격보다 크더라도 같은 Actor 위치를 여러 번 읽어 가짜 중간 샘플을 만들지 않는다.

```text
ObservationAgeSeconds += SafeDeltaTime
if ObservationAgeSeconds >= TargetObservationIntervalSeconds:
    실제 Actor 위치는 이번 Tick에 최대 1회만 Sample
    SampleDeltaTime = ObservationAgeSeconds
    ObservationAgeSeconds = 0
```

따라서 저 FPS나 hitch에서도 `while (Accumulator >= Interval)` 방식으로 동일 위치를 여러 샘플로 소비하지 않는다.

이 Raw Estimate를 `TargetVelocityEstimateResponseTimeSeconds`로 필터링해 `FilteredTargetVelocityEstimate`를 만든다.
필터 기본 계약은 기존 Guidance response와 같은 결정론적 1차 응답으로 시작한다.

```text
EstimateResponseAlpha
= Clamp(SampleDeltaTime / TargetVelocityEstimateResponseTimeSeconds, 0, 1)
```

### 7.3 Guidance Tick 사이의 Target 상태

관측 Tick 사이에서 매 frame 실제 `TargetActor->GetActorLocation()`을 다시 읽지 않는다.

Guidance가 사용할 Target 상태는 마지막 관측 상태에서 계산한다.

```text
EstimatedTargetLocation
= LastObservedTargetLocation
+ FilteredTargetVelocityEstimate * ObservationAgeSeconds
```

이 방식은 완벽한 미래 예측이 아니다.
마지막으로 관측한 위치와 당시까지 추정한 속도만으로 짧게 외삽한다.
목표가 갑자기 방향을 바꾸면 다음 관측과 속도 추정이 따라오기 전까지 기존 추정에 오차가 생긴다.

`SampledPositionEstimate`에서는 `LastObservedTargetLocation / EstimatedTargetLocation / FilteredTargetVelocityEstimate`가 하나의 관측 계층에서 나온 **Sensor State**다.
숨은 Actor truth를 소비자별로 다시 읽어 보정하지 않는다.

소비 규칙은 Guidance Law 의도에 따라 명시적으로 분리한다.

```text
Stateful Seeker angle / Tracking / LostGrace / Reacquisition
→ EstimatedTargetLocation

Target distance / Stateful Overshoot
→ EstimatedTargetLocation + bOvershootArmed 계약

PurePursuit GuidanceAimPoint
→ LastObservedTargetLocation

LeadPursuit GuidanceAimPoint
→ LastObservedTargetLocation
   + ClampMagnitude(FilteredTargetVelocityEstimate * LeadTimeSeconds, MaxLeadDistanceCm)

ProportionalNavigation
→ EstimatedTargetLocation + FilteredTargetVelocityEstimate
```

따라서 "Single Sensor Truth"는 모든 Law가 같은 aim point를 써야 한다는 뜻이 아니다.
같은 observer가 만든 Sensor State만 사용하고, 각 Guidance Law가 허용된 정보량만 소비한다는 뜻이다.

### 7.4 Legacy 경로

`TargetObservationMode=DirectActorKinematics`에서는 기존 Source 행동을 유지한다.

```text
TargetLocation = TargetActor->GetActorLocation()
TargetVelocityEstimate = TargetActor->GetVelocity()
```

신규 sampled estimator 검증이 끝나기 전에 Legacy 경로를 삭제하지 않는다.

---

## 8. Seeker 각도 데이터 교정

현재 필드:

```text
SeekerFieldOfViewDeg
LockBreakAngleDeg
```

현재 Runtime은 다음처럼 사용한다.

```text
AllowedAngle = min(SeekerFieldOfViewDeg * 0.5, LockBreakAngleDeg)
```

이 방식은 최초 목표 획득과 Tracking 유지 한계를 구분하지 못한다.
특히 `SeekerFieldOfViewDeg=120`, `LockBreakAngleDeg=85`이면 실제 허용각은 항상 60도라 LockBreak 85도가 Tracking 유지 범위를 확장하지 못한다.

### 8.1 Stateful 모델의 명시적 반각 필드

계획 필드:

```text
AcquisitionConeHalfAngleDeg
TrackingConeHalfAngleDeg
ReacquisitionConeHalfAngleDeg
ReacquisitionTimeSeconds
```

모든 Angle 값은 **미사일 진행 중심선에서 한쪽 방향으로 허용하는 반각**으로 정의한다.
전체 FOV와 반각을 혼용하지 않는다.

예:

```text
AcquisitionConeHalfAngleDeg = 30
→ 전체 최초 획득 원뿔 = 60도

TrackingConeHalfAngleDeg = 65
→ 이미 Tracking 중인 목표는 중심선에서 65도까지 유지 가능
```

### 8.2 기존 필드 Migration

`SeekerModel=LegacySingleGate`일 때는 신규 Angle 필드를 사용하지 않고 기존 필드를 현재와 동일하게 소비한다.

```text
AllowedAngle
= min(SeekerFieldOfViewDeg * 0.5, LockBreakAngleDeg)
```

`SeekerModel=Stateful`인 DA만 신규 반각 필드를 사용한다.

기존 `SeekerFieldOfViewDeg` / `LockBreakAngleDeg` 제거·리네임은 첫 구현에서 하지 않는다.
Deprecated 전환이 필요하면 별도 Migration 검수 후 진행한다.

---

## 9. Seeker 상태 모델

계획 Enum:

```text
ECFMissileSeekerState

Inactive
Acquiring
Tracking
LostGrace
Reacquiring
LostFinal
```

### 9.1 Inactive

Guidance가 꺼져 있거나 현재 `GuidanceActivationMode`의 activation gate가 아직 만족되지 않은 상태다.
`FollowFlightGuidanceWindow`는 FlightComp의 Guidance Window를 따르고, `Independent`는 latched delay/distance 조건을 따른다.

### 9.2 Acquiring

Guidance Activation gate가 열린 뒤 발사 순간 Snapshot Target을 Missile Seeker가 받아들일 수 있는지 검사한다.

조건:

```text
Target reference valid
AND
Target angle <= AcquisitionConeHalfAngleDeg
```

성공:

```text
→ Tracking
```

실패:

```text
Target reference는 유효하지만 Acquisition cone 밖
→ Acquiring 유지
→ Guidance Command 없음

Target reference 자체가 무효/Destroyed
→ LostFinal
```

`Acquiring`에서는 아직 Tracking에 성공한 적이 없으므로 `TargetLostGraceTimeSeconds`를 사용하지 않는다.
최초 획득 실패를 LostGrace로 우회하지 않는다.

최초 획득에 성공한 적이 없는 Target에 `TargetLostGraceTimeSeconds`를 부여하지 않는다.
LostGrace는 한 번 `Tracking`이 성립한 뒤 추적을 놓친 경우에만 사용한다.

중요:

```text
Acquiring != Equipment Lock-On
```

장비 Lock은 발사 전 교전 권한이고, Missile Acquiring은 발사 후 온보드 Seeker의 Target 수용 상태다.

### 9.3 Tracking

추적 중에는 더 넓은 Tracking Cone을 사용할 수 있다.

```text
Target angle <= TrackingConeHalfAngleDeg
→ Tracking 유지
```

Tracking에서는 선택한 TargetObservationMode에 따라 Target 상태를 갱신하고 Guidance Command를 계산한다.

### 9.4 LostGrace

한 번 `Tracking`이 성립한 뒤 추적이 끊겼을 때 즉시 영구 상실로 만들지 않는 유예 상태다.

```text
TargetLostGraceTimeSeconds
```

동안 마지막 관측/추정 Target 상태를 사용할 수 있다.
이 동작은 현재 Legacy가 grace 동안 LastKnownTargetLocation을 사용하는 계약과 정합성을 유지한다.

LostGrace 진입 원인은 두 종류로 분리한다.

```text
A. Target Actor reference는 유효하지만 Tracking Cone 이탈
B. Target Actor reference 자체가 무효/Destroyed
```

A에서는 같은 Snapshot Target이 `TrackingConeHalfAngleDeg` 안으로 다시 들어오면:

```text
→ Tracking
```

B에서는 더 이상 같은 Actor를 재관측할 수 없으므로 grace 동안 마지막 관측/추정 상태만 사용하고 Tracking 복귀 판정을 하지 않는다.

LostGrace 동안 실제 Actor를 매 frame 별도 관측하는지 여부는 `TargetObservationMode`를 그대로 따른다.
`SampledPositionEstimate`는 새 관측 주기가 오기 전까지 Estimated Target State만 사용한다.

유예 종료 시:

```text
A. Target reference valid + ReacquisitionMode=None
→ LostFinal

A. Target reference valid + Reacquisition enabled
→ Reacquiring

B. Target reference invalid/Destroyed
→ LostFinal
→ Reacquiring 금지
```

`TargetLostGraceTimeSeconds <= 0`이면 LostGrace 상태에 한 Simulation Step도 의도적으로 머물지 않고 같은 `AdvanceGuidanceSimulation()` 호출 안에서 다음 상태를 결정한다.

### 9.5 Reacquiring

선택적 재포착 상태다.

첫 구현 Product Enum:

```text
ECFMissileReacquisitionMode

None
ForwardCone
```

`WideCone`, `DataLinkAssisted`는 미래 설계 후보일 뿐 첫 Product Enum에 예약값으로 넣지 않는다.
필요해질 때 append-only Migration 검수 후 추가한다.

재포착은 **기존 `GuidanceTargetActor` Snapshot과 같은 Actor만** 대상으로 한다.
주변의 다른 Actor를 검색해 자동 Retarget하지 않는다.

재포착 성공:

```text
same Snapshot Target valid
AND
Target angle <= ReacquisitionConeHalfAngleDeg
→ Tracking
```

재포착 실패:

```text
ReacquisitionTimeSeconds 경과
→ LostFinal
```

`ReacquisitionTimeSeconds <= 0`이면 `Reacquiring` 상태에 한 Simulation Step도 의도적으로 머물지 않고 바로 `LostFinal`로 전환한다.

재포착은 Missile의 현재 물리 방향을 순간적으로 Target 쪽으로 돌리지 않는다.
Target을 다시 Guidance Input으로 허용할 뿐이며 기존 최대 선회율·횡가속도 한계를 그대로 지킨다.

Overshoot는 실제로 Target을 향해 접근한 적이 있는 Stateful 미사일에서만 종결 Miss로 arm한다.
발사 시점부터 Target이 뒤에 있거나 Course Capture로 U-turn 중인 상태를 Overshoot로 오인하면 안 된다.

```text
초기값:
bOvershootArmed = false

Stateful Tracking 중 한 번이라도
Target State 기준 거리 감소
AND
Missile 진행 방향의 Target 쪽 ForwardClosingVelocity > 0
→ bOvershootArmed = true

bOvershootArmed == true
AND
Target State 기준 거리 증가
AND
ForwardClosingVelocity <= 0
→ Overshoot
→ LostFinal
→ Reacquisition 시도하지 않음
```

즉 뒤쪽 Target을 처음 획득한 미사일은 `bOvershootArmed=false`인 동안 물리 제한형 U-turn/Course Capture를 수행할 수 있다.
한 번 실제 접근 기하를 형성한 뒤 Target을 지나쳤을 때만 Overshoot가 Reacquisition보다 우선한다.
`SampledPositionEstimate`에서는 거리와 ForwardClosingVelocity도 실제 Actor의 숨은 위치가 아니라 Estimated Target State를 기준으로 판정한다.

기존 `LegacySingleGate`의 이미 구현된 Overshoot 의미는 호환 경로로 보존한다.
Approach-Armed Overshoot는 신규 Stateful 경로의 current 계약이다.

### 9.6 LostFinal

현재 미사일이 더 이상 해당 Target Actor를 Seeker Tracking/Reacquisition하지 않는 최종 상태다.

이후 행동은 `LostTargetPolicy`가 결정한다.

```text
ContinueStraight
HoldLastKnownPoint
Expire
```

`HoldLastKnownPoint`는 마지막으로 확보한 **고정 지점**을 향한 제한형 Guidance를 계속할 수 있지만 Target Actor Tracking을 재개하는 것은 아니다.
`ContinueStraight` 미사일이 수명이 남았다고 180도 U-turn해 목표를 계속 찾지 않는다.

---

## 10. 기본 Runtime 흐름

```text
Launch Target Snapshot
→ FlightComp가 ElapsedFlightTime / DistanceFromRelease / GuidanceWindow 상태 갱신
→ GuideComp Guidance Activation 판정
    ├─ FollowFlightGuidanceWindow → FlightComp::IsGuidanceWindowOpen()
    └─ Independent → DelaySatisfied AND DistanceSatisfied → activation latch
→ Seeker Acquiring
    ├─ Target reference invalid/destroyed → LostFinal
    ├─ inside Acquisition cone → Tracking
    └─ outside Acquisition cone → Acquiring 유지 / Guidance Command 없음

Tracking
→ observation model
→ Sensor State 갱신
→ Guidance Law가 GuidanceAimPoint/Command 계산
    ├─ PurePursuit → LastObservedTargetLocation pursuit
    ├─ LeadPursuit → bounded lead point pursuit
    └─ ProportionalNavigation
         ├─ non-closing/rear geometry → bounded Course Capture
         └─ approach geometry → PN
→ guidance response filter
→ turn-rate/lateral-accel limited velocity direction change
→ Stateful은 실제 접근이 성립한 뒤에만 bOvershootArmed=true

Tracking target leaves Tracking cone
→ LostGrace
    ├─ Target reference valid + 같은 Snapshot Target이 Tracking cone 안으로 복귀 → Tracking
    ├─ Target reference invalid/destroyed + grace expired → LostFinal
    └─ Target reference valid + grace expired
         ├─ ReacquisitionMode=None → LostFinal
         └─ ReacquisitionMode=ForwardCone → Reacquiring

Reacquiring
    ├─ same Snapshot Target enters effective reacquisition cone → Tracking
    ├─ Target reference invalid/destroyed → LostFinal
    └─ reacquisition time exhausted → LostFinal
```

Legacy 모델은 이 새 상태 흐름을 강제로 사용하지 않는다.

`SeekerModel`과 `TargetObservationMode`는 서로 독립된 축이다.

```text
LegacySingleGate + DirectActorKinematics      // 기존 호환 기본값
LegacySingleGate + SampledPositionEstimate    // 기존 단일 각도 + 제한된 관측 성능
Stateful + DirectActorKinematics              // 상태형 Seeker + 완전 Actor 관측
Stateful + SampledPositionEstimate            // 상태형 Seeker + 제한된 관측 성능
```

Runtime에서 특정 조합만 허용하도록 암묵적으로 결합하지 않는다.

---

## 11. 성능 Variant 작성 방식

Runtime에 `Low/Medium/High` 분기 코드를 넣지 않는다.
다른 `UCFProjectileData`가 다른 수치를 가지게 한다.

예시 값은 튜닝 출발점일 뿐 고정 스펙이 아니다.

### 11.1 저가형 / 초기형 예시

```text
SeekerModel = Stateful
TargetObservationMode = SampledPositionEstimate
TargetObservationIntervalSeconds = 0.15
TargetVelocityEstimateResponseTimeSeconds = 0.35
GuidanceResponseTimeSeconds = 0.30
MaximumTurnRateDegPerSec = 25
MaximumLateralAccelerationCmPerSecSq = 낮음
AcquisitionConeHalfAngleDeg = 25
TrackingConeHalfAngleDeg = 35
TargetLostGraceTimeSeconds = 0.10
ReacquisitionMode = None
```

예상 체감:

```text
- 목표 방향 전환에 늦게 반응
- 측면 기동에 쉽게 Tracking 상실
- 한 번 놓치면 거의 그대로 Miss
- 쉬운 정면 표적은 정상적으로 맞을 수 있음
```

### 11.2 일반형 예시

```text
SeekerModel = Stateful
TargetObservationMode = SampledPositionEstimate
TargetObservationIntervalSeconds = 0.08
TargetVelocityEstimateResponseTimeSeconds = 0.20
GuidanceResponseTimeSeconds = 0.20
MaximumTurnRateDegPerSec = 45~55
MaximumLateralAccelerationCmPerSecSq = 중간
AcquisitionConeHalfAngleDeg = 35
TrackingConeHalfAngleDeg = 60~65
TargetLostGraceTimeSeconds = 0.35~0.50
ReacquisitionMode = ForwardCone
ReacquisitionConeHalfAngleDeg = 35~45
ReacquisitionTimeSeconds = 짧음
```

### 11.3 고급형 예시

```text
SeekerModel = Stateful
TargetObservationMode = SampledPositionEstimate
TargetObservationIntervalSeconds = 0.03
TargetVelocityEstimateResponseTimeSeconds = 0.08~0.12
GuidanceResponseTimeSeconds = 0.10~0.15
MaximumTurnRateDegPerSec = 70~90
MaximumLateralAccelerationCmPerSecSq = 높음
AcquisitionConeHalfAngleDeg = 45~50
TrackingConeHalfAngleDeg = 80~85
TargetLostGraceTimeSeconds = 0.75~1.00
ReacquisitionMode = ForwardCone 또는 후속 WideCone
ReacquisitionConeHalfAngleDeg = 60 이상
ReacquisitionTimeSeconds = 중간~김
```

고급형도 다음 때문에 명중을 보장하지 않는다.

```text
- 발사 기하가 너무 나쁨
- 목표가 물리 선회 한계보다 강하게 기동
- 오버슈트
- 추진 에너지 부족
- 장애물
- 요격
- LifeTime 종료
```

---

## 12. 정지 표적과 Miss 판정 교정

정지 표적에 명중하지 않았다는 사실만으로 Product defect로 판정하지 않는다.

### 12.1 반드시 명중 가능한 기준 시나리오

자동 검증용 명확한 조건을 별도로 둔다.

```text
- Target이 정면
- Acquisition/Tracking cone 안
- 충분한 LifeTime
- 충돌을 막는 장애물 없음
- Projectile collision contract 정상
- 발사 경로가 Target collision volume을 직접 교차
```

이 조건에서 실제 Projectile이 Target을 통과하고 LifeExpired가 되면 충돌/비행 회귀 후보로 본다.

### 12.2 정상 Miss가 가능한 정지 표적 시나리오

```text
- 발사 방향이 나쁨
- Target 접근 중 과도한 가속으로 선회 반경 확대
- Target을 지나침
- Target이 현재 진행 방향 뒤쪽으로 넘어감
- Tracking cone 이탈
- Reacquisition이 없는 Missile
```

이 경우:

```text
Target Lost / Overshoot / ContinueStraight / LifeExpired
```

은 정상 결과가 될 수 있다.

따라서 테스트 이름도 단순 `StaticTargetMustHit`가 아니라 **교전 기하와 기대 결과를 명시**한다.

---

## 13. Terminal Phase 경계

현재 `Terminal` 상태와 설정 필드는 Foundation에 존재하지만 Direct Runtime에서 실제 Terminal 상태 전환은 구현되지 않았다.

이번 설계 교정은 Terminal을 명중률 보정용으로 즉시 활성화하지 않는다.

```text
금지:
Target에 가까워지면 방향을 강제로 Target에 맞춤
Target 근처에서 Hit를 보장
Overshoot 직전 순간 회전
```

후속 Terminal 구현이 필요하면 다음 중 실제 목적을 먼저 정의한다.

```text
- 종말 센서 갱신률 변화
- 종말 기동 한계 변화
- TopAttack 하강
- 근접신관
- 별도 종말 유도 Law
```

그리고 동일한 물리 제한과 실제 충돌 계약을 유지한다.

---

## 13.1 MG-P0-08 Source/API 구현 계약

2026-09-07 current Source와 대조한 구현 경계다.

### 변경 Owner

```text
CFMissileGuideTypes.h
- 신규 Config enum/필드
- Seeker runtime enum
- Guide Snapshot 확장
- 기존 enum 순서/값 보존

CFMissileGuideComp.h / .cpp
- Legacy/New 모델 dispatch
- Stateful Seeker 상태
- Sampled observation/velocity estimator 상태
- Reset / Snapshot / Summary
```

### 기본적으로 변경하지 않는 Owner

```text
CFMissileGuideMath.h / .cpp
- 이미 TargetLocation + TargetVelocityEstimate를 순수 입력으로 받음
- PN / turn-rate / lateral-accel 계산 계약 유지

CFProjectileLaunchTypes.h
- GuidanceTargetActor Snapshot 계약 유지
- 신규 Target 또는 Retarget 필드 추가하지 않음

CFProjectileActor.h / .cpp
- StartMissileGuidance / ResetMissileGuidance 호출 경계 유지
- 기존 Pool / Collision / Damage lifecycle 유지

CFProjectileData.h / .cpp
- MissileGuideConfig 중첩 구조와 GetEffectiveMissileGuideConfig() public API 유지
- 신규 성능 축은 FCFMissileGuideConfig 내부 확장으로 제공
```

### Public API 보존

다음 `UCFMissileGuideComp` public signature는 변경하지 않는다.

```text
StartMissileGuidance(...)
ResetMissileGuidance()
GetGuidanceTargetActor()
GetGuideSnapshot()
GetGuidanceCommand()
BuildGuidanceSummary()
AdvanceGuidanceForAutomation(float)
```

신규 Debug용 public getter를 개별적으로 늘리지 않는다.
관측 가능한 신규 상태는 기존 `FCFMissileGuideSnapshot` 확장을 우선한다.

현재 private `TryResolveTargetObservation(...) const`는 Sampled 상태를 갱신해야 하므로 내부 signature 변경을 허용한다.
권장 내부 분리는 다음과 같다.

```text
TryResolveTargetObservation(DeltaTime, OutLocation, OutVelocity)
├─ ResolveDirectActorObservation(...)
└─ AdvanceSampledTargetObservation(DeltaTime, ...)
```

private 구현 세부이며 Product public API 변경으로 보지 않는다.

### 신규 Config 기본값과 Clamp 계약

```text
SeekerModel = LegacySingleGate
TargetObservationMode = DirectActorKinematics
ReacquisitionMode = None

TargetObservationIntervalSeconds = 0.08
TargetVelocityEstimateResponseTimeSeconds = 0.20
AcquisitionConeHalfAngleDeg = 30
TrackingConeHalfAngleDeg = 60
ReacquisitionConeHalfAngleDeg = 45
ReacquisitionTimeSeconds = 0.50
```

Effective clamp:

```text
TargetObservationIntervalSeconds: 0.001 .. 10.0
TargetVelocityEstimateResponseTimeSeconds: 0.001 .. 10.0
AcquisitionConeHalfAngleDeg: 0 .. 180
TrackingConeHalfAngleDeg: 0 .. 180
ReacquisitionConeHalfAngleDeg: 0 .. 180
ReacquisitionTimeSeconds: 0 .. 30
```

Stateful effective angle invariant:

```text
EffectiveTrackingConeHalfAngleDeg
= clamp(TrackingConeHalfAngleDeg, 0, 180)

EffectiveAcquisitionConeHalfAngleDeg
= min(clamp(AcquisitionConeHalfAngleDeg, 0, 180), EffectiveTrackingConeHalfAngleDeg)

EffectiveReacquisitionConeHalfAngleDeg
= min(clamp(ReacquisitionConeHalfAngleDeg, 0, 180), EffectiveTrackingConeHalfAngleDeg)
```

즉 Tracking으로 전환할 수 있는 Acquisition/Reacquisition 범위가 Tracking 유지 범위보다 넓어서 다음 Tick에 즉시 LostGrace로 떨어지는 상태 진동을 만들지 않는다.

Legacy 모델에서는 Stateful 전용 Angle/Reacquisition 값이 현재 행동에 영향을 주지 않는다.
DirectActorKinematics에서는 observation interval/velocity-estimator response 값이 현재 행동에 영향을 주지 않는다.

### Enum/직렬화 보호

기존 `ECFMissileGuideMode`, `ECFMissileLostTargetPolicy`, `ECFMissileMissReason`의 기존 항목 순서를 변경하지 않는다.
MG-P0-08~10에서는 상태 세부를 `ECFMissileSeekerState`와 Snapshot으로 표현하고, 기존 `ECFMissileMissReason`에 새 값을 끼워 넣지 않는다.
향후 MissReason 확장이 필요하면 기존 마지막 항목 뒤 append-only로 별도 Migration 검수한다.

첫 구현의 `ECFMissileReacquisitionMode`는 실제 지원하는 값만 노출한다.

```text
None
ForwardCone
```

`WideCone`, `DataLinkAssisted`는 구현 전 enum에 예약값처럼 먼저 노출하지 않는다.

### Start / Guidance Window 계약

`StartMissileGuidance()`의 기존 Target Snapshot 초기화 경계는 유지한다.
Sampled mode는 Start 시 유효 Target 위치를 최초 estimator seed로 저장한다.
단 이것은 Seeker 획득 완료를 의미하지 않는다.

```text
StartMissileGuidance
→ SeekerState = Inactive
→ 최초 Sample seed 저장
→ ObservationAgeSeconds = 0

Guidance Window Closed
→ Steering 없음
→ Seeker 상태 전이 없음
→ 추가 Actor sample 없음
→ ObservationAgeSeconds만 실제 DeltaTime만큼 누적

Guidance Window Open
→ Stateful은 Acquiring 시작
→ Sampled mode에서 누적 시간이 ObservationInterval 이상이면 이번 Guidance Tick에 실제 Actor 위치를 최대 1회 Sample
→ 발사 시 seed와 새 Sample 사이 실제 누적 시간으로 velocity estimate 계산
```

따라서 Clearance 시간이 길어도 Guidance Window가 열린 뒤 다시 전체 ObservationInterval을 추가 대기하지 않는다.
반대로 Guidance Window가 닫힌 동안 TargetActor를 주기적으로 샘플링해 발사 직후부터 완벽한 추적 history를 축적하지도 않는다.

### LostTargetPolicy 실행 경계

`LostFinal`은 더 이상 Target Actor Tracking/Reacquisition을 하지 않는 Guide 상태다.

```text
ContinueStraight
→ Guidance Command 없음
→ 현재 Projectile 물리 비행과 기존 LifeTime 유지

HoldLastKnownPoint
→ LostFinal 진입 시 그 순간 Guide가 소비하던 Sensor Truth 위치를 HoldTargetLocation으로 1회 Freeze
→ DirectActorKinematics에서는 마지막 유효 직접 관측 위치
→ SampledPositionEstimate에서는 LostFinal 진입 순간의 EstimatedTargetLocation
→ Freeze 이후 Target velocity estimate와 ObservationAge를 더 이상 위치 이동에 사용하지 않음
→ 해당 고정 지점만 bounded Guidance 입력으로 사용 가능
→ Target Actor observation / Tracking / Reacquisition은 재개하지 않음

Expire
→ MG-P0-08~10에서는 Guidance 종료 + MissReason=LifeExpired 기록
→ ACFProjectileActor 즉시 비활성화 호출은 하지 않음
→ 실제 Projectile 수명 종료/Pool 반환은 기존 Projectile LifeTime timer가 계속 소유
```

`Expire`를 즉시 Deactivate로 바꾸는 작업은 현재 no-touch `ACFProjectileActor` 경계를 깨므로 이번 단계에 포함하지 않는다.
실제 즉시 만료 semantics가 필요하면 별도 Projectile↔Guide termination 계약으로 검수한다.

### Snapshot 계약

기존 `FCFMissileGuideSnapshot` 필드를 삭제·리네임하지 않는다.
`TargetLocation`의 기존 의미도 Legacy 호환을 위해 **마지막으로 Guidance가 보존한 LastKnownTargetLocation**으로 유지한다.

신규 필드:

```text
SeekerModel
SeekerState
TargetObservationMode
LastObservedTargetLocation
EstimatedTargetLocation
FilteredTargetVelocityEstimate
ObservationAgeSeconds
ReacquisitionElapsedTimeSeconds
bHasValidObservation
```

의미:

```text
LastObservedTargetLocation
= Sampled mode에서 실제 Actor에서 마지막으로 취득한 위치

EstimatedTargetLocation
= 현재 Seeker/PN/Overshoot가 소비하는 Sensor Truth 위치

FilteredTargetVelocityEstimate
= Sampled 위치 차분을 필터링한 Target velocity estimate

ObservationAgeSeconds
= 마지막 실제 Actor sample 이후 경과 시간

bHasValidObservation
= 현재 activation에서 유효한 sensor seed/sample을 한 번 이상 확보했는지 여부
```

DirectActorKinematics에서는 `LastObservedTargetLocation`과 `EstimatedTargetLocation`을 현재 직접 관측 위치로 동기화할 수 있다.

기존 `bTargetValid` 의미는 호환을 위해 다음처럼 고정한다.

```text
LegacySingleGate
= 현재처럼 이번 Step의 Actor observation + Seeker gate가 모두 유효할 때 True

Stateful
= 이번 Step에서 Sensor Truth가 현재 Seeker 상태의 Tracking Guidance 입력으로 승인됐을 때 True
= Acquiring / LostGrace fallback / Reacquiring / LostFinal에서는 False
```

Stateful의 임시 대기 상태는 최종 Miss로 기록하지 않는다.

```text
Acquiring에서 cone 밖
LostGrace 진행 중
Reacquiring 진행 중
→ CurrentGuidanceCommand.bCommandValid = false
→ MissReason = None
```

최종 진단은 다음처럼 단순화한다.

```text
LostFinal due target invalid / acquisition·tracking·reacquisition exhaustion
→ MissReason = TargetLost

Overshoot
→ MissReason = Overshoot

LostTargetPolicy=Expire
→ MissReason = LifeExpired
```

기존 `SeekerFieldOfViewExceeded` / `LockBreakAngleExceeded` 진단은 LegacySingleGate 경로에서 현재 의미를 보존한다.
첫 Stateful 구현에서 새 MissReason enum 값을 추가하지 않는다.

개별 Debug public getter를 추가하지 않고 기존 `GetGuideSnapshot()`으로 노출한다.

### DataAsset 편집 노출 계약

신규 UPROPERTY는 사용하지 않는 설정을 동시에 편집하지 않도록 `EditCondition`을 둔다.

```text
Stateful 전용 각도 / ReacquisitionMode
→ bUseGuidance && SeekerModel == Stateful

Observation Interval / Velocity Estimate Response
→ bUseGuidance && TargetObservationMode == SampledPositionEstimate

Reacquisition Cone / Time
→ bUseGuidance && SeekerModel == Stateful && ReacquisitionMode == ForwardCone
```

Tooltip은 각 Angle이 전체 FOV가 아니라 중심선 기준 반각이라는 점을 한글로 명시한다.

### Tick 순서 보존 계약

현재 `ACFProjectileActor`는 이미 다음 prerequisite를 보유한다.

```text
ProjectileMotor
→ MissileFlight
→ MissileGuide
→ ProjectileMovement
```

MG-P0-08~10은 이 Tick architecture를 변경하지 않는다.
신규 observer/seeker 상태도 기존 `MissileGuideComponent` Tick 안에서 진행한다.

### Pool Reset 계약

`ResetMissileGuidance()`는 기존 상태와 함께 다음 신규 상태를 모두 초기화한다.

```text
SeekerState = Inactive
PreviousObservedTargetLocation = Zero
LastObservedTargetLocation = Zero
EstimatedTargetLocation = Zero
FilteredTargetVelocityEstimate = Zero
ObservationAgeSeconds = 0
ReacquisitionElapsedTimeSeconds = 0
HoldTargetLocation = Zero
bHasHoldTargetLocation = false
observation-valid flags = false
```

기존 `GuideActivationCount`만 현재 계약대로 Reset에서 보존한다.

### Test Source 경계

```text
CFMissileFoundationTests.cpp
- 신규 enum/config 기본값
- Effective clamp
- Legacy default compatibility

신규 CFMissileGuideStateTests.cpp
- Stateful Seeker 상태 전이
- Sampled observation / estimator
- Overshoot > Reacquisition 우선순위
- 동일 Snapshot만 Reacquisition
- Pool-style Reset state
```

현재 이미 dirty인 `CFMissileRuntimeTests.cpp v1.3.0`은 MG-P0-08~10의 세부 상태 테스트를 계속 누적하는 기본 위치로 사용하지 않는다.
기존 Fire→Pool→persisted ActorClass / Blocking Contact / Damage evidence를 보호한다.
MG-P0-11 통합 회귀에서 필요한 최소 assertion만 별도 검수 후 추가할 수 있다.

### C++ / DataAsset 책임

```text
C++
- 상태 머신
- 관측 타이머
- Velocity estimator
- 물리 제한 적용
- Reset
- deterministic Automation

DataAsset
- MissileGuideConfig의 개별 성능 수치 선택
- Legacy/Stateful 선택
- Direct/Sampled observation 선택
- Reacquisition 사용 여부 선택

Blueprint Tick
- 신규 Guidance 계산을 중복 구현하지 않음
```

---

## 14. 구현 단계 교정

기존 MG-P0-00~07 Technical evidence는 관련 회귀 증거 없이 다시 열지 않는다.
새 설계는 후속 단계로 추가한다.

```text
MG-P0-08 Guidance Performance Variant Contract
- FCFMissileGuideConfig 신규 축 확정
- ECFMissileSeekerModel / ECFMissileTargetObservationMode 확정
- Legacy compatibility path 확정
- Stateful Seeker API와 Snapshot-only reacquisition 계약 확정
- Low/Normal/High를 하드코딩하지 않는 Data-driven 원칙 확정

MG-P0-09 Seeker State Model
- ECFMissileSeekerState
- Acquisition / Tracking / LostGrace / Reacquisition / LostFinal
- Acquisition/Tracking angle 분리
- ReacquisitionMode=None/ForwardCone
- 기존 Target Snapshot과 LostTargetPolicy 호환
- 기존 DA 무변경 동작 호환 검증

MG-P0-10 Observation + Velocity Estimator
- TargetObservationIntervalSeconds
- 위치 샘플 기반 Target velocity estimate
- TargetVelocityEstimateResponseTimeSeconds
- 관측 사이 EstimatedTargetLocation 외삽
- Sampled mode에서 매 frame TargetActor->GetVelocity() 정답 소비 금지
- deterministic Automation

MG-P0-11 Guidance Variant Verification Matrix
- 저성능/기준/고성능 config fixture
- 같은 Target motion에서 반응 차이 증명
- 쉬운 정면 정지 Target의 실제 충돌 가능성 검증
- Tracking cone 이탈 시 정상 Miss 증명
- Reacquisition None/Enabled 차이 증명
- 물리 한계 및 Pool Reset 회귀

MG-P0-12A USER Guidance Feel Test Setup
- 기존 DA_Missile_DirectTest는 회귀 기준 자산으로 보존하고 직접 덮어쓰지 않음
- persisted Low/Normal/High DataAsset 3벌을 만들지 않고, 기존 DirectTest Projectile/Weapon/Equipment를 PIE Runtime에서 transient 복제해 사용
- transient ProjectileData에서 MissileGuideConfig 성능축만 Variant별로 변경하고 Product Low/Normal/High enum은 만들지 않음
- 실제 UCFVehicleWeaponComp::InitializeWeaponRuntimeFromFitting 경로로 transient EquipmentPreset을 적용해 Fire→Pool→Missile production 경로가 그대로 소비하게 함
- MissileDirectTest PIE에서 Baseline/Low/Normal/High를 반복 전환하는 Editor-only 콘솔 명령 제공
- Baseline 명령은 저장 EQ_Missile_DirectTest를 다시 적용해 transient override를 제거
- 저장 DirectTest Asset, Blueprint, Map은 읽기 전용이며 Content mutation 0
- current UE WriteProbe의 persisted DataAsset mutation 경계와 무관하게 USER 비교가 가능하도록 C++ Editor-only transient 경로를 사용
- 현재 구현·Build·명령 등록/수치 Automation은 PASS, 실제 PIE에서 네 전환 명령을 실행한 Runtime switch readback은 USER/Live validation pending

MG-P0-12 USER Guidance Feel Validation
- MG-P0-12A 실제 PIE 비교 경로가 준비된 뒤에만 USER Gate를 시작
- 저성능/기준/고성능 Missile의 차이가 눈으로 구분되는가
- 기준 Missile이 Target motion을 마법처럼 완벽하게 선행하지 않는가
- 저성능 Missile이 단순 랜덤이 아니라 관측/기동 한계 때문에 자연스럽게 놓치는가
- 고성능 Missile도 물리·Seeker 한계를 넘으면 정상적으로 놓칠 수 있는가
```

---

## 15. 자동 검증 요구사항

기존 `DirectRuntimeContract` 보호 회귀에 다음 성격의 검증을 추가한다.

```text
1. SeekerModel=LegacySingleGate가 기존 Seeker 허용각 동작을 보존
2. TargetObservationMode=DirectActorKinematics가 기존 target observation 동작을 보존
3. Stateful Acquisition cone 밖 유효 Target은 Acquiring 유지 / LostGrace 미진입
4. Acquisition 성공 후 Tracking cone 안에서는 Acquisition cone 밖이어도 Tracking 유지 가능
5. Effective Acquisition/Reacquisition cone은 Tracking cone보다 넓지 않음
6. Tracking cone 이탈 → LostGrace
7. Grace 안에 같은 Snapshot Target 복귀 → Tracking 복원
8. Target Actor Destroyed/invalid는 grace 종료 후 Reacquisition 없이 LostFinal
9. Reacquisition=None에서 grace 종료 후 영구 Actor Tracking 상실
10. Reacquisition=ForwardCone에서 같은 Snapshot Target 재진입 시 Tracking 복원
11. Reacquisition이 다른 Actor로 Retarget하지 않음
12. ReacquisitionTime=0이면 즉시 LostFinal
13. 실제 Overshoot가 Reacquisition보다 우선하며 U-turn 재추적하지 않음
14. Guidance Window 닫힘 동안 observation elapsed만 누적되고 추가 Actor sample/Seeker 전이는 없음
15. Guidance Window가 열릴 때 누적 interval 충족 시 실제 Actor sample을 최대 1회 즉시 취득
16. 관측 갱신 주기가 길수록 Target state update가 늦음
17. 한 Guidance Tick에서 실제 Actor sample을 최대 1회만 취득
18. Sampled mode가 TargetActor->GetVelocity() 정답 없이 위치 샘플로 이동 방향을 추정
19. Sampled mode의 Seeker/Overshoot/PN이 모두 같은 Estimated Target State를 사용
20. 목표가 방향을 바꾼 직후 이전 velocity estimate와 새 관측 사이에 결정론적 추정 오차가 존재
21. Snapshot의 LastObserved/Estimated/VelocityEstimate/ObservationAge가 내부 상태와 일치
22. Sampled observer가 단일 ObservationAgeSeconds clock만 사용해 sample interval과 extrapolation age가 일치
23. Acquiring/LostGrace/Reacquiring 임시 상태의 MissReason=None과 bTargetValid=false가 유지
24. HoldLastKnownPoint가 LostFinal 진입 순간 Sensor Truth 위치를 1회 Freeze하고 이후 외삽하지 않음
25. ContinueStraight/HoldLastKnownPoint/Expire의 Guide-level 의미가 계약대로 분리됨
26. 동일 정지 오프축 교전과 동일 이동→반전 Target motion에서 Low/Normal/High config의 관측·속도 추정·Applied Guidance response 차이 존재
27. 모든 variant가 MaximumTurnRate / MaximumLateralAcceleration 한계를 넘지 않음
28. 쉬운 정면 정지 Target collision path는 실제 Blocking 접촉 가능
29. 잘못된 발사 기하에서 Tracking 상실/오버슈트/LifeExpired가 정상 결과가 될 수 있음
30. Pool 재사용에서 Seeker State / observation samples / estimator / HoldTargetLocation 상태 완전 Reset
```

랜덤 노이즈를 첫 구현에 넣지 않으므로 자동 테스트는 결정론적으로 유지한다.

---

## 16. USER 검증 기준

최종 USER Gate는 단순히 "잘 맞는가"가 아니다.

### 16.1 성능 차이

```text
같은 목표와 같은 발사 조건에서
저가형 / 일반형 / 고급형의 반응 차이가 육안으로 납득되는가
```

### 16.2 자연스러운 Miss

```text
놓쳤을 때 랜덤 판정 때문에 빗나간 느낌이 아니라
관측 지연, 선회 한계, Seeker 상실 또는 오버슈트 때문에 놓쳤다고 보이는가
```

### 16.3 고급형의 위협성

```text
고급형은 단순 좌우 이동만으로 쉽게 피하기 어렵지만
물리적으로 불가능한 180도 U-turn이나 순간 방향 전환으로 명중을 강제하지 않는가
```

### 16.4 기준 Missile의 현실감

```text
Target motion을 알고 즉시 미래 위치로 끌려가는 느낌이 줄어들었는가
```

---

## 17. 제외 범위

이번 설계 교정에서 구현하지 않는 항목:

```text
- Equipment Lock-On 진행/UI
- Radar/Scanner 품질에 따른 실시간 DataLink 정확도
- DataLinkAssisted Reacquisition Runtime
- 랜덤 센서 노이즈
- ECM / Jammer / Flare
- 실제 IR/Radar seeker 물리 모델
- 6-DOF 공력 시뮬레이션
- 근접신관
- 명중 확률 직접 지정
- Terminal 강제 명중 보정
```

---

## 18. 설계 검수

### v0.1.0 초안 검수

```text
P0 = 0
P1 = 1
P2 = 0
```

P1:

```text
신규 Acquisition/Tracking 반각 필드가 "기존 Asset에서는 옛 FOV 값에서 자동 파생"된다고만 정의되어 있었으나,
UE 신규 UPROPERTY의 직렬화 기본값만으로 기존 Asset이 신규 필드를 명시 저장했는지 안정적으로 판별할 수 없다.
```

교정:

```text
- ECFMissileSeekerModel 추가 설계
  - LegacySingleGate 기본값
  - Stateful 선택형
- ECFMissileTargetObservationMode 추가 설계
  - DirectActorKinematics 기본값
  - SampledPositionEstimate 선택형
- 기존 저장 DA는 재저장 없이 Legacy 경로 유지
- 신규/교정 DA만 Stateful + Sampled mode 선택
```

추가 검수에서 확정한 경계:

```text
- Reacquisition은 같은 Launch Target Snapshot만 허용하고 자동 Retarget 금지
- Sampled mode는 관측 사이 마지막 관측 위치 + filtered velocity estimate로 estimated target state를 구성
- 최초 두 번째 샘플 전 Velocity Estimate는 0에서 시작해 완벽한 초기 속도 정답을 주지 않음
- Equipment Lock-On과 Missile Seeker Acquisition은 분리
- Terminal은 이번 교정의 명중 보정 수단이 아님
```

### v0.1.1 교정 후 재검수

```text
P0 = 0
P1 = 0
P2 = 0

PASS
```

### v0.1.2 MG-P0-08 Current Source/API 계약검수

Current Source 대조:

```text
- CFMissileGuideMath는 이미 TargetLocation / TargetVelocityEstimate 순수 입력 구조라 재설계 불필요
- UCFMissileGuideComp가 Actor observation, Seeker gate, LostGrace, Overshoot, response filter를 소유
- ACFProjectileActor는 기존 StartMissileGuidance / ResetMissileGuidance integration 경계로 충분
- UCFProjectileData는 FCFMissileGuideConfig 중첩 + GetEffectiveMissileGuideConfig() 경계로 충분
- FCFProjectileLaunchContext는 GuidanceTargetActor Snapshot을 이미 소유하며 GuidanceTargetLocation 별도 필드는 현재 Source에 없음
- current CFMissileRuntimeTests.cpp는 v1.3.0 CF-FQ-030 correction dirty를 이미 소유
```

초기 Source/API 검수:

```text
P0 = 0
P1 = 3
P2 = 2
```

P1 교정:

```text
P1-1 Reacquisition과 기존 Overshoot의 우선순위가 미정
→ Overshoot를 Reacquisition보다 우선하는 terminal miss로 고정

P1-2 Sampled mode가 Guidance만 estimate를 쓰고 Seeker/Overshoot에서 실제 Actor 위치를 다시 읽을 여지가 있음
→ Estimated Target State를 Sampled mode의 단일 Sensor Truth로 고정

P1-3 기존 public API / serialized enum / shared integration owner 변경 경계가 미정
→ GuideComp public signature 보존, 기존 enum 순서 보존, Math/ProjectileActor/LaunchContext/ProjectileData API no-touch 기본 경계 확정
```

P2 교정:

```text
P2-1 low FPS에서 한 Tick에 여러 synthetic sample을 만들 수 있는 accumulator 의미 미정
→ 실제 Actor sample은 Guidance Tick당 최대 1회, 누적 실제 elapsed time으로 velocity estimate 계산

P2-2 기존 dirty RuntimeTests에 상태형 테스트를 계속 누적할 가능성
→ Foundation config test + 신규 CFMissileGuideStateTests.cpp로 분리하고 RuntimeTests v1.3.0 evidence 보호
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
MG-P0-08 Source/API Contract Review = PASS
Implementation Gate = OPEN
```

### v0.1.3 구현 직전 독립 설계검수 + 교정

v0.1.2를 current Source와 다시 대조한 독립 설계검수 결과:

```text
P0 = 0
P1 = 6
P2 = 2

CORRECTION REQUIRED
```

교정:

```text
P1-1 Acquiring 문구 충돌
→ Acquisition cone 밖의 유효 Target은 Acquiring 유지, LostGrace 미진입으로 단일화

P1-2 Destroyed Target과 단순 Tracking cone 이탈의 LostGrace/Reacquisition 혼합
→ invalid/destroyed Target은 grace 이후 무조건 LostFinal, Reacquisition 금지

P1-3 Acquisition/Reacquisition cone이 Tracking cone보다 넓을 때 즉시 상태 진동 가능
→ effective Acquisition/Reacquisition cone을 Tracking cone 이하로 제한

P1-4 Start sample과 Guidance Window 사이 observation 시간 의미 미정
→ Window 닫힘 동안 elapsed만 누적, 추가 sample 없음, Window open 시 interval 충족이면 최대 1회 즉시 sample

P1-5 LostTargetPolicy=Expire가 current no-touch ProjectileActor 경계와 충돌
→ MG-P0-08~10에서는 Guide 종료 + LifeExpired 진단만 소유, 실제 Projectile 만료는 기존 LifeTime timer 소유

P1-6 Snapshot에서 LastObserved / Estimated / Hold point 의미 미정
→ 기존 TargetLocation 호환 의미 보존 + 신규 observer/seeker snapshot 필드 exact-list 확정

P2-1 Reacquisition enum에 구현하지 않을 WideCone/DataLinkAssisted가 문서 일부에 잔존
→ 첫 Product enum은 None/ForwardCone만 노출

P2-2 0초 시간값 / DataAsset EditCondition 의미 미정
→ zero-time same-step 전이와 모델별 EditCondition 계약 확정
```

추가 Source 확인:

```text
ACFProjectileActor current Tick prerequisite:
ProjectileMotor → MissileFlight → MissileGuide → ProjectileMovement

→ 기존 Tick architecture는 충분하며 MG-P0-08~10에서 변경하지 않음
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
GuidancePerformanceDesign v0.1.3 = Correction Complete
→ final implementation-readiness re-review required
```

---

### v0.1.4 최종 구현 준비 재검수

v0.1.3 전체 교정 후 문서와 current Source를 다시 대조하면서 구현 결과가 갈릴 수 있는 잔여 모호성 3건을 추가 확인했다.

```text
P0 = 0
P1 = 2
P2 = 1
```

추가 교정:

```text
P1-1 ObservationElapsedTimeSeconds와 TimeSinceLastObservationSeconds가 동일 의미로 중복돼 estimator sample 시간과 extrapolation 시간이 어긋날 수 있음
→ 내부 시간을 ObservationAgeSeconds 단일 clock으로 통합

P1-2 HoldLastKnownPoint가 무엇을 Freeze하는지 정확한 위치 출처가 미정
→ LostFinal 진입 순간의 Sensor Truth 위치를 HoldTargetLocation으로 1회 Freeze하고 이후 외삽 금지

P2-1 Stateful 임시 상태의 bTargetValid / MissReason debug 의미가 미정
→ Acquiring/LostGrace/Reacquiring은 bTargetValid=false, MissReason=None. 최종 LostFinal은 TargetLost, Overshoot/LifeExpired는 기존 사유 사용
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
GuidancePerformanceDesign v0.1.4 = Final Implementation-Ready
→ Pool reuse state completeness final check required
```

---

### v0.1.5 Pool Reuse State Completeness 최종 교정

v0.1.4 최종 재검수 뒤 새로 확정한 `HoldTargetLocation`이 Pool Reset exact-list에 포함되지 않은 것을 확인했다.

```text
P0 = 0
P1 = 1
P2 = 0
```

교정:

```text
P1-1 HoldLastKnownPoint가 HoldTargetLocation을 activation 간 내부 상태로 소유하지만 Reset 계약에서 누락
→ ResetMissileGuidance에서 HoldTargetLocation=Zero, bHasHoldTargetLocation=false를 강제
→ Pool 재사용 미사일이 이전 activation의 Hold point를 재사용할 가능성을 제거
→ Automation 요구사항 30에도 HoldTargetLocation Reset을 명시
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
GuidancePerformanceDesign v0.1.5 = Final Implementation-Ready
MG-P0-08 Types / Config Foundation Gate = OPEN
```

### v0.1.6 MG-P0-08 Types / Config Foundation 구현 체크포인트

설계 v0.1.5의 구현 Gate에 따라 Runtime 행동을 바꾸지 않는 Foundation만 먼저 구현했다.

구현 Owner:

```text
UE/Source/CarFight_Re/Public/CFMissileGuideTypes.h v1.1.0
UE/Source/CarFight_Re/Private/CFMissileFoundationTests.cpp v1.1.0
```

구현 범위:

```text
신규 UENUM
- ECFMissileSeekerModel: LegacySingleGate / Stateful
- ECFMissileTargetObservationMode: DirectActorKinematics / SampledPositionEstimate
- ECFMissileSeekerState: Inactive / Acquiring / Tracking / LostGrace / Reacquiring / LostFinal
- ECFMissileReacquisitionMode: None / ForwardCone

FCFMissileGuideConfig 신규 필드
- SeekerModel = LegacySingleGate
- TargetObservationMode = DirectActorKinematics
- ReacquisitionMode = None
- TargetObservationIntervalSeconds = 0.08
- TargetVelocityEstimateResponseTimeSeconds = 0.20
- AcquisitionConeHalfAngleDeg = 30
- TrackingConeHalfAngleDeg = 60
- ReacquisitionConeHalfAngleDeg = 45
- ReacquisitionTimeSeconds = 0.50

Effective Config
- Observation interval 0.001..10
- Velocity estimate response 0.001..10
- Tracking cone 0..180
- Acquisition effective cone <= Tracking cone
- Reacquisition effective cone <= Tracking cone
- Reacquisition time 0..30
- non-finite 값 fail-safe 보정

FCFMissileGuideSnapshot 신규 Foundation 필드
- SeekerModel / SeekerState / TargetObservationMode
- LastObservedTargetLocation / EstimatedTargetLocation
- FilteredTargetVelocityEstimate
- ObservationAgeSeconds / ReacquisitionElapsedTimeSeconds
- bHasValidObservation
```

호환 경계:

```text
- 기존 ECFMissileGuideMode / ECFMissileLostTargetPolicy / ECFMissileMissReason 순서 변경 0
- 기존 SeekerFieldOfViewDeg / LockBreakAngleDeg 삭제·리네임 0
- 기존 UCFProjectileData public API 변경 0
- 기존 UCFMissileGuideComp Runtime 변경 0
- 기존 CFMissileGuideMath 변경 0
- 기존 ACFProjectileActor 변경 0
- 기존 CFMissileRuntimeTests.cpp v1.3.0 변경 0
- Content Asset / Blueprint / Map 저장 변경 0
```

중요:

```text
MG-P0-08은 새 enum/config/snapshot을 사용할 수 있게 만든 Foundation이다.
Stateful Seeker의 실제 상태 전이, Sampled observer와 velocity estimator는 아직 Runtime에 연결하지 않았다.
따라서 현재 저장 Missile은 기본값 LegacySingleGate + DirectActorKinematics로 기존 행동을 유지한다.
Stateful/Sampled 값을 Product DA에 적용하는 것은 MG-P0-09~10 Technical PASS 전까지 금지한다.
```

검증:

```text
Focused Automation:
CarFight.Missile.MG_P0_08.ConfigFoundation
→ Success 1 / Failure 0
→ Process 02c4f2a145454c36a6f59d0b5032dcba
→ Engine Exit Code 0

Missile affected regression:
CarFight.Missile filter
→ Success 3 / Failure 0
→ Process 783cd387684a4a1fa364de3535587ee3
→ Engine Exit Code 0
→ PASS MG_P0_00.FoundationContract
→ PASS MG_P0_01_04.DirectRuntimeContract
→ PASS MG_P0_08.ConfigFoundation
```

공식 Editor Build:

```text
Job b2cfa60513ec4fd6af5467722b697ba1
- canonical carfight.editor.development
- D:\UnrealEngine_Source
- CarFight_ReEditor Win64 Development
- UHT 진입 확인
- later canonical interaction에서 exact Job receipt 회수 완료: Result Succeeded / Exit Code 0
```

새 `MG_P0_08.ConfigFoundation`이 실제 UE process에서 발견·실행되어 Success였고, canonical Build Job terminal receipt도 Result Succeeded / Exit Code 0으로 회수됐다.
UHT 5 generated files, CFMissileFoundationTests.cpp / CFMissileRuntimeTests.cpp compile, UnrealEditor-CarFight_Re.dll link까지 모두 PASS했다.

### Historical Checkpoint — MG-P0-10 완료 당시 판정

아래 상태는 v0.1.9 / MG-P0-10 완료 당시 기록이며 현재 checkpoint가 아니다.

```text
MG-P0-08 Source Implementation = Complete
MG-P0-08 Focused + Missile Regression Automation = PASS
Official Build Terminal Receipt = PASS / Exit Code 0
MG-P0-09 Seeker State Model Runtime = Technical PASS
MG-P0-10 Observation + Velocity Estimator = Technical PASS
MG-P0-11 Guidance Variant Verification Matrix = Ready (당시)
```

---

## 19. 설계 완료 조건

당시 v0.1.9는 다음 조건을 만족했다.

```text
- P0/P1/P2 설계 결함 0
- Equipment Lock과 Missile Seeker Acquisition 소유권 혼동 0
- 기존 ProjectileData 저장 호환 경로 명시
- LegacySingleGate / Stateful 모델 선택 명시
- DirectActorKinematics / SampledPositionEstimate 관측 모델 선택 명시
- HitChance 같은 직접 명중률 필드 없음
- Low/Normal/High Runtime 하드코딩 없음
- Acquisition/Tracking/Reacquisition 각도 의미가 반각으로 명확함
- Observation update와 Guidance response가 별도 성능 축으로 분리됨
- Reacquisition 자동 Retarget 금지
- 정상 Hit와 정상 Miss 검증 조건이 모두 정의됨
- 기존 MG-P0-00~07 Technical PASS를 이유 없이 다시 열지 않음
- Sampled mode의 Sensor Truth 단일성 보장
- Overshoot가 Reacquisition보다 우선
- 기존 GuideComp public signature와 shared Projectile integration 경계 보존
- 기존 serialized enum 항목 순서 보존
- 상태/Estimator 전용 테스트를 기존 Runtime integration evidence와 분리
- Acquiring cone 밖 유효 Target의 상태 의미 단일화
- Destroyed Target의 Reacquisition 금지
- Acquisition/Reacquisition effective cone이 Tracking cone 이하임을 보장
- Clearance 중 observation elapsed 의미와 Window-open 첫 sample 시점 확정
- LostTargetPolicy=Expire의 Guide-level 진단과 Projectile lifetime ownership 분리
- Snapshot의 LastObserved / Estimated / VelocityEstimate 의미 확정
- 첫 Reacquisition Product enum은 None/ForwardCone만 노출
- zero-time 상태 전이와 DataAsset EditCondition 확정
- 기존 Motor→Flight→Guide→ProjectileMovement Tick prerequisite 보존
- Sampled observer sample interval과 extrapolation이 ObservationAgeSeconds 단일 clock을 공유
- HoldLastKnownPoint가 LostFinal 순간 Sensor Truth 위치를 1회 Freeze하고 이후 외삽하지 않음
- Stateful 임시 상태와 최종 Miss의 bTargetValid/MissReason 의미 확정
- HoldTargetLocation과 bHasHoldTargetLocation이 Pool Reset exact-list에 포함됨
```

Historical 판정:

```text
GuidancePerformanceDesign v0.1.9 MG-P0-10 구현 후 재검수 PASS
→ 당시 MG-P0-11 Guidance Variant Verification Matrix 착수 가능
```

Current 판정은 문서 상단 v0.1.25 Status와 아래 v0.1.25 Changelog를 따른다. 현재 구현 계약은 main_game `Document/Systems/Combat/MissileGuidance.md v1.0.1`을 우선한다. 이 절의 MG-P0-10 완료 조건과 당시 invariant는 Historical evidence이며, MG-P0-12B에서 설계하고 MG-P0-12D에서 구현·검증한 Free Stateful Seeker Geometry / Approach-Armed Overshoot / Law-Independent Exact Rear Tie-Break, MG-P0-12E passive Guidance Preset + Editor-only authoring + persisted idempotence 시험 계약과 Low 초기 탄도 존중 교정 계약을 현재 규칙보다 우선하지 않는다.

---

## 20. Changelog

### v0.1.25 - 2026-09-08

- CF-FQ-030 Post-Closure Final Audit Correction + Re-review PASS를 반영했다. Final Audit 자체는 Guidance 알고리즘을 변경하지 않았고 Current System의 Flight 상태/lifecycle·Tick prerequisite 표현과 Plan projection을 Source 기준으로 교정했다.
- 현재 구현 owner를 main_game `Document/Systems/Combat/MissileGuidance.md v1.0.1`으로 동기화했다.
- Product Source/Asset mutation은 0이며 기존 MG-P0-12C/12D/12E Technical evidence, Guidance Preset AssetDump와 Low/Normal/High USER Feel acceptance를 그대로 보존한다.
- Final Audit 재검수 결과 P0=0 / P1=0 / P2=0 PASS이며 이 문서는 계속 Historical Design Evidence + Retained Path다.

### v0.1.24 - 2026-09-08

- CF-FQ-030 P0 Closure + Current System Promotion Review PASS를 반영해 이 문서를 Historical Design Evidence owner로 전환했다.
- Direct TargetActor Missile Flight/Guidance의 현재 구현 계약은 main_game `Document/Systems/Combat/MissileGuidance.md v1.0.0`으로 승격했다.
- CF-TC-027은 Technical + USER Guidance Feel Complete / PASS이며 Low/Normal/High USER 표현 "얼추 PASS"를 P0 acceptance로 유지한다.
- LaserPoint·DataLink/Inertial 실제 Runtime, Angled/Vertical/Loft/TopAttack, 실제 Expire와 장비별 Salvo 연출은 별도 후속 범위다.
- 기존 Build/Authoring/Focused/Missile 10/10과 Guidance Preset AssetDump 3/3은 문서 승격만을 이유로 반복하지 않았다.

### v0.1.23 - 2026-09-08

- USER가 실제 MissileDirectTest PIE에서 persisted Guidance Preset 기반 Low / Normal / High를 모두 직접 확인했고 최종 체감 판정을 "얼추 PASS"로 승인했다.
- MG-P0-12 USER Guidance Feel Validation은 USER ACCEPTED / Complete다. 첫 Low REJECT는 Historical evidence로 보존하며 corrected Low 재검증과 Normal/High USER 확인은 현재 P0 체감 기준에서 PASS다.
- 이 승인은 완벽한 최종 밸런스 고정이 아니다. 숫자 미세조정은 이후에도 Guidance Preset DA에서 비차단으로 수행할 수 있으며 Product Runtime 구조 변경이나 C++ rebuild를 요구하지 않는다.
- CF-TC-027은 기존 Technical Integration PASS와 이번 USER Feel ACCEPTED를 결합해 현재 검증 범위에서 Complete / PASS다.
- MG-P0-12E Mid-review P0/P1/P2 0 PASS, fresh Build/Authoring/Focused/Missile 10/10과 기존 Guidance Preset AssetDump 3/3은 새 failure/change evidence 없이 반복하지 않았다.
- CF-FQ-030 P0 Closure + Current System Promotion Review는 PASS로 완료됐다. Laser Point와 Angled/Vertical/Loft/TopAttack은 Direct P0와 분리된 후속 범위이며 현재 P0 blocker가 아니다.

### v0.1.22 - 2026-09-08

- MG-P0-12E Post-Implementation Mid-review에서 P0=0 / P1=3 / P2=1을 확인했다. 핵심은 Guidance Preset DA의 load-time seed ownership, Product 모듈 품질 이름 분기, Current/Historical projection과 persisted idempotence 증거 공백이었다.
- `UCFMissileGuidePresetData v1.2.0`을 passive data container로 교정했다. `PostInitProperties`와 자산 이름 기반 Low/Normal/High seed를 제거하고 저장된 PresetId/표시 이름/설명/MissileGuideConfig만 소유한다. Asset load-time에는 사용자 튜닝값을 변경하는 Product-side hook이 없다.
- Low/Normal/High 신규 seed를 `CarFight_ReEditor`의 `CFMissileFeelCommands.cpp v1.5.0` authoring으로 이동했다. `EnsureMissileFeelPresetAsset`은 existing Asset을 찾으면 즉시 같은 객체를 반환하고 seed/save/dirty를 수행하지 않으며, 신규 Asset 생성 직후에만 Editor seed를 적용한다.
- Product `CarFight_Re` Source audit에서 `MissileFeel_Low / Normal / High`와 `EMissileFeelTestVariant` 실행 흔적 0을 확인했다. 품질 이름은 Editor authoring/test fixture에만 남고 Product Guidance Runtime은 실제 Guidance 데이터축만 소비한다.
- `CarFight.Authoring.MissileFeel.EnsurePresets`에 별도 임시 persisted Preset round-trip을 추가했다. USER sentinel, Guidance Law, 임의 응답값과 C++ 기본값과 같은 USER 값을 저장하고 다른 Variant seed로 같은 path Ensure를 재실행한 뒤 existing/clean/value-preserved를 검증한다. 이후 package unload → 디스크 package reload → 동일값 readback → package unload → 임시 파일 삭제까지 검증한다.
- 첫 idempotence process `b572202cef824120bbb920ae51c8d0c3`은 값 보존이 아니라 임시 `.uasset` cleanup assertion만 FAIL했다. Test Fixture Cleanup Defect로 분류하고 exact 임시 Asset 1개를 server-owned Trash로 격리한 뒤 UnrealEd package unload/load 경로로 테스트 정리를 교정했다.
- 교정 후 Authoring process `e2e62abaac2148e8806850a137598d03` = 1/1 PASS / Failure 0 / Engine Exit 0. Fresh MG-P0-12E focused process `f430b8d395fc4bb18db238e9eff8766e` = 1/1 PASS / Failure 0 / Engine Exit 0. Fresh full Missile process `2f6b1341bc5c42f988fa13a61d5e8997` = 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Engine Exit 0.
- Final official UE 5.8 Editor Build `823b88eda5fc42b4b83f41135aa2df0f` = Succeeded / Exit 0. `CFMissileFeelCommands.cpp v1.5.0`이 독립 compile되고 CarFight_ReEditor DLL이 재링크됐다.
- 실제 Low/Normal/High persisted Preset과 DirectTest baseline은 correction에서 수정하지 않았다. 기존 Guidance Preset AssetDump 3/3 persisted PASS는 그대로 Current evidence로 재사용하고 불필요한 AssetDump replay는 수행하지 않았다.
- 문서 Current/Historical 경계를 대표 Plan v0.6.22와 동기화했다. v0.1.21 이전의 Corrected Low Ready/당시 next gate 표현은 버전 Changelog의 Historical evidence로 보존하고 현재 next gate는 Mid-review closure 뒤의 `MG-P0-12 USER Guidance Feel Validation — Corrected Low DA Revalidation`이다.
- 최종 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. `CF-TC-027 USER Feel`은 USER 재검증 전까지 NOT ACCEPTED를 유지한다.

### v0.1.21 - 2026-09-08

- USER 체감 수치 조정마다 C++를 수정·빌드하던 비용을 제거하기 위해 Low/Normal/High USER 시험 fixture를 persisted Guidance Preset DataAsset으로 전환했다.
- 신규 `UCFMissileGuidePresetData v1.1.0`은 `PresetId / PresetDisplayName / PresetDescription / FCFMissileGuideConfig`만 소유한다. Product Projectile의 속도·Damage·Collision·FX·ActorClass 소유권은 이동시키지 않았다.
- Preset root는 `/Game/Test/CarFight/Missile/FeelPresets`다. `DA_MissileFeel_Low / Normal / High`를 persisted authoring했으며 AssetDump에서 3/3 readback했다.
- `CFMissileFeelCommands.cpp v1.4.0`은 기존 Low/Normal/High numeric builder를 제거하고 선택 Preset의 `MissileGuideConfig`만 DirectTest transient Projectile 복제본에 overlay한다. Baseline은 저장 `EQ_Missile_DirectTest` 재적용 계약을 유지한다.
- Low/Normal/High의 구조 의미는 각각 PurePursuit / LeadPursuit / ProportionalNavigation으로 유지한다. 숫자 체감 튜닝은 이제 Preset DA가 SSOT이며 수치만 변경할 때 C++ rebuild를 요구하지 않는다.
- `CarFight.Authoring.MissileFeel.EnsurePresets`는 명시 실행형 최초 authoring 진입점이다. 기존 Preset이 있으면 덮어쓰지 않으며 WriteProbe의 rollback-only test mutation 경계를 persisted save 권한으로 확대하지 않는다.
- Official UE 5.8 Editor Build `f0e60b97d7844f46b891eb7e18d13709` = Succeeded / Exit 0.
- Authoring process `bc4f634ca24c464fb8e823af3b0d59b3` = `CarFight.Authoring.MissileFeel.EnsurePresets` 1/1 PASS / Exit 0.
- AssetDump dataset `adset_v1_af183e2d0c12bdb5db3a0f1c5b775ee9.03130ec4fd3c9b7aaa8b7d0a` = Guidance Preset 3/3 persisted readback PASS. Low/Normal/High ID, Guidance Law와 핵심 activation/seeker/maneuver 값이 저장값과 일치한다.
- Focused process `af64ddaf4d224bcf9161411e1ed4b0d4` = `CarFight.Missile.MG_P0_12E.FeelSetupRewire` 1/1 PASS.
- 전체 regression process `5bf5ae819c96429fb5b3e56c8289c398` = `CarFight.Missile` 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit 0.
- 최종 코드·설계 재검수 P0=0 / P1=0 / P2=0 PASS. Product 품질 enum/branch는 0이고 기존 DirectTest persisted baseline은 수정하지 않았다.
- Low first USER Feel REJECTED는 Historical evidence로 유지한다. 2026-09-08 USER가 corrected Low/Normal/High를 직접 확인해 `CF-TC-027 USER Feel ACCEPTED`로 전진했으며 다음 exact Gate는 `CF-FQ-030 P0 Closure + Current System Promotion Review`다.

### v0.1.20 - 2026-09-08

- MG-P0-12 USER Guidance Feel Validation의 첫 Low PIE를 USER가 REJECT했다. 이동 Target 앞을 잘 선행 조준한 발사에서도 초기 궤적을 이르게 버리고 Target 현재/최근 위치 쪽으로 꺾은 뒤 Target 근처에서 다시 크게 수정해 거의 피하는 것처럼 Miss하는 체감이 확인됐다.
- 새 Low acceptance를 `잘 만든 USER 초기 탄도를 Guidance가 발사 직후 즉시 망가뜨리지 않아야 한다`로 명시했다. 이는 HitChance/forced hit가 아니라 Guidance Activation과 단순 추적 반응의 체감 계약이다.
- Product `PurePursuit` Runtime은 변경하지 않고 Editor-only `CFMissileFeelCommands.cpp v1.2.1`의 Low fixture만 교정했다.
- corrected Low = `PurePursuit + Independent 0.25s AND 500cm + Observation 0.08s + VelocityEstimateResponse 0.25s + GuidanceResponse 0.18s + MaximumTurnRate 35deg/s + MaximumLateralAcceleration 2000cm/s^2 + Acquisition 45deg + Tracking 60deg + Grace 0.20s + Reacquisition None`이다. LeadTime/MaxLead는 0이다.
- Official UE 5.8 Editor Build `7136ad4764b24dde9583ec9771e61b94` = Succeeded / Exit Code 0. `CFMissileFeelCommands.cpp` compile + `UnrealEditor-CarFight_ReEditor.dll` link PASS.
- Focused process `f81f55fadfcf48cebbe6b04b91056c4a` = `CarFight.Missile.MG_P0_12E.FeelSetupRewire` 1/1 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- 전체 regression process `7c93d6f7f52f47f486fe90cefa7d3633` = `CarFight.Missile` 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- Persisted Content/DataAsset/Blueprint/Map mutation은 0이고 Product Low/Normal/High enum/branch도 0이다.
- corrected Low USER 재검증은 fresh Editor lifetime에서 수행한다. 현재 당시 실행 중이던 Editor/PIE는 v1.2.1 build 이전 DLL을 로드한 lifetime이라 새 USER evidence로 사용하지 않는다.
- MG-P0-12E Technical PASS는 유지하며, MG-P0-12 USER Guidance Feel Validation은 `Low First Pass REJECTED / Corrected Low Revalidation Ready / Normal·High Pending`이다. CF-TC-027은 NOT ACCEPTED를 유지한다.

### v0.1.19 - 2026-09-08

- MG-P0-12E USER Feel Test Setup Rewire를 Technical PASS로 닫았다. Product Runtime을 추가 분기하지 않고 Editor-only `CFMissileFeelCommands.cpp v1.2.0`의 transient Low/Normal/High 시험 fixture를 실제 Guidance Law / Activation / Seeker geometry 차이로 재배선했다.
- Low fixture는 `PurePursuit + FollowFlightGuidanceWindow + SampledPositionEstimate + Stateful`, Observation 0.15s, Velocity Estimate Response 0.35s, Guidance Response 0.30s, MaximumTurnRate 25deg/s, MaximumLateralAcceleration 1200cm/s^2, Acquisition 25deg, Tracking 35deg, Reacquisition None이다. LeadTime/MaxLead는 0으로 고정해 선행 추적을 사용하지 않는다.
- Normal fixture는 `LeadPursuit + Independent`, Activation Delay 0.06s AND Distance 100cm, LeadTime 0.18s, MaxLead 900cm, Observation 0.08s, Guidance Response 0.20s, MaximumTurnRate 50deg/s, MaximumLateralAcceleration 6000cm/s^2, Acquisition 75deg, Tracking 90deg, Reacquisition 120deg / 0.30s다. Reacquisition > Tracking 조합으로 MG-P0-12D free geometry를 실제 USER fixture에서 사용한다.
- High fixture는 `ProportionalNavigation + Independent 0/0`, Observation 0.03s, Velocity Estimate Response 0.10s, Guidance Response 0.05s, MaximumTurnRate 140deg/s, MaximumLateralAcceleration 15000cm/s^2, Acquisition/Tracking/Reacquisition 모두 180deg다. 발사 첫 유효 Guidance tick부터 exact-rear Target도 각도상 획득 가능한 시험 조합이며 실제 U-turn은 기존 물리 한계를 따른다.
- 세 fixture 모두 Stateful + SampledPositionEstimate를 유지하지만 Guidance Law는 Pure / Lead / PN으로 서로 다르다. Low/Normal/High는 Editor 시험 fixture 이름일 뿐 Product enum/branch가 아니며 Runtime 품질 등급 하드코딩은 0이다.
- 기존 `CarFight.Missile.MG_P0_12A.FeelCommandSetup`은 Baseline/Low/Normal/High console command 등록과 transient 시험 기반 보존을 검증한다. 신규 `CarFight.Missile.MG_P0_12E.FeelSetupRewire`가 exact Law/Activation/Seeker fixture 행렬을 소유한다.
- Official UE 5.8 Editor Build `108388074e0d40619e969026a596713f` = Succeeded / Exit Code 0. `CFMissileFeelCommands.cpp` compile 및 `UnrealEditor-CarFight_ReEditor.dll` link까지 PASS했다.
- Focused process `6f14a6adbb524b0db05ce1e39cd5a5d5` = `CarFight.Missile.MG_P0_12E.FeelSetupRewire` 1/1 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- 전체 regression process `852026f5f3914c1c92220443e059d0ba` = `CarFight.Missile` 10/10 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- Persisted DirectTest Projectile/Weapon/Equipment/Vehicle/Map mutation은 0이다. Baseline command는 저장 `EQ_Missile_DirectTest`를 그대로 재적용하고 Variant는 VehiclePawn outer의 transient 복제에만 GuideConfig를 바꾼다.
- MG-P0-12E Technical PASS는 USER Feel 승인으로 확대하지 않는다. 다음 exact Gate는 기존 Stage 이름인 `MG-P0-12 USER Guidance Feel Validation`이며 USER가 동일 시험 환경에서 Low/Normal/High의 실제 궤적·반응·rear-aspect 체감을 직접 판정해야 한다. CF-TC-027은 NOT ACCEPTED를 유지한다.

### v0.1.18 - 2026-09-07

- v0.1.17 Final closure 후 재검수에서 P1 1건을 발견했다. exact 180deg rear Target의 Launch Right deterministic tie-break가 PN Course Capture 경로에만 연결되어 있어 PurePursuit / LeadPursuit는 정확히 정후방 Target에서 bounded pursuit lateral direction이 0이 될 수 있었다.
- Product를 `CFMissileGuideComp.h/.cpp v1.4.1`로 교정했다. `BuildBoundedPursuitCommandWithRearTieBreak()` 공통 helper를 추가하고 PurePursuit, LeadPursuit, PN Course Capture가 모두 동일한 law-independent exact-rear tie-break를 통과하도록 했다.
- tie-break는 실제 `GuidanceAimPoint`와 Target Snapshot을 변경하지 않는다. 계산용 Pursuit 입력에 Launch Right 기반 `0.001` steering bias만 주어 거의 180deg인 방향오차 크기를 유지하면서 좌우 turn-side만 결정하고, 기존 MaximumTurnRate / MaximumLateralAcceleration / GuidanceResponse 제한을 그대로 적용한다.
- `CFMissileGuideStateTests.cpp v1.4.1`에 PurePursuit와 LeadPursuit exact 180deg rear regression을 추가했다. 두 Law 모두 Tracking 성립, +LaunchRight 실제 횡가속, SeekerAngle 180deg 유지, 요청 회전량이 사실상 180deg를 유지함, 물리 상한 준수, Debug GuidanceAimPoint가 실제 rear Target을 유지함을 직접 검증한다. 기존 PN exact-rear / approach-armed Overshoot / Independent Activation / free Seeker geometry 검증도 함께 유지한다.
- 교정 후 Official UE 5.8 Editor Build `0370b20c40bf45f4a5103d9ef8c807ce` = Succeeded / Exit Code 0. UHT 후 `CFMissileGuideComp.cpp`, `CFMissileGuideStateTests.cpp`, affected Runtime source와 `UnrealEditor-CarFight_Re.dll` link까지 PASS했다.
- 교정 후 Focused process `9a601179fb3a407cb8002ee832c266f0` = `CarFight.Missile.MG_P0_12D.GuidanceActivationRearAspectContract` 1/1 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- 교정 후 전체 Missile regression process `d0b9c2d62f254950bd3af6dabf39dde7` = `CarFight.Missile` 9/9 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- 마지막 source 변경은 `CFMissileGuideStateTests.cpp`의 테스트 함수 설명 주석을 v1.4.1 의미와 맞춘 comment-only correction이며 Runtime/Automation 동작 변경은 없다.
- 교정 후 재검수 결과 P0=0 / P1=0 / P2=0 PASS. v0.1.17의 v1.4.0 Build/Automation 증거는 당시 pre-correction Historical checkpoint로 보존하고 Current 최종 증거는 본 v0.1.18의 v1.4.1 fresh Build/Automation이다.
- Persisted Content/DataAsset/Blueprint/Map mutation은 0이며 DirectTest baseline과 MG-P0-12A Editor-only setup을 보존한다. 다음 exact Gate는 MG-P0-12E USER Feel Test Setup Rewire이며 CF-TC-027 USER Feel은 NOT ACCEPTED를 유지한다.

### v0.1.17 - 2026-09-07

- MG-P0-12D Guidance Activation + Rear Aspect Runtime을 Final Technical PASS로 닫았다. `CFMissileGuideTypes.h v1.3.0`, `CFMissileGuideComp.h/.cpp v1.4.0`에서 FollowFlightGuidanceWindow 호환 경로와 Independent Delay/Distance AND + activation latch를 구현했다.
- Independent activation은 `UCFMissileFlightComp`의 `ElapsedFlightTimeSeconds` / `DistanceFromReleaseCm` Snapshot을 읽고 `UCFMissileGuideComp`가 판정·latch를 소유한다. `Delay=0 / Distance=0`은 첫 유효 Guidance tick부터 활성화되며, 한 번 활성화된 뒤 Release 지점 거리가 다시 줄어도 닫히지 않고 `ResetMissileGuidance()`에서 초기화된다.
- Stateful Acquisition / Tracking / Reacquisition 반각의 기존 Tracking `Min()` coupling을 제거하고 각각 독립적인 0~180deg clamp로 구현했다. `LegacySingleGate`의 기존 FOV / LockBreak 의미는 변경하지 않았다.
- Stateful Overshoot에 `bOvershootArmed`를 추가해 실제 거리 감소 + 양의 ForwardClosingVelocity 접근이 한 번 성립한 뒤에만 Overshoot 판정을 허용했다. 초기 rear Course Capture / U-turn 중 거리 증가를 terminal miss로 오인하지 않으며 Legacy Overshoot 의미는 보존한다.
- exact 180deg rear target의 좌우 특이점은 Launch Context의 `LaunchTransform` Right Vector를 activation 순간 복사해 deterministic turn-side 기준으로 사용한다. random 선택, 위치 순간이동, Velocity 강제 덮어쓰기와 명중 보정은 추가하지 않았고 기존 turn-rate / lateral-acceleration 제한을 그대로 통과한다.
- `FCFMissileGuideSnapshot`에 `GuidanceActivationMode`, `bGuidanceActivationSatisfied`, `bOvershootArmed`를 append-only로 연결하고 Pool-style Reset에서 activation-local 상태를 제거한다.
- `CFMissileFoundationTests.cpp v1.3.0`은 과거 `Acquisition/Reacquisition <= Tracking` assertion을 Current free-geometry 계약으로 교정했고, `CFMissileGuideStateTests.cpp v1.4.0`에 `CarFight.Missile.MG_P0_12D.GuidanceActivationRearAspectContract`를 추가했다.
- Official UE 5.8 Editor Build `16a2f3c957414cb29f18cb812bf4e807` = Succeeded / Exit Code 0. UHT, `CFMissileGuideComp.cpp`, `CFMissileGuideStateTests.cpp`, Runtime/Editor DLL link까지 PASS했다.
- Focused process `723d04a116c54944973be5a6c498de28` = `CarFight.Missile.MG_P0_12D.GuidanceActivationRearAspectContract` 1/1 PASS / Failure 0 / Exit Code 0.
- 전체 Missile regression process `66d2fa64d1314308bd8cc1ea637cdeca` = `CarFight.Missile` 9/9 PASS / Failure 0 / Missing 0 / Unexpected 0 / Duplicate 0 / Exit Code 0.
- 최종 코드·계약 재검수 P0=0 / P1=0 / P2=0 PASS. Persisted Content/DataAsset/Blueprint/Map mutation은 0이며 기존 DirectTest baseline과 MG-P0-12A Editor-only setup은 보존한다.
- MG-P0-12E USER Feel Test Setup Rewire가 다음 exact Gate다. CF-TC-027 USER Feel은 실제 USER 재검증 전까지 NOT ACCEPTED를 유지한다.

### v0.1.16 - 2026-09-07

- MG-P0-12C Guidance Law Runtime을 Technical PASS로 닫았다. PurePursuit는 LastObservedTargetLocation-only, LeadPursuit는 LastObserved + bounded filtered velocity lead, ProportionalNavigation은 Estimated target state를 소비하는 Law별 Sensor State 분리가 실제 Runtime과 Automation에서 확인됐다.
- PN rear/non-closing Course Capture가 bounded Pursuit로 접근 기하를 먼저 만들고, 같은 activation에서 실제 Guidance 결과 Velocity를 유지한 채 ForwardClosingVelocity>0이 성립하면 PN으로 복귀하는 전환을 검증했다.
- 중간검수에서 Course Capture 진입과 별도의 정상 PN만 검증하고 동일 activation 전환을 직접 증명하지 않은 P1 coverage gap을 발견했다. 최초 보강안은 기존 테스트 helper가 매 step +X Velocity를 복원하는 Test Fixture Stimulus Defect가 있어 Product 결함으로 분류하지 않고 CFMissileGuideStateTests.cpp v1.3.1의 AdvanceGuidanceKeepingVelocity()로 Test-only 교정했다.
- Product Runtime correction은 0이며 MG-P0-12D 소유인 Independent Guidance Activation, free Stateful Seeker geometry, approach-armed Overshoot, exact 180deg deterministic tie-break Runtime은 아직 구현하지 않았다.
- Official Build `8c30576af506485f8e811fc8eb326806` = Succeeded / Exit Code 0.
- Focused process `62d9858360ab45caacd31924867d2604` = `CarFight.Missile.MG_P0_12C.GuidanceLawContract` 1/1 PASS / Exit Code 0.
- Affected regression process `43b6343b4cad4ed985698e4c6bd600ea` = `CarFight.Missile` 8/8 PASS / Failure 0 / Missing 0 / Unexpected 0 / Exit Code 0.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. 다음 exact Gate는 MG-P0-12D Guidance Activation + Rear Aspect Runtime이며 CF-TC-027 USER Feel은 NOT ACCEPTED 상태를 유지한다.

### v0.1.15 - 2026-09-07

```text
- MG-P0-12B 최초 설계검수 P0=2 / P1=5 / P2=3을 교정했다.
- P0-1: current PN은 rear/non-closing geometry에서 ClosingSpeed=0으로 Guidance command가 0이 될 수 있으므로 ProportionalNavigation에 물리 제한형 Course Capture 단계를 명시했다. 유효 Target을 향해 기수를 돌려 ForwardClosingVelocity>0 접근 기하가 형성된 뒤 PN으로 전환하며 기존 Turn/Lateral/Response 제한을 모두 지킨다.
- P0-2: 기존 Stateful Overshoot가 rear-aspect U-turn 도중 거리 증가 + non-closing을 즉시 terminal miss로 오인하지 않도록 bOvershootArmed 계약을 추가했다. 실제 거리 감소 + ForwardClosingVelocity>0 접근이 한 번 성립한 뒤에만 Overshoot를 arm하며, LegacySingleGate의 기존 Overshoot 의미는 호환 경로로 보존한다.
- P1-1: Activation enum의 의미를 실제 Source owner와 맞추기 위해 AfterFlightClearance를 FollowFlightGuidanceWindow로 교정했다. 기본 호환 모드는 UCFMissileFlightComp::IsGuidanceWindowOpen()을 그대로 따르며 Clearance와 GuidanceWindow가 미래 상태에서 항상 동일하다고 가정하지 않는다.
- P1-2: Independent activation의 Delay/Distance 조건을 AND로 확정하고 FlightSnapshot.ElapsedFlightTimeSeconds / DistanceFromReleaseCm을 사용한다. 한 번 만족하면 bGuidanceActivationSatisfied를 activation 동안 latch하고 ResetMissileGuidance에서만 초기화한다.
- P1-3: Sampled observer의 Single Sensor Truth를 Sensor State 개념으로 정리하고 GuidanceAimPoint 소비를 Law별로 분리했다. Seeker/Overshoot는 EstimatedTargetLocation, PurePursuit는 LastObservedTargetLocation, LeadPursuit는 LastObserved + bounded velocity lead, PN은 Estimated state를 소비한다.
- P1-4: LeadPursuit exact formula를 LeadOffset=FilteredVelocity*LeadTime, magnitude<=MaxLeadDistance, AimPoint=LastObserved+LeadOffset로 확정해 EstimatedTargetLocation과의 이중 예측을 금지했다.
- P1-5: 정확히 180deg rear target의 회전 방향 특이점은 LaunchContext.LaunchTransform Right Vector를 deterministic tie-break로 사용하고 random 선택을 금지한다. 이는 회전 방향만 정하며 물리 기동 한계는 그대로 적용한다.
- P2: DataAsset EditCondition/finite clamp, Snapshot append-only 진단값(GuidanceLaw/Activation/AimPoint/CourseCapture/OvershootArmed), Stateful Seeker tooltip/test coupling 교정 범위를 확정했다.
- Stateful Acquisition/Tracking/Reacquisition 반각은 각각 독립 0~180deg clamp로 바꾸며 기존 Min coupling은 Historical MG-P0-08 계약으로만 남긴다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. Product Source/Content mutation은 0이며 다음 exact Gate는 MG-P0-12C Guidance Law Runtime이다.
```

### v0.1.14 - 2026-09-07

```text
- MG-P0-12A Low USER PIE에서 "Low/Normal/High가 같은 PN 계열 로직에 숫자만 다른 구조라 근본적인 행동 차이가 부족하다"는 USER Feel rejection을 기록했다.
- 저성능은 현재 관측 Target 위치를 단순히 따라가는 PurePursuit, 중급은 bounded LeadPursuit, 고급은 기존 ProportionalNavigation을 선택할 수 있도록 Guidance Law Strategy를 신규 독립 데이터축으로 설계했다.
- Product Low/Normal/High 품질 enum은 만들지 않고 ECFMissileGuidanceLaw 자체가 알고리즘 의미만 표현한다. 신규 필드 기본값은 ProportionalNavigation으로 계획해 기존 Asset 행동을 보존한다.
- PurePursuit는 TargetVelocityEstimate/LOS/closing-speed를 선행 예측에 사용하지 않으며 Sampled observer 조합에서는 LastObservedTargetLocation을 기본 추적점으로 사용해 숨은 extrapolation 예측을 금지한다.
- USER 추가 요구로 발사 후 Seeker/Guidance 시작 시점을 Flight Clearance와 분리하는 ECFMissileGuidanceActivationMode + GuidanceActivationDelaySeconds + GuidanceActivationDistanceCm 설계를 추가했다.
- current Source의 MinimumClearanceTimeSeconds 0.15s AND MinimumClearanceDistanceCm 300cm가 Direct GuidedFlight 진입을 지연시키는 실제 원인임을 source audit으로 확인했다.
- AfterFlightClearance를 호환 기본값으로 두고 Independent + Delay0 + Distance0에서는 발사 후 첫 유효 Guidance tick부터 Acquiring/Tracking을 허용하는 방향을 확정했다.
- Acquisition/Tracking/Reacquisition 반각은 각각 독립 0~180deg 데이터가 되어야 하며 current Acquisition/Reacquisition <= Tracking Min coupling은 후속 구현에서 제거/재설계 대상으로 기록했다.
- Acquisition=180 / Tracking=180과 빠른 activation, 높은 turn/lateral authority를 조합하면 발사 직후 뒤쪽 Launch Snapshot Target도 각도상 획득하고 물리 제한 안에서 크게 U-turn할 수 있어야 한다. 순간이동/강제 명중은 계속 금지한다.
- MG-P0-12A 기존 기술 Setup/7개 Automation baseline은 폐기하지 않고 Historical/Technical baseline으로 보존한다. USER Feel은 미승인 유지한다.
- 다음 exact Gate는 MG-P0-12B Guidance Law + Launch Activation + Seeker Geometry Design Audit이다.
```

### v0.1.13 - 2026-09-07

```text
- MG-P0-12A USER 시험 helper의 소유권을 Product Runtime 모듈과 분리하기 위해 CFMissileFeelCommands.cpp를 CarFight_ReEditor 모듈로 이동했다.
- Current Source: UE/Source/CarFight_ReEditor/Private/CFMissileFeelCommands.cpp v1.1.1.
- v1.1.1은 기능/명령/Variant 수치를 변경하지 않고 Editor-only ownership만 교정했다. CarFight_ReEditor는 기존에 CarFight_Re를 의존하므로 실제 Missile Runtime API를 그대로 사용한다.
- Product packaged Runtime에는 시험용 Baseline/Low/Normal/High command/branch가 포함되지 않는 구조가 파일 소유권으로도 명확해졌다.
- 모듈 이동 후 공식 UE 5.8 Build acadfea728224ebe9ffdbeb3e8ece08d: CFMissileFeelCommands.cpp가 CarFight_ReEditor source로 compile되고 UnrealEditor-CarFight_ReEditor.dll link까지 Exit 0 PASS.
- 모듈 이동 후 Focused CarFight.Missile.MG_P0_12A.FeelCommandSetup process 89c440f6ff034fe48f3aa243082db89b: 1/1 PASS / Exit 0.
- 모듈 이동 후 전체 CarFight.Missile affected regression process 9731fccffc194204b5ac31c408e5472b: 7/7 PASS / Failure 0 / Missing 0 / Unexpected 0 / Exit 0.
- 기존 MG-P0-00 Foundation, MG-P0-01~04 DirectRuntime, MG-P0-08 ConfigFoundation, MG-P0-09 StatefulSeeker, MG-P0-10 ObservationEstimator, MG-P0-11 GuidanceVariantMatrix와 MG-P0-12A FeelCommandSetup이 모두 PASS다.
- Content Asset mutation은 0이며 기존 DA_Missile_DirectTest 및 관련 DirectTest Vehicle/Weapon/Equipment/Map은 계속 regression baseline으로 보존한다.
- Technical setup은 최종 PASS. 남은 유일한 다음 Gate는 실제 MissileDirectTest PIE에서 Baseline/Low/Normal/High Runtime switch와 발사를 확인하는 MG-P0-12A USER PIE Runtime Switch Validation이다.
```

### v0.1.12 - 2026-09-07

```text
- MG-P0-12A의 실제 USER 비교 경로를 persisted Low/Normal/High DataAsset 3벌 대신 Editor-only transient Runtime override 방식으로 구현했다.
- 당시 신규 Source(pre-module-move): UE/Source/CarFight_Re/Private/CFMissileFeelCommands.cpp v1.1.0. Current source는 v0.1.13에서 UE/Source/CarFight_ReEditor/Private/CFMissileFeelCommands.cpp v1.1.1로 이동했다.
- 구현은 WITH_EDITOR 경계 안에서만 콘솔 명령을 등록하며 packaged Product Runtime에 Low/Normal/High enum/branch를 추가하지 않는다.
- 저장 DA_Missile_TestSUV와 EQ_Missile_DirectTest를 read-only 기준으로 로드하고, persisted WeaponData/ProjectileData를 VehiclePawn Outer의 RF_Transient UObject로 DuplicateObject한다.
- transient ProjectileData에서는 MissileGuideConfig만 변경하고 Mesh, ProjectileActorClass, Flight, Collision, Damage, Trail/Thruster 등 나머지 DirectTest 계약은 원본 복제로 유지한다.
- 실제 VehicleWeaponComp public seam인 InitializeWeaponRuntimeFromFitting을 사용해 transient EquipmentPreset을 활성화하며 ActiveWeaponData/ActiveProjectileData exact pointer readback이 실패하면 즉시 Baseline을 복원한다.
- USER 명령은 CarFight.MissileFeel.Baseline / Low / Normal / High 네 개다.
- Low/Normal/High 수치는 MG-P0-11 검증 fixture와 동일하다: Low 0.15/0.35/0.30s, 25deg/s, 1200cm/s2, Tracking35, Reacq=None; Normal 0.08/0.20/0.20s, 50deg/s, 6000cm/s2, Tracking65, ForwardCone; High 0.03/0.10/0.10s, 80deg/s, 15000cm/s2, Tracking85, ForwardCone.
- 최초 v1.0.0 빌드는 UE 5.8에 존재하지 않는 FAutoConsoleCommandWithArgs 타입 사용으로 Exit 6 실패했으며, current source evidence의 FAutoConsoleCommand + FConsoleCommandDelegate 패턴으로 v1.0.1 교정했다.
- 교정 후 Build 7e010a90c3104eaa9cff3e9adcc83bb6 PASS 후, v1.1.0 Focused Automation을 추가해 네 명령 등록과 Variant exact config를 PIE 없이 검증했다.
- 최종 공식 UE 5.8 Build de0111d1155945b1a28e962c8fdf9e2e: CFMissileFeelCommands.cpp compile + CarFight lib/dll link / Exit 0 PASS.
- Focused CarFight.Missile.MG_P0_12A.FeelCommandSetup process 876d4dd231b44c418d19dd72486644a5: 1/1 PASS / Exit 0.
- 전체 CarFight.Missile process 74e7dc8034084fa1a7ab7566bb596252: 7/7 PASS / Exit 0.
- canonical carfight.pie.start는 process 1ca5fb9f1dd949299fd3673e0776dc50에서 Managed UE Bridge interpreter identity mismatch로 PIE 시작 전에 실패했다. UE Bridge의 direct StartPIE tool 역시 server policy 차단이므로 AI가 live PIE를 우회 시작하지 않았다.
- 따라서 MG-P0-12A 구현/Build/Offline Automation은 PASS지만 실제 MissileDirectTest PIE에서 Baseline→Low→Normal→High Runtime switch가 production VehicleWeaponComp에 반영되는 live validation은 아직 Pending이다.
- 다음 exact Gate는 MG-P0-12A USER PIE Runtime Switch Validation이며, 이것이 PASS한 뒤 MG-P0-12 USER Guidance Feel Validation을 시작한다.
```

### v0.1.11 - 2026-09-07

```text
- MG-P0-11 최종검수에서 P0=0 / P1=3 / P2=0을 확인했다.
- P1-1은 MG-P0-11의 Low/Normal/High 핵심 response 비교가 20deg 정지 Target 중심이라 MG-P0-10에서 독립 검증한 observer timing을 실제 Variant moving Target 차이와 직접 연결하지 못한 점이었다.
- CFMissileGuideStateTests.cpp를 v1.2.1로 올리고 동일 Target을 launch seed → +600Y 이동 → -600Y 반전시키는 Low/Normal/High 결정론적 행렬을 추가했다.
- 누적 0.05s에서 High(0.03s)가 +600Y를 먼저 관측하고 Low(0.15s)/Normal(0.08s)은 seed에 남는 차이를 검증했다.
- 누적 0.09s 반전 시점에서 Low는 아직 seed에 남고 Normal/High는 -600Y를 관측하며, FilteredTargetVelocityEstimate가 High < Normal < Low 순으로 더 빠르게 -Y 반전에 반응함을 검증했다.
- 같은 reversal 자극의 실제 Guidance Command도 Low < Normal < High Applied Lateral Acceleration 차이를 유지하고 각 Variant 물리 상한을 넘지 않음을 검증했다.
- 교정 후 Focused GuidanceVariantMatrix PASS: process a0ff269df8b14c3892c5155e2a24056f / Success 1 / Failure 0 / Engine Exit 0.
- 교정 후 CarFight.Missile affected regression PASS: process 5244de95c48d44e6b961cd0c5793392f / Success 6 / Failure 0 / Engine Exit 0.
- 교정 Source 공식 UE 5.8 Editor Build PASS: job bc116df36570486984d1afcd0f5b7508 / CFMissileGuideStateTests.cpp independent compile + CarFight lib/dll link / Exit 0.
- P1-2는 MG-P0-12를 실제 USER Ready로 표기했지만 Low/Normal/High가 transient Automation fixture에만 존재하고 실제 PIE 비교용 persisted 선택 경로가 없다는 상태 과장이었다.
- Current UE WriteProbe에서 /Game/Test 하위 UCFProjectileData 신규 생성 가능성은 확인했지만 ObjectTools property mutation은 검증 후 자동 inverse-write되어 persisted setup을 만들 수 없음을 확인했다. 시험 생성한 DA_Missile_Feel_Low는 저장하지 않았고 AI-owned Editor를 no-save 종료해 폐기했다.
- 따라서 MG-P0-12를 즉시 Ready로 두지 않고 MG-P0-12A USER Guidance Feel Test Setup을 선행 Gate로 분리했다.
- MG-P0-12A는 기존 DA_Missile_DirectTest 회귀 자산을 보존하고, 같은 발사체/외형/충돌 계약에서 MissileGuideConfig만 다른 Low/Normal/High 실제 PIE 비교 자산과 선택 경로를 준비하는 단계다.
- P1-3은 §18~19의 v0.1.9 `현재 판정`이 상단 v0.1.10과 충돌한 문서 projection 문제였으며 해당 블록을 Historical MG-P0-10 checkpoint로 명시했다.
- 최종 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. MG-P0-11은 최종 Technical PASS이며 다음 exact Gate는 MG-P0-12A USER Guidance Feel Test Setup이다.
```

### v0.1.10 - 2026-09-07

```text
- MG-P0-11 Guidance Variant Verification Matrix를 CFMissileGuideStateTests.cpp v1.2.0에 추가했다.
- Product Runtime에 Low/Normal/High enum이나 품질 분기를 추가하지 않고, Automation 내부 transient FCFMissileGuideConfig 세 개만 서로 다른 수치로 구성했다.
- 저성능 fixture는 0.15s 관측 / 0.35s 속도 추정 응답 / 0.30s Guidance 응답 / 25deg/s / 1200cm/s2 / Acquisition25 / Tracking35 / grace0.10 / Reacquisition=None으로 구성했다.
- 기준형 fixture는 0.08s / 0.20s / 0.20s / 50deg/s / 6000cm/s2 / Acquisition35 / Tracking65 / grace0.40 / ForwardCone45 / reacquisition0.30으로 구성했다.
- 고성능 fixture는 0.03s / 0.10s / 0.10s / 80deg/s / 15000cm/s2 / Acquisition50 / Tracking85 / grace0.80 / ForwardCone65 / reacquisition0.60으로 구성했다.
- 같은 20도 정지 오프축 Target engagement에서 세 fixture 모두 Tracking을 획득하고, Applied Lateral Acceleration과 Applied Turn Rate가 Low < Normal < High 순서로 갈리며 각자 MaximumTurnRate / MaximumLateralAcceleration 상한을 넘지 않음을 검증했다.
- 같은 50도 Target 기하에서 Low는 Tracking35 이탈 + Reacquisition=None으로 LostFinal/TargetLost가 되고 Normal/High는 Tracking을 유지해 정상적인 성능 차이를 증명했다.
- 같은 Launch Snapshot Target을 80도로 이탈시킨 뒤 40도로 복귀시키는 행렬에서 Reacquisition=None은 LostFinal을 유지하고 ForwardCone은 Reacquiring→Tracking으로 복원됨을 검증했다.
- Variant 자극 뒤 Low/Normal/High 모두 Pool-style Reset에서 SeekerState Inactive, observation valid false, EstimatedTarget zero, GuidanceTargetActor null을 확인했다.
- 쉬운 정면 정지 Target은 transient actual ACFProjectileActor + production ProjectileMovement로 teleport 없이 실제 Blocking contact가 가능함을 검증했다. 첫 실행의 독립 contact 진단은 35 steps / Contact X=871.4 / Target X=1000 / CenterDistance=128.6cm였고, 최종 교정 실행에서도 동일 contact assertion이 PASS했다.
- 최초 Focused 실행 03e81128cf974c16bb107c8ef9a1ed31은 on-axis seed 뒤 순간 측면 위치변화를 같은 sample에서 velocity estimate로 만든 시험 자극 때문에 PN 상대운동이 상쇄되어 Low/Normal/High Applied Guidance가 모두 0이 되는 fixture failure였다. Straight Blocking contact는 이 실행에서도 PASS했으며 Product defect로 분류하지 않았다.
- 비교 자극을 모든 Variant가 Acquisition 가능한 동일 20도 정지 오프축 Target으로 교정해 관측 위치 변화 artifact 없이 Guidance response / physical performance 차이만 측정하도록 했다. Product Runtime 수정은 없었다.
- 교정 후 Focused GuidanceVariantMatrix PASS: process 5f07656c176d43e4ba770ee3a67fbbc4 / Success 1 / Failure 0 / Engine Exit 0.
- CarFight.Missile affected regression PASS: process 0aa46624f9364f71b226492ed19a92d7 / Success 6 / Failure 0 / Engine Exit 0.
- MG_P0_00 Foundation, MG_P0_01_04 DirectRuntime, MG_P0_08 ConfigFoundation, MG_P0_09 StatefulSeeker, MG_P0_10 ObservationEstimator, MG_P0_11 GuidanceVariantMatrix가 모두 함께 PASS했다.
- 최종 공식 UE 5.8 Editor Build 0f75526616124297949ccb22438d07b8은 CFMissileGuideStateTests.cpp independent compile, CarFight lib/dll link까지 Result Succeeded / Exit Code 0 PASS했다.
- MG-P0-11 Product Runtime/Config type/Projectile integration/기존 RuntimeTests/Content Asset/Blueprint/Map mutation은 0이다.
- 랜덤 sensor noise와 HitChance/MissProbability는 추가하지 않았다.
- 최종 구현검수 P0=0 / P1=0 / P2=0 PASS. 다음 exact Gate는 MG-P0-12 USER Guidance Feel Validation이다.
```

### v0.1.9 - 2026-09-07

```text
- MG-P0-10 Observation + Velocity Estimator를 UCFMissileGuideComp v1.2.0으로 구현했다.
- SampledPositionEstimate는 발사 순간 Target 위치를 seed로 1회 확보하고, 이후 TargetObservationIntervalSeconds가 충족될 때만 Target Actor 위치를 최대 1회 샘플한다.
- Sampled 경로에서 TargetActor->GetVelocity()를 Guidance 정답으로 소비하지 않으며, 위치 차분 / 실제 ObservationAgeSeconds로 Raw Target Velocity를 추정한다.
- TargetVelocityEstimateResponseTimeSeconds로 위치 차분 속도 추정값을 결정론적으로 필터링하고, 관측 사이 EstimatedTargetLocation은 LastObservedTargetLocation + FilteredTargetVelocityEstimate * ObservationAgeSeconds로 외삽한다.
- ObservationAgeSeconds 단일 clock이 sample interval, velocity sample delta, extrapolation age를 함께 소유한다.
- Stateful Sampled에서 Seeker angle / LostGrace / Reacquisition / Overshoot / PN은 같은 Estimated Target State를 소비하고 hidden Actor kinematics로 보정하지 않는다.
- Guidance Window가 닫힌 동안 Sampled observer는 ObservationAgeSeconds만 누적하고 새 Actor 위치 sample이나 Seeker 전이를 수행하지 않는다. Window open 첫 Guidance step에서 interval이 이미 충족됐으면 실제 위치 sample을 최대 1회 즉시 취득한다.
- HoldLastKnownPoint는 Sampled LostFinal 진입 순간 현재 Estimated Sensor Truth를 고정하고 이후 Actor 이동이나 estimator 외삽으로 갱신하지 않는다.
- LegacySingleGate + SampledPositionEstimate 조합도 지원해 SeekerModel과 TargetObservationMode 독립 축 계약을 유지했다.
- CFMissileGuideStateTests.cpp를 v1.1.0으로 올리고 CarFight.Missile.MG_P0_10.ObservationEstimatorContract를 추가했다.
- Focused ObservationEstimatorContract PASS: process b786eb16bdcc46bd947e274b9bf03e53 / Success 1 / Failure 0 / Engine Exit 0.
- CarFight.Missile affected regression PASS: process 9210ce22552840c5bdcf9f7a8af55d81 / Success 5 / Failure 0 / Engine Exit 0.
- 기존 MG_P0_00 Foundation, MG_P0_01_04 DirectRuntime, MG_P0_08 ConfigFoundation, MG_P0_09 StatefulSeeker와 신규 MG_P0_10 ObservationEstimator가 모두 함께 PASS했다.
- 공식 UE 5.8 Editor Build b6c4cd46bba941958abd5c45de009dc0은 UHT 1 generated file, CFMissileGuideComp.cpp / CFMissileGuideStateTests.cpp / CFMissileRuntimeTests.cpp 독립 compile, CarFight lib/dll link까지 Result Succeeded / Exit Code 0 PASS했다.
- UCFMissileGuideComp public API, CFMissileGuideMath, Projectile Launch/Actor/Data public integration, 기존 CFMissileRuntimeTests.cpp v1.3.0과 Content Asset/Blueprint/Map은 변경하지 않았다.
- 기존 저장 DA_Missile_DirectTest는 LegacySingleGate + DirectActorKinematics 기본값을 유지하며 이번 단계에서 Product DA를 Sampled로 전환하지 않았다.
- 랜덤 센서 노이즈와 직접 HitChance는 추가하지 않았다.
- 최종 구현검수 P0=0 / P1=0 / P2=0 PASS. 다음 exact Gate는 MG-P0-11 Guidance Variant Verification Matrix다.
```

### v0.1.8 - 2026-09-07

```text
- MG-P0-09 Stateful Seeker Runtime을 UCFMissileGuideComp v1.1.0으로 구현했다.
- Stateful 상태를 Inactive / Acquiring / Tracking / LostGrace / Reacquiring / LostFinal로 분리했다.
- Acquisition / Tracking / Reacquisition 반각을 각각 적용하고 같은 Launch GuidanceTargetActor Snapshot만 추적·재포착하도록 유지했다.
- Tracking 상실 뒤 LostGrace 복귀, Reacquisition=None 영구 상실, ForwardCone 재포착, Destroyed Target 재포착 금지, ReacquisitionTime=0 즉시 LostFinal, Overshoot 우선 종결을 구현했다.
- HoldLastKnownPoint는 LostFinal 진입 순간 고정 지점을 Freeze하고 이후 Target Actor Tracking을 재개하지 않는다.
- ContinueStraight / Expire는 LostFinal에서 Guide Tick을 종료하며 Projectile 실제 수명·Pool 반환 소유권은 기존 Projectile Runtime에 유지한다.
- Pool Reset에서 Seeker/observer/hold 상태를 완전 초기화하고 GuideActivationCount 누적 계약을 보존한다.
- 신규 CFMissileGuideStateTests.cpp v1.0.0 / CarFight.Missile.MG_P0_09.StatefulSeekerContract를 추가했다.
- Focused StatefulSeekerContract 1/1 PASS: process 52ecbb245246445aaa75e529e69b07e0 / Engine Exit 0.
- CarFight.Missile affected regression 4/4 PASS: process 00f1b39cc8d840d9839397816d802f4c / Engine Exit 0.
- 기존 MG_P0_00 Foundation, MG_P0_01_04 DirectRuntime, MG_P0_08 ConfigFoundation이 모두 함께 PASS해 Legacy default path 비침범을 확인했다.
- 첫 공식 Build 5a76c185be894db08345c9c93a4d246c은 새 source 추가에 따른 non-unity 재배치에서 unrelated CFVehicleVisualComp.cpp의 latent AActor include 의존성을 노출해 Exit 6으로 실패했다.
- CFVehicleVisualComp.cpp v1.0.2에 GameFramework/Actor.h 명시 include만 추가해 Runtime/API 변경 없이 standalone IWYU blocker를 교정했다.
- 재빌드 9d495eda12cc42958cf19ce62d3d9ed7은 CFVehicleVisualComp.cpp를 실제 non-unity 독립 컴파일한 뒤 UnrealEditor-CarFight_Re.dll link까지 Result Succeeded / Exit Code 0 PASS했다.
- MG-P0-09의 Technical PASS 범위는 Stateful + DirectActorKinematics다. SampledPositionEstimate의 관측 주기/위치 샘플/속도 추정/외삽은 MG-P0-10 미구현 범위이며 아직 Product DA에 적용하지 않는다.
- 최종 구현검수 P0=0 / P1=0 / P2=0 PASS. 다음 exact Gate는 MG-P0-10 Observation + Velocity Estimator다.
```

### v0.1.7 - 2026-09-07

```text
- 기존 공식 Build Job b2cfa60513ec4fd6af5467722b697ba1 terminal receipt를 later canonical interaction에서 회수했다.
- CarFight_ReEditor Win64 Development / D:\UnrealEngine_Source / Result Succeeded / Exit Code 0을 확인했다.
- UHT 5 generated files, CFMissileFoundationTests.cpp, CFMissileRuntimeTests.cpp compile, UnrealEditor-CarFight_Re.dll link까지 PASS했다.
- Focused MG_P0_08.ConfigFoundation 1/1 PASS와 CarFight.Missile 3/3 PASS 증거와 결합해 MG-P0-08 최종 구현검수를 P0=0 / P1=0 / P2=0 PASS로 닫았다.
- 다음 exact Gate를 MG-P0-09 Stateful Seeker Runtime 구현으로 전환한다.
```

### v0.1.6 - 2026-09-07

```text
- MG-P0-08 Types / Config Foundation을 CFMissileGuideTypes.h v1.1.0과 CFMissileFoundationTests.cpp v1.1.0으로 구현했다.
- LegacySingleGate/Stateful, DirectActorKinematics/SampledPositionEstimate, None/ForwardCone 및 SeekerState 타입을 추가했다.
- 신규 관측·Seeker 성능 Config, effective clamp와 Tracking 이하 Acquisition/Reacquisition angle invariant를 구현했다.
- FCFMissileGuideSnapshot에 후속 Runtime이 채울 observer/seeker 상태 Foundation 필드를 append-only로 추가했다.
- UCFMissileGuideComp Runtime, GuideMath, ProjectileActor, ProjectileData public API, 기존 RuntimeTests와 Content는 변경하지 않았다.
- Focused MG_P0_08.ConfigFoundation 1/1 PASS와 CarFight.Missile 전체 3/3 PASS를 확인했다.
- 공식 Editor Build Job b2cfa60513ec4fd6af5467722b697ba1은 D:\UnrealEngine_Source/UHT 진입을 확인했으나 Browser build observation budget 소진으로 terminal Exit Code receipt는 Pending이다.
- 새 Automation이 실제 UE process에서 실행돼 신규 모듈 compile/link/load는 확인했지만 build Job 자체를 terminal PASS로 과장하지 않는다.
- 다음 Runtime 단계 MG-P0-09는 아직 시작하지 않았다.
```

### v0.1.5 - 2026-09-07

```text
- v0.1.4에서 HoldLastKnownPoint의 HoldTargetLocation semantics를 확정한 뒤 Pool Reset exact-list에 해당 신규 내부 상태가 빠진 P1 1건을 최종 확인했다.
- ResetMissileGuidance가 HoldTargetLocation=Zero와 bHasHoldTargetLocation=false를 반드시 초기화하도록 계약을 보강했다.
- Pool-style Reset Automation에도 HoldTargetLocation/hold-valid flag 오염 없음 검증을 추가했다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. Product Source/Content mutation 없이 MG-P0-08 Types / Config Foundation 구현 Gate를 유지한다.
```

### v0.1.4 - 2026-09-07

```text
- v0.1.3 교정 전체를 구현 준비 관점에서 다시 검수해 P0=0 / P1=2 / P2=1의 잔여 의미 모호성을 확인했다.
- Sampled observer의 ObservationElapsedTimeSeconds / TimeSinceLastObservationSeconds 이중 시간 개념을 ObservationAgeSeconds 단일 clock으로 통합했다.
- Sample interval 판정, raw velocity estimate의 실제 sample delta와 관측 사이 EstimatedTargetLocation 외삽이 모두 같은 ObservationAgeSeconds를 소비하도록 고정했다.
- HoldLastKnownPoint는 LostFinal 진입 순간의 현재 Sensor Truth 위치를 HoldTargetLocation으로 1회 Freeze하고 이후 velocity estimate로 계속 움직이지 않도록 확정했다.
- Stateful Acquiring/LostGrace/Reacquiring 임시 상태는 bTargetValid=false / MissReason=None으로 두고, 최종 LostFinal은 TargetLost, Overshoot와 Expire는 기존 진단값을 사용하도록 확정했다.
- LegacySingleGate의 SeekerFieldOfViewExceeded / LockBreakAngleExceeded 진단은 그대로 보존하고 Stateful 때문에 기존 MissReason enum을 확장하지 않는다.
- 최종 재검수 P0=0 / P1=0 / P2=0 PASS. MG-P0-08 Types / Config Foundation 구현 Gate를 최종 OPEN했다.
```

### v0.1.3 - 2026-09-07

```text
- 구현 직전 독립 설계검수에서 P0=0 / P1=6 / P2=2를 확인했다.
- Acquisition cone 밖의 유효 Target은 Acquiring을 유지하고 최초 획득 전 LostGrace로 진입하지 않도록 문서 충돌을 제거했다.
- Tracking cone 이탈과 Target Actor invalid/destroyed를 분리해 destroyed Target은 grace 이후 Reacquisition 없이 LostFinal로 고정했다.
- Effective Acquisition/Reacquisition cone을 Tracking cone 이하로 제한해 Tracking 진입 직후 즉시 LostGrace로 떨어지는 상태 진동을 방지했다.
- Start sample 이후 Guidance Window가 닫힌 동안 observation elapsed만 누적하고 추가 Actor sample/Seeker 전이는 하지 않으며, Window open 시 interval 충족이면 최대 1회 즉시 sample하도록 시간 계약을 확정했다.
- LostTargetPolicy=Expire는 MG-P0-08~10에서 Guide 종료 + LifeExpired 진단만 소유하고 실제 Projectile 만료/Pool 반환은 기존 Projectile LifeTime timer가 소유하도록 no-touch 경계를 유지했다.
- FCFMissileGuideSnapshot의 기존 TargetLocation 의미를 보존하면서 SeekerModel/State, ObservationMode, LastObserved, EstimatedTarget, VelocityEstimate, ObservationAge, ReacquisitionElapsed, valid flag 신규 필드를 확정했다.
- 첫 ECFMissileReacquisitionMode Product enum은 None/ForwardCone만 노출하고 WideCone/DataLinkAssisted는 미래 후보로 이동했다.
- TargetLostGraceTime=0 / ReacquisitionTime=0의 same-step 전이와 모델별 DataAsset EditCondition을 확정했다.
- current ACFProjectileActor의 Motor→Flight→Guide→ProjectileMovement prerequisite를 확인해 Tick architecture를 변경하지 않는 것으로 확정했다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. MG-P0-08 Types / Config Foundation 구현 Gate를 OPEN했다.
```

### v0.1.2 - 2026-09-07

```text
- MG-P0-08 Source/API 계약검수를 current CFMissileGuideTypes / GuideComp / GuideMath / ProjectileData / ProjectileActor / LaunchContext / Automation source와 대조했다.
- 초기 검수 P0=0 / P1=3 / P2=2를 확인하고 Reacquisition↔Overshoot 우선순위, Sampled Sensor Truth, public API/enum/shared owner 경계를 교정했다.
- Overshoot는 Reacquisition보다 우선하는 terminal miss로 고정해 목표를 지나친 뒤 U-turn 재추적하지 않도록 했다.
- SampledPositionEstimate에서는 Estimated Target State를 Seeker/상태전이/PN/Overshoot가 공통 소비하며 실제 Actor 위치·속도의 숨은 재조회로 보정하지 않도록 했다.
- low-FPS/hitch에서 실제 Actor sample은 Guidance Tick당 최대 1회만 취득하고 누적 실제 elapsed time으로 velocity estimate를 계산하도록 했다.
- UCFMissileGuideComp 기존 public signature, CFMissileGuideMath, CFProjectileLaunchTypes, CFProjectileActor, UCFProjectileData public API를 no-touch 기본 경계로 확정했다.
- 기존 ECFMissileGuideMode/LostTargetPolicy/MissReason 순서를 보존하고 첫 Reacquisition enum은 None/ForwardCone만 노출하도록 했다.
- 신규 Config 기본값/clamp, Guidance Window 진입, Pool Reset 상태를 구현 계약으로 확정했다.
- 상태형/Estimator Automation은 신규 CFMissileGuideStateTests.cpp로 분리해 현재 dirty CFMissileRuntimeTests.cpp v1.3.0 integration evidence를 보호한다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS. MG-P0-08 Implementation Gate를 OPEN했다.
```

### v0.1.1 - 2026-09-07

```text
- v0.1.0 설계검수 P1 1건을 교정했다.
- 신규 필드 저장 여부 추측 대신 ECFMissileSeekerModel과 ECFMissileTargetObservationMode의 명시적 Legacy/New 모델 선택을 추가했다.
- 기존 Asset은 LegacySingleGate + DirectActorKinematics 기본값으로 재저장 없이 현재 행동을 유지하도록 했다.
- SampledPositionEstimate의 최초 샘플, 위치 차분 Velocity Estimate, 응답 필터와 관측 사이 Target 상태 외삽 계약을 구체화했다.
- Reacquisition은 기존 Launch Target Snapshot과 같은 Actor만 허용하고 주변 Actor 자동 Retarget을 금지했다.
- ReacquisitionTimeSeconds와 Pool Reset 요구를 추가했다.
- 교정 후 재검수 P0=0 / P1=0 / P2=0 PASS.
```

### v0.1.0 - 2026-09-07

```text
- CF-TC-027 USER PIE에서 측면 이동 Target 추적·명중은 확인됐지만 지나치게 완벽한 예측성으로 USER Feel 미승인을 기록했다.
- 한 개 Missile의 정답 튜닝 대신 UCFProjectileData별 Guidance Performance Variant를 만드는 방향으로 설계를 교정했다.
- 명중률 직접 확률값을 금지하고 관측·Seeker·기동·추진 성능과 교전 기하에서 결과가 나오도록 고정했다.
- 현재 TargetActor->GetLocation/GetVelocity 직접 관측을 Target observation interval + 위치 샘플 기반 velocity estimator로 단계적으로 교정하도록 설계했다.
- 현재 min(Seeker FOV half-angle, LockBreakAngle) 단일 Gate를 Acquisition / Tracking / LostGrace / Reacquisition / LostFinal 상태 모델로 분리했다.
- 발사 후 Missile Seeker Acquisition과 발사 전 Equipment Lock-On의 소유권을 명시적으로 분리했다.
- 신규 각도는 full FOV 혼동을 피하기 위해 Acquisition/Tracking/Reacquisition Cone Half Angle로 정의했다.
- 정지 Target 미스를 무조건 결함으로 보지 않고, 명확히 Hit가 기대되는 교전 기하와 정상 Miss 가능한 기하를 분리했다.
- Terminal은 명중 보장용 교정 수단으로 사용하지 않도록 경계를 고정했다.
- MG-P0-08~12 후속 구현·검증 단계를 정의했다.
```

---

## 21. Migration

```text
- 이 문서는 MissileGuidancePlan.md의 CF-FQ-030 대표 Plan을 보조하는 상세 설계 문서다.
- Current MG-P0-12E Preset authoring 계약은 `UCFMissileGuidePresetData` passive container + `CarFight_ReEditor` 신규 seed + existing Asset no-seed/no-save + persisted idempotence round-trip이다. Product module은 Low/Normal/High authoring 분기를 소유하지 않는다.
- 기존 MG-P0-00~07의 Technical PASS와 대표 Plan §16.4 Review Correction evidence를 폐기하거나 재해석하지 않는다.
- 기존 DA_Missile_DirectTest는 새 필드 구현 전까지 현재 동작을 유지한다.
- 신규 Config 구현 시 SeekerModel 기본값은 LegacySingleGate, TargetObservationMode 기본값은 DirectActorKinematics, ReacquisitionMode 기본값은 None으로 둔다.
- SeekerModel과 TargetObservationMode는 독립 축으로 유지하며 특정 조합을 Runtime에서 강제하지 않는다.
- SampledPositionEstimate의 Seeker/PN/Overshoot는 Estimated Target State만 소비하고 실제 Actor kinematics를 숨은 보정값으로 다시 읽지 않는다.
- LegacySingleGate의 기존 Overshoot 의미는 보존한다. Stateful에서는 실제 접근(거리 감소 + ForwardClosingVelocity>0)이 한 번 성립해 bOvershootArmed가 된 뒤에만 Overshoot가 Reacquisition보다 우선하는 terminal miss가 된다.
- rear/non-closing ProportionalNavigation은 bounded Course Capture로 접근 기하를 먼저 형성할 수 있으며, 이 단계도 기존 물리 제한을 지킨다.
- UCFMissileGuideComp의 기존 public signature와 ACFProjectileActor의 Start/Reset integration 경계를 유지한다.
- 기존 ECFMissileGuideMode / ECFMissileLostTargetPolicy / ECFMissileMissReason 항목 순서를 변경하지 않는다.
- Stateful Acquisition/Tracking/Reacquisition effective cone은 각각 독립 0~180deg clamp를 사용하며 서로를 Min으로 결합하지 않는다.
- Target Actor가 invalid/destroyed이면 LostGrace 이후 Reacquisition하지 않고 LostFinal로 간다.
- Guidance Window가 닫힌 동안 Sampled observer는 ObservationAgeSeconds만 누적하고 새 Actor sample이나 Seeker 상태 전이를 하지 않는다.
- Sampled observer는 sample interval, velocity sample delta와 EstimatedTargetLocation 외삽에 ObservationAgeSeconds 단일 clock을 사용한다.
- HoldLastKnownPoint는 LostFinal 진입 순간 Sensor Truth 위치를 1회 Freeze하며 이후 estimator 외삽을 적용하지 않는다.
- Stateful Acquiring/LostGrace/Reacquiring은 bTargetValid=false / MissReason=None으로 유지하고 최종 LostFinal에서 TargetLost를 기록한다.
- LostTargetPolicy=Expire는 MG-P0-08~10에서 즉시 Projectile Deactivate를 요구하지 않으며 기존 LifeTime timer ownership을 유지한다.
- 기존 Snapshot.TargetLocation 의미는 LastKnownTargetLocation 호환으로 유지하고 신규 EstimatedTargetLocation을 별도로 추가한다.
- 첫 ECFMissileReacquisitionMode enum은 None / ForwardCone만 포함한다.
- TargetLostGraceTimeSeconds=0과 ReacquisitionTimeSeconds=0은 same-step 상태 전이로 처리한다.
- HoldTargetLocation과 bHasHoldTargetLocation은 매 ResetMissileGuidance에서 반드시 초기화해 Pool activation 간 고정 지점 오염을 금지한다.
- bGuidanceActivationSatisfied / bOvershootArmed / Course Capture 관련 activation-local 상태도 매 ResetMissileGuidance에서 반드시 초기화한다.
- GuidanceActivationMode 기본값은 FollowFlightGuidanceWindow로 두고 기존 IsGuidanceWindowOpen 기반 동작을 보존한다.
- Independent activation은 FlightSnapshot의 elapsed time / release distance를 소비하고 조건 만족 후 activation 동안 latch한다.
- 정확히 180deg rear-aspect steering 특이점은 LaunchContext.LaunchTransform Right Vector deterministic tie-break를 사용하며 random 선택을 금지한다.
- 기존 SeekerFieldOfViewDeg / LockBreakAngleDeg를 즉시 삭제·리네임하지 않는다.
- Stateful 모델에서만 신규 Acquisition/Tracking/Reacquisition Half Angle을 사용한다.
- SampledPositionEstimate가 검증되기 전에 DirectActorKinematics 경로를 삭제하지 않는다.
- Target Snapshot/Pool/Collision/Damage 계약은 MG-P0-08~10 변경에서 보존한다.
- USER Feel은 MG-P0-12에서 다시 판정하며, 2026-09-07의 기존 단일 Turn Feel Gate는 Design Correction을 연 근거로 보존한다.
```
