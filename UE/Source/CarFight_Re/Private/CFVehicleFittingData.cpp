// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-01
// Description: CarFight 출격 전 차량 피팅 선택과 결정론적 Snapshot 구현
// Scope: 피팅 기본값, 데이터 계약, MountProfile·Hardpoint·장비·방어 호환 검증과 Pawn 없는 질량 Snapshot 생성을 구현합니다.
// Changelog:
// - v1.1.0: CF-FQ-034 FIT-P0-03 결정론적 Compatibility Validation, 방어 3상태 해석, 질량 합산과 GrossMass 검증을 추가.
// - v1.0.0: CF-FQ-034 FIT-P0-02 VehicleFittingData 계약 검증과 요약을 최초 구현.
// Migration:
// - BuildFittingSnapshot은 VehicleData.MountProfiles 순서로 결과를 생성하므로 MountSelections 배열 순서가 결과 순서를 바꾸지 않는다.
// - 기존 VehicleData와 차량 런타임은 VehicleFittingData를 자동 참조하지 않으므로 동작이 변경되지 않는다.
// - BaseVehicleMassKg와 MaximumGrossMassKg가 0인 기존 VehicleData는 피팅에 명시적으로 연결할 때만 계약 오류가 된다.
// - AmmoMassKg는 CF-FQ-031 연동 전까지 0으로 유지하며 임시 Ammo 타입을 만들지 않는다.
// - VehiclePawn, VehicleMovement와 Chaos 질량 적용은 FIT-P0-04~05 범위로 유지한다.

#include "CFVehicleFittingData.h"

#include "CFEquipmentPresetData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
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

		if (!MountSelection.EquipmentPresetData->HasCompleteEquipmentData())
		{
			OutValidationErrors.Add(FText::Format(
				LOCTEXT("IncompleteEquipmentPreset", "MountProfileId '{0}'의 EquipmentPresetData에 TurretMountData와 WeaponData가 모두 필요합니다."),
				FText::FromName(MountSelection.MountProfileId)));
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

		if (!ResolvedEquipmentPresetData->HasCompleteEquipmentData())
		{
			AddFittingValidationIssue(
				Snapshot,
				ECFFittingIssueSeverity::Error,
				ECFFittingIssueCode::IncompleteEquipmentPreset,
				FText::Format(
					LOCTEXT("IncompleteEquipmentPresetIssue", "MountProfileId '{0}'의 EquipmentPresetData에 TurretMountData와 WeaponData가 모두 필요합니다."),
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

		// [v1.1.0] 프리셋 또는 무기 자체의 크기가 장착 제한을 초과하는지 여부입니다.
		bool bWeaponSizeExceeded = DoesWeaponSizeExceedLimit(
			ResolvedEquipmentPresetData->RequiredWeaponSize,
			MountProfile.SizeLimit);

		if (ResolvedEquipmentPresetData->DefaultWeaponData)
		{
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
		Snapshot.ResolvedMounts.Add(ResolvedMount);
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
		TEXT("VehicleFittingData: Id=%s, Name=%s, Vehicle=%s, MountSelections=%d, MissingMountPolicy=%s, DefenseMode=%s, DefenseData=%s, Tags=%d"),
		*FittingId.ToString(),
		*DisplayName.ToString(),
		*VehicleDataText,
		MountSelections.Num(),
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
