# CarFight — CF_AimVerify

> 역할: Aim 시스템의 PIE / Listen Server / Client 검증 기준을 정의한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Planning

---

## 1. 검증 목적

Aim 시스템은 로컬 조준감, 서버 검증, 복제 시각화가 분리되어야 한다.

따라서 검증도 다음 3개 축으로 나눈다.

```text
Local Aim
  -> 소유 클라이언트 조준감 / Reticle 예측

Server Aim
  -> 서버 발사 검증 / HitScan Trace / Fire Result

Replicated Aim Visual
  -> 다른 클라이언트에서 보이는 조준 방향 / 발사 FX
```

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

검증은 단계별로 다음 실행 모드를 사용한다.

| 단계 | 실행 모드 |
|---|---|
| Local Aim 기본 검증 | PIE Single Player |
| Server Request 검증 | PIE Listen Server + 1 Client |
| 복제 시각 검증 | PIE Listen Server + 2 Clients |
| 후속 전용 서버 검증 | Dedicated Server 후보 |

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
- WaitingServer 후보
- ServerRejected 후보

PASS 기준:

- Local 상태 변화가 ReticleState에 반영된다.
- Reticle UI가 없더라도 Debug에서 상태 문자열을 확인할 수 있다.

---

## 4. VehicleDebug 검증

### 4.1 Aim 카테고리 표시

검증 항목:

- VehicleDebug Panel에 Aim 카테고리가 추가되어 있다.
- Navigation에서 Aim 항목을 선택할 수 있다.
- Aim 섹션에 Local / Server / Replicated 값이 구분되어 있다.

PASS 기준:

- Aim Debug Section이 비어 있지 않다.
- LocalAimTarget, ReticleState, CanFire 값이 표시된다.
- LastFireRejectReason 표시 자리가 있다.

### 4.2 Network 상태 표시

검증 항목:

- LocalRole
- RemoteRole
- bHasAuthority
- bIsLocallyControlled
- LastFireRequestId
- LastRejectReason

PASS 기준:

- Listen Server와 Client에서 역할 값이 서로 다르게 보인다.
- 서버 권한 여부를 Debug에서 확인할 수 있다.

---

## 5. Server Fire Request 검증

### 5.1 RPC 호출

검증 환경:

```text
PIE Listen Server + 1 Client
```

검증 항목:

- Client에서 Fire 입력을 누른다.
- `ServerRequestFire`가 서버에서 호출된다.
- FireRequestId가 증가한다.
- Server가 FireResult를 생성한다.
- Owner Client가 FireResult를 받는다.

PASS 기준:

- Client 입력 1회당 서버 요청 1회가 확인된다.
- FireResult가 Owner Client로 돌아온다.
- Debug에 LastFireRequestId가 표시된다.

FAIL 기준:

- Client에서만 처리되고 서버가 모른다.
- 서버 요청은 오지만 Owner 검증에 실패한다.
- FireResult가 Client로 돌아오지 않는다.

### 5.2 서버 거부 검증

검증 항목:

- OutOfArc 상태에서 Fire 입력
- InvalidAimDirection 요청
- InvalidOwner 상황 후보
- Cooldown 후보
- NoWeapon 후보

PASS 기준:

- 서버가 거부한다.
- RejectReason이 정확히 표시된다.
- Owner Client Reticle 또는 Debug가 `ServerRejected`를 표시할 수 있다.

---

## 6. 서버 HitScan 더미 검증

검증 환경:

```text
PIE Listen Server + 1 Client
```

검증 항목:

- 서버에서 Trace를 실행한다.
- Trace 결과 HitLocation을 기록한다.
- FireResult에 ServerHitLocation을 넣는다.
- Debug Line 또는 Debug Panel에서 서버 Trace를 확인한다.

PASS 기준:

- 클라이언트 예측 위치와 서버 위치가 크게 어긋나지 않는다.
- 서버 Trace 기준 결과가 FireResult에 표시된다.
- Damage는 아직 적용하지 않는다.

FAIL 기준:

- Client Trace만 있고 서버 Trace가 없다.
- FireResult의 HitLocation이 항상 Zero다.
- 서버에서 Trace 시작점이 비정상이다.

---

## 7. Replicated Aim Visual 검증

검증 환경:

```text
PIE Listen Server + 2 Clients
```

검증 항목:

- Client A가 조준 방향을 바꾼다.
- Client B에서 Client A의 조준 방향 시각 정보가 갱신된다.
- Fire 시 모든 클라이언트에서 Fire FX가 보인다.

PASS 기준:

- 다른 클라이언트에서 무기 방향 또는 조준 방향이 보인다.
- Fire FX가 모든 클라이언트에 재생된다.
- 전투 판정용 상태와 시각 복제 상태가 분리되어 있다.

FAIL 기준:

- Owner Client에서만 FX가 보인다.
- 다른 클라이언트에서 조준 방향이 고정된다.
- RepAimVisualState가 전투 판정에 사용된다.

---

## 8. Reticle UI 검증

검증 항목:

- Ready 상태 표시
- Blocked 상태 표시
- OutOfArc 상태 표시
- WaitingServer 상태 표시
- ServerRejected 상태 표시

PASS 기준:

- 상태별 표현이 최소한 구분된다.
- Reticle UI가 직접 Trace를 수행하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.

---

## 9. 회귀 검증

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

## 10. 1차 완료 기준

Aim 1차 완료 기준은 다음과 같다.

- Single Player에서 Local Aim State가 정상 갱신된다.
- VehicleDebug에서 Aim 값을 확인할 수 있다.
- Listen Server + 1 Client에서 Fire Request가 서버로 전달된다.
- 서버가 Fire Request를 승인 / 거부할 수 있다.
- RejectReason이 Owner Client Debug에 표시된다.
- 서버 HitScan 더미 Trace가 동작한다.
- Listen Server + 2 Clients에서 Fire FX 또는 RepAimVisual이 보인다.
- 기존 주행 / 카메라 / 디버그 기능에 회귀 문제가 없다.

---

## 11. 후속 검증 후보

후속 단계에서 추가할 검증 항목:

- Dedicated Server 검증
- 지연 / 패킷 손실 환경에서 Reticle 보정
- 연사 무기 Reliable / Unreliable 정책 검증
- Projectile 서버 Spawn 검증
- Damage System 연동 검증
- Lock-On 서버 검증
- 부품 단위 피격 검증
- AI AimSource 검증
