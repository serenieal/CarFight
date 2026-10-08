# OutOfArc 전용 경고 조건 수정 Task Source

- Version: 1.0.0
- Date: 2026-07-13
- Status: Ready for Codex Contract
- Project: CarFight
- Work Type: C++ UI condition bug fix
- artifact_role: intermediate
- codex_input: false

## Goal

`Text_OutOfArcWarning`이 바인딩된 상태에서 현재 조준이 무기 각도 밖이라는 이유만으로 `FireSuccess`, `Cooldown`, `FireRejected` 등 다른 일반 FireFeedback 텍스트가 가려지지 않도록 전용 OutOfArc 경고 사용 조건을 수정한다.

## Current Problem

현재 `UCFAimReticleWidget::RefreshTextBlocks()`의 전용 경고 조건은 다음과 같다.

```cpp
const bool bUseDedicatedOutOfArcWarning = IsValid(Text_OutOfArcWarning)
    && CachedFireFeedbackViewData.bShowOutOfArcWarning;
```

`bShowOutOfArcWarning`은 `BuildFireFeedbackViewData()`에서 현재 `LocalAimState.bLocalWithinWeaponArc == false`인 경우에도 true가 될 수 있다.

따라서 조준각 밖인 상태에서 발사 성공이나 쿨다운 피드백이 활성화되면 다음 문제가 생길 수 있다.

```text
- Text_OutOfArcWarning에는 각도 경고가 표시됨
- bUseDedicatedOutOfArcWarning == true
- Text_FireFeedbackState / Text_FireFeedbackHint가 빈 텍스트 처리됨
- 실제 FireSuccess 또는 Cooldown 문구가 전용 각도 경고에 의해 가려짐
```

이 동작은 이전 작업지시서와 완료 보고에 적힌 조건과 다르다.
전용 경고가 일반 State/Hint를 대체해야 하는 경우는 현재 활성 피드백 자체가 `OutOfArcWarning`일 때뿐이다.

## In Scope

- `UCFAimReticleWidget::RefreshTextBlocks()`의 `bUseDedicatedOutOfArcWarning` 조건 수정
- `CFAimReticleWidget.cpp` Version / Changelog / Migration 갱신
- Unreal Editor 타깃 빌드 확인

## Target Files

- `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`

## Constraints

- 수정 파일은 `CFAimReticleWidget.cpp` 한 개로 제한한다.
- 헤더 파일과 `.uasset` 파일은 수정하지 않는다.
- `CFVehiclePawn.cpp`, FireFeedback ViewData 생성, 쿨다운 계산, 발사 판정을 변경하지 않는다.
- `Text_OutOfArcWarning` 자체의 표시 조건은 현재 `bShowOutOfArcWarning` 기준을 유지한다.
- 주 Reticle 색상 계산을 변경하지 않는다.
- 기존 한국어 문구를 변경하지 않는다.
- 기존 공개 함수 시그니처를 변경하지 않는다.
- Git commit 또는 push를 수행하지 않는다.

## Required Change

`RefreshTextBlocks()`에서 전용 OutOfArc 경고가 일반 FireFeedback State/Hint를 대체하는 조건을 다음 의미로 변경한다.

```cpp
const bool bUseDedicatedOutOfArcWarning = IsValid(Text_OutOfArcWarning)
    && CachedFireFeedbackViewData.bFeedbackActive
    && CachedFireFeedbackViewData.bShowOutOfArcWarning
    && CachedFireFeedbackViewData.FeedbackState == ECFVehicleFireFeedbackState::OutOfArcWarning;
```

동등하게 읽기 쉬운 지역 bool로 분리해도 된다.

중요한 동작 구분:

```text
현재 FeedbackState == OutOfArcWarning:
- 전용 Text_OutOfArcWarning이 있으면 전용 경고만 표시
- 일반 Text_FireFeedbackState / Hint는 빈 텍스트
- 전용 위젯이 없으면 일반 State/Hint fallback 유지

현재 FeedbackState == FireSuccess / Cooldown / FireRejected / NoWeapon / AimBlocked:
- bShowOutOfArcWarning이 true여도 일반 FireFeedback State/Hint는 정상 표시
- Text_OutOfArcWarning은 현재 정책대로 보조 경고를 동시에 표시할 수 있음
```

## Acceptance Criteria

1. 수정 파일은 `CFAimReticleWidget.cpp` 한 개뿐이다.
2. 전용 OutOfArc 경고가 일반 피드백을 대체하려면 아래 조건이 모두 필요하다.
   - `Text_OutOfArcWarning` 유효
   - `bFeedbackActive == true`
   - `bShowOutOfArcWarning == true`
   - `FeedbackState == OutOfArcWarning`
3. `FeedbackState == FireSuccess`이고 `bShowOutOfArcWarning == true`여도 `Text_FireFeedbackState / Hint`에 발사 성공 문구가 표시된다.
4. `FeedbackState == Cooldown`이고 `bShowOutOfArcWarning == true`여도 재사용 대기 문구와 쿨다운 숫자가 표시된다.
5. 실제 `OutOfArcWarning` 피드백에서는 전용 위젯이 있으면 중복 State/Hint가 표시되지 않는다.
6. 전용 위젯이 없으면 기존 State/Hint fallback이 유지된다.
7. 주 Reticle 색상과 기존 피드백 종료 동작이 바뀌지 않는다.
8. Unreal Editor 타깃 빌드가 성공한다.

## Verification

### Static Review

- `RefreshTextBlocks()`의 전용 경고 조건에 `bFeedbackActive`와 `FeedbackState == OutOfArcWarning`이 포함됐는지 확인한다.
- `RefreshVisualStyle()`의 `Text_OutOfArcWarning` 표시 로직은 불필요하게 변경하지 않았는지 확인한다.
- `GetMainReticleColor()`와 `GetFireFeedbackTextColor()`에 변경이 없는지 확인한다.
- `git diff --name-only`에서 대상 cpp 외 이번 작업 파일 수정이 없는지 확인한다.

### Build

- Unreal Editor를 종료한 상태에서 `D:\Work\CarFight_git\Tools\BuildEditor.bat`를 실행한다.
- 빌드 성공 여부를 보고한다.

### PIE Manual Check

1. 조준각 밖 상태를 만든다.
2. 정상 발사 가능한 조건에서 발사한다.
3. 녹색 Reticle과 `발사 / 발사 요청 수락`이 보이고, 각도 경고가 이를 가리지 않는지 확인한다.
4. 쿨다운 중에는 `재사용 대기 / 무기 재사용 대기 중 / 남은 시간`이 보이는지 확인한다.
5. 실제 OutOfWeaponArc 거부 상태에서는 전용 `각도 경고`만 보이고 State/Hint 중복이 없는지 확인한다.
6. 유지 시간이 끝나면 경고가 사라지는지 확인한다.

## Out of Scope

- `bShowOutOfArcWarning`의 데이터 의미 변경
- OutOfArc 판정 방식 변경
- 발사 가능 조건 변경
- Reticle 색상 기본값 변경
- WBP 배치 변경
- 문서 현행화
- Git commit 또는 push

## Unresolved

- 없음.

## Changelog

- v1.0.0: 전용 OutOfArc 경고가 다른 FireFeedback State/Hint를 잘못 가리는 조건 문제 수정 계획 작성.
