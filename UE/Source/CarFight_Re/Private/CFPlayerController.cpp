// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-18
// Description: CarFight UI 공통 입력·Mapping Context·싱글플레이 Pause PlayerController 구현
// Scope: Controller 소유 Context, Pause·Back 요청, 입력 중립화, 실제 World Pause와 UI USER Visual 검증 진입점을 구현합니다.
// Changelog:
// - v1.4.0: M_VehicleDefensePIE 전용 UI-P0-03 USER Visual 검증을 위해 PIE 한정 방어 Pawn 빙의·기준 Pawn 복귀·방어 피해 Exec 진입점을 구현.
// - v1.3.0: Pause 중 Enter·게임패드 확인 버튼을 Continue Fallback 입력으로 처리.
// - v1.1.0: UI-P0-02 차량 입력 중립화, 눌린 키 Flush와 World Pause 적용·해제 API를 구현.
// - v1.0.0: UI-P0-01A 최소 PlayerController 기반을 최초 구현.
// Migration:
// - Legacy Pawn DefaultInputMappingContext는 Controller가 제거하지 않는다.
// - UI 입력 중 Pawn 입력 억제는 기존 Binding과 Context를 삭제하지 않는 임시 호환 게이트다.
// - Pause 진입은 ACFVehiclePawn 입력만 중립화하며 Launcher·Projectile Runtime을 취소하거나 초기화하지 않는다.
// - v1.4.0 UI Visual Exec 진입점은 PIE의 M_VehicleDefensePIE 계열 맵에서만 동작하고 저장 에셋을 변경하지 않는다.




#include "CFPlayerController.h"

#include "CFDamageData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehiclePawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/InputComponent.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"
#include "UI/CFUISubsystem.h"


// [v1.0.0] 게임 입력을 기본 상태로 하고 커서를 숨깁니다.
ACFPlayerController::ACFPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

// [v1.0.0] LocalPlayer UI Subsystem 등록과 Controller 소유 Context를 준비합니다.
void ACFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UCFUISubsystem* UISubsystem = GetUISubsystem())
		{
			UISubsystem->RegisterPlayerController(this);
		}
		RefreshInputMappingContexts();
		NotifyControlledPawnChanged(nullptr, GetPawn());
	}
}

// [v1.0.0] Controller가 등록한 Context와 UI Subsystem 연결을 정리합니다.
void ACFPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalController())
	{
		ApplyGameplayInputSuppression(false);
		RemoveAllOwnedMappingContexts();
		if (UCFUISubsystem* UISubsystem = GetUISubsystem())
		{
			UISubsystem->UnregisterPlayerController(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

// [v1.0.0] Pause·Back Enhanced Input 또는 안전 Fallback Key를 바인딩합니다.
void ACFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent || !IsLocalController())
	{
		return;
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InputAction_Pause)
		{
			EnhancedInputComponent->BindAction(InputAction_Pause, ETriggerEvent::Started, this, &ACFPlayerController::HandlePauseInputAction);
		}
		if (InputAction_Back)
		{
			EnhancedInputComponent->BindAction(InputAction_Back, ETriggerEvent::Started, this, &ACFPlayerController::HandleBackInputAction);
		}
	}

	if (!bBindFallbackSystemKeysWhenActionsMissing)
	{
		return;
	}

	if (!InputAction_Pause)
	{
		FInputKeyBinding& EscapePauseBinding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ACFPlayerController::HandlePauseFallbackKey);
		EscapePauseBinding.bExecuteWhenPaused = true;

		FInputKeyBinding& GamepadPauseBinding = InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &ACFPlayerController::HandlePauseFallbackKey);
		GamepadPauseBinding.bExecuteWhenPaused = true;
	}

		if (!InputAction_Back)
	{
		FInputKeyBinding& KeyboardBackBinding = InputComponent->BindKey(EKeys::BackSpace, IE_Pressed, this, &ACFPlayerController::HandleBackFallbackKey);
		KeyboardBackBinding.bExecuteWhenPaused = true;

		FInputKeyBinding& GamepadBackBinding = InputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &ACFPlayerController::HandleBackFallbackKey);
		GamepadBackBinding.bExecuteWhenPaused = true;
	}

	// [v1.3.0] UI Mapping Action이 없는 P0 Pause Menu에서도 확인 입력으로 Continue를 실행할 키보드 Fallback입니다.
	FInputKeyBinding& KeyboardConfirmBinding = InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ACFPlayerController::HandlePauseConfirmFallbackKey);
	KeyboardConfirmBinding.bExecuteWhenPaused = true;
	KeyboardConfirmBinding.bConsumeInput = false;

	// [v1.3.0] 게임패드 확인 버튼으로도 Pause Menu Continue를 실행할 Fallback입니다.
	FInputKeyBinding& GamepadConfirmBinding = InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &ACFPlayerController::HandlePauseConfirmFallbackKey);
	GamepadConfirmBinding.bExecuteWhenPaused = true;
	GamepadConfirmBinding.bConsumeInput = false;
}

