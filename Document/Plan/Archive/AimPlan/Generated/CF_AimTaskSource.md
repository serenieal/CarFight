# Task Source

- 문서 버전: v1.0
- 작성일: 2026-05-08
- 문서 상태: Generated
- 작업 유형: 신규
- source_folder_path: `Document/ProjectSSOT/Plan/AimPlan`

## Goal

- CarFight 프로젝트에 Aim 시스템의 1차 C++ 기반을 추가한다.
- 이번 작업은 전체 조준 시스템 완성이 아니다.
- 이번 작업의 목표는 다음 2개로 제한한다.
- ```text R1 — Aim 타입 정의 R2 — UCFVehicleAimComp 골격 추가 ```
- 이번 작업에서 Local Aim 계산, VehicleDebug Aim 카테고리, Fire RPC, 서버 검증, Reticle UI는 구현하지 않는다.
- ---
- CarFight 프로젝트에 Aim 시스템의 1차 C++ 기반을 추가한다.

이번 작업은 전체 조준 시스템 완성이 아니다.

이번 작업의 목표는 다음 2개로 제한한다.

```text
R1 — Aim 타입 정의
R2 — UCFVehicleAimComp 골격 추가
```

이번 작업에서 Local Aim 계산, VehicleDebug Aim 카테고리, Fire RPC, 서버 검증, Reticle UI는 구현하지 않는다.

---
- AimPlan 문서 묶음을 만들고, 기존 기능 / 미래 기능 / 멀티플레이 관계를 정리한다.
- C++ 타입을 먼저 정의해 이후 구현의 기준을 만든다.
- AimComp를 생성하고 Pawn에서 소유할 수 있게 한다.
- 소유 클라이언트 기준으로 즉시 반응하는 Aim State를 계산한다.
- Aim 상태를 눈으로 검증할 수 있게 한다.
- 발사 입력을 서버 요청으로 보낼 준비를 한다.
- 서버가 클라이언트 발사 요청을 검증한다.
- 서버에서 Trace를 실행해 더미 발사 결과를 만든다.
- 다른 클라이언트가 최소 조준 시각 상태를 볼 수 있게 한다.
- Owner Client에서 Reticle State를 볼 수 있게 한다.
- Aim 시스템을 미래 Weapon / Damage / Lock-On으로 넘길 준비를 한다.
- AimPlan 문서 묶음을 만들고, 기존 기능 / 미래 기능 / 멀티플레이 관계를 정리한다.


C++ 타입을 먼저 정의해 이후 구현의 기준을 만든다.


AimComp를 생성하고 Pawn에서 소유할 수 있게 한다.


소유 클라이언트 기준으로 즉시 반응하는 Aim State를 계산한다.


Aim 상태를 눈으로 검증할 수 있게 한다.


발사 입력을 서버 요청으로 보낼 준비를 한다.


서버가 클라이언트 발사 요청을 검증한다.


서버에서 Trace를 실행해 더미 발사 결과를 만든다.


다른 클라이언트가 최소 조준 시각 상태를 볼 수 있게 한다.


Owner Client에서 Reticle State를 볼 수 있게 한다.


Aim 시스템을 미래 Weapon / Damage / Lock-On으로 넘길 준비를 한다.
- Aim 시스템의 목적, 멀티플레이 원칙, 기능 명세, 구조 설계, 작업 순서, 검증 기준을 문서화한다.
- Aim 시스템의 C++ 타입 기준을 먼저 만든다.
- `UCFVehicleAimComp`를 추가하고 `ACFVehiclePawn`에 소유시킨다.
- Owner Client에서 즉시 반응하는 Local Aim State를 계산한다.
- Aim 상태를 VehicleDebug Panel에서 확인할 수 있게 한다.
- 발사 입력을 서버 요청으로 보낼 수 있는 최소 흐름을 만든다.
- 서버가 클라이언트의 Fire Request를 승인하거나 거부한다.
- 서버 Trace로 더미 발사 결과를 만든다.
- Owner Client가 Reticle State를 화면에서 볼 수 있게 한다.
- Aim 시스템을 Weapon / Damage / Lock-On / AI / Part Damage로 연결할 준비를 한다.
- Aim 시스템의 목적, 멀티플레이 원칙, 기능 명세, 구조 설계, 작업 순서, 검증 기준을 문서화한다.


