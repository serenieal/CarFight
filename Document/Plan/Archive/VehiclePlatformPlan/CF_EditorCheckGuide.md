# CarFight - 전투 플랫폼 에디터 확인 가이드

> 역할: 코드 작업 전에 사용자가 언리얼 에디터에서 직접 확인해야 할 항목을 단계별로 안내한다.
> 문서 버전: v0.16
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / User Editor Checklist / P0 DA Active

---

## 1. 목적

이 문서는 `CF_EditorAssetAudit.md`에서 남은 시각 확인과 PIE 확인을 에디터에서 수행하기 위한 실무 가이드다.

AssetDump 이후 BP / DataAsset / Reticle / Fire 입력의 기본 연결은 확인됐다.
이 문서는 자동 덤프로 확인된 StaticMesh Socket 목록을 에디터에서 시각 검증하고, PIE 동작을 확인하는 데 집중한다.

확인 목표:

```text
기준 BP가 맞는가?
기준 DataAsset이 맞는가?
Reticle과 Debug 위젯 연결이 맞는가?
P0 후보 메시의 하드포인트 Socket 후보가 차체 위치에 맞는가?
Socket 후보를 `HP_` 표준명으로 리네임한 뒤 통합 캡처 버튼으로 차량 DataAsset 위치 슬롯에 기록할 수 있는가?
```

---

## 2. 확인 전 원칙

1. `.uasset`를 수정하지 않고 확인만 한다.
2. 값을 바꾸게 되면 먼저 별도 변경 작업으로 분리한다.
3. Socket 확인 결과와 Transform 후보 수치는 이 문서의 기록 양식에 적는다.
4. 하드포인트 위치 슬롯과 장착 타입을 분리해서 기록한다.
5. 레티클 디자인 수정은 하지 않는다.
6. Transform 수치는 전 차량 공통값이 아니라 P0 대상 차량 DataAsset별 값으로 기록한다.
7. `DA_PoliceCar`는 더 이상 현재 작업에 사용하지 않으며, 과거 감사 참고로만 남긴다.
8. P0 기본 DataAsset은 `DA_TestSedan`이며, `DA_TestSUV`는 대조 테스트 후보로 둔다.
9. 하드포인트 위치 작성 방식은 `CF_HardpointAuthoring.md`의 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처를 따른다.
   캡처 실행은 바퀴 위치 캡처 버튼에 통합된 차량 레이아웃 캡처 버튼을 기준으로 한다.
10. Mesh Socket 이름은 `HP_Front_01`, `HP_Top_01`처럼 위치 슬롯 인스턴스만 표현하고, Fixed / Gimbal / Turret 같은 타입은 DA 장착 / 터렛 섹션에 기록한다.
11. 같은 위치 카테고리에 여러 무장이 달릴 수 있으므로 `Front`나 `Top`만으로 슬롯을 기록하지 않는다.
12. 하드포인트 소켓은 모든 차량에 필수로 존재하는 것이 아니라, 차량 DataAsset 설정에 따라 있을 수도 있고 없을 수도 있다.

---

## 3. Step 1. 기준 차량 BP 확인

열기:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles
  -> BP_CFVehiclePawn 열기
```

확인 위치:

```text
클래스 기본값(Class Defaults)
  또는 세부 정보(Details)
```

확인 항목:

| 항목 | 기대값 | 결과 |
|---|---|---|
| VehicleData | `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan` | MCP 실시간 확인 및 저장 완료, 현재 P0 기본 |
| AimReticleWidgetClass | `/Game/CarFight/UI/WBP_AimReticle` | AssetDump 확인 |
| InputAction_Fire | `/Game/CarFight/Input/IA_Fire` | AssetDump 확인 |
| VehicleAimComp 존재 | 있음 | AssetDump 확인 |
| VehicleDebug HUD 참조 | 실제 BP 기본값 확인 | AssetDump에서는 비어 있음 |
| VehicleDebug Panel 참조 | 실제 BP 기본값 확인 | AssetDump에서는 비어 있음 |

판정:

```text
위 항목이 기대값과 다르면 코드 작업을 시작하지 말고,
CF_EditorAssetAudit.md와 CF_CodeStartGate.md를 먼저 갱신한다.
```

---

## 4. Step 2. 기준 DataAsset 확인

열기:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/Data/Cars
  -> DA_TestSedan 열기
```

