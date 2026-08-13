// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.0
// Date: 2026-08-13
// Description: CF-FQ-032 UI-P0-03 + CF-FQ-031 AMMO-P0-06 HUD Presenter 구현
// Scope: Provider ViewData만 사용해 D1-11 Production Widget의 Weapon Ammo·Reserve·Reload·공통 Launcher Presentation lifecycle을 갱신합니다.
// Changelog:
// - v1.7.0: 폐기된 Salvo 전용 시간 Hold를 제거하고 LauncherSequenceRevision 기반 Active→Terminal Snapshot→Cooldown/READY 공통 lifecycle로 교정. FirePattern은 표시 문구에만 사용.
// - v1.6.0: [폐기 이력] TestMap_DRSalvo USER PIE에서 0.25초 Salvo 완료 Hold를 0.75초로 확대했던 접근.
// - v1.5.0: [폐기 이력] 같은 Game Thread 처리 묶음의 Salvo를 위해 Presentation-only 0.25초 Hold를 추가했던 접근.
// - v1.4.0: Primary Ammo를 `Loaded / MagazineCapacity`로 변경하고 ReserveAmmoCount를 label-less 별도 숫자로 분리. Immediate/CurrentUsable 상태 판정은 유지.
// - v1.3.0: 실제 finite Ammo를 `ImmediateUsable | CurrentUsable`로 표시하고 기존 상태 행을 Sequence > Reload > NoAmmo > Cooldown/Ready 우선순위로 적용.
// - v1.2.0: 정상 Ripple/Salvo Sequence를 WeaponPanel 전용 Row/Progress로 표시하고 Sequence 중 Cooldown을 숨기며 AlertFeed에서는 정상 Sequence를 제외.
// - v1.1.0: AlertKey 의미 기반 Warning/Launcher 슬롯 라우팅과 단독 Launcher Alert 가시성을 교정.
// - v1.0.0: Vehicle/Defense/Weapon/Target/Radar/Alert Runtime 적용과 Mock 값 제거 경로를 구현.
// Migration:
// - 정상 Launcher Sequence 진행은 AlertFeed가 아니라 WeaponPanel의 HorizontalBox_LauncherSequence/Text_WeaponLauncherSequence/ProgressBar_LauncherSequence가 소유합니다.
// - Ammo 행은 finite Runtime Snapshot의 LoadedAmmoCount/MagazineCapacity만 Primary로 표시하고 ReserveAmmoCount는 별도 Text_WeaponReserveAmmo에 표시합니다.
// - ImmediateUsableAmmoCount와 CurrentUsableAmmoCount는 UI 상태 판정·회귀용으로 유지하며 Primary Ammo 문자열을 만들지 않습니다.
// - Reload와 NoAmmo는 기존 Weapon 상태 행을 재사용합니다.
// - Widget 이름이 누락된 경우 null-safe로 건너뛰며 Gameplay나 Pause 흐름은 실패시키지 않습니다.

#include "UI/CFHUDPresenter.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFStyledWidgetBase.h"

// [v1.0.0] HUDDataProvider 변경 이벤트를 구독하고 현재 ViewData를 준비합니다.
bool UCFHUDPresenter::InitializePresenter(UCFHUDDataProvider* InDataProvider)
{
	if (!InDataProvider)
	{
		return false;
	}

	if (DataProvider == InDataProvider)
	{
		ApplyViewData(DataProvider->GetCurrentViewData());
		return true;
	}

	ShutdownPresenter();
	DataProvider = InDataProvider;
	DataProvider->OnHUDViewDataChanged.RemoveDynamic(this, &UCFHUDPresenter::HandleHUDViewDataChanged);
	DataProvider->OnHUDViewDataChanged.AddDynamic(this, &UCFHUDPresenter::HandleHUDViewDataChanged);
	ApplyViewData(DataProvider->GetCurrentViewData());
	return true;
}

// [v1.0.0] Provider 이벤트와 Production Widget 참조를 모두 해제합니다.
void UCFHUDPresenter::ShutdownPresenter()
{
	if (DataProvider)
	{
		DataProvider->OnHUDViewDataChanged.RemoveDynamic(this, &UCFHUDPresenter::HandleHUDViewDataChanged);
	}
	DataProvider = nullptr;
	ProductionWidget.Reset();
	ResetLauncherPresentationLifecycle();
	LastAppliedBindingGeneration = INDEX_NONE;
}

// [v1.0.0] 현재 Production WBP_CFInGameHUD 인스턴스를 Presenter 출력 대상으로 연결합니다.
void UCFHUDPresenter::SetProductionWidget(UCFStyledWidgetBase* InProductionWidget)
{
	ProductionWidget = InProductionWidget;
	ResetLauncherPresentationLifecycle();
	LastAppliedBindingGeneration = INDEX_NONE;
	if (DataProvider && ProductionWidget.IsValid())
	{
		ApplyViewData(DataProvider->GetCurrentViewData());
	}
}

// [v1.0.0] Provider의 새 ViewData를 현재 Production Widget에 적용합니다.
void UCFHUDPresenter::HandleHUDViewDataChanged(FCFInGameUIViewData ViewData)
{
	ApplyViewData(ViewData);
}

// [v1.0.0] 전체 ViewData를 Vehicle/Weapon/Target/Radar/Alert 영역별로 적용합니다.
void UCFHUDPresenter::ApplyViewData(const FCFInGameUIViewData& ViewData)
{
	UCFStyledWidgetBase* RootWidget = ProductionWidget.Get();
	if (!RootWidget)
	{
		return;
	}

	// [v1.7.0] Pawn Source가 바뀌면 이전 차량의 Launcher Presentation lifecycle을 새 차량 HUD에 넘기지 않습니다.
	if (LastAppliedBindingGeneration != ViewData.BindingGeneration)
	{
		ResetLauncherPresentationLifecycle();
		LastAppliedBindingGeneration = ViewData.BindingGeneration;
	}

	ApplyVehicleAndDefenseViewData(RootWidget, ViewData.Vehicle, ViewData.Defense);
	ApplyWeaponViewData(RootWidget, ViewData.Weapon);
	ApplyTargetViewData(RootWidget, ViewData.Target);
	ApplyRadarViewData(RootWidget, ViewData.Radar);
	ApplyAlertViewData(RootWidget, ViewData.Alerts);
}

// [v1.0.0] Vehicle 속도와 Defense 상태를 Production VehiclePanel에 적용합니다.
void UCFHUDPresenter::ApplyVehicleAndDefenseViewData(
	UUserWidget* RootWidget,
	const FCFVehicleHUDData& VehicleViewData,
	const FCFDefenseHUDData& DefenseViewData) const
{
	UUserWidget* VehiclePanel = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFVehiclePanel")));
	if (!VehiclePanel)
	{
		return;
	}

	UUserWidget* SpeedGauge = FindNamedUserWidget(VehiclePanel, FName(TEXT("WBP_CFSpeedGauge")));
	if (SpeedGauge)
	{
		// [v1.0.0] 3자리 Speed Slot의 표시 상한 999에 맞춘 실제 속도 정수입니다.
		const int32 DisplaySpeed = FMath::Clamp(FMath::RoundToInt(VehicleViewData.SpeedKmh), 0, 999);
		SetTextValue(
			SpeedGauge,
			FName(TEXT("Text_Speed")),
			FText::FromString(FString::Printf(TEXT("%03d"), DisplaySpeed)),
			VehicleViewData.SpeedAvailability == ECFUIViewAvailability::Known
				|| VehicleViewData.SpeedAvailability == ECFUIViewAvailability::KnownZero);
		SetTextValue(
			SpeedGauge,
			FName(TEXT("Text_Gear")),
			VehicleViewData.GearText,
			VehicleViewData.GearAvailability == ECFUIViewAvailability::Known
				|| VehicleViewData.GearAvailability == ECFUIViewAvailability::KnownZero);
	}

	// [v1.0.0] Legacy Defense에서는 Shield Row 전체를 숨기고 Integrity만 유지할지 여부입니다.
	const bool bShowShield = DefenseViewData.Availability == ECFUIViewAvailability::Known
		&& DefenseViewData.MaximumShield > KINDA_SMALL_NUMBER;
	SetNamedVisibility(VehiclePanel, FName(TEXT("HorizontalBox_Shield")), bShowShield);
	SetTextValue(
		VehiclePanel,
		FName(TEXT("Text_ShieldValue")),
		FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), DefenseViewData.CurrentShield, DefenseViewData.MaximumShield)),
		bShowShield);
	SetProgressValue(VehiclePanel, FName(TEXT("ProgressBar_Shield")), DefenseViewData.ShieldRatio, bShowShield);

	// [v1.0.0] VehicleHealth Runtime이 실제 Integrity를 제공하는지 여부입니다.
	const bool bShowIntegrity = DefenseViewData.IntegrityAvailability == ECFUIViewAvailability::Known
		|| DefenseViewData.IntegrityAvailability == ECFUIViewAvailability::KnownZero;
	SetNamedVisibility(VehiclePanel, FName(TEXT("HorizontalBox_Integrity")), bShowIntegrity);
	SetTextValue(
		VehiclePanel,
		FName(TEXT("Text_IntegrityValue")),
		FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), DefenseViewData.CurrentIntegrity, DefenseViewData.MaximumIntegrity)),
		bShowIntegrity);
	SetProgressValue(VehiclePanel, FName(TEXT("ProgressBar_Integrity")), DefenseViewData.IntegrityRatio, bShowIntegrity);

	UUserWidget* ArmorBodyMap = FindNamedUserWidget(VehiclePanel, FName(TEXT("WBP_CFArmorBodyMap")));
	if (!ArmorBodyMap)
	{
		return;
	}

	// [v1.0.0] 정식 Defense Runtime이 있을 때만 6방향 Armor Bar를 실제 비율로 표시합니다.
	const bool bShowArmor = DefenseViewData.ArmorAvailability == ECFUIViewAvailability::Known
		|| DefenseViewData.ArmorAvailability == ECFUIViewAvailability::KnownZero;
	SetProgressValue(ArmorBodyMap, FName(TEXT("ProgressBar_ArmorFront")), DefenseViewData.FrontArmorRatio, bShowArmor);
	SetProgressValue(ArmorBodyMap, FName(TEXT("ProgressBar_ArmorRight")), DefenseViewData.RightArmorRatio, bShowArmor);
	SetProgressValue(ArmorBodyMap, FName(TEXT("ProgressBar_ArmorRear")), DefenseViewData.RearArmorRatio, bShowArmor);
	SetProgressValue(ArmorBodyMap, FName(TEXT("ProgressBar_ArmorLeft")), DefenseViewData.LeftArmorRatio, bShowArmor);
	SetProgressValue(ArmorBodyMap, FName(TEXT("ProgressBar_ArmorTop")), DefenseViewData.TopArmorRatio, bShowArmor);
	SetProgressValue(ArmorBodyMap, FName(TEXT("ProgressBar_ArmorBottom")), DefenseViewData.BottomArmorRatio, bShowArmor);
}

// [v1.7.0] Active 여부와 무관한 유효 Launcher Snapshot을 Pattern 문구와 0~1 진행률로 변환합니다.
bool UCFHUDPresenter::ResolveLauncherSequenceSnapshotPresentation(
	const FCFWeaponHUDData& WeaponViewData,
	FText& OutSequenceText,
	float& OutSequenceProgress)
{
	OutSequenceText = FText::GetEmpty();
	OutSequenceProgress = 0.0f;

	// [v1.7.0] Snapshot 유효성은 FirePattern이 아니라 실제 Launcher 채널과 전체 발사 수로만 판정합니다.
	const bool bHasPresentableLauncherSnapshot =
		(WeaponViewData.LauncherAvailability == ECFUIViewAvailability::Known
			|| WeaponViewData.LauncherAvailability == ECFUIViewAvailability::KnownZero)
		&& WeaponViewData.LauncherTotalProjectileCount > 0;
	if (!bHasPresentableLauncherSnapshot)
	{
		return false;
	}

	// [v1.7.0] FirePattern은 Presentation 수명에 관여하지 않고 Player-facing Pattern 문구 선택에만 사용합니다.
	const TCHAR* LauncherPatternText = nullptr;
	switch (WeaponViewData.LauncherPattern)
	{
	case ECFLauncherFirePattern::Ripple:
		LauncherPatternText = TEXT("RIPPLE");
		break;
	case ECFLauncherFirePattern::Salvo:
		LauncherPatternText = TEXT("SALVO");
		break;
	default:
		return false;
	}

	// [v1.7.0] 현재 또는 완료 Snapshot에서 실제 승인된 발사 수를 전체 수와 함께 표시할 값입니다.
	const int32 DisplayAcceptedProjectileCount = FMath::Clamp(
		WeaponViewData.LauncherAcceptedProjectileCount,
		0,
		WeaponViewData.LauncherTotalProjectileCount);
	OutSequenceText = FText::FromString(FString::Printf(
		TEXT("%s %d / %d"),
		LauncherPatternText,
		DisplayAcceptedProjectileCount,
		WeaponViewData.LauncherTotalProjectileCount));
	OutSequenceProgress = FMath::Clamp(
		static_cast<float>(DisplayAcceptedProjectileCount) / static_cast<float>(WeaponViewData.LauncherTotalProjectileCount),
		0.0f,
		1.0f);
	return true;
}

// [v1.2.0] 실제 Active Launcher ViewData를 WeaponPanel Sequence 문구와 0~1 진행률로 변환합니다.
bool UCFHUDPresenter::ResolveLauncherSequencePresentation(
	const FCFWeaponHUDData& WeaponViewData,
	FText& OutSequenceText,
	float& OutSequenceProgress)
{
	OutSequenceText = FText::GetEmpty();
	OutSequenceProgress = 0.0f;

	// [v1.2.0] 실제 Active Sequence와 유효한 전체 발사 수가 함께 존재할 때만 표시합니다.
	const bool bHasActiveLauncherSequence = WeaponViewData.LauncherAvailability == ECFUIViewAvailability::Known
		&& WeaponViewData.bLauncherSequenceActive
		&& WeaponViewData.LauncherTotalProjectileCount > 0;
	if (!bHasActiveLauncherSequence)
	{
		return false;
	}

	return ResolveLauncherSequenceSnapshotPresentation(
		WeaponViewData,
		OutSequenceText,
		OutSequenceProgress);
}


// [v1.4.0] 실제 finite Ammo ViewData를 승인된 `Loaded / MagazineCapacity` Primary 문구로 변환합니다.
bool UCFHUDPresenter::ResolveAmmoPresentation(
	const FCFWeaponHUDData& WeaponViewData,
	FText& OutAmmoText)
{
	OutAmmoText = FText::GetEmpty();

	// [v1.4.0] 실제 finite Ammo Snapshot이 0 또는 비0 현재값을 제공하는 상태인지 여부입니다.
	const bool bAmmoKnown = WeaponViewData.AmmoAvailability == ECFUIViewAvailability::Known
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::KnownZero;
	if (!bAmmoKnown)
	{
		return false;
	}

	// [v1.4.0] WeaponPanel Primary 왼쪽에 표시할 실제 현재 장전량입니다.
	const int32 LoadedAmmoCount = FMath::Max(WeaponViewData.LoadedAmmoCount, 0);

	// [v1.4.0] WeaponPanel Primary 오른쪽에 표시할 실제 탄창 총 용량입니다.
	const int32 MagazineCapacity = FMath::Max(WeaponViewData.MagazineCapacity, 0);
	OutAmmoText = FText::FromString(FString::Printf(
		TEXT("%d / %d"),
		LoadedAmmoCount,
		MagazineCapacity));
	return true;
}

// [v1.4.0] 실제 finite Ammo ViewData의 ReserveAmmoCount를 label-less 우상단 숫자로 변환합니다.
bool UCFHUDPresenter::ResolveReserveAmmoPresentation(
	const FCFWeaponHUDData& WeaponViewData,
	FText& OutReserveAmmoText)
{
	OutReserveAmmoText = FText::GetEmpty();

	// [v1.4.0] Primary Ammo와 동일한 finite Runtime 가용 상태에서만 Reserve 숫자를 노출합니다.
	const bool bAmmoKnown = WeaponViewData.AmmoAvailability == ECFUIViewAvailability::Known
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::KnownZero;
	if (!bAmmoKnown)
	{
		return false;
	}

	// [v1.4.0] 같은 AmmoId의 차량 공유 예비량을 라벨 없이 숫자로만 표시합니다.
	const int32 ReserveAmmoCount = FMath::Max(WeaponViewData.ReserveAmmoCount, 0);
	OutReserveAmmoText = FText::AsNumber(ReserveAmmoCount);
	return true;
}

// [v1.3.0] Sequence 외 Weapon 상태를 Reload > NoAmmo > Cooldown/Ready 우선순위의 상태 문구와 진행률로 변환합니다.
bool UCFHUDPresenter::ResolveWeaponStatusPresentation(
	const FCFWeaponHUDData& WeaponViewData,
	const bool bLauncherSequenceVisible,
	FText& OutStatusText,
	float& OutStatusProgress)
{
	OutStatusText = FText::GetEmpty();
	OutStatusProgress = 0.0f;
	if (bLauncherSequenceVisible)
	{
		return false;
	}

	// [v1.3.0] 실제 finite Ammo Snapshot이 상태 판단에 사용할 수 있는지 여부입니다.
	const bool bAmmoKnown = WeaponViewData.AmmoAvailability == ECFUIViewAvailability::Known
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::KnownZero;
	if (bAmmoKnown && WeaponViewData.ReloadState == ECFWeaponReloadState::Reloading)
	{
		// [v1.3.0] Runtime이 제공한 실제 Reload 전체 시간입니다.
		const float ReloadDurationSeconds = FMath::Max(WeaponViewData.ReloadDurationSeconds, 0.0f);

		// [v1.3.0] Runtime이 제공한 실제 Reload 남은 시간입니다.
		const float RemainingReloadTimeSeconds = FMath::Max(WeaponViewData.RemainingReloadTimeSeconds, 0.0f);

		// [v1.3.0] 10초 미만은 0.1초, 10초 이상은 정수 초로 읽기 쉽게 표시한 남은 시간입니다.
		const FString RemainingReloadTimeText = RemainingReloadTimeSeconds < 10.0f
			? FString::Printf(TEXT("%.1f"), RemainingReloadTimeSeconds)
			: FString::Printf(TEXT("%.0f"), RemainingReloadTimeSeconds);

		// [v1.3.0] 전체 Reload 시간도 같은 표시 정밀도 규칙으로 만든 문자열입니다.
		const FString ReloadDurationText = ReloadDurationSeconds < 10.0f
			? FString::Printf(TEXT("%.1f"), ReloadDurationSeconds)
			: FString::Printf(TEXT("%.0f"), ReloadDurationSeconds);
		OutStatusText = FText::FromString(FString::Printf(
			TEXT("RELOAD %s / %s s"),
			*RemainingReloadTimeText,
			*ReloadDurationText));
		OutStatusProgress = ReloadDurationSeconds > KINDA_SMALL_NUMBER
			? 1.0f - FMath::Clamp(RemainingReloadTimeSeconds / ReloadDurationSeconds, 0.0f, 1.0f)
			: 1.0f;
		return true;
	}

	if (bAmmoKnown && WeaponViewData.CurrentOnboardAmmoCount <= 0)
	{
		OutStatusText = FText::FromString(TEXT("NO AMMO"));
		OutStatusProgress = 0.0f;
		return true;
	}

	// [v1.3.0] 기존 Weapon/Volley Cooldown이 실제 Runtime 값으로 제공되는지 여부입니다.
	const bool bCooldownKnown = WeaponViewData.CooldownDurationSeconds > KINDA_SMALL_NUMBER
		&& (WeaponViewData.CooldownAvailability == ECFUIViewAvailability::Known
			|| WeaponViewData.CooldownAvailability == ECFUIViewAvailability::KnownZero);
	if (!bCooldownKnown)
	{
		return false;
	}

	// [v1.3.0] 현재 Weapon/Volley Cooldown의 실제 남은 시간입니다.
	const float RemainingCooldownSeconds = FMath::Max(WeaponViewData.RemainingCooldownSeconds, 0.0f);
	if (RemainingCooldownSeconds <= KINDA_SMALL_NUMBER)
	{
		OutStatusText = FText::FromString(TEXT("READY"));
		OutStatusProgress = 1.0f;
		return true;
	}

	OutStatusText = RemainingCooldownSeconds < 10.0f
		? FText::FromString(FString::Printf(TEXT("%.1f s"), RemainingCooldownSeconds))
		: FText::FromString(FString::Printf(TEXT("%.0f s"), RemainingCooldownSeconds));
	OutStatusProgress = 1.0f - FMath::Clamp(
		RemainingCooldownSeconds / WeaponViewData.CooldownDurationSeconds,
		0.0f,
		1.0f);
	return true;
}

// [v1.7.0] 실제 Launcher 상태 이벤트 Revision을 기준으로 모든 Sequence의 공통 Presentation lifecycle을 해석합니다.
bool UCFHUDPresenter::ResolveLauncherSequenceDisplay(
	const FCFWeaponHUDData& WeaponViewData,
	FText& OutSequenceText,
	float& OutSequenceProgress)
{
	OutSequenceText = FText::GetEmpty();
	OutSequenceProgress = 0.0f;

	// [v1.7.0] 현재 실제 Active Sequence Snapshot입니다. FirePattern은 이 lifecycle 판정에 사용하지 않습니다.
	FText ActiveSequenceText;
	float ActiveSequenceProgress = 0.0f;
	const bool bActiveSequenceVisible = ResolveLauncherSequencePresentation(
		WeaponViewData,
		ActiveSequenceText,
		ActiveSequenceProgress);
	if (bActiveSequenceVisible)
	{
		// [v1.7.0] 새 Weapon의 Active Sequence가 시작되면 이전 Weapon의 Presentation 상태를 먼저 폐기합니다.
		if (LauncherPresentationWeaponId != WeaponViewData.WeaponId)
		{
			ResetLauncherPresentationLifecycle();
		}

		bLauncherSequencePresentationActive = true;
		bLauncherTerminalPresentationShown = false;
		LauncherPresentationWeaponId = WeaponViewData.WeaponId;
		LastLauncherSequenceRevision = WeaponViewData.LauncherSequenceRevision;
		LastLauncherSequenceText = ActiveSequenceText;
		LastLauncherSequenceProgress = ActiveSequenceProgress;
		OutSequenceText = LastLauncherSequenceText;
		OutSequenceProgress = LastLauncherSequenceProgress;
		return true;
	}

	// [v1.7.0] Active를 관측하지 않은 Idle/SingleCycle 경로는 Launcher Row를 만들지 않고 공통 Weapon Status로 진행합니다.
	if (!bLauncherSequencePresentationActive && !bLauncherTerminalPresentationShown)
	{
		return false;
	}

	// [v1.7.0] Weapon이 바뀌면 이전 Sequence의 terminal Snapshot을 새 무기로 넘기지 않습니다.
	if (LauncherPresentationWeaponId != WeaponViewData.WeaponId)
	{
		ResetLauncherPresentationLifecycle();
		return false;
	}

	if (bLauncherSequencePresentationActive)
	{
		// [v1.7.0] Launcher Revision이 그대로면 Ammo 예약 해제 같은 부수 Refresh입니다. 정식 terminal 이벤트 전까지 마지막 Active Presentation을 유지합니다.
		if (WeaponViewData.LauncherSequenceRevision == LastLauncherSequenceRevision)
		{
			OutSequenceText = LastLauncherSequenceText;
			OutSequenceProgress = LastLauncherSequenceProgress;
			return !OutSequenceText.IsEmpty();
		}

		// [v1.7.0] Revision이 바뀐 inactive Snapshot은 실제 Completed/Cancelled Launcher 이벤트입니다. 최종 Snapshot을 정확히 한 ViewData 주기 표시합니다.
		FText TerminalSequenceText;
		float TerminalSequenceProgress = 0.0f;
		if (ResolveLauncherSequenceSnapshotPresentation(
			WeaponViewData,
			TerminalSequenceText,
			TerminalSequenceProgress))
		{
			bLauncherSequencePresentationActive = false;
			bLauncherTerminalPresentationShown = true;
			LastLauncherSequenceRevision = WeaponViewData.LauncherSequenceRevision;
			LastLauncherSequenceText = TerminalSequenceText;
			LastLauncherSequenceProgress = TerminalSequenceProgress;
			OutSequenceText = LastLauncherSequenceText;
			OutSequenceProgress = LastLauncherSequenceProgress;
			return true;
		}

		ResetLauncherPresentationLifecycle();
		return false;
	}

	// [v1.7.0] terminal Snapshot을 한 번 적용한 다음 ViewData부터는 공통 Weapon Status가 Cooldown→READY를 표시합니다.
	ResetLauncherPresentationLifecycle();
	return false;
}

// [v1.7.0] Pawn·Widget·Weapon 전환에서 Launcher Presentation lifecycle 상태를 초기화합니다.
void UCFHUDPresenter::ResetLauncherPresentationLifecycle()
{
	bLauncherSequencePresentationActive = false;
	bLauncherTerminalPresentationShown = false;
	LauncherPresentationWeaponId = NAME_None;
	LastLauncherSequenceRevision = INDEX_NONE;
	LastLauncherSequenceText = FText::GetEmpty();
	LastLauncherSequenceProgress = 0.0f;
}

// [v1.5.0] Weapon, Ammo, Reload와 Launcher 상태를 Production WeaponPanel에 우선순위대로 적용합니다.
void UCFHUDPresenter::ApplyWeaponViewData(UUserWidget* RootWidget, const FCFWeaponHUDData& WeaponViewData)
{
	UUserWidget* WeaponPanel = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFWeaponPanel")));
	if (!WeaponPanel)
	{
		return;
	}

	// [v1.0.0] 내부 WeaponId를 Player-facing 이름으로 노출하지 않는 안전한 제목입니다.
	FText WeaponTitle = FText::FromString(TEXT("WEAPON UNAVAILABLE"));
	if (WeaponViewData.Availability == ECFUIViewAvailability::KnownZero)
	{
		WeaponTitle = FText::FromString(TEXT("NO WEAPON"));
	}
	else if (WeaponViewData.Availability == ECFUIViewAvailability::Known)
	{
		WeaponTitle = WeaponViewData.DisplayNameAvailability == ECFUIViewAvailability::Known && !WeaponViewData.DisplayName.IsEmpty()
			? WeaponViewData.DisplayName
			: FText::FromString(TEXT("WEAPON"));
	}
	SetTextValue(WeaponPanel, FName(TEXT("Text_WeaponTitle")), WeaponTitle, true);

			// [v1.4.0] 실제 finite Ammo Snapshot을 승인된 `Loaded / MagazineCapacity` Primary 문구로 변환한 값입니다.
	FText AmmoText;

	// [v1.4.0] Ammo Runtime이 실제 현재 수량을 제공해 Ammo Row를 표시할지 여부입니다.
	const bool bShowAmmo = ResolveAmmoPresentation(WeaponViewData, AmmoText);
	SetNamedVisibility(WeaponPanel, FName(TEXT("HorizontalBox_Ammo")), bShowAmmo);
	SetTextValue(WeaponPanel, FName(TEXT("Text_WeaponAmmo")), AmmoText, bShowAmmo);

	// [v1.4.0] WeaponPanel 우상단에 라벨 없이 표시할 실제 차량 Reserve Ammo 숫자입니다.
	FText ReserveAmmoText;
	const bool bShowReserveAmmo = ResolveReserveAmmoPresentation(WeaponViewData, ReserveAmmoText);
	SetTextValue(WeaponPanel, FName(TEXT("Text_WeaponReserveAmmo")), ReserveAmmoText, bShowReserveAmmo);
	SetNamedVisibility(WeaponPanel, FName(TEXT("HorizontalBox_Heat")), false);
	SetNamedVisibility(WeaponPanel, FName(TEXT("ProgressBar_Heat")), false);

	// [v1.2.0] 실제 Launcher Sequence가 진행 중일 때 WeaponPanel Primary Action으로 사용할 문구입니다.
	FText LauncherSequenceText;
	// [v1.2.0] 현재 Launcher Sequence의 0~1 실제 승인 진행률입니다.
	float LauncherSequenceProgress = 0.0f;
			const bool bShowLauncherSequence = ResolveLauncherSequenceDisplay(
		WeaponViewData,
		LauncherSequenceText,
		LauncherSequenceProgress);
	SetNamedVisibility(WeaponPanel, FName(TEXT("HorizontalBox_LauncherSequence")), bShowLauncherSequence);
	SetTextValue(
		WeaponPanel,
		FName(TEXT("Text_WeaponLauncherSequence")),
		LauncherSequenceText,
		bShowLauncherSequence);
	SetProgressValue(
		WeaponPanel,
		FName(TEXT("ProgressBar_LauncherSequence")),
		LauncherSequenceProgress,
		bShowLauncherSequence);

		// [v1.3.0] Sequence가 아닌 현재 Weapon 상태를 Reload > NoAmmo > Cooldown/Ready 우선순위로 변환한 문구입니다.
	FText WeaponStatusText;

	// [v1.3.0] 현재 Weapon 상태 행에 표시할 실제 0~1 진행률입니다.
	float WeaponStatusProgress = 0.0f;

	// [v1.3.0] Sequence 전용 Row가 보일 때 상태 행을 숨기고 그 외에는 실제 상태가 있을 때만 표시합니다.
	const bool bShowWeaponStatus = ResolveWeaponStatusPresentation(
		WeaponViewData,
		bShowLauncherSequence,
		WeaponStatusText,
		WeaponStatusProgress);
	SetNamedVisibility(WeaponPanel, FName(TEXT("HorizontalBox_Cooldown")), bShowWeaponStatus);
	SetTextValue(
		WeaponPanel,
		FName(TEXT("Text_WeaponCooldown")),
		WeaponStatusText,
		bShowWeaponStatus);
	SetProgressValue(
		WeaponPanel,
		FName(TEXT("ProgressBar_Cooldown")),
		WeaponStatusProgress,
		bShowWeaponStatus);
}

// [v1.0.0] 선택 Target 공개 정보를 Production TargetPanel에 적용합니다.
void UCFHUDPresenter::ApplyTargetViewData(UUserWidget* RootWidget, const FCFTargetHUDData& TargetViewData) const
{
	UUserWidget* TargetPanel = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFTargetPanel")));
	if (!TargetPanel)
	{
		return;
	}

	if (!TargetViewData.bHasSelectedTarget)
	{
		SetTextValue(TargetPanel, FName(TEXT("Text_TargetTitle")), FText::FromString(TEXT("NO TARGET")), true);
		SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetDistance")), false);
		SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetIdentity")), false);
		SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetArmor")), false);
		SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetScan")), false);
		SetNamedVisibility(TargetPanel, FName(TEXT("ProgressBar_TargetScan")), false);
		return;
	}

	// [v1.0.0] Identified 이상에서만 공개 DisplayName을 사용하고 Detected 단계에서는 Unknown Contact를 사용합니다.
	const bool bIdentityKnown = TargetViewData.IdentityAvailability == ECFUIViewAvailability::Known
		&& !TargetViewData.DisplayName.IsEmpty();
	SetTextValue(
		TargetPanel,
		FName(TEXT("Text_TargetTitle")),
		bIdentityKnown ? TargetViewData.DisplayName : FText::FromString(TEXT("UNKNOWN CONTACT")),
		true);
	SetTextValue(
		TargetPanel,
		FName(TEXT("Text_TargetIdentity")),
		bIdentityKnown
			? FText::Format(FText::FromString(TEXT("식별  {0}")), TargetViewData.DisplayName)
			: FText::FromString(TEXT("식별  ???")),
		true);

	// [v1.0.0] Sensor/Knowledge Provider가 아직 없는 거리, Armor Intelligence와 Scan Progress는 가짜 값을 표시하지 않습니다.
	SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetDistance")), false);
	SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetArmor")), false);
	SetNamedVisibility(TargetPanel, FName(TEXT("Text_TargetScan")), false);
	SetNamedVisibility(TargetPanel, FName(TEXT("ProgressBar_TargetScan")), false);
}

// [v1.0.0] Sensor Provider 부재 또는 Contact 상태를 Production RadarPanel에 적용합니다.
void UCFHUDPresenter::ApplyRadarViewData(UUserWidget* RootWidget, const FCFRadarHUDData& RadarViewData) const
{
	UUserWidget* RadarPanel = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFRadarPanel")));
	if (!RadarPanel)
	{
		return;
	}

	// [v1.0.0] 현재 P0에 Sensor Runtime Provider가 없으므로 Static Mock Contact를 숨기고 상태를 명시합니다.
	const bool bRadarAvailable = RadarViewData.Availability == ECFUIViewAvailability::Known
		|| RadarViewData.Availability == ECFUIViewAvailability::KnownZero;
	SetTextValue(
		RadarPanel,
		FName(TEXT("Text_RadarTitle")),
		bRadarAvailable ? FText::FromString(TEXT("RADAR")) : FText::FromString(TEXT("RADAR — UNAVAILABLE")),
		true);
	SetNamedVisibility(RadarPanel, FName(TEXT("CanvasPanel_RadarContacts")), bRadarAvailable);
}

// [v1.2.0] 정상 Launcher Sequence를 제외한 현재 최고 우선순위 전역 Warning/Critical 1개를 Production AlertFeed에 적용합니다.
void UCFHUDPresenter::ApplyAlertViewData(UUserWidget* RootWidget, const FCFCombatAlertViewData& AlertViewData) const
{
	UUserWidget* AlertFeed = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFAlertFeed")));
	if (!AlertFeed)
	{
		return;
	}

	// [v1.2.0] 혹시 외부 공급자가 LauncherSequence를 넣더라도 전역 Alert에서 제외하고 첫 실제 Warning/Critical만 선택합니다.
	const FCFHUDAlertItem* PrimaryAlert = nullptr;
	for (const FCFHUDAlertItem& AlertItem : AlertViewData.ActiveAlerts)
	{
		if (AlertItem.AlertKey != FName(TEXT("LauncherSequence")))
		{
			PrimaryAlert = &AlertItem;
			break;
		}
	}

	// [v1.2.0] 현재 전역 Alert가 실제 존재하는지 나타냅니다.
	const bool bHasPrimaryAlert = PrimaryAlert != nullptr;
	AlertFeed->SetVisibility(bHasPrimaryAlert ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	SetNamedVisibility(AlertFeed, FName(TEXT("VerticalBox_ArmorAlert")), bHasPrimaryAlert);
	SetNamedVisibility(AlertFeed, FName(TEXT("VerticalBox_RippleAlert")), false);
	SetNamedVisibility(AlertFeed, FName(TEXT("Spacer_AlertGap")), false);
	if (!PrimaryAlert)
	{
		return;
	}

	SetTextValue(AlertFeed, FName(TEXT("Text_AlertPrimary")), PrimaryAlert->Title, true);
	SetTextValue(AlertFeed, FName(TEXT("Text_AlertArmor")), PrimaryAlert->Detail, !PrimaryAlert->Detail.IsEmpty());
}

// [v1.0.0] 지정 UserWidget의 WidgetTree에서 이름으로 자식 Widget을 찾습니다.
UWidget* UCFHUDPresenter::FindNamedWidget(UUserWidget* ParentWidget, const FName WidgetName)
{
	return ParentWidget && !WidgetName.IsNone() ? ParentWidget->GetWidgetFromName(WidgetName) : nullptr;
}

// [v1.0.0] 지정 UserWidget의 WidgetTree에서 이름으로 중첩 UserWidget을 찾습니다.
UUserWidget* UCFHUDPresenter::FindNamedUserWidget(UUserWidget* ParentWidget, const FName WidgetName)
{
	return Cast<UUserWidget>(FindNamedWidget(ParentWidget, WidgetName));
}

// [v1.0.0] 지정 이름 TextBlock의 Text와 표시 상태를 함께 적용합니다.
void UCFHUDPresenter::SetTextValue(UUserWidget* ParentWidget, const FName WidgetName, const FText& Text, const bool bVisible)
{
	if (UTextBlock* TextBlock = Cast<UTextBlock>(FindNamedWidget(ParentWidget, WidgetName)))
	{
		TextBlock->SetText(Text);
		TextBlock->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// [v1.0.0] 지정 이름 ProgressBar의 0~1 비율과 표시 상태를 함께 적용합니다.
void UCFHUDPresenter::SetProgressValue(UUserWidget* ParentWidget, const FName WidgetName, const float Percent, const bool bVisible)
{
	if (UProgressBar* ProgressBar = Cast<UProgressBar>(FindNamedWidget(ParentWidget, WidgetName)))
	{
		ProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		ProgressBar->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// [v1.0.0] 지정 이름 Widget의 Visibility를 Collapsed 또는 HitTestInvisible로 적용합니다.
void UCFHUDPresenter::SetNamedVisibility(UUserWidget* ParentWidget, const FName WidgetName, const bool bVisible)
{
	if (UWidget* Widget = FindNamedWidget(ParentWidget, WidgetName))
	{
		Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
