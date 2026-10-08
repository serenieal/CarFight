# TaskSource — CF-FQ-027 Projectile Flight FX

- Version: 0.1.0
- Date: 2026-07-27
- Status: Ready for Work-Order Compilation / Final YAML Not Generated
- Feature ID: `CF-FQ-027`
- Planned Test ID: `CF-TC-023`
- Owner Repository: `main_game`
- Repository Root: `D:\Work\CarFight_git`
- Git Branch at Preparation: `SwitchSourceEngine`

---

## 1. 작업 요청

현재 `ACFProjectileActor`의 Pool 생명주기 안에 데이터 기반 Trail과 추진 화염 Niagara를 추가한다.

투사체 StaticMesh의 `FX_Trail` 또는 `FX_Exhaust` 소켓을 우선 사용할 수 있어야 하며, 소켓이 없으면 `UCFProjectileData`의 Relative Transform으로 안전하게 Fallback해야 한다.

Projectile가 Hit, LifeExpired, Manual 또는 InvalidActivation으로 비활성화될 때는 Pool 반환 전에 두 Niagara를 정지·초기화해야 한다.

---

## 2. 근거 문서

```text
AGENTS.md
Document/CodeWorkGate.md
Document/ProjectSSOT/01_ProjectState.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/ProjectSSOT/04_ProjectDecisions.md
Document/Systems/Combat/Projectile.md
Document/Systems/Combat/CombatFx.md
Document/Plan/Archive/ProjectileFlightFxPlan.md
Document/Plan/ProjectileFlightFxRoadmap.md
```

프로젝트 결정:

```text
CF-PDL-0009: 게임 사운드 비지원
CF-PDL-0010: FAB 우선 FX, Mesh Socket/Fallback, Projectile Actor 소유와 Pool Reset
```

---

## 3. 현재 코드 근거

```text
UE/Source/CarFight_Re/Public/CFProjectileActor.h
- Version 1.6.0
- CollisionComponent, MeshComponent, ProjectileMovementComponent
- ActivateProjectile / DeactivateProjectileWithReason / FinishDeactivatePolicy

UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
- ApplyProjectileVisual
- ApplyProjectileCollision
- ApplyProjectileMovement
- Hit / LifeExpired / Manual / InvalidActivation 공용 비활성화 경로

UE/Source/CarFight_Re/Public/CFProjectileData.h
- Version 1.5.0
- Projectile Actor, Mesh, Movement, Collision, Impact FX와 DamageData
- 지속형 Trail/Thruster 설정 없음

UE/Source/CarFight_Re/Private/CFProjectileData.cpp
- BuildProjectileSummary
```

---

## 4. 구현 범위

### 4.1 데이터 타입

`CFProjectileData.h`에 다음 타입을 추가한다.

```text
ECFProjectileFxAttachMode
FCFProjectileAttachedFxSettings
```

필드:

```text
bEnabled
NiagaraSystem
AttachMode
AttachSocketName
RelativeTransform
```

`UCFProjectileData` 슬롯:

```text
TrailFxSettings
- bEnabled=false
- AttachSocketName=FX_Trail

ThrusterFxSettings
- bEnabled=false
- AttachSocketName=FX_Exhaust
```

### 4.2 Actor 컴포넌트

`ACFProjectileActor` 생성자:

```text
TrailOriginComponent
TrailNiagaraComponent
ThrusterOriginComponent
ThrusterNiagaraComponent
AutoActivate=false
AutoDestroy=false
```

컴포넌트는 발사마다 생성·파괴하지 않는다.

### 4.3 부착 해석

```text
MeshSocketWithFallback
→ MeshComponent가 소켓을 가지면 소켓 사용
→ 없으면 CollisionComponent 기준 RelativeTransform

ProjectileRelative
→ 항상 CollisionComponent 기준 RelativeTransform
```

소켓 검색은 활성화 시에만 수행한다.
메시 비균일 Scale이 FX Scale을 변형하지 않게 한다.

### 4.4 활성화

```text
ApplyProjectileVisual 이후
→ 이전 FX Reset
→ Trail 설정 적용
→ Thruster 설정 적용
→ 두 Niagara 활성화
```

FX 설정 실패는 Projectile 활성화를 취소하지 않는다.

### 4.5 비활성화

모든 사유:

```text
InvalidActivation
Manual
Hit
LifeExpired
```

`FinishDeactivatePolicy` 이전:

```text
Trail DeactivateImmediate / ResetSystem
Thruster DeactivateImmediate / ResetSystem
```

이전 Asset과 상태가 다음 Activation에 남지 않게 한다.

### 4.6 Debug

최소 정보:

```text
Trail: Disabled / MissingSystem / Active
Trail Attachment: MeshSocket:<Name> / ProjectileRelative / MissingSocketFallback:<Name>
Thruster: Disabled / MissingSystem / Active
Thruster Attachment: MeshSocket:<Name> / ProjectileRelative / MissingSocketFallback:<Name>
```

기존 VehicleDebug까지 확장하지 않는다.

---

## 5. C++ 품질 규칙

```text
- 모든 함수 바로 위에 역할 한 줄 주석
- 모든 변수 바로 위에 역할 한 줄 주석
- Blueprint 노출 필드에 한국어 DisplayName과 ToolTip
- 직관적인 변수·함수명
- 파일·클래스명 32자 이하
- Public/Private 경로 유지
- 기존 스타일과 탭 사용
- 파일 Header Version, Date, Changelog, Migration 갱신
- 무분별한 리네이밍 금지
```

예정 버전:

```text
CFProjectileActor 1.6.0 → 1.7.0
CFProjectileData 1.5.0 → 1.6.0
CFProjectileData.cpp 1.5.0 → 1.6.0
CFProjectileFlightFxTests.cpp 1.0.0 신규 선택
```

---

## 6. 허용 변경 경로

### 필수

```text
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileData.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
```

### 선택

```text
UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp
```

### 조건부

```text
UE/Source/CarFight_Re/CarFight_Re.Build.cs
```

Build.cs는 Niagara 의존성이 실제 빌드에서 부족하다는 증거가 있을 때만 수정한다.

---

## 7. 금지 변경 경로와 보호 범위

```text
UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h
UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp
UE/Source/CarFight_Re/Public/CFCombatFxComp.h
UE/Source/CarFight_Re/Private/CFCombatFxComp.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
TargetSelect 관련 전체 코드·에셋
기존 FAB 원본 콘텐츠
게임 오디오 관련 코드·자산·모듈
```

다음 의미를 변경하지 않는다.

```text
- 첫 Impact 단일 처리
- DamageHitContext 생성
- BaseDamage 적용
- Pool Release 결과 전달
- Sweep/Sub-step/보조 Sphere Sweep
- Muzzle/Impact/Destroyed CombatFx
- FireFeedback와 Reticle
```

---

## 8. 구현 금지사항

```text
- 발사마다 NiagaraComponent Spawn/Destroy
- Tick마다 소켓 검색
- Tick마다 SetAsset 또는 Reinitialize
- 소켓 누락 시 Projectile 활성화 실패
- FX 누락 시 Damage 또는 이동 취소
- Trail과 Thruster를 한 NiagaraComponent로 공유
- CombatFxComp에 지속형 FX 책임 추가
- Blueprint EventGraph에 Pool Reset 판정 구현
- 자연 소멸을 위해 Pool Actor 반환 지연
- AudioComponent, USoundBase 또는 Sound 자산 추가
- unrelated dirty 변경 정리
- commit, push, reset, checkout, stash
```

---

## 9. Automation 요구

가능하면 다음 파일을 추가한다.

```text
UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp
```

테스트 이름:

```text
CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract
```

최소 검증:

```text
- CDO에 두 Origin과 두 NiagaraComponent 존재
- Niagara AutoActivate=false
- ProjectileData 기본 두 슬롯 비활성
- Missing Socket Fallback
- Deactivate 후 Niagara 비활성·Reset
- 비활성 설정으로 재활성화 시 이전 FX 상태 없음
```

Automation을 안전하게 구성할 수 없으면 억지로 private 상태를 공개하지 않는다.
공개 Debug 요약과 CDO 컴포넌트 조회로 검증 가능한 범위를 작성하고 미검증 항목을 결과에 명시한다.

---

## 10. 빌드 요구

```text
repository_id: main_game
preset_id: carfight.editor.development
```

PASS 기준:

```text
- UnrealHeaderTool 성공
- CarFight_ReEditor Win64 Development 컴파일·링크 성공
- Exit Code 0
```

---

## 11. 사용자 PIE 요구

Codex는 자산을 임의 선택하거나 바이너리 `.uasset`을 수정하지 않는다.
코드·빌드·Automation 완료 후 사용자가 Editor에서 수행할 체크리스트를 결과에 제공한다.

```text
- Trail-only
- Thruster-only
- Trail+Thruster
- 유효 소켓
- Missing Socket Fallback
- Hit 종료
- LifeExpired 종료
- 같은 Pool Actor 10회 이상 반복
- 이전 Ribbon/화염 잔류 없음
- 30 FPS 고속 발사
- 기존 Impact·Damage·Destroyed 회귀
```

---

## 12. 완료 조건

```text
- 허용 경로만 변경
- 데이터 기본값 backward compatible
- 두 FX 독립·동시 지원
- 소켓/Fallback 지원
- Pool 반환 전 Reset
- Debug 상태 확인 가능
- 공식 Editor 빌드 PASS
- Automation PASS 또는 명확한 미검증 사유
- 사용자 PIE 체크리스트 제공
- 기존 전투 판정 회귀 없음
- Audio 참조 0개
```

---

## 13. 실패 조건

```text
- 허용 경로 외 변경
- unrelated dirty 변경 수정
- 빌드 실패
- Pool 재사용에서 FX 상태 잔류
- FX 누락이 Projectile 실패를 유발
- 첫 Impact/Damage 횟수 변화
- Audio 추가
- commit/push/reset/checkout/stash 수행
```

---

## 14. 결과 보고 형식

```text
1. 변경 요약
2. 변경 파일과 버전
3. 데이터 계약
4. Actor 컴포넌트 구조
5. 활성화·비활성화·Pool Reset 순서
6. 소켓/Fallback 동작
7. Automation 결과
8. 공식 Editor 빌드 결과와 Exit Code
9. 사용자 PIE 절차
10. 미검증·후속 항목
11. 보호 범위 준수 확인
12. Git 작업 미수행 확인
```

---

## 15. Changelog

### v0.1.0 - 2026-07-27

```text
- CF-FQ-027 구현용 TaskSource를 생성했다.
- 데이터, Actor, Pool, 소켓/Fallback, Automation과 빌드 요구를 고정했다.
- 허용 경로·보호 범위·완료·실패 조건을 명시했다.
- 최종 Codex YAML 생성 전 입력으로 준비했다.
```
