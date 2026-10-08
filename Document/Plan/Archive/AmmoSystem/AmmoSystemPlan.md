# Vehicle Ammo System Implementation Plan

- Version: 0.7.0
- Date: 2026-08-13
- Status: Completed / AMMO-P0-00~08 Done / Heavy·Ripple USER PIE PASS / `Document/Systems/Combat/Ammo.md v1.0.0` Current System Promoted
- Feature: `CF-FQ-031` 차량 탄약·재장전 런타임
- Active Dependency: `CF-FQ-029` 모듈형 런처 및 발사 인계
- Related Ready Feature: `CF-FQ-030` 물리 제한형 미사일 비행·유도
- Representative Plan: `Document/Plan/AmmoSystemPlan.md`
- Design: `Document/Plan/AmmoSystemDesign.md`
- Roadmap: `Document/Plan/AmmoSystemRoadmap.md`
- First TaskSource: `Document/Plan/AmmoTaskSource.md`
- First WorkOrder: `Document/Plan/AmmoWorkOrder.md`

---

## 1. 목적

현재 출격 차량의 실제 탄약을 무기별 장전량과 탄종별 차량 예비량으로 관리하고, 단발·HitScan·Projectile·SingleCycle·Ripple·Salvo·재장전·무기 교환·HUD·피팅 중량이 하나의 탄약 Runtime 계약을 사용하도록 구현한다.

이 Plan은 `CF-FQ-031` 완료 당시 구현·검증 체크포인트를 보존한다. `AMMO-P0-00~08`, 공식 UE 5.8 Build, 전체 Automation, Heavy·Ripple finite-ammo USER PIE가 모두 PASS했고 `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System으로 승격했다.
현재 탄약 구현 판단은 실제 코드·에셋과 `Document/Systems/Combat/Ammo.md`를 우선한다. 이 Plan은 다시 Active 구현 문서로 복원하지 않으며 후속 탄약 확장은 별도 기능 범위에서 시작한다.

---

## 2. 현재 체크포인트

| 항목 | 상태 |
|---|---|
| 사용자 요구사항 | 확정 |
| 현재 코드·문서 조사 | Done |
| 설계 문서 | Ready |
| 로드맵 | Ready |
| 첫 TaskSource | Ready |
| 첫 WorkOrder | Ready |
| 기능 상태 | `Done / AMMO-P0-00~08 완료 / Systems Current` |
| 현재 Active | `CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크로 복귀` |
| 보호 체크포인트 | `CF-FQ-029 Launcher 계약 유지 / CF-FQ-032 UI-P0-03 부분 USER PASS 보존` |
| 현재 Task | `없음 — CF-FQ-031 완료 / 후속은 CF-FQ-032 UI-P0-03` |
| Source Changes | `Applied — Loaded/Capacity contract preserved / fixed 180px Reserve spacer removed / Weapon title Fill + right-edge Reserve slot / Header pre-root lookup fixed / zero-value Presenter contract retained` |
| Production Test Assets | `Applied — /Game/CarFight/Tests/AmmoIntegration + TestMap_AmmoHeavy + TestMap_AmmoRipple` |
| Asset Apply | `PASS — Dry Run 13463998d9de49c98f8f3c4b05d67eae / Apply 4fd0ab34d783484fb1d2c3cfc7b33e03 / protected SHA unchanged` |
| Production HUD | `PASS — final Apply c265e55a39ee48f2983e04c2230d264e / Readback 3f0c9dbf9bb04c8aa41b4229f1332107 / semantic_child_widget_structure_pass=true / d1_11_pass=true / errors 0` |
| Build | `PASS — final 2230e755e05844d1b1f7c0729e226d3a / CFUIHUDProdEditorBridge.cpp compile + lib·DLL link + Metadata / Exit 0` |
| Automation | `PASS — 4c68c30619554fd8985a83bfe06c5e04 / Required 32/32 / Total 64 / Failed 0 / AMMO_P0_06.HUD·AMMO_P0_08.AssetChain Success` |
| PIE | `PASS — Heavy new-display 전체 + Ripple 시작 3/4·Reserve4 / 3-shot Partial Ripple / Auto Reload 4/4·Reserve0 / 두 번째 4-shot Ripple / 최종 0/4·Reserve0·NO AMMO` |

### 2.1 현재 구현 증거

```text
AMMO-P0-01
- UCFAmmoData / Ammo 공용 enum·Runtime·Snapshot 계약 추가
- WeaponData DefaultAmmoData·InitialLoadedAmmoCount·AmmoUnitsPerShot·Reload 정책 추가
- 기존 WeaponData는 bUseInfiniteAmmoForDebug=true 기본값으로 기존 무한탄 발사 호환 유지

AMMO-P0-02
- UCFVehicleAmmoComp 기본 서브오브젝트 연결
- WeaponInstanceId별 Loaded 독립 / AmmoId별 Reserve 공유
- 반복 초기화 중복 생성 방지 / Snapshot 계산 / EndPlay·재초기화 Reset

AMMO-P0-03
- SingleCycle Validate → Reserve → Execute → Commit 또는 Rollback
- HitScan Trace Miss도 실제 발사로 1회 소비
- Projectile 실행 실패는 소비 없이 Rollback
- Direct Projectile fallback HitScan 성공은 소비
- 빈 탄창은 NotEnoughLoadedAmmo / Fire NoAmmo 경계