확인 항목:

| 그룹 | 확인 항목 | 결과 |
|---|---|---|
| VehicleVisualConfig | ChassisMesh가 `Mesh_TestSedan` 계열인지 | MCP / 에디터 확인 |
| VehicleVisualConfig | WheelMeshFL/FR/RL/RR가 세단용 휠인지 | MCP / 에디터 확인 |
| VehicleMovementConfig | `bUseMovementOverrides` 상태 | AssetDump 확인 |
| WheelVisualConfig | ExpectedWheelCount | AssetDump 확인 |
| VehicleReferenceConfig | FrontWheelClass / RearWheelClass | AssetDump 확인 |
| DriveStateConfig | 차량별 DriveState 값 존재 | AssetDump 확인 |

판정:

```text
DA_TestSedan이 UCFVehicleData 기반이고 세단 차체/휠을 참조하면,
현재 차량 데이터 적용 경로를 확인하는 P0 기본 기준으로 사용할 수 있다.
DA_TestSUV는 대조 테스트 후보로 유지한다.
```

### 4.1 P0 후보 DataAsset 확인

열기:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/Data/Cars
  -> DA_TestSedan 열기
  -> DA_TestSUV 열기
```

AssetDump 기준 확인값:

| 그룹 | `DA_TestSedan` | `DA_TestSUV` |
|---|---|---|
| ChassisMesh | `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan` | `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV` |
| WheelMeshFL/FR/RL/RR | `/Game/CarFight/Vehicles/TestSedan/Wheel_*` | `/Game/CarFight/Vehicles/TestSUV/Wheel_*` |
| WheelClass | 비어 있음 | `BP_Wheel_Front_C`, `BP_Wheel_Rear_C` |
| Movement / WheelVisual / DriveState Override | 꺼짐 | 켜짐 |

판정:

```text
DA_TestSUV는 현재 P0 후보 DataAsset으로 비교적 바로 확인 가능하다.
DA_TestSedan은 WheelClass와 Override 설정을 보강할지, 기본값 폴백 검증용으로 둘지 결정해야 한다.
```

---

## 5. Step 3. 차체 Mesh Socket 확인

열기:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/police_car/StaticMeshes/Combined
  -> Combined_Body 열기
```

확인 위치:

```text
스태틱 메시 에디터(Static Mesh Editor)
  -> 소켓(Socket) 목록 또는 소켓 관리자(Socket Manager)
```

주의:

AssetDump `bpdump -IncludeDetails=true` 기준으로 StaticMesh Socket 목록은 확인됐다.
에디터에서는 목록 자체보다 Socket 위치가 실제 차체에 맞는지 시각적으로 확인한다.
`Combined_Body`는 0 소켓이고, P0 후보 메시인 `Mesh_TestSedan` / `Mesh_TestSUV`에는 하드포인트 후보 Socket이 있다.

확인 항목:

| 항목 | 결과 |
|---|---|
| `Combined_Body` 기존 Socket 있음 / 없음 | 없음, 0 소켓 |
| `Mesh_TestSedan` 전방 위치 Socket 후보 | 감사 당시 `Front_01`, 표준명 `HP_Front_01` |
| `Mesh_TestSedan` 루프 위치 Socket 후보 | 감사 당시 `Top_01`, 표준명 `HP_Top_01` |
| `Mesh_TestSUV` 전방 위치 Socket 후보 | 감사 당시 `Front_01`, 표준명 `HP_Front_01` |
| `Mesh_TestSUV` 루프 위치 Socket 후보 | 감사 당시 `Top_01`, `Top_02`, 표준명 `HP_Top_01`, `HP_Top_02` |
| 전방 추가 위치 Socket 후보 | `HP_Front_02`는 현재 없음 |
| Socket 방향이 차량 전방과 맞는지 | Rotation은 `(0,0,0)`, 에디터 시각 확인 필요 |

판정:

```text
P0에서는 Mesh Socket을 런타임 원본으로 사용하지 않는다.
P0 하드포인트는 DataAsset 로컬 Transform 기준으로 시작한다.
BP SceneComponent는 배치 확인용 Preview로만 사용한다.
```

---

## 6. Step 4. 차량별 DataAsset Transform 후보 기록

