# CarFight — 02_Roadmap

> 역할: CarFight 프로젝트의 **현재 해야 할 일 / 우선순위 / 완료 조건 / 진행 순서**를 고정한다.
> 기준 상태 문서: `01_ProjectState.md`
> 상위 방향 문서: `00_Vision.md`
> 문서 버전: v2.19.0
> 마지막 정리(Asia/Seoul): 2026-07-30


---

## 이 로드맵의 성격
이 문서는 아이디어 모음이 아니다.

이 문서의 목적은 아래 4가지다.
- 문서 방향을 먼저 정렬한다.
- 확인된 싱글 실행 / 로컬 Aim / Fire / Reticle 기반 위에 전투 루프를 올린다.
- 기준 차량 1대의 무기 조준, 발사, 피격, 피해, 피드백 흐름을 검증한다.
- 서버/멀티/관리툴 작업을 현재 일정에서 제외해 개발 기간을 단축한다.

---

## 이번 사이클의 최상위 목표
이번 사이클의 목표는 아래 한 문장으로 고정한다.

> **서버/멀티플레이 범위를 현재 일정에서 제외하고, 싱글 플레이 기준 1대 차량의 조준, 발사, 피격, 피해, 피드백이 반복되는 핵심 전투 루프를 검증한다.**

이번 사이클은 2~3개월 일정 단축을 목표로 한다.
서버 권한 전투, Dedicated Server 검증, 2클라 테스트, CFNetSmooth 차량 적용, 서버 런처, 세션/로비는 현재 범위에서 제외한다.

---

## 현재 우선순위 상태 (2026-07-30)

`CF-FQ-029 모듈형 런처 및 발사 인계`는 `LM-P0-01~04` C++ 적용·공식 Editor 빌드와 `LM-P0-05 Launcher Editor Assets` 적용·독립 AssetDump 검증을 완료했다. 현재 단계는 `LM-P0-06 Launcher Integration PIE`이며 `CF-TC-025·026`은 사용자 확인 전 TODO다. `CF-FQ-030 물리 제한형 미사일 비행·유도`는 Ready 상태 안에서 `MG-P0-00` 비활성 타입·Config·Guidance 수학과 Editor 빌드를 완료했지만 런타임에는 연결하지 않았다. `MG-P0-01`은 이 통합 검증과 CF-FQ-029 완료 판정을 기다린다.

```text
1. CF-FQ-029 모듈형 런처 및 발사 인계: Active / LM-P0-01~04 Code Applied / LM-P0-05 Assets Applied·AssetDump PASS / LM-P0-06 CF-TC-025·026 User PIE Active
2. CF-FQ-030 물리 제한형 미사일 비행·유도: Ready / MG-P0-00 Inactive Foundation Code Applied·Build PASS / MG-P0-01은 CF-FQ-029 LM-P0-06·완료 판정 선행
3. CF-FQ-031 차량 탄약·재장전 런타임: Ready / Documentation Done / Source Not Modified
4. CF-FQ-032 인게임 전투 HUD 및 UI 프레임워크: Ready / UI-P0-00 Documentation Done / Source·Asset Not Modified
5. CF-FQ-028 발사체 추진 시스템: Done / User PIE PASS / CF-TC-024 PASS / Systems Current
6. CF-FQ-027 투사체 비행 FX: Done / User PIE PASS / CF-TC-023 PASS / Systems Current
7. CF-FQ-026 타겟 선택 시스템: Paused / TS-P0-08 체크포인트 보존
8. CF-FQ-024 전투 FX: Done / User PIE PASS / CF-TC-021 PASS
9. CF-FQ-020 조작감, 전투 템포, 피드백 개선: Candidate
10. CF-FQ-021 핵심 게임 루프 검증: Candidate
11. CF-FQ-019 주행과 전투 흐름 반복 테스트: Deferred / 런처·미사일 구현 범위 확정 후 재설계
```

현재 작업은 `CF-FQ-029`다. `CF-FQ-030`, `CF-FQ-031`과 `CF-FQ-032`는 Ready 상태이며 사용자가 실제 착수를 선택하기 전에는 현재 Active 작업을 변경하지 않는다. `CF-FQ-019`는 현재 착수 순서에서 제외한 Deferred 작업이다.

발사체 추진 완료 체크포인트:

```text
- 완료 Feature: CF-FQ-028 Done
- 완료 Test: CF-TC-024 PASS
- Current System: Document/Systems/Combat/Projectile.md v1.5.0
- 완료 Plan: Document/Plan/ProjectilePropulsionPlan.md v1.2.0
- CFProjectileMotorTypes v1.0.0
- UCFProjectileMotorComp v1.0.0
- UCFProjectileData v1.7.1 / PropulsionConfig·독립 FX Scale
- ACFProjectileActor v1.8.1 / MotorComp·Burning 기반 Thruster·Pool Reset
- InitialSpeed 분리 → IgnitionDelay → Burning 고정 방향 가속 → BurnedOut 관성·중력 비행
- 최종 빌드: e8b812bd479549299dd116f9bae8996f / Exit Code 0
- Automation: RuntimeContract 소스 컴파일 PASS / 실행 Not Run
- 사용자 PIE: DA_PFX_ThrusterTest 추진·FX_Exhaust·Scale 0.2 PASS
- 반복 전투 확장 회귀: CF-FQ-019 Deferred / 런처·미사일 구현 범위 확정 후 재설계
```

투사체 비행 FX 완료 체크포인트:

```text
- 완료 Feature: CF-FQ-027 Done
- 완료 Test: CF-TC-023 PASS
- Current System: Document/Systems/Combat/Projectile.md v1.5.0
- 완료 Plan: Document/Plan/ProjectileFlightFxPlan.md v1.0.0
- Trail-only / Thruster-only / Trail+Thruster: PASS
- 유효 소켓 / Missing Socket Fallback: PASS
- Hit·LifeExpired Reset / Pool 20발 이상 / Ribbon History 무잔류: PASS
- 30 FPS 고속 Bounds·컬링과 Impact·Damage·Pool 회귀: PASS
- Automation: RuntimeContract 소스 컴파일 PASS / 실행 Not Run — Runner 미노출
```

`CF-FQ-024`의 일회성 Muzzle·Impact·Destroyed Current System은 다시 열지 않는다.
`CF-FQ-027`과 `CF-FQ-028`의 검증된 지속형 FX·비유도 Rocket 추진 기준은 `Systems/Combat/Projectile.md`를 우선한다.
`CF-FQ-019`는 기존 반복 전투 범위를 그대로 실행하지 않고 Deferred로 이동한다. `CF-FQ-029`와 `CF-FQ-030` 이후 런처·미사일을 포함한 새 통합 회귀 범위로 다시 설계한다.
`CF-FQ-026`의 완료 코드와 Automation 결과는 Paused 체크포인트로 유지한다.

---

## 현재 진행 상태 (2026-07-15)

### 완료된 기준선

