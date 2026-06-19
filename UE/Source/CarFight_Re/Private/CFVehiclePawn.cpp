// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.62.0
// Date: 2026-06-19
// Description: CarFight 싱글플레이 차량 Pawn 구현 (멀티 동기화 진단 잔여 코드 제거)
// Changelog:
// - v2.62.0: 싱글플레이 기준선에서 차량 네트워크 진단 샘플/RepMove 수신 로그/복제 등록 경로를 제거.
// - v2.61.0: 싱글플레이 전환에 맞춰 C++ 기준선에서 Actor 복제와 Replicate Movement 강제 활성화를 중단.
// - v2.60.0: 싱글플레이 전환에 맞춰 상단 기준 설명에서 CFNetSmooth 적용 전 문구를 제거.
// - v2.59.0: CFNetSmooth Visual/Shell 적용 전 기준선을 깨끗하게 만들기 위해 차량 진단 로그와 Owner 표시 안정화 기본값을 False로 통일.
// Migration:
// - BP_CFVehiclePawn의 Actor Replicates/Replicate Movement도 False로 저장해 C++ 기본값과 맞춘다.
// - 멀티플레이 진단이 다시 필요하면 별도 멀티플레이 브랜치/문서에서 복구한다.

#include "CFVehiclePawn.h"

#include "CFVehicleData.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleDriveComp.h"
#include "CFWheelSyncComp.h"
#include "CarFightVehicleUtils.h"
#include "UI/CFAimReticleWidget.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
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

	// [v2.48.0] 이름이 일치하는 SceneComponent를 Owner에서 찾습니다.
	USceneComponent* FindSceneComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// [v2.48.0] Owner에 등록된 SceneComponent 후보 목록입니다.
		TArray<USceneComponent*> SceneComponents;
		OwnerActor->GetComponents<USceneComponent>(SceneComponents);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (SceneComponent && SceneComponent->GetFName() == ComponentName)
			{
				return SceneComponent;
			}
		}

		return nullptr;
	}

	// [v2.47.0] 이름이 일치하는 SkeletalMeshComponent를 Owner에서 찾습니다.
	USkeletalMeshComponent* FindSkeletalMeshComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// [v2.47.0] Owner에 등록된 SkeletalMeshComponent 후보 목록입니다.
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		OwnerActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
		for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
		{
			if (SkeletalMeshComponent && SkeletalMeshComponent->GetFName() == ComponentName)
			{
				return SkeletalMeshComponent;
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
	VehicleAimComp = CreateDefaultSubobject<UCFVehicleAimComp>(TEXT("VehicleAimComp"));
	OwnerVisualRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("OwnerVisualRoot"));
	if (OwnerVisualRootComp)
	{
		OwnerVisualRootComp->SetupAttachment(GetMesh());
	}

	// [v2.61.0] C++ 기본 객체 기준으로 싱글플레이 차량 기본값을 먼저 적용합니다.
	ApplyVehicleSinglePlayerBaseline();

	bAutoInitializeOnBeginPlay = true;
	bEnableWheelVisualTick = true;
	bAutoRegisterInputMappingContext = true;
	InputDeviceMode = ECFVehicleInputDeviceMode::Auto;
	InputDeviceAnalogThreshold = 0.1f;
	InputMappingPriority = 0;

	// [v2.44.0] 키보드/축 조향도 보간 경로를 타도록 기본 활성화합니다.
	bSmoothLegacySteeringInput = true;

	// [v2.44.0] LegacyAxis 목표 조향 초기값입니다.
	LegacyTargetSteeringInput = 0.0f;

	bVehicleRuntimeReady = false;
	LastVehicleRuntimeSummary = TEXT("Constructed");
	bEnableDriveStateOnScreenDebug = false;
	bEnableVehicleDebugOnScreenMessage = false;
	DriveStateDebugDisplayMode = ECFVehicleDebugDisplayMode::SingleLine;
	bShowDriveStateTransitionSummary = true;
	bShowVehicleDebugHud = true;
	bShowVehicleDebugPanel = true;
	bShowVehicleDebugText = false;
	bShowVehicleDebugEvents = false;
	DriveStateDebugMessageDuration = 0.0f;
	bShowAimReticle = true;
	AimReticleZOrder = 10;

	// [v2.48.2] Owner 표시 루트 안정화 기본 사용 여부입니다.
	bEnableOwnerVisualStabilization = false;

	// [v2.48.1] Owner 표시 루트 안정화 기본 보간 속도입니다.
	OwnerVisualStabilizationInterpSpeed = 4.0f;

	// [v2.48.1] Owner 표시 루트 안정화 기본 최대 지연각입니다.
	OwnerVisualStabilizationMaxLagDeg = 30.0f;

	// [v2.48.1] Owner 표시 루트 Yaw 안정화 기본 사용 여부입니다.
	bOwnerVisualStabilizeYaw = false;

	// [v2.48.0] Owner 표시 루트 Pitch/Roll 안정화 기본 사용 여부입니다.
	bOwnerVisualStabilizePitchRoll = false;

	// [v2.48.0] Owner 표시 안정화 중 물리 루트 렌더링 숨김 기본 사용 여부입니다.
	bHideOwnerPhysicsMeshWhenStabilized = false;

	// [v2.53.0] Owner 차체 표시 안정화 기본 사용 여부입니다.
	bEnableOwnerBodyVisualStabilization = false;

	// [v2.53.1] Owner 차체 표시 안정화 기본 보간 속도입니다.
	OwnerBodyVisualInterpSpeed = 15.0f;

	// [v2.53.1] Owner 차체 표시 안정화 기본 최대 지연각입니다.
	OwnerBodyVisualMaxLagDeg = 5.0f;

	// [v2.53.2] Owner 차체 표시 Yaw 안정화 기본 사용 여부입니다.
	bOwnerBodyVisualStabilizeYaw = false;

	// [v2.53.0] Owner 차체 표시 Pitch/Roll 안정화 기본 사용 여부입니다.
	bOwnerBodyVisualStabilizePitchRoll = false;

	// [v2.48.0] Owner 표시 안정화 준비 상태 초기값입니다.
	bOwnerVisualStabilizationReady = false;

	// [v2.48.0] Owner 표시 안정화 표시 회전 초기값입니다.
	SmoothedOwnerVisualRotation = FRotator::ZeroRotator;

	// [v2.48.0] Owner 표시 안정화 회전 기준값 유효 여부 초기값입니다.
	bHasSmoothedOwnerVisualRotation = false;

	// [v2.48.0] Owner 표시 안정화로 물리 루트 렌더링을 숨겼는지 여부 초기값입니다.
	bOwnerVisualPhysicsMeshHidden = false;

	// [v2.53.0] Owner 차체 표시 안정화 준비 상태 초기값입니다.
	bOwnerBodyVisualStabilizationReady = false;

	// [v2.53.0] Owner 차체 표시 안정화 표시 회전 초기값입니다.
	SmoothedOwnerBodyVisualRotation = FRotator::ZeroRotator;

	// [v2.53.0] Owner 차체 표시 안정화 회전 기준값 유효 여부 초기값입니다.
	bHasSmoothedOwnerBodyVisualRotation = false;

	// [v2.53.0] Owner 차체 표시 안정화 전 SM_Body 기본 상대 회전 초기값입니다.
	OriginalOwnerBodyVisualRelativeRotation = FRotator::ZeroRotator;

	// [v2.53.0] Owner 차체 표시 안정화 전 SM_Body 상대 회전 저장 여부 초기값입니다.
	bHasOriginalOwnerBodyVisualRelativeRotation = false;

	// [v2.21.0] Dedicated Server 테스트에서 C++ 기본 Pawn이 로컬 Player0을 강제 점유하지 않도록 기본값을 비활성화합니다.
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

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

	// [v2.61.0] BP 저장값이 이전 네트워크 테스트 기준으로 남아 있어도 런타임 싱글플레이 기준선을 보장합니다.
	ApplyVehicleSinglePlayerBaseline();

	// [v2.21.0] 로컬 Viewport/입력 UI 처리를 실행할 수 있는 Pawn인지 여부입니다.
	const bool bCanRunLocalPresentation = (GetNetMode() != NM_DedicatedServer) && IsLocallyControlled();
	if (bCanRunLocalPresentation && bAutoRegisterInputMappingContext)
	{
		RegisterDefaultInputMappingContext();
	}
	if (bAutoInitializeOnBeginPlay)
	{
		InitializeVehicleRuntime();
	}

	if (bCanRunLocalPresentation)
	{
		CreateAimReticleWidget();
	}
}

void ACFVehiclePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyAimReticleWidget();

	Super::EndPlay(EndPlayReason);
}

void ACFVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bVehicleRuntimeReady)
	{
		DisplayDriveStateOnScreenDebug();
		return;
	}

	// [v2.8.0] VehicleMove 기반 조향은 입력 이벤트가 없는 동안에도 중립 복귀가 필요하므로 Tick에서 계속 갱신합니다.
	if ((GetNetMode() != NM_DedicatedServer) && IsLocallyControlled())
	{
		UpdateVehicleMoveSteeringInput(DeltaSeconds);
	}

	// [v2.37.0] 정리 기준선에서 원격 Visual/Shell 분기 없이 WheelSync 시각 갱신을 실행할 수 있는지 여부입니다.
	const bool bCanUpdateVehicleWheelVisuals = (GetNetMode() != NM_DedicatedServer)
		&& bEnableWheelVisualTick;
	if (bCanUpdateVehicleWheelVisuals)
	{
		UpdateVehicleWheelVisuals(DeltaSeconds);
	}
	UpdateOwnerVisualStabilization(DeltaSeconds);
	UpdateOwnerBodyVisualStabilization(DeltaSeconds);
	DisplayDriveStateOnScreenDebug();
}

// [v2.61.0] 차량 Pawn의 싱글플레이 기본 복제 상태를 적용합니다.
void ACFVehiclePawn::ApplyVehicleSinglePlayerBaseline()
{
	// [v2.61.0] 싱글플레이 차량 Actor는 네트워크 복제 대상이 아닙니다.
	bReplicates = false;

	// [v2.61.0] 싱글플레이 차량 이동은 로컬 물리와 입력만 사용하므로 Actor Movement Replication을 끕니다.
	SetReplicateMovement(false);
}

void ACFVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		return;
	}

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
	if (InputAction_Fire)
	{
		EnhancedInputComponent->BindAction(InputAction_Fire, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleFireStarted);
	}

	// [v2.21.0] 소유 입력 컴포넌트가 준비된 뒤 로컬 Viewport UI 생성을 한 번 더 시도합니다.
	CreateAimReticleWidget();
}

bool ACFVehiclePawn::RegisterDefaultInputMappingContext()
{
	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		return false;
	}

	// [v2.21.0] Enhanced Input 매핑을 등록할 로컬 플레이어 컨트롤러입니다.
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}
	// [v2.21.0] Enhanced Input Subsystem을 소유한 로컬 플레이어입니다.
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return false;
	}
	// [v2.21.0] 실제 Input Mapping Context를 추가할 Enhanced Input Subsystem입니다.
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

	// [v2.48.0] 로컬 Owner 표시 안정화 계층 준비 결과입니다.
	const bool bOwnerVisualReady = PrepareOwnerVisualStabilization();
	const bool bDriveReady = (VehicleDriveComp != nullptr) && VehicleDriveComp->CacheVehicleMovementComponent();
	const bool bWheelSyncReady = PrepareWheelSync();

	// [v2.15.0] AimComp가 Owner Pawn과 VehicleCameraComp를 안전하게 찾았는지 여부입니다.
	const bool bAimReady = VehicleAimComp ? VehicleAimComp->InitializeAimRuntime() : false;
	bVehicleRuntimeReady = bDriveReady && bWheelSyncReady;
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: Data=%s, Drive=%s, WheelSync=%s, Aim=%s, OwnerVisual=%s, Ready=%s | %s"), VehicleData ? TEXT("Present") : TEXT("Missing"), bDriveReady ? TEXT("Ready") : TEXT("Missing"), bWheelSyncReady ? TEXT("Ready") : TEXT("Missing"), bAimReady ? TEXT("Ready") : TEXT("Missing"), bOwnerVisualReady ? TEXT("Ready") : TEXT("Skipped"), bVehicleRuntimeReady ? TEXT("True") : TEXT("False"), *DataConfigSummary);
	return bVehicleRuntimeReady;
}

bool ACFVehiclePawn::ShouldShowAimReticle() const
{
	// [v2.20.0] Dedicated Server에서는 Viewport UI를 생성하지 않기 위한 네트워크 모드 조건입니다.
	const bool bHasViewportContext = GetNetMode() != NM_DedicatedServer;

	return bShowAimReticle && bHasViewportContext && IsLocallyControlled();
}

UCFAimReticleWidget* ACFVehiclePawn::CreateAimReticleWidget()
{
	if (AimReticleWidgetInstance)
	{
		RefreshAimReticleWidget();
		return AimReticleWidgetInstance;
	}

	if (!ShouldShowAimReticle() || !AimReticleWidgetClass)
	{
		return nullptr;
	}

	// [v2.20.0] Aim Reticle 위젯을 소유할 로컬 플레이어 컨트롤러입니다.
	APlayerController* OwningPlayerController = Cast<APlayerController>(GetController());
	if (!OwningPlayerController)
	{
		return nullptr;
	}

	// [v2.20.0] Viewport에 추가할 Aim Reticle 위젯 인스턴스입니다.
	UCFAimReticleWidget* CreatedAimReticleWidget = CreateWidget<UCFAimReticleWidget>(OwningPlayerController, AimReticleWidgetClass);
	if (!CreatedAimReticleWidget)
	{
		return nullptr;
	}

	AimReticleWidgetInstance = CreatedAimReticleWidget;
	AimReticleWidgetInstance->SetVehiclePawnRef(this);
	AimReticleWidgetInstance->AddToViewport(AimReticleZOrder);
	RefreshAimReticleWidget();

	return AimReticleWidgetInstance;
}

void ACFVehiclePawn::DestroyAimReticleWidget()
{
	if (!AimReticleWidgetInstance)
	{
		return;
	}

	AimReticleWidgetInstance->RemoveFromParent();
	AimReticleWidgetInstance = nullptr;
}

void ACFVehiclePawn::RefreshAimReticleWidget()
{
	if (!AimReticleWidgetInstance)
	{
		return;
	}

	AimReticleWidgetInstance->SetVehiclePawnRef(this);
	AimReticleWidgetInstance->SetVisibility(ShouldShowAimReticle() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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

FString ACFVehiclePawn::BuildVehicleDebugTextSingleLine(const FCFVehicleDebugSnapshot& VehicleDebugSnapshot, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	// [v2.14.1] 기존 SingleLine 의미를 유지하기 위해 구분자는 ` | `를 그대로 사용합니다.
	const FString SegmentSeparator = TEXT(" | ");

	// [v2.14.1] 기존 출력 의미를 유지할 핵심 문자열 세그먼트 목록입니다.
	TArray<FString> DebugSegments;
	DebugSegments.Reserve(16);
	DebugSegments.Add(FString::Printf(TEXT("Ready=%s"), VehicleDebugSnapshot.Runtime.bRuntimeReady ? TEXT("True") : TEXT("False")));

	if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
	{
		DebugSegments.Add(FString::Printf(TEXT("State=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Drive.CurrentDriveState)));
		DebugSegments.Add(FString::Printf(TEXT("Speed=%.1f km/h"), VehicleDebugSnapshot.Drive.SpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("ForwardSpeed=%.1f km/h"), VehicleDebugSnapshot.Drive.ForwardSpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("Throttle=%.2f"), VehicleDebugSnapshot.Drive.Throttle));
		DebugSegments.Add(FString::Printf(TEXT("Brake=%.2f"), VehicleDebugSnapshot.Drive.Brake));
		DebugSegments.Add(FString::Printf(TEXT("Steering=%.2f"), VehicleDebugSnapshot.Drive.Steering));
		DebugSegments.Add(FString::Printf(TEXT("Handbrake=%s"), VehicleDebugSnapshot.Drive.bHandbrake ? TEXT("On") : TEXT("Off")));
	}
	else
	{
		DebugSegments.Add(TEXT("State=DriveCompMissing"));
	}

	if (bIncludeInputState)
	{
		DebugSegments.Add(FString::Printf(TEXT("DeviceMode=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.DeviceMode)));
		DebugSegments.Add(FString::Printf(TEXT("InputOwner=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.InputOwner)));
		DebugSegments.Add(FString::Printf(TEXT("MoveZone=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveZone)));
		DebugSegments.Add(FString::Printf(TEXT("MoveIntent=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveIntent)));
		DebugSegments.Add(FString::Printf(TEXT("MoveRaw=(%.2f, %.2f)"), VehicleDebugSnapshot.Input.MoveRaw.X, VehicleDebugSnapshot.Input.MoveRaw.Y));
		DebugSegments.Add(FString::Printf(TEXT("MoveMag=%.2f"), VehicleDebugSnapshot.Input.MoveMagnitude));
		DebugSegments.Add(FString::Printf(TEXT("MoveAngle=%.1f"), VehicleDebugSnapshot.Input.MoveAngle));
		DebugSegments.Add(FString::Printf(TEXT("BlackHold=%s"), VehicleDebugSnapshot.Input.bUsedBlackZoneHold ? TEXT("True") : TEXT("False")));
	}

	if (bIncludeTransitionSummary)
	{
		if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
		{
			DebugSegments.Add(VehicleDebugSnapshot.Drive.DriveStateTransitionSummary);
		}
		else
		{
			DebugSegments.Add(TEXT("DriveStateTransition: DriveCompMissing"));
		}
	}

	if (bIncludeRuntimeSummary)
	{
		DebugSegments.Add(VehicleDebugSnapshot.Runtime.RuntimeSummary);
	}

	return FString::Join(DebugSegments, *SegmentSeparator);
}

FString ACFVehiclePawn::BuildVehicleDebugTextMultiLine(const FCFVehicleDebugSnapshot& VehicleDebugSnapshot, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	// [v2.14.1] 기존 MultiLine 의미를 유지하기 위해 구분자만 줄바꿈으로 바꿉니다.
	const FString SegmentSeparator = TEXT("\n");

	// [v2.14.1] MultiLine도 기존 핵심 항목 순서를 유지합니다.
	TArray<FString> DebugSegments;
	DebugSegments.Reserve(16);
	DebugSegments.Add(FString::Printf(TEXT("Ready=%s"), VehicleDebugSnapshot.Runtime.bRuntimeReady ? TEXT("True") : TEXT("False")));

	if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
	{
		DebugSegments.Add(FString::Printf(TEXT("State=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Drive.CurrentDriveState)));
		DebugSegments.Add(FString::Printf(TEXT("Speed=%.1f km/h"), VehicleDebugSnapshot.Drive.SpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("ForwardSpeed=%.1f km/h"), VehicleDebugSnapshot.Drive.ForwardSpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("Throttle=%.2f"), VehicleDebugSnapshot.Drive.Throttle));
		DebugSegments.Add(FString::Printf(TEXT("Brake=%.2f"), VehicleDebugSnapshot.Drive.Brake));
		DebugSegments.Add(FString::Printf(TEXT("Steering=%.2f"), VehicleDebugSnapshot.Drive.Steering));
		DebugSegments.Add(FString::Printf(TEXT("Handbrake=%s"), VehicleDebugSnapshot.Drive.bHandbrake ? TEXT("On") : TEXT("Off")));
	}
	else
	{
		DebugSegments.Add(TEXT("State=DriveCompMissing"));
	}

	if (bIncludeInputState)
	{
		DebugSegments.Add(FString::Printf(TEXT("DeviceMode=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.DeviceMode)));
		DebugSegments.Add(FString::Printf(TEXT("InputOwner=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.InputOwner)));
		DebugSegments.Add(FString::Printf(TEXT("MoveZone=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveZone)));
		DebugSegments.Add(FString::Printf(TEXT("MoveIntent=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveIntent)));
		DebugSegments.Add(FString::Printf(TEXT("MoveRaw=(%.2f, %.2f)"), VehicleDebugSnapshot.Input.MoveRaw.X, VehicleDebugSnapshot.Input.MoveRaw.Y));
		DebugSegments.Add(FString::Printf(TEXT("MoveMag=%.2f"), VehicleDebugSnapshot.Input.MoveMagnitude));
		DebugSegments.Add(FString::Printf(TEXT("MoveAngle=%.1f"), VehicleDebugSnapshot.Input.MoveAngle));
		DebugSegments.Add(FString::Printf(TEXT("BlackHold=%s"), VehicleDebugSnapshot.Input.bUsedBlackZoneHold ? TEXT("True") : TEXT("False")));
	}

	if (bIncludeTransitionSummary)
	{
		if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
		{
			DebugSegments.Add(VehicleDebugSnapshot.Drive.DriveStateTransitionSummary);
		}
		else
		{
			DebugSegments.Add(TEXT("DriveStateTransition: DriveCompMissing"));
		}
	}

	if (bIncludeRuntimeSummary)
	{
		DebugSegments.Add(VehicleDebugSnapshot.Runtime.RuntimeSummary);
	}

	return FString::Join(DebugSegments, *SegmentSeparator);
}

FString ACFVehiclePawn::BuildVehicleDebugSummary(bool bUseMultilineFormat, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	// [v2.14.1] 텍스트 출력도 동일한 Snapshot 원본을 공유하도록 먼저 현재 스냅샷을 확보합니다.
	const FCFVehicleDebugSnapshot VehicleDebugSnapshot = GetVehicleDebugSnapshot();

	return bUseMultilineFormat
		? BuildVehicleDebugTextMultiLine(VehicleDebugSnapshot, bIncludeRuntimeSummary, bIncludeTransitionSummary, bIncludeInputState)
		: BuildVehicleDebugTextSingleLine(VehicleDebugSnapshot, bIncludeRuntimeSummary, bIncludeTransitionSummary, bIncludeInputState);
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
		if (CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D)
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
	ResolvedMoveInput.SteeringValue = CalculateVehicleMoveTargetSteering(MoveInputVector, ResolvedMoveInput.Magnitude);

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
	// [v2.8.0] VehicleMove 조향은 목표값만 갱신하고, 실제 적용값은 Tick의 UpdateVehicleMoveSteeringInput에서 제한 속도로 추적합니다.
	TargetSteeringInput = ResolvedMoveInput.SteeringValue;
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

	// [v2.56.1] 실제 Chaos Vehicle Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = ResolveVehicleMovementComponent(TEXT("VehicleRuntime: DriveComp cache failed during ApplyVehicleWheelPhysicsConfig."), TEXT("VehicleRuntime: VehicleMovementComponent is null during ApplyVehicleWheelPhysicsConfig."));
	if (!ResolvedVehicleMovementComponent)
	{
		return;
	}

	// [v2.56.1] VehicleData에서 읽은 차량 물리 설정입니다.
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;

	// [v2.56.1] VehicleData에서 읽은 차량 참조 설정입니다.
	const FCFVehicleReferenceConfig& VehicleReferenceConfig = VehicleData->VehicleReferenceConfig;

	// [v2.56.1] 런타임 휠 물리 덮어쓰기를 사용할지 여부입니다.
	const bool bUseRuntimeWheelPhysicsOverrides = VehicleMovementConfig.bUseMovementOverrides;

	// [v2.56.1] 휠 setup에 클래스/오프셋을 적용합니다.
	const auto ConfigureWheelSetup = [&](FChaosWheelSetup& WheelSetup, const TSubclassOf<UChaosVehicleWheel> WheelClass, const bool bIsFrontWheel)
	{
		WheelSetup.WheelClass = WheelClass;
		WheelSetup.AdditionalOffset = bIsFrontWheel ? VehicleMovementConfig.FrontWheelAdditionalOffset : VehicleMovementConfig.RearWheelAdditionalOffset;
		if (!WheelClass)
		{
			return;
		}

		// [v2.56.1] 휠 클래스의 기본 오브젝트입니다.
		UChaosVehicleWheel* WheelClassDefaultObject = WheelClass->GetDefaultObject<UChaosVehicleWheel>();
		if (!WheelClassDefaultObject)
		{
			return;
		}

		if (!bUseRuntimeWheelPhysicsOverrides)
		{
			return;
		}

		// [v2.56.1] 클래스 기본값 임시 변경 전 복구용 스냅샷입니다.
		const FCFWheelClassRuntimeSnapshot WheelClassRuntimeSnapshot = CaptureWheelClassRuntimeSnapshot(*WheelClassDefaultObject);
		ApplyVehicleMovementWheelTuningToWheelClass(*WheelClassDefaultObject, VehicleMovementConfig, bIsFrontWheel);
		WheelSetup.WheelClass = WheelClass;
		RestoreWheelClassRuntimeSnapshot(*WheelClassDefaultObject, WheelClassRuntimeSnapshot);
	};

	for (int32 WheelIndex = 0; WheelIndex < ResolvedVehicleMovementComponent->WheelSetups.Num(); ++WheelIndex)
	{
		// [v2.56.1] 현재 순회 중인 Chaos 휠 setup입니다.
		FChaosWheelSetup& WheelSetup = ResolvedVehicleMovementComponent->WheelSetups[WheelIndex];

		// [v2.56.1] 휠 본 이름 문자열입니다.
		const FString BoneNameString = WheelSetup.BoneName.ToString();

		// [v2.56.1] 현재 휠을 앞바퀴로 볼지 여부입니다.
		const bool bIsFrontWheel = BoneNameString.Contains(TEXT("F"));

		// [v2.56.1] 현재 휠에 적용할 휠 클래스입니다.
		const TSubclassOf<UChaosVehicleWheel> WheelClass = bIsFrontWheel
			? VehicleReferenceConfig.FrontWheelClass
			: VehicleReferenceConfig.RearWheelClass;

		ConfigureWheelSetup(WheelSetup, WheelClass, bIsFrontWheel);
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
		// [v2.55.0] 현재 속도 기준 조향 제한을 적용한 실제 Chaos Vehicle 입력값입니다.
		const float SpeedLimitedSteeringValue = CalculateSpeedLimitedSteeringInput(InSteeringValue);

		VehicleDriveComp->ApplySteeringInput(SpeedLimitedSteeringValue);
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
	// [v2.14.1] VehicleDebug v2 Phase 1: 기존 필드를 유지하면서 카테고리형 Snapshot을 함께 채웁니다.
	FCFVehicleDebugSnapshot DebugSnapshot;

	// [v2.14.1] 현재 Drive 컴포넌트 존재 여부를 먼저 고정합니다.
	const bool bHasDriveComponent = (VehicleDriveComp != nullptr);

	// [v2.14.1] 현재 WheelSync 컴포넌트 존재 여부를 먼저 고정합니다.
	const bool bHasWheelSyncComponent = (WheelSyncComp != nullptr);

	// [v2.14.1] Drive 카테고리 채우기에 사용할 최신 Drive 상태 스냅샷입니다.
	FCFVehicleDriveStateSnapshot CurrentDriveStateSnapshot;

	DebugSnapshot.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.RuntimeSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.bHasDriveComponent = bHasDriveComponent;
	DebugSnapshot.bHasWheelSyncComponent = bHasWheelSyncComponent;

	DebugSnapshot.Runtime.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.Runtime.bHasDriveComponent = bHasDriveComponent;
	DebugSnapshot.Runtime.bHasWheelSyncComponent = bHasWheelSyncComponent;
	DebugSnapshot.Runtime.RuntimeSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.Runtime.LastInitAttemptSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.Runtime.LastValidationSummary = LastVehicleRuntimeSummary;

		DebugSnapshot.Input.DeviceMode = InputDeviceMode;
	DebugSnapshot.Input.InputOwner = CurrentInputOwnership;
	DebugSnapshot.Input.MoveZone = LastVehicleMoveInputResult.ResolvedZone;
	DebugSnapshot.Input.MoveIntent = LastMoveDirectionIntent;
	DebugSnapshot.Input.MoveRaw = LastVehicleMoveInputResult.RawMoveInput;
	DebugSnapshot.Input.MoveMagnitude = LastVehicleMoveInputResult.Magnitude;
	DebugSnapshot.Input.MoveAngle = LastVehicleMoveInputResult.AngleDeg;
	DebugSnapshot.Input.bUsedBlackZoneHold = LastVehicleMoveInputResult.bUsedBlackZoneHold;
	DebugSnapshot.Input.TargetSteeringInput = TargetSteeringInput;
	DebugSnapshot.Input.CurrentSteeringInput = CurrentSteeringInput;
	DebugSnapshot.Input.LastSteeringTurnRate = LastSteeringTurnRate;
	DebugSnapshot.Input.LastSteeringReturnRate = LastSteeringReturnRate;
	DebugSnapshot.Input.bSteeringReturningToCenter = bSteeringReturningToCenter;


	// [v2.7.0] Camera 카테고리는 VehicleCameraComp가 제공하는 원본 런타임 스냅샷과 표시용 압축 비율을 함께 담습니다.
	DebugSnapshot.Camera.bHasVehicleCameraComponent = (VehicleCameraComp != nullptr);
	if (DebugSnapshot.Camera.bHasVehicleCameraComponent)
	{
		DebugSnapshot.Camera.CameraRuntimeState = VehicleCameraComp->GetCameraRuntimeState();

		const float DesiredArmLength = DebugSnapshot.Camera.CameraRuntimeState.DesiredArmLength;
		const float SolvedArmLength = DebugSnapshot.Camera.CameraRuntimeState.SolvedArmLength;

		if (DesiredArmLength > KINDA_SMALL_NUMBER)
		{
			DebugSnapshot.Camera.CollisionCompressionRatio = FMath::Clamp(SolvedArmLength / DesiredArmLength, 0.0f, 1.0f);
		}
		else
		{
			DebugSnapshot.Camera.CollisionCompressionRatio = 1.0f;
		}

		DebugSnapshot.Camera.bCameraCompressedByCollision = DebugSnapshot.Camera.CollisionCompressionRatio < 0.90f;
	}

	// [v2.16.0] Aim 카테고리는 VehicleAimComp가 제공하는 현재 상태를 표시용으로만 읽습니다.
	DebugSnapshot.Aim.bHasVehicleAimComponent = (VehicleAimComp != nullptr);
	if (DebugSnapshot.Aim.bHasVehicleAimComponent)
	{
		DebugSnapshot.Aim.bAimRuntimeReady = VehicleAimComp->IsAimRuntimeReady();
		DebugSnapshot.Aim.LocalAimState = VehicleAimComp->GetLocalAimState();
		DebugSnapshot.Aim.ServerAimState = VehicleAimComp->GetServerAimState();
		DebugSnapshot.Aim.RepAimVisualState = VehicleAimComp->GetRepAimVisualState();
		DebugSnapshot.Aim.ReticleState = VehicleAimComp->GetReticleState();
		DebugSnapshot.Aim.AimRuntimeSummary = VehicleAimComp->GetLastAimRuntimeSummary();
		DebugSnapshot.Aim.LastFireRequest = LastFireRequest;
		DebugSnapshot.Aim.LastFireResult = LastFireResult;
	}

	DebugSnapshot.Overview.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.Overview.DeviceMode = InputDeviceMode;
	DebugSnapshot.Overview.InputOwner = CurrentInputOwnership;

	if (bHasDriveComponent)
	{
		CurrentDriveStateSnapshot = VehicleDriveComp->GetDriveStateSnapshot();
		DebugSnapshot.CurrentDriveState = VehicleDriveComp->GetDriveState();
		DebugSnapshot.PreviousDriveState = VehicleDriveComp->GetPreviousDriveState();
		DebugSnapshot.bDriveStateChangedThisFrame = VehicleDriveComp->HasDriveStateChangedThisFrame();
		DebugSnapshot.DriveStateTransitionSummary = VehicleDriveComp->GetLastDriveStateTransitionSummary();
		DebugSnapshot.DriveStateSnapshot = CurrentDriveStateSnapshot;

		DebugSnapshot.Drive.CurrentDriveState = DebugSnapshot.CurrentDriveState;
		DebugSnapshot.Drive.PreviousDriveState = DebugSnapshot.PreviousDriveState;
		DebugSnapshot.Drive.bDriveStateChangedThisFrame = DebugSnapshot.bDriveStateChangedThisFrame;
		DebugSnapshot.Drive.SpeedKmh = CurrentDriveStateSnapshot.CurrentSpeedKmh;
		DebugSnapshot.Drive.ForwardSpeedKmh = CurrentDriveStateSnapshot.ForwardSpeedKmh;
		DebugSnapshot.Drive.Throttle = CurrentDriveStateSnapshot.CurrentInputState.ThrottleInput;
		DebugSnapshot.Drive.Brake = CurrentDriveStateSnapshot.CurrentInputState.BrakeInput;
		DebugSnapshot.Drive.Steering = CurrentDriveStateSnapshot.CurrentInputState.SteeringInput;
		DebugSnapshot.Drive.bHandbrake = CurrentDriveStateSnapshot.CurrentInputState.bHandbrakePressed;
		DebugSnapshot.Drive.DriveStateTransitionSummary = DebugSnapshot.DriveStateTransitionSummary;
		DebugSnapshot.Drive.DriveStateSnapshot = CurrentDriveStateSnapshot;

		DebugSnapshot.Overview.CurrentDriveState = DebugSnapshot.CurrentDriveState;
		DebugSnapshot.Overview.SpeedKmh = CurrentDriveStateSnapshot.CurrentSpeedKmh;
		DebugSnapshot.Overview.ForwardSpeedKmh = CurrentDriveStateSnapshot.ForwardSpeedKmh;
		DebugSnapshot.Overview.LastTransitionShortText = DebugSnapshot.DriveStateTransitionSummary;
	}

	return DebugSnapshot;
}

FCFVehicleDebugOverview ACFVehiclePawn::GetVehicleDebugOverview() const
{
	// [v2.14.2] HUD 위젯이 필요한 카테고리만 직접 읽을 수 있도록 Overview를 반환합니다.
	return GetVehicleDebugSnapshot().Overview;
}

FCFVehicleDebugDrive ACFVehiclePawn::GetVehicleDebugDrive() const
{
	// [v2.14.2] 상세 패널이 필요한 Drive 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Drive;
}

FCFVehicleDebugInput ACFVehiclePawn::GetVehicleDebugInput() const
{
	// [v2.14.2] 상세 패널이 필요한 Input 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Input;
}

FCFVehicleDebugCamera ACFVehiclePawn::GetVehicleDebugCamera() const
{
	// [v2.7.0] 상세 패널이 필요한 Camera 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Camera;
}

FCFVehicleDebugAim ACFVehiclePawn::GetVehicleDebugAim() const
{
	// [v2.16.0] 상세 패널이 필요한 Aim 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Aim;
}

FCFVehicleDebugRuntime ACFVehiclePawn::GetVehicleDebugRuntime() const
{
	// [v2.14.2] 상세 패널이 필요한 Runtime 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Runtime;
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

bool ACFVehiclePawn::ShouldShowVehicleDebugUi() const
{
	// [v2.21.0] VehicleDebug HUD/Panel은 Viewport가 있는 로컬 제어 Pawn에서만 표시합니다.
	return bEnableDriveStateOnScreenDebug && (GetNetMode() != NM_DedicatedServer) && IsLocallyControlled();
}

bool ACFVehiclePawn::ShouldShowDebugWidget() const
{
	// [v2.14.3] 레거시 WBP_VehicleDebug 제거 전환을 위해 기존 Text Widget 표시는 항상 비활성화합니다.
	return false;
}

bool ACFVehiclePawn::ShouldShowVehicleDebugHud() const
{
	// [v2.14.3] HUD는 레거시 Text Widget과 분리된 공통 UI 표시 조건과 HUD 전용 토글을 함께 만족할 때만 표시합니다.
	return ShouldShowVehicleDebugUi() && bShowVehicleDebugHud;
}

bool ACFVehiclePawn::ShouldShowVehicleDebugPanel() const
{
	// [v2.14.3] 상세 패널은 레거시 Text Widget과 분리된 공통 UI 표시 조건과 Panel 전용 토글을 함께 만족할 때만 표시합니다.
	return ShouldShowVehicleDebugUi() && bShowVehicleDebugPanel;
}

bool ACFVehiclePawn::ShouldShowVehicleDebugText() const
{
	// [v2.14.3] 레거시 WBP_VehicleDebug 제거 전환을 위해 Legacy Text View 표시는 항상 비활성화합니다.
	return false;
}

ESlateVisibility ACFVehiclePawn::GetDebugWidgetVisibility() const
{
	// [v2.14.3] 레거시 WBP_VehicleDebug 제거 전환을 위해 Visibility는 항상 Collapsed를 반환합니다.
	return ESlateVisibility::Collapsed;
}

// [v2.48.0] 로컬 Owner 표시 안정화용 차체/휠 표시 계층을 준비합니다.
bool ACFVehiclePawn::PrepareOwnerVisualStabilization()
{
	bOwnerVisualStabilizationReady = false;
	OwnerVisualStabilizedComponents.Reset();

	if (!bEnableOwnerVisualStabilization)
	{
		ResetOwnerVisualStabilization();
		return false;
	}

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		ResetOwnerVisualStabilization();
		return false;
	}

	if (!OwnerVisualRootComp)
	{
		return false;
	}

	// [v2.48.0] Owner 표시 루트의 부모가 될 차량 물리 루트 SkeletalMeshComponent입니다.
	USkeletalMeshComponent* VehicleMeshComponent = FindSkeletalMeshComponentByName(this, TEXT("VehicleMesh"));
	if (!VehicleMeshComponent)
	{
		VehicleMeshComponent = GetMesh();
	}

	if (!VehicleMeshComponent)
	{
		return false;
	}

	if (OwnerVisualRootComp->GetAttachParent() != VehicleMeshComponent)
	{
		OwnerVisualRootComp->AttachToComponent(VehicleMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	OwnerVisualRootComp->SetRelativeLocation(FVector::ZeroVector);
	OwnerVisualRootComp->SetRelativeRotation(FRotator::ZeroRotator);
	OwnerVisualRootComp->SetRelativeScale3D(FVector::OneVector);

	// [v2.48.0] Owner 표시 루트 아래로 묶을 차체/휠 표시 컴포넌트 이름 목록입니다.
	const TArray<FName> OwnerVisualComponentNames =
	{
		TEXT("SM_Body"),
		TEXT("Wheel_Anchor_FL"),
		TEXT("Wheel_Anchor_FR"),
		TEXT("Wheel_Anchor_RL"),
		TEXT("Wheel_Anchor_RR"),
		TEXT("Wheel_Mesh_FL"),
		TEXT("Wheel_Mesh_FR"),
		TEXT("Wheel_Mesh_RL"),
		TEXT("Wheel_Mesh_RR")
	};

	// [v2.48.0] Owner 표시 루트 아래로 이동한 컴포넌트 개수입니다.
	int32 AttachedVisualComponentCount = 0;
	for (const FName& OwnerVisualComponentName : OwnerVisualComponentNames)
	{
		// [v2.48.0] 이름으로 찾은 표시 대상 SceneComponent입니다.
		USceneComponent* VisualComponent = FindSceneComponentByName(this, OwnerVisualComponentName);
		if (AttachOwnerVisualComponent(VisualComponent))
		{
			++AttachedVisualComponentCount;
		}
	}

	if (VehicleMeshComponent && bHideOwnerPhysicsMeshWhenStabilized)
	{
		VehicleMeshComponent->SetVisibility(false, false);
		VehicleMeshComponent->SetHiddenInGame(true, false);
		bOwnerVisualPhysicsMeshHidden = true;
	}
	else if (VehicleMeshComponent && bOwnerVisualPhysicsMeshHidden)
	{
		VehicleMeshComponent->SetVisibility(true, false);
		VehicleMeshComponent->SetHiddenInGame(false, false);
		bOwnerVisualPhysicsMeshHidden = false;
	}

	SmoothedOwnerVisualRotation = GetActorRotation();
	bHasSmoothedOwnerVisualRotation = true;
	bOwnerVisualStabilizationReady = AttachedVisualComponentCount > 0;

	return bOwnerVisualStabilizationReady;
}

// [v2.48.0] 지정한 표시 컴포넌트를 Owner 표시 루트 아래로 안전하게 이동합니다.
bool ACFVehiclePawn::AttachOwnerVisualComponent(USceneComponent* VisualComponent)
{
	if (!VisualComponent || !OwnerVisualRootComp)
	{
		return false;
	}

	if ((VisualComponent == OwnerVisualRootComp) || (VisualComponent == GetRootComponent()) || (VisualComponent == GetMesh()))
	{
		return false;
	}

	if (!VisualComponent->IsRegistered())
	{
		return false;
	}

	if (!VisualComponent->IsAttachedTo(OwnerVisualRootComp))
	{
		VisualComponent->AttachToComponent(OwnerVisualRootComp, FAttachmentTransformRules::KeepWorldTransform);
	}

	OwnerVisualStabilizedComponents.AddUnique(VisualComponent);
	return true;
}

// [v2.48.0] 로컬 Owner 표시 루트 회전을 현재 Actor 회전에 부드럽게 맞춥니다.
void ACFVehiclePawn::UpdateOwnerVisualStabilization(const float DeltaSeconds)
{
	if (!bEnableOwnerVisualStabilization)
	{
		ResetOwnerVisualStabilization();
		return;
	}

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		ResetOwnerVisualStabilization();
		return;
	}

	if (!OwnerVisualRootComp)
	{
		return;
	}

	if (!bOwnerVisualStabilizationReady)
	{
		// [v2.48.0] 이번 Tick에서 Owner 표시 안정화 계층 준비에 성공했는지 여부입니다.
		const bool bPreparedOwnerVisualThisFrame = PrepareOwnerVisualStabilization();
		if (bPreparedOwnerVisualThisFrame && WheelSyncComp && bVehicleRuntimeReady)
		{
			PrepareWheelSync();
		}
	}

	if (!bOwnerVisualStabilizationReady)
	{
		return;
	}

	// [v2.48.0] 현재 물리 Actor 회전입니다.
	const FRotator CurrentActorRotation = GetActorRotation();

	if (!bHasSmoothedOwnerVisualRotation)
	{
		SmoothedOwnerVisualRotation = CurrentActorRotation;
		bHasSmoothedOwnerVisualRotation = true;
	}

	// [v2.48.0] 음수 Tick 간격을 방지한 표시 안정화 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v2.48.0] 표시 루트 회전 보간에 사용할 안전한 보간 속도입니다.
	const float SafeInterpSpeed = FMath::Max(OwnerVisualStabilizationInterpSpeed, 0.1f);

	// [v2.48.0] 현재 보간 속도로 Actor 회전을 따라간 후보 표시 회전입니다.
	const FRotator InterpolatedVisualRotation = FMath::RInterpTo(SmoothedOwnerVisualRotation, CurrentActorRotation, SafeDeltaSeconds, SafeInterpSpeed);

	// [v2.48.0] 축별 사용 여부를 반영한 표시 회전 후보입니다.
	FRotator DesiredVisualRotation = CurrentActorRotation;
	if (bOwnerVisualStabilizePitchRoll)
	{
		DesiredVisualRotation.Pitch = InterpolatedVisualRotation.Pitch;
		DesiredVisualRotation.Roll = InterpolatedVisualRotation.Roll;
	}
	if (bOwnerVisualStabilizeYaw)
	{
		DesiredVisualRotation.Yaw = InterpolatedVisualRotation.Yaw;
	}

	SmoothedOwnerVisualRotation = ClampOwnerVisualStabilizedRotation(CurrentActorRotation, DesiredVisualRotation);
	OwnerVisualRootComp->SetWorldRotation(SmoothedOwnerVisualRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

// [v2.48.0] Owner 표시 안정화 상태를 기본 회전으로 되돌립니다.
void ACFVehiclePawn::ResetOwnerVisualStabilization()
{
	if (OwnerVisualRootComp)
	{
		OwnerVisualRootComp->SetRelativeRotation(FRotator::ZeroRotator);
	}

	SmoothedOwnerVisualRotation = GetActorRotation();
	bHasSmoothedOwnerVisualRotation = false;
	bOwnerVisualStabilizationReady = false;
	OwnerVisualStabilizedComponents.Reset();

	// [v2.48.0] 물리 루트 렌더링 복구 대상 SkeletalMeshComponent입니다.
	USkeletalMeshComponent* VehicleMeshComponent = FindSkeletalMeshComponentByName(this, TEXT("VehicleMesh"));
	if (!VehicleMeshComponent)
	{
		VehicleMeshComponent = GetMesh();
	}

	if (VehicleMeshComponent && bOwnerVisualPhysicsMeshHidden)
	{
		VehicleMeshComponent->SetVisibility(true, false);
		VehicleMeshComponent->SetHiddenInGame(false, false);
		bOwnerVisualPhysicsMeshHidden = false;
	}
}

// [v2.48.0] Actor 회전과 표시 회전 사이의 지연각을 설정 한도 안으로 제한합니다.
FRotator ACFVehiclePawn::ClampOwnerVisualStabilizedRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	// [v2.48.0] 표시 루트가 Actor 회전에서 벗어날 수 있는 최대 축별 각도입니다.
	const float SafeMaxLagDeg = FMath::Max(OwnerVisualStabilizationMaxLagDeg, 0.0f);

	// [v2.48.0] Actor Pitch에서 표시 Pitch까지의 지연각입니다.
	const float PitchLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Pitch, DesiredVisualRotation.Pitch);

	// [v2.48.0] Actor Yaw에서 표시 Yaw까지의 지연각입니다.
	const float YawLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Yaw, DesiredVisualRotation.Yaw);

	// [v2.48.0] Actor Roll에서 표시 Roll까지의 지연각입니다.
	const float RollLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Roll, DesiredVisualRotation.Roll);

	// [v2.48.0] 최대 지연각으로 제한한 표시 Pitch입니다.
	const float ClampedPitchDeg = CurrentActorRotation.Pitch + FMath::Clamp(PitchLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.48.0] 최대 지연각으로 제한한 표시 Yaw입니다.
	const float ClampedYawDeg = CurrentActorRotation.Yaw + FMath::Clamp(YawLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.48.0] 최대 지연각으로 제한한 표시 Roll입니다.
	const float ClampedRollDeg = CurrentActorRotation.Roll + FMath::Clamp(RollLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.48.0] 정규화 전 최종 표시 회전입니다.
	FRotator ClampedVisualRotation(ClampedPitchDeg, ClampedYawDeg, ClampedRollDeg);
	ClampedVisualRotation.Normalize();

	return ClampedVisualRotation;
}

// [v2.53.0] 로컬 조작 차량의 SM_Body 표시 회전을 부드럽게 안정화합니다.
void ACFVehiclePawn::UpdateOwnerBodyVisualStabilization(const float DeltaSeconds)
{
	if (!bEnableOwnerBodyVisualStabilization || bEnableOwnerVisualStabilization)
	{
		ResetOwnerBodyVisualStabilization();
		return;
	}

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		ResetOwnerBodyVisualStabilization();
		return;
	}

	// [v2.53.0] 안정화 대상 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (!BodyMeshComponent || !BodyMeshComponent->IsRegistered() || BodyMeshComponent->IsSimulatingPhysics())
	{
		bOwnerBodyVisualStabilizationReady = false;
		return;
	}

	if (!bHasOriginalOwnerBodyVisualRelativeRotation)
	{
		// [v2.53.0] 안정화 전 차체 표시 기본 상대 회전입니다.
		const FRotator CurrentBodyRelativeRotation = BodyMeshComponent->GetRelativeRotation();

		OriginalOwnerBodyVisualRelativeRotation = CurrentBodyRelativeRotation;
		bHasOriginalOwnerBodyVisualRelativeRotation = true;
	}

	// [v2.53.0] 현재 물리 Actor 회전입니다.
	const FRotator CurrentActorRotation = GetActorRotation();

	if (!bHasSmoothedOwnerBodyVisualRotation)
	{
		SmoothedOwnerBodyVisualRotation = CurrentActorRotation;
		bHasSmoothedOwnerBodyVisualRotation = true;
	}

	// [v2.53.0] 음수 Tick 간격을 방지한 차체 표시 안정화 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v2.53.0] 차체 표시 회전 보간에 사용할 안전한 보간 속도입니다.
	const float SafeInterpSpeed = FMath::Max(OwnerBodyVisualInterpSpeed, 0.1f);

	// [v2.53.0] 현재 보간 속도로 Actor 회전을 따라간 후보 차체 표시 회전입니다.
	const FRotator InterpolatedBodyRotation = FMath::RInterpTo(SmoothedOwnerBodyVisualRotation, CurrentActorRotation, SafeDeltaSeconds, SafeInterpSpeed);

	// [v2.53.0] 축별 사용 여부를 반영한 차체 표시 회전 후보입니다.
	FRotator DesiredBodyRotation = CurrentActorRotation;
	if (bOwnerBodyVisualStabilizePitchRoll)
	{
		DesiredBodyRotation.Pitch = InterpolatedBodyRotation.Pitch;
		DesiredBodyRotation.Roll = InterpolatedBodyRotation.Roll;
	}
	if (bOwnerBodyVisualStabilizeYaw)
	{
		DesiredBodyRotation.Yaw = InterpolatedBodyRotation.Yaw;
	}

	SmoothedOwnerBodyVisualRotation = ClampOwnerBodyVisualRotation(CurrentActorRotation, DesiredBodyRotation);
	BodyMeshComponent->SetWorldRotation(SmoothedOwnerBodyVisualRotation, false, nullptr, ETeleportType::TeleportPhysics);
	bOwnerBodyVisualStabilizationReady = true;
}

// [v2.53.0] 로컬 조작 차량의 SM_Body 표시 안정화 상태를 기본 상태로 되돌립니다.
void ACFVehiclePawn::ResetOwnerBodyVisualStabilization()
{
	// [v2.53.0] 리셋 대상 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (BodyMeshComponent && BodyMeshComponent->IsRegistered() && bHasOriginalOwnerBodyVisualRelativeRotation)
	{
		BodyMeshComponent->SetRelativeRotation(OriginalOwnerBodyVisualRelativeRotation);
	}

	SmoothedOwnerBodyVisualRotation = GetActorRotation();
	bHasSmoothedOwnerBodyVisualRotation = false;
	bOwnerBodyVisualStabilizationReady = false;
}

// [v2.53.0] Actor 회전과 SM_Body 표시 회전 사이의 지연각을 설정 한도 안으로 제한합니다.
FRotator ACFVehiclePawn::ClampOwnerBodyVisualRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	// [v2.53.0] 차체 표시가 Actor 회전에서 벗어날 수 있는 최대 축별 각도입니다.
	const float SafeMaxLagDeg = FMath::Max(OwnerBodyVisualMaxLagDeg, 0.0f);

	// [v2.53.0] Actor Pitch에서 차체 표시 Pitch까지의 지연각입니다.
	const float PitchLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Pitch, DesiredVisualRotation.Pitch);

	// [v2.53.0] Actor Yaw에서 차체 표시 Yaw까지의 지연각입니다.
	const float YawLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Yaw, DesiredVisualRotation.Yaw);

	// [v2.53.0] Actor Roll에서 차체 표시 Roll까지의 지연각입니다.
	const float RollLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Roll, DesiredVisualRotation.Roll);

	// [v2.53.0] 최대 지연각으로 제한한 차체 표시 Pitch입니다.
	const float ClampedPitchDeg = CurrentActorRotation.Pitch + FMath::Clamp(PitchLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.53.0] 최대 지연각으로 제한한 차체 표시 Yaw입니다.
	const float ClampedYawDeg = CurrentActorRotation.Yaw + FMath::Clamp(YawLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.53.0] 최대 지연각으로 제한한 차체 표시 Roll입니다.
	const float ClampedRollDeg = CurrentActorRotation.Roll + FMath::Clamp(RollLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.53.0] 정규화 전 최종 차체 표시 회전입니다.
	FRotator ClampedBodyRotation(ClampedPitchDeg, ClampedYawDeg, ClampedRollDeg);
	ClampedBodyRotation.Normalize();

	return ClampedBodyRotation;
}

void ACFVehiclePawn::DisplayDriveStateOnScreenDebug() const
{
	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled() || !bEnableVehicleDebugOnScreenMessage || !GEngine)
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

float ACFVehiclePawn::CalculateVehicleMoveTargetSteering(const FVector2D& MoveInputVector, const float MoveInputMagnitude) const
{
	if (MoveInputMagnitude < SteeringDirectionMinMagnitude)
	{
		return 0.0f;
	}
	const FVector2D SteeringDirection = MoveInputVector / MoveInputMagnitude;
	return FMath::Clamp(SteeringDirection.X, -1.0f, 1.0f);
}

float ACFVehiclePawn::CalculateSteeringReturnRateKmh(const float SpeedKmh) const
{
	const float AbsoluteSpeedKmh = FMath::Abs(SpeedKmh);
	if (AbsoluteSpeedKmh <= SteeringReturnMinSpeedKmh)
	{
		return 0.0f;
	}

	const float ReturnSpeedRangeKmh = FMath::Max(SteeringReturnMaxSpeedKmh - SteeringReturnMinSpeedKmh, 1.0f);
	const float SpeedAlpha = FMath::Clamp((AbsoluteSpeedKmh - SteeringReturnMinSpeedKmh) / ReturnSpeedRangeKmh, 0.0f, 1.0f);
	return FMath::Lerp(SteeringReturnMinRate, SteeringReturnMaxRate, SpeedAlpha);
}

// [v2.55.0] 속도에 따라 실제 Chaos Vehicle로 전달할 조향 입력을 제한합니다.
float ACFVehiclePawn::CalculateSpeedLimitedSteeringInput(const float RawSteeringInput) const
{
	// [v2.55.0] 입력 경로에서 들어온 원본 조향값을 Chaos 입력 허용 범위로 고정한 값입니다.
	const float ClampedRawSteeringInput = FMath::Clamp(RawSteeringInput, -1.0f, 1.0f);

	if (!bEnableSpeedSteeringLimit || FMath::Abs(ClampedRawSteeringInput) <= KINDA_SMALL_NUMBER)
	{
		return ClampedRawSteeringInput;
	}

	// [v2.55.0] 현재 차량 속도를 얻기 위한 Drive 상태 스냅샷입니다.
	const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();

	// [v2.55.0] 전진과 후진 모두 같은 제한을 적용하기 위한 절대 속도(km/h)입니다.
	const float AbsoluteSpeedKmh = FMath::Abs(DriveStateSnapshot.CurrentSpeedKmh);

	// [v2.55.0] 제한 시작 속도를 음수가 되지 않도록 보정한 값입니다.
	const float LimitStartSpeedKmh = FMath::Max(SpeedSteeringLimitStartSpeedKmh, 0.0f);

	// [v2.55.0] 제한 최대 속도가 시작 속도보다 낮게 설정되지 않도록 보정한 값입니다.
	const float LimitFullSpeedKmh = FMath::Max(SpeedSteeringLimitFullSpeedKmh, LimitStartSpeedKmh + 1.0f);

	if (AbsoluteSpeedKmh <= LimitStartSpeedKmh)
	{
		return ClampedRawSteeringInput;
	}

	// [v2.55.0] 현재 속도가 제한 시작과 최대 제한 사이에서 어느 정도 진행됐는지 나타내는 비율입니다.
	const float SpeedLimitAlpha = FMath::Clamp((AbsoluteSpeedKmh - LimitStartSpeedKmh) / (LimitFullSpeedKmh - LimitStartSpeedKmh), 0.0f, 1.0f);

	// [v2.55.0] 고속 구간에서 허용할 최소 조향 배율입니다.
	const float MinimumSteeringScale = FMath::Clamp(SpeedSteeringLimitMinScale, 0.05f, 1.0f);

	// [v2.55.0] 현재 속도 기준으로 원본 조향 입력에 곱할 최종 배율입니다.
	const float SteeringScale = FMath::Lerp(1.0f, MinimumSteeringScale, SpeedLimitAlpha);

	return FMath::Clamp(ClampedRawSteeringInput * SteeringScale, -1.0f, 1.0f);
}

// [v2.44.0] 현재 입력 경로의 목표 조향값을 제한 속도로 추적해 DriveComp에 적용합니다.
void ACFVehiclePawn::UpdateVehicleMoveSteeringInput(const float DeltaSeconds)
{
	// [v2.44.0] 현재 LegacyAxis 조향을 보간 경로로 처리해야 하는지 여부입니다.
	const bool bUseSmoothedLegacySteering = bSmoothLegacySteeringInput && CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis;
	if (CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis && !bUseSmoothedLegacySteering)
	{
		CurrentSteeringInput = 0.0f;
		TargetSteeringInput = 0.0f;
		LegacyTargetSteeringInput = 0.0f;
		LastSteeringTurnRate = 0.0f;
		LastSteeringReturnRate = 0.0f;
		bSteeringReturningToCenter = false;
		return;
	}

	// [v2.44.0] 음수 Tick 간격이 들어오지 않도록 보정한 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v2.44.0] 현재 입력 경로에서 실제 조향 보간이 따라갈 목표값입니다.
	const float ActiveTargetSteeringInput = bUseSmoothedLegacySteering ? LegacyTargetSteeringInput : TargetSteeringInput;

	// [v2.44.0] 중립이 아닌 조향 목표가 있는지 여부입니다.
	const bool bHasSteeringIntent = FMath::Abs(ActiveTargetSteeringInput) > KINDA_SMALL_NUMBER;
	if (bHasSteeringIntent)
	{
		LastSteeringTurnRate = 2.0f / FMath::Max(SteeringLockToLockTimeSec, 0.01f);
		LastSteeringReturnRate = 0.0f;
		bSteeringReturningToCenter = false;
		CurrentSteeringInput = FMath::FInterpConstantTo(CurrentSteeringInput, ActiveTargetSteeringInput, SafeDeltaSeconds, LastSteeringTurnRate);
	}
	else
	{
		const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();
		LastSteeringTurnRate = 0.0f;
		LastSteeringReturnRate = CalculateSteeringReturnRateKmh(DriveStateSnapshot.CurrentSpeedKmh);
		bSteeringReturningToCenter = true;
		CurrentSteeringInput = FMath::FInterpConstantTo(CurrentSteeringInput, 0.0f, SafeDeltaSeconds, LastSteeringReturnRate);
	}

	CurrentSteeringInput = FMath::Clamp(CurrentSteeringInput, -1.0f, 1.0f);
	SetVehicleSteeringInput(CurrentSteeringInput);
}

// [v1.6.0] 차량 이동용 2D 입력 액션값을 읽어 해석 결과를 Drive 입력으로 전달합니다.
void ACFVehiclePawn::HandleVehicleMoveInput(const FInputActionValue& InputActionValue)
{
	const FVector2D MoveInputVector = InputActionValue.Get<FVector2D>();
	const float MoveInputMagnitude = FMath::Clamp(MoveInputVector.Length(), 0.0f, 1.0f);
	if (!ShouldAcceptActionInput(InputAction_VehicleMove, MoveInputMagnitude) || !CanProcessVehicleMoveInput(MoveInputMagnitude))
	{
		LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
		LastVehicleMoveInputResult.RawMoveInput = MoveInputVector;
		LastVehicleMoveInputResult.Magnitude = MoveInputMagnitude;
		// [v2.8.0] 입력이 최소 기준 아래로 내려가면 목표 조향을 0으로 두고 Tick의 중립 복귀에 맡깁니다.
		TargetSteeringInput = 0.0f;
		ReleaseInputOwnershipIfIdle();
		return;
	}

	UpdateInputOwnershipFromVehicleMove(MoveInputMagnitude);
	LegacyTargetSteeringInput = 0.0f;
	LastVehicleMoveInputResult = ResolveVehicleMoveInput(MoveInputVector);
	ApplyResolvedVehicleMoveInput(LastVehicleMoveInputResult);
}



void ACFVehiclePawn::HandleVehicleMoveReleased(const FInputActionValue&)
{
	LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
	TargetSteeringInput = 0.0f;
	if (CurrentInputOwnership == ECFVehicleInputOwnership::VehicleMove2D)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleThrottleInput);
		ResetAxisInput(&ACFVehiclePawn::SetVehicleBrakeInput);
		// [v2.8.0] 조향은 즉시 0으로 리셋하지 않고 Tick의 속도 기반 중립 복귀에 맡깁니다.
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
	// [v2.44.1] VehicleMove2D가 소유 중이 아닐 때는 소유권이 None이어도 남은 LegacyAxis 스로틀을 반드시 정리합니다.
	const bool bShouldResetLegacyThrottleOnRelease = CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D;
	if (bShouldResetLegacyThrottleOnRelease)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleThrottleInput);
	}
	ReleaseInputOwnershipIfIdle();
}

// [v2.44.0] LegacyAxis 조향 입력을 직접 적용 또는 smoothing 목표값으로 갱신합니다.
void ACFVehiclePawn::HandleSteeringInput(const FInputActionValue& InputActionValue)
{
	// [v2.44.0] 현재 입력 액션에서 읽은 원본 LegacyAxis 조향값입니다.
	const float SteeringAxisValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(InputAction_Steering, SteeringAxisValue))
	{
		if (CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D)
		{
			if (bSmoothLegacySteeringInput)
			{
				LegacyTargetSteeringInput = 0.0f;
			}
			else
			{
				ResetAxisInput(&ACFVehiclePawn::SetVehicleSteeringInput);
			}
			ReleaseInputOwnershipIfIdle();
		}
		return;
	}
	if (!CanProcessLegacyAxisInput(SteeringAxisValue))
	{
		return;
	}
	UpdateInputOwnershipFromLegacyAxis(SteeringAxisValue);
	if (bSmoothLegacySteeringInput)
	{
		LegacyTargetSteeringInput = FMath::Clamp(SteeringAxisValue, -1.0f, 1.0f);
		return;
	}

	LegacyTargetSteeringInput = 0.0f;
	SetVehicleSteeringInput(SteeringAxisValue);
}

// [v2.44.0] LegacyAxis 조향 해제 시 직접 조향 또는 smoothing 목표값을 중립으로 되돌립니다.
void ACFVehiclePawn::HandleSteeringReleased(const FInputActionValue&)
{
	// [v2.44.1] VehicleMove2D가 소유 중이 아닐 때는 소유권이 None이어도 남은 LegacyAxis 조향을 반드시 정리합니다.
	const bool bShouldResetLegacySteeringOnRelease = CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D;
	if (bShouldResetLegacySteeringOnRelease)
	{
		if (bSmoothLegacySteeringInput)
		{
			LegacyTargetSteeringInput = 0.0f;
		}
		else
		{
			ResetAxisInput(&ACFVehiclePawn::SetVehicleSteeringInput);
		}
	}
	ReleaseInputOwnershipIfIdle();
}

void ACFVehiclePawn::HandleBrakeInput(const FInputActionValue& InputActionValue)
{
	ApplyAxisInputFromAction(InputAction_Brake, InputActionValue, &ACFVehiclePawn::SetVehicleBrakeInput);
}

void ACFVehiclePawn::HandleBrakeReleased(const FInputActionValue&)
{
	// [v2.44.1] VehicleMove2D가 소유 중이 아닐 때는 소유권이 None이어도 남은 LegacyAxis 브레이크/후진 입력을 반드시 정리합니다.
	const bool bShouldResetLegacyBrakeOnRelease = CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D;
	if (bShouldResetLegacyBrakeOnRelease)
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

FCFVehicleFireRequest ACFVehiclePawn::BuildFireRequest()
{
	// [v2.17.0] 현재 월드 시간 또는 fallback 0초입니다.
	const float ClientFireTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// [v2.17.0] 이번 발사 요청에 사용할 요청 ID입니다.
	const int32 FireRequestId = NextFireRequestId++;

	if (VehicleAimComp)
	{
		return VehicleAimComp->BuildFireRequest(FireRequestId, ClientFireTimeSeconds);
	}

	// [v2.17.0] AimComp가 없을 때도 크래시 없이 반환할 fallback 요청입니다.
	FCFVehicleFireRequest FireRequest;
	FireRequest.FireRequestId = FireRequestId;
	FireRequest.ClientFireTimeSeconds = ClientFireTimeSeconds;
	FireRequest.AimOrigin = GetActorLocation();
	FireRequest.AimDirection = GetActorForwardVector();
	FireRequest.PredictedAimTargetLocation = GetActorLocation() + GetActorForwardVector() * 1000.0f;
	return FireRequest;
}

bool ACFVehiclePawn::ValidateFireRequestOnServer(const FCFVehicleFireRequest& FireRequest, FCFVehicleFireResult& OutFireResult)
{
	OutFireResult = FCFVehicleFireResult();
	OutFireResult.FireRequestId = FireRequest.FireRequestId;
	OutFireResult.ServerAimTargetLocation = FireRequest.PredictedAimTargetLocation;
	OutFireResult.ServerHitLocation = FireRequest.PredictedAimTargetLocation;

	if (!HasAuthority())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoAuthority;
		return false;
	}

	if (!GetController())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidOwner;
		return false;
	}

	if (!VehicleAimComp)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
		return false;
	}

	if (!VehicleAimComp->IsAimRuntimeReady())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::VehicleDisabled;
		return false;
	}

	// [v2.17.0] 서버 검증에 사용할 요청 조준 방향입니다.
	const FVector AimDirection = FVector(FireRequest.AimDirection);
	if (AimDirection.ContainsNaN() || AimDirection.IsNearlyZero())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// [v2.17.0] 서버 검증에 사용할 요청 조준 시작 위치입니다.
	const FVector AimOrigin = FVector(FireRequest.AimOrigin);
	if (AimOrigin.ContainsNaN())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimOrigin;
		return false;
	}

	// [v2.17.0] 서버 기준 Pawn 위치와 요청 조준 시작점 사이 거리입니다.
	const float AimOriginDistance = FVector::Dist(AimOrigin, GetActorLocation());

	// [v2.17.0] 기본 Aim Profile에서 허용하는 최대 거리입니다.
	const float MaxAimDistance = VehicleAimComp->GetDefaultAimProfile().MaxAimDistance;
	if (AimOriginDistance > MaxAimDistance)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimOrigin;
		return false;
	}

	if (!VehicleAimComp->IsFireRequestWithinDefaultProfile(FireRequest))
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::OutOfWeaponArc;
		return false;
	}

	if (VehicleAimComp->GetLocalAimState().bLocalAimBlocked)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::AimBlocked;
		return false;
	}

	OutFireResult.bAccepted = true;
	OutFireResult.RejectReason = ECFVehicleFireRejectReason::None;
	RunServerDummyHitScan(FireRequest, OutFireResult);
	return true;
}

bool ACFVehiclePawn::RunServerDummyHitScan(const FCFVehicleFireRequest& FireRequest, FCFVehicleFireResult& InOutFireResult) const
{
	if (!HasAuthority() || !VehicleAimComp)
	{
		return false;
	}

	// [v2.18.0] 서버 Trace에 사용할 월드입니다.
	UWorld* World = GetWorld();

	// [v2.18.0] 서버 Trace 시작 위치입니다.
	const FVector TraceStart = FVector(FireRequest.AimOrigin);

	// [v2.18.0] 서버 Trace 방향입니다.
	const FVector TraceDirection = FVector(FireRequest.AimDirection).GetSafeNormal();

	// [v2.18.0] 서버 Trace에 사용할 최대 거리입니다.
	const float TraceDistance = VehicleAimComp->GetDefaultAimProfile().MaxAimDistance;

	// [v2.18.0] 서버 Trace 종료 위치입니다.
	const FVector TraceEnd = TraceStart + TraceDirection * TraceDistance;

	if (!World || TraceDirection.IsNearlyZero())
	{
		InOutFireResult.ServerAimTargetLocation = TraceEnd;
		InOutFireResult.ServerHitLocation = TraceEnd;
		InOutFireResult.ServerHitNormal = FVector::UpVector;
		return false;
	}

	// [v2.18.0] 서버 Trace 적중 결과입니다.
	FHitResult HitResult;

	// [v2.18.0] 서버 Trace에서 Owner Pawn을 무시하기 위한 쿼리 설정입니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightServerAimTrace), false);
	QueryParams.AddIgnoredActor(this);

	// [v2.18.0] 서버 Trace가 월드의 가시성 채널에 적중했는지 여부입니다.
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bHit)
	{
		InOutFireResult.ServerAimTargetLocation = HitResult.ImpactPoint;
		InOutFireResult.ServerHitLocation = HitResult.ImpactPoint;
		InOutFireResult.ServerHitNormal = HitResult.ImpactNormal;
	}
	else
	{
		InOutFireResult.ServerAimTargetLocation = TraceEnd;
		InOutFireResult.ServerHitLocation = TraceEnd;
		InOutFireResult.ServerHitNormal = FVector::UpVector;
	}

	if (bDrawServerAimTraceDebug)
	{
		// [v2.18.0] 서버 Trace 디버그 라인 색상입니다.
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;

		DrawDebugLine(World, TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, TraceColor, false, ServerAimTraceDebugDuration, 0, 2.0f);
		if (bHit)
		{
			DrawDebugSphere(World, HitResult.ImpactPoint, 24.0f, 12, FColor::Yellow, false, ServerAimTraceDebugDuration);
		}
	}

	return bHit;
}

void ACFVehiclePawn::HandleFireStarted(const FInputActionValue&)
{
	LastFireRequest = BuildFireRequest();

	if (HasAuthority())
	{
		ServerRequestFire_Implementation(LastFireRequest);
		return;
	}

	ServerRequestFire(LastFireRequest);
}

void ACFVehiclePawn::ServerRequestFire_Implementation(const FCFVehicleFireRequest& FireRequest)
{
	LastFireRequest = FireRequest;

	// [v2.17.0] 서버 검증 결과를 담을 발사 결과입니다.
	FCFVehicleFireResult FireResult;

	// [v2.17.0] 서버 검증이 발사 요청을 승인했는지 여부입니다.
	const bool bAccepted = ValidateFireRequestOnServer(FireRequest, FireResult);

	if (VehicleAimComp)
	{
		VehicleAimComp->BuildServerAimStateFromFireRequest(FireRequest, FireResult.RejectReason, bAccepted);
		VehicleAimComp->ApplyServerFireResult(FireResult);
		VehicleAimComp->UpdateRepAimVisualFromFireResult(FireRequest, FireResult);
	}

	LastFireResult = FireResult;
	ClientReceiveFireResult(FireResult);
}

void ACFVehiclePawn::ClientReceiveFireResult_Implementation(const FCFVehicleFireResult& FireResult)
{
	LastFireResult = FireResult;

	if (VehicleAimComp)
	{
		VehicleAimComp->ApplyServerFireResult(FireResult);
	}
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
