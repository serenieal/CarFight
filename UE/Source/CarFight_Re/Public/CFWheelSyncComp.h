#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFWheelSyncComp.generated.h"

class UChaosWheeledVehicleMovementComponent;
class USceneComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FCFWheelVisualInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	int32 WheelIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bIsValidInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SteeringYawDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SpinPitchDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SuspensionOffsetZ = 0.0f;
};

USTRUCT(BlueprintType)
struct FCFWheelVisualState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	int32 WheelIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	bool bApplied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	FVector LastAppliedAnchorLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	FRotator LastAppliedAnchorRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	FRotator LastAppliedMeshRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FCFWheelHelperCompareState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	int32 WheelIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	bool bHelperSampleAttempted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float DebugSteeringYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float HelperSteeringYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float DeltaSteeringYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float DebugSpinPitchDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float HelperSpinPitchDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float DeltaSpinPitchDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float DebugSuspensionOffsetZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float HelperSuspensionOffsetZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="WheelSync")
	float DeltaSuspensionOffsetZ = 0.0f;
};

UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFWheelSyncComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCFWheelSyncComp();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	int32 ExpectedWheelCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bAutoPrepareOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bAutoFindComponentsByName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug", meta=(InlineEditConditionToggle, ToolTip="True이면 개발/검증용 상세 옵션을 디테일 패널에 노출합니다."))
	bool bDebugMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 WheelSync 상세 로그를 출력합니다."))
	bool bVerboseLog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bEnableApplyTransformsInCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 입력 경로를 사용합니다."))
	bool bUseRealWheelTransformHelper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 결과가 디버그 파이프 입력 전체를 덮어씁니다."))
	bool bHelperOverridesDebugInputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 결과로 SteeringYaw를 덮어씁니다."))
	bool bHelperOverridesSteeringYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 결과로 SuspensionZ를 덮어씁니다."))
	bool bHelperOverridesSuspensionZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 결과로 SpinPitch를 덮어씁니다."))
	bool bHelperOverridesSpinPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper와 디버그 파이프 입력 차이를 비교합니다."))
	bool bEnableHelperCompareMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 비교 상세 로그를 출력합니다."))
	bool bLogHelperCompareDetails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="helper 비교 경고 각도 임계값입니다."))
	float HelperCompareWarnDeltaDeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="helper 비교 경고 Z 임계값입니다."))
	float HelperCompareWarnDeltaZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Debug|Helper", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="True이면 helper 미사용/실패 로그를 1회만 출력합니다."))
	bool bLogHelperFallbackOnce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bUseSteeringYawDebugPipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	int32 FrontWheelCountForSteering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SteeringYawScaleDeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SteeringVisualSign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bUseSuspensionZDebugPipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SuspensionZDebugOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SuspensionVisualSign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bUseWheelSpinPitchDebugPipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float WheelSpinPitchDebugDeg;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float WheelSpinVisualSign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	TArray<FName> WheelAnchorComponentNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	TArray<FName> WheelMeshComponentNames;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedVehicleMovementComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	TArray<TObjectPtr<USceneComponent>> CachedWheelAnchorComponents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	TArray<TObjectPtr<UStaticMeshComponent>> CachedWheelMeshComponents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	TArray<FVector> BaseWheelAnchorLocations;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	TArray<FRotator> BaseWheelAnchorRotations;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	TArray<FRotator> BaseWheelMeshRotations;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="마지막으로 계산된 Wheel Visual 입력 배열입니다."))
	TArray<FCFWheelVisualInput> LastWheelVisualInputs;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="마지막으로 적용된 Wheel Visual 상태 배열입니다."))
	TArray<FCFWheelVisualState> LastWheelVisualStates;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="helper 입력 vs 디버그 파이프 입력 비교 상태 배열입니다."))
	TArray<FCFWheelHelperCompareState> LastHelperCompareStates;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="helper 비교 요약 문자열입니다."))
	FString LastHelperCompareSummary;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="helper 비교 경고 휠 인덱스 목록 문자열입니다."))
	FString LastHelperCompareWarnWheelIndices;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|HelperCompare", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="helper 비교 전/후륜 요약 문자열입니다."))
	FString LastHelperCompareFrontRearSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync|Legacy", meta=(EditCondition="bDebugMode", EditConditionHides, DeprecatedProperty, DeprecationMessage="Legacy single-axis test field. No longer used by CFWheelSyncComp runtime.", ToolTip="레거시 단일축 테스트 필드입니다. 현재 런타임에서는 사용하지 않습니다."))
	bool bEnableSingleAxisSteeringYawCppApplyOnly;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Legacy", meta=(EditCondition="bDebugMode", EditConditionHides, DeprecatedProperty, DeprecationMessage="Legacy single-axis summary field. No longer updated by CFWheelSyncComp runtime.", ToolTip="레거시 단일축 테스트 요약 문자열입니다. 현재 런타임에서는 갱신하지 않습니다."))
	FString SingleAxisCppApplyTestSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bApplySteeringYawInCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bApplySuspensionZInCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bApplySpinPitchInCpp;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	bool bWheelSyncReady;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	FString LastValidationSummary;

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	void ResetWheelSyncState();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool CacheVehicleMovementComponent();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool BuildWheelComponentCache();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool CaptureBaseWheelVisualState();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool ValidateWheelSyncSetup();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool TryPrepareWheelSync();

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool BuildWheelVisualInputsFromDebugPipe(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs);

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool BuildWheelVisualInputsPhase2(float DeltaSeconds, TArray<FCFWheelVisualInput>& OutWheelInputs);

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool ApplyWheelVisualInputsPhase2(const TArray<FCFWheelVisualInput>& InWheelInputs);

	UFUNCTION(BlueprintCallable, Category="WheelSync")
	bool UpdateWheelVisualsPhase2(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsWheelSyncReady() const;

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsSteeringYawOwnedByCpp() const;

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsSuspensionZOwnedByCpp() const;

	UFUNCTION(BlueprintPure, Category="WheelSync")
	bool IsSpinPitchOwnedByCpp() const;

private:
	void InitializeDefaultWheelNames();
	USceneComponent* FindSceneComponentByName(FName ComponentName) const;
	UStaticMeshComponent* FindStaticMeshComponentByName(FName ComponentName) const;
	bool ApplySingleWheelInputPhase2(const FCFWheelVisualInput& WheelInput);
	bool TryApplyRealTransformHelperToInput(int32 WheelIndex, const FCFWheelVisualInput& DebugWheelInput, FCFWheelVisualInput& InOutFinalWheelInput);
	void ResetHelperCompareStates();
	void FinalizeHelperCompareSummary();
	bool IsFrontWheelIndexForSteering(int32 WheelIndex) const;
	void ResetBaseVisualCaches();
	void ResetLastWheelStates();
	bool bHasLoggedHelperFallback;
	bool FailValidation(const FString& FailureMessage);
	bool PassValidation(const FString& SuccessMessage);
};