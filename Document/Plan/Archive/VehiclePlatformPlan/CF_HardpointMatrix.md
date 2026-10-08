# CarFight - P0 하드포인트 매트릭스

> 역할: P0 하드포인트가 무기, 조준, Fire 검증, UI와 어떻게 연결되는지 정리한다.
> 문서 버전: v0.12
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / P0 Planning / P0 DA Active

---

## 1. 목적

이 문서는 하드포인트를 단순 슬롯 목록이 아니라 전투기능 연결 기준으로 정리한다.

하드포인트는 아래를 동시에 결정한다.

```text
어떤 무기를 달 수 있는가?
어디서 발사되는가?
어느 각도로 쏠 수 있는가?
어떤 Reticle 상태가 필요한가?
어떤 중량 비용을 만든다?
피격 시 어떤 위험을 갖는가?
```

---

## 2. P0 하드포인트 목록

P0 하드포인트는 위치 슬롯과 장착 타입을 분리해서 기록한다.

```text
위치 카테고리:
  Front, Back, LeftSide, RightSide, Top 같은 차체 기준 위치 분류

위치 슬롯 인스턴스:
  Front_01, Front_02, Top_01, Top_02 같은 실제 장착 기준점

장착 타입:
  Fixed, Gimbal, Turret, Launcher, Utility 같은 전투 규칙

기존 통합 표기:
  MountProfileId 후보 또는 문서 호환용 별칭
  Mesh Socket 이름으로 쓰지 않음
```

| MountProfileId 후보 | LocationSlotRef | LocationCategory | MountType | 대표 무기 | P0 목적 |
|---|---|---|---|---|---|
| `FrontFixed_SmallOrMedium` | `Front_01` | `Front` | `Fixed` | 오토캐논 | 차체 조준과 운전 실력 검증 |
| `FrontGimbal_Small` | `Front_02` | `Front` | `Gimbal` | 소형 기관총 | 빠른 목표 대응 검증 |
| `RoofTurret_MediumOrLarge` | `Top_01` | `Top` | `Turret` | 대구경 저속포 | 느린 각속도 / 안정화 검증 |
| `RoofOrRearLauncher_Small` | `Top_02` 또는 `Back_01` | `Top` 또는 `Back` | `Launcher` | 경량 미사일 | 락온 / 대응 장비 검증 |
| `UtilitySlot_01` | `Back_01`, `LeftSide_01`, `RightSide_01`, `Top_02` 또는 내부 슬롯 | 위치별 | `Utility` | 연막 / 플레어 | 미사일 대응 검증 |

Socket 이름은 위치만 표현한다.
예를 들어 전방 위치 소켓은 `HP_Front_01`, 루프 위치 소켓은 `HP_Top_01`이다.
`HP_FrontFixed`, `HP_RoofTurret`, `HP_FrontGimbal`처럼 위치와 장착 타입을 섞은 이름은 쓰지 않는다.
같은 위치 카테고리에 여러 무장이 달릴 수 있으므로 `Front`, `Top`만으로 실제 장착점을 식별하지 않는다.
복수 터렛은 `Top_01`, `Top_02`처럼 각각 별도 슬롯 인스턴스로 기록한다.

---

## 3. 하드포인트별 계약

### 3.1 `Front` + `Fixed`

호환용 MountProfileId 후보: `FrontFixed_SmallOrMedium`

| 항목 | 기준 |
|---|---|
| 조준 기준 | 차량 전방 |
| Aim 요구 | 플레이어 조준점과 차체 전방 차이 확인 |
| Fire 검증 | 차체 방향이 목표를 향하는지 확인 |
| Reticle 상태 | `Ready`, `OutOfArc`, `AimBlocked`, `NoWeapon` |
| 중량 영향 | 중간 |
| 피격 위험 | 전방 노출 장비 가능 |
| P0 판정 | 운전 방향과 사격 방향이 연결되는지 확인 |

주의:

- 이 하드포인트는 무기가 자유롭게 회전하면 안 된다.
- 운전 실력과 조준이 묶이는 것이 목적이다.

---

