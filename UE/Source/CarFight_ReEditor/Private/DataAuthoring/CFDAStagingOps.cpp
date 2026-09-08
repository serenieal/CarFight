// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingOps.cpp
// Version: v1.1.1
// Date: 2026-09-09
// Description: CF-FQ-049 DAS-P0-05 Product Staging safe sync, exact selection discovery와 Preview→Review→ApplyReviewed operational session 구현입니다.
// Changelog:
// - v1.1.1: UE 5.8 console registration API에 맞춰 args delegate를 FAutoConsoleCommand overload로 교정.
// - v1.1.0: exact selected Preview, selection-bound fresh Review session, console Preview args와 Sync partial-write exact-byte rollback을 추가.
// - v1.0.0: Low/Normal/High Product converged-only Staging Sync, canonical discovery Preview/hash와 console operation entry를 추가.
// Migration:
// - `CarFight.DAStaging.Preview`는 인수가 없으면 전체 canonical folder를, 인수가 있으면 StableLogicalId 또는 canonical relative JSON path exact-list만 Preview합니다.
// - Review는 마지막 Preview의 same selection만 fresh re-discovery/hash 검증하고, ApplyReviewed는 그 Reviewed one-shot approval만 전달합니다.
// - SyncProduct는 Product .uasset을 저장하지 않으며 write/verification 실패 시 이번 호출에서 touched된 Staging 파일을 호출 전 raw bytes/absence로 rollback합니다.

#include "DataAuthoring/CFDAStagingOps.h"

#include "CFMissileGuidePresetData.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFDAStagingOps, Log, All);

namespace CFDAStagingOpsPrivate
{
	// P0 MissileGuidePreset canonical main_game-relative Staging directory입니다.
	static constexpr TCHAR MissilePresetStagingRoot[] = TEXT("Authoring/DataAssetStaging/MissileGuidePreset");

#if WITH_DEV_AUTOMATION_TESTS
	// zero-based changed-file write ordinal에서 write 성공 뒤 failure를 강제하는 test-only 값입니다.
	int32 GForceSyncFailureAfterWriteOrdinal = INDEX_NONE;
#endif

	// Product Pilot 한 건의 immutable stable identity와 target path입니다.
	struct FProductTarget
	{
		// Product stable logical identity입니다.
		const TCHAR* StableLogicalId = TEXT("");

		// persisted Product DataAsset exact object path입니다.
		const TCHAR* TargetObjectPath = TEXT("");
	};

	// Product Sync preflight가 확정한 한 JSON write 계획입니다.
	struct FPlannedProductWrite
	{
		// exact Product target object path입니다.
		FString TargetObjectPath;

		// Sync 시작 시 Product semantic fingerprint입니다.
		FString OriginalFingerprint;

		// main_game-relative canonical Staging JSON path입니다.
		FString StagingRelativePath;

		// physical canonical Staging JSON path입니다.
		FString StagingAbsolutePath;

		// current Product를 표현하는 revision-2 canonical JSON text입니다.
		FString JsonText;

		// 호출 전 Staging file이 존재했는지 나타냅니다.
		bool bOriginalFileExisted = false;

		// 호출 전 Staging file raw bytes입니다.
		TArray<uint8> OriginalFileBytes;

		// disk text가 달라 실제 JSON write가 필요한지 나타냅니다.
		bool bNeedsWrite = false;
	};

	// DAS-P0-05 Product Pilot exact Low/Normal/High target 집합을 반환합니다.
	const TArray<FProductTarget>& GetProductTargets()
	{
		// Product Pilot exact 3종 immutable target 목록입니다.
		static const TArray<FProductTarget> ProductTargets =
		{
			{TEXT("MissileFeel_Low"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low")},
			{TEXT("MissileFeel_Normal"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Normal.DA_MissileFeel_Normal")},
			{TEXT("MissileFeel_High"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_High.DA_MissileFeel_High")}
		};
		return ProductTargets;
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

	// main_game root 문자열을 slash-normalized prefix 비교용 형태로 반환합니다.
	FString GetMainGameRootPrefix()
	{
		// canonical main_game root입니다.
		FString MainGameRootPrefix = GetMainGameRoot();
		FPaths::NormalizeFilename(MainGameRootPrefix);
		if (!MainGameRootPrefix.EndsWith(TEXT("/")))
		{
			MainGameRootPrefix += TEXT("/");
		}
		return MainGameRootPrefix;
	}

	// canonical main_game-relative Staging path를 containment 검증한 absolute path로 변환합니다.
	bool ResolveStagingAbsolutePath(
		const FString& StagingRelativePath,
		FString& OutAbsolutePath,
		FString& OutError)
	{
		// slash-normalized source path입니다.
		FString NormalizedRelativePath = StagingRelativePath;
		FPaths::NormalizeFilename(NormalizedRelativePath);
		if (!FPaths::IsRelative(NormalizedRelativePath)
			|| NormalizedRelativePath.Contains(TEXT(".."), ESearchCase::CaseSensitive)
			|| !NormalizedRelativePath.StartsWith(FString(MissilePresetStagingRoot) + TEXT("/"), ESearchCase::CaseSensitive)
			|| !NormalizedRelativePath.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
		{
			OutAbsolutePath.Reset();
			OutError = FString::Printf(TEXT("canonical MissileGuidePreset Staging path가 아닙니다: %s"), *StagingRelativePath);
			return false;
		}

		// canonical main_game absolute root입니다.
		const FString MainGameRoot = GetMainGameRoot();
		// requested staging absolute path입니다.
		FString AbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(MainGameRoot, NormalizedRelativePath));
		FPaths::NormalizeFilename(AbsolutePath);
		// containment 비교용 main_game prefix입니다.
		const FString MainGameRootPrefix = GetMainGameRootPrefix();
		if (!AbsolutePath.StartsWith(MainGameRootPrefix, ESearchCase::IgnoreCase))
		{
			OutAbsolutePath.Reset();
			OutError = TEXT("Staging absolute path가 main_game root 밖으로 벗어났습니다.");
			return false;
		}

		OutAbsolutePath = MoveTemp(AbsolutePath);
		OutError.Reset();
		return true;
	}

	// main_game 아래 absolute filename을 normalized repository-relative path로 변환합니다.
	bool MakeMainGameRelativePath(
		const FString& AbsolutePath,
		FString& OutRelativePath,
		FString& OutError)
	{
		// slash-normalized absolute filename입니다.
		FString NormalizedAbsolutePath = FPaths::ConvertRelativePathToFull(AbsolutePath);
		FPaths::NormalizeFilename(NormalizedAbsolutePath);
		// containment 비교용 main_game prefix입니다.
		const FString MainGameRootPrefix = GetMainGameRootPrefix();
		if (!NormalizedAbsolutePath.StartsWith(MainGameRootPrefix, ESearchCase::IgnoreCase))
		{
			OutRelativePath.Reset();
			OutError = TEXT("발견한 Staging file이 main_game root 밖에 있습니다.");
			return false;
		}

		OutRelativePath = NormalizedAbsolutePath.Mid(MainGameRootPrefix.Len());
		FPaths::NormalizeFilename(OutRelativePath);
		OutError.Reset();
		return true;
	}

	// exact selected relative path 목록을 canonical validation/dedupe/sort합니다.
	bool NormalizeSelectedPaths(
		const TArray<FString>& SelectedStagingRelativePaths,
		TArray<FString>& OutNormalizedPaths,
		FString& OutError)
	{
		OutNormalizedPaths.Reset();
		// exact duplicate를 차단할 selected path 집합입니다.
		TSet<FString> SeenPaths;
		for (const FString& SelectedPath : SelectedStagingRelativePaths)
		{
			// slash-normalized selected relative path입니다.
			FString NormalizedPath = SelectedPath;
			FPaths::NormalizeFilename(NormalizedPath);
			// validation 용도로 resolve한 exact absolute path입니다.
			FString AbsolutePath;
			if (!ResolveStagingAbsolutePath(NormalizedPath, AbsolutePath, OutError))
			{
				return false;
			}
			if (SeenPaths.Contains(NormalizedPath))
			{
				OutError = FString::Printf(TEXT("selected Staging path가 중복됐습니다: %s"), *NormalizedPath);
				return false;
			}
			SeenPaths.Add(NormalizedPath);
			OutNormalizedPaths.Add(MoveTemp(NormalizedPath));
		}
		OutNormalizedPaths.Sort();
		OutError.Reset();
		return true;
	}

	// console Preview 인수를 StableLogicalId 또는 canonical relative path selection으로 변환합니다.
	bool BuildConsoleSelection(
		const TArray<FString>& Args,
		TArray<FString>& OutSelectedPaths,
		FString& OutError)
	{
		// console args에서 만든 raw selected paths입니다.
		TArray<FString> RawSelectedPaths;
		for (const FString& Arg : Args)
		{
			if (Arg.Contains(TEXT("/")) || Arg.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
			{
				RawSelectedPaths.Add(Arg);
			}
			else
			{
				RawSelectedPaths.Add(FString::Printf(TEXT("%s/%s.json"), MissilePresetStagingRoot, *Arg));
			}
		}
		return NormalizeSelectedPaths(RawSelectedPaths, OutSelectedPaths, OutError);
	}

	// enum typed value를 exact reflected source token으로 변환합니다.
	template <typename TEnum>
	FString GetEnumToken(const TEnum Value)
	{
		// requested enum reflected metadata입니다.
		const UEnum* Enum = StaticEnum<TEnum>();
		return Enum != nullptr
			? Enum->GetNameStringByValue(static_cast<int64>(Value))
			: FString();
	}

	// Literal FText JSON object를 만듭니다.
	TSharedRef<FJsonObject> BuildLiteralTextObject(const FString& Text)
	{
		// strict Literal FText JSON object입니다.
		TSharedRef<FJsonObject> TextObject = MakeShared<FJsonObject>();
		TextObject->SetStringField(TEXT("Kind"), TEXT("Literal"));
		TextObject->SetStringField(TEXT("Text"), Text);
		return TextObject;
	}

	// exact typed MissileGuideConfig를 strict 26-field JSON object로 직렬화합니다.
	TSharedRef<FJsonObject> BuildGuideConfigObject(const FCFMissileGuideConfig& Config)
	{
		// exact writable GuideConfig JSON object입니다.
		TSharedRef<FJsonObject> ConfigObject = MakeShared<FJsonObject>();
		ConfigObject->SetBoolField(TEXT("bUseGuidance"), Config.bUseGuidance);
		ConfigObject->SetStringField(TEXT("GuideMode"), GetEnumToken(Config.GuideMode));
		ConfigObject->SetStringField(TEXT("LostTargetPolicy"), GetEnumToken(Config.LostTargetPolicy));
		ConfigObject->SetNumberField(TEXT("NavigationConstant"), Config.NavigationConstant);
		ConfigObject->SetNumberField(TEXT("MaximumTurnRateDegPerSec"), Config.MaximumTurnRateDegPerSec);
		ConfigObject->SetNumberField(TEXT("MaximumLateralAccelerationCmPerSecSq"), Config.MaximumLateralAccelerationCmPerSecSq);
		ConfigObject->SetNumberField(TEXT("GuidanceResponseTimeSeconds"), Config.GuidanceResponseTimeSeconds);
		ConfigObject->SetNumberField(TEXT("MinimumGuidanceSpeedCmPerSec"), Config.MinimumGuidanceSpeedCmPerSec);
		ConfigObject->SetNumberField(TEXT("SeekerFieldOfViewDeg"), Config.SeekerFieldOfViewDeg);
		ConfigObject->SetNumberField(TEXT("LockBreakAngleDeg"), Config.LockBreakAngleDeg);
		ConfigObject->SetNumberField(TEXT("TargetLostGraceTimeSeconds"), Config.TargetLostGraceTimeSeconds);
		ConfigObject->SetStringField(TEXT("SeekerModel"), GetEnumToken(Config.SeekerModel));
		ConfigObject->SetStringField(TEXT("TargetObservationMode"), GetEnumToken(Config.TargetObservationMode));
		ConfigObject->SetStringField(TEXT("GuidanceLaw"), GetEnumToken(Config.GuidanceLaw));
		ConfigObject->SetStringField(TEXT("GuidanceActivationMode"), GetEnumToken(Config.GuidanceActivationMode));
		ConfigObject->SetNumberField(TEXT("GuidanceActivationDelaySeconds"), Config.GuidanceActivationDelaySeconds);
		ConfigObject->SetNumberField(TEXT("GuidanceActivationDistanceCm"), Config.GuidanceActivationDistanceCm);
		ConfigObject->SetNumberField(TEXT("LeadTimeSeconds"), Config.LeadTimeSeconds);
		ConfigObject->SetNumberField(TEXT("MaxLeadDistanceCm"), Config.MaxLeadDistanceCm);
		ConfigObject->SetStringField(TEXT("ReacquisitionMode"), GetEnumToken(Config.ReacquisitionMode));
		ConfigObject->SetNumberField(TEXT("TargetObservationIntervalSeconds"), Config.TargetObservationIntervalSeconds);
		ConfigObject->SetNumberField(TEXT("TargetVelocityEstimateResponseTimeSeconds"), Config.TargetVelocityEstimateResponseTimeSeconds);
		ConfigObject->SetNumberField(TEXT("AcquisitionConeHalfAngleDeg"), Config.AcquisitionConeHalfAngleDeg);
		ConfigObject->SetNumberField(TEXT("TrackingConeHalfAngleDeg"), Config.TrackingConeHalfAngleDeg);
		ConfigObject->SetNumberField(TEXT("ReacquisitionConeHalfAngleDeg"), Config.ReacquisitionConeHalfAngleDeg);
		ConfigObject->SetNumberField(TEXT("ReacquisitionTimeSeconds"), Config.ReacquisitionTimeSeconds);
		return ConfigObject;
	}

	// current typed Product payload를 revision-2 strict whole-record Update JSON으로 직렬화합니다.
	bool BuildProductStagingJson(
		const FCFDAMissilePresetPayload& Payload,
		const FString& TargetObjectPath,
		const FString& BaseSemanticFingerprint,
		FString& OutJsonText,
		FString& OutError)
	{
		// exact writable Payload JSON object입니다.
		TSharedRef<FJsonObject> PayloadObject = MakeShared<FJsonObject>();
		PayloadObject->SetStringField(TEXT("PresetId"), Payload.PresetId.ToString());
		PayloadObject->SetObjectField(TEXT("PresetDisplayName"), BuildLiteralTextObject(Payload.PresetDisplayName.Text));
		PayloadObject->SetObjectField(TEXT("PresetDescription"), BuildLiteralTextObject(Payload.PresetDescription.Text));
		PayloadObject->SetObjectField(TEXT("MissileGuideConfig"), BuildGuideConfigObject(Payload.MissileGuideConfig));

		// strict 8-field Staging root JSON object입니다.
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("SchemaId"), FCFDAStagingService::GetMissilePresetSchemaId());
		RootObject->SetNumberField(TEXT("SchemaRevision"), FCFDAStagingService::GetMissilePresetSchemaRevision());
		RootObject->SetNumberField(TEXT("AdapterContractRevision"), FCFDAStagingService::GetMissilePresetAdapterRevision());
		RootObject->SetStringField(TEXT("DataAssetTypeClassPath"), FCFDAStagingService::GetMissilePresetClassPath());
		RootObject->SetStringField(TEXT("StableLogicalId"), Payload.PresetId.ToString());
		RootObject->SetStringField(TEXT("TargetObjectPath"), TargetObjectPath);
		RootObject->SetStringField(TEXT("BaseSemanticFingerprint"), BaseSemanticFingerprint);
		RootObject->SetObjectField(TEXT("Payload"), PayloadObject);

		OutJsonText.Reset();
		// Git review 가능한 deterministic JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
		if (!FJsonSerializer::Serialize(RootObject, JsonWriter))
		{
			OutJsonText.Reset();
			OutError = TEXT("Product Staging JSON serialization에 실패했습니다.");
			return false;
		}
		JsonWriter->Close();
		OutJsonText += LINE_TERMINATOR;
		OutError.Reset();
		return true;
	}

	// diagnostic 목록을 bounded 한 줄 문자열로 합칩니다.
	FString FormatIssues(const TArray<FCFDAStagingIssue>& Issues)
	{
		// 사용자에게 보여줄 bounded issue text입니다.
		FString Result;
		for (int32 IssueIndex = 0; IssueIndex < Issues.Num(); ++IssueIndex)
		{
			if (IssueIndex > 0)
			{
				Result += TEXT(" | ");
			}
			Result += Issues[IssueIndex].Message;
		}
		return Result;
	}

	// Product target 하나를 read-only로 추출해 safe canonical Staging write plan을 만듭니다.
	bool BuildProductWritePlan(
		const FProductTarget& ProductTarget,
		FPlannedProductWrite& OutPlan,
		FString& OutError)
	{
		OutPlan = FPlannedProductWrite();
		// exact persisted/loaded Product asset입니다.
		UCFMissileGuidePresetData* ProductAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, ProductTarget.TargetObjectPath);
		if (ProductAsset == nullptr)
		{
			OutError = FString::Printf(TEXT("Product MissileGuidePreset을 읽지 못했습니다: %s"), ProductTarget.TargetObjectPath);
			return false;
		}

		// Product owning package입니다.
		UPackage* ProductPackage = ProductAsset->GetOutermost();
		if (ProductPackage == nullptr || ProductPackage->IsDirty())
		{
			OutError = FString::Printf(TEXT("Product package가 dirty이므로 Staging baseline을 캡처하지 않습니다: %s"), ProductTarget.TargetObjectPath);
			return false;
		}
		if (ProductAsset->PresetId != FName(ProductTarget.StableLogicalId))
		{
			OutError = FString::Printf(TEXT("Product PresetId가 expected StableLogicalId와 다릅니다: %s"), ProductTarget.TargetObjectPath);
			return false;
		}

		// persisted Product whole-record typed payload입니다.
		FCFDAMissilePresetPayload CurrentPayload;
		// payload extraction diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!FCFDAStagingService::ExtractMissilePresetPayload(*ProductAsset, CurrentPayload, ExtractIssues))
		{
			OutError = FString::Printf(TEXT("Product typed payload extraction 실패: %s / %s"), ProductTarget.TargetObjectPath, *FormatIssues(ExtractIssues));
			return false;
		}

		// current Product semantic baseline입니다.
		FString CurrentFingerprint;
		// fingerprint generation error입니다.
		FString FingerprintError;
		if (!FCFDAStagingService::BuildSemanticFingerprint(CurrentPayload, CurrentFingerprint, FingerprintError))
		{
			OutError = FingerprintError;
			return false;
		}

		// Product stable id 기반 canonical repository-relative Staging path입니다.
		const FString StagingRelativePath = FString::Printf(TEXT("%s/%s.json"), MissilePresetStagingRoot, ProductTarget.StableLogicalId);
		// Product staging physical path입니다.
		FString StagingAbsolutePath;
		if (!ResolveStagingAbsolutePath(StagingRelativePath, StagingAbsolutePath, OutError))
		{
			return false;
		}

		// current Product와 exact base가 같은 canonical revision-2 JSON입니다.
		FString CanonicalJsonText;
		if (!BuildProductStagingJson(CurrentPayload, ProductTarget.TargetObjectPath, CurrentFingerprint, CanonicalJsonText, OutError))
		{
			return false;
		}

		// 호출 전 existing Staging text입니다.
		FString ExistingJsonText;
		// 호출 전 file 존재 여부입니다.
		const bool bExistingFile = IFileManager::Get().FileExists(*StagingAbsolutePath);
		// 호출 전 raw bytes입니다.
		TArray<uint8> ExistingFileBytes;
		if (bExistingFile)
		{
			if (!FFileHelper::LoadFileToString(ExistingJsonText, *StagingAbsolutePath)
				|| !FFileHelper::LoadFileToArray(ExistingFileBytes, *StagingAbsolutePath))
			{
				OutError = FString::Printf(TEXT("기존 Product Staging file을 읽지 못했습니다: %s"), *StagingRelativePath);
				return false;
			}

			// existing JSON strict parse 결과입니다.
			const FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(ExistingJsonText, StagingRelativePath);
			if (!ParseResult.bValid)
			{
				OutError = FString::Printf(TEXT("기존 Product Staging이 Invalid라 자동 rewrite하지 않습니다: %s / %s"), *StagingRelativePath, *FormatIssues(ParseResult.Issues));
				return false;
			}
			if (ParseResult.Record.StableLogicalId != FName(ProductTarget.StableLogicalId)
				|| !ParseResult.Record.TargetObjectPath.Equals(ProductTarget.TargetObjectPath, ESearchCase::CaseSensitive))
			{
				OutError = FString::Printf(TEXT("기존 Product Staging identity/target이 canonical Product와 다르므로 자동 rewrite하지 않습니다: %s"), *StagingRelativePath);
				return false;
			}

			// existing record의 current UE truth입니다.
			FCFDAStagingCurrentState CurrentState;
			// resolver diagnostics입니다.
			TArray<FCFDAStagingIssue> CurrentIssues;
			if (!FCFDAStagingService::ResolveMissilePresetCurrentState(ParseResult.Record, CurrentState, CurrentIssues))
			{
				OutError = FString::Printf(TEXT("기존 Product Staging current truth를 확정하지 못해 rewrite를 차단합니다: %s / %s"), *StagingRelativePath, *FormatIssues(CurrentIssues));
				return false;
			}

			// existing Staging/current exact 3-way Preview입니다.
			const FCFDAStagingPreviewRow ExistingPreview = FCFDAStagingService::BuildPreview(ParseResult.Record, CurrentState);
			if (ExistingPreview.Kind != ECFDAStagingPreviewKind::NoChange
				|| !ParseResult.Record.StagingSemanticFingerprint.Equals(CurrentFingerprint, ESearchCase::CaseSensitive))
			{
				OutError = FString::Printf(TEXT("기존 Product Staging에 사용자 편집 또는 drift가 있어 SyncProduct가 덮어쓰지 않습니다: %s"), *StagingRelativePath);
				return false;
			}
		}

		OutPlan.TargetObjectPath = ProductTarget.TargetObjectPath;
		OutPlan.OriginalFingerprint = MoveTemp(CurrentFingerprint);
		OutPlan.StagingRelativePath = StagingRelativePath;
		OutPlan.StagingAbsolutePath = MoveTemp(StagingAbsolutePath);
		OutPlan.JsonText = MoveTemp(CanonicalJsonText);
		OutPlan.bOriginalFileExisted = bExistingFile;
		OutPlan.OriginalFileBytes = MoveTemp(ExistingFileBytes);
		OutPlan.bNeedsWrite = !bExistingFile || !ExistingJsonText.Equals(OutPlan.JsonText, ESearchCase::CaseSensitive);
		OutError.Reset();
		return true;
	}

	// Product Sync 뒤 Product package clean과 semantic fingerprint 불변을 재확인합니다.
	bool VerifyProductUnchanged(
		const FPlannedProductWrite& Plan,
		FString& OutError)
	{
		// Sync 뒤 exact Product asset입니다.
		UCFMissileGuidePresetData* ProductAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, *Plan.TargetObjectPath);
		if (ProductAsset == nullptr)
		{
			OutError = FString::Printf(TEXT("Sync 뒤 Product readback에 실패했습니다: %s"), *Plan.TargetObjectPath);
			return false;
		}
		// Sync 뒤 Product owning package입니다.
		UPackage* ProductPackage = ProductAsset->GetOutermost();
		if (ProductPackage == nullptr || ProductPackage->IsDirty())
		{
			OutError = FString::Printf(TEXT("SyncProduct가 Product package dirty를 유발했습니다: %s"), *Plan.TargetObjectPath);
			return false;
		}
		// Sync 뒤 Product typed payload입니다.
		FCFDAMissilePresetPayload CurrentPayload;
		// Sync 뒤 extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		if (!FCFDAStagingService::ExtractMissilePresetPayload(*ProductAsset, CurrentPayload, ExtractIssues))
		{
			OutError = FormatIssues(ExtractIssues);
			return false;
		}
		// Sync 뒤 semantic fingerprint입니다.
		FString CurrentFingerprint;
		// Sync 뒤 fingerprint error입니다.
		FString FingerprintError;
		if (!FCFDAStagingService::BuildSemanticFingerprint(CurrentPayload, CurrentFingerprint, FingerprintError))
		{
			OutError = FingerprintError;
			return false;
		}
		if (!CurrentFingerprint.Equals(Plan.OriginalFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = FString::Printf(TEXT("SyncProduct 전후 Product semantic fingerprint가 바뀌었습니다: %s"), *Plan.TargetObjectPath);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// 이번 Sync 호출에서 touched된 Staging 파일을 호출 전 raw bytes/absence로 역순 복원합니다.
	bool RollbackTouchedStagingFiles(
		const TArray<FPlannedProductWrite>& Plans,
		const TArray<int32>& TouchedPlanIndices,
		FString& OutRollbackError)
	{
		OutRollbackError.Reset();
		// rollback 전체 성공 여부입니다.
		bool bRollbackSucceeded = true;
		for (int32 ReverseIndex = TouchedPlanIndices.Num() - 1; ReverseIndex >= 0; --ReverseIndex)
		{
			// rollback할 original plan index입니다.
			const int32 PlanIndex = TouchedPlanIndices[ReverseIndex];
			// rollback할 exact write plan입니다.
			const FPlannedProductWrite& Plan = Plans[PlanIndex];
			// 한 file rollback 성공 여부입니다.
			bool bFileRestored = false;
			if (Plan.bOriginalFileExisted)
			{
				bFileRestored = FFileHelper::SaveArrayToFile(Plan.OriginalFileBytes, *Plan.StagingAbsolutePath);
				if (bFileRestored)
				{
					// rollback readback raw bytes입니다.
					TArray<uint8> RestoredBytes;
					bFileRestored = FFileHelper::LoadFileToArray(RestoredBytes, *Plan.StagingAbsolutePath)
						&& RestoredBytes == Plan.OriginalFileBytes;
				}
			}
			else
			{
				if (IFileManager::Get().FileExists(*Plan.StagingAbsolutePath))
				{
					bFileRestored = IFileManager::Get().Delete(*Plan.StagingAbsolutePath, false, true, true);
				}
				else
				{
					bFileRestored = true;
				}
				bFileRestored = bFileRestored && !IFileManager::Get().FileExists(*Plan.StagingAbsolutePath);
			}

			if (!bFileRestored)
			{
				bRollbackSucceeded = false;
				if (!OutRollbackError.IsEmpty())
				{
					OutRollbackError += TEXT(" | ");
				}
				OutRollbackError += FString::Printf(TEXT("rollback 확인 실패: %s"), *Plan.StagingRelativePath);
			}
		}
		return bRollbackSucceeded;
	}

	// Product Staging write 결과를 exact canonical text로 확인합니다.
	bool VerifyWrittenStagingFile(
		const FPlannedProductWrite& Plan,
		FString& OutError)
	{
		// write 뒤 exact Staging text입니다.
		FString WrittenText;
		if (!FFileHelper::LoadFileToString(WrittenText, *Plan.StagingAbsolutePath)
			|| !WrittenText.Equals(Plan.JsonText, ESearchCase::CaseSensitive))
		{
			OutError = FString::Printf(TEXT("Product Staging write readback이 canonical text와 다릅니다: %s"), *Plan.StagingRelativePath);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// parse/resolver 단계 failure를 Preview Invalid row로 보존합니다.
	FCFDAStagingPreviewRow BuildInvalidPreviewRow(
		const FCFDAStagingRecord& Record,
		const FString& StagingRelativePath,
		const TArray<FCFDAStagingIssue>& Issues)
	{
		// 반환할 Invalid Preview row입니다.
		FCFDAStagingPreviewRow Row;
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		Row.Record = Record;
		if (Row.Record.StagingRelativePath.IsEmpty())
		{
			Row.Record.StagingRelativePath = StagingRelativePath;
		}
		Row.Issues = Issues;
		return Row;
	}

	// Preview kind를 USER-facing text로 변환합니다.
	const TCHAR* GetPreviewKindText(const ECFDAStagingPreviewKind Kind)
	{
		switch (Kind)
		{
		case ECFDAStagingPreviewKind::Create:
			return TEXT("Create");
		case ECFDAStagingPreviewKind::Update:
			return TEXT("Update");
		case ECFDAStagingPreviewKind::NoChange:
			return TEXT("NoChange");
		case ECFDAStagingPreviewKind::Conflict:
			return TEXT("Conflict");
		case ECFDAStagingPreviewKind::Invalid:
		default:
			return TEXT("Invalid");
		}
	}

	// Preview 결과를 bounded Editor log로 표시합니다.
	void LogPreview(const FCFDAStagingOpsPreview& Preview)
	{
		UE_LOG(
			LogCFDAStagingOps,
			Display,
			TEXT("[DA Staging] rows=%d Create=%d Update=%d NoChange=%d Conflict=%d Invalid=%d blocked=%s"),
			Preview.Rows.Num(),
			Preview.CreateCount,
			Preview.UpdateCount,
			Preview.NoChangeCount,
			Preview.ConflictCount,
			Preview.InvalidCount,
			Preview.bBlocked ? TEXT("true") : TEXT("false"));

		for (const FCFDAStagingPreviewRow& Row : Preview.Rows)
		{
			UE_LOG(
				LogCFDAStagingOps,
				Display,
				TEXT("[DA Staging] %s | %s | %s | %s"),
				GetPreviewKindText(Row.Kind),
				*Row.Record.StableLogicalId.ToString(),
				*Row.Record.TargetObjectPath,
				*Row.Record.StagingRelativePath);
			for (const FCFDAStagingIssue& Issue : Row.Issues)
			{
				UE_LOG(
					LogCFDAStagingOps,
					Display,
					TEXT("[DA Staging]   %s%s"),
					Issue.bBlocking ? TEXT("[차단] ") : TEXT("[안내] "),
					*Issue.Message);
			}
		}

		if (Preview.bHasMutationCandidates && !Preview.BatchPlanHash.IsEmpty())
		{
			UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging] BatchPlanHash=%s"), *Preview.BatchPlanHash);
		}
		else if (!Preview.bBlocked)
		{
			UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging] mutation candidate가 없습니다. Review/Apply가 필요하지 않습니다."));
		}
	}

	// console commands가 공유하는 operational review session입니다.
	FCFDAStagingOpsSession GConsoleSession;

	// Product Low/Normal/High canonical Staging을 bootstrap/rebase하는 Editor console 명령입니다.
	void SyncProductCommand()
	{
		// 실제 변경된 Staging source paths입니다.
		TArray<FString> ChangedPaths;
		// Sync failure detail입니다.
		FString Error;
		GConsoleSession.Reset();
		if (!FCFDAStagingOps::SyncProductStaging(ChangedPaths, Error))
		{
			UE_LOG(LogCFDAStagingOps, Error, TEXT("[DA Staging] SyncProduct 실패: %s"), *Error);
			return;
		}
		UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging] Product Sync 완료. Product UE Asset Save=0, changed staging files=%d"), ChangedPaths.Num());
		for (const FString& ChangedPath : ChangedPaths)
		{
			UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging]   %s"), *ChangedPath);
		}
	}

	// canonical Staging folder 또는 exact selected Staging을 mutation0 Preview하는 Editor console 명령입니다.
	void PreviewCommand(const TArray<FString>& Args)
	{
		// console args에서 만든 exact selected relative paths입니다.
		TArray<FString> SelectedPaths;
		// selection/Preview failure detail입니다.
		FString Error;
		if (!BuildConsoleSelection(Args, SelectedPaths, Error))
		{
			GConsoleSession.Reset();
			UE_LOG(LogCFDAStagingOps, Error, TEXT("[DA Staging] Preview selection 실패: %s"), *Error);
			return;
		}

		// current exact selected Preview입니다.
		FCFDAStagingOpsPreview Preview;
		if (!GConsoleSession.Preview(SelectedPaths, Preview, Error))
		{
			UE_LOG(LogCFDAStagingOps, Error, TEXT("[DA Staging] Preview 실패: %s"), *Error);
			return;
		}
		if (SelectedPaths.IsEmpty())
		{
			UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging] selection=ALL canonical MissileGuidePreset JSON"));
		}
		else
		{
			UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging] selection=%d exact JSON"), SelectedPaths.Num());
		}
		LogPreview(Preview);
	}

	// 마지막 selected Preview를 fresh same-selection discovery/hash와 다시 대조해 Reviewed approval을 동결하는 Editor console 명령입니다.
	void ReviewCommand()
	{
		// Review failure detail입니다.
		FString Error;
		if (!GConsoleSession.Review(Error))
		{
			UE_LOG(LogCFDAStagingOps, Error, TEXT("[DA Staging] Review 실패: %s"), *Error);
			return;
		}
		UE_LOG(LogCFDAStagingOps, Display, TEXT("[DA Staging] Reviewed approval 준비 완료: %s"), *GConsoleSession.GetReviewedApproval().BatchPlanHash);
		UE_LOG(LogCFDAStagingOps, Warning, TEXT("[DA Staging] 실제 UE Asset 저장은 별도 CarFight.DAStaging.ApplyReviewed 명령에서만 수행됩니다."));
	}

	// 명시적으로 Reviewed된 exact approval만 기존 Apply service에 전달하는 Editor console 명령입니다.
	void ApplyReviewedCommand()
	{
		// exact reviewed batch Apply report입니다.
		FCFDAStagingApplyReport ApplyReport;
		// Apply failure detail입니다.
		FString Error;
		if (GConsoleSession.ApplyReviewed(ApplyReport, Error))
		{
			UE_LOG(
				LogCFDAStagingOps,
				Display,
				TEXT("[DA Staging] ApplyReviewed 종료: result=%d durable=%d notRun=%d / %s"),
				static_cast<int32>(ApplyReport.Result),
				ApplyReport.DurableAppliedCount,
				ApplyReport.NotRunCount,
				*ApplyReport.Diagnostic);
			return;
		}

		UE_LOG(
			LogCFDAStagingOps,
			Error,
			TEXT("[DA Staging] ApplyReviewed 실패: result=%d durable=%d notRun=%d / %s / %s"),
			static_cast<int32>(ApplyReport.Result),
			ApplyReport.DurableAppliedCount,
			ApplyReport.NotRunCount,
			*ApplyReport.Diagnostic,
			*Error);
	}

	// Product Staging bootstrap/rebase console entry입니다.
	FAutoConsoleCommand SyncProductConsoleCommand(
		TEXT("CarFight.DAStaging.SyncProduct"),
		TEXT("Low/Normal/High Product를 current persisted semantic baseline으로 canonical Staging JSON에 bootstrap/rebase합니다. Product UE Asset은 저장하지 않습니다."),
		FConsoleCommandDelegate::CreateStatic(&SyncProductCommand));

	// canonical Staging discovery/selected Preview console entry입니다.
	FAutoConsoleCommand PreviewConsoleCommand(
		TEXT("CarFight.DAStaging.Preview"),
		TEXT("인수 없음=전체 canonical JSON, 인수 있음=StableLogicalId 또는 canonical relative JSON path exact-list만 Preview합니다."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&PreviewCommand));

	// fresh reviewed approval freeze console entry입니다.
	FAutoConsoleCommand ReviewConsoleCommand(
		TEXT("CarFight.DAStaging.Review"),
		TEXT("마지막 selected Preview의 same selection을 fresh discovery/hash와 다시 대조한 뒤 one-shot Reviewed approval을 준비합니다. UE Asset 저장은 수행하지 않습니다."),
		FConsoleCommandDelegate::CreateStatic(&ReviewCommand));

	// explicit reviewed Apply console entry입니다.
	FAutoConsoleCommand ApplyReviewedConsoleCommand(
		TEXT("CarFight.DAStaging.ApplyReviewed"),
		TEXT("명시적으로 Review된 selected exact batch만 materialize/SavePackage합니다. 실행 전 Preview와 Review 결과를 확인해야 합니다."),
		FConsoleCommandDelegate::CreateStatic(&ApplyReviewedCommand));
}

// P0 MissileGuidePreset canonical repository-relative Staging directory를 반환합니다.
const TCHAR* FCFDAStagingOps::GetMissilePresetStagingRoot()
{
	return CFDAStagingOpsPrivate::MissilePresetStagingRoot;
}

// persisted Product Low/Normal/High를 current semantic baseline으로 canonical Staging JSON에 bootstrap/rebase하며 Product .uasset은 저장하지 않습니다.
bool FCFDAStagingOps::SyncProductStaging(
	TArray<FString>& OutChangedStagingPaths,
	FString& OutError)
{
	OutChangedStagingPaths.Reset();
	// all-or-nothing semantic preflight를 통과한 Product JSON write plans입니다.
	TArray<CFDAStagingOpsPrivate::FPlannedProductWrite> PlannedWrites;
	for (const CFDAStagingOpsPrivate::FProductTarget& ProductTarget : CFDAStagingOpsPrivate::GetProductTargets())
	{
		// current Product 한 건의 preflight write plan입니다.
		CFDAStagingOpsPrivate::FPlannedProductWrite PlannedWrite;
		if (!CFDAStagingOpsPrivate::BuildProductWritePlan(ProductTarget, PlannedWrite, OutError))
		{
			return false;
		}
		PlannedWrites.Add(MoveTemp(PlannedWrite));
	}

	// 이번 호출에서 실제 write를 시도한 plan indices입니다. 실패 시 original raw bytes/absence rollback 대상입니다.
	TArray<int32> TouchedPlanIndices;
	// 실제 changed-file write ordinal입니다.
	int32 WriteOrdinal = 0;
	for (int32 PlanIndex = 0; PlanIndex < PlannedWrites.Num(); ++PlanIndex)
	{
		// exact current write plan입니다.
		const CFDAStagingOpsPrivate::FPlannedProductWrite& PlannedWrite = PlannedWrites[PlanIndex];
		if (!PlannedWrite.bNeedsWrite)
		{
			continue;
		}

		// exact Staging parent directory입니다.
		const FString StagingDirectory = FPaths::GetPath(PlannedWrite.StagingAbsolutePath);
		TouchedPlanIndices.Add(PlanIndex);
		// current canonical JSON write 성공 여부입니다.
		const bool bWriteSucceeded = IFileManager::Get().MakeDirectory(*StagingDirectory, true)
			&& FFileHelper::SaveStringToFile(
				PlannedWrite.JsonText,
				*PlannedWrite.StagingAbsolutePath,
				FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

		// write 단계 failure detail입니다.
		FString WriteFailure;
		if (!bWriteSucceeded)
		{
			WriteFailure = FString::Printf(TEXT("canonical Product Staging JSON write에 실패했습니다: %s"), *PlannedWrite.StagingRelativePath);
		}
#if WITH_DEV_AUTOMATION_TESTS
		else if (CFDAStagingOpsPrivate::GForceSyncFailureAfterWriteOrdinal == WriteOrdinal)
		{
			WriteFailure = FString::Printf(TEXT("DAS-P0-05 Automation fixture가 write 성공 직후 failure를 강제했습니다: %s"), *PlannedWrite.StagingRelativePath);
		}
#endif
		if (!WriteFailure.IsEmpty())
		{
			// partial write rollback failure detail입니다.
			FString RollbackError;
			const bool bRollbackSucceeded = CFDAStagingOpsPrivate::RollbackTouchedStagingFiles(PlannedWrites, TouchedPlanIndices, RollbackError);
			OutChangedStagingPaths.Reset();
			OutError = bRollbackSucceeded
				? WriteFailure + TEXT(" / touched Staging rollback confirmed")
				: WriteFailure + TEXT(" / Staging rollback unconfirmed: ") + RollbackError;
			return false;
		}
		++WriteOrdinal;
	}

	for (const int32 PlanIndex : TouchedPlanIndices)
	{
		// write readback을 확인할 exact plan입니다.
		const CFDAStagingOpsPrivate::FPlannedProductWrite& PlannedWrite = PlannedWrites[PlanIndex];
		// write readback failure detail입니다.
		FString VerificationError;
		if (!CFDAStagingOpsPrivate::VerifyWrittenStagingFile(PlannedWrite, VerificationError))
		{
			// verification failure 뒤 rollback detail입니다.
			FString RollbackError;
			const bool bRollbackSucceeded = CFDAStagingOpsPrivate::RollbackTouchedStagingFiles(PlannedWrites, TouchedPlanIndices, RollbackError);
			OutChangedStagingPaths.Reset();
			OutError = bRollbackSucceeded
				? VerificationError + TEXT(" / touched Staging rollback confirmed")
				: VerificationError + TEXT(" / Staging rollback unconfirmed: ") + RollbackError;
			return false;
		}
	}

	for (const CFDAStagingOpsPrivate::FPlannedProductWrite& PlannedWrite : PlannedWrites)
	{
		if (!CFDAStagingOpsPrivate::VerifyProductUnchanged(PlannedWrite, OutError))
		{
			// Product mutation0 verification failure도 successful Sync로 보고하지 않도록 touched Staging을 원상복구합니다.
			FString RollbackError;
			const bool bRollbackSucceeded = CFDAStagingOpsPrivate::RollbackTouchedStagingFiles(PlannedWrites, TouchedPlanIndices, RollbackError);
			OutChangedStagingPaths.Reset();
			if (!bRollbackSucceeded)
			{
				OutError += TEXT(" / Staging rollback unconfirmed: ") + RollbackError;
			}
			else
			{
				OutError += TEXT(" / touched Staging rollback confirmed");
			}
			return false;
		}
	}

	for (const int32 PlanIndex : TouchedPlanIndices)
	{
		OutChangedStagingPaths.Add(PlannedWrites[PlanIndex].StagingRelativePath);
	}
	OutError.Reset();
	return true;
}

// canonical MissileGuidePreset Staging JSON 전체를 deterministic discover/parse/resolve/Preview하고 approval용 BatchPlanHash까지 계산합니다.
bool FCFDAStagingOps::DiscoverMissilePresetPreview(
	FCFDAStagingOpsPreview& OutPreview,
	FString& OutError)
{
	// empty selection은 canonical folder 전체를 뜻합니다.
	const TArray<FString> AllSelection;
	return DiscoverMissilePresetPreview(AllSelection, OutPreview, OutError);
}

// canonical MissileGuidePreset Staging 중 exact selected relative paths만 deterministic discover/parse/resolve/Preview하고 approval용 BatchPlanHash까지 계산합니다.
bool FCFDAStagingOps::DiscoverMissilePresetPreview(
	const TArray<FString>& SelectedStagingRelativePaths,
	FCFDAStagingOpsPreview& OutPreview,
	FString& OutError)
{
	OutPreview = FCFDAStagingOpsPreview();
	// canonical validation/dedupe/sort된 selected relative paths입니다.
	TArray<FString> NormalizedSelectedPaths;
	if (!CFDAStagingOpsPrivate::NormalizeSelectedPaths(SelectedStagingRelativePaths, NormalizedSelectedPaths, OutError))
	{
		return false;
	}

	// Preview할 exact physical JSON filenames입니다.
	TArray<FString> StagingFiles;
	if (NormalizedSelectedPaths.IsEmpty())
	{
		// canonical Staging root absolute path입니다.
		FString StagingRootSentinelAbsolutePath;
		// canonical root 자체는 file이 아니므로 sentinel filename을 resolve해 parent directory를 얻습니다.
		const FString RootSentinelRelativePath = FString::Printf(TEXT("%s/__RootSentinel__.json"), CFDAStagingOpsPrivate::MissilePresetStagingRoot);
		if (!CFDAStagingOpsPrivate::ResolveStagingAbsolutePath(RootSentinelRelativePath, StagingRootSentinelAbsolutePath, OutError))
		{
			return false;
		}
		// canonical Staging root physical directory입니다.
		const FString StagingRootAbsolutePath = FPaths::GetPath(StagingRootSentinelAbsolutePath);
		if (!IFileManager::Get().DirectoryExists(*StagingRootAbsolutePath))
		{
			OutError = FString::Printf(TEXT("canonical Staging directory가 없습니다. 먼저 SyncProduct 또는 Staging 작성이 필요합니다: %s"), CFDAStagingOpsPrivate::MissilePresetStagingRoot);
			return false;
		}
		IFileManager::Get().FindFilesRecursive(StagingFiles, *StagingRootAbsolutePath, TEXT("*.json"), true, false, false);
		StagingFiles.Sort();
	}
	else
	{
		for (const FString& SelectedPath : NormalizedSelectedPaths)
		{
			// exact selected physical path입니다.
			FString SelectedAbsolutePath;
			if (!CFDAStagingOpsPrivate::ResolveStagingAbsolutePath(SelectedPath, SelectedAbsolutePath, OutError))
			{
				return false;
			}
			if (!IFileManager::Get().FileExists(*SelectedAbsolutePath))
			{
				OutError = FString::Printf(TEXT("selected Staging JSON이 존재하지 않습니다: %s"), *SelectedPath);
				return false;
			}
			StagingFiles.Add(MoveTemp(SelectedAbsolutePath));
		}
	}

	if (StagingFiles.IsEmpty())
	{
		OutError = TEXT("Preview할 canonical MissileGuidePreset Staging JSON이 없습니다.");
		return false;
	}

	for (const FString& StagingAbsolutePath : StagingFiles)
	{
		// discovered file의 canonical main_game-relative source path입니다.
		FString StagingRelativePath;
		if (!CFDAStagingOpsPrivate::MakeMainGameRelativePath(StagingAbsolutePath, StagingRelativePath, OutError))
		{
			return false;
		}

		// discovered source JSON text입니다.
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *StagingAbsolutePath))
		{
			OutError = FString::Printf(TEXT("Staging JSON을 읽지 못했습니다: %s"), *StagingRelativePath);
			return false;
		}

		// strict source parse 결과입니다.
		const FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(JsonText, StagingRelativePath);
		if (!ParseResult.bValid)
		{
			OutPreview.Rows.Add(CFDAStagingOpsPrivate::BuildInvalidPreviewRow(ParseResult.Record, StagingRelativePath, ParseResult.Issues));
			continue;
		}

		// exact current UE truth입니다.
		FCFDAStagingCurrentState CurrentState;
		// current resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!FCFDAStagingService::ResolveMissilePresetCurrentState(ParseResult.Record, CurrentState, CurrentIssues))
		{
			OutPreview.Rows.Add(CFDAStagingOpsPrivate::BuildInvalidPreviewRow(ParseResult.Record, StagingRelativePath, CurrentIssues));
			continue;
		}

		OutPreview.Rows.Add(FCFDAStagingService::BuildPreview(ParseResult.Record, CurrentState));
	}

	FCFDAStagingService::ApplyBatchDuplicateValidation(OutPreview.Rows);
	for (const FCFDAStagingPreviewRow& Row : OutPreview.Rows)
	{
		switch (Row.Kind)
		{
		case ECFDAStagingPreviewKind::Create:
			++OutPreview.CreateCount;
			break;
		case ECFDAStagingPreviewKind::Update:
			++OutPreview.UpdateCount;
			break;
		case ECFDAStagingPreviewKind::NoChange:
			++OutPreview.NoChangeCount;
			break;
		case ECFDAStagingPreviewKind::Conflict:
			++OutPreview.ConflictCount;
			break;
		case ECFDAStagingPreviewKind::Invalid:
		default:
			++OutPreview.InvalidCount;
			break;
		}
	}

	OutPreview.bBlocked = OutPreview.ConflictCount > 0 || OutPreview.InvalidCount > 0;
	OutPreview.bHasMutationCandidates = OutPreview.CreateCount > 0 || OutPreview.UpdateCount > 0;
	if (!OutPreview.bBlocked && OutPreview.bHasMutationCandidates)
	{
		if (!FCFDAStagingService::BuildBatchPlanHash(OutPreview.Rows, OutPreview.BatchPlanHash, OutError))
		{
			return false;
		}
	}
	else
	{
		OutPreview.BatchPlanHash.Reset();
	}

	OutError.Reset();
	return true;
}

// selected exact Staging set을 fresh Preview하고 이후 Review가 같은 selection을 재검증하도록 동결합니다.
bool FCFDAStagingOpsSession::Preview(
	const TArray<FString>& InSelectedStagingRelativePaths,
	FCFDAStagingOpsPreview& OutPreview,
	FString& OutError)
{
	Reset();
	// canonical validation/dedupe/sort된 session selection입니다.
	TArray<FString> NormalizedSelection;
	if (!CFDAStagingOpsPrivate::NormalizeSelectedPaths(InSelectedStagingRelativePaths, NormalizedSelection, OutError))
	{
		return false;
	}
	if (!FCFDAStagingOps::DiscoverMissilePresetPreview(NormalizedSelection, OutPreview, OutError))
	{
		return false;
	}
	SelectedStagingRelativePaths = MoveTemp(NormalizedSelection);
	LastPreview = OutPreview;
	bHasLastPreview = true;
	OutError.Reset();
	return true;
}

// 마지막 mutation Preview를 동일 selection의 fresh discovery/hash와 재대조해 Reviewed approval로 동결합니다.
bool FCFDAStagingOpsSession::Review(FString& OutError)
{
	ReviewedApproval = FCFDAStagingReviewedApproval();
	if (!bHasLastPreview || LastPreview.bBlocked || !LastPreview.bHasMutationCandidates || LastPreview.BatchPlanHash.IsEmpty())
	{
		OutError = TEXT("Review할 mutation Preview가 없습니다. 먼저 selected Preview를 실행하고 blocker를 해소하세요.");
		return false;
	}

	// Review 직전 same-selection fresh discovery Preview입니다.
	FCFDAStagingOpsPreview FreshPreview;
	if (!FCFDAStagingOps::DiscoverMissilePresetPreview(SelectedStagingRelativePaths, FreshPreview, OutError))
	{
		return false;
	}
	if (FreshPreview.bBlocked
		|| !FreshPreview.bHasMutationCandidates
		|| !FreshPreview.BatchPlanHash.Equals(LastPreview.BatchPlanHash, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Preview 이후 selected source/current target set이 바뀌었습니다. same selection의 fresh Preview가 필요합니다.");
		return false;
	}

	// fresh exact selected rows로 동결할 Reviewed approval입니다.
	FCFDAStagingReviewedApproval NewReviewedApproval;
	if (!FCFDAStagingApplyService::BuildReviewedApproval(FreshPreview.Rows, NewReviewedApproval, OutError)
		|| !NewReviewedApproval.BatchPlanHash.Equals(FreshPreview.BatchPlanHash, ESearchCase::CaseSensitive))
	{
		ReviewedApproval = FCFDAStagingReviewedApproval();
		return false;
	}

	LastPreview = FreshPreview;
	ReviewedApproval = MoveTemp(NewReviewedApproval);
	OutError.Reset();
	return true;
}

// current session에 실제 Reviewed one-shot approval이 존재하는지 반환합니다.
bool FCFDAStagingOpsSession::HasReviewedApproval() const
{
	return ReviewedApproval.State == ECFDAStagingApprovalState::Reviewed
		&& !ReviewedApproval.BatchPlanHash.IsEmpty()
		&& !ReviewedApproval.IncludedTargets.IsEmpty();
}

// 현재 Reviewed approval evidence를 read-only로 반환합니다.
const FCFDAStagingReviewedApproval& FCFDAStagingOpsSession::GetReviewedApproval() const
{
	return ReviewedApproval;
}

// 현재 Reviewed approval을 기존 exact Apply service에 one-shot 전달합니다.
bool FCFDAStagingOpsSession::ApplyReviewed(
	FCFDAStagingApplyReport& OutReport,
	FString& OutError)
{
	if (!HasReviewedApproval())
	{
		OutReport = FCFDAStagingApplyReport();
		OutError = TEXT("Reviewed approval이 없습니다. Preview → Review 순서를 먼저 완료하세요.");
		return false;
	}
	// existing exact Apply safety owner의 terminal result입니다.
	const bool bApplySucceeded = FCFDAStagingApplyService::ApplyReviewedBatch(ReviewedApproval, OutReport);
	SelectedStagingRelativePaths.Reset();
	LastPreview = FCFDAStagingOpsPreview();
	bHasLastPreview = false;
	OutError = bApplySucceeded ? FString() : OutReport.Diagnostic;
	return bApplySucceeded;
}

// Preview/selection/approval session state를 전부 초기화합니다.
void FCFDAStagingOpsSession::Reset()
{
	SelectedStagingRelativePaths.Reset();
	LastPreview = FCFDAStagingOpsPreview();
	bHasLastPreview = false;
	ReviewedApproval = FCFDAStagingReviewedApproval();
}

#if WITH_DEV_AUTOMATION_TESTS
// 모든 test-only Ops fault injection state를 초기화합니다.
void FCFDAStagingOpsTestControl::Reset()
{
	CFDAStagingOpsPrivate::GForceSyncFailureAfterWriteOrdinal = INDEX_NONE;
}

// zero-based changed-file write ordinal에서 실제 write 성공 직후 failure를 강제해 rollback을 검증합니다.
void FCFDAStagingOpsTestControl::ForceSyncFailureAfterWrite(const int32 WriteOrdinal)
{
	CFDAStagingOpsPrivate::GForceSyncFailureAfterWriteOrdinal = WriteOrdinal;
}
#endif
