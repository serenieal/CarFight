# CarFight - Vehicle Platform 검증 체크리스트

> 역할: 최소 차량 플랫폼 규격이 P0 전투기능을 받을 준비가 되었는지 확인한다.
> 문서 버전: v0.23
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Single Player Planning / Combat Data Ownership Documented

---

## 1. 목적

이 체크리스트는 차량 종류 확장 전에 확인해야 할 최소 조건을 정리한다.

체크리스트를 통과하지 못하면 차량 종류를 늘리지 않는다.

---

## 2. 문서 경계 체크

| 체크 | 기준 | 상태 |
|---|---|---|
| CombatPlan은 기획 SSOT로 유지 | 작업 초안이 들어가지 않음 | Done |
| VehiclePlatformPlan은 작업 문서로 분리 | 실행 계획 / 규격 / 체크리스트 저장 | Done |
| 레티클이 중심 주제가 아님 | UI 연동 항목으로만 기록 | Done |
| 차량 확장이 P0 이후 판단으로 남음 | P0는 1~2종 기준 | Done |
| 현재 기준선 문서가 있음 | `CF_PlatformBaseline.md` | Done |
| 에디터 자산 감사 문서가 있음 | `CF_EditorAssetAudit.md` | Done |
| AssetDump 확인 결과 문서가 있음 | `CF_AssetDumpResult.md` | Done |
| 에디터 확인 가이드가 있음 | `CF_EditorCheckGuide.md` | Done |
| 하드포인트 작성 방식 문서가 있음 | `CF_HardpointAuthoring.md` | Done |
| 전투 데이터 소유권 문서가 있음 | `CF_CombatDataOwnership.md` | Done |
| 코드 착수 게이트가 있음 | `CF_CodeStartGate.md` | Done |

---

## 3. 차량 플랫폼 데이터 체크

| 체크 | 기준 | 상태 |
|---|---|---|
| 차량 ID가 있다 | 차량 식별 가능 | Pending |
| 차급이 있다 | 경량 / 중형 / 중량 중 하나 | Pending |
| 전투 역할이 있다 | 추격, 중거리, 압박, 락온 등 | Pending |
| 기본 주행 방향이 있다 | 가속 / 제동 / 선회 / 최고속도 | Pending |
| 기본 중량이 있다 | 총중량 계산 기준 | Pending |
| 중량 페널티 방향이 있다 | 가속 / 제동 / 선회 영향 | Pending |
| 장갑 방향이 있다 | 정면 / 측면 / 후면 | Pending |
| 모듈 기준이 있다 | 바퀴 / 엔진 / 터렛 | Pending |

---

## 4. 하드포인트 체크

| 체크 | 기준 | 상태 |
|---|---|---|
| `Front_01 + Fixed` 장착 프로파일이 있다 | 고정 오토캐논 검증 | Pending |
| `Top_01 + Turret` 장착 프로파일이 있다 | 대구경 저속 터렛 검증 | Pending |
| `Front_02 + Gimbal`은 확장 후보로만 문서화됨 | 전방 하드포인트가 늘어날 수 있다는 가능성 | Done |
| `Top_02 또는 Back_01 + Launcher` 장착 프로파일이 있다 | 경량 락온 미사일 검증 | Pending |
| `위치 슬롯 + Utility` 장착 프로파일이 있다 | 연막 또는 플레어 검증 | Pending |
| 위치 슬롯과 장착 타입이 분리되어 있다 | `LocationSlotId` / `LocationSlotRef` / `MountType` 분리 | Done |
| 같은 위치 카테고리에 복수 슬롯을 둘 수 있다 | `Front_01`, `Front_02`, `Top_01`, `Top_02` 허용 | Done |
| Mesh Socket 이름이 위치 슬롯만 표현한다 | `HP_Front_01`처럼 타입 없는 위치 슬롯 중심 이름 | Done |
| P0 메시 Socket 후보가 감사됨 | `Mesh_TestSedan`, `Mesh_TestSUV` 소켓 목록 확인 | Done |
| 실제 SocketName prefix 정책이 확정됨 | `HP_` prefix 리네임 | Done |
| 차량별 하드포인트 소켓 존재 정책이 정리됨 | 소켓은 차량 설정에 따라 있을 수도 있고 없을 수도 있음 | Done |
| 하드포인트마다 크기 제한이 있다 | 소형 / 중형 / 대형 | Pending |
| 하드포인트마다 발사각이 있다 | 고정 방향 또는 Yaw / Pitch 제한 | Pending |
| 하드포인트 중량이 총중량에 반영된다 | 피팅 비용 검증 | Pending |

---

## 5. 전투기능 연동 체크

