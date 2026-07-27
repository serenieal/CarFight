# CarFight — 03_FeatureQueue

> 문서 버전: v1.18.0
> 작성일(Asia/Seoul): 2026-07-27
> 문서 상태: Active
> 역할: CarFight의 **기능 후보 / 착수 판단 / 클라이언트·서버·관리툴 필요성**을 한 곳에서 관리한다.

---

## 1. 목적

이 문서는 CarFight에서 앞으로 개발할 기능 후보를 기능 단위로 정리한다.

이 문서는 상세 설계서가 아니다.
상세 설계는 기능 착수 후 `Document/Plan/<기능명>/`에 작성한다.

이 문서의 목적은 아래 판단을 돕는 것이다.

```text
- 다음에 어떤 기능을 착수할지
- 이 기능이 싱글 / 로컬 플레이 기준으로 검증 가능한지
- 서버 권한 구조가 현재 범위에서 제외되어야 하는지
- 관리툴 또는 임시 운영 도구가 필요한지
- 완료 후 어떤 Systems 문서로 승격할지
```

---

## 2. 문서 위치와 생명주기

CarFight 문서 흐름은 아래 기준으로 본다.

```text
기능 후보
  -> Document/ProjectSSOT/03_FeatureQueue.md

개발 착수 / 상세 계획
  -> Document/Plan/<기능명>/

개발 완료 / 현재 구현 기록
  -> Document/Systems/<분류>/<기능명>.md

오래된 계획 / 완료된 계획 원본
  -> Document/Plan/Archive/
```

중요:
- `FeatureQueue`는 상세 구현 계획을 쓰지 않는다.
- `Plan`은 앞으로 개발할 기능의 상세 계획만 담는다.
- `Systems`는 개발 완료된 기능의 현재 구현 기준만 담는다.

---

## 3. 상태 표기

| 상태 | 의미 |
|---|---|
| `Candidate` | 후보. 아직 착수하지 않음 |
| `Ready` | 설계와 선행 조건이 준비됐지만 현재 단일 Active 작업은 아닌 상태 |
| `Active` | 현재 Plan에서 진행 중 |
| `Paused` | 진행 체크포인트를 보존한 채 다른 주력 작업으로 일시중지 |
| `Blocked` | 선행 조건 때문에 막힘 |
| `Done` | 구현 완료. Systems 문서로 승격됨 |
| `Deferred` | 보류 |
| `Rejected` | 기각 |

---

## 4. 우선순위 표기

| 우선순위 | 의미 | 처리 기준 |
|---|---|---|
| `P0` | 없으면 다음 개발/검증이 막힘 | 최우선 착수 후보 |
| `P1` | 핵심 게임 루프에 필요 | 가까운 사이클에서 착수 |
| `P2` | 생산성/운영/품질 개선 | 반복 비용이 커질 때 착수 |
| `P3` | 편의/폴리싱 | 핵심 루프 이후 |
| `Icebox` | 아이디어 보관 | 당장 계획하지 않음 |

---

## 5. 기능 후보 큐

