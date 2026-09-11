# CarFight — 02_Roadmap

> 역할: CarFight 프로젝트의 **현재 해야 할 일 / 우선순위 / 완료 조건 / 진행 순서**를 고정한다.
> 기준 상태 문서: `01_ProjectState.md`
> 상위 방향 문서: `00_Vision.md`
> 문서 버전: v2.30.6
> 마지막 정리(Asia/Seoul): 2026-09-11
> 문서 상태: Current

---

## 1. 이번 사이클의 목표

> **싱글 플레이 기준 차량 전투를 중심으로 현재 구현된 차량·무기·피격·방어·센서·HUD 기반을 안정적으로 이어 붙이고, 남은 USER 검증과 핵심 게임 루프 품질을 단계적으로 닫는다.**

서버 권한 전투, Dedicated Server 검증, 2클라이언트 테스트, 세션/로비와 서버 운영 기능은 현재 사이클에서 제외한다.

---

## 2. 현재 Active

현재 단일 Active Feature는 **`CF-FQ-039 Production UI Visual Rework`**다.

```text
대표 Plan: Document/Plan/InGameUIVisual/InGameUIVisualPlan.md v0.1.5
Roadmap: Document/Plan/InGameUIVisual/InGameUIVisualRoadmap.md v0.1.5
Current Gate: VPR-P0-01 VehiclePanel Production Vertical Slice — ARMOR TECHNICAL PASS / USER VISUAL REVIEW PENDING
```

`CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`는 Done을 유지한다. 이번 작업은 기존 기능 기반을 다시 여는 대신 승인 Visual Target → Art Breakdown → Production Asset → Designer → Visual Match 순서로 시각 품질만 후속 정리한다.

Feature 상태의 전체 목록은 `03_FeatureQueue.md`, 세션 복원 체크포인트는 `Document/ActiveWork.md`를 우선한다.

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
| 센서·Scanner | `CF-FQ-036`, `037` / SensorContact |
| 인게임 HUD/UI | `CF-FQ-032` / InGameUI·AimReticle·SensorContact |
| 무기 Data | `CF-FQ-008` / WeaponData |

완료 evidence의 세부 Build ID, Automation 결과와 USER Acceptance는 각 대표 Plan을 우선한다. 새 관련 failure evidence 없이 완료 검증을 습관적으로 반복하지 않는다.

---

## 4. 지금 선택 가능한 작업

현재 자동 우선순위를 강제하지 않는다. 아래 항목은 각각 독립적인 재개 후보이며 사용자가 고르면 Active로 승격한다.

| Feature | 상태 | 재개 지점 | 성격 |
| --- | --- | --- | --- |
| `CF-FQ-038` 차량 Data Authoring | Paused | `UA-07 Driving Feel Authoring` | UA-01~06 USER PASS 보존, Authoring 사용성·주행감 계속 검증 |
| `CF-FQ-034` 차량 피팅·질량 | Paused | `FIT-P0-07D USER Driving Feel Comparison` | 정량 Mobility 이후 실제 체감 비교 |
| `CF-FQ-035` 인벤토리 Foundation | Paused | USER Field UI·Mobility | Field Fitting 사용자 흐름 |
| `CF-FQ-026` 타겟 선택 | Paused | `TS-P0-08 USER PIE` | 실제 선택 범위·표시·조작감 |
| `CF-FQ-015` 차량 데이터 튜닝 | Paused | `VD-P0-04 USER Tuning` | 실제 차량 주행 튜닝 |

UI의 `Radar/Edge Visual·Zoom Feel`과 D1-11-ART 잔여 Visual Review를 포함한 Production Visual 후속은 현재 `CF-FQ-039`로 선택됐다. `CF-FQ-032 Done`을 되돌리지 않고 새 Visual Plan의 `VPR-*` Gate에서 진행한다.

---

## 5. 가까운 Candidate

| Feature | 상태 | 목적 |
| --- | --- | --- |
| `CF-FQ-012` 1대 차량 주행감 고도화 | Candidate | 현재 차량의 주행 감각 자체를 개선 |
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
- 현재 Active가 바뀐다.
- Feature의 Ready/Paused/Done/Deferred 상태가 프로젝트 진행 순서를 실질적으로 바꾼다.
- 우선순위나 dependency가 바뀐다.
- 현재 사이클의 목표가 바뀐다.
```

개별 Build PASS, 작은 bugfix, Automation run 하나가 추가됐다는 이유만으로 Roadmap에 상세 로그를 붙이지 않는다.

---

## 9. Changelog

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
