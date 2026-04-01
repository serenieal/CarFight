// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-03-18
// Description: CarFight DriveState 튜닝 공용 설정 구조체
// Scope: DriveComp와 VehicleData가 공통으로 사용하는 DriveState 임계값/히스테리시스 설정입니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleDriveStateConfig.generated.h"

/**
 * 차량별 DriveState 튜닝 설정입니다.
 * - Idle / Reversing / Airborne 판정과 히스테리시스 값을 차량별로 조정할 때 사용합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDriveStateConfig
{
	GENERATED_BODY()

	// [v1.0.0] True이면 이 차량 데이터의 DriveState 설정을 DriveComp에 적용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(DisplayName="DriveState 덮어쓰기 사용 (bUseDriveStateOverrides)", ToolTip="True이면 이 차량 데이터의 DriveState 설정을 DriveComp에 적용합니다."))
	bool bUseDriveStateOverrides = false;

	// [v1.0.0] True이면 상태 진입/이탈 임계값과 최소 유지 시간을 사용해 Drive 상태 튐을 줄입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(DisplayName="DriveState 히스테리시스 사용 (bEnableDriveStateHysteresis)", ToolTip="True이면 상태 진입/이탈 임계값과 최소 유지 시간을 사용해 Drive 상태 튐을 줄입니다."))
	bool bEnableDriveStateHysteresis = true;

	// [v1.0.0] True이면 Idle, Reversing, Airborne 상태마다 다른 최소 유지 시간을 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(DisplayName="상태별 유지 시간 사용 (bUsePerStateHoldTimes)", ToolTip="True이면 Idle, Reversing, Airborne 상태마다 다른 최소 유지 시간을 사용합니다."))
	bool bUsePerStateHoldTimes = true;

	// [v1.0.0] 기본 상태 유지 시간(초)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="기본 DriveState 최소 유지 시간 (DriveStateMinimumHoldTimeSeconds)", ToolTip="기본 DriveState 최소 유지 시간(초)입니다."))
	float DriveStateMinimumHoldTimeSeconds = 0.12f;

	// [v1.0.0] Idle 상태 유지 시간(초)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Idle 최소 유지 시간 (IdleStateMinimumHoldTimeSeconds)", ToolTip="Idle 상태 최소 유지 시간(초)입니다."))
	float IdleStateMinimumHoldTimeSeconds = 0.10f;

	// [v1.0.0] Reversing 상태 유지 시간(초)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Reversing 최소 유지 시간 (ReversingStateMinimumHoldTimeSeconds)", ToolTip="Reversing 상태 최소 유지 시간(초)입니다."))
	float ReversingStateMinimumHoldTimeSeconds = 0.18f;

	// [v1.0.0] Airborne 상태 유지 시간(초)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Airborne 최소 유지 시간 (AirborneStateMinimumHoldTimeSeconds)", ToolTip="Airborne 상태 최소 유지 시간(초)입니다."))
	float AirborneStateMinimumHoldTimeSeconds = 0.08f;

	// [v1.0.0] Idle 상태로 진입할 때 사용할 속도 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Idle 진입 속도 임계값 (IdleEnterSpeedThresholdKmh)", ToolTip="Idle 상태로 진입할 때 사용할 속도 임계값(km/h)입니다."))
	float IdleEnterSpeedThresholdKmh = 0.75f;

	// [v1.0.0] Idle 상태에서 빠져나올 때 사용할 속도 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Idle 이탈 속도 임계값 (IdleExitSpeedThresholdKmh)", ToolTip="Idle 상태에서 빠져나올 때 사용할 속도 임계값(km/h)입니다."))
	float IdleExitSpeedThresholdKmh = 1.5f;

	// [v1.0.0] Reversing 상태로 진입할 때 사용할 후진 속도 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Reversing 진입 속도 임계값 (ReverseEnterSpeedThresholdKmh)", ToolTip="Reversing 상태로 진입할 때 사용할 후진 속도 임계값(km/h)입니다."))
	float ReverseEnterSpeedThresholdKmh = 1.25f;

	// [v1.0.0] Reversing 상태에서 빠져나올 때 사용할 후진 속도 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="Reversing 이탈 속도 임계값 (ReverseExitSpeedThresholdKmh)", ToolTip="Reversing 상태에서 빠져나올 때 사용할 후진 속도 임계값(km/h)입니다."))
	float ReverseExitSpeedThresholdKmh = 0.75f;

	// [v1.0.0] 공중 상태 진입으로 볼 최소 속도 임계값(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="공중 상태 최소 속도 임계값 (AirborneMinSpeedThresholdKmh)", ToolTip="공중 상태 진입으로 볼 최소 속도 임계값(km/h)입니다."))
	float AirborneMinSpeedThresholdKmh = 3.0f;

	// [v1.0.0] 공중 상태 진입으로 볼 수직 속도 임계값(cm/s)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", DisplayName="공중 상태 수직 속도 임계값 (AirborneVerticalSpeedThresholdCmPerSec)", ToolTip="공중 상태 진입으로 볼 수직 속도 임계값(cm/s)입니다."))
	float AirborneVerticalSpeedThresholdCmPerSec = 100.0f;

	// [v1.0.0] 입력을 유효 입력으로 판정할 최소 절대값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="유효 입력 임계값 (ActiveInputThreshold)", ToolTip="입력을 유효 입력으로 판정할 최소 절대값입니다."))
	float ActiveInputThreshold = 0.05f;

	// [v1.0.0] 진행 방향 반대 스로틀을 제동 의도로 해석할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data|DriveState", meta=(DisplayName="반대 스로틀을 브레이크로 해석 (bTreatOppositeThrottleAsBrake)", ToolTip="True이면 진행 방향 반대 스로틀 입력을 제동 의도로 해석합니다."))
	bool bTreatOppositeThrottleAsBrake = true;
};
