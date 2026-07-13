# CarFight — 05_TestChecklist

> 문서 버전: v1.4.0
> 작성일(Asia/Seoul): 2026-07-13
> 문서 상태: Active
> 역할: CarFight의 **완료된 Systems 기준 최소 회귀 테스트**를 관리한다.

---

## 1. 목적

이 문서는 CarFight에서 이미 구현되어 `Document/Systems/`에 기록된 기능들의 최소 회귀 테스트를 관리한다.

이 문서는 진행 중 기능의 상세 테스트 계획이 아니다.
진행 중 기능의 테스트 계획은 `Document/Plan/<기능명>/TestPlan.md`에서 관리한다.

이 문서의 목적은 아래와 같다.

```text
- 완료된 기능이 이후 작업으로 깨졌는지 빠르게 확인한다.
- 기능별 테스트 기준 문서 위치를 한 곳에서 찾는다.
- 새로운 기능 완료 후 회귀 테스트 목록에 추가한다.
- AI가 수정 작업 전후로 확인할 최소 검증 범위를 알 수 있게 한다.
```

---

## 2. 문서 사용 기준

문서 역할은 아래처럼 분리한다.

| 문서 위치 | 역할 |
|---|---|
| `Document/Plan/<기능명>/TestPlan.md` | 진행 중 기능의 상세 테스트 계획 |
| `Document/ProjectSSOT/05_TestChecklist.md` | 현재 기준선/P0 검증 기록 |
| `Document/ProjectSSOT/05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 인덱스 |
| `Document/Systems/<분류>/<기능명>.md` | 완료 기능의 현재 구현 기준 |

---

## 3. 테스트 상태 표기

| 상태 | 의미 |
|---|---|
| `PASS` | 통과 |
| `FAIL` | 실패 |
| `PARTIAL` | 일부 통과 / 일부 미확인 |
| `N/A` | 현재 조건에서 해당 없음 |
| `TODO` | 아직 테스트 항목만 있고 수행 전 |

---

## 4. 공통 테스트 원칙

```text
1. 현재 구현 기준은 Document/Systems/를 우선한다.
2. 미래 계획은 테스트 PASS 기준으로 쓰지 않는다.
3. 실패 시 원인을 코어 문제 / 수치 문제 / 에디터 설정 문제 / 테스트 환경 문제로 분리한다.
4. 테스트 결과가 기능 책임을 바꾸면 해당 Systems 문서도 갱신한다.
5. 2026-06-18 싱글 전환 기준에서는 서버/멀티/2클라 테스트를 기본 회귀 조건으로 쓰지 않는다.
6. 서버/멀티 항목은 현재 사이클에서 N/A로 두고, 서버 작업 재개 결정이 있을 때만 다시 활성화한다.
7. 2026-06-19 전투 루프 기준에서는 조준/발사/피격/피해/피드백을 서버 없이 싱글 PIE에서 먼저 검증한다.
8. 2026-07-09 Reticle / FireFeedback 정리 기준에서는 서버 대기 / 서버 거부 표현을 현재 UI PASS 기준으로 쓰지 않는다.
9. OutOfArc / bLocalWithinWeaponArc는 현재 P0 싱글플레이 기준에서 단독 발사 차단 조건이 아니라 조준각 경고/디버그 상태로 본다.
10. 발사 성공 여부는 WeaponFire의 LastFireResult / ValidateFireCommand 결과를 기준으로 판단한다.
```

---

## 4-1. 2026-06-18 싱글 전환 테스트 기준

현재 사이클의 최소 검증 목표는 아래다.

```text
1. 서버 실행 없이 에디터와 PIE 1인 플레이만으로 기준 차량을 검증한다.
2. 기준 차량 1대가 바로 조작 가능해야 한다.
3. 주행감, 카메라, 로컬 Aim, Reticle, WheelSync 시각 품질을 우선 확인한다.
4. Dedicated Server, 2클라, 이동 복제, 서버 권한 발사 검증은 현재 기준 N/A다.
```

---

## 4-2. 2026-06-19 전투 루프 검증 기준

현재 사이클의 전투 루프 검증 목표는 아래다.

```text
1. 기준 차량 1대가 주행 중 조준과 발사를 수행한다.
2. 발사 성공/불가/쿨다운 상태가 Reticle, 간단한 UI, 이펙트, 사운드 후보로 읽힌다.
3. 발사 결과가 빗나감/피격 기록으로 남고, 후속 단계에서 피해 처리로 이어질 수 있어야 한다.
4. 주행, 조준, 발사, 피격, 피해 흐름을 반복해도 상태가 꼬이지 않는다.
5. 조작감, 전투 템포, 피드백 문제를 기능 차단과 품질 후속으로 분리한다.
```

---

## 4-3. 2026-07-09 WeaponFire / FireFeedback 테스트 기준

Reticle 관련 작업 전 최소 검증 기준은 아래다.

```text
1. WeaponFire는 판정/기록 담당으로 본다.
2. AimReticle / FireFeedback은 표시 담당으로 본다.
3. Reticle은 서버 상태를 표시하지 않는다.
4. FirePending은 서버 대기가 아니라 로컬 발사 처리 피드백 상태다.
5. FireRejected는 서버 거부가 아니라 로컬 발사 조건 미충족 상태다.
6. WeaponCooldown은 Cooldown 표시 후보로 변환될 수 있어야 한다.
7. NoWeapon은 무기 없음 표시 후보로 변환될 수 있어야 한다.
8. AimBlocked는 Blocked 표시 후보로 변환될 수 있어야 한다.
9. OutOfArcWarning은 발사 차단이 아니라 보조 경고/디버그 상태로 본다.
```

## 4-4. 2026-07-13 Reticle / FireFeedback 검증 결과

확인 완료:

```text
- Unreal Editor 타깃 빌드 성공
- WBP_AimReticle Reticle 이미지 5개와 FireFeedback TextBlock 바인딩 확인
- Ready 상태의 흰색 Reticle 확인
- FireSuccess 상태의 녹색 Reticle / 텍스트 확인
- FireSuccess 표시 후 Cooldown 파란색 상태 전환 확인
- 연속 Fire 입력 후 쿨다운 종료 시 FireFeedback 텍스트가 남지 않음
- 전용 Text_OutOfArcWarning이 실제 OutOfArcWarning 피드백에서만 일반 State/Hint를 대체함
- 조준각 밖에서 FireSuccess / Cooldown / FireRejected 피드백이 활성화되어도 일반 FireFeedback 텍스트가 가려지지 않음
```

아직 확인하지 않은 항목:

```text
- NoWeapon 실제 PIE 표시와 회색 상태
- AimBlocked 실제 PIE 표시와 주황 상태
- NoWeapon / AimBlocked 유지 시간 종료 후 텍스트 제거
```

상태 판정:

```text
- CF-TC-006 조준 Reticle: PASS
- CF-TC-013 차량 무기 조준/발사: PARTIAL
- CF-TC-014 발사 피드백/UI: PARTIAL
```

---

## 5. 전체 최소 회귀 테스트 세트

아래 테스트는 큰 구조 수정 후 최소 1회 확인한다.

| ID | 영역 | 테스트 | PASS 기준 | 관련 Systems 문서 | 상태 |
|---|---|---|---|---|---|
| `CF-TC-001` | Vehicle | 기본 차량 조작 | 전진/후진/조향/브레이크/핸드브레이크가 동작 | `Document/Systems/Vehicles/VehicleDrive.md` | `TODO` |
| `CF-TC-002` | Vehicle | 휠 시각 동기화 | 휠 위치/조향/스핀 시각 반응이 정상 | `Document/Systems/Vehicles/WheelSync.md` | `TODO` |
| `CF-TC-003` | Vehicle | 카메라 | 차량 기준 카메라가 정상 추적/회전 | `Document/Systems/Vehicles/VehicleCamera.md` | `TODO` |
| `CF-TC-004` | Input | 기본 입력 등록 | Enhanced Input Mapping Context 등록 성공 | `Document/Systems/Input/Input.md` | `TODO` |
| `CF-TC-005` | UI | 차량 디버그 표시 | 디버그 HUD/Panel이 필요한 조건에서 표시 | `Document/Systems/UI/VehicleDebug.md`, `Document/Systems/UI/VehicleDebugPanel.md` | `TODO` |
| `CF-TC-006` | UI | 조준 Reticle | 조준 Reticle 표시/갱신이 정상이며 로컬 발사 결과 피드백 후보와 충돌하지 않음 | `Document/Systems/UI/AimReticle.md`, `Document/Systems/Combat/FireFeedback.md` | `PASS` |
| `CF-TC-007` | Network | Dedicated Server 실행 | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-008` | Network | 1클라 Spawn/Possess | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-009` | Network | 2클라 Spawn/Possess | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-010` | Network | 입력 분리 | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md`, `Document/Systems/Input/Input.md` | `N/A` |
| `CF-TC-011` | Network | 이동 복제 | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-012` | Config | 런타임 설정 | 현재 Config 기준 경로/모드가 깨지지 않음 | `Document/Systems/Config/ProjectRuntimeConfig.md` | `TODO` |
| `CF-TC-013` | Combat | 차량 무기 조준/발사 | 조준 방향으로 로컬 발사 명령이 생성되고 성공/불가/쿨다운 상태가 구분됨 | `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/VehicleDebugPanel.md` | `PARTIAL` |
| `CF-TC-014` | Feedback | 발사 피드백/UI | 발사 성공/불가/쿨다운/무기 없음 상태가 Reticle 또는 FireFeedback UI 기준으로 읽힘 | `Document/Systems/Combat/FireFeedback.md`, `Document/Systems/UI/AimReticle.md`, `Document/Systems/Combat/WeaponFire.md` | `PARTIAL` |
| `CF-TC-015` | Combat | 피격 판정 기록 | 발사 결과가 빗나감/피격으로 구분되어 Debug Context로 기록됨 | `Document/Systems/Combat/DamageHitContext.md`, `Document/Systems/Combat/Projectile.md` | `TODO` |
| `CF-TC-016` | Combat | 피해 처리 | 피격 결과가 피해량과 생존 상태에 반영됨 | `Document/Systems/Combat/HitDamage.md` 예정 | `TODO` |
| `CF-TC-017` | Loop | 주행/전투 반복 | 주행, 조준, 발사, 피격, 피해 루프를 반복해도 상태가 꼬이지 않음 | `Document/Systems/Combat/CoreLoop.md` 예정 | `TODO` |
| `CF-TC-018` | Feel | 전투 템포/피드백 | 조작감, 발사 리듬, 피격 반응 문제가 기능 차단과 품질 후속으로 분리됨 | `Document/Systems/Combat/CombatFeel.md` 예정 | `TODO` |