하드포인트 위치 작성 방식은 `CF_HardpointAuthoring.md`의 4번 방식으로 확정한다.

Socket 또는 BP Preview를 사용해 위치를 잡을 수 있지만, 최종 수치는 눈대중이 아니라 차량 DataAsset의 LocalTransform 값으로 기록한다.

중요:

```text
여기서 기록하는 수치는 전 차량 공통 고정값이 아니다.
현재 첫 기록 대상은 `DA_TestSedan`, `DA_TestSUV`다.
위치 카테고리, 위치 슬롯 인스턴스, 장착 프로파일 역할은 공통 규격이지만, LocalLocation / LocalRotation 값은 차량별로 달라진다.
다른 차량을 추가할 때는 같은 LocationSlotId를 재사용하더라도 해당 차량 메쉬 기준으로 수치를 다시 잡는다.
DA_PoliceCar에는 최종 P0 수치를 기록하지 않는다.
현재 작업 수치는 DA_TestSedan 또는 DA_TestSUV에만 기록한다.
```

권장 기록 단위:

```text
기준:
  차량 로컬 좌표

수치 소유자:
  P0 대상 차량 DataAsset

위치:
  X, Y, Z

회전:
  Pitch, Yaw, Roll
```

차량 설정에 선언 가능한 위치 슬롯 후보:

| LocationCategory | LocationSlotId | SocketName 후보 | 위치 후보 | 회전 후보 | 결과 |
|---|---|---|---|---|---|
| `Front` | `Front_01` | `HP_Front_01` | 전방 범퍼 또는 본넷 중앙 | 차량 전방 | 미기록 |
| `Front` | `Front_02` | `HP_Front_02` | 전방 좌/우 또는 상단 보조점 | 차량 전방 또는 제한 짐벌 기준 | 확장 가능성 |
| `Top` | `Top_01` | `HP_Top_01` | 루프 중앙 | 차량 전방 기준 | 미기록 |
| `Top` | `Top_02` | `HP_Top_02` | 루프 보조 장착점 | 차량 전방 기준 | P1 후보 |
| `Back` | `Back_01` | `HP_Back_01` | 후방 또는 트렁크 상단 | 차량 후방 또는 장착 방향 기준 | P1 후보 |
| `LeftSide` | `LeftSide_01` | `HP_LeftSide_01` | 좌측 외부 장착점 | 차량 좌측 기준 | P1 후보 |
| `RightSide` | `RightSide_01` | `HP_RightSide_01` | 우측 외부 장착점 | 차량 우측 기준 | P1 후보 |

P0 장착 프로파일 후보:

| LocationSlotRef | LocationCategory | MountType | 호환용 MountProfileId 후보 | 위치 보정 | 결과 |
|---|---|---|---|---|---|
| `Front_01` | `Front` | `Fixed` | `FrontFixed_SmallOrMedium` | 필요 시 `MountLocalOffset`로 기록 | 미기록 |
| `Top_01` | `Top` | `Turret` | `RoofTurret_MediumOrLarge` | 필요 시 `MountLocalOffset`로 기록 | 미기록 |
| `Top_02` | `Top` | `Turret` | 추가 터렛 후보 | 필요 시 `MountLocalOffset`로 기록 | P1 후보 |

확장 가능성:

| LocationSlotRef | LocationCategory | MountType | 호환용 MountProfileId 후보 | 의미 |
|---|---|---|---|---|
| `Front_02` | `Front` | `Gimbal` | `FrontGimbal_Small` | 전방 하드포인트가 늘어날 수 있다는 가능성. P0 필수 아님 |

주의:

```text
BP SceneComponent로 위치를 잡더라도,
전투 규칙의 원본은 BP가 아니라 DataAsset / C++가 가져야 한다.
SceneComponent는 배치 확인용 Preview로만 취급한다.
StaticMesh Socket을 추가하더라도 P0 런타임 필수 조건으로 취급하지 않는다.
차량 설정에 선언하지 않은 하드포인트 소켓은 존재하지 않아도 정상이다.
Socket / Preview 이름은 위치만 표현한다.
다만 같은 위치 카테고리에 여러 슬롯이 있을 수 있으므로 `HP_Front_01`, `HP_Top_02`처럼 숫자 suffix로 실제 슬롯을 구분한다.
Fixed / Gimbal / Turret 타입은 DA 장착 / 터렛 섹션에서 LocationSlotRef와 함께 기록한다.
```

