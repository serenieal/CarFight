// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.16.0
// Date: 2026-08-19
// Description: CF-FQ-032 UI-P0-06 truthful Weapon Rail + actual Weapon Charge/Heat + Dynamic Resource Visual Stage B + RPM Gauge Production Presenter
// Scope: Provider ViewData만 사용해 선택 무기 Resource와 비선택 실제 무기 Rail을 Compact 계약으로 표시합니다.
// Changelog:
// - v1.16.0: 실제 WeaponCharge Percent를 Ammo/Launcher 유무에 따라 Primary 또는 Secondary로 Projection하고 `NO CHARGE`를 Reload > NoAmmo > NoCharge > Overheated > Cooldown/Ready 우선순위에 추가. Secondary 최대 2 계약 보존.
// - v1.15.0: WeaponSelection ViewData에서 선택 무기를 제외한 fixed-order Rail을 1~3개 또는 2+overflow로 Projection하고 Production Rail Text 슬롯에 적용. 내부 ID, 가짜 icon, 비선택 resource summary는 사용하지 않음.
// - v1.14.0: 실제 Heat Resource Channel을 HEAT Percent Secondary로 Projection하고 과열 시 Reload > NoAmmo > Overheated > Cooldown/Ready 단일 FireState 우선순위를 적용. Launcher 단일 소비 계약 유지.
// - v1.13.0: 기존 Production SpeedGauge의 21 ProgressBar Tick을 RPM Gauge ratio sink로 연결. 0~100% 20 interval 위치를 보존하며 current ratio가 다음 Tick까지 부분 진행되고, Redline unavailable/invalid이면 21 Tick fill을 모두 0으로 reset.
// - v1.12.0: explicit RedlineStartRPM을 승인된 Tachometer 0.85 위치, EngineMaxRPM을 1.0 끝점으로 piecewise 매핑하는 RPM Gauge Resolver 추가. Production Widget 적용은 후속 visual binding으로 분리.
// - v1.11.0: Dynamic Resource Visual Stage B. ApplyWeaponViewData가 BuildWeaponResourceEntries를 정확히 1회 호출하고 Primary 1 + Secondary A/B + FireState 1 의미 슬롯에 적용. 기존 Launcher/Ammo/Heat/Cooldown 전용 Row 직접 적용을 제거하고 ReserveAmmo Header owner를 유지.
// - v1.10.0: Dynamic Resource Visual Stage A. 기존 Ammo/Status/Launcher Resolver와 LauncherSequenceRevision lifecycle을 재사용해 Primary 1 + Secondary 최대 2 + FireState 1의 Presentation Entry를 생성. ReserveAmmo Header owner와 Production Asset 구조는 변경하지 않음.
// - v1.9.0: UI-P0-06 ResourceChannels를 Ammo·Reserve·Reload·Cooldown·Launcher의 우선 Source로 migration하고 legacy 필드 fallback을 보존. Heat는 실제 Heat Resource Channel이 있을 때만 기존 Row를 표시.
// - v1.8.0: Sensor Radar가 Known이 되어도 정적 디자인 placeholder Contact Canvas를 실제 Contact처럼 노출하지 않도록 계속 숨김. 동적 Contact 렌더링은 Radar Range/Zoom 계약 이후로 분리.
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
// - v1.8.0에서 Radar Provider 가용성과 실제 Contact Widget 생성은 분리합니다. 기존 CanvasPanel_RadarContacts의 정적 placeholder는 Sensor 데이터로 간주하지 않습니다.
// - v1.9.0에서 ResourceChannels가 존재하면 Presenter가 이를 우선 소비하며, 기존 개별 Weapon 필드는 legacy/fallback 호환을 위해 제거하지 않습니다.
// - v1.10.0 Presentation Projection은 ResourceChannels 배열 자체를 Visual Row 순서로 사용하지 않습니다. Launcher가 보이면 Launcher를 Primary로 승격하고 기존 Ammo는 Secondary로 유지하며 일반 FireState는 억제합니다.
// - ReserveAmmo는 기존 Header 우측 label-less Text owner를 유지하고 Projection Entry에는 포함하지 않습니다.
// - VehicleBattery 값은 실제 Runtime 전 Presentation Entry를 만들지 않습니다. WeaponCharge는 v1.16.0부터, Heat는 v1.14.0부터 실제 Runtime Resource Channel이 존재할 때만 표시하며 정적 WeaponData 값으로 현재 상태를 추정하지 않습니다.
// - v1.11.0부터 Production WeaponPanel은 BuildWeaponResourceEntries 결과만 Resource Visual에 사용합니다. ApplyWeaponViewData 안에서 Launcher/Ammo/Status Resolver를 별도로 다시 호출하지 않습니다.
// - ReserveAmmo는 Resource Presentation Container 밖의 Header 우측 owner를 계속 사용합니다.
// - VehicleBattery는 실제 Gameplay Runtime이 없어 Production Visual 슬롯에 가짜 값이나 전용 Row를 만들지 않습니다. WeaponCharge/Heat는 실제 Resource Channel만 기존 Compact 슬롯에 투영합니다.
// - v1.13.0 Production RPM Visual Binding은 저장 SpeedGauge 구조를 바꾸지 않고 ProgressBar_RPMTick00~20의 Percent만 갱신합니다. Resolver fail-closed에서는 모든 Tick fill을 0으로 reset하며 EngineMaxRPM 비율 fallback을 만들지 않습니다.
// - v1.15.0 Weapon Rail은 실제 WeaponSelectionAvailability/SelectedWeaponIndex만 소비합니다. 선택 무기는 Rail에서 제외하고 DisplayName 부재 시 내부 ID fallback 없이 `WEAPON`을 사용하며 비선택 자원 상태는 추정하지 않습니다.


#include "UI/CFHUDPresenter.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFStyledWidgetBase.h"

namespace
{
	// [v1.9.0] 지정 종류의 additive Weapon Resource Channel을 찾고 없으면 nullptr를 반환합니다.
	const FCFWeaponResourceHUDData* FindWeaponResourceChannel(
		const FCFWeaponHUDData& WeaponViewData,
		const ECFWeaponResourceChannelType ChannelType)
	{
		for (const FCFWeaponResourceHUDData& ResourceChannel : WeaponViewData.ResourceChannels)
		{
			if (ResourceChannel.ChannelType == ChannelType)
			{
				return &ResourceChannel;
			}
		}

		return nullptr;
	}
}

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

// [v1.12.0] 실제 Chaos RPM과 explicit Redline/Maximum을 승인 Tachometer 화면 비율로 변환합니다.
bool UCFHUDPresenter::ResolveEngineRpmGaugePresentation(
	const FCFVehicleHUDData& VehicleViewData,
	float& OutGaugeRatio)
{
	OutGaugeRatio = 0.0f;

	// [v1.12.0] 현재 RPM은 실제 Chaos Runtime에서 Known 또는 KnownZero로 제공돼야 합니다.
	const bool bHasCurrentEngineRpm = VehicleViewData.EngineRpmAvailability == ECFUIViewAvailability::Known
		|| VehicleViewData.EngineRpmAvailability == ECFUIViewAvailability::KnownZero;
	if (!bHasCurrentEngineRpm
		|| VehicleViewData.EngineRedlineStartRpmAvailability != ECFUIViewAvailability::Known
		|| VehicleViewData.EngineMaximumRpmAvailability != ECFUIViewAvailability::Known)
	{
		return false;
	}

	// [v1.12.0] Piecewise mapping에 사용할 실제 Chaos 현재 RPM입니다.
	const float CurrentEngineRpm = VehicleViewData.EngineRpm;
	// [v1.12.0] VehicleData가 명시한 실제 레드라인 시작 RPM입니다.
	const float RedlineStartRpm = VehicleViewData.EngineRedlineStartRpm;
	// [v1.12.0] Chaos EngineSetup.MaxRPM과 같은 source의 실제 최대 RPM입니다.
	const float MaximumEngineRpm = VehicleViewData.EngineMaximumRpm;
	if (!FMath::IsFinite(CurrentEngineRpm)
		|| !FMath::IsFinite(RedlineStartRpm)
		|| !FMath::IsFinite(MaximumEngineRpm)
		|| RedlineStartRpm <= KINDA_SMALL_NUMBER
		|| MaximumEngineRpm <= RedlineStartRpm)
	{
		return false;
	}

	// [v1.12.0] 승인 VehiclePanel 사양에서 실제 RedlineStartRPM이 위치하는 Tachometer 화면 비율입니다.
	constexpr float RedlineDisplayRatio = 0.85f;
	// [v1.12.0] 음수/Overrun을 UI 범위 밖으로 보내지 않도록 0~MaximumEngineRPM으로 제한한 실제 RPM입니다.
	const float ClampedEngineRpm = FMath::Clamp(CurrentEngineRpm, 0.0f, MaximumEngineRpm);
	if (ClampedEngineRpm <= RedlineStartRpm)
	{
		OutGaugeRatio = RedlineDisplayRatio * (ClampedEngineRpm / RedlineStartRpm);
	}
	else
	{
		// [v1.12.0] Redline 이후 실제 RPM이 차지하는 물리 구간 비율입니다.
		const float RedlineToMaximumRatio = (ClampedEngineRpm - RedlineStartRpm) / (MaximumEngineRpm - RedlineStartRpm);
		OutGaugeRatio = RedlineDisplayRatio + ((1.0f - RedlineDisplayRatio) * RedlineToMaximumRatio);
	}

	OutGaugeRatio = FMath::Clamp(OutGaugeRatio, 0.0f, 1.0f);
	return true;
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

		// [v1.13.0] explicit Redline/Maximum 계약으로 계산된 현재 Tachometer 0~1 화면 비율입니다.
		float EngineRpmGaugeRatio = 0.0f;
		// [v1.13.0] RedlineStartRPM이 명시되고 current/max까지 유효해 Production Tick fill을 표시할 수 있는지 나타냅니다.
		const bool bHasEngineRpmGaugePresentation = ResolveEngineRpmGaugePresentation(
			VehicleViewData,
			EngineRpmGaugeRatio);
		// [v1.13.0] 저장 SpeedGauge가 가진 고정 RPM Tick 개수입니다. 위치는 0%~100%를 포함한 21개 marker입니다.
		constexpr int32 EngineRpmGaugeTickCount = 21;
		// [v1.13.0] 21개 marker 사이 실제 0%~100% 구간 수입니다.
		constexpr float EngineRpmGaugeIntervalCount = static_cast<float>(EngineRpmGaugeTickCount - 1);
		// [v1.13.0] 현재 Gauge Ratio를 20개 marker interval 위치로 변환한 진행 위치입니다.
		const float EngineRpmGaugeIntervalPosition = bHasEngineRpmGaugePresentation
			? FMath::Clamp(EngineRpmGaugeRatio, 0.0f, 1.0f) * EngineRpmGaugeIntervalCount
			: 0.0f;
		for (int32 TickIndex = 0; TickIndex < EngineRpmGaugeTickCount; ++TickIndex)
		{
			// [v1.13.0] 현재 Production RPM Tick Widget의 고유 이름입니다.
			const FName TickWidgetName(*FString::Printf(TEXT("ProgressBar_RPMTick%02d"), TickIndex));
			// [v1.13.0] 현재 RPM이 0보다 클 때 시작 marker를 켜고 이후 marker는 직전 interval을 통과한 만큼 0~1로 채우는 값입니다.
			float TickFillRatio = 0.0f;
			if (bHasEngineRpmGaugePresentation && EngineRpmGaugeRatio > KINDA_SMALL_NUMBER)
			{
				TickFillRatio = TickIndex == 0
					? 1.0f
					: FMath::Clamp(EngineRpmGaugeIntervalPosition - static_cast<float>(TickIndex - 1), 0.0f, 1.0f);
			}

			SetProgressValue(SpeedGauge, TickWidgetName, TickFillRatio, true);
		}
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

		// [v1.9.0] 실제 LauncherSequence 공통 Resource Channel입니다. 없으면 legacy 필드 계약을 사용합니다.
	const FCFWeaponResourceHUDData* LauncherSequenceChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::LauncherSequence);

	// [v1.9.0] Resource Channel이 유효하면 그 current/max를, 없으면 기존 Launcher 필드를 그대로 사용합니다.
	const bool bUseLauncherResourceChannel = LauncherSequenceChannel
		&& (LauncherSequenceChannel->Availability == ECFUIViewAvailability::Known
			|| LauncherSequenceChannel->Availability == ECFUIViewAvailability::KnownZero)
		&& LauncherSequenceChannel->bHasCurrentValue
		&& LauncherSequenceChannel->bHasMaximumValue
		&& LauncherSequenceChannel->MaximumValue > KINDA_SMALL_NUMBER;

	// [v1.9.0] Player-facing Sequence 전체 발사 수입니다.
	const int32 LauncherTotalProjectileCount = bUseLauncherResourceChannel
		? FMath::Max(FMath::RoundToInt(LauncherSequenceChannel->MaximumValue), 0)
		: WeaponViewData.LauncherTotalProjectileCount;

	// [v1.9.0] Snapshot 유효성은 FirePattern이 아니라 실제 Launcher 채널과 전체 발사 수로만 판정합니다.
	const bool bHasPresentableLauncherSnapshot = bUseLauncherResourceChannel
		? LauncherTotalProjectileCount > 0
		: ((WeaponViewData.LauncherAvailability == ECFUIViewAvailability::Known
				|| WeaponViewData.LauncherAvailability == ECFUIViewAvailability::KnownZero)
			&& LauncherTotalProjectileCount > 0);
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

		// [v1.9.0] 현재 또는 완료 Snapshot에서 실제 승인된 발사 수를 전체 수와 함께 표시할 값입니다.
	const int32 DisplayAcceptedProjectileCount = FMath::Clamp(
		bUseLauncherResourceChannel
			? FMath::RoundToInt(LauncherSequenceChannel->CurrentValue)
			: WeaponViewData.LauncherAcceptedProjectileCount,
		0,
		LauncherTotalProjectileCount);
	OutSequenceText = FText::FromString(FString::Printf(
		TEXT("%s %d / %d"),
		LauncherPatternText,
		DisplayAcceptedProjectileCount,
		LauncherTotalProjectileCount));
	OutSequenceProgress = FMath::Clamp(
		static_cast<float>(DisplayAcceptedProjectileCount) / static_cast<float>(LauncherTotalProjectileCount),
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

		// [v1.9.0] 실제 LauncherSequence 공통 Resource Channel입니다. 없으면 legacy Active 상태를 사용합니다.
	const FCFWeaponResourceHUDData* LauncherSequenceChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::LauncherSequence);

	// [v1.9.0] Resource Channel 기반 현재 Active Sequence 여부입니다.
	const bool bResourceLauncherSequenceActive = LauncherSequenceChannel
		&& LauncherSequenceChannel->Availability == ECFUIViewAvailability::Known
		&& LauncherSequenceChannel->bIsActive
		&& LauncherSequenceChannel->bHasMaximumValue
		&& LauncherSequenceChannel->MaximumValue > KINDA_SMALL_NUMBER;

	// [v1.9.0] 실제 Active Sequence와 유효한 전체 발사 수가 함께 존재할 때만 표시합니다.
	const bool bHasActiveLauncherSequence = LauncherSequenceChannel
		? bResourceLauncherSequenceActive
		: (WeaponViewData.LauncherAvailability == ECFUIViewAvailability::Known
			&& WeaponViewData.bLauncherSequenceActive
			&& WeaponViewData.LauncherTotalProjectileCount > 0);
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

		// [v1.9.0] 실제 Ammo 공통 Resource Channel입니다. 없으면 legacy finite Ammo 필드를 사용합니다.
	const FCFWeaponResourceHUDData* AmmoChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::Ammo);

	// [v1.9.0] Resource Channel이 실제 current/max Ammo를 제공하는지 여부입니다.
	const bool bUseAmmoResourceChannel = AmmoChannel
		&& (AmmoChannel->Availability == ECFUIViewAvailability::Known
			|| AmmoChannel->Availability == ECFUIViewAvailability::KnownZero)
		&& AmmoChannel->bHasCurrentValue
		&& AmmoChannel->bHasMaximumValue;

	// [v1.9.0] 실제 finite Ammo Snapshot이 0 또는 비0 현재값을 제공하는 상태인지 여부입니다.
	const bool bAmmoKnown = bUseAmmoResourceChannel
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::Known
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::KnownZero;
	if (!bAmmoKnown)
	{
		return false;
	}

	// [v1.9.0] WeaponPanel Primary 왼쪽에 표시할 실제 현재 장전량입니다.
	const int32 LoadedAmmoCount = bUseAmmoResourceChannel
		? FMath::Max(FMath::RoundToInt(AmmoChannel->CurrentValue), 0)
		: FMath::Max(WeaponViewData.LoadedAmmoCount, 0);

	// [v1.9.0] WeaponPanel Primary 오른쪽에 표시할 실제 탄창 총 용량입니다.
	const int32 MagazineCapacity = bUseAmmoResourceChannel
		? FMath::Max(FMath::RoundToInt(AmmoChannel->MaximumValue), 0)
		: FMath::Max(WeaponViewData.MagazineCapacity, 0);
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

		// [v1.9.0] 실제 ReserveAmmo 공통 Resource Channel입니다. 없으면 legacy ReserveAmmoCount를 사용합니다.
	const FCFWeaponResourceHUDData* ReserveAmmoChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::ReserveAmmo);

	// [v1.9.0] Resource Channel이 실제 Reserve current 값을 제공하는지 여부입니다.
	const bool bUseReserveAmmoResourceChannel = ReserveAmmoChannel
		&& (ReserveAmmoChannel->Availability == ECFUIViewAvailability::Known
			|| ReserveAmmoChannel->Availability == ECFUIViewAvailability::KnownZero)
		&& ReserveAmmoChannel->bHasCurrentValue;

	// [v1.9.0] Primary Ammo와 동일한 finite Runtime 가용 상태에서만 Reserve 숫자를 노출합니다.
	const bool bAmmoKnown = bUseReserveAmmoResourceChannel
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::Known
		|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::KnownZero;
	if (!bAmmoKnown)
	{
		return false;
	}

	// [v1.9.0] 같은 AmmoId의 차량 공유 예비량을 라벨 없이 숫자로만 표시합니다.
	const int32 ReserveAmmoCount = bUseReserveAmmoResourceChannel
		? FMath::Max(FMath::RoundToInt(ReserveAmmoChannel->CurrentValue), 0)
		: FMath::Max(WeaponViewData.ReserveAmmoCount, 0);
	OutReserveAmmoText = FText::AsNumber(ReserveAmmoCount);
	return true;
}

// [v1.9.0] Sequence 외 Weapon 상태를 Resource Channel 우선 Reload > NoAmmo > Cooldown/Ready 순서의 문구와 진행률로 변환합니다.
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

	// [v1.9.0] 실제 Reload 공통 Resource Channel입니다. 있으면 Reload 상태의 authoritative Presentation Source입니다.
	const FCFWeaponResourceHUDData* ReloadChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::Reload);

		// [v1.9.0] 실제 Ammo 공통 Resource Channel입니다. 있으면 NoAmmo 판정의 authoritative Presentation Source입니다.
	const FCFWeaponResourceHUDData* AmmoChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::Ammo);

	// [v1.16.0] 실제 per-weapon Charge 공통 Resource Channel입니다. 있으면 충전 부족 FireState의 authoritative Presentation Source입니다.
	const FCFWeaponResourceHUDData* ChargeChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::WeaponCharge);

		// [v1.14.0] 실제 per-weapon Heat 공통 Resource Channel입니다. 있으면 과열 FireState의 authoritative Presentation Source입니다.
	const FCFWeaponResourceHUDData* HeatChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::Heat);

	// [v1.9.0] 실제 Cooldown 공통 Resource Channel입니다. 있으면 Cooldown/READY의 authoritative Presentation Source입니다.
	const FCFWeaponResourceHUDData* CooldownChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::Cooldown);

	// [v1.9.0] Resource Channel 또는 legacy finite Ammo가 상태 판단에 사용할 수 있는지 여부입니다.
	const bool bAmmoKnown = AmmoChannel
		? (AmmoChannel->Availability == ECFUIViewAvailability::Known
			|| AmmoChannel->Availability == ECFUIViewAvailability::KnownZero)
		: (WeaponViewData.AmmoAvailability == ECFUIViewAvailability::Known
			|| WeaponViewData.AmmoAvailability == ECFUIViewAvailability::KnownZero);

	// [v1.9.0] Resource Channel이 실제 진행 중 Reload를 제공하는지 여부입니다.
	const bool bResourceReloadActive = ReloadChannel
		&& (ReloadChannel->Availability == ECFUIViewAvailability::Known
			|| ReloadChannel->Availability == ECFUIViewAvailability::KnownZero)
		&& ReloadChannel->bIsActive
		&& ReloadChannel->bHasMaximumValue
		&& ReloadChannel->bHasRemainingTimeSeconds;

	// [v1.9.0] Resource Channel이 없을 때만 기존 ReloadState를 fallback으로 사용합니다.
	const bool bLegacyReloadActive = !ReloadChannel
		&& bAmmoKnown
		&& WeaponViewData.ReloadState == ECFWeaponReloadState::Reloading;
	if (bResourceReloadActive || bLegacyReloadActive)
	{
		// [v1.9.0] Runtime이 제공한 실제 Reload 전체 시간입니다.
		const float ReloadDurationSeconds = bResourceReloadActive
			? FMath::Max(ReloadChannel->MaximumValue, 0.0f)
			: FMath::Max(WeaponViewData.ReloadDurationSeconds, 0.0f);

		// [v1.9.0] Runtime이 제공한 실제 Reload 남은 시간입니다.
		const float RemainingReloadTimeSeconds = bResourceReloadActive
			? FMath::Max(ReloadChannel->RemainingTimeSeconds, 0.0f)
			: FMath::Max(WeaponViewData.RemainingReloadTimeSeconds, 0.0f);

		// [v1.9.0] 10초 미만은 0.1초, 10초 이상은 정수 초로 읽기 쉽게 표시한 남은 시간입니다.
		const FString RemainingReloadTimeText = RemainingReloadTimeSeconds < 10.0f
			? FString::Printf(TEXT("%.1f"), RemainingReloadTimeSeconds)
			: FString::Printf(TEXT("%.0f"), RemainingReloadTimeSeconds);

		// [v1.9.0] 전체 Reload 시간도 같은 표시 정밀도 규칙으로 만든 문자열입니다.
		const FString ReloadDurationText = ReloadDurationSeconds < 10.0f
			? FString::Printf(TEXT("%.1f"), ReloadDurationSeconds)
			: FString::Printf(TEXT("%.0f"), ReloadDurationSeconds);
		OutStatusText = FText::FromString(FString::Printf(
			TEXT("RELOAD %s / %s s"),
			*RemainingReloadTimeText,
			*ReloadDurationText));
		OutStatusProgress = bResourceReloadActive && ReloadChannel->bHasNormalizedValue
			? FMath::Clamp(ReloadChannel->NormalizedValue, 0.0f, 1.0f)
			: (ReloadDurationSeconds > KINDA_SMALL_NUMBER
				? 1.0f - FMath::Clamp(RemainingReloadTimeSeconds / ReloadDurationSeconds, 0.0f, 1.0f)
				: 1.0f);
		return true;
	}

	// [v1.9.0] Resource Ammo가 있으면 실제 bBlocksFire를, 없으면 legacy CurrentOnboardAmmoCount를 사용합니다.
	const bool bNoAmmo = bAmmoKnown && (AmmoChannel
		? AmmoChannel->bBlocksFire
		: WeaponViewData.CurrentOnboardAmmoCount <= 0);
		if (bNoAmmo)
	{
		OutStatusText = FText::FromString(TEXT("NO AMMO"));
		OutStatusProgress = 0.0f;
		return true;
	}

		// [v1.16.0] 실제 WeaponCharge Runtime이 현재 충전 부족으로 발사를 막는지 여부입니다. Reload/NoAmmo 다음, Heat/Cooldown 전에 판정합니다.
	const bool bWeaponChargeInsufficient = ChargeChannel
		&& (ChargeChannel->Availability == ECFUIViewAvailability::Known
			|| ChargeChannel->Availability == ECFUIViewAvailability::KnownZero)
		&& ChargeChannel->bBlocksFire;
	if (bWeaponChargeInsufficient)
	{
		OutStatusText = FText::FromString(TEXT("NO CHARGE"));
		OutStatusProgress = ChargeChannel->bHasNormalizedValue
			? FMath::Clamp(ChargeChannel->NormalizedValue, 0.0f, 1.0f)
			: 0.0f;
		return true;
	}

	// [v1.14.0] 실제 Heat Runtime이 현재 과열로 발사를 막는지 여부입니다. Reload/NoAmmo/NoCharge 다음, 기존 Cooldown/READY 전에만 새 상태를 삽입합니다.
	const bool bWeaponOverheated = HeatChannel
		&& (HeatChannel->Availability == ECFUIViewAvailability::Known
			|| HeatChannel->Availability == ECFUIViewAvailability::KnownZero)
		&& HeatChannel->bBlocksFire;
	if (bWeaponOverheated)
	{
		OutStatusText = FText::FromString(TEXT("OVERHEATED"));
		OutStatusProgress = HeatChannel->bHasNormalizedValue
			? FMath::Clamp(HeatChannel->NormalizedValue, 0.0f, 1.0f)
			: 1.0f;
		return true;
	}

	// [v1.9.0] Resource Channel 또는 legacy Weapon/Volley Cooldown이 실제 Runtime 값으로 제공되는지 여부입니다.
	const bool bCooldownKnown = CooldownChannel
		? ((CooldownChannel->Availability == ECFUIViewAvailability::Known
				|| CooldownChannel->Availability == ECFUIViewAvailability::KnownZero)
			&& CooldownChannel->bHasMaximumValue
			&& CooldownChannel->MaximumValue > KINDA_SMALL_NUMBER
			&& CooldownChannel->bHasRemainingTimeSeconds)
		: (WeaponViewData.CooldownDurationSeconds > KINDA_SMALL_NUMBER
			&& (WeaponViewData.CooldownAvailability == ECFUIViewAvailability::Known
				|| WeaponViewData.CooldownAvailability == ECFUIViewAvailability::KnownZero));
	if (!bCooldownKnown)
	{
		return false;
	}

	// [v1.9.0] 현재 Weapon/Volley Cooldown의 실제 전체 시간입니다.
	const float CooldownDurationSeconds = CooldownChannel
		? FMath::Max(CooldownChannel->MaximumValue, 0.0f)
		: FMath::Max(WeaponViewData.CooldownDurationSeconds, 0.0f);

	// [v1.9.0] 현재 Weapon/Volley Cooldown의 실제 남은 시간입니다.
	const float RemainingCooldownSeconds = CooldownChannel
		? FMath::Max(CooldownChannel->RemainingTimeSeconds, 0.0f)
		: FMath::Max(WeaponViewData.RemainingCooldownSeconds, 0.0f);
	if (RemainingCooldownSeconds <= KINDA_SMALL_NUMBER)
	{
		OutStatusText = FText::FromString(TEXT("READY"));
		OutStatusProgress = 1.0f;
		return true;
	}

	OutStatusText = RemainingCooldownSeconds < 10.0f
		? FText::FromString(FString::Printf(TEXT("%.1f s"), RemainingCooldownSeconds))
		: FText::FromString(FString::Printf(TEXT("%.0f s"), RemainingCooldownSeconds));
	OutStatusProgress = CooldownChannel && CooldownChannel->bHasNormalizedValue
		? FMath::Clamp(CooldownChannel->NormalizedValue, 0.0f, 1.0f)
		: 1.0f - FMath::Clamp(
			RemainingCooldownSeconds / CooldownDurationSeconds,
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

// [v1.16.0] raw ResourceChannels를 직접 Visual Row로 사용하지 않고 Compact Primary/Secondary/FireState Presentation Entry로 Projection합니다.
void UCFHUDPresenter::BuildWeaponResourceEntries(
	const FCFWeaponHUDData& WeaponViewData,
	TArray<FCFWeaponResourcePresentationEntry>& OutPresentationEntries)
{
	OutPresentationEntries.Reset();

	// [v1.10.0] 기존 LauncherSequenceRevision lifecycle이 이번 ViewData 주기에 실제 Launcher 표현을 제공하는지 나타냅니다.
	FText LauncherSequenceText;
	// [v1.10.0] 기존 Launcher Resolver가 제공한 실제 0~1 Sequence 진행률입니다.
	float LauncherSequenceProgress = 0.0f;
	// [v1.10.0] Active 또는 terminal Snapshot 1회가 현재 Compact Primary를 소유하는지 나타냅니다.
	const bool bShowLauncherSequence = ResolveLauncherSequenceDisplay(
		WeaponViewData,
		LauncherSequenceText,
		LauncherSequenceProgress);

	// [v1.10.0] 기존 Ammo Resolver가 승인된 Loaded / MagazineCapacity 문자열을 제공할 수 있는지 나타냅니다.
	FText AmmoText;
	// [v1.10.0] finite Ammo가 현재 Compact Resource 표현에 사용 가능한지 나타냅니다.
	const bool bShowAmmo = ResolveAmmoPresentation(WeaponViewData, AmmoText);

	// [v1.16.0] 실제 WeaponCharge Runtime이 제공한 현재 Charge Percent 채널입니다. VehicleBattery나 정적 WeaponData 값은 여기서 읽지 않습니다.
	const FCFWeaponResourceHUDData* ChargeChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::WeaponCharge);
	// [v1.16.0] 실제 Charge Percent를 Compact Resource로 표시할 수 있는지 나타냅니다.
	const bool bShowCharge = ChargeChannel
		&& ChargeChannel->bIsVisible
		&& ChargeChannel->bHasNormalizedValue
		&& (ChargeChannel->Availability == ECFUIViewAvailability::Known
			|| ChargeChannel->Availability == ECFUIViewAvailability::KnownZero);

	// [v1.16.0] Compact Secondary 최대 2 계약을 유지하기 위해 현재 사용 중인 Secondary 수를 추적합니다.
	int32 SecondaryEntryCount = 0;

	if (bShowLauncherSequence)
	{
		// [v1.10.0] Launcher Active/terminal Snapshot을 Compact Primary로 승격한 최종 Presentation Entry입니다.
		FCFWeaponResourcePresentationEntry LauncherPrimaryEntry;
		LauncherPrimaryEntry.Role = ECFWeaponResourcePresentationRole::Primary;
		LauncherPrimaryEntry.SourceChannelType = ECFWeaponResourceChannelType::LauncherSequence;
		LauncherPrimaryEntry.DisplayMode = ECFWeaponResourceDisplayMode::Sequence;
		LauncherPrimaryEntry.DisplayText = LauncherSequenceText;
		LauncherPrimaryEntry.bHasProgress = true;
		LauncherPrimaryEntry.Progress01 = FMath::Clamp(LauncherSequenceProgress, 0.0f, 1.0f);
		OutPresentationEntries.Add(LauncherPrimaryEntry);

		if (bShowAmmo)
		{
			// [v1.10.0] Launcher가 Primary를 소유하는 동안에도 기존 Ammo 정보를 잃지 않도록 Secondary로 내린 Entry입니다.
			FCFWeaponResourcePresentationEntry AmmoSecondaryEntry;
			AmmoSecondaryEntry.Role = ECFWeaponResourcePresentationRole::Secondary;
			AmmoSecondaryEntry.SourceChannelType = ECFWeaponResourceChannelType::Ammo;
			AmmoSecondaryEntry.DisplayMode = ECFWeaponResourceDisplayMode::CountPair;
			AmmoSecondaryEntry.DisplayText = AmmoText;
			AmmoSecondaryEntry.bHasProgress = false;
			AmmoSecondaryEntry.Progress01 = 0.0f;
			OutPresentationEntries.Add(AmmoSecondaryEntry);
			++SecondaryEntryCount;
		}
	}
	else if (bShowAmmo)
	{
		// [v1.10.0] 일반 finite Ammo 무기의 승인된 Loaded / Capacity를 Compact Primary로 유지하는 Entry입니다.
		FCFWeaponResourcePresentationEntry AmmoPrimaryEntry;
		AmmoPrimaryEntry.Role = ECFWeaponResourcePresentationRole::Primary;
		AmmoPrimaryEntry.SourceChannelType = ECFWeaponResourceChannelType::Ammo;
		AmmoPrimaryEntry.DisplayMode = ECFWeaponResourceDisplayMode::CountPair;
		AmmoPrimaryEntry.DisplayText = AmmoText;
		AmmoPrimaryEntry.bHasProgress = false;
		AmmoPrimaryEntry.Progress01 = 0.0f;
		OutPresentationEntries.Add(AmmoPrimaryEntry);
	}
	else if (bShowCharge)
	{
		// [v1.16.0] Ammo/Launcher가 없는 Charge 기반 무기는 실제 내부 Charge를 Compact Primary로 사용합니다.
		FCFWeaponResourcePresentationEntry ChargePrimaryEntry;
		ChargePrimaryEntry.Role = ECFWeaponResourcePresentationRole::Primary;
		ChargePrimaryEntry.SourceChannelType = ECFWeaponResourceChannelType::WeaponCharge;
		ChargePrimaryEntry.DisplayMode = ECFWeaponResourceDisplayMode::Percent;
		ChargePrimaryEntry.DisplayText = FText::FromString(FString::Printf(
			TEXT("CHARGE %.0f%%"),
			FMath::Clamp(ChargeChannel->NormalizedValue, 0.0f, 1.0f) * 100.0f));
		ChargePrimaryEntry.bHasProgress = true;
		ChargePrimaryEntry.Progress01 = FMath::Clamp(ChargeChannel->NormalizedValue, 0.0f, 1.0f);
		OutPresentationEntries.Add(ChargePrimaryEntry);
	}

	if (bShowCharge && (bShowLauncherSequence || bShowAmmo) && SecondaryEntryCount < 2)
	{
		// [v1.16.0] Launcher 또는 Ammo가 Primary를 소유할 때 실제 Charge를 Compact Secondary로 유지합니다.
		FCFWeaponResourcePresentationEntry ChargeSecondaryEntry;
		ChargeSecondaryEntry.Role = ECFWeaponResourcePresentationRole::Secondary;
		ChargeSecondaryEntry.SourceChannelType = ECFWeaponResourceChannelType::WeaponCharge;
		ChargeSecondaryEntry.DisplayMode = ECFWeaponResourceDisplayMode::Percent;
		ChargeSecondaryEntry.DisplayText = FText::FromString(FString::Printf(
			TEXT("CHARGE %.0f%%"),
			FMath::Clamp(ChargeChannel->NormalizedValue, 0.0f, 1.0f) * 100.0f));
		ChargeSecondaryEntry.bHasProgress = true;
		ChargeSecondaryEntry.Progress01 = FMath::Clamp(ChargeChannel->NormalizedValue, 0.0f, 1.0f);
		OutPresentationEntries.Add(ChargeSecondaryEntry);
		++SecondaryEntryCount;
	}

	// [v1.14.0] 실제 Heat Runtime이 제공한 현재 Heat Percent 채널입니다. 정적 WeaponData 값은 여기서 읽지 않습니다.
	const FCFWeaponResourceHUDData* HeatChannel = FindWeaponResourceChannel(
		WeaponViewData,
		ECFWeaponResourceChannelType::Heat);
	// [v1.14.0] 실제 Heat Percent를 Compact Resource로 표시할 수 있는지 나타냅니다.
	const bool bShowHeat = HeatChannel
		&& HeatChannel->bIsVisible
		&& HeatChannel->bHasNormalizedValue
		&& (HeatChannel->Availability == ECFUIViewAvailability::Known
			|| HeatChannel->Availability == ECFUIViewAvailability::KnownZero);
	if (bShowHeat && SecondaryEntryCount < 2)
	{
		// [v1.16.0] Charge와 함께 있어도 Compact Secondary 최대 2 계약을 넘지 않는 실제 Heat Entry입니다.
		FCFWeaponResourcePresentationEntry HeatSecondaryEntry;
		HeatSecondaryEntry.Role = ECFWeaponResourcePresentationRole::Secondary;
		HeatSecondaryEntry.SourceChannelType = ECFWeaponResourceChannelType::Heat;
		HeatSecondaryEntry.DisplayMode = ECFWeaponResourceDisplayMode::Percent;
		HeatSecondaryEntry.DisplayText = FText::FromString(FString::Printf(
			TEXT("HEAT %.0f%%"),
			FMath::Clamp(HeatChannel->NormalizedValue, 0.0f, 1.0f) * 100.0f));
		HeatSecondaryEntry.bHasProgress = true;
		HeatSecondaryEntry.Progress01 = FMath::Clamp(HeatChannel->NormalizedValue, 0.0f, 1.0f);
		OutPresentationEntries.Add(HeatSecondaryEntry);
		++SecondaryEntryCount;
	}

	// [v1.16.0] Reload > NoAmmo > NoCharge > Overheated > Cooldown/READY 우선순위를 단일 Compact FireState로 Projection합니다.
	FText WeaponStatusText;
	// [v1.10.0] 기존 Weapon Status Resolver가 제공하는 실제 0~1 상태 진행률입니다.
	float WeaponStatusProgress = 0.0f;
	// [v1.10.0] Launcher 표시 중 일반 상태 억제를 포함해 현재 FireState를 표시할지 나타냅니다.
	const bool bShowWeaponStatus = ResolveWeaponStatusPresentation(
		WeaponViewData,
		bShowLauncherSequence,
		WeaponStatusText,
		WeaponStatusProgress);
	if (bShowWeaponStatus)
	{
		// [v1.16.0] Resource별 복합 우선순위를 단일 Compact FireState로 보존한 Entry입니다.
		FCFWeaponResourcePresentationEntry FireStateEntry;
		FireStateEntry.Role = ECFWeaponResourcePresentationRole::FireState;
		FireStateEntry.SourceChannelType = ECFWeaponResourceChannelType::None;
		FireStateEntry.DisplayMode = ECFWeaponResourceDisplayMode::Progress;
		FireStateEntry.DisplayText = WeaponStatusText;
		FireStateEntry.bHasProgress = true;
		FireStateEntry.Progress01 = FMath::Clamp(WeaponStatusProgress, 0.0f, 1.0f);
		OutPresentationEntries.Add(FireStateEntry);
	}
}


// [v1.15.0] 현재 선택 무기를 제외하고 Provider fixed order를 유지한 비선택 무기 Rail 최대 3개를 순번+DisplayName 또는 2개+overflow로 Projection합니다.
void UCFHUDPresenter::BuildWeaponRailEntries(
	const FCFWeaponHUDData& WeaponViewData,
	TArray<FCFWeaponRailPresentationEntry>& OutRailEntries)
{
	OutRailEntries.Reset();

	// [v1.15.0] 실제 Weapon Selection Runtime이 제공한 목록과 선택 인덱스를 사용할 수 있는 최소 조건입니다.
	const bool bSelectionAvailable = WeaponViewData.WeaponSelectionAvailability == ECFUIViewAvailability::Known
		&& WeaponViewData.SelectableWeapons.IsValidIndex(WeaponViewData.SelectedWeaponIndex);
	if (!bSelectionAvailable)
	{
		return;
	}

	// [v1.15.0] Provider fixed order를 유지하면서 현재 선택 무기만 제외한 원본 배열 인덱스 목록입니다.
	TArray<int32> UnselectedWeaponIndices;
	UnselectedWeaponIndices.Reserve(FMath::Max(WeaponViewData.SelectableWeapons.Num() - 1, 0));
	for (int32 WeaponIndex = 0; WeaponIndex < WeaponViewData.SelectableWeapons.Num(); ++WeaponIndex)
	{
		if (WeaponIndex != WeaponViewData.SelectedWeaponIndex)
		{
			UnselectedWeaponIndices.Add(WeaponIndex);
		}
	}

	if (UnselectedWeaponIndices.IsEmpty())
	{
		return;
	}

	// [v1.15.0] Rail 4개 이상에서 앞 2개 + overflow를 만들지, 최대 3개를 그대로 표시할지 결정하는 개수입니다.
	const int32 VisibleNamedWeaponCount = UnselectedWeaponIndices.Num() >= 4
		? 2
		: FMath::Min(UnselectedWeaponIndices.Num(), 3);
	for (int32 RailEntryIndex = 0; RailEntryIndex < VisibleNamedWeaponCount; ++RailEntryIndex)
	{
		// [v1.15.0] Provider fixed order에서 현재 Rail 항목이 가리키는 실제 selectable weapon 인덱스입니다.
		const int32 WeaponIndex = UnselectedWeaponIndices[RailEntryIndex];
		// [v1.15.0] 내부 ID 없이 실제 EquipmentPresetData.DisplayName 가용 상태와 선택 여부만 가진 HUD 항목입니다.
		const FCFWeaponSelectionHUDItem& SelectionItem = WeaponViewData.SelectableWeapons[WeaponIndex];
		// [v1.15.0] 실제 DisplayName이 없을 때 내부 ID를 노출하지 않기 위한 일반 Player-facing fallback입니다.
		const FString WeaponName = SelectionItem.DisplayNameAvailability == ECFUIViewAvailability::Known && !SelectionItem.DisplayName.IsEmpty()
			? SelectionItem.DisplayName.ToString()
			: FString(TEXT("WEAPON"));

		// [v1.15.0] 1-based Provider fixed ordinal과 실제 DisplayName만 결합한 최종 Rail 표시 항목입니다.
		FCFWeaponRailPresentationEntry RailEntry;
		RailEntry.DisplayText = FText::FromString(FString::Printf(TEXT("%02d  %s"), WeaponIndex + 1, *WeaponName));
		RailEntry.bOverflow = false;
		OutRailEntries.Add(RailEntry);
	}

	if (UnselectedWeaponIndices.Num() >= 4)
	{
		// [v1.15.0] 앞 두 비선택 무기 뒤에 숨겨진 실제 비선택 무기 수입니다.
		const int32 HiddenWeaponCount = UnselectedWeaponIndices.Num() - 2;
		// [v1.15.0] 숨겨진 수만 표시하고 특정 내부 identity를 암시하지 않는 overflow 항목입니다.
		FCFWeaponRailPresentationEntry OverflowEntry;
		OverflowEntry.DisplayText = FText::FromString(FString::Printf(TEXT("+%d"), HiddenWeaponCount));
		OverflowEntry.bOverflow = true;
		OutRailEntries.Add(OverflowEntry);
	}
}

