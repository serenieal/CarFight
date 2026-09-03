// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleCatalogPromoService.cpp
// Version: v1.0.0
// Date: 2026-09-02
// Description: CF-FQ-044 VRCP-P0-02 Editor-only Runtime Catalog Promotion 구현입니다.
// Changelog:
// - v1.0.0: raw Settings resolve → target/catalog pre-validation → exact membership → transaction add → post-validation/readback → defensive rollback을 구현.
// Migration:
// - 정상 성공은 Catalog package를 dirty로 남기며 자동 Save를 수행하지 않습니다.
// - 내부 실패 rollback만 pre-dirty와 exact AllowedVehicleData 배열을 복원하고 normal USER Undo/Redo는 표준 Transaction에 맡깁니다.

#include "DataAuthoring/CFVehicleCatalogPromoService.h"

#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"
#include "CFVehicleData.h"
#include "Misc/PackageName.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"

#define LOCTEXT_NAMESPACE "CFVehicleCatalogPromoService"

namespace CFVehicleCatalogPromoPrivate
{
	// Result의 공통 no-save/readback 필드를 current object 상태로 채웁니다.
	void PopulateCommonResultState(
		FCFVehicleCatalogPromoResult& InOutResult,
		UCFVehicleData* TargetVehicleData,
		UCFRuntimeTestCatalogData* RuntimeCatalog)
	{
		InOutResult.TargetVehicleData = TargetVehicleData;
		InOutResult.Catalog = RuntimeCatalog;
		InOutResult.bSavePerformed = false;
		InOutResult.bCatalogPackageDirty = RuntimeCatalog
			&& RuntimeCatalog->GetOutermost()
			&& RuntimeCatalog->GetOutermost()->IsDirty();
		InOutResult.bIsMember = RuntimeCatalog
			&& TargetVehicleData
			&& RuntimeCatalog->AllowedVehicleData.ContainsByPredicate(
				[TargetVehicleData](const TObjectPtr<UCFVehicleData>& CandidateVehicleData)
				{
					return CandidateVehicleData.Get() == TargetVehicleData;
				});
	}

	// Catalog full validation 오류를 한 줄 bounded diagnostic으로 결합합니다.
	FString BuildCatalogValidationError(const TArray<FText>& ValidationErrors)
	{
		// 최대 3개 validation issue를 표시할 문자열 배열입니다.
		TArray<FString> ErrorTexts;
		const int32 ErrorCountToReport = FMath::Min(ValidationErrors.Num(), 3);
		ErrorTexts.Reserve(ErrorCountToReport);
		for (int32 ErrorIndex = 0; ErrorIndex < ErrorCountToReport; ++ErrorIndex)
		{
			ErrorTexts.Add(ValidationErrors[ErrorIndex].ToString());
		}

		// 추가로 생략된 validation issue 수입니다.
		const int32 OmittedErrorCount = ValidationErrors.Num() - ErrorCountToReport;
		FString Result = FString::Join(ErrorTexts, TEXT(" | "));
		if (OmittedErrorCount > 0)
		{
			Result += FString::Printf(TEXT(" | +%d more"), OmittedErrorCount);
		}
		return Result;
	}

	// Catalog가 promotion 전/후 전체 RuntimeTestCatalog 계약을 만족하는지 검사합니다.
	bool ValidateCatalog(
		const UCFRuntimeTestCatalogData& RuntimeCatalog,
		FString& OutError)
	{
		// Catalog가 반환한 전체 validation issue입니다.
		TArray<FText> ValidationErrors;
		if (!RuntimeCatalog.ValidateRuntimeTestCatalog(ValidationErrors))
		{
			OutError = BuildCatalogValidationError(ValidationErrors);
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Exact VehicleData pointer identity가 AllowedVehicleData에 몇 번 존재하는지 셉니다.
	int32 CountExactMembership(
		const UCFRuntimeTestCatalogData& RuntimeCatalog,
		const UCFVehicleData* TargetVehicleData)
	{
		// Exact pointer identity 일치 개수입니다.
		int32 MembershipCount = 0;
		for (const TObjectPtr<UCFVehicleData>& CandidateVehicleData : RuntimeCatalog.AllowedVehicleData)
		{
			if (CandidateVehicleData.Get() == TargetVehicleData)
			{
				++MembershipCount;
			}
		}
		return MembershipCount;
	}

	// Service 내부 mutation failure에서 AllowedVehicleData와 package dirty를 transaction 이전 상태로 exact 복원합니다.
	void RestoreCatalogAfterInternalFailure(
		UCFRuntimeTestCatalogData& RuntimeCatalog,
		const TArray<TObjectPtr<UCFVehicleData>>& PreviousAllowedVehicleData,
		const bool bPackageWasDirty,
		FScopedTransaction& Transaction)
	{
		RuntimeCatalog.AllowedVehicleData = PreviousAllowedVehicleData;
		RuntimeCatalog.PostEditChange();
		Transaction.Cancel();

		// Catalog outer package입니다.
		UPackage* RuntimeCatalogPackage = RuntimeCatalog.GetOutermost();
		if (RuntimeCatalogPackage)
		{
			RuntimeCatalogPackage->SetDirtyFlag(bPackageWasDirty);
		}
	}
}

// Project Settings의 current DefaultCatalog soft reference를 raw load하며 Catalog validity는 아직 판정하지 않습니다.
bool FCFVehicleCatalogPromoService::ResolveDefaultRuntimeCatalogRaw(
	UCFRuntimeTestCatalogData*& OutCatalog,
	FCFVehicleCatalogPromoResult& OutFailure)
{
	// Current Game Config Runtime Test Settings CDO입니다.
	const UCFRuntimeTestSettings* RuntimeTestSettings = GetDefault<UCFRuntimeTestSettings>();
	return ResolveRuntimeCatalogRaw(RuntimeTestSettings, OutCatalog, OutFailure);
}

// 명시 Settings object의 DefaultCatalog soft reference를 raw load하며 focused automation에서도 동일 계약을 재사용합니다.
bool FCFVehicleCatalogPromoService::ResolveRuntimeCatalogRaw(
	const UCFRuntimeTestSettings* RuntimeTestSettings,
	UCFRuntimeTestCatalogData*& OutCatalog,
	FCFVehicleCatalogPromoResult& OutFailure)
{
	OutCatalog = nullptr;
	OutFailure = FCFVehicleCatalogPromoResult();
	OutFailure.Outcome = ECFVehicleCatalogPromoOutcome::CatalogUnavailable;
	OutFailure.bSavePerformed = false;

	if (!IsValid(RuntimeTestSettings))
	{
		OutFailure.Message = TEXT("Runtime Test Settings를 사용할 수 없습니다.");
		return false;
	}
	if (RuntimeTestSettings->DefaultCatalog.IsNull())
	{
		OutFailure.Message = TEXT("Runtime Test Settings의 DefaultCatalog가 지정되지 않았습니다.");
		return false;
	}

	// Settings soft reference에서 raw load한 Catalog입니다.
	UCFRuntimeTestCatalogData* LoadedCatalog = RuntimeTestSettings->DefaultCatalog.LoadSynchronous();
	if (!IsValid(LoadedCatalog))
	{
		OutFailure.Message = FString::Printf(
			TEXT("Default Runtime Test Catalog를 raw load할 수 없습니다: %s"),
			*RuntimeTestSettings->DefaultCatalog.ToSoftObjectPath().ToString());
		return false;
	}

	OutCatalog = LoadedCatalog;
	OutFailure.Catalog = LoadedCatalog;
	OutFailure.Message.Reset();
	return true;
}

// Promotion target이 exact persistent /Game VehicleData인지 fail-closed로 검증합니다.
bool FCFVehicleCatalogPromoService::ValidatePromotionTarget(
	const UCFVehicleData* TargetVehicleData,
	FString& OutError)
{
	if (!IsValid(TargetVehicleData))
	{
		OutError = TEXT("Promotion target VehicleData가 null이거나 유효하지 않습니다.");
		return false;
	}
	if (TargetVehicleData->HasAnyFlags(RF_Transient))
	{
		OutError = TEXT("RF_Transient VehicleData는 Runtime Catalog에 등록할 수 없습니다.");
		return false;
	}

	// Target의 owning package입니다.
	const UPackage* TargetPackage = TargetVehicleData->GetOutermost();
	if (!TargetPackage || TargetPackage == GetTransientPackage())
	{
		OutError = TEXT("TransientPackage VehicleData는 Runtime Catalog에 등록할 수 없습니다.");
		return false;
	}
	if (TargetPackage->HasAnyPackageFlags(PKG_PlayInEditor))
	{
		OutError = TEXT("PIE package VehicleData는 Runtime Catalog에 등록할 수 없습니다.");
		return false;
	}

	// Target persistent package name입니다.
	const FString PackageName = TargetPackage->GetName();
	if (!PackageName.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
		|| !FPackageName::IsValidLongPackageName(PackageName))
	{
		OutError = FString::Printf(
			TEXT("Promotion target은 persistent /Game package VehicleData여야 합니다: %s"),
			*PackageName);
		return false;
	}

	// Exact persistent object identity를 나타내는 soft object path입니다.
	const FSoftObjectPath TargetObjectPath(TargetVehicleData);
	if (!TargetObjectPath.IsValid() || TargetObjectPath.IsNull())
	{
		OutError = TEXT("Promotion target VehicleData에 valid persistent object path가 없습니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// Current Catalog 전체 validity를 먼저 확인한 뒤 exact VehicleData membership을 fresh readback합니다.
FCFVehicleCatalogPromoResult FCFVehicleCatalogPromoService::ReadPromotionMembership(
	UCFVehicleData* TargetVehicleData,
	UCFRuntimeTestCatalogData* RuntimeCatalog)
{
	// Fresh membership readback 결과입니다.
	FCFVehicleCatalogPromoResult Result;
	Result.bSavePerformed = false;

	// Target validation 오류입니다.
	FString TargetValidationError;
	if (!ValidatePromotionTarget(TargetVehicleData, TargetValidationError))
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::InvalidTarget;
		Result.Message = TargetValidationError;
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}
	if (!IsValid(RuntimeCatalog))
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::CatalogUnavailable;
		Result.Message = TEXT("Runtime Test Catalog를 사용할 수 없습니다.");
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	// Full Catalog validation 오류입니다.
	FString CatalogValidationError;
	if (!CFVehicleCatalogPromoPrivate::ValidateCatalog(*RuntimeCatalog, CatalogValidationError))
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::CatalogInvalid;
		Result.Message = FString::Printf(TEXT("Runtime Test Catalog validation 실패: %s"), *CatalogValidationError);
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	// Exact pointer identity membership count입니다.
	const int32 MembershipCount = CFVehicleCatalogPromoPrivate::CountExactMembership(*RuntimeCatalog, TargetVehicleData);
	Result.Outcome = MembershipCount == 1
		? ECFVehicleCatalogPromoOutcome::AlreadyRegistered
		: ECFVehicleCatalogPromoOutcome::MutationFailed;
	Result.Message = MembershipCount == 1
		? TEXT("VehicleData가 Runtime Test Catalog에 exact 1회 등록되어 있습니다.")
		: TEXT("VehicleData가 Runtime Test Catalog에 등록되어 있지 않습니다.");
	CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
	return Result;
}

// Project Settings의 DefaultCatalog로 exact persistent VehicleData를 idempotent promotion합니다.
FCFVehicleCatalogPromoResult FCFVehicleCatalogPromoService::PromoteVehicleToRuntimeCatalog(
	UCFVehicleData* TargetVehicleData)
{
	// Promotion target validation 오류입니다.
	FString TargetValidationError;
	if (!ValidatePromotionTarget(TargetVehicleData, TargetValidationError))
	{
		FCFVehicleCatalogPromoResult Result;
		Result.Outcome = ECFVehicleCatalogPromoOutcome::InvalidTarget;
		Result.Message = TargetValidationError;
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, nullptr);
		return Result;
	}

	// Raw Settings resolve 결과 Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = nullptr;
	// Raw resolve failure 결과입니다.
	FCFVehicleCatalogPromoResult ResolveFailure;
	if (!ResolveDefaultRuntimeCatalogRaw(RuntimeCatalog, ResolveFailure))
	{
		ResolveFailure.TargetVehicleData = TargetVehicleData;
		return ResolveFailure;
	}

	return PromoteVehicleToCatalog(TargetVehicleData, RuntimeCatalog);
}

// 이미 resolve된 Catalog에 exact persistent VehicleData를 idempotent promotion합니다.
FCFVehicleCatalogPromoResult FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(
	UCFVehicleData* TargetVehicleData,
	UCFRuntimeTestCatalogData* RuntimeCatalog)
{
	// Promotion terminal result입니다.
	FCFVehicleCatalogPromoResult Result;
	Result.bSavePerformed = false;

	// Target validation 오류입니다.
	FString TargetValidationError;
	if (!ValidatePromotionTarget(TargetVehicleData, TargetValidationError))
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::InvalidTarget;
		Result.Message = TargetValidationError;
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}
	if (!IsValid(RuntimeCatalog))
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::CatalogUnavailable;
		Result.Message = TEXT("Runtime Test Catalog를 사용할 수 없습니다.");
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	// Mutation 전 full Catalog validation 오류입니다.
	FString CatalogValidationError;
	if (!CFVehicleCatalogPromoPrivate::ValidateCatalog(*RuntimeCatalog, CatalogValidationError))
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::CatalogInvalid;
		Result.Message = FString::Printf(TEXT("Runtime Test Catalog pre-validation 실패: %s"), *CatalogValidationError);
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	// Mutation 전 exact pointer identity membership count입니다.
	const int32 ExistingMembershipCount = CFVehicleCatalogPromoPrivate::CountExactMembership(*RuntimeCatalog, TargetVehicleData);
	if (ExistingMembershipCount == 1)
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::AlreadyRegistered;
		Result.Message = TEXT("VehicleData가 Runtime Test Catalog에 이미 exact 1회 등록되어 있습니다.");
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}
	if (ExistingMembershipCount != 0)
	{
		Result.Outcome = ECFVehicleCatalogPromoOutcome::CatalogInvalid;
		Result.Message = TEXT("Runtime Test Catalog에 exact VehicleData identity가 중복되어 있습니다.");
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	// Internal failure rollback에서 exact 복원할 mutation 전 Vehicle allowlist입니다.
	const TArray<TObjectPtr<UCFVehicleData>> PreviousAllowedVehicleData = RuntimeCatalog->AllowedVehicleData;
	// Catalog outer package입니다.
	UPackage* RuntimeCatalogPackage = RuntimeCatalog->GetOutermost();
	// Internal rollback에서 exact 복원할 mutation 전 package dirty 상태입니다.
	const bool bCatalogPackageWasDirty = RuntimeCatalogPackage && RuntimeCatalogPackage->IsDirty();

	FScopedTransaction Transaction(
		NSLOCTEXT("CarFightDataAuthoring", "PromoteVehicleToRuntimeCatalog", "차량 Runtime Catalog 등록"));
	RuntimeCatalog->Modify();
	RuntimeCatalog->AllowedVehicleData.Add(TargetVehicleData);
	RuntimeCatalog->MarkPackageDirty();
	RuntimeCatalog->PostEditChange();

	// Mutation 뒤 full Catalog validation 오류입니다.
	FString PostValidationError;
	if (!CFVehicleCatalogPromoPrivate::ValidateCatalog(*RuntimeCatalog, PostValidationError))
	{
		CFVehicleCatalogPromoPrivate::RestoreCatalogAfterInternalFailure(
			*RuntimeCatalog,
			PreviousAllowedVehicleData,
			bCatalogPackageWasDirty,
			Transaction);
		Result.Outcome = ECFVehicleCatalogPromoOutcome::MutationFailed;
		Result.Message = FString::Printf(
			TEXT("Runtime Test Catalog post-validation 실패로 rollback했습니다: %s"),
			*PostValidationError);
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	// Mutation 뒤 exact pointer identity membership count입니다.
	const int32 ReadbackMembershipCount = CFVehicleCatalogPromoPrivate::CountExactMembership(*RuntimeCatalog, TargetVehicleData);
	if (ReadbackMembershipCount != 1
		|| RuntimeCatalog->AllowedVehicleData.Num() != PreviousAllowedVehicleData.Num() + 1)
	{
		CFVehicleCatalogPromoPrivate::RestoreCatalogAfterInternalFailure(
			*RuntimeCatalog,
			PreviousAllowedVehicleData,
			bCatalogPackageWasDirty,
			Transaction);
		Result.Outcome = ECFVehicleCatalogPromoOutcome::MutationFailed;
		Result.Message = TEXT("Runtime Test Catalog exact membership/readback이 요청 결과와 달라 rollback했습니다.");
		CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
		return Result;
	}

	Result.Outcome = ECFVehicleCatalogPromoOutcome::Registered;
	Result.Message = TEXT("VehicleData를 Runtime Test Catalog에 등록했습니다. Catalog는 dirty이며 자동 저장하지 않았습니다.");
	CFVehicleCatalogPromoPrivate::PopulateCommonResultState(Result, TargetVehicleData, RuntimeCatalog);
	return Result;
}

// Fresh result를 USER-facing 한 줄 상태 문자열로 변환합니다.
FString FCFVehicleCatalogPromoService::BuildPromotionStatus(const FCFVehicleCatalogPromoResult& Result)
{
	// Result outcome의 stable label입니다.
	const TCHAR* OutcomeLabel = TEXT("MutationFailed");
	switch (Result.Outcome)
	{
	case ECFVehicleCatalogPromoOutcome::Registered:
		OutcomeLabel = TEXT("Registered");
		break;
	case ECFVehicleCatalogPromoOutcome::AlreadyRegistered:
		OutcomeLabel = TEXT("AlreadyRegistered");
		break;
	case ECFVehicleCatalogPromoOutcome::InvalidTarget:
		OutcomeLabel = TEXT("InvalidTarget");
		break;
	case ECFVehicleCatalogPromoOutcome::CatalogUnavailable:
		OutcomeLabel = TEXT("CatalogUnavailable");
		break;
	case ECFVehicleCatalogPromoOutcome::CatalogInvalid:
		OutcomeLabel = TEXT("CatalogInvalid");
		break;
	case ECFVehicleCatalogPromoOutcome::MutationFailed:
	default:
		break;
	}

	return FString::Printf(
		TEXT("CatalogPromotion=%s | Member=%s | Dirty=%s | AutoSave=%s | %s"),
		OutcomeLabel,
		Result.bIsMember ? TEXT("Yes") : TEXT("No"),
		Result.bCatalogPackageDirty ? TEXT("Yes") : TEXT("No"),
		Result.bSavePerformed ? TEXT("Yes") : TEXT("No"),
		*Result.Message);
}

#undef LOCTEXT_NAMESPACE
