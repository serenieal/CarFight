// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.13.0
// Date: 2026-04-10
// Description: CarFight ???ル㎦??癲ル슓堉곁땟???Pawn ??れ삀?? ??????????열野?(癲ル슓堉곁땟???類λ룱?DA ????筌먲퐢六?????????ㅼ굣????됰슣維??/ ????곸죷 ??????ш낄援???怨뚮옖????/ ????곸죷 ?袁⑸즴??????釉먮폏?遺룹쐺???釉먯뒠??Movement ?釉뚰???癲??????됰슣維??/ VehicleCameraComp Look ????곸죷 ???ㅻ쿋筌???⑤베堉?)

#include "CFVehiclePawn.h"

#include "CFVehicleData.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleDriveComp.h"
#include "CFWheelSyncComp.h"
#include "CarFightVehicleUtils.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"

namespace
{
	// ??????????れ삀???筌ｋ〃泥???도 ??ш끽維뽳쭛?????곷츉??繹먮끏?????モ봼????ш끽維???怨뚮옖甕??????怨좊룴??猷?獄??怨뚮옖????筌뤾퍓???
	struct FCFWheelClassRuntimeSnapshot
	{
		float MaxSteerAngle = 0.0f;
		float MaxBrakeTorque = 0.0f;
		float MaxHandBrakeTorque = 0.0f;
		float WheelRadius = 0.0f;
		float WheelWidth = 0.0f;
		float FrictionForceMultiplier = 0.0f;
		float CorneringStiffness = 0.0f;
		float WheelLoadRatio = 0.0f;
		float SpringRate = 0.0f;
		float SpringPreload = 0.0f;
		float SuspensionMaxRaise = 0.0f;
		float SuspensionMaxDrop = 0.0f;
		bool bAffectedByEngine = false;
		ESweepShape SweepShape = ESweepShape::Raycast;
	};

	// 癲ル슣??????????????れ삀???筌ｋ〃泥???틖 ??ш끽維????筌먲퐢六????ㅺ컼??????怨좊룴??猷?筌뤿뱶?????濚왿몾??????덊렡.
	FCFWheelClassRuntimeSnapshot CaptureWheelClassRuntimeSnapshot(const UChaosVehicleWheel& WheelClassDefaultObject)
	{
		FCFWheelClassRuntimeSnapshot Snapshot;
		Snapshot.MaxSteerAngle = WheelClassDefaultObject.MaxSteerAngle;
		Snapshot.MaxBrakeTorque = WheelClassDefaultObject.MaxBrakeTorque;
		Snapshot.MaxHandBrakeTorque = WheelClassDefaultObject.MaxHandBrakeTorque;
		Snapshot.WheelRadius = WheelClassDefaultObject.WheelRadius;
		Snapshot.WheelWidth = WheelClassDefaultObject.WheelWidth;
		Snapshot.FrictionForceMultiplier = WheelClassDefaultObject.FrictionForceMultiplier;
		Snapshot.CorneringStiffness = WheelClassDefaultObject.CorneringStiffness;
		Snapshot.WheelLoadRatio = WheelClassDefaultObject.WheelLoadRatio;
		Snapshot.SpringRate = WheelClassDefaultObject.SpringRate;
		Snapshot.SpringPreload = WheelClassDefaultObject.SpringPreload;
		Snapshot.SuspensionMaxRaise = WheelClassDefaultObject.SuspensionMaxRaise;
		Snapshot.SuspensionMaxDrop = WheelClassDefaultObject.SuspensionMaxDrop;
		Snapshot.bAffectedByEngine = WheelClassDefaultObject.bAffectedByEngine;
		Snapshot.SweepShape = WheelClassDefaultObject.SweepShape;
		return Snapshot;
	}

	// ???怨좊룴??猷멸강?????潁뺛꺈彛???????????れ삀???筌ｋ〃泥???도 ????????ㅺ컼?얜쑚????嚥▲꺃??繹먮끏????
	void RestoreWheelClassRuntimeSnapshot(UChaosVehicleWheel& WheelClassDefaultObject, const FCFWheelClassRuntimeSnapshot& Snapshot)
	{
		WheelClassDefaultObject.MaxSteerAngle = Snapshot.MaxSteerAngle;
		WheelClassDefaultObject.MaxBrakeTorque = Snapshot.MaxBrakeTorque;
		WheelClassDefaultObject.MaxHandBrakeTorque = Snapshot.MaxHandBrakeTorque;
		WheelClassDefaultObject.WheelRadius = Snapshot.WheelRadius;
		WheelClassDefaultObject.WheelWidth = Snapshot.WheelWidth;
		WheelClassDefaultObject.FrictionForceMultiplier = Snapshot.FrictionForceMultiplier;
		WheelClassDefaultObject.CorneringStiffness = Snapshot.CorneringStiffness;
		WheelClassDefaultObject.WheelLoadRatio = Snapshot.WheelLoadRatio;
		WheelClassDefaultObject.SpringRate = Snapshot.SpringRate;
		WheelClassDefaultObject.SpringPreload = Snapshot.SpringPreload;
		WheelClassDefaultObject.SuspensionMaxRaise = Snapshot.SuspensionMaxRaise;
		WheelClassDefaultObject.SuspensionMaxDrop = Snapshot.SuspensionMaxDrop;
		WheelClassDefaultObject.bAffectedByEngine = Snapshot.bAffectedByEngine;
		WheelClassDefaultObject.SweepShape = Snapshot.SweepShape;
	}

	// 癲ル슓堉곁땟???DA????ш끽維????ш끽維???????쒓랜萸????????????れ삀???筌ｋ〃泥???군 ??ш끽維뽳쭛???낆뒩????筌뤾퍓???
	void ApplyVehicleMovementWheelTuningToWheelClass(
		UChaosVehicleWheel& WheelClassDefaultObject,
		const FCFVehicleMovementConfig& VehicleMovementConfig,
		const bool bIsFrontWheel)
	{
		WheelClassDefaultObject.MaxBrakeTorque =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelMaxBrakeTorque
			: VehicleMovementConfig.RearWheelMaxBrakeTorque;
		WheelClassDefaultObject.WheelRadius =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelRadius
			: VehicleMovementConfig.RearWheelRadius;
		WheelClassDefaultObject.WheelWidth =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelWidth
			: VehicleMovementConfig.RearWheelWidth;
		WheelClassDefaultObject.FrictionForceMultiplier =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelFrictionForceMultiplier
			: VehicleMovementConfig.RearWheelFrictionForceMultiplier;
		WheelClassDefaultObject.CorneringStiffness =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelCorneringStiffness
			: VehicleMovementConfig.RearWheelCorneringStiffness;
		WheelClassDefaultObject.WheelLoadRatio =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelLoadRatio
			: VehicleMovementConfig.RearWheelLoadRatio;
		WheelClassDefaultObject.SpringRate =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSpringRate
			: VehicleMovementConfig.RearWheelSpringRate;
		WheelClassDefaultObject.SpringPreload =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSpringPreload
			: VehicleMovementConfig.RearWheelSpringPreload;
		WheelClassDefaultObject.SuspensionMaxRaise =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSuspensionMaxRaise
			: VehicleMovementConfig.RearWheelSuspensionMaxRaise;
		WheelClassDefaultObject.SuspensionMaxDrop =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSuspensionMaxDrop
			: VehicleMovementConfig.RearWheelSuspensionMaxDrop;
		WheelClassDefaultObject.bAffectedByEngine =
			bIsFrontWheel
			? VehicleMovementConfig.bFrontWheelAffectedByEngine
			: VehicleMovementConfig.bRearWheelAffectedByEngine;
		WheelClassDefaultObject.SweepShape =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSweepShape
			: VehicleMovementConfig.RearWheelSweepShape;

		if (bIsFrontWheel)
		{
			WheelClassDefaultObject.MaxSteerAngle = VehicleMovementConfig.FrontWheelMaxSteerAngle;
			return;
		}

		WheelClassDefaultObject.MaxHandBrakeTorque = VehicleMovementConfig.RearWheelMaxHandBrakeTorque;
	}

	// ???????⑥??StaticMeshComponent??癲ル슓??젆??눀???癰???????猿?????????⑤베肄????筌뤿걩???筌뤾퍓???
	UStaticMeshComponent* FindStaticMeshComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		OwnerActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (UStaticMeshComponent* StaticMeshComp : StaticMeshComponents)
		{
			if (StaticMeshComp && StaticMeshComp->GetFName() == ComponentName)
			{
				return StaticMeshComp;
			}
		}

		return nullptr;
	}

	// Triggered/Completed ????????????곸죷 ????력??袁⑸즴????獄?獄????살씁??癲ル슪?ｇ몭???筌뤾퍓???
	template<typename TriggeredHandlerType, typename CompletedHandlerType>
	void BindTriggeredCompletedInputAction(
		UEnhancedInputComponent* EnhancedInputComponent,
		UInputAction* SourceInputAction,
		ACFVehiclePawn* VehiclePawn,
		TriggeredHandlerType TriggeredHandler,
		CompletedHandlerType CompletedHandler)
	{
		if (!EnhancedInputComponent || !SourceInputAction || !VehiclePawn)
		{
			return;
		}

		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Triggered, VehiclePawn, TriggeredHandler);
		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Completed, VehiclePawn, CompletedHandler);
	}

	// Started/Completed ???????????????곸죷 ????력??袁⑸즴????獄?獄????살씁??癲ル슪?ｇ몭???筌뤾퍓???
	template<typename StartedHandlerType, typename CompletedHandlerType>
	void BindStartedCompletedInputAction(
		UEnhancedInputComponent* EnhancedInputComponent,
		UInputAction* SourceInputAction,
		ACFVehiclePawn* VehiclePawn,
		StartedHandlerType StartedHandler,
		CompletedHandlerType CompletedHandler)
	{
		if (!EnhancedInputComponent || !SourceInputAction || !VehiclePawn)
		{
			return;
		}

		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Started, VehiclePawn, StartedHandler);
		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Completed, VehiclePawn, CompletedHandler);
	}

	// ??れ삀??????????釉먯뒠??????WheelSync ??ш낄援ο쭛??????? ??癰귙끋源????れ삀??????뽮덫????⑤챶援??袁⑸즵????筌뤾퍓???
	FString StripWheelSyncRuntimeSummarySuffix(const FString& RuntimeSummary)
	{
		// ??れ삀??????????釉먯뒠??????WheelSyncBuild ?????? ??筌믨퀣援??嚥▲꺂痢???ш끽維?????낇돲??
		const int32 ExistingWheelSyncBuildIndex = RuntimeSummary.Find(TEXT(" | WheelSyncBuild="));

		// ??れ삀??????????釉먯뒠??????WheelSyncRuntime ?????? ??筌믨퀣援??嚥▲꺂痢???ш끽維?????낇돲??
		const int32 ExistingWheelSyncRuntimeIndex = RuntimeSummary.Find(TEXT(" | WheelSyncRuntime="));

		// ??れ삀???WheelSync ?????濚???좊읈????沃섅굥?? ?????嚥▲꺂痢???筌믨퀣援???ш끽維?????낇돲??
		int32 ExistingWheelSyncSummaryIndex = INDEX_NONE;

		if (ExistingWheelSyncBuildIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = ExistingWheelSyncBuildIndex;
		}
		if (ExistingWheelSyncRuntimeIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = (ExistingWheelSyncSummaryIndex != INDEX_NONE)
				? FMath::Min(ExistingWheelSyncSummaryIndex, ExistingWheelSyncRuntimeIndex)
				: ExistingWheelSyncRuntimeIndex;
		}

		return (ExistingWheelSyncSummaryIndex != INDEX_NONE)
			? RuntimeSummary.Left(ExistingWheelSyncSummaryIndex)
			: RuntimeSummary;
	}
}

ACFVehiclePawn::ACFVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;
	VehicleDriveComp = CreateDefaultSubobject<UCFVehicleDriveComp>(TEXT("VehicleDriveComp"));
	WheelSyncComp = CreateDefaultSubobject<UCFWheelSyncComp>(TEXT("WheelSyncComp"));
	VehicleCameraComp = CreateDefaultSubobject<UCFVehicleCameraComp>(TEXT("VehicleCameraComp"));
	bAutoInitializeOnBeginPlay = true;
	bEnableWheelVisualTick = true;
	bAutoRegisterInputMappingContext = true;
	InputDeviceMode = ECFVehicleInputDeviceMode::Auto;
	InputDeviceAnalogThreshold = 0.1f;
	InputMappingPriority = 0;
	bVehicleRuntimeReady = false;
	LastVehicleRuntimeSummary = TEXT("Constructed");
	bEnableDriveStateOnScreenDebug = false;
	DriveStateDebugDisplayMode = ECFVehicleDebugDisplayMode::SingleLine;
	bShowDriveStateTransitionSummary = true;
	DriveStateDebugMessageDuration = 0.0f;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// [v2.14.0] 입력 자산은 BP/파생 클래스에서 지정한 값을 우선 사용하고, 비어 있을 때만 기본 fallback 자산을 로드합니다.
	if (!DefaultInputMappingContext)
	{
		DefaultInputMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default"));
	}

	if (!InputAction_Throttle)
	{
		InputAction_Throttle = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Throttle.IA_Throttle"));
	}

	if (!InputAction_Steering)
	{
		InputAction_Steering = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Steering.IA_Steering"));
	}

	if (!InputAction_Brake)
	{
		InputAction_Brake = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Brake.IA_Brake"));
	}

	if (!InputAction_Handbrake)
	{
		InputAction_Handbrake = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Handbrake.IA_Handbrake"));
	}

	if (!InputAction_Look)
	{
		InputAction_Look = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_LookAround.IA_LookAround"));
	}
}

// [v2.5.2] Construction 시점에 차체뿐 아니라 휠 메시도 기존 Wheel_Mesh_* 컴포넌트에 적용해 에디터 뷰포트 미리보기를 갱신합니다.
void ACFVehiclePawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVehicleVisualConfig();
	ApplyVehicleWheelVisualConfig();
	ApplyVehicleLayoutConfig();
}

void ACFVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoRegisterInputMappingContext)
	{
		RegisterDefaultInputMappingContext();
	}
	if (bAutoInitializeOnBeginPlay)
	{
		InitializeVehicleRuntime();
	}
}

void ACFVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bEnableWheelVisualTick || !bVehicleRuntimeReady)
	{
		DisplayDriveStateOnScreenDebug();
		return;
	}
	UpdateVehicleWheelVisuals(DeltaSeconds);
	DisplayDriveStateOnScreenDebug();
}

void ACFVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (bAutoRegisterInputMappingContext)
	{
		RegisterDefaultInputMappingContext();
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_VehicleMove,
		this,
		&ACFVehiclePawn::HandleVehicleMoveInput,
		&ACFVehiclePawn::HandleVehicleMoveReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Throttle,
		this,
		&ACFVehiclePawn::HandleThrottleInput,
		&ACFVehiclePawn::HandleThrottleReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Steering,
		this,
		&ACFVehiclePawn::HandleSteeringInput,
		&ACFVehiclePawn::HandleSteeringReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Brake,
		this,
		&ACFVehiclePawn::HandleBrakeInput,
		&ACFVehiclePawn::HandleBrakeReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Look,
		this,
		&ACFVehiclePawn::HandleLookInput,
		&ACFVehiclePawn::HandleLookReleased);
	BindStartedCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Handbrake,
		this,
		&ACFVehiclePawn::HandleHandbrakeStarted,
		&ACFVehiclePawn::HandleHandbrakeCompleted);
}

bool ACFVehiclePawn::RegisterDefaultInputMappingContext()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return false;
	}
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!EnhancedInputSubsystem || !DefaultInputMappingContext)
	{
		return false;
	}
	EnhancedInputSubsystem->AddMappingContext(DefaultInputMappingContext, InputMappingPriority);
	return true;
}

bool ACFVehiclePawn::InitializeVehicleRuntime()
{
	bVehicleRuntimeReady = false;
	LastVehicleRuntimeSummary = TEXT("VehicleRuntime: InitializeStarted");
	ApplyVehicleDataConfig();
	const FString DataConfigSummary = LastVehicleRuntimeSummary;
	const bool bDriveReady = (VehicleDriveComp != nullptr) && VehicleDriveComp->CacheVehicleMovementComponent();
	const bool bWheelSyncReady = PrepareWheelSync();
	bVehicleRuntimeReady = bDriveReady && bWheelSyncReady;
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: Data=%s, Drive=%s, WheelSync=%s, Ready=%s | %s"), VehicleData ? TEXT("Present") : TEXT("Missing"), bDriveReady ? TEXT("Ready") : TEXT("Missing"), bWheelSyncReady ? TEXT("Ready") : TEXT("Missing"), bVehicleRuntimeReady ? TEXT("True") : TEXT("False"), *DataConfigSummary);
	return bVehicleRuntimeReady;
}

bool ACFVehiclePawn::PrepareWheelSync()
{
	if (!WheelSyncComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: WheelSyncComp is null.");
		return false;
	}
	return WheelSyncComp->TryPrepareWheelSync();
}

bool ACFVehiclePawn::UpdateVehicleWheelVisuals(float DeltaSeconds)
{
	if (!WheelSyncComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: WheelSyncComp is null during UpdateVehicleWheelVisuals.");
		return false;
	}
	const bool bUpdated = WheelSyncComp->UpdateWheelVisualsPhase2(DeltaSeconds);
	if (!bUpdated)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: Wheel visual update failed.");
	}
	else
	{
		AppendWheelSyncRuntimeSummary();
	}
	return bUpdated;
}

UChaosWheeledVehicleMovementComponent* ACFVehiclePawn::ResolveVehicleMovementComponent(const TCHAR* CacheFailureSummary, const TCHAR* MissingComponentSummary)
{
	if (!VehicleDriveComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: VehicleDriveComp is null.");
		return nullptr;
	}
	if (!VehicleDriveComp->CacheVehicleMovementComponent())
	{
		LastVehicleRuntimeSummary = CacheFailureSummary;
		return nullptr;
	}
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = VehicleDriveComp->GetVehicleMovementComponent();
	if (!ResolvedVehicleMovementComponent)
	{
		LastVehicleRuntimeSummary = MissingComponentSummary;
		return nullptr;
	}
	return ResolvedVehicleMovementComponent;
}

FString ACFVehiclePawn::BuildVehicleDebugSummary(bool bUseMultilineFormat, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	const FString LineBreak = bUseMultilineFormat ? TEXT("\n") : TEXT(" | ");
	TArray<FString> DebugSegments;
	DebugSegments.Reserve(16);
	DebugSegments.Add(FString::Printf(TEXT("Ready=%s"), bVehicleRuntimeReady ? TEXT("True") : TEXT("False")));
	if (VehicleDriveComp)
	{
		const FCFVehicleDriveStateSnapshot DriveStateSnapshot = VehicleDriveComp->GetDriveStateSnapshot();
		DebugSegments.Add(FString::Printf(TEXT("State=%s"), *UEnum::GetValueAsString(DriveStateSnapshot.CurrentDriveState)));
		DebugSegments.Add(FString::Printf(TEXT("Speed=%.1f km/h"), DriveStateSnapshot.CurrentSpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("ForwardSpeed=%.1f km/h"), DriveStateSnapshot.ForwardSpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("Throttle=%.2f"), DriveStateSnapshot.CurrentInputState.ThrottleInput));
		DebugSegments.Add(FString::Printf(TEXT("Brake=%.2f"), DriveStateSnapshot.CurrentInputState.BrakeInput));
		DebugSegments.Add(FString::Printf(TEXT("Steering=%.2f"), DriveStateSnapshot.CurrentInputState.SteeringInput));
		DebugSegments.Add(FString::Printf(TEXT("Handbrake=%s"), DriveStateSnapshot.CurrentInputState.bHandbrakePressed ? TEXT("On") : TEXT("Off")));
		if (bIncludeInputState)
		{
						DebugSegments.Add(FString::Printf(TEXT("DeviceMode=%s"), *UEnum::GetValueAsString(InputDeviceMode)));
			DebugSegments.Add(FString::Printf(TEXT("InputOwner=%s"), *UEnum::GetValueAsString(CurrentInputOwnership)));
			DebugSegments.Add(FString::Printf(TEXT("MoveZone=%s"), *UEnum::GetValueAsString(LastVehicleMoveInputResult.ResolvedZone)));

			DebugSegments.Add(FString::Printf(TEXT("MoveIntent=%s"), *UEnum::GetValueAsString(LastMoveDirectionIntent)));
			DebugSegments.Add(FString::Printf(TEXT("MoveRaw=(%.2f, %.2f)"), LastVehicleMoveInputResult.RawMoveInput.X, LastVehicleMoveInputResult.RawMoveInput.Y));
			DebugSegments.Add(FString::Printf(TEXT("MoveMag=%.2f"), LastVehicleMoveInputResult.Magnitude));
			DebugSegments.Add(FString::Printf(TEXT("MoveAngle=%.1f"), LastVehicleMoveInputResult.AngleDeg));
			DebugSegments.Add(FString::Printf(TEXT("BlackHold=%s"), LastVehicleMoveInputResult.bUsedBlackZoneHold ? TEXT("True") : TEXT("False")));
		}
	}
	else
	{
		DebugSegments.Add(TEXT("State=DriveCompMissing"));
	}
	if (bIncludeTransitionSummary)
	{
		if (VehicleDriveComp)
		{
			DebugSegments.Add(VehicleDriveComp->GetLastDriveStateTransitionSummary());
		}
		else
		{
			DebugSegments.Add(TEXT("DriveStateTransition: DriveCompMissing"));
		}
	}
	if (bIncludeRuntimeSummary)
	{
		DebugSegments.Add(LastVehicleRuntimeSummary);
	}
	return FString::Join(DebugSegments, *LineBreak);
}

void ACFVehiclePawn::AppendWheelSyncRuntimeSummary()
{
	const FString BaseRuntimeSummary = StripWheelSyncRuntimeSummarySuffix(LastVehicleRuntimeSummary);
	const FString WheelSyncBuildSummary = WheelSyncComp ? WheelSyncComp->LastValidationSummary : TEXT("WheelSyncBuild=MissingWheelSyncComp");
	const FString WheelSyncRuntimeSummary = WheelSyncComp ? WheelSyncComp->LastInputBuildSummary : TEXT("WheelSyncRuntime=MissingWheelSyncComp");
	LastVehicleRuntimeSummary = FString::Printf(TEXT("%s | WheelSyncBuild=%s | WheelSyncRuntime=%s"), *BaseRuntimeSummary, *WheelSyncBuildSummary, *WheelSyncRuntimeSummary);
}

void ACFVehiclePawn::ApplyAxisInputFromAction(const UInputAction* SourceInputAction, const FInputActionValue& InputActionValue, void (ACFVehiclePawn::*AxisInputSetter)(float))
{
	const float AxisValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(SourceInputAction, AxisValue))
	{
		if (CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis)
		{
			(this->*AxisInputSetter)(0.0f);
			ReleaseInputOwnershipIfIdle();
		}
		return;
	}
	if (!CanProcessLegacyAxisInput(AxisValue))
	{
		return;
	}
	UpdateInputOwnershipFromLegacyAxis(AxisValue);
	(this->*AxisInputSetter)(AxisValue);
}


void ACFVehiclePawn::ResetAxisInput(void (ACFVehiclePawn::*AxisInputSetter)(float))
{
	(this->*AxisInputSetter)(0.0f);
}

float ACFVehiclePawn::ConvertMoveInputToAngleDeg(const FVector2D& MoveInputVector) const
{
	const float RawAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(MoveInputVector.X, MoveInputVector.Y));
	return FMath::Fmod(RawAngleDeg + 360.0f, 360.0f);
}

bool ACFVehiclePawn::IsAngleWithinRange(const float InAngleDeg, const float StartAngleDeg, const float EndAngleDeg) const
{
	if (StartAngleDeg <= EndAngleDeg)
	{
		return (InAngleDeg >= StartAngleDeg) && (InAngleDeg <= EndAngleDeg);
	}

	return (InAngleDeg >= StartAngleDeg) || (InAngleDeg <= EndAngleDeg);
}

ECFVehicleMoveDirectionIntent ACFVehiclePawn::ResolveDirectionIntentFallback() const
{
	const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();
	if (DriveStateSnapshot.ForwardSpeedKmh > 0.0f)
	{
		return ECFVehicleMoveDirectionIntent::Forward;
	}
	if (DriveStateSnapshot.ForwardSpeedKmh < 0.0f)
	{
		return ECFVehicleMoveDirectionIntent::Reverse;
	}
	return ECFVehicleMoveDirectionIntent::None;
}

FCFVehicleMoveInputResult ACFVehiclePawn::ResolveVehicleMoveInput(const FVector2D& MoveInputVector) const
{
	FCFVehicleMoveInputResult ResolvedMoveInput;
	ResolvedMoveInput.RawMoveInput = MoveInputVector;
	ResolvedMoveInput.Magnitude = FMath::Clamp(MoveInputVector.Length(), 0.0f, 1.0f);
	ResolvedMoveInput.SteeringValue = FMath::Clamp(MoveInputVector.X, -1.0f, 1.0f);

	if (ResolvedMoveInput.Magnitude <= KINDA_SMALL_NUMBER)
	{
		ResolvedMoveInput.ResolvedDirectionIntent = ResolveDirectionIntentFallback();
		return ResolvedMoveInput;
	}

	ResolvedMoveInput.AngleDeg = ConvertMoveInputToAngleDeg(MoveInputVector);

		const bool bInThrottleZone = IsAngleWithinRange(
		ResolvedMoveInput.AngleDeg,
		VehicleMoveInputConfig.ThrottleStartAngleDeg,
		VehicleMoveInputConfig.ThrottleEndAngleDeg);
	const bool bInReverseZone = IsAngleWithinRange(
		ResolvedMoveInput.AngleDeg,
		VehicleMoveInputConfig.ReverseStartAngleDeg,
		VehicleMoveInputConfig.ReverseEndAngleDeg);

	// [v2.6.1] 차량의 최신 Drive 상태 스냅샷을 가져옵니다.
	const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();

	// [v2.6.1] 후진 전환 시 너무 엄격한 완전 정지 판정 대신,
	// DriveState가 Idle이거나 전방 기준 속도가 아주 낮아진 상태를 후진 허용 구간으로 봅니다.
	const float ReverseBrakeHoldSpeedThresholdKmh = 0.75f;

	// [v2.6.1] 아직 전방으로 의미 있는 속도가 남아 있으면 뒤 입력을 브레이크로 유지합니다.
	const bool bShouldHoldBrakeForForwardMotion =
		(DriveStateSnapshot.CurrentDriveState != ECFVehicleDriveState::Idle)
		&& (DriveStateSnapshot.ForwardSpeedKmh > ReverseBrakeHoldSpeedThresholdKmh);

	// [v2.6.1] 검은 영역 유지 시 사용할 fallback 진행 방향 의도입니다.
	const ECFVehicleMoveDirectionIntent FallbackIntent =
		(LastMoveDirectionIntent != ECFVehicleMoveDirectionIntent::None)
			? LastMoveDirectionIntent
			: ResolveDirectionIntentFallback();

	if (bInThrottleZone)
	{
		ResolvedMoveInput.ResolvedZone = ECFVehicleMoveZone::Throttle;
		ResolvedMoveInput.ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::Forward;
		ResolvedMoveInput.ThrottleValue = ResolvedMoveInput.Magnitude;
		return ResolvedMoveInput;
	}

		if (bInReverseZone)
	{
		ResolvedMoveInput.ResolvedZone = ECFVehicleMoveZone::Reverse;
		if (bShouldHoldBrakeForForwardMotion)
		{
			ResolvedMoveInput.ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::Forward;
			ResolvedMoveInput.BrakeValue = ResolvedMoveInput.Magnitude;
		}
		else
		{
			// [v2.6.3] A안: Chaos Vehicle의 bUseAutoReverse에 후진 전환을 맡기기 위해,
			// 후진 의도도 음수 스로틀 대신 브레이크 입력으로 전달합니다.
			ResolvedMoveInput.ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::Reverse;
			ResolvedMoveInput.BrakeValue = ResolvedMoveInput.Magnitude;
		}
		return ResolvedMoveInput;
	}


	ResolvedMoveInput.ResolvedZone = ECFVehicleMoveZone::Black;
	ResolvedMoveInput.ResolvedDirectionIntent = FallbackIntent;
	ResolvedMoveInput.bUsedBlackZoneHold = true;
	if (FallbackIntent == ECFVehicleMoveDirectionIntent::Forward)
	{
		ResolvedMoveInput.ThrottleValue = ResolvedMoveInput.Magnitude;
	}
	else if (FallbackIntent == ECFVehicleMoveDirectionIntent::Reverse)
	{
		// [v2.6.3] A안: 검은 영역에서도 직전 후진 의도는 브레이크 입력 유지로 전달합니다.
		ResolvedMoveInput.BrakeValue = ResolvedMoveInput.Magnitude;
	}


	return ResolvedMoveInput;
}

void ACFVehiclePawn::ApplyResolvedVehicleMoveInput(const FCFVehicleMoveInputResult& ResolvedMoveInput)
{
	SetVehicleSteeringInput(ResolvedMoveInput.SteeringValue);
	SetVehicleBrakeInput(ResolvedMoveInput.BrakeValue);

	// [v2.6.3] A안: 수동 기어 강제를 제거하고 Chaos Vehicle의 bUseAutoReverse가
	// 브레이크 -> 후진 전환을 직접 처리하도록 둡니다.
	SetVehicleThrottleInput(ResolvedMoveInput.ThrottleValue);

	if (ResolvedMoveInput.ResolvedDirectionIntent != ECFVehicleMoveDirectionIntent::None)
	{
		LastMoveDirectionIntent = ResolvedMoveInput.ResolvedDirectionIntent;
	}
}



void ACFVehiclePawn::ApplyVehicleDataConfig()
{
	ApplyVehicleMovementConfig();
	ApplyVehicleReferenceConfig();
	ApplyVehicleWheelPhysicsConfig();
	ApplyVehicleWheelVisualConfig();
	if (VehicleDriveComp && VehicleData)
	{
		VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig);
	}
}

void ACFVehiclePawn::ApplyVehicleVisualConfig()
{
	if (!VehicleData)
	{
		return;
	}
	UStaticMeshComponent* ChassisStaticMeshComp = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (ChassisStaticMeshComp && VehicleData->VehicleVisualConfig.ChassisMesh)
	{
		ChassisStaticMeshComp->SetStaticMesh(VehicleData->VehicleVisualConfig.ChassisMesh);
	}
	// 현재 WheelSync 컴포넌트에는 휠 메쉬 자산 적용 전용 API가 없습니다.
	// 휠 시각 메쉬 교체는 별도 구현 전까지 여기서 수행하지 않습니다.
}

void ACFVehiclePawn::ApplyVehicleLayoutConfig()
{
	if (!VehicleData)
	{
		return;
	}
	LastVehicleRuntimeSummary = TEXT("VehicleLayout: ManualAnchorLayout=Required");
}

void ACFVehiclePawn::ApplyVehicleMovementConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: VehicleData is null during ApplyVehicleMovementConfig.");
		return;
	}
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = ResolveVehicleMovementComponent(TEXT("VehicleRuntime: DriveComp cache failed during ApplyVehicleMovementConfig."), TEXT("VehicleRuntime: VehicleMovementComponent is null during ApplyVehicleMovementConfig."));
	if (!ResolvedVehicleMovementComponent)
	{
		return;
	}
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;
	ResolvedVehicleMovementComponent->ChassisHeight = VehicleMovementConfig.ChassisHeight;
	ResolvedVehicleMovementComponent->DragCoefficient = VehicleMovementConfig.DragCoefficient;
	ResolvedVehicleMovementComponent->DownforceCoefficient = VehicleMovementConfig.DownforceCoefficient;
	ResolvedVehicleMovementComponent->bEnableCenterOfMassOverride = VehicleMovementConfig.bEnableCenterOfMassOverride;
	ResolvedVehicleMovementComponent->CenterOfMassOverride = VehicleMovementConfig.CenterOfMassOverride;
	ResolvedVehicleMovementComponent->EngineSetup.MaxTorque = VehicleMovementConfig.EngineMaxTorque;
	ResolvedVehicleMovementComponent->EngineSetup.MaxRPM = VehicleMovementConfig.EngineMaxRPM;
	ResolvedVehicleMovementComponent->EngineSetup.EngineIdleRPM = VehicleMovementConfig.EngineIdleRPM;
	ResolvedVehicleMovementComponent->EngineSetup.EngineBrakeEffect = VehicleMovementConfig.EngineBrakeEffect;
	ResolvedVehicleMovementComponent->EngineSetup.EngineRevUpMOI = VehicleMovementConfig.EngineRevUpMOI;
	ResolvedVehicleMovementComponent->EngineSetup.EngineRevDownRate = VehicleMovementConfig.EngineRevDownRate;
	ResolvedVehicleMovementComponent->DifferentialSetup.DifferentialType = VehicleMovementConfig.DifferentialType;
	ResolvedVehicleMovementComponent->DifferentialSetup.FrontRearSplit = VehicleMovementConfig.FrontRearSplit;
	ResolvedVehicleMovementComponent->SteeringSetup.SteeringType = VehicleMovementConfig.SteeringType;
	ResolvedVehicleMovementComponent->SteeringSetup.AngleRatio = VehicleMovementConfig.SteeringAngleRatio;
	ResolvedVehicleMovementComponent->bLegacyWheelFrictionPosition = VehicleMovementConfig.bLegacyWheelFrictionPosition;
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: MovementProfile=%s, MaxTorque=%.1f, MaxRPM=%.1f, Differential=%s, SteeringType=%s"), *VehicleMovementConfig.MovementProfileName.ToString(), VehicleMovementConfig.EngineMaxTorque, VehicleMovementConfig.EngineMaxRPM, *UEnum::GetValueAsString(VehicleMovementConfig.DifferentialType), *UEnum::GetValueAsString(VehicleMovementConfig.SteeringType));
}

void ACFVehiclePawn::ApplyVehicleWheelPhysicsConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: VehicleData is null during ApplyVehicleWheelPhysicsConfig.");
		return;
	}
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = ResolveVehicleMovementComponent(TEXT("VehicleRuntime: DriveComp cache failed during ApplyVehicleWheelPhysicsConfig."), TEXT("VehicleRuntime: VehicleMovementComponent is null during ApplyVehicleWheelPhysicsConfig."));
	if (!ResolvedVehicleMovementComponent)
	{
		return;
	}
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;
	const FCFVehicleReferenceConfig& VehicleReferenceConfig = VehicleData->VehicleReferenceConfig;
	const bool bUseRuntimeWheelPhysicsOverrides = VehicleMovementConfig.bUseMovementOverrides;
	const auto ConfigureWheelSetup = [&](FChaosWheelSetup& WheelSetup, const TSubclassOf<UChaosVehicleWheel> WheelClass, const bool bIsFrontWheel)
	{
		WheelSetup.WheelClass = WheelClass;
		WheelSetup.AdditionalOffset = bIsFrontWheel ? VehicleMovementConfig.FrontWheelAdditionalOffset : VehicleMovementConfig.RearWheelAdditionalOffset;
		if (!bUseRuntimeWheelPhysicsOverrides || !WheelClass)
		{
			return;
		}

		UChaosVehicleWheel* WheelClassDefaultObject = WheelClass->GetDefaultObject<UChaosVehicleWheel>();
		if (!WheelClassDefaultObject)
		{
			return;
		}

		const FCFWheelClassRuntimeSnapshot WheelClassRuntimeSnapshot = CaptureWheelClassRuntimeSnapshot(*WheelClassDefaultObject);
		ApplyVehicleMovementWheelTuningToWheelClass(*WheelClassDefaultObject, VehicleMovementConfig, bIsFrontWheel);
		WheelSetup.WheelClass = WheelClass;
		RestoreWheelClassRuntimeSnapshot(*WheelClassDefaultObject, WheelClassRuntimeSnapshot);
	};

	for (FChaosWheelSetup& WheelSetup : ResolvedVehicleMovementComponent->WheelSetups)
	{
		const FString BoneNameString = WheelSetup.BoneName.ToString();
		const bool bIsFrontWheel = BoneNameString.Contains(TEXT("F"));
		ConfigureWheelSetup(
			WheelSetup,
			bIsFrontWheel ? VehicleReferenceConfig.FrontWheelClass : VehicleReferenceConfig.RearWheelClass,
			bIsFrontWheel);
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: WheelPhysicsOverrides=%s, FrontWheelClass=%s, RearWheelClass=%s, FrontOffset=%s, RearOffset=%s"),
		bUseRuntimeWheelPhysicsOverrides ? TEXT("True") : TEXT("False"),
		VehicleReferenceConfig.FrontWheelClass ? *VehicleReferenceConfig.FrontWheelClass->GetName() : TEXT("None"),
		VehicleReferenceConfig.RearWheelClass ? *VehicleReferenceConfig.RearWheelClass->GetName() : TEXT("None"),
		*VehicleMovementConfig.FrontWheelAdditionalOffset.ToCompactString(),
		*VehicleMovementConfig.RearWheelAdditionalOffset.ToCompactString());
}

// [v2.5.2] VehicleData의 휠 메시 자산을 기존 Wheel_Mesh_* 컴포넌트에 적용하고 WheelSync 기본 시각 설정을 함께 갱신합니다.
void ACFVehiclePawn::ApplyVehicleWheelVisualConfig()
{
	if (!WheelSyncComp || !VehicleData)
	{
		return;
	}

	// VehicleData 기준 WheelSync 기본 설정값을 반영합니다.
	WheelSyncComp->ExpectedWheelCount = VehicleData->WheelVisualConfig.ExpectedWheelCount;
	WheelSyncComp->FrontWheelCountForSteering = VehicleData->WheelVisualConfig.FrontWheelCountForSteering;

	// 앞왼쪽 휠 메시 컴포넌트에 VehicleData의 FL 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshFLComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_FL")))
	{
		WheelMeshFLComp->SetStaticMesh(VehicleData->VehicleVisualConfig.WheelMeshFL);
	}

	// 앞오른쪽 휠 메시 컴포넌트에 VehicleData의 FR 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshFRComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_FR")))
	{
		WheelMeshFRComp->SetStaticMesh(VehicleData->VehicleVisualConfig.WheelMeshFR);
	}

	// 뒤왼쪽 휠 메시 컴포넌트에 VehicleData의 RL 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshRLComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_RL")))
	{
		WheelMeshRLComp->SetStaticMesh(VehicleData->VehicleVisualConfig.WheelMeshRL);
	}

	// 뒤오른쪽 휠 메시 컴포넌트에 VehicleData의 RR 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshRRComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_RR")))
	{
		WheelMeshRRComp->SetStaticMesh(VehicleData->VehicleVisualConfig.WheelMeshRR);
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: WheelVisual ExpectedWheelCount=%d, FrontWheelCount=%d"), WheelSyncComp->ExpectedWheelCount, WheelSyncComp->FrontWheelCountForSteering);
}

void ACFVehiclePawn::ApplyVehicleReferenceConfig()
{
	if (!VehicleData)
	{
		return;
	}
	const FCFVehicleReferenceConfig& VehicleReferenceConfig = VehicleData->VehicleReferenceConfig;
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: FrontWheelClass=%s, RearWheelClass=%s"), VehicleReferenceConfig.FrontWheelClass ? *VehicleReferenceConfig.FrontWheelClass->GetName() : TEXT("None"), VehicleReferenceConfig.RearWheelClass ? *VehicleReferenceConfig.RearWheelClass->GetName() : TEXT("None"));
}

void ACFVehiclePawn::SetVehicleThrottleInput(const float InThrottleValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyThrottleInput(InThrottleValue);
	}
}

void ACFVehiclePawn::SetVehicleSteeringInput(const float InSteeringValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplySteeringInput(InSteeringValue);
	}
}

void ACFVehiclePawn::SetVehicleBrakeInput(const float InBrakeValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyBrakeInput(InBrakeValue);
	}
}

void ACFVehiclePawn::SetVehicleHandbrakeInput(const bool bInHandbrakePressed)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyHandbrakeInput(bInHandbrakePressed);
	}
}

float ACFVehiclePawn::GetVehicleSpeed() const
{
	return VehicleDriveComp ? VehicleDriveComp->GetCurrentSpeedKmh() : 0.0f;
}

ECFVehicleDriveState ACFVehiclePawn::GetDriveState() const
{
	return VehicleDriveComp ? VehicleDriveComp->GetDriveState() : ECFVehicleDriveState::Disabled;
}

FCFVehicleDriveStateSnapshot ACFVehiclePawn::GetDriveStateSnapshot() const
{
	return VehicleDriveComp ? VehicleDriveComp->GetDriveStateSnapshot() : FCFVehicleDriveStateSnapshot();
}

FCFVehicleDebugSnapshot ACFVehiclePawn::GetVehicleDebugSnapshot() const
{
	FCFVehicleDebugSnapshot DebugSnapshot;
	DebugSnapshot.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.RuntimeSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.bHasDriveComponent = (VehicleDriveComp != nullptr);
	DebugSnapshot.bHasWheelSyncComponent = (WheelSyncComp != nullptr);
	if (VehicleDriveComp)
	{
		DebugSnapshot.CurrentDriveState = VehicleDriveComp->GetDriveState();
		DebugSnapshot.PreviousDriveState = VehicleDriveComp->GetPreviousDriveState();
		DebugSnapshot.bDriveStateChangedThisFrame = VehicleDriveComp->HasDriveStateChangedThisFrame();
		DebugSnapshot.DriveStateTransitionSummary = VehicleDriveComp->GetLastDriveStateTransitionSummary();
		DebugSnapshot.DriveStateSnapshot = VehicleDriveComp->GetDriveStateSnapshot();
	}
	return DebugSnapshot;
}

FText ACFVehiclePawn::GetDebugTextSingleLine() const
{
	return FText::FromString(BuildVehicleDebugSummary(false, true, bShowDriveStateTransitionSummary, true));
}

FText ACFVehiclePawn::GetDebugTextMultiLine() const
{
	return FText::FromString(BuildVehicleDebugSummary(true, true, bShowDriveStateTransitionSummary, true));
}

FText ACFVehiclePawn::GetDebugTextByDisplayMode() const
{
	if (DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::Off)
	{
		return FText::GetEmpty();
	}
	if (DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::MultiLine)
	{
		return GetDebugTextMultiLine();
	}
	return GetDebugTextSingleLine();
}

bool ACFVehiclePawn::ShouldShowDebugWidget() const
{
	return bEnableDriveStateOnScreenDebug && (DriveStateDebugDisplayMode != ECFVehicleDebugDisplayMode::Off);
}

ESlateVisibility ACFVehiclePawn::GetDebugWidgetVisibility() const
{
	return ShouldShowDebugWidget() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
}

void ACFVehiclePawn::DisplayDriveStateOnScreenDebug() const
{
	if (!bEnableDriveStateOnScreenDebug || !GEngine)
	{
		return;
	}
	const FString DebugSummary = BuildVehicleDebugSummary(
		DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::MultiLine,
		true,
		bShowDriveStateTransitionSummary,
		true);
	GEngine->AddOnScreenDebugMessage(
		reinterpret_cast<uint64>(this),
		DriveStateDebugMessageDuration,
		FColor::Cyan,
		DebugSummary);
}

bool ACFVehiclePawn::ShouldAcceptActionInput(const UInputAction* SourceInputAction, const float CurrentInputValue) const
{
	if (InputDeviceMode == ECFVehicleInputDeviceMode::Auto)
	{
		return true;
	}
	const bool bRequireGamepadKey = (InputDeviceMode == ECFVehicleInputDeviceMode::GamepadOnly);
	if (FMath::Abs(CurrentInputValue) < InputDeviceAnalogThreshold)
	{
		return false;
	}
	return HasActiveMappedKeyForDevice(SourceInputAction, bRequireGamepadKey);
}

bool ACFVehiclePawn::HasActiveMappedKeyForDevice(const UInputAction* SourceInputAction, const bool bRequireGamepadKey) const
{
	if (!SourceInputAction || !DefaultInputMappingContext)
	{
		return false;
	}
	for (const FEnhancedActionKeyMapping& ActionKeyMapping : DefaultInputMappingContext->GetMappings())
	{
		if (ActionKeyMapping.Action != SourceInputAction)
		{
			continue;
		}
		if (ActionKeyMapping.Key.IsGamepadKey() != bRequireGamepadKey)
		{
			continue;
		}
		if (IsMappedKeyCurrentlyActive(ActionKeyMapping.Key))
		{
			return true;
		}
	}
	return false;
}

bool ACFVehiclePawn::IsMappedKeyCurrentlyActive(const FKey& MappingKey) const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}
	if (PlayerController->IsInputKeyDown(MappingKey))
	{
		return true;
	}
	const float AnalogValue = PlayerController->GetInputAnalogKeyState(MappingKey);
	return FMath::Abs(AnalogValue) >= InputDeviceAnalogThreshold;
}

bool ACFVehiclePawn::IsMeaningfulInputValue(const float CurrentInputValue) const
{
	return FMath::Abs(CurrentInputValue) >= InputDeviceAnalogThreshold;
}

bool ACFVehiclePawn::CanProcessVehicleMoveInput(const float MoveInputMagnitude) const
{
	if (!IsMeaningfulInputValue(MoveInputMagnitude))
	{
		return false;
	}
	if (CurrentInputOwnership != ECFVehicleInputOwnership::LegacyAxis)
	{
		return true;
	}
	if (!GetWorld())
	{
		return true;
	}
	return (GetWorld()->GetTimeSeconds() - LastLegacyAxisInputTimeSec) >= InputOwnershipHoldTimeSec;
}

bool ACFVehiclePawn::CanProcessLegacyAxisInput(const float AxisValue) const
{
	if (!IsMeaningfulInputValue(AxisValue))
	{
		return false;
	}
	if (CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D)
	{
		return true;
	}
	if (!GetWorld())
	{
		return true;
	}
	return (GetWorld()->GetTimeSeconds() - LastVehicleMoveInputTimeSec) >= InputOwnershipHoldTimeSec;
}

void ACFVehiclePawn::UpdateInputOwnershipFromVehicleMove(const float MoveInputMagnitude)
{
	if (!IsMeaningfulInputValue(MoveInputMagnitude))
	{
		return;
	}
	LastVehicleMoveInputTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentInputOwnership = ECFVehicleInputOwnership::VehicleMove2D;
}

void ACFVehiclePawn::UpdateInputOwnershipFromLegacyAxis(const float AxisValue)
{
	if (!IsMeaningfulInputValue(AxisValue))
	{
		return;
	}
	LastLegacyAxisInputTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentInputOwnership = ECFVehicleInputOwnership::LegacyAxis;
}

void ACFVehiclePawn::ReleaseInputOwnershipIfIdle()
{
	if (!GetWorld())
	{
		return;
	}
	const float CurrentTimeSec = GetWorld()->GetTimeSeconds();
	const bool bVehicleMoveExpired = (LastVehicleMoveInputTimeSec < 0.0f) || ((CurrentTimeSec - LastVehicleMoveInputTimeSec) >= InputOwnershipHoldTimeSec);
	const bool bLegacyAxisExpired = (LastLegacyAxisInputTimeSec < 0.0f) || ((CurrentTimeSec - LastLegacyAxisInputTimeSec) >= InputOwnershipHoldTimeSec);
	if (bVehicleMoveExpired && bLegacyAxisExpired)
	{
		CurrentInputOwnership = ECFVehicleInputOwnership::None;
	}
}

void ACFVehiclePawn::HandleVehicleMoveInput(const FInputActionValue& InputActionValue)
{
	const FVector2D MoveInputVector = InputActionValue.Get<FVector2D>();
	const float MoveInputMagnitude = FMath::Clamp(MoveInputVector.Length(), 0.0f, 1.0f);
	if (!ShouldAcceptActionInput(InputAction_VehicleMove, MoveInputMagnitude) || !CanProcessVehicleMoveInput(MoveInputMagnitude))
	{
		LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
		LastVehicleMoveInputResult.RawMoveInput = MoveInputVector;
		LastVehicleMoveInputResult.Magnitude = MoveInputMagnitude;
		ReleaseInputOwnershipIfIdle();
		return;
	}

	UpdateInputOwnershipFromVehicleMove(MoveInputMagnitude);
	LastVehicleMoveInputResult = ResolveVehicleMoveInput(MoveInputVector);
	ApplyResolvedVehicleMoveInput(LastVehicleMoveInputResult);
}


void ACFVehiclePawn::HandleVehicleMoveReleased(const FInputActionValue&)
{
	LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
	if (CurrentInputOwnership == ECFVehicleInputOwnership::VehicleMove2D)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleThrottleInput);
		ResetAxisInput(&ACFVehiclePawn::SetVehicleBrakeInput);
		ResetAxisInput(&ACFVehiclePawn::SetVehicleSteeringInput);
		CurrentInputOwnership = ECFVehicleInputOwnership::None;
	}
	ReleaseInputOwnershipIfIdle();
}

void ACFVehiclePawn::HandleThrottleInput(const FInputActionValue& InputActionValue)
{
	ApplyAxisInputFromAction(InputAction_Throttle, InputActionValue, &ACFVehiclePawn::SetVehicleThrottleInput);
}

void ACFVehiclePawn::HandleThrottleReleased(const FInputActionValue&)
{
	if (CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleThrottleInput);
	}
	ReleaseInputOwnershipIfIdle();
}

void ACFVehiclePawn::HandleSteeringInput(const FInputActionValue& InputActionValue)
{
	ApplyAxisInputFromAction(InputAction_Steering, InputActionValue, &ACFVehiclePawn::SetVehicleSteeringInput);
}

void ACFVehiclePawn::HandleSteeringReleased(const FInputActionValue&)
{
	if (CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleSteeringInput);
	}
	ReleaseInputOwnershipIfIdle();
}

void ACFVehiclePawn::HandleBrakeInput(const FInputActionValue& InputActionValue)
{
	ApplyAxisInputFromAction(InputAction_Brake, InputActionValue, &ACFVehiclePawn::SetVehicleBrakeInput);
}

void ACFVehiclePawn::HandleBrakeReleased(const FInputActionValue&)
{
	if (CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleBrakeInput);
	}
	ReleaseInputOwnershipIfIdle();
}


void ACFVehiclePawn::HandleLookInput(const FInputActionValue& InputActionValue)
{
	if (!VehicleCameraComp)
	{
		return;
	}

	const FVector2D LookInputValue = InputActionValue.Get<FVector2D>();
	VehicleCameraComp->SetLookInput(LookInputValue);
}

void ACFVehiclePawn::HandleLookReleased(const FInputActionValue&)
{
	if (!VehicleCameraComp)
	{
		return;
	}

	VehicleCameraComp->ClearLookInput();
}

void ACFVehiclePawn::HandleHandbrakeStarted(const FInputActionValue&)
{
	if (!ShouldAcceptActionInput(InputAction_Handbrake, 1.0f))
	{
		SetVehicleHandbrakeInput(false);
		return;
	}
	SetVehicleHandbrakeInput(true);
}

void ACFVehiclePawn::HandleHandbrakeCompleted(const FInputActionValue&)
{
	SetVehicleHandbrakeInput(false);
}
