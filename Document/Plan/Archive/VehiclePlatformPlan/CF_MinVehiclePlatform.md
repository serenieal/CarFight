# CarFight - 최소 차량 플랫폼 규격

> 역할: P0 전투기능을 붙일 수 있는 최소 차량 플랫폼 규격을 정의한다.
> 문서 버전: v0.16
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Single Player Planning / Initial Layout Capture Implemented / Combat Link Priority

---

## 1. 목적

이 문서는 차량 종류 확장 계획서가 아니다.

이 문서는 전투기능을 구현하기 전에 차량 플랫폼이 최소한 어떤 정보를 가져야 하는지 정리한다.

목표:

```text
전투기능이 참조할 수 있는 차량 플랫폼 규격을 먼저 고정한다.
그 뒤 P0 차량 1~2종으로 조준, 사격, 피해, 모듈, 중량, UI 연동을 검증한다.
차량 종류 확장은 검증 이후로 미룬다.
```

---

## 2. 현재 기준

CombatPlan SSOT 기준으로 현재 확정된 방향은 다음과 같다.

1. 현재 전투 기준은 싱글플레이 AI 교전이다.
2. P0는 완성형 전투가 아니라 방향성 검증이다.
3. P0에서는 차량을 많이 늘리지 않고 차체 1~2종과 피팅 차이로 감각을 확인한다.
4. 차량은 클래스가 아니라 플랫폼이다.
5. 하드포인트는 위치 슬롯과 장착 타입을 분리해 전투 방향성을 결정하는 구조다.
6. 중량은 차량 피팅 선택의 핵심 비용이다.
7. 조준점과 무기 실제 조준점은 분리된다.
8. 레티클은 상세 디자인이 아니라 전투 상태 피드백으로 접근한다.

---

## 3. 최소 차량 플랫폼 정의

최소 차량 플랫폼은 아래 정보를 함께 가진 전투기능의 기준 단위다.

```text
차량 플랫폼
  = 차체
  + 주행 기준값
  + 하드포인트
  + 장갑 방향
  + 모듈 기준
  + 중량 계산 입력
  + 전투 UI 참조 정보
  + AI 역할 연결 정보
```

차량 플랫폼은 단순히 메쉬와 속도만 가진 데이터가 아니다.

전투기능 입장에서 차량 플랫폼은 아래 질문에 답해야 한다.

1. 이 차량은 어떤 무기를 어디에 달 수 있는가?
2. 이 차량은 어떤 방향을 적에게 보여줘야 유리한가?
3. 이 차량은 어떤 모듈이 손상될 수 있는가?
4. 이 차량은 중량 증가를 전투 피팅 비용으로 어떻게 기록할 것인가?
5. 이 차량은 어떤 AI 역할로 테스트할 수 있는가?
6. 이 차량에서 UI는 어떤 조준 / 손상 / 자원 정보를 보여줘야 하는가?

---

## 4. P0 차량 플랫폼 범위

P0에서는 차량 종류를 많이 늘리지 않는다.

권장 P0 플랫폼:

| 플랫폼 | 목적 | 이유 |
|---|---|---|
| 세단(Sedan) | 낮은 차체, 평범한 기준 주행, 전방 고정 / 소형 짐벌 검증 | 가장 기본적인 승용차 기준으로 조준 / 사격 / 피팅 차이를 확인 |
| SUV | 높은 차체, 무게감, 루프 장착 / 중량 부담 검증 | 세단보다 높은 장착 위치와 무거운 무기 운용 감각을 비교 |

대안:

| 대안 | 조건 |
|---|---|
| 차량 1종 + 피팅 2세트 | 구현 범위를 더 줄여야 할 때 |
| 차량 2종 + 고정 피팅 | 차량 차급 차이를 먼저 보고 싶을 때 |

기본 추천은 `세단 + SUV` 또는 구현 범위를 줄인 `세단 1종 + 피팅 2세트`다.

`DA_PoliceCar`는 더 이상 현재 작업에 사용하지 않는다.
P0 후보 차량 DataAsset은 `DA_TestSedan`, `DA_TestSUV`로 지정되어 AssetDump 확인까지 완료했다.
현재 `BP_CFVehiclePawn.VehicleData` 기본 연결은 `DA_TestSedan`으로 저장 완료됐으며, `DA_TestSUV`는 대조 테스트 후보로 유지한다.
`DA_TestSedan` 기준 기본 주행 검증은 완료됐다.
`DA_TestSUV` 대조 테스트는 수동 VehicleData 전환으로 확인한다.
주행감은 현재 충분히 나눠진 상태로 보고, 추가 튜닝은 하드포인트 / Fire / UI 연결 이후로 미룬다.

P0 후보 메시 / DataAsset:

| 차형 | DataAsset | 메시 | 파일 경로 |
|---|---|---|---|
| 세단(Sedan) | `DA_TestSedan` | `Mesh_TestSedan` | `UE/Content/CarFight/Vehicles/TestSedan/Mesh_TestSedan.uasset` |
| SUV | `DA_TestSUV` | `Mesh_TestSUV` | `UE/Content/CarFight/Vehicles/TestSUV/Mesh_TestSUV.uasset` |

---

## 5. 최소 데이터 항목

차량 플랫폼은 최소한 아래 데이터 묶음을 가져야 한다.

| 데이터 묶음 | 역할 | P0 필요 |
|---|---|---:|
| Identity | 차량 식별, 차급, 전투 역할 | 필수 |
| Chassis | 차체 메쉬, 차체 기준 위치, 기본 크기 | 필수 |
| MovementBase | 기본 가속, 제동, 선회, 최고속도 방향 | 필수 |
| WeightModel | 기본 중량, 최대 권장 중량, 중량 페널티 방향 | 필수 |
| Hardpoints | 무기 장착 위치와 조준 방식 제한 | 필수 |
| Armor | 정면 / 측면 / 후면 피해 계수와 장갑 타입 | 필수 |
| Modules | 바퀴 / 엔진 / 터렛 손상 기준 | 필수 |
| ResourceHooks | 탄약 / 연료 / 배터리 / 무기 열 참조 지점 | 필수 |
| UiHooks | 조준점, 무기 실제 조준점, 레티클 상태, 손상 표시 | 필수 |
| AiRole | 테스트 가능한 AI 역할 | 권장 |

---

## 6. 하드포인트 최소 규격

P0 하드포인트는 CombatPlan의 5종을 기준으로 한다.

| LocationSlotRef | LocationCategory | MountType | 호환용 MountProfileId 후보 | 지원 무기 | 목적 |
|---|---|---|---|---|---|
| `Front_01` | `Front` | `Fixed` | `FrontFixed_SmallOrMedium` | 고정 오토캐논 | 차체 조준과 운전 실력 검증 |
| `Front_02` | `Front` | `Gimbal` | `FrontGimbal_Small` | 소형 짐벌 기관총 | 빠른 목표 대응 검증 |
| `Top_01` | `Top` | `Turret` | `RoofTurret_MediumOrLarge` | 대구경 저속 터렛포 | 느린 각속도와 안정화 검증 |
| `Top_02` 또는 `Back_01` | `Top` 또는 `Back` | `Launcher` | `RoofOrRearLauncher_Small` | 경량 락온 미사일 | 락온 / 배터리 / 대응 검증 |
| 미정 슬롯 인스턴스 | 위치별 | `Utility` | `UtilitySlot_01` | 연막 또는 플레어 | 락온 대응 검증 |

하드포인트 하나는 최소한 아래 정보를 가져야 한다.

| 항목 | 설명 |
|---|---|
| `LocationCategory` | `Front`, `Back`, `LeftSide`, `RightSide`, `Top` 같은 위치 분류 |
| `LocationSlotId` | `Front_01`, `Front_02`, `Top_01`, `Top_02` 같은 실제 장착 기준점 |
| `MountProfileId` | 장착 프로파일 식별자. 기존 통합 표기는 호환용 후보 |
| `LocationSlotRef` | 장착 프로파일이 참조하는 위치 슬롯 |
| `MountType` | `Fixed`, `Gimbal`, `Turret`, `Launcher`, `Utility` |
| 크기 제한 | 소형, 중형, 대형 |
| 발사 가능 각도 | Yaw / Pitch 제한 또는 고정 방향 |
| 기준 Transform | 좌표계는 DataAsset 로컬 Transform, 실제 수치는 차량별 DataAsset 값 |
| 장착 중량 | 총중량에 반영할 장착 비용 |
| 피격 위험 | 외부 노출 장비인지 여부 |
| UI 참조 | 레티클 / 실제 조준점 표시가 필요한지 여부 |

P0 기준:

```text
현재 임시 감사 자산인 Combined_Body에는 Static Mesh Socket이 없다.
P0 후보 메시인 Mesh_TestSedan / Mesh_TestSUV에는 위치 작성 보조 Socket 후보가 있다.
최종 P0 차량 메시가 무엇이든 P0 하드포인트는 Mesh Socket 필수 기준이 아니라 DataAsset 로컬 Transform 기준으로 시작한다.
BP SceneComponent는 배치 확인용 Preview로만 사용한다.
Mesh Socket을 둘 경우 이름은 `HP_Front_01`, `HP_Top_01`처럼 위치 슬롯 인스턴스만 표현한다.
단, 2026-06-25 AssetDump 당시 실제 테스트 메시 SocketName은 `Front_01`, `Top_01`, `Top_02`처럼 `HP_` prefix가 없었다.
이 소켓을 캡처 입력으로 계속 사용할 경우 `HP_Front_01`, `HP_Top_01`, `HP_Top_02`로 리네임한다.
차량마다 하드포인트 소켓은 해당 차량 설정상 있을 수도 있고 없을 수도 있다.
Fixed / Gimbal / Turret 같은 장착 타입은 DA 장착 / 터렛 섹션에서 기록한다.
같은 `Front`나 `Top` 위치 카테고리에 여러 슬롯이 있을 수 있다.
예를 들어 전방 2문 무장은 `Front_01`, `Front_02`, 루프 2터렛은 `Top_01`, `Top_02`로 기록한다.
LocationCategory / LocationSlotId와 MountType 계약은 공통 규격이지만, 위치 / 회전 수치는 차량마다 별도로 기록한다.
```

---

## 7. 전투기능 연동 항목

차량 플랫폼은 전투기능에 아래 정보를 제공해야 한다.

| 전투기능 | 차량 플랫폼에서 필요한 정보 |
|---|---|
| 조준 | 카메라 기준 조준점, 장착 프로파일 발사각, 무기 실제 조준점 |
| 사격 | 무기 그룹, 발사 위치, 발사 방향, 탄종 / 탄수 |
| 피해 | 피격 방향, 장갑 타입, 차체 HP, 모듈 위치 |
| 모듈 손상 | 바퀴 / 엔진 / 터렛의 손상 단계와 페널티 |
| 중량 | 장갑 / 무기 / 탄약 / 연료 / 배터리 총합 |
| 락온 | 센서 기준 위치, 락온 유지각, 연막 / 플레어 슬롯 |
| UI | 레티클 상태, 열, 탄약, 배터리, 연료, 손상 표시 |
| AI | 거리 유지, 추격, 압박, 락온 등 역할 수행 기준 |

---

## 8. 레티클 연동 범위

레티클은 최소 차량 플랫폼 규격의 중심 항목이 아니다.

다만 전투기능 검증을 위해 아래 정보는 차량 / 하드포인트 / 무기에서 제공되어야 한다.

| 레티클 참조 정보 | 제공 주체 | 목적 |
|---|---|---|
| 플레이어 조준점 | Aim / Camera | 플레이어가 명령하는 방향 표시 |
| 무기 실제 조준점 | Weapon / Hardpoint | 무기가 실제 바라보는 방향 표시 |
| 조준 가능 범위 | Hardpoint / Weapon | OutOfArc 상태 표시 |
| 안정화 상태 | Weapon | 지금 쏴도 되는지 표시 |
| 락온 진행률 | Lock-On | 미사일 발사 가능 상태 표시 |
| 사격 불가 사유 | Weapon / Combat | 탄약 부족, 과열, 차단 상태 표시 |

본 문서는 레티클 모양, 색, 애니메이션, 최종 HUD 배치를 결정하지 않는다.

---

## 9. P0 검증 기준

이 규격은 아래가 확인되면 P0 기준으로 충분하다.

1. 하드포인트 위치에 따라 같은 무기라도 발사각과 사각이 달라진다.
2. 장착 / 터렛 / Fire 명령이 최소 전투 루프로 연결된다.
3. UI가 조준 가능 여부, 무기 실제 조준점, 레티클 상태, 손상 상태를 표시할 수 있다.
4. 장갑 / 탄약 / 연료 / 배터리 / 무기 중량이 총중량에 반영된다.
5. 경량 테스트 차량이 느린 터렛의 각속도 한계를 만들 수 있다.
6. 중형 또는 중량 테스트 차량이 정면 장갑으로 압박할 수 있다.
7. 바퀴 / 엔진 / 터렛 손상이 전투 흐름을 바꾼다.
8. AI 역할 4종 중 최소 2종이 같은 플랫폼 규격을 읽어 동작할 수 있다.

---

## 10. P1 차량 종류 확장 조건

차량 종류 확장은 아래 조건 이후에 진행한다.

| 조건 | 이유 |
|---|---|
| P0 하드포인트 5종 중 최소 3종이 동작 | 차량별 무장 차이를 검증할 기준 필요 |
| 조준 / 사격 / 피해 / 모듈 손상 최소 루프 동작 | 차량 차이가 전투 결과로 드러나야 함 |
| 중량이 전투 피팅 비용으로 반영 | 피팅과 차종 차이의 비용 확인 필요 |
| 레티클 / 디버그 UI가 발사 불가 이유를 표시 | 판정 불신 방지 |
| 테스트 차량 1~2종에서 역할 차이 확인 | 역할 차이 없는 차량 증식 방지 |

---

## 11. 아직 정하지 않은 것

| 항목 | 상태 |
|---|---|
| 최종 차량 종류 목록 | 아직 정하지 않음 |
| P0 대상 차형 | 세단(Sedan) + SUV 기본안 |
| P0 대상 메시 | 임시 메시 `Mesh_TestSedan`, `Mesh_TestSUV` 지정됨 |
| P0 대상 DataAsset | `DA_TestSedan`, `DA_TestSUV` 지정됨, 하드포인트 슬롯 저장 완료. 기본 연결은 `DA_TestSedan`, SUV 대조는 수동 전환 |
| P0 메시 Socket 후보 | 현재 `Mesh_TestSedan`: `HP_Front_01`, `HP_Top_01`; `Mesh_TestSUV`: `HP_Front_01`, `HP_Top_01`, `HP_Top_02` |
| 최종 차급별 수치 | P0 테스트 후 조정 |
| 세부 하드포인트 좌표 | 차량 DataAsset에 선언된 슬롯 기준으로 `DA_TestSedan`, `DA_TestSUV`에 LocalTransform 저장 완료 |
| DA 장착 / 터렛 섹션 | LocationSlotRef와 MountType 분리 필요 |
| 레티클 상세 디자인 | 별도 UI 문서에서 결정 |
| 최종 DataAsset 필드명 | 구현 문서에서 결정 |
| P1 추가 차종 개수 | P0 결과 후 결정 |

---

## 12. Changelog

### v0.16

- 주행감은 현재 충분히 나눠진 상태로 보고 추가 튜닝을 후순위로 낮췄다.
- P0 검증 기준의 첫 초점을 주행감 차이가 아니라 하드포인트 / Fire / UI 연결로 조정했다.

### v0.15

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.14

- `DA_PoliceCar` 폐기 결정과 `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 상태를 현재 플랫폼 규격에 반영했다.
- P0 기본 테스트 DA를 `DA_TestSedan`, 대조 후보를 `DA_TestSUV`로 명확히 했다.

### v0.13

- 차량 레이아웃 통합 캡처 구현 후 P0 대상 DataAsset과 `HP_*` 메시 소켓의 현재 저장 상태를 반영했다.
- 세부 하드포인트 좌표 항목을 기록 필요 상태에서 `DA_TestSedan` / `DA_TestSUV` 저장 완료 상태로 갱신했다.

### v0.12

- 하드포인트 소켓이 모든 차량에 필수로 존재해야 한다는 해석을 제거했다.
- `HP_Front_02` / `Front_02 + Gimbal`은 전방 하드포인트 확장 가능성으로만 다루도록 정리했다.
- 세부 하드포인트 좌표 기록 기준을 차량 DataAsset에 선언된 슬롯 기준으로 갱신했다.

### v0.11

- 실제 테스트 메시 SocketName을 `HP_` prefix 표준명으로 리네임해야 한다고 확정했다.
- 세부 하드포인트 좌표 기록 방식을 바퀴 위치 캡처 버튼에 통합된 DataAsset LocalTransform 캡처로 갱신했다.
- P0 메시 Socket 후보 표에서 감사 당시 이름과 코드 착수 표준명을 분리했다.

### v0.10

- `CF_MeshSocketAudit.md` 기준 P0 후보 메시의 하드포인트 작성 보조 Socket 후보를 반영했다.
- 현재 실제 SocketName이 `HP_` prefix 없이 위치 슬롯만 표현하므로 코드 착수 전 명명 결정이 필요함을 추가했다.
- 세부 하드포인트 좌표 상태를 Socket 후보 확인 / DataAsset 캡처 확정 필요로 갱신했다.

### v0.9

- 같은 위치 카테고리에 여러 무장 / 터렛이 달릴 수 있음을 최소 플랫폼 규격에 반영했다.
- `Front`, `Top`을 `LocationCategory`로 낮추고 실제 참조 단위를 `Front_01`, `Top_01` 같은 `LocationSlotId` 인스턴스로 갱신했다.
- P0 예시를 `Front_01 + Fixed`, `Front_02 + Gimbal`, `Top_01 + Turret` 기준으로 정리했다.
  이 판단은 v0.12에서 `Front_02 + Gimbal`을 확장 가능성으로 낮추는 것으로 정정됐다.

### v0.8

- 하드포인트 최소 규격을 `LocationSlotId`, `LocationSlotRef`, `MountType` 분리 구조로 정리했다.
- 기존 결합형 하드포인트 표기는 MountProfileId 후보 또는 호환용 별칭으로 낮췄다.
- Mesh Socket 이름은 위치만 표현하고 장착 타입은 DA 장착 / 터렛 섹션에서 기록한다는 기준을 추가했다.

### v0.7

- P0 후보 DataAsset을 `DA_TestSedan`, `DA_TestSUV`로 갱신했다.
- P0 후보 메시 경로를 AssetDump 기준 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV`로 정정했다.
- `BP_CFVehiclePawn.VehicleData` 기본 연결은 아직 `DA_PoliceCar`이므로 테스트 연결 방식 결정이 남았음을 명시했다.

### v0.6

- 임시 P0 메시로 `Mesh_TestSedan`, `Mesh_TestSUV`를 기록했다.
- 메시 지정 완료와 차량 DataAsset 미정을 분리했다.

### v0.5

- P0 대상 차형 기본안을 세단(Sedan) + SUV로 기록했다.
- 실제 차량 메시 / DataAsset 지정은 아직 남은 작업으로 분리했다.

### v0.4

- `DA_PoliceCar`를 P0 대상 차량에서 제외하고 임시 파이프라인 확인 기준으로 정리했다.
- P0 대상 차량 1~2종은 이후 실제 차량 메시 / DataAsset으로 지정한다고 명시했다.

### v0.3

- Transform 좌표계와 차량별 수치 소유권을 분리했다.
- `DA_PoliceCar` 수치가 전체 차량 공통값이 아님을 명확히 했다.
  이 판단은 후속 문서에서 `DA_PoliceCar` 자체를 최종 P0 수치 원본에서 제외하는 것으로 정정됐다.

### v0.2

- P0 하드포인트 기준 Transform을 DataAsset 로컬 Transform으로 명시했다.
- `Combined_Body` Socket 없음 확인 결과를 최소 플랫폼 규격에 반영했다.
- 미정 항목을 Transform 기준이 아니라 실제 좌표 수치로 좁혔다.

### v0.1

- 최소 차량 플랫폼 규격 초안 작성.
- P0 차량 범위를 차량 1~2종과 피팅 차이 검증으로 제한.
- P0 하드포인트 5종을 전투기능 연동 기준으로 정리.
- 레티클을 하위 UI 연동 항목으로만 정의.

---

## 13. Migration 메모

- 이 문서는 CombatPlan SSOT를 수정하지 않는다.
- 기존 `DataPlan`의 `UCFVehicleData` 방향과 충돌하지 않도록, 실제 C++ 필드명 확정은 구현 단계 문서로 넘긴다.
- `ProjectSSOT/CombatPlan`에 반영할 내용은 P0 규격 검증 후 별도 변경안으로 분리한다.
