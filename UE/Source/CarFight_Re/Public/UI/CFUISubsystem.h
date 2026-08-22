// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.0
// Date: 2026-08-21
// Description: CarFight LocalPlayer UI 수명·레이어·Pause·Production HUD·Target Marker·Radar Zoom·Screen-edge Relation Visual 연결 Subsystem
// Scope: 기존 UI 수명을 보존하면서 UI-P0-08 Screen-Off Selected Target Edge Marker가 공용 HUD Visual/Style Data를 주입받도록 연결합니다.
// Changelog:
// - v1.10.0: USER 가독성 피드백에 따라 HUDDataProvider와 Friendly/Hostile/Unknown Style 색을 TargetSelect Screen-edge에 주입. Neutral은 Edge에서만 Unknown 회색을 공유하고 전역 NeutralColor는 변경하지 않음.
// - v1.9.0: DefaultHUDVisualDataAsset을 Subsystem 수명에서 해석하고 RadarSelectedEdgeBracket·AccentTactical·SafeMargin만 TargetSelect Widget에 주입. WBP_TargetSelect 저장 구조와 TargetSelect Gameplay ownership은 변경하지 않음.
// - v1.8.0: Pawn 입력 계층이 Provider 구현에 직접 결합되지 않도록 RequestRadarZoomIn/Out C++ wrapper를 추가. Scanner/Sensor detection range와 Profile은 변경하지 않음.
// - v1.7.0: UI-P0-05 WBP_TargetSelect Config Class, Game Layer 단일 인스턴스, Current Pawn Rebind와 World Cleanup 수명을 UISubsystem으로 이전.
// - v1.6.0: UI-P0-04 WBP_AimReticle Config Class, HUD Layer 단일 인스턴스, Current Pawn Rebind와 World Cleanup 수명을 UISubsystem으로 이전.
// - v1.5.0: UI-P0-03 HUDDataProvider/HUDPresenter와 Config Production WBP_CFInGameHUD 생성·HUD Layer 연결 수명을 추가.
// - v1.4.0: Config 기반 Style·Density·HUD Layout Soft Reference, 1회 해석 Cache와 Native CDO Fallback Getter를 추가.
// - v1.3.0: UI Root를 Legacy Pawn HUD와 동일한 AddToViewport 계층에 등록해 실제 ZOrder·Hit Test 우선권을 보장.
// - v1.2.0: UI Root를 Legacy Pawn HUD보다 높은 Viewport ZOrder에 두고 Pause Focus를 실제 Continue 버튼으로 고정.
// - v1.1.1: Pause 중 Pawn Gameplay는 억제하고 PlayerController Pause·Back과 Menu Focus는 GameAndUI로 함께 유지.
// - v1.1.0: UI-P0-02 실제 World Pause, Menu 레이어 Pause Widget, Pause·Back 토글, Continue과 World Cleanup 해제를 추가.
// - v1.0.1: UI Root 레이어 반환 타입을 실제 ZOrder를 지원하는 CanvasPanel로 변경.
// - v1.0.0: UI-P0-01B LocalPlayer Subsystem, World별 Root, Screen·Modal 전환과 Pawn 약한 참조를 최초 추가.
// Migration:
// - Pause Menu는 C++ UCFPauseMenuWidget을 사용하며 Unreal Asset을 요구하지 않는다.
// - Pause 진입은 PlayerController가 Gameplay 입력을 중립화한 뒤 World Pause를 적용한다.
// - 진행 중 Launcher·Projectile·Timer 상태는 취소하지 않고 World Pause 동안 정지한 뒤 해제 시 이어진다.
// - Subsystem은 Loadout·Fitting Draft·게임플레이 판정을 소유하지 않는다.
// - 기존 Pawn 소유 HUD는 UI-P0-04~05 전까지 그대로 유지하되 싱글플레이 UI Root는 동일한 AddToViewport 계층의 더 높은 ZOrder를 사용한다.
// - D1-09B Config가 비었거나 Asset 검증에 실패하면 Style·Density·Layout 각각의 Native CDO Fallback을 반환합니다.
// - Production HUD는 WBP_CFInGameHUD의 기존 CFStyledWidgetBase Parent를 유지하며 Gameplay Runtime은 Provider/Presenter를 통해서만 전달합니다.
// - v1.6.0부터 AimReticle 생성·레이어·Pawn Rebind 수명은 UISubsystem이 소유하며, ACFVehiclePawn의 기존 Reticle Class/API는 직렬화·호환 경계로만 남깁니다.
// - v1.7.0부터 TargetSelect Marker 생성·Game Layer·Pawn Rebind 수명도 UISubsystem이 소유하며 Pawn의 기존 TargetSelect HUD Class/API는 직렬화·호환 경계로만 남깁니다.
// - v1.8.0 Radar Zoom wrapper는 현재 HUDDataProvider의 UI-local Range Preset 선택에만 위임합니다. Sensor detection 범위·Scanner Profile·Gameplay Runtime을 수정하지 않습니다.
// - v1.9.0 Target Screen-edge 표현은 UISubsystem이 HUD Visual/Style Data를 해석해 주입하며 TargetSelect Widget은 콘텐츠 경로를 직접 Load하지 않습니다.
// - v1.10.0 Screen-edge 관계색은 Provider가 이미 판정한 Target.Relation을 소비하며 Friendly/Hostile/Unknown Style Token만 주입합니다. Neutral은 Screen-edge에서만 Unknown 회색으로 표시하고 전역 Palette는 보존합니다.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/CFUITypes.h"
#include "CFUISubsystem.generated.h"

class ACFPlayerController;
class APawn;
class UCanvasPanel;
class UCFAimReticleWidget;
class UCFTargetSelectWidget;
class UCFHUDDataProvider;
class UCFHUDLayoutData;
class UCFHUDPresenter;
class UCFHUDVisualData;
class UCFPauseMenuWidget;
class UCFStyledWidgetBase;
class UCFUIDensityData;
class UCFUIRootWidget;
class UCFUIStyleData;
class UUserWidget;
class UWidget;
class UWorld;

/**
 * UI 공통 Pause 요청이 들어왔음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCFUIPauseRequestSignature);

/**
 * UI 공통 Back 요청이 들어왔음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCFUIBackRequestSignature);

/**
 * LocalPlayer UI가 참조할 Pawn이 변경됐음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFUIPawnChangedSignature, APawn*, PreviousPawn, APawn*, NewPawn);

/**
 * LocalPlayer UI 화면 상태가 변경됐음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFUIScreenStateChangedSignature, ECFUIScreenState, PreviousState, ECFUIScreenState, NewState);

/**
 * 싱글플레이 Pause 활성 상태가 변경됐음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFUIPauseStateChangedSignature, bool, bWasPaused, bool, bIsPaused);

/**
 * LocalPlayer마다 하나 존재하며 현재 World용 UI Root와 레이어·Pause 수명을 관리합니다.
 */
