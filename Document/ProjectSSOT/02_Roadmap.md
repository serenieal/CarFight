# CarFight — 02_Roadmap

> 역할: CarFight 프로젝트의 **현재 사이클 목표 / 거시 진행 순서 / dependency / Candidate ordering**을 고정한다.
> 기준 상태 문서: `01_ProjectState.md`
> 상위 방향 문서: `00_Vision.md`
> 문서 버전: v2.30.13
> 마지막 정리(Asia/Seoul): 2026-09-17
> 문서 상태: Current / Work Lifecycle Slice UDS Authority0

---

## 1. 이번 사이클의 목표

> **싱글 플레이 기준 차량 전투를 중심으로 현재 구현된 차량·무기·피격·방어·센서·HUD 기반을 안정적으로 이어 붙이고, 남은 USER 검증과 핵심 게임 루프 품질을 단계적으로 닫는다.**

서버 권한 전투, Dedicated Server 검증, 2클라이언트 테스트, 세션/로비와 서버 운영 기능은 현재 사이클에서 제외한다.

---

## 2. 현재 Active / Ready Work — UDS Reference

현재 Active/Ready Work lifecycle의 canonical authority는 `Document/UDS/records/**`다. bounded human view는 `Document/UDS/derived/Current.md`를 사용한다.

이 Roadmap은 cycle 목표, sequencing, 다음 후보와 deferred scope를 계속 소유한다. 다만 UDS로 승격된 Work의 current state/phase/next를 독립적으로 소유하거나 dual-write하지 않는다.

현재 Active/Ready Work의 exact 목록과 state/phase/next는 Roadmap에 복제하지 않는다. 필요할 때 `Document/UDS/derived/Current.md`를 탐색 힌트로 fresh canonical Work를 확인하며, Ready Work가 자동으로 Active가 되지 않는 정책은 유지한다.

---

## 3. 완료 기반

현재 사이클에서 다시 기초부터 열지 않는 대표 완료 기반은 다음과 같다.

| 영역 | 대표 완료 Feature / Current owner |
| --- | --- |
| 로컬 조준·발사·Reticle | `CF-FQ-013`, `016`, `017`, `022`, `025` / VehicleAim·WeaponFire·AimReticle |
| Projectile·피해·FX | `CF-FQ-018`, `023`, `024`, `027`, `028` / Projectile·HitDamage·CombatFx |
| 모듈형 런처·발사 인계 | `CF-FQ-029` / Launcher·WeaponFire·Projectile |
| 물리 제한형 미사일 비행·유도 | `CF-FQ-030` / MissileGuidance·Projectile |
| 탄약 | `CF-FQ-031` / Ammo |
| 방어 | `CF-FQ-033` / VehicleDefense·HitDamage |
| 타겟 선택 | `CF-FQ-026` / TargetSelect / USER feel·Debug debt는 필요 시 별도 lifecycle |
| 센서·Scanner | `CF-FQ-036`, `037` / SensorContact |
| 인게임 HUD/UI | `CF-FQ-032` / InGameUI·AimReticle·SensorContact |
| 무기 Data | `CF-FQ-008` / WeaponData |
| 차량 제작·Authoring·Performance Tuning | `CF-FQ-015`, `038`, `040`, `042`, `043`, `044`, `047` / VehicleBuilder + VehicleData |
| 차량 Fitting·Mass Runtime | `CF-FQ-034` / VehicleRuntime + VehicleData / USER feel tuning은 VehicleBuilder |
| Inventory Foundation | `CF-FQ-035` / VehicleInventory + VehicleRuntime / formal ownership-aware USER frontend는 필요 시 새 lifecycle |

완료 evidence의 세부 Build ID, Automation 결과와 USER Acceptance는 각 대표 Plan을 우선한다. 새 관련 failure evidence 없이 완료 검증을 습관적으로 반복하지 않는다.

---

## 4. 지금 선택 가능한 작업

현재 자동 우선순위를 강제하지 않는다. 아래 항목은 각각 독립적인 재개 후보이며 사용자가 고르면 Active로 승격한다.

승격된 Ready/Paused Work의 정확한 lifecycle과 재개 Gate는 UDS Current를 탐색 힌트로 fresh canonical Work에서 확인한다. 승격 전 Candidate의 우선순위와 착수 판단은 FeatureQueue가 소유하며, 사용자 착수 결정 뒤 실제 lifecycle 전환은 immutable UDS successor로 기록한다.

