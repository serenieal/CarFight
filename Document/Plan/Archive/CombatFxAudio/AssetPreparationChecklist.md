# Combat FX Asset Preparation Checklist

- Version: 0.8.0
- Date: 2026-07-27
- Status: Complete / P0 Assets Accepted / User PIE PASS / CF-TC-021 PASS
- Feature ID: `CF-FQ-024`
- Purpose: FAB FX 팩을 CarFight P0 전투 FX에 빠르게 적용하기 위한 자산 반입, 선별, 최소 수정과 착수 준비 기준

---

## 1. 현재 체크포인트

```text
현재 완료: CF-FQ-024 전투 FX / Done / User PIE PASS / CF-TC-021 PASS
CF-FQ-026 상태: Paused / TS-P0-08 체크포인트 보존
최종 미술 방향: 세미리얼
제작 정책: FAB FX 팩 우선 활용 / 직접 신규 제작 최소화
반입 팩 1: /Game/RocketThrusterExhaustFX
- NiagaraSystem 18개 / Dump 성공 18 / 실패 0
반입 팩 2: /Game/sA_Megapack_v1
- NiagaraSystem 96개 / Dump 성공 96 / 실패 0
현재 확인 NiagaraSystem 합계: 114개
Cascade ParticleSystem: 이번 Niagara 전용 조사 범위 밖 / 미확인
C++ Niagara 의존성: 추가 완료
신규 Combat FX Runtime: UCFCombatFxData / UCFCombatFxComp 구현 완료
기존 데이터 참조 필드: WeaponData Fire / ProjectileData Impact / VehicleData Destroyed 구현 완료
공식 Editor Build: Build Job bd5a50bf388049ec84123aec4942ae96 / Exit Code 0
Combat FX DataAsset 인스턴스: /Game/CarFight/FX/Data 아래 3개 확인 / AssetDump 3 성공 0 실패
Editor Preview Actor: ACFCombatFxPreviewActor v1.2.0 구현 완료
- 자동 반복 재생
- Uniform Scale
- 최대 수명 자동 정지
- Override Niagara DataAsset 적용
- ComponentTransform / NiagaraUserVector 전달 모드
Rapid Preview Tuning Build: Build Job 89b6d82710af47e89a4c5b3823037d4e / Exit Code 0
사용자 직접 Editor 빌드: PASS 보고 / Admin Build Job ID 없음
Loop 안전 퓨즈: MaximumLifetimeSeconds 구현 완료
Impact P0 선택: NS_BasicHit / 현재 자산 크기 그대로 사용
Impact Scale 추가 조사: 중단 / 외부 Transform Scale 미반응 허용
기준 VehicleData: DA_TestSedan
기준 차체 메시: Sedan
Destroyed 위치: SM_Body의 FX_Destroyed 소켓 적용 후 사용자 확인 해결
DestroyedFxSocketName 기본값: FX_Destroyed
DA_PoliceCar: 현재 사용하지 않는 폐기 자산
NS_Explossion: Loop 반복 확인 / Destroyed 후보 제외
SmokeBuilder: NiagaraSystem 0 / Cascade ParticleSystem 64 / 현재 P0 미사용
DataAsset 참조 연결: Editor에서 사용자 저장 / AssetDump 프로퍼티 값 미노출 제한 유지
최종 사용자 PIE: PASS
- Muzzle 정상 발사 1회 / 발사 거부 0회 PASS
- NS_BasicHit Impact 실제 위치 / 첫 1회 / 중복·잔류 없음 PASS
- Destroyed FX_Destroyed 위치 / 최초 1회 / 추가 피해 중복·잔류 없음 PASS
- 기존 전투 회귀 PASS
현재 판정: P0 자산 준비 완료 / CF-FQ-024 Done / CF-TC-021 PASS
Current System: Document/Systems/Combat/CombatFx.md
```

이 문서는 FX 현재 구현 문서가 아니라 `CF-FQ-024` 완료 과정에서 사용한 자산 반입·선별·최소 수정 체크리스트다.
현재 런타임 구현 판단은 `Document/Systems/Combat/CombatFx.md`를 우선하며, 이 문서는 P0 자산 선택과 시간 제한 원칙의 완료 기록으로 유지한다.

### Phase 0 AssetDump 결과

