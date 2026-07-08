// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-06-25
// Description: Vehicle DA 입력 보조용 읽기 전용 검증 헬퍼 구현입니다.
// Scope: CFVehicleData 필수 참조, 소켓, 레이아웃, Movement, WheelVisual, DriveState, 기준 DA 비교 검증 리포트를 제공합니다.
// Changelog:
// - v1.2.0: HardpointSlots의 슬롯 ID, 선택 캡처 소켓, LocalTransform 상태 검증을 추가.
// - v1.1.0: ThrottleInputScale 검사와 기준 DA 비교 항목을 추가.
// - v1.0.0: EUW_VDAWizard 연동을 위한 BlueprintCallable 검증 함수와 결과 구조체를 추가.
// Migration:
// - 하드포인트 소켓 누락은 휠 소켓 누락과 달리 Warning으로만 보고 전체 오류로 승격하지 않는다.
// - 가속감 Quick Tune 적용 차량은 ThrottleInputScale 차이를 비교 리포트에서 확인한다.
// - 기존 VehicleData 적용 경로는 변경하지 않는다.
// - 에디터 위젯은 BP 내부 판단 대신 이 헬퍼의 리포트를 표시한다.

#include "CFVDAValidator.h"

#include "CFVehicleData.h"
#include "Containers/Set.h"
#include "Engine/StaticMesh.h"

namespace CFVDAValidatorInternal
{
	// 수치 비교에서 사실상 같은 값으로 볼 허용 오차입니다.
	constexpr float FloatCompareTolerance = 0.01f;

	// 휠 앵커 위치가 사실상 비어 있는지 판단할 허용 오차입니다.
	constexpr float AnchorZeroTolerance = 1.0f;

	// 표준 차체 휠 소켓 이름 목록입니다.
	const FName StandardSocketNames[4] =
	{
		FName(TEXT("Wheel_Anchor_FL")),
		FName(TEXT("Wheel_Anchor_FR")),
		FName(TEXT("Wheel_Anchor_RL")),
		FName(TEXT("Wheel_Anchor_RR"))
	};

	// 현재 심각도 우선순위 값을 반환합니다.
	int32 GetSeverityRank(const ECFVDASeverity Severity)
	{
		switch (Severity)
		{
		case ECFVDASeverity::Pass:
			return 0;
		case ECFVDASeverity::Info:
			return 1;
		case ECFVDASeverity::Warning:
			return 2;
		case ECFVDASeverity::Error:
			return 3;
		case ECFVDASeverity::Blocked:
			return 4;
		default:
			return 0;
		}
	}

	// 심각도 값을 UI 표시용 문자열로 변환합니다.
	FString GetSeverityText(const ECFVDASeverity Severity)
	{
		switch (Severity)
		{
		case ECFVDASeverity::Pass:
			return TEXT("정상");
		case ECFVDASeverity::Info:
			return TEXT("정보");
		case ECFVDASeverity::Warning:
			return TEXT("경고");
		case ECFVDASeverity::Error:
			return TEXT("오류");
		case ECFVDASeverity::Blocked:
			return TEXT("보류");
		default:
			return TEXT("알 수 없음");
		}
	}

	// 객체 이름을 안전하게 문자열로 변환합니다.
	FString GetObjectDisplayName(const UObject* ObjectValue)
	{
		return ObjectValue ? ObjectValue->GetName() : TEXT("None");
	}

	// bool 값을 한글 표시 문자열로 변환합니다.
	FString GetBoolDisplayText(const bool bValue)
	{
		return bValue ? TEXT("예") : TEXT("아니오");
	}

	// float 값을 비교용 짧은 문자열로 변환합니다.
	FString GetFloatDisplayText(const float Value)
	{
		return FString::Printf(TEXT("%.2f"), Value);
	}

	// FVector 값을 비교용 짧은 문자열로 변환합니다.
	FString GetVectorDisplayText(const FVector& Value)
	{
		return FString::Printf(TEXT("(X=%.2f,Y=%.2f,Z=%.2f)"), Value.X, Value.Y, Value.Z);
	}

	// FName 값을 비교용 문자열로 변환합니다.
	FString GetNameDisplayText(const FName Value)
	{
		return Value.IsNone() ? TEXT("None") : Value.ToString();
	}

	// 검증 리포트의 대상/기준 DA 이름을 채웁니다.
	void InitializeReport(FCFVDAValidationReport& OutReport, const UCFVehicleData* TargetVehicleData, const UCFVehicleData* SourceVehicleData)
	{
		OutReport.TargetDAName = GetObjectDisplayName(TargetVehicleData);
		OutReport.SourceDAName = GetObjectDisplayName(SourceVehicleData);
	}

	// 리포트에 세부 검사 항목을 추가합니다.
	void AddItem(
		FCFVDAValidationReport& InOutReport,
		const ECFVDASeverity Severity,
		const FName GroupName,
		const FString& FieldPath,
		const FString& DisplayName,
		const FString& Message,
		const FString& RecommendedAction)
	{
		// 새로 추가할 세부 검사 항목입니다.
		FCFVDAValidationItem NewItem;
		NewItem.Severity = Severity;
		NewItem.GroupName = GroupName;
		NewItem.FieldPath = FieldPath;
		NewItem.DisplayName = FText::FromString(DisplayName);
		NewItem.Message = FText::FromString(Message);
		NewItem.RecommendedAction = FText::FromString(RecommendedAction);

		InOutReport.Items.Add(NewItem);
	}