| 체크 | 기준 | 상태 |
|---|---|---|
| Aim이 장착 프로파일 발사각을 읽을 수 있다 | OutOfArc 판단 가능 | Pending |
| Weapon이 발사 위치를 읽을 수 있다 | Muzzle / Hardpoint 위치 사용 | Pending |
| Damage가 피격 방향을 판정할 수 있다 | 정면 / 측면 / 후면 | Pending |
| ModuleDamage가 모듈을 찾을 수 있다 | 바퀴 / 엔진 / 터렛 | Pending |
| WeightModel이 주행 성능에 반영된다 | 무거우면 둔해짐 | Pending |
| Lock-On이 센서 / 런처 조건을 읽는다 | 락온 미사일 조건 | Pending |
| UI가 조준 상태를 표시할 수 있다 | 레티클 / 실제 조준점 | Pending |
| AI가 플랫폼 역할을 읽을 수 있다 | 추격 / 중거리 / 압박 / 락온 | Pending |

---

## 6. 코드 착수 게이트 체크

| 체크 | 기준 | 상태 |
|---|---|---|
| 현재 문서 준비 상태가 정리됨 | 설계 가능 / 코드 미착수 구분 | Done |
| 에디터 확인 항목이 분리됨 | 기준 BP / DataAsset / Mesh Socket 후보 / Transform 수치 | Done |
| 사용자가 따라갈 에디터 확인 순서가 있음 | `CF_EditorCheckGuide.md` | Done |
| 파일 시스템 기준 자산이 확인됨 | BP / DataAsset / Reticle / Mesh 존재 | Done |
| AssetDump 기준 핵심 연결이 확인됨 | BP / DataAsset / Reticle / Fire 입력 | Done |
| VehicleDebug 연결 상태가 분리됨 | 파일 존재와 BP 기본 참조를 분리 | Done |
| P0가 Mesh Socket에 의존하지 않음 | DataAsset 로컬 Transform 기준 | Done |
| 하드포인트 Transform 기준이 결정됨 | DataAsset 로컬 Transform 기준 | Done |
| 하드포인트 위치 작성 방식이 결정됨 | 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처 | Done |
| 하드포인트 캡처 자동화 방식이 결정됨 | 바퀴 위치 캡처 버튼에 하드포인트 캡처 통합 | Done |
| 통합 캡처 코드 작업 계획이 있음 | `CF_SocketCaptureCodePlan.md` | Done |
| 하드포인트 위치 / 타입 분리 기준이 결정됨 | Socket / Preview는 위치 슬롯 인스턴스, DA 장착 / 터렛 섹션은 `MountType` | Done |
| 같은 위치 카테고리의 복수 슬롯 기준이 결정됨 | `Front_01`, `Front_02`, `Top_01`, `Top_02` 인스턴스 허용 | Done |
| P0 대상 차형이 지정됨 | 세단(Sedan) + SUV | Done |
| P0 대상 메시가 지정됨 | `Mesh_TestSedan`, `Mesh_TestSUV` | Done |
| P0 대상 DataAsset이 지정됨 | `DA_TestSedan`, `DA_TestSUV` | Done |
| P0 대상 DataAsset 설정 차이가 감사됨 | `DA_TestSedan`, `DA_TestSUV` 하드포인트 슬롯 저장 완료 | Done |
| 세단 DataAsset 보강 여부가 결정됨 | WheelClass / Movement / WheelVisual / DriveState / Layout / HardpointSlots 저장 완료 | Done |
| P0 테스트 VehicleData 기본 연결이 결정됨 | `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 완료 | Done |
| 세단 기본 주행 검증이 완료됨 | 사용자 PIE 주행 검증 완료 | Done |
| SUV 대조 테스트 방식이 결정됨 | 별도 자동화 없이 수동 VehicleData 전환 | Done |
| 주행감 세분화가 충분함 | 현재 단계에서는 더 파지 않음 | Done |
| 하드포인트 Transform 수치가 기록됨 | `DA_TestSedan`, `DA_TestSUV`의 선언 슬롯 기준 LocalTransform 저장 완료 | Done |
| 전투 데이터 소유권 기준이 결정됨 | P0 1차는 `MountProfile + UCFVehicleWeaponComp + FireOrigin` | Done |
| 첫 코드 작업 후보가 작게 잘림 | 장착 프로파일 1종 + WeaponComp 골격 + FireOrigin | Done |
| 보류 작업이 명확함 | 차량 대량 추가 / 상세 Reticle / 락온 완성 | Done |
| 사용자가 코드 작업 시작을 승인함 | 별도 요청 필요 | Pending |

---

## 7. 레티클 연동 체크

레티클은 별도 주제가 아니라 전투기능 검증용 표시 항목이다.

| 체크 | 기준 | 상태 |
|---|---|---|
| 플레이어 조준점 표시 가능 | Camera / Aim 기준 | Pending |
| 무기 실제 조준점 표시 가능 | Weapon / Hardpoint 기준 | Pending |
| 조준 가능 범위 표시 가능 | OutOfArc 피드백 | Pending |
| 안정화 상태 표시 가능 | 지금 쏴도 되는지 판단 | Pending |
| 락온 진행률 표시 가능 | 미사일 발사 조건 | Pending |
| 사격 불가 사유 표시 가능 | 과열 / 탄약 / 차단 / 범위 밖 | Pending |

---

## 8. P0 테스트 체크

| 체크 | 기준 | 상태 |
|---|---|---|
| 세단 기본 주행 검증 | `DA_TestSedan` 기본 연결 상태에서 PIE 주행 | Done |
| SUV 대조 테스트 방식 | 사용자가 수동으로 `DA_TestSUV` 전환 | Done |
| 경량 테스트 역할이 있다 | 빠른 접근 / 측후면 압박 | Pending |
| 중형 또는 중량 테스트 역할이 있다 | 정면 압박 / 대구경 / 지속전 | Pending |
| 같은 차량의 피팅 차이가 체감된다 | 중량 / 무기 / 장갑 차이. 현재 핵심 아님 | Later |
| 빠른 차량이 터렛 각속도 한계를 만든다 | 대구경 약점 확인 | Pending |
| 정면 장갑과 측후면 약점이 체감된다 | 위치 선점 의미 확인 | Pending |
| 모듈 손상이 흐름을 바꾼다 | 바퀴 / 엔진 / 터렛 | Pending |
| UI가 실패 이유를 설명한다 | 불공정감 방지 | Pending |

---

## 9. 차량 종류 확장 승인 체크

아래 항목이 통과되기 전에는 차량 종류를 늘리지 않는다.

| 체크 | 승인 기준 | 상태 |
|---|---|---|
| P0 하드포인트 최소 3종 동작 | 고정 / 짐벌 / 터렛 또는 락온 | Pending |
| 조준-사격-피해 루프 동작 | 최소 전투 루프 확인 | Pending |
| 중량 페널티 체감 | 피팅 비용 확인. 현재 핵심 아님 | Later |
| 장갑 방향 체감 | 정면 / 측후면 차이 확인 | Pending |
| 레티클 / UI 피드백 확인 | 조준 상태와 실패 이유 표시 | Pending |
| AI 역할 최소 2종 확인 | 전투 교훈이 다름 | Pending |

---

## 10. Changelog

### v0.23

- `CF_CombatDataOwnership.md` 존재 여부와 전투 데이터 소유권 기준 결정을 체크리스트에 추가했다.
- 첫 코드 작업 후보를 장착 프로파일 1종, WeaponComp 골격, FireOrigin 검증으로 좁혔다.

### v0.22

- 주행감 세분화는 현재 충분한 상태로 보고 Done 처리했다.
- 피팅 차이 체감과 중량 페널티 체감은 현재 핵심 작업이 아니므로 Later로 낮췄다.

### v0.21

- 사용자 확인 기준으로 `DA_TestSedan` 기본 주행 검증을 Done 처리했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.20

- `DA_PoliceCar` 폐기 결정과 `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 상태를 체크리스트에 반영했다.
- 세단 보강, VehicleData 기본 연결, 하드포인트 Transform 기록 항목을 Done으로 갱신했다.
- `DA_TestSUV`는 대조 테스트 전환 방식 결정 항목으로 분리했다.

