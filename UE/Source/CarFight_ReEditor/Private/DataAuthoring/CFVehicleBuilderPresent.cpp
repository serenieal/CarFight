// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderPresent.cpp
// Version: v1.5.0
// Date: 2026-09-04
// Description: CF-FQ-046 USER presentation + CF-FQ-047 Hardpoint/Mount semantic readback / Step 8 progress·Driving Apply·Recipe Save formatter 구현입니다.
// Changelog:
// - v1.5.0: VBHAI-P0-07E에서 cached 7단계 benchmark progress/elapsed와 exact current Recipe explicit Save preflight/outcome을 raw path/hash 없이 USER-facing 한국어로 변환.
// - v1.4.0: VBHAI-P0-07B에서 typed Driving Apply blocker와 benchmark-running 상태를 raw path/hash 없이 USER가 바로 복구할 수 있는 한국어 안내로 변환하는 formatter를 추가.
// - v1.3.1: CF-FQ-046×047 통합 정리. .h/.cpp 버전·changelog를 동기화하고 USER presentation ownership과 Hardpoint integrity authority 경계를 명시. formatter logic 변경 없음.
// - v1.3.0: CF-FQ-047 Hardpoint/Mount semantic readback formatter를 추가해 Step 6/7/8의 typed 장착 상태를 USER 문장으로 투영.
// - v1.2.0: VBIUX-P0-04. Step 7 typed diff descriptor/grouping/unknown fallback과 Step 8 기술 측정·직접 주행 checklist formatter를 추가.
// - v1.1.0: VBIUX-P0-03. Step 5 AI Draft/승인 Profile source label과 엔진·변속기·구동·조향·제동·서스펜션·질량의 값+뜻+영향 설명을 추가.
// - v1.0.0: Step 1~4의 USER purpose/action과 Reference/Mesh/Socket/Layout human-readable formatter 최초 구현.
// Migration:
// - v1.5.0 progress/save formatter는 typed input만 표시하며 disk polling, SavePackage, receipt validation authority를 소유하지 않습니다.
// - v1.4.0 Driving Apply readiness는 typed blocker를 표시 문장으로만 변환하며 stable readiness authority는 BuilderVM, benchmark process/wrong-step 상태는 BuilderTab에 남습니다.
// - v1.3.x에서 이 helper는 CF-FQ-047 typed truth를 USER 문장으로 투영만 하며 Hardpoint integrity/receipt validation/mutation authority는 FCFVehicleBuilderHardpointIntegrity/BuilderVM에 남습니다.
// - 이 파일은 표시 문자열만 만들며 Authoring/Resolver/Apply/Save authority를 소유하지 않습니다.

#include "CFVehicleBuilderPresent.h"

namespace
{
	// Optional asset path를 USER가 알아보기 쉬운 Asset 이름으로 변환합니다.
	FString AssetDisplayName(const FSoftObjectPath& AssetPath)
	{
		return AssetPath.IsValid() ? AssetPath.GetAssetName() : TEXT("미지정");
	}

	// Optional Reference identity 문자열 하나를 구분자와 함께 누적합니다.
	void AppendIdentityPart(FString& InOutText, const FString& Part)
	{
		if (Part.IsEmpty())
		{
			return;
		}

		if (!InOutText.IsEmpty())
		{
			InOutText += TEXT(" | ");
		}
		InOutText += Part;
	}

	/** Step 7 known field의 USER value formatter 종류입니다. */
	enum class ECFVehicleReviewValueFormat : uint8
	{
		Plain,
		TorqueNm,
		Rpm,
		MassKg,
		Seconds,
		Centimeters,
		AngleDegrees,
		Ratio,
		Boolean
	};

	/** Resolver field 의미와 분리된 Editor-only USER metadata입니다. */
	struct FCFVehicleReviewFieldDescriptor
	{
		const TCHAR* CanonicalPattern;
		const TCHAR* UserLabel;
		ECFVehicleReviewValueFormat ValueFormat;
		const TCHAR* Meaning;
		const TCHAR* EffectSummary;
	};