| ID | 기능 | 목적 | 우선순위 | 상태 | 클라 | 서버 | 관리툴 | 완료 후 Systems 위치 |
|---|---|---|---|---|---|---|---|---|
| `CF-FQ-011` | 싱글 실행 기준선 전환 | 서버 GameMode 전제 없이 PIE 1인 플레이에서 기준 차량 1대를 바로 조작 가능하게 만들기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Config/ProjectRuntimeConfig.md`, `Document/Systems/Vehicles/VehicleRuntime.md` 갱신 |
| `CF-FQ-012` | 1대 차량 주행감 고도화 | `DA_PoliceCar` 기준 전진/후진/조향/브레이크/핸드브레이크 감각을 전투 루프 안에서 조정 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleDrive.md`, `Document/Systems/Vehicles/VehicleSteering.md` 갱신 |
| `CF-FQ-013` | 카메라/로컬 조준 고도화 | 서버 판정 없이 차량 카메라, Local Aim, Reticle 피드백을 싱글 기준으로 정리 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleCamera.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-014` | WheelSync 시각 품질 폴리싱 | 고속 휠 스핀/조향/서스펜션 시각 품질을 기능 FAIL과 품질 후속으로 분리하고 개선 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/WheelSync.md` 갱신 |
| `CF-FQ-015` | 차량 데이터 튜닝 패스 | 기준 차량 1대의 Movement/Wheel/DriveState 값을 추적 가능한 데이터 기준으로 정리 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleData.md` 갱신 |
| `CF-FQ-016` | 차량 무기 조준 및 발사 | 기준 차량 1대에서 로컬 조준 상태를 실제 발사 경로로 연결하고 WeaponFire 결과를 기록 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/VehicleDebugPanel.md` 갱신 |
| `CF-FQ-017` | Reticle / FireFeedback UI 구현 | WeaponFire 결과를 Reticle 색상, 상태 문구와 쿨다운 UI로 읽히게 만들기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/FireFeedback.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-018` | 피격 판정 및 피해 처리 | 시각 차체 기반 피격 콜리전 구현 결과를 유지하면서 신뢰 가능한 HitContext를 실제 피해 누적으로 연결하기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/HitDamage.md` |
| `CF-FQ-022` | 조준점·터렛·총구 정렬 | Reticle 월드 목표점과 실제 발사 해를 통합하고, `TurretMountData`별 정렬 중 발사 허용 정책을 선택 가능하게 만들기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/UI/AimReticle.md` 갱신 |
| `CF-FQ-023` | 고속 Projectile 연속 충돌 | Sweep/Sub-stepping과 보조 Sphere Sweep으로 고속 발사체 터널링을 방지하고 신뢰 가능한 HitContext를 보장하기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/Projectile.md`, `Document/Systems/Combat/DamageHitContext.md` 갱신 |
| `CF-FQ-025` | 이중 레티클 및 터렛방향 시각화 | Image_CenterDot 조준 레티클과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클을 분리하고 탄종·착탄 위치와 독립적으로 표시 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/UI/AimReticle.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/VehicleDebugPanel.md` 갱신 |
| `CF-FQ-026` | 타겟 선택 시스템 | 직접 조준 우선과 크로스헤어 근접 후보를 기반으로 지속 선택 대상을 만들고 HUD·센서·유틸리티 장비가 공통으로 조회할 기반을 구현 | `P1` | `Paused` | 필요 | 불필요 | 불필요 | `Document/Systems/Targeting/TargetSelect.md`, 관련 UI·장비 Systems 문서 갱신 |
| `CF-FQ-024` | 전투 FX 구현 | 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 Niagara 기반 시각 연출로 1회씩 표현하기 | `P0` | `Done` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/CombatFx.md`, 기존 Combat Systems 문서 갱신 |
| `CF-FQ-019` | 주행/전투 반복 테스트 | 주행 중 조준/발사/피격/피해와 시각 FX가 반복되는지 PIE 기준으로 검증 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/ProjectSSOT/05_TestChecklist.md`, `Document/Systems/Combat/CoreLoop.md` 갱신 |
| `CF-FQ-020` | 조작감/전투 템포/피드백 개선 | 조작감, 발사 리듬, 피격 반응, UI와 시각 피드백을 핵심 루프 기준으로 조정 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/CombatFeel.md` |
| `CF-FQ-021` | 핵심 게임 루프 검증 | 싱글 차량 전투 루프가 다음 개발 단계로 넘어갈 수 있는지 PASS/FAIL 판정 | `P1` | `Candidate` | 필요 | 불필요 | 불필요 | `Document/Systems/Combat/CoreLoop.md`, `Document/ProjectSSOT/05_TestChecklist.md` 갱신 |
| `CF-FQ-001` | 서버 권한 발사 요청 | 2클라 환경에서 발사 요청을 서버 권한 구조로 통과시키기 | `Icebox` | `Deferred` | 필요 | 필요 | 불필요 | `Document/Systems/Network/ServerFire.md` 또는 `Document/Systems/Combat/Fire.md` |
| `CF-FQ-002` | 조준/발사 피드백 분리 | 서버 판정과 로컬 조준/이펙트/Reticle 피드백 책임 분리 | `Icebox` | `Deferred` | 필요 | 필요 | 불필요 | 현재는 `CF-FQ-017`의 로컬 FireFeedback으로 대체 |
| `CF-FQ-003` | 체력/대미지 최소 구조 | 장기 전투 확장을 위한 체력/생존 상태 기반. 현재 최소 피해 처리는 `CF-FQ-018`에서 먼저 다룸 | `P2` | `Deferred` | 필요 | 불필요 | 낮음 | `Document/Systems/Combat/Damage.md` |
| `CF-FQ-004` | 리스폰 최소 구조 | 2클라 전투 테스트 반복 가능 상태 만들기 | `Icebox` | `Deferred` | 필요 | 필요 | 낮음 | `Document/Systems/Network/Respawn.md` |
| `CF-FQ-005` | 전투 결과 기록 | 매치 종료/승패/기본 결과 기록 기반 만들기 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Combat/MatchResult.md` |
| `CF-FQ-006` | 테스트 계정/상태 초기화 도구 | 반복 테스트 준비 비용 줄이기 | `Icebox` | `Deferred` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/TestReset.md` |
| `CF-FQ-007` | 차량 로드아웃 저장 | 차량/무장 장착 상태를 재접속 후 유지 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Data/VehicleLoadout.md` |
| `CF-FQ-008` | 무장 데이터 정의 | 차량 장착형 터렛/무기 데이터 기준 만들기 | `P2` | `Candidate` | 필요 | 불필요 | 낮음 | `Document/Systems/Data/WeaponData.md` |
| `CF-FQ-009` | 운영 로그 조회 기준 | 서버 전투/스폰/에러 로그를 추적 가능한 형태로 정리 | `Icebox` | `Deferred` | 불필요 | 필요 | 필요 | `Document/Systems/Admin/LogView.md` |
| `CF-FQ-010` | 세션/로비 기초 | Dedicated Server 이후 접속 흐름 확장 | `Icebox` | `Deferred` | 필요 | 필요 | 중간 | `Document/Systems/Network/Session.md` |

---

## 6. 현재 최우선 착수 후보

현재 자동 선택된 Active 기능은 없다.

```text
1. CF-FQ-019 주행/전투 반복 테스트: Candidate / 착수 가능
2. CF-FQ-026 타겟 선택 시스템: Paused / TS-P0-08 재개 가능
3. CF-FQ-020 조작감/전투 템포/피드백 개선: Candidate
4. CF-FQ-021 핵심 게임 루프 검증: Candidate
```

판단 근거:

```text
- CF-FQ-024 전투 FX가 Done / User PIE PASS / CF-TC-021 PASS로 완료됐다.
- Aim, Fire, Projectile, Damage, Reticle과 CombatFx를 포함한 반복 전투 회귀의 선행 조건이 확보됐다.
- 따라서 기존 순서상 CF-FQ-019가 다음 우선 Candidate다.
- CF-FQ-019는 사용자의 명시적 선택 전에는 Active로 전환하지 않는다.
- CF-FQ-026은 TS-P0-00~07 Done과 Automation 7/7 PASS를 유지한 채 TS-P0-08에서 Paused다.
- CarFight는 프로젝트 전역에서 게임 사운드를 지원하지 않으며 Audio 기능을 별도 후보로 자동 등록하지 않는다.
```

---

## 6-1. Reticle / FireFeedback 현재 구현 및 완료 기준

`CF-FQ-017`의 현재 구현 판단과 완료 기록에는 아래 문서를 우선한다.

```text
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/UI/VehicleDebugPanel.md
```

현재 구현 상태:

```text
- WeaponFire는 판정/기록 담당이고, ACFVehiclePawn::BuildFireFeedbackViewData()가 UI용 표시 데이터를 만든다.
- UCFAimReticleWidget은 기본 Aim 상태와 FireFeedback ViewData를 합쳐 최종 Reticle 상태, 텍스트, 색상을 갱신한다.
- WBP_AimReticle에는 중앙점/4방향 브라켓, FireFeedback State/Hint, Cooldown, 전용 OutOfArc 경고가 연결되어 있다.
- FireSuccess는 짧게 성공 색상으로 표시된 뒤 Cooldown으로 전환되고, 쿨다운 종료 후 텍스트가 사라진다.
- OutOfArcWarning은 전용 보조 경고로 표시하며 주 Reticle 상태와 색상을 강제로 덮어쓰지 않는다.
- FirePending은 타입과 표시 문구만 유지하며 현재 동기 발사 구조에서는 사용하지 않는다.
```

최종 사용자 PIE 확인:

```text
- NoWeapon 회색 Reticle, "무기 없음"과 "사용 가능한 무기 없음" 표시 PASS
- AimBlocked 주황 Reticle, "조준 가림"과 "조준선이 막힘" 표시 PASS
- 두 실패 상태의 유지 시간 종료 후 FireFeedback 텍스트 제거 PASS
- 장애물 제거 후 정상 Aim 상태 복귀 PASS
- Ready → FireSuccess → Cooldown → Ready 정상 발사 회귀 PASS
- 판정: CF-FQ-017 Done / CF-TC-014 PASS
```

---

## 6-2. CF-FQ-018 피격/피해 구현 선행 기준

현재 기준 문서:

```text
- Document/Plan/HitDamage/ImplementationDesign.md
- Document/Systems/Combat/DamageHitContext.md
- Document/Systems/Combat/Projectile.md
- Document/Systems/Vehicles/VehicleCoreDecisions.md
```

현재 구현 및 확인 상태:

```text
- WeaponHit = ECC_GameTraceChannel1, Projectile = ECC_GameTraceChannel2로 구현됐다.
- VehicleVisualHit Collision Profile이 구현됐다.
- VehicleMesh는 기존 차량 물리를 유지하면서 WeaponHit / Projectile을 Ignore한다.
- SM_Body는 QueryOnly 상태에서 WeaponHit / Projectile을 Block한다.
- Dummy HitScan과 Projectile HitComponentName이 DamageHitContext에 기록된다.
- Unreal Editor 타깃 빌드가 성공했다.
- 일반 속도 HitScan / Projectile이 SM_Body에서 정상 충돌하는 것을 사용자 PIE에서 확인했다.
```

당시 확인했던 선행 문제와 해결 결과:

```text
- Reticle 월드 목표점과 실제 Muzzle 발사 해 불일치 → CF-FQ-022에서 해결 완료
- Camera Aim Trace와 WeaponHit 표면 기준 차이 → CF-FQ-022에서 단일 Aim Solution 계약으로 정리
- 터렛 추적 중 실제 Muzzle 방향과 요구 방향 차이 → bAllowFireWhileAligning 정책과 Weapon Aim Solution으로 해결
- 고속 Projectile 프레임 사이 터널링 → CF-FQ-023 연속 충돌로 해결 완료
```

현재 판정:

```text
- CF-FQ-022 Done / CF-TC-019 PASS
- CF-FQ-023 Done / CF-TC-020 PASS
- CF-FQ-018 Done / CF-TC-016 PASS
```

Damage Runtime 당시 착수 조건:

```text
- CF-FQ-022 조준점·터렛·총구 정렬 완료
- CF-FQ-023 고속 Projectile 연속 충돌 완료
- 일반 속도와 고속 Projectile 모두 신뢰 가능한 DamageHitContext 생성
- 이후 BaseDamage, 체력 감소, 파괴 상태 구현
```

---

## 6-3. CF-FQ-022 / CF-FQ-023 선행 작업 기준

### 조준점·터렛·총구 정렬

기준 문서:

```text
Document/Plan/AimFireAlignment/ImplementationDesign.md
```

확정 원칙:

```text
- Reticle이 지시하는 월드 위치를 DesiredAimTargetLocation SSOT로 사용한다.
- Camera Aim Trace와 실제 무기 Trace는 WeaponHit 응답 기준을 공유한다.
- 터렛은 Muzzle 위치에서 DesiredAimTargetLocation으로 향하는 요구 발사 방향을 추적한다.
- 현재 Muzzle 방향과 요구 방향의 정렬 오차를 계산한다.
- `UCFTurretMountData.bAllowFireWhileAligning`으로 터렛별 정렬 중 발사 허용 여부를 선택한다.
- 기본값은 `true`이며, true인 터렛은 정렬 중 현재 Muzzle 방향으로 발사하고 false인 터렛은 정렬 완료 전 발사를 거부한다.
- 정렬 완료 후에는 Muzzle → Reticle 목표 방향을 실제 발사 방향으로 사용한다.
- 총구 앞 장애물은 실제 최종 발사 방향 기준 WeaponHit Trace로 검사하며, `MuzzleBlocked`는 정책값과 관계없이 발사를 거부한다.
- OutOfArc와 TurretAligning은 서로 다른 상태로 관리한다.
- Core/Presentation C++, Reticle Recovery Hotfix와 터렛별 정렬 중 발사 정책 구현을 완료했다.
- `BuildEditor.bat` 공식 빌드와 사용자 PIE에서 정책 true/false, 정렬 완료 탄착, `MuzzleBlocked`를 확인했다.
- 현재 판정은 `CF-FQ-022` Done / `CF-TC-019` PASS다.
```

### 고속 Projectile 연속 충돌

기준 문서:

```text
Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md
```

확정 원칙:

```text
- ProjectileMovement Sweep을 ProjectileData 값으로 명시적으로 적용한다.
- 고속/중력 Projectile은 Sub-stepping과 MaxSimulationTimeStep / MaxSimulationIterations를 데이터에서 적용한다.
- P0 안전 기본값은 Sweep=true, Sub-step=true, MaxStep=0.008333, Iterations=8이다.
- 잔여 터널링은 Previous → Current 구간의 CollisionRadius 기반 Sphere Sweep으로 차단한다.
- OnComponentHit과 보조 Sweep은 ResolveProjectileImpact()와 bImpactResolvedThisActivation을 공유한다.
- Pool 재활성화마다 PreviousCollisionLocation과 Impact 처리 상태를 초기화한다.
- Projectile 자신, InstigatorActor, OwnerActor Ignore를 기존 이동과 보조 Sweep 모두에서 유지한다.
- CCD는 주 해결책이 아니라 탄종별 보조 안전장치이며 기본값은 false다.
- 초고속 탄종은 실제 판정을 HitScan으로 분리하고 Projectile은 Tracer로 사용할 수 있다.
- 2026-07-14 기준 연속 충돌 C++ 구현과 사용자 실행 Tools\\BuildEditor.bat 공식 Editor 빌드는 완료됐다.
- 사용자 PIE에서 일반 속도 Projectile 피격, 피격 Actor `BP_CFVehiclePawn_C_1`, 피격 컴포넌트 `SM_Body` 기록을 확인했다.
- HitComponentName 데이터 전달 경로를 Debug Panel의 `피격 컴포넌트` 독립 항목으로 노출했다.
- 사용자 PIE에서 `30 FPS + 기준 InitialSpeed 4배` 차량 집중 발사 테스트를 통과했다.
- 얇은 벽 앞 차량 배치에서 벽 관통 없음, 첫 Blocking Hit, 중복 Impact 없음과 Pool 재사용 정상 동작을 확인했다.
- 위 집중 스트레스 범위를 CF-FQ-023의 P0 완료 기준으로 적용해 상태를 Done으로 전환했다.
- 60 / 120 FPS 전체 조합과 이동 차량 검증은 CF-FQ-019 확장 회귀 범위로 유지한다.
```

---

## 6-4. CF-FQ-024 전투 FX 완료 기준

현재 구현 문서:

```text
Document/Systems/Combat/CombatFx.md
```

완료 Plan:

```text
Document/Plan/CombatFxAudio/ImplementationDesign.md
```

경로의 `CombatFxAudio`는 기존 디렉터리 호환을 위한 레거시 이름이며 현재 구현은 시각 FX만 다룬다.

완료 구현:

```text
- UCFCombatFxData와 UCFCombatFxComp 데이터 기반 런타임
- UCFWeaponData.DefaultFireFxData
- UCFProjectileData.DefaultImpactFxData
- UCFVehicleData.DefaultDestroyedFxData / DestroyedFxSocketName
- 승인된 발사의 실제 Muzzle FX 1회
- HitScan / Projectile 첫 ImpactLocation의 FX 1회
- 최초 Destroyed 전환의 차량별 SM_Body.FX_Destroyed 소켓 FX 1회
- 발사 거부, 중복 Impact와 추가 피해의 FX 중복 방지
- MaximumLifetimeSeconds Loop 잔류 안전 퓨즈
- ACFCombatFxPreviewActor EditorOnly 튜닝 도구
- Impact P0 NS_BasicHit 현재 크기 승인
- FX 자산 미연결 또는 생성 실패에서도 기존 판정 유지
```

최종 검증:

```text
- 공식 Admin Editor 빌드 3건 PASS
- 최신 사용자 직접 Editor 빌드 PASS
- Muzzle 정상 발사 1회 / 발사 거부 0회 PASS
- Impact 실제 위치 / 첫 1회 / 중복·잔류 없음 PASS
- Destroyed FX_Destroyed 위치 / 최초 1회 / 추가 피해 중복·잔류 없음 PASS
- 기존 조준·발사·피격·피해·파괴 회귀 PASS
- 오디오 클래스·모듈·에셋 참조 0개 유지
- CF-TC-021 PASS
- CF-FQ-024 Done
```

프로젝트 전역 제외 범위:

```text
- 모든 게임 사운드와 오디오 런타임
- 물리 표면별 Impact 세분화
- 지속형 Projectile Trail
- 완성형 Geometry Collection 파괴
- 서버 복제와 원격 클라이언트 FX 동기화
- 모든 외부 Niagara에 공통 적용되는 범용 Scale 보장
```

---

## 6-5. CF-FQ-025 이중 레티클 및 사격방향 시각화 기준

대표 Plan:

```text
Document/Plan/ReticleAimDirection/ImplementationDesign.md
```

완료 범위:

```text
- Image_CenterDot을 사용자 조준 레티클로 유지
- FCFVehicleWeaponAimSolution.CurrentMuzzleDirection을 터렛 레티클 방향 기준으로 사용
- 사용자 조준점과 같은 비교 거리의 TurretReticleWorldLocation을 Image_WeaponReticle로 화면 투영
- HitScan/Projectile과 중력 여부에 독립적인 단일 터렛 레티클 계산 적용
- 기존 DirectImpact/LaunchDirection Preview는 UI 소비에서 제외하고 Legacy Debug로만 보존
- 정렬 중 발사 허용/금지, MuzzleBlocked와 기존 FireFeedback 회귀 검증
```

현재 제외 범위:

```text
- 중력 Projectile Ballistic Solver
- 투사체 착탄 위치의 월드 공간 3D 표시
- 이동 목표 Lead Indicator
- 자동 락온과 Aim Assist
- 다중 터렛 동시 Reticle
- 네트워크 지연 보정
- 완성형 전투 HUD 전체 재설계
```

완료 기준:

```text
- Tools\BuildEditor.bat PASS
- Image_CenterDot 기존 조준 동작 유지
- Image_WeaponReticle이 CurrentMuzzleDirection 기반 터렛 조준 지점을 표시
- 정렬 중 두 레티클 분리와 정렬 완료 수렴
- HitScan/Projectile 및 중력 여부에서 동일한 터렛 레티클 의미 유지
- MuzzleBlocked 발사 차단과 터렛 레티클 위치 책임 분리
- Ready / FireSuccess / Cooldown / NoWeapon / AimBlocked 회귀 PASS
- CF-FQ-025 사용자 싱글 PIE PASS
```

---

## 6-6. CF-FQ-026 타겟 선택 시스템 구현 기준

기준 문서:

```text
- Document/Design/TargetSelect.md
- Document/Plan/TargetSelectPlan.md
- Document/Plan/TargetSelectRoadmap.md
- Document/Plan/TargetSelectWorkOrder.md
```

현재 구현 상태:

```text
- TS-P0-00 구조 조사와 기존 차량·카메라·입력·충돌 경계 복구 완료
- TS-P0-01 핵심 상태·인터페이스·설정 DataAsset·Pawn 기본 통합 완료
- TS-P0-02 TargetPoint → SM_Body Bounds → Actor 위치 Fallback 완료
- TS-P0-03 직접 조준 우선·화면 근접도·거리·안정 키 후보 정렬 완료
- TS-P0-04 파괴·EndPlay·가림 유예·추적 거리 선택 수명 완료
- TS-P0-05 IA_SelectTarget / IA_ClearTarget과 IMC_Vehicle_Default 입력 연결 완료
- TS-P0-06 UCFTargetSelectWidget / WBP_TargetSelect 후보·선택 HUD 완료
- TS-P0-07 TargetUsePolicy와 VehicleWeaponComp 읽기 전용 장비 평가 연동 완료
- Build Job 0fce6d253d9548dfa0ed39c94501ff47에서 Editor 빌드와 TargetSelect Automation 7/7 PASS
- TS-P0-08 사용자 PIE 통합 검증과 튜닝은 Paused
```

현재 체크포인트:

```text
재개 위치: TS-P0-08 P0 통합 검증과 튜닝
첫 결함: 후보 텍스트와 대상 겹침, 후보 범위 과대, 디버그 원 비가시
보존 증거: TS-P0-00~07 Done / TargetSelect Automation 7/7 PASS
다음 조치: 단일 대상 범위 검증 → 다중 후보 히스테리시스 분리 → HUD·디버그·성능 튜닝
```

P0 완료 전 제외 범위:

```text
- 자동 조준과 선택 대상 방향으로 직접 조준 무기 보정
- 다중 타겟과 다중 락온
- 센서 기반 상세 정보 공개
- 장비별 락온·해킹·견인 획득 로직
- 부위 선택
- 서버 복제와 원격 클라이언트 동기화
```

---

## 7. 기능을 Plan으로 승격하는 기준

기능을 `Document/Plan/<기능명>/`로 승격하려면 아래 조건을 만족해야 한다.

```text
- 목적이 한 문장으로 설명된다.
- 클라이언트 / 서버 / 관리툴 필요성이 1차로 판단됐다.
- 완료 후 Systems 문서 위치가 정해졌다.
- 이번 범위에서 제외할 항목이 정해졌다.
- 선행 Systems 문서가 무엇인지 확인됐다.
```

승격 시 `Plan` 폴더에는 아래 문서 구성을 권장한다.

```text
Document/Plan/<기능명>/README.md
Document/Plan/<기능명>/Scope.md
Document/Plan/<기능명>/CurrentState.md
Document/Plan/<기능명>/Design.md
Document/Plan/<기능명>/TaskList.md
Document/Plan/<기능명>/TestPlan.md
Document/Plan/<기능명>/DecisionLog.md
```

단, 기능 규모가 작으면 `README.md`, `TaskList.md`, `TestPlan.md`만으로 줄일 수 있다.

---

## 8. 관리툴 후보 큐

관리툴은 현재 싱글 차량 고도화 범위에서 제외한다.
아래 항목은 장기 서버/운영 재개 시 다시 검토할 보류 후보로만 유지한다.

| ID | 후보 | 발생 조건 | 임시 대체 | 정식화 시점 | 상태 |
|---|---|---|---|---|---|
| `CF-ADM-001` | 테스트 계정/상태 초기화 | 같은 테스트 세팅을 3회 이상 반복 | 콘솔 명령 / 임시 서버 명령 | 계정/로드아웃/인벤토리 도입 후 | `Candidate` |
| `CF-ADM-002` | 유저 차량 상태 조회 | 차량 소유/로드아웃 저장 도입 | 로그 / DB 직접 조회 | 로드아웃 서버 저장 후 | `Candidate` |
| `CF-ADM-003` | 보상 지급/회수 | 전투 결과와 보상 도입 | 수동 DB 수정 | MatchResult/RewardLog 도입 후 | `Candidate` |
| `CF-ADM-004` | 전투 로그 조회 | 서버 판정 전투 도입 | 로그 파일 검색 | MatchResult 저장 후 | `Candidate` |
| `CF-ADM-005` | 서버 상태 확인 | 장시간 Dedicated Server 테스트 반복 | 콘솔 로그 확인 | 테스트 서버 상시 운용 전 | `Candidate` |

현재 관리툴 원칙:

```text
1. 이번 싱글 차량 고도화 사이클에서는 관리툴을 만들지 않는다.
2. 서버/운영 기능이 재개될 때만 콘솔 명령 / CLI / 임시 서버 명령부터 검토한다.
3. 데이터 구조가 안정화된 뒤 정식 관리툴로 승격한다.
4. 관리툴 기능도 완료되면 Document/Systems/Admin/ 아래에 기록한다.
```

---

## 9. 기능 큐 갱신 조건

아래 상황이 발생하면 이 문서를 갱신한다.

```text
- 새 기능 후보가 생김
- 기능 우선순위가 바뀜
- 기능이 Plan으로 승격됨
- 기능이 완료되어 Systems로 승격됨
- 관리툴 후보가 생김
- 기능이 보류/기각됨
```

---

## 10. 문서 버전 관리

- 현재 문서 버전: `v1.18.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 기능 큐 운영 방식 자체 변경 |
| Minor | 기능 후보/관리툴 후보/승격 기준 추가 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 11. 체인지로그

