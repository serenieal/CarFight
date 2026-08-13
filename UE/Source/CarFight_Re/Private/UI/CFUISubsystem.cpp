// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-08-13
// Description: CarFight LocalPlayer UI 수명·레이어·Pause·Production HUD Runtime 연결 Subsystem 구현
// Scope: 기존 Pause·Root 동작을 보존하면서 UI-P0-03 Provider/Presenter와 Production HUD를 World HUD Layer에 연결합니다.
// Changelog:
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

#include "UI/CFUISubsystem.h"

#include "CFPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFHUDPresenter.h"
#include "UI/CFPauseMenuWidget.h"
#include "UI/CFStyledWidgetBase.h"
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
	ResolvedInGameHUDWidgetClass = nullptr;
	Super::Deinitialize();
}

// [v1.4.0] Config Soft Reference의 Style·Density·HUD Layout을 Subsystem 수명에서 한 번 Load·검증해 Cache합니다.
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

	ResolvedInGameHUDWidgetClass = DefaultInGameHUDWidgetClass.IsNull() ? nullptr : DefaultInGameHUDWidgetClass.LoadSynchronous();
	if (ResolvedInGameHUDWidgetClass && !ResolvedInGameHUDWidgetClass->IsChildOf(UCFStyledWidgetBase::StaticClass()))
	{
		ResolvedInGameHUDWidgetClass = nullptr;
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

		// [v1.3.0] Legacy Aim Reticle·TargetSelect HUD와 동일한 Viewport 계층에서 실제 ZOrder와 Hit Test 우선권을 적용합니다.
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
