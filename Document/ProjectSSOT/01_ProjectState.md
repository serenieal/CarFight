# CarFight — 01_ProjectState

> 역할: CarFight 프로젝트의 **프로젝트 전역 기준선 / 현재 리스크 / 전역 결정**을 고정한다.
> 공통 규칙 원본: `Document/SSOT/`
> 문서 버전: v2.45.14
> 마지막 정리(Asia/Seoul): 2026-09-17
> 문서 상태: Current / Work Lifecycle Slice UDS Authority0

---

## 1. 현재 공식 Unreal Engine 기준선

```text
Engine Version: Unreal Engine 5.8
Engine Distribution: Source Build
Engine Root: D:\UnrealEngine_Source
Editor Build Entry: D:\Work\CarFight_git\Tools\BuildEditor.bat
Editor Run Entry: D:\Work\CarFight_git\Tools\RunEditor.bat
```

별도 엔진 업그레이드 결정 전까지 CarFight의 엔진 호환성, API와 플러그인 판단은 UE 5.8 Source Build를 기준으로 한다.

과거 문서의 UE 5.7, `D:\UE_5.7`, `D:\UE_5.7_Source` 표기는 Historical이며 현재 기준으로 사용하지 않는다.

---

## 2. 현재 프로젝트 상태

### 2.1 현재 Active / Ready Work — UDS Reference

현재 Active/Ready Work lifecycle의 canonical authority는 `Document/UDS/records/**`다. 사람용 bounded restore view는 `Document/UDS/derived/Current.md`를 사용한다.

이 문서는 프로젝트 기준선·전역 결정·Current implementation/risk owner 역할을 계속 유지하지만, Active/Ready Work의 state/phase/next를 독립적으로 소유하거나 dual-write하지 않는다.

현재 Active/Ready Work 목록과 state/phase/next는 이 문서에 복제하지 않는다. 필요할 때 `Document/UDS/derived/Current.md`를 탐색 힌트로 사용하고 fresh canonical Work head를 확인하며, 상세 checkpoint/evidence는 각 representative Plan이 소유한다.

### 2.2 최근 완료

`CF-FQ-026 타겟 선택 시스템`은 current Source/Asset/System Rebaseline 결과 Done / Historical이다.

현재 구현 owner:

```text
Target Candidate/Selected Actor authority → Document/Systems/Targeting/TargetSelect.md v1.0.0
Detection/Contact/Knowledge boundary → Document/Systems/Targeting/SensorContact.md v1.2.0
```

기존 `TS-P0-00~07`과 TS-P0-08 remote technical evidence는 Historical Technical PASS로 보존한다. old `TS-P0-08 USER PIE` 전체는 USER PASS가 아니라 `Superseded / Not Executed`다.

동일 차량 TargetPoint/인식 영역은 기술 교정 evidence만 보존하고 USER 상태는 `Inconclusive`; 7°/1200m 범위 체감은 Deferred USER tuning, Debug Sphere는 Deferred observational debt다. 후보 텍스트/old 16:9·32:9 UI workflow는 successor UISubsystem + TargetPanel 구조로 역할이 바뀌었다.

과거 20Hz 전체 World 반복 후보 수집은 현재 `UCFTargetRegistrySubsystem`의 bootstrap + 증분 등록 + snapshot 공급으로 successor-resolved 됐다. 이번 closure는 문서 lifecycle 마감이며 Source/Asset/Build/Automation/PIE mutation과 새 USER PASS는 0이다. 상세 Historical evidence는 `Document/Plan/TargetSelect/TargetSelectPlan.md v0.13.0`이 보존한다.

---

## 3. 현재 재개 가능한 체크포인트

현재 Paused/Ready Work의 정확한 lifecycle과 재개 Gate는 UDS Current를 탐색 힌트로 fresh canonical Work record를 확인한다. 승격 전 Candidate의 우선순위와 착수 판단은 FeatureQueue를 우선한다.

`CF-FQ-020 조작감/전투 템포/피드백 개선`, `CF-FQ-021 핵심 게임 루프 검증`, `CF-FQ-012 1대 차량 주행감 고도화`, `CF-FQ-014 WheelSync 시각 품질 폴리싱`은 Candidate다.

`CF-FQ-019 주행/전투 반복 테스트`는 런처·미사일 이후 통합 회귀 범위를 다시 설계하기 위해 Deferred다.

상세 Feature 상태는 `03_FeatureQueue.md`, 상세 구현·검증 evidence는 각 대표 Plan을 우선한다.

---

## 4. 현재 구현 기준

현재 구현 상세는 이 문서에 복제하지 않는다. `Document/Systems/SystemIndex.md`를 진입점으로 사용한다.