// [v1.0.0] 새 Pawn Possession을 UI와 Gameplay Context 수명에 통지합니다.
void ACFPlayerController::OnPossess(APawn* InPawn)
{
	APawn* PreviousPawn = GetPawn();
	Super::OnPossess(InPawn);

	if (IsLocalController())
	{
		if (bGameplayInputSuppressed && InPawn)
		{
			InPawn->DisableInput(this);
		}
		RefreshInputMappingContexts();
		NotifyControlledPawnChanged(PreviousPawn, InPawn);
	}
}

// [v1.0.0] 기존 Pawn의 입력 억제를 복원하고 Gameplay Context를 제거합니다.
void ACFPlayerController::OnUnPossess()
{
	APawn* PreviousPawn = GetPawn();
	if (IsLocalController() && bGameplayInputSuppressed && PreviousPawn)
	{
		PreviousPawn->EnableInput(this);
	}

	RemoveOwnedMappingContext(RegisteredGameplayContext);
	Super::OnUnPossess();

	if (IsLocalController())
	{
		RefreshInputMappingContexts();
		NotifyControlledPawnChanged(PreviousPawn, nullptr);
	}
}

// [v1.0.0] System은 유지하고 Gameplay와 UI Context를 현재 입력 상태에 맞게 갱신합니다.
void ACFPlayerController::RefreshInputMappingContexts()
{
	if (!IsLocalController())
	{
		return;
	}

	RegisterOwnedMappingContext(SystemInputMappingContext, SystemInputPriority, RegisteredSystemContext);

	if (bUIInputEnabled)
	{
		RemoveOwnedMappingContext(RegisteredGameplayContext);
		RegisterOwnedMappingContext(UIInputMappingContext, UIInputPriority, RegisteredUIContext);
	}
	else
	{
		RemoveOwnedMappingContext(RegisteredUIContext);
		if (GetPawn())
		{
			RegisterOwnedMappingContext(GameplayInputMappingContext, GameplayInputPriority, RegisteredGameplayContext);
		}
		else
		{
			RemoveOwnedMappingContext(RegisteredGameplayContext);
		}
	}
}

// [v1.0.0] UI 전용 상태와 Gameplay 상태 사이에서 Context와 Pawn 입력 억제를 전환합니다.
void ACFPlayerController::SetUIInputEnabled(const bool bEnabled)
{
	bUIInputEnabled = bEnabled;
	ApplyGameplayInputSuppression(bUIInputEnabled);
	RefreshInputMappingContexts();
}

// [v1.0.0] Unreal Input Mode, Focus, Cursor와 Controller 소유 Context를 함께 적용합니다.
void ACFPlayerController::ApplyUIInputMode(const ECFUIInputMode InputMode, UWidget* FocusWidget, const bool bShowCursor)
{
	switch (InputMode)
	{
	case ECFUIInputMode::GameOnly:
	{
		FInputModeGameOnly GameOnlyInputMode;
		GameOnlyInputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(GameOnlyInputMode);
		SetUIInputEnabled(false);
		break;
	}
	case ECFUIInputMode::GameAndUI:
	{
		FInputModeGameAndUI GameAndUIInputMode;
		GameAndUIInputMode.SetHideCursorDuringCapture(false);
		GameAndUIInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if (FocusWidget)
		{
			GameAndUIInputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		}
		SetInputMode(GameAndUIInputMode);
		SetUIInputEnabled(true);
		break;
	}
	case ECFUIInputMode::UIOnly:
	default:
	{
		FInputModeUIOnly UIOnlyInputMode;
		UIOnlyInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if (FocusWidget)
		{
			UIOnlyInputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		}
		SetInputMode(UIOnlyInputMode);
		SetUIInputEnabled(true);
		break;
	}
	}

	bShowMouseCursor = bShowCursor;
	bEnableClickEvents = bShowCursor;
	bEnableMouseOverEvents = bShowCursor;
}

