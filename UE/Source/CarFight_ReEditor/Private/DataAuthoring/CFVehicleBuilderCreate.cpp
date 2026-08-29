// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBuilderCreate.cpp
// Version: v1.3.0
// Date: 2026-08-27
// Description: CF-FQ-040 VB-P0-05 baseline-safe Builder companion + deterministic prospective Profile identity preview/create 구현입니다.
// Scope: Existing Definition+Recipe에 Reference Evidence + Builder-private VehicleBase/Drivetrain/Handling/Performance Profile을 보완합니다.
// Changelog:
// - v1.3.0: 새 Evidence를 빈 companion으로 만들지 않고 initial Research payload + fixed EvidenceId를 Preview/approval/Commit에 binding하며 prospective Evidence fingerprint를 proposal hash에 포함.
// - v1.2.0: Missing Profile prospective resolve가 transient UObject path를 SourceSignature/approval hash에 섞지 않도록 preview에서 확정한 persistent Profile path + typed seed snapshot으로 Pure Resolver를 실행.
// - v1.1.0: Missing private Profile은 complete initial typed payload로만 seed하고, CompleteExisting 모드는 current/prospective Resolver Definition hash 동일성을 강제해 companion 보완만으로 기존 차량 주행 특성이 바뀌는 것을 차단.
// - v1.0.1: 새 private Profile Asset 생성은 existing Profile payload mutation이 아니므로 mutation footprint의 bProfileChanged를 false로 고정.
// - v1.0.0: Existing companion은 보존하고 Missing companion만 생성하며 shared/foreign-owner Profile은 fail-closed Conflict로 차단하는 R2 preview/create 구현.
// Migration:
// - 기존 CreateVehicleRecords의 Definition+Recipe 생성 계약은 유지합니다.
// - Existing Vehicle Completion은 이 operation을 재사용해 already-complete companion을 다시 만들지 않습니다.
// - Disk Save, raw VehicleData write, Shared Profile mutation, DriveState Profile 생성은 수행하지 않습니다.
// - Preview/Commit fresh replay에서 prospective SourceSignature/ProposalHash는 transient object allocation order와 무관하게 deterministic해야 합니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleBuilderCreatePrivate
{
	// Builder companion create operation의 stable identity입니다.
	const FName CreateOperationName(TEXT("CreateBuilderCompanions"));

	// Common result를 operation 시작 상태로 초기화합니다.
	void InitializeResult(FCFAuthoringOpResult& OutResult, const FName OperationName, const ECFAuthoringRiskClass RiskClass, const FString& ClientOperationId)
	{
		OutResult = FCFAuthoringOpResult();
		OutResult.OperationName = OperationName;
		OutResult.RiskClass = RiskClass;
		OutResult.ClientOperationId = ClientOperationId;
		OutResult.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		OutResult.Mutation.bSavePerformed = false;
		OutResult.Mutation.bAutomaticRetryPerformed = false;
	}

	// Result를 fail-closed Blocked 상태로 설정합니다.
	bool Block(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
		return false;
	}

	// Result를 concurrent/current-state Conflict 상태로 설정합니다.
	bool Conflict(FCFAuthoringOpResult& OutResult, const ECFAuthoringErrorCode ErrorCode, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Conflict;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
		return false;
	}

	// Result를 성공 상태로 설정합니다.
	void Succeed(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::Succeeded;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Result를 mutation 없는 NoChange 상태로 설정합니다.
	void NoChange(FCFAuthoringOpResult& OutResult, const FString& Message)
	{
		OutResult.Status = ECFAuthoringOpStatus::NoChange;
		OutResult.ErrorCode = ECFAuthoringErrorCode::None;
		OutResult.Message = Message;
		OutResult.bRetryAllowed = false;
	}

	// Delimiter 충돌 없는 canonical hash token을 누적합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("|");
	}

	// UTF-8 payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR payload의 canonical UTF-8 bytes입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Deterministic digest state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// Final MD5 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);
		// Lowercase hexadecimal result입니다.
		FString Result;
		Result.Reserve(32);
		// Fixed hexadecimal lookup table입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			Result.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			Result.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return Result;
	}

	// 새 asset package/object pair가 현재 memory/disk에 존재하지 않는지 fail-closed 검사합니다.
	bool ValidateNewAssetIdentity(const FCFBuilderAssetIdentity& Identity, FSoftObjectPath& OutObjectPath, FString& OutError)
	{
		if (!FPackageName::IsValidLongPackageName(Identity.PackageName) || Identity.AssetName.IsNone())
		{
			OutError = TEXT("Missing Builder companion에는 유효한 /Game long package name과 asset name이 필요합니다.");
			return false;
		}

		// Expected object path text입니다.
		const FString ObjectPathText = Identity.PackageName + TEXT(".") + Identity.AssetName.ToString();
		OutObjectPath = FSoftObjectPath(ObjectPathText);
		if (!OutObjectPath.IsValid())
		{
			OutError = FString::Printf(TEXT("유효하지 않은 Builder companion object path입니다: %s"), *ObjectPathText);
			return false;
		}
		if (FindPackage(nullptr, *Identity.PackageName) || FPackageName::DoesPackageExist(Identity.PackageName) || OutObjectPath.ResolveObject())
		{
			OutError = FString::Printf(TEXT("기존 package/object를 Builder companion 생성으로 덮어쓸 수 없습니다: %s"), *ObjectPathText);
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Existing Evidence를 exact class/Recipe/Definition binding으로 검증합니다.
	UCFVehicleRefEvidence* ValidateExistingEvidence(const FSoftObjectPath& EvidencePath, UCFVehicleRecipeData& Recipe, UCFVehicleData& Target, FString& OutError)
	{
		// 이미 load된 Evidence 또는 path에서 load한 UObject입니다.
		UObject* LoadedObject = EvidencePath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = EvidencePath.TryLoad();
		}
		// Exact expected Evidence class입니다.
		UCFVehicleRefEvidence* Evidence = Cast<UCFVehicleRefEvidence>(LoadedObject);
		if (!Evidence)
		{
			OutError = FString::Printf(TEXT("Existing Evidence가 UCFVehicleRefEvidence가 아닙니다: %s"), *EvidencePath.ToString());
			return nullptr;
		}
		if (Evidence->TargetRecipeId != Recipe.RecipeId
			|| Evidence->TargetRecipePath != FSoftObjectPath(&Recipe)
			|| Evidence->TargetDefinitionPath != FSoftObjectPath(&Target))
		{
			OutError = TEXT("Existing Evidence의 Recipe/Definition ownership binding이 선택한 차량과 일치하지 않습니다.");
			return nullptr;
		}

		// Current semantic payload에서 fresh 계산한 Evidence fingerprint입니다.
		FString FreshFingerprint;
		if (!Evidence->BuildEvidenceFingerprint(FreshFingerprint, OutError) || FreshFingerprint != Evidence->EvidenceFingerprint)
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("Existing Evidence generated fingerprint가 current semantic payload와 일치하지 않습니다.");
			}
			return nullptr;
		}

		OutError.Reset();
		return Evidence;
	}

	// Existing 또는 새 initial Evidence의 exact current/prospective fingerprint를 preview에 계산합니다.
	bool BuildProspectiveEvidenceFingerprint(
		const FCFBuilderCompanionRequest& Request,
		UCFVehicleData& Target,
		FCFBuilderCompanionPreview& OutPreview,
		FString& OutError)
	{
		if (Request.ExistingEvidencePath.IsValid())
		{
			if (Request.bHasInitialEvidencePayload)
			{
				OutError = TEXT("Existing Evidence research payload는 Companion 생성 경로에서 덮어쓸 수 없습니다. 새 Research 변경은 별도 reviewed flow가 필요합니다.");
				return false;
			}

			// 선택 차량에 exact binding된 existing Evidence입니다.
			UCFVehicleRefEvidence* ExistingEvidence = ValidateExistingEvidence(Request.ExistingEvidencePath, *Request.Recipe, Target, OutError);
			if (!ExistingEvidence)
			{
				return false;
			}
			OutPreview.ProspectiveEvidenceFingerprint = ExistingEvidence->EvidenceFingerprint;
			OutError.Reset();
			return true;
		}

		if (!Request.bHasInitialEvidencePayload || !Request.NewEvidenceId.IsValid())
		{
			OutError = TEXT("새 Reference Evidence는 빈 record로 생성할 수 없습니다. fixed NewEvidenceId와 complete initial Research payload가 필요합니다.");
			return false;
		}

		// Preview fingerprint 계산에만 사용하는 transient Evidence carrier입니다.
		UCFVehicleRefEvidence* ProspectiveEvidence = NewObject<UCFVehicleRefEvidence>(GetTransientPackage());
		if (!ProspectiveEvidence)
		{
			OutError = TEXT("Prospective Reference Evidence carrier를 만들 수 없습니다.");
			return false;
		}
		ProspectiveEvidence->EvidenceId = Request.NewEvidenceId;
		ProspectiveEvidence->TargetRecipeId = Request.Recipe->RecipeId;
		ProspectiveEvidence->TargetRecipePath = FSoftObjectPath(Request.Recipe);
		ProspectiveEvidence->TargetDefinitionPath = FSoftObjectPath(&Target);
		if (!ProspectiveEvidence->ApplyInitialResearchPayload(Request.InitialEvidencePayload, OutError))
		{
			return false;
		}

		OutPreview.ProspectiveEvidenceFingerprint = ProspectiveEvidence->EvidenceFingerprint;
		OutError.Reset();
		return true;
	}

	// Existing VehicleBase Profile이 exact Builder owner인지 검증합니다.
	UCFVehicleBaseProfile* ValidateVehicleBaseProfile(const TSoftObjectPtr<UCFVehicleBaseProfile>& ProfileRef, const FGuid& RecipeId, FString& OutError)
	{
		// Recipe binding에서 load한 exact VehicleBase Profile입니다.
		UCFVehicleBaseProfile* Profile = ProfileRef.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != RecipeId)
		{
			OutError = TEXT("VehicleBase binding은 Builder-private exact OwnerRecipeId가 아니므로 자동 보완할 수 없습니다. Shared/foreign Profile은 먼저 명시적으로 분리해야 합니다.");
			return nullptr;
		}
		OutError.Reset();
		return Profile;
	}

	// Existing Drivetrain Profile이 exact Builder owner인지 검증합니다.
	UCFDrivetrainProfile* ValidateDrivetrainProfile(const TSoftObjectPtr<UCFDrivetrainProfile>& ProfileRef, const FGuid& RecipeId, FString& OutError)
	{
		// Recipe binding에서 load한 exact Drivetrain Profile입니다.
		UCFDrivetrainProfile* Profile = ProfileRef.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != RecipeId)
		{
			OutError = TEXT("Drivetrain binding은 Builder-private exact OwnerRecipeId가 아니므로 자동 보완할 수 없습니다. Shared/foreign Profile은 먼저 명시적으로 분리해야 합니다.");
			return nullptr;
		}
		OutError.Reset();
		return Profile;
	}

	// Existing Handling Profile이 exact Builder owner인지 검증합니다.
	UCFHandlingProfile* ValidateHandlingProfile(const TSoftObjectPtr<UCFHandlingProfile>& ProfileRef, const FGuid& RecipeId, FString& OutError)
	{
		// Recipe binding에서 load한 exact Handling Profile입니다.
		UCFHandlingProfile* Profile = ProfileRef.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != RecipeId)
		{
			OutError = TEXT("Handling binding은 Builder-private exact OwnerRecipeId가 아니므로 자동 보완할 수 없습니다. Shared/foreign Profile은 먼저 명시적으로 분리해야 합니다.");
			return nullptr;
		}
		OutError.Reset();
		return Profile;
	}

	// Existing Performance Profile이 exact Builder owner인지 검증합니다.
	UCFPerformanceProfile* ValidatePerformanceProfile(const TSoftObjectPtr<UCFPerformanceProfile>& ProfileRef, const FGuid& RecipeId, FString& OutError)
	{
		// Recipe binding에서 load한 exact Performance Profile입니다.
		UCFPerformanceProfile* Profile = ProfileRef.LoadSynchronous();
		if (!Profile || Profile->Meta.OwnerRecipeId != RecipeId)
		{
			OutError = TEXT("Performance binding은 Builder-private exact OwnerRecipeId가 아니므로 자동 보완할 수 없습니다. Shared/foreign Profile은 먼저 명시적으로 분리해야 합니다.");
			return nullptr;
		}
		OutError.Reset();
		return Profile;
	}

	// Builder companion preview를 current Recipe/Definition snapshots와 exact path set에 binding하는 hash를 생성합니다.
	FString BuildProposalHash(const FCFBuilderCompanionRequest& Request, const FCFBuilderCompanionPreview& Preview, const FString& RecipeFingerprint, const FString& TargetDefinitionHash)
	{
		// Deterministic proposal payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("op"), CreateOperationName.ToString());
		AppendToken(Payload, TEXT("recipeId"), Request.Recipe ? Request.Recipe->RecipeId.ToString(EGuidFormats::DigitsWithHyphensLower) : FString());
		AppendToken(Payload, TEXT("recipePath"), Request.Recipe ? FSoftObjectPath(Request.Recipe).ToString() : FString());
		AppendToken(Payload, TEXT("recipeFingerprint"), RecipeFingerprint);
		AppendToken(Payload, TEXT("targetHash"), TargetDefinitionHash);
		AppendToken(Payload, TEXT("mode"), FString::FromInt(static_cast<int32>(Request.Mode)));
		AppendToken(Payload, TEXT("currentResolvedHash"), Preview.CurrentResolvedDefinitionHash);
		AppendToken(Payload, TEXT("prospectiveResolvedHash"), Preview.ProspectiveResolvedDefinitionHash);
		AppendToken(Payload, TEXT("prospectiveSourceSignature"), Preview.ProspectiveSourceSignature);
		AppendToken(Payload, TEXT("evidence"), Preview.EvidencePath.ToString());
		AppendToken(Payload, TEXT("evidenceFingerprint"), Preview.ProspectiveEvidenceFingerprint);
		AppendToken(Payload, TEXT("vehicleBase"), Preview.VehicleBasePath.ToString());
		AppendToken(Payload, TEXT("drivetrain"), Preview.DrivetrainPath.ToString());
		AppendToken(Payload, TEXT("handling"), Preview.HandlingPath.ToString());
		AppendToken(Payload, TEXT("performance"), Preview.PerformancePath.ToString());
		AppendToken(Payload, TEXT("resolverRevision"), FString::FromInt(FCFVehicleResolver::CurrentResolverContractRevision));
		return HashUtf8Payload(Payload);
	}

	// Preview가 사용하는 current Recipe/Target semantic snapshots를 생성합니다.
	bool BuildCurrentHashes(UCFVehicleRecipeData& Recipe, UCFVehicleData& Target, FString& OutRecipeFingerprint, FString& OutTargetDefinitionHash, FString& OutError)
	{
		// Current Recipe semantic snapshot입니다.
		FCFVehicleRecipeSnapshot RecipeSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, RecipeSnapshot, OutError))
		{
			return false;
		}
		// Current Target full Definition snapshot입니다.
		FCFVehicleDefinitionSnapshot DefinitionSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, DefinitionSnapshot, OutError))
		{
			return false;
		}
		OutRecipeFingerprint = RecipeSnapshot.RecipeFingerprint;
		OutTargetDefinitionHash = DefinitionSnapshot.DefinitionHash;
		OutError.Reset();
		return true;
	}

	// Current 또는 transient Recipe를 shared Resolver facade로 계산하고 exact ResolveResult를 반환합니다.
	bool ResolveRecipe(
		UCFVehicleRecipeData& Recipe,
		UCFVehicleData& Target,
		const ECFAuthoringCallerKind CallerKind,
		FCFVehicleResolveResult& OutResolveResult,
		FString& OutError)
	{
		// Shared Resolver facade request입니다.
		FCFVehicleAuthoringReadRequest ReadRequest;
		ReadRequest.Recipe = &Recipe;
		ReadRequest.TargetVehicleData = &Target;
		ReadRequest.CallerKind = CallerKind;

		// Shared Resolver facade의 typed read result입니다.
		FCFVehicleResolveReadResult ReadResult;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ReadRequest, ReadResult))
		{
			OutError = ReadResult.Operation.Message.IsEmpty()
				? TEXT("Builder companion Resolver preview를 실행할 수 없습니다.")
				: ReadResult.Operation.Message;
			return false;
		}
		if (ReadResult.ResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
		{
			OutError = TEXT("Builder companion Resolver 결과가 Success가 아닙니다.");
			return false;
		}

		OutResolveResult = ReadResult.ResolveResult;
		OutError.Reset();
		return true;
	}

	// Missing private Profile을 complete seed payload로 transient binding해 baseline preservation과 prospective validity를 검증합니다.
	bool ValidateProspectiveCompanionResolve(
		const FCFBuilderCompanionRequest& Request,
		UCFVehicleData& Target,
		FCFBuilderCompanionPreview& OutPreview,
		FString& OutError)
	{
		// Missing VehicleBase Profile 여부입니다.
		const bool bMissingVehicleBase = Request.Recipe->ProfileBindings.VehicleBaseProfile.IsNull();
		// Missing Drivetrain Profile 여부입니다.
		const bool bMissingDrivetrain = Request.Recipe->ProfileBindings.DrivetrainProfile.IsNull();
		// Missing Handling Profile 여부입니다.
		const bool bMissingHandling = Request.Recipe->ProfileBindings.HandlingProfile.IsNull();
		// Missing Performance Profile 여부입니다.
		const bool bMissingPerformance = Request.Recipe->ProfileBindings.PerformanceProfile.IsNull();
		// 하나 이상의 private Profile seed가 필요한지 여부입니다.
		const bool bAnyProfileMissing = bMissingVehicleBase || bMissingDrivetrain || bMissingHandling || bMissingPerformance;

		// Current Recipe/Profile/ProjectDefault/Asset/Target snapshot을 stable baseline으로 확보할 shared read request입니다.
		FCFVehicleAuthoringReadRequest CurrentReadRequest;
		CurrentReadRequest.Recipe = Request.Recipe;
		CurrentReadRequest.TargetVehicleData = &Target;
		CurrentReadRequest.CallerKind = Request.CallContext.CallerKind;

		// Current stable snapshot과 Resolver 결과를 함께 보존하는 shared read result입니다.
		FCFVehicleResolveReadResult CurrentReadResult;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(CurrentReadRequest, CurrentReadResult))
		{
			OutError = CurrentReadResult.Operation.Message.IsEmpty()
				? TEXT("Builder companion current snapshot/Resolver preview를 실행할 수 없습니다.")
				: CurrentReadResult.Operation.Message;
			return false;
		}

		// Existing completion baseline 비교에 사용할 current Resolver 성공 여부입니다.
		const bool bCurrentResolveSucceeded = CurrentReadResult.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success;
		if (bCurrentResolveSucceeded)
		{
			OutPreview.CurrentResolvedDefinitionHash = CurrentReadResult.ResolveResult.ResolvedDefinitionHash;
		}
		else if (Request.Mode == ECFBuilderCompanionMode::CompleteExisting)
		{
			OutError = TEXT("기존 차량 baseline을 보존하려면 current Resolver가 먼저 Success여야 합니다.");
			return false;
		}

		if (!bAnyProfileMissing)
		{
			if (!bCurrentResolveSucceeded)
			{
				OutError = TEXT("Builder companion current Resolver 결과가 Success가 아닙니다.");
				return false;
			}
			OutPreview.ProspectiveResolvedDefinitionHash = CurrentReadResult.ResolveResult.ResolvedDefinitionHash;
			OutPreview.ProspectiveSourceSignature = CurrentReadResult.ResolveResult.SourceSignature;
			OutError.Reset();
			return true;
		}
		if (!Request.bHasInitialProfilePayload)
		{
			OutError = TEXT("Missing Builder-private Profile은 default constructor 값으로 만들 수 없습니다. current effective/reference proposal에서 만든 complete InitialProfilePayload가 필요합니다.");
			return false;
		}

		// Current stable snapshots를 복사하고 Missing private Profile domain만 prospective seed로 교체할 Pure Resolver request입니다.
		FCFVehicleResolveRequest ProspectiveResolveRequest = CurrentReadResult.ResolveRequest;

		if (bMissingVehicleBase)
		{
			// Complete VehicleBase seed payload를 fingerprint하기 위한 transient value carrier입니다.
			UCFVehicleBaseProfile* SeedProfile = NewObject<UCFVehicleBaseProfile>(GetTransientPackage());
			if (!SeedProfile)
			{
				OutError = TEXT("Builder companion VehicleBase seed carrier를 만들 수 없습니다.");
				return false;
			}
			SeedProfile->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
			SeedProfile->Data = Request.InitialProfilePayload.VehicleBaseData;
			// VehicleBase seed의 typed fingerprint snapshot입니다.
			FCFVehicleProfileSnapshotSet SeedSnapshot;
			if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(SeedProfile, nullptr, nullptr, nullptr, nullptr, SeedSnapshot, OutError))
			{
				return false;
			}
			SeedSnapshot.BaseSource.SourceObjectPath = OutPreview.VehicleBasePath;
			ProspectiveResolveRequest.Profiles.BaseSource = SeedSnapshot.BaseSource;
			ProspectiveResolveRequest.Profiles.BaseData = SeedSnapshot.BaseData;
			ProspectiveResolveRequest.Recipe.ProfileBindings.VehicleBaseProfile = TSoftObjectPtr<UCFVehicleBaseProfile>(OutPreview.VehicleBasePath);
		}
		if (bMissingDrivetrain)
		{
			// Complete Drivetrain seed payload를 fingerprint하기 위한 transient value carrier입니다.
			UCFDrivetrainProfile* SeedProfile = NewObject<UCFDrivetrainProfile>(GetTransientPackage());
			if (!SeedProfile)
			{
				OutError = TEXT("Builder companion Drivetrain seed carrier를 만들 수 없습니다.");
				return false;
			}
			SeedProfile->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
			SeedProfile->Data = Request.InitialProfilePayload.DrivetrainData;
			// Drivetrain seed의 typed fingerprint snapshot입니다.
			FCFVehicleProfileSnapshotSet SeedSnapshot;
			if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, SeedProfile, nullptr, nullptr, nullptr, SeedSnapshot, OutError))
			{
				return false;
			}
			SeedSnapshot.DrivetrainSource.SourceObjectPath = OutPreview.DrivetrainPath;
			ProspectiveResolveRequest.Profiles.DrivetrainSource = SeedSnapshot.DrivetrainSource;
			ProspectiveResolveRequest.Profiles.DrivetrainData = SeedSnapshot.DrivetrainData;
			ProspectiveResolveRequest.Recipe.ProfileBindings.DrivetrainProfile = TSoftObjectPtr<UCFDrivetrainProfile>(OutPreview.DrivetrainPath);
		}
		if (bMissingHandling)
		{
			// Complete Handling seed payload를 fingerprint하기 위한 transient value carrier입니다.
			UCFHandlingProfile* SeedProfile = NewObject<UCFHandlingProfile>(GetTransientPackage());
			if (!SeedProfile)
			{
				OutError = TEXT("Builder companion Handling seed carrier를 만들 수 없습니다.");
				return false;
			}
			SeedProfile->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
			SeedProfile->Data = Request.InitialProfilePayload.HandlingData;
			// Handling seed의 typed fingerprint snapshot입니다.
			FCFVehicleProfileSnapshotSet SeedSnapshot;
			if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, nullptr, SeedProfile, nullptr, nullptr, SeedSnapshot, OutError))
			{
				return false;
			}
			SeedSnapshot.HandlingSource.SourceObjectPath = OutPreview.HandlingPath;
			ProspectiveResolveRequest.Profiles.HandlingSource = SeedSnapshot.HandlingSource;
			ProspectiveResolveRequest.Profiles.HandlingData = SeedSnapshot.HandlingData;
			ProspectiveResolveRequest.Recipe.ProfileBindings.HandlingProfile = TSoftObjectPtr<UCFHandlingProfile>(OutPreview.HandlingPath);
		}
		if (bMissingPerformance)
		{
			// Complete Performance seed payload를 fingerprint하기 위한 transient value carrier입니다.
			UCFPerformanceProfile* SeedProfile = NewObject<UCFPerformanceProfile>(GetTransientPackage());
			if (!SeedProfile)
			{
				OutError = TEXT("Builder companion Performance seed carrier를 만들 수 없습니다.");
				return false;
			}
			SeedProfile->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
			SeedProfile->Data = Request.InitialProfilePayload.PerformanceData;
			// Performance seed의 typed fingerprint snapshot입니다.
			FCFVehicleProfileSnapshotSet SeedSnapshot;
			if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, nullptr, nullptr, SeedProfile, nullptr, SeedSnapshot, OutError))
			{
				return false;
			}
			SeedSnapshot.PerformanceSource.SourceObjectPath = OutPreview.PerformancePath;
			ProspectiveResolveRequest.Profiles.PerformanceSource = SeedSnapshot.PerformanceSource;
			ProspectiveResolveRequest.Profiles.PerformanceData = SeedSnapshot.PerformanceData;
			ProspectiveResolveRequest.Recipe.ProfileBindings.PerformanceProfile = TSoftObjectPtr<UCFPerformanceProfile>(OutPreview.PerformancePath);
		}

		// Prospective persistent binding path가 semantic Recipe intent에 반영됐으므로 deterministic Recipe fingerprint를 다시 계산합니다.
		if (!FCFVehicleSnapshotBuilder::BuildRecipeFingerprintFromSnapshot(
			ProspectiveResolveRequest.Recipe,
			ProspectiveResolveRequest.Recipe.RecipeFingerprint,
			OutError))
		{
			return false;
		}
		ProspectiveResolveRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;

		// Transient allocation identity와 무관한 prospective persistent paths + typed payload만으로 계산한 Resolver result입니다.
		FCFVehicleResolveResult ProspectiveResolveResult;
		if (!FCFVehicleResolver::Resolve(ProspectiveResolveRequest, ProspectiveResolveResult))
		{
			OutError = TEXT("Builder companion prospective Shared Pure Resolver가 internal Error로 실패했습니다.");
			return false;
		}
		if (ProspectiveResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
		{
			OutError = TEXT("Builder companion prospective Resolver 결과가 Success가 아닙니다.");
			return false;
		}
		OutPreview.ProspectiveResolvedDefinitionHash = ProspectiveResolveResult.ResolvedDefinitionHash;
		OutPreview.ProspectiveSourceSignature = ProspectiveResolveResult.SourceSignature;

		if (Request.Mode == ECFBuilderCompanionMode::CompleteExisting
			&& OutPreview.CurrentResolvedDefinitionHash != OutPreview.ProspectiveResolvedDefinitionHash)
		{
			OutError = TEXT("Existing Vehicle Completion의 private Profile seed가 current resolved vehicle state를 변경합니다. baseline-preserving complete seed를 다시 생성·review해야 합니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Preview path 다섯 개가 서로 다른 exact object identity인지 검사합니다.
	bool ValidateDistinctPaths(const FCFBuilderCompanionPreview& Preview, FString& OutError)
	{
		// Duplicate detection에 사용할 canonical path set입니다.
		TSet<FString> UniquePaths;
		const FSoftObjectPath Paths[] =
		{
			Preview.EvidencePath,
			Preview.VehicleBasePath,
			Preview.DrivetrainPath,
			Preview.HandlingPath,
			Preview.PerformancePath
		};
		for (const FSoftObjectPath& Path : Paths)
		{
			if (!Path.IsValid() || UniquePaths.Contains(Path.ToString()))
			{
				OutError = TEXT("Evidence와 private 4 Profile은 서로 다른 유효 object identity여야 합니다.");
				return false;
			}
			UniquePaths.Add(Path.ToString());
		}
		OutError.Reset();
		return true;
	}
}

// Existing Definition+Recipe에 Reference Evidence + private 4 Profile을 만들 exact R2 companion plan을 mutation0 preview합니다.
bool FCFVehicleAuthoringService::PreviewBuilderCompanions(
	const FCFBuilderCompanionRequest& Request,
	FCFBuilderCompanionPreview& OutPreview)
{
	OutPreview = FCFBuilderCompanionPreview();
	CFVehicleBuilderCreatePrivate::InitializeResult(OutPreview.Operation, TEXT("PreviewBuilderCompanions"), ECFAuthoringRiskClass::R0_ReadOnly, Request.CallContext.ClientOperationId);
	if (!Request.Recipe || !Request.Recipe->RecipeId.IsValid())
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Builder companion preview에는 persistent Recipe와 valid RecipeId가 필요합니다."));
	}
	if (Request.Recipe->ImportState.ManageState == ECFVehicleManageState::Unmanaged)
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::ManagedRequired, TEXT("Unmanaged VehicleData는 Initial Import로 Recipe ownership을 먼저 확립해야 합니다."));
	}

	// Recipe binding에서 load한 canonical Target VehicleData입니다.
	UCFVehicleData* TargetVehicleData = Request.Recipe->TargetVehicleData.LoadSynchronous();
	if (!TargetVehicleData)
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Recipe-bound Target VehicleData를 load할 수 없습니다."));
	}

	// Current Recipe/Target stale authority입니다.
	FString RecipeFingerprint;
	// Current full Target Definition hash입니다.
	FString TargetDefinitionHash;
	// Snapshot/path validation diagnostic입니다.
	FString Error;
	if (!CFVehicleBuilderCreatePrivate::BuildCurrentHashes(*Request.Recipe, *TargetVehicleData, RecipeFingerprint, TargetDefinitionHash, Error))
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::InternalError, Error);
	}

	if (Request.ExistingEvidencePath.IsValid())
	{
		OutPreview.EvidencePath = Request.ExistingEvidencePath;
	}
	else if (!CFVehicleBuilderCreatePrivate::ValidateNewAssetIdentity(Request.EvidenceAsset, OutPreview.EvidencePath, Error))
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
	}

	if (!CFVehicleBuilderCreatePrivate::BuildProspectiveEvidenceFingerprint(Request, *TargetVehicleData, OutPreview, Error))
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, Error);
	}

	if (!Request.Recipe->ProfileBindings.VehicleBaseProfile.IsNull())
	{
		if (!CFVehicleBuilderCreatePrivate::ValidateVehicleBaseProfile(Request.Recipe->ProfileBindings.VehicleBaseProfile, Request.Recipe->RecipeId, Error))
		{
			return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
		}
		OutPreview.VehicleBasePath = Request.Recipe->ProfileBindings.VehicleBaseProfile.ToSoftObjectPath();
	}
	else if (!CFVehicleBuilderCreatePrivate::ValidateNewAssetIdentity(Request.VehicleBaseAsset, OutPreview.VehicleBasePath, Error))
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
	}

	if (!Request.Recipe->ProfileBindings.DrivetrainProfile.IsNull())
	{
		if (!CFVehicleBuilderCreatePrivate::ValidateDrivetrainProfile(Request.Recipe->ProfileBindings.DrivetrainProfile, Request.Recipe->RecipeId, Error))
		{
			return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
		}
		OutPreview.DrivetrainPath = Request.Recipe->ProfileBindings.DrivetrainProfile.ToSoftObjectPath();
	}
	else if (!CFVehicleBuilderCreatePrivate::ValidateNewAssetIdentity(Request.DrivetrainAsset, OutPreview.DrivetrainPath, Error))
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
	}

	if (!Request.Recipe->ProfileBindings.HandlingProfile.IsNull())
	{
		if (!CFVehicleBuilderCreatePrivate::ValidateHandlingProfile(Request.Recipe->ProfileBindings.HandlingProfile, Request.Recipe->RecipeId, Error))
		{
			return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
		}
		OutPreview.HandlingPath = Request.Recipe->ProfileBindings.HandlingProfile.ToSoftObjectPath();
	}
	else if (!CFVehicleBuilderCreatePrivate::ValidateNewAssetIdentity(Request.HandlingAsset, OutPreview.HandlingPath, Error))
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
	}

	if (!Request.Recipe->ProfileBindings.PerformanceProfile.IsNull())
	{
		if (!CFVehicleBuilderCreatePrivate::ValidatePerformanceProfile(Request.Recipe->ProfileBindings.PerformanceProfile, Request.Recipe->RecipeId, Error))
		{
			return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
		}
		OutPreview.PerformancePath = Request.Recipe->ProfileBindings.PerformanceProfile.ToSoftObjectPath();
	}
	else if (!CFVehicleBuilderCreatePrivate::ValidateNewAssetIdentity(Request.PerformanceAsset, OutPreview.PerformancePath, Error))
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutPreview.Operation, ECFAuthoringErrorCode::StateChanged, Error);
	}

	if (!CFVehicleBuilderCreatePrivate::ValidateDistinctPaths(OutPreview, Error))
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::InvalidSemanticInput, Error);
	}
	if (!CFVehicleBuilderCreatePrivate::ValidateProspectiveCompanionResolve(Request, *TargetVehicleData, OutPreview, Error))
	{
		return CFVehicleBuilderCreatePrivate::Block(OutPreview.Operation, ECFAuthoringErrorCode::ValidationBlocked, Error);
	}

	OutPreview.Operation.CurrentRecipeFingerprint = RecipeFingerprint;
	OutPreview.Operation.CurrentTargetDefinitionHash = TargetDefinitionHash;
	OutPreview.Proposal.OperationName = CFVehicleBuilderCreatePrivate::CreateOperationName;
	OutPreview.Proposal.RiskClass = ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite;
	OutPreview.Proposal.RequiredApprovalClass = ECFAuthoringApprovalClass::OwnershipWrite;
	OutPreview.Proposal.ExpectedRecipeFingerprint = RecipeFingerprint;
	OutPreview.Proposal.ExpectedTargetDefinitionHash = TargetDefinitionHash;
	OutPreview.Proposal.ProspectiveSourceSignature = OutPreview.ProspectiveSourceSignature;
	OutPreview.Proposal.ProspectiveResolvedDefinitionHash = OutPreview.ProspectiveResolvedDefinitionHash;
	OutPreview.Proposal.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	OutPreview.Proposal.bTargetMutation = false;
	OutPreview.Proposal.bSavePerformed = false;
	OutPreview.Proposal.ProposalHash = CFVehicleBuilderCreatePrivate::BuildProposalHash(Request, OutPreview, RecipeFingerprint, TargetDefinitionHash);
	CFVehicleBuilderCreatePrivate::Succeed(OutPreview.Operation, TEXT("Existing Builder companion은 보존하고 Missing Evidence/private Profile만 생성할 R2 plan을 preview했습니다."));
	return true;
}

