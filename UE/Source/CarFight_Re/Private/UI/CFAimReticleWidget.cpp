// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.1
// Date: 2026-07-13
// Description: Aim Reticle UI용 C++ 부모 위젯 클래스 구현입니다.
// Changelog:
// - v1.5.1: 전용 OutOfArc 경고가 실제 OutOfArcWarning 피드백일 때만 일반 FireFeedback 텍스트를 대체하도록 조건을 제한.
// - v1.5.0: Text_OutOfArcWarning이 바인딩된 경우 OutOfArc 경고를 전용 텍스트에만 표시하고 일반 FireFeedback 텍스트 중복을 방지.
// - v1.4.0: 선택적 Reticle 이미지와 FireFeedback TextBlock에 상태별 색상을 안전하게 적용.
// - v1.3.0: Pawn FireFeedback ViewData를 Reticle 최종 상태와 선택적 피드백 TextBlock에 통합.
// - v1.2.0: Reticle enum 값 이름을 FirePending / FireRejected 싱글플레이 명칭으로 교체.
// - v1.1.0: 싱글플레이 전환에 맞춰 서버 대기/거부 표시 문구를 로컬 발사 처리/거부 문구로 변경.
// Migration:
// - ECFVehicleReticleState::WaitingServer는 FirePending으로, ServerRejected는 FireRejected로 교체한다.
// - WBP에서 색상 적용이 필요하면 신규 Optional 이미지 5개와 Text_OutOfArcWarning의 Is Variable을 활성화한다.
// Scope: VehicleAimComp의 Reticle 상태를 읽어 선택적 TextBlock과 위젯 가시성을 갱신합니다.

#include "UI/CFAimReticleWidget.h"

#include "CFVehicleAimComp.h"
#include "CFVehiclePawn.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

// [v1.0.0] Reticle이 읽을 차량 Pawn 참조를 설정하고 즉시 갱신합니다.
void UCFAimReticleWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
	// [v1.0.0] Reticle이 읽을 차량 Pawn 참조입니다.
	VehiclePawnRef = InVehiclePawnRef;

	RefreshFromPawn();
	UpdateReticleVisibility();
}

// [v1.0.0] 현재 Pawn의 VehicleAimComp에서 최신 Reticle 상태를 읽어 갱신합니다.
void UCFAimReticleWidget::RefreshFromPawn()
{
	// [v1.0.0] 유효한 Pawn이 없을 때 적용할 안전한 fallback 상태입니다.
	const ECFVehicleReticleState FallbackReticleState = ECFVehicleReticleState::Hidden;

	if (!IsValid(VehiclePawnRef))
	{
		bCachedCanFire = false;
		ApplyFireFeedbackViewData(FCFVehicleFireFeedbackViewData());
		ApplyReticleState(FallbackReticleState);
		UpdateReticleVisibility();
		return;
	}

	// [v1.0.0] 현재 Pawn에서 읽은 Aim 컴포넌트입니다.
	const UCFVehicleAimComp* VehicleAimComp = VehiclePawnRef->GetVehicleAimComp();
	if (!IsValid(VehicleAimComp))
	{
		bCachedCanFire = false;
		ApplyFireFeedbackViewData(FCFVehicleFireFeedbackViewData());
		ApplyReticleState(FallbackReticleState);
		UpdateReticleVisibility();
		return;
	}

	// [v1.0.0] Reticle 표시와 발사 가능 표시를 계산할 최신 Local Aim 상태입니다.
	const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();
	bCachedCanFire = LocalAimState.bLocalCanFire;

	// [v1.3.0] VehicleAimComp가 계산한 기본 Reticle 상태입니다.
	const ECFVehicleReticleState BaseReticleState = VehicleAimComp->GetReticleState();

	ApplyFireFeedbackViewData(VehiclePawnRef->BuildFireFeedbackViewData());

	// [v1.3.0] FireFeedback 오버레이 정책이 반영된 최종 Reticle 상태입니다.
	const ECFVehicleReticleState FinalReticleState = ResolveReticleStateFromFireFeedback(CachedFireFeedbackViewData, BaseReticleState);

	ApplyReticleState(FinalReticleState);
	UpdateReticleVisibility();
}

// [v1.0.0] 전달받은 Reticle 상태를 캐시와 선택적 텍스트 위젯에 반영합니다.
void UCFAimReticleWidget::ApplyReticleState(const ECFVehicleReticleState InReticleState)
{
	CachedReticleState = InReticleState;
	RefreshTextBlocks();
}

// [v1.0.0] 현재 Reticle 상태에 따라 위젯 가시성을 갱신합니다.
void UCFAimReticleWidget::UpdateReticleVisibility()
{
	// [v1.0.0] Reticle이 화면에 표시되어야 하는지 여부입니다.
	const bool bShouldShowReticle = CachedReticleState != ECFVehicleReticleState::Hidden;

	SetVisibility(bShouldShowReticle ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

// [v1.0.0] 현재 Reticle 상태를 UI 표시용 텍스트로 반환합니다.
FText UCFAimReticleWidget::GetReticleStateDisplayText() const
{
	switch (CachedReticleState)
	{
	case ECFVehicleReticleState::Ready:
		return FText::FromString(TEXT("조준: 준비"));
	case ECFVehicleReticleState::Blocked:
		return FText::FromString(TEXT("조준: 가림"));
	case ECFVehicleReticleState::OutOfArc:
		return FText::FromString(TEXT("조준: 각도 밖"));
	case ECFVehicleReticleState::NoWeapon:
		return FText::FromString(TEXT("조준: 무기 없음"));
	case ECFVehicleReticleState::Cooldown:
		return FText::FromString(TEXT("조준: 재사용 대기"));
	case ECFVehicleReticleState::Reloading:
		return FText::FromString(TEXT("조준: 재장전"));
	case ECFVehicleReticleState::FirePending:
		return FText::FromString(TEXT("조준: 발사 처리 중"));
	case ECFVehicleReticleState::FireRejected:
		return FText::FromString(TEXT("조준: 발사 거부"));
	case ECFVehicleReticleState::Hidden:
	default:
		return FText::FromString(TEXT("조준: 숨김"));
	}
}

// [v1.0.0] 현재 발사 가능 캐시를 UI 표시용 텍스트로 반환합니다.
FText UCFAimReticleWidget::GetCanFireDisplayText() const
{
	return bCachedCanFire ? FText::FromString(TEXT("발사 가능")) : FText::FromString(TEXT("발사 불가"));
}

// [v1.0.0] 위젯 생성 직후 현재 참조 기준으로 첫 Reticle 갱신을 수행합니다.
void UCFAimReticleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshFromPawn();
	UpdateReticleVisibility();
}

// [v1.0.0] 옵션이 켜져 있으면 매 프레임 Pawn 기준 Reticle 갱신을 수행합니다.
void UCFAimReticleWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// [v1.0.0] 자동 갱신이 꺼져 있으면 매 프레임 갱신을 건너뜁니다.
	if (!bAutoRefreshEveryTick)
	{
		return;
	}

	RefreshFromPawn();
	UpdateReticleVisibility();
}

// [v1.3.0] FireFeedback 표시 데이터를 캐시에 저장하고 선택적 TextBlock에 반영합니다.
void UCFAimReticleWidget::ApplyFireFeedbackViewData(const FCFVehicleFireFeedbackViewData& InViewData)
{
	CachedFireFeedbackViewData = InViewData;
	RefreshTextBlocks();
}

// [v1.0.0] 현재 캐시값을 선택적 TextBlock들에 반영합니다.
void UCFAimReticleWidget::RefreshTextBlocks()
{
	// [v1.5.1] 전용 OutOfArc 경고 TextBlock이 일반 FireFeedback 텍스트를 대체할 수 있는지 여부입니다.
	const bool bUseDedicatedOutOfArcWarning = IsValid(Text_OutOfArcWarning)
		&& CachedFireFeedbackViewData.bFeedbackActive
		&& CachedFireFeedbackViewData.FeedbackState == ECFVehicleFireFeedbackState::OutOfArcWarning
		&& CachedFireFeedbackViewData.bShowOutOfArcWarning;

	// [v1.5.0] 일반 FireFeedback State/Hint TextBlock에 현재 피드백 문구를 표시할지 여부입니다.
	const bool bShowGeneralFireFeedbackText = CachedFireFeedbackViewData.bFeedbackActive
		&& !bUseDedicatedOutOfArcWarning;

	if (Text_ReticleState)
	{
		Text_ReticleState->SetText(GetReticleStateDisplayText());
	}

	if (Text_CanFire)
	{
		Text_CanFire->SetText(GetCanFireDisplayText());
	}

	if (Text_ReticleHint)
	{
		Text_ReticleHint->SetText(GetReticleHintDisplayText());
	}

	if (Text_FireFeedbackState)
	{
		Text_FireFeedbackState->SetText(
			bShowGeneralFireFeedbackText
				? GetFireFeedbackStateDisplayText(CachedFireFeedbackViewData.FeedbackState)
				: FText::GetEmpty());
	}

	if (Text_FireFeedbackHint)
	{
		Text_FireFeedbackHint->SetText(
			bShowGeneralFireFeedbackText
				? GetFireFeedbackHintDisplayText(CachedFireFeedbackViewData.FeedbackState)
				: FText::GetEmpty());
	}

	if (Text_Cooldown)
	{
		Text_Cooldown->SetText(
			CachedFireFeedbackViewData.bShowCooldown
				? FText::FromString(FString::Printf(TEXT("%.2f초"), CachedFireFeedbackViewData.RemainingCooldownSeconds))
				: FText::GetEmpty());
	}

	RefreshVisualStyle();
}

// [v1.4.0] 현재 캐시값에 맞는 이미지와 피드백 텍스트 색상을 안전하게 갱신합니다.
void UCFAimReticleWidget::RefreshVisualStyle()
{
	// [v1.4.0] 모든 주 Reticle 이미지에 공통으로 적용할 상태 색상입니다.
	const FLinearColor MainReticleColor = GetMainReticleColor();

	// [v1.4.0] 활성 발사 피드백 텍스트에 적용할 상태 색상입니다.
	const FLinearColor FireFeedbackTextColor = GetFireFeedbackTextColor();

	// [v1.4.0] 주 Reticle 이미지를 같은 색상으로 갱신하기 위한 선택적 이미지 목록입니다.
	const TObjectPtr<UImage> MainReticleImages[] = {
		Image_CenterDot,
		Image_LeftBracket,
		Image_RightBracket,
		Image_TopBracket,
		Image_BottomBracket
	};

	for (UImage* MainReticleImage : MainReticleImages)
	{
		if (IsValid(MainReticleImage))
		{
			MainReticleImage->SetColorAndOpacity(MainReticleColor);
		}
	}

	if (Text_FireFeedbackState)
	{
		Text_FireFeedbackState->SetColorAndOpacity(FireFeedbackTextColor);
	}

	if (Text_FireFeedbackHint)
	{
		Text_FireFeedbackHint->SetColorAndOpacity(FireFeedbackTextColor);
	}

	if (Text_Cooldown)
	{
		Text_Cooldown->SetColorAndOpacity(CooldownReticleColor);
	}

	if (Text_OutOfArcWarning)
	{
		Text_OutOfArcWarning->SetText(
			CachedFireFeedbackViewData.bShowOutOfArcWarning
				? FText::FromString(TEXT("각도 경고"))
				: FText::GetEmpty());
		Text_OutOfArcWarning->SetColorAndOpacity(OutOfArcWarningColor);
	}
}

// [v1.4.0] 현재 Reticle 상태와 활성 FireFeedback 기준의 주 Reticle 색상을 반환합니다.
FLinearColor UCFAimReticleWidget::GetMainReticleColor() const
{
	if (CachedFireFeedbackViewData.bFeedbackActive
		&& CachedFireFeedbackViewData.FeedbackState == ECFVehicleFireFeedbackState::FireSuccess)
	{
		return FireSuccessReticleColor;
	}

	return GetReticleStateColor(CachedReticleState);
}

// [v1.4.0] 현재 활성 FireFeedback 기준의 피드백 텍스트 색상을 반환합니다.
FLinearColor UCFAimReticleWidget::GetFireFeedbackTextColor() const
{
	if (!CachedFireFeedbackViewData.bFeedbackActive)
	{
		return GetReticleStateColor(CachedReticleState);
	}

	switch (CachedFireFeedbackViewData.FeedbackState)
	{
	case ECFVehicleFireFeedbackState::FireSuccess:
		return FireSuccessReticleColor;
	case ECFVehicleFireFeedbackState::FirePending:
		return FirePendingReticleColor;
	case ECFVehicleFireFeedbackState::FireRejected:
		return FireRejectedReticleColor;
	case ECFVehicleFireFeedbackState::Cooldown:
		return CooldownReticleColor;
	case ECFVehicleFireFeedbackState::NoWeapon:
		return NoWeaponReticleColor;
	case ECFVehicleFireFeedbackState::AimBlocked:
		return AimBlockedReticleColor;
	case ECFVehicleFireFeedbackState::OutOfArcWarning:
		return OutOfArcWarningColor;
	case ECFVehicleFireFeedbackState::None:
	default:
		return GetReticleStateColor(CachedReticleState);
	}
}

// [v1.4.0] 전달된 Reticle 상태에 대응하는 기본 색상을 반환합니다.
FLinearColor UCFAimReticleWidget::GetReticleStateColor(const ECFVehicleReticleState InReticleState) const
{
	switch (InReticleState)
	{
	case ECFVehicleReticleState::Blocked:
		return AimBlockedReticleColor;
	case ECFVehicleReticleState::OutOfArc:
		return ReadyReticleColor;
	case ECFVehicleReticleState::NoWeapon:
		return NoWeaponReticleColor;
	case ECFVehicleReticleState::Cooldown:
		return CooldownReticleColor;
	case ECFVehicleReticleState::Reloading:
		return ReloadingReticleColor;
	case ECFVehicleReticleState::FirePending:
		return FirePendingReticleColor;
	case ECFVehicleReticleState::FireRejected:
		return FireRejectedReticleColor;
	case ECFVehicleReticleState::Ready:
	case ECFVehicleReticleState::Hidden:
	default:
		return ReadyReticleColor;
	}
}

// [v1.0.0] Reticle 상태별 보조 설명 텍스트를 반환합니다.
FText UCFAimReticleWidget::GetReticleHintDisplayText() const
{
	switch (CachedReticleState)
	{
	case ECFVehicleReticleState::Ready:
		return FText::FromString(TEXT("목표 조준 가능"));
	case ECFVehicleReticleState::Blocked:
		return FText::FromString(TEXT("목표가 가려짐"));
	case ECFVehicleReticleState::OutOfArc:
		return FText::FromString(TEXT("무기 조준각 밖"));
	case ECFVehicleReticleState::NoWeapon:
		return FText::FromString(TEXT("사용 가능한 무기 없음"));
	case ECFVehicleReticleState::Cooldown:
		return FText::FromString(TEXT("무기 대기 중"));
	case ECFVehicleReticleState::Reloading:
		return FText::FromString(TEXT("재장전 중"));
	case ECFVehicleReticleState::FirePending:
		return FText::FromString(TEXT("발사 처리 대기"));
	case ECFVehicleReticleState::FireRejected:
		return FText::FromString(TEXT("발사 조건 미충족"));
	case ECFVehicleReticleState::Hidden:
	default:
		return FText::FromString(TEXT("Reticle 숨김"));
	}
}

// [v1.3.0] FireFeedback 상태를 UI 표시용 한국어 텍스트로 변환합니다.
FText UCFAimReticleWidget::GetFireFeedbackStateDisplayText(const ECFVehicleFireFeedbackState InFeedbackState) const
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

// [v1.3.0] FireFeedback 상태를 UI 보조 설명 텍스트로 변환합니다.
FText UCFAimReticleWidget::GetFireFeedbackHintDisplayText(const ECFVehicleFireFeedbackState InFeedbackState) const
{
	switch (InFeedbackState)
	{
	case ECFVehicleFireFeedbackState::FireSuccess:
		return FText::FromString(TEXT("발사 요청 수락"));
	case ECFVehicleFireFeedbackState::FirePending:
		return FText::FromString(TEXT("로컬 발사 처리 중"));
	case ECFVehicleFireFeedbackState::FireRejected:
		return FText::FromString(TEXT("발사 조건 미충족"));
	case ECFVehicleFireFeedbackState::Cooldown:
		return FText::FromString(TEXT("무기 재사용 대기 중"));
	case ECFVehicleFireFeedbackState::NoWeapon:
		return FText::FromString(TEXT("사용 가능한 무기 없음"));
	case ECFVehicleFireFeedbackState::AimBlocked:
		return FText::FromString(TEXT("조준선이 막힘"));
	case ECFVehicleFireFeedbackState::OutOfArcWarning:
		return FText::FromString(TEXT("조준각 경고"));
	case ECFVehicleFireFeedbackState::None:
	default:
		return FText::GetEmpty();
	}
}

// [v1.3.0] 기본 Reticle 상태와 FireFeedback 표시 데이터를 합쳐 최종 Reticle 상태를 반환합니다.
ECFVehicleReticleState UCFAimReticleWidget::ResolveReticleStateFromFireFeedback(
	const FCFVehicleFireFeedbackViewData& InViewData,
	const ECFVehicleReticleState BaseReticleState) const
{
	if (!InViewData.bFeedbackActive)
	{
		return BaseReticleState;
	}

	if (!InViewData.bOverrideReticleState)
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
	case ECFVehicleFireFeedbackState::FireSuccess:
	case ECFVehicleFireFeedbackState::OutOfArcWarning:
	case ECFVehicleFireFeedbackState::None:
	default:
		return BaseReticleState;
	}
}
