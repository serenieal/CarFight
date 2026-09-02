// Version: 4.2.5
// Date: 2026-08-29
// Description: CarFight 차량 디버그 유틸리티 구현
// Changelog: v4.2.5 - 사용 중단된 GetRealWheelTransform reflection 구현과 미사용 AppendWrappedDebugSummaryLines 제거
// Migration: WheelSync runtime은 UCFWheelSyncComp의 정식 Chaos wheel 경로를 계속 사용하며 동작 변경 없음

#include "CarFightVehicleUtils.h"

// 요약 문자열에서 특정 접두사의 값을 찾아 반환합니다.
static FString ExtractSummaryValueByPrefix(
	const FString& InSummaryText,
	const FString& InPrefixText)
{
	// 구분자 기준으로 분리한 세부 조각 배열입니다.
	TArray<FString> SummaryParts;
	InSummaryText.ParseIntoArray(SummaryParts, TEXT(", "), true);

	// 접두사와 일치하는 세부 조각을 순서대로 찾습니다.
	for (const FString& SummaryPart : SummaryParts)
	{
		if (SummaryPart.StartsWith(InPrefixText))
		{
			return SummaryPart.RightChop(InPrefixText.Len());
		}
	}

	return FString();
}

// Runtime 요약에서 현재 운영에 필요한 핵심 정보만 추려 여러 줄로 붙입니다.
static void AppendCompactRuntimeSummaryLines(
	FString& InOutDebugSummary,
	const FString& InRuntimeSummaryText)
{
	// 구분자 기준으로 분리한 Runtime 요약 조각 배열입니다.
	TArray<FString> RuntimeSummaryParts;
	InRuntimeSummaryText.ParseIntoArray(RuntimeSummaryParts, TEXT(" | "), true);

	// Runtime 메인 상태 한 줄 문자열입니다.
	FString RuntimeCoreLine;

	// Runtime 보조 상태 라인 배열입니다.
	TArray<FString> RuntimeDetailLines;

	// Runtime 경고 라인 배열입니다.
	TArray<FString> RuntimeWarningLines;

	// 분리된 Runtime 요약 조각을 순서대로 해석합니다.
	for (const FString& RuntimeSummaryPart : RuntimeSummaryParts)
	{
		if (RuntimeSummaryPart.StartsWith(TEXT("VehicleRuntime: ")))
		{
			// 현재 데이터/드라이브/휠싱크 준비 상태 핵심 문자열입니다.
			RuntimeCoreLine = RuntimeSummaryPart.RightChop(15).Replace(TEXT(", "), TEXT(" | "));
			continue;
		}

		if (RuntimeSummaryPart.StartsWith(TEXT("VehicleLayout: ")) || RuntimeSummaryPart.StartsWith(TEXT("VehicleLayoutConfig: ")))
		{
			// 현재 레이아웃 운영 기준을 짧게 보여줄 문자열입니다.
			const FString LayoutSummaryText = RuntimeSummaryPart.StartsWith(TEXT("VehicleLayout: "))
				? RuntimeSummaryPart.RightChop(15)
				: RuntimeSummaryPart.RightChop(21);

			if (LayoutSummaryText.Contains(TEXT("ManualAnchorLayout=Required")))
			{
				RuntimeDetailLines.Add(TEXT("Layout=ManualAnchor"));
			}
			else
			{
				RuntimeDetailLines.Add(LayoutSummaryText);
			}
			continue;
		}

		if (RuntimeSummaryPart.StartsWith(TEXT("WheelSyncBuild=")))
		{
			// WheelSync 입력 생성 단계 세부 문자열입니다.
			const FString WheelSyncBuildSummaryText = RuntimeSummaryPart.RightChop(15);

			// C++ 적용 여부 문자열입니다.
			const FString ApplyInCppText = ExtractSummaryValueByPrefix(WheelSyncBuildSummaryText, TEXT("ApplyInCpp="));

			// Spin 소유 여부 문자열입니다.
			const FString SpinOwnedText = ExtractSummaryValueByPrefix(WheelSyncBuildSummaryText, TEXT("SpinOwned="));

			// Runtime spin 경로 사용 여부 문자열입니다.
			const FString RuntimeSpinText = ExtractSummaryValueByPrefix(WheelSyncBuildSummaryText, TEXT("RuntimeSpin="));

			RuntimeDetailLines.Add(FString::Printf(
				TEXT("WheelSyncBuild: Apply=%s | Spin=%s | RuntimeSpin=%s"),
				ApplyInCppText.IsEmpty() ? TEXT("?") : *ApplyInCppText,
				SpinOwnedText.IsEmpty() ? TEXT("?") : *SpinOwnedText,
				RuntimeSpinText.IsEmpty() ? TEXT("?") : *RuntimeSpinText));
			continue;
		}

		if (RuntimeSummaryPart.StartsWith(TEXT("WheelSyncRuntime=")))
		{
			// WheelSync 적용 단계 세부 문자열입니다.
			const FString WheelSyncRuntimeSummaryText = RuntimeSummaryPart.RightChop(17);

			// WheelSync 적용 성공 여부 문자열입니다.
			const FString SuccessText = ExtractSummaryValueByPrefix(WheelSyncRuntimeSummaryText, TEXT("Success="));

			// WheelSync transform 적용 여부 문자열입니다.
			const FString CppApplyText = ExtractSummaryValueByPrefix(WheelSyncRuntimeSummaryText, TEXT("CppApply="));

			RuntimeDetailLines.Add(FString::Printf(
				TEXT("WheelSyncRuntime: CppApply=%s | Success=%s"),
				CppApplyText.IsEmpty() ? TEXT("?") : *CppApplyText,
				SuccessText.IsEmpty() ? TEXT("?") : *SuccessText));
			continue;
		}

		// 현재 프레임에서 경고로 볼 만한 키워드가 포함된 Runtime 조각인지 여부입니다.
		const bool bLooksLikeWarningPart =
			RuntimeSummaryPart.Contains(TEXT("Missing")) ||
			RuntimeSummaryPart.Contains(TEXT("null")) ||
			RuntimeSummaryPart.Contains(TEXT("failed")) ||
			RuntimeSummaryPart.Contains(TEXT("disabled")) ||
			RuntimeSummaryPart.Contains(TEXT("Not"));

		if (bLooksLikeWarningPart)
		{
			RuntimeWarningLines.Add(RuntimeSummaryPart);
		}
	}

	// Runtime 핵심 상태가 비어 있으면 원본 전체를 폴백으로 사용합니다.
	if (RuntimeCoreLine.IsEmpty())
	{
		RuntimeCoreLine = InRuntimeSummaryText;
	}

	InOutDebugSummary += FString::Printf(TEXT("\nRuntime      : %s"), *RuntimeCoreLine);

	// Runtime 보조 상태를 순서대로 여러 줄로 붙입니다.
	for (const FString& RuntimeDetailLine : RuntimeDetailLines)
	{
		InOutDebugSummary += FString::Printf(TEXT("\nRuntime+     : %s"), *RuntimeDetailLine);
	}

	// Runtime 경고 상태를 순서대로 여러 줄로 붙입니다.
	for (const FString& RuntimeWarningLine : RuntimeWarningLines)
	{
		InOutDebugSummary += FString::Printf(TEXT("\nRuntime!     : %s"), *RuntimeWarningLine);
	}
}

// Transition 요약에서 필요한 항목만 추려 한 줄로 붙입니다.
static void AppendCompactTransitionSummaryLines(
	FString& InOutDebugSummary,
	const FString& InTransitionSummaryText)
{
	// 구분자 기준으로 분리한 Transition 요약 조각 배열입니다.
	TArray<FString> TransitionSummaryParts;
	InTransitionSummaryText.ParseIntoArray(TransitionSummaryParts, TEXT(" | "), true);

	if (TransitionSummaryParts.Num() <= 0)
	{
		return;
	}

	// Transition에서 유지할 접두사 목록입니다.
	const TArray<FString> AllowedTransitionPrefixes = {
		TEXT("Candidate="),
		TEXT("Hold="),
		TEXT("Count=")
	};

	// 최종 표시할 Transition 세부 문자열 배열입니다.
	TArray<FString> CompactTransitionParts;

	// 순서대로 Transition에서 유지할 세부 조각을 검사합니다.
	for (int32 TransitionSummaryPartIndex = 0; TransitionSummaryPartIndex < TransitionSummaryParts.Num(); ++TransitionSummaryPartIndex)
	{
		// 현재 줄에 출력할 Transition 세부 요약 문자열입니다.
		const FString& CurrentTransitionSummaryPart = TransitionSummaryParts[TransitionSummaryPartIndex];

		// 현재 세부 요약이 유지 대상인지 여부입니다.
		bool bIsAllowedTransitionPart = false;
		for (const FString& AllowedTransitionPrefix : AllowedTransitionPrefixes)
		{
			if (CurrentTransitionSummaryPart.StartsWith(AllowedTransitionPrefix))
			{
				bIsAllowedTransitionPart = true;
				break;
			}
		}

		if (!bIsAllowedTransitionPart)
		{
			continue;
		}

		CompactTransitionParts.Add(CurrentTransitionSummaryPart);
	}

	if (CompactTransitionParts.Num() > 0)
	{
		// 한 줄로 합친 Transition 세부 요약 문자열입니다.
		const FString CompactTransitionSummaryText = FString::Join(CompactTransitionParts, TEXT(" | "));
		InOutDebugSummary += FString::Printf(TEXT("\nTransition   : %s"), *CompactTransitionSummaryText);
	}
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
	// 상단에 한 줄로 출력할 핵심 차량 상태 요약 문자열입니다.
	const FString VehicleStateSummary = FString::Printf(
		TEXT("RuntimeReady=%s | DriveState=%s | PrevState=%s | Changed=%s | Speed=%.2f km/h | Forward=%.2f km/h"),
		InVehicleDebugSnapshot.bRuntimeReady ? TEXT("True") : TEXT("False"),
		*ConvDriveStateToDisplayString(InVehicleDebugSnapshot.CurrentDriveState),
		*ConvDriveStateToDisplayString(InVehicleDebugSnapshot.PreviousDriveState),
		InVehicleDebugSnapshot.bDriveStateChangedThisFrame ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.DriveStateSnapshot.CurrentSpeedKmh,
		InVehicleDebugSnapshot.DriveStateSnapshot.ForwardSpeedKmh);

	// 상단 두 번째 줄에 출력할 접지/컴포넌트 준비 상태 요약 문자열입니다.
	const FString VehicleComponentSummary = FString::Printf(
		TEXT("Grounded=%s | DriveComp=%s | WheelSync=%s"),
		InVehicleDebugSnapshot.DriveStateSnapshot.bIsGrounded ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.bHasDriveComponent ? TEXT("True") : TEXT("False"),
		InVehicleDebugSnapshot.bHasWheelSyncComponent ? TEXT("True") : TEXT("False"));

	FString VehicleDebugSummary = FString::Printf(
		TEXT("%s\n%s"),
		*VehicleStateSummary,
		*VehicleComponentSummary);

	if (bIncludeInputState)
	{
		VehicleDebugSummary += FString::Printf(
			TEXT("\nInput        : Throttle=%.2f | Steering=%.2f | Brake=%.2f | Handbrake=%s"),
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.ThrottleInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.SteeringInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.BrakeInput,
			InVehicleDebugSnapshot.DriveStateSnapshot.CurrentInputState.bHandbrakePressed ? TEXT("True") : TEXT("False"));
	}

	if (bIncludeTransitionSummary)
	{
		AppendCompactTransitionSummaryLines(
			VehicleDebugSummary,
			InVehicleDebugSnapshot.DriveStateTransitionSummary);
	}

	if (bIncludeRuntimeSummary)
	{
		AppendCompactRuntimeSummaryLines(
			VehicleDebugSummary,
			InVehicleDebugSnapshot.RuntimeSummary);
	}

	return VehicleDebugSummary;
}
