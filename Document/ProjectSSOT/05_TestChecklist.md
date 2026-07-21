# CarFight — 05_TestChecklist

> 문서 버전: v1.9.0
> 작성일(Asia/Seoul): 2026-07-15
> 문서 상태: Active
> 역할: CarFight의 **완료된 Systems 기준 최소 회귀 테스트**를 관리한다.

---

## 1. 목적

이 문서는 CarFight에서 이미 구현되어 `Document/Systems/`에 기록된 기능들의 최소 회귀 테스트를 관리한다.

이 문서는 진행 중 기능의 상세 테스트 계획이 아니다.
진행 중 기능의 테스트 계획은 `Document/Plan/<기능명>/TestPlan.md`에서 관리한다.

이 문서의 목적은 아래와 같다.

```text
- 완료된 기능이 이후 작업으로 깨졌는지 빠르게 확인한다.
- 기능별 테스트 기준 문서 위치를 한 곳에서 찾는다.
- 새로운 기능 완료 후 회귀 테스트 목록에 추가한다.
- AI가 수정 작업 전후로 확인할 최소 검증 범위를 알 수 있게 한다.
```

---

## 2. 문서 사용 기준

문서 역할은 아래처럼 분리한다.

| 문서 위치 | 역할 |
|---|---|
| `Document/Plan/<기능명>/TestPlan.md` | 진행 중 기능의 상세 테스트 계획 |
| `Document/ProjectSSOT/05_TestChecklist.md` | 현재 기준선/P0 검증 기록 |
| `Document/ProjectSSOT/05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 인덱스 |
| `Document/Systems/<분류>/<기능명>.md` | 완료 기능의 현재 구현 기준 |

---

## 3. 테스트 상태 표기

| 상태 | 의미 |
|---|---|
| `PASS` | 통과 |
| `FAIL` | 실패 |
| `PARTIAL` | 일부 통과 / 일부 미확인 |
| `N/A` | 현재 조건에서 해당 없음 |
| `TODO` | 아직 테스트 항목만 있고 수행 전 |

---

## 4. 공통 테스트 원칙

```text
1. 현재 구현 기준은 Document/Systems/를 우선한다.
2. 미래 계획은 테스트 PASS 기준으로 쓰지 않는다.
3. 실패 시 원인을 코어 문제 / 수치 문제 / 에디터 설정 문제 / 테스트 환경 문제로 분리한다.
4. 테스트 결과가 기능 책임을 바꾸면 해당 Systems 문서도 갱신한다.
5. 2026-06-18 싱글 전환 기준에서는 서버/멀티/2클라 테스트를 기본 회귀 조건으로 쓰지 않는다.
6. 서버/멀티 항목은 현재 사이클에서 N/A로 두고, 서버 작업 재개 결정이 있을 때만 다시 활성화한다.
7. 2026-06-19 전투 루프 기준에서는 조준/발사/피격/피해/피드백을 서버 없이 싱글 PIE에서 먼저 검증한다.
8. 2026-07-09 Reticle / FireFeedback 정리 기준에서는 서버 대기 / 서버 거부 표현을 현재 UI PASS 기준으로 쓰지 않는다.
9. OutOfArc / bLocalWithinWeaponArc는 현재 P0 싱글플레이 기준에서 단독 발사 차단 조건이 아니라 조준각 경고/디버그 상태로 본다.
10. 발사 성공 여부는 WeaponFire의 LastFireResult / ValidateFireCommand 결과를 기준으로 판단한다.
11. 무기 피격 형상은 VehicleMesh Physics Asset이 아니라 플레이어에게 보이는 시각 차체 SM_Body를 기준으로 전환한다.
12. Reticle이 지시하는 월드 목표점, 터렛 추적 방향, Muzzle 발사 방향, 실제 탄환 방향은 하나의 Aim Solution을 공유해야 한다.
13. 고속 Projectile은 단일 프레임 위치 검사만으로 PASS 처리하지 않고 Sweep/Sub-stepping 및 필요 시 보조 Sphere Sweep으로 연속 충돌을 검증한다.
14. 터렛 정렬 중 발사는 `UCFTurretMountData.bAllowFireWhileAligning`의 true/false 정책을 각각 검증하며, 두 정책 모두 `MuzzleBlocked`와 `TurretAligning` 표시 의미를 유지해야 한다.
15. 전투 FX / Audio는 승인된 발사, 첫 Blocking Hit와 최초 Destroyed 전환에만 1회 발생해야 하며 연출 실패가 전투 판정을 바꾸면 안 된다.
```

---

## 4-1. 2026-06-18 싱글 전환 테스트 기준

현재 사이클의 최소 검증 목표는 아래다.

```text
1. 서버 실행 없이 에디터와 PIE 1인 플레이만으로 기준 차량을 검증한다.
2. 기준 차량 1대가 바로 조작 가능해야 한다.
3. 주행감, 카메라, 로컬 Aim, Reticle, WheelSync 시각 품질을 우선 확인한다.
4. Dedicated Server, 2클라, 이동 복제, 서버 권한 발사 검증은 현재 기준 N/A다.
```

---

## 4-2. 2026-06-19 전투 루프 검증 기준

현재 사이클의 전투 루프 검증 목표는 아래다.

```text
1. 기준 차량 1대가 주행 중 조준과 발사를 수행한다.
2. 발사 성공/불가/쿨다운 상태가 Reticle, 간단한 UI, 이펙트, 사운드 후보로 읽힌다.
3. 발사 결과가 빗나감/피격 기록으로 남고, 후속 단계에서 피해 처리로 이어질 수 있어야 한다.
4. 주행, 조준, 발사, 피격, 피해 흐름을 반복해도 상태가 꼬이지 않는다.
5. 조작감, 전투 템포, 피드백 문제를 기능 차단과 품질 후속으로 분리한다.
```

---

## 4-3. 2026-07-09 WeaponFire / FireFeedback 테스트 기준

Reticle 관련 작업 전 최소 검증 기준은 아래다.

```text
1. WeaponFire는 판정/기록 담당으로 본다.
2. AimReticle / FireFeedback은 표시 담당으로 본다.
3. Reticle은 서버 상태를 표시하지 않는다.
4. FirePending은 서버 대기가 아니라 로컬 발사 처리 피드백 상태다.
5. FireRejected는 서버 거부가 아니라 로컬 발사 조건 미충족 상태다.
6. WeaponCooldown은 Cooldown 표시 후보로 변환될 수 있어야 한다.
7. NoWeapon은 무기 없음 표시 후보로 변환될 수 있어야 한다.
8. AimBlocked는 Blocked 표시 후보로 변환될 수 있어야 한다.
9. OutOfArcWarning은 발사 차단이 아니라 보조 경고/디버그 상태로 본다.
```

## 4-4. 2026-07-13 Reticle / FireFeedback 검증 결과

확인 완료:

```text
- Unreal Editor 타깃 빌드 성공
- WBP_AimReticle Reticle 이미지 5개와 FireFeedback TextBlock 바인딩 확인
- Ready 상태의 흰색 Reticle 확인
- FireSuccess 상태의 녹색 Reticle / 텍스트 확인
- FireSuccess 표시 후 Cooldown 파란색 상태 전환 확인
- 연속 Fire 입력 후 쿨다운 종료 시 FireFeedback 텍스트가 남지 않음
- 전용 Text_OutOfArcWarning이 실제 OutOfArcWarning 피드백에서만 일반 State/Hint를 대체함
- 조준각 밖에서 FireSuccess / Cooldown / FireRejected 피드백이 활성화되어도 일반 FireFeedback 텍스트가 가려지지 않음
```

2026-07-15 최종 사용자 PIE 확인:

```text
- NoWeapon 회색 Reticle과 무기 없음 안내 문구: PASS
- AimBlocked 주황 Reticle과 조준 가림 안내 문구: PASS
- 두 실패 상태의 유지 시간 종료 후 텍스트 제거: PASS
- AimBlocked 장애물 제거 후 정상 Aim 상태 복귀: PASS
- Ready → FireSuccess → Cooldown → Ready 정상 발사 회귀: PASS
```

상태 판정:

```text
- CF-TC-006 조준 Reticle: PASS
- CF-TC-013 차량 무기 조준/발사: PARTIAL
- CF-TC-014 발사 피드백/UI: PASS
```

## 4-5. 2026-07-13 시각 메시 기반 피격 검증 결과

구현 및 확인 완료:

```text
- WeaponHit Trace Channel, Projectile Object Channel, VehicleVisualHit Profile 구현
- VehicleMesh는 차량 물리를 유지하면서 WeaponHit / Projectile Ignore
- SM_Body는 QueryOnly 상태에서 WeaponHit / Projectile Block
- HitScan과 Projectile의 HitComponentName 기록
- Unreal Editor 타깃 빌드 성공
- 일반 속도 HitScan / Projectile이 SM_Body에서 정상 충돌하는 것을 사용자 PIE에서 확인
- 차량 주행 물리와 시각 차체 피격 분리가 정상 작동
```

후속 범위:

```text
- Reticle·Muzzle 정렬과 최소 BaseDamage 체력 감소는 P0 사용자 PIE 완료
- 전체 FPS·속도 조합, 이동 차량과 주행 중 반복 전투는 CF-FQ-019 확장 회귀로 남음
- 장갑·모듈 피해와 완성형 파괴 연출은 후속 기능 범위
```

현재 판정:

```text
- CF-TC-015 시각 메시 기반 피격 판정 기록: PASS
  - 일반 속도 시각 차체 Hit PASS
  - 30 FPS + 기준 속도 4배 P0 집중 스트레스 PASS
