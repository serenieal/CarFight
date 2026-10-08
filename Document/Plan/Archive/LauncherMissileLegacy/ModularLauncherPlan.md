# Modular Launcher Implementation Plan

- Version: 0.6.0
- Date: 2026-07-30
- Status: Active / Direct Ripple·SingleCycle User PIE PASS / Damage Blocked / Salvo·Ejection Pending
- Feature ID: `CF-FQ-029`
- Planned Tests: `CF-TC-025`, `CF-TC-026`

---

## 1. 목적

발사구 개수가 달라져도 동일한 코드로 동작하고, 발사 패턴과 사출 방식이 DataAsset 설정으로 교체되는 범용 런처 기반을 구현한다.

```text
4연장 Rocket Launcher
6연장 Homing Missile Launcher
수직발사 Laser Guided Launcher
```

각 조합을 별도 터렛 C++ 클래스로 만들지 않는다.

---

## 2. 런처 책임

```text
- 발사관 목록과 순서
- Muzzle Transform
- Single / Ripple / Salvo
- Direct / Angled / Vertical Release
- 사출 방향·속도
- 차량 속도 상속
- 총구 안전 검사
- Volley 상태와 쿨다운 인계
- Launch Context 생성
```

런처 비책임:

```text
- 발사 후 Projectile 위치
- 모터 점화
- Missile Guidance
- 충돌·피해
- Pool 반환
```

---

## 3. 데이터 계층

### 3.1 계획 클래스

```text
UCFLauncherData
```

계획 파일:

```text
CFLauncherTypes.h
CFLauncherData.h
CFLauncherData.cpp
```

### 3.2 Identity

```text
LauncherId
```

### 3.3 Muzzle

```text
MuzzleSocketNames
bRequireAllMuzzles
```

해석 순서:

```text
1. MuzzleSocketNames의 유효 소켓을 배열 순서대로 사용
2. 배열이 비어 있으면 TurretMountData.MuzzleSocketName 사용
3. 둘 다 없으면 기존 FireOrigin Fallback 또는 준비 실패 정책
```

기존 단일 Muzzle 에셋은 재저장 없이 동작해야 한다.

### 3.4 Fire Pattern

계획 enum:

```text
ECFLauncherFirePattern
- SingleCycle
- Ripple
- Salvo
```

계획 필드:

```text
FirePattern
ProjectileCountPerTrigger
InterMuzzleDelaySeconds
bCycleStartMuzzle
```

### 3.5 Release

계획 enum:

```text
ECFProjectileReleaseMode
- Direct
- AngledEjection
- VerticalEjection
```

계획 필드:

```text
ReleaseMode
LocalEjectionDirection
EjectionSpeed
CarrierVelocityRatio
EjectionGravityScale
LauncherClearanceTraceDistanceCm
```

---

## 4. 런타임 계층

### 4.1 현재 컴포넌트

```text
UCFLauncherComp v1.0.0
```

현재 구현 책임:

```text
- 첫 승인 발사 이후 Ripple·Salvo Dispatch
- Volley ID와 시도·승인·실패·남은 수량 집계
- 첫 입력 순간 Command Target 고정
- 추가 입력 중복 방지
- 실패 정책과 차량·무기·터렛 변경 취소
- Volley 단위 쿨다운 완료 시점 인계
```

`LM-P0-04`에서 Release 계산을 `WeaponData → WeaponComp → Pawn Launch Context → Pool·Projectile Actor` 경로에 적용했다. `UCFLauncherComp`는 기존 시퀀스 소유 책임을 유지하며 각 Ripple·Salvo 발사가 같은 Release 설정을 사용한다.

`LM-P0-05`에서 실제 `DA_RocketBody`, `RocketLauncherYaw·Pitch`, `Muzzle_1~4`, `DA_RocketLauncher`, `RocketLauncher` EquipmentPresetData, `DA_TestSUV`, `TestMap`과 `BP_CFVehiclePawn` CDO를 연결했다.

저장값은 독립 AssetDump와 새 Unreal 프로세스에서 재검증했으며 현재 단계는 `LM-P0-06 CF-TC-025·026 사용자 PIE`다.

