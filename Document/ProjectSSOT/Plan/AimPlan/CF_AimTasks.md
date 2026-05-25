# CarFight — CF_AimTasks

> 역할: Aim 시스템의 실제 구현 작업 순서를 작은 단위로 분해한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Planning

---

## 1. 작업 원칙

Aim 시스템은 멀티플레이까지 고려해야 하므로 구현 순서를 기능 완성 중심이 아니라 검증 가능한 작은 단위 중심으로 나눈다.

원칙은 다음과 같다.

- 문서 기준을 먼저 닫는다.
- Local Aim을 먼저 만든다.
- Debug를 먼저 붙인다.
- 그다음 Server Fire Request 더미를 만든다.
- 실제 Damage / Projectile / Lock-On은 후속 단계로 미룬다.

---

## 2. Phase 0 — 문서 기준 정리

### 목표

AimPlan 문서 묶음을 만들고, 기존 기능 / 미래 기능 / 멀티플레이 관계를 정리한다.

### 작업

- [x] `AimPlan/README.md` 작성
- [x] `CF_AimContext.md` 작성
- [x] `CF_AimNet.md` 작성
- [x] `CF_AimSpec.md` 작성
- [x] `CF_AimDesign.md` 작성
- [x] `CF_AimTasks.md` 작성
- [x] `CF_AimVerify.md` 작성

### 완료 조건

- Aim의 프로젝트 내 위치가 문서화되어 있다.
- 멀티플레이 권한 원칙이 문서화되어 있다.
- 1차 구현 범위와 제외 범위가 명확하다.

---

## 3. Phase 1 — Aim 타입 정의

### 목표

C++ 타입을 먼저 정의해 이후 구현의 기준을 만든다.

### 대상 파일

```text
CFVehicleAimTypes.h
```

### 작업

- [ ] `ECFVehicleReticleState` 정의
- [ ] `ECFVehicleFireRejectReason` 정의
- [ ] `FCFVehicleAimProfile` 정의
- [ ] `FCFVehicleLocalAimState` 정의
- [ ] `FCFVehicleServerAimState` 정의
- [ ] `FCFVehicleRepAimVisualState` 정의
- [ ] `FCFVehicleFireRequest` 정의
- [ ] `FCFVehicleFireResult` 정의

### 완료 조건

- 모든 타입이 `BlueprintType` 또는 필요한 UE 타입으로 노출되어 있다.
- 변수 이름과 Tooltip이 직관적이다.
- 네트워크 전송 후보 필드는 `FVector_NetQuantize` 계열 사용을 검토했다.

---

## 4. Phase 2 — UCFVehicleAimComp 골격

### 목표

AimComp를 생성하고 Pawn에서 소유할 수 있게 한다.

### 대상 파일

```text
CFVehicleAimComp.h
CFVehicleAimComp.cpp
CFVehiclePawn.h
CFVehiclePawn.cpp
```

### 작업

- [ ] `UCFVehicleAimComp` 클래스 생성
- [ ] `ACFVehiclePawn` 생성자에서 AimComp 생성
- [ ] `GetVehicleAimComp()` 추가
- [ ] `InitializeAimRuntime()` 추가
- [ ] Owner Pawn 캐시 추가
- [ ] VehicleCameraComp 참조 해결 함수 추가

### 완료 조건

- 빌드 성공
- `BP_CFVehiclePawn`에서 AimComp가 보인다.
- BeginPlay에서 AimComp가 CameraComp를 찾을 수 있다.

---

## 5. Phase 3 — Local Aim 계산

### 목표

소유 클라이언트 기준으로 즉시 반응하는 Aim State를 계산한다.

### 작업

- [ ] CameraRuntimeState 읽기
- [ ] LocalAimTargetLocation 계산
- [ ] LocalAimDirection 계산
- [ ] 차량 기준 Yaw / Pitch 계산
- [ ] AimProfile 범위 판정
- [ ] Local Reticle State 계산
- [ ] `GetLocalAimState()` 추가
- [ ] `GetReticleState()` 추가

### 완료 조건

- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- 무기 조준각 밖이면 `OutOfArc`가 된다.
- Trace가 막히면 `Blocked`가 된다.
- 발사 가능 예측이면 `Ready`가 된다.

---

## 6. Phase 4 — VehicleDebug Aim 연결

### 목표

Aim 상태를 눈으로 검증할 수 있게 한다.

### 대상 파일

```text
CFVehiclePawn.h
CFVehiclePawn.cpp
UI/CFVehicleDebugPanelWidget.h
UI/CFVehicleDebugPanelWidget.cpp
```

### 작업

- [ ] `FCFVehicleDebugAim` 구조 추가
- [ ] `FCFVehicleDebugSnapshot`에 Aim 추가
- [ ] `GetVehicleDebugAim()` 추가
- [ ] Debug Snapshot에서 AimComp 상태 읽기
- [ ] `UCFVehicleDebugPanelWidget`에 CachedAim 추가
- [ ] `BuildAimSectionViewData()` 추가
- [ ] Navigation에 Aim 카테고리 추가