### v1.18.0 - 2026-07-27

```text
- 최종 사용자 PIE 전체 PASS를 반영해 CF-FQ-024를 Active에서 Done으로 전환했다.
- CF-TC-021 PASS와 Document/Systems/Combat/CombatFx.md Current System 승격을 등록했다.
- NS_BasicHit Impact 현재 크기 승인과 차량별 SM_Body.FX_Destroyed 위치 완료를 기록했다.
- 현재 Active 기능을 비우고 CF-FQ-019를 착수 가능한 다음 Candidate로 재정렬했다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지했다.
```

### v1.17.0 - 2026-07-24

```text
- 사용자 결정에 따라 CF-FQ-024 전투 FX를 Ready에서 Active로 전환했다.
- CF-FQ-026 타겟 선택 시스템은 TS-P0-08 체크포인트를 보존한 Paused 상태로 변경했다.
- 상태 표기에 Paused를 추가하고 기능 큐와 현재 최우선 순서를 갱신했다.
- 현재 FX 단계를 FAB 콘텐츠 조사와 Muzzle·Impact·Destroyed 후보 선별 Phase 0으로 기록했다.
- TargetSelect TS-P0-00~07 완료 상태와 Automation 7/7 PASS는 회귀 보호 기준으로 유지했다.
```

