# CarFight — CF_AimStatus

> 역할: AimPlan의 현재 완료 상태, 남은 작업, 다음 권장 작업을 한눈에 보기 위해 정리한다.  
> 문서 버전: v1.0.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Completed / Aim Core Phase 1

---

## 1. 현재 상태 요약

Aim 시스템 1차 구현은 완료 상태다.

완료된 범위는 다음이다.

```text
Aim Core
  -> 완료

Reticle C++ / WBP 연결
  -> 완료

Fire Request / Server Validation
  -> 완료

Server Dummy HitScan
  -> 완료

Single Player 기본 확인
  -> 완료
```

아직 완료로 보지 않는 범위는 다음이다.

```text
Listen Server 검증
RepAimVisual 원격 표시 검증
Fire FX Multicast
Damage 적용
WeaponComp 분리
Projectile
Lock-On
AI Combat
Part Damage / Destruction 연동
```

---

## 2. 완료된 C++ 범위

| 단계 | 내용 | 상태 |
|---:|---|---:|
| R1/R2 | Aim 타입 + AimComp 골격 | 완료 |
| R3 | Local Aim 계산 | 완료 |
| R4 | VehicleDebug Aim | 완료 |
| R5/R6 | Fire Request + Server 검증 | 완료 |
| R7 | Server HitScan 더미 | 완료 |
| R8 | RepAimVisual | 완료 |
| R9 | Reticle UI C++ 부모 | 완료 |
| R10 | Reticle Widget Pawn 연결 준비 | 완료 |

---

## 3. 완료된 UE Editor 연결 범위

| 항목 | 상태 |
|---|---:|
| `WBP_AimReticle` 생성 | 완료 |
| `WBP_AimReticle` Parent Class 설정 | 완료 |
| `Text_ReticleState` 배치 | 완료 |
| `Text_CanFire` 배치 | 완료 |
| `Text_ReticleHint` 배치 | 완료 |
| `IA_Fire` 생성 | 완료 |
| `IMC_Vehicle_Default`에 Fire 매핑 | 완료 |
| `BP_CFVehiclePawn.InputAction_Fire` 연결 | 완료 |
| `BP_CFVehiclePawn.AimReticleWidgetClass` 연결 | 완료 |
| Single Player Fire 입력 확인 | 완료 |

---

## 4. 현재 기능 흐름

현재 Aim 시스템 흐름은 다음과 같다.

```text
Look 입력
  -> CameraComp Aim Trace
  -> AimComp Local Aim 계산
  -> ReticleState 계산
  -> WBP_AimReticle 표시

Fire 입력
  -> IA_Fire
  -> HandleFireStarted()
  -> BuildFireRequest()
  -> ServerRequestFire()
  -> ValidateFireRequestOnServer()
  -> RunServerDummyHitScan()
  -> FireResult 생성
  -> ClientReceiveFireResult()
  -> ServerAimState / RepAimVisual 갱신
  -> VehicleDebug Aim에서 확인
```

---

## 5. 현재 제약

현재 Fire는 실제 전투 결과를 만들지 않는다.

즉, 아래는 아직 없다.

```text
총구 이펙트
발사 사운드
Hit Marker
실제 Damage
탄약
쿨다운
재장전
무기 그룹
Projectile
```

따라서 현재 Fire는 다음 목적의 더미 기능이다.

```text
Aim Target 확인
발사 요청 확인
서버 검증 확인
서버 Trace 확인
Debug 상태 확인
```

---

## 6. 다음 권장 작업

바로 기능을 추가하기보다 다음 순서를 권장한다.

```text
1. Listen Server + 1 Client 검증
2. Listen Server + 2 Clients 검증
3. 검증 문제 수정
4. Fire FX Multicast
5. HitScan Damage 더미
6. WeaponComp 설계
```

다음 문서 후보:

```text
CF_AimNetVerify.md
```

이 문서는 멀티플레이 검증 절차만 다룬다.

---

## 7. 현재 판정

```text
AimPlan Phase 1:
  완료

Aim Multiplayer Verification:
  대기

Weapon / Damage Integration:
  미착수
```
