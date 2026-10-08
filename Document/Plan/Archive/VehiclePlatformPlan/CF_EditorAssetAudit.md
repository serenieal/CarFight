# CarFight - 전투 플랫폼 에디터 자산 감사

> 역할: 코드 작업 전에 기준 BP, DataAsset, Reticle, Debug, Mesh 자산의 확인 상태를 기록한다.
> 문서 버전: v0.19
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Asset Audit / Initial Layout Capture Implemented / Combat Link Priority

---

## 1. 목적

이 문서는 `CF_CodeStartGate.md`의 에디터 확인 항목을 채우기 위한 감사 기록이다.

코드나 `.uasset`를 수정하지 않고, 아래 근거만 사용했다.

| 근거 | 의미 |
|---|---|
| 파일 시스템 확인 | 해당 `.uasset` 파일이 실제로 존재함 |
| C++ 소스 확인 | C++ 공개 슬롯과 적용 경로가 존재함 |
| `.uasset` 문자열 확인 | 바이너리 내부에 참조 경로 또는 이름 문자열이 보임 |
| AssetDump 확인 | UE 커맨드렛으로 자산 Details / References / Manifest를 추출함 |
| 사용자 에디터 확인 | 언리얼 에디터 화면에서 직접 확인한 결과 |

주의:

```text
AssetDump 확인은 BP / DataAsset / Widget 연결 근거로 사용한다.
StaticMesh Socket은 AssetDump `bpdump -IncludeDetails=true`와 에디터 확인을 함께 근거로 사용한다.
장착 위치 수치와 PIE 동작은 반드시 언리얼 에디터에서 최종 확인한다.
```

---

## 2. 확인 상태 요약

| 항목 | 상태 | 판단 |
|---|---:|---|
| 기준 차량 BP 파일 | 확인됨 | `BP_CFVehiclePawn.uasset` 존재 |
| 레거시 감사 DataAsset 파일 | 확인됨 | `DA_PoliceCar.uasset` 존재, 현재 작업에는 사용하지 않음 |
| P0 세단 DataAsset 파일 | 확인됨, 기본 테스트 기준 | `DA_TestSedan.uasset` 존재, 하드포인트 슬롯 저장 완료 |
| P0 SUV DataAsset 파일 | 확인됨, 대조 테스트 후보 | `DA_TestSUV.uasset` 존재, 하드포인트 슬롯 저장 완료 |
| DataAsset Native Class | 확인됨 | AssetDump 기준 `CFVehicleData` |
| Reticle WBP 파일 | 확인됨 | `WBP_AimReticle.uasset` 존재 |
| Reticle Native Parent | 확인됨 | AssetDump 기준 `CFAimReticleWidget` |
| VehicleDebug HUD / Panel 파일 | 확인됨 | 파일 존재, 단 BP 기본 참조는 최신 AssetDump에서 비어 있음 |
| Fire 입력 자산 | 확인됨 | AssetDump 기준 `IA_Fire` 참조 확인 |
| 차체 Mesh 파일 | 확인됨 | `Combined_Body.uasset` 존재 |
| P0 임시 세단 Mesh | 확인됨 | `Mesh_TestSedan.uasset` 존재 |
| P0 임시 SUV Mesh | 확인됨 | `Mesh_TestSUV.uasset` 존재 |
| 하드포인트 Socket | 작성 보조 후보 확인 | `Combined_Body`는 0 소켓, `Mesh_TestSedan` / `Mesh_TestSUV`에는 위치 슬롯 후보 소켓 있음 |
| 하드포인트 Transform 기준 | P0 기본안 확정 | DataAsset 로컬 Transform 기준, BP SceneComponent는 Preview 전용 |

자동 확인 원문:

```text
Document/Plan/VehiclePlatformPlan/CF_AssetDumpResult.md
Document/Plan/VehiclePlatformPlan/AssetDumpEvidence/
```

---

## 3. 기준 차량 BP

### 3.1 파일 존재

| 항목 | 경로 | 상태 |
|---|---|---:|
| 기준 차량 BP | `/Game/CarFight/Vehicles/BP_CFVehiclePawn` | 확인됨 |