### 완료 조건

- VehicleDebug Panel에서 Aim 카테고리를 볼 수 있다.
- Local / Server / Replicated Aim 항목이 분리되어 보인다.
- Last Fire Request / Reject Reason 표시 자리가 있다.

---

## 7. Phase 5 — Fire Request 더미

### 목표

발사 입력을 서버 요청으로 보낼 준비를 한다.

### 작업

- [ ] 발사 Input Action 후보 결정
- [ ] `InputAction_Fire` 참조 추가
- [ ] `HandleFireStarted()` 추가
- [ ] `BuildFireRequest()` 추가
- [ ] `ServerRequestFire()` 더미 RPC 추가
- [ ] `ClientReceiveFireResult()` 더미 RPC 추가
- [ ] FireRequestId 증가 로직 추가

### 완료 조건

- Client에서 발사 입력을 누르면 서버 RPC가 호출된다.
- 서버가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.

---

## 8. Phase 6 — 서버 검증

### 목표

서버가 클라이언트 발사 요청을 검증한다.

### 작업

- [ ] Owner 검증
- [ ] Pawn 유효성 검증
- [ ] AimDirection 유효성 검증
- [ ] AimOrigin 거리 검증
- [ ] Weapon Arc 검증
- [ ] 쿨다운 더미 검증
- [ ] Reject Reason 설정
- [ ] ServerAimState 갱신

### 완료 조건

- 잘못된 조준 방향 요청은 거부된다.
- 조준각 밖 요청은 거부된다.
- 유효 요청은 승인된다.
- Reject Reason이 Debug에 표시된다.

---

## 9. Phase 7 — 서버 HitScan 더미

### 목표

서버에서 Trace를 실행해 더미 발사 결과를 만든다.

### 작업

- [ ] 서버 Trace 시작점 결정
- [ ] 서버 Trace 방향 결정
- [ ] 서버 Trace 거리 결정
- [ ] HitActor / HitLocation / HitNormal 기록
- [ ] FireResult에 서버 결과 기록
- [ ] Debug Line 출력 옵션 추가

### 완료 조건

- 서버 기준 Trace가 실행된다.
- 서버 HitLocation이 FireResult에 들어간다.
- Damage는 적용하지 않는다.
- Debug로 서버 Trace를 확인할 수 있다.

---

## 10. Phase 8 — Replicated Aim Visual

### 목표

다른 클라이언트가 최소 조준 시각 상태를 볼 수 있게 한다.

### 작업

- [ ] `FCFVehicleRepAimVisualState` 변수 추가
- [ ] Replication 설정
- [ ] `OnRep_RepAimVisualState()` 후보 추가
- [ ] AimDirection 변화 시 서버 상태 갱신
- [ ] Simulated Proxy에서 시각 보간 후보 추가

### 완료 조건

- 다른 클라이언트에서 무기 방향 또는 조준 방향 시각화를 확인할 수 있다.
- 전투 판정과 시각 복제 정보가 분리되어 있다.

---

## 11. Phase 9 — Reticle UI 최소 버전

### 목표

Owner Client에서 Reticle State를 볼 수 있게 한다.

### 대상 에셋 / 파일 후보

```text
WBP_AimReticle
CFAimReticleWidget.h
CFAimReticleWidget.cpp
```

### 작업

- [ ] Reticle C++ 부모 필요 여부 결정
- [ ] WBP_AimReticle 생성
- [ ] AimComp 참조 연결
- [ ] ReticleState별 표시 분기
- [ ] `WaitingServer` / `ServerRejected` 표시 추가

### 완료 조건

- Ready / Blocked / OutOfArc 상태가 화면에서 구분된다.
- 서버 응답 대기 상태를 표시할 수 있다.
- 서버 거부 상태를 표시할 수 있다.

---

## 12. Phase 10 — 후속 연결 준비

### 목표

Aim 시스템을 미래 Weapon / Damage / Lock-On으로 넘길 준비를 한다.

### 작업

- [ ] WeaponComp 후보 설계 메모 작성
- [ ] AimData / WeaponData 분리 후보 정리
- [ ] Damage 연결 시 필요한 Hit 정보 목록 정리
- [ ] Lock-On 확장용 AimTargetMode 후보 정리
- [ ] AI AimSource 후보 정리

### 완료 조건

- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.

---

## 13. 우선 구현 순서 요약

```text
1. 타입 정의
2. AimComp 골격
3. Local Aim 계산
4. VehicleDebug Aim
5. Fire Request 더미
6. 서버 검증
7. 서버 HitScan 더미
8. RepAimVisual
9. Reticle UI
10. Weapon / Damage 후속 준비
```
