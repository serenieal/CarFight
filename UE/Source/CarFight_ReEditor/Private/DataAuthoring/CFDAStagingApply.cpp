// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingApply.cpp
// Version: v1.3.0
// Date: 2026-09-08
// Description: CF-FQ-049 DAS-P0-03 one-shot approval, disk/current global TOCTOU preflight, MissileGuidePreset exact typed materialization과 durable single-package save 구현입니다.
// Changelog:
// - v1.3.0: CF-FQ-050 DACE-P0-01 전용 WITH_DEV_AUTOMATION_TESTS transient materializer→production extractor private roundtrip probe를 추가. Product Apply/Save path는 변경하지 않음.
// - v1.2.0: DAS-P0-04 Automation fixture 전용 deterministic save/confirmation/before-mutation fault injection을 WITH_DEV_AUTOMATION_TESTS에 한정해 추가.
// - v1.1.0: DurableApplied를 SavePackage 뒤 exact package 비대화 없는 non-interactive disk reload + unified typed semantic extractor readback으로 강화하고 Apply/rollback의 UObject→payload 변환 authority를 CFDAStagingService 하나로 통합.
// - v1.0.1: global preflight 이후 각 target mutation 직전 current truth를 다시 검증하고, Create 실패 뒤 operation-created package가 memory에 남으면 rollback confirmed를 주장하지 않도록 보수화.
// - v1.0.0: Reviewed approval freeze, disk Staging re-read, fresh Preview/hash revalidation, deterministic Create/Update, pre-save rollback, SaveStateUnconfirmed와 PartialApplied aggregation을 추가.
// Migration:
// - P0 write allowlist는 UCFMissileGuidePresetData 하나뿐입니다. Product Low/Normal/High를 자동 적용하지 않으며 caller가 Reviewed approval을 명시적으로 전달해야 합니다.

#include "DataAuthoring/CFDAStagingApply.h"
#include "CFDAContractGuard.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFMissileGuidePresetData.h"
#include "HAL/FileManager.h"
#include "Internationalization/Text.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace CFDAStagingApplyPrivate
{
#if WITH_DEV_AUTOMATION_TESTS
	// SavePackage 호출 지점에서 uncertainty를 강제할 exact test target path입니다.
	FString GForceSaveFailureTargetPath;

	// SavePackage 성공 뒤 durable confirmation 실패를 강제할 exact test target path입니다.
	FString GForceConfirmationFailureTargetPath;

	// global preflight 뒤 mutation 직전에 known block을 강제할 exact test target path입니다.
	FString GForceBeforeMutationBlockTargetPath;

	// exact target이 지정된 test fault path와 일치하는지 확인합니다.
	bool MatchesTestFaultTarget(const FString& FaultTargetPath, const FString& TargetObjectPath)
	{
		return !FaultTargetPath.IsEmpty()
			&& FaultTargetPath.Equals(TargetObjectPath, ESearchCase::CaseSensitive);
	}
#endif
	// Stable issue 하나를 Apply report에 추가합니다.
	void AddIssue(
		TArray<FCFDAStagingIssue>& OutIssues,
		const ECFDAStagingIssueCode Code,
		const FString& FieldPath,
		const FString& Message)
	{
		// 새 Apply diagnostic입니다.
		FCFDAStagingIssue Issue;
		Issue.Code = Code;
		Issue.FieldPath = FieldPath;
		Issue.Message = Message;
		Issue.bBlocking = true;
		OutIssues.Add(MoveTemp(Issue));
	}

	// Preview/approval target의 deterministic execution key입니다.
	FString BuildTargetSortKey(
		const FString& DataAssetTypeClassPath,
		const FName StableLogicalId,
		const FString& TargetObjectPath,
		const FString& StagingRelativePath)
	{
		return DataAssetTypeClassPath
			+ TEXT("\n")
			+ StableLogicalId.ToString().ToLower()
			+ TEXT("\n")
			+ TargetObjectPath
			+ TEXT("\n")
			+ StagingRelativePath;
	}

	// Preview row 하나를 immutable approval target projection으로 복사합니다.
	FCFDAStagingApprovalTarget BuildApprovalTarget(const FCFDAStagingPreviewRow& Row)
	{
		// 반환할 exact approval target입니다.
		FCFDAStagingApprovalTarget Target;
		Target.SchemaId = Row.Record.SchemaId;
		Target.SchemaRevision = Row.Record.SchemaRevision;
		Target.AdapterContractRevision = Row.Record.AdapterContractRevision;
		Target.DataAssetTypeClassPath = Row.Record.DataAssetTypeClassPath;
		Target.StableLogicalId = Row.Record.StableLogicalId;
		Target.TargetObjectPath = Row.Record.TargetObjectPath;
		Target.StagingRelativePath = Row.Record.StagingRelativePath;
		Target.bHasBaseSemanticFingerprint = Row.Record.bHasBaseSemanticFingerprint;
		Target.BaseSemanticFingerprint = Row.Record.BaseSemanticFingerprint;
		Target.CurrentSemanticFingerprint = Row.CurrentSemanticFingerprint;
		Target.StagingSemanticFingerprint = Row.Record.StagingSemanticFingerprint;
		Target.PlannedOperation = Row.Kind;
		return Target;
	}

	// Approval target 하나의 initial report projection을 만듭니다.
	FCFDAStagingTargetApplyReport BuildInitialTargetReport(const FCFDAStagingApprovalTarget& Target)
	{
		// 반환할 target report입니다.
		FCFDAStagingTargetApplyReport Report;
		Report.TargetObjectPath = Target.TargetObjectPath;
		Report.StableLogicalId = Target.StableLogicalId;
		Report.PlannedOperation = Target.PlannedOperation;
		return Report;
	}

	// CarFight main_game root를 UE ProjectDir의 부모로 exact resolve합니다.
	FString GetMainGameRoot()
	{
		// CarFight .uproject가 위치한 `<main_game>/UE/` directory입니다.
		const FString UnrealProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		// canonical Staging root가 위치한 `<main_game>/` directory입니다.
		FString MainGameRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(UnrealProjectRoot, TEXT("..")));
		FPaths::NormalizeDirectoryName(MainGameRoot);
		return MainGameRoot;
	}

	// Approval target의 exact main_game-relative Staging JSON을 UTF-8 text로 다시 읽습니다.
	bool ReadStagingFile(const FString& StagingRelativePath, FString& OutJsonText, FString& OutError)
	{
		// canonical main_game absolute root입니다.
		const FString MainGameRoot = GetMainGameRoot();
		// approval이 가리키는 exact Staging source absolute path입니다.
		FString StagingAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(MainGameRoot, StagingRelativePath));
		FPaths::NormalizeFilename(StagingAbsolutePath);

		// normalized main_game root의 slash-normalized comparison text입니다.
		FString MainGameRootCompare = MainGameRoot;
		FPaths::NormalizeFilename(MainGameRootCompare);
		if (!MainGameRootCompare.EndsWith(TEXT("/")))
		{
			MainGameRootCompare += TEXT("/");
		}
		if (!StagingAbsolutePath.StartsWith(MainGameRootCompare, ESearchCase::IgnoreCase))
		{
			OutError = TEXT("Staging absolute path가 main_game root 밖으로 벗어났습니다.");
			return false;
		}
		if (!FFileHelper::LoadFileToString(OutJsonText, *StagingAbsolutePath))
		{
			OutError = FString::Printf(TEXT("Reviewed Staging source를 다시 읽지 못했습니다: %s"), *StagingRelativePath);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Approval target과 fresh Preview row의 exact evidence binding이 일치하는지 검증합니다.
	bool MatchesApprovalTarget(
		const FCFDAStagingApprovalTarget& ApprovalTarget,
		const FCFDAStagingPreviewRow& FreshRow,
		FString& OutError)
	{
		if (ApprovalTarget.SchemaId != FreshRow.Record.SchemaId
			|| ApprovalTarget.SchemaRevision != FreshRow.Record.SchemaRevision
			|| ApprovalTarget.AdapterContractRevision != FreshRow.Record.AdapterContractRevision
			|| ApprovalTarget.DataAssetTypeClassPath != FreshRow.Record.DataAssetTypeClassPath
			|| ApprovalTarget.StableLogicalId != FreshRow.Record.StableLogicalId
			|| !ApprovalTarget.TargetObjectPath.Equals(FreshRow.Record.TargetObjectPath, ESearchCase::CaseSensitive)
			|| !ApprovalTarget.StagingRelativePath.Equals(FreshRow.Record.StagingRelativePath, ESearchCase::CaseSensitive)
			|| ApprovalTarget.bHasBaseSemanticFingerprint != FreshRow.Record.bHasBaseSemanticFingerprint
			|| !ApprovalTarget.BaseSemanticFingerprint.Equals(FreshRow.Record.BaseSemanticFingerprint, ESearchCase::CaseSensitive)
			|| !ApprovalTarget.CurrentSemanticFingerprint.Equals(FreshRow.CurrentSemanticFingerprint, ESearchCase::CaseSensitive)
			|| !ApprovalTarget.StagingSemanticFingerprint.Equals(FreshRow.Record.StagingSemanticFingerprint, ESearchCase::CaseSensitive)
			|| ApprovalTarget.PlannedOperation != FreshRow.Kind)
		{
			OutError = TEXT("Reviewed approval target evidence와 fresh disk/current Preview가 exact 일치하지 않습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Create candidate가 새 package를 만들기 전에 package 자체도 완전히 absent인지 확인합니다.
	bool ValidateCreatePackageAbsent(const FCFDAStagingPreviewRow& Row, FString& OutError)
	{
		if (Row.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError.Reset();
			return true;
		}

		// exact target package long name입니다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(Row.Record.TargetObjectPath);
		if (!FPackageName::IsValidLongPackageName(PackageName)
			|| FindPackage(nullptr, *PackageName) != nullptr
			|| FPackageName::DoesPackageExist(PackageName))
		{
			OutError = FString::Printf(TEXT("Create target package가 이미 memory 또는 disk에 존재합니다: %s"), *PackageName);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Typed whole-record payload를 exact MissileGuidePreset DataAsset에 적용합니다.
	void ApplyPayloadToAsset(const FCFDAMissilePresetPayload& Payload, UCFMissileGuidePresetData& Asset)
	{
		Asset.PresetId = Payload.PresetId;
		Asset.PresetDisplayName = FText::FromString(Payload.PresetDisplayName.Text);
		Asset.PresetDescription = FText::FromString(Payload.PresetDescription.Text);
		Asset.MissileGuideConfig = Payload.MissileGuideConfig;
	}

	// Asset의 current semantic fingerprint가 current resolver와 동일한 exact extractor 기준으로 expected fingerprint와 일치하는지 검증합니다.
	bool ValidateAssetFingerprint(
		const UCFMissileGuidePresetData& Asset,
		const FString& ExpectedFingerprint,
		FString& OutError)
	{
		// current Asset을 whole-record typed payload로 lossless 추출한 값입니다.
		FCFDAMissilePresetPayload CurrentPayload;
		// FText representation/typed authored validation까지 포함한 extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractionIssues;
		if (!FCFDAStagingService::ExtractMissilePresetPayload(Asset, CurrentPayload, ExtractionIssues))
		{
			OutError = ExtractionIssues.IsEmpty()
				? TEXT("typed semantic readback payload 추출에 실패했습니다.")
				: ExtractionIssues[0].Message;
			return false;
		}

		// current Asset semantic fingerprint입니다.
		FString CurrentFingerprint;
		// fingerprint generation failure입니다.
		FString FingerprintError;
		if (!FCFDAStagingService::BuildSemanticFingerprint(CurrentPayload, CurrentFingerprint, FingerprintError))
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

	// Exact target package 하나만 SavePackage하고 disk reload 기반 persisted semantic readback까지 durable confirmation을 수행합니다.
	bool SaveExactPackage(
		UPackage& Package,
		UCFMissileGuidePresetData& Asset,
		const FString& ExpectedFingerprint,
		bool& bOutSaveWasCalled,
		FString& OutError)
	{
		bOutSaveWasCalled = false;
		// reload 뒤 old UObject pointer를 사용하지 않기 위해 save 전에 동결하는 exact package long name입니다.
		const FString PackageName = Package.GetName();
		// reload 뒤 exact persisted object를 다시 resolve하기 위한 object path입니다.
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

		// package의 canonical .uasset filename입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackageName,
			FPackageName::GetAssetPackageExtension());
		// 신규 package를 위한 exact output directory입니다.
		const FString PackageDirectory = FPaths::GetPath(PackageFilename);
		IFileManager::Get().MakeDirectory(*PackageDirectory, true);

		// UE 5.8 exact single-package save arguments입니다.
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		bOutSaveWasCalled = true;
#if WITH_DEV_AUTOMATION_TESTS
		if (MatchesTestFaultTarget(GForceSaveFailureTargetPath, TargetObjectPath))
		{
			OutError = TEXT("DAS-P0-04 Automation fixture가 SavePackage outcome uncertainty를 강제했습니다.");
			return false;
		}
#endif
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
#if WITH_DEV_AUTOMATION_TESTS
		if (MatchesTestFaultTarget(GForceConfirmationFailureTargetPath, TargetObjectPath))
		{
			OutError = TEXT("DAS-P0-04 Automation fixture가 SavePackage 성공 뒤 persisted confirmation uncertainty를 강제했습니다.");
			return false;
		}
#endif

		// 방금 저장한 exact package 하나만 disk state로 non-interactive reload할 목록입니다.
		TArray<UPackage*> PackagesToReload;
		PackagesToReload.Add(&Package);
		// package reload 실패 상세입니다.
		FText ReloadError;
		if (!UPackageTools::ReloadPackages(PackagesToReload, ReloadError, EReloadPackagesInteractionMode::AssumePositive))
		{
			OutError = FString::Printf(
				TEXT("SavePackage success 뒤 exact persisted package reload를 확인하지 못했습니다: %s / %s"),
				*PackageName,
				*ReloadError.ToString());
			return false;
		}

		// disk reload 뒤 exact object path에서 다시 resolve한 persisted Pilot DataAsset입니다.
		UCFMissileGuidePresetData* ReloadedAsset = Cast<UCFMissileGuidePresetData>(FSoftObjectPath(TargetObjectPath).ResolveObject());
		if (ReloadedAsset == nullptr)
		{
			ReloadedAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, *TargetObjectPath);
		}
		// reloaded object가 소유하는 exact package입니다.
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
		if (!ValidateAssetFingerprint(*ReloadedAsset, ExpectedFingerprint, OutError))
		{
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Create target의 pre-save failure에서 operation-created UObject/registry/dirty state를 best-effort로 정리하고 original package absence까지 확인합니다.
	bool CleanupCreatedAsset(
		UCFMissileGuidePresetData* Asset,
		UPackage* Package,
		const bool bRegistryNotified)
	{
		if (Asset == nullptr || Package == nullptr)
		{
			return false;
		}
		// Create 이전에는 존재하지 않았어야 하는 exact package long name입니다.
		const FString CreatedPackageName = Package->GetName();
		if (bRegistryNotified)
		{
			FAssetRegistryModule::AssetDeleted(Asset);
		}
		Asset->ClearFlags(RF_Public | RF_Standalone);
		// operation-created object를 transient package로 옮겨 original target path ownership을 해제합니다.
		const bool bRenamed = Asset->Rename(
			nullptr,
			GetTransientPackage(),
			REN_DontCreateRedirectors | REN_NonTransactional);
		Asset->MarkAsGarbage();
		Package->SetDirtyFlag(false);

		// Package UObject 자체가 process memory에 남으면 original `package absent` 상태를 exact 복원했다고 주장할 수 없습니다.
		const bool bPackageAbsenceRestored = FindPackage(nullptr, *CreatedPackageName) == nullptr;
		return bRenamed && !Package->IsDirty() && bPackageAbsenceRestored;
	}

	// Update target의 typed snapshot과 original dirty state를 pre-save failure 뒤 복원하고 검증합니다.
	bool RestoreUpdatedAsset(
		UCFMissileGuidePresetData& Asset,
		UPackage& Package,
		const FCFDAMissilePresetPayload& OriginalPayload,
		const bool bOriginalPackageDirty,
		const FString& OriginalFingerprint)
	{
		ApplyPayloadToAsset(OriginalPayload, Asset);
		Package.SetDirtyFlag(bOriginalPackageDirty);
		// rollback semantic readback failure입니다.
		FString RollbackError;
		return Package.IsDirty() == bOriginalPackageDirty
			&& ValidateAssetFingerprint(Asset, OriginalFingerprint, RollbackError);
	}

	// Fresh global preflight를 approval exact target order로 다시 구축합니다.
	bool BuildFreshRows(
		const FCFDAStagingReviewedApproval& Approval,
		TArray<FCFDAStagingPreviewRow>& OutFreshRows,
		FCFDAStagingApplyReport& OutReport)
	{
		OutFreshRows.Reset();
		for (int32 TargetIndex = 0; TargetIndex < Approval.IncludedTargets.Num(); ++TargetIndex)
		{
			// immutable approval target입니다.
			const FCFDAStagingApprovalTarget& ApprovalTarget = Approval.IncludedTargets[TargetIndex];
			// matching report row입니다.
			FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[TargetIndex];

			// disk Staging source current text입니다.
			FString JsonText;
			// preflight failure detail입니다.
			FString PreflightError;
			if (!ReadStagingFile(ApprovalTarget.StagingRelativePath, JsonText, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("StagingRelativePath"), PreflightError);
				return false;
			}

			// fresh disk text를 strict typed DTO로 다시 parse한 결과입니다.
			const FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(JsonText, ApprovalTarget.StagingRelativePath);
			if (!ParseResult.bValid)
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = ParseResult.Issues;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Staging"), TEXT("Reviewed 이후 Staging parse/validation truth가 바뀌었습니다."));
				TargetReport.Diagnostic = TEXT("Reviewed 이후 Staging JSON이 current strict schema를 만족하지 않습니다.");
				return false;
			}

			// fresh current UE target/identity/package truth입니다.
			FCFDAStagingCurrentState CurrentState;
			// resolver diagnostics입니다.
			TArray<FCFDAStagingIssue> CurrentIssues;
			if (!FCFDAStagingService::ResolveMissilePresetCurrentState(ParseResult.Record, CurrentState, CurrentIssues))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = MoveTemp(CurrentIssues);
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Current"), TEXT("Apply 직전 current UE truth를 확정하지 못했습니다."));
				TargetReport.Diagnostic = TEXT("Apply global preflight current resolver가 실패했습니다.");
				return false;
			}

			// current truth와 disk staging을 exact 3-way로 재분류한 fresh Preview입니다.
			FCFDAStagingPreviewRow FreshRow = FCFDAStagingService::BuildPreview(ParseResult.Record, CurrentState);
			if (FreshRow.Kind == ECFDAStagingPreviewKind::Conflict || FreshRow.Kind == ECFDAStagingPreviewKind::Invalid)
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = FreshRow.Issues;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Preview"), TEXT("Apply 직전 fresh Preview가 reviewed Create/Update 상태가 아닙니다."));
				TargetReport.Diagnostic = TEXT("Apply global preflight에서 fresh Preview가 blocker로 전환됐습니다.");
				return false;
			}
			if (!MatchesApprovalTarget(ApprovalTarget, FreshRow, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Approval"), PreflightError);
				return false;
			}
			if (!ValidateCreatePackageAbsent(FreshRow, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::PathCollision, TEXT("TargetObjectPath"), PreflightError);
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Approval"), TEXT("Create package absence가 reviewed 이후 바뀌었습니다."));
				return false;
			}
			OutFreshRows.Add(MoveTemp(FreshRow));
		}

		FCFDAStagingService::ApplyBatchDuplicateValidation(OutFreshRows);
		for (int32 RowIndex = 0; RowIndex < OutFreshRows.Num(); ++RowIndex)
		{
			// duplicate validation 이후 fresh row입니다.
			const FCFDAStagingPreviewRow& FreshRow = OutFreshRows[RowIndex];
			if (FreshRow.Kind == ECFDAStagingPreviewKind::Conflict || FreshRow.Kind == ECFDAStagingPreviewKind::Invalid)
			{
				FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[RowIndex];
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = FreshRow.Issues;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Batch"), TEXT("Apply 직전 batch duplicate validation 결과가 blocker입니다."));
				TargetReport.Diagnostic = TEXT("Apply global batch preflight duplicate validation이 실패했습니다.");
				return false;
			}
		}

		// fresh exact target set의 BatchPlanHash입니다.
		FString FreshBatchPlanHash;
		// fresh hash generation error입니다.
		FString HashError;
		if (!FCFDAStagingService::BuildBatchPlanHash(OutFreshRows, FreshBatchPlanHash, HashError)
			|| !FreshBatchPlanHash.Equals(Approval.BatchPlanHash, ESearchCase::CaseSensitive))
		{
			if (!OutReport.Targets.IsEmpty())
			{
				FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[0];
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("BatchPlanHash"), TEXT("Apply 직전 fresh BatchPlanHash가 reviewed approval과 다릅니다."));
				TargetReport.Diagnostic = HashError.IsEmpty() ? TEXT("BatchPlanHash stale mismatch입니다.") : HashError;
			}
			return false;
		}
		return true;
	}

	// Global preflight 뒤 각 target mutation 직전에 current truth가 exact reviewed execution row와 여전히 같은지 다시 검증합니다.
	bool RevalidateImmediatelyBeforeMutation(
		const FCFDAStagingPreviewRow& PlannedRow,
		TArray<FCFDAStagingIssue>& OutIssues,
		FString& OutError)
	{
		// mutation 직전 exact current UE target/identity/package truth입니다.
		FCFDAStagingCurrentState CurrentState;
		// current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!FCFDAStagingService::ResolveMissilePresetCurrentState(PlannedRow.Record, CurrentState, CurrentIssues))
		{
			OutIssues = MoveTemp(CurrentIssues);
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Current"), TEXT("Global preflight 이후 mutation 직전 current truth를 다시 확정하지 못했습니다."));
			OutError = TEXT("Mutation 직전 current resolver가 실패했습니다.");
			return false;
		}

		// mutation 직전 exact 3-way Preview입니다.
		const FCFDAStagingPreviewRow ImmediateRow = FCFDAStagingService::BuildPreview(PlannedRow.Record, CurrentState);
		if (ImmediateRow.Kind == ECFDAStagingPreviewKind::Conflict || ImmediateRow.Kind == ECFDAStagingPreviewKind::Invalid)
		{
			OutIssues = ImmediateRow.Issues;
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Preview"), TEXT("Global preflight 이후 mutation 직전 current state가 blocker로 변했습니다."));
			OutError = TEXT("Mutation 직전 fresh Preview가 Conflict/Invalid로 변했습니다.");
			return false;
		}

		// global preflight가 동결한 exact row를 temporary approval projection으로 재사용합니다.
		const FCFDAStagingApprovalTarget PlannedTarget = BuildApprovalTarget(PlannedRow);
		if (!MatchesApprovalTarget(PlannedTarget, ImmediateRow, OutError))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("CurrentSemanticFingerprint"), TEXT("Global preflight 이후 mutation 직전 exact target evidence가 바뀌었습니다."));
			return false;
		}
		if (!ValidateCreatePackageAbsent(ImmediateRow, OutError))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::PathCollision, TEXT("TargetObjectPath"), OutError);
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Approval"), TEXT("Global preflight 이후 Create package absence가 바뀌었습니다."));
			return false;
		}
		OutError.Reset();
		return true;
	}

	// One target을 exact typed materialization하고 durable terminal result를 반환합니다.
	void ApplyOneTarget(
		const FCFDAStagingPreviewRow& Row,
		FCFDAStagingTargetApplyReport& OutReport)
	{
		// exact target soft path입니다.
		const FSoftObjectPath TargetPath(Row.Record.TargetObjectPath);
		// exact target package long name입니다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(Row.Record.TargetObjectPath);
		// exact target asset object name입니다.
		const FString AssetName = TargetPath.GetAssetName();
		if (!TargetPath.IsValid()
			|| !FPackageName::IsValidLongPackageName(PackageName)
			|| AssetName.IsEmpty())
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = TEXT("execution target package/object identity가 유효하지 않습니다.");
			return;
		}

#if WITH_DEV_AUTOMATION_TESTS
		if (MatchesTestFaultTarget(GForceBeforeMutationBlockTargetPath, Row.Record.TargetObjectPath))
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = TEXT("DAS-P0-04 Automation fixture가 global preflight 뒤 mutation 직전 known block을 강제했습니다.");
			return;
		}
#endif

		// Global preflight와 실제 mutation 사이에 생긴 drift를 target-local로 마지막 한 번 차단합니다.
		FString ImmediatePreflightError;
		if (!RevalidateImmediatelyBeforeMutation(Row, OutReport.Issues, ImmediatePreflightError))
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = ImmediatePreflightError;
			return;
		}

		if (Row.Kind == ECFDAStagingPreviewKind::Create)
		{
			if (FindPackage(nullptr, *PackageName) != nullptr || FPackageName::DoesPackageExist(PackageName) || TargetPath.ResolveObject() != nullptr)
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				OutReport.Diagnostic = TEXT("global preflight 뒤 Create target/package가 새로 생겨 mutation을 시작하지 않았습니다.");
				return;
			}

			// exact new target package입니다.
			UPackage* Package = CreatePackage(*PackageName);
			// exact new typed DataAsset입니다.
			UCFMissileGuidePresetData* Asset = Package
				? NewObject<UCFMissileGuidePresetData>(Package, FName(*AssetName), RF_Public | RF_Standalone | RF_Transactional)
				: nullptr;
			if (Package == nullptr || Asset == nullptr)
			{
				// CreatePackage가 성공한 뒤 NewObject가 실패하면 operation-created package가 memory에 남을 수 있어 exact rollback을 주장하지 않습니다.
				OutReport.Result = Package == nullptr
					? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
					: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				OutReport.Diagnostic = TEXT("CreatePackage/NewObject 단계에서 exact typed target 생성에 실패했습니다.");
				return;
			}

			ApplyPayloadToAsset(Row.Record.Payload, *Asset);
			// pre-save typed readback error입니다.
			FString ReadbackError;
			if (!ValidateAssetFingerprint(*Asset, Row.Record.StagingSemanticFingerprint, ReadbackError))
			{
				const bool bRollbackConfirmed = CleanupCreatedAsset(Asset, Package, false);
				OutReport.Result = bRollbackConfirmed
					? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
					: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				OutReport.Diagnostic = ReadbackError;
				return;
			}

			FAssetRegistryModule::AssetCreated(Asset);
			Package->MarkPackageDirty();
			// SavePackage 호출 여부입니다.
			bool bSaveWasCalled = false;
			// exact save/durable confirmation error입니다.
			FString SaveError;
			if (!SaveExactPackage(*Package, *Asset, Row.Record.StagingSemanticFingerprint, bSaveWasCalled, SaveError))
			{
				if (bSaveWasCalled)
				{
					OutReport.Result = ECFDAStagingTargetApplyResult::SaveStateUnconfirmed;
				}
				else
				{
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
			// exact update target object입니다.
			UCFMissileGuidePresetData* Asset = Cast<UCFMissileGuidePresetData>(TargetPath.ResolveObject());
			if (Asset == nullptr)
			{
				Asset = LoadObject<UCFMissileGuidePresetData>(nullptr, *Row.Record.TargetObjectPath);
			}
			// exact update target package입니다.
			UPackage* Package = Asset ? Asset->GetOutermost() : nullptr;
			if (Asset == nullptr || Package == nullptr || Package->IsDirty())
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				OutReport.Diagnostic = TEXT("global preflight 뒤 Update target이 사라졌거나 package가 dirty로 바뀌어 mutation을 시작하지 않았습니다.");
				return;
			}

			// rollback을 위한 exact original typed payload입니다.
			FCFDAMissilePresetPayload OriginalPayload;
			// original payload의 FText/typed whole-record extraction diagnostics입니다.
			TArray<FCFDAStagingIssue> OriginalPayloadIssues;
			if (!FCFDAStagingService::ExtractMissilePresetPayload(*Asset, OriginalPayload, OriginalPayloadIssues))
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite;
				OutReport.Issues = MoveTemp(OriginalPayloadIssues);
				OutReport.Diagnostic = TEXT("Update mutation 시작 전에 original typed snapshot을 lossless 추출하지 못했습니다.");
				return;
			}
			// original package dirty state입니다.
			const bool bOriginalPackageDirty = Package->IsDirty();
			// original semantic fingerprint입니다.
			FString OriginalFingerprint;
			// original fingerprint generation error입니다.
			FString OriginalFingerprintError;
			if (!FCFDAStagingService::BuildSemanticFingerprint(OriginalPayload, OriginalFingerprint, OriginalFingerprintError))
			{
				OutReport.Result = ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite;
				OutReport.Diagnostic = OriginalFingerprintError;
				return;
			}

			Asset->Modify();
			ApplyPayloadToAsset(Row.Record.Payload, *Asset);
			Package->MarkPackageDirty();
			// pre-save typed readback error입니다.
			FString ReadbackError;
			if (!ValidateAssetFingerprint(*Asset, Row.Record.StagingSemanticFingerprint, ReadbackError))
			{
				const bool bRollbackConfirmed = RestoreUpdatedAsset(*Asset, *Package, OriginalPayload, bOriginalPackageDirty, OriginalFingerprint);
				OutReport.Result = bRollbackConfirmed
					? ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite
					: ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed;
				OutReport.Diagnostic = ReadbackError;
				return;
			}

			// SavePackage 호출 여부입니다.
			bool bSaveWasCalled = false;
			// exact save/durable confirmation error입니다.
			FString SaveError;
			if (!SaveExactPackage(*Package, *Asset, Row.Record.StagingSemanticFingerprint, bSaveWasCalled, SaveError))
			{
				if (bSaveWasCalled)
				{
					OutReport.Result = ECFDAStagingTargetApplyResult::SaveStateUnconfirmed;
				}
				else
				{
					const bool bRollbackConfirmed = RestoreUpdatedAsset(*Asset, *Package, OriginalPayload, bOriginalPackageDirty, OriginalFingerprint);
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
		OutReport.Diagnostic = TEXT("P0 materializer는 Create/Update operation만 실행합니다.");
	}

	// Target terminal results를 fail-safe batch aggregate taxonomy로 계산합니다.
	ECFDAStagingBatchApplyResult AggregateBatchResult(
		const TArray<FCFDAStagingTargetApplyReport>& Targets,
		const int32 DurableAppliedCount)
	{
		// uncertainty와 known failure 종류 존재 여부입니다.
		bool bHasSaveStateUnconfirmed = false;
		bool bHasInMemoryStateUnconfirmed = false;
		bool bHasKnownFailure = false;
		bool bHasPostCommitWarning = false;
		bool bHasBlockedBeforeMutation = false;
		for (const FCFDAStagingTargetApplyReport& Target : Targets)
		{
			switch (Target.Result)
			{
			case ECFDAStagingTargetApplyResult::SaveStateUnconfirmed:
				bHasSaveStateUnconfirmed = true;
				break;
			case ECFDAStagingTargetApplyResult::InMemoryStateUnconfirmed:
				bHasInMemoryStateUnconfirmed = true;
				break;
			case ECFDAStagingTargetApplyResult::FailedBeforeDurableWrite:
				bHasKnownFailure = true;
				break;
			case ECFDAStagingTargetApplyResult::BlockedBeforeMutation:
				bHasKnownFailure = true;
				bHasBlockedBeforeMutation = true;
				break;
			case ECFDAStagingTargetApplyResult::PostCommitWarning:
				bHasPostCommitWarning = true;
				break;
			default:
				break;
			}
		}
		if (bHasSaveStateUnconfirmed)
		{
			return ECFDAStagingBatchApplyResult::SaveStateUnconfirmed;
		}
		if (bHasInMemoryStateUnconfirmed)
		{
			return ECFDAStagingBatchApplyResult::InMemoryStateUnconfirmed;
		}
		if (DurableAppliedCount > 0 && bHasKnownFailure)
		{
			return ECFDAStagingBatchApplyResult::PartialApplied;
		}
		if (bHasKnownFailure)
		{
			return bHasBlockedBeforeMutation
				? ECFDAStagingBatchApplyResult::BlockedBeforeMutation
				: ECFDAStagingBatchApplyResult::FailedBeforeDurableWrite;
		}
		if (bHasPostCommitWarning)
		{
			return ECFDAStagingBatchApplyResult::PostCommitWarning;
		}
		if (DurableAppliedCount > 0)
		{
			return ECFDAStagingBatchApplyResult::DurableApplied;
		}
		return ECFDAStagingBatchApplyResult::NoChange;
	}
}

#if WITH_DEV_AUTOMATION_TESTS
// 모든 test-only fault injection state를 초기화합니다.
void FCFDAStagingApplyTestControl::Reset()
{
	CFDAStagingApplyPrivate::GForceSaveFailureTargetPath.Reset();
	CFDAStagingApplyPrivate::GForceConfirmationFailureTargetPath.Reset();
	CFDAStagingApplyPrivate::GForceBeforeMutationBlockTargetPath.Reset();
}

// exact target의 SavePackage 호출 시점을 outcome-unknown failure로 강제합니다.
void FCFDAStagingApplyTestControl::ForceSaveFailure(const FString& TargetObjectPath)
{
	CFDAStagingApplyPrivate::GForceSaveFailureTargetPath = TargetObjectPath;
}

// exact target의 SavePackage 성공 뒤 persisted confirmation 실패를 강제합니다.
void FCFDAStagingApplyTestControl::ForceConfirmationFailure(const FString& TargetObjectPath)
{
	CFDAStagingApplyPrivate::GForceConfirmationFailureTargetPath = TargetObjectPath;
}

// exact target의 global preflight 뒤 mutation 직전 known block을 강제합니다.
void FCFDAStagingApplyTestControl::ForceBeforeMutationBlock(const FString& TargetObjectPath)
{
	CFDAStagingApplyPrivate::GForceBeforeMutationBlockTargetPath = TargetObjectPath;
}
#endif

// Conflict/Invalid 없는 exact Create/Update Preview set을 Reviewed approval evidence로 동결합니다.
bool FCFDAStagingApplyService::BuildReviewedApproval(
	const TArray<FCFDAStagingPreviewRow>& PreviewRows,
	FCFDAStagingReviewedApproval& OutApproval,
	FString& OutError)
{
	OutApproval = FCFDAStagingReviewedApproval();
	OutError.Reset();

	// duplicate validation을 포함해 approval 가능한 exact preview copy입니다.
	TArray<FCFDAStagingPreviewRow> ValidatedRows = PreviewRows;
	FCFDAStagingService::ApplyBatchDuplicateValidation(ValidatedRows);
	for (const FCFDAStagingPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Conflict || Row.Kind == ECFDAStagingPreviewKind::Invalid)
		{
			OutError = TEXT("Conflict/Invalid가 포함된 Preview set은 Reviewed approval로 승격할 수 없습니다.");
			return false;
		}
	}

	// exact reviewed mutation set의 batch plan hash입니다.
	FString BatchPlanHash;
	if (!FCFDAStagingService::BuildBatchPlanHash(ValidatedRows, BatchPlanHash, OutError))
	{
		return false;
	}

	for (const FCFDAStagingPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Create || Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			OutApproval.IncludedTargets.Add(CFDAStagingApplyPrivate::BuildApprovalTarget(Row));
		}
	}
	if (OutApproval.IncludedTargets.IsEmpty())
	{
		OutError = TEXT("Reviewed approval에는 Create/Update mutation candidate가 하나 이상 필요합니다.");
		return false;
	}

	OutApproval.IncludedTargets.Sort([](const FCFDAStagingApprovalTarget& Left, const FCFDAStagingApprovalTarget& Right)
	{
		return CFDAStagingApplyPrivate::BuildTargetSortKey(
			Left.DataAssetTypeClassPath,
			Left.StableLogicalId,
			Left.TargetObjectPath,
			Left.StagingRelativePath)
			< CFDAStagingApplyPrivate::BuildTargetSortKey(
				Right.DataAssetTypeClassPath,
				Right.StableLogicalId,
				Right.TargetObjectPath,
				Right.StagingRelativePath);
	});
	OutApproval.BatchPlanHash = MoveTemp(BatchPlanHash);
	OutApproval.State = ECFDAStagingApprovalState::Reviewed;
	return true;
}

// Reviewed approval을 one-shot 소비하고 disk Staging + current UE truth를 global preflight한 뒤 exact target만 순서대로 materialize/save합니다.
bool FCFDAStagingApplyService::ApplyReviewedBatch(
	FCFDAStagingReviewedApproval& InOutApproval,
	FCFDAStagingApplyReport& OutReport)
{
	OutReport = FCFDAStagingApplyReport();
	if (InOutApproval.State != ECFDAStagingApprovalState::Reviewed
		|| InOutApproval.BatchPlanHash.IsEmpty()
		|| InOutApproval.IncludedTargets.IsEmpty())
	{
		OutReport.Result = ECFDAStagingBatchApplyResult::BlockedBeforeMutation;
		OutReport.Diagnostic = TEXT("Reviewed one-shot approval이 없거나 이미 소비되었습니다. fresh Preview/Review가 필요합니다.");
		return false;
	}

	InOutApproval.State = ECFDAStagingApprovalState::ApplyAttempted;
	for (const FCFDAStagingApprovalTarget& Target : InOutApproval.IncludedTargets)
	{
		OutReport.Targets.Add(CFDAStagingApplyPrivate::BuildInitialTargetReport(Target));
	}

	// disk Staging + fresh current truth로 다시 만든 exact mutation rows입니다.
	TArray<FCFDAStagingPreviewRow> FreshRows;
	if (!CFDAStagingApplyPrivate::BuildFreshRows(InOutApproval, FreshRows, OutReport))
	{
		InOutApproval.State = ECFDAStagingApprovalState::Consumed;
		OutReport.bApprovalConsumed = true;
		OutReport.NotRunCount = 0;
		for (const FCFDAStagingTargetApplyReport& TargetReport : OutReport.Targets)
		{
			if (TargetReport.Result == ECFDAStagingTargetApplyResult::NotRun)
			{
				++OutReport.NotRunCount;
			}
		}
		OutReport.Result = ECFDAStagingBatchApplyResult::BlockedBeforeMutation;
		OutReport.Diagnostic = TEXT("Apply global TOCTOU preflight가 실패했습니다. mutation 0이며 approval은 재사용할 수 없습니다.");
		return false;
	}

	// Global preflight 전체 PASS 뒤 첫 mutation 직전에 one-shot approval을 소비합니다.
	InOutApproval.State = ECFDAStagingApprovalState::Consumed;
	OutReport.bApprovalConsumed = true;

	for (int32 RowIndex = 0; RowIndex < FreshRows.Num(); ++RowIndex)
	{
		// deterministic execution row입니다.
		const FCFDAStagingPreviewRow& FreshRow = FreshRows[RowIndex];
		// matching target report입니다.
		FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[RowIndex];
		CFDAStagingApplyPrivate::ApplyOneTarget(FreshRow, TargetReport);
		if (TargetReport.Result == ECFDAStagingTargetApplyResult::DurableApplied)
		{
			++OutReport.DurableAppliedCount;
			continue;
		}

		// first failure 이후 target은 deterministic stop으로 NotRun을 유지합니다.
		for (int32 NotRunIndex = RowIndex + 1; NotRunIndex < OutReport.Targets.Num(); ++NotRunIndex)
		{
			OutReport.Targets[NotRunIndex].Result = ECFDAStagingTargetApplyResult::NotRun;
		}
		break;
	}

	for (const FCFDAStagingTargetApplyReport& TargetReport : OutReport.Targets)
	{
		if (TargetReport.Result == ECFDAStagingTargetApplyResult::NotRun)
		{
			++OutReport.NotRunCount;
		}
	}
	OutReport.Result = CFDAStagingApplyPrivate::AggregateBatchResult(OutReport.Targets, OutReport.DurableAppliedCount);
	OutReport.Diagnostic = OutReport.Result == ECFDAStagingBatchApplyResult::DurableApplied
		? TEXT("Reviewed exact batch의 모든 mutation target이 durable confirmation을 통과했습니다.")
		: TEXT("Reviewed exact batch가 first-failure stop 또는 uncertainty terminal state로 종료됐습니다.");
	return OutReport.Result == ECFDAStagingBatchApplyResult::DurableApplied
		|| OutReport.Result == ECFDAStagingBatchApplyResult::PostCommitWarning
		|| OutReport.Result == ECFDAStagingBatchApplyResult::NoChange;
}

#if WITH_DEV_AUTOMATION_TESTS
// Production materializer를 test-owned transient asset에 적용한 뒤 production extractor로 semantic roundtrip합니다.
bool CFDAContractProbeMaterializeRoundTrip(
	const FCFDAMissilePresetPayload& Payload,
	FCFDAMissilePresetPayload& OutReadbackPayload,
	FString& OutError)
{
	// Product package/path를 전혀 소유하지 않는 transient test-only asset입니다.
	UCFMissileGuidePresetData* TransientAsset = NewObject<UCFMissileGuidePresetData>(GetTransientPackage(), NAME_None, RF_Transient);
	if (TransientAsset == nullptr)
	{
		OutReadbackPayload = FCFDAMissilePresetPayload();
		OutError = TEXT("DACE transient MissileGuidePreset 생성에 실패했습니다.");
		return false;
	}
	CFDAStagingApplyPrivate::ApplyPayloadToAsset(Payload, *TransientAsset);
	// Production extractor가 반환하는 semantic diagnostics입니다.
	TArray<FCFDAStagingIssue> ExtractIssues;
	if (!FCFDAStagingService::ExtractMissilePresetPayload(*TransientAsset, OutReadbackPayload, ExtractIssues))
	{
		OutError = ExtractIssues.IsEmpty() ? TEXT("DACE transient materializer readback extractor가 실패했습니다.") : ExtractIssues[0].Message;
		return false;
	}
	OutError.Reset();
	return true;
}
#endif