### v1.16.0 - 2026-07-24

```text
- 사용자 결정에 따라 CarFight 프로젝트 전역에서 게임 사운드를 지원하지 않는 정책을 확정했다.
- CF-FQ-024를 전투 FX 및 사운드 구현에서 전투 FX 전용 구현으로 변경했다.
- 대표 Plan을 Document/Plan/CombatFxAudio/ImplementationDesign.md로 유지하되 시각 FX 전용으로 해석했다.
- CF-FQ-019와 CF-FQ-020의 Audio·사운드 완료 조건을 시각 FX와 UI 피드백 기준으로 정리했다.
- SoundWave, SoundCue, MetaSound, Sound Attenuation, USoundBase, UAudioComponent와 오디오 모듈 도입을 프로젝트 범위에서 제외했다.
```

### v1.15.0 - 2026-07-23

```text
- CF-FQ-026 타겟 선택 시스템을 P1 Active 기능으로 등록했다.
- TS-P0-01 C++ 계약과 Editor 빌드 PASS, Blueprint·PIE Pending 상태를 현재 구현 체크포인트로 기록했다.
- CF-FQ-024 전투 FX 및 사운드 구현은 취소하지 않고 Ready로 전환했다.
- 현재 최우선 순서를 CF-FQ-026 → CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021로 변경했다.
- TargetSelect 기획·Plan·로드맵·작업지시 규격과 P0 제외 범위를 FeatureQueue에 연결했다.
```