	// 다른 리포트의 세부 항목을 현재 리포트에 병합합니다.
	void AppendReportItems(FCFVDAValidationReport& InOutReport, const FCFVDAValidationReport& SourceReport)
	{
		for (const FCFVDAValidationItem& SourceItem : SourceReport.Items)
		{
			InOutReport.Items.Add(SourceItem);
		}
	}

	// 리포트 항목을 집계해 전체 심각도와 요약 문자열을 갱신합니다.
	void FinalizeReport(FCFVDAValidationReport& InOutReport)
	{
		// 리포트 전체에서 가장 높은 심각도입니다.
		ECFVDASeverity HighestSeverity = ECFVDASeverity::Pass;

		InOutReport.ErrorCount = 0;
		InOutReport.WarningCount = 0;
		InOutReport.InfoCount = 0;
		InOutReport.BlockedCount = 0;

		for (const FCFVDAValidationItem& Item : InOutReport.Items)
		{
			if (GetSeverityRank(Item.Severity) > GetSeverityRank(HighestSeverity))
			{
				HighestSeverity = Item.Severity;
			}

			if (Item.Severity == ECFVDASeverity::Error)
			{
				++InOutReport.ErrorCount;
			}
			else if (Item.Severity == ECFVDASeverity::Warning)
			{
				++InOutReport.WarningCount;
			}
			else if (Item.Severity == ECFVDASeverity::Info)
			{
				++InOutReport.InfoCount;
			}
			else if (Item.Severity == ECFVDASeverity::Blocked)
			{
				++InOutReport.BlockedCount;
			}
		}

		InOutReport.OverallSeverity = HighestSeverity;
		InOutReport.SummaryText = FString::Printf(
			TEXT("VehicleDAValidation: Target=%s, Source=%s, Overall=%s, Error=%d, Warning=%d, Info=%d, Blocked=%d"),
			*InOutReport.TargetDAName,
			*InOutReport.SourceDAName,
			*GetSeverityText(InOutReport.OverallSeverity),
			InOutReport.ErrorCount,
			InOutReport.WarningCount,
			InOutReport.InfoCount,
			InOutReport.BlockedCount);
	}

	// Config에 지정된 차체 소켓 이름을 표준 기본값과 함께 해석합니다.
	FName ResolveSocketName(const FName ConfiguredSocketName, const FName DefaultSocketName)
	{
		return ConfiguredSocketName.IsNone() ? DefaultSocketName : ConfiguredSocketName;
	}

	// 휠 앵커 포즈의 위치가 사실상 비어 있는지 판정합니다.
	bool IsAnchorPoseLocationNearlyZero(const FCFWheelAnchorPose& WheelAnchorPose)
	{
		return WheelAnchorPose.RelativeLocation.IsNearlyZero(AnchorZeroTolerance);
	}

	// 네 휠 앵커 위치가 모두 비어 있는지 판정합니다.
	bool AreAllAnchorLocationsNearlyZero(const FCFVehicleLayoutConfig& LayoutConfig)
	{
		return IsAnchorPoseLocationNearlyZero(LayoutConfig.WheelAnchorFL)
			&& IsAnchorPoseLocationNearlyZero(LayoutConfig.WheelAnchorFR)
			&& IsAnchorPoseLocationNearlyZero(LayoutConfig.WheelAnchorRL)
			&& IsAnchorPoseLocationNearlyZero(LayoutConfig.WheelAnchorRR);
	}

	// 표준 소켓 이름 사용 여부를 판정합니다.
	bool IsStandardSocketName(const FName SocketName)
	{
		for (const FName& StandardSocketName : StandardSocketNames)
		{
			if (SocketName == StandardSocketName)
			{
				return true;
			}
		}

		return false;
	}

	// 하드포인트 소켓 이름이 HP_ prefix 표준을 따르는지 판정합니다.
	bool IsHardpointSocketNameUsingPrefix(const FName SocketName)
	{
		if (SocketName.IsNone())
		{
			return true;
		}

		return SocketName.ToString().StartsWith(TEXT("HP_"));
	}

	// 하드포인트 위치 값이 사실상 비어 있는지 판정합니다.
	bool IsHardpointLocationNearlyZero(const FCFVehicleHardpointSlot& HardpointSlot)
	{
		return HardpointSlot.LocalLocation.IsNearlyZero(AnchorZeroTolerance);
	}

	// Target DA가 없을 때 공통 오류 리포트를 생성합니다.
	FCFVDAValidationReport MakeMissingTargetReport()
	{
		// 대상 없음 오류를 담을 리포트입니다.
		FCFVDAValidationReport Report;
		InitializeReport(Report, nullptr, nullptr);
		AddItem(
			Report,
			ECFVDASeverity::Error,
			FName(TEXT("Target")),
			TEXT("TargetVehicleData"),
			TEXT("대상 차량 DA"),
			TEXT("대상 차량 DA가 없습니다."),
			TEXT("EUW_VDAWizard에서 검사할 대상 CFVehicleData 자산을 선택하세요."));
		FinalizeReport(Report);
		return Report;
	}

	// float 필드가 기준 DA와 다른 경우 비교 결과를 추가합니다.
	void AddFloatCompareItem(
		FCFVDAValidationReport& InOutReport,
		const FName GroupName,
		const FString& FieldPath,
		const FString& DisplayName,
		const float TargetValue,
		const float SourceValue,
		const float WarningDelta,
		const FString& RecommendedAction)
	{
		// 기준값과 대상값의 절대 차이입니다.
		const float DeltaValue = FMath::Abs(TargetValue - SourceValue);
		if (DeltaValue <= FloatCompareTolerance)
		{
			return;
		}

		// 차이가 큰지 여부에 따른 심각도입니다.
		const ECFVDASeverity Severity = DeltaValue >= WarningDelta ? ECFVDASeverity::Warning : ECFVDASeverity::Info;

		// 사용자에게 보여줄 비교 메시지입니다.
		const FString Message = FString::Printf(
			TEXT("기준값 %s, 대상값 %s, 차이 %s입니다."),
			*GetFloatDisplayText(SourceValue),
			*GetFloatDisplayText(TargetValue),
			*GetFloatDisplayText(TargetValue - SourceValue));

		AddItem(InOutReport, Severity, GroupName, FieldPath, DisplayName, Message, RecommendedAction);
	}