	// Step 7에서 자주 등장하는 scalar field만 bounded USER descriptor로 정의합니다.
	const FCFVehicleReviewFieldDescriptor* FindReviewFieldDescriptor(const FCFVehicleFieldPath& FieldPath)
	{
		static const FCFVehicleReviewFieldDescriptor Descriptors[] =
		{
			{TEXT("BaseVehicleMassKg"), TEXT("기준 차량 질량"), ECFVehicleReviewValueFormat::MassKg, TEXT("장비·탄약·방어 추가 전 차량 플랫폼의 기준 질량입니다."), TEXT("차량 관성과 가속·제동, 피팅 총중량 계산의 출발점에 영향을 줍니다.")},
			{TEXT("MaximumGrossMassKg"), TEXT("최대 허용 총중량"), ECFVehicleReviewValueFormat::MassKg, TEXT("장비를 포함한 차량이 허용하는 최대 총중량입니다."), TEXT("장비 피팅이 허용되는 총중량 한계를 결정합니다.")},
			{TEXT("VehicleMovementConfig.EngineMaxTorque"), TEXT("엔진 최대 토크"), ECFVehicleReviewValueFormat::TorqueNm, TEXT("엔진이 낼 수 있는 최대 회전 힘입니다."), TEXT("다른 조건이 같다면 값이 커질수록 가속에 사용할 수 있는 힘이 커집니다.")},
			{TEXT("VehicleMovementConfig.EngineMaxRPM"), TEXT("엔진 최대 RPM"), ECFVehicleReviewValueFormat::Rpm, TEXT("엔진이 사용할 수 있는 최대 회전수입니다."), TEXT("기어비와 함께 고회전 사용 범위와 속도 영역에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.EngineIdleRPM"), TEXT("공회전 RPM"), ECFVehicleReviewValueFormat::Rpm, TEXT("가속 입력이 없을 때 엔진이 유지하는 기본 회전수입니다."), TEXT("정차·저속에서 엔진이 유지되는 회전 상태에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RedlineStartRPM"), TEXT("레드라인 시작 RPM"), ECFVehicleReviewValueFormat::Rpm, TEXT("고회전 주의 구간이 시작되는 기준입니다."), TEXT("HUD와 고회전 운전 판단 기준에 사용됩니다.")},
			{TEXT("VehicleMovementConfig.ChangeUpRPM"), TEXT("상향 변속 RPM"), ECFVehicleReviewValueFormat::Rpm, TEXT("자동 변속에서 다음 높은 기어로 올리는 기준 RPM입니다."), TEXT("높을수록 한 기어를 더 높은 RPM까지 사용하는 방향으로 바뀝니다.")},
			{TEXT("VehicleMovementConfig.ChangeDownRPM"), TEXT("하향 변속 RPM"), ECFVehicleReviewValueFormat::Rpm, TEXT("자동 변속에서 더 낮은 기어로 내리는 기준 RPM입니다."), TEXT("저속 재가속 때 낮은 기어로 돌아가는 시점에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.FinalRatio"), TEXT("최종 감속비"), ECFVehicleReviewValueFormat::Ratio, TEXT("변속기 이후 바퀴까지 적용되는 최종 기어비입니다."), TEXT("다른 조건이 같다면 높을수록 구동력 쪽, 낮을수록 속도 범위 쪽으로 기울 수 있습니다.")},
			{TEXT("VehicleMovementConfig.GearChangeTime"), TEXT("변속 시간"), ECFVehicleReviewValueFormat::Seconds, TEXT("기어가 바뀌는 데 사용하는 시간입니다."), TEXT("길수록 변속 반응이 느리고 가속이 끊기는 느낌이 커질 수 있습니다.")},
			{TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"), TEXT("앞바퀴 최대 조향각"), ECFVehicleReviewValueFormat::AngleDegrees, TEXT("앞바퀴가 꺾일 수 있는 최대 각도입니다."), TEXT("다른 조건이 같다면 큰 각도는 작은 회전반경에 유리할 수 있습니다.")},
			{TEXT("VehicleMovementConfig.FrontWheelMaxBrakeTorque"), TEXT("앞바퀴 최대 브레이크 토크"), ECFVehicleReviewValueFormat::TorqueNm, TEXT("앞바퀴 브레이크가 낼 수 있는 최대 제동 토크입니다."), TEXT("타이어 그립·질량과 함께 실제 제동 성능에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RearWheelMaxBrakeTorque"), TEXT("뒷바퀴 최대 브레이크 토크"), ECFVehicleReviewValueFormat::TorqueNm, TEXT("뒷바퀴 브레이크가 낼 수 있는 최대 제동 토크입니다."), TEXT("앞뒤 제동 밸런스와 실제 제동 성능에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RearWheelMaxHandBrakeTorque"), TEXT("뒤 핸드브레이크 토크"), ECFVehicleReviewValueFormat::TorqueNm, TEXT("핸드브레이크 입력 때 뒷바퀴에 줄 수 있는 최대 제동 토크입니다."), TEXT("핸드브레이크로 뒷바퀴를 감속·잠그는 강도에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.FrontWheelRadius"), TEXT("앞바퀴 반지름"), ECFVehicleReviewValueFormat::Centimeters, TEXT("앞바퀴 물리 반지름입니다."), TEXT("차량 높이·회전 운동과 기어비 대비 실제 이동 속도에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RearWheelRadius"), TEXT("뒷바퀴 반지름"), ECFVehicleReviewValueFormat::Centimeters, TEXT("뒷바퀴 물리 반지름입니다."), TEXT("차량 높이·회전 운동과 기어비 대비 실제 이동 속도에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.FrontWheelSpringRate"), TEXT("앞 서스펜션 스프링 강성"), ECFVehicleReviewValueFormat::Plain, TEXT("앞 서스펜션 스프링의 단단함을 정하는 값입니다."), TEXT("질량·프리로드·이동량과 함께 차체 앞쪽 움직임에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RearWheelSpringRate"), TEXT("뒤 서스펜션 스프링 강성"), ECFVehicleReviewValueFormat::Plain, TEXT("뒤 서스펜션 스프링의 단단함을 정하는 값입니다."), TEXT("질량·프리로드·이동량과 함께 차체 뒤쪽 움직임에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.FrontWheelSuspensionMaxRaise"), TEXT("앞 서스펜션 최대 압축 이동"), ECFVehicleReviewValueFormat::Centimeters, TEXT("앞바퀴가 위쪽으로 움직일 수 있는 최대 거리입니다."), TEXT("요철에서 앞 서스펜션이 흡수할 수 있는 이동 범위에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RearWheelSuspensionMaxRaise"), TEXT("뒤 서스펜션 최대 압축 이동"), ECFVehicleReviewValueFormat::Centimeters, TEXT("뒷바퀴가 위쪽으로 움직일 수 있는 최대 거리입니다."), TEXT("요철에서 뒤 서스펜션이 흡수할 수 있는 이동 범위에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.FrontWheelSuspensionMaxDrop"), TEXT("앞 서스펜션 최대 신장 이동"), ECFVehicleReviewValueFormat::Centimeters, TEXT("앞바퀴가 아래쪽으로 움직일 수 있는 최대 거리입니다."), TEXT("지면을 따라 내려갈 수 있는 앞 서스펜션 이동 범위에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.RearWheelSuspensionMaxDrop"), TEXT("뒤 서스펜션 최대 신장 이동"), ECFVehicleReviewValueFormat::Centimeters, TEXT("뒷바퀴가 아래쪽으로 움직일 수 있는 최대 거리입니다."), TEXT("지면을 따라 내려갈 수 있는 뒤 서스펜션 이동 범위에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.bFrontWheelAffectedByEngine"), TEXT("앞바퀴 엔진 구동"), ECFVehicleReviewValueFormat::Boolean, TEXT("앞바퀴가 엔진 힘을 받는지 정합니다."), TEXT("전륜/사륜 구동 구성에 직접 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.bRearWheelAffectedByEngine"), TEXT("뒷바퀴 엔진 구동"), ECFVehicleReviewValueFormat::Boolean, TEXT("뒷바퀴가 엔진 힘을 받는지 정합니다."), TEXT("후륜/사륜 구동 구성에 직접 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.ChassisWidth"), TEXT("차체 폭"), ECFVehicleReviewValueFormat::Centimeters, TEXT("차량 물리 설정에서 사용하는 차체 폭입니다."), TEXT("차량 크기와 일부 물리·시각 기준에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.ChassisHeight"), TEXT("차체 높이"), ECFVehicleReviewValueFormat::Centimeters, TEXT("차량 물리 설정에서 사용하는 차체 높이입니다."), TEXT("차량 크기와 일부 물리·시각 기준에 영향을 줍니다.")},
			{TEXT("VehicleMovementConfig.SteeringAngleRatio"), TEXT("조향 각도 비율"), ECFVehicleReviewValueFormat::Ratio, TEXT("조향 입력과 실제 바퀴 조향각의 관계를 조정하는 값입니다."), TEXT("운전 입력에 대한 조향 민감도에 영향을 줍니다.")}
		};

		const FString CanonicalPath = FieldPath.ToCanonicalString(true);
		for (const FCFVehicleReviewFieldDescriptor& Descriptor : Descriptors)
		{
			if (CanonicalPath == Descriptor.CanonicalPattern)
			{
				return &Descriptor;
			}
		}
		return nullptr;
	}

	// Known Final Review primitive value를 USER 단위로 변환합니다.
	FString FormatReviewValue(const FCFVehicleFieldValue& Value, const ECFVehicleReviewValueFormat Format)
	{
		const FString& CanonicalText = Value.CanonicalValueText;
		if (Format == ECFVehicleReviewValueFormat::Boolean)
		{
			if (CanonicalText.Equals(TEXT("True"), ESearchCase::IgnoreCase) || CanonicalText == TEXT("1"))
			{
				return TEXT("사용");
			}
			if (CanonicalText.Equals(TEXT("False"), ESearchCase::IgnoreCase) || CanonicalText == TEXT("0"))
			{
				return TEXT("사용하지 않음");
			}
			return CanonicalText;
		}

		if (Format == ECFVehicleReviewValueFormat::Plain || !CanonicalText.IsNumeric())
		{
			return CanonicalText;
		}

		const double NumberValue = FCString::Atod(*CanonicalText);
		switch (Format)
		{
		case ECFVehicleReviewValueFormat::TorqueNm: return FString::Printf(TEXT("%.0f Nm"), NumberValue);
		case ECFVehicleReviewValueFormat::Rpm: return FString::Printf(TEXT("%.0f RPM"), NumberValue);
		case ECFVehicleReviewValueFormat::MassKg: return FString::Printf(TEXT("%.0f kg"), NumberValue);
		case ECFVehicleReviewValueFormat::Seconds: return FString::Printf(TEXT("%.2f초"), NumberValue);
		case ECFVehicleReviewValueFormat::Centimeters: return FString::Printf(TEXT("%.1f cm"), NumberValue);
		case ECFVehicleReviewValueFormat::AngleDegrees: return FString::Printf(TEXT("%.1f°"), NumberValue);
		case ECFVehicleReviewValueFormat::Ratio: return FString::Printf(TEXT("%.3f"), NumberValue);
		default: return CanonicalText;
		}
	}

	// Final Review operation enum을 USER 행동 이름으로 변환합니다.
	const TCHAR* ReviewOperationText(const ECFVehicleDiffOp Operation)
	{
		switch (Operation)
		{
		case ECFVehicleDiffOp::SetLeaf: return TEXT("값 변경");
		case ECFVehicleDiffOp::AddArrayElement: return TEXT("추가");
		case ECFVehicleDiffOp::RemoveArrayElement: return TEXT("삭제");
		case ECFVehicleDiffOp::MoveArrayElement: return TEXT("순서 변경");
		default: return TEXT("변경");
		}
	}

	// Stable collection의 한 element를 USER가 알아볼 이름으로 변환합니다.
	FString StableCollectionLabel(const FCFVehicleFieldPath& FieldPath)
	{
		if (FieldPath.CollectionPropertyName == TEXT("HardpointSlots"))
		{
			return FieldPath.SelectorKeyValue.IsNone() ? TEXT("장비 장착 위치") : FString::Printf(TEXT("장비 장착 위치 %s"), *FieldPath.SelectorKeyValue.ToString());
		}
		if (FieldPath.CollectionPropertyName == TEXT("MountProfiles"))
		{
			return FieldPath.SelectorKeyValue.IsNone() ? TEXT("장착 규칙") : FString::Printf(TEXT("장착 규칙 %s"), *FieldPath.SelectorKeyValue.ToString());
		}
		return TEXT("차량 목록 설정");
	}

	// Stable collection leaf 중 USER에게 직접 의미가 있는 항목 이름을 반환합니다.
	FString StableCollectionLeafLabel(const FCFVehicleFieldPath& FieldPath)
	{
		const FName LeafName = FieldPath.PropertyChain.IsEmpty() ? NAME_None : FieldPath.PropertyChain.Last();
		if (LeafName == TEXT("SocketName")) return TEXT("Socket 이름");
		if (LeafName == TEXT("LocationCategory")) return TEXT("장착 위치 종류");
		if (LeafName == TEXT("LocalLocation")) return TEXT("장착 위치 좌표");
		if (LeafName == TEXT("LocalRotation")) return TEXT("장착 위치 회전");
		if (LeafName == TEXT("LocationSlotRef")) return TEXT("사용 장착 위치");
		if (LeafName == TEXT("MountType")) return TEXT("장착 방식");
		if (LeafName == TEXT("SizeLimit")) return TEXT("허용 장비 크기");
		if (LeafName == TEXT("DefaultEquipmentPresetData")) return TEXT("기본 장비 프리셋");
		if (LeafName == TEXT("bExposedModule")) return TEXT("외부 노출 장비 여부");
		return FString();
	}

	// Stable collection group identity를 grouping key로 변환합니다.
	FString StableCollectionGroupKey(const FCFVehicleFieldPath& FieldPath)
	{
		if (FieldPath.CollectionPropertyName.IsNone())
		{
			return FString();
		}
		return FString::Printf(
			TEXT("%s|%s|%s"),
			*FieldPath.CollectionPropertyName.ToString(),
			*FieldPath.SelectorKeyPropertyName.ToString(),
			*FieldPath.SelectorKeyValue.ToString());
	}
}

// Stable StepId를 초보자용 단계 이름으로 변환합니다.
FText FCFVehicleBuilderPresentation::GetStepTitle(
	const ECFVehicleBuilderStepId StepId,
	const FText& FallbackTitle)
{
	switch (StepId)
	{
	case ECFVehicleBuilderStepId::IdentityReference: return FText::FromString(TEXT("차량 / 기준 자료"));
	case ECFVehicleBuilderStepId::MeshPrep: return FText::FromString(TEXT("차량 Mesh 준비"));
	case ECFVehicleBuilderStepId::SocketGuide: return FText::FromString(TEXT("소켓 준비 / 이름 설정"));
	case ECFVehicleBuilderStepId::LayoutCapture: return FText::FromString(TEXT("차량 배치 확인"));
	case ECFVehicleBuilderStepId::PhysicsProposal: return FText::FromString(TEXT("AI 물리 설정"));
	case ECFVehicleBuilderStepId::GameplaySetup: return FText::FromString(TEXT("게임플레이 설정"));
	case ECFVehicleBuilderStepId::FinalReview: return FText::FromString(TEXT("최종 검토"));
	case ECFVehicleBuilderStepId::DrivingTest: return FText::FromString(TEXT("주행 테스트"));
	default: return FallbackTitle;
	}
}

// Backend Step state를 USER 상태 표현으로 변환합니다.
FString FCFVehicleBuilderPresentation::GetStepStateLabel(const ECFVehicleBuilderStepState State)
{
	switch (State)
	{
	case ECFVehicleBuilderStepState::Unavailable: return TEXT("사용할 수 없음");
	case ECFVehicleBuilderStepState::Locked: return TEXT("이전 단계 필요");
	case ECFVehicleBuilderStepState::Ready: return TEXT("확인 / 작업 필요");
	case ECFVehicleBuilderStepState::Complete: return TEXT("완료");
	case ECFVehicleBuilderStepState::Blocked: return TEXT("진행 불가");
	case ECFVehicleBuilderStepState::Stale: return TEXT("다시 확인 필요");
	default: return TEXT("알 수 없음");
	}
}

// Step 1~8이 무엇을 확인하는 단계인지 기본 목적 설명을 반환합니다.
FText FCFVehicleBuilderPresentation::GetStepPurpose(
	const ECFVehicleBuilderStepId StepId,
	const FText& FallbackSummary)
{
	switch (StepId)
	{
	case ECFVehicleBuilderStepId::IdentityReference:
		return FText::FromString(TEXT("어떤 실존 차량과 자료를 기준으로 이 차량의 제작 값을 정할지 확인하는 단계입니다."));
	case ECFVehicleBuilderStepId::MeshPrep:
		return FText::FromString(TEXT("차체와 네 바퀴에 사용할 Static Mesh를 지정하고 필수 Mesh가 준비됐는지 확인하는 단계입니다."));
	case ECFVehicleBuilderStepId::SocketGuide:
		return FText::FromString(TEXT("바퀴와 장비를 실제로 붙일 Chassis Socket의 이름과 위치가 준비됐는지 확인하는 단계입니다."));
	case ECFVehicleBuilderStepId::LayoutCapture:
		return FText::FromString(TEXT("Wheel Socket 위치를 기준으로 Wheelbase와 좌우 바퀴 간격을 확인하고 VehicleData 배치와 비교하는 단계입니다."));
	case ECFVehicleBuilderStepId::PhysicsProposal:
		return FText::FromString(TEXT("AI가 준비한 차량의 엔진, 변속기, 브레이크 등 주행 물리 설정을 확인하는 단계입니다."));
	case ECFVehicleBuilderStepId::GameplaySetup:
		return FText::FromString(TEXT("무기와 장비를 달 장착 위치의 규칙과 차량의 게임플레이 설정을 확인하는 단계입니다."));
	case ECFVehicleBuilderStepId::FinalReview:
		return FText::FromString(TEXT("지금까지 정한 제작 설정이 실제 VehicleData에서 무엇을 바꾸는지 최종 확인하는 단계입니다."));
	case ECFVehicleBuilderStepId::DrivingTest:
		return FText::FromString(TEXT("기술 측정값을 확인하고 실제 PIE 주행으로 차량이 의도대로 움직이는지 직접 판정하는 단계입니다."));
	default:
		return FallbackSummary;
	}
}

// Step 1~4의 현재 state에 맞는 USER next action을 반환합니다.
FText FCFVehicleBuilderPresentation::GetStepNextAction(
	const FCFVehicleBuilderStepView& Step,
	const FText& FallbackResolution)
{
	switch (Step.StepId)
	{
	case ECFVehicleBuilderStepId::IdentityReference:
		switch (Step.State)
		{
		case ECFVehicleBuilderStepState::Complete:
			return FText::FromString(TEXT("기준 자료 확인이 끝났습니다. 아래 정보가 맞는지 확인한 뒤 '다음'으로 진행하세요."));
		case ECFVehicleBuilderStepState::Ready:
			return FText::FromString(TEXT("아래 기준 자료 상태를 확인하세요. 필요한 경우 AI 차량 정보를 불러오거나 제작 데이터를 준비한 뒤 '이 기준 정보로 진행'을 누르세요."));
		case ECFVehicleBuilderStepState::Blocked:
			return FText::FromString(TEXT("기준 자료나 차량 전용 제작 데이터에 해결해야 할 문제가 있습니다. 아래 기준 정보와 버튼을 확인한 뒤 현재 상태를 다시 확인하세요."));
		case ECFVehicleBuilderStepState::Stale:
			return FText::FromString(TEXT("기준 자료가 마지막 확인 이후 달라졌습니다. 현재 상태를 다시 확인하고 기준 정보를 다시 검토하세요."));
		default:
			return FText::FromString(TEXT("먼저 작업할 차량을 선택하거나 새 차량 제작을 시작하세요."));
		}

	case ECFVehicleBuilderStepId::MeshPrep:
		switch (Step.State)
		{
		case ECFVehicleBuilderStepState::Complete:
			return FText::FromString(TEXT("차체와 필수 Wheel Mesh가 준비됐습니다. 아래 Mesh가 의도한 자산인지 확인한 뒤 '다음'으로 진행하세요."));
		case ECFVehicleBuilderStepState::Ready:
			return FText::FromString(TEXT("차체와 앞왼쪽 Wheel Mesh를 지정하세요. 나머지 Wheel Mesh는 비워두면 앞왼쪽 Mesh를 재사용합니다."));
		case ECFVehicleBuilderStepState::Blocked:
			return FText::FromString(TEXT("선택한 Mesh를 차량 제작에 사용할 수 없습니다. 누락되거나 잘못된 Mesh를 다시 지정한 뒤 'Mesh 설정 반영'을 누르세요."));
		case ECFVehicleBuilderStepState::Stale:
			return FText::FromString(TEXT("Mesh 설정이 마지막 확인 이후 달라졌습니다. 현재 상태를 다시 확인하고 아래 Mesh 선택을 검토하세요."));
		default:
			return FText::FromString(TEXT("먼저 이전 단계의 기준 자료 확인을 완료하세요."));
		}

	case ECFVehicleBuilderStepId::SocketGuide:
		switch (Step.State)
		{
		case ECFVehicleBuilderStepState::Complete:
			return FText::FromString(TEXT("필수 Wheel Socket과 필요한 장착 위치가 준비됐습니다. 위치를 마지막으로 확인한 뒤 '다음'으로 진행하세요."));
		case ECFVehicleBuilderStepState::Ready:
			return FText::FromString(TEXT("아래에서 누락된 Socket을 확인하고 Chassis Static Mesh Editor에서 위치·회전·스케일을 직접 맞춘 뒤 저장하고 현재 상태를 다시 확인하세요."));
		case ECFVehicleBuilderStepState::Blocked:
			return FText::FromString(TEXT("필수 Wheel Socket 또는 장착 위치 설정에 진행을 막는 문제가 있습니다. 아래 누락 항목과 정확한 Socket 이름을 확인하세요."));
		case ECFVehicleBuilderStepState::Stale:
			return FText::FromString(TEXT("Socket 상태가 마지막 확인 이후 달라졌습니다. Chassis Mesh를 확인한 뒤 현재 상태를 다시 확인하세요."));
		default:
			return FText::FromString(TEXT("먼저 차체와 Wheel Mesh 준비를 완료하세요."));
		}

	case ECFVehicleBuilderStepId::LayoutCapture:
		switch (Step.State)
		{
		case ECFVehicleBuilderStepState::Complete:
			return FText::FromString(TEXT("현재 VehicleData의 Wheel 배치와 Chassis Socket 위치가 일치합니다. 수치를 확인한 뒤 '다음'으로 진행하세요."));
		case ECFVehicleBuilderStepState::Ready:
			return FText::FromString(TEXT("현재 Socket 배치는 정상이며 아직 VehicleData에 반영되지 않은 상태입니다. 아래 수치를 확인한 뒤 '다음'으로 진행하면 최종 반영은 7단계에서 검토합니다."));
		case ECFVehicleBuilderStepState::Stale:
			return FText::FromString(TEXT("현재 VehicleData의 Wheel 배치와 Chassis Socket 위치가 다릅니다. '현재 상태 다시 확인' 후에도 계속 다르면 Chassis Socket과 고급 차량 데이터 제작 화면의 배치 상태를 점검하세요."));
		case ECFVehicleBuilderStepState::Blocked:
			return FText::FromString(TEXT("배치 수치를 계산할 필수 Wheel Socket 또는 Target VehicleData를 확인할 수 없습니다. 3단계 Socket 상태를 먼저 정상화하세요."));
		default:
			return FText::FromString(TEXT("먼저 3단계의 필수 Wheel Socket 준비를 완료하세요."));
		}

	case ECFVehicleBuilderStepId::FinalReview:
		switch (Step.State)
		{
		case ECFVehicleBuilderStepState::Complete:
			return FText::FromString(TEXT("현재 제작 설정이 VehicleData에 반영된 상태입니다. 변경 내용을 확인한 뒤 저장하고 '다음'으로 주행 테스트를 진행하세요."));
		case ECFVehicleBuilderStepState::Ready:
			return FText::FromString(TEXT("아래 '현재 → 변경 후' 목록을 확인하세요. 의도한 변경이 맞으면 '변경 적용'을 누르고, 적용하지 않을 항목이 있으면 이전 단계에서 설정을 수정하세요."));
		case ECFVehicleBuilderStepState::Stale:
			return FText::FromString(TEXT("마지막 확인 이후 VehicleData 또는 제작 설정이 달라졌습니다. '현재 상태 다시 확인'을 누른 뒤 새 변경 목록을 다시 검토하세요."));
		case ECFVehicleBuilderStepState::Blocked:
			return FText::FromString(TEXT("VehicleData에 적용하기 전에 해결해야 할 문제가 있습니다. 아래 변경 상태를 확인하고 필요한 이전 단계 설정을 수정하세요."));
		default:
			return FText::FromString(TEXT("먼저 앞 단계의 차량 제작 설정을 완료하세요."));
		}

	case ECFVehicleBuilderStepId::DrivingTest:
		switch (Step.State)
		{
		case ECFVehicleBuilderStepState::Complete:
			return FText::FromString(TEXT("현재 차량 상태에 대한 직접 주행 확인이 완료되었습니다. 데모 차량 목록 등록 상태와 저장 여부를 마지막으로 확인하세요."));
		case ECFVehicleBuilderStepState::Ready:
			return FText::FromString(TEXT("먼저 기술 주행 측정을 실행하고, PIE에서 선택 차량을 직접 주행하세요. 체크리스트를 확인한 뒤 이상이 없으면 '주행 테스트 통과'를 누르세요."));
		case ECFVehicleBuilderStepState::Stale:
			return FText::FromString(TEXT("주행 확인 뒤 차량 데이터가 달라졌습니다. 저장 상태와 기술 측정을 다시 확인하고 현재 차량으로 직접 주행을 다시 진행하세요."));
		case ECFVehicleBuilderStepState::Blocked:
			return FText::FromString(TEXT("주행 테스트를 시작하기 위한 저장 상태 또는 이전 단계 조건이 준비되지 않았습니다. 아래 안내에 따라 필요한 항목을 먼저 완료하세요."));
		default:
			return FText::FromString(TEXT("먼저 7단계 최종 변경 적용을 완료하세요."));
		}

	default:
		return FallbackResolution;
	}
}

// Step 1 persistent Evidence 또는 loaded Draft scalar truth를 사용자용 기준 자료 요약으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildReferenceSummary(const FCFVehicleReferencePresentInfo& Info)
{
	if (!Info.bHasPersistentEvidence && !Info.bHasLoadedDraft)
	{
		return FText::FromString(
			TEXT("아직 차량 기준 정보가 준비되지 않았습니다. 'AI 차량 정보 불러오기'로 조사 결과를 읽은 뒤 필요한 제작 데이터를 검토하세요."));
	}

	// USER에게 보여줄 Primary Reference identity입니다.
	FString IdentityText;
	AppendIdentityPart(IdentityText, Info.Manufacturer);
	AppendIdentityPart(IdentityText, Info.Model);
	if (Info.ModelYearStart > 0)
	{
		AppendIdentityPart(IdentityText, FString::Printf(TEXT("%d년형"), Info.ModelYearStart));
	}
	AppendIdentityPart(IdentityText, Info.Trim);
	AppendIdentityPart(IdentityText, Info.Powertrain);
	AppendIdentityPart(IdentityText, Info.Transmission);
	if (IdentityText.IsEmpty())
	{
		IdentityText = TEXT("Primary 차량 정보 없음");
	}

	// Persistent/AI Draft source 구분을 명확히 하는 USER label입니다.
	const FString SourceLabel = Info.bHasPersistentEvidence
		? TEXT("현재 기준 자료")
		: TEXT("AI 기준 자료 제안");

	FString Summary = FString::Printf(
		TEXT("[%s]\n%s\n\n참고 자료: %d개\n충돌 항목: %d개"),
		*SourceLabel,
		*IdentityText,
		Info.SourceCount,
		Info.ConflictCount);

	if (Info.BlockingConflictCount > 0)
	{
		Summary += FString::Printf(TEXT(" — 이 중 %d개는 해결 필요"), Info.BlockingConflictCount);
	}
	Summary += FString::Printf(TEXT("\n아직 확인되지 않은 정보: %d개"), Info.UnknownCount);
	if (Info.BlockingUnknownCount > 0)
	{
		Summary += FString::Printf(TEXT(" — 이 중 %d개는 물리 설정 진행 전 확인 필요"), Info.BlockingUnknownCount);
	}

	if (Info.bHasPersistentEvidence)
	{
		Summary += Info.bUserReviewed
			? TEXT("\n\n이 기준 정보는 사용자 확인을 마쳤습니다.")
			: TEXT("\n\n기준 자료는 준비됐지만 아직 사용자 확인이 필요합니다.");
	}
	else
	{
		Summary += TEXT("\n\n아직 기준 자료로 확정되지 않은 AI 제안입니다.");
	}

	return FText::FromString(Summary);
}

// Step 2 Recipe AssetIntent를 사용자용 Chassis/Wheel Mesh 요약으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildMeshSummary(const FCFVehicleAssetIntent& AssetIntent)
{
	// Current Chassis Asset 이름입니다.
	const FString ChassisName = AssetDisplayName(AssetIntent.ChassisMesh.ToSoftObjectPath());
	// Required FL Wheel Asset 이름입니다.
	const FString WheelFLName = AssetDisplayName(AssetIntent.WheelMeshFL.ToSoftObjectPath());

	// Optional role이 비었을 때 USER에게 FL fallback을 명확히 표시합니다.
	auto OptionalWheelText = [&WheelFLName](const TSoftObjectPtr<UStaticMesh>& WheelMesh) -> FString
	{
		return WheelMesh.IsNull()
			? FString::Printf(TEXT("%s 재사용"), *WheelFLName)
			: AssetDisplayName(WheelMesh.ToSoftObjectPath());
	};

	const FString WheelFRName = OptionalWheelText(AssetIntent.WheelMeshFR);
	const FString WheelRLName = OptionalWheelText(AssetIntent.WheelMeshRL);
	const FString WheelRRName = OptionalWheelText(AssetIntent.WheelMeshRR);

	return FText::FromString(FString::Printf(
		TEXT("차체 Mesh: %s\n앞왼쪽 Wheel: %s\n앞오른쪽 Wheel: %s\n뒤왼쪽 Wheel: %s\n뒤오른쪽 Wheel: %s\n\n앞오른쪽/뒤쪽 Wheel Mesh를 따로 지정하지 않으면 앞왼쪽 Wheel Mesh를 재사용합니다."),
		*ChassisName,
		*WheelFLName,
		*WheelFRName,
		*WheelRLName,
		*WheelRRName));
}

// Step 3 required Wheel Socket + shared layout facts를 사용자용 요약으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildSocketSummary(const FCFVehicleSocketPresentInfo& Info)
{
	static const TCHAR* WheelRoleLabels[] =
	{
		TEXT("앞왼쪽"),
		TEXT("앞오른쪽"),
		TEXT("뒤왼쪽"),
		TEXT("뒤오른쪽")
	};

	FString Summary;
	for (int32 WheelRoleIndex = 0; WheelRoleIndex < 4; ++WheelRoleIndex)
	{
		// 이 role의 effective Socket 이름입니다.
		const FString SocketName = Info.WheelSocketNames.IsValidIndex(WheelRoleIndex)
			? Info.WheelSocketNames[WheelRoleIndex].ToString()
			: TEXT("알 수 없음");
		// 이 role Socket의 current Chassis 존재 여부입니다.
		const bool bFound = Info.WheelSocketFound.IsValidIndex(WheelRoleIndex)
			&& Info.WheelSocketFound[WheelRoleIndex];

		Summary += FString::Printf(
			TEXT("%s: %s — %s\n"),
			WheelRoleLabels[WheelRoleIndex],
			*SocketName,
			bFound ? TEXT("있음") : TEXT("없음"));
	}

	if (Info.LayoutFacts.bAvailable)
	{
		Summary += FString::Printf(
			TEXT("\n현재 Socket 기준 Wheelbase: %.0f mm\n앞 윤거: %.0f mm\n뒤 윤거: %.0f mm"),
			Info.LayoutFacts.WheelbaseCm * 10.0f,
			Info.LayoutFacts.FrontTrackCm * 10.0f,
			Info.LayoutFacts.RearTrackCm * 10.0f);
	}

	Summary += TEXT("\n\nSocket 이름은 차량 코드가 위치를 찾기 위한 정확한 키입니다. 위치·회전·스케일은 Chassis Static Mesh Editor에서 직접 맞춥니다.");
	return FText::FromString(Summary);
}

// Step 4 current Socket/VehicleData layout facts를 사용자용 배치 요약으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildLayoutSummary(
	const FCFVehicleBuilderLayoutFacts& Facts,
	const ECFVehicleBuilderStepState State)
{
	if (!Facts.bAvailable)
	{
		return FText::FromString(TEXT("현재 차량 배치 수치를 읽을 수 없습니다. 3단계의 필수 Wheel Socket을 확인한 뒤 '현재 상태 다시 확인'을 눌러주세요."));
	}

	FString Summary = FString::Printf(
		TEXT("차량 전방 기준: +X\nWheelbase: %.0f mm\n앞 윤거(Front Track): %.0f mm\n뒤 윤거(Rear Track): %.0f mm\n\n"),
		Facts.WheelbaseCm * 10.0f,
		Facts.FrontTrackCm * 10.0f,
		Facts.RearTrackCm * 10.0f);

	Summary += TEXT("Wheelbase는 앞/뒤 차축 중심 사이 거리이고, 윤거는 같은 차축의 좌우 바퀴 중심 사이 거리입니다.\n\n");

	if (!Facts.bTargetUsesLayoutOverrides && State == ECFVehicleBuilderStepState::Ready)
	{
		Summary += TEXT("현재 Socket 배치는 유효합니다. 아직 VehicleData에는 이 Wheel 배치가 저장되지 않았으며 실제 반영은 7단계 최종 검토에서 확인합니다.");
	}
	else if (Facts.bTargetUsesLayoutOverrides && Facts.WheelLayoutMismatchCount == 0)
	{
		Summary += TEXT("현재 VehicleData의 Wheel 배치와 Chassis Socket 위치가 일치합니다.");
	}
	else if (Facts.bTargetUsesLayoutOverrides && Facts.WheelLayoutMismatchCount > 0)
	{
		Summary += FString::Printf(
			TEXT("현재 VehicleData의 Wheel 배치와 Chassis Socket 위치 사이에 차이 항목이 %d개 있습니다. 자동으로 덮어쓰지 않습니다."),
			Facts.WheelLayoutMismatchCount);
	}
	else
	{
		Summary += TEXT("현재 배치 상태를 다시 확인해야 합니다.");
	}

	if (Facts.HardpointLayoutWarningCount > 0)
	{
		Summary += FString::Printf(
			TEXT("\n장비 장착 위치도 %d곳에서 현재 Socket과 저장된 위치가 다를 수 있습니다. 이 항목의 최종 필요 여부는 6단계 게임플레이 설정에서 확인합니다."),
			Facts.HardpointLayoutWarningCount);
	}

	return FText::FromString(Summary);
}