```text
/Game/RocketThrusterExhaustFX
- NiagaraSystem: 18
- 성공: 18
- 실패: 0
- 이름 기반 우선 검토: NS_RocketExhaust_Realistic, NS_RocketExhaust_Plume_1~3, NS_RocketExhaust_Afterburn, NS_RocketExhaust_Afterburn_Jet

/Game/sA_Megapack_v1
- NiagaraSystem: 96
- 성공: 96
- 실패: 0
- Muzzle 예비 후보: NS_AR_Muzzleflash_1_ONCE, NS_AR_Muzzleflash_2_ONCE, NS_Muzzle 계열
- Impact 예비 후보: NS_Impact_1, NS_Impact_2, NS_Hit 계열
- Destroyed 조사 결과: NS_Explossion은 Loop로 제외, NS_AOE_Explosion_1은 Stylized 성격과 Loop 여부를 추가 확인해야 하는 보류 후보
- Trail 예비 후보: NS_BulletTrail_1~3, NS_ROCKET_Trail, NS_GRENADE_RibbonTrail
```

판정 제한:

```text
- 위 목록은 자산명과 폴더 역할을 기준으로 추출한 예비 후보다.
- 실제 화면을 확인하지 않았으므로 세미리얼 적합성이나 최종 채택을 의미하지 않는다.
- INFINITE Muzzleflash는 일회성 Muzzle P0 후보에서 우선 제외한다.
- StylizedAttacksPack 계열은 세미리얼 방향과 충돌할 가능성이 있으므로 우선순위를 낮춘다.
```

### 첫 연결에 사용할 기술 후보

```text
Muzzle Niagara:
/Game/sA_Megapack_v1/sA_ShootingVfxPack/FX/NiagaraSystems/NS_AR_Muzzleflash_1_ONCE

Impact Niagara:
/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_BasicHit
- 사용자 P0 선택 확정
- 외부 Transform Scale 미반응을 허용하고 현재 크기 그대로 사용

Destroyed Niagara:
- NS_Explossion 제외: Loop 반복으로 P0 일회성 계약 위반
- 사용자가 DA_FX_ProtoVehicleDead에 저장한 현재 후보를 유지
- 차량별 위치는 Niagara 오프셋이 아니라 SM_Body.FX_Destroyed 소켓으로 해결
```

Muzzle과 Impact 자산은 최종 미술 확정이 아니라 런타임 연결과 크기·방향·횟수 검증을 위한 첫 기술 후보다.
Destroyed 자산은 현재 비Loop 대체 후보가 미확정이다. 각 역할은 두 번의 최소 보정으로 세미리얼 방향에 맞지 않으면 같은 역할의 다음 후보로 교체한다.

### 현재 존재하는 CarFight CombatFxData

```text
/Game/CarFight/FX/Data/DA_FX_ProtoWeaponFire
- CombatFxId: ProtoWeaponFire
- NiagaraSystem: NS_AR_Muzzleflash_1_ONCE

/Game/CarFight/FX/Data/DA_FX_ProtoShellImpact
- CombatFxId: ProtoShellImpact
- NiagaraSystem: NS_BasicHit
- FxScale: 1,1,1 기준 / 현재 Niagara 내부 크기 사용
- 공용 Scale 추가 튜닝: P0 중단

/Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead
- CombatFxId: ProtoVehicleDestroyed
- NiagaraSystem: 사용자가 현재 저장한 비Loop 후보 유지
- MaximumLifetimeSeconds: 5.0 기본 안전 퓨즈
- Spawn 위치: DA_TestSedan ChassisMesh의 FX_Destroyed 소켓
```

### 연결할 기존 게임 DataAsset

```text
/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon
- DefaultFireFxData = DA_FX_ProtoWeaponFire

/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell
- DefaultImpactFxData = DA_FX_ProtoShellImpact

/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan
- DefaultDestroyedFxData = DA_FX_ProtoVehicleDead
```

`BP_CFVehiclePawn.VehicleData = DA_TestSedan`이 현재 기준이다. `DA_PoliceCar`는 사용하지 않는 폐기 자산이며 새 FX 연결 대상으로 사용하지 않는다.

---

## 2. 시간 제한 원칙

```text
- FAB 팩은 가장 유력한 1개만 먼저 프로젝트에 추가한다.
- 처음부터 여러 팩을 동시에 비교하지 않는다.
- 각 FX는 원본 연결 → 크기·방향 보정 → 밝기·수명 보정의 최대 2회 수정으로 판정한다.
- 세 번째 대규모 수정이 필요하면 직접 고치지 않고 다른 후보로 교체한다.
- 외부 Scale 미반응 자산이 현재 플레이에 허용 가능하면 P0에서는 내부 구조 분석을 중단하고 그대로 사용한다.
- P0에서는 고급 Material, Flipbook, Niagara Module과 파편 시스템을 신규 제작하지 않는다.
- 미술 품질보다 위치·방향·횟수·가독성·잔류 없음의 기능 계약을 우선한다.
```

