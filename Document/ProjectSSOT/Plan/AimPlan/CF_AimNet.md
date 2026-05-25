# CarFight — CF_AimNet

> 역할: Aim 시스템의 멀티플레이 권한, RPC, 복제, 예측, 검증 기준을 고정한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Planning

---

## 1. 문서 목적

이 문서는 조준 시스템의 네트워크 설계 기준을 고정한다.

멀티플레이 기준 Aim은 단일 상태가 아니다.

```text
Local Aim
  -> 소유 클라이언트의 즉각적인 조준감 / Reticle 예측

Server Aim
  -> 서버의 발사 가능 여부 검증 / 전투 결과 확정

Replicated Aim Visual
  -> 다른 클라이언트에게 보여줄 무기 방향 / 발사 연출
```

---

## 2. 핵심 원칙

### 2.1 클라이언트는 조준감을 책임진다

로컬 플레이어의 조준 입력은 즉시 반응해야 한다.

로컬 클라이언트에서 즉시 처리할 항목은 다음과 같다.

- 카메라 회전
- 카메라 Aim Trace
- Local Aim Target
- Local Reticle State
- Local CanFire 예측
- 로컬 Muzzle Flash / Camera Shake 같은 즉시 피드백

### 2.2 서버는 전투 결과를 책임진다

서버가 최종 결정할 항목은 다음과 같다.

- 발사 가능 여부 최종 판정
- HitScan Trace 최종 결과
- Projectile Spawn
- Damage 적용
- Part Damage / Destruction 적용
- Kill / Score / 상태 이상 적용

클라이언트는 발사 결과를 선언하지 않는다. 클라이언트는 서버에 발사를 요청한다.

```text
Client:
  나는 이 시각에 이 방향으로 발사했다.

Server:
  그 요청이 유효한지 검증하고 결과를 확정한다.
```

### 2.3 조준 상태와 발사 요청을 분리한다

Aim State는 매 프레임 갱신되는 상태다.

Fire Request는 발사 입력 순간에만 발생하는 이벤트다.

두 개념을 하나의 RPC로 묶지 않는다.

---

## 3. 권한 모델

| 항목 | 소유 클라이언트 | 서버 | 다른 클라이언트 |
|---|---:|---:|---:|
| 카메라 회전 | 계산 | 검증용 참조 가능 | 불필요 |
| Local Reticle | 계산/표시 | 불필요 | 불필요 |
| 발사 요청 | 생성 | 수신/검증 | 불필요 |
| 발사 승인 | 수신 | 생성 | 필요 시 FX 수신 |
| HitScan 결과 | 예측 가능 | 최종 확정 | 결과 표시 |
| Damage | 표시 | 최종 적용 | 표시 |
| 무기 방향 시각화 | 표시 | 기준 상태 관리 | 표시 |

---

## 4. 네트워크 상태 분리

### 4.1 Local Aim State

소유 클라이언트가 즉시 계산하는 상태다.

필드 후보:

- `LocalAimTargetLocation`
- `LocalAimDirection`
- `LocalReticleState`
- `bLocalCanFire`
- `bLocalWithinWeaponArc`
- `bLocalAimBlocked`

복제하지 않는다. Owner UI와 Owner Debug에 사용한다.

### 4.2 Server Aim State

서버가 검증한 상태다.

필드 후보:

- `ServerAimTargetLocation`
- `bServerWithinWeaponArc`
- `bServerCanFire`
- `LastServerRejectReason`
- `LastAcceptedFireRequestId`
- `LastRejectedFireRequestId`

Owner Client에 결과를 알려줄 수 있으나, 전체 복제 대상은 아니다.

### 4.3 Replicated Aim Visual State

다른 클라이언트에게 보여주기 위한 최소 시각 상태다.

필드 후보:

- `RepAimDirection`
- `RepAimTargetLocation`
- `bIsFiringVisual`
- `RepWeaponVisualMode`

전투 판정용이 아니다. 시각 표현용이다.

---

## 5. Fire Request

클라이언트가 서버에 보내는 발사 요청이다.

필드 후보:

- `FireRequestId`
- `ClientFireTimeSeconds`
- `AimOrigin`
- `AimDirection`
- `PredictedAimTargetLocation`
- `WeaponGroupId`

서버는 이 값을 그대로 믿지 않는다.

### 5.1 서버 검증 항목

서버는 발사 요청을 받으면 다음을 검사한다.

1. 요청한 Pawn이 실제로 해당 클라이언트 소유인가
2. Pawn이 살아 있는가
3. Pawn이 발사 가능한 상태인가
4. 활성 무기 그룹이 유효한가
5. 탄약이 충분한가
6. 쿨다운이 끝났는가
7. 요청 조준 방향이 무기 허용각 안인가
8. 요청 AimOrigin이 비정상적으로 조작되지 않았는가
9. 서버 Trace 기준 결과가 유효한가
10. 서버 기준으로 Damage 적용 대상이 유효한가

