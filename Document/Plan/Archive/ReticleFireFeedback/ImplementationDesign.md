# Reticle / FireFeedback ImplementationDesign

- Version: 1.2.0
- Date: 2026-07-15
- Status: Done / User PIE PASS / Systems Current
- Scope: Reticle / FireFeedback의 구현 계획, 실제 완료 결과와 최종 사용자 PIE 검증 기록

---

## 현재 작업 체크포인트

- 현재 상태: 구현, 공식 Editor 빌드와 최종 사용자 PIE 검증 완료
- 빌드 상태: `Tools\BuildEditor.bat` PASS
- PIE 상태: `NoWeapon`, `AimBlocked`, 정상 발사 회귀 PASS / `CF-FQ-017` Done / `CF-TC-014` PASS

### 완료한 범위

- FireFeedback ViewData와 Reticle 통합 구현
- WBP Reticle 이미지, 상태 텍스트, Hint, Cooldown과 OutOfArc 경고 연결
- Ready, FireSuccess, Cooldown 색상 전환과 연속 입력 후 텍스트 종료 사용자 PIE 확인
- OutOfArc 전용 경고 조건 사용자 PIE 확인
- AimFireAlignment의 `TurretAligning`과 `MuzzleBlocked` 표시 경로 연결

### 최종 사용자 PIE 확인

- `NoWeapon` 회색 Reticle과 무기 없음 안내 문구 PASS
- `AimBlocked` 주황 Reticle과 조준 가림 안내 문구 PASS
- 두 실패 상태의 유지 시간 종료 후 텍스트 제거 PASS
- 장애물 제거 후 정상 Aim 상태 복귀 PASS
- `Ready → FireSuccess → Cooldown → Ready` 정상 발사 회귀 PASS
- `CF-FQ-022` 정렬 정책 Reticle 상태 회귀 PASS

### 완료 후 처리

1. `CF-FQ-017`을 Done, `CF-TC-014`를 PASS로 유지한다.
2. 현재 구현 판단은 `Document/Systems/Combat/FireFeedback.md`와 `Document/Systems/UI/AimReticle.md`를 우선한다.
3. 이 Plan은 완료 당시 설계와 검증 기록 보존용으로 유지하고 Archive 이동을 별도 문서 정리 때 검토한다.

### 관련 코드와 문서 경로

- `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `UE/Content/CarFight/UI/WBP_AimReticle.uasset`
- `Document/Systems/UI/AimReticle.md`
- `Document/Systems/Combat/FireFeedback.md`
- `Document/Systems/Combat/WeaponFire.md`

### 보호 범위

- 서버 대기·서버 거부와 멀티플레이 피드백을 현재 싱글 전투 범위에 추가하지 않는다.
- WBP 바인딩 이름과 기존 정상 상태 표현을 불필요하게 변경하지 않는다.
- 완료 이후 현재 구현 판단은 Systems 문서를 우선하고, 이 Plan은 검증 이력 보존용으로 유지한다.

---

## 1. 문서 목적

이 문서는 `Reticle / FireFeedback` 구현 당시의 C++/BP 분담, 파일 단위 계획, 결정 이력과 실제 구현 결과를 함께 보존하는 Plan 문서다.

현재 구현의 SSOT는 아래 Systems 문서이며, 이 Plan은 남은 PIE 검증이 끝나면 Archive 대상으로 전환한다.

```text
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Vehicles/VehicleAim.md
```

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 이 문서에서 Reticle / FireFeedback은 서버 대기 / 서버 거부 상태를 표시하지 않는다.

### 1.1 구현 결과 요약

현재 구현 완료:

```text
- CFVehicleFireFeedbackTypes.h 타입 정의
- ACFVehiclePawn::BuildFireFeedbackViewData() 구현
- UCFAimReticleWidget FireFeedback 통합
- WBP_AimReticle 이미지 5개와 State/Hint/Cooldown/OutOfArc 경고 연결
- 상태별 색상 적용
- FireSuccess → Cooldown → 종료 우선순위 구현
- WeaponCooldown / OutOfArcWarning 텍스트 잔류 수정
- 전용 OutOfArc 경고 중복 방지와 다른 FireFeedback 비가림 조건 수정
- Unreal Editor 타깃 빌드 성공
```

PIE 확인 완료:

```text
- Ready 흰색
- FireSuccess 녹색
- Cooldown 파란색
- 연속 입력 후 쿨다운 종료 시 텍스트 제거
- 조준각 밖에서 FireSuccess / Cooldown / FireRejected 문구 유지
- 실제 OutOfArcWarning에서 전용 경고만 일반 State/Hint를 대체
```

Plan 종료 전 최종 확인 완료:

```text
- NoWeapon 실제 PIE 표시와 회색 상태 PASS
- AimBlocked 실제 PIE 표시와 주황 상태 PASS
- 두 실패 상태의 유지 시간 종료 후 텍스트 제거 PASS
- 장애물 제거 후 정상 Aim 복귀와 정상 발사 회귀 PASS
```

---

## 2. 설계 목표

이번 구현의 목표는 아래다.

```text
- WeaponFire가 기록한 로컬 발사 결과를 Reticle UI에서 읽을 수 있게 한다.
- 발사 성공 / 발사 불가 / 쿨다운 / 무기 없음 / 조준 막힘을 구분한다.
- OutOfArcWarning은 단독 발사 차단이 아니라 보조 경고로 표시한다.
- BP는 표시만 담당하고, 상태 계산은 C++에서 처리한다.
- 기존 AimReticle 구조를 크게 흔들지 않는다.
```

P0에서 가장 중요한 플레이어 경험은 아래다.

```text
플레이어가 Fire 입력을 눌렀을 때,
쐈는지 / 왜 못 쐈는지 / 아직 쿨다운인지 즉시 알 수 있어야 한다.
```

---

## 3. 구현하지 않을 항목

이번 P0 구현에서 제외할 항목은 아래다.

```text
- 전투 HUD 전체 신설
- Common UI 레이어 이전
- 무기별 고유 Reticle 대량 제작
- 복잡한 원형 쿨다운 게이지
- 고품질 VFX / SFX 완성
- Damage / HP / 파괴 처리
- 서버 권한 발사 / 복제 / 서버 응답 UI
- FireFeedback 전담 ActorComponent 신설
```

`FireFeedback` 전담 컴포넌트는 장기적으로 가능하지만, 현재 P0에서는 연결 지점이 늘어나므로 만들지 않는다.

---

## 4. 전체 구조 결정

이번 구현은 아래 구조를 따른다.

```text
ACFVehiclePawn
  -> LastFireRequest / LastFireResult 보관
  -> VehicleWeaponComp의 쿨다운 상태 읽기
  -> VehicleAimComp의 LocalAimState 읽기
  -> BuildFireFeedbackViewData()로 UI 표시 데이터 생성

