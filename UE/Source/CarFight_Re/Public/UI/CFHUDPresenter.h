// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.0
// Date: 2026-08-13
// Description: CF-FQ-032 UI-P0-03 + CF-FQ-031 AMMO-P0-06 HUD ViewData → Production Widget Presenter
// Scope: Gameplay 참조 없이 FCFInGameUIViewData만 소비해 WBP_CFInGameHUD의 Weapon Ammo·Reserve·Reload·공통 Launcher Presentation lifecycle을 갱신합니다.
// Changelog:
// - v1.7.0: 폐기된 Salvo 전용 0.25/0.75초 Hold를 제거. LauncherSequenceRevision 기반 공통 Active→Terminal Snapshot→Cooldown/READY lifecycle로 Ripple·Salvo를 통일하고 FirePattern은 표시 문구에만 사용.
// - v1.6.0: [폐기 이력] TestMap_DRSalvo USER PIE에서 0.25초 완료 Hold가 실제 인지에 너무 짧아 Salvo Presentation 최소 유지시간을 0.75초로 확대했던 접근.
// - v1.5.0: [폐기 이력] 같은 Game Thread 처리 묶음에서 완료되는 동시 Salvo를 위해 Presentation-only 0.25초 Hold를 추가했던 접근.
// - v1.4.0: Primary Ammo를 `Loaded / MagazineCapacity`로 교체하고 label-less Reserve 숫자를 별도 Presentation으로 분리. Immediate/CurrentUsable은 상태 판정용으로 유지.
// - v1.3.0: 실제 Ammo를 `ImmediateUsable | CurrentUsable`로 표시하고 기존 상태 행을 Sequence > Reload > NoAmmo > Cooldown/Ready 우선순위로 적용.
// - v1.2.0: 정상 Ripple/Salvo Sequence를 WeaponPanel Primary Action으로 표시하고 Sequence 중 Cooldown/READY 중복을 숨기며 AlertFeed 정상 Sequence 표시를 제거.
// - v1.1.0: Alert 배열 순번 대신 AlertKey 의미로 Warning/Launcher Production 슬롯을 결정해 단독 Ripple Alert가 전용 슬롯에 표시되도록 교정.
// - v1.0.0: Vehicle/Defense/Weapon/Target/Radar/Alert Production Widget 적용과 Unavailable Collapse를 최초 구현.
// Migration:
// - Production WBP_CFInGameHUD Parent는 CFStyledWidgetBase를 그대로 유지합니다.
// - Presenter는 Pawn, Actor Component, Gameplay DataAsset을 Cast하거나 탐색하지 않습니다.
// - D1-11 Designer Mock 값은 Runtime ViewData가 연결되면 실제 값 또는 Unavailable 표현으로 대체됩니다.
// - Salvo 전용 시간 Hold 상태는 제거됐으며 Ripple·Salvo 모두 LauncherSequenceRevision 기반 동일 lifecycle을 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "UI/CFHUDViewData.h"
#include "UObject/Object.h"
#include "CFHUDPresenter.generated.h"

class UCFHUDDataProvider;
class UCFStyledWidgetBase;
class UProgressBar;
class UTextBlock;
class UUserWidget;
class UWidget;

