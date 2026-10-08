# CarFight - Vehicle Platform 작업 순서

> 역할: 최소 차량 플랫폼 규격을 실제 구현 준비로 옮기기 위한 작업 순서를 정의한다.
> 문서 버전: v0.22
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Single Player Planning / Initial Layout Capture Implemented / Combat Data Ownership Documented

---

## 1. 목적

이 문서는 구현 순서를 정리한다.

핵심 목표는 차량 종류 확장이 아니라, 전투기능이 붙을 수 있는 최소 차량 플랫폼을 만드는 것이다.

---

## 2. 작업 원칙

1. `Document/ProjectSSOT/CombatPlan/`에는 작업 초안을 저장하지 않는다.
2. 확정 전 판단은 `Document/Plan/VehiclePlatformPlan/`에 기록한다.
3. 차량 종류를 늘리기 전에 P0 플랫폼 규격을 먼저 검증한다.
4. 레티클은 전투 UI 연동 항목으로만 다룬다.
5. 세부 수치는 P0 테스트 전까지 임시값으로 둔다.
6. 기존 `DataPlan`, `AimPlan`, `VehicleDebugPlan`과 충돌하지 않게 연결한다.

---

## 3. 전체 작업 순서

```text
Phase 0. 문서 경계 확정
Phase 1. 현재 차량 기준선 확인
Phase 2. 최소 플랫폼 데이터 규격 확정
Phase 3. 하드포인트 규격 확정
Phase 4. 전투기능 연동 계약 작성
Phase 5. 코드 착수 게이트 확인
Phase 6. P0 테스트 차량 구성
Phase 7. 검증 후 차량 종류 확장 여부 판단
```

---

## 4. Phase 0. 문서 경계 확정

목표:

`ProjectSSOT`와 `Plan`의 책임을 분리한다.

작업:

| 작업 | 산출물 |
|---|---|
| CombatPlan은 기획 원본으로만 사용 | `README.md` 경계 문구 |
| VehiclePlatformPlan은 작업 문서로 분리 | 이 폴더의 문서 세트 |
| 세션 목적 고정 | `CF_MinVehiclePlatform.md` |

완료 기준:

- 작업자가 어느 폴더에 무엇을 써야 하는지 헷갈리지 않는다.

---

## 5. Phase 1. 현재 차량 기준선 확인

목표:

현재 프로젝트에서 전투기능을 붙일 임시 P0 대상 세단 / SUV 메시와 차량 DataAsset을 확인한다.
`DA_PoliceCar`는 더 이상 현재 작업에 사용하지 않는 레거시 감사 자산이다.

작업:

| 작업 | 확인 대상 |
|---|---|
| 기준 Pawn 확인 | `BP_CFVehiclePawn`, `CFVehiclePawn` |
| 차량 DataAsset 현황 확인 | `UCFVehicleData`, 현재 P0 기본 `DA_TestSedan`, 대조 후보 `DA_TestSUV` |
| P0 대상 차형 선정 | 세단(Sedan), SUV |
| P0 대상 메시 선정 | `Mesh_TestSedan`, `Mesh_TestSUV` |
| P0 대상 차량 DataAsset 확인 | `DA_TestSedan`, `DA_TestSUV` 확인 및 하드포인트 슬롯 저장 완료 |
| P0 대상 메시 Socket 감사 | `CF_MeshSocketAudit.md` |
| 기존 차량 데이터 문서 확인 | `Document/Plan/DataPlan/` |
| 조준 시스템 현황 확인 | `Document/Plan/AimPlan/` |
| 차량 디버그 현황 확인 | `Document/Plan/VehicleDebugPlan/` |
| 현재 기준선 문서 작성 | `CF_PlatformBaseline.md` |
| 에디터 자산 감사 작성 | `CF_EditorAssetAudit.md` |
| AssetDump 결과 기록 | `CF_AssetDumpResult.md` |
| 사용자 에디터 확인 가이드 작성 | `CF_EditorCheckGuide.md` |

완료 기준:

- 현재 임시 차량 데이터 위치와 구현 책임을 설명할 수 있다.
- 최종 P0 대상 차형은 세단 / SUV이며, 메시가 `Mesh_TestSedan`, `Mesh_TestSUV`임을 설명할 수 있다.
- P0 후보 DataAsset이 `DA_TestSedan`, `DA_TestSUV`이고, 하드포인트 슬롯까지 저장됐음을 설명할 수 있다.
- `BP_CFVehiclePawn.VehicleData` 기본 연결이 `DA_TestSedan`으로 저장됐음을 설명할 수 있다.
- BP / DataAsset / Reticle / Fire 입력은 AssetDump로 확인된다.
- `Combined_Body` StaticMesh Socket은 AssetDump와 에디터에서 0개로 확인된다.
- 단, `Combined_Body`는 임시 감사 자산이며 최종 P0 차량 메시가 아니다.
- `Mesh_TestSedan` / `Mesh_TestSUV`에는 하드포인트 작성 보조 Socket 후보가 있음을 설명할 수 있다.
- P0 하드포인트 기준은 최종 차량 메시의 Socket 존재 여부와 무관하게 DataAsset 로컬 Transform으로 시작한다.

---

## 6. Phase 2. 최소 플랫폼 데이터 규격 확정

목표:

전투기능이 참조할 최소 데이터 묶음을 확정한다.

작업:

| 작업 | 결과 |
|---|---|
| 차량 Identity 정의 | 차급, 역할, 테스트 목적 |
| MovementBase 정의 | 가속, 제동, 선회, 최고속도 방향 |
| WeightModel 정의 | 기본 중량, 장착 중량, 페널티 방향 |
| Armor 정의 | 방향별 피해 계수, 장갑 타입 |
| Modules 정의 | 바퀴, 엔진, 터렛 |
| UiHooks 정의 | 조준점, 레티클 상태, 손상 표시 참조 |
| 전투 데이터 소유권 정의 | `CF_CombatDataOwnership.md` 기준 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage / UI 상태 분리 |

완료 기준:

- 조준 / 사격 / 피해 / UI / AI가 필요한 차량 정보를 한 표로 설명할 수 있다.
- P0 1차 구현 범위가 `MountProfile + UCFVehicleWeaponComp + FireOrigin`으로 제한된다.

---

## 7. Phase 3. 하드포인트 규격 확정

목표:

무기 시스템이 참조할 하드포인트 최소 계약을 만든다.

작업:

| 작업 | 결과 |
|---|---|
| P0 위치 카테고리 고정 | `Front`, `Back`, `LeftSide`, `RightSide`, `Top` 기본 분류 |
| P0 위치 슬롯 인스턴스 고정 | `Front_01`, `Front_02`, `Top_01`, `Top_02` 같은 실제 장착점 |
| 하드포인트 위치 작성 방식 결정 | 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처 |
| StaticMesh SocketName 정책 결정 | 하드포인트 소켓을 둘 경우 `HP_Front_01`, `HP_Top_01`처럼 `HP_` prefix 사용 |
| 하드포인트 캡처 자동화 결정 | 바퀴 위치 캡처 버튼에 하드포인트 LocalTransform 캡처 통합 |
| 장착 타입 지정 | `Fixed`, `Gimbal`, `Turret`, `Launcher`, `Utility` |
| P0 장착 프로파일 지정 | `Front_01 + Fixed`, `Top_01 + Turret` 우선. `Front_02 + Gimbal`은 확장 가능성 |
| 발사각 / 사각 정보 정의 | Yaw / Pitch / 고정 방향 |
| 장착 중량 반영 방식 정의 | 총중량 입력 |
| UI 참조 필요 여부 정의 | 레티클 / 실제 조준점 |

완료 기준:

- 무기를 장착하지 않아도 위치 슬롯 인스턴스와 장착 타입만 보고 어떤 전투 역할인지 알 수 있다.
- Mesh Socket 이름은 위치만 표현하고, 전투 규칙은 DA 장착 / 터렛 섹션의 `MountType`에서 설명된다.
- 같은 `Front`나 `Top` 위치 카테고리에 여러 슬롯 인스턴스를 둘 수 있다.
- 하드포인트 위치 작성 방식이 `CF_HardpointAuthoring.md` 기준 4번 방식으로 설명된다.
- 하드포인트 소켓은 차량 설정에 따라 있을 수도 있고 없을 수도 있으며, 선언된 슬롯만 통합 캡처 버튼에서 읽는다.

---

## 8. Phase 4. 전투기능 연동 계약 작성

목표:

전투기능별로 차량 플랫폼에서 필요한 정보를 명확히 한다.

작업:

| 전투기능 | 필요한 계약 |
|---|---|
| Aim | 조준점, 장착 프로파일 발사각, OutOfArc |
| Weapon | 발사 위치, 무기 그룹, 탄약, 열 |
| Damage | 피격 방향, 장갑, 차체 HP |
| ModuleDamage | 바퀴 / 엔진 / 터렛 손상 단계 |
| Lock-On | 센서 범위, 락온 유지각, 연막 / 플레어 |
| UI | 레티클 상태, 열 / 탄약 / 자원 / 손상 표시 |
| AI | 추격형, 중거리형, 중장갑형, 락온형 역할 입력 |

완료 기준:

- 각 전투기능 구현자가 차량 플랫폼에서 어떤 값을 읽어야 하는지 알 수 있다.

---

## 9. Phase 5. 코드 착수 게이트 확인

목표:

코드 작업을 시작하기 전에 문서, 에디터 확인, 첫 작업 범위를 분리한다.

작업:

| 작업 | 결과 |
|---|---|
| 현재 문서 준비 상태 판단 | `CF_CodeStartGate.md` |
| 에디터 확인 항목 분리 | 기준 BP / DataAsset / Mesh Socket 후보 / Transform 수치 |
| 사용자가 직접 확인할 순서 정리 | `CF_EditorCheckGuide.md` |
| 첫 코드 작업 후보 선정 | `CF_CombatDataOwnership.md` 기준 장착 프로파일 1종 + WeaponComp 골격 + FireOrigin |
| 보류 작업 분리 | 차량 대량 추가 / 상세 Reticle / 락온 완성 |

완료 기준:

- 코드 작업을 시작해도 되는 항목과 아직 확인해야 하는 항목을 구분할 수 있다.

---

## 10. Phase 6. P0 테스트 차량 구성

목표:

P0 테스트를 위한 차량 1~2종을 구성한다.

권장 구성:

| 차량 | 용도 |
|---|---|
| 세단(Sedan) / `DA_TestSedan` / `Mesh_TestSedan` | 기본 주행, 낮은 차체, 전방 고정 / 소형 짐벌. 기본 주행 검증 완료 |
| SUV / `DA_TestSUV` / `Mesh_TestSUV` | 높은 차체, 루프 장착, 중량 부담, 정면 압박. 대조 테스트는 수동 VehicleData 전환 |

현재 판단:

```text
주행감은 세단 / SUV 기준으로 적당히 분리된 상태다.
지금 가까운 핵심 작업은 주행감 추가 튜닝이 아니라 하드포인트 장착 / 터렛 / Fire / UI 상태 계약 연결이다.
```

최소 대안:

```text
차량 1종
  + 경량 피팅
  + 중장갑 / 대구경 피팅
```

완료 기준:

- 차량 수가 아니라 전투 역할 차이가 먼저 확인된다.

---

## 11. Phase 7. 차량 종류 확장 판단

목표:

P0 검증 후 차량 종류 확장 여부를 결정한다.

확장 조건:

1. 하드포인트가 무기 운용 차이를 만든다.
2. 중량이 전투 피팅 비용 차이를 만든다.
3. 장갑 방향이 전투 결과 차이를 만든다.
4. 모듈 손상이 전투 흐름을 바꾼다.
5. UI가 실패 이유를 설명한다.
6. AI가 플랫폼 역할을 드러낸다.

확장 금지 조건:

| 금지 조건 | 이유 |
|---|---|
| 차량 역할 차이가 불명확함 | 단순 스킨 증가가 됨 |
| 하드포인트가 아직 고정되지 않음 | 무기 장착 구조 재작업 위험 |
| 중량 페널티가 체감되지 않음 | 피팅 전략 붕괴 |
| UI가 판정 이유를 설명하지 못함 | 플레이어가 불공정하게 느낌 |

---

## 12. Changelog

### v0.22

- `CF_CombatDataOwnership.md`를 Phase 2 산출물로 연결했다.
- 첫 코드 작업 후보를 통합 캡처 버튼 이후 단계인 장착 프로파일 1종, WeaponComp 골격, FireOrigin 검증으로 갱신했다.

### v0.21

- 주행감은 현재 충분히 나눠진 상태로 보고 가까운 우선순위에서 제외했다.
- 다음 작업 방향을 하드포인트 장착 / 터렛 / Fire / UI 상태 계약 연결로 정리했다.

### v0.20

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 Phase 6에 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.19

- `DA_PoliceCar` 폐기 결정과 `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 상태를 반영했다.
- Phase 1의 현재 차량 DataAsset 기준을 `DA_TestSedan` / `DA_TestSUV`로 갱신했다.

### v0.18

- 차량 레이아웃 통합 캡처 구현 후 `DA_TestSedan` / `DA_TestSUV` 하드포인트 슬롯 저장 완료 상태를 반영했다.
- Phase 1 완료 기준에서 세단 설정 보강 잔여 문구를 제거했다.

### v0.17

- 첫 코드 작업 후보를 통합 캡처 버튼 구현 계획으로 구체화했다.
- `CF_SocketCaptureCodePlan.md`를 Phase 5 산출물로 반영했다.

### v0.16

- `Front_02 + Gimbal`을 P0 우선 장착 프로파일이 아니라 확장 가능성으로 낮췄다.
- 하드포인트 소켓 존재 여부를 차량 설정에 따른 선택 사항으로 정리했다.
- 통합 캡처 버튼은 선언된 슬롯만 읽는 흐름으로 작업 순서를 갱신했다.

### v0.15

- Phase 3에 StaticMesh SocketName `HP_` prefix 리네임 결정을 추가했다.
- Phase 3에 하드포인트 LocalTransform 캡처를 바퀴 위치 캡처 버튼에 통합하는 자동화 결정을 추가했다.
- 완료 기준에 실제 SocketName 리네임 후 통합 캡처 버튼에서 읽는 흐름을 반영했다.

### v0.14

- Phase 1에 P0 대상 메시 Socket 감사 산출물을 추가했다.
- `Mesh_TestSedan` / `Mesh_TestSUV`에 작성 보조 Socket 후보가 있음을 완료 기준에 반영했다.
- Phase 5의 에디터 확인 항목을 Mesh Socket 없음에서 Mesh Socket 후보 / Transform 수치 확인으로 갱신했다.

### v0.13

- Phase 3에 위치 카테고리와 위치 슬롯 인스턴스 분리를 반영했다.
- 같은 위치 카테고리에 여러 슬롯이 달릴 수 있음을 작업 완료 기준에 추가했다.
- P0 우선 장착 프로파일을 `Front_01 + Fixed`, `Top_01 + Turret`, `Front_02 + Gimbal`로 갱신했다.
  이 판단은 v0.16에서 `Front_02 + Gimbal`을 확장 가능성으로 낮추는 것으로 정정됐다.

### v0.12

- Phase 3을 결합형 하드포인트 이름 고정이 아니라 위치 슬롯과 장착 타입 분리 기준으로 갱신했다.
- P0 우선 장착 프로파일을 `Front + Fixed`, `Top + Turret`, `Front + Gimbal`로 정리했다.

### v0.11

- 하드포인트 위치 작성 방식을 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처로 확정해 Phase 3에 반영했다.

### v0.10

- Phase 3에 하드포인트 위치 작성 방식 결정 항목을 추가했다.
- `CF_HardpointAuthoring.md` 기준으로 DataAsset Transform 원본과 Socket / BP Preview 작성 보조 방식을 분리했다.

### v0.9

- P0 대상 차량 DataAsset을 `DA_TestSedan`, `DA_TestSUV`로 갱신했다.
- Phase 1 완료 기준에 `BP_CFVehiclePawn.VehicleData` 전환 방식과 세단 설정 보강 판단을 추가했다.
- Phase 6 테스트 차량 구성표에 DataAsset 이름을 명시했다.

### v0.8

- 임시 P0 메시로 `Mesh_TestSedan`, `Mesh_TestSUV`를 기록했다.
- 차량 메시 선정과 차량 DataAsset 선정을 분리했다.

### v0.7

- P0 대상 차형을 세단(Sedan) + SUV로 기록했다.
- Phase 1과 Phase 6에서 차형 선정과 실제 에셋 선정을 분리했다.

### v0.6

- `DA_PoliceCar`를 최종 P0 기준 차량이 아니라 임시 파이프라인 감사 기준으로 정리했다.
- Phase 1에 P0 대상 차량 1~2종 선정 항목을 추가했다.

### v0.5

- `Combined_Body` Socket Manager 0 소켓 확인 결과를 작업 순서에 반영했다.
- P0 하드포인트 기준을 DataAsset 로컬 Transform으로 정리했다.
- Phase 5의 남은 확인 범위를 Transform 수치 기록으로 좁혔다.

### v0.4

- Phase 1 산출물에 AssetDump 결과 문서를 추가했다.
- 자동 확인 완료 항목과 에디터 확인 잔여 항목을 완료 기준에 분리했다.

### v0.3

- 에디터 자산 감사 문서와 사용자 에디터 확인 가이드를 Phase 1 산출물에 추가했다.
- 코드 착수 게이트 Phase에 사용자가 직접 확인할 순서 정리를 추가했다.

### v0.2

- Phase 5에 코드 착수 게이트 확인 단계를 추가했다.
- 현재 기준선 문서 작성 산출물을 Phase 1에 추가했다.
- P0 테스트 차량 구성과 차량 종류 확장 판단을 한 단계씩 뒤로 이동했다.

### v0.1

- 최소 차량 플랫폼 구현 준비 작업 순서 작성.
- 차량 종류 확장 전 선행해야 할 Phase를 정의.
- 레티클을 Phase 4의 UI 연동 계약 하위 항목으로 제한.

---

## 13. Migration 메모

- 기존 `DataPlan`은 차량 DataAsset 소유권 문서로 유지한다.
- 기존 `AimPlan`은 조준 / 레티클 / Fire Command 문서로 유지한다.
- 본 문서는 두 문서를 대체하지 않고, 전투기능이 차량 플랫폼에 붙기 위한 연결 작업만 담당한다.
