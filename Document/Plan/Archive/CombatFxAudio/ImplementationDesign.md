# Combat FX ImplementationDesign

- Version: 0.10.0
- Date: 2026-07-27
- Status: Done / User PIE PASS / CF-FQ-024 Complete / CF-TC-021 PASS
- Feature ID: `CF-FQ-024`
- Scope: 승인된 발사, 첫 Impact와 최초 차량 파괴 결과를 Niagara 기반 시각 연출로 표현하고 FAB FX 자산을 최소 수정으로 통합하는 P0 계획
- Path Note: `CombatFxAudio`는 기존 디렉터리 호환을 위한 레거시 경로명이며 Audio 구현을 의미하지 않는다.

---

## 현재 작업 체크포인트

```text
현재 완료: CF-FQ-024 전투 FX / Done / User PIE PASS / CF-TC-021 PASS
CF-FQ-026 상태: Paused / TS-P0-08 체크포인트 보존
선행 Combat 판정: 완료
최종 미술 방향: 세미리얼
제작 정책: FAB FX 팩 우선 활용 / 직접 신규 제작 최소화
반입 팩 1: /Game/RocketThrusterExhaustFX
- NiagaraSystem: 18개 / Dump 성공 18 / 실패 0
반입 팩 2: /Game/sA_Megapack_v1
- NiagaraSystem: 96개 / Dump 성공 96 / 실패 0
현재 확인 NiagaraSystem 합계: 114개
Cascade ParticleSystem: 이번 Niagara 전용 조사 범위 밖 / 미확인
신규 C++ 구현:
- UCFCombatFxData 추가
- UCFCombatFxComp 추가
- UCFWeaponData.DefaultFireFxData 추가
- UCFProjectileData.DefaultImpactFxData 추가
- UCFVehicleData.DefaultDestroyedFxData 추가
- ACFVehiclePawn 발사·HitScan·Projectile·최초 파괴 FX 요청 연결
Niagara 모듈 의존성: CarFight_Re.Build.cs 반영 완료
공식 C++ Foundation 빌드: Build Job bd5a50bf388049ec84123aec4942ae96 / Exit Code 0
Editor Preview Tool 최초 빌드: Build Job c9682d4a0be24ead95e87a5a7e8b5849 / Exit Code 0
Rapid Preview Tuning 빌드: Build Job 89b6d82710af47e89a4c5b3823037d4e / Exit Code 0
CombatFxData 인스턴스: /Game/CarFight/FX/Data 아래 3개 확인 / AssetDump 3 성공 0 실패
Editor Preview Actor: ACFCombatFxPreviewActor v1.2.0 구현 완료
- Editor Viewport 자동 반복 재생
- Uniform Scale 빠른 조정
- Preview 최대 수명 자동 정지
- 선택한 Override Niagara를 DataAsset에 함께 적용 가능
- ComponentTransform / NiagaraUserVector Scale 전달 모드 지원
Loop 안전 퓨즈: UCFCombatFxData.MaximumLifetimeSeconds 구현 완료
차량별 Destroyed 위치: UCFVehicleData.DestroyedFxSocketName 기본 `FX_Destroyed`
Destroyed 위치 해석: SM_Body 소켓 → SM_Body Bounds 중심 → Actor Transform → DamageHitContext Fallback
기준 VehicleData: /Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan
기준 차체 메시: /Game/CarFight/Vehicles/Meshes/Sedan/Sedan
DA_PoliceCar: 현재 사용하지 않는 폐기 자산
Impact P0 선택: NS_BasicHit / 외부 Transform Scale 미반응을 허용하고 현재 크기 그대로 사용
NS_Impact_1 추가 튜닝과 공용 Niagara Scale 완성: P0 중단
NS_Explossion: Loop 반복으로 Destroyed P0 후보 제외
Destroyed 위치: Sedan의 FX_Destroyed 소켓 적용 후 사용자 확인 해결
SmokeBuilder: NiagaraSystem 0개 / Cascade ParticleSystem 64개 / 현재 Niagara 전용 P0 계약에는 자동 채택하지 않음
AssetDump 제한: 현재 프로필은 DataAsset 프로퍼티 값을 노출하지 않아 실제 참조값은 Editor 저장 상태와 사용자 확인을 우선함
사용자 직접 Editor 빌드: PASS 보고 / Admin Build Job ID 없음
최종 사용자 PIE: PASS
- 정상 발사 Muzzle 1회 / 발사 거부 0회 PASS
- Impact 실제 충돌 위치 / 첫 1회 / 중복·잔류 없음 PASS
- Destroyed FX_Destroyed 소켓 위치 / 최초 1회 / 추가 피해 중복·잔류 없음 PASS
- 기존 조준·발사·피격·피해·파괴 회귀 PASS
최종 판정: CF-FQ-024 Done / CF-TC-021 PASS
Current System: Document/Systems/Combat/CombatFx.md
다음 기능: 자동 착수하지 않음 / CF-FQ-019 Candidate / CF-FQ-026 TS-P0-08 Paused
```

---

## 1. 프로젝트 전역 사운드 제외 계약

CarFight는 게임 사운드를 지원하지 않는다.

금지 범위:

```text
- SoundWave / SoundCue / MetaSound Source / Sound Attenuation
- USoundBase / UAudioComponent
- PlaySoundAtLocation / SpawnSoundAtLocation / SpawnSoundAttached 계열 호출
- AudioMixer / Audio Modulation 별도 의존성
- 엔진음 / 타이어음 / 차량 충돌음
- 무기 발사음 / Projectile 비행음 / Impact 사운드 / 파괴음
- UI음 / 경고음 / BGM / 환경음
```

허용 상태:

```text
- Unreal이 생성한 기본 AudioSampleRate 등 플랫폼 설정은 유지할 수 있다.
- 기본 설정 존재는 CarFight 게임 사운드 구현으로 해석하지 않는다.
- 엔진 또는 에디터 자체 소리는 게임 콘텐츠 범위와 구분한다.
```

보호 규칙:

```text
- 사운드를 누락 기능이나 품질 후속으로 자동 제안하지 않는다.
- FX DataAsset에 Sound 또는 Attenuation 필드를 예약하지 않는다.
- Projectile Pool에 Audio 상태 초기화 책임을 추가하지 않는다.
- 빌드와 PIE 완료 조건에 청각 확인 항목을 넣지 않는다.
```

---

## 2. 목적

이미 검증된 발사·피격·피해·파괴 판정을 다시 계산하지 않고, 확정된 결과를 플레이어가 화면에서 즉시 이해할 수 있도록 시각 연출을 연결한다.

P0 목표:

> 승인된 발사는 실제 총구에서 1회, 첫 유효 Impact는 충돌 위치에서 1회, 최초 차량 파괴 전환은 차량별 `SM_Body.FX_Destroyed` 소켓 위치에서 1회의 Niagara FX를 발생시킨다.

---

## 3. P0 필수 범위

### 3.1 발사 FX

```text
- 실제 최종 Muzzle 위치와 AimDirection 사용
- 승인된 HitScan 또는 Projectile 실행 성공 이후 1회 호출
- Cooldown, NoWeapon, AimBlocked, 정렬 거부와 MuzzleBlocked에서는 호출하지 않음
```

### 3.2 Impact FX

```text
- FCFDamageHitContext.ImpactLocation 사용
- FCFDamageHitContext.ImpactNormal 기준 회전
- HitScan과 Projectile이 같은 요청 구조 사용
- 첫 Blocking Hit에서 1회 호출
- OnComponentHit과 보조 Sweep의 중복 Impact 방지 상태 공유
```

### 3.3 Destroyed FX

```text
- UCFVehicleHealthComp 최초 Destroyed 전환 사용
- UCFVehicleData.DestroyedFxSocketName 기본값은 FX_Destroyed
- SM_Body에 유효 소켓이 있으면 해당 월드 위치·회전을 사용
- 소켓이 없으면 SM_Body Bounds 중심, Actor Transform, DamageHitContext 순으로 Fallback
- 파괴 후 추가 피해에서는 다시 호출하지 않음
- Health 판정과 연출 성공 여부 분리
```

### 3.4 데이터 기반 자산 선택

```text
- 소스 코드에 Niagara 자산 경로를 하드코딩하지 않음
- WeaponData / ProjectileData / VehicleData가 FX 프로파일 DataAsset 선택
- FX 자산 미연결 상태에서도 발사·피해·파괴 판정 유지
- 연결 상태를 Debug 문자열 또는 로그로 확인 가능
```

---

## 4. P0 선택 범위

```text
- Projectile Trail Niagara
- 총구 Point Light 플래시
- Impact Decal 1종
- 경미한 카메라 셰이크
- 게임패드 진동
```

필수 범위를 지연시키면 `CF-FQ-020`으로 이관한다.

---

## 5. 기능 내 제외 범위

```text
- 모든 게임 사운드와 오디오 설정 UI
- 물리 표면별 Impact 세분화
- 완성형 Geometry Collection 차량 분해
- 대량 파편 물리와 폭발 범위 피해
- 피해 숫자와 킬로그
- 서버 복제와 원격 클라이언트 FX 동기화
- 대규모 Presentation Subsystem
```

게임 사운드는 후속 범위가 아니라 프로젝트 전역 비지원 범위다.

---

## 6. C++와 Blueprint 책임 분리

### C++ 책임

```text
- 발사 성공, 첫 Impact와 최초 파괴 전환 시점 확정
- 공용 FX 요청 데이터 구조 정의
- DataAsset 참조와 유효성 검사
- 위치·방향·스케일 전달
- 중복 호출 방지
- Projectile Pool 재사용 시 지속형 Projectile FX 상태 초기화 계약 보장
- Impact 월드 Transform을 Pool 반환 전에 복사하고 일회성 FX 요청 전달
- 소켓 존재 여부, Fallback Transform과 FAB 축 보정 상태 Debug 제공
- Debug 요약 제공
```

### Blueprint / Unreal Editor 책임

```text
- FAB Niagara System 선별, 원본 보존과 필요한 경우 CarFight 복제본 생성
- Niagara System의 크기, 회전, 수명, 밝기, 파티클 수와 Bounds 최소 보정
- ACFCombatFxPreviewActor를 TestMap에 배치해 PIE 없이 Scale·Rotation·Location 반복 조정
- Preview 값을 CombatFxData에 적용하고 패키지 저장
- 필요 시 Material, Decal, Point Light와 Camera Shake 연결
- Combat FX DataAsset 생성과 참조 연결
- Mesh 소켓 또는 Fallback Transform의 위치·방향 확인
- 최종 TestMap PIE에서 발생 위치·횟수와 회귀 결과 검증
```

### 6.1 Editor Preview Actor

```text
클래스: ACFCombatFxPreviewActor
성격: EditorOnly / 게임 빌드와 전투 판정에서 제외
직접 배치: Place Actors의 C++ Classes에서 배치 가능
Blueprint 자식: 선택 사항이며 필수 아님
```

구성:

```text
PreviewRoot
├─ ReferenceMesh
├─ PreviewOrigin
│  ├─ EmissionDirection Arrow (+X)
│  └─ PreviewNiagara
```

조정 프로퍼티:

```text
PreviewCombatFxData
PreviewNiagaraSystemOverride
bApplyOverrideNiagaraToDataAsset
PreviewLocationOffset
PreviewRotationOffset
bUseUniformScale
PreviewUniformScale
PreviewFxScale
PreviewMaximumLifetimeSeconds
bRestartPreviewWhenPropertyChanges
bAutoActivatePreview
bAutoReplayPreview
PreviewReplayIntervalSeconds
bStopPreviewAtMaximumLifetime
```

Call In Editor 버튼:

```text
미리보기 재생
미리보기 재시작
미리보기 정지
DataAsset 값 불러오기
Preview 값을 DataAsset에 적용
```

`Preview 값을 DataAsset에 적용`은 Scale, RotationOffset과 MaximumLifetimeSeconds를 복사하고 DataAsset 패키지를 Dirty 처리한다. `bApplyOverrideNiagaraToDataAsset=true`이고 Override가 지정돼 있으면 선택 Niagara도 함께 적용한다. 자동 저장하지 않으므로 사용자가 저장한다.

자동 반복 사용 권장값:

```text
bAutoReplayPreview = true
PreviewReplayIntervalSeconds = 2.0
bUseUniformScale = true
bStopPreviewAtMaximumLifetime = true
```

일회성 후보는 자동 반복으로 여러 번 관찰하고, Loop 후보는 PreviewMaximumLifetimeSeconds에서 자동 정지시켜 에디터에 영구 잔류하지 않도록 한다.

### 명시적 비책임

```text
- Sound 자산 제작
- AudioComponent 생성
- 사운드 재생·정지·공간화·감쇠
- 오디오 Pool 상태 초기화
- 믹싱과 음량 옵션
```

---

## 7. 데이터 구조 후보

신규 DataAsset 후보:

```text
UCFCombatFxData
```

파일 후보:

```text
UE/Source/CarFight_Re/Public/CFCombatFxData.h
UE/Source/CarFight_Re/Private/CFCombatFxData.cpp
```

현재 구현 필드:

```text
CombatFxId
NiagaraSystem
FxScale
FxScaleMode
NiagaraUserScaleParameterName
RotationOffset
MaximumLifetimeSeconds
```

`MaximumLifetimeSeconds`는 Loop Niagara가 실수로 연결됐을 때 영구 잔류를 막는 안전 퓨즈다. 0이면 강제 종료를 사용하지 않는다. 이 필드가 Loop 자산을 정상 일회성 후보로 승인하는 것은 아니다.

Sound, Audio와 Attenuation 필드는 두지 않는다.

참조 구조:

```text
UCFWeaponData -> DefaultFireFxData
UCFProjectileData -> DefaultImpactFxData
UCFVehicleData -> DefaultDestroyedFxData
UCFVehicleData -> DestroyedFxSocketName = FX_Destroyed
```

Projectile 메시에 종속된 구조 정보 후보:

```text
UCFProjectileData
- TrailAttachSocketName = FX_Exhaust
- TrailFallbackRelativeTransform
```

`TrailAttachSocketName`과 Fallback Transform은 Niagara 자산의 속성이 아니라 Projectile Mesh의 구조적 속성이므로 `UCFCombatFxData`가 아니라 `UCFProjectileData`가 소유한다.

기존 `ImpactEffectId`는 VFX / Decal용 레거시 Debug fallback으로만 유지한다.

---

## 8. 런타임 구조 후보

```text
UCFCombatFxComp
```

파일 후보:

```text
UE/Source/CarFight_Re/Public/CFCombatFxComp.h
UE/Source/CarFight_Re/Private/CFCombatFxComp.cpp
```

권장 소유:

```text
ACFVehiclePawn
└─ UCFCombatFxComp
   - Muzzle 일회성 FX 요청
   - Impact 일회성 FX 요청
   - Destroyed 일회성 FX 요청

ACFProjectileActor
└─ UNiagaraComponent ProjectileTrailFxComp
   - 추진 화염 / Trail 활성화
   - 소켓 또는 Fallback 부착
   - Impact / LifeExpired / Manual / InvalidActivation 종료
   - Pool 반환 전 Deactivate 및 Reset
```

함수 후보:

```text
UCFCombatFxComp
- PlayFireFx(...)
- PlayImpactFx(...)
- PlayDestroyedFx(...)
- BuildCombatFxSummary()

ACFProjectileActor
- ActivateProjectileFx(...)
- DeactivateProjectileFx(...)
- ResolveProjectileFxAttachment(...)
```

타입과 함수명에 `Audio`, `Sound`, `Sfx`를 사용하지 않는다.

---

## 9. 호출 시점

```text
발사:
ValidateFireCommand 성공
→ HitScan 또는 Projectile 실행 성공
→ LastFireResult.bAccepted 기록
→ Fire FX 1회

Impact:
첫 Blocking Hit
→ FCFDamageHitContext 생성
→ Damage 적용 요청
→ Impact FX 1회
→ Pool 반환 또는 HitScan 종료

파괴:
CurrentHealth > 0
→ 피해 적용
→ CurrentHealth <= 0
→ 최초 Destroyed 전환
→ Destroyed FX 1회
```

피해가 거부되더라도 실제 표면 충돌이 있었다면 Impact FX는 1회 발생할 수 있다.

---

## 10. 구현 단계

```text
Phase 0: FAB FX 팩 1개 반입, 자산 목록·의존성·라이선스 메모와 P0 후보 선별
Phase 1: UCFCombatFxData / UCFCombatFxComp / Projectile 지속 FX 계약 확정
Phase 2: Niagara 모듈 의존성, 공용 C++ FX 런타임, null-safe 처리, Debug, Editor 빌드
Phase 3: 승인된 발사 Muzzle FX 연결
Phase 4: HitScan / Projectile 첫 Impact FX 연결과 중복 방지
Phase 5: Projectile 추진 화염 또는 Trail 연결과 Pool 초기화
Phase 6: 최초 Destroyed FX 연결
Phase 7: /Game/CarFight/FX/Adapted 및 /Game/CarFight/FX/Data 자산 연결
Phase 8: CF-TC-021 사용자 PIE와 기존 Combat 회귀
```

권장 임시 자산:

```text
NS_CF_Muzzle_Proto
NS_CF_Impact_Proto
NS_CF_Destroyed_Proto
DA_FX_ProtoWeaponFire
DA_FX_ProtoShellImpact
DA_FX_ProtoVehicleDead
```

---

## 11. P0 테스트

```text
- 정상 발사: Muzzle FX 1회
- 발사 거부: Muzzle FX 없음
- HitScan Impact: ImpactLocation에서 FX 1회와 피해 1회
- Projectile Impact: 첫 Hit FX 1회, 중복 없음, Pool 재사용 정상
- 차량 파괴: 최초 전환 FX 1회, `SM_Body.FX_Destroyed` 소켓 위치 사용, 추가 피해 중복 없음
- 반복 전투: 10회 이상 반복 후 Niagara/Trail 잔류 없음
- Pool 재사용: 이전 Ribbon이 다음 Projectile까지 이어지지 않음
- 위치 기준: Muzzle과 FX_Exhaust 소켓이 있으면 소켓 Transform 사용
- Fallback 기준: 소켓이 없으면 DataAsset 상대 Transform으로 정상 부착
- 축 기준: CarFight 소켓 +X가 방출 방향이며 FAB 자산 차이는 RotationOffset으로만 보정
- 공간 기준: 추진 화염은 Attached 동작, 배출 연기·Ribbon은 필요 시 World Space 확인
- Bounds 기준: 고속 이동과 카메라 회전에서 FX가 갑자기 사라지지 않음
- 스케일 기준: Projectile Mesh 표시 스케일이 FX 크기를 의도치 않게 증폭하지 않음
- 오디오 검사: 게임 오디오 에셋·클래스·모듈 참조 0개
```

---

## 12. 보호 범위

```text
- Weapon Aim Solution과 MuzzleBlocked 의미 유지
- LastFireResult와 FireFeedback UI 우선순위 유지
- DamageData.BaseDamage와 Health 계산 유지
- FCFDamageHitContext 의미 유지
- Projectile 첫 Impact 1회와 Pool 반환 순서 유지
- WBP_AimReticle 바인딩 유지
- 게임 사운드 구현 금지
- 기존 미커밋 변경을 정리하거나 되돌리지 않음
- commit, push, reset, checkout, stash 수행 금지
```

---

## 13. 완료 판정

```text
- C++ 공식 Editor 빌드 PASS
- 승인 발사 Muzzle FX 1회
- 발사 거부 FX 없음
- HitScan Impact FX 1회
- Projectile Impact FX 1회와 중복 없음
- 최초 차량 파괴 FX 1회와 추가 피해 중복 없음
- 반복 발사와 Pool 재사용에서 Niagara 잔류 없음
- 오디오 에셋·클래스·모듈 참조 0개
- 기존 발사·Reticle·피격·피해 회귀 이상 없음
- 사용자 PIE 확인 완료
```

완료 후 문서:

```text
Document/Systems/Combat/CombatFx.md
Document/Systems/Combat/FireFeedback.md
Document/Systems/Combat/Projectile.md
Document/Systems/Combat/HitDamage.md
```

---

## 14. 최종 미술 방향

```text
방향: 세미리얼
기반: 현실적인 화염, 연기, 스파크, 먼지와 재질 반응
강조: 전투 결과가 즉시 읽히는 범위에서 밝기·크기·방향성을 제한적으로 과장
금지: 모든 전투 FX를 네온 색상으로 통일하거나 과도한 Bloom으로 형태를 지우는 방식
```

색과 형태 기준:

```text
- 화약 발사: 백색 핵심, 옅은 황색과 주황 화염, 중성 회색 연기
- 금속 Impact: 백색·황색·주황 스파크와 작은 금속성 파편
- 월드 Impact: 재질 기반 먼지와 파편, 차량 Impact보다 낮은 발광 우선순위
- 차량 파괴: 백색 고온 핵심, 황색·주황 화염, 회색·검은 연기
- 무기 차이는 색만 바꾸지 않고 형태, 길이, 밀도와 시간으로 구분
```

---

## 15. 1인 개발용 FX 제작 정책

```text
- 직접 신규 제작은 예외로 두고 FAB FX 팩 활용을 기본으로 한다.
- P0는 Muzzle, Impact와 Destroyed 각 1개를 먼저 확보한다.
- Projectile Trail은 실제 가독성 필요가 있는 탄종에만 연결한다.
- FAB 원본 폴더는 수정하지 않는다.
- 크기·회전·수명·밝기·파티클 수·Bounds 최소 수정으로 해결되지 않으면 다른 자산으로 교체한다.
- 서로 다른 팩을 대규모로 혼합하거나 공통 Niagara Module을 새로 만드는 작업은 P0에서 제외한다.
- 미술 통일보다 발사 성공, 진행 방향, 충돌 위치와 파괴 전환의 가독성을 우선한다.
```

자산 배치 원칙:

```text
FAB 원본
- /Game/<VendorOrPack>/...

CarFight 수정본이 필요한 경우
- /Game/CarFight/FX/Adapted/...

CarFight FX 데이터
- /Game/CarFight/FX/Data/...
```

---

## 16. FX 위치와 부착 기준

### 16.1 위치 유형

| 위치 유형 | 기준 | 예시 |
|---|---|---|
| 메시에 고정 | Mesh Socket 우선 | Muzzle, FX_Exhaust, FX_Destroyed |
| 런타임 충돌 결과 | World Transform 직접 사용 | ImpactLocation, ImpactNormal |
| 런타임 회전·장착 기준 | SceneComponent | TurretYawPivot, TurretPitchPivot |

### 16.2 소켓 규칙

```text
- CarFight FX 소켓의 +X축은 해당 FX의 방출 방향이다.
- 총구 Muzzle +X는 발사 방향이다.
- 미사일 FX_Exhaust +X는 배기 분출 방향이므로 미사일 진행 방향의 반대를 향할 수 있다.
- FAB Niagara의 원본 축 차이는 소켓을 바꾸지 않고 RotationOffset으로 보정한다.
- 소켓 검색과 해석은 활성화 또는 Mesh 변경 시 수행하고 매 Tick 반복하지 않는다.
```

### 16.3 Fallback 규칙

```text
소켓 존재
→ 소켓 위치·회전 사용

소켓 누락
→ MeshComponent 기준 TrailFallbackRelativeTransform 사용

FX 자산 누락
→ FX만 생략하고 발사·이동·충돌·피해 판정 유지
```

### 16.4 일회성·지속형 구분

```text
Muzzle / Impact / Destroyed
→ 일회성 Spawn

Projectile 추진 화염 / Trail
→ ACFProjectileActor에 부착된 지속형 NiagaraComponent
→ Projectile 활성화와 함께 시작
→ Impact, 수명 종료, 수동 종료와 활성화 실패에서 정지
→ P0에서는 Pool 반환 전에 즉시 제거하고 자연 소멸 분리는 후속으로 둠
```

---

## 17. FAB 자산 반입 준비

실제 자산 반입과 선별은 다음 문서를 따른다.

```text
Document/Plan/CombatFxAudio/AssetPreparationChecklist.md
```

준비 완료 조건:

```text
- P0에 사용할 FAB FX 팩 1개가 CarFight 프로젝트에 추가됨
- NiagaraSystem 목록과 Demo 의존성 확인
- Muzzle / Vehicle Impact / World Impact / Destroyed / Trail 후보 기록
- 원본 수정 여부와 CarFight 복제 필요 여부 결정
- 각 후보의 기본 축, Local/World Space, Loop, Auto Destroy와 Bounds 확인
- P0 선택 자산과 제외 자산을 문서에 기록
```

---

## 18. Changelog

### v0.10.0 - 2026-07-27

```text
- 최종 사용자 PIE에서 Muzzle 정상 발사 1회와 발사 거부 0회를 PASS했다.
- Impact 실제 충돌 위치, 첫 유효 Impact 1회, 중복 없음과 잔류 없음을 PASS했다.
- Destroyed FX가 SM_Body.FX_Destroyed 소켓 위치에서 최초 1회 발생하고 추가 피해에 반복·잔류하지 않음을 PASS했다.
- 기존 조준·발사·피격·피해·파괴 전투 회귀를 PASS했다.
- CF-FQ-024를 Done으로 전환하고 CF-TC-021 PASS를 기록했다.
- 현재 구현 기준을 Document/Systems/Combat/CombatFx.md로 승격했다.
- 다음 기능은 자동 착수하지 않고 CF-FQ-019 Candidate와 CF-FQ-026 Paused 상태를 유지했다.
```

### v0.9.0 - 2026-07-27

```text
- Impact P0 Niagara를 NS_BasicHit으로 사용자 확정했다.
- NS_BasicHit이 외부 Transform Scale에 반응하지 않더라도 현재 크기를 허용하고 추가 스케일 튜닝을 중단했다.
- NS_Impact_1 분석, Adapted Niagara 제작과 공용 User Scale 연결을 P0 범위에서 종료했다.
- UCFCombatFxData v1.2.0의 FxScaleMode와 NiagaraUserScaleParameterName 구현 상태를 문서에 반영했다.
- UCFVehicleData.DestroyedFxSocketName 기본 FX_Destroyed와 SM_Body 소켓 우선 파괴 위치 해석을 반영했다.
- Sedan 차체 메시의 FX_Destroyed 소켓 적용 후 폭발 위치가 해결됐다는 사용자 결과를 기록했다.
- 사용자의 직접 Editor 빌드 PASS 보고를 기록하되 Admin Build Job ID가 없는 사용자 검증으로 구분했다.
- 현재 단계를 후보 튜닝에서 최종 통합 PIE 단일 게이트로 이동했다.
```

### v0.8.0 - 2026-07-25

```text
- ACFCombatFxPreviewActor를 v1.1.0으로 확장했다.
- Editor Viewport 자동 반복 재생, Uniform Scale, Preview 최대 수명 자동 정지와 Override Niagara DataAsset 적용을 추가했다.
- Build Job 89b6d82710af47e89a4c5b3823037d4e에서 UHT, 컴파일과 링크 성공 / Exit Code 0을 확인했다.
- 현재 팩의 Explosion 명명 Niagara가 NS_Explossion과 NS_AOE_Explosion_1 두 개뿐임을 확인했다.
- NS_Explossion은 Loop로 제외하고 NS_AOE_Explosion_1은 수동 Loop·스타일 검토 대기 후보로 유지했다.
- SmokeBuilder에서 NiagaraSystem 0개와 Cascade ParticleSystem 64개를 확인했지만 Niagara 전용 P0 계약에는 자동 채택하지 않았다.
- 현재 AssetDump 프로필이 DataAsset 프로퍼티 값을 노출하지 않아 실제 FX 참조 연결은 Editor 확인이 필요함을 기록했다.
- P0 테스트의 stale ImpactPoint 표현을 실제 코드 계약인 ImpactLocation으로 정정했다.
```

### v0.7.0 - 2026-07-24

```text
- ACFCombatFxPreviewActor EditorOnly 도구를 구현했다.
- ReferenceMesh, PreviewOrigin, +X DirectionArrow와 PreviewNiagara 구성으로 실제 크기·방향 비교가 가능해졌다.
- PIE 없이 Preview Scale, Rotation과 Location을 반복 조정하고 Call In Editor 버튼으로 재생·정지·재시작할 수 있다.
- DataAsset 값 불러오기와 Preview 값을 DataAsset에 적용하는 편집 흐름을 추가했다.
- UCFCombatFxData.MaximumLifetimeSeconds와 UCFCombatFxComp 강제 제거 타이머를 추가해 Loop 오연결의 영구 잔류를 방지했다.
- NS_Explossion은 실제 Loop 반복 확인으로 Destroyed P0 후보에서 제외했다.
- 기준 VehicleData를 DA_TestSedan으로 정정하고 DA_PoliceCar와 존재하지 않는 /Game/CarFight/Data 경로를 현재 지시에서 제거했다.
- /Game/CarFight/FX/Data의 CombatFxData 3개 존재를 AssetDump 3/3 성공으로 확인했다.
- Build Job c9682d4a0be24ead95e87a5a7e8b5849에서 UHT와 Editor 빌드 Exit Code 0을 확인했다.
```

### v0.6.0 - 2026-07-24

```text
- UCFCombatFxData와 UCFCombatFxComp P0 C++ 기반을 구현했다.
- Niagara 시스템, 스케일과 회전 오프셋을 DataAsset에서 교체하는 계약을 추가했다.
- WeaponData 발사 FX, ProjectileData Impact FX와 VehicleData 파괴 FX 참조를 추가했다.
- 승인된 발사만 Muzzle FX를 요청하고 HitScan과 Projectile 첫 Impact가 같은 DamageHitContext 기반 FX 요청을 사용하도록 연결했다.
- VehicleHealthComp 최초 Destroyed 이벤트를 구독하고 파괴 FX 중복 요청을 방지했다.
- FX 미연결과 생성 실패가 전투 판정을 취소하지 않는 null 안전 경로를 구현했다.
- Build Job bd5a50bf388049ec84123aec4942ae96에서 UHT와 Editor 빌드 성공, Exit Code 0을 확인했다.
- 다음 단계를 CombatFxData 3개 생성, 기존 DataAsset 연결과 사용자 PIE로 이동했다.
```

### v0.5.0 - 2026-07-24

