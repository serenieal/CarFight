// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentDurable.cpp
// Version: v1.1.0
// Date: 2026-09-17
// Description: CF-FQ-054 EquipmentPreset durable semantic/Review adapter 구현입니다.
// Changelog:
// - v1.1.0: 기존 localized DisplayName을 preserve-only semantic으로 보존하는 Contract Revision 2를 추가.
// - v1.0.0: Literal FText exact7 fingerprint, canonical ReviewProposalDigest, persisted/clean child guard와 shared durable callbacks를 구현.
// Migration:
// - Product/child Asset을 직접 저장하지 않습니다.
// - localized DisplayName preserve mode에서는 해당 FText를 materialize하지 않아 기존 localization identity를 그대로 유지합니다.
// - durable sequencing은 CFDADurableCore가 계속 단독 소유합니다.

#include "EquipmentAuthoring/CFEquipmentDurable.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFEquipmentPresetData.h"
#include "CFProjectileData.h"
#include "CFTurretMountData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFEquipmentDurablePrivate
{
	// Current target native class path constant입니다.
	const TCHAR* TargetClassPath = TEXT("/Script/CarFight_Re.CFEquipmentPresetData");

	// Localized DisplayName preserve semantic을 포함한 current contract revision입니다.
	constexpr int32 ContractRevision = 2;

	// Create current-state stable sentinel입니다.
	const TCHAR* AbsentTargetToken = TEXT("AbsentTarget");

	// Reviewed validation terminal stable token입니다.
	const TCHAR* ReadyValidationToken = TEXT("Ready");

	// 기존 localized DisplayName을 다시 쓰지 않는 semantic stable sentinel입니다.
	const TCHAR* PreservedDisplayNameToken = TEXT("PreserveExistingLocalizedDisplayName");

	// MountType을 durable semantic token으로 변환합니다.
	FString BuildMountTypeToken(ECFVehicleMountType MountType)
	{
		switch (MountType)
		{
		case ECFVehicleMountType::None: return TEXT("None");
		case ECFVehicleMountType::Fixed: return TEXT("Fixed");
		case ECFVehicleMountType::Gimbal: return TEXT("Gimbal");
		case ECFVehicleMountType::Turret: return TEXT("Turret");
		case ECFVehicleMountType::Launcher: return TEXT("Launcher");
		case ECFVehicleMountType::Utility: return TEXT("Utility");
		default: return FString();
		}
	}

	// WeaponSize를 durable semantic token으로 변환합니다.
	FString BuildWeaponSizeToken(ECFVehicleWeaponSize WeaponSize)
	{
		switch (WeaponSize)
		{
		case ECFVehicleWeaponSize::None: return TEXT("None");
		case ECFVehicleWeaponSize::Small: return TEXT("Small");
		case ECFVehicleWeaponSize::Medium: return TEXT("Medium");
		case ECFVehicleWeaponSize::Large: return TEXT("Large");
		default: return FString();
		}
	}

	// nullable Draft reference를 canonical top-level SoftObjectPath로 정규화합니다.
	bool CanonicalizeReferencePath(
		const FSoftObjectPath& SourcePath,
		const FString& FieldPath,
		FSoftObjectPath& OutPath,
		FString& OutError)
	{
		OutPath.Reset();
		if (SourcePath.IsNull())
		{
			return true;
		}

		// Canonical path parse diagnostic입니다.
		TArray<FCFDAStagingIssue> Issues;
		if (!CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
			SourcePath.ToString(),
			FieldPath,
			false,
			OutPath,
			Issues))
		{
			OutError = Issues.IsEmpty() ? FString::Printf(TEXT("%s reference path를 canonicalize하지 못했습니다."), *FieldPath) : Issues[0].Message;
			return false;
		}
		return true;
	}

	// exact typed child dependency 하나의 persisted/current clean truth를 검증합니다.
	template <typename TAsset>
	bool ValidateDependency(
		TAsset* Asset,
		const FString& FieldPath,
		const FString& ForcedUnconfirmedObjectPath,
		FString& OutError)
	{
		if (Asset == nullptr || Asset->GetClass() != TAsset::StaticClass())
		{
			OutError = FString::Printf(TEXT("ChildStateUnconfirmed: %s exact class object를 확인하지 못했습니다."), *FieldPath);
			return false;
		}

		// 현재 child exact object path입니다.
		const FSoftObjectPath ObjectPath(Asset);
		// Child persisted truth에 사용할 canonical path입니다.
		FSoftObjectPath CanonicalPath;
		if (!CanonicalizeReferencePath(ObjectPath, FieldPath, CanonicalPath, OutError))
		{
			OutError = FString::Printf(TEXT("ChildStateUnconfirmed: %s"), *OutError);
			return false;
		}

		if (!ForcedUnconfirmedObjectPath.IsEmpty() && CanonicalPath.ToString() == ForcedUnconfirmedObjectPath)
		{
			OutError = FString::Printf(TEXT("ChildStateUnconfirmed: Automation fault가 %s persisted/clean truth를 강제 차단했습니다."), *FieldPath);
			return false;
		}

		if (Asset->HasAnyFlags(RF_Transient | RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			OutError = FString::Printf(TEXT("ChildStateUnconfirmed: %s object가 transient/CDO/archetype 상태입니다."), *FieldPath);
			return false;
		}

		// Child package입니다.
		UPackage* Package = Asset->GetOutermost();
		if (Package == nullptr || Package == GetTransientPackage() || Package->IsDirty())
		{
			OutError = FString::Printf(TEXT("ChildStateUnconfirmed: %s package가 persisted clean 상태가 아닙니다."), *FieldPath);
			return false;
		}

		// Current Asset Registry authority입니다.
		FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AssetRegistry = RegistryModule.Get();
		if (AssetRegistry.IsLoadingAssets())
		{
			OutError = TEXT("ChildStateUnconfirmed: Asset Registry가 loading 중이라 child persisted truth를 확정할 수 없습니다.");
			return false;
		}

		// Exact object path의 persisted metadata입니다.
		const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(CanonicalPath, false, false);
		if (!AssetData.IsValid() || AssetData.AssetClassPath != TAsset::StaticClass()->GetClassPathName())
		{
			OutError = FString::Printf(TEXT("ChildStateUnconfirmed: %s의 persisted exact-class Asset Registry metadata가 없습니다."), *FieldPath);
			return false;
		}
		return true;
	}
}

// EquipmentPreset exact native class path를 반환합니다.
FString FCFEquipmentDurable::GetTargetClassPath()
{
	return CFEquipmentDurablePrivate::TargetClassPath;
}

// Equipment Authoring durable contract revision을 반환합니다.
int32 FCFEquipmentDurable::GetContractRevision()
{
	return CFEquipmentDurablePrivate::ContractRevision;
}

// Create current-state binding에 사용하는 stable sentinel을 반환합니다.
const TCHAR* FCFEquipmentDurable::GetAbsentTargetToken()
{
	return CFEquipmentDurablePrivate::AbsentTargetToken;
}

// Reviewed validation terminal state의 stable token을 반환합니다.
const TCHAR* FCFEquipmentDurable::GetReadyValidationToken()
{
	return CFEquipmentDurablePrivate::ReadyValidationToken;
}

// localized DisplayName preserve-mode fingerprint에 사용하는 stable sentinel을 반환합니다.
const TCHAR* FCFEquipmentDurable::GetPreservedDisplayNameToken()
{
	return CFEquipmentDurablePrivate::PreservedDisplayNameToken;
}

// 현재 FText를 Equipment Builder가 lossless literal로 직접 편집할 수 있는지 확인합니다.
bool FCFEquipmentDurable::CanEditDisplayNameLosslessly(const FText& DisplayName)
{
	// Literal text projection 결과입니다.
	FCFDAStagingLiteralText LiteralDisplayName;
	// Unsupported representation diagnostics입니다.
	TArray<FCFDAStagingIssue> TextIssues;
	return CFDACommonPrimitives::ReadLiteralTextFromAsset(DisplayName, TEXT("DisplayName"), LiteralDisplayName, TextIssues);
}

// transient Draft exact7을 lossless/preserve-safe durable semantic snapshot으로 정규화합니다.
bool FCFEquipmentDurable::BuildSnapshotFromDraft(
	const FCFEquipmentPresetDraft& Draft,
	FCFEquipmentSemanticSnapshot& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFEquipmentSemanticSnapshot();
	OutError.Reset();

	OutSnapshot.EquipmentId = Draft.EquipmentId;
	OutSnapshot.bPreserveExistingDisplayName = Draft.bPreserveExistingDisplayName;
	if (Draft.bPreserveExistingDisplayName)
	{
		OutSnapshot.DisplayNameSource = GetPreservedDisplayNameToken();
	}
	else
	{
		// DisplayName Literal source readback입니다.
		FCFDAStagingLiteralText LiteralDisplayName;
		// Unsupported FText representation diagnostics입니다.
		TArray<FCFDAStagingIssue> TextIssues;
		if (!CFDACommonPrimitives::ReadLiteralTextFromAsset(Draft.DisplayName, TEXT("DisplayName"), LiteralDisplayName, TextIssues))
		{
			OutError = TextIssues.IsEmpty() ? TEXT("표시 이름을 손실 없이 편집 가능한 텍스트로 표현할 수 없습니다.") : TextIssues[0].Message;
			return false;
		}
		OutSnapshot.DisplayNameSource = LiteralDisplayName.Text;
	}

	OutSnapshot.RequiredMountType = Draft.RequiredMountType;
	OutSnapshot.RequiredWeaponSize = Draft.RequiredWeaponSize;
	if (!CFEquipmentDurablePrivate::CanonicalizeReferencePath(Draft.DefaultTurretMountData.ToSoftObjectPath(), TEXT("DefaultTurretMountData"), OutSnapshot.DefaultTurretMountDataPath, OutError)
		|| !CFEquipmentDurablePrivate::CanonicalizeReferencePath(Draft.DefaultWeaponData.ToSoftObjectPath(), TEXT("DefaultWeaponData"), OutSnapshot.DefaultWeaponDataPath, OutError)
		|| !CFEquipmentDurablePrivate::CanonicalizeReferencePath(Draft.DefaultSensorData.ToSoftObjectPath(), TEXT("DefaultSensorData"), OutSnapshot.DefaultSensorDataPath, OutError))
	{
		return false;
	}

	return BuildSemanticFingerprint(OutSnapshot, OutSnapshot.SemanticFingerprint, OutError);
}

// persisted EquipmentPreset exact7을 lossless/preserve-safe durable semantic snapshot으로 추출합니다.
bool FCFEquipmentDurable::ExtractSnapshotFromAsset(
	const UCFEquipmentPresetData& Asset,
	FCFEquipmentSemanticSnapshot& OutSnapshot,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutSnapshot = FCFEquipmentSemanticSnapshot();
	OutIssues.Reset();
	OutSnapshot.EquipmentId = Asset.EquipmentId;

	// Persisted DisplayName Literal source입니다.
	FCFDAStagingLiteralText LiteralDisplayName;
	// Literal projection 진단은 preserve fallback이 성공하면 외부 blocking issue로 노출하지 않습니다.
	TArray<FCFDAStagingIssue> DisplayNameIssues;
	if (CFDACommonPrimitives::ReadLiteralTextFromAsset(Asset.DisplayName, TEXT("DisplayName"), LiteralDisplayName, DisplayNameIssues))
	{
		OutSnapshot.DisplayNameSource = LiteralDisplayName.Text;
	}
	else
	{
		OutSnapshot.bPreserveExistingDisplayName = true;
		OutSnapshot.DisplayNameSource = GetPreservedDisplayNameToken();
	}

	OutSnapshot.RequiredMountType = Asset.RequiredMountType;
	OutSnapshot.RequiredWeaponSize = Asset.RequiredWeaponSize;
	OutSnapshot.DefaultTurretMountDataPath = Asset.DefaultTurretMountData != nullptr ? FSoftObjectPath(Asset.DefaultTurretMountData) : FSoftObjectPath();
	OutSnapshot.DefaultWeaponDataPath = Asset.DefaultWeaponData != nullptr ? FSoftObjectPath(Asset.DefaultWeaponData) : FSoftObjectPath();
	OutSnapshot.DefaultSensorDataPath = Asset.DefaultSensorData != nullptr ? FSoftObjectPath(Asset.DefaultSensorData) : FSoftObjectPath();

	// Semantic fingerprint build failure입니다.
	FString FingerprintError;
	if (!BuildSemanticFingerprint(OutSnapshot, OutSnapshot.SemanticFingerprint, FingerprintError))
	{
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("$semantic"), FingerprintError, true);
		return false;
	}
	return true;
}

// exact7 semantic snapshot을 deterministic SHA-256 fingerprint로 변환합니다.
bool FCFEquipmentDurable::BuildSemanticFingerprint(
	const FCFEquipmentSemanticSnapshot& Snapshot,
	FString& OutFingerprint,
	FString& OutError)
{
	OutFingerprint.Reset();
	OutError.Reset();

	// Stable mount enum token입니다.
	const FString MountTypeToken = CFEquipmentDurablePrivate::BuildMountTypeToken(Snapshot.RequiredMountType);
	// Stable size enum token입니다.
	const FString WeaponSizeToken = CFEquipmentDurablePrivate::BuildWeaponSizeToken(Snapshot.RequiredWeaponSize);
	if (MountTypeToken.IsEmpty() || WeaponSizeToken.IsEmpty())
	{
		OutError = TEXT("EquipmentPreset enum semantic token을 canonicalize하지 못했습니다.");
		return false;
	}
	if (Snapshot.bPreserveExistingDisplayName && Snapshot.DisplayNameSource != GetPreservedDisplayNameToken())
	{
		OutError = TEXT("표시 이름 preserve semantic token이 current contract와 일치하지 않습니다.");
		return false;
	}

	// Contract Revision 2 exact7 semantic byte stream과 DisplayName ownership mode입니다.
	TArray<uint8> Bytes;
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("EquipmentId"), CFDACommonPrimitives::CanonicalNameText(Snapshot.EquipmentId));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DisplayNameMode"), Snapshot.bPreserveExistingDisplayName ? TEXT("PreserveExisting") : TEXT("EditableLiteral"));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DisplayName"), Snapshot.DisplayNameSource);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("RequiredMountType"), MountTypeToken);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("RequiredWeaponSize"), WeaponSizeToken);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DefaultTurretMountData"), Snapshot.DefaultTurretMountDataPath.IsNull() ? FString() : Snapshot.DefaultTurretMountDataPath.ToString());
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DefaultWeaponData"), Snapshot.DefaultWeaponDataPath.IsNull() ? FString() : Snapshot.DefaultWeaponDataPath.ToString());
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DefaultSensorData"), Snapshot.DefaultSensorDataPath.IsNull() ? FString() : Snapshot.DefaultSensorDataPath.ToString());
	return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
}

// desired exact7 snapshot을 exact EquipmentPreset UObject에 materialize합니다.
void FCFEquipmentDurable::MaterializeSnapshot(
	UCFEquipmentPresetData& Asset,
	const FCFEquipmentSemanticSnapshot& Snapshot)
{
	Asset.EquipmentId = Snapshot.EquipmentId;
	if (!Snapshot.bPreserveExistingDisplayName)
	{
		Asset.DisplayName = FText::FromString(Snapshot.DisplayNameSource);
	}
	Asset.RequiredMountType = Snapshot.RequiredMountType;
	Asset.RequiredWeaponSize = Snapshot.RequiredWeaponSize;
	Asset.DefaultTurretMountData = Snapshot.DefaultTurretMountDataPath.IsNull() ? nullptr : LoadObject<UCFTurretMountData>(nullptr, *Snapshot.DefaultTurretMountDataPath.ToString());
	Asset.DefaultWeaponData = Snapshot.DefaultWeaponDataPath.IsNull() ? nullptr : LoadObject<UCFWeaponData>(nullptr, *Snapshot.DefaultWeaponDataPath.ToString());
	Asset.DefaultSensorData = Snapshot.DefaultSensorDataPath.IsNull() ? nullptr : LoadObject<UCFVehicleSensorData>(nullptr, *Snapshot.DefaultSensorDataPath.ToString());
}

// Review/Commit에 사용되는 child dependency가 persisted exact-class clean truth인지 fail-closed 검증합니다.
bool FCFEquipmentDurable::ValidatePersistedChildDependencies(
	const FCFEquipmentPresetDraft& Draft,
	const FString& ForcedUnconfirmedObjectPath,
	FString& OutError)
{
	OutError.Reset();
	if (Draft.DraftMode == ECFEquipmentDraftMode::Scanner)
	{
		// Scanner principal child입니다.
		UCFVehicleSensorData* SensorData = Draft.DefaultSensorData.IsNull() ? nullptr : LoadObject<UCFVehicleSensorData>(nullptr, *Draft.DefaultSensorData.ToSoftObjectPath().ToString());
		return CFEquipmentDurablePrivate::ValidateDependency(SensorData, TEXT("DefaultSensorData"), ForcedUnconfirmedObjectPath, OutError);
	}

	// Weapon principal TurretMountData child입니다.
	UCFTurretMountData* TurretMountData = Draft.DefaultTurretMountData.IsNull() ? nullptr : LoadObject<UCFTurretMountData>(nullptr, *Draft.DefaultTurretMountData.ToSoftObjectPath().ToString());
	if (!CFEquipmentDurablePrivate::ValidateDependency(TurretMountData, TEXT("DefaultTurretMountData"), ForcedUnconfirmedObjectPath, OutError))
	{
		return false;
	}

	// Weapon principal WeaponData child입니다.
	UCFWeaponData* WeaponData = Draft.DefaultWeaponData.IsNull() ? nullptr : LoadObject<UCFWeaponData>(nullptr, *Draft.DefaultWeaponData.ToSoftObjectPath().ToString());
	if (!CFEquipmentDurablePrivate::ValidateDependency(WeaponData, TEXT("DefaultWeaponData"), ForcedUnconfirmedObjectPath, OutError))
	{
		return false;
	}

	if (WeaponData->DefaultAmmoData != nullptr
		&& !CFEquipmentDurablePrivate::ValidateDependency(WeaponData->DefaultAmmoData.Get(), TEXT("DefaultWeaponData.DefaultAmmoData"), ForcedUnconfirmedObjectPath, OutError))
	{
		return false;
	}
	if (WeaponData->DefaultProjectileData != nullptr)
	{
		if (!CFEquipmentDurablePrivate::ValidateDependency(WeaponData->DefaultProjectileData.Get(), TEXT("DefaultWeaponData.DefaultProjectileData"), ForcedUnconfirmedObjectPath, OutError))
		{
			return false;
		}
		if (WeaponData->DefaultProjectileData->DefaultDamageData != nullptr
			&& !CFEquipmentDurablePrivate::ValidateDependency(WeaponData->DefaultProjectileData->DefaultDamageData.Get(), TEXT("DefaultWeaponData.DefaultProjectileData.DefaultDamageData"), ForcedUnconfirmedObjectPath, OutError))
		{
			return false;
		}
	}
	return true;
}

// current contract ReviewProposalDigest를 deterministic SHA-256으로 계산합니다.
bool FCFEquipmentDurable::BuildReviewProposalDigest(
	ECFEquipmentReviewOperation Operation,
	const FString& TargetObjectPath,
	const FString& TargetClassPath,
	const FString& CurrentSemanticFingerprint,
	const FString& ProspectiveSemanticFingerprint,
	int32 ContractRevision,
	const FString& ValidationState,
	FString& OutDigest,
	FString& OutError)
{
	OutDigest.Reset();
	OutError.Reset();

	// Stable operation token입니다.
	FString OperationToken;
	if (Operation == ECFEquipmentReviewOperation::Create)
	{
		OperationToken = TEXT("Create");
	}
	else if (Operation == ECFEquipmentReviewOperation::Update)
	{
		OperationToken = TEXT("Update");
	}
	else
	{
		OutError = TEXT("ReviewProposalDigest operation이 Create/Update가 아닙니다.");
		return false;
	}

	if (TargetClassPath != GetTargetClassPath()
		|| ContractRevision != GetContractRevision()
		|| ValidationState != GetReadyValidationToken()
		|| CurrentSemanticFingerprint.IsEmpty()
		|| ProspectiveSemanticFingerprint.IsEmpty())
	{
		OutError = TEXT("ReviewProposalDigest canonical binding 값이 current Equipment Authoring 계약과 일치하지 않습니다.");
		return false;
	}

	// Current contract exact7 approval byte stream입니다.
	TArray<uint8> Bytes;
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("OperationKind"), OperationToken);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("TargetObjectPath"), TargetObjectPath);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("TargetClassPath"), TargetClassPath);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("CurrentSemanticFingerprint"), CurrentSemanticFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ProspectiveSemanticFingerprint"), ProspectiveSemanticFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("EquipmentAuthoringContractRevision"), LexToString(ContractRevision));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ValidationState"), ValidationState);
	return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutDigest, OutError);
}
