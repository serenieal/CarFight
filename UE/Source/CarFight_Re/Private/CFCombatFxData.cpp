// Copyright (c) CarFight. All Rights Reserved.
// Version: 1.2.0
// Date: 2026-07-25

#include "CFCombatFxData.h"

#include "NiagaraSystem.h"

bool UCFCombatFxData::IsCombatFxConfigured() const
{
	return IsValid(NiagaraSystem.Get());
}

FString UCFCombatFxData::BuildCombatFxSummary() const
{
	const FString NiagaraSystemName = NiagaraSystem ? NiagaraSystem->GetName() : TEXT("MissingOptional");
	const TCHAR* ScaleModeText = FxScaleMode == ECFCombatFxScaleMode::NiagaraUserVector
		? TEXT("NiagaraUserVector")
		: TEXT("ComponentTransform");

	return FString::Printf(
		TEXT("CombatFxData: Id=%s, Niagara=%s, Scale=(%.2f, %.2f, %.2f), ScaleMode=%s, UserScaleParameter=%s, RotationOffset=(P=%.1f, Y=%.1f, R=%.1f), MaximumLifetime=%.2fs"),
		*CombatFxId.ToString(),
		*NiagaraSystemName,
		FxScale.X,
		FxScale.Y,
		FxScale.Z,
		ScaleModeText,
		*NiagaraUserScaleParameterName.ToString(),
		RotationOffset.Pitch,
		RotationOffset.Yaw,
		RotationOffset.Roll,
		MaximumLifetimeSeconds);
}