- `ProjectSSOT` 루트 정리는 완료됐다.
- `Document/ProjectSSOT/`는 `README + 00~05 + Archive` 구조로 정리됐다.
- 완료 기능과 현재 구현 기준은 `Document/Systems/`를 우선 기준으로 본다.
- 차량 코어 기준선은 `BP_CFVehiclePawn / ACFVehiclePawn / UCFVehicleDriveComp / UCFWheelSyncComp / UCFVehicleData / DA_PoliceCar` 조합으로 유지한다.
- 차량 코어의 기본 주행 / WheelSync / DriveState는 과거 P0 기준에서 PASS로 본다.
- `VehicleCamera`, Local Aim, Fire, Reticle 골격은 현재 싱글 전투 루프의 선행 기반으로 유지한다.
- `Document/Systems/Network/ServerSpawn.md` 기준 Dedicated Server 최소 Spawn/Possess 흐름은 존재하지만 현재 일정에서는 보류한다.

### 현재 진행 체크포인트

2026-07-15 기준 완료 상태:

```text
- C1 차량 무기 조준 및 발사: 완료
- C2-A Reticle / FireFeedback UI: 완료
- C3 피격 판정 및 피해 처리: 완료
- 조준점·터렛·총구 정렬과 고속 Projectile 신뢰성: 완료
```

현재 남은 사이클 과제:

```text
1. 완료된 주행·조준·발사·피격·피해·전투 FX를 반복 전투에서 함께 회귀 검증한다.
2. 전체 FPS·속도 조합과 이동 차량 대상 반복 전투를 확인한다.
3. 전투 템포와 피드백 문제를 기능 차단과 품질 개선으로 분리한다.
4. 핵심 게임 루프가 다음 개발 단계로 넘어갈 수 있는지 최종 판정한다.
```

프로젝트 전역 사운드 정책:

```text
- CarFight는 게임 사운드를 구현하지 않는다.
- SoundWave, SoundCue, MetaSound Source, Sound Attenuation 자산을 제작·도입하지 않는다.
- USoundBase, UAudioComponent와 사운드 재생 코드를 추가하지 않는다.
- 엔진음, 타이어음, 충돌음, 무기음, 피격음, 파괴음, UI음과 BGM은 완료 조건에서 제외한다.
- Unreal의 기본 플랫폼 오디오 설정은 엔진 생성 설정으로 남길 수 있으나 게임 기능으로 간주하지 않는다.
```

따라서 현재 진행 순서는 아래로 갱신한다.

```text
완료: C2-B / CF-FQ-024 전투 FX
완료: PROP / CF-FQ-028 발사체 추진 시스템
- PP-P0-00~09 구현·빌드·사용자 PIE Done
- PP-P0-10A Systems 승격 Done
- CF-TC-024 PASS
- Current System: Document/Systems/Combat/Projectile.md
현재 Active: LCH / CF-FQ-029 모듈형 런처 및 발사 인계
현재 단계: LM-P0-01~04 Code Applied / LM-P0-05 Assets Applied·AssetDump PASS / LM-P0-06 Launcher Integration PIE Active
선행 대기: MSL / CF-FQ-030 물리 제한형 미사일 비행·유도 / Ready / MG-P0-00 Inactive Foundation Build PASS / Runtime Not Connected / MG-P0-01은 CF-TC-025·026 선행
후속 Candidate: C5 / CF-FQ-020 조작감, 전투 템포, 피드백 개선
후속 Candidate: C6 / CF-FQ-021 핵심 게임 루프 검증
후순위 Deferred: C4 / CF-FQ-019 주행과 전투 흐름 반복 테스트 / 런처·미사일 이후 재설계
장기 후속: CF-FQ-012 1대 차량 주행감 고도화
```

`CF-FQ-028`은 현재 구현 기준으로 승격됐으며 Plan은 완료 이력으로 전환했다. `CF-FQ-019`는 Deferred이며 현재 후보 순서에서 자동 또는 수동 착수 대상으로 해석하지 않는다.

### 이번 사이클에서 하지 않을 것

```text
- Dedicated Server 고도화
- 2클라 Spawn/Possess 검증
- 이동 복제 / 원격 차량 보간
- 서버 권한 발사 요청
- CFNetSmooth 차량 적용
- 서버 런처 / 서버 운영 도구
- 로비 / 세션 / 매치메이킹
- 정식 웹 관리툴 선구축
- 대규모 서버 프레임워크 선구축
- 인벤토리 / 보상 / 경제 시스템 선구축
- 새 차종 양산
- CMVS 최종 구조 전환 구현
- 차량 모델링 파이프라인 확정
```

관리툴은 현재 싱글 전투 루프 검증 범위에서 제외한다.

---

## 이번 사이클의 절대 기준

### 먼저 끝내야 하는 것

1. 확인된 싱글 실행 / 기준 차량 조작 / 로컬 Aim / Fire / Reticle 기반을 유지한다.
2. 기준 차량 1대에 최소 무기 조준 / 발사 경로를 구현한다.
3. 발사 성공/불가/쿨다운 같은 상태를 시각 이펙트와 조준 UI로 읽히게 한다.
4. 피격 판정과 피해 처리를 붙여 전투 결과가 남게 한다.
5. 주행 중 조준/발사/피격/피해가 반복되는 테스트 루프를 만든다.
6. 조작감, 전투 템포, 피드백을 한 번에 크게 바꾸지 않고 관찰 가능한 단위로 조정한다.
7. 서버/멀티 관련 Plan은 현재 착수 대상에서 제외하거나 보류 표시한다.

### 아직 하지 않을 것

- 서버 Target 삭제
- `CFMPGameMode` 하드 삭제
- CFNetSmooth 서브모듈 제거
- 서버 관련 Plan 폴더 대량 이동
- 정식 웹 관리툴 구현
- 인벤토리 / 보상 / 경제 시스템 구현
- 탄종/무기 밸런스 대량 설계
- 무기 종류 대량 추가
- 장갑 방향 / 피팅 / 열 / 탄약 경제 전체 구현
- AI 교전 전체 구현
- 새 차종 양산
- CMVS 최종 구조 전환 구현
- 차량 모델링 파이프라인 확정
- 서버 프레임워크를 게임 기능 없이 먼저 크게 만드는 작업

주의:
- 2026-06-19 이전 전환 세션은 기능 추가 없이 싱글플레이 전환 정리만 다뤘다.
- 현재 신규 개발은 싱글 전투 루프 검증 순서로 진행한다.
- 서버 Target, CFMPGameMode, CFNetSmooth, 서버 관련 보관 문서는 삭제하지 않고 Deferred 대상으로 보존한다.

---