---

## 3. FAB 팩 반입 전 기록

팩을 프로젝트에 추가하기 전에 아래 정보를 기록한다.

```text
팩 이름:
FAB 라이브러리 표시 이름:
제작자 또는 공급자:
팩 버전:
지원 Unreal Engine 버전:
Niagara 기반 여부:
Cascade 포함 여부:
샘플 맵 포함 여부:
콘텐츠 플러그인 여부:
예상 프로젝트 경로:
라이선스 또는 사용 조건 확인 메모:
선택 이유:
```

PASS 기준:

```text
- 현재 엔진 버전에서 프로젝트에 추가 가능한 팩이다.
- 전투용 Muzzle, Impact 또는 Explosion 후보가 하나 이상 있다.
- 특정 샘플 Level Blueprint에만 의존하지 않는다.
- 런타임 코드 플러그인 설치가 필수인 팩은 우선순위를 낮춘다.
```

---

## 4. 프로젝트 반입 직후 확인

### 4.1 자산 위치

```text
FAB 원본 경로:
/Game/<VendorOrPack>/...
```

원본 폴더는 이름 변경, 이동 또는 직접 수정하지 않는다.

CarFight 수정본이 필요한 경우에만 다음 경로로 복제한다.

```text
/Game/CarFight/FX/Adapted/...
```

CarFight FX DataAsset은 다음 경로를 사용한다.

```text
/Game/CarFight/FX/Data/...
```

### 4.2 AssetDump 확인

반입 후 최소 확인:

```text
- /Game 전체 NiagaraSystem 수
- FAB 팩 경로의 NiagaraSystem 목록
- Cascade ParticleSystem 포함 여부
- 후보 Niagara의 직접 의존성
- Demo Blueprint 또는 Demo Map 전용 참조 여부
```

### 4.3 프로젝트 이상 여부

```text
[ ] 프로젝트 로드 성공
[ ] 에셋 누락 경고 없음
[ ] 플러그인 누락 경고 없음
[ ] 대량 Redirector 또는 경로 오류 없음
[ ] 불필요한 Project Settings 변경 없음
[ ] 게임 사운드 자산과 Audio 런타임을 CarFight 기능에 연결하지 않음
```

팩 안에 Sound 자산이 포함되어 있어도 CarFight에서 참조하지 않는다.
팩 원본에 포함된 Sound 자산을 삭제하기 위한 별도 정리 작업은 P0에 포함하지 않는다.

---

## 5. P0 후보 선별표

| 역할 | 필수 여부 | 선택 Niagara | 원본 경로 | 판정 |
|---|---:|---|---|---|
| Muzzle | 필수 | `NS_AR_Muzzleflash_1_ONCE`, `NS_AR_Muzzleflash_2_ONCE`, `NS_Muzzle` 계열 | `/Game/sA_Megapack_v1/...` | Visual Review Pending |
| Vehicle Impact | 필수 | `NS_BasicHit` | `/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_BasicHit` | P0 Selected / Use As-Is |
| World Impact | 조건부 | P0에서는 `NS_BasicHit` 공용 | `/Game/sA_Megapack_v1/...` | P0 Shared |
| Vehicle Destroyed | 필수 | 사용자가 현재 DA에 저장한 비Loop 후보 / `NS_Explossion` 제외 | DataAsset 참조 기준 | P0 Selected / Socket Position Resolved |
| Projectile Trail | 조건부 | `NS_BulletTrail_1~3`, `NS_ROCKET_Trail` | `/Game/sA_Megapack_v1/...` | Visual Review Pending |
| Missile Exhaust | 조건부 | `NS_RocketExhaust_Realistic`, `Plume_1~3`, `Afterburn` 계열 | `/Game/RocketThrusterExhaustFX/FX/...` | Visual Review Pending |

P0 최소 연결은 다음 세 가지다.

```text
- Muzzle 1개
- Impact 1개
- Destroyed 1개
```

Vehicle Impact와 World Impact 후보가 모두 적합하면 분리한다.
적합한 후보가 하나뿐이면 P0에서는 범용 Impact 하나로 먼저 전투 루프를 닫는다.