UCLASS(BlueprintType, Config=Game)
class CARFIGHT_RE_API UCFUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// [v1.0.0] World Cleanup 감시를 등록하고 초기 상태를 준비합니다.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// [v1.1.0] 활성 Pause를 해제하고 현재 Root와 World Cleanup 감시를 정리합니다.
	virtual void Deinitialize() override;

	// [v1.0.0] 현재 LocalPlayer Controller를 등록하고 해당 World용 Root를 보장합니다.
	bool RegisterPlayerController(ACFPlayerController* PlayerController);

	// [v1.1.0] 현재 Pause를 해제한 뒤 LocalPlayer Controller 등록과 Root를 정리합니다.
	void UnregisterPlayerController(ACFPlayerController* PlayerController);

	// [v1.0.0] 현재 World에 정확히 하나의 UI Root가 존재하도록 보장합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Lifecycle", meta=(DisplayName="현재 World UI Root 보장 (Ensure UI Root For World)", ToolTip="현재 LocalPlayer Controller가 속한 World에 UI Root가 없으면 생성하고, 다른 World Root가 남아 있으면 먼저 제거합니다."))
	bool EnsureUIRootForWorld(UWorld* World);

	// [v1.1.0] 활성 Pause와 현재 World Root의 모든 레이어 자식을 제거합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Lifecycle", meta=(DisplayName="현재 UI Root 해제 (Release UI Root)", ToolTip="활성 Pause를 해제한 뒤 현재 World용 UI Root의 모든 레이어를 비우고 Viewport에서 제거합니다."))
	void ReleaseUIRoot();

		// [v1.0.0] 현재 World용 UI Root를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Lifecycle", meta=(DisplayName="현재 UI Root 반환 (Get UI Root)", ToolTip="현재 LocalPlayer와 World에 연결된 UI Root를 반환합니다."))
	UCFUIRootWidget* GetUIRoot() const { return RootWidget; }

	// [v1.2.0] 기존 Pawn 소유 HUD보다 위에 UI Root를 표시할 Viewport ZOrder를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Lifecycle", meta=(DisplayName="UI Root Viewport ZOrder 반환 (Get UI Root Viewport ZOrder)", ToolTip="Pause, Modal과 시스템 UI가 기존 Aim Reticle과 TargetSelect HUD보다 위에 표시되도록 사용하는 UI Root Viewport ZOrder입니다."))
	int32 GetRootViewportZOrder() const { return RootViewportZOrder; }

	// [v1.0.1] 현재 UI Root의 지정 레이어 CanvasPanel을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Layer", meta=(DisplayName="현재 UI 레이어 반환 (Get Current UI Layer)", ToolTip="현재 UI Root에서 지정한 표준 CanvasPanel 레이어를 반환합니다."))
	UCanvasPanel* GetLayerWidget(ECFUILayer Layer) const;

	// [v1.0.0] Widget을 현재 Root의 지정 레이어에 추가합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Layer", meta=(DisplayName="현재 레이어에 Widget 추가 (Push Widget To Layer)", ToolTip="현재 Root의 지정 레이어에 Widget을 추가합니다. Root가 없으면 실패합니다."))
	bool PushWidgetToLayer(UWidget* WidgetToAdd, ECFUILayer Layer, int32 ZOrder = 0);

	// [v1.0.0] 현재 Root의 지정 레이어를 비웁니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Layer", meta=(DisplayName="현재 UI 레이어 비우기 (Clear Current UI Layer)", ToolTip="현재 UI Root의 지정 표준 UI 레이어에 있는 모든 자식 Widget을 제거합니다."))
	void ClearLayer(ECFUILayer Layer);

	// [v1.1.0] Pause가 아닐 때 주요 전체 화면 Widget을 Screen 레이어의 단일 화면으로 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Screen", meta=(DisplayName="주요 화면 설정 (Set Primary Screen)", ToolTip="Pause가 아닐 때 Screen 레이어의 기존 화면을 제거하고 새 전체 화면 Widget을 표시하며 지정 입력 모드와 Focus를 적용합니다."))
	bool SetPrimaryScreenWidget(UUserWidget* ScreenWidget, ECFUIInputMode InputMode = ECFUIInputMode::UIOnly, bool bShowMouseCursor = true);

	// [v1.1.0] 현재 주요 전체 화면을 닫고 Pause 중이 아니면 인게임 입력으로 복귀합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Screen", meta=(DisplayName="주요 화면 닫기 (Clear Primary Screen)", ToolTip="Screen 레이어를 비우고 Pause 중이 아니면 GameOnly 입력 모드로 복귀합니다."))
	void ClearPrimaryScreenWidget();

	// [v1.1.0] Pause가 아닐 때 Modal 레이어에 단일 Modal Widget을 표시합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Modal", meta=(DisplayName="Modal 화면 설정 (Set Modal Widget)", ToolTip="Pause가 아닐 때 Modal 레이어의 기존 Widget을 제거하고 새 Modal을 표시하며 UI 입력과 Focus를 적용합니다."))
	bool SetModalWidget(UUserWidget* ModalWidget, bool bShowMouseCursor = true);

	// [v1.1.0] 현재 Modal Widget을 닫고 Pause 또는 이전 화면 성격에 맞는 입력 모드로 복귀합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Modal", meta=(DisplayName="Modal 화면 닫기 (Clear Modal Widget)", ToolTip="Modal 레이어를 비우고 Pause Menu, 전체 화면 또는 인게임 중 현재 상태에 맞는 입력 모드와 Focus를 복원합니다."))
	void ClearModalWidget();

		// [v1.1.1] 입력을 중립화하고 Menu 레이어 Pause Widget을 만든 뒤 실제 싱글플레이 World Pause를 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Pause", meta=(DisplayName="싱글플레이 Pause 진입 (Enter Single Player Pause)", ToolTip="차량 Gameplay 입력을 중립화하고 Pause Menu를 Menu 레이어에 표시한 뒤 World Pause를 적용합니다. Pawn Gameplay는 억제하고 PlayerController Pause·Back과 Menu Focus는 유지합니다."))
	bool EnterSinglePlayerPause();

	// [v1.1.0] 실제 World Pause와 Pause Menu를 해제하고 이전 화면 입력 상태를 복원합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Pause", meta=(DisplayName="싱글플레이 Pause 해제 (Exit Single Player Pause)", ToolTip="World Pause를 해제하고 Pause Menu를 제거한 뒤 인게임 또는 기존 전체 화면 입력 상태로 복귀합니다."))
	bool ExitSinglePlayerPause();

	// [v1.1.0] 현재 Pause 상태에 따라 Pause 진입과 해제를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Pause", meta=(DisplayName="싱글플레이 Pause 전환 (Toggle Single Player Pause)", ToolTip="현재 Pause 상태이면 해제하고 아니면 Pause Menu와 World Pause를 활성화합니다."))
	bool ToggleSinglePlayerPause();

	// [v1.1.0] 이 Subsystem이 소유한 싱글플레이 Pause가 활성 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Pause", meta=(DisplayName="UI Pause 활성 여부 반환 (Is UI Pause Active)", ToolTip="현재 UCFUISubsystem이 Pause Menu와 World Pause 수명을 소유하고 있으면 True입니다."))
	bool IsSinglePlayerPauseActive() const { return bSinglePlayerPauseActive; }

	// [v1.1.0] 현재 Menu 레이어에 표시 중인 Pause Menu Widget을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Pause", meta=(DisplayName="Pause Menu 반환 (Get Pause Menu)", ToolTip="현재 표시 중인 C++ Pause Menu Widget을 반환합니다. Pause가 아니면 Null입니다."))
	UCFPauseMenuWidget* GetPauseMenuWidget() const { return PauseMenuWidget; }

	// [v1.1.0] Controller의 Pause 공통 입력을 이벤트로 전달하고 Pause 상태를 전환합니다.
	void NotifyPauseInputRequested();

	// [v1.1.0] Controller의 Back 입력을 이벤트로 전달하고 Modal 또는 Pause를 우선 닫습니다.
	void NotifyBackInputRequested();

	// [v1.0.0] Controller의 Possession 변경을 약한 Pawn 참조와 UI 이벤트에 반영합니다.
	void NotifyPossessedPawnChanged(APawn* PreviousPawn, APawn* NewPawn);

		// [v1.0.0] 현재 UI가 참조하는 Pawn을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Lifecycle", meta=(DisplayName="현재 UI Pawn 반환 (Get Current UI Pawn)", ToolTip="현재 LocalPlayer UI가 데이터 소스로 연결할 Pawn을 반환합니다. Pawn 파괴 후에는 Null일 수 있습니다."))
		APawn* GetCurrentPawn() const { return CurrentPawn.Get(); }

	// [v1.5.0] 현재 LocalPlayer의 HUD Gameplay Runtime → ViewData Provider를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD", meta=(DisplayName="HUD 데이터 Provider 반환 (Get HUD Data Provider)", ToolTip="현재 LocalPlayer의 Gameplay Runtime을 ViewData로 변환하는 HUD Provider를 반환합니다. Production Widget은 Pawn 대신 이 Provider 데이터를 소비합니다."))
	UCFHUDDataProvider* GetHUDDataProvider() const { return HUDDataProvider; }

	// [v1.8.0] 현재 LocalPlayer Radar 표시 범위를 한 단계 작은 Preset으로 요청합니다.
	bool RequestRadarZoomIn();

	// [v1.8.0] 현재 LocalPlayer Radar 표시 범위를 한 단계 큰 Preset으로 요청합니다.
	bool RequestRadarZoomOut();

		// [v1.5.0] 현재 HUD Layer에 표시 중인 Production WBP_CFInGameHUD 인스턴스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD", meta=(DisplayName="Production HUD Widget 반환 (Get Production HUD Widget)", ToolTip="현재 World HUD Layer에 생성된 Production WBP_CFInGameHUD 인스턴스를 반환합니다. 없으면 Null입니다."))
	UCFStyledWidgetBase* GetInGameHUDWidget() const { return InGameHUDWidget; }

	// [v1.6.0] 현재 HUD Layer에 표시 중인 UISubsystem 소유 Aim Reticle 인스턴스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD", meta=(DisplayName="Aim Reticle Widget 반환 (Get Aim Reticle Widget)", ToolTip="현재 World HUD Layer에 UISubsystem이 단일 생성한 WBP_AimReticle 인스턴스를 반환합니다. 없으면 Null입니다."))
	UCFAimReticleWidget* GetAimReticleWidget() const { return AimReticleWidget; }

		// [v1.6.0] Production HUD보다 위에 배치되는 Aim Reticle의 HUD Layer 내부 ZOrder를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD", meta=(DisplayName="Aim Reticle HUD Layer ZOrder 반환", ToolTip="HUD Layer 안에서 Production HUD보다 Aim Reticle을 위에 배치하는 로컬 ZOrder입니다."))
	int32 GetAimReticleHUDLayerZOrder() const { return AimReticleHUDLayerZOrder; }

	// [v1.7.0] 현재 Game Layer에 표시 중인 UISubsystem 소유 TargetSelect World Marker 인스턴스를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Game", meta=(DisplayName="Target Marker Widget 반환", ToolTip="현재 World Game Layer에 UISubsystem이 단일 생성한 WBP_TargetSelect World Marker 인스턴스를 반환합니다. 없으면 Null입니다."))
	UCFTargetSelectWidget* GetTargetSelectWidget() const { return TargetSelectWidget; }

	// [v1.7.0] TargetSelect World Marker의 Game Layer 내부 ZOrder를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Game", meta=(DisplayName="Target Marker Game Layer ZOrder 반환", ToolTip="Game Layer 안에서 TargetSelect World Marker에 적용하는 로컬 ZOrder입니다."))
	int32 GetTargetSelectGameLayerZOrder() const { return TargetSelectGameLayerZOrder; }



		// [v1.0.0] 현재 화면 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Screen", meta=(DisplayName="현재 UI 화면 상태 반환 (Get UI Screen State)", ToolTip="현재 LocalPlayer UI의 화면 상태를 반환합니다."))
	ECFUIScreenState GetScreenState() const { return ScreenState; }

	// [v1.4.0] Config에서 해석된 Style Data를 반환하고 없으면 Native CDO Fallback을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Style", meta=(DisplayName="현재 UI Style Data 반환 (Get Resolved UI Style Data)", ToolTip="Config 기본 Style DataAsset을 반환합니다. 미설정·로드 실패·검증 실패 시 Native Safe Fallback을 반환합니다."))
	UCFUIStyleData* GetResolvedStyleData() const;

	// [v1.4.0] Config에서 해석된 Density Data를 반환하고 없으면 Native Standard CDO Fallback을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Style", meta=(DisplayName="현재 UI Density Data 반환 (Get Resolved UI Density Data)", ToolTip="Config 기본 Density DataAsset을 반환합니다. 미설정·로드 실패·검증 실패 시 Standard Native Fallback을 반환합니다."))
	UCFUIDensityData* GetResolvedDensityData() const;

	// [v1.4.0] Config에서 해석된 HUD Layout Data를 반환하고 없으면 D1-07 1080p Native CDO Fallback을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Style", meta=(DisplayName="현재 HUD Layout Data 반환 (Get Resolved HUD Layout Data)", ToolTip="Config 기본 HUD Layout DataAsset을 반환합니다. 미설정·로드 실패·검증 실패 시 승인된 1080p Native Fallback을 반환합니다."))
	UCFHUDLayoutData* GetResolvedHUDLayoutData() const;

	// [v1.0.0] Pause 공통 입력 요청 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Input")
	FCFUIPauseRequestSignature OnPauseInputRequested;

	// [v1.0.0] Back 공통 입력 요청 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Input")
	FCFUIBackRequestSignature OnBackInputRequested;

	// [v1.0.0] UI 데이터 소스 Pawn 변경 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Lifecycle")
	FCFUIPawnChangedSignature OnCurrentPawnChanged;

	// [v1.0.0] 화면 상태 변경 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Screen")
	FCFUIScreenStateChangedSignature OnScreenStateChanged;

	// [v1.1.0] Pause 진입·해제 성공 후 발생하는 상태 변경 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|UI|Pause")
	FCFUIPauseStateChangedSignature OnPauseStateChanged;