	// bool 필드가 기준 DA와 다른 경우 비교 결과를 추가합니다.
	void AddBoolCompareItem(
		FCFVDAValidationReport& InOutReport,
		const FName GroupName,
		const FString& FieldPath,
		const FString& DisplayName,
		const bool bTargetValue,
		const bool bSourceValue,
		const ECFVDASeverity Severity,
		const FString& RecommendedAction)
	{
		if (bTargetValue == bSourceValue)
		{
			return;
		}

		// 사용자에게 보여줄 비교 메시지입니다.
		const FString Message = FString::Printf(
			TEXT("기준값 %s, 대상값 %s입니다."),
			*GetBoolDisplayText(bSourceValue),
			*GetBoolDisplayText(bTargetValue));

		AddItem(InOutReport, Severity, GroupName, FieldPath, DisplayName, Message, RecommendedAction);
	}

	// FName 필드가 기준 DA와 다른 경우 비교 결과를 추가합니다.
	void AddNameCompareItem(
		FCFVDAValidationReport& InOutReport,
		const FName GroupName,
		const FString& FieldPath,
		const FString& DisplayName,
		const FName TargetValue,
		const FName SourceValue,
		const FString& RecommendedAction)
	{
		if (TargetValue == SourceValue)
		{
			return;
		}

		// 사용자에게 보여줄 비교 메시지입니다.
		const FString Message = FString::Printf(
			TEXT("기준값 %s, 대상값 %s입니다."),
			*GetNameDisplayText(SourceValue),
			*GetNameDisplayText(TargetValue));

		AddItem(InOutReport, ECFVDASeverity::Info, GroupName, FieldPath, DisplayName, Message, RecommendedAction);
	}
}