---

## 6. Vehicles 테스트

## 6.1 VehicleDrive

- 관련 문서: `Document/Systems/Vehicles/VehicleDrive.md`

### 확인 항목

```text
- Throttle 입력이 차량 전진에 적용된다.
- Brake 입력이 감속/정지에 적용된다.
- Steering 입력이 좌우 조향에 적용된다.
- Handbrake 입력이 핸드브레이크 동작에 적용된다.
- DriveState가 Idle / Accelerating / Braking / Reversing / Coasting / Airborne / Disabled 범위에서 의미 있게 변한다.
- VehicleMovement를 찾지 못하는 경우 Disabled로 안전하게 떨어지는지 확인한다.
```

### PASS 기준

```text
- 입력 적용이 끊기지 않는다.
- DriveState가 명백히 틀린 상태로 고정되지 않는다.
- 입력/상태 디버그가 확인 가능하다.
```

---

## 6.2 WheelSync

- 관련 문서: `Document/Systems/Vehicles/WheelSync.md`

### 확인 항목

```text
- Wheel_Anchor_* 위치가 기준 차량 휠 중심과 맞는다.
- 조향 시 앞바퀴 피벗이 자연스럽다.
- 전진/후진 시 휠 스핀 방향이 정상으로 보인다.
- 심한 떨림 또는 이중 적용이 없다.
- 고속 휠 시각 품질 이슈가 기능 FAIL인지, 품질 후속인지 분리된다.
```

### PASS 기준

```text
- 기본 주행 중 휠 위치/조향/스핀 시각 동기화가 기능적으로 정상이다.
- 품질 이슈가 있어도 기능 판정과 분리해서 기록 가능하다.
```

---

## 6.3 VehicleCamera

- 관련 문서: `Document/Systems/Vehicles/VehicleCamera.md`

### 확인 항목

```text
- 로컬 플레이어 차량 기준 카메라가 정상 생성/활성화된다.
- Look 입력이 카메라에 반영된다.
- 싱글 PIE에서 카메라가 차량 기준으로 추적/회전한다.
- 서버/비소유 Pawn 검증은 현재 사이클에서 N/A다.
```

### PASS 기준

```text
- 로컬 클라이언트 카메라가 정상 동작한다.
- PIE 1인 플레이에서 카메라 이동, 회전, 충돌 감각이 확인 가능하다.
```

---

## 6.4 VehicleAim

- 관련 문서: `Document/Systems/Vehicles/VehicleAim.md`

### 확인 항목

```text
- 현재 AimComp가 의도한 기준으로 조준 방향/타겟 정보를 계산한다.
- UI Reticle과 연결되는 데이터가 유효하다.
- LocalAimState와 FireValidationState를 구분해 확인할 수 있다.
- bLocalWithinWeaponArc / OutOfArc는 표시/디버그 상태로 확인한다.
- 서버 권한 발사 구조는 현재 보류하고, 로컬 Aim / Reticle 피드백을 우선한다.
```

### PASS 기준

```text
- 조준 데이터가 UI 또는 후속 Fire 요청에서 읽을 수 있는 형태로 유지된다.
- 서버 없이 로컬 조준 상태와 Reticle 상태를 확인할 수 있다.
- OutOfArc를 단독 발사 차단으로 오해하지 않도록 Debug / UI 기준이 분리되어 있다.
```

---

## 7. Input 테스트

- 관련 문서: `Document/Systems/Input/Input.md`

### 확인 항목

```text
- DefaultInputMappingContext가 로컬 플레이어에 등록된다.
- Throttle / Brake / Steering / Handbrake 입력이 바인딩된다.
- VehicleMove 2D 입력과 Legacy Axis 입력이 충돌하지 않는다.
- InputDeviceMode가 Auto / KeyboardMouseOnly / GamepadOnly 조건에서 의도대로 필터링된다.
- Look 입력이 VehicleCameraComp로 전달된다.
```

### PASS 기준

```text
- 현재 테스트 장치에서 차량 조작 입력이 정상 동작한다.
- 장치 모드 제한 때문에 입력이 막힌 경우 구조 실패와 설정 문제를 분리해 기록한다.
```

---

## 8. UI 테스트

## 8.1 AimReticle

- 관련 문서:
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Combat/FireFeedback.md`

### 확인 항목

```text
- Reticle 위젯이 필요한 조건에서 생성된다.
- 조준 상태 변화가 Reticle에 반영된다.
- 싱글 PIE에서 로컬 차량 기준 Reticle 표시/갱신을 확인한다.
- Reticle은 서버 대기 / 서버 거부 상태를 표시하지 않는다.
- FirePending / FireRejected는 로컬 발사 피드백 상태로 해석된다.
- Cooldown / NoWeapon / FireRejected 표시 후보가 WeaponFire 결과와 충돌하지 않는다.
```

### PASS 기준

```text
- 로컬 클라이언트 화면에서 Reticle이 정상 표시된다.
- Aim 상태 변화가 Reticle 표시 상태로 읽힌다.
- 로컬 발사 결과 피드백이 Reticle 기준과 충돌하지 않는다.
```

---

## 8.2 VehicleDebug / VehicleDebugPanel

- 관련 문서:
  - `Document/Systems/UI/VehicleDebug.md`
  - `Document/Systems/UI/VehicleDebugPanel.md`

### 확인 항목

```text
- 디버그 UI 생성 조건이 명확하다.
- 표시 텍스트가 현재 Systems 기준 필드와 맞는다.
- 너무 긴 Runtime 문자열이 가독성을 해치지 않는지 확인한다.
- Aim / Weapon / Camera / Runtime 섹션이 필요한 정보를 제공한다.
- Weapon 섹션에서 WeaponData, 쿨다운, Projectile, DamageHitContext, FireOrigin 상태를 확인할 수 있다.
- 서버 전용 UI 차단 검증은 현재 사이클에서 N/A다.
```

### PASS 기준

```text
- 디버그 UI가 테스트 중 필요한 정보를 제공한다.
- 싱글 PIE에서 주행/카메라/Aim/Weapon/WheelSync 정보를 읽을 수 있다.
- Debug Panel은 판정 계산기가 아니라 Snapshot 표시 UI로 동작한다.
```

---

## 9. Combat / Feedback / Loop 테스트

현재 전투 루프 테스트는 싱글플레이 로컬 기준으로 검증한다.
`WeaponFire`, `FireFeedback`, `Projectile`, `DamageHitContext` 기준 문서는 현재 `Document/Systems/Combat/` 아래에 존재한다.
단, 실제 플레이 테스트 결과가 아직 기록되지 않은 항목은 `TODO` 상태를 유지한다.

## 9.1 WeaponFire

- 관련 문서:
  - `Document/Systems/Combat/WeaponFire.md`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/UI/VehicleDebugPanel.md`

### 확인 항목

```text
- Fire 입력이 로컬 기준에서 발사 명령으로 연결된다.
- 발사 원점과 발사 방향을 확인할 수 있다.
- 발사 성공 / 발사 불가 / 쿨다운 상태가 구분된다.
- LastFireResult와 RejectReason을 확인할 수 있다.
- Weapon Debug에서 WeaponData / Cooldown / FireOrigin 상태를 확인할 수 있다.
- OutOfArc / bLocalWithinWeaponArc는 단독 발사 차단 조건으로 취급하지 않는다.
- 서버 권한 발사와 복제 검증은 현재 사이클에서 N/A다.
```

### PASS 기준

```text
- 싱글 PIE에서 기준 차량이 조준 방향으로 로컬 발사 명령을 만든다.
- 실패 시 원인을 입력 / 조준 막힘 / 쿨다운 / 데이터 문제로 분리할 수 있다.
- 발사 결과가 AimComp / VehicleDebug 상태에 기록된다.
```

---

## 9.2 FireFeedback

- 관련 문서:
  - `Document/Systems/Combat/FireFeedback.md`
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Combat/WeaponFire.md`

### 확인 항목

```text
- 발사 성공 시 최소 피드백 후보가 발생하거나 표시 기준이 확인된다.
- 발사 불가 또는 쿨다운 상태가 조준 UI / Reticle / Debug로 구분된다.
- NoWeapon / WeaponCooldown / AimBlocked / FireRejected 표시 후보가 구분된다.
- 피드백 호출과 실제 판정 흐름이 분리되어 있다.
- UI 표시 문구는 한국어 표시 정책을 따른다.
- 서버 대기 / 서버 거부 표현을 현재 UI 표시 기준으로 쓰지 않는다.
```

### PASS 기준

```text
- 플레이어가 발사 성공, 발사 불가, 쿨다운, 무기 없음 상태를 즉시 이해할 수 있다.
- 이펙트/사운드가 없어도 판정 흐름을 추적할 수 있다.
- 피드백은 WeaponFire 판정 결과를 임의로 바꾸지 않는다.
```

---

## 9.3 HitDamage / DamageHitContext

- 관련 문서:
  - `Document/Systems/Combat/DamageHitContext.md`
  - `Document/Systems/Combat/Projectile.md`
  - `Document/Systems/Combat/HitDamage.md` 예정

### 확인 항목

```text
- 발사 결과가 빗나감 / 피격으로 구분된다.
- Dummy HitScan 또는 Projectile Actor 충돌 결과가 DamageHitContext로 기록된다.
- 피격 결과가 후속 피해량으로 변환될 준비가 되어 있다.
- 피해 처리 실패 시 Aim / Fire / Hit / Damage 중 어느 단계 문제인지 분리된다.
```

### PASS 기준

```text
- 로컬 테스트에서 피격/빗나감 기록을 확인할 수 있다.
- 피해 적용이 미구현이어도 HitContext 기록 경로는 추적 가능하다.
```

---

## 9.4 CoreLoop

- 관련 문서: `Document/Systems/Combat/CoreLoop.md` 예정

### 확인 항목

```text
- 주행 중 조준/발사 입력이 끊기지 않는다.
- 조준/발사/피격/피해 루프를 5회 이상 반복한다.
- 반복 중 UI, 디버그, 피해 상태가 꼬이지 않는다.
- 남은 문제를 기능 차단 / 품질 후속 / 장기 확장으로 분리한다.
```

### PASS 기준

```text
- 기준 차량 1대로 주행, 조준, 발사, 피격, 피해 흐름을 반복할 수 있다.
- 다음 개발 단계로 넘어갈 수 있는지 PASS / FAIL 판정이 가능하다.
```

---

## 10. Network 테스트

현재 싱글 전환 기준에서 Network 테스트는 기본 회귀 테스트가 아니다.
아래 항목은 서버/멀티 작업을 재개할 때 다시 활성화한다.

## 10.1 ServerSpawn

- 관련 문서: `Document/Systems/Network/ServerSpawn.md`

### 확인 항목

```text
- CarFight_ReServer Target 빌드/실행이 가능하다.
- 서버가 TestMap 또는 서버 테스트맵을 로드한다.
- Client 1이 접속한다.
- 서버에서 PlayerController 1개가 확인된다.
- ACFMPGameMode::PostLogin() 흐름이 호출된다.
- 차량 Pawn이 Spawn된다.
- PlayerController가 Spawn된 차량을 Possess한다.
- Client 1 화면이 검정 화면에 고정되지 않는다.
```

### PASS 기준

```text
- 1클라 접속 후 자기 차량을 조작할 수 있다.
- 서버 로그에 Spawn/Possess 성공 로그가 확인된다.
```

---

## 10.2 2클라 소유권 / 입력 분리

- 관련 문서:
  - `Document/Systems/Network/ServerSpawn.md`
  - `Document/Systems/Input/Input.md`

### 확인 항목

```text
- Client 1과 Client 2가 모두 접속한다.
- 차량 Pawn이 2대 생성된다.
- Client 1과 Client 2가 서로 다른 차량을 소유한다.
- Client 1 입력은 Client 1 차량에만 적용된다.
- Client 2 입력은 Client 2 차량에만 적용된다.
- 비소유 차량이 로컬 입력에 반응하지 않는다.
```

### PASS 기준

```text
- 각 클라이언트가 자기 차량만 조작한다.
- 입력 교차 오염이 없다.
```

---

## 10.3 이동 복제

- 관련 문서: `Document/Systems/Network/ServerSpawn.md`

### 확인 항목

```text
- Client 1 차량 이동이 Client 2 화면에서 보인다.
- Client 2 차량 이동이 Client 1 화면에서 보인다.
- 위치 복제가 확인된다.
- 회전 복제가 확인된다.
- 정지 상태도 동기화된다.
- 급가속/급조향 시 품질 문제를 기능 FAIL과 분리한다.
```

### PASS 기준

```text
- 상대 차량 위치/회전이 갱신된다.
- 상대 차량이 전혀 안 보이거나 완전히 다른 위치로 분리되지 않는다.
```

---

## 11. Config 테스트

- 관련 문서: `Document/Systems/Config/ProjectRuntimeConfig.md`

### 확인 항목

```text
- DefaultEngine.ini의 주요 GameMode/ServerGameMode 연결이 현재 싱글 전환 기준과 맞는다.
- 기본 실행 경로에서 CFMPGameMode가 의도치 않게 사용되지 않는지 확인한다.
- 입력/맵/런타임 설정 경로가 현재 Systems 문서와 충돌하지 않는다.
- 오래된 ProjectSSOT/Plan 경로가 남아 있으면 실제 구조에 맞게 수정 후보로 기록한다.
```

### PASS 기준

```text
- 현재 실행 경로와 문서 경로가 서로 충돌하지 않는다.
- 런타임 설정 때문에 기본 테스트가 막히지 않는다.
```

---

## 12. 테스트 기록 양식

각 테스트 후 아래 형식으로 기록한다.

```text
테스트 일시:
테스트 환경:
엔진 버전:
실행 방식:
대상 맵:
대상 기능:
관련 Systems 문서:
결과: PASS / FAIL / PARTIAL / N/A
관찰 내용:
로그 요약:
수정 필요 항목:
다음 액션:
```

---

## 13. 실패 분류 기준

실패 시 아래 중 하나로 분류한다.

| 분류 | 의미 |
|---|---|
| `Core` | 코드 구조 또는 핵심 로직 문제 |
| `Data` | DataAsset / Config / 값 문제 |
| `Asset` | BP / 맵 / 메시 / 위젯 자산 문제 |
| `Server` | 권한 / 복제 / Dedicated Server 문제 |
| `Input` | 입력 매핑 / 장치 필터링 문제 |
| `UI` | 표시 / 위젯 생성 / 로컬 전용 처리 문제 |
| `Quality` | 기능은 되지만 품질 개선 필요 |
| `DocMismatch` | 문서와 실제 구현이 불일치 |

---

## 14. 문서 갱신 조건

아래 상황이 발생하면 이 문서를 갱신한다.

```text
- Document/Systems/에 새 완료 기능 문서가 추가됨
- 기존 Systems 문서의 기능 책임이 바뀜
- 회귀 테스트 항목이 늘거나 줄어듦
- 테스트 기준이 PASS/FAIL 판정에 영향을 줄 만큼 바뀜
- 반복 테스트 자동화 또는 관리툴 후보가 생김
```

---

## 15. 문서 버전 관리

- 현재 문서 버전: `v1.4.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 테스트 문서 운영 방식 자체 변경 |
| Minor | 새 시스템 테스트 섹션 추가 또는 PASS 기준 확장 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 16. 체인지로그

### v1.4.0 - 2026-07-13

```text
- CF-TC-006 조준 Reticle을 PASS로 갱신
- CF-TC-013 차량 무기 조준/발사와 CF-TC-014 발사 피드백/UI를 PARTIAL로 갱신
- Ready / FireSuccess / Cooldown 색상 전환과 연속 입력 후 텍스트 종료 확인 결과 추가
- 전용 OutOfArc 경고의 중복 방지, 종료 조건, 다른 FireFeedback 비가림 확인 결과 추가
- NoWeapon / AimBlocked 실제 PIE 검증을 남은 항목으로 기록
```

### v1.3.0 - 2026-07-09

```text
- WeaponFire / FireFeedback / AimReticle / VehicleAim 문서 교통정리 결과를 테스트 기준에 반영
- FireFeedback.md 신규 Systems 문서를 예정 표현에서 현재 참조 문서로 전환
- WeaponFire.md 예정 표현을 제거하고 현재 Systems 문서 기준으로 정리
- CF-TC-013 / CF-TC-014 / CF-TC-015 관련 문서 참조를 최신화
- OutOfArc / bLocalWithinWeaponArc를 단독 발사 차단 조건이 아닌 조준각 경고/디버그 상태로 명시
- FirePending / FireRejected를 서버 상태가 아닌 로컬 발사 피드백 상태로 정리
- VehicleDebugPanel의 Aim / Weapon 섹션을 전투 검증 기준에 반영
```

### v1.2.0 - 2026-06-19

```text
- 2026-06-19 전투 루프 검증 기준 추가
- CF-TC-013 ~ CF-TC-018 전투/피드백/루프 테스트 후보 추가
- WeaponFire, FireFeedback, HitDamage, CoreLoop 테스트 섹션 추가
- Network 이후 섹션 번호를 전투 루프 테스트 추가에 맞춰 조정
```

### v1.1.0 - 2026-06-18

```text
- 싱글 전환 테스트 기준을 추가
- Dedicated Server / 1클라 / 2클라 / 입력 분리 / 이동 복제 항목을 현재 사이클 N/A로 변경
- VehicleCamera / VehicleAim / AimReticle / VehicleDebug 검증 기준을 싱글 PIE 중심으로 재정렬
- Config 테스트에 CFMPGameMode 기본 실행 경로 제거 확인을 추가
```

### v1.0.0 - 2026-06-02

```text
- TestChecklist 문서 최초 작성
- Systems 기준 최소 회귀 테스트 인덱스 작성
- Vehicles / Input / UI / Network / Config 테스트 섹션 추가
- 테스트 기록 양식과 실패 분류 기준 추가
```