// Step 5 AI Draft 또는 승인 완료 Builder-private Profile을 값+뜻+주행 영향 구조로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildPhysicsSummary(
	const FCFBuilderPrivateProfilePayload& ProfilePayload,
	const ECFVehiclePhysicsPresentationSource Source)
{
	// 차량 플랫폼/질량 설정입니다.
	const FCFVehicleBaseProfileData& VehicleBaseData = ProfilePayload.VehicleBaseData;
	// 구동계와 변속기 설정입니다.
	const FCFDrivetrainProfileData& DrivetrainData = ProfilePayload.DrivetrainData;
	// 조향/제동/서스펜션 설정입니다.
	const FCFHandlingProfileData& HandlingData = ProfilePayload.HandlingData;
	// 엔진/성능 설정입니다.
	const FCFPerformanceProfileData& PerformanceData = ProfilePayload.PerformanceData;

	// 현재 payload가 어떤 승인 단계의 truth인지 USER에게 명확히 표시합니다.
	const FString SourceText = Source == ECFVehiclePhysicsPresentationSource::AiDraft
		? TEXT("[정보 출처] AI 물리 설정 제안 — 아직 차량 전용 제작 프로필에 반영되지 않았습니다.")
		: TEXT("[정보 출처] 승인 완료 차량 전용 제작 프로필 — Step 5 반영이 끝난 제작 기준입니다.");

	// 실제 엔진 영향을 받는 바퀴 기준의 쉬운 구동 방식 설명입니다.
	FString DriveTypeText;
	if (DrivetrainData.bFrontWheelAffectedByEngine && DrivetrainData.bRearWheelAffectedByEngine)
	{
		DriveTypeText = TEXT("사륜구동 — 앞/뒤 바퀴 모두 엔진 힘을 받습니다.");
	}
	else if (DrivetrainData.bFrontWheelAffectedByEngine)
	{
		DriveTypeText = TEXT("전륜구동 — 앞바퀴가 엔진 힘을 받습니다.");
	}
	else if (DrivetrainData.bRearWheelAffectedByEngine)
	{
		DriveTypeText = TEXT("후륜구동 — 뒷바퀴가 엔진 힘을 받습니다.");
	}
	else
	{
		DriveTypeText = TEXT("구동 바퀴 설정 없음 — 실제 적용 전에 반드시 확인이 필요합니다.");
	}

	// 차량별 엔진 토크 커브 사용 상태입니다.
	const FString EngineCurveText = PerformanceData.bUseEngineTorqueCurve
		? FString::Printf(
			TEXT("사용 (%d개 RPM 지점) — RPM에 따라 엔진이 최대 토크를 얼마나 내는지 정하는 곡선입니다."),
			PerformanceData.EngineTorqueCurve.Points.Num())
		: TEXT("사용하지 않음 — 기존 차량/Chaos 엔진 커브를 유지하는 설정입니다.");
	// 차량별 변속기 설정 사용 상태입니다.
	const FString TransmissionModeText = DrivetrainData.bUseTransmissionConfig
		? FString::Printf(
			TEXT("%s / 전진 %d단"),
			DrivetrainData.bUseAutomaticGears ? TEXT("자동 변속") : TEXT("수동 변속"),
			DrivetrainData.TransmissionRatios.ForwardGearRatios.Num())
		: TEXT("차량별 변속기 설정 미사용 — 기존 변속기 설정을 유지합니다.");
	// 레드라인이 명시된 경우에만 실제 RPM을 표시합니다.
	const FString RedlineText = PerformanceData.RedlineStartRPM > 0.0f
		? FString::Printf(
			TEXT("%.0f RPM — HUD에서 고회전 주의 구간이 시작되는 기준입니다."),
			PerformanceData.RedlineStartRPM)
		: TEXT("미설정 — 별도 레드라인 시작값을 사용하지 않습니다.");

	FString Summary = SourceText;
	Summary += TEXT("\n\n※ '낮음 / 기본 / 높음' 3개 값은 동시에 VehicleData에 들어가는 값이 아닙니다. 차량의 주행감 성향(Feel)에 따라 이 프로필에서 실제 값을 고르거나 보간합니다. 최종 VehicleData 값은 7단계에서 확인합니다.\n\n");

	Summary += TEXT("[엔진]\n");
	Summary += FString::Printf(
		TEXT("최대 토크 프로필: 낮음 %.0f / 기본 %.0f / 높음 %.0f Nm\n"),
		PerformanceData.EngineMaxTorqueByFeel.LowValue,
		PerformanceData.EngineMaxTorqueByFeel.NeutralValue,
		PerformanceData.EngineMaxTorqueByFeel.HighValue);
	Summary += TEXT("→ 가속 성향에 따라 사용할 엔진 힘의 기준입니다. 일반적으로 토크가 커질수록 같은 조건에서 바퀴에 전달할 수 있는 가속 힘이 커집니다.\n");
	Summary += FString::Printf(
		TEXT("최대 RPM 프로필: 낮음 %.0f / 기본 %.0f / 높음 %.0f RPM\n"),
		PerformanceData.EngineMaxRPMByFeel.LowValue,
		PerformanceData.EngineMaxRPMByFeel.NeutralValue,
		PerformanceData.EngineMaxRPMByFeel.HighValue);
	Summary += TEXT("→ 가속 성향에 따라 엔진이 사용할 수 있는 최대 회전수의 기준입니다. 실제 최고속도는 기어비·타이어 크기·토크·저항과 함께 결정됩니다.\n");
	Summary += FString::Printf(
		TEXT("공회전 RPM: %.0f RPM\n→ 가속 입력이 없을 때 엔진이 유지하는 기본 회전수입니다.\n"),
		PerformanceData.EngineIdleRPM);
	Summary += FString::Printf(TEXT("레드라인 시작: %s\n"), *RedlineText);
	Summary += FString::Printf(TEXT("엔진 토크 커브: %s\n\n"), *EngineCurveText);

	Summary += TEXT("[변속기]\n");
	Summary += FString::Printf(TEXT("방식: %s\n"), *TransmissionModeText);
	if (DrivetrainData.bUseTransmissionConfig)
	{
		Summary += FString::Printf(
			TEXT("상향 변속: %.0f RPM\n→ 자동 변속에서 엔진이 이 RPM에 도달하면 다음 높은 기어로 올리는 기준입니다.\n"),
			DrivetrainData.ChangeUpRPM);
		Summary += FString::Printf(
			TEXT("하향 변속: %.0f RPM\n→ 자동 변속에서 RPM이 이 기준까지 낮아지면 더 낮은 기어로 내리는 기준입니다.\n"),
			DrivetrainData.ChangeDownRPM);
		Summary += FString::Printf(
			TEXT("최종 감속비: %.2f\n→ 다른 조건이 같다면 값이 높을수록 바퀴 구동력 쪽, 낮을수록 속도 범위 쪽으로 기울 수 있습니다. 실제 결과는 각 기어비·엔진·Wheel 크기와 함께 결정됩니다.\n"),
			DrivetrainData.FinalRatio);
		Summary += FString::Printf(
			TEXT("변속 시간: %.2f초\n→ 한 기어에서 다음 기어로 전환하는 데 사용하는 시간입니다. 길수록 변속 반응이 느리고 끊기는 느낌이 커질 수 있습니다.\n"),
			DrivetrainData.GearChangeTime);
	}
	Summary += TEXT("\n");

	Summary += TEXT("[구동]\n");
	Summary += FString::Printf(TEXT("구동 방식: %s\n\n"), *DriveTypeText);

	Summary += TEXT("[조향 / 제동]\n");
	Summary += FString::Printf(
		TEXT("앞바퀴 최대 조향각 프로필: 낮음 %.1f / 기본 %.1f / 높음 %.1f°\n"),
		HandlingData.FrontWheelMaxSteerAngleByFeel.LowValue,
		HandlingData.FrontWheelMaxSteerAngleByFeel.NeutralValue,
		HandlingData.FrontWheelMaxSteerAngleByFeel.HighValue);
	Summary += TEXT("→ 조향 민첩성 성향에 따라 앞바퀴가 꺾일 수 있는 최대 각도의 기준입니다. 큰 각도는 보통 더 작은 회전반경에 유리하지만 실제 반응은 조향 비율·속도·그립과 함께 결정됩니다.\n");
	Summary += FString::Printf(
		TEXT("최대 브레이크 토크: 앞 %.0f / 뒤 %.0f Nm\n"),
		HandlingData.FrontWheelMaxBrakeTorque,
		HandlingData.RearWheelMaxBrakeTorque);
	Summary += TEXT("→ 각 바퀴에 걸 수 있는 최대 제동 토크입니다. 큰 값은 더 강한 제동 여력을 주지만 실제 제동거리는 타이어 그립·질량·노면과 함께 결정됩니다.\n");
	Summary += FString::Printf(
		TEXT("뒤 핸드브레이크 토크: %.0f Nm\n→ 핸드브레이크 입력 때 뒷바퀴에 줄 수 있는 최대 제동 토크입니다.\n\n"),
		HandlingData.RearWheelMaxHandBrakeTorque);

	Summary += TEXT("[서스펜션]\n");
	Summary += FString::Printf(
		TEXT("앞 스프링 강성 프로필: 낮음 %.0f / 기본 %.0f / 높음 %.0f\n"),
		HandlingData.FrontSpringRateByFeel.LowValue,
		HandlingData.FrontSpringRateByFeel.NeutralValue,
		HandlingData.FrontSpringRateByFeel.HighValue);
	Summary += FString::Printf(
		TEXT("뒤 스프링 강성 프로필: 낮음 %.0f / 기본 %.0f / 높음 %.0f\n"),
		HandlingData.RearSpringRateByFeel.LowValue,
		HandlingData.RearSpringRateByFeel.NeutralValue,
		HandlingData.RearSpringRateByFeel.HighValue);
	Summary += TEXT("→ 서스펜션 단단함 성향에 따라 사용할 스프링 강성 기준입니다. 값이 높을수록 일반적으로 차체 움직임이 단단해지는 방향이지만 실제 거동은 질량·프리로드·서스펜션 이동량과 함께 결정됩니다.\n\n");

	Summary += TEXT("[차량 기본 / 질량]\n");
	Summary += FString::Printf(
		TEXT("기준 차량 질량: %.0f kg\n→ 장비·탄약·방어를 더하기 전 차량 플랫폼의 기준 질량입니다. 피팅과 차량 총질량 계산의 출발점으로 사용됩니다.\n"),
		VehicleBaseData.BaseVehicleMassKg);
	Summary += FString::Printf(
		TEXT("최대 허용 총중량: %.0f kg\n→ 장비를 포함한 차량이 허용하는 최대 총중량입니다. 기준 차량 질량보다 작을 수 없습니다.\n"),
		VehicleBaseData.MaximumGrossMassKg);
	Summary += FString::Printf(
		TEXT("차체 폭: %.0f mm\n→ 차량 물리 설정에서 사용하는 차체 폭입니다. UE 내부 cm 값을 사용자 제원 표기로 mm 변환해 표시합니다.\n"),
		VehicleBaseData.ChassisWidth * 10.0f);

	Summary += Source == ECFVehiclePhysicsPresentationSource::AiDraft
		? TEXT("\n이 값들은 아직 AI 제안입니다. 'AI 물리 설정 검토 후 반영'을 승인해야 차량 전용 제작 프로필에 들어갑니다. 실제 VehicleData는 아직 바뀌지 않습니다.")
		: TEXT("\n이 값들은 Step 5에서 승인된 차량 전용 제작 프로필입니다. 실제 VehicleData에 어떤 값이 들어갈지는 7단계 '최종 검토'에서 다시 확인한 뒤 적용합니다.");

	return FText::FromString(Summary);
}