- CF-TC-016 피해 처리: PASS
  - 최소 Damage Runtime C++와 공식 Editor 빌드 완료
  - BaseDamage 체력 누적 감소, 파괴 전환, 추가 피해 거부와 이벤트 1회성 사용자 PIE 확인
- CF-TC-020 고속 Projectile 연속 충돌: PASS (P0 집중 스트레스 범위)
```

## 4-6. 2026-07-13 Aim / Projectile 신뢰성 선행 기준

조준 정렬 기준:

```text
- Reticle의 월드 목표점을 DesiredAimTargetLocation 단일 기준으로 사용한다.
- Camera Aim Trace와 실제 무기 Trace는 WeaponHit 응답표를 공유한다.
- 터렛은 Muzzle → DesiredAimTargetLocation 요구 방향을 추적한다.
- CurrentMuzzleDirection과 DesiredLaunchDirection의 정렬 오차를 계산한다.
- `UCFTurretMountData.bAllowFireWhileAligning=true`이면 정렬 중 현재 Muzzle 방향 발사를 허용한다.
- `bAllowFireWhileAligning=false`이면 정렬 완료 전 발사를 거부한다.
- 정렬 완료 후에는 Muzzle → Reticle 목표 방향을 사용한다.
- 총구 앞 장애물은 실제 최종 발사 방향 Trace로 확인하며 정책과 관계없이 발사를 차단한다.
```

고속 Projectile 기준:

```text
- bSweepCollision과 UpdatedComponent를 명시적으로 검증한다.
- 고속/중력 Projectile은 Sub-stepping을 사용한다.
- 잔여 터널링은 Previous → Current CollisionRadius Sphere Sweep으로 검출한다.
- OnComponentHit과 보조 Sweep은 동일 Impact를 두 번 처리하지 않는다.
- P0 완료는 30 FPS + 기준 속도 4배의 차량 집중 발사와 얇은 벽 첫 Hit, Pool 재사용 스트레스로 검증한다.
- 60 / 120 FPS와 기준 속도 1배 / 2배, 이동 차량 검증은 CF-FQ-019 확장 회귀로 수행한다.
```

2026-07-14 P0 검증 결과:

```text
- 차량 집중 발사: PASS
- 얇은 벽 앞 차량 배치와 첫 Blocking Hit: PASS
- 중복 Impact: 없음
- Pool 재사용 이상: 없음
- 판정: CF-TC-020 PASS / CF-FQ-023 Done
```

## 4-7. 2026-07-13 AimFireAlignment 빌드 결과

구현 및 빌드 확인:

```text
- Camera Aim Trace와 무기 Trace의 WeaponHit 기준 통일 코드 반영
- FCFVehicleWeaponAimSolution, TurretAligning, WeaponNotAligned, MuzzleBlocked 상태 기록 경로 반영
- HitScan / Projectile이 같은 AimOrigin / AimDirection / Target을 사용하도록 Core 경로 반영
- Reticle UI가 TurretAligning을 정렬 중 문구와 amber 보조 색상으로 표시
- VehicleDebug Panel Aim 섹션에 Weapon Aim Solution 표시 추가
- Reticle Recovery Hotfix 포함 Unreal Editor 타깃 빌드 성공
```

Align Fire Policy 구현 및 빌드 완료:

```text
- `UCFTurretMountData.bAllowFireWhileAligning` 터렛별 스위치와 기본값 true 반영
- Weapon Aim Solution에 정책값, DesiredAimDirection, CurrentMuzzleDirection, 실제 최종 AimDirection 분리
- 정책 true의 정렬 중 CurrentMuzzleDirection 발사와 정책 false의 정렬 거부 코드 반영
- 실제 최종 발사 경로 기준 MuzzleBlocked 검사 반영
- `Tools/BuildEditor.bat` 성공
```

2026-07-14 사용자 PIE 확인:

```text
- Reticle 목표점과 정렬 완료 후 실제 탄착 일치: PASS
- 터렛 정렬 중 TurretAligning amber 표시: PASS
- `bAllowFireWhileAligning=true` 정렬 중 발사 승인과 CurrentMuzzleDirection 진행: PASS
- `bAllowFireWhileAligning=false` 정렬 중 발사 거부와 정렬 완료 후 승인: PASS
- WeaponNotAligned가 빨간 FireRejected로 주 Reticle을 덮지 않음: PASS
- 정책 true/false 양쪽 총구 장애물 MuzzleBlocked 발사 차단: PASS
```

현재 판정:

```text
- CF-TC-019 Reticle·터렛·총구 정렬: PASS
  - AimFireAlignment / Reticle Recovery / Align Fire Policy 공식 빌드 완료
  - P0 축약 사용자 PIE 완료
  - 주행·거리별 정량 오차와 경계각 검증은 CF-FQ-019 확장 회귀