### v1.14.0 - 2026-07-21

```text
- CF-FQ-025를 CurrentMuzzleDirection 기반 탄종 독립 터렛 레티클 기능으로 정정했다.
- 공식 에디터 빌드와 사용자 PIE PASS를 근거로 CF-FQ-025를 Done으로 전환했다.
- CF-FQ-024를 현재 구현순위 1위로 복원했다.
- 투사체 착탄 위치 3D 표시는 별도 후속 Feature 논의 대상으로 분리했다.
```

### v1.13.0 - 2026-07-16

```text
- CF-FQ-025 이중 레티클 및 사격방향 시각화를 P0 Active 기능으로 공식 등록했다.
- 현재 구현순위를 CF-FQ-025 → CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021 순서로 변경했다.
- CF-FQ-024는 취소하지 않고 구현순위 2위 Active 작업으로 유지했다.
- ReticleAimDirection 대표 Plan, 첫 구현 범위, 제외 범위와 사용자 PIE 완료 기준을 등록했다.
- 실제 사격 방향이 명확해진 뒤 전투 FX와 사운드를 연결하도록 개발 순서를 조정했다.
```

### v1.12.0 - 2026-07-15

```text
- CF-FQ-024 전투 FX 및 사운드 구현을 P0 Active 기능으로 추가했다.
- 완료된 CF-FQ-017의 범위를 Reticle / FireFeedback UI로 명확히 분리했다.
- 현재 우선순위를 CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021 순서로 갱신했다.
- CombatFxAudio 대표 Plan과 P0 완료 기준을 등록했다.
- CF-FQ-022의 오래된 PIE Pending 문구를 현재 Done / PASS 상태로 정정했다.
```

