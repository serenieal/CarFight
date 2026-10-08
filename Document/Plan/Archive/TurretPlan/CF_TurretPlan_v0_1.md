# CarFight - 차량 터렛 구현 계획

> 역할: 차량에 설치될 P0 터렛의 데이터, 런타임, 조준, 발사, UI, 검증 순서를 정의한다.
> 문서 버전: v0.5
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Documentation-Only Session / Combat Data Ownership Linked

---

## 1. 목적

이 문서는 차량에 설치될 터렛을 바로 만들기 위한 코드 지시서가 아니다.

목적은 현재 문서와 코드를 기준으로, 터렛 구현을 어떤 순서로 잘라야 안전한지 정리하는 것이다.

핵심 목표:

```text
Top_01 위치 슬롯에 대구경 저속 터렛포를 설치하고,
플레이어 조준점과 무기 실제 조준점이 다르게 움직이는 구조를 검증한다.
```

P0 터렛은 완성형 무기 시스템이 아니라 아래 감각을 검증하는 첫 단계다.

1. 차량은 이동과 장갑 방향을 관리한다.
2. 터렛은 목표 방향을 늦게 따라간다.
3. 빠른 차량은 느린 터렛 각속도 한계를 이용할 수 있다.
4. Reticle / Debug는 왜 지금 맞지 않거나 쏠 수 없는지 설명한다.

---

## 1.1 이번 세션 범위

이번 세션은 문서 작업만 수행한다.

명시적으로 하지 않는 작업:

| 항목 | 이번 세션 처리 |
|---|---|
| C++ 파일 수정 | 하지 않음 |
| Blueprint 생성 / 수정 | 하지 않음 |
| `.uasset` 편집 | 하지 않음 |
| 에디터 실행 / 자산 저장 | 하지 않음 |
| 빌드 실행 | 하지 않음 |
| 구현 승인 | 하지 않음 |

이번 세션에서 할 수 있는 작업:

1. 터렛을 별도 BP로 둘지, 차량 내부 컴포넌트로 둘지 구조를 문서화한다.
2. 차량 DataAsset의 `Top_01` 하드포인트와 터렛 장착 프로파일 관계를 문서화한다.
3. 이후 코드 작업자가 같은 방향으로 구현할 수 있도록 파일 후보, 책임 경계, 검증 기준을 정리한다.
4. 기존 `VehiclePlatformPlan`, `AimPlan`, `DataPlan`과 충돌하지 않게 문서 간 연결점을 정리한다.

---

## 2. 확인한 근거

이번 계획은 아래 파일을 기준으로 작성했다.

| 구분 | 확인 대상 | 핵심 내용 |
|---|---|---|
| UE C++ 운영 규칙 | `Document/SSOT/UE_SSOT/UE_CPP_SSOT/UE_AI_Cpp_SSOT_v0_1.md` | 핵심 규칙 / 상태 / 계산은 C++, BP는 얇은 조립 계층 |
| BP 전환 판단 | `UE_BP2Cpp_Check_v0_1.md`, `UE_BP_Patterns_v0_1.md` | 터렛 회전과 Fire 검증은 BP 비대화 위험이 커서 C++ 우선 |
| 차량 플랫폼 계획 | `Document/Plan/VehiclePlatformPlan/README.md` | P0는 차량 증식보다 최소 전투 플랫폼 검증 |
| 하드포인트 계약 | `CF_HardpointMatrix.md`, `CF_HardpointAuthoring.md` | `Front_01 + Fixed`, `Top_01 + Turret` 우선 |
| 전투 계약 | `CF_CombatContract.md` | DataAsset은 저장소, C++는 해석자, BP는 조립 계층 |
| 전투 데이터 소유권 | `CF_CombatDataOwnership.md` | 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage / UI 상태의 소유권 분리 |
| 코드 착수 게이트 | `CF_CodeStartGate.md`, `CF_PlatformChecklist.md` | 코드 착수 전 남은 항목과 보류 항목 구분 |
| 조준 상태 | `CF_AimStatus.md`, `CF_AimSpec.md`, `CF_AimDesign.md` | Aim Core / Reticle / Local Fire 검증 / Dummy HitScan 완료 |
| 현재 구현 문서 | `Document/Systems/Vehicles/VehicleAim.md` | 현재 Aim은 로컬 표시 / 검증 / 시각화 중간 계층 |
| 차량 데이터 문서 | `Document/Systems/Vehicles/VehicleData.md`, `VehicleRuntime.md` | `UCFVehicleData`가 현재 차량 런타임 구성 루트 |
| 전투 SSOT | `CombatPlan/05_AimingSystem.md`, `06_WeaponTypes.md`, `12_Hardpoints.md` | 터렛은 자유롭지만 느리고 안정화가 필요해야 함 |
| 현재 코드 | `CFVehicleData.h/.cpp`, `CFVehiclePawn.h/.cpp`, `CFVDAValidator.h/.cpp` | 하드포인트 위치 슬롯, 선택 캡처, Validator 연결이 코드에 존재 |
| 최신 플랫폼 문서 | `CF_CodeStartGate.md`, `CF_SocketCaptureCodePlan.md`, `CF_AssetDumpResult.md` | `DA_TestSedan` 기본, `DA_TestSUV` 대조, 하드포인트 Transform 저장 완료 상태로 정리됨 |
| 구형 상태 문서 | `Document/Systems/Vehicles/VehicleData.md` | 아직 `DA_PoliceCar`를 대표 DataAsset처럼 설명하므로 터렛 계획의 최신 근거로 쓰지 않음 |

---

## 3. 현재 확인된 상태

### 3.1 이미 있는 것

현재 코드 기준으로 아래는 이미 들어와 있다.

| 항목 | 상태 | 근거 |
|---|---:|---|
| 차량 데이터 루트 | 있음 | `UCFVehicleData` |
| 하드포인트 위치 슬롯 구조 | 있음 | `FCFVehicleHardpointSlot` |
| 위치 슬롯 배열 | 있음 | `UCFVehicleData::HardpointSlots` |
| DataAsset 소켓 캡처 버튼 | 있음 | `CaptureLayoutFromChassisSockets()` |
| Pawn 소켓 캡처 버튼 | 있음 | `CaptureWheelLayoutFromBodySockets()` |
| 하드포인트 선택 캡처 정책 | 있음 | `SocketName`이 비어 있으면 유지, 누락은 경고 |
| 하드포인트 슬롯 Validator | 있음 | `ValidateHardpointSlots()` 구현 및 `ValidateVehicleData()` 연결 |
| Aim Core | 있음 | `UCFVehicleAimComp` |
| Reticle C++ 부모 | 있음 | `UCFAimReticleWidget` |
| Fire 입력 경로 | 있음 | `HandleFireStarted()` |
| 로컬 Fire 검증 | 있음 | `ValidateFireCommand()` |
| Dummy HitScan | 있음 | `RunLocalDummyHitScan()` |
| VehicleDebug Aim 표시 | 있음 | `CFVehicleDebugPanelWidget` Aim 섹션 |
| P0 기본 DataAsset 문서 기준 | 있음 | 최신 계획 문서 기준 `DA_TestSedan` 기본, `DA_TestSUV` 수동 대조 |

### 3.2 아직 없는 것

터렛 구현에 필요한 아래 항목은 아직 없거나 불완전하다.

| 항목 | 상태 | 계획상 의미 |
|---|---:|---|
| 장착 프로파일 구조 | 없음 | `Top_01` 슬롯에 어떤 터렛이 달리는지 설명 필요 |
| 터렛 런타임 상태 | 없음 | 현재 Yaw / Pitch / 안정화 / 실제 조준점 필요 |
| 실제 무기 발사 위치 | 없음 | 현재 Fire `AimOrigin`은 차량 Actor 위치 기반 |
| 무기 실제 조준 방향 | 없음 | 현재 Fire `AimDirection`은 AimComp 기준 방향 |
| 터렛 각속도 추적 | 없음 | 느린 각속도 검증 불가 |
| 쿨다운 / 재장전 | 없음 | P0 1차에서는 Debug 값 또는 단순 시간으로 시작 |
| 탄약 / 열 | 없음 | P0 1차 비범위 |
| Damage 적용 | 없음 | P0 1차는 Dummy HitScan 유지 |
| Projectile | 없음 | P0 1차 비범위 |
| 터렛 손상 모듈 | 없음 | P0 후속 |

### 3.3 문서와 코드가 어긋난 부분

이번 점검 기준으로 아래 항목은 더 이상 선행 차단 항목이 아니다.

| 항목 | 현재 판단 |
|---|---|
| `CF_SocketCaptureCodePlan.md` 상태 | `Implemented / Build Verified / DA Seeded`로 갱신되어 있음 |
| `CFVDAValidator::ValidateHardpointSlots()` | `.cpp` 구현 있음 |
| `ValidateVehicleData()` 연결 | `ValidateHardpointSlots(TargetVehicleData)` 호출 확인 |
| 하드포인트 빈 배열 정책 | Info 처리, 오류 아님 |
| 하드포인트 소켓 누락 정책 | Warning 처리, 전체 캡처 실패 아님 |

아직 충돌 또는 확인 필요로 남길 항목:

| 항목 | 상태 | 반영 방식 |
|---|---|---|
| 기존 AssetDump 산출물 | 일부 오래됨 | `BP_CFVehiclePawn.VehicleData = DA_PoliceCar`, `Top_01` 비 HP prefix 같은 과거 기록은 최신 자산 대표값으로 쓰지 않음 |
| 최신 `.uasset` 내부값 | 이번 세션에서 미확인 | 문서 기준은 최신 `VehiclePlatformPlan`을 따르고, 코드 착수 전 필요 시 MCP 또는 AssetDump 재실행 |
| `Document/Systems/Vehicles/VehicleData.md` | 구형 설명 포함 | 터렛 계획에서는 참고 문서로만 두고 최신 기준은 `CF_CodeStartGate.md` / `CF_AssetDumpResult.md` 우선 |
| 터렛 / 무기 전용 자산 | 파일 시스템 검색 기준 없음 | `BP_CFTurret_Proto`와 WeaponComp는 신규 작업으로 둠 |

따라서 터렛 코드 착수 전 남은 핵심은 Validator 보강이 아니라 아래 두 가지다.

1. `Top_01` 슬롯을 소비하는 장착 프로파일 구조를 확정한다.
2. `UCFVehicleWeaponComp`가 기존 Aim / Fire 흐름에 어떻게 끼어드는지 최소 경로를 확정한다.

---

## 4. P0 터렛 구현 원칙

### 4.1 기본안

P0 기본안은 아래 하나로 고정한다.

```text
LocationSlotId:
  Top_01

LocationCategory:
  Top

MountType:
  Turret

MountProfileId:
  RoofTurret_MediumOrLarge

대표 무기:
  대구경 저속 터렛포
```

### 4.2 책임 분리

| 계층 | 소유할 것 | 소유하지 않을 것 |
|---|---|---|
| `UCFVehicleData` | 슬롯 위치, 장착 프로파일 값, 터렛 튜닝값 | 런타임 회전 상태, 발사 판정 실행 |
| 신규 Weapon/Turret C++ 계층 | 장착 해석, 현재 터렛 회전, 실제 발사 위치 / 방향 | 카메라 이동, UI 디자인 |
| `UCFVehicleAimComp` | 플레이어 조준점 해석, Reticle 상태 연결 | 터렛 데이터 저장소, Damage 적용 |
| `ACFVehiclePawn` | 입력 진입점, 컴포넌트 소유, 기존 Fire 경로 연결 | 터렛 회전 계산의 장기 소유 |
| BP | 터렛 메시, VFX, SFX, 테스트용 배치 | 발사 가능 여부 최종 판단, 터렛 회전 규칙 |
| UI / Debug | 상태 표시 | 판정 원본 |

### 4.3 금지선

P0 터렛에서 하지 않는다.

1. BP EventGraph에 터렛 회전 상태머신을 만들지 않는다.
2. `UCFVehicleCameraComp`에 무기 발사 판정을 넣지 않는다.
3. `UCFVehicleAimComp`에 터렛 장착 데이터 저장소 역할을 맡기지 않는다.
4. `Top_01` 소켓 존재를 런타임 필수 조건으로 삼지 않는다.
5. `DA_PoliceCar` 전용 하드코딩으로 시작하지 않는다.
6. 미사일 / 락온 / Utility까지 한 번에 구현하지 않는다.

### 4.4 터렛 BP 부착 방식 결정

P0 권장안:

```text
터렛은 별도 Actor Blueprint로 둔다.
다만 P0에서는 gameplay 원본이 아니라 "얇은 시각 표현 BP"로만 사용한다.
차량에 붙이는 실제 기준은 DataAsset의 Top_01 하드포인트 LocalTransform이다.
```

이 결정의 이유:

1. 차량 BP 내부에 터렛 메시를 직접 심으면 차종마다 복제 작업이 늘어난다.
2. 독립 터렛 Actor로 너무 일찍 키우면 소유권, Replication, Damage, 생명주기 범위가 한 번에 커진다.
3. 별도 BP를 시각 계층으로 두면 메시 / 포신 / 포구 기준점은 에디터에서 빠르게 바꿀 수 있다.
4. C++ `UCFVehicleWeaponComp`가 슬롯 해석과 실제 조준 / 발사 판정을 소유하면 BP 비대화를 막을 수 있다.

따라서 P0는 아래처럼 나눈다.

| 구분 | P0 결정 |
|---|---|
| 터렛 BP 존재 여부 | 별도 BP 사용 |
| BP 역할 | 메시, 포신 피벗, 포구 기준점, 디버그 표시 후보 |
| BP 비역할 | 발사 가능 여부 판단, 터렛 각속도 계산, Damage, 탄약 |
| 차량 부착 기준 | `HardpointSlots.Top_01`의 `LocalLocation`, `LocalRotation` |
| 런타임 소유자 | 차량 Pawn의 `UCFVehicleWeaponComp` |
| 에디터 프리뷰 | `BP_CFVehiclePawn`의 임시 Child Actor Component 허용 |
| 최종 런타임 | WeaponComp가 선택한 터렛 Visual Actor를 Spawn / Attach |

### 4.5 `BP_CFTurret_Proto` 구조 후보

파일 경로:

```text
UE/Content/CarFight/Vehicles/Weapons/BP_CFTurret_Proto.uasset
```

작업 유형:

```text
신규 / v1.0 예정
```

역할:

```text
P0 터렛의 시각 구조를 검증하는 Actor Blueprint다.
전투 판정과 조준 규칙은 계산하지 않는다.
```

권장 컴포넌트 트리:

```text
BP_CFTurret_Proto
  DefaultSceneRoot
    TurretYawRoot
      TurretBaseMesh
      BarrelPitchRoot
        BarrelMesh
        MuzzlePoint
      DebugAimPoint
```

컴포넌트 역할:

| 컴포넌트 | 역할 |
|---|---|
| `TurretYawRoot` | 터렛 좌우 회전 시각 기준 |
| `TurretBaseMesh` | 터렛 하부 메시 |
| `BarrelPitchRoot` | 포신 상하 회전 시각 기준 |
| `BarrelMesh` | 포신 메시 |
| `MuzzlePoint` | 포구 위치 확인 기준점 |
| `DebugAimPoint` | 실제 조준 방향 디버그 후보 |

Blueprint 변수 / 함수 툴팁 후보:

| 이름 | 종류 | 툴팁 |
|---|---|---|
| `TurretYawRoot` | Component | "터렛 좌우 회전의 시각 기준입니다. 실제 회전값은 C++ WeaponComp 상태를 따릅니다." |
| `BarrelPitchRoot` | Component | "포신 상하 회전의 시각 기준입니다. 발사 판정 원본이 아니라 표현용입니다." |
| `MuzzlePoint` | Component | "포구 위치 확인용 기준점입니다. P0 판정은 C++ FireOrigin 계산을 우선합니다." |
| `TurretBaseMesh` | Component | "터렛 하부 시각 메시입니다. 전투 규칙을 계산하지 않습니다." |
| `BarrelMesh` | Component | "포신 시각 메시입니다. Pitch 표현만 담당합니다." |
| `SetTurretVisualPose` | Function | "C++에서 계산한 터렛 Yaw/Pitch를 시각 컴포넌트에 적용합니다. 전투 판정은 하지 않습니다." |

주의:

1. `MuzzlePoint`는 P0에서 시각 확인 기준점이다.
2. 실제 FireOrigin 원본은 `UCFVehicleWeaponComp`가 만든 `FCFVehicleFireOrigin`이다.
3. 나중에 포구 소켓 / Niagara / 사운드가 필요해지면 `MuzzlePoint`를 표현 계층 기준으로 재사용한다.

### 4.6 차에 붙이는 데이터 흐름

P0 부착 기준은 아래 순서로 정한다.

```text
VehicleData
  -> HardpointSlots
    -> LocationSlotId = Top_01
      -> LocalLocation / LocalRotation
        -> WeaponComp가 월드 Transform으로 변환
          -> BP_CFTurret_Proto를 차량 차체에 Attach
```

런타임 원칙:

1. `SocketName`은 캡처 입력 또는 에디터 보조값이다.
2. 런타임 필수 원본은 `LocalLocation`, `LocalRotation`이다.
3. 부모 컴포넌트는 우선 차체 메시인 `SM_Body`로 둔다.
4. `SM_Body`를 찾지 못하면 차량 Root에 붙일 수 있지만, 이 경우 Debug 경고를 남긴다.
5. `Top_01` 슬롯이 없으면 터렛을 생성하지 않고 `NoHardpointSlot`에 해당하는 경고 상태로 둔다.

개념 흐름:

```text
HardpointLocalTransform = Top_01.LocalLocation + Top_01.LocalRotation
BodyWorldTransform = SM_Body 월드 Transform
MountWorldTransform = HardpointLocalTransform을 BodyWorldTransform 기준으로 변환한 결과
```

위 식은 구현 방향 설명이며, 실제 코드는 Unreal의 `FTransform` API로 처리한다.

### 4.7 부착 방식별 선택 기준

