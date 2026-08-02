// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-01
// Description: CarFight 차량 쉴드·방향별 장갑 DataAsset과 피팅 방어 질량 구현
// Scope: P0 기본값, 피팅 방어 질량, 안전 Getter, 디버그 요약과 DataValidation 계약을 구현합니다.
// Changelog:
// - v1.1.0: CF-FQ-034 FIT-P0-02 유효 방어 질량 Getter, 요약과 음수·비유한 값 검증을 추가.
// - v1.0.0: CF-FQ-033 DR-P0-01 VehicleDefenseData Foundation 구현.
// Migration:
// - 기존 VehicleDefenseData는 DefenseMassKg=0 기본값으로 기존 피해와 차량 주행 결과를 유지한다.
// - 기존 VehicleData는 DefaultDefenseData가 비어 있으므로 이 코드 추가만으로 런타임 피해 결과가 변경되지 않는다.
// - 잘못된 음수 값은 DataValidation에서 오류로 보고하고 안전 Getter에서는 0으로 보정한다.

#include "CFVehicleDefenseData.h"

#define LOCTEXT_NAMESPACE "CFVehicleDefenseData"

namespace
{
	// [v1.0.0] 한 방향 장갑 설정의 음수 값을 데이터 계약 오류로 추가합니다.
	void ValidateDirectionalArmorConfig(
		const FText& DirectionDisplayName,
		const FCFDirectionalArmorConfig& ArmorConfig,
		TArray<FText>& OutValidationErrors)
	{
		if (ArmorConfig.MaximumArmor < 0.0f)
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("NegativeMaximumArmor", "{0}의 MaximumArmor는 0 이상이어야 합니다."),
				DirectionDisplayName));
		}

		if (ArmorConfig.DamageMultiplier < 0.0f)
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("NegativeDamageMultiplier", "{0}의 DamageMultiplier는 0 이상이어야 합니다."),
				DirectionDisplayName));
		}
	}
}

// [v1.0.0] P0 테스트 기준 쉴드·장갑 기본값을 초기화합니다.
UCFVehicleDefenseData::UCFVehicleDefenseData()
{
	FrontArmorConfig.MaximumArmor = 100.0f;
	FrontArmorConfig.DamageMultiplier = 1.0f;

	LeftArmorConfig.MaximumArmor = 100.0f;
	LeftArmorConfig.DamageMultiplier = 1.2f;

	RightArmorConfig.MaximumArmor = 100.0f;
	RightArmorConfig.DamageMultiplier = 1.2f;

	RearArmorConfig.MaximumArmor = 100.0f;
	RearArmorConfig.DamageMultiplier = 1.5f;

	TopArmorConfig.MaximumArmor = 100.0f;
	TopArmorConfig.DamageMultiplier = 1.3f;

	BottomArmorConfig.MaximumArmor = 100.0f;
	BottomArmorConfig.DamageMultiplier = 1.6f;
}

// [v1.0.0] 음수를 제거한 유효 최대 쉴드를 반환합니다.
float UCFVehicleDefenseData::GetEffectiveMaximumShield() const
{
	return bUseShield ? FMath::Max(MaximumShield, 0.0f) : 0.0f;
}

// [v1.0.0] 음수를 제거한 유효 쉴드 재생 지연 시간을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveShieldRegenerationDelaySeconds() const
{
	return bUseShield ? FMath::Max(ShieldRegenerationDelaySeconds, 0.0f) : 0.0f;
}

// [v1.0.0] 음수를 제거한 유효 초당 쉴드 재생량을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveShieldRegenerationPerSecond() const
{
	return bUseShield ? FMath::Max(ShieldRegenerationPerSecond, 0.0f) : 0.0f;
}

// [v1.0.0] 음수를 제거한 유효 장갑 저항을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveArmorResistance() const
{
	return FMath::Max(ArmorResistance, 0.0f);
}

// [v1.1.0] 음수나 비유한 값을 제거한 피팅용 유효 방어 질량을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveDefenseMassKg() const
{
	return FMath::IsFinite(DefenseMassKg) ? FMath::Max(DefenseMassKg, 0.0f) : 0.0f;
}

// [v1.0.0] 지정 방향의 음수 값을 보정한 장갑 설정 복사본을 반환합니다.
FCFDirectionalArmorConfig UCFVehicleDefenseData::GetEffectiveDirectionalArmorConfig(const ECFArmorDirection ArmorDirection) const
{
	// [v1.0.0] 지정 방향에서 읽은 원본 장갑 설정 복사본입니다.
	FCFDirectionalArmorConfig EffectiveArmorConfig;

	switch (ArmorDirection)
	{
	case ECFArmorDirection::Front:
		EffectiveArmorConfig = FrontArmorConfig;
		break;
	case ECFArmorDirection::Left:
		EffectiveArmorConfig = LeftArmorConfig;
		break;
	case ECFArmorDirection::Right:
		EffectiveArmorConfig = RightArmorConfig;
		break;
	case ECFArmorDirection::Rear:
		EffectiveArmorConfig = RearArmorConfig;
		break;
	case ECFArmorDirection::Top:
		EffectiveArmorConfig = TopArmorConfig;
		break;
	case ECFArmorDirection::Bottom:
		EffectiveArmorConfig = BottomArmorConfig;
		break;
	case ECFArmorDirection::None:
	default:
		EffectiveArmorConfig.MaximumArmor = 0.0f;
		EffectiveArmorConfig.DamageMultiplier = 1.0f;
		break;
	}

	EffectiveArmorConfig.MaximumArmor = EffectiveArmorConfig.GetEffectiveMaximumArmor();
	EffectiveArmorConfig.DamageMultiplier = EffectiveArmorConfig.GetEffectiveDamageMultiplier();
	return EffectiveArmorConfig;
}

// [v1.0.0] 음수를 제거한 유효 쉴드 단계 부품 피해 배율을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveShieldComponentDamageScale() const
{
	return FMath::Max(ShieldComponentDamageScale, 0.0f);
}

// [v1.0.0] 음수를 제거한 유효 장갑 단계 부품 피해 배율을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveArmorComponentDamageScale() const
{
	return FMath::Max(ArmorComponentDamageScale, 0.0f);
}

// [v1.0.0] 음수를 제거한 유효 차량 내구도 단계 부품 피해 배율을 반환합니다.
float UCFVehicleDefenseData::GetEffectiveIntegrityComponentDamageScale() const
{
	return FMath::Max(IntegrityComponentDamageScale, 0.0f);
}

// [v1.0.0] 디버그 패널과 로그에서 사용할 방어 데이터 요약 문자열을 생성합니다.
FString UCFVehicleDefenseData::BuildVehicleDefenseSummary() const
{
	return FString::Printf(
				TEXT("VehicleDefenseData: Id=%s, Mass=%.1fkg, Shield=%s %.1f, RegenDelay=%.2fs, RegenRate=%.1f/s, ArmorType=%s, Resistance=%.1f, Armor[F=%.1f x%.2f, L=%.1f x%.2f, R=%.1f x%.2f, Rear=%.1f x%.2f, Top=%.1f x%.2f, Bottom=%.1f x%.2f]"),
		*DefenseId.ToString(),
		GetEffectiveDefenseMassKg(),
		bUseShield ? TEXT("On") : TEXT("Off"),
		GetEffectiveMaximumShield(),
		GetEffectiveShieldRegenerationDelaySeconds(),
		GetEffectiveShieldRegenerationPerSecond(),
		*UEnum::GetValueAsString(ArmorType),
		GetEffectiveArmorResistance(),
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Front).MaximumArmor,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Front).DamageMultiplier,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Left).MaximumArmor,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Left).DamageMultiplier,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Right).MaximumArmor,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Right).DamageMultiplier,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Rear).MaximumArmor,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Rear).DamageMultiplier,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Top).MaximumArmor,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Top).DamageMultiplier,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Bottom).MaximumArmor,
		GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Bottom).DamageMultiplier);
}

// [v1.0.0] DataValidation과 Automation이 공유할 데이터 계약 오류 목록을 생성합니다.
bool UCFVehicleDefenseData::ValidateDefenseDataContract(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (DefenseId.IsNone())
	{
		OutValidationErrors.Add(LOCTEXT("MissingDefenseId", "DefenseId는 None일 수 없습니다."));
	}

	if (MaximumShield < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeMaximumShield", "MaximumShield는 0 이상이어야 합니다."));
	}

	if (ShieldRegenerationDelaySeconds < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeShieldRegenerationDelay", "ShieldRegenerationDelaySeconds는 0 이상이어야 합니다."));
	}

	if (ShieldRegenerationPerSecond < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeShieldRegenerationRate", "ShieldRegenerationPerSecond는 0 이상이어야 합니다."));
	}

	if (bUseShield && MaximumShield <= 0.0f && ShieldRegenerationPerSecond > 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("RegenerationWithoutShield", "쉴드 재생량이 0보다 크면 MaximumShield도 0보다 커야 합니다."));
	}

		if (ArmorResistance < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeArmorResistance", "ArmorResistance는 0 이상이어야 합니다."));
	}

	if (!FMath::IsFinite(DefenseMassKg) || DefenseMassKg < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidDefenseMass", "DefenseMassKg는 유한한 0 이상의 값이어야 합니다."));
	}

	ValidateDirectionalArmorConfig(LOCTEXT("FrontArmor", "정면 장갑"), FrontArmorConfig, OutValidationErrors);
	ValidateDirectionalArmorConfig(LOCTEXT("LeftArmor", "좌측 장갑"), LeftArmorConfig, OutValidationErrors);
	ValidateDirectionalArmorConfig(LOCTEXT("RightArmor", "우측 장갑"), RightArmorConfig, OutValidationErrors);
	ValidateDirectionalArmorConfig(LOCTEXT("RearArmor", "후면 장갑"), RearArmorConfig, OutValidationErrors);
	ValidateDirectionalArmorConfig(LOCTEXT("TopArmor", "상부 장갑"), TopArmorConfig, OutValidationErrors);
	ValidateDirectionalArmorConfig(LOCTEXT("BottomArmor", "하부 장갑"), BottomArmorConfig, OutValidationErrors);

	if (ShieldComponentDamageScale < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeShieldComponentDamageScale", "ShieldComponentDamageScale은 0 이상이어야 합니다."));
	}

	if (ArmorComponentDamageScale < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeArmorComponentDamageScale", "ArmorComponentDamageScale은 0 이상이어야 합니다."));
	}

	if (IntegrityComponentDamageScale < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeIntegrityComponentDamageScale", "IntegrityComponentDamageScale은 0 이상이어야 합니다."));
	}

	return OutValidationErrors.IsEmpty();
}

#if WITH_EDITOR
// [v1.0.0] Unreal Data Validation에서 잘못된 방어 설정을 보고합니다.
EDataValidationResult UCFVehicleDefenseData::IsDataValid(FDataValidationContext& Context) const
{
	// [v1.0.0] 상위 PrimaryDataAsset이 반환한 기본 검증 결과입니다.
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);

	// [v1.0.0] 차량 방어 데이터 계약에서 발견된 오류 목록입니다.
	TArray<FText> ValidationErrors;
	if (!ValidateDefenseDataContract(ValidationErrors))
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
