// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.0
// Date: 2026-08-21
// Description: CarFight LocalPlayer UI 수명·레이어·Pause·Production HUD·Target Marker·Radar Zoom·Screen-edge Relation Visual 연결 Subsystem 구현
// Scope: 기존 UI 수명을 보존하면서 UI-P0-08 Screen-Off Selected Target Edge Marker에 공용 HUD Visual/Style Data를 주입합니다.
// Changelog:
// - v1.10.0: USER 가독성 피드백에 따라 HUDDataProvider와 Friendly/Hostile/Unknown Style 색을 TargetSelect Screen-edge에 주입. Neutral은 Edge에서만 Unknown 회색을 공유하며 전역 NeutralColor는 보존.
// - v1.9.0: DefaultHUDVisualDataAsset을 해석하고 T_UI_RadarEdge 기반 RadarSelectedEdgeBracket·AccentTactical·SafeMargin을 TargetSelect Widget에 주입. WBP_TargetSelect 저장 구조와 Gameplay 선택 상태는 변경하지 않음.
// - v1.8.0: RequestRadarZoomIn/Out을 추가해 Pawn Mouse Wheel 입력이 HUDDataProvider의 Provider-local Range Preset 선택만 변경하도록 연결. Sensor/Scanner Gameplay Range는 수정하지 않음.
// - v1.7.0: Config WBP_TargetSelect을 Game Layer ZOrder 0에 단일 생성하고 Possess·UnPossess·World Cleanup에서 같은 Marker 인스턴스를 현재 Pawn에 재바인딩.
// - v1.6.0: Config WBP_AimReticle을 HUD Layer ZOrder 10에 단일 생성하고 Possess·UnPossess·World Cleanup에서 같은 인스턴스를 현재 Pawn에 재바인딩.
// - v1.5.0: HUDDataProvider/HUDPresenter 생성, Production WBP_CFInGameHUD Config Class 해석과 HUD Layer 수명을 추가.
// - v1.4.0: D1-09A Config Soft Reference 해석, 검증 실패 Fallback과 Style·Density·Layout Getter를 추가.
// - v1.3.0: UI Root를 AddToPlayerScreen에서 AddToViewport로 전환해 Legacy HUD 위 표시와 Pause 마우스 Hit Test를 복구.
// - v1.2.0: Pause 입력 모드의 Focus 대상을 Pause Menu 전체가 아니라 실제 Continue 버튼으로 고정.
// - v1.1.1: Pause 중 Pawn 입력 억제를 유지하면서 PlayerController Pause·Back 입력을 받도록 GameAndUI 모드로 보정.
// - v1.1.0: UI-P0-02 실제 World Pause, C++ Pause Menu, Continue·Pause·Back 전환과 해제 수명을 구현.
// - v1.0.0: UI-P0-01B Root 1개 보장, 약한 Pawn 참조와 입력 모드 연동을 최초 구현.
// Migration:
// - Pause 진입은 차량 입력을 중립화하지만 Launcher·Projectile·Timer Runtime을 취소하거나 초기화하지 않는다.
// - Pause 해제 뒤 Primary Screen이 남아 있으면 UIOnly, 없으면 GameOnly 입력으로 복귀한다.
// - UI Data Config가 비었거나 유효하지 않으면 각 타입의 Native CDO Fallback을 사용하며 Pause·Gameplay 수명에는 영향을 주지 않는다.
// - Production HUD Asset은 재부모화하지 않으며 CFStyledWidgetBase를 유지하고 Provider/Presenter가 Runtime ViewData만 주입합니다.
// - v1.6.0 AimReticle은 기존 WBP_AimReticle/UCFAimReticleWidget 시각·Gameplay 읽기 계약을 그대로 사용하고, 생성·Parent·Pawn Source 수명만 UISubsystem이 소유합니다.
// - v1.7.0 TargetSelect은 기존 후보·선택·TrackState Gameplay 소유권을 유지하고 WBP_TargetSelect의 생성·Game Layer·Pawn Source 수명만 UISubsystem이 소유합니다.
// - v1.8.0 Radar Zoom은 UISubsystem이 새 Range 상태를 소유하지 않고 기존 HUDDataProvider API에만 위임합니다.
// - v1.9.0 Screen-edge World Marker는 UISubsystem이 Visual/Style Data를 해석해 주입하고 TargetSelect Widget은 콘텐츠 경로를 직접 Load하지 않습니다.
// - v1.10.0 관계색은 HUDDataProvider가 이미 판정한 Target.Relation을 소비하며 Friendly/Hostile/Unknown Style Token만 주입합니다. Neutral은 Screen-edge에서만 Unknown 회색을 사용합니다.

#include "UI/CFUISubsystem.h"

