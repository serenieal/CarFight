# CarFight — 01_ProjectState

> 역할: CarFight 프로젝트의 **현재 실제 기준선 / 현재 Feature 상태 / 현재 리스크**를 고정한다.
> 공통 규칙 원본: `Document/SSOT/`
> 문서 버전: v2.45.7
> 마지막 정리(Asia/Seoul): 2026-09-11
> 문서 상태: Current

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

### 2.1 현재 Active

현재 단일 Active Feature는 **`CF-FQ-039 Production UI Visual Rework`**다.

```text
대표 Plan: Document/Plan/InGameUIVisual/InGameUIVisualPlan.md v0.1.24
Roadmap: Document/Plan/InGameUIVisual/InGameUIVisualRoadmap.md v0.1.24
Current Gate: VPR-P0-01 VehiclePanel Production Vertical Slice — VEHICLE-SPECIFIC SILHOUETTE SOURCE + PRODUCTION TEXTURE/CATALOG ASSETIZATION PASS / DEFENSE SOURCE FAMILY NEXT / MASTER SOURCE BINDING PENDING
```

이 후속 Feature는 `CF-FQ-032 Done`을 취소하지 않는다. 기존 Runtime·Provider·Presenter·ViewData·Designer ownership과 완료 evidence를 보존하면서 Visual Production 품질만 별도 범위로 진행한다.

### 2.2 최근 완료

`CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크`는 Done이다.

현재 구현 owner:

```text
Document/Systems/UI/InGameUI.md v1.1.10
Document/Systems/UI/AimReticle.md v1.10.0
Document/Systems/Targeting/SensorContact.md v1.2.0
```

완료 범위는 UI-P0-02~05 USER PASS, UI-P0-06~10 Technical Complete/AI Runtime Technical Validation, UI-P0-11 Systems Promotion과 2026-08-22 post-closure remediation까지다.

post-closure remediation의 최종 Official Build `bd3640c616794cd7a54cc8a8afa4b021`, broad `CarFight.UI` 44/44, `CarFight.Sensor` 14/14 PASS evidence는 Historical Plan `Document/Plan/Archive/InGameUIPlan.md v0.59.36`가 소유한다. 새 관련 failure evidence가 없으면 반복하지 않는다.

UI-P0-08 Radar/Edge Visual·Zoom Feel과 D1-11-ART SpeedGauge·VehiclePanel·전체 Visual Review는 **비차단 Deferred/Pending**이며 USER PASS로 확대하지 않는다.

---

## 3. 현재 재개 가능한 체크포인트

| Feature | 상태 | 현재 재개 지점 |
| --- | --- | --- |
| `CF-FQ-038` 차량 데이터 Authoring | Paused | `Document/Plan/DataAuthoring/DataAuthoringPlan.md v0.2.47` / `Document/Plan/DataAuthoring/DataAuthoringRoadmap.md v0.1.52` / UA-01~06 USER PASS 보존 / 다음 `UA-07 Driving Feel Authoring` / Remote Technical Readiness PASS |
| `CF-FQ-034` 차량 피팅·질량 런타임 | Paused | `Document/Plan/VehicleFitting/VehicleFittingPlan.md v0.17.0` / `FIT-P0-07D USER Driving Feel Comparison` |
| `CF-FQ-035` 인벤토리 Foundation | Paused | 기존 Technical checkpoint 보존 / USER Field UI·Mobility Pending |
| `CF-FQ-026` 타겟 선택 시스템 | Paused | `TS-P0-08 USER PIE Pending` |
| `CF-FQ-015` 차량 데이터 튜닝 패스 | Paused | `VD-P0-04 USER Tuning Pending` |

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
- SensorContact + Scanner integration

UI
- InGameUI / AimReticle / VehicleDebug

Vehicles
- VehicleData / VehicleRuntime / VehicleDrive / VehicleSteering
- WheelSync / VehicleCamera / VehicleAim

Inventory/Fitting
- 완료 또는 부분 완료 Current 계약은 Systems와 대표 Plan의 실제 상태를 함께 확인
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
- ProjectSSOT/ActiveWork/FeatureQueue는 상세 evidence 저장소로 사용하지 않는다.
```

---

## 8. Changelog

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
