// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-31
// Description: CF-FQ-033 DR-P0-01 차량 방어 데이터 계약 자동화 테스트
// Scope: 공용 enum·결과 기본값, 방향별 장갑 기본값·보정, DataValidation 계약과 VehicleData 선택 참조 호환을 검증합니다.
// Changelog:
// - v1.0.0: CarFight.Damage.DR_P0_01.DataContract 최초 추가.
// Migration:
// - 실제 쉴드·장갑 런타임 상태와 피해 분배는 DR-P0-02 이후 테스트로 확장한다.
// - 기존 VehicleData는 DefaultDefenseData가 None인지 이 테스트에서 회귀 검증한다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFDamageRuntimeTypes.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleDefenseDataContractTest,
	"CarFight.Damage.DR_P0_01.DataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] VehicleDefenseData 기본값, 안전 보정, 오류 검출과 기존 VehicleData 호환을 검증합니다.
bool FCFVehicleDefenseDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 새 VehicleDefenseData의 P0 테스트 기본값을 확인할 Transient 인스턴스입니다.
	UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>();
	if (!TestNotNull(TEXT("VehicleDefenseData 생성"), DefenseData))
	{
		return false;
	}

	TestEqual(TEXT("기본 방어 ID"), DefenseData->DefenseId, FName(TEXT("ProtoVehicleDefense")));
	TestTrue(TEXT("기본 쉴드 사용"), DefenseData->bUseShield);
	TestEqual(TEXT("기본 최대 쉴드"), DefenseData->GetEffectiveMaximumShield(), 100.0f);
	TestEqual(TEXT("기본 쉴드 재생 지연"), DefenseData->GetEffectiveShieldRegenerationDelaySeconds(), 5.0f);
	TestEqual(TEXT("기본 초당 쉴드 재생"), DefenseData->GetEffectiveShieldRegenerationPerSecond(), 10.0f);
	TestEqual(TEXT("기본 장갑 타입"), DefenseData->ArmorType, ECFArmorType::Standard);
	TestEqual(TEXT("기본 장갑 저항"), DefenseData->GetEffectiveArmorResistance(), 100.0f);

	TestEqual(TEXT("정면 장갑 배율"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Front).DamageMultiplier, 1.0f);
	TestEqual(TEXT("좌측 장갑 배율"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Left).DamageMultiplier, 1.2f);
	TestEqual(TEXT("우측 장갑 배율"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Right).DamageMultiplier, 1.2f);
	TestEqual(TEXT("후면 장갑 배율"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Rear).DamageMultiplier, 1.5f);
	TestEqual(TEXT("상부 장갑 배율"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Top).DamageMultiplier, 1.3f);
	TestEqual(TEXT("하부 장갑 배율"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Bottom).DamageMultiplier, 1.6f);
	TestEqual(TEXT("None 방향 장갑 내구도 0"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::None).MaximumArmor, 0.0f);
	TestEqual(TEXT("None 방향 피해 배율 1"), DefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::None).DamageMultiplier, 1.0f);

	// [v1.0.0] 기본 VehicleDefenseData가 데이터 계약을 통과하는지 확인할 오류 목록입니다.
	TArray<FText> ValidationErrors;
	TestTrue(TEXT("기본 VehicleDefenseData 계약 유효"), DefenseData->ValidateDefenseDataContract(ValidationErrors));
	TestEqual(TEXT("기본 계약 오류 없음"), ValidationErrors.Num(), 0);
	TestTrue(TEXT("방어 요약에 ID 포함"), DefenseData->BuildVehicleDefenseSummary().Contains(TEXT("ProtoVehicleDefense")));

	// [v1.0.0] 잘못된 값의 검출과 안전 Getter 보정을 확인할 Transient 인스턴스입니다.
	UCFVehicleDefenseData* InvalidDefenseData = NewObject<UCFVehicleDefenseData>();
	if (!TestNotNull(TEXT("Invalid VehicleDefenseData 생성"), InvalidDefenseData))
	{
		return false;
	}

	InvalidDefenseData->DefenseId = NAME_None;
	InvalidDefenseData->MaximumShield = -50.0f;
	InvalidDefenseData->ShieldRegenerationDelaySeconds = -2.0f;
	InvalidDefenseData->ShieldRegenerationPerSecond = -3.0f;
	InvalidDefenseData->ArmorResistance = -100.0f;
	InvalidDefenseData->RearArmorConfig.MaximumArmor = -10.0f;
	InvalidDefenseData->RearArmorConfig.DamageMultiplier = -1.0f;
	InvalidDefenseData->ShieldComponentDamageScale = -1.0f;
	InvalidDefenseData->ArmorComponentDamageScale = -1.0f;
	InvalidDefenseData->IntegrityComponentDamageScale = -1.0f;

	TestFalse(TEXT("잘못된 VehicleDefenseData 계약 거부"), InvalidDefenseData->ValidateDefenseDataContract(ValidationErrors));
	TestTrue(TEXT("잘못된 계약 오류가 하나 이상 존재"), ValidationErrors.Num() > 0);
	TestEqual(TEXT("음수 최대 쉴드는 0으로 보정"), InvalidDefenseData->GetEffectiveMaximumShield(), 0.0f);
	TestEqual(TEXT("음수 쉴드 재생 지연은 0으로 보정"), InvalidDefenseData->GetEffectiveShieldRegenerationDelaySeconds(), 0.0f);
	TestEqual(TEXT("음수 쉴드 재생량은 0으로 보정"), InvalidDefenseData->GetEffectiveShieldRegenerationPerSecond(), 0.0f);
	TestEqual(TEXT("음수 장갑 저항은 0으로 보정"), InvalidDefenseData->GetEffectiveArmorResistance(), 0.0f);
	TestEqual(TEXT("음수 후면 장갑은 0으로 보정"), InvalidDefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Rear).MaximumArmor, 0.0f);
	TestEqual(TEXT("음수 후면 배율은 0으로 보정"), InvalidDefenseData->GetEffectiveDirectionalArmorConfig(ECFArmorDirection::Rear).DamageMultiplier, 0.0f);
	TestEqual(TEXT("음수 쉴드 부품 배율은 0으로 보정"), InvalidDefenseData->GetEffectiveShieldComponentDamageScale(), 0.0f);
	TestEqual(TEXT("음수 장갑 부품 배율은 0으로 보정"), InvalidDefenseData->GetEffectiveArmorComponentDamageScale(), 0.0f);
	TestEqual(TEXT("음수 내구도 부품 배율은 0으로 보정"), InvalidDefenseData->GetEffectiveIntegrityComponentDamageScale(), 0.0f);

	InvalidDefenseData->bUseShield = false;
	InvalidDefenseData->MaximumShield = 500.0f;
	InvalidDefenseData->ShieldRegenerationDelaySeconds = 5.0f;
	InvalidDefenseData->ShieldRegenerationPerSecond = 20.0f;
	TestEqual(TEXT("쉴드 비활성 최대 쉴드 0"), InvalidDefenseData->GetEffectiveMaximumShield(), 0.0f);
	TestEqual(TEXT("쉴드 비활성 재생 지연 0"), InvalidDefenseData->GetEffectiveShieldRegenerationDelaySeconds(), 0.0f);
	TestEqual(TEXT("쉴드 비활성 재생량 0"), InvalidDefenseData->GetEffectiveShieldRegenerationPerSecond(), 0.0f);

	// [v1.0.0] 기존 VehicleData가 신규 방어 에셋 없이 현재 직접 Health fallback을 유지하는지 확인할 인스턴스입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>();
	if (!TestNotNull(TEXT("VehicleData 생성"), VehicleData))
	{
		return false;
	}
	TestNull(TEXT("기존 VehicleData 기본 DefenseData는 None"), VehicleData->DefaultDefenseData);

	// [v1.0.0] 신규 공용 피해 결과가 런타임 적용 전 안전한 기본값을 갖는지 확인합니다.
	const FCFVehicleDamageResult DefaultDamageResult;
	TestFalse(TEXT("기본 피해 요청 미수락"), DefaultDamageResult.bDamageAccepted);
	TestFalse(TEXT("기본 방어층 미적용"), DefaultDamageResult.bAppliedToAnyLayer);
	TestFalse(TEXT("기본 Legacy Fallback 미사용"), DefaultDamageResult.bUsedLegacyHealthFallback);
	TestEqual(TEXT("기본 피해 전달 방식은 DirectHit"), DefaultDamageResult.DamageDeliveryType, ECFDamageDeliveryType::DirectHit);
	TestEqual(TEXT("기본 장갑 방향은 None"), DefaultDamageResult.ArmorDirection, ECFArmorDirection::None);
	TestEqual(TEXT("기본 요청 피해 0"), DefaultDamageResult.RequestedDamage, 0.0f);
	TestEqual(TEXT("기본 부품 피해 0"), DefaultDamageResult.DamageAppliedToComponents, 0.0f);
	TestEqual(TEXT("기본 물리 충격 0"), DefaultDamageResult.PhysicalImpulseApplied, 0.0f);
	TestEqual(TEXT("기본 영향 부품 없음"), DefaultDamageResult.AffectedComponentIds.Num(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