/**
 * HUD Provider 이벤트를 Production Widget의 의미 요소에 적용하는 순수 Presentation Adapter입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFHUDPresenter : public UObject
{
	GENERATED_BODY()

public:
	// [v1.0.0] HUDDataProvider 변경 이벤트를 구독하고 현재 ViewData를 준비합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Presenter", meta=(DisplayName="HUD Presenter 초기화", ToolTip="HUDDataProvider의 ViewData 변경 이벤트만 구독합니다. Gameplay Pawn이나 Component는 조회하지 않습니다."))
	bool InitializePresenter(UCFHUDDataProvider* InDataProvider);

	// [v1.0.0] Provider 이벤트와 Production Widget 참조를 모두 해제합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Presenter", meta=(DisplayName="HUD Presenter 종료", ToolTip="Provider 이벤트와 Production Widget 연결을 제거합니다."))
	void ShutdownPresenter();

	// [v1.0.0] 현재 Production WBP_CFInGameHUD 인스턴스를 Presenter 출력 대상으로 연결합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Presenter", meta=(DisplayName="Production HUD Widget 설정", ToolTip="CFStyledWidgetBase 계열 Production HUD를 ViewData 출력 대상으로 설정합니다. Gameplay 조회는 수행하지 않습니다."))
	void SetProductionWidget(UCFStyledWidgetBase* InProductionWidget);

	// [v1.0.0] 현재 Presenter가 연결한 Production Widget을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Presenter", meta=(DisplayName="Production HUD Widget 반환", ToolTip="현재 ViewData를 적용 중인 Production HUD Widget을 반환합니다."))
	UCFStyledWidgetBase* GetProductionWidget() const { return ProductionWidget.Get(); }

	// [v1.7.0] Active 여부와 무관한 유효 Launcher Snapshot을 Pattern 문구와 0~1 진행률로 변환합니다.
	static bool ResolveLauncherSequenceSnapshotPresentation(
		const FCFWeaponHUDData& WeaponViewData,
		FText& OutSequenceText,
		float& OutSequenceProgress);

	// [v1.2.0] 실제 Active Launcher ViewData를 WeaponPanel Sequence 문구와 0~1 진행률로 변환합니다.
	static bool ResolveLauncherSequencePresentation(
		const FCFWeaponHUDData& WeaponViewData,
		FText& OutSequenceText,
		float& OutSequenceProgress);

	// [v1.7.0] LauncherSequenceRevision을 기준으로 Active, terminal Snapshot 1회, 이후 Weapon Status 전환을 공통 처리합니다.
	bool ResolveLauncherSequenceDisplay(
		const FCFWeaponHUDData& WeaponViewData,
		FText& OutSequenceText,
		float& OutSequenceProgress);

	// [v1.4.0] 실제 finite Ammo ViewData를 승인된 `Loaded / MagazineCapacity` Primary 문구로 변환합니다.
	static bool ResolveAmmoPresentation(
		const FCFWeaponHUDData& WeaponViewData,
		FText& OutAmmoText);

	// [v1.4.0] 실제 finite Ammo ViewData의 ReserveAmmoCount를 label-less 우상단 숫자로 변환합니다.
	static bool ResolveReserveAmmoPresentation(
		const FCFWeaponHUDData& WeaponViewData,
		FText& OutReserveAmmoText);

	// [v1.3.0] Sequence 외 Weapon 상태를 Reload > NoAmmo > Cooldown/Ready 우선순위의 상태 문구와 진행률로 변환합니다.
	static bool ResolveWeaponStatusPresentation(
		const FCFWeaponHUDData& WeaponViewData,
		bool bLauncherSequenceVisible,
		FText& OutStatusText,
		float& OutStatusProgress);

private:
	// [v1.0.0] Provider의 새 ViewData를 현재 Production Widget에 적용합니다.
	UFUNCTION()
	void HandleHUDViewDataChanged(FCFInGameUIViewData ViewData);

	// [v1.0.0] 전체 ViewData를 Vehicle/Weapon/Target/Radar/Alert 영역별로 적용합니다.
	void ApplyViewData(const FCFInGameUIViewData& ViewData);

	// [v1.0.0] Vehicle 속도와 Defense 상태를 Production VehiclePanel에 적용합니다.
	void ApplyVehicleAndDefenseViewData(UUserWidget* RootWidget, const FCFVehicleHUDData& VehicleViewData, const FCFDefenseHUDData& DefenseViewData) const;

	// [v1.7.0] Weapon·Ammo·Launcher의 공통 Presentation 우선순위를 Production WeaponPanel에 적용합니다.
	void ApplyWeaponViewData(UUserWidget* RootWidget, const FCFWeaponHUDData& WeaponViewData);

	// [v1.7.0] Pawn·Widget·Weapon 전환에서 이전 Launcher Presentation lifecycle 상태를 초기화합니다.
	void ResetLauncherPresentationLifecycle();

	// [v1.0.0] 선택 Target 공개 정보를 Production TargetPanel에 적용합니다.
	void ApplyTargetViewData(UUserWidget* RootWidget, const FCFTargetHUDData& TargetViewData) const;

	// [v1.0.0] Sensor Provider 부재 또는 Contact 상태를 Production RadarPanel에 적용합니다.
	void ApplyRadarViewData(UUserWidget* RootWidget, const FCFRadarHUDData& RadarViewData) const;

	// [v1.2.0] 현재 상태 기반 전역 Warning/Critical Alert를 Production AlertFeed에 적용합니다.
	void ApplyAlertViewData(UUserWidget* RootWidget, const FCFCombatAlertViewData& AlertViewData) const;

	// [v1.0.0] 지정 UserWidget의 WidgetTree에서 이름으로 자식 Widget을 찾습니다.
	static UWidget* FindNamedWidget(UUserWidget* ParentWidget, FName WidgetName);

	// [v1.0.0] 지정 UserWidget의 WidgetTree에서 이름으로 중첩 UserWidget을 찾습니다.
	static UUserWidget* FindNamedUserWidget(UUserWidget* ParentWidget, FName WidgetName);

	// [v1.0.0] 지정 이름 TextBlock의 Text와 표시 상태를 함께 적용합니다.
	static void SetTextValue(UUserWidget* ParentWidget, FName WidgetName, const FText& Text, bool bVisible = true);

	// [v1.0.0] 지정 이름 ProgressBar의 0~1 비율과 표시 상태를 함께 적용합니다.
	static void SetProgressValue(UUserWidget* ParentWidget, FName WidgetName, float Percent, bool bVisible = true);

	// [v1.0.0] 지정 이름 Widget의 Visibility를 Collapsed 또는 HitTestInvisible로 적용합니다.
	static void SetNamedVisibility(UUserWidget* ParentWidget, FName WidgetName, bool bVisible);

	// [v1.0.0] Provider ViewData 변경 이벤트를 구독할 강한 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFHUDDataProvider> DataProvider = nullptr;

	// [v1.0.0] 현재 Production HUD 인스턴스의 약한 참조입니다.
	TWeakObjectPtr<UCFStyledWidgetBase> ProductionWidget;

	// [v1.7.0] 현재 Presenter가 Active Launcher Presentation lifecycle을 추적 중인지 나타냅니다.
	bool bLauncherSequencePresentationActive = false;

	// [v1.7.0] 실제 Launcher terminal 이벤트의 최종 Snapshot을 이번 ViewData 주기에서 표시했는지 나타냅니다.
	bool bLauncherTerminalPresentationShown = false;

	// [v1.7.0] Presentation 상태가 다른 무기로 새지 않도록 현재 lifecycle의 WeaponId를 보존합니다.
	FName LauncherPresentationWeaponId = NAME_None;

	// [v1.7.0] 마지막으로 의미 있게 처리한 LauncherSequenceRevision입니다.
	int32 LastLauncherSequenceRevision = INDEX_NONE;

	// [v1.7.0] terminal 이벤트보다 먼저 들어오는 Ammo 부수 Refresh에서 마지막 Active 문구를 보존하는 Cache입니다.
	FText LastLauncherSequenceText;

	// [v1.7.0] 마지막 Active 또는 terminal Launcher 진행률 Cache입니다.
	float LastLauncherSequenceProgress = 0.0f;

	// [v1.7.0] Pawn Rebind 시 이전 차량의 Launcher Presentation lifecycle을 즉시 폐기하기 위한 마지막 BindingGeneration입니다.
	int32 LastAppliedBindingGeneration = INDEX_NONE;
};