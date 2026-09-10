// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADurableCore.h
// Version: v1.0.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-03 provider-neutral typed durable Create/Update transaction core입니다.
// Changelog:
// - v1.0.0: exact Create/Update, pre-save typed readback, single-package SavePackage, disk reload, rollback/uncertainty taxonomy를 typed provider 공용 template로 분리.
// Migration:
// - typed payload는 provider stack에서 template 인자로만 전달되며 shared runtime DTO/approval에는 저장하지 않습니다.
// - provider는 exact typed Materialize/Extract/Fingerprint callback만 제공하고 durable sequencing은 이 core를 재사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingApply.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace CFDADurableCore
{
	// Create target의 pre-save failure에서 operation-created UObject/registry/dirty state를 best-effort로 정리합니다.
	bool CleanupCreatedAsset(
		UObject* Asset,
		UPackage* Package,
		bool bRegistryNotified);

	// 현재 exact target이 test-only forced SavePackage uncertainty 대상인지 확인합니다.
	bool ShouldForceSaveFailure(const FString& TargetObjectPath);

	// 현재 exact target이 test-only forced post-save confirmation uncertainty 대상인지 확인합니다.
	bool ShouldForceConfirmationFailure(const FString& TargetObjectPath);

#if WITH_DEV_AUTOMATION_TESTS
	// 모든 shared durable test fault state를 초기화합니다.
	void ResetTestFaults();

	// exact target의 SavePackage 호출 지점에서 outcome-unknown failure를 강제합니다.
	void SetForceSaveFailureTarget(const FString& TargetObjectPath);

	// exact target의 SavePackage 성공 뒤 persisted confirmation uncertainty를 강제합니다.
	void SetForceConfirmationFailureTarget(const FString& TargetObjectPath);
#endif

	// Exact typed UObject를 production extractor/fingerprint 기준으로 expected semantic fingerprint와 대조합니다.
	template <typename TAsset, typename TPayload>
	bool ValidateTypedAssetFingerprint(
		const TAsset& Asset,
		const FString& ExpectedFingerprint,
		bool (*ExtractPayload)(const TAsset&, TPayload&, TArray<FCFDAStagingIssue>&),
		bool (*BuildFingerprint)(const TPayload&, FString&, FString&),
		FString& OutError)
	{
		// Current exact typed UObject에서 추출한 provider-local payload입니다.
		TPayload CurrentPayload;
		// Production typed extractor가 반환한 diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractionIssues;
		if (!ExtractPayload(Asset, CurrentPayload, ExtractionIssues))
		{
			OutError = ExtractionIssues.IsEmpty()
				? TEXT("typed semantic readback payload 추출에 실패했습니다.")
				: ExtractionIssues[0].Message;
			return false;
		}

		// Current typed payload에서 계산한 semantic fingerprint입니다.
		FString CurrentFingerprint;
		// Production typed fingerprint 생성 실패 상세입니다.
		FString FingerprintError;
		if (!BuildFingerprint(CurrentPayload, CurrentFingerprint, FingerprintError))
		{
			OutError = FString::Printf(TEXT("typed semantic readback fingerprint 생성에 실패했습니다: %s"), *FingerprintError);
			return false;
		}
		if (!CurrentFingerprint.Equals(ExpectedFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("typed semantic readback이 reviewed StagingSemanticFingerprint와 다릅니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Exact target package 하나만 저장하고 disk reload + typed semantic readback까지 durable confirmation합니다.
	template <typename TAsset, typename TPayload>
	bool SaveExactPackage(
		UPackage& Package,
		TAsset& Asset,
		const FString& ExpectedFingerprint,
		bool (*ExtractPayload)(const TAsset&, TPayload&, TArray<FCFDAStagingIssue>&),
		bool (*BuildFingerprint)(const TPayload&, FString&, FString&),
		bool& bOutSaveWasCalled,
		FString& OutError)
	{
		bOutSaveWasCalled = false;

		// Reload 뒤 old UObject pointer를 사용하지 않기 위해 저장 전에 동결하는 exact package long name입니다.
		const FString PackageName = Package.GetName();
		// Reload 뒤 exact persisted typed object를 다시 찾기 위한 canonical object path입니다.
		const FString TargetObjectPath = FSoftObjectPath(&Asset).ToString();
		if (Asset.GetOutermost() != &Package
			|| &Package == GetTransientPackage()
			|| !PackageName.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
			|| !FPackageName::IsValidLongPackageName(PackageName)
			|| TargetObjectPath.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Exact Staging save package/object identity가 유효하지 않습니다: %s"), *PackageName);
			return false;
		}

		// Package long name에서 계산한 exact .uasset filename입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackageName,
			FPackageName::GetAssetPackageExtension());
		// 신규 package가 저장될 exact output directory입니다.
		const FString PackageDirectory = FPaths::GetPath(PackageFilename);
		IFileManager::Get().MakeDirectory(*PackageDirectory, true);

		// UE 5.8 exact single-package save arguments입니다.
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		bOutSaveWasCalled = true;
		if (ShouldForceSaveFailure(TargetObjectPath))
		{
			OutError = TEXT("Automation fixture가 SavePackage outcome uncertainty를 강제했습니다.");
			return false;
		}
		if (!UPackage::SavePackage(&Package, &Asset, *PackageFilename, SaveArgs))
		{
			OutError = FString::Printf(TEXT("UPackage::SavePackage가 false를 반환했습니다: %s"), *PackageName);
			return false;
		}
		if (Package.IsDirty() || !FPackageName::DoesPackageExist(PackageName))
		{
			OutError = FString::Printf(TEXT("SavePackage success 뒤 package clean/persisted 존재를 확인하지 못했습니다: %s"), *PackageName);
			return false;
		}
		if (ShouldForceConfirmationFailure(TargetObjectPath))
		{
			OutError = TEXT("Automation fixture가 SavePackage 성공 뒤 persisted confirmation uncertainty를 강제했습니다.");
			return false;
		}

		// 방금 저장한 exact package 하나만 disk state로 reload할 목록입니다.
		TArray<UPackage*> PackagesToReload;
		PackagesToReload.Add(&Package);
		// Exact package reload 실패 상세입니다.
		FText ReloadError;
		if (!UPackageTools::ReloadPackages(PackagesToReload, ReloadError, EReloadPackagesInteractionMode::AssumePositive))
		{
			OutError = FString::Printf(
				TEXT("SavePackage success 뒤 exact persisted package reload를 확인하지 못했습니다: %s / %s"),
				*PackageName,
				*ReloadError.ToString());
			return false;
		}

		// Disk reload 뒤 exact object path에서 다시 resolve한 persisted typed DataAsset입니다.
		TAsset* ReloadedAsset = Cast<TAsset>(FSoftObjectPath(TargetObjectPath).ResolveObject());
		if (ReloadedAsset == nullptr)
		{
			ReloadedAsset = LoadObject<TAsset>(nullptr, *TargetObjectPath);
		}
		// Reloaded typed object가 소유하는 exact package입니다.
		UPackage* ReloadedPackage = ReloadedAsset != nullptr ? ReloadedAsset->GetOutermost() : nullptr;
		if (ReloadedAsset == nullptr
			|| ReloadedPackage == nullptr
			|| !ReloadedPackage->GetName().Equals(PackageName, ESearchCase::CaseSensitive)
			|| ReloadedPackage->IsDirty()
			|| !FPackageName::DoesPackageExist(PackageName))
		{
			OutError = FString::Printf(TEXT("disk reload 뒤 exact persisted object/package identity 또는 clean state를 확인하지 못했습니다: %s"), *PackageName);
			return false;
		}
		if (!ValidateTypedAssetFingerprint<TAsset, TPayload>(
			*ReloadedAsset,
			ExpectedFingerprint,
			ExtractPayload,
			BuildFingerprint,
			OutError))
		{
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Update target의 provider-local typed snapshot과 original dirty state를 pre-save failure 뒤 복원하고 semantic readback합니다.
	template <typename TAsset, typename TPayload>
	bool RestoreUpdatedAsset(
		TAsset& Asset,
		UPackage& Package,
		const TPayload& OriginalPayload,
		bool bOriginalPackageDirty,
		const FString& OriginalFingerprint,
		void (*MaterializePayload)(TAsset&, const TPayload&),
		bool (*ExtractPayload)(const TAsset&, TPayload&, TArray<FCFDAStagingIssue>&),
		bool (*BuildFingerprint)(const TPayload&, FString&, FString&))
	{
		MaterializePayload(Asset, OriginalPayload);
		Package.SetDirtyFlag(bOriginalPackageDirty);

		// Rollback 뒤 exact original semantic state 검증 실패 상세입니다.
		FString RollbackError;
		return Package.IsDirty() == bOriginalPackageDirty
			&& ValidateTypedAssetFingerprint<TAsset, TPayload>(
				Asset,
				OriginalFingerprint,
				ExtractPayload,
				BuildFingerprint,
				RollbackError);
	}

	// Provider-local typed payload를 shared durable sequencing으로 exact Create/Update하고 terminal report를 작성합니다.
	template <typename TAsset, typename TPayload>
	void ApplyTypedTarget(
		const FCFDACommonPreviewRow& Row,
		const TPayload& DesiredPayload,
		void (*MaterializePayload)(TAsset&, const TPayload&),
		bool (*ExtractPayload)(const TAsset&, TPayload&, TArray<FCFDAStagingIssue>&),
		bool (*BuildFingerprint)(const TPayload&, FString&, FString&),
		FCFDAStagingTargetApplyReport& OutReport)
	{
		// Exact reviewed target soft path입니다.
		const FSoftObjectPath TargetPath(Row.Envelope.TargetObjectPath);
		// Exact reviewed target package long name입니다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(Row.Envelope.TargetObjectPath);
		// Exact reviewed target object name입니다.
		const FString AssetName = TargetPath.GetAssetName();
		if (!TargetPath.IsValid()
			|| !FPackageName::IsValidLongPackageName(PackageName)
			|| AssetName.IsEmpty())
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = TEXT("execution target package/object identity가 유효하지 않습니다.");
			return;
		}

		if (Row.Kind == ECFDAStagingPreviewKind::Create)
		{
			if (FindPackage(nullptr, *PackageName) != nullptr
				|| FPackageName::DoesPackageExist(PackageName)
				|| TargetPath.ResolveObject() != nullptr)
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				OutReport.Diagnostic = TEXT("global preflight 뒤 Create target/package가 새로 생겨 mutation을 시작하지 않았습니다.");
				return;
			}

			// Exact new target package입니다.
			UPackage* Package = CreatePackage(*PackageName);
			// Exact new typed DataAsset UObject입니다.
			TAsset* Asset = Package != nullptr
				? NewObject<TAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone | RF_Transactional)
				: nullptr;
			if (Package == nullptr || Asset == nullptr)
			{
				OutReport.Result = Package == nullptr
					? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
					: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				OutReport.Diagnostic = TEXT("CreatePackage/NewObject 단계에서 exact typed target 생성에 실패했습니다.");
				return;
			}

			MaterializePayload(*Asset, DesiredPayload);
			// Pre-save typed semantic readback 실패 상세입니다.
			FString ReadbackError;
			if (!ValidateTypedAssetFingerprint<TAsset, TPayload>(
				*Asset,
				Row.Envelope.StagingSemanticFingerprint,
				ExtractPayload,
				BuildFingerprint,
				ReadbackError))
			{
				// Create pre-save failure 뒤 operation-created state cleanup 확인 결과입니다.
				const bool bRollbackConfirmed = CleanupCreatedAsset(Asset, Package, false);
				OutReport.Result = bRollbackConfirmed
					? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
					: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				OutReport.Diagnostic = ReadbackError;
				return;
			}

			FAssetRegistryModule::AssetCreated(Asset);
			Package->MarkPackageDirty();
			// SavePackage가 실제로 호출되었는지 나타내는 uncertainty 분기 evidence입니다.
			bool bSaveWasCalled = false;
			// Exact save + reload + persisted typed readback 실패 상세입니다.
			FString SaveError;
			if (!SaveExactPackage<TAsset, TPayload>(
				*Package,
				*Asset,
				Row.Envelope.StagingSemanticFingerprint,
				ExtractPayload,
				BuildFingerprint,
				bSaveWasCalled,
				SaveError))
			{
				if (bSaveWasCalled)
				{
					OutReport.Result = ECFDAStagingTargetApplyResult::SaveStateUnconfirmed;
				}
				else
				{
					// Save 진입 전 Create failure 뒤 cleanup 확인 결과입니다.
					const bool bRollbackConfirmed = CleanupCreatedAsset(Asset, Package, true);
					OutReport.Result = bRollbackConfirmed
						? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
						: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				}
				OutReport.Diagnostic = SaveError;
				return;
			}

			OutReport.Result = ECFDAStagingTargetApplyResult::DurableApplied;
			OutReport.Diagnostic = TEXT("Create target가 exact package save + clean + persisted existence + typed semantic readback을 통과했습니다.");
			return;
		}

		if (Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			// Exact existing typed update target입니다.
			TAsset* Asset = Cast<TAsset>(TargetPath.ResolveObject());
			if (Asset == nullptr)
			{
				Asset = LoadObject<TAsset>(nullptr, *Row.Envelope.TargetObjectPath);
			}
			// Exact update target package입니다.
			UPackage* Package = Asset != nullptr ? Asset->GetOutermost() : nullptr;
			if (Asset == nullptr || Package == nullptr || Package->IsDirty())
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				OutReport.Diagnostic = TEXT("global preflight 뒤 Update target이 사라졌거나 package가 dirty로 바뀌어 mutation을 시작하지 않았습니다.");
				return;
			}

			// Rollback을 위해 mutation 전에 lossless하게 추출한 provider-local original payload입니다.
			TPayload OriginalPayload;
			// Original payload typed extraction diagnostics입니다.
			TArray<FCFDAStagingIssue> OriginalPayloadIssues;
			if (!ExtractPayload(*Asset, OriginalPayload, OriginalPayloadIssues))
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite;
				OutReport.Issues = MoveTemp(OriginalPayloadIssues);
				OutReport.Diagnostic = TEXT("Update mutation 시작 전에 original typed snapshot을 lossless 추출하지 못했습니다.");
				return;
			}
			// Mutation 전 original package dirty state입니다.
			const bool bOriginalPackageDirty = Package->IsDirty();
			// Mutation 전 original typed semantic fingerprint입니다.
			FString OriginalFingerprint;
			// Original semantic fingerprint 생성 실패 상세입니다.
			FString OriginalFingerprintError;
			if (!BuildFingerprint(OriginalPayload, OriginalFingerprint, OriginalFingerprintError))
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite;
				OutReport.Diagnostic = OriginalFingerprintError;
				return;
			}

			Asset->Modify();
			MaterializePayload(*Asset, DesiredPayload);
			Package->MarkPackageDirty();

			// Pre-save typed semantic readback 실패 상세입니다.
			FString ReadbackError;
			if (!ValidateTypedAssetFingerprint<TAsset, TPayload>(
				*Asset,
				Row.Envelope.StagingSemanticFingerprint,
				ExtractPayload,
				BuildFingerprint,
				ReadbackError))
			{
				// Update pre-save failure 뒤 original typed state rollback 확인 결과입니다.
				const bool bRollbackConfirmed = RestoreUpdatedAsset<TAsset, TPayload>(
					*Asset,
					*Package,
					OriginalPayload,
					bOriginalPackageDirty,
					OriginalFingerprint,
					MaterializePayload,
					ExtractPayload,
					BuildFingerprint);
				OutReport.Result = bRollbackConfirmed
					? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
					: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				OutReport.Diagnostic = ReadbackError;
				return;
			}

			// SavePackage가 실제로 호출되었는지 나타내는 uncertainty 분기 evidence입니다.
			bool bSaveWasCalled = false;
			// Exact save + reload + persisted typed readback 실패 상세입니다.
			FString SaveError;
			if (!SaveExactPackage<TAsset, TPayload>(
				*Package,
				*Asset,
				Row.Envelope.StagingSemanticFingerprint,
				ExtractPayload,
				BuildFingerprint,
				bSaveWasCalled,
				SaveError))
			{
				if (bSaveWasCalled)
				{
					OutReport.Result = ECFDAStagingTargetApplyResult::SaveStateUnconfirmed;
				}
				else
				{
					// Save 진입 전 Update failure 뒤 original typed state rollback 확인 결과입니다.
					const bool bRollbackConfirmed = RestoreUpdatedAsset<TAsset, TPayload>(
						*Asset,
						*Package,
						OriginalPayload,
						bOriginalPackageDirty,
						OriginalFingerprint,
						MaterializePayload,
						ExtractPayload,
						BuildFingerprint);
					OutReport.Result = bRollbackConfirmed
						? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
						: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				}
				OutReport.Diagnostic = SaveError;
				return;
			}

			OutReport.Result = ECFDAStagingTargetApplyResult::DurableApplied;
			OutReport.Diagnostic = TEXT("Update target가 exact package save + clean + persisted existence + typed semantic readback을 통과했습니다.");
			return;
		}

		OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		OutReport.Diagnostic = TEXT("P0 durable core는 Create/Update operation만 실행합니다.");
	}
}