### v1.11.0 - 2026-07-15

```text
- CF-FQ-017 NoWeapon, AimBlocked와 정상 발사 회귀 사용자 PIE PASS를 기록했다.
- CF-FQ-017 상태를 Active에서 Done으로 변경했다.
- CF-TC-014 발사 피드백/UI PASS와 관련 Systems 현재 상태를 반영했다.
- 다음 후보 CF-FQ-019는 Candidate로 유지하고 자동 착수하지 않았다.
```

### v1.10.0 - 2026-07-15

```text
- CF-FQ-018 최소 Damage Runtime 사용자 PIE PASS를 기록했다.
- CF-FQ-018 상태를 Active에서 Done으로 변경했다.
- 현재 구현 기준을 Document/Systems/Combat/HitDamage.md로 승격했다.
- 최우선 착수 후보를 CF-FQ-017 최종 상태 검증과 CF-FQ-019 반복 회귀 순서로 갱신했다.
```

### v1.9.6 - 2026-07-14

```text
- CF-FQ-022 사용자 PIE에서 정렬 중 발사 정책 true/false, 정렬 완료 탄착과 MuzzleBlocked를 PASS 처리했다.
- CF-FQ-022 상태를 Active에서 Done으로 변경했다.
- CF-FQ-018 Damage Runtime의 사전 조건이 모두 완료된 것으로 정리했다.
- 주행·거리별 정량 조준 검증은 CF-FQ-019 확장 회귀로 이관했다.
```

### v1.9.5 - 2026-07-14

```text
- 사용자 PIE 축약 스트레스 테스트 통과를 기록했다.
- 30 FPS + 기준 속도 4배 차량 집중 발사, 얇은 벽 첫 Blocking Hit, 중복 Impact와 Pool 재사용 검증을 PASS 처리했다.
- CF-FQ-023 상태를 Active에서 Done으로 변경했다.
- 전체 FPS·속도 매트릭스와 이동 차량 검증은 CF-FQ-019 확장 회귀로 이관했다.
- CF-FQ-018의 남은 직접 선행 조건을 CF-FQ-022 사용자 PIE 완료로 정리했다.
```

### v1.9.4 - 2026-07-14

```text
- 사용자 PIE에서 피격 Actor BP_CFVehiclePawn_C_1과 피격 컴포넌트 SM_Body 기대값 출력을 확인했다.
- CF-FQ-023의 일반 속도 Visual HitContext 경로를 PIE PASS로 기록했다.
- CF-FQ-023은 Active 상태를 유지하고 고속·저프레임·얇은 충돌체 검증만 Pending으로 남겼다.
- 고속 PIE 전체 확인 전에는 Done 또는 전체 PASS로 판정하지 않는 기준을 유지했다.
```

