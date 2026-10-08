# CarFight — CF_AimRoadmap

> 역할: AimPlan 문서 묶음을 기준으로 실제 개발 진행 순서, 의존성, 완료 조건을 고정한다.  
> 문서 버전: v0.3.0  
> 마지막 정리(Asia/Seoul): 2026-06-19  
> 상태: Draft / Single Player Roadmap

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
```

따라서 구현 순서는 다음 기준을 따른다.

1. 문서와 타입 기준을 먼저 닫는다.
2. Local Aim을 먼저 구현한다.
3. Debug를 빠르게 붙인다.
4. 그다음 Local Fire Command를 붙인다.
5. 실제 Damage / Projectile / Lock-On은 후속 단계로 미룬다.

---

## 2. 상위 원칙

### 2.1 C++ / BP 분담

C++ 담당:

- Aim State 계산
- Weapon Arc 판정
- Reticle State 결정
- Fire Command / Result 구조
- Local Fire 검증
- Local Fire Visual State
- VehicleDebug Snapshot

BP 담당:

- Reticle Widget 시각 표현
- Reticle 색상 / 애니메이션
- DataAsset 연결
- 테스트용 표시 에셋

### 2.2 싱글플레이 원칙

```text
Local Aim
  -> 플레이어 입력과 카메라 기준 조준감

Local Fire Validation
  -> 조준각 / 차단 / 무기 상태 검증

Local Fire Visual
  -> Reticle / Debug / FX 표시 상태
```

전투 결과는 현재 싱글플레이 로컬 런타임에서 확정한다. 네트워크 RPC, 복제, Dedicated Server 검증은 현재 로드맵에서 제외한다.

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
| R5 | Local Fire Command | 발사 입력과 로컬 발사 명령 흐름 추가 | 예정 |
| R6 | Local Fire 검증 | 로컬에서 발사 승인/거부 | 예정 |
| R7 | Local HitScan 더미 | 로컬 Trace 결과 생성 | 예정 |
| R8 | Fire Visual | 로컬 발사 표시 상태와 FX 연결 준비 | 예정 |
| R9 | Reticle UI | 플레이어 조준점 UI 최소 구현 | 예정 |
| R10 | 후속 연결 준비 | Weapon / Damage / Lock-On 확장 경계 정리 | 예정 |

---

## 4. R0 — 문서 기준 정리

### 목표

Aim 시스템의 목적, 싱글플레이 원칙, 기능 명세, 구조 설계, 작업 순서, 검증 기준을 문서화한다.

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
- Local Aim / Fire Validation / Fire Visual 분리가 명확하다.
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
- `FCFVehicleFireValidationState`
- `FCFVehicleFireVisualState`
- `FCFVehicleFireCommand`
- `FCFVehicleFireResult`

### 설계 기준

- 모든 BP 노출 후보에는 `BlueprintType`을 사용한다.
- BP에서 읽을 변수에는 `BlueprintReadOnly`를 사용한다.
- 디테일 패널 튜닝 변수에는 직관적인 `ToolTip`을 넣는다.
- 위치 / 방향 값은 디버그 가독성과 후속 전투 연결을 고려해 명확한 타입으로 둔다.

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

플레이어 입력에 즉시 반응하는 Local Aim State를 계산한다.

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
- Local Aim / Fire Validation / Fire Visual 값이 구분되어 보인다.
- LastFireCommandId / LastRejectReason 표시 자리가 있다.

---

## 9. R5 — Local Fire Command

### 목표

발사 입력을 로컬 발사 명령으로 처리할 수 있는 최소 흐름을 만든다.

### 구현 항목

- `InputAction_Fire` 후보 추가
- `HandleFireStarted()` 추가
- `BuildFireCommand()` 추가
- `ValidateFireCommand()` 더미 추가
- `ApplyFireResult()` 더미 추가
- FireRequestId 증가 로직 추가

### 완료 조건

- 플레이어가 Fire 입력을 누르면 로컬 발사 명령이 생성된다.
- FireCommandId가 증가한다.
- FireResult가 로컬 Debug와 Reticle에 반영된다.
- Debug에 LastFireRequestId가 표시된다.

---

## 10. R6 — Local Fire 검증

### 목표

로컬 전투 흐름이 Fire Command를 승인하거나 거부한다.

### 구현 항목

- Pawn 유효성 검증
- AimDirection 유효성 검증
- AimOrigin 거리 검증
- Weapon Arc 검증
- 더미 쿨다운 검증
- Reject Reason 설정
- FireValidationState 갱신

### 완료 조건

- 조준각 밖 발사는 거부된다.
- 유효 발사는 승인된다.
- Reject Reason이 Debug에 표시된다.

---

## 11. R7 — Local HitScan 더미

### 목표

로컬 Trace로 더미 발사 결과를 만든다.

### 구현 항목

- 로컬 Trace 시작점 결정
- 로컬 Trace 방향 결정
- 로컬 Trace 거리 결정
- HitActor / HitLocation / HitNormal 기록
- FireResult에 로컬 Hit 결과 기록
- Debug Line 출력 옵션 추가

### 완료 조건

- 로컬 기준 Trace가 실행된다.
- FireResult에 HitLocation이 들어간다.
- Damage는 아직 적용하지 않는다.

---

## 12. R8 — Fire Visual

### 목표

로컬 발사 표시 상태와 FX 연결 지점을 준비한다.

### 구현 항목

- `FCFVehicleFireVisualState` 변수 추가
- AimDirection / HitLocation 표시 상태 갱신
- Fire 입력 시 Muzzle Flash / Camera Shake 후보 연결
- Reticle / VehicleDebug 표시 상태 갱신

### 완료 조건

- 플레이어 화면에서 조준 방향 또는 발사 표시 상태를 확인할 수 있다.
- 전투 판정용 상태와 시각 표시 상태가 분리되어 있다.

---

## 13. R9 — Reticle UI 최소 버전

### 목표

플레이어가 Reticle State를 화면에서 볼 수 있게 한다.

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
- `FireBlocked` / `FireConfirmed` 표시

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

### Commit D — Local Fire Command

포함 단계:

- R5
- R6 일부

검증:

- PIE Single Player
- Fire Command 생성 확인

### Commit E — Local HitScan 더미

포함 단계:

- R6 완료
- R7

검증:

- 로컬 Trace 결과 확인
- FireResult 확인

### Commit F — Fire Visual / Reticle

포함 단계:

- R8
- R9

검증:

- PIE Single Player
- Standalone Game
- Reticle 표시와 로컬 발사 FX 확인

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
- Local Aim / Debug / Fire는 그 다음 커밋으로 나누는 편이 안전하다.
- 현재 프로젝트는 1인 개발 기준이므로 한 번에 너무 큰 범위를 바꾸지 않는다.

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 로드맵을 멀티플레이/RPC/복제 단계에서 싱글플레이 로컬 발사 흐름으로 전환했다.
  - R5~R9 단계를 Local Fire Command, Local Fire 검증, Local HitScan, Fire Visual, Reticle UI로 재정의했다.
  - 검증 기준을 Listen Server/Clients에서 PIE Single Player/Standalone Game 중심으로 바꿨다.

## 마이그레이션 지침

- 신규 작업은 R5 이후 서버/RPC 단계를 진행하지 않는다.
- 기존 코드에 `FireRequestId`가 남아 있으면 우선 유지하고, 문서상 의미를 로컬 발사 명령 ID로 해석한다.
- `RepAimVisual` 관련 구현이 이미 있다면 현재 기능 의존성을 확인한 뒤 `FireVisual`로 별도 리팩터링한다.