Aim 시스템의 C++ 타입 기준을 먼저 만든다.


`UCFVehicleAimComp`를 추가하고 `ACFVehiclePawn`에 소유시킨다.


Owner Client에서 즉시 반응하는 Local Aim State를 계산한다.


Aim 상태를 VehicleDebug Panel에서 확인할 수 있게 한다.


발사 입력을 서버 요청으로 보낼 수 있는 최소 흐름을 만든다.


서버가 클라이언트의 Fire Request를 승인하거나 거부한다.


서버 Trace로 더미 발사 결과를 만든다.


다른 클라이언트가 최소 조준 시각 상태를 볼 수 있게 한다.


Owner Client가 Reticle State를 화면에서 볼 수 있게 한다.


Aim 시스템을 Weapon / Damage / Lock-On / AI / Part Damage로 연결할 준비를 한다.

## Current Decisions

- 로컬 플레이어의 조준 입력은 즉시 반응해야 한다.
- 로컬 클라이언트에서 즉시 처리할 항목은 다음과 같다.
- 카메라 회전
- 카메라 Aim Trace
- Local Aim Target
- Local Reticle State
- Local CanFire 예측
- 로컬 Muzzle Flash / Camera Shake 같은 즉시 피드백
- 서버가 최종 결정할 항목은 다음과 같다.
- 발사 가능 여부 최종 판정
- HitScan Trace 최종 결과
- Projectile Spawn
- Damage 적용
- Part Damage / Destruction 적용
- Kill / Score / 상태 이상 적용
- 클라이언트는 발사 결과를 선언하지 않는다. 클라이언트는 서버에 발사를 요청한다.
- ```text Client: 나는 이 시각에 이 방향으로 발사했다.
- Server: 그 요청이 유효한지 검증하고 결과를 확정한다. ```
- Aim State는 매 프레임 갱신되는 상태다.
- Fire Request는 발사 입력 순간에만 발생하는 이벤트다.
- 두 개념을 하나의 RPC로 묶지 않는다.
- ---
- ### 2.1 클라이언트는 조준감을 책임진다

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

## In Scope

- 1차 Aim 시스템은 다음을 포함한다.
- Local Aim Target 계산
- Local Aim Direction 계산
- Weapon Arc 판정
- Reticle State 계산
- Fire Request 데이터 구조
- Fire Result 데이터 구조
- Server Fire 검증 구조
- Replicated Aim Visual State 구조
- VehicleDebug Aim 표시 기준
- HitScan 더미 발사 검증 기준
- 1차 Aim 시스템은 다음을 포함한다.

- Local Aim Target 계산
- Local Aim Direction 계산
- Weapon Arc 판정
- Reticle State 계산
- Fire Request 데이터 구조
- Fire Result 데이터 구조
- Server Fire 검증 구조
- Replicated Aim Visual State 구조
- VehicleDebug Aim 표시 기준
- HitScan 더미 발사 검증 기준

## Out of Scope

- 이번 Codex 작업에서 하지 말아야 할 것:
- `ServerRequestFire()` 구현하지 말 것
- `InputAction_Fire` 생성하지 말 것
- Reticle UI 만들지 말 것
- VehicleDebug Aim 카테고리 만들지 말 것
- Local Aim 계산 로직을 완성하려고 하지 말 것
- Damage / Projectile / WeaponComp를 만들지 말 것
- BP 에셋을 수정하지 말 것
- PoliceCar 전용 하드코딩을 넣지 말 것
- ---
- 이번 Codex 작업에서 하지 말아야 할 것:

- `ServerRequestFire()` 구현하지 말 것
- `InputAction_Fire` 생성하지 말 것
- Reticle UI 만들지 말 것
- VehicleDebug Aim 카테고리 만들지 말 것
- Local Aim 계산 로직을 완성하려고 하지 말 것
- Damage / Projectile / WeaponComp를 만들지 말 것
- BP 에셋을 수정하지 말 것
- PoliceCar 전용 하드코딩을 넣지 말 것

