# 13. Fitting System

- 문서 버전: v0.3.0
- 작성일: 2026-06-08
- 최근 갱신일: 2026-08-01
- 문서 상태: Current Direction / Fitting·Inventory Separate Ready Plans Registered
- 담당 범위: 차량 피팅 / 슬롯 / 하드포인트 / 질량 / 출격·필드 장착
- 구현 Feature: `CF-FQ-034 차량 피팅·질량 런타임`
- 지원 Feature: `CF-FQ-035 인벤토리 Foundation`
- 대표 Plan: `Document/Plan/VehicleFitting/VehicleFittingPlan.md`
- Design: `Document/Plan/VehicleFitting/VehicleFittingDesign.md`
- Roadmap: `Document/Plan/VehicleFitting/VehicleFittingRoadmap.md`

---

## 1. 목적

이 문서는 CarFight의 장기 피팅 방향과 첫 구현 범위를 정의한다.

CarFight의 피팅은 EVE Online식 전략성을 참고하지만, 우주선의 전력·CPU 중심 구조를 그대로 복제하지 않는다. 자동차 게임이므로 피팅의 핵심 비용은 `중량`, `장착 위치`, `탄약 적재`, `방어 패키지`, `차량 기동성 변화`여야 한다.

장기적으로는 연료, 배터리와 Utility도 피팅에 포함하지만, 현재 실제 구현 기반과 별도 기능 P0에서는 아직 존재하지 않는 시스템을 임의로 완성된 것처럼 다루지 않는다.

---

## 2. 핵심 정의

> 피팅은 차량 플랫폼의 장착 위치·소유 장비·방어·탄약을 선택하고, 호환성·소유권·총중량을 검증하여 출격 초기 또는 조건부 필드 시간 액션으로 실제 전투 방식과 기동성을 결정하는 전략 선택이다.

피팅은 다음 계층을 분리한다.

```text
Vehicle Platform
= 차체, 하드포인트, 장착 제한과 기본 주행 특성

Saved Fitting Template
= 원하는 장비 종류와 출격 구성을 저장한 선택

Field Fitting Draft
= 접근 가능한 Inventory의 실제 Item Instance를 사용하는 편집 중 선택

Applied Fitting Snapshot
= 모든 호환·소유권·수량·질량 검증과 Commit을 통과한 실제 차량 구성
```

VehicleData 원본을 플레이어 선택 저장소로 직접 수정하지 않는다.

---

## 3. 장기 피팅 요소

| 요소 | 전투 영향 | 현재 구현 상태 |
|---|---|---|
| 차급·플랫폼 | 기본 질량, 크기, 하드포인트, 주행 특성 | VehicleData 구현 / 명시 BaseMass 없음 |
| 무기 | 교전 거리, 발사 방식, 화력, 각속도 | Weapon·Turret·Launcher 구현 |
| 탄종·탄수 | 지속 전투력, 장갑 대응, 중량 | 데이터 후보 / Ammo Runtime Ready Plan |
| 방어 | Shield, 방향별 Armor, 생존력, 중량 | Defense Runtime 적용 / DefenseMass 없음 |
| 배터리 | 센서, 락온, 방어장비 지속력 | 미구현 |
| 연료 | 작전 지속력, 추격·이탈 여유, 중량 | 미구현 |
| Utility | 연막, 플레어, 냉각, 수리, 센서 보조 | Runtime 미구현 |

장기 방향과 현재 구현 범위를 혼동하지 않는다.

---

## 4. 현재 실제 구현 기반

### 4.1 차량 플랫폼과 하드포인트

현재 `UCFVehicleData`는 다음을 제공한다.

```text
HardpointSlots
MountProfiles
VehicleMovementConfig
VehicleDurabilityConfig
DefaultDefenseData
Vehicle Visual / Wheel / Drive 설정
```

현재 위치와 장착 규칙:

```text
HardpointSlot
= 물리적 장착 위치와 Socket

MountProfile
= 위치 참조, MountType, SizeLimit, 기본 EquipmentPreset
```

슬롯과 하드포인트를 별도 신규 체계로 다시 만들지 않는다.
피팅은 현재 `HardpointSlots + MountProfiles`를 사용한다.

### 4.2 장비 연결

현재 단일 장비 연결 경로:

```text
MountProfile.DefaultEquipmentPresetData
→ EquipmentPresetData.DefaultTurretMountData
→ EquipmentPresetData.DefaultWeaponData
```

`EquipmentPresetData`는 MountType과 WeaponSize 호환 검증을 제공한다.
피팅 장비 선택도 이 경로를 사용한다.

### 4.3 탄약

현재 WeaponData에는 `MagazineSize`, `ReloadTimeSeconds`, `AmmoTypeId` 후보가 있지만 실제 소비·재장전 런타임은 아직 없다.

`CF-FQ-031 차량 탄약·재장전 런타임`이 다음을 소유할 예정이다.

```text
AmmoData.UnitMassKg
MaximumLoadableAmmoCount
InitialSortieAmmoCount
WeaponInstanceId별 장전량
AmmoData별 예비량
발사 소비·예약·재장전
```

피팅은 출격 적재 수량과 질량을 선택하지만 탄약 Runtime 상태를 중복 소유하지 않는다.

### 4.4 방어

현재 `CF-FQ-033` 기반:

```text
VehicleData.DefaultDefenseData
→ VehicleDefenseData
→ Shield
→ 6방향 Armor
→ Vehicle Integrity
```

피팅 P0는 완성된 `VehicleDefenseData` 하나를 선택한다.
방향별 장갑 수치를 피팅 화면에서 개별 조립하지 않는다.

### 4.5 질량

현재 명시적으로 확인된 장비 질량 필드는 다음 하나다.

```text
TurretMountData.TurretMountWeightKg
```

이 값도 현재 차량 총중량이나 Chaos 질량에 적용되지 않는다.

현재 없는 질량 계약:

```text
VehicleData.BaseVehicleMassKg
VehicleData.MaximumGrossMassKg
WeaponData.WeaponMassKg
VehicleDefenseData.DefenseMassKg
AmmoData.UnitMassKg 실제 구현
Fitting TotalMass Snapshot
Chaos Vehicle Mass 적용 경로
```

`CenterOfMassOverride`는 중심질량 위치 보정이며 총중량을 대신하지 않는다.

---

## 5. 슬롯과 하드포인트 원칙

장기 슬롯 분류:

| 슬롯 | 역할 | P0 상태 |
|---|---|---|
| Weapon | 주무기·보조무기 장착 | 포함 |
| Defense | 완성된 방어 패키지 선택 | 포함 |
| Ammo Loadout | 탄종과 출격 탄수 선택 | 의존 기능 연동 포함 |
| Utility | 연막·플레어·냉각·수리 | 제외 |
| Battery | 배터리 용량·출력 | 제외 |
| Fuel | 연료량 | 제외 |

핵심 구분:

```text
Slot / MountProfile
= 어떤 종류·크기의 장비를 허용하는가

HardpointSlot
= 차량의 어디에 장비가 붙는가
```

현재 코드에서는 MountProfile이 장비 제한, HardpointSlot이 위치를 담당한다.

---

## 6. 핵심 비용: 중량

장기 총중량 방향:

```text
총중량
= 차체 기본 중량
+ 장착 구조물
+ 무기
+ 탄약
+ 방어
+ 연료
+ 배터리
+ Utility
```

첫 구현 P0 공식:

```text
TotalVehicleMassKg
= BaseVehicleMassKg
+ Σ(TurretMountMassKg)
+ Σ(WeaponMassKg)
+ AmmoMassKg
+ DefenseMassKg
```

연료·배터리·Utility는 구현 전까지 0 또는 명시적 미지원으로 둔다.
가상의 기본 질량을 임의로 합산하지 않는다.

중량 영향의 장기 방향:

| 성능 | 방향 |
|---|---|
| 가속 | 무거울수록 저하 |
| 제동 | 무거울수록 제동거리 증가 |
| 선회 | 무거울수록 반응 둔화 |
| 최고속도 | 일정 중량 이상에서 감소 가능 |
| 충돌 | 무거운 차량이 일부 유리할 수 있음 |
| 반동 안정성 | 무거운 차량이 대구경 운용에 일부 유리할 수 있음 |
| 연료 소비 | 무거울수록 증가 가능 |

P0는 가속·제동·선회 변화부터 검증한다.
최고속도·충돌·반동·연료 소비는 후속이다.

---

## 7. 별도 Feature P0 범위

`CF-FQ-034`와 `CF-FQ-035`의 연동 P0는 아래로 제한한다.

```text
01. VehicleData와 분리된 Saved Fitting Template
02. MountProfile별 EquipmentPreset 선택
03. Hardpoint·MountType·WeaponSize 호환 검증
04. VehicleDefenseData 패키지 선택 또는 None
05. AmmoData별 출격 탄수 선택과 질량 연동
06. Base·Mount·Weapon·Ammo·Defense 질량 Breakdown
07. TotalVehicleMass와 탑재 한도 검증
08. 검증된 Applied Fitting Snapshot
09. 출격 초기 1회 장비·방어·탄약 적용
10. 실제 질량 또는 제한된 Mobility Adapter를 통한 가속·제동·선회 변화
11. Item Instance·VehicleCargo·Mounted 소유 상태
12. Reservation과 Atomic Transfer
13. 비전투·쿨타임 종료·차량 정지 조건의 시간 소모 Equip·Unequip
14. 호환되는 빈 Mount Slot에만 Equip
15. 이동·전투·피해·상태 변경 시 취소와 원상 유지
16. Preview·Debug용 읽기 전용 ViewData
17. Default·Light·Heavy·Invalid·Cancelled Field Action 검증
```

P0의 목표는 장비 종류 수가 아니라 다음 변환의 신뢰성이다.

```text
선택
→ 호환 검증
→ 질량 계산
→ 출격 적용
→ 기동성 변화
```

---

## 8. P0 제외 범위

```text
- 완성형 차고와 전체 게임플로우
- 구매·판매·가격·재화
- PlayerStorage 전체 UX와 다중 차량 공유 창고
- 월드 루팅·보상·제작·내구도·수리
- SaveGame·재접속 영구 저장
- 해금·연구
- 연료 탱크와 실제 연료 소비
- 배터리·발전기·전력망
- Utility Runtime
- 방향별 장갑판 아이템 조립
- 전투 상태 또는 이동 중 즉시 장비 Hot Swap
- Occupied Slot에 새 장비 직접 덮어쓰기
- 탄약 한 발마다 실시간 질량 갱신
- 장착 위치별 중심질량 이동
- AI 자동 피팅
- 네트워크 권한과 복제
```

기존 초안에서 P0로 잡았던 연료·배터리·Utility 1종과 네 종류 무기 동시 제공은 실제 기반이 준비될 때까지 후속으로 이동한다.

---

## 9. 피팅 선택의 교환 관계

| 선택 | 장점 | 단점 |
|---|---|---|
| 가벼운 장비 | 가속·선회 유리 | 화력·내구 선택 제한 가능 |
| 중장비 | 화력·공간 통제 | 가속·제동·선회 부담 |
| 방어 패키지 | 생존력 증가 | 질량 증가 |
| 탄약 다량 | 장기전 유리 | 질량 증가 |
| 탄약 소량 | 기동성 유리 | 지속 전투력 감소 |
| 대구경 무기 | 한 발 위력 | 무기·마운트 질량 증가 |
| 소형 무기 | 빠른 반응 | 장갑 대응 약화 가능 |

중량은 무조건 단점만은 아니다.
다만 P0에서는 무거움의 장점인 충돌·반동 안정성을 아직 구현하지 않으므로, 밸런스 판정은 임시라는 점을 명시한다.

---

## 10. 차급과 피팅 방향

| 차급 방향 | 피팅 정체성 |
|---|---|
| 경량 차량 | 적게 싣고 빠르게 움직이며 각속도 우위를 만드는 빌드 |
| 중형 차량 | 중거리 주력, 상황 대응, 무기·탄약·방어 균형 빌드 |
| 중량 차량 | 방어, 대구경, 지속전과 공간 통제 빌드 |

P0 첫 검증은 여러 차급 완성이 아니라 한 기준 차량에서 `Light / Default / Heavy` 피팅의 차이를 확인하는 방식으로 수행한다.

---

## 11. 데이터 책임 원칙

```text
VehicleData
= 플랫폼, 하드포인트, 장착 제한, 기본 구성

VehicleFittingData
= Saved Template과 Legacy 출격 선택

Inventory Foundation
= Item Instance, VehicleCargo·Mounted 소유 상태, 접근 조회, Reservation, Atomic Transfer

Field Fitting Draft
= 실제 Item Instance 기반 편집 중 선택과 시간 액션 입력

EquipmentPresetData
= TurretMountData + WeaponData 조합

AmmoData / VehicleAmmoComp
= 탄약 정적 정보와 런타임 수량

VehicleDefenseData / VehicleDefenseComp
= 방어 정적 정보와 런타임 상태

VehicleFittingSnapshot
= 해석된 참조, 검증 결과와 질량

VehicleRuntime
= Snapshot을 실제 차량에 적용
```

UI는 호환성과 질량을 재계산하지 않고 Snapshot·ViewData를 표시한다.

---

## 12. P0 피팅 UI 요구

| UI 항목 | 표시 내용 |
|---|---|
| 장착 위치 | MountProfile과 Hardpoint 위치 |
| 장비 선택 | EquipmentPreset·TurretMount·Weapon |
| 호환 상태 | MountType·Size·참조 유효성 |
| 방어 | 선택 VehicleDefenseData와 질량 |
| 탄약 | 탄종별 출격 적재량·최대량·질량 |
| 질량 Breakdown | Base·Mount·Weapon·Ammo·Defense |
| 총중량 | 현재 총중량 / 최대 허용 총중량 |
| 성능 Preview | 가속·제동·선회 변화 |
| 적용 상태 | Valid·Warning·Invalid·Applied |
| 오류 이유 | 구체적인 검증 실패 메시지 |

피팅 기능은 ViewData를 제공한다.
UI Root와 화면 프레임은 `CF-FQ-032`와 조율한다.

---

## 13. 확정 원칙

```text
01. 차량은 클래스가 아니라 플랫폼이다.
02. 피팅은 전투 방식을 결정해야 한다.
03. 중량은 첫 피팅 구현의 핵심 비용이다.
04. VehicleData 원본과 플레이어 출격 선택을 분리한다.
05. 현재 HardpointSlot과 MountProfile 계약을 재사용한다.
06. EquipmentPresetData가 장비 조합의 단일 진입점이다.
07. 탄약 Runtime은 CF-FQ-031이 소유한다.
08. 방어 Runtime은 CF-FQ-033이 소유한다.
09. 피팅은 선택·호환·질량·시간 액션·적용 Snapshot을 소유한다.
10. Inventory Foundation은 실제 Item 소유권·Container·Reservation·Atomic Transfer를 소유한다.
11. 오류·취소가 있으면 부분 적용하지 않고 기존 Applied Snapshot과 Inventory 상태를 보존한다.
12. 필드 Equip·Unequip은 비전투·관련 쿨타임 종료·차량 정지·예약 성공 상태에서만 가능하다.
13. Equip은 호환되는 Empty Slot에만 가능하고 교환은 Unequip 후 별도 Equip으로 수행한다.
14. 기존 차량은 FittingData 미지정 시 현재 기본 구성을 유지한다.
```

---

## 14. 미결정 사항

| ID | 미결정 질문 | 현재 권장안 |
|---|---|---|
| `FIT-D-001` | 차체 기준 질량 SSOT | VehicleData 명시값 + 실제 Body 질량 검증 |
| `FIT-D-002` | 무기 질량 소유 위치 | WeaponData |
| `FIT-D-003` | 방어 질량 단위 | P0는 VehicleDefenseData 단일 패키지 질량 |
| `FIT-D-004` | 최대 한도 기준 | MaximumGrossMass 단일 SSOT |
| `FIT-D-005` | 피팅 저장 단위 | 별도 VehicleFittingData |
| `FIT-D-006` | 런타임 소유자 | VehicleFittingComp |
| `FIT-D-007` | 실제 Chaos 질량 적용 방식 | 초기 1회 실제 질량 적용 후 필요 시 제한 보정 |
| `FIT-D-008` | 가속·제동·선회 공식 | 기준 질량 비율·Exponent·Clamp 데이터화 |
| `FIT-D-009` | 탄약 소모 질량 갱신 | P0 출격 초기 고정 |
| `FIT-D-010` | 누락 MountSelection 처리 | VehicleData 기본 장비 유지 |
| `FIT-D-011` | Defense None 허용 | Legacy·경량 빌드를 위해 허용 |
| `FIT-D-012` | 첫 테스트 차량 | 원본을 보호하는 별도 Test VehicleData |
| `FIT-D-013` | UI 구현 선행 관계 | 피팅 ViewData 우선, CF-FQ-032 화면 프레임 연동 |
| `FIT-D-014` | 과적 허용 여부 | P0는 최대 총중량 초과 적용 금지 |

상세 근거와 결정 게이트는 `VehicleFittingDesign.md`와 `VehicleFittingPlan.md`를 따른다.

---

## 15. 현재 기능 상태

```text
Feature: CF-FQ-034 차량 피팅·질량 런타임
Support Feature: CF-FQ-035 인벤토리 Foundation
Priority: P1
Status: Ready Plans
FIT-P0-00~03: Done
FFIT-P0-00: Field Action Contract Done
INV-P0-00: Inventory Contract Done
Next Fitting: FIT-P0-04 Sortie Apply Runtime
Next Inventory: INV-P0-01 Item Identity and Definition
Source: Field·Inventory 계약에서는 Not Modified
Blueprint: Not Modified
Asset: Not Modified
Build: FIT-P0-03 포함 후속 공식 Editor Build PASS
Automation: 최신 전체 32/32 Success
PIE: Field Fitting Not Run
Current Active: CF-FQ-032 유지
```

이번 문서 작업의 직접 AssetDump 재조회는 `WinError 87`로 실패했다.
따라서 신규 에셋 검증 PASS를 기록하지 않으며, 실제 코드·에셋 착수 전에 대상 VehicleData와 EquipmentPresetData를 다시 확인한다.

---

## 16. Changelog

### v0.3.0 - 2026-08-01

- `CF-FQ-035 인벤토리 Foundation`을 피팅과 분리된 별도 Ready 기능으로 연결했다.
- Item Instance, VehicleCargo·Mounted 소유 상태, Reservation과 Atomic Transfer를 최소 Inventory P0로 추가했다.
- 비전투·전체 관련 쿨타임 종료·차량 정지·예약 성공 조건의 시간 소모 Field Equip·Unequip을 피팅 P0에 추가했다.
- Equip은 호환되는 빈 슬롯에만 가능하며 교환은 Unequip과 Equip 두 액션으로 분리했다.
- 이동·전투·피해·상태 변경·예약 손실·화면 종료 취소 시 원상 유지 계약을 확정했다.
- 상점·루팅·경제·SaveGame과 전투 중 Hot Swap은 제외 범위로 유지했다.
- Source, Blueprint와 Unreal Asset은 수정하지 않았다.

### v0.2.0 - 2026-07-31

- 현재 VehicleData·Hardpoint·MountProfile·EquipmentPreset·Weapon·Ammo·Defense·질량 구현 상태를 반영했다.
- 별도 기능 `CF-FQ-034 차량 피팅·질량 런타임`과 대표 Plan·Design·Roadmap을 연결했다.
- 장기 피팅 방향과 첫 구현 P0를 분리했다.
- P0를 별도 출격 피팅, 호환 검증, 질량 Snapshot, 초기 적용과 가속·제동·선회 검증으로 축소했다.
- 연료·배터리·Utility·경제·영구 저장·전투 중 교환을 P0에서 제외했다.
- 질량 SSOT·Chaos 적용·기동 공식 등 미결정 사항을 별도 목록으로 정리했다.
- Source와 Asset은 수정하지 않았다.

### v0.1 - 2026-06-08

- 차량 피팅 시스템 초안 작성
- 슬롯, 하드포인트, 중량 비용의 역할 분리
- 초기 P0 피팅 범위 정의

---

## 17. Migration

```text
- v0.3.0부터 피팅의 실제 Item 소유권은 CF-FQ-035 Inventory Foundation을 사용한다.
- 기존 VehicleFittingData의 DataAsset 참조는 Saved Template·Legacy 입력으로 유지한다.
- Field Fitting은 INV-P0-03 Reservation·Atomic Transfer와 INV-P0-04 Fitting Adapter 없이는 실제 Item 이동을 구현하지 않는다.
- 필드 질량 재적용은 FIT-P0-05 Runtime Mass Gate 없이는 구현하지 않는다.
- 연료·배터리·Utility와 네 종류 무기 동시 제공은 취소가 아니라 후속 범위다.
- 현재 VehicleData 기본 EquipmentPreset은 피팅 Runtime이 구현될 때까지 기존 동작을 유지한다.
- 피팅 구현은 CF-FQ-031 Ammo와 CF-FQ-033 Defense의 소유권을 침범하지 않는다.
- 사용자 승인과 구현·검증 전에는 CF-FQ-034·035를 Active·Done 또는 Current System으로 해석하지 않는다.