Production Visual 후속이 현재 실행 중인지와 정확한 Gate는 UDS가 소유한다. Roadmap은 `CF-FQ-032 Done` 기반을 되돌리지 않고 후속 Visual 작업이 별도 lifecycle에서 진행된다는 거시 의존성만 유지한다.

---

## 5. 가까운 Candidate

| Feature | 상태 | 목적 |
| --- | --- | --- |
| `CF-FQ-012` 1대 차량 주행감 고도화 | Candidate | VehicleBuilder Performance Tuning Protocol을 사용해 현재 차량의 주행 감각 자체를 개선. CF-FQ-015의 Validator/Compare/Runtime Apply foundation을 다시 만들지 않는다. |
| `CF-FQ-014` WheelSync 시각 품질 폴리싱 | Candidate | 고속 휠·조향·서스펜션 시각 품질 개선 |
| `CF-FQ-020` 조작감/전투 템포/피드백 개선 | Candidate | 기능 구현 이후 전투 체감 개선 |
| `CF-FQ-021` 핵심 게임 루프 검증 | Candidate | 현재 싱글 차량 전투가 다음 단계로 갈 수 있는지 종합 판정 |

`CF-FQ-019 주행/전투 반복 테스트`는 런처·미사일까지 포함한 통합 회귀 범위를 다시 설계하기 전까지 Deferred다.

---

## 6. 진행 원칙

### 6.1 완료된 기반을 다시 구현하지 않는다

새 failure evidence나 실제 계약 변경이 없으면 완료된 Foundation/Build/Automation/USER Gate를 재실행하지 않는다.

### 6.2 Technical PASS와 USER PASS를 분리한다

AI가 RuntimeRead나 Automation으로 판정할 수 있는 기술 사실은 Technical PASS로 닫을 수 있다. 시각 품질, UX, 조작감, 주행감, 조준감과 연출 감각은 사용자 확인 전 USER PASS로 확대하지 않는다.

### 6.3 Feature 전체와 부분 Current System을 구분한다

일부 Systems 문서나 기술 Foundation이 존재해도 남은 USER Gate가 있는 Feature를 자동 Done 처리하지 않는다.

### 6.4 ProjectSSOT는 상세 로그를 보관하지 않는다

Roadmap은 진행 순서와 선택지에 집중한다. Feature별 세부 P0 목록, Build UUID, Automation run history, 긴 RCA는 대표 Plan이 소유한다.

---

## 7. 후순위 / Deferred

다음 범위는 현재 싱글 플레이 완성보다 뒤에 둔다.

```text
- 서버 권한 전투
- Dedicated Server / 2클라이언트 검증
- 세션 / 로비
- 서버 운영·관리 기능
- 네트워크용 차량 동기화 확장
- 장기 저장·경제·보상·제작 시스템
```

기존 문서와 구현 흔적은 삭제하지 않고 Deferred/Icebox 또는 Historical로 유지한다.

---

## 8. Roadmap 갱신 조건

이 문서는 다음 경우에만 갱신한다.

```text
- Work lifecycle 변화가 현재 사이클의 거시 진행 순서나 dependency를 실질적으로 바꾼다.
- Candidate 우선순위나 dependency가 바뀐다.
- 완료 기반 변화가 다음 단계의 거시 순서를 바꾼다.
- 현재 사이클의 목표가 바뀐다.
```

Active/Ready state가 바뀌었다는 사실만으로 Roadmap을 갱신하지 않는다. 정확한 lifecycle 변화는 UDS가 소유한다.

개별 Build PASS, 작은 bugfix, Automation run 하나가 추가됐다는 이유만으로 Roadmap에 상세 로그를 붙이지 않는다.

---

## 9. Changelog

### v2.30.13 - 2026-09-17

- Roadmap 역할을 cycle goal·거시 sequencing·dependency·Candidate ordering으로 좁히고 Active/Ready exact 목록 mirror를 제거했다.
- Ready/Paused 선택 경로에서 legacy `ActiveWork.md` 참조를 제거하고 UDS Current 탐색 힌트 → fresh canonical Work를 사용하도록 교정했다.
- Candidate 우선순위·착수 판단은 FeatureQueue, 실제 lifecycle 전환은 immutable UDS successor가 소유하도록 분리했다.
- Active/Ready state 변화 자체는 Roadmap 갱신 사유가 아니며 macro sequence/dependency가 실제로 변할 때만 갱신하도록 write frequency를 축소했다.

