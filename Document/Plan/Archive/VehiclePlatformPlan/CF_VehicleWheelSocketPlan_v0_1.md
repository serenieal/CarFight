# CF Vehicle Socket Capture Plan v0.8

- 작성일: 2026-06-23
- 최신 변경: v0.8 / 2026-06-25
- 대상 프로젝트: `D:\Work\CarFight_git`
- 대상 메시: `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan`, `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV`
- 대상 DataAsset: `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan`, `/Game/CarFight/Vehicles/Data/Cars/DA_TestSUV`
- 대상 C++: `UCFVehicleData`, `ACFVehiclePawn`
- 목표: 차체 메시의 바퀴 소켓과 차량 설정에 선언된 하드포인트 캡처 소켓을 한 번에 읽어 차량 DataAsset의 Layout / Hardpoint LocalTransform에 자동 기록한다.

---

## 1. 현재 문제

`VehicleLayoutConfig` 도입 후 런타임 기준은 DataAsset으로 이동했다. 하드포인트도 같은 원칙을 따른다.
하지만 새 차량을 만들 때는 여전히 다음 수작업이 필요하다.

1. 차체 메시를 보며 바퀴 중심을 눈으로 잡는다.
2. `Wheel_Anchor_FL/FR/RL/RR`를 수동 배치한다.
3. 해당 차량 설정에서 소켓 캡처를 쓰는 하드포인트 위치 소켓을 수동 배치한다.
4. 각 소켓의 위치/회전을 숫자로 읽는다.
5. `DA_*`의 Layout / Hardpoint 섹션에 직접 입력한다.

이 과정은 오타와 좌표 복사 실수가 생기기 쉽고, 차량 메시가 바뀔 때마다 반복 비용이 크다.

---

## 2. 목표 구조

소켓은 최종 런타임 원본이 아니라, 차량 DataAsset 값을 만들기 위한 에디터 입력 소스로 사용한다.

```text
StaticMesh Socket
  -> UCFVehicleData.CaptureLayoutFromChassisSockets()
  -> VehicleData.VehicleLayoutConfig
  -> VehicleData.Hardpoint / WeaponMount 위치 슬롯 섹션
  -> ApplyVehicleLayoutConfig()
  -> Combat Fire / Aim은 DataAsset LocalTransform 사용
  -> Wheel_Anchor_FL/FR/RL/RR
  -> CFWheelSyncComp 기준 캡처
```

핵심 결정:

- 런타임 Source of Truth는 계속 `VehicleData.VehicleLayoutConfig`다.
- 차체 메시 소켓은 에디터 캡처용 입력 소스다.
- spawned-only 차량 기준의 기본 캡처 위치는 `DA_*` 상세 패널이다.
- `BP_CFVehiclePawn`의 CallInEditor 버튼은 레벨 배치 인스턴스가 있을 때만 보조로 사용한다.
- 기본 소켓 이름은 `Wheel_Anchor_FL/FR/RL/RR`로 둔다.
- 하드포인트 소켓을 둘 경우 이름은 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`처럼 `HP_` prefix를 사용한다.
- 외부 메시 예외를 위해 `VehicleLayoutConfig`에 소켓 이름 override 필드를 둔다.
- 소켓 4개 중 하나라도 누락되면 DataAsset을 부분 저장하지 않는다.
- 하드포인트 위치 슬롯은 차량 설정에 따라 있을 수도 있고 없을 수도 있다.
- 선언된 하드포인트 슬롯이 `SocketName`을 캡처 입력으로 사용할 때만 해당 소켓을 찾는다.

---

## 3. 범위 / 비범위

### 범위

- `FCFVehicleLayoutConfig`에 차체 휠 소켓 이름 4개 추가
- 기본값을 `Wheel_Anchor_FL/FR/RL/RR`로 설정
- `UCFVehicleData`의 에디터 전용 캡처 버튼에 하드포인트 캡처 통합
  - `CaptureLayoutFromChassisSockets`
- `ACFVehiclePawn`에 에디터 전용 버튼 추가
  - `CaptureWheelLayoutFromBodySockets`
  - `ApplyVehicleLayoutFromDataInEditor`
- `SM_Body`에 적용된 StaticMesh의 바퀴 / 하드포인트 소켓 Transform을 읽어 DataAsset에 기록
- `Tools\BuildEditor.bat` 빌드 검증

세부 코드 작업 계획:

```text
CF_SocketCaptureCodePlan.md
```

### 비범위

- 소켓을 런타임마다 직접 읽어 WheelSync 기준으로 쓰는 구조
- BP 그래프에 캡처 로직 추가
- `Wheel_Anchor_*` 컴포넌트 이름 변경
- 휠 소켓 이름 변경
- 하드포인트 Socket을 런타임 원본으로 사용
- Chaos 물리 휠 `WheelSetup.AdditionalOffset` 자동 동기화

---

## 4. 데이터 구조 제안

`FCFVehicleLayoutConfig`에 아래 필드를 추가한다.

| 필드 | 타입 | 기본값 | 설명 |
|---|---|---|---|
| `BodyWheelSocketFL` | `FName` | `Wheel_Anchor_FL` | 앞왼쪽 바퀴 중심 소켓 |
| `BodyWheelSocketFR` | `FName` | `Wheel_Anchor_FR` | 앞오른쪽 바퀴 중심 소켓 |
| `BodyWheelSocketRL` | `FName` | `Wheel_Anchor_RL` | 뒤왼쪽 바퀴 중심 소켓 |
| `BodyWheelSocketRR` | `FName` | `Wheel_Anchor_RR` | 뒤오른쪽 바퀴 중심 소켓 |

운영 규칙:

- 비어 있거나 `None`이면 프로젝트 표준 이름을 사용한다.
- 소켓 이름 필드는 캡처용 메타데이터다.
- 런타임 적용은 기존 `WheelAnchorFL/FR/RL/RR` 포즈 값을 사용한다.

하드포인트 쪽은 차량 DataAsset의 위치 슬롯 섹션에 아래 성격의 데이터를 둔다.

| 필드 | 타입 | 기본값 예 | 설명 |
|---|---|---|---|
| `LocationSlotId` | `FName` | `Front_01` | 전투 규칙에서 참조할 위치 슬롯 ID |
| `LocationCategory` | enum 또는 `FName` | `Front` | Front / Back / LeftSide / RightSide / Top |
| `SocketName` | `FName` | `HP_Front_01` | 선택 캡처 입력. 비어 있으면 기존 LocalTransform을 직접 작성 / 유지 |
| `LocalLocation` | `FVector` | 소켓 캡처값 | 차량 로컬 기준 위치 |
| `LocalRotation` | `FRotator` | 소켓 캡처값 | 차량 로컬 기준 회전 |

하드포인트 권장 SocketName:

| 슬롯 | 권장 SocketName | 적용 조건 |
|---|---|---|
| `Front_01` | `HP_Front_01` | 차량이 전방 고정 슬롯을 소켓으로 캡처할 때 |
| `Top_01` | `HP_Top_01` | 차량이 루프 터렛 슬롯을 소켓으로 캡처할 때 |
| `Top_02` | `HP_Top_02` | 차량이 복수 루프 슬롯을 지원할 때 |
| `Front_02` | `HP_Front_02` | 전방 슬롯이 늘어날 수 있음을 보여주는 확장 가능성. P0 필수 아님 |

---

## 5. C++ 적용 흐름

### DataAsset 캡처 버튼

`UCFVehicleData::CaptureLayoutFromChassisSockets()`

1. `VehicleVisualConfig.ChassisMesh`가 있는지 확인한다.
2. `VehicleLayoutConfig.BodyWheelSocket*` 이름을 해석한다.
3. 차체 StaticMesh 소켓 4개가 모두 있는지 확인한다.
4. 소켓 로컬 위치/회전을 `WheelAnchorFL/FR/RL/RR`에 기록한다.
5. 차량 DataAsset에 선언된 하드포인트 위치 슬롯을 순회한다.
6. `SocketName`이 비어 있으면 해당 슬롯은 소켓 캡처 대상에서 제외하고 기존 LocalTransform을 유지한다.
7. `SocketName`이 있고 차체 메시에도 해당 소켓이 있으면 위치 슬롯 `LocalLocation / LocalRotation`에 기록한다.
8. `SocketName`이 있지만 차체 메시에서 찾지 못하면 해당 슬롯은 덮어쓰지 않고 경고를 남긴다.
9. `bUseLayoutOverrides=true`로 설정한다.
10. `VehicleData` 패키지를 Dirty 처리한다.
11. 성공/실패 요약을 Output Log, 화면 메시지, 에디터 알림으로 표시한다.

제약:

- 이 경로는 `SM_Body`가 BP 안에서 위치/회전 보정 없이 사용되는 현재 구조에 맞춘다.
- 나중에 `SM_Body` 컴포넌트 자체에 상대 Transform이 들어가면, 레벨 배치 인스턴스 버튼 또는 별도 프리뷰 액터 기반 캡처가 더 정확하다.

### Pawn 인스턴스 캡처 버튼

`ACFVehiclePawn::CaptureWheelLayoutFromBodySockets()`

1. `VehicleData`가 있는지 확인한다.
2. `ApplyVehicleVisualConfig()`로 `SM_Body`에 현재 차체 메시를 반영한다.
3. `SM_Body` StaticMeshComponent를 찾는다.
4. `VehicleLayoutConfig.BodyWheelSocket*` 이름을 해석한다.
5. 차체 메시 소켓 4개가 모두 있는지 확인한다.
6. `Wheel_Anchor_*` 컴포넌트가 모두 있는지 확인한다.
7. 소켓 월드 Transform을 각 앵커 부모 기준 상대 Transform으로 변환한다.
8. `VehicleData.VehicleLayoutConfig.WheelAnchor*`에 기록한다.
9. 차량 DataAsset에 선언된 하드포인트 위치 슬롯 중 `SocketName`이 있는 항목만 해석한다.
10. 차체 메시 하드포인트 소켓이 있으면 월드 Transform을 차량 로컬 또는 Body 기준 Transform으로 변환한다.
11. 변환에 성공한 슬롯만 `VehicleData`의 하드포인트 위치 슬롯 LocalTransform에 기록한다.
12. `bUseLayoutOverrides=true`로 설정한다.
13. `VehicleData` 패키지를 Dirty 처리한다.
14. `ApplyVehicleLayoutConfig()`를 호출해 현재 에디터 프리뷰에도 즉시 반영한다.
15. 에디터 컴포넌트 갱신을 위해 `Wheel_Anchor_*`에 `Modify`, `SetRelativeLocationAndRotation`, `UpdateComponentToWorld`를 적용한다.
16. `WheelSyncComp.TryPrepareWheelSync()`를 다시 호출해 기준 위치 캐시를 새 레이아웃으로 갱신한다.
17. 성공/실패 요약을 Output Log, 화면 메시지, 에디터 알림으로 표시한다.

### 적용 버튼

`ACFVehiclePawn::ApplyVehicleLayoutFromDataInEditor()`

1. 현재 `VehicleData.VehicleLayoutConfig`를 읽는다.
2. 기존 `ApplyVehicleLayoutConfig()`를 호출한다.
3. `WheelSyncComp.TryPrepareWheelSync()`를 호출해 기준 위치 캐시를 다시 잡는다.
4. DataAsset 값이 실제 앵커 위치로 반영되는지 에디터에서 확인한다.

---

## 6. 에디터 사용 절차

### 기본 절차: spawned-only 차량

1. 정적 메시 편집기(`Static Mesh Editor`)에서 `Mesh_TestSUV`를 연다.
2. 소켓 관리자(`Socket Manager`)에 아래 휠 소켓이 있는지 확인한다.
   - `Wheel_Anchor_FL`
   - `Wheel_Anchor_FR`
   - `Wheel_Anchor_RL`
   - `Wheel_Anchor_RR`
3. 이 차량 DataAsset의 하드포인트 위치 슬롯이 `SocketName`을 쓰는 경우에만 대응 소켓을 확인한다.
   - `HP_Front_01`
   - `HP_Top_01`
   - `HP_Top_02`
4. `DA_TestSUV`를 연다.
5. `DA_TestSUV`의 차체 메쉬(`ChassisMesh`)가 `Mesh_TestSUV`인지 확인한다.
6. `DA_TestSUV` 세부 정보(`Details`)에서 `차체 소켓에서 차량 레이아웃 캡처 (Capture Vehicle Layout From Chassis Sockets)` 버튼을 누른다.
7. `DA_TestSUV`를 저장한다.
8. PIE에서 스폰된 차량의 휠 시각 위치, 조향 피벗, 하드포인트 Debug 표시를 확인한다.

### 보조 절차: 레벨 배치 액터가 있을 때

1. 레벨에 배치된 `BP_CFVehiclePawn` 인스턴스에 `DA_TestSUV`를 연결한다.
2. 배치 액터를 선택한다.
3. 세부 정보(`Details`)에서 `메시 소켓에서 차량 레이아웃 캡처 (Capture Vehicle Layout From Body Sockets)` 버튼을 누른다.
4. `DA_TestSUV`를 저장한다.

주의:

- spawned-only 차량에서는 BP 에디터의 Pawn 버튼을 기준으로 삼지 않는다.
- BP 클래스 에디터 프리뷰에서 Pawn CallInEditor 버튼은 표시되어도 실제 호출이 안 되거나 로그가 남지 않을 수 있다.
- Pawn 버튼은 선택된 Pawn 인스턴스의 `VehicleData`를 갱신한다. 다른 DA가 연결된 액터에서 누르면 `DA_TestSUV`가 바뀌지 않는다.
- 성공 시 `VehicleLayoutSocketCapture: Applied ... WheelSync=Ready ... Hardpoints=...` 메시지가 Output Log, 화면 메시지, 또는 에디터 알림으로 표시되어야 한다.

---

## 7. 검증 방법

### 빌드 검증

- 명령: `D:\Work\CarFight_git\Tools\BuildEditor.bat`
- 예상 결과: UHT와 C++ 컴파일 성공

### 에디터 검증

- `Mesh_TestSUV` 소켓 4개가 모두 존재한다.
- `DA_TestSUV` 캡처 버튼을 누르면 `DA_TestSUV.VehicleLayoutConfig.bUseLayoutOverrides`가 켜진다.
- `WheelAnchorFL/FR/RL/RR`에 소켓 기반 위치/회전이 기록된다.
- 차량 DataAsset에 선언된 하드포인트 슬롯 중 `SocketName`이 있고 실제 소켓도 존재하는 항목만 위치 슬롯 LocalTransform으로 기록된다.
- 버튼 실행 후 `VehicleDataSocketCapture: Applied` 또는 실패 사유가 표시된다.
- PIE 외부에서 화면 메시지가 보이지 않더라도 에디터 알림 또는 Output Log에는 결과가 표시된다.
- `BP_CFVehiclePawn` EventGraph에는 새 로직이 추가되지 않는다.

### 런타임 검증

- PIE에서 차량이 정상 스폰된다.
- 휠 시각 위치가 SUV 차체 휠하우스 중심과 맞는다.
- `WheelSync`가 Ready 상태로 유지된다.
- Debug 또는 로그에서 선언된 하드포인트 위치 슬롯의 캡처 / 유지 결과가 확인된다.
- 조향/스핀/서스펜션 시각이 기존 경찰차 기준에서 후퇴하지 않는다.

---

## 8. 실패 체크포인트

- 소켓 4개 중 하나가 없는데 DataAsset 일부만 갱신된다.
  - 실패: 부분 저장 방지 원칙 위반
- 차량 DataAsset에 선언되지 않은 하드포인트 소켓까지 필수처럼 검사한다.
  - 실패: 차량별 설정에 따라 소켓은 있을 수도 있고 없을 수도 있다는 결정 위반
- `SocketName`이 지정된 하드포인트 슬롯의 실제 소켓이 없는데 기존 LocalTransform을 조용히 덮어쓴다.
  - 실패: 누락 경고를 남기고 해당 슬롯은 덮어쓰지 않아야 한다.
- 하드포인트 캡처용 소켓 이름이 `Front_01`처럼 `HP_` prefix 없이 남아 있다.
  - 실패: 캡처용 SocketName 표준 위반
- `SM_Body`에 차체 메시가 적용되지 않은 상태로 캡처된다.
  - 실패: 현재 차량 DataAsset의 Visual 적용 순서 확인
- 차체 메시 소켓 Transform을 그대로 복사해 앵커 부모 기준이 틀어진다.
  - 실패: 월드 Transform을 앵커 부모 기준 상대 Transform으로 변환해야 한다.
- 런타임에서 매번 소켓을 읽는다.
  - 실패: 런타임 Source of Truth가 DataAsset이라는 결정 위반
- 외부 메시의 소켓 이름이 달라서 표준 이름만 강제한다.
  - 실패: DA 소켓 이름 override로 흡수해야 한다.
- 버튼을 눌렀는데 변화가 없어 보인다.
  - spawned-only 흐름에서는 `DA_TestSUV`의 버튼을 눌렀는지 확인한다.
  - Output Log에서 `VehicleDataSocketCapture` 메시지를 확인한다.
  - 에디터를 코드 빌드 후 재시작하지 않았다면 이전 DLL이 실행 중일 수 있으므로 에디터를 재시작한다.
  - BP 클래스 프리뷰의 Pawn 버튼을 눌렀다면 `DA_TestSUV` 상세 패널 버튼으로 다시 테스트한다.

---

## 9. Changelog

### v0.7

- 세부 코드 작업 계획 문서 `CF_SocketCaptureCodePlan.md`를 참조로 추가했다.
- 통합 캡처 버튼의 구현 대상 파일과 실패 기준은 별도 코드 작업 계획에서 관리하도록 분리했다.

### v0.6

- 하드포인트 소켓을 차량별 설정에 따라 존재할 수도 있고 없을 수도 있는 선택 캡처 입력으로 정리했다.
- `HP_Front_01`, `HP_Top_01`을 모든 차량 필수 소켓처럼 해석하던 표현을 제거했다.
- `HP_Front_02`는 전방 하드포인트가 늘어날 수 있다는 확장 가능성으로만 남겼다.

### v0.5

- 바퀴 소켓 캡처 버튼에 하드포인트 위치 슬롯 캡처를 통합하는 방향으로 목표를 확장했다.
- 하드포인트 표준 SocketName을 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`로 확정했다.
- DataAsset 캡처 버튼과 Pawn 인스턴스 캡처 버튼이 휠 레이아웃과 하드포인트 LocalTransform을 함께 기록하도록 작업 흐름을 갱신했다.