## 전체 진행 순서
1. **S0 — ProjectSSOT 싱글 전환**: 완료
2. **S0-1 — 멀티플레이 선로 비활성화**: 완료
3. **S1 — 싱글 실행 기준선 전환**: 완료
4. **C1 — 차량 무기 조준 및 발사 기능 구현**: 완료
5. **C2-A — Reticle / FireFeedback UI 구현**: 완료
6. **C3 — 피격 판정 및 피해 처리 구현**: 완료
7. **C2-B — 전투 FX 구현**: 완료 / User PIE PASS / CF-TC-021 PASS
8. **PFX — CF-FQ-027 투사체 비행 FX**: 완료 / User PIE PASS / CF-TC-023 PASS / Systems Current
9. **PROP — CF-FQ-028 발사체 추진 시스템**: 완료 / User PIE PASS / CF-TC-024 PASS / Systems Current
10. **LCH — CF-FQ-029 모듈형 런처 및 발사 인계**: Active / LM-P0-01~04 Code Applied·Build PASS / LM-P0-05 Assets Applied·AssetDump PASS / LM-P0-06 CF-TC-025·026 User PIE Active
11. **MSL — CF-FQ-030 물리 제한형 미사일 비행·유도**: Ready / MG-P0-00 Inactive Foundation Applied·Build PASS / MG-P0-01은 LCH CF-TC-025·026 사용자 PIE와 완료 판정 선행
12. **C5 — 조작감 / 전투 템포 / 피드백 개선**: Candidate
13. **C6 — 핵심 게임 루프 검증**: Candidate
14. **C4 — 주행과 전투 흐름 반복 테스트**: Deferred / 런처·미사일 구현 범위 확정 후 재설계

현재 Active 단계는 LCH / CF-FQ-029다. Launch Handoff, Multi-Muzzle, SingleCycle, Pattern Data, Ripple·Salvo Scheduler와 Angled·Vertical Ejection의 C++·공식 빌드, LM-P0-05 Launcher Editor Assets와 독립 AssetDump 검증은 완료했다. 현재 작업은 LM-P0-06 CF-TC-025·026 사용자 PIE다. MSL / CF-FQ-030은 MG-P0-00 비활성 Foundation만 준비됐으며 MG-P0-01 런타임은 이 검증과 CF-FQ-029 완료 판정 뒤 착수한다.

---

## S0 — ProjectSSOT 싱글 전환
### 목표
활성 판단 문서가 모두 싱글 플레이 1대 차량 기준과 이후 로컬 전투 루프 검증 방향을 가리키게 한다.

### 완료 조건
- [x] `00_Vision.md`가 싱글 전환 목표를 명시한다.
- [x] `01_ProjectState.md`가 서버/멀티를 보류 상태로 해석한다.
- [x] `02_Roadmap.md`가 S0, S0-1, S1, C1~C6 순서로 재정렬된다.
- [x] `03_FeatureQueue.md`의 P0가 싱글 전투 루프 작업으로 바뀐다.
- [x] `04_ProjectDecisions.md`에 전환 결정이 기록된다.
- [x] `05_TestChecklist.md`의 Network 항목이 현재 기준 `N/A`로 내려간다.

---

## S0-1 — 멀티플레이 선로 비활성화
### 목표
서버/멀티 관련 문서가 현재 싱글 작업의 활성 계획처럼 읽히지 않게 분리한다.

### 해야 할 일
- [x] `Document/Plan/Archive/MP_ServerPlan`, `ServerUpgradePlan`, `VehicleNetSyncPlan`, `CFNetSmoothPlan`을 보류 계획으로 분류
- [x] `Document/Plan/README.md`와 `Document/Plan/Archive/README.md`에 활성/보관 기준 문서화
- [x] 코드, Target, 플러그인, 배치 파일은 이번 전환 범위에서 삭제/분류하지 않는다고 명시

### 완료 조건
- [x] 현재 착수 계획에서 서버/멀티 문서 항목이 제거되어 있다.
- [x] 남기는 서버 관련 문서는 보류 이유가 문서화되어 있다.
- [x] 서버 관련 코드/툴 파일은 현상 유지하고, 이번 세션에서 삭제/분류하지 않는다는 기준이 문서화되어 있다.

---

## S1 — 싱글 실행 기준선 전환
### 목표
에디터 실행과 PIE 1인 플레이가 서버 GameMode 전제 없이 기준 차량 1대를 바로 조작하는 상태가 되게 한다.

### 해야 할 일
- [x] `DefaultEngine.ini`의 기본 GameMode 연결 확인
- [x] 싱글 기준 GameMode 또는 기존 배치 차량 점유 방식 결정
- `/Game/Maps/TestMap`에서 기준 차량 1대 조작 루프 확인

### 완료 조건
- [x] `Tools\RunEditor.bat` 실행 후 에디터가 열린다.
- [x] PIE 1인 플레이에서 `BP_CFVehiclePawn`을 바로 조작할 수 있다.
- [x] 서버 창, 2클라 창, 네트워크 접속 없이 검증 가능하다.

---

## C1 — 차량 무기 조준 및 발사 기능 구현
### 목표
기준 차량 1대에서 로컬 조준 상태를 실제 발사 경로로 연결한다.

### 범위
- 첫 구현은 단일 기준 차량과 최소 기준 무기 1개로 제한한다.
- 서버 권한 발사, 복제, PvP 판정은 현재 범위에서 제외한다.
- 무기 종류 대량 설계보다 `조준 입력 -> 발사 가능 판단 -> 발사 결과` 경로를 우선한다.

### 해야 할 일
- 현재 `VehicleCamera / VehicleAim / AimReticle` 기반에서 발사 방향과 발사 원점을 추적한다.
- Fire 입력이 성공/실패 사유를 남기도록 정리한다.
- 최소 기준 무기의 쿨다운 또는 연사 제한 위치를 정한다.
- 발사 방식은 상세 Plan에서 확정하되, ProjectSSOT 기준에서는 로컬 발사 루프 존재 여부만 판정한다.

### 완료 조건
- [x] PIE 1인 플레이에서 기준 차량이 조준 방향으로 발사 요청을 만든다.
- [x] 발사 성공 / 발사 불가 / 쿨다운 상태를 구분할 수 있다.
- [x] 발사 원점, 발사 방향, 현재 조준 상태를 디버그 또는 UI로 확인할 수 있다.
- [x] 서버 창, 2클라 창, 네트워크 접속 없이 검증 가능하다.

현재 판정:

```text
CF-FQ-016 Done
CF-TC-013은 전체 확장 회귀 전까지 PARTIAL 유지
```

---

## C2 — 전투 피드백 구현

C2는 UI와 실제 월드 연출의 생명주기를 분리한다.

### C2-A — Reticle / FireFeedback UI

상태: 완료

완료 범위:

```text
- Ready / FireSuccess / Cooldown 상태와 색상
- NoWeapon 회색 표시
- AimBlocked 주황 표시
- TurretAligning amber와 MuzzleBlocked 표시
- 피드백 유지 시간 종료와 정상 상태 복귀
```

현재 판정:

```text
CF-FQ-017 Done
CF-TC-014 PASS
```

### C2-B — 전투 FX

상태: `CF-FQ-024 Done / User PIE PASS / CF-TC-021 PASS`

현재 구현 기준:

```text
Document/Systems/Combat/CombatFx.md
```

목표:
- 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 Niagara 기반 시각 연출로 즉시 이해하게 만든다.

범위:
- 판정과 연출을 분리한다.
- 소스 코드에 특정 Niagara 자산 경로를 하드코딩하지 않는다.
- 기준 무기·발사체·차량의 P0 시각 연출을 우선한다.
- 게임 사운드 자산, 런타임과 오디오 모듈은 프로젝트 전역 범위에서 제외한다.
- 상세 기준은 `Document/Plan/CombatFxAudio/ImplementationDesign.md`를 따른다.
- `CombatFxAudio`는 경로 호환용 레거시 디렉터리명이며 현재 범위에 Audio는 포함하지 않는다.

