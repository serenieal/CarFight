// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-02
// Description: CF-FQ-041 RTA-P0-03 장비 슬롯 Runtime Apply 서비스 계약
// Scope: Runtime Test Catalog 장비 허용 검증, transient Fitting 후보 생성, 단일 Mount 교체, Runtime 적용·복구 결과와 readback을 제공합니다.
// Changelog:
// - v1.0.0: RTA-P0-03 Equipment Slot Runtime Apply의 Succeeded/ValidationFailed/ApplyFailed/RecoveryFailed 결과 계약과 Catalog/직접 적용 API를 추가.
// Migration:
// - Debug/Demo Runtime Apply는 Inventory를 우회하지만 UCFVehicleFittingData::BuildFittingSnapshot과 기존 Fitting Runtime Commit 계약을 그대로 사용합니다.
// - 기존 FittingData와 같은 VehicleData면 MountSelections, InitialSortieAmmoLoads, DefenseSelection을 transient 후보에 보존합니다.
// - finite 무기에 필요한 출격 탄약이 없으면 BuildFittingSnapshot 검증 실패를 그대로 반환하며 탄약 수량을 자동 생성하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFRuntimeEquipApply.generated.h"

class ACFVehiclePawn;
class UCFEquipmentPresetData;
class UCFRuntimeTestCatalogData;

/** RTA-P0-03 단일 장비 슬롯 Runtime Apply의 최종 상태입니다. */
UENUM(BlueprintType)
enum class ECFRuntimeEquipApplyStatus : uint8
{
	NotAttempted UMETA(DisplayName="시도 전 (Not Attempted)"),
	Succeeded UMETA(DisplayName="적용 성공 (Succeeded)"),
	ValidationFailed UMETA(DisplayName="검증 실패 (Validation Failed)"),
	ApplyFailed UMETA(DisplayName="적용 실패 (Apply Failed)"),
	RecoveryFailed UMETA(DisplayName="복구 실패 (Recovery Failed)")
};

/** RTA-P0-03 단일 장비 슬롯 Runtime Apply 결과와 최종 readback입니다. */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFRuntimeEquipApplyResult
{
	GENERATED_BODY()

	// [v1.0.0] 이번 장비 적용의 최종 분류 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	ECFRuntimeEquipApplyStatus Status = ECFRuntimeEquipApplyStatus::NotAttempted;

	// [v1.0.0] 교체를 요청한 VehicleData MountProfileId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FName RequestedMountProfileId = NAME_None;

	// [v1.0.0] 요청한 EquipmentPresetData의 UObject 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString RequestedEquipmentPath;

	// [v1.0.0] 적용 전 Pawn이 보유한 VehicleFittingData 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString PreviousVehicleFittingDataPath;

	// [v1.0.0] 작업 종료 시 Pawn이 보유한 VehicleFittingData 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString CurrentVehicleFittingDataPath;

	// [v1.0.0] 작업 종료 시 Applied Snapshot에서 해당 Mount가 실제 사용 중인 EquipmentPresetData 경로입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString CurrentEquipmentPath;

	// [v1.0.0] 작업 종료 시 차량 Combat Runtime이 준비 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	bool bRuntimeReady = false;

	// [v1.0.0] 후보 적용 실패 뒤 직전 Fitting/Mass 상태 복구를 시도했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	bool bRecoveryAttempted = false;

	// [v1.0.0] 복구가 필요했던 경우 Fitting, Mass와 Fitting 의존 Runtime이 모두 직전 상태로 돌아왔는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	bool bRecoverySucceeded = false;

	// [v1.0.0] 성공 결과에서 Pawn이 저장 Asset이 아닌 transient Fitting 사본을 현재 소유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	bool bAppliedTransientFittingActive = false;

	// [v1.0.0] 후보 총질량이 현재 Chaos 설정 질량과 달라 hot reapply가 필요했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	bool bMassReapplyRequired = false;

	// [v1.0.0] 적용 전 Chaos VehicleMovement의 설정 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	float PreviousConfiguredMassKg = 0.0f;

	// [v1.0.0] 후보 Fitting Snapshot이 계산한 차량 총질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	float CandidateTotalMassKg = 0.0f;

	// [v1.0.0] 작업 종료 시 Chaos VehicleMovement의 설정 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	float CurrentConfiguredMassKg = 0.0f;

	// [v1.0.0] 최종 Fitting Runtime이 기록한 bounded 진단 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString FittingRuntimeSummary;

	// [v1.0.0] 최종 차량 Runtime readback 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString RuntimeSummary;

	// [v1.0.0] UI와 테스트가 표시할 적용 또는 실패 원인 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeApply|Equipment")
	FString Message;

	// [v1.0.0] 완전 성공 결과인지 반환합니다.
	bool IsSuccessful() const { return Status == ECFRuntimeEquipApplyStatus::Succeeded; }
};

/** RTA-P0-03 Debug/Demo 장비 슬롯 Runtime Apply를 기존 Fitting authority에 연결하는 C++ 서비스입니다. */
struct CARFIGHT_RE_API FCFRuntimeEquipApplyService
{
	// [v1.0.0] Runtime Test Catalog 전체 계약과 exact EquipmentPresetData membership을 mutation 없이 검증합니다.
	static bool ValidateCatalogEquipmentCandidate(
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		const UCFEquipmentPresetData* CandidateEquipmentPresetData,
		FString& OutFailureReason);

	// [v1.0.0] Catalog가 허용한 장비만 단일 MountProfile에 Runtime 적용합니다.
	static FCFRuntimeEquipApplyResult ApplyCatalogEquipment(
		ACFVehiclePawn* VehiclePawn,
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		FName TargetMountProfileId,
		UCFEquipmentPresetData* CandidateEquipmentPresetData);

	// [v1.0.0] 이미 authorization이 끝난 장비를 단일 MountProfile에 Runtime 적용합니다.
	static FCFRuntimeEquipApplyResult ApplyEquipmentRuntime(
		ACFVehiclePawn* VehiclePawn,
		FName TargetMountProfileId,
		UCFEquipmentPresetData* CandidateEquipmentPresetData);
};
