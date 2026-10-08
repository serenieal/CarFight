# CF-FQ-025 Phase 1 — Weapon Preview World 검토·보정 TaskSource

- 문서 버전: v1.0
- 작성일: 2026-07-16
- 문서 상태: Active / Codex Review Input
- 작업 ID: `CF-FQ-025-P1`
- 실행 성격: 기존 Browser Direct Edit 정책 위반분의 Codex 검토·보정
- 대표 Plan: `Document/Plan/ReticleAimDirection/ImplementationDesign.md`

## Goal

- 기존 Browser Direct Edit 정책 위반분을 Codex가 검토·보정한다.
- 실제 최종 `AimDirection`을 기준으로 직선 사격용 Weapon Preview 월드 데이터와 VehicleDebug 표시를 완성한다.
- 기존 Aim, Fire, Reticle 계약과 unrelated dirty 변경을 보존한다.

## In Scope

- `FCFVehicleWeaponAimSolution`의 기존 Preview 필드 검토·보정
- `BuildWeaponAimSolution()`의 DirectImpact 지원 조건과 Preview 결과 계산
- MuzzleBlocked와 Preview가 공유하는 단일 `WeaponHit` Trace
- 활성 무기 MaxRange와 AimProfile fallback 거리 처리
- Preview 네 필드의 `OutWeaponAimSolution` 할당
- VehicleDebug의 Preview 유효 여부, Hit 여부, 월드 위치와 거리 표시
- 세 허용 파일의 Version, Date, Changelog 정합성 보정
- scoped diff, `Tools\\BuildEditor.bat`와 정적 계약 검증

## Out of Scope

- World To Screen 투영
- `CFAimReticleWidget` 변경
- `WBP_AimReticle` 변경
- Weapon Reticle 이미지 표시와 시각 디자인
- Ballistic Solver와 중력 Projectile 예상 탄착점
- Lead Indicator, 화면 경계 Clamp, 다중 터렛, 네트워크 보정
- 기존 WeaponFire 판정 정책 변경
- commit, push 또는 작업 트리 정리

## Constraints

- 정책 위반 이력을 숨기거나 기존 직접 수정이 Codex 실행 결과였다고 기록하지 않는다.
- 허용된 세 코드 파일만 수정한다.
- AimTargetLocation, AimDirection, FireRequest.AimDirection, 정렬 중 발사 정책과 MuzzleBlocked 계약을 유지한다.
- Preview와 MuzzleBlocked는 동일한 첫 Blocking Hit 결과를 공유한다.
- 중력 Projectile은 DirectImpact Preview를 제공하지 않는다.
- unrelated dirty 변경을 되돌리거나 재포맷하지 않는다.
- 신규 소스 파일과 UE 에셋을 만들거나 수정하지 않는다.
- 사용자 PIE는 PASS로 선언하지 않는다.

## Target Files

- `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`

## Verification

- 작업 전후 세 허용 파일의 scoped Git diff 확인
- 허용 파일 밖 신규 변경 없음 확인
- `Tools\\BuildEditor.bat` PASS
- HitScan과 중력 없는 Projectile Preview 유효 경로 확인
- 중력 Projectile, 무기 없음과 비호환 Preview 무효 경로 확인
- MuzzleBlocked의 10.0 허용값과 Command 거리 비교 유지 확인
- Preview 네 필드 할당과 VehicleDebug 네 행 확인
- Widget과 WBP 미변경 확인
- PIE 상태는 `User Validation Pending`

---

## 1. 작업 목적

`FCFVehicleWeaponAimSolution.AimDirection`이 나타내는 실제 최종 사격 방향을 기준으로, 직선 사격용 Weapon Preview 월드 위치를 계산하고 VehicleDebug에서 확인할 수 있게 한다.

이번 작업은 새 기능을 깨끗한 상태에서 처음 구현하는 작업이 아니다. 웹브라우저 AI가 CodeWorkGate를 위반해 일부 코드를 직접 수정한 뒤 수행하는 검토·보정 작업이다. 기존 직접 수정이 Codex 실행 결과였던 것처럼 기록하거나 이력을 숨기지 않는다.