#include "CFPlayerController.h"
#include "CFVehiclePawn.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UI/CFAimReticleWidget.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFHUDPresenter.h"
#include "UI/CFHUDVisualData.h"
#include "UI/CFPauseMenuWidget.h"
#include "UI/CFStyledWidgetBase.h"
#include "UI/CFTargetSelectWidget.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIRootWidget.h"
#include "UI/CFUIStyleData.h"

// [v1.5.0] UI Data를 해석하고 HUD Provider/Presenter를 LocalPlayer 수명으로 준비합니다.
void UCFUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResolveDefaultUIDataAssets();

	HUDDataProvider = NewObject<UCFHUDDataProvider>(this);
	if (HUDDataProvider)
	{
		HUDDataProvider->InitializeProvider(this);
	}

	HUDPresenter = NewObject<UCFHUDPresenter>(this);
	if (HUDPresenter && HUDDataProvider)
	{
		HUDPresenter->InitializePresenter(HUDDataProvider);
	}

	WorldCleanupDelegateHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UCFUISubsystem::HandleWorldCleanup);
}

// [v1.1.0] 활성 Pause를 해제하고 현재 Root와 World Cleanup 감시를 정리합니다.
void UCFUISubsystem::Deinitialize()
{
	ReleaseUIRoot();

	if (HUDPresenter)
	{
		HUDPresenter->ShutdownPresenter();
	}
	if (HUDDataProvider)
	{
		HUDDataProvider->ShutdownProvider();
	}
	HUDPresenter = nullptr;
	HUDDataProvider = nullptr;

		if (WorldCleanupDelegateHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupDelegateHandle);
		WorldCleanupDelegateHandle.Reset();
	}

	ActivePlayerController.Reset();
	CurrentPawn.Reset();
		ResolvedStyleData = nullptr;
	ResolvedDensityData = nullptr;
	ResolvedHUDLayoutData = nullptr;
	ResolvedHUDVisualData = nullptr;
		ResolvedInGameHUDWidgetClass = nullptr;
	ResolvedAimReticleWidgetClass = nullptr;
	ResolvedTargetSelectWidgetClass = nullptr;
	Super::Deinitialize();
}

// [v1.9.0] Config Soft Reference의 Style·Density·HUD Layout·HUD Visual을 Subsystem 수명에서 한 번 Load·검증해 Cache합니다.
void UCFUISubsystem::ResolveDefaultUIDataAssets()
{
	ResolvedStyleData = DefaultStyleDataAsset.IsNull() ? nullptr : DefaultStyleDataAsset.LoadSynchronous();
	if (ResolvedStyleData)
	{
		// [v1.4.0] 잘못된 Style DataAsset을 Native Fallback으로 내릴 때 제공할 검증 실패 사유입니다.
		FString StyleValidationFailureReason;
		if (!ResolvedStyleData->ValidateStyleData(StyleValidationFailureReason))
		{
			ResolvedStyleData = nullptr;
		}
	}

		ResolvedDensityData = DefaultDensityDataAsset.IsNull() ? nullptr : DefaultDensityDataAsset.LoadSynchronous();
	if (ResolvedDensityData)
	{
		// [v1.4.0] 잘못된 Density DataAsset을 Native Fallback으로 내릴 때 제공할 검증 실패 사유입니다.
		FString DensityValidationFailureReason;
		if (!ResolvedDensityData->ValidateDensityData(DensityValidationFailureReason))
		{
			ResolvedDensityData = nullptr;
		}
	}

		ResolvedHUDLayoutData = DefaultHUDLayoutDataAsset.IsNull() ? nullptr : DefaultHUDLayoutDataAsset.LoadSynchronous();
	if (ResolvedHUDLayoutData)
	{
		// [v1.4.0] 잘못된 Layout DataAsset을 Native Fallback으로 내릴 때 제공할 검증 실패 사유입니다.
		FString LayoutValidationFailureReason;
		if (!ResolvedHUDLayoutData->ValidateLayoutData(LayoutValidationFailureReason))
		{
			ResolvedHUDLayoutData = nullptr;
		}
		}

	// [v1.9.0] Production HUD와 World Marker가 공유할 공용 HUD Visual DataAsset입니다.
	ResolvedHUDVisualData = DefaultHUDVisualDataAsset.IsNull() ? nullptr : DefaultHUDVisualDataAsset.LoadSynchronous();

	ResolvedInGameHUDWidgetClass = DefaultInGameHUDWidgetClass.IsNull() ? nullptr : DefaultInGameHUDWidgetClass.LoadSynchronous();
	if (ResolvedInGameHUDWidgetClass && !ResolvedInGameHUDWidgetClass->IsChildOf(UCFStyledWidgetBase::StaticClass()))
	{
		ResolvedInGameHUDWidgetClass = nullptr;
	}

		ResolvedAimReticleWidgetClass = DefaultAimReticleWidgetClass.IsNull() ? nullptr : DefaultAimReticleWidgetClass.LoadSynchronous();
	if (ResolvedAimReticleWidgetClass && !ResolvedAimReticleWidgetClass->IsChildOf(UCFAimReticleWidget::StaticClass()))
	{
		ResolvedAimReticleWidgetClass = nullptr;
	}

	ResolvedTargetSelectWidgetClass = DefaultTargetSelectWidgetClass.IsNull() ? nullptr : DefaultTargetSelectWidgetClass.LoadSynchronous();
	if (ResolvedTargetSelectWidgetClass && !ResolvedTargetSelectWidgetClass->IsChildOf(UCFTargetSelectWidget::StaticClass()))
	{
		ResolvedTargetSelectWidgetClass = nullptr;
	}
}