---

## 6. 후보 Niagara 검사 항목

각 후보마다 아래를 기록한다.

```text
자산 이름:
역할:
원본 경로:
Niagara System 또는 Cascade:
기본 방출 축:
Looping 여부:
Auto Activate 여부:
Auto Destroy 가능 여부:
Local Space Emitter:
World Space Emitter:
Fixed Bounds 여부:
Dynamic Light 사용 여부:
Collision Particle 사용 여부:
Mesh Particle 사용 여부:
Ribbon 사용 여부:
User Parameter 목록:
Demo Blueprint 종속 여부:
Post Process 종속 여부:
원본 그대로 사용 가능 여부:
CarFight 복제 필요 여부:
제외 사유:
```

### 6.1 즉시 제외 후보

```text
- 무한 Loop가 기본인 일회성 폭발
- NS_Explossion: 사용자 실제 재생에서 Loop 반복 확인
- Demo Controller가 없으면 재생할 수 없는 시스템
- 매우 많은 Dynamic Light를 생성하는 시스템
- 대량 Collision Particle 또는 물리 Mesh Particle을 사용하는 시스템
- 특정 Post Process가 없으면 형태가 사라지는 시스템
- 원본 구조를 크게 다시 만들어야 CarFight에 맞는 시스템
- 세미리얼 방향에서 크게 벗어나는 과도한 네온·판타지 형태
```

---

## 7. Editor Preview Actor 사용

클래스:

```text
ACFCombatFxPreviewActor
```

사용 순서:

```text
1. TestMap에 C++ 클래스 ACFCombatFxPreviewActor를 배치한다.
2. ReferenceMesh에 크기 비교용 차량 차체 또는 무기 메시를 지정한다.
3. PreviewCombatFxData에 기존 DA_FX 자산을 지정한다.
4. 후보 직접 비교가 필요하면 PreviewNiagaraSystemOverride를 지정한다.
5. 기본적으로 bUseUniformScale=true에서 PreviewUniformScale 하나만 조정한다.
6. bAutoReplayPreview=true와 PreviewReplayIntervalSeconds=2.0으로 일회성 FX를 반복 관찰한다.
7. Loop 후보는 bStopPreviewAtMaximumLifetime=true로 자동 정지시킨다.
8. 회전과 위치가 필요할 때 PreviewRotationOffset과 PreviewLocationOffset을 조정한다.
9. 후보가 확정되면 bApplyOverrideNiagaraToDataAsset=true 상태에서 Preview 값을 DataAsset에 적용 버튼을 누른다.
10. DataAsset 패키지를 저장한다.
11. 최종 게임 PIE는 발생 위치·횟수와 회귀만 확인한다.
```

버튼:

```text
미리보기 재생
미리보기 재시작
미리보기 정지
DataAsset 값 불러오기
Preview 값을 DataAsset에 적용
```

주의:

```text
- Actor는 EditorOnly이므로 게임 전투 Actor로 사용하지 않는다.
- DataAsset 적용 버튼은 자동 저장하지 않고 Dirty 처리만 한다.
- +X Arrow는 CarFight FX 방출 방향 기준이다.
- MaximumLifetimeSeconds는 Loop 잔류 방지 안전 퓨즈이며 Loop 자산 채택 근거가 아니다.
```

---

## 8. 최소 수정 허용 범위

P0에서 수정 가능한 항목:

```text
- 전체 Scale
- Rotation Offset
- 핵심 섬광과 파티클 수명
- Spawn Count 또는 Burst Count
- Emissive 강도
- Looping / Auto Activate / Auto Destroy
- Local Space / World Space 확인과 최소 보정
- Fixed Bounds 크기
- Dynamic Light 비활성화 또는 수량 축소
```

P0에서 하지 않는 항목:

```text
- 신규 Flipbook 제작
- 신규 연기 Material 제작
- 커스텀 Niagara Module 제작
- 여러 팩의 Emitter 대규모 혼합
- 고급 파편 물리 제작
- 완성형 Geometry Collection 파괴
- 표면 재질별 다수 Impact 세트 제작
```

---

## 9. FX 위치와 축 확인

### 9.1 고정 위치

```text
Muzzle
→ TurretPitchMesh의 Muzzle Socket 우선

Missile Exhaust / Projectile Trail
→ Projectile Mesh의 FX_Exhaust Socket 우선
```

CarFight 소켓 규칙:

```text
+X = FX 방출 방향
+Z = FX 위쪽 방향
```

FAB Niagara의 방출 축이 다르면 CarFight 소켓을 수정하지 않고 FX DataAsset의 RotationOffset으로 보정한다.

### 9.2 소켓 Fallback

```text
소켓 존재
→ 소켓 Transform 사용

소켓 누락
→ UCFProjectileData.TrailFallbackRelativeTransform 사용
```

P0에서는 모든 외부 Projectile Mesh에 소켓을 강제하지 않는다.
Fallback 위치로 충분한 메시에는 원본 수정 없이 진행한다.
정밀한 부착이 필요한 메시만 CarFight 폴더로 복제한 뒤 `FX_Exhaust` 소켓을 추가한다.

### 9.3 런타임 위치

```text
Impact
→ FCFDamageHitContext.ImpactLocation / ImpactNormal

Destroyed 일회성 폭발
→ 최초 Destroyed 전환 시점의 SM_Body.FX_Destroyed 소켓 월드 Transform
→ 소켓 누락 시 SM_Body Bounds 중심 Fallback
```

Impact와 일회성 Destroyed FX는 Projectile 또는 차량 Pool 이동 위치를 추적하지 않고 발생 시점의 월드 Transform을 복사해 사용한다.

---

## 10. Projectile 지속 FX 준비

Projectile 추진 화염과 Trail은 `ACFProjectileActor`가 직접 소유한다.

예정 구조:

```text
ACFProjectileActor
├─ CollisionComponent
├─ MeshComponent
├─ ProjectileMovementComponent
└─ ProjectileTrailFxComp
```

P0 생명주기:

```text
ActivateProjectile
→ FX 소켓 또는 Fallback 해석
→ Niagara Reset
→ Niagara Activate

Impact / LifeExpired / Manual / InvalidActivation
→ Niagara Deactivate Immediate
→ Niagara Reset
→ Actor Hide
→ Projectile Pool 반환
```

P0에서는 기존 Trail 입자의 자연 소멸을 위해 FX를 Actor에서 분리하지 않는다.
즉시 제거가 시각적으로 거슬리는 경우에만 P1 품질 후보로 기록한다.

확인 항목:

```text
[ ] 이전 Ribbon이 재사용 Projectile까지 이어지지 않음
[ ] 비활성 Projectile에서 Niagara Tick이 남지 않음
[ ] 재활성화 시 FX가 처음부터 정상 재생됨
[ ] Projectile Mesh 표시 스케일이 FX 크기를 의도치 않게 증폭하지 않음
[ ] 고속 이동 중 Bounds 문제로 FX가 사라지지 않음
```

---

## 11. C++ 착수 전 준비

코드 구현 전에 다음을 확정한다.

```text
[ ] UCFCombatFxData 최종 필드 목록
[ ] UCFCombatFxComp 소유자와 일회성 FX 함수 시그니처
[ ] UCFProjectileData의 TrailAttachSocketName
[ ] UCFProjectileData의 TrailFallbackRelativeTransform
[ ] ACFProjectileActor의 ProjectileTrailFxComp 생명주기
[ ] Impact FX 호출 시점이 Pool 반환 전인지 확인
[ ] FX 누락이 전투 판정을 취소하지 않는 null-safe 계약
[ ] CarFight_Re.Build.cs Niagara 모듈 의존성 추가 계획
[ ] Debug Summary에 자산, 소켓, Fallback과 활성 상태 표시 계획
```

---

## 12. P0 DataAsset 준비

현재 확인 자산:

```text
/Game/CarFight/FX/Data/DA_FX_ProtoWeaponFire
/Game/CarFight/FX/Data/DA_FX_ProtoShellImpact
/Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead
```

AssetDump 결과는 3개 성공, 0개 실패다. 이 자산들은 재생성하지 않고 기존 인스턴스를 조정한다.

선택 자산을 직접 원본 이름에 묶지 않는다.
DataAsset을 통해 참조해 FAB 팩 교체 시 C++와 Weapon 판정 코드를 수정하지 않도록 한다.

기록 양식:

```text
DataAsset:
FxProfileId:
Niagara 원본 또는 복제 경로:
Scale:
RotationOffset:
Socket 또는 World Transform 기준:
Fallback Transform:
적용 대상 WeaponData / ProjectileData / VehicleData:
```

---

## 13. P0 테스트 준비

### 기능 테스트