### 3.2 `Front` + `Gimbal`

호환용 MountProfileId 후보: `FrontGimbal_Small`

| 항목 | 기준 |
|---|---|
| 조준 기준 | 전방 제한 짐벌 |
| Aim 요구 | 제한 각도 안에서 빠른 목표 추적 |
| Fire 검증 | Yaw / Pitch 제한, 사선 차단 |
| Reticle 상태 | `Ready`, `OutOfArc`, `Cooldown`, `NoWeapon` |
| 중량 영향 | 낮음 |
| 피격 위험 | 전방 노출 가능 |
| P0 판정 | 경량 차량이 빠른 대응 무기로 압박 가능한지 확인 |

주의:

- 짐벌은 터렛보다 넓거나 강하면 안 된다.
- 빠른 대응은 가능하지만 전방 중심이라는 제한을 유지한다.

---

### 3.3 `Top` + `Turret`

호환용 MountProfileId 후보: `RoofTurret_MediumOrLarge`

| 항목 | 기준 |
|---|---|
| 조준 기준 | 루프 터렛 회전 |
| Aim 요구 | 무기 실제 조준점과 안정화 상태 표시 |
| Fire 검증 | 터렛 회전각, Pitch 제한, 쿨다운 |
| Reticle 상태 | `Ready`, `OutOfArc`, `Cooldown`, `Reloading`, `FirePending` |
| 중량 영향 | 높음 |
| 피격 위험 | 외부 노출 높음 |
| P0 판정 | 느린 각속도가 경량 차량의 회피 기회를 만드는지 확인 |

주의:

- 이 하드포인트는 강력해야 하지만 느려야 한다.
- 경량 차량이 측후면으로 파고들 때 약점이 드러나야 한다.

---

### 3.4 `Top` 또는 `Back` + `Launcher`

호환용 MountProfileId 후보: `RoofOrRearLauncher_Small`

| 항목 | 기준 |
|---|---|
| 조준 기준 | 락온 유지각 / 센서 기준 |
| Aim 요구 | 락온 진행률과 유지 상태 |
| Fire 검증 | 락온 완료, 탄약, 배터리 또는 쿨다운 |
| Reticle 상태 | `NoWeapon`, `FirePending`, `FireRejected`, `Cooldown` |
| 중량 영향 | 중간 |
| 피격 위험 | 외부 노출 가능 |
| P0 판정 | 락온과 대응 장비의 최소 루프 확인 |

주의:

- P0에서 미사일을 완성형 Projectile로 만들 필요는 없다.
- 먼저 락온 상태와 발사 가능 / 불가능 피드백이 맞는지 확인한다.

---

### 3.5 위치 슬롯 + `Utility`

호환용 MountProfileId 후보: `UtilitySlot_01`

| 항목 | 기준 |
|---|---|
| 조준 기준 | 조준 없음 또는 방향성 낮음 |
| Aim 요구 | 없음 또는 최소 |
| Fire 검증 | 사용 가능 상태, 쿨다운, 보유 수량 |
| Reticle 상태 | 필요 시 `Cooldown`, `NoWeapon`, `FireRejected` |
| 중량 영향 | 낮음 |
| 피격 위험 | 내부형이면 낮음, 외부형이면 중간 |
| P0 판정 | 연막 / 플레어가 락온 대응으로 작동하는지 확인 |

주의:

- Utility는 무기보다 방어 대응 장비로 본다.
- Reticle보다 HUD / 상태 표시 쪽이 더 중요할 수 있다.

---

## 4. 하드포인트 데이터 최소 항목

하드포인트 하나는 최소한 아래 값을 설명할 수 있어야 한다.