### v0.19

- 통합 캡처 버튼 코드 작업 계획 문서 존재 여부를 체크리스트에 추가했다.

### v0.18

- `Front_02 + Gimbal`을 P0 필수 체크가 아니라 하드포인트 확장 가능성으로 정리했다.
- 하드포인트 소켓 존재 여부는 차량별 설정에 따라 달라질 수 있음을 체크리스트에 반영했다.
- Transform 수치 기록 기준을 모든 차량 공통 필수 소켓 캡처가 아니라 선언된 슬롯 기준 기록으로 갱신했다.

### v0.17

- 실제 SocketName prefix 정책을 `HP_` prefix 리네임으로 Done 처리했다.
- 하드포인트 캡처 자동화 방식을 바퀴 위치 캡처 버튼 통합으로 Done 처리했다.
- Transform 수치 기록 Pending 항목을 통합 버튼 구현 후 캡처해야 하는 작업으로 정리했다.

### v0.16

- `CF_MeshSocketAudit.md` 기준으로 P0 메시 Socket 후보 감사 완료 체크를 추가했다.
- 실제 SocketName prefix 정책과 `Front_02` 위치 후보를 Pending 항목으로 분리했다.
- 하드포인트 Transform 수치 기록 기준을 C++ 하드코딩값이 아니라 DataAsset LocalTransform 캡처로 명확히 했다.

### v0.15

- 같은 위치 카테고리에 여러 위치 슬롯 인스턴스를 둘 수 있다는 체크를 추가했다.
- 장착 프로파일 체크를 `Front_01 + Fixed`, `Front_02 + Gimbal`, `Top_01 + Turret` 기준으로 갱신했다.

### v0.14

- 하드포인트 체크에 위치 슬롯과 장착 타입 분리 기준을 추가하고 문서 기준 Done으로 기록했다.
- Mesh Socket 이름은 위치만 표현한다는 체크를 추가했다.
- 실제 차량별 Transform 수치 기록은 계속 Pending으로 유지했다.

### v0.13

- 하드포인트 위치 작성 방식을 4번 방식으로 확정해 체크리스트 기준에 반영했다.
- 실제 하드포인트 Transform 수치 기록은 계속 Pending으로 유지했다.

### v0.12

- `CF_HardpointAuthoring.md` 추가에 따라 하드포인트 위치 작성 방식 결정을 Done으로 기록했다.
- 실제 하드포인트 Transform 수치 기록은 계속 Pending으로 유지했다.

### v0.11

- `DA_TestSedan`, `DA_TestSUV`를 P0 대상 DataAsset 지정 완료로 갱신했다.
- `DA_TestSedan` 설정 보강 여부와 P0 테스트 VehicleData 전환 방식은 별도 Pending 체크로 분리했다.

### v0.10

- 임시 P0 메시 `Mesh_TestSedan`, `Mesh_TestSUV`를 완료 처리했다.
- 차량 DataAsset 지정은 별도 Pending 항목으로 분리했다.

### v0.9

- P0 대상 차형을 세단(Sedan) + SUV로 완료 처리했다.
- 실제 메시 / DataAsset 지정은 별도 Pending 항목으로 분리했다.

### v0.8

- `DA_PoliceCar`를 최종 P0 대상 차량에서 제외했다.
- 체크리스트에 P0 대상 차량 1~2종 지정 항목을 추가했다.
- Transform 수치 기록 기준을 최종 지정 차량별 DataAsset으로 변경했다.

### v0.7

- Transform 수치 기록 체크를 기준 차량 `DA_PoliceCar` DataAsset 값으로 명확히 했다.
- 차량별로 수치가 달라진다는 전제를 체크리스트에 반영했다.
  이 판단은 v0.8에서 P0 대상 차량 1~2종 기준으로 정정됐다.

### v0.6

- `Combined_Body` Socket Manager 0 소켓 확인 결과를 체크리스트에 반영했다.
- 하드포인트 Transform 기준을 DataAsset 로컬 Transform으로 완료 처리했다.
- 남은 체크를 하드포인트별 Transform 수치 기록으로 좁혔다.

### v0.5

- AssetDump 결과 문서를 체크 항목에 추가했다.
- BP / DataAsset / Reticle / Fire 입력은 AssetDump 기준 확인 완료로 기록했다.
- VehicleDebug 연결 상태는 별도 확인 대상으로 분리했다.

### v0.4

- 에디터 확인 가이드 문서를 체크 항목에 추가했다.
- 사용자가 직접 수행할 확인 순서가 준비되었음을 코드 착수 게이트 체크에 추가했다.

### v0.3

- 에디터 자산 감사 문서를 체크 항목에 추가했다.
- 파일 시스템 기준 자산 확인 완료와 하드포인트 Socket / Transform 미확정을 분리했다.

### v0.2

- 현재 기준선 문서와 코드 착수 게이트 문서를 체크 항목에 추가했다.
- 코드 착수 게이트 체크 섹션을 추가했다.
- 레티클 / P0 테스트 / 차량 확장 체크 섹션 번호를 조정했다.

### v0.1

- 최소 차량 플랫폼 검증 체크리스트 작성.
- 차량 종류 확장 승인 조건을 별도 체크 항목으로 분리.
- 레티클을 전투 UI 연동 체크로 제한.

---

## 11. Migration 메모

- 체크리스트 상태는 실제 구현 검증 후 `Pending`, `Pass`, `Fail`, `Blocked` 중 하나로 갱신한다.
- 현재 문서는 초안이므로 CombatPlan SSOT에는 아직 반영하지 않는다.
