# CombatFx

- Version: 1.0.1
- Date: 2026-07-28
- Status: Current System / P0 User PIE Verified
- Feature: `CF-FQ-024 전투 FX`
- Test: `CF-TC-021 PASS`
- Scope: 승인된 발사, 첫 유효 Impact와 최초 차량 파괴 결과를 데이터 기반 Niagara 시각 연출로 정확히 한 번 표현하는 현재 구현 기준

---

## 1. 문서 목적

이 문서는 CarFight의 현재 전투 FX 시스템이 실제로 어떤 판정 결과를 입력받고, 어떤 DataAsset과 Niagara를 사용하며, 발사·Impact·차량 파괴 연출을 어느 위치에서 몇 번 재생하는지 기록한다.

이 문서는 미래 설계서가 아니다.
`CF-FQ-024` 구현과 최종 사용자 PIE에서 확인된 현재 동작을 기준으로 한다.

현재 CombatFx의 핵심 목적은 다음과 같다.

> 이미 확정된 발사·충돌·피해·파괴 판정을 다시 계산하지 않고, 그 결과를 화면에서 읽을 수 있는 일회성 Niagara FX로 변환한다.

프로젝트 결정 `CF-PDL-0009`에 따라 CarFight는 게임 사운드를 지원하지 않는다.
따라서 이 시스템에는 Sound, AudioComponent, 공간화, 감쇠와 청각 테스트 책임이 없다.

---

## 2. 현재 완료 상태

```text
Feature ID: CF-FQ-024
Feature 상태: Done
사용자 PIE: PASS
테스트 ID: CF-TC-021
현재 구현 문서: Document/Systems/Combat/CombatFx.md
완료 Plan: Document/Plan/CombatFxAudio/ImplementationDesign.md
자산 준비 기록: Document/Plan/CombatFxAudio/AssetPreparationChecklist.md
```

최종 사용자 PIE 결과:

```text
Muzzle
- 정상 발사에서 1회 발생: PASS
- 발사 거부에서 0회 발생: PASS

Impact
- 실제 충돌 위치에서 발생: PASS
- 첫 유효 충돌당 1회 발생: PASS
- 중복 또는 잔류 없음: PASS

Destroyed
- SM_Body.FX_Destroyed 소켓 위치에서 발생: PASS
- 최초 파괴 전환에서 1회 발생: PASS
- 추가 피해 중복 또는 잔류 없음: PASS

기존 전투 회귀
- 조준·발사·피격·피해·파괴 흐름: PASS
```

---

## 3. 현재 구현 구성

### 3.1 C++ 클래스

| 클래스 | 현재 역할 |
| --- | --- |
| `UCFCombatFxData` | 한 종류의 일회성 Niagara FX, Scale 적용 방식, 회전 보정과 최대 수명을 제공한다. |
| `UCFCombatFxComp` | 승인된 Fire, 첫 Impact와 최초 Destroyed 결과를 월드 위치 Niagara로 재생한다. |
| `ACFCombatFxPreviewActor` | Editor Viewport에서 PIE 없이 Niagara 후보를 반복 재생하고 DataAsset 값을 조정하는 EditorOnly 도구다. |
| `ACFVehiclePawn` | 실제 발사·HitScan·Projectile·차량 파괴 판정 지점에서 CombatFxComp를 호출한다. |
| `UCFWeaponData` | `DefaultFireFxData`로 발사 FX를 선택한다. |
| `UCFProjectileData` | `DefaultImpactFxData`로 Impact FX를 선택한다. |
| `UCFVehicleData` | `DefaultDestroyedFxData`와 `DestroyedFxSocketName`으로 파괴 FX와 차량별 위치를 선택한다. |

### 3.2 소스 파일

