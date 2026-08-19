// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringService.cpp
// Version: v1.5.0
// Date: 2026-08-18
// Description: DAUTH-P0-08I~P0-12 Common Authoring Service 구현입니다.
// Scope: R0 read facade, semantic preview/commit, bounded Mesh-only Candidate projection, R3 shared Apply lane를 제공합니다.
// Changelog:
// - v1.5.0: P0-12 UA-02에서 sibling directory만 조회해 실제 Mesh-only 차량이 0개로 보이던 문제를 common vehicle mesh collection root recursive query로 교정.
// - v1.4.0: P0-12 USER Acceptance에서 발견된 WheelMesh→Mesh-only Candidate 오분류를 실제 VehicleVisualConfig wheel 참조 기반으로 차단.
// - v1.3.0: P0-11 Vehicle Browser에 existing Chassis directories 기반 bounded Mesh-only Candidate projection 추가.
// - v1.2.0: P0-09 reviewed Initial Import R2 proposal/commit facade를 추가하고 Existing Import Core로만 Recipe를 생성/초기화.
// - v1.1.0: RecipeFingerprint 비소비 semantic field도 typed desired-state equality로 NoChange를 판정하도록 교정.
// - v1.0.0: Snapshot/Resolver/Apply Core를 재사용하는 FCFVehicleAuthoringService 최초 구현.
// Migration:
// - Raw Stable Field write, direct existing Target UCFVehicleData mutation, Save/SaveAll을 구현하지 않습니다.
// - Shared Profile/Drift/Create UX operation은 별도 CFVehicleUXOps.cpp에서 existing Core를 orchestration합니다.
// - ApplyResolvedVehicle은 FCFVehicleApplyService::Apply를 정확히 한 번 호출하는 shared writer lane만 사용합니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAssetReader.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleAuthoringPrivate
{
	// Editor lifetime에서 보관할 mutation terminal dedupe result 최대 개수입니다.
	static constexpr int32 MaxDedupeEntries = 128;

	/** ClientOperationId 하나의 request identity와 terminal result를 보존합니다. */
	struct FDedupeEntry
	{
		// 같은 operation id가 같은 logical request인지 판정할 deterministic request hash입니다.
		FString RequestHash;

		// 첫 실행의 terminal typed result입니다.
		FCFAuthoringOpResult Result;
	};

	// Editor lifetime에만 존재하며 Recipe/Definition SSOT가 아닌 bounded mutation dedupe cache입니다.
	TMap<FString, FDedupeEntry> GDedupeCache;

	// 오래된 dedupe entry를 deterministic FIFO로 제거하기 위한 insertion order입니다.
	TArray<FString> GDedupeOrder;

	/** Fresh resolve orchestration이 내부에서 공유하는 loaded UObject와 immutable request/result입니다. */
	struct FFreshResolveState
	{
		// Persistent Recipe object입니다.
		UCFVehicleRecipeData* Recipe = nullptr;

		// Recipe가 binding한 current Target object입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// Shared SnapshotBuilder/AssetReader가 구성한 immutable resolver request입니다.
		FCFVehicleResolveRequest ResolveRequest;

		// Shared Pure Resolver의 exact result입니다.
		FCFVehicleResolveResult ResolveResult;
	};

	// UTF-8 canonical payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR 표현과 분리된 canonical UTF-8 payload입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// UTF-8 bytes의 deterministic digest를 계산할 MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// MD5 128-bit 결과 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Platform-independent lowercase hexadecimal 결과 문자열입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// 한 nibble을 lowercase hex로 바꾸는 고정 문자표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Delimiter 충돌 없이 canonical payload 조각을 label/문자수/value 형태로 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

		// Initial Import request에서 validated package/object identity를 deterministic하게 만듭니다.
	bool BuildInitialImportIdentity(
		const FCFVehicleInitialImportRequest& Request,
		FString& OutPackageName,
		FSoftObjectPath& OutObjectPath,
		FString& OutError)
	{
		// Caller가 입력한 package folder의 trim된 값입니다.
		FString FolderPath = Request.RecipePackagePath.TrimStartAndEnd();
		FolderPath.RemoveFromEnd(TEXT("/"));
		// Recipe asset/object에 사용할 이름 문자열입니다.
		const FString AssetName = Request.RecipeAssetName.ToString();
		if (FolderPath.IsEmpty() || Request.RecipeAssetName.IsNone() || AssetName.Contains(TEXT("/")) || AssetName.Contains(TEXT("\\")) || AssetName.Contains(TEXT(".")))
		{
			OutError = TEXT("Initial Import에는 valid Recipe folder와 단일 asset name이 필요합니다.");
			return false;
		}
		if (Request.CallContext.CallerKind != ECFAuthoringCallerKind::Automation && !FolderPath.StartsWith(TEXT("/Game/")) && FolderPath != TEXT("/Game"))
		{
			OutError = TEXT("Vehicle Authoring UI의 새 Recipe는 /Game 이하 package에만 만들 수 있습니다.");
			return false;
		}

		OutPackageName = FolderPath + TEXT("/") + AssetName;
		if (!FPackageName::IsValidLongPackageName(OutPackageName))
		{
			OutError = FString::Printf(TEXT("유효하지 않은 Recipe long package name입니다: %s"), *OutPackageName);
			return false;
		}
		OutObjectPath = FSoftObjectPath(OutPackageName + TEXT(".") + AssetName);
		if (!OutObjectPath.IsValid())
		{
			OutError = TEXT("Initial Import Recipe object path를 만들 수 없습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Current loaded/registered Recipe 중 Target VehicleData를 이미 binding한 record가 있는지 검사합니다.
	bool HasExistingRecipeForTarget(UCFVehicleData& TargetVehicleData, FString& OutRecipePath)
	{
		// Target identity 비교에 사용할 canonical soft object path입니다.
		const FSoftObjectPath TargetPath(&TargetVehicleData);
		for (TObjectIterator<UCFVehicleRecipeData> Iterator; Iterator; ++Iterator)
		{
			// 현재 loaded Recipe candidate입니다.
			UCFVehicleRecipeData* Recipe = *Iterator;
			if (IsValid(Recipe) && Recipe->TargetVehicleData.ToSoftObjectPath() == TargetPath)
			{
				OutRecipePath = Recipe->GetPathName();
				return true;
			}
		}

		// Asset Registry의 unloaded Recipe candidates입니다.
		TArray<FAssetData> RecipeAssets;
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().GetAssetsByClass(UCFVehicleRecipeData::StaticClass()->GetClassPathName(), RecipeAssets, true);
		for (const FAssetData& RecipeAsset : RecipeAssets)
		{
			// Registry candidate를 exact Recipe로 load한 결과입니다.
			UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(RecipeAsset.GetAsset());
			if (Recipe && Recipe->TargetVehicleData.ToSoftObjectPath() == TargetPath)
			{
				OutRecipePath = Recipe->GetPathName();
				return true;
			}
		}
		OutRecipePath.Reset();
		return false;
	}

	// Initial Import exact target/destination/import summary를 approval scope hash로 결합합니다.
	FString BuildInitialImportProposalHash(
		const UCFVehicleData& TargetVehicleData,
		const FSoftObjectPath& RecipeObjectPath,
		const FCFVehicleDefinitionSnapshot& DefinitionSnapshot,
		const FCFVehicleImportResult& ImportSummary)
	{
		// Localized text/UObject address를 제외한 R2 approval canonical payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), TEXT("ImportExistingDefinition"));
		AppendToken(Payload, TEXT("TargetPath"), FSoftObjectPath(&TargetVehicleData).ToString());
		AppendToken(Payload, TEXT("TargetDefinitionHash"), DefinitionSnapshot.DefinitionHash);
		AppendToken(Payload, TEXT("RecipeObjectPath"), RecipeObjectPath.ToString());
		AppendToken(Payload, TEXT("LegacyPinnedFieldCount"), FString::FromInt(ImportSummary.LegacyPinnedFieldCount));
		AppendToken(Payload, TEXT("LegacySerializedFieldCount"), FString::FromInt(ImportSummary.LegacySerializedFieldCount));
		AppendToken(Payload, TEXT("SemanticCandidateFieldCount"), FString::FromInt(ImportSummary.SemanticCandidateFieldCount));
		AppendToken(Payload, TEXT("ResolverRevision"), FString::FromInt(FCFVehicleResolver::CurrentResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// float semantic payload를 signed-zero 차이 없이 deterministic text로 변환합니다.
	FString CanonicalFloat(const float Value)
	{
		// +0/-0을 하나로 정규화한 값입니다.
		const double Normalized = Value == 0.0f ? 0.0 : static_cast<double>(Value);
		return FString::Printf(TEXT("%.9g"), Normalized);
	}

	// Typed semantic enum을 사람이 읽는 stable operation name으로 변환합니다.
	FName GetSemanticOperationName(const ECFVehicleSemanticOp Operation)
	{
		switch (Operation)
		{
		case ECFVehicleSemanticOp::BindVehicleProfile:
			return TEXT("BindVehicleProfile");
		case ECFVehicleSemanticOp::SetVehicleArchetype:
			return TEXT("SetVehicleArchetype");
		case ECFVehicleSemanticOp::SetVehicleAssetIntent:
			return TEXT("SetVehicleAssetIntent");
		case ECFVehicleSemanticOp::SetDrivingFeel:
			return TEXT("SetDrivingFeel");
		case ECFVehicleSemanticOp::SetMassIntent:
			return TEXT("SetMassIntent");
		case ECFVehicleSemanticOp::SetDurabilityIntent:
			return TEXT("SetDurabilityIntent");
		case ECFVehicleSemanticOp::SetDefaultDataIntent:
			return TEXT("SetDefaultDataIntent");
		case ECFVehicleSemanticOp::SetWheelVisualIntent:
			return TEXT("SetWheelVisualIntent");
		case ECFVehicleSemanticOp::SetDriveStateMode:
			return TEXT("SetDriveStateMode");
		case ECFVehicleSemanticOp::UpsertHardpointIntent:
			return TEXT("UpsertHardpointIntent");
		case ECFVehicleSemanticOp::UpsertMountIntent:
			return TEXT("UpsertMountIntent");
		default:
			return TEXT("UnsupportedSemanticOperation");
		}
	}

	// Common typed result envelope를 operation identity/risk의 초기 상태로 만듭니다.
	void InitializeResult(
		FCFAuthoringOpResult& OutResult,
		const FName OperationName,
		const ECFAuthoringRiskClass RiskClass,
		const FString& ClientOperationId = FString())
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = OperationName;
		OutResult.RiskClass = RiskClass;
		OutResult.ClientOperationId = ClientOperationId;
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		OutResult.Mutation.bSavePerformed = false;
		OutResult.Mutation.bAutomaticRetryPerformed = false;
	}

	// Result를 mutation 없는 typed Blocked 상태로 설정합니다.
	void SetBlocked(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Result를 concurrent/idempotency conflict 상태로 설정합니다.
	void SetConflict(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Conflict;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Result를 internal failure 상태로 설정합니다.
	void SetFailed(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Result를 정상 read/write success 상태로 설정합니다.
	void SetSucceeded(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Resolver validation issue 배열 하나를 compact summary에 누적합니다.
	void AccumulateValidation(
		const TArray<FCFVehicleValidationIssue>& Issues,
		FCFAuthoringValidationSummary& InOutSummary)
	{
		for (const FCFVehicleValidationIssue& Issue : Issues)
		{
			switch (Issue.Severity)
			{
			case ECFVehicleValidationSeverity::Info:
				++InOutSummary.InfoCount;
				break;
			case ECFVehicleValidationSeverity::Warning:
				++InOutSummary.WarningCount;
				break;
			case ECFVehicleValidationSeverity::Blocked:
				++InOutSummary.BlockedCount;
				InOutSummary.bApplyBlocking = true;
				break;
			case ECFVehicleValidationSeverity::Error:
				++InOutSummary.ErrorCount;
				InOutSummary.bApplyBlocking = true;
				break;
			default:
				break;
			}
		}
	}

	// Shared Resolver result의 세 validation layer를 common compact summary로 변환합니다.
	FCFAuthoringValidationSummary BuildValidationSummary(const FCFVehicleResolveResult& ResolveResult)
	{
		// 세 validation layer를 누적할 결과입니다.
		FCFAuthoringValidationSummary Summary;
		AccumulateValidation(ResolveResult.RecipeValidation, Summary);
		AccumulateValidation(ResolveResult.ResolverValidation, Summary);
		AccumulateValidation(ResolveResult.DefinitionValidation, Summary);
		return Summary;
	}

	// Shared Resolve result의 current hashes/validation을 common result envelope에 기록합니다.
	void PopulateResultFromResolve(
		const FCFVehicleResolveRequest& ResolveRequest,
		const FCFVehicleResolveResult& ResolveResult,
		FCFAuthoringOpResult& OutResult)
	{
		OutResult.CurrentRecipeFingerprint = ResolveRequest.Recipe.RecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = ResolveRequest.bHasCurrentDefinition
			? ResolveRequest.CurrentDefinition.DefinitionHash
			: FString();
		OutResult.CurrentSourceSignature = ResolveResult.SourceSignature;
		OutResult.CurrentResolvedDefinitionHash = ResolveResult.ResolvedDefinitionHash;
		OutResult.CurrentDiffHash = FCFVehicleAuthoringService::BuildDiffHash(ResolveResult.FieldDiff);
		OutResult.ResolverContractRevision = ResolveResult.ResolverContractRevision;
		OutResult.ValidationSummary = BuildValidationSummary(ResolveResult);
	}

	// Soft path를 load하고 exact expected class인지 검증합니다.
	UObject* LoadTypedObject(
		const FSoftObjectPath& ObjectPath,
		const UClass* ExpectedClass,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		if (!ObjectPath.IsValid())
		{
			OutErrorCode = ECFAuthoringErrorCode::MissingRequiredSource;
			OutError = TEXT("필수 Authoring asset path가 비어 있습니다.");
			return nullptr;
		}

		// 이미 로드된 object가 있으면 package load를 반복하지 않고 사용합니다.
		UObject* LoadedObject = ObjectPath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = ObjectPath.TryLoad();
		}
		if (!LoadedObject)
		{
			OutErrorCode = ECFAuthoringErrorCode::ProfileNotFound;
			OutError = FString::Printf(TEXT("Authoring asset을 찾을 수 없습니다: %s"), *ObjectPath.ToString());
			return nullptr;
		}
		if (!ExpectedClass || !LoadedObject->IsA(ExpectedClass))
		{
			OutErrorCode = ECFAuthoringErrorCode::WrongAssetType;
			OutError = FString::Printf(
				TEXT("Authoring asset type이 기대 class와 다릅니다: %s / Actual=%s / Expected=%s"),
				*ObjectPath.ToString(),
				*LoadedObject->GetClass()->GetPathName(),
				ExpectedClass ? *ExpectedClass->GetPathName() : TEXT("None"));
			return nullptr;
		}

		OutErrorCode = ECFAuthoringErrorCode::None;
		OutError.Reset();
		return LoadedObject;
	}

	// Optional StaticMesh soft reference가 존재하면 exact UStaticMesh로 resolve되는지 검증합니다.
	bool ValidateOptionalStaticMesh(const TSoftObjectPtr<UStaticMesh>& MeshReference, FString& OutError)
	{
		// Soft reference identity입니다.
		const FSoftObjectPath MeshPath = MeshReference.ToSoftObjectPath();
		if (!MeshPath.IsValid())
		{
			OutError.Reset();
			return true;
		}

		// 이미 로드된 mesh 또는 path에서 load한 object입니다.
		UObject* LoadedObject = MeshPath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = MeshPath.TryLoad();
		}
		if (!Cast<UStaticMesh>(LoadedObject))
		{
			OutError = FString::Printf(TEXT("StaticMesh reference가 없거나 잘못된 type입니다: %s"), *MeshPath.ToString());
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Recipe Snapshot의 5개 typed Profile binding을 기존 SnapshotBuilder 입력 UObject로 resolve합니다.
	bool BuildProfileSnapshots(
		const FCFVehicleRecipeSnapshot& RecipeSnapshot,
		FCFVehicleProfileSnapshotSet& OutProfiles,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		// Vehicle Base Profile path입니다.
		const FSoftObjectPath BasePath = RecipeSnapshot.ProfileBindings.VehicleBaseProfile.ToSoftObjectPath();
		// Drivetrain Profile path입니다.
		const FSoftObjectPath DrivetrainPath = RecipeSnapshot.ProfileBindings.DrivetrainProfile.ToSoftObjectPath();
		// Handling Profile path입니다.
		const FSoftObjectPath HandlingPath = RecipeSnapshot.ProfileBindings.HandlingProfile.ToSoftObjectPath();
		// Performance Profile path입니다.
		const FSoftObjectPath PerformancePath = RecipeSnapshot.ProfileBindings.PerformanceProfile.ToSoftObjectPath();
		// DriveState Profile path입니다.
		const FSoftObjectPath DriveStatePath = RecipeSnapshot.ProfileBindings.DriveStateProfile.ToSoftObjectPath();

		// Optional Vehicle Base Profile UObject입니다.
		UCFVehicleBaseProfile* BaseProfile = nullptr;
		// Optional Drivetrain Profile UObject입니다.
		UCFDrivetrainProfile* DrivetrainProfile = nullptr;
		// Optional Handling Profile UObject입니다.
		UCFHandlingProfile* HandlingProfile = nullptr;
		// Optional Performance Profile UObject입니다.
		UCFPerformanceProfile* PerformanceProfile = nullptr;
		// Optional DriveState Profile UObject입니다.
		UCFDriveStateProfile* DriveStateProfile = nullptr;

		if (BasePath.IsValid())
		{
			BaseProfile = Cast<UCFVehicleBaseProfile>(LoadTypedObject(BasePath, UCFVehicleBaseProfile::StaticClass(), OutErrorCode, OutError));
			if (!BaseProfile)
			{
				return false;
			}
		}
		if (DrivetrainPath.IsValid())
		{
			DrivetrainProfile = Cast<UCFDrivetrainProfile>(LoadTypedObject(DrivetrainPath, UCFDrivetrainProfile::StaticClass(), OutErrorCode, OutError));
			if (!DrivetrainProfile)
			{
				return false;
			}
		}
		if (HandlingPath.IsValid())
		{
			HandlingProfile = Cast<UCFHandlingProfile>(LoadTypedObject(HandlingPath, UCFHandlingProfile::StaticClass(), OutErrorCode, OutError));
			if (!HandlingProfile)
			{
				return false;
			}
		}
		if (PerformancePath.IsValid())
		{
			PerformanceProfile = Cast<UCFPerformanceProfile>(LoadTypedObject(PerformancePath, UCFPerformanceProfile::StaticClass(), OutErrorCode, OutError));
			if (!PerformanceProfile)
			{
				return false;
			}
		}
		if (DriveStatePath.IsValid())
		{
			DriveStateProfile = Cast<UCFDriveStateProfile>(LoadTypedObject(DriveStatePath, UCFDriveStateProfile::StaticClass(), OutErrorCode, OutError));
			if (!DriveStateProfile)
			{
				return false;
			}
		}

		if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			BaseProfile,
			DrivetrainProfile,
			HandlingProfile,
			PerformanceProfile,
			DriveStateProfile,
			OutProfiles,
			OutError))
		{
			OutErrorCode = ECFAuthoringErrorCode::InternalError;
			return false;
		}

		OutErrorCode = ECFAuthoringErrorCode::None;
		return true;
	}

	// Recipe binding과 optional explicit Target request를 교차검증하고 current Target UObject를 반환합니다.
	UCFVehicleData* ResolveTargetVehicleData(
		UCFVehicleRecipeData& Recipe,
		UCFVehicleData* ExplicitTarget,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		// Recipe가 저장한 canonical target path입니다.
		const FSoftObjectPath BoundTargetPath = Recipe.TargetVehicleData.ToSoftObjectPath();
		if (!BoundTargetPath.IsValid())
		{
			OutErrorCode = ECFAuthoringErrorCode::TargetNotFound;
			OutError = TEXT("Recipe에 Target VehicleData binding이 없습니다.");
			return nullptr;
		}

		if (ExplicitTarget)
		{
			// Caller가 넘긴 explicit Target identity입니다.
			const FSoftObjectPath ExplicitTargetPath(ExplicitTarget);
			if (ExplicitTargetPath != BoundTargetPath)
			{
				OutErrorCode = ECFAuthoringErrorCode::StateChanged;
				OutError = TEXT("Explicit Target과 Recipe Target binding identity가 다릅니다.");
				return nullptr;
			}
			OutErrorCode = ECFAuthoringErrorCode::None;
			OutError.Reset();
			return ExplicitTarget;
		}

		// 이미 load된 Target 또는 binding path에서 load한 UObject입니다.
		UObject* LoadedObject = BoundTargetPath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = BoundTargetPath.TryLoad();
		}
		// Recipe binding의 실제 UCFVehicleData입니다.
		UCFVehicleData* TargetVehicleData = Cast<UCFVehicleData>(LoadedObject);
		if (!TargetVehicleData)
		{
			OutErrorCode = LoadedObject ? ECFAuthoringErrorCode::WrongAssetType : ECFAuthoringErrorCode::TargetNotFound;
			OutError = FString::Printf(TEXT("Recipe Target VehicleData를 resolve할 수 없습니다: %s"), *BoundTargetPath.ToString());
			return nullptr;
		}

		OutErrorCode = ECFAuthoringErrorCode::None;
		OutError.Reset();
		return TargetVehicleData;
	}

	// Persistent Recipe/Target truth를 existing SnapshotBuilder/AssetReader/Pure Resolver로 orchestration합니다.
	bool BuildFreshResolveState(
		const FCFVehicleAuthoringReadRequest& Request,
		FFreshResolveState& OutState,
		FCFAuthoringOpResult& OutOperation)
	{
		if (!Request.Recipe)
		{
			SetBlocked(OutOperation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Recipe가 null입니다."));
			return false;
		}

		OutState = FFreshResolveState();
		OutState.Recipe = Request.Recipe;

		// Target resolve failure taxonomy입니다.
		ECFAuthoringErrorCode TargetErrorCode = ECFAuthoringErrorCode::None;
		// Target resolve failure 메시지입니다.
		FString ResolveError;
		OutState.TargetVehicleData = ResolveTargetVehicleData(*Request.Recipe, Request.TargetVehicleData, TargetErrorCode, ResolveError);
		if (!OutState.TargetVehicleData)
		{
			SetBlocked(OutOperation, TargetErrorCode, ResolveError);
			return false;
		}

		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.Recipe, OutState.ResolveRequest.Recipe, ResolveError))
		{
			SetFailed(OutOperation, ResolveError);
			return false;
		}

		// Profile snapshot build failure taxonomy입니다.
		ECFAuthoringErrorCode ProfileErrorCode = ECFAuthoringErrorCode::None;
		if (!BuildProfileSnapshots(OutState.ResolveRequest.Recipe, OutState.ResolveRequest.Profiles, ProfileErrorCode, ResolveError))
		{
			SetBlocked(OutOperation, ProfileErrorCode, ResolveError);
			return false;
		}

		if (!FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(OutState.ResolveRequest.ProjectDefaults, ResolveError)
			|| !FCFVehicleAssetReader::BuildAssetSnapshot(OutState.ResolveRequest.Recipe, OutState.ResolveRequest.Assets, ResolveError)
			|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*OutState.TargetVehicleData, OutState.ResolveRequest.CurrentDefinition, ResolveError))
		{
			SetFailed(OutOperation, ResolveError);
			return false;
		}

		OutState.ResolveRequest.bHasCurrentDefinition = true;
		OutState.ResolveRequest.bHasPreviewContext = Request.bHasPreviewContext;
		OutState.ResolveRequest.PreviewContext = Request.PreviewContext;
		OutState.ResolveRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		if (!FCFVehicleResolver::Resolve(OutState.ResolveRequest, OutState.ResolveResult))
		{
			SetFailed(OutOperation, TEXT("Shared Pure Resolver가 internal Error로 실패했습니다."));
			return false;
		}

		PopulateResultFromResolve(OutState.ResolveRequest, OutState.ResolveResult, OutOperation);
		return true;
	}

	// Persistent Recipe UObject를 다시 읽지 않고 prospective Recipe Snapshot을 shared Core로 resolve합니다.
	bool ResolveProspectiveSnapshot(
		const FCFVehicleRecipeSnapshot& ProspectiveRecipe,
		UCFVehicleData& TargetVehicleData,
		const bool bHasPreviewContext,
		const FCFVehicleResolverPreviewContext& PreviewContext,
		FCFVehicleResolveRequest& OutRequest,
		FCFVehicleResolveResult& OutResult,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		OutRequest = FCFVehicleResolveRequest();
		OutRequest.Recipe = ProspectiveRecipe;

		if (!BuildProfileSnapshots(ProspectiveRecipe, OutRequest.Profiles, OutErrorCode, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(OutRequest.ProjectDefaults, OutError)
			|| !FCFVehicleAssetReader::BuildAssetSnapshot(ProspectiveRecipe, OutRequest.Assets, OutError)
			|| !FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(TargetVehicleData, OutRequest.CurrentDefinition, OutError))
		{
			if (OutErrorCode == ECFAuthoringErrorCode::None)
			{
				OutErrorCode = ECFAuthoringErrorCode::InternalError;
			}
			return false;
		}

		OutRequest.bHasCurrentDefinition = true;
		OutRequest.bHasPreviewContext = bHasPreviewContext;
		OutRequest.PreviewContext = PreviewContext;
		OutRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		if (!FCFVehicleResolver::Resolve(OutRequest, OutResult))
		{
			OutErrorCode = ECFAuthoringErrorCode::InternalError;
			OutError = TEXT("Prospective shared Pure Resolver가 internal Error로 실패했습니다.");
			return false;
		}
		OutErrorCode = ECFAuthoringErrorCode::None;
		OutError.Reset();
		return true;
	}

	// exact Profile Domain이 기대하는 UClass를 반환합니다.
	const UClass* GetProfileClass(const ECFVehicleProfileDomain Domain)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return UCFVehicleBaseProfile::StaticClass();
		case ECFVehicleProfileDomain::Drivetrain:
			return UCFDrivetrainProfile::StaticClass();
		case ECFVehicleProfileDomain::Handling:
			return UCFHandlingProfile::StaticClass();
		case ECFVehicleProfileDomain::Performance:
			return UCFPerformanceProfile::StaticClass();
		case ECFVehicleProfileDomain::DriveState:
			return UCFDriveStateProfile::StaticClass();
		default:
			return nullptr;
		}
	}

	// Prospective Recipe Snapshot에 exact Profile binding path를 설정합니다.
	bool SetProfileBinding(
		FCFVehicleRecipeSnapshot& Snapshot,
		const ECFVehicleProfileDomain Domain,
		const FSoftObjectPath& ProfilePath,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		// Domain별 expected Profile class입니다.
		const UClass* ExpectedClass = GetProfileClass(Domain);
		if (!ExpectedClass || !ProfilePath.IsValid())
		{
			OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
			OutError = TEXT("Profile binding에는 exact Domain과 non-empty Profile asset path가 필요합니다.");
			return false;
		}
		if (!LoadTypedObject(ProfilePath, ExpectedClass, OutErrorCode, OutError))
		{
			return false;
		}

		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			Snapshot.ProfileBindings.VehicleBaseProfile = TSoftObjectPtr<UCFVehicleBaseProfile>(ProfilePath);
			break;
		case ECFVehicleProfileDomain::Drivetrain:
			Snapshot.ProfileBindings.DrivetrainProfile = TSoftObjectPtr<UCFDrivetrainProfile>(ProfilePath);
			break;
		case ECFVehicleProfileDomain::Handling:
			Snapshot.ProfileBindings.HandlingProfile = TSoftObjectPtr<UCFHandlingProfile>(ProfilePath);
			break;
		case ECFVehicleProfileDomain::Performance:
			Snapshot.ProfileBindings.PerformanceProfile = TSoftObjectPtr<UCFPerformanceProfile>(ProfilePath);
			break;
		case ECFVehicleProfileDomain::DriveState:
			Snapshot.ProfileBindings.DriveStateProfile = TSoftObjectPtr<UCFDriveStateProfile>(ProfilePath);
			break;
		default:
			OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
			OutError = TEXT("지원하지 않는 Profile Domain입니다.");
			return false;
		}

		OutErrorCode = ECFAuthoringErrorCode::None;
		OutError.Reset();
		return true;
	}

	// Persistent Recipe UObject에 exact Profile binding path를 설정합니다.
	bool SetPersistentProfileBinding(
		UCFVehicleRecipeData& Recipe,
		const ECFVehicleProfileDomain Domain,
		const FSoftObjectPath& ProfilePath)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			Recipe.ProfileBindings.VehicleBaseProfile = TSoftObjectPtr<UCFVehicleBaseProfile>(ProfilePath);
			return true;
		case ECFVehicleProfileDomain::Drivetrain:
			Recipe.ProfileBindings.DrivetrainProfile = TSoftObjectPtr<UCFDrivetrainProfile>(ProfilePath);
			return true;
		case ECFVehicleProfileDomain::Handling:
			Recipe.ProfileBindings.HandlingProfile = TSoftObjectPtr<UCFHandlingProfile>(ProfilePath);
			return true;
		case ECFVehicleProfileDomain::Performance:
			Recipe.ProfileBindings.PerformanceProfile = TSoftObjectPtr<UCFPerformanceProfile>(ProfilePath);
			return true;
		case ECFVehicleProfileDomain::DriveState:
			Recipe.ProfileBindings.DriveStateProfile = TSoftObjectPtr<UCFDriveStateProfile>(ProfilePath);
			return true;
		default:
			return false;
		}
	}

	// SetDrivingFeel partial patch가 0..1 범위인지 검사하고 Snapshot의 selected axes만 바꿉니다.
	bool ApplyDrivingFeelPatch(
		FCFVehicleFeelIntent& InOutIntent,
		const FCFDrivingFeelPatch& Patch,
		FString& OutError)
	{
		// 하나 이상의 axis가 실제 typed patch에 포함됐는지 여부입니다.
		const bool bHasAnyAxis = Patch.bSetAccelerationFeel
			|| Patch.bSetSteeringAgility
			|| Patch.bSetGripFeel
			|| Patch.bSetSuspensionFirmness;
		if (!bHasAnyAxis)
		{
			OutError = TEXT("SetDrivingFeel patch에는 최소 하나의 semantic axis가 필요합니다.");
			return false;
		}

		// selected axis 값이 Frozen 0..1 범위 안인지 검사하는 helper입니다.
		auto IsValidAxis = [](const bool bSetAxis, const float AxisValue)
		{
			return !bSetAxis || (FMath::IsFinite(AxisValue) && AxisValue >= 0.0f && AxisValue <= 1.0f);
		};
		if (!IsValidAxis(Patch.bSetAccelerationFeel, Patch.AccelerationFeel)
			|| !IsValidAxis(Patch.bSetSteeringAgility, Patch.SteeringAgility)
			|| !IsValidAxis(Patch.bSetGripFeel, Patch.GripFeel)
			|| !IsValidAxis(Patch.bSetSuspensionFirmness, Patch.SuspensionFirmness))
		{
			OutError = TEXT("Driving Feel semantic value는 clamp하지 않으며 0..1 범위여야 합니다.");
			return false;
		}

		if (Patch.bSetAccelerationFeel)
		{
			InOutIntent.AccelerationFeel = Patch.AccelerationFeel;
		}
		if (Patch.bSetSteeringAgility)
		{
			InOutIntent.SteeringAgility = Patch.SteeringAgility;
		}
		if (Patch.bSetGripFeel)
		{
			InOutIntent.GripFeel = Patch.GripFeel;
		}
		if (Patch.bSetSuspensionFirmness)
		{
			InOutIntent.SuspensionFirmness = Patch.SuspensionFirmness;
		}
		OutError.Reset();
		return true;
	}

	// normal R1 typed semantic command를 prospective Recipe Snapshot에 적용합니다.
	bool ApplySemanticChangeToSnapshot(
		FCFVehicleRecipeSnapshot& InOutSnapshot,
		const FCFVehicleSemanticChange& Change,
		ECFAuthoringErrorCode& OutErrorCode,
		FString& OutError)
	{
		switch (Change.Operation)
		{
		case ECFVehicleSemanticOp::BindVehicleProfile:
			return SetProfileBinding(InOutSnapshot, Change.ProfileDomain, Change.ProfileAssetPath, OutErrorCode, OutError);

		case ECFVehicleSemanticOp::SetVehicleArchetype:
			if (Change.VehicleArchetypeId.IsNone())
			{
				OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
				OutError = TEXT("VehicleArchetypeId는 None일 수 없습니다.");
				return false;
			}
			InOutSnapshot.VehicleArchetypeId = Change.VehicleArchetypeId;
			break;

		case ECFVehicleSemanticOp::SetVehicleAssetIntent:
			if (!ValidateOptionalStaticMesh(Change.AssetIntent.ChassisMesh, OutError)
				|| !ValidateOptionalStaticMesh(Change.AssetIntent.WheelMeshFL, OutError)
				|| !ValidateOptionalStaticMesh(Change.AssetIntent.WheelMeshFR, OutError)
				|| !ValidateOptionalStaticMesh(Change.AssetIntent.WheelMeshRL, OutError)
				|| !ValidateOptionalStaticMesh(Change.AssetIntent.WheelMeshRR, OutError))
			{
				OutErrorCode = ECFAuthoringErrorCode::WrongAssetType;
				return false;
			}
			InOutSnapshot.AssetIntent = Change.AssetIntent;
			break;

		case ECFVehicleSemanticOp::SetDrivingFeel:
			if (!ApplyDrivingFeelPatch(InOutSnapshot.DrivingFeelIntent, Change.DrivingFeelPatch, OutError))
			{
				OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
				return false;
			}
			break;

		case ECFVehicleSemanticOp::SetMassIntent:
			if (!FMath::IsFinite(Change.MassIntent.ExplicitBaseMassKg)
				|| !FMath::IsFinite(Change.MassIntent.ExplicitGrossMassKg)
				|| Change.MassIntent.ExplicitBaseMassKg < 0.0f
				|| Change.MassIntent.ExplicitGrossMassKg < 0.0f)
			{
				OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
				OutError = TEXT("Mass semantic explicit value는 음수/비유한 값일 수 없습니다.");
				return false;
			}
			InOutSnapshot.MassIntent = Change.MassIntent;
			break;

		case ECFVehicleSemanticOp::SetDurabilityIntent:
			if (!FMath::IsFinite(Change.DurabilityIntent.ExplicitMaxHealth)
				|| Change.DurabilityIntent.ExplicitMaxHealth < 0.0f)
			{
				OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
				OutError = TEXT("Durability semantic explicit value는 음수/비유한 값일 수 없습니다.");
				return false;
			}
			InOutSnapshot.DurabilityIntent = Change.DurabilityIntent;
			break;

		case ECFVehicleSemanticOp::SetDefaultDataIntent:
			InOutSnapshot.DefaultDataIntent = Change.DefaultDataIntent;
			break;

		case ECFVehicleSemanticOp::SetWheelVisualIntent:
			InOutSnapshot.WheelVisualIntent = Change.WheelVisualIntent;
			break;

		case ECFVehicleSemanticOp::SetDriveStateMode:
			InOutSnapshot.DriveStateMode = Change.DriveStateMode;
			break;

		case ECFVehicleSemanticOp::UpsertHardpointIntent:
		{
			if (Change.HardpointIntent.LocationSlotId.IsNone())
			{
				OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
				OutError = TEXT("Hardpoint semantic write에는 non-None LocationSlotId가 필요합니다.");
				return false;
			}
			// 동일 Stable-ID의 existing prospective Hardpoint입니다.
			FCFHardpointIntent* ExistingHardpoint = InOutSnapshot.HardpointIntents.FindByPredicate([&Change](const FCFHardpointIntent& Intent)
			{
				return Intent.LocationSlotId == Change.HardpointIntent.LocationSlotId;
			});
			if (ExistingHardpoint)
			{
				*ExistingHardpoint = Change.HardpointIntent;
			}
			else
			{
				InOutSnapshot.HardpointIntents.Add(Change.HardpointIntent);
			}
			break;
		}

		case ECFVehicleSemanticOp::UpsertMountIntent:
		{
			if (Change.MountIntent.MountProfileId.IsNone() || Change.MountIntent.LocationSlotRef.IsNone())
			{
				OutErrorCode = ECFAuthoringErrorCode::InvalidSemanticInput;
				OutError = TEXT("Mount semantic write에는 MountProfileId와 LocationSlotRef가 필요합니다.");
				return false;
			}
			// Mount가 참조하는 prospective Hardpoint identity 존재 여부입니다.
			const bool bHardpointExists = InOutSnapshot.HardpointIntents.ContainsByPredicate([&Change](const FCFHardpointIntent& Intent)
			{
				return Intent.LocationSlotId == Change.MountIntent.LocationSlotRef;
			});
			if (!bHardpointExists)
			{
				OutErrorCode = ECFAuthoringErrorCode::DependencyConflict;
				OutError = FString::Printf(TEXT("Mount가 참조하는 Hardpoint가 Recipe에 없습니다: %s"), *Change.MountIntent.LocationSlotRef.ToString());
				return false;
			}
			// 동일 Stable-ID의 existing prospective Mount입니다.
			FCFMountIntent* ExistingMount = InOutSnapshot.MountIntents.FindByPredicate([&Change](const FCFMountIntent& Intent)
			{
				return Intent.MountProfileId == Change.MountIntent.MountProfileId;
			});
			if (ExistingMount)
			{
				*ExistingMount = Change.MountIntent;
			}
			else
			{
				InOutSnapshot.MountIntents.Add(Change.MountIntent);
			}
			break;
		}

		default:
			OutErrorCode = ECFAuthoringErrorCode::UnsupportedOperation;
			OutError = TEXT("지원하지 않는 Recipe semantic operation입니다.");
			return false;
		}

		OutErrorCode = ECFAuthoringErrorCode::None;
		OutError.Reset();
		return true;
	}

		// Current persistent Recipe가 typed semantic desired state를 이미 만족하는지 exact operation별로 판정합니다.
	bool IsSemanticChangeSatisfied(const UCFVehicleRecipeData& Recipe, const FCFVehicleSemanticChange& Change)
	{
		switch (Change.Operation)
		{
		case ECFVehicleSemanticOp::BindVehicleProfile:
			switch (Change.ProfileDomain)
			{
			case ECFVehicleProfileDomain::VehicleBase:
				return Recipe.ProfileBindings.VehicleBaseProfile.ToSoftObjectPath() == Change.ProfileAssetPath;
			case ECFVehicleProfileDomain::Drivetrain:
				return Recipe.ProfileBindings.DrivetrainProfile.ToSoftObjectPath() == Change.ProfileAssetPath;
			case ECFVehicleProfileDomain::Handling:
				return Recipe.ProfileBindings.HandlingProfile.ToSoftObjectPath() == Change.ProfileAssetPath;
			case ECFVehicleProfileDomain::Performance:
				return Recipe.ProfileBindings.PerformanceProfile.ToSoftObjectPath() == Change.ProfileAssetPath;
			case ECFVehicleProfileDomain::DriveState:
				return Recipe.ProfileBindings.DriveStateProfile.ToSoftObjectPath() == Change.ProfileAssetPath;
			default:
				return false;
			}

		case ECFVehicleSemanticOp::SetVehicleArchetype:
			return Recipe.VehicleArchetypeId == Change.VehicleArchetypeId;

		case ECFVehicleSemanticOp::SetVehicleAssetIntent:
			return FCFVehicleAssetIntent::StaticStruct()->CompareScriptStruct(&Recipe.AssetIntent, &Change.AssetIntent, 0);

		case ECFVehicleSemanticOp::SetDrivingFeel:
			return (!Change.DrivingFeelPatch.bSetAccelerationFeel || Recipe.DrivingFeelIntent.AccelerationFeel == Change.DrivingFeelPatch.AccelerationFeel)
				&& (!Change.DrivingFeelPatch.bSetSteeringAgility || Recipe.DrivingFeelIntent.SteeringAgility == Change.DrivingFeelPatch.SteeringAgility)
				&& (!Change.DrivingFeelPatch.bSetGripFeel || Recipe.DrivingFeelIntent.GripFeel == Change.DrivingFeelPatch.GripFeel)
				&& (!Change.DrivingFeelPatch.bSetSuspensionFirmness || Recipe.DrivingFeelIntent.SuspensionFirmness == Change.DrivingFeelPatch.SuspensionFirmness);

		case ECFVehicleSemanticOp::SetMassIntent:
			return FCFVehicleMassIntent::StaticStruct()->CompareScriptStruct(&Recipe.MassIntent, &Change.MassIntent, 0);

		case ECFVehicleSemanticOp::SetDurabilityIntent:
			return FCFVehicleDurabilityIntent::StaticStruct()->CompareScriptStruct(&Recipe.DurabilityIntent, &Change.DurabilityIntent, 0);

		case ECFVehicleSemanticOp::SetDefaultDataIntent:
			return FCFVehicleDefaultIntent::StaticStruct()->CompareScriptStruct(&Recipe.DefaultDataIntent, &Change.DefaultDataIntent, 0);

		case ECFVehicleSemanticOp::SetWheelVisualIntent:
			return FCFWheelVisualIntent::StaticStruct()->CompareScriptStruct(&Recipe.WheelVisualIntent, &Change.WheelVisualIntent, 0);

		case ECFVehicleSemanticOp::SetDriveStateMode:
			return Recipe.DriveStateMode == Change.DriveStateMode;

		case ECFVehicleSemanticOp::UpsertHardpointIntent:
		{
			// Same Stable-ID의 persistent Hardpoint입니다.
			const FCFHardpointIntent* ExistingHardpoint = Recipe.HardpointIntents.FindByPredicate([&Change](const FCFHardpointIntent& Intent)
			{
				return Intent.LocationSlotId == Change.HardpointIntent.LocationSlotId;
			});
			return ExistingHardpoint
				&& FCFHardpointIntent::StaticStruct()->CompareScriptStruct(ExistingHardpoint, &Change.HardpointIntent, 0);
		}

		case ECFVehicleSemanticOp::UpsertMountIntent:
		{
			// Same Stable-ID의 persistent Mount입니다.
			const FCFMountIntent* ExistingMount = Recipe.MountIntents.FindByPredicate([&Change](const FCFMountIntent& Intent)
			{
				return Intent.MountProfileId == Change.MountIntent.MountProfileId;
			});
			return ExistingMount
				&& FCFMountIntent::StaticStruct()->CompareScriptStruct(ExistingMount, &Change.MountIntent, 0);
		}

		default:
			return false;
		}
	}

	// 검증이 끝난 normal R1 typed semantic command를 persistent Recipe에 적용합니다.
	bool ApplySemanticChangeToRecipe(UCFVehicleRecipeData& Recipe, const FCFVehicleSemanticChange& Change)
	{
		switch (Change.Operation)
		{
		case ECFVehicleSemanticOp::BindVehicleProfile:
			return SetPersistentProfileBinding(Recipe, Change.ProfileDomain, Change.ProfileAssetPath);
		case ECFVehicleSemanticOp::SetVehicleArchetype:
			Recipe.VehicleArchetypeId = Change.VehicleArchetypeId;
			return true;
		case ECFVehicleSemanticOp::SetVehicleAssetIntent:
			Recipe.AssetIntent = Change.AssetIntent;
			return true;
		case ECFVehicleSemanticOp::SetDrivingFeel:
		{
			// Preview에서 이미 검증된 partial patch를 persistent state에도 동일하게 적용할 진단 문자열입니다.
			FString IgnoredError;
			return ApplyDrivingFeelPatch(Recipe.DrivingFeelIntent, Change.DrivingFeelPatch, IgnoredError);
		}
		case ECFVehicleSemanticOp::SetMassIntent:
			Recipe.MassIntent = Change.MassIntent;
			return true;
		case ECFVehicleSemanticOp::SetDurabilityIntent:
			Recipe.DurabilityIntent = Change.DurabilityIntent;
			return true;
		case ECFVehicleSemanticOp::SetDefaultDataIntent:
			Recipe.DefaultDataIntent = Change.DefaultDataIntent;
			return true;
		case ECFVehicleSemanticOp::SetWheelVisualIntent:
			Recipe.WheelVisualIntent = Change.WheelVisualIntent;
			return true;
		case ECFVehicleSemanticOp::SetDriveStateMode:
			Recipe.DriveStateMode = Change.DriveStateMode;
			return true;
		case ECFVehicleSemanticOp::UpsertHardpointIntent:
		{
			// 동일 Stable-ID의 existing persistent Hardpoint입니다.
			FCFHardpointIntent* ExistingHardpoint = Recipe.HardpointIntents.FindByPredicate([&Change](const FCFHardpointIntent& Intent)
			{
				return Intent.LocationSlotId == Change.HardpointIntent.LocationSlotId;
			});
			if (ExistingHardpoint)
			{
				*ExistingHardpoint = Change.HardpointIntent;
			}
			else
			{
				Recipe.HardpointIntents.Add(Change.HardpointIntent);
			}
			return true;
		}
		case ECFVehicleSemanticOp::UpsertMountIntent:
		{
			// 동일 Stable-ID의 existing persistent Mount입니다.
			FCFMountIntent* ExistingMount = Recipe.MountIntents.FindByPredicate([&Change](const FCFMountIntent& Intent)
			{
				return Intent.MountProfileId == Change.MountIntent.MountProfileId;
			});
			if (ExistingMount)
			{
				*ExistingMount = Change.MountIntent;
			}
			else
			{
				Recipe.MountIntents.Add(Change.MountIntent);
			}
			return true;
		}
		default:
			return false;
		}
	}

	// Typed semantic command payload 자체를 stable approval/dedupe input으로 hash합니다.
	FString BuildSemanticPayloadHash(const FCFVehicleSemanticChange& Change)
	{
		// Raw field/value 없이 typed semantic payload만 기록할 canonical 문자열입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), FString::FromInt(static_cast<int32>(Change.Operation)));
		AppendToken(Payload, TEXT("ProfileDomain"), FString::FromInt(static_cast<int32>(Change.ProfileDomain)));
		AppendToken(Payload, TEXT("ProfileAssetPath"), Change.ProfileAssetPath.ToString());
		AppendToken(Payload, TEXT("VehicleArchetypeId"), Change.VehicleArchetypeId.ToString());

		AppendToken(Payload, TEXT("ChassisMesh"), Change.AssetIntent.ChassisMesh.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("WheelMeshFL"), Change.AssetIntent.WheelMeshFL.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("WheelMeshFR"), Change.AssetIntent.WheelMeshFR.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("WheelMeshRL"), Change.AssetIntent.WheelMeshRL.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("WheelMeshRR"), Change.AssetIntent.WheelMeshRR.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("BodyWheelSocketFL"), Change.AssetIntent.BodyWheelSocketFL.ToString());
		AppendToken(Payload, TEXT("BodyWheelSocketFR"), Change.AssetIntent.BodyWheelSocketFR.ToString());
		AppendToken(Payload, TEXT("BodyWheelSocketRL"), Change.AssetIntent.BodyWheelSocketRL.ToString());
		AppendToken(Payload, TEXT("BodyWheelSocketRR"), Change.AssetIntent.BodyWheelSocketRR.ToString());

		AppendToken(Payload, TEXT("SetAccelerationFeel"), Change.DrivingFeelPatch.bSetAccelerationFeel ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("AccelerationFeel"), CanonicalFloat(Change.DrivingFeelPatch.AccelerationFeel));
		AppendToken(Payload, TEXT("SetSteeringAgility"), Change.DrivingFeelPatch.bSetSteeringAgility ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("SteeringAgility"), CanonicalFloat(Change.DrivingFeelPatch.SteeringAgility));
		AppendToken(Payload, TEXT("SetGripFeel"), Change.DrivingFeelPatch.bSetGripFeel ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("GripFeel"), CanonicalFloat(Change.DrivingFeelPatch.GripFeel));
		AppendToken(Payload, TEXT("SetSuspensionFirmness"), Change.DrivingFeelPatch.bSetSuspensionFirmness ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("SuspensionFirmness"), CanonicalFloat(Change.DrivingFeelPatch.SuspensionFirmness));

		AppendToken(Payload, TEXT("BaseMassMode"), FString::FromInt(static_cast<int32>(Change.MassIntent.BaseMassMode)));
		AppendToken(Payload, TEXT("BaseMass"), CanonicalFloat(Change.MassIntent.ExplicitBaseMassKg));
		AppendToken(Payload, TEXT("GrossMassMode"), FString::FromInt(static_cast<int32>(Change.MassIntent.GrossMassMode)));
		AppendToken(Payload, TEXT("GrossMass"), CanonicalFloat(Change.MassIntent.ExplicitGrossMassKg));
		AppendToken(Payload, TEXT("MaxHealthMode"), FString::FromInt(static_cast<int32>(Change.DurabilityIntent.MaxHealthMode)));
		AppendToken(Payload, TEXT("MaxHealth"), CanonicalFloat(Change.DurabilityIntent.ExplicitMaxHealth));

		AppendToken(Payload, TEXT("DefenseMode"), FString::FromInt(static_cast<int32>(Change.DefaultDataIntent.DefenseMode)));
		AppendToken(Payload, TEXT("DefenseData"), Change.DefaultDataIntent.DefaultDefenseData.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("DestroyedFxMode"), FString::FromInt(static_cast<int32>(Change.DefaultDataIntent.DestroyedFxMode)));
		AppendToken(Payload, TEXT("DestroyedFxData"), Change.DefaultDataIntent.DefaultDestroyedFxData.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("DestroyedFxSocket"), Change.DefaultDataIntent.DestroyedFxSocketName.ToString());
		AppendToken(Payload, TEXT("WheelVisualMode"), FString::FromInt(static_cast<int32>(Change.WheelVisualIntent.Mode)));
		AppendToken(Payload, TEXT("DriveStateMode"), FString::FromInt(static_cast<int32>(Change.DriveStateMode)));

		AppendToken(Payload, TEXT("HardpointId"), Change.HardpointIntent.LocationSlotId.ToString());
		AppendToken(Payload, TEXT("HardpointCategory"), Change.HardpointIntent.LocationCategory.ToString());
		AppendToken(Payload, TEXT("HardpointSocket"), Change.HardpointIntent.SocketName.ToString());
		AppendToken(Payload, TEXT("MountId"), Change.MountIntent.MountProfileId.ToString());
		AppendToken(Payload, TEXT("MountLocationRef"), Change.MountIntent.LocationSlotRef.ToString());
		AppendToken(Payload, TEXT("MountType"), FString::FromInt(static_cast<int32>(Change.MountIntent.MountType)));
		AppendToken(Payload, TEXT("MountSize"), FString::FromInt(static_cast<int32>(Change.MountIntent.SizeLimit)));
		AppendToken(Payload, TEXT("MountPreset"), Change.MountIntent.DefaultEquipmentPresetData.ToSoftObjectPath().ToString());
		AppendToken(Payload, TEXT("MountExposed"), Change.MountIntent.bExposedModule ? TEXT("1") : TEXT("0"));
		return HashUtf8Payload(Payload);
	}

	// R1 prospective state와 exact typed command를 approval scope hash로 binding합니다.
	FString BuildRecipeProposalHash(
		const UCFVehicleRecipeData& Recipe,
		const UCFVehicleData& Target,
		const FCFVehicleSemanticChange& Change,
		const FCFVehicleRecipeSnapshot& BaselineRecipe,
		const FCFVehicleDefinitionSnapshot& CurrentDefinition,
		const FCFVehicleResolveResult& ProspectiveResolve,
		const FString& ProspectiveRecipeFingerprint,
		const FString& DiffHash)
	{
		// Section 25 approval binding을 canonical token으로 결합할 payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), GetSemanticOperationName(Change.Operation).ToString());
		AppendToken(Payload, TEXT("Risk"), TEXT("R1"));
		AppendToken(Payload, TEXT("RecipePath"), FSoftObjectPath(&Recipe).ToString());
		AppendToken(Payload, TEXT("RecipeId"), Recipe.RecipeId.ToString(EGuidFormats::Digits));
		AppendToken(Payload, TEXT("TargetPath"), FSoftObjectPath(&Target).ToString());
		AppendToken(Payload, TEXT("SemanticPayloadHash"), BuildSemanticPayloadHash(Change));
		AppendToken(Payload, TEXT("ExpectedRecipeFingerprint"), BaselineRecipe.RecipeFingerprint);
		AppendToken(Payload, TEXT("ExpectedTargetDefinitionHash"), CurrentDefinition.DefinitionHash);
		AppendToken(Payload, TEXT("ProspectiveRecipeFingerprint"), ProspectiveRecipeFingerprint);
		AppendToken(Payload, TEXT("ProspectiveSourceSignature"), ProspectiveResolve.SourceSignature);
		AppendToken(Payload, TEXT("ProspectiveResolvedDefinitionHash"), ProspectiveResolve.ResolvedDefinitionHash);
		AppendToken(Payload, TEXT("DiffHash"), DiffHash);
		AppendToken(Payload, TEXT("ResolverContractRevision"), FString::FromInt(ProspectiveResolve.ResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// R3 reviewed Apply evidence를 exact DefinitionApply approval scope hash로 binding합니다.
	FString BuildApplyProposalHash(const FCFVehicleApplyRequest& ApplyRequest, const FString& DiffHash)
	{
		// Section 25.53 R3 approval evidence canonical payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), TEXT("ApplyResolvedVehicle"));
		AppendToken(Payload, TEXT("Risk"), TEXT("R3"));
		AppendToken(Payload, TEXT("RecipePath"), ApplyRequest.Recipe ? FSoftObjectPath(ApplyRequest.Recipe).ToString() : FString());
		AppendToken(Payload, TEXT("RecipeId"), ApplyRequest.Recipe ? ApplyRequest.Recipe->RecipeId.ToString(EGuidFormats::Digits) : FString());
		AppendToken(Payload, TEXT("TargetPath"), ApplyRequest.TargetVehicleData ? FSoftObjectPath(ApplyRequest.TargetVehicleData).ToString() : FString());
		AppendToken(Payload, TEXT("ExpectedRecipeFingerprint"), ApplyRequest.ExpectedRecipeFingerprint);
		AppendToken(Payload, TEXT("ExpectedSourceSignature"), ApplyRequest.ExpectedSourceSignature);
		AppendToken(Payload, TEXT("ExpectedTargetDefinitionHash"), ApplyRequest.ExpectedTargetDefinitionHash);
		AppendToken(Payload, TEXT("ExpectedResolvedDefinitionHash"), ApplyRequest.ExpectedResolvedDefinitionHash);
		AppendToken(Payload, TEXT("DiffHash"), DiffHash);
		AppendToken(Payload, TEXT("ResolverContractRevision"), FString::FromInt(ApplyRequest.ExpectedResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// R1 dedupe가 같은 ClientOperationId의 logical request 차이를 판정할 request hash를 만듭니다.
	FString BuildRecipeWriteRequestHash(const FCFVehicleRecipeChangeRequest& Request)
	{
		// Caller-supplied stable desired request만 포함하고 current server state는 포함하지 않는 payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), GetSemanticOperationName(Request.Change.Operation).ToString());
		AppendToken(Payload, TEXT("SemanticPayloadHash"), BuildSemanticPayloadHash(Request.Change));
		AppendToken(Payload, TEXT("ApprovalClass"), FString::FromInt(static_cast<int32>(Request.CallContext.ApprovalClass)));
		AppendToken(Payload, TEXT("ApprovalScopeHash"), Request.CallContext.ApprovalScopeHash);
		AppendToken(Payload, TEXT("ExpectedRecipeFingerprint"), Request.CallContext.ExpectedRecipeFingerprint);
		AppendToken(Payload, TEXT("ExpectedTargetDefinitionHash"), Request.CallContext.ExpectedTargetDefinitionHash);
		AppendToken(Payload, TEXT("ExpectedResolverContractRevision"), FString::FromInt(Request.CallContext.ExpectedResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// R3 dedupe가 같은 ClientOperationId의 logical request 차이를 판정할 request hash를 만듭니다.
	FString BuildApplyWriteRequestHash(const FCFVehicleApplyOpRequest& Request)
	{
		// Caller-supplied R3 evidence/approval만 포함하는 payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Operation"), TEXT("ApplyResolvedVehicle"));
		AppendToken(Payload, TEXT("ApprovalClass"), FString::FromInt(static_cast<int32>(Request.CallContext.ApprovalClass)));
		AppendToken(Payload, TEXT("ApprovalScopeHash"), Request.CallContext.ApprovalScopeHash);
		AppendToken(Payload, TEXT("ContextExpectedRecipe"), Request.CallContext.ExpectedRecipeFingerprint);
		AppendToken(Payload, TEXT("ContextExpectedTarget"), Request.CallContext.ExpectedTargetDefinitionHash);
		AppendToken(Payload, TEXT("ContextResolverRevision"), FString::FromInt(Request.CallContext.ExpectedResolverContractRevision));
		AppendToken(Payload, TEXT("ApplyExpectedRecipe"), Request.ApplyRequest.ExpectedRecipeFingerprint);
		AppendToken(Payload, TEXT("ApplyExpectedSource"), Request.ApplyRequest.ExpectedSourceSignature);
		AppendToken(Payload, TEXT("ApplyExpectedTarget"), Request.ApplyRequest.ExpectedTargetDefinitionHash);
		AppendToken(Payload, TEXT("ApplyExpectedResolved"), Request.ApplyRequest.ExpectedResolvedDefinitionHash);
		AppendToken(Payload, TEXT("ExpectedDiffHash"), Request.ExpectedDiffHash);
		AppendToken(Payload, TEXT("ApplyResolverRevision"), FString::FromInt(Request.ApplyRequest.ExpectedResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// Mutation 시작 전에 ClientOperationId cache를 확인하고 replay/conflict 여부를 반환합니다.
	bool CheckDedupe(
		const FString& ClientOperationId,
		const FString& RequestHash,
		FCFAuthoringOpResult& OutResult,
		bool& bOutHandled)
	{
		bOutHandled = false;
		if (ClientOperationId.IsEmpty())
		{
			SetBlocked(OutResult, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("Mutation operation에는 non-empty ClientOperationId가 필요합니다."));
			bOutHandled = true;
			return false;
		}

		// 같은 ClientOperationId의 기존 terminal entry입니다.
		const FDedupeEntry* ExistingEntry = GDedupeCache.Find(ClientOperationId);
		if (!ExistingEntry)
		{
			return true;
		}
		if (ExistingEntry->RequestHash != RequestHash)
		{
			SetConflict(OutResult, ECFAuthoringErrorCode::OperationIdConflict, TEXT("같은 ClientOperationId가 다른 mutation request에 재사용되었습니다."));
			bOutHandled = true;
			return false;
		}

		OutResult = ExistingEntry->Result;
		OutResult.bResultReplayedFromDedupe = true;
		OutResult.Mutation.bRecipeChanged = false;
		OutResult.Mutation.bTargetChanged = false;
		OutResult.Mutation.bProfileChanged = false;
		OutResult.Mutation.bCreatedAssets = false;
		OutResult.Mutation.bAutomaticRetryPerformed = false;
		OutResult.Message += TEXT(" [Editor lifetime dedupe replay: mutation 반복 0]");
		bOutHandled = true;
		return true;
	}

	// Mutation terminal result를 bounded Editor-lifetime dedupe cache에 기록합니다.
	void StoreDedupe(const FString& ClientOperationId, const FString& RequestHash, const FCFAuthoringOpResult& Result)
	{
		if (ClientOperationId.IsEmpty())
		{
			return;
		}
		if (!GDedupeCache.Contains(ClientOperationId))
		{
			GDedupeOrder.Add(ClientOperationId);
		}

		// 새 cache entry입니다.
		FDedupeEntry& Entry = GDedupeCache.FindOrAdd(ClientOperationId);
		Entry.RequestHash = RequestHash;
		Entry.Result = Result;
		while (GDedupeOrder.Num() > MaxDedupeEntries)
		{
			// 가장 오래된 insertion key입니다.
			const FString OldestKey = GDedupeOrder[0];
			GDedupeOrder.RemoveAt(0);
			GDedupeCache.Remove(OldestKey);
		}
	}

	// Same-class reflected UPROPERTY storage 전체를 backup object에서 persistent Recipe로 복원합니다.
	bool RestoreRecipeFromBackup(UCFVehicleRecipeData& Recipe, const UCFVehicleRecipeData& Backup, FString& OutError)
	{
		if (Recipe.GetClass() != Backup.GetClass())
		{
			OutError = TEXT("Recipe rollback backup class가 current Recipe class와 다릅니다.");
			return false;
		}
		for (TFieldIterator<FProperty> PropertyIt(Recipe.GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			// 같은 class backup에서 복원할 reflected property입니다.
			FProperty* Property = *PropertyIt;
			Property->CopyCompleteValue_InContainer(&Recipe, &Backup);
		}
		OutError.Reset();
		return true;
	}

	// Profile UObject 하나를 shared SnapshotBuilder로 읽고 list/read metadata를 추출합니다.
	bool BuildSingleProfileSnapshot(
		const ECFVehicleProfileDomain Domain,
		UObject& ProfileObject,
		FCFVehicleProfileSnapshotSet& OutSnapshot,
		FText& OutDisplayName,
		int32& OutRevision,
		FString& OutFingerprint,
		FString& OutError)
	{
		// Domain별 typed Profile pointer입니다.
		UCFVehicleBaseProfile* BaseProfile = Domain == ECFVehicleProfileDomain::VehicleBase ? Cast<UCFVehicleBaseProfile>(&ProfileObject) : nullptr;
		// Domain별 typed Profile pointer입니다.
		UCFDrivetrainProfile* DrivetrainProfile = Domain == ECFVehicleProfileDomain::Drivetrain ? Cast<UCFDrivetrainProfile>(&ProfileObject) : nullptr;
		// Domain별 typed Profile pointer입니다.
		UCFHandlingProfile* HandlingProfile = Domain == ECFVehicleProfileDomain::Handling ? Cast<UCFHandlingProfile>(&ProfileObject) : nullptr;
		// Domain별 typed Profile pointer입니다.
		UCFPerformanceProfile* PerformanceProfile = Domain == ECFVehicleProfileDomain::Performance ? Cast<UCFPerformanceProfile>(&ProfileObject) : nullptr;
		// Domain별 typed Profile pointer입니다.
		UCFDriveStateProfile* DriveStateProfile = Domain == ECFVehicleProfileDomain::DriveState ? Cast<UCFDriveStateProfile>(&ProfileObject) : nullptr;

		if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			BaseProfile,
			DrivetrainProfile,
			HandlingProfile,
			PerformanceProfile,
			DriveStateProfile,
			OutSnapshot,
			OutError))
		{
			return false;
		}

		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			if (!BaseProfile) return false;
			OutDisplayName = BaseProfile->Meta.DisplayName;
			OutRevision = OutSnapshot.BaseSource.AuthoringRevision;
			OutFingerprint = OutSnapshot.BaseSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::Drivetrain:
			if (!DrivetrainProfile) return false;
			OutDisplayName = DrivetrainProfile->Meta.DisplayName;
			OutRevision = OutSnapshot.DrivetrainSource.AuthoringRevision;
			OutFingerprint = OutSnapshot.DrivetrainSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::Handling:
			if (!HandlingProfile) return false;
			OutDisplayName = HandlingProfile->Meta.DisplayName;
			OutRevision = OutSnapshot.HandlingSource.AuthoringRevision;
			OutFingerprint = OutSnapshot.HandlingSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::Performance:
			if (!PerformanceProfile) return false;
			OutDisplayName = PerformanceProfile->Meta.DisplayName;
			OutRevision = OutSnapshot.PerformanceSource.AuthoringRevision;
			OutFingerprint = OutSnapshot.PerformanceSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::DriveState:
			if (!DriveStateProfile) return false;
			OutDisplayName = DriveStateProfile->Meta.DisplayName;
			OutRevision = OutSnapshot.DriveStateSource.AuthoringRevision;
			OutFingerprint = OutSnapshot.DriveStateSource.ProfileFingerprint;
			break;
		default:
			OutError = TEXT("지원하지 않는 Profile Domain입니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Asset Registry에서 exact class의 assets를 반환합니다.
	void GetAssetsByClass(const UClass& AssetClass, TArray<FAssetData>& OutAssets)
	{
		// Project Asset Registry module입니다.
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		// ClassPath filter를 사용할 project registry입니다.
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		// exact/derived class asset query filter입니다.
		FARFilter Filter;
		Filter.ClassPaths.Add(AssetClass.GetClassPathName());
		Filter.bRecursiveClasses = true;
		OutAssets.Reset();
		AssetRegistry.GetAssets(Filter, OutAssets);
	}

	// Stable Field Path 두 개가 exact selector 포함 canonical identity로 같은지 비교합니다.
	bool AreFieldPathsEqual(const FCFVehicleFieldPath& Left, const FCFVehicleFieldPath& Right)
	{
		return Left.ToCanonicalString(true) == Right.ToCanonicalString(true);
	}
}

// Project Asset Registry에서 Vehicle Definition/Recipe identity를 read-only로 나열합니다.
bool FCFVehicleAuthoringService::ListVehicles(const FCFVehicleListRequest& Request, FCFVehicleListResult& OutResult)
{
	OutResult = FCFVehicleListResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ListVehicles"), ECFAuthoringRiskClass::R0_ReadOnly);

		// Recipe가 연결한 Definition path 집합입니다.
	TSet<FString> DefinitionPathsWithRecipe;
	// Project Asset Registry의 모든 canonical UCFVehicleData assets입니다.
	TArray<FAssetData> DefinitionAssets;
	CFVehicleAuthoringPrivate::GetAssetsByClass(*UCFVehicleData::StaticClass(), DefinitionAssets);
		// Existing Definition이 실제 사용하는 Chassis StaticMesh path 집합입니다.
	TSet<FString> UsedChassisMeshPaths;
	// Existing Definition이 wheel visual로 사용하는 StaticMesh path 집합이며 차체 후보에서 제외합니다.
	TSet<FString> KnownWheelMeshPaths;
		// Mesh-only Candidate를 bounded query할 common vehicle mesh collection roots입니다.
	TSet<FString> ChassisMeshCollectionRoots;
	for (const FAssetData& DefinitionAssetData : DefinitionAssets)
	{
		// Chassis reference inventory를 읽기 위해 load한 canonical Definition입니다.
		const UCFVehicleData* Definition = Cast<UCFVehicleData>(DefinitionAssetData.GetAsset());
		if (!Definition || !Definition->VehicleVisualConfig.ChassisMesh)
		{
			continue;
		}
				// Existing canonical Chassis mesh identity입니다.
		const FString MeshObjectPath = FSoftObjectPath(Definition->VehicleVisualConfig.ChassisMesh).ToString();
		UsedChassisMeshPaths.Add(MeshObjectPath);

		// VehicleData가 명시적으로 wheel visual로 참조하는 mesh를 차체 후보에서 제외하는 helper입니다.
		auto AddKnownWheelMesh = [&KnownWheelMeshPaths](const TObjectPtr<UStaticMesh>& WheelMesh)
		{
			if (WheelMesh)
			{
				KnownWheelMeshPaths.Add(FSoftObjectPath(WheelMesh).ToString());
			}
		};
		AddKnownWheelMesh(Definition->VehicleVisualConfig.WheelMeshFL);
		AddKnownWheelMesh(Definition->VehicleVisualConfig.WheelMeshFR);
		AddKnownWheelMesh(Definition->VehicleVisualConfig.WheelMeshRL);
		AddKnownWheelMesh(Definition->VehicleVisualConfig.WheelMeshRR);

				// Current chassis asset이 속한 exact vehicle subdirectory입니다.
		const FString MeshPackagePath = FPackageName::GetLongPackagePath(Definition->VehicleVisualConfig.ChassisMesh->GetOutermost()->GetName());
		// SUV/Sedan처럼 각 차량 폴더의 한 단계 위 common mesh collection root입니다.
		const FString MeshCollectionRoot = FPackageName::GetLongPackagePath(MeshPackagePath);
		if (!MeshCollectionRoot.IsEmpty())
		{
			ChassisMeshCollectionRoots.Add(MeshCollectionRoot);
		}
	}
	if (Request.bIncludeRecipeRecords)
	{
		// Project Asset Registry의 Recipe assets입니다.
		TArray<FAssetData> RecipeAssets;
		CFVehicleAuthoringPrivate::GetAssetsByClass(*UCFVehicleRecipeData::StaticClass(), RecipeAssets);
		for (const FAssetData& RecipeAssetData : RecipeAssets)
		{
			// Read-only context summary를 위해 load한 Editor-only Recipe입니다.
			UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(RecipeAssetData.GetAsset());
			if (!Recipe)
			{
				continue;
			}

			// Recipe가 binding한 Definition path입니다.
			const FSoftObjectPath DefinitionPath = Recipe->TargetVehicleData.ToSoftObjectPath();
			// Search filter에 사용할 combined identity text입니다.
			const FString SearchIdentity = RecipeAssetData.GetSoftObjectPath().ToString() + TEXT(" ") + DefinitionPath.ToString();
			if (!Request.SearchText.IsEmpty() && !SearchIdentity.Contains(Request.SearchText, ESearchCase::IgnoreCase))
			{
				continue;
			}

			// 하나의 managed/imported Recipe record입니다.
			FCFVehicleListEntry& Entry = OutResult.Vehicles.AddDefaulted_GetRef();
			Entry.DefinitionPath = DefinitionPath;
			Entry.RecipePath = RecipeAssetData.GetSoftObjectPath();
			Entry.RecipeId = Recipe->RecipeId;
			Entry.ManageState = Recipe->ImportState.ManageState;
			Entry.AdvancedOverrideCount = Recipe->AdvancedOverrides.Num();
			if (DefinitionPath.IsValid())
			{
				DefinitionPathsWithRecipe.Add(DefinitionPath.ToString());
			}
		}
	}

		if (Request.bIncludeUnmanagedDefinitions)
	{
		for (const FAssetData& DefinitionAssetData : DefinitionAssets)
		{
			// Current Definition identity입니다.
			const FSoftObjectPath DefinitionPath = DefinitionAssetData.GetSoftObjectPath();
			if (DefinitionPathsWithRecipe.Contains(DefinitionPath.ToString()))
			{
				continue;
			}
			if (!Request.SearchText.IsEmpty()
				&& !DefinitionPath.ToString().Contains(Request.SearchText, ESearchCase::IgnoreCase))
			{
				continue;
			}

			// Recipe가 없는 raw Definition record입니다.
			FCFVehicleListEntry& Entry = OutResult.Vehicles.AddDefaulted_GetRef();
			Entry.DefinitionPath = DefinitionPath;
			Entry.ManageState = ECFVehicleManageState::Unmanaged;
		}
		}

		if (Request.bIncludeMeshCandidates && !ChassisMeshCollectionRoots.IsEmpty())
	{
		// Bounded StaticMesh query를 수행할 Asset Registry입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
				// Existing Chassis와 같은 bounded vehicle mesh collection root 아래 StaticMesh만 조회하는 filter입니다.
		FARFilter MeshFilter;
		MeshFilter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
		MeshFilter.bRecursivePaths = true;
				for (const FString& CollectionRoot : ChassisMeshCollectionRoots)
		{
			MeshFilter.PackagePaths.Add(*CollectionRoot);
		}
		// Bounded vehicle mesh collection assets입니다.
		TArray<FAssetData> MeshAssets;
		AssetRegistry.GetAssets(MeshFilter, MeshAssets);
		for (const FAssetData& MeshAssetData : MeshAssets)
		{
			// Exact StaticMesh object identity입니다.
			const FSoftObjectPath MeshPath = MeshAssetData.GetSoftObjectPath();
						if (UsedChassisMeshPaths.Contains(MeshPath.ToString()) || KnownWheelMeshPaths.Contains(MeshPath.ToString()))
			{
				continue;
			}
			if (!Request.SearchText.IsEmpty() && !MeshPath.ToString().Contains(Request.SearchText, ESearchCase::IgnoreCase))
			{
				continue;
			}
			// Definition이 아닌 explicit Mesh-only Candidate row입니다.
			FCFVehicleListEntry& Entry = OutResult.Vehicles.AddDefaulted_GetRef();
			Entry.ManageState = ECFVehicleManageState::Unmanaged;
			Entry.bMeshOnlyCandidate = true;
			Entry.ChassisMeshPath = MeshPath;
		}
	}

	OutResult.Vehicles.Sort([](const FCFVehicleListEntry& Left, const FCFVehicleListEntry& Right)
	{
		// Managed/unmanaged Definition 또는 Mesh Candidate를 동일한 stable identity space로 정렬합니다.
		const FString LeftIdentity = Left.bMeshOnlyCandidate ? Left.ChassisMeshPath.ToString() : Left.DefinitionPath.ToString();
		// Managed/unmanaged Definition 또는 Mesh Candidate를 동일한 stable identity space로 정렬합니다.
		const FString RightIdentity = Right.bMeshOnlyCandidate ? Right.ChassisMeshPath.ToString() : Right.DefinitionPath.ToString();
		// Definition path가 같을 때 Recipe path로 안정 정렬합니다.
		const FString LeftKey = FString::Printf(TEXT("%d|%s|%s"), Left.bMeshOnlyCandidate ? 1 : 0, *LeftIdentity, *Left.RecipePath.ToString());
		// Definition path가 같을 때 Recipe path로 안정 정렬합니다.
		const FString RightKey = FString::Printf(TEXT("%d|%s|%s"), Right.bMeshOnlyCandidate ? 1 : 0, *RightIdentity, *Right.RecipePath.ToString());
		return LeftKey < RightKey;
	});

		CFVehicleAuthoringPrivate::SetSucceeded(OutResult.Operation, TEXT("차량/레시피/차체 메시 후보 목록을 읽기 전용으로 조회했습니다."));
	return true;
}

// Project Asset Registry에서 Frozen 5-domain Profile을 read-only로 나열합니다.
bool FCFVehicleAuthoringService::ListProfiles(const FCFProfileListRequest& Request, FCFProfileListResult& OutResult)
{
	OutResult = FCFProfileListResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ListProfiles"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Domain 하나의 AssetRegistry query를 수행하는 helper입니다.
	auto AppendDomain = [&Request, &OutResult](const ECFVehicleProfileDomain Domain)
	{
		if (Request.Domain != ECFVehicleProfileDomain::None && Request.Domain != Domain)
		{
			return;
		}
		// Domain별 expected Profile class입니다.
		const UClass* ProfileClass = CFVehicleAuthoringPrivate::GetProfileClass(Domain);
		if (!ProfileClass)
		{
			return;
		}
		// 해당 Domain의 registered assets입니다.
		TArray<FAssetData> Assets;
		CFVehicleAuthoringPrivate::GetAssetsByClass(*ProfileClass, Assets);
		for (const FAssetData& AssetData : Assets)
		{
			// Metadata/fingerprint read를 위해 load한 existing Profile UObject입니다.
			UObject* ProfileObject = AssetData.GetAsset();
			if (!ProfileObject || !ProfileObject->IsA(ProfileClass))
			{
				continue;
			}
			// Shared SnapshotBuilder가 생성할 one-domain Snapshot입니다.
			FCFVehicleProfileSnapshotSet ProfileSnapshot;
			// Profile UI 표시 이름입니다.
			FText DisplayName;
			// Profile diagnostic revision입니다.
			int32 Revision = 0;
			// Resolver payload fingerprint입니다.
			FString Fingerprint;
			// Snapshot build failure diagnostic입니다.
			FString Error;
			if (!CFVehicleAuthoringPrivate::BuildSingleProfileSnapshot(Domain, *ProfileObject, ProfileSnapshot, DisplayName, Revision, Fingerprint, Error))
			{
				continue;
			}

			// Lightweight profile list row입니다.
			FCFProfileListEntry& Entry = OutResult.Profiles.AddDefaulted_GetRef();
			Entry.Domain = Domain;
			Entry.ProfilePath = AssetData.GetSoftObjectPath();
			Entry.DisplayName = DisplayName;
			Entry.AuthoringRevision = Revision;
			Entry.ProfileFingerprint = Fingerprint;
		}
	};

	AppendDomain(ECFVehicleProfileDomain::VehicleBase);
	AppendDomain(ECFVehicleProfileDomain::Drivetrain);
	AppendDomain(ECFVehicleProfileDomain::Handling);
	AppendDomain(ECFVehicleProfileDomain::Performance);
	AppendDomain(ECFVehicleProfileDomain::DriveState);

	OutResult.Profiles.Sort([](const FCFProfileListEntry& Left, const FCFProfileListEntry& Right)
	{
		if (Left.Domain != Right.Domain)
		{
			return static_cast<int32>(Left.Domain) < static_cast<int32>(Right.Domain);
		}
		return Left.ProfilePath.ToString() < Right.ProfilePath.ToString();
	});

	CFVehicleAuthoringPrivate::SetSucceeded(OutResult.Operation, TEXT("Frozen 5-domain Profile records를 read-only로 조회했습니다."));
	return true;
}

// Exact existing Profile을 shared SnapshotBuilder로 read-only snapshot합니다.
bool FCFVehicleAuthoringService::ReadProfile(const FCFProfileReadRequest& Request, FCFProfileReadResult& OutResult)
{
	OutResult = FCFProfileReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ReadProfile"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Request Domain이 기대하는 exact Profile class입니다.
	const UClass* ProfileClass = CFVehicleAuthoringPrivate::GetProfileClass(Request.Domain);
	if (!ProfileClass || !Request.ProfilePath.IsValid())
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, TEXT("ReadProfile에는 exact Domain과 Profile path가 필요합니다."));
		return false;
	}
	// Profile load failure taxonomy입니다.
	ECFAuthoringErrorCode ErrorCode = ECFAuthoringErrorCode::None;
	// Profile load/snapshot diagnostic입니다.
	FString Error;
	// Exact typed existing Profile object입니다.
	UObject* ProfileObject = CFVehicleAuthoringPrivate::LoadTypedObject(Request.ProfilePath, ProfileClass, ErrorCode, Error);
	if (!ProfileObject)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ErrorCode, Error);
		return false;
	}

	// Metadata extraction에 사용하지 않는 표시 이름입니다.
	FText DisplayName;
	// Metadata extraction에 사용하지 않는 revision입니다.
	int32 Revision = 0;
	// Metadata extraction에 사용하지 않는 fingerprint입니다.
	FString Fingerprint;
	if (!CFVehicleAuthoringPrivate::BuildSingleProfileSnapshot(Request.Domain, *ProfileObject, OutResult.Snapshot, DisplayName, Revision, Fingerprint, Error))
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, Error);
		return false;
	}

	OutResult.Domain = Request.Domain;
	CFVehicleAuthoringPrivate::SetSucceeded(OutResult.Operation, TEXT("Profile을 shared SnapshotBuilder로 읽었습니다."));
	return true;
}

// Write-before-read authority에 사용할 bounded Vehicle/Recipe context를 반환합니다.
bool FCFVehicleAuthoringService::ReadVehicleContext(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleContextReadResult& OutResult)
{
	OutResult = FCFVehicleContextReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ReadVehicleContext"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Shared Core의 fresh resolve state입니다.
	CFVehicleAuthoringPrivate::FFreshResolveState FreshState;
	if (!CFVehicleAuthoringPrivate::BuildFreshResolveState(Request, FreshState, OutResult.Operation))
	{
		return false;
	}

	OutResult.RecipeId = FreshState.ResolveRequest.Recipe.RecipeId;
	OutResult.RecipePath = FSoftObjectPath(Request.Recipe);
	OutResult.DefinitionPath = FSoftObjectPath(FreshState.TargetVehicleData);
	OutResult.ManageState = FreshState.ResolveRequest.Recipe.ImportState.ManageState;
	OutResult.ProfileBindings = FreshState.ResolveRequest.Recipe.ProfileBindings;
	OutResult.LegacyPinnedFieldCount = FreshState.ResolveRequest.Recipe.ImportState.LegacyPinnedFields.Num();
	OutResult.LegacySerializedFieldCount = FreshState.ResolveRequest.Recipe.ImportState.LegacySerializedFields.Num();
	OutResult.AdvancedOverrideCount = FreshState.ResolveRequest.Recipe.AdvancedOverrides.Num();
	OutResult.AppliedDefinitionHash = FreshState.ResolveRequest.Recipe.AppliedState.AppliedDefinitionHash;
	OutResult.StaleReport = FreshState.ResolveResult.StaleReport;
	CFVehicleAuthoringPrivate::SetSucceeded(OutResult.Operation, TEXT("Vehicle Authoring context를 fresh shared Core로 읽었습니다."));
	return true;
}

// Live UObject를 shared SnapshotBuilder/AssetReader로 끊은 뒤 기존 Pure Resolver를 호출합니다.
bool FCFVehicleAuthoringService::ResolveVehiclePreview(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleResolveReadResult& OutResult)
{
	OutResult = FCFVehicleResolveReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ResolveVehiclePreview"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Shared Core의 fresh resolve state입니다.
	CFVehicleAuthoringPrivate::FFreshResolveState FreshState;
	if (!CFVehicleAuthoringPrivate::BuildFreshResolveState(Request, FreshState, OutResult.Operation))
	{
		return false;
	}
	OutResult.ResolveRequest = FreshState.ResolveRequest;
	OutResult.ResolveResult = FreshState.ResolveResult;
	CFVehicleAuthoringPrivate::SetSucceeded(OutResult.Operation, TEXT("Shared Pure Resolver Preview를 계산했습니다."));
	return true;
}

// Shared Resolver FieldDiff를 별도 비교 로직 없이 반환합니다.
bool FCFVehicleAuthoringService::ReadPendingDiff(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleDiffReadResult& OutResult)
{
	OutResult = FCFVehicleDiffReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ReadPendingDiff"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Shared Resolver Preview 결과입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("ReadPendingDiff");
		return false;
	}
	OutResult.FieldDiff = ResolveRead.ResolveResult.FieldDiff;
	OutResult.DiffHash = BuildDiffHash(OutResult.FieldDiff);
	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = TEXT("ReadPendingDiff");
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.CurrentDiffHash = OutResult.DiffHash;
	OutResult.Operation.Message = TEXT("Shared Resolver FieldDiff를 읽었습니다.");
	return true;
}

// Shared Resolver SourceTrace를 optional exact Stable Field Path로 projection합니다.
bool FCFVehicleAuthoringService::ReadSourceTrace(
	const FCFVehicleAuthoringReadRequest& Request,
	const TArray<FCFVehicleFieldPath>& OptionalFieldPaths,
	FCFVehicleTraceReadResult& OutResult)
{
	OutResult = FCFVehicleTraceReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ReadSourceTrace"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Shared Resolver Preview 결과입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("ReadSourceTrace");
		return false;
	}

	if (OptionalFieldPaths.IsEmpty())
	{
		OutResult.SourceTrace = ResolveRead.ResolveResult.PreviewSourceTrace;
	}
	else
	{
		for (const FCFVehicleSourceTrace& Trace : ResolveRead.ResolveResult.PreviewSourceTrace)
		{
			// 이 Trace가 requested exact field 중 하나인지 여부입니다.
			const bool bRequested = OptionalFieldPaths.ContainsByPredicate([&Trace](const FCFVehicleFieldPath& RequestedPath)
			{
				return CFVehicleAuthoringPrivate::AreFieldPathsEqual(Trace.FieldPath, RequestedPath);
			});
			if (bRequested)
			{
				OutResult.SourceTrace.Add(Trace);
			}
		}
	}

	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = TEXT("ReadSourceTrace");
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.Message = TEXT("Shared Resolver Source Trace를 읽었습니다.");
	return true;
}

// Recipe/Resolver/Definition/Sync validation layer를 shared Resolver result에서 반환합니다.
bool FCFVehicleAuthoringService::ReadValidation(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleValidationReadResult& OutResult)
{
	OutResult = FCFVehicleValidationReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ReadValidation"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Shared Resolver Preview 결과입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("ReadValidation");
		return false;
	}
	OutResult.RecipeValidation = ResolveRead.ResolveResult.RecipeValidation;
	OutResult.ResolverValidation = ResolveRead.ResolveResult.ResolverValidation;
	OutResult.DefinitionValidation = ResolveRead.ResolveResult.DefinitionValidation;
	OutResult.StaleReport = ResolveRead.ResolveResult.StaleReport;
	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = TEXT("ReadValidation");
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.Message = TEXT("Shared Resolver validation layers를 읽었습니다.");
	return true;
}

// Shared R16 StaleReport를 External Drift read contract로 반환합니다.
bool FCFVehicleAuthoringService::ReviewExternalDrift(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleDriftReadResult& OutResult)
{
	OutResult = FCFVehicleDriftReadResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("ReviewExternalDrift"), ECFAuthoringRiskClass::R0_ReadOnly);

	// Shared Resolver Preview 결과입니다.
	FCFVehicleResolveReadResult ResolveRead;
	if (!ResolveVehiclePreview(Request, ResolveRead))
	{
		OutResult.Operation = ResolveRead.Operation;
		OutResult.Operation.OperationName = TEXT("ReviewExternalDrift");
		return false;
	}
	OutResult.StaleReport = ResolveRead.ResolveResult.StaleReport;
	OutResult.Operation = ResolveRead.Operation;
	OutResult.Operation.OperationName = TEXT("ReviewExternalDrift");
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.Message = TEXT("Shared R16 Stale/External Drift report를 읽었습니다. persistent decision은 수행하지 않았습니다.");
	return true;
}

// Existing unmanaged Definition을 수정하지 않고 exact Initial Import summary/approval proposal을 만듭니다.
bool FCFVehicleAuthoringService::BuildInitialImportProposal(
	const FCFVehicleInitialImportRequest& Request,
	FCFVehicleInitialImportPreviewResult& OutResult)
{
	OutResult = FCFVehicleInitialImportPreviewResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("BuildInitialImportProposal"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!Request.TargetVehicleData)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Initial Import 대상 VehicleData가 없습니다."));
		return false;
	}

	// Reviewed Recipe destination package name입니다.
	FString RecipePackageName;
	// Reviewed Recipe destination object path입니다.
	FSoftObjectPath RecipeObjectPath;
	// Destination/import preview diagnostic입니다.
	FString PreviewError;
	if (!CFVehicleAuthoringPrivate::BuildInitialImportIdentity(Request, RecipePackageName, RecipeObjectPath, PreviewError))
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, PreviewError);
		return false;
	}

	// 이미 Target을 소유하는 Recipe path가 있는 경우 채울 diagnostic입니다.
	FString ExistingRecipePath;
	if (CFVehicleAuthoringPrivate::HasExistingRecipeForTarget(*Request.TargetVehicleData, ExistingRecipePath))
	{
		CFVehicleAuthoringPrivate::SetBlocked(
			OutResult.Operation,
			ECFAuthoringErrorCode::UnmanagedRequired,
			FString::Printf(TEXT("Initial Import는 unmanaged Definition에만 가능합니다. Existing Recipe=%s"), *ExistingRecipePath));
		return false;
	}

		// Destination exact object가 현재 loaded state에 이미 존재하는지 확인합니다.
	UObject* ExistingDestination = RecipeObjectPath.ResolveObject();
	// Destination exact asset이 Asset Registry에 이미 존재하는지 non-loading으로 확인합니다.
	FAssetRegistryModule& DestinationRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// Disk/registered destination asset metadata입니다. 새 asset의 정상적인 미존재를 TryLoad warning으로 만들지 않습니다.
	const FAssetData ExistingDestinationAsset = DestinationRegistryModule.Get().GetAssetByObjectPath(RecipeObjectPath);
	if (ExistingDestination || ExistingDestinationAsset.IsValid())
	{
		CFVehicleAuthoringPrivate::SetBlocked(
			OutResult.Operation,
			ECFAuthoringErrorCode::StableIdConflict,
			FString::Printf(TEXT("Initial Import Recipe destination이 이미 존재합니다: %s"), *RecipeObjectPath.ToString()));
		return false;
	}

	// Current Target의 immutable full Definition snapshot입니다.
	FCFVehicleDefinitionSnapshot DefinitionSnapshot;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Request.TargetVehicleData, DefinitionSnapshot, PreviewError))
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, PreviewError);
		return false;
	}

	// Existing Import Core가 persistent mutation 없이 계산한 exact import summary입니다.
	FCFVehicleImportResult ImportSummary;
	if (!FCFVehicleImportService::PreviewDefinitionImport(DefinitionSnapshot, ImportSummary, PreviewError))
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ValidationBlocked, PreviewError);
		return false;
	}

	OutResult.ProspectiveRecipePath = RecipeObjectPath;
	OutResult.ImportSummary = ImportSummary;
	OutResult.Proposal.OperationName = TEXT("ImportExistingDefinition");
	OutResult.Proposal.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
	OutResult.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	OutResult.Proposal.ExpectedTargetDefinitionHash = DefinitionSnapshot.DefinitionHash;
	OutResult.Proposal.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutResult.Proposal.bTargetMutation = false;
	OutResult.Proposal.bSavePerformed = false;
	OutResult.Proposal.ProposalHash = CFVehicleAuthoringPrivate::BuildInitialImportProposalHash(
		*Request.TargetVehicleData,
		RecipeObjectPath,
		DefinitionSnapshot,
		ImportSummary);

	OutResult.Operation.CurrentTargetDefinitionHash = DefinitionSnapshot.DefinitionHash;
	OutResult.Operation.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	CFVehicleAuthoringPrivate::SetSucceeded(
		OutResult.Operation,
		TEXT("Current Definition을 변경하지 않고 exact Legacy Pin/semantic candidate Initial Import proposal을 생성했습니다."));
	return true;
}

// Exact OwnershipWrite approval/current Target hash를 재검사한 뒤 새 Recipe record만 만들고 Existing Definition을 import합니다.
bool FCFVehicleAuthoringService::ImportExistingDefinition(
	const FCFVehicleInitialImportRequest& Request,
	FCFVehicleInitialImportResult& OutResult)
{
	OutResult = FCFVehicleInitialImportResult();
	CFVehicleAuthoringPrivate::InitializeResult(
		OutResult.Operation,
		TEXT("ImportExistingDefinition"),
		ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite,
		Request.CallContext.ClientOperationId);
	if (Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::OwnershipWrite)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Initial Import commit에는 exact OwnershipWrite approval이 필요합니다."));
		return false;
	}
	if (!Request.TargetVehicleData)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Initial Import 대상 VehicleData가 없습니다."));
		return false;
	}

	// Commit 직전 current truth에서 다시 만든 fresh reviewed proposal입니다.
	FCFVehicleInitialImportPreviewResult FreshPreview;
	if (!BuildInitialImportProposal(Request, FreshPreview))
	{
		OutResult.Operation = FreshPreview.Operation;
		OutResult.Operation.OperationName = TEXT("ImportExistingDefinition");
		OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
		OutResult.Operation.ClientOperationId = Request.CallContext.ClientOperationId;
		return false;
	}
	if (Request.CallContext.ExpectedResolverContractRevision != FreshPreview.Proposal.ResolverContractRevision
		|| Request.CallContext.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Initial Import approval 이후 Resolver contract revision이 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash.IsEmpty()
		|| Request.CallContext.ExpectedTargetDefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Initial Import approval 이후 Target Definition hash가 변경됐습니다."));
		return false;
	}
	if (Request.CallContext.ApprovalScopeHash.IsEmpty()
		|| Request.CallContext.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("Initial Import approval scope가 fresh Target/destination/import evidence와 일치하지 않습니다."));
		return false;
	}

	// Commit에 실제 사용할 current Definition snapshot입니다.
	FCFVehicleDefinitionSnapshot DefinitionSnapshot;
	// Commit snapshot diagnostic입니다.
	FString CommitError;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Request.TargetVehicleData, DefinitionSnapshot, CommitError)
		|| DefinitionSnapshot.DefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Initial Import mutation 직전 Target Definition이 proposal과 달라졌습니다."));
		return false;
	}

	// Persistent destination 생성 전에 Existing Import Core를 완주할 transient Recipe입니다.
	UCFVehicleRecipeData* TransientRecipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional);
	if (!TransientRecipe)
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, TEXT("Initial Import transient Recipe를 만들 수 없습니다."));
		return false;
	}
	TransientRecipe->TargetVehicleData = Request.TargetVehicleData;

	// Transient import로 재검증한 exact summary입니다.
	FCFVehicleImportResult ImportResult;
	if (!FCFVehicleImportService::ImportDefinitionSnapshot(DefinitionSnapshot, *TransientRecipe, ImportResult, CommitError))
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::ValidationBlocked, CommitError);
		return false;
	}
	if (ImportResult.ImportedDefinitionHash != FreshPreview.ImportSummary.ImportedDefinitionHash
		|| ImportResult.LegacyPinnedFieldCount != FreshPreview.ImportSummary.LegacyPinnedFieldCount
		|| ImportResult.LegacySerializedFieldCount != FreshPreview.ImportSummary.LegacySerializedFieldCount
		|| ImportResult.SemanticCandidateFieldCount != FreshPreview.ImportSummary.SemanticCandidateFieldCount)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::PreviewOutOfDate, TEXT("Initial Import Core readback summary가 reviewed proposal과 달라 persistent Recipe를 만들지 않았습니다."));
		return false;
	}

	// Reviewed destination package name입니다.
	FString RecipePackageName;
	// Reviewed destination object path입니다.
	FSoftObjectPath RecipeObjectPath;
	if (!CFVehicleAuthoringPrivate::BuildInitialImportIdentity(Request, RecipePackageName, RecipeObjectPath, CommitError))
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, CommitError);
		return false;
	}

	// Successful transient Core import 이후에만 persistent package를 만듭니다.
	UPackage* RecipePackage = CreatePackage(*RecipePackageName);
	if (!RecipePackage)
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, TEXT("Initial Import Recipe package를 만들 수 없습니다."));
		return false;
	}
	// Transient imported truth를 persistent Editor-only Recipe로 복제합니다.
	UCFVehicleRecipeData* PersistentRecipe = DuplicateObject<UCFVehicleRecipeData>(TransientRecipe, RecipePackage, Request.RecipeAssetName);
	if (!PersistentRecipe)
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, TEXT("Initial Import persistent Recipe 복제에 실패했습니다."));
		return false;
	}
	PersistentRecipe->ClearFlags(RF_Transient);
	PersistentRecipe->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
	FAssetRegistryModule::AssetCreated(PersistentRecipe);
	PersistentRecipe->MarkPackageDirty();

	// Created Recipe semantic fingerprint readback입니다.
	FCFVehicleRecipeSnapshot CreatedRecipeSnapshot;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*PersistentRecipe, CreatedRecipeSnapshot, CommitError))
	{
		OutResult.Operation.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Operation.Message = FString::Printf(TEXT("Initial Import Recipe는 생성됐지만 readback snapshot에 실패했습니다: %s"), *CommitError);
		OutResult.Recipe = PersistentRecipe;
		OutResult.RecipePath = FSoftObjectPath(PersistentRecipe);
		OutResult.ImportSummary = ImportResult;
		return false;
	}

	OutResult.Recipe = PersistentRecipe;
	OutResult.RecipePath = FSoftObjectPath(PersistentRecipe);
	OutResult.ImportSummary = ImportResult;
	OutResult.Operation.Status = ECFAuthoringOpStatus::Succeeded;
	OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::None;
	OutResult.Operation.Message = TEXT("Existing Definition을 Legacy baseline으로 보존한 새 Recipe를 만들었습니다. Target Definition과 disk file은 변경하지 않았습니다.");
	OutResult.Operation.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	OutResult.Operation.CurrentRecipeFingerprint = CreatedRecipeSnapshot.RecipeFingerprint;
	OutResult.Operation.CurrentTargetDefinitionHash = DefinitionSnapshot.DefinitionHash;
	OutResult.Operation.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutResult.Operation.Mutation.bRecipeChanged = true;
	OutResult.Operation.Mutation.bTargetChanged = false;
	OutResult.Operation.Mutation.bProfileChanged = false;
	OutResult.Operation.Mutation.bCreatedAssets = true;
	OutResult.Operation.Mutation.bPackageDirty = RecipePackage->IsDirty();
	OutResult.Operation.Mutation.bSavePerformed = false;
	OutResult.Operation.Mutation.bAutomaticRetryPerformed = false;
	return true;
}

// Typed semantic command를 transient Recipe Snapshot에만 적용해 prospective Resolve/Proposal을 생성합니다.
bool FCFVehicleAuthoringService::PreviewRecipeChange(const FCFVehicleRecipeChangeRequest& Request, FCFVehicleRecipePreviewResult& OutResult)
{
	OutResult = FCFVehicleRecipePreviewResult();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult.Operation, TEXT("PreviewRecipeChange"), ECFAuthoringRiskClass::R0_ReadOnly, Request.CallContext.ClientOperationId);
	if (!Request.Recipe)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("PreviewRecipeChange Recipe가 null입니다."));
		return false;
	}

	// Current persistent state를 shared Core로 읽기 위한 R0 request입니다.
	FCFVehicleAuthoringReadRequest ReadRequest;
	ReadRequest.Recipe = Request.Recipe;
	ReadRequest.TargetVehicleData = Request.TargetVehicleData;
	ReadRequest.bHasPreviewContext = Request.bHasPreviewContext;
	ReadRequest.PreviewContext = Request.PreviewContext;
	ReadRequest.CallerKind = Request.CallContext.CallerKind;

	// Current baseline shared resolve state입니다.
	CFVehicleAuthoringPrivate::FFreshResolveState BaselineState;
	if (!CFVehicleAuthoringPrivate::BuildFreshResolveState(ReadRequest, BaselineState, OutResult.Operation))
	{
		OutResult.Operation.OperationName = TEXT("PreviewRecipeChange");
		OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
		return false;
	}

	// Persistent Recipe를 수정하지 않는 prospective value copy입니다.
	FCFVehicleRecipeSnapshot ProspectiveRecipe = BaselineState.ResolveRequest.Recipe;
	// Typed semantic validation failure taxonomy입니다.
	ECFAuthoringErrorCode SemanticErrorCode = ECFAuthoringErrorCode::None;
	// Typed semantic validation diagnostic입니다.
	FString Error;
	if (!CFVehicleAuthoringPrivate::ApplySemanticChangeToSnapshot(ProspectiveRecipe, Request.Change, SemanticErrorCode, Error))
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, SemanticErrorCode, Error);
		return false;
	}

	if (!FCFVehicleSnapshotBuilder::BuildRecipeFingerprintFromSnapshot(ProspectiveRecipe, ProspectiveRecipe.RecipeFingerprint, Error))
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, Error);
		return false;
	}

	// Prospective semantic state로 다시 구성한 immutable Resolver request입니다.
	FCFVehicleResolveRequest ProspectiveRequest;
	// Prospective shared Pure Resolver result입니다.
	FCFVehicleResolveResult ProspectiveResolve;
	// Profile/source/build failure taxonomy입니다.
	ECFAuthoringErrorCode ProspectiveErrorCode = ECFAuthoringErrorCode::None;
	if (!CFVehicleAuthoringPrivate::ResolveProspectiveSnapshot(
		ProspectiveRecipe,
		*BaselineState.TargetVehicleData,
		Request.bHasPreviewContext,
		Request.PreviewContext,
		ProspectiveRequest,
		ProspectiveResolve,
		ProspectiveErrorCode,
		Error))
	{
		if (ProspectiveErrorCode == ECFAuthoringErrorCode::InternalError)
		{
			CFVehicleAuthoringPrivate::SetFailed(OutResult.Operation, Error);
		}
		else
		{
			CFVehicleAuthoringPrivate::SetBlocked(OutResult.Operation, ProspectiveErrorCode, Error);
		}
		return false;
	}

	// Prospective exact diff approval binding입니다.
	const FString DiffHash = BuildDiffHash(ProspectiveResolve.FieldDiff);
	// Semantic commit operation identity입니다.
	const FName SemanticOperationName = CFVehicleAuthoringPrivate::GetSemanticOperationName(Request.Change.Operation);
	OutResult.Proposal.OperationName = SemanticOperationName;
	OutResult.Proposal.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
	OutResult.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::AuthoringWrite;
	OutResult.Proposal.ExpectedRecipeFingerprint = BaselineState.ResolveRequest.Recipe.RecipeFingerprint;
	OutResult.Proposal.ExpectedTargetDefinitionHash = BaselineState.ResolveRequest.CurrentDefinition.DefinitionHash;
	OutResult.Proposal.ProspectiveRecipeFingerprint = ProspectiveRecipe.RecipeFingerprint;
	OutResult.Proposal.ProspectiveSourceSignature = ProspectiveResolve.SourceSignature;
	OutResult.Proposal.ProspectiveResolvedDefinitionHash = ProspectiveResolve.ResolvedDefinitionHash;
	OutResult.Proposal.DiffHash = DiffHash;
	OutResult.Proposal.ResolverContractRevision = ProspectiveResolve.ResolverContractRevision;
	OutResult.Proposal.bTargetMutation = false;
	OutResult.Proposal.bSavePerformed = false;
	OutResult.Proposal.ProposalHash = CFVehicleAuthoringPrivate::BuildRecipeProposalHash(
		*Request.Recipe,
		*BaselineState.TargetVehicleData,
		Request.Change,
		BaselineState.ResolveRequest.Recipe,
		BaselineState.ResolveRequest.CurrentDefinition,
		ProspectiveResolve,
		ProspectiveRecipe.RecipeFingerprint,
		DiffHash);

	OutResult.ProspectiveResolveResult = ProspectiveResolve;
	OutResult.ProspectiveImportState = ProspectiveRecipe.ImportState;
	CFVehicleAuthoringPrivate::PopulateResultFromResolve(ProspectiveRequest, ProspectiveResolve, OutResult.Operation);
	OutResult.Operation.OperationName = TEXT("PreviewRecipeChange");
	OutResult.Operation.RiskClass = ECFAuthoringRiskClass::R0_ReadOnly;
	OutResult.Operation.ClientOperationId = Request.CallContext.ClientOperationId;
	OutResult.Operation.Mutation = FCFAuthoringMutationFootprint();
	CFVehicleAuthoringPrivate::SetSucceeded(OutResult.Operation, TEXT("Typed semantic command의 prospective Recipe/Resolve Preview를 persistent mutation 없이 계산했습니다."));
	return true;
}

// Fresh ProposalHash/expected fingerprint/approval을 재검사한 뒤 Recipe만 transaction mutation합니다.
bool FCFVehicleAuthoringService::CommitRecipeChange(const FCFVehicleRecipeChangeRequest& Request, FCFAuthoringOpResult& OutResult)
{
	CFVehicleAuthoringPrivate::InitializeResult(
		OutResult,
		CFVehicleAuthoringPrivate::GetSemanticOperationName(Request.Change.Operation),
		ECFAuthoringRiskClass::R1_AuthoringRecordWrite,
		Request.CallContext.ClientOperationId);

	// Same ClientOperationId dedupe가 비교할 caller request hash입니다.
	const FString RequestHash = CFVehicleAuthoringPrivate::BuildRecipeWriteRequestHash(Request);
	// Cache replay/conflict가 이미 terminal result를 만들었는지 여부입니다.
	bool bDedupeHandled = false;
	if (!CFVehicleAuthoringPrivate::CheckDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult, bDedupeHandled))
	{
		if (bDedupeHandled && !Request.CallContext.ClientOperationId.IsEmpty())
		{
			CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		}
		return false;
	}
	if (bDedupeHandled)
	{
		return OutResult.Status == ECFAuthoringOpStatus::Succeeded || OutResult.Status == ECFAuthoringOpStatus::NoChange;
	}

	if (!Request.Recipe)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::RecipeNotFound, TEXT("CommitRecipeChange Recipe가 null입니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::AuthoringWrite)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("R1 semantic write에는 exact AuthoringWrite approval class가 필요합니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Resolver contract revision이 Preview와 다릅니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ExpectedRecipeFingerprint.IsEmpty() || Request.CallContext.ApprovalScopeHash.IsEmpty())
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("R1 write에는 ExpectedRecipeFingerprint와 exact ApprovalScopeHash가 필요합니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

	// Commit 직전 server-side fresh prospective Preview request입니다.
	FCFVehicleRecipeChangeRequest FreshPreviewRequest = Request;
	FreshPreviewRequest.CallContext.ApprovalClass = ECFAuthoringApprovalClass::None;
	FreshPreviewRequest.CallContext.ApprovalScopeHash.Reset();
	// Fresh server-side prospective result입니다.
	FCFVehicleRecipePreviewResult FreshPreview;
	if (!PreviewRecipeChange(FreshPreviewRequest, FreshPreview))
	{
		OutResult = FreshPreview.Operation;
		OutResult.OperationName = CFVehicleAuthoringPrivate::GetSemanticOperationName(Request.Change.Operation);
		OutResult.RiskClass = ECFAuthoringRiskClass::R1_AuthoringRecordWrite;
		OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

	if (FreshPreview.Proposal.ExpectedRecipeFingerprint != Request.CallContext.ExpectedRecipeFingerprint)
	{
				CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("Recipe가 Preview 이후 변경되었습니다."));
		OutResult.CurrentRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
		OutResult.CurrentSourceSignature = FreshPreview.Proposal.ProspectiveSourceSignature;
		OutResult.CurrentResolvedDefinitionHash = FreshPreview.Proposal.ProspectiveResolvedDefinitionHash;
		OutResult.CurrentDiffHash = FreshPreview.Proposal.DiffHash;
		OutResult.ValidationSummary = FreshPreview.Operation.ValidationSummary;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash.IsEmpty()
		|| FreshPreview.Proposal.ExpectedTargetDefinitionHash != Request.CallContext.ExpectedTargetDefinitionHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Target Definition이 Preview 이후 변경되었거나 expected hash가 없습니다."));
		OutResult.CurrentRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (FreshPreview.Proposal.ResolverContractRevision != Request.CallContext.ExpectedResolverContractRevision)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Fresh prospective Preview의 Resolver revision이 approval state와 다릅니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (FreshPreview.Proposal.ProposalHash != Request.CallContext.ApprovalScopeHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("ApprovalScopeHash가 fresh exact semantic proposal과 일치하지 않습니다."));
		OutResult.CurrentRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

		// Resolver fingerprint 포함 여부와 무관하게 exact typed desired state 자체가 이미 만족되는지 판정합니다.
	const bool bSemanticDesiredStateAlreadySatisfied = CFVehicleAuthoringPrivate::IsSemanticChangeSatisfied(*Request.Recipe, Request.Change);
	if (bSemanticDesiredStateAlreadySatisfied)
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = TEXT("Requested Recipe semantic desired state가 이미 만족되어 transaction을 만들지 않았습니다.");
		OutResult.CurrentRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
		OutResult.CurrentSourceSignature = FreshPreview.Proposal.ProspectiveSourceSignature;
		OutResult.CurrentResolvedDefinitionHash = FreshPreview.Proposal.ProspectiveResolvedDefinitionHash;
		OutResult.CurrentDiffHash = FreshPreview.Proposal.DiffHash;
		OutResult.ValidationSummary = FreshPreview.Operation.ValidationSummary;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return true;
	}

	// Internal failure rollback에 사용할 full Recipe transient backup입니다.
	TStrongObjectPtr<UCFVehicleRecipeData> RecipeBackup(DuplicateObject<UCFVehicleRecipeData>(Request.Recipe, GetTransientPackage()));
	if (!RecipeBackup.IsValid())
	{
		CFVehicleAuthoringPrivate::SetFailed(OutResult, TEXT("Recipe transaction rollback backup 생성에 실패했습니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	// Transaction 전 Recipe package dirty 상태입니다.
	const bool bRecipePackageWasDirty = Request.Recipe->GetOutermost() && Request.Recipe->GetOutermost()->IsDirty();
	// R1 persistent Recipe semantic write transaction입니다.
	FScopedTransaction RecipeTransaction(NSLOCTEXT("CarFightDataAuthoring", "CommitVehicleRecipeSemanticChange", "차량 Recipe 의미 변경"));
	Request.Recipe->Modify();
	if (!CFVehicleAuthoringPrivate::ApplySemanticChangeToRecipe(*Request.Recipe, Request.Change))
	{
		// Unexpected persistent apply failure rollback diagnostic입니다.
		FString RollbackError;
		CFVehicleAuthoringPrivate::RestoreRecipeFromBackup(*Request.Recipe, *RecipeBackup, RollbackError);
		RecipeTransaction.Cancel();
		if (UPackage* RecipePackage = Request.Recipe->GetOutermost())
		{
			RecipePackage->SetDirtyFlag(bRecipePackageWasDirty);
		}
		OutResult.Status = ECFAuthoringOpStatus::FailedRolledBack;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = TEXT("Persistent Recipe semantic change 적용에 실패해 rollback했습니다.");
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	++Request.Recipe->AuthoringRevision;
	Request.Recipe->MarkPackageDirty();
	Request.Recipe->PostEditChange();

	// Commit readback semantic fingerprint입니다.
	FCFVehicleRecipeSnapshot CommittedSnapshot;
	// Commit readback build diagnostic입니다.
	FString SnapshotError;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.Recipe, CommittedSnapshot, SnapshotError)
		|| CommittedSnapshot.RecipeFingerprint != FreshPreview.Proposal.ProspectiveRecipeFingerprint)
	{
		// Commit mismatch rollback diagnostic입니다.
		FString RollbackError;
		const bool bRollbackRestored = CFVehicleAuthoringPrivate::RestoreRecipeFromBackup(*Request.Recipe, *RecipeBackup, RollbackError);
		RecipeTransaction.Cancel();
		if (UPackage* RecipePackage = Request.Recipe->GetOutermost())
		{
			RecipePackage->SetDirtyFlag(bRecipePackageWasDirty);
		}
		OutResult.Status = bRollbackRestored ? ECFAuthoringOpStatus::FailedRolledBack : ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Message = bRollbackRestored
			? TEXT("Recipe commit readback fingerprint가 prospective proposal과 달라 rollback했습니다.")
			: FString::Printf(TEXT("Recipe commit mismatch 후 rollback 검증도 실패했습니다: %s"), *RollbackError);
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

	OutResult.Status = ECFAuthoringOpStatus::Succeeded;
	OutResult.ErrorCode = ECFAuthoringErrorCode::None;
	OutResult.Message = TEXT("Reviewed typed semantic change를 Recipe transaction으로 commit했습니다. Target/Save mutation은 없습니다.");
	OutResult.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	OutResult.CurrentRecipeFingerprint = CommittedSnapshot.RecipeFingerprint;
	OutResult.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
	OutResult.CurrentSourceSignature = FreshPreview.Proposal.ProspectiveSourceSignature;
	OutResult.CurrentResolvedDefinitionHash = FreshPreview.Proposal.ProspectiveResolvedDefinitionHash;
	OutResult.CurrentDiffHash = FreshPreview.Proposal.DiffHash;
	OutResult.ResolverContractRevision = FreshPreview.Proposal.ResolverContractRevision;
	OutResult.ValidationSummary = FreshPreview.Operation.ValidationSummary;
	OutResult.Mutation.bRecipeChanged = true;
	OutResult.Mutation.bTargetChanged = false;
	OutResult.Mutation.bProfileChanged = false;
	OutResult.Mutation.bCreatedAssets = false;
	OutResult.Mutation.bPackageDirty = Request.Recipe->GetOutermost() && Request.Recipe->GetOutermost()->IsDirty();
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;
	CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
	return true;
}

// Reviewed shared ApplyRequest에서 exact R3 DefinitionApply approval proposal을 생성합니다.
bool FCFVehicleAuthoringService::BuildApplyApprovalProposal(
	const FCFVehicleApplyRequest& ApplyRequest,
	FCFAuthoringProposal& OutProposal,
	FCFAuthoringOpResult& OutResult)
{
	OutProposal = FCFAuthoringProposal();
	CFVehicleAuthoringPrivate::InitializeResult(OutResult, TEXT("BuildApplyApprovalProposal"), ECFAuthoringRiskClass::R0_ReadOnly);
	if (!ApplyRequest.Recipe || !ApplyRequest.TargetVehicleData)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApplyPreconditionFailed, TEXT("Apply approval proposal에는 Recipe와 Target VehicleData가 필요합니다."));
		return false;
	}
	if (ApplyRequest.ApprovedResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ValidationBlocked, TEXT("R3 approval proposal은 successful reviewed Resolve result에서만 만들 수 있습니다."));
		return false;
	}
	if (ApplyRequest.ExpectedRecipeFingerprint.IsEmpty()
		|| ApplyRequest.ExpectedSourceSignature.IsEmpty()
		|| ApplyRequest.ExpectedTargetDefinitionHash.IsEmpty()
		|| ApplyRequest.ExpectedResolvedDefinitionHash.IsEmpty()
		|| ApplyRequest.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApplyPreconditionFailed, TEXT("R3 approval proposal의 frozen expected evidence가 불완전하거나 revision이 stale합니다."));
		return false;
	}

	// Reviewed exact FieldDiff hash입니다.
	const FString DiffHash = BuildDiffHash(ApplyRequest.ApprovedResolveResult.FieldDiff);
	OutProposal.OperationName = TEXT("ApplyResolvedVehicle");
	OutProposal.RiskClass = ECFAuthoringRiskClass::R3_DefinitionApply;
	OutProposal.RequiredApprovalClass = ECFAuthoringApprovalClass::DefinitionApply;
	OutProposal.ExpectedRecipeFingerprint = ApplyRequest.ExpectedRecipeFingerprint;
	OutProposal.ExpectedTargetDefinitionHash = ApplyRequest.ExpectedTargetDefinitionHash;
	OutProposal.ProspectiveRecipeFingerprint = ApplyRequest.ExpectedRecipeFingerprint;
	OutProposal.ProspectiveSourceSignature = ApplyRequest.ExpectedSourceSignature;
	OutProposal.ProspectiveResolvedDefinitionHash = ApplyRequest.ExpectedResolvedDefinitionHash;
	OutProposal.DiffHash = DiffHash;
	OutProposal.ResolverContractRevision = ApplyRequest.ExpectedResolverContractRevision;
	OutProposal.bTargetMutation = !ApplyRequest.ApprovedResolveResult.FieldDiff.IsEmpty();
	OutProposal.bSavePerformed = false;
	OutProposal.ProposalHash = CFVehicleAuthoringPrivate::BuildApplyProposalHash(ApplyRequest, DiffHash);

	OutResult.CurrentRecipeFingerprint = ApplyRequest.ExpectedRecipeFingerprint;
	OutResult.CurrentTargetDefinitionHash = ApplyRequest.ExpectedTargetDefinitionHash;
	OutResult.CurrentSourceSignature = ApplyRequest.ExpectedSourceSignature;
	OutResult.CurrentResolvedDefinitionHash = ApplyRequest.ExpectedResolvedDefinitionHash;
	OutResult.CurrentDiffHash = DiffHash;
	OutResult.ResolverContractRevision = ApplyRequest.ExpectedResolverContractRevision;
	OutResult.ValidationSummary = CFVehicleAuthoringPrivate::BuildValidationSummary(ApplyRequest.ApprovedResolveResult);
	CFVehicleAuthoringPrivate::SetSucceeded(OutResult, TEXT("Reviewed Resolve evidence로 exact R3 DefinitionApply approval scope를 생성했습니다."));
	return true;
}

// R3 approval/precondition을 확인한 뒤 Target writer를 새로 만들지 않고 FCFVehicleApplyService를 호출합니다.
bool FCFVehicleAuthoringService::ApplyResolvedVehicle(const FCFVehicleApplyOpRequest& Request, FCFAuthoringOpResult& OutResult)
{
	CFVehicleAuthoringPrivate::InitializeResult(
		OutResult,
		TEXT("ApplyResolvedVehicle"),
		ECFAuthoringRiskClass::R3_DefinitionApply,
		Request.CallContext.ClientOperationId);

	// Same ClientOperationId dedupe가 비교할 caller R3 request hash입니다.
	const FString RequestHash = CFVehicleAuthoringPrivate::BuildApplyWriteRequestHash(Request);
	// Cache replay/conflict가 이미 terminal result를 만들었는지 여부입니다.
	bool bDedupeHandled = false;
	if (!CFVehicleAuthoringPrivate::CheckDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult, bDedupeHandled))
	{
		if (bDedupeHandled && !Request.CallContext.ClientOperationId.IsEmpty())
		{
			CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		}
		return false;
	}
	if (bDedupeHandled)
	{
		return OutResult.Status == ECFAuthoringOpStatus::Succeeded || OutResult.Status == ECFAuthoringOpStatus::NoChange;
	}

	if (Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::DefinitionApply)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalRequired, TEXT("R3 Apply에는 exact DefinitionApply approval class가 필요합니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (!Request.ApplyRequest.Recipe || !Request.ApplyRequest.TargetVehicleData)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApplyPreconditionFailed, TEXT("ApplyResolvedVehicle Recipe/Target이 null입니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ExpectedResolverContractRevision != Request.ApplyRequest.ExpectedResolverContractRevision
		|| Request.CallContext.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("R3 CallContext와 shared ApplyRequest resolver revision이 일치하지 않습니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ExpectedRecipeFingerprint != Request.ApplyRequest.ExpectedRecipeFingerprint)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("R3 CallContext와 ApplyRequest Recipe fingerprint가 일치하지 않습니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ExpectedTargetDefinitionHash != Request.ApplyRequest.ExpectedTargetDefinitionHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("R3 CallContext와 ApplyRequest Target hash가 일치하지 않습니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

	// Approved Resolve exact diff에서 fresh 계산한 hash입니다.
	const FString ReviewedDiffHash = BuildDiffHash(Request.ApplyRequest.ApprovedResolveResult.FieldDiff);
	if (Request.ExpectedDiffHash.IsEmpty() || ReviewedDiffHash != Request.ExpectedDiffHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::PreviewOutOfDate, TEXT("ExpectedDiffHash가 reviewed FieldDiff authority와 일치하지 않습니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

	// Reviewed ApplyRequest에서 다시 만든 exact R3 approval proposal입니다.
	FCFAuthoringProposal FreshApprovalProposal;
	// Approval proposal helper result입니다.
	FCFAuthoringOpResult ProposalResult;
	if (!BuildApplyApprovalProposal(Request.ApplyRequest, FreshApprovalProposal, ProposalResult))
	{
		OutResult = ProposalResult;
		OutResult.OperationName = TEXT("ApplyResolvedVehicle");
		OutResult.RiskClass = ECFAuthoringRiskClass::R3_DefinitionApply;
		OutResult.ClientOperationId = Request.CallContext.ClientOperationId;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}
	if (Request.CallContext.ApprovalScopeHash.IsEmpty()
		|| Request.CallContext.ApprovalScopeHash != FreshApprovalProposal.ProposalHash)
	{
		CFVehicleAuthoringPrivate::SetBlocked(OutResult, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("DefinitionApply ApprovalScopeHash가 reviewed exact Diff/evidence와 일치하지 않습니다."));
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return false;
	}

	// Already-applied no-change 판정을 위한 current shared facade read입니다.
	FCFVehicleAuthoringReadRequest CurrentReadRequest;
	CurrentReadRequest.Recipe = Request.ApplyRequest.Recipe;
	CurrentReadRequest.TargetVehicleData = Request.ApplyRequest.TargetVehicleData;
	CurrentReadRequest.CallerKind = Request.CallContext.CallerKind;
	// Current shared resolver result입니다.
	FCFVehicleResolveReadResult CurrentReadResult;
	if (ResolveVehiclePreview(CurrentReadRequest, CurrentReadResult)
		&& CurrentReadResult.ResolveRequest.Recipe.RecipeFingerprint == Request.ApplyRequest.ExpectedRecipeFingerprint
		&& CurrentReadResult.ResolveResult.SourceSignature == Request.ApplyRequest.ExpectedSourceSignature
		&& CurrentReadResult.ResolveResult.ResolvedDefinitionHash == Request.ApplyRequest.ExpectedResolvedDefinitionHash
		&& CurrentReadResult.ResolveResult.FieldDiff.IsEmpty()
		&& Request.ApplyRequest.Recipe->AppliedState.AppliedRecipeFingerprint == Request.ApplyRequest.ExpectedRecipeFingerprint
		&& Request.ApplyRequest.Recipe->AppliedState.AppliedSourceSignature == Request.ApplyRequest.ExpectedSourceSignature
		&& Request.ApplyRequest.Recipe->AppliedState.AppliedDefinitionHash == Request.ApplyRequest.ExpectedResolvedDefinitionHash
		&& Request.ApplyRequest.Recipe->AppliedState.ResolverContractRevision == Request.ApplyRequest.ExpectedResolverContractRevision)
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = TEXT("Target과 AppliedState가 이미 reviewed resolved/source state와 일치하여 두 번째 Apply transaction을 만들지 않았습니다.");
		OutResult.CurrentRecipeFingerprint = CurrentReadResult.ResolveRequest.Recipe.RecipeFingerprint;
		OutResult.CurrentTargetDefinitionHash = CurrentReadResult.ResolveRequest.CurrentDefinition.DefinitionHash;
		OutResult.CurrentSourceSignature = CurrentReadResult.ResolveResult.SourceSignature;
		OutResult.CurrentResolvedDefinitionHash = CurrentReadResult.ResolveResult.ResolvedDefinitionHash;
		OutResult.CurrentDiffHash = BuildDiffHash(CurrentReadResult.ResolveResult.FieldDiff);
		OutResult.ValidationSummary = CurrentReadResult.Operation.ValidationSummary;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return true;
	}

	// 실제 Target write를 독점하는 기존 P0-08H shared ApplyService result입니다.
	FCFVehicleApplyResult ApplyResult;
	const bool bApplySucceeded = FCFVehicleApplyService::Apply(Request.ApplyRequest, ApplyResult);
	OutResult.AppliedFieldCount = ApplyResult.AppliedDiffOperationCount;
	OutResult.CurrentRecipeFingerprint = Request.ApplyRequest.ExpectedRecipeFingerprint;
	OutResult.CurrentSourceSignature = Request.ApplyRequest.ExpectedSourceSignature;
	OutResult.CurrentResolvedDefinitionHash = ApplyResult.AppliedDefinitionHash.IsEmpty()
		? Request.ApplyRequest.ExpectedResolvedDefinitionHash
		: ApplyResult.AppliedDefinitionHash;
	OutResult.CurrentDiffHash = Request.ExpectedDiffHash;
	OutResult.ResolverContractRevision = Request.ApplyRequest.ExpectedResolverContractRevision;
	OutResult.Mutation.bRecipeChanged = ApplyResult.bRecipeAppliedStateUpdated;
	OutResult.Mutation.bTargetChanged = ApplyResult.bTargetMutationCommitted;
	OutResult.Mutation.bProfileChanged = false;
	OutResult.Mutation.bCreatedAssets = false;
	OutResult.Mutation.bPackageDirty = (Request.ApplyRequest.Recipe->GetOutermost() && Request.ApplyRequest.Recipe->GetOutermost()->IsDirty())
		|| (Request.ApplyRequest.TargetVehicleData->GetOutermost() && Request.ApplyRequest.TargetVehicleData->GetOutermost()->IsDirty());
	OutResult.Mutation.bSavePerformed = false;
	OutResult.Mutation.bAutomaticRetryPerformed = false;

	// Apply 종료 시 full Target Definition hash readback입니다.
	FCFVehicleDefinitionSnapshot CurrentTargetSnapshot;
	// Target full Snapshot readback diagnostic입니다.
	FString SnapshotError;
	if (FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Request.ApplyRequest.TargetVehicleData, CurrentTargetSnapshot, SnapshotError))
	{
		OutResult.CurrentTargetDefinitionHash = CurrentTargetSnapshot.DefinitionHash;
	}
	// Apply 종료 시 Recipe semantic fingerprint readback입니다.
	FCFVehicleRecipeSnapshot CurrentRecipeSnapshot;
	if (FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(*Request.ApplyRequest.Recipe, CurrentRecipeSnapshot, SnapshotError))
	{
		OutResult.CurrentRecipeFingerprint = CurrentRecipeSnapshot.RecipeFingerprint;
	}

	if (bApplySucceeded && ApplyResult.Status == ECFVehicleApplyStatus::Success)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = ApplyResult.Message;
		OutResult.AuthoringActionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
		OutResult.ValidationSummary = ProposalResult.ValidationSummary;
		CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
		return true;
	}

	switch (ApplyResult.FailureCode)
	{
	case ECFVehicleApplyFailureCode::PreviewOutOfDate:
	case ECFVehicleApplyFailureCode::ReviewedDiffMismatch:
	case ECFVehicleApplyFailureCode::ResolveNotSuccessful:
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApplyPreconditionFailed;
		break;
	case ECFVehicleApplyFailureCode::PreflightValidationFailed:
	case ECFVehicleApplyFailureCode::TargetValidationFailed:
		OutResult.Status = ApplyResult.bRollbackVerified ? ECFAuthoringOpStatus::FailedRolledBack : ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApplyValidationFailed;
		break;
	case ECFVehicleApplyFailureCode::RollbackFailed:
		OutResult.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApplyRollbackFailed;
		break;
	case ECFVehicleApplyFailureCode::TargetApplyFailed:
	case ECFVehicleApplyFailureCode::TargetReadbackMismatch:
		OutResult.Status = ApplyResult.bRollbackVerified ? ECFAuthoringOpStatus::FailedRolledBack : ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ApplyResult.bRollbackVerified ? ECFAuthoringErrorCode::ApplyValidationFailed : ECFAuthoringErrorCode::ApplyRollbackFailed;
		break;
	case ECFVehicleApplyFailureCode::PreflightApplyFailed:
	case ECFVehicleApplyFailureCode::PreflightHashMismatch:
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ECFAuthoringErrorCode::ApplyPreconditionFailed;
		break;
	default:
		OutResult.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.ErrorCode = ECFAuthoringErrorCode::InternalError;
		break;
	}
	OutResult.Message = ApplyResult.Message;
	OutResult.ValidationSummary = ProposalResult.ValidationSummary;
	CFVehicleAuthoringPrivate::StoreDedupe(Request.CallContext.ClientOperationId, RequestHash, OutResult);
	return false;
}

// Shared Resolver FieldDiff exact rows의 deterministic hash를 반환합니다.
FString FCFVehicleAuthoringService::BuildDiffHash(const TArray<FCFVehicleFieldDiff>& FieldDiff)
{
	// Resolver가 만든 deterministic row 순서를 그대로 hash할 canonical payload입니다.
	FString Payload;
	CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("Kind"), TEXT("VehicleFieldDiff"));
	CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("FormatRevision"), TEXT("1"));
	CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("Count"), FString::FromInt(FieldDiff.Num()));
	for (const FCFVehicleFieldDiff& Diff : FieldDiff)
	{
		CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("Operation"), FString::FromInt(static_cast<int32>(Diff.Operation)));
		CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("Path"), Diff.FieldPath.ToCanonicalString(true));
		CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("HasBefore"), Diff.bHasBeforeValue ? TEXT("1") : TEXT("0"));
		if (Diff.bHasBeforeValue)
		{
			CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("BeforeType"), Diff.BeforeValue.PropertyTypeSignature);
			CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("BeforeValue"), Diff.BeforeValue.CanonicalValueText);
		}
		CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("HasAfter"), Diff.bHasAfterValue ? TEXT("1") : TEXT("0"));
		if (Diff.bHasAfterValue)
		{
			CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("AfterType"), Diff.AfterValue.PropertyTypeSignature);
			CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("AfterValue"), Diff.AfterValue.CanonicalValueText);
		}
		CFVehicleAuthoringPrivate::AppendToken(Payload, TEXT("SourceTraceIndex"), FString::FromInt(Diff.SourceTraceIndex));
	}
	return CFVehicleAuthoringPrivate::HashUtf8Payload(Payload);
}
