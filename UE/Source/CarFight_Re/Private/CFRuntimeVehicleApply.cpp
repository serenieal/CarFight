// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-02
// Description: CF-FQ-041 RTA-P0-02 same-Pawn Vehicle Runtime Apply 구현
// Scope: transient VehicleData 적용, previous VehicleData/Fitting checkpoint, 실패 복구와 final runtime readback을 소유합니다.
// Changelog:
// - v1.0.0: Builder Step 8의 transient duplicate + InitializeVehicleRuntime 경로를 Runtime 서비스로 구현.
// Migration:
// - Vehicle Builder Source와 CFVehiclePawn Source를 수정하지 않습니다.
// - 실패 복구는 이전 VehicleData/Fitting 포인터를 복원한 뒤 동일 InitializeVehicleRuntime()을 다시 호출합니다.

#include "CFRuntimeVehicleApply.h"

#include "CFRuntimeTestCatalogData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	// [v1.0.0] Vehicle Apply mutation 동안 GC로부터 이전 VehicleData/Fitting을 보존하는 내부 checkpoint입니다.
	struct FCFRuntimeVehicleCheckpoint
	{
		// 이전 VehicleData를 operation 종료까지 강하게 보존합니다.
		TStrongObjectPtr<UCFVehicleData> PreviousVehicleData;

		// 이전 VehicleFittingData를 operation 종료까지 강하게 보존합니다.
		TStrongObjectPtr<UCFVehicleFittingData> PreviousVehicleFittingData;
	};

	// [v1.0.0] 현재 Pawn의 VehicleData/Fitting 경로와 RuntimeDebug 상태를 결과에 다시 읽습니다.
	void CaptureVehicleRuntimeReadback(
		ACFVehiclePawn* VehiclePawn,
		const UCFVehicleData* ExpectedTransientVehicleData,
		FCFRuntimeVehicleApplyResult& InOutResult)
	{
		if (!IsValid(VehiclePawn))
		{
			InOutResult.CurrentVehicleDataPath = TEXT("None");
			InOutResult.CurrentVehicleFittingDataPath = TEXT("None");
			InOutResult.bRuntimeReady = false;
			InOutResult.bAppliedTransientCopyActive = false;
			InOutResult.RuntimeSummary = TEXT("VehicleRuntimeReadback: PawnInvalid");
			return;
		}

		// 현재 Pawn.VehicleData의 실제 runtime object입니다.
		UCFVehicleData* CurrentVehicleData = VehiclePawn->VehicleData.Get();

		// 현재 Pawn.VehicleFittingData의 실제 runtime object입니다.
		UCFVehicleFittingData* CurrentVehicleFittingData = VehiclePawn->VehicleFittingData.Get();

		// 공개 VehicleDebug API에서 다시 읽은 current Runtime 상태입니다.
		const FCFVehicleDebugRuntime RuntimeReadback = VehiclePawn->GetVehicleDebugRuntime();

		InOutResult.CurrentVehicleDataPath = GetPathNameSafe(CurrentVehicleData);
		InOutResult.CurrentVehicleFittingDataPath = GetPathNameSafe(CurrentVehicleFittingData);
		InOutResult.bRuntimeReady = RuntimeReadback.bRuntimeReady;
		InOutResult.RuntimeSummary = RuntimeReadback.RuntimeSummary;
		InOutResult.bAppliedTransientCopyActive = ExpectedTransientVehicleData
			&& CurrentVehicleData == ExpectedTransientVehicleData
			&& CurrentVehicleData->HasAnyFlags(RF_Transient)
			&& CurrentVehicleData->GetOutermost() == GetTransientPackage();
	}

	// [v1.0.0] source authorization 이후 실제 same-Pawn mutation/recovery를 수행합니다.
	FCFRuntimeVehicleApplyResult ApplyValidatedVehicleRuntime(
		ACFVehiclePawn* VehiclePawn,
		UCFVehicleData* CandidateVehicleData)
	{
		// 호출자에게 반환할 bounded operation 결과입니다.
		FCFRuntimeVehicleApplyResult Result;

		if (IsValid(CandidateVehicleData))
		{
			Result.RequestedVehicleDataPath = FSoftObjectPath(CandidateVehicleData);
		}

		if (!IsValid(VehiclePawn))
		{
			Result.Status = ECFRuntimeVehicleApplyStatus::ValidationFailed;
			Result.Message = TEXT("Vehicle Runtime Apply 대상 CFVehiclePawn이 유효하지 않습니다.");
			CaptureVehicleRuntimeReadback(nullptr, nullptr, Result);
			return Result;
		}

		if (!IsValid(CandidateVehicleData))
		{
			Result.Status = ECFRuntimeVehicleApplyStatus::ValidationFailed;
			Result.Message = TEXT("Vehicle Runtime Apply 후보 VehicleData가 유효하지 않습니다.");
			CaptureVehicleRuntimeReadback(VehiclePawn, nullptr, Result);
			return Result;
		}

		// mutation 직전 이전 VehicleData를 GC로부터 보존하는 checkpoint입니다.
		TStrongObjectPtr<UCFVehicleData> PreviousVehicleData(VehiclePawn->VehicleData.Get());

		// mutation 직전 이전 VehicleFittingData를 GC로부터 보존하는 checkpoint입니다.
		TStrongObjectPtr<UCFVehicleFittingData> PreviousVehicleFittingData(VehiclePawn->VehicleFittingData.Get());

		// 이전 두 runtime source pointer를 하나의 operation checkpoint로 묶습니다.
		FCFRuntimeVehicleCheckpoint RuntimeCheckpoint{
			MoveTemp(PreviousVehicleData),
			MoveTemp(PreviousVehicleFittingData)
		};

		Result.PreviousVehicleDataPath = GetPathNameSafe(RuntimeCheckpoint.PreviousVehicleData.Get());
		Result.PreviousVehicleFittingDataPath = GetPathNameSafe(RuntimeCheckpoint.PreviousVehicleFittingData.Get());

		// 원본 persistent Asset을 runtime mutation에서 격리할 unique transient object 이름입니다.
		const FName TransientVehicleDataName = MakeUniqueObjectName(
			GetTransientPackage(),
			UCFVehicleData::StaticClass(),
			TEXT("RTA_VehicleData"));

		// Builder Step 8과 동일한 value-equivalent transient VehicleData 복사본입니다.
		UCFVehicleData* TransientVehicleData = DuplicateObject<UCFVehicleData>(
			CandidateVehicleData,
			GetTransientPackage(),
			TransientVehicleDataName);

		if (!IsValid(TransientVehicleData))
		{
			Result.Status = ECFRuntimeVehicleApplyStatus::ApplyFailed;
			Result.Message = TEXT("VehicleData transient duplicate 생성에 실패했습니다. 기존 Pawn Runtime은 변경하지 않았습니다.");
			CaptureVehicleRuntimeReadback(VehiclePawn, nullptr, Result);
			return Result;
		}

		TransientVehicleData->ClearFlags(RF_Public | RF_Standalone);
		TransientVehicleData->SetFlags(RF_Transient);

		// Builder Step 8 Vehicle Apply와 동일하게 별도 Fitting override 없이 후보 VehicleData 기본 경로를 사용합니다.
		VehiclePawn->VehicleFittingData = nullptr;
		VehiclePawn->VehicleData = TransientVehicleData;

		// 후보 VehicleData를 현재 same Pawn 전체 runtime에 다시 적용한 결과입니다.
		const bool bCandidateRuntimeInitialized = VehiclePawn->InitializeVehicleRuntime();

		CaptureVehicleRuntimeReadback(VehiclePawn, TransientVehicleData, Result);

		// Initialize 반환과 실제 Pawn readback이 모두 candidate transient runtime을 가리키는지 확인합니다.
		const bool bCandidateReadbackMatches = bCandidateRuntimeInitialized
			&& Result.bRuntimeReady
			&& Result.bAppliedTransientCopyActive
			&& VehiclePawn->VehicleFittingData == nullptr;

		if (bCandidateReadbackMatches)
		{
			Result.Status = ECFRuntimeVehicleApplyStatus::Succeeded;
			Result.Message = FString::Printf(
				TEXT("Vehicle Runtime Apply 성공: %s"),
				*Result.RequestedVehicleDataPath.ToString());
			return Result;
		}

		Result.bRecoveryAttempted = true;

		// 후보 적용 실패 전 checkpoint의 VehicleData pointer를 복원합니다.
		VehiclePawn->VehicleData = RuntimeCheckpoint.PreviousVehicleData.Get();

		// 후보 적용 실패 전 checkpoint의 VehicleFittingData pointer를 복원합니다.
		VehiclePawn->VehicleFittingData = RuntimeCheckpoint.PreviousVehicleFittingData.Get();

		// 부분 적용된 runtime component 상태까지 이전 VehicleData/Fitting 기준으로 다시 구성하는 복구 결과입니다.
		const bool bRecoveryRuntimeInitialized = VehiclePawn->InitializeVehicleRuntime();

		CaptureVehicleRuntimeReadback(VehiclePawn, nullptr, Result);

		// 복구 후 포인터 identity와 RuntimeReady를 모두 확인한 결과입니다.
		const bool bRecoveryReadbackMatches = bRecoveryRuntimeInitialized
			&& Result.bRuntimeReady
			&& VehiclePawn->VehicleData.Get() == RuntimeCheckpoint.PreviousVehicleData.Get()
			&& VehiclePawn->VehicleFittingData.Get() == RuntimeCheckpoint.PreviousVehicleFittingData.Get();

		Result.bRecoverySucceeded = bRecoveryReadbackMatches;
		if (bRecoveryReadbackMatches)
		{
			Result.Status = ECFRuntimeVehicleApplyStatus::ApplyFailed;
			Result.Message = FString::Printf(
				TEXT("Vehicle Runtime Apply는 실패했지만 이전 VehicleData/Fitting Runtime을 복구했습니다. Candidate=%s"),
				*Result.RequestedVehicleDataPath.ToString());
			return Result;
		}

		Result.Status = ECFRuntimeVehicleApplyStatus::RecoveryFailed;
		Result.Message = FString::Printf(
			TEXT("Vehicle Runtime Apply 실패 후 이전 VehicleData/Fitting 포인터는 복원했지만 Runtime 재초기화 복구에도 실패했습니다. Candidate=%s"),
			*Result.RequestedVehicleDataPath.ToString());
		return Result;
	}
}

// [v1.0.0] Candidate가 현재 Runtime Test Catalog에 명시 등록된 VehicleData인지 fail-closed로 검증합니다.
bool FCFRuntimeVehicleApplyService::ValidateCatalogVehicleCandidate(
	const UCFRuntimeTestCatalogData* RuntimeCatalog,
	const UCFVehicleData* CandidateVehicleData,
	FString& OutError)
{
	OutError.Reset();

	if (!IsValid(RuntimeCatalog))
	{
		OutError = TEXT("Runtime Test Catalog가 유효하지 않습니다.");
		return false;
	}

	// Catalog 전체 hard-reference 계약의 상세 오류 목록입니다.
	TArray<FText> CatalogValidationErrors;
	if (!RuntimeCatalog->ValidateRuntimeTestCatalog(CatalogValidationErrors))
	{
		OutError = FString::Printf(
			TEXT("Runtime Test Catalog 계약이 유효하지 않습니다. Issues=%d"),
			CatalogValidationErrors.Num());
		return false;
	}

	if (!IsValid(CandidateVehicleData))
	{
		OutError = TEXT("선택한 VehicleData가 유효하지 않습니다.");
		return false;
	}

	// exact UObject identity로 Candidate가 명시 등록 목록에 포함됐는지 확인합니다.
	const bool bCandidateRegistered = RuntimeCatalog->AllowedVehicleData.ContainsByPredicate(
		[CandidateVehicleData](const TObjectPtr<UCFVehicleData>& RegisteredVehicleData)
		{
			return RegisteredVehicleData.Get() == CandidateVehicleData;
		});

	if (!bCandidateRegistered)
	{
		OutError = FString::Printf(
			TEXT("선택한 VehicleData가 Runtime Test Catalog 허용 목록에 없습니다: %s"),
			*GetPathNameSafe(CandidateVehicleData));
		return false;
	}

	return true;
}

// [v1.0.0] Catalog authorization을 통과한 VehicleData만 same-Pawn Runtime operation으로 전달합니다.
FCFRuntimeVehicleApplyResult FCFRuntimeVehicleApplyService::ApplyCatalogVehicle(
	ACFVehiclePawn* VehiclePawn,
	const UCFRuntimeTestCatalogData* RuntimeCatalog,
	UCFVehicleData* CandidateVehicleData)
{
	// Catalog authorization 실패 원인을 보존할 문자열입니다.
	FString ValidationError;
	if (!ValidateCatalogVehicleCandidate(RuntimeCatalog, CandidateVehicleData, ValidationError))
	{
		// mutation 없이 반환할 검증 실패 결과입니다.
		FCFRuntimeVehicleApplyResult Result;
		Result.Status = ECFRuntimeVehicleApplyStatus::ValidationFailed;
		Result.Message = ValidationError;
		if (IsValid(CandidateVehicleData))
		{
			Result.RequestedVehicleDataPath = FSoftObjectPath(CandidateVehicleData);
		}
		CaptureVehicleRuntimeReadback(VehiclePawn, nullptr, Result);
		return Result;
	}

	return ApplyValidatedVehicleRuntime(VehiclePawn, CandidateVehicleData);
}

// [v1.0.0] 호출자가 source authorization을 완료한 VehicleData를 transient duplicate + InitializeVehicleRuntime 경로로 적용합니다.
FCFRuntimeVehicleApplyResult FCFRuntimeVehicleApplyService::ApplyVehicleRuntime(
	ACFVehiclePawn* VehiclePawn,
	UCFVehicleData* CandidateVehicleData)
{
	return ApplyValidatedVehicleRuntime(VehiclePawn, CandidateVehicleData);
}