```

## 4-8. CF-FQ-024 전투 FX / Audio 검증 기준

현재 구현 전 테스트 계약:

```text
- 승인된 발사 1회당 실제 Muzzle 위치에서 발사 FX와 공간 사운드가 각 1회 발생한다.
- Cooldown, NoWeapon, AimBlocked와 MuzzleBlocked 거부에서는 발사 FX와 사운드가 발생하지 않는다.
- HitScan과 Projectile은 FCFDamageHitContext의 첫 ImpactPoint / ImpactNormal에서 FX와 사운드를 각 1회 발생시킨다.
- Projectile OnComponentHit과 보조 Sweep은 같은 Impact 연출을 중복 재생하지 않는다.
- 최초 Destroyed 상태 전환에서만 파괴 FX와 사운드가 각 1회 발생하며 추가 피해에서 반복되지 않는다.
- Projectile Pool 재사용에서 Trail, Niagara 또는 Audio 상태가 남지 않는다.
- 연출 자산이 비어 있거나 재생에 실패해도 발사·피해·파괴 판정은 유지된다.
```

현재 판정:

```text
- CF-FQ-024: Active / Implementation Not Started
- CF-TC-021: TODO
- 신규 Build / PIE 결과 없음
```

## 4-9. 2026-07-21 이중 레티클 검증 결과

구현 및 빌드 확인:

```text
- Weapon Aim Solution에 bHasValidTurretReticlePoint, TurretReticleWorldLocation, TurretReticleDistance 추가
- TurretReticleWorldLocation은 CurrentMuzzleDirection과 사용자 조준점 비교 거리로 계산
- Image_WeaponReticle의 ECFWeaponReticleMode / WeaponPreviewWorldLocation 소비 제거
- DirectImpact / LaunchDirection Preview는 Legacy Debug로만 보존
- Tools\BuildEditor.bat PASS
```

2026-07-21 사용자 PIE 확인:

```text
- 계획한 Image_CenterDot 조준 레티클 동작 유지: PASS
- Image_WeaponReticle의 실제 터렛 방향 추적: PASS
- 정렬 중 분리와 정렬 완료 수렴: PASS
- 탄종과 착탄 위치에서 분리된 터렛 레티클 의미: PASS
- 판정: CF-TC-022 PASS / CF-FQ-025 Done
```

---

## 5. 전체 최소 회귀 테스트 세트

아래 테스트는 큰 구조 수정 후 최소 1회 확인한다.

| ID | 영역 | 테스트 | PASS 기준 | 관련 Systems 문서 | 상태 |
|---|---|---|---|---|---|
| `CF-TC-001` | Vehicle | 기본 차량 조작 | 전진/후진/조향/브레이크/핸드브레이크가 동작 | `Document/Systems/Vehicles/VehicleDrive.md` | `TODO` |
| `CF-TC-002` | Vehicle | 휠 시각 동기화 | 휠 위치/조향/스핀 시각 반응이 정상 | `Document/Systems/Vehicles/WheelSync.md` | `TODO` |
| `CF-TC-003` | Vehicle | 카메라 | 차량 기준 카메라가 정상 추적/회전 | `Document/Systems/Vehicles/VehicleCamera.md` | `TODO` |
| `CF-TC-004` | Input | 기본 입력 등록 | Enhanced Input Mapping Context 등록 성공 | `Document/Systems/Input/Input.md` | `TODO` |
| `CF-TC-005` | UI | 차량 디버그 표시 | 디버그 HUD/Panel이 필요한 조건에서 표시 | `Document/Systems/UI/VehicleDebug.md`, `Document/Systems/UI/VehicleDebugPanel.md` | `TODO` |
| `CF-TC-006` | UI | 조준 Reticle | 조준 Reticle 표시/갱신이 정상이며 로컬 발사 결과 피드백 후보와 충돌하지 않음 | `Document/Systems/UI/AimReticle.md`, `Document/Systems/Combat/FireFeedback.md` | `PASS` |
| `CF-TC-007` | Network | Dedicated Server 실행 | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-008` | Network | 1클라 Spawn/Possess | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-009` | Network | 2클라 Spawn/Possess | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-010` | Network | 입력 분리 | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md`, `Document/Systems/Input/Input.md` | `N/A` |
| `CF-TC-011` | Network | 이동 복제 | 현재 싱글 전환 기준에서는 기본 회귀에서 제외 | `Document/Systems/Network/ServerSpawn.md` | `N/A` |
| `CF-TC-012` | Config | 런타임 설정 | 현재 Config 기준 경로/모드가 깨지지 않음 | `Document/Systems/Config/ProjectRuntimeConfig.md` | `TODO` |
| `CF-TC-013` | Combat | 차량 무기 조준/발사 | 조준 방향으로 로컬 발사 명령이 생성되고 성공/불가/쿨다운 상태가 구분됨 | `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/UI/VehicleDebugPanel.md` | `PARTIAL` |
| `CF-TC-014` | Feedback | 발사 피드백/UI | 발사 성공/불가/쿨다운/무기 없음 상태가 Reticle 또는 FireFeedback UI 기준으로 읽힘 | `Document/Systems/Combat/FireFeedback.md`, `Document/Systems/UI/AimReticle.md`, `Document/Systems/Combat/WeaponFire.md` | `PASS` |
| `CF-TC-015` | Combat | 시각 메시 기반 피격 판정 기록 | HitScan/Projectile이 SM_Body 시각 차체에서 명중하고 P0 고속 집중 조건에서도 DamageHitContext로 누락 없이 기록됨 | `Document/Plan/HitDamage/ImplementationDesign.md`, `Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md`, `Document/Systems/Combat/DamageHitContext.md`, `Document/Systems/Combat/Projectile.md` | `PASS` |
| `CF-TC-016` | Combat | 피해 처리 | BaseDamage가 차량 체력에 정확히 한 번 누적되고 체력 0 이하에서 파괴 상태가 한 번만 전환됨 | `Document/Systems/Combat/HitDamage.md`, `Document/Systems/Combat/DamageHitContext.md` | `PASS` |
| `CF-TC-017` | Loop | 주행/전투 반복 | 주행, 조준, 발사, 피격, 피해 루프를 반복해도 상태가 꼬이지 않음 | `Document/Systems/Combat/CoreLoop.md` 예정 | `TODO` |
| `CF-TC-018` | Feel | 전투 템포/피드백 | 조작감, 발사 리듬, 피격 반응 문제가 기능 차단과 품질 후속으로 분리됨 | `Document/Systems/Combat/CombatFeel.md` 예정 | `TODO` |
| `CF-TC-019` | Combat | Reticle·터렛·총구 정렬 | 동일 Aim Solution을 공유하고, `bAllowFireWhileAligning` true/false 양쪽에서 실제 발사 방향·거부 조건·TurretAligning·MuzzleBlocked 표시가 정책대로 동작함 | `Document/Plan/AimFireAlignment/ImplementationDesign.md`, `Document/Systems/Vehicles/VehicleAim.md`, `Document/Systems/Combat/WeaponFire.md`, `Document/Systems/UI/AimReticle.md` | `PASS` |
| `CF-TC-020` | Combat | 고속 Projectile 연속 충돌 | 30 FPS + 기준 속도 4배에서 차량과 얇은 벽을 통과하지 않고 첫 Hit을 한 번 기록하며 Pool 재사용이 정상임 | `Document/Plan/ProjectileContinuousCollision/ImplementationDesign.md`, `Document/Systems/Combat/Projectile.md`, `Document/Systems/Combat/DamageHitContext.md` | `PASS` |
| `CF-TC-021` | Presentation | 전투 FX / Audio | 승인된 발사, 첫 Impact와 최초 파괴에서 FX·Sound가 각 1회 발생하고 거부·중복 판정·Pool 재사용에서 중복 또는 잔류가 없음 | `Document/Plan/CombatFxAudio/ImplementationDesign.md`, 완료 후 `Document/Systems/Combat/CombatFxAudio.md` | `TODO` |
| `CF-TC-022` | UI | 조준·터렛 이중 레티클 | Image_CenterDot은 사용자 조준점을 유지하고 Image_WeaponReticle은 CurrentMuzzleDirection 기반 터렛 조준 지점을 탄종·착탄 위치와 무관하게 표시 | `Document/Plan/ReticleAimDirection/ImplementationDesign.md`, `Document/Systems/UI/AimReticle.md`, `Document/Systems/Vehicles/VehicleAim.md` | `PASS` |

---

## 6. Vehicles 테스트

## 6.1 VehicleDrive

- 관련 문서: `Document/Systems/Vehicles/VehicleDrive.md`

### 확인 항목

```text
- Throttle 입력이 차량 전진에 적용된다.
- Brake 입력이 감속/정지에 적용된다.
- Steering 입력이 좌우 조향에 적용된다.
- Handbrake 입력이 핸드브레이크 동작에 적용된다.
- DriveState가 Idle / Accelerating / Braking / Reversing / Coasting / Airborne / Disabled 범위에서 의미 있게 변한다.
- VehicleMovement를 찾지 못하는 경우 Disabled로 안전하게 떨어지는지 확인한다.
```

### PASS 기준

```text
- 입력 적용이 끊기지 않는다.
- DriveState가 명백히 틀린 상태로 고정되지 않는다.
- 입력/상태 디버그가 확인 가능하다.
```

---

## 6.2 WheelSync

- 관련 문서: `Document/Systems/Vehicles/WheelSync.md`

### 확인 항목

```text
- Wheel_Anchor_* 위치가 기준 차량 휠 중심과 맞는다.
- 조향 시 앞바퀴 피벗이 자연스럽다.
- 전진/후진 시 휠 스핀 방향이 정상으로 보인다.
- 심한 떨림 또는 이중 적용이 없다.
- 고속 휠 시각 품질 이슈가 기능 FAIL인지, 품질 후속인지 분리된다.
```

### PASS 기준

```text
- 기본 주행 중 휠 위치/조향/스핀 시각 동기화가 기능적으로 정상이다.
- 품질 이슈가 있어도 기능 판정과 분리해서 기록 가능하다.
```

---

## 6.3 VehicleCamera

- 관련 문서: `Document/Systems/Vehicles/VehicleCamera.md`

### 확인 항목

```text
- 로컬 플레이어 차량 기준 카메라가 정상 생성/활성화된다.
- Look 입력이 카메라에 반영된다.
- 싱글 PIE에서 카메라가 차량 기준으로 추적/회전한다.
- 서버/비소유 Pawn 검증은 현재 사이클에서 N/A다.
```

### PASS 기준

```text
- 로컬 클라이언트 카메라가 정상 동작한다.
- PIE 1인 플레이에서 카메라 이동, 회전, 충돌 감각이 확인 가능하다.
```

---

## 6.4 VehicleAim

- 관련 문서: `Document/Systems/Vehicles/VehicleAim.md`

### 확인 항목

```text
- 현재 AimComp가 의도한 기준으로 조준 방향/타겟 정보를 계산한다.
- UI Reticle과 연결되는 데이터가 유효하다.
- LocalAimState와 FireValidationState를 구분해 확인할 수 있다.
- bLocalWithinWeaponArc / OutOfArc는 표시/디버그 상태로 확인한다.
- Reticle 월드 목표점이 DesiredAimTargetLocation 단일 기준으로 유지되는지 확인한다.
- Muzzle → DesiredAimTargetLocation 요구 방향과 CurrentMuzzleDirection의 정렬 오차를 확인한다.
- TurretAligning과 OutOfArc가 서로 다른 원인으로 구분되는지 확인한다.
- 총구 앞 장애물 Trace 결과를 확인한다.
- 서버 권한 발사 구조는 현재 보류하고, 로컬 Aim / Reticle 피드백을 우선한다.
```

### PASS 기준

```text
- 조준 데이터가 UI 또는 후속 Fire 요청에서 읽을 수 있는 형태로 유지된다.
- 서버 없이 로컬 조준 상태와 Reticle 상태를 확인할 수 있다.
- OutOfArc를 단독 발사 차단으로 오해하지 않도록 Debug / UI 기준이 분리되어 있다.
- Reticle 목표점, 터렛 요구 방향, 총구 발사 방향이 동일 Aim Solution에서 파생된다.
- `bAllowFireWhileAligning=true`의 정렬 중 발사 허용과 `false`의 정렬 중 발사 차단을 각각 확인할 수 있다.
- 총구 앞 장애물 차단은 두 정책 모두에서 원인별로 확인할 수 있다.
```

---

## 7. Input 테스트

- 관련 문서: `Document/Systems/Input/Input.md`

### 확인 항목

```text
- DefaultInputMappingContext가 로컬 플레이어에 등록된다.
- Throttle / Brake / Steering / Handbrake 입력이 바인딩된다.
- VehicleMove 2D 입력과 Legacy Axis 입력이 충돌하지 않는다.
- InputDeviceMode가 Auto / KeyboardMouseOnly / GamepadOnly 조건에서 의도대로 필터링된다.
- Look 입력이 VehicleCameraComp로 전달된다.
```

### PASS 기준

```text
- 현재 테스트 장치에서 차량 조작 입력이 정상 동작한다.
- 장치 모드 제한 때문에 입력이 막힌 경우 구조 실패와 설정 문제를 분리해 기록한다.
```

---

## 8. UI 테스트

## 8.1 AimReticle

- 관련 문서:
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Combat/FireFeedback.md`

