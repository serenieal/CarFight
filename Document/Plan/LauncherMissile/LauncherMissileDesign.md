# Launcher / Missile Architecture Design

- Version: 0.1.0
- Date: 2026-07-28
- Status: Approved Design / Pre-Implementation
- Related Features: `CF-FQ-029`, `CF-FQ-030`

---

## 1. 설계 목표

하나의 차량 터렛 리소스와 가변 발사관 구성을 사용해 다음 조합을 데이터 중심으로 확장한다.

```text
- 단일 또는 다연장 비유도 Rocket
- 직접발사 Target Homing Missile
- 대각 사출 후 점화·전환하는 Missile
- 수직 사출 후 Pitch-Over하는 Missile
- Laser Point Guided Missile
- Direct / Loft / Top-Attack 비행 프로파일
```

구현은 조합마다 별도 Actor나 터렛 클래스를 복제하지 않는다.

---

## 2. 전체 책임 구조

```text
ACFVehiclePawn
├─ UCFVehicleWeaponComp
│  └─ 장비 선택, 호환, 쿨다운, 발사 승인과 상위 조정
├─ UCFLauncherComp
│  └─ 발사관, 발사 패턴, 사출 상태와 Launch Context 생성
├─ Turret Visual Components
│  └─ Base / Yaw / Pitch 시각 회전과 Muzzle Transform 제공
└─ UCFProjectilePoolComp
   └─ Projectile Actor 확보와 반환

ACFProjectileActor
├─ UProjectileMovementComponent
├─ UCFProjectileMotorComp
├─ UCFMissileFlightComp
├─ UCFMissileGuideComp
├─ Collision / Continuous Sweep
└─ Trail / Thruster FX
```

`UCFLauncherComp`, `UCFMissileFlightComp`, `UCFMissileGuideComp`는 계획 이름이다.
실제 첫 Task에서는 Launch Context만 추가하며 컴포넌트 생성은 각 단계에서 별도 검토한다.

---

## 3. 도메인 책임 경계

### 3.1 Turret Mount

책임:

```text
- 차량 하드포인트 장착
- Base / Yaw / Pitch 메시
- 회전 제한과 회전 속도
- 피벗 소켓
- 총구를 포함한 시각 장착 구조
```

비책임:

```text
- 발사 패턴
- Ejection 속도
- Missile Guidance
- 점화와 비행 전환
```

### 3.2 Launcher

책임:

```text
- 사용할 Muzzle 선택
- Muzzle 유효성 해석
- Single / Ripple / Salvo 진행 상태
- Direct / Angled / Vertical Release 설정
- 초기 사출 방향과 속도
- 차량 속도 상속
- 발사관 앞 안전 검사
- Launch Context 생성
- 발사관별 FX 요청
```

비책임:

```text
- 발사 후 Projectile 위치 갱신
- 모터 점화 판단
- Missile 방향 전환
- Guidance 계산
- 충돌과 피해
```

### 3.3 Projectile Actor

책임:

```text
- Launch Context 복사
- 독립 위치와 Velocity
- ProjectileMovement
- 충돌·피해·Pool 생명주기
- Motor / Flight / Guidance 컴포넌트 조정
```

### 3.4 Projectile Motor

책임:

```text
- 점화 지연
- 연소 시간
- 추진 가속도
- 최대 추진 속도
- BurnedOut 상태
```

추가 설계:

```text
Rocket
→ FixedLaunchDirection

Missile
→ CurrentVelocityDirection 또는 CurrentForwardDirection
```

첫 Launch Handoff Task에서는 기존 `FixedLaunchDirection`을 변경하지 않는다.

### 3.5 Missile Flight

책임:

```text
- Released / Ejection / Clearance / Ignition / Boost / Transition / GuidedFlight / Terminal
- 사출 후 안전 이탈 조건
- 방향 전환 시작 조건
- Attack Profile 상태 진행
- Guidance 활성 시점
```

### 3.6 Missile Guidance

책임:

```text
- Target Actor 또는 외부 Guidance Point 관측
- 시선 변화율과 요구 횡가속도 계산
- 최대 횡가속도 제한
- 최대 선회율 제한
- 탐색기 FOV와 Lock Break
- 목표 상실 처리
```

비책임:

```text
- Actor 위치 직접 이동
- 목표 방향으로 Velocity 즉시 교체
- 충돌 없이 명중 판정
- 런처 Transform 추종
```

---

## 4. Projectile Launch Handoff

### 4.1 목적

현재 직사 발사에서 결합된 다음 의미를 분리한다.

```text
Command Target
Initial Launch State
Guidance Target
```

### 4.2 계획 타입

```text
파일: CFProjectileLaunchTypes.h
구조체: FCFProjectileLaunchContext
```

계획 필드:

| 필드 | 의미 |
|---|---|
| `LaunchTransform` | 실제 분리 순간 Projectile의 월드 Transform |
| `InitialLaunchDirection` | 런처가 제공한 초기 사출 방향 |
| `InitialLaunchVelocity` | 사출 속도와 차량 속도 상속을 합친 초기 월드 Velocity |
| `InheritedCarrierVelocity` | 발사 플랫폼에서 상속한 월드 Velocity |
| `CommandTargetLocation` | 플레이어 조준 또는 센서가 지시한 월드 목표 위치 |
| `GuidanceTargetActor` | 발사 순간 복사한 선택 대상 참조 |
| `GuidanceTargetLocation` | 좌표·레이저 유도용 초기 목표 위치 |
| `ReleaseMode` | Direct / AngledEjection / VerticalEjection |
| `FireRequestId` | 발사 요청 추적 ID |
| `WeaponGroupId` | 발사 장비 그룹 식별자 |

첫 Task에서 모든 필드를 실제 런타임에 소비할 필요는 없다.
직사 호환에 필요한 필드부터 추가하고 나머지는 후속 단계에서 확장할 수 있다.

### 4.3 생명주기

```text
1. 런처가 Context 생성
2. ProjectilePool이 값 전달
3. ProjectileActor가 자신의 활성화 상태로 복사
4. 런처는 Context를 다시 변경하지 않음
5. Projectile 비활성화 시 Runtime 참조 Reset
```

`GuidanceTargetActor`는 약한 참조 또는 유효성 검사 가능한 형태로 보존한다.
차량의 현재 TargetSelect 선택 변경은 이미 발사된 Projectile Context를 바꾸지 않는다.

---

## 5. 발사 방식

### 5.1 Direct Release

```text
InitialLaunchDirection = Muzzle Socket Forward
InitialLaunchVelocity = Direction * EjectionSpeed + CarrierVelocity
```

사용:

```text
- Cannon Projectile
- 직접발사 Rocket
- 직접발사 Homing Missile
```

### 5.2 Angled Ejection

```text
InitialLaunchDirection
= Muzzle Forward 또는 LauncherData의 Local Ejection Vector
```

흐름:

```text
Released
→ Ejection 탄도 비행
→ Clearance
→ Ignition
→ Transition
→ GuidedFlight
```

### 5.3 Vertical Ejection

```text
InitialLaunchDirection
= 수직 Muzzle Forward 또는 World Up 기반 설정

CommandTargetLocation
= 실제 적 또는 조준 목표
```

초기 방향과 목표 방향은 일치하지 않는다.

```text
상승
→ 안전 거리 또는 시간 확보
→ 점화
→ 제한된 Pitch-Over
→ 정상 유도
```

---

## 6. Ejection 운동 설계

### 6.1 기본 방식

```text
UProjectileMovementComponent
+ Initial Velocity
+ Gravity Scale
+ Sweep / Sub-step
```

최종 초기 속도 개념:

```text
InitialLaunchVelocity
= EjectionDirection * EjectionSpeed
+ CarrierWorldVelocity * CarrierVelocityRatio
```

### 6.2 Simulate Physics 기본 제외 이유

```text
- 발사관과 즉시 충돌하는 불안정
- Chaos에서 ProjectileMovement 전환 시 속도·회전 단절
- Pool Reset 복잡도 증가
- 프레임과 콜리전 미세 오차에 따른 비결정성
- 기존 고속 충돌 계약과 이중 이동 위험
```

### 6.3 Chaos Physics 사용 후보

```text
- 발사관 덮개
- 캐니스터 캡
- 분리 부스터
- 장식 파편
- 별도 물리 실험 탄종
```

---

## 7. 런처 발사 패턴

### 7.1 SingleCycle

```text
입력 1회
→ 현재 Muzzle에서 1발
→ 발사 성공 시에만 다음 Muzzle
```

### 7.2 Ripple

```text
입력 1회
→ 여러 Muzzle을 InterMuzzleDelay 간격으로 순차 발사
```

### 7.3 Salvo

```text
입력 1회
→ 선택 Muzzle에서 같은 프레임 또는 매우 짧은 간격으로 다수 발사
```

발사 거부 시 Muzzle Index를 진행시키지 않는다.
Volley 중 쿨다운은 각 Projectile이 아니라 Volley 정책에 따라 기록한다.

---

## 8. 미사일 비행 상태

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

Motor State와 Flight State는 분리한다.

```text
Motor State
= 추진기관 상태

Flight State
= 미사일 전체 비행 단계
```

예시:

```text
FlightState = Transition
MotorState = Burning
```

동시에 존재할 수 있다.

---

## 9. 물리 제한형 유도

### 9.1 강제 원칙

```text
- 명중 확률 100% 금지
- SetActorLocation 유도 금지
- 목표 방향으로 Velocity 즉시 교체 금지
- 무제한 RInterpTo 금지
- 근거리 강제 흡착 금지
```

### 9.2 입력과 출력

```text
입력
- 현재 위치·Velocity
- 목표 위치·Velocity 추정
- Seeker 상태
- DeltaSeconds

출력
- 허용 범위 안의 Lateral Acceleration Command
```

### 9.3 제한값

```text
MaximumTurnRateDegPerSec
MaximumLateralAccelCmPerSecSq
GuidanceResponseTimeSeconds
SeekerFieldOfViewDeg
LockBreakAngleDeg
MinimumGuidanceSpeed
TargetLostGraceTimeSeconds
```

