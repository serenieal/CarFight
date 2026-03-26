// Version: 1.1.0
// Date: 2026-03-19
// Description: CarFight 휠 시각 동기화 컴포넌트 구현 (운영 경로 정리 + DebugMode 기반 개발용 항목 숨김)
// Scope: 단일 축 테스트/Phase1Stub 제거. helper 비교/override/상태 관측은 DebugMode에서만 활성화됩니다.

#include "CFWheelSyncComp.h"

#include "CarFightVehicleUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFWheelSync, Log, All);

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
	// - Z / SpinPitch는 후속 단계에서 확장합니다.
	bApplySteeringYawInCpp = true;
	bApplySuspensionZInCpp = false;
	bApplySpinPitchInCpp = false;

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

	bWheelSyncReady = false;
	LastValidationSummary = TEXT("NotValidated");
	LastHelperCompareSummary = TEXT("HelperCompare:Disabled");
	LastHelperCompareWarnWheelIndices = TEXT("None");
	LastHelperCompareFrontRearSummary = TEXT("HelperCompareFR:Disabled");

	InitializeDefaultWheelNames();
	ResetBaseVisualCaches();
	ResetLastWheelStates();
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
	ResetHelperCompareStates();

	LastHelperCompareSummary = TEXT("HelperCompare:Reset");
	LastHelperCompareWarnWheelIndices = TEXT("None");
	LastHelperCompareFrontRearSummary = TEXT("HelperCompareFR:Reset");

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

bool UCFWheelSyncComp::BuildWheelVisualInputsPhase2(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs)
{
	if (!BuildWheelVisualInputsFromDebugPipe(DeltaSeconds, OutWheelInputs))
	{
		return false;
	}

	const bool bUseHelperInput = bDebugMode && bUseRealWheelTransformHelper;
	const bool bUseHelperCompare = bDebugMode && bEnableHelperCompareMode;

	ResetHelperCompareStates();
	LastHelperCompareSummary = bUseHelperInput ? TEXT("HelperCompare:Pending") : TEXT("HelperCompare:Disabled");
	LastHelperCompareWarnWheelIndices = TEXT("None");
	LastHelperCompareFrontRearSummary = bUseHelperCompare ? TEXT("HelperCompareFR:Pending") : TEXT("HelperCompareFR:Disabled");

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

	LastWheelVisualInputs = OutWheelInputs;

	// [v1.1.1] 긴 %s 체인은 UE 포맷 문자열 검증기에서 빌드 오류를 낼 수 있으므로
// 상태 문자열을 나눠서 안전하게 조합합니다.
	const TCHAR* DebugModeText = bDebugMode ? TEXT("True") : TEXT("False");
	const TCHAR* ApplyInCppText = bEnableApplyTransformsInCpp ? TEXT("True") : TEXT("False");
	const TCHAR* SteeringOwnedText = IsSteeringYawOwnedByCpp() ? TEXT("On") : TEXT("Off");
	const TCHAR* SuspensionOwnedText = IsSuspensionZOwnedByCpp() ? TEXT("On") : TEXT("Off");
	const TCHAR* SpinOwnedText = IsSpinPitchOwnedByCpp() ? TEXT("On") : TEXT("Off");
	const TCHAR* HelperUsedText = bUseHelperInput ? TEXT("True") : TEXT("False");
	const TCHAR* HelperAllText = (bDebugMode && bHelperOverridesDebugInputs) ? TEXT("True") : TEXT("False");
	const TCHAR* HelperYawText = (bDebugMode && bHelperOverridesSteeringYaw) ? TEXT("True") : TEXT("False");
	const TCHAR* HelperZText = (bDebugMode && bHelperOverridesSuspensionZ) ? TEXT("True") : TEXT("False");
	const TCHAR* HelperPitchText = (bDebugMode && bHelperOverridesSpinPitch) ? TEXT("True") : TEXT("False");
	const TCHAR* CompareText = bUseHelperCompare ? TEXT("True") : TEXT("False");

	LastValidationSummary = FString::Printf(
		TEXT("Phase2InputsBuilt: Count=%d"),
		OutWheelInputs.Num());

	LastValidationSummary += FString::Printf(
		TEXT(", Debug=%s, ApplyInCpp=%s, Own(Y/Z/P)=%s/%s/%s, Helper=%s, HAll=%s, HYaw=%s, HZ=%s, HPitch=%s, Cmp=%s"),
		DebugModeText,
		ApplyInCppText,
		SteeringOwnedText,
		SuspensionOwnedText,
		SpinOwnedText,
		HelperUsedText,
		HelperAllText,
		HelperYawText,
		HelperZText,
		HelperPitchText,
		CompareText);

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Log, TEXT("[WheelSync] %s"), *LastValidationSummary);
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
			FRotator CurrentMeshRotation = MeshComponent->GetRelativeRotation();
			CurrentMeshRotation.Pitch = TargetMeshRotation.Pitch;
			MeshComponent->SetRelativeRotation(CurrentMeshRotation);
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
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	TArray<USceneComponent*> FoundSceneComponents;
	OwnerActor->GetComponents<USceneComponent>(FoundSceneComponents);

	for (USceneComponent* SceneComponent : FoundSceneComponents)
	{
		if (SceneComponent && SceneComponent->GetFName() == ComponentName)
		{
			return SceneComponent;
		}
	}

	if (bDebugMode && bVerboseLog)
	{
		UE_LOG(LogCFWheelSync, Warning, TEXT("[WheelSync] SceneComponent not found: %s"), *ComponentName.ToString());
	}

	return nullptr;
}

UStaticMeshComponent* UCFWheelSyncComp::FindStaticMeshComponentByName(FName ComponentName) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	TArray<UStaticMeshComponent*> FoundMeshComponents;
	OwnerActor->GetComponents<UStaticMeshComponent>(FoundMeshComponents);

	for (UStaticMeshComponent* MeshComponent : FoundMeshComponents)
	{
		if (MeshComponent && MeshComponent->GetFName() == ComponentName)
		{
			return MeshComponent;
		}
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