로컬 파일:

```text
UE/Content/CarFight/Vehicles/BP_CFVehiclePawn.uasset
```

### 3.2 확인된 참조 후보

최신 AssetDump 기준으로 아래 참조가 확인됐다.

| 참조 | 판단 |
|---|---|
| `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan` | 현재 P0 기본 VehicleData |
| `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar` | 레거시 감사 VehicleData, 현재 작업 사용 안 함 |
| `/Game/CarFight/UI/WBP_AimReticle` | Aim Reticle 위젯 |
| `/Game/CarFight/Input/IA_Fire` | Fire 입력 |
| `/Game/CarFight/Input/IA_VehicleMove` | 이동 입력 후보 |
| `Wheel_Anchor_FL/FR/RL/RR` | 기존 휠 Anchor 컴포넌트 이름 후보 |
| `Wheel_Mesh_FL/FR/RL/RR` | 기존 휠 Mesh 컴포넌트 이름 후보 |
| `VehicleDriveComp`, `WheelSyncComp`, `VehicleCameraComp`, `VehicleAimComp` | 기준 C++ 컴포넌트 |

주의:

```text
최신 AssetDump에서 VehicleDebugHudRef / VehicleDebugPanelRef 클래스 기본값은 비어 있다.
VehicleDebug 파일 존재와 BP 연결 여부는 분리해서 본다.
```

에디터에서 확인할 항목:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles
  -> BP_CFVehiclePawn 열기

세부 정보(Details)
  -> VehicleData
  -> AimReticleWidgetClass
  -> InputAction_Fire
  -> VehicleDebug HUD / Panel 관련 위젯 참조
```

---

## 4. 레거시 감사 DataAsset

### 4.1 파일 존재

| 항목 | 경로 | 상태 |
|---|---|---:|
| 경찰차 DataAsset | `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar` | 확인됨, 레거시 감사 기준. 현재 작업에는 사용하지 않음 |

로컬 파일:

```text
UE/Content/CarFight/Vehicles/Data/Cars/DA_PoliceCar.uasset
```

### 4.2 확인된 Native Class

AssetDump 기준으로 아래가 확인됐다.

| 항목 | 값 |
|---|---|
| Native Class | `/Script/CarFight_Re.CFVehicleData` |
| Asset Class | `CFVehicleData` |
| Asset Family | `primary_data_asset` |

따라서 현재 코드 작업의 기준 DataAsset은 `UCFVehicleData` 기반으로 보는 것이 맞다.

### 4.3 확인된 자산 참조 후보

| 참조 | 판단 |
|---|---|
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body` | 차체 Mesh 후보 |
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_FL` | FL 휠 Mesh 후보 |
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_FR` | FR 휠 Mesh 후보 |
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_RL` | RL 휠 Mesh 후보 |
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Wheel_RR` | RR 휠 Mesh 후보 |
| `/Game/CarFight/Vehicles/BP_Wheel_Front` | 전륜 Wheel Class 후보 |
| `/Game/CarFight/Vehicles/BP_Wheel_Rear` | 후륜 Wheel Class 후보 |

AssetDump 추가 확인:

| 항목 | 값 |
|---|---|
| `WheelVisualConfig.ExpectedWheelCount` | `4` |
| `WheelVisualConfig.FrontWheelCountForSteering` | `2` |
| `VehicleMovementConfig.DifferentialType` | `RearWheelDrive` |
| `VehicleMovementConfig.FrontWheelMaxSteerAngle` | `35` |

에디터에서 확인할 항목:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/Data/Cars
  -> DA_PoliceCar는 현재 작업 기준으로 열지 않음

세부 정보(Details)
  -> VehicleVisualConfig
  -> VehicleMovementConfig
  -> WheelVisualConfig
  -> VehicleReferenceConfig
  -> DriveStateConfig
