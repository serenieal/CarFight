# CarFight - 하드포인트 위치 작성 방식

> 역할: P0 하드포인트 위치 슬롯과 장착 타입을 분리하고, Socket / Preview 값을 DataAsset에 캡처하는 방식을 정의한다.
> 문서 버전: v1.0
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Implemented / Authoring Baseline

---

## 1. 목적

이 문서는 하드포인트 위치 작성 방식을 정리한다.

현재 휠 위치는 차체 메시의 `Wheel_Anchor_FL/FR/RL/RR` 소켓을 기준으로 잡고, 그 값을 `VehicleLayoutConfig`에 캡처하는 흐름이 있다.

하드포인트도 비슷하게 소켓을 사용할 수 있지만, 휠과 하드포인트는 성격이 다르다.

```text
휠 위치:
  차체 기하 구조에 가까움
  바퀴 중심을 어디에 둘지 결정

하드포인트 위치:
  전투 규칙에 가까움
  어디서 발사되는지
  어떤 무기를 달 수 있는지
  어떤 조준 방식과 발사각을 갖는지
  UI와 Fire 검증이 어떤 값을 읽는지 결정
```

따라서 P0 하드포인트는 아래 둘을 분리한다.

```text
위치 카테고리:
  Front, Back, LeftSide, RightSide, Top 같은 차체 위치 분류

위치 슬롯 인스턴스:
  Front_01, Front_02, Top_01, Top_02 같은 실제 장착 기준점

장착 타입:
  Fixed, Gimbal, Turret, Launcher, Utility 같은 전투 규칙
```

StaticMesh Socket은 위치 슬롯 인스턴스만 표현한다.
Fixed / Gimbal / Turret 같은 장착 타입은 DataAsset의 장착 / 터렛 섹션에서 해석한다.
같은 `Front`나 `Top` 위치 카테고리에도 여러 슬롯 인스턴스를 둘 수 있다.
예를 들어 전방 좌우 고정포는 `Front_01`, `Front_02`, 루프 쌍열 터렛은 `Top_01`, `Top_02`로 기록한다.

---

## 2. 현재 결론

P0 선택안:

```text
선택 방식:
  4. Socket / Preview로 배치 후 DataAsset에 캡처

런타임 Source of Truth:
  차량 DataAsset의 하드포인트 LocalTransform

에디터 작성 보조:
  StaticMesh Socket 또는 BP SceneComponent Preview 사용 가능

캡처 방향:
  Socket / Preview 위치를 보고 차량 DataAsset의 LocalLocation / LocalRotation 값으로 확정

명명 원칙:
  Mesh Socket은 위치 슬롯 인스턴스 중심으로 표기한다.
  DA 장착 / 터렛 섹션은 위치 슬롯을 참조하고 MountType을 별도로 가진다.

금지:
  Mesh Socket 이름에 Fixed / Gimbal / Turret 같은 장착 타입을 섞지 않는다.
  P0 런타임 코드가 Mesh Socket 존재를 필수 조건으로 삼지 않는다.
  BP SceneComponent를 전투 규칙 원본으로 삼지 않는다.
```

이 결정은 `CF_HardpointMatrix.md`의 P0 Transform 기준을 구체화한다.

2026-06-25 현재 저장된 메시 Socket 기준:

```text
Mesh_TestSedan:
  HP_Front_01
  HP_Top_01

Mesh_TestSUV:
  HP_Front_01
  HP_Top_01
  HP_Top_02
```

현재 테스트 메시 소켓은 `HP_` prefix 표준명으로 저장되어 있다.
`DA_TestSedan`과 `DA_TestSUV`의 `HardpointSlots`도 이 소켓명을 선택 캡처 입력으로 사용한다.

표준명:

| 위치 슬롯 | 표준 소켓 |
|---|---|
| `Front_01` | `HP_Front_01` |
| `Top_01` | `HP_Top_01` |
| `Top_02` | `HP_Top_02` |

---

## 3. 방식 비교

