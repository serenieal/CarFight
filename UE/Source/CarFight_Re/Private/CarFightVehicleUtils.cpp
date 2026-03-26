// Version: 4.2.0
// Date: 2026-03-18
// Description: CarFight 차량 물리/디버그 유틸리티 구현

#include "CarFightVehicleUtils.h"

#include "ChaosWheeledVehicleMovementComponent.h"

// 정적 변수 캐싱: 매 프레임 검색하면 느리므로, 처음에 한 번 찾은 위치를 저장해둡니다.
static FArrayProperty* CachedTransformArrayProp = nullptr;
static bool bHasAttemptedCache = false;

void UCarFightVehicleUtils::GetRealWheelTransform(
	UChaosWheeledVehicleMovementComponent* MovementComponent,
	int32 WheelIndex,
	FVector& OutOffset,
	FRotator& OutRotation)
{
	// 1. 초기화
	OutOffset = FVector::ZeroVector;
	OutRotation = FRotator::ZeroRotator;

	if (!MovementComponent)
	{
		return;
	}

	// 2. 변수 찾기 (최초 1회 실행)
	if (!bHasAttemptedCache)
	{
		bHasAttemptedCache = true;
		UClass* TargetClass = UChaosVehicleMovementComponent::StaticClass();

		// A. "InterpolatedWheelTransforms" (UE 5.0 ~ 5.4 이름) 검색
		CachedTransformArrayProp = FindFProperty<FArrayProperty>(TargetClass, FName("InterpolatedWheelTransforms"));

		// B. 만약 없다면 "WheelTransforms" (UE 5.5+ 변경 가능성) 검색
		if (!CachedTransformArrayProp)
		{
			CachedTransformArrayProp = FindFProperty<FArrayProperty>(TargetClass, FName("WheelTransforms"));
		}
	}

	// 3. 찾은 변수에서 데이터 꺼내기
	if (CachedTransformArrayProp)
	{
		// 컴포넌트 메모리 주소에서 배열의 위치를 찾습니다.
		void* TransformArrayAddress = CachedTransformArrayProp->ContainerPtrToValuePtr<void>(MovementComponent);

		// FScriptArrayHelper: 리플렉션 배열을 다루는 도우미
		FScriptArrayHelper TransformArrayHelper(CachedTransformArrayProp, TransformArrayAddress);

		if (TransformArrayHelper.IsValidIndex(WheelIndex))
		{
			// 배열의 i번째 요소(FTransform)를 가져옵니다.
			const FTransform* WheelWorldTransform = reinterpret_cast<const FTransform*>(TransformArrayHelper.GetRawPtr(WheelIndex));

			if (WheelWorldTransform)
			{
				// 4. 상대 좌표 변환 (차체 기준)
				const FTransform VehicleTransform = MovementComponent->UpdatedComponent ? MovementComponent->UpdatedComponent->GetComponentTransform() : FTransform::Identity;
				const FTransform RelativeWheelTransform = WheelWorldTransform->GetRelativeTransform(VehicleTransform);

				OutOffset = RelativeWheelTransform.GetLocation();
				OutRotation = RelativeWheelTransform.Rotator();
				return;
			}
		}
	}

	// 4. (실패 시 폴백) 리플렉션도 실패하면 기본값을 반환합니다.
}

FString UCarFightVehicleUtils::ConvDriveStateToDisplayString(ECFVehicleDriveState InDriveState)
{
	const UEnum* DriveStateEnum = StaticEnum<ECFVehicleDriveState>();
	if (!DriveStateEnum)
	{
		return TEXT("Unknown");
	}

	return DriveStateEnum->GetNameStringByValue(static_cast<int64>(InDriveState));
}

FString UCarFightVehicleUtils::MakeDriveStateSnapshotDebugString(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot)
{
	const FString DriveStateName = ConvDriveStateToDisplayString(InDriveStateSnapshot.CurrentDriveState);
	const FString DriveInputSummary = FString::Printf(
		TEXT("Throttle=%.2f Steering=%.2f Brake=%.2f Handbrake=%s"),
		InDriveStateSnapshot.CurrentInputState.ThrottleInput,
		InDriveStateSnapshot.CurrentInputState.SteeringInput,
		InDriveStateSnapshot.CurrentInputState.BrakeInput,
		InDriveStateSnapshot.CurrentInputState.bHandbrakePressed ? TEXT("True") : TEXT("False"));

	return FString::Printf(
		TEXT("State=%s | Speed=%.2f km/h | Forward=%.2f km/h | Grounded=%s | %s"),
		*DriveStateName,
		InDriveStateSnapshot.CurrentSpeedKmh,
		InDriveStateSnapshot.ForwardSpeedKmh,
		InDriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"),
		*DriveInputSummary);
}

FString UCarFightVehicleUtils::MakeDriveStateSnapshotMultilineDebugString(
	const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot,
	bool bIncludeInputState)
{
	FString DriveDebugSummary = FString::Printf(
		TEXT("State      : %s\nSpeed      : %.2f km/h\nForward    : %.2f km/h\nGrounded   : %s"),
		*ConvDriveStateToDisplayString(InDriveStateSnapshot.CurrentDriveState),
		InDriveStateSnapshot.CurrentSpeedKmh,
		InDriveStateSnapshot.ForwardSpeedKmh,
		InDriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"));

	if (bIncludeInputState)
	{
		DriveDebugSummary += FString::Printf(
			TEXT("\nThrottle   : %.2f\nSteering   : %.2f\nBrake      : %.2f\nHandbrake  : %s"),
			InDriveStateSnapshot.CurrentInputState.ThrottleInput,
			InDriveStateSnapshot.CurrentInputState.SteeringInput,
			InDriveStateSnapshot.CurrentInputState.BrakeInput,
			InDriveStateSnapshot.CurrentInputState.bHandbrakePressed ? TEXT("True") : TEXT("False"));
	}

	return DriveDebugSummary;
}

FString UCarFightVehicleUtils::MakeVehicleDebugSnapshotDebugString(
	const FCFVehicleDebugSnapshot& InVehicleDebugSnapshot,
	bool bIncludeRuntimeSummary,
	bool bIncludeTransitionSummary,
	bool bIncludeInputState)
{
	FString VehicleDebugSummary = FString::Printf(
		TEXT("Ready=%s | Drive=%s | Prev=%s | Changed=%s | Speed=%.2f km/h | Forward=%.2f km/h | Grounded=%s | DriveComp=%s | WheelSync=%s"),
		InVehicleDebugSnapshot.bRuntimeReady ? TEXT("True") : TEXT("False"),
		*ConvDriveStateToDisplayString(InVehicleDebugSnapshot.CurrentDriveState),
		*ConvDriveStateToDisplayString(InVehicleDebugSnapshot.PreviousDriveState),
		InVehicleDebugSnapshot.bDriveStateChangedThisFrame ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.DriveStateSnapshot.CurrentSpeedKmh,
		InVehicleDebugSnapshot.DriveStateSnapshot.ForwardSpeedKmh,
		InVehicleDebugSnapshot.DriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.bHasDriveComponent ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.bHasWheelSyncComponent ? TEXT("True") : TEXT("False"));

	if (bIncludeInputState)
	{
		const FString InputStateSummary = FString::Printf(
			TEXT(" | Input(T=%.2f S=%.2f B=%.2f HB=%s)"),
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.ThrottleInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.SteeringInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.BrakeInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.bHandbrakePressed ? TEXT("True") : TEXT("False"));
		VehicleDebugSummary += InputStateSummary;
	}

	if (bIncludeTransitionSummary)
	{
		VehicleDebugSummary += FString::Printf(TEXT(" | Transition=%s"), *InVehicleDebugSnapshot.DriveStateTransitionSummary);
	}

	if (bIncludeRuntimeSummary)
	{
		VehicleDebugSummary += FString::Printf(TEXT(" | Runtime=%s"), *InVehicleDebugSnapshot.RuntimeSummary);
	}

	return VehicleDebugSummary;
}

FString UCarFightVehicleUtils::MakeVehicleDebugSnapshotMultilineDebugString(
	const FCFVehicleDebugSnapshot& InVehicleDebugSnapshot,
	bool bIncludeRuntimeSummary,
	bool bIncludeTransitionSummary,
	bool bIncludeInputState)
{
	FString VehicleDebugSummary = FString::Printf(
		TEXT("RuntimeReady : %s\nDriveState   : %s\nPrevState    : %s\nChanged      : %s\nSpeed        : %.2f km/h\nForward      : %.2f km/h\nGrounded     : %s\nDriveComp    : %s\nWheelSync    : %s"),
		InVehicleDebugSnapshot.bRuntimeReady ? TEXT("True") : TEXT("False"),
		*ConvDriveStateToDisplayString(InVehicleDebugSnapshot.CurrentDriveState),
		*ConvDriveStateToDisplayString(InVehicleDebugSnapshot.PreviousDriveState),
		InVehicleDebugSnapshot.bDriveStateChangedThisFrame ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.DriveStateSnapshot.CurrentSpeedKmh,
		InVehicleDebugSnapshot.DriveStateSnapshot.ForwardSpeedKmh,
		InVehicleDebugSnapshot.DriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.bHasDriveComponent ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.bHasWheelSyncComponent ? TEXT("True") : TEXT("False"));

	if (bIncludeInputState)
	{
		VehicleDebugSummary += FString::Printf(
			TEXT("\nThrottle     : %.2f\nSteering     : %.2f\nBrake        : %.2f\nHandbrake    : %s"),
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.ThrottleInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.SteeringInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.BrakeInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.bHandbrakePressed ? TEXT("True") : TEXT("False"));
	}

	if (bIncludeTransitionSummary)
	{
		VehicleDebugSummary += FString::Printf(TEXT("\nTransition   : %s"), *InVehicleDebugSnapshot.DriveStateTransitionSummary);
	}

	if (bIncludeRuntimeSummary)
	{
		VehicleDebugSummary += FString::Printf(TEXT("\nRuntime      : %s"), *InVehicleDebugSnapshot.RuntimeSummary);
	}

	return VehicleDebugSummary;
}