// Fresh OwnershipWrite approval 뒤 Evidence + private 4 Profile 생성과 Recipe binding을 한 transaction으로 commit합니다.
bool FCFVehicleAuthoringService::CreateBuilderCompanions(
	const FCFBuilderCompanionRequest& Request,
	FCFBuilderCompanionResult& OutResult)
{
	OutResult = FCFBuilderCompanionResult();
	CFVehicleBuilderCreatePrivate::InitializeResult(OutResult.Operation, CFVehicleBuilderCreatePrivate::CreateOperationName, ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite, Request.CallContext.ClientOperationId);

	// Commit 직전 current state에서 다시 만든 exact companion preview입니다.
	FCFBuilderCompanionPreview FreshPreview;
	if (!PreviewBuilderCompanions(Request, FreshPreview))
	{
		OutResult.Operation = FreshPreview.Operation;
		OutResult.Operation.OperationName = CFVehicleBuilderCreatePrivate::CreateOperationName;
		return false;
	}
	if (Request.CallContext.ApprovalClass != ECFAuthoringApprovalClass::OwnershipWrite)
	{
		return CFVehicleBuilderCreatePrivate::Block(OutResult.Operation, ECFAuthoringErrorCode::ApprovalRequired, TEXT("Builder companion 생성에는 explicit OwnershipWrite approval이 필요합니다."));
	}
	if (Request.CallContext.ApprovalScopeHash.IsEmpty() || Request.CallContext.ApprovalScopeHash != FreshPreview.Proposal.ProposalHash)
	{
		return CFVehicleBuilderCreatePrivate::Block(OutResult.Operation, ECFAuthoringErrorCode::ApprovalScopeMismatch, TEXT("Builder companion approval scope가 fresh Recipe/Target/path state와 일치하지 않습니다."));
	}
	if (!Request.CallContext.ExpectedRecipeFingerprint.IsEmpty()
		&& Request.CallContext.ExpectedRecipeFingerprint != FreshPreview.Proposal.ExpectedRecipeFingerprint)
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutResult.Operation, ECFAuthoringErrorCode::RecipeFingerprintMismatch, TEXT("Builder companion commit 전 Recipe가 preview 이후 변경되었습니다."));
	}
	if (!Request.CallContext.ExpectedTargetDefinitionHash.IsEmpty()
		&& Request.CallContext.ExpectedTargetDefinitionHash != FreshPreview.Proposal.ExpectedTargetDefinitionHash)
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutResult.Operation, ECFAuthoringErrorCode::TargetHashMismatch, TEXT("Builder companion commit 전 Target VehicleData가 preview 이후 변경되었습니다."));
	}
	if (Request.CallContext.ExpectedResolverContractRevision != 0
		&& Request.CallContext.ExpectedResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		return CFVehicleBuilderCreatePrivate::Conflict(OutResult.Operation, ECFAuthoringErrorCode::ResolverRevisionMismatch, TEXT("Builder companion preview 이후 Resolver contract revision이 변경되었습니다."));
	}
	if (!Request.Recipe)
	{
		return CFVehicleBuilderCreatePrivate::Block(OutResult.Operation, ECFAuthoringErrorCode::RecipeNotFound, TEXT("Builder companion commit에는 Recipe가 필요합니다."));
	}

	// Recipe binding에서 load한 canonical Target VehicleData입니다.
	UCFVehicleData* TargetVehicleData = Request.Recipe->TargetVehicleData.LoadSynchronous();
	if (!TargetVehicleData)
	{
		return CFVehicleBuilderCreatePrivate::Block(OutResult.Operation, ECFAuthoringErrorCode::TargetNotFound, TEXT("Builder companion commit Target VehicleData를 load할 수 없습니다."));
	}

	// Existing Evidence가 없어서 새 Evidence를 만들어야 하는지 여부입니다.
	const bool bCreateEvidence = !Request.ExistingEvidencePath.IsValid();
	// Existing VehicleBase binding이 없어서 새 Profile을 만들어야 하는지 여부입니다.
	const bool bCreateVehicleBase = Request.Recipe->ProfileBindings.VehicleBaseProfile.IsNull();
	// Existing Drivetrain binding이 없어서 새 Profile을 만들어야 하는지 여부입니다.
	const bool bCreateDrivetrain = Request.Recipe->ProfileBindings.DrivetrainProfile.IsNull();
	// Existing Handling binding이 없어서 새 Profile을 만들어야 하는지 여부입니다.
	const bool bCreateHandling = Request.Recipe->ProfileBindings.HandlingProfile.IsNull();
	// Existing Performance binding이 없어서 새 Profile을 만들어야 하는지 여부입니다.
	const bool bCreatePerformance = Request.Recipe->ProfileBindings.PerformanceProfile.IsNull();
	if (!bCreateEvidence && !bCreateVehicleBase && !bCreateDrivetrain && !bCreateHandling && !bCreatePerformance)
	{
		OutResult.Operation.CurrentRecipeFingerprint = FreshPreview.Proposal.ExpectedRecipeFingerprint;
		OutResult.Operation.CurrentTargetDefinitionHash = FreshPreview.Proposal.ExpectedTargetDefinitionHash;
		CFVehicleBuilderCreatePrivate::NoChange(OutResult.Operation, TEXT("Reference Evidence와 Builder-private 4 Profile이 이미 complete 상태라 생성할 companion이 없습니다."));
		return true;
	}

	// 모든 companion creation과 Recipe binding을 하나의 Undo 단위로 묶습니다.
	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "CreateBuilderCompanions", "CarFight Create Builder Companions"));
	Request.Recipe->Modify();

	// 새 Reference Evidence package입니다.
	UPackage* EvidencePackage = bCreateEvidence ? CreatePackage(*Request.EvidenceAsset.PackageName) : nullptr;
	// 새 VehicleBase Profile package입니다.
	UPackage* VehicleBasePackage = bCreateVehicleBase ? CreatePackage(*Request.VehicleBaseAsset.PackageName) : nullptr;
	// 새 Drivetrain Profile package입니다.
	UPackage* DrivetrainPackage = bCreateDrivetrain ? CreatePackage(*Request.DrivetrainAsset.PackageName) : nullptr;
	// 새 Handling Profile package입니다.
	UPackage* HandlingPackage = bCreateHandling ? CreatePackage(*Request.HandlingAsset.PackageName) : nullptr;
	// 새 Performance Profile package입니다.
	UPackage* PerformancePackage = bCreatePerformance ? CreatePackage(*Request.PerformanceAsset.PackageName) : nullptr;
	if ((bCreateEvidence && !EvidencePackage)
		|| (bCreateVehicleBase && !VehicleBasePackage)
		|| (bCreateDrivetrain && !DrivetrainPackage)
		|| (bCreateHandling && !HandlingPackage)
		|| (bCreatePerformance && !PerformancePackage))
	{
		Transaction.Cancel();
		return CFVehicleBuilderCreatePrivate::Block(OutResult.Operation, ECFAuthoringErrorCode::InternalError, TEXT("Builder companion package 생성에 실패했습니다."));
	}

	// 새 Reference Evidence UObject입니다.
	UCFVehicleRefEvidence* Evidence = bCreateEvidence
		? NewObject<UCFVehicleRefEvidence>(EvidencePackage, Request.EvidenceAsset.AssetName, RF_Public | RF_Standalone | RF_Transactional)
		: Cast<UCFVehicleRefEvidence>(Request.ExistingEvidencePath.ResolveObject());
	if (!Evidence && !bCreateEvidence)
	{
		Evidence = Cast<UCFVehicleRefEvidence>(Request.ExistingEvidencePath.TryLoad());
	}
	// 새 VehicleBase Profile UObject입니다.
	UCFVehicleBaseProfile* VehicleBase = bCreateVehicleBase
		? NewObject<UCFVehicleBaseProfile>(VehicleBasePackage, Request.VehicleBaseAsset.AssetName, RF_Public | RF_Standalone | RF_Transactional)
		: Request.Recipe->ProfileBindings.VehicleBaseProfile.LoadSynchronous();
	// 새 Drivetrain Profile UObject입니다.
	UCFDrivetrainProfile* Drivetrain = bCreateDrivetrain
		? NewObject<UCFDrivetrainProfile>(DrivetrainPackage, Request.DrivetrainAsset.AssetName, RF_Public | RF_Standalone | RF_Transactional)
		: Request.Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	// 새 Handling Profile UObject입니다.
	UCFHandlingProfile* Handling = bCreateHandling
		? NewObject<UCFHandlingProfile>(HandlingPackage, Request.HandlingAsset.AssetName, RF_Public | RF_Standalone | RF_Transactional)
		: Request.Recipe->ProfileBindings.HandlingProfile.LoadSynchronous();
	// 새 Performance Profile UObject입니다.
	UCFPerformanceProfile* Performance = bCreatePerformance
		? NewObject<UCFPerformanceProfile>(PerformancePackage, Request.PerformanceAsset.AssetName, RF_Public | RF_Standalone | RF_Transactional)
		: Request.Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!Evidence || !VehicleBase || !Drivetrain || !Handling || !Performance)
	{
		Transaction.Cancel();
		OutResult.Operation.Status = ECFAuthoringOpStatus::FailedUnknownState;
		OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::InternalError;
		OutResult.Operation.Message = TEXT("Builder companion UObject 생성/resolve 중 예상하지 못한 실패가 발생했습니다. 새 asset은 Save하지 않았습니다.");
		return false;
	}

	if (bCreateEvidence)
	{
		Evidence->Modify();
		Evidence->EvidenceId = Request.NewEvidenceId;
		Evidence->TargetRecipeId = Request.Recipe->RecipeId;
		Evidence->TargetRecipePath = FSoftObjectPath(Request.Recipe);
		Evidence->TargetDefinitionPath = FSoftObjectPath(TargetVehicleData);
		// 새 semantic ownership/research payload를 적용하는 diagnostic입니다.
		FString EvidenceError;
		if (!Evidence->ApplyInitialResearchPayload(Request.InitialEvidencePayload, EvidenceError)
			|| Evidence->EvidenceFingerprint != FreshPreview.ProspectiveEvidenceFingerprint)
		{
			Transaction.Cancel();
			OutResult.Operation.Status = ECFAuthoringOpStatus::FailedUnknownState;
			OutResult.Operation.ErrorCode = ECFAuthoringErrorCode::InternalError;
			OutResult.Operation.Message = EvidenceError.IsEmpty()
				? TEXT("Committed Reference Evidence fingerprint가 approved prospective fingerprint와 일치하지 않습니다.")
				: EvidenceError;
			return false;
		}
	}
	if (bCreateVehicleBase)
	{
		VehicleBase->Modify();
		VehicleBase->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
		VehicleBase->Data = Request.InitialProfilePayload.VehicleBaseData;
		Request.Recipe->ProfileBindings.VehicleBaseProfile = VehicleBase;
	}
	if (bCreateDrivetrain)
	{
		Drivetrain->Modify();
		Drivetrain->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
		Drivetrain->Data = Request.InitialProfilePayload.DrivetrainData;
		Request.Recipe->ProfileBindings.DrivetrainProfile = Drivetrain;
	}
	if (bCreateHandling)
	{
		Handling->Modify();
		Handling->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
		Handling->Data = Request.InitialProfilePayload.HandlingData;
		Request.Recipe->ProfileBindings.HandlingProfile = Handling;
	}
	if (bCreatePerformance)
	{
		Performance->Modify();
		Performance->Meta.OwnerRecipeId = Request.Recipe->RecipeId;
		Performance->Data = Request.InitialProfilePayload.PerformanceData;
		Request.Recipe->ProfileBindings.PerformanceProfile = Performance;
	}

	Request.Recipe->AuthoringRevision += 1;
	if (bCreateEvidence)
	{
		FAssetRegistryModule::AssetCreated(Evidence);
		Evidence->MarkPackageDirty();
		Evidence->PostEditChange();
	}
	if (bCreateVehicleBase)
	{
		FAssetRegistryModule::AssetCreated(VehicleBase);
		VehicleBase->MarkPackageDirty();
		VehicleBase->PostEditChange();
	}
	if (bCreateDrivetrain)
	{
		FAssetRegistryModule::AssetCreated(Drivetrain);
		Drivetrain->MarkPackageDirty();
		Drivetrain->PostEditChange();
	}
	if (bCreateHandling)
	{
		FAssetRegistryModule::AssetCreated(Handling);
		Handling->MarkPackageDirty();
		Handling->PostEditChange();
	}
	if (bCreatePerformance)
	{
		FAssetRegistryModule::AssetCreated(Performance);
		Performance->MarkPackageDirty();
		Performance->PostEditChange();
	}
	Request.Recipe->MarkPackageDirty();
	Request.Recipe->PostEditChange();

	OutResult.CreatedEvidence = bCreateEvidence ? Evidence : nullptr;
	OutResult.CreatedVehicleBase = bCreateVehicleBase ? VehicleBase : nullptr;
	OutResult.CreatedDrivetrain = bCreateDrivetrain ? Drivetrain : nullptr;
	OutResult.CreatedHandling = bCreateHandling ? Handling : nullptr;
	OutResult.CreatedPerformance = bCreatePerformance ? Performance : nullptr;
	OutResult.Operation.Mutation.bRecipeChanged = bCreateVehicleBase || bCreateDrivetrain || bCreateHandling || bCreatePerformance;
	// 새 private Profile Asset 생성은 기존/shared Profile payload mutation이 아니므로 false를 유지합니다.
	OutResult.Operation.Mutation.bProfileChanged = false;
	OutResult.Operation.Mutation.bCreatedAssets = bCreateEvidence || bCreateVehicleBase || bCreateDrivetrain || bCreateHandling || bCreatePerformance;
	OutResult.Operation.Mutation.bPackageDirty = true;
	OutResult.Operation.Mutation.bTargetChanged = false;
	OutResult.Operation.Mutation.bSavePerformed = false;
	OutResult.Operation.Mutation.bAutomaticRetryPerformed = false;
	CFVehicleBuilderCreatePrivate::Succeed(OutResult.Operation, TEXT("Existing companion은 보존하고 Missing Reference Evidence/Builder-private Profile만 생성·Recipe binding했습니다. Save/Apply는 수행하지 않았습니다."));
	return true;
}