```text
[ ] 정상 발사 Muzzle FX 1회
[ ] 발사 거부 Muzzle FX 0회
[ ] HitScan 첫 Impact FX 1회
[ ] Projectile 첫 Impact FX 1회
[ ] OnComponentHit / 보조 Sweep 중복 FX 없음
[ ] 최초 Destroyed FX 1회
[ ] 추가 피해 Destroyed FX 중복 없음
[ ] FX 자산 누락 시 전투 판정 유지
```

### 자산 테스트

```text
[ ] 밝은 배경에서 중심 형태가 읽힘
[ ] 어두운 배경에서 Bloom이 화면을 덮지 않음
[ ] Muzzle 방향이 실제 발사 방향과 일치
[ ] Impact 방향이 표면 노멀과 일치
[ ] Missile Exhaust 위치가 메시 뒤쪽에 고정
[ ] 고속 이동과 카메라 회전에서 Bounds 소실 없음
[ ] 10회 이상 Pool 재사용 후 Trail 잔류 없음
```

### 범위 제한

```text
- Muzzle, Impact와 Destroyed가 정상 연결되면 Trail 품질이 부족해도 P0 완료 가능
- World / Vehicle Impact 분리는 적합한 FAB 후보가 있을 때만 P0에 포함
- Trail 자연 소멸, 다중 노즐, 지속 파괴 연기와 표면별 Impact는 P1 이후
```

---

## 14. 준비 완료 판정

다음이 모두 충족되면 P0 자산 준비와 최종 PIE 단계로 진행할 수 있다.

```text
- FAB FX 팩 1개가 프로젝트에 정상 반입됨
- NiagaraSystem 후보 목록을 AssetDump로 확인함
- Muzzle, Impact와 Destroyed 후보를 각각 1개 확정함
- Trail 또는 Missile Exhaust의 P0 포함 여부를 결정함
- 원본 사용과 CarFight 복제 여부를 결정함
- 각 후보의 방출 축, Loop, 공간, Bounds와 의존성을 기록함
- UCFCombatFxData와 Projectile 구조 필드가 확정됨
- Paused 상태인 CF-FQ-026 코드·에셋과 기존 미커밋 변경을 건드리지 않음
```

---

## 15. Changelog

### v0.8.0 - 2026-07-27

```text
- 최종 사용자 PIE에서 Muzzle, Impact와 Destroyed P0 자산을 모두 승인했다.
- 정상 발사 1회와 발사 거부 0회, Impact 첫 1회와 중복·잔류 없음, Destroyed 소켓 위치와 최초 1회를 PASS했다.
- 기존 전투 회귀 PASS와 CF-TC-021 PASS를 기록했다.
- CF-FQ-024를 Done으로 전환하고 자산 준비 체크리스트를 Complete 상태로 닫았다.
- 현재 구현 기준을 Document/Systems/Combat/CombatFx.md로 연결했다.
```

### v0.7.0 - 2026-07-27

```text
- Impact P0 자산을 NS_BasicHit으로 사용자 확정하고 현재 크기 그대로 사용하도록 고정했다.
- 외부 Transform Scale 미반응을 허용하고 NS_Impact_1 및 공용 Scale 추가 튜닝을 중단했다.
- DA_FX_ProtoShellImpact의 현재 기준을 NS_BasicHit / FxScale 1,1,1로 갱신했다.
- Destroyed FX 위치를 차량별 SM_Body.FX_Destroyed 소켓으로 지정하는 구조를 반영했다.
- DA_TestSedan의 Sedan 차체 소켓 적용 후 폭발 위치 해결 사용자 결과를 기록했다.
- 현재 단계를 후보 검토에서 최종 통합 PIE 단일 게이트로 이동했다.
```

### v0.6.0 - 2026-07-25

```text
- ACFCombatFxPreviewActor v1.1.0의 자동 반복, Uniform Scale, 최대 수명 자동 정지와 Override Niagara 적용을 반영했다.
- Build Job 89b6d82710af47e89a4c5b3823037d4e의 UHT·컴파일·링크 PASS와 Exit Code 0을 기록했다.
- Explosion 명명 Niagara가 NS_Explossion과 NS_AOE_Explosion_1 두 개뿐임을 조사했다.
- NS_AOE_Explosion_1을 자동 승인하지 않고 수동 Loop·세미리얼 확인 후보로 유지했다.
- SmokeBuilder의 NiagaraSystem 0개와 Cascade ParticleSystem 64개를 기록하고 현재 Niagara P0 계약에서 제외했다.
- 현재 AssetDump 프로필이 CombatFxData와 참조 DataAsset의 프로퍼티 값을 노출하지 않는 검증 한계를 기록했다.
```

