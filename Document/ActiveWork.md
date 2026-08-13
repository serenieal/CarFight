# CarFight Active Work

- 문서 버전: v2.48
- 최근 갱신일: 2026-08-13
- 문서 상태: Current
- 역할: CarFight 게임 프로젝트에서 현재 실제로 진행 중인 작업을 선택하고 대표 Plan으로 연결하는 세션 복원 projection

---

## 1. 운영 원칙

이 문서는 `main_game`의 현재 작업 선택만 관리한다.
AssetDump와 GoPyMCP의 내부 lifecycle은 각 독립 저장소 문서를 사용한다.

```text
FeatureQueue
= 기능 우선순위와 Active/Paused/Ready/Done 상태

ActiveWork
= 현재 Active/Paused 작업 선택 + 대표 Plan + next gate projection

대표 Plan
= 상세 체크포인트와 validation/evidence owner

Systems
= 완료 기능의 현재 구현

Archive Index
= Historical Plan 탐색
```

이 문서에는 대표 Plan의 전체 Build ID, Automation run history, USER PIE 세부 로그와 과거 override를 복제하지 않는다.

---

## 2. 현재 단일 Active

```text
work_id: CF-FQ-032
name: 인게임 전투 HUD 및 UI 프레임워크
status: Active / UI-P0-02 USER PASS / UI-P0-03 Common Launcher Technical PASS / TestMap_DRSalvo USER PIE Pending
representative_plan: Document/Plan/InGameUIPlan.md v0.58.1
feature_queue: Document/ProjectSSOT/03_FeatureQueue.md
```

### 현재 next gate

1. 공통 Weapon/Launcher Presentation lifecycle 교정은 적용 완료다. Salvo 전용 v1.5.0·v1.6.0 시간 Hold 상태·함수·변수는 제거했고, `LauncherSequenceRevision`으로 실제 Launcher 이벤트와 Ammo/Timer 부수 Refresh를 구분한다.
2. HeavyCannon SingleCycle은 공통 Weapon Status의 `Cooldown → READY`, Ripple·Salvo는 동일한 `Active Sequence → terminal Snapshot 1회 → Cooldown → READY` 전이를 사용한다. `FirePattern`은 `RIPPLE` / `SALVO` 표시 문구 선택에만 관여한다.
3. 공식 UE 5.8 Editor Build `6978e029bfaa48c7addfc9acd87484b1`은 Exit Code 0으로 PASS했다.
4. 전체 `CarFight` Automation `00ee23a01aa7480cabc557f312780ae8`은 64/64 PASS, 필수 32/32 Success, 실패 0으로 PASS했다.
5. 다음 Gate는 `/Game/Maps/TestMap_DRSalvo` Launcher Presentation USER PIE다. `SALVO 진행 → SALVO 4 / 4 → Cooldown → READY` 순서를 사용자가 직접 확인한다.
6. Launcher 전체 USER PASS 후에만 Defense 실제 변화 → Pawn Rebind 순서로 검증한다.
7. 위 USER Gate 전에는 `UI-P0-03` 또는 `CF-FQ-032`를 완료 처리하지 않는다.

### 이미 보호하는 완료 증거

- `UI-P0-02` Runtime Lifetime USER PASS.
- UI-P0-03 부분 USER PASS: 실제 Speed 증가, Weapon Cooldown `READY → 남은 초 → READY`, Target 선택·해제.
- `CF-FQ-031` Ammo는 Done / Current System이며 Heavy·Ripple Ammo USER PIE를 반복하지 않는다.
- `CF-FQ-033` Defense는 Done / Current System이며 Shield·Armor·Integrity 공식을 UI 작업에서 변경하지 않는다.
- D1-11 Production Structure Gate는 PASS이며 D1-11-ART 세부 시각 폴리시·Art Import·DA 연결은 기능 개발 비차단 후순위다.

### 보호 범위

```text
CF-FQ-031 Ammo Current
CF-FQ-033 VehicleDefense / HitDamage Current
CF-FQ-029 Launcher Paused checkpoint
CF-FQ-026 TargetSelect Paused checkpoint
CF-FQ-030 Missile Ready checkpoint
CF-FQ-034 Fitting Ready checkpoint
CF-FQ-035 Inventory Ready checkpoint
승인 VehiclePanel / WeaponPanel / Reticle 의미 계약
기존 dirty source / assets / user work
```

---

## 3. Paused 작업

| Work ID | 상태 | 대표 Plan | 재개 지점 |
| --- | --- | --- | --- |
| `CF-FQ-029` | Paused | `Document/Plan/LauncherMissilePlan.md v0.10.0` | `LM-P0-06` Launcher Integration PIE |
| `CF-FQ-026` | Paused | `Document/Plan/TargetSelectPlan.md` | `TS-P0-08` 후보 표시·범위·디버그 시각성·튜닝 |