```text
UE/Source/CarFight_Re/Public/CFCombatFxData.h
UE/Source/CarFight_Re/Private/CFCombatFxData.cpp
UE/Source/CarFight_Re/Public/CFCombatFxComp.h
UE/Source/CarFight_Re/Private/CFCombatFxComp.cpp
UE/Source/CarFight_Re/Public/CFCombatFxPreviewActor.h
UE/Source/CarFight_Re/Private/CFCombatFxPreviewActor.cpp
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Public/CFVehicleData.h
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

Niagara C++ 의존성은 `CarFight_Re.Build.cs`에 반영되어 있다.

---

## 4. C++와 Unreal Editor 책임 분리

### 4.1 C++ 책임

```text
- 발사가 실제로 승인·실행된 시점을 확정한다.
- HitScan과 Projectile의 첫 유효 Blocking Hit을 확정한다.
- 차량이 최초 Destroyed 상태로 전환된 시점을 확정한다.
- CombatFxData 참조와 Niagara 유효성을 검사한다.
- 실제 월드 위치와 회전을 계산한다.
- 발사 거부, 충돌 중복과 파괴 중복에서 FX 재생을 막는다.
- Loop 오연결이 영구 잔류하지 않도록 최대 수명 안전 종료를 적용한다.
- FX 재생 실패가 전투 판정에 영향을 주지 않게 null-safe로 처리한다.
- Debug 요약, 발생 횟수와 파괴 위치 해석 출처를 제공한다.
```

### 4.2 Unreal Editor와 DataAsset 책임

```text
- 각 역할에 사용할 Niagara System을 선택한다.
- Niagara 원본 또는 CarFight 복제본의 크기·회전·수명·밝기·Bounds를 조정한다.
- UCFCombatFxData 인스턴스를 생성하고 Niagara를 연결한다.
- WeaponData, ProjectileData와 VehicleData에 CombatFxData를 연결한다.
- 차량 StaticMesh에 FX_Destroyed 소켓을 배치한다.
- Preview Actor로 후보를 비교하고 DataAsset을 저장한다.
- 최종 PIE에서 시각 위치, 발생 횟수와 잔류 여부를 확인한다.
```

판정 로직, 피해 계산과 생존 상태를 Blueprint 또는 Niagara에 중복 작성하지 않는다.

---

## 5. CombatFxData 계약

`UCFCombatFxData`는 발사, Impact 또는 Destroyed 역할 하나에 사용할 Niagara 설정을 제공한다.

현재 필드:

| 필드 | 타입 | 현재 의미 |
| --- | --- | --- |
| `CombatFxId` | `FName` | Debug와 데이터 구분에 사용할 안정적인 ID다. |
| `NiagaraSystem` | `UNiagaraSystem*` | 실제 재생할 Niagara System이다. 비어 있으면 시각 FX만 생략한다. |
| `FxScale` | `FVector` | 선택한 Scale 적용 모드에 전달할 값이다. |
| `FxScaleMode` | `ECFCombatFxScaleMode` | Component Transform 또는 Niagara User Vector 전달 방식을 선택한다. |
| `NiagaraUserScaleParameterName` | `FName` | User Vector 방식에서 값을 전달할 Niagara User Parameter 이름이다. 기본값은 `User.CF_FxScale`이다. |
| `RotationOffset` | `FRotator` | CarFight 기준 방향과 FAB Niagara 원본 축 차이를 보정한다. |
| `MaximumLifetimeSeconds` | `float` | Loop 오연결이 영구 잔류하지 않도록 강제 제거할 최대 시간이다. 0이면 강제 종료를 사용하지 않는다. |

### 5.1 Scale 적용 모드

#### ComponentTransform

```text
NiagaraComponent Transform Scale
= CombatFxData.FxScale
```

일반적인 Local Space 또는 Owner Scale 반응 Niagara에 사용한다.

#### NiagaraUserVector

```text
NiagaraComponent Transform Scale = 1,1,1
Niagara User Vector Parameter = CombatFxData.FxScale
```

이 모드는 Niagara 내부에서 지정 User Parameter를 Sprite Size, Mesh Scale, 위치 반경 또는 다른 크기 계산에 실제로 연결한 경우에만 화면 크기가 변한다.
C++이 User Parameter를 전달한다고 해서 모든 외부 Niagara의 크기가 자동으로 변경되는 것은 아니다.

### 5.2 P0 Impact Scale 해석

현재 P0 Impact 자산 `NS_BasicHit`은 외부 Component Transform Scale에 시각적으로 반응하지 않는다.
사용자는 현재 기본 크기를 승인했고, `CF-FQ-024`에서는 추가 Niagara 내부 분석이나 공용 Scale 모듈 제작을 중단했다.

현재 기준:

```text
DA_FX_ProtoShellImpact.FxScale = 1,1,1
DA_FX_ProtoShellImpact.FxScaleMode = ComponentTransform
NS_BasicHit의 내부 시각 크기를 그대로 사용
```

이 결정은 다른 모든 Niagara가 Scale을 무시한다는 의미가 아니다.
각 Niagara의 내부 속성 계산 방식에 따라 적용 가능 여부가 다르다.

---

## 6. 현재 DataAsset 연결

### 6.1 CombatFxData

```text
/Game/CarFight/FX/Data/DA_FX_ProtoWeaponFire
/Game/CarFight/FX/Data/DA_FX_ProtoShellImpact
/Game/CarFight/FX/Data/DA_FX_ProtoVehicleDead
```

AssetDump 확인:

```text
자산 수: 3
성공: 3
실패: 0
```

현재 AssetDump 프로필은 DataAsset 내부 프로퍼티 값을 노출하지 않는다.
따라서 정확한 저장 참조는 Unreal Editor에서 사용자가 저장하고 PIE로 확인한 상태를 우선한다.

### 6.2 기존 게임 데이터 연결

```text
/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon
- DefaultFireFxData = DA_FX_ProtoWeaponFire

/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell
- DefaultImpactFxData = DA_FX_ProtoShellImpact

/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan
- DefaultDestroyedFxData = DA_FX_ProtoVehicleDead
- DestroyedFxSocketName = FX_Destroyed
```

현재 기준 차량 Blueprint는 `BP_CFVehiclePawn`이며 기준 VehicleData는 `DA_TestSedan`이다.
`DA_PoliceCar`는 현재 사용하지 않는 폐기 자산이므로 CombatFx 현재 연결 대상으로 보지 않는다.

### 6.3 현재 Niagara 선택

```text
Muzzle
- DataAsset: DA_FX_ProtoWeaponFire
- 기술 기준 후보: NS_AR_Muzzleflash_1_ONCE

Impact
- DataAsset: DA_FX_ProtoShellImpact
- Niagara: /Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_BasicHit
- 외부 Scale 미반응 허용 / 현재 크기 그대로 사용

Destroyed
- DataAsset: DA_FX_ProtoVehicleDead
- Niagara: 사용자 Editor 저장 상태의 현재 비Loop 후보
- 정확한 Niagara 프로퍼티 값은 현재 AssetDump 프로필로 기계 검증하지 못함
- 발생 위치는 Niagara의 공통 위치 오프셋이 아니라 차량 StaticMesh의 FX_Destroyed 소켓이 결정
```

`NS_Explossion`은 실제 반복 Loop가 확인되어 일회성 Destroyed 후보에서 제외한다.

---

## 7. 런타임 초기화

`ACFVehiclePawn`은 기본 서브오브젝트로 `UCFCombatFxComp`를 소유한다.

```text
ACFVehiclePawn
└─ CombatFxComp
```

차량 런타임 초기화에서 다음을 호출한다.

```cpp
CombatFxComp->InitializeCombatFxRuntime(
    this,
    VehicleData,
    VehicleHealthComp);
```

초기화 시 수행하는 작업:

```text
- 이전 VehicleHealth 이벤트 구독 해제
- OwnerVehiclePawn 저장
- VehicleHealthComp 저장
- VehicleData.DefaultDestroyedFxData 저장
- VehicleData.DestroyedFxSocketName 저장
- 파괴 FX 처리 상태 초기화
- Fire / Impact / Destroyed Debug 발생 횟수 초기화
- VehicleHealthComp.OnVehicleDestroyed 이벤트 구독
- 런타임 준비 상태와 Debug 요약 갱신
```

기본 파괴 소켓 이름:

```text
FX_Destroyed
```

VehicleData의 이름이 비어 있으면 런타임에서도 `FX_Destroyed`를 사용한다.

---

## 8. 발사 FX 흐름

### 8.1 호출 조건

발사 FX는 검증 함수가 호출됐다는 이유만으로 발생하지 않는다.
HitScan 또는 Projectile 실제 실행이 성공하고 최종 발사 결과가 승인된 뒤 호출한다.

현재 호출:

```cpp
CombatFxComp->PlayFireFx(
    ActiveWeaponData->DefaultFireFxData,
    FireCommand.AimOrigin,
    FireCommand.AimDirection);
