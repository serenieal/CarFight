# Reticle 상태별 색상 표현 Task Source

- Version: 1.0.0
- Date: 2026-07-10
- Status: Ready for Codex Contract
- Project: CarFight
- Work Type: C++ UI presentation update
- artifact_role: intermediate
- codex_input: false

## Goal

`UCFAimReticleWidget`가 현재 `CachedReticleState`와 `CachedFireFeedbackViewData`를 사용해 기존 Reticle 이미지와 FireFeedback 텍스트에 의미가 분명한 상태별 색상을 적용하도록 구현한다.

현재 `CFVehiclePawn.cpp v2.106.0`의 발사 성공 → 쿨다운 → 피드백 종료 흐름은 빌드 및 PIE 검증이 완료되었다. 이번 작업은 표시 스타일만 추가하며 발사 로직과 상태 타이밍은 변경하지 않는다.

## Current Decisions

- 상태 계산과 색상 선택은 C++가 담당한다.
- WBP는 위젯 배치, 폰트, 브러시, 크기 등 시각 구성만 담당한다.
- `WBP_AimReticle.uasset`은 Codex가 직접 수정하지 않는다.
- 기존 WBP에는 아래 이미지가 이미 존재한다.
  - `Image_CenterDot`
  - `Image_LeftBracket`
  - `Image_RightBracket`
  - `Image_TopBracket`
  - `Image_BottomBracket`
- 기존 FireFeedback 텍스트는 유지한다.
  - `Text_FireFeedbackState`
  - `Text_FireFeedbackHint`
  - `Text_Cooldown`
- `OutOfArcWarning`은 주 Reticle 상태를 덮어쓰지 않는 보조 경고다.
- P0에서는 애니메이션, 머티리얼, 원형 쿨다운 게이지를 추가하지 않는다.

## In Scope

1. `UCFAimReticleWidget`에 `UImage` 전방 선언을 추가한다.
2. 위 이미지 5개를 `BindWidgetOptional`로 추가한다.
3. 필요 시 보조 경고용 `Text_OutOfArcWarning`을 `BindWidgetOptional`로 추가한다.
4. WBP에서 조정 가능한 상태별 `FLinearColor` 프로퍼티를 직관적인 이름으로 추가한다.
5. 주 Reticle 이미지 색상과 FireFeedback 텍스트 색상을 분리 계산한다.
6. `RefreshVisualStyle()` 같은 전용 함수에서 Optional 위젯을 안전하게 갱신한다.
7. 기존 `RefreshTextBlocks()` 흐름과 표시 종료 동작을 보존한다.
8. 헤더와 cpp의 Version, Changelog, Migration을 갱신한다.

## Required Color Semantics

- `Ready`: 흰색
- `FireSuccess`: 밝은 녹색 또는 흰색 계열의 짧은 성공 색상
- `Cooldown`: 파란색
- `FirePending`: 청록색 또는 파란색
- `FireRejected`: 빨간색
- `NoWeapon`: 회색
- `AimBlocked`: 주황색
- `OutOfArcWarning`: 노란색 보조 표시
- `Reloading`: 파란색 또는 청록색
- `Hidden`: 실제 표시되지 않으므로 안전한 기본색 사용

정확한 기본값은 과도하게 채도가 높지 않은 `FLinearColor`로 정한다. 색상은 `EditDefaultsOnly` 또는 `EditAnywhere`와 `BlueprintReadWrite`로 노출하여 WBP 파생 클래스에서 조정할 수 있게 한다.

## Visual Resolution Rules

### Main Reticle Color

1. 활성 FireFeedback이 `FireSuccess`이면 성공 색상을 우선 사용한다.
2. 활성 FireFeedback이 `OutOfArcWarning`이면 주 Reticle 색상을 강제로 노란색으로 변경하지 않는다.
3. 그 외에는 최종 `CachedReticleState`에 대응하는 색상을 사용한다.
4. 이미지 5개는 동일한 주 Reticle 색상을 사용한다.

### FireFeedback Text Color

1. `Text_FireFeedbackState`와 `Text_FireFeedbackHint`는 활성 `FeedbackState`에 대응하는 색상을 사용한다.
2. `Text_Cooldown`은 표시 중일 때 Cooldown 색상을 사용한다.
3. 피드백이 비활성화되면 기존처럼 빈 텍스트로 사라져야 한다.
4. 색상만으로 의미를 전달하지 않으며 기존 한국어 텍스트를 유지한다.

### OutOfArcWarning

- `CachedFireFeedbackViewData.bShowOutOfArcWarning == true`일 때만 보조 경고를 표시한다.
- `Text_OutOfArcWarning`을 추가하는 경우 문구는 `각도 경고`로 한다.
- 보조 경고 색상은 노란색이다.
- 주 Reticle 상태와 색상을 덮어쓰지 않는다.
- Optional 위젯이 WBP에 없어도 크래시가 발생하지 않아야 한다.

## Target Files

1. `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
2. `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`

## Constraints

- 수정 파일은 위 두 C++ 파일로 제한한다.
- `WBP_AimReticle.uasset`을 포함한 `.uasset` 파일은 수정하지 않는다.
- `CFVehiclePawn`, FireFeedback 타입, 발사 검증, 쿨다운 계산 및 상태 지속시간을 변경하지 않는다.
- 기존 공개 함수 시그니처와 현재 한국어 표시 문구를 깨지 않는다.
- Optional 바인딩 누락을 정상 상황으로 처리하며 null 포인터 접근을 허용하지 않는다.
- 변수명과 함수명은 역할이 드러나는 직관적인 이름을 사용한다.
- 각 선언과 정의 위에 1줄 요약 주석을 유지한다.
- 파일 버전, Changelog, Migration을 갱신한다.
- 클래스명과 파일명은 변경하지 않는다.
- Git commit 또는 push를 수행하지 않는다.

## Read-Only References

- `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `Document/Plan/ReticleFireFeedback/VisualGuide.md`
- `Document/Plan/ReticleFireFeedback/ImplementationDesign.md`

## Implementation Requirements

### Header

- `class UImage;` 추가.
- 이미지 5개 `TObjectPtr<UImage>` Optional 바인딩 추가.
- 선택적으로 `Text_OutOfArcWarning` Optional 바인딩 추가.
- 상태별 색상 프로퍼티를 의미가 드러나는 이름으로 선언한다.
  - 예: `ReadyReticleColor`, `FireSuccessReticleColor`, `CooldownReticleColor`, `FireRejectedReticleColor`, `NoWeaponReticleColor`, `AimBlockedReticleColor`, `OutOfArcWarningColor`, `FirePendingReticleColor`, `ReloadingReticleColor`.
- 다음 역할의 private 함수 또는 동등한 구조를 둔다.
  - 현재 주 Reticle 색상 계산
  - FireFeedback 텍스트 색상 계산
  - 이미지와 텍스트에 스타일 적용
- 함수와 변수 선언 위에 1줄 요약 주석을 작성한다.
- 클래스/파일명은 변경하지 않는다.

### CPP

- `Components/Image.h` include 추가.
- `RefreshVisualStyle()`를 구현하고 `RefreshTextBlocks()` 완료 후 또는 캐시 갱신 시 호출한다.
- 동일 이미지 갱신 코드를 과도하게 복제하지 말고 작은 헬퍼 또는 명확한 반복 구조를 사용한다.
- `SetColorAndOpacity()`를 사용한다.
- Optional 포인터를 매번 null 검사한다.
- 현재 표시 텍스트와 Visibility 정책은 변경하지 않는다.
- `FireSuccess`는 `CachedReticleState`가 Ready로 유지되더라도 활성 FeedbackState를 보고 성공 색상이 보이게 한다.
- `OutOfArcWarning`은 별도 경고 텍스트만 노란색으로 표시하고 메인 Reticle 색상은 기존 상태 색상을 유지한다.

## Acceptance Criteria

1. 두 C++ 파일 외의 프로젝트 파일은 수정하지 않는다.
2. WBP에 이미지 5개가 정확한 이름과 `Is Variable=true`로 존재할 때 모두 같은 주 Reticle 색상으로 갱신된다.
3. Optional 이미지 또는 `Text_OutOfArcWarning`이 없어도 위젯 생성과 Tick에서 크래시가 발생하지 않는다.
4. Ready 상태의 주 Reticle은 흰색이다.
5. 정상 발사 직후 주 Reticle과 FireFeedback 텍스트에 성공 색상이 적용된다.
6. 성공 표시 종료 후 쿨다운 상태와 쿨다운 텍스트는 파란색이 된다.
7. 쿨다운 종료 후 기존 검증 결과처럼 FireFeedback 텍스트가 남지 않는다.
8. FireRejected는 빨간색, NoWeapon은 회색, AimBlocked는 주황색으로 표시된다.
9. OutOfArcWarning은 노란색 보조 경고로 표시되며 주 Reticle을 노란색으로 강제 변경하지 않는다.
10. 기존 한국어 문구와 `FireSuccess → Cooldown → 빈 텍스트` 전환을 보존한다.
11. 헤더와 cpp의 Version/Changelog/Migration 및 1줄 요약 주석 규칙을 지킨다.
12. Unreal Editor 타깃 빌드가 성공한다.

## Verification

### Static Review

- 수정 파일이 정확히 두 개인지 `git diff --name-only`로 확인한다.
- `BindWidgetOptional` 이름과 WBP 예상 이름이 정확히 일치하는지 확인한다.
- `UImage` include/전방 선언 누락 여부를 확인한다.
- `RefreshVisualStyle()`가 캐시 갱신 후 호출되는지 확인한다.
- `OutOfArcWarning`이 주 Reticle 색상 계산을 덮어쓰지 않는지 확인한다.

### Build

- Unreal Editor를 종료한 상태에서 `D:\Work\CarFight_git\Tools\BuildEditor.bat`를 실행한다.
- 성공 시 마지막 성공 요약을 보고한다.
- 실패 시 최초 컴파일 오류부터 원인과 파일/라인을 보고하며 임의로 범위를 넓히지 않는다.

### PIE Manual Checklist

- Ready: 흰색 Reticle.
- 정상 발사: 짧은 성공 색상 + 기존 `발사` 텍스트.
- Cooldown: 파란색 Reticle/텍스트 + 남은 시간.
- Cooldown 종료: FireFeedback 텍스트 잔류 없음.
- AimBlocked: 주황색.
- NoWeapon: 회색.
- FireRejected: 빨간색.
- OutOfArcWarning: 노란색 보조 경고, 주 Reticle 색상 유지.

## Out of Scope

- `CFVehiclePawn.cpp`와 FireFeedback 우선순위/지속시간 수정
- `CFVehicleFireFeedbackTypes.h` 변경
- 발사 검증, 무기 쿨다운, Projectile, Damage 로직 변경
- 네트워크/RPC/복제 코드
- `WBP_AimReticle.uasset` 자동 수정
- UMG 애니메이션 생성
- Reticle Scale 플래시, 흔들림, Pulse
- 원형 쿨다운 게이지
- 신규 Texture/Material/VFX/SFX 제작
- Git commit 또는 push

## Codex Reporting Requirements

Codex는 작업 완료 후 아래 순서로 보고한다.

1. 수정 파일
2. 구현한 바인딩과 색상 프로퍼티
3. 상태별 색상 결정 규칙
4. 빌드 결과
5. PIE 수동 검증 체크리스트
6. 사용자가 WBP에서 해야 할 작업
   - 이미지 5개 이름 확인
   - 이미지 5개 `Is Variable=true`
   - 선택한 경우 `Text_OutOfArcWarning` 추가 및 `Is Variable=true`
7. 범위 밖 파일을 수정하지 않았는지 확인

## Unresolved

- 없음. 기본 색상값은 Codex가 VisualGuide 의미를 따르는 과하지 않은 값으로 정하고 WBP에서 조정 가능하게 노출한다.

## Changelog

- v1.0.0: Reticle 상태별 색상 표현을 위한 두 파일 한정 Codex 작업 원본 작성.
