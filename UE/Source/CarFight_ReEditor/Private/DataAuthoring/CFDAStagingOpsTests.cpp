// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingOpsTests.cpp
// Version: v1.1.0
// Date: 2026-09-09
// Description: CF-FQ-049 DAS-P0-05 selected Preview→Review state machine, Product mutation0와 Sync partial-write rollback Automation입니다.
// Changelog:
// - v1.1.0: unselected-invalid isolation, selected Update Preview→stale Review reject→fresh Review PASS, Apply 0과 forced partial-write exact-byte rollback을 검증.
// - v1.0.0: console entry 등록, Product Sync idempotence, revision-2 canonical JSON, Product NoChange discovery와 Product semantic/package mutation 0을 검증.
// Migration:
// - 이 Automation은 Product Low/Normal/High .uasset을 수정하거나 SavePackage하지 않습니다.
// - test 중 canonical Staging JSON text만 일시 변경하며 RAII raw-byte restore guard가 test exit에서 호출 전 bytes를 복원합니다.
// - durable Apply/Save safety는 기존 DAS-P0-03/04 exact13이 계속 소유하며 이 test는 ApplyReviewed를 실행하지 않습니다.

#include "DataAuthoring/CFDAStagingOps.h"

#include "CFMissileGuidePresetData.h"
#include "Dom/JsonObject.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAStagingOpsTestsPrivate
{
	// Product Pilot 한 건의 expected immutable identity/path입니다.
	struct FProductExpectation
	{
		// expected StableLogicalId입니다.
		const TCHAR* StableLogicalId = TEXT("");

		// persisted Product exact object path입니다.
		const TCHAR* TargetObjectPath = TEXT("");
	};

	// test 시작 전 Product semantic/package state snapshot입니다.
	struct FProductSnapshot
	{
		// exact Product target path입니다.
		FString TargetObjectPath;

		// test 시작 전 semantic fingerprint입니다.
		FString SemanticFingerprint;
	};

	// test exit에서 exact Staging raw bytes를 호출 전 상태로 복원하는 guard입니다.
	struct FStagingRestoreGuard
	{
		// physical file path별 호출 전 exact raw bytes입니다.
		TMap<FString, TArray<uint8>> OriginalBytesByPath;

		// exact restore 대상 file의 호출 전 raw bytes를 캡처합니다.
		bool Capture(const FString& AbsolutePath)
		{
			// capture할 exact file bytes입니다.
			TArray<uint8> FileBytes;
			if (!FFileHelper::LoadFileToArray(FileBytes, *AbsolutePath))
			{
				return false;
			}
			OriginalBytesByPath.Add(AbsolutePath, MoveTemp(FileBytes));
			return true;
		}

		// test exit에서 test-only Staging text mutation과 fault state를 best-effort 정리합니다.
		~FStagingRestoreGuard()
		{
			FCFDAStagingOpsTestControl::Reset();
			for (const TPair<FString, TArray<uint8>>& Entry : OriginalBytesByPath)
			{
				FFileHelper::SaveArrayToFile(Entry.Value, *Entry.Key);
			}
		}
	};

	// DAS-P0-05 Product Pilot exact Low/Normal/High expectations를 반환합니다.
	const TArray<FProductExpectation>& GetProductExpectations()
	{
		// exact Product Pilot 3종입니다.
		static const TArray<FProductExpectation> ProductExpectations =
		{
			{TEXT("MissileFeel_Low"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low")},
			{TEXT("MissileFeel_Normal"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Normal.DA_MissileFeel_Normal")},
			{TEXT("MissileFeel_High"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_High.DA_MissileFeel_High")}
		};
		return ProductExpectations;
	}

	// CarFight main_game root를 `<main_game>/UE/` ProjectDir의 부모로 resolve합니다.
	FString GetMainGameRoot()
	{
		// CarFight .uproject가 위치한 `<main_game>/UE/` directory입니다.
		const FString UnrealProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		// canonical Staging root가 위치한 `<main_game>/` directory입니다.
		FString MainGameRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(UnrealProjectRoot, TEXT("..")));
		FPaths::NormalizeDirectoryName(MainGameRoot);
		return MainGameRoot;
	}

	// canonical Product Staging main_game-relative path를 만듭니다.
	FString BuildProductStagingPath(const FString& StableLogicalId)
	{
		return FString::Printf(
			TEXT("%s/%s.json"),
			FCFDAStagingOps::GetMissilePresetStagingRoot(),
			*StableLogicalId);
	}

	// main_game-relative Staging path를 physical absolute path로 변환합니다.
	FString BuildStagingAbsolutePath(const FString& StagingRelativePath)
	{
		// physical absolute Staging path입니다.
		FString StagingAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(GetMainGameRoot(), StagingRelativePath));
		FPaths::NormalizeFilename(StagingAbsolutePath);
		return StagingAbsolutePath;
	}

	// exact file raw bytes를 읽습니다.
	bool ReadFileBytes(const FString& AbsolutePath, TArray<uint8>& OutBytes)
	{
		OutBytes.Reset();
		return FFileHelper::LoadFileToArray(OutBytes, *AbsolutePath);
	}

	// exact Product asset의 clean semantic state를 snapshot합니다.
	bool CaptureProductSnapshot(
		const FProductExpectation& ProductExpectation,
		FProductSnapshot& OutSnapshot,
		FString& OutError)
	{
		OutSnapshot = FProductSnapshot();
		// exact Product DataAsset입니다.
		UCFMissileGuidePresetData* ProductAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, ProductExpectation.TargetObjectPath);
		if (ProductAsset == nullptr)
		{
			OutError = FString::Printf(TEXT("Product asset load 실패: %s"), ProductExpectation.TargetObjectPath);
			return false;
		}
		// Product owning package입니다.
		UPackage* ProductPackage = ProductAsset->GetOutermost();
		if (ProductPackage == nullptr || ProductPackage->IsDirty())
		{
			OutError = FString::Printf(TEXT("Product package가 test 시작 전에 dirty입니다: %s"), ProductExpectation.TargetObjectPath);
			return false;
		}
		if (ProductAsset->PresetId != FName(ProductExpectation.StableLogicalId))
		{
			OutError = FString::Printf(TEXT("Product PresetId mismatch: %s"), ProductExpectation.TargetObjectPath);
			return false;
		}

		// exact Product typed payload입니다.
		FCFDAMissilePresetPayload Payload;
		// typed extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!FCFDAStagingService::ExtractMissilePresetPayload(*ProductAsset, Payload, ExtractIssues))
		{
			OutError = TEXT("Product typed payload extraction에 실패했습니다.");
			return false;
		}
		// exact Product semantic fingerprint입니다.
		FString SemanticFingerprint;
		// fingerprint generation error입니다.
		FString FingerprintError;
		if (!FCFDAStagingService::BuildSemanticFingerprint(Payload, SemanticFingerprint, FingerprintError))
		{
			OutError = FingerprintError;
			return false;
		}

		OutSnapshot.TargetObjectPath = ProductExpectation.TargetObjectPath;
		OutSnapshot.SemanticFingerprint = MoveTemp(SemanticFingerprint);
		OutError.Reset();
		return true;
	}

	// Product asset이 test 전 snapshot과 semantic/package exact 동일한지 검증합니다.
	bool VerifyProductSnapshotUnchanged(
		const FProductSnapshot& Snapshot,
		FString& OutError)
	{
		// test 뒤 exact Product DataAsset입니다.
		UCFMissileGuidePresetData* ProductAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, *Snapshot.TargetObjectPath);
		if (ProductAsset == nullptr)
		{
			OutError = FString::Printf(TEXT("Product readback load 실패: %s"), *Snapshot.TargetObjectPath);
			return false;
		}
		// test 뒤 owning package입니다.
		UPackage* ProductPackage = ProductAsset->GetOutermost();
		if (ProductPackage == nullptr || ProductPackage->IsDirty())
		{
			OutError = FString::Printf(TEXT("Product package가 mutation0 workflow 뒤 dirty입니다: %s"), *Snapshot.TargetObjectPath);
			return false;
		}
		// test 뒤 exact Product typed payload입니다.
		FCFDAMissilePresetPayload Payload;
		// typed extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!FCFDAStagingService::ExtractMissilePresetPayload(*ProductAsset, Payload, ExtractIssues))
		{
			OutError = TEXT("Product readback typed payload extraction에 실패했습니다.");
			return false;
		}
		// test 뒤 semantic fingerprint입니다.
		FString SemanticFingerprint;
		// fingerprint generation error입니다.
		FString FingerprintError;
		if (!FCFDAStagingService::BuildSemanticFingerprint(Payload, SemanticFingerprint, FingerprintError))
		{
			OutError = FingerprintError;
			return false;
		}
		if (!SemanticFingerprint.Equals(Snapshot.SemanticFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = FString::Printf(TEXT("Product semantic fingerprint가 mutation0 workflow에서 바뀌었습니다: %s"), *Snapshot.TargetObjectPath);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// canonical Staging JSON의 PresetDescription Literal text에 suffix를 추가해 valid Update candidate를 만듭니다.
	bool AppendDescriptionSuffix(
		const FString& AbsolutePath,
		const FString& Suffix,
		FString& OutError)
	{
		// source JSON text입니다.
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *AbsolutePath))
		{
			OutError = TEXT("Staging JSON read 실패");
			return false;
		}
		// source JSON reader입니다.
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonText);
		// mutable root JSON object입니다.
		TSharedPtr<FJsonObject> RootObject;
		if (!FJsonSerializer::Deserialize(JsonReader, RootObject) || !RootObject.IsValid())
		{
			OutError = TEXT("Staging JSON deserialize 실패");
			return false;
		}
		// mutable Payload object입니다.
		const TSharedPtr<FJsonObject> PayloadObject = RootObject->GetObjectField(TEXT("Payload"));
		// mutable PresetDescription object입니다.
		const TSharedPtr<FJsonObject> DescriptionObject = PayloadObject->GetObjectField(TEXT("PresetDescription"));
		// original description text입니다.
		const FString OriginalText = DescriptionObject->GetStringField(TEXT("Text"));
		DescriptionObject->SetStringField(TEXT("Text"), OriginalText + Suffix);

		// modified valid JSON text입니다.
		FString ModifiedJsonText;
		// deterministic JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ModifiedJsonText);
		if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter))
		{
			OutError = TEXT("modified Staging JSON serialize 실패");
			return false;
		}
		JsonWriter->Close();
		ModifiedJsonText += LINE_TERMINATOR;
		if (!FFileHelper::SaveStringToFile(ModifiedJsonText, *AbsolutePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			OutError = TEXT("modified Staging JSON write 실패");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// exact raw bytes를 file에 복원합니다.
	bool RestoreFileBytes(const FString& AbsolutePath, const TArray<uint8>& Bytes)
	{
		return FFileHelper::SaveArrayToFile(Bytes, *AbsolutePath);
	}

	// valid JSON의 semantic은 유지하고 trailing whitespace만 추가해 Sync canonical rewrite candidate를 만듭니다.
	bool AppendWhitespace(const FString& AbsolutePath)
	{
		// current JSON text입니다.
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *AbsolutePath))
		{
			return false;
		}
		JsonText += LINE_TERMINATOR;
		return FFileHelper::SaveStringToFile(JsonText, *AbsolutePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingOperationalEntryTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_05.OperationalEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product Save 0에서 selection-bound Preview→Review state machine과 Sync rollback을 검증합니다.
bool FCFDAStagingOperationalEntryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingOpsTestsPrivate;

	// reusable console operation entry 4종입니다.
	const TCHAR* ConsoleCommandNames[] =
	{
		TEXT("CarFight.DAStaging.SyncProduct"),
		TEXT("CarFight.DAStaging.Preview"),
		TEXT("CarFight.DAStaging.Review"),
		TEXT("CarFight.DAStaging.ApplyReviewed")
	};
	for (const TCHAR* ConsoleCommandName : ConsoleCommandNames)
	{
		TestNotNull(
			FString::Printf(TEXT("console entry registered: %s"), ConsoleCommandName),
			IConsoleManager::Get().FindConsoleObject(ConsoleCommandName));
	}

	// Product mutation0 검증을 위한 test-start snapshots입니다.
	TArray<FProductSnapshot> ProductSnapshots;
	for (const FProductExpectation& ProductExpectation : GetProductExpectations())
	{
		// exact Product test-start snapshot입니다.
		FProductSnapshot Snapshot;
		// Product snapshot error입니다.
		FString SnapshotError;
		if (!TestTrue(TEXT("Product pre-Sync semantic snapshot succeeds"), CaptureProductSnapshot(ProductExpectation, Snapshot, SnapshotError)))
		{
			AddError(SnapshotError);
			return false;
		}
		ProductSnapshots.Add(MoveTemp(Snapshot));
	}

	// Product 3종 canonical relative paths입니다.
	TArray<FString> ProductStagingRelativePaths;
	// Product 3종 physical paths입니다.
	TArray<FString> ProductStagingAbsolutePaths;
	for (const FProductExpectation& ProductExpectation : GetProductExpectations())
	{
		// exact Product stable id입니다.
		const FString StableLogicalId(ProductExpectation.StableLogicalId);
		// exact canonical relative path입니다.
		const FString RelativePath = BuildProductStagingPath(StableLogicalId);
		ProductStagingRelativePaths.Add(RelativePath);
		ProductStagingAbsolutePaths.Add(BuildStagingAbsolutePath(RelativePath));
	}

	// test exit에서 호출 전 JSON bytes를 복원하는 guard입니다.
	FStagingRestoreGuard RestoreGuard;
	for (const FString& AbsolutePath : ProductStagingAbsolutePaths)
	{
		if (!TestTrue(TEXT("capture pre-test Staging raw bytes"), RestoreGuard.Capture(AbsolutePath)))
		{
			return false;
		}
	}

	// current canonical baseline으로 Sync한 changed paths입니다.
	TArray<FString> InitialChangedPaths;
	// operational failure detail입니다.
	FString OperationError;
	if (!TestTrue(TEXT("initial Product canonical Staging Sync succeeds"), FCFDAStagingOps::SyncProductStaging(InitialChangedPaths, OperationError)))
	{
		AddError(OperationError);
		return false;
	}

	// Sync 직후 canonical raw bytes입니다.
	TArray<TArray<uint8>> CanonicalBytes;
	for (const FString& AbsolutePath : ProductStagingAbsolutePaths)
	{
		// 한 canonical file raw bytes입니다.
		TArray<uint8> FileBytes;
		if (!TestTrue(TEXT("read canonical Staging bytes"), ReadFileBytes(AbsolutePath, FileBytes)))
		{
			return false;
		}
		CanonicalBytes.Add(MoveTemp(FileBytes));
	}

	// 전체 canonical Product discovery Preview입니다.
	FCFDAStagingOpsPreview AllPreview;
	if (!TestTrue(TEXT("all Product discovery Preview succeeds"), FCFDAStagingOps::DiscoverMissilePresetPreview(AllPreview, OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestEqual(TEXT("all Product discovery exact row count"), AllPreview.Rows.Num(), 3);
	TestEqual(TEXT("all Product discovery NoChange count"), AllPreview.NoChangeCount, 3);
	TestFalse(TEXT("all Product discovery has no blocker"), AllPreview.bBlocked);

	// selected orchestration 대상 Low relative path입니다.
	const FString LowRelativePath = ProductStagingRelativePaths[0];
	// selected orchestration 대상 Low physical path입니다.
	const FString LowAbsolutePath = ProductStagingAbsolutePaths[0];
	// unselected invalid isolation 대상 Normal relative path입니다.
	const FString NormalRelativePath = ProductStagingRelativePaths[1];
	// unselected invalid isolation 대상 Normal physical path입니다.
	const FString NormalAbsolutePath = ProductStagingAbsolutePaths[1];

	if (!TestTrue(TEXT("make selected Low valid Update candidate"), AppendDescriptionSuffix(LowAbsolutePath, TEXT(" [DAS-P0-05 Selection]"), OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	if (!TestTrue(TEXT("make unselected Normal invalid"), FFileHelper::SaveStringToFile(TEXT("{ invalid"), *NormalAbsolutePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)))
	{
		return false;
	}

	// actual reusable Preview→Review operational session입니다.
	FCFDAStagingOpsSession OpsSession;
	// selected Low exact-list입니다.
	const TArray<FString> LowSelection = {LowRelativePath};
	// selected Low Preview입니다.
	FCFDAStagingOpsPreview SelectedPreview;
	if (!TestTrue(TEXT("selected Low Preview ignores unselected invalid Normal"), OpsSession.Preview(LowSelection, SelectedPreview, OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestEqual(TEXT("selected Preview exact row count"), SelectedPreview.Rows.Num(), 1);
	TestEqual(TEXT("selected Preview Update count"), SelectedPreview.UpdateCount, 1);
	TestEqual(TEXT("selected Preview Invalid count"), SelectedPreview.InvalidCount, 0);
	TestFalse(TEXT("selected Preview not blocked by unselected invalid file"), SelectedPreview.bBlocked);
	TestTrue(TEXT("selected mutation Preview has BatchPlanHash"), !SelectedPreview.BatchPlanHash.IsEmpty());

	// unselected invalid Normal 자체의 isolated Preview입니다.
	FCFDAStagingOpsPreview InvalidNormalPreview;
	const TArray<FString> NormalSelection = {NormalRelativePath};
	if (!TestTrue(TEXT("isolated invalid Normal discovery returns projection"), FCFDAStagingOps::DiscoverMissilePresetPreview(NormalSelection, InvalidNormalPreview, OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestEqual(TEXT("isolated invalid Normal row count"), InvalidNormalPreview.Rows.Num(), 1);
	TestEqual(TEXT("isolated invalid Normal Invalid count"), InvalidNormalPreview.InvalidCount, 1);
	TestTrue(TEXT("isolated invalid Normal is blocked"), InvalidNormalPreview.bBlocked);

	if (!TestTrue(TEXT("change selected Low after Preview to force stale Review"), AppendDescriptionSuffix(LowAbsolutePath, TEXT(" [ReviewDrift]"), OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestFalse(TEXT("Review rejects stale selected Preview hash"), OpsSession.Review(OperationError));
	TestFalse(TEXT("stale Review creates no approval"), OpsSession.HasReviewedApproval());

	// drift 반영 뒤 fresh selected Preview입니다.
	FCFDAStagingOpsPreview FreshSelectedPreview;
	if (!TestTrue(TEXT("fresh selected Preview after drift succeeds"), OpsSession.Preview(LowSelection, FreshSelectedPreview, OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestEqual(TEXT("fresh selected Preview remains one Update"), FreshSelectedPreview.UpdateCount, 1);
	if (!TestTrue(TEXT("fresh same-selection Review succeeds"), OpsSession.Review(OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestTrue(TEXT("Review creates actual Reviewed approval"), OpsSession.HasReviewedApproval());
	TestEqual(TEXT("Reviewed approval exact selected target count"), OpsSession.GetReviewedApproval().IncludedTargets.Num(), 1);
	if (!OpsSession.GetReviewedApproval().IncludedTargets.IsEmpty())
	{
		TestEqual(TEXT("Reviewed target is selected Low only"), OpsSession.GetReviewedApproval().IncludedTargets[0].StagingRelativePath, LowRelativePath);
	}
	// Product Save 0을 유지하기 위해 ApplyReviewed를 실행하지 않고 session을 폐기합니다.
	OpsSession.Reset();

	// atomicity test 전에 Low/Normal을 exact canonical bytes로 복원합니다.
	TestTrue(TEXT("restore Low canonical bytes before rollback test"), RestoreFileBytes(LowAbsolutePath, CanonicalBytes[0]));
	TestTrue(TEXT("restore Normal canonical bytes before rollback test"), RestoreFileBytes(NormalAbsolutePath, CanonicalBytes[1]));

	// 모든 Product JSON을 semantic NoChange지만 textual rewrite가 필요한 상태로 만듭니다.
	for (const FString& AbsolutePath : ProductStagingAbsolutePaths)
	{
		TestTrue(TEXT("append semantic-neutral whitespace"), AppendWhitespace(AbsolutePath));
	}

	// forced failure 직전 raw bytes입니다.
	TArray<TArray<uint8>> PreFailureBytes;
	for (const FString& AbsolutePath : ProductStagingAbsolutePaths)
	{
		// 한 pre-failure file raw bytes입니다.
		TArray<uint8> FileBytes;
		if (!TestTrue(TEXT("capture pre-failure raw bytes"), ReadFileBytes(AbsolutePath, FileBytes)))
		{
			return false;
		}
		PreFailureBytes.Add(MoveTemp(FileBytes));
	}

	FCFDAStagingOpsTestControl::ForceSyncFailureAfterWrite(1);
	// forced partial write에서 changed paths를 반환하면 안 됩니다.
	TArray<FString> FailedSyncChangedPaths;
	TestFalse(TEXT("forced write-after-success failure returns false"), FCFDAStagingOps::SyncProductStaging(FailedSyncChangedPaths, OperationError));
	TestTrue(TEXT("forced failure reports rollback confirmed"), OperationError.Contains(TEXT("rollback confirmed"), ESearchCase::CaseSensitive));
	TestEqual(TEXT("failed Sync reports no committed changed paths"), FailedSyncChangedPaths.Num(), 0);

	for (int32 FileIndex = 0; FileIndex < ProductStagingAbsolutePaths.Num(); ++FileIndex)
	{
		// forced failure 뒤 rollback readback bytes입니다.
		TArray<uint8> RolledBackBytes;
		if (!TestTrue(TEXT("read rollback bytes"), ReadFileBytes(ProductStagingAbsolutePaths[FileIndex], RolledBackBytes)))
		{
			return false;
		}
		TestTrue(TEXT("partial Sync rollback restores exact pre-call bytes"), RolledBackBytes == PreFailureBytes[FileIndex]);
	}
	FCFDAStagingOpsTestControl::Reset();

	// fault 제거 뒤 same semantic NoChange files를 canonical text로 정상 rewrite합니다.
	TArray<FString> SuccessfulRewritePaths;
	if (!TestTrue(TEXT("Sync succeeds after fault reset"), FCFDAStagingOps::SyncProductStaging(SuccessfulRewritePaths, OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestEqual(TEXT("successful canonical rewrite changes exact Product 3 files"), SuccessfulRewritePaths.Num(), 3);
	for (int32 FileIndex = 0; FileIndex < ProductStagingAbsolutePaths.Num(); ++FileIndex)
	{
		// successful rewrite readback bytes입니다.
		TArray<uint8> RewrittenBytes;
		if (!TestTrue(TEXT("read successful rewrite bytes"), ReadFileBytes(ProductStagingAbsolutePaths[FileIndex], RewrittenBytes)))
		{
			return false;
		}
		TestTrue(TEXT("successful rewrite returns canonical bytes"), RewrittenBytes == CanonicalBytes[FileIndex]);
	}

	// convergence 뒤 두 번째 Sync idempotence를 확인합니다.
	TArray<FString> SecondChangedPaths;
	if (!TestTrue(TEXT("second converged Product Staging Sync succeeds"), FCFDAStagingOps::SyncProductStaging(SecondChangedPaths, OperationError)))
	{
		AddError(OperationError);
		return false;
	}
	TestEqual(TEXT("second converged Sync is idempotent"), SecondChangedPaths.Num(), 0);

	for (const FProductSnapshot& Snapshot : ProductSnapshots)
	{
		// Product mutation0 post-check error입니다.
		FString ProductCheckError;
		if (!TestTrue(TEXT("Product remains semantically unchanged and clean"), VerifyProductSnapshotUnchanged(Snapshot, ProductCheckError)))
		{
			AddError(ProductCheckError);
		}
	}

	return !HasAnyErrors();
}

#endif
