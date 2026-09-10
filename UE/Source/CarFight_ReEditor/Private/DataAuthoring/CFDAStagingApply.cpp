// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingApply.cpp
// Version: v1.8.1
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-03 payload-free Apply/TOCTOU + provider-neutral typed durable core dispatch 구현입니다.
// Changelog:
// - v1.8.1: CFDADurableCore 전환 뒤 남아 있던 Missile-only durable transaction dead copy와 legacy save/confirmation fault state를 제거해 durable sequencing 단일 authority를 확정.
// - v1.8.0: Missile durable execution을 CFDADurableCore shared typed transaction으로 rewire하고 Save/confirmation fault authority도 shared core로 이동해 Ammo second writer가 동일 durable sequencing을 재사용하도록 함.
// - v1.7.0: provider readiness를 global preflight와 mutation 직전에 검증해 ReadOnlyPreviewReady provider의 source re-read/Reviewed mutation 진입을 fail-closed.
// - v1.6.0: Reviewed approval 동결 자체를 payload-free common Review authority로 이동하고 Public Missile BuildReviewedApproval은 provider-local integrity projection facade로 축소.
// - v1.5.0: Reviewed approval/fresh disk parse/current resolve/immediate TOCTOU/materialize를 common row + complete provider operation entry로 재배선. Shared Apply는 Missile parser/current/materializer를 직접 호출하지 않으며 typed durable body는 Missile provider-local callback 안에 격리.
// - v1.4.0: approval/fresh-read/mutation 직전에 trusted provider를 exact TypeKey로 resolve하고 provider-owned StagingRoot/common envelope 계약을 검증하도록 재배선. 기존 Missile materializer/Public Apply API와 durable save semantics는 보존.
// - v1.3.0: CF-FQ-050 DACE-P0-01 전용 WITH_DEV_AUTOMATION_TESTS transient materializer→production extractor private roundtrip probe를 추가. Product Apply/Save path는 변경하지 않음.
// - v1.2.0: DAS-P0-04 Automation fixture 전용 deterministic save/confirmation/before-mutation fault injection을 WITH_DEV_AUTOMATION_TESTS에 한정해 추가.
// - v1.1.0: DurableApplied를 SavePackage 뒤 exact package 비대화 없는 non-interactive disk reload + unified typed semantic extractor readback으로 강화하고 Apply/rollback의 UObject→payload 변환 authority를 CFDAStagingService 하나로 통합.
// - v1.0.1: global preflight 이후 각 target mutation 직전 current truth를 다시 검증하고, Create 실패 뒤 operation-created package가 memory에 남으면 rollback confirmed를 주장하지 않도록 보수화.
// - v1.0.0: Reviewed approval freeze, disk Staging re-read, fresh Preview/hash revalidation, deterministic Create/Update, pre-save rollback, SaveStateUnconfirmed와 PartialApplied aggregation을 추가.
// Migration:
// - Current write allowlist는 trusted TypeKey provider registry + ReviewedMutationReady readiness가 소유하며 MissileGuidePreset과 AmmoData exact2가 mutation-ready입니다.
// - Product Low/Normal/High 또는 Product Ammo를 자동 적용하지 않으며 caller가 Reviewed approval을 명시적으로 전달해야 합니다.
// - v1.8.1부터 durable Create/Update/Save/reload/readback/rollback sequencing은 CFDADurableCore 단일 authority를 사용합니다.

#include "DataAuthoring/CFDAStagingApply.h"
#include "CFDAContractGuard.h"
#include "CFDADurableCore.h"
#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"

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

	// Payload-free common row 하나를 immutable approval target projection으로 복사합니다.
	FCFDAStagingApprovalTarget BuildApprovalTarget(const FCFDACommonPreviewRow& Row)
	{
		// Shared orchestration common envelope입니다.
		const FCFDACommonEnvelope& Envelope = Row.Envelope;
		// 반환할 exact approval target입니다.
		FCFDAStagingApprovalTarget Target;
		Target.SchemaId = Envelope.SchemaId;
		Target.SchemaRevision = Envelope.SchemaRevision;
		Target.AdapterContractRevision = Envelope.AdapterContractRevision;
		Target.DataAssetTypeClassPath = Envelope.DataAssetTypeClassPath;
		Target.StableLogicalId = Envelope.StableLogicalId;
		Target.TargetObjectPath = Envelope.TargetObjectPath;
		Target.StagingRelativePath = Envelope.StagingRelativePath;
		Target.bHasBaseSemanticFingerprint = Envelope.bHasBaseSemanticFingerprint;
		Target.BaseSemanticFingerprint = Envelope.BaseSemanticFingerprint;
		Target.CurrentSemanticFingerprint = Envelope.CurrentSemanticFingerprint;
		Target.StagingSemanticFingerprint = Envelope.StagingSemanticFingerprint;
		Target.PlannedOperation = Envelope.PlannedOperation;
		return Target;
	}

	// Existing Public Missile Preview row를 provider-local projection 뒤 common approval target으로 변환합니다.
	FCFDAStagingApprovalTarget BuildApprovalTarget(const FCFDAStagingPreviewRow& Row)
	{
		// Public Missile row를 payload-free common row로 투영한 compatibility intermediate입니다.
		FCFDACommonPreviewRow CommonRow;
		CommonRow.Kind = Row.Kind;
		CommonRow.Envelope = CFDAMissileProviderImpl::BuildCommonEnvelope(
			Row.Record,
			Row.CurrentSemanticFingerprint,
			Row.Kind);
		CommonRow.Issues = Row.Issues;
		return BuildApprovalTarget(CommonRow);
	}

	// Immutable approval target을 payload-free common orchestration envelope로 투영합니다.
	FCFDACommonEnvelope BuildApprovalEnvelope(const FCFDAStagingApprovalTarget& Target)
	{
		// Shared provider validation에 전달할 common envelope입니다.
		FCFDACommonEnvelope Envelope;
		Envelope.SchemaId = Target.SchemaId;
		Envelope.SchemaRevision = Target.SchemaRevision;
		Envelope.AdapterContractRevision = Target.AdapterContractRevision;
		Envelope.DataAssetTypeClassPath = Target.DataAssetTypeClassPath;
		Envelope.StableLogicalId = Target.StableLogicalId;
		Envelope.TargetObjectPath = Target.TargetObjectPath;
		Envelope.StagingRelativePath = Target.StagingRelativePath;
		Envelope.bHasBaseSemanticFingerprint = Target.bHasBaseSemanticFingerprint;
		Envelope.BaseSemanticFingerprint = Target.BaseSemanticFingerprint;
		Envelope.CurrentSemanticFingerprint = Target.CurrentSemanticFingerprint;
		Envelope.StagingSemanticFingerprint = Target.StagingSemanticFingerprint;
		Envelope.PlannedOperation = Target.PlannedOperation;
		return Envelope;
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

	// Approval target의 provider-owned exact main_game-relative Staging JSON을 UTF-8 text로 다시 읽습니다.
	bool ReadStagingFile(
		const FCFDATypeProvider& Provider,
		const FString& StagingRelativePath,
		FString& OutJsonText,
		FString& OutError)
	{
		// provider authority로 canonicalized 된 reviewed source path입니다.
		FString NormalizedStagingRelativePath;
		if (!CFDATypeDispatch::NormalizeProviderStagingPath(Provider, StagingRelativePath, NormalizedStagingRelativePath)
			|| !NormalizedStagingRelativePath.Equals(StagingRelativePath, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Reviewed Staging source가 selected provider의 canonical StagingRoot 밖에 있습니다.");
			return false;
		}
		// canonical main_game absolute root입니다.
		const FString MainGameRoot = GetMainGameRoot();
		// approval이 가리키는 exact Staging source absolute path입니다.
		FString StagingAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(MainGameRoot, NormalizedStagingRelativePath));
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

	// Approval target과 payload-free fresh Preview row의 exact evidence binding이 일치하는지 검증합니다.
	bool MatchesApprovalTarget(
		const FCFDAStagingApprovalTarget& ApprovalTarget,
		const FCFDACommonPreviewRow& FreshRow,
		FString& OutError)
	{
		// Immutable approval target을 common envelope로 투영한 reviewed evidence입니다.
		const FCFDACommonEnvelope ApprovalEnvelope = BuildApprovalEnvelope(ApprovalTarget);
		if (FreshRow.Kind != ApprovalTarget.PlannedOperation
			|| FreshRow.Envelope.PlannedOperation != FreshRow.Kind
			|| !CFDATypeDispatch::AreCommonEnvelopesEquivalent(ApprovalEnvelope, FreshRow.Envelope))
		{
			OutError = TEXT("Reviewed approval target evidence와 fresh disk/current common Preview가 exact 일치하지 않습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Fresh parse가 current truth를 읽기 전에 approval의 immutable source/candidate binding과 일치하는지 검증합니다.
	bool MatchesApprovalCandidateBinding(
		const FCFDAStagingApprovalTarget& ApprovalTarget,
		const FCFDACommonEnvelope& FreshCandidate,
		FString& OutError)
	{
		if (!ApprovalTarget.SchemaId.Equals(FreshCandidate.SchemaId, ESearchCase::CaseSensitive)
			|| ApprovalTarget.SchemaRevision != FreshCandidate.SchemaRevision
			|| ApprovalTarget.AdapterContractRevision != FreshCandidate.AdapterContractRevision
			|| !ApprovalTarget.DataAssetTypeClassPath.Equals(FreshCandidate.DataAssetTypeClassPath, ESearchCase::CaseSensitive)
			|| ApprovalTarget.StableLogicalId != FreshCandidate.StableLogicalId
			|| !ApprovalTarget.TargetObjectPath.Equals(FreshCandidate.TargetObjectPath, ESearchCase::CaseSensitive)
			|| !ApprovalTarget.StagingRelativePath.Equals(FreshCandidate.StagingRelativePath, ESearchCase::CaseSensitive)
			|| ApprovalTarget.bHasBaseSemanticFingerprint != FreshCandidate.bHasBaseSemanticFingerprint
			|| !ApprovalTarget.BaseSemanticFingerprint.Equals(FreshCandidate.BaseSemanticFingerprint, ESearchCase::CaseSensitive)
			|| !ApprovalTarget.StagingSemanticFingerprint.Equals(FreshCandidate.StagingSemanticFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Reviewed approval source/candidate binding과 fresh provider parse 결과가 exact 일치하지 않습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Create common candidate가 새 package를 만들기 전에 package 자체도 완전히 absent인지 확인합니다.
	bool ValidateCreatePackageAbsent(const FCFDACommonPreviewRow& Row, FString& OutError)
	{
		if (Row.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError.Reset();
			return true;
		}

		// Exact target package long name입니다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(Row.Envelope.TargetObjectPath);
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

	// Typed whole-record payload를 exact MissileGuidePreset DataAsset에 적용하는 existing production body입니다.
	void ApplyPayloadToAsset(const FCFDAMissilePresetPayload& Payload, UCFMissileGuidePresetData& Asset)
	{
		Asset.PresetId = Payload.PresetId;
		Asset.PresetDisplayName = FText::FromString(Payload.PresetDisplayName.Text);
		Asset.PresetDescription = FText::FromString(Payload.PresetDescription.Text);
		Asset.MissileGuideConfig = Payload.MissileGuideConfig;
	}

	// Fresh global preflight를 approval exact target order로 payload-free common rows로 다시 구축합니다.
	bool BuildFreshRows(
		const FCFDAStagingReviewedApproval& Approval,
		TArray<FCFDACommonPreviewRow>& OutFreshRows,
		FCFDAStagingApplyReport& OutReport)
	{
		OutFreshRows.Reset();
		for (int32 TargetIndex = 0; TargetIndex < Approval.IncludedTargets.Num(); ++TargetIndex)
		{
			// Immutable approval target입니다.
			const FCFDAStagingApprovalTarget& ApprovalTarget = Approval.IncludedTargets[TargetIndex];
			// Matching report row입니다.
			FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[TargetIndex];
			// Reviewed approval의 payload-free common envelope입니다.
			const FCFDACommonEnvelope ApprovalEnvelope = BuildApprovalEnvelope(ApprovalTarget);
			// Preflight failure detail입니다.
			FString PreflightError;
			// Reviewed exact TypeKey에 등록된 complete trusted provider entry입니다.
			const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
				ApprovalTarget.SchemaId,
				ApprovalTarget.DataAssetTypeClassPath,
				&PreflightError);
			if (ProviderEntry == nullptr
				|| !CFDATypeDispatch::ValidateProviderContract(ApprovalEnvelope, ProviderEntry->Descriptor, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError.IsEmpty()
					? TEXT("Reviewed approval의 exact TypeKey provider contract를 확정하지 못했습니다.")
					: PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("TypeKey"), TargetReport.Diagnostic);
				return false;
			}
			if (!CFDATypeDispatch::ValidateProviderMutationReady(*ProviderEntry, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("TypeKey.Readiness"), PreflightError);
				return false;
			}

			// Disk Staging source current text입니다.
			FString JsonText;
			if (!ReadStagingFile(ProviderEntry->Descriptor, ApprovalTarget.StagingRelativePath, JsonText, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("StagingRelativePath"), PreflightError);
				return false;
			}

			// Provider-local typed parse 뒤 shared core에 반환된 fresh payload-free candidate입니다.
			FCFDACommonEnvelope FreshCandidate;
			// Provider parser diagnostics입니다.
			TArray<FCFDAStagingIssue> ParseIssues;
			if (!ProviderEntry->Operations.ParseCommonCandidate(
				JsonText,
				ApprovalTarget.StagingRelativePath,
				FreshCandidate,
				ParseIssues))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = MoveTemp(ParseIssues);
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Staging"), TEXT("Reviewed 이후 provider typed parse/validation truth가 바뀌었습니다."));
				TargetReport.Diagnostic = TEXT("Reviewed 이후 Staging JSON이 selected provider contract를 만족하지 않습니다.");
				return false;
			}
			if (!MatchesApprovalCandidateBinding(ApprovalTarget, FreshCandidate, PreflightError))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Diagnostic = PreflightError;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Staging"), PreflightError);
				return false;
			}

			// Fresh current UE target/identity/package payload-free truth입니다.
			FCFDACommonCurrentState CurrentState;
			// Provider current resolver diagnostics입니다.
			TArray<FCFDAStagingIssue> CurrentIssues;
			if (!ProviderEntry->Operations.ResolveCommonCurrentState(FreshCandidate, CurrentState, CurrentIssues))
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = MoveTemp(CurrentIssues);
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Current"), TEXT("Apply 직전 provider current truth를 확정하지 못했습니다."));
				TargetReport.Diagnostic = TEXT("Apply global preflight provider current resolver가 실패했습니다.");
				return false;
			}

			// Current truth와 disk candidate를 exact shared 3-way로 재분류한 fresh common Preview입니다.
			FCFDACommonPreviewRow FreshRow = CFDATypeDispatch::BuildCommonPreview(FreshCandidate, CurrentState);
			if (FreshRow.Kind == ECFDAStagingPreviewKind::Conflict || FreshRow.Kind == ECFDAStagingPreviewKind::Invalid)
			{
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = FreshRow.Issues;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Preview"), TEXT("Apply 직전 fresh common Preview가 reviewed Create/Update 상태가 아닙니다."));
				TargetReport.Diagnostic = TEXT("Apply global preflight에서 fresh common Preview가 blocker로 전환됐습니다.");
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

		CFDATypeDispatch::ApplyCommonBatchDuplicateValidation(OutFreshRows);
		for (int32 RowIndex = 0; RowIndex < OutFreshRows.Num(); ++RowIndex)
		{
			// Duplicate validation 이후 fresh common row입니다.
			const FCFDACommonPreviewRow& FreshRow = OutFreshRows[RowIndex];
			if (FreshRow.Kind == ECFDAStagingPreviewKind::Conflict || FreshRow.Kind == ECFDAStagingPreviewKind::Invalid)
			{
				FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[RowIndex];
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				TargetReport.Issues = FreshRow.Issues;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Batch"), TEXT("Apply 직전 common batch duplicate validation 결과가 blocker입니다."));
				TargetReport.Diagnostic = TEXT("Apply global common batch preflight duplicate validation이 실패했습니다.");
				return false;
			}
		}

		// Fresh exact common target set의 BatchPlanHash입니다.
		FString FreshBatchPlanHash;
		// Fresh hash generation error입니다.
		FString HashError;
		if (!CFDATypeDispatch::BuildCommonBatchPlanHash(OutFreshRows, FreshBatchPlanHash, HashError)
			|| !FreshBatchPlanHash.Equals(Approval.BatchPlanHash, ESearchCase::CaseSensitive))
		{
			if (!OutReport.Targets.IsEmpty())
			{
				FCFDAStagingTargetApplyReport& TargetReport = OutReport.Targets[0];
				TargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
				AddIssue(TargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("BatchPlanHash"), TEXT("Apply 직전 fresh common BatchPlanHash가 reviewed approval과 다릅니다."));
				TargetReport.Diagnostic = HashError.IsEmpty() ? TEXT("BatchPlanHash stale mismatch입니다.") : HashError;
			}
			return false;
		}
		return true;
	}

	// Global preflight 뒤 각 target mutation 직전에 Staging source + current truth가 reviewed common row와 여전히 같은지 다시 검증합니다.
	bool RevalidateImmediatelyBeforeMutation(
		const FCFDACommonPreviewRow& PlannedRow,
		FString& OutFreshJsonText,
		TArray<FCFDAStagingIssue>& OutIssues,
		FString& OutError)
	{
		OutFreshJsonText.Reset();
		OutIssues.Reset();

		// Planned row exact TypeKey에 등록된 complete provider entry입니다.
		const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
			PlannedRow.Envelope.SchemaId,
			PlannedRow.Envelope.DataAssetTypeClassPath,
			&OutError);
		if (ProviderEntry == nullptr)
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("TypeKey"), OutError);
			return false;
		}
		if (!CFDATypeDispatch::ValidateProviderMutationReady(*ProviderEntry, OutError))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("TypeKey.Readiness"), OutError);
			return false;
		}

		// Mutation 직전 exact Staging source를 다시 읽습니다.
		if (!ReadStagingFile(
			ProviderEntry->Descriptor,
			PlannedRow.Envelope.StagingRelativePath,
			OutFreshJsonText,
			OutError))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("StagingRelativePath"), OutError);
			return false;
		}

		// Provider-local typed parse 뒤 shared core에 반환된 immediate common candidate입니다.
		FCFDACommonEnvelope ImmediateCandidate;
		// Immediate provider parser diagnostics입니다.
		TArray<FCFDAStagingIssue> ParseIssues;
		if (!ProviderEntry->Operations.ParseCommonCandidate(
			OutFreshJsonText,
			PlannedRow.Envelope.StagingRelativePath,
			ImmediateCandidate,
			ParseIssues))
		{
			OutIssues = MoveTemp(ParseIssues);
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Staging"), TEXT("Global preflight 이후 mutation 직전 provider parse truth가 바뀌었습니다."));
			OutError = TEXT("Mutation 직전 provider typed parse가 실패했습니다.");
			return false;
		}

		// Global preflight source/candidate binding을 temporary approval projection으로 동결합니다.
		const FCFDAStagingApprovalTarget PlannedTarget = BuildApprovalTarget(PlannedRow);
		if (!MatchesApprovalCandidateBinding(PlannedTarget, ImmediateCandidate, OutError))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Staging"), OutError);
			return false;
		}

		// Mutation 직전 provider-specific current UE target/identity/package truth입니다.
		FCFDACommonCurrentState CurrentState;
		// Current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!ProviderEntry->Operations.ResolveCommonCurrentState(ImmediateCandidate, CurrentState, CurrentIssues))
		{
			OutIssues = MoveTemp(CurrentIssues);
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Current"), TEXT("Global preflight 이후 mutation 직전 provider current truth를 다시 확정하지 못했습니다."));
			OutError = TEXT("Mutation 직전 provider current resolver가 실패했습니다.");
			return false;
		}

		// Mutation 직전 exact shared 3-way Preview입니다.
		const FCFDACommonPreviewRow ImmediateRow = CFDATypeDispatch::BuildCommonPreview(ImmediateCandidate, CurrentState);
		if (ImmediateRow.Kind == ECFDAStagingPreviewKind::Conflict || ImmediateRow.Kind == ECFDAStagingPreviewKind::Invalid)
		{
			OutIssues = ImmediateRow.Issues;
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Preview"), TEXT("Global preflight 이후 mutation 직전 current state가 blocker로 변했습니다."));
			OutError = TEXT("Mutation 직전 fresh common Preview가 Conflict/Invalid로 변했습니다.");
			return false;
		}
		if (!MatchesApprovalTarget(PlannedTarget, ImmediateRow, OutError))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::ApprovalStale, TEXT("CurrentSemanticFingerprint"), TEXT("Global preflight 이후 mutation 직전 exact common evidence가 바뀌었습니다."));
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

	// Payload-free common row 하나를 exact provider operation entry로 dispatch해 durable terminal result를 반환합니다.
	void ApplyOneTarget(
		const FCFDACommonPreviewRow& Row,
		FCFDAStagingTargetApplyReport& OutReport)
	{
		// Execution row exact TypeKey에 등록된 complete provider entry입니다.
		FString ProviderError;
		const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
			Row.Envelope.SchemaId,
			Row.Envelope.DataAssetTypeClassPath,
			&ProviderError);
		if (ProviderEntry == nullptr
			|| !CFDATypeDispatch::ValidateProviderContract(Row.Envelope, ProviderEntry->Descriptor, ProviderError))
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = ProviderError.IsEmpty()
				? TEXT("Execution common row의 exact provider contract를 확정하지 못했습니다.")
				: ProviderError;
			AddIssue(OutReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("TypeKey"), OutReport.Diagnostic);
			return;
		}
		if (!CFDATypeDispatch::ValidateProviderMutationReady(*ProviderEntry, ProviderError))
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = ProviderError;
			AddIssue(OutReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("TypeKey.Readiness"), ProviderError);
			return;
		}

#if WITH_DEV_AUTOMATION_TESTS
		if (MatchesTestFaultTarget(GForceBeforeMutationBlockTargetPath, Row.Envelope.TargetObjectPath))
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = TEXT("DAS-P0-04 Automation fixture가 global preflight 뒤 mutation 직전 known block을 강제했습니다.");
			return;
		}
#endif

		// Mutation 직전 source/current TOCTOU를 재검증한 exact fresh JSON입니다.
		FString FreshJsonText;
		// Mutation 직전 provider/common preflight 실패 상세입니다.
		FString ImmediatePreflightError;
		if (!RevalidateImmediatelyBeforeMutation(
			Row,
			FreshJsonText,
			OutReport.Issues,
			ImmediatePreflightError))
		{
			OutReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
			OutReport.Diagnostic = ImmediatePreflightError;
			return;
		}

		ProviderEntry->Operations.ApplyReviewedMutation(FreshJsonText, Row, OutReport);
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

// Existing Missile typed materializer production body를 first typed provider callback에 연결합니다.
void CFDAMissileProviderImpl::MaterializePayload(
	UCFMissileGuidePresetData& TargetAsset,
	const FCFDAMissilePresetPayload& Payload)
{
	CFDAStagingApplyPrivate::ApplyPayloadToAsset(Payload, TargetAsset);
}

// Fresh JSON을 provider-local typed payload로 다시 parse한 뒤 exact reviewed common row를 durable Missile mutation에 연결합니다.
void CFDAMissileProviderImpl::ApplyReviewedMutation(
	const FString& JsonText,
	const FCFDACommonPreviewRow& FreshRow,
	FCFDAStagingTargetApplyReport& OutTargetReport)
{
	// Mutation 직전 fresh JSON의 provider-local strict typed parse 결과입니다.
	const FCFDAStagingParseResult ParseResult = ParseJson(JsonText, FreshRow.Envelope.StagingRelativePath);
	if (!ParseResult.bValid)
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		OutTargetReport.Issues = ParseResult.Issues;
		CFDAStagingApplyPrivate::AddIssue(
			OutTargetReport.Issues,
			ECFDAStagingIssueCode::ApprovalStale,
			TEXT("Staging"),
			TEXT("Provider materialize 진입 직전 fresh JSON typed parse가 실패했습니다."));
		OutTargetReport.Diagnostic = TEXT("Missile provider materialize 진입 직전 typed parse가 실패했습니다.");
		return;
	}

	// Provider-local typed parse 결과를 reviewed current/operation binding까지 포함해 common envelope로 재투영합니다.
	const FCFDACommonEnvelope ParsedEnvelope = BuildCommonEnvelope(
		ParseResult.Record,
		FreshRow.Envelope.CurrentSemanticFingerprint,
		FreshRow.Kind);
	if (!CFDATypeDispatch::AreCommonEnvelopesEquivalent(ParsedEnvelope, FreshRow.Envelope))
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		CFDAStagingApplyPrivate::AddIssue(
			OutTargetReport.Issues,
			ECFDAStagingIssueCode::ApprovalStale,
			TEXT("Staging"),
			TEXT("Provider-local typed parse 결과가 reviewed common execution row와 exact 일치하지 않습니다."));
		OutTargetReport.Diagnostic = TEXT("Provider-local typed execution binding mismatch입니다.");
		return;
	}

	// Missile provider-local typed payload를 provider-neutral durable sequencing에 전달합니다.
	CFDADurableCore::ApplyTypedTarget<UCFMissileGuidePresetData, FCFDAMissilePresetPayload>(
		FreshRow,
		ParseResult.Record.Payload,
		&CFDAMissileProviderImpl::MaterializePayload,
		&CFDAMissileProviderImpl::ExtractPayload,
		&CFDAMissileProviderImpl::BuildSemanticFingerprint,
		OutTargetReport);
}

#if WITH_DEV_AUTOMATION_TESTS
// 모든 test-only fault injection state를 초기화합니다.
void FCFDAStagingApplyTestControl::Reset()
{
	CFDADurableCore::ResetTestFaults();
	CFDAStagingApplyPrivate::GForceBeforeMutationBlockTargetPath.Reset();
}

// exact target의 SavePackage 호출 시점을 outcome-unknown failure로 강제합니다.
void FCFDAStagingApplyTestControl::ForceSaveFailure(const FString& TargetObjectPath)
{
	CFDADurableCore::SetForceSaveFailureTarget(TargetObjectPath);
}

// exact target의 SavePackage 성공 뒤 persisted confirmation 실패를 강제합니다.
void FCFDAStagingApplyTestControl::ForceConfirmationFailure(const FString& TargetObjectPath)
{
	CFDADurableCore::SetForceConfirmationFailureTarget(TargetObjectPath);
}

// exact target의 global preflight 뒤 mutation 직전 known block을 강제합니다.
void FCFDAStagingApplyTestControl::ForceBeforeMutationBlock(const FString& TargetObjectPath)
{
	CFDAStagingApplyPrivate::GForceBeforeMutationBlockTargetPath = TargetObjectPath;
}
#endif

// Payload-free common Preview set을 deterministic Reviewed approval evidence로 동결합니다.
bool CFDATypeDispatch::BuildCommonReviewedApproval(
	const TArray<FCFDACommonPreviewRow>& PreviewRows,
	FCFDAStagingReviewedApproval& OutApproval,
	FString& OutError)
{
	OutApproval = FCFDAStagingReviewedApproval();
	OutError.Reset();

	// Duplicate validation을 포함해 approval 가능한 payload-free common preview copy입니다.
	TArray<FCFDACommonPreviewRow> ValidatedRows = PreviewRows;
	ApplyCommonBatchDuplicateValidation(ValidatedRows);
	for (const FCFDACommonPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Conflict || Row.Kind == ECFDAStagingPreviewKind::Invalid)
		{
			OutError = TEXT("Conflict/Invalid가 포함된 common Preview set은 Reviewed approval로 승격할 수 없습니다.");
			return false;
		}
	}

	// Exact reviewed common mutation set의 batch plan hash입니다.
	FString BatchPlanHash;
	if (!BuildCommonBatchPlanHash(ValidatedRows, BatchPlanHash, OutError))
	{
		return false;
	}

	for (const FCFDACommonPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Create || Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			OutApproval.IncludedTargets.Add(CFDAStagingApplyPrivate::BuildApprovalTarget(Row));
		}
	}
	if (OutApproval.IncludedTargets.IsEmpty())
	{
		OutError = TEXT("Reviewed approval에는 Create/Update common mutation candidate가 하나 이상 필요합니다.");
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

// Existing Public Missile Preview set을 provider-local integrity projection 뒤 common Reviewed approval authority로 전달하는 compatibility facade입니다.
bool FCFDAStagingApplyService::BuildReviewedApproval(
	const TArray<FCFDAStagingPreviewRow>& PreviewRows,
	FCFDAStagingReviewedApproval& OutApproval,
	FString& OutError)
{
	OutApproval = FCFDAStagingReviewedApproval();
	OutError.Reset();

	// Shared Review authority에 전달할 payload-free common rows입니다.
	TArray<FCFDACommonPreviewRow> CommonRows;
	CommonRows.Reserve(PreviewRows.Num());
	for (const FCFDAStagingPreviewRow& PreviewRow : PreviewRows)
	{
		// Provider-local typed mutable integrity 검증 뒤 반환된 common row입니다.
		FCFDACommonPreviewRow CommonRow;
		if (!CFDAMissileProviderImpl::ProjectCompatibilityPreviewRow(PreviewRow, CommonRow, OutError))
		{
			return false;
		}
		CommonRows.Add(MoveTemp(CommonRow));
	}

	return CFDATypeDispatch::BuildCommonReviewedApproval(CommonRows, OutApproval, OutError);
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

	// Disk Staging + fresh current truth로 다시 만든 payload-free exact mutation rows입니다.
	TArray<FCFDACommonPreviewRow> FreshRows;
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
		// Deterministic payload-free execution row입니다.
		const FCFDACommonPreviewRow& FreshRow = FreshRows[RowIndex];
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