// Step 6 Recipe authored Hardpoint↔Mount 관계를 completion authority와 분리된 USER 참고 정보로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildMountSemanticSummary(const FCFBuilderHardpointSemanticFacts& Facts)
{
	if (!Facts.bAvailable)
	{
		return FText::FromString(TEXT("장비 장착 위치와 장착 규칙의 연결 상태를 읽을 수 없습니다."));
	}

	FString Summary = TEXT("[현재 제작 기록의 장착 관계]\n");
	Summary += FString::Printf(
		TEXT("장착 위치 %d개 / 장착 규칙 %d개\n"),
		Facts.AuthoredHardpointCount,
		Facts.AuthoredMountCount);

	if (Facts.AuthoredRelations.IsEmpty())
	{
		Summary += TEXT("현재 제작 기록에는 장착 위치가 없습니다. 이 숫자는 VehicleData 적용 결과가 아니라 6단계에서 작성 중인 제작 기록입니다.");
		return FText::FromString(Summary);
	}

	for (const FCFBuilderHardpointMountRelationFact& Relation : Facts.AuthoredRelations)
	{
		const FString SocketText = Relation.SocketName.IsNone() ? TEXT("Socket 없음") : Relation.SocketName.ToString();
		Summary += FString::Printf(TEXT("- %s → %s"), *Relation.LocationSlotId.ToString(), *SocketText);
		if (Relation.MountProfileIds.IsEmpty())
		{
			Summary += TEXT(" → 장착 규칙 없음\n");
		}
		else
		{
			Summary += TEXT(" → ");
			for (int32 Index = 0; Index < Relation.MountProfileIds.Num(); ++Index)
			{
				if (Index > 0)
				{
					Summary += TEXT(", ");
				}
				Summary += Relation.MountProfileIds[Index].ToString();
			}
			Summary += TEXT("\n");
		}
	}

	Summary += TEXT("이 관계는 제작 기록 기준입니다. 실제 VehicleData 반영 여부는 7단계에서 따로 확인합니다.");
	return FText::FromString(Summary);
}

