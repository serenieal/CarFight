# CarFight — CF_AimNetVerify

> 역할: Aim 시스템 1차 구현 이후 Listen Server / Client 환경에서 검증해야 할 항목을 정의한다.  
> 문서 버전: v1.0.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Ready / Manual Network Verification

---

## 1. 문서 목적

이 문서는 Aim 시스템의 멀티플레이 검증 절차를 정의한다.

현재 Aim 시스템은 다음 단계까지 완료되어 있다.

```text
Local Aim
Reticle UI
Fire Request
Server Validation
Server Dummy HitScan
ServerAimState
RepAimVisualState
VehicleDebug Aim
```

하지만 아직 Listen Server / Client 환경에서 실제 동작을 확인하지 않았다.

따라서 이 문서는 새 기능 구현 문서가 아니라, 현재 구현된 Aim 시스템이 멀티플레이 환경에서 안전하게 동작하는지 확인하는 검증 문서다.

---

## 2. 검증 대상

검증 대상은 다음이다.

```text
1. Listen Server + 1 Client
2. Listen Server + 2 Clients
3. Owner Client Reticle 표시
4. Client Fire Request 서버 전달
5. Server Validation 결과
6. Server Dummy HitScan 결과
7. Owner Client FireResult 수신
8. VehicleDebug Aim 상태 갱신
9. RepAimVisualState 갱신
10. 기존 차량 이동 / 카메라 입력 회귀 여부
```

Dedicated Server는 이번 문서의 필수 검증 대상이 아니다. 후속 단계에서 별도 검증한다.

---

## 3. 검증 전 준비

### 3.1 필수 에셋 상태

아래 에셋이 연결되어 있어야 한다.

```text
/Game/CarFight/UI/WBP_AimReticle
/Game/CarFight/Input/IA_Fire
/Game/CarFight/Input/IMC_Vehicle_Default
/Game/CarFight/Vehicles/BP_CFVehiclePawn
```

`BP_CFVehiclePawn`에서 확인할 값:

```text
InputAction_Fire = IA_Fire
AimReticleWidgetClass = WBP_AimReticle
bShowAimReticle = true
```

### 3.2 에디터 시작 전 확인

- [ ] 모든 에셋 Save 완료
- [ ] PIE 실행 중 아님
- [ ] Output Log 열기
- [ ] VehicleDebug UI 확인 가능 상태
- [ ] `TestMap` 또는 현재 기준 테스트 맵 열기

---

## 4. PIE 설정

UE Editor에서 다음 설정을 사용한다.

```text
Play Settings
  Number of Players: 2
  Net Mode: Play As Listen Server
```

1차 검증은 2 Players로 진행한다.

2차 검증은 3 Players로 진행한다.

```text
1차:
  Listen Server + 1 Client

2차:
  Listen Server + 2 Clients
```

---

## 5. Listen Server + 1 Client 검증

### 5.1 시작 검증

절차:

1. Number of Players를 2로 설정한다.
2. Net Mode를 `Play As Listen Server`로 설정한다.
3. PIE를 시작한다.
4. Server 창과 Client 창이 모두 뜨는지 확인한다.

PASS 기준:

- [ ] Server 창 크래시 없음
- [ ] Client 창 크래시 없음
- [ ] 두 창 모두 차량 Pawn 생성 확인
- [ ] 기존 차량 이동 가능
- [ ] 기존 카메라 Look 가능

FAIL 기록:

```text
문제 발생 창:
재현 절차:
로그 메시지:
추정 원인:
```

---

### 5.2 Reticle Owner 표시 검증

절차:

1. Server 창에서 Reticle 표시 여부를 확인한다.
2. Client 창에서 Reticle 표시 여부를 확인한다.
3. 각 창에서 조준 방향을 움직여 Reticle 텍스트가 갱신되는지 확인한다.

PASS 기준:

- [ ] 로컬 제어 Pawn에서 Reticle 표시
- [ ] 비로컬 Pawn이 별도 Reticle을 생성하지 않음
- [ ] Reticle State 텍스트 표시
- [ ] CanFire 텍스트 표시
- [ ] 카메라 Look에 따라 상태 갱신 가능

주의:

Reticle Widget은 로컬 제어 Pawn에서만 생성되어야 한다.

---

### 5.3 Fire Request 서버 전달 검증

절차:

1. Client 창을 활성화한다.
2. 마우스 왼쪽 버튼으로 Fire 입력을 누른다.
3. VehicleDebug Aim에서 Fire 관련 값이 바뀌는지 확인한다.
4. Output Log에 오류가 없는지 확인한다.

PASS 기준:

- [ ] Client Fire 입력 시 크래시 없음
- [ ] `HandleFireStarted()` 경로가 동작한 것으로 보임
- [ ] Server Validation 결과가 갱신됨
- [ ] Last Accepted Fire Request Id 또는 Last Rejected Fire Request Id 갱신
- [ ] Last Server Reject Reason 갱신 또는 None 확인
- [ ] ServerAimTargetLocation 갱신 가능

FAIL 기록:

```text
Fire 입력 창:
Reticle 상태:
VehicleDebug Aim 값:
Output Log 오류:
```

---

### 5.4 Server Dummy HitScan 검증

절차:

1. Client 창에서 조준 후 Fire 입력을 누른다.
2. ServerAimTargetLocation 또는 ServerHitLocation에 해당하는 값이 바뀌는지 확인한다.
3. Debug Draw가 켜져 있다면 서버 Trace 라인이 보이는지 확인한다.

PASS 기준:

- [ ] 검증 승인 시 서버 Trace 결과가 반영됨
- [ ] 거부된 요청에서는 Trace가 실행되지 않는 것으로 보임
- [ ] ServerAimState가 Zero에 고정되지 않음
- [ ] Fire 입력 반복 시 Request Id 증가

주의:

현재 단계에서는 Damage나 Fire FX가 없어도 정상이다.

---

## 6. Listen Server + 2 Clients 검증

### 6.1 시작 검증

설정:

```text
Number of Players: 3
Net Mode: Play As Listen Server
```

PASS 기준:

- [ ] Server 창 생성
- [ ] Client 1 창 생성
- [ ] Client 2 창 생성
- [ ] 세 창 모두 크래시 없음
- [ ] 각 로컬 창에서 차량 이동 가능
- [ ] 각 로컬 창에서 카메라 Look 가능

---

### 6.2 RepAimVisual 검증

목적:

다른 클라이언트가 볼 수 있는 최소 조준 시각 상태가 서버에서 갱신되는지 확인한다.

절차:

1. Client 1에서 Fire 입력을 누른다.
2. VehicleDebug Aim에서 RepAimVisualState 값을 확인한다.
3. 가능하면 Server 창 또는 Client 2 창에서 Client 1의 RepAimVisual 값이 갱신되는지 확인한다.

PASS 기준:

- [ ] RepAimDirection 값 갱신
- [ ] RepAimTargetLocation 값 갱신
- [ ] bIsFiringVisual 값이 승인 결과에 따라 갱신
- [ ] RepWeaponVisualMode 값 확인 가능
- [ ] 다른 클라이언트에서 크래시 없음

주의:

현재는 실제 무기 방향 Mesh나 FX가 없으므로, 눈에 보이는 발사 연출이 없어도 정상이다.

---

## 7. 회귀 검증

Aim 네트워크 검증 후 기존 기능이 깨지면 안 된다.

확인 항목:

- [ ] 차량 이동 유지
- [ ] 조향 유지
- [ ] 카메라 Look 유지
- [ ] SpringArm / Camera 충돌 유지
- [ ] VehicleDebug Panel 표시 유지
- [ ] Camera Debug 표시 유지
- [ ] Aim Debug 표시 유지
- [ ] Reticle이 입력을 가로막지 않음
- [ ] Fire 입력이 기존 Look / Move 입력과 충돌하지 않음

---

## 8. 실패 유형별 후속 조치 기준

### 8.1 Reticle이 안 보임

후속 확인:

```text
BP_CFVehiclePawn.AimReticleWidgetClass
bShowAimReticle
IsLocallyControlled 조건
WBP_AimReticle Parent Class
Widget Visibility
```

### 8.2 Fire 입력이 안 들어감

후속 확인:

```text
IA_Fire
IMC_Vehicle_Default mapping
BP_CFVehiclePawn.InputAction_Fire
SetupPlayerInputComponent 바인딩
창 포커스
```

### 8.3 Server Request가 안 감

후속 확인:

```text
HasAuthority / IsLocallyControlled 분기
ServerRequestFire RPC 선언
Owner / Controller 유효성
Pawn possession 상태
```

### 8.4 Server Validation이 계속 거부됨

후속 확인:

```text
AimDirection 유효성
AimOrigin 거리 검증
DefaultAimProfile Yaw / Pitch 범위
bLocalAimBlocked
Camera AimHitLocation
```

### 8.5 RepAimVisual이 갱신되지 않음

후속 확인:

```text
UCFVehicleAimComp replication 설정
RepAimVisualState ReplicatedUsing
UpdateRepAimVisualFromFireResult 호출 여부
Component replication 여부
Pawn replication 여부
```

---

## 9. 검증 결과 기록 양식

검증 후 아래 양식으로 결과를 기록한다.

```text
검증 일시:
검증 맵:
검증 모드:
Number of Players:
Net Mode:

Server 창 결과:
Client 1 결과:
Client 2 결과:

Reticle 표시:
Fire 입력:
Server Validation:
Server Dummy HitScan:
RepAimVisual:
VehicleDebug Aim:
기존 이동/카메라 회귀:

발견 문제:
재현 절차:
후속 조치:
```

---

## 10. 완료 기준

Aim 네트워크 검증 1차 완료 기준은 다음이다.

- [ ] Listen Server + 1 Client에서 크래시 없음
- [ ] Client Fire 입력이 서버 검증으로 전달됨
- [ ] ServerAimState가 갱신됨
- [ ] Server Dummy HitScan 결과가 확인됨
- [ ] Owner Client가 FireResult를 수신함
- [ ] VehicleDebug Aim에서 결과 확인 가능
- [ ] Listen Server + 2 Clients에서 크래시 없음
- [ ] RepAimVisualState 갱신 확인
- [ ] 기존 차량 이동 / 카메라 입력 회귀 없음

---

## 11. 다음 단계

이 검증이 통과하면 다음 단계는 기능 추가로 넘어갈 수 있다.

권장 순서:

```text
1. Fire FX Multicast
2. HitScan Damage 더미
3. WeaponComp 설계 / 분리
4. Ammo / Cooldown / Reload
5. Projectile
```

검증 중 문제가 발견되면, 기능 추가 전에 버그 수정 Codex 작업지시서를 먼저 작성한다.
