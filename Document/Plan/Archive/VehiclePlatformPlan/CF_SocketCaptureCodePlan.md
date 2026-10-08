# CarFight - 차량 레이아웃 통합 캡처 코드 작업 계획

> 역할: 휠 소켓 캡처 버튼에 하드포인트 LocalTransform 캡처를 통합하기 위한 코드 작업 범위를 정의한다.
> 문서 버전: v0.2
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Implemented / Build Verified / DA Seeded

---

## 1. 목적

이 문서는 코드 작업을 바로 시작하기 위한 실행 계획이다.
구현 대상은 기존 휠 레이아웃 캡처 흐름을 유지하면서, 차량 DataAsset에 선언된 하드포인트 위치 슬롯만 선택적으로 캡처하는 것이다.

핵심 결론:

```text
휠 소켓은 차량 레이아웃 캡처의 필수 입력이다.
하드포인트 소켓은 차량별 설정에 따라 있을 수도 있고 없을 수도 있는 선택 캡처 입력이다.
런타임 Fire / Aim 기준은 소켓이 아니라 DataAsset LocalTransform이다.
```

---

## 2. 현재 코드 근거

현재 코드에 있는 흐름:

| 파일 | 현재 역할 |
|---|---|
| `UE/Source/CarFight_Re/Public/CFVehicleData.h` | `FCFVehicleLayoutConfig`, `FCFVehicleHardpointSlot`, `HardpointSlots`, `UCFVehicleData::CaptureLayoutFromChassisSockets()` 선언 |
| `UE/Source/CarFight_Re/Private/CFVehicleData.cpp` | `VehicleVisualConfig.ChassisMesh`의 `Wheel_Anchor_*` 소켓과 선언된 하드포인트 `SocketName`을 DataAsset 값으로 캡처 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | `ACFVehiclePawn::CaptureWheelLayoutFromBodySockets()` 에디터 버튼 선언 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | `SM_Body` 소켓을 읽어 `VehicleData.VehicleLayoutConfig`와 `HardpointSlots`에 기록하고 WheelSync 재준비 |
| `UE/Source/CarFight_ReEditor/Private/CFVDAWizardTab.cpp` | VDA Wizard에서 `CaptureLayoutFromChassisSockets()` 호출 |

함수 시그니처는 유지했고, 표시명과 내부 동작은 차량 레이아웃 통합 캡처로 확장했다.

---

## 3. 구현 원칙

1. 기존 휠 캡처 동작을 깨지 않는다.
2. 하드포인트 캡처는 차량 DataAsset에 선언된 슬롯만 순회한다.
3. `SocketName`이 비어 있는 하드포인트 슬롯은 캡처하지 않고 기존 `LocalLocation / LocalRotation`을 유지한다.
4. `SocketName`이 있지만 실제 소켓이 없으면 경고를 남기고 해당 슬롯은 덮어쓰지 않는다.
5. 하드포인트 소켓 누락은 전체 캡처 실패가 아니다.
6. 휠 소켓 누락은 기존처럼 캡처 실패이며 DataAsset을 부분 저장하지 않는다.
7. `HP_Front_02`는 P0 구현 대상이 아니라 하드포인트 확장 가능성 예시다.
8. BP 그래프에는 캡처 로직을 추가하지 않는다.

---

## 4. 데이터 구조 계획

### 4.1 신규 구조체

대상 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleData.h
```

작업 유형:

```text
수정 / v1.18.0 예정
```

추가 후보:

| 이름 | 역할 |
|---|---|
| `FCFVehicleHardpointSlot` | 차량별 하드포인트 위치 슬롯 LocalTransform과 선택 캡처 입력을 저장 |

필드 후보:

| 필드 | 타입 | 의미 |
|---|---|---|
| `LocationSlotId` | `FName` | 전투 규칙에서 참조할 슬롯 ID. 예: `Front_01`, `Top_01` |
| `LocationCategory` | `FName` | 분류용 위치. 예: `Front`, `Top`, `Back` |
| `SocketName` | `FName` | 선택 캡처 입력. 비어 있으면 소켓 캡처를 건너뜀 |
| `LocalLocation` | `FVector` | 차량 / 차체 기준 위치 |
| `LocalRotation` | `FRotator` | 차량 / 차체 기준 회전 |

툴팁 기준:

```text
SocketName:
  "이 슬롯의 위치를 차체 StaticMesh 소켓에서 캡처할 때 사용할 이름입니다. 비어 있으면 캡처하지 않고 LocalTransform 값을 유지합니다."