---
- CameraComp 안에 무기 발사 판정을 넣지 않는다.
- Reticle Widget에서 Trace를 직접 수행하지 않는다.
- 클라이언트 Fire Result를 서버가 그대로 믿지 않는다.
- Aim Runtime State 전체를 모든 클라이언트에 매 프레임 복제하지 않는다.
- PoliceCar 전용 하드코딩을 AimComp에 넣지 않는다.
- Damage를 AimComp에서 직접 적용하지 않는다.
- - CameraComp 안에 무기 발사 판정을 넣지 않는다.
- Reticle Widget에서 Trace를 직접 수행하지 않는다.
- 클라이언트 Fire Result를 서버가 그대로 믿지 않는다.
- Aim Runtime State 전체를 모든 클라이언트에 매 프레임 복제하지 않는다.
- PoliceCar 전용 하드코딩을 AimComp에 넣지 않는다.
- Damage를 AimComp에서 직접 적용하지 않는다.

---
- 1차 Aim 시스템은 다음을 제외한다.
- 완성형 무기 시스템
- 완성형 데미지 시스템
- Projectile 예측
- Lock-On
- AI Combat 완성 구현
- 부품 파괴 / Geometry Collection 연동
- 대규모 네트워크 최적화
- 1차 Aim 시스템은 다음을 제외한다.

- 완성형 무기 시스템
- 완성형 데미지 시스템
- Projectile 예측
- Lock-On
- AI Combat 완성 구현
- 부품 파괴 / Geometry Collection 연동
- 대규모 네트워크 최적화

---

## Target Files

- ```text CFVehicleAimTypes.h ```
- ```text CFVehicleAimComp.h CFVehicleAimComp.cpp CFVehiclePawn.h CFVehiclePawn.cpp ```
- ```text CFVehiclePawn.h CFVehiclePawn.cpp UI/CFVehicleDebugPanelWidget.h UI/CFVehicleDebugPanelWidget.cpp ```
- ```text
CFVehicleAimTypes.h
```


```text
CFVehicleAimComp.h
CFVehicleAimComp.cpp
CFVehiclePawn.h
CFVehiclePawn.cpp
```


```text
CFVehiclePawn.h
CFVehiclePawn.cpp
UI/CFVehicleDebugPanelWidget.h
UI/CFVehicleDebugPanelWidget.cpp
```
- ```text UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h ```
- ```text UE/Source/CarFight_Re/Public/CFVehicleAimComp.h UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp UE/Source/CarFight_Re/Public/CFVehiclePawn.h UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp ```
- ```text UE/Source/CarFight_Re/Public/CFVehiclePawn.h UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp ```
- UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
- UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h
- UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
- UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
- ```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
```


```text
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```


```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```
- Document/ProjectSSOT/Plan/AimPlan/README.md
- Document/ProjectSSOT/Plan/AimPlan/CF_AimContext.md
- Document/ProjectSSOT/Plan/AimPlan/CF_AimNet.md
- Document/ProjectSSOT/Plan/AimPlan/CF_AimSpec.md
- Document/ProjectSSOT/Plan/AimPlan/CF_AimDesign.md
- Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md
- Document/ProjectSSOT/Plan/AimPlan/CF_AimVerify.md
- UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
- UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h
- UE/Source/CarFight_Re/Public/CFVehicleCameraData.h
- UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
- UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
- UE/Source/CarFight_Re/Public
- UE/Source/CarFight_Re/Private
- UE/Source/CarFight_Re/Public/UI
- UE/Source/CarFight_Re/Private/UI
- Document/ProjectSSOT/03_VisionAlign.md
- Document/ProjectSSOT/00_Handover.md
- Document/ProjectSSOT/01_Roadmap.md
- Document/ProjectSSOT/16_CPP_DecisionLog.md
- Document/ProjectSSOT/Plan/CameraPlan
- Document/ProjectSSOT/Plan/CameraDebugPlan
- Document/ProjectSSOT/Plan/VehicleDebugPlan
- Document/ProjectSSOT/Plan/DataPlan