AMMO-P0-04
- Ripple·Salvo 첫 발 실행 전에 전체 유효 발수를 현재 Loaded에서 예약
- 6발 요청·4발 장전·부분 Sequence 허용 시 4발로 축소
- 성공한 각 내부 발사만 Loaded·Reserved에서 Commit
- 실패한 내부 발사는 Loaded 소비 없이 해당 예약만 Release
- Completed·Cancelled·시작 실패에서 미실행 예약 0과 LauncherSequenceActive Action Lock 해제
- Active Sequence 중 추가 행동 거부가 기존 Sequence를 취소하지 않음

AMMO-P0-05
- P0 실제 Reload는 FullMagazine만 구현 / PerRound는 데이터 계약 유지
- Reload 시작 시 PendingReloadAmount와 Reloading Action Lock만 설정하고 수량 이동 0
- 일반 Component Tick·bTickEvenWhenPaused=false로 Pause-safe 진행
- 완료 순간 실제 공유 Reserve를 다시 확인해 필요한 수량만 Reserve→Loaded 이동
- 예비 부족 부분 Reload / 수동 Cancel / SingleCycle Empty 자동 Reload / Launcher Terminal 후 자동 Reload
- 차량 파괴 중 Reload는 수량 이동 없이 취소·Disabled

AMMO-P0-06
- 실제 finite `FCFAmmoRuntimeSnapshot`을 `Gameplay Runtime → UCFHUDDataProvider → FCFInGameUIViewData → UCFHUDPresenter → WeaponPanel`로 전달
- WeaponPanel 기본 Ammo는 `LoadedAmmoCount / MagazineCapacity`로 표시한다.
- `ReserveAmmoCount`는 WeaponPanel 우상단에 라벨 없이 작은 숫자 하나로 별도 표시하며 KnownZero도 `0`으로 유지한다.
- `ImmediateUsableAmmoCount`와 `CurrentUsableAmmoCount`는 Runtime·ViewData에서 삭제하지 않고 발사 가능/예약/회귀 판정용 내부 값으로 유지하며 기본 Ammo 숫자에는 사용하지 않는다.
- Reload > NoAmmo > Cooldown/Ready 상태 우선순위와 Launcher Sequence 전용 Row 우선 유지
- InfiniteCompatibility·미초기화 Ammo Runtime은 가짜 수치 없이 Unavailable 유지
- Ammo event도 `OnCurrentPawnChanged` Rebind에서 Old 제거 / New 연결

AMMO-P0-07
- `VehicleFittingData.InitialSortieAmmoLoads`를 실제 출격 탄약 수량 원본으로 추가
- Snapshot에 같은 목록을 결정론적으로 보존하고 `수량 × UnitMassKg`를 `AmmoMassKg`에 반영
- `MaximumLoadableAmmoCount`는 상한 검증 전용이며 현재 수량으로 자동 대입하지 않음
- finite 무기 AmmoData·탄창 설정, AmmoId 중복, 상한 초과, 초기 장전 필요량 부족을 Snapshot 단계에서 거부
- Applied Fitting Snapshot의 finite ResolvedMounts와 InitialSortieAmmoLoads로 `VehicleAmmoComp` 초기화
- FittingMass 테스트 Fixture의 map 전환 GC Access Violation은 Product Runtime 문제가 아닌 테스트 수명 문제로 확인했고 `CFAmmoFittingTests.cpp v1.0.1`에서 `CreateNewMap()`을 Transient 객체 생성보다 앞으로 이동해 교정

Official Build
- Job 3fa31c19a8c149cb940db32a9d31dcbd
- `CFAmmoFittingTests.cpp` Compile / lib / DLL / Metadata / Exit 0

Automation
- Process c11fe9edfdf24487bad01c620c7533de
- Required 31/31 Success
- Total 63 / Failed 0
- `AMMO_P0_01.Contract`~`AMMO_P0_07.FittingMass` 전부 Success
```

### 2.2 완료 후 재개 규칙

```text
- AMMO-P0-08과 CF-FQ-031은 완료됐으므로 신규 세션에서 구현·USER PIE를 반복하지 않는다.
- VehicleFittingData.InitialSortieAmmoLoads가 실제 출격 수량의 단일 원본이다.
- MaximumLoadableAmmoCount는 계속 상한 전용이며 HUD나 Runtime 현재 수량으로 사용하지 않는다.
- 현재 구현 판단은 실제 코드·에셋과 Document/Systems/Combat/Ammo.md v1.0.0을 우선한다.
- 후속 Ammo 확장이 필요하면 기존 완료 Plan을 다시 Active로 바꾸지 말고 별도 기능 범위를 정의한다.
- 현재 작업은 CF-FQ-032 UI-P0-03의 남은 Launcher Presentation 전체 회귀·Defense 실제 변화·Pawn Rebind다.
```

`CF-FQ-031`은 완료 이력으로 유지하고 현재 단일 Active는 `CF-FQ-032`다.

---

## 3. 확정된 구현 원칙

```text
1. 인게임 기본 WeaponPanel 탄약 표시는 현재 무기의 실제 `LoadedAmmoCount / MagazineCapacity`다.
2. 같은 탄종의 차량 `ReserveAmmoCount`는 WeaponPanel 우상단에 라벨 없는 작은 숫자로 분리 표시하며, 실제 값이 0인 KnownZero 상태도 숨기지 않고 `0`을 표시한다. 피팅 `MaximumLoadableAmmoCount`는 인게임 HUD에 표시하지 않는다.
3. 차량 예비 탄약은 탄종별로 관리한다.
4. 장전 탄약은 WeaponInstanceId별로 관리한다.
5. 탄약 수량과 재장전 상태는 UCFVehicleAmmoComp가 단일 소유한다.
6. 발사 성공 단위로 탄약을 Commit하고 실행 실패는 Rollback한다.
7. 런처 시퀀스는 시작 전에 전체 유효 발수를 예약한다.
8. 런처 시퀀스 Active 중 재장전·무기·탄종·장비 교환을 거부한다.
9. 교환 또는 재장전 거부가 현재 시퀀스를 취소하지 않는다.
10. 재장전은 예비 탄약을 장전부로 이동하며 차량 전체 탄약량을 바꾸지 않는다.
11. C++은 수량·상태·검증을, Blueprint는 표시·자산 설정을 담당한다.
12. 기존 MagazineSize, ReloadTimeSeconds와 AmmoTypeId는 호환 유지한다.
```

---

## 4. 현재 연결 대상

### 현재 소스

```text
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Private/CFWeaponData.cpp
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/CFLauncherTypes.h
UE/Source/CarFight_Re/Public/CFLauncherComp.h
UE/Source/CarFight_Re/Private/CFLauncherComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