private:
	// [v1.9.0] Config Soft Reference의 Style·Density·HUD Layout·HUD Visual을 Subsystem 수명에서 한 번 Load·검증해 Cache합니다.
	void ResolveDefaultUIDataAssets();

	// [v1.1.0] C++ Pause Menu를 생성하고 Menu 레이어에 단일 인스턴스로 추가합니다.
	bool CreatePauseMenuWidget();

		// [v1.1.0] Continue 바인딩과 Menu 레이어의 Pause Menu 인스턴스를 정리합니다.
	void DestroyPauseMenuWidget();

	// [v1.5.0] Config Production Widget Class로 WBP_CFInGameHUD를 만들고 HUD Layer와 Presenter에 연결합니다.
	bool CreateInGameHUDWidget();

		// [v1.5.0] Presenter 연결을 먼저 해제하고 현재 Production HUD Widget을 제거합니다.
	void DestroyInGameHUDWidget();

	// [v1.6.0] Config Aim Reticle Class로 단일 Widget을 만들고 HUD Layer에 연결합니다.
	bool CreateAimReticleWidget();

	// [v1.6.0] 현재 Reticle의 Pawn 참조를 먼저 비운 뒤 HUD Layer에서 제거합니다.
	void DestroyAimReticleWidget();

		// [v1.6.0] 현재 Possessed Pawn만 Reticle Source로 연결하고 표시 토글을 갱신합니다.
	void RebindAimReticleToCurrentPawn();

	// [v1.7.0] Config TargetSelect Class로 단일 World Marker Widget을 만들고 Game Layer에 연결합니다.
	bool CreateTargetSelectWidget();

	// [v1.7.0] 현재 Target Marker의 Pawn 참조와 Delegate를 먼저 비운 뒤 Game Layer에서 제거합니다.
	void DestroyTargetSelectWidget();

		// [v1.7.0] 현재 Possessed Pawn만 Target Marker Source로 연결하고 표시 토글을 갱신합니다.
	void RebindTargetSelectToCurrentPawn();

	// [v1.9.0] 공용 HUD Visual/Style Data에서 Screen-edge Texture·Tint·SafeInset을 해석해 현재 Target Marker에 주입합니다.
	void ConfigureTargetSelectScreenEdgePresentation();



	// [v1.1.0] Pause 해제 후 Primary Screen 또는 인게임 입력 모드와 화면 상태를 복원합니다.
	void RestoreInputModeAfterPause();

	// [v1.1.0] Continue 버튼 요청을 싱글플레이 Pause 해제로 연결합니다.
	UFUNCTION()
	void HandlePauseContinueRequested();

	// [v1.0.0] 현재 화면 상태를 변경하고 이벤트를 한 번만 방송합니다.
	void SetScreenState(ECFUIScreenState NewScreenState);

	// [v1.1.0] 현재 World 정리 시 활성 Pause, Root와 약한 참조를 제거합니다.
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

		// [v1.4.0] 프로젝트 기본 Style DataAsset을 한 곳에서 소유하는 Config Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 Style Data Asset (Default Style Data Asset)", ToolTip="D1-09B DA_CFUIStyle_Default를 연결합니다. Widget은 이 경로를 직접 Load하지 않습니다."))
	TSoftObjectPtr<UCFUIStyleData> DefaultStyleDataAsset;

	// [v1.4.0] 프로젝트 기본 Density DataAsset을 한 곳에서 소유하는 Config Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 Density Data Asset (Default Density Data Asset)", ToolTip="D1-09B 기본 Standard Density DataAsset을 연결합니다."))
	TSoftObjectPtr<UCFUIDensityData> DefaultDensityDataAsset;

				// [v1.4.0] 프로젝트 기본 HUD Layout DataAsset을 한 곳에서 소유하는 Config Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 HUD Layout Data Asset (Default HUD Layout Data Asset)", ToolTip="D1-09B DA_CFHUDLayout_1080_16을 연결합니다."))
	TSoftObjectPtr<UCFHUDLayoutData> DefaultHUDLayoutDataAsset;

	// [v1.9.0] Production HUD와 World Marker가 공유할 기본 HUD Visual DataAsset을 한 곳에서 소유하는 Config Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 HUD Visual Data Asset (Default HUD Visual Data Asset)", ToolTip="DA_CFHUDVisual_Default의 Radar/Target 시각 자산을 UISubsystem에서 해석합니다. 개별 Widget은 콘텐츠 경로를 직접 Load하지 않습니다."))
	TSoftObjectPtr<UCFHUDVisualData> DefaultHUDVisualDataAsset;

		// [v1.5.0] 현재 World HUD Layer에 생성할 Production WBP_CFInGameHUD Class Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 인게임 HUD Widget Class (Default InGame HUD Widget Class)", ToolTip="D1-11 Production WBP_CFInGameHUD Class를 연결합니다. Widget은 Gameplay Pawn을 직접 조회하지 않습니다."))
	TSoftClassPtr<UCFStyledWidgetBase> DefaultInGameHUDWidgetClass;

		// [v1.6.0] UISubsystem이 HUD Layer에서 단일 소유할 기존 WBP_AimReticle Class Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 Aim Reticle Widget Class (Default Aim Reticle Widget Class)", ToolTip="기존 WBP_AimReticle Class를 UISubsystem HUD Layer 수명으로 생성합니다. Gameplay 계산과 Reticle 시각 의미는 변경하지 않습니다."))
	TSoftClassPtr<UCFAimReticleWidget> DefaultAimReticleWidgetClass;

	// [v1.7.0] UISubsystem이 Game Layer에서 단일 소유할 기존 WBP_TargetSelect Class Soft Reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, Category="CarFight|UI|Default Data", meta=(DisplayName="기본 Target Marker Widget Class", ToolTip="기존 WBP_TargetSelect Class를 UISubsystem Game Layer의 World Marker 수명으로 생성합니다. TargetSelect Gameplay 선택 상태는 변경하지 않습니다."))
	TSoftClassPtr<UCFTargetSelectWidget> DefaultTargetSelectWidgetClass;



	// [v1.4.0] 현재 Subsystem 수명에서 한 번 해석된 Style Data 강한 Cache입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFUIStyleData> ResolvedStyleData = nullptr;

	// [v1.4.0] 현재 Subsystem 수명에서 한 번 해석된 Density Data 강한 Cache입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFUIDensityData> ResolvedDensityData = nullptr;

				// [v1.4.0] 현재 Subsystem 수명에서 한 번 해석된 HUD Layout Data 강한 Cache입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFHUDLayoutData> ResolvedHUDLayoutData = nullptr;

	// [v1.9.0] 현재 Subsystem 수명에서 한 번 해석된 HUD Visual Data 강한 Cache입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFHUDVisualData> ResolvedHUDVisualData = nullptr;

		// [v1.5.0] Config Soft Class에서 한 번 해석한 Production InGame HUD Class입니다.
	UPROPERTY(Transient)
	TSubclassOf<UCFStyledWidgetBase> ResolvedInGameHUDWidgetClass;

		// [v1.6.0] Config Soft Class에서 한 번 해석한 Aim Reticle Class입니다.
	UPROPERTY(Transient)
	TSubclassOf<UCFAimReticleWidget> ResolvedAimReticleWidgetClass;

	// [v1.7.0] Config Soft Class에서 한 번 해석한 TargetSelect World Marker Class입니다.
	UPROPERTY(Transient)
	TSubclassOf<UCFTargetSelectWidget> ResolvedTargetSelectWidgetClass;



	// [v1.5.0] Current Pawn Gameplay Runtime을 통합 ViewData로 변환하는 LocalPlayer HUD Provider입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFHUDDataProvider> HUDDataProvider = nullptr;

	// [v1.5.0] Provider ViewData를 Production Widget 의미 요소에 적용하는 LocalPlayer HUD Presenter입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFHUDPresenter> HUDPresenter = nullptr;

		// [v1.5.0] 현재 World HUD Layer에 표시 중인 Production WBP_CFInGameHUD 강한 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFStyledWidgetBase> InGameHUDWidget = nullptr;

		// [v1.6.0] 현재 World HUD Layer에 표시 중인 UISubsystem 소유 Aim Reticle 강한 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFAimReticleWidget> AimReticleWidget = nullptr;

	// [v1.7.0] 현재 World Game Layer에 표시 중인 UISubsystem 소유 TargetSelect World Marker 강한 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFTargetSelectWidget> TargetSelectWidget = nullptr;



	// [v1.0.0] 현재 LocalPlayer를 소유하는 CarFight PlayerController입니다.
	TWeakObjectPtr<ACFPlayerController> ActivePlayerController;

	// [v1.0.0] 현재 UI 데이터 소스로 사용할 Possessed Pawn입니다.
	TWeakObjectPtr<APawn> CurrentPawn;

	// [v1.0.0] Root Widget이 속한 현재 World입니다.
	TWeakObjectPtr<UWorld> RootWorld;

	// [v1.0.0] 현재 World에 표시 중인 강한 UI Root 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFUIRootWidget> RootWidget = nullptr;

	// [v1.0.0] 현재 주요 Screen 레이어에 표시 중인 화면입니다.
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PrimaryScreenWidget = nullptr;

	// [v1.0.0] 현재 Modal 레이어에 표시 중인 화면입니다.
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentModalWidget = nullptr;

	// [v1.1.0] 현재 Menu 레이어에 표시 중인 C++ Pause Menu입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFPauseMenuWidget> PauseMenuWidget = nullptr;

	// [v1.1.0] 이 Subsystem이 Pause Menu와 World Pause 수명을 소유하고 있는지 여부입니다.
	bool bSinglePlayerPauseActive = false;

	// [v1.1.0] Pause 진입 직전 복원 대상으로 저장한 화면 상태입니다.
	ECFUIScreenState PrePauseScreenState = ECFUIScreenState::InGame;

	// [v1.0.0] 현재 LocalPlayer UI 화면 상태입니다.
	ECFUIScreenState ScreenState = ECFUIScreenState::None;

				// [v1.7.0] 표준 UI Root를 게임 Viewport에 추가할 Viewport ZOrder입니다.
	int32 RootViewportZOrder = 100;

	// [v1.6.0] 같은 HUD Layer의 Production HUD ZOrder 0보다 위에 Aim Reticle을 배치할 로컬 ZOrder입니다.
	int32 AimReticleHUDLayerZOrder = 10;

	// [v1.7.0] Game Layer 안에서 TargetSelect World Marker에 적용할 로컬 ZOrder입니다.
	int32 TargetSelectGameLayerZOrder = 0;



	// [v1.0.0] 전역 World Cleanup Delegate 등록 핸들입니다.
	FDelegateHandle WorldCleanupDelegateHandle;
};