// [v1.4.0] Config에서 해석된 Style Data를 반환하고 없으면 Native CDO Fallback을 반환합니다.
UCFUIStyleData* UCFUISubsystem::GetResolvedStyleData() const
{
	return ResolvedStyleData ? ResolvedStyleData.Get() : GetMutableDefault<UCFUIStyleData>();
}

// [v1.4.0] Config에서 해석된 Density Data를 반환하고 없으면 Native Standard CDO Fallback을 반환합니다.
UCFUIDensityData* UCFUISubsystem::GetResolvedDensityData() const
{
	return ResolvedDensityData ? ResolvedDensityData.Get() : GetMutableDefault<UCFUIDensityData>();
}

// [v1.4.0] Config에서 해석된 HUD Layout Data를 반환하고 없으면 D1-07 1080p Native CDO Fallback을 반환합니다.
UCFHUDLayoutData* UCFUISubsystem::GetResolvedHUDLayoutData() const
{
	return ResolvedHUDLayoutData ? ResolvedHUDLayoutData.Get() : GetMutableDefault<UCFHUDLayoutData>();
}

// [v1.8.0] 현재 LocalPlayer Radar 표시 범위를 한 단계 작은 Provider Preset으로 요청합니다.
bool UCFUISubsystem::RequestRadarZoomIn()
{
	return HUDDataProvider && HUDDataProvider->RequestRadarZoomIn();
}

// [v1.8.0] 현재 LocalPlayer Radar 표시 범위를 한 단계 큰 Provider Preset으로 요청합니다.
bool UCFUISubsystem::RequestRadarZoomOut()
{
	return HUDDataProvider && HUDDataProvider->RequestRadarZoomOut();
}

// [v1.0.0] 현재 LocalPlayer Controller를 등록하고 해당 World용 Root를 보장합니다.
bool UCFUISubsystem::RegisterPlayerController(ACFPlayerController* PlayerController)
{
	if (!PlayerController || PlayerController->GetLocalPlayer() != GetLocalPlayer())
	{
		return false;
	}

	ActivePlayerController = PlayerController;
	NotifyPossessedPawnChanged(CurrentPawn.Get(), PlayerController->GetPawn());
	return EnsureUIRootForWorld(PlayerController->GetWorld());
}

// [v1.1.0] 현재 Pause를 해제한 뒤 LocalPlayer Controller 등록과 Root를 정리합니다.
void UCFUISubsystem::UnregisterPlayerController(ACFPlayerController* PlayerController)
{
	if (PlayerController && ActivePlayerController.IsValid() && ActivePlayerController.Get() != PlayerController)
	{
		return;
	}

	if (bSinglePlayerPauseActive)
	{
		ExitSinglePlayerPause();
	}

	NotifyPossessedPawnChanged(CurrentPawn.Get(), nullptr);
	ReleaseUIRoot();
	ActivePlayerController.Reset();
}

// [v1.0.0] 현재 World에 정확히 하나의 UI Root가 존재하도록 보장합니다.
bool UCFUISubsystem::EnsureUIRootForWorld(UWorld* World)
{
		if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

		if (RootWidget && RootWorld.Get() == World && RootWidget->IsInViewport())
	{
		CreateInGameHUDWidget();
		CreateAimReticleWidget();
		CreateTargetSelectWidget();
		return true;
	}

	ReleaseUIRoot();

	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	if (!PlayerController || PlayerController->GetWorld() != World)
	{
		return false;
	}

	UCFUIRootWidget* CreatedRootWidget = CreateWidget<UCFUIRootWidget>(PlayerController, UCFUIRootWidget::StaticClass());
		if (!CreatedRootWidget || !CreatedRootWidget->EnsureLayerTree())
	{
		return false;
	}

	// [v1.3.0] Legacy TargetSelect HUD와 동일한 Viewport 계층에서 실제 ZOrder와 Hit Test 우선권을 적용합니다.
	CreatedRootWidget->AddToViewport(RootViewportZOrder);
		if (!CreatedRootWidget->IsInViewport())
	{
		return false;
	}

	RootWidget = CreatedRootWidget;
	RootWorld = World;
	PrePauseScreenState = ECFUIScreenState::InGame;
	SetScreenState(ECFUIScreenState::InGame);
	if (!CreateInGameHUDWidget())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CarFight][UI] Production InGame HUD creation skipped or failed; Root/Pause lifetime remains active."));
	}
		if (!CreateAimReticleWidget())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CarFight][UI] Aim Reticle creation skipped or failed; Root/Production HUD lifetime remains active."));
	}
	if (!CreateTargetSelectWidget())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CarFight][UI] Target Marker creation skipped or failed; Root/Production HUD/AimReticle lifetime remains active."));
	}
	return true;
}

// [v1.1.0] 활성 Pause와 현재 World Root의 모든 레이어 자식을 제거합니다.
void UCFUISubsystem::ReleaseUIRoot()
{
	if (bSinglePlayerPauseActive)
	{
		if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			PlayerController->SetSinglePlayerPaused(false);
		}

		bSinglePlayerPauseActive = false;
		OnPauseStateChanged.Broadcast(true, false);
	}

				DestroyPauseMenuWidget();
	DestroyTargetSelectWidget();
	DestroyAimReticleWidget();
	DestroyInGameHUDWidget();
	PrimaryScreenWidget = nullptr;
	CurrentModalWidget = nullptr;

	if (RootWidget)
	{
		RootWidget->ClearAllLayers();
		RootWidget->RemoveFromParent();
		RootWidget = nullptr;
	}

	RootWorld.Reset();
	PrePauseScreenState = ECFUIScreenState::InGame;
	SetScreenState(ECFUIScreenState::None);
}

// [v1.0.0] 현재 UI Root의 지정 레이어 CanvasPanel을 반환합니다.
UCanvasPanel* UCFUISubsystem::GetLayerWidget(const ECFUILayer Layer) const
{
	return RootWidget ? RootWidget->GetLayerWidget(Layer) : nullptr;
}

// [v1.0.0] Widget을 현재 Root의 지정 레이어에 추가합니다.
bool UCFUISubsystem::PushWidgetToLayer(UWidget* WidgetToAdd, const ECFUILayer Layer, const int32 ZOrder)
{
	return RootWidget && RootWidget->AddWidgetToLayer(WidgetToAdd, Layer, ZOrder);
}

// [v1.0.0] 현재 Root의 지정 레이어를 비웁니다.
void UCFUISubsystem::ClearLayer(const ECFUILayer Layer)
{
	if (RootWidget)
	{
		RootWidget->ClearLayer(Layer);
	}
}

// [v1.1.0] Pause가 아닐 때 주요 전체 화면 Widget을 Screen 레이어의 단일 화면으로 전환합니다.
bool UCFUISubsystem::SetPrimaryScreenWidget(UUserWidget* ScreenWidget, const ECFUIInputMode InputMode, const bool bShowMouseCursor)
{
	if (bSinglePlayerPauseActive || !RootWidget || !ScreenWidget)
	{
		return false;
	}

	RootWidget->ClearLayer(ECFUILayer::Screen);
	if (!RootWidget->AddWidgetToLayer(ScreenWidget, ECFUILayer::Screen, 0))
	{
		return false;
	}

	PrimaryScreenWidget = ScreenWidget;
	SetScreenState(ECFUIScreenState::FullScreen);

	if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
	{
		PlayerController->ApplyUIInputMode(InputMode, ScreenWidget, bShowMouseCursor);
	}
	return true;
}

// [v1.1.0] 현재 주요 전체 화면을 닫고 Pause 중이 아니면 인게임 입력으로 복귀합니다.
void UCFUISubsystem::ClearPrimaryScreenWidget()
{
	if (RootWidget)
	{
		RootWidget->ClearLayer(ECFUILayer::Screen);
	}
	PrimaryScreenWidget = nullptr;

	if (bSinglePlayerPauseActive)
	{
		PrePauseScreenState = ECFUIScreenState::InGame;
		SetScreenState(ECFUIScreenState::Paused);
		return;
	}

	if (!CurrentModalWidget)
	{
		SetScreenState(ECFUIScreenState::InGame);
		if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			PlayerController->ApplyUIInputMode(ECFUIInputMode::GameOnly, nullptr, false);
		}
	}
}

// [v1.1.0] Pause가 아닐 때 Modal 레이어에 단일 Modal Widget을 표시합니다.
bool UCFUISubsystem::SetModalWidget(UUserWidget* ModalWidget, const bool bShowMouseCursor)
{
	if (bSinglePlayerPauseActive || !RootWidget || !ModalWidget)
	{
		return false;
	}

	RootWidget->ClearLayer(ECFUILayer::Modal);
	if (!RootWidget->AddWidgetToLayer(ModalWidget, ECFUILayer::Modal, 0))
	{
		return false;
	}

	CurrentModalWidget = ModalWidget;
	SetScreenState(ECFUIScreenState::Modal);

	if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
	{
		PlayerController->ApplyUIInputMode(ECFUIInputMode::UIOnly, ModalWidget, bShowMouseCursor);
	}
	return true;
}

// [v1.1.0] 현재 Modal Widget을 닫고 Pause 또는 이전 화면 성격에 맞는 입력 모드로 복귀합니다.
void UCFUISubsystem::ClearModalWidget()
{
	if (RootWidget)
	{
		RootWidget->ClearLayer(ECFUILayer::Modal);
	}
	CurrentModalWidget = nullptr;

		if (bSinglePlayerPauseActive)
	{
		SetScreenState(ECFUIScreenState::Paused);
				if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			// [v1.2.0] Pause Menu 전체가 아니라 실제 Continue 버튼을 직접 Focus 대상으로 사용합니다.
			UWidget* PauseFocusWidget = PauseMenuWidget;
			if (PauseMenuWidget && PauseMenuWidget->GetContinueButton())
			{
				PauseFocusWidget = PauseMenuWidget->GetContinueButton();
			}
			PlayerController->ApplyUIInputMode(ECFUIInputMode::GameAndUI, PauseFocusWidget, true);
		}
		if (PauseMenuWidget)
		{
			PauseMenuWidget->FocusDefaultButton();
		}
		return;
	}

	if (PrimaryScreenWidget)
	{
		SetScreenState(ECFUIScreenState::FullScreen);
		if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			PlayerController->ApplyUIInputMode(ECFUIInputMode::UIOnly, PrimaryScreenWidget, true);
		}
	}
	else
	{
		SetScreenState(ECFUIScreenState::InGame);
		if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			PlayerController->ApplyUIInputMode(ECFUIInputMode::GameOnly, nullptr, false);
		}
	}
}

// [v1.1.0] 입력을 중립화하고 Menu 레이어 Pause Widget을 만든 뒤 실제 싱글플레이 World Pause를 적용합니다.
bool UCFUISubsystem::EnterSinglePlayerPause()
{
	if (bSinglePlayerPauseActive)
	{
		if (PauseMenuWidget)
		{
			PauseMenuWidget->FocusDefaultButton();
		}
		return true;
	}

	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	UWorld* CurrentWorld = RootWorld.Get();
	if (!PlayerController || !CurrentWorld || CurrentWorld->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	if (!EnsureUIRootForWorld(CurrentWorld) || CurrentModalWidget)
	{
		return false;
	}

	PrePauseScreenState = PrimaryScreenWidget ? ECFUIScreenState::FullScreen : ECFUIScreenState::InGame;
	if (!CreatePauseMenuWidget())
	{
		return false;
	}

	if (!PlayerController->SetSinglePlayerPaused(true))
	{
		DestroyPauseMenuWidget();
		RestoreInputModeAfterPause();
		return false;
	}

	const bool bWasPaused = bSinglePlayerPauseActive;
		bSinglePlayerPauseActive = true;
	SetScreenState(ECFUIScreenState::Paused);
		// [v1.2.0] Pawn Gameplay 입력은 억제하고 실제 Continue 버튼을 GameAndUI Focus 대상으로 사용합니다.
	PlayerController->ApplyUIInputMode(ECFUIInputMode::GameAndUI, PauseMenuWidget->GetContinueButton(), true);
	PauseMenuWidget->FocusDefaultButton();
	OnPauseStateChanged.Broadcast(bWasPaused, true);
	return true;
}

// [v1.1.0] 실제 World Pause와 Pause Menu를 해제하고 이전 화면 입력 상태를 복원합니다.
bool UCFUISubsystem::ExitSinglePlayerPause()
{
	if (!bSinglePlayerPauseActive)
	{
		return true;
	}

	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	if (!PlayerController || !PlayerController->SetSinglePlayerPaused(false))
	{
		return false;
	}

	const bool bWasPaused = bSinglePlayerPauseActive;
	bSinglePlayerPauseActive = false;
	DestroyPauseMenuWidget();
	RestoreInputModeAfterPause();
	OnPauseStateChanged.Broadcast(bWasPaused, false);
	return true;
}

// [v1.1.0] 현재 Pause 상태에 따라 Pause 진입과 해제를 전환합니다.
bool UCFUISubsystem::ToggleSinglePlayerPause()
{
	return bSinglePlayerPauseActive ? ExitSinglePlayerPause() : EnterSinglePlayerPause();
}

// [v1.1.0] Controller의 Pause 공통 입력을 이벤트로 전달하고 Pause 상태를 전환합니다.
void UCFUISubsystem::NotifyPauseInputRequested()
{
	OnPauseInputRequested.Broadcast();
	ToggleSinglePlayerPause();
}

// [v1.1.0] Controller의 Back 입력을 이벤트로 전달하고 Modal 또는 Pause를 우선 닫습니다.
void UCFUISubsystem::NotifyBackInputRequested()
{
	OnBackInputRequested.Broadcast();

	if (CurrentModalWidget)
	{
		ClearModalWidget();
		return;
	}

	if (bSinglePlayerPauseActive)
	{
		ExitSinglePlayerPause();
	}
}

// [v1.0.0] Controller의 Possession 변경을 약한 Pawn 참조와 UI 이벤트에 반영합니다.
void UCFUISubsystem::NotifyPossessedPawnChanged(APawn* PreviousPawn, APawn* NewPawn)
{
	if (PreviousPawn == NewPawn && CurrentPawn.Get() == NewPawn)
	{
		return;
	}

			CurrentPawn = NewPawn;
	RebindAimReticleToCurrentPawn();
	RebindTargetSelectToCurrentPawn();
	OnCurrentPawnChanged.Broadcast(PreviousPawn, NewPawn);
}

// [v1.1.0] C++ Pause Menu를 생성하고 Menu 레이어에 단일 인스턴스로 추가합니다.
bool UCFUISubsystem::CreatePauseMenuWidget()
{
	if (PauseMenuWidget)
	{
		return true;
	}

	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	if (!PlayerController || !RootWidget)
	{
		return false;
	}

	UCFPauseMenuWidget* CreatedPauseMenu = CreateWidget<UCFPauseMenuWidget>(PlayerController, UCFPauseMenuWidget::StaticClass());
	if (!CreatedPauseMenu || !CreatedPauseMenu->EnsurePauseMenuTree())
	{
		return false;
	}

	RootWidget->ClearLayer(ECFUILayer::Menu);
	if (!RootWidget->AddWidgetToLayer(CreatedPauseMenu, ECFUILayer::Menu, 0))
	{
		return false;
	}

	CreatedPauseMenu->OnContinueRequested.RemoveDynamic(this, &UCFUISubsystem::HandlePauseContinueRequested);
	CreatedPauseMenu->OnContinueRequested.AddDynamic(this, &UCFUISubsystem::HandlePauseContinueRequested);
	PauseMenuWidget = CreatedPauseMenu;
	return true;
}

// [v1.1.0] Continue 바인딩과 Menu 레이어의 Pause Menu 인스턴스를 정리합니다.
void UCFUISubsystem::DestroyPauseMenuWidget()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->OnContinueRequested.RemoveDynamic(this, &UCFUISubsystem::HandlePauseContinueRequested);
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}

	if (RootWidget)
	{
		RootWidget->ClearLayer(ECFUILayer::Menu);
	}
}