```

### 4.4 P0 대상 DataAsset 추가 확인

2026-06-23 AssetDump 기준으로 아래 DataAsset이 확인됐다.

| 항목 | `DA_TestSedan` | `DA_TestSUV` |
|---|---|---|
| 경로 | `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan` | `/Game/CarFight/Vehicles/Data/Cars/DA_TestSUV` |
| Native Class | `/Script/CarFight_Re.CFVehicleData` | `/Script/CarFight_Re.CFVehicleData` |
| ChassisMesh | `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan` | `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV` |
| WheelMesh | `/Game/CarFight/Vehicles/TestSedan/Wheel_*` | `/Game/CarFight/Vehicles/TestSUV/Wheel_*` |
| WheelClass | `null` / `null` | `BP_Wheel_Front_C` / `BP_Wheel_Rear_C` |
| Movement Override | `false` | `true` |
| WheelVisual Override | `false` | `true` |
| DriveState Override | `false` | `true` |

판단:

```text
P0 대상 DataAsset은 지정됐다.
DA_TestSedan은 현재 기본 테스트 기준이다.
BP_CFVehiclePawn.VehicleData는 DA_TestSedan으로 저장 완료됐다.
DA_TestSedan 기준 기본 주행 검증은 완료됐다.
DA_TestSUV는 대조 테스트 시 사용자가 수동으로 VehicleData를 전환해 사용한다.
주행감 추가 튜닝은 현재 핵심 작업이 아니며, 다음 초점은 하드포인트 / Fire / UI 연결이다.
```

---

## 5. Reticle 자산

### 5.1 파일 존재

| 항목 | 경로 | 상태 |
|---|---|---:|
| Aim Reticle WBP | `/Game/CarFight/UI/WBP_AimReticle` | 확인됨 |

로컬 파일:

```text
UE/Content/CarFight/UI/WBP_AimReticle.uasset
```

### 5.2 확인된 부모와 위젯 이름

AssetDump 기준으로 아래가 확인됐다.

| 항목 | 값 |
|---|---|
| Native Parent Class | `CFAimReticleWidget` |
| Text 위젯 후보 | `Text_ReticleState`, `Text_CanFire`, `Text_ReticleHint` |
| 기본 상태 | `CachedReticleState = Hidden`, `bCachedCanFire = false` |
| 자동 갱신 | `bAutoRefreshEveryTick = true` |

판단:

```text
Reticle은 이미 C++ 부모 위젯과 WBP 자식 연결 기준이 있다.
전투 플랫폼 작업에서는 Reticle 디자인보다 상태 입력 확장이 우선이다.
```

에디터에서 확인할 항목:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/UI
  -> WBP_AimReticle 열기

디자이너(Designer)
  -> Text_ReticleState
  -> Text_CanFire
  -> Text_ReticleHint

클래스 설정(Class Settings)
  -> 부모 클래스(Parent Class)
  -> CFAimReticleWidget 확인
```

---

## 6. 차체 Mesh와 하드포인트 기준

### 6.1 파일 존재

| 항목 | 경로 | 상태 |
|---|---|---:|
| 경찰차 차체 Mesh | `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body` | 확인됨 |
| P0 임시 세단 Mesh | `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan` | 확인됨 |
| P0 임시 SUV Mesh | `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV` | 확인됨 |

로컬 파일:

```text
UE/Content/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body.uasset
UE/Content/CarFight/Vehicles/TestSedan/Mesh_TestSedan.uasset
UE/Content/CarFight/Vehicles/TestSUV/Mesh_TestSUV.uasset
```

### 6.2 StaticMesh Socket 확인

2026-06-25 AssetDump `bpdump -IncludeDetails=true` 기준으로 아래가 확인됐다.

| 메시 | Socket 수 | 하드포인트 후보 |
|---|---:|---|
| `Combined_Body` | 0 | 없음 |
| `Mesh_TestSedan` | 6 | `Front_01`, `Top_01` |
| `Mesh_TestSUV` | 7 | `Front_01`, `Top_01`, `Top_02` |

위 이름은 AssetDump 당시 실제 이름이다.
이 소켓을 캡처 입력으로 계속 사용할 경우 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`로 리네임한다.
단, 하드포인트 소켓은 차량 설정에 따라 있을 수도 있고 없을 수도 있다.

상세 근거:

```text
Document/Plan/VehiclePlatformPlan/CF_MeshSocketAudit.md
Document/Plan/VehiclePlatformPlan/SocketAudit_20260625/
```

결론:

```text
Combined_Body에는 현재 기준 하드포인트 Socket이 없다.
단, Combined_Body는 임시 감사 자산이며 최종 P0 차량 메시가 아니다.
Mesh_TestSedan / Mesh_TestSUV에는 하드포인트 위치 작성 보조로 쓸 수 있는 Socket 후보가 있다.
P0 하드포인트 기준은 Socket 존재 여부와 무관하게 DataAsset 로컬 Transform으로 시작한다.
BP SceneComponent는 배치 확인용 Preview로만 사용한다.
Mesh Socket은 작성 보조이며 P0 런타임 원본에서 제외한다.
하드포인트 위치 작성 방식 비교와 P0 선택안은 `CF_HardpointAuthoring.md`를 따른다.
P0 선택 방식은 Socket / BP Preview로 위치를 잡고 최종 수치를 DataAsset에 캡처하는 4번 방식이다.
```

에디터에서 확인할 항목:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/TestSedan
  -> Mesh_TestSedan 열기

콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles/TestSUV
  -> Mesh_TestSUV 열기

스태틱 메시 에디터(Static Mesh Editor)
  -> 소켓 관리자(Socket Manager)
  -> 해당 차량 설정에서 사용하는 HP_* 하드포인트 소켓 위치가 차체에 맞는지 시각 확인
```

---

## 7. C++ 소스 확인

| 파일 | 확인된 항목 |
|---|---|
| `CFVehiclePawn.h` | `VehicleData`가 `TObjectPtr<UCFVehicleData>` 타입 |
| `CFVehiclePawn.h` | `AimReticleWidgetClass`, `InputAction_Fire`, `VehicleAimComp` 존재 |
| `CFVehiclePawn.cpp` | `ApplyVehicleDataConfig()`가 VehicleData를 읽음 |
| `CFVehiclePawn.cpp` | `BuildFireCommand()`, `ValidateFireCommand()`, `RunLocalDummyHitScan()`, `ApplyFireResult()` 로컬 Fire 흐름 존재 |
| `CFVehicleData.h` | `VehicleVisualConfig`, `VehicleMovementConfig`, `WheelVisualConfig`, `VehicleReferenceConfig`, `DriveStateConfig` 존재 |
| `CFVehicleAimTypes.h` | `OutOfArc`, `NoWeapon`, `Cooldown`, `Reloading`, `FireRejected` Reticle 상태 존재 |
| `CFAimReticleWidget.h` | Reticle 상태 표시용 C++ 부모 위젯 존재 |

판단:

```text
전투 플랫폼 데이터는 기존 UCFVehicleData에 작게 확장하는 방향이 현재 코드 흐름과 가장 덜 충돌한다.
하드포인트 Transform 기준은 DataAsset 로컬 Transform으로 시작하되, 실제 좌표 수치는 최종 지정 P0 차량 DataAsset별로 에디터에서 기록해야 한다.
`DA_PoliceCar`는 폐기됐으므로 최종 P0 좌표 수치 원본으로 쓰지 않는다.
```

---

## 8. 코드 착수 전 남은 에디터 확인

| 순서 | 확인 항목 | 현재 상태 | 결과에 따른 결정 |
|---:|---|---:|---|
| 1 | `BP_CFVehiclePawn.VehicleData`가 `DA_TestSedan`인지 | MCP 실시간 확인 및 저장 완료 | 현재 P0 기본 테스트 기준 |
| 2 | `BP_CFVehiclePawn.AimReticleWidgetClass`가 `WBP_AimReticle`인지 | AssetDump 확인 | 다르면 Reticle 연결 문서 갱신 |
| 3 | `DA_TestSedan` / `DA_TestSUV`가 P0 후보인지 | AssetDump 및 에디터 저장 확인 | 하드포인트 슬롯 저장 완료 |
| 4 | `DA_TestSUV` 대조 테스트 전환 방식 | 확정 | 별도 자동화 없이 수동 VehicleData 전환 |
| 5 | `Combined_Body`에 Socket이 있는지 | 확인됨, 없음 | P0는 DataAsset 로컬 Transform 기준 |
| 6 | 차체 전방 방향이 +X 기준인지 | `HP_Front_01` 확인 | Sedan / SUV 전방 하드포인트 후보 저장 완료 |
| 7 | 루프 장착 위치 후보가 있는지 | `HP_Top_01` / `HP_Top_02` 확인 | Sedan `HP_Top_01`, SUV `HP_Top_01` / `HP_Top_02` 저장 완료 |
| 8 | 같은 위치 카테고리에 복수 슬롯이 필요한지 | 후보 확인 | SUV `Top_02` 있음. `Front_02`는 아직 없음 |
| 9 | VehicleDebug HUD / Panel이 PIE에서 표시 가능한지 | 미확정 | BP 기본 참조가 비어 있으므로 별도 확인 |

