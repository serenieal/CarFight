// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-16
// Description: CarFight 출격 전 차량 피팅 선택과 결정론적 Snapshot 구현
// Scope: 피팅 기본값, 데이터 계약, MountProfile·Hardpoint·장비·Scanner·출격 탄약·방어 호환 검증과 Pawn 없는 질량 Snapshot 생성을 구현합니다.
// Changelog:
// - v1.3.0: CF-FQ-037 SCAN-P0-01 Utility Scanner SensorData 해석, mount-aware 장비 완성도와 단일 Scanner Source 검증을 추가.
// - v1.2.0: CF-FQ-031 AMMO-P0-07 명시적 출격 탄약 검증, 결정론적 Snapshot 복사, finite 무기 초기 장전 충족 검사와 AmmoMassKg 합산을 추가.
// - v1.1.0: CF-FQ-034 FIT-P0-03 결정론적 Compatibility Validation, 방어 3상태 해석, 질량 합산과 GrossMass 검증을 추가.
// - v1.0.0: CF-FQ-034 FIT-P0-02 VehicleFittingData 계약 검증과 요약을 최초 구현.
// Migration:
// - BuildFittingSnapshot은 VehicleData.MountProfiles 순서와 AmmoId 이름 순서로 결과를 생성하므로 편집 배열 순서가 결정론적 결과를 바꾸지 않는다.
// - 기존 VehicleData와 차량 런타임은 VehicleFittingData를 자동 참조하지 않으므로 동작이 변경되지 않는다.
// - BaseVehicleMassKg와 MaximumGrossMassKg가 0인 기존 VehicleData는 피팅에 명시적으로 연결할 때만 계약 오류가 된다.
// - AmmoMassKg는 InitialSortieAmmoLoads의 실제 출격 수량만 사용하며 MaximumLoadableAmmoCount를 현재 수량으로 대입하지 않는다.
// - finite WeaponData는 자신의 AmmoId가 출격 목록에 명시되고 같은 탄종 무기들의 초기 장전량 합계를 충족해야 한다.
// - 기존 VehiclePawn 초기 질량 적용 순서는 유지하고 Snapshot의 AmmoMassKg만 같은 질량 계산에 포함한다.

#include "CFVehicleFittingData.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"

#define LOCTEXT_NAMESPACE "CFVehicleFittingData"

namespace
{
	// [v1.1.0] Snapshot에 구조화된 피팅 검증 문제 한 건을 추가합니다.
	void AddFittingValidationIssue(
		FCFVehicleFittingSnapshot& Snapshot,
		const ECFFittingIssueSeverity Severity,
		const ECFFittingIssueCode IssueCode,
		const FText& Message,
		const FName MountProfileId = NAME_None)
	{
		// [v1.1.0] Snapshot에 추가할 구조화된 검증 문제입니다.
		FCFFittingValidationIssue ValidationIssue;
		ValidationIssue.Severity = Severity;
		ValidationIssue.IssueCode = IssueCode;
		ValidationIssue.Message = Message;
		ValidationIssue.MountProfileId = MountProfileId;
		Snapshot.ValidationIssues.Add(ValidationIssue);
	}

	// [v1.1.0] Snapshot 문제 목록에 지정 심각도가 하나라도 있는지 반환합니다.
	bool HasFittingIssueSeverity(
		const FCFVehicleFittingSnapshot& Snapshot,
		const ECFFittingIssueSeverity Severity)
	{
		// [v1.1.0] 심각도를 확인할 한 검증 문제입니다.
		for (const FCFFittingValidationIssue& ValidationIssue : Snapshot.ValidationIssues)
		{
			if (ValidationIssue.Severity == Severity)
			{
				return true;
			}
		}

		return false;
	}

	// [v1.1.0] 현재 검증 문제 심각도를 기준으로 Snapshot 최종 상태를 확정합니다.
	void FinalizeFittingValidationState(FCFVehicleFittingSnapshot& Snapshot)
	{
		// [v1.1.0] Snapshot에 적용을 차단하는 오류가 존재하는지 여부입니다.
		const bool bHasErrors = HasFittingIssueSeverity(Snapshot, ECFFittingIssueSeverity::Error);

		// [v1.1.0] Snapshot에 적용 가능한 경고가 존재하는지 여부입니다.
		const bool bHasWarnings = HasFittingIssueSeverity(Snapshot, ECFFittingIssueSeverity::Warning);

		if (bHasErrors)
		{
			Snapshot.ValidationState = ECFFittingValidationState::Invalid;
		}
		else if (bHasWarnings)
		{
			Snapshot.ValidationState = ECFFittingValidationState::ValidWithWarnings;
		}
		else
		{
			Snapshot.ValidationState = ECFFittingValidationState::Valid;
		}
	}

	// [v1.1.0] FName 목록을 이름 문자열 오름차순으로 정렬합니다.
	void SortNamesDeterministically(TArray<FName>& Names)
	{
		Names.Sort([](const FName LeftName, const FName RightName)
		{
			return LeftName.ToString() < RightName.ToString();
		});
	}

	// [v1.1.0] 요구 무기 크기가 장착 프로파일 제한을 초과하는지 반환합니다.
	bool DoesWeaponSizeExceedLimit(
		const ECFVehicleWeaponSize RequiredWeaponSize,
		const ECFVehicleWeaponSize MountSizeLimit)
	{
		if (RequiredWeaponSize == ECFVehicleWeaponSize::None)
		{
			return false;
		}

		if (MountSizeLimit == ECFVehicleWeaponSize::None)
		{
			return true;
		}

		// [v1.1.0] 요구 무기 크기를 비교 가능한 정수 값으로 변환한 값입니다.
		const uint8 RequiredWeaponSizeValue = static_cast<uint8>(RequiredWeaponSize);

		// [v1.1.0] 장착 프로파일 크기 제한을 비교 가능한 정수 값으로 변환한 값입니다.
		const uint8 MountSizeLimitValue = static_cast<uint8>(MountSizeLimit);

		return RequiredWeaponSizeValue > MountSizeLimitValue;
	}

	// [v1.1.0] 필수 질량 값을 검증하고 합산 가능한 양수 질량 또는 0을 반환합니다.
	float ResolveRequiredFittingMass(
		FCFVehicleFittingSnapshot& Snapshot,
		const float RawMassKg,
		const FText& MassDisplayName,
		const FName MountProfileId = NAME_None)
	{
		if (!FMath::IsFinite(RawMassKg) || RawMassKg < 0.0f)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidMassValue,
				FText::Format(
					LOCTEXT("InvalidRequiredMass", "{0}은 유한한 0 이상의 kg 값이어야 합니다."),
					MassDisplayName),
				MountProfileId);
			return 0.0f;
		}

		if (RawMassKg <= 0.0f)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::MissingMassSource,
				FText::Format(
					LOCTEXT("MissingRequiredMass", "{0}이 0kg로 미설정되어 있습니다."),
					MassDisplayName),
				MountProfileId);
			return 0.0f;
		}

		return RawMassKg;
	}
}

// [v1.0.0] P0 피팅 데이터의 안전한 기본값을 초기화합니다.
UCFVehicleFittingData::UCFVehicleFittingData()
	: DisplayName(NSLOCTEXT("CarFight", "ProtoVehicleFittingDisplayName", "Prototype Vehicle Fitting"))
{
}

// [v1.0.0] DataValidation과 Automation이 공유할 피팅 데이터 계약 오류 목록을 생성합니다.
bool UCFVehicleFittingData::ValidateFittingDataContract(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (FittingId.IsNone())
	{
		OutValidationErrors.Add(LOCTEXT("MissingFittingId", "FittingId는 None일 수 없습니다."));
	}

	if (!VehicleData)
	{
		OutValidationErrors.Add(LOCTEXT("MissingVehicleData", "VehicleData가 지정되어야 합니다."));
	}
	else
	{
		if (!FMath::IsFinite(VehicleData->BaseVehicleMassKg) || VehicleData->BaseVehicleMassKg <= 0.0f)
		{
			OutValidationErrors.Add(LOCTEXT("InvalidBaseVehicleMass", "VehicleData.BaseVehicleMassKg는 유한한 0보다 큰 값이어야 합니다."));
		}

		if (!FMath::IsFinite(VehicleData->MaximumGrossMassKg) || VehicleData->MaximumGrossMassKg <= 0.0f)
		{
			OutValidationErrors.Add(LOCTEXT("InvalidMaximumGrossMass", "VehicleData.MaximumGrossMassKg는 유한한 0보다 큰 값이어야 합니다."));
		}
		else if (VehicleData->MaximumGrossMassKg < VehicleData->BaseVehicleMassKg)
		{
			OutValidationErrors.Add(LOCTEXT("GrossMassBelowBaseMass", "MaximumGrossMassKg는 BaseVehicleMassKg보다 작을 수 없습니다."));
		}
	}

	// [v1.0.0] 피팅 내 중복 MountProfileId를 검출하기 위한 집합입니다.
	TSet<FName> SeenMountProfileIds;

	// [v1.0.0] 계약을 검사할 한 장착 선택입니다.
	for (const FCFVehicleMountSelection& MountSelection : MountSelections)
	{
		if (MountSelection.MountProfileId.IsNone())
		{
			OutValidationErrors.Add(LOCTEXT("MissingMountProfileId", "MountSelections의 MountProfileId는 None일 수 없습니다."));
			continue;
		}

		if (SeenMountProfileIds.Contains(MountSelection.MountProfileId))
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("DuplicateMountProfileId", "MountProfileId '{0}' 선택이 중복되어 있습니다."),
				FText::FromName(MountSelection.MountProfileId)));
		}
		else
		{
			SeenMountProfileIds.Add(MountSelection.MountProfileId);
		}

		if (!MountSelection.bEnabled)
		{
			continue;
		}

		if (!MountSelection.EquipmentPresetData)
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("MissingEquipmentPreset", "활성 MountProfileId '{0}'에 EquipmentPresetData가 지정되어야 합니다."),
				FText::FromName(MountSelection.MountProfileId)));
			continue;
		}

								// [v1.3.0] 현재 선택이 가리키는 VehicleData 장착 프로파일입니다. 존재하면 실제 MountType 기준 payload 계약을 검사합니다.
		const FCFVehicleMountProfile* MatchingMountProfile = VehicleData
			? VehicleData->MountProfiles.FindByPredicate([&MountSelection](const FCFVehicleMountProfile& MountProfile)
			{
				return MountProfile.MountProfileId == MountSelection.MountProfileId;
			})
			: nullptr;

		// [v1.3.0] 알려진 MountProfile은 mount-aware 계약을, 알 수 없는 선택은 프리셋 자체의 단일 payload 계약을 검사합니다.
		const bool bEquipmentPresetComplete = MatchingMountProfile
			? MountSelection.EquipmentPresetData->HasCompleteEquipmentDataForMount(MatchingMountProfile->MountType)
			: MountSelection.EquipmentPresetData->HasCompleteEquipmentData();
		if (!bEquipmentPresetComplete)
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("IncompleteEquipmentPreset", "MountProfileId '{0}'의 EquipmentPresetData가 해당 장착 타입에 필요한 단일 payload 계약을 만족하지 않습니다."),
				FText::FromName(MountSelection.MountProfileId)));
		}


		if (MountSelection.EquipmentPresetData->DefaultSensorData
			&& !MountSelection.EquipmentPresetData->DefaultSensorData->IsSensorConfigValid())
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("InvalidEquipmentSensorData", "MountProfileId '{0}'의 VehicleSensorData SensorConfig가 유효하지 않습니다."),
				FText::FromName(MountSelection.MountProfileId)));
		}

		}

	// [v1.2.0] 출격 탄약 목록에서 같은 AmmoId 중복을 검출할 집합입니다.
	TSet<FName> SeenAmmoIds;
	for (const FCFAmmoSortieLoad& AmmoLoad : InitialSortieAmmoLoads)
	{
		UCFAmmoData* AmmoData = AmmoLoad.AmmoData;
		if (!IsValid(AmmoData) || !AmmoData->IsAmmoDataValid())
		{
			OutValidationErrors.Add(LOCTEXT("InvalidSortieAmmoData", "InitialSortieAmmoLoads의 AmmoData와 AmmoId는 유효해야 합니다."));
			continue;
		}

		if (AmmoLoad.InitialSortieAmmoCount < 0)
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("NegativeSortieAmmoCount", "AmmoId '{0}'의 InitialSortieAmmoCount는 0 이상이어야 합니다."),
				FText::FromName(AmmoData->AmmoId)));
		}

		if (SeenAmmoIds.Contains(AmmoData->AmmoId))
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("DuplicateSortieAmmoId", "AmmoId '{0}'가 InitialSortieAmmoLoads에 중복되어 있습니다."),
				FText::FromName(AmmoData->AmmoId)));
		}
		else
		{
			SeenAmmoIds.Add(AmmoData->AmmoId);
		}

		if (AmmoLoad.InitialSortieAmmoCount > AmmoData->GetEffectiveMaximumLoadableAmmoCount())
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("SortieAmmoCountExceeded", "AmmoId '{0}'의 출격 수량 {1}이 최대 적재 가능 수량 {2}를 초과합니다."),
				FText::FromName(AmmoData->AmmoId),
				FText::AsNumber(AmmoLoad.InitialSortieAmmoCount),
				FText::AsNumber(AmmoData->GetEffectiveMaximumLoadableAmmoCount())));
		}

		if (AmmoLoad.InitialSortieAmmoCount > 0 && AmmoData->GetEffectiveUnitMassKg() <= 0.0f)
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("MissingSortieAmmoMass", "AmmoId '{0}'를 1발 이상 적재하려면 UnitMassKg가 유한한 0보다 큰 값이어야 합니다."),
				FText::FromName(AmmoData->AmmoId)));
		}
	}

	if (DefenseSelection.SelectionMode == ECFDefenseSelectionMode::Override && !DefenseSelection.DefenseData)
	{
		OutValidationErrors.Add(LOCTEXT("MissingOverrideDefenseData", "DefenseSelection이 Override이면 DefenseData가 지정되어야 합니다."));
	}

	return OutValidationErrors.IsEmpty();
}

// [v1.1.0] 차량 플랫폼과 피팅 선택을 검증해 Pawn 없이 결정론적 Snapshot을 생성합니다.
FCFVehicleFittingSnapshot UCFVehicleFittingData::BuildFittingSnapshot() const
{
	// [v1.1.0] 이번 피팅 해석의 모든 결과와 검증 문제를 보존할 Snapshot입니다.
	FCFVehicleFittingSnapshot Snapshot;
	Snapshot.FittingId = FittingId;
	Snapshot.VehicleData = VehicleData;
	Snapshot.ResolvedDefenseSelectionMode = DefenseSelection.SelectionMode;

	if (FittingId.IsNone())
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::MissingFittingId,
			LOCTEXT("SnapshotMissingFittingId", "FittingId가 지정되지 않아 피팅 Snapshot을 적용할 수 없습니다."));
	}

	if (!VehicleData)
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::MissingVehicleData,
			LOCTEXT("SnapshotMissingVehicleData", "VehicleData가 지정되지 않아 피팅 Snapshot을 생성할 수 없습니다."));
		FinalizeFittingValidationState(Snapshot);
		return Snapshot;
	}

	Snapshot.BaseVehicleMassKg = ResolveRequiredFittingMass(
		Snapshot,
		VehicleData->BaseVehicleMassKg,
		LOCTEXT("BaseVehicleMassDisplayName", "기준 차량 질량"));

	Snapshot.MaximumGrossMassKg = ResolveRequiredFittingMass(
		Snapshot,
		VehicleData->MaximumGrossMassKg,
		LOCTEXT("MaximumGrossMassDisplayName", "최대 허용 총중량"));

	if (Snapshot.BaseVehicleMassKg > 0.0f
		&& Snapshot.MaximumGrossMassKg > 0.0f
		&& Snapshot.MaximumGrossMassKg < Snapshot.BaseVehicleMassKg)
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::InvalidMassValue,
			FText::Format(
				LOCTEXT("GrossMassBelowBaseMassIssue", "최대 허용 총중량 {0}kg은 기준 차량 질량 {1}kg보다 작을 수 없습니다."),
				FText::AsNumber(Snapshot.MaximumGrossMassKg),
				FText::AsNumber(Snapshot.BaseVehicleMassKg)));
	}

	// [v1.1.0] VehicleData에서 실제 제공하는 하드포인트 위치 슬롯 ID 집합입니다.
	TSet<FName> KnownHardpointSlotIds;
	for (const FCFVehicleHardpointSlot& HardpointSlot : VehicleData->HardpointSlots)
	{
		if (!HardpointSlot.LocationSlotId.IsNone())
		{
			KnownHardpointSlotIds.Add(HardpointSlot.LocationSlotId);
		}
	}

	// [v1.1.0] VehicleData에서 실제 제공하는 장착 프로파일 ID 집합입니다.
	TSet<FName> KnownMountProfileIds;
	for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
	{
		if (!MountProfile.MountProfileId.IsNone())
		{
			KnownMountProfileIds.Add(MountProfile.MountProfileId);
		}
	}

	// [v1.1.0] 유일한 MountProfileId에 대응하는 피팅 선택 포인터 맵입니다.
	TMap<FName, const FCFVehicleMountSelection*> UniqueMountSelections;

	// [v1.1.0] 중복되어 어떤 선택도 적용할 수 없는 MountProfileId 집합입니다.
	TSet<FName> DuplicateMountSelectionIds;

	// [v1.1.0] 알 수 없는 프로파일까지 포함한 모든 유효 선택 ID 집합입니다.
	TSet<FName> AllMountSelectionIds;

	// [v1.1.0] MountProfileId가 None인 잘못된 선택의 개수입니다.
	int32 MissingMountProfileIdSelectionCount = 0;

	for (const FCFVehicleMountSelection& MountSelection : MountSelections)
	{
		if (MountSelection.MountProfileId.IsNone())
		{
			++MissingMountProfileIdSelectionCount;
			continue;
		}

		AllMountSelectionIds.Add(MountSelection.MountProfileId);

		if (DuplicateMountSelectionIds.Contains(MountSelection.MountProfileId))
		{
			continue;
		}

		if (UniqueMountSelections.Contains(MountSelection.MountProfileId))
		{
			UniqueMountSelections.Remove(MountSelection.MountProfileId);
			DuplicateMountSelectionIds.Add(MountSelection.MountProfileId);
			continue;
		}

		UniqueMountSelections.Add(MountSelection.MountProfileId, &MountSelection);
	}

	if (MissingMountProfileIdSelectionCount > 0)
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::MissingMountSelection,
			FText::Format(
				LOCTEXT("MissingMountProfileIdSelections", "MountProfileId가 None인 장착 선택이 {0}개 있습니다."),
				FText::AsNumber(MissingMountProfileIdSelectionCount)));
	}

	// [v1.1.0] 이름 순서로 구조화된 오류를 추가할 중복 장착 선택 ID 목록입니다.
	TArray<FName> SortedDuplicateMountSelectionIds = DuplicateMountSelectionIds.Array();
	SortNamesDeterministically(SortedDuplicateMountSelectionIds);
	for (const FName DuplicateMountSelectionId : SortedDuplicateMountSelectionIds)
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::DuplicateMountSelection,
			FText::Format(
				LOCTEXT("DuplicateMountSelectionIssue", "MountProfileId '{0}'의 피팅 선택이 중복되어 어느 선택도 적용하지 않습니다."),
				FText::FromName(DuplicateMountSelectionId)),
			DuplicateMountSelectionId);
	}

	// [v1.1.0] VehicleData에 존재하지 않는 선택 ID를 이름 순서로 보고할 목록입니다.
	TArray<FName> SortedUnknownMountSelectionIds;
	for (const FName MountSelectionId : AllMountSelectionIds)
	{
		if (!KnownMountProfileIds.Contains(MountSelectionId))
		{
			SortedUnknownMountSelectionIds.Add(MountSelectionId);
		}
	}
		SortNamesDeterministically(SortedUnknownMountSelectionIds);
	for (const FName UnknownMountSelectionId : SortedUnknownMountSelectionIds)
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::UnknownMountProfile,
			FText::Format(
				LOCTEXT("UnknownMountProfileIssue", "MountProfileId '{0}'가 VehicleData.MountProfiles에 존재하지 않습니다."),
				FText::FromName(UnknownMountSelectionId)),
			UnknownMountSelectionId);
	}

	// [v1.3.0] 현재 Snapshot에 해석된 Scanner Source 수입니다. 단 하나만 최종 ResolvedSensorData가 될 수 있습니다.
	int32 ResolvedSensorSourceCount = 0;

	for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
	{
		// [v1.1.0] VehicleData 순서대로 생성할 한 장착 프로파일의 최종 해석 결과입니다.
		FCFResolvedFittingMount ResolvedMount;
		ResolvedMount.MountProfileId = MountProfile.MountProfileId;
		ResolvedMount.LocationSlotId = MountProfile.LocationSlotRef;
		ResolvedMount.MountType = MountProfile.MountType;
		ResolvedMount.SizeLimit = MountProfile.SizeLimit;

		if (MountProfile.MountProfileId.IsNone())
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::VehicleDataMismatch,
				LOCTEXT("VehicleMountProfileIdMissing", "VehicleData.MountProfiles에 MountProfileId가 None인 항목이 있습니다."));
		}

		if (MountProfile.LocationSlotRef.IsNone() || !KnownHardpointSlotIds.Contains(MountProfile.LocationSlotRef))
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::MissingHardpointSlot,
				FText::Format(
					LOCTEXT("MissingHardpointSlotIssue", "MountProfileId '{0}'가 참조하는 하드포인트 슬롯 '{1}'을 찾을 수 없습니다."),
					FText::FromName(MountProfile.MountProfileId),
					FText::FromName(MountProfile.LocationSlotRef)),
				MountProfile.MountProfileId);
		}

						// [v1.1.0] 현재 장착 프로파일에 최종 적용할 장비 프리셋입니다.
		UCFEquipmentPresetData* ResolvedEquipmentPresetData = nullptr;

		if (!DuplicateMountSelectionIds.Contains(MountProfile.MountProfileId))
		{
			// [v1.1.0] 현재 장착 프로파일에 대응하는 유일한 피팅 선택 포인터 저장 위치입니다.
			const FCFVehicleMountSelection* const* FoundMountSelection = UniqueMountSelections.Find(MountProfile.MountProfileId);

			if (FoundMountSelection && *FoundMountSelection)
			{
				// [v1.1.0] 현재 장착 프로파일에 대응하는 실제 피팅 선택입니다.
				const FCFVehicleMountSelection& MountSelection = **FoundMountSelection;
				if (MountSelection.bEnabled)
				{
					ResolvedMount.SelectionSource = ECFFittingSelectionSource::FittingOverride;
					ResolvedEquipmentPresetData = MountSelection.EquipmentPresetData;
				}
				else
				{
					ResolvedMount.SelectionSource = ECFFittingSelectionSource::ExplicitEmpty;
				}
			}
			else
			{
				switch (MissingMountSelectionPolicy)
				{
				case ECFMissingMountPolicy::UseVehicleDefault:
					ResolvedMount.SelectionSource = ECFFittingSelectionSource::VehicleDefault;
					ResolvedEquipmentPresetData = MountProfile.DefaultEquipmentPresetData;
					break;
				case ECFMissingMountPolicy::TreatAsEmpty:
					ResolvedMount.SelectionSource = ECFFittingSelectionSource::MissingPolicyEmpty;
					break;
				case ECFMissingMountPolicy::TreatAsError:
				default:
					AddFittingValidationIssue(
						Snapshot,
						ECFFittingIssueSeverity::Error,
						ECFFittingIssueCode::MissingMountSelection,
						FText::Format(
							LOCTEXT("MissingMountSelectionIssue", "MountProfileId '{0}'의 장착 선택이 없고 누락 정책이 TreatAsError입니다."),
							FText::FromName(MountProfile.MountProfileId)),
						MountProfile.MountProfileId);
					break;
				}
			}
		}

						ResolvedMount.EquipmentPresetData = ResolvedEquipmentPresetData;

		// [v1.1.0] 명시적 또는 정책상 빈 장착인지 여부입니다.
		const bool bResolvedAsEmpty = ResolvedMount.SelectionSource == ECFFittingSelectionSource::ExplicitEmpty
			|| ResolvedMount.SelectionSource == ECFFittingSelectionSource::MissingPolicyEmpty;

		if (!ResolvedEquipmentPresetData)
		{
			if (!bResolvedAsEmpty && ResolvedMount.SelectionSource != ECFFittingSelectionSource::None)
			{
				AddFittingValidationIssue(
					Snapshot,
					ECFFittingIssueSeverity::Error,
					ECFFittingIssueCode::MissingEquipmentPreset,
					FText::Format(
						LOCTEXT("MissingEquipmentPresetIssue", "MountProfileId '{0}'에서 해석된 EquipmentPresetData가 없습니다."),
						FText::FromName(MountProfile.MountProfileId)),
					MountProfile.MountProfileId);
			}

			Snapshot.ResolvedMounts.Add(ResolvedMount);
			continue;
		}

								ResolvedMount.TurretMountData = ResolvedEquipmentPresetData->DefaultTurretMountData;
		ResolvedMount.WeaponData = ResolvedEquipmentPresetData->DefaultWeaponData;
		ResolvedMount.SensorData = ResolvedEquipmentPresetData->DefaultSensorData;

		if (!ResolvedEquipmentPresetData->HasCompleteEquipmentDataForMount(MountProfile.MountType))
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::IncompleteEquipmentPreset,
				FText::Format(
					LOCTEXT("IncompleteEquipmentPresetIssue", "MountProfileId '{0}'의 EquipmentPresetData가 해당 장착 타입에 필요한 단일 payload 계약을 만족하지 않습니다."),
					FText::FromName(MountProfile.MountProfileId)),
				MountProfile.MountProfileId);
		}

		if (ResolvedMount.SensorData && !ResolvedMount.SensorData->IsSensorConfigValid())
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidSensorData,
				FText::Format(
					LOCTEXT("InvalidSensorDataIssue", "MountProfileId '{0}'의 VehicleSensorData SensorConfig가 유효하지 않습니다."),
					FText::FromName(MountProfile.MountProfileId)),
				MountProfile.MountProfileId);
						}

		if (ResolvedEquipmentPresetData->RequiredMountType != ECFVehicleMountType::None
			&& ResolvedEquipmentPresetData->RequiredMountType != MountProfile.MountType)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::MountTypeMismatch,
				FText::Format(
					LOCTEXT("MountTypeMismatchIssue", "MountProfileId '{0}'의 타입과 EquipmentPresetData 요구 장착 타입이 일치하지 않습니다."),
					FText::FromName(MountProfile.MountProfileId)),
				MountProfile.MountProfileId);
		}

								// [v1.3.0] 실제 WeaponData가 존재하는 무장 패키지에서만 무기 크기 제한을 계산합니다.
		bool bWeaponSizeExceeded = false;

		if (ResolvedEquipmentPresetData->DefaultWeaponData)
		{
			bWeaponSizeExceeded = DoesWeaponSizeExceedLimit(
				ResolvedEquipmentPresetData->RequiredWeaponSize,
				MountProfile.SizeLimit);

			if (!ResolvedEquipmentPresetData->DefaultWeaponData->SupportsMountType(MountProfile.MountType))
			{
				AddFittingValidationIssue(
					Snapshot,
					ECFFittingIssueSeverity::Error,
					ECFFittingIssueCode::WeaponMountIncompatible,
					FText::Format(
						LOCTEXT("WeaponMountIncompatibleIssue", "MountProfileId '{0}'의 WeaponData가 장착 타입을 지원하지 않습니다."),
						FText::FromName(MountProfile.MountProfileId)),
					MountProfile.MountProfileId);
			}

			bWeaponSizeExceeded = bWeaponSizeExceeded
				|| !ResolvedEquipmentPresetData->DefaultWeaponData->SupportsWeaponSize(MountProfile.SizeLimit);
		}

		if (bWeaponSizeExceeded)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::WeaponSizeExceeded,
				FText::Format(
					LOCTEXT("WeaponSizeExceededIssue", "MountProfileId '{0}'의 장비 또는 무기 크기가 SizeLimit을 초과합니다."),
					FText::FromName(MountProfile.MountProfileId)),
				MountProfile.MountProfileId);
		}

		if (ResolvedEquipmentPresetData->DefaultTurretMountData)
		{
			ResolvedMount.TurretMountMassKg = ResolveRequiredFittingMass(
				Snapshot,
				ResolvedEquipmentPresetData->DefaultTurretMountData->TurretMountWeightKg,
				LOCTEXT("TurretMountMassDisplayName", "터렛 마운트 질량"),
				MountProfile.MountProfileId);
		}

		if (ResolvedEquipmentPresetData->DefaultWeaponData)
		{
			ResolvedMount.WeaponMassKg = ResolveRequiredFittingMass(
				Snapshot,
				ResolvedEquipmentPresetData->DefaultWeaponData->WeaponMassKg,
				LOCTEXT("WeaponMassDisplayName", "무기 질량"),
				MountProfile.MountProfileId);
		}

										Snapshot.EquipmentMassKg += ResolvedMount.TurretMountMassKg + ResolvedMount.WeaponMassKg;

		if (ResolvedMount.SensorData)
		{
			++ResolvedSensorSourceCount;
			if (ResolvedSensorSourceCount == 1)
			{
				Snapshot.ResolvedSensorData = ResolvedMount.SensorData;
			}
			else
			{
				Snapshot.ResolvedSensorData = nullptr;
				AddFittingValidationIssue(
					Snapshot,
					ECFFittingIssueSeverity::Error,
					ECFFittingIssueCode::MultipleSensorSources,
					FText::Format(
						LOCTEXT("MultipleSensorSourcesIssue", "MountProfileId '{0}'까지 둘 이상의 Scanner SensorData가 해석되어 단일 Sensor Runtime Source를 결정할 수 없습니다."),
						FText::FromName(MountProfile.MountProfileId)),
					MountProfile.MountProfileId);
			}
		}

				Snapshot.ResolvedMounts.Add(ResolvedMount);
	}

	// [v1.2.0] 편집 배열 순서와 무관하게 AmmoId 이름 순서로 Snapshot에 복사할 출격 탄약 목록입니다.
	TArray<FCFAmmoSortieLoad> SortedSortieAmmoLoads = InitialSortieAmmoLoads;
	SortedSortieAmmoLoads.Sort([](const FCFAmmoSortieLoad& LeftLoad, const FCFAmmoSortieLoad& RightLoad)
	{
		// [v1.2.0] 정렬에 사용할 왼쪽 탄종의 안정 ID 문자열입니다.
		const FString LeftAmmoIdText = IsValid(LeftLoad.AmmoData) ? LeftLoad.AmmoData->AmmoId.ToString() : FString();

		// [v1.2.0] 정렬에 사용할 오른쪽 탄종의 안정 ID 문자열입니다.
		const FString RightAmmoIdText = IsValid(RightLoad.AmmoData) ? RightLoad.AmmoData->AmmoId.ToString() : FString();
		return LeftAmmoIdText < RightAmmoIdText;
	});

	// [v1.2.0] 검증된 AmmoId별 실제 출격 총수량을 finite 무기 초기 장전 요구와 비교할 맵입니다.
	TMap<FName, int32> SortieAmmoCountByAmmoId;

	// [v1.2.0] 같은 AmmoId가 중복돼 질량과 수량을 이중 계산하지 않도록 추적할 집합입니다.
	TSet<FName> ProcessedAmmoIds;
	for (const FCFAmmoSortieLoad& AmmoLoad : SortedSortieAmmoLoads)
	{
		UCFAmmoData* AmmoData = AmmoLoad.AmmoData;
		if (!IsValid(AmmoData) || !AmmoData->IsAmmoDataValid())
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				LOCTEXT("InvalidAmmoSelectionData", "출격 탄약 선택의 AmmoData와 AmmoId는 유효해야 합니다."));
			continue;
		}

		if (ProcessedAmmoIds.Contains(AmmoData->AmmoId))
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				FText::Format(
					LOCTEXT("DuplicateAmmoSelectionIssue", "AmmoId '{0}'가 출격 탄약 목록에 중복되어 있어 이중 계산하지 않습니다."),
					FText::FromName(AmmoData->AmmoId)));
			continue;
		}
		ProcessedAmmoIds.Add(AmmoData->AmmoId);

		if (AmmoLoad.InitialSortieAmmoCount < 0)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				FText::Format(
					LOCTEXT("NegativeAmmoSelectionIssue", "AmmoId '{0}'의 출격 수량은 0 이상이어야 합니다."),
					FText::FromName(AmmoData->AmmoId)));
			continue;
		}

		// [v1.2.0] 피팅에서 허용하는 해당 탄종의 최대 적재 수량입니다. 현재 출격 수량을 생성하는 값이 아닙니다.
		const int32 MaximumLoadableAmmoCount = AmmoData->GetEffectiveMaximumLoadableAmmoCount();
		if (AmmoLoad.InitialSortieAmmoCount > MaximumLoadableAmmoCount)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::AmmoCountExceeded,
				FText::Format(
					LOCTEXT("AmmoCountExceededIssue", "AmmoId '{0}'의 출격 수량 {1}이 최대 적재 가능 수량 {2}를 초과합니다."),
					FText::FromName(AmmoData->AmmoId),
					FText::AsNumber(AmmoLoad.InitialSortieAmmoCount),
					FText::AsNumber(MaximumLoadableAmmoCount)));
		}

		// [v1.2.0] 실제 적재 수량이 있을 때만 질량 소스로 요구되는 탄약 한 단위 질량입니다.
		const float UnitMassKg = AmmoData->GetEffectiveUnitMassKg();
		if (AmmoLoad.InitialSortieAmmoCount > 0 && UnitMassKg <= 0.0f)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				FText::Format(
					LOCTEXT("InvalidAmmoUnitMassIssue", "AmmoId '{0}'를 1발 이상 적재하려면 UnitMassKg가 유한한 0보다 큰 값이어야 합니다."),
					FText::FromName(AmmoData->AmmoId)));
		}

		Snapshot.InitialSortieAmmoLoads.Add(AmmoLoad);
		SortieAmmoCountByAmmoId.Add(AmmoData->AmmoId, AmmoLoad.InitialSortieAmmoCount);

		// [v1.2.0] 실제 출격 수량만 질량에 반영한 현재 탄종의 총질량입니다.
		const float AmmoLoadMassKg = static_cast<float>(AmmoLoad.InitialSortieAmmoCount) * UnitMassKg;
		if (!FMath::IsFinite(AmmoLoadMassKg) || AmmoLoadMassKg < 0.0f)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidMassValue,
				FText::Format(
					LOCTEXT("InvalidAmmoLoadMassIssue", "AmmoId '{0}'의 출격 탄약 총질량이 유효하지 않습니다."),
					FText::FromName(AmmoData->AmmoId)));
		}
		else
		{
			Snapshot.AmmoMassKg += AmmoLoadMassKg;
		}
	}

	// [v1.2.0] 같은 탄종을 사용하는 모든 finite WeaponInstance의 출격 초기 장전량 합계입니다.
	TMap<FName, int32> RequiredInitialLoadedAmmoByAmmoId;
	for (const FCFResolvedFittingMount& ResolvedMount : Snapshot.ResolvedMounts)
	{
		UCFWeaponData* WeaponData = ResolvedMount.WeaponData;
		if (!IsValid(WeaponData) || WeaponData->bUseInfiniteAmmoForDebug)
		{
			continue;
		}

		if (!WeaponData->UsesFiniteAmmoRuntime() || !IsValid(WeaponData->DefaultAmmoData))
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				FText::Format(
					LOCTEXT("IncompleteFiniteWeaponAmmoIssue", "MountProfileId '{0}'의 WeaponData가 finite 탄약을 요구하지만 AmmoData 또는 탄창 설정이 불완전합니다."),
					FText::FromName(ResolvedMount.MountProfileId)),
				ResolvedMount.MountProfileId);
			continue;
		}

		// [v1.2.0] 현재 finite WeaponData가 사용하는 안정 AmmoId입니다.
		const FName RequiredAmmoId = WeaponData->DefaultAmmoData->AmmoId;
		RequiredInitialLoadedAmmoByAmmoId.FindOrAdd(RequiredAmmoId) += WeaponData->GetEffectiveInitialLoadedAmmoCount();

		if (!SortieAmmoCountByAmmoId.Contains(RequiredAmmoId))
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				FText::Format(
					LOCTEXT("MissingFiniteWeaponAmmoLoadIssue", "MountProfileId '{0}'의 finite WeaponData가 요구하는 AmmoId '{1}'가 출격 탄약 목록에 명시되지 않았습니다."),
					FText::FromName(ResolvedMount.MountProfileId),
					FText::FromName(RequiredAmmoId)),
				ResolvedMount.MountProfileId);
		}
	}

	for (const TPair<FName, int32>& RequiredInitialLoadedAmmoPair : RequiredInitialLoadedAmmoByAmmoId)
	{
		// [v1.2.0] 같은 탄종 finite 무기들의 초기 장전에 필요한 전체 탄약 수입니다.
		const int32 RequiredInitialLoadedAmmoCount = FMath::Max(RequiredInitialLoadedAmmoPair.Value, 0);

		// [v1.2.0] 이번 출격에 명시적으로 선택된 해당 탄종의 실제 전체 수량입니다.
		const int32 SelectedSortieAmmoCount = SortieAmmoCountByAmmoId.FindRef(RequiredInitialLoadedAmmoPair.Key);
		if (!SortieAmmoCountByAmmoId.Contains(RequiredInitialLoadedAmmoPair.Key)
			|| SelectedSortieAmmoCount < RequiredInitialLoadedAmmoCount)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::InvalidAmmoSelection,
				FText::Format(
					LOCTEXT("InitialLoadedAmmoExceedsSortieIssue", "AmmoId '{0}'의 출격 수량 {1}이 finite 무기 초기 장전 필요량 {2}보다 적습니다."),
					FText::FromName(RequiredInitialLoadedAmmoPair.Key),
					FText::AsNumber(SelectedSortieAmmoCount),
					FText::AsNumber(RequiredInitialLoadedAmmoCount)));
		}
	}

	switch (DefenseSelection.SelectionMode)
	{
	case ECFDefenseSelectionMode::UseVehicleDefault:
		Snapshot.ResolvedDefenseData = VehicleData->DefaultDefenseData;
		break;
	case ECFDefenseSelectionMode::ExplicitNone:
		Snapshot.ResolvedDefenseData = nullptr;
		break;
	case ECFDefenseSelectionMode::Override:
		Snapshot.ResolvedDefenseData = DefenseSelection.DefenseData;
		if (!Snapshot.ResolvedDefenseData)
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::MissingDefenseData,
				LOCTEXT("MissingOverrideDefenseIssue", "DefenseSelection이 Override이지만 DefenseData가 지정되지 않았습니다."));
		}
		break;
	default:
		Snapshot.ResolvedDefenseData = nullptr;
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::MissingDefenseData,
			LOCTEXT("UnknownDefenseSelectionMode", "알 수 없는 방어 선택 방식입니다."));
		break;
	}

	if (Snapshot.ResolvedDefenseData)
	{
		Snapshot.DefenseMassKg = ResolveRequiredFittingMass(
			Snapshot,
			Snapshot.ResolvedDefenseData->DefenseMassKg,
			LOCTEXT("DefenseMassDisplayName", "방어 패키지 질량"));
	}

	Snapshot.PayloadMassKg = Snapshot.EquipmentMassKg + Snapshot.AmmoMassKg + Snapshot.DefenseMassKg;
	Snapshot.TotalVehicleMassKg = Snapshot.BaseVehicleMassKg + Snapshot.PayloadMassKg;

	// [v1.1.0] VehicleData 총중량 한도에서 파생한 최대 탑재 가능 질량입니다.
	const float MaximumPayloadMassKg = Snapshot.MaximumGrossMassKg - Snapshot.BaseVehicleMassKg;
	if (MaximumPayloadMassKg > KINDA_SMALL_NUMBER)
	{
		Snapshot.PayloadUsageRatio = Snapshot.PayloadMassKg / MaximumPayloadMassKg;
	}

	if (Snapshot.MaximumGrossMassKg > KINDA_SMALL_NUMBER)
	{
		Snapshot.GrossMassUsageRatio = Snapshot.TotalVehicleMassKg / Snapshot.MaximumGrossMassKg;
	}

	if (Snapshot.MaximumGrossMassKg > 0.0f
		&& Snapshot.TotalVehicleMassKg > Snapshot.MaximumGrossMassKg + KINDA_SMALL_NUMBER)
	{
		AddFittingValidationIssue(
			Snapshot,
			ECFFittingIssueSeverity::Error,
			ECFFittingIssueCode::GrossMassExceeded,
			FText::Format(
				LOCTEXT("GrossMassExceededIssue", "차량 총중량 {0}kg이 최대 허용 총중량 {1}kg을 초과합니다."),
				FText::AsNumber(Snapshot.TotalVehicleMassKg),
				FText::AsNumber(Snapshot.MaximumGrossMassKg)));
	}

	FinalizeFittingValidationState(Snapshot);
	return Snapshot;
}

// [v1.0.0] 디버그와 로그에서 사용할 피팅 선택 요약 문자열을 생성합니다.
FString UCFVehicleFittingData::BuildVehicleFittingSummary() const
{
	// [v1.0.0] 기준 VehicleData 에셋 이름 또는 미지정 표시입니다.
	const FString VehicleDataText = VehicleData ? VehicleData->GetName() : TEXT("MissingRequired");

	// [v1.0.0] 누락 장착 선택 정책을 표시할 enum 문자열입니다.
	const FString MissingMountPolicyText = UEnum::GetValueAsString(MissingMountSelectionPolicy);

	// [v1.0.0] 방어 선택 방식을 표시할 enum 문자열입니다.
	const FString DefenseSelectionModeText = UEnum::GetValueAsString(DefenseSelection.SelectionMode);

	// [v1.0.0] Override 방어 DataAsset 이름 또는 현재 선택 방식에 맞는 표시입니다.
	const FString DefenseDataText = DefenseSelection.DefenseData
		? DefenseSelection.DefenseData->GetName()
		: TEXT("None");

		return FString::Printf(
		TEXT("VehicleFittingData: Id=%s, Name=%s, Vehicle=%s, MountSelections=%d, AmmoLoads=%d, MissingMountPolicy=%s, DefenseMode=%s, DefenseData=%s, Tags=%d"),
		*FittingId.ToString(),
		*DisplayName.ToString(),
		*VehicleDataText,
		MountSelections.Num(),
		InitialSortieAmmoLoads.Num(),
		*MissingMountPolicyText,
		*DefenseSelectionModeText,
		*DefenseDataText,
		FittingTags.Num());
}

#if WITH_EDITOR
// [v1.1.0] Unreal Data Validation에서 기본 계약과 결정론적 Snapshot 검증 문제를 보고합니다.
EDataValidationResult UCFVehicleFittingData::IsDataValid(FDataValidationContext& Context) const
{
	// [v1.1.0] 상위 PrimaryDataAsset이 반환한 기본 검증 결과입니다.
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);

	// [v1.1.0] 피팅 데이터의 직렬화·필수 참조 계약에서 발견된 오류 목록입니다.
	TArray<FText> ValidationErrors;
	if (!ValidateFittingDataContract(ValidationErrors))
	{
		// [v1.1.0] Unreal Data Validation Context에 전달할 한 기본 계약 오류입니다.
		for (const FText& ValidationError : ValidationErrors)
		{
			Context.AddError(ValidationError);
		}

		return EDataValidationResult::Invalid;
	}

	// [v1.1.0] 실제 Mount·Defense·질량 해석 결과를 포함한 결정론적 Snapshot입니다.
	const FCFVehicleFittingSnapshot Snapshot = BuildFittingSnapshot();

	// [v1.1.0] Unreal Data Validation Context에 전달할 한 구조화된 Snapshot 문제입니다.
	for (const FCFFittingValidationIssue& ValidationIssue : Snapshot.ValidationIssues)
	{
		if (ValidationIssue.Severity == ECFFittingIssueSeverity::Error)
		{
			Context.AddError(ValidationIssue.Message);
		}
		else if (ValidationIssue.Severity == ECFFittingIssueSeverity::Warning)
		{
			Context.AddWarning(ValidationIssue.Message);
		}
	}

	if (!Snapshot.IsValid())
	{
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
