// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-04-23
// Description: VehicleDebug HUD용 C++ 부모 위젯 클래스 구현입니다.
// Scope: VehicleDebug Overview를 읽어 HUD 텍스트를 갱신하고 HUD 가시성을 제어합니다.

#include "UI/CFVehicleDebugHudWidget.h"

#include "Components/TextBlock.h"

void UCFVehicleDebugHudWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
	// [v1.0.0] HUD가 읽을 차량 Pawn 참조를 저장합니다.
	VehiclePawnRef = InVehiclePawnRef;

	RefreshFromPawn();
	UpdateHudVisibility();
}

void UCFVehicleDebugHudWidget::RefreshFromPawn()
{
	// [v1.0.0] 유효한 Pawn이 없으면 HUD를 숨기고 갱신을 중단합니다.
	if (!IsValid(VehiclePawnRef))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// [v1.0.0] 현재 Pawn 기준 최신 Overview를 가져옵니다.
	const FCFVehicleDebugOverview LatestOverview = VehiclePawnRef->GetVehicleDebugOverview();

	// [v1.0.0] 현재 HUD 캐시에 최신 Overview를 저장합니다.
	CachedOverview = LatestOverview;

	ApplyVehicleDebugOverview(CachedOverview);
}

void UCFVehicleDebugHudWidget::ApplyVehicleDebugOverview(const FCFVehicleDebugOverview& InOverview)
{
	// [v1.0.0] HUD 제목 텍스트를 고정 문자열로 설정합니다.
	if (Text_Title)
	{
		Text_Title->SetText(FText::FromString(TEXT("Vehicle Debug HUD")));
	}

	// [v1.0.0] 런타임 준비 상태 문자열입니다.
	const FString RuntimeReadyText = InOverview.bRuntimeReady ? TEXT("Runtime: Ready") : TEXT("Runtime: Not Ready");

	// [v1.0.0] 런타임 준비 상태 텍스트를 반영합니다.
	if (Text_RuntimeReady)
	{
		Text_RuntimeReady->SetText(FText::FromString(RuntimeReadyText));
	}

	// [v1.0.0] 현재 Drive 상태 표시 문자열입니다.
	const FString DriveStateText = FString::Printf(
		TEXT("State: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.CurrentDriveState)));

	// [v1.0.0] 현재 Drive 상태 텍스트를 반영합니다.
	if (Text_DriveState)
	{
		Text_DriveState->SetText(FText::FromString(DriveStateText));
	}

	// [v1.0.0] 현재 속도 표시 문자열입니다.
	const FString SpeedText = FString::Printf(TEXT("Speed: %.1f km/h"), InOverview.SpeedKmh);

	// [v1.0.0] 현재 속도 텍스트를 반영합니다.
	if (Text_Speed)
	{
		Text_Speed->SetText(FText::FromString(SpeedText));
	}

	// [v1.0.0] 현재 전후 방향 속도 표시 문자열입니다.
	const FString ForwardSpeedText = FString::Printf(TEXT("Forward: %.1f km/h"), InOverview.ForwardSpeedKmh);

	// [v1.0.0] 현재 전후 방향 속도 텍스트를 반영합니다.
	if (Text_ForwardSpeed)
	{
		Text_ForwardSpeed->SetText(FText::FromString(ForwardSpeedText));
	}

	// [v1.0.0] 현재 입력 장치 모드 표시 문자열입니다.
	const FString DeviceModeText = FString::Printf(
		TEXT("Input Mode: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.DeviceMode)));

	// [v1.0.0] 현재 입력 장치 모드 텍스트를 반영합니다.
	if (Text_DeviceMode)
	{
		Text_DeviceMode->SetText(FText::FromString(DeviceModeText));
	}

	// [v1.0.0] 현재 입력 주도권 표시 문자열입니다.
	const FString InputOwnerText = FString::Printf(
		TEXT("Input Owner: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.InputOwner)));

	// [v1.0.0] 현재 입력 주도권 텍스트를 반영합니다.
	if (Text_InputOwner)
	{
		Text_InputOwner->SetText(FText::FromString(InputOwnerText));
	}

	// [v1.0.0] 마지막 상태 전이 요약 표시 문자열입니다.
	const FString LastTransitionText = FString::Printf(TEXT("Last: %s"), *InOverview.LastTransitionShortText);

	// [v1.0.0] 마지막 상태 전이 요약 텍스트를 반영합니다.
	if (Text_LastTransition)
	{
		Text_LastTransition->SetText(FText::FromString(LastTransitionText));
	}
}

void UCFVehicleDebugHudWidget::UpdateHudVisibility()
{
	// [v1.0.0] 유효한 Pawn이 없으면 HUD를 숨깁니다.
	if (!IsValid(VehiclePawnRef))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// [v1.0.0] Pawn의 HUD 표시 정책을 계산한 결과입니다.
	const bool bShouldShowHud = VehiclePawnRef->ShouldShowVehicleDebugHud();

	SetVisibility(bShouldShowHud ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCFVehicleDebugHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshFromPawn();
	UpdateHudVisibility();
}

void UCFVehicleDebugHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// [v1.0.0] 자동 갱신이 꺼져 있으면 매 프레임 갱신을 건너뜁니다.
	if (!bAutoRefreshEveryTick)
	{
		return;
	}

	RefreshFromPawn();
	UpdateHudVisibility();
}

FString UCFVehicleDebugHudWidget::ConvertEnumValueToDisplayString(const TCHAR* EnumValueString) const
{
	// [v1.0.0] HUD에 표시할 enum 문자열 원본입니다.
	FString DisplayString = EnumValueString ? EnumValueString : TEXT("");

	// [v1.0.0] 마지막 `::` 뒤 값만 남기기 위한 구분 인덱스입니다.
	int32 ValueSeparatorIndex = INDEX_NONE;

	// [v1.0.0] enum 풀네임에서 실제 값 이름 시작 위치를 찾습니다.
	if (DisplayString.FindLastChar(TEXT(':'), ValueSeparatorIndex) && ValueSeparatorIndex + 1 < DisplayString.Len())
	{
		return DisplayString.Mid(ValueSeparatorIndex + 1);
	}

	return DisplayString;
}
