# CarFight - 차량 전투 플랫폼 현재 기준선

> 역할: 코드 작업 전에 현재 차량 / 조준 / UI / 디버그 기준선을 한 문서에서 확인한다.
> 문서 버전: v0.4
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Baseline Updated / P0 DA Active

---

## 1. 목적

이 문서는 전투 차량 플랫폼 작업의 출발점을 고정한다.

현재 목표는 코드 구현이 아니다.
목표는 이후 어떤 AI 세션이 읽어도 아래를 같은 방식으로 이해하게 만드는 것이다.

```text
현재 차량 시스템은 어디까지 준비되어 있는가?
전투 플랫폼을 붙일 때 기존 시스템 중 무엇을 재사용해야 하는가?
아직 없는 것은 무엇인가?
코드 작업이 시작되면 어떤 경계를 넘지 않아야 하는가?
```

---

## 2. 확인한 기준 문서

| 문서 | 현재 작업에서 쓰는 의미 |
|---|---|
| `Document/ProjectSSOT/CombatPlan/` | 전투 기획 원본 |
| `Document/Plan/DataPlan/CF_VehicleDataPlan.md` | 차량 DataAsset 소유권 기준 |
| `Document/Plan/DataPlan/CF_VehicleDataInventory.md` | 기존 차량 데이터 위치와 레거시 경계 |
| `Document/Plan/AimPlan/CF_AimStatus.md` | 조준 / 로컬 Fire Command 현재 상태 |
| `Document/Plan/VehicleDebugPlan/README.md` | 디버그 Snapshot / HUD / Panel 확장 방향 |
| `Document/SSOT/UE_SSOT/UE_CPP_SSOT/UE_AI_Cpp_SSOT_v0_1.md` | C++ 중심, BP Thin 원칙 |
| `Document/SSOT/UE_SSOT/UE_CPP_SSOT/UE_BP_Patterns_v0_1.md` | BP는 조립 / 표현 계층이라는 원칙 |

---

## 3. 현재 C++ 기준선

아래는 2026-06-19 기준으로 확인한 소스 표면이다.

| 파일 | 현재 역할 | 전투 플랫폼 관점 |
|---|---|---|
| `UE/Source/CarFight_Re/Public/CFVehicleData.h` | `UCFVehicleData` 기반 차량 DataAsset | 전투 플랫폼 데이터 확장 후보 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 기준 차량 Pawn | 전투 시스템 조립 / 적용 진입점 후보 |
| `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h` | 주행 입력 / 주행 상태 | 중량 페널티와 DriveState 연동 후보 |
| `UE/Source/CarFight_Re/Public/CFWheelSyncComp.h` | 휠 시각 동기화 / 디버그 | 바퀴 모듈 손상 연동 후보 |
| `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h` | Aim / Reticle / Fire 타입 | 하드포인트 조준각과 발사 불가 사유 확장 후보 |
| `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h` | 로컬 조준 계산 / Fire Command | 현재는 기본 AimProfile 중심, 하드포인트별 AimProfile 필요 |
| `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h` | Reticle C++ 부모 위젯 | 전투 상태 표시 계층, 판단 로직 소유 금지 |

---

## 4. 이미 준비된 것

### 4.1 차량 데이터

확인된 상태:

- `UCFVehicleData`가 이미 존재한다.
- 차량 시각 자산, 이동 설정, 휠 시각 설정, 참조 설정, DriveState 설정 계층이 있다.
- DataPlan 기준으로 장기 방향은 `DataAsset은 저장소`, `C++는 해석과 적용`, `BP는 Thin 조립`이다.

전투 플랫폼에서 재사용할 부분:

| 기존 요소 | 재사용 방향 |
|---|---|
| 차량 시각 자산 | 차체 기준 위치와 하드포인트 프리뷰 기준 |
| 이동 설정 | 중량 페널티 적용 입력 |
| 휠 시각 설정 | 바퀴 모듈 손상 표시의 미래 기준 |
| DriveState 설정 | 피격 / 모듈 손상으로 주행 상태 제한 시 참조 |

아직 없는 부분:

- 전투 역할 식별값
- 하드포인트 목록
- 방향별 장갑 정보
- 모듈 손상 정보
- 무장 / 탄약 / 열 / 배터리 / 연료 자원 연결 정보
- 전투 UI가 참조할 플랫폼 상태 묶음

---

### 4.2 차량 Pawn

확인된 상태:

- `ACFVehiclePawn`이 현재 차량 Pawn 기준이다.
- 현재 `BP_CFVehiclePawn.VehicleData` 기본 연결은 `DA_TestSedan`이다.
- `DA_PoliceCar`는 더 이상 현재 작업에 사용하지 않는 레거시 감사 자산이다.
- P0 후보 차량 DataAsset은 `DA_TestSedan`, `DA_TestSUV`로 확인됐고, `DA_TestSUV`는 대조 테스트 후보로 유지한다.
- DriveComp, WheelSyncComp, VehicleCameraComp, VehicleAimComp를 소유한다.
- 로컬 Fire Command 생성, 검증, 더미 HitScan 흐름이 있다.
- Reticle Widget 생성 / 갱신 흐름이 있다.
- VehicleDebug Snapshot 조회 API가 있다.

전투 플랫폼에서 재사용할 부분:

| 기존 요소 | 재사용 방향 |
|---|---|
| `ApplyVehicleDataConfig` 계열 | 전투 플랫폼 데이터 적용 단계 추가 후보 |
| `BuildFireCommand` | 현재 하드포인트 / 무기 그룹을 포함하도록 확장 후보 |
| `ValidateFireCommand` | 탄약, 쿨다운, 발사각, 차체 상태 검증 후보 |
| `ApplyFireResult` | 피해 / 이펙트 / UI 상태 갱신 후보 |
| Reticle 갱신 흐름 | 표시만 유지, 판단은 Aim / Weapon / Combat 쪽에서 제공 |

주의:

- Pawn이 모든 전투 규칙을 직접 소유하면 비대해진다.
- P0에서는 Pawn을 조립 진입점으로 쓰되, 무기 / 피해 / 모듈 규칙은 분리 가능한 구조로 문서화한 뒤 구현해야 한다.

---

### 4.3 조준 / Fire Command

확인된 상태:

- Aim Core 1차 구현은 완료 상태다.
- Reticle C++ / WBP 연결은 완료 상태다.
- Fire Command / Local Validation / Local Dummy HitScan은 완료 상태다.
- 실제 Damage, WeaponComp, Projectile, Lock-On, AI Combat은 아직 없다.

전투 플랫폼에서 재사용할 부분:

| 기존 요소 | 재사용 방향 |
|---|---|
| `ECFVehicleReticleState` | 하드포인트 / 무기 상태 반영 |
| `ECFVehicleFireRejectReason` | 발사 불가 사유 확장 |
| `FCFVehicleAimProfile` | 하드포인트별 발사각 / 최대거리 후보 |
| `FCFVehicleFireRequest` | 현재는 로컬 Fire Command 의미로 해석 |
| `FCFVehicleFireValidationState` | 탄약, 쿨다운, OutOfArc 등 검증 결과 표시 |

주의:

- 현재 AimProfile은 기본 프로필 중심이다.
- P0 전투 플랫폼에서는 무기 또는 하드포인트 단위 AimProfile이 필요해질 가능성이 높다.
- 아직 이름에 `Request`가 남아 있어도 현재 의미는 네트워크 요청이 아니라 로컬 Fire Command로 해석한다.

---

### 4.4 Reticle

확인된 상태:

- `UCFAimReticleWidget`은 AimComp 상태를 읽어 표시하는 C++ 부모 위젯이다.
- WBP 자식이 레이아웃과 스타일을 담당한다.

전투 플랫폼에서의 위치:

```text
Reticle은 전투 플랫폼의 중심이 아니다.
Reticle은 하드포인트 / 무기 / Aim / Fire 검증 결과를 표시하는 UI 표면이다.
```

주의:

- Reticle이 하드포인트 규칙을 판단하면 안 된다.
- Reticle은 `Ready`, `OutOfArc`, `NoWeapon`, `Cooldown`, `Reloading`, `FireRejected` 같은 상태를 읽어 표현한다.
- 무기별 상세 레티클 디자인은 P0 플랫폼 규격 확정 이후 UI 작업으로 분리한다.

---

### 4.5 VehicleDebug

확인된 상태:

- VehicleDebug는 구조화된 Snapshot 기반으로 확장하는 방향이 잡혀 있다.
- HUD / Panel / Event 표시가 같은 원본 데이터를 공유하는 것이 장기 방향이다.

전투 플랫폼에서 재사용할 부분:

| 디버그 항목 | 전투 플랫폼에서 필요한 이유 |
|---|---|
| Aim 상태 | 발사 가능 / 불가능 이유 확인 |
| Drive 상태 | 중량 / 피해가 주행에 주는 영향 확인 |
| Runtime 상태 | 차량 DataAsset 적용 여부 확인 |
| 향후 Combat 상태 | 하드포인트, 무기, 장갑, 모듈 손상 관측 |

주의:

- 디버그값은 차종 데이터가 아니다.
- 전투 플랫폼 데이터와 디버그 토글을 섞지 않는다.

---

## 5. 현재 가장 큰 빈칸

| 빈칸 | 이유 |
|---|---|
| 하드포인트 데이터가 없다 | 무기를 어디에 달고 어떤 발사각을 갖는지 알 수 없음 |
| 무기 그룹이 없다 | Fire Command가 어떤 무기를 쏘는지 구분 불가 |
| 장갑 / 피해 데이터가 없다 | Hit 결과가 전투 결과로 이어지지 않음 |
| 모듈 손상 데이터가 없다 | 바퀴 / 엔진 / 터렛 피해를 구분할 수 없음 |
| 중량 모델이 전투 장비와 연결되지 않았다 | 피팅 선택의 비용이 주행에 드러나지 않음 |
| AI Combat 입력이 없다 | 차량 역할을 AI가 사용할 기준이 없음 |
| Reticle이 무기별 표시를 모른다 | 단, 이것은 UI 하위 작업이다 |

---

## 6. 구현 금지선

코드 작업을 시작할 때 아래는 피한다.

1. 차량 종류를 먼저 늘리지 않는다.
2. BP Details에 전투 규칙 원본을 추가하지 않는다.
3. Reticle 디자인부터 만들지 않는다.
4. Debug 토글을 차종 데이터처럼 쓰지 않는다.
5. 멀티플레이 / 서버 RPC 구조를 다시 끌어오지 않는다.
6. 기존 `UCFVehicleData` 구조를 설명 없이 갈아엎지 않는다.

---

## 7. 다음 문서로 넘길 내용

이 문서는 기준선만 고정한다.

구현 계약은 아래 문서에서 다룬다.

- `CF_CombatContract.md`
- `CF_HardpointMatrix.md`
- `CF_CodeStartGate.md`

---

## 8. Changelog

### v0.4

- `DA_PoliceCar` 폐기 결정과 `BP_CFVehiclePawn.VehicleData = DA_TestSedan` 저장 상태를 기준선에 반영했다.
- 문서 상태를 코드 착수 전 기준선에서 현재 P0 DA 활성 기준선으로 갱신했다.

### v0.3

- P0 후보 차량 DataAsset을 `DA_TestSedan`, `DA_TestSUV`로 기록했다.
- 현재 `BP_CFVehiclePawn.VehicleData` 기본 연결은 아직 `DA_PoliceCar`임을 기준선에 남겼다.

### v0.2

- `DA_PoliceCar`가 최종 P0 차량이 아니라 임시 파이프라인 확인 기준임을 명시했다.

### v0.1

- 전투 차량 플랫폼 작업의 현재 코드 / 문서 기준선을 정리했다.
- 기존 DataPlan, AimPlan, VehicleDebugPlan과의 연결 관계를 기록했다.
- 현재 없는 전투 플랫폼 데이터와 구현 금지선을 분리했다.

---

## 9. Migration 메모

- 이 문서는 코드를 변경하지 않는다.
- 이후 구현 세션은 이 문서를 먼저 읽고 현재 기준선을 확인한다.
- 기준선이 코드와 달라지면 이 문서를 갱신한 뒤 작업 계획을 수정한다.
