// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-05-21
// Description: Aim Reticle UI용 C++ 부모 위젯 클래스 구현입니다.
// Scope: VehicleAimComp의 Reticle 상태를 읽어 선택적 TextBlock과 위젯 가시성을 갱신합니다.

#include "UI/CFAimReticleWidget.h"

#include "CFVehicleAimComp.h"
#include "CFVehiclePawn.h"
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
		ApplyReticleState(FallbackReticleState);
		UpdateReticleVisibility();
		return;
	}

	// [v1.0.0] 현재 Pawn에서 읽은 Aim 컴포넌트입니다.
	const UCFVehicleAimComp* VehicleAimComp = VehiclePawnRef->GetVehicleAimComp();
	if (!IsValid(VehicleAimComp))
	{
		bCachedCanFire = false;
		ApplyReticleState(FallbackReticleState);
		UpdateReticleVisibility();
		return;
	}

	// [v1.0.0] Reticle 표시와 발사 가능 표시를 계산할 최신 Local Aim 상태입니다.
	const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();
	bCachedCanFire = LocalAimState.bLocalCanFire;

	ApplyReticleState(VehicleAimComp->GetReticleState());
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
	case ECFVehicleReticleState::WaitingServer:
		return FText::FromString(TEXT("조준: 서버 대기"));
	case ECFVehicleReticleState::ServerRejected:
		return FText::FromString(TEXT("조준: 서버 거부"));
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

// [v1.0.0] 현재 캐시값을 선택적 TextBlock들에 반영합니다.
void UCFAimReticleWidget::RefreshTextBlocks()
{
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
	case ECFVehicleReticleState::WaitingServer:
		return FText::FromString(TEXT("서버 응답 대기"));
	case ECFVehicleReticleState::ServerRejected:
		return FText::FromString(TEXT("서버에서 발사 거부"));
	case ECFVehicleReticleState::Hidden:
	default:
		return FText::FromString(TEXT("Reticle 숨김"));
	}
}
