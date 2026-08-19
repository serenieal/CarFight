// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.15.0
// Date: 2026-08-19
// Description: CF-FQ-032 UI-P0-06 truthful Weapon Rail + actual Weapon Charge/Heat + Dynamic Resource Visual Stage B + RPM Gauge Production Presenter
// Scope: Gameplay 참조 없이 FCFInGameUIViewData만 소비하고 실제 선택 무기는 Header/Selected Card에 유지하며 비선택 무기만 최대 3개의 이름 기반 Compact Rail로 Projection합니다.
// Changelog:
// - v1.15.0: UI-P0-06 Technical Complete 범위 교정에 맞춰 Current 주석을 동기화. WeaponCharge/Heat actual Resource를 소비하고 VehicleBattery만 future shared-power Runtime으로 남김.
// - v1.14.0: Applied Fitting 기반 SelectableWeapons fixed order에서 현재 선택을 제외한 비선택 무기만 Rail에 Projection. 1~3개는 순번+DisplayName, 4개 이상은 앞 2개 + `+N`이며 내부 ID/가짜 icon/resource summary를 사용하지 않음.
// - v1.13.0: 실제 Heat Resource Channel을 Compact Secondary와 Overheated FireState로 소비하는 계약을 추가. Battery·Charge 미구현, Launcher 단일 소비와 기존 Ammo/Reload/Cooldown 계약은 유지.
// - v1.12.0: 기존 WBP_CFSpeedGauge의 ProgressBar_RPMTick00~20을 explicit RPM Gauge ratio의 Production Visual sink로 사용. Redline unavailable/invalid이면 21 Tick fill을 0으로 reset해 stale/fallback 표시를 금지.
// - v1.11.0: UI-P0-06 explicit RPM Gauge Resolver를 추가. RedlineStartRPM을 0.85 위치, EngineMaxRPM을 1.0 끝점으로 piecewise 매핑하며 미설정/invalid 계약은 표시하지 않음.
// - v1.10.0: UI-P0-06 Dynamic Resource Visual Stage B. Production WeaponPanel은 한 ViewData 적용당 BuildWeaponResourceEntries 결과를 정확히 한 번 소비하고 Primary 1 + Secondary 최대 2 + FireState 1 의미 슬롯에 반영하는 계약으로 전환.
// - v1.9.0: UI-P0-06 Dynamic Resource Visual Stage A. Primary 최대 1 + Secondary 최대 2 + FireState 최대 1의 Presenter 전용 FCFWeaponResourcePresentationEntry 계약과 BuildWeaponResourceEntries Projection을 추가. ReserveAmmo는 Header owner로 유지하고 Production Asset은 변경하지 않음.
// - v1.8.0: UI-P0-06 additive ResourceChannels를 Ammo·Reserve·Reload·Cooldown·Launcher의 우선 Presentation Source로 사용하고 legacy 개별 필드 fallback을 보존. 실제 Heat 채널이 있을 때만 기존 Heat Row 표시 허용.
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
// - v1.8.0부터 Provider가 ResourceChannels를 제공하면 공통 채널을 우선 소비합니다. 채널이 없는 legacy ViewData는 기존 개별 필드 경로로 동일하게 동작합니다.
// - v1.9.0의 Presentation Projection은 raw ResourceChannels 배열 순서를 Visual Row 순서로 해석하지 않습니다. Launcher lifecycle과 Weapon Status 우선순위를 먼저 해석한 뒤 Compact 역할을 부여합니다.
// - ReserveAmmo는 Presentation Entry에 넣지 않고 기존 Header 우측 label-less owner를 유지합니다.
// - VehicleBattery는 실제 shared-power Runtime Provider가 생기기 전 Presentation Entry를 만들지 않습니다. WeaponCharge와 Heat는 actual Runtime Resource Channel이 있을 때만 Compact Resource/FireState로 Projection합니다.
// - BuildWeaponResourceEntries는 LauncherSequenceRevision lifecycle을 소비하는 C++ 전용 단일-적용 함수이며 Blueprint에서 별도 반복 호출하지 않습니다.
// - v1.10.0부터 ApplyWeaponViewData는 Resource Visual을 만들 때 BuildWeaponResourceEntries만 호출하고 Launcher/Ammo/Status Resolver를 별도로 다시 호출하지 않습니다.
// - v1.12.0부터 기존 SpeedGauge 21 Tick은 구조를 늘리지 않고 Runtime RPM fill sink로 사용합니다. RedlineStartRPM 미설정/invalid에서는 Tick 구조는 보존하되 fill=0으로 reset하며 EngineMaxRPM 비율 fallback을 만들지 않습니다.
// - ReserveAmmo는 계속 Header 우측 label-less owner입니다.
// - v1.14.0부터 Weapon Rail은 WeaponSelectionAvailability가 Known이고 SelectedWeaponIndex가 유효할 때만 비선택 무기를 표시합니다. 현재 선택 무기는 Rail에 중복하지 않습니다.
// - Rail은 실제 weapon icon source와 비선택 무기별 resource summary source가 없으므로 순번+EquipmentPresetData.DisplayName만 사용합니다. DisplayName이 없으면 내부 ID fallback 없이 일반 `WEAPON`만 표시합니다.

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
 * Compact WeaponPanel에서 한 Presentation Entry가 차지하는 의미 역할입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="무기 자원 표현 역할 (Weapon Resource Presentation Role)", ToolTip="Presenter가 raw ResourceChannels를 정리한 뒤 Primary, Secondary, FireState 중 어느 Compact 영역에 표시할지 나타냅니다."))