UCFAimReticleWidget
  -> VehicleAimComp에서 기본 Aim / Reticle 상태 읽기
  -> VehiclePawn에서 FireFeedbackViewData 읽기
  -> 최종 Reticle 표시 상태와 피드백 텍스트 갱신

WBP_AimReticle
  -> 이미지 / 텍스트 배치, 브러시, 폰트 등 시각 자산 구성
  -> C++가 적용한 상태별 색상과 문구 표시
```

핵심 분리 기준:

```text
C++ = 상태 계산 / 우선순위 / 표시 데이터 생성 / 텍스트와 상태별 색상 적용
BP = 이미지 / 텍스트 배치 / 브러시 / 폰트 / 크기 등 시각 자산 구성
```

---

## 5. C++ 신규 파일 계획

### 5.1 신규 파일

```text
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

작업 유형:

```text
신규 작성
```

이 파일에는 Reticle / FireFeedback UI가 공통으로 읽을 표시 상태 타입을 정의한다.

### 5.2 파일명 기준

파일명 길이는 UE C++ 클래스/파일명 관리 기준 안에 들어온다.

```text
CFVehicleFireFeedbackTypes.h
```

이 파일은 UObject 클래스가 아니라 enum / struct 타입 모음이다.
따라서 별도 `.cpp` 파일은 만들지 않는다.

---

## 6. 신규 enum 설계

삽입 위치:

```text
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

선언안:

```cpp
// [v0.1] Reticle / UI에 표시할 로컬 발사 피드백 상태입니다.
UENUM(BlueprintType)
enum class ECFVehicleFireFeedbackState : uint8
{
    None,

    FireSuccess,

    FirePending,

    FireRejected,

    Cooldown,

    NoWeapon,

    AimBlocked,

    OutOfArcWarning
};
```

상태 의미:

| 상태 | 의미 |
| --- | --- |
| `None` | 현재 별도 발사 피드백 없음 |
| `FireSuccess` | 발사 성공 플래시 후보 |
| `FirePending` | 로컬 발사 처리 중 후보. 서버 대기 아님 |
| `FireRejected` | 로컬 발사 조건 미충족 |
| `Cooldown` | 무기 재사용 대기 |
| `NoWeapon` | 무기 없음 또는 무기 데이터 비호환 |
| `AimBlocked` | 조준선 막힘 |
| `OutOfArcWarning` | 조준각 경고. 단독 발사 차단 아님 |

---

## 7. 신규 struct 설계

삽입 위치:

```text
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

선언안:

```cpp
// [v0.1] Reticle / FireFeedback UI가 읽을 로컬 발사 피드백 표시 데이터입니다.
USTRUCT(BlueprintType)
struct FCFVehicleFireFeedbackViewData
{
    GENERATED_BODY()

    // [v0.1] 현재 표시할 발사 피드백 상태입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    ECFVehicleFireFeedbackState FeedbackState = ECFVehicleFireFeedbackState::None;

    // [v0.1] WeaponFire에서 기록한 마지막 발사 거부 사유입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    ECFVehicleFireRejectReason LastRejectReason = ECFVehicleFireRejectReason::None;

    // [v0.1] 현재 피드백을 화면에 표시해야 하는지 여부입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    bool bFeedbackActive = false;

    // [v0.1] 기본 Reticle 상태를 피드백 상태로 덮어써야 하는지 여부입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    bool bOverrideReticleState = false;

    // [v0.1] 쿨다운 UI를 표시해야 하는지 여부입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    bool bShowCooldown = false;

    // [v0.1] 남은 쿨다운 시간입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    float RemainingCooldownSeconds = 0.0f;

    // [v0.1] 전체 쿨다운 시간입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    float TotalCooldownSeconds = 0.0f;

    // [v0.1] 0.0~1.0 범위의 쿨다운 비율입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    float CooldownRatio = 0.0f;

    // [v0.1] 조준각 경고를 보조 표시로 띄워야 하는지 여부입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    bool bShowOutOfArcWarning = false;

    // [v0.1] FireFeedback 상태를 한국어 표시로 변환하기 전의 짧은 내부 표시 키입니다.
    UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
    FName FeedbackDisplayKey = NAME_None;
};
```

필요 include 후보:

```cpp
#include "CoreMinimal.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleFireFeedbackTypes.generated.h"
```

`ECFVehicleFireRejectReason`가 `CFVehicleAimTypes.h`에 있으므로 해당 include가 필요하다.

---

## 8. ACFVehiclePawn 수정 계획

수정 파일:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

작업 유형:

```text
수정
```

---

## 9. ACFVehiclePawn.h 추가 항목

### 9.1 Include 추가

삽입 위치:

```text
CFVehiclePawn.h 상단 include 영역
```

추가 후보:

```cpp
#include "CFVehicleFireFeedbackTypes.h"
```

### 9.2 UPROPERTY 추가

삽입 위치:

```text
ACFVehiclePawn 클래스의 Fire / Weapon 관련 설정 영역
```

선언안:

```cpp
// [v0.1] 마지막 로컬 발사 피드백이 시작된 월드 시간입니다.
UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
double LastFireFeedbackStartTimeSeconds = -1.0;

// [v0.1] 발사 성공 피드백을 화면에 유지할 시간입니다.
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
float FireSuccessFeedbackDurationSeconds = 0.12f;

// [v0.1] 발사 실패 피드백을 화면에 유지할 시간입니다.
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
float FireRejectedFeedbackDurationSeconds = 0.35f;
```

### 9.3 UFUNCTION 추가

삽입 위치:

```text
ACFVehiclePawn 클래스 public 함수 영역 또는 UI / Debug 함수 영역
```

선언안:

```cpp
// [v0.1] Reticle / FireFeedback UI가 읽을 현재 로컬 발사 피드백 표시 데이터를 만듭니다.
UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle|FireFeedback")
FCFVehicleFireFeedbackViewData BuildFireFeedbackViewData() const;
```

---

## 10. ACFVehiclePawn.cpp 구현 계획

### 10.1 ApplyFireResult 수정

수정 대상:

```text
ACFVehiclePawn::ApplyFireResult()
```

추가 동작:

```text
- LastFireRequest 저장
- LastFireResult 저장
- 기존 VehicleWeaponComp / VehicleAimComp 반영 유지
- 마지막에 LastFireFeedbackStartTimeSeconds 갱신
```

추가 후보 코드:

```cpp
// [v0.1] 로컬 FireFeedback 표시 시작 시간을 기록합니다.
LastFireFeedbackStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;
```

주의:

```text
- 성공/실패 모두 피드백 시작 시간은 갱신한다.
- 단, 완전히 유효하지 않은 입력에서 ApplyFireResult가 호출되지 않는 구조라면 별도 처리하지 않는다.
```

### 10.2 BuildFireFeedbackViewData 구현

삽입 위치:

```text
CFVehiclePawn.cpp의 Fire 관련 함수 근처
```

처리 순서:

```text
1. 기본 ViewData 생성
2. LastFireResult.RejectReason 저장
3. VehicleWeaponComp에서 쿨다운 값 읽기
4. VehicleAimComp에서 LocalAimState 읽기
5. OutOfArcWarning 보조 여부 계산
6. 쿨다운 표시 여부 계산
7. LastFireResult 기반 FeedbackState 계산
8. 표시 유지 시간 계산
9. bFeedbackActive / bOverrideReticleState 결정
10. FeedbackDisplayKey 설정
```

의사 코드:

```cpp
// [v0.1] Reticle / FireFeedback UI가 읽을 현재 로컬 발사 피드백 표시 데이터를 만듭니다.
FCFVehicleFireFeedbackViewData ACFVehiclePawn::BuildFireFeedbackViewData() const
{
    FCFVehicleFireFeedbackViewData ViewData;

    ViewData.LastRejectReason = LastFireResult.RejectReason;

    const double CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    const double FeedbackAgeSeconds = LastFireFeedbackStartTimeSeconds >= 0.0
        ? CurrentTimeSeconds - LastFireFeedbackStartTimeSeconds
        : TNumericLimits<double>::Max();

    const float TotalCooldownSeconds = VehicleWeaponComp
        ? VehicleWeaponComp->GetActiveWeaponCooldownSeconds()
        : 0.0f;

        const float RemainingCooldownSeconds = VehicleWeaponComp
        ? VehicleWeaponComp->GetRemainingCooldownSeconds(static_cast<float>(CurrentTimeSeconds))
        : 0.0f;

    ViewData.TotalCooldownSeconds = TotalCooldownSeconds;
    ViewData.RemainingCooldownSeconds = RemainingCooldownSeconds;
    ViewData.bShowCooldown = TotalCooldownSeconds > 0.0f && RemainingCooldownSeconds > 0.0f;
    ViewData.CooldownRatio = ViewData.bShowCooldown
        ? FMath::Clamp(RemainingCooldownSeconds / TotalCooldownSeconds, 0.0f, 1.0f)
        : 0.0f;

    if (VehicleAimComp)
    {
        const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();
        ViewData.bShowOutOfArcWarning = !LocalAimState.bLocalWithinWeaponArc;
    }

    // 우선순위 1: 쿨다운은 지속 상태이므로 최근 실패 피드백보다 우선 표시할 수 있다.
    if (ViewData.bShowCooldown)
    {
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::Cooldown;
        ViewData.bFeedbackActive = true;
        ViewData.bOverrideReticleState = true;
        ViewData.FeedbackDisplayKey = TEXT("Cooldown");
        return ViewData;
    }

    if (LastFireResult.bAccepted)
    {
        const bool bWithinSuccessFeedbackTime = FeedbackAgeSeconds <= FireSuccessFeedbackDurationSeconds;
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::FireSuccess;
        ViewData.bFeedbackActive = bWithinSuccessFeedbackTime;
        ViewData.bOverrideReticleState = false;
        ViewData.FeedbackDisplayKey = TEXT("FireSuccess");
        return ViewData;
    }

    switch (LastFireResult.RejectReason)
    {
    case ECFVehicleFireRejectReason::NoWeapon:
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::NoWeapon;
        ViewData.bFeedbackActive = FeedbackAgeSeconds <= FireRejectedFeedbackDurationSeconds;
        ViewData.bOverrideReticleState = true;
        ViewData.FeedbackDisplayKey = TEXT("NoWeapon");
        break;

    case ECFVehicleFireRejectReason::WeaponCooldown:
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::Cooldown;
        ViewData.bFeedbackActive = true;
        ViewData.bOverrideReticleState = true;
        ViewData.FeedbackDisplayKey = TEXT("Cooldown");
        break;

    case ECFVehicleFireRejectReason::AimBlocked:
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::AimBlocked;
        ViewData.bFeedbackActive = FeedbackAgeSeconds <= FireRejectedFeedbackDurationSeconds;
        ViewData.bOverrideReticleState = true;
        ViewData.FeedbackDisplayKey = TEXT("AimBlocked");
        break;

    case ECFVehicleFireRejectReason::OutOfWeaponArc:
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::OutOfArcWarning;
        ViewData.bFeedbackActive = true;
        ViewData.bOverrideReticleState = false;
        ViewData.bShowOutOfArcWarning = true;
        ViewData.FeedbackDisplayKey = TEXT("OutOfArcWarning");
        break;

    case ECFVehicleFireRejectReason::None:
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::None;
        ViewData.bFeedbackActive = false;
        ViewData.bOverrideReticleState = false;
        ViewData.FeedbackDisplayKey = NAME_None;
        break;

    default:
        ViewData.FeedbackState = ECFVehicleFireFeedbackState::FireRejected;
        ViewData.bFeedbackActive = FeedbackAgeSeconds <= FireRejectedFeedbackDurationSeconds;
        ViewData.bOverrideReticleState = true;
        ViewData.FeedbackDisplayKey = TEXT("FireRejected");
        break;
    }

    return ViewData;
}
```

주의:

```text
- 위 코드는 설계용 의사 코드다.
- 실제 구현 전 VehicleWeaponComp의 Getter 함수명이 정확히 존재하는지 확인해야 한다.
- Getter가 없다면 VehicleWeaponComp에 BlueprintCallable / const Getter를 추가해야 한다.
```

---

## 11. UCFVehicleWeaponComp Getter 확인 결과 / 기존 Getter 사용 계획

확인 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
```

현재 코드에 이미 존재하는 Getter:

```cpp
// [v1.7.0] 활성 WeaponData의 분당 발사속도를 초 단위 발사 간격으로 환산해 반환합니다.
float GetActiveWeaponCooldownSeconds() const;

// [v1.2.0] 현재 시간 기준 남은 무기 쿨다운 시간을 반환합니다.
float GetRemainingCooldownSeconds(float CurrentTimeSeconds) const;

// [v1.2.0] 현재 시간 기준 활성 무기가 쿨다운 중인지 반환합니다.
bool IsActiveWeaponOnCooldown(float CurrentTimeSeconds) const;
```

따라서 P0 Reticle / FireFeedback 구현에서는 `UCFVehicleWeaponComp`에 새 쿨다운 Getter를 추가하지 않는다.
전체 쿨다운 시간은 `GetActiveWeaponCooldownSeconds()`를 사용하고, 남은 쿨다운 시간은 `GetRemainingCooldownSeconds(CurrentTimeSeconds)`를 사용한다.

---

## 12. UCFAimReticleWidget 수정 계획

수정 파일:

```text
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

작업 유형:

```text
수정
```

---

## 13. CFAimReticleWidget.h 추가 항목

### 13.1 Include 추가

```cpp
#include "CFVehicleFireFeedbackTypes.h"
```

### 13.2 Optional TextBlock 추가

삽입 위치:

```text
기존 Text_ReticleState / Text_CanFire / Text_ReticleHint 근처
```

선언안:

```cpp
// [v0.1] 현재 발사 피드백 상태를 표시하는 선택적 텍스트입니다.
UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="CarFight|AimReticle")
TObjectPtr<UTextBlock> Text_FireFeedbackState = nullptr;

// [v0.1] 현재 발사 피드백 보조 설명을 표시하는 선택적 텍스트입니다.
UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="CarFight|AimReticle")
TObjectPtr<UTextBlock> Text_FireFeedbackHint = nullptr;

// [v0.1] 현재 남은 쿨다운 시간을 표시하는 선택적 텍스트입니다.
UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="CarFight|AimReticle")
TObjectPtr<UTextBlock> Text_Cooldown = nullptr;
```

### 13.3 캐시 변수 추가

```cpp
// [v0.1] 마지막으로 표시한 FireFeedback ViewData입니다.
UPROPERTY(BlueprintReadOnly, Category="CarFight|AimReticle")
FCFVehicleFireFeedbackViewData CachedFireFeedbackViewData;
```

### 13.4 함수 추가

```cpp
// [v0.1] FireFeedback ViewData를 Reticle 표시 텍스트에 반영합니다.
void ApplyFireFeedbackViewData(const FCFVehicleFireFeedbackViewData& InViewData);

// [v0.1] FireFeedback 상태를 한국어 표시 텍스트로 변환합니다.
FText GetFireFeedbackStateDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const;

// [v0.1] FireFeedback 상태를 한국어 보조 설명으로 변환합니다.
FText GetFireFeedbackHintDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const;

// [v0.1] FireFeedback 상태가 기본 ReticleState를 덮어쓸 때 사용할 ReticleState를 반환합니다.
ECFVehicleReticleState ResolveReticleStateFromFireFeedback(const FCFVehicleFireFeedbackViewData& InViewData, ECFVehicleReticleState BaseReticleState) const;
```

---

## 14. CFAimReticleWidget.cpp 수정 계획

### 14.1 RefreshFromPawn 확장

현재 흐름:

```text
VehicleAimComp에서 LocalAimState 읽기
VehicleAimComp에서 ReticleState 읽기
ApplyReticleState 호출
UpdateReticleVisibility 호출
```

변경 흐름:

```text
VehicleAimComp에서 LocalAimState 읽기
VehicleAimComp에서 BaseReticleState 읽기
VehiclePawn에서 FireFeedbackViewData 읽기
FireFeedbackViewData가 ReticleState를 덮어써야 하면 최종 ReticleState 교체
ApplyReticleState 호출
ApplyFireFeedbackViewData 호출
UpdateReticleVisibility 호출
```

의사 코드:

```cpp
const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();
bCachedCanFire = LocalAimState.bLocalCanFire;

const ECFVehicleReticleState BaseReticleState = VehicleAimComp->GetReticleState();
CachedFireFeedbackViewData = VehiclePawnRef->BuildFireFeedbackViewData();

const ECFVehicleReticleState FinalReticleState = ResolveReticleStateFromFireFeedback(CachedFireFeedbackViewData, BaseReticleState);

ApplyReticleState(FinalReticleState);
ApplyFireFeedbackViewData(CachedFireFeedbackViewData);
UpdateReticleVisibility();
```

---

## 15. FireFeedback -> ReticleState 변환 기준

함수:

```cpp
ECFVehicleReticleState UCFAimReticleWidget::ResolveReticleStateFromFireFeedback(
    const FCFVehicleFireFeedbackViewData& InViewData,
    ECFVehicleReticleState BaseReticleState) const;
```

변환 기준:

| FireFeedbackState | ReticleState | 비고 |
| --- | --- | --- |
| `None` | BaseReticleState | 덮어쓰기 없음 |
| `FireSuccess` | BaseReticleState | 성공 플래시는 ReticleState를 유지 |
| `FirePending` | `FirePending` | 아주 짧게 표시 가능 |
| `FireRejected` | `FireRejected` | 발사 조건 미충족 |
| `Cooldown` | `Cooldown` | 쿨다운 우선 표시 |
| `NoWeapon` | `NoWeapon` | 무기 없음 |
| `AimBlocked` | `Blocked` | 조준 막힘 |
| `OutOfArcWarning` | BaseReticleState | 주 상태 덮어쓰기 없음 |

의사 코드:

```cpp
if (!InViewData.bFeedbackActive || !InViewData.bOverrideReticleState)
{
    return BaseReticleState;
}

switch (InViewData.FeedbackState)
{
case ECFVehicleFireFeedbackState::FirePending:
    return ECFVehicleReticleState::FirePending;
case ECFVehicleFireFeedbackState::FireRejected:
    return ECFVehicleReticleState::FireRejected;
case ECFVehicleFireFeedbackState::Cooldown:
    return ECFVehicleReticleState::Cooldown;
case ECFVehicleFireFeedbackState::NoWeapon:
    return ECFVehicleReticleState::NoWeapon;
case ECFVehicleFireFeedbackState::AimBlocked:
    return ECFVehicleReticleState::Blocked;
case ECFVehicleFireFeedbackState::OutOfArcWarning:
case ECFVehicleFireFeedbackState::FireSuccess:
case ECFVehicleFireFeedbackState::None:
default:
    return BaseReticleState;
}
```

---

## 16. FireFeedback 텍스트 표시 기준

### 16.1 State 표시

```cpp
// [v0.1] FireFeedback 상태를 한국어 표시 텍스트로 변환합니다.
FText UCFAimReticleWidget::GetFireFeedbackStateDisplayText(ECFVehicleFireFeedbackState InFeedbackState) const
{
    switch (InFeedbackState)
    {
    case ECFVehicleFireFeedbackState::FireSuccess:
        return FText::FromString(TEXT("발사"));
    case ECFVehicleFireFeedbackState::FirePending:
        return FText::FromString(TEXT("발사 처리 중"));
    case ECFVehicleFireFeedbackState::FireRejected:
        return FText::FromString(TEXT("발사 불가"));
    case ECFVehicleFireFeedbackState::Cooldown:
        return FText::FromString(TEXT("재사용 대기"));
    case ECFVehicleFireFeedbackState::NoWeapon:
        return FText::FromString(TEXT("무기 없음"));
    case ECFVehicleFireFeedbackState::AimBlocked:
        return FText::FromString(TEXT("조준 가림"));
    case ECFVehicleFireFeedbackState::OutOfArcWarning:
        return FText::FromString(TEXT("각도 경고"));
    case ECFVehicleFireFeedbackState::None:
    default:
        return FText::GetEmpty();
    }
}
```

### 16.2 Hint 표시

```text
FireSuccess       -> 발사 요청 수락
FirePending       -> 로컬 발사 처리 중
FireRejected      -> 발사 조건 미충족
Cooldown          -> 무기 재사용 대기 중
NoWeapon          -> 사용 가능한 무기 없음
AimBlocked        -> 조준선이 막힘
OutOfArcWarning   -> 조준각 경고
None              -> Empty
```

### 16.3 Cooldown 표시

`Text_Cooldown` 표시 후보:

```text
1.23초
```

표시 기준:

```text
- bShowCooldown == true일 때만 표시
- RemainingCooldownSeconds를 소수점 둘째 자리 또는 첫째 자리로 표시
- P0에서는 게이지보다 숫자 우선
```

---

## 17. WBP_AimReticle 구성 계획

작업 대상:

```text
WBP_AimReticle 또는 현재 AimReticleWidgetClass로 지정된 Widget Blueprint
```

정확한 자산명은 구현 전 에디터에서 확인해야 한다.

### 17.1 최소 위젯 계층 후보

```text
CanvasPanel_Root
  Overlay_ReticleRoot
    Image_LeftBracket
    Image_RightBracket
    Image_CenterDot
    Image_StateIcon
    Image_OutOfArcWarning
    Text_ReticleState
    Text_CanFire
    Text_ReticleHint
    Text_FireFeedbackState
    Text_FireFeedbackHint
    Text_Cooldown
```

### 17.2 Optional 위젯 원칙

모든 신규 위젯은 C++에서 `BindWidgetOptional`로 받는다.
따라서 아래가 가능하다.

```text
- WBP에 Text_Cooldown이 없어도 크래시 없음
- WBP에 Image_StateIcon이 없어도 크래시 없음
- 먼저 텍스트만 구현하고, 나중에 이미지/애니메이션 추가 가능
```

### 17.3 P0 WBP 최소 구현

P0에서 반드시 필요한 것은 아래 정도다.

```text
- 중앙 점
- 좌우 브라켓
- Text_FireFeedbackState
- Text_Cooldown
```

나머지는 후속으로 미룰 수 있다.

---

## 18. BP 애니메이션 계획

P0에서 선택적으로 만들 수 있는 Animation 후보:

```text
Anim_FireSuccessFlash
Anim_FireRejectedShake
Anim_OutOfArcWarningPulse
```

단, P0 필수는 아니다.
텍스트/색상만으로 먼저 검증하고, 이후 애니메이션을 붙이는 순서를 추천한다.

애니메이션 연결 방식 후보:

```text
C++에서 FeedbackState 변경 감지
-> BP Implementable Event 호출
-> WBP에서 해당 Animation Play
```

이번 P0에서 더 단순하게 가려면:

```text
C++는 텍스트만 갱신
BP 애니메이션은 후속 작업
```

---

## 19. BlueprintImplementableEvent 추가 여부

P0에서는 필수가 아니다.
다만 애니메이션을 바로 넣고 싶다면 `UCFAimReticleWidget`에 아래 이벤트를 추가할 수 있다.

```cpp
// [v0.1] FireFeedback 상태가 갱신되었을 때 BP 애니메이션 재생을 위해 호출됩니다.
UFUNCTION(BlueprintImplementableEvent, Category="CarFight|AimReticle")
void BP_OnFireFeedbackStateChanged(ECFVehicleFireFeedbackState NewFeedbackState);
```

주의:

```text
- 상태가 매 Tick마다 같아도 계속 호출되면 애니메이션이 반복 재시작될 수 있다.
- 호출하려면 PreviousFeedbackState 캐시가 필요하다.
- 그래서 P0에서는 우선 텍스트 표시만 구현하고, 애니메이션 이벤트는 후속으로 미루는 것이 안전하다.
```

---

## 20. 구현 순서

### Step 1. 타입 파일 추가

```text
신규:
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

내용:

```text
- ECFVehicleFireFeedbackState
- FCFVehicleFireFeedbackViewData
```

### Step 2. Pawn ViewData 함수 추가

```text
수정:
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

내용:

```text
- LastFireFeedbackStartTimeSeconds
- FireSuccessFeedbackDurationSeconds
- FireRejectedFeedbackDurationSeconds
- BuildFireFeedbackViewData()
- ApplyFireResult()에서 피드백 시작 시간 기록
```

### Step 3. WeaponComp 기존 Getter 사용

```text
확인 완료:
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
```

내용:

```text
- GetActiveWeaponCooldownSeconds() 사용
- GetRemainingCooldownSeconds(float CurrentTimeSeconds) 사용
- 신규 쿨다운 Getter 추가 불필요
```

### Step 4. Reticle Widget 확장

```text
수정:
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

내용:

```text
- FireFeedback Optional TextBlock 추가
- CachedFireFeedbackViewData 추가
- RefreshFromPawn() 확장
- ApplyFireFeedbackViewData() 추가
- ResolveReticleStateFromFireFeedback() 추가
```

### Step 5. WBP 최소 연결

```text
수정:
현재 AimReticleWidgetClass로 지정된 WBP
```

내용:

```text
- Text_FireFeedbackState
- Text_Cooldown
- 필요 시 Text_FireFeedbackHint
```

### Step 6. PIE 검증

검증 항목:

```text
- 발사 성공 시 FireSuccess 짧은 표시
- 연속 발사 시 Cooldown 표시
- 무기 데이터 제거 시 NoWeapon 표시
- 조준 막힘 시 AimBlocked 표시
- OutOfArcWarning은 주 상태를 무조건 덮어쓰지 않음
```

---

## 21. 테스트 기준

관련 테스트 항목:

```text
- CF-TC-006 조준 Reticle
- CF-TC-013 차량 무기 조준/발사
- CF-TC-014 발사 피드백/UI
```

PASS 기준:

```text
- 싱글 PIE에서 Reticle이 정상 표시된다.
- 발사 성공 / 발사 불가 / 쿨다운 / 무기 없음이 구분된다.
- WeaponFire 판정 결과와 Reticle 표시가 충돌하지 않는다.
- OutOfArcWarning이 발사 차단 상태처럼 표시되지 않는다.
- 서버 대기 / 서버 거부 표현이 UI에 나오지 않는다.
```

---

## 22. 리스크와 대응

| 리스크 | 원인 | 대응 |
| --- | --- | --- |
| Reticle이 계속 Cooldown으로 고정됨 | RemainingCooldown 계산이 0으로 복귀하지 않음 | WeaponComp 쿨다운 Getter 검증 |
| FireSuccess가 보이지 않음 | 표시 시간이 너무 짧거나 LastFireFeedbackStartTimeSeconds 미갱신 | 0.12초에서 0.2초로 임시 조정 |
| FireRejected가 매 Tick 반복 표시됨 | 이전 결과를 계속 최근 결과처럼 해석 | FeedbackAgeSeconds로 표시 시간 제한 |
| OutOfArc가 발사 불가처럼 보임 | 주 상태를 OutOfArc로 덮어씀 | OutOfArcWarning은 보조 표시로만 처리 |
| WBP가 크래시 | BindWidget 필수 위젯 누락 | 신규 위젯은 BindWidgetOptional 사용 |
| BP 애니메이션이 계속 재시작됨 | 매 Tick 이벤트 호출 | P0에서는 이벤트 생략 또는 PreviousState 캐시 |

---

## 23. 문서 반영 결과

현재 구현 결과를 반영한 Systems / SSOT 문서:

```text
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/Combat/WeaponFire.md
- Document/ProjectSSOT/05_TestChecklist.md
```

현재 반영한 내용:

```text
- 실제 추가된 C++ 타입 / 함수 / 변수
- 실제 WBP 위젯명
- 실제 표시 우선순위
- 실제 테스트 결과
- 미구현으로 남긴 항목
```

---

## 24. 관련 문서

```text
- Document/Plan/ReticleFireFeedback/VisualGuide.md
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/UI/VehicleDebugPanel.md
- Document/ProjectSSOT/03_FeatureQueue.md
- Document/ProjectSSOT/05_TestChecklist.md
```

---

## 25. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- FireFeedback 전담 컴포넌트를 만들기로 결정할 때
- CFVehicleFireFeedbackTypes.h 파일명이 변경될 때
- BuildFireFeedbackViewData() 위치가 Pawn이 아닌 다른 클래스로 이동할 때
- UCFAimReticleWidget이 아닌 별도 HUD 위젯에서 FireFeedback을 표시할 때
- WBP_AimReticle 구조가 확정될 때
- BP 애니메이션 이벤트를 P0에 포함하기로 결정할 때
- Cooldown 표시 방식이 숫자에서 게이지로 변경될 때
```

---

## 26. Migration

### v1.1.0 -> v1.2.0

```text
- NoWeapon와 AimBlocked 최종 사용자 PIE 결과를 완료 기준으로 반영했다.
- Ready → FireSuccess → Cooldown → Ready 정상 발사 회귀를 확인했다.
- CF-FQ-017을 Done, CF-TC-014를 PASS로 전환했다.
- 현재 구현 판단은 Systems/Combat/FireFeedback.md와 Systems/UI/AimReticle.md를 우선한다.
- 코드와 WBP 자산은 변경하지 않았다.
```

### v1.0.0 -> v1.1.0

```text
- 문서 상단에 다중 작업 세션 복원용 현재 작업 체크포인트를 추가했다.
- 기존 Reticle, FireFeedback C++·WBP 동작과 자산은 변경하지 않았다.
- 새 세션은 NoWeapon, AimBlocked와 CF-FQ-022 회귀 검증 Pending 상태부터 재개한다.
```

### v0.1.1 -> v1.0.0

```text
- 이 문서는 사전 설계 전용 문서에서 구현 이력과 남은 검증을 함께 보존하는 Plan 문서로 전환한다.
- 실제 현재 구현 기준은 Document/Systems/UI/AimReticle.md와 Document/Systems/Combat/FireFeedback.md를 우선한다.
- 기존 의사 코드와 설계안은 구현 당시 계획 이력으로 유지하며, 현재 동작 판단에는 Systems 문서를 사용한다.
- NoWeapon / AimBlocked 검증 완료 후 이 Plan은 Archive 대상으로 전환한다.
- 코드 또는 데이터 마이그레이션은 필요하지 않다.
```

---

## 27. Changelog

### v1.2.0 - 2026-07-15

```text
- NoWeapon 회색 표시, AimBlocked 주황 표시와 두 상태의 텍스트 만료 사용자 PIE PASS를 기록했다.
- 장애물 제거 후 정상 Aim 복귀와 Ready → FireSuccess → Cooldown → Ready 회귀를 확인했다.
- CF-FQ-017 Done / CF-TC-014 PASS로 전환했다.
- Plan을 완료 상태와 Archive 검토 대상으로 변경했다.
```

### v1.1.0 - 2026-07-14

```text
- 다중 작업 세션 복원을 위한 표준 현재 작업 체크포인트 추가
- 구현 및 주요 PIE 완료, NoWeapon·AimBlocked 최종 검증 Pending 상태를 상단에서 정리
- 다음 작업, 관련 코드·문서 경로와 보호 범위를 명시
```

### v1.0.0 - 2026-07-13

```text
- 문서 상태를 사전 설계 Draft에서 구현 완료/최종 상태 검증 대기로 변경
- 실제 구현된 타입, ViewData, WBP 바인딩, 상태별 색상, 표시 우선순위 결과 요약 추가
- WeaponCooldown / OutOfArcWarning 잔류 및 중복 표시 수정 결과 반영
- 확인 완료된 PIE 항목과 NoWeapon / AimBlocked 남은 검증 항목 분리
```

### v0.1.1 - 2026-07-09

```text
- 코드 확인 결과에 맞춰 남은 쿨다운 Getter명을 GetRemainingCooldownSeconds(float CurrentTimeSeconds) 기준으로 정정
- UCFVehicleWeaponComp에 신규 쿨다운 Getter를 추가하지 않고 기존 Getter를 사용한다고 명시
- 구현 순서 Step 3을 WeaponComp 기존 Getter 사용 기준으로 정리
```

### v0.1.0 - 2026-07-09

```text
- Reticle / FireFeedback ImplementationDesign 신규 작성
- CFVehicleFireFeedbackTypes.h 신규 타입 설계 추가
- ECFVehicleFireFeedbackState / FCFVehicleFireFeedbackViewData 설계 추가
- ACFVehiclePawn::BuildFireFeedbackViewData() 설계 추가
- UCFAimReticleWidget FireFeedback 확장 설계 추가
- WBP_AimReticle Optional 위젯 구성 후보 추가
- 구현 순서, 테스트 기준, 리스크 대응, 구현 완료 후 문서 반영 계획 추가
```