---

## 2. 현재 저장소 사실

### 2.1 직접 수정된 부분

`UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`

- 다음 필드가 이미 직접 추가돼 있다.
  - `bHasValidWeaponPreview`
  - `bWeaponPreviewHasBlockingHit`
  - `WeaponPreviewWorldLocation`
  - `WeaponPreviewDistance`
- Codex는 필드 이름, 타입, 기본값, `UPROPERTY` 의미와 주석을 검토한다.
- 설계와 일치하면 유지하고, 실제 계약과 맞지 않는 부분만 보정한다.
- 단순히 정책 위반분이라는 이유만으로 필드를 삭제하거나 파일 전체를 되돌리지 않는다.

`UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

- 파일 헤더는 v2.112.0이며 Weapon Preview와 MuzzleBlocked가 공유 Trace를 사용하는 구현이 완료된 것처럼 기록돼 있다.
- 실제 `BuildWeaponAimSolution()`에는 Preview 계산과 네 필드 할당이 없다.
- Codex는 Phase 1 구현을 완성한 뒤 버전, Date와 Changelog가 실제 최종 코드와 일치하도록 정리한다.
- 구현되지 않은 내용을 완료된 것처럼 남겨서는 안 된다.

### 2.2 기존 검증 증거

- Phase 0 기준선 Editor 빌드 성공 기록이 있다.
- Preview 필드만 추가된 부분 상태에서도 Editor 빌드가 성공했다.
- 위 결과는 컴파일 가능성만 증명하며 Phase 1 기능 완료나 PIE PASS를 의미하지 않는다.
- CF-FQ-025 사용자 PIE는 아직 수행하지 않았다.

### 2.3 기존 생성 산출물

- 기존 계약: `Document/Plan/ReticleAimDirection/Generated/Final/Phase1_WeaponPreviewWorld_CodexTask.md`
- 기존 계약은 TaskSource와 연결되지 않았고 Codex가 실행하지 않았다.
- 자동 생성된 `Generated/Intermediate/FolderRequest_TaskSource.md`와 `Generated/Final/P1-T1_Contract.yaml`에는 GoPyMCP 테스트 파일이 혼입돼 있으므로 이번 CarFight 코드 작업 입력으로 사용하지 않는다.

---

## 3. 변경 허용 파일

Codex는 다음 세 파일만 수정할 수 있다.

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

---

## 4. 검토 전용 파일

다음 파일은 현재 계약과 타입을 확인하기 위한 읽기 전용 대상이다. 이번 Task에서 수정하지 않는다.

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Public/CFProjectileData.h
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Public/CFCollisionChannels.h
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
```

다음 UE 에셋도 수정하지 않는다.

```text
/Game/CarFight/UI/WBP_AimReticle
```

---

## 5. 구현 요구사항

### 5.1 DirectImpact Preview 지원 조건

Preview는 다음 조건에서만 유효하다.

```text
활성 WeaponData 존재
AND 활성 WeaponData 호환
AND (
    FireMode == HitScan
    OR (
        FireMode == Projectile
        AND 활성 ProjectileData 존재
        AND ProjectileData.bAffectedByGravity == false
    )
)
```

다음 상태에서는 Preview를 명시적으로 무효화한다.

```text
bHasValidWeaponPreview = false
bWeaponPreviewHasBlockingHit = false
WeaponPreviewWorldLocation = FVector::ZeroVector
WeaponPreviewDistance = 0.0f
```

무효 조건:

- 활성 무기 없음
- 활성 WeaponData 비호환
- Projectile 모드인데 ProjectileData 없음
- 중력 Projectile
- Preview 거리 또는 결과 위치가 유효하지 않음

### 5.2 Preview 기준

```text
PreviewStart = WeaponAimSolution.AimOrigin
PreviewDirection = WeaponAimSolution.AimDirection
```

