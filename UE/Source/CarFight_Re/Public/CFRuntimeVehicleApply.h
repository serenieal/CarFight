// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-09-02
// Description: CF-FQ-041 RTA-P0-02 same-Pawn Vehicle Runtime Apply 서비스 계약
// Scope: 승인된 VehicleData를 transient copy로 현재 VehiclePawn에 적용하고 실패 시 이전 VehicleData/Fitting을 재초기화 복구합니다.
// Changelog:
// - v1.0.1: transient duplicate 단계 실패도 ApplyFailed에 포함될 수 있으므로 표시명을 복구 성공으로 오해하지 않게 일반화.
// - v1.0.0: Catalog candidate validation, VehicleData/Fitting checkpoint, ApplyFailed/RecoveryFailed 분리, runtime readback을 추가.
// Migration:
// - Builder Step 8과 동일하게 Pawn replacement 없이 VehicleData transient copy + InitializeVehicleRuntime()을 사용합니다.
// - Catalog는 selection authorization만 담당하며 실제 Runtime operation은 Catalog와 분리된 ApplyVehicleRuntime()에 유지합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFRuntimeVehicleApply.generated.h"

class ACFVehiclePawn;
class UCFRuntimeTestCatalogData;
class UCFVehicleData;

/**
 * 차량 Runtime 적용 작업의 최종 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFRuntimeVehicleApplyStatus : uint8
{
	NotAttempted UMETA(DisplayName="시도 안 함 (Not Attempted)"),
	Succeeded UMETA(DisplayName="성공 (Succeeded)"),
	ValidationFailed UMETA(DisplayName="검증 실패 (Validation Failed)"),
	ApplyFailed UMETA(DisplayName="적용 실패 (Apply Failed)"),
	RecoveryFailed UMETA(DisplayName="복구 실패 (Recovery Failed)")
};

/**
 * Vehicle Runtime Apply가 UI/로그에 반환하는 bounded readback 결과입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFRuntimeVehicleApplyResult
{
	GENERATED_BODY()

	// [v1.0.0] 이번 Runtime Apply의 최종 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="차량 Runtime 적용 상태 (Status)", ToolTip="성공, 검증 실패, 적용 실패, 복구 실패를 구분합니다. 복구 시도/성공 여부는 별도 필드에서 확인합니다."))
	ECFRuntimeVehicleApplyStatus Status = ECFRuntimeVehicleApplyStatus::NotAttempted;

	// [v1.0.0] 호출자가 적용을 요청한 persistent VehicleData Asset 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="요청 차량 데이터 경로 (RequestedVehicleDataPath)", ToolTip="Runtime에 적용하도록 요청한 원본 VehicleData Asset의 경로입니다."))
	FSoftObjectPath RequestedVehicleDataPath;

	// [v1.0.0] mutation 직전에 checkpoint한 VehicleData object 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="이전 차량 데이터 경로 (PreviousVehicleDataPath)", ToolTip="적용 직전에 Pawn이 사용하던 VehicleData의 object 경로입니다. 실패 복구 대상 확인에 사용합니다."))
	FString PreviousVehicleDataPath = TEXT("None");

	// [v1.0.0] mutation 직전에 checkpoint한 VehicleFittingData object 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="이전 피팅 데이터 경로 (PreviousVehicleFittingDataPath)", ToolTip="적용 직전에 Pawn이 사용하던 VehicleFittingData의 object 경로입니다. 비어 있으면 None입니다."))
	FString PreviousVehicleFittingDataPath = TEXT("None");

	// [v1.0.0] 작업 종료 뒤 Pawn에서 다시 읽은 current VehicleData object 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="현재 차량 데이터 경로 (CurrentVehicleDataPath)", ToolTip="성공 또는 복구 종료 뒤 실제 Pawn.VehicleData에서 다시 읽은 object 경로입니다."))
	FString CurrentVehicleDataPath = TEXT("None");

	// [v1.0.0] 작업 종료 뒤 Pawn에서 다시 읽은 current VehicleFittingData object 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="현재 피팅 데이터 경로 (CurrentVehicleFittingDataPath)", ToolTip="성공 또는 복구 종료 뒤 실제 Pawn.VehicleFittingData에서 다시 읽은 object 경로입니다."))
	FString CurrentVehicleFittingDataPath = TEXT("None");

	// [v1.0.0] 최종 Pawn Runtime readback의 Core Runtime 준비 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="최종 Runtime 준비 여부 (bRuntimeReady)", ToolTip="작업 종료 뒤 VehiclePawn의 VehicleDebug Runtime readback에서 읽은 준비 완료 여부입니다."))
	bool bRuntimeReady = false;

	// [v1.0.0] 후보 적용 실패 뒤 이전 checkpoint 복구를 실제 시도했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="복구 시도 여부 (bRecoveryAttempted)", ToolTip="후보 VehicleData 적용 실패 뒤 이전 VehicleData/Fitting 재초기화를 시도했는지 표시합니다."))
	bool bRecoveryAttempted = false;

	// [v1.0.0] 이전 VehicleData/Fitting 재초기화 복구가 readback까지 성공했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="복구 성공 여부 (bRecoverySucceeded)", ToolTip="이전 VehicleData/Fitting 포인터 복원과 InitializeVehicleRuntime 재초기화 및 runtime readback이 모두 성공했는지 표시합니다."))
	bool bRecoverySucceeded = false;

	// [v1.0.0] 성공 결과가 원본 Asset이 아니라 transient VehicleData copy를 실제 current 값으로 유지하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="Transient 차량 복사본 활성 여부 (bAppliedTransientCopyActive)", ToolTip="성공 시 Pawn.VehicleData가 요청 원본 Asset이 아니라 transient duplicate인지 readback으로 확인한 결과입니다."))
	bool bAppliedTransientCopyActive = false;

	// [v1.0.0] 최종 VehicleDebug Runtime readback 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="Runtime 요약 (RuntimeSummary)", ToolTip="작업 종료 뒤 VehiclePawn의 VehicleDebug Runtime에서 읽은 마지막 초기화 요약입니다."))
	FString RuntimeSummary = TEXT("NotRead");

	// [v1.0.0] UI와 로그가 상태를 설명할 수 있는 bounded 결과 메시지입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Vehicle", meta=(DisplayName="결과 메시지 (Message)", ToolTip="검증, 적용, 복구 결과를 사용자에게 설명하는 한 줄 메시지입니다."))
	FString Message;
};

/**
 * VehicleData source authorization과 same-Pawn Runtime 적용을 분리하는 정적 서비스입니다.
 */
struct CARFIGHT_RE_API FCFRuntimeVehicleApplyService
{
	// [v1.0.0] Candidate가 현재 Runtime Test Catalog에 명시 등록된 VehicleData인지 fail-closed로 검증합니다.
	static bool ValidateCatalogVehicleCandidate(
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		const UCFVehicleData* CandidateVehicleData,
		FString& OutError);

	// [v1.0.0] Catalog authorization을 통과한 VehicleData만 same-Pawn Runtime operation으로 전달합니다.
	static FCFRuntimeVehicleApplyResult ApplyCatalogVehicle(
		ACFVehiclePawn* VehiclePawn,
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		UCFVehicleData* CandidateVehicleData);

	// [v1.0.0] 호출자가 source authorization을 완료한 VehicleData를 transient duplicate + InitializeVehicleRuntime 경로로 적용합니다.
	static FCFRuntimeVehicleApplyResult ApplyVehicleRuntime(
		ACFVehiclePawn* VehiclePawn,
		UCFVehicleData* CandidateVehicleData);
};
