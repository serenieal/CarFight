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

	// 이 입력이 대응하는 휠 인덱스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	int32 WheelIndex = INDEX_NONE;

	// 현재 프레임 입력이 유효한지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bIsValidInput = false;

	// Anchor에 적용할 조향 Yaw 각도(deg)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SteeringYawDeg = 0.0f;

	// Mesh에 반영할 누적 SpinPitch 각도(deg)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SpinPitchDeg = 0.0f;

	// Mesh에 이번 프레임 추가로 반영할 SpinPitch delta 각도(deg)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	float SpinPitchDeltaDeg = 0.0f;

	// SpinPitch를 절대각 대신 delta 회전으로 적용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync")
	bool bApplySpinPitchAsDelta = false;

	// Anchor에 적용할 서스펜션 Z 오프셋입니다.
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

	// Wheel mesh의 로컬 Pitch 축 방향 보정용 부호입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="Wheel mesh의 로컬 Pitch 축 방향 보정용 부호입니다. 현재 프로젝트 PoliceCar 기준 기본값은 -1입니다."))
	float WheelSpinMeshAxisSign;

	// 매우 작은 휠 각속도를 0으로 보는 dead zone 임계값(deg/s)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="매우 작은 휠 각속도를 0으로 보는 dead zone 임계값(deg/s)입니다."))
	float WheelSpinAngularVelocityDeadZoneDegPerSec;

	// Forward 속도를 방향 안정화 기준으로 사용하기 위한 dead zone 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="Forward 속도를 방향 안정화 기준으로 사용하기 위한 dead zone 임계값(km/h)입니다."))
	float WheelSpinForwardDeadZoneKmh;

	// 휠 자체 각속도 부호를 신뢰할 만큼 충분히 강한 슬립으로 보는 임계값(deg/s)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="휠 자체 각속도 부호를 신뢰할 만큼 충분히 강한 슬립으로 보는 임계값(deg/s)입니다."))
	float WheelSpinSlipAngularVelocityThresholdDegPerSec;

	// 방향 반전 후보가 이 시간 이상 유지될 때만 실제 stable sign을 바꾸는 hold 시간(sec)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="방향 반전 후보가 이 시간 이상 유지될 때만 실제 stable sign을 바꾸는 hold 시간(sec)입니다."))
	float WheelSpinDirectionChangeHoldTimeSeconds;

	// 접지 중 저속 상태에서 visual spin 과회전을 막기 위해 상한을 적용할 최대 Forward 속도(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="접지 중 저속 상태에서 visual spin 과회전을 막기 위해 상한을 적용할 최대 Forward 속도(km/h)입니다."))
	float WheelSpinLowSpeedVisualCapMaxKmh;

	// 접지 중 저속 상태에서 무슬립 기준 각속도에 허용할 최대 overspeed 배수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WheelSync", meta=(ToolTip="접지 중 저속 상태에서 무슬립 기준 각속도에 허용할 최대 overspeed 배수입니다."))
	float WheelSpinLowSpeedMaxOverspeedRatio;

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

	// 마지막 프레임에 읽은 실제 휠 회전각(deg) 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="마지막 프레임에 읽은 실제 휠 회전각(deg) 배열입니다."))
	TArray<float> LastRuntimeWheelAnglesDeg;

	// 래핑을 풀어서 누적한 실제 휠 시각 스핀 각(deg) 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="래핑을 풀어서 누적한 실제 휠 시각 스핀 각(deg) 배열입니다."))
	TArray<float> AccumulatedRuntimeWheelSpinDeg;

	// 실제 휠 회전각의 이전 프레임 샘플이 유효한지 나타내는 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="실제 휠 회전각의 이전 프레임 샘플이 유효한지 나타내는 배열입니다."))
	TArray<bool> bHasRuntimeWheelAngleSample;

	// 현재 휠별로 시각 회전에 사용 중인 안정화 방향 부호 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="현재 휠별로 시각 회전에 사용 중인 안정화 방향 부호 배열입니다."))
	TArray<int32> StableWheelSpinDirectionSigns;

	// 현재 휠별로 방향 전환 후보 부호를 임시 보관하는 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="현재 휠별로 방향 전환 후보 부호를 임시 보관하는 배열입니다."))
	TArray<int32> PendingWheelSpinDirectionSigns;

	// 현재 휠별로 방향 전환 후보가 유지된 누적 시간(sec) 배열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync|Debug|Runtime", meta=(EditCondition="bDebugMode", EditConditionHides, ToolTip="현재 휠별로 방향 전환 후보가 유지된 누적 시간(sec) 배열입니다."))
	TArray<float> WheelSpinDirectionHoldElapsedSeconds;

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

	// 마지막 Phase2 입력 생성 요약 문자열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="WheelSync")
	FString LastInputBuildSummary;

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

	// 실제 휠 회전각 샘플 캐시를 초기화합니다.
	void ResetRuntimeWheelSpinCaches();

	// 현재 차량 Forward 속도(km/h)를 WheelSync 기준으로 계산합니다.
	float GetCurrentForwardSpeedKmhForWheelSpin() const;

	// 휠 각속도와 Forward 속도 기준으로 원하는 회전 방향 부호를 결정합니다.
	int32 ResolveDesiredWheelSpinDirectionSign(float InForwardSpeedKmh, float InWheelAngularVelocityDegPerSec) const;

	// 휠별 방향 hold 상태를 반영해 실제 stable sign을 갱신합니다.
	int32 UpdateStableWheelSpinDirectionSign(int32 InWheelIndex, int32 InDesiredDirectionSign, float InDeltaSeconds);
	bool bHasLoggedHelperFallback;
	bool FailValidation(const FString& FailureMessage);
	bool PassValidation(const FString& SuccessMessage);
};