// [v1.5.0] Config Production Widget Class로 WBP_CFInGameHUD를 만들고 HUD Layer와 Presenter에 연결합니다.
bool UCFUISubsystem::CreateInGameHUDWidget()
{
	if (InGameHUDWidget && InGameHUDWidget->GetParent() == GetLayerWidget(ECFUILayer::HUD))
	{
		if (HUDPresenter)
		{
			HUDPresenter->SetProductionWidget(InGameHUDWidget);
		}
		return true;
	}

	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	if (!PlayerController || !RootWidget || !ResolvedInGameHUDWidgetClass)
	{
		return false;
	}

	UCFStyledWidgetBase* CreatedHUDWidget = CreateWidget<UCFStyledWidgetBase>(PlayerController, ResolvedInGameHUDWidgetClass);
	if (!CreatedHUDWidget)
	{
		return false;
	}

	UCFHUDLayoutData* LayoutData = GetResolvedHUDLayoutData();
	CreatedHUDWidget->SetUIVisualContext(
		GetResolvedStyleData(),
		GetResolvedDensityData(),
		LayoutData ? LayoutData->GeometryScale : 1.0f,
		LayoutData ? LayoutData->TypographyScale : 1.0f,
		LayoutData ? LayoutData->MinimumEffectiveFontSizes : FCFUITypographyFloor());

	if (!RootWidget->AddWidgetToLayer(CreatedHUDWidget, ECFUILayer::HUD, 0))
	{
		CreatedHUDWidget->RemoveFromParent();
		return false;
	}

	InGameHUDWidget = CreatedHUDWidget;
	if (HUDPresenter)
	{
		HUDPresenter->SetProductionWidget(InGameHUDWidget);
	}
	return true;
}

// [v1.5.0] Presenter 연결을 먼저 해제하고 현재 Production HUD Widget을 제거합니다.
void UCFUISubsystem::DestroyInGameHUDWidget()
{
	if (HUDPresenter)
	{
		HUDPresenter->SetProductionWidget(nullptr);
	}

	if (InGameHUDWidget)
	{
		InGameHUDWidget->RemoveFromParent();
		InGameHUDWidget = nullptr;
	}
}

