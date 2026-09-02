// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleEngineCurveUtils.cpp
// Version: v1.1.0
// Date: 2026-09-01
// Description: CarFight typed Engine Torque Curve의 runtime validation 및 UE 5.8 Chaos curve 변환 구현입니다.
// Changelog:
// - v1.1.0: current/proposed Chaos TorqueCurve key shape equality helper를 추가해 runtime hot-apply의 필요 PhysicsState 재생성만 판정.
// - v1.0.0: strictly increasing RPM + normalized 0..1 multiplier 검증, ExternalCurve 해제, RichCurve linear-key 재구성을 최초 구현.

#include "CFVehicleEngineCurveUtils.h"

#include "CFVehicleData.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Curves/RichCurve.h"

// RPM은 엄격 증가하고 TorqueMultiplier는 0..1인 최소 2개 point인지 검증합니다.
bool FCFVehicleEngineCurveUtils::ValidateCurve(const FCFVehicleEngineTorqueCurve& Curve, FString& OutError)
{
	OutError.Reset();

	if (Curve.Points.Num() < 2)
	{
		OutError = TEXT("Engine Torque Curve는 최소 2개 point가 필요합니다.");
		return false;
	}

	float PreviousRpm = -1.0f;
	for (int32 PointIndex = 0; PointIndex < Curve.Points.Num(); ++PointIndex)
	{
		const FCFVehicleEngineTorquePoint& Point = Curve.Points[PointIndex];
		if (!FMath::IsFinite(Point.EngineRPM) || Point.EngineRPM < 0.0f)
		{
			OutError = FString::Printf(TEXT("Engine Torque Curve point %d의 RPM이 유효하지 않습니다."), PointIndex);
			return false;
		}
		if (!FMath::IsFinite(Point.TorqueMultiplier) || Point.TorqueMultiplier < 0.0f || Point.TorqueMultiplier > 1.0f)
		{
			OutError = FString::Printf(TEXT("Engine Torque Curve point %d의 TorqueMultiplier가 0..1 범위를 벗어났습니다."), PointIndex);
			return false;
		}
		if (PointIndex > 0 && Point.EngineRPM <= PreviousRpm)
		{
			OutError = FString::Printf(TEXT("Engine Torque Curve RPM은 엄격히 증가해야 합니다. point=%d"), PointIndex);
			return false;
		}
		PreviousRpm = Point.EngineRPM;
	}

	return true;
}

// 검증된 CarFight typed curve를 Chaos EngineSetup TorqueCurve에 exact 적용합니다.
bool FCFVehicleEngineCurveUtils::ApplyCurveToChaos(const FCFVehicleEngineTorqueCurve& Curve, FVehicleEngineConfig& InOutEngineConfig, FString& OutError)
{
	if (!ValidateCurve(Curve, OutError))
	{
		return false;
	}

	// 외부 Curve Asset이 남아 있으면 internal EditorCurveData보다 우선될 수 있으므로 차량별 typed payload 적용 시 명시적으로 해제합니다.
	InOutEngineConfig.TorqueCurve.ExternalCurve = nullptr;

	// UE 5.8 FVehicleEngineConfig TorqueCurve는 X=RPM, Y=normalized torque 0..1 계약입니다.
	FRichCurve* RuntimeCurve = InOutEngineConfig.TorqueCurve.GetRichCurve();
	if (!RuntimeCurve)
	{
		OutError = TEXT("Chaos EngineSetup.TorqueCurve RichCurve를 가져올 수 없습니다.");
		return false;
	}

	RuntimeCurve->Reset();
	RuntimeCurve->ReserveKeys(Curve.Points.Num());
	for (const FCFVehicleEngineTorquePoint& Point : Curve.Points)
	{
		const FKeyHandle KeyHandle = RuntimeCurve->AddKey(Point.EngineRPM, Point.TorqueMultiplier);
		RuntimeCurve->SetKeyInterpMode(KeyHandle, ERichCurveInterpMode::RCIM_Linear, false);
	}

	return true;
}

// 두 Chaos EngineSetup의 TorqueCurve key 시간/값/보간이 동일한지 비교합니다.
bool FCFVehicleEngineCurveUtils::AreTorqueCurvesEquivalent(const FVehicleEngineConfig& LeftEngineConfig, const FVehicleEngineConfig& RightEngineConfig)
{
	const FRichCurve* LeftCurve = LeftEngineConfig.TorqueCurve.GetRichCurveConst();
	const FRichCurve* RightCurve = RightEngineConfig.TorqueCurve.GetRichCurveConst();
	if (LeftCurve == RightCurve)
	{
		return true;
	}
	if (!LeftCurve || !RightCurve)
	{
		return false;
	}

	const TArray<FRichCurveKey>& LeftKeys = LeftCurve->GetConstRefOfKeys();
	const TArray<FRichCurveKey>& RightKeys = RightCurve->GetConstRefOfKeys();
	if (LeftKeys.Num() != RightKeys.Num())
	{
		return false;
	}

	for (int32 KeyIndex = 0; KeyIndex < LeftKeys.Num(); ++KeyIndex)
	{
		const FRichCurveKey& LeftKey = LeftKeys[KeyIndex];
		const FRichCurveKey& RightKey = RightKeys[KeyIndex];
		if (!FMath::IsNearlyEqual(LeftKey.Time, RightKey.Time)
			|| !FMath::IsNearlyEqual(LeftKey.Value, RightKey.Value)
			|| LeftKey.InterpMode != RightKey.InterpMode)
		{
			return false;
		}
	}

	return true;
}