현재 주요 Current Systems는 다음 영역을 포함한다.

```text
Combat
- WeaponData / WeaponFire / Launcher / Ammo / FireFeedback
- Projectile / MissileGuidance / DamageHitContext / HitDamage / VehicleDefense / CombatFx

Targeting
- TargetSelect candidate/selected authority
- SensorContact + Scanner detection/contact/knowledge integration

UI
- InGameUI / AimReticle / VehicleDebug

Vehicles
- VehicleData / VehicleRuntime / VehicleDrive / VehicleSteering
- WheelSync / VehicleCamera / VehicleAim
- VehicleBuilder / Data Authoring Backend + Advanced Workspace + Performance Tuning Protocol

Inventory/Fitting
- Inventory ownership / Container / Reservation / Atomic Transfer는 `VehicleInventory.md v1.0.0` Current 계약을 우선
- Fitting/Mass Runtime은 `VehicleRuntime.md v1.3.0` Current 계약을 우선
- RuntimeApply는 Inventory ownership을 조작하지 않는 non-owning 즉시 적용 경로
```

Feature가 Done으로 승격되지 않은 영역은 Systems 일부가 존재하더라도 해당 Feature 전체를 Done으로 추정하지 않는다.

---

## 5. 프로젝트 전역 결정

### 5.1 싱글 플레이 우선

현재 개발 일정은 싱글 플레이 기준 차량 전투 완성에 집중한다. 서버 권한 전투, Dedicated Server, 2클라이언트 검증, 세션/로비와 서버 운영 기능은 Deferred/Icebox다.

### 5.2 게임 사운드 비지원

프로젝트 결정 `CF-PDL-0009`에 따라 CarFight는 게임 사운드를 구현하거나 제공하지 않는다.

```text
- SoundWave, SoundCue, MetaSound Source, Sound Attenuation 자산을 제작·도입하지 않는다.
- USoundBase, UAudioComponent와 사운드 재생 함수를 런타임 계약에 추가하지 않는다.
- 엔진음, 타이어음, 충돌음, 무기음, 피격음, 파괴음, UI음과 BGM은 완료 조건에서 제외한다.
```

상세 결정 원문은 `04_ProjectDecisions.md`를 우선한다.

---

## 6. 현재 검증 정책

현재 검증 evidence의 소유권은 다음처럼 분리한다.

```text
Persisted Asset/DataAsset/Blueprint/package 사실 → AssetDump
Current Editor/world/PIE runtime 기술 사실 → Accepted GoPyMCP UE MCP / RuntimeRead
시각 품질·UX·조작감·주행감·조준감·연출 감각 → USER validation
상세 Build/Automation/USER evidence → 대표 Plan
현재 구현 계약 → Systems
```

AI Technical PASS와 USER PASS를 서로 대체하지 않는다.

완료된 Build/Automation/USER evidence는 새 failure evidence 없이 반복하지 않는다. 다만 관련 코드·자산·계약이 바뀌어 기존 evidence의 유효 범위를 벗어나면 영향 범위에 맞춰 다시 검증한다.

---

## 7. 현재 리스크와 보호 조건

현재 문서·작업 복원 시 다음을 보호한다.

```text
- 기존 dirty work를 임의로 정리·되돌리지 않는다.
- Done Feature의 완료 evidence를 USER 미확인 항목과 혼동해 되돌리지 않는다.
- 반대로 Deferred/Pending USER Visual·Feel 항목을 Technical PASS만으로 USER PASS 처리하지 않는다.
- Paused Feature의 완료 Gate를 재개 시 반복하지 않는다.
- Current 구현 판단은 실제 Source/Asset과 Systems를 우선한다.
- ProjectSSOT/FeatureQueue/UDS Current는 상세 evidence 저장소로 사용하지 않는다. `ActiveWork.md`는 pre-cutover retained frozen Historical snapshot으로만 보존한다.
```

---

## 8. Changelog

### v2.45.14 - 2026-09-17

- ProjectState의 역할을 프로젝트 전역 기준선·리스크·전역 결정으로 좁히고 Active/Ready exact 목록의 Current mirror를 제거했다.
- Ready/Paused 재개 판단에서 legacy `ActiveWork.md` 경로를 제거하고 UDS Current 탐색 힌트 → fresh canonical Work record를 사용하도록 교정했다.
- Candidate 우선순위·착수 판단은 FeatureQueue, 상세 checkpoint/evidence는 representative Plan이 소유하도록 owner 경계를 다시 고정했다.
- `ActiveWork.md`는 retained frozen Historical snapshot이며 ProjectState의 Current/evidence 경로가 아님을 명시했다.

