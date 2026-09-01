// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-01
// Description: CF-FQ-041 런타임 테스트 Catalog 검증 구현
// Scope: Empty/Null/Duplicate hard reference를 fail-closed로 검증하고 bounded 요약을 제공합니다.
// Changelog:
// - v1.0.0: RTA-P0-01 runtime validation과 summary 구현.
// Migration:
// - Catalog는 Asset을 수정하지 않으며 등록 여부만 판정합니다.

#include "CFRuntimeTestCatalogData.h"

#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"

#define LOCTEXT_NAMESPACE "CFRuntimeTestCatalogData"

// [v1.0.0] Catalog가 Runtime 선택 목록으로 사용 가능한 최소 계약을 만족하는지 반환합니다.
bool UCFRuntimeTestCatalogData::IsRuntimeTestCatalogUsable() const
{
	// [v1.0.0] 상세 오류 수집을 위한 임시 배열입니다.
	TArray<FText> ValidationErrors;
	return ValidateRuntimeTestCatalog(ValidationErrors);
}

// [v1.0.0] Catalog의 모든 계약 오류를 호출자에게 반환합니다.
bool UCFRuntimeTestCatalogData::ValidateRuntimeTestCatalog(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (AllowedVehicleData.IsEmpty())
	{
		OutValidationErrors.Add(LOCTEXT("VehicleListEmpty", "허용 차량 데이터 목록에 최소 1개의 VehicleData가 필요합니다."));
	}

	// [v1.0.0] 차량 목록의 중복 Asset identity를 검출하기 위한 집합입니다.
	TSet<const UCFVehicleData*> SeenVehicleData;
	for (int32 VehicleIndex = 0; VehicleIndex < AllowedVehicleData.Num(); ++VehicleIndex)
	{
		// [v1.0.0] 현재 검사 중인 VehicleData입니다.
		const UCFVehicleData* VehicleData = AllowedVehicleData[VehicleIndex].Get();
		if (!IsValid(VehicleData))
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("VehicleEntryInvalid", "허용 차량 데이터 목록의 {0}번 항목이 비어 있거나 유효하지 않습니다."),
				FText::AsNumber(VehicleIndex)));
			continue;
		}

		if (SeenVehicleData.Contains(VehicleData))
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("VehicleEntryDuplicate", "VehicleData '{0}'가 허용 차량 데이터 목록에 중복 등록되어 있습니다."),
				FText::FromString(VehicleData->GetName())));
			continue;
		}

		SeenVehicleData.Add(VehicleData);
	}

	if (AllowedEquipmentPresetData.IsEmpty())
	{
		OutValidationErrors.Add(LOCTEXT("EquipmentListEmpty", "허용 장비 프리셋 목록에 최소 1개의 EquipmentPresetData가 필요합니다."));
	}

	// [v1.0.0] 장비 목록의 중복 Asset identity를 검출하기 위한 집합입니다.
	TSet<const UCFEquipmentPresetData*> SeenEquipmentPresetData;
	for (int32 EquipmentIndex = 0; EquipmentIndex < AllowedEquipmentPresetData.Num(); ++EquipmentIndex)
	{
		// [v1.0.0] 현재 검사 중인 EquipmentPresetData입니다.
		const UCFEquipmentPresetData* EquipmentPresetData = AllowedEquipmentPresetData[EquipmentIndex].Get();
		if (!IsValid(EquipmentPresetData))
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("EquipmentEntryInvalid", "허용 장비 프리셋 목록의 {0}번 항목이 비어 있거나 유효하지 않습니다."),
				FText::AsNumber(EquipmentIndex)));
			continue;
		}

		if (SeenEquipmentPresetData.Contains(EquipmentPresetData))
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("EquipmentEntryDuplicate", "EquipmentPresetData '{0}'가 허용 장비 프리셋 목록에 중복 등록되어 있습니다."),
				FText::FromString(EquipmentPresetData->GetName())));
			continue;
		}

		SeenEquipmentPresetData.Add(EquipmentPresetData);
	}

	return OutValidationErrors.IsEmpty();
}

// [v1.0.0] 디버그 UI와 로그에 사용할 Catalog 등록 개수와 유효성 요약을 생성합니다.
FString UCFRuntimeTestCatalogData::BuildRuntimeTestCatalogSummary() const
{
	// [v1.0.0] Catalog 요약에 사용할 상세 검증 오류 목록입니다.
	TArray<FText> ValidationErrors;
	const bool bCatalogUsable = ValidateRuntimeTestCatalog(ValidationErrors);

	return FString::Printf(
		TEXT("RuntimeTestCatalog: Validation=%s, Vehicles=%d, Equipment=%d, Issues=%d"),
		bCatalogUsable ? TEXT("Valid") : TEXT("Invalid"),
		AllowedVehicleData.Num(),
		AllowedEquipmentPresetData.Num(),
		ValidationErrors.Num());
}

#undef LOCTEXT_NAMESPACE