---

## 7. Step 5. PIE 최소 확인

실행:

```text
플레이(Play)
  -> 선택한 뷰포트(Selected Viewport) 또는 새 에디터 창(New Editor Window)
```

확인 항목:

| 항목 | 기대 결과 | 결과 |
|---|---|---|
| 차량 Spawn | 정상 Spawn | 미기록 |
| Reticle 표시 | `WBP_AimReticle` 표시 | 미기록 |
| Fire 입력 | 누르면 FireValidation / Reticle 상태 변화 | 미기록 |
| VehicleDebug HUD / Panel | 설정이 켜져 있으면 표시 | 미기록 |
| Runtime Summary | VehicleData Present 또는 동등한 표시 | 미기록 |

판정:

```text
PIE에서 기존 Aim / Reticle / Debug가 깨져 있으면,
전투 플랫폼 코드를 시작하기 전에 기존 연결 회귀부터 고친다.
```

---

## 8. 결과 기록 양식

에디터 확인 후 아래 형식으로 기록한다.

```text
확인 날짜:
확인자:

BP_CFVehiclePawn:
  VehicleData:
  AimReticleWidgetClass:
  InputAction_Fire:
  VehicleDebug HUD:
  VehicleDebug Panel:

레거시 감사 참고:
  DA_PoliceCar:
    역할:
      과거 CFVehicleData 적용 경로 확인용. 현재 작업에는 사용하지 않음
    P0 최종 수치 기록:
      하지 않음

P0_Sedan:
  DataAsset:
    /Game/CarFight/Vehicles/Data/Cars/DA_TestSedan.DA_TestSedan
  ChassisMesh:
    /Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan.Mesh_TestSedan
  WheelMeshFL:
    /Game/CarFight/Vehicles/TestSedan/Wheel_FL.Wheel_FL
  WheelMeshFR:
    /Game/CarFight/Vehicles/TestSedan/Wheel_FR.Wheel_FR
  WheelMeshRL:
    /Game/CarFight/Vehicles/TestSedan/Wheel_RL.Wheel_RL
  WheelMeshRR:
    /Game/CarFight/Vehicles/TestSedan/Wheel_RR.Wheel_RR
  FrontWheelClass:
    현재 null, 보강 여부 결정 필요
  RearWheelClass:
    현재 null, 보강 여부 결정 필요

P0_Sedan Mesh:
  Socket 있음:
  Transform 기준:
    DataAsset 로컬 Transform
  Transform 수치 소유자:
    DA_TestSedan
  LocationSlots:
    Front_01:
      LocationCategory:
        Front
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
    Front_02:
      LocationCategory:
        Front
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
        전방 하드포인트 증가 가능성. P0 필수 아님
    Top_01:
      LocationCategory:
        Top
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
    Top_02:
      LocationCategory:
        Top
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
        P1 또는 추가 터렛 후보
  MountProfiles:
    Front_01 + Fixed:
      MountProfileId 후보:
        FrontFixed_SmallOrMedium
      LocationSlotRef:
        Front_01
      MountType:
        Fixed
      MountLocalOffset:
      SizeLimit:
      Yaw / Pitch 제한:
    Top_01 + Turret:
      MountProfileId 후보:
        RoofTurret_MediumOrLarge
      LocationSlotRef:
        Top_01
      MountType:
        Turret
      MountLocalOffset:
      SizeLimit:
      Yaw / Pitch 제한:
      TurnRate / Stabilization:
  ExpansionCandidates:
    Front_02 + Gimbal:
      의미:
        전방 하드포인트가 늘어날 수 있다는 가능성
      P0 필수 여부:
        아니오

P0_SUV:
  DataAsset:
    /Game/CarFight/Vehicles/Data/Cars/DA_TestSUV.DA_TestSUV
  ChassisMesh:
    /Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV.Mesh_TestSUV
  WheelMeshFL:
    /Game/CarFight/Vehicles/TestSUV/Wheel_FL.Wheel_FL
  WheelMeshFR:
    /Game/CarFight/Vehicles/TestSUV/Wheel_FR.Wheel_FR
  WheelMeshRL:
    /Game/CarFight/Vehicles/TestSUV/Wheel_RL.Wheel_RL
  WheelMeshRR:
    /Game/CarFight/Vehicles/TestSUV/Wheel_RR.Wheel_RR
  FrontWheelClass:
    /Game/CarFight/Vehicles/BP_Wheel_Front.BP_Wheel_Front_C
  RearWheelClass:
    /Game/CarFight/Vehicles/BP_Wheel_Rear.BP_Wheel_Rear_C

P0_SUV Mesh:
  Socket 있음:
  Transform 기준:
    DataAsset 로컬 Transform
  Transform 수치 소유자:
    DA_TestSUV
  LocationSlots:
    Front_01:
      LocationCategory:
        Front
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
    Front_02:
      LocationCategory:
        Front
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
        전방 하드포인트 증가 가능성. P0 필수 아님
    Top_01:
      LocationCategory:
        Top
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
    Top_02:
      LocationCategory:
        Top
      SocketName:
      LocalLocation:
      LocalRotation:
      메모:
        P1 또는 추가 터렛 후보
  MountProfiles:
    Front_01 + Fixed:
      MountProfileId 후보:
        FrontFixed_SmallOrMedium
      LocationSlotRef:
        Front_01
      MountType:
        Fixed
      MountLocalOffset:
      SizeLimit:
      Yaw / Pitch 제한:
    Top_01 + Turret:
      MountProfileId 후보:
        RoofTurret_MediumOrLarge
      LocationSlotRef:
        Top_01
      MountType:
        Turret
      MountLocalOffset:
      SizeLimit:
      Yaw / Pitch 제한:
      TurnRate / Stabilization:
  ExpansionCandidates:
    Front_02 + Gimbal:
      의미:
        전방 하드포인트가 늘어날 수 있다는 가능성
      P0 필수 여부:
        아니오

PIE:
  차량 Spawn:
  Reticle:
  Fire 입력:
  VehicleDebug:

결론:
```

