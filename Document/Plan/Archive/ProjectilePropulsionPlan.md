# Projectile Propulsion Implementation Plan

- Version: 1.2.0
- Date: 2026-07-28
- Status: Completed / Systems Promoted / CF-FQ-028 Done / CF-TC-024 PASS
- Feature ID: `CF-FQ-028`
- Feature Name: 발사체 추진 시스템
- Planned Test ID: `CF-TC-024`
- Representative Plan: `Document/Plan/ProjectilePropulsionPlan.md`

---

## 1. 목적

CarFight의 공통 Projectile Actor에 로켓·미사일이 사용할 실제 자체 추진 비행 기반을 추가한다.

첫 구현 목표는 다음 한 문장으로 고정한다.

> 비유도 로켓이 InitialSpeed로 발사된 뒤 선택적 점화 지연을 거쳐 고정 발사 방향으로 일정 시간 가속하고, 연소 종료 후 기존 속도와 중력으로 관성 비행하며, 실제 Burning 상태에서만 추진 화염 FX를 재생하게 한다.

이 기능은 차량 부스터가 아니다.
차량 Pawn이나 Chaos VehicleMovement에 힘을 추가하지 않는다.

---

## 2. 확정 분류

CarFight에서 Rocket과 Missile은 자체 추진 여부만으로 구분하지 않는다.

```text
발사 후 유도 정보를 사용하여 진로를 수정하는가?
```

| 분류 | 자체 추진 | 발사 후 유도 정보로 진로 수정 |
|---|---:|---:|
| 일반 포탄 | 없음 | 없음 |
| Rocket | 있음 | 없음 |
| Missile | 있음 | 있음 |

`CF-FQ-028`의 P0 대상은 `Rocket`이다.
Target Homing, Laser Guided, 수동 유도와 탐색기 로직은 후속 Guidance 기능으로 분리한다.

---

## 3. 현재 선행 구현

현재 Projectile 런타임은 다음 기반을 제공한다.

```text
UCFProjectileData
ACFProjectileActor
UCFProjectilePoolComp
UProjectileMovementComponent
Sweep / Sub-step / Supplemental Sphere Sweep
첫 유효 Impact 단일 처리
DamageHitContext / DamageApplyResult
TrailFxSettings / ThrusterFxSettings
```

`CF-FQ-027`에서 Trail과 Thruster Niagara의 소켓/Fallback/Pool Reset C++ 기반은 적용됐다.
그러나 기존 이동은 다음과 같았다.

```text
InitialSpeed 적용
MaxSpeed = InitialSpeed
추가 가속 없음
Thruster FX는 Projectile 활성 시간 전체에 재생 가능
```

따라서 외형상 로켓처럼 보일 수 있어도 실제 자체 추진 운동은 존재하지 않았다.

---

## 4. P0 비행 상태

```text
Inactive
→ IgnitionDelay
→ Burning
→ BurnedOut
→ Hit 또는 LifeExpired
```

| 상태 | 의미 |
|---|---|
| `Inactive` | 발사 전, Pool 보관 또는 비활성화 후 Reset 상태 |
| `Disabled` | 이 ProjectileData가 실제 추진을 사용하지 않는 기존 포탄 상태 |
| `IgnitionDelay` | 발사됐지만 로켓 모터가 아직 점화되지 않은 상태 |
| `Burning` | 고정 발사 방향으로 실제 추진 가속을 적용하는 상태 |
| `BurnedOut` | 연소가 종료되어 추가 가속 없이 관성·중력 비행하는 상태 |

중요 계약:

```text
BurnedOut != Projectile Deactivate
```

연소 종료는 추가 가속과 추진 화염만 종료한다.
충돌, 피해, Trail, 수명과 Pool 생명주기는 계속 유지한다.

---

## 5. 데이터 계약

### 5.1 타입 파일

```text
UE/Source/CarFight_Re/Public/CFProjectileMotorTypes.h
```

### 5.2 `FCFProjectilePropulsionConfig`

| 필드 | 기본값 | 의미 |
|---|---:|---|
| `bUsePropulsion` | `false` | 실제 추진 시스템 사용 여부 |
| `IgnitionDelaySeconds` | `0.05` | 발사 후 Burning 진입 전 대기 시간 |
| `BurnDurationSeconds` | `1.0` | 실제 추진 가속이 발생하는 총 시간 |
| `ThrustAccelerationCmPerSecSq` | `9000` | 고정 추진 방향 가속도, 단위 cm/s² |
| `MaximumPropelledSpeed` | `10000` | 추진 가속으로 허용할 최대 속도, 단위 cm/s |

기존 ProjectileData 호환 규칙:

```text
bUsePropulsion=false
→ MaxSpeed=InitialSpeed
→ 기존 포탄 이동 유지
→ ProjectileMotorState=Disabled
→ Thruster FX 재생하지 않음
```

추진 Projectile 규칙:

```text
bUsePropulsion=true
→ InitialSpeed로 발사대에서 분리
→ MaxSpeed=max(MaximumPropelledSpeed, InitialSpeed)
→ 점화 지연 후 고정 LaunchDirection 가속
→ BurnDuration 종료 후 BurnedOut
```

### 5.3 `FCFProjectileMotorSnapshot`

Debug와 Blueprint 읽기용 값:

```text
CurrentMotorState
ElapsedFlightTimeSeconds
ElapsedBurnTimeSeconds
RemainingBurnTimeSeconds
CurrentSpeedCmPerSec
CurrentThrustDirection
bIsProducingThrust
bThrusterFxShouldBeActive
MotorActivationCount
```

---

## 6. C++ 책임

### 6.1 `UCFProjectileMotorComp`

파일:

```text
UE/Source/CarFight_Re/Public/CFProjectileMotorComp.h
UE/Source/CarFight_Re/Private/CFProjectileMotorComp.cpp
```

책임:

```text
- PropulsionConfig 복사와 안전값 해석
- 발사 시 LaunchDirection 정규화와 고정 저장
- 점화 지연 시간 진행
- 실제 Burning 시간 진행
- ProjectileMovement Velocity에 가속도 적용
- 최대 추진 속도 제한
- BurnedOut 상태 전환
- Pool 재사용을 위한 Reset
- 상태 변경 이벤트
- 상태 스냅샷과 Debug 요약
```

비책임:

```text
- 목표 탐색
- TargetSelect 조회
- Lock-on 검증
- 비행 방향 유도
- 충돌·피해·Pool 반환 최종 처리
- Niagara 자산 선택
```

### 6.2 `ACFProjectileActor`

책임:

```text
- ProjectileMotorComp 기본 서브오브젝트 생성
- ProjectileMovement보다 MotorComp가 먼저 Tick하도록 선행 조건 연결
- ActivateProjectile에서 PropulsionConfig와 LaunchDirection 전달
- DeactivateProjectileWithReason에서 MotorComp Reset
- Motor 상태 변경을 Thruster Niagara에 연결
- 기존 충돌·피해·Pool 순서 보존
```

### 6.3 `UCFProjectileData`

책임:

```text
- PropulsionConfig 소유
- 기존 InitialSpeed 의미를 발사대 분리 속도로 유지
- 추진 설정을 Debug 요약에 표시
- 기존 데이터는 기본 비활성으로 호환 유지
```

---

## 7. 추진 계산 계약

P0 추진 방향은 발사 시 전달된 `LaunchDirection`으로 고정한다.

```text
FixedThrustDirection = SafeNormal(LaunchDirection)
```

Burning 중 속도 갱신:

```text
Velocity += FixedThrustDirection
          * ThrustAccelerationCmPerSecSq
          * AppliedBurnDurationSeconds
```

속도 제한:

```text
Velocity = ClampMagnitude(
    Velocity,
    max(MaximumPropelledSpeed, CurrentSpeedBeforeThrust)
)
```

잘못된 `MaximumPropelledSpeed < InitialSpeed` 설정이 발사 직후 속도를 강제로 낮추지 않는다.

P0에서 추진 방향은 목표, Actor 회전, 현재 Velocity, Reticle 또는 TargetSelect에 의해 변경되지 않는다.
이 원칙이 비유도 Rocket과 후속 Missile Guidance의 책임 경계를 만든다.

---

## 8. Thruster FX 동기화

### 8.1 Trail

```text
Projectile 활성화 시 시작
→ 비행 중 유지
→ Hit / LifeExpired / Manual / InvalidActivation에서 Reset
```

### 8.2 Thruster

```text
Projectile 활성화 시 Asset과 부착 위치만 준비
→ MotorState == Burning일 때 Activate
→ IgnitionDelay / Disabled / BurnedOut에서 정지
→ Projectile 비활성화 시 Asset까지 전체 Reset
```

| Motor State | Thruster FX 상태 |
|---|---|
| `Inactive` | `Inactive` |
| `Disabled` | `MotorDisabled` |
| `IgnitionDelay` | `IgnitionPending` |
| `Burning` | `Active` 또는 `ActivationRequested` |
| `BurnedOut` | `BurnedOut` |

의존 방향:

```text
Motor State → Thruster FX
```

