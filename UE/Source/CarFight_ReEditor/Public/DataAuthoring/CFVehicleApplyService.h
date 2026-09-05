// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleApplyService.h
// Version: v1.1.0
// Date: 2026-09-04
// Description: DAUTH Apply Transaction Foundation의 Target writer + Recipe AppliedState finalize 계약입니다.
// Scope: Frozen TOCTOU precondition, reviewed Resolve payload, atomic Apply와 no-diff AppliedState repair를 제공합니다.
// Changelog:
// - v1.1.0: VBHAI-P0-07H fresh-restart partial persistence 복구를 위해 Target mutation 없이 existing BuildAppliedState authority로 Recipe AppliedState만 finalize하는 entry point를 추가.
// - v1.0.0: FCFVehicleApplyRequest/Result, stable failure taxonomy, FCFVehicleApplyService entry point 최초 구현.
// Migration:
// - Target UCFVehicleData write는 이 service만 수행합니다. FinalizeAppliedState는 Target을 수정하지 않습니다.
// - Apply/FinalizeAppliedState 모두 Package Dirty까지만 수행하며 Save/SavePackage를 호출하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"
#include "CFVehicleApplyService.generated.h"

class UCFVehicleData;
class UCFVehicleRecipeData;
class FCFVehicleApplyTestAccess;

/** Apply 요청의 최종 처리 상태입니다. */
UENUM()
enum class ECFVehicleApplyStatus : uint8
{
	Success,
	Blocked,
	Error
};

/** Apply가 mutation 전 또는 transaction 중 중단된 stable 이유입니다. */
UENUM()
enum class ECFVehicleApplyFailureCode : uint8
{
	None,
	InvalidRequest,
	PreviewOutOfDate,
	ResolveNotSuccessful,
	ReviewedDiffMismatch,
	PreflightApplyFailed,
	PreflightValidationFailed,
	PreflightHashMismatch,
	TargetApplyFailed,
	TargetReadbackMismatch,
	TargetValidationFailed,
	RollbackFailed,
	InternalError
};

/** Reviewed Preview와 fresh immutable resolve context를 Target write lane에 전달하는 Apply 요청입니다. */
USTRUCT()
struct FCFVehicleApplyRequest
{
	GENERATED_BODY()

	// AppliedState를 갱신할 persistent Editor-only Recipe입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleRecipeData> Recipe = nullptr;

	// 승인된 Diff를 적용할 Runtime Canonical VehicleData target입니다.
	UPROPERTY()
	TObjectPtr<UCFVehicleData> TargetVehicleData = nullptr;

	// Profile/Asset snapshots를 포함하는 reviewed Resolver request context입니다.
	UPROPERTY()
	FCFVehicleResolveRequest ResolveRequest;

	// 사용자가 검토한 Source Trace/Diff/Validation을 포함하는 approved Resolver result입니다.
	UPROPERTY()
	FCFVehicleResolveResult ApprovedResolveResult;

	// Preview 승인 시점의 Recipe semantic fingerprint입니다.
	UPROPERTY()
	FString ExpectedRecipeFingerprint;

	// Preview 승인 시점의 effective Source set signature입니다.
	UPROPERTY()
	FString ExpectedSourceSignature;

	// Preview 승인 시점 Current Target의 full Definition Snapshot hash입니다.
	UPROPERTY()
	FString ExpectedTargetDefinitionHash;

	// Preview가 적용 후 기대하는 Resolver-owned Definition projection hash입니다.
	UPROPERTY()
	FString ExpectedResolvedDefinitionHash;

	// Preview가 사용한 Frozen Resolver semantic contract revision입니다.
	UPROPERTY()
	int32 ExpectedResolverContractRevision = 0;
};

/** Apply의 mutation/validation/rollback 결과를 value-copy로 반환합니다. */
USTRUCT()
struct FCFVehicleApplyResult
{
	GENERATED_BODY()

	// Apply가 Success/Blocked/Error 중 어디에서 끝났는지 나타냅니다.
	UPROPERTY()
	ECFVehicleApplyStatus Status = ECFVehicleApplyStatus::Error;

	// 자동화/AI가 문자열 해석 없이 분류할 stable failure code입니다.
	UPROPERTY()
	ECFVehicleApplyFailureCode FailureCode = ECFVehicleApplyFailureCode::InternalError;

	// 사람이 읽는 한국어 처리 결과입니다.
	UPROPERTY()
	FString Message;

	// Preflight 또는 actual Target Validator에서 수집한 Definition issues입니다.
	UPROPERTY()
	TArray<FCFVehicleValidationIssue> DefinitionValidation;

	// 실제 transaction에 사용한 dependency-safe diff operation 수입니다.
	UPROPERTY()
	int32 AppliedDiffOperationCount = 0;

	// 성공한 Target readback의 Resolver-owned Definition hash입니다.
	UPROPERTY()
	FString AppliedDefinitionHash;

	// Target mutation이 정상 transaction으로 commit됐는지 여부입니다.
	UPROPERTY()
	bool bTargetMutationCommitted = false;

	// Recipe AppliedState가 정상 transaction으로 갱신됐는지 여부입니다.
	UPROPERTY()
	bool bRecipeAppliedStateUpdated = false;

	// 실패 transaction에서 Target + AppliedState rollback 검증까지 성공했는지 여부입니다.
	UPROPERTY()
	bool bRollbackVerified = false;
};

/** Frozen Apply lane을 독점하는 Editor-only Vehicle Definition writer입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleApplyService
{
public:
	// Reviewed Preview를 TOCTOU 재검증한 뒤 Target + Recipe AppliedState를 atomic transaction으로 적용합니다.
	static bool Apply(const FCFVehicleApplyRequest& Request, FCFVehicleApplyResult& OutResult);

	// Fresh Resolve와 이미 일치하는 Target을 재검증한 뒤 Target mutation 없이 Recipe AppliedState만 authoritative state로 finalize합니다.
	static bool FinalizeAppliedState(const FCFVehicleApplyRequest& Request, FCFVehicleApplyResult& OutResult);

private:
	// Production Apply와 Automation rollback probe가 공유하는 실제 transaction implementation입니다.
	static bool ApplyInternal(
		const FCFVehicleApplyRequest& Request,
		FCFVehicleApplyResult& OutResult,
		bool bInjectFailureAfterTargetMutation);

	friend class FCFVehicleApplyTestAccess;
};