### 4.2 런타임 상태

```text
NextMuzzleIndex
ActiveVolleyId
RemainingVolleyShots
NextVolleyShotTimeSeconds
bVolleyInProgress
LastResolvedMuzzleName
```

### 4.3 소유권

```text
UCFVehicleWeaponComp
= 상위 장비·쿨다운·발사 승인

UCFLauncherComp
= Muzzle·Volley·Release 실행
```

두 컴포넌트가 같은 쿨다운 상태를 중복 소유하지 않는다.

---

## 5. Muzzle 선택 규칙

```text
- 배열 순서는 실제 발사 순서다.
- 유효 Muzzle만 Runtime 목록에 포함한다.
- bRequireAllMuzzles=true이고 하나라도 누락되면 준비 실패다.
- false이면 누락 항목을 건너뛰고 유효 항목만 사용한다.
- 발사 거부 시 Index를 진행하지 않는다.
- 발사 성공 후 다음 Index로 순환한다.
```

4연장 초기 권장 순서:

```text
Muzzle_1
Muzzle_4
Muzzle_2
Muzzle_3
```

대각 교차 순서는 시각적 편향과 차량 흔들림을 줄이기 위한 초기 후보이며 PIE에서 확정한다.

---

## 6. 발사 패턴 상세

### 6.1 SingleCycle

```text
한 입력에 한 발
성공 후 다음 Muzzle
```

P0 첫 다연장 구현으로 사용한다.

### 6.2 Ripple

```text
한 입력에 여러 발
InterMuzzleDelaySeconds 간격
```

중간 실패 정책은 Data 또는 Runtime 정책으로 명시한다.
P0 기본 후보:

```text
실패한 Muzzle은 해당 발만 거부
남은 Volley는 계속
```

차량 파괴·장비 비활성화는 전체 Volley를 취소한다.

### 6.3 Salvo

```text
한 입력에 여러 발
같은 프레임 또는 최소 간격
```

Pool 확보 실패 시 이미 발사된 Projectile을 되돌리지 않는다.
부분 Salvo 결과를 Debug에 기록한다.

---

## 7. Release 계산

### 7.1 Direct

```text
Direction = Muzzle Forward
```

### 7.2 AngledEjection

```text
Direction
= Muzzle Transform.TransformVectorNoScale(LocalEjectionDirection)
```

### 7.3 VerticalEjection

기본 후보:

```text
Direction = Muzzle Forward
```

수직 발사관 소켓의 X축을 위로 설정한다.
World Up 강제 옵션은 후속 호환 설정으로 검토한다.

### 7.4 초기 Velocity

```text
EjectionVelocity = Direction * EjectionSpeed
CarrierVelocity = VehicleVelocity * CarrierVelocityRatio
InitialLaunchVelocity = EjectionVelocity + CarrierVelocity
```

기존 직사 Projectile 호환 기본값은 기존 결과를 유지해야 한다.
신규 Ejection Launcher Data에서 차량 속도 상속 `1.0`을 권장한다.

---

## 8. 발사 안전 검사

검사 기준:

```text
각 발사에 실제 선택된 Muzzle 위치
+ 실제 InitialLaunchDirection
```

금지:

```text
- 대표 Muzzle 하나로 모든 발사관 검사
- CommandTarget 방향으로 Vertical Ejection 안전 검사
```

발사관 바로 앞 비피해 장애물만 사전 거부한다.
더 먼 장애물은 실제 Projectile 충돌로 처리한다.

---

## 9. Launch Context 생성

런처가 채우는 값:

```text
LaunchTransform
InitialLaunchDirection
InitialLaunchVelocity
InheritedCarrierVelocity
CommandTargetLocation
ReleaseMode
FireRequestId
WeaponGroupId
```

Guidance Target은 상위 Weapon / Target 평가 결과에서 전달받아 Context에 복사할 수 있지만 런처가 추적 상태를 소유하지 않는다.

---

## 10. Cooldown과 Volley

기본 원칙:

