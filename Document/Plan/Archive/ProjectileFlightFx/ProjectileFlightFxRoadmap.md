# Projectile Flight FX Roadmap

- Version: 0.5.0
- Date: 2026-07-28
- Status: Paused / Runtime Integrated / Rocket Thruster PASS / CF-TC-023 Full Matrix Pending
- Feature ID: `CF-FQ-027`
- Representative Plan: `ProjectileFlightFxPlan.md`
- TaskSource: `ProjectileFxTaskSource.md`
- Implementation Reference: `ProjectileFxWorkOrder.md`

---

## 1. 로드맵 목표

투사체 Trail과 추진 화염을 현재 Projectile Actor와 Pool 생명주기에 안전하게 연결하고, 코드·자산·PIE 검증을 작은 단계로 분리한다.

```text
완료된 Projectile 이동·충돌·Damage·Pool
+
완료된 CombatFx 일회성 Muzzle·Impact·Destroyed
→ CF-FQ-027 지속형 Projectile Flight FX
→ CF-FQ-019 반복 전투 확장 회귀
```

---

## 2. 단계 요약

| 단계 | 목표 | 코드 | 자산 | 빌드/테스트 | 현재 상태 |
|---|---|---:|---:|---:|---|
| `PFX-P0-00` | 조사, 설계, 범위, TaskSource와 구현 참고 작업지시 | 없음 | 없음 | 없음 | `Complete` |
| `PFX-P0-01` | ProjectileData 지속형 FX 계약 | 완료 | 없음 | UHT/Compile | `Complete / Build PASS` |
| `PFX-P0-02` | Actor 컴포넌트와 소켓/Fallback 해석 | 완료 | 없음 | RuntimeContract 소스 | `Complete / Build PASS` |
| `PFX-P0-03` | 활성화·비활성화·Pool Reset | 완료 | 없음 | RuntimeContract 소스 | `Complete / Build PASS` |
| `PFX-P0-04` | 공식 Editor 빌드와 자동 회귀 | 보정 완료 | 없음 | Build PASS / Automation Runner Missing | `PARTIAL` |
| `PFX-P0-05` | Niagara 선택, DataAsset 연결, 소켓 배치 | Scale 보정 완료 | Rocket Thruster 저장 완료 / 전체 행렬 자산 Pending | User Rocket Thruster PASS | `PARTIAL` |
| `PFX-P0-06` | 사용자 PIE 통합 검증·튜닝 | 필요 시 보정 | 일부 완료 | CF-TC-024 범위 PASS / CF-TC-023 Pending | `PARTIAL / Paused` |
| `PFX-P0-07` | Systems 런타임 기준 연결·상태 종료 | 없음 | 없음 | Projectile Current System 반영 / 전체 증거 미완료 | `PARTIAL / CF-FQ-027 Paused` |

현재 체크포인트:

```text
- 지속형 FX 런타임은 Document/Systems/Combat/Projectile.md v1.4.0에 반영됨
- DA_PFX_ThrusterTest 실제 발사·Burning 동기화·FX_Exhaust·Scale 0.2 PASS
- CF-TC-024 PASS
- CF-TC-023 전체 Flight FX 행렬은 Pending
- CF-FQ-027은 Paused이며 Done으로 전환하지 않음
- 재개 시 Trail-only, 두 FX 동시, Missing Socket, Hit/LifeExpired Reset, Pool 10회 이상부터 진행
```

---

## 3. PFX-P0-00 — 준비

### 완료 항목

```text
- main_game / plan_repo Git 상태 확인
- AGENTS, CodeWorkGate와 문서 라우팅 확인
- Projectile, CombatFx Current System 확인
- CF-PDL-0010 위치·생명주기 결정 확인
- ACFProjectileActor v1.6.0 실제 컴포넌트와 Pool 경로 확인
- UCFProjectileData v1.5.0 실제 필드 확인
- 별도 CF-FQ-027 기능으로 분리 결정
- C++/Editor 책임 분리
- 허용 경로·보호 범위·완료·실패 조건 확정
- TaskSource 작성
- 사람이 검토 가능한 WorkOrder 초안 작성
```

### 다음 실행 게이트

```text
- 변경 허용 파일과 보호 범위 재확인
- 현재 CFProjectileData와 CFProjectileActor 코드 재확인
- PFX-P0-01 데이터 계약 직접 구현
- 수정 직후 Git diff 검수
- 공식 Editor 빌드와 관련 자동 테스트 실행
```

현재 상태:

```text
Ready — Browser AI Direct Implementation
```

TaskSource와 WorkOrder는 구현 참고 자료이며 최종 Codex YAML이나 plan.* 품질 게이트는 착수 조건이 아니다.

---

## 4. PFX-P0-01 — 데이터 계약

### 작업

```text
- ECFProjectileFxAttachMode 추가
- FCFProjectileAttachedFxSettings 추가
- TrailFxSettings 추가
- ThrusterFxSettings 추가
- 기본값 비활성으로 기존 자산 호환
- BuildProjectileSummary에 짧은 상태 추가
- Blueprint DisplayName·ToolTip 추가
- 파일 버전·Changelog·Migration 갱신
```

### 완료 조건

```text
- 기존 ProjectileData를 재저장하지 않아도 Trail/Thruster가 비활성이다.
- HeaderTool 통과 가능 구조다.
- Niagara 또는 소켓 누락이 유효한 선택적 상태로 표현된다.
```

---

## 5. PFX-P0-02 — 컴포넌트와 부착 해석

### 작업

```text
- TrailOriginComponent
- TrailNiagaraComponent
- ThrusterOriginComponent
- ThrusterNiagaraComponent
- ResolveProjectileFxAttachment 또는 동등한 단일 책임 함수
- 소켓 존재 검사
- CollisionComponent Fallback 재부착
- Scale 상속 분리
- 부착 출처 Debug
```

### 완료 조건

```text
- 두 FX 슬롯이 독립적으로 구성된다.
- 소켓 검색은 활성화 시 한 번만 수행된다.
- Missing Socket은 Fallback으로 처리된다.
- 메시 Scale 때문에 FX Scale이 변형되지 않는다.
```

---

## 6. PFX-P0-03 — Pool 생명주기

### 작업

```text
- ResetProjectileFlightFx
- ApplyProjectileFlightFx
- DeactivateProjectileFlightFx
- ActivateProjectile의 시각 적용 직후 FX 구성
- DeactivateProjectileWithReason의 Pool 반환 전 FX 정리
- InvalidActivation, Manual, Hit, LifeExpired 공용 경로
- Asset 전환과 비활성 슬롯 초기화
```

### 완료 조건

```text
- FX 정리가 FinishDeactivatePolicy보다 앞선다.
- 다음 활성화에서 이전 Ribbon History가 없다.
- Thruster-only와 Trail-only 데이터 전환에서 이전 슬롯이 남지 않는다.
- FX 실패가 Projectile 활성 상태를 바꾸지 않는다.
```

---

## 7. PFX-P0-04 — 자동 검증과 빌드

Automation 후보:

```text
파일: UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp
테스트: CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract
```

검증:

```text
- CDO 컴포넌트 존재
- AutoActivate=false
- DataAsset 기본 비활성
- Missing Socket Fallback
- 비활성화 후 Niagara 정지·Reset
- Pool 재활성화 상태 초기화
```

공식 빌드:

```text
repository_id=main_game
preset_id=carfight.editor.development
```

---

## 8. PFX-P0-05 — Editor 자산 연결

완료된 조사:

```text
- RocketThrusterExhaustFX NiagaraSystem 18/18 AssetDump PASS
- Stylized Attacks NiagaraSystem 14/14 AssetDump PASS
- DA_HeavyShell CFProjectileData 1/1 AssetDump PASS
- Trail 기술 1차 후보: NS_RibbonTrail
- Trail 비교 후보: NS_Flame_Trail
- Thruster 1차 후보: NS_RocketExhaust_Realistic
- Thruster 비교 후보: NS_RocketExhaust_White / NS_RocketExhaust_Yellow
```

현재 Editor 작업:

```text
1. 후보 Niagara의 Loop, Local Space, Spawn Per Unit와 Bounds를 시각 확인
2. 원본 수정이 필요하면 /Game/CarFight/FX/Adapted에 복제
3. DA_HeavyShell은 Trail-only로 설정
4. DA_PFX_ThrusterTest와 DA_PFX_BothTest를 DA_HeavyShell 복제로 준비
5. 기준 Projectile StaticMesh에 FX_Trail 소켓 추가
6. 로켓형 테스트 메시에는 FX_Exhaust 소켓 추가
7. 두 소켓 +X를 꼬리·배기가 뻗는 후방으로 배치
8. 소켓 Transform을 RelativeTransform에 복사해 Missing Socket Fallback 준비
9. 자산 저장 뒤 AssetDump 재검증
```

P0에서는 하나의 기준 Trail과 하나의 기준 Thruster면 충분하다.
모든 탄종에 강제 연결하지 않으며 `DA_HeavyShell`의 Thruster는 기본 비활성으로 유지한다.

---

## 9. PFX-P0-06 — 사용자 PIE

```text
1. 정지 차량에서 단발 확인
2. Trail 위치·방향 확인
3. Thruster 위치·방향 확인
4. 두 FX 동시 확인
5. 소켓 사용 확인
6. 소켓 이름을 의도적으로 누락시켜 Fallback 확인
7. 차량·벽 Hit 종료 확인
8. LifeExpired 종료 확인
9. 같은 Pool Actor 10회 이상 반복
10. 30 FPS와 고속 발사 확인
11. Impact·Damage·Destroyed 회귀 확인
```

FAIL 분류:

| 분류 | 예시 |
|---|---|
| `Core` | Pool 재사용에서 이전 Niagara가 남음 |
| `Data` | 잘못된 소켓 이름이나 Fallback Transform |
| `Asset` | Niagara Local Space, Bounds, Loop 문제 |
| `Quality` | Trail 폭, 밝기, 길이, 밀도 부적합 |
| `Regression` | 첫 Impact 또는 Damage 횟수 변화 |

---

## 10. PFX-P0-07 — 완료 승격

승격 조건:

```text
- Codex 적용 diff 검수 PASS
- 공식 Editor 빌드 PASS
- Automation PASS
- 사용자 PIE PASS
- CF-TC-023 PASS
```

문서 작업:

```text
- Systems/Combat/Projectile.md에 지속형 FX 현재 구현 추가
- Systems/Combat/CombatFx.md의 비책임/연관 구조 갱신
- ProjectSSOT/05_TestChecklist.md에 CF-TC-023 등록
- FeatureQueue CF-FQ-027 Done
- ActiveWork에서 제거
- 다음 후보 CF-FQ-019 복원
```

---

## 11. 보호 게이트

```text
- Projectile 첫 Impact와 Damage 순서를 변경하지 않는다.
- ProjectilePoolComp를 구조 개선 명목으로 재작성하지 않는다.
- CombatFxComp에 Trail/Thruster를 넣지 않는다.
- Tick에 FX 검색·재부착·자산 설정을 추가하지 않는다.
- Audio를 추가하지 않는다.
- TargetSelect 및 unrelated dirty 변경을 수정하지 않는다.
- commit/push/reset/checkout/stash를 수행하지 않는다.
```

---

## 12. Changelog

### v0.5.0 - 2026-07-28

```text
- Projectile Current System에 지속형 FX 런타임, 소켓/Fallback, 독립 FX Scale과 Pool Reset 기준이 반영됐음을 기록했다.
- DA_PFX_ThrusterTest의 Rocket Thruster 사용자 PIE와 Scale 0.2 PASS를 PFX-P0-05~06 부분 완료로 반영했다.
- PFX-P0-07은 Systems 런타임 기준 연결만 부분 완료이며 CF-FQ-027 종료 조건은 충족하지 않은 것으로 구분했다.
- CF-TC-024 추진 검증 PASS와 CF-TC-023 전체 Flight FX 행렬 Pending을 분리했다.
- CF-FQ-027을 Paused로 유지하고 미완료 전체 행렬을 재개 체크포인트로 고정했다.
```

### v0.4.0 - 2026-07-27

```text
- PFX-P0-05 AssetDump 조사에서 Rocket Thruster 18/18, Stylized Attacks 14/14와 DA_HeavyShell 1/1 성공을 확인했다.
- NS_RibbonTrail과 NS_RocketExhaust_Realistic을 각각 Trail·Thruster 기술 1차 후보로 지정했다.
- AssetDump가 Niagara 내부 동작과 DataAsset 저장값을 노출하지 않으므로 시각 승인과 .uasset 저장은 Editor 게이트로 유지했다.
- DA_HeavyShell Trail-only, DA_PFX_ThrusterTest와 DA_PFX_BothTest 분리 운영과 소켓/Fallback 절차를 확정했다.
```

### v0.3.0 - 2026-07-27

```text
- PFX-P0-01 데이터 계약, PFX-P0-02 Actor 부착 구조와 PFX-P0-03 Pool Reset C++ 구현을 완료했다.
- 신규 RuntimeContract 자동화 소스를 추가하고 공식 Editor 빌드에서 컴파일됨을 확인했다.
- 최초 NiagaraComponent include 오류와 메타데이터를 보정한 뒤 Build Job 445848ab0f7749fcab8188b164bb1487 Exit Code 0을 최종 기록했다.
- Unreal Automation 실행 도구가 현재 Admin 표면에 없어 PFX-P0-04를 Build PASS / Automation Not Run의 PARTIAL로 기록했다.
- 현재 다음 단계를 PFX-P0-05 Editor Niagara 선택, DataAsset 연결과 소켓 배치로 변경했다.
```

### v0.2.0 - 2026-07-27

```text
- CarFight CodeWorkGate v2.0 직접 구현 정책을 적용했다.
- PFX-P0-00의 Final YAML Blocked 상태를 해제했다.
- PFX-P0-01부터 현재 AI 세션이 직접 구현하도록 실행 게이트를 변경했다.
- TaskSource와 WorkOrder를 필수 착수 산출물에서 구현 참고 자료로 재분류했다.
```

### v0.1.0 - 2026-07-27

```text
- CF-FQ-027의 PFX-P0-00~07 단계 로드맵을 생성했다.
- 설계·TaskSource·WorkOrder 초안 준비와 당시 plan.* 품질 게이트를 분리했다.
- 데이터 계약, Actor 부착, Pool Reset, Automation, 자산 연결, PIE와 Systems 승격 순서를 확정했다.
- 당시 PFX-P0-00을 Prepared / Final YAML Blocked로 기록했다.
```

---

## 13. Migration

```text
- 새 세션에서 CF-FQ-027을 Active 또는 Done으로 자동 복원하지 않는다.
- 지속형 FX 런타임의 현재 구현 판단은 Document/Systems/Combat/Projectile.md를 우선한다.
- PFX-P0-01~03 C++ 기반과 Scale 보정을 다시 구현하지 않는다.
- CF-TC-024의 Rocket Thruster PASS를 CF-TC-023 전체 행렬 PASS로 확대 해석하지 않는다.
- 사용자가 CF-FQ-027을 선택하면 PFX-P0-06의 미완료 전체 행렬부터 재개한다.
- Automation 실제 실행, Trail-only, 동시 FX, Missing Socket과 장시간 Pool 검증 전에는 CF-FQ-027을 Done으로 전환하지 않는다.
```