### v0.5.0 - 2026-07-24

```text
- ACFCombatFxPreviewActor EditorOnly 도구와 Call In Editor 조정 흐름을 추가했다.
- PIE를 반복 종료하지 않고 Scale, Rotation과 Location을 조정하고 DataAsset에 적용할 수 있도록 했다.
- UCFCombatFxData.MaximumLifetimeSeconds와 런타임 강제 제거 안전 퓨즈를 기록했다.
- /Game/CarFight/FX/Data의 CombatFxData 3개 존재를 AssetDump 3/3 성공으로 확인했다.
- 기준 VehicleData를 DA_TestSedan으로 정정하고 DA_PoliceCar를 현재 연결 대상에서 제거했다.
- 존재하지 않는 /Game/CarFight/Data 경로를 /Game/CarFight/FX/Data로 정정했다.
- NS_Explossion을 Loop 반복으로 Destroyed 후보에서 제외하고 비Loop 대체 후보 미확정 상태로 전환했다.
- Build Job c9682d4a0be24ead95e87a5a7e8b5849의 Editor 빌드 Exit Code 0을 기록했다.
```

### v0.4.0 - 2026-07-24

```text
- UCFCombatFxData, UCFCombatFxComp와 Niagara 모듈 의존성의 Editor 빌드 성공을 기록했다.
- 공식 성공 증거를 Build Job bd5a50bf388049ec84123aec4942ae96 / Exit Code 0으로 고정했다.
- 첫 기술 후보를 Muzzle NS_AR_Muzzleflash_1_ONCE, Impact NS_Impact_1, Destroyed NS_Explossion으로 지정했다.
- 생성할 CombatFxData 3개의 경로, ID와 Niagara 연결값을 확정했다.
- DA_ProtoTurretCannon, DA_HeavyShell과 현재 기준 DA_PoliceCar의 FX 참조 연결 위치를 확정했다.
- 중복 DA_PoliceCar 중 _Legacy와 과거 Data/Cars 경로는 수정하지 않도록 보호 규칙을 추가했다.
```

### v0.3.0 - 2026-07-24

```text
- 반입된 FAB 팩 2개의 NiagaraSystem AssetDump를 완료했다.
- RocketThrusterExhaustFX 18개와 sA_Megapack_v1 96개, 합계 114개가 성공하고 실패 0개임을 기록했다.
- Muzzle, Impact, Destroyed, Trail과 Missile Exhaust 이름 기반 예비 후보를 선별표에 등록했다.
- INFINITE Muzzleflash와 Stylized 계열을 P0 우선 후보에서 낮추는 판정 제한을 추가했다.
- 현재 단계를 Asset Intake Pending에서 Asset Inventory Complete / Visual Review Pending으로 이동했다.
```

### v0.2.0 - 2026-07-24

```text
- 사용자 우선순위 결정에 따라 체크리스트를 CF-FQ-024 Phase 0의 Active 실행 문서로 전환했다.
- CF-FQ-026 TargetSelect는 TS-P0-08 Paused 상태로 보존하도록 갱신했다.
- 현재 다음 작업을 가장 유력한 FAB FX 팩 1개 프로젝트 반입으로 지정했다.
```

### v0.1.0 - 2026-07-24

```text
- 세미리얼 최종 미술 방향과 FAB 우선 제작 정책에 맞춘 자산 반입 체크리스트를 생성했다.
- P0 후보 선별, 최소 수정 허용 범위와 즉시 제외 기준을 정의했다.
- Muzzle과 FX_Exhaust 소켓, Fallback Transform과 +X 방출 축 기준을 기록했다.
- Projectile 지속형 FX의 Pool 생명주기와 즉시 제거 P0 정책을 기록했다.
- C++ 착수 전 준비, DataAsset 연결과 P0 테스트 준비 조건을 정리했다.
```

---

## 16. Migration

### v0.8.0 적용 안내

```text
- 이 체크리스트는 CF-FQ-024 P0 자산 선택 완료 기록으로 유지한다.
- 현재 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
- Impact P0는 NS_BasicHit을 현재 크기 그대로 사용한다.
- 차량별 Destroyed 위치는 SM_Body.FX_Destroyed 소켓으로 관리한다.
- CF-TC-021은 PASS이며 후보 조사와 Scale 튜닝을 P0 작업으로 다시 열지 않는다.
```