```text
- RocketThrusterExhaustFX NiagaraSystem 18개와 sA_Megapack_v1 NiagaraSystem 96개의 AssetDump를 완료했다.
- 합계 114개가 성공하고 실패 0개임을 Phase 0 자산 인벤토리 증거로 기록했다.
- ShootingVfxPack의 ONCE Muzzleflash, Impact, Explosion, Bullet/Rocket Trail과 Rocket Exhaust 계열을 예비 후보로 추출했다.
- RocketThrusterExhaustFX에서는 Realistic, Plume와 Afterburn 계열을 우선 시각 검토 후보로 지정했다.
- 자산 이름만으로 최종 미술 적합성을 확정하지 않고 Editor 재생 검토를 다음 게이트로 유지했다.
```

### v0.4.0 - 2026-07-24

```text
- 사용자 우선순위 변경에 따라 CF-FQ-024를 Ready에서 Active로 전환했다.
- 현재 단계를 Phase 0 FAB FX 팩 1개 반입과 후보 선별로 확정했다.
- CF-FQ-026 TargetSelect는 TS-P0-08 미완료 체크포인트를 보존한 Paused 상태로 유지한다.
- FX 작업은 기존 TargetSelect 코드와 에셋을 되돌리거나 정리하지 않는다.
- 첫 실행 순서를 FAB 팩 반입 → AssetDump → Muzzle / Impact / Destroyed 후보 확정으로 고정했다.
```

### v0.3.0 - 2026-07-24

```text
- 최종 미술 방향을 세미리얼로 확정했다.
- 1인 개발 일정에 맞춰 FAB FX 팩 우선 활용과 직접 신규 제작 최소화 정책을 확정했다.
- 원본 FAB 자산 보존, CarFight 수정본 경로와 최소 보정 범위를 정의했다.
- 고정 위치는 Mesh Socket, 충돌 위치는 World Transform, 런타임 피벗은 SceneComponent로 구분했다.
- Muzzle과 FX_Exhaust 소켓 +X축을 방출 방향으로 통일하고 FAB 축 차이는 RotationOffset으로 보정하도록 했다.
- 소켓 누락 시 UCFProjectileData의 Fallback Relative Transform을 사용하는 계약을 추가했다.
- Projectile 지속형 FX 소유권을 ACFProjectileActor로 분리하고 Pool 반환 전 즉시 정지·초기화하도록 했다.
- Trail 자연 소멸, 다중 노즐, 표면별 Impact와 고급 파괴는 P0에서 제외했다.
- AssetPreparationChecklist.md를 실제 자산 반입과 선별 체크포인트로 추가했다.
```

### v0.2.0 - 2026-07-24

```text
- 사용자 결정에 따라 게임 사운드를 프로젝트 전역에서 제외했다.
- CF-FQ-024를 Combat FX 전용으로 재정의했다.
- UCFCombatFxData / UCFCombatFxComp 후보 구조로 명칭과 책임을 정리했다.
- Sound, Audio, Attenuation 필드와 AudioComponent 생명주기를 제거했다.
- Niagara 1회성 계약과 오디오 참조 0개 완료 조건을 확정했다.
```

---

## 19. Migration

### v0.10.0 적용 안내

```text
- 이 Plan은 CF-FQ-024 완료 당시 설계·자산 선택·튜닝·검증 기록이다.
- 현재 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- CF-FQ-024는 Done / User PIE PASS / CF-TC-021 PASS다.
- Impact P0는 NS_BasicHit 현재 크기를 승인값으로 유지하며 Scale 분석을 다시 열지 않는다.
- 차량별 Destroyed 위치는 SM_Body.FX_Destroyed 소켓을 사용한다.
- 다음 기능은 사용자 선택 전까지 자동 Active 전환하지 않는다.
```

### v0.9.0 적용 안내

```text
- 새 세션은 Impact P0 자산을 NS_BasicHit으로 복원하며 외부 Scale 미반응을 결함 추적 대상으로 다시 열지 않는다.
- DA_FX_ProtoShellImpact의 FxScale은 1,1,1 기준으로 두고 현재 Niagara 내부 크기를 사용한다.
- NS_Impact_1, NS_Impact_Proto와 User.CF_FxScale 연결 작업을 P0에서 재개하지 않는다.
- Destroyed FX 위치는 차량마다 SM_Body StaticMesh의 FX_Destroyed 소켓으로 지정한다.
- DestroyedFxSocketName이 없거나 소켓이 누락되면 SM_Body Bounds 중심 Fallback을 사용한다.
- 사용자가 저장한 Muzzle·Impact·Destroyed DataAsset 연결은 최종 PIE에서 위치·횟수·잔류만 확인한다.
- 최종 PIE PASS 전에는 CF-FQ-024를 Done 또는 Current System으로 승격하지 않는다.
```

### v0.8.0 적용 안내

```text
- 새 세션은 ACFCombatFxPreviewActor v1.1.0과 Build Job 89b6d82710af47e89a4c5b3823037d4e PASS 상태로 복원한다.
- Scale 조정은 기본적으로 PreviewUniformScale 하나로 수행하고 축별 조정이 필요할 때만 Vector Scale을 사용한다.
- 일회성 후보는 자동 반복 재생으로 관찰하고 Loop 후보는 Preview 최대 수명 자동 정지로 잔류를 막는다.
- Override Niagara를 확정한 경우 Preview 값을 DataAsset에 적용 버튼으로 Niagara와 튜닝값을 함께 반영할 수 있다.
- NS_AOE_Explosion_1은 비Loop 또는 최종 후보로 확정된 것이 아니며 Editor 수동 확인 전에는 DA_FX_ProtoVehicleDead에 저장하지 않는다.
- SmokeBuilder는 Cascade 전용이므로 현재 Niagara 데이터·런타임 계약을 확장하지 않는다.
- 현재 AssetDump 결과만으로 기존 Weapon·Projectile·Vehicle DataAsset의 FX 참조값이 연결됐다고 단정하지 않는다.
```

### v0.7.0 적용 안내

```text
- 새 세션은 ACFCombatFxPreviewActor와 MaximumLifetimeSeconds 안전 퓨즈가 구현·빌드 완료된 상태로 복원한다.
- FX 크기·회전·위치 보정은 실제 전투 PIE 반복보다 Preview Actor에서 먼저 수행한다.
- /Game/CarFight/FX/Data의 기존 CombatFxData 3개를 재생성하지 않는다.
- 기준 VehicleData는 DA_TestSedan이며 DA_PoliceCar를 새 FX 연결 대상으로 사용하지 않는다.
- NS_Explossion은 Loop 부적합 후보이므로 DA_FX_ProtoVehicleDead의 최종 Niagara로 유지하지 않는다.
- MaximumLifetimeSeconds는 안전 퓨즈이며 비Loop Destroyed 후보 선정을 대체하지 않는다.
- Preview 값을 DataAsset에 적용한 뒤 사용자가 패키지를 저장하고 최종 PIE를 진행한다.
```

### v0.6.0 적용 안내

```text
- Combat FX C++ 기반과 Niagara 모듈 의존성은 구현 완료 상태로 복원한다.
- UCFCombatFxData 또는 Niagara 미연결은 전투 판정 실패가 아니라 선택적 시각 연출 없음으로 해석한다.
- Muzzle FX는 승인된 FireResult 이후, Impact FX는 DamageHitContext 첫 Blocking Hit, Destroyed FX는 VehicleHealthComp 최초 파괴 이벤트에서 요청한다.
- 다음 단계에서는 소스 코드를 다시 열지 않고 /Game/CarFight/FX/Data의 DataAsset과 기존 게임 DataAsset 참조만 연결한다.
- 사용자 PIE PASS 전에는 CombatFx Systems 문서를 Current로 만들지 않는다.
```

### v0.5.0 적용 안내

```text
- FAB 팩 추가와 NiagaraSystem 전체 목록 조사는 완료됐으므로 반복하지 않는다.
- 현재 자산 인벤토리는 RocketThrusterExhaustFX 18개와 sA_Megapack_v1 96개다.
- 다음 단계는 예비 후보의 실제 화면, 방출 축, Loop, Local/World Space, Bounds와 의존성 검토다.
- 시각 검토가 끝나기 전에는 후보 이름을 최종 CarFight FX로 확정하지 않는다.
- 최종 Muzzle / Impact / Destroyed 후보 확정 뒤 Phase 1 공용 데이터·런타임 계약으로 이동한다.
```

### v0.4.0 적용 안내

```text
- 사용자 우선순위 결정으로 CF-FQ-024는 Active이며 Phase 0 자산 반입부터 진행한다.
- CF-FQ-026은 TS-P0-08 Paused 체크포인트를 유지하며 FX 작업 중 정리·되돌리기·재구현하지 않는다.
- FAB 팩이 반입되기 전에는 C++ 런타임 구현을 시작하지 않는다.
- FX 구현 전 AssetPreparationChecklist.md에서 FAB 팩 1개를 먼저 선별한다.
- 기존 FAB 원본 자산은 직접 수정하지 않고, 필요한 경우 /Game/CarFight/FX/Adapted에 복제한다.
- Projectile 지속형 FX는 UCFCombatFxComp가 아니라 ACFProjectileActor가 활성화·비활성화와 Pool 초기화를 소유한다.
- ProjectileData에 FX 구조 필드가 추가되더라도 기존 자산은 소켓 없음 + Identity Fallback으로 안전하게 유지한다.
- P0에서는 Trail 자연 소멸이나 별도 FX Pool을 구현하지 않는다.
- C++에서 UNiagaraSystem 또는 UNiagaraComponent를 사용하기 전에 CarFight_Re.Build.cs에 Niagara 모듈 의존성을 추가한다.
```

### 기존 적용 안내

```text
- CF-FQ-024 ID는 유지하며 현재 상태는 Active / Phase 0이다.
- CombatFxAudio 디렉터리명은 레거시 경로 호환용으로만 유지한다.
- 기존 CombatFxAudio 이름의 클래스·DataAsset 후보는 사용하지 않는다.
- ImpactEffectId의 SFX 의미를 제거하고 VFX / Decal 식별자로만 읽는다.
- 기존 전투 판정과 UI 완료 상태는 변경하지 않는다.
```