```

실행 순서:

```text
Fire 입력
→ BuildFireCommand
→ ValidateFireCommand
→ HitScan 또는 Projectile 실행 성공
→ LastFireResult.bAccepted = true
→ DefaultFireFxData로 Fire FX 1회 요청
```

### 8.2 발사 거부

다음과 같은 거부 결과에서는 발사 FX를 호출하지 않는다.

```text
NoWeapon
WeaponCooldown
AimBlocked
MuzzleBlocked
InvalidAimOrigin
InvalidAimDirection
정책상 발사 불가 상태
```

최종 사용자 PIE에서 정상 발사 1회와 발사 거부 0회를 확인했다.

### 8.3 위치와 회전

```text
위치 = 최종 FireCommand.AimOrigin
기본 회전 = FireCommand.AimDirection.Rotation()
최종 회전 = 기본 회전 * CombatFxData.RotationOffset
```

실제 AimOrigin은 현재 Weapon Aim Solution과 유효 Muzzle 소켓을 거친 최종 발사 위치다.

---

## 9. Impact FX 흐름

### 9.1 공용 입력

HitScan과 Projectile은 모두 `FCFDamageHitContext`를 Impact FX 입력으로 사용한다.

필수 조건:

```text
DamageHitContext.bBlockingHit = true
```

위치와 방향:

```text
위치 = DamageHitContext.ImpactLocation
기본 회전 = DamageHitContext.ImpactNormal.Rotation()
최종 회전 = 기본 회전 * CombatFxData.RotationOffset
```

유효하지 않은 ImpactNormal은 `FVector::UpVector`로 안전 보정한다.

### 9.2 HitScan

```text
WeaponHit Trace 첫 Blocking Hit
→ FCFDamageHitContext 생성
→ Damage 적용
→ ActiveProjectileData.DefaultImpactFxData로 Impact FX 1회
```

### 9.3 Projectile

```text
Projectile 첫 유효 Impact
→ FCFDamageHitContext 생성 및 Projectile에 결과 보존
→ Damage 적용
→ Pawn으로 Impact 결과 전달
→ ImpactProjectileData.DefaultImpactFxData로 Impact FX 1회
→ Pool 반환
```

Projectile의 기존 `bImpactResolvedThisActivation`과 단일 `ResolveProjectileImpact` 계약이 OnComponentHit과 보조 Sweep 중복 처리를 막는다.
CombatFx는 이미 확정된 첫 Impact 결과를 소비하며 별도의 충돌 판정을 다시 수행하지 않는다.

### 9.4 P0 선택

```text
Niagara = NS_BasicHit
Scale = 1,1,1
추가 Scale 튜닝 = 중단
```

최종 사용자 PIE에서 실제 충돌 위치, 1회 발생, 중복 없음과 잔류 없음을 확인했다.

---

## 10. Destroyed FX 흐름

### 10.1 호출 조건

`UCFCombatFxComp`는 `UCFVehicleHealthComp.OnVehicleDestroyed`를 구독한다.

```text
CurrentHealth > 0
→ 유효 피해 적용
→ CurrentHealth <= 0
→ UCFVehicleHealthComp 최초 Destroyed 전환
→ OnVehicleDestroyed 1회
→ CombatFxComp.PlayDestroyedFx()
```

`bDestroyedFxHandled`가 이미 true이면 후속 호출을 `DuplicateIgnored`로 처리한다.
HitDamage도 이미 파괴된 대상의 후속 피해를 `TargetDestroyed`로 거부하므로, 체력 이벤트와 CombatFx 양쪽에서 최초 1회 계약을 보호한다.

### 10.2 차량별 위치

차량마다 차체 형상과 피벗이 다르므로 공통 `PreviewLocationOffset` 또는 DataAsset 위치 오프셋으로 Destroyed 위치를 결정하지 않는다.

`UCFVehicleData.DestroyedFxSocketName`이 차량별 StaticMesh 소켓 이름을 제공한다.
기본값은 다음과 같다.

```text
FX_Destroyed
```

현재 위치 해석 순서:

```text
1. SM_Body의 DestroyedFxSocketName 소켓 Transform
2. SM_Body Bounds 중심과 SM_Body 회전
3. Owner 차량 Actor Transform
4. DamageHitContext ImpactLocation / ImpactNormal
```

소켓 사용 성공 시 Debug 출처:

```text
SM_Body.Socket:FX_Destroyed
```

소켓 누락 시 Bounds Fallback 예시:

```text
SM_Body.BoundsFallback:MissingSocket=FX_Destroyed
```

기준 Sedan 차체 StaticMesh에 `FX_Destroyed` 소켓을 배치했고, 최종 사용자 PIE에서 해당 소켓 위치에서 폭발하는 것을 확인했다.

### 10.3 소켓 제작 기준

```text
- 모든 차량에서 소켓 이름은 가능하면 FX_Destroyed로 통일한다.
- 차량별로 소켓 위치와 회전만 다르게 배치한다.
- 소켓 Scale은 FX 크기 조절 수단으로 사용하지 않는다.
- 차체 피벗이 아니라 차량 부피 또는 주요 폭발 중심에 배치한다.
- StaticMesh와 VehicleData를 저장해야 런타임에 적용된다.
```

---

## 11. Niagara 생성과 생명주기

공용 생성 함수는 `UNiagaraFunctionLibrary::SpawnSystemAtLocation`을 사용한다.

현재 주요 옵션:

```text
AutoDestroy = true
AutoActivate = false
PoolMethod = None
PreCullCheck = true
```

생성 뒤 Scale 전달 방식을 적용하고 `Activate(true)`를 호출한다.

### 11.1 null-safe 계약

다음 상황에서는 false를 반환하고 FX만 건너뛴다.

```text
CombatFx 런타임이 준비되지 않음
Dedicated Server
CombatFxData 누락
NiagaraSystem 누락
World 누락
SpawnLocation이 NaN
NiagaraComponent 생성 실패
```

이 실패는 다음 판정을 취소하거나 되돌리지 않는다.

```text
발사 승인
Projectile 생성
Blocking Hit
DamageData 적용
차량 체력 감소
파괴 상태 전환
```

### 11.2 최대 수명 안전 퓨즈

`MaximumLifetimeSeconds > 0`이면 타이머를 예약한다.

시간이 지나도 NiagaraComponent가 남아 있으면 다음을 수행한다.

```text
DeactivateImmediate()
DestroyComponent()
```

이는 Loop Niagara를 정상 일회성 자산으로 승인하는 기능이 아니다.
잘못 연결된 Loop가 월드에 영구 잔류하는 것을 막는 마지막 안전장치다.

---

## 12. 중복 방지 계약

### Fire

```text
실제 발사 실행 성공 후에만 호출
발사 거부 결과에서는 호출하지 않음
```

### Impact

```text
HitScan 첫 Blocking Hit 결과만 사용
Projectile는 기존 활성화별 첫 Impact guard 사용
OnComponentHit과 보조 Sweep이 같은 Impact를 중복 확정하지 않음
CombatFx는 확정된 DamageHitContext를 한 번 소비
```

### Destroyed

```text
VehicleHealthComp가 최초 파괴 전환 이벤트만 발생
CombatFxComp.bDestroyedFxHandled가 후속 중복 요청 차단
추가 피해는 HitDamage TargetDestroyed로 거부
```

최종 사용자 PIE에서 세 역할의 중복과 잔류가 없음을 확인했다.

---

## 13. Debug 계약

`UCFCombatFxComp`는 다음 조회 API를 제공한다.

```text
IsCombatFxRuntimeReady()
GetLastCombatFxSummary()
GetLastDestroyedFxSpawnSource()
GetFireFxSpawnCount()
GetImpactFxSpawnCount()
GetDestroyedFxSpawnCount()
```

현재 Debug가 구분하는 대표 상태:

```text
RuntimeNotReady
DedicatedServerSkipped
MissingOptionalData
InvalidWorldOrLocation
NoBlockingHit
InvalidDirection
DuplicateIgnored
SM_Body.Socket:<SocketName>
SM_Body.BoundsFallback:MissingSocket=<SocketName>
ActorTransformFallback:SM_BodyMissing
DamageHitContextFallback:OwnerMissing
```

정상 생성 요약에는 다음 값이 포함된다.

```text
FX 역할
Spawn 성공 여부
CombatFxData 이름
Niagara 이름
Scale 적용 모드
Scale 값
User Parameter 이름
MaximumLifetimeSeconds
Spawn 월드 위치
```

---

## 14. Editor Preview Actor

`ACFCombatFxPreviewActor`는 EditorOnly Actor다.
게임 전투 판정이나 패키지 런타임 Actor로 사용하지 않는다.

컴포넌트 구조:

```text
PreviewRoot
├─ ReferenceMesh
└─ PreviewOrigin
   ├─ EmissionDirection (+X)
   └─ PreviewNiagara
```

주요 기능:

```text
- CombatFxData 또는 Niagara Override 선택
- Editor Viewport 자동 반복 재생
- Uniform 또는 Vector Scale 조정
- ComponentTransform / NiagaraUserVector Scale 모드 시험
- 위치와 회전 오프셋 조정
- 최대 수명 후 Preview 자동 정지
- Call In Editor 재생·재시작·정지
- DataAsset 값 불러오기
- Preview 값을 DataAsset에 적용하고 패키지 Dirty 처리
```

주의:

```text
- DataAsset 적용 버튼은 자동 저장하지 않는다.
- PreviewLocationOffset은 실제 차량별 Destroyed 위치 해결 수단이 아니다.
- 실제 Muzzle 위치는 Muzzle 소켓과 Weapon Aim Solution이 결정한다.
- 실제 Impact 위치는 DamageHitContext.ImpactLocation이 결정한다.
- 실제 Destroyed 위치는 차량 StaticMesh의 FX_Destroyed 소켓이 결정한다.
- NiagaraUserVector는 Niagara 내부가 같은 Parameter를 소비해야만 시각 크기가 변한다.
```

---

## 15. 빌드와 검증 근거

### 15.1 공식 Admin 빌드 기록

```text
C++ Foundation
- Build Job: bd5a50bf388049ec84123aec4942ae96
- Result: PASS
- Exit Code: 0

