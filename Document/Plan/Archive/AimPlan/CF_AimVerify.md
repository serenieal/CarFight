# CarFight - CF_AimVerify

> 역할: Aim 시스템의 싱글플레이 PIE / Standalone 검증 기준을 정의한다.
> 문서 버전: v0.4.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Draft / Single Player Verification + Local Fire Rename

---

## 1. 검증 목적

Aim 시스템은 카메라 조준, 로컬 발사 가능 여부, HitScan/Projectile 연결 준비, Reticle/Debug 피드백이 분리되어야 한다.

검증 축은 다음 4개로 나눈다.

```text
Local Aim
  -> 플레이어 입력과 카메라 기준 조준 방향 계산

Local Fire Validation
  -> 조준각 / 차단 / 무기 상태 검증

Local Hit Result
  -> HitScan 또는 Projectile 초기 결과 생성

Local Feedback
  -> Reticle / VehicleDebug / FX 표시
```

멀티플레이 검증은 현재 완료 조건이 아니다.

---

## 2. 검증 환경

### 2.1 기본 맵

기본 검증 맵은 현재 기준선인 다음 맵을 사용한다.

```text
/Game/Maps/TestMap
```

### 2.2 기준 Pawn

기준 Pawn은 현재 기준선인 다음 자산을 사용한다.

```text
/Game/CarFight/Vehicles/BP_CFVehiclePawn
```

### 2.3 기준 차량 데이터

기준 차량 데이터는 다음 자산을 사용한다.

```text
/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar
```

### 2.4 실행 모드

검증은 다음 실행 모드를 사용한다.

| 단계 | 실행 모드 |
|---|---|
| 기본 기능 검증 | PIE Single Player |
| 에디터 외 실행 확인 | Standalone Game |
| 후속 후보 | 패키지 실행 |

---

## 3. Local Aim 검증

### 3.1 카메라 Aim 연동

검증 항목:

- 오른쪽 스틱 또는 마우스 Look 입력으로 카메라가 움직인다.
- AimComp가 CameraRuntimeState를 읽는다.
- LocalAimDirection이 카메라 방향과 함께 변한다.
- LocalAimTargetLocation이 Aim Trace 결과를 따라간다.

PASS 기준:

- Look 입력 변화에 따라 Debug Aim 값이 즉시 변한다.
- AimTarget이 카메라가 보는 방향과 일치한다.

FAIL 기준:

- 카메라는 움직이지만 AimTarget이 갱신되지 않는다.
- AimTarget이 차량 정면에 고정된다.
- AimComp가 CameraComp를 찾지 못한다.

### 3.2 Weapon Arc 판정

검증 항목:

- AimProfile의 Yaw / Pitch 제한을 적용한다.
- 조준 방향이 허용각 안이면 `bLocalWithinWeaponArc = true`다.
- 조준 방향이 허용각 밖이면 `bLocalWithinWeaponArc = false`다.

PASS 기준:

- 허용각 안과 밖이 Debug에서 구분된다.
- OutOfArc 상태가 ReticleState에 반영된다.

### 3.3 Reticle State

검증 항목:

- Ready
- Blocked
- OutOfArc
- NoWeapon
- Cooldown 후보
- FireBlocked 후보
- FireConfirmed 후보

PASS 기준:

- Local 상태 변화가 ReticleState에 반영된다.
- Reticle UI가 없더라도 Debug에서 상태 문자열을 확인할 수 있다.

---

## 4. VehicleDebug 검증

### 4.1 Aim 카테고리 표시

검증 항목:

- VehicleDebug Panel에 Aim 카테고리가 추가되어 있다.
- Navigation에서 Aim 항목을 선택할 수 있다.
- Aim 섹션에 Local Aim / Fire Validation / Fire Visual 값이 구분되어 있다.

PASS 기준:

- Aim Debug Section이 비어 있지 않다.
- LocalAimTarget, ReticleState, CanFire 값이 표시된다.
- LastFireRejectReason 표시 자리가 있다.
- Local Hit 결과가 생기면 위치 또는 상태를 확인할 수 있다.

### 4.2 불필요한 네트워크 표시 확인

검증 항목:

- 현재 싱글플레이 완료 조건에 LocalRole / RemoteRole 구분을 넣지 않는다.
- AimVisualState는 로컬 UI/디버그/후속 이펙트 확인 대상으로만 본다.
- Dedicated Server 상태를 VehicleDebug Aim의 필수 표시로 보지 않는다.

PASS 기준:

- Aim Debug가 싱글플레이 조준/발사 검증에 필요한 값 중심으로 읽힌다.
- 네트워크 값이 남아 있더라도 Deprecated 또는 참고 정보로 구분된다.

---

## 5. Local Fire Command 검증

검증 환경:

```text
PIE Single Player
```

검증 항목:

- 플레이어가 Fire 입력을 누른다.
- `HandleFireStarted()` 경로가 실행된다.
- Fire Command 또는 레거시 Fire Request ID가 증가한다.
- Local Fire Validation 결과가 생성된다.
- Fire Result가 Reticle 또는 VehicleDebug에 반영된다.

PASS 기준:

- 입력 1회당 발사 명령 1회가 확인된다.
- 발사 가능 상태와 거부 상태가 Debug에서 구분된다.
- 크래시나 입력 충돌이 없다.

FAIL 기준:

- Fire 입력이 들어오지 않는다.
- Fire 입력이 Look / Move 입력을 막는다.
- FireResult가 항상 기본값에 머문다.

---

## 6. Local HitScan 더미 검증

검증 환경:

```text
PIE Single Player
Standalone Game
```

검증 항목:

- 조준 후 Fire 입력을 누른다.
- 로컬 Trace 결과 HitLocation을 기록한다.
- FireResult에 HitLocation / HitNormal 후보가 들어간다.
- Debug Line 또는 Debug Panel에서 로컬 Trace를 확인한다.

PASS 기준:

- 조준 방향과 Trace 방향이 크게 어긋나지 않는다.
- HitLocation이 항상 Zero에 고정되지 않는다.
- Damage는 아직 적용하지 않아도 정상이다.

FAIL 기준:

- FireResult의 HitLocation이 항상 Zero다.
- Trace 시작점이 차량 또는 카메라와 무관한 위치다.
- OutOfArc / Blocked 상태에서도 결과가 무조건 승인된다.

---

## 7. Reticle UI 검증

검증 항목:

- Ready 상태 표시
- Blocked 상태 표시
- OutOfArc 상태 표시
- FireBlocked 상태 표시
- FireConfirmed 상태 표시 후보

PASS 기준:

- 상태별 표현이 최소한 구분된다.
- Reticle UI가 직접 Trace를 수행하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.
- Reticle UI가 조준 / 발사 입력을 가로막지 않는다.

---

## 8. 회귀 검증

Aim 시스템 추가 후 기존 기능이 깨지면 안 된다.

검증 항목:

- 기본 주행 PASS
- SteeringPlan 조향 감각 유지
- WheelSync 위치 / 회전 유지
- DriveState Debug 유지
- Camera Yaw / Pitch 유지
- CameraDebug Panel 유지
- VehicleDebug Panel Navigation 유지

PASS 기준:

- AimComp 추가 후 기존 차량 주행이 깨지지 않는다.
- 기존 Camera Debug 값이 유지된다.
- VehicleDebug Panel이 크래시 없이 표시된다.

---

## 9. 1차 완료 기준

Aim 1차 완료 기준은 다음과 같다.

- PIE Single Player에서 Local Aim State가 정상 갱신된다.
- Standalone Game에서 크래시 없이 차량 이동 / 카메라 / Fire 입력이 동작한다.
- VehicleDebug에서 Aim 값을 확인할 수 있다.
- Fire 입력 시 Local Fire Command가 생성된다.
- 로컬 Fire Validation이 승인 / 거부 상태를 만든다.
- RejectReason이 Debug에 표시된다.
- 로컬 HitScan 더미 Trace가 동작한다.
- Reticle UI가 상태를 표시하고 입력을 막지 않는다.
- 기존 주행 / 카메라 / 디버그 기능에 회귀 문제가 없다.

---

## 10. 후속 검증 후보

후속 단계에서 추가할 검증 항목:

- 패키지 실행 검증
- Fire FX / 사운드 검증
- HitScan Damage 더미 검증
- Ammo / Cooldown / Reload 검증
- Projectile 로컬 Spawn 검증
- Lock-On 로컬 검증
- 부품 단위 피격 검증
- AI AimSource 검증

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 검증 기준을 Listen Server / Client / Dedicated Server에서 PIE Single Player / Standalone Game으로 전환했다.
  - Server Fire Request, Server HitScan, Replicated Aim Visual 검증을 현재 완료 조건에서 제거했다.
  - Local Fire Command, Local HitScan, Reticle UI, 회귀 검증 중심으로 재정리했다.
- v0.4.0 / 2026-06-19
  - `RepAimVisualState` 원격 표시 기준을 현재 `AimVisualState` 로컬 확인 기준으로 정정했다.
  - Pawn 단위 서버/RPC Fire 경로 제거 후 검증 의미를 현재 로컬 흐름에 맞췄다.

## 마이그레이션 지침

- 현재 검증 작업은 이 문서를 기준으로 한다.
- `CF_AimNetVerify.md`는 Deprecated 문서이므로 신규 완료 조건으로 사용하지 않는다.
- 새 검증 기록에는 `BuildFireCommand`, `ValidateFireCommand`, `RunLocalDummyHitScan`, `ApplyFireResult` 기준 이름을 사용한다.
- 남은 `FCFVehicleFireRequest` 이름은 로컬 Fire Command 데이터 의미로 기록한다.

## Change Note

- 2026-06-19: 과거 네트워크 표시 검증 문장을 현재 로컬 AimVisualState 기준으로 축소했다.
