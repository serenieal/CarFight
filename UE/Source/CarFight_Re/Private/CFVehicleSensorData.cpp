// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-20
// Description: CF-FQ-036 Sensor 설정 + CF-FQ-032 UI-P0-08 Radar 표시 Range Profile 검증 구현
// Scope: SensorConfig와 Scanner 소유 Radar Range Preset의 전체 DataAsset 계약, 오류 보고와 디버그 요약을 구현합니다.
// Changelog:
// - v1.2.0: RadarDisplayRangePresetsCm의 finite/positive/strict-ascending/ActiveScanRange 상한과 명시 Default index 검증을 추가. 빈 Profile은 기존 동작 호환으로 허용.
// - v1.1.0: Active Scan 0=비활성 계약과 MaxActorScansPerUpdate 1~4096 검증을 추가.
// - v1.0.0: SEN-P0-01 DataValidation과 설정 요약을 최초 구현.
// Migration:
// - 기존 Content의 빈 Radar Range Profile은 유효하며 Radar Range/Zoom을 활성화하지 않습니다.
// - 이 구현은 Content Asset을 생성하거나 변경하지 않습니다.

#include "CFVehicleSensorData.h"

#define LOCTEXT_NAMESPACE "CFVehicleSensorData"

// [v1.0.0] 현재 SensorConfig가 유한하고 허용 범위 안에 있는지 반환합니다.
bool UCFVehicleSensorData::IsSensorConfigValid() const
{
	return SensorConfig.IsValid();
}

// [v1.2.0] SensorConfig와 Radar 표시 Range Profile을 합친 전체 DataAsset 계약이 유효한지 반환합니다.
bool UCFVehicleSensorData::IsSensorDataContractValid() const
{
	// [v1.2.0] 전체 계약 검증에서 발견된 오류를 일시 보관할 목록입니다.
	TArray<FText> ValidationErrors;
	return ValidateSensorDataContract(ValidationErrors);
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

	if (RadarDisplayRangePresetsCm.IsEmpty())
	{
		if (DefaultRadarDisplayRangePresetIndex != INDEX_NONE)
		{
			OutValidationErrors.Add(LOCTEXT("InvalidEmptyRadarRangeDefault", "RadarDisplayRangePresetsCm이 비어 있으면 DefaultRadarDisplayRangePresetIndex는 -1이어야 합니다."));
		}
	}
	else
	{
		if (!FMath::IsFinite(SensorConfig.ActiveScanRangeCm) || SensorConfig.ActiveScanRangeCm <= KINDA_SMALL_NUMBER)
		{
			OutValidationErrors.Add(LOCTEXT("RadarRangeRequiresActiveScan", "Radar 표시 Range Preset을 사용하려면 ActiveScanRangeCm이 0보다 커야 합니다."));
		}

						if (!RadarDisplayRangePresetsCm.IsValidIndex(DefaultRadarDisplayRangePresetIndex))
		{
			OutValidationErrors.Add(LOCTEXT("InvalidRadarRangeDefaultIndex", "DefaultRadarDisplayRangePresetIndex는 RadarDisplayRangePresetsCm의 유효한 인덱스여야 합니다."));
		}

		// [v1.2.0] strict ascending 검증에서 직전 Radar 표시 범위를 보존할 값입니다.
		float PreviousRadarDisplayRangeCm = -1.0f;
		// [v1.2.0] Radar Range Preset을 작은 값부터 순회할 0-based 인덱스입니다.
		for (int32 RadarRangeIndex = 0; RadarRangeIndex < RadarDisplayRangePresetsCm.Num(); ++RadarRangeIndex)
		{
			// [v1.2.0] Scanner Profile이 현재 단계에 명시한 Radar 표시 거리 cm 값입니다.
			const float RadarDisplayRangeCm = RadarDisplayRangePresetsCm[RadarRangeIndex];
			if (!FMath::IsFinite(RadarDisplayRangeCm) || RadarDisplayRangeCm <= KINDA_SMALL_NUMBER)
			{
				OutValidationErrors.Add(FText::Format(
					LOCTEXT("InvalidRadarRangeValue", "RadarDisplayRangePresetsCm[{0}]은 유한한 0 초과 값이어야 합니다."),
					FText::AsNumber(RadarRangeIndex)));
				continue;
			}

			if (RadarRangeIndex > 0 && RadarDisplayRangeCm <= PreviousRadarDisplayRangeCm + KINDA_SMALL_NUMBER)
			{
				OutValidationErrors.Add(FText::Format(
					LOCTEXT("InvalidRadarRangeOrder", "RadarDisplayRangePresetsCm[{0}]은 앞 Preset보다 반드시 커야 합니다."),
					FText::AsNumber(RadarRangeIndex)));
			}

			if (FMath::IsFinite(SensorConfig.ActiveScanRangeCm)
				&& SensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER
				&& RadarDisplayRangeCm > SensorConfig.ActiveScanRangeCm + KINDA_SMALL_NUMBER)
			{
				OutValidationErrors.Add(FText::Format(
					LOCTEXT("RadarRangeExceedsActiveScan", "RadarDisplayRangePresetsCm[{0}]은 ActiveScanRangeCm을 넘을 수 없습니다."),
					FText::AsNumber(RadarRangeIndex)));
			}

			PreviousRadarDisplayRangeCm = RadarDisplayRangeCm;
		}
	}

	return OutValidationErrors.IsEmpty();
}

// [v1.0.0] 디버그와 로그에 사용할 SensorData 설정 요약 문자열을 생성합니다.
FString UCFVehicleSensorData::BuildSensorDataSummary() const
{
	return FString::Printf(
		TEXT("VehicleSensorData: %s RadarRanges=%d DefaultRadarRangeIndex=%d"),
		*SensorConfig.BuildDebugSummary(),
		RadarDisplayRangePresetsCm.Num(),
		DefaultRadarDisplayRangePresetIndex);
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