### v0.7.0 적용 안내

```text
- Impact P0는 NS_BasicHit을 사용하며 외부 Scale 미반응을 다시 조사하지 않는다.
- DA_FX_ProtoShellImpact는 FxScale 1,1,1 기준으로 두고 Niagara 내부 크기를 그대로 사용한다.
- NS_Impact_1, NS_Impact_Proto와 User Scale 연결 작업은 P0에서 재개하지 않는다.
- 각 차량 ChassisMesh에는 동일한 FX_Destroyed 소켓 이름을 사용하고 위치만 차량별로 배치한다.
- UCFVehicleData.DestroyedFxSocketName 기본값 FX_Destroyed를 유지한다.
- 최종 PIE에서는 위치·1회성·중복·잔류만 검증하고 미술 세부 튜닝은 다시 열지 않는다.
```

### v0.6.0 적용 안내

```text
- Preview Actor는 v1.1.0이며 자동 반복과 Uniform Scale을 우선 사용한다.
- Override 후보를 확정한 경우 DataAsset 적용 버튼으로 Niagara와 튜닝값을 함께 반영할 수 있다.
- NS_AOE_Explosion_1은 수동 확인 전까지 최종 Destroyed 후보로 기록하지 않는다.
- SmokeBuilder Cascade 자산을 사용하기 위해 현재 Niagara 전용 C++ 계약을 확장하지 않는다.
- DefaultFireFxData, DefaultImpactFxData와 DefaultDestroyedFxData 연결은 Editor Details에서 직접 확인한다.
- DataAsset 적용 후 패키지를 저장하고 최종 PIE를 수행한다.
```

### v0.5.0 적용 안내

```text
- 기존 CombatFxData 3개를 재생성하지 않는다.
- FX 튜닝은 먼저 ACFCombatFxPreviewActor에서 수행하고 최종 PIE 횟수를 줄인다.
- 현재 기준 차량 데이터는 DA_TestSedan이며 DA_PoliceCar를 수정하지 않는다.
- NS_Explossion은 Loop 부적합 후보로 유지하고 비Loop Destroyed 후보를 새로 선택한다.
- MaximumLifetimeSeconds는 안전 퓨즈로만 사용하며 Loop 자산을 P0 완료 후보로 승인하지 않는다.
- Preview 값을 DataAsset에 적용한 뒤 반드시 패키지를 저장한다.
```

### v0.4.0 적용 안내

```text
- C++와 Editor 빌드는 완료됐으므로 다시 구현하거나 빌드 원인을 조사하지 않는다.
- 다음 작업은 세 CombatFxData 생성과 정확한 기존 DataAsset 참조 연결이다.
- 첫 Niagara 후보는 기술 연결용이며 사용자 PIE에서 크기·방향·반복·잔류와 세미리얼 적합성을 확인한 뒤 교체할 수 있다.
- DA_PoliceCar는 /Game/CarFight/Vehicles/Data/Cars 경로만 현재 연결 대상으로 사용한다.
- DataAsset 연결 전에는 FX가 보이지 않는 것이 정상이며 전투 판정 FAIL로 처리하지 않는다.
```

### v0.3.0 적용 안내

```text
- FAB 팩 추가와 NiagaraSystem 수량 조사는 완료됐으므로 반복하지 않는다.
- 예비 후보는 이름 기반이며 Unreal Editor 재생 검토 전에는 최종 선택으로 해석하지 않는다.
- 다음 단계는 Muzzle, Impact, Destroyed와 Missile Exhaust 후보의 화면·축·Loop·Space·Bounds 확인이다.
- 최종 후보 확정 후에만 UCFCombatFxData와 UCFCombatFxComp 구현으로 이동한다.
```

### 기존 적용 안내

```text
- 이 문서는 현재 CF-FQ-024 Phase 0의 Active 실행 체크리스트다.
- CF-FQ-026 TargetSelect는 TS-P0-08 Paused 상태로 보존하며 FX 작업에서 수정하지 않는다.
- FAB 팩이 프로젝트에 들어오기 전에는 후보 자산명과 성능을 추정으로 확정하지 않는다.
- 자산 반입 후 AssetDump 결과를 이 문서에 기록하고 P0 후보를 확정한다.
- 구현 완료 후 현재 시스템 기준은 Document/Systems/Combat/CombatFx.md로 승격한다.
```