완료 범위:
- 실제 Muzzle 위치의 발사 Niagara 연결
- `FCFDamageHitContext.ImpactLocation / ImpactNormal` 기반 Impact Niagara 연결
- `SM_Body.FX_Destroyed` 소켓 기반 최초 Destroyed Niagara 연결
- 발사 거부, 중복 Impact와 파괴 후 추가 피해의 FX 중복 방지
- WeaponData / ProjectileData / VehicleData의 CombatFxData 참조
- MaximumLifetimeSeconds Loop 잔류 안전 퓨즈
- Editor Preview Actor와 DataAsset 튜닝 흐름
- 게임 오디오 클래스·모듈·에셋 참조 0개 유지

완료 조건:
- [x] 승인된 발사 1회당 Muzzle FX가 1회 발생한다.
- [x] 발사 거부 상태에서는 발사 FX가 발생하지 않는다.
- [x] HitScan과 Projectile의 첫 Impact에서 FX가 1회 발생한다.
- [x] 최초 차량 파괴 전환에서 `FX_Destroyed` 소켓 위치의 파괴 FX가 1회 발생한다.
- [x] 중복 충돌, 추가 피해와 반복 전투에서 Niagara 중복·잔류가 없다.
- [x] FX 자산 누락 또는 생성 실패가 기존 발사·피해·파괴 판정을 변경하지 않는 null-safe 계약이 구현됐다.
- [x] 게임 오디오 자산·런타임·모듈 참조가 0개다.
- [x] Editor 빌드와 사용자 PIE에서 `CF-TC-021`을 통과했다.

---

## C3 — 피격 판정 및 피해 처리 구현
### 목표
발사 결과가 피격 판정과 피해 처리로 이어져 전투 결과가 남게 한다.

### 범위
- 첫 구현은 최소 목표물 또는 기준 차량 간 피격 판정으로 제한한다.
- 장갑 방향, 모듈 손상, 탄종 상성, 파괴 연출 전체 구현은 현재 범위에서 제외한다.

### 해야 할 일
- 발사 결과가 맞았는지 판단하는 최소 피격 경로를 정한다.
- 피해량 적용 대상과 생존 상태 위치를 정한다.
- 피해 처리 실패 시 원인을 Aim / Fire / Hit / Damage로 분리해 기록한다.
- 완료 후 Systems 승격 후보 문서 경로를 정한다.

### 완료 조건
- [x] 발사 결과가 목표물 또는 차량에 피격으로 기록된다.
- [x] 피해량이 누적되고 현재 체력 또는 파괴 상태를 확인할 수 있다.
- [x] 빗나감 / 피격 / 피해 적용 실패를 구분할 수 있다.
- [x] 서버 없이 로컬 테스트에서 반복 검증 가능하다.

현재 판정:

```text
CF-FQ-018 Done
CF-TC-015 / CF-TC-016 / CF-TC-020 PASS
```

---

## C4 — 주행과 전투 흐름 반복 테스트
### 목표
주행, 조준, 발사, 피격, 피해와 전투 시각 FX가 한 흐름으로 반복되는지 확인한다.

### 범위
- 테스트는 `DA_PoliceCar`와 현재 기준 테스트 맵을 우선 사용한다.
- 새 맵, 새 차종, AI 교전 전체 구현은 핵심 루프가 닫힌 뒤 검토한다.

### 해야 할 일
- 정지 상태, 저속, 중속, 회전 중 발사 테스트를 나눈다.
- 발사 중 주행 조작이 끊기지 않는지 확인한다.
- 휠 시각 품질, 카메라 흔들림, Reticle 가독성, 발사·Impact·파괴 FX를 함께 관찰한다.
- 발사 1회당 FX 횟수, Projectile Pool 재사용과 Niagara 잔류를 확인한다.
- 실패를 Core / Data / Asset / Input / UI / FX / Quality / DocMismatch로 분류한다.

### 완료 조건
- [ ] 주행 중 조준/발사 입력이 끊기지 않는다.
- [ ] 같은 루프를 10회 이상 반복해도 입력, UI, 판정과 FX 상태가 꼬이지 않는다.
- [ ] 발사·Impact·파괴 연출의 중복과 Projectile Pool 잔류가 없다.
- [ ] 조작감 문제와 전투 판정·연출 문제를 분리해 기록할 수 있다.
- [ ] 반복 테스트 결과가 `05_TestChecklist.md`에 반영된다.

---

## C5 — 조작감 / 전투 템포 / 피드백 개선
### 목표
핵심 전투 루프가 기능적으로 도는 상태에서 조작감과 템포를 조정한다.

### 범위
- 조정은 한 번에 하나의 축만 바꾼다.
- 차량 주행값, 무기 발사 템포, 조준 UI와 시각 이펙트를 동시에 크게 바꾸지 않는다.

### 해야 할 일
- 조작감: 주행, 조향, 카메라, 조준이 서로 방해하는지 확인한다.
- 전투 템포: 발사 간격, 쿨다운, 피격 반응이 너무 빠르거나 느린지 확인한다.
- 피드백: 플레이어가 왜 맞았고 왜 못 맞혔는지 읽을 수 있는지 확인한다.
- 튜닝값이 `VehicleData`, 무기 데이터, UI 설정, C++ 기본값 중 어디에 속하는지 기록한다.

### 완료 조건
- [ ] 조작감 문제가 기능 FAIL인지 튜닝 문제인지 분리된다.
- [ ] 전투 템포 조정값의 위치와 이유가 기록된다.
- [ ] 발사/피격/피해 피드백이 플레이어 관점에서 납득 가능하다.
- [ ] 남은 품질 문제는 핵심 루프 차단 여부와 함께 기록된다.

---

## C6 — 핵심 게임 루프 검증
### 목표
현재 싱글 차량 전투 루프가 다음 개발 단계로 넘어갈 수 있는지 판정한다.

### PASS 기준
- [ ] 기준 차량 1대를 조작할 수 있다.
- [ ] 조준 방향을 만들 수 있다.
- [ ] 발사 입력이 성공/실패 상태로 판정된다.
- [ ] 발사 피드백이 화면과 UI로 읽힌다.
- [ ] 피격과 피해가 결과로 남는다.
- [ ] 주행과 전투를 반복해도 상태가 꼬이지 않는다.
- [ ] 남은 문제를 기능 차단 / 품질 후속 / 장기 확장으로 분리할 수 있다.

### 완료 후 처리
- 완료된 기능은 `Document/Systems/Combat/` 또는 관련 Systems 문서로 승격한다.
- 진행 중 상세 계획은 `Document/Plan/CombatPlan/` 또는 별도 기능 Plan으로 정리한다.
- 완료 판정 결과는 `05_TestChecklist.md`와 `04_ProjectDecisions.md`에 반영한다.

---

## 기존 기준선 기록 (2026-06-02 이전)
아래 D0/P0/P1/P2/P3 기록은 기존 차량 코어 기준선과 판단 근거를 보존하기 위한 참고 기록이다.
2026-06-19 이후의 실제 진행 순서는 위 S0, S0-1, S1, C1~C6를 우선한다.