// Step 7 current Final Review의 typed diff를 Before→After + 의미/영향 구조로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildFinalReviewSummary(const FCFBuilderFinalReviewResult& Review)
{
	FString Summary = TEXT("[최종 변경 상태]\n");
	if (Review.BlockingIssueCount > 0)
	{
		Summary += FString::Printf(TEXT("적용 전에 해결해야 할 문제가 %d개 있습니다. 먼저 표시된 문제를 수정하고 '현재 상태 다시 확인'을 눌러주세요.\n"), Review.BlockingIssueCount);
	}
	else if (Review.bHasExternalDrift)
	{
		Summary += TEXT("마지막 확인 이후 VehicleData가 다른 경로에서 변경되었습니다. 현재 상태를 다시 확인한 뒤 새 변경 목록을 검토하세요.\n");
	}
	else if (Review.FieldDiff.IsEmpty())
	{
		Summary += TEXT("현재 VehicleData에 추가로 적용할 변경이 없습니다.\n");
	}
	else
	{
		Summary += TEXT("VehicleData에 적용할 변경이 있습니다. 아래 내용을 확인한 뒤 적용 여부를 결정하세요.\n");
	}
	if (Review.WarningCount > 0)
	{
		Summary += FString::Printf(TEXT("참고 경고: %d개 — 적용 가능 여부와 별개로 확인을 권장합니다.\n"), Review.WarningCount);
	}
	Summary += TEXT("자동 저장하지 않습니다. 실제 적용은 아래 '변경 적용' 버튼을 승인할 때만 수행됩니다.\n");

	if (Review.FieldDiff.IsEmpty())
	{
		return FText::FromString(Summary);
	}

	Summary += TEXT("\n[변경 내용]\n");

	// Add/Remove/Move structural row가 있는 stable collection element를 먼저 찾아 child SetLeaf 중복 표시를 막습니다.
	TSet<FString> StructuralGroups;
	for (const FCFVehicleFieldDiff& Diff : Review.FieldDiff)
	{
		if (Diff.Operation != ECFVehicleDiffOp::SetLeaf && !Diff.FieldPath.CollectionPropertyName.IsNone())
		{
			StructuralGroups.Add(StableCollectionGroupKey(Diff.FieldPath));
		}
	}

	// 이미 출력한 stable collection structural group입니다.
	TSet<FString> EmittedStructuralGroups;
	int32 DisplayIndex = 0;
	for (const FCFVehicleFieldDiff& Diff : Review.FieldDiff)
	{
		const FString GroupKey = StableCollectionGroupKey(Diff.FieldPath);
		if (!GroupKey.IsEmpty() && StructuralGroups.Contains(GroupKey))
		{
			if (Diff.Operation == ECFVehicleDiffOp::SetLeaf || EmittedStructuralGroups.Contains(GroupKey))
			{
				continue;
			}

			EmittedStructuralGroups.Add(GroupKey);
			++DisplayIndex;
			const FString CollectionLabel = StableCollectionLabel(Diff.FieldPath);
			Summary += FString::Printf(TEXT("\n%d. [%s] %s\n"), DisplayIndex, ReviewOperationText(Diff.Operation), *CollectionLabel);
			if (Diff.FieldPath.CollectionPropertyName == TEXT("HardpointSlots"))
			{
				Summary += TEXT("   의미: 차량에서 무기·장비를 붙일 물리 위치입니다.\n");
				Summary += Diff.Operation == ECFVehicleDiffOp::AddArrayElement
					? TEXT("   영향: 새 장비 장착 위치와 그 Socket 기반 위치 정보가 VehicleData에 추가됩니다.\n")
					: Diff.Operation == ECFVehicleDiffOp::RemoveArrayElement
						? TEXT("   영향: 이 차량의 장비 장착 위치 정보가 VehicleData에서 제거됩니다. Chassis StaticMesh Socket 자체를 삭제하는 작업은 아닙니다.\n")
						: TEXT("   영향: 저장된 장비 장착 위치의 배열 순서가 바뀝니다.\n");
			}
			else if (Diff.FieldPath.CollectionPropertyName == TEXT("MountProfiles"))
			{
				Summary += TEXT("   의미: 특정 장착 위치에 어떤 종류와 크기의 장비를 허용할지 정하는 규칙입니다.\n");
				Summary += Diff.Operation == ECFVehicleDiffOp::AddArrayElement
					? TEXT("   영향: 새 장착 규칙이 VehicleData에 추가됩니다.\n")
					: Diff.Operation == ECFVehicleDiffOp::RemoveArrayElement
						? TEXT("   영향: 이 장착 규칙이 VehicleData에서 제거됩니다. 장착 위치나 Chassis Socket은 삭제하지 않습니다.\n")
						: TEXT("   영향: 저장된 장착 규칙의 배열 순서가 바뀝니다.\n");
			}
			else
			{
				Summary += TEXT("   의미: 차량의 목록형 설정이 구조적으로 변경됩니다.\n");
				Summary += TEXT("   영향: 세부 기술 값은 아래 진단 정보에서 확인할 수 있습니다.\n");
			}
			continue;
		}

		++DisplayIndex;
		const FCFVehicleReviewFieldDescriptor* Descriptor = FindReviewFieldDescriptor(Diff.FieldPath);
		if (Descriptor)
		{
			const FString BeforeText = Diff.bHasBeforeValue ? FormatReviewValue(Diff.BeforeValue, Descriptor->ValueFormat) : TEXT("없음");
			const FString AfterText = Diff.bHasAfterValue ? FormatReviewValue(Diff.AfterValue, Descriptor->ValueFormat) : TEXT("없음");
			Summary += FString::Printf(
				TEXT("\n%d. [%s] %s\n   현재: %s\n   변경 후: %s\n   의미: %s\n   영향: %s\n"),
				DisplayIndex,
				ReviewOperationText(Diff.Operation),
				Descriptor->UserLabel,
				*BeforeText,
				*AfterText,
				Descriptor->Meaning,
				Descriptor->EffectSummary);
			continue;
		}

		if (!Diff.FieldPath.CollectionPropertyName.IsNone())
		{
			const FString LeafLabel = StableCollectionLeafLabel(Diff.FieldPath);
			if (!LeafLabel.IsEmpty())
			{
				const FString CollectionLabel = StableCollectionLabel(Diff.FieldPath);
				Summary += FString::Printf(TEXT("\n%d. [%s] %s — %s\n"), DisplayIndex, ReviewOperationText(Diff.Operation), *CollectionLabel, *LeafLabel);
				if (Diff.bHasBeforeValue && Diff.bHasAfterValue
					&& Diff.BeforeValue.CanonicalValueText.Len() <= 80
					&& Diff.AfterValue.CanonicalValueText.Len() <= 80)
				{
					Summary += FString::Printf(TEXT("   현재: %s\n   변경 후: %s\n"), *Diff.BeforeValue.CanonicalValueText, *Diff.AfterValue.CanonicalValueText);
				}
				Summary += TEXT("   의미: 장비 장착 위치 또는 장착 규칙의 실제 설정값입니다.\n");
				Summary += TEXT("   영향: 이 장착 위치에서 사용할 수 있는 장비 또는 실제 부착 위치에 영향을 줍니다.\n");
				continue;
			}
		}

		Summary += FString::Printf(TEXT("\n%d. [%s] 기타 차량 설정 변경\n"), DisplayIndex, ReviewOperationText(Diff.Operation));
		Summary += TEXT("   설명되지 않은 차량 설정이 변경됩니다.\n");
		Summary += TEXT("   세부 기술 경로와 값은 '진단 정보'에서 확인할 수 있습니다.\n");
	}
	return FText::FromString(Summary);
}

