// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.0
// Date: 2026-08-22
// Description: CF-FQ-032 HUD Provider + post-closure hot-path copy/snapshot 교정
// Scope: Current Pawn Gameplay Runtime과 actor-free Sensor Snapshot을 ViewData로 변환하며 기존 Camera/Aim Runtime을 읽기 전용 ViewMode 데이터로 투영합니다.
// Changelog:
// - v1.10.0: C++ hot-path용 const ViewData 참조 getter를 추가하고 한 Refresh 안에서 Sensor Snapshot/Radar Range Profile을 한 번만 해석해 Target/Radar filler가 공유할 수 있도록 내부 계약을 확장.
// - v1.9.0: VehicleCameraComp/AimComp의 기존 public Runtime을 재계산 없이 읽어 Camera Mode, 차량 Heading, 카메라·터렛 상대 Yaw/Pitch를 FCFViewModeHUDData로 전달.
// - v1.8.0: 적용 Scanner Radar Range Preset을 읽어 Provider-local RangePresetIndex, Zoom In/Out command와 Heading-Up normalized Contact/selected edge 방향 ViewData를 추가. Sensor 탐지 성능 mutation 0.
// - v1.7.0: UI-P0-06 Technical Complete 범위 교정에 맞춰 Current 주석을 동기화. WeaponCharge/Heat는 실제 Runtime Provider가 존재하며 VehicleBattery만 미구현 shared-power Gameplay 기능으로 분리.
// - v1.6.0: UI-P0-06 기존 실제 Weapon 필드를 additive 공통 ResourceChannels로 투영. 당시 실제 Runtime 없던 Battery·Charge·Heat는 생성하지 않음. Chaos RPM/Gear 실제 Provider 상태를 Current 계약에 반영.
// - v1.5.0: TargetSelect는 선택/TrackState만, Sensor Snapshot은 Target Knowledge/Radar Contact만 제공하도록 read-only 소비 경계를 확정.
// - v1.4.0: OnLauncherSequenceChanged에서만 증가하는 LauncherSequenceRevision을 추가해 Launcher 의미 전이와 Ammo/Timer 부수 Refresh를 분리.
// - v1.3.0: VehicleAmmoComp OnAmmoRuntimeChanged를 Pawn Rebind 수명에 연결하고 실제 finite Ammo·Reload Snapshot을 Weapon ViewData로 변환.
// - v1.2.0: 정상 Launcher Sequence는 Weapon ViewData가 소유하고 AlertFeed에는 전역 주의 상태만 전달하도록 역할을 명확화.
// - v1.1.0: Launcher Sequence 상태 변경 이벤트를 Rebind 수명에 연결해 Alert/진행 상태를 10Hz Polling 대기 없이 즉시 Refresh.
// - v1.0.0: Pawn Rebind, Defense/Health/Target 이벤트 구독, 10Hz 연속값 갱신과 전체 HUD ViewData Broadcast를 최초 추가.
// Migration:
// - Widget은 Pawn/Component를 직접 Cast하지 않고 이 Provider의 ViewData만 소비합니다.
// - Launcher 계산을 UI에서 재구현하지 않고 UCFLauncherComp의 public 상태 변경 이벤트를 읽기 전용으로 소비합니다.
// - v1.5.0부터 Target Knowledge와 Radar는 FCFSensorSnapshot만 읽으며 TargetSelectable 원본 InformationLevel/이름을 Player Knowledge로 사용하지 않습니다.
// - TargetSelect는 선택 기록·유효성·TrackState owner로 유지하고 Sensor Contact lifecycle을 재계산하지 않습니다.
// - v1.8.0 Radar Range/Zoom은 Scanner가 명시한 RadarDisplayRangePresetsCm만 사용합니다. Provider가 현재 Preset 선택을 소유하며 Sensor 탐지/Active Scan 성능은 변경하지 않습니다.
// - Range Profile이 비어 있으면 기존처럼 DisplayRange/NormalizedPosition은 Unavailable이며 Passive/Active 거리로 임의 fallback하지 않습니다.
// - Engine RPM과 Gear는 실제 UE 5.8 Chaos Vehicle Runtime을 읽고, RPM Gauge Redline은 별도 명시 계약 전 추정하지 않습니다.
// - 실제 Gameplay Runtime Provider가 없는 VehicleBattery는 Unavailable로 유지하고 ResourceChannels 항목도 생성하지 않습니다. WeaponCharge와 Heat는 실제 Runtime Provider가 있을 때만 actual ResourceChannel로 전달합니다.
// - v1.10.0 C++ per-frame 소비자는 GetCurrentViewDataRef()로 최신 cache를 읽어 동적 배열을 포함한 전체 FCFInGameUIViewData 복사를 피할 수 있습니다. Blueprint용 GetCurrentViewData() by-value 계약은 그대로 유지합니다.
// - v1.10.0 RefreshViewData는 Sensor Snapshot과 Radar Range Profile을 각각 한 번만 캡처한 뒤 Target/Radar에 같은 read-only 입력을 전달합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageRuntimeTypes.h"
#include "CFDamageTypes.h"
#include "CFLauncherTypes.h"
#include "CFTargetSelectTypes.h"
#include "TimerManager.h"
#include "UI/CFHUDViewData.h"
#include "UObject/Object.h"
#include "CFHUDDataProvider.generated.h"