### v2.30.12 - 2026-09-16

- UDS-08 MIG-05 cutover에 따라 Active/Ready Work lifecycle slice를 authority0 UDS reference로 전환했다.
- cycle goal, sequencing, candidate ordering과 deferred scope는 Roadmap이 계속 소유하며 current Work state/phase/next는 `Document/UDS/records/**`에 dual-write하지 않는다.
- current lifecycle 확인은 `Document/UDS/derived/Current.md`에서 시작한다.

### v2.30.11 - 2026-09-14

- `CF-FQ-026 타겟 선택 시스템`을 current Source/Asset/System Rebaseline 결과 완료 기반에 편입하고 Paused 선택지에서 제거했다.
- Candidate/Selected Actor authority는 `TargetSelect.md v1.0.0`, Sensor Detection/Contact/Knowledge는 `SensorContact.md v1.2.0`이 소유하며 과거 전체 World 반복 후보 수집은 TargetRegistry successor 구조로 해결됐다.
- old `TS-P0-08 USER PIE`는 USER PASS가 아니라 `Superseded / Not Executed`다. 동일 차량 TargetPoint USER 상태는 Inconclusive, 7°/1200m 후보 체감과 Debug Sphere 가시성은 Deferred이며 필요 시 Current TargetSelect 기준 새 tuning/UX lifecycle로 연다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v2.30.10 - 2026-09-14

- `CF-FQ-035 인벤토리 Foundation`을 current Source/System Rebaseline 결과 완료 기반에 편입하고 Paused 선택지에서 제거했다.
- ItemInstance ownership, VehicleCargo/MountedEquipment, Access/Capacity, Reservation, Atomic Transfer/Rollback, ViewData와 Inventory→Fitting 경계는 `VehicleInventory.md v1.0.0` Current Foundation이 소유한다.
- `FFIT-P0-05 Field Fitting UI and PIE`는 USER PASS가 아니라 `Superseded / Not Executed`이며, 현재 RuntimeApply non-owning UX와 구분해 formal ownership-aware frontend가 실제 요구될 때 별도 lifecycle로 다룬다. 현재 단일 Active `CF-FQ-039`와 다른 재개 후보 순서는 변경하지 않았다.

### v2.30.9 - 2026-09-14

- `CF-FQ-034 차량 피팅·질량 런타임`을 current Source/System Rebaseline 결과 완료 기반에 편입하고 Paused 선택지에서 제거했다.
- `FIT-P0-07A~07C` quantitative Mobility evidence는 Historical Technical PASS로 보존하며 `FIT-P0-07D USER Driving Feel Comparison`은 USER PASS가 아니라 `Superseded / Not Executed`다.
- 현재 Fitting/Mass Runtime은 `VehicleRuntime.md v1.3.0` + `VehicleData.md v2.3.0`, 향후 질량 체감 튜닝은 `VehicleBuilder.md v1.6.0`이 소유한다. 현재 단일 Active `CF-FQ-039`와 다른 재개 후보 순서는 변경하지 않았다.

### v2.30.8 - 2026-09-11

- `CF-FQ-015 Vehicle Data Tuning`을 Rebaseline Complete로 완료 기반에 편입하고 Paused 재개 후보에서 제거했다.
- VD-P0-00~03 Historical Technical PASS는 보존하며 VD-P0-04는 USER PASS가 아니라 `Superseded / Not Executed`로 종료했다. Current tuning owner는 `Systems/Vehicles/VehicleBuilder.md v1.6.0`, data foundation은 `VehicleData.md v2.3.0`이다.
- `CF-FQ-012`가 향후 착수되면 VehicleBuilder Performance Tuning Protocol을 사용하며 CF-FQ-015 foundation을 중복 구현하지 않는다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v2.30.7 - 2026-09-11