| 항목 | 설명 |
|---|---|
| `LocationSlotId` | 차량 기준 실제 장착점. 예: `Front_01`, `Front_02`, `Top_01`, `Top_02` |
| `LocationCategory` | 위치 분류. 예: `Front`, `Back`, `LeftSide`, `RightSide`, `Top` |
| `MountProfileId` | 장착 프로파일 식별자. 기존 통합 표기는 호환용 후보로만 사용 |
| `LocationSlotRef` | 이 장착 프로파일이 참조할 위치 슬롯 |
| `MountType` | `Fixed`, `Gimbal`, `Turret`, `Launcher`, `Utility` |
| 장착 크기 제한 | 소형 / 중형 / 대형 |
| 기준 Transform | 발사 위치와 방향의 기준 |
| Yaw 제한 | 좌우 발사각 |
| Pitch 제한 | 상하 발사각 |
| 회전 속도 | 짐벌 / 터렛 추적 속도 |
| 장착 중량 | 총중량 계산 입력 |
| 노출 여부 | 피격 가능성 판단 |
| UI 표시 필요 | Reticle 또는 HUD 표시 여부 |

---

## 5. P0 Transform 기준

하드포인트 위치 작성 방식 비교와 P0 선택안은 `CF_HardpointAuthoring.md`를 따른다.
P0 선택 방식은 Socket / BP Preview로 위치를 잡고, 바퀴 위치 캡처 버튼에 통합된 캡처 실행으로 최종 수치를 DataAsset LocalTransform에 기록하는 4번 방식이다.

`Combined_Body`의 소켓 관리자(Socket Manager)는 사용자 에디터 확인 기준 0 소켓이다.
다만 `Combined_Body`는 임시 감사 자산이며, 최종 P0 차량 메시가 아니다.

따라서 P0 하드포인트 기준은 특정 메시 Socket 존재 여부에 의존하지 않고 아래로 고정한다.

```text
기준 Transform:
  DataAsset 로컬 Transform

수치 소유권:
  차량별 DataAsset이 LocalLocation / LocalRotation 값을 가진다.

Preview:
  StaticMesh Socket 또는 BP SceneComponent를 둘 수는 있지만 규칙 원본으로 쓰지 않는다.
  Preview / Socket 값은 DataAsset LocalTransform으로 캡처한 뒤 확정한다.

금지:
  P0 코드가 Mesh Socket 존재를 필수 조건으로 삼지 않는다.
  최종 차량 메시가 Socket을 갖고 있어도 P0 규칙 원본은 DataAsset Transform이다.
```

DataAsset 위치 슬롯 최소 기록 항목:

| 항목 | 설명 |
|---|---|
| `LocationSlotId` | `Front_01`, `Front_02`, `Top_01`, `Top_02` 같은 위치 슬롯 인스턴스 |
| `LocationCategory` | `Front`, `Back`, `LeftSide`, `RightSide`, `Top` 같은 위치 분류 |
| `LocalLocation` | 차량 로컬 좌표 기준 발사 위치 |
| `LocalRotation` | 차량 로컬 좌표 기준 발사 방향 |
| `SocketName` | 선택 캡처 입력, `HP_Front_01`, `HP_Top_02`처럼 위치 슬롯만 표현. 차량별 설정에 따라 비울 수 있음 |
| `PreviewComponentName` | 선택 항목, BP에서 배치 확인용 컴포넌트를 찾을 때만 사용 |

DataAsset 장착 / 터렛 프로파일 최소 기록 항목:

| 항목 | 설명 |
|---|---|
| `MountProfileId` | 장착 프로파일 식별자. 기존 통합 표기는 호환용 별칭으로만 사용 |
| `LocationSlotRef` | 참조할 위치 슬롯 |
| `MountType` | `Fixed`, `Gimbal`, `Turret`, `Launcher`, `Utility` |
| `MountLocalOffset` | 같은 위치 슬롯 안에서 장착 타입별 세부 오프셋이 필요할 때 사용 |
| `SizeLimit` | 장착 가능한 무기 크기 |
| `Yaw / Pitch 제한` | 발사각 / 조준각 검증 기준 |
| `TurnRate / Stabilization` | 짐벌 / 터렛 추적 성능 |

주의:

```text
LocalLocation / LocalRotation 수치는 차량마다 다르다.
`DA_PoliceCar`는 더 이상 현재 작업에 사용하지 않으므로 P0 최종 Transform 수치 원본으로 쓰지 않는다.
현재 Transform 수치 원본은 `DA_TestSedan`, `DA_TestSUV`의 `HardpointSlots`다.
P0 후보 DataAsset은 `DA_TestSedan`, `DA_TestSUV`이며, LocationCategory / LocationSlotId / MountProfileId 역할은 유지하되 각 차량 메쉬 기준으로 Transform을 별도 작성한다.
한 위치 카테고리에 여러 슬롯 인스턴스가 있을 수 있으므로 `LocationSlotRef`는 항상 `Front_01`, `Top_02`처럼 인스턴스 단위로 기록한다.
```

---

## 6. Fire 검증 순서

P0 Fire 검증은 아래 순서를 따른다.

```text
01. 차량이 전투 가능한 상태인가?
02. 활성 장착 프로파일이 있는가?
03. 활성 무기가 장착되어 있는가?
04. 장착 프로파일이 해당 무기 크기 / MountType을 지원하는가?
05. 조준 방향이 장착 프로파일 발사각 안에 있는가?
06. 사선이 막히지 않았는가?
07. 탄약 / 열 / 쿨다운 / 락온 조건이 맞는가?
08. 발사 위치와 발사 방향을 만들 수 있는가?
09. Fire Command를 실행한다.
10. 결과를 Aim / UI / Debug에 기록한다.
```

---

## 7. Reticle 연결 규칙

Reticle은 하드포인트별로 다른 모양을 먼저 만들지 않는다.

P0에서는 아래 상태만 확실히 표시한다.

| 상태 | 의미 |
|---|---|
| Ready | 현재 하드포인트 / 무기가 발사 가능 |
| OutOfArc | 조준 방향이 발사각 밖 |
| AimBlocked | 사선 차단 |
| NoWeapon | 장착 무기 없음 |
| Cooldown | 쿨다운 중 |
| Reloading | 재장전 중 |
| FirePending | 발사 처리 중 또는 락온 대기 |
| FireRejected | 발사 검증 실패 |

중요:

```text
하드포인트가 Reticle을 결정하는 것이 아니라,
하드포인트와 무기 검증 결과가 Reticle 상태로 표시된다.
```

---

## 8. P0 우선순위

| 우선순위 | 위치 슬롯 인스턴스 | 위치 카테고리 | 장착 타입 | 호환용 MountProfileId 후보 | 이유 |
|---:|---|---|---|---|---|
| 1 | `Front_01` | `Front` | `Fixed` | `FrontFixed_SmallOrMedium` | 차체 조준과 Fire 검증 기본선 |
| 2 | `Top_01` | `Top` | `Turret` | `RoofTurret_MediumOrLarge` | 무기 실제 조준점 / 각속도 / 중량 검증 |
| 확장 후보 | `Front_02` | `Front` | `Gimbal` | `FrontGimbal_Small` | 전방 하드포인트가 늘어날 수 있다는 가능성 |
| 확장 후보 | `Top_02` 또는 `Back_01` | `Top` 또는 `Back` | `Launcher` | `RoofOrRearLauncher_Small` | 락온은 별도 루프가 필요하므로 후순위 |
| 확장 후보 | 미정 슬롯 인스턴스 | 위치별 | `Utility` | `UtilitySlot_01` | 락온 이후 대응 장비로 검증 |

기본 추천:

```text
처음 코드 작업은
  LocationSlot=Front_01 + MountType=Fixed
  LocationSlot=Top_01 + MountType=Turret
두 장착 프로파일만으로 시작한다.

그 뒤 하드포인트가 늘어나는 구조를 검증할 때
LocationSlot=Front_02 + MountType=Gimbal을 확장 후보로 추가할 수 있다.
```

---

## 9. Changelog

### v0.12

- `DA_PoliceCar` 폐기 결정과 `DA_TestSedan` / `DA_TestSUV` 하드포인트 슬롯 저장 상태를 Transform 원본 기준에 반영했다.

### v0.11

- `Front_02 + Gimbal`을 P0 우선순위에서 확장 후보로 낮췄다.
- `SocketName`을 차량별 설정에 따라 비울 수 있는 선택 캡처 입력으로 정리했다.
- P0 첫 코드 작업 기준을 `Front_01 + Fixed`, `Top_01 + Turret` 두 장착 프로파일로 유지했다.