// 대상 VehicleData를 전체 검사하고 기준 VehicleData가 있으면 주요 차이도 함께 비교합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateVehicleData(UCFVehicleData* TargetVehicleData, UCFVehicleData* SourceVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// 전체 검사 결과를 모을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, SourceVehicleData);

	CFVDAValidatorInternal::AppendReportItems(Report, ValidateRequiredReferences(TargetVehicleData));
	CFVDAValidatorInternal::AppendReportItems(Report, ValidateWheelSockets(TargetVehicleData));
	CFVDAValidatorInternal::AppendReportItems(Report, ValidateLayoutConfig(TargetVehicleData));
	CFVDAValidatorInternal::AppendReportItems(Report, ValidateHardpointSlots(TargetVehicleData));
	CFVDAValidatorInternal::AppendReportItems(Report, ValidateMovementConfig(TargetVehicleData));
	CFVDAValidatorInternal::AppendReportItems(Report, ValidateWheelVisualConfig(TargetVehicleData));
	CFVDAValidatorInternal::AppendReportItems(Report, ValidateDriveStateConfig(TargetVehicleData));

	if (SourceVehicleData)
	{
		CFVDAValidatorInternal::AppendReportItems(Report, CompareVehicleData(TargetVehicleData, SourceVehicleData));
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 필수 메쉬와 Wheel Class 참조 누락 여부를 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateRequiredReferences(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// 필수 참조 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 대상 DA의 시각 자산 참조 묶음입니다.
	const FCFVehicleVisualConfig& VisualConfig = TargetVehicleData->VehicleVisualConfig;

	// 대상 DA의 Wheel Class 참조 묶음입니다.
	const FCFVehicleReferenceConfig& ReferenceConfig = TargetVehicleData->VehicleReferenceConfig;

	if (!VisualConfig.ChassisMesh)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("RequiredRefs")), TEXT("VehicleVisualConfig.ChassisMesh"), TEXT("차체 메쉬"), TEXT("차체 메쉬가 비어 있습니다."), TEXT("새 차량 차체 Static Mesh를 ChassisMesh에 지정하세요."));
	}

	if (!VisualConfig.WheelMeshFL)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("RequiredRefs")), TEXT("VehicleVisualConfig.WheelMeshFL"), TEXT("앞왼쪽 휠 메쉬"), TEXT("앞왼쪽 휠 메쉬가 비어 있습니다."), TEXT("최소 하나의 기준 휠 메쉬를 WheelMeshFL에 지정하세요."));
	}

	if (!VisualConfig.WheelMeshFR)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("RequiredRefs")), TEXT("VehicleVisualConfig.WheelMeshFR"), TEXT("앞오른쪽 휠 메쉬"), TEXT("앞오른쪽 휠 메쉬가 비어 있습니다."), TEXT("임시로 FL 휠을 재사용하는 의도인지 확인하고 기록하세요."));
	}

	if (!VisualConfig.WheelMeshRL)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("RequiredRefs")), TEXT("VehicleVisualConfig.WheelMeshRL"), TEXT("뒤왼쪽 휠 메쉬"), TEXT("뒤왼쪽 휠 메쉬가 비어 있습니다."), TEXT("임시로 FL 휠을 재사용하는 의도인지 확인하고 기록하세요."));
	}

	if (!VisualConfig.WheelMeshRR)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("RequiredRefs")), TEXT("VehicleVisualConfig.WheelMeshRR"), TEXT("뒤오른쪽 휠 메쉬"), TEXT("뒤오른쪽 휠 메쉬가 비어 있습니다."), TEXT("임시로 FL 휠을 재사용하는 의도인지 확인하고 기록하세요."));
	}

	if (!ReferenceConfig.FrontWheelClass)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("RequiredRefs")), TEXT("VehicleReferenceConfig.FrontWheelClass"), TEXT("전륜 Wheel Class"), TEXT("전륜 Wheel Class가 비어 있습니다."), TEXT("전륜에 사용할 BP_Wheel_Front 계열 Wheel Class를 지정하세요."));
	}

	if (!ReferenceConfig.RearWheelClass)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("RequiredRefs")), TEXT("VehicleReferenceConfig.RearWheelClass"), TEXT("후륜 Wheel Class"), TEXT("후륜 Wheel Class가 비어 있습니다."), TEXT("후륜에 사용할 BP_Wheel_Rear 계열 Wheel Class를 지정하세요."));
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 차체 Static Mesh에 필요한 휠 앵커 소켓이 있는지 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateWheelSockets(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// 소켓 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 소켓을 검사할 차체 Static Mesh입니다.
	const UStaticMesh* ChassisMesh = TargetVehicleData->VehicleVisualConfig.ChassisMesh;
	if (!ChassisMesh)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Socket")), TEXT("VehicleVisualConfig.ChassisMesh"), TEXT("차체 메쉬"), TEXT("차체 메쉬가 없어 소켓 검사를 할 수 없습니다."), TEXT("먼저 ChassisMesh를 지정한 뒤 소켓 검사를 다시 실행하세요."));
		CFVDAValidatorInternal::FinalizeReport(Report);
		return Report;
	}

	// 대상 DA의 레이아웃 설정입니다.
	const FCFVehicleLayoutConfig& LayoutConfig = TargetVehicleData->VehicleLayoutConfig;

	// 앞왼쪽 휠 앵커 소켓 이름입니다.
	const FName SocketFL = CFVDAValidatorInternal::ResolveSocketName(LayoutConfig.BodyWheelSocketFL, FName(TEXT("Wheel_Anchor_FL")));

	// 앞오른쪽 휠 앵커 소켓 이름입니다.
	const FName SocketFR = CFVDAValidatorInternal::ResolveSocketName(LayoutConfig.BodyWheelSocketFR, FName(TEXT("Wheel_Anchor_FR")));

	// 뒤왼쪽 휠 앵커 소켓 이름입니다.
	const FName SocketRL = CFVDAValidatorInternal::ResolveSocketName(LayoutConfig.BodyWheelSocketRL, FName(TEXT("Wheel_Anchor_RL")));

	// 뒤오른쪽 휠 앵커 소켓 이름입니다.
	const FName SocketRR = CFVDAValidatorInternal::ResolveSocketName(LayoutConfig.BodyWheelSocketRR, FName(TEXT("Wheel_Anchor_RR")));

	// 검사할 소켓 이름 배열입니다.
	const FName SocketNames[4] = { SocketFL, SocketFR, SocketRL, SocketRR };

	// 소켓 필드 경로 배열입니다.
	const FString SocketFieldPaths[4] =
	{
		TEXT("VehicleLayoutConfig.BodyWheelSocketFL"),
		TEXT("VehicleLayoutConfig.BodyWheelSocketFR"),
		TEXT("VehicleLayoutConfig.BodyWheelSocketRL"),
		TEXT("VehicleLayoutConfig.BodyWheelSocketRR")
	};

	// 소켓 표시 이름 배열입니다.
	const FString SocketDisplayNames[4] =
	{
		TEXT("앞왼쪽 차체 소켓"),
		TEXT("앞오른쪽 차체 소켓"),
		TEXT("뒤왼쪽 차체 소켓"),
		TEXT("뒤오른쪽 차체 소켓")
	};

	for (int32 SocketIndex = 0; SocketIndex < 4; ++SocketIndex)
	{
		// 현재 검사 중인 소켓 이름입니다.
		const FName SocketName = SocketNames[SocketIndex];

		if (!ChassisMesh->FindSocket(SocketName))
		{
			// 소켓 누락 메시지입니다.
			const FString Message = FString::Printf(TEXT("차체 메쉬에 %s 소켓이 없습니다."), *SocketName.ToString());
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Socket")), SocketFieldPaths[SocketIndex], SocketDisplayNames[SocketIndex], Message, TEXT("Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 휠 앵커 소켓을 추가하거나 DA의 소켓 이름을 실제 이름으로 수정하세요."));
		}
		else if (!CFVDAValidatorInternal::IsStandardSocketName(SocketName))
		{
			// 커스텀 소켓 이름 안내 메시지입니다.
			const FString Message = FString::Printf(TEXT("표준 Wheel_Anchor 이름 대신 %s 소켓을 사용 중입니다."), *SocketName.ToString());
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Info, FName(TEXT("Socket")), SocketFieldPaths[SocketIndex], SocketDisplayNames[SocketIndex], Message, TEXT("의도한 예외 소켓 이름이면 작업 메모에 기록하세요."));
		}
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 휠 레이아웃 덮어쓰기와 앵커 값 상태를 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateLayoutConfig(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// 레이아웃 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 대상 DA의 레이아웃 설정입니다.
	const FCFVehicleLayoutConfig& LayoutConfig = TargetVehicleData->VehicleLayoutConfig;

	if (!LayoutConfig.bUseLayoutOverrides)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Layout")), TEXT("VehicleLayoutConfig.bUseLayoutOverrides"), TEXT("레이아웃 덮어쓰기 사용"), TEXT("DA 레이아웃 덮어쓰기가 꺼져 있습니다."), TEXT("새 차량은 차체 소켓에서 차량 레이아웃 캡처를 실행해 bUseLayoutOverrides를 켜는 것을 권장합니다."));
	}

	if (LayoutConfig.bUseLayoutOverrides && CFVDAValidatorInternal::AreAllAnchorLocationsNearlyZero(LayoutConfig))
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Layout")), TEXT("VehicleLayoutConfig.WheelAnchor*"), TEXT("휠 앵커 값"), TEXT("레이아웃 덮어쓰기가 켜졌지만 네 휠 앵커 위치가 모두 0에 가깝습니다."), TEXT("차체 소켓에서 차량 레이아웃 캡처를 다시 실행하세요."));
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 하드포인트 위치 슬롯과 선택 캡처 소켓 상태를 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateHardpointSlots(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// 하드포인트 슬롯 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 대상 DA의 하드포인트 위치 슬롯 목록입니다.
	const TArray<FCFVehicleHardpointSlot>& HardpointSlots = TargetVehicleData->HardpointSlots;
	if (HardpointSlots.IsEmpty())
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Info, FName(TEXT("Hardpoint")), TEXT("HardpointSlots"), TEXT("하드포인트 위치 슬롯"), TEXT("하드포인트 위치 슬롯 배열이 비어 있습니다."), TEXT("이 차량이 무기 위치를 제공해야 한다면 Front_01, Top_01 같은 위치 슬롯을 추가하세요. 빈 배열 자체는 오류가 아닙니다."));
		CFVDAValidatorInternal::FinalizeReport(Report);
		return Report;
	}

	// 선택 캡처 소켓을 검사할 차체 Static Mesh입니다.
	const UStaticMesh* ChassisMesh = TargetVehicleData->VehicleVisualConfig.ChassisMesh;

	// 중복 LocationSlotId를 찾기 위해 이미 등장한 슬롯 ID를 저장합니다.
	TSet<FName> SeenLocationSlotIds;

	// 현재 검사 중인 하드포인트 슬롯 인덱스입니다.
	for (int32 HardpointIndex = 0; HardpointIndex < HardpointSlots.Num(); ++HardpointIndex)
	{
		// 현재 검사 중인 하드포인트 슬롯입니다.
		const FCFVehicleHardpointSlot& HardpointSlot = HardpointSlots[HardpointIndex];

		// 현재 슬롯의 리포트 필드 경로 접두사입니다.
		const FString FieldPathPrefix = FString::Printf(TEXT("HardpointSlots[%d]"), HardpointIndex);

		// 현재 슬롯을 UI에 표시할 이름입니다.
		const FString SlotDisplayName = HardpointSlot.LocationSlotId.IsNone()
			? FString::Printf(TEXT("하드포인트 슬롯 %d"), HardpointIndex)
			: FString::Printf(TEXT("하드포인트 슬롯 %s"), *HardpointSlot.LocationSlotId.ToString());

		if (HardpointSlot.LocationSlotId.IsNone())
		{
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Hardpoint")), FieldPathPrefix + TEXT(".LocationSlotId"), SlotDisplayName, TEXT("LocationSlotId가 비어 있어 전투 규칙에서 이 위치를 안정적으로 참조할 수 없습니다."), TEXT("Front_01, Top_01처럼 위치 슬롯 인스턴스 ID를 입력하세요."));
		}
		else if (SeenLocationSlotIds.Contains(HardpointSlot.LocationSlotId))
		{
			// 중복된 슬롯 ID 안내 메시지입니다.
			const FString Message = FString::Printf(TEXT("LocationSlotId %s가 중복됩니다."), *HardpointSlot.LocationSlotId.ToString());
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Hardpoint")), FieldPathPrefix + TEXT(".LocationSlotId"), SlotDisplayName, Message, TEXT("각 하드포인트 위치 슬롯은 고유한 LocationSlotId를 사용하세요."));
		}
		else
		{
			SeenLocationSlotIds.Add(HardpointSlot.LocationSlotId);
		}

		if (!CFVDAValidatorInternal::IsHardpointSocketNameUsingPrefix(HardpointSlot.SocketName))
		{
			// 비표준 하드포인트 소켓 이름 안내 메시지입니다.
			const FString Message = FString::Printf(TEXT("하드포인트 캡처 소켓 %s가 HP_ prefix 표준을 따르지 않습니다."), *HardpointSlot.SocketName.ToString());
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Hardpoint")), FieldPathPrefix + TEXT(".SocketName"), SlotDisplayName, Message, TEXT("Static Mesh Socket 이름을 HP_Front_01, HP_Top_01처럼 위치 슬롯 중심 이름으로 맞추세요."));
		}

		if (!HardpointSlot.SocketName.IsNone() && ChassisMesh && !ChassisMesh->FindSocket(HardpointSlot.SocketName))
		{
			// 누락된 선택 캡처 소켓 안내 메시지입니다.
			const FString Message = FString::Printf(TEXT("차체 메쉬에 하드포인트 캡처 소켓 %s가 없습니다."), *HardpointSlot.SocketName.ToString());
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Hardpoint")), FieldPathPrefix + TEXT(".SocketName"), SlotDisplayName, Message, TEXT("해당 슬롯을 소켓으로 캡처하려면 Static Mesh 소켓을 추가하세요. 직접 입력 슬롯이면 SocketName을 비워도 됩니다."));
		}

		if (!HardpointSlot.LocationSlotId.IsNone() && CFVDAValidatorInternal::IsHardpointLocationNearlyZero(HardpointSlot))
		{
			CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Hardpoint")), FieldPathPrefix + TEXT(".LocalLocation"), SlotDisplayName, TEXT("하드포인트 LocalLocation이 0에 가깝습니다."), TEXT("차체 소켓에서 차량 레이아웃 캡처를 실행하거나 차량별 발사 위치를 직접 입력하세요."));
		}
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 Movement 수치 기본 범위와 위험값을 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateMovementConfig(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// Movement 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 대상 DA의 Movement 설정입니다.
	const FCFVehicleMovementConfig& MovementConfig = TargetVehicleData->VehicleMovementConfig;

	if (!MovementConfig.bUseMovementOverrides)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.bUseMovementOverrides"), TEXT("VehicleMovement 설정 사용"), TEXT("VehicleMovement 덮어쓰기가 꺼져 있습니다."), TEXT("새 차량 DA가 Movement 값을 적용해야 한다면 bUseMovementOverrides를 켜세요."));
	}

	if (MovementConfig.bUseMovementOverrides && MovementConfig.FrontWheelRadius <= 0.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.FrontWheelRadius"), TEXT("전륜 휠 반지름"), TEXT("전륜 휠 반지름은 0보다 커야 합니다."), TEXT("차량 휠 크기에 맞는 전륜 반지름(cm)을 입력하세요."));
	}

	if (MovementConfig.bUseMovementOverrides && MovementConfig.RearWheelRadius <= 0.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.RearWheelRadius"), TEXT("후륜 휠 반지름"), TEXT("후륜 휠 반지름은 0보다 커야 합니다."), TEXT("차량 휠 크기에 맞는 후륜 반지름(cm)을 입력하세요."));
	}

	if (MovementConfig.bUseMovementOverrides && MovementConfig.FrontWheelWidth <= 0.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.FrontWheelWidth"), TEXT("전륜 휠 폭"), TEXT("전륜 휠 폭은 0보다 커야 합니다."), TEXT("차량 휠 크기에 맞는 전륜 폭(cm)을 입력하세요."));
	}

	if (MovementConfig.bUseMovementOverrides && MovementConfig.RearWheelWidth <= 0.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.RearWheelWidth"), TEXT("후륜 휠 폭"), TEXT("후륜 휠 폭은 0보다 커야 합니다."), TEXT("차량 휠 크기에 맞는 후륜 폭(cm)을 입력하세요."));
	}

	if (MovementConfig.EngineMaxRPM <= MovementConfig.EngineIdleRPM)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.EngineMaxRPM"), TEXT("엔진 최대 RPM"), TEXT("엔진 최대 RPM은 엔진 아이들 RPM보다 커야 합니다."), TEXT("EngineMaxRPM을 EngineIdleRPM보다 큰 값으로 수정하세요."));
	}

	if (MovementConfig.ThrottleInputScale <= 0.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.ThrottleInputScale"), TEXT("스로틀 입력 배율"), TEXT("스로틀 입력 배율이 0 이하라 차량이 전진하지 않을 수 있습니다."), TEXT("가속감 Quick Tune을 다시 적용하거나 ThrottleInputScale을 0.2~1.0 사이로 조정하세요."));
	}

	if (MovementConfig.FrontWheelMaxSteerAngle > 50.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"), TEXT("전륜 최대 조향각"), TEXT("전륜 최대 조향각이 큽니다. 고속 불안정이 생길 수 있습니다."), TEXT("저속/중속/고속 조향 테스트를 나눠서 확인하세요."));
	}

	if (MovementConfig.bEnableCenterOfMassOverride)
	{
		// 중심질량 오버라이드 현재값을 포함한 메시지입니다.
		const FString Message = FString::Printf(TEXT("중심질량 오버라이드가 켜져 있습니다. 현재 값은 %s입니다."), *CFVDAValidatorInternal::GetVectorDisplayText(MovementConfig.CenterOfMassOverride));
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Movement")), TEXT("VehicleMovementConfig.CenterOfMassOverride"), TEXT("중심질량 오프셋"), Message, TEXT("스폰/조향/브레이크 기본 검증을 통과한 뒤 마지막 단계에서만 조정하세요."));
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 WheelVisual 예상 휠 개수와 조향 전륜 개수를 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateWheelVisualConfig(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// WheelVisual 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 대상 DA의 WheelVisual 설정입니다.
	const FCFVehicleWheelVisualConfig& WheelVisualConfig = TargetVehicleData->WheelVisualConfig;

	if (!WheelVisualConfig.bUseWheelVisualOverrides)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("WheelVisual")), TEXT("WheelVisualConfig.bUseWheelVisualOverrides"), TEXT("WheelVisual 덮어쓰기 사용"), TEXT("WheelVisual 덮어쓰기가 꺼져 있습니다."), TEXT("새 차량이 별도 휠 시각 설정을 사용해야 한다면 bUseWheelVisualOverrides를 켜세요."));
	}

	if (WheelVisualConfig.bUseWheelVisualOverrides && WheelVisualConfig.ExpectedWheelCount <= 0)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("WheelVisual")), TEXT("WheelVisualConfig.ExpectedWheelCount"), TEXT("예상 휠 개수"), TEXT("예상 휠 개수는 1 이상이어야 합니다."), TEXT("4륜 기본 차량이면 ExpectedWheelCount를 4로 설정하세요."));
	}

	if (WheelVisualConfig.FrontWheelCountForSteering > WheelVisualConfig.ExpectedWheelCount)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("WheelVisual")), TEXT("WheelVisualConfig.FrontWheelCountForSteering"), TEXT("조향 전륜 개수"), TEXT("조향 전륜 개수가 예상 휠 개수보다 큽니다."), TEXT("4륜 전륜 조향 차량이면 FrontWheelCountForSteering을 2로 설정하세요."));
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 대상 VehicleData의 DriveState 판정 임계값과 유지 시간 기본 범위를 검사합니다.
FCFVDAValidationReport UCFVDAValidator::ValidateDriveStateConfig(UCFVehicleData* TargetVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// DriveState 검사 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, nullptr);

	// 대상 DA의 DriveState 설정입니다.
	const FCFVehicleDriveStateConfig& DriveStateConfig = TargetVehicleData->DriveStateConfig;

	if (!DriveStateConfig.bUseDriveStateOverrides)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("DriveState")), TEXT("DriveStateConfig.bUseDriveStateOverrides"), TEXT("DriveState 덮어쓰기 사용"), TEXT("DriveState 덮어쓰기가 꺼져 있습니다."), TEXT("차량별 DriveState 판정값을 적용하려면 bUseDriveStateOverrides를 켜세요."));
	}

	if (DriveStateConfig.ActiveInputThreshold < 0.0f || DriveStateConfig.ActiveInputThreshold > 1.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("DriveState")), TEXT("DriveStateConfig.ActiveInputThreshold"), TEXT("유효 입력 임계값"), TEXT("ActiveInputThreshold는 0~1 범위여야 합니다."), TEXT("입력 임계값을 0.0 이상 1.0 이하로 수정하세요."));
	}

	if (DriveStateConfig.DriveStateMinimumHoldTimeSeconds < 0.0f || DriveStateConfig.IdleStateMinimumHoldTimeSeconds < 0.0f || DriveStateConfig.ReversingStateMinimumHoldTimeSeconds < 0.0f || DriveStateConfig.AirborneStateMinimumHoldTimeSeconds < 0.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Error, FName(TEXT("DriveState")), TEXT("DriveStateConfig.*HoldTimeSeconds"), TEXT("DriveState 최소 유지 시간"), TEXT("DriveState 최소 유지 시간은 음수가 될 수 없습니다."), TEXT("모든 DriveState 유지 시간을 0 이상으로 수정하세요."));
	}

	if (DriveStateConfig.IdleExitSpeedThresholdKmh < DriveStateConfig.IdleEnterSpeedThresholdKmh)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("DriveState")), TEXT("DriveStateConfig.IdleExitSpeedThresholdKmh"), TEXT("Idle 이탈 속도 임계값"), TEXT("Idle 이탈 속도 임계값이 진입 임계값보다 낮습니다."), TEXT("Idle 상태 튐이 생기는지 PIE에서 정지/저속 이동을 확인하세요."));
	}

	if (DriveStateConfig.ReverseEnterSpeedThresholdKmh < DriveStateConfig.ReverseExitSpeedThresholdKmh)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("DriveState")), TEXT("DriveStateConfig.ReverseEnterSpeedThresholdKmh"), TEXT("Reversing 진입 속도 임계값"), TEXT("Reversing 진입 임계값이 이탈 임계값보다 낮습니다."), TEXT("후진 진입/이탈 상태가 튀는지 PIE에서 확인하세요."));
	}

	if (DriveStateConfig.AirborneMinSpeedThresholdKmh <= 0.1f || DriveStateConfig.AirborneVerticalSpeedThresholdCmPerSec <= 1.0f)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("DriveState")), TEXT("DriveStateConfig.Airborne*Threshold"), TEXT("Airborne 임계값"), TEXT("Airborne 임계값이 매우 낮습니다. 공중 상태가 과하게 빨리 들어갈 수 있습니다."), TEXT("작은 턱과 낙하 상황에서 Airborne 판정이 과한지 확인하세요."));
	}

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}

// 기준 VehicleData와 대상 VehicleData의 주요 튜닝값 차이를 비교합니다.
FCFVDAValidationReport UCFVDAValidator::CompareVehicleData(UCFVehicleData* TargetVehicleData, UCFVehicleData* SourceVehicleData)
{
	if (!TargetVehicleData)
	{
		return CFVDAValidatorInternal::MakeMissingTargetReport();
	}

	// 기준 DA 비교 결과를 담을 리포트입니다.
	FCFVDAValidationReport Report;
	CFVDAValidatorInternal::InitializeReport(Report, TargetVehicleData, SourceVehicleData);

	if (!SourceVehicleData)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Blocked, FName(TEXT("Compare")), TEXT("SourceVehicleData"), TEXT("기준 차량 DA"), TEXT("기준 차량 DA가 없어 비교를 건너뜁니다."), TEXT("기준값 비교가 필요하면 Source DA를 선택하세요."));
		CFVDAValidatorInternal::FinalizeReport(Report);
		return Report;
	}

	if (TargetVehicleData == SourceVehicleData)
	{
		CFVDAValidatorInternal::AddItem(Report, ECFVDASeverity::Warning, FName(TEXT("Compare")), TEXT("TargetVehicleData"), TEXT("대상 차량 DA"), TEXT("기준 DA와 대상 DA가 같습니다."), TEXT("새 차량 DA를 검사하려면 기준 DA를 복제한 별도 대상 DA를 선택하세요."));
		CFVDAValidatorInternal::FinalizeReport(Report);
		return Report;
	}

	// 대상 DA의 Movement 설정입니다.
	const FCFVehicleMovementConfig& TargetMovementConfig = TargetVehicleData->VehicleMovementConfig;

	// 기준 DA의 Movement 설정입니다.
	const FCFVehicleMovementConfig& SourceMovementConfig = SourceVehicleData->VehicleMovementConfig;

	CFVDAValidatorInternal::AddNameCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.MovementProfileName"), TEXT("이동 프로필 이름"), TargetMovementConfig.MovementProfileName, SourceMovementConfig.MovementProfileName, TEXT("프로필 이름이 의도한 새 차량 구분자인지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.EngineMaxTorque"), TEXT("엔진 최대 토크"), TargetMovementConfig.EngineMaxTorque, SourceMovementConfig.EngineMaxTorque, 150.0f, TEXT("직선 가속 5초와 출발 휠스핀을 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.EngineMaxRPM"), TEXT("엔진 최대 RPM"), TargetMovementConfig.EngineMaxRPM, SourceMovementConfig.EngineMaxRPM, 1500.0f, TEXT("고속 성격 변경이 의도한 것인지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.ThrottleInputScale"), TEXT("스로틀 입력 배율"), TargetMovementConfig.ThrottleInputScale, SourceMovementConfig.ThrottleInputScale, 0.15f, TEXT("가속감 0%/100% 비교 시 0~30km/h 도달 시간을 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"), TEXT("전륜 최대 조향각"), TargetMovementConfig.FrontWheelMaxSteerAngle, SourceMovementConfig.FrontWheelMaxSteerAngle, 10.0f, TEXT("저속/중속/고속 조향 테스트를 나눠서 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.FrontWheelRadius"), TEXT("전륜 휠 반지름"), TargetMovementConfig.FrontWheelRadius, SourceMovementConfig.FrontWheelRadius, 8.0f, TEXT("실제 휠 메쉬 크기와 반지름 값이 맞는지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.RearWheelRadius"), TEXT("후륜 휠 반지름"), TargetMovementConfig.RearWheelRadius, SourceMovementConfig.RearWheelRadius, 8.0f, TEXT("실제 휠 메쉬 크기와 반지름 값이 맞는지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.FrontWheelFrictionForceMultiplier"), TEXT("전륜 마찰력 배수"), TargetMovementConfig.FrontWheelFrictionForceMultiplier, SourceMovementConfig.FrontWheelFrictionForceMultiplier, 0.75f, TEXT("좌우 슬라럼과 원형 회전 테스트로 접지감을 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.RearWheelFrictionForceMultiplier"), TEXT("후륜 마찰력 배수"), TargetMovementConfig.RearWheelFrictionForceMultiplier, SourceMovementConfig.RearWheelFrictionForceMultiplier, 0.75f, TEXT("출발 휠스핀과 코너 탈출 미끄러짐을 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.FrontWheelSpringRate"), TEXT("전륜 스프링 강성"), TargetMovementConfig.FrontWheelSpringRate, SourceMovementConfig.FrontWheelSpringRate, 100.0f, TEXT("정지 차고와 작은 턱 통과 시 차체 출렁임을 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.RearWheelSpringRate"), TEXT("후륜 스프링 강성"), TargetMovementConfig.RearWheelSpringRate, SourceMovementConfig.RearWheelSpringRate, 100.0f, TEXT("정지 차고와 작은 턱 통과 시 차체 출렁임을 확인하세요."));
	CFVDAValidatorInternal::AddBoolCompareItem(Report, FName(TEXT("Compare")), TEXT("VehicleMovementConfig.bEnableCenterOfMassOverride"), TEXT("중심질량 오버라이드 사용 여부"), TargetMovementConfig.bEnableCenterOfMassOverride, SourceMovementConfig.bEnableCenterOfMassOverride, ECFVDASeverity::Warning, TEXT("중심질량 변경은 전복과 롤링에 큰 영향을 주므로 마지막 단계에서 확인하세요."));

	// 대상 DA의 DriveState 설정입니다.
	const FCFVehicleDriveStateConfig& TargetDriveStateConfig = TargetVehicleData->DriveStateConfig;

	// 기준 DA의 DriveState 설정입니다.
	const FCFVehicleDriveStateConfig& SourceDriveStateConfig = SourceVehicleData->DriveStateConfig;

	CFVDAValidatorInternal::AddBoolCompareItem(Report, FName(TEXT("Compare")), TEXT("DriveStateConfig.bUseDriveStateOverrides"), TEXT("DriveState 덮어쓰기 사용"), TargetDriveStateConfig.bUseDriveStateOverrides, SourceDriveStateConfig.bUseDriveStateOverrides, ECFVDASeverity::Warning, TEXT("차량별 DriveState 설정 적용 여부가 의도와 맞는지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("DriveStateConfig.IdleEnterSpeedThresholdKmh"), TEXT("Idle 진입 속도 임계값"), TargetDriveStateConfig.IdleEnterSpeedThresholdKmh, SourceDriveStateConfig.IdleEnterSpeedThresholdKmh, 1.0f, TEXT("정지/저속 이동 중 Idle 상태가 튀는지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("DriveStateConfig.ReverseEnterSpeedThresholdKmh"), TEXT("Reversing 진입 속도 임계값"), TargetDriveStateConfig.ReverseEnterSpeedThresholdKmh, SourceDriveStateConfig.ReverseEnterSpeedThresholdKmh, 1.0f, TEXT("후진 진입 상태가 의도대로 표시되는지 확인하세요."));
	CFVDAValidatorInternal::AddFloatCompareItem(Report, FName(TEXT("Compare")), TEXT("DriveStateConfig.AirborneMinSpeedThresholdKmh"), TEXT("공중 상태 최소 속도 임계값"), TargetDriveStateConfig.AirborneMinSpeedThresholdKmh, SourceDriveStateConfig.AirborneMinSpeedThresholdKmh, 2.0f, TEXT("작은 턱과 점프 상황에서 Airborne 상태가 과하게 잡히는지 확인하세요."));

	CFVDAValidatorInternal::FinalizeReport(Report);
	return Report;
}