```text
- SingleCycle: 승인된 1발에서 Cooldown 기록
- Ripple / Salvo: Trigger 또는 Volley 단위 Cooldown
- 각 내부 Projectile마다 전체 무기 Cooldown을 다시 시작하지 않음
```

정확한 시작 시점 후보:

```text
Volley 첫 발 승인 시
```

마지막 발 시점 시작 방식은 발사 리듬이 과도하게 느려질 수 있어 대안으로만 둔다.

---

## 11. Debug 계약

```text
LauncherId
FirePattern
ReleaseMode
MuzzleCount
ValidMuzzleCount
CurrentMuzzleIndex
CurrentMuzzleName
VolleyId
RemainingShots
EjectionSpeed
InheritedCarrierVelocity
LastLaunchSummary
```

디버그 값은 발사 결과와 데이터 문제를 분리할 수 있어야 한다.

---

## 12. 구현 단계

```text
LM-P0-01 Launch Handoff — Code Applied / Build PASS
LM-P0-02 Muzzle Data + SingleCycle — Code Applied / Build PASS / PIE Pending
LM-P0-03A Pattern Data Contract — Code Applied / Build PASS
LM-P0-03B Ripple / Salvo Scheduler — Code Applied / Build PASS / PIE Pending
LM-P0-04 Angled / Vertical Ejection — Code Applied / Build PASS
LM-P0-05 Editor Assets — Done / Assets Applied / Independent AssetDump PASS
LM-P0-06 Verification — Active / Direct Ripple·SingleCycle PASS / Salvo·실패·취소·Ejection·Carrier Velocity Pending
```

---

## 13. Automation 계획

### CF-TC-025 Launch Handoff Regression

```text
- Legacy Direct Context
- 기존 속도·방향
- Pool Acquire / Actor Activate
- Reset
```

### CF-TC-026 Modular Launcher

```text
- 1 / 2 / 4 / 6 Muzzle
- 순환
- 발사 실패 시 Index 유지
- Ripple 간격
- Salvo 수량
- Angled / Vertical Velocity
- Carrier Velocity Ratio
```

---

## 14. Editor 검증

```text
- DA_RocketBody
- RocketLauncherPitch
- Muzzle_1~4
- 정지 차량
- 이동 차량
- Muzzle별 발사 위치
- SingleCycle 순서
- Ripple
- Vertical Ejection
- Pool 10회 이상
```

---

## 15. LM-P0-04 적용 결과

```text
- ReleaseMode 기본값 Direct는 기존 ProjectileData.InitialSpeed 결과를 유지한다.
- AngledEjection은 실제 선택 Muzzle Transform 기준 LocalEjectionDirection을 월드 방향으로 변환한다.
- VerticalEjection은 실제 선택 Muzzle 소켓 X축을 초기 사출 방향으로 사용한다.
- InitialLaunchVelocity = InitialLaunchDirection * EjectionSpeed + VehicleVelocity * CarrierVelocityRatio를 적용했다.
- CommandTargetLocation은 InitialLaunchDirection과 독립적으로 Launch Context에 보존한다.
- 비Direct 안전 검사는 Command Target 방향이 아니라 실제 InitialLaunchDirection을 사용한다.
- 안전 검사와 실제 Projectile 활성화는 같은 Launch Context 인스턴스를 사용한다.
- ProjectileMovement의 기존 GravityScale·Sweep·Sub-step을 유지한다.
- 발사 직후 Projectile은 런처 Transform을 다시 조회하지 않는다.
- Ejection 전용 중력 배율 전환은 Missile Flight State가 생기기 전에는 추가하지 않는다.
- ReleaseContract Source Compile PASS / 실행 Not Run이다.
- 최종 Editor Build Job acd575ffac2f4d64bd73194e160a3f62 / Exit Code 0이다.
```

## 16. Changelog

### v0.6.0 - 2026-07-30

