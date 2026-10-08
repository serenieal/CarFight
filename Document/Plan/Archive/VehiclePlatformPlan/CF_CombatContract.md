# CarFight - 전투 차량 플랫폼 계약

> 역할: 전투기능이 차량 플랫폼에서 어떤 정보를 읽고, 어떤 책임을 가져야 하는지 정의한다.
> 문서 버전: v0.15
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Contract Before Code / Combat Data Ownership Documented

---

## 1. 목적

이 문서는 코드 설계서가 아니라 구현 계약서다.

목표는 전투기능 구현 전에 아래 경계를 고정하는 것이다.

```text
차량 플랫폼은 무엇을 제공하는가?
Aim / Weapon / Damage / UI / AI는 무엇을 읽는가?
어떤 규칙은 C++가 해석해야 하는가?
Reticle은 어디까지 표시해야 하는가?
```

---

## 2. 기본 소유권

| 계층 | 소유해야 하는 것 | 소유하면 안 되는 것 |
|---|---|---|
| C++ | 규칙, 검증, 적용 순서, 폴백 | 에디터 편의용 임시값 남발 |
| DataAsset | 차종별 값, 하드포인트, 장갑, 모듈 기준 | 계산 로직, 런타임 상태 |
| BP | 컴포넌트 조립, 표현 연결, WBP 배치 | 전투 규칙 원본, 차종 수치 원본 |
| UI | 상태 표시, 힌트, 경고 | 발사 가능 여부 판단 |
| Debug | 관측, 비교, 로그 | 정식 차종 데이터 |

핵심 원칙:

```text
DataAsset은 저장소다.
C++는 해석자다.
BP는 조립 계층이다.
UI는 표시 계층이다.
Debug는 관측 계층이다.
```

세부 데이터 소유권은 `CF_CombatDataOwnership.md`를 따른다.

---

## 3. 최소 플랫폼 데이터 묶음

전투 차량 플랫폼은 P0 기준으로 아래 묶음을 제공해야 한다.

| 데이터 묶음 | 필수 여부 | 설명 |
|---|---:|---|
| CombatIdentity | 필수 | 차급, 전투 역할, 테스트 목적 |
| MovementBase | 필수 | 기본 주행 성향, 중량 페널티 입력 |
| Hardpoints | 필수 | 위치 카테고리, 위치 슬롯 인스턴스, 장착 프로파일, 크기, 조준 방식, 발사각 |
| ArmorProfile | 필수 | 정면 / 측면 / 후면 피해 계수 |
| ModuleProfile | 필수 | 바퀴 / 엔진 / 터렛 손상 기준 |
| WeightModel | 필수 | 기본 중량, 장착 중량, 페널티 방향 |
| ResourceHooks | P0 선택 | 탄약 / 열 / 배터리 / 연료 참조 |
| CombatUiHooks | 필수 | Reticle, 조준점, 손상 표시 참조 |
| AiRoleHint | P0 권장 | 추격형, 중거리형, 중장갑형, 락온형 테스트 힌트 |

---

## 4. 기능별 읽기 계약

### 4.1 Aim

Aim은 차량 플랫폼에서 아래를 읽어야 한다.

| 입력 | 용도 |
|---|---|
| 활성 장착 프로파일 | 어느 무기가 조준 중인지 결정 |
| LocationSlotRef | 발사 위치 기준이 되는 차량별 위치 슬롯 인스턴스. 예: `Front_01`, `Top_02` |
| LocationCategory | UI / Debug 분류용 위치. 예: `Front`, `Top` |
| MountType | 고정 / 짐벌 / 터렛 / 락온 구분 |
| 장착 프로파일 발사각 | `OutOfArc` 판단 |
| 최대 조준 거리 | Aim Trace 거리 제한 |
| 무기 실제 조준점 | 플레이어 조준점과 실제 총구 방향 차이 표시 |

Aim이 하지 말아야 할 것:

- Damage 적용
- 탄약 차감
- 장갑 판정
- Reticle 디자인 결정

---

### 4.2 Weapon / Fire

Weapon / Fire는 차량 플랫폼에서 아래를 읽어야 한다.

| 입력 | 용도 |
|---|---|
| 활성 무기 그룹 | Fire Command가 어떤 무기를 쓰는지 결정 |
| LocationSlot 발사 위치 | HitScan / Projectile 시작점 |
| MountType 적용 후 발사 방향 | 실제 탄도 방향 |
| 무기 크기 제한 | 장착 가능 여부 검증 |
| 자원 상태 | 탄약 / 열 / 배터리 / 쿨다운 검증 |

P0 최소 계약:

```text
Fire Command는 최소한
  활성 장착 프로파일
  활성 무기 그룹
  발사 위치
  발사 방향
  발사 불가 사유
를 설명할 수 있어야 한다.
```

---

### 4.3 Damage

Damage는 차량 플랫폼에서 아래를 읽어야 한다.

| 입력 | 용도 |
|---|---|
| 피격 방향 | 정면 / 측면 / 후면 판정 |
| ArmorProfile | 피해 계수 적용 |
| 차체 HP | 차량 파괴 판정 |
| ModuleProfile | 부품 손상 판정 |
| 하드포인트 노출 여부 | 외부 장비 피격 가능성 판정 |

P0에서 먼저 필요한 것:

1. 피격 방향을 분류할 수 있어야 한다.
2. 방향별 피해 계수를 적용할 수 있어야 한다.
3. 최소한 바퀴 / 엔진 / 터렛 중 어떤 계층이 손상 후보인지 알 수 있어야 한다.

---

### 4.4 Weight

Weight는 차량 플랫폼과 피팅을 연결한다.

| 입력 | 용도 |
|---|---|
| 기본 차체 중량 | 총중량 기준 |
| 무기 장착 중량 | 피팅 비용 |
| 장갑 중량 | 방어력 비용 |
| 탄약 / 연료 / 배터리 중량 | 자원 선택 비용 |
| 중량 페널티 곡선 또는 단계 | 가속 / 제동 / 선회 영향 |

P0에서는 수치 정밀도보다 방향이 중요하다.
현재 주행감은 충분히 나눠진 상태로 보고, 추가 튜닝은 하드포인트 / Fire / UI 연결 이후로 미룬다.

```text
무거운 피팅을 하면
  가속이 둔해지고
  제동이 길어지고
  선회가 무거워지는지
먼저 확인한다.
```

---

### 4.5 UI / Reticle

UI는 차량 플랫폼에서 직접 판단하지 않고, 계산된 상태를 표시한다.

| 표시 대상 | 원본 |
|---|---|
| 플레이어 조준점 | Aim / Camera |
| 무기 실제 조준점 | Weapon / Hardpoint |
| 발사 가능 여부 | Fire Validation |
| 발사 불가 사유 | Aim / Weapon / Vehicle State |
| 탄약 / 열 / 배터리 | Weapon / Resource |
| 손상 상태 | Damage / Module |
| 락온 상태 | Lock-On |

Reticle의 P0 계약:

```text
Reticle은 하드포인트와 무기 상태를 표시한다.
Reticle은 하드포인트와 무기 규칙을 판단하지 않는다.
```

---

### 4.6 AI

AI는 차량 플랫폼을 읽어 역할을 수행해야 한다.

| AI 역할 | 필요한 플랫폼 정보 |
|---|---|
| 추격형 | 속도, 선회, 근접 무기, 약한 정면 장갑 |
| 중거리형 | 중거리 무기, 안정 조준, 평균 장갑 |
| 중장갑형 | 정면 장갑, 대구경 무기, 느린 선회 |
| 락온형 | 센서, 미사일, 플레어 / 연막 대응 |

P0에서는 모든 AI를 완성하지 않는다.

최소 목표는 같은 플랫폼 규격을 AI가 읽을 수 있도록 입력 형태를 막아두는 것이다.

---

## 5. 기존 시스템과의 연결

| 기존 시스템 | 연결 방식 |
|---|---|
| `UCFVehicleData` | 전투 플랫폼 데이터의 저장 위치 후보 |
| `CF_CombatDataOwnership.md` | 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage / UI 상태 데이터 소유권 기준 |
| `ACFVehiclePawn` | 차량 데이터 적용 / 컴포넌트 연결 진입점 |
| `UCFVehicleAimComp` | 하드포인트별 조준각 / ReticleState 확장 지점 |
| `UCFAimReticleWidget` | 상태 표시 전용 위젯 부모 |
| `UCFVehicleDriveComp` | 중량 / 피해에 따른 주행 영향 적용 후보 |
| `UCFWheelSyncComp` | 바퀴 모듈 손상 표시 / 디버그 관측 후보 |
| `VehicleDebug` | Combat Snapshot 추가 후보 |

---

## 6. P0에서 확정할 것과 미룰 것

| 구분 | P0에서 확정 | P0 이후 |
|---|---|---|
| 차량 수 | 1~2종 | 역할 검증 후 확장 |
| 하드포인트 | 5종 정의 | 차종별 변형 |
| 무기 | 4종 + 대응 장비 | 세부 밸런싱 |
| 장갑 | 방향별 계수 | 재질 / 관통 세부 규칙 |
| 모듈 | 바퀴 / 엔진 / 터렛 | 세부 파괴 연출 |
| 중량 | 주행 영향 방향 | 정밀 곡선 |
| Reticle | 상태 표시 | 디자인 / 애니메이션 |
| AI | 역할 입력 형태 | 전술 세부 행동 |

---

## 7. 구현 전 질문

코드 작업 직전에 아래 질문에 답해야 한다.

1. 전투 플랫폼 데이터는 기존 `UCFVehicleData` 안에 확장할 것인가, 별도 Combat DataAsset으로 나눌 것인가?
2. `DA_TestSUV` 수동 전환 테스트 결과를 장착 / 터렛 프로파일 결정에 어떻게 반영할 것인가?
3. `DA_TestSedan` 기본 주행 검증 완료 상태를 전투 장착 기준값의 출발점으로만 사용하고, 추가 주행감 튜닝은 후순위로 둘 것인가?
4. 지정된 세단 / SUV별 위치 슬롯 좌표 수치는 차량 DataAsset에 선언된 슬롯 기준으로 통합 캡처 버튼 또는 직접 입력으로 기록할 것인가?
5. P0 첫 구현은 차량 1종 + 피팅 2세트인가, 차량 2종 + 고정 피팅인가?
6. 무기 그룹은 Pawn이 소유하는가, 별도 Weapon 계층이 소유하는가?
7. Damage는 HitScan 더미 결과에서 바로 시작할 것인가, 무기 계층을 먼저 둘 것인가?

현재 문서 기준 기본 추천:

```text
기본안:
  UCFVehicleData에 전투 플랫폼 데이터를 작게 추가
  전투 데이터 소유권은 CF_CombatDataOwnership.md 기준으로 분리
  P0 1차 구현 범위는 MountProfile + UCFVehicleWeaponComp + FireOrigin
  DA_TestSedan을 현재 P0 기본 테스트 기준으로 사용
  DA_TestSedan 기본 주행 검증 완료
  DA_TestSUV는 수동 VehicleData 전환으로 대조 테스트
  추가 주행감 튜닝은 하드포인트 / Fire / UI 연결 이후로 미룸
  DA_PoliceCar는 현재 작업에 사용하지 않음
  P0 차형은 세단(Sedan) + SUV를 기본안으로 사용
  P0 후보 메시는 Mesh_TestSedan / Mesh_TestSUV 사용
  P0 후보 DataAsset은 DA_TestSedan / DA_TestSUV 사용
  DA_TestSedan은 BP_CFVehiclePawn.VehicleData 기본 연결로 저장 완료
  하드포인트는 DataAsset 정의 + Pawn 적용
  하드포인트 위치 슬롯은 차량 DataAsset에 선언된 항목만 기록
  SocketName이 있는 슬롯은 통합 캡처 버튼으로 읽고, SocketName이 없으면 DataAsset LocalTransform을 직접 작성 / 유지
  Front / Top은 위치 카테고리이고 실제 참조는 숫자 suffix가 붙은 슬롯 인스턴스가 담당
  같은 위치 카테고리에 여러 터렛 / 무장이 달릴 수 있음
  장착 타입은 DA 장착 / 터렛 섹션의 MountType으로 분리
  Front_01 + Fixed, Top_01 + Turret 순서로 먼저 검증
  Front_02 + Gimbal은 전방 하드포인트가 늘어날 수 있다는 확장 가능성으로만 유지
  Reticle은 기존 위젯에 상태만 확장
  Damage는 더미 HitScan 결과에서 최소 방향별 피해로 시작
```

---

## 8. Changelog

### v0.15

- `CF_CombatDataOwnership.md`를 전투 데이터 소유권 기준 문서로 연결했다.
- P0 1차 구현 범위를 `MountProfile + UCFVehicleWeaponComp + FireOrigin`으로 제한한다는 기본 추천을 추가했다.

### v0.14