### v2.45.13 - 2026-09-16

- UDS-08 MIG-05 cutover에 따라 Active/Ready Work lifecycle slice를 authority0 UDS reference로 전환했다.
- 프로젝트 엔진 기준선, 전역 결정, Current implementation/risk owner 역할은 이 문서가 계속 유지하며 Work state/phase/next는 `Document/UDS/records/**`에 dual-write하지 않는다.
- 세션 복원은 `Document/UDS/derived/Current.md`에서 시작하고 상세 checkpoint/evidence는 representative Plan을 따른다.

### v2.45.12 - 2026-09-14

- `CF-FQ-026 타겟 선택 시스템`을 current Source/Asset/System Rebaseline 결과 Paused → Done / Historical로 전환하고 현재 재개 체크포인트에서 제거했다.
- Current owner는 `TargetSelect.md v1.0.0`; Sensor/Scanner Detection·Contact·Knowledge는 `SensorContact.md v1.2.0`으로 분리한다. 과거 전체 World 반복 후보 수집은 TargetRegistry successor 구조로 해결됐다.
- old `TS-P0-08 USER PIE`는 USER PASS가 아닌 `Superseded / Not Executed`다. 동일 차량 TargetPoint는 USER Inconclusive, 7°/1200m 후보 범위와 Debug Sphere는 Deferred debt로 보존한다. Source/Asset/Build/Automation/PIE mutation과 새 USER PASS는 0이며 단일 Active `CF-FQ-039`는 유지한다.

### v2.45.11 - 2026-09-14

- `CF-FQ-035 인벤토리 Foundation`을 current Source/System Rebaseline로 Paused → Done 전환하고 재개 가능한 체크포인트에서 제거했다.
- Inventory ownership·Container·Reservation·Atomic Transfer·ViewData·Inventory→Fitting 경계를 `VehicleInventory.md v1.0.0` Current System으로 승격했다. RuntimeApply는 non-owning 즉시 적용 UX로 구분한다.
- `FFIT-P0-05`는 USER PASS가 아닌 `Superseded / Not Executed`이며 formal ownership-aware frontend가 필요해질 때 별도 lifecycle로 연다. Source/Asset/Build/Automation/PIE mutation 0, 현재 단일 Active `CF-FQ-039` 유지다.

### v2.45.10 - 2026-09-14

- `CF-FQ-034 차량 피팅·질량 런타임`을 current Source/System Rebaseline로 Paused → Done 전환하고 재개 가능한 체크포인트에서 제거했다.
- Fitting/Mass Current owner를 `VehicleRuntime.md v1.3.0`, Vehicle mass data foundation을 `VehicleData.md v2.3.0`, 향후 feel/tuning workflow를 `VehicleBuilder.md v1.6.0`으로 고정했다.
- `FIT-P0-07D`는 USER PASS가 아닌 `Superseded / Not Executed`이며 향후 필요 시 Deferred observational debt로 successor tuning workflow에서 관찰한다. Source/Asset/Build/Automation/PIE mutation 0, 현재 단일 Active `CF-FQ-039` 유지다.

### v2.45.9 - 2026-09-11

- `CF-FQ-015 Vehicle Data Tuning`을 Rebaseline Complete로 Paused → Done 전환하고 재개 가능한 체크포인트에서 제거했다.
- VD-P0-00~03 Historical Technical PASS는 보존하고 VD-P0-04는 `Superseded / Not Executed`로 종료했다. Current tuning owner는 `VehicleBuilder.md v1.6.0`, data foundation은 `VehicleData.md v2.3.0`이다.
- 제동·Yaw Rate·횡가속·Slip Angle·Suspension 계측은 현재 완료 기능으로 확대하지 않고 Measurement Gap 후보로 보존한다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v2.45.8 - 2026-09-11

- `CF-FQ-038 Vehicle Data Authoring`을 DEL1~DEL7 PASS, Legacy Wizard physical retirement, 전체 `CarFight.DataAuthoring` 111/111 PASS를 근거로 Paused → Done 전환했다.
- 현재 구현 owner는 `Systems/Vehicles/VehicleBuilder.md v1.5.0`, Historical Plan/Roadmap은 `DataAuthoringPlan.md v0.2.53` / `DataAuthoringRoadmap.md v0.1.57` Retained Path다.
- UA-08은 Runtime Technical PASS / USER Inconclusive / quantitative comparison Deferred로 보존하며 USER PASS 7/8을 8/8로 확대하지 않는다. 현재 단일 Active `CF-FQ-039`는 변경하지 않았다.

### v2.45.7 - 2026-09-11

- `CF-FQ-029` 모듈형 런처 및 발사 인계를 LM-P0-06 Final Technical Integration PASS로 Done 처리하고 재개 후보에서 제거했다.
- Launcher Current owner를 `Systems/Combat/Launcher.md v1.0.0`으로 추가했다. 기존 USER PIE evidence는 보존하며 Angled/Vertical Release·Carrier Velocity·MuzzleBlocked는 Current Product-path Automation으로 마감했다.
- 이미 2026-09-08 Done으로 승격된 `CF-FQ-030`의 stale Ready/Manual PIE row도 제거하고 `MissileGuidance`를 완료 Combat 기반에 반영했다.
- 현재 단일 Active `CF-FQ-039 Production UI Visual Rework`는 변경하지 않았다.

### v2.45.6 - 2026-08-25

- CF-FQ-039 Vehicle-specific silhouette Source/Production Assetization을 persisted Technical PASS로 반영했다. Sedan/SUV Production Texture 2종과 exact VehicleData 3-entry catalog가 저장됐고 legacy fallback은 보존됐다.
- 대표 Plan/Roadmap 포인터를 v0.1.24로, Current UI System owner를 `InGameUI.md v1.1.10`으로 동기화했다.
- 다음 VehiclePanel Gate를 Defense source family production으로 전진하고 exact `VT-VEH-01` Master Source repository binding은 별도 Pending으로 유지했다.

### v2.45.5 - 2026-08-24

- CF-FQ-039을 `VPR-P0-01 VehiclePanel Production Vertical Slice — Armor Technical PASS / USER Visual Review Pending`으로 전진했다.
- 대표 Plan/Roadmap 포인터를 v0.1.5로, Current UI System owner를 `InGameUI.md v1.1.2`로 동기화했다.
- Armor 기술 검증 PASS를 반영하되 전체 VehiclePanel `VT-VEH-01` USER 승인은 아직 Pending으로 유지한다.

### v2.45.4 - 2026-08-24

- CF-FQ-039 VehiclePanel의 editable Structure Readiness를 기존 `WBP_CFArmorSector` 6개 재사용 + BodyMap Canvas + 탑다운 VehicleSilhouette 구조로 확인했다.
- 현재 Gate를 editable composition 확인 / `VT-VEH-01` USER 명시 승인 대기로 갱신하고 대표 Plan/Roadmap/System 포인터를 동기화했다.
- Production Visual Target USER PASS는 아직 부여하지 않았다.

### v2.45.3 - 2026-08-23

- CF-FQ-039 VPR-P0-00 AI Recovery 완료와 USER Vehicle Target Pending 상태를 반영했다.

### v2.45.2 - 2026-08-23

- CF-FQ-039 VPR-P0-00 실제 Recovery 착수를 반영해 Plan/Roadmap 포인터와 Gate 상태를 갱신했다.

### v2.45.1 - 2026-08-23

- CF-FQ-039 pre-start 설계 교정 후 대표 Plan/Roadmap 포인터를 v0.1.1로 동기화했다.

### v2.45.0 - 2026-08-23

- 사용자 선택에 따라 `CF-FQ-039 Production UI Visual Rework`를 현재 단일 Active Feature로 등록했다.
- 첫 Gate를 `VPR-P0-00 Visual Target Recovery & Registry`로 고정하고 대표 Plan/Roadmap을 연결했다.
- `CF-FQ-032 Done`과 기존 Current System owner는 유지하며 새 Feature가 Visual Production 품질만 후속 소유하도록 분리했다.

Migration: 현재 UI Visual 재개는 CF-FQ-039 Plan을 사용한다. CF-FQ-032 Historical Plan의 old next-action은 현재 Active 기준이 아니다.

### v2.44.0 - 2026-08-22

- 2026-08-18의 `CF-FQ-032 Active / UI-P0-03` stale 상태를 제거하고 최신 `CF-FQ-032 Done / 현재 Active 없음`으로 교정했다.
- `CF-FQ-038`을 UA-01~06 USER PASS / 다음 UA-07 기준으로 갱신했다.
- 과거 Feature별 대형 Build/Automation/PIE 로그와 여러 세대의 Current 상태를 제거하고 현재 기준선·재개 지점·프로젝트 전역 정책만 남겼다.
- 상세 evidence는 대표 Plan, 현재 구현은 Systems가 소유하도록 문서 경계를 정상화했다.

Migration: 과거 ProjectState에 존재하던 Feature별 상세 evidence는 삭제 근거로 사용하지 않는다. 필요한 이력은 해당 대표 Plan, Systems, Archive와 Git history에서 조회한다.