| 방식 | 설명 | 장점 | 단점 | P0 판단 |
|---|---|---|---|---|
| StaticMesh Socket 원본 | 차체 메시 안에 `HP_Front_01`, `HP_Top_01` 같은 위치 슬롯 소켓을 두고 런타임에서 읽음 | 메시를 보면서 위치 잡기 쉬움 | 메시 리임포트 / 교체에 취약, 전투 규칙 저장 불가, Socket 누락 시 런타임 실패 위험 | 원본으로는 비추천 |
| DataAsset LocalTransform 원본 | `DA_TestSedan`, `DA_TestSUV`에 하드포인트 위치와 회전을 저장 | 차량별 규칙과 수치가 한 곳에 모임, Socket이 없어도 동작 | 숫자 입력만으로는 위치 잡기 불편 | 기본 원본 |
| BP SceneComponent 원본 | `BP_CFVehiclePawn`에 하드포인트 컴포넌트를 두고 위치를 읽음 | 뷰포트에서 배치가 직관적 | BP가 규칙 원본이 되어 확장 시 흐려짐 | 원본으로는 비추천 |
| Socket / Preview 캡처 | Socket 또는 BP Preview로 위치를 잡고 DataAsset에 복사 | 작성 편의와 런타임 안정성을 둘 다 확보 | 캡처 / 검증 절차 필요 | P0 선택 |
| 하이브리드 런타임 | DataAsset에 `LocalTransform`과 선택적 `SocketName`을 같이 둠 | 외부 에셋 예외를 흡수 가능 | 우선순위 규칙이 복잡해짐 | P1 후보 |

---

## 4. P0 선택 작성 흐름

P0 하드포인트는 아래 순서로 작성한다.

```text
01. 차체 메시 또는 BP Preview에서 위치를 눈으로 잡는다.
02. Socket / Preview 이름은 위치 슬롯 기준으로 붙인다.
03. 위치 / 회전을 차량 로컬 Transform으로 환산한다.
04. 차량 DataAsset의 위치 슬롯 섹션에 LocalLocation / LocalRotation을 기록한다.
05. DA 장착 / 터렛 섹션에서 위치 슬롯을 참조하고 MountType을 별도로 기록한다.
06. C++는 런타임에서 DataAsset 값만 읽는다.
07. Reticle / Debug는 C++가 계산한 하드포인트 상태만 표시한다.
```

작성 방식의 핵심:

```text
배치는 눈으로 한다.
확정은 자동 캡처 버튼이 DataAsset 숫자로 저장한다.
런타임은 DataAsset만 믿는다.
```

P0 우선 기록 대상:

| 우선순위 | 위치 슬롯 | 장착 타입 | 기존 통합 표기 | 대상 차량 |
|---:|---|---|---|---|
| 1 | `Front_01` | `Fixed` | `FrontFixed_SmallOrMedium` | `DA_TestSedan`, `DA_TestSUV` |
| 2 | `Top_01` | `Turret` | `RoofTurret_MediumOrLarge` | `DA_TestSedan`, `DA_TestSUV` |

P0에서는 `Front_02 + Gimbal`, `RoofOrRearLauncher_Small`, `UtilitySlot_01`의 위치까지 먼저 완성할 필요는 없다.
`Front_02`는 전방 하드포인트가 늘어날 수 있다는 가능성 예시로만 다룬다.
다만 ID와 역할은 `CF_HardpointMatrix.md` 기준을 유지한다.

기존 통합 표기는 문서 호환을 위한 별칭으로 남긴다.
실제 데이터 구조에서는 `LocationSlotId`와 `MountType`을 분리한다.
`Front`, `Top` 같은 값은 위치 카테고리이며, 실제 참조 대상은 `Front_01`, `Top_02` 같은 슬롯 인스턴스다.

---

## 5. 데이터 소유권

하드포인트 데이터는 위치 슬롯과 장착 / 터렛 프로파일로 나눈다.

| 데이터 | 소유자 | 이유 |
|---|---|---|
| `LocationSlotId` | 차량 DataAsset 위치 슬롯 섹션 | `Front_01`, `Front_02`, `Top_01` 같은 실제 장착 기준점 |
| `LocationCategory` | 차량 DataAsset 위치 슬롯 섹션 | `Front`, `Back`, `LeftSide`, `RightSide`, `Top` 같은 위치 분류 |
| `LocalLocation` | 차량 DataAsset 위치 슬롯 섹션 | 차량별 위치가 다름 |
| `LocalRotation` | 차량 DataAsset 위치 슬롯 섹션 | 차량별 기본 방향이 다름 |
| `SocketName` | 차량 DataAsset 위치 슬롯 섹션 | 선택 캡처 입력. 값이 있으면 `HP_Front_01`, `HP_Top_02` 같은 위치 슬롯 소켓을 찾음 |
| `PreviewComponentName` | 차량 DataAsset 위치 슬롯 섹션 | BP Preview를 찾을 때만 사용하는 선택 입력 |
| `MountProfileId` | DA 장착 / 터렛 섹션 | 기존 통합 표기 호환 또는 장착 프로파일 식별 |
| `LocationSlotRef` | DA 장착 / 터렛 섹션 | 어떤 위치 슬롯에 장착할지 참조 |
| `MountType` | DA 장착 / 터렛 섹션 | `Fixed`, `Gimbal`, `Turret`, `Launcher`, `Utility` 구분 |
| `MountLocalOffset` | DA 장착 / 터렛 섹션 | 같은 위치 슬롯 안에서 장착 타입별 세부 오프셋이 필요할 때 사용 |
| `SizeLimit` | DA 장착 / 터렛 섹션 | 장착 가능한 무기 크기 제한 |
| `Yaw / Pitch 제한` | DA 장착 / 터렛 섹션 | Fire 검증과 Reticle 상태에 필요 |
| `TurnRate / Stabilization` | DA 장착 / 터렛 섹션 | 터렛 / 짐벌 조준 성능에 필요 |