// Step 7 current Target과 fresh Final Review prospective result를 분리해 표시합니다.
FText FCFVehicleBuilderPresentation::BuildFinalReviewHardpointReadback(const FCFBuilderHardpointSemanticFacts& Facts)
{
	if (!Facts.bAvailable || !Facts.bHasCurrentTarget)
	{
		return FText::FromString(TEXT("[장착 데이터 확인]\n현재 VehicleData의 장착 구성을 읽을 수 없습니다."));
	}

	FString Summary = TEXT("[장착 데이터 확인]\n");
	Summary += FString::Printf(
		TEXT("현재 VehicleData: 장착 위치 %d / 장착 규칙 %d\n"),
		Facts.CurrentTargetHardpointCount,
		Facts.CurrentTargetMountCount);
	if (Facts.bHasProspectiveTargetCounts)
	{
		Summary += FString::Printf(
			TEXT("변경 적용 후: 장착 위치 %d / 장착 규칙 %d\n"),
			Facts.ProspectiveTargetHardpointCount,
			Facts.ProspectiveTargetMountCount);
	}
	else
	{
		Summary += TEXT("변경 적용 후 장착 구성은 현재 fresh Final Review에서 계산할 수 없습니다.\n");
	}
	Summary += TEXT("위 두 줄은 현재 값과 적용 예정 값을 구분한 것입니다. 아직 '변경 적용'을 누르지 않았다면 적용 후 값은 VehicleData에 저장된 값이 아닙니다.");
	return FText::FromString(Summary);
}