### 관련 Current Systems

```text
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/Projectile.md
Document/Systems/Combat/FireFeedback.md
Document/Systems/UI/AimReticle.md
Document/Systems/UI/VehicleDebugPanel.md
Document/Systems/Vehicles/VehicleRuntime.md
Document/Systems/Vehicles/VehicleData.md
```

### 장기 방향

```text
Document/ProjectSSOT/CombatPlan/01_Identity.md
Document/ProjectSSOT/CombatPlan/10_BattFuel.md
```

---

## 5. 권장 신규 소스

```text
UE/Source/CarFight_Re/Public/CFAmmoTypes.h
UE/Source/CarFight_Re/Public/CFAmmoData.h
UE/Source/CarFight_Re/Private/CFAmmoData.cpp
UE/Source/CarFight_Re/Public/CFVehicleAmmoComp.h
UE/Source/CarFight_Re/Private/CFVehicleAmmoComp.cpp
UE/Source/CarFight_Re/Private/CFAmmoContractTests.cpp
UE/Source/CarFight_Re/Private/CFAmmoRuntimeTests.cpp
```

모든 이름은 32자 이하이다.

---

## 6. 작업 패키지

| Task ID | 작업명 | 주 책임 | 선행 | 상태 |
|---|---|---|---|---|
| `AMMO-P0-00` | 요구사항·구조 조사와 문서 준비 | 문서·분석 | 없음 | Done |
| `AMMO-P0-01` | Ammo Data Contract Foundation | C++ Data | P0-00 | Done — Build·Automation PASS |
| `AMMO-P0-02` | Vehicle Ammo Runtime | C++ Component | P0-01 | Done — Build·Automation PASS |
| `AMMO-P0-03` | Single Fire Ammo Transaction | C++ Fire | P0-02 | Done — Build·Automation PASS |
| `AMMO-P0-04` | Launcher Reservation and Action Lock | C++ Launcher | P0-02~03, CF-FQ-029 | Done — Build·Automation PASS |
| `AMMO-P0-05` | Reload Runtime | C++/BP | P0-02~04 | Done — Build·Automation PASS |
| `AMMO-P0-06` | Ammo HUD and Debug | C++/UI/BP | P0-02~05 | Done — Build·Automation PASS |
| `AMMO-P0-07` | Fitting and Ammo Mass | Data/Vehicle | P0-01~02 | Done — Build·Automation PASS |
| `AMMO-P0-08` | Integrated Verification and Tuning | QA/Data | P0-03~07 | In Progress — Asset·Build·Automation PASS / USER PIE Ready |

---

## 7. AMMO-P0-00 — Done

### 완료 범위

```text
- 기존 WeaponData 탄약 후보 필드 확인
- NoAmmo와 Reloading 상태 후보 확인
- VehiclePawn 발사 검증·실행·결과 반영 경계 확인
- LauncherComp Ripple·Salvo 상태·실패·취소 계약 확인
- CombatPlan의 탄약·중량 원칙 확인
- 인게임 표시와 피팅 표시 용어 분리
- 런처 시퀀스 행동 잠금 결정
- 설계·로드맵·TaskSource·WorkOrder 생성
```

### 미수행

```text
- C++ 수정
- .uasset 수정
- 빌드
- Automation
- PIE
```

---

## 8. AMMO-P0-01 — Ammo Data Contract Foundation

### 목표

기존 발사 Runtime을 변경하지 않고 AmmoData, 탄약 공용 enum·구조체와 WeaponData의 정적 탄약 설정 계약을 추가한다.

### 생성

```text
CFAmmoTypes.h
CFAmmoData.h
CFAmmoData.cpp
CFAmmoContractTests.cpp
```

### 수정 후보

```text
CFWeaponData.h
CFWeaponData.cpp
```

### 핵심 범위

```text
- UCFAmmoData
- ECFWeaponReloadMode
- ECFWeaponReloadState
- ECFWeaponActionLockReason
- 기본 Runtime·Snapshot 구조체
- WeaponData.DefaultAmmoData
- InitialLoadedAmmoCount
- AmmoUnitsPerShot
- 자동·부분 재장전·부분 시퀀스 정책
- 기존 AmmoTypeId·MagazineSize·ReloadTimeSeconds 호환
- 유효값 Getter와 Debug Summary
```

### 종료 조건

```text
- UHT와 Editor 빌드 성공
- 기존 WeaponData 에셋 로드 호환
- 기본값이 기존 무한탄 발사 결과를 변경하지 않음
- Contract Automation PASS 또는 Runner 미노출 상태 명시
```