Niagara 성공 여부가 추진 계산을 변경하는 역방향 의존은 금지한다.

---

## 9. C++ / Blueprint / Editor 배분

### C++

```text
- 상태 머신
- 시간 진행
- 가속과 속도 제한
- Pool Reset
- 기존 Projectile 호환
- Thruster 상태 이벤트
- Debug와 Automation
```

### Blueprint / DataAsset / Editor

```text
- 실제 Rocket ProjectileData 생성 또는 기존 테스트 데이터 복제
- PropulsionConfig 수치 입력
- ProjectileStaticMesh 선택
- FX_Exhaust 소켓 또는 Fallback Transform 조정
- Thruster Niagara 선택
- Trail Niagara 선택
- PIE 비행감과 시각 튜닝
```

Blueprint Tick에서 가속이나 상태 머신을 중복 구현하지 않는다.

---

## 10. P0 제외 범위

```text
- Target Homing Missile
- Laser Guided Missile
- 수동 유도
- GuidanceComp
- 목표 상실 정책
- 탐색기 시야각과 유도 갱신 주기
- 다단 추진
- 추진 출력 Curve
- 공기저항과 항력 계수
- 추력 편향
- 측면·수직 추력
- 연료량 UI
- 근접신관
- 폭발 범위 피해
- 플레어·연막·재밍
- 미사일 경고 UI
- 네트워크 복제
- 게임 사운드
```

---

## 11. 구현 마일스톤

| ID | 작업 | 상태 |
|---|---|---|
| `PP-P0-00` | 추진 계약·범위·분류 확정 | Done |
| `PP-P0-01` | Motor Types와 PropulsionConfig | Applied |
| `PP-P0-02` | ProjectileMotorComp 상태 머신 | Applied |
| `PP-P0-03` | Actor 활성화·비활성화·Pool Reset 연결 | Applied |
| `PP-P0-04` | 가속·최대 속도·BurnedOut 관성 비행 | Applied |
| `PP-P0-05` | Thruster FX를 Burning 상태와 동기화 | Applied |
| `PP-P0-06` | RuntimeContract Automation 소스 | Applied / Source Compile PASS / Execution Pending |
| `PP-P0-07` | 공식 Unreal Editor 빌드 | Done / Latest Build Job `e8b812bd479549299dd116f9bae8996f` / Exit Code 0 |
| `PP-P0-08` | 테스트용 Rocket DataAsset 작성 | Done / `DA_PFX_ThrusterTest` |
| `PP-P0-09` | 사용자 PIE 비행·FX·Pool 검증 | Done / User PIE PASS / `CF-TC-024 PASS` |
| `PP-P0-10A` | Current Systems 문서 승격 | Done / `Document/Systems/Combat/Projectile.md` v1.4.0 |
| `PP-P0-10B` | 반복 전투 확장 회귀 | Moved / `CF-FQ-019` Candidate / Not Started |

---

## 12. 자동화 계약

테스트 소스:

```text
UE/Source/CarFight_Re/Private/CFProjectileMotorTests.cpp
```

테스트 이름:

```text
CarFight.ProjectilePropulsion.PP_P0_01.RuntimeContract
```

검증 항목:

```text
- bUsePropulsion 기본 false
- 추진 기본값
- ProjectileMotorComp 기본 서브오브젝트
- InitialSpeed 발사
- IgnitionDelay 중 비가속
- 점화 지연 이후 같은 프레임 잔여 시간만 Burning 처리
- BurnDuration 종료
- MaximumPropelledSpeed 제한
- BurnedOut 후 Projectile 활성 유지
- Deactivate 시 Inactive Reset
- 기존 비추진 Projectile의 MaxSpeed=InitialSpeed 회귀
```

현재 Admin 표면에는 Unreal Automation 실행 도구가 없다. 따라서 공식 Editor 빌드에서 테스트 소스 컴파일까지 검증했으며, 실제 Automation 실행은 사용 가능한 실행 경로 또는 사용자 Editor Session Frontend에서 수행한다.

빌드 증거:

```text
첫 빌드: 984f0ccab94241148e7b40b9d30a5572 / Exit Code 6
첫 결함: FVector::Size() 반환 double과 float 기대값의 TestEqual 오버로드 충돌
수정: 속도 기대값을 double로 통일
추진 Foundation 빌드: 2ffd09357e654bb7970a2e379f5ab45f / Exit Code 0
Scale 보정 최종 빌드: e8b812bd479549299dd116f9bae8996f / Exit Code 0
검증: UHT, MotorTypes, MotorComp, ProjectileData v1.7.1, ProjectileActor v1.8.1, FlightFxTests v1.1.0, Automation 소스 컴파일과 링크 PASS
Automation 실행: Not Run
사용자 PIE: PASS / CF-TC-024 PASS
```

---

## 13. PIE 검증 항목

### 이동

```text
- 로켓이 InitialSpeed로 분리된다.
- IgnitionDelay 동안 속도가 증가하지 않는다.
- Burning에서 눈에 띄게 가속한다.
- 최대 추진 속도를 초과하지 않는다.
- BurnDuration 종료 후 가속이 멈춘다.
- BurnedOut 뒤에도 충돌 또는 수명 종료까지 계속 날아간다.
- 중력 설정이 전체 비행에 계속 적용된다.
```

### FX

```text
- Trail은 비행 중 유지된다.
- Thruster는 IgnitionDelay에 재생되지 않는다.
- Thruster는 Burning에서만 재생된다.
- Thruster는 BurnedOut에서 종료된다.
- Niagara 미연결이어도 추진·충돌·피해가 유지된다.
```

### Pool / 회귀

```text
- Hit과 LifeExpired에서 MotorComp와 Niagara가 Reset된다.
- 같은 Projectile Actor 재사용 시 이전 연소 시간이 남지 않는다.
- 기존 포탄 속도와 충돌이 변하지 않는다.
- 첫 Impact 단일 피해와 Supplemental Sweep이 유지된다.
```

### 13.1 사용자 PIE 최종 결과 — 2026-07-28

```text
- DA_PFX_ThrusterTest 실제 발사와 추진 동작 정상
- IgnitionDelay → Burning → BurnedOut 상태 표현 정상
- Thruster FX 연소 상태 동기화 정상
- FX_Exhaust 소켓 부착 정상
- RelativeTransform.Scale 0.2 적용 시 추진 화염 축소 정상
- Scale 1.0과 0.2 시각 차이 확인
- 사용자 확인 기준 나머지 비행·FX 동작 정상
- CF-TC-024 PASS
```

Scale 결함 원인과 수정:

```text
기존:
유효 FX_Exhaust 소켓 경로가 Origin Scale을 1,1,1로 강제

수정:
Location / Rotation은 소켓 우선
Scale은 소켓·Fallback 공통으로 RelativeTransform.Scale 적용

최종 코드:
UCFProjectileData v1.7.1
ACFProjectileActor v1.8.1
CFProjectileFlightFxTests v1.1.0

최종 공식 빌드:
e8b812bd479549299dd116f9bae8996f / Exit Code 0
```

Systems 승격은 완료됐다. 반복 전투 확장 회귀는 이 Plan의 미완료 단계가 아니라 별도 `CF-FQ-019 주행/전투 반복 테스트` Candidate로 이관했으며 자동 착수하지 않는다.

---

## 14. 초기 로켓 튜닝값

첫 Editor DataAsset 시작값:

```text
InitialSpeed                     = 3000 cm/s
bUsePropulsion                   = true
IgnitionDelaySeconds             = 0.05 s
BurnDurationSeconds              = 1.0 s
ThrustAccelerationCmPerSecSq     = 9000 cm/s²
MaximumPropelledSpeed            = 10000 cm/s
LifeTimeSeconds                  = 4.0 s
bAffectedByGravity               = true
GravityScale                     = 0.35
CollisionRadius                  = 8~15 cm
```

이 값은 최종 밸런스가 아니라 점화·가속·연소 종료를 시각적으로 구분하기 위한 첫 시작값이다.

---

## 15. 후속 Guidance 경계

향후 Missile 구현은 다음 별도 컴포넌트 후보로 분리한다.

```text
UCFProjectileGuidanceComp
```

책임 분리:

```text
ProjectileMotorComp
→ 속력과 추진 연소를 만든다.

ProjectileGuidanceComp
→ 목표 또는 유도 지점에 따라 추진 방향을 변경한다.

ProjectileActor
→ 충돌, 피해, Pool과 전체 생명주기를 관리한다.
```

Target Homing Missile은 유효 Lock-on 완료 전 발사를 금지하고 Dumb Fire를 허용하지 않는 CombatPlan 결정을 유지한다.

---

## 16. 변경 파일

신규:

```text
UE/Source/CarFight_Re/Public/CFProjectileMotorTypes.h
UE/Source/CarFight_Re/Public/CFProjectileMotorComp.h
UE/Source/CarFight_Re/Private/CFProjectileMotorComp.cpp
UE/Source/CarFight_Re/Private/CFProjectileMotorTests.cpp
```

수정:

```text
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileData.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
```

현재 기존 `.uasset`은 자동 변경하지 않는다.

---

## 17. Migration

### 기존 ProjectileData

```text
- PropulsionConfig는 C++ 기본값으로 추가된다.
- bUsePropulsion=false이므로 기존 포탄 비행은 유지된다.
- 기존 DataAsset을 즉시 재저장할 필요가 없다.
- ThrusterFxSettings가 연결돼 있더라도 Motor가 Disabled이면 추진 화염은 재생하지 않는다.
```

### 신규 Rocket ProjectileData

```text
- bUsePropulsion=true로 설정한다.
- InitialSpeed는 발사대 분리 속도로 설정한다.
- BurnDuration과 Acceleration을 먼저 튜닝하고 MaximumPropelledSpeed를 안전 상한으로 둔다.
- FX_Exhaust 소켓 또는 RelativeTransform을 확인한다.
- Guidance 관련 값은 추가하지 않는다.
```

---

## 17.1 완료 후 문서 역할

```text
현재 구현 기준:
Document/Systems/Combat/Projectile.md

완료 이력과 설계·빌드·PIE 근거:
Document/Plan/ProjectilePropulsionPlan.md

후속 반복 전투 회귀:
CF-FQ-019 Candidate / Not Started
```

이 문서는 `CF-FQ-028`의 완료 기록으로 유지한다. 현재 런타임 책임이나 데이터 계약을 확인할 때는 Systems 문서를 우선한다.

---

## 18. Changelog

### v1.2.0 - 2026-07-28

```text
- CF-FQ-028의 검증된 비유도 Rocket 추진 구현을 Document/Systems/Combat/Projectile.md v1.4.0으로 승격했다.
- Projectile Current System에 PropulsionConfig, ProjectileMotorComp, Motor 상태, Burning 기반 Thruster와 독립 FX Scale을 통합했다.
- Systems/SystemIndex.md와 CombatFx 책임 경계를 현재 구현에 맞게 갱신했다.
- PP-P0-10A Systems 승격을 Done으로 전환했다.
- 반복 전투 확장 회귀는 PP-P0-10B 미완료가 아니라 별도 CF-FQ-019 Candidate로 이관했으며 이번 작업에서 수행하지 않았다.
- CF-FQ-028을 Done / CF-TC-024 PASS로 종료하고 이 Plan을 완료 이력 문서로 전환했다.
```

### v1.1.0 - 2026-07-28

```text
- DA_PFX_ThrusterTest의 추진과 FX 기본 동작은 정상이고 Scale만 반영되지 않는 사용자 PIE 결함을 확인했다.
- 유효 FX_Exhaust 소켓 경로가 FX Origin Scale을 1,1,1로 덮어쓰던 원인을 수정했다.
- RelativeTransform의 Scale을 소켓·Fallback 공통 독립 FX Scale로 적용했다.
- UCFProjectileData v1.7.1, ACFProjectileActor v1.8.1과 CFProjectileFlightFxTests v1.1.0으로 갱신했다.
- 최종 Build Job e8b812bd479549299dd116f9bae8996f에서 UHT·컴파일·링크 Exit Code 0을 확인했다.
- 사용자 재검증에서 Scale 0.2 적용 시 추진 화염 축소가 정상 동작함을 확인했다.
- PP-P0-08과 PP-P0-09를 Done, CF-TC-024를 PASS로 전환했다.
- 다음 단계를 PP-P0-10 Systems 승격과 반복 전투 회귀로 이동했다.
```

### v1.0.0 - 2026-07-28

```text
- CF-FQ-028 비유도 로켓 추진 계약을 확정했다.
- ProjectileMotorTypes와 ProjectileMotorComp C++ 기반을 추가했다.
- ProjectileData에 PropulsionConfig를 추가했다.
- ProjectileActor 활성화·비활성화·Pool 생명주기에 MotorComp를 연결했다.
- 점화 지연, 고정 발사 방향 가속, 최대 추진 속도와 BurnedOut 관성 비행을 구현했다.
- Thruster FX를 실제 Burning 상태와 동기화했다.
- RuntimeContract Automation 소스를 추가했다.
- 첫 빌드에서 Automation의 double-float TestEqual 오버로드 오류를 확인하고 기대값 타입을 수정했다.
- 최종 형식 정리 후 Build Job `2ffd09357e654bb7970a2e379f5ab45f`에서 Unreal Editor Development 빌드 Exit Code 0을 확인했다.
- Automation은 소스 컴파일 PASS와 실제 실행 Not Run을 분리했다.
- Editor DataAsset과 사용자 PIE 검증은 Pending으로 유지했다.
```
