// Copyright (c) CarFight. All Rights Reserved.
// File: CFProdTargetBindings.cpp
// Version: v1.0.0
// Date: 2026-10-06
// Description: CCAS Production target를 existing Weapon Guide / Equipment durable backend에 결속하는 concrete adapter 구현입니다.
// Changelog:
// - v1.0.0: exact binding registry, backend desired-fingerprint admission, persisted read/apply callbacks를 구현.
// Migration:
// - 새 package writer 없음. Weapon child는 Weapon Guide backend, EquipmentPreset은 Equipment durable core를 재사용합니다.

#include "DataAuthoring/CFProdTargetBindings.h"

#include "CFEquipmentPresetData.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFDADurableCore.h"
#include "EquipmentAuthoring/CFEquipmentDurable.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"
#include "WeaponAuthoring/CFWeaponGuideBackend.h"

namespace CFProdTargetBindingsPrivate
{
	// Exact binding stable key를 반환합니다.
	FString MakeBindingKey(const FCFProdProvisionTarget& Target)
	{
		return Target.ContentKey.ToStableString();
	}

	// Binding request가 기본 Production target contract를 만족하는지 검증합니다.
	bool ValidateBindingTarget(
		const FCFProdProvisionTarget& Target,
		FString& OutError)
	{
		OutError.Reset();
		if (!Target.ContentKey.IsValid()
			|| Target.TargetObjectPath.IsEmpty()
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Target.DesiredFingerprint))
		{
			OutError = TEXT("Production target binding의 ContentKey/ObjectPath/DesiredFingerprint가 유효하지 않습니다.");
			return false;
		}

		// Canonical Unreal package path입니다.
		const FString PackageName =
			FPackageName::ObjectPathToPackageName(
				Target.TargetObjectPath);
		if (!FPackageName::IsValidLongPackageName(PackageName)
			|| !Target.TargetObjectPath.StartsWith(
				TEXT("/Game/CarFight/"),
				ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Production target binding ObjectPath가 canonical CarFight package 경로가 아닙니다.");
			return false;
		}
		return true;
	}

	// Backend가 계산한 desired fingerprint와 reviewed target fingerprint를 exact 결속합니다.
	bool ValidateDesiredFingerprint(
		const FCFProdProvisionTarget& Target,
		const FString& BackendDesiredFingerprint,
		FString& OutError)
	{
		if (!BackendDesiredFingerprint.Equals(
			Target.DesiredFingerprint,
			ESearchCase::CaseSensitive))
		{
			OutError = FString::Printf(
				TEXT("Production target %s 의 reviewed DesiredFingerprint와 existing typed backend fingerprint가 다릅니다."),
				*Target.ContentKey.ToStableString());
			return false;
		}
		return true;
	}

	// Persisted EquipmentPreset exact semantic fingerprint를 existing Equipment durable extractor로 읽습니다.
	bool ReadEquipmentFingerprint(
		const FString& TargetObjectPath,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError)
	{
		bOutExists = false;
		OutFingerprint.Reset();
		OutError.Reset();

		// Exact target soft object path입니다.
		const FSoftObjectPath ObjectPath(TargetObjectPath);
		// Exact target package name입니다.
		const FString PackageName =
			FPackageName::ObjectPathToPackageName(TargetObjectPath);
		if (!ObjectPath.IsValid()
			|| !FPackageName::IsValidLongPackageName(PackageName))
		{
			OutError = TEXT("EquipmentPreset Production target path가 canonical하지 않습니다.");
			return false;
		}

		// 현재 disk 또는 loaded memory target 존재 여부입니다.
		const bool bPackageExists =
			FPackageName::DoesPackageExist(PackageName);
		// Already loaded target object입니다.
		UObject* ResolvedObject = ObjectPath.ResolveObject();
		if (!bPackageExists && ResolvedObject == nullptr)
		{
			return true;
		}

		// Persisted/readable exact EquipmentPreset입니다.
		UCFEquipmentPresetData* EquipmentPreset =
			Cast<UCFEquipmentPresetData>(
				ResolvedObject != nullptr
					? ResolvedObject
					: LoadObject<UCFEquipmentPresetData>(
						nullptr,
						*TargetObjectPath));
		if (EquipmentPreset == nullptr
			|| EquipmentPreset->GetClass()
				!= UCFEquipmentPresetData::StaticClass())
		{
			OutError = TEXT("EquipmentPreset Production target이 존재하지만 exact class로 읽히지 않습니다.");
			return false;
		}

		// Persisted target package입니다.
		UPackage* Package = EquipmentPreset->GetOutermost();
		if (Package == nullptr
			|| Package == GetTransientPackage()
			|| Package->IsDirty())
		{
			OutError = TEXT("EquipmentPreset Production target package가 dirty/unconfirmed 상태입니다.");
			return false;
		}

		// Existing Equipment durable semantic snapshot입니다.
		FCFEquipmentSemanticSnapshot Snapshot;
		// Existing Equipment durable extraction diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!FCFEquipmentDurable::ExtractSnapshotFromAsset(
			*EquipmentPreset,
			Snapshot,
			ExtractIssues))
		{
			OutError = ExtractIssues.IsEmpty()
				? TEXT("EquipmentPreset Production persisted snapshot extraction이 실패했습니다.")
				: ExtractIssues[0].Message;
			return false;
		}

		OutFingerprint = Snapshot.SemanticFingerprint;
		if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			OutFingerprint))
		{
			OutError = TEXT("EquipmentPreset Production persisted fingerprint가 canonical하지 않습니다.");
			return false;
		}

		bOutExists = true;
		return true;
	}

	// Existing Equipment durable core로 EquipmentPreset Create/Update를 적용하고 readback합니다.
	bool ApplyEquipment(
		const FCFEquipmentPresetDraft& Draft,
		const FCFProdProvisionTarget& Target,
		FString& OutPersistedFingerprint,
		FString& OutError)
	{
		OutPersistedFingerprint.Reset();
		OutError.Reset();

		// Existing Equipment backend가 생성한 desired semantic snapshot입니다.
		FCFEquipmentSemanticSnapshot DesiredSnapshot;
		if (!FCFEquipmentDurable::BuildSnapshotFromDraft(
			Draft,
			DesiredSnapshot,
			OutError)
			|| !DesiredSnapshot.SemanticFingerprint.Equals(
				Target.DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("EquipmentPreset reviewed DesiredFingerprint와 Equipment durable backend fingerprint가 다릅니다.");
			}
			return false;
		}

		// Fresh persisted target existence입니다.
		bool bExists = false;
		// Fresh persisted target fingerprint입니다.
		FString CurrentFingerprint;
		if (!ReadEquipmentFingerprint(
			Target.TargetObjectPath,
			bExists,
			CurrentFingerprint,
			OutError))
		{
			return false;
		}

		// Existing Equipment durable core common row입니다.
		FCFDACommonPreviewRow Row;
		Row.Kind = bExists
			? ECFDAStagingPreviewKind::Update
			: ECFDAStagingPreviewKind::Create;
		Row.Envelope.SchemaId =
			TEXT("CarFight.EquipmentAuthoring.EquipmentPreset");
		Row.Envelope.SchemaRevision = 1;
		Row.Envelope.AdapterContractRevision =
			FCFEquipmentDurable::GetContractRevision();
		Row.Envelope.DataAssetTypeClassPath =
			FCFEquipmentDurable::GetTargetClassPath();
		Row.Envelope.StableLogicalId =
			DesiredSnapshot.EquipmentId;
		Row.Envelope.TargetObjectPath =
			Target.TargetObjectPath;
		Row.Envelope.StagingRelativePath =
			TEXT("CCASProduction://EquipmentPreset");
		Row.Envelope.bHasBaseSemanticFingerprint = bExists;
		Row.Envelope.BaseSemanticFingerprint =
			bExists ? CurrentFingerprint : FString();
		Row.Envelope.CurrentSemanticFingerprint =
			bExists ? CurrentFingerprint : FString();
		Row.Envelope.StagingSemanticFingerprint =
			DesiredSnapshot.SemanticFingerprint;
		Row.Envelope.PlannedOperation = Row.Kind;

		// Existing Equipment durable core terminal report입니다.
		FCFDAStagingTargetApplyReport ApplyReport;
		CFDADurableCore::ApplyTypedTarget<
			UCFEquipmentPresetData,
			FCFEquipmentSemanticSnapshot>(
				Row,
				DesiredSnapshot,
				&FCFEquipmentDurable::MaterializeSnapshot,
				&FCFEquipmentDurable::ExtractSnapshotFromAsset,
				&FCFEquipmentDurable::BuildSemanticFingerprint,
				ApplyReport);
		if (ApplyReport.Result
			!= ECFDAStagingTargetApplyResult::DurableApplied)
		{
			OutError = ApplyReport.Diagnostic.IsEmpty()
				? TEXT("EquipmentPreset existing durable backend가 DurableApplied에 도달하지 못했습니다.")
				: ApplyReport.Diagnostic;
			return false;
		}

		// Durable write 직후 exact persisted target existence입니다.
		bool bReadbackExists = false;
		if (!ReadEquipmentFingerprint(
			Target.TargetObjectPath,
			bReadbackExists,
			OutPersistedFingerprint,
			OutError)
			|| !bReadbackExists
			|| !OutPersistedFingerprint.Equals(
				Target.DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("EquipmentPreset post-write readback fingerprint가 reviewed target과 다릅니다.");
			}
			return false;
		}
		return true;
	}
}

// Exact target binding 하나를 중복 없이 등록합니다.
bool FCFProdBoundTargetAdapter::RegisterBinding(
	const FCFProdTargetBinding& Binding,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Binding.Target,
		OutError))
	{
		return false;
	}
	if (!Binding.ReadPersistedFingerprint
		|| !Binding.ApplyReviewedTarget)
	{
		OutError = TEXT("Production target binding callback이 완전하지 않습니다.");
		return false;
	}

	// Exact ContentKey stable binding identity입니다.
	const FString BindingKey =
		CFProdTargetBindingsPrivate::MakeBindingKey(
			Binding.Target);
	if (BindingsByStableKey.Contains(BindingKey))
	{
		OutError = FString::Printf(
			TEXT("Production target binding이 중복 등록되었습니다: %s"),
			*BindingKey);
		return false;
	}

	BindingsByStableKey.Add(BindingKey, Binding);
	OutError.Reset();
	return true;
}

// 등록된 exact target contract와 결속된 existing backend에서 current persisted fingerprint를 읽습니다.
bool FCFProdBoundTargetAdapter::ReadPersistedFingerprint(
	const FCFProdProvisionTarget& Target,
	bool& bOutExists,
	FString& OutFingerprint,
	FString& OutError) const
{
	// Requested ContentKey stable binding identity입니다.
	const FString BindingKey =
		CFProdTargetBindingsPrivate::MakeBindingKey(Target);
	// Exact registered binding입니다.
	const FCFProdTargetBinding* Binding =
		BindingsByStableKey.Find(BindingKey);
	if (Binding == nullptr
		|| !(Binding->Target.ContentKey == Target.ContentKey)
		|| Binding->Target.TargetObjectPath
			!= Target.TargetObjectPath
		|| Binding->Target.DesiredFingerprint
			!= Target.DesiredFingerprint)
	{
		OutError = FString::Printf(
			TEXT("Production target %s 의 exact typed backend binding이 없거나 reviewed target contract와 다릅니다."),
			*BindingKey);
		return false;
	}
	return Binding->ReadPersistedFingerprint(
		bOutExists,
		OutFingerprint,
		OutError);
}

// Exact target contract와 결속된 existing backend에 reviewed durable apply를 위임합니다.
bool FCFProdBoundTargetAdapter::ApplyReviewedTarget(
	const FCFProdProvisionTarget& Target,
	FString& OutPersistedFingerprint,
	FString& OutError)
{
	// Requested ContentKey stable binding identity입니다.
	const FString BindingKey =
		CFProdTargetBindingsPrivate::MakeBindingKey(Target);
	// Exact registered binding입니다.
	FCFProdTargetBinding* Binding =
		BindingsByStableKey.Find(BindingKey);
	if (Binding == nullptr
		|| !(Binding->Target.ContentKey == Target.ContentKey)
		|| Binding->Target.TargetObjectPath
			!= Target.TargetObjectPath
		|| Binding->Target.DesiredFingerprint
			!= Target.DesiredFingerprint)
	{
		OutError = FString::Printf(
			TEXT("Production target %s 의 exact typed backend binding이 없거나 reviewed target contract와 다릅니다."),
			*BindingKey);
		return false;
	}
	return Binding->ApplyReviewedTarget(
		OutPersistedFingerprint,
		OutError);
}

// TurretMount Production target을 existing Weapon Guide durable backend에 결속합니다.
bool FCFProdTargetBindings::BindTurret(
	FCFProdBoundTargetAdapter& Adapter,
	const FCFProdProvisionTarget& Target,
	const FCFWeaponGuideDraft& Draft,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Target,
		OutError))
	{
		return false;
	}

	// Target ContentId와 동일한 logical TurretMount identity입니다.
	const FName TurretMountId(*Target.ContentKey.ContentId);
	// Existing Weapon Guide backend desired fingerprint입니다.
	FString BackendDesiredFingerprint;
	if (!CFWeaponGuideBackend::BuildTurretDesiredFingerprint(
		Draft,
		TurretMountId,
		BackendDesiredFingerprint,
		OutError)
		|| !CFProdTargetBindingsPrivate::ValidateDesiredFingerprint(
			Target,
			BackendDesiredFingerprint,
			OutError))
	{
		return false;
	}

	// Existing backend에 결속할 immutable target contract입니다.
	FCFProdTargetBinding Binding;
	Binding.Target = Target;
	Binding.ReadPersistedFingerprint =
		[Target](
			bool& bOutExists,
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ReadTurretFingerprint(
				Target.TargetObjectPath,
				bOutExists,
				OutFingerprint,
				CallbackError);
		};
	Binding.ApplyReviewedTarget =
		[Target, Draft, TurretMountId](
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ApplyTurret(
				Draft,
				TurretMountId,
				Target.TargetObjectPath,
				Target.DesiredFingerprint,
				OutFingerprint,
				CallbackError);
		};
	return Adapter.RegisterBinding(Binding, OutError);
}

// Damage Production target을 existing Damage reviewed provider에 결속합니다.
bool FCFProdTargetBindings::BindDamage(
	FCFProdBoundTargetAdapter& Adapter,
	const FCFProdProvisionTarget& Target,
	const FCFWeaponGuideDraft& Draft,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Target,
		OutError))
	{
		return false;
	}

	// Target ContentId와 동일한 logical Damage identity입니다.
	const FName DamageId(*Target.ContentKey.ContentId);
	// Existing Damage provider desired fingerprint입니다.
	FString BackendDesiredFingerprint;
	if (!CFWeaponGuideBackend::BuildDamageDesiredFingerprint(
		Draft,
		DamageId,
		BackendDesiredFingerprint,
		OutError)
		|| !CFProdTargetBindingsPrivate::ValidateDesiredFingerprint(
			Target,
			BackendDesiredFingerprint,
			OutError))
	{
		return false;
	}

	// Existing backend에 결속할 immutable target contract입니다.
	FCFProdTargetBinding Binding;
	Binding.Target = Target;
	Binding.ReadPersistedFingerprint =
		[Target](
			bool& bOutExists,
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ReadDamageFingerprint(
				Target.TargetObjectPath,
				bOutExists,
				OutFingerprint,
				CallbackError);
		};
	Binding.ApplyReviewedTarget =
		[Target, Draft, DamageId](
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ApplyDamage(
				Draft,
				DamageId,
				Target.TargetObjectPath,
				Target.DesiredFingerprint,
				OutFingerprint,
				CallbackError);
		};
	return Adapter.RegisterBinding(Binding, OutError);
}

// Ammo Production target을 existing Ammo reviewed provider에 결속합니다.
bool FCFProdTargetBindings::BindAmmo(
	FCFProdBoundTargetAdapter& Adapter,
	const FCFProdProvisionTarget& Target,
	const FCFWeaponGuideDraft& Draft,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Target,
		OutError))
	{
		return false;
	}

	// Target ContentId와 동일한 logical Ammo identity입니다.
	const FName AmmoId(*Target.ContentKey.ContentId);
	// Existing Ammo provider desired fingerprint입니다.
	FString BackendDesiredFingerprint;
	if (!CFWeaponGuideBackend::BuildAmmoDesiredFingerprint(
		Draft,
		AmmoId,
		BackendDesiredFingerprint,
		OutError)
		|| !CFProdTargetBindingsPrivate::ValidateDesiredFingerprint(
			Target,
			BackendDesiredFingerprint,
			OutError))
	{
		return false;
	}

	// Existing backend에 결속할 immutable target contract입니다.
	FCFProdTargetBinding Binding;
	Binding.Target = Target;
	Binding.ReadPersistedFingerprint =
		[Target](
			bool& bOutExists,
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ReadAmmoFingerprint(
				Target.TargetObjectPath,
				bOutExists,
				OutFingerprint,
				CallbackError);
		};
	Binding.ApplyReviewedTarget =
		[Target, Draft, AmmoId](
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ApplyAmmo(
				Draft,
				AmmoId,
				Target.TargetObjectPath,
				Target.DesiredFingerprint,
				OutFingerprint,
				CallbackError);
		};
	return Adapter.RegisterBinding(Binding, OutError);
}

// Projectile Production target을 existing Weapon Guide durable backend에 결속합니다.
bool FCFProdTargetBindings::BindProjectile(
	FCFProdBoundTargetAdapter& Adapter,
	const FCFProdProvisionTarget& Target,
	const FCFWeaponGuideDraft& Draft,
	const FString& DamageObjectPath,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Target,
		OutError))
	{
		return false;
	}

	// Target ContentId와 동일한 logical Projectile identity입니다.
	const FName ProjectileId(*Target.ContentKey.ContentId);
	// Resolved persisted Damage reference입니다.
	const FSoftObjectPath DamagePath(DamageObjectPath);
	// Existing Projectile backend desired fingerprint입니다.
	FString BackendDesiredFingerprint;
	if (!CFWeaponGuideBackend::BuildProjectileDesiredFingerprint(
		Draft,
		ProjectileId,
		DamagePath,
		BackendDesiredFingerprint,
		OutError)
		|| !CFProdTargetBindingsPrivate::ValidateDesiredFingerprint(
			Target,
			BackendDesiredFingerprint,
			OutError))
	{
		return false;
	}

	// Existing backend에 결속할 immutable target contract입니다.
	FCFProdTargetBinding Binding;
	Binding.Target = Target;
	Binding.ReadPersistedFingerprint =
		[Target](
			bool& bOutExists,
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ReadProjectileFingerprint(
				Target.TargetObjectPath,
				bOutExists,
				OutFingerprint,
				CallbackError);
		};
	Binding.ApplyReviewedTarget =
		[Target, Draft, ProjectileId, DamagePath](
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ApplyProjectile(
				Draft,
				ProjectileId,
				DamagePath,
				Target.TargetObjectPath,
				Target.DesiredFingerprint,
				OutFingerprint,
				CallbackError);
		};
	return Adapter.RegisterBinding(Binding, OutError);
}

// Weapon Production target을 existing Weapon Guide durable backend에 결속합니다.
bool FCFProdTargetBindings::BindWeapon(
	FCFProdBoundTargetAdapter& Adapter,
	const FCFProdProvisionTarget& Target,
	const FCFWeaponGuideDraft& Draft,
	const FString& ProjectileObjectPath,
	const FString& AmmoObjectPath,
	const FName ResolvedAmmoTypeId,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Target,
		OutError))
	{
		return false;
	}

	// Target ContentId와 동일한 logical Weapon identity입니다.
	const FName WeaponId(*Target.ContentKey.ContentId);
	// Resolved persisted Projectile reference입니다.
	const FSoftObjectPath ProjectilePath(ProjectileObjectPath);
	// Resolved persisted Ammo reference입니다.
	const FSoftObjectPath AmmoPath(AmmoObjectPath);
	// Existing Weapon backend desired fingerprint입니다.
	FString BackendDesiredFingerprint;
	if (!CFWeaponGuideBackend::BuildWeaponDesiredFingerprint(
		Draft,
		WeaponId,
		ProjectilePath,
		AmmoPath,
		ResolvedAmmoTypeId,
		BackendDesiredFingerprint,
		OutError)
		|| !CFProdTargetBindingsPrivate::ValidateDesiredFingerprint(
			Target,
			BackendDesiredFingerprint,
			OutError))
	{
		return false;
	}

	// Existing backend에 결속할 immutable target contract입니다.
	FCFProdTargetBinding Binding;
	Binding.Target = Target;
	Binding.ReadPersistedFingerprint =
		[Target](
			bool& bOutExists,
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ReadWeaponFingerprint(
				Target.TargetObjectPath,
				bOutExists,
				OutFingerprint,
				CallbackError);
		};
	Binding.ApplyReviewedTarget =
		[Target, Draft, WeaponId, ProjectilePath, AmmoPath, ResolvedAmmoTypeId](
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFWeaponGuideBackend::ApplyWeapon(
				Draft,
				WeaponId,
				ProjectilePath,
				AmmoPath,
				ResolvedAmmoTypeId,
				Target.TargetObjectPath,
				Target.DesiredFingerprint,
				OutFingerprint,
				CallbackError);
		};
	return Adapter.RegisterBinding(Binding, OutError);
}

// EquipmentPreset Production target을 existing Equipment durable path에 결속합니다.
bool FCFProdTargetBindings::BindEquipmentPreset(
	FCFProdBoundTargetAdapter& Adapter,
	const FCFProdProvisionTarget& Target,
	const FCFEquipmentPresetDraft& Draft,
	FString& OutError)
{
	if (!CFProdTargetBindingsPrivate::ValidateBindingTarget(
		Target,
		OutError))
	{
		return false;
	}

	// Existing Equipment durable backend desired snapshot입니다.
	FCFEquipmentSemanticSnapshot DesiredSnapshot;
	if (!FCFEquipmentDurable::BuildSnapshotFromDraft(
		Draft,
		DesiredSnapshot,
		OutError)
		|| !CFProdTargetBindingsPrivate::ValidateDesiredFingerprint(
			Target,
			DesiredSnapshot.SemanticFingerprint,
			OutError))
	{
		return false;
	}

	// Existing backend에 결속할 immutable target contract입니다.
	FCFProdTargetBinding Binding;
	Binding.Target = Target;
	Binding.ReadPersistedFingerprint =
		[Target](
			bool& bOutExists,
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFProdTargetBindingsPrivate::ReadEquipmentFingerprint(
				Target.TargetObjectPath,
				bOutExists,
				OutFingerprint,
				CallbackError);
		};
	Binding.ApplyReviewedTarget =
		[Target, Draft](
			FString& OutFingerprint,
			FString& CallbackError)
		{
			return CFProdTargetBindingsPrivate::ApplyEquipment(
				Draft,
				Target,
				OutFingerprint,
				CallbackError);
		};
	return Adapter.RegisterBinding(Binding, OutError);
}
