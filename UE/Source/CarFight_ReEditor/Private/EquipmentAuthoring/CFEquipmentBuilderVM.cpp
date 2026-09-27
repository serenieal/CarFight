// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentBuilderVM.cpp
// Version: v1.6.1
// Date: 2026-09-17
// Description: CF-FQ-054 Equipment Builder durable authoring, USER UX correction과 read-only Vehicle Mount compatibility probe 구현입니다.
// Changelog:
// - v1.6.1: 신규 Draft의 초안 되돌리기에서 에셋 이름과 자동 Create target도 함께 초기화해 EquipmentId/path 표시 불일치를 제거.
// - v1.6.0: 기존 EquipmentPreset의 Weapon/Scanner 종류 변경을 차단하고 신규 에셋 이름에서 EquipmentId와 EquipmentPresets canonical target을 자동 파생.
// - v1.5.2: 기존 Weapon/Sensor native validator의 영문 필드 식별자를 Equipment Builder 표시 단계에서 한글 용어로 변환.
// - v1.5.1: USER 화면으로 전달되는 child/reference/review validation 문구를 한글 중심으로 정리.
// - v1.5.0: localized DisplayName preserve-only 편집 보호, Scanner Utility 고정과 Weapon mount-type session cache를 추가.
// - v1.4.0: transient VehicleDataObjectPath + MountProfileId, fresh exact1 Mount lookup, exact3 compatibility advisory와 durable gate 완전 분리를 추가.
// - v1.3.0: exact7 ReviewProposalDigest, complete inventory + loaded-only identity union, persisted-clean child guard, one-shot approval와 CFDADurableCore apply orchestration을 추가.
// - v1.2.0: intrinsic DraftMode/RequiredMountType/WeaponData mount contradiction을 blocking validation에 추가하고 mode별 transient selection cache로 전환 선택 손실을 제거.
// - v1.1.0: Weapon/Scanner child reference selection, mixed-payload transient 예방과 Weapon/Sensor native + Turret/Projectile/Damage bounded validation을 추가.
// - v1.0.0: existing Data Asset Manager metadata inventory를 재사용하고 existing/new EquipmentPreset을 UObject mutation 없이 transient draft로 관리.
// Migration:
// - exact EquipmentPreset durable sequencing은 existing CFDADurableCore::ApplyTypedTarget만 사용합니다.
// - localized DisplayName preserve mode는 해당 필드를 다시 쓰지 않으며 Runtime/Product schema를 변경하지 않습니다.
// - Scanner Draft는 Builder policy상 Utility mount + size None으로 고정되며 기존 persisted Asset의 Weapon/Scanner 종류는 변경할 수 없습니다.
// - 신규 Product EquipmentPreset은 사용자용 에셋 이름에서 EquipmentId와 `/Game/CarFight/Weapons/Data/EquipmentPresets` target을 자동 파생합니다.
// - compatibility probe는 Editor-only transient advisory이며 semantic fingerprint, ReviewProposalDigest, Apply readiness와 durable mutation path를 변경하지 않습니다.
// - VehicleData/Fitting/RuntimeApply는 read-only/no-call 경계입니다.

#include "EquipmentAuthoring/CFEquipmentBuilderVM.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFEquipmentPresetData.h"
#include "CFProjectileData.h"
#include "CFVehicleData.h"
#include "CFTurretMountData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFDADurableCore.h"
#include "DataAuthoring/CFDAStagingApply.h"
#include "DataAuthoring/CFDATypeDispatch.h"
#include "EquipmentAuthoring/CFEquipmentDurable.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace
{
	// 신규 Product EquipmentPreset을 저장하는 canonical content root입니다.
	const FString EquipmentPresetProductRoot(TEXT("/Game/CarFight/Weapons/Data/EquipmentPresets"));

	// Equipment Builder browser가 허용하는 exact Runtime class path를 반환합니다.
	FString GetEquipmentPresetClassPath()
	{
		return UCFEquipmentPresetData::StaticClass()->GetClassPathName().ToString();
	}

	// Runtime native validator의 기술 필드명을 Equipment Builder 사용자 표시용 한글 용어로 변환합니다.
	FString BuildUserValidationMessage(const FString& TechnicalMessage)
	{
		// 원본 validator 의미는 유지하면서 사용자에게 보여줄 복사본입니다.
		FString UserMessage = TechnicalMessage;
		UserMessage.ReplaceInline(TEXT("WeaponId"), TEXT("무기 ID"));
		UserMessage.ReplaceInline(TEXT("WeaponSize"), TEXT("무기 크기"));
		UserMessage.ReplaceInline(TEXT("CompatibleMountTypes"), TEXT("호환 장착 타입"));
		UserMessage.ReplaceInline(TEXT("WeaponMassKg"), TEXT("무기 질량"));
		UserMessage.ReplaceInline(TEXT("FireRatePerMinute"), TEXT("분당 발사속도"));
		UserMessage.ReplaceInline(TEXT("MaxRange"), TEXT("최대 사거리"));
		UserMessage.ReplaceInline(TEXT("SpreadDeg"), TEXT("탄퍼짐 각도"));
		UserMessage.ReplaceInline(TEXT("MagazineSize"), TEXT("탄창 용량"));
		UserMessage.ReplaceInline(TEXT("ReloadTimeSeconds"), TEXT("재장전 시간"));
		UserMessage.ReplaceInline(TEXT("InitialLoadedAmmoCount"), TEXT("초기 장전량"));
		UserMessage.ReplaceInline(TEXT("AmmoUnitsPerShot"), TEXT("발사당 탄약 소모량"));
		UserMessage.ReplaceInline(TEXT("DefaultAmmoData"), TEXT("기본 탄약 데이터"));
		UserMessage.ReplaceInline(TEXT("AmmoId"), TEXT("탄약 ID"));
		UserMessage.ReplaceInline(TEXT("MaximumWeaponCharge"), TEXT("최대 무기 충전량"));
		UserMessage.ReplaceInline(TEXT("InitialWeaponCharge"), TEXT("초기 무기 충전량"));
		UserMessage.ReplaceInline(TEXT("WeaponChargePerShot"), TEXT("발사당 무기 충전 소모량"));
		UserMessage.ReplaceInline(TEXT("WeaponChargeRecoveryPerSecond"), TEXT("초당 무기 충전 회복량"));
		UserMessage.ReplaceInline(TEXT("HeatPerShot"), TEXT("발사당 열 발생량"));
		UserMessage.ReplaceInline(TEXT("MaxHeat"), TEXT("최대 열"));
		UserMessage.ReplaceInline(TEXT("HeatDissipationPerSecond"), TEXT("초당 열 방출량"));
		UserMessage.ReplaceInline(TEXT("PassiveDetectionRangeCm"), TEXT("수동 탐지 거리"));
		UserMessage.ReplaceInline(TEXT("ActiveScanRangeCm"), TEXT("능동 스캔 거리"));
		UserMessage.ReplaceInline(TEXT("VisualDetectionRangeCm"), TEXT("시각 탐지 거리"));
		UserMessage.ReplaceInline(TEXT("UpdateIntervalSec"), TEXT("갱신 간격"));
		UserMessage.ReplaceInline(TEXT("MaxActorScansPerUpdate"), TEXT("갱신당 최대 스캔 대상 수"));
		UserMessage.ReplaceInline(TEXT("ContactMemoryTimeSec"), TEXT("접촉 기억 시간"));
		UserMessage.ReplaceInline(TEXT("DestroyedHoldTimeSec"), TEXT("파괴 대상 유지 시간"));
		UserMessage.ReplaceInline(TEXT("ActiveScanDurationSec"), TEXT("능동 스캔 지속 시간"));
		UserMessage.ReplaceInline(TEXT("AnalysisGainPerSec"), TEXT("초당 분석 증가량"));
		UserMessage.ReplaceInline(TEXT("AnalysisDecayPerSec"), TEXT("초당 분석 감소량"));
		UserMessage.ReplaceInline(TEXT("IdentifiedThreshold"), TEXT("식별 임계값"));
		UserMessage.ReplaceInline(TEXT("DetailedScanThreshold"), TEXT("정밀 스캔 임계값"));
		UserMessage.ReplaceInline(TEXT("RadarDisplayRangePresetsCm"), TEXT("레이더 표시 거리 프리셋"));
		UserMessage.ReplaceInline(TEXT("DefaultRadarDisplayRangePresetIndex"), TEXT("기본 레이더 표시 거리 프리셋 인덱스"));
		UserMessage.ReplaceInline(TEXT("Active Scan"), TEXT("능동 스캔"));
		UserMessage.ReplaceInline(TEXT("Radar"), TEXT("레이더"));
		UserMessage.ReplaceInline(TEXT("Preset"), TEXT("프리셋"));
		UserMessage.ReplaceInline(TEXT("Runtime"), TEXT("런타임"));
		UserMessage.ReplaceInline(TEXT("fallback"), TEXT("대체값"));
		return UserMessage;
	}
}

// Data Asset Manager의 metadata-only inventory를 준비하고 EquipmentPreset browser projection을 구성합니다.
bool FCFEquipmentBuilderVM::Initialize(FString& OutError)
{
	if (!DataManagementViewModel.Initialize(OutError))
	{
		PresetEntries.Reset();
		return false;
	}
	return RebuildPresetEntries(OutError);
}

// fresh metadata inventory를 읽어 EquipmentPreset browser projection만 갱신합니다.
bool FCFEquipmentBuilderVM::RefreshBrowser(FString& OutError)
{
	InvalidateReviewedApproval();
	if (!DataManagementViewModel.Refresh(OutError))
	{
		PresetEntries.Reset();
		return false;
	}
	if (!RebuildPresetEntries(OutError))
	{
		return false;
	}
	if (!bIsNewDraft && !SelectedObjectPath.IsEmpty() && !HasPresetEntry(SelectedObjectPath))
	{
		ClearCurrentDraft();
	}
	return true;
}

// 저장된 exact EquipmentPreset 하나를 read-only load해 transient draft와 source snapshot으로 복사합니다.
bool FCFEquipmentBuilderVM::LoadExistingPreset(const FString& ObjectPath, FString& OutError)
{
	if (ObjectPath.IsEmpty() || !HasPresetEntry(ObjectPath))
	{
		OutError = TEXT("현재 장비 프리셋 목록에 없는 대상입니다. 목록을 새로고침한 뒤 다시 선택하세요.");
		return false;
	}
	if (bHasActiveDraft && !bIsNewDraft && SelectedObjectPath == ObjectPath)
	{
		OutError.Reset();
		return true;
	}

	// 선택한 persisted EquipmentPreset을 읽기 전용으로 load합니다.
	UCFEquipmentPresetData* LoadedPreset = LoadObject<UCFEquipmentPresetData>(nullptr, *ObjectPath);
	if (!LoadedPreset || LoadedPreset->GetClass() != UCFEquipmentPresetData::StaticClass())
	{
		OutError = FString::Printf(TEXT("장비 프리셋을 읽지 못했거나 올바른 형식이 아닙니다: %s"), *ObjectPath);
		return false;
	}

	// Product UObject와 분리된 transient draft를 새로 구성합니다.
	FCFEquipmentPresetDraft LoadedDraft;
	LoadedDraft.EquipmentId = LoadedPreset->EquipmentId;
	LoadedDraft.DisplayName = LoadedPreset->DisplayName;
	LoadedDraft.bPreserveExistingDisplayName = !FCFEquipmentDurable::CanEditDisplayNameLosslessly(LoadedPreset->DisplayName);
	LoadedDraft.RequiredMountType = LoadedPreset->RequiredMountType;
	LoadedDraft.RequiredWeaponSize = LoadedPreset->RequiredWeaponSize;
	LoadedDraft.DefaultTurretMountData = LoadedPreset->DefaultTurretMountData.Get();
	LoadedDraft.DefaultWeaponData = LoadedPreset->DefaultWeaponData.Get();
	LoadedDraft.DefaultSensorData = LoadedPreset->DefaultSensorData.Get();
	LoadedDraft.DraftMode = InferDraftMode(*LoadedPreset);

	// Scanner Builder policy는 Utility + size None이며 persisted Asset은 explicit save 전까지 건드리지 않습니다.
	FCFEquipmentPresetDraft WorkingDraft = LoadedDraft;
	if (WorkingDraft.DraftMode == ECFEquipmentDraftMode::Scanner)
	{
		WorkingDraft.RequiredMountType = ECFVehicleMountType::Utility;
		WorkingDraft.RequiredWeaponSize = ECFVehicleWeaponSize::None;
	}

	InvalidateReviewedApproval();
	Draft = WorkingDraft;
	SourceDraft = LoadedDraft;
	SelectedObjectPath = ObjectPath;
	CreateAssetName.Reset();
	CreateTargetObjectPath.Reset();
	bHasActiveDraft = true;
	bIsNewDraft = false;
	bDraftDirty = !AreDraftsEquivalentForEditing(Draft, SourceDraft);
	RefreshModeCachesFromDraft();
	RefreshCompatibilityProbe();
	OutError.Reset();
	return true;
}

// Product target을 만들지 않고 새 EquipmentPreset transient draft를 시작합니다.
void FCFEquipmentBuilderVM::BeginNewPreset()
{
	InvalidateReviewedApproval();
	// 신규 draft의 안전한 blank identity + current weapon-package 기본 장착값입니다.
	FCFEquipmentPresetDraft NewDraft;
	NewDraft.EquipmentId = NAME_None;
	NewDraft.DisplayName = FText::GetEmpty();
	NewDraft.RequiredMountType = ECFVehicleMountType::Turret;
	NewDraft.RequiredWeaponSize = ECFVehicleWeaponSize::Large;
	NewDraft.DraftMode = ECFEquipmentDraftMode::Weapon;
	Draft = NewDraft;
	SourceDraft = NewDraft;
	SelectedObjectPath.Reset();
	CreateAssetName.Reset();
	CreateTargetObjectPath.Reset();
	bHasActiveDraft = true;
	bIsNewDraft = true;
	bDraftDirty = false;
	RefreshModeCachesFromDraft();
	RefreshCompatibilityProbe();
}

// 현재 draft를 source snapshot 또는 새 draft 최초값으로 되돌립니다.
void FCFEquipmentBuilderVM::ResetDraftToSource()
{
	if (!bHasActiveDraft)
	{
		return;
	}
	InvalidateReviewedApproval();
	Draft = SourceDraft;
	if (bIsNewDraft)
	{
		CreateAssetName.Reset();
		CreateTargetObjectPath.Reset();
	}
	RefreshModeCachesFromDraft();
	bDraftDirty = false;
	RefreshCompatibilityProbe();
}

// current draft의 EquipmentId만 변경하고 Product UObject는 수정하지 않습니다.
void FCFEquipmentBuilderVM::SetEquipmentId(FName EquipmentId)
{
	if (!bHasActiveDraft)
	{
		return;
	}
	Draft.EquipmentId = EquipmentId;
	RecomputeDraftDirty();
}

// current draft의 DisplayName만 변경하고 Product UObject는 수정하지 않습니다.
void FCFEquipmentBuilderVM::SetDisplayName(const FText& DisplayName)
{
	if (!bHasActiveDraft || Draft.bPreserveExistingDisplayName)
	{
		return;
	}
	Draft.DisplayName = DisplayName;
	RecomputeDraftDirty();
}

// 신규 Draft에서만 Weapon/Scanner Draft Mode를 변경하고 mode별 transient 선택을 보존한 채 active payload만 Draft에 projection합니다.
void FCFEquipmentBuilderVM::SetDraftMode(ECFEquipmentDraftMode DraftMode)
{
	if (!bHasActiveDraft || !bIsNewDraft || Draft.DraftMode == DraftMode)
	{
		return;
	}

	if (Draft.DraftMode == ECFEquipmentDraftMode::Weapon)
	{
		WeaponModeTurretMountCache = Draft.DefaultTurretMountData;
		WeaponModeWeaponDataCache = Draft.DefaultWeaponData;
		WeaponModeMountTypeCache = Draft.RequiredMountType == ECFVehicleMountType::Utility
			? ECFVehicleMountType::Turret
			: Draft.RequiredMountType;
		WeaponModeSizeCache = Draft.RequiredWeaponSize == ECFVehicleWeaponSize::None
			? ECFVehicleWeaponSize::Large
			: Draft.RequiredWeaponSize;
	}
	else
	{
		ScannerModeSensorDataCache = Draft.DefaultSensorData;
	}

	Draft.DraftMode = DraftMode;
	if (DraftMode == ECFEquipmentDraftMode::Scanner)
	{
		Draft.RequiredMountType = ECFVehicleMountType::Utility;
		Draft.RequiredWeaponSize = ECFVehicleWeaponSize::None;
		Draft.DefaultTurretMountData.Reset();
		Draft.DefaultWeaponData.Reset();
		Draft.DefaultSensorData = ScannerModeSensorDataCache;
	}
	else
	{
		Draft.RequiredMountType = WeaponModeMountTypeCache == ECFVehicleMountType::Utility
			? ECFVehicleMountType::Turret
			: WeaponModeMountTypeCache;
		Draft.RequiredWeaponSize = WeaponModeSizeCache == ECFVehicleWeaponSize::None
			? ECFVehicleWeaponSize::Large
			: WeaponModeSizeCache;
		Draft.DefaultTurretMountData = WeaponModeTurretMountCache;
		Draft.DefaultWeaponData = WeaponModeWeaponDataCache;
		Draft.DefaultSensorData.Reset();
	}
	RecomputeDraftDirty();
}

// current draft의 RequiredMountType만 변경합니다.
void FCFEquipmentBuilderVM::SetRequiredMountType(ECFVehicleMountType MountType)
{
	if (!bHasActiveDraft)
	{
		return;
	}
	if (Draft.DraftMode == ECFEquipmentDraftMode::Scanner)
	{
		Draft.RequiredMountType = ECFVehicleMountType::Utility;
		RecomputeDraftDirty();
		return;
	}
	Draft.RequiredMountType = MountType;
	WeaponModeMountTypeCache = MountType;
	RecomputeDraftDirty();
}

// current draft의 RequiredWeaponSize만 변경합니다.
void FCFEquipmentBuilderVM::SetRequiredWeaponSize(ECFVehicleWeaponSize WeaponSize)
{
	if (!bHasActiveDraft || Draft.DraftMode == ECFEquipmentDraftMode::Scanner)
	{
		return;
	}
	Draft.RequiredWeaponSize = WeaponSize;
	WeaponModeSizeCache = WeaponSize;
	RecomputeDraftDirty();
}

// Weapon Draft의 TurretMountData reference를 exact typed read-only selection으로 갱신합니다.
bool FCFEquipmentBuilderVM::SetTurretMountDataPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!bHasActiveDraft || Draft.DraftMode != ECFEquipmentDraftMode::Weapon)
	{
		OutError = TEXT("터렛 마운트는 무장 장비 초안에서만 선택할 수 있습니다.");
		return false;
	}

	if (ObjectPath.IsNull())
	{
		Draft.DefaultTurretMountData.Reset();
		WeaponModeTurretMountCache.Reset();
		RecomputeDraftDirty();
		OutError.Reset();
		return true;
	}

	// 사용자가 선택한 exact TurretMountData를 read-only로 확인하기 위해 load한 객체입니다.
	UCFTurretMountData* SelectedTurretMount = LoadObject<UCFTurretMountData>(nullptr, *ObjectPath.ToString());
	if (!SelectedTurretMount || SelectedTurretMount->GetClass() != UCFTurretMountData::StaticClass())
	{
		OutError = FString::Printf(TEXT("선택한 터렛 마운트 데이터를 올바른 형식으로 읽지 못했습니다: %s"), *ObjectPath.ToString());
		return false;
	}

	Draft.DefaultTurretMountData = SelectedTurretMount;
	WeaponModeTurretMountCache = SelectedTurretMount;
	RecomputeDraftDirty();
	OutError.Reset();
	return true;
}

// Weapon Draft의 WeaponData reference를 exact typed read-only selection으로 갱신합니다.
bool FCFEquipmentBuilderVM::SetWeaponDataPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!bHasActiveDraft || Draft.DraftMode != ECFEquipmentDraftMode::Weapon)
	{
		OutError = TEXT("무기 데이터는 무장 장비 초안에서만 선택할 수 있습니다.");
		return false;
	}

	if (ObjectPath.IsNull())
	{
		Draft.DefaultWeaponData.Reset();
		WeaponModeWeaponDataCache.Reset();
		RecomputeDraftDirty();
		OutError.Reset();
		return true;
	}

	// 사용자가 선택한 exact WeaponData를 read-only로 확인하기 위해 load한 객체입니다.
	UCFWeaponData* SelectedWeaponData = LoadObject<UCFWeaponData>(nullptr, *ObjectPath.ToString());
	if (!SelectedWeaponData || SelectedWeaponData->GetClass() != UCFWeaponData::StaticClass())
	{
		OutError = FString::Printf(TEXT("선택한 무기 데이터를 올바른 형식으로 읽지 못했습니다: %s"), *ObjectPath.ToString());
		return false;
	}

	Draft.DefaultWeaponData = SelectedWeaponData;
	WeaponModeWeaponDataCache = SelectedWeaponData;
	RecomputeDraftDirty();
	OutError.Reset();
	return true;
}

// Scanner Draft의 VehicleSensorData reference를 exact typed read-only selection으로 갱신합니다.
bool FCFEquipmentBuilderVM::SetSensorDataPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!bHasActiveDraft || Draft.DraftMode != ECFEquipmentDraftMode::Scanner)
	{
		OutError = TEXT("센서 데이터는 스캐너 장비 초안에서만 선택할 수 있습니다.");
		return false;
	}

	if (ObjectPath.IsNull())
	{
		Draft.DefaultSensorData.Reset();
		ScannerModeSensorDataCache.Reset();
		RecomputeDraftDirty();
		OutError.Reset();
		return true;
	}

	// 사용자가 선택한 exact VehicleSensorData를 read-only로 확인하기 위해 load한 객체입니다.
	UCFVehicleSensorData* SelectedSensorData = LoadObject<UCFVehicleSensorData>(nullptr, *ObjectPath.ToString());
	if (!SelectedSensorData || SelectedSensorData->GetClass() != UCFVehicleSensorData::StaticClass())
	{
		OutError = FString::Printf(TEXT("선택한 센서 데이터를 올바른 형식으로 읽지 못했습니다: %s"), *ObjectPath.ToString());
		return false;
	}

	Draft.DefaultSensorData = SelectedSensorData;
	ScannerModeSensorDataCache = SelectedSensorData;
	RecomputeDraftDirty();
	OutError.Reset();
	return true;
}

// 신규 Draft의 사용자용 에셋 이름을 받아 EquipmentId와 canonical Product target을 함께 자동 갱신합니다.
void FCFEquipmentBuilderVM::SetCreateAssetName(const FString& AssetName)
{
	if (!bHasActiveDraft || !bIsNewDraft)
	{
		return;
	}

	// 사용자가 입력한 에셋 이름에서 앞뒤 공백만 제거한 값입니다.
	const FString TrimmedAssetName = AssetName.TrimStartAndEnd();
	if (CreateAssetName == TrimmedAssetName)
	{
		return;
	}

	CreateAssetName = TrimmedAssetName;
	Draft.EquipmentId = TrimmedAssetName.IsEmpty() ? NAME_None : FName(*TrimmedAssetName);
	CreateTargetObjectPath = TrimmedAssetName.IsEmpty()
		? FString()
		: FString::Printf(TEXT("%s/%s.%s"), *EquipmentPresetProductRoot, *TrimmedAssetName, *TrimmedAssetName);
	RecomputeDraftDirty();
}

// 신규 Draft의 low-level canonical Product target object path를 transient하게 갱신합니다.
void FCFEquipmentBuilderVM::SetCreateTargetObjectPath(const FString& TargetObjectPath)
{
	if (!bHasActiveDraft || !bIsNewDraft)
	{
		return;
	}

	// UI 입력에서 앞뒤 공백만 제거한 transient target path입니다.
	const FString TrimmedTargetObjectPath = TargetObjectPath.TrimStartAndEnd();
	if (CreateTargetObjectPath == TrimmedTargetObjectPath)
	{
		return;
	}
	CreateTargetObjectPath = TrimmedTargetObjectPath;
	InvalidateReviewedApproval();
}

// current Create/Update Review target object path를 반환합니다.
FString FCFEquipmentBuilderVM::GetTargetObjectPathForReview() const
{
	return bIsNewDraft ? CreateTargetObjectPath : SelectedObjectPath;
}

// target path가 Product 또는 Automation disposable policy에 맞는 canonical path인지 검증합니다.
bool FCFEquipmentBuilderVM::ValidateReviewTargetPath(const FString& TargetObjectPath, FString& OutError) const
{
	OutError.Reset();
	// Shared canonical target-path diagnostic입니다.
	TArray<FCFDAStagingIssue> PathIssues;
	if (!CFDACommonPrimitives::ValidateTargetObjectPath(TargetObjectPath, PathIssues))
	{
		OutError = PathIssues.IsEmpty() ? TEXT("장비 프리셋 저장 경로가 올바른 `/Game/.../Asset.Asset` 형식이 아닙니다.") : PathIssues[0].Message;
		return false;
	}
	if (!TargetObjectPath.StartsWith(TEXT("/Game/CarFight/"), ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Equipment Builder target은 `/Game/CarFight/` namespace 안에 있어야 합니다.");
		return false;
	}

	const bool bIsAnyTestTarget = TargetObjectPath.StartsWith(TEXT("/Game/CarFight/Tests/"), ESearchCase::CaseSensitive);
	if (bIsAnyTestTarget)
	{
#if WITH_DEV_AUTOMATION_TESTS
		const bool bIsApprovedDisposableTarget = bAllowDisposableTestTargets
			&& TargetObjectPath.StartsWith(TEXT("/Game/CarFight/Tests/EquipmentAuthoring/"), ESearchCase::CaseSensitive);
		if (bIsApprovedDisposableTarget)
		{
			return true;
		}
#endif
		OutError = TEXT("Product Equipment Builder에서는 `/Game/CarFight/Tests/**` target을 생성/수정할 수 없습니다.");
		return false;
	}
	return true;
}

// exact class inventory와 loaded-only EquipmentPreset union으로 prospective EquipmentId uniqueness를 fail-closed 검증합니다.
bool FCFEquipmentBuilderVM::ValidateProspectiveIdentityUniqueness(
	FName ProspectiveEquipmentId,
	const FString& TargetObjectPath,
	ECFEquipmentReviewOperation Operation,
	FString& OutError)
{
	OutError.Reset();
	if (ProspectiveEquipmentId.IsNone())
	{
		OutError = TEXT("IdentityUniquenessUnconfirmed: EquipmentId가 비어 있습니다.");
		return false;
	}

	// Review/Commit 시점의 fresh DataManagement metadata inventory입니다.
	FString InventoryError;
	if (!DataManagementViewModel.Refresh(InventoryError))
	{
		OutError = FString::Printf(TEXT("IdentityUniquenessUnconfirmed: fresh DataManagement inventory를 만들지 못했습니다. %s"), *InventoryError);
		return false;
	}
	// Complete exact-class inventory authority입니다.
	const FCFDAInventoryResult& Inventory = DataManagementViewModel.GetInventory();
	if (!Inventory.IsInventoryComplete())
	{
		OutError = TEXT("IdentityUniquenessUnconfirmed: DataManagement inventory coverage가 complete가 아닙니다.");
		return false;
	}

	// Current Asset Registry authority입니다.
	FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = RegistryModule.Get();
	if (AssetRegistry.IsLoadingAssets())
	{
		OutError = TEXT("IdentityUniquenessUnconfirmed: Asset Registry가 loading 중입니다.");
		return false;
	}

	// Persisted inventory + loaded-only union에서 이미 처리한 exact object path 집합입니다.
	TSet<FString> CandidateObjectPaths;
	// Exact EquipmentPreset native class path입니다.
	const FString EquipmentPresetClassPath = GetEquipmentPresetClassPath();

	for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
	{
		if (AssetRecord.ClassPath != EquipmentPresetClassPath)
		{
			continue;
		}

		// Inventory candidate path의 canonicality를 검증하는 diagnostics입니다.
		TArray<FCFDAStagingIssue> CandidatePathIssues;
		if (!CFDACommonPrimitives::ValidateTargetObjectPath(AssetRecord.ObjectPath, CandidatePathIssues))
		{
			OutError = FString::Printf(TEXT("IdentityUniquenessUnconfirmed: exact-class inventory candidate path를 canonicalize할 수 없습니다: %s"), *AssetRecord.ObjectPath);
			return false;
		}
		CandidateObjectPaths.Add(AssetRecord.ObjectPath);
		if (Operation == ECFEquipmentReviewOperation::Update && AssetRecord.ObjectPath == TargetObjectPath)
		{
			continue;
		}

		// Candidate persisted identity를 exact typed UObject에서 읽습니다.
		UCFEquipmentPresetData* CandidatePreset = LoadObject<UCFEquipmentPresetData>(nullptr, *AssetRecord.ObjectPath);
		if (!CandidatePreset || CandidatePreset->GetClass() != UCFEquipmentPresetData::StaticClass())
		{
			OutError = FString::Printf(TEXT("IdentityUniquenessUnconfirmed: persisted exact-class candidate를 읽지 못했습니다: %s"), *AssetRecord.ObjectPath);
			return false;
		}
		if (CandidatePreset->EquipmentId == ProspectiveEquipmentId)
		{
			OutError = FString::Printf(TEXT("장비 프리셋 ID 중복: `%s` ID를 다른 장비 프리셋이 이미 사용하고 있습니다: %s"), *ProspectiveEquipmentId.ToString(), *AssetRecord.ObjectPath);
			return false;
		}
	}

	for (TObjectIterator<UCFEquipmentPresetData> It; It; ++It)
	{
		// Loaded-only identity closure가 검사할 exact EquipmentPreset UObject입니다.
		UCFEquipmentPresetData* LoadedPreset = *It;
		if (!LoadedPreset
			|| LoadedPreset->GetClass() != UCFEquipmentPresetData::StaticClass()
			|| LoadedPreset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
		{
			continue;
		}

		// Loaded candidate exact object path입니다.
		const FString LoadedObjectPath = LoadedPreset->GetPathName();
		if (!LoadedObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		// Loaded candidate path의 canonicality diagnostics입니다.
		TArray<FCFDAStagingIssue> LoadedPathIssues;
		if (!CFDACommonPrimitives::ValidateTargetObjectPath(LoadedObjectPath, LoadedPathIssues))
		{
			OutError = FString::Printf(TEXT("IdentityUniquenessUnconfirmed: loaded exact-class candidate path를 canonicalize할 수 없습니다: %s"), *LoadedObjectPath);
			return false;
		}
		if (CandidateObjectPaths.Contains(LoadedObjectPath))
		{
			continue;
		}
		CandidateObjectPaths.Add(LoadedObjectPath);
		if (Operation == ECFEquipmentReviewOperation::Update && LoadedObjectPath == TargetObjectPath)
		{
			continue;
		}
		if (LoadedPreset->EquipmentId == ProspectiveEquipmentId)
		{
			OutError = FString::Printf(TEXT("장비 프리셋 ID 중복: 현재 메모리에 열린 장비 프리셋 `%s`가 `%s` ID를 이미 사용하고 있습니다."), *LoadedObjectPath, *ProspectiveEquipmentId.ToString());
			return false;
		}
	}
	return true;
}

// current Draft에 대해 fresh target/semantic/identity/child evidence와 current-contract digest를 생성합니다.
bool FCFEquipmentBuilderVM::BuildFreshReviewEvidence(FCFEquipmentReviewState& OutEvidence, FString& OutError)
{
	OutEvidence = FCFEquipmentReviewState();
	OutError.Reset();
	if (!bHasActiveDraft)
	{
		OutError = TEXT("검토할 장비 프리셋 초안이 없습니다.");
		return false;
	}

	// Current Create/Update Review operation입니다.
	const ECFEquipmentReviewOperation Operation = bIsNewDraft ? ECFEquipmentReviewOperation::Create : ECFEquipmentReviewOperation::Update;
	// Current exact Review target object path입니다.
	const FString TargetObjectPath = GetTargetObjectPathForReview();
	if (!ValidateReviewTargetPath(TargetObjectPath, OutError))
	{
		return false;
	}

	OutEvidence.Operation = Operation;
	OutEvidence.TargetObjectPath = TargetObjectPath;
	OutEvidence.TargetClassPath = FCFEquipmentDurable::GetTargetClassPath();
	OutEvidence.ContractRevision = FCFEquipmentDurable::GetContractRevision();
	OutEvidence.ValidationState = FCFEquipmentDurable::GetReadyValidationToken();

	if (Operation == ECFEquipmentReviewOperation::Update)
	{
		// Fresh current exact EquipmentPreset target입니다.
		UCFEquipmentPresetData* CurrentPreset = LoadObject<UCFEquipmentPresetData>(nullptr, *TargetObjectPath);
		if (!CurrentPreset || CurrentPreset->GetClass() != UCFEquipmentPresetData::StaticClass())
		{
			OutError = TEXT("ReviewStale: 수정할 장비 프리셋이 없거나 올바른 형식이 아닙니다.");
			return false;
		}
		// Update target package입니다.
		UPackage* CurrentPackage = CurrentPreset->GetOutermost();
		if (CurrentPackage == nullptr || CurrentPackage->IsDirty())
		{
			OutError = TEXT("TargetDirtyUnowned: Update target package가 dirty 상태라 Equipment Builder가 덮어쓸 수 없습니다.");
			return false;
		}

		// Current exact7 extraction diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!FCFEquipmentDurable::ExtractSnapshotFromAsset(*CurrentPreset, OutEvidence.BeforeSnapshot, CurrentIssues))
		{
			OutError = CurrentIssues.IsEmpty() ? TEXT("ReviewStale: 현재 장비 프리셋 내용을 손실 없이 읽지 못했습니다.") : CurrentIssues[0].Message;
			return false;
		}
		OutEvidence.bHasBeforeSnapshot = true;
		OutEvidence.CurrentSemanticFingerprint = OutEvidence.BeforeSnapshot.SemanticFingerprint;
	}
	else
	{
		// Create target package name입니다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(TargetObjectPath);
		// Create target soft path입니다.
		const FSoftObjectPath TargetSoftPath(TargetObjectPath);
		// Create target registry metadata입니다.
		FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AssetRegistry = RegistryModule.Get();
		if (AssetRegistry.IsLoadingAssets())
		{
			OutError = TEXT("ReviewStale: Asset Registry loading 중이라 Create target absence를 확정할 수 없습니다.");
			return false;
		}
		const FAssetData ExistingAssetData = AssetRegistry.GetAssetByObjectPath(TargetSoftPath, false, false);
		if (FindPackage(nullptr, *PackageName) != nullptr
			|| FPackageName::DoesPackageExist(PackageName)
			|| TargetSoftPath.ResolveObject() != nullptr
			|| ExistingAssetData.IsValid())
		{
			OutError = TEXT("CreateCollision: target object/package가 memory 또는 disk에 이미 존재합니다.");
			return false;
		}
		OutEvidence.CurrentSemanticFingerprint = FCFEquipmentDurable::GetAbsentTargetToken();
	}

	if (!FCFEquipmentDurable::BuildSnapshotFromDraft(Draft, OutEvidence.AfterSnapshot, OutError))
	{
		return false;
	}
	OutEvidence.ProspectiveSemanticFingerprint = OutEvidence.AfterSnapshot.SemanticFingerprint;

	if (!ValidateProspectiveIdentityUniqueness(Draft.EquipmentId, TargetObjectPath, Operation, OutError))
	{
		return false;
	}

	// Automation-only persisted/clean child truth fault path입니다.
	FString ForcedChildPath;
#if WITH_DEV_AUTOMATION_TESTS
	ForcedChildPath = ForcedChildStateUnconfirmedObjectPath;
#endif
	if (!FCFEquipmentDurable::ValidatePersistedChildDependencies(Draft, ForcedChildPath, OutError))
	{
		return false;
	}

	// Existing P0-02 native-or-bounded composition validation입니다.
	const FCFEquipmentDraftValidation Validation = BuildValidationForDraft(Draft);
	if (!Validation.bPackageComplete || Validation.bHasBlockingErrors)
	{
		OutError = TEXT("검토 실패: 장비 구성의 필수 데이터 또는 하위 데이터 검증 오류를 먼저 해결하세요.");
		return false;
	}

	if (!FCFEquipmentDurable::BuildReviewProposalDigest(
		Operation,
		TargetObjectPath,
		OutEvidence.TargetClassPath,
		OutEvidence.CurrentSemanticFingerprint,
		OutEvidence.ProspectiveSemanticFingerprint,
		OutEvidence.ContractRevision,
		OutEvidence.ValidationState,
		OutEvidence.ReviewProposalDigest,
		OutError))
	{
		return false;
	}
	return true;
}

// current Draft/target/identity/child truth를 fresh 검증해 one-shot Reviewed approval을 생성합니다.
bool FCFEquipmentBuilderVM::ReviewCurrentDraft(FString& OutError)
{
	// Fresh Review가 생성한 immutable evidence입니다.
	FCFEquipmentReviewState FreshEvidence;
	if (!BuildFreshReviewEvidence(FreshEvidence, OutError))
	{
		InvalidateReviewedApproval();
		return false;
	}
	FreshEvidence.ApprovalState = ECFEquipmentApprovalState::Reviewed;
	ReviewState = MoveTemp(FreshEvidence);
	OutError.Reset();
	return true;
}

// shared durable core terminal result를 Equipment Builder public report로 투영합니다.
ECFEquipmentApplyResult FCFEquipmentBuilderVM::MapDurableResult(uint8 DurableResultValue)
{
	const ECFDAStagingTargetApplyResult DurableResult = static_cast<ECFDAStagingTargetApplyResult>(DurableResultValue);
	switch (DurableResult)
	{
	case ECFDAStagingTargetApplyResult::BlockedBeforeMutation:
		return ECFEquipmentApplyResult::BlockedBeforeMutation;
	case ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite:
		return ECFEquipmentApplyResult::FailedBeforeDurableWrite;
	case ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed:
		return ECFEquipmentApplyResult::InMemoryStateUnconfirmed;
	case ECFDAStagingTargetApplyResult::DurableApplied:
	case ECFDAStagingTargetApplyResult::PostCommitWarning:
		return ECFEquipmentApplyResult::DurableApplied;
	case ECFDAStagingTargetApplyResult::SaveStateUnconfirmed:
		return ECFEquipmentApplyResult::SaveStateUnconfirmed;
	default:
		return ECFEquipmentApplyResult::NotRun;
	}
}

// exact Reviewed approval을 one-shot 소비하고 immediate no-yield fresh TOCTOU 뒤 CFDADurableCore로 적용 및 저장합니다.
bool FCFEquipmentBuilderVM::ApplyReviewedDraft(FCFEquipmentApplyReport& OutReport)
{
	OutReport = FCFEquipmentApplyReport();
	if (ReviewState.ApprovalState != ECFEquipmentApprovalState::Reviewed)
	{
		OutReport.Result = ECFEquipmentApplyResult::BlockedBeforeMutation;
		OutReport.Diagnostic = TEXT("사용 가능한 검토 결과가 없거나 이미 사용했습니다. 현재 상태에서 다시 검토하세요.");
		return false;
	}

	ReviewState.ApprovalState = ECFEquipmentApprovalState::ApplyAttempted;
	OutReport.TargetObjectPath = ReviewState.TargetObjectPath;

	// Apply 시점의 fresh no-yield TOCTOU evidence입니다.
	FCFEquipmentReviewState FreshEvidence;
	// Fresh preflight failure diagnostic입니다.
	FString FreshError;
	if (!BuildFreshReviewEvidence(FreshEvidence, FreshError))
	{
		ReviewState.ApprovalState = ECFEquipmentApprovalState::Consumed;
		OutReport.Result = ECFEquipmentApplyResult::BlockedBeforeMutation;
		OutReport.bApprovalConsumed = true;
		OutReport.Diagnostic = FString::Printf(TEXT("Apply fresh TOCTOU preflight 실패: %s"), *FreshError);
		return false;
	}
	if (FreshEvidence.ReviewProposalDigest != ReviewState.ReviewProposalDigest)
	{
		ReviewState.ApprovalState = ECFEquipmentApprovalState::Consumed;
		OutReport.Result = ECFEquipmentApplyResult::BlockedBeforeMutation;
		OutReport.bApprovalConsumed = true;
		OutReport.Diagnostic = TEXT("ReviewStale: 검토 후 장비 상태가 달라졌습니다. 저장하지 않았으며 현재 상태에서 다시 검토하세요.");
		return false;
	}

	// Immediate durable core 호출에 전달하는 exact fresh common row입니다.
	FCFDACommonPreviewRow FreshRow;
	FreshRow.Kind = FreshEvidence.Operation == ECFEquipmentReviewOperation::Create
		? ECFDAStagingPreviewKind::Create
		: ECFDAStagingPreviewKind::Update;
	FreshRow.Envelope.SchemaId = TEXT("CarFight.EquipmentAuthoring.EquipmentPreset");
	FreshRow.Envelope.SchemaRevision = 1;
	FreshRow.Envelope.AdapterContractRevision = FCFEquipmentDurable::GetContractRevision();
	FreshRow.Envelope.DataAssetTypeClassPath = FCFEquipmentDurable::GetTargetClassPath();
	FreshRow.Envelope.StableLogicalId = FreshEvidence.AfterSnapshot.EquipmentId;
	FreshRow.Envelope.TargetObjectPath = FreshEvidence.TargetObjectPath;
	FreshRow.Envelope.StagingRelativePath = TEXT("EquipmentBuilder://ReviewedDraft");
	FreshRow.Envelope.bHasBaseSemanticFingerprint = FreshEvidence.Operation == ECFEquipmentReviewOperation::Update;
	FreshRow.Envelope.BaseSemanticFingerprint = FreshEvidence.Operation == ECFEquipmentReviewOperation::Update ? FreshEvidence.CurrentSemanticFingerprint : FString();
	FreshRow.Envelope.CurrentSemanticFingerprint = FreshEvidence.Operation == ECFEquipmentReviewOperation::Update ? FreshEvidence.CurrentSemanticFingerprint : FString();
	FreshRow.Envelope.StagingSemanticFingerprint = FreshEvidence.ProspectiveSemanticFingerprint;
	FreshRow.Envelope.PlannedOperation = FreshRow.Kind;

	// Final fresh digest revalidation 이후 same stack에서 approval을 consume하고 바로 shared durable core에 진입합니다.
	ReviewState.ApprovalState = ECFEquipmentApprovalState::Consumed;
	OutReport.bApprovalConsumed = true;

	// Shared durable core exact target report입니다.
	FCFDAStagingTargetApplyReport DurableReport;
	DurableReport.TargetObjectPath = FreshEvidence.TargetObjectPath;
	DurableReport.StableLogicalId = FreshEvidence.AfterSnapshot.EquipmentId;
	DurableReport.PlannedOperation = FreshRow.Kind;
	CFDADurableCore::ApplyTypedTarget<UCFEquipmentPresetData, FCFEquipmentSemanticSnapshot>(
		FreshRow,
		FreshEvidence.AfterSnapshot,
		&FCFEquipmentDurable::MaterializeSnapshot,
		&FCFEquipmentDurable::ExtractSnapshotFromAsset,
		&FCFEquipmentDurable::BuildSemanticFingerprint,
		DurableReport);

	OutReport.Result = MapDurableResult(static_cast<uint8>(DurableReport.Result));
	OutReport.Diagnostic = DurableReport.Diagnostic;
	if (OutReport.Result != ECFEquipmentApplyResult::DurableApplied)
	{
		return false;
	}

	SelectedObjectPath = FreshEvidence.TargetObjectPath;
	CreateAssetName.Reset();
	CreateTargetObjectPath.Reset();
	SourceDraft = Draft;
	bHasActiveDraft = true;
	bIsNewDraft = false;
	bDraftDirty = false;
	RefreshCompatibilityProbe();
	return true;
}

// read-only compatibility 대상 VehicleData object path를 변경하고 이전 Mount 선택을 폐기합니다.
void FCFEquipmentBuilderVM::SetCompatibilityVehicleDataPath(const FSoftObjectPath& VehicleDataObjectPath)
{
	CompatibilityVehicleDataObjectPath = VehicleDataObjectPath;
	CompatibilityMountProfileId = NAME_None;
	RefreshCompatibilityProbe();
}

// current VehicleData 안에서 평가할 transient MountProfileId를 선택합니다.
void FCFEquipmentBuilderVM::SetCompatibilityMountProfileId(const FName MountProfileId)
{
	CompatibilityMountProfileId = MountProfileId;
	RefreshCompatibilityProbe();
}

// current Vehicle/Mount/Draft source를 fresh 재해석해 advisory probe를 갱신합니다.
void FCFEquipmentBuilderVM::RefreshCompatibilityProbe()
{
	RebuildCompatibilityMountOptions();
	CompatibilityResult = EvaluateCompatibilityProbe();
}

// current transient path를 exact UCFVehicleData로 read-only resolve합니다.
UCFVehicleData* FCFEquipmentBuilderVM::ResolveCompatibilityVehicleData() const
{
	if (CompatibilityVehicleDataObjectPath.IsNull())
	{
		return nullptr;
	}

	// 이미 메모리에 있는 exact object가 있으면 재사용하는 read-only 후보입니다.
	UObject* ResolvedObject = CompatibilityVehicleDataObjectPath.ResolveObject();
	if (ResolvedObject != nullptr)
	{
		// 이미 메모리에 resolve된 object를 exact VehicleData로 해석한 read-only candidate입니다.
		UCFVehicleData* ResolvedVehicleData = Cast<UCFVehicleData>(ResolvedObject);
		return ResolvedVehicleData != nullptr && ResolvedVehicleData->GetClass() == UCFVehicleData::StaticClass()
			? ResolvedVehicleData
			: nullptr;
	}

	// persisted object path에서 read-only로 load한 VehicleData입니다.
	UCFVehicleData* LoadedVehicleData = LoadObject<UCFVehicleData>(nullptr, *CompatibilityVehicleDataObjectPath.ToString());
	return LoadedVehicleData != nullptr && LoadedVehicleData->GetClass() == UCFVehicleData::StaticClass()
		? LoadedVehicleData
		: nullptr;
}

// selected VehicleData의 current MountProfiles를 read-only option projection으로 다시 구성합니다.
void FCFEquipmentBuilderVM::RebuildCompatibilityMountOptions()
{
	CompatibilityMountOptions.Reset();

	// current transient identity에서 fresh resolve한 exact VehicleData입니다.
	UCFVehicleData* VehicleData = ResolveCompatibilityVehicleData();
	if (VehicleData == nullptr)
	{
		return;
	}

	for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
	{
		// VehicleData의 persisted mount fields를 복사한 read-only picker row입니다.
		FCFEquipmentMountOption MountOption;
		MountOption.MountProfileId = MountProfile.MountProfileId;
		MountOption.MountType = MountProfile.MountType;
		MountOption.SizeLimit = MountProfile.SizeLimit;
		CompatibilityMountOptions.Add(MoveTemp(MountOption));
	}
}

// current Vehicle/Mount/Draft source를 exact3 compatibility result로 평가합니다.
FCFEquipmentCompatibilityResult FCFEquipmentBuilderVM::EvaluateCompatibilityProbe() const
{
	// 이번 fresh evaluation의 bounded advisory 결과입니다.
	FCFEquipmentCompatibilityResult Result;
	Result.VehicleDataObjectPath = CompatibilityVehicleDataObjectPath;
	Result.MountProfileId = CompatibilityMountProfileId;

	if (CompatibilityVehicleDataObjectPath.IsNull())
	{
		Result.Reason = ECFEquipmentCompatibilityReason::NoVehicleSelected;
		Result.Diagnostic = TEXT("호환성을 확인할 VehicleData를 선택하세요.");
		return Result;
	}

	// fresh current source에서 exact class로 resolve한 VehicleData입니다.
	UCFVehicleData* VehicleData = ResolveCompatibilityVehicleData();
	if (VehicleData == nullptr)
	{
		Result.Reason = ECFEquipmentCompatibilityReason::VehicleSourceUnresolved;
		Result.Diagnostic = TEXT("선택한 VehicleData를 exact class로 resolve할 수 없습니다.");
		return Result;
	}

	// selected Vehicle source의 package입니다.
	UPackage* VehiclePackage = VehicleData->GetOutermost();
	// selected Vehicle source의 long package name입니다.
	const FString VehiclePackageName = VehiclePackage != nullptr ? VehiclePackage->GetName() : FString();
	if (VehiclePackage == nullptr || VehiclePackageName.IsEmpty() || !FPackageName::DoesPackageExist(VehiclePackageName))
	{
		Result.Reason = ECFEquipmentCompatibilityReason::VehicleSourceUnpersisted;
		Result.Diagnostic = TEXT("선택한 VehicleData가 disk에 persist된 source가 아니라 compatibility를 확정할 수 없습니다.");
		return Result;
	}
	if (VehiclePackage->IsDirty())
	{
		Result.Reason = ECFEquipmentCompatibilityReason::VehicleSourceDirty;
		Result.Diagnostic = TEXT("선택한 VehicleData package가 dirty 상태입니다. 저장된 source 기준으로 다시 확인하세요.");
		return Result;
	}
	if (CompatibilityMountProfileId.IsNone())
	{
		Result.Reason = ECFEquipmentCompatibilityReason::MountProfileIdNone;
		Result.Diagnostic = TEXT("호환성을 확인할 MountProfile을 선택하세요.");
		return Result;
	}

	// current VehicleData에서 같은 MountProfileId를 가진 항목 수입니다.
	int32 MatchingMountCount = 0;
	// exact1일 때 평가에 사용할 current MountProfile입니다.
	const FCFVehicleMountProfile* SelectedMountProfile = nullptr;
	for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
	{
		if (MountProfile.MountProfileId != CompatibilityMountProfileId)
		{
			continue;
		}
		++MatchingMountCount;
		SelectedMountProfile = &MountProfile;
	}

	if (MatchingMountCount == 0 || SelectedMountProfile == nullptr)
	{
		Result.Reason = ECFEquipmentCompatibilityReason::MountProfileMissing;
		Result.Diagnostic = TEXT("선택한 MountProfileId가 current VehicleData에 없습니다. Mount 목록을 다시 선택하세요.");
		return Result;
	}
	if (MatchingMountCount != 1)
	{
		Result.Reason = ECFEquipmentCompatibilityReason::MountProfileIdDuplicate;
		Result.Diagnostic = TEXT("선택한 MountProfileId가 current VehicleData에서 중복되어 compatibility를 확정할 수 없습니다.");
		return Result;
	}

	Result.MountType = SelectedMountProfile->MountType;
	Result.SizeLimit = SelectedMountProfile->SizeLimit;
	if (SelectedMountProfile->MountType == ECFVehicleMountType::None)
	{
		Result.Reason = ECFEquipmentCompatibilityReason::MountTypeNone;
		Result.Diagnostic = TEXT("선택한 MountProfile의 MountType=None이라 compatibility를 평가할 수 없습니다.");
		return Result;
	}

	// durable authoring과 동일한 기존 intrinsic Draft validation 결과입니다.
	const FCFEquipmentDraftValidation IntrinsicValidation = BuildValidationForDraft(Draft);
	if (!bHasActiveDraft || !IntrinsicValidation.bPackageComplete || IntrinsicValidation.bHasBlockingErrors)
	{
		Result.Reason = ECFEquipmentCompatibilityReason::EquipmentIntrinsicInvalid;
		Result.Diagnostic = TEXT("Equipment Draft의 intrinsic 조립/child validation을 먼저 해결해야 compatibility를 평가할 수 있습니다.");
		return Result;
	}

	// current runtime EquipmentPreset mount 의미와 동일한 bounded 판정 이유입니다.
	ECFEquipmentCompatibilityReason CompatibilityReason = ECFEquipmentCompatibilityReason::None;
	// 사용자에게 보여줄 bounded compatibility 설명입니다.
	FString CompatibilityDiagnostic;
	const bool bCompatible = EvaluateDraftMountContract(
		SelectedMountProfile->MountType,
		SelectedMountProfile->SizeLimit,
		CompatibilityReason,
		CompatibilityDiagnostic);

	Result.State = bCompatible
		? ECFEquipmentCompatibilityState::Compatible
		: ECFEquipmentCompatibilityState::Incompatible;
	Result.Reason = CompatibilityReason;
	Result.Diagnostic = MoveTemp(CompatibilityDiagnostic);
	return Result;
}

// current Equipment Draft를 runtime EquipmentPreset mount 의미와 동일하게 read-only 평가합니다.
bool FCFEquipmentBuilderVM::EvaluateDraftMountContract(
	const ECFVehicleMountType MountType,
	const ECFVehicleWeaponSize SizeLimit,
	ECFEquipmentCompatibilityReason& OutReason,
	FString& OutDiagnostic) const
{
	OutReason = ECFEquipmentCompatibilityReason::None;
	OutDiagnostic.Reset();

	// Draft의 TurretMountData를 transient runtime-equivalent preset에 투영할 exact typed object입니다.
	UCFTurretMountData* TurretMountData = Draft.DefaultTurretMountData.IsNull()
		? nullptr
		: LoadObject<UCFTurretMountData>(nullptr, *Draft.DefaultTurretMountData.ToSoftObjectPath().ToString());
	// Draft의 WeaponData를 transient runtime-equivalent preset에 투영할 exact typed object입니다.
	UCFWeaponData* WeaponData = Draft.DefaultWeaponData.IsNull()
		? nullptr
		: LoadObject<UCFWeaponData>(nullptr, *Draft.DefaultWeaponData.ToSoftObjectPath().ToString());
	// Draft의 VehicleSensorData를 transient runtime-equivalent preset에 투영할 exact typed object입니다.
	UCFVehicleSensorData* SensorData = Draft.DefaultSensorData.IsNull()
		? nullptr
		: LoadObject<UCFVehicleSensorData>(nullptr, *Draft.DefaultSensorData.ToSoftObjectPath().ToString());

	// Product UObject를 수정하지 않고 existing UCFEquipmentPresetData::CanUseOnMount authority를 호출할 transient preset입니다.
	UCFEquipmentPresetData* ProbePreset = NewObject<UCFEquipmentPresetData>(GetTransientPackage(), NAME_None, RF_Transient);
	ProbePreset->EquipmentId = Draft.EquipmentId;
	ProbePreset->DisplayName = Draft.DisplayName;
	ProbePreset->RequiredMountType = Draft.RequiredMountType;
	ProbePreset->RequiredWeaponSize = Draft.RequiredWeaponSize;
	ProbePreset->DefaultTurretMountData = TurretMountData;
	ProbePreset->DefaultWeaponData = WeaponData;
	ProbePreset->DefaultSensorData = SensorData;

	if (ProbePreset->CanUseOnMount(MountType, SizeLimit))
	{
		OutDiagnostic = FString::Printf(
			TEXT("Compatible: 현재 Equipment Draft는 MountType=%s, SizeLimit=%s에서 existing EquipmentPreset runtime 계약을 통과합니다."),
			*UEnum::GetValueAsString(MountType),
			*UEnum::GetValueAsString(SizeLimit));
		return true;
	}

	if (Draft.RequiredMountType != ECFVehicleMountType::None && Draft.RequiredMountType != MountType)
	{
		OutReason = ECFEquipmentCompatibilityReason::RequiredMountTypeMismatch;
		OutDiagnostic = TEXT("Incompatible: EquipmentPreset의 RequiredMountType이 선택한 MountType과 일치하지 않습니다.");
		return false;
	}
	if (Draft.RequiredWeaponSize != ECFVehicleWeaponSize::None
		&& (SizeLimit == ECFVehicleWeaponSize::None
			|| static_cast<uint8>(Draft.RequiredWeaponSize) > static_cast<uint8>(SizeLimit)))
	{
		OutReason = ECFEquipmentCompatibilityReason::RequiredWeaponSizeTooLarge;
		OutDiagnostic = TEXT("Incompatible: EquipmentPreset의 RequiredWeaponSize가 선택한 Mount의 SizeLimit보다 큽니다.");
		return false;
	}
	if (WeaponData != nullptr && !WeaponData->SupportsMountType(MountType))
	{
		OutReason = ECFEquipmentCompatibilityReason::WeaponMountUnsupported;
		OutDiagnostic = TEXT("Incompatible: 선택한 WeaponData가 현재 MountType을 지원하지 않습니다.");
		return false;
	}
	if (WeaponData != nullptr && !WeaponData->SupportsWeaponSize(SizeLimit))
	{
		OutReason = ECFEquipmentCompatibilityReason::WeaponSizeUnsupported;
		OutDiagnostic = TEXT("Incompatible: 선택한 WeaponData의 WeaponSize가 현재 Mount SizeLimit을 통과하지 못합니다.");
		return false;
	}

	OutReason = ECFEquipmentCompatibilityReason::RuntimeContractRejected;
	OutDiagnostic = TEXT("Incompatible: payload-for-mount 또는 SensorConfig를 포함한 existing EquipmentPreset runtime 계약이 현재 Mount를 거부했습니다.");
	return false;
}

// current transient Draft의 package/child/reference chain을 bounded read-only validation으로 평가합니다.
FCFEquipmentDraftValidation FCFEquipmentBuilderVM::BuildCurrentValidation() const
{
	return BuildValidationForDraft(Draft);
}

// 임의 transient Draft를 동일한 EBA-P0-02 validation hierarchy로 평가합니다.
FCFEquipmentDraftValidation FCFEquipmentBuilderVM::BuildValidationForDraft(const FCFEquipmentPresetDraft& DraftToValidate) const
{
	// 이번 bounded validation의 누적 결과입니다.
	FCFEquipmentDraftValidation Validation;

	// 무장 패키지에 필요한 두 reference가 모두 있는지 나타냅니다.
	const bool bHasWeaponPair = !DraftToValidate.DefaultTurretMountData.IsNull() && !DraftToValidate.DefaultWeaponData.IsNull();

	// Scanner payload reference가 있는지 나타냅니다.
	const bool bHasSensor = !DraftToValidate.DefaultSensorData.IsNull();

	if (DraftToValidate.DraftMode == ECFEquipmentDraftMode::Weapon)
	{
		Validation.bPackageComplete = bHasWeaponPair && !bHasSensor;
		if (bHasSensor)
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("장비 구성"), TEXT("무장 장비에는 센서 데이터를 함께 둘 수 없습니다. 무장 또는 스캐너 중 하나의 구성만 사용해야 합니다."));
		}
		if (DraftToValidate.DefaultTurretMountData.IsNull())
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("터렛 마운트"), TEXT("무장 장비에는 터렛 마운트 데이터가 필요합니다."));
		}
		if (DraftToValidate.DefaultWeaponData.IsNull())
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("무기 데이터"), TEXT("무장 장비에는 무기 데이터가 필요합니다."));
		}
		if (DraftToValidate.RequiredMountType == ECFVehicleMountType::Utility)
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("요구 장착 타입"), TEXT("무장 장비는 유틸리티 전용 장착 타입을 사용할 수 없습니다. 제한 없음 또는 무기 데이터가 지원하는 무장 장착 타입을 선택하세요."));
		}

		if (!DraftToValidate.DefaultTurretMountData.IsNull())
		{
			// 선택한 TurretMountData를 bounded read-only 검증하기 위해 load한 객체입니다.
			UCFTurretMountData* TurretMountData = LoadObject<UCFTurretMountData>(nullptr, *DraftToValidate.DefaultTurretMountData.ToSoftObjectPath().ToString());
			if (!TurretMountData || TurretMountData->GetClass() != UCFTurretMountData::StaticClass())
			{
				AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("터렛 마운트"), TEXT("선택한 터렛 마운트 데이터를 올바른 형식으로 읽을 수 없습니다."));
			}
			else if (TurretMountData->TurretMountId.IsNone())
			{
				AddValidationItem(Validation, ECFEquipmentValidationSeverity::Warning, TEXT("터렛 마운트 제한 검증"), TEXT("터렛 마운트 ID가 비어 있습니다. 현재는 완전한 자동 검증이 없어 이 항목만으로 전체 정상 여부를 판단하지 않습니다."));
			}
			else
			{
				AddValidationItem(Validation, ECFEquipmentValidationSeverity::Limited, TEXT("터렛 마운트 제한 검증"), FString::Printf(TEXT("참조와 터렛 마운트 ID(%s)를 확인했습니다. 메시/소켓 전체 구조까지 자동 검증한 것은 아닙니다."), *TurretMountData->TurretMountId.ToString()));
			}
		}

		if (!DraftToValidate.DefaultWeaponData.IsNull())
		{
			// 선택한 WeaponData native validation과 child chain을 읽기 위해 load한 객체입니다.
			UCFWeaponData* WeaponData = LoadObject<UCFWeaponData>(nullptr, *DraftToValidate.DefaultWeaponData.ToSoftObjectPath().ToString());
			if (!WeaponData || WeaponData->GetClass() != UCFWeaponData::StaticClass())
			{
				AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("무기 데이터"), TEXT("선택한 무기 데이터를 올바른 형식으로 읽을 수 없습니다."));
			}
			else
			{
				if (DraftToValidate.RequiredMountType != ECFVehicleMountType::None
					&& DraftToValidate.RequiredMountType != ECFVehicleMountType::Utility
					&& !WeaponData->SupportsMountType(DraftToValidate.RequiredMountType))
				{
					AddValidationItem(
						Validation,
						ECFEquipmentValidationSeverity::Error,
						TEXT("요구 장착 타입"),
						FString::Printf(
							TEXT("현재 요구 장착 타입(%s)을 선택한 무기 데이터가 지원하지 않습니다. 장비 프리셋의 요구 타입과 무기 데이터의 호환 타입을 일치시키세요."),
							*UEnum::GetValueAsString(DraftToValidate.RequiredMountType)));
				}

				// WeaponData native contract가 반환한 상세 오류 목록입니다.
				TArray<FText> WeaponErrors;
				// 기존 WeaponData native validation 결과입니다.
				const bool bWeaponValid = WeaponData->ValidateWeaponDataContract(WeaponErrors);
				if (bWeaponValid)
				{
					AddValidationItem(Validation, ECFEquipmentValidationSeverity::Pass, TEXT("무기 데이터"), FString::Printf(TEXT("무기 데이터 검증 통과: %s"), *WeaponData->WeaponId.ToString()));
				}
				else
				{
					for (const FText& WeaponError : WeaponErrors)
					{
						AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("무기 데이터"), BuildUserValidationMessage(WeaponError.ToString()));
					}
				}

				if (WeaponData->DefaultAmmoData)
				{
					// WeaponData가 참조하는 AmmoData native identity validation 결과입니다.
					const bool bAmmoValid = WeaponData->DefaultAmmoData->IsAmmoDataValid();
					AddValidationItem(
						Validation,
						bAmmoValid ? ECFEquipmentValidationSeverity::Pass : ECFEquipmentValidationSeverity::Error,
						TEXT("탄약 데이터"),
						bAmmoValid
							? FString::Printf(TEXT("탄약 데이터 검증 통과: %s"), *WeaponData->DefaultAmmoData->AmmoId.ToString())
							: TEXT("무기 데이터가 참조하는 탄약 데이터의 탄약 ID가 유효하지 않습니다."));
				}

				if (WeaponData->DefaultProjectileData)
				{
					// WeaponData가 참조하는 ProjectileData입니다.
					const UCFProjectileData* ProjectileData = WeaponData->DefaultProjectileData;
					AddValidationItem(
						Validation,
						ProjectileData->ProjectileId.IsNone() ? ECFEquipmentValidationSeverity::Warning : ECFEquipmentValidationSeverity::Limited,
						TEXT("발사체 제한 검증"),
						ProjectileData->ProjectileId.IsNone()
							? TEXT("발사체 ID가 비어 있습니다. 현재는 발사체 데이터 전체를 자동 검증하지 않습니다.")
							: FString::Printf(TEXT("발사체 참조와 발사체 ID(%s)를 확인했습니다. 전체 발사체 설정까지 자동 검증한 것은 아닙니다."), *ProjectileData->ProjectileId.ToString()));

					if (ProjectileData->DefaultDamageData)
					{
						// ProjectileData가 참조하는 DamageData입니다.
						const UCFDamageData* DamageData = ProjectileData->DefaultDamageData;
						AddValidationItem(
							Validation,
							DamageData->DamageId.IsNone() ? ECFEquipmentValidationSeverity::Warning : ECFEquipmentValidationSeverity::Limited,
							TEXT("피해 데이터 제한 검증"),
							DamageData->DamageId.IsNone()
								? TEXT("피해 데이터 ID가 비어 있습니다. 현재는 피해 데이터 전체를 자동 검증하지 않습니다.")
								: FString::Printf(TEXT("피해 데이터 참조와 피해 데이터 ID(%s)를 확인했습니다. 기존 데이터 저작 저장 경로는 그대로 유지합니다."), *DamageData->DamageId.ToString()));
					}
					else
					{
						AddValidationItem(Validation, ECFEquipmentValidationSeverity::Warning, TEXT("피해 데이터"), TEXT("발사체에 기본 피해 데이터가 지정되지 않았습니다. 현재 하위 데이터 연결을 확인하세요."));
					}
				}
			}
		}
	}
	else
	{
		Validation.bPackageComplete = bHasSensor && !bHasWeaponPair
			&& DraftToValidate.DefaultTurretMountData.IsNull()
			&& DraftToValidate.DefaultWeaponData.IsNull()
			&& DraftToValidate.RequiredWeaponSize == ECFVehicleWeaponSize::None;

		if (!DraftToValidate.DefaultTurretMountData.IsNull() || !DraftToValidate.DefaultWeaponData.IsNull())
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("장비 구성"), TEXT("스캐너 장비에는 터렛 마운트 데이터 또는 무기 데이터를 함께 둘 수 없습니다."));
		}
		if (!bHasSensor)
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("센서 데이터"), TEXT("스캐너 장비에는 센서 데이터가 필요합니다."));
		}
		if (DraftToValidate.RequiredWeaponSize != ECFVehicleWeaponSize::None)
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("요구 장착 크기"), TEXT("스캐너 장비의 요구 장착 크기는 해당 없음으로 고정됩니다."));
		}
		if (DraftToValidate.RequiredMountType != ECFVehicleMountType::Utility)
		{
			AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("요구 장착 타입"), TEXT("스캐너 장비의 요구 장착 타입은 유틸리티로 고정됩니다."));
		}

		if (!DraftToValidate.DefaultSensorData.IsNull())
		{
			// 선택한 VehicleSensorData native validation을 실행하기 위해 load한 객체입니다.
			UCFVehicleSensorData* SensorData = LoadObject<UCFVehicleSensorData>(nullptr, *DraftToValidate.DefaultSensorData.ToSoftObjectPath().ToString());
			if (!SensorData || SensorData->GetClass() != UCFVehicleSensorData::StaticClass())
			{
				AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("센서 데이터"), TEXT("선택한 센서 데이터를 올바른 형식으로 읽을 수 없습니다."));
			}
			else
			{
				// SensorData native contract가 반환한 상세 오류 목록입니다.
				TArray<FText> SensorErrors;
				// 기존 SensorData native validation 결과입니다.
				const bool bSensorValid = SensorData->ValidateSensorDataContract(SensorErrors);
				if (bSensorValid)
				{
					AddValidationItem(Validation, ECFEquipmentValidationSeverity::Pass, TEXT("센서 데이터"), TEXT("센서 데이터 검증 통과"));
				}
				else
				{
					for (const FText& SensorError : SensorErrors)
					{
						AddValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("센서 데이터"), BuildUserValidationMessage(SensorError.ToString()));
					}
				}
			}
		}
	}

	if (Validation.bPackageComplete && !Validation.bHasBlockingErrors)
	{
		AddValidationItem(Validation, ECFEquipmentValidationSeverity::Pass, TEXT("장비 패키지"), TEXT("선택한 장비 구성의 필수 데이터가 준비되었습니다."));
	}
	return Validation;
}

// validation result에 새 항목을 추가하고 Error severity면 blocking 상태도 함께 올립니다.
void FCFEquipmentBuilderVM::AddValidationItem(
	FCFEquipmentDraftValidation& Validation,
	ECFEquipmentValidationSeverity Severity,
	const FString& Title,
	const FString& Message)
{
	// 누적 결과에 추가할 사용자 표시 항목입니다.
	FCFEquipmentValidationItem Item;
	Item.Severity = Severity;
	Item.Title = Title;
	Item.Message = Message;
	Validation.Items.Add(MoveTemp(Item));
	if (Severity == ECFEquipmentValidationSeverity::Error)
	{
		Validation.bHasBlockingErrors = true;
	}
}

// current draft/source 상태를 사용자용 한 줄 요약으로 반환합니다.
FString FCFEquipmentBuilderVM::BuildDraftStateSummary() const
{
	if (!bHasActiveDraft)
	{
		return TEXT("작업 대상을 선택하지 않았습니다.");
	}
	if (bIsNewDraft)
	{
		return bDraftDirty
			? TEXT("새 장비 프리셋 초안을 편집 중입니다. 아직 저장된 Asset은 없습니다.")
			: TEXT("새 장비 프리셋 초안을 시작했습니다. 아직 저장된 Asset은 없습니다.");
	}
	return bDraftDirty
		? TEXT("저장된 프리셋을 기준으로 transient 초안이 변경되었습니다. Product Asset은 아직 수정되지 않았습니다.")
		: TEXT("저장된 프리셋과 transient 초안이 같은 상태입니다. Product Asset은 수정되지 않았습니다.");
}

// current Data Asset Manager inventory에서 exact CFEquipmentPresetData class rows만 browser DTO로 투영합니다.
bool FCFEquipmentBuilderVM::RebuildPresetEntries(FString& OutError)
{
	PresetEntries.Reset();
	// Data Asset Manager가 제공한 current metadata-only inventory입니다.
	const FCFDAInventoryResult& Inventory = DataManagementViewModel.GetInventory();
	if (!Inventory.IsInventoryComplete())
	{
		OutError = Inventory.RegistryMessage.IsEmpty()
			? TEXT("EquipmentPreset 목록을 만들 수 있도록 Asset Registry가 아직 준비되지 않았습니다.")
			: Inventory.RegistryMessage;
		return false;
	}
	// exact EquipmentPreset class path입니다.
	const FString EquipmentPresetClassPath = GetEquipmentPresetClassPath();
	for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
	{
		if (AssetRecord.ClassPath != EquipmentPresetClassPath)
		{
			continue;
		}
		// metadata-only browser row입니다.
		FCFEquipmentPresetListEntry Entry;
		Entry.AssetName = AssetRecord.AssetName;
		Entry.ObjectPath = AssetRecord.ObjectPath;
		Entry.PackagePath = AssetRecord.PackagePath;
		PresetEntries.Add(MoveTemp(Entry));
	}
	OutError.Reset();
	return true;
}

// exact object path가 current EquipmentPreset browser에 존재하는지 확인합니다.
bool FCFEquipmentBuilderVM::HasPresetEntry(const FString& ObjectPath) const
{
	return PresetEntries.ContainsByPredicate([&ObjectPath](const FCFEquipmentPresetListEntry& Entry)
	{
		return Entry.ObjectPath == ObjectPath;
	});
}

// persisted EquipmentPreset payload를 UI-only Weapon/Scanner mode로 해석합니다.
ECFEquipmentDraftMode FCFEquipmentBuilderVM::InferDraftMode(const UCFEquipmentPresetData& Preset)
{
	if (Preset.DefaultSensorData != nullptr && Preset.DefaultTurretMountData == nullptr && Preset.DefaultWeaponData == nullptr)
	{
		return ECFEquipmentDraftMode::Scanner;
	}
	return ECFEquipmentDraftMode::Weapon;
}

// source/current draft의 P0-01 edit state가 같은지 비교합니다. 이 비교는 Review semantic fingerprint를 대체하지 않습니다.
bool FCFEquipmentBuilderVM::AreDraftsEquivalentForEditing(const FCFEquipmentPresetDraft& Left, const FCFEquipmentPresetDraft& Right)
{
	return Left.EquipmentId == Right.EquipmentId
		&& Left.DisplayName.ToString() == Right.DisplayName.ToString()
		&& Left.bPreserveExistingDisplayName == Right.bPreserveExistingDisplayName
		&& Left.RequiredMountType == Right.RequiredMountType
		&& Left.RequiredWeaponSize == Right.RequiredWeaponSize
		&& Left.DefaultTurretMountData.ToSoftObjectPath() == Right.DefaultTurretMountData.ToSoftObjectPath()
		&& Left.DefaultWeaponData.ToSoftObjectPath() == Right.DefaultWeaponData.ToSoftObjectPath()
		&& Left.DefaultSensorData.ToSoftObjectPath() == Right.DefaultSensorData.ToSoftObjectPath()
		&& Left.DraftMode == Right.DraftMode;
}

// current Draft의 active/inactive payload를 mode별 transient cache에 동기화합니다.
void FCFEquipmentBuilderVM::RefreshModeCachesFromDraft()
{
	WeaponModeTurretMountCache = Draft.DefaultTurretMountData;
	WeaponModeWeaponDataCache = Draft.DefaultWeaponData;
	WeaponModeMountTypeCache = Draft.DraftMode == ECFEquipmentDraftMode::Weapon && Draft.RequiredMountType != ECFVehicleMountType::Utility
		? Draft.RequiredMountType
		: ECFVehicleMountType::Turret;
	WeaponModeSizeCache = Draft.RequiredWeaponSize == ECFVehicleWeaponSize::None
		? ECFVehicleWeaponSize::Large
		: Draft.RequiredWeaponSize;
	ScannerModeSensorDataCache = Draft.DefaultSensorData;
}

// Draft/target semantic 변경 뒤 기존 Reviewed approval을 즉시 무효화합니다.
void FCFEquipmentBuilderVM::InvalidateReviewedApproval()
{
	ReviewState = FCFEquipmentReviewState();
}

// current draft와 source snapshot을 비교해 presentation dirty state를 갱신합니다.
void FCFEquipmentBuilderVM::RecomputeDraftDirty()
{
	InvalidateReviewedApproval();
	bDraftDirty = bHasActiveDraft && !AreDraftsEquivalentForEditing(Draft, SourceDraft);
	RefreshCompatibilityProbe();
}

// current selection과 transient draft state를 모두 해제합니다.
void FCFEquipmentBuilderVM::ClearCurrentDraft()
{
	Draft = FCFEquipmentPresetDraft();
	SourceDraft = FCFEquipmentPresetDraft();
	WeaponModeTurretMountCache.Reset();
	WeaponModeWeaponDataCache.Reset();
	WeaponModeMountTypeCache = ECFVehicleMountType::Turret;
	WeaponModeSizeCache = ECFVehicleWeaponSize::Large;
	ScannerModeSensorDataCache.Reset();
	SelectedObjectPath.Reset();
	CreateAssetName.Reset();
	CreateTargetObjectPath.Reset();
	InvalidateReviewedApproval();
	bHasActiveDraft = false;
	bIsNewDraft = false;
	bDraftDirty = false;
	RefreshCompatibilityProbe();
}
