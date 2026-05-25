# CarFight — CF_AimRoadmap

> 역할: AimPlan 문서 묶음을 기준으로 실제 개발 진행 순서, 의존성, 완료 조건을 고정한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Roadmap

---

## 1. 로드맵 목적

이 로드맵은 `AimPlan` 문서 묶음을 기준으로 조준 시스템을 실제 구현 가능한 단계로 나눈다.

Aim 시스템은 단순 조준점 UI가 아니라 다음 계층을 연결하는 전투 준비 코어다.

```text
Camera
  -> Aim
  -> Weapon
  -> Projectile / HitScan
  -> Damage
  -> Vehicle Combat
  -> Destruction / Part Damage
  -> Multiplayer Replication
```

따라서 구현 순서는 다음 기준을 따른다.

1. 문서와 타입 기준을 먼저 닫는다.
2. Local Aim을 먼저 구현한다.
3. Debug를 빠르게 붙인다.
4. 그다음 Server Fire Request를 붙인다.
5. 실제 Damage / Projectile / Lock-On은 후속 단계로 미룬다.

---

## 2. 상위 원칙

### 2.1 C++ / BP 분담

C++ 담당:

- Aim State 계산
- Weapon Arc 판정
- Reticle State 결정
- Fire Request / Result 구조
- Server Fire 검증
- Replicated Aim Visual State
- VehicleDebug Snapshot

BP 담당:

- Reticle Widget 시각 표현
- Reticle 색상 / 애니메이션
- DataAsset 연결
- 테스트용 표시 에셋

### 2.2 멀티플레이 원칙

```text
Local Aim
  -> 소유 클라이언트 조준감 / Reticle 예측

Server Aim
  -> 서버 발사 검증 / 전투 결과 확정

Replicated Aim Visual
  -> 다른 클라이언트용 조준 방향 / 발사 연출
```

전투 결과는 서버가 확정한다. 클라이언트는 발사 결과를 선언하지 않고, 서버에 발사를 요청한다.

### 2.3 장기 전환 원칙

Aim 시스템은 현재 `AWheeledVehiclePawn + ChaosWheeledVehicleMovementComponent` 구조에 붙지만, 장기적으로 `CMVS / Cluster Union / Geometry Collection` 기반 구조 전환을 막지 않아야 한다.

따라서 AimComp는 다음에 의존하지 않는다.

- PoliceCar 전용 하드코딩
- Wheel Index
- ChaosWheeledVehicleMovementComponent 내부 구현
- 현재 차량 root 구조 전용 가정

AimComp가 의존해야 할 것은 다음이다.

- Owner Actor Transform
- Camera Aim State
- Aim Profile
- Weapon Mount Transform 후보
- Aim Target

---

## 3. 전체 단계 요약

| 단계 | 이름 | 목표 | 상태 |
|---:|---|---|---|
| R0 | 문서 기준 정리 | AimPlan 문서 묶음 작성 | 완료 |
| R1 | Aim 타입 정의 | enum / struct 기반 타입 정의 | 예정 |
| R2 | AimComp 골격 | Pawn에 AimComp 추가 및 초기화 | 예정 |
| R3 | Local Aim 계산 | 로컬 조준 대상 / Reticle State 계산 | 예정 |
| R4 | VehicleDebug Aim | Debug Panel에 Aim 카테고리 추가 | 예정 |
| R5 | Fire Request 더미 | 발사 입력과 서버 요청 흐름 추가 | 예정 |
| R6 | Server Fire 검증 | 서버에서 발사 요청 승인/거부 | 예정 |
| R7 | Server HitScan 더미 | 서버 Trace 결과 생성 | 예정 |
| R8 | RepAimVisual | 다른 클라이언트용 조준 시각 상태 복제 | 예정 |
| R9 | Reticle UI | Owner Client 조준점 UI 최소 구현 | 예정 |
| R10 | 후속 연결 준비 | Weapon / Damage / Lock-On 확장 경계 정리 | 예정 |

---

## 4. R0 — 문서 기준 정리

### 목표

Aim 시스템의 목적, 멀티플레이 원칙, 기능 명세, 구조 설계, 작업 순서, 검증 기준을 문서화한다.

### 산출물

```text
README.md
CF_AimContext.md
CF_AimNet.md
CF_AimSpec.md
CF_AimDesign.md
CF_AimTasks.md
CF_AimVerify.md
CF_AimRoadmap.md
CF_AimCodexTask.md
```