---

## 9. 다음 결정

에디터 확인 후 아래 중 하나로 결정한다.

| 결과 | 다음 작업 |
|---|---|
| Socket 있음 | `HP_` 표준명 확인 후 통합 캡처 버튼으로 DataAsset LocalTransform에 캡처 |
| Socket 없음 | BP Preview로 위치를 잡고 통합 캡처 버튼 또는 DataAsset 직접 기록 절차로 진행 |
| Reticle / Debug 연결 정상 | 하드포인트 상태 표시 확장 준비 |
| Reticle / Debug 연결 깨짐 | 전투 플랫폼 전 기존 UI 회귀 수정 |
| VehicleData 연결 다름 | 기준 DataAsset 문서 갱신 후 진행 |

---

## 10. Changelog

### v0.16

- `DA_PoliceCar` 폐기 결정과 `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 상태를 에디터 확인 순서에 반영했다.
- 사용자가 열어야 하는 기준 DataAsset을 `DA_PoliceCar`에서 `DA_TestSedan`으로 변경했다.

### v0.15

- 하드포인트 소켓은 차량 설정에 따라 있을 수도 있고 없을 수도 있음을 확인 원칙에 추가했다.
- `Front_02 + Gimbal`을 P0 필수 기록 대상이 아니라 전방 하드포인트 확장 가능성으로 낮췄다.
- 에디터 기록 양식에서 `Front_02`를 확장 후보로 표시했다.

### v0.14

- SocketName 정책을 `HP_` prefix 리네임 기준으로 확정해 에디터 확인 항목에 반영했다.
- 하드포인트 Transform 기록 방식을 바퀴 위치 캡처 버튼에 통합된 차량 레이아웃 캡처로 정리했다.
- AssetDump 당시 이름과 코드 착수 표준 SocketName을 분리해 기록했다.

### v0.13

- StaticMesh Socket 확인을 AssetDump 미지원 항목에서 `bpdump -IncludeDetails=true` 기반 확인 항목으로 갱신했다.
- `Mesh_TestSedan`, `Mesh_TestSUV`의 실제 하드포인트 후보 Socket을 Step 3에 반영했다.
- Socket이 있더라도 P0 런타임 원본은 DataAsset LocalTransform이라는 판정을 유지했다.

### v0.12

- `Front`, `Top`을 단일 위치 슬롯이 아니라 `LocationCategory`로 정리하고 실제 기록 단위를 `Front_01`, `Front_02`, `Top_01`, `Top_02`로 갱신했다.
- 같은 위치 카테고리에 여러 터렛 / 무장이 달릴 수 있음을 기록 양식과 후보 표에 반영했다.
- 차량별 Transform 기록 양식을 `LocationSlotId` 인스턴스 중심으로 바꿨다.

### v0.11

- 에디터 기록 양식을 `LocationSlots`와 `MountProfiles`로 분리했다.
- SocketName 후보는 `HP_Front`, `HP_Top`처럼 위치만 표현하고, Fixed / Gimbal / Turret 타입은 DA 장착 / 터렛 섹션에 기록하도록 정리했다.
- 기존 `FrontFixed`, `RoofTurret`, `FrontGimbal` 표기는 MountProfileId 후보로 낮췄다.

### v0.10

- 하드포인트 위치 작성 방식을 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처로 확정해 확인 원칙과 Step 4에 반영했다.

### v0.9

- 하드포인트 위치 작성 방식 기준 문서 `CF_HardpointAuthoring.md`를 연결했다.
- Socket 또는 BP Preview는 위치 작성 보조이고 최종 원본은 차량 DataAsset LocalTransform임을 Step 4에 반영했다.

### v0.8

- P0 후보 DataAsset을 `DA_TestSedan`, `DA_TestSUV`로 반영했다.
- 결과 기록 양식의 메시 경로를 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV`로 정정했다.
- Transform 수치 소유자를 `DA_TestSedan`, `DA_TestSUV`로 명시하고, 세단 WheelClass / Override 보강 필요를 기록했다.