### v0.10

- 하드포인트 위치 작성 방식을 바퀴 위치 캡처 버튼에 통합된 DataAsset 캡처 실행으로 갱신했다.
- `SocketName`을 P0 캡처 입력으로 정리했다.
  이 판단은 v0.11에서 차량별 설정에 따라 비울 수 있는 선택 캡처 입력으로 정정됐다.
- `PreviewComponentName`은 BP Preview용 선택 항목으로 유지했다.

### v0.9

- `Front`, `Top`을 단일 슬롯이 아니라 `LocationCategory`로 정리하고, 실제 참조는 `Front_01`, `Top_01` 같은 `LocationSlotId` 인스턴스로 고정했다.
- 같은 위치 카테고리에 여러 터렛 / 무장을 달 수 있음을 명시하고 `Top_01`, `Top_02`, `Front_01`, `Front_02` 예시를 추가했다.
- P0 우선순위를 `Front_01 + Fixed`, `Top_01 + Turret`, `Front_02 + Gimbal` 기준으로 갱신했다.
  이 판단은 v0.11에서 `Front_02 + Gimbal`을 확장 후보로 낮추는 것으로 정정됐다.

### v0.8

- 하드포인트 규격을 `LocationSlotId`와 `MountType`으로 분리했다.
- 기존 `FrontFixed_SmallOrMedium`, `RoofTurret_MediumOrLarge`, `FrontGimbal_Small` 표기는 MountProfileId 후보 또는 호환용 별칭으로 낮췄다.
- Mesh Socket 이름은 `HP_Front`, `HP_Top`처럼 위치만 표현한다는 기준을 추가했다.

### v0.7

- 하드포인트 위치 작성 방식을 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처로 확정했다.
- P0 Transform 기준에 Preview / Socket 값은 캡처 후 DataAsset LocalTransform으로 확정한다는 문장을 추가했다.

### v0.6

- 하드포인트 위치 작성 방식의 세부 비교를 `CF_HardpointAuthoring.md`로 분리했다.
- StaticMesh Socket과 BP SceneComponent는 작성 보조이며 P0 규칙 원본이 아님을 Transform 기준에 반영했다.

### v0.5

- P0 Transform 수치 원본 후보를 `DA_TestSedan`, `DA_TestSUV`로 갱신했다.
- 하드포인트 ID는 공통 규격으로 유지하고 LocalLocation / LocalRotation은 차량별로 기록한다는 기준을 유지했다.

### v0.4

- `DA_PoliceCar`를 P0 최종 Transform 수치 원본에서 제외했다.
- 하드포인트 Transform 수치 원본을 이후 지정될 P0 차량 1~2종의 DataAsset으로 변경했다.

### v0.3

- 하드포인트 ID는 공통 규격, Transform 수치는 차량별 DataAsset 값으로 분리했다.
- `DA_PoliceCar` 후보 수치가 다른 차종 공통값이 아님을 명시했다.
  이 판단은 v0.4에서 `DA_PoliceCar` 자체를 최종 수치 원본에서 제외하는 것으로 정정됐다.

### v0.2

- `Combined_Body` Socket Manager 0 소켓 확인 결과를 반영했다.
- P0 기준 Transform을 DataAsset 로컬 Transform으로 고정했다.
- DataAsset에 기록할 최소 Transform 항목을 분리했다.

### v0.1

- P0 하드포인트 5종을 전투기능 연결 기준으로 정리했다.
- 하드포인트별 Aim / Fire / Reticle / Weight / Risk 계약을 분리했다.
- P0 구현 우선순위를 `FrontFixed`, `RoofTurret`, `FrontGimbal` 순서로 제안했다.

---

## 10. Migration 메모

- 이 문서는 하드포인트 이름과 역할의 작업 기준이다.
- 실제 C++ struct, enum, DataAsset 필드명은 구현 작업 문서에서 별도로 확정한다.
- CombatPlan SSOT에 반영할 필요가 생기면 검증 후 별도 변경안으로 분리한다.