- `AimDirection`은 정책과 정렬 상태를 반영한 실제 최종 발사 방향이다.
- `CurrentMuzzleDirection`이나 `DesiredAimDirection`을 Preview 전용으로 다시 선택하지 않는다.

### 5.3 Preview 거리

- 활성 무기 사거리는 `VehicleWeaponComp`의 기존 `GetActiveWeaponMaxRange()` 경로를 사용한다.
- 안전 fallback은 `VehicleAimComp->GetDefaultAimProfile().MaxAimDistance`를 사용한다.
- NaN, 무한대 또는 음수는 안전하게 무효 또는 0으로 처리한다.
- 유효한 DirectImpact Preview의 거리는 0보다 커야 한다.

### 5.4 MuzzleBlocked와 공유 Trace

`BuildWeaponAimSolution()` 안에서 기존 MuzzleBlocked 검사와 Weapon Preview를 서로 다른 두 Trace로 계산하지 않는다.

공유 Trace 기준:

```text
CommandPathDistance = AimOrigin에서 AimTargetLocation까지의 기존 거리
PreviewMaxRange = 유효한 DirectImpact 무기의 최대 사거리
SharedTraceDistance = Max(CommandPathDistance, PreviewMaxRange)
TraceStart = AimOrigin
TraceDirection = AimDirection
TraceChannel = CFCollisionChannels::WeaponHit
```

- 기존처럼 현재 Pawn을 Ignore한다.
- 새로운 Ignore Actor 목록을 임의로 추가하지 않는다.
- 한 번 얻은 첫 Blocking Hit 결과를 MuzzleBlocked와 Preview가 함께 해석한다.

### 5.5 MuzzleBlocked 계약 보존

기존 의미와 허용값을 바꾸지 않는다.

```text
TargetSurfaceTolerance = 10.0f
bMuzzleBlocked = 첫 Blocking Hit 존재
    AND HitDistance + TargetSurfaceTolerance < CommandPathDistance
```

- Preview MaxRange 때문에 Trace가 길어져도 MuzzleBlocked 판정은 Command 목표 거리와 비교한다.
- MuzzleBlocked는 정렬 중 발사 정책과 관계없이 기존 발사 거부 계약을 유지한다.

### 5.6 Preview 결과

첫 Blocking Hit이 Preview MaxRange 안에 있으면:

```text
bHasValidWeaponPreview = true
bWeaponPreviewHasBlockingHit = true
WeaponPreviewWorldLocation = HitResult.ImpactPoint
WeaponPreviewDistance = Max(0, HitResult.Distance)
```

첫 Blocking Hit이 없거나 Preview MaxRange 밖이면:

```text
bHasValidWeaponPreview = true
bWeaponPreviewHasBlockingHit = false
WeaponPreviewWorldLocation = AimOrigin + AimDirection * PreviewMaxRange
WeaponPreviewDistance = PreviewMaxRange
```

- 결과 위치에 NaN이 있으면 Preview 전체를 무효 상태로 되돌린다.
- 계산한 네 값을 `OutWeaponAimSolution`에 명시적으로 할당한다.

### 5.7 VehicleDebug 표시

`CFVehicleDebugPanelWidget.cpp`의 기존 `Weapon Aim Solution` 하위 섹션에 다음 네 행을 추가한다.

```text
Weapon Preview 유효 여부
Weapon Preview Blocking Hit 여부
Weapon Preview 월드 위치
Weapon Preview 거리
```

- 기존 `FCFVehicleWeaponAimSolution` 캐시 값을 읽기만 한다.
- Debug Panel에서 Trace나 방향을 다시 계산하지 않는다.
- 기존 FieldId와 섹션 구조를 깨지 않는다.

### 5.8 파일 메타데이터

- 수정 파일의 Version, Date, Changelog와 필요한 Migration을 실제 최종 구현과 일치시킨다.
- 함수, 변수 또는 의미 있는 구현 블록에는 CarFight 규칙에 맞는 한 줄 설명 주석을 유지한다.
- 파일 전체 재포맷이나 관련 없는 주석 정리는 하지 않는다.

---

## 6. 보호 범위

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

