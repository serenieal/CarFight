# CarFight - 전투 플랫폼 코드 착수 게이트

> 역할: 문서 준비가 코드 작업으로 넘어가도 되는지 판단하는 기준을 정의한다.
> 문서 버전: v0.23
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Initial Code Implemented / Combat Data Ownership Documented

---

## 1. 목적

이 문서는 코드 작업을 바로 시작하지 않기 위한 안전장치다.

문서 준비가 충분한지 판단하고, 충분하다면 어떤 최소 코드 작업부터 시작할지 정한다.

---

## 2. 현재 결론

현재 문서 기반은 전투 차량 플랫폼 코드 작업을 설계할 만큼은 충분해졌다.

다만 바로 구현을 시작하기 전 아래 확인은 필요하다.

```text
문서 기준:
  충분

코드 착수 상태:
  차량 레이아웃 통합 캡처 1차 구현 완료

다음 구현 전 필요한 마지막 확인:
  P0 테스트 실행 시 기본은 DA_TestSedan
  DA_TestSUV 대조 테스트는 사용자가 수동으로 VehicleData를 전환
  주행감 세분화는 현재 충분하므로 가까운 핵심 작업에서 제외
  전투 데이터 소유권은 CF_CombatDataOwnership.md 기준
  P0 1차 구현 범위는 MountProfile + UCFVehicleWeaponComp + FireOrigin
  Fire / UI 상태 계약 연결
  P0 테스트 차량 구성 방식
```

---

## 3. 코드 착수 전 필수 입력

| 입력 | 상태 | 확인 방법 |
|---|---:|---|
| 세션 목표 | 완료 | 최소 차량 플랫폼 규격 정의로 고정 |
| CombatPlan SSOT 방향 | 완료 | P0 차량 1~2종 + 피팅 차이 |
| 현재 차량 기준선 | 완료 | `CF_PlatformBaseline.md` |
| 플랫폼 계약 | 완료 | `CF_CombatContract.md` |
| 전투 데이터 소유권 | 완료 | `CF_CombatDataOwnership.md` |
| 하드포인트 계약 | 완료 | `CF_HardpointMatrix.md` |
| Reticle 위치 | 완료 | UI 표시 계층으로 고정 |
| 구현 금지선 | 완료 | 차량 종류 선확장 금지 |
| 에디터 자산 확인 | AssetDump 완료, StaticMesh Socket 감사 완료 | `CF_EditorAssetAudit.md`, `CF_AssetDumpResult.md`, `CF_MeshSocketAudit.md` |
| 사용자가 직접 확인할 에디터 체크 | 준비 완료 | `CF_EditorCheckGuide.md` |
| Mesh Socket 의존 여부 | 완료 | P0는 최종 차량 메시에서도 Mesh Socket 필수 의존 없음 |
| 하드포인트 Transform 기준 | 완료 | P0는 DataAsset 로컬 Transform 기준 |
| 하드포인트 위치 작성 방식 | 완료 | 4번 방식 선택. Socket / BP Preview로 배치 후 DataAsset에 캡처 |
| 하드포인트 캡처 자동화 방식 | 구현 완료 | 바퀴 위치 캡처 버튼에 하드포인트 LocalTransform 캡처를 통합 |
| 통합 캡처 코드 작업 계획 | 완료 | `CF_SocketCaptureCodePlan.md` |
| 하드포인트 위치 / 타입 분리 | 완료 | Socket을 둘 경우 `HP_Front_01`, `HP_Top_01` 같은 위치 슬롯 인스턴스만 표현. DA 장착 / 터렛 섹션에서 `MountType` 기록 |
| 같은 위치의 복수 슬롯 지원 | 완료 | `Front_01`, `Front_02`, `Top_01`, `Top_02`처럼 한 위치 카테고리에 여러 장착점 허용 |
| SocketName prefix 정책 | 완료 | 캡처 입력으로 사용할 하드포인트 소켓은 `HP_` prefix 사용. 단 차량별 소켓 존재는 설정에 따라 다름 |
| P0 대상 차형 | 완료 | 세단(Sedan) + SUV 기본안 |
| P0 대상 메시 | 완료 | `Mesh_TestSedan`, `Mesh_TestSUV`, AssetDump 기준 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV` |
| P0 대상 DataAsset | 완료 | `DA_TestSedan`, `DA_TestSUV` 확인 및 하드포인트 슬롯 저장. `BP_CFVehiclePawn.VehicleData` 기본값은 `DA_TestSedan`으로 저장 완료 |
| 세단 DataAsset 보강 판단 | 완료 | `DA_TestSedan` WheelClass / Movement / WheelVisual / DriveState / Layout Override 저장 확인 |
| 세단 기본 주행 검증 | 완료 | 사용자 PIE 주행 검증 완료 |
| SUV 대조 테스트 방식 | 완료 | 별도 자동화 없이 수동 VehicleData 전환으로 진행 |
| 주행감 세분화 | 완료 / 후순위 | 현재 충분히 나눠진 상태. 가까운 핵심 작업 아님 |
| 하드포인트 Transform 수치 | 완료 | `DA_TestSedan`, `DA_TestSUV`에 `HP_*` 소켓 기준 `HardpointSlots` 저장 |
| 첫 구현 범위 | 완료 | 차량 레이아웃 통합 캡처와 하드포인트 슬롯 Validator 구현 |

---

## 4. 착수 전 에디터 확인 체크

코드 작업 전에 사용자가 언리얼 에디터에서 확인하면 좋은 항목이다.

| 확인 항목 | 이유 |
|---|---|
| `BP_CFVehiclePawn`가 현재 기준 차량인지 | 잘못된 BP에 적용하는 것을 방지 |
| 현재 연결된 차량 DataAsset | `UCFVehicleData` 기반인지 확인 |
| P0가 Mesh Socket에 의존하지 않는지 | 확인 완료, DataAsset Transform 기준 |
| 하드포인트 로컬 Transform 후보 | P0 대상 차량 DataAsset별 위치 / 방향 수치 결정 |
| 차체 전방 방향 | Fixed 무기 발사 방향 기준 |
| 루프 / 전방 장착 위치 후보 | 하드포인트 위치 산정 |
| 기존 Reticle WBP 연결 상태 | UI 회귀 방지 |
| VehicleDebug 표시 상태 | 구현 후 관측 가능성 확인 |

현재 파일 감사 결과:

| 항목 | 상태 | 근거 |
|---|---:|---|
| `BP_CFVehiclePawn` 파일 / 부모 | 확인됨 | AssetDump 기준 `CFVehiclePawn` |
| `DA_PoliceCar` 파일 / 클래스 | 폐기 / 레거시 증거 | 현재 작업에는 사용하지 않음. 과거 AssetDump 감사 기록으로만 남김 |
| `DA_TestSedan` 파일 / 클래스 | 확인됨, 기본 테스트 기준 | AssetDump 기준 `CFVehicleData`, 세단 후보 |
| `DA_TestSUV` 파일 / 클래스 | 확인됨 | AssetDump 기준 `CFVehicleData`, SUV 후보 |
| `WBP_AimReticle` 파일 / 부모 | 확인됨 | AssetDump 기준 `CFAimReticleWidget` |
| `InputAction_Fire` | 확인됨 | AssetDump 기준 `IA_Fire` |
| `Combined_Body` 파일 | 확인됨 | AssetDump 목록 / StaticMesh |
| 하드포인트 Socket | 작성 보조 후보 확인 | `Combined_Body`는 0 소켓, `Mesh_TestSedan` / `Mesh_TestSUV`에는 위치 슬롯 후보 있음 |
| 하드포인트 작성 보조 | 선택 완료 | StaticMesh Socket 또는 BP Preview로 배치 후 DataAsset에 캡처 |
| VehicleDebug BP 기본 참조 | 미확정 | 최신 AssetDump에서 기본 참조 비어 있음 |

상세 감사 기록:

```text
Document/Plan/VehiclePlatformPlan/CF_EditorAssetAudit.md
Document/Plan/VehiclePlatformPlan/CF_AssetDumpResult.md
```

메뉴 경로 예시:

```text
콘텐츠 브라우저(Content Browser)
  -> /Game/CarFight/Vehicles
  -> BP_CFVehiclePawn 열기

디테일(Details)
  -> VehicleData
  -> AimReticleWidgetClass
  -> VehicleAimComp
  -> VehicleDebug 관련 표시 옵션
```

---

## 5. 첫 코드 작업 후보

첫 코드 작업은 작아야 한다.

권장 순서:

| 순서 | 작업 | 이유 |
|---:|---|---|
| 1 | 전투 플랫폼 데이터 최소 구조 추가 | 이후 모든 기능의 입력 |
| 2 | P0 장착 프로파일 1종 적용 | `LocationSlot=Top_01 + MountType=Turret + RoofTurret_MediumOrLarge`부터 검증 |
| 3 | `UCFVehicleWeaponComp` 골격 추가 | 장착 프로파일 해석과 FireOrigin 계산 담당 |
| 4 | Fire Command에 WeaponComp FireOrigin 우선 연결 | 차량 중심이 아니라 실제 터렛 발사 원점 추적 가능 |
| 5 | Reticle / Debug에 하드포인트 / 터렛 상태 표시 | 판정 불신 방지 |

보류:

| 보류 작업 | 이유 |
|---|---|
| 차량 종류 대량 추가 | 플랫폼 검증 전 확장 위험 |
| 미사일 / 락온 완성 | 기본 Fire / 하드포인트 검증 후 진행 |
| 상세 Reticle 디자인 | 상태 계약 안정화 후 진행 |
| 복잡한 Damage 모델 | 방향별 피해부터 검증 필요 |
| AI Combat 전체 구현 | 플랫폼 입력 안정화 후 진행 |

---

## 6. 첫 구현의 최소 완료 조건

첫 코드 작업이 끝났다고 보려면 아래가 필요하다.

1. 기준 차량 1대에서 하드포인트 목록을 읽을 수 있다.
2. `LocationSlot=Front_01 + MountType=Fixed` 또는 `LocationSlot=Top_01 + MountType=Turret` 중 최소 1종이 Fire 검증에 참여한다.
3. 발사 가능 / 불가능 이유가 Reticle 또는 Debug에 표시된다.
4. 장착 프로파일 발사각 밖에서는 `OutOfArc` 또는 동등한 상태가 나온다.
5. 기존 Aim Core, Fire Command, Reticle 생성 흐름이 깨지지 않는다.
6. BP가 전투 규칙의 원본 저장소가 되지 않는다.

---

## 7. 코드 작업 시작 시 문서 갱신 규칙

코드 작업을 시작하면 다음 문서를 함께 갱신한다.

| 상황 | 갱신 문서 |
|---|---|
| 구조체 / enum 이름 확정 | `CF_CombatContract.md` |
| 전투 데이터 소유권 변경 | `CF_CombatDataOwnership.md` |
| LocationSlotId / MountProfileId 변경 | `CF_HardpointMatrix.md` |
| 작업 단계 변경 | `CF_PlatformTasks.md` |
| 완료 조건 변경 | `CF_PlatformChecklist.md` |
| 기준선 변경 | `CF_PlatformBaseline.md` |

중요:

```text
코드 변경이 문서를 앞질러 가면 안 된다.
문서가 최소한 변경 이유와 검증 기준을 먼저 설명해야 한다.
```

---

## 8. 착수 판단표

| 질문 | 현재 답 |
|---|---|
| 차량 종류 확장부터 해야 하는가? | 아니오 |
| 전투 기능부터 무작정 구현해야 하는가? | 아니오 |
| 최소 플랫폼 규격부터 정해야 하는가? | 예 |
| 하드포인트 설정이 선행되어야 하는가? | 예, 단 최소 2종부터 |
| Reticle 추가가 중심 작업인가? | 아니오 |
| Reticle 상태 연결은 필요한가? | 예, 검증 피드백으로 필요 |
| 현재 문서만으로 첫 작업 방향을 잡을 수 있는가? | 예 |
| 바로 코드 작업을 시작해도 되는가? | 사용자가 승인하면 가능 |

---

## 9. Changelog

### v0.23

- 전투 데이터 소유권 기준 문서 `CF_CombatDataOwnership.md`를 코드 착수 필수 입력으로 추가했다.
- 첫 코드 작업 후보를 `MountProfile + UCFVehicleWeaponComp + FireOrigin` 중심으로 재정렬했다.

### v0.22

- 주행감 세분화는 현재 충분히 완료된 상태로 보고 후순위로 낮췄다.
- 다음 코드 / 설계 초점을 장착 / 터렛 프로파일과 Fire / UI 상태 계약 연결로 좁혔다.

### v0.21

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.20

- `DA_PoliceCar`를 현재 작업 기준에서 폐기하고, `BP_CFVehiclePawn.VehicleData` 기본값이 `DA_TestSedan`으로 저장된 상태를 반영했다.
- P0 대상 DataAsset 판단에서 `DA_TestSedan`을 기본 테스트 기준, `DA_TestSUV`를 대조 후보로 정리했다.

### v0.19

- 코드 착수 상태를 `Initial Code Implemented`로 갱신했다.
- 차량 레이아웃 통합 캡처, 하드포인트 슬롯 저장, Validator 연결, `DA_TestSedan` / `DA_TestSUV` 하드포인트 슬롯 입력 완료 상태를 반영했다.
- 다음 확인 대상을 BP 기본 VehicleData 전환 방식과 장착 / 터렛 프로파일 구조로 좁혔다.

### v0.18

- 통합 캡처 버튼 구현 기준 문서 `CF_SocketCaptureCodePlan.md`를 코드 착수 게이트에 추가했다.
- 선언된 하드포인트 슬롯만 선택 캡처하는 구현 기준을 코드 작업 계획으로 분리했다.

### v0.17

- 하드포인트 소켓을 모든 차량 필수 조건이 아니라 차량별 설정에 따른 선택 캡처 입력으로 정리했다.
- `SocketName` prefix 정책을 캡처 입력으로 쓰는 소켓에 적용하는 규칙으로 좁혔다.
- Transform 수치 기록 기준을 선언된 위치 슬롯 기준으로 갱신했다.

### v0.16

- StaticMesh SocketName 정책을 `HP_` prefix 리네임으로 완료 처리했다.
- 하드포인트 캡처 방식을 수동 기록 / 별도 버튼 선택 문제가 아니라 바퀴 위치 캡처 버튼 통합 구현으로 확정했다.
- 하드포인트 Transform 수치 상태를 하드코딩 후보가 아니라 통합 캡처 버튼 구현 필요 항목으로 갱신했다.

### v0.15

- StaticMesh Socket 감사 결과를 코드 착수 게이트에 반영했다.
- 하드포인트 Transform 수치를 하드코딩 후보가 아니라 DataAsset 캡처 확정 대상으로 정리했다.
- `CF_MeshSocketAudit.md`를 에디터 자산 확인 근거에 추가했다.

### v0.14

- 같은 위치 카테고리에 여러 위치 슬롯 인스턴스가 있을 수 있다는 착수 기준을 추가했다.
- 첫 구현 예시를 `Front_01 + Fixed`, `Top_01 + Turret` 기준으로 갱신했다.

### v0.13

- 코드 착수 기준에 하드포인트 위치 슬롯과 장착 타입 분리 완료 항목을 추가했다.
- 첫 구현 범위를 결합형 하드포인트 이름이 아니라 `LocationSlot + MountType` 장착 프로파일 기준으로 정리했다.

### v0.12

- 하드포인트 위치 작성 방식을 4번, 즉 Socket / Preview 배치 후 DataAsset 캡처로 확정했다.
- 코드 착수 전 남은 확인에서 작성 방식 선택을 제거하고, 캡처 자동화 수준과 실제 Transform 수치 기록으로 좁혔다.

### v0.11

- 하드포인트 위치 작성 방식 문서 `CF_HardpointAuthoring.md`를 코드 착수 게이트에 연결했다.
- Socket / BP Preview는 작성 보조이고 P0 런타임 원본은 DataAsset Transform이라는 기준을 게이트에 반영했다.

### v0.10

- `DA_TestSedan`, `DA_TestSUV` AssetDump 확인 결과를 코드 착수 게이트에 반영했다.
- P0 대상 DataAsset 지정은 부분 완료로 올리되, 런타임 테스트 연결 방식과 `DA_TestSedan` 보강 판단을 남은 입력으로 분리했다.
- P0 후보 메시 경로를 AssetDump 기준 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV`로 정정했다.

### v0.9

- 임시 P0 메시로 `Mesh_TestSedan`, `Mesh_TestSUV`를 완료 처리했다.
- 남은 코드 착수 입력을 세단 / SUV용 DataAsset 지정과 Transform 수치 기록으로 좁혔다.

### v0.8

- P0 대상 차형 기본안을 세단(Sedan) + SUV로 기록했다.
- 코드 착수 전 남은 입력을 실제 차량 메시 / DataAsset 지정으로 좁혔다.

### v0.7

- `DA_PoliceCar`를 코드 착수용 최종 P0 대상 차량에서 제외했다.
- 코드 착수 전 필수 입력에 P0 대상 차량 1~2종 확정을 추가했다.
- 하드포인트 Transform 수치 기록 대상을 최종 지정 차량별 DataAsset으로 변경했다.

### v0.6

- 하드포인트 Transform 수치를 전 차량 공통값이 아니라 기준 차량 DataAsset 값으로 명시했다.
- 코드 착수 전 필요한 수치 기록 대상을 `DA_PoliceCar` 후보 좌표로 좁혔다.
  이 판단은 v0.7에서 P0 대상 차량 1~2종 기준으로 정정됐다.

### v0.5

- `Combined_Body` Socket Manager 0 소켓 확인 결과를 코드 착수 게이트에 반영했다.
- 하드포인트 Transform 기준을 DataAsset 로컬 Transform으로 완료 처리했다.
- 코드 착수 전 남은 확인을 Transform 방식이 아니라 실제 좌표 수치 후보로 좁혔다.

### v0.4

- AssetDump 확인 결과를 코드 착수 게이트에 반영했다.
- BP / DataAsset / Reticle / Fire 입력 확인 상태를 완료로 올렸다.
- StaticMesh Socket과 VehicleDebug PIE 확인은 별도 에디터 확인 대상으로 남겼다.

### v0.3

- 사용자가 직접 수행할 에디터 확인 가이드 `CF_EditorCheckGuide.md`를 연결했다.
- Mesh Socket / Transform 기준 결정 전에는 C++ 구현을 시작하지 않는 흐름을 강화했다.

### v0.2

- `CF_EditorAssetAudit.md` 결과를 반영했다.
- 에디터 자산 확인 상태를 `필요`에서 `부분 완료`로 갱신했다.
- Mesh Socket / 하드포인트 Transform 기준은 에디터 확인 필요로 분리했다.

### v0.1

- 전투 플랫폼 코드 착수 전 게이트를 정의했다.
- 현재 문서 준비 상태를 `설계 가능 / 코드 미착수`로 분리했다.
- 첫 코드 작업 후보와 보류 대상을 정리했다.

---

## 10. Migration 메모

- 이 문서는 코드 작업 시작 승인 문서가 아니다.
- 사용자가 코드 작업 시작을 승인하면 이 문서를 기준으로 첫 구현 범위를 잘라야 한다.
- 코드 작업이 진행되면 변경점, 검증 결과, 마이그레이션 지침을 관련 문서에 반영한다.
