# CarFight - 전투 플랫폼 AssetDump 확인 결과

> 역할: 전투 플랫폼 문서의 자산 근거를 AssetDump 결과로 고정한다.
> 문서 버전: v0.15
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / AssetDump Evidence / Superseded BP Link / Sedan Driving Verified

---

## 1. 목적

이 문서는 `CF_EditorAssetAudit.md`의 자동 자산 확인 근거다.

목표는 코드 작업 전 아래를 분리하는 것이다.

```text
AssetDump로 확인된 사실
AssetDump로 확인할 수 없는 사실
언리얼 에디터에서 직접 확인해야 하는 사실
```

주의:

```text
AssetDump는 자산 연결과 일부 Details 값을 확인하는 근거다.
StaticMesh Socket은 `bpdump -IncludeDetails=true`로 확인한다.
기존 `asset_details` 또는 details 비활성 `bpdump`만으로는 전체 Socket 목록을 확인할 수 없다.
장착 위치의 시각적 적합성과 PIE 동작은 계속 별도 확인이 필요하다.
```

---

## 2. 실행 요약

2026-06-22 과거 실행 기록:

처음 실행 시 `CarFight_Re`와 `AssetDump` 모듈 BuildId가 당시 UE 5.7.4 실행 BuildId와 달라 커맨드렛이 실패했다.

조치:

```text
D:\UE_5.7\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project=D:\Work\CarFight_git\UE\CarFight_Re.uproject -WaitMutex
```

결과:

| 항목 | 상태 |
|---|---:|
| 에디터 타깃 빌드 | 성공 |
| AssetDump 커맨드렛 실행 | 성공 |
| 소스 코드 수정 | 없음 |
| `.uasset` 수정 | 없음 |
| 빌드 산출물 생성 | 있음 |

공통 경고:

```text
BP_CFVehiclePawn, BP_Wheel_Front, BP_Wheel_Rear가 빈 엔진 버전으로 저장되었다는 경고가 있다.
자산은 로드되었지만, 필요하면 별도 에디터 저장 정리 작업으로 분리한다.
```

주의:

```text
위 D:\UE_5.7 빌드 기록은 과거 BP / 임시 자산 감사 실행 기록이다.
2026-06-23 DA_TestSedan / DA_TestSUV 확인은 아래 소스 엔진 경로로 실행했다.
```

2026-06-23 추가 실행:

```text
DA_TestSedan / DA_TestSUV 확인은 uproject EngineAssociation이 가리키는 소스 엔진으로 실행했다.
사용한 에디터:
  D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe

확인 대상:
  /Game/CarFight/Vehicles/Data/Cars/DA_TestSedan.DA_TestSedan
  /Game/CarFight/Vehicles/Data/Cars/DA_TestSUV.DA_TestSUV

결과:
  AssetDump 성공
  소스 코드 수정 없음
  .uasset 수정 없음
```

2026-06-25 StaticMesh Socket 추가 실행:

```text
Combined_Body / Mesh_TestSedan / Mesh_TestSUV 확인은 소스 엔진으로 실행했다.
사용한 에디터:
  D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe

실행 옵션:
  -Mode=bpdump
  -IncludeDetails=true
  -IncludeSummary=true
  -IncludeGraphs=false
  -IncludeReferences=false

결과:
  AssetDump JSON dump_status success
  소스 코드 수정 없음
  .uasset 수정 없음
```

---

## 3. 생성된 근거 파일

근거 파일 위치:

```text
Document/Plan/VehiclePlatformPlan/AssetDumpEvidence/
```

주요 파일:

| 파일 | 의미 |
|---|---|
| `asset_list_carfight.json` | `/Game/CarFight` 현재 자산 목록 |
| `BP_CFVehiclePawn/manifest.json` | 기준 차량 BP 덤프 메타 |
| `BP_CFVehiclePawn/details.json` | 기준 차량 BP Details |
| `BP_CFVehiclePawn/references.json` | 기준 차량 BP 참조 관계 |
| `DA_PoliceCar/details.json` | 과거 레거시 감사 기준 DataAsset 값 |
| `DA_TestSedan/details.json` | P0 세단 후보 DataAsset 값 |
| `DA_TestSUV/details.json` | P0 SUV 후보 DataAsset 값 |
| `WBP_AimReticle/details.json` | Reticle WBP 값 |
| `Combined_Body/details.json` | 과거 차체 StaticMesh 덤프 결과 |
| `SocketAudit_20260625/` | StaticMesh Socket 상세 덤프 결과 |