- `CF-FQ-038 Vehicle Data Authoring`을 DEL1~DEL7 PASS / Legacy Wizard retirement / 전체 DataAuthoring 111/111 PASS로 완료 기반에 편입하고 Paused 재개 후보에서 제거했다.
- 현재 차량 제작·Authoring 완료 기반은 `Systems/Vehicles/VehicleBuilder.md v1.5.0`이 소유한다. UA-08 quantitative comparison은 비차단 Deferred observational debt이며 USER PASS 7/8을 확대하지 않는다.
- 현재 단일 Active `CF-FQ-039 Production UI Visual Rework`와 다른 재개 후보의 순서는 변경하지 않았다.

### v2.30.6 - 2026-09-11

- `CF-FQ-029`을 LM-P0-06 Final Technical Integration PASS / Current System Promotion으로 완료 기반에 편입하고 Paused 재개 후보에서 제거했다.
- `CF-FQ-030`도 이미 완료된 `MissileGuidance` Current System 상태에 맞춰 stale Ready/Manual PIE 후보에서 제거했다.
- Launcher→Projectile/Missile Guidance의 완료 기반을 Roadmap에 명시했으며 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v2.30.5 - 2026-08-24

- CF-FQ-039의 현재 진행을 VPR-P0-01 Armor Technical PASS / USER Visual Review Pending으로 전진했다.
- 대표 Plan/Roadmap 포인터를 v0.1.5로 동기화하고 전체 `VT-VEH-01`은 USER 명시 승인 전이라 Pending으로 유지했다.

### v2.30.4 - 2026-08-24

- CF-FQ-039 VehiclePanel의 editable Structure Readiness PASS와 `VT-VEH-01` USER 명시 승인 대기 상태를 프로젝트 Roadmap에 반영했다.
- 기존 `WBP_CFArmorSector` 6개 재사용 + Designer-owned BodyMap Canvas + 탑다운 VehicleSilhouette를 유지하며 신규 슬롯 위젯 중복 생성 없이 진행하도록 했다.

### v2.30.3 - 2026-08-23

- CF-FQ-039 VPR-P0-00 AI Recovery 완료와 USER Vehicle Target Pending 상태를 프로젝트 Roadmap에 반영했다.

### v2.30.2 - 2026-08-23

- CF-FQ-039 VPR-P0-00 실제 Target Recovery 착수를 반영해 대표 Plan/Roadmap과 Gate 상태를 갱신했다.

### v2.30.1 - 2026-08-23

- CF-FQ-039 pre-start 설계 교정 후 대표 Plan/Roadmap 포인터를 v0.1.1로 동기화했다.

### v2.30.0 - 2026-08-23

- 사용자 선택에 따라 `CF-FQ-039 Production UI Visual Rework`를 현재 단일 Active로 반영했다.
- 첫 Gate를 `VPR-P0-00 Visual Target Recovery & Registry`로 연결하고, 과거 승인 이미지를 실제 Visual Target으로 고정한 뒤 Production Art에 들어가는 순서를 프로젝트 Roadmap에 반영했다.
- CF-FQ-032의 Deferred Visual 후속은 새 CF-FQ-039 범위로 이동하되 CF-FQ-032 Done과 완료 evidence는 그대로 유지했다.

Migration: 현재 UI Visual 작업은 `Document/Plan/InGameUIVisual/InGameUIVisualRoadmap.md`의 VPR 순서를 사용한다. CF-FQ-032의 Historical UI-P0 순서를 현재 작업 순서로 사용하지 않는다.

### v2.29.0 - 2026-08-22

- stale `CF-FQ-032 Active / UI-P0-03` 로드맵을 제거하고 `CF-FQ-032 Done / 현재 Active 없음`으로 정상화했다.
- 완료 Feature의 상세 P0 체크리스트와 Build/Automation 로그를 제거하고 현재 선택 가능한 Ready/Paused/Candidate 중심으로 재구성했다.
- `CF-FQ-038`은 UA-01~06 USER PASS 보존 / 다음 UA-07, `CF-FQ-034`는 FIT-P0-07D, `CF-FQ-029`는 LM-P0-06, `CF-FQ-030`은 CF-TC-027 Manual PIE 기준으로 교정했다.
- Roadmap이 다시 완료 로그 저장소로 비대해지지 않도록 갱신 조건을 명시했다.

Migration: 삭제된 상세 실행 evidence는 각 대표 Plan과 Systems, Git history가 계속 소유한다. 과거 Roadmap의 Historical "현재 Active" 섹션은 현재 작업 복원 기준으로 사용하지 않는다.