### 9.4 정상적인 실패

```text
- 최소 사거리 안쪽 발사
- 선회 공간 부족
- 목표의 강한 측면 기동
- 오버슈트
- 탐색기 FOV 이탈
- 장애물 가림
- 목표 파괴
- 연료와 속도 부족
- 레이저 조사 상실
- 수명 종료
```

---

## 10. Target / Laser 연결

### 10.1 Target Actor

```text
발사 순간 TargetSelect 결과 복사
→ Missile이 독립 참조 소유
→ 차량 선택 변경과 무관
```

`CF-FQ-026`의 선택 상태는 읽기 전용 입력이다.
미사일이 TargetSelect의 공용 선택을 변경하지 않는다.

### 10.2 Laser Point

레이저 조사점은 발사 후 갱신될 수 있다.
그러나 다음 원칙을 유지한다.

```text
레이저 위치
= Guidance Input
≠ Projectile 위치 또는 Velocity 강제값
```

레이저 상실 기본 P0 후보:

```text
ContinueStraight
```

`LastKnownPoint`, `SelfDestruct`는 후속 데이터 정책으로 분리한다.

---

## 11. 데이터 구조 계획

### 11.1 Launcher Data

계획 클래스:

```text
UCFLauncherData
```

계획 필드군:

```text
Identity
MuzzleSocketNames
FirePattern
ProjectileCountPerTrigger
InterMuzzleDelaySeconds
ReleaseMode
LocalEjectionDirection
EjectionSpeed
CarrierVelocityRatio
LauncherClearanceTraceDistance
```

### 11.2 ProjectileData 확장

별도 Missile DataAsset을 즉시 추가하지 않는다.
기존 `UCFProjectileData`에 중첩 설정 구조체를 추가하는 방향을 우선한다.

```text
FCFMissileFlightConfig
FCFMissileGuideConfig
```

기존 ProjectileData 기본값은 모두 비활성으로 유지한다.

---

## 12. C++ / Blueprint / Editor 배분

### C++

```text
- Launch Context
- Muzzle 해석과 발사 순서
- Volley 상태
- Ejection 초기 Velocity
- Flight State
- Guidance 계산과 제한
- Pool Reset
- Runtime Debug
- Automation
```

### DataAsset / Editor

```text
- Muzzle 배열과 순서
- Release Mode와 Ejection 값
- Fire Pattern과 간격
- Missile Flight / Guidance 수치
- Projectile Mesh·FX
- Socket 위치·축
- PIE 감각과 시각 튜닝
```

### Blueprint

```text
- HUD 표시
- 선택적 디버그 시각화
- 에디터 테스트 프리셋
```

Blueprint Tick에서 유도·추진·Volley 상태 머신을 중복 구현하지 않는다.

---

## 13. 계획 파일명과 클래스명

모든 신규 파일명은 32자 이하로 유지한다.

```text
CFProjectileLaunchTypes.h
CFLauncherTypes.h
CFLauncherData.h / .cpp
CFLauncherComp.h / .cpp
CFMissileFlightTypes.h
CFMissileFlightComp.h / .cpp
CFMissileGuideTypes.h
CFMissileGuideComp.h / .cpp
```

---

## 14. 제외 범위

```text
- 네트워크 복제와 서버 권한 Missile
- 레이더 전파·RCS 시뮬레이션
- 실제 6-DOF 공기역학 완전 시뮬레이션
- 플레어·재밍·연막 Countermeasure
- 근접신관과 범위 폭발
- 다단 분리 애니메이션
- 탄약 재장전 UI 완성
- 게임 Audio
```

---

## 15. Changelog

### v0.1.0 - 2026-07-28

```text
- 런처, Projectile, Motor, Missile Flight와 Guidance의 책임 경계를 정의했다.
- Projectile Launch Handoff Context를 공통 경계로 확정했다.
- Direct, Angled Ejection과 Vertical Ejection의 데이터 흐름을 분리했다.
- Ejection 기본 이동을 ProjectileMovement 기반으로 결정했다.
- SingleCycle, Ripple, Salvo 발사 패턴 의미를 정의했다.
- Motor State와 Missile Flight State를 분리했다.
- 유도 명중 비보장과 물리 제한 강제 원칙을 확정했다.
- Target Actor Snapshot과 Laser Point 외부 입력의 독립 이동 계약을 정의했다.
```

---

## 16. Migration

```text
- 기존 TurretMountData의 단일 MuzzleSocketName은 즉시 삭제하지 않는다.
- 기존 FCFVehicleFireRequest와 FCFVehicleFireOrigin 필드는 호환을 위해 유지한다.
- 기존 ProjectilePool Acquire와 ProjectileActor Activate 경로는 Launch Context 어댑터를 거쳐 단계적으로 이전한다.
- 기존 Rocket은 FixedLaunchDirection을 유지한다.
- 신규 Missile만 별도 Flight / Guidance 설정을 활성화한다.
```