// [v1.6.0] Config Aim Reticle Class로 단일 Widget을 만들고 HUD Layer에 연결합니다.
bool UCFUISubsystem::CreateAimReticleWidget()
{
	if (AimReticleWidget && AimReticleWidget->GetParent() == GetLayerWidget(ECFUILayer::HUD))
	{
		RebindAimReticleToCurrentPawn();
		return true;
	}

	if (AimReticleWidget)
	{
		DestroyAimReticleWidget();
	}

	// [v1.6.0] Aim Reticle을 소유할 현재 LocalPlayer의 PlayerController입니다.
	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	if (!PlayerController || !RootWidget || !ResolvedAimReticleWidgetClass)
	{
		return false;
	}

	// [v1.6.0] 기존 WBP_AimReticle Class로 생성한 UISubsystem 소유 단일 인스턴스입니다.
	UCFAimReticleWidget* CreatedAimReticleWidget = CreateWidget<UCFAimReticleWidget>(PlayerController, ResolvedAimReticleWidgetClass);
	if (!CreatedAimReticleWidget)
	{
		return false;
	}

	if (!RootWidget->AddWidgetToLayer(CreatedAimReticleWidget, ECFUILayer::HUD, AimReticleHUDLayerZOrder))
	{
		CreatedAimReticleWidget->RemoveFromParent();
		return false;
	}

	AimReticleWidget = CreatedAimReticleWidget;
	RebindAimReticleToCurrentPawn();
	return true;
}

// [v1.6.0] 현재 Reticle의 Pawn 참조를 먼저 비운 뒤 HUD Layer에서 제거합니다.
void UCFUISubsystem::DestroyAimReticleWidget()
{
	if (!AimReticleWidget)
	{
		return;
	}

	AimReticleWidget->SetVehiclePawnRef(nullptr);
	AimReticleWidget->RemoveFromParent();
	AimReticleWidget = nullptr;
}