## Task Candidates

### P1-T1. ```text README.md CF_AimContext.md CF_AimNet.md CF_AimSpec.md CF_AimDesign.md CF_AimTasks.md CF_AimVerify.md CF_AimRoadmap.md CF_AimCodexTask.md ```

목표:
- ```text README.md CF_AimContext.md CF_AimNet.md CF_AimSpec.md CF_AimDesign.md CF_AimTasks.md CF_AimVerify.md CF_AimRoadmap.md CF_AimCodexTask.md ```

대상:
- ```text CFVehicleAimTypes.h ```
- ```text CFVehicleAimComp.h CFVehicleAimComp.cpp CFVehiclePawn.h CFVehiclePawn.cpp ```
- ```text CFVehiclePawn.h CFVehiclePawn.cpp UI/CFVehicleDebugPanelWidget.h UI/CFVehicleDebugPanelWidget.cpp ```
- ```text
CFVehicleAimTypes.h
```


```text
CFVehicleAimComp.h
CFVehicleAimComp.cpp
CFVehiclePawn.h
CFVehiclePawn.cpp
```


```text
CFVehiclePawn.h
CFVehiclePawn.cpp
UI/CFVehicleDebugPanelWidget.h
UI/CFVehicleDebugPanelWidget.cpp
```
- ```text UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h ```
- ```text UE/Source/CarFight_Re/Public/CFVehicleAimComp.h UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp UE/Source/CarFight_Re/Public/CFVehiclePawn.h UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp ```
- ```text UE/Source/CarFight_Re/Public/CFVehiclePawn.h UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp ```
- UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h

Acceptance:
- '```text README.md CF_AimContext.md CF_AimNet.md CF_AimSpec.md CF_AimDesign.md CF_AimTasks.md CF_AimVerify.md CF_AimRoadmap.md CF_AimCodexTask.md ```' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- Aim의 프로젝트 내 위치가 문서화되어 있다.
- 멀티플레이 권한 원칙이 문서화되어 있다.

## Acceptance Criteria

- Aim의 프로젝트 내 위치가 문서화되어 있다.
- 멀티플레이 권한 원칙이 문서화되어 있다.
- 1차 구현 범위와 제외 범위가 명확하다.
- ---
- 모든 타입이 `BlueprintType` 또는 필요한 UE 타입으로 노출되어 있다.
- 변수 이름과 Tooltip이 직관적이다.
- 네트워크 전송 후보 필드는 `FVector_NetQuantize` 계열 사용을 검토했다.
- 빌드 성공
- `BP_CFVehiclePawn`에서 AimComp가 보인다.
- BeginPlay에서 AimComp가 CameraComp를 찾을 수 있다.
- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- 무기 조준각 밖이면 `OutOfArc`가 된다.
- Trace가 막히면 `Blocked`가 된다.
- 발사 가능 예측이면 `Ready`가 된다.
- VehicleDebug Panel에서 Aim 카테고리를 볼 수 있다.
- Local / Server / Replicated Aim 항목이 분리되어 보인다.
- Last Fire Request / Reject Reason 표시 자리가 있다.
- Client에서 발사 입력을 누르면 서버 RPC가 호출된다.
- 서버가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.
- 잘못된 조준 방향 요청은 거부된다.
- 조준각 밖 요청은 거부된다.
- 유효 요청은 승인된다.
- Reject Reason이 Debug에 표시된다.
- 서버 기준 Trace가 실행된다.
- 서버 HitLocation이 FireResult에 들어간다.
- Damage는 적용하지 않는다.
- Debug로 서버 Trace를 확인할 수 있다.
- 다른 클라이언트에서 무기 방향 또는 조준 방향 시각화를 확인할 수 있다.
- 전투 판정과 시각 복제 정보가 분리되어 있다.
- Ready / Blocked / OutOfArc 상태가 화면에서 구분된다.
- 서버 응답 대기 상태를 표시할 수 있다.
- 서버 거부 상태를 표시할 수 있다.
- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.
- - Aim의 프로젝트 내 위치가 문서화되어 있다.
- 멀티플레이 권한 원칙이 문서화되어 있다.
- 1차 구현 범위와 제외 범위가 명확하다.

