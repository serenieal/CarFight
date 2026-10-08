# CF-FQ-025 — 중력 Projectile LaunchDirection 확장 TaskSource

- 문서 버전: v1.0
- 작성일: 2026-07-16
- 문서 상태: Active / Codex Input Source
- 작업 ID: `CF-FQ-025-LD`
- 대표 Plan: `Document/Plan/ReticleAimDirection/ImplementationDesign.md`
- 선행 작업: `CF-FQ-025-P1 DirectImpact Weapon Preview`

## Goal

- 현재 실사용 Heavy Cannon처럼 중력이 적용되는 Projectile에서도 실제 초기 발사 방향을 Weapon Reticle 데이터로 제공한다.
- 직선 사격의 `DirectImpact`와 중력 Projectile의 `LaunchDirection` 의미를 명시적으로 분리한다.
- 예상 탄착 위치는 계산하거나 표시하지 않고 후속 `BallisticImpact` 단계로 남긴다.
- 기존 Aim, Fire, MuzzleBlocked, Reticle과 unrelated dirty 변경을 보존한다.

## In Scope

- `ECFWeaponReticleMode` 신규 enum 추가
- `FCFVehicleWeaponAimSolution`에 현재 Reticle Mode 필드 추가
- 기존 Preview 필드의 의미를 DirectImpact와 LaunchDirection 공용 표시 데이터로 확장
- `BuildWeaponAimSolution()`의 모드 판정과 LaunchDirection 월드 위치 계산
- 기존 DirectImpact Preview와 공유 WeaponHit Trace 회귀 보호
- VehicleDebug에 Weapon Reticle Mode 표시 추가
- FireOrigin 요약 문자열에 Mode 표시 추가
- 허용 파일의 Version, Date, Changelog 정합성 보정
- scoped diff, `Tools\\BuildEditor.bat`와 정적 계약 검증

## Out of Scope

- 중력 Projectile 예상 탄착 위치 계산
- Ballistic Solver, 탄도 궤적 Trace와 BallisticImpact Reticle
- World To Screen 투영
- `CFAimReticleWidget` 변경
- `WBP_AimReticle` 변경
- Weapon Reticle 이미지, 색상, Opacity와 애니메이션
- Lead Indicator와 이동 목표 예측
- 화면 가장자리 Clamp
- 실제 Projectile 중력, 속도 또는 DataAsset 값 변경
- 기존 WeaponFire 판정 정책 변경
- commit, push 또는 작업 트리 정리

## Constraints

- `DA_HeavyShell.bAffectedByGravity`를 변경하지 않는다.
- `LaunchDirection`은 예상 탄착점이 아니라 `AimOrigin`에서 `AimDirection`으로 출발하는 초기 발사 방향이다.
- `DirectImpact`만 Blocking Hit 또는 최대 사거리 끝점을 실제 예상 충돌 위치로 해석한다.
- `LaunchDirection`에서는 `bWeaponPreviewHasBlockingHit=false`를 유지한다.
- `MuzzleBlocked`는 기존 공유 WeaponHit Trace와 Command 목표 거리 비교 계약을 유지한다.
- AimTargetLocation, AimDirection, FireRequest.AimDirection과 정렬 중 발사 정책을 변경하지 않는다.
- 허용된 세 코드 파일만 수정한다.
- unrelated dirty 변경을 되돌리거나 재포맷하지 않는다.
- 사용자 PIE를 PASS로 선언하지 않는다.

## Target Files

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`

## Verification

- 작업 전후 세 허용 파일의 scoped Git diff 확인
- 허용 파일 밖 신규 변경 없음 확인
- `Tools\\BuildEditor.bat` PASS
- HitScan과 중력 없는 Projectile은 `DirectImpact`
- 중력 Projectile은 `LaunchDirection`
- 무기 없음, 비호환과 ProjectileData 없음은 `Hidden`
- LaunchDirection의 Preview 위치가 `AimOrigin + AimDirection * PreviewDistance`
- LaunchDirection에서 Blocking Hit가 항상 false
- DirectImpact 공유 Trace와 MuzzleBlocked 10.0 허용값 유지
- VehicleDebug에 Mode와 기존 Preview 네 값 표시
- Widget, WBP와 DataAsset 미변경 확인
- PIE 상태는 `User Validation Pending`

---

## 1. 확정 설계

사용자는 2026-07-16 다음 구조를 확정했다.

```text
중력 Projectile
→ Weapon Reticle = LaunchDirection
→ 의미: 현재 실제 초기 발사 방향