// [v1.15.0] Weapon Header, Stage A Compact Resource와 truthful 비선택 Weapon Rail을 Production WeaponPanel 의미 슬롯에 적용합니다.
void UCFHUDPresenter::ApplyWeaponViewData(UUserWidget* RootWidget, const FCFWeaponHUDData& WeaponViewData)
{
	// [v1.11.0] 현재 ViewData를 표시할 Production WeaponPanel 인스턴스입니다.
	UUserWidget* WeaponPanel = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFWeaponPanel")));
	if (!WeaponPanel)
	{
		return;
	}

	// [v1.11.0] 내부 WeaponId를 Player-facing 이름으로 노출하지 않는 안전한 제목입니다.
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

	// [v1.11.0] WeaponPanel Header 우측에 라벨 없이 표시할 실제 차량 Reserve Ammo 숫자입니다.
	FText ReserveAmmoText;
	// [v1.11.0] ReserveAmmo가 실제 Runtime Source를 가져 Header owner에 표시 가능한지 나타냅니다.
	const bool bShowReserveAmmo = ResolveReserveAmmoPresentation(WeaponViewData, ReserveAmmoText);
	SetTextValue(WeaponPanel, FName(TEXT("Text_WeaponReserveAmmo")), ReserveAmmoText, bShowReserveAmmo);

	// [v1.11.0] 이번 ViewData에서 LauncherSequenceRevision lifecycle을 정확히 한 번 소비해 만든 Compact Resource Presentation 목록입니다.
	TArray<FCFWeaponResourcePresentationEntry> PresentationEntries;
	BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);

	// [v1.11.0] Compact Primary 슬롯에 적용할 최대 한 개의 Presentation Entry입니다.
	const FCFWeaponResourcePresentationEntry* PrimaryEntry = nullptr;
	// [v1.11.0] Compact Secondary A 슬롯에 적용할 첫 번째 보조 Presentation Entry입니다.
	const FCFWeaponResourcePresentationEntry* SecondaryEntryA = nullptr;
	// [v1.11.0] Compact Secondary B 슬롯에 적용할 두 번째 보조 Presentation Entry입니다.
	const FCFWeaponResourcePresentationEntry* SecondaryEntryB = nullptr;
	// [v1.11.0] Compact FireState 슬롯에 적용할 최대 한 개의 상태 Presentation Entry입니다.
	const FCFWeaponResourcePresentationEntry* FireStateEntry = nullptr;

	for (const FCFWeaponResourcePresentationEntry& PresentationEntry : PresentationEntries)
	{
		switch (PresentationEntry.Role)
		{
		case ECFWeaponResourcePresentationRole::Primary:
			if (!PrimaryEntry)
			{
				PrimaryEntry = &PresentationEntry;
			}
			break;
		case ECFWeaponResourcePresentationRole::Secondary:
			if (!SecondaryEntryA)
			{
				SecondaryEntryA = &PresentationEntry;
			}
			else if (!SecondaryEntryB)
			{
				SecondaryEntryB = &PresentationEntry;
			}
			break;
		case ECFWeaponResourcePresentationRole::FireState:
			if (!FireStateEntry)
			{
				FireStateEntry = &PresentationEntry;
			}
			break;
		default:
			break;
		}
	}

	// [v1.11.0] 의미 슬롯 하나에 Entry의 Text/Progress/Visibility를 일관되게 적용하는 Presentation-only helper입니다.
	auto ApplyPresentationEntry = [WeaponPanel](
		const FCFWeaponResourcePresentationEntry* PresentationEntry,
		const FName ContainerName,
		const FName TextName,
		const FName ProgressName)
	{
		// [v1.11.0] 실제 Entry와 Player-facing 문자열이 모두 존재할 때만 현재 의미 슬롯을 표시합니다.
		const bool bShowEntry = PresentationEntry && !PresentationEntry->DisplayText.IsEmpty();
		UCFHUDPresenter::SetNamedVisibility(WeaponPanel, ContainerName, bShowEntry);
		UCFHUDPresenter::SetTextValue(
			WeaponPanel,
			TextName,
			bShowEntry ? PresentationEntry->DisplayText : FText::GetEmpty(),
			bShowEntry);

		// [v1.11.0] Launcher/FireState처럼 실제 Progress가 있는 Entry에서만 ProgressBar를 표시합니다.
		const bool bShowProgress = bShowEntry && PresentationEntry->bHasProgress;
		UCFHUDPresenter::SetProgressValue(
			WeaponPanel,
			ProgressName,
			bShowProgress ? PresentationEntry->Progress01 : 0.0f,
			bShowProgress);
	};

	ApplyPresentationEntry(
		PrimaryEntry,
		FName(TEXT("HorizontalBox_PrimaryResource")),
		FName(TEXT("Text_WeaponPrimaryResource")),
		FName(TEXT("ProgressBar_PrimaryResource")));
	ApplyPresentationEntry(
		SecondaryEntryA,
		FName(TEXT("VerticalBox_SecondaryResourceA")),
		FName(TEXT("Text_WeaponSecondaryResourceA")),
		FName(TEXT("ProgressBar_SecondaryResourceA")));
	ApplyPresentationEntry(
		SecondaryEntryB,
		FName(TEXT("VerticalBox_SecondaryResourceB")),
		FName(TEXT("Text_WeaponSecondaryResourceB")),
		FName(TEXT("ProgressBar_SecondaryResourceB")));
	ApplyPresentationEntry(
		FireStateEntry,
		FName(TEXT("HorizontalBox_FireState")),
		FName(TEXT("Text_WeaponFireState")),
		FName(TEXT("ProgressBar_FireState")));

	// [v1.11.0] Secondary가 0개면 행 전체를 접고, 1개면 A가 Fill 전체 폭을 사용하며, 2개면 A/B가 같은 Fill 규칙으로 나뉩니다.
	const bool bShowSecondaryResources = SecondaryEntryA || SecondaryEntryB;
	SetNamedVisibility(WeaponPanel, FName(TEXT("HorizontalBox_SecondaryResources")), bShowSecondaryResources);

	// [v1.11.0] Primary/Secondary/FireState 중 하나라도 있을 때만 Resource Presentation 영역 자체를 유지합니다.
	const bool bShowResourcePresentation = PrimaryEntry || SecondaryEntryA || SecondaryEntryB || FireStateEntry;
	SetNamedVisibility(WeaponPanel, FName(TEXT("VerticalBox_ResourcePresentation")), bShowResourcePresentation);

		// [v1.15.0] 현재 선택을 제외한 실제 비선택 무기만 fixed-order 최대 3개 슬롯으로 만든 Rail Presentation입니다.
	TArray<FCFWeaponRailPresentationEntry> RailEntries;
	BuildWeaponRailEntries(WeaponViewData, RailEntries);

	// [v1.15.0] Production Rail의 세 고정 Text Tile 이름입니다. Widget 자체는 Gameplay 배열을 순회하거나 동적으로 생성하지 않습니다.
	const FName RailContainerNames[] =
	{
		FName(TEXT("SizeBox_WeaponRail1")),
		FName(TEXT("SizeBox_WeaponRail2")),
		FName(TEXT("SizeBox_WeaponRail3"))
	};
	// [v1.15.0] 세 고정 Tile 내부에 Presenter가 완성한 순번+DisplayName/+N만 표시할 Text 이름입니다.
	const FName RailTextNames[] =
	{
		FName(TEXT("Text_WeaponRail1")),
		FName(TEXT("Text_WeaponRail2")),
		FName(TEXT("Text_WeaponRail3"))
	};
	for (int32 RailSlotIndex = 0; RailSlotIndex < UE_ARRAY_COUNT(RailContainerNames); ++RailSlotIndex)
	{
		// [v1.15.0] 현재 고정 슬롯에 실제 Rail Entry가 존재하는지 여부입니다.
		const bool bShowRailSlot = RailEntries.IsValidIndex(RailSlotIndex) && !RailEntries[RailSlotIndex].DisplayText.IsEmpty();
		SetNamedVisibility(WeaponPanel, RailContainerNames[RailSlotIndex], bShowRailSlot);
		SetTextValue(
			WeaponPanel,
			RailTextNames[RailSlotIndex],
			bShowRailSlot ? RailEntries[RailSlotIndex].DisplayText : FText::GetEmpty(),
			bShowRailSlot);
	}

	// [v1.15.0] 선택 무기 하나뿐이거나 Selection Runtime이 없으면 Rail 행과 예약 높이를 모두 제거합니다.
	SetNamedVisibility(WeaponPanel, FName(TEXT("HorizontalBox_WeaponRail")), !RailEntries.IsEmpty());
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

// [v1.8.0] Sensor Provider 가용 상태를 Production RadarPanel 제목에 적용하되 정적 placeholder Contact는 실제 데이터로 노출하지 않습니다.
void UCFHUDPresenter::ApplyRadarViewData(UUserWidget* RootWidget, const FCFRadarHUDData& RadarViewData) const
{
	// [v1.8.0] Sensor Radar 상태를 표시할 Production Radar Panel입니다.
	UUserWidget* RadarPanel = FindNamedUserWidget(RootWidget, FName(TEXT("WBP_CFRadarPanel")));
	if (!RadarPanel)
	{
		return;
	}

	// [v1.8.0] 실제 Sensor Snapshot이 준비됐거나 준비됐지만 Contact가 0개인 상태입니다.
	const bool bRadarAvailable = RadarViewData.Availability == ECFUIViewAvailability::Known
		|| RadarViewData.Availability == ECFUIViewAvailability::KnownZero;
	SetTextValue(
		RadarPanel,
		FName(TEXT("Text_RadarTitle")),
		bRadarAvailable ? FText::FromString(TEXT("RADAR")) : FText::FromString(TEXT("RADAR — UNAVAILABLE")),
		true);

	// [v1.8.0] 이 Canvas의 기존 Image들은 디자인용 정적 placeholder이며 FCFRadarHUDData.Contacts를 동적으로 렌더링하는 Widget이 아닙니다.
	// Radar 표시 Range/Zoom과 동적 Contact 생성 계약 전에는 Sensor가 Known이어도 가짜 Contact를 보이지 않습니다.
	SetNamedVisibility(RadarPanel, FName(TEXT("CanvasPanel_RadarContacts")), false);
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