- 주행감 추가 튜닝을 가까운 핵심 작업에서 제외했다.
- 전투 계약의 다음 초점을 하드포인트 / Fire / UI 연결로 유지했다.

### v0.13

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 전투 계약의 기본안에 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.12

- 전투 계약의 현재 기준 DataAsset을 `DA_TestSedan`으로 갱신하고, `DA_PoliceCar`를 현재 작업에서 제외했다.
- `DA_TestSUV`는 대조 테스트 후보로 유지했다.

### v0.11

- `Front_02 + Gimbal`을 구현 전 결정 항목이 아니라 하드포인트 확장 가능성으로 낮췄다.
- 하드포인트 소켓은 차량 설정에 따라 있을 수도 있고 없을 수도 있음을 기본 추천에 반영했다.
- 위치 슬롯 좌표 기록 방식을 선언된 DataAsset 슬롯 기준으로 정리했다.

### v0.10

- 위치 슬롯 좌표 수치 질문을 수동값 선택이 아니라 `HP_` prefix 리네임 후 통합 캡처 버튼 기록 문제로 갱신했다.
- 기본 추천에 `HP_Front_01`, `HP_Top_01`, `HP_Top_02` 소켓을 DataAsset LocalTransform으로 캡처하는 흐름을 반영했다.

### v0.9

- 위치 슬롯을 단일 `Front`, `Top` 값으로 보지 않고 `LocationCategory`와 `LocationSlotRef` 인스턴스로 분리했다.
- 같은 위치 카테고리에 여러 터렛 / 무장이 달릴 수 있음을 구현 전 질문과 기본 추천에 반영했다.
- P0 검증 순서를 `Front_01 + Fixed`, `Top_01 + Turret`, `Front_02 + Gimbal`로 갱신했다.
  이 판단은 v0.11에서 `Front_02 + Gimbal`을 확장 가능성으로 낮추는 것으로 정정됐다.

### v0.8

- Aim / Weapon / Fire 읽기 계약을 위치 슬롯과 장착 프로파일 기준으로 갱신했다.
- 구현 전 질문과 기본 추천에서 `FrontFixed`, `RoofTurret`, `FrontGimbal` 결합형 표현을 `Front + Fixed`, `Top + Turret`, `Front + Gimbal` 검증 순서로 바꿨다.

### v0.7

- P0 후보 DataAsset을 `DA_TestSedan`, `DA_TestSUV`로 반영했다.
- 구현 전 질문을 DataAsset 지정 여부가 아니라 VehicleData 전환 방식과 `DA_TestSedan` 보강 여부로 갱신했다.

### v0.6

- 임시 P0 메시로 `Mesh_TestSedan`, `Mesh_TestSUV`를 기록했다.
- 구현 전 질문을 세단 / SUV 차량 DataAsset 지정으로 좁혔다.

### v0.5

- P0 차형 기본안을 세단(Sedan) + SUV로 기록했다.
- 구현 전 질문을 세단 / SUV 실제 메시와 DataAsset 지정으로 좁혔다.

### v0.4

- `DA_PoliceCar`를 P0 최종 차량 후보에서 제외하고 임시 감사 기준으로 정리했다.
- 구현 전 질문에 P0 대상 차량 1~2종 지정 항목을 추가했다.

### v0.3

- 하드포인트 ID와 차량별 Transform 수치 소유권을 분리했다.
- P0 첫 좌표 결정 대상을 `DA_PoliceCar`로 명확히 했다.
  이 판단은 v0.4에서 P0 대상 차량 1~2종 기준으로 정정됐다.

### v0.2

- 하드포인트 좌표 기준을 P0에서 DataAsset 로컬 Transform으로 정리했다.
- 구현 전 질문에서 Transform 기준 선택 항목을 제거하고 실제 좌표 수치 결정으로 좁혔다.

### v0.1

- 전투 차량 플랫폼의 계층별 소유권을 정의했다.
- Aim / Weapon / Damage / Weight / UI / AI의 읽기 계약을 분리했다.
- Reticle을 판단 계층이 아니라 표시 계층으로 고정했다.

---

## 9. Migration 메모

- 이 문서는 코드 필드명을 확정하지 않는다.
- 구현 시 기존 함수 시그니처와 파일 구조 변경은 별도 작업 문서에서 명시해야 한다.
- 전투 데이터가 `UCFVehicleData`에 추가될 경우 기존 DataPlan과 충돌하지 않도록 변경안을 먼저 작성한다.