class AActor;
class ACFVehiclePawn;
class APawn;
class UCFLauncherComp;
class UCFVehicleAmmoComp;
class UCFTargetSelectComp;
class UCFUISubsystem;
class UCFVehicleDefenseComp;
class UCFVehicleHealthComp;
class UWorld;
struct FCFSensorSnapshot;


/**
 * Provider가 새 통합 ViewData를 만들었음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFHUDViewDataChangedSignature, FCFInGameUIViewData, ViewData);

/**
 * LocalPlayer Current Pawn을 실제 Gameplay Runtime에 Rebind하고 HUD 표시 데이터로 변환합니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFHUDDataProvider : public UObject
{
	GENERATED_BODY()

public:
	// [v1.0.0] UISubsystem의 Current Pawn 변경 이벤트를 구독하고 현재 Pawn을 즉시 Rebind합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="HUD 데이터 Provider 초기화", ToolTip="UISubsystem의 OnCurrentPawnChanged를 구독하고 현재 Pawn의 Gameplay Runtime을 HUD ViewData Source로 연결합니다."))
	bool InitializeProvider(UCFUISubsystem* InUISubsystem);

	// [v1.0.0] UISubsystem과 Gameplay Component 이벤트, 연속값 Timer를 모두 해제합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="HUD 데이터 Provider 종료", ToolTip="Current Pawn, Gameplay 이벤트와 갱신 Timer 연결을 모두 해제합니다."))
	void ShutdownProvider();

	// [v1.0.0] 지정 Pawn을 새 UI Gameplay Source로 바꾸고 이전 Pawn 구독을 먼저 제거합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="현재 HUD Pawn 재연결", ToolTip="Old Pawn 이벤트와 Timer를 제거한 뒤 새 Pawn이 CFVehiclePawn이면 Gameplay ViewData Source로 연결합니다."))
	void RebindCurrentPawn(APawn* NewPawn);

			// [v1.0.0] 현재 Bound Pawn의 실제 Runtime에서 통합 ViewData를 다시 계산하고 Broadcast합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="HUD ViewData 새로고침", ToolTip="현재 차량 Runtime의 실제 값을 Vehicle, Weapon, Defense, Target, Radar, Alert ViewData로 다시 계산합니다."))
	void RefreshViewData();

	// [v1.8.0] 현재 Scanner Range Profile에서 한 단계 작은 Radar 표시 범위로 이동합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar Zoom In 요청", ToolTip="현재 Scanner가 명시한 Radar 표시 Range Preset 중 한 단계 작은 범위로 이동합니다. Sensor 탐지 거리와 Active Scan 성능은 변경하지 않습니다."))
	bool RequestRadarZoomIn();

	// [v1.8.0] 현재 Scanner Range Profile에서 한 단계 큰 Radar 표시 범위로 이동합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar Zoom Out 요청", ToolTip="현재 Scanner가 명시한 Radar 표시 Range Preset 중 한 단계 큰 범위로 이동합니다. Sensor 탐지 거리와 Active Scan 성능은 변경하지 않습니다."))
	bool RequestRadarZoomOut();

		// [v1.0.0] 마지막으로 계산된 통합 ViewData를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="현재 HUD ViewData 반환", ToolTip="가장 최근 Provider 갱신에서 생성한 전체 HUD ViewData 사본을 반환합니다."))
	FCFInGameUIViewData GetCurrentViewData() const { return CurrentViewData; }

	// [v1.10.0] C++ hot-path가 동적 배열을 포함한 전체 ViewData 복사 없이 최신 읽기 전용 Cache를 참조합니다.
	const FCFInGameUIViewData& GetCurrentViewDataRef() const { return CurrentViewData; }


	// [v1.0.0] 현재 Provider가 연결한 차량 Pawn을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="현재 HUD 차량 Pawn 반환", ToolTip="Provider 내부 Gameplay Source 진단용입니다. Production Widget은 이 Getter를 사용하지 말고 ViewData만 소비합니다."))
	ACFVehiclePawn* GetBoundVehiclePawn() const { return BoundVehiclePawn.Get(); }

	// [v1.0.0] 실제 Pawn Rebind 횟수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="HUD Pawn Binding Generation 반환", ToolTip="OnCurrentPawnChanged로 실제 Rebind될 때 증가하는 Generation입니다."))
	int32 GetBindingGeneration() const { return BindingGeneration; }

	// [v1.0.0] Production Presenter와 테스트가 구독할 전체 ViewData 변경 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|HUD|Provider", meta=(DisplayName="HUD ViewData 변경 이벤트", ToolTip="Gameplay Runtime을 새 ViewData로 변환한 뒤 발생합니다."))
	FCFHUDViewDataChangedSignature OnHUDViewDataChanged;

private:
	// [v1.0.0] UISubsystem OnCurrentPawnChanged를 Provider Rebind로 변환합니다.
	UFUNCTION()
	void HandleCurrentPawnChanged(APawn* PreviousPawn, APawn* NewPawn);

	// [v1.0.0] VehicleHealth 값 변경을 즉시 ViewData Refresh로 반영합니다.
	UFUNCTION()
	void HandleVehicleHealthChanged(float PreviousHealth, float CurrentHealth, float MaxHealth);

	// [v1.0.0] Vehicle Destroyed 전이를 즉시 ViewData와 Alert에 반영합니다.
	UFUNCTION()
	void HandleVehicleDestroyed(FCFDamageHitContext DamageHitContext);

	// [v1.0.0] Shield 변경을 즉시 ViewData Refresh로 반영합니다.
	UFUNCTION()
	void HandleShieldChanged(float PreviousShield, float CurrentShield, float MaximumShield);

	// [v1.0.0] 방향 Armor 변경을 즉시 ViewData Refresh로 반영합니다.
	UFUNCTION()
	void HandleArmorChanged(ECFArmorDirection ArmorDirection, float PreviousArmor, float CurrentArmor, float MaximumArmor);

						// [v1.2.0] Launcher Sequence 시작·진행·완료·취소 상태를 즉시 Weapon ViewData로 반영합니다.
	UFUNCTION()
	void HandleLauncherSequenceChanged(FCFLauncherSequenceRuntime SequenceRuntime);

	// [v1.3.0] 장전·예비·예약·Reload·Action Lock 변경을 즉시 Weapon ViewData로 반영합니다.
	UFUNCTION()
	void HandleAmmoRuntimeChanged(FCFAmmoRuntimeSnapshot AmmoSnapshot);

	// [v1.0.0] 선택 Target 교체를 즉시 Player-facing Target ViewData로 반영합니다.
	UFUNCTION()
	void HandleSelectedTargetChanged(AActor* PreviousTarget, AActor* NewTarget, FCFTargetDisplayInfo DisplayInfo);

	// [v1.0.0] 선택 Target 해제를 즉시 ViewData에 반영합니다.
	UFUNCTION()
	void HandleSelectedTargetCleared(AActor* ClearedTarget, ECFTargetClearReason ClearReason);

	// [v1.0.0] 선택 Target 유효성 전이를 즉시 ViewData에 반영합니다.
	UFUNCTION()
	void HandleSelectedTargetValidityChanged(AActor* TargetActor, bool bIsValidTarget);

	// [v1.0.0] 선택 Target 추적 상태 전이를 즉시 ViewData에 반영합니다.
	UFUNCTION()
	void HandleSelectedTargetTrackStateChanged(AActor* TargetActor, ECFTargetTrackState PreviousState, ECFTargetTrackState NewState);

			// [v1.1.0] 이전 Pawn의 Health/Defense/Launcher/Target 이벤트 구독을 모두 제거합니다.
	void UnbindGameplayEvents();

	// [v1.1.0] 새 Pawn의 Health/Defense/Launcher/Target 이벤트를 Provider에 연결합니다.
	void BindGameplayEvents();

	// [v1.0.0] 속도·쿨다운·Launcher 진행처럼 연속 변화하는 값의 10Hz Game-Time 갱신 Timer를 시작합니다.
	void StartContinuousRefreshTimer();

	// [v1.0.0] 이전 Pawn World의 연속값 갱신 Timer를 제거합니다.
	void StopContinuousRefreshTimer();

	// [v1.0.0] 현재 차량의 주행 ViewData를 채웁니다.
	void FillVehicleViewData(FCFVehicleHUDData& OutVehicleViewData) const;

		// [v1.9.0] 현재 차량의 Camera/Aim Runtime을 외부 3인칭 View Mode 방향 ViewData로 변환합니다.
	void FillViewModeViewData(FCFViewModeHUDData& OutViewModeViewData) const;

	// [v1.0.0] 현재 차량의 Defense/Integrity ViewData를 채웁니다.
	void FillDefenseViewData(FCFDefenseHUDData& OutDefenseViewData) const;

	// [v1.0.0] 현재 활성 Weapon과 Launcher ViewData를 채웁니다.
	void FillWeaponViewData(FCFWeaponHUDData& OutWeaponViewData) const;

				// [v1.10.0] TargetSelect의 선택/TrackState와 이번 Refresh에서 한 번 캡처한 Sensor Snapshot의 Player Knowledge를 중복 판정 없이 합성합니다.
	void FillTargetViewData(FCFTargetHUDData& OutTargetViewData, const FCFSensorSnapshot* SensorSnapshot) const;

	// [v1.10.0] 이번 Refresh에서 한 번 캡처한 Sensor Snapshot과 Radar Range Profile을 Radar ViewData로 읽기 전용 변환합니다.
	void FillRadarViewData(FCFRadarHUDData& OutRadarViewData, const FCFSensorSnapshot* SensorSnapshot, const TArray<float>& RadarRangePresetsMeters) const;

	// [v1.8.0] 현재 적용 Scanner의 Radar Range Preset을 cm→m Player-facing 표시 단위로 해석합니다.
	bool ResolveRadarRangeProfile(TArray<float>& OutRadarRangePresetsMeters, int32& OutDefaultPresetIndex) const;

		// [v1.10.0] 이미 해석된 Scanner Range Profile로 현재 Radar RangePresetIndex를 검증하고 명시 Default로 초기화합니다.
	void ReconcileRadarRangePresetSelection(const TArray<float>& RadarRangePresetsMeters, int32 DefaultPresetIndex);

	// [v1.0.0] 실제 Destroyed, Shield Down, Armor Breach, Launcher Active 상태로 현재 Alert 목록을 만듭니다.
	void FillAlertViewData(FCFCombatAlertViewData& OutAlertViewData) const;

	// [v1.0.0] 현재값이 실제 제공됐을 때 0과 비0을 KnownZero/Known으로 구분합니다.
	static ECFUIViewAvailability ResolveKnownNumericAvailability(bool bAvailable, float CurrentValue);

	// [v1.0.0] 현재 Alert 목록에 안정 Key 하나를 중복 없이 추가합니다.
	static void AddAlertUnique(TArray<FCFHUDAlertItem>& Alerts, FName AlertKey, ECFHUDAlertPriority Priority, const FText& Title, const FText& Detail);

	// [v1.0.0] 이 Provider를 소유하고 Current Pawn 변경을 통지하는 LocalPlayer UI Subsystem입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFUISubsystem> UISubsystem = nullptr;

	// [v1.0.0] 현재 Gameplay Source로 연결된 차량 Pawn 약한 참조입니다.
	TWeakObjectPtr<ACFVehiclePawn> BoundVehiclePawn;

	// [v1.0.0] 현재 이벤트를 구독한 VehicleHealthComp 약한 참조입니다.
	TWeakObjectPtr<UCFVehicleHealthComp> BoundHealthComponent;

	// [v1.0.0] 현재 이벤트를 구독한 VehicleDefenseComp 약한 참조입니다.
	TWeakObjectPtr<UCFVehicleDefenseComp> BoundDefenseComponent;

				// [v1.1.0] 현재 이벤트를 구독한 LauncherComp 약한 참조입니다.
	TWeakObjectPtr<UCFLauncherComp> BoundLauncherComponent;

	// [v1.3.0] 현재 이벤트를 구독한 VehicleAmmoComp 약한 참조입니다.
	TWeakObjectPtr<UCFVehicleAmmoComp> BoundAmmoComponent;

	// [v1.0.0] 현재 이벤트를 구독한 TargetSelectComp 약한 참조입니다.
	TWeakObjectPtr<UCFTargetSelectComp> BoundTargetSelectComponent;

	// [v1.0.0] 연속값 Timer를 등록한 현재 Pawn World 약한 참조입니다.
	TWeakObjectPtr<UWorld> BoundWorld;

	// [v1.0.0] 속도·쿨다운·Launcher 진행 갱신에 사용하는 Game-Time Timer Handle입니다.
	FTimerHandle ContinuousRefreshTimerHandle;

	// [v1.0.0] 연속값 ViewData 갱신 간격입니다.
	float ContinuousRefreshIntervalSeconds = 0.10f;

	// [v1.0.0] 실제 Pawn Rebind 횟수입니다.
	int32 BindingGeneration = 0;

					// [v1.4.0] 현재 Pawn Binding에서 실제 Launcher 상태 이벤트가 발생한 누적 Revision입니다.
	int32 LauncherSequenceRevision = 0;

	// [v1.8.0] 현재 LocalPlayer HUD가 선택한 Scanner Radar 표시 Range Preset의 0-based 인덱스입니다.
	int32 RadarRangePresetIndex = INDEX_NONE;

	// [v1.0.0] Provider가 ViewData를 생성한 누적 Revision입니다.
	int32 ViewDataRevision = 0;

	// [v1.0.0] Production Presenter에 마지막으로 전달한 통합 ViewData Cache입니다.
	UPROPERTY(Transient)
	FCFInGameUIViewData CurrentViewData;
};
