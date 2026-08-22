// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.0
// Date: 2026-08-22
// Description: Aim Reticle UI용 C++ 부모 위젯 클래스 / post-closure per-frame refresh 중복 제거입니다.
// Changelog:
// - v1.10.0: 자동 Pawn Refresh 경로에서 FireFeedback/Reticle setter를 연속 호출하던 중복 Text/Style 갱신과 외부 Visibility 재호출을 제거. Cache를 한 번에 갱신한 뒤 Text/Style/Visibility를 각각 한 번 적용하며 공개 Apply API 의미는 유지.
// - v1.9.0: UI-P0-04 Weak Pawn Binding을 적용하고 Refresh 시점마다 약한 참조를 안전하게 해석해 Old Pawn lifetime 비소유를 보장.
// - v1.8.0: Image_WeaponReticle을 탄종별 Weapon Preview가 아닌 CurrentMuzzleDirection 기반 터렛 레티클 지점에 투영.
// - v1.7.1: Weapon Reticle Canvas Slot의 앵커를 좌측 상단으로 고정해 뷰포트 좌표가 중복 오프셋되지 않도록 수정.
// - v1.7.0: Weapon Preview 월드 위치를 화면 좌표로 투영해 선택적 Weapon Reticle 이미지를 표시.
// - v1.6.1: TurretAligning Reticle 상태와 Tick 유지형 Hidden 표시를 추가.
// - v1.6.0: TurretAligning FireFeedback DisplayKey를 정렬 중 문구와 amber 색상으로 표시.
// - v1.5.1: 전용 OutOfArc 경고가 실제 OutOfArcWarning 피드백일 때만 일반 FireFeedback 텍스트를 대체하도록 조건을 제한.
// - v1.5.0: Text_OutOfArcWarning이 바인딩된 경우 OutOfArc 경고를 전용 텍스트에만 표시하고 일반 FireFeedback 텍스트 중복을 방지.
// - v1.4.0: 선택적 Reticle 이미지와 FireFeedback TextBlock에 상태별 색상을 안전하게 적용.
// - v1.3.0: Pawn FireFeedback ViewData를 Reticle 최종 상태와 선택적 피드백 TextBlock에 통합.
// - v1.2.0: Reticle enum 값 이름을 FirePending / FireRejected 싱글플레이 명칭으로 교체.
// - v1.1.0: 싱글플레이 전환에 맞춰 서버 대기/거부 표시 문구를 로컬 발사 처리/거부 문구로 변경.
// Migration:
// - v1.10.0부터 bAutoRefreshEveryTick 경로는 Cache를 직접 일괄 갱신해 RefreshTextBlocks/RefreshVisualStyle/UpdateReticleVisibility를 각각 한 번만 호출합니다. 외부 Blueprint/C++ ApplyReticleState/ApplyFireFeedbackViewData 호출 계약은 그대로 유지합니다.
// - v1.9.0부터 Pawn이 소멸하거나 Rebind에서 해제되면 VehiclePawnRef.Get()이 Null이 되고 Reticle은 Hidden fallback으로 전환한다.
// - Image_WeaponReticle은 bHasValidTurretReticlePoint와 TurretReticleWorldLocation만 소비하며 Legacy Weapon Preview 모드와 착탄 정보에는 의존하지 않는다.
// - Image_WeaponReticle의 디자이너 앵커 값과 관계없이 런타임에는 좌측 상단 앵커를 사용한다.
// - Image_WeaponReticle은 BindWidgetOptional로 사용하며 누락 시 기존 Command Reticle과 FireFeedback에는 영향이 없다.
// - 상태 기반 Hidden은 Root Visibility를 바꾸지 않고 RenderOpacity 0으로만 표현한다.
// - TurretAligning은 ReticleState enum 상태와 기존 FireFeedback DisplayKey 보조 표시를 함께 지원한다.
// - ECFVehicleReticleState::WaitingServer는 FirePending으로, ServerRejected는 FireRejected로 교체한다.
// - WBP에서 색상 적용이 필요하면 신규 Optional 이미지 5개와 Text_OutOfArcWarning의 Is Variable을 활성화한다.
// Scope: VehicleAimComp의 Reticle 상태를 읽어 선택적 TextBlock과 위젯 가시성을 갱신합니다.

#include "UI/CFAimReticleWidget.h"

#include "CFVehicleAimComp.h"
#include "CFVehiclePawn.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

// [v1.10.0] Reticle이 읽을 차량 Pawn 참조를 설정하고 단일 통합 Refresh로 즉시 갱신합니다.
void UCFAimReticleWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
	// [v1.0.0] Reticle이 읽을 차량 Pawn 참조입니다.
	VehiclePawnRef = InVehiclePawnRef;

	RefreshFromPawn();
}

// [v1.10.0] 현재 Pawn의 VehicleAimComp에서 최신 Reticle/FireFeedback Cache를 한 번에 갱신하고 Visual Refresh를 한 번씩 수행합니다.
void UCFAimReticleWidget::RefreshFromPawn()
{
	// [v1.0.0] 유효한 Pawn이 없을 때 적용할 안전한 fallback 상태입니다.
	const ECFVehicleReticleState FallbackReticleState = ECFVehicleReticleState::Hidden;

	// [v1.9.0] Weak Pawn Reference에서 이번 갱신 동안만 사용할 안전한 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	if (!IsValid(CurrentVehiclePawn))
	{
		bCachedCanFire = false;
		CachedFireFeedbackViewData = FCFVehicleFireFeedbackViewData();
		CachedReticleState = FallbackReticleState;
		HideWeaponReticle();
		RefreshTextBlocks();
		UpdateReticleVisibility();
		return;
	}

	// [v1.0.0] 현재 Pawn에서 읽은 Aim 컴포넌트입니다.
	const UCFVehicleAimComp* VehicleAimComp = CurrentVehiclePawn->GetVehicleAimComp();
	if (!IsValid(VehicleAimComp))
	{
		bCachedCanFire = false;
		CachedFireFeedbackViewData = FCFVehicleFireFeedbackViewData();
		CachedReticleState = FallbackReticleState;
		HideWeaponReticle();
		RefreshTextBlocks();
		UpdateReticleVisibility();
		return;
	}

	// [v1.0.0] Reticle 표시와 발사 가능 표시를 계산할 최신 Local Aim 상태입니다.
	const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();
	bCachedCanFire = LocalAimState.bLocalCanFire;

	// [v1.7.0] Weapon Reticle 표시 위치를 계산할 최신 Weapon Aim Solution입니다.
	const FCFVehicleWeaponAimSolution WeaponAimSolution = VehicleAimComp->GetWeaponAimSolution();

	// [v1.3.0] VehicleAimComp가 계산한 기본 Reticle 상태입니다.
	const ECFVehicleReticleState BaseReticleState = VehicleAimComp->GetReticleState();

	CachedFireFeedbackViewData = CurrentVehiclePawn->BuildFireFeedbackViewData();

	// [v1.3.0] FireFeedback 오버레이 정책이 반영된 최종 Reticle 상태입니다.
	const ECFVehicleReticleState FinalReticleState = ResolveReticleStateFromFireFeedback(CachedFireFeedbackViewData, BaseReticleState);
	CachedReticleState = FinalReticleState;

	RefreshTextBlocks();
	RefreshWeaponReticle(WeaponAimSolution);
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
	// [v1.6.1] 논리적 Hidden 상태를 Tick 유지 방식으로 표현하기 위한 위젯 불투명도입니다.
	const float ReticleRenderOpacity = CachedReticleState == ECFVehicleReticleState::Hidden ? 0.0f : 1.0f;

	SetRenderOpacity(ReticleRenderOpacity);
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
	case ECFVehicleReticleState::TurretAligning:
		return FText::FromString(TEXT("조준: 터렛 정렬 중"));
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

// [v1.10.0] 위젯 생성 직후 현재 참조 기준으로 단일 통합 Reticle 갱신을 수행합니다.
void UCFAimReticleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshFromPawn();
}

// [v1.10.0] 옵션이 켜져 있으면 매 프레임 Pawn 기준 단일 통합 Reticle 갱신을 수행합니다.
void UCFAimReticleWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// [v1.0.0] 자동 갱신이 꺼져 있으면 매 프레임 갱신을 건너뜁니다.
	if (!bAutoRefreshEveryTick)
	{
		return;
	}

		
	RefreshFromPawn();
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
	// [v1.6.0] 현재 피드백이 터렛 정렬 중 표시인지 여부입니다.
	const bool bIsTurretAligningFeedback = IsTurretAligningFeedbackActive();

	// [v1.5.1] 전용 OutOfArc 경고 TextBlock이 일반 FireFeedback 텍스트를 대체할 수 있는지 여부입니다.
	const bool bUseDedicatedOutOfArcWarning = IsValid(Text_OutOfArcWarning)
		&& CachedFireFeedbackViewData.bFeedbackActive
		&& CachedFireFeedbackViewData.FeedbackState == ECFVehicleFireFeedbackState::OutOfArcWarning
		&& CachedFireFeedbackViewData.bShowOutOfArcWarning;

	// [v1.5.0] 일반 FireFeedback State/Hint TextBlock에 현재 피드백 문구를 표시할지 여부입니다.
	const bool bShowGeneralFireFeedbackText = CachedFireFeedbackViewData.bFeedbackActive
		&& !bUseDedicatedOutOfArcWarning;

	// [v1.6.0] 일반 FireFeedback State TextBlock에 표시할 최종 문구입니다.
	const FText FireFeedbackStateText = bIsTurretAligningFeedback
		? FText::FromString(TEXT("정렬 중"))
		: GetFireFeedbackStateDisplayText(CachedFireFeedbackViewData.FeedbackState);

	// [v1.6.0] 일반 FireFeedback Hint TextBlock에 표시할 최종 보조 문구입니다.
	const FText FireFeedbackHintText = bIsTurretAligningFeedback
		? FText::FromString(TEXT("터렛이 목표 방향으로 정렬 중"))
		: GetFireFeedbackHintDisplayText(CachedFireFeedbackViewData.FeedbackState);

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
				? FireFeedbackStateText
				: FText::GetEmpty());
	}

	if (Text_FireFeedbackHint)
	{
		Text_FireFeedbackHint->SetText(
			bShowGeneralFireFeedbackText
				? FireFeedbackHintText
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
		// [v1.6.0] OutOfArcWarning 전용 TextBlock이 TurretAligning 문구를 표시해야 하는지 여부입니다.
		const bool bUseTurretAligningWarningText = IsTurretAligningFeedbackActive();

		// [v1.6.0] OutOfArcWarning 전용 TextBlock에 표시할 최종 문구입니다.
		const FText OutOfArcWarningText = bUseTurretAligningWarningText
			? FText::FromString(TEXT("정렬 중"))
			: FText::FromString(TEXT("각도 경고"));

		// [v1.6.0] OutOfArcWarning 전용 TextBlock에 적용할 최종 색상입니다.
		const FLinearColor OutOfArcWarningTextColor = bUseTurretAligningWarningText
			? TurretAligningReticleColor
			: OutOfArcWarningColor;

		Text_OutOfArcWarning->SetText(
			CachedFireFeedbackViewData.bShowOutOfArcWarning
				? OutOfArcWarningText
				: FText::GetEmpty());
		Text_OutOfArcWarning->SetColorAndOpacity(OutOfArcWarningTextColor);
	}

	if (Image_WeaponReticle)
	{
		Image_WeaponReticle->SetColorAndOpacity(FLinearColor(
			WeaponReticleColor.R,
			WeaponReticleColor.G,
			WeaponReticleColor.B,
			WeaponReticleOpacity));
	}
}

// [v1.8.0] Weapon Aim Solution의 터렛 조준 월드 지점을 화면 좌표로 투영해 선택적 터렛 Reticle을 갱신합니다.
void UCFAimReticleWidget::RefreshWeaponReticle(const FCFVehicleWeaponAimSolution& InWeaponAimSolution)
{
	if (!IsValid(Image_WeaponReticle))
	{
		return;
	}

	if (!InWeaponAimSolution.bHasValidSolution
		|| !InWeaponAimSolution.bHasValidTurretReticlePoint
		|| InWeaponAimSolution.TurretReticleWorldLocation.ContainsNaN())
	{
		HideWeaponReticle();
		return;
	}

		// [v1.9.0] Weapon Reticle 투영 시점에 Weak Reference에서 안전하게 해석한 현재 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = VehiclePawnRef.Get();
	// [v1.7.0] Weapon Reticle 투영에 사용할 로컬 플레이어 컨트롤러입니다.
	APlayerController* PlayerController = IsValid(CurrentVehiclePawn) ? Cast<APlayerController>(CurrentVehiclePawn->GetController()) : nullptr;
	if (!IsValid(PlayerController))
	{
		HideWeaponReticle();
		return;
	}

	// [v1.7.0] DPI 스케일이 반영된 위젯 좌표계의 Weapon Reticle 화면 위치입니다.
	FVector2D WeaponReticleScreenPosition = FVector2D::ZeroVector;

	// [v1.8.0] 터렛 조준 월드 지점이 화면 좌표로 투영 가능한지 여부입니다.
	const bool bProjectedToScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		PlayerController,
		InWeaponAimSolution.TurretReticleWorldLocation,
		WeaponReticleScreenPosition,
		false);

	if (!bProjectedToScreen || WeaponReticleScreenPosition.ContainsNaN())
	{
		HideWeaponReticle();
		return;
	}

	// [v1.7.0] 현재 Viewport의 위젯 좌표계 크기입니다.
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();

	// [v1.7.0] Weapon Reticle이 화면 안에 들어와 있는지 여부입니다.
	const bool bWeaponReticleOnScreen =
		ViewportSize.X > 0.0f
		&& ViewportSize.Y > 0.0f
		&& WeaponReticleScreenPosition.X >= 0.0f
		&& WeaponReticleScreenPosition.Y >= 0.0f
		&& WeaponReticleScreenPosition.X <= ViewportSize.X
		&& WeaponReticleScreenPosition.Y <= ViewportSize.Y;

	if (!bWeaponReticleOnScreen)
	{
		HideWeaponReticle();
		return;
	}

	if (UCanvasPanelSlot* WeaponReticleCanvasSlot = Cast<UCanvasPanelSlot>(Image_WeaponReticle->Slot))
	{
		WeaponReticleCanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		WeaponReticleCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		WeaponReticleCanvasSlot->SetPosition(WeaponReticleScreenPosition);
	}
	else
	{
		Image_WeaponReticle->SetRenderTranslation(WeaponReticleScreenPosition);
	}

	Image_WeaponReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
	Image_WeaponReticle->SetColorAndOpacity(FLinearColor(
		WeaponReticleColor.R,
		WeaponReticleColor.G,
		WeaponReticleColor.B,
		WeaponReticleOpacity));
}

// [v1.7.0] 선택적 Weapon Reticle 이미지를 안전하게 숨깁니다.
void UCFAimReticleWidget::HideWeaponReticle()
{
	if (IsValid(Image_WeaponReticle))
	{
		Image_WeaponReticle->SetVisibility(ESlateVisibility::Collapsed);
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

	if (IsTurretAligningFeedbackActive())
	{
		return TurretAligningReticleColor;
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

// [v1.5.0] 현재 FireFeedback이 터렛 정렬 중 보조 표시인지 반환합니다.
bool UCFAimReticleWidget::IsTurretAligningFeedbackActive() const
{
	return CachedFireFeedbackViewData.bFeedbackActive
		&& CachedFireFeedbackViewData.FeedbackState == ECFVehicleFireFeedbackState::OutOfArcWarning
		&& CachedFireFeedbackViewData.FeedbackDisplayKey == TEXT("TurretAligning");
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
	case ECFVehicleReticleState::TurretAligning:
		return TurretAligningReticleColor;
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
	case ECFVehicleReticleState::TurretAligning:
		return FText::FromString(TEXT("총구가 조준점을 추적 중"));
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
