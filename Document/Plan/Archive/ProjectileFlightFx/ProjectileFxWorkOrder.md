# Implementation Work Order — CF-FQ-027 Projectile Flight FX

- Version: 0.2.0
- Date: 2026-07-27
- Status: Ready for Browser AI Direct Implementation
- Feature ID: `CF-FQ-027`
- Source: `ProjectileFxTaskSource.md`
- Representative Historical Plan: `Document/Plan/Archive/ProjectileFlightFxPlan.md`

---

## 작업지시 상태

```text
TaskSource: Ready
Human-Readable Work Order: Ready
Implementation Source: Browser AI Direct
Code Gate: Ready — Direct Implementation
Next Step: PFX-P0-01 ProjectileData Contract
Source Changes: Not Started
Diff Review: Not Started
Build: Not Run
Automation: Not Run
PIE: Not Run
```

`Document/CodeWorkGate.md` v2.0에 따라 현재 AI 세션이 이 문서를 구현 참고 기준으로 사용해 실제 코드 수정, diff 검수, 빌드와 자동 테스트를 직접 수행한다.
최종 Codex YAML과 plan.* 품질 게이트는 착수 조건이 아니다.

---

## 1. 목표

`ACFProjectileActor`에 데이터 기반 Trail과 추진 화염 Niagara를 추가한다.
두 FX는 Projectile Actor가 영구 소유하며, Projectile Pool 재사용 때 이전 상태가 남지 않도록 모든 비활성화 경로에서 Pool 반환 전에 정지·초기화한다.

---

## 2. 작업 전 필수 확인

```text
1. git status 확인
2. 기존 dirty 변경을 건드리지 않음
3. AGENTS.md 확인
4. Document/CodeWorkGate.md 확인
5. Document/Plan/Archive/ProjectileFlightFxPlan.md 확인
6. Document/Plan/ProjectileFxTaskSource.md 확인
7. 실제 CFProjectileActor/Data 코드 재확인
```

---

## 3. 변경 허용 파일

```text
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Private/CFProjectileData.cpp
UE/Source/CarFight_Re/Public/CFProjectileActor.h
UE/Source/CarFight_Re/Private/CFProjectileActor.cpp
UE/Source/CarFight_Re/Private/CFProjectileFlightFxTests.cpp   # 선택
UE/Source/CarFight_Re/CarFight_Re.Build.cs                    # 의존성 실패 증거가 있을 때만
```

그 외 파일은 수정하지 않는다.

---

## 4. 구현 지시

### 4.1 ProjectileData

```text
- ECFProjectileFxAttachMode 추가
  - ProjectileRelative
  - MeshSocketWithFallback

- FCFProjectileAttachedFxSettings 추가
  - bEnabled
  - NiagaraSystem
  - AttachMode
  - AttachSocketName
  - RelativeTransform

- UCFProjectileData 추가 필드
  - TrailFxSettings / 기본 FX_Trail / Disabled
  - ThrusterFxSettings / 기본 FX_Exhaust / Disabled
```

모든 Blueprint 필드에 한국어 DisplayName과 ToolTip을 작성한다.
기존 DataAsset은 재저장 없이 이전 동작을 유지해야 한다.

### 4.2 Projectile Actor 컴포넌트

```text
CollisionComponent
├─ MeshComponent
├─ TrailOriginComponent
│  └─ TrailNiagaraComponent
└─ ThrusterOriginComponent
   └─ ThrusterNiagaraComponent
```

```text
Niagara AutoActivate=false
Niagara AutoDestroy=false
```

Trail과 Thruster를 하나의 컴포넌트로 합치지 않는다.

### 4.3 부착

```text
MeshSocketWithFallback + 유효 소켓
→ MeshComponent Socket 사용

그 외
→ CollisionComponent 기준 RelativeTransform 사용
```

소켓 누락은 오류가 아니라 Fallback 상태다.
메시 Scale과 FX Scale을 분리한다.

### 4.4 활성화

```text
Actor 회전과 Mesh 적용
→ 이전 FX Reset
→ 설정 적용
→ 부착 해석
→ Asset 지정
→ Reinitialize/Activate
```

FX 설정 실패는 Projectile 활성화를 취소하지 않는다.

### 4.5 비활성화

다음 모든 사유에서:

```text
InvalidActivation
Manual
Hit
LifeExpired
```

`FinishDeactivatePolicy` 이전에:

```text
Trail DeactivateImmediate + ResetSystem
Thruster DeactivateImmediate + ResetSystem
```

이전 Asset과 상태가 다음 Activation에 남지 않게 한다.

### 4.6 Debug

최소한 다음 상태를 읽을 수 있게 한다.

```text
Trail 상태 / 부착 출처
Thruster 상태 / 부착 출처
```

기존 VehicleDebug까지 확장하지 않는다.
Projectile Actor의 짧은 getter 또는 Summary로 제한한다.

---

## 5. 버전·주석 규칙

```text
CFProjectileActor: 1.6.0 → 1.7.0
CFProjectileData: 1.5.0 → 1.6.0
신규 Test: 1.0.0
```

각 파일에 Version, Date, Description, Scope, Changelog와 Migration을 갱신한다.
모든 변수와 함수 바로 위에 한 줄 역할 주석을 작성한다.
기존 함수 시그니처와 이름을 불필요하게 바꾸지 않는다.

---

## 6. 보호 요구

```text
- ProjectilePoolComp 수정 금지
- CombatFxComp 수정 금지
- VehiclePawn 수정 금지
- TargetSelect 수정 금지
- first Impact / Damage / Pool Release 의미 변경 금지
- Tick 기반 FX 관리 금지
- Audio 추가 금지
- FAB 원본 수정 금지
- unrelated dirty 변경 수정 금지
- commit/push/reset/checkout/stash 금지
```

---

## 7. 검증

### Automation

가능하면:

```text
CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract
```

검증:

```text
- 컴포넌트 존재
- AutoActivate=false
- 데이터 기본 비활성
- Missing Socket Fallback
- Deactivate 후 Reset
- 재활성화 상태 오염 없음
```

### Build

```text
repository_id=main_game
preset_id=carfight.editor.development
```

### 사용자 PIE 체크리스트

```text
- Trail-only
- Thruster-only
- Trail+Thruster
- Socket
- Missing Socket Fallback
- Hit/LifeExpired 종료
- Pool 10회 반복
- 30 FPS 고속 발사
- 기존 Impact/Damage/Destroyed 회귀
```

---

## 8. 성공 판정

```text
- 허용 파일만 변경
- Editor Build PASS
- Automation PASS 또는 미검증 사유 명시
- Pool 반환 전 두 FX Reset
- 기존 ProjectileData 호환
- 소켓/Fallback 동작
- 전투 판정 회귀 없음
- 오디오 참조 0개
```

---

## 9. 결과 보고

```text
변경 요약
변경 파일/버전
데이터 구조
컴포넌트 구조
생명주기
소켓/Fallback
Automation
Build Job / Exit Code
PIE 절차
미검증 항목
보호 범위 준수
Git 쓰기 작업 미수행
```

---

## 10. 직접 구현 메타데이터

```text
repository_id: main_game
branch: SwitchSourceEngine
feature_id: CF-FQ-027
planned_test_id: CF-TC-023
reasoning_level: high
implementation_source: Browser AI Direct
source_document: Document/Plan/ProjectileFxTaskSource.md
representative_plan: Document/Plan/Archive/ProjectileFlightFxPlan.md
build_preset: carfight.editor.development
allow_commit: false
allow_push: false
allow_reset: false
allow_checkout: false
allow_stash: false
```

---

## 11. Changelog

### v0.2.0 - 2026-07-27

```text
- CarFight CodeWorkGate v2.0 직접 구현 정책을 적용했다.
- 외부 Codex용 초안 상태를 Browser AI Direct 구현 작업지시로 전환했다.
- 최종 YAML과 plan.* 품질 게이트 의존성을 제거했다.
- 변경 범위, 보호 범위와 검증 형식은 그대로 유지했다.
```

### v0.1.0 - 2026-07-27

```text
- CF-FQ-027의 사람이 검토 가능한 구현 작업지시 초안을 생성했다.
- 코드 변경 범위, 구현 요구, 보호 범위와 검증 형식을 고정했다.
- 당시 plan.* 품질·증거 게이트 미평가와 최종 YAML Missing 상태를 명시했다.
```