### 확인 항목

```text
- Reticle 위젯이 필요한 조건에서 생성된다.
- 조준 상태 변화가 Reticle에 반영된다.
- 싱글 PIE에서 로컬 차량 기준 Reticle 표시/갱신을 확인한다.
- 주 Reticle은 플레이어가 지정한 DesiredAimTargetLocation을 나타내는 Command Reticle로 해석한다.
- TurretAligning / Ready / Blocked 상태가 서로 구분된다.
- Reticle은 서버 대기 / 서버 거부 상태를 표시하지 않는다.
- FirePending / FireRejected는 로컬 발사 피드백 상태로 해석된다.
- Cooldown / NoWeapon / FireRejected 표시가 WeaponFire 결과와 충돌하지 않는다.
```

### PASS 기준

```text
- 로컬 클라이언트 화면에서 Reticle이 정상 표시된다.
- Aim 상태 변화가 Reticle 표시 상태로 읽힌다.
- Reticle이 지정한 목표점과 실제 탄착이 허용 오차 안에서 일치한다.
- 터렛 정렬 중과 정렬 완료를 UI에서 구분할 수 있다.
- 로컬 발사 결과 피드백이 Reticle 기준과 충돌하지 않는다.
```

---

## 8.2 VehicleDebug / VehicleDebugPanel