---

## D0 — 문서 방향 정렬
### 목표
최종 방향, 현재 기준선, 구조 갭, 작업 순서를 문서상에서 서로 충돌하지 않게 맞춘다.

### 해야 할 일
- `00_Vision.md`에 최종 방향 / 현재 기준선 / 구조 갭 / 유지할 것 / 교체할 것을 고정한다.
- `01_ProjectState.md`에 현재 실제 상태와 임시 운영 편차만 남긴다.
- `05_TestChecklist.md`에 현재 검증 기준을 통합한다.
- `VehicleCoreDecisions.md`에 유지 구조와 교체 구조 결정을 적는다.
- 중복 문서는 루트에서 내리고 `Archive/`로 이동한다.

### 완료 조건
- [ ] 최종 방향 문장이 문서마다 충돌하지 않는다.
- [ ] 현재 기준선 문장이 문서마다 충돌하지 않는다.
- [ ] `GamepadOnly` 같은 운영 편차와 구조 갭이 분리돼 있다.
- [ ] 루트 활성 문서가 최소 집합으로 줄어 있다.

---

## P0 — 현재 기준선 실측
### 목표
현재 프로젝트가 어떤 자산 / 값 / 맵 / 루틴 위에서 굴러가는지 실제 기준으로 닫는다.

### 해야 할 일
- `BP_CFVehiclePawn` 실제 구성 확인
- `DA_PoliceCar` 실제 값 확인
- `TestMap` 현재 검증 루프 확인
- `WheelSync` 현재 운용 방식 확인
- `DriveState` 디버그 표시 확인

### 완료 조건
- [ ] `BP_CFVehiclePawn` 실제 구성표가 있다.
- [ ] `DA_PoliceCar` 실제 설정표가 있다.
- [ ] `TestMap` 검증 루프가 적혀 있다.
- [ ] WheelSync 현재 설정과 미사용 대상을 분리했다.
- [ ] 실측 결과가 `01_ProjectState.md`와 `05_TestChecklist.md`에 반영돼 있다.

### 현재 상태 메모
- 위 항목 중 문서 / 자산 / 코드 실측으로 확인 가능한 부분은 반영 완료
- 남은 항목은 PIE 런타임 PASS / FAIL 검증이다.
- 현재는 에디터 복구가 확인되었으므로 이 검증을 바로 재개한다.
- 현재 런타임 진행 결과:
  - `P0-001`: PASS
  - `P0-002`: PASS
  - `P0-003`: PASS
  - 다음 작업 대상은 `P0-002` 후속 정리, 특히 디버그 가독성 개선과 Anchor `Z` 미세 보정이다.

### P0-002 역사 기록 — 수정 전 확정 절차
아래 항목은 `P0-002`를 처음 분해하던 시점의 기록이다.
현재 운영 기준과 우선순위는 문서 상단 `현재 진행 상태`를 우선해서 본다.

직접 수정에 들어가기 전에 아래를 먼저 닫는다.

1. `확정 사실`
   - `ApplyVehicleLayoutConfig()`는 현재 `Wheel_Mesh_*`만 재배치한다.
   - `WheelSync`의 조향 회전은 `Wheel_Anchor_*` 기준으로 적용된다.
   - 당시 기본 설정에서는 스핀 적용 경로가 꺼져 있었다.
2. `강한 가설`
   - 초기 배치 FAIL과 조향 피벗 FAIL은 `Anchor`와 `Mesh`의 기준 분리에서 같이 발생했을 가능성이 높다.
3. `수정 전 결정`
   - 최종 배치 기준을 `Wheel_Anchor_*` 중심 구조로 둘지 먼저 확정한다.
   - `Wheel_Mesh_*`는 Anchor 자식 기준 시각 표현 전용으로 둘지 먼저 확정한다.
   - 스핀 값을 임시 DebugPipe로 볼지, 실제 휠 회전값 경로로 볼지 먼저 정한다.
4. `수정 순서`
   - 배치 기준 정리
   - 조향 피벗 정상화
   - 스핀 시각 경로 복구
   - 디버그 UX 정리

---

## P1 — 공통 코어 검증
### 목표
현재 코어가 특정 테스트 차량 전용 임시 구조가 아니라, 다음 차종에도 옮길 수 있는 공통 코어인지 검증한다.

### 해야 할 일
- `05_TestChecklist.md`의 DriveState / WheelSync 기준으로 검증한다.
- 실패 항목을 코어 문제와 튜닝 문제로 분리한다.
- 공통 규칙이 아닌 현재 구조 종속 처리 후보를 기록한다.

### 완료 조건
- [ ] DriveState 기본 흐름 PASS / FAIL이 정리되어 있다.
- [ ] WheelSync 공통 규칙 관점의 문제점이 정리되어 있다.
- [ ] 코어 문제와 차량별 수치 문제가 분리되어 있다.

---

## P2 — 첫 차량 기준값 정리
### 목표
`DA_PoliceCar`를 현재 기준선 차량으로서 안정화하되, 최종 구조 대체물처럼 굳지 않게 정리한다.

### 해야 할 일
- `05_TestChecklist.md`에 정리한 시작값 기준으로 DriveState를 조정한다.
- 현재 차량 전용 조정과 공통 코어 문제를 구분한다.
- 이후 다른 차량에도 옮길 수 있는 항목과 아닌 항목을 구분한다.

### 완료 조건
- [ ] 현재 시작값 표가 정리되어 있다.
- [ ] 차량 전용 조정 항목이 분리되어 있다.
- [ ] 공통 코어를 덮는 식의 튜닝이 금지되어 있다.

---

## P3 — 구조 전환 설계 입력값 고정
### 목표
실제 구조 전환 작업 전에, 무엇을 유지하고 무엇을 교체할지 문서와 결정 로그로 먼저 고정한다.

### 해야 할 일
- `VehicleCoreDecisions.md`에 유지 코어와 교체 구조를 확정한다.
- 현재 하이브리드 구조에서 전환 시 영향을 받을 지점을 정리한다.
- 최종 구조 전환을 별도 UE 작업으로 시작할 준비를 마친다.

### 완료 조건
- [ ] 유지할 코어 목록이 고정되어 있다.
- [ ] 교체할 구조 목록이 고정되어 있다.
- [ ] 다음 UE 작업이 "무엇을 바꾸는 작업인지" 문서상에서 명확하다.

---

## 후속 착수 메모 — Camera Baseline (2026-04-15 추가)
이 섹션은 기존 로드맵을 뒤집는 새 상위 목표가 아니라,
현재 기준선 코어 정리 이후 실제로 착수한 **카메라 기준선 작업**이 어디에 붙는지 설명하는 보강 메모다.

### 현재 판정
- 현재 카메라 작업은 `차량 전투 규칙 구현`이나 `최종 카메라/HUD 확장` 단계로 보지 않는다.
- 현재 작업의 성격은 **차량 기준선 위에 기본 카메라 시스템 뼈대와 입력 경로를 먼저 고정하는 보조 기준선 확보 작업**이다.
- 따라서 이 작업은 로드맵 상위 우선순위를 바꾸기보다, 이후 HUD / 무기 연동 / 카메라 확장을 안전하게 붙이기 위한 준비 단계로 본다.