---


- 모든 타입이 `BlueprintType` 또는 필요한 UE 타입으로 노출되어 있다.
- 변수 이름과 Tooltip이 직관적이다.
- 네트워크 전송 후보 필드는 `FVector_NetQuantize` 계열 사용을 검토했다.

---


- 빌드 성공
- `BP_CFVehiclePawn`에서 AimComp가 보인다.
- BeginPlay에서 AimComp가 CameraComp를 찾을 수 있다.

---


- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- 무기 조준각 밖이면 `OutOfArc`가 된다.
- Trace가 막히면 `Blocked`가 된다.
- 발사 가능 예측이면 `Ready`가 된다.

---


- VehicleDebug Panel에서 Aim 카테고리를 볼 수 있다.
- Local / Server / Replicated Aim 항목이 분리되어 보인다.
- Last Fire Request / Reject Reason 표시 자리가 있다.

---


- Client에서 발사 입력을 누르면 서버 RPC가 호출된다.
- 서버가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.

---


- 잘못된 조준 방향 요청은 거부된다.
- 조준각 밖 요청은 거부된다.
- 유효 요청은 승인된다.
- Reject Reason이 Debug에 표시된다.

---


- 서버 기준 Trace가 실행된다.
- 서버 HitLocation이 FireResult에 들어간다.
- Damage는 적용하지 않는다.
- Debug로 서버 Trace를 확인할 수 있다.

---


- 다른 클라이언트에서 무기 방향 또는 조준 방향 시각화를 확인할 수 있다.
- 전투 판정과 시각 복제 정보가 분리되어 있다.

---


- Ready / Blocked / OutOfArc 상태가 화면에서 구분된다.
- 서버 응답 대기 상태를 표시할 수 있다.
- 서버 거부 상태를 표시할 수 있다.

---


- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.

---
- Aim의 프로젝트 내 위치가 명확하다.
- Local / Server / Replicated Aim 분리가 명확하다.
- 프로젝트가 빌드된다.
- 타입명이 기존 Camera 타입과 충돌하지 않는다.
- 타입만 추가하고 기존 동작은 바꾸지 않는다.
- `BP_CFVehiclePawn`에 AimComp가 표시된다.
- BeginPlay 이후 AimComp가 CameraComp를 찾을 수 있다.
- 기존 주행 / 카메라 동작이 깨지지 않는다.
- Weapon Arc 안이면 `Ready` 후보가 된다.
- Weapon Arc 밖이면 `OutOfArc`가 된다.
- Aim Trace가 막히면 `Blocked`가 된다.
- VehicleDebug Panel에서 Aim 섹션을 선택할 수 있다.
- Local / Server / Replicated 값이 구분되어 보인다.
- LastFireRequestId / LastRejectReason 표시 자리가 있다.
- Client에서 Fire 입력 시 서버 RPC가 호출된다.
- Server가 FireRequestId를 읽는다.
- 조준각 밖 요청은 서버가 거부한다.
- 유효 요청은 서버가 승인한다.
- Reject Reason이 Owner Client Debug에 표시된다.
- FireResult에 ServerHitLocation이 들어간다.
- Damage는 아직 적용하지 않는다.
- 다른 클라이언트에서 조준 방향 또는 무기 방향 시각화가 가능하다.
- 전투 판정용 상태와 시각 복제 상태가 분리되어 있다.
- Ready / Blocked / OutOfArc가 화면에서 구분된다.
- Reticle UI가 직접 Trace를 하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.
- - Aim의 프로젝트 내 위치가 명확하다.
- Local / Server / Replicated Aim 분리가 명확하다.
- 1차 구현 범위와 제외 범위가 명확하다.


- 프로젝트가 빌드된다.
- 타입명이 기존 Camera 타입과 충돌하지 않는다.
- 타입만 추가하고 기존 동작은 바꾸지 않는다.

---


- `BP_CFVehiclePawn`에 AimComp가 표시된다.
- BeginPlay 이후 AimComp가 CameraComp를 찾을 수 있다.
- 기존 주행 / 카메라 동작이 깨지지 않는다.

---


- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- Weapon Arc 안이면 `Ready` 후보가 된다.
- Weapon Arc 밖이면 `OutOfArc`가 된다.
- Aim Trace가 막히면 `Blocked`가 된다.

---


- VehicleDebug Panel에서 Aim 섹션을 선택할 수 있다.
- Local / Server / Replicated 값이 구분되어 보인다.
- LastFireRequestId / LastRejectReason 표시 자리가 있다.

---


- Client에서 Fire 입력 시 서버 RPC가 호출된다.
- Server가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.

---


- 조준각 밖 요청은 서버가 거부한다.
- 유효 요청은 서버가 승인한다.
- Reject Reason이 Owner Client Debug에 표시된다.

---


- 서버 기준 Trace가 실행된다.
- FireResult에 ServerHitLocation이 들어간다.
- Damage는 아직 적용하지 않는다.

---


- 다른 클라이언트에서 조준 방향 또는 무기 방향 시각화가 가능하다.
- 전투 판정용 상태와 시각 복제 상태가 분리되어 있다.

---


- Ready / Blocked / OutOfArc가 화면에서 구분된다.
- Reticle UI가 직접 Trace를 하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.

---


- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.

---
- 1차 Aim Spec 완료 조건은 다음과 같다.
- Local Aim State 구조가 정의되어 있다.
- Server Aim State 구조가 정의되어 있다.
- Replicated Aim Visual State 구조가 정의되어 있다.
- Fire Request / Result 구조가 정의되어 있다.
- Reticle State가 정의되어 있다.
- 서버 검증 기준이 정의되어 있다.
- VehicleDebug Aim 표시 항목이 정의되어 있다.
- 1차 구현에서 제외할 범위가 명시되어 있다.
- 1차 Aim Spec 완료 조건은 다음과 같다.

- Local Aim State 구조가 정의되어 있다.
- Server Aim State 구조가 정의되어 있다.
- Replicated Aim Visual State 구조가 정의되어 있다.
- Fire Request / Result 구조가 정의되어 있다.
- Reticle State가 정의되어 있다.
- 서버 검증 기준이 정의되어 있다.
- VehicleDebug Aim 표시 항목이 정의되어 있다.
- 1차 구현에서 제외할 범위가 명시되어 있다.

## Verification

- Aim의 프로젝트 내 위치가 문서화되어 있다.
- 멀티플레이 권한 원칙이 문서화되어 있다.
- 1차 구현 범위와 제외 범위가 명확하다.
- ---
- 모든 타입이 `BlueprintType` 또는 필요한 UE 타입으로 노출되어 있다.
- 변수 이름과 Tooltip이 직관적이다.
- 네트워크 전송 후보 필드는 `FVector_NetQuantize` 계열 사용을 검토했다.
- 빌드 성공
- `BP_CFVehiclePawn`에서 AimComp가 보인다.
- BeginPlay에서 AimComp가 CameraComp를 찾을 수 있다.
- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- 무기 조준각 밖이면 `OutOfArc`가 된다.
- Trace가 막히면 `Blocked`가 된다.
- 발사 가능 예측이면 `Ready`가 된다.
- VehicleDebug Panel에서 Aim 카테고리를 볼 수 있다.
- Local / Server / Replicated Aim 항목이 분리되어 보인다.
- Last Fire Request / Reject Reason 표시 자리가 있다.
- Client에서 발사 입력을 누르면 서버 RPC가 호출된다.
- 서버가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.
- 잘못된 조준 방향 요청은 거부된다.
- 조준각 밖 요청은 거부된다.
- 유효 요청은 승인된다.
- Reject Reason이 Debug에 표시된다.
- 서버 기준 Trace가 실행된다.
- 서버 HitLocation이 FireResult에 들어간다.
- Damage는 적용하지 않는다.
- Debug로 서버 Trace를 확인할 수 있다.
- 다른 클라이언트에서 무기 방향 또는 조준 방향 시각화를 확인할 수 있다.
- 전투 판정과 시각 복제 정보가 분리되어 있다.
- Ready / Blocked / OutOfArc 상태가 화면에서 구분된다.
- 서버 응답 대기 상태를 표시할 수 있다.
- 서버 거부 상태를 표시할 수 있다.
- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.
- - Aim의 프로젝트 내 위치가 문서화되어 있다.
- 멀티플레이 권한 원칙이 문서화되어 있다.
- 1차 구현 범위와 제외 범위가 명확하다.

---


- 모든 타입이 `BlueprintType` 또는 필요한 UE 타입으로 노출되어 있다.
- 변수 이름과 Tooltip이 직관적이다.
- 네트워크 전송 후보 필드는 `FVector_NetQuantize` 계열 사용을 검토했다.

---


- 빌드 성공
- `BP_CFVehiclePawn`에서 AimComp가 보인다.
- BeginPlay에서 AimComp가 CameraComp를 찾을 수 있다.

---


- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- 무기 조준각 밖이면 `OutOfArc`가 된다.
- Trace가 막히면 `Blocked`가 된다.
- 발사 가능 예측이면 `Ready`가 된다.

---


- VehicleDebug Panel에서 Aim 카테고리를 볼 수 있다.
- Local / Server / Replicated Aim 항목이 분리되어 보인다.
- Last Fire Request / Reject Reason 표시 자리가 있다.

---


- Client에서 발사 입력을 누르면 서버 RPC가 호출된다.
- 서버가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.

---


- 잘못된 조준 방향 요청은 거부된다.
- 조준각 밖 요청은 거부된다.
- 유효 요청은 승인된다.
- Reject Reason이 Debug에 표시된다.

---


- 서버 기준 Trace가 실행된다.
- 서버 HitLocation이 FireResult에 들어간다.
- Damage는 적용하지 않는다.
- Debug로 서버 Trace를 확인할 수 있다.

---


- 다른 클라이언트에서 무기 방향 또는 조준 방향 시각화를 확인할 수 있다.
- 전투 판정과 시각 복제 정보가 분리되어 있다.

---


- Ready / Blocked / OutOfArc 상태가 화면에서 구분된다.
- 서버 응답 대기 상태를 표시할 수 있다.
- 서버 거부 상태를 표시할 수 있다.

---


- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.

---
- Aim의 프로젝트 내 위치가 명확하다.
- Local / Server / Replicated Aim 분리가 명확하다.
- 프로젝트가 빌드된다.
- 타입명이 기존 Camera 타입과 충돌하지 않는다.
- 타입만 추가하고 기존 동작은 바꾸지 않는다.
- `BP_CFVehiclePawn`에 AimComp가 표시된다.
- BeginPlay 이후 AimComp가 CameraComp를 찾을 수 있다.
- 기존 주행 / 카메라 동작이 깨지지 않는다.
- Weapon Arc 안이면 `Ready` 후보가 된다.
- Weapon Arc 밖이면 `OutOfArc`가 된다.
- Aim Trace가 막히면 `Blocked`가 된다.
- VehicleDebug Panel에서 Aim 섹션을 선택할 수 있다.
- Local / Server / Replicated 값이 구분되어 보인다.
- LastFireRequestId / LastRejectReason 표시 자리가 있다.
- Client에서 Fire 입력 시 서버 RPC가 호출된다.
- Server가 FireRequestId를 읽는다.
- 조준각 밖 요청은 서버가 거부한다.
- 유효 요청은 서버가 승인한다.
- Reject Reason이 Owner Client Debug에 표시된다.
- FireResult에 ServerHitLocation이 들어간다.
- Damage는 아직 적용하지 않는다.
- 다른 클라이언트에서 조준 방향 또는 무기 방향 시각화가 가능하다.
- 전투 판정용 상태와 시각 복제 상태가 분리되어 있다.
- Ready / Blocked / OutOfArc가 화면에서 구분된다.
- Reticle UI가 직접 Trace를 하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.
- - Aim의 프로젝트 내 위치가 명확하다.
- Local / Server / Replicated Aim 분리가 명확하다.
- 1차 구현 범위와 제외 범위가 명확하다.


- 프로젝트가 빌드된다.
- 타입명이 기존 Camera 타입과 충돌하지 않는다.
- 타입만 추가하고 기존 동작은 바꾸지 않는다.

---


- `BP_CFVehiclePawn`에 AimComp가 표시된다.
- BeginPlay 이후 AimComp가 CameraComp를 찾을 수 있다.
- 기존 주행 / 카메라 동작이 깨지지 않는다.

---


- 카메라 조준 방향에 따라 LocalAimTarget이 변한다.
- Weapon Arc 안이면 `Ready` 후보가 된다.
- Weapon Arc 밖이면 `OutOfArc`가 된다.
- Aim Trace가 막히면 `Blocked`가 된다.

---


- VehicleDebug Panel에서 Aim 섹션을 선택할 수 있다.
- Local / Server / Replicated 값이 구분되어 보인다.
- LastFireRequestId / LastRejectReason 표시 자리가 있다.

---


- Client에서 Fire 입력 시 서버 RPC가 호출된다.
- Server가 FireRequestId를 읽는다.
- Owner Client가 FireResult를 받는다.
- Debug에 LastFireRequestId가 표시된다.

---


- 조준각 밖 요청은 서버가 거부한다.
- 유효 요청은 서버가 승인한다.
- Reject Reason이 Owner Client Debug에 표시된다.

---


- 서버 기준 Trace가 실행된다.
- FireResult에 ServerHitLocation이 들어간다.
- Damage는 아직 적용하지 않는다.

---


- 다른 클라이언트에서 조준 방향 또는 무기 방향 시각화가 가능하다.
- 전투 판정용 상태와 시각 복제 상태가 분리되어 있다.

---


- Ready / Blocked / OutOfArc가 화면에서 구분된다.
- Reticle UI가 직접 Trace를 하지 않는다.
- Reticle UI는 AimComp 상태만 읽는다.

---


- AimComp가 완성형 WeaponComp 없이도 동작한다.
- WeaponComp가 생겼을 때 연결할 함수 경계가 명확하다.
- Damage와 Lock-On 확장 경로가 막히지 않는다.

---
- 1차 Aim Spec 완료 조건은 다음과 같다.
- Local Aim State 구조가 정의되어 있다.
- Server Aim State 구조가 정의되어 있다.
- Replicated Aim Visual State 구조가 정의되어 있다.
- Fire Request / Result 구조가 정의되어 있다.
- Reticle State가 정의되어 있다.
- 서버 검증 기준이 정의되어 있다.
- VehicleDebug Aim 표시 항목이 정의되어 있다.
- 1차 구현에서 제외할 범위가 명시되어 있다.
- 1차 Aim Spec 완료 조건은 다음과 같다.

- Local Aim State 구조가 정의되어 있다.
- Server Aim State 구조가 정의되어 있다.
- Replicated Aim Visual State 구조가 정의되어 있다.
- Fire Request / Result 구조가 정의되어 있다.
- Reticle State가 정의되어 있다.
- 서버 검증 기준이 정의되어 있다.
- VehicleDebug Aim 표시 항목이 정의되어 있다.
- 1차 구현에서 제외할 범위가 명시되어 있다.

## Unresolved

- 없음

## Source Inventory

- `Document/ProjectSSOT/Plan/AimPlan/CF_AimCodexTask.md` role=codex_task status=active priority=high signals=goal, forbidden
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimTasks.md` role=task status=active priority=high signals=goal, target_files, verification
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md` role=roadmap status=active priority=high signals=goal, target_files, verification
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimDesign.md` role=design status=active priority=medium signals=forbidden
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimSpec.md` role=design status=active priority=medium signals=scope, out_of_scope, verification
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimVerify.md` role=validation status=active priority=medium signals=none
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimContext.md` role=unknown status=active priority=low signals=none
- `Document/ProjectSSOT/Plan/AimPlan/CF_AimNet.md` role=unknown status=active priority=low signals=decision
- `Document/ProjectSSOT/Plan/AimPlan/README.md` role=unknown status=active priority=low signals=none

## Changelog

- v1.0
- plan.synthesize_task_source로 생성