### 완료 조건

- Aim의 프로젝트 내 위치가 명확하다.
- Local / Server / Replicated Aim 분리가 명확하다.
- 1차 구현 범위와 제외 범위가 명확하다.

### 현재 상태

완료.

---

## 5. R1 — Aim 타입 정의

### 목표

Aim 시스템의 C++ 타입 기준을 먼저 만든다.

### 대상 파일

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
```

### 구현 항목

- `ECFVehicleReticleState`
- `ECFVehicleFireRejectReason`
- `FCFVehicleAimProfile`
- `FCFVehicleLocalAimState`
- `FCFVehicleServerAimState`
- `FCFVehicleRepAimVisualState`
- `FCFVehicleFireRequest`
- `FCFVehicleFireResult`

### 설계 기준

- 모든 BP 노출 후보에는 `BlueprintType`을 사용한다.
- BP에서 읽을 변수에는 `BlueprintReadOnly`를 사용한다.
- 디테일 패널 튜닝 변수에는 직관적인 `ToolTip`을 넣는다.
- 네트워크 전송 후보 위치 / 방향 값에는 `FVector_NetQuantize` 또는 `FVector_NetQuantizeNormal` 사용을 검토한다.

### 완료 조건

- 프로젝트가 빌드된다.
- 타입명이 기존 Camera 타입과 충돌하지 않는다.
- 타입만 추가하고 기존 동작은 바꾸지 않는다.

---

## 6. R2 — AimComp 골격

### 목표

`UCFVehicleAimComp`를 추가하고 `ACFVehiclePawn`에 소유시킨다.

### 대상 파일

```text
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

### 구현 항목

- `UCFVehicleAimComp` 클래스 생성
- `PrimaryComponentTick` 정책 설정
- `InitializeAimRuntime()` 추가
- Owner Pawn 캐시
- VehicleCameraComp 참조 해결
- `ACFVehiclePawn` 생성자에서 AimComp 생성
- `GetVehicleAimComp()` 추가

### 완료 조건

- `BP_CFVehiclePawn`에 AimComp가 표시된다.
- BeginPlay 이후 AimComp가 CameraComp를 찾을 수 있다.
- 기존 주행 / 카메라 동작이 깨지지 않는다.

---

## 7. R3 — Local Aim 계산

### 목표

Owner Client에서 즉시 반응하는 Local Aim State를 계산한다.

### 구현 항목

- CameraRuntimeState 읽기
- LocalAimTargetLocation 계산
- LocalAimDirection 계산
- 차량 기준 Yaw / Pitch 계산
- AimProfile 범위 판정
- Local Reticle State 계산
- `GetLocalAimState()` 추가
- `GetReticleState()` 추가

### 완료 조건

- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- Weapon Arc 안이면 `Ready` 후보가 된다.
- Weapon Arc 밖이면 `OutOfArc`가 된다.
- Aim Trace가 막히면 `Blocked`가 된다.

---

## 8. R4 — VehicleDebug Aim

### 목표

Aim 상태를 VehicleDebug Panel에서 확인할 수 있게 한다.

### 대상 파일

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

### 구현 항목

- `FCFVehicleDebugAim` 추가
- `FCFVehicleDebugSnapshot`에 Aim 추가
- `GetVehicleDebugAim()` 추가
- Debug Snapshot에서 AimComp 상태 읽기
- `CachedAim` 추가
- `BuildAimSectionViewData()` 추가
- Navigation에 Aim 카테고리 추가

### 완료 조건

- VehicleDebug Panel에서 Aim 섹션을 선택할 수 있다.
- Local / Server / Replicated 값이 구분되어 보인다.
- LastFireRequestId / LastRejectReason 표시 자리가 있다.

---

## 9. R5 — Fire Request 더미

### 목표

발사 입력을 서버 요청으로 보낼 수 있는 최소 흐름을 만든다.

### 구현 항목

- `InputAction_Fire` 후보 추가
- `HandleFireStarted()` 추가
- `BuildFireRequest()` 추가
- `ServerRequestFire()` 더미 RPC 추가
- `ClientReceiveFireResult()` 더미 RPC 추가
- FireRequestId 증가 로직 추가

### 완료 조건

- Client에서 Fire 입력 시 서버 RPC가 호출된다.
- Server가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.

---

## 10. R6 — Server Fire 검증

### 목표

서버가 클라이언트의 Fire Request를 승인하거나 거부한다.

### 구현 항목

- Owner 검증
- Pawn 유효성 검증
- AimDirection 유효성 검증
- AimOrigin 거리 검증
- Weapon Arc 검증
- 더미 쿨다운 검증
- Reject Reason 설정
- ServerAimState 갱신

### 완료 조건

- 조준각 밖 요청은 서버가 거부한다.
- 유효 요청은 서버가 승인한다.
- Reject Reason이 Owner Client Debug에 표시된다.

---

## 11. R7 — Server HitScan 더미

### 목표

서버 Trace로 더미 발사 결과를 만든다.

### 구현 항목

- 서버 Trace 시작점 결정
- 서버 Trace 방향 결정
- 서버 Trace 거리 결정
- HitActor / HitLocation / HitNormal 기록
- FireResult에 서버 결과 기록
- Debug Line 출력 옵션 추가

### 완료 조건

- 서버 기준 Trace가 실행된다.
- FireResult에 ServerHitLocation이 들어간다.
- Damage는 아직 적용하지 않는다.

---

## 12. R8 — RepAimVisual

### 목표

다른 클라이언트가 최소 조준 시각 상태를 볼 수 있게 한다.

### 구현 항목

- `FCFVehicleRepAimVisualState` 변수 추가
- Replication 설정
- `OnRep_RepAimVisualState()` 후보 추가
- AimDirection 변화 시 서버 상태 갱신
- Simulated Proxy에서 시각 보간 후보 추가

### 완료 조건

- 다른 클라이언트에서 조준 방향 또는 무기 방향 시각화가 가능하다.
- 전투 판정용 상태와 시각 복제 상태가 분리되어 있다.

---

## 13. R9 — Reticle UI 최소 버전

### 목표

Owner Client가 Reticle State를 화면에서 볼 수 있게 한다.

### 후보 파일 / 에셋

```text
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
/Game/CarFight/UI/WBP_AimReticle
```

### 구현 항목

- Reticle C++ 부모 필요 여부 결정
- `WBP_AimReticle` 생성
- AimComp 참조 연결
- ReticleState별 표시 분기
- `WaitingServer` / `ServerRejected` 표시

### 완료 조건

- Ready / Blocked / OutOfArc가 화면에서 구분된다.
- Reticle UI가 직접 Trace를 하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.

---

## 14. R10 — 후속 연결 준비

### 목표

Aim 시스템을 Weapon / Damage / Lock-On / AI / Part Damage로 연결할 준비를 한다.

### 구현 항목

- WeaponComp 후보 설계 메모
- AimData / WeaponData 분리 후보 정리
- Damage 연결 시 필요한 Hit 정보 목록 정리
- Lock-On 확장용 AimTargetMode 후보 정리
- AI AimSource 후보 정리

### 완료 조건

- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.

---

## 15. 권장 구현 묶음

### Commit A — Aim 타입과 컴포넌트 골격

포함 단계:

- R1
- R2

검증:

- 빌드 성공
- BP에서 AimComp 확인
- 기존 기능 회귀 없음

### Commit B — Local Aim 계산

포함 단계:

- R3

검증:

- Single Player PIE
- Local Aim Debug 로그 또는 화면 확인

### Commit C — VehicleDebug Aim

포함 단계:

- R4

검증:

- VehicleDebug Panel에서 Aim 카테고리 확인

### Commit D — Fire Request 더미

포함 단계:

- R5
- R6 일부

검증:

- Listen Server + 1 Client
- Fire RPC 호출 확인

### Commit E — Server HitScan 더미

포함 단계:

- R6 완료
- R7

검증:

- 서버 Trace 결과 확인
- FireResult 확인

### Commit F — RepAimVisual / Reticle

포함 단계:

- R8
- R9

검증:

- Listen Server + 2 Clients
- Owner Reticle 표시
- 타 클라이언트 발사 FX 확인

---

## 16. 현재 바로 착수할 작업

현재 바로 착수할 1차 Codex 작업은 `Commit A`로 제한한다.

즉, Codex는 먼저 다음만 구현한다.

```text
R1 — Aim 타입 정의
R2 — AimComp 골격
```

이유:

- 타입과 컴포넌트 골격이 먼저 안정되어야 한다.
- Local Aim / Debug / Network는 그 다음 커밋으로 나누는 편이 안전하다.
- 현재 프로젝트는 1인 개발 기준이므로 한 번에 너무 큰 범위를 바꾸지 않는다.