### 5.2 Fire Reject Reason

거부 사유 enum 후보는 다음과 같다.

- `None`
- `NoAuthority`
- `InvalidOwner`
- `VehicleDisabled`
- `NoWeapon`
- `WeaponCooldown`
- `NoAmmo`
- `OutOfWeaponArc`
- `AimBlocked`
- `InvalidAimOrigin`
- `InvalidAimDirection`
- `ServerTraceMiss`

거부 사유는 VehicleDebug Aim 또는 Network 카테고리에 표시한다.

---

## 6. RPC 설계

초기 후보는 다음과 같다.

```text
ServerRequestFire(FireRequest)
  Client -> Server
  Reliable 후보

ClientReceiveFireResult(FireResult)
  Server -> Owner Client
  Reliable 후보

MulticastPlayFireFx(FireFxData)
  Server -> All Clients
  Unreliable 후보
```

### 6.1 Reliable / Unreliable 기준

- 발사 요청은 초기에는 `Reliable`로 둔다.
- 연사 무기가 많아지면 입력 묶음 방식으로 재검토한다.
- 발사 FX Multicast는 `Unreliable` 후보가 적절하다.
- Damage 결과는 서버 상태 복제를 통해 보장한다.

---

## 7. HitScan / Projectile 기준

### 7.1 HitScan

1차 멀티플레이 구현은 HitScan 더미를 우선한다.

이유:

- 서버 검증이 단순하다.
- 조준각 / Trace / Debug 검증이 쉽다.
- Projectile 예측 문제를 뒤로 미룰 수 있다.

흐름:

```text
Client Fire Input
  -> ServerRequestFire
  -> Server Validate
  -> Server Trace
  -> FireResult
  -> Multicast FireFx
```

### 7.2 Projectile

Projectile은 후속 단계로 미룬다.

Projectile 단계에서 다룰 항목:

- 서버 Projectile Spawn
- Projectile Movement 복제
- 클라이언트 임시 Projectile 예측 여부
- 서버 Projectile 보정
- 충돌 판정 서버 권한

---

## 8. 예측과 보정

초기 AimNet은 완전한 예측 시스템을 목표로 하지 않는다.

초기 목표는 다음과 같다.

- Reticle은 로컬에서 즉시 반응한다.
- Fire 입력 시 로컬 FX는 즉시 재생할 수 있다.
- 서버 결과가 오면 성공 / 실패만 보정한다.
- Damage와 실제 피격은 서버 결과만 인정한다.

Reticle 상태 후보:

- `Ready`
- `Blocked`
- `OutOfArc`
- `NoWeapon`
- `Cooldown`
- `Reloading`
- `WaitingServer`
- `ServerRejected`

---

## 9. 복제 전략

### 9.1 복제하지 않을 것

- Local Reticle 상세 상태
- 매 프레임 Camera Trace 전체
- 긴 Debug 문자열
- Owner 전용 Fire Prediction 정보

### 9.2 복제할 것

- 무기 방향 시각화에 필요한 최소 Aim Direction
- 발사 연출 상태
- Projectile Actor 또는 HitScan FX
- Damage 결과
- 차량 상태 변화

### 9.3 최적화 후보

초기에는 단순 구조로 시작한다.

후속 최적화 후보:

- Yaw / Pitch 압축 복제
- 변화량이 일정 이상일 때만 복제
- 발사 중일 때 우선 복제
- 무기 Actor 회전만 복제
- OwnerOnly Debug 분리

---

## 10. Debug 기준

멀티플레이 Aim Debug는 다음을 분리해서 보여야 한다.

```text
Local Aim
Server Aim
Replicated Aim Visual
Last Fire Request
Last Fire Result
Last Reject Reason
```

필수 Debug 항목:

- `LocalRole`
- `RemoteRole`
- `bHasAuthority`
- `bIsLocallyControlled`
- `LocalReticleState`
- `bLocalCanFire`
- `bServerCanFire`
- `LastFireRequestId`
- `LastFireRejectReason`
- `RepAimDirection`

---

## 11. 1차 AimNet 완료 조건

- Listen Server에서 Owner Client Reticle이 즉시 반응한다.
- Client가 Fire 입력을 누르면 `ServerRequestFire`가 호출된다.
- 서버가 조준각 밖 요청을 거부한다.
- 서버가 유효 요청을 승인한다.
- Owner Client가 Fire Result를 받는다.
- 모든 클라이언트가 Fire FX를 본다.
- VehicleDebug에서 Local / Server / Replicated 상태를 구분해서 확인할 수 있다.