Editor Preview Tool
- Build Job: c9682d4a0be24ead95e87a5a7e8b5849
- Result: PASS
- Exit Code: 0

Rapid Preview Tuning
- Build Job: 89b6d82710af47e89a4c5b3823037d4e
- Result: PASS
- Exit Code: 0
```

### 15.2 최신 직접 빌드

Destroyed 소켓과 Scale 전달 모드가 포함된 최신 변경은 사용자가 직접 Editor 빌드를 실행해 성공을 확인했다.
이 결과에는 Admin Build Job ID가 없으므로 사용자 직접 검증으로 구분한다.

```text
최신 사용자 직접 Editor 빌드: PASS
```

### 15.3 AssetDump

```text
/Game/CarFight/FX/Data
UCFCombatFxData: 3개
성공: 3
실패: 0
```

### 15.4 최종 사용자 PIE

```text
승인 발사 Muzzle FX 1회: PASS
발사 거부 Muzzle FX 0회: PASS
Impact 실제 충돌 위치: PASS
Impact 첫 충돌 1회: PASS
Impact 중복·잔류 없음: PASS
Destroyed FX_Destroyed 소켓 위치: PASS
Destroyed 최초 1회: PASS
Destroyed 추가 피해 중복·잔류 없음: PASS
기존 전투 회귀: PASS
```

최종 판정:

```text
CF-FQ-024 = Done
CF-TC-021 = PASS
```

---

## 16. 현재 비책임 항목

다음은 현재 CombatFx P0 완료 범위가 아니다.

```text
- 모든 게임 사운드와 오디오 옵션
- Projectile Trail·Thruster 지속형 FX와 추진 상태 동기화 — 현재 책임은 `Document/Systems/Combat/Projectile.md`가 소유
- 표면 재질별 Impact 세분화
- Impact Decal 시스템
- 지속 파괴 화염과 연기 상태
- Geometry Collection 차량 분해
- 물리 파편과 폭발 범위 피해
- 별도 Combat Presentation Subsystem
- Niagara 전용 FX Pool
- 서버 복제와 원격 클라이언트 FX 동기화
- 모든 외부 Niagara에 공통 적용되는 범용 Scale 보장
```

이 항목이 필요해지면 `CF-FQ-020` 품질 개선 또는 별도 기능 ID로 설계한다.

---

## 17. 회귀 보호 기준

CombatFx 변경 시 최소 확인:

```text
- 승인된 발사에서 Muzzle FX가 정확히 한 번 발생한다.
- Cooldown, NoWeapon, AimBlocked와 MuzzleBlocked에서 Muzzle FX가 발생하지 않는다.
- HitScan Impact가 ImpactLocation에서 한 번 발생한다.
- Projectile 첫 Impact가 한 번 발생하고 Pool 반환으로 다시 발생하지 않는다.
- Destroyed FX가 차량별 FX_Destroyed 소켓에서 최초 한 번 발생한다.
- 추가 피해에서 Destroyed FX가 다시 발생하지 않는다.
- FX가 MaximumLifetimeSeconds 이후 영구 잔류하지 않는다.
- CombatFxData 또는 Niagara를 비워도 발사·피해·파괴 판정이 유지된다.
- Weapon Aim Solution, FireFeedback, DamageHitContext와 HitDamage 결과가 바뀌지 않는다.
- 게임 오디오 자산·클래스·모듈이 추가되지 않는다.
```

연관 테스트:

```text
CF-TC-014 FireFeedback
CF-TC-015 시각 차체 피격
CF-TC-016 HitDamage
CF-TC-019 AimFireAlignment
CF-TC-020 고속 Projectile 연속 충돌
CF-TC-021 CombatFx
```

---

## 18. 연관 Systems 문서

```text
Document/Systems/Combat/WeaponFire.md
- 승인 발사와 실제 FireOrigin/AimDirection을 소유한다.

