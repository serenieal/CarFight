# CFVNetDbg 코드 리뷰 / 빌드 검증

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 대상 작업: 차량 네트워크 물리 흔들림 1차 진단 패치
- 상태: Build Passed / Runtime Test Pending

---

## 1. 검토 목적

이 문서는 Codex가 적용한 `CFVNetDbg` 진단 패치의 실제 코드 반영 상태와 빌드 검증 결과를 기록한다.

이번 패치는 차량 네트워크 물리 문제의 최종 해결책이 아니다.

목표는 아래 수치를 런타임 로그로 확인할 수 있는 첫 진단 기반을 만드는 것이다.

```text
- 서버 권위 위치/회전/속도 샘플
- 클라이언트 로컬 위치/회전/속도
- 서버 샘플 대비 클라이언트 오차
- bReplicateMovement true/false 비교
```

---

## 2. 수정 파일

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

---

## 3. 확인된 구현 내용

### 3.1 추가 구조체

`CFVehiclePawn.h`에 아래 구조체가 추가되었다.

```text
FCFVehicleNetDebugSample
```

현재 포함 필드:

```text
- bValid
- SampleSequenceId
- ServerWorldTimeSeconds
- ServerLocation
- ServerRotation
- ServerLinearVelocity
```

판단:

```text
첫 진단용 최소 상태로는 사용 가능하다.
다만 원래 계획에 있던 AngularVelocity 계열은 아직 없다.
```

---

### 3.2 추가 설정값

`ACFVehiclePawn`에 아래 설정값이 추가되었다.

```text
- bEnableVehicleNetDebug
- VehicleNetDebugSampleIntervalSec
- VehicleNetDebugClientLogIntervalSec
- VehicleNetDebugServerSample
```

현재 기본값:

```text
bEnableVehicleNetDebug = true
VehicleNetDebugSampleIntervalSec = 0.25
VehicleNetDebugClientLogIntervalSec = 0.5
```

판단:

```text
테스트 세션에서는 바로 로그가 나와 편하다.
다만 장기적으로는 기본값 true가 로그 노이즈를 만들 수 있으므로, 진단 종료 후 기본 false 전환을 검토한다.
```

---

### 3.3 복제 등록

`CFVehiclePawn.cpp`에 아래가 추가되었다.

```text
#include "Net/UnrealNetwork.h"
DOREPLIFETIME(ACFVehiclePawn, VehicleNetDebugServerSample)
```

판단:

```text
서버 샘플을 클라이언트에 전달하는 최소 복제 경로가 생겼다.
```

---

### 3.4 Tick 흐름

`ACFVehiclePawn::Tick`에서 아래 호출이 추가되었다.

```text
UpdateVehicleNetDebug(DeltaSeconds)
```

현재 호출 위치:

```text
Super::Tick 이후, bVehicleRuntimeReady 검사 이전
```

판단:

```text
초기화 이전 상태도 샘플링될 수 있다.
치명적 문제는 아니지만, 로그 해석 시 초기 몇 줄은 무시할 수 있다.
필요하면 후속 패치에서 bVehicleRuntimeReady 이후로 이동한다.
```

---

### 3.5 로그 출력

클라이언트 로그는 아래 값을 출력한다.

```text
- Pawn
- LocalRole
- RemoteRole
- RepMove
- Seq
- SampleAge
- LocErr
- RotErr
- VelErr
- SpeedErr
- ServerLoc
- LocalLoc
- ServerVel
- LocalVel
```

판단:

```text
bReplicateMovement true/false 비교와 위치/회전/속도 오차 확인에는 충분하다.
다만 원래 후보였던 NetMode, HasAuthority, IsLocallyControlled, bReplicates, OwnerController, AngularVelocity는 아직 없다.
```

---

## 4. 빌드 검증

### 4.1 Editor Target

```text
Target: CarFight_ReEditor
Platform: Win64
Configuration: Development
Result: Succeeded
```

빌드 로그 요약:

```text
Target is up to date
Result: Succeeded
```

---

### 4.2 Server Target

```text
Target: CarFight_ReServer
Platform: Win64
Configuration: Development
Result: Succeeded
```

빌드 로그 요약:

```text
Target is up to date
Result: Succeeded
```

---

## 5. 현재 판정

```text
CFVNetDbg 1차 진단 패치는 빌드 기준 통과.
Dedicated Server 2클라 런타임 로그 수집은 아직 필요.
```

이번 패치는 아래 원칙을 지켰다.

```text
- bReplicateMovement 정책을 확정하지 않음
- 차량 Transform 보정/보간/예측을 추가하지 않음
- 무기/발사 서버 권한 구조로 넘어가지 않음
- 차량 네트워크 물리 안정화 진단에만 집중
```

---

## 6. 런타임 테스트 절차

### 6.1 공통 준비

```text
- Dedicated Server 실행
- Client 1 실행 및 접속
- Client 2 실행 및 접속
- 두 클라이언트가 각자 자기 차량 소유 확인
- 로그에서 VehicleNetDebug 검색
```

---

### 6.2 테스트 A: bReplicateMovement=true

```text
1. BP_CFVehiclePawn의 Replicate Movement를 true로 설정
2. 저속 가속
3. 쓰로틀 강하게 입력해 차 앞부분 Pitch 변화 관찰
4. 급조향
5. 정지
6. Client 1 / Client 2 로그에서 LocErr, RotErr, VelErr 기록
```

관찰 포인트:

```text
- 흔들리는 순간 RotErr가 튀는가?
- 원격 차량 순간이동 시 LocErr가 주기적으로 튀는가?
- SampleAge가 과하게 커지는가?
```

---

### 6.3 테스트 B: bReplicateMovement=false

```text
1. BP_CFVehiclePawn의 Replicate Movement를 false로 설정
2. 동일한 주행 패턴 반복
3. 장시간 주행 후 LocErr 누적 여부 확인
4. 충돌 근접 통과 상황에서 양 클라이언트 화면 차이 확인
```

관찰 포인트:

```text
- 시각적 끊김이 줄어드는가?
- LocErr가 시간에 따라 계속 증가하는가?
- 충돌 판정이 갈라지는 시점의 LocErr는 얼마인가?
```

---

## 7. 후속 보강 후보

현재 패치가 런타임 로그 수집에 충분하지 않으면 아래 항목을 보강한다.

```text
- AngularVelocityDeg 추가
- NetMode 로그 추가
- HasAuthority 로그 추가
- IsLocallyControlled 로그 추가
- bReplicates 로그 추가
- OwnerController 이름 로그 추가
- 서버 자체 로그 추가
- bVehicleRuntimeReady 이후에만 진단 Tick 수행하도록 이동
- 진단 기본값 true -> false 전환
```

---

## 8. Changelog

### v0.1.0

```text
- CFVNetDbg Codex 적용 코드 검토 결과 기록
- Editor / Server Target 빌드 성공 기록
- 누락된 진단 항목과 런타임 테스트 절차 정리
```