### 현재 완료 기준
- `UCFVehicleCameraComp` 기반 카메라 코어 추가
- `ACFVehiclePawn`의 Look 입력 연동 추가
- `BP_CFVehiclePawn` 카메라 계층 기준선 정리
- `IA_LookAround` 2D 입력 기준선 확인
- Yaw / Pitch 기본 회전 확인
- 빌드 성공 확인

### 현재 남은 작업
- 카메라 감각 튜닝
- 충돌 감각 튜닝
- HUD / 조준점 최소 버전
- 무기 시스템 기반 Aim Profile 실제 연동
- Reverse / Airborne / Destroyed 확장 모드 정리

### 해석 규칙
- 현재 카메라 작업은 어디까지나 **기본 카메라 기준선 확보**다.
- `차량 전투 규칙 구현`, `HUD 확장`, `무기별 카메라 모드 완성`은 아직 상위 로드맵 기준에서 후속 단계로 남는다.
- 세부 구현 기준은 `Document/Plan/CameraPlan/` 문서군에서 본다.

---

## 변경 이력

### v2.19.0 - 2026-07-30

```text
- CF-TC-023 투사체 비행 FX 사용자 PIE 전체 행렬을 PASS 처리했다.
- Trail-only, Thruster-only, Trail+Thruster와 유효 소켓·Missing Socket Fallback을 완료했다.
- Hit·LifeExpired Reset, Pool 20발 이상, Ribbon History 무잔류와 30 FPS 고속 Bounds 회귀를 완료했다.
- CF-FQ-027을 Paused에서 Done으로 전환하고 Projectile Current System v1.5.0에 연결했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run 상태와 Runner 미노출 사실을 유지했다.
- 현재 Active CF-FQ-029 LM-P0-06과 CF-FQ-030~032 Ready, CF-FQ-019 Deferred 상태는 변경하지 않았다.
```

### v2.17.0 - 2026-07-29

```text
- CF-FQ-030 Ready 상태 안에서 MG-P0-00 비활성 Foundation Code Applied와 Editor Build PASS를 반영했다.
- Flight·Guidance Config 기본 비활성, 제한형 비례항법 순수 수학과 Runtime Not Connected 경계를 기록했다.
- 현재 Active는 CF-FQ-029 LM-P0-06을 유지하고 MG-P0-01 Flight State는 해당 사용자 PIE 뒤로 유지했다.
```

### v2.16.0 - 2026-07-29

```text
- LM-P0-05 Launcher Editor Assets, 실제 플레이 진입 경로와 독립 AssetDump 검증 완료를 반영했다.
- LM-P0-06 CF-TC-025·026 사용자 PIE를 현재 Active 단계로 이동했다.
- CF-FQ-030은 LM-P0-06과 CF-FQ-029 완료 판정 이후 착수하는 Ready 작업으로 유지했다.
- 사용자 PIE 전에는 CF-FQ-029 또는 Launcher Systems를 완료 처리하지 않는다.
```
- v2.13.0 (2026-07-28)
  - 사용자 결정에 따라 `CF-FQ-019 주행·전투 반복 테스트`를 가까운 후속 Candidate에서 Deferred로 이동했다.
  - 기존 반복 전투 체크리스트는 폐기하지 않고 `CF-FQ-029`와 `CF-FQ-030` 이후 런처·미사일을 포함한 통합 회귀 범위로 다시 설계하도록 고정했다.
  - 현재 Active `CF-FQ-029`와 Ready `CF-FQ-030`의 순서는 유지했다.
  - 소스, 에셋, 빌드와 PIE 상태는 변경하지 않았다.

- v2.12.0 (2026-07-28)
  - `CF-FQ-029 모듈형 런처 및 발사 인계`를 현재 Active 단계로 등록했다.
  - `CF-FQ-030 물리 제한형 미사일 비행·유도`를 CF-FQ-029 선행 조건을 가진 Ready 단계로 등록했다.
  - LauncherMissile Plan·Design·Roadmap과 런처·미사일 상세 Plan을 생성했다.
  - Direct·Angled·Vertical 사출, Single·Ripple·Salvo와 ProjectileMovement 기반 Ejection 순서를 확정했다.
  - 발사 후 런처 독립, Target Snapshot, 물리 제한형 유도와 명중 비보장 원칙을 고정했다.
  - 첫 코드 단계를 `LM-P0-01 Projectile Launch Handoff Foundation`으로 제한하고 현재 Source Not Modified / Code Entry Ready로 기록했다.
  - `CF-FQ-019`, `CF-FQ-027`과 `CF-FQ-026`의 기존 상태는 유지했다.

- v2.11.0 (2026-07-28)
  - `CF-FQ-028 발사체 추진 시스템`을 Done / User PIE PASS / `CF-TC-024 PASS`로 종료했다.
  - `Document/Systems/Combat/Projectile.md v1.4.0`을 비유도 Rocket 추진과 지속형 Projectile FX의 Current System으로 승격했다.
  - `ProjectilePropulsionPlan.md v1.2.0`을 완료 이력 문서로 전환했다.
  - 현재 Active 단계를 비웠다.
  - `CF-FQ-019`를 다음 Candidate / Not Started로 유지하고 자동 착수하지 않았다.
  - `CF-FQ-027`은 런타임 기반이 Current System에 반영됐지만 별도 전체 체크리스트와 `CF-TC-023` 미완료로 Paused 상태를 유지했다.
  - 이번 승격 작업에서는 반복 전투 회귀를 수행하거나 완료 처리하지 않았다.

- v2.10.0 (2026-07-27)
  - 사용자가 승인한 비유도 Rocket 실제 추진 설계를 `CF-FQ-028 발사체 추진 시스템`으로 등록하고 단일 Active 단계로 전환했다.
  - CFProjectileMotorTypes, UCFProjectileMotorComp, ProjectileData PropulsionConfig와 ProjectileActor Motor·Thruster 상태 연결을 PP-P0-01~05 완료 기준으로 등록했다.
  - InitialSpeed 분리, IgnitionDelay, 고정 LaunchDirection 가속, 최대 추진 속도와 BurnedOut 관성·중력 비행을 구현 순서에 추가했다.
  - 첫 Build Job 984f0ccab94241148e7b40b9d30a5572의 Automation 타입 오류를 수정하고, 최종 형식 정리 후 Build Job 2ffd09357e654bb7970a2e379f5ab45f Exit Code 0을 공식 증거로 기록했다.
  - CF-FQ-027은 C++ Foundation을 유지한 채 Editor 자산 연결 단계에서 Paused로 보존하고, CF-FQ-019를 CF-FQ-028 사용자 PIE 후속으로 이동했다.

- v2.9.0 (2026-07-27)
  - CarFight 코드 작업 기본 방침을 별도 Codex 위임에서 현재 AI 세션의 직접 구현으로 변경했다.
  - CF-FQ-027의 Plan Work-Order Generation 차단 상태를 해제하고 PFX-P0-01 직접 구현을 다음 단계로 지정했다.
  - TaskSource와 WorkOrder는 구현 참고 자료로 유지하고 최종 Codex YAML을 착수 조건에서 제거했다.
  - 직접 구현 후 Git diff, 공식 빌드, Automation과 사용자 PIE 검증 순서는 유지했다.

