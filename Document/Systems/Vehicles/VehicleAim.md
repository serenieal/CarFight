# VehicleAim

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleAim` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleAim` 기능은 아래 요소를 묶어서 본다.

- 핵심 컴포넌트: `UCFVehicleAimComp`
- 공용 타입 정의: `CFVehicleAimTypes.h`
- 대표 소유 주체: `ACFVehiclePawn`
- 카메라 상태 공급 주체: `UCFVehicleCameraComp`
- 로컬 표시 소비 주체: `UCFAimReticleWidget`
- 서버 발사 요청 연동 주체: `ACFVehiclePawn::ServerRequestFire()` 계열 함수
- 복제 시각 상태: `FCFVehicleRepAimVisualState`
- 현재 대표 입력/전투 연결 경로:
  - `ACFVehiclePawn::BuildFireRequest()`
  - `ACFVehiclePawn::ValidateFireRequestOnServer()`
  - `ACFVehiclePawn::RunServerDummyHitScan()`
  - `ACFVehiclePawn::ServerRequestFire_Implementation()`
  - `ACFVehiclePawn::ClientReceiveFireResult_Implementation()`

즉, 현재 기준 `VehicleAim`은 **카메라 조준점만 계산하는 기능이 아니라, 로컬 조준 표시 / 서버 발사 검증 상태 / 다른 클라이언트용 복제 시각 상태 / FireRequest 생성 보조까지 묶은 조준-발사 중간 계층**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleAim`의 핵심 역할은 **VehicleCamera가 계산한 조준 결과를 차량 무기 조준각 기준으로 해석하고, 로컬 Reticle 표시 상태와 서버 발사 검증 상태, 복제 시각 상태로 나누어 관리하는 것**이다.

현재 `VehicleAim`은 아래 일을 한다.

### 1. Aim 런타임 참조를 준비한다
`UCFVehicleAimComp`는 `BeginPlay()`에서 `InitializeAimRuntime()`을 호출한다.

현재 초기화에서 확인하는 참조:
- Owner 차량 Pawn: `ACFVehiclePawn`
- 차량 카메라 컴포넌트: `UCFVehicleCameraComp`

관련 함수:
- `InitializeAimRuntime()`
- `RefreshAimRuntimeReferences()`
- `ResolveOwnerVehiclePawn()`
- `ResolveVehicleCameraComp()`

현재 동작:
- Owner Actor를 `ACFVehiclePawn`으로 캐스팅한다.
- Owner Pawn에서 `VehicleCameraComp`를 가져온다.
- 두 참조가 모두 있으면 `bAimRuntimeReady = true`로 본다.
- 결과는 `LastAimRuntimeSummary` 문자열로 남긴다.

즉 현재 `VehicleAim`은 **VehicleCamera를 직접 만들거나 소유하지 않고, 현재 차량 Pawn에 붙어 있는 CameraComp를 찾아 조준 해석에 사용한다.**

### 2. Dedicated Server에서는 로컬 Aim 표시를 계산하지 않는다
`UCFVehicleAimComp::TickComponent()`는 Dedicated Server 환경에서 로컬 Reticle 상태를 `Hidden`으로 두고 바로 반환한다.

현재 의미:
- Dedicated Server는 Viewport 표시가 없으므로 Local Reticle 계산이 필요 없다.
- 서버는 로컬 HUD를 표시하는 주체가 아니다.
- 서버 검증 상태는 발사 요청 처리 흐름에서 별도로 갱신된다.

즉 현재 `VehicleAim`에서 Local Aim은 **소유 클라이언트 표시용 상태**이고, Dedicated Server에서 매 프레임 표시 계산을 수행하지 않는다.

### 3. 로컬 클라이언트 기준 Aim 상태를 계산한다
`RefreshLocalAimState()`는 로컬 제어 Pawn에서만 실제 Local Aim 상태를 갱신한다.

현재 처리 흐름:
1. 런타임 참조가 준비되지 않았으면 다시 참조를 찾는다.
2. 참조가 없으면 Reticle을 `Hidden`으로 둔다.
3. Pawn이 로컬 제어가 아니면 Reticle을 `Hidden`으로 둔다.
4. `VehicleCameraComp->GetCameraRuntimeState()`에서 카메라 런타임 상태를 읽는다.
5. 카메라의 `AimHitLocation`을 목표 위치로 사용한다.
6. Owner 차량 위치에서 목표 위치까지의 방향을 구한다.
7. 목표 위치가 비정상적이면 차량 정면 방향을 fallback으로 사용한다.
8. 월드 조준 방향을 차량 로컬 Yaw/Pitch 각도로 변환한다.
9. `DefaultAimProfile`의 Yaw/Pitch 범위 안에 있는지 확인한다.
10. 카메라 런타임 상태의 `bAimBlocked`를 읽는다.
11. `bWithinWeaponArc && !bAimBlocked`이면 로컬 발사 가능으로 본다.
12. 결과를 `FCFVehicleLocalAimState`에 저장한다.

현재 `LocalAimState`에 저장되는 핵심 값:
- `LocalAimTargetLocation`
- `LocalAimDirection`
- `LocalReticleState`
- `bLocalCanFire`
- `bLocalWithinWeaponArc`
- `bLocalAimBlocked`

즉 현재 Local Aim은 **서버 최종 판정이 아니라, 소유 클라이언트가 즉시 Reticle을 표시하기 위한 예측/표시 상태**다.

### 4. 차량 로컬 조준각을 계산한다
`CalculateAimAnglesRelativeToVehicle()`는 월드 조준 방향을 차량 Actor의 로컬 공간 방향으로 변환한다.

현재 계산 방식:
- `OwnerVehiclePawn->GetActorTransform().InverseTransformVectorNoScale(AimDirection)`으로 월드 방향을 차량 로컬 방향으로 변환한다.
- 로컬 방향의 `Y/X`로 Yaw를 계산한다.
- 로컬 방향의 `Z / 수평 길이`로 Pitch를 계산한다.

현재 의미:
- 차량 정면 기준 좌우 조준각은 Yaw로 본다.
- 차량 기준 상하 조준각은 Pitch로 본다.
- 이 값이 `DefaultAimProfile` 범위 안에 있어야 무기 조준각 안으로 본다.

### 5. Default Aim Profile로 조준 가능 범위를 제한한다
현재 `UCFVehicleAimComp`는 `FCFVehicleAimProfile DefaultAimProfile`을 가진다.

현재 기본 프로필 항목:
- `ProfileName`
- `MinYawDeg`
- `MaxYawDeg`
- `MinPitchDeg`
- `MaxPitchDeg`
- `MaxAimDistance`

`IsAimWithinDefaultProfile()`은 현재 Yaw/Pitch가 기본 프로필 범위 안에 있는지만 검사한다.

현재 의미:
- 이 프로필은 아직 무기별/터렛별 완성 튜닝 체계가 아니라, Aim 시스템이 가진 기본 제한값이다.
- 현재 발사 요청의 `WeaponGroupId`는 `DefaultAimProfile.ProfileName`을 사용한다.
- 후속 무기 시스템이 들어오면 무기별 AimProfile로 분리될 수 있다.

### 6. Reticle 상태를 만든다
`BuildLocalReticleState()`는 현재 Local Aim 조건을 `ECFVehicleReticleState`로 바꾼다.

현재 실제 계산에서 사용하는 상태:
- `Hidden`
- `Blocked`
- `OutOfArc`
- `Ready`

현재 판정 순서:
1. Aim 런타임이 준비되지 않으면 `Hidden`
2. 조준이 막혔으면 `Blocked`
3. 무기 조준각 밖이면 `OutOfArc`
4. 발사 가능이면 `Ready`
5. 나머지는 `Hidden`

`ECFVehicleReticleState` enum에는 그 외에도 아래 상태가 정의돼 있다.
- `NoWeapon`
- `Cooldown`
- `Reloading`
- `WaitingServer`
- `ServerRejected`

현재 의미:
- enum은 후속 무기/탄약/쿨다운/서버 응답 상태까지 고려해 넓게 정의돼 있다.
- 하지만 현재 실제 `BuildLocalReticleState()` 계산에서 사용하는 상태는 제한적이다.

### 7. FireRequest를 생성한다
`UCFVehicleAimComp::BuildFireRequest()`는 현재 Local Aim 상태를 기반으로 서버 발사 요청 데이터를 만든다.

현재 채우는 값:
- `FireRequestId`
- `ClientFireTimeSeconds`
- `PredictedAimTargetLocation`
- `AimDirection`
- `WeaponGroupId`
- `AimOrigin`

현재 `AimOrigin`:
- `OwnerVehiclePawn`이 있으면 차량 Actor 위치를 사용한다.

현재 의미:
- 클라이언트는 현재 조준 방향과 예측 목표 위치를 서버에 보낼 수 있다.
- 이 값은 서버 최종 판정이 아니라 요청 데이터다.
- 서버는 별도 검증을 수행한다.

### 8. 서버 발사 결과를 Aim 상태에 반영한다
현재 서버 발사 결과는 두 경로로 Aim 상태에 반영된다.

관련 함수:
- `BuildServerAimStateFromFireRequest()`
- `ApplyServerFireResult()`
- `UpdateRepAimVisualFromFireResult()`

현재 `BuildServerAimStateFromFireRequest()` 역할:
- 서버가 받은 요청의 예측 목표 위치를 서버 Aim 상태에 저장한다.
- 요청 조준 방향이 기본 프로필 안에 있는지 계산한다.
- 승인 여부와 거부 사유를 저장한다.
- 승인/거부된 요청 ID를 기록한다.

현재 `ApplyServerFireResult()` 역할:
- 서버 확정 목표 위치를 `ServerAimState`에 반영한다.
- 승인 여부를 `bServerCanFire`에 반영한다.
- 거부 사유와 요청 ID를 기록한다.

현재 의미:
- Server Aim은 Local Aim과 별도 상태다.
- Local Aim은 즉시 표시용이고, Server Aim은 발사 검증 결과 기록용이다.

### 9. 다른 클라이언트용 복제 Aim 시각 상태를 갱신한다
`RepAimVisualState`는 `ReplicatedUsing=OnRep_RepAimVisualState`로 복제된다.

현재 `UpdateRepAimVisualFromFireResult()` 동작:
- Authority가 없으면 실행하지 않는다.
- 발사 요청의 조준 방향을 정규화한다.
- 비정상 방향이면 Local Aim 방향 또는 차량 정면 방향을 fallback으로 사용한다.
- 서버 결과의 목표 위치, 적중 위치, 요청 예측 목표 위치 순서로 복제 목표 위치를 결정한다.
- 발사 승인 여부를 `bIsFiringVisual`에 넣는다.
- `WeaponGroupId`를 `RepWeaponVisualMode`에 넣는다.

현재 의미:
- `RepAimVisualState`는 전투 판정용 데이터가 아니다.
- 다른 클라이언트에게 조준 방향/목표/발사 시각화를 보여주기 위한 최소 시각 상태다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleAim`은 아래 역할을 가진다.

1. **카메라 조준 결과 해석 계층**
   - `VehicleCameraComp`의 `CameraRuntimeState`를 읽는다.
   - Aim 목표 위치와 방향을 차량 기준 각도로 변환한다.
   - 기본 AimProfile의 조준각 안/밖을 판정한다.

2. **로컬 Reticle 상태 공급 계층**
   - Local Aim 상태를 만든다.
   - Reticle 상태와 발사 가능 예측 값을 제공한다.
   - Dedicated Server나 원격 Pawn에서는 로컬 표시 상태를 숨긴다.

3. **서버 발사 검증 상태 기록 계층**
   - FireRequest를 만든다.
   - 서버 처리 결과를 ServerAimState에 반영한다.
   - 승인/거부 요청 ID와 거부 사유를 기록한다.

4. **복제 시각 상태 계층**
   - 다른 클라이언트에게 보여줄 최소 조준 방향/목표/발사 시각 상태를 보관한다.
   - 실제 전투 판정용이 아니라 시각화용 데이터로 본다.

따라서 현재 `VehicleAim`은 단순한 조준점 계산기가 아니라,
**VehicleCamera와 WeaponFire 사이에서 Local 표시, 서버 검증, 복제 시각화를 분리해주는 차량 조준 운영 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `VehicleAim`은 아래 방식으로 동작한다.

### 1. 생성 및 초기화
`UCFVehicleAimComp` 생성자에서 수행하는 일:
- Tick 활성화
- 기본 복제 활성화
- `bAimRuntimeReady = false`
- `LastAimRuntimeSummary = "Constructed"`

`BeginPlay()`에서 수행하는 일:
- `InitializeAimRuntime()` 호출

### 2. Tick 기반 Local Aim 갱신
매 Tick에서 수행하는 일:
- Dedicated Server면 Reticle을 숨기고 종료
- 그 외에는 `RefreshLocalAimState()` 호출

### 3. 발사 요청 생성 및 처리 연결
현재 발사 요청의 상위 흐름은 `ACFVehiclePawn` 쪽에서 관리한다.

대표 흐름:
1. `ACFVehiclePawn::HandleFireStarted()`
2. `ACFVehiclePawn::BuildFireRequest()`
3. `VehicleAimComp->BuildFireRequest()`
4. `ACFVehiclePawn::ServerRequestFire()`
5. `ACFVehiclePawn::ValidateFireRequestOnServer()`
6. `VehicleAimComp->BuildServerAimStateFromFireRequest()`
7. `VehicleAimComp->ApplyServerFireResult()`
8. `VehicleAimComp->UpdateRepAimVisualFromFireResult()`
9. `ACFVehiclePawn::ClientReceiveFireResult()`

현재 구조 해석:
- `VehicleAimComp`는 발사 요청 데이터를 만들고 Aim 상태를 갱신한다.
- 실제 서버 RPC 진입점은 `ACFVehiclePawn`이 가진다.
- 실제 HitScan 더미 Trace도 `ACFVehiclePawn`이 수행한다.

## 현재 표시 조건 / 실행 조건
현재 `VehicleAim`이 정상 동작하려면 아래 조건이 중요하다.

- Owner가 `ACFVehiclePawn`이어야 한다.
- Owner Pawn에 `VehicleCameraComp`가 있어야 한다.
- Local Aim 계산은 `OwnerVehiclePawn->IsLocallyControlled()`인 경우에만 의미가 있다.
- Dedicated Server에서는 Local Reticle 계산을 하지 않는다.
- 서버 발사 검증은 Authority가 있는 서버에서 수행되어야 한다.
- `RepAimVisualState` 갱신은 Authority에서만 수행된다.

## 현재 자산 / 클래스 역할

### `UCFVehicleAimComp`
- 종류: C++ ActorComponent
- 현재 역할: Aim 런타임 참조 준비, Local Aim 계산, Reticle 상태 제공, FireRequest 생성, ServerAimState 갱신, RepAimVisualState 복제 관리

### `CFVehicleAimTypes.h`
- 종류: C++ 공용 타입
- 현재 역할: Reticle 상태 enum, FireRejectReason enum, AimProfile, Local/Server/Rep Aim 상태, FireRequest/FireResult 구조 정의

### `ACFVehiclePawn`
- 종류: C++ Pawn
- 현재 역할: VehicleAimComp 소유, 발사 입력 처리, FireRequest 생성 요청, 서버 검증, 서버 더미 HitScan, 클라이언트 결과 수신

### `UCFVehicleCameraComp`
- 종류: C++ ActorComponent
- 현재 역할: Aim 목표 위치, Aim 가림 여부 등 카메라 런타임 상태 공급

### `UCFAimReticleWidget`
- 종류: C++ UserWidget 부모 클래스
- 현재 역할: VehicleAimComp의 Local Aim/Reticle 상태를 읽어 UI 표시로 변환

## 현재 생성 및 연결 구조
현재 연결 구조는 아래와 같다.

```text
ACFVehiclePawn
  -> VehicleCameraComp
  -> VehicleAimComp
      -> OwnerVehiclePawn 캐시
      -> VehicleCameraComp 캐시
      -> LocalAimState 갱신
      -> ReticleState 제공
      -> FireRequest 생성 보조
      -> ServerAimState / RepAimVisualState 갱신
```

발사 요청 처리 구조는 아래와 같다.

```text
HandleFireStarted
  -> BuildFireRequest
  -> VehicleAimComp.BuildFireRequest
  -> ServerRequestFire
  -> ValidateFireRequestOnServer
  -> RunServerDummyHitScan
  -> VehicleAimComp.BuildServerAimStateFromFireRequest
  -> VehicleAimComp.ApplyServerFireResult
  -> VehicleAimComp.UpdateRepAimVisualFromFireResult
  -> ClientReceiveFireResult
```

## 현재 기능 책임
현재 `VehicleAim`의 책임은 아래와 같다.

- Owner Pawn과 VehicleCameraComp 참조를 준비한다.
- CameraRuntimeState의 Aim 목표 위치를 읽는다.
- 차량 로컬 기준 Aim Yaw/Pitch를 계산한다.
- 기본 AimProfile 범위 안/밖을 판정한다.
- 로컬 Reticle 상태를 계산한다.
- 로컬 발사 가능 예측 값을 보관한다.
- FireRequest 생성을 보조한다.
- 서버 발사 결과를 ServerAimState에 반영한다.
- 다른 클라이언트용 복제 Aim 시각 상태를 관리한다.

## 현재 기준 비책임 항목
현재 구현상 `VehicleAim`의 직접 책임이 아닌 것은 아래와 같다.

- 실제 무기 장착/해제 시스템
- 탄약 수량 관리
- 쿨다운/재장전 시간 계산
- 실제 데미지 적용
- 실제 투사체 생성
- 실제 터렛 회전 애니메이션
- 서버 RPC 함수의 직접 소유
- Dedicated Server에서 UI 생성
- 최종 전투 밸런스 결정

현재 `VehicleAim`은 **무기 시스템 자체가 아니라, 카메라 조준 결과와 향후 무기 발사 사이의 조준 해석/상태 보관 계층**이다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleAim` 기능은,

**VehicleCamera가 만든 조준 결과를 Local 표시, 서버 검증, 복제 시각화 상태로 나누어 관리하는 차량 조준 중간 계층**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleAim`은 현재 차량의 조준 방향이 무기 조준각 안에 있는지, 조준이 막혔는지, 로컬 표시와 서버 발사 검증에서 어떤 상태로 읽혀야 하는지를 정리해주는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- 실제 무기 데이터/터렛 데이터가 `DefaultAimProfile`을 대체하는 최종 경로
- `NoWeapon`, `Cooldown`, `Reloading`, `WaitingServer`, `ServerRejected` 상태를 실제로 전환하는 최종 운영 경로
- `RepAimVisualState`를 실제 다른 클라이언트의 시각 이펙트가 소비하는 최종 경로
- `AimProfileOverride` 또는 무기별 AimProfile과 `VehicleCamera`의 연결 정책
- 서버 발사 검증에서 클라이언트 LocalAimState의 `bLocalAimBlocked`를 계속 참고할지 장기 정책

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `UCFVehicleAimComp`의 Local Aim 계산 규칙 변경
- `ECFVehicleReticleState` 상태 전환 규칙 변경
- `FCFVehicleAimProfile` 구조 변경
- FireRequest/FireResult 구조 변경
- 서버 발사 검증 책임이 `ACFVehiclePawn`에서 다른 시스템으로 이동할 때
- `RepAimVisualState` 복제 정책 변경
- 무기/터렛 데이터와 AimProfile 연결 방식 변경

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
- 문서 상태: `Initial`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준
- `Major`
  - 기능 해석 자체가 바뀌는 수준의 대규모 재작성
  - Aim이 무기 시스템 전체 문서로 확장되거나 분리될 때
- `Minor`
  - 새로운 Aim 상태, 발사 검증 항목, 복제 시각 항목이 추가될 때
  - 무기/터렛 데이터와 실제 연결될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.0 - 2026-06-02
- `VehicleAim` 시스템 문서 최초 작성
- `UCFVehicleAimComp`, `CFVehicleAimTypes`, `ACFVehiclePawn` 발사 요청 흐름 기준으로 현재 기능 범위 정리
- Local Aim / Server Aim / Rep Aim Visual 책임 분리 기록
- 현재 Reticle 상태 계산 범위와 미확정 상태 기록

## 마지막 확인 기준
- 확인 일시: `2026-06-02`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
