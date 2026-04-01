// Version: 1.1.9
// Date: 2026-04-01
// Description: CarFight 휠 시각 동기화 컴포넌트 구현 (컴포넌트 탐색/Helper 상태 초기화 중복 정리)
// Scope: 단일 축 테스트/Phase1Stub 제거. helper 비교/override/상태 관측은 DebugMode에서만 활성화됩니다.

#include "CFWheelSyncComp.h"

#include "CarFightVehicleUtils.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFWheelSync, Log, All);

namespace
{
	// helper 회전값에서 실제 바퀴 굴림에 가까운 축의 각도를 추출합니다.
	float ExtractWheelSpinAngleDegFromRotation(const FRotator& InWheelRotation)
	{
		// Pitch 축 절대값을 보관합니다.
		const float PitchAbsDeg = FMath::Abs(InWheelRotation.Pitch);

		// Roll 축 절대값을 보관합니다.
		const float RollAbsDeg = FMath::Abs(InWheelRotation.Roll);

		return (PitchAbsDeg >= RollAbsDeg) ? InWheelRotation.Pitch : InWheelRotation.Roll;
	}

	// Chaos 휠 인스턴스에서 현재 누적 회전각(deg)을 안전하게 읽습니다.
	float GetWheelSpinAngleDegFromRuntimeWheel(const UChaosWheeledVehicleMovementComponent* InMovementComponent, const int32 InWheelIndex)
	{
		if (!InMovementComponent || !InMovementComponent->Wheels.IsValidIndex(InWheelIndex) || InMovementComponent->Wheels[InWheelIndex] == nullptr)
		{
			return 0.0f;
		}

		return InMovementComponent->Wheels[InWheelIndex]->GetRotationAngle();
	}

	// Chaos 휠 인스턴스에서 현재 회전 각속도(deg/s)를 안전하게 읽습니다.
	float GetWheelSpinAngularVelocityDegPerSecFromRuntimeWheel(const UChaosWheeledVehicleMovementComponent* InMovementComponent, const int32 InWheelIndex)
	{
		if (!InMovementComponent || !InMovementComponent->Wheels.IsValidIndex(InWheelIndex) || InMovementComponent->Wheels[InWheelIndex] == nullptr)
		{
			return 0.0f;
		}

		return InMovementComponent->Wheels[InWheelIndex]->GetRotationAngularVelocity();
	}

	// Owner Actor에서 이름이 정확히 일치하는 지정 타입 컴포넌트를 찾습니다.
	template<typename ComponentType>
	ComponentType* FindActorComponentByExactName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// 현재 Actor에서 찾은 지정 타입 컴포넌트 목록입니다.
		TArray<ComponentType*> FoundComponents;
		OwnerActor->GetComponents<ComponentType>(FoundComponents);

		for (ComponentType* FoundComponent : FoundComponents)
		{
			if (FoundComponent && FoundComponent->GetFName() == ComponentName)
			{
				return FoundComponent;
			}
		}

		return nullptr;
	}

	// Helper 비교 상태 요약 문자열 묶음을 한 번에 초기화합니다.
	void SetHelperCompareStatus(UCFWheelSyncComp& WheelSyncComp, const TCHAR* SummaryText, const TCHAR* FrontRearSummaryText)
	{
		WheelSyncComp.LastHelperCompareSummary = SummaryText;
		WheelSyncComp.LastHelperCompareWarnWheelIndices = TEXT("None");
		WheelSyncComp.LastHelperCompareFrontRearSummary = FrontRearSummaryText;
	}
}

// WheelSync 기본 소유 축과 디버그/운영 기본값을 초기화합니다.
UCFWheelSyncComp::UCFWheelSyncComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	ExpectedWheelCount = 4;
	bAutoPrepareOnBeginPlay = false;
	bAutoFindComponentsByName = true;
	bDebugMode = false;
	bVerboseLog = false;

	// [v1.1.0] 신규 BP 기준 기본값:
	// - C++이 조향 Yaw를 실제 적용합니다.
	// - helper 실험 경로는 DebugMode에서만 사용합니다.
	bEnableApplyTransformsInCpp = true;
	bUseRealWheelTransformHelper = false;
	bHelperOverridesDebugInputs = false;
	bHelperOverridesSteeringYaw = false;
	bHelperOverridesSuspensionZ = false;
	bHelperOverridesSpinPitch = false;
	bLogHelperFallbackOnce = true;
	bHasLoggedHelperFallback = false;

	bEnableHelperCompareMode = false;
	bLogHelperCompareDetails = false;
	HelperCompareWarnDeltaDeg = 10.0f;
	HelperCompareWarnDeltaZ = 5.0f;

	// [v1.1.0] 축별 C++ 적용 기본값:
	// - 신규 BP 첫 단계에서는 조향 Yaw만 C++가 소유합니다.
	// - Z는 후속 단계에서 확장하고, SpinPitch는 현재 기준선에서 C++가 소유합니다.
	bApplySteeringYawInCpp = true;
	bApplySuspensionZInCpp = false;
	bApplySpinPitchInCpp = true;

	bUseSteeringYawDebugPipe = true;
	FrontWheelCountForSteering = 2;
	SteeringYawScaleDeg = 40.0f;
	SteeringVisualSign = 1.0f;

	bUseSuspensionZDebugPipe = false;
	SuspensionZDebugOffset = 0.0f;
	SuspensionVisualSign = 1.0f;

	bUseWheelSpinPitchDebugPipe = false;
	WheelSpinPitchDebugDeg = 0.0f;
	WheelSpinVisualSign = 1.0f;
	WheelSpinMeshAxisSign = -1.0f;
	WheelSpinAngularVelocityDeadZoneDegPerSec = 5.0f;
	WheelSpinForwardDeadZoneKmh = 1.0f;
	WheelSpinSlipAngularVelocityThresholdDegPerSec = 360.0f;
	WheelSpinDirectionChangeHoldTimeSeconds = 0.08f;
	WheelSpinLowSpeedVisualCapMaxKmh = 12.0f;
	WheelSpinLowSpeedMaxOverspeedRatio = 1.35f;

	bWheelSyncReady = false;
	LastValidationSummary = TEXT("NotValidated");
	LastInputBuildSummary = TEXT("NotBuilt");
	LastHelperCompareSummary = TEXT("HelperCompare:Disabled");
	LastHelperCompareWarnWheelIndices = TEXT("None");
	LastHelperCompareFrontRearSummary = TEXT("HelperCompareFR:Disabled");

	InitializeDefaultWheelNames();
	ResetBaseVisualCaches();
	ResetLastWheelStates();
	ResetRuntimeWheelSpinCaches();
	ResetHelperCompareStates();
}

void UCFWheelSyncComp::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoPrepareOnBeginPlay)
	{
		TryPrepareWheelSync();
	}
}

void UCFWheelSyncComp::ResetWheelSyncState()
{
	CachedVehicleMovementComponent = nullptr;
	CachedWheelAnchorComponents.Reset();
	CachedWheelMeshComponents.Reset();

	ResetBaseVisualCaches();
	LastWheelVisualInputs.Reset();
	ResetLastWheelStates();
	ResetRuntimeWheelSpinCaches();
	ResetHelperCompareStates();

	SetHelperCompareStatus(*this, TEXT("HelperCompare:Reset"), TEXT("HelperCompareFR:Reset"));

	bWheelSyncReady = false;
	bHasLoggedHelperFallback = false;
	LastValidationSummary = TEXT("Reset");

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] ResetWheelSyncState completed."));
	}
}

bool UCFWheelSyncComp::CacheVehicleMovementComponent()
{
	CachedVehicleMovementComponent = nullptr;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FailValidation(TEXT("Owner actor is null."));
	}

	CachedVehicleMovementComponent = OwnerActor->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	if (!CachedVehicleMovementComponent)
	{
		return FailValidation(TEXT("UChaosWheeledVehicleMovementComponent not found on owner."));
	}

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] Cached VehicleMovement: %s"), *GetNameSafe(CachedVehicleMovementComponent));
	}

	return true;
}

bool UCFWheelSyncComp::BuildWheelComponentCache()
{
	CachedWheelAnchorComponents.Reset();
	CachedWheelMeshComponents.Reset();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FailValidation(TEXT("Owner actor is null while building wheel component cache."));
	}

	if (!bAutoFindComponentsByName)
	{
		return FailValidation(TEXT("BuildWheelComponentCache requires bAutoFindComponentsByName=true."));
	}

	if (WheelAnchorComponentNames.Num() <= 0 || WheelMeshComponentNames.Num() <= 0)
	{
		return FailValidation(TEXT("Wheel name arrays are empty. Initialize wheel component names first."));
	}

	if (WheelAnchorComponentNames.Num() != WheelMeshComponentNames.Num())
	{
		return FailValidation(TEXT("WheelAnchorComponentNames and WheelMeshComponentNames count mismatch."));
	}

	for (int32 WheelIndex = 0; WheelIndex < WheelAnchorComponentNames.Num(); ++WheelIndex)
	{
		USceneComponent* FoundAnchorComponent = FindSceneComponentByName(WheelAnchorComponentNames[WheelIndex]);
		UStaticMeshComponent* FoundMeshComponent = FindStaticMeshComponentByName(WheelMeshComponentNames[WheelIndex]);

		CachedWheelAnchorComponents.Add(FoundAnchorComponent);
		CachedWheelMeshComponents.Add(FoundMeshComponent);
	}

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] BuildWheelComponentCache completed. Anchor=%d, Mesh=%d"),
			CachedWheelAnchorComponents.Num(),
			CachedWheelMeshComponents.Num());
	}

	return true;
}

bool UCFWheelSyncComp::CaptureBaseWheelVisualState()
{
	if (CachedWheelAnchorComponents.Num() != ExpectedWheelCount || CachedWheelMeshComponents.Num() != ExpectedWheelCount)
	{
		return FailValidation(TEXT("CaptureBaseWheelVisualState requires valid wheel caches with ExpectedWheelCount size."));
	}

	ResetBaseVisualCaches();

	for (int32 WheelIndex = 0; WheelIndex < ExpectedWheelCount; ++WheelIndex)
	{
		USceneComponent* AnchorComponent = CachedWheelAnchorComponents[WheelIndex];
		UStaticMeshComponent* MeshComponent = CachedWheelMeshComponents[WheelIndex];

		if (!AnchorComponent || !MeshComponent)
		{
			return FailValidation(FString::Printf(TEXT("CaptureBaseWheelVisualState failed: null component at wheel index %d."), WheelIndex));
		}

		BaseWheelAnchorLocations[WheelIndex] = AnchorComponent->GetRelativeLocation();
		BaseWheelAnchorRotations[WheelIndex] = AnchorComponent->GetRelativeRotation();
		BaseWheelMeshRotations[WheelIndex] = MeshComponent->GetRelativeRotation();
	}

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] Base wheel visual state captured."));
	}

	return true;
}

bool UCFWheelSyncComp::ValidateWheelSyncSetup()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FailValidation(TEXT("Owner actor is null."));
	}

	if (ExpectedWheelCount <= 0)
	{
		return FailValidation(TEXT("ExpectedWheelCount must be greater than zero."));
	}

	if (!CachedVehicleMovementComponent)
	{
		return FailValidation(TEXT("VehicleMovement cache is null. Call CacheVehicleMovementComponent or TryPrepareWheelSync first."));
	}

	if (CachedWheelAnchorComponents.Num() != ExpectedWheelCount)
	{
		return FailValidation(FString::Printf(TEXT("Anchor cache count mismatch. Expected=%d, Actual=%d"), ExpectedWheelCount, CachedWheelAnchorComponents.Num()));
	}

	if (CachedWheelMeshComponents.Num() != ExpectedWheelCount)
	{
		return FailValidation(FString::Printf(TEXT("Mesh cache count mismatch. Expected=%d, Actual=%d"), ExpectedWheelCount, CachedWheelMeshComponents.Num()));
	}

	for (int32 WheelIndex = 0; WheelIndex < ExpectedWheelCount; ++WheelIndex)
	{
		if (!CachedWheelAnchorComponents.IsValidIndex(WheelIndex) || CachedWheelAnchorComponents[WheelIndex] == nullptr)
		{
			return FailValidation(FString::Printf(TEXT("Anchor component is invalid at wheel index %d."), WheelIndex));
		}

		if (!CachedWheelMeshComponents.IsValidIndex(WheelIndex) || CachedWheelMeshComponents[WheelIndex] == nullptr)
		{
			return FailValidation(FString::Printf(TEXT("Mesh component is invalid at wheel index %d."), WheelIndex));
		}
	}

	return PassValidation(TEXT("Wheel sync setup validated."));
}

bool UCFWheelSyncComp::TryPrepareWheelSync()
{
	ResetWheelSyncState();

	if (!CacheVehicleMovementComponent())
	{
		return false;
	}

	if (!BuildWheelComponentCache())
	{
		return false;
	}

	if (!CaptureBaseWheelVisualState())
	{
		return false;
	}

	return ValidateWheelSyncSetup();
}

bool UCFWheelSyncComp::BuildWheelVisualInputsFromDebugPipe(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs)
{
	OutWheelInputs.Reset();

	if (!bWheelSyncReady)
	{
		return FailValidation(TEXT("BuildWheelVisualInputsFromDebugPipe called while wheel sync is not ready."));
	}

	if (!CachedVehicleMovementComponent)
	{
		return FailValidation(TEXT("BuildWheelVisualInputsFromDebugPipe requires CachedVehicleMovementComponent."));
	}

	if (ExpectedWheelCount <= 0)
	{
		return FailValidation(TEXT("ExpectedWheelCount must be greater than zero."));
	}

	OutWheelInputs.SetNum(ExpectedWheelCount);

	const float SteeringInput = CachedVehicleMovementComponent->GetSteeringInput();
	const float SteeringYawDegFromDebug = SteeringInput * SteeringYawScaleDeg * SteeringVisualSign;
	const float SuspensionZFromDebug = bUseSuspensionZDebugPipe ? (SuspensionZDebugOffset * SuspensionVisualSign) : 0.0f;
	const float SpinPitchDegFromDebug = bUseWheelSpinPitchDebugPipe ? (WheelSpinPitchDebugDeg * WheelSpinVisualSign) : 0.0f;

	for (int32 WheelIndex = 0; WheelIndex < ExpectedWheelCount; ++WheelIndex)
	{
		FCFWheelVisualInput& WheelInput = OutWheelInputs[WheelIndex];
		WheelInput.WheelIndex = WheelIndex;
		WheelInput.bIsValidInput = true;
		WheelInput.SteeringYawDeg = 0.0f;
		WheelInput.SpinPitchDeg = 0.0f;
		WheelInput.SpinPitchDeltaDeg = 0.0f;
		WheelInput.bApplySpinPitchAsDelta = false;
		WheelInput.SuspensionOffsetZ = 0.0f;

		if (bUseSteeringYawDebugPipe && IsFrontWheelIndexForSteering(WheelIndex))
		{
			WheelInput.SteeringYawDeg = SteeringYawDegFromDebug;
		}

		WheelInput.SuspensionOffsetZ = SuspensionZFromDebug;
		WheelInput.SpinPitchDeg = SpinPitchDegFromDebug;
	}

	LastValidationSummary = FString::Printf(
		TEXT("DebugPipeBuilt: Count=%d, SteeringIn=%.3f, YawDeg=%.2f, SuspZ=%.2f, SpinPitch=%.2f, Dt=%.4f"),
		OutWheelInputs.Num(),
		SteeringInput,
		SteeringYawDegFromDebug,
		SuspensionZFromDebug,
		SpinPitchDegFromDebug,
		DeltaSeconds);

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync][DebugPipe] %s"), *LastValidationSummary);
	}

	return true;
}

bool UCFWheelSyncComp::TryApplyRealTransformHelperToInput(int32 WheelIndex, const FCFWheelVisualInput& DebugWheelInput, FCFWheelVisualInput& InOutFinalWheelInput)
{
	if (!bDebugMode || !bUseRealWheelTransformHelper || !CachedVehicleMovementComponent)
	{
		return false;
	}

	FVector HelperOffset = FVector::ZeroVector;
	FRotator HelperRotation = FRotator::ZeroRotator;
	UCarFightVehicleUtils::GetRealWheelTransform(CachedVehicleMovementComponent, WheelIndex, HelperOffset, HelperRotation);

	if (LastHelperCompareStates.IsValidIndex(WheelIndex))
	{
		FCFWheelHelperCompareState& CompareState = LastHelperCompareStates[WheelIndex];
		CompareState.WheelIndex = WheelIndex;
		CompareState.bHelperSampleAttempted = true;
		CompareState.DebugSteeringYawDeg = DebugWheelInput.SteeringYawDeg;
		CompareState.HelperSteeringYawDeg = HelperRotation.Yaw;
		CompareState.DeltaSteeringYawDeg = HelperRotation.Yaw - DebugWheelInput.SteeringYawDeg;
		CompareState.DebugSpinPitchDeg = DebugWheelInput.SpinPitchDeg;
		CompareState.HelperSpinPitchDeg = HelperRotation.Pitch;
		CompareState.DeltaSpinPitchDeg = HelperRotation.Pitch - DebugWheelInput.SpinPitchDeg;
		CompareState.DebugSuspensionOffsetZ = DebugWheelInput.SuspensionOffsetZ;
		CompareState.HelperSuspensionOffsetZ = HelperOffset.Z;
		CompareState.DeltaSuspensionOffsetZ = HelperOffset.Z - DebugWheelInput.SuspensionOffsetZ;
	}

	const bool bOverrideYaw = bHelperOverridesDebugInputs || bHelperOverridesSteeringYaw;
	const bool bOverrideZ = bHelperOverridesDebugInputs || bHelperOverridesSuspensionZ;
	const bool bOverridePitch = bHelperOverridesDebugInputs || bHelperOverridesSpinPitch;

	if (bOverrideZ)
	{
		InOutFinalWheelInput.SuspensionOffsetZ = HelperOffset.Z;
	}

	if (bOverrideYaw)
	{
		InOutFinalWheelInput.SteeringYawDeg = HelperRotation.Yaw;
	}

	if (bOverridePitch)
	{
		InOutFinalWheelInput.SpinPitchDeg = HelperRotation.Pitch;
	}

	return true;
}

void UCFWheelSyncComp::ResetHelperCompareStates()
{
	const int32 SafeWheelCount = FMath::Max(0, ExpectedWheelCount);
	LastHelperCompareStates.Init(FCFWheelHelperCompareState(), SafeWheelCount);

	for (int32 WheelIndex = 0; WheelIndex < LastHelperCompareStates.Num(); ++WheelIndex)
	{
		LastHelperCompareStates[WheelIndex].WheelIndex = WheelIndex;
	}
}

void UCFWheelSyncComp::FinalizeHelperCompareSummary()
{
	float MaxAbsYawDelta = 0.0f;
	float MaxAbsPitchDelta = 0.0f;
	float MaxAbsZDelta = 0.0f;
	int32 WarnCount = 0;
	int32 SampledCount = 0;

	int32 FrontWarnCount = 0;
	int32 RearWarnCount = 0;
	float FrontMaxAbsYawDelta = 0.0f;
	float RearMaxAbsYawDelta = 0.0f;
	float FrontMaxAbsPitchDelta = 0.0f;
	float RearMaxAbsPitchDelta = 0.0f;
	float FrontMaxAbsZDelta = 0.0f;
	float RearMaxAbsZDelta = 0.0f;

	TArray<FString> WarnWheelIndexTokens;

	for (const FCFWheelHelperCompareState& CompareState : LastHelperCompareStates)
	{
		if (!CompareState.bHelperSampleAttempted)
		{
			continue;
		}

		++SampledCount;

		const float AbsYaw = FMath::Abs(CompareState.DeltaSteeringYawDeg);
		const float AbsPitch = FMath::Abs(CompareState.DeltaSpinPitchDeg);
		const float AbsZ = FMath::Abs(CompareState.DeltaSuspensionOffsetZ);
		MaxAbsYawDelta = FMath::Max(MaxAbsYawDelta, AbsYaw);
		MaxAbsPitchDelta = FMath::Max(MaxAbsPitchDelta, AbsPitch);
		MaxAbsZDelta = FMath::Max(MaxAbsZDelta, AbsZ);

		const bool bIsFrontWheel = IsFrontWheelIndexForSteering(CompareState.WheelIndex);
		if (bIsFrontWheel)
		{
			FrontMaxAbsYawDelta = FMath::Max(FrontMaxAbsYawDelta, AbsYaw);
			FrontMaxAbsPitchDelta = FMath::Max(FrontMaxAbsPitchDelta, AbsPitch);
			FrontMaxAbsZDelta = FMath::Max(FrontMaxAbsZDelta, AbsZ);
		}
		else
		{
			RearMaxAbsYawDelta = FMath::Max(RearMaxAbsYawDelta, AbsYaw);
			RearMaxAbsPitchDelta = FMath::Max(RearMaxAbsPitchDelta, AbsPitch);
			RearMaxAbsZDelta = FMath::Max(RearMaxAbsZDelta, AbsZ);
		}

		const bool bWarn = (AbsYaw > HelperCompareWarnDeltaDeg) || (AbsPitch > HelperCompareWarnDeltaDeg) || (AbsZ > HelperCompareWarnDeltaZ);
		if (bWarn)
		{
			++WarnCount;
			WarnWheelIndexTokens.Add(FString::FromInt(CompareState.WheelIndex));

			if (bIsFrontWheel)
			{
				++FrontWarnCount;
			}
			else
			{
				++RearWarnCount;
			}
		}

		if (bDebugMode && bLogHelperCompareDetails)
		{
			UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync][HelperCmp][W%d][%s] dYaw=%.2f dPitch=%.2f dZ=%.2f"),
				CompareState.WheelIndex,
				bIsFrontWheel ? TEXT("Front") : TEXT("Rear"),
				CompareState.DeltaSteeringYawDeg,
				CompareState.DeltaSpinPitchDeg,
				CompareState.DeltaSuspensionOffsetZ);
		}
	}

	LastHelperCompareWarnWheelIndices = (WarnWheelIndexTokens.Num() > 0)
		? FString::Join(WarnWheelIndexTokens, TEXT(","))
		: TEXT("None");

	LastHelperCompareFrontRearSummary = FString::Printf(
		TEXT("HelperCompareFR: FrontWarn=%d RearWarn=%d FrontMax(Y/P/Z)=%.2f/%.2f/%.2f RearMax(Y/P/Z)=%.2f/%.2f/%.2f"),
		FrontWarnCount,
		RearWarnCount,
		FrontMaxAbsYawDelta,
		FrontMaxAbsPitchDelta,
		FrontMaxAbsZDelta,
		RearMaxAbsYawDelta,
		RearMaxAbsPitchDelta,
		RearMaxAbsZDelta);

	LastHelperCompareSummary = FString::Printf(
		TEXT("HelperCompare: Sampled=%d Warn=%d WarnWheels=%s MaxYaw=%.2f MaxPitch=%.2f MaxZ=%.2f"),
		SampledCount,
		WarnCount,
		*LastHelperCompareWarnWheelIndices,
		MaxAbsYawDelta,
		MaxAbsPitchDelta,
		MaxAbsZDelta);
}

// 현재 프레임에 적용할 휠 시각 입력을 조합하고 필요 시 실제 스핀 회전값을 주입합니다.
bool UCFWheelSyncComp::BuildWheelVisualInputsPhase2(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs)
{
	if (!BuildWheelVisualInputsFromDebugPipe(DeltaSeconds, OutWheelInputs))
	{
		return false;
	}

	const bool bUseHelperInput = bDebugMode && bUseRealWheelTransformHelper;
	const bool bUseHelperCompare = bDebugMode && bEnableHelperCompareMode;

	ResetHelperCompareStates();
	SetHelperCompareStatus(
		*this,
		bUseHelperInput ? TEXT("HelperCompare:Pending") : TEXT("HelperCompare:Disabled"),
		bUseHelperCompare ? TEXT("HelperCompareFR:Pending") : TEXT("HelperCompareFR:Disabled"));

	if (bUseHelperInput)
	{
		for (int32 WheelIndex = 0; WheelIndex < OutWheelInputs.Num(); ++WheelIndex)
		{
			const FCFWheelVisualInput DebugInputCopy = OutWheelInputs[WheelIndex];
			TryApplyRealTransformHelperToInput(WheelIndex, DebugInputCopy, OutWheelInputs[WheelIndex]);
		}

		if (bUseHelperCompare)
		{
			FinalizeHelperCompareSummary();

			if (bDebugMode && bVerboseLog)
			{
				UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync][HelperCmp] %s"), *LastHelperCompareSummary);
			}
		}
	}
	else if (bDebugMode)
	{
		if (!bHasLoggedHelperFallback || !bLogHelperFallbackOnce)
		{
			UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] RealWheelTransform helper disabled. Using debug pipe inputs only."));
			bHasLoggedHelperFallback = true;
		}
	}

	// 현재 프레임의 차량 Forward 속도(km/h)입니다.
	const float CurrentForwardSpeedKmh = GetCurrentForwardSpeedKmhForWheelSpin();

	// 현재 WheelSync가 Mesh SpinPitch를 직접 적용해야 하는지 여부입니다.
	const bool bUseRuntimeSpinPitch = IsSpinPitchOwnedByCpp() && CachedVehicleMovementComponent != nullptr;

	// 첫 번째 휠의 실제 누적 회전각을 디버그 문자열에 남기기 위한 변수입니다.
	float FirstWheelRuntimeAngleDeg = 0.0f;

	// 첫 번째 휠의 실제 회전 각속도를 디버그 문자열에 남기기 위한 변수입니다.
	float FirstWheelRuntimeAngularVelocityDegPerSec = 0.0f;

	// 첫 번째 휠의 이번 프레임 스핀 누적 스텝(deg)을 디버그 문자열에 남기기 위한 변수입니다.
	float FirstWheelRuntimeSpinStepDeg = 0.0f;

	// 첫 번째 휠에 최종 누적된 스핀 각도를 디버그 문자열에 남기기 위한 변수입니다.
	float FirstWheelAccumulatedSpinDeg = 0.0f;

	// 첫 번째 휠에 최종 적용된 안정화 방향 부호를 디버그 문자열에 남기기 위한 변수입니다.
	int32 FirstWheelStableDirectionSign = 0;
	if (bUseRuntimeSpinPitch)
	{
		for (int32 WheelIndex = 0; WheelIndex < OutWheelInputs.Num(); ++WheelIndex)
		{
			// 현재 휠의 실제 누적 회전각(deg)입니다.
			const float RuntimeWheelAngleDeg = GetWheelSpinAngleDegFromRuntimeWheel(CachedVehicleMovementComponent, WheelIndex);

			// 현재 휠의 실제 회전 각속도(deg/s)입니다.
			const float RuntimeWheelAngularVelocityDegPerSec = GetWheelSpinAngularVelocityDegPerSecFromRuntimeWheel(CachedVehicleMovementComponent, WheelIndex);

			// 현재 휠 각속도의 절대값에서 dead zone을 적용한 회전 크기(deg/s)입니다.
			const float FilteredWheelAngularVelocityMagnitudeDegPerSec =
				(FMath::Abs(RuntimeWheelAngularVelocityDegPerSec) >= WheelSpinAngularVelocityDeadZoneDegPerSec)
				? FMath::Abs(RuntimeWheelAngularVelocityDegPerSec)
				: 0.0f;

			// 현재 휠 인덱스에 대응하는 runtime wheel 인스턴스가 유효한지 여부입니다.
			const bool bHasRuntimeWheelInstance =
				CachedVehicleMovementComponent->Wheels.IsValidIndex(WheelIndex) &&
				CachedVehicleMovementComponent->Wheels[WheelIndex] != nullptr;

			// 현재 휠 상태를 안전하게 읽을 수 있는지 여부입니다.
			const bool bHasWheelStatus = WheelIndex >= 0 && WheelIndex < CachedVehicleMovementComponent->GetNumWheels();

			// 현재 휠이 지면과 접촉 중인지 여부입니다.
			const bool bWheelInContact = bHasWheelStatus
				? CachedVehicleMovementComponent->GetWheelState(WheelIndex).bInContact
				: false;

			// 현재 차량 Forward 속도의 절대값(km/h)입니다.
			const float ForwardSpeedAbsKmh = FMath::Abs(CurrentForwardSpeedKmh);

			// 현재 프레임 시각 스핀 계산에 실제로 사용할 각속도 크기(deg/s)입니다.
			float VisualWheelAngularVelocityMagnitudeDegPerSec = FilteredWheelAngularVelocityMagnitudeDegPerSec;

			// 저속 visual cap 기능이 활성화될 최대 Forward 속도(km/h)입니다.
			const float LowSpeedVisualCapMaxKmh = FMath::Max(0.0f, WheelSpinLowSpeedVisualCapMaxKmh);

			// 저속 visual cap에서 허용할 최대 overspeed 배수입니다.
			const float LowSpeedMaxOverspeedRatio = FMath::Max(1.0f, WheelSpinLowSpeedMaxOverspeedRatio);

			if (bHasRuntimeWheelInstance && bWheelInContact && LowSpeedVisualCapMaxKmh > 0.0f && ForwardSpeedAbsKmh <= LowSpeedVisualCapMaxKmh)
			{
				// 현재 runtime wheel 반경(cm)입니다.
				const float RuntimeWheelRadiusCm = CachedVehicleMovementComponent->Wheels[WheelIndex]->GetWheelRadius();

				if (RuntimeWheelRadiusCm > KINDA_SMALL_NUMBER)
				{
					// 현재 차량 Forward 속도의 절대값(cm/s)입니다.
					const float ForwardSpeedAbsCmPerSec = ForwardSpeedAbsKmh / 0.036f;

					// 무슬립 기준으로 기대되는 휠 각속도(deg/s)입니다.
					const float ExpectedNoSlipAngularVelocityDegPerSec =
						FMath::RadiansToDegrees(ForwardSpeedAbsCmPerSec / RuntimeWheelRadiusCm);

					// 저속 접지 상태에서 허용할 최대 시각 휠 각속도(deg/s)입니다.
					const float MaxAllowedVisualAngularVelocityDegPerSec =
						ExpectedNoSlipAngularVelocityDegPerSec * LowSpeedMaxOverspeedRatio;

					VisualWheelAngularVelocityMagnitudeDegPerSec = FMath::Min(
						VisualWheelAngularVelocityMagnitudeDegPerSec,
						MaxAllowedVisualAngularVelocityDegPerSec);
				}
			}

			// 현재 프레임에 원하는 휠 회전 방향 후보 부호입니다.
			const int32 DesiredWheelSpinDirectionSign =
				ResolveDesiredWheelSpinDirectionSign(CurrentForwardSpeedKmh, RuntimeWheelAngularVelocityDegPerSec);

			// 현재 프레임에 hold 규칙을 반영한 최종 휠 회전 방향 부호입니다.
			const int32 StableWheelSpinDirectionSign =
				UpdateStableWheelSpinDirectionSign(WheelIndex, DesiredWheelSpinDirectionSign, DeltaSeconds);

			// 현재 프레임에 실제 휠 각속도와 DeltaSeconds로 적분한 스핀 스텝(deg)입니다.
			const float RuntimeWheelSpinStepDeg =
				VisualWheelAngularVelocityMagnitudeDegPerSec *
				DeltaSeconds *
				static_cast<float>(StableWheelSpinDirectionSign);

			// 누적 스핀 각 캐시가 유효하면 현재 프레임 스텝을 더합니다.
			if (AccumulatedRuntimeWheelSpinDeg.IsValidIndex(WheelIndex))
			{
				AccumulatedRuntimeWheelSpinDeg[WheelIndex] += RuntimeWheelSpinStepDeg;
			}

			if (LastRuntimeWheelAnglesDeg.IsValidIndex(WheelIndex))
			{
				LastRuntimeWheelAnglesDeg[WheelIndex] = RuntimeWheelAngleDeg;
			}

			if (bHasRuntimeWheelAngleSample.IsValidIndex(WheelIndex))
			{
				bHasRuntimeWheelAngleSample[WheelIndex] = true;
			}

			// 현재 프레임에 메시 스핀으로 사용할 누적 각도(deg)입니다.
			const float AccumulatedWheelSpinDeg =
				AccumulatedRuntimeWheelSpinDeg.IsValidIndex(WheelIndex)
				? AccumulatedRuntimeWheelSpinDeg[WheelIndex]
				: 0.0f;

			// 현재 프로젝트 mesh 축 보정까지 반영한 최종 visual spin 부호입니다.
			const float EffectiveWheelSpinVisualSign = WheelSpinVisualSign * WheelSpinMeshAxisSign;

			OutWheelInputs[WheelIndex].SpinPitchDeg = AccumulatedWheelSpinDeg * EffectiveWheelSpinVisualSign;
			OutWheelInputs[WheelIndex].SpinPitchDeltaDeg = RuntimeWheelSpinStepDeg * EffectiveWheelSpinVisualSign;
			OutWheelInputs[WheelIndex].bApplySpinPitchAsDelta = true;

			if (WheelIndex == 0)
			{
				FirstWheelRuntimeAngleDeg = RuntimeWheelAngleDeg;
				FirstWheelRuntimeSpinStepDeg = RuntimeWheelSpinStepDeg;
				FirstWheelRuntimeAngularVelocityDegPerSec = RuntimeWheelAngularVelocityDegPerSec;
				FirstWheelAccumulatedSpinDeg = OutWheelInputs[WheelIndex].SpinPitchDeg;
				FirstWheelStableDirectionSign = StableWheelSpinDirectionSign;
			}
		}
	}

	LastWheelVisualInputs = OutWheelInputs;

	// [v1.1.1] 긴 %s 체인은 UE 포맷 문자열 검증기에서 빌드 오류를 낼 수 있으므로
// 상태 문자열을 나눠서 안전하게 조합합니다.
	// 현재 디버그 모드 활성 여부를 문자열로 표현한 값입니다.
	const TCHAR* DebugModeText = bDebugMode ? TEXT("True") : TEXT("False");

	// 현재 Transform C++ 적용 활성 여부를 문자열로 표현한 값입니다.
	const TCHAR* ApplyInCppText = bEnableApplyTransformsInCpp ? TEXT("True") : TEXT("False");

	// 현재 SpinPitch 소유 여부를 문자열로 표현한 값입니다.
	const TCHAR* SpinOwnedText = IsSpinPitchOwnedByCpp() ? TEXT("On") : TEXT("Off");

	// 현재 Runtime 스핀 경로 사용 여부를 문자열로 표현한 값입니다.
	const TCHAR* RuntimeSpinText = bUseRuntimeSpinPitch ? TEXT("True") : TEXT("False");

	// 현재 Wheel mesh 축 보정까지 반영된 최종 visual spin 부호입니다.
	const float EffectiveWheelSpinVisualSign = WheelSpinVisualSign * WheelSpinMeshAxisSign;

	LastInputBuildSummary = FString::Printf(
		TEXT("Phase2InputsBuilt: Count=%d, Debug=%s, ApplyInCpp=%s, SpinOwned=%s, RuntimeSpin=%s, Fwd=%.2f, VisualSign=%.1f, SpinW0(Raw=%.2f, Step=%.2f, Vel=%.2f, Accum=%.2f, Sign=%d)"),
		OutWheelInputs.Num(),
		DebugModeText,
		ApplyInCppText,
		SpinOwnedText,
		RuntimeSpinText,
		CurrentForwardSpeedKmh,
		EffectiveWheelSpinVisualSign,
		FirstWheelRuntimeAngleDeg,
		FirstWheelRuntimeSpinStepDeg,
		FirstWheelRuntimeAngularVelocityDegPerSec,
		FirstWheelAccumulatedSpinDeg,
		FirstWheelStableDirectionSign);

	LastValidationSummary = LastInputBuildSummary;

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] %s"), *LastInputBuildSummary);
	}

	return true;
}

bool UCFWheelSyncComp::ApplySingleWheelInputPhase2(const FCFWheelVisualInput& WheelInput)
{
	if (!WheelInput.bIsValidInput)
	{
		return false;
	}

	if (!CachedWheelAnchorComponents.IsValidIndex(WheelInput.WheelIndex) || !CachedWheelMeshComponents.IsValidIndex(WheelInput.WheelIndex))
	{
		UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync] Invalid wheel index in ApplySingleWheelInputPhase2: %d"), WheelInput.WheelIndex);
		return false;
	}

	USceneComponent* AnchorComponent = CachedWheelAnchorComponents[WheelInput.WheelIndex];
	UStaticMeshComponent* MeshComponent = CachedWheelMeshComponents[WheelInput.WheelIndex];
	if (!AnchorComponent || !MeshComponent)
	{
		UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync] Null wheel component in ApplySingleWheelInputPhase2. Index=%d"), WheelInput.WheelIndex);
		return false;
	}

	if (!BaseWheelAnchorLocations.IsValidIndex(WheelInput.WheelIndex) ||
		!BaseWheelAnchorRotations.IsValidIndex(WheelInput.WheelIndex) ||
		!BaseWheelMeshRotations.IsValidIndex(WheelInput.WheelIndex))
	{
		UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync] Base visual cache missing for wheel index %d"), WheelInput.WheelIndex);
		return false;
	}

	FVector TargetAnchorLocation = BaseWheelAnchorLocations[WheelInput.WheelIndex];
	TargetAnchorLocation.Z += WheelInput.SuspensionOffsetZ;

	FRotator TargetAnchorRotation = BaseWheelAnchorRotations[WheelInput.WheelIndex];
	TargetAnchorRotation.Yaw += WheelInput.SteeringYawDeg;

	FRotator TargetMeshRotation = BaseWheelMeshRotations[WheelInput.WheelIndex];
	TargetMeshRotation.Pitch += WheelInput.SpinPitchDeg;

	if (bEnableApplyTransformsInCpp)
	{
		const bool bOwnSteeringYaw = IsSteeringYawOwnedByCpp();
		const bool bOwnSuspensionZ = IsSuspensionZOwnedByCpp();
		const bool bOwnSpinPitch = IsSpinPitchOwnedByCpp();

		if (bOwnSuspensionZ)
		{
			FVector CurrentAnchorLocation = AnchorComponent->GetRelativeLocation();
			CurrentAnchorLocation.Z = TargetAnchorLocation.Z;
			AnchorComponent->SetRelativeLocation(CurrentAnchorLocation);
		}

		if (bOwnSteeringYaw)
		{
			FRotator CurrentAnchorRotation = AnchorComponent->GetRelativeRotation();
			CurrentAnchorRotation.Yaw = TargetAnchorRotation.Yaw;
			AnchorComponent->SetRelativeRotation(CurrentAnchorRotation);
		}

		if (bOwnSpinPitch)
		{
			if (WheelInput.bApplySpinPitchAsDelta)
			{
				// 현재 프레임 휠 메시의 local pitch delta 회전값입니다.
				const FRotator MeshLocalSpinDeltaRotation(WheelInput.SpinPitchDeltaDeg, 0.0f, 0.0f);

				MeshComponent->AddLocalRotation(MeshLocalSpinDeltaRotation, false, nullptr, ETeleportType::TeleportPhysics);
			}
			else
			{
				FRotator CurrentMeshRotation = MeshComponent->GetRelativeRotation();
				CurrentMeshRotation.Pitch = TargetMeshRotation.Pitch;
				MeshComponent->SetRelativeRotation(CurrentMeshRotation);
			}
		}
	}

	if (LastWheelVisualStates.IsValidIndex(WheelInput.WheelIndex))
	{
		FCFWheelVisualState& WheelState = LastWheelVisualStates[WheelInput.WheelIndex];
		WheelState.WheelIndex = WheelInput.WheelIndex;
		WheelState.bApplied = true;
		WheelState.LastAppliedAnchorLocation = TargetAnchorLocation;
		WheelState.LastAppliedAnchorRotation = TargetAnchorRotation;
		WheelState.LastAppliedMeshRotation = TargetMeshRotation;
	}

	return true;
}

bool UCFWheelSyncComp::ApplyWheelVisualInputsPhase2(const TArray<FCFWheelVisualInput>& InWheelInputs)
{
	if (!bWheelSyncReady)
	{
		return FailValidation(TEXT("ApplyWheelVisualInputsPhase2 called while wheel sync is not ready."));
	}

	if (InWheelInputs.Num() != ExpectedWheelCount)
	{
		return FailValidation(FString::Printf(TEXT("Phase2 input count mismatch. Expected=%d, Actual=%d"), ExpectedWheelCount, InWheelInputs.Num()));
	}

	ResetLastWheelStates();

	bool bAllAppliedSuccessfully = true;
	for (const FCFWheelVisualInput& WheelInput : InWheelInputs)
	{
		const bool bSingleApplied = ApplySingleWheelInputPhase2(WheelInput);
		bAllAppliedSuccessfully = bAllAppliedSuccessfully && bSingleApplied;
	}

	LastValidationSummary = FString::Printf(
		TEXT("Phase2Applied: Count=%d, CppApply=%s, Own(Y/Z/P)=%s/%s/%s, Success=%s"),
		InWheelInputs.Num(),
		bEnableApplyTransformsInCpp ? TEXT("True") : TEXT("False"),
		IsSteeringYawOwnedByCpp() ? TEXT("On") : TEXT("Off"),
		IsSuspensionZOwnedByCpp() ? TEXT("On") : TEXT("Off"),
		IsSpinPitchOwnedByCpp() ? TEXT("On") : TEXT("Off"),
		bAllAppliedSuccessfully ? TEXT("True") : TEXT("False"));

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] ApplyWheelVisualInputsPhase2 done. CppApply=%s, Success=%s"),
			bEnableApplyTransformsInCpp ? TEXT("True") : TEXT("False"),
			bAllAppliedSuccessfully ? TEXT("True") : TEXT("False"));
	}

	return bAllAppliedSuccessfully;
}

bool UCFWheelSyncComp::UpdateWheelVisualsPhase2(float DeltaSeconds)
{
	TArray<FCFWheelVisualInput> BuiltInputs;
	if (!BuildWheelVisualInputsPhase2(DeltaSeconds, BuiltInputs))
	{
		return false;
	}

	return ApplyWheelVisualInputsPhase2(BuiltInputs);
}

bool UCFWheelSyncComp::IsWheelSyncReady() const
{
	return bWheelSyncReady;
}

bool UCFWheelSyncComp::IsSteeringYawOwnedByCpp() const
{
	return bEnableApplyTransformsInCpp && bApplySteeringYawInCpp;
}

bool UCFWheelSyncComp::IsSuspensionZOwnedByCpp() const
{
	return bEnableApplyTransformsInCpp && bApplySuspensionZInCpp;
}

bool UCFWheelSyncComp::IsSpinPitchOwnedByCpp() const
{
	return bEnableApplyTransformsInCpp && bApplySpinPitchInCpp;
}

// 현재 차량 Forward 속도(km/h)를 Wheel spin 방향 안정화용으로 계산합니다.
float UCFWheelSyncComp::GetCurrentForwardSpeedKmhForWheelSpin() const
{
	// UpdatedComponent 기준 현재 선형 속도(cm/s)를 보관하는 변수입니다.
	const FVector CurrentLinearVelocityCmPerSec =
		(CachedVehicleMovementComponent && CachedVehicleMovementComponent->UpdatedComponent)
		? CachedVehicleMovementComponent->UpdatedComponent->GetComponentVelocity()
		: FVector::ZeroVector;

	// UpdatedComponent 기준 현재 Forward 방향 벡터입니다.
	const FVector CurrentForwardVector =
		(CachedVehicleMovementComponent && CachedVehicleMovementComponent->UpdatedComponent)
		? CachedVehicleMovementComponent->UpdatedComponent->GetForwardVector()
		: FVector::ForwardVector;

	// 현재 Forward 축 선속도(cm/s)입니다.
	const float CurrentForwardSpeedCmPerSec = FVector::DotProduct(CurrentLinearVelocityCmPerSec, CurrentForwardVector);

	// 현재 Forward 축 선속도(km/h)입니다.
	const float CurrentForwardSpeedKmh = CurrentForwardSpeedCmPerSec * 0.036f;

	return CurrentForwardSpeedKmh;
}

// 휠 각속도와 Forward 속도 기준으로 현재 프레임이 원하는 회전 방향 부호를 결정합니다.
int32 UCFWheelSyncComp::ResolveDesiredWheelSpinDirectionSign(float InForwardSpeedKmh, float InWheelAngularVelocityDegPerSec) const
{
	// Forward 속도의 절대값입니다.
	const float ForwardSpeedAbsKmh = FMath::Abs(InForwardSpeedKmh);

	// 휠 각속도의 절대값입니다.
	const float WheelAngularVelocityAbsDegPerSec = FMath::Abs(InWheelAngularVelocityDegPerSec);

	// Forward 속도만으로도 진행 방향을 신뢰할 수 있는지 여부입니다.
	const bool bHasReliableForwardDirection = (ForwardSpeedAbsKmh >= WheelSpinForwardDeadZoneKmh);

	// 정지 근처에서도 휠 자체 부호를 허용할 만큼 강한 휠스핀인지 여부입니다.
	const bool bHasStrongWheelSpinDirection = (WheelAngularVelocityAbsDegPerSec >= WheelSpinSlipAngularVelocityThresholdDegPerSec);

	// Forward 속도 부호를 정수 방향 부호로 변환한 값입니다.
	const int32 ForwardDirectionSign = (InForwardSpeedKmh > 0.0f) ? 1 : ((InForwardSpeedKmh < 0.0f) ? -1 : 0);

	// 휠 각속도 부호를 정수 방향 부호로 변환한 값입니다.
	const int32 WheelDirectionSign = (InWheelAngularVelocityDegPerSec > 0.0f) ? 1 : ((InWheelAngularVelocityDegPerSec < 0.0f) ? -1 : 0);

	if (bHasReliableForwardDirection)
	{
		return ForwardDirectionSign;
	}

	if (bHasStrongWheelSpinDirection)
	{
		return WheelDirectionSign;
	}

	return 0;
}

// 휠별 방향 hold 규칙을 반영해 실제 stable sign을 갱신합니다.
int32 UCFWheelSyncComp::UpdateStableWheelSpinDirectionSign(int32 InWheelIndex, int32 InDesiredDirectionSign, float InDeltaSeconds)
{
	if (!StableWheelSpinDirectionSigns.IsValidIndex(InWheelIndex) ||
		!PendingWheelSpinDirectionSigns.IsValidIndex(InWheelIndex) ||
		!WheelSpinDirectionHoldElapsedSeconds.IsValidIndex(InWheelIndex))
	{
		return InDesiredDirectionSign;
	}

	// 현재 휠의 안정화 방향 부호입니다.
	int32& StableDirectionSign = StableWheelSpinDirectionSigns[InWheelIndex];

	// 현재 휠의 방향 전환 후보 부호입니다.
	int32& PendingDirectionSign = PendingWheelSpinDirectionSigns[InWheelIndex];

	// 현재 휠의 방향 전환 후보 유지 시간(sec)입니다.
	float& DirectionHoldElapsedSeconds = WheelSpinDirectionHoldElapsedSeconds[InWheelIndex];

	if (InDesiredDirectionSign == 0)
	{
		PendingDirectionSign = 0;
		DirectionHoldElapsedSeconds = 0.0f;
		return StableDirectionSign;
	}

	if (StableDirectionSign == 0)
	{
		StableDirectionSign = InDesiredDirectionSign;
		PendingDirectionSign = 0;
		DirectionHoldElapsedSeconds = 0.0f;
		return StableDirectionSign;
	}

	if (InDesiredDirectionSign == StableDirectionSign)
	{
		PendingDirectionSign = 0;
		DirectionHoldElapsedSeconds = 0.0f;
		return StableDirectionSign;
	}

	if (PendingDirectionSign != InDesiredDirectionSign)
	{
		PendingDirectionSign = InDesiredDirectionSign;
		DirectionHoldElapsedSeconds = 0.0f;
		return StableDirectionSign;
	}

	DirectionHoldElapsedSeconds += InDeltaSeconds;

	if (DirectionHoldElapsedSeconds >= WheelSpinDirectionChangeHoldTimeSeconds)
	{
		StableDirectionSign = InDesiredDirectionSign;
		PendingDirectionSign = 0;
		DirectionHoldElapsedSeconds = 0.0f;
	}

	return StableDirectionSign;
}

void UCFWheelSyncComp::InitializeDefaultWheelNames()
{
	WheelAnchorComponentNames =
	{
		TEXT("Wheel_Anchor_FL"),
		TEXT("Wheel_Anchor_FR"),
		TEXT("Wheel_Anchor_RL"),
		TEXT("Wheel_Anchor_RR")
	};

	WheelMeshComponentNames =
	{
		TEXT("Wheel_Mesh_FL"),
		TEXT("Wheel_Mesh_FR"),
		TEXT("Wheel_Mesh_RL"),
		TEXT("Wheel_Mesh_RR")
	};
}

USceneComponent* UCFWheelSyncComp::FindSceneComponentByName(FName ComponentName) const
{
	// 현재 검색 대상 Owner Actor 포인터입니다.
	AActor* OwnerActor = GetOwner();

	// 이름으로 찾은 SceneComponent 결과 포인터입니다.
	USceneComponent* FoundSceneComponent = FindActorComponentByExactName<USceneComponent>(OwnerActor, ComponentName);
	if (FoundSceneComponent)
	{
		return FoundSceneComponent;
	}

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync] SceneComponent not found: %s"), *ComponentName.ToString());
	}

	return nullptr;
}

UStaticMeshComponent* UCFWheelSyncComp::FindStaticMeshComponentByName(FName ComponentName) const
{
	// 현재 검색 대상 Owner Actor 포인터입니다.
	AActor* OwnerActor = GetOwner();

	// 이름으로 찾은 StaticMeshComponent 결과 포인터입니다.
	UStaticMeshComponent* FoundMeshComponent = FindActorComponentByExactName<UStaticMeshComponent>(OwnerActor, ComponentName);
	if (FoundMeshComponent)
	{
		return FoundMeshComponent;
	}

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync] StaticMeshComponent not found: %s"), *ComponentName.ToString());
	}

	return nullptr;
}

bool UCFWheelSyncComp::IsFrontWheelIndexForSteering(int32 WheelIndex) const
{
	return (WheelIndex >= 0) && (WheelIndex < FrontWheelCountForSteering);
}

void UCFWheelSyncComp::ResetBaseVisualCaches()
{
	const int32 SafeWheelCount = FMath::Max(0, ExpectedWheelCount);
	BaseWheelAnchorLocations.Init(FVector::ZeroVector, SafeWheelCount);
	BaseWheelAnchorRotations.Init(FRotator::ZeroRotator, SafeWheelCount);
	BaseWheelMeshRotations.Init(FRotator::ZeroRotator, SafeWheelCount);
}

void UCFWheelSyncComp::ResetLastWheelStates()
{
	const int32 SafeWheelCount = FMath::Max(0, ExpectedWheelCount);
	LastWheelVisualStates.Init(FCFWheelVisualState(), SafeWheelCount);

	for (int32 WheelIndex = 0; WheelIndex < LastWheelVisualStates.Num(); ++WheelIndex)
	{
		LastWheelVisualStates[WheelIndex].WheelIndex = WheelIndex;
		LastWheelVisualStates[WheelIndex].bApplied = false;
	}
}

// 실제 휠 회전각 캐시와 누적 스핀 캐시를 기본 상태로 초기화합니다.
void UCFWheelSyncComp::ResetRuntimeWheelSpinCaches()
{
	// 현재 기대 휠 개수 기준의 안전한 배열 크기입니다.
	const int32 SafeWheelCount = FMath::Max(0, ExpectedWheelCount);

	// 마지막 프레임에 읽은 실제 휠 회전각(deg) 배열입니다.
	LastRuntimeWheelAnglesDeg.Init(0.0f, SafeWheelCount);

	// 래핑을 풀어서 누적한 실제 휠 시각 스핀 각(deg) 배열입니다.
	AccumulatedRuntimeWheelSpinDeg.Init(0.0f, SafeWheelCount);

	// 실제 휠 회전각의 이전 프레임 샘플 유효 여부 배열입니다.
	bHasRuntimeWheelAngleSample.Init(false, SafeWheelCount);

	// 현재 휠별 안정화 방향 부호 배열입니다.
	StableWheelSpinDirectionSigns.Init(0, SafeWheelCount);

	// 현재 휠별 방향 전환 후보 부호 배열입니다.
	PendingWheelSpinDirectionSigns.Init(0, SafeWheelCount);

	// 현재 휠별 방향 전환 후보 유지 시간(sec) 배열입니다.
	WheelSpinDirectionHoldElapsedSeconds.Init(0.0f, SafeWheelCount);
}

bool UCFWheelSyncComp::FailValidation(const FString& FailureMessage)
{
	bWheelSyncReady = false;
	LastValidationSummary = FString::Printf(TEXT("Invalid: %s"), *FailureMessage);

	UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync][Invalid] %s"), *FailureMessage);
	return false;
}

bool UCFWheelSyncComp::PassValidation(const FString& SuccessMessage)
{
	bWheelSyncReady = true;
	LastValidationSummary = SuccessMessage;

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync][Ready] %s"), *SuccessMessage);
	}

	return true;
}