이번 Phase에서 구현하지 않는다.

```text
World To Screen 투영
CFAimReticleWidget 변경
WBP_AimReticle 변경
Weapon Reticle 이미지 표시
Ballistic Solver
중력 Projectile 예상 탄착점
Lead Indicator
화면 가장자리 Clamp
다중 터렛 Reticle
네트워크 보정
```

---

## 7. 저장소 보호

- 기존 unrelated dirty 변경을 정리, 되돌림, 재포맷하거나 덮어쓰지 않는다.
- 세 허용 파일 안에서도 이번 Phase와 관계없는 기존 변경을 제거하지 않는다.
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

## 8. 검증

### 8.1 변경 전후 검토

- 작업 전 세 허용 파일의 현재 diff를 확인한다.
- 작업 후 동일 파일의 scoped diff를 확인한다.
- 허용 파일 밖의 새로운 변경이 생기지 않았는지 확인한다.

### 8.2 빌드

```text
Tools\BuildEditor.bat
```

- Editor 빌드가 성공해야 한다.
- 빌드 실패 시 오류를 숨기지 않고 실패 원인과 관련 파일을 보고한다.

### 8.3 정적 계약 확인

- HitScan Preview 유효 경로 존재
- 중력 없는 Projectile Preview 유효 경로 존재
- 중력 Projectile Preview 무효 경로 존재
- 활성 무기 없음/비호환 Preview 무효 경로 존재
- MuzzleBlocked 기존 10.0 허용값과 Command 거리 비교 유지
- 공유 WeaponHit Trace 사용
- 네 Preview 필드 할당 존재
- VehicleDebug 네 행 존재
- Widget/WBP 미변경

### 8.4 PIE

- Codex는 PIE를 PASS로 선언하지 않는다.
- 결과는 `User Validation Pending`으로 보고한다.

---

## 9. 성공 조건

- 세 허용 파일 외 소스, 설정과 에셋을 수정하지 않는다.
- Phase 1 World Preview 데이터와 Debug 표시만 완성한다.
- 기존 직접 수정분을 검토하고 실제 최종 구현과 일치하도록 보정한다.
- `Tools\BuildEditor.bat`가 성공한다.
- 기존 Aim/Fire/Reticle 계약을 보존한다.
- 정책 위반 이력을 숨기지 않는다.

---

## 10. 실패 조건

다음 중 하나라도 발생하면 성공이 아니다.

- 허용 파일 밖 코드·설정·에셋 변경
- Phase 2 Widget 또는 WBP 작업 포함
- 중력 Projectile을 DirectImpact 예상 탄착점으로 표시
- MuzzleBlocked 의미 또는 정렬 중 발사 정책 변경
- Preview와 MuzzleBlocked에 서로 다른 Trace 정책 사용
- 관련 없는 dirty 변경 정리 또는 되돌림
- 빌드 실패
- PIE를 근거 없이 PASS 처리
- 직접 수정분을 Codex가 원래 구현한 것처럼 보고

---

## 11. Codex 결과 보고 형식

1. 실제 변경 파일
2. 기존 직접 수정분 중 유지한 내용
3. 기존 직접 수정분 중 보정한 내용
4. 구현한 DirectImpact Preview 계산 계약
5. MuzzleBlocked 및 기존 Aim/Fire 보호 범위 준수 여부
6. 실행한 빌드 명령과 결과
7. 정적 검증 결과
8. 사용자 PIE Pending 항목
9. 수행하지 않은 Git 작업 확인

---

## 12. Changelog

### v1.0 - 2026-07-16

- CF-FQ-025 Phase 1 World Preview 작업 범위를 세 코드 파일로 제한.
- Browser Direct Edit 정책 위반분을 Codex가 검토·보정하는 실행 성격을 명시.
- DirectImpact 지원 조건, 공유 WeaponHit Trace, MuzzleBlocked 보호 계약과 Preview 결과 규칙 확정.
- VehicleDebug 표시, 빌드·정적 검증, PIE Pending과 Git 금지사항 정의.