- v2.8.0 (2026-07-27)
  - 사용자 선택에 따라 CF-FQ-027 투사체 비행 FX를 현재 단일 Active 단계로 등록했다.
  - PFX-P0-00에서 대표 Plan, 로드맵, TaskSource와 사람이 검토 가능한 WorkOrder 초안을 준비했다.
  - ACFProjectileActor 소유, FX_Trail·FX_Exhaust 소켓 우선, ProjectileData Fallback과 Pool 반환 전 Reset을 구현 순서로 고정했다.
  - 계획 테스트 ID CF-TC-023과 PFX-P0-01~07 코드·빌드·자산·PIE·Systems 승격 순서를 추가했다.
  - 현재 Admin 표면에 plan.* 품질·증거 게이트가 없어 최종 Codex YAML Missing과 Source Not Modified 상태를 기록했다.
  - CF-FQ-019를 CF-FQ-027 완료 후 후속 Candidate로 이동하고 CF-FQ-026 TS-P0-08 Paused 상태를 유지했다.

- v2.7.0 (2026-07-27)
  - `CF-FQ-024` 최종 사용자 PIE 전체 PASS와 `CF-TC-021 PASS`를 반영했다.
  - C2-B 전투 FX를 완료 상태로 전환하고 `Document/Systems/Combat/CombatFx.md`를 Current System으로 연결했다.
  - Impact `NS_BasicHit` 현재 크기 승인과 차량별 `FX_Destroyed` 소켓 위치 완료를 기록했다.
  - C4 / `CF-FQ-019`를 선행 조건이 해제된 Candidate로 표시하되 자동 Active 전환하지 않았다.
  - `CF-FQ-026`은 TS-P0-08 Paused 상태를 그대로 유지했다.

- v2.6.0 (2026-07-24)
  - 사용자 결정에 따라 `CF-FQ-024 전투 FX`를 현재 단일 Active 단계로 승격했다.
  - 현재 실행 순서를 FAB 콘텐츠 조사 → Muzzle·Impact·Destroyed 후보 확정 → 공용 FX 계약 구현으로 변경했다.
  - `CF-FQ-026`을 `TS-P0-08 Paused`로 보존하고 완료 코드와 Automation 결과를 회귀 보호 대상으로 유지했다.
  - C2-B 상태와 전체 진행 순서를 Active / Phase 0 기준으로 갱신했다.

- v2.4.0 (2026-07-23)
  - 사용자 결정에 따라 CF-FQ-026 타겟 선택 시스템을 현재 Active 단계로 삽입했다.
  - TS-P0-01 소스 및 Editor 빌드 PASS, Blueprint·PIE Pending 체크포인트를 기록했다.
  - CF-FQ-024는 취소하지 않고 Ready / CF-FQ-026 이후 재개 상태로 변경했다.
  - 기존 전투 사이클의 완료 조건과 후속 순서는 유지했다.

- v2.3.0 (2026-07-15)
  - 완료된 C1, C2-A, C3와 조준·Projectile 신뢰성 작업을 현재 진행 체크포인트에 반영했다.
  - 기존 C2를 Reticle/UI 완료 단계 C2-A와 실제 Niagara/Audio 활성 단계 C2-B로 분리했다.
  - CF-FQ-024를 현재 Active로 지정하고 이후 순서를 C4, C5, C6로 재정렬했다.
  - C4 반복 테스트에 FX/Audio 중복, Pool 잔류와 10회 반복 검증을 추가했다.

- v2.2.0 (2026-06-19)
  - 현재 신규 개발 순서를 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증으로 재정렬했다.
  - 기존 S2~S4의 주행감, 카메라/로컬 Aim, WheelSync 품질 항목은 전투 반복 테스트와 피드백 개선 단계 안에서 함께 검증하도록 통합했다.
  - 서버/멀티 제외 원칙은 유지하되, 다음 목표를 싱글 로컬 전투 루프 검증으로 명확히 했다.

- v2.1.4 (2026-06-19)
  - 사용자 에디터 정상작동 확인을 반영해 S1 싱글 실행 기준선과 S3 로컬 Aim/Reticle 전환 완료 조건을 체크했다.
  - Standalone/패키지 검증은 기능 추가가 아니라 후속 검증 후보로 남겼다.

- v2.1.3 (2026-06-19)
  - 이번 세션 목표를 기능 추가 없이 싱글플레이 전환 정리로 고정했다.
  - `CFSingleGameMode` 기본 실행 기준과 로컬 Aim/Fire 전환 완료분을 로드맵에 반영했다.
  - 서버 Target/CFMPGameMode/CFNetSmooth/보관 문서는 삭제하지 않고 Deferred로 보존한다고 정리했다.

- v2.1.2 (2026-06-19)
  - 이번 세션의 목적을 코드/툴 정리가 아니라 멀티플레이 진행 선로를 싱글플레이 진행 선로로 바꾸는 문서 전환으로 축소했다.
  - 서버/멀티 문서 보류 정리를 개발 전 선행 단계인 S0-1로 이동했다.
  - 서버 관련 코드, Target, 플러그인, 배치 파일은 현상 유지하며 이번 세션에서 삭제/분류하지 않는다고 명시했다.

- v2.1.1 (2026-06-19)
  - 서버/멀티 보류 정리 중 문서 Archive 분리 완료분을 체크했다.
  - Plan 루트/Archive README 신설에 맞춰 보류 문서 경로를 `Document/Plan/Archive/` 기준으로 정정했다.
  - 서버 관련 코드/툴 파일 판단은 이번 문서 전환 세션 범위 밖으로 분리했다.

- v2.1.0 (2026-06-18)
  - 이번 사이클 최상위 목표를 서버 권한 Combat Vertical Slice에서 싱글 플레이 1대 차량 고도화로 전환했다.
  - 싱글 전환 진행 순서를 추가하고 서버/멀티/CFNetSmooth/서버 런처/세션/관리툴을 현재 범위 제외로 명시했다.
  - 기존 D0/P0/P1/P2/P3 기록은 2026-06-02 이전 기준선 참고 기록으로 분리했다.
  - 싱글 실행 기준선, 주행감, 카메라/로컬 Aim/Reticle, WheelSync 품질, 서버/멀티 보류 정리를 다음 작업 흐름으로 고정했다.
  - S0 ProjectSSOT 싱글 전환 완료 조건을 완료 상태로 표시했다.
  - CameraPlan 참조 경로를 현재 `Document/Plan/CameraPlan/` 기준으로 정정했다.

- v1.1.0 (2026-04-15)
  - 문서 버전 / 마지막 정리 날짜를 갱신했다.
  - 기존 로드맵을 바꾸지 않는 범위에서 Camera Baseline 후속 착수 메모를 추가했다.

---

## Migration

### v2.19.0 적용 안내