### v1.9.3 - 2026-07-14

```text
- 사용자 실행 Tools\\BuildEditor.bat에서 피격 컴포넌트 독립 표시 보강분 공식 Editor 빌드 성공을 기록했다.
- CF-FQ-023은 Active 상태를 유지하고 피격 컴포넌트 SM_Body 확인과 고속 PIE만 Pending으로 남겼다.
- 사용자 PIE 전체 확인 전에는 Done 또는 PASS로 판정하지 않는 기준을 유지했다.
```

### v1.9.2 - 2026-07-14

```text
- 사용자 PIE에서 일반 속도 Projectile 피격과 피격 Actor BP_CFVehiclePawn_C_1 기록을 확인했다.
- HitComponentName 데이터 경로는 존재하지만 Debug Panel에 독립 표시가 없던 상태를 확인했다.
- VehicleDebug Panel의 Damage HitContext에 피격 컴포넌트 독립 표시 행을 추가했다.
- 표시 보강분 공식 빌드, SM_Body 재확인과 고속 PIE는 Pending으로 유지했다.
```

### v1.9.1 - 2026-07-14

```text
- 사용자 실행 Tools\\BuildEditor.bat에서 CF-FQ-023 공식 Editor 빌드 성공을 기록했다.
- CF-FQ-023은 Active 상태를 유지하고 사용자 PIE 고속 검증만 Pending으로 남겼다.
- 사용자 PIE 확인 전에는 Done 또는 PASS로 판정하지 않는 기준을 유지했다.
```

### v1.9.0 - 2026-07-14

```text
- CF-FQ-023 상태를 Ready에서 Active로 변경했다.
- UCFProjectileData 연속 충돌 설정과 ACFProjectileActor 보조 Sphere Sweep C++ 구현 완료 범위를 기록했다.
- Sweep=true, Sub-step=true, MaxStep=0.008333, Iterations=8, SupplementalSweep=true, CCD=false 안전 기본값을 기록했다.
- OnComponentHit과 보조 Sweep의 ResolveProjectileImpact 통합, 활성화별 중복 방지, Pool 상태 초기화를 기록했다.
- 기존 Projectile 채널, 시각 차체 충돌, HitComponentName, DamageHitContext 전달 경로 유지 상태를 기록했다.
- Tools\\BuildEditor.bat 공식 빌드와 사용자 PIE 검증은 Pending으로 유지했다.
- CF-FQ-022 정렬 중 발사 정책의 코드·빌드 완료와 PIE Pending 상태가 충돌하지 않도록 문장을 정정했다.
```

### v1.8.1 - 2026-07-14

```text
- CF-FQ-022 Align Fire Policy C++ 구현과 `BuildEditor.bat` 성공을 기록했다.
- CF-FQ-022는 사용자 PIE 검증 전이므로 Active / PIE Pending을 유지했다.
- WBP와 DataAsset 인스턴스는 이번 작업에서 변경하지 않았다.
```

### v1.8.0 - 2026-07-14

```text
- CF-FQ-022에 `UCFTurretMountData.bAllowFireWhileAligning` 터렛별 정책을 추가했다.
- 기본값 true, 정렬 중 허용 시 현재 Muzzle 방향 발사, 비허용 시 정렬 완료 전 거부 기준을 기록했다.
- MuzzleBlocked는 항상 거부하고 TurretAligning Reticle은 발사 허용 여부와 분리해 유지하도록 기록했다.
- 설계와 Codex 작업지시서 작성은 완료됐지만 코드/빌드/PIE는 Pending으로 유지했다.
```

### v1.7.1 - 2026-07-13

```text
- Reticle Recovery Hotfix 코드 완료와 BuildEditor.bat 성공 상태 기록
- 에디터 종료 후 UnrealEditor-CarFight_Re.dll 링크가 성공했음을 기록
- CF-FQ-022는 Active / PIE Pending 유지
```

### v1.7.0 - 2026-07-13

```text
- CF-FQ-022 상태를 Ready에서 Active로 변경
- AimFireAlignment Core/Presentation C++ 빌드 완료 상태와 PIE Pending 상태를 분리 기록
- TurretAligning / WeaponNotAligned / MuzzleBlocked 표시 연결이 완료됐지만 PIE PASS는 아직 기록하지 않음
- 실제 피해 처리와 고속 Projectile 작업은 CF-FQ-022 PIE 검증 이후 진행하도록 우선순위 설명 갱신
```

### v1.6.0 - 2026-07-13

```text
- CF-FQ-018을 Ready에서 Active로 변경하고 시각 차체 기반 피격 구현/PIE 확인 결과 반영
- CF-FQ-022 조준점·터렛·총구 정렬 P0 Ready 기능 추가
- CF-FQ-023 고속 Projectile 연속 충돌 P0 Ready 기능 추가
- Reticle 목표와 실제 Muzzle 발사 방향 불일치, 고속 Projectile 터널링을 Damage Runtime 선행 문제로 등록
- Document/Plan/AimFireAlignment/ImplementationDesign.md와 Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md를 기준 문서로 연결
- 실제 피해 처리보다 두 사격 신뢰성 작업을 먼저 수행하도록 우선순위 재정렬
```

### v1.5.0 - 2026-07-13

```text
- CF-FQ-018을 Candidate에서 Ready로 변경
- 현재 차량 무기 피격이 VehicleMesh Physics Asset 기준이고 SM_Body는 NoCollision이라는 확인 결과 반영
- 실제 피해 처리 전에 VehicleMesh 물리 충돌과 SM_Body 무기 피격 Query를 분리한다는 선행 조건 추가
- Document/Plan/HitDamage/ImplementationDesign.md를 CF-FQ-018 현재 설계 기준으로 등록
- Collision Channel/Profile 이름과 차량별 StaticMesh Collision 상태를 코드 작업 전 미확정 항목으로 분리
```

### v1.4.0 - 2026-07-13