---

## 9. 사용자 확인 가이드

에디터에서 직접 확인할 순서는 아래 문서를 따른다.

```text
Document/Plan/VehiclePlatformPlan/CF_EditorCheckGuide.md
```

이 가이드는 다음 항목을 단계별로 확인한다.

| 단계 | 확인 대상 |
|---:|---|
| 1 | `BP_CFVehiclePawn` 기준 연결 |
| 2 | `DA_PoliceCar` DataAsset 세부 설정은 과거 감사 참고로만 유지 |
| 3 | `DA_TestSedan`, `DA_TestSUV` P0 후보 DataAsset 설정 차이 |
| 4 | `Combined_Body` Socket 존재 여부 |
| 5 | DataAsset 하드포인트 Transform 후보 |
| 6 | PIE Reticle / Fire / VehicleDebug 회귀 |

---

## 10. 하드포인트 Transform 기본안

에디터 Socket 확인 이후 P0 기본안은 아래로 둔다.

| 방식 | 판단 |
|---|---|
| Mesh Socket 기준 | `Mesh_TestSedan` / `Mesh_TestSUV`에 후보가 있으나 작성 보조로만 사용 |
| BP SceneComponent 기준 | 시각적으로 배치하기 쉽지만 규칙 원본으로 쓰지 않음 |
| DataAsset Transform 기준 | P0 규칙 원본으로 사용 |

P0 확정 기본안:

```text
P0 기본안:
  DataAsset에 하드포인트 로컬 Transform을 둔다.
  BP SceneComponent는 배치 확인용 Preview로만 사용한다.
  Mesh Socket은 작성 보조로만 사용하고 P0 코드가 Socket 존재에 의존하지 않는다.
```

---

## 11. Changelog

### v0.19

- 주행감 추가 튜닝을 현재 핵심 작업에서 제외했다.
- 자산 감사 기준의 다음 초점을 하드포인트 / Fire / UI 연결로 맞췄다.

### v0.18

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 확정했다.

### v0.17

- `DA_PoliceCar` 폐기 결정과 `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 상태를 자산 감사 기준에 반영했다.
- P0 런타임 테스트 기본 DA를 `DA_TestSedan`으로 고정하고, `DA_TestSUV`는 대조 테스트 전환 항목으로 분리했다.

### v0.16

- 차량 레이아웃 통합 캡처 구현 후 `DA_TestSedan` / `DA_TestSUV` 하드포인트 슬롯 저장 완료 상태를 반영했다.
- 에디터 확인 잔여 항목에서 리네임 후 확인 필요 문구를 현재 `HP_*` 저장 상태로 갱신했다.

### v0.15

- AssetDump로 확인한 소켓 후보가 모든 차량의 필수 소켓은 아님을 명시했다.
- 리네임 기준을 캡처 입력으로 계속 사용할 하드포인트 소켓에 적용하는 규칙으로 좁혔다.
- 에디터 시각 확인 항목을 차량 설정에서 사용하는 `HP_*` 소켓 기준으로 갱신했다.

### v0.14

- AssetDump 당시 SocketName과 코드 착수 표준 SocketName을 분리했다.
- 하드포인트 소켓 리네임 대상을 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`로 기록했다.
- 에디터 시각 확인 항목을 리네임 후 표준 SocketName 기준으로 갱신했다.

### v0.13