중요:

```text
SocketName과 PreviewComponentName은 위치 슬롯 인스턴스 캡처 보조다.
차량마다 하드포인트 소켓은 해당 차량 설정상 있을 수도 있고 없을 수도 있다.
Fixed / Gimbal / Turret 같은 장착 타입은 SocketName에 넣지 않는다.
P0 런타임에서 Socket / Preview가 반드시 존재해야 하는 필수 데이터가 아니다.
한 위치 카테고리에 여러 무장이 달릴 수 있으므로 `Front`나 `Top`만으로는 실제 장착점을 식별하지 않는다.
```

---

## 6. 에디터 작성 절차

### 6.1 StaticMesh Socket을 보조로 쓸 때

1. 콘텐츠 브라우저(`Content Browser`)에서 차체 메시를 연다.
   - `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan`
   - `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV`
2. 스태틱 메시 에디터(`Static Mesh Editor`)에서 소켓 관리자(`Socket Manager`)를 연다.
3. 차량 설정에서 소켓 캡처를 쓰는 위치 슬롯 인스턴스만 보조 소켓을 만든다.
   - `HP_Front_01`
   - `HP_Back_01`
   - `HP_LeftSide_01`
   - `HP_RightSide_01`
   - `HP_Top_01`
   - `HP_Top_02`
   - `HP_Front_02`는 전방 슬롯 확장 가능성 예시이며 P0 필수 소켓이 아니다.
4. 같은 위치 카테고리에 여러 기준점이 필요하면 숫자 suffix를 늘린다.
   - 예: `HP_Front_01`, `HP_Front_02`
   - 예: `HP_Top_01`, `HP_Top_02`
5. 소켓 이름에는 `Fixed`, `Gimbal`, `Turret` 같은 장착 타입을 넣지 않는다.
6. 각 소켓의 위치 / 회전을 차량 로컬 기준으로 잡는다.
7. 소켓 Transform을 `DA_TestSedan` / `DA_TestSUV`의 위치 슬롯 값으로 기록한다.
8. DA 장착 / 터렛 섹션에서 해당 위치 슬롯을 참조하고 MountType을 별도로 지정한다.
9. 소켓은 저장 편의용으로 남겨도 되지만, P0 런타임 원본으로 취급하지 않는다.

현재 테스트 메시에는 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`처럼 `HP_` prefix 표준 소켓이 저장되어 있다.
이 소켓은 선택 캡처 입력이며, 런타임 원본은 캡처 후 `DA_TestSedan` / `DA_TestSUV`에 저장된 `HardpointSlots.LocalLocation / LocalRotation`이다.
자동 캡처 버튼은 바퀴 위치 캡처 버튼에 통합하고, 휠 소켓과 하드포인트 소켓을 같은 실행에서 읽는다.

### 6.2 BP SceneComponent를 Preview로 쓸 때

1. `BP_CFVehiclePawn`을 연다.
2. 컴포넌트(`Components`) 패널에서 Preview 전용 SceneComponent를 둔다.
   - 예: `Preview_HP_Front_01`
   - 예: `Preview_HP_Front_02`
   - 예: `Preview_HP_Back_01`
   - 예: `Preview_HP_LeftSide_01`
   - 예: `Preview_HP_RightSide_01`
   - 예: `Preview_HP_Top_01`
   - 예: `Preview_HP_Top_02`
3. 뷰포트에서 위치를 조정한다.
4. 세부 정보(`Details`)의 상대 위치(`Relative Location`)와 상대 회전(`Relative Rotation`) 값을 읽는다.
5. 값을 차량 DataAsset의 위치 슬롯 후보값으로 기록한다.
6. Preview 컴포넌트는 시각 확인용이며 Fire 검증 원본으로 사용하지 않는다.

주의:

```text
현재 단계에서는 실제 BP 컴포넌트를 추가하라는 지시가 아니다.
이 절차는 위치 작성 방법 후보를 설명하기 위한 문서 기준이다.
코드 작업 또는 BP 변경은 별도 승인 후 진행한다.
```

---

## 7. 검증 기준

하드포인트 위치 작성 방식은 아래를 통과해야 한다.

| 검증 항목 | 통과 기준 |
|---|---|
| DataAsset 원본성 | 런타임 Fire / Aim은 DataAsset 하드포인트 값을 읽는다 |
| 차량별 차이 | `DA_TestSedan`, `DA_TestSUV`가 같은 ID를 쓰더라도 위치 수치는 다를 수 있다 |
| 위치 / 타입 분리 | Socket / Preview 이름은 위치 슬롯 인스턴스만 나타내고, `MountType`은 DA 장착 / 터렛 섹션에 있다 |
| 복수 슬롯 지원 | 같은 `Front`나 `Top` 위치 카테고리에 `Front_01`, `Front_02`, `Top_01`, `Top_02` 같은 복수 슬롯을 둘 수 있다 |
| Socket 비의존 | 차체 메시에서 하드포인트 소켓을 지워도 DataAsset 값이 있으면 P0는 동작한다 |
| BP 비원본 | BP SceneComponent 삭제가 전투 규칙 손실로 이어지지 않는다 |
| Reticle 연동 | Reticle은 위치를 직접 판단하지 않고 Fire / Aim 상태를 표시한다 |
| Debug 관측 | Debug에는 활성 `MountProfileId`, `LocationSlotId`, LocalTransform, Fire Reject Reason을 확인할 수 있어야 한다 |

실패로 보는 경우:

```text
DataAsset에 선언된 위치 슬롯의 LocalTransform이 없으면 발사 위치를 만들 수 없다.
Socket / BP Preview가 없다는 이유만으로 Fire 검증이 실패해서는 안 된다.
Socket 이름이 `HP_FrontFixed`, `HP_RoofTurret`처럼 위치와 타입을 섞는다.
하드포인트 위치는 있는데 ID / AimMode / SizeLimit / Arc 정보가 없다.
세단과 SUV가 같은 수치를 복사해 실제 차체와 맞지 않는다.
```

---

## 8. 코드 착수 전 결정

코드 작업 직전에 아래만 확정하면 된다.

| 결정 | P0 선택 |
|---|---|
| 하드포인트 원본 | DataAsset LocalTransform |
| 위치 작성 보조 | StaticMesh Socket 또는 BP Preview 사용 후 바퀴 위치 캡처 버튼에서 함께 DataAsset에 캡처 |
| Socket 명명 | 위치 슬롯 인스턴스 중심. `HP_Front_01`, `HP_Back_01`, `HP_LeftSide_01`, `HP_RightSide_01`, `HP_Top_01` |
| 타입 저장 | DA 장착 / 터렛 섹션의 `MountType` |
| 런타임 Socket 읽기 | P0에서는 필수 아님 |
| 첫 장착 프로파일 | `LocationSlot=Front_01`, `MountType=Fixed`와 `LocationSlot=Top_01`, `MountType=Turret` |
| 확장 장착 프로파일 | `LocationSlot=Front_02`, `MountType=Gimbal`은 전방 하드포인트 증가 가능성으로만 기록 |
| Reticle 작업 | 하드포인트 상태 표시까지만 |

남은 Open Question:

| 항목 | 상태 |
|---|---|
| DA 섹션 최종 명칭을 `Hardpoint`, `WeaponMount`, `TurretMount` 중 무엇으로 둘지 | 구현 문서에서 확정 |
| `SocketName` 필드 포함 여부 | 선택 캡처 입력으로 포함. 차량별 설정에 따라 비울 수 있음 |
| `PreviewComponentName` 필드 포함 여부 | P0 필수 아님. BP Preview / 외부 메시 예외 대응용 선택 입력 |

---

## 9. Changelog

### v1.0

- 현재 저장된 테스트 메시 소켓명을 `HP_Front_01`, `HP_Top_01`, `HP_Top_02` 기준으로 갱신했다.
- `DA_TestSedan`, `DA_TestSUV`의 `HardpointSlots`가 표준 소켓명을 선택 캡처 입력으로 가진 상태를 반영했다.
- `Front_01`, `Top_01`, `Top_02`를 리네임해야 한다는 과거 상태 문구를 현재 완료 상태로 정리했다.

### v0.9

- `Front_02 + Gimbal`을 P0 우선 검증 대상이 아니라 전방 하드포인트 확장 가능성으로 낮췄다.
- 하드포인트 소켓은 차량별 설정에 따라 있을 수도 있고 없을 수도 있음을 명시했다.
- `SocketName`을 P0 필수 입력이 아니라 선택 캡처 입력으로 정리했다.

### v0.8

- 통합 캡처 버튼 기준으로 `SocketName`을 P0 위치 슬롯 구조체의 캡처 입력으로 명확히 했다.
- `PreviewComponentName`은 P0 필수가 아니라 BP Preview / 예외 대응용 선택 입력으로 분리했다.
- 남은 Open Question에서 `SocketName` 포함 여부를 제거하고, `Front_02 + Gimbal`과 DA 섹션 명칭만 남겼다.

### v0.7

- 실제 메시 소켓명을 `HP_` prefix 표준으로 리네임해야 한다는 결정을 반영했다.
- 하드포인트 Transform 캡처를 바퀴 위치 캡처 버튼에 통합하는 자동 버튼 방식으로 확정했다.
- 수동 캡처 여부 Open Question을 제거하고, `Front_02 + Gimbal` 기준점 문제를 남은 결정으로 분리했다.
  이 판단은 v0.9에서 전방 하드포인트 확장 가능성으로 정정됐다.

### v0.6

- `CF_MeshSocketAudit.md` 기준으로 `Mesh_TestSedan`, `Mesh_TestSUV`의 실제 하드포인트 후보 Socket을 반영했다.
- 현재 실제 SocketName이 `HP_` prefix 없이 위치 슬롯만 표현하므로, 코드 착수 전 명명 결정을 남겼다.
- SocketName을 쓰더라도 C++가 prefix를 하드코딩하지 않고 DataAsset에 기록된 실제 이름을 읽어야 함을 추가했다.

### v0.5

- `Front`, `Top` 같은 위치를 단일 슬롯으로 보지 않고 `LocationCategory`와 `LocationSlotId`를 분리했다.
- 같은 위치 카테고리에 여러 터렛 / 무장을 달 수 있도록 `Front_01`, `Front_02`, `Top_01`, `Top_02` 슬롯 인스턴스 규칙을 추가했다.
- Socket / Preview 명명 예시를 `HP_Front_01`, `Preview_HP_Top_02`처럼 복수 슬롯을 지원하는 형식으로 갱신했다.

### v0.4

- Debug 관측 기준을 활성 하드포인트 ID 단일 표현에서 `MountProfileId`, `LocationSlotId`, LocalTransform 확인 기준으로 정리했다.

### v0.3

- 하드포인트 Socket / Preview 이름은 위치 슬롯만 표현하고, Fixed / Gimbal / Turret 같은 장착 타입은 DA 장착 / 터렛 섹션에서 분리해 기록하는 원칙을 추가했다.
- `HP_FrontFixed`, `HP_RoofTurret`, `HP_FrontGimbal` 같은 결합형 소켓 이름을 금지하고 `HP_Front`, `HP_Top` 같은 위치 중심 이름으로 정리했다.
- 기존 `FrontFixed_SmallOrMedium`, `RoofTurret_MediumOrLarge`, `FrontGimbal_Small` 표기는 호환용 별칭으로 남기고 실제 데이터 구조는 `LocationSlotId + MountType`으로 분리했다.

### v0.2

- 사용자가 4번 방식, 즉 Socket / Preview로 배치 후 DataAsset에 캡처하는 방식을 P0 선택안으로 확정했다.
- `P0 권장` 표현을 `P0 선택`으로 승격했다.
- 남은 Open Question을 작성 방식 선택이 아니라 캡처 자동화 수준과 보조 입력 선택 문제로 좁혔다.

### v0.1

- 하드포인트 위치 작성 방식을 StaticMesh Socket, DataAsset LocalTransform, BP SceneComponent, 캡처 방식, 하이브리드 방식으로 비교했다.
- P0 기본안을 DataAsset LocalTransform 원본 + Socket / Preview 작성 보조로 정리했다.
- 하드포인트와 휠 위치 작성의 차이를 명시했다.

---

## 10. Migration 메모

- 이 문서는 코드 또는 `.uasset` 변경 지시서가 아니다.
- 기존 `CF_HardpointMatrix.md`의 통합 표기는 호환용 별칭으로 유지한다.
- 신규 문서 기준은 위치 슬롯과 장착 타입을 분리한다.
- 기존 휠 소켓 캡처 문서의 원칙을 하드포인트에 그대로 복사하지 않는다.
- 하드포인트 소켓이 있더라도 P0 규칙 원본은 DataAsset LocalTransform이다.