```text
- CF-FQ-017 발사 피드백 구현 상태를 Ready에서 Active로 변경
- Reticle 이미지/텍스트 상태별 색상과 FireSuccess → Cooldown → 종료 흐름의 구현 및 PIE 확인 결과 반영
- 전용 OutOfArc 경고의 중복 방지, 종료 조건, 다른 FireFeedback 비가림 조건 확인 결과 반영
- NoWeapon / AimBlocked 실제 PIE 검증을 Done 전 남은 조건으로 명시
- 다음 우선순위를 CF-FQ-017 최종 검증 후 CF-FQ-018 착수 순서로 갱신
```

### v1.3.0 - 2026-07-09

```text
- CF-FQ-016 차량 무기 조준 및 발사를 Done으로 조정
- CF-FQ-017 발사 피드백 구현을 Ready로 조정
- 현재 최우선 착수 후보를 FireFeedback / Reticle 구현 기준으로 재정렬
- FireFeedback.md 신규 Systems 문서 생성 상태를 완료 후 Systems 위치와 착수 기준에 반영
- CF-FQ-002 조준/발사 피드백 분리의 현재 대체 기준을 CF-FQ-017 로컬 FireFeedback으로 갱신
- Reticle / FireFeedback 착수 기준 섹션을 추가
- OutOfArc / FirePending / FireRejected / Cooldown / NoWeapon 관련 현재 기준을 기능 큐에 반영
```

### v1.2.0 - 2026-06-19

```text
- CF-FQ-016 ~ CF-FQ-021 싱글 로컬 전투 루프 개발 후보를 추가
- 현재 최우선 착수 후보를 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증 순서로 재정렬
- CF-FQ-011 싱글 실행 기준선 전환과 CF-FQ-013 카메라/로컬 조준 고도화를 완료 기반으로 표시
- 체력/대미지 장기 확장 항목은 유지하되, 최소 피해 처리는 CF-FQ-018에서 먼저 다루도록 분리
```

### v1.1.0 - 2026-06-18

```text
- 기능 후보 큐의 P0를 서버 권한 발사 요청에서 싱글 실행 기준선 / 1대 차량 주행감 / 카메라·로컬 조준 고도화로 변경
- 서버 권한 발사, 리스폰, 세션, 서버 로그, 관리툴 후보를 Deferred 또는 Icebox로 조정
- CF-FQ-011 ~ CF-FQ-015 싱글 차량 고도화 후보 추가
- 관리툴 후보 큐를 현재 범위 밖 장기 보류 후보로 재해석
```

### v1.0.0 - 2026-06-02

```text
- FeatureQueue 문서 최초 작성
- 기능 후보 큐 작성
- Plan 승격 기준 정의
- Systems 승격 위치 개념 정의
- 관리툴 후보 큐 추가
```

---

## 12. Migration

### v1.18.0 적용 안내

```text
- CF-FQ-024는 Done / User PIE PASS / CF-TC-021 PASS다.
- 현재 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- CombatFxAudio Plan은 완료 이력이며 Active 구현 문서로 사용하지 않는다.
- 현재 Active 기능은 없고 CF-FQ-019는 사용자가 선택할 때만 Active로 전환한다.
- CF-FQ-026은 TS-P0-08 Paused 상태를 유지한다.
```

### v1.17.0 적용 안내

```text
- 새 세션은 CF-FQ-024와 Document/Plan/CombatFxAudio/ImplementationDesign.md를 우선 복원한다.
- 첫 실행은 AssetPreparationChecklist.md 기준 FAB Niagara 자산 조사다.
- CF-FQ-026은 TS-P0-08 Paused이며 FX 작업 중 TargetSelect 코드·에셋·검증 결과를 변경하지 않는다.
- CF-FQ-024 완료 전에는 CombatFx를 Current System으로 승격하지 않는다.
- 게임 사운드 자산, 런타임, 모듈과 테스트 완료 조건을 추가하지 않는다.
```

### v1.16.0 적용 안내

```text
- CF-FQ-024는 동일 ID를 유지하지만 의미는 전투 FX 전용 기능이다.
- 신규 구현은 `Document/Plan/CombatFxAudio/ImplementationDesign.md`를 사용한다.
- `CombatFxAudio` 디렉터리명은 레거시 경로이며 문서 내용과 구현 범위는 시각 FX 전용으로 해석한다.
- 게임 사운드 자산, 런타임, 모듈과 테스트 완료 조건을 새 기능에 추가하지 않는다.
- Unreal의 기본 플랫폼 오디오 설정은 엔진 생성 설정으로 남길 수 있으나 게임 기능 구현으로 해석하지 않는다.
```

### v1.14.0 적용 안내

```text
- CF-FQ-025는 Done / User PIE PASS로 해석한다.
- Weapon Reticle은 실제 최종 AimDirection이나 착탄 Preview가 아니라 CurrentMuzzleDirection 기반 터렛 레티클이다.
- 기존 DirectImpact/LaunchDirection 기록은 Legacy 구현 이력으로만 해석한다.
- 신규 구현 우선순위는 CF-FQ-024부터 재개한다.
```

### v1.13.0 적용 안내

```text
- CF-FQ-025를 현재 P0 Active 구현순위 1위로 해석한다.
- 신규 구현 세션은 Document/Plan/ReticleAimDirection/ImplementationDesign.md를 우선 복원한다.
- CF-FQ-024는 취소하지 않고 P0 Active 구현순위 2위로 유지한다.
- CF-FQ-025 Phase 0~4와 사용자 PIE 완료 또는 사용자 명시적 전환 후 CF-FQ-024를 재개한다.
- CF-FQ-025 구현 완료 전에는 Weapon Reticle과 Weapon Preview를 Current System으로 해석하지 않는다.
- 기존 AimDirection, AimTargetLocation, FireFeedback와 MuzzleBlocked 판정 계약은 변경하지 않는다.
```

### v1.12.0 적용 안내

```text
- CF-FQ-017은 Reticle / FireFeedback UI 완료 기능으로 유지한다.
- 실제 Niagara와 공간 사운드 구현은 CF-FQ-024에서 진행한다.
- CF-FQ-019 반복 테스트는 CF-FQ-024 사용자 PIE 완료 뒤 착수한다.
- CF-FQ-024 구현 완료 전에는 CombatFxAudio를 Current System으로 해석하지 않는다.
```