// Step 8 saved-state/benchmark/checklist를 USER 판단용 정보로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildDrivingSummary(const FCFVehicleDrivingPresentInfo& Info)
{
	FString Summary = TEXT("[1. 저장 상태]\n");
	if (Info.bRecipeDirty || Info.bTargetDirty)
	{
		Summary += TEXT("기술 주행 측정 전에 저장이 필요합니다.\n");
		if (Info.bRecipeDirty) Summary += TEXT("- 차량 제작 기록에 저장되지 않은 변경이 있습니다.\n");
		if (Info.bTargetDirty) Summary += TEXT("- VehicleData에 저장되지 않은 변경이 있습니다.\n");
		Summary += TEXT("필요한 Asset을 직접 저장한 뒤 '현재 상태 다시 확인'을 눌러주세요. Builder는 자동 저장하지 않습니다.\n");
	}
	else
	{
		Summary += TEXT("현재 차량 제작 기록과 VehicleData에 저장되지 않은 변경이 없습니다.\n");
	}

	Summary += TEXT("\n[2. 기술 주행 측정]\n");
	if (!Info.bHasBenchmarkResult || !Info.BenchmarkResult.bHasMetric)
	{
		Summary += TEXT("현재 차량에 유효한 기술 주행 측정 결과가 없습니다. 저장 상태를 확인한 뒤 '기술 주행 측정 실행'을 눌러주세요.\n");
	}
	else
	{
		const FCFVehicleBuilderDrivingMetric& Metric = Info.BenchmarkResult.Metric;
		const FString Accel50Text = Metric.Acceleration0To50Seconds >= 0.0 ? FString::Printf(TEXT("%.3f초"), Metric.Acceleration0To50Seconds) : TEXT("미도달");
		const FString Accel100Text = Metric.Acceleration0To100Seconds >= 0.0 ? FString::Printf(TEXT("%.3f초"), Metric.Acceleration0To100Seconds) : TEXT("미도달");
		const FString BrakingText = Metric.bBraking100Available
			? FString::Printf(TEXT("%.3f초 / %.2f m"), Metric.Braking100ToIdleSeconds, Metric.Braking100ToIdleDistanceMeters)
			: TEXT("100 km/h 미도달로 미측정");
		const FString TurningRadiusText = Metric.EffectiveTurningRadiusMeters >= 0.0
			? FString::Printf(TEXT("%.2f m"), Metric.EffectiveTurningRadiusMeters)
			: TEXT("미측정");

		Summary += FString::Printf(TEXT("0→50 km/h: %s\n"), *Accel50Text);
		Summary += FString::Printf(TEXT("0→100 km/h: %s\n"), *Accel100Text);
		Summary += FString::Printf(TEXT("관측 최고 속도: %.1f km/h\n"), Metric.PeakSpeedKmh);
		Summary += FString::Printf(TEXT("최고 RPM / 당시 기어: %.0f RPM / %d단\n"), Metric.PeakEngineRpm, Metric.PeakSpeedGear);
		Summary += FString::Printf(TEXT("100 km/h 부근→저속 제동: %s\n"), *BrakingText);
		Summary += FString::Printf(TEXT("회전반경: %s\n"), *TurningRadiusText);
		Summary += FString::Printf(TEXT("회전 측정 평균 속도: %.1f km/h\n"), Metric.TurningAverageSpeedKmh);
		Summary += TEXT("\n※ 위 수치는 차량 상태를 관측한 측정값입니다. 자동 합격/불합격 기준이 아니며 실제 주행 판단을 돕는 참고 자료입니다.\n");
	}

	Summary += TEXT("\n[3. 직접 주행]\n");
	Summary += Info.bUserDrivePrepared
		? TEXT("현재 PIE에 선택 차량이 적용되어 직접 주행을 확인할 수 있습니다.\n")
		: TEXT("측정 결과가 준비되면 PIE를 시작하고 '현재 PIE에 선택 차량 적용'을 눌러 직접 주행하세요.\n");
	Summary += TEXT("\n직접 확인할 항목:\n");
	Summary += TEXT("□ 출발과 가속 반응이 의도한 차량 성격에 어울리는가\n");
	Summary += TEXT("□ 변속 시점과 변속 후 RPM 회복이 자연스러운가\n");
	Summary += TEXT("□ 조향 반응과 회전반경이 차량 크기와 성격에 어울리는가\n");
	Summary += TEXT("□ 브레이크와 핸드브레이크가 정상적으로 제어되는가\n");
	Summary += TEXT("□ 고속에서 RPM·변속·차체 반응에 명백한 이상이 없는가\n");
	Summary += TEXT("□ 바퀴 떨림·과도한 튀김·차체 물리 이상이 플레이를 방해하지 않는가\n");
	Summary += TEXT("이 체크리스트는 화면 안내이며 항목별 체크 상태를 Asset에 저장하지 않습니다.\n");

	Summary += TEXT("\n[4. 사용자 주행 판정]\n");
	Summary += Info.bUserDrivingAccepted
		? TEXT("현재 VehicleData 상태에 대한 사용자 주행 확인이 완료되었습니다.\n")
		: TEXT("직접 주행을 마친 뒤 이상이 없다고 판단하면 '주행 테스트 통과'를 누르세요.\n");
	return FText::FromString(Summary);
}

