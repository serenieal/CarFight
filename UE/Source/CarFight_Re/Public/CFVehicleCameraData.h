// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.1.0
// Date: 2026-04-10
// Description: CarFight 차량 카메라 DataAsset 정의
// Scope: 차량 공통 카메라 튜닝값과 기본 Aim Profile을 DataAsset으로 분리해 관리합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFVehicleCameraTypes.h"
#include "CFVehicleCameraData.generated.h"

/**
 * 차량 공통 카메라 튜닝값입니다.
 * - 피벗 오프셋
 * - 기본 Arm 길이 / FOV
 * - 속도 기반 보정
 * - 보간 속도
 * - 충돌 / Aim Trace
 * 를 묶어서 관리합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleCameraTuningConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="Pivot 로컬 오프셋 (PivotLocalOffset)", ToolTip="차량 기준 카메라 피벗에 추가로 적용할 로컬 오프셋입니다. XY는 차량 중심 미세 보정, Z는 높이 보정에 사용합니다."))
	FVector PivotLocalOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="기본 Arm 길이 (BaseArmLength)", ToolTip="차량 카메라 기본 Arm 길이입니다."))
	float BaseArmLength = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="기본 높이 오프셋 (BaseHeightOffset)", ToolTip="카메라 외부 시점을 구성할 기본 높이 오프셋입니다."))
	float BaseHeightOffset = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="기본 좌우 오프셋 (BaseSideOffset)", ToolTip="카메라 프레이밍용 기본 좌우 오프셋입니다. 양수는 오른쪽입니다."))
	float BaseSideOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="1.0", DisplayName="기본 FOV (BaseFOV)", ToolTip="차량 카메라 기본 FOV입니다."))
	float BaseFOV = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Yaw 입력 속도 (LookYawSpeedDegPerSec)", ToolTip="Look X 입력을 Yaw 변화량으로 변환할 초당 회전 속도(deg/s)입니다."))
	float LookYawSpeedDegPerSec = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Pitch 입력 속도 (LookPitchSpeedDegPerSec)", ToolTip="Look Y 입력을 Pitch 변화량으로 변환할 초당 회전 속도(deg/s)입니다."))
	float LookPitchSpeedDegPerSec = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="DeltaTime Look 스케일 사용 (bScaleLookInputByDeltaTime)", ToolTip="True이면 Look 입력을 DeltaTime 기준으로 스케일링합니다. Enhanced Input의 축 입력을 초당 회전량으로 해석할 때 사용합니다."))
	bool bScaleLookInputByDeltaTime = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Arm 길이 보간 속도 (ArmLengthInterpSpeed)", ToolTip="현재 Arm 길이를 목표 Arm 길이로 보간할 속도입니다."))
	float ArmLengthInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="FOV 보간 속도 (FOVInterpSpeed)", ToolTip="현재 FOV를 목표 FOV로 보간할 속도입니다."))
	float FOVInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="전투 Arm 길이 오프셋 (CombatArmLengthOffset)", ToolTip="Combat 상태에서 추가로 적용할 Arm 길이 오프셋입니다."))
	float CombatArmLengthOffset = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="전투 FOV 오프셋 (CombatFOVOffset)", ToolTip="Combat 상태에서 추가로 적용할 FOV 오프셋입니다."))
	float CombatFOVOffset = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="후진 Arm 길이 오프셋 (ReverseArmLengthOffset)", ToolTip="Reverse 상태에서 추가로 적용할 Arm 길이 오프셋입니다."))
	float ReverseArmLengthOffset = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="후진 FOV 오프셋 (ReverseFOVOffset)", ToolTip="Reverse 상태에서 추가로 적용할 FOV 오프셋입니다."))
	float ReverseFOVOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="공중 FOV 오프셋 (AirborneFOVOffset)", ToolTip="Airborne 상태에서 추가로 적용할 FOV 오프셋입니다."))
	float AirborneFOVOffset = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="속도 기반 FOV 사용 (bUseSpeedBasedFOV)", ToolTip="True이면 차량 속도에 따라 FOV를 확장합니다."))
	bool bUseSpeedBasedFOV = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="속도 기반 Arm 길이 사용 (bUseSpeedBasedArmLength)", ToolTip="True이면 차량 속도에 따라 Arm 길이를 확장합니다."))
	bool bUseSpeedBasedArmLength = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="최대 보정 속도 (SpeedForMaxBonusKmh)", ToolTip="속도 기반 FOV/Arm 길이 보정을 최대치까지 적용할 기준 속도(km/h)입니다."))
	float SpeedForMaxBonusKmh = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="최대 FOV 보너스 (MaxSpeedFOVBonus)", ToolTip="최대 보정 속도에 도달했을 때 추가할 최대 FOV 값입니다."))
	float MaxSpeedFOVBonus = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="최대 Arm 길이 보너스 (MaxSpeedArmLengthBonus)", ToolTip="최대 보정 속도에 도달했을 때 추가할 최대 Arm 길이 값입니다."))
	float MaxSpeedArmLengthBonus = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="Spring Arm 충돌 테스트 사용 (bEnableBoomCollisionTest)", ToolTip="True이면 Spring Arm의 기본 충돌 테스트를 사용합니다."))
	bool bEnableBoomCollisionTest = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="1.0", DisplayName="충돌 Probe 크기 (CollisionProbeSize)", ToolTip="Spring Arm 충돌 판정에 사용할 Probe 크기입니다."))
	float CollisionProbeSize = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="최소 Arm 길이 (MinArmLength)", ToolTip="충돌 또는 좁은 지형에서 허용할 최소 카메라 거리입니다."))
	float MinArmLength = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Aim Trace 길이 (AimTraceLength)", ToolTip="조준점 계산에 사용할 기본 Aim Trace 길이입니다."))
	float AimTraceLength = 50000.0f;
};

/**
 * 차량 카메라 기본 DataAsset입니다.
 * - 차량 공통 카메라 튜닝값
 * - 기본 Aim Profile
 * 을 함께 제공합니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleCameraData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="카메라 튜닝 설정 (CameraTuningConfig)", ToolTip="차량 공통 카메라 튜닝값입니다."))
	FCFVehicleCameraTuningConfig CameraTuningConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="기본 Aim Profile (DefaultAimProfile)", ToolTip="무기 시스템에서 별도 Aim Profile을 공급하지 못할 때 사용할 기본 조준 가능 범위입니다."))
	FCFVehicleCameraAimProfile DefaultAimProfile;
};