---

## 9. AMMO-P0-02 — Vehicle Ammo Runtime

### 목표

차량 단위 탄약 컴포넌트가 탄종별 예비량과 WeaponInstanceId별 장전량을 단일 소유하고 Snapshot을 제공한다.

### 핵심 작업

```text
- UCFVehicleAmmoComp 생성
- 차량 Pawn 기본 서브오브젝트 연결
- 초기 출격 탄약 구성 입력
- WeaponInstanceId 해석
- 장전·예비·예약 수량 계산
- CurrentUsableAmmoCount와 CurrentOnboardAmmoCount
- 출격 통계
- 상태 변경 이벤트
- 초기화·Reset·EndPlay 안전 처리
```

### 종료 조건

```text
- 동일 AmmoData 공유 무기 두 개의 장전량 독립
- 차량 예비량 공유
- 음수와 초과 장전 방지
- 초기화 반복 시 중복 탄약 생성 없음
- Snapshot 계산 Automation PASS
```

---

## 10. AMMO-P0-03 — Single Fire Ammo Transaction

### 목표

일반 단발, HitScan과 Projectile의 각 정상 실행에서 탄약을 정확히 한 번 소비하고 실패 시 반환한다.

### 연결

```text
ValidateFireCommandInternal
→ 행동 잠금·재장전·장전 탄약 검증

ExecuteAcceptedFireCommand
→ Reserve / Execute / Commit 또는 Rollback

ApplyFireResultInternal
→ 표시·이벤트 반영
```

### 종료 조건

```text
- 정상 단발 1회 소비 — Automation PASS
- Trace Miss 소비 — Automation PASS
- 쿨다운·검증 단계 거부는 Reserve 전 차단 — Source 계약 적용
- Projectile 실행 실패 Rollback — Automation PASS
- Direct fallback 성공 소비 — Automation PASS
- 빈 탄창 NoAmmo / Reloading·ActionLock 구분용 RejectReason 계약 추가
- Official Build 0129a0ea40be4ee18ee4bcb73128764d PASS
- Combat Automation 3a29d03cfe05462ea772123f597304ff / 27/27 Required / Total 59 Failed 0
```

`AMMO-P0-03`은 Source·공식 Build·Automation 기준 Done이다. 실제 finite-ammo 에셋 연결과 사용자 PIE는 AMMO-P0-04~08 통합 범위에서 수행한다.

---

## 11. AMMO-P0-04 — Launcher Reservation and Action Lock

### 목표

SingleCycle·Ripple·Salvo가 시작 전에 유효 발수를 예약하고 각 성공 발사만 소비하며, Active 시퀀스 동안 재장전과 교환 행동을 잠근다.

### 핵심 작업

```text
- Ammo 제한 FirePattern 계산
- 부분 시퀀스 기본 허용
- 첫 발 포함 전체 수량 예약
- Dispatch 성공 Commit
- Shot 실패 예약 해제
- StopSequence 미실행 예약 전체 해제
- Completed·Cancelled 최종 예약 0 보장
- LauncherSequenceActive 행동 잠금
- 재장전·무기·탄종·장비 교환 거부
- 거부 요청으로 시퀀스 취소 금지
```

### 종료 조건

```text
- 6발 요청·4발 장전은 4발 시퀀스 — Automation PASS
- 각 성공 내부 발사마다 1회 소비 — Automation PASS
- 실패한 발은 소비하지 않고 해당 예약 반환 — Automation PASS
- 시퀀스 중 재장전·추가 Sequence 예약 거부 — Automation PASS
- 거부 뒤 기존 시퀀스 계속 진행 — Automation PASS
- Manual Cancel terminal에서 미발사 예약 0·Action Lock 해제 — Automation PASS
- 기존 Launcher Scheduler·Cooldown·무한탄 회귀 유지 — 전체 회귀 PASS
```

`AMMO-P0-04`는 Source·공식 Build·Automation 기준 Done이다. 차량 파괴 실제 PIE는 AMMO-P0-08 통합 사용자 검증에서 확인한다.

---

## 12. AMMO-P0-05 — Reload Runtime

### 목표

시퀀스가 없는 상태에서 수동·자동 FullMagazine 재장전을 수행한다.

### 핵심 작업

```text
- ReloadState
- PendingReloadAmount
- ReloadTimeSeconds
- 수동 재장전 요청
- Empty 자동 재장전
- 시퀀스 종료 후 자동 재장전
- 무기·차량 무효화 취소
- 재장전 중 발사와 교환 잠금
```

### 종료 조건

```text
- 재장전 완료 전 수량 이동 없음 — Automation PASS
- 완료 시 필요한 수량만 이동 — Automation PASS
- 예비 부족 부분 재장전 — Automation PASS
- 차량 전체 탄약량 보존 — Automation PASS
- Active Launcher Sequence 중 시작 불가·Sequence 유지 — Automation PASS
- 재장전 중 발사 Reloading 거부 — Automation PASS
- 수동 Cancel은 수량 이동 없이 Lock 해제 — Automation PASS
- SingleCycle Empty 자동 Reload — Automation PASS
- Launcher Terminal 후 자동 Reload — Automation PASS
- Component Tick bTickEvenWhenPaused=false — Automation PASS
- PerRound는 P0 실제 실행 미지원 ExecutionFailed 계약 — Automation PASS
- Official Build 53f02701b7ed421388d4a44a78714a77 PASS
- Combat Automation 3eb58429e9534593ab4ad2dd75a7169c / 29/29 Required / Total 61 Failed 0
```

`AMMO-P0-05`는 Source·공식 Build·Automation 기준 Done이다. 실제 입력 바인딩과 finite-ammo 사용자 PIE는 P0-06~08 통합 범위에서 진행한다.

---

## 13. AMMO-P0-06 — Ammo HUD and Debug

### 목표

현재 선택 무기의 실제 장전량·탄창 용량과 차량 예비량을 서로 분리해 플레이어에게 표시하고, 즉시 사용 가능량·현재 사용 가능 총량·재장전·런처 시퀀스 상태는 내부 판정과 Debug에서 계속 읽을 수 있게 한다.

### 기본 표시

```text
일반 finite Ammo 상태
Primary: 5 / 10
우상단 Reserve: 10  (라벨 없음)

내부 판정값
ImmediateUsableAmmoCount: 표시하지 않음
CurrentUsableAmmoCount: 표시하지 않음

런처 시퀀스
RIPPLE 2 / 6
남은 발사 4
예약 중에도 Commit 전 Loaded / MagazineCapacity 표기는 실제 장전량을 유지
```

### UI 책임

```text
C++
- Snapshot과 상태 변경 이벤트

Blueprint
- 배치·스타일·애니메이션
- LowAmmo·NoAmmo·Reloading 표시
- 시퀀스 진행 표시
```

### 종료 조건

```text
- 피팅 최대 적재량이 인게임 기본 HUD에 섞이지 않음
- 탄종별 전체 목록은 상세 자원창으로 분리
- Reloading·NoAmmo와 SequenceActive 구분
- 16:9와 32:9 가독성
```

---

## 14. AMMO-P0-07 — Fitting and Ammo Mass

### 목표

AmmoData의 탄당 중량과 피팅 적재량을 차량 총중량 계산에 연결한다.

### 핵심 작업

```text
- MaximumLoadableAmmoCount
- InitialSortieAmmoCount
- UnitMassKg
- TotalAmmoMassKg
- 피팅 현재 적재 / 최대 적재 표시
- VehicleData·VehicleRuntime 총중량 적용 경계 조사
- 소구경 배치 질량 갱신 정책
```

### 종료 조건

```text
- 장전·예비 중복 계산 없음
- 출격 초기 총중량 반영
- 피팅 한도와 현재 인게임 탄약 분리
- 기존 차량 질량 설정 회귀 없음
```

---

## 15. AMMO-P0-08 — Integrated Verification and Tuning

### Production finite-ammo 준비 — PASS

```text
격리 에셋 루트
- /Game/CarFight/Tests/AmmoIntegration

Heavy 체인
- DA_Ammo_HeavyFinite
- DA_Mount_HeavyFinite
- DA_Wpn_HeavyFinite
- Preset_HeavyFinite
- DA_Veh_HeavyFinite
- DA_Fit_HeavyFinite
- TestMap_AmmoHeavy
- MagazineCapacity 10 / 시작 Loaded 5 / Reserve 10 / CurrentUsable 15
- 새 WeaponPanel 계약: 시작 Primary `5 / 10` + 우상단 Reserve `10`
- 한 발 Commit 후 Primary `4 / 10` + 우상단 Reserve `10`
- 기존 `5 | 15`→`4 | 14` USER PIE는 이전 표시 계약의 역사 증거이며 새 표시 회귀 기준으로 사용하지 않는다.
- AmmoMassKg 30 / TotalVehicleMassKg 1500

Ripple 체인
- DA_Ammo_RocketFinite
- DA_Mount_RippleFinite
- DA_Wpn_RippleFinite
- Preset_RippleFinite
- DA_Veh_RippleFinite
- DA_Fit_RippleFinite
- TestMap_AmmoRipple
- MagazineCapacity 4 / 시작 Loaded 3 / Reserve 4 / CurrentUsable 7
- 새 WeaponPanel 계약: 시작 Primary `3 / 4` + 우상단 Reserve `4`
- 4발 요청에서 bAllowPartialSequence=true로 실제 3발 예약
- 예약만 된 동안 Loaded는 아직 소비되지 않으므로 Primary `3 / 4` + 우상단 Reserve `4`를 유지한다.
- 예약 중 `ImmediateUsableAmmoCount=0 / CurrentUsableAmmoCount=4`는 내부 판정값으로 유지하지만 기본 Ammo 표시에 사용하지 않는다.
- AmmoMassKg 35 / TotalVehicleMassKg 1505

공통
- ReloadTimeSeconds 2.0 / FullMagazine / bAutoReloadWhenEmpty=true
- MaximumLoadableAmmoCount는 현재 탄약이 아니라 피팅 상한으로만 사용
- 공유 TestMap·Vehicle·Weapon·Preset·Mount·BP_CFVehiclePawn·WBP_CFInGameHUD SHA-256 전후 일치
- 사용자 에셋 직접 finite-ammo 전환 없음
- 수동 Reload 입력 바인딩 추가 없음; 기존 Gameplay API RequestReloadCurrentWeapon은 유지
```

### 자동화 — PASS

```text
CarFight.Ammo.AMMO_P0_01.Contract
CarFight.Ammo.AMMO_P0_02.Runtime
CarFight.Ammo.AMMO_P0_03.FireTransaction
CarFight.Ammo.AMMO_P0_04.LauncherLock
CarFight.Ammo.AMMO_P0_05.Reload
CarFight.Ammo.AMMO_P0_06.HUD
CarFight.Ammo.AMMO_P0_07.FittingMass
CarFight.Ammo.AMMO_P0_08.AssetChain

Asset Apply
- Dry Run 13463998d9de49c98f8f3c4b05d67eae PASS
- Apply 4fd0ab34d783484fb1d2c3cfc7b33e03 PASS

Ammo Display Contract Revision
- HUD ViewData `MagazineCapacity` 전달 PASS
- Primary `LoadedAmmoCount / MagazineCapacity` Presenter 회귀 PASS
- Header `Text_WeaponReserveAmmo` label-less 별도 슬롯 Production 연결 PASS
- `ImmediateUsableAmmoCount` / `CurrentUsableAmmoCount` 내부값 유지 회귀 PASS
- Ripple 예약 중 Primary `3 / 4` 유지 회귀 PASS

Production HUD
- 최초 Reserve 가시성 교정 Apply 309ee3b0da85471bb858672e487ea2ab: WeaponPanel 생성 전 Header Title을 Root WidgetTree에서 조회해 `Production role tree construction failed`로 중단
- `CFUIHUDProdEditorBridge v1.5.1`에서 Header의 세 번째 직접 자식 Title을 사용하도록 생성 순서를 교정
- 최종 Apply c265e55a39ee48f2983e04c2230d264e PASS / success=true / Editor Exit 0
- 최종 Readback 3f0c9dbf9bb04c8aa41b4229f1332107 PASS
- `/Game/CarFight/UI/HUD/Panels/WBP_CFWeaponPanel` 포함 exact mutable assets 10
- semantic_child_widget_structure_pass=true
- d1_11_structure_pass=true / d1_11_pass=true / errors 0

Official Build
- 최초 표시 계약 시도 93b3dba43ce0445ab57b045251db2e07: WeaponPanel Header 신규 enum/slot API 컴파일 오류로 FAIL, 저장 에셋 Apply 전 중단
- 표시 계약 최소 Build Repair 021e98cf561940829364d80d1678b22a PASS / Exit 0
- Reserve 우측 가시성 첫 빌드 cdc031774c90499c85361ab845edf3e4: C++ Compile PASS, 실행 중 UnrealEditor.exe DLL 점유로 LNK1104
- 에디터 종료 후 b111a751bcdf4c68b110e457a7cb0933 PASS
- Header pre-root 조회 교정 후 최종 2230e755e05844d1b1f7c0729e226d3a PASS / Compile·lib·DLL·Metadata / Exit 0

Full Regression
- 4c68c30619554fd8985a83bfe06c5e04
- Required 32/32 Success
- Total 64 / Failed 0
- `CarFight.Ammo.AMMO_P0_06.HUD` Success
- `CarFight.Ammo.AMMO_P0_08.AssetChain` Success
```

### 사용자 PIE — Heavy·Ripple PASS

```text
Heavy — USER PASS
- TestMap_AmmoHeavy 전체 gameplay 항목 PASS.
- Loaded / MagazineCapacity 표시, 실제 발사 소비, 2초 FullMagazine Auto Reload PASS.
- 완전 소진 Primary `0 / 10` + 우상단 label-less Reserve `0` 가시성 PASS.

Ripple — USER PASS / 2026-08-13 15:15 KST
- 시작 Primary `3 / 4` + Reserve `4` PASS.
- 첫 4발 요청이 실제 3발 Partial Ripple로 제한되고 `RIPPLE N / 3` 표시 PASS.
- 실제 Commit마다 Loaded `3→2→1→0`, Reload 전 Reserve `4` 유지 PASS.
- Sequence terminal 뒤 2초 Auto Reload → Primary `4 / 4`, Reserve `0` PASS.
- 두 번째 Trigger 실제 4발 Ripple PASS.
- 최종 Primary `0 / 4`, Reserve `0`, `NO AMMO` PASS.
- 정상 Ripple 진행이 상단 AlertFeed가 아닌 WeaponPanel에 표시되는 계약 PASS.
```

### 종료 조건

```text
- 공식 Editor 빌드 PASS — 완료
- 관련 Automation PASS — 완료
- Production finite-ammo 저장 체인과 사용자 PIE 맵 준비 — 완료
- 기존 Direct·Ripple·Salvo·Damage·FX·Pool 회귀 없음 — 자동화 기준 완료
- 사용자 PIE PASS — 완료 / Heavy·Ripple 전체 PASS
- Systems 문서 승격 — 완료 / `Document/Systems/Combat/Ammo.md v1.0.0`
```

`AMMO-P0-08 = Done`, `CF-FQ-031 = Done`이다. Ammo 현재 구현은 `Document/Systems/Combat/Ammo.md v1.0.0`이 소유하며 현재 단일 Active는 `CF-FQ-032`로 복귀한다.

---

## 16. 보호 범위

```text
- CF-FQ-029의 기존 Launcher 시퀀스 상태·쿨다운 정책을 임의 재작성하지 않는다.
- Projectile Actor와 Pool의 첫 Impact·Deactivate 순서를 변경하지 않는다.
- DamageData 소유 경로를 변경하지 않는다.
- TargetSelect 소유권과 발사 방향을 변경하지 않는다.
- 기존 WeaponData 필드를 삭제하거나 무분별하게 리네이밍하지 않는다.
- 사용자 미커밋 .uasset을 임의 재저장하지 않는다.
- 게임 Audio를 추가하지 않는다.
- commit, push, reset, checkout과 stash를 수행하지 않는다.
```

---

## 17. Current System 승격 결과

```text
승격 완료
- Document/Systems/Combat/Ammo.md v1.0.0

관련 기존 Systems
- WeaponFire·Projectile·VehicleRuntime 등은 각 문서가 다음에 실제로 갱신될 때 Ammo Current System 링크를 추가할 수 있다.
- Launcher 전체 완료 여부는 CF-FQ-029가 별도로 소유하므로 Ammo 완료와 함께 Launcher를 자동 승격하지 않는다.
```

현재 구현 판단은 `Document/Systems/Combat/Ammo.md`와 실제 코드를 우선하며 이 Plan은 완료 이력으로 보존한다.

---

## 18. 다음 작업

```text
CF-FQ-031 / AMMO-P0-08 = Done
→ Ammo Current System = Document/Systems/Combat/Ammo.md v1.0.0
→ 현재 단일 Active를 CF-FQ-032 UI-P0-03으로 복귀
→ Ammo 자체 USER PIE는 반복하지 않는다.
→ UI-P0-03의 남은 Production Runtime 검증만 이어간다.
   - Launcher Presentation의 남은 전체 회귀(Salvo·Terminal Cooldown→READY 포함)
   - Defense 실제 변화 표시
   - Pawn Rebind
→ 위 UI 검증 전에는 CF-FQ-032/UI-P0-03을 완료 처리하지 않는다.
```

---

## 19. Changelog

### v0.7.0 - 2026-08-13

```text
- 사용자가 TestMap_AmmoRipple의 시작 `3 / 4` + Reserve `4`, 실제 3발 Partial Ripple, Commit별 Loaded 감소, terminal 2초 Auto Reload, 두 번째 4발 Ripple과 최종 `0 / 4` + Reserve `0` + `NO AMMO`를 전부 PASS로 확인했다.
- 정상 Ripple 진행이 WeaponPanel 소유이고 상단 AlertFeed에 나타나지 않는 계약도 사용자 검증했다.
- 이에 따라 `AMMO-P0-08 = Done`, `CF-FQ-031 = Done`으로 종료했다.
- `Document/Systems/Combat/Ammo.md v1.0.0`을 Current System으로 승격했다.
- 현재 단일 Active는 `CF-FQ-032`로 복귀하며 Ammo USER PIE 자체는 반복하지 않는다.
- CF-FQ-032의 Launcher 전체 UI 회귀, Defense 실제 변화와 Pawn Rebind는 별도 UI-P0-03 Gate로 남긴다.
```

### v0.6.5 - 2026-08-13

```text
- Heavy 새 Ammo 표기 USER PIE 최종 PASS 체크포인트에서 TestMap_AmmoRipple USER PIE Gate를 시작했다.
- 시작 `3 / 4` + Reserve `4`, 4발 요청→실제 3발 Partial Ripple, 실제 Commit별 Loaded 감소, terminal 후 2초 Auto Reload, 두 번째 4발 후 `NO AMMO`를 사용자 검증 항목으로 고정했다.
- 예약-only 내부 상태의 ImmediateUsable=0 / CurrentUsable=4는 Automation 증거로 유지하며 순간 화면 캡처를 USER 필수 항목으로 요구하지 않는다.
- 시퀀스 중 수동 Reload는 현재 사용자 입력 바인딩이 없으므로 USER PIE 필수 항목으로 강제하지 않고 기존 Automation의 ActionLocked 증거를 유지한다.
- 공식 UE 5.8 Editor Build 6b15b6ebd9b94262ac1a6b25d97bc381은 Target up-to-date / Exit 0 PASS다.
- Ripple USER PASS 전 AMMO-P0-08과 CF-FQ-031은 Done 처리하지 않는다.
```

### v0.6.4 - 2026-08-13

```text
- 사용자가 TestMap_AmmoHeavy의 최종 Reserve `0` 가시성 재검증을 PASS로 확인했다.
- Heavy 새 표시 계약은 Loaded/Capacity, 발사 소비, Reload 수치 분리와 label-less Reserve KnownZero `0` 표시까지 USER PASS로 닫았다.
- AMMO-P0-08과 CF-FQ-031은 Ripple USER PIE가 남아 있으므로 Done 처리하지 않는다.
- TestMap_AmmoRipple USER PIE는 아직 시작하지 않았고 다음 사용자 지시 전까지 중단 상태를 유지한다.
```

### v0.6.3 - 2026-08-13

```text
- ReserveAmmoCount KnownZero는 숨기지 않고 WeaponPanel 우상단에 숫자 `0`으로 유지하는 계약을 명시했다.
- 고정 180px Spacer를 제거하고 무기 제목 Fill + 우측 Reserve Slot으로 교정한 v1.5.0 방향을 유지했다.
- 첫 Production Apply에서 Header가 Root Tree에 붙기 전에 WidgetTree 전역 검색으로 Title을 찾지 못해 WeaponPanel 생성이 중단되는 결함을 확인했다.
- CFUIHUDProdEditorBridge v1.5.1에서 BuildHeader의 직접 자식 Title을 사용하도록 최소 교정했다.
- 최종 공식 Build 2230e755e05844d1b1f7c0729e226d3a PASS, Production Apply c265e55a39ee48f2983e04c2230d264e PASS, Readback 3f0c9dbf9bb04c8aa41b4229f1332107 PASS를 확인했다.
- 전체 Automation 4c68c30619554fd8985a83bfe06c5e04는 Required 32/32, Total 64, Failed 0이며 Ammo HUD와 AssetChain이 Success다.
- Heavy의 Loaded/Capacity·소비·Reload 사용자 PASS는 보존하고, 우상단 Reserve `0` 가시성 한 항목만 USER 재확인 Pending으로 남겼다. Ripple USER PIE는 시작하지 않았다.
```

### v0.6.2 - 2026-08-13

```text
- Heavy 새 Ammo 표기의 Loaded/Capacity, 발사 소비와 Reload 수치 분리는 사용자 확인 PASS로 기록했다.
- 탄약 완전 소진 화면에서 ReserveAmmoCount=0 값 계약은 정상인데 Text_WeaponReserveAmmo가 보이지 않는 시각 결함을 사용자 스크린샷으로 확인했다.
- 원인은 WeaponPanel Header의 고정 180px Spacer가 Reserve Text를 실제 패널 폭 밖으로 밀어 클리핑한 것으로 확인했다.
- CFUIHUDProdEditorBridge v1.5.0에서 고정 Spacer를 제거하고 무기 제목 Fill + 우측 Reserve Slot으로 교정했으며 기본 Preview도 0으로 변경했다.
- Reserve KnownZero→`0` Presenter·Automation 계약은 이미 PASS 상태라 Runtime/Presenter 계산은 변경하지 않았다.
- Repair Build cdc031774c90499c85361ab845edf3e4에서 해당 C++ Compile은 PASS했지만 실행 중 UnrealEditor.exe가 DLL을 점유해 최종 Link가 LNK1104로 중단됐다.
- 에디터 종료 후 최종 Build·Production Apply·Automation과 Heavy Reserve `0` USER 재확인이 남았으며 Ripple USER PIE는 시작하지 않는다.
```

### v0.6.1 - 2026-08-13

```text
- 새 Ammo 표시 계약의 HUD ViewData, Presenter와 Production WeaponPanel 연결을 실제 적용했다.
- 최초 Build의 WeaponPanel Header enum/slot API 컴파일 오류를 기존 BuildHeader 재사용 방식으로 최소 교정한 뒤 공식 UE 5.8 Editor Build를 PASS했다.
- Production HUD Apply·Readback에서 WBP_CFWeaponPanel 포함 semantic structure와 D1-11 구조 계약을 PASS했다.
- 전체 Automation Required 32/32, Total 64, Failed 0이며 AMMO_P0_06.HUD와 AMMO_P0_08.AssetChain이 새 표시 계약으로 Success다.
- Heavy의 기존 gameplay USER PASS는 보존하되 새 표시의 짧은 USER PIE는 Pending이며 Ripple USER PIE는 시작하지 않았다.
- AMMO-P0-08과 CF-FQ-031은 USER PASS 전 Done 처리하지 않는다.
```

### v0.6.0 - 2026-08-13

```text
- 사용자 기획 변경으로 WeaponPanel 기본 Ammo 계약을 `ImmediateUsable | CurrentUsable`에서 `LoadedAmmoCount / MagazineCapacity`로 교체했다.
- ReserveAmmoCount는 WeaponPanel 우상단에 라벨 없이 작은 숫자로 별도 표시하도록 확정했다.
- ImmediateUsableAmmoCount와 CurrentUsableAmmoCount는 삭제하지 않고 Runtime·ViewData 내부 발사 가능/예약/회귀 판정값으로 유지한다.
- Heavy USER PIE의 기존 gameplay 전체 PASS는 보존하되 이전 표시 계약 결과는 역사 증거로 분리하고, 새 표시만 짧은 Heavy 회귀를 요구한다.
- Ripple USER PIE 전에서 멈추며 USER PASS 전 AMMO-P0-08·CF-FQ-031 Done/Systems 승격 금지를 유지한다.
```

### v0.5.0 - 2026-08-13

```text
- AMMO-P0-08 격리 Production finite-ammo Heavy/Ripple DataAsset·Fitting·PIE 준비 맵을 적용했다.
- 저장 Fitting Snapshot → VehicleAmmoComp → HUD Provider → WeaponPanel 표시 계약을 AMMO_P0_08.AssetChain으로 검증했다.
- Heavy `5 | 15`→1발 소비 `4 | 14`, Ripple `3 | 7`→4발 요청 실제 3발 예약·예약 중 `0 | 4`를 자동 검증했다.
- 공식 Build와 전체 32/32 Required·64 Total·0 Failed 회귀를 PASS했다.
- 사용자 PIE는 실행하지 않았으며 USER PASS 전 P0-08과 CF-FQ-031을 Done 처리하지 않는다.
```

### v0.1.0 - 2026-07-29

```text
- CF-FQ-031 차량 탄약·재장전 런타임 대표 Plan을 생성했다.
- AMMO-P0-00~08 작업 패키지와 선행 관계를 정의했다.
- 인게임 현재 사용 가능량, 런처 예약·행동 잠금과 재장전 계약을 구현 단계로 분해했다.
- 첫 코드 작업을 기존 발사 Runtime을 변경하지 않는 Ammo Data Contract로 제한했다.
```

---

## 20. Migration

```text
- 이 문서는 현재 Active인 CF-FQ-029 Launcher Plan을 대체하지 않는다.
- CF-FQ-031 착수 전에는 기존 WeaponFire가 현재와 같이 탄약 비제한 상태로 동작한다.
- AMMO-P0-01은 신규 데이터 계약만 추가하고 실제 발사 소비는 AMMO-P0-03 전까지 연결하지 않는다.
- 기존 MagazineSize, ReloadTimeSeconds와 AmmoTypeId는 단계적 마이그레이션 동안 유지한다.