| 방식 | 장점 | 단점 | P0 판단 |
|---|---|---|---|
| 차량 BP 내부 StaticMeshComponent | 가장 빠른 육안 배치 | 차종마다 BP가 비대해지고 재사용성이 낮음 | 임시 실험만 허용 |
| 차량 BP의 Child Actor Component | 에디터 프리뷰가 쉽고 BP를 별도로 유지 가능 | 런타임 데이터 기반 교체가 약함 | 프리뷰용 허용 |
| WeaponComp Runtime Spawn + Attach | DataAsset 기반이고 차종 복제가 적음 | 초기 코드 골격이 필요함 | P0 최종 권장 |
| 완전 독립 Turret Actor | HP, Damage, Replication 확장에 유리 | P0 범위가 과해짐 | P1/P2 이후 |

P0 결론:

```text
에디터 프리뷰:
  Child Actor Component로 BP_CFTurret_Proto를 Top_01 위치에 임시 배치할 수 있다.

런타임 기준:
  UCFVehicleWeaponComp가 Top_01 슬롯을 해석해 BP_CFTurret_Proto를 Spawn / Attach한다.
```

### 4.8 에디터 프리뷰 절차

이 절차는 구현 전 배치 감각 확인용이다. 이번 문서 세션에서는 실제 BP를 만들지 않는다.

사용자 에디터 작업 후보:

1. 콘텐츠 브라우저(`Content Browser`)에서 `BP_CFTurret_Proto`를 만든다.
2. `BP_CFVehiclePawn`을 연다.
3. 컴포넌트(`Components`) 패널에서 추가(`Add`)를 누른다.
4. 자식 액터 컴포넌트(`Child Actor Component`)를 추가한다.
5. 이름을 `CAC_Turret_Top_01`로 둔다.
6. 세부 정보(`Details`)의 자식 액터 클래스(`Child Actor Class`)를 `BP_CFTurret_Proto`로 지정한다.
7. 부모를 `SM_Body` 아래로 둔다.
8. 상대 위치(`Relative Location`)와 상대 회전(`Relative Rotation`)을 `Top_01` 캡처값과 맞춘다.

중요:

1. 이 방식은 위치 확인용이다.
2. 최종 판정과 런타임 장착 선택은 `UCFVehicleWeaponComp`가 소유한다.
3. `BP_CFVehiclePawn` EventGraph에 터렛 회전 상태머신을 만들지 않는다.

### 4.9 런타임 부착 절차

코드 구현 단계에서는 아래 순서로 처리한다.

1. `ACFVehiclePawn::InitializeVehicleRuntime()`이 차량 데이터를 준비한다.
2. `UCFVehicleWeaponComp`가 Owner Pawn과 `UCFVehicleData`를 캐시한다.
3. `UCFVehicleWeaponComp`가 활성 장착 프로파일을 선택한다.
4. 선택된 프로파일의 `LocationSlotRef`로 `Top_01` 슬롯을 찾는다.
5. `Top_01`의 `LocalLocation`, `LocalRotation`으로 Mount Transform을 만든다.
6. 터렛 Visual Class 후보를 선택한다.
7. Visual Actor를 생성하고 `SM_Body` 또는 차량 Root에 Attach한다.
8. Tick 또는 명시 업데이트에서 `CurrentYawDeg`, `CurrentPitchDeg`를 BP 시각 컴포넌트에 전달한다.
9. Fire 시점에는 BP 위치가 아니라 WeaponComp의 `FCFVehicleFireOrigin`을 사용한다.

P0에서 필요한 상태 이름 후보:

| 이름 | 의미 |
|---|---|
| `CurrentYawDeg` | 현재 터렛 좌우 각도 |
| `CurrentPitchDeg` | 현재 포신 상하 각도 |
| `TargetYawDeg` | 플레이어 조준점 기준 목표 좌우 각도 |
| `TargetPitchDeg` | 플레이어 조준점 기준 목표 상하 각도 |
| `bTurretSettled` | 목표 각도에 충분히 도달했는지 |
| `FireOrigin` | 실제 발사 위치 / 방향 결과 |

### 4.10 검증 기준

에디터 프리뷰 체크포인트:

1. `BP_CFTurret_Proto`가 차량 지붕 `Top_01` 위치에 보인다.
2. 차량 회전 시 터렛 프리뷰가 차체와 같이 움직인다.
3. 상대 위치가 차체 중심이나 월드 원점으로 튀지 않는다.

런타임 체크포인트:

1. `Top_01` 슬롯이 있으면 터렛 Visual Actor가 생성된다.
2. `Top_01` 슬롯이 없으면 생성 실패가 조용히 묻히지 않고 Debug 상태로 보인다.
3. 차량 이동 중 터렛이 차체를 따라간다.
4. 카메라를 빠르게 돌리면 터렛 Yaw / Pitch가 늦게 따라온다.
5. Dummy HitScan 또는 Debug Trace가 차량 중심이 아니라 터렛 발사 원점에서 시작한다.
6. `HP_Top_01` 소켓이 없어도 DataAsset의 `LocalLocation`, `LocalRotation`이 있으면 P0 런타임은 유지된다.

---

## 5. 권장 코드 구조

### 5.1 신규 타입 파일 후보

파일 경로:

```text
UE/Source/CarFight_Re/Public/CFVehicleWeaponTypes.h
```

작업 유형:

```text
신규 / v1.0 예정
```

역할:

```text
하드포인트 장착 프로파일, 터렛 런타임 상태, 무기 발사 원점 계산 결과를 정의한다.
```

후보 타입:

| 타입 | 역할 |
|---|---|
| `ECFVehicleMountType` | `Fixed`, `Gimbal`, `Turret`, `Launcher`, `Utility` 구분 |
| `ECFVehicleWeaponSize` | `Small`, `Medium`, `Large` 구분 |
| `FCFVehicleMountProfile` | 위치 슬롯 참조, MountType, 발사각, 회전 속도, 중량 |
| `FCFVehicleTurretState` | 현재 Yaw / Pitch, 목표 Yaw / Pitch, 안정화 상태 |
| `FCFVehicleFireOrigin` | 월드 발사 위치, 월드 발사 방향, 슬롯 / 프로파일 ID |

### 5.2 신규 컴포넌트 후보