---

## 4. 확인된 기준 자산

| 자산 | AssetDump 상태 | 판단 |
|---|---:|---|
| `/Game/CarFight/Vehicles/BP_CFVehiclePawn` | 성공 | 기준 차량 Pawn BP |
| `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar` | 성공 | 과거 레거시 감사 기준 `CFVehicleData` DataAsset. 현재 작업 사용 안 함 |
| `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan` | 성공 | P0 세단 후보 `CFVehicleData` DataAsset |
| `/Game/CarFight/Vehicles/Data/Cars/DA_TestSUV` | 성공 | P0 SUV 후보 `CFVehicleData` DataAsset |
| `/Game/CarFight/UI/WBP_AimReticle` | 성공 | `CFAimReticleWidget` 자식 WBP |
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body` | 성공 | StaticMesh Socket 0개 |
| `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan` | 성공 | StaticMesh Socket 6개 |
| `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV` | 성공 | StaticMesh Socket 7개 |
| `/Game/CarFight/Input/IA_Fire` | 목록 확인 | Fire 입력 자산 존재 |

별도 `Weapon`, `Hardpoint`, `Turret`, `Cannon`, `Missile`, `Launcher`, `Gun`, `Muzzle`, `Socket` 이름의 전투 자산은 `/Game/CarFight` 자산 목록 검색에서 확인되지 않았다.

---

## 5. `BP_CFVehiclePawn` 확인 결과

AssetDump 기준:

| 항목 | 값 |
|---|---|
| Asset Class | `Blueprint` |
| Parent Class | `/Script/CarFight_Re.CFVehiclePawn` |
| Generated Class | `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn_C` |
| Dump Status | `success` |

핵심 연결:

| 항목 | 값 |
|---|---|
| `VehicleData` | 과거 AssetDump 당시 `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar.DA_PoliceCar`. 현재 저장 기준은 `DA_TestSedan` |
| `InputAction_Fire` | `/Game/CarFight/Input/IA_Fire.IA_Fire` |
| `AimReticleWidgetClass` | `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle_C` |
| `VehicleDriveComp` | `CFVehicleDriveComp` |
| `WheelSyncComp` | `CFWheelSyncComp` |
| `VehicleCameraComp` | `CFVehicleCameraComp` |
| `VehicleAimComp` | `CFVehicleAimComp` |

Aim / Fire 관련 확인:

| 항목 | 값 |
|---|---|
| `LastFireRequest` | `CFVehicleFireRequest` |
| `WeaponGroupId` 기본값 | `None` |
| `DefaultAimProfile` | Yaw `-20 ~ 20`, Pitch `-10 ~ 10`, Distance `10000` |
| `LocalReticleState` 기본값 | `Hidden` |
| `bLocalCanFire` 기본값 | `false` |
| `FireValidationState` | 존재 |
| `AimVisualState` | 존재 |

판단:

```text
기존 조준 / 로컬 발사 명령의 껍질은 있다.
하지만 하드포인트 목록, 무기 정의, 탄약, 쿨다운, 실제 피해 연결은 아직 없다.
```

Debug 관련 주의:

```text
최신 AssetDump에서 VehicleDebugHudRef / VehicleDebugPanelRef 클래스 기본값은 비어 있다.
파일 존재와 BP 연결 여부를 분리해서 확인해야 한다.
PIE에서 Debug 표시 가능 여부는 에디터 확인 대상으로 남긴다.
```

---

## 6. 과거 `DA_PoliceCar` 확인 결과

AssetDump 기준:

| 항목 | 값 |
|---|---|
| Asset Class | `CFVehicleData` |
| Asset Family | `primary_data_asset` |
| Parent Class | `/Script/Engine.PrimaryDataAsset` |
| Dump Status | `success` |

확인된 데이터 그룹:

| 그룹 | 확인된 값 |
|---|---|
| `VehicleVisualConfig.ChassisMesh` | `Combined_Body` |
| `VehicleVisualConfig.WheelMeshFL/FR/RL/RR` | 경찰차 휠 Mesh 4개 |
| `VehicleMovementConfig.bUseMovementOverrides` | `true` |
| `VehicleMovementConfig.DifferentialType` | `RearWheelDrive` |
| `VehicleMovementConfig.FrontWheelMaxSteerAngle` | `35` |
| `WheelVisualConfig.ExpectedWheelCount` | `4` |
| `WheelVisualConfig.FrontWheelCountForSteering` | `2` |
| `VehicleReferenceConfig.FrontWheelClass` | `BP_Wheel_Front_C` |
| `VehicleReferenceConfig.RearWheelClass` | `BP_Wheel_Rear_C` |
| `DriveStateConfig` | 존재 |

판단:

```text
DA_PoliceCar는 과거 CFVehicleData 적용 경로를 확인한 레거시 감사 기준으로만 남긴다.
현재 작업에는 사용하지 않는다.
P0 전투 플랫폼의 후보 입력 DataAsset은 `DA_TestSedan`, `DA_TestSUV`로 별도 확인했다.
전투 플랫폼 데이터는 UCFVehicleData에 작게 확장하는 방향이 현재 구조와 가장 덜 충돌한다.
```

### 6.1 `DA_TestSedan` / `DA_TestSUV` 확인 결과

AssetDump 기준 두 자산 모두 `CFVehicleData` 기반 `PrimaryDataAsset`이다.

| 항목 | `DA_TestSedan` | `DA_TestSUV` |
|---|---|---|
| ChassisMesh | `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan` | `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV` |
| WheelMeshFL/FR/RL/RR | `/Game/CarFight/Vehicles/TestSedan/Wheel_*` | `/Game/CarFight/Vehicles/TestSUV/Wheel_*` |
| `bUseLayoutOverrides` | `true` | `true` |
| 휠 Anchor X/Y | FL `(160,-75)`, FR `(160,75)`, RL `(-110,-75)`, RR `(-110,75)` | FL `(160,-80)`, FR `(160,80)`, RL `(-130,-80)`, RR `(-130,80)` |
| `bUseMovementOverrides` | `false` | `true` |
| `bUseWheelVisualOverrides` | `false` | `true` |
| `FrontWheelClass` / `RearWheelClass` | `null` / `null` | `BP_Wheel_Front_C` / `BP_Wheel_Rear_C` |
| `bUseDriveStateOverrides` | `false` | `true` |
| Differential | `RearWheelDrive` | `RearWheelDrive` |
| Steer Angle | `35` | `33.48` |

판단:

```text
두 P0 후보 DataAsset은 메시와 휠 메시 참조가 분리되어 있다.
SUV는 주행 / 휠 / DriveState / WheelClass 설정까지 비교적 채워져 있다.
세단은 Chassis / WheelMesh / Layout 값은 있으나 WheelClass가 비어 있고 Movement / WheelVisual / DriveState Override가 꺼져 있다.
따라서 코드 착수 전 세단 설정을 SUV 수준으로 보강할지, 기본값 폴백 검증 대상으로 둘지 결정해야 한다.
```

---

## 7. `WBP_AimReticle` 확인 결과

AssetDump 기준:

| 항목 | 값 |
|---|---|
| Asset Class | `WidgetBlueprint` |
| Parent Class | `/Script/CarFight_Re.CFAimReticleWidget` |
| Generated Class | `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle_C` |
| Dump Status | `success` |

확인된 표시 슬롯:

| 항목 | 의미 |
|---|---|
| `Text_ReticleState` | Reticle 상태 표시 후보 |
| `Text_CanFire` | 발사 가능 여부 표시 후보 |
| `Text_ReticleHint` | 보조 설명 표시 후보 |
| `VehiclePawnRef` | Reticle이 읽을 차량 Pawn 참조 |
| `CachedReticleState` | 기본값 `Hidden` |
| `bCachedCanFire` | 기본값 `false` |
| `bAutoRefreshEveryTick` | 기본값 `true` |

판단:

```text
Reticle은 이미 Aim 상태를 표시할 표면이 있다.
전투 플랫폼 작업에서는 레티클 디자인보다 하드포인트 / 무기 상태 입력 계약이 우선이다.
```

---

## 8. StaticMesh Socket 확인 결과

AssetDump 기준:

| 자산 | Dump Status | Socket 수 | 판단 |
|---|---:|---:|---|
| `Combined_Body` | success | 0 | 임시 감사 자산. 하드포인트 Socket 없음 |
| `Mesh_TestSedan` | success | 6 | 휠 4개 + `Front_01`, `Top_01` |
| `Mesh_TestSUV` | success | 7 | 휠 4개 + `Front_01`, `Top_01`, `Top_02` |

판단:

```text
StaticMesh Socket 목록은 AssetDump bpdump IncludeDetails=true 기준으로 확인 가능하다.
P0 테스트 메시에는 하드포인트 작성 보조로 쓸 수 있는 위치 슬롯 소켓 후보가 있다.
하지만 런타임 원본은 계속 DataAsset 로컬 Transform이다.
```

---

## 9. Socket 감사 세부 결과

세부 결과는 별도 문서에 기록한다.

```text
Document/Plan/VehiclePlatformPlan/CF_MeshSocketAudit.md
```

판단:

```text
Combined_Body에는 현재 기준 Socket이 없다.
Mesh_TestSedan / Mesh_TestSUV에는 하드포인트 후보 Socket이 있다.
과거 AssetDump 당시 실제 SocketName은 Front_01, Top_01, Top_02처럼 HP_ prefix 없이 위치 슬롯만 표현했다.
2026-06-25 현재 저장 기준 테스트 메시 SocketName은 HP_Front_01, HP_Top_01, HP_Top_02 표준명으로 갱신되어 있다.
단, 모든 차량에 해당 하드포인트 소켓이 필수로 존재해야 하는 것은 아니다.
P0 하드포인트의 기본 기준은 DataAsset 로컬 Transform이다.
BP SceneComponent는 실제 규칙 원본이 아니라 배치 확인용 Preview로만 사용한다.
```

---

## 10. 코드 착수 판단

AssetDump 이후 상태:

| 항목 | 상태 |
|---|---:|
| 기준 BP | 확인됨 |
| 기준 DataAsset | 확인됨, 현재 기본은 `DA_TestSedan` |
| P0 대상 DataAsset | 확인됨, 세단 / SUV 하드포인트 슬롯 저장 완료 |
| 기준 Reticle | 확인됨 |
| Fire 입력 자산 | 확인됨 |
| 현재 전투 전용 자산 존재 | 확인되지 않음 |
| StaticMesh Socket | Combined_Body 없음, TestSedan / TestSUV 후보 있음 |
| 하드포인트 Transform 방식 | P0 기본안 확정 |

결론:

```text
코드 작업 베이스 문서는 더 단단해졌다.
하드포인트 위치 기준은 DataAsset 로컬 Transform으로 시작한다.
P0 후보 DataAsset은 `DA_TestSedan`, `DA_TestSUV`로 AssetDump 확인됐다.
`BP_CFVehiclePawn.VehicleData = DA_TestSedan` 기준 PIE 기본 주행 검증은 완료됐다.
`DA_TestSUV` 대조 테스트는 별도 자동화 없이 사용자가 수동으로 VehicleData를 전환해 확인한다.
다음 확인은 하드포인트 장착 / 터렛 프로파일을 어느 DataAsset 섹션으로 확정할지다.
현재 Sedan / SUV 모두 `Front_02` 소켓은 없지만, 이는 P0 차단 조건이 아니다.
`Front_02 + Gimbal`은 전방 하드포인트가 늘어날 수 있다는 가능성으로만 유지한다.
추가 터렛 / 무장이 필요하면 같은 위치 카테고리 안에서 `Top_02`, `Front_03`처럼 슬롯 인스턴스를 늘린다.
```

---

## 11. Changelog

### v0.15

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.14

- 과거 AssetDump의 `DA_PoliceCar` 연결 기록이 현재 저장 상태를 대표하지 않음을 명시했다.
- 현재 BP 기본 VehicleData가 `DA_TestSedan`으로 저장된 상태와 `DA_TestSUV` 대조 테스트 잔여 항목을 반영했다.

### v0.13

- 현재 저장된 테스트 메시의 하드포인트 소켓명이 `HP_` prefix 표준명으로 갱신된 상태를 반영했다.
- `DA_TestSedan`, `DA_TestSUV`의 하드포인트 슬롯 저장 완료 상태를 코드 착수 판단에 반영했다.
- 남은 작업을 `BP_CFVehiclePawn.VehicleData` 테스트 전환 방식과 장착 / 터렛 프로파일 구조 확정으로 좁혔다.

### v0.12

- 하드포인트 소켓이 모든 차량에 필수로 존재해야 한다는 표현을 제거했다.
- `Front_02 + Gimbal`을 P0 결정 항목이 아니라 하드포인트 증가 가능성으로 낮췄다.
- AssetDump 소켓 목록은 캡처 후보 근거일 뿐 런타임 필수 조건이 아님을 명시했다.

### v0.11

- 감사 당시 SocketName과 코드 착수 표준 SocketName을 분리했다.
- `Front_01`, `Top_01`, `Top_02`를 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`로 리네임하는 결정을 반영했다.
- 하드포인트 LocalTransform 캡처 방식을 바퀴 위치 캡처 버튼 통합 구현으로 정리했다.