### v0.4

- spawned-only 차량 기준으로 `UCFVehicleData` 직접 캡처 버튼을 기본 절차로 승격했다.
- BP 클래스 에디터의 Pawn CallInEditor 버튼은 표시되어도 실제 호출이 안 될 수 있음을 명시했다.
- 검증 로그 기준을 `VehicleDataSocketCapture`로 추가했다.

### v0.2

- 버튼 실행 후 에디터 컴포넌트 갱신과 WheelSync 재준비를 수행하도록 적용 흐름을 보강했다.
- 버튼 반응이 없어 보일 때 확인할 선택 액터, VehicleData, Output Log, WheelSync 체크포인트를 추가했다.

### v0.3

- PIE 외부에서도 버튼 실행 결과를 확인할 수 있도록 에디터 알림 표시 기준을 추가했다.
- 코드 빌드 후 에디터 재시작이 필요할 수 있다는 실패 체크포인트를 추가했다.

### v0.1

- `Mesh_TestSUV`의 `Wheel_Anchor_FL/FR/RL/RR` 소켓을 기준으로 `VehicleLayoutConfig`를 자동 캡처하는 계획을 추가했다.
- 소켓은 런타임 원본이 아니라 DataAsset 값을 생성하는 에디터 입력 소스라고 명시했다.
- 기본 소켓 이름은 표준화하되, DataAsset에서 예외 이름을 override할 수 있게 설계했다.
- 휠 소켓 누락 시 부분 저장을 금지하는 실패 처리 기준을 추가했다.

## 10. Migration 메모

- `DA_PoliceCar`는 현재 작업에 사용하지 않는다.
- 현재 기본 테스트 DA는 `DA_TestSedan`이며, `DA_TestSUV`는 대조 테스트 후보로 유지한다.
- `DA_TestSUV`는 `Mesh_TestSUV` 소켓 캡처 테스트 대상이다.
- 기존 BP 수동 앵커 배치는 fallback으로 유지한다.
- 차량별 차이는 BP 자식이 아니라 `DA_*`와 차체 메시 소켓 캡처로 관리한다.