- 관련 문서:
  - `Document/Systems/UI/VehicleDebug.md`
  - `Document/Systems/UI/VehicleDebugPanel.md`

### 확인 항목

```text
- 디버그 UI 생성 조건이 명확하다.
- 표시 텍스트가 현재 Systems 기준 필드와 맞는다.
- 너무 긴 Runtime 문자열이 가독성을 해치지 않는지 확인한다.
- Aim / Weapon / Camera / Runtime 섹션이 필요한 정보를 제공한다.
- Weapon 섹션에서 WeaponData, 쿨다운, Projectile, DamageHitContext, FireOrigin 상태를 확인할 수 있다.
- 서버 전용 UI 차단 검증은 현재 사이클에서 N/A다.
```

### PASS 기준

```text
- 디버그 UI가 테스트 중 필요한 정보를 제공한다.
- 싱글 PIE에서 주행/카메라/Aim/Weapon/WheelSync 정보를 읽을 수 있다.
- Debug Panel은 판정 계산기가 아니라 Snapshot 표시 UI로 동작한다.
```

---

## 9. Combat / Feedback / Loop 테스트

현재 전투 루프 테스트는 싱글플레이 로컬 기준으로 검증한다.
`WeaponFire`, `FireFeedback`, `Projectile`, `DamageHitContext` 기준 문서는 현재 `Document/Systems/Combat/` 아래에 존재한다.
단, 실제 플레이 테스트 결과가 아직 기록되지 않은 항목은 `TODO` 상태를 유지한다.

## 9.1 WeaponFire

- 관련 문서:
  - `Document/Systems/Combat/WeaponFire.md`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/UI/VehicleDebugPanel.md`

### 확인 항목

```text
- Fire 입력이 로컬 기준에서 발사 명령으로 연결된다.
- 발사 원점과 발사 방향을 확인할 수 있다.
- FireRequest.AimDirection이 Muzzle → DesiredAimTargetLocation 요구 방향을 사용한다.
- CurrentMuzzleDirection과 DesiredLaunchDirection의 정렬 오차를 확인할 수 있다.
- `bAllowFireWhileAligning=true`에서 정렬 중 현재 Muzzle 방향 발사, `false`에서 정렬 중 발사 거부가 구분된다.
- 총구 앞 장애물 거부는 두 정책 모두에서 별도로 구분된다.
- 발사 성공 / 발사 불가 / 쿨다운 상태가 구분된다.
- LastFireResult와 RejectReason을 확인할 수 있다.
- Weapon Debug에서 WeaponData / Cooldown / FireOrigin / Aim Alignment 상태를 확인할 수 있다.
- OutOfArc / bLocalWithinWeaponArc는 TurretAligning과 구분한다.
- 서버 권한 발사와 복제 검증은 현재 사이클에서 N/A다.
```

### PASS 기준

```text
- 싱글 PIE에서 기준 차량이 Reticle 목표점을 향하는 로컬 발사 명령을 만든다.
- `bAllowFireWhileAligning=true`이면 정렬 중 현재 Muzzle 방향으로 발사가 승인된다.
- `bAllowFireWhileAligning=false`이면 정렬 완료 후에만 발사가 승인된다.
- 정렬 완료 후에는 두 정책 모두 Reticle 목표 방향을 사용한다.
- 실패 시 원인을 입력 / 터렛 정렬 / 총구 막힘 / 쿨다운 / 데이터 문제로 분리할 수 있다.
- 발사 결과가 AimComp / VehicleDebug 상태에 기록된다.
```

---

## 9.2 FireFeedback

- 관련 문서:
  - `Document/Systems/Combat/FireFeedback.md`
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Combat/WeaponFire.md`

### 확인 항목

```text
- 발사 성공 시 최소 피드백 후보가 발생하거나 표시 기준이 확인된다.
- 발사 불가 또는 쿨다운 상태가 조준 UI / Reticle / Debug로 구분된다.
- NoWeapon / WeaponCooldown / AimBlocked / FireRejected 표시 후보가 구분된다.
- 피드백 호출과 실제 판정 흐름이 분리되어 있다.
- UI 표시 문구는 한국어 표시 정책을 따른다.
- 서버 대기 / 서버 거부 표현을 현재 UI 표시 기준으로 쓰지 않는다.
```

### PASS 기준

```text
- 플레이어가 발사 성공, 발사 불가, 쿨다운, 무기 없음 상태를 즉시 이해할 수 있다.
- 이펙트/사운드가 없어도 판정 흐름을 추적할 수 있다.
- 피드백은 WeaponFire 판정 결과를 임의로 바꾸지 않는다.
```

---

## 9.3 HitDamage / DamageHitContext

- 관련 문서:
  - `Document/Systems/Combat/DamageHitContext.md`
  - `Document/Systems/Combat/Projectile.md`
  - `Document/Systems/Combat/HitDamage.md` 예정

### 확인 항목

```text
- 발사 결과가 빗나감 / 피격으로 구분된다.
- VehicleMesh의 보이지 않는 돌출부가 무기 피격을 받지 않는지 확인한다.
- SM_Body의 보이는 차체 표면에서 HitScan과 Projectile이 Blocking Hit을 만드는지 확인한다.
- HitResult.HitComponent가 SM_Body 또는 승인된 시각 피격 컴포넌트인지 확인한다.
- Dummy HitScan 또는 Projectile Actor 충돌 결과가 DamageHitContext로 기록된다.
- 고속 Projectile을 30 / 60 / 120 FPS에서 발사한다.
- 기준 속도 1배 / 2배 / 4배에서 얇은 벽과 SM_Body 통과 여부를 확인한다.
- OnComponentHit과 보조 Sweep이 같은 충돌을 두 번 기록하지 않는지 확인한다.
- 첫 Blocking Hit만 DamageHitContext에 기록되는지 확인한다.
- 차량 주행과 지면/벽 물리 충돌이 기존처럼 유지되는지 확인한다.
- 벽과 지형은 무기 Trace와 Projectile을 계속 차단하는지 확인한다.
- 자기 차량 Ignore 정책이 유지되는지 확인한다.
- 피격 결과가 후속 피해량으로 변환될 준비가 되어 있다.
- 피해 처리 실패 시 Aim / Fire / Hit / Damage 중 어느 단계 문제인지 분리된다.
```

### PASS 기준

```text
- 보이는 SM_Body 차체 표면과 실제 무기 피격 표면이 기능적으로 일치한다.
- VehicleMesh Physics Asset은 차량 물리에만 참여하고 무기 피격 Query를 가로채지 않는다.
- HitScan과 일반/고속 Projectile 모두 승인된 시각 피격 컴포넌트에서 Hit을 기록한다.
- 고속/저프레임에서도 얇은 충돌체를 통과하지 않는다.
- 한 Projectile Activation에서 Impact와 Pool 반환이 정확히 한 번 발생한다.
- 로컬 테스트에서 피격/빗나감과 HitComponent를 확인할 수 있다.
- DamageHitContext와 DamageApplyResult를 통해 피격 기록부터 체력 변경 결과까지 추적할 수 있다.
- 차량 주행 물리와 월드 충돌에 회귀가 없다.
```

---

## 9.4 CoreLoop

- 관련 문서: `Document/Systems/Combat/CoreLoop.md` 예정

### 확인 항목

```text
- 주행 중 조준/발사 입력이 끊기지 않는다.
- 조준/발사/피격/피해 루프를 5회 이상 반복한다.
- 반복 중 UI, 디버그, 피해 상태가 꼬이지 않는다.
- 남은 문제를 기능 차단 / 품질 후속 / 장기 확장으로 분리한다.
```

### PASS 기준

```text
- 기준 차량 1대로 주행, 조준, 발사, 피격, 피해 흐름을 반복할 수 있다.
- 다음 개발 단계로 넘어갈 수 있는지 PASS / FAIL 판정이 가능하다.
```

---

## 10. Network 테스트

현재 싱글 전환 기준에서 Network 테스트는 기본 회귀 테스트가 아니다.
아래 항목은 서버/멀티 작업을 재개할 때 다시 활성화한다.

## 10.1 ServerSpawn

- 관련 문서: `Document/Systems/Network/ServerSpawn.md`

### 확인 항목

```text
- CarFight_ReServer Target 빌드/실행이 가능하다.
- 서버가 TestMap 또는 서버 테스트맵을 로드한다.
- Client 1이 접속한다.
- 서버에서 PlayerController 1개가 확인된다.
- ACFMPGameMode::PostLogin() 흐름이 호출된다.
- 차량 Pawn이 Spawn된다.
- PlayerController가 Spawn된 차량을 Possess한다.
- Client 1 화면이 검정 화면에 고정되지 않는다.
```

### PASS 기준

```text
- 1클라 접속 후 자기 차량을 조작할 수 있다.
- 서버 로그에 Spawn/Possess 성공 로그가 확인된다.
```

---

## 10.2 2클라 소유권 / 입력 분리

- 관련 문서:
  - `Document/Systems/Network/ServerSpawn.md`
  - `Document/Systems/Input/Input.md`

### 확인 항목

```text
- Client 1과 Client 2가 모두 접속한다.
- 차량 Pawn이 2대 생성된다.
- Client 1과 Client 2가 서로 다른 차량을 소유한다.
- Client 1 입력은 Client 1 차량에만 적용된다.
- Client 2 입력은 Client 2 차량에만 적용된다.
- 비소유 차량이 로컬 입력에 반응하지 않는다.
```

### PASS 기준

```text
- 각 클라이언트가 자기 차량만 조작한다.
- 입력 교차 오염이 없다.
```

---

## 10.3 이동 복제

- 관련 문서: `Document/Systems/Network/ServerSpawn.md`

### 확인 항목

```text
- Client 1 차량 이동이 Client 2 화면에서 보인다.
- Client 2 차량 이동이 Client 1 화면에서 보인다.
- 위치 복제가 확인된다.
- 회전 복제가 확인된다.
- 정지 상태도 동기화된다.
- 급가속/급조향 시 품질 문제를 기능 FAIL과 분리한다.
```

### PASS 기준

```text
- 상대 차량 위치/회전이 갱신된다.
- 상대 차량이 전혀 안 보이거나 완전히 다른 위치로 분리되지 않는다.
```

---

## 11. Config 테스트

- 관련 문서: `Document/Systems/Config/ProjectRuntimeConfig.md`

### 확인 항목

```text
- DefaultEngine.ini의 주요 GameMode/ServerGameMode 연결이 현재 싱글 전환 기준과 맞는다.
- 기본 실행 경로에서 CFMPGameMode가 의도치 않게 사용되지 않는지 확인한다.
- 입력/맵/런타임 설정 경로가 현재 Systems 문서와 충돌하지 않는다.
- 오래된 ProjectSSOT/Plan 경로가 남아 있으면 실제 구조에 맞게 수정 후보로 기록한다.
```

### PASS 기준

```text
- 현재 실행 경로와 문서 경로가 서로 충돌하지 않는다.
- 런타임 설정 때문에 기본 테스트가 막히지 않는다.
```

---

## 12. 테스트 기록 양식

각 테스트 후 아래 형식으로 기록한다.

```text
테스트 일시:
테스트 환경:
엔진 버전:
실행 방식:
대상 맵:
대상 기능:
관련 Systems 문서:
결과: PASS / FAIL / PARTIAL / N/A
관찰 내용:
로그 요약:
수정 필요 항목:
다음 액션:
```

---

## 13. 실패 분류 기준

실패 시 아래 중 하나로 분류한다.

| 분류 | 의미 |
|---|---|
| `Core` | 코드 구조 또는 핵심 로직 문제 |
| `Data` | DataAsset / Config / 값 문제 |
| `Asset` | BP / 맵 / 메시 / 위젯 자산 문제 |
| `Server` | 권한 / 복제 / Dedicated Server 문제 |
| `Input` | 입력 매핑 / 장치 필터링 문제 |
| `UI` | 표시 / 위젯 생성 / 로컬 전용 처리 문제 |
| `Quality` | 기능은 되지만 품질 개선 필요 |
| `DocMismatch` | 문서와 실제 구현이 불일치 |

---

## 14. 문서 갱신 조건

아래 상황이 발생하면 이 문서를 갱신한다.

```text
- Document/Systems/에 새 완료 기능 문서가 추가됨
- 기존 Systems 문서의 기능 책임이 바뀜
- 회귀 테스트 항목이 늘거나 줄어듦
- 테스트 기준이 PASS/FAIL 판정에 영향을 줄 만큼 바뀜
- 반복 테스트 자동화 또는 관리툴 후보가 생김
```

---

## 15. 문서 버전 관리

- 현재 문서 버전: `v1.9.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 테스트 문서 운영 방식 자체 변경 |
| Minor | 새 시스템 테스트 섹션 추가 또는 PASS 기준 확장 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 16. 체인지로그

### v1.9.0 - 2026-07-21

```text
- CF-FQ-025 이중 레티클의 공식 빌드와 사용자 PIE 결과를 기록했다.
- CF-TC-022 조준·터렛 이중 레티클 회귀 테스트를 추가하고 PASS로 등록했다.
- 터렛 레티클을 CurrentMuzzleDirection 기반이며 탄종·착탄 위치와 독립적인 표시로 고정했다.
```

### v1.8.7 - 2026-07-15

```text
- CF-FQ-024 전투 FX / Audio의 구현 전 테스트 계약을 추가했다.
- 승인 발사, 첫 Impact, 최초 파괴의 1회성 연출과 거부·중복·Pool 잔류 방지 기준을 고정했다.
- CF-TC-021 전투 FX / Audio 테스트를 TODO로 등록했다.
- 신규 구현, 빌드 또는 PIE 결과는 아직 기록하지 않았다.
```

### v1.8.6 - 2026-07-15

```text
- 사용자 PIE에서 NoWeapon 회색 표시, AimBlocked 주황 표시와 두 상태의 텍스트 만료를 확인했다.
- 장애물 제거 후 정상 Aim 복귀와 Ready → FireSuccess → Cooldown → Ready 회귀를 확인했다.
- CF-TC-014 발사 피드백/UI를 PARTIAL에서 PASS로 변경했다.
- CF-TC-013은 더 넓은 WeaponFire 회귀 항목이므로 PARTIAL을 유지했다.
```

### v1.8.5 - 2026-07-15

```text
- 사용자 싱글 PIE에서 최소 Damage Runtime 정상 동작을 확인했다.
- BaseDamage 체력 누적 감소, 체력 0 이하 파괴 전환, 추가 피해 TargetDestroyed 거부와 파괴 이벤트 1회성을 PASS 처리했다.
- CF-TC-016을 PARTIAL에서 PASS로 변경하고 현재 구현 문서를 Systems/Combat/HitDamage.md로 연결했다.
- 전체 속도·FPS 조합과 이동 차량 반복 전투는 CF-FQ-019 확장 회귀로 유지했다.
```

### v1.8.4 - 2026-07-14

```text
- CF-FQ-018 최소 Damage Runtime C++와 공식 Editor 빌드 성공을 반영했다.
- CF-TC-016을 TODO에서 PARTIAL로 변경하고 사용자 PIE 피해 누적·파괴 전환을 남은 조건으로 기록했다.
- HitScan/Projectile 공용 피해 적용과 Projectile 첫 Impact 1회 적용 계약을 검증 기준에 반영했다.
- 사용자 확인 전에는 피해 처리를 PASS로 기록하지 않았다.
```

### v1.8.3 - 2026-07-14

```text
- CF-FQ-022 P0 사용자 PIE 결과를 CF-TC-019에 기록했다.
- 정책 true/false, 정렬 완료 탄착, TurretAligning amber와 양쪽 MuzzleBlocked를 PASS 처리했다.
- CF-TC-019 상태를 PARTIAL에서 PASS로 변경했다.
- 주행·거리별 정량 조준 검증은 CF-FQ-019 확장 회귀로 이관했다.
```

### v1.8.2 - 2026-07-14

```text
- 사용자 PIE 축약 스트레스 테스트 통과를 기록했다.
- CF-TC-015와 CF-TC-020을 P0 집중 스트레스 범위 PASS로 변경했다.
- 30 FPS + 기준 속도 4배 차량 집중 발사, 얇은 벽 첫 Blocking Hit, 중복 Impact 없음과 Pool 재사용 정상을 기록했다.
- 전체 FPS·속도 매트릭스와 이동 차량 검증을 CF-FQ-019 확장 회귀로 이관했다.
```

### v1.8.1 - 2026-07-14

```text
- Align Fire Policy C++ 구현과 `BuildEditor.bat` 성공을 CF-TC-019에 기록했다.
- 정책 true/false, 실제 최종 발사 경로 MuzzleBlocked, TurretAligning amber 검증은 PIE Pending으로 유지했다.
- CF-TC-019는 PARTIAL을 유지하고 사용자 확인 없이 PIE PASS를 기록하지 않았다.
```

### v1.8.0 - 2026-07-14

```text
- `UCFTurretMountData.bAllowFireWhileAligning` true/false 양쪽 검증 기준을 CF-TC-019에 추가했다.
- true일 때 정렬 중 현재 Muzzle 방향 발사, false일 때 정렬 완료 전 거부 기준을 기록했다.
- MuzzleBlocked는 항상 거부하고 TurretAligning amber 표시는 두 정책 모두 유지하도록 기록했다.
- 기존 AimFireAlignment 빌드 PASS와 Align Fire Policy 코드/빌드 Pending 상태를 분리했다.
```

### v1.7.1 - 2026-07-13

```text
- Reticle Recovery Hotfix 정적 구현 상태와 BuildEditor.bat 성공 상태 기록
- CF-TC-019 빌드 PASS / PIE Pending 상태 반영
- CF-TC-019는 PARTIAL / PIE Pending 유지
```

### v1.7.0 - 2026-07-13

```text
- AimFireAlignment Core/Presentation C++ 구현과 Unreal Editor 타깃 빌드 성공 결과 추가
- CF-TC-019를 TODO에서 PARTIAL로 변경
- PIE 검증은 Pending으로 남기고 PASS 기록을 보류
- TurretAligning / WeaponNotAligned / MuzzleBlocked UI/Debug 확인 항목 추가
```

### v1.6.0 - 2026-07-13

```text
- 일반 속도 HitScan / Projectile의 SM_Body 시각 차체 충돌 PIE 확인 결과 반영
- CF-TC-015를 TODO에서 PARTIAL로 변경하고 고속 Projectile 연속 충돌을 남은 조건으로 명시
- CF-TC-019 Reticle·터렛·총구 정렬 테스트 추가
- CF-TC-020 고속 Projectile 연속 충돌 테스트 추가
- DesiredAimTargetLocation, AimAlignmentError, Muzzle obstruction 기준 추가
- Sweep/Sub-stepping, 보조 Sphere Sweep, 중복 Impact 방지 테스트 기준 추가
- Document/Plan/AimFireAlignment 및 ProjectileContinuousCollision 설계 문서 연결
```

### v1.5.0 - 2026-07-13

```text
- 현재 차량 피격이 VehicleMesh Physics Asset 기준이라는 확인 결과 추가
- SM_Body를 시각 차체 기반 무기 피격 표면으로 전환한다는 테스트 원칙 추가
- CF-TC-015를 시각 메시 기반 HitScan/Projectile 피격 판정 기록 기준으로 보강
- VehicleMesh 무기 Query 제외, SM_Body 명중, HitComponent 확인, 차량 주행 물리 회귀 검증 항목 추가
- Document/Plan/HitDamage/ImplementationDesign.md를 CF-TC-015 관련 설계 문서로 연결
```

### v1.4.0 - 2026-07-13

```text
- CF-TC-006 조준 Reticle을 PASS로 갱신
- CF-TC-013 차량 무기 조준/발사와 CF-TC-014 발사 피드백/UI를 PARTIAL로 갱신
- Ready / FireSuccess / Cooldown 색상 전환과 연속 입력 후 텍스트 종료 확인 결과 추가
- 전용 OutOfArc 경고의 중복 방지, 종료 조건, 다른 FireFeedback 비가림 확인 결과 추가
- NoWeapon / AimBlocked 실제 PIE 검증을 남은 항목으로 기록
```

### v1.3.0 - 2026-07-09

```text
- WeaponFire / FireFeedback / AimReticle / VehicleAim 문서 교통정리 결과를 테스트 기준에 반영
- FireFeedback.md 신규 Systems 문서를 예정 표현에서 현재 참조 문서로 전환
- WeaponFire.md 예정 표현을 제거하고 현재 Systems 문서 기준으로 정리
- CF-TC-013 / CF-TC-014 / CF-TC-015 관련 문서 참조를 최신화
- OutOfArc / bLocalWithinWeaponArc를 단독 발사 차단 조건이 아닌 조준각 경고/디버그 상태로 명시
- FirePending / FireRejected를 서버 상태가 아닌 로컬 발사 피드백 상태로 정리
- VehicleDebugPanel의 Aim / Weapon 섹션을 전투 검증 기준에 반영
```

### v1.2.0 - 2026-06-19

```text
- 2026-06-19 전투 루프 검증 기준 추가
- CF-TC-013 ~ CF-TC-018 전투/피드백/루프 테스트 후보 추가
- WeaponFire, FireFeedback, HitDamage, CoreLoop 테스트 섹션 추가
- Network 이후 섹션 번호를 전투 루프 테스트 추가에 맞춰 조정
```

### v1.1.0 - 2026-06-18

```text
- 싱글 전환 테스트 기준을 추가
- Dedicated Server / 1클라 / 2클라 / 입력 분리 / 이동 복제 항목을 현재 사이클 N/A로 변경
- VehicleCamera / VehicleAim / AimReticle / VehicleDebug 검증 기준을 싱글 PIE 중심으로 재정렬
- Config 테스트에 CFMPGameMode 기본 실행 경로 제거 확인을 추가
```

### v1.0.0 - 2026-06-02

```text
- TestChecklist 문서 최초 작성
- Systems 기준 최소 회귀 테스트 인덱스 작성
- Vehicles / Input / UI / Network / Config 테스트 섹션 추가
- 테스트 기록 양식과 실패 분류 기준 추가
```

---

## 17. Migration

### v1.9.0 적용 안내

```text
- CF-TC-022는 PASS이며 CF-FQ-025 완료 범위의 최소 회귀 기준으로 사용한다.
- 후속 투사체 착탄 위치 3D 기능은 CF-TC-022의 두 Reticle 의미를 변경하면 안 된다.
- 기존 CF-TC-006, 014, 019와 신규 CF-TC-022의 PASS 상태를 함께 보호한다.
```

### v1.8.7 적용 안내

```text
- CF-TC-021은 구현 전 TODO이며 PASS 또는 PARTIAL로 해석하지 않는다.
- 테스트 기준은 Document/Plan/CombatFxAudio/ImplementationDesign.md를 사용한다.
- 사용자 PIE 확인 전에는 CF-FQ-024 Done 또는 CombatFxAudio Systems 승격을 기록하지 않는다.
- 기존 CF-TC-014, 015, 016, 019, 020의 PASS 상태는 유지한다.
```