- `bpdump -IncludeDetails=true` 기준 StaticMesh Socket 감사 결과를 반영했다.
- `Mesh_TestSedan`의 `Front_01`, `Top_01`과 `Mesh_TestSUV`의 `Front_01`, `Top_01`, `Top_02` 후보를 기록했다.
- Mesh Socket을 P0 런타임 원본이 아니라 DataAsset 캡처 작성 보조로 정리했다.

### v0.12

- 코드 착수 전 에디터 확인 항목에 같은 위치 카테고리의 복수 슬롯 필요 여부를 추가했다.
- `Front + Fixed`, `Top + Turret` 표현을 `Front_01 + Fixed`, `Top_01 + Turret` 기준으로 갱신했다.

### v0.11

- 코드 착수 전 에디터 확인 항목에서 루프 터렛 기준을 `LocationSlot=Top`과 `MountType=Turret` 분리 표현으로 갱신했다.

### v0.10

- 하드포인트 위치 작성 방식을 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처로 확정해 감사 문서 표현을 갱신했다.

### v0.9

- 하드포인트 위치 작성 방식 기준 문서 `CF_HardpointAuthoring.md`를 차체 Mesh / 하드포인트 기준 섹션에 연결했다.

### v0.8

- `DA_TestSedan`, `DA_TestSUV` AssetDump 확인 결과를 추가했다.
- P0 후보 메시 경로를 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV` 기준으로 정정했다.
- `DA_TestSedan`의 WheelClass / Override 보강 필요와 P0 테스트 VehicleData 전환 방식 미정을 분리했다.

### v0.7

- P0 임시 세단 / SUV 메시 `Mesh_TestSedan`, `Mesh_TestSUV` 파일 존재를 기록했다.
- 메시 확인과 차량 DataAsset 미정을 분리했다.

### v0.6

- `DA_PoliceCar`를 기준 차량 DataAsset에서 임시 감사 DataAsset으로 정정했다.
- P0 하드포인트 좌표 수치 원본을 최종 지정 세단 / SUV의 DataAsset으로 변경했다.

### v0.5

- 하드포인트 Transform 수치가 차량별 DataAsset 값임을 감사 문서에도 반영했다.
- P0 첫 수치 기록 대상이 기준 차량 `DA_PoliceCar`임을 다른 문서와 맞췄다.
  이 판단은 v0.6에서 P0 대상 차량 1~2종 기준으로 정정됐다.

### v0.4

- 사용자 에디터 확인 결과로 `Combined_Body` Socket Manager가 0 소켓임을 반영했다.
- 하드포인트 기준을 P0에서 DataAsset 로컬 Transform으로 고정했다.
- BP SceneComponent는 Preview 전용이고 Mesh Socket 기준은 P0에서 제외한다고 정리했다.

### v0.3

- AssetDump 최신 확인 결과를 반영했다.
- BP / DataAsset / Reticle / Fire 입력 확인 상태를 AssetDump 확인으로 올렸다.
- `Combined_Body` StaticMesh Details 미지원으로 Socket 확인이 남았음을 명확히 했다.
- VehicleDebug 파일 존재와 BP 기본 참조 상태를 분리했다.

### v0.2

- 사용자 에디터 확인 가이드 `CF_EditorCheckGuide.md` 연결 섹션을 추가했다.
- 남은 에디터 확인 항목을 실제 확인 단계로 이어지게 정리했다.

### v0.1

- 기준 차량 BP, 기준 DataAsset, Reticle WBP, Debug WBP, 경찰차 Mesh 파일 존재를 확인했다.
- `BP_CFVehiclePawn`과 `DA_PoliceCar`의 바이너리 문자열 참조 후보를 기록했다.
- `Combined_Body`에서 하드포인트 / Socket 후보 문자열이 확인되지 않았음을 기록했다.
- 코드 착수 전 남은 에디터 확인 항목을 분리했다.

---

## 12. Migration 메모

- 이 문서는 코드와 `.uasset`을 변경하지 않는다.
- 에디터 확인 후 경로 또는 연결이 다르면 이 문서를 먼저 갱신한다.
- 하드포인트 좌표 수치가 기록되기 전에는 P0 기본값을 최종값처럼 취급하지 않는다.