// [v1.1.0] Pause 진입 전 현재 차량 Gameplay 입력과 Controller 키 상태를 중립화합니다.
void ACFPlayerController::PrepareGameplayInputForPause()
{
	if (ACFVehiclePawn* VehiclePawn = Cast<ACFVehiclePawn>(GetPawn()))
	{
		VehiclePawn->ClearGameplayInputForPause();
	}

	FlushPressedKeys();
}

// [v1.1.0] 싱글플레이 World Pause를 적용하거나 해제합니다.
bool ACFPlayerController::SetSinglePlayerPaused(const bool bShouldPause)
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld || CurrentWorld->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	if (bShouldPause)
	{
		PrepareGameplayInputForPause();
	}

	if (CurrentWorld->IsPaused() == bShouldPause)
	{
		FlushPressedKeys();
		return true;
	}

	const bool bPauseRequestAccepted = SetPause(bShouldPause);
	const bool bPauseStateMatchesRequest = CurrentWorld->IsPaused() == bShouldPause;
	if (bPauseRequestAccepted || bPauseStateMatchesRequest)
	{
		FlushPressedKeys();
		return true;
	}

	return false;
}

// [v1.1.0] 현재 World가 Pause 상태인지 반환합니다.
bool ACFPlayerController::IsSinglePlayerPaused() const
{
	const UWorld* CurrentWorld = GetWorld();
	return CurrentWorld && CurrentWorld->IsPaused();
}

// [v1.0.0] Controller가 실제 등록해 소유 중인 Mapping Context 수를 반환합니다.
int32 ACFPlayerController::GetOwnedMappingContextCount() const
{
	int32 OwnedContextCount = 0;
	OwnedContextCount += RegisteredSystemContext.IsValid() ? 1 : 0;
	OwnedContextCount += RegisteredGameplayContext.IsValid() ? 1 : 0;
	OwnedContextCount += RegisteredUIContext.IsValid() ? 1 : 0;
	return OwnedContextCount;
}

// [v1.0.0] 현재 LocalPlayer의 CarFight UI Subsystem을 반환합니다.
UCFUISubsystem* ACFPlayerController::GetUISubsystem() const
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetSubsystem<UCFUISubsystem>() : nullptr;
}

// [v1.4.0] UI-P0-03 USER Visual에서 현재 PIE의 정식 Defense Pawn으로 빙의합니다.
void ACFPlayerController::CFUIVisualPossessDefensePawn()
{
	if (!IsUIVisualAcceptancePIE() || !HasAuthority())
	{
		ClientMessage(TEXT("CFUIVisual: M_VehicleDefensePIE PIE authority에서만 사용할 수 있습니다."));
		return;
	}

	ACFVehiclePawn* DefensePawn = FindUIVisualDefensePawn();
	if (!DefensePawn)
	{
		ClientMessage(TEXT("CFUIVisual: 정식 Defense Pawn을 찾지 못했습니다."));
		return;
	}

	if (GetPawn() != DefensePawn)
	{
		Possess(DefensePawn);
	}

	ClientMessage(FString::Printf(TEXT("CFUIVisual: Defense Pawn 빙의 완료 | %s"), *DefensePawn->GetName()));
}

// [v1.4.0] UI-P0-03 Pawn Rebind USER Visual에서 비방어 기준 Pawn으로 돌아갑니다.
void ACFPlayerController::CFUIVisualPossessBaselinePawn()
{
	if (!IsUIVisualAcceptancePIE() || !HasAuthority())
	{
		ClientMessage(TEXT("CFUIVisual: M_VehicleDefensePIE PIE authority에서만 사용할 수 있습니다."));
		return;
	}

	ACFVehiclePawn* BaselinePawn = FindUIVisualBaselinePawn();
	if (!BaselinePawn)
	{
		ClientMessage(TEXT("CFUIVisual: 비방어 기준 Pawn을 찾지 못했습니다."));
		return;
	}

	if (GetPawn() != BaselinePawn)
	{
		Possess(BaselinePawn);
	}

	ClientMessage(FString::Printf(TEXT("CFUIVisual: Baseline Pawn 빙의 완료 | %s"), *BaselinePawn->GetName()));
}