enum class ECFWeaponResourcePresentationRole : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	Primary UMETA(DisplayName="주 자원 (Primary)"),
	Secondary UMETA(DisplayName="보조 자원 (Secondary)"),
	FireState UMETA(DisplayName="발사 상태 (Fire State)")
};

/**
 * raw Weapon Resource Channel을 Compact WeaponPanel의 최종 의미 슬롯으로 정리한 Presenter 전용 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 자원 표현 항목 (Weapon Resource Presentation Entry)", ToolTip="Primary 최대 1, Secondary 최대 2, FireState 최대 1 계약으로 정리된 표시용 항목입니다. Gameplay Runtime을 다시 조회하거나 계산하지 않습니다."))
struct CARFIGHT_RE_API FCFWeaponResourcePresentationEntry
{
	GENERATED_BODY()

	// [v1.9.0] Compact WeaponPanel에서 이 항목이 차지하는 최종 의미 역할입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Presentation", meta=(DisplayName="표현 역할", ToolTip="Primary, Secondary 또는 FireState 중 Presenter가 확정한 한 역할입니다."))
	ECFWeaponResourcePresentationRole Role = ECFWeaponResourcePresentationRole::None;

	// [v1.9.0] 이 표시가 직접 대표하는 Resource Channel 종류이며 복합 FireState처럼 단일 채널로 축약할 수 없으면 None입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Presentation", meta=(DisplayName="원본 자원 채널", ToolTip="Ammo나 LauncherSequence처럼 직접 대응하는 Resource Channel입니다. Reload/NoAmmo/Cooldown 우선순위를 합성한 FireState는 None일 수 있습니다."))
	ECFWeaponResourceChannelType SourceChannelType = ECFWeaponResourceChannelType::None;

	// [v1.9.0] Visual Widget이 Text와 Progress를 어떤 기본 표현 종류로 다룰지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Presentation", meta=(DisplayName="표현 방식", ToolTip="CountPair, Sequence, Progress 등 Presenter가 확정한 표시 방식입니다."))
	ECFWeaponResourceDisplayMode DisplayMode = ECFWeaponResourceDisplayMode::None;

	// [v1.9.0] 기존 승인된 Ammo·Launcher·Weapon Status 포맷을 재사용해 Presenter가 완성한 Player-facing 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Presentation", meta=(DisplayName="표시 문자열", ToolTip="Widget이 Gameplay 값을 다시 포맷하지 않도록 Presenter가 완성한 Player-facing 문자열입니다."))
	FText DisplayText;

	// [v1.9.0] 이 항목이 실제 0~1 진행률을 함께 표시하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Presentation", meta=(DisplayName="진행률 존재", ToolTip="True일 때만 Progress01을 ProgressBar 표현에 사용합니다."))
	bool bHasProgress = false;

	// [v1.9.0] Launcher 또는 Weapon Status가 제공한 실제 0~1 표시 진행률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Presentation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="진행률", ToolTip="bHasProgress가 True일 때만 유효한 0~1 표시 진행률입니다."))
	float Progress01 = 0.0f;
};

/**
 * 비선택 무기 Compact Rail의 최대 3개 고정 시각 슬롯에 적용할 Presenter 전용 표시 항목입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 Rail 표현 항목 (Weapon Rail Presentation Entry)", ToolTip="현재 선택 무기를 제외한 실제 selectable weapon을 Provider 고정 순서대로 최대 3개 Rail 슬롯에 표시하기 위한 최종 문자열입니다. 내부 ID와 가짜 아이콘을 포함하지 않습니다."))
struct CARFIGHT_RE_API FCFWeaponRailPresentationEntry
{
	GENERATED_BODY()

	// [v1.14.0] Presenter가 완성한 Player-facing Rail 문자열입니다. 일반 항목은 `02  WEAPON NAME`, overflow는 `+N` 형식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Rail", meta=(DisplayName="Rail 표시 문자열", ToolTip="Widget이 Gameplay ID를 다시 해석하지 않도록 Presenter가 완성한 순번+DisplayName 또는 +N 문자열입니다."))
	FText DisplayText;

	// [v1.14.0] 실제 무기 항목이 아니라 숨겨진 비선택 무기 수를 나타내는 +N overflow 슬롯인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Rail", meta=(DisplayName="Overflow 항목", ToolTip="True이면 실제 한 무기 이름이 아니라 Rail 표시 한도를 넘은 비선택 무기 수를 +N으로 나타냅니다."))
	bool bOverflow = false;
};

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

		// [v1.11.0] 실제 Chaos RPM + explicit RedlineStartRPM + EngineMaxRPM을 승인 Tachometer 화면 비율로 변환합니다.
	static bool ResolveEngineRpmGaugePresentation(
		const FCFVehicleHUDData& VehicleViewData,
		float& OutGaugeRatio);

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

		// [v1.9.0] raw ResourceChannels를 직접 렌더링하지 않고 기존 Ammo/Status/Launcher 해석과 LauncherSequenceRevision lifecycle을 재사용해 Compact Presentation Entry 목록을 만듭니다.
	void BuildWeaponResourceEntries(
		const FCFWeaponHUDData& WeaponViewData,
		TArray<FCFWeaponResourcePresentationEntry>& OutPresentationEntries);

	// [v1.14.0] 현재 선택 무기를 제외하고 Provider fixed order를 유지한 비선택 무기 Rail 최대 3개를 순번+DisplayName 또는 2개+overflow로 Projection합니다.
	static void BuildWeaponRailEntries(
		const FCFWeaponHUDData& WeaponViewData,
		TArray<FCFWeaponRailPresentationEntry>& OutRailEntries);


private:
	// [v1.0.0] Provider의 새 ViewData를 현재 Production Widget에 적용합니다.
	UFUNCTION()
	void HandleHUDViewDataChanged(FCFInGameUIViewData ViewData);

	// [v1.0.0] 전체 ViewData를 Vehicle/Weapon/Target/Radar/Alert 영역별로 적용합니다.
	void ApplyViewData(const FCFInGameUIViewData& ViewData);

	// [v1.0.0] Vehicle 속도와 Defense 상태를 Production VehiclePanel에 적용합니다.
	void ApplyVehicleAndDefenseViewData(UUserWidget* RootWidget, const FCFVehicleHUDData& VehicleViewData, const FCFDefenseHUDData& DefenseViewData) const;

			// [v1.14.0] Header Reserve, 한 번 Projection한 Compact Resource Presentation과 truthful 비선택 Weapon Rail을 Production WeaponPanel 의미 슬롯에 적용합니다.
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