# Launcher / Missile Roadmap

- Version: 0.6.0
- Date: 2026-07-29
- Status: Active / LM-P0-06 Launcher Integration PIE Active / MG-P0-00 Inactive Foundation Code Applied·Build PASS / Missile Runtime Not Connected
- Active Feature: `CF-FQ-029`
- Dependent Feature: `CF-FQ-030`

---

## 1. 로드맵 목표

런처와 미사일을 한 번에 결합 구현하지 않고, 기존 직사 무기 회귀를 보호하는 작은 빌드 가능 단계로 나눈다.

```text
현재 Projectile / Aim / Turret / Pool / Propulsion
→ Launch Handoff
→ Multi-Muzzle Direct Launcher
→ Ripple / Salvo
→ Angled / Vertical Ejection
→ Missile Flight State
→ Physics-Limited Guidance
→ Laser / Attack Profile
→ 반복 전투 통합 검증
```

---

## 2. 전체 단계

| ID | 기능 | 목표 | 상태 |
|---|---|---|---|
| `LM-P0-00` | 조사·설계·문서 | 책임, 계약, 로드맵과 작업지시 확정 | Done |
| `LM-P0-01` | Launch Handoff | 목표와 초기 발사 상태 분리, Legacy Direct 호환 | Code Applied / Build PASS |
| `LM-P0-02` | Launcher Data | 가변 Muzzle와 Direct SingleCycle | Code Applied / Build PASS / Editor Validation Pending |
| `LM-P0-03A` | Volley Data | SingleCycle / Ripple / Salvo 데이터와 정책 계약 | Code Applied / Build PASS |
| `LM-P0-03B` | Volley Runtime | Ripple / Salvo Scheduler와 쿨다운·취소 정책 | Code Applied / Build PASS / Editor Validation Pending |
| `LM-P0-04` | Release Mode | Angled / Vertical Ejection과 차량 속도 상속 | Code Applied / Build PASS |
| `LM-P0-05` | Launcher Editor | DA_RocketBody·DA_RocketLauncher·Preset·TestSUV·TestMap·기본 Pawn 연결 | Assets Applied / Independent AssetDump PASS |
| `LM-P0-06` | Launcher Verification | CF-TC-025 / 026, PIE·Pool·Release 회귀 | Active / User PIE Pending |
| `MG-P0-00` | Missile Foundation | Flight / Guide 타입, 비활성 Config와 제한형 Guidance 수학 | Inactive Foundation Code Applied / Build PASS / Runtime Disabled |
| `MG-P0-01` | Flight State | Ejection→Clearance→Transition→GuidedFlight | Pending / Blocked by LM-P0-06 |
| `MG-P0-02` | Thrust Direction | Rocket Fixed와 Missile Current 방향 분리 | Pending |
| `MG-P0-03` | Target Guidance | 제한된 횡가속도와 선회율 | Pending |
| `MG-P0-04` | Target Lifetime | 발사 시 Snapshot, 목표 상실과 오버슈트 | Pending |
| `MG-P0-05` | Laser Guidance | 외부 Laser Point 입력과 상실 정책 | Pending |
| `MG-P0-06` | Attack Profile | Direct / Loft / Pitch-Over / Top-Attack | Pending |
| `MG-P0-07` | Missile Verification | CF-TC-027, 실패 가능성과 실제 충돌 검증 | Pending |
| `LMG-P1-01` | Integration | 주행·다연장·미사일 반복 전투 회귀 | Pending |

---

## 3. LM-P0-00 — 준비 완료

완료 항목:

```text
- main_game / plan_repo Git 상태 확인
- AGENTS / CodeWorkGate / Document Entry 확인
- 현재 Projectile, Motor, Pool, Aim, Turret 구조 확인
- RocketLauncherPitch Muzzle_1~4 리소스 확인
- 런처와 미사일 분리 결정
- Ejection ProjectileMovement 기본 결정
- 유도 명중 비보장 계약 확정
- 발사 후 런처 독립 계약 확정
- 설계·상세 Plan 작성
- TaskSource / WorkOrder 작성
```

상태:

```text
Documentation Complete
Source Not Modified
Code Entry Ready
```

---

## 4. LM-P0-01 — Launch Handoff

목표:

```text
현재 Direct Fire 결과를 바꾸지 않고
Command Target과 Initial Launch State를 별도 데이터로 전달한다.
```

계획 작업:

```text
- CFProjectileLaunchTypes.h 추가
- ECFProjectileReleaseMode 최소 enum 추가
- FCFProjectileLaunchContext 추가
- ProjectilePool Acquire Context 경로 추가
- ProjectileActor Activate Context 경로 추가
- 기존 호출 호환 어댑터 유지
- Direct Fire Context 빌더 추가
- RuntimeContract Automation 추가
```

완료 조건:

```text
- 기존 단일 Muzzle Heavy Cannon / Rocket 위치·방향·속도 회귀 없음
- 기존 Damage / Pool / FX 회귀 없음
- 신규 필드 기본값 안전
- 공식 Editor 빌드 PASS
- Automation PASS 또는 실행 불가 사유 명시
```

---

## 5. LM-P0-02 — 가변 Muzzle와 SingleCycle

계획 작업:

```text
- UCFLauncherData 또는 동등 데이터 계층
- MuzzleSocketNames 배열
- Legacy MuzzleSocketName Fallback
- NextMuzzleIndex
- 발사 성공 시에만 Index 진행
- 배열 순환
- Muzzle별 FireOrigin / Block Trace / FX
```

검증:

```text
- 1 / 2 / 4 / 6개 Muzzle
- Missing Socket
- 배열 Empty Legacy Fallback
- 각 Muzzle 동일 Command Target 조준
```

---

## 6. LM-P0-03A~03B — Ripple / Salvo

현재 상태:

```text
- FCFLauncherFirePatternConfig 적용
- SingleCycle / Ripple / Salvo 데이터 계약 적용
- UCFLauncherComp Runtime Scheduler 적용
- 첫 입력 순간 Command Target 고정
- ContinueRemaining / StopSequence 실패 정책 적용
- 차량 파괴·무기 변경·터렛 변경 취소 적용
- FirstAcceptedProjectile / SequenceCompleted 쿨다운 정책 적용
- 공식 Editor Build Job 58761c19b152486ba783baf57526f9e5 / Exit Code 0
- Automation Source Compile PASS / 실행 Not Run
- 실제 Muzzle·Ripple·Salvo PIE Pending
```

구현 항목:

```text
- ECFLauncherFirePattern
- ProjectileCountPerTrigger
- InterMuzzleDelaySeconds
- Volley ID와 진행 상태
- 입력 중복 정책
- Volley 단위 쿨다운
- 발사 중 장비 변경·파괴·비활성화 취소
```

검증:

```text
- SingleCycle
- 4발 Ripple
- 4발 Salvo
- Pool 부족
- 중간 MuzzleBlocked
- Volley 중 Pawn 파괴
```

---

## 7. LM-P0-04 — Angled / Vertical Ejection

현재 상태:

```text
- FCFLauncherReleaseConfig 적용
- Direct / AngledEjection / VerticalEjection 적용
- LocalEjectionDirection과 실제 Muzzle Transform 월드 변환 적용
- EjectionSpeed와 ProjectileData.InitialSpeed fallback 적용
- CarrierVelocityRatio와 발사 순간 차량 World Velocity 상속 적용
- CommandTargetLocation과 InitialLaunchDirection 분리 적용
- 비Direct 실제 InitialLaunchDirection 안전 검사 적용
- 같은 Launch Context를 안전 검사와 Projectile 활성화에 전달
- 기존 ProjectileMovement GravityScale·Sweep·Sub-step 유지
- CFLauncherReleaseTests.cpp Source Compile PASS / 실행 Not Run
- 최종 Editor Build Job acd575ffac2f4d64bd73194e160a3f62 / Exit Code 0
```

Editor 검증:

```text
- 정지 차량
- 전진·후진·회전 차량
- Angled 포물선
- Vertical 상승
- 차량 속도 상속
- 런처와 즉시 독립
- 사출 방향 앞 장애물 MuzzleBlocked
```

---

## 8. LM-P0-05~06 — Editor와 런처 완료

현재 단계:

```text
LM-P0-05 Launcher Editor Assets: Done / Assets Applied / Independent AssetDump PASS
LM-P0-06 Launcher Verification: Active / CF-TC-025·026 User PIE Pending
```

Editor 대상:

```text
/Game/CarFight/Weapons/Turrets/RocketLauncher
/Game/CarFight/Weapons/Data/TurretMounts/DA_RocketBody
RocketLauncherPitch
Muzzle_1
Muzzle_2
Muzzle_3
Muzzle_4
런처용 WeaponData LauncherFirePatternConfig
런처용 WeaponData LauncherReleaseConfig
```

계획 테스트:

```text
CF-TC-025 Launch Handoff Regression
CF-TC-026 Modular Launcher / Ejection
```

완료 전에는 `CF-FQ-029`를 Done 또는 Systems Current로 승격하지 않는다.

---

## 9. MG-P0-00~02 — 미사일 비행 기반

MG-P0-00 현재 완료 범위:

```text
- CFMissileFlightTypes / CFMissileGuideTypes
- FCFMissileFlightConfig / FCFMissileGuideConfig
- Flight·Guide Snapshot과 Guidance Input·Command
- CFMissileGuideMath 제한형 비례항법 순수 계산
- UCFProjectileData 중첩 Config와 Foundation Summary
- bUseMissileFlight=false / bUseGuidance=false / GuideMode=None
- CFMissileFoundationTests Source Compile PASS
- Editor Build Job 8c4f952fa58f499b9145b95caa1ff926 / Exit Code 0
```

MG-P0-00 제외·보호 범위:

```text
- UCFMissileFlightComp / UCFMissileGuideComp 미생성
- ProjectileActor / ProjectileMovement / MotorComp 런타임 미연결
- Target Actor / Laser Point 미전달
- .uasset 변경 없음
- 기존 Projectile과 Rocket 동작 유지
```

MG-P0-01~02는 LM-P0-06 사용자 PIE와 CF-FQ-029 완료 판정 뒤 시작한다.

---

## 10. MG-P0-03~04 — 대상 유도

계획:

```text
- Target Actor Snapshot
- Seeker FOV
- MaximumTurnRate
- MaximumLateralAcceleration
- Guidance Response
- Target Lost Grace
- ContinueStraight 상실 정책
```

명중 보장 금지 검증:

```text
- 최소 사거리 안쪽 목표
- 고속 측면 기동
- 뒤쪽 목표
- 오버슈트
- 목표 파괴
- 장애물 가림
- 속도 부족
```

---

## 11. MG-P0-05~06 — Laser와 공격 프로파일

Laser:

```text
- 외부 Designator Source
- 유효 Laser Point
- 신호 상실
- 이동은 Missile 자체 물리 제한 유지
```

Attack Profile:

```text
Direct
Loft
PitchOver
TopAttack
```

프로파일은 목표점으로 순간 보정하는 스크립트가 아니라 Flight State의 중간 목표와 전환 조건을 제공한다.

---

## 12. MG-P0-07 — 미사일 완료 검증

계획 테스트:

```text
CF-TC-027 Physics-Limited Missile Guidance
```

PASS 기준:

```text
- 런처 회전과 차량 Target 변경이 기존 미사일 경로를 바꾸지 않음
- 최대 선회율·횡가속도 초과 없음
- 성공·실패 요격이 모두 발생 가능
- 실제 충돌에서만 Damage
- Target Lost 후 정책대로 비행
- Pool 재사용 상태 오염 없음
```

---

## 13. 보호 게이트

```text
- LM-P0-01에서 Multi-Muzzle를 함께 구현하지 않는다.
- LM-P0-02에서 Ripple / Salvo를 함께 구현하지 않는다.
- Launcher 완료 전 Guidance를 구현하지 않는다.
- Guidance에서 위치 직접 이동을 사용하지 않는다.
- 기존 Rocket Motor를 한 번에 재작성하지 않는다.
- 기존 .uasset을 C++ 단계에서 자동 변경하지 않는다.
- Audio를 추가하지 않는다.
```

---

## 14. Changelog

### v0.6.0 - 2026-07-29

```text
- CF-FQ-030 Ready 상태 안에서 MG-P0-00 비활성 Foundation 타입·Config·Guidance 수학을 적용했다.
- UCFProjectileData v1.8.0의 두 Missile Config는 기본 비활성이며 기존 런타임에 연결하지 않았다.
- CFMissileFoundationTests 소스 컴파일과 Editor Build Job 8c4f952fa58f499b9145b95caa1ff926 / Exit Code 0을 기록했다.
- 현재 Active는 계속 CF-FQ-029 LM-P0-06이며 MG-P0-01 Flight State 런타임은 해당 사용자 PIE 뒤로 유지했다.
```

### v0.5.0 - 2026-07-29

```text
- LM-P0-05 Launcher Editor Assets와 실제 플레이 진입 경로 연결을 완료했다.
- DA_RocketBody Muzzle 1→4→2→3, DA_RocketLauncher Ripple 4발·0.15초, RocketLauncher Preset과 DA_TestSUV 연결을 저장했다.
- TestMap의 SUV Launcher와 Sedan HeavyCannon 분리, BP_CFVehiclePawn CDO의 DA_TestSUV 연결을 확인했다.
- LauncherPost 10/10, VehiclePost 3/3, MapPost 19/19와 CDO 재검증을 PASS로 기록했다.
- LM-P0-06 CF-TC-025·026 사용자 PIE를 Active로 이동했으며 테스트는 TODO를 유지한다.
```

### v0.4.0 - 2026-07-29

```text
- LM-P0-04 Direct·Angled·Vertical Release, 차량 속도 상속과 실제 사출 방향 안전 검사 적용을 반영했다.
- ReleaseContract 소스 컴파일과 최종 Editor Build Job acd575ffac2f4d64bd73194e160a3f62 / Exit Code 0을 기록했다.
- LM-P0-05 Launcher Editor Assets를 현재 Active 단계로 이동했다.
- 실제 Muzzle·Ripple·Salvo·Ejection PIE는 Pending으로 유지했다.
```

### v0.3.0 - 2026-07-29

```text
- LM-P0-03을 03A Pattern Data Contract와 03B Runtime Scheduler로 분리했다.
- LM-P0-01~03B Code Applied와 최신 Editor Build PASS를 반영했다.
- LM-P0-04 Angled·Vertical Ejection을 현재 Code Entry 단계로 지정했다.
- 실제 Muzzle·Ripple·Salvo PIE는 Pending으로 유지했다.
```

### v0.1.0 - 2026-07-28

```text
- CF-FQ-029과 CF-FQ-030을 선행 관계가 있는 두 기능으로 분리했다.
- LM-P0-00~06과 MG-P0-00~07 단계 로드맵을 생성했다.
- 첫 코드 작업을 Launch Handoff Foundation으로 제한했다.
- 가변 Muzzle, Volley, Ejection, Flight State, Guidance와 Attack Profile을 순차 단계로 분리했다.
- 계획 테스트 CF-TC-025~027을 예약했다.
```

---

## 15. Migration

```text
- 새 세션은 LM-P0-01~05와 MG-P0-00 비활성 Foundation을 반복하지 않고 LM-P0-06에서 시작한다.
- LM-P0-00 조사와 문서 설계를 반복하지 않는다.
- CF-TC-025·026 사용자 PIE 결과 전에는 LM-P0-06, CF-FQ-029와 Systems 승격을 완료 처리하지 않는다.
- CF-FQ-030은 Ready를 유지하고 MG-P0-01 런타임은 LM-P0-06 Direct·Ejection Launcher 통합 검증과 CF-FQ-029 완료 판정 전에는 시작하지 않는다.
- 단계별 공식 빌드와 회귀를 생략하지 않는다.
```
