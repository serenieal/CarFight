// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-034 FIT-P0-06 차량 피팅 읽기 전용 ViewData·Debug 구현
// Scope: FCFVehicleFittingSnapshot의 최종 해석 결과를 Mount·Defense·Ammo·Mass·Mobility·Issue ViewData로 부작용 없이 투영합니다.
// Changelog:
// - v1.0.0: Snapshot 순서·값 보존, Vehicle Asset 이름 fallback, PhysicalMassOnly Mobility Preview와 DebugSummary Builder를 최초 구현.
// Migration:
// - Snapshot 호환성·Validation·질량 Breakdown을 다시 계산하지 않습니다.
// - Ammo row는 실제 출격 Count와 UnitMass만 표시하고 전체 AmmoMassKg는 Snapshot aggregate를 사용합니다.
// - 추가 Mobility Adapter가 생기기 전까지 축 Scale은 1.0이며 PreviewMode=PhysicalMassOnly로 명시합니다.

#include "CFFittingViewData.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"

namespace
{
	// [v1.0.0] Snapshot 질량 값을 재계산하지 않고 표시 행 하나로 추가합니다.
	void AddMassViewRow(
		TArray<FCFFittingMassViewRow>& MassRows,
		const ECFFittingMassRowType MassRowType,
		const float MassKg)
	{
		// [v1.0.0] Snapshot 질량 값과 표시 종류를 묶을 읽기 전용 행입니다.
		FCFFittingMassViewRow MassRow;
		MassRow.MassRowType = MassRowType;
		MassRow.MassKg = MassKg;
		MassRows.Add(MassRow);
	}
}

// [v1.0.0] Snapshot을 변경하지 않고 Blueprint/UI용 ViewData를 결정론적으로 생성합니다.
FCFVehicleFittingViewData FCFFittingViewBuilder::Build(const FCFVehicleFittingSnapshot& FittingSnapshot)
{
	// [v1.0.0] UI와 Debug가 Snapshot 내부 구조를 직접 읽지 않고 소비할 최종 ViewData입니다.
	FCFVehicleFittingViewData ViewData;
	ViewData.VehicleData = FittingSnapshot.VehicleData;
	if (FittingSnapshot.VehicleData)
	{
		ViewData.VehicleDisplayName = FText::FromString(FittingSnapshot.VehicleData->GetName());
		ViewData.bVehicleDisplayNameUsesAssetName = true;
	}

	ViewData.FittingId = FittingSnapshot.FittingId;
	ViewData.ValidationState = FittingSnapshot.ValidationState;
	ViewData.bCanApply = FittingSnapshot.IsValid();

	ViewData.MountRows.Reserve(FittingSnapshot.ResolvedMounts.Num());
	for (const FCFResolvedFittingMount& ResolvedMount : FittingSnapshot.ResolvedMounts)
	{
		// [v1.0.0] 한 ResolvedMount의 최종 값과 참조를 순서 그대로 표시할 행입니다.
		FCFFittingMountViewRow MountRow;
		MountRow.MountProfileId = ResolvedMount.MountProfileId;
		MountRow.LocationSlotId = ResolvedMount.LocationSlotId;
		MountRow.MountType = ResolvedMount.MountType;
		MountRow.SizeLimit = ResolvedMount.SizeLimit;
		MountRow.EquipmentPresetData = ResolvedMount.EquipmentPresetData;
		MountRow.EquipmentDisplayName = ResolvedMount.EquipmentPresetData
			? ResolvedMount.EquipmentPresetData->DisplayName
			: FText::GetEmpty();
		MountRow.TurretMountData = ResolvedMount.TurretMountData;
		MountRow.WeaponData = ResolvedMount.WeaponData;
		MountRow.SelectionSource = ResolvedMount.SelectionSource;
		MountRow.TurretMountMassKg = ResolvedMount.TurretMountMassKg;
		MountRow.WeaponMassKg = ResolvedMount.WeaponMassKg;
		ViewData.MountRows.Add(MountRow);
	}

	ViewData.DefenseRow.SelectionMode = FittingSnapshot.ResolvedDefenseSelectionMode;
	ViewData.DefenseRow.DefenseData = FittingSnapshot.ResolvedDefenseData;
	ViewData.DefenseRow.DefenseId = FittingSnapshot.ResolvedDefenseData
		? FittingSnapshot.ResolvedDefenseData->DefenseId
		: NAME_None;
	ViewData.DefenseRow.DefenseMassKg = FittingSnapshot.DefenseMassKg;

	ViewData.AmmoRows.Reserve(FittingSnapshot.InitialSortieAmmoLoads.Num());
	for (const FCFAmmoSortieLoad& AmmoSortieLoad : FittingSnapshot.InitialSortieAmmoLoads)
	{
		// [v1.0.0] 실제 출격 Ammo 선택을 최대 적재량과 혼동하지 않고 표시할 행입니다.
		FCFFittingAmmoViewRow AmmoRow;
		AmmoRow.AmmoData = AmmoSortieLoad.AmmoData;
		AmmoRow.InitialSortieAmmoCount = AmmoSortieLoad.InitialSortieAmmoCount;
		if (AmmoSortieLoad.AmmoData)
		{
			AmmoRow.AmmoId = AmmoSortieLoad.AmmoData->AmmoId;
			AmmoRow.AmmoDisplayName = AmmoSortieLoad.AmmoData->AmmoDisplayName;
			AmmoRow.UnitMassKg = AmmoSortieLoad.AmmoData->GetEffectiveUnitMassKg();
		}
		ViewData.AmmoRows.Add(AmmoRow);
	}

	ViewData.MassRows.Reserve(7);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::BaseVehicle, FittingSnapshot.BaseVehicleMassKg);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::Equipment, FittingSnapshot.EquipmentMassKg);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::Ammo, FittingSnapshot.AmmoMassKg);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::Defense, FittingSnapshot.DefenseMassKg);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::Payload, FittingSnapshot.PayloadMassKg);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::TotalVehicle, FittingSnapshot.TotalVehicleMassKg);
	AddMassViewRow(ViewData.MassRows, ECFFittingMassRowType::MaximumGross, FittingSnapshot.MaximumGrossMassKg);

	ViewData.TotalVehicleMassKg = FittingSnapshot.TotalVehicleMassKg;
	ViewData.MaximumGrossMassKg = FittingSnapshot.MaximumGrossMassKg;
	ViewData.PayloadUsageRatio = FittingSnapshot.PayloadUsageRatio;
	ViewData.GrossMassUsageRatio = FittingSnapshot.GrossMassUsageRatio;

	// [v1.0.0] 추가 Mobility Adapter가 없는 현재 P0에서도 실제 물리 질량 Preview를 설명할 질량 비율입니다.
	const bool bMassRatioAvailable = FMath::IsFinite(FittingSnapshot.BaseVehicleMassKg)
		&& FittingSnapshot.BaseVehicleMassKg > 0.0f
		&& FMath::IsFinite(FittingSnapshot.TotalVehicleMassKg)
		&& FittingSnapshot.TotalVehicleMassKg >= 0.0f;
	if (bMassRatioAvailable)
	{
		ViewData.MobilityPreview.PreviewMode = ECFFittingMobilityPreviewMode::PhysicalMassOnly;
		ViewData.MobilityPreview.MassRatio = FittingSnapshot.TotalVehicleMassKg / FittingSnapshot.BaseVehicleMassKg;
	}
	else
	{
		ViewData.MobilityPreview.PreviewMode = ECFFittingMobilityPreviewMode::Unavailable;
		ViewData.MobilityPreview.MassRatio = 0.0f;
	}
	ViewData.MobilityPreview.AccelerationScale = 1.0f;
	ViewData.MobilityPreview.BrakingScale = 1.0f;
	ViewData.MobilityPreview.SteeringResponseScale = 1.0f;
	ViewData.MobilityPreview.bAdditionalAdapterApplied = false;

	ViewData.IssueRows = FittingSnapshot.ValidationIssues;
	ViewData.DebugSummary = FString::Printf(
		TEXT("FittingView: Fitting=%s, Validation=%d, CanApply=%s, Mounts=%d, Ammo=%d, Issues=%d, Mass=%.3f/%.3fkg, PayloadRatio=%.4f, MassRatio=%.4f, Mobility=%s"),
		*ViewData.FittingId.ToString(),
		static_cast<int32>(ViewData.ValidationState),
		ViewData.bCanApply ? TEXT("True") : TEXT("False"),
		ViewData.MountRows.Num(),
		ViewData.AmmoRows.Num(),
		ViewData.IssueRows.Num(),
		ViewData.TotalVehicleMassKg,
		ViewData.MaximumGrossMassKg,
		ViewData.PayloadUsageRatio,
		ViewData.MobilityPreview.MassRatio,
		ViewData.MobilityPreview.PreviewMode == ECFFittingMobilityPreviewMode::PhysicalMassOnly
			? TEXT("PhysicalMassOnly")
			: TEXT("Unavailable"));
	return ViewData;
}