### v0.10

- `bpdump -IncludeDetails=true`로 StaticMesh Socket 상세 확인이 가능함을 반영했다.
- `Combined_Body`, `Mesh_TestSedan`, `Mesh_TestSUV` Socket 감사 결과를 추가했다.
- `Mesh_TestSedan` / `Mesh_TestSUV`의 하드포인트 후보 Socket을 `CF_MeshSocketAudit.md`로 분리했다.
- 기존 StaticMesh Details 미지원 표현을 과거 실행 맥락으로 낮추고 현재 확인 기준을 갱신했다.

### v0.9

- 다음 확인 대상을 위치 카테고리가 아니라 `Front_01`, `Front_02`, `Top_01` 같은 위치 슬롯 인스턴스 기준으로 갱신했다.
- 같은 위치 카테고리에 여러 터렛 / 무장을 둘 수 있음을 AssetDump 이후 확인 항목에 반영했다.

### v0.8

- 다음 확인 대상을 결합형 하드포인트 이름이 아니라 위치 슬롯 Transform과 장착 프로파일 참조 기준으로 갱신했다.

### v0.7

- 하드포인트 위치 작성 방식 기준 문서 `CF_HardpointAuthoring.md`를 Migration 메모에 연결했다.

### v0.6

- 소스 엔진 `D:\UnrealEngine_Source` 기준으로 `DA_TestSedan`, `DA_TestSUV` AssetDump를 실행한 결과를 추가했다.
- P0 후보 DataAsset의 실제 메시 경로를 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV` 기준으로 정정했다.
- `DA_TestSedan`은 WheelClass와 일부 Override 설정 보강이 필요하고, `DA_TestSUV`는 상대적으로 설정이 채워져 있음을 기록했다.

### v0.5

- 파일 시스템 기준 P0 임시 메시 `Mesh_TestSedan`, `Mesh_TestSUV` 지정 상태를 반영했다.
- 다음 확인 대상을 세단 / SUV용 차량 DataAsset 지정과 메시별 Transform 후보 수치로 좁혔다.

### v0.4

- `DA_PoliceCar`를 최종 P0 입력 DataAsset이 아니라 임시 감사 기준으로 정정했다.
- 다음 확인 대상을 P0 대상 세단 / SUV의 실제 메시와 DataAsset 지정, 지정 차량별 Transform 후보 수치로 변경했다.

### v0.3

- 하드포인트 Transform 수치가 전 차량 공통값이 아니라 기준 차량 `DA_PoliceCar` 후보값임을 명시했다.
  이 판단은 v0.4에서 `DA_PoliceCar` 임시 감사 기준으로 정정됐다.

### v0.2

- 사용자 에디터 확인 결과로 `Combined_Body` Socket 수가 0개임을 기록했다.
- P0 하드포인트 기준을 Mesh Socket이 아니라 DataAsset 로컬 Transform으로 고정했다.
- BP SceneComponent는 규칙 원본이 아니라 Preview 용도로만 사용한다고 정리했다.

### v0.1

- AssetDump 실행 가능 상태를 확인했다.
- 기준 BP / DataAsset / Reticle / 차체 StaticMesh 덤프 결과를 기록했다.
- StaticMesh Details 미지원으로 Socket 확인이 남았음을 명확히 분리했다.
- VehicleDebug 위젯 파일 존재와 BP 연결 여부를 분리해야 함을 기록했다.

---

## 12. Migration 메모

- 이 문서는 코드 또는 `.uasset` 변경 지시서가 아니다.
- AssetDump 원본 JSON은 `AssetDumpEvidence` 폴더에 보관한다.
- StaticMesh Socket 감사 결과는 `SocketAudit_20260625` 폴더와 `CF_MeshSocketAudit.md`를 기준으로 본다.
- Socket 후보가 있더라도 DataAsset Transform 기준은 `CF_EditorAssetAudit.md`, `CF_CodeStartGate.md`, `CF_HardpointMatrix.md`, `CF_HardpointAuthoring.md`에 함께 반영한다.
