# Projectile Flight FX Implementation Design

- Version: 1.0.1
- Date: 2026-08-02
- Status: Completed / Systems Current / CF-FQ-027 Done / CF-TC-023 User PIE PASS
- Feature ID: `CF-FQ-027`
- Feature Name: 투사체 비행 FX
- Planned Test ID: `CF-TC-023`
- Representative Plan: `Document/Plan/ProjectileFlightFxPlan.md`
- Roadmap: `Document/Plan/ProjectileFlightFxRoadmap.md`
- TaskSource: `Document/Plan/ProjectileFxTaskSource.md`
- Implementation Reference: `Document/Plan/ProjectileFxWorkOrder.md`

---

## 현재 작업 체크포인트

```text
현재 단계: Completed / Full Flight FX Matrix User PIE PASS
기능 상태: CF-FQ-027 Done
현재 구현 기준: Document/Systems/Combat/Projectile.md v1.5.0
구현 방식: Browser AI Direct
실제 소스 변경: Applied / UCFProjectileData v1.7.1 / ACFProjectileActor v1.8.1
Unreal 자산 변경: Applied by User Editor / Trail·Thruster 테스트 DataAsset, FX_Trail·FX_Exhaust 소켓과 Fallback 설정
공식 Editor 빌드: PASS
Foundation Build Job: 445848ab0f7749fcab8188b164bb1487 / Exit Code 0
Scale 보정 최종 Build Job: e8b812bd479549299dd116f9bae8996f / Exit Code 0
Automation 소스: CFProjectileFlightFxTests.cpp v1.1.0 / Editor 빌드 컴파일 PASS
Automation 실행: PASS — `UE/Saved/Automation/CombatRuntime/index.json`에서 `CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract` Success / 초기 2026-07-30 Not Run은 당시 체크포인트 이력 / 실행 수단은 작업 전용 임시 경로 / 공용 재실행 진입점은 정의하지 않음
AssetDump: RocketThrusterExhaustFX 18/18 PASS, Stylized Attacks Niagara 14/14 PASS, DA_HeavyShell 1/1 PASS
검증된 Trail: /Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_RibbonTrail
검증된 Thruster: /Game/RocketThrusterExhaustFX/FX/NS_RocketExhaust_Realistic
사용자 PIE 확인: Trail-only / Thruster-only / Trail+Thruster / 유효 소켓 / Missing Socket Fallback / Hit·LifeExpired Reset / Pool 20발 이상 / Ribbon History 무잔류 / 30 FPS 고속 Bounds / 기존 Impact·Damage·Pool 회귀 PASS
Current System 반영: 지속형 FX 런타임, 소켓/Fallback, 독립 FX Scale과 Pool Reset 계약
미완료 전체 행렬: 없음
CF-TC-023: PASS
TaskSource: Document/Plan/ProjectileFxTaskSource.md
구현 참고 작업지시: Document/Plan/ProjectileFxWorkOrder.md
Plan 역할: 완료 당시 설계·구현·빌드·PIE 기록 보존
코드 재작성: 금지 — 후속 변경은 Document/Systems/Combat/Projectile.md 현재 계약을 기준으로 별도 작업 등록
```

`Document/CodeWorkGate.md` v2.0에 따라 현재 AI 세션이 PFX-P0-01부터 실제 코드 수정, diff 검수, 빌드와 자동 테스트를 직접 수행한다.
TaskSource와 WorkOrder는 구현 요구사항과 보호 범위를 제공하는 참고 문서이며 최종 Codex YAML은 착수 조건이 아니다.

---

## 1. 목적

현재 `ACFProjectileActor`가 소유하는 발사체 활성화·비활성화·Pool 재사용 생명주기에 Trail과 추진 화염 Niagara를 연결한다.

핵심 목표는 다음 한 문장으로 고정한다.

> 실제 Projectile Actor가 비행하는 동안 Trail과 추진 화염을 데이터 기반으로 재생하고, 충돌·수명 종료·수동 종료·활성화 실패에서 FX를 확실히 초기화해 다음 Pool 재사용에 이전 FX 상태가 남지 않게 한다.

이 기능은 완료된 `CF-FQ-024 전투 FX`를 다시 여는 작업이 아니다.
`CF-FQ-024`는 Muzzle, 첫 Impact와 최초 Destroyed의 일회성 FX Current System으로 유지한다.
`CF-FQ-027`은 Projectile Actor에 부착되어 활성화 동안 유지되는 지속형 비행 FX만 담당한다.

---

## 2. 선행 기준

### 2.1 현재 Systems

```text
Document/Systems/Combat/Projectile.md
- ProjectileData 적용
- 공통 Projectile Actor
- Sweep / Sub-step / 보조 연속 Sweep
- 첫 Impact 단일 처리
- Pool 확보·반환 생명주기

Document/Systems/Combat/CombatFx.md
- 승인 Fire, 첫 Impact, 최초 Destroyed 일회성 Niagara
- Projectile Trail과 추진 화염은 현재 비책임
```

### 2.2 프로젝트 결정

`CF-PDL-0010`의 다음 결정을 그대로 적용한다.

```text
- 메시에 고정된 FX 위치는 Mesh Socket을 우선한다.
- 소켓이 없으면 ProjectileData의 Fallback Relative Transform을 사용한다.
- Projectile Trail과 추진 화염은 ACFProjectileActor가 소유한다.
- Projectile Pool 반환 전에 지속형 Niagara를 정지·초기화한다.
- P0에서는 Trail 자연 소멸, 별도 FX Pool과 다중 노즐을 구현하지 않는다.
```

### 2.3 현재 코드 기준

현재 `ACFProjectileActor v1.7.0` 컴포넌트 구성:

```text
CollisionComponent: USphereComponent
├─ MeshComponent: UStaticMeshComponent
├─ TrailOriginComponent
│  └─ TrailNiagaraComponent
└─ ThrusterOriginComponent
   └─ ThrusterNiagaraComponent

ProjectileMovementComponent: UProjectileMovementComponent
```

현재 `UCFProjectileData v1.6.0`은 이동·충돌·메시·Impact·DamageData에 더해 `TrailFxSettings`와 `ThrusterFxSettings`를 제공한다. 두 슬롯은 기본 비활성이고 각각 `FX_Trail`, `FX_Exhaust` 소켓을 기본값으로 사용한다.

Niagara 모듈 의존성은 기존 CombatFx 구현으로 `CarFight_Re.Build.cs`에 이미 반영되어 있다.
실제 구현 시 컴파일이 이를 부정하지 않는 한 Build.cs를 다시 수정하지 않는다.

---

## 3. 범위

### 3.1 P0 필수 범위

```text
1. Trail과 Thruster를 독립 슬롯으로 지원한다.
2. 두 슬롯은 동시에 활성화할 수 있다.
3. 각 슬롯은 Niagara System, 부착 방식, 소켓 이름과 Fallback Transform을 데이터로 가진다.
4. 메시 소켓이 유효하면 소켓 위치·회전을 사용한다.
5. 소켓이 없으면 Actor 기준 Fallback Transform으로 안전하게 전환한다.
6. NiagaraComponent는 Projectile Actor가 생성 시 한 번 만들고 Pool과 함께 재사용한다.
7. Projectile 활성화마다 이전 Niagara 상태를 초기화하고 새 데이터로 시작한다.
8. Hit, LifeExpired, Manual, InvalidActivation에서 Pool 반환 전에 두 FX를 정지·초기화한다.
9. FX 누락이나 소켓 누락은 이동, 충돌, 피해와 Pool 반환을 실패시키지 않는다.
10. 부착 출처와 활성 상태를 Debug에서 확인할 수 있다.
```

### 3.2 명시적 제외 범위

```text
- Trail 자연 소멸을 위한 NiagaraComponent Detach
- 충돌 후 독립 잔류 Trail Actor 또는 별도 FX Pool
- 다중 노즐 배열과 FX_Exhaust_01~N 자동 검색
- 추진 모터 연소 시간과 관성 비행 단계
- 속도·연료·가속도에 따른 추진 출력 곡선
- 표면별 Impact, Decal과 파편
- HitScan Tracer 일반화
- 네트워크 복제와 원격 클라이언트 FX 동기화
- 서버 권한 Projectile 변경
- 모든 외부 Niagara를 위한 범용 Scale 모듈
- 게임 사운드와 AudioComponent
- FAB 원본 Niagara 직접 수정
```

---

## 4. C++와 Unreal Editor 책임 분리

### 4.1 C++ 책임

```text
- 데이터 구조와 안전 기본값 정의
- Trail/Thruster NiagaraComponent 생성과 소유
- 소켓 존재 여부와 Fallback 부착 해석
- 활성화 시 이전 상태 Reset, Asset 지정, Transform 적용과 Activate
- 비활성화 시 DeactivateImmediate, Reset과 Pool 반환 전 정리
- Dedicated Server와 Null 상태에서 FX만 건너뛰는 안전 처리
- 기존 Projectile 이동·충돌·피해·Pool 순서 보존
- Debug 요약과 부착 출처 제공
- Automation 가능한 계약 검증
```

### 4.2 Unreal Editor와 DataAsset 책임

```text
- 실제 Trail Niagara 선택
- 실제 추진 화염 Niagara 선택
- Trail은 World Space 잔류 또는 Spawn Per Unit 특성이 적절한지 확인
- 추진 FX는 부착 방향과 Local Space 동작 확인
- Projectile StaticMesh에 필요하면 FX_Trail 또는 FX_Exhaust 소켓 추가
- 소켓이 없는 자산의 Fallback Relative Transform 조정
- 기준 ProjectileData에 TrailFxSettings와 ThrusterFxSettings 저장
- FAB 원본이 부적합하면 /Game/CarFight/FX/Adapted 복제본 사용
- 최종 PIE에서 크기, 방향, Bounds와 잔류 여부 확인
```

Blueprint에 활성화·비활성화 판정이나 Pool 생명주기를 중복 구현하지 않는다.

---

## 5. 권장 컴포넌트 구조

```text
ACFProjectileActor
└─ CollisionComponent
   ├─ MeshComponent
   ├─ TrailOriginComponent
   │  └─ TrailNiagaraComponent
   └─ ThrusterOriginComponent
      └─ ThrusterNiagaraComponent
```

### 5.1 Origin Component를 두는 이유

```text
- FX 부착 기준을 메시 소켓과 Actor Fallback 사이에서 일관되게 전환할 수 있다.
- NiagaraComponent 자체에 소켓·Fallback 분기 책임을 집중시키지 않는다.
- Trail과 추진 FX가 동시에 존재해도 독립 Transform을 유지한다.
- Debug에서 각 슬롯의 실제 부착 출처를 구분할 수 있다.
```

### 5.2 Scale 상속 규칙

`MeshComponent`에는 `ProjectileMeshRelativeScale`이 적용된다.
FX가 메시의 비균일 Scale을 그대로 상속하면 Ribbon 폭이나 화염 모양이 찌그러질 수 있다.

권장 규칙:

```text
- Origin은 소켓 위치와 회전은 따른다.
- FX Scale은 메시 표시 Scale과 분리한다.
- Origin 또는 NiagaraComponent에서 절대 Scale을 사용해 최종 Scale 1,1,1을 기본으로 유지한다.
- 시각 크기 조정은 Niagara 자산 또는 후속 명시적 FX Scale 필드에서 수행한다.
```

P0 데이터에는 복잡한 Scale 전달 모드를 추가하지 않는다.
실제 후보가 Scale 조정을 반드시 요구할 때만 후속 필드로 확장한다.

---

## 6. 데이터 계약

### 6.1 부착 방식 enum

예정 타입:

```text
ECFProjectileFxAttachMode
- ProjectileRelative
- MeshSocketWithFallback
```

| 값 | 의미 |
|---|---|
| `ProjectileRelative` | CollisionComponent 기준 RelativeTransform만 사용한다. |
| `MeshSocketWithFallback` | Mesh 소켓을 우선하고 누락 시 RelativeTransform으로 전환한다. |

### 6.2 공용 설정 구조체

예정 타입:

```text
FCFProjectileAttachedFxSettings
```

| 필드 | 타입 | 의미 |
|---|---|---|
| `bEnabled` | `bool` | 이 FX 슬롯을 사용할지 결정한다. 기본값 false다. |
| `NiagaraSystem` | `UNiagaraSystem*` | 재생할 지속형 Niagara다. 비어 있으면 FX만 생략한다. |
| `AttachMode` | `ECFProjectileFxAttachMode` | Actor Fallback 전용 또는 소켓 우선 방식을 선택한다. |
| `AttachSocketName` | `FName` | 메시에서 찾을 소켓 이름이다. |
| `RelativeTransform` | `FTransform` | Actor 기준 Fallback 위치·회전·스케일이다. |

모든 Blueprint 노출 필드에는 한국어 `DisplayName`과 `ToolTip`을 제공한다.

### 6.3 ProjectileData 슬롯

```text
TrailFxSettings
- 기본 bEnabled=false
- 기본 AttachSocketName=FX_Trail

ThrusterFxSettings
- 기본 bEnabled=false
- 기본 AttachSocketName=FX_Exhaust
```

기존 ProjectileData는 두 슬롯 기본값이 비활성이라 재저장 없이 기존 동작을 유지한다.

### 6.4 명명 결정

추진 소켓 이름은 `FX_Thruster`가 아니라 프로젝트 결정에 이미 사용 중인 `FX_Exhaust`를 기준으로 한다.

```text
Trail 기본 소켓: FX_Trail
추진 기본 소켓: FX_Exhaust
```

소켓의 `+X`축은 FX 방출 방향이다.
미사일 진행 방향과 배기 분출 방향이 반대라면 소켓 회전 또는 DataAsset Fallback 회전으로 표현한다.

---

## 7. 런타임 생명주기

### 7.1 생성자

```text
- TrailOriginComponent 생성
- TrailNiagaraComponent 생성
- ThrusterOriginComponent 생성
- ThrusterNiagaraComponent 생성
- AutoActivate=false
- AutoDestroy=false
- 초기 Asset 없음
- 초기 비활성 상태
```

컴포넌트는 Projectile 발사마다 생성·파괴하지 않는다.
Projectile Actor와 함께 Pool에서 재사용한다.

### 7.2 Projectile 활성화

권장 순서:

```text
ActivateProjectile 시작
→ 이전 발사 Ignore/Debug 상태 정리
→ ActiveProjectileData 저장
→ Actor 회전 확정
→ ApplyProjectileVisual
→ ResetProjectileFlightFx
→ ApplyProjectileFlightFx
   → Trail 설정 해석
   → Thruster 설정 해석
   → 소켓 또는 Fallback 부착
   → Niagara Asset 지정
   → Reinitialize/Activate
→ ApplyProjectileCollision
→ ApplyProjectileMovement
→ 수명 타이머 예약
→ Actor 표시·충돌·Tick 활성
```

FX 적용 실패는 활성화 전체 실패로 전환하지 않는다.

### 7.3 Projectile 비활성화

다음 모든 사유에서 같은 정리 함수를 통과한다.

```text
InvalidActivation
Manual
Hit
LifeExpired
```

권장 순서:

```text
수명 타이머 정리
→ Debug 결과 보존
→ Projectile Tick 정지
→ Movement 정지
→ Collision 비활성
→ DeactivateProjectileFlightFx
   → Trail DeactivateImmediate
   → Trail ResetSystem
   → Thruster DeactivateImmediate
   → Thruster ResetSystem
   → 부착 출처 Debug 갱신
→ Actor 숨김
→ ActiveProjectileData 해제
→ Pool 반환 또는 Destroy
```

`FinishDeactivatePolicy()`보다 반드시 먼저 지속형 FX가 정리되어야 한다.

### 7.4 Pool 재활성화

```text
- 이전 Ribbon History가 없어야 한다.
- 이전 Niagara User Parameter가 다음 Projectile에 남지 않아야 한다.
- 이전 소켓 부착이 다른 메시의 잘못된 소켓으로 유지되지 않아야 한다.
- 이전 Trail 자산이 Thruster-only Projectile에 남지 않아야 한다.
- 두 슬롯이 비활성인 ProjectileData로 재사용할 때 두 Niagara 모두 정지 상태여야 한다.
```

---

## 8. 부착 해석

### 8.1 소켓 우선

```text
AttachMode=MeshSocketWithFallback
+ MeshComponent 유효
+ AttachSocketName 유효
+ Mesh가 소켓 보유
→ Origin을 MeshComponent의 해당 소켓에 Snap
→ 위치·회전 소켓 사용
→ Scale은 FX 독립 기준 유지
→ Debug Source=MeshSocket:<SocketName>
```

### 8.2 Fallback

```text
ProjectileRelative 모드
또는
소켓 이름 없음
또는
메시 없음
또는
메시가 소켓을 보유하지 않음
→ Origin을 CollisionComponent에 부착
→ Settings.RelativeTransform 적용
→ Debug Source=ProjectileRelative 또는 MissingSocketFallback:<SocketName>
```

소켓 검색은 활성화 또는 메시 변경 시 한 번 수행한다.
매 Tick 검색하지 않는다.

### 8.3 FX 누락

```text
bEnabled=false
→ Disabled

bEnabled=true + NiagaraSystem=null
→ MissingOptionalSystem
```

두 경우 모두 발사체 이동과 충돌은 정상 진행한다.

---

## 9. Niagara 콘텐츠 기준

### 9.1 Trail

```text
- 고속 이동에서 Spawn Rate만으로 끊기면 Spawn Per Unit 또는 거리 기반 Ribbon을 우선한다.
- 생성된 꼬리 점이 발사체와 함께 끌려오지 않도록 Local Space 사용 여부를 확인한다.
- P0에서는 충돌 순간 즉시 정지하며 자연 소멸은 요구하지 않는다.
- Impact FX가 종료 지점을 시각적으로 덮는지 확인한다.
- Fixed Bounds 또는 동적 Bounds가 고속 이동에서 갑자기 컬링되지 않는지 확인한다.
```

### 9.2 Thruster

```text
- 발사체에 Attached 상태로 유지한다.
- FX_Exhaust +X 방향과 Niagara 방출 방향을 일치시킨다.
- 로켓 노즐 위치가 메시마다 다르면 소켓을 사용한다.
- 일반 포탄처럼 정밀 노즐이 필요하지 않으면 Fallback Transform을 사용한다.
- P0에서는 Projectile 수명 전체를 추진 활성 시간으로 본다.
```

### 9.3 탄종별 기본 표현

| 탄종 | 권장 P0 표현 |
|---|---|
| 기관총탄 | 짧은 Tracer 또는 별도 후속 HitScan 표현. 장기 연기 Trail은 기본 적용하지 않는다. |
| 기관포탄 | 얇은 Trail 또는 밝은 짧은 Ribbon 후보. |
| 대구경 포탄 | Trail 선택 적용. 추진 FX는 보통 비활성. |
| 로켓 | Trail과 Thruster 동시 활성. |
| 미사일 | Trail과 Thruster 동시 활성. 유도·연소 단계는 후속. |
| 에너지탄 | 에너지 Ribbon 후보. 현재 Feature에서는 별도 에너지 규칙을 만들지 않는다. |

### 9.4 PFX-P0-05 AssetDump 조사 결과

2026-07-27 AssetDump 재검증 결과:

```text
/Game/RocketThrusterExhaustFX
- NiagaraSystem 18개
- 성공 18 / 실패 0

/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems
- NiagaraSystem 14개
- 성공 14 / 실패 0

/Game/CarFight/Weapons/Data/ProjectileDefs
- DA_HeavyShell 1개
- 성공 1 / 실패 0
```

현재 AssetDump 기본 프로필의 한계:

```text
- Niagara System의 Emitter Loop, Local Space, Spawn Per Unit, Fixed Bounds를 노출하지 않는다.
- CFProjectileData의 실제 저장 프로퍼티 값과 참조를 노출하지 않는다.
- 따라서 자산 존재·경로 검증은 가능하지만 시각 적합성과 저장 연결 완료를 기계적으로 판정할 수 없다.
```

### 9.5 기술 1차 후보

#### Trail

```text
1순위 기능 후보:
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_RibbonTrail

2순위 비교 후보:
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_Flame_Trail
```

판정 이유:

```text
- NS_RibbonTrail은 이름과 용도상 고속 Projectile 궤적 확인에 가장 직접적이다.
- NS_Flame_Trail은 화염 표현이 필요한 로켓·에너지탄 비교용으로만 본다.
- Stylized Attacks 팩이므로 NS_RibbonTrail도 세미리얼 미술 승인 대상이 아니라 기능 검증용 1차 후보다.
```

#### Thruster

```text
1순위 시각·기능 후보:
/Game/RocketThrusterExhaustFX/FX/NS_RocketExhaust_Realistic

2순위 중성색 비교 후보:
/Game/RocketThrusterExhaustFX/FX/NS_RocketExhaust_White

3순위 화약 연소색 비교 후보:
/Game/RocketThrusterExhaustFX/FX/NS_RocketExhaust_Yellow
```

초기 비교 우선순위가 낮은 이름 기반 후보:

```text
Blue, Green, Violet, Energy, SciFi, Pixie, Maniac, Crackling
```

이는 Asset 이름과 현재 세미리얼 방향에 따른 시각 검토 우선순위이며 기술적 제외 판정은 아니다.

### 9.6 기준 DataAsset 운영

실전 기준 `DA_HeavyShell`은 대구경 포탄이므로 다음 구성을 우선한다.

```text
DA_HeavyShell
- TrailFxSettings: 사용 후보
- ThrusterFxSettings: 기본 비활성 유지
```

Thruster와 동시 활성 검증은 실전 Heavy Shell에 추진 화염을 영구 저장하지 않고 테스트용 복제 DataAsset으로 분리한다.

권장 테스트 자산명:

```text
DA_PFX_ThrusterTest
DA_PFX_BothTest
```

두 자산은 `DA_HeavyShell`을 복제해 이동·충돌·Damage 기준을 동일하게 유지하고 비행 FX 슬롯만 다르게 설정한다.

### 9.7 Unreal Editor 연결 절차

#### A. 후보 시각 확인

```text
1. NS_RibbonTrail을 Niagara Editor에서 연다.
2. System Overview에서 Emitter Loop 여부를 확인한다.
3. Local Space 여부를 확인한다.
4. Spawn Per Unit 또는 거리 기반 생성 여부를 확인한다.
5. System Bounds가 고속 이동에 충분한지 확인한다.
6. NS_RocketExhaust_Realistic도 같은 방식으로 Loop, Local Space와 Bounds를 확인한다.
7. 원본 수정이 필요하면 /Game/CarFight/FX/Adapted에 복제하고 원본은 수정하지 않는다.
```

#### B. DA_HeavyShell Trail 설정

```text
Trail FX 설정 (TrailFxSettings)
- 비행 FX 사용: True
- Niagara 시스템: NS_RibbonTrail 또는 승인된 Adapted 복제본
- 부착 방식: Mesh Socket With Fallback
- 부착 소켓 이름: FX_Trail
- Fallback 상대 Transform Scale: 1,1,1

추진 FX 설정 (ThrusterFxSettings)
- 비행 FX 사용: False
```

#### C. 테스트용 Thruster 설정

```text
DA_PFX_ThrusterTest
- Trail 사용: False
- Thruster 사용: True
- Niagara: NS_RocketExhaust_Realistic 또는 승인된 Adapted 복제본
- AttachMode: MeshSocketWithFallback
- AttachSocketName: FX_Exhaust
- RelativeTransform Scale: 1,1,1

DA_PFX_BothTest
- Trail: DA_HeavyShell과 동일
- Thruster: DA_PFX_ThrusterTest와 동일
```

#### D. 소켓과 Fallback

```text
1. DA_HeavyShell의 Projectile Static Mesh 옆 돋보기로 실제 메시를 찾는다.
2. Static Mesh Editor에서 메시 후방 중심에 FX_Trail 소켓을 추가한다.
3. 로켓형 테스트 메시에는 FX_Exhaust 소켓을 추가한다.
4. FX_Trail +X와 FX_Exhaust +X는 꼬리·배기가 뻗는 후방을 향하게 한다.
5. Projectile 진행 방향 +X와 반대 방향이 되는 것이 정상일 수 있다.
6. 소켓 Transform을 각 DataAsset의 RelativeTransform에도 같은 값으로 복사한다.
7. Missing Socket 테스트에서는 소켓 이름만 FX_Trail_Missing 또는 FX_Exhaust_Missing으로 바꾸고 같은 RelativeTransform이 사용되는지 확인한다.
```

#### E. 저장과 다음 게이트

```text
- Niagara 원본 또는 Adapted 복제본 저장
- Projectile Static Mesh 소켓 저장
- DA_HeavyShell 저장
- DA_PFX_ThrusterTest 저장
- DA_PFX_BothTest 저장
- 저장 뒤 AssetDump 재검증
- PFX-P0-06 사용자 PIE 진행
```

---

## 10. 예정 소스 변경 범위

### 필수 허용 경로

```text
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileData.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
```

### 선택 허용 경로

```text
UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp
```

### 조건부 허용 경로

```text
UE/Source/CarFight_Re/CarFight_Re.Build.cs
```

Niagara 의존성이 실제 빌드에서 누락됐다는 증거가 있을 때만 수정한다.
현재는 기존 CombatFx가 이미 Niagara 모듈을 사용하므로 변경하지 않는 것이 기본이다.

### Unreal Editor 자산 경로

코드 작업과 분리하여 사용자가 직접 저장한다.

```text
/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell
/Game/CarFight/FX/Adapted/...
선택한 Projectile StaticMesh의 FX_Trail 또는 FX_Exhaust 소켓
```

---

## 11. 보호 범위

```text
- ACFProjectileActor의 첫 Impact 단일 처리 계약 유지
- bImpactResolvedThisActivation 의미 유지
- Sweep/Sub-step/보조 연속 Sweep 설정 유지
- DamageHitContext 생성과 BaseDamage 적용 순서 유지
- Pool Release 시 Damage 결과 전달 유지
- ProjectilePoolComp 버킷·확보·반환 정책 유지
- CombatFxComp의 Muzzle/Impact/Destroyed 일회성 책임 유지
- Weapon Aim Solution, Reticle과 FireFeedback 유지
- TargetSelect TS-P0-00~07 완료 코드·에셋 유지
- 게임 사운드 클래스·모듈·에셋 추가 금지
- FAB 원본 자산 직접 수정 금지
- 기존 dirty 변경 정리·되돌리기 금지
- commit, push, reset, checkout, stash 금지
```

---

## 12. 구현 단계

```text
PFX-P0-00 설계·범위·작업지시 준비
PFX-P0-01 데이터 계약과 안전 기본값 구현
PFX-P0-02 Projectile Actor 컴포넌트와 부착 해석 구현
PFX-P0-03 활성화·비활성화·Pool Reset 연결
PFX-P0-04 Automation과 공식 Editor 빌드
PFX-P0-05 Unreal Editor 자산 연결과 소켓/Fallback 설정
PFX-P0-06 사용자 PIE 통합 검증과 튜닝
PFX-P0-07 Systems·TestChecklist 승격과 Plan 종료
```

현재 상태:

```text
PFX-P0-00 = Complete
PFX-P0-01 = Complete / Build PASS
PFX-P0-02 = Complete / Build PASS
PFX-P0-03 = Complete / Build PASS
PFX-P0-04 = Complete / Build PASS / Automation PASS — 2026-08-02 후속 증거 / Reusable Entry Point Not Defined
PFX-P0-05 = Complete / Trail·Thruster Assets·Sockets·Fallback User Editor PASS
PFX-P0-06 = Complete / CF-TC-023 Full Matrix User PIE PASS
PFX-P0-07 = Complete / Systems·TestChecklist Updated / CF-FQ-027 Done
```

---

## 13. 테스트 계획

### 13.1 자동 검증

예정 Automation 이름:

```text
CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract
```

검증 후보:

```text
- ACFProjectileActor CDO에 Trail/Thruster NiagaraComponent가 존재한다.
- 두 컴포넌트 AutoActivate가 false다.
- ProjectileData 기본 Trail/Thruster가 비활성이다.
- 기존 ProjectileData 기본값으로 런타임 계약이 바뀌지 않는다.
- 소켓 누락 시 Fallback Source가 선택된다.
- 활성화 후 비활성화에서 두 Niagara가 비활성·Reset 상태다.
- 비활성 데이터로 재활성화 시 이전 자산 또는 활성 상태가 남지 않는다.
```

### 13.2 공식 빌드

```text
repository_id: main_game
preset_id: carfight.editor.development
실제 실행: Tools/BuildEditor.bat --non-interactive
PASS: UHT, 컴파일, 링크와 Exit Code 0
```

### 13.3 사용자 PIE

```text
1. Trail-only Projectile
2. Thruster-only Projectile
3. Trail + Thruster Projectile
4. 유효 소켓 부착
5. 소켓 누락 Fallback
6. Hit 종료
7. LifeExpired 종료
8. Manual 종료 가능 경로
9. 같은 Pool Actor 10회 이상 재사용
10. 이전 Ribbon/연기/화염이 다음 발사에 이어지지 않음
11. 30 FPS와 기준 고속 조건에서 FX가 끊기거나 과도하게 컬링되지 않음
12. 첫 Impact, 피해 1회, Pool 반환과 기존 CombatFx 회귀 정상
13. 오디오 참조 0개
```

계획 테스트 ID:

```text
CF-TC-023 투사체 비행 FX
```

`CF-TC-023`은 기능 완료 후 `Document/ProjectSSOT/05_TestChecklist.md`에 Current System 회귀 테스트로 등록한다.

---

## 14. 완료 조건

```text
- 데이터 구조와 Blueprint 툴팁이 구현된다.
- 기존 ProjectileData의 기본 동작이 재저장 없이 유지된다.
- Trail과 Thruster를 독립 또는 동시에 활성화할 수 있다.
- 유효 소켓과 Missing Socket Fallback이 모두 동작한다.
- 모든 비활성화 사유에서 Pool 반환 전에 FX가 정리된다.
- 10회 이상 Pool 재사용에서 이전 FX 상태가 남지 않는다.
- FX 누락이 이동·충돌·피해 판정을 바꾸지 않는다.
- 공식 Editor 빌드 PASS다.
- Automation 소스가 공식 Editor 빌드에서 컴파일 PASS이고 최신 후속 실행 증거에서 RuntimeContract가 Success다.
- 초기 Not Run은 당시 체크포인트 범위로 기록하고 작업 전용 실행 수단을 공용 재실행 계약으로 승격하지 않는다.
- 사용자 PIE PASS다.
- 기존 CF-TC-015, 016, 020, 021 회귀에 이상이 없다.
- 게임 오디오 참조 0개를 유지한다.
```

완료 후 갱신:

```text
Document/Systems/Combat/Projectile.md
Document/Systems/Combat/CombatFx.md
Document/ProjectSSOT/05_TestChecklist.md
Document/ProjectSSOT/01_ProjectState.md
Document/ProjectSSOT/02_Roadmap.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/ActiveWork.md
```

---

## 15. 실패 조건

```text
- Trail 또는 Thruster 실패가 Projectile 활성화를 막는다.
- FX 컴포넌트를 발사마다 생성·파괴한다.
- Pool 반환 뒤 Niagara가 계속 활성이다.
- 이전 Ribbon이 다음 Projectile Activation에 이어진다.
- 소켓 누락이 크래시 또는 전체 발사 실패를 만든다.
- 메시 비균일 Scale이 FX를 의도치 않게 찌그러뜨린다.
- 매 Tick 소켓 검색 또는 자산 재지정을 수행한다.
- CombatFxComp에 지속형 Projectile FX 책임을 추가한다.
- 첫 Impact·Damage·Pool 반환 순서가 바뀐다.
- Audio 클래스·모듈·자산을 추가한다.
- TargetSelect 또는 unrelated dirty 변경을 수정한다.
```

---

## 16. 현재 코드 버전

```text
CFProjectileActor v1.8.1
- ProjectileMotorComp와 Burning 기반 Thruster 동기화 포함
- 소켓·Fallback 공통 독립 FX Scale 적용

CFProjectileData v1.7.1
- PropulsionConfig 포함
- Trail·Thruster 독립 FX Scale 계약과 Debug 포함

CFProjectileFlightFxTests.cpp v1.1.0
- 기본값, Fallback, 독립 Scale과 Reset RuntimeContract 소스
```

Automation 소스는 공식 Editor 빌드에서 컴파일됐으며, 2026-08-02 후속 CombatRuntime 보고서에서 `PFX_P0_01.RuntimeContract` Success를 확인했다. 초기 미실행 상태는 당시 체크포인트 이력이고 공용 재실행 진입점은 정의하지 않았다.

각 변경 파일은 Header의 Version, Date, Description, Scope, Changelog와 Migration을 갱신한다.
모든 변수와 함수 바로 위에 역할 주석을 작성한다.

---

## 17. Changelog

### v1.0.1 - 2026-08-02

```text
- 초기 Automation Not Run을 2026-07-30 당시 체크포인트 상태로 분리했다.
- UE/Saved/Automation/CombatRuntime/index.json의 최신 결과에서 PFX_P0_01.RuntimeContract Success를 반영했다.
- 검증 결과, 증거, 실행 수단과 공용 재실행 진입점의 존재 여부를 분리했다.
- 당시 작업 전용 임시 실행 경로를 공용 저장소 도구나 영구 재실행 계약으로 승격하지 않았다.
- 공식 빌드와 사용자 PIE 전체 행렬 PASS는 Automation과 독립된 완료 증거로 계속 유지했다.
```

### v1.0.0 - 2026-07-30

```text
- 사용자가 CF-TC-023 에디터 전체 행렬의 정상 동작을 확인했다.
- Trail-only, Thruster-only, Trail+Thruster, 유효 소켓과 Missing Socket Fallback을 PASS 처리했다.
- Hit·LifeExpired 종료 Reset, 20발 이상 Pool 재사용, Ribbon History 무잔류와 30 FPS 고속 Bounds를 PASS 처리했다.
- 기존 Impact·Damage 단일 처리, Pool 반환과 게임 오디오 참조 0개 회귀를 PASS 처리했다.
- CF-TC-023을 PASS, CF-FQ-027을 Done으로 전환했다.
- Automation은 소스 컴파일 PASS / 실행 Not Run 상태를 정직하게 유지하며, 현재 Admin Runner 미노출로 사용자 PIE 전체 행렬과 기존 공식 빌드를 완료 증거로 사용했다.
- 현재 구현 기준을 Document/Systems/Combat/Projectile.md v1.5.0으로 연결하고 이 Plan을 완료 이력으로 전환했다.
```

### v0.5.0 - 2026-07-28

```text
- 지속형 Projectile FX 런타임을 Document/Systems/Combat/Projectile.md v1.4.0 Current System에 반영했다.
- UCFProjectileData v1.7.1, ACFProjectileActor v1.8.1과 CFProjectileFlightFxTests.cpp v1.1.0을 현재 코드 기준으로 갱신했다.
- DA_PFX_ThrusterTest의 실제 발사, Burning 기반 Thruster, FX_Exhaust 소켓과 Scale 0.2 사용자 PIE PASS를 기록했다.
- 최종 Build Job e8b812bd479549299dd116f9bae8996f Exit Code 0을 기록했다.
- CF-TC-024 추진 범위 PASS와 CF-TC-023 전체 Flight FX 행렬 미완료를 분리했다.
- CF-FQ-027을 Active가 아닌 Paused로 정정하고 Trail-only·동시 FX·Missing Socket·Pool 10회 이상 등 미완료 행렬을 재개 기준으로 보존했다.
- Current System 반영을 CF-FQ-027 Done 또는 CF-TC-023 PASS로 해석하지 않도록 명시했다.
```