```text
- Direct Handoff, Rocket 추진·독립 비행, Pool 5 Volley 이상 사용자 PIE를 PASS했다.
- Direct Ripple 4발·0.15초, Muzzle 1→4→2→3, Command Target 고정, 중복 입력 방지와 SequenceCompleted를 PASS했다.
- SingleCycle 입력당 1발, Muzzle 순환과 발사 거부 시 인덱스 유지를 PASS했다.
- Damage는 보강 미완료로 별도 차단하고 런처 실패로 판정하지 않았다.
- 다음 LM-P0-06 검증을 Salvo로 이동했다.
```

### v0.5.0 - 2026-07-29

```text
- DA_RocketBody에 RocketLauncherYaw·Pitch와 Muzzle 순서 1→4→2→3을 적용했다.
- DA_RocketLauncher와 RocketLauncher EquipmentPresetData를 분리 생성하고 Ripple 4발·0.15초·SequenceCompleted·Direct 설정을 적용했다.
- DA_TestSUV, TestMap Launcher 차량과 BP_CFVehiclePawn CDO의 실제 플레이 진입 경로를 연결했다.
- LauncherPost 10/10, VehiclePost 3/3, MapPost 19/19와 CDO 새 프로세스 재검증을 기록했다.
- LM-P0-06 CF-TC-025·026 사용자 PIE를 Active로 이동했으며 PASS로 기록하지 않았다.
```

### v0.4.0 - 2026-07-29

```text
- LM-P0-04 Direct·Angled·Vertical Release Runtime과 차량 속도 상속을 적용했다.
- 실제 사출 방향 안전 검사와 같은 Launch Context 재사용을 반영했다.
- ReleaseContract 소스 컴파일과 최종 Editor Build PASS를 기록했다.
- LM-P0-05 Editor Assets를 현재 Active 단계로 이동했다.
```

### v0.3.0 - 2026-07-29

```text
- LM-P0-03A/B 구현·공식 빌드 완료 상태를 반영했다.
- UCFLauncherComp를 계획 컴포넌트가 아닌 현재 Runtime Scheduler로 갱신했다.
- LM-P0-04 Release Mode 코드 진입 계약과 기존 Projectile 중력 유지 기준을 확정했다.
```

## 17. Migration

```text
- 기존 WeaponData는 Direct / CarrierVelocityRatio 0 기본값으로 재저장 없이 같은 직사 결과를 유지한다.
- 새 Ejection 데이터는 EjectionSpeed와 CarrierVelocityRatio를 명시적으로 설정한다.
- 새 세션은 LM-P0-01~05 구현·자산 적용을 반복하지 않고 LM-P0-06 사용자 PIE에서 시작한다.
- 기본 런처 자산은 Direct / Ripple 4발 / 0.15초 / SequenceCompleted / CarrierVelocityRatio 0이다.
- Angled·Vertical·Carrier Velocity 검증은 임시 값을 사용한 뒤 기본 Direct 설정으로 복구한다.
- 실제 Muzzle·Ripple·Ejection·Pool·취소 결과는 CF-TC-025·026 사용자 PIE에서 확정한다.
```

## 18. 제외 범위

```text
- Missile Guidance
- Laser Designator
- Top-Attack
- 탄약 재장전 UI
- 발사관 파괴 모듈
- 네트워크 Volley 복제
- Audio
```

---

## 16. Changelog

### v0.1.0 - 2026-07-28

```text
- 가변 Muzzle, Legacy Fallback과 발사 성공 기반 순환 규칙을 정의했다.
- SingleCycle, Ripple, Salvo 발사 패턴을 정의했다.
- Direct, AngledEjection, VerticalEjection Release Data를 설계했다.
- 초기 Velocity와 차량 속도 상속 계산을 정의했다.
- 런처와 WeaponComp의 책임 경계, Volley Cooldown과 Debug 계약을 정리했다.
```

---

## 17. Migration

```text
- 기존 TurretMountData.MuzzleSocketName은 Legacy Fallback으로 유지한다.
- 신규 LauncherData가 없는 장비는 기존 Direct Single Muzzle 경로를 유지한다.
- 첫 다연장 단계는 SingleCycle만 활성화한다.
- 기존 WeaponData FireRate와 Cooldown 의미를 임의로 변경하지 않는다.
```