// Step 8 PIE Apply preflight blocker를 raw hash/path 없이 USER recovery 문장으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(
	const FCFVehicleDrivingApplyPreflight& Preflight,
	const bool bBenchmarkRunning)
{
	if (bBenchmarkRunning)
	{
		return FText::FromString(TEXT("기술 주행 측정이 진행 중입니다. 측정 완료를 기다리면 현재 차량 적용 준비 상태를 다시 확인합니다."));
	}

	switch (Preflight.Blocker)
	{
	case ECFVehicleDrivingApplyBlocker::None:
		return FText::FromString(TEXT("현재 차량은 플레이 적용 준비가 완료됐습니다. Play가 꺼져 있다면 플레이를 시작한 뒤 '현재 PIE에 선택 차량 적용'을 누르세요."));
	case ECFVehicleDrivingApplyBlocker::BenchmarkUnavailable:
		return FText::FromString(TEXT("현재 저장된 차량의 기술 주행 측정 결과가 없습니다. 먼저 '기술 주행 측정 실행'을 완료하세요."));
	case ECFVehicleDrivingApplyBlocker::BenchmarkStale:
		return FText::FromString(TEXT("기술 주행 측정 결과가 현재 저장된 차량 상태와 다릅니다. '현재 상태 다시 확인' 후 기술 주행 측정을 다시 실행하세요."));
	case ECFVehicleDrivingApplyBlocker::RecipeUnavailable:
		return FText::FromString(TEXT("현재 차량의 제작 기록을 읽을 수 없습니다. 차량을 다시 선택하고 '현재 상태 다시 확인'을 눌러주세요."));
	case ECFVehicleDrivingApplyBlocker::TargetUnavailable:
		return FText::FromString(TEXT("현재 차량 데이터를 읽을 수 없습니다. 차량을 다시 선택하고 '현재 상태 다시 확인'을 눌러주세요."));
	case ECFVehicleDrivingApplyBlocker::RecipeUnsaved:
		return FText::FromString(TEXT("차량 제작 기록에 저장되지 않은 변경이 있습니다. Recipe Asset을 직접 저장한 뒤 다시 확인하세요. 차량 제작 가이드는 자동 저장하지 않습니다."));
	case ECFVehicleDrivingApplyBlocker::TargetUnsaved:
		return FText::FromString(TEXT("VehicleData에 저장되지 않은 변경이 있습니다. VehicleData Asset을 직접 저장한 뒤 다시 확인하세요. 차량 제작 가이드는 자동 저장하지 않습니다."));
	case ECFVehicleDrivingApplyBlocker::RecipeNotPersisted:
		return FText::FromString(TEXT("차량 제작 기록이 아직 디스크에 저장되지 않았습니다. Recipe Asset을 먼저 저장하세요."));
	case ECFVehicleDrivingApplyBlocker::TargetNotPersisted:
		return FText::FromString(TEXT("VehicleData가 아직 디스크에 저장되지 않았습니다. VehicleData Asset을 먼저 저장하세요."));
	case ECFVehicleDrivingApplyBlocker::AppliedStateStale:
		return FText::FromString(TEXT("현재 VehicleData와 마지막 최종 적용 기록이 다릅니다. 7단계에서 현재 변경을 다시 검토·적용하고 관련 Asset을 저장하세요."));
	case ECFVehicleDrivingApplyBlocker::StateReadFailed:
	default:
		return FText::FromString(TEXT("현재 차량 적용 준비 상태를 정확히 읽지 못했습니다. '현재 상태 다시 확인'을 누른 뒤 계속되면 '진단 정보'를 확인하세요."));
	}
}

// Running benchmark의 cached coarse phase와 monotonic elapsed seconds를 가짜 ETA 없이 USER 문장으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildDrivingBenchmarkProgress(
	const bool bRunning,
	const bool bHasProgress,
	const FCFVehicleBuilderBenchmarkProgress& Progress,
	const double ElapsedSeconds)
{
	if (!bRunning)
	{
		return FText::GetEmpty();
	}

	// 음수가 들어오더라도 USER 표시에서 0초 미만으로 내려가지 않는 경과시간입니다.
	const int32 ElapsedWholeSeconds = FMath::Max(0, FMath::FloorToInt(ElapsedSeconds));
	if (!bHasProgress || Progress.Phase == ECFVehicleBuilderBenchmarkProgressPhase::None)
	{
		return FText::FromString(FString::Printf(
			TEXT("기술 주행 측정 중 · 상세 단계 확인 대기 · 경과 %d초\n완료되면 결과가 자동으로 갱신됩니다."),
			ElapsedWholeSeconds));
	}

	// Typed coarse phase를 초보자가 이해할 수 있는 짧은 단계명으로 변환합니다.
	const TCHAR* PhaseLabel = TEXT("진행 중");
	switch (Progress.Phase)
	{
	case ECFVehicleBuilderBenchmarkProgressPhase::Preparing: PhaseLabel = TEXT("준비"); break;
	case ECFVehicleBuilderBenchmarkProgressPhase::Acceleration: PhaseLabel = TEXT("가속"); break;
	case ECFVehicleBuilderBenchmarkProgressPhase::TopSpeed: PhaseLabel = TEXT("최고속도"); break;
	case ECFVehicleBuilderBenchmarkProgressPhase::Braking: PhaseLabel = TEXT("제동"); break;
	case ECFVehicleBuilderBenchmarkProgressPhase::Steering: PhaseLabel = TEXT("조향"); break;
	case ECFVehicleBuilderBenchmarkProgressPhase::TurningRadius: PhaseLabel = TEXT("회전반경"); break;
	case ECFVehicleBuilderBenchmarkProgressPhase::Finalizing: PhaseLabel = TEXT("결과 정리"); break;
	default: break;
	}

	return FText::FromString(FString::Printf(
		TEXT("기술 주행 측정 중 · %d/%d %s · 경과 %d초\n완료되면 결과가 자동으로 갱신됩니다."),
		Progress.PhaseIndex,
		Progress.PhaseCount,
		PhaseLabel,
		ElapsedWholeSeconds));
}

// Current persistent acceptance receipt/Recipe dirty preflight를 exact Save scope가 드러나는 USER 문장으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildDrivingRecipeSaveStatus(const FCFVehicleRecipeSavePreflight& Preflight)
{
	if (Preflight.Blocker == ECFVehicleRecipeSaveBlocker::None)
	{
		if (!Preflight.bRecipeDirty)
		{
			return FText::FromString(TEXT("주행 테스트 통과 기록됨\n차량 제작 기록: 저장 완료 또는 저장할 변경 없음."));
		}

		return FText::FromString(TEXT(
			"주행 테스트 통과 기록됨\n"
			"차량 제작 기록: 저장 필요\n"
			"현재 차량의 제작 데이터(Recipe) 1개에 있는 미저장 변경 전체를 저장합니다.\n"
			"VehicleData, StaticMesh, 데모 차량 목록 등 다른 Asset은 저장하지 않습니다."));
	}

	switch (Preflight.Blocker)
	{
	case ECFVehicleRecipeSaveBlocker::AcceptanceUnavailableOrStale:
		return FText::FromString(TEXT("현재 차량의 주행 통과 기록을 저장할 수 없습니다. 현재 VehicleData 상태에 대한 주행 테스트 통과를 다시 확인하세요."));
	case ECFVehicleRecipeSaveBlocker::RecipeUnavailable:
		return FText::FromString(TEXT("현재 차량의 제작 데이터(Recipe)를 읽을 수 없습니다. 차량을 다시 선택하고 현재 상태를 다시 확인하세요."));
	case ECFVehicleRecipeSaveBlocker::SelectionMismatch:
		return FText::FromString(TEXT("화면에서 선택한 차량과 저장 대상 제작 데이터가 다릅니다. 차량을 다시 선택한 뒤 현재 상태를 다시 확인하세요."));
	case ECFVehicleRecipeSaveBlocker::TargetUnavailable:
		return FText::FromString(TEXT("현재 VehicleData를 읽을 수 없어 주행 통과 기록의 저장 대상을 확인할 수 없습니다."));
	case ECFVehicleRecipeSaveBlocker::PackageInvalid:
		return FText::FromString(TEXT("현재 제작 데이터가 일반 게임 Asset으로 저장할 수 있는 상태가 아닙니다. 진단 정보를 확인하세요."));
	case ECFVehicleRecipeSaveBlocker::StateReadFailed:
	default:
		return FText::FromString(TEXT("차량 제작 기록 저장 상태를 정확히 읽지 못했습니다. 현재 상태를 다시 확인한 뒤 계속되면 진단 정보를 확인하세요."));
	}
}

// Exact current Recipe Save terminal outcome을 USER success/warning/recovery 문장으로 변환합니다.
FText FCFVehicleBuilderPresentation::BuildDrivingRecipeSaveResult(const FCFVehicleRecipeSaveResult& Result)
{
	switch (Result.Outcome)
	{
	case ECFVehicleRecipeSaveOutcome::Saved:
		return FText::FromString(TEXT("차량 제작 기록 저장 완료. 현재 차량의 Recipe Asset 1개만 저장했습니다."));
	case ECFVehicleRecipeSaveOutcome::NoSaveNeeded:
		return FText::FromString(TEXT("차량 제작 기록에 새로 저장할 변경이 없습니다."));
	case ECFVehicleRecipeSaveOutcome::SavedRefreshWarning:
		return FText::FromString(TEXT("차량 제작 기록 저장은 완료됐지만 화면 상태를 다시 읽지 못했습니다. '현재 상태 다시 확인'을 한 번 눌러주세요."));
	case ECFVehicleRecipeSaveOutcome::SaveStateUnconfirmed:
		return FText::FromString(TEXT("저장 동작은 완료됐지만 Recipe가 아직 미저장 상태로 표시됩니다. 자동 재시도하지 않았습니다. 진단 정보를 확인하세요."));
	case ECFVehicleRecipeSaveOutcome::SaveFailed:
		return FText::FromString(TEXT("차량 제작 기록 저장에 실패했습니다. 다른 Asset은 저장하지 않았습니다. 진단 정보를 확인하세요."));
	case ECFVehicleRecipeSaveOutcome::Blocked:
	default:
		return FText::FromString(TEXT("현재 상태에서는 차량 제작 기록을 저장할 수 없습니다. 저장 상태 안내와 진단 정보를 확인하세요."));
	}
}

// Step 8 current Target VehicleData의 실제 Hardpoint/Mount readback만 표시합니다.
FText FCFVehicleBuilderPresentation::BuildDrivingHardpointReadback(const FCFBuilderHardpointSemanticFacts& Facts)
{
	if (!Facts.bAvailable || !Facts.bHasCurrentTarget)
	{
		return FText::FromString(TEXT("[현재 차량의 장착 구성]\nVehicleData 장착 구성을 읽을 수 없습니다."));
	}

	return FText::FromString(FString::Printf(
		TEXT("[현재 차량의 장착 구성]\n장착 위치 %d / 장착 규칙 %d\n이 값은 현재 VehicleData에서 읽은 값입니다. 제작 기록의 예정 개수나 아직 적용하지 않은 변경 수가 아닙니다."),
		Facts.CurrentTargetHardpointCount,
		Facts.CurrentTargetMountCount));
}
