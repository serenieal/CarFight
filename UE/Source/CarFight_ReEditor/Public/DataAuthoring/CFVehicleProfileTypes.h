// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleProfileTypes.h
// Version: v1.2.0
// Date: 2026-08-26
// Description: CF-FQ-040 VB-P0-05 Builder-private ownership, reference wheel geometry와 typed Transmission payload를 포함한 Profile 계약입니다.
// Scope: Profile metadata, Feel response, mass context와 Domain별 balance payload를 제공합니다.
// Changelog:
// - v1.2.0: Builder-private OwnerRecipeId, opt-in Reference Wheel Geometry, ChassisWidth와 UE 5.8 Transmission complete payload를 additive 추가.
// - v1.1.0: UI-P0-06 explicit RedlineStartRPM을 PerformanceProfileDirect typed payload로 additive 추가. EngineMaxRPMByFeel에서 자동 유도하지 않음.
// - v1.0.0: DAUTH-P0-08A Profile schema를 최초 구현.
// Migration:
// - v1.2.0 기존 Profile은 OwnerRecipeId invalid로 shared/legacy이며 Builder-private commit 대상이 아닙니다.
// - 기존 VehicleBase Profile은 bUseReferenceWheelGeometry=false, 기존 Drivetrain Profile은 bUseTransmissionConfig=false라 새 fallback/Transmission candidate를 자동 생성하지 않습니다.
// - Builder-private Profile은 소유 RecipeId를 명시하고 complete payload를 commit할 때 opt-in gate를 함께 설정합니다.
// - RedlineStartRPM 기본값 0은 미설정이며 기존 Profile/VehicleData 수치를 자동 채우지 않습니다.
// - Profile C++ 기본값을 기존 VehicleData 숫자표 복사본으로 사용하지 않습니다.
// - 실제 balance 값은 후속 Authoring Profile Asset에서 명시적으로 작성합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleData.h"
#include "CFVehicleProfileTypes.generated.h"

class UCFCombatFxData;
class UCFVehicleDefenseData;

/** 5개 flat Profile이 composition으로 공유하는 Editor Authoring metadata입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleProfileMeta
{
	GENERATED_BODY()

	// Authoring UI에 표시할 Profile 이름입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="표시 이름"))
	FText DisplayName;

	// 이 Profile의 의도와 사용 범위를 설명합니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(MultiLine="true", DisplayName="설명"))
	FString Description;

	// Builder-private Profile을 독점 소유하는 RecipeId입니다. Invalid GUID는 기존 shared/legacy Profile을 뜻합니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="Builder 소유 Recipe ID", ToolTip="Guided Vehicle Builder가 차량 전용으로 만든 Profile의 소유 RecipeId입니다. 비어 있으면 기존 shared/legacy Profile이며 Builder-private commit으로 수정할 수 없습니다."))
	FGuid OwnerRecipeId;

	// 사람이 수행한 Profile Authoring edit의 diagnostic revision입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="Authoring Revision"))
	int32 AuthoringRevision = 0;
};

/** 0 / 0.5 / 1.0 Feel intent를 piecewise-linear raw value로 변환하는 Profile response입니다. */
USTRUCT(BlueprintType)
struct FCFFeelResponse
{
	GENERATED_BODY()

	// Feel 0.0에서 사용할 authored raw value입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="낮음 값"))
	float LowValue = 0.0f;

	// Feel 0.5에서 사용할 authored neutral raw value입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="중립 값"))
	float NeutralValue = 0.0f;

	// Feel 1.0에서 사용할 authored raw value입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="높음 값"))
	float HighValue = 0.0f;
};

/** Profile이 명시적으로 선택한 경우에만 적용되는 질량 context scale 규칙입니다. */
USTRUCT(BlueprintType)
struct FCFMassScaleRule
{
	GENERATED_BODY()

	// 질량 context scaling을 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="질량 스케일 사용"))
	bool bEnabled = false;

	// MassScale 분모로 사용할 authored reference mass kg입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(ClampMin="0.0", Units="kg", DisplayName="기준 질량 kg"))
	float ReferenceMassKg = 0.0f;

	// Pow(BaseMass/ReferenceMass, exponent)에 사용할 authored exponent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="질량 지수"))
	float MassExponent = 0.0f;
};

