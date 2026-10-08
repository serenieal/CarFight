# Codex Task Source — Align Fire Policy

- Task ID: `CF-FQ-022-ALIGN-FIRE-POLICY`
- Version: `1.0.0`
- Date: `2026-07-13`
- Status: `Ready for Codex`
- Detailed Source: `TaskSource_AlignFirePolicy.md`

## Goal

`UCFTurretMountData`에 터렛별 정렬 중 발사 허용 정책을 추가한다.

`bAllowFireWhileAligning=true`이면 터렛/총구 정렬 중에도 발사를 허용하되 실제 탄환은 현재 Muzzle Socket X축 방향으로 발사한다. 정렬 완료 후에는 Muzzle에서 Reticle 목표점으로 향하는 요구 방향을 사용한다.

`bAllowFireWhileAligning=false`이면 현재처럼 `bTurretAligning` 또는 `bWeaponNotAligned` 상태에서 발사를 거부한다.

## Repository First

코드 변경 전에 현재 Repository와 SSOT를 다시 읽는다. 이전 대화와 추정보다 현재 코드와 문서를 우선한다.

## Target Files

```text
UE/Source/CarFight_Re/Public/CFTurretMountData.h
UE/Source/CarFight_Re/Private/CFTurretMountData.cpp
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

위 5개 코드 파일만 수정한다. 추가 C++ 파일이 반드시 필요하면 구현하지 말고 missing contract로 보고한다.

## In Scope

1. `UCFTurretMountData`에 `bAllowFireWhileAligning`을 추가한다.
2. 기본값은 `true`다.
3. 한국어 `DisplayName`과 다음 의미가 분명한 `ToolTip`을 작성한다.

```text
True: 터렛 회전 또는 총구 정렬 중에도 현재 Muzzle 방향으로 발사
False: 터렛과 총구 정렬 완료 전 발사 거부
MuzzleBlocked: 옵션과 관계없이 항상 발사 거부
```

4. `BuildTurretMountSummary()`에서 정책값을 확인할 수 있게 한다.
5. `FCFVehicleWeaponAimSolution`이 아래 의미를 구분하도록 최소 필드를 추가한다.

```text
bAllowFireWhileAligning
DesiredAimDirection
CurrentMuzzleDirection
AimDirection = 실제 HitScan/Projectile 최종 방향
```

6. 최종 발사 방향 정책은 다음과 같다.

```text
정렬 미완료 + 허용 true  -> CurrentMuzzleDirection
정렬 완료                -> DesiredAimDirection
정렬 미완료 + 허용 false -> ValidateFireCommand에서 거부
```

7. `BuildFireCommand`, HitScan, Projectile은 같은 최종 `FireRequest.AimDirection`을 사용한다.
8. `ValidateFireCommand()`는 정책이 false일 때만 `TurretAligning`과 `WeaponNotAligned`를 거부한다.
9. `MuzzleBlocked`는 정책과 관계없이 계속 거부한다.
10. 정렬 중 발사 허용 시 Muzzle obstruction 검사는 Reticle 요구 경로가 아니라 실제 최종 발사 경로를 검사한다.
11. `UCFVehicleAimComp`의 `bLocalCanFire`는 정책 true, 유효한 Aim Solution, 유효한 최종 방향, `MuzzleBlocked=false`이면 정렬 중 true가 될 수 있어야 한다.
12. `LocalReticleState`는 발사 허용 여부와 무관하게 정렬 중 `TurretAligning`을 유지한다.
13. 관련 코드의 Version / Changelog / Migration을 갱신한다.
14. 관련 SSOT와 Systems 문서를 현재 구현 기준으로 갱신한다.
15. `Tools\BuildEditor.bat`를 실행한다.

## Required Document Updates

아래 문서를 코드 변경 후 현재 구현 기준으로 갱신한다.

```text
Document/Plan/AimFireAlignment/ImplementationDesign.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/ProjectSSOT/05_TestChecklist.md
Document/Systems/Vehicles/VehicleAim.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Combat/FireFeedback.md
```

문서는 `Build Complete / PIE Pending`으로 기록하고 사용자 확인 전 PIE PASS를 기록하지 않는다.

## Out of Scope

```text
- WBP 구조 또는 바인딩 변경
- Reticle Recovery Hotfix 제거
- TurretAligning 상태 또는 amber 표시 제거
- 신규 BindWidget
- Projectile 터널링 수정
- Damage / HP
- Ballistic Solver
- 네트워크 / 복제
- DataAsset 인스턴스 생성 또는 저장
- .uasset / .umap / StaticMesh 수정
- Git add / commit / push
```

## Constraints

1. 파일/클래스명 32자 제한을 지킨다.
2. 변수와 함수 위에 버전 포함 한 줄 요약 주석을 작성한다.
3. Blueprint 노출 프로퍼티에 한국어 `DisplayName`과 구체적인 `ToolTip`을 작성한다.
4. 계산과 발사 판정은 C++에 둔다.
5. `MuzzleBlocked`, `FireSuccess`, `Cooldown`, `WeaponNotAligned`, `OutOfArcWarning` 의미를 회귀시키지 않는다.
6. `WeaponNotAligned`가 빨간 `FireRejected`로 주 Reticle을 덮지 않는 기존 정책을 유지한다.
7. 정렬 중 발사 허용 상태에서도 Reticle은 amber `TurretAligning`을 유지한다.
8. 활성 `TurretMountData`가 없을 때 새 정책이 Missing Data 검증을 우회하지 않는다.
9. 기존 dirty worktree를 정리, restore, reset, revert하지 않는다.
10. 기존 `.uasset` 변경을 이번 작업 변경으로 오인하지 않는다.
11. 코드 작업 전후 pathspec diff로 이번 변경을 분리한다.
12. PIE PASS는 사용자가 직접 확인하기 전까지 기록하지 않는다.
13. commit/push를 수행하지 않는다.

## Acceptance Criteria

1. `bAllowFireWhileAligning`이 존재하고 기본값이 true다.
2. DisplayName, ToolTip, 버전 주석이 존재한다.
3. Turret Mount Summary에 정책값이 표시된다.
4. Aim Solution이 요구 방향, 현재 Muzzle 방향, 최종 발사 방향을 구분한다.
5. 정책 true이면 정렬 중 발사가 승인된다.
6. 정책 true의 정렬 중 탄환은 현재 Muzzle Socket X축 방향으로 진행한다.
7. 정렬 완료 후 탄환은 Reticle 목표점 방향으로 진행한다.
8. 정책 false이면 정렬 중 `TurretAligning` 또는 `WeaponNotAligned`로 거부된다.
9. `MuzzleBlocked`는 양쪽 정책에서 계속 거부된다.
10. 정렬 중 Reticle은 사라지지 않고 amber `TurretAligning`을 유지한다.
11. 정책 true일 때 `bLocalCanFire`가 정렬 중 true가 될 수 있다.
12. HitScan과 Projectile이 같은 최종 `FireRequest.AimDirection`을 사용한다.
13. FireSuccess, Cooldown, OutOfArcWarning, WeaponNotAligned UI 정책에 회귀가 없다.
14. WBP와 `.uasset` 변경이 없다.
15. `Tools\BuildEditor.bat`가 성공한다.
16. CF-FQ-022는 Active, CF-TC-019는 PARTIAL을 유지한다.
17. 문서는 Build Complete / PIE Pending으로 기록한다.
18. commit/push를 수행하지 않는다.

## Verification

### Static Review

```text
- DA 필드 기본값 true와 ToolTip 확인
- Summary 정책값 확인
- Aim Solution 방향 3종과 정책값 확인
- 최종 AimDirection 선택 조건 확인
- 실제 최종 방향 기반 Muzzle obstruction 검사 확인
- 정책 false일 때만 정렬 거부 확인
- 정책 true의 bLocalCanFire 확인
- HitScan/Projectile 방향 공유 확인
- TurretAligning Reticle 유지 확인
- 대상 5개 외 C++ 및 .uasset 변경 없음 확인
```

### Build

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

### Manual PIE — User Verification Required

정책 true:

```text
- 정렬 중 amber Reticle 유지
- 정렬 중 발사 승인
- 탄환이 현재 총구 방향으로 진행
- 정렬 완료 후 Reticle 목표 방향으로 진행
- MuzzleBlocked는 발사 거부
- FireSuccess -> Cooldown -> 현재 Aim 상태 복귀
```

정책 false:

```text
- 정렬 중 발사 거부
- Reticle 유지
- 정렬 완료 후 Ready
- 정렬 완료 후 발사 승인
```

## Documentation State

```text
CF-FQ-022 = Active
CF-TC-019 = PARTIAL
Align Fire Policy Code/Build = 완료 후 기록
Align Fire Policy PIE = Pending
```

## Failure Conditions

```text
- 정책 true인데 정렬 중 발사가 계속 거부되면 실패
- 정렬 중 탄환이 Reticle 방향으로 꺾이면 실패
- 정책 false인데 정렬 중 발사가 승인되면 실패
- MuzzleBlocked를 우회하면 실패
- Reticle Recovery가 회귀하면 실패
- 대상 외 C++ 또는 .uasset을 수정하면 실패
- 사용자 확인 없이 PIE PASS를 기록하면 실패
- commit/push를 수행하면 실패
```

## Unresolved

- 없음.