예상 탄착점
→ BallisticImpact
→ 후속 Ballistic Solver에서 별도 구현
```

현재 실사용 연결:

```text
HeavyCannon
→ DA_ProtoTurretCannon
   FireMode = Projectile
→ DA_HeavyShell
   bAffectedByGravity = true
```

현재 DirectImpact 전용 구현에서는 위 무기가 Preview 무효가 되므로, Phase 2 UI 전에 LaunchDirection 데이터를 추가해야 한다.

---

## 2. 변경 허용 파일

Codex는 다음 세 파일만 수정할 수 있다.

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

다음 파일과 에셋은 검토 전용이며 수정하지 않는다.

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Public/CFCollisionChannels.h
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
/Game/CarFight/UI/WBP_AimReticle
/Game/CarFight/Weapons/Data/ProjectileDefs/DA_HeavyShell
/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon
```

---

## 3. 타입 계약

### 3.1 ECFWeaponReticleMode

`CFVehicleAimTypes.h`에 Blueprint 사용 가능한 enum을 추가한다.

```cpp
enum class ECFWeaponReticleMode : uint8
{
    Hidden,
    DirectImpact,
    LaunchDirection
};
```

의미:

```text
Hidden
= 표시 가능한 Weapon Reticle 월드 데이터가 없음

DirectImpact
= HitScan 또는 중력 없는 Projectile의 첫 Blocking Hit 또는 최대 사거리 끝점

LaunchDirection
= 중력 Projectile의 실제 초기 발사 방향을 나타내는 방향 표식용 월드 점
```

`BallisticImpact`는 이번 enum에 미리 추가하지 않는다. 실제 Ballistic Solver가 구현되는 후속 단계에서 추가한다.

### 3.2 FCFVehicleWeaponAimSolution 확장

다음 필드를 추가한다.

```cpp
ECFWeaponReticleMode WeaponReticleMode = ECFWeaponReticleMode::Hidden;
```

기존 필드는 유지하며 의미를 다음처럼 정리한다.

```text
bHasValidWeaponPreview
= WeaponReticleMode가 Hidden이 아니고 표시 가능한 월드 위치와 거리가 유효함

bWeaponPreviewHasBlockingHit
= DirectImpact 모드에서만 첫 Blocking Hit을 얻었는지 여부
= LaunchDirection에서는 항상 false

WeaponPreviewWorldLocation
= DirectImpact: 첫 Hit 또는 최대 사거리 끝점
= LaunchDirection: 초기 발사 방향을 투영하기 위한 방향 표식 월드 점

WeaponPreviewDistance
= AimOrigin에서 WeaponPreviewWorldLocation까지 거리
```

기존 Blueprint 필드 이름은 변경하지 않는다.

---

## 4. 모드 판정

활성 WeaponData가 존재하고 현재 MountProfile과 호환되는 경우에만 모드를 판정한다.

```text
FireMode == HitScan
→ DirectImpact

FireMode == Projectile
AND ActiveProjectileData 존재
AND bAffectedByGravity == false
→ DirectImpact

FireMode == Projectile
AND ActiveProjectileData 존재
AND bAffectedByGravity == true
→ LaunchDirection

그 외
→ Hidden
```

Hidden 조건:

- 활성 WeaponData 없음
- 활성 WeaponData 비호환
- Projectile 모드인데 ActiveProjectileData 없음
- Preview 거리 무효
- 결과 월드 위치 또는 거리가 NaN/비정상

---

## 5. Preview 거리

모드가 Hidden이 아니면 기존 경로를 사용한다.

```text
PreviewDistance = VehicleWeaponComp->GetActiveWeaponMaxRange(FallbackPreviewDistance)
FallbackPreviewDistance = VehicleAimComp->GetDefaultAimProfile().MaxAimDistance
```

- NaN, 무한대, 0 이하 값은 무효 처리한다.
- 별도 LaunchDirection 전용 상수를 만들지 않는다.
- 표시 거리 때문에 실제 발사, Projectile 수명 또는 충돌 판정을 변경하지 않는다.

---

## 6. DirectImpact 계산

기존 구현을 회귀 보호한다.

```text
PreviewStart = AimOrigin
PreviewDirection = AimDirection
SharedTraceDistance = Max(CommandPathDistance, PreviewMaxRange)
```

- MuzzleBlocked와 DirectImpact는 동일한 첫 `WeaponHit` Trace 결과를 공유한다.
- Hit가 PreviewMaxRange 안이면 ImpactPoint와 Hit Distance를 사용한다.
- Hit가 없거나 범위 밖이면 최대 사거리 끝점을 사용한다.
- `bWeaponPreviewHasBlockingHit` 의미를 유지한다.

---

## 7. LaunchDirection 계산

LaunchDirection은 충돌 예상이 아니라 방향 표식이다.

```text
WeaponReticleMode = LaunchDirection
bHasValidWeaponPreview = true
bWeaponPreviewHasBlockingHit = false
WeaponPreviewWorldLocation = AimOrigin + AimDirection * PreviewMaxRange
WeaponPreviewDistance = PreviewMaxRange
```

중요 규칙:

- LaunchDirection 결과를 SharedWeaponHitResult의 ImpactPoint로 대체하지 않는다.
- 가까운 장애물이 있어도 LaunchDirection 월드 위치는 초기 방향 표식의 끝점을 유지한다.
- 실제 발사 차단 여부는 기존 `bMuzzleBlocked`로 별도 표시한다.
- 총구가 막힌 경우에도 Reticle Mode와 방향 데이터는 유지할 수 있으나 발사 가능 판정은 기존대로 거부된다.
- 결과 위치에 NaN이 있거나 거리가 유한하지 않으면 전체 Preview를 Hidden/무효 상태로 되돌린다.

---

## 8. MuzzleBlocked 회귀 보호

기존 의미와 허용값을 변경하지 않는다.

```text
TargetSurfaceTolerance = 10.0f
bMuzzleBlocked = 첫 Blocking Hit 존재
    AND HitDistance + TargetSurfaceTolerance < CommandPathDistance
```

Trace 거리:

```text
DirectImpact
→ Max(CommandPathDistance, PreviewMaxRange)

LaunchDirection 또는 Hidden
→ CommandPathDistance
```

이유:

- DirectImpact는 최대 사거리 안의 첫 충돌을 예측해야 한다.
- LaunchDirection은 충돌 예측이 아니므로 PreviewMaxRange까지 Trace를 확장할 필요가 없다.
- MuzzleBlocked는 계속 Command 목표 거리와 비교한다.

---

## 9. 결과 할당

`OutWeaponAimSolution`에 다음 값을 명시적으로 할당한다.

```text
WeaponReticleMode
bHasValidWeaponPreview
bWeaponPreviewHasBlockingHit
WeaponPreviewWorldLocation
WeaponPreviewDistance
```

무효 상태:

```text
WeaponReticleMode = Hidden
bHasValidWeaponPreview = false
bWeaponPreviewHasBlockingHit = false
WeaponPreviewWorldLocation = FVector::ZeroVector
WeaponPreviewDistance = 0.0f
```

기존 Aim Solution 필드 할당 순서와 의미는 유지한다.

---

## 10. VehicleDebug와 요약 문자열

`Weapon Aim Solution` 하위 섹션에 다음 행을 추가한다.

```text
Weapon Reticle Mode
```

표시값:

```text
Hidden
DirectImpact
LaunchDirection
```

기존 네 행은 유지한다.

```text
Weapon Preview 유효 여부
Weapon Preview Blocking Hit 여부
Weapon Preview 월드 위치
Weapon Preview 거리
```

FireOrigin 요약 문자열에도 다음 정보를 추가한다.

```text
WeaponReticleMode=<Mode>
```

Debug Panel과 요약 문자열은 계산하지 않고 Aim Solution 값을 읽기만 한다.

---

## 11. 보호 범위

다음 계약은 변경하지 않는다.

```text
AimTargetLocation = Command Reticle 목표점
AimDirection = 실제 최종 발사 방향
HitScan과 Projectile = 같은 FireRequest.AimDirection 사용
bAllowFireWhileAligning=true = 정렬 중 CurrentMuzzleDirection 발사
bAllowFireWhileAligning=false = 정렬 완료 전 발사 거부
MuzzleBlocked = 정책과 관계없이 발사 거부
OutOfArc = 현재 단독 발사 차단 조건 아님
AimReticle/WBP = 판정과 Trace를 계산하지 않음
기존 FireFeedback 상태와 WBP 바인딩 유지
```

이번 작업에서 구현하지 않는다.

```text
World To Screen 투영
CFAimReticleWidget 변경
WBP_AimReticle 변경
Ballistic Solver
BallisticImpact
Lead Indicator
화면 가장자리 Clamp
다중 터렛 Reticle
네트워크 보정
```

---

## 12. 저장소 보호

- 기존 unrelated dirty 변경을 정리, 되돌림, 재포맷하거나 덮어쓰지 않는다.
- 세 허용 파일 안에서도 이번 작업과 관계없는 기존 변경을 제거하지 않는다.
- 신규 소스 파일을 만들지 않는다.
- 다음 Git 작업을 수행하지 않는다.

```text
commit
push
reset
checkout
restore
stash
clean
rebase
```

---

## 13. 검증

### 13.1 정적 검증

- `ECFWeaponReticleMode`에 Hidden, DirectImpact, LaunchDirection 존재
- Aim Solution에 `WeaponReticleMode` 존재
- HitScan → DirectImpact
- 무중력 Projectile → DirectImpact
- 중력 Projectile → LaunchDirection
- ProjectileData 없음 → Hidden
- LaunchDirection은 Preview Hit를 false로 유지
- LaunchDirection 월드 위치가 AimOrigin + AimDirection × PreviewMaxRange
- DirectImpact 기존 첫 Hit/최대 거리 계약 유지
- MuzzleBlocked 10.0 허용값과 CommandPathDistance 비교 유지
- LaunchDirection에서 Shared Trace가 PreviewMaxRange까지 불필요하게 확장되지 않음
- VehicleDebug Mode 행과 기존 네 Preview 행 존재
- Widget, WBP와 DataAsset 미변경

### 13.2 빌드

```text
Tools\BuildEditor.bat
```

Editor 빌드가 성공해야 한다.

### 13.3 사용자 PIE Pending

Codex는 PIE를 PASS로 선언하지 않는다.

사용자 PIE 항목:

```text
- Heavy Cannon에서 Mode = LaunchDirection
- Preview Valid = Yes
- Preview Blocking Hit = No
- 조준 이동 시 Preview World Location과 Distance가 유효하게 갱신
- MuzzleBlocked 발사 차단 회귀
- 중력탄 실제 초기 발사 방향과 후속 화면 Reticle 위치 일치
```

---

## 14. 성공 조건

- 세 허용 파일 외 소스, 설정과 에셋을 수정하지 않는다.
- Heavy Cannon에서 사용할 LaunchDirection 데이터 계약을 제공한다.
- DirectImpact와 LaunchDirection의 의미가 enum과 Debug에서 구분된다.
- 중력 Projectile 탄착점을 예측한 것처럼 표시하지 않는다.
- 기존 Aim/Fire/MuzzleBlocked 계약을 보존한다.
- `Tools\\BuildEditor.bat`가 성공한다.

---

## 15. 실패 조건

다음 중 하나라도 발생하면 성공이 아니다.

- 중력 Projectile을 DirectImpact로 분류
- LaunchDirection에 Blocking Hit 위치를 사용
- DA_HeavyShell 또는 다른 DataAsset 중력 설정 변경
- BallisticImpact를 계산하지 않고 예상 탄착점으로 표기
- MuzzleBlocked 의미 또는 정렬 중 발사 정책 변경
- 허용 파일 밖 코드·설정·에셋 변경
- Widget 또는 WBP 작업 포함
- 관련 없는 dirty 변경 정리 또는 되돌림
- 빌드 실패
- PIE를 근거 없이 PASS 처리

---

## 16. Codex 결과 보고 형식

1. 실제 변경 파일
2. 추가한 `ECFWeaponReticleMode`와 Aim Solution 필드
3. DirectImpact와 LaunchDirection 판정 및 계산 계약
4. MuzzleBlocked와 기존 Aim/Fire 보호 범위 준수 여부
5. VehicleDebug와 요약 문자열 변경
6. 실행한 빌드 명령과 결과
7. 정적 검증 결과
8. 사용자 PIE Pending 항목
9. 수행하지 않은 Git 작업 확인

---

## 17. Changelog

### v1.0 - 2026-07-16

- 중력 Projectile의 Weapon Reticle 의미를 LaunchDirection으로 확정.
- DirectImpact, LaunchDirection과 Hidden 모드 계약 정의.
- LaunchDirection을 실제 초기 발사 방향으로 제한하고 BallisticImpact와 분리.
- Heavy Cannon과 DA_HeavyShell의 중력 설정을 유지하는 보호 규칙 추가.
- MuzzleBlocked Trace 거리, Debug 표시, 빌드·정적 검증과 PIE Pending 정의.
