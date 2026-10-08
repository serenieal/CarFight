# OutOfArcWarning 보조 표시 완성 Task Source

- Version: 1.1.0
- Date: 2026-07-10
- Status: Ready for Codex Contract
- Project: CarFight
- Work Type: C++ UI ViewData and presentation bug fix
- artifact_role: intermediate
- codex_input: false

## Goal

`Text_OutOfArcWarning`을 WBP에 추가했을 때 조준각 경고가 정해진 유지 시간 동안 한 곳에만 표시되고, 유지 시간이 끝나면 완전히 사라지도록 수정한다.

## Current State

### 표시 종료 문제

`ACFVehiclePawn::BuildFireFeedbackViewData()`의 `OutOfWeaponArc` 분기는 다음 상태를 만든다.

```cpp
ViewData.bFeedbackActive = bWithinRejectedFeedbackTime;
ViewData.bShowOutOfArcWarning = true;
```

따라서 피드백 유지 시간이 끝나 `bFeedbackActive == false`가 되어도 `bShowOutOfArcWarning == true`가 남을 수 있다. `UCFAimReticleWidget::RefreshVisualStyle()`는 `bShowOutOfArcWarning`만 보고 전용 경고 문구를 표시하므로 `Text_OutOfArcWarning`이 잔류할 수 있다.

### 중복 표시 문제

현재 `RefreshTextBlocks()`는 활성 `OutOfArcWarning`을 일반 FireFeedback과 동일하게 처리한다.

```text
Text_FireFeedbackState = 각도 경고
Text_FireFeedbackHint  = 조준각 경고
```

여기에 전용 `Text_OutOfArcWarning`을 추가하면 같은 경고가 중복 표시된다.

원하는 정책은 다음과 같다.

```text
전용 Text_OutOfArcWarning이 바인딩됨:
- 전용 경고 텍스트만 표시
- Text_FireFeedbackState / Hint에는 OutOfArcWarning 문구를 중복 출력하지 않음

전용 Text_OutOfArcWarning이 없음:
- 기존 Text_FireFeedbackState / Hint를 fallback으로 사용
```

## In Scope

- `CFVehiclePawn.cpp`에서 `bShowOutOfArcWarning` 표시 종료 조건 수정
- `CFAimReticleWidget.cpp`에서 전용 OutOfArc 경고의 중복 표시 방지와 fallback 처리
- 두 파일의 Version, Changelog, Migration 갱신
- Unreal Editor 타깃 빌드 확인

## Target Files

1. `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
2. `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`

## Constraints

- 수정 파일은 위 두 cpp 파일로 제한한다.
- 헤더 파일과 `WBP_AimReticle.uasset`은 수정하지 않는다.
- 발사 판정, RejectReason 선택, 피드백 지속시간 값, 쿨다운 계산을 변경하지 않는다.
- `OutOfArcWarning`이 주 Reticle 상태와 주 Reticle 색상을 덮어쓰지 않는 정책을 유지한다.
- 기존 `FireSuccess → Cooldown → 종료` 동작을 변경하지 않는다.
- 다른 RejectReason 분기의 동작을 변경하지 않는다.
- 기존 공개 함수 시그니처를 변경하지 않는다.
- 새로운 위젯 바인딩이나 UPROPERTY를 추가하지 않는다.
- 파일 Version, Changelog, Migration을 갱신한다.
- 변경 함수와 핵심 지역 변수 위에 1줄 요약 주석 규칙을 유지한다.
- Git commit 또는 push를 수행하지 않는다.

## Required Changes

### 1. CFVehiclePawn.cpp

`ECFVehicleFireRejectReason::OutOfWeaponArc` 분기에서:

```cpp
ViewData.bShowOutOfArcWarning = true;
```

를 다음 의미가 되도록 변경한다.

```cpp
ViewData.bShowOutOfArcWarning = bWithinRejectedFeedbackTime;
```

또는 동일한 결과를 내는 명확한 조건식을 사용한다.

`bShowOutOfArcWarning`은 이름과 타입 주석대로 “현재 조준각 경고를 화면에 표시해야 하는지”를 직접 나타내야 한다.

### 2. CFAimReticleWidget.cpp

`Text_OutOfArcWarning`이 실제 바인딩되어 있고 현재 `OutOfArcWarning`이 표시 중인지 판단하는 명확한 bool 또는 private 구현 로직을 둔다. 헤더 수정은 금지하므로 cpp 내부 지역 변수나 기존 함수 내부 로직으로 처리한다.

전용 경고 사용 조건 예시:

```cpp
const bool bUseDedicatedOutOfArcWarning =
    IsValid(Text_OutOfArcWarning)
    && CachedFireFeedbackViewData.bFeedbackActive
    && CachedFireFeedbackViewData.bShowOutOfArcWarning
    && CachedFireFeedbackViewData.FeedbackState == ECFVehicleFireFeedbackState::OutOfArcWarning;
```

정책:

- `bUseDedicatedOutOfArcWarning == true`이면 `Text_FireFeedbackState`와 `Text_FireFeedbackHint`는 빈 텍스트로 처리한다.
- 전용 위젯에는 `각도 경고`를 노란색으로 표시한다.
- 전용 위젯이 없으면 기존 `Text_FireFeedbackState = 각도 경고`, `Text_FireFeedbackHint = 조준각 경고` fallback을 유지한다.
- 유지 시간이 끝나면 전용 위젯과 fallback 텍스트가 모두 빈 텍스트가 된다.
- 메인 Reticle 색상은 기존 상태 색상을 유지한다.

## Acceptance Criteria

1. 수정 파일은 `CFVehiclePawn.cpp`와 `CFAimReticleWidget.cpp` 두 개뿐이다.
2. `OutOfWeaponArc` 발생 직후 유지 시간 안에서는 `bFeedbackActive == true`, `bShowOutOfArcWarning == true`다.
3. 유지 시간이 끝나면 `bFeedbackActive == false`, `bShowOutOfArcWarning == false`다.
4. `bOverrideReticleState == false` 정책은 유지된다.
5. `FeedbackDisplayKey`는 유지 시간 안에서만 `OutOfArcWarning`, 이후 `NAME_None`인 기존 동작을 유지한다.
6. `Text_OutOfArcWarning`이 바인딩되면 전용 문구만 표시되고 State/Hint에는 같은 경고가 중복되지 않는다.
7. `Text_OutOfArcWarning`이 없으면 기존 State/Hint 경고가 정상 fallback으로 표시된다.
8. 유지 시간이 끝나면 전용 경고와 fallback 경고가 모두 사라진다.
9. 주 Reticle 색상은 OutOfArcWarning 때문에 노란색으로 강제 변경되지 않는다.
10. WeaponCooldown 잔류 수정과 FireSuccess 우선순위 동작을 변경하지 않는다.
11. Unreal Editor 타깃 빌드가 성공한다.

## Verification

### Static Review

- 이번 작업 diff에서 대상 두 cpp 파일 외 파일을 수정하지 않았는지 확인한다.
- Pawn의 `bShowOutOfArcWarning`이 `bWithinRejectedFeedbackTime`을 따르는지 확인한다.
- 전용 경고 위젯 존재 여부에 따른 중복 방지/fallback 분기가 명확한지 확인한다.
- `GetMainReticleColor()`의 OutOfArcWarning 비덮어쓰기 정책이 유지되는지 확인한다.

### Build

- Unreal Editor를 종료한 상태에서 `D:\Work\CarFight_git\Tools\BuildEditor.bat`를 실행한다.
- 빌드 성공 여부를 보고한다.

### PIE Manual Check A: 전용 위젯 없음

1. 기존 WBP 상태로 OutOfWeaponArc 발사를 시도한다.
2. 기존 `Text_FireFeedbackState / Hint`에 경고가 잠깐 표시되는지 확인한다.
3. 유지 시간이 끝나면 두 텍스트가 사라지는지 확인한다.

### PIE Manual Check B: 전용 위젯 있음

1. WBP에 `Text_OutOfArcWarning`을 정확한 이름으로 추가하고 `Is Variable=true`로 설정한다.
2. OutOfWeaponArc 발사를 시도한다.
3. 노란색 `각도 경고`가 전용 위젯에만 표시되는지 확인한다.
4. State/Hint에 같은 경고가 중복되지 않는지 확인한다.
5. 유지 시간이 끝나면 전용 경고가 사라지는지 확인한다.
6. 주 Reticle 색상과 상태가 강제로 노란색/OutOfArc로 바뀌지 않는지 확인한다.

## Out of Scope

- OutOfArc 판정 방식과 조준각 계산 변경
- 새로운 경고 지속시간 프로퍼티 추가
- 위젯 배치, 폰트, 색상 기본값 변경
- 헤더 또는 FireFeedback 타입 구조 변경
- `.uasset` 자동 수정
- 문서 현행화
- Git commit 또는 push

## Unresolved

- 없음.

## Changelog

- v1.1.0: 전용 OutOfArc 경고의 표시 종료와 State/Hint 중복 방지 fallback 정책을 함께 정의.
- v1.0.0: `bShowOutOfArcWarning` 잔류 가능성을 제거하기 위한 단일 파일 작업 원본 작성.
