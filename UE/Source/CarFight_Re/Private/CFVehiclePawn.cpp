// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.10.7
// Date: 2026-03-30
// Description: CarFight 신규 차량 Pawn 기준 클래스 구현

#include "CFVehiclePawn.h"

#include "CFVehicleData.h"
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
	// 이름으로 StaticMeshComponent를 찾아 시각 표현 컴포넌트를 식별합니다.
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
}

ACFVehiclePawn::ACFVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;
	VehicleDriveComp = CreateDefaultSubobject<UCFVehicleDriveComp>(TEXT("VehicleDriveComp"));
	WheelSyncComp = CreateDefaultSubobject<UCFWheelSyncComp>(TEXT("WheelSyncComp"));
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
	DefaultInputMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default"));
	InputAction_Throttle = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Throttle.IA_Throttle"));
	InputAction_Steering = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Steering.IA_Steering"));
	InputAction_Brake = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Brake.IA_Brake"));
	InputAction_Handbrake = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Handbrake.IA_Handbrake"));
}

void ACFVehiclePawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVehicleVisualConfig();
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

	if (InputAction_Throttle)
	{
		EnhancedInputComponent->BindAction(InputAction_Throttle, ETriggerEvent::Triggered, this, &ACFVehiclePawn::HandleThrottleInput);
		EnhancedInputComponent->BindAction(InputAction_Throttle, ETriggerEvent::Completed, this, &ACFVehiclePawn::HandleThrottleReleased);
	}
	if (InputAction_Steering)
	{
		EnhancedInputComponent->BindAction(InputAction_Steering, ETriggerEvent::Triggered, this, &ACFVehiclePawn::HandleSteeringInput);
		EnhancedInputComponent->BindAction(InputAction_Steering, ETriggerEvent::Completed, this, &ACFVehiclePawn::HandleSteeringReleased);
	}
	if (InputAction_Brake)
	{
		EnhancedInputComponent->BindAction(InputAction_Brake, ETriggerEvent::Triggered, this, &ACFVehiclePawn::HandleBrakeInput);
		EnhancedInputComponent->BindAction(InputAction_Brake, ETriggerEvent::Completed, this, &ACFVehiclePawn::HandleBrakeReleased);
	}
	if (InputAction_Handbrake)
	{
		EnhancedInputComponent->BindAction(InputAction_Handbrake, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleHandbrakeStarted);
		EnhancedInputComponent->BindAction(InputAction_Handbrake, ETriggerEvent::Completed, this, &ACFVehiclePawn::HandleHandbrakeCompleted);
	}
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
		// 기존 런타임 요약에서 WheelSyncBuild 접미사가 시작되는 위치입니다.
		const int32 ExistingWheelSyncBuildIndex = LastVehicleRuntimeSummary.Find(TEXT(" | WheelSyncBuild="));

		// 기존 런타임 요약에서 WheelSyncRuntime 접미사가 시작되는 위치입니다.
		const int32 ExistingWheelSyncRuntimeIndex = LastVehicleRuntimeSummary.Find(TEXT(" | WheelSyncRuntime="));

		// 기존 WheelSync 접미사 중 가장 먼저 나타나는 시작 위치입니다.
		int32 ExistingWheelSyncSummaryIndex = INDEX_NONE;
		if (ExistingWheelSyncBuildIndex != INDEX_NONE && ExistingWheelSyncRuntimeIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = FMath::Min(ExistingWheelSyncBuildIndex, ExistingWheelSyncRuntimeIndex);
		}
		else if (ExistingWheelSyncBuildIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = ExistingWheelSyncBuildIndex;
		}
		else if (ExistingWheelSyncRuntimeIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = ExistingWheelSyncRuntimeIndex;
		}

		// WheelSync 접미사를 제외한 기본 런타임 요약 문자열입니다.
		const FString BaseRuntimeSummary = (ExistingWheelSyncSummaryIndex != INDEX_NONE)
			? LastVehicleRuntimeSummary.Left(ExistingWheelSyncSummaryIndex)
			: LastVehicleRuntimeSummary;

		// 현재 프레임 입력 생성 단계 요약 문자열입니다.
		const FString WheelSyncBuildSummary = WheelSyncComp->LastInputBuildSummary.IsEmpty()
			? TEXT("NotBuilt")
			: WheelSyncComp->LastInputBuildSummary;

		// 현재 프레임 적용 단계 요약 문자열입니다.
		const FString WheelSyncRuntimeSummary = WheelSyncComp->LastValidationSummary.IsEmpty()
			? TEXT("NotApplied")
			: WheelSyncComp->LastValidationSummary;

		LastVehicleRuntimeSummary = FString::Printf(
			TEXT("%s | WheelSyncBuild=%s | WheelSyncRuntime=%s"),
			*BaseRuntimeSummary,
			*WheelSyncBuildSummary,
			*WheelSyncRuntimeSummary);
	}
	return bUpdated;
}

void ACFVehiclePawn::SetVehicleThrottleInput(float InThrottleValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyThrottleInput(InThrottleValue);
	}
}

void ACFVehiclePawn::SetVehicleSteeringInput(float InSteeringValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplySteeringInput(InSteeringValue);
	}
}

void ACFVehiclePawn::SetVehicleBrakeInput(float InBrakeValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyBrakeInput(InBrakeValue);
	}
}

void ACFVehiclePawn::SetVehicleHandbrakeInput(bool bInHandbrakePressed)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyHandbrakeInput(bInHandbrakePressed);
	}
}

float ACFVehiclePawn::GetVehicleSpeed() const
{
	if (!VehicleDriveComp)
	{
		return 0.0f;
	}

	return VehicleDriveComp->GetCurrentSpeedKmh();
}

ECFVehicleDriveState ACFVehiclePawn::GetDriveState() const
{
	if (!VehicleDriveComp)
	{
		return ECFVehicleDriveState::Disabled;
	}

	return VehicleDriveComp->GetDriveState();
}

FCFVehicleDriveStateSnapshot ACFVehiclePawn::GetDriveStateSnapshot() const
{
	if (!VehicleDriveComp)
	{
		return FCFVehicleDriveStateSnapshot();
	}

	return VehicleDriveComp->GetDriveStateSnapshot();
}

FCFVehicleDebugSnapshot ACFVehiclePawn::GetVehicleDebugSnapshot() const
{
	FCFVehicleDebugSnapshot VehicleDebugSnapshot;
	VehicleDebugSnapshot.bRuntimeReady = bVehicleRuntimeReady;
	VehicleDebugSnapshot.RuntimeSummary = LastVehicleRuntimeSummary;
	VehicleDebugSnapshot.bHasDriveComponent = (VehicleDriveComp != nullptr);
	VehicleDebugSnapshot.bHasWheelSyncComponent = (WheelSyncComp != nullptr);

	if (!VehicleDriveComp)
	{
		return VehicleDebugSnapshot;
	}

	VehicleDebugSnapshot.CurrentDriveState = VehicleDriveComp->GetDriveState();
	VehicleDebugSnapshot.PreviousDriveState = VehicleDriveComp->GetPreviousDriveState();
	VehicleDebugSnapshot.bDriveStateChangedThisFrame = VehicleDriveComp->HasDriveStateChangedThisFrame();
	VehicleDebugSnapshot.DriveStateTransitionSummary = VehicleDriveComp->GetLastDriveStateTransitionSummary();
	VehicleDebugSnapshot.DriveStateSnapshot = VehicleDriveComp->GetDriveStateSnapshot();
	return VehicleDebugSnapshot;
}

FText ACFVehiclePawn::GetDebugTextSingleLine() const
{
	const FCFVehicleDebugSnapshot VehicleDebugSnapshot = GetVehicleDebugSnapshot();
	const FString DebugSummary = UCarFightVehicleUtils::MakeVehicleDebugSnapshotDebugString(
		VehicleDebugSnapshot,
		false,
		true,
		true);

	return FText::FromString(DebugSummary);
}

FText ACFVehiclePawn::GetDebugTextMultiLine() const
{
	const FCFVehicleDebugSnapshot VehicleDebugSnapshot = GetVehicleDebugSnapshot();
	const FString DebugSummary = UCarFightVehicleUtils::MakeVehicleDebugSnapshotMultilineDebugString(
		VehicleDebugSnapshot,
		true,
		true,
		true);

	return FText::FromString(DebugSummary);
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
	return bEnableDriveStateOnScreenDebug && DriveStateDebugDisplayMode != ECFVehicleDebugDisplayMode::Off;
}

ESlateVisibility ACFVehiclePawn::GetDebugWidgetVisibility() const
{
	return ShouldShowDebugWidget() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
}

void ACFVehiclePawn::ApplyVehicleDataConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleDataConfig: VehicleData is null.");
		return;
	}
	ApplyVehicleVisualConfig();
	const FString VehicleVisualSummary = LastVehicleRuntimeSummary;
	ApplyVehicleLayoutConfig();
	const FString VehicleLayoutSummary = LastVehicleRuntimeSummary;
	ApplyVehicleMovementConfig();
	const FString VehicleMovementSummary = LastVehicleRuntimeSummary;
	ApplyVehicleReferenceConfig();
	const FString VehicleReferenceSummary = LastVehicleRuntimeSummary;
	ApplyVehicleWheelVisualConfig();
	const FString WheelVisualSummary = LastVehicleRuntimeSummary;
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig);
	}
	LastVehicleRuntimeSummary = FString::Printf(TEXT("%s | %s | %s | %s | %s | DriveStateConfig=%s"), *VehicleVisualSummary, *VehicleLayoutSummary, *VehicleMovementSummary, *VehicleReferenceSummary, *WheelVisualSummary, VehicleData->DriveStateConfig.bUseDriveStateOverrides ? TEXT("Applied") : TEXT("Default"));
}

void ACFVehiclePawn::ApplyVehicleVisualConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleVisualConfig: VehicleData is null.");
		return;
	}
	UStaticMeshComponent* ChassisMeshComp = FindStaticMeshComponentByName(this, TEXT("SM_Chassis"));
	UStaticMeshComponent* WheelMeshCompFL = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_FL"));
	UStaticMeshComponent* WheelMeshCompFR = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_FR"));
	UStaticMeshComponent* WheelMeshCompRL = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_RL"));
	UStaticMeshComponent* WheelMeshCompRR = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_RR"));
	if (!ChassisMeshComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleVisualConfig: SM_Chassis component is missing.");
		return;
	}
	UStaticMesh* ChassisMesh = VehicleData->VehicleVisualConfig.ChassisMesh.Get();
	UStaticMesh* WheelMeshFL = VehicleData->VehicleVisualConfig.WheelMeshFL.Get();
	UStaticMesh* WheelMeshFR = VehicleData->VehicleVisualConfig.WheelMeshFR.Get() ? VehicleData->VehicleVisualConfig.WheelMeshFR.Get() : WheelMeshFL;
	UStaticMesh* WheelMeshRL = VehicleData->VehicleVisualConfig.WheelMeshRL.Get() ? VehicleData->VehicleVisualConfig.WheelMeshRL.Get() : WheelMeshFL;
	UStaticMesh* WheelMeshRR = VehicleData->VehicleVisualConfig.WheelMeshRR.Get() ? VehicleData->VehicleVisualConfig.WheelMeshRR.Get() : WheelMeshFL;
	ChassisMeshComp->SetStaticMesh(ChassisMesh);
	ChassisMeshComp->MarkRenderStateDirty();
	int32 AppliedWheelMeshCount = 0;
	int32 MissingWheelMeshComponentCount = 0;
	auto ApplyWheelMesh = [&](UStaticMeshComponent* WheelMeshComp, UStaticMesh* WheelMeshAsset)
	{
		if (!WheelMeshComp)
		{
			++MissingWheelMeshComponentCount;
			return;
		}
		WheelMeshComp->SetStaticMesh(WheelMeshAsset);
		WheelMeshComp->MarkRenderStateDirty();
		if (WheelMeshAsset)
		{
			++AppliedWheelMeshCount;
		}
	};
	ApplyWheelMesh(WheelMeshCompFL, WheelMeshFL);
	ApplyWheelMesh(WheelMeshCompFR, WheelMeshFR);
	ApplyWheelMesh(WheelMeshCompRL, WheelMeshRL);
	ApplyWheelMesh(WheelMeshCompRR, WheelMeshRR);
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleVisualConfig: Chassis=%s, WheelMeshesApplied=%d, MissingWheelMeshComponents=%d"), ChassisMesh ? TEXT("Present") : TEXT("Missing"), AppliedWheelMeshCount, MissingWheelMeshComponentCount);
}

void ACFVehiclePawn::ApplyVehicleLayoutConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleLayout: VehicleData is null.");
		return;
	}

	LastVehicleRuntimeSummary = TEXT("VehicleLayout: ManualAnchorLayout=Required");
}

void ACFVehiclePawn::ApplyVehicleMovementConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleMovementConfig: VehicleData is null.");
		return;
	}
	if (!VehicleDriveComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleMovementConfig: VehicleDriveComp is null.");
		return;
	}
	UChaosWheeledVehicleMovementComponent* ChaosVehicleMovementComp = VehicleDriveComp->GetVehicleMovementComponent();
	if (!ChaosVehicleMovementComp && !VehicleDriveComp->CacheVehicleMovementComponent())
	{
		LastVehicleRuntimeSummary = TEXT("VehicleMovementConfig: VehicleMovement cache failed.");
		return;
	}
	ChaosVehicleMovementComp = VehicleDriveComp->GetVehicleMovementComponent();
	if (!ChaosVehicleMovementComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleMovementConfig: VehicleMovement component is null.");
		return;
	}

	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;
	if (!VehicleMovementConfig.bUseMovementOverrides)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleMovementConfig: Override disabled.");
		return;
	}

	int32 AppliedMovementFieldCount = 0;
	int32 AppliedWheelFieldCount = 0;
	int32 AppliedBodyFieldCount = 0;
	int32 AppliedEngineFieldCount = 0;
	int32 AppliedDifferentialFieldCount = 0;
	int32 AppliedSteeringFieldCount = 0;

	UChaosVehicleWheel* FrontWheelClassDefaultObject = VehicleData->VehicleReferenceConfig.FrontWheelClass ? VehicleData->VehicleReferenceConfig.FrontWheelClass->GetDefaultObject<UChaosVehicleWheel>() : nullptr;
	UChaosVehicleWheel* RearWheelClassDefaultObject = VehicleData->VehicleReferenceConfig.RearWheelClass ? VehicleData->VehicleReferenceConfig.RearWheelClass->GetDefaultObject<UChaosVehicleWheel>() : nullptr;
	const int32 FrontWheelSetupCount = FMath::Max(0, VehicleData->WheelVisualConfig.FrontWheelCountForSteering);

	#define APPLY_FRONT_BOOL_FLOAT(BoolField, TargetField, ValueField, NullMsg) if (VehicleMovementConfig.BoolField) { if (!FrontWheelClassDefaultObject) { LastVehicleRuntimeSummary = TEXT(NullMsg); return; } FrontWheelClassDefaultObject->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedWheelFieldCount; }
	#define APPLY_REAR_BOOL_FLOAT(BoolField, TargetField, ValueField, NullMsg) if (VehicleMovementConfig.BoolField) { if (!RearWheelClassDefaultObject) { LastVehicleRuntimeSummary = TEXT(NullMsg); return; } RearWheelClassDefaultObject->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedWheelFieldCount; }
	#define APPLY_FRONT_BOOL_BOOL(BoolField, TargetField, ValueField, NullMsg) if (VehicleMovementConfig.BoolField) { if (!FrontWheelClassDefaultObject) { LastVehicleRuntimeSummary = TEXT(NullMsg); return; } FrontWheelClassDefaultObject->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedWheelFieldCount; }
	#define APPLY_REAR_BOOL_BOOL(BoolField, TargetField, ValueField, NullMsg) if (VehicleMovementConfig.BoolField) { if (!RearWheelClassDefaultObject) { LastVehicleRuntimeSummary = TEXT(NullMsg); return; } RearWheelClassDefaultObject->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedWheelFieldCount; }
	#define APPLY_FRONT_BOOL_ENUM(BoolField, TargetField, ValueField, NullMsg) if (VehicleMovementConfig.BoolField) { if (!FrontWheelClassDefaultObject) { LastVehicleRuntimeSummary = TEXT(NullMsg); return; } FrontWheelClassDefaultObject->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedWheelFieldCount; }
	#define APPLY_REAR_BOOL_ENUM(BoolField, TargetField, ValueField, NullMsg) if (VehicleMovementConfig.BoolField) { if (!RearWheelClassDefaultObject) { LastVehicleRuntimeSummary = TEXT(NullMsg); return; } RearWheelClassDefaultObject->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedWheelFieldCount; }
	#define APPLY_BODY_BOOL_FLOAT(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedBodyFieldCount; }
	#define APPLY_BODY_BOOL_BOOL(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedBodyFieldCount; }
	#define APPLY_BODY_BOOL_VECTOR(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedBodyFieldCount; }
	#define APPLY_ENGINE_BOOL_FLOAT(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->EngineSetup.TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedEngineFieldCount; }
	#define APPLY_DIFF_BOOL_FLOAT(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->DifferentialSetup.TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedDifferentialFieldCount; }
	#define APPLY_DIFF_BOOL_ENUM(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->DifferentialSetup.TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedDifferentialFieldCount; }
	#define APPLY_STEER_BOOL_FLOAT(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->SteeringSetup.TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedSteeringFieldCount; }
	#define APPLY_STEER_BOOL_ENUM(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->SteeringSetup.TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedSteeringFieldCount; }
	#define APPLY_SETUP_BOOL_BOOL(BoolField, TargetField, ValueField) if (VehicleMovementConfig.BoolField) { ChaosVehicleMovementComp->TargetField = VehicleMovementConfig.ValueField; ++AppliedMovementFieldCount; ++AppliedBodyFieldCount; }
	#define APPLY_FRONT_SETUP_OFFSET(BoolField, ValueField) if (VehicleMovementConfig.BoolField) { for (int32 WheelSetupIndex = 0; WheelSetupIndex < ChaosVehicleMovementComp->WheelSetups.Num(); ++WheelSetupIndex) { if (WheelSetupIndex < FrontWheelSetupCount) { ChaosVehicleMovementComp->WheelSetups[WheelSetupIndex].AdditionalOffset = VehicleMovementConfig.ValueField; } } ++AppliedMovementFieldCount; ++AppliedBodyFieldCount; }
	#define APPLY_REAR_SETUP_OFFSET(BoolField, ValueField) if (VehicleMovementConfig.BoolField) { for (int32 WheelSetupIndex = 0; WheelSetupIndex < ChaosVehicleMovementComp->WheelSetups.Num(); ++WheelSetupIndex) { if (WheelSetupIndex >= FrontWheelSetupCount) { ChaosVehicleMovementComp->WheelSetups[WheelSetupIndex].AdditionalOffset = VehicleMovementConfig.ValueField; } } ++AppliedMovementFieldCount; ++AppliedBodyFieldCount; }

	APPLY_BODY_BOOL_FLOAT(bOverrideChassisHeight, ChassisHeight, ChassisHeight);
	APPLY_BODY_BOOL_FLOAT(bOverrideDragCoefficient, DragCoefficient, DragCoefficient);
	APPLY_BODY_BOOL_FLOAT(bOverrideDownforceCoefficient, DownforceCoefficient, DownforceCoefficient);
	APPLY_BODY_BOOL_BOOL(bOverrideEnableCenterOfMassOverride, bEnableCenterOfMassOverride, bEnableCenterOfMassOverride);
	APPLY_BODY_BOOL_VECTOR(bOverrideCenterOfMassOverride, CenterOfMassOverride, CenterOfMassOverride);
	APPLY_SETUP_BOOL_BOOL(bOverrideLegacyWheelFrictionPosition, bLegacyWheelFrictionPosition, bLegacyWheelFrictionPosition);
	APPLY_FRONT_SETUP_OFFSET(bOverrideFrontWheelAdditionalOffset, FrontWheelAdditionalOffset);
	APPLY_REAR_SETUP_OFFSET(bOverrideRearWheelAdditionalOffset, RearWheelAdditionalOffset);
	APPLY_ENGINE_BOOL_FLOAT(bOverrideEngineMaxTorque, MaxTorque, EngineMaxTorque);
	APPLY_ENGINE_BOOL_FLOAT(bOverrideEngineMaxRPM, MaxRPM, EngineMaxRPM);
	APPLY_ENGINE_BOOL_FLOAT(bOverrideEngineIdleRPM, EngineIdleRPM, EngineIdleRPM);
	APPLY_ENGINE_BOOL_FLOAT(bOverrideEngineBrakeEffect, EngineBrakeEffect, EngineBrakeEffect);
	APPLY_ENGINE_BOOL_FLOAT(bOverrideEngineRevUpMOI, EngineRevUpMOI, EngineRevUpMOI);
	APPLY_ENGINE_BOOL_FLOAT(bOverrideEngineRevDownRate, EngineRevDownRate, EngineRevDownRate);
	APPLY_DIFF_BOOL_ENUM(bOverrideDifferentialType, DifferentialType, DifferentialType);
	APPLY_DIFF_BOOL_FLOAT(bOverrideFrontRearSplit, FrontRearSplit, FrontRearSplit);
	APPLY_STEER_BOOL_ENUM(bOverrideSteeringType, SteeringType, SteeringType);
	APPLY_STEER_BOOL_FLOAT(bOverrideSteeringAngleRatio, AngleRatio, SteeringAngleRatio);
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelMaxSteerAngle, MaxSteerAngle, FrontWheelMaxSteerAngle, "VehicleMovementConfig: FrontWheelClass default object is null.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelMaxBrakeTorque, MaxBrakeTorque, FrontWheelMaxBrakeTorque, "VehicleMovementConfig: FrontWheelClass default object is null for MaxBrakeTorque.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelMaxBrakeTorque, MaxBrakeTorque, RearWheelMaxBrakeTorque, "VehicleMovementConfig: RearWheelClass default object is null for MaxBrakeTorque.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelMaxHandBrakeTorque, MaxHandBrakeTorque, RearWheelMaxHandBrakeTorque, "VehicleMovementConfig: RearWheelClass default object is null for MaxHandBrakeTorque.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelRadius, WheelRadius, FrontWheelRadius, "VehicleMovementConfig: FrontWheelClass default object is null for WheelRadius.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelRadius, WheelRadius, RearWheelRadius, "VehicleMovementConfig: RearWheelClass default object is null for WheelRadius.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelWidth, WheelWidth, FrontWheelWidth, "VehicleMovementConfig: FrontWheelClass default object is null for WheelWidth.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelWidth, WheelWidth, RearWheelWidth, "VehicleMovementConfig: RearWheelClass default object is null for WheelWidth.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelFrictionForceMultiplier, FrictionForceMultiplier, FrontWheelFrictionForceMultiplier, "VehicleMovementConfig: FrontWheelClass default object is null for FrictionForceMultiplier.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelFrictionForceMultiplier, FrictionForceMultiplier, RearWheelFrictionForceMultiplier, "VehicleMovementConfig: RearWheelClass default object is null for FrictionForceMultiplier.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelCorneringStiffness, CorneringStiffness, FrontWheelCorneringStiffness, "VehicleMovementConfig: FrontWheelClass default object is null for CorneringStiffness.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelCorneringStiffness, CorneringStiffness, RearWheelCorneringStiffness, "VehicleMovementConfig: RearWheelClass default object is null for CorneringStiffness.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelLoadRatio, WheelLoadRatio, FrontWheelLoadRatio, "VehicleMovementConfig: FrontWheelClass default object is null for WheelLoadRatio.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelLoadRatio, WheelLoadRatio, RearWheelLoadRatio, "VehicleMovementConfig: RearWheelClass default object is null for WheelLoadRatio.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelSpringRate, SpringRate, FrontWheelSpringRate, "VehicleMovementConfig: FrontWheelClass default object is null for SpringRate.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelSpringRate, SpringRate, RearWheelSpringRate, "VehicleMovementConfig: RearWheelClass default object is null for SpringRate.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelSpringPreload, SpringPreload, FrontWheelSpringPreload, "VehicleMovementConfig: FrontWheelClass default object is null for SpringPreload.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelSpringPreload, SpringPreload, RearWheelSpringPreload, "VehicleMovementConfig: RearWheelClass default object is null for SpringPreload.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelSuspensionMaxRaise, SuspensionMaxRaise, FrontWheelSuspensionMaxRaise, "VehicleMovementConfig: FrontWheelClass default object is null for SuspensionMaxRaise.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelSuspensionMaxRaise, SuspensionMaxRaise, RearWheelSuspensionMaxRaise, "VehicleMovementConfig: RearWheelClass default object is null for SuspensionMaxRaise.");
	APPLY_FRONT_BOOL_FLOAT(bOverrideFrontWheelSuspensionMaxDrop, SuspensionMaxDrop, FrontWheelSuspensionMaxDrop, "VehicleMovementConfig: FrontWheelClass default object is null for SuspensionMaxDrop.");
	APPLY_REAR_BOOL_FLOAT(bOverrideRearWheelSuspensionMaxDrop, SuspensionMaxDrop, RearWheelSuspensionMaxDrop, "VehicleMovementConfig: RearWheelClass default object is null for SuspensionMaxDrop.");
	APPLY_FRONT_BOOL_BOOL(bOverrideFrontWheelAffectedByEngine, bAffectedByEngine, bFrontWheelAffectedByEngine, "VehicleMovementConfig: FrontWheelClass default object is null for bAffectedByEngine.");
	APPLY_REAR_BOOL_BOOL(bOverrideRearWheelAffectedByEngine, bAffectedByEngine, bRearWheelAffectedByEngine, "VehicleMovementConfig: RearWheelClass default object is null for bAffectedByEngine.");
	APPLY_FRONT_BOOL_ENUM(bOverrideFrontWheelSweepShape, SweepShape, FrontWheelSweepShape, "VehicleMovementConfig: FrontWheelClass default object is null for SweepShape.");
	APPLY_REAR_BOOL_ENUM(bOverrideRearWheelSweepShape, SweepShape, RearWheelSweepShape, "VehicleMovementConfig: RearWheelClass default object is null for SweepShape.");

	#undef APPLY_FRONT_BOOL_FLOAT
	#undef APPLY_REAR_BOOL_FLOAT
	#undef APPLY_FRONT_BOOL_BOOL
	#undef APPLY_REAR_BOOL_BOOL
	#undef APPLY_FRONT_BOOL_ENUM
	#undef APPLY_REAR_BOOL_ENUM
	#undef APPLY_BODY_BOOL_FLOAT
	#undef APPLY_BODY_BOOL_BOOL
	#undef APPLY_BODY_BOOL_VECTOR
	#undef APPLY_ENGINE_BOOL_FLOAT
	#undef APPLY_DIFF_BOOL_FLOAT
	#undef APPLY_DIFF_BOOL_ENUM
	#undef APPLY_STEER_BOOL_FLOAT
	#undef APPLY_STEER_BOOL_ENUM
	#undef APPLY_SETUP_BOOL_BOOL
	#undef APPLY_FRONT_SETUP_OFFSET
	#undef APPLY_REAR_SETUP_OFFSET

	if (AppliedMovementFieldCount > 0)
	{
		LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleMovementConfig: AppliedFields=%d, Wheel=%d, Body=%d, Engine=%d, Differential=%d, Steering=%d, ChassisHeight=%.2f, Drag=%.2f, Downforce=%.2f, COMEnabled=%s, LegacyWheelFriction=%s, EngineMaxTorque=%.2f, EngineMaxRPM=%.2f, DifferentialType=%d, FrontRearSplit=%.2f, SteeringType=%d, AngleRatio=%.2f, FrontOffset=(%.2f,%.2f,%.2f), RearOffset=(%.2f,%.2f,%.2f), FrontSteer=%.2f, FrontBrake=%.2f, RearBrake=%.2f, RearHandBrake=%.2f, FrontRadius=%.2f, RearRadius=%.2f, FrontWidth=%.2f, RearWidth=%.2f, FrontFriction=%.2f, RearFriction=%.2f, FrontCornering=%.2f, RearCornering=%.2f, FrontLoadRatio=%.2f, RearLoadRatio=%.2f, FrontSpringRate=%.2f, RearSpringRate=%.2f, FrontSpringPreload=%.2f, RearSpringPreload=%.2f, FrontRaise=%.2f, RearRaise=%.2f, FrontDrop=%.2f, RearDrop=%.2f, FrontEngine=%s, RearEngine=%s, FrontSweep=%d, RearSweep=%d, Profile=%s"), AppliedMovementFieldCount, AppliedWheelFieldCount, AppliedBodyFieldCount, AppliedEngineFieldCount, AppliedDifferentialFieldCount, AppliedSteeringFieldCount, VehicleMovementConfig.ChassisHeight, VehicleMovementConfig.DragCoefficient, VehicleMovementConfig.DownforceCoefficient, VehicleMovementConfig.bEnableCenterOfMassOverride ? TEXT("True") : TEXT("False"), VehicleMovementConfig.bLegacyWheelFrictionPosition ? TEXT("True") : TEXT("False"), VehicleMovementConfig.EngineMaxTorque, VehicleMovementConfig.EngineMaxRPM, static_cast<int32>(VehicleMovementConfig.DifferentialType), VehicleMovementConfig.FrontRearSplit, static_cast<int32>(VehicleMovementConfig.SteeringType), VehicleMovementConfig.SteeringAngleRatio, VehicleMovementConfig.FrontWheelAdditionalOffset.X, VehicleMovementConfig.FrontWheelAdditionalOffset.Y, VehicleMovementConfig.FrontWheelAdditionalOffset.Z, VehicleMovementConfig.RearWheelAdditionalOffset.X, VehicleMovementConfig.RearWheelAdditionalOffset.Y, VehicleMovementConfig.RearWheelAdditionalOffset.Z, VehicleMovementConfig.FrontWheelMaxSteerAngle, VehicleMovementConfig.FrontWheelMaxBrakeTorque, VehicleMovementConfig.RearWheelMaxBrakeTorque, VehicleMovementConfig.RearWheelMaxHandBrakeTorque, VehicleMovementConfig.FrontWheelRadius, VehicleMovementConfig.RearWheelRadius, VehicleMovementConfig.FrontWheelWidth, VehicleMovementConfig.RearWheelWidth, VehicleMovementConfig.FrontWheelFrictionForceMultiplier, VehicleMovementConfig.RearWheelFrictionForceMultiplier, VehicleMovementConfig.FrontWheelCorneringStiffness, VehicleMovementConfig.RearWheelCorneringStiffness, VehicleMovementConfig.FrontWheelLoadRatio, VehicleMovementConfig.RearWheelLoadRatio, VehicleMovementConfig.FrontWheelSpringRate, VehicleMovementConfig.RearWheelSpringRate, VehicleMovementConfig.FrontWheelSpringPreload, VehicleMovementConfig.RearWheelSpringPreload, VehicleMovementConfig.FrontWheelSuspensionMaxRaise, VehicleMovementConfig.RearWheelSuspensionMaxRaise, VehicleMovementConfig.FrontWheelSuspensionMaxDrop, VehicleMovementConfig.RearWheelSuspensionMaxDrop, VehicleMovementConfig.bFrontWheelAffectedByEngine ? TEXT("True") : TEXT("False"), VehicleMovementConfig.bRearWheelAffectedByEngine ? TEXT("True") : TEXT("False"), static_cast<int32>(VehicleMovementConfig.FrontWheelSweepShape), static_cast<int32>(VehicleMovementConfig.RearWheelSweepShape), VehicleMovementConfig.MovementProfileName.IsNone() ? TEXT("None") : *VehicleMovementConfig.MovementProfileName.ToString());
		return;
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleMovementConfig: Override enabled, Profile=%s, Status=PendingDetailedFields"), VehicleMovementConfig.MovementProfileName.IsNone() ? TEXT("None") : *VehicleMovementConfig.MovementProfileName.ToString());
}

void ACFVehiclePawn::ApplyVehicleWheelVisualConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("WheelVisualConfig: VehicleData is null.");
		return;
	}
	if (!WheelSyncComp)
	{
		LastVehicleRuntimeSummary = TEXT("WheelVisualConfig: WheelSyncComp is null.");
		return;
	}
	if (VehicleData->WheelVisualConfig.bUseWheelVisualOverrides)
	{
		WheelSyncComp->ExpectedWheelCount = VehicleData->WheelVisualConfig.ExpectedWheelCount;
		WheelSyncComp->FrontWheelCountForSteering = VehicleData->WheelVisualConfig.FrontWheelCountForSteering;
		LastVehicleRuntimeSummary = FString::Printf(TEXT("WheelVisualConfig: Applied ExpectedWheelCount=%d, FrontWheelCountForSteering=%d"), WheelSyncComp->ExpectedWheelCount, WheelSyncComp->FrontWheelCountForSteering);
		return;
	}
	LastVehicleRuntimeSummary = TEXT("WheelVisualConfig: Override disabled.");
}

void ACFVehiclePawn::ApplyVehicleReferenceConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleReferenceConfig: VehicleData is null.");
		return;
	}
	if (!VehicleDriveComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleReferenceConfig: VehicleDriveComp is null.");
		return;
	}
	UChaosWheeledVehicleMovementComponent* ChaosVehicleMovementComp = VehicleDriveComp->GetVehicleMovementComponent();
	if (!ChaosVehicleMovementComp && !VehicleDriveComp->CacheVehicleMovementComponent())
	{
		LastVehicleRuntimeSummary = TEXT("VehicleReferenceConfig: VehicleMovement cache failed.");
		return;
	}
	ChaosVehicleMovementComp = VehicleDriveComp->GetVehicleMovementComponent();
	if (!ChaosVehicleMovementComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleReferenceConfig: VehicleMovement component is null.");
		return;
	}
	const bool bHasFrontWheelClass = (VehicleData->VehicleReferenceConfig.FrontWheelClass != nullptr);
	const bool bHasRearWheelClass = (VehicleData->VehicleReferenceConfig.RearWheelClass != nullptr);
	const int32 FrontWheelCount = FMath::Max(0, VehicleData->WheelVisualConfig.FrontWheelCountForSteering);
	const int32 WheelSetupCount = ChaosVehicleMovementComp->WheelSetups.Num();
	if (WheelSetupCount <= 0)
	{
		LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleReferenceConfig: WheelSetups missing. FrontWheelClass=%s, RearWheelClass=%s"), bHasFrontWheelClass ? TEXT("Present") : TEXT("Missing"), bHasRearWheelClass ? TEXT("Present") : TEXT("Missing"));
		return;
	}
	int32 AppliedFrontCount = 0;
	int32 AppliedRearCount = 0;
	for (int32 WheelSetupIndex = 0; WheelSetupIndex < WheelSetupCount; ++WheelSetupIndex)
	{
		if (WheelSetupIndex < FrontWheelCount)
		{
			if (bHasFrontWheelClass)
			{
				ChaosVehicleMovementComp->WheelSetups[WheelSetupIndex].WheelClass = VehicleData->VehicleReferenceConfig.FrontWheelClass;
				++AppliedFrontCount;
			}
		}
		else if (bHasRearWheelClass)
		{
			ChaosVehicleMovementComp->WheelSetups[WheelSetupIndex].WheelClass = VehicleData->VehicleReferenceConfig.RearWheelClass;
			++AppliedRearCount;
		}
	}
	ChaosVehicleMovementComp->RecreatePhysicsState();
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleReferenceConfig: AppliedFront=%d, AppliedRear=%d, WheelSetups=%d, IndexPolicy=FrontThenRear"), AppliedFrontCount, AppliedRearCount, WheelSetupCount);
}

void ACFVehiclePawn::DisplayDriveStateOnScreenDebug() const
{
	if (!bEnableDriveStateOnScreenDebug || !GEngine || DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::Off)
	{
		return;
	}

	const FCFVehicleDebugSnapshot VehicleDebugSnapshot = GetVehicleDebugSnapshot();
	FString DriveDebugSummary;

	if (DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::MultiLine)
	{
		DriveDebugSummary = UCarFightVehicleUtils::MakeVehicleDebugSnapshotMultilineDebugString(
			VehicleDebugSnapshot,
			false,
			false,
			true);
	}
	else
	{
		DriveDebugSummary = UCarFightVehicleUtils::MakeVehicleDebugSnapshotDebugString(
			VehicleDebugSnapshot,
			false,
			false,
			true);
	}

	GEngine->AddOnScreenDebugMessage(
		reinterpret_cast<uint64>(this),
		DriveStateDebugMessageDuration,
		FColor::White,
		DriveDebugSummary,
		false);

	if (bShowDriveStateTransitionSummary)
	{
		GEngine->AddOnScreenDebugMessage(
			reinterpret_cast<uint64>(this) + 1,
			DriveStateDebugMessageDuration,
			FColor::White,
			VehicleDebugSnapshot.DriveStateTransitionSummary,
			false);
	}
}

bool ACFVehiclePawn::ShouldAcceptActionInput(const UInputAction* SourceInputAction, float CurrentInputValue) const
{
	if (InputDeviceMode == ECFVehicleInputDeviceMode::Auto)
	{
		return true;
	}
	if (!SourceInputAction || FMath::IsNearlyZero(CurrentInputValue))
	{
		return true;
	}
	const bool bRequireGamepadKey = (InputDeviceMode == ECFVehicleInputDeviceMode::GamepadOnly);
	return HasActiveMappedKeyForDevice(SourceInputAction, bRequireGamepadKey);
}

bool ACFVehiclePawn::HasActiveMappedKeyForDevice(const UInputAction* SourceInputAction, bool bRequireGamepadKey) const
{
	if (!DefaultInputMappingContext || !SourceInputAction)
	{
		return false;
	}
	const TArray<FEnhancedActionKeyMapping>& InputMappings = DefaultInputMappingContext->GetMappings();
	for (const FEnhancedActionKeyMapping& InputMapping : InputMappings)
	{
		if (InputMapping.Action != SourceInputAction)
		{
			continue;
		}
		const bool bIsGamepadKey = InputMapping.Key.IsGamepadKey();
		if (bIsGamepadKey != bRequireGamepadKey)
		{
			continue;
		}
		if (IsMappedKeyCurrentlyActive(InputMapping.Key))
		{
			return true;
		}
	}
	return false;
}

bool ACFVehiclePawn::IsMappedKeyCurrentlyActive(const FKey& MappingKey) const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !MappingKey.IsValid())
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

void ACFVehiclePawn::HandleThrottleInput(const FInputActionValue& InputActionValue)
{
	const float InputValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(InputAction_Throttle, InputValue))
	{
		SetVehicleThrottleInput(0.0f);
		return;
	}
	SetVehicleThrottleInput(InputValue);
}

void ACFVehiclePawn::HandleThrottleReleased(const FInputActionValue&)
{
	SetVehicleThrottleInput(0.0f);
}

void ACFVehiclePawn::HandleSteeringInput(const FInputActionValue& InputActionValue)
{
	const float InputValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(InputAction_Steering, InputValue))
	{
		SetVehicleSteeringInput(0.0f);
		return;
	}
	SetVehicleSteeringInput(InputValue);
}

void ACFVehiclePawn::HandleSteeringReleased(const FInputActionValue&)
{
	SetVehicleSteeringInput(0.0f);
}

void ACFVehiclePawn::HandleBrakeInput(const FInputActionValue& InputActionValue)
{
	const float InputValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(InputAction_Brake, InputValue))
	{
		SetVehicleBrakeInput(0.0f);
		return;
	}
	SetVehicleBrakeInput(InputValue);
}

void ACFVehiclePawn::HandleBrakeReleased(const FInputActionValue&)
{
	SetVehicleBrakeInput(0.0f);
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
