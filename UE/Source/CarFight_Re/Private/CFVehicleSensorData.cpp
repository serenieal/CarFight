// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-15
// Description: CF-FQ-036 차량 Sensor 설정 DataAsset 구현 / SEN-P0-02 bounded Passive 설정 검증
// Scope: SensorConfig 검증, 오류 보고와 디버그 요약을 구현합니다.
// Changelog:
// - v1.1.0: Active Scan 0=비활성 계약과 MaxActorScansPerUpdate 1~4096 검증을 추가.
// - v1.0.0: SEN-P0-01 DataValidation과 설정 요약을 최초 구현.
// Migration:
// - 이 구현은 Content Asset을 생성하거나 변경하지 않습니다.

#include "CFVehicleSensorData.h"

#define LOCTEXT_NAMESPACE "CFVehicleSensorData"

// [v1.0.0] 현재 SensorConfig가 유한하고 허용 범위 안에 있는지 반환합니다.
bool UCFVehicleSensorData::IsSensorConfigValid() const
{
	return SensorConfig.IsValid();
}

// [v1.0.0] DataValidation과 Automation이 공유할 SensorData 계약 오류 목록을 생성합니다.
bool UCFVehicleSensorData::ValidateSensorDataContract(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (!FMath::IsFinite(SensorConfig.PassiveDetectionRangeCm) || SensorConfig.PassiveDetectionRangeCm < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidPassiveRange", "PassiveDetectionRangeCm은 유한한 0 이상의 값이어야 합니다."));
	}

		if (!FMath::IsFinite(SensorConfig.ActiveScanRangeCm)
		|| SensorConfig.ActiveScanRangeCm < 0.0f
		|| (SensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER
			&& SensorConfig.ActiveScanRangeCm < SensorConfig.PassiveDetectionRangeCm))
	{
		OutValidationErrors.Add(LOCTEXT("InvalidActiveRange", "ActiveScanRangeCm은 0으로 비활성화하거나, 활성 값이면 PassiveDetectionRangeCm 이상이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.VisualDetectionRangeCm) || SensorConfig.VisualDetectionRangeCm < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidVisualRange", "VisualDetectionRangeCm은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.UpdateIntervalSec) || SensorConfig.UpdateIntervalSec <= 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidUpdateInterval", "UpdateIntervalSec은 유한한 0 초과 값이어야 합니다."));
	}

		if (SensorConfig.MaxActorScansPerUpdate < 1 || SensorConfig.MaxActorScansPerUpdate > 4096)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidActorScanBudget", "MaxActorScansPerUpdate는 1~4096 범위여야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.ContactMemoryTimeSec) || SensorConfig.ContactMemoryTimeSec < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidContactMemory", "ContactMemoryTimeSec은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.DestroyedHoldTimeSec) || SensorConfig.DestroyedHoldTimeSec < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidDestroyedHold", "DestroyedHoldTimeSec은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.ActiveScanDurationSec) || SensorConfig.ActiveScanDurationSec < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidActiveDuration", "ActiveScanDurationSec은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.AnalysisGainPerSec) || SensorConfig.AnalysisGainPerSec < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidAnalysisGain", "AnalysisGainPerSec은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.AnalysisDecayPerSec) || SensorConfig.AnalysisDecayPerSec < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidAnalysisDecay", "AnalysisDecayPerSec은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.IdentifiedThreshold)
		|| SensorConfig.IdentifiedThreshold < 0.0f
		|| SensorConfig.IdentifiedThreshold > 1.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidIdentifiedThreshold", "IdentifiedThreshold는 0~1 범위의 유한한 값이어야 합니다."));
	}

	if (!FMath::IsFinite(SensorConfig.DetailedScanThreshold)
		|| SensorConfig.DetailedScanThreshold <= SensorConfig.IdentifiedThreshold
		|| SensorConfig.DetailedScanThreshold > 1.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidDetailedThreshold", "DetailedScanThreshold는 IdentifiedThreshold보다 크고 1 이하여야 합니다."));
	}

	return OutValidationErrors.IsEmpty();
}

// [v1.0.0] 디버그와 로그에 사용할 SensorData 설정 요약 문자열을 생성합니다.
FString UCFVehicleSensorData::BuildSensorDataSummary() const
{
	return FString::Printf(TEXT("VehicleSensorData: %s"), *SensorConfig.BuildDebugSummary());
}

#if WITH_EDITOR
// [v1.0.0] Unreal Data Validation에서 잘못된 Sensor 설정을 보고합니다.
EDataValidationResult UCFVehicleSensorData::IsDataValid(FDataValidationContext& Context) const
{
	// [v1.0.0] 상위 PrimaryDataAsset의 기본 검증 결과입니다.
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);

	// [v1.0.0] SensorData 계약에서 발견된 오류 목록입니다.
	TArray<FText> ValidationErrors;
	if (!ValidateSensorDataContract(ValidationErrors))
	{
		for (const FText& ValidationError : ValidationErrors)
		{
			Context.AddError(ValidationError);
		}

		return EDataValidationResult::Invalid;
	}

	if (ValidationResult == EDataValidationResult::NotValidated)
	{
		ValidationResult = EDataValidationResult::Valid;
	}

	return ValidationResult;
}
#endif

#undef LOCTEXT_NAMESPACE