```

### 4.2 DataAsset 필드

`UCFVehicleData`에 아래 배열을 추가한다.

| 필드 | 타입 | 의미 |
|---|---|---|
| `HardpointSlots` | `TArray<FCFVehicleHardpointSlot>` | 이 차량이 실제로 제공하는 하드포인트 위치 슬롯 목록 |

기본값:

```text
빈 배열
```

의미:

```text
빈 배열이면 해당 차량은 아직 하드포인트 위치 슬롯을 선언하지 않은 상태다.
이 상태는 오류가 아니다.
```

---

## 5. CaptureLayoutFromChassisSockets 확장

대상 파일:

```text
UE/Source/CarFight_Re/Private/CFVehicleData.cpp
```

작업 유형:

```text
수정 / v1.18.0 예정
```

현재 동작:

```text
1. ChassisMesh 확인
2. Wheel_Anchor_* 소켓 4개 캡처
3. 하나라도 실패하면 DataAsset 수정 없이 종료
4. 성공 시 VehicleLayoutConfig 저장
```

추가 동작:

```text
5. HardpointSlots 배열을 순회
6. SocketName이 None이면 Skipped 처리
7. SocketName이 있고 ChassisMesh에 소켓이 있으면 LocalLocation / LocalRotation 갱신
8. SocketName이 있지만 소켓이 없으면 Missing 처리하고 기존 값 유지
9. 성공 메시지에 Hardpoints=Captured/Skipped/Missing 카운트 표시
```

상태 판정:

| 상태 | 처리 |
|---|---|
| `Captured` | 해당 슬롯의 LocalTransform 갱신 |
| `SkippedNoSocketName` | 기존 LocalTransform 유지, 실패 아님 |
| `MissingSocket` | 기존 LocalTransform 유지, 경고 |
| `InvalidSlotId` | 경고, 가능하면 해당 슬롯 유지 |

로그 예:

```text
VehicleDataSocketCapture: Applied, Wheels=4/4, Hardpoints=Captured=2, Skipped=1, Missing=0
VehicleDataSocketCapture: AppliedWithWarning, Wheels=4/4, Hardpoints=Captured=1, Skipped=1, Missing=1
```

---

## 6. CaptureWheelLayoutFromBodySockets 확장

대상 파일:

```text
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

작업 유형:

```text
수정 / v2.71.0 예정
```

현재 동작:

```text
SM_Body의 휠 소켓 월드 Transform을 Wheel_Anchor_* 부모 기준 상대 Transform으로 변환해 VehicleData에 저장한다.
```

추가 동작:

```text
1. 기존 휠 캡처가 성공한 뒤 HardpointSlots 순회
2. SocketName이 None이면 Skip
3. SocketName이 있고 SM_Body에 소켓이 있으면 Transform 캡처
4. 하드포인트 Transform은 P0에서 차체 기준 LocalTransform으로 저장
5. SocketName 누락은 경고만 남기고 전체 캡처를 실패시키지 않음
```

주의:

```text
DataAsset 버튼은 StaticMesh 소켓의 로컬 Transform을 그대로 읽는다.
Pawn 버튼은 SM_Body 컴포넌트 기준으로 변환해야 한다.
두 경로의 결과 좌표계가 어긋나지 않도록 "차체 기준 LocalTransform"을 P0 기준으로 고정한다.
```

---

## 7. 버튼 표시명 / 툴팁 변경

기존 함수 시그니처는 유지한다.
표시명과 툴팁만 차량 레이아웃 통합 캡처 의미로 갱신한다.

| 함수 | 현재 표시명 | 변경 표시명 |
|---|---|---|
| `UCFVehicleData::CaptureLayoutFromChassisSockets()` | 차체 소켓에서 휠 레이아웃 캡처 | 차체 소켓에서 차량 레이아웃 캡처 |
| `ACFVehiclePawn::CaptureWheelLayoutFromBodySockets()` | 메시 소켓에서 휠 레이아웃 캡처 | 메시 소켓에서 차량 레이아웃 캡처 |

툴팁 기준:

```text
차체 메시의 Wheel_Anchor_* 소켓과, 차량 DataAsset에 선언된 하드포인트 SocketName을 읽어 차량 레이아웃 값을 기록합니다.
하드포인트 SocketName이 비어 있거나 소켓이 없어도 휠 캡처 성공 자체를 실패로 보지 않습니다.
```

---

## 8. Validator 반영

대상 파일:

```text
UE/Source/CarFight_Re/Public/CFVDAValidator.h
UE/Source/CarFight_Re/Private/CFVDAValidator.cpp
```

작업 유형:

```text
수정 / v1.x 예정
```

검증 원칙:

| 항목 | 판정 |
|---|---|
| 휠 소켓 4개 누락 | Error |
| `HardpointSlots` 빈 배열 | Error 아님 |
| 하드포인트 `SocketName` 비어 있음 | 정상. 직접 입력 / 유지 슬롯 |
| 하드포인트 `SocketName`이 `HP_` prefix가 아님 | Warning |
| 하드포인트 `SocketName`이 있으나 메시 소켓 없음 | Warning |
| `LocationSlotId` 중복 | Warning 또는 Error 후보 |

금지:

```text
HP_Front_01 또는 HP_Top_01이 없다는 이유만으로 차량 DataAsset을 실패 처리하지 않는다.
```

---

## 9. VDA Wizard 영향

대상 파일:

```text
UE/Source/CarFight_ReEditor/Private/CFVDAWizardTab.cpp
```

현재 VDA Wizard는 `CaptureLayoutFromChassisSockets()`를 호출한다.
함수 시그니처를 유지하면 호출부는 큰 수정 없이 통합 캡처를 사용한다.

필요한 작업:

```text
버튼 라벨 / 설명 문구가 "휠 레이아웃"에 고정되어 있으면 "차량 레이아웃"으로 갱신한다.
```

---

## 10. 구현 순서

1. `CFVehicleData.h`에 `FCFVehicleHardpointSlot`과 `HardpointSlots` 추가.
2. `CFVehicleData.cpp`에 StaticMesh 소켓 기반 하드포인트 캡처 헬퍼 추가.
3. `CaptureLayoutFromChassisSockets()`에서 휠 캡처 성공 후 하드포인트 선택 캡처 실행.
4. `CFVehiclePawn.cpp`에 SM_Body 기준 하드포인트 캡처 헬퍼 추가.
5. `CaptureWheelLayoutFromBodySockets()`에서 휠 캡처 성공 후 하드포인트 선택 캡처 실행.
6. 버튼 표시명 / 툴팁을 차량 레이아웃 기준으로 갱신.
7. `CFVDAValidator`에 하드포인트 슬롯 검증을 Warning 중심으로 추가.
8. `D:\Work\CarFight_git\Tools\BuildEditor.bat`로 빌드.
9. 에디터에서 `DA_TestSUV` 캡처 버튼으로 로그와 저장값 확인.

---

## 11. 검증 기준

빌드 검증:

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

에디터 검증:

| 상황 | 기대 결과 |
|---|---|
| `HardpointSlots` 비어 있음 | 휠 캡처 성공, 하드포인트 Captured=0 |
| 슬롯이 있고 `SocketName=None` | Skipped 증가, 기존 LocalTransform 유지 |
| 슬롯이 있고 실제 소켓 있음 | Captured 증가, LocalTransform 갱신 |
| 슬롯이 있고 실제 소켓 없음 | Missing 증가, 경고 표시, 기존 LocalTransform 유지 |
| 휠 소켓 누락 | 전체 캡처 실패, DataAsset 부분 저장 없음 |

로그 검증:

```text
VehicleDataSocketCapture: Applied...
VehicleLayoutSocketCapture: Applied...
Hardpoints=Captured=...
Skipped=...
Missing=...
```

---

## 12. 실패 체크포인트

| 실패 | 의미 |
|---|---|
| `HP_Front_01`이 없다는 이유로 모든 차량 캡처 실패 | 차량별 소켓 선택 정책 위반 |
| `SocketName=None` 슬롯의 LocalTransform을 0으로 덮어씀 | 직접 입력 / 유지 슬롯 파괴 |
| 누락된 하드포인트 소켓 때문에 휠 캡처 저장까지 취소 | 하드포인트 선택 캡처 정책 위반 |
| DataAsset 버튼과 Pawn 버튼의 좌표계가 서로 다름 | 차체 기준 LocalTransform 정의 실패 |
| BP 그래프에 캡처 판단 로직 추가 | C++ 중심 / BP 얇은 계층 원칙 위반 |

---

## 13. 비범위

- `Front_02 + Gimbal` 실제 슬롯 추가
- 무기 장착 / Fire / Aim 런타임 구현
- Reticle 모양 추가
- 락온 / 런처 / Utility 구현
- 차량 종류 대량 추가
- BP 그래프 기반 캡처 로직 작성

---

## 14. Changelog

### v0.2

- 통합 캡처 코드가 구현되어 `Implemented / Build Verified / DA Seeded` 상태로 갱신했다.
- `FCFVehicleHardpointSlot`, `HardpointSlots`, DataAsset / Pawn 캡처 확장, Validator 연결, VDA Wizard 문구 갱신을 현재 코드 근거에 반영했다.
- `DA_TestSedan`, `DA_TestSUV`에 현재 `HP_*` 소켓 기준 하드포인트 슬롯이 저장된 상태를 반영했다.

### v0.1

- 기존 휠 소켓 캡처 버튼을 차량 레이아웃 통합 캡처로 확장하기 위한 코드 작업 계획을 작성했다.
- 하드포인트 소켓을 모든 차량 필수 조건이 아니라 차량 DataAsset에 선언된 슬롯의 선택 캡처 입력으로 정의했다.
- `HP_Front_02`는 P0 구현 대상이 아니라 전방 하드포인트 확장 가능성으로만 유지했다.