### v0.7

- 임시 P0 메시 `Mesh_TestSedan`, `Mesh_TestSUV`의 경로를 기록했다.
- Transform 수치 소유 DataAsset은 아직 지정 필요로 남겼다.

### v0.6

- P0 대상 차형 기본안을 세단(Sedan) + SUV로 반영했다.
- Transform 기록 양식의 placeholder를 `P0_Sedan`, `P0_SUV`로 바꿨다.

### v0.5

- `DA_PoliceCar`를 P0 최종 수치 기록 대상에서 제외하고 임시 감사 기준으로 낮췄다.
- Transform 수치 기록 대상을 이후 지정될 P0 차량 1~2종의 DataAsset으로 변경했다.

### v0.4

- Transform 수치가 전 차량 공통 고정값이 아니라 차량별 DataAsset 값임을 명시했다.
- P0 첫 수치 기록 대상을 `DA_PoliceCar` 기준 후보값으로 좁혔다.
  이 판단은 v0.5에서 P0 대상 차량 1~2종 기준으로 정정됐다.

### v0.3

- 사용자 에디터 확인 결과로 `Combined_Body` Socket Manager가 0 소켓임을 기록했다.
- P0 기준을 DataAsset 로컬 Transform으로 고정했다.
- 다음 확인 대상을 Socket 존재 여부가 아니라 하드포인트별 Transform 수치 후보로 좁혔다.

### v0.2

- AssetDump 확인 결과를 반영해 BP / DataAsset / Reticle / Fire 입력 항목을 확인 완료로 표시했다.
- VehicleDebug 기본 참조가 AssetDump에서 비어 있음을 별도 확인 항목으로 분리했다.
- StaticMesh Socket은 AssetDump 미지원으로 에디터 확인 대상임을 명확히 했다.

### v0.1

- 코드 작업 전 사용자가 에디터에서 확인해야 할 항목을 단계별로 정리했다.
- 기준 BP, 기준 DataAsset, 차체 Mesh Socket, PIE 확인 항목을 분리했다.
- Socket이 없을 때 DataAsset Transform 기준으로 시작하는 대안을 기록했다.

---

## 11. Migration 메모

- 이 문서는 에디터 확인 가이드이며 코드 변경 지시서가 아니다.
- 확인 결과가 나오면 `CF_EditorAssetAudit.md`와 `CF_CodeStartGate.md`를 먼저 갱신한다.
- 하드포인트 Transform 기준은 DataAsset 로컬 Transform으로 시작한다.
- 좌표 수치가 기록되기 전까지 P0 발사 위치 기본값은 임시값으로 취급한다.