Paused 작업의 상세 Build/Automation/PIE evidence는 각 대표 Plan이 소유한다.

---

## 4. Ready 기능은 ActiveWork에 복제하지 않음

현재 Ready 후보는 `Document/ProjectSSOT/03_FeatureQueue.md`가 소유한다.

```text
CF-FQ-030 Missile
CF-FQ-034 Vehicle Fitting
CF-FQ-035 Inventory Foundation
```

사용자가 작업 전환을 선택하면 FeatureQueue와 해당 대표 Plan을 fresh read한 뒤 Active로 승격한다.

---

## 5. 완료 작업

완료 기능의 상세 검증 history는 ActiveWork에 유지하지 않는다.

현재 구현:

```text
Document/Systems/SystemIndex.md
```

완료 당시 Plan/Acceptance history:

```text
Document/Plan/Archive/README.md
```

대표적으로 `CF-FQ-031 Ammo`, `CF-FQ-033 Vehicle Defense`, `CF-FQ-027/028 Projectile`, `CF-FQ-024/025/022/023/018/017`은 Systems 승격 후 Historical route로 관리한다.

---

## 6. 세션 복원 규칙

사용자가 `이전 작업 이어서 진행해`라고 요청하면:

```text
1. 이 문서에서 현재 Active 선택
2. FeatureQueue 상태 확인
3. 대표 Plan 최신 checkpoint 확인
4. 관련 Systems / 실제 코드·에셋 교차검증
5. 대표 Plan의 next gate에서 계속
```

특정 Work ID를 지정하면 해당 작업을 우선한다.
대표 Plan과 이 projection이 충돌하면 대표 Plan과 FeatureQueue를 기준으로 이 문서를 stale projection으로 교정한다.

---

## 7. Changelog / Migration

### v2.48 - 2026-08-13

- UI-P0-03 정적 구조 감사를 수행해 Provider Rebind·Old Pawn/Timer 해제와 Presenter 공통 Launcher lifecycle에 현재 Blocking 구조 결함이 없음을 확인했다. Source·Unreal Asset은 변경하지 않았다.
- UI-P0-04 AimReticle 통합은 `InGameUIPlan.md v0.58.1`에 사전 설계만 준비했다. 기존 HUD Layer 재사용, UISubsystem 단일 수명, Pawn direct Viewport 생성과 병행 금지, Weak Pawn Binding, 기존 `WBP_AimReticle` Visual 재사용이 핵심 경계이며 실제 UI-P0-04는 아직 Not Started다.
- next gate는 변경 없이 `TestMap_DRSalvo` USER PIE → Launcher USER PASS 후 Defense 실제 변화 → Pawn Rebind다. 코드 변경이 없어 기존 Build·64/64 Automation을 재실행하지 않는다.

### v2.47 - 2026-08-13

- Salvo 전용 v1.5.0·v1.6.0 시간 Hold를 폐기하고 `LauncherSequenceRevision` 기반 공통 Weapon/Launcher Presentation lifecycle 교정을 실제 코드에 적용했다.
- HeavyCannon SingleCycle은 공통 Weapon Status, Ripple·Salvo는 동일한 Active→terminal Snapshot→Cooldown→READY 전이를 사용하며 FirePattern을 수명 정책 분기에서 제거했다.
- 공식 Editor Build `6978e029bfaa48c7addfc9acd87484b1` PASS, 전체 Automation `00ee23a01aa7480cabc557f312780ae8` 64/64 PASS·필수 32/32 Success·실패 0을 기록했다.
- `TestMap_DRSalvo` USER PIE는 Ready/Pending이며 USER 확인 전 UI-P0-03/CF-FQ-032 완료 처리를 금지한다.

### v2.46 - 2026-08-13

- 1,700줄 이상 누적된 완료 작업·Build·Automation·USER PIE·historical override 복제를 제거하고 ActiveWork를 세션 복원 projection으로 축약했다.
- `CF-FQ-032 / InGameUIPlan v0.57.0`을 단일 Active로 동기화하고 Salvo 전용 Hold 폐기 후 공통 Weapon/Launcher Presentation lifecycle 교정을 next gate로 고정했다.
- Paused `CF-FQ-029`, `CF-FQ-026`만 체크포인트로 유지하고 Ready 기능은 FeatureQueue가 소유하도록 중복을 제거했다.
- 완료 기능은 Systems와 `Document/Plan/Archive/README.md` Historical route로 내렸다.

Migration: 상세 작업 evidence는 삭제된 것이 아니라 각 대표 Plan과 Systems에 유지된다. 현재 세션 복원은 이 문서에서 work_id·대표 Plan·next gate만 선택한 뒤 representative owner를 읽는다.