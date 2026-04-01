// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.2
// Date: 2026-04-01
// Description: CarFight 차량 Drive 상태/입력 해석 컴포넌트 (DriveComp 카테고리 루트 분리)
// Scope: Throttle / Steering / Brake / Handbrake 입력을 Chaos Vehicle Movement에 전달하고, 현재 주행 상태와 상태 전이를 Native 기준으로 계산합니다.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFVehicleDriveStateConfig.h"
#include "CFVehicleDriveComp.generated.h"

class UChaosWheeledVehicleMovementComponent;

UENUM(BlueprintType)
enum class ECFVehicleDriveState : uint8
{
	Idle UMETA(DisplayName="Idle"),
	Accelerating UMETA(DisplayName="Accelerating"),
	Coasting UMETA(DisplayName="Coasting"),
	Braking UMETA(DisplayName="Braking"),
	Reversing UMETA(DisplayName="Reversing"),
	Airborne UMETA(DisplayName="Airborne"),
	Disabled UMETA(DisplayName="Disabled")
};

USTRUCT(BlueprintType)
struct FCFVehicleInputState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="스로틀 입력 (ThrottleInput)", ToolTip="마지막으로 DriveComp에 적용한 스로틀 입력값입니다."))
	float ThrottleInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="조향 입력 (SteeringInput)", ToolTip="마지막으로 DriveComp에 적용한 조향 입력값입니다."))
	float SteeringInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="브레이크 입력 (BrakeInput)", ToolTip="마지막으로 DriveComp에 적용한 브레이크 입력값입니다."))
	float BrakeInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="핸드브레이크 입력 상태 (bHandbrakePressed)", ToolTip="마지막으로 DriveComp에 적용한 핸드브레이크 상태입니다."))
	bool bHandbrakePressed = false;
};

USTRUCT(BlueprintType)
struct FCFVehicleDriveStateSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 차량 주행 상태 Enum 입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="현재 속도 km/h (CurrentSpeedKmh)", ToolTip="현재 차량 전체 속도(km/h)입니다."))
	float CurrentSpeedKmh = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="전방 기준 속도 km/h (ForwardSpeedKmh)", ToolTip="차량 전방 벡터 기준 부호가 반영된 속도(km/h)입니다."))
	float ForwardSpeedKmh = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="접지 상태 (bIsGrounded)", ToolTip="현재 프레임을 지면 접촉 상태로 볼지에 대한 판정 결과입니다."))
	bool bIsGrounded = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="현재 입력 상태 스냅샷 (CurrentInputState)", ToolTip="현재 Drive 상태를 계산할 때 사용한 마지막 입력 상태 스냅샷입니다."))
	FCFVehicleInputState CurrentInputState;
};

UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleDriveComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCFVehicleDriveComp();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="캐시된 Vehicle Movement 컴포넌트 (CachedVehicleMovementComponent)", ToolTip="Owner에서 찾은 Chaos Wheeled Vehicle Movement Component 캐시입니다."))
	TObjectPtr<UChaosWheeledVehicleMovementComponent> CachedVehicleMovementComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(DisplayName="Drive 상태 자동 갱신 (bEnableDriveStateUpdates)", ToolTip="True이면 Tick과 입력 갱신 시 현재 Drive 상태를 자동으로 갱신합니다."))
	bool bEnableDriveStateUpdates = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(DisplayName="Drive 상태 히스테리시스 사용 (bEnableDriveStateHysteresis)", ToolTip="True이면 상태 진입/이탈 임계값과 최소 유지 시간을 사용해 Drive 상태 튐을 줄입니다."))
	bool bEnableDriveStateHysteresis = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(DisplayName="상태별 유지 시간 사용 (bUsePerStateHoldTimes)", ToolTip="True이면 Idle, Reversing, Airborne 상태마다 서로 다른 최소 유지 시간을 사용합니다."))
	bool bUsePerStateHoldTimes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="기본 DriveState 최소 유지 시간 (DriveStateMinimumHoldTimeSeconds)", ToolTip="기본 DriveState 최소 유지 시간(초)입니다."))
	float DriveStateMinimumHoldTimeSeconds = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Idle 최소 유지 시간 (IdleStateMinimumHoldTimeSeconds)", ToolTip="Idle 상태 최소 유지 시간(초)입니다."))
	float IdleStateMinimumHoldTimeSeconds = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Reversing 최소 유지 시간 (ReversingStateMinimumHoldTimeSeconds)", ToolTip="Reversing 상태 최소 유지 시간(초)입니다."))
	float ReversingStateMinimumHoldTimeSeconds = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Airborne 최소 유지 시간 (AirborneStateMinimumHoldTimeSeconds)", ToolTip="Airborne 상태 최소 유지 시간(초)입니다."))
	float AirborneStateMinimumHoldTimeSeconds = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Idle 진입 속도 임계값 (IdleEnterSpeedThresholdKmh)", ToolTip="Idle 상태로 진입할 때 사용할 속도 임계값(km/h)입니다."))
	float IdleEnterSpeedThresholdKmh = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Idle 이탈 속도 임계값 (IdleExitSpeedThresholdKmh)", ToolTip="Idle 상태에서 빠져나올 때 사용할 속도 임계값(km/h)입니다."))
	float IdleExitSpeedThresholdKmh = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Reversing 진입 속도 임계값 (ReverseEnterSpeedThresholdKmh)", ToolTip="Reversing 상태로 진입할 때 사용할 후진 속도 임계값(km/h)입니다."))
	float ReverseEnterSpeedThresholdKmh = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="Reversing 이탈 속도 임계값 (ReverseExitSpeedThresholdKmh)", ToolTip="Reversing 상태에서 빠져나올 때 사용할 후진 속도 임계값(km/h)입니다."))
	float ReverseExitSpeedThresholdKmh = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="공중 상태 최소 속도 임계값 (AirborneMinSpeedThresholdKmh)", ToolTip="공중 상태 진입으로 볼 최소 속도 임계값(km/h)입니다."))
	float AirborneMinSpeedThresholdKmh = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="유효 입력 임계값 (ActiveInputThreshold)", ToolTip="입력을 유효 입력으로 판정할 최소 절대값입니다."))
	float ActiveInputThreshold = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(ClampMin="0.0", DisplayName="공중 상태 수직 속도 임계값 (AirborneVerticalSpeedThresholdCmPerSec)", ToolTip="공중 상태 진입으로 볼 수직 속도 임계값(cm/s)입니다."))
	float AirborneVerticalSpeedThresholdCmPerSec = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(DisplayName="휠 접촉 힌트 사용 (bUseWheelContactSurfaceHint)", ToolTip="True이면 Chaos Vehicle Movement의 휠 접촉 표면 힌트를 사용해 접지 판정을 보강합니다. 실패하면 기존 속도 기반 폴백을 사용합니다."))
	bool bUseWheelContactSurfaceHint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDrive", meta=(DisplayName="반대 스로틀을 브레이크로 해석 (bTreatOppositeThrottleAsBrake)", ToolTip="True이면 진행 방향 반대 스로틀 입력을 제동 의도로 해석합니다."))
	bool bTreatOppositeThrottleAsBrake = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="현재 입력 상태 (CurrentInputState)", ToolTip="마지막으로 DriveComp에 적용한 입력 상태입니다."))
	FCFVehicleInputState CurrentInputState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="이전 Drive 상태 (PreviousDriveState)", ToolTip="직전 프레임 기준 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 프레임 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="마지막 상태 변경 시각 (LastDriveStateChangeTimeSeconds)", ToolTip="마지막 Drive 상태 변경 시각(초)입니다. 월드 시간 기준 값입니다."))
	float LastDriveStateChangeTimeSeconds = -1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="이번 프레임 상태 변경 여부 (bDriveStateChangedThisFrame)", ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="상태 변경 횟수 (DriveStateChangeCount)", ToolTip="BeginPlay 이후 누적된 Drive 상태 변경 횟수입니다."))
	int32 DriveStateChangeCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="마지막 상태 전이 요약 (LastDriveStateTransitionSummary)", ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString LastDriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDrive", meta=(DisplayName="마지막 Drive 상태 스냅샷 (LastDriveStateSnapshot)", ToolTip="마지막으로 계산해 저장한 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot LastDriveStateSnapshot;

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	bool CacheVehicleMovementComponent();

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	bool IsDriveReady() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	UChaosWheeledVehicleMovementComponent* GetVehicleMovementComponent() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	float GetCurrentSpeedKmh() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	float GetForwardSpeedKmh() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	ECFVehicleDriveState GetDriveState() const { return CurrentDriveState; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	ECFVehicleDriveState GetPreviousDriveState() const { return PreviousDriveState; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	bool HasDriveStateChangedThisFrame() const { return bDriveStateChangedThisFrame; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	const FString& GetLastDriveStateTransitionSummary() const { return LastDriveStateTransitionSummary; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive")
	FCFVehicleDriveStateSnapshot GetDriveStateSnapshot() const { return LastDriveStateSnapshot; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDrive|Debug", meta=(ToolTip="현재 Drive 상태 스냅샷과 이전 상태, 상태 변경 여부를 사람이 읽기 쉬운 디버그 문자열로 반환합니다."))
	FString GetDriveStateDebugString(bool bIncludeTransitionSummary = true) const;

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	void UpdateDriveState();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	void ApplyDriveStateConfig(const FCFVehicleDriveStateConfig& InDriveStateConfig);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	FCFVehicleDriveStateSnapshot BuildDriveStateSnapshot();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	void ApplyThrottleInput(float InThrottleValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	void ApplySteeringInput(float InSteeringValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	void ApplyBrakeInput(float InBrakeValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDrive")
	void ApplyHandbrakeInput(bool bInHandbrakePressed);

private:
	ECFVehicleDriveState EvaluateDriveState(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot) const;
	bool ShouldEnterAirborneState(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot, float InVerticalSpeedCmPerSec) const;
	bool HasAnyWheelContactSurfaceHint() const;
	float GetCurrentStateMinimumHoldTimeSeconds() const;
	bool CanApplyStateTransition(ECFVehicleDriveState CandidateDriveState, float CurrentWorldTimeSeconds) const;
	void UpdateDriveStateTransitionSummary(const FCFVehicleDriveStateSnapshot& InDriveStateSnapshot);
	float ConvertCmPerSecToKmh(float InSpeedCmPerSec) const;
};