/** Vehicle Base Profile이 소유하는 baseline payload입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleBaseProfileData
{
	GENERATED_BODY()

	// 장비/탄약/방어를 더하기 전 차량 플랫폼 기준 질량 kg입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base", meta=(ClampMin="0.0", Units="kg"))
	float BaseVehicleMassKg = 0.0f;

	// 장비를 포함한 최대 허용 총중량 kg입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base", meta=(ClampMin="0.0", Units="kg"))
	float MaximumGrossMassKg = 0.0f;

	// VehicleDurabilityConfig.MaxHealth baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base", meta=(ClampMin="0.0"))
	float MaxHealth = 0.0f;

	// VehicleMovementConfig.ChassisWidth baseline입니다. UE 5.8 기본 180cm와 동일한 additive 기본값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base", meta=(ClampMin="0.01", Units="cm", DisplayName="차체 폭 cm", ToolTip="VehicleMovementConfig.ChassisWidth에 제공할 차체 폭입니다. UE 5.8 기본값 180cm를 additive 기본값으로 사용합니다."))
	float ChassisWidth = 180.0f;

	// VehicleMovementConfig.ChassisHeight baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	float ChassisHeight = 0.0f;

	// 실차 Reference wheel geometry를 Asset measurement보다 낮은 precedence의 fallback으로 제공할지 선택합니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base|Wheel Geometry", meta=(DisplayName="Reference 휠 치수 사용", ToolTip="True이면 이 VehicleBase Profile의 전/후륜 반지름·폭을 VehicleData 후보로 제공합니다. 채택된 실제 Wheel Mesh 측정값이 있으면 그 값이 더 높은 precedence로 우선합니다."))
	bool bUseReferenceWheelGeometry = false;

	// Reference 기반 전륜 반지름 cm fallback입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base|Wheel Geometry", meta=(EditCondition="bUseReferenceWheelGeometry", ClampMin="0.0", Units="cm", DisplayName="Reference 전륜 반지름 cm"))
	float FrontWheelRadius = 30.0f;

	// Reference 기반 후륜 반지름 cm fallback입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base|Wheel Geometry", meta=(EditCondition="bUseReferenceWheelGeometry", ClampMin="0.0", Units="cm", DisplayName="Reference 후륜 반지름 cm"))
	float RearWheelRadius = 30.0f;

	// Reference 기반 전륜 폭 cm fallback입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base|Wheel Geometry", meta=(EditCondition="bUseReferenceWheelGeometry", ClampMin="0.0", Units="cm", DisplayName="Reference 전륜 폭 cm"))
	float FrontWheelWidth = 6.0f;

	// Reference 기반 후륜 폭 cm fallback입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base|Wheel Geometry", meta=(EditCondition="bUseReferenceWheelGeometry", ClampMin="0.0", Units="cm", DisplayName="Reference 후륜 폭 cm"))
	float RearWheelWidth = 6.0f;

	// Profile이 제공할 기본 VehicleDefenseData입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	TSoftObjectPtr<UCFVehicleDefenseData> DefaultDefenseData;

	// Profile이 제공할 기본 Destroyed CombatFxData입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	TSoftObjectPtr<UCFCombatFxData> DefaultDestroyedFxData;

	// WheelVisualConfig.ExpectedWheelCount baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base", meta=(ClampMin="0"))
	int32 ExpectedWheelCount = 0;

	// WheelVisualConfig.FrontWheelCountForSteering baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base", meta=(ClampMin="0"))
	int32 FrontWheelCountForSteering = 0;

	// Profile의 기본 wheel mesh auto-scale policy입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	bool bAutoScaleWheelMeshToRadius = false;

	// Profile의 기본 wheel mesh radius 측정 모드입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	ECFWheelMeshRadiusMeasureMode WheelMeshRadiusMeasureMode = ECFWheelMeshRadiusMeasureMode::AutoMaxXZ;

	// Profile의 기본 wheel bounds auto-center policy입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	bool bAutoCenterWheelMeshBoundsToOrigin = false;

	// Profile의 wheel mesh auto-scale 최소값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	float WheelMeshScaleClampMin = 0.0f;

	// Profile의 wheel mesh auto-scale 최대값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Base")
	float WheelMeshScaleClampMax = 0.0f;
};

/** Drivetrain Profile이 소유하는 typed payload입니다. */
USTRUCT(BlueprintType)
struct FCFDrivetrainProfileData
{
	GENERATED_BODY()

	// DifferentialSetup.DifferentialType baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain")
	EVehicleDifferential DifferentialType = EVehicleDifferential::RearWheelDrive;

	// DifferentialSetup.FrontRearSplit baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrontRearSplit = 0.0f;

	// 이 Drivetrain Profile이 차량별 TransmissionSetup complete payload를 제공할지 선택합니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(DisplayName="차량별 변속기 설정 사용", ToolTip="True이면 이 Profile의 UE 5.8 Transmission complete payload를 VehicleData 후보로 제공합니다. 기존 shared Profile은 기본 False라 현재 차량에 새 변속기 값을 주입하지 않습니다."))
	bool bUseTransmissionConfig = false;

	// Chaos Transmission의 automatic control mode 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", DisplayName="자동 변속 사용"))
	bool bUseAutomaticGears = true;

	// Chaos AutoReverse 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", DisplayName="자동 후진 전환 사용"))
	bool bUseAutoReverse = true;

	// 전진/후진 기어비 complete typed ratio-set입니다. ReverseGearRatios는 positive magnitude만 저장합니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", DisplayName="변속기 기어비", ToolTip="전진/후진 기어비를 순서대로 저장합니다. 후진 기어비도 양수 크기만 저장하며 UE 5.8 GetGearRatio가 역방향 부호를 적용합니다."))
	FCFVehicleTransmissionRatios TransmissionRatios;

	// 각 기어비에 곱하는 최종 감속비입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", ClampMin="0.0001", DisplayName="최종 감속비"))
	float FinalRatio = 3.08f;

	// 자동 상향 변속 RPM입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", ClampMin="0.0", Units="rpm", DisplayName="상향 변속 RPM"))
	float ChangeUpRPM = 4500.0f;

	// 자동 하향 변속 RPM입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", ClampMin="0.0", Units="rpm", DisplayName="하향 변속 RPM"))
	float ChangeDownRPM = 2000.0f;

	// 기어 전환 시간 초입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", ClampMin="0.0", Units="s", DisplayName="변속 시간"))
	float GearChangeTime = 0.4f;

	// 변속기 기계 효율 배율입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain|Transmission", meta=(EditCondition="bUseTransmissionConfig", ClampMin="0.0", ClampMax="1.0", DisplayName="변속 효율"))
	float TransmissionEfficiency = 0.9f;

	// 전륜이 엔진 구동 영향을 받는지 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain")
	bool bFrontWheelAffectedByEngine = false;

	// 후륜이 엔진 구동 영향을 받는지 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain")
	bool bRearWheelAffectedByEngine = false;

	// Definition의 전륜 Wheel Class가 될 soft class reference입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain")
	TSoftClassPtr<UChaosVehicleWheel> FrontWheelClass;

	// Definition의 후륜 Wheel Class가 될 soft class reference입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Drivetrain")
	TSoftClassPtr<UChaosVehicleWheel> RearWheelClass;
};

/** Handling Profile이 소유하는 Feel response와 direct baseline payload입니다. */
USTRUCT(BlueprintType)
struct FCFHandlingProfileData
{
	GENERATED_BODY()

	// SteeringAgility로 FrontWheelMaxSteerAngle을 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse FrontWheelMaxSteerAngleByFeel;

	// SteeringAgility로 SteeringAngleRatio를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse SteeringAngleRatioByFeel;

	// GripFeel로 전륜 FrictionForceMultiplier를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse FrontWheelFrictionByFeel;

	// GripFeel로 후륜 FrictionForceMultiplier를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse RearWheelFrictionByFeel;

	// GripFeel로 전륜 CorneringStiffness를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse FrontCorneringByFeel;

	// GripFeel로 후륜 CorneringStiffness를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse RearCorneringByFeel;

	// SuspensionFirmness로 전륜 SpringRate를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse FrontSpringRateByFeel;

	// SuspensionFirmness로 후륜 SpringRate를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse RearSpringRateByFeel;

	// SuspensionFirmness로 전륜 SpringPreload를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse FrontSpringPreloadByFeel;

	// SuspensionFirmness로 후륜 SpringPreload를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFFeelResponse RearSpringPreloadByFeel;

	// Suspension response에 선택적으로 적용할 mass context rule입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	FCFMassScaleRule SuspensionMassScale;

	// 전륜 최대 브레이크 토크 baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float FrontWheelMaxBrakeTorque = 0.0f;

	// 후륜 최대 브레이크 토크 baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float RearWheelMaxBrakeTorque = 0.0f;

	// 후륜 최대 핸드브레이크 토크 baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float RearWheelMaxHandBrakeTorque = 0.0f;

	// 전륜 WheelLoadRatio baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float FrontWheelLoadRatio = 0.0f;

	// 후륜 WheelLoadRatio baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float RearWheelLoadRatio = 0.0f;

	// 전륜 SuspensionMaxRaise baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float FrontWheelSuspensionMaxRaise = 0.0f;

	// 후륜 SuspensionMaxRaise baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float RearWheelSuspensionMaxRaise = 0.0f;

	// 전륜 SuspensionMaxDrop baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float FrontWheelSuspensionMaxDrop = 0.0f;

	// 후륜 SuspensionMaxDrop baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	float RearWheelSuspensionMaxDrop = 0.0f;

	// 전륜 Wheel SweepShape baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	ESweepShape FrontWheelSweepShape = ESweepShape::Raycast;

	// 후륜 Wheel SweepShape baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	ESweepShape RearWheelSweepShape = ESweepShape::Raycast;

	// SteeringSetup.SteeringType baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Handling")
	ESteeringType SteeringType = ESteeringType::AngleRatio;
};

/** Performance Profile이 소유하는 acceleration Feel response와 direct baseline payload입니다. */
USTRUCT(BlueprintType)
struct FCFPerformanceProfileData
{
	GENERATED_BODY()

	// AccelerationFeel로 EngineMaxTorque를 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	FCFFeelResponse EngineMaxTorqueByFeel;

	// AccelerationFeel로 EngineMaxRPM을 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	FCFFeelResponse EngineMaxRPMByFeel;

	// AccelerationFeel로 ThrottleInputScale을 만드는 response입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	FCFFeelResponse ThrottleInputScaleByFeel;

	// EngineMaxTorque response에 선택적으로 적용할 mass context rule입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	FCFMassScaleRule TorqueMassScale;

		// EngineIdleRPM baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	float EngineIdleRPM = 0.0f;

	// [v1.1.0] HUD Tachometer용 explicit RedlineStartRPM baseline입니다. 0은 미설정이며 EngineMaxRPM response에서 자동 유도하지 않습니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance", meta=(ClampMin="0.0", Units="rpm", DisplayName="레드라인 시작 RPM", ToolTip="차량별 HUD Tachometer의 실제 레드라인 시작 RPM입니다. 0은 미설정이며 EngineMaxRPM이나 변속 RPM에서 자동 추정하지 않습니다."))
	float RedlineStartRPM = 0.0f;

	// EngineBrakeEffect baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	float EngineBrakeEffect = 0.0f;

	// EngineRevUpMOI baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	float EngineRevUpMOI = 0.0f;

	// EngineRevDownRate baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	float EngineRevDownRate = 0.0f;

	// VehicleMovement DragCoefficient baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	float DragCoefficient = 0.0f;

	// VehicleMovement DownforceCoefficient baseline입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Performance")
	float DownforceCoefficient = 0.0f;
};

/** Runtime gate를 제외한 DriveState Profile behavior 14개 field입니다. */
USTRUCT(BlueprintType)
struct FCFDriveStateProfileData
{
	GENERATED_BODY()

	// DriveState hysteresis 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	bool bEnableDriveStateHysteresis = false;

	// 상태별 hold time 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	bool bUsePerStateHoldTimes = false;

	// 기본 DriveState 최소 유지 시간입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float DriveStateMinimumHoldTimeSeconds = 0.0f;

	// Idle 최소 유지 시간입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float IdleStateMinimumHoldTimeSeconds = 0.0f;

	// Reversing 최소 유지 시간입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float ReversingStateMinimumHoldTimeSeconds = 0.0f;

	// Airborne 최소 유지 시간입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float AirborneStateMinimumHoldTimeSeconds = 0.0f;

	// Idle 진입 속도 임계값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float IdleEnterSpeedThresholdKmh = 0.0f;

	// Idle 이탈 속도 임계값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float IdleExitSpeedThresholdKmh = 0.0f;

	// Reversing 진입 속도 임계값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float ReverseEnterSpeedThresholdKmh = 0.0f;

	// Reversing 이탈 속도 임계값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float ReverseExitSpeedThresholdKmh = 0.0f;

	// Airborne 진입 최소 속도 임계값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float AirborneMinSpeedThresholdKmh = 0.0f;

	// Airborne 진입 수직 속도 임계값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	float AirborneVerticalSpeedThresholdCmPerSec = 0.0f;

	// 유효 입력 최소 절대값입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ActiveInputThreshold = 0.0f;

	// 진행방향 반대 throttle을 brake intent로 볼지 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|DriveState")
	bool bTreatOppositeThrottleAsBrake = false;
};
