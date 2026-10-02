// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.4.1
// Date: 2026-10-02
// Description: VCFX-P0-03 Camera collision boundary chatter 방지를 위한 collision release hysteresis/recovery tuning 추가
// Scope: 차량 공통 카메라 튜닝, normalized 주행 연출 Curve/Clamp/감쇠, 실제 차체 Motion 강도와 기본 Aim Profile을 DataAsset으로 관리합니다.
// Changelog:
// - v0.4.1: SpringArm 충돌 경계에서 hit/clear가 반복될 때 즉시 재확장하지 않도록 Clear hold, probe padding, recovery interpolation tuning을 추가.
// - v0.4.0: 기존 0~100% Speed Curve를 보존하면서 Reference 100% 이후에만 Overspeed Presentation headroom을 추가하고, Acceleration Rear Kick은 별도 Scale로 보강.
// - v0.3.0: USER feel feedback에 따라 Lateral Roll을 실제 차체 Roll/Yaw motion intensity로 조절하기 위한 보간 속도와 full-intensity threshold exact4를 추가.
// - v0.2.0: FCFVehicleDrivingFXConfig를 추가하고 Speed FOV/Arm, Accel/Brake Kick, Lateral Roll, 안전 Clamp와 상태 감쇠 Seed를 동결값으로 제공.
// Migration:
// - 새 Lateral body-motion 설정은 기존 normalized LateralRate 방향을 대체하지 않고 Presentation Roll 강도만 조절합니다.
// - bUseNormalizedDrivingFX 기본값은 false라 기존 DataAsset의 legacy speed FOV/Arm 동작을 그대로 유지합니다.
// - normalized 활성화는 차량별 ReferenceMaxSpeedKmh가 persisted 검증된 뒤에만 수행합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"
#include "CFVehicleCameraTypes.h"
#include "CFVehicleCameraData.generated.h"

/** 차량별 성능 비율을 화면 연출로 변환하는 normalized Driving FX 설정입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleDrivingFXConfig
{
	GENERATED_BODY()

	// [v0.2.0] 동결된 Curve Seed를 구성하는 기본 생성자입니다.
	FCFVehicleDrivingFXConfig()
	{
		SpeedFOVOffsetCurve.GetRichCurve()->AddKey(0.00f, 0.0f);
		SpeedFOVOffsetCurve.GetRichCurve()->AddKey(0.25f, 0.0f);
		SpeedFOVOffsetCurve.GetRichCurve()->AddKey(0.50f, 1.0f);
		SpeedFOVOffsetCurve.GetRichCurve()->AddKey(0.75f, 3.0f);
		SpeedFOVOffsetCurve.GetRichCurve()->AddKey(1.00f, 6.0f);

		SpeedArmOffsetCurve.GetRichCurve()->AddKey(0.00f, 0.0f);
		SpeedArmOffsetCurve.GetRichCurve()->AddKey(0.25f, 0.0f);
		SpeedArmOffsetCurve.GetRichCurve()->AddKey(0.50f, 10.0f);
		SpeedArmOffsetCurve.GetRichCurve()->AddKey(0.75f, 25.0f);
		SpeedArmOffsetCurve.GetRichCurve()->AddKey(1.00f, 45.0f);

		AccelerationArmKickCurve.GetRichCurve()->AddKey(0.00f, 0.0f);
		AccelerationArmKickCurve.GetRichCurve()->AddKey(0.05f, 0.0f);
		AccelerationArmKickCurve.GetRichCurve()->AddKey(0.15f, 12.0f);
		AccelerationArmKickCurve.GetRichCurve()->AddKey(0.30f, 20.0f);
		AccelerationArmKickCurve.GetRichCurve()->AddKey(1.00f, 25.0f);

		BrakingArmKickCurve.GetRichCurve()->AddKey(0.00f, 0.0f);
		BrakingArmKickCurve.GetRichCurve()->AddKey(0.05f, 0.0f);
		BrakingArmKickCurve.GetRichCurve()->AddKey(0.15f, 10.0f);
		BrakingArmKickCurve.GetRichCurve()->AddKey(0.30f, 18.0f);
		BrakingArmKickCurve.GetRichCurve()->AddKey(1.00f, 22.0f);

		LateralRollCurve.GetRichCurve()->AddKey(-1.00f, -2.5f);
		LateralRollCurve.GetRichCurve()->AddKey(-0.25f, -1.5f);
		LateralRollCurve.GetRichCurve()->AddKey(-0.10f, -0.75f);
		LateralRollCurve.GetRichCurve()->AddKey(-0.03f, 0.0f);
		LateralRollCurve.GetRichCurve()->AddKey(0.00f, 0.0f);
		LateralRollCurve.GetRichCurve()->AddKey(0.03f, 0.0f);
		LateralRollCurve.GetRichCurve()->AddKey(0.10f, 0.75f);
		LateralRollCurve.GetRichCurve()->AddKey(0.25f, 1.5f);
		LateralRollCurve.GetRichCurve()->AddKey(1.00f, 2.5f);
	}

	// 기존 Product의 legacy speed FOV/Arm과 신규 normalized Driving FX를 명시적으로 전환합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(DisplayName="정규화 주행 카메라 FX 사용 (bUseNormalizedDrivingFX)", ToolTip="True이면 차량별 ReferenceMaxSpeedKmh를 기준으로 신규 주행 카메라 FX를 사용합니다. False이면 기존 SpeedForMaxBonusKmh 기반 동작을 그대로 유지합니다."))
	bool bUseNormalizedDrivingFX = false;

	// 차량 기준속도 비율에 따른 추가 FOV Curve입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(DisplayName="속도 FOV 오프셋 커브 (SpeedFOVOffsetCurve)", ToolTip="0~MaxSpeedPresentationRatio SpeedRatio를 추가 FOV(deg)로 변환합니다. 1.0은 차량 ReferenceMaxSpeedKmh입니다."))
	FRuntimeFloatCurve SpeedFOVOffsetCurve;

	// 차량 기준속도 비율에 따른 추가 Arm 길이 Curve입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(DisplayName="속도 Arm 오프셋 커브 (SpeedArmOffsetCurve)", ToolTip="0~MaxSpeedPresentationRatio SpeedRatio를 추가 Arm 길이(cm)로 변환합니다. 1.0은 차량 ReferenceMaxSpeedKmh입니다."))
	FRuntimeFloatCurve SpeedArmOffsetCurve;

	// 정규화된 가속률을 뒤쪽 Camera Kick으로 변환하는 Curve입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(DisplayName="가속 Arm 킥 커브 (AccelerationArmKickCurve)", ToolTip="0~1 AccelerationRate를 카메라가 뒤로 물러나는 Arm 증가량(cm)으로 변환합니다."))
	FRuntimeFloatCurve AccelerationArmKickCurve;

	// 정규화된 제동률을 앞쪽 Camera Kick으로 변환하는 Curve입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(DisplayName="제동 Arm 킥 커브 (BrakingArmKickCurve)", ToolTip="0~1 BrakingRate를 카메라가 앞으로 당겨지는 Arm 감소량(cm)으로 변환합니다."))
	FRuntimeFloatCurve BrakingArmKickCurve;

	// signed 횡가속률을 화면 Roll로 변환하는 Curve입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(DisplayName="횡가속 롤 커브 (LateralRollCurve)", ToolTip="-1~1 LateralRate를 카메라 Roll(deg)로 변환합니다."))
	FRuntimeFloatCurve LateralRollCurve;

	// ReferenceMaxSpeedKmh=1.0 이후에도 Speed FOV/Arm이 계속 진행할 수 있는 Presentation ratio 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Speed", meta=(ClampMin="1.0", ClampMax="3.0", DisplayName="최대 속도 표현 비율 (MaxSpeedPresentationRatio)", ToolTip="1.0은 차량 ReferenceMaxSpeedKmh입니다. 1.5이면 Reference의 150%까지 Speed FOV/Arm Curve가 계속 진행됩니다. 차량 물리 최고속도나 제한속도는 바꾸지 않습니다."))
	float MaxSpeedPresentationRatio = 1.5f;

	// MaxSpeedPresentationRatio 도달 시 100% Speed Curve 출력에 추가할 FOV headroom입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Speed", meta=(ClampMin="0.0", Units="deg", DisplayName="최대 초과속도 FOV 보너스 (MaxOverspeedFOVBonusDeg)", ToolTip="ReferenceMaxSpeedKmh 초과 구간에서만 선형으로 추가되는 FOV입니다. 0~100% 기존 SpeedFOVOffsetCurve는 변경하지 않습니다."))
	float MaxOverspeedFOVBonusDeg = 4.0f;

	// MaxSpeedPresentationRatio 도달 시 100% Speed Curve 출력에 추가할 Arm headroom입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Speed", meta=(ClampMin="0.0", Units="cm", DisplayName="최대 초과속도 Arm 보너스 (MaxOverspeedArmBonusCm)", ToolTip="ReferenceMaxSpeedKmh 초과 구간에서만 선형으로 추가되는 Arm 길이입니다. 0~100% 기존 SpeedArmOffsetCurve는 변경하지 않습니다."))
	float MaxOverspeedArmBonusCm = 30.0f;

	// 가속 Rear Kick Curve 출력에 적용할 Presentation 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Acceleration", meta=(ClampMin="0.0", ClampMax="3.0", DisplayName="가속 Rear Kick 배율 (AccelerationRearKickScale)", ToolTip="AccelerationArmKickCurve 출력에 곱하는 연출 배율입니다. 물리 가속도에는 영향을 주지 않습니다."))
	float AccelerationRearKickScale = 1.25f;

	// Accel/Brake/Lateral normalized signal 보간 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX", meta=(ClampMin="0.0", DisplayName="운동률 보간 속도 (MotionRateInterpSpeed)", ToolTip="가속·제동·횡가속 normalized 입력이 목표값을 따라가는 보간 속도입니다."))
	float MotionRateInterpSpeed = 10.0f;

	// 실제 차체 Motion intensity가 목표값을 따라가는 보간 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Lateral", meta=(ClampMin="0.0", DisplayName="횡방향 차체 Motion 보간 속도 (LateralBodyMotionInterpSpeed)", ToolTip="차체 Roll 각도와 Roll/Yaw 각속도에서 계산한 Lateral Camera Motion intensity가 목표값을 따라가는 속도입니다."))
	float LateralBodyMotionInterpSpeed = 6.0f;

	// 이 차체 Roll 각도에서 Lateral body-motion intensity를 1로 보는 기준입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Lateral", meta=(ClampMin="0.01", Units="deg", DisplayName="횡방향 최대강도 차체 Roll 각도 (LateralBodyRollAngleForFullIntensityDeg)", ToolTip="차량 차체의 절대 Roll 각도가 이 값에 도달하면 Roll-angle 기반 Motion intensity가 1이 됩니다. 카메라가 차체 Roll을 직접 따라가지는 않습니다."))
	float LateralBodyRollAngleForFullIntensityDeg = 7.5f;

	// 이 차체 Roll 각속도에서 Lateral body-motion intensity를 1로 보는 기준입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Lateral", meta=(ClampMin="0.01", Units="deg/s", DisplayName="횡방향 최대강도 차체 Roll 각속도 (LateralBodyRollRateForFullIntensityDegPerSec)", ToolTip="차량 차체의 절대 Roll 각속도가 이 값에 도달하면 Roll-rate 기반 Motion intensity가 1이 됩니다."))
	float LateralBodyRollRateForFullIntensityDegPerSec = 45.0f;

	// 이 차체 Yaw 각속도에서 Lateral body-motion intensity를 1로 보는 기준입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Lateral", meta=(ClampMin="0.01", Units="deg/s", DisplayName="횡방향 최대강도 차체 Yaw 각속도 (LateralBodyYawRateForFullIntensityDegPerSec)", ToolTip="차량 차체의 절대 Yaw 각속도가 이 값에 도달하면 Yaw-rate 기반 Motion intensity가 1이 됩니다."))
	float LateralBodyYawRateForFullIntensityDegPerSec = 90.0f;

	// Speed FOV Curve 출력의 절대 안전 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="0.0", Units="deg", DisplayName="최대 속도 FOV 오프셋 (MaxSpeedFOVOffsetDeg)"))
	float MaxSpeedFOVOffsetDeg = 12.0f;

	// Speed Arm Curve 출력의 절대 안전 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="0.0", Units="cm", DisplayName="최대 속도 Arm 오프셋 (MaxSpeedArmOffsetCm)"))
	float MaxSpeedArmOffsetCm = 120.0f;

	// 가속 Rear Kick 출력의 절대 안전 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="0.0", Units="cm", DisplayName="최대 가속 Arm 킥 (MaxAccelerationArmKickCm)"))
	float MaxAccelerationArmKickCm = 35.0f;

	// 제동 Forward Kick 출력의 절대 안전 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="0.0", Units="cm", DisplayName="최대 제동 Arm 킥 (MaxBrakingArmKickCm)"))
	float MaxBrakingArmKickCm = 35.0f;

	// 횡가속 Roll 출력의 절대 안전 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="0.0", Units="deg", DisplayName="최대 횡가속 롤 (MaxLateralRollDeg)"))
	float MaxLateralRollDeg = 3.0f;

	// 최종 Presentation FOV 하한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="1.0", Units="deg", DisplayName="최소 표현 FOV (MinPresentationFOVDeg)"))
	float MinPresentationFOVDeg = 60.0f;

	// 최종 Presentation FOV 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="1.0", Units="deg", DisplayName="최대 표현 FOV (MaxPresentationFOVDeg)"))
	float MaxPresentationFOVDeg = 120.0f;

	// 최종 Presentation Arm 길이 상한입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Clamp", meta=(ClampMin="0.0", Units="cm", DisplayName="최대 표현 Arm 길이 (MaxPresentationArmLengthCm)"))
	float MaxPresentationArmLengthCm = 900.0f;

	// Combat 상태의 Speed FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="전투 속도 FX 배율 (CombatSpeedFXScale)"))
	float CombatSpeedFXScale = 0.50f;

	// Combat 상태의 Motion FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="전투 운동 FX 배율 (CombatMotionFXScale)"))
	float CombatMotionFXScale = 0.25f;

	// 명시적 Aim Presentation 상태의 Speed FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="조준 속도 FX 배율 (AimSpeedFXScale)"))
	float AimSpeedFXScale = 0.50f;

	// 명시적 Aim Presentation 상태의 Motion FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="조준 운동 FX 배율 (AimMotionFXScale)"))
	float AimMotionFXScale = 0.20f;

	// Airborne 상태의 Speed FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="공중 속도 FX 배율 (AirborneSpeedFXScale)"))
	float AirborneSpeedFXScale = 0.25f;

	// Airborne 상태의 Motion FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="공중 운동 FX 배율 (AirborneMotionFXScale)"))
	float AirborneMotionFXScale = 0.00f;

	// Reverse 상태의 Speed FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="후진 속도 FX 배율 (ReverseSpeedFXScale)"))
	float ReverseSpeedFXScale = 0.50f;

	// Reverse 상태의 Motion FX 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Driving FX|Attenuation", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="후진 운동 FX 배율 (ReverseMotionFXScale)"))
	float ReverseMotionFXScale = 0.35f;
};

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
	float BaseArmLength = 560.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="기본 높이 오프셋 (BaseHeightOffset)", ToolTip="카메라 외부 시점을 구성할 기본 높이 오프셋입니다."))
	float BaseHeightOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="기본 좌우 오프셋 (BaseSideOffset)", ToolTip="카메라 프레이밍용 기본 좌우 오프셋입니다. 양수는 오른쪽입니다."))
	float BaseSideOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="1.0", DisplayName="기본 FOV (BaseFOV)", ToolTip="차량 카메라 기본 FOV입니다."))
	float BaseFOV = 87.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Yaw 입력 속도 (LookYawSpeedDegPerSec)", ToolTip="Look X 입력을 Yaw 변화량으로 변환할 초당 회전 속도(deg/s)입니다."))
	float LookYawSpeedDegPerSec = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="Pitch 입력 속도 (LookPitchSpeedDegPerSec)", ToolTip="Look Y 입력을 Pitch 변화량으로 변환할 초당 회전 속도(deg/s)입니다."))
	float LookPitchSpeedDegPerSec = 75.0f;

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
	float MaxSpeedFOVBonus = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="최대 Arm 길이 보너스 (MaxSpeedArmLengthBonus)", ToolTip="최대 보정 속도에 도달했을 때 추가할 최대 Arm 길이 값입니다."))
	float MaxSpeedArmLengthBonus = 45.0f;

	// [v0.2.0] 차량별 기준속도 기반 신규 Driving FX 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="주행 카메라 FX 설정 (DrivingFXConfig)", ToolTip="차량별 ReferenceMaxSpeedKmh를 사용하는 FOV/Arm/Kick/Roll 연출과 안전 Clamp·상태 감쇠 설정입니다."))
	FCFVehicleDrivingFXConfig DrivingFXConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="Spring Arm 충돌 테스트 사용 (bEnableBoomCollisionTest)", ToolTip="True이면 Spring Arm의 기본 충돌 테스트를 사용합니다."))
	bool bEnableBoomCollisionTest = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="1.0", DisplayName="충돌 Probe 크기 (CollisionProbeSize)", ToolTip="Spring Arm 충돌 판정에 사용할 Probe 크기입니다."))
	float CollisionProbeSize = 12.0f;

	// 충돌에서 벗어난 뒤 복귀를 시작하기 전에 연속으로 Clear 상태를 유지해야 하는 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Collision", meta=(ClampMin="0.0", ClampMax="1.0", Units="s", DisplayName="충돌 해제 확인 시간 (CollisionReleaseHoldTimeSec)", ToolTip="충돌 경계에서 한두 프레임 Clear가 발생해도 즉시 카메라를 늘리지 않고, 이 시간 동안 연속 Clear가 확인된 뒤 복귀를 시작합니다."))
	float CollisionReleaseHoldTimeSec = 0.10f;

	// 충돌 복귀 중 경계 흔들림을 줄이기 위해 Clearance 검사 Probe에 추가하는 여유 반경입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Collision", meta=(ClampMin="0.0", ClampMax="20.0", Units="cm", DisplayName="충돌 해제 Probe 여유 (CollisionReleaseProbePaddingCm)", ToolTip="충돌 복귀 상태에서는 기본 Probe보다 이 값만큼 큰 Sphere Sweep으로 Clear를 확인해 경계면 hit/clear 반복을 줄입니다."))
	float CollisionReleaseProbePaddingCm = 4.0f;

	// 충돌이 완전히 해제된 뒤 카메라 Arm이 원래 길이로 복귀하는 보간 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera|Collision", meta=(ClampMin="0.0", DisplayName="충돌 복귀 Arm 보간 속도 (CollisionRecoveryInterpSpeed)", ToolTip="충돌이 안정적으로 해제된 뒤 압축된 카메라 Arm이 Presentation 목표 길이로 부드럽게 복귀하는 속도입니다."))
	float CollisionRecoveryInterpSpeed = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="최소 Arm 길이 (MinArmLength)", ToolTip="충돌 또는 좁은 지형에서 허용할 최소 카메라 거리입니다."))
	float MinArmLength = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(DisplayName="충돌 시 시야 보조 사용 (bUseCollisionViewAssist)", ToolTip="True이면 충돌로 카메라가 크게 압축될 때 높이/FOV 보조를 적용해 가시성을 보완합니다."))
	bool bUseCollisionViewAssist = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.1", ClampMax="1.0", DisplayName="충돌 시야 보조 시작 비율 (CollisionViewAssistStartRatio)", ToolTip="실제 카메라 거리 비율이 이 값보다 작아지면 충돌 시야 보조를 시작합니다. 1.0에 가까울수록 더 일찍 보조가 들어갑니다."))
	float CollisionViewAssistStartRatio = 0.82f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="최대 충돌 높이 보조 (MaxCollisionHeightAssist)", ToolTip="충돌로 카메라가 당겨졌을 때 추가할 최대 높이 보조량입니다."))
	float MaxCollisionHeightAssist = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Camera", meta=(ClampMin="0.0", DisplayName="최대 충돌 FOV 보조 (MaxCollisionFOVAssist)", ToolTip="충돌로 카메라가 당겨졌을 때 추가할 최대 FOV 보조량입니다."))
	float MaxCollisionFOVAssist = 3.0f;

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