### v0.4.0 - 2026-07-27

```text
- RocketThrusterExhaustFX Niagara 18개, Stylized Attacks Niagara 14개와 DA_HeavyShell을 AssetDump로 재검증하고 전체 성공을 확인했다.
- AssetDump 기본 프로필이 Niagara 내부 Loop·Local Space·Spawn Per Unit·Bounds와 DataAsset 실제 값을 노출하지 않는 한계를 기록했다.
- Trail 기술 1차 후보를 NS_RibbonTrail, Thruster 1차 후보를 NS_RocketExhaust_Realistic으로 좁혔다.
- DA_HeavyShell은 Trail-only 실전 기준, Thruster와 동시 활성은 DA_PFX_ThrusterTest·DA_PFX_BothTest로 분리하도록 확정했다.
- Unreal Editor의 후보 시각 확인, DataAsset 설정, FX_Trail·FX_Exhaust 소켓, Fallback Transform과 저장 절차를 구체화했다.
- PFX-P0-05를 Inventory Complete / Editor Visual Selection & Asset Save Pending으로 이동했다.
```

### v0.3.0 - 2026-07-27

```text
- CodeWorkGate v2.0 직접 구현 방침에 따라 CF-FQ-027 C++ Foundation을 현재 AI 세션이 직접 작성했다.
- UCFProjectileData v1.6.0에 ECFProjectileFxAttachMode, FCFProjectileAttachedFxSettings, TrailFxSettings와 ThrusterFxSettings를 추가했다.
- ACFProjectileActor v1.7.0에 Trail·Thruster Origin/Niagara 컴포넌트, 소켓 우선·Fallback 부착과 Pool 반환 전 Reset을 구현했다.
- CFProjectileFlightFxTests.cpp v1.0.0 RuntimeContract 자동화 소스를 추가했다.
- 최초 Build Job 604e6b11db87486fa37cc09b14907e18에서 NiagaraComponent 완전 타입 include 누락을 확인하고 보정했다.
- Blueprint 툴팁 정렬 후 최종 Build Job 445848ab0f7749fcab8188b164bb1487에서 UHT·신규 테스트 컴파일·링크와 Exit Code 0을 확인했다.
- 현재 Admin 도구에 Unreal Automation 실행 표면이 없어 테스트 실행은 미수행이며, 테스트 소스의 Editor 빌드 컴파일만 확인했다.
- 다음 단계를 PFX-P0-05 Editor Niagara·DataAsset·소켓 연결과 사용자 PIE 준비로 이동했다.
```

### v0.2.0 - 2026-07-27

```text
- CarFight CodeWorkGate v2.0 직접 구현 정책을 적용했다.
- PFX-P0-00의 Final YAML Blocked 상태를 해제하고 Direct Implementation Ready로 전환했다.
- 현재 AI 세션이 PFX-P0-01부터 코드 수정, diff 검수, 빌드와 자동 테스트를 직접 수행하도록 실행 주체를 변경했다.
- TaskSource와 WorkOrder는 필수 게이트가 아니라 구현 참고 자료로 재분류했다.
```

### v0.1.0 - 2026-07-27

```text
- CF-FQ-027 투사체 비행 FX 대표 Plan을 생성했다.
- CF-FQ-024 일회성 CombatFx와 지속형 Projectile FX 책임을 분리했다.
- TrailOrigin/ThrusterOrigin과 독립 NiagaraComponent 구조를 확정했다.
- UCFProjectileData의 공용 Attached FX 설정 구조와 두 슬롯을 설계했다.
- FX_Trail / FX_Exhaust 소켓 우선과 Projectile Relative Fallback을 확정했다.
- Pool 활성화·비활성화·Reset 순서와 보호 범위를 확정했다.
- Trail 자연 소멸, 별도 FX Pool, 다중 노즐과 모터 연소 단계를 P0에서 제외했다.
- CF-TC-023 테스트 계획과 완료·실패 조건을 정의했다.
- 당시 plan.* 품질 게이트 미노출로 최종 Codex YAML 차단 상태를 기록했다.
```

---

## 18. Migration

```text
- 새 세션에서 CF-FQ-027을 Active 또는 Paused 작업으로 복원하지 않는다.
- CF-FQ-027의 현재 상태는 Done이며 CF-TC-023은 PASS다.
- 지속형 Projectile Flight FX의 현재 구현 판단은 Document/Systems/Combat/Projectile.md를 우선한다.
- 이 문서는 완료 당시 설계·구현·빌드·사용자 PIE 기록으로 유지한다.
- Automation의 현재 증거는 `UE/Saved/Automation/CombatRuntime/index.json`의 `PFX_P0_01.RuntimeContract` Success다.
- 초기 Not Run과 실행 경로 미확보는 2026-07-30 당시 체크포인트 이력으로만 유지한다.
- 당시 작업 전용 임시 실행 경로를 공용 도구, Git 등록 대상 또는 다음 세션의 고정 재실행 경로로 가정하지 않는다.
- 후속 결함이나 확장은 기존 코드를 재작성하지 않고 별도 Feature 또는 회귀 작업으로 등록한다.
```