Document/Systems/Combat/FireFeedback.md
- 발사 결과의 Reticle/UI 표시를 소유하며 실제 Niagara 재생은 CombatFx가 소유한다.

Document/Systems/Combat/Projectile.md
- Projectile 추진, Trail·Thruster 지속형 FX, 첫 유효 Impact와 Pool 생명주기를 소유한다. CombatFx는 그중 확정된 첫 Impact 결과의 일회성 Impact FX 요청만 소비한다.

Document/Systems/Combat/DamageHitContext.md
- ImpactLocation, ImpactNormal과 Hit 정보를 공용 Context로 제공한다.

Document/Systems/Combat/HitDamage.md
- 체력 감소와 최초 Destroyed 상태 전환을 소유하며 파괴 시각 연출은 CombatFx가 소비한다.
```

---

## 19. 문서 갱신 조건

다음 변경이 생기면 이 문서를 함께 갱신한다.

```text
- UCFCombatFxData 필드 또는 Scale 적용 계약 변경
- UCFCombatFxComp 소유권이나 호출 함수 변경
- 발사 FX 호출 시점 변경
- HitScan 또는 Projectile Impact FX 호출 순서 변경
- DestroyedFxSocketName 또는 Fallback 순서 변경
- CombatFxData 참조 소유 위치 변경
- Niagara Pool 또는 지속형 Projectile FX가 현재 시스템 책임으로 추가
- 표면별 Impact나 지속 파괴 연출이 Current System으로 승격
- CF-TC-021 회귀 기준 변경
```

---

## 20. 문서 버전 관리

- 현재 문서 버전: `1.0.1`
- 문서 상태: `Current System / P0 User PIE Verified`

### Changelog

#### v1.0.1 - 2026-07-28

```text
- CF-FQ-028 Systems 승격에 맞춰 Projectile 지속형 Trail·Thruster와 추진 상태 동기화의 책임을 Systems/Combat/Projectile.md로 연결했다.
- CombatFx는 승인 Fire, 첫 Impact와 최초 Destroyed의 일회성 Niagara만 소유한다는 경계를 유지했다.
- Projectile의 Burning 기반 Thruster와 Pool Reset을 CombatFx 책임으로 오해하지 않도록 비책임·연관 문서를 갱신했다.
- CombatFx 런타임과 자산 동작 자체는 변경하지 않았다.
```

#### v1.0.0 - 2026-07-27

```text
- CF-FQ-024 전투 FX 현재 구현 문서를 생성했다.
- UCFCombatFxData, UCFCombatFxComp와 ACFCombatFxPreviewActor 책임을 기록했다.
- WeaponData Fire, ProjectileData Impact와 VehicleData Destroyed 참조 구조를 기록했다.
- 승인 Fire, 첫 Impact와 최초 Destroyed 1회성 호출 흐름을 기록했다.
- SM_Body.FX_Destroyed 소켓과 Bounds/Actor/DamageHitContext Fallback 순서를 기록했다.
- NS_BasicHit을 Impact P0 자산으로 현재 크기 그대로 사용하는 결정을 기록했다.
- 최대 수명 안전 퓨즈, null-safe 처리와 Debug 계약을 기록했다.
- 사용자 직접 Editor 빌드 PASS와 최종 사용자 PIE 전체 PASS를 기록했다.
- CF-FQ-024 Done과 CF-TC-021 PASS를 현재 시스템 기준으로 승격했다.
```

### Migration

#### v1.0.1 적용 안내

```text
- 지속형 Projectile Trail·Thruster와 추진 상태는 Systems/Combat/Projectile.md를 우선한다.
- CombatFx는 기존 Muzzle·Impact·Destroyed 일회성 Current System만 유지한다.
- CF-FQ-028 승격은 CombatFx의 재구현이나 CF-TC-021 재검증을 의미하지 않는다.
```

#### Initial Current System 적용 안내

```text
- CF-FQ-024의 현재 구현 판단은 이 문서를 우선한다.
- Document/Plan/CombatFxAudio/ImplementationDesign.md는 완료 당시 설계와 검증 체크포인트 보존용으로 유지한다.
- Impact P0는 NS_BasicHit을 현재 크기 그대로 사용하며 Scale 미반응 분석을 다시 열지 않는다.
- 차량별 Destroyed 위치는 SM_Body StaticMesh의 FX_Destroyed 소켓으로 지정한다.
- CombatFxData 또는 Niagara 누락은 전투 판정 실패가 아니라 선택적 시각 연출 없음으로 해석한다.
- 게임 사운드는 후속 누락 기능이 아니라 프로젝트 전역 비지원 범위다.
```

---

## 21. 마지막 확인 기준

- 확인 일시: `2026-07-27`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFCombatFxData.h`
  - `UE/Source/CarFight_Re/Private/CFCombatFxData.cpp`
  - `UE/Source/CarFight_Re/Public/CFCombatFxComp.h`
  - `UE/Source/CarFight_Re/Private/CFCombatFxComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFCombatFxPreviewActor.h`
  - `UE/Source/CarFight_Re/Private/CFCombatFxPreviewActor.cpp`
  - `UE/Source/CarFight_Re/Public/CFWeaponData.h`
  - `UE/Source/CarFight_Re/Public/CFProjectileData.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `/Game/CarFight/FX/Data` AssetDump 3/3 성공
  - 사용자 직접 Editor 빌드 성공 보고
  - 최종 사용자 PIE 전체 PASS 보고