// [v1.4.0] UI-P0-03 Defense USER Visual에서 방어 차량에 Shield·Armor·Integrity가 모두 변하는 정면 피해를 1회 적용합니다.
void ACFPlayerController::CFUIVisualApplyDefenseDamage()
{
	if (!IsUIVisualAcceptancePIE() || !HasAuthority())
	{
		ClientMessage(TEXT("CFUIVisual: M_VehicleDefensePIE PIE authority에서만 사용할 수 있습니다."));
		return;
	}

	ACFVehiclePawn* DefensePawn = FindUIVisualDefensePawn();
	if (!DefensePawn)
	{
		ClientMessage(TEXT("CFUIVisual: 피해를 적용할 정식 Defense Pawn을 찾지 못했습니다."));
		return;
	}

	UCFDamageData* VisualDamageData = NewObject<UCFDamageData>(this);
	if (!VisualDamageData)
	{
		ClientMessage(TEXT("CFUIVisual: Transient DamageData 생성에 실패했습니다."));
		return;
	}

	VisualDamageData->DamageId = TEXT("UIVisualDefenseDamage");
	VisualDamageData->BaseDamage = 200.0f;
	VisualDamageData->ArmorPenetration = 50.0f;
	VisualDamageData->bCanDamageSelf = false;

	FCFDamageHitContext DamageHitContext;
	DamageHitContext.DamageData = VisualDamageData;
	DamageHitContext.HitActor = DefensePawn;
	DamageHitContext.InstigatorActor = this;
	DamageHitContext.ImpactLocation = DefensePawn->GetActorLocation() + (DefensePawn->GetActorForwardVector() * 100.0f);
	DamageHitContext.ImpactNormal = DefensePawn->GetActorForwardVector();
	DamageHitContext.IncomingDirection = -DefensePawn->GetActorForwardVector();
	DamageHitContext.bBlockingHit = true;

	FCFVehicleDamageResult VehicleDamageResult;
	if (!UCFVehicleDefenseComp::TryApplyDamageToActor(DamageHitContext, VehicleDamageResult))
	{
		ClientMessage(TEXT("CFUIVisual: 정식 VehicleDefense 피해 적용에 실패했습니다."));
		return;
	}

	UCFVehicleDefenseComp* DefenseComponent = DefensePawn->GetVehicleDefenseComp();
	ClientMessage(FString::Printf(
		TEXT("CFUIVisual: Defense 피해 적용 완료 | %s"),
		DefenseComponent ? *DefenseComponent->BuildVehicleDefenseSummary() : TEXT("Defense summary unavailable")));
}

// [v1.4.0] UI Visual Exec 진입점이 허용되는 M_VehicleDefensePIE 계열 PIE World인지 확인합니다.
bool ACFPlayerController::IsUIVisualAcceptancePIE() const
{
	const UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld || CurrentWorld->WorldType != EWorldType::PIE)
	{
		return false;
	}

	return CurrentWorld->GetMapName().Contains(TEXT("M_VehicleDefensePIE"));
}

// [v1.4.0] 현재 PIE World에서 정식 DefenseData가 초기화된 차량 Pawn을 찾습니다.
ACFVehiclePawn* ACFPlayerController::FindUIVisualDefensePawn() const
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		return nullptr;
	}

	for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(CurrentWorld); VehiclePawnIterator; ++VehiclePawnIterator)
	{
		ACFVehiclePawn* CandidatePawn = *VehiclePawnIterator;
		if (!IsValid(CandidatePawn))
		{
			continue;
		}

		UCFVehicleDefenseComp* DefenseComponent = CandidatePawn->GetVehicleDefenseComp();
		if (DefenseComponent && DefenseComponent->IsDefenseInitialized() && DefenseComponent->GetActiveDefenseData())
		{
			return CandidatePawn;
		}
	}

	return nullptr;
}

// [v1.4.0] 현재 PIE World에서 DefenseData가 없는 비방어 기준 차량 Pawn을 찾습니다.
ACFVehiclePawn* ACFPlayerController::FindUIVisualBaselinePawn() const
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		return nullptr;
	}

	for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(CurrentWorld); VehiclePawnIterator; ++VehiclePawnIterator)
	{
		ACFVehiclePawn* CandidatePawn = *VehiclePawnIterator;
		if (!IsValid(CandidatePawn) || CandidatePawn == GetPawn())
		{
			continue;
		}

		UCFVehicleDefenseComp* DefenseComponent = CandidatePawn->GetVehicleDefenseComp();
		if (!DefenseComponent || !DefenseComponent->IsDefenseInitialized() || !DefenseComponent->GetActiveDefenseData())
		{
			return CandidatePawn;
		}
	}

	return nullptr;
}

// [v1.0.0] Enhanced Input Pause Action 입력을 공통 요청으로 변환합니다.
void ACFPlayerController::HandlePauseInputAction(const FInputActionValue& InputActionValue)
{
	(void)InputActionValue;
	BroadcastPauseInputRequest();
}

// [v1.0.0] Enhanced Input Back Action 입력을 공통 요청으로 변환합니다.
void ACFPlayerController::HandleBackInputAction(const FInputActionValue& InputActionValue)
{
	(void)InputActionValue;
	BroadcastBackInputRequest();
}

// [v1.0.0] C++ Fallback Pause Key 입력을 공통 요청으로 변환합니다.
void ACFPlayerController::HandlePauseFallbackKey()
{
	BroadcastPauseInputRequest();
}

// [v1.0.0] C++ Fallback Back Key 입력을 공통 요청으로 변환합니다.
void ACFPlayerController::HandleBackFallbackKey()
{
	BroadcastBackInputRequest();
}

// [v1.3.0] Pause 중 Enter·게임패드 확인 입력을 Continue 요청으로 변환합니다.
void ACFPlayerController::HandlePauseConfirmFallbackKey()
{
	if (UCFUISubsystem* UISubsystem = GetUISubsystem())
	{
		if (UISubsystem->IsSinglePlayerPauseActive())
		{
			UISubsystem->ExitSinglePlayerPause();
		}
	}
}

// [v1.0.0] Pause 요청을 Controller와 UI Subsystem에 한 번씩 전달합니다.
void ACFPlayerController::BroadcastPauseInputRequest()
{
	OnPauseInputRequested.Broadcast();
	if (UCFUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->NotifyPauseInputRequested();
	}
}

// [v1.0.0] Back 요청을 Controller와 UI Subsystem에 한 번씩 전달합니다.
void ACFPlayerController::BroadcastBackInputRequest()
{
	OnBackInputRequested.Broadcast();
	if (UCFUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->NotifyBackInputRequested();
	}
}

// [v1.0.0] Possession 변경을 Controller와 UI Subsystem에 전달합니다.
void ACFPlayerController::NotifyControlledPawnChanged(APawn* PreviousPawn, APawn* NewPawn)
{
	OnControlledPawnChanged.Broadcast(PreviousPawn, NewPawn);
	if (UCFUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->NotifyPossessedPawnChanged(PreviousPawn, NewPawn);
	}
}

// [v1.0.0] 현재 LocalPlayer의 Enhanced Input Subsystem을 반환합니다.
UEnhancedInputLocalPlayerSubsystem* ACFPlayerController::GetEnhancedInputSubsystem() const
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
}

// [v1.0.0] 지정 Context를 Controller 소유로 중복 없이 등록합니다.
bool ACFPlayerController::RegisterOwnedMappingContext(UInputMappingContext* MappingContext, const int32 Priority, TWeakObjectPtr<const UInputMappingContext>& RegisteredContext)
{
	if (!MappingContext)
	{
		RemoveOwnedMappingContext(RegisteredContext);
		return false;
	}

	if (RegisteredContext.Get() == MappingContext)
	{
		return true;
	}

	RemoveOwnedMappingContext(RegisteredContext);

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem();
	if (!EnhancedInputSubsystem)
	{
		return false;
	}

	EnhancedInputSubsystem->AddMappingContext(MappingContext, Priority);
	RegisteredContext = MappingContext;
	return true;
}

// [v1.0.0] Controller가 등록한 지정 Context만 제거합니다.
void ACFPlayerController::RemoveOwnedMappingContext(TWeakObjectPtr<const UInputMappingContext>& RegisteredContext)
{
	const UInputMappingContext* MappingContext = RegisteredContext.Get();
	if (MappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			EnhancedInputSubsystem->RemoveMappingContext(MappingContext);
		}
	}
	RegisteredContext.Reset();
}

// [v1.0.0] Controller가 등록한 모든 Context를 제거합니다.
void ACFPlayerController::RemoveAllOwnedMappingContexts()
{
	RemoveOwnedMappingContext(RegisteredUIContext);
	RemoveOwnedMappingContext(RegisteredGameplayContext);
	RemoveOwnedMappingContext(RegisteredSystemContext);
}

// [v1.0.0] Legacy Pawn Gameplay 입력을 삭제하지 않고 현재 UI 상태에 따라 억제·복원합니다.
void ACFPlayerController::ApplyGameplayInputSuppression(const bool bSuppressGameplayInput)
{
	if (bGameplayInputSuppressed == bSuppressGameplayInput)
	{
		return;
	}

	SetIgnoreMoveInput(bSuppressGameplayInput);
	SetIgnoreLookInput(bSuppressGameplayInput);

	if (APawn* ControlledPawn = GetPawn())
	{
		if (bSuppressGameplayInput)
		{
			ControlledPawn->DisableInput(this);
		}
		else
		{
			ControlledPawn->EnableInput(this);
		}
	}

	bGameplayInputSuppressed = bSuppressGameplayInput;
}
