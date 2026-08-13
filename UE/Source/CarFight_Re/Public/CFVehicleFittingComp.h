// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-04
// Description: CF-FQ-033~034 초기 출격 피팅·질량·Defense Commit 상태 컴포넌트
// Scope: Legacy·Snapshot 입력 준비, Initial Mass 설정값·실제 질량 Coverage 검증, Weapon·Defense 원자 Commit·Rollback과 Applied Snapshot 수명을 소유합니다.
// Changelog:
// - v1.2.0: Movement 설정값은 Snapshot Target과 정확 비교하고 VehicleMesh 실제 질량은 PhysicsAsset 집계 질량을 허용하는 Target 하한 검증으로 분리.
// - v1.1.0: PreRegister와 BeginPlay가 공유할 Initial Mass Target, Legacy fallback, Verify Only, 재적용 거부와 실제 질량 검증 계약을 추가.
// - v1.0.0: Sortie Fitting Prepare·Commit·Rollback, Applied Snapshot, Legacy 경로와 Pawn 없는 Adapter 계약을 최초 추가.
// Migration:
// - VehicleFittingData가 없으면 기존 VehicleData 기반 Weapon·Defense와 기존 Chaos 질량을 그대로 사용한다.
// - 초기 Invalid Snapshot은 Legacy 입력으로 fallback하며, 이미 다른 Snapshot 질량이 구성된 수명에서는 변경 요청을 거부한다.
// - Initial Mass는 Pawn이 물리 등록 전에 Movement Mass에 기록하고 이 컴포넌트는 Target·상태·허용 오차만 관리한다.
// - VehicleMesh 실제 질량이 Target보다 큰 경우는 PhysicsAsset의 집계 질량 증거로 허용하며, 허용 오차보다 크게 부족한 경우만 전파 실패로 거부한다.
// - SetMassOverrideInKg, Physics State 재생성, Ammo, Inventory Adapter와 Field Equip·Unequip은 이 컴포넌트에서 처리하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFFittingTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleFittingComp.generated.h"

class ACFVehiclePawn;
class UCFEquipmentPresetData;
class UCFVehicleData;
class UCFVehicleDefenseComp;
class UCFVehicleDefenseData;
class UCFVehicleFittingData;
class UCFVehicleWeaponComp;

/** 출격 피팅 Runtime Apply의 현재 트랜잭션 상태입니다. */
UENUM(BlueprintType)
enum class ECFFittingRuntimeApplyState : uint8
{
	Uninitialized UMETA(DisplayName="초기화 전 (Uninitialized)"),
	PreparedLegacy UMETA(DisplayName="Legacy 준비됨 (Prepared Legacy)"),
	PreparedSnapshot UMETA(DisplayName="Snapshot 준비됨 (Prepared Snapshot)"),
	CommittedLegacy UMETA(DisplayName="Legacy 적용 완료 (Committed Legacy)"),
	CommittedSnapshot UMETA(DisplayName="Snapshot 적용 완료 (Committed Snapshot)"),
	ApplyFailed UMETA(DisplayName="적용 실패 (Apply Failed)"),
	RolledBack UMETA(DisplayName="Rollback 완료 (Rolled Back)")
};

/** 초기 출격 질량 Prepare·기록·검증의 현재 상태입니다. */
UENUM(BlueprintType)
enum class ECFInitialMassState : uint8
{
	Uninitialized UMETA(DisplayName="초기화 전 (Uninitialized)"),
	LegacyPreserved UMETA(DisplayName="Legacy 질량 유지 (Legacy Preserved)"),
	PreparedSnapshot UMETA(DisplayName="Snapshot 질량 준비됨 (Prepared Snapshot)"),
	ConfiguredBeforePhysics UMETA(DisplayName="물리 생성 전 설정됨 (Configured Before Physics)"),
	Verified UMETA(DisplayName="실제 질량 검증 완료 (Verified)"),
	VerificationFailed UMETA(DisplayName="실제 질량 검증 실패 (Verification Failed)"),
	ReapplyRejected UMETA(DisplayName="런타임 재적용 거부 (Reapply Rejected)")
};

/** VehicleWeaponComp에 전달할 출격 피팅 입력입니다. */
USTRUCT(BlueprintType)
struct FCFFittingWeaponRuntimeInput
{
	GENERATED_BODY()

	// [v1.0.0] 이 입력이 VehicleData 기본 장비 경로를 사용하는지 반환합니다.
	bool UsesLegacyVehicleConfiguration() const { return bUseLegacyVehicleConfiguration; }

	// [v1.0.0] True이면 VehicleData 기본 장비 초기화 경로를 사용합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Weapon")
	bool bUseLegacyVehicleConfiguration = true;

	// [v1.0.0] Snapshot에서 실제 장착 프로파일 한 건을 해결했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Weapon")
	bool bHasResolvedMount = false;

	// [v1.0.0] VehicleWeaponComp가 활성 장착으로 사용할 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Weapon")
	FName ActiveMountProfileId = NAME_None;

	// [v1.0.0] Snapshot이 선택한 EquipmentPresetData입니다. 빈 장착이면 None입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Weapon")
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

	// [v1.0.0] Snapshot 장비 선택의 원본입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Weapon")
	ECFFittingSelectionSource SelectionSource = ECFFittingSelectionSource::None;
};

/** VehicleDefenseComp에 전달할 출격 피팅 입력입니다. */
USTRUCT(BlueprintType)
struct FCFFittingDefenseRuntimeInput
{
	GENERATED_BODY()

	// [v1.0.0] 이 입력이 VehicleData 기본 방어 경로를 사용하는지 반환합니다.
	bool UsesLegacyVehicleConfiguration() const { return bUseLegacyVehicleConfiguration; }

	// [v1.0.0] True이면 VehicleData.DefaultDefenseData 경로를 사용합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Defense")
	bool bUseLegacyVehicleConfiguration = true;

	// [v1.0.0] Snapshot이 사용한 방어 선택 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Defense")
	ECFDefenseSelectionMode SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] Snapshot이 최종 해석한 VehicleDefenseData입니다. ExplicitNone이면 None입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Defense")
	TObjectPtr<UCFVehicleDefenseData> DefenseData = nullptr;
};

/** 하나의 출격 초기화에서 Weapon·Defense가 함께 사용할 원자 적용 입력입니다. */
USTRUCT(BlueprintType)
struct FCFFittingSortieRuntimeInput
{
	GENERATED_BODY()

	// [v1.0.0] Legacy 또는 Snapshot Runtime 입력 계약이 유효한지 반환합니다.
	bool IsValid() const;

	// [v1.0.0] VehicleData 기본값만 사용하는 Legacy 경로인지 반환합니다.
	bool UsesLegacyVehicleConfiguration() const { return bUseLegacyVehicleConfiguration; }

	// [v1.0.0] True이면 Snapshot을 적용하지 않고 기존 VehicleData 초기화를 사용합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime")
	bool bUseLegacyVehicleConfiguration = true;

	// [v1.0.0] Weapon·Defense 입력의 기준 VehicleData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime")
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	// [v1.0.0] Snapshot 경로에서 Commit할 결정론적 피팅 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime")
	FCFVehicleFittingSnapshot FittingSnapshot;

	// [v1.0.0] VehicleWeaponComp에 전달할 장비 입력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime")
	FCFFittingWeaponRuntimeInput WeaponInput;

	// [v1.0.0] VehicleDefenseComp에 전달할 방어 입력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Runtime")
	FCFFittingDefenseRuntimeInput DefenseInput;
};

/** Pawn 없이 Prepare·Commit·Rollback을 검증할 수 있게 하는 Runtime Adapter 계약입니다. */
class CARFIGHT_RE_API ICFFittingRuntimeApplyAdapter
{
public:
	// [v1.0.0] 파생 Adapter를 안전하게 파괴하기 위한 가상 소멸자입니다.
	virtual ~ICFFittingRuntimeApplyAdapter() = default;

	// [v1.0.0] Weapon Runtime 입력을 적용하고 실제 준비 상태를 반환합니다.
	virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) = 0;

	// [v1.0.0] Defense Runtime 입력을 적용하고 실제 준비 상태를 반환합니다.
	virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) = 0;
};

/** 출격 피팅의 검증, 원자 적용 경계와 Applied Snapshot을 소유하는 차량 컴포넌트입니다. */
UCLASS(ClassGroup=(CarFight), BlueprintType, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleFittingComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] Tick이 필요 없는 출격 피팅 컴포넌트 기본값을 초기화합니다.
	UCFVehicleFittingComp();

	// [v1.0.0] EndPlay에서 Prepared·Applied 상태를 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

		// [v1.0.0] VehicleFittingData를 검증해 아직 적용하지 않은 Runtime 입력을 준비합니다.
	bool PrepareSortieFitting(const UCFVehicleFittingData* InVehicleFittingData, UCFVehicleData* InVehicleData, FName RequestedActiveMountProfileId);

	// [v1.1.0] 초기 출격에서 PreRegister와 BeginPlay가 공유할 Runtime 입력과 질량 Target을 한 번 준비합니다.
	bool PrepareInitialSortieFitting(const UCFVehicleFittingData* InVehicleFittingData, UCFVehicleData* InVehicleData, FName RequestedActiveMountProfileId);

	// [v1.1.0] 물리 생성 전 Movement Mass 기록이 필요한 Snapshot 준비 상태인지 반환합니다.
	bool ShouldApplyPreparedInitialMass() const;

	// [v1.1.0] 현재 Prepared 입력이 기존 Chaos 질량을 그대로 사용하는 Legacy 경로인지 반환합니다.
	bool UsesLegacyInitialMass() const { return bPreparedInitialMassUsesLegacy; }

	// [v1.1.0] 물리 생성 전에 기록할 Prepared Snapshot 총중량을 반환합니다.
	float GetPreparedInitialMassKg() const { return PreparedInitialMassKg; }

	// [v1.1.0] Target Mass 검증에 사용할 최소 1kg 또는 1% 허용 오차를 계산합니다.
	static float CalculateInitialMassToleranceKg(float TargetMassKg);

	// [v1.1.0] Pawn이 물리 생성 전에 기록한 이전·설정 Movement Mass를 상태에 등록합니다.
	bool RecordInitialMassBeforePhysics(float PreviousMovementMassKg, float ConfiguredMovementMassKg);

	// [v1.2.0] BeginPlay에서 Movement 설정값의 Target 일치, VehicleMesh 실제 질량의 Target Coverage와 Physics 상태를 검증합니다.
	bool VerifyInitialMassAfterPhysics(float ConfiguredMovementMassKg, float ActualVehicleMeshMassKg, bool bHasPhysicsState, bool bSimulatesPhysics, bool bHasPhysicsAsset);

	// [v1.1.0] 물리 생성 전 Snapshot Mass 적용 실패를 Legacy Runtime 입력으로 되돌립니다.
	bool FallbackPreparedInitialMassToLegacy(UCFVehicleData* InVehicleData, FName RequestedActiveMountProfileId, const FString& FailureReason);

	// [v1.0.0] 준비된 입력을 Adapter에 원자 적용하고 실패 시 직전 입력으로 복원합니다.
	bool CommitPreparedSortieFitting(ICFFittingRuntimeApplyAdapter& RuntimeApplyAdapter);

	// [v1.0.0] 실제 차량 Weapon·Defense 컴포넌트에 준비된 출격 피팅을 Commit합니다.
	bool CommitPreparedSortieFittingToVehicle(ACFVehiclePawn* OwnerVehiclePawn, UCFVehicleWeaponComp* VehicleWeaponComp, UCFVehicleDefenseComp* VehicleDefenseComp);

	// [v1.0.0] 하위 Runtime을 변경하지 않고 Prepared 입력만 취소합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Fitting|Runtime")
	void RollbackPreparedSortieFitting();

	// [v1.0.0] Prepared·Applied 상태와 Snapshot을 모두 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Fitting|Runtime")
	void ResetFittingRuntimeState();

		// [v1.0.0] 현재 Runtime Apply 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	ECFFittingRuntimeApplyState GetRuntimeApplyState() const { return RuntimeApplyState; }

	// [v1.1.0] 현재 초기 출격 질량 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime|Mass")
	ECFInitialMassState GetInitialMassState() const { return InitialMassState; }

	// [v1.2.0] Movement 설정값 일치, VehicleMesh 질량 Coverage와 Physics 계약을 모두 통과했는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime|Mass")
	bool HasVerifiedInitialMass() const { return bInitialMassVerified; }

	// [v1.1.0] 현재 Pawn 수명에 Snapshot 초기 질량이 구성됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime|Mass")
	bool HasConfiguredInitialMass() const { return bHasConfiguredInitialMass; }

	// [v1.1.0] 현재 Pawn 수명에 구성된 Snapshot 초기 질량을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime|Mass")
	float GetConfiguredInitialMassKg() const { return ConfiguredInitialMassKg; }

	// [v1.1.0] 마지막 초기 질량 Prepare·검증 결과를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime|Mass")
	FString GetLastInitialMassSummary() const { return LastInitialMassSummary; }

	// [v1.0.0] Prepared 입력 존재 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	bool HasPreparedRuntimeInput() const { return bHasPreparedRuntimeInput; }

	// [v1.0.0] Commit된 Runtime 입력 존재 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	bool HasAppliedRuntimeInput() const { return bHasAppliedRuntimeInput; }

	// [v1.0.0] 현재 출격에 적용된 Snapshot 존재 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	bool HasAppliedFittingSnapshot() const { return bHasAppliedFittingSnapshot; }

	// [v1.0.0] 현재 출격에 적용된 읽기 전용 Snapshot을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	FCFVehicleFittingSnapshot GetAppliedFittingSnapshot() const { return AppliedFittingSnapshot; }

	// [v1.0.0] 마지막 Commit 후 Weapon 준비 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	bool WasLastWeaponRuntimeReady() const { return bLastWeaponRuntimeReady; }

	// [v1.0.0] 마지막 Commit 후 DefenseData 준비 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	bool WasLastDefenseRuntimeReady() const { return bLastDefenseRuntimeReady; }

	// [v1.0.0] 마지막 Prepare·Commit·Rollback 결과를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Fitting|Runtime")
	FString GetLastFittingRuntimeSummary() const { return LastFittingRuntimeSummary; }

private:
	// [v1.0.0] VehicleFittingData가 없을 때 Legacy Runtime 입력을 생성합니다.
	static FCFFittingSortieRuntimeInput BuildLegacyRuntimeInput(UCFVehicleData* InVehicleData, FName RequestedActiveMountProfileId);

	// [v1.0.0] 유효 Snapshot을 Weapon·Defense 입력으로 변환합니다.
	static bool BuildSnapshotRuntimeInput(const FCFVehicleFittingSnapshot& FittingSnapshot, FName RequestedActiveMountProfileId, FCFFittingSortieRuntimeInput& OutRuntimeInput, FString& OutFailureSummary);

	// [v1.0.0] 직전 Applied 또는 Legacy 입력을 Adapter에 다시 적용합니다.
	static bool RestoreRuntimeInput(ICFFittingRuntimeApplyAdapter& RuntimeApplyAdapter, const FCFFittingSortieRuntimeInput& RuntimeInput, bool& bOutWeaponRuntimeReady, bool& bOutDefenseRuntimeReady);

		// [v1.0.0] Prepared 입력만 기본값으로 정리합니다.
	void ClearPreparedRuntimeInput();

	// [v1.1.0] 현재 Prepared Initial Mass Target과 Legacy·Verify Only 플래그를 정리합니다.
	void ClearPreparedInitialMassInput();

	// [v1.0.0] 아직 Commit되지 않은 입력 존재 여부입니다.
	UPROPERTY(Transient)
	bool bHasPreparedRuntimeInput = false;

	// [v1.0.0] 다음 Commit이 사용할 Runtime 입력입니다.
	UPROPERTY(Transient)
	FCFFittingSortieRuntimeInput PreparedRuntimeInput;

	// [v1.0.0] 현재 Commit된 입력 존재 여부입니다.
	UPROPERTY(Transient)
	bool bHasAppliedRuntimeInput = false;

	// [v1.0.0] 실패 Rollback에 사용할 직전 Commit 입력입니다.
	UPROPERTY(Transient)
	FCFFittingSortieRuntimeInput AppliedRuntimeInput;

	// [v1.0.0] Legacy가 아닌 Snapshot이 현재 출격에 적용됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime", meta=(AllowPrivateAccess="true"))
	bool bHasAppliedFittingSnapshot = false;

	// [v1.0.0] 현재 출격 동안 읽기 전용으로 유지할 최종 Snapshot입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime", meta=(AllowPrivateAccess="true"))
	FCFVehicleFittingSnapshot AppliedFittingSnapshot;

		// [v1.0.0] 현재 트랜잭션 상태입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime", meta=(AllowPrivateAccess="true"))
	ECFFittingRuntimeApplyState RuntimeApplyState = ECFFittingRuntimeApplyState::Uninitialized;

	// [v1.1.0] 현재 초기 출격 질량 상태입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Mass", meta=(AllowPrivateAccess="true"))
	ECFInitialMassState InitialMassState = ECFInitialMassState::Uninitialized;

	// [v1.1.0] Prepared 입력이 VehicleData 기본 Chaos 질량을 그대로 사용하는지 여부입니다.
	UPROPERTY(Transient)
	bool bPreparedInitialMassUsesLegacy = true;

	// [v1.1.0] Prepared Snapshot의 유효한 초기 질량 Target을 보유하는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasPreparedInitialMass = false;

	// [v1.1.0] 물리 생성 전 실제 Movement Mass 기록이 필요한지 여부입니다.
	UPROPERTY(Transient)
	bool bPreparedInitialMassNeedsApply = false;

	// [v1.1.0] 이미 구성된 같은 질량을 재작성하지 않고 검증만 수행할지 여부입니다.
	UPROPERTY(Transient)
	bool bPreparedInitialMassVerifyOnly = false;

	// [v1.1.0] Prepared Snapshot의 결정론적 TotalVehicleMassKg입니다.
	UPROPERTY(Transient)
	float PreparedInitialMassKg = 0.0f;

	// [v1.1.0] 최초 Snapshot 적용 전에 보존한 기존 Movement Mass입니다.
	UPROPERTY(Transient)
	float PreviousMovementMassKg = 0.0f;

	// [v1.1.0] 현재 Pawn 물리 수명에 Snapshot 질량이 구성됐는지 여부입니다.
	UPROPERTY(Transient)
	bool bHasConfiguredInitialMass = false;

	// [v1.1.0] 현재 Pawn 물리 수명에 기록된 Snapshot Target Mass입니다.
	UPROPERTY(Transient)
	float ConfiguredInitialMassKg = 0.0f;

	// [v1.2.0] BeginPlay Movement 설정값·VehicleMesh 질량 Coverage·Physics 계약 검증 통과 여부입니다.
	UPROPERTY(Transient)
	bool bInitialMassVerified = false;

	// [v1.1.0] 마지막 Initial Mass Prepare·기록·검증 결과 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime|Mass", meta=(AllowPrivateAccess="true"))
	FString LastInitialMassSummary = TEXT("InitialMass: Uninitialized");

	// [v1.0.0] 마지막 Commit 후 Weapon 준비 상태입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime", meta=(AllowPrivateAccess="true"))
	bool bLastWeaponRuntimeReady = false;

	// [v1.0.0] 마지막 Commit 후 DefenseData 준비 상태입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime", meta=(AllowPrivateAccess="true"))
	bool bLastDefenseRuntimeReady = false;

	// [v1.0.0] 마지막 피팅 Runtime 결과 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Fitting|Runtime", meta=(AllowPrivateAccess="true"))
	FString LastFittingRuntimeSummary = TEXT("FittingRuntime: Uninitialized");
};