```text
- CF-FQ-027은 Done / CF-TC-023 PASS이며 Active 또는 Paused로 복원하지 않는다.
- 투사체 비행 FX의 현재 구현 판단은 Document/Systems/Combat/Projectile.md v1.5.0을 우선한다.
- ProjectileFlightFxPlan.md v1.0.0은 완료 당시 구현·빌드·사용자 PIE 기록으로 유지한다.
- 현재 단일 Active는 CF-FQ-029 LM-P0-06이며 이번 완료 처리로 런처 우선순위를 변경하지 않는다.
- Automation 실행은 Runner 미노출로 Not Run 상태를 유지한다.
```

### v2.13.0 적용 안내

```text
- CF-FQ-019는 Deferred이며 현재 후속 Candidate로 복원하지 않는다.
- 기존 CF-FQ-019 테스트 항목은 삭제하지 않고 런처·미사일 구현 뒤 새 통합 회귀 범위의 입력으로 사용한다.
- 현재 코드 진입점은 LM-P0-01 Projectile Launch Handoff Foundation이다.
- CF-FQ-029 Active와 CF-FQ-030 Ready 상태는 유지한다.
```

### v2.12.0 적용 안내

```text
- 현재 Active 단계는 LCH / CF-FQ-029 모듈형 런처 및 발사 인계다.
- 대표 Plan은 Document/Plan/LauncherMissilePlan.md다.
- LM-P0-00 조사·설계·문서 준비는 완료됐으므로 반복하지 않는다.
- 다음 작업은 LM-P0-01 Projectile Launch Handoff Foundation 직접 구현이다.
- 첫 Task에서는 기존 직사 발사의 위치·방향·속도와 Rocket FixedThrustDirection을 회귀 보호한다.
- CF-FQ-030은 CF-FQ-029 Launch Handoff와 Direct Launcher 검증 전에는 코드 착수하지 않는다.
- CF-FQ-019, CF-FQ-027과 CF-FQ-026의 기존 Candidate·Paused 상태를 유지한다.
```

### v2.11.0 적용 안내

```text
- CF-FQ-028은 Done / CF-TC-024 PASS이며 Active로 복원하지 않는다.
- 현재 구현 판단은 Document/Systems/Combat/Projectile.md를 우선한다.
- ProjectilePropulsionPlan.md는 완료 이력으로 유지한다.
- 현재 Active 단계는 없다.
- CF-FQ-019는 다음 Candidate지만 사용자가 선택하기 전에는 착수하지 않는다.
- CF-FQ-027과 CF-FQ-026의 Paused 체크포인트를 유지한다.
```

### v2.10.0 적용 안내

```text
- 현재 Active 단계는 PROP / CF-FQ-028 발사체 추진 시스템이다.
- 대표 Plan은 Document/Plan/ProjectilePropulsionPlan.md다.
- PP-P0-00~07 C++ Foundation과 공식 Editor 빌드는 완료됐으므로 반복 구현하지 않는다.
- 다음 작업은 Editor 테스트 Rocket ProjectileData 생성, PropulsionConfig·FX_Exhaust 저장과 사용자 PIE다.
- 기존 ProjectileData는 bUsePropulsion=false 기본값으로 기존 InitialSpeed 비행을 유지한다.
- CF-FQ-027은 Editor 자산 연결 단계 Paused, CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
- CF-FQ-019는 CF-FQ-028 사용자 PIE 완료 후 착수하는 반복 전투 Candidate다.
```

### v2.9.0 적용 안내

```text
- 현재 Active 단계는 CF-FQ-027 투사체 비행 FX다.
- 새 세션은 CodeWorkGate.md v2.0에 따라 현재 AI 세션이 코드 작업을 직접 수행한다.
- PFX-P0-00 조사·설계는 완료됐으므로 반복하지 않는다.
- PFX-P0-01 ProjectileData 계약부터 직접 구현한다.
- TaskSource와 WorkOrder는 구현 참고 자료이며 최종 Codex YAML을 기다리지 않는다.
- CF-FQ-024는 Done 상태를 유지하고 일회성 CombatFx를 다시 구현하지 않는다.
- CF-FQ-019는 CF-FQ-027 완료 후 반복 전투 Candidate다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v2.8.0 적용 안내 — 폐기됨, v2.9.0이 대체

> 아래 내용은 당시 실행 기준 보존용이며 현재 코드 작업 판단에는 사용하지 않는다.

```text
- 현재 Active 단계는 CF-FQ-027 투사체 비행 FX다.
- 새 세션은 Document/Plan/ProjectileFlightFxPlan.md의 PFX-P0-00 체크포인트를 우선 복원한다.
- PFX-P0-00 조사·설계는 완료됐으므로 반복하지 않는다.
- 최종 Codex YAML은 Missing이며 plan.* 품질·증거 게이트 전에는 소스 변경을 시작하지 않는다.
- CF-FQ-024는 Done 상태를 유지하고 일회성 CombatFx를 다시 구현하지 않는다.
- CF-FQ-019는 CF-FQ-027 완료 후 반복 전투 Candidate다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v2.7.0 적용 안내

```text
- C2-B / CF-FQ-024는 Done / User PIE PASS / CF-TC-021 PASS다.
- 현재 전투 FX 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- CombatFxAudio Plan은 완료 이력으로 유지하며 Active 실행 문서로 사용하지 않는다.
- C4 / CF-FQ-019는 착수 가능하지만 사용자가 선택할 때만 Active로 전환한다.
- CF-FQ-026은 TS-P0-08 Paused 체크포인트를 유지한다.
```

### v2.6.0 적용 안내

```text
- 현재 Active 단계는 C2-B / CF-FQ-024 전투 FX다.
- 첫 단계는 반입된 FAB 콘텐츠의 NiagaraSystem과 의존성을 확인하는 Phase 0이다.
- CF-FQ-026은 TS-P0-08 Paused이며 FX 작업 중 TargetSelect 구현을 변경하거나 되돌리지 않는다.
- 후보 FX가 확정되기 전에는 공용 C++ 런타임 구현으로 넘어가지 않는다.
- C4 반복 전투 테스트는 CF-FQ-024 사용자 PIE 완료 뒤 착수한다.
```

### v2.5.0 적용 안내

```text
- C2-B와 CF-FQ-024는 Niagara 기반 전투 FX 전용 단계로 해석한다.
- CombatFxAudio는 레거시 경로명이며 Sound 자산이나 Audio 런타임을 뜻하지 않는다.
- C4~C6의 완료 판정은 화면, UI, 시각 FX와 게임 판정만으로 수행한다.
- 게임 오디오 에셋·클래스·모듈을 새 작업에 추가하지 않는다.
```

### v2.3.0 적용 안내

```text
- 기존 C2의 Reticle/UI 완료 결과는 C2-A로 유지한다.
- 실제 Niagara와 공간 사운드 구현은 C2-B / CF-FQ-024로 진행한다.
- C4 반복 전투 테스트는 C2-B 사용자 PIE 완료 전에는 착수하지 않는다.
- 완료된 C1과 C3의 판정 기준은 관련 Systems와 TestChecklist PASS 상태를 유지한다.
- 현재 활성 상세 계획은 Document/Plan/CombatFxAudio/ImplementationDesign.md를 사용한다.
```