파일 경로:

```text
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
```

작업 유형:

```text
신규 / v1.0 예정
```

역할:

```text
차량 DataAsset의 하드포인트 슬롯과 장착 프로파일을 읽고,
현재 활성 무기의 실제 조준 방향과 발사 원점을 계산한다.
```

초기 책임:

1. Owner `ACFVehiclePawn`과 `UCFVehicleData` 참조 준비.
2. 활성 장착 프로파일 선택.
3. `LocationSlotRef`로 `HardpointSlots`에서 위치 슬롯 검색.
4. `Top_01 + Turret`의 목표 Yaw / Pitch 계산.
5. `YawTurnRateDegPerSec`, `PitchTurnRateDegPerSec`로 현재 터렛 방향 보간.
6. 발사 시점의 `FCFVehicleFireOrigin` 반환.
7. Debug에 활성 슬롯 / 실제 조준 방향 / 안정화 여부 제공.

후속 책임:

1. 고정 무기와 짐벌 무기 공통 처리.
2. 쿨다운 / 재장전 / 탄약 / 열.
3. Projectile 또는 Damage 계층으로 위임.

### 5.3 `ACFVehiclePawn` 연결

파일 경로:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

작업 유형:

```text
수정 / v2.x 예정
```

변경 방향:

1. `UCFVehicleWeaponComp`를 기본 컴포넌트로 추가한다.
2. `InitializeVehicleRuntime()`에서 WeaponComp 초기화를 호출한다.
3. 기존 `BuildFireCommand()` 시그니처는 유지한다.
4. `BuildFireCommand()` 내부에서 WeaponComp가 있으면 실제 발사 위치 / 방향을 우선 사용한다.
5. WeaponComp가 없거나 초기화되지 않았으면 현재 AimComp fallback 흐름을 유지한다.
6. `ValidateFireCommand()`에는 최소 거부 사유만 추가한다.
   - `NoWeapon`
   - `OutOfWeaponArc`
   - `AimBlocked`
   - `WeaponCooldown`

### 5.4 `UCFVehicleAimComp` 연결

파일 경로:

```text
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
```

작업 유형:

```text
수정 / v1.x 예정
```

변경 방향:

1. 현재 `DefaultAimProfile`은 fallback으로 유지한다.
2. WeaponComp가 제공하는 활성 장착 프로파일이 있으면 그 발사각을 우선 사용한다.
3. Local Aim의 `bLocalWithinWeaponArc`는 터렛 현재 방향이 아니라 장착 프로파일 제한각 기준으로 계산한다.
4. 터렛의 "아직 못 따라옴"은 별도 상태로 Debug / Reticle에 표시한다.
5. Reticle은 플레이어 조준점과 무기 실제 조준점 차이를 표시할 수 있는 입력만 받는다.

중요:

```text
AimComp는 조준 명령을 해석한다.
WeaponComp는 무기가 실제 어디를 향하는지 계산한다.
```

---

## 6. DataAsset 설계

### 6.1 현재 유지할 구조

현재 `FCFVehicleHardpointSlot`은 유지한다.

현재 필드:

| 필드 | 의미 |
|---|---|
| `LocationSlotId` | `Top_01` 같은 실제 위치 슬롯 |
| `LocationCategory` | `Top` 같은 분류 |
| `SocketName` | 선택 캡처 입력 |
| `LocalLocation` | 차체 기준 위치 |
| `LocalRotation` | 차체 기준 회전 |

### 6.2 추가할 장착 프로파일 후보

`UCFVehicleData` 또는 별도 전투 DataAsset 안에 아래 배열을 추가하는 방향을 검토한다.

P0 기본안은 `UCFVehicleData`에 작게 추가하는 것이다.

| 필드 | 타입 후보 | 의미 |
|---|---|---|
| `MountProfileId` | `FName` | `RoofTurret_MediumOrLarge` |
| `LocationSlotRef` | `FName` | `Top_01` |
| `MountType` | `ECFVehicleMountType` | `Turret` |
| `SizeLimit` | `ECFVehicleWeaponSize` | `Medium` 또는 `Large` |
| `MinYawDeg` | `float` | 좌측 제한각 |
| `MaxYawDeg` | `float` | 우측 제한각 |
| `MinPitchDeg` | `float` | 하향 제한각 |
| `MaxPitchDeg` | `float` | 상향 제한각 |
| `YawTurnRateDegPerSec` | `float` | 좌우 회전 속도 |
| `PitchTurnRateDegPerSec` | `float` | 상하 회전 속도 |
| `AimSettleTimeSeconds` | `float` | 조준 안정화 시간 |
| `MountWeightKg` | `float` | 장착 중량 |
| `bExposedModule` | `bool` | 터렛 손상 후보 여부 |

P0 임시 추천값:

| 값 | 추천 |
|---|---:|
| `MinYawDeg` | -160 |
| `MaxYawDeg` | 160 |
| `MinPitchDeg` | -8 |
| `MaxPitchDeg` | 25 |
| `YawTurnRateDegPerSec` | 35 |
| `PitchTurnRateDegPerSec` | 20 |
| `AimSettleTimeSeconds` | 0.35 |
| `MountWeightKg` | 350 |

수치는 밸런스 확정값이 아니라, "느리지만 따라오는 대구경 저속 터렛"을 확인하기 위한 P0 시작값이다.

---

## 7. 구현 순서

### Phase 0. 현재 상태 고정

목표:

```text
터렛 코드 전에 이미 들어온 하드포인트 기반 코드를 재작업하지 않고,
남은 범위를 장착 프로파일과 WeaponComp로 좁힌다.
```

작업:

1. `CFVDAValidator` 하드포인트 검사는 이미 구현 / 연결된 상태로 보고 재구현하지 않는다.
2. `CF_SocketCaptureCodePlan.md`는 `Implemented / Build Verified / DA Seeded` 상태를 최신 기준으로 본다.
3. 기존 AssetDump 산출물 중 `DA_PoliceCar` 또는 비 `HP_` 하드포인트 소켓 기록은 과거 증거로만 본다.
4. 실제 `.uasset` 내부 기본값이 필요한 코드 세션에서는 MCP 또는 AssetDump를 새로 실행한다.
5. 빌드 검증이 필요한 코드 세션에서는 `D:\Work\CarFight_git\Tools\BuildEditor.bat`를 사용한다.

완료 기준:

1. 터렛 계획에서 Validator / 캡처 구현을 선행 미해결 항목으로 다시 다루지 않는다.
2. 다음 코드 착수 범위가 `CFVehicleWeaponTypes.h`와 `UCFVehicleWeaponComp`로 좁혀진다.
3. 자산 기본값 확인은 문서 기준과 실시간 에디터 확인을 분리해 기록한다.

### Phase 1. P0 터렛 데이터 입력

목표:

```text
DA_TestSedan 또는 DA_TestSUV에서 Top_01 위치 슬롯과 터렛 장착 프로파일을 읽을 수 있게 한다.
```

사용자 에디터 작업:

1. 콘텐츠 브라우저(`Content Browser`)에서 기본 테스트 기준인 `DA_TestSedan`을 먼저 연다.
2. 세부 정보(`Details`)에서 `하드포인트 위치 슬롯 (HardpointSlots)`을 확인한다.
3. `Top_01` 슬롯이 있는지 확인한다.
4. `LocationCategory`가 `Top`인지 확인한다.
5. `SocketName`은 캡처를 쓸 경우 `HP_Top_01`인지 확인한다.
6. `LocalLocation`, `LocalRotation`이 0에 가깝지 않은지 확인한다.
7. SUV 지붕 높이 / 추가 상단 슬롯 비교가 필요하면 `DA_TestSUV`도 같은 기준으로 확인한다.
8. 값이 비어 있거나 오래된 경우에만 차체 소켓에서 차량 레이아웃 캡처(`Capture Vehicle Layout From Chassis Sockets`) 버튼을 실행한다.

Codex 코드 작업:

1. 장착 프로파일 구조를 추가한다.
2. `UCFVehicleData`에 P0 장착 프로파일 배열을 작게 추가한다.
3. `RoofTurret_MediumOrLarge` 프로파일이 `Top_01`을 참조하게 한다.

완료 기준:

1. `Top_01` 위치 슬롯을 찾을 수 있다.
2. `RoofTurret_MediumOrLarge`가 `Top_01`을 참조한다.
3. 슬롯이 없거나 프로파일이 없으면 명확한 Debug 경고가 나온다.

### Phase 2. WeaponComp 골격

목표:

```text
터렛 회전 전, 활성 장착 프로파일과 발사 원점을 계산할 수 있게 한다.
```

작업:

1. `UCFVehicleWeaponComp` 생성.
2. Owner Pawn / VehicleData 참조 캐시.
3. 활성 장착 프로파일 ID 기본값을 `RoofTurret_MediumOrLarge`로 둔다.
4. `ResolveActiveMountProfile()` 추가.
5. `ResolveHardpointSlot()` 추가.
6. `BuildFireOrigin()` 추가.
7. VehicleDebug에 최소 상태를 추가할 준비를 한다.

완료 기준:

1. PIE에서 `Top_01` 월드 위치를 Debug Draw 또는 로그로 확인할 수 있다.
2. Fire 입력 시 기존 차량 Actor 위치가 아니라 `Top_01` 기준 발사 원점 후보가 계산된다.
3. 아직 실제 터렛 회전은 하지 않아도 된다.

### Phase 3. 터렛 회전 상태

목표:

```text
플레이어 조준점과 터렛 실제 조준점이 분리되어 움직인다.
```

작업:

1. `FCFVehicleTurretState` 추가.
2. 목표 Yaw / Pitch를 차량 또는 장착 슬롯 기준으로 계산한다.
3. 목표 각도를 장착 프로파일 제한각으로 Clamp한다.
4. 현재 Yaw / Pitch를 TurnRate 기준으로 보간한다.
5. 목표 각도와 현재 각도 차이가 작고 `AimSettleTimeSeconds`를 만족하면 안정화로 본다.
6. 안정화 전에는 Reticle / Debug에서 "터렛 추적 중" 상태를 표시한다.

완료 기준:

1. 카메라를 빠르게 돌리면 터렛 실제 조준점이 늦게 따라온다.
2. 빠른 목표 또는 급격한 카메라 회전에 대해 즉시 Ready가 되지 않는다.
3. 제한각 밖에서는 `OutOfArc` 또는 동등 상태가 표시된다.

### Phase 4. Fire Command 전환

목표:

```text
발사 명령이 플레이어 조준 방향이 아니라 터렛 실제 발사 방향을 사용한다.
```

작업:

1. 기존 `BuildFireCommand()` 시그니처는 유지한다.
2. WeaponComp가 준비됐으면 `AimOrigin`은 `Top_01` 기반 발사 위치로 둔다.
3. `AimDirection`은 터렛 현재 방향으로 둔다.
4. 안정화 전 발사 정책을 선택한다.
   - 기본안: 발사는 가능하지만 정확도가 낮다는 표현은 P1로 미룸.
   - P0 안전안: 안정화 전에는 `FireRejected` 또는 `FirePending`으로 처리.
5. `ValidateFireCommand()`는 최소한 OutOfArc / AimBlocked / NoWeapon / WeaponCooldown을 구분한다.

완료 기준:

1. Debug에서 `AimOrigin`이 루프 터렛 위치에 가깝게 나온다.
2. Debug에서 `AimDirection`이 터렛 현재 방향으로 나온다.
3. Dummy HitScan 라인이 터렛 방향에서 시작한다.

### Phase 5. Reticle / Debug 표시

목표:

```text
플레이어가 "내가 조준한 곳"과 "터렛이 실제 보는 곳"을 구분할 수 있게 한다.
```

작업:

1. VehicleDebug Aim 또는 신규 Weapon 섹션에 아래 값을 표시한다.
   - ActiveMountProfileId
   - LocationSlotId
   - MountType
   - CurrentYawDeg
   - TargetYawDeg
   - bWithinArc
   - bTurretSettled
   - FireRejectReason
2. Reticle에는 기존 상태를 유지하되, 후속으로 무기 실제 조준점 표시를 추가한다.
3. UI는 판단하지 않고 C++ 상태만 표시한다.

완료 기준:

1. `Ready`, `OutOfArc`, `FirePending`, `FireRejected`의 이유가 Debug에서 설명된다.
2. 터렛이 아직 못 따라왔을 때 플레이어가 판정 이유를 이해할 수 있다.

### Phase 6. 터렛 메시 / BP 표현 연결

목표:

```text
P0 터렛 상태를 눈으로 확인할 수 있게 한다.
```

작업:

1. `BP_CFTurret_Proto`를 별도 Actor Blueprint로 만든다.
2. `BP_CFVehiclePawn`의 Child Actor Component 배치는 프리뷰 용도로만 쓴다.
3. 런타임 부착은 `UCFVehicleWeaponComp`가 `Top_01` 슬롯 기준으로 처리한다.
4. 기본안은 C++ 상태를 먼저 만들고, BP는 그 값을 따라 회전하는 얇은 표현 계층으로 둔다.
5. BP 변수 / 함수에는 툴팁을 붙인다.

BP 변수 툴팁 후보:

| 변수 | 툴팁 |
|---|---|
| `TurretMeshRef` | "터렛의 시각 메시 컴포넌트입니다. 실제 터렛 판정은 C++ WeaponComp 상태를 따릅니다." |
| `BarrelMeshRef` | "포신의 시각 메시 컴포넌트입니다. Pitch 표현에만 사용하고 발사 판정 원본으로 쓰지 않습니다." |
| `bShowTurretDebug` | "터렛 회전과 발사 원점 확인용 디버그 표시를 켤지 정합니다." |

완료 기준:

1. BP가 터렛 판정을 계산하지 않는다.
2. 터렛 시각 회전이 C++ 상태와 크게 어긋나지 않는다.
3. `BP_CFVehiclePawn`의 임시 프리뷰 배치를 제거해도 런타임 Spawn / Attach 경로가 유지된다.

---

## 8. 비범위

P0 터렛 1차에서는 아래를 하지 않는다.

| 비범위 | 이유 |
|---|---|
| 완성형 WeaponComp 전체 | 탄약 / 열 / 재장전까지 한 번에 넣으면 범위가 커짐 |
| Projectile | 먼저 실제 발사 원점 / 방향 검증 필요 |
| Damage 적용 | 하드포인트 조준 / Fire 경로 안정화 이후 |
| 터렛 파괴 / 모듈 손상 | Damage와 ModuleProfile 이후 |
| AI 터렛 운용 | 플레이어 터렛 감각 검증 이후 |
| 락온 / 런처 | 별도 루프 필요 |
| 차량 종류 추가 | P0 터렛 감각 검증 전 확장 금지 |

---

## 9. 검증 방법

### 9.1 빌드 검증

명령:

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

예상 결과:

1. `CFVehicleData`, `CFVehiclePawn`, `CFVDAValidator`, 신규 WeaponComp 관련 컴파일 오류가 없다.
2. UHT 오류가 없다.
3. 새 UPROPERTY / UFUNCTION이 에디터에 정상 노출된다.

### 9.2 에디터 검증

확인 위치:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/Data/Cars
  -> DA_TestSUV 또는 DA_TestSedan
```

체크포인트:

1. `HardpointSlots`에 `Top_01`이 있다.
2. `Top_01.SocketName`이 비어 있거나 `HP_Top_01`로 의도에 맞게 설정되어 있다.
3. 캡처 버튼 실행 후 `Top_01.LocalLocation`, `Top_01.LocalRotation`이 기록된다.
4. 누락된 하드포인트 소켓은 오류가 아니라 경고로 표시된다.

### 9.3 PIE 검증

체크포인트:

1. 차량이 기존처럼 정상 스폰된다.
2. 조준점이 기존처럼 표시된다.
3. 터렛 현재 조준 방향이 플레이어 조준 방향을 늦게 따라간다.
4. 제한각 밖에서 발사 시 `OutOfArc` 또는 동등한 거부 사유가 표시된다.
5. 발사 Debug Trace가 차량 중심이 아니라 터렛 발사 원점에서 시작한다.
6. 기존 주행 / 카메라 / WheelSync 기능이 깨지지 않는다.

---

## 10. 위험과 대응

| 위험 | 대응 |
|---|---|
| 기존 AimComp가 비대해짐 | 터렛 실제 회전 / 무기 상태는 WeaponComp에 둔다 |
| BP가 다시 규칙 원본이 됨 | BP는 메시 회전 표현만 담당한다 |
| Socket 누락으로 런타임 실패 | 런타임 원본은 DataAsset LocalTransform으로 유지한다 |
| Fire 시작 위치가 차량 중심에 남음 | Phase 4에서 WeaponComp FireOrigin을 우선 사용한다 |
| Reticle이 왜 안 쏘는지 설명 못 함 | Debug에 FireRejectReason과 터렛 안정화 상태를 먼저 노출한다 |
| 오래된 AssetDump와 최신 자산 문서가 섞임 | AssetDump는 생성 시점을 기록하고, 코드 착수 전 필요한 자산만 새로 덤프한다 |

---

## 11. 결정 사항

1. P0 첫 터렛은 `Top_01 + Turret`으로 시작한다.
2. 대표 프로파일은 `RoofTurret_MediumOrLarge`로 둔다.
3. 터렛 회전 / 안정화 / 실제 발사 방향은 C++가 계산한다.
4. DataAsset은 슬롯 위치와 장착 프로파일 값을 저장한다.
5. 터렛은 별도 Actor Blueprint인 `BP_CFTurret_Proto`로 두되, P0에서는 시각 표현 전용으로 제한한다.
6. 차량에 붙이는 기준은 `Top_01` 소켓이 아니라 DataAsset의 `Top_01` LocalTransform이다.
7. `BP_CFVehiclePawn`의 Child Actor Component는 에디터 프리뷰 용도로만 허용한다.
8. 런타임 터렛 부착은 `UCFVehicleWeaponComp`가 Spawn / Attach 방식으로 처리한다.
9. BP는 터렛 메시 표현과 에셋 연결만 담당한다.
10. 기존 `BuildFireCommand()` 시그니처는 유지하고 내부 입력값만 WeaponComp 우선으로 확장한다.
11. Damage / Projectile / 탄약 / 열은 P0 터렛 1차 이후로 미룬다.

---

## 12. 미결 사항

| 항목 | 현재 판단 |
|---|---|
| 장착 프로파일을 `UCFVehicleData`에 둘지 별도 Combat DataAsset으로 둘지 | `CF_CombatDataOwnership.md` 기준 P0는 `UCFVehicleData`에 작게 추가, 커지면 분리 |
| 터렛 Visual Class를 어디에 저장할지 | P0는 장착 프로파일 또는 임시 WeaponComp 기본값 중 하나로 시작, 장기적으로는 Weapon DataAsset 분리 검토 |
| `DA_TestSedan`을 먼저 쓸지 `DA_TestSUV`를 먼저 쓸지 | 현재 문서 기준 기본 실행은 `DA_TestSedan`, 루프 높이 / `Top_02` 비교는 `DA_TestSUV` |
| 터렛 안정화 전 발사를 허용할지 | P0 안전안은 거부 또는 Pending |
| 터렛 메시를 언제 붙일지 | FireOrigin / Debug 검증 후 `BP_CFTurret_Proto`를 얇게 연결 |
| 최신 `.uasset` 내부 기본값 | 이번 문서 세션에서는 미확인. 코드 / 에셋 세션에서 MCP 또는 AssetDump 재확인 필요 |
| `Document/Systems/Vehicles/VehicleData.md`의 `DA_PoliceCar` 설명 | 구형 구현 문서로 남아 있어 별도 갱신 필요 |

---

## 13. 다음 작업 추천

권장 순서:

1. `RoofTurret_MediumOrLarge` 장착 프로파일을 `UCFVehicleData` 안에 작게 둘지 최종 확정한다.
2. `CFVehicleWeaponTypes.h`와 `UCFVehicleWeaponComp`의 최소 책임을 코드 착수 전 문서에 고정한다.
3. 코드 / 에셋 세션으로 넘어가기 직전에 `BP_CFVehiclePawn`, `DA_TestSedan`, `DA_TestSUV`, `Mesh_TestSedan`, `Mesh_TestSUV`만 MCP 또는 AssetDump로 새로 확인한다.
4. `Top_01 + Turret` FireOrigin Debug부터 검증한다.
5. 터렛 회전 / 안정화 상태를 추가한다.
6. Reticle / VehicleDebug 상태 표시를 확장한다.
7. `Document/Systems/Vehicles/VehicleData.md`의 구형 `DA_PoliceCar` 설명을 최신 기준으로 별도 갱신한다.

---

## 14. Changelog

### v0.5

- `CF_CombatDataOwnership.md`를 터렛 계획의 전투 데이터 소유권 기준 문서로 연결했다.
- 장착 프로파일의 P0 저장 위치 판단을 새 소유권 문서 기준으로 갱신했다.

### v0.4

- 현재 C++ 기준 `ValidateHardpointSlots()` 구현과 `ValidateVehicleData()` 연결이 확인된 상태를 반영했다.
- `CF_SocketCaptureCodePlan.md`를 `Implemented / Build Verified / DA Seeded` 기준으로 보고, Validator / 캡처 작업을 선행 미해결 항목에서 제거했다.
- 최신 플랫폼 문서 기준 `DA_TestSedan` 기본, `DA_TestSUV` 대조 테스트 상태를 반영했다.
- 기존 AssetDump 산출물과 `VehicleData.md`의 `DA_PoliceCar` 설명은 구형 근거로 분리했다.
- 다음 작업 추천을 장착 프로파일 확정과 `UCFVehicleWeaponComp` 골격 작성 중심으로 재정렬했다.

### v0.3

- 터렛 BP를 별도 Actor Blueprint로 두되 P0에서는 시각 표현 전용으로 제한한다고 결정했다.
- 차량에 붙이는 기준을 `Top_01` 소켓이 아니라 DataAsset의 `Top_01` LocalTransform으로 명시했다.
- `BP_CFTurret_Proto` 컴포넌트 트리, 변수 / 함수 툴팁, 에디터 프리뷰 절차를 추가했다.
- Child Actor Component는 프리뷰용, `UCFVehicleWeaponComp` Runtime Spawn / Attach는 P0 최종 권장안으로 구분했다.
- 런타임 부착 절차와 터렛 BP 검증 체크포인트를 구체화했다.

### v0.2

- 이번 세션 범위를 문서 작업 전용으로 고정했다.
- C++ / Blueprint / `.uasset` / 에디터 실행 / 빌드 작업은 이번 세션에서 하지 않는다고 명시했다.
- 터렛 구조 판단과 구현 준비 문서화만 이번 세션 범위로 제한했다.

### v0.1

- 차량 터렛 구현 계획 문서를 신규 작성했다.
- 기존 VehiclePlatformPlan, AimPlan, CombatPlan, VehicleData / VehicleRuntime 현재 상태를 터렛 구현 관점으로 묶었다.
- 현재 코드에 이미 하드포인트 슬롯 / 선택 캡처가 일부 반영된 사실을 문서화했다.
- P0 첫 터렛 범위를 `Top_01 + Turret + RoofTurret_MediumOrLarge`로 제한했다.
- Damage / Projectile / 탄약 / 열 / AI Combat를 비범위로 분리했다.

---

## 15. Migration 메모

- 기존 `FrontFixed_SmallOrMedium`, `RoofTurret_MediumOrLarge` 같은 결합형 표기는 `MountProfileId` 호환명으로 유지한다.
- 실제 위치 참조는 `LocationSlotId` / `LocationSlotRef`인 `Top_01`을 사용한다.
- 기존 `HardpointSlots`가 빈 차량 DataAsset은 오류로 보지 않는다.
- 하드포인트 `SocketName`이 비어 있어도 기존 `LocalTransform`을 유지한다.
- 기존 Fire 함수 시그니처는 터렛 1차 구현에서 변경하지 않는다.
- 터렛 구현이 시작되면 `Document/Systems/Vehicles/VehicleAim.md`, `VehicleData.md`, 필요 시 신규 `VehicleWeapon.md`를 함께 갱신한다.