// [v1.6.0] 현재 Possessed Pawn만 Reticle Source로 연결하고 표시 토글을 갱신합니다.
void UCFUISubsystem::RebindAimReticleToCurrentPawn()
{
	if (!AimReticleWidget)
	{
		return;
	}

	// [v1.6.0] Aim Reticle의 Gameplay Source로 허용할 현재 Possessed 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = Cast<ACFVehiclePawn>(CurrentPawn.Get());
	AimReticleWidget->SetVehiclePawnRef(CurrentVehiclePawn);
	AimReticleWidget->SetVisibility(
		CurrentVehiclePawn && CurrentVehiclePawn->ShouldShowAimReticle()
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
}

// [v1.7.0] Config TargetSelect Class로 단일 World Marker Widget을 만들고 Game Layer에 연결합니다.
bool UCFUISubsystem::CreateTargetSelectWidget()
{
		if (TargetSelectWidget && TargetSelectWidget->GetParent() == GetLayerWidget(ECFUILayer::Game))
	{
		ConfigureTargetSelectScreenEdgePresentation();
		RebindTargetSelectToCurrentPawn();
		return true;
	}

	if (TargetSelectWidget)
	{
		DestroyTargetSelectWidget();
	}

	// [v1.7.0] Target Marker를 소유할 현재 LocalPlayer의 PlayerController입니다.
	ACFPlayerController* PlayerController = ActivePlayerController.Get();
	if (!PlayerController || !RootWidget || !ResolvedTargetSelectWidgetClass)
	{
		return false;
	}

	// [v1.7.0] 기존 WBP_TargetSelect Class로 생성한 UISubsystem 소유 단일 World Marker 인스턴스입니다.
	UCFTargetSelectWidget* CreatedTargetSelectWidget = CreateWidget<UCFTargetSelectWidget>(PlayerController, ResolvedTargetSelectWidgetClass);
	if (!CreatedTargetSelectWidget)
	{
		return false;
	}

	if (!RootWidget->AddWidgetToLayer(CreatedTargetSelectWidget, ECFUILayer::Game, TargetSelectGameLayerZOrder))
	{
		CreatedTargetSelectWidget->RemoveFromParent();
		return false;
	}

		TargetSelectWidget = CreatedTargetSelectWidget;
	ConfigureTargetSelectScreenEdgePresentation();
	RebindTargetSelectToCurrentPawn();
	return true;
}

// [v1.7.0] 현재 Target Marker의 Pawn 참조와 Delegate를 먼저 비운 뒤 Game Layer에서 제거합니다.
void UCFUISubsystem::DestroyTargetSelectWidget()
{
	if (!TargetSelectWidget)
	{
		return;
	}

	TargetSelectWidget->SetVehiclePawnRef(nullptr);
	TargetSelectWidget->RemoveFromParent();
	TargetSelectWidget = nullptr;
}

// [v1.7.0] 현재 Possessed Pawn만 Target Marker Source로 연결하고 표시 토글을 갱신합니다.
void UCFUISubsystem::RebindTargetSelectToCurrentPawn()
{
	if (!TargetSelectWidget)
	{
		return;
	}

	// [v1.7.0] Target Marker의 Gameplay Source로 허용할 현재 Possessed 차량 Pawn입니다.
	ACFVehiclePawn* CurrentVehiclePawn = Cast<ACFVehiclePawn>(CurrentPawn.Get());
	TargetSelectWidget->SetVehiclePawnRef(CurrentVehiclePawn);
	TargetSelectWidget->SetVisibility(
		CurrentVehiclePawn && CurrentVehiclePawn->ShouldShowTargetSelectHud()
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
}

// [v1.10.0] 공용 HUD Visual/Style Data와 Provider를 Screen-edge 관계색 Presentation 입력으로 주입합니다.
void UCFUISubsystem::ConfigureTargetSelectScreenEdgePresentation()
{
	if (!TargetSelectWidget)
	{
		return;
	}

	// [v1.9.0] Screen-edge 2-Corner Bracket에 사용할 공용 Radar Edge Texture입니다.
	UTexture2D* EdgeBracketTexture = nullptr;
	if (ResolvedHUDVisualData && !ResolvedHUDVisualData->RadarSelectedEdgeBracket.IsNull())
	{
		EdgeBracketTexture = ResolvedHUDVisualData->RadarSelectedEdgeBracket.LoadSynchronous();
	}

	// [v1.10.0] Friendly/Hostile/Unknown 관계색과 Safe Region을 제공하는 현재 Style Data입니다.
	UCFUIStyleData* StyleData = GetResolvedStyleData();
	// [v1.10.0] Friendly Screen-edge Marker에 사용할 기존 Style Token의 파란색입니다.
	const FLinearColor FriendlyEdgeColor = StyleData ? StyleData->ResolveColor(ECFUIColorToken::Friendly) : FLinearColor(0.325f, 0.663f, 1.0f, 1.0f);
	// [v1.10.0] Hostile Screen-edge Marker에 사용할 기존 Style Token의 빨간색입니다.
	const FLinearColor HostileEdgeColor = StyleData ? StyleData->ResolveColor(ECFUIColorToken::Hostile) : FLinearColor(1.0f, 0.392f, 0.310f, 1.0f);
	// [v1.10.0] Unknown과 Neutral Screen-edge Marker가 공통으로 사용할 기존 Unknown 회색입니다.
	const FLinearColor UnknownEdgeColor = StyleData ? StyleData->ResolveColor(ECFUIColorToken::Unknown) : FLinearColor(0.655f, 0.678f, 0.702f, 1.0f);
	// [v1.9.0] Screen-edge Bracket이 HUD Safe Region 바깥으로 나가지 않도록 적용할 Style Safe Margin입니다.
	const float EdgeSafeInset = StyleData ? StyleData->Spacing.SafeMargin : 0.0f;

	TargetSelectWidget->ConfigureScreenEdgePresentation(
		EdgeBracketTexture,
		HUDDataProvider,
		FriendlyEdgeColor,
		HostileEdgeColor,
		UnknownEdgeColor,
		EdgeSafeInset);
}

// [v1.1.0] Pause 해제 후 Primary Screen 또는 인게임 입력 모드와 화면 상태를 복원합니다.
void UCFUISubsystem::RestoreInputModeAfterPause()
{
	if (PrimaryScreenWidget)
	{
		SetScreenState(ECFUIScreenState::FullScreen);
		if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			PlayerController->ApplyUIInputMode(ECFUIInputMode::UIOnly, PrimaryScreenWidget, true);
		}
	}
	else
	{
		SetScreenState(ECFUIScreenState::InGame);
		if (ACFPlayerController* PlayerController = ActivePlayerController.Get())
		{
			PlayerController->ApplyUIInputMode(ECFUIInputMode::GameOnly, nullptr, false);
		}
	}

	PrePauseScreenState = PrimaryScreenWidget ? ECFUIScreenState::FullScreen : ECFUIScreenState::InGame;
}

// [v1.1.0] Continue 버튼 요청을 싱글플레이 Pause 해제로 연결합니다.
void UCFUISubsystem::HandlePauseContinueRequested()
{
	ExitSinglePlayerPause();
}

// [v1.0.0] 현재 화면 상태를 변경하고 이벤트를 한 번만 방송합니다.
void UCFUISubsystem::SetScreenState(const ECFUIScreenState NewScreenState)
{
	if (ScreenState == NewScreenState)
	{
		return;
	}

	const ECFUIScreenState PreviousState = ScreenState;
	ScreenState = NewScreenState;
	OnScreenStateChanged.Broadcast(PreviousState, ScreenState);
}

// [v1.1.0] 현재 World 정리 시 활성 Pause, Root와 약한 참조를 제거합니다.
void UCFUISubsystem::HandleWorldCleanup(UWorld* World, const bool bSessionEnded, const bool bCleanupResources)
{
	(void)bSessionEnded;
	(void)bCleanupResources;

	if (World && RootWorld.Get() == World)
	{
		NotifyPossessedPawnChanged(CurrentPawn.Get(), nullptr);
		ReleaseUIRoot();
	}
}
