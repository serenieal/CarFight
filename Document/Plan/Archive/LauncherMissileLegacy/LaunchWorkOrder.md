# LM-P0-01 Work Order

- Version: 0.1.0
- Date: 2026-07-28
- Status: Ready for Browser AI Direct Implementation
- Feature: `CF-FQ-029`
- Task: `LM-P0-01 Projectile Launch Handoff Foundation`

---

## 1. 실행 목표

기존 직사 Projectile의 실제 결과를 유지하면서 `FCFProjectileLaunchContext`가 Vehicle Fire 경로에서 ProjectilePool과 ProjectileActor까지 전달되도록 구현한다.

---

## 2. 실행 전 필수 확인

```text
1. main_game Git branch / upstream / dirty 확인
2. AGENTS.md 확인
3. Document/CodeWorkGate.md 확인
4. LauncherMissilePlan.md 확인
5. LauncherMissileDesign.md 확인
6. LaunchTaskSource.md 확인
7. 아래 수정 후보 파일의 현재 diff 확인
```

기존 dirty가 같은 함수와 겹치면 전체 파일 교체를 금지하고 최소 패치 가능성을 먼저 판단한다.

---

## 3. 구현 순서

### Step 1 — Launch Types

신규 파일:

```text
UE/Source/CarFight_Re/Public/CFProjectileLaunchTypes.h
```

작업:

```text
- 파일 Header, Version, Date, Description, Scope
- ECFProjectileReleaseMode
- FCFProjectileLaunchContext
- 안전 기본값
- 변수 위 한 줄 주석
- Blueprint 노출 시 한국어 DisplayName·ToolTip
- Changelog와 Migration
```

첫 버전:

```text
v1.0.0
```

### Step 2 — ProjectileActor Context 수용

수정 후보:

```text
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
```

작업:

```text
- Context 기반 ActivateProjectile 오버로드 또는 내부 함수
- Legacy ActivateProjectile Adapter
- ActiveLaunchContext Runtime 복사
- bHasActiveLaunchContext
- Deactivate Reset
- Launch Summary 또는 최소 Getter
- 기존 Motor / FX / Collision / Damage 순서 유지
```

주의:

```text
- Activate 순서를 임의로 재배치하지 않는다.
- ActiveProjectileData 설정과 PreviousCollisionLocation 초기화를 유지한다.
- Motor Start에 전달하는 방향은 Context.InitialLaunchDirection을 사용한다.
```

### Step 3 — Initial Velocity

작업:

```text
- Context.InitialLaunchVelocity 안전값 해석
- Legacy 경로는 Direction * InitialSpeed와 동일
- ProjectileMovement Velocity에 결과 적용
- MaxSpeed와 Propulsion 상한 계약 유지
```

NaN 또는 0 Velocity 처리:

```text
InitialLaunchDirection * ProjectileData.InitialSpeed Fallback
```

### Step 4 — ProjectilePool 전달

수정 후보:

```text
UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h
UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp
```

작업:

```text
- Context 기반 AcquireProjectile 경로
- Legacy AcquireProjectile Adapter
- SpawnTransform은 Context.LaunchTransform 사용
- Actor 활성화에 Context 전달
- 기존 ActorClass Pool Key와 반환 정책 유지
```

### Step 5 — Vehicle Direct Context

수정 후보:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

작업:

```text
- 기존 TrySpawnProjectileActorFromFireCommand에서 Direct Context 생성
- AimOrigin, AimDirection, PredictedAimTargetLocation, FireRequestId, WeaponGroupId 전달
- ActiveProjectileData.InitialSpeed로 Legacy Velocity 생성
- InheritedCarrierVelocity=Zero
- ReleaseMode=Direct
- ProjectilePool Context Acquire 호출
```

첫 Task에서 차량 Velocity 상속을 추가하지 않는다.

### Step 6 — Automation

신규 파일:

```text
UE/Source/CarFight_Re/Private/CFProjectileLaunchTests.cpp
```

테스트 이름:

```text
CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract
```

검증:

```text
- Context 기본값
- Legacy Direct 생성값
- Actor Active Context
- Deactivate Reset
- 기존 InitialSpeed
- Motor LaunchDirection
```

---

## 4. 변경하지 않을 항목

```text
- CFTurretMountData
- UCFWeaponData
- TargetSelect
- ProjectileMotorTypes / Comp 알고리즘
- CombatFx
- Damage / Health
- .uasset
- Config
- Build scripts
```

컴파일을 위해 include만 필요한 경우 변경 근거를 보고한다.

---

## 5. 코드 품질 체크

```text
[ ] 신규 파일명 32자 이하
[ ] 모든 변수 바로 위 역할 주석
[ ] 모든 함수 바로 위 역할 주석
[ ] Blueprint 툴팁 한글
[ ] 버전 증가
[ ] Changelog
[ ] Migration
[ ] Legacy Adapter
[ ] Null / NaN / Zero 안전
[ ] Pool Reset
[ ] Tick 추가 없음
```

---

## 6. Diff 검수

수정 직후 다음만 검수한다.

```text
UE/Source/CarFight_Re/Public/CFProjectileLaunchTypes.h
UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h
UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/CFProjectileLaunchTests.cpp
```

확인:

```text
- unrelated 변경 없음
- 기존 dirty 보존
- 전체 파일 교체 없음
- 기존 API 삭제 없음
- 코드 포맷과 주석 규칙 준수
```

---

## 7. 공식 빌드

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

다른 엔진 경로를 사용하지 않는다.

PASS:

```text
- UHT
- CarFight_ReEditor Win64 Development
- 신규 테스트 소스 컴파일
- Link
- Exit Code 0
```

---

## 8. Automation

실행 도구가 노출되어 있으면 다음 테스트를 실행한다.

```text
CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract
```

노출되지 않으면:

```text
Automation Source Compile PASS
Execution Not Run — Runner Unavailable
```

으로 구분한다.

---

## 9. 사용자 PIE 절차

대상:

```text
- 기존 Heavy Cannon
- 기존 비유도 Rocket 테스트 DataAsset
```

검증:

```text
1. 정지 차량에서 기존 Muzzle 위치 발사
2. 기존 Reticle 목표 방향 발사
3. 기존 InitialSpeed 비교
4. Rocket IgnitionDelay / Burning / BurnedOut
5. 벽과 차량 첫 Impact
6. 같은 Pool Actor 반복 발사
```

PASS:

```text
- 이전과 체감 위치·방향·속도 동일
- Damage 1회
- FX와 Motor 상태 정상
- Pool 잔류 없음
```

---

## 10. 완료 보고 형식

```text
Implementation Source: Browser AI Direct
Source Changes: Applied
Diff Review: PASS / FAIL
Build: PASS / FAIL
Automation: PASS / Not Run
PIE: User Validation Pending / PASS / FAIL
Current Task: LM-P0-01
Next Task: LM-P0-02
```

변경 파일, Build Job, Automation 결과, PIE 미검증 항목을 분리해 보고한다.

---

## 11. 문서 동기화

LM-P0-01 완료 후 최소 갱신:

```text
Document/Plan/LauncherMissilePlan.md
Document/Plan/LauncherMissileRoadmap.md
```

코드가 아직 Current System 완료 상태가 아니므로 `Systems` 승격은 하지 않는다.

---

## 12. Changelog

### v0.1.0 - 2026-07-28

```text
- LM-P0-01의 파일별 구현 순서와 Legacy Adapter 전략을 작성했다.
- Initial Velocity, Pool, Vehicle Direct Context와 Automation 절차를 고정했다.
- Diff, 공식 빌드, 사용자 PIE와 완료 보고 형식을 정의했다.
```

---

## 13. Migration

```text
- 이 WorkOrder는 LM-P0-01에만 사용한다.
- Multi-Muzzle와 Ejection 구현에 재사용하지 않고 후속 WorkOrder를 별도로 작성한다.
- 기존 함수 삭제나 리네이밍은 이 작업 범위가 아니다.
```
