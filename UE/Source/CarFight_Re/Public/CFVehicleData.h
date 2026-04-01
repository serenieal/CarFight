// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.9.2
// Date: 2026-04-01
// Description: CarFight 차량 루트 DataAsset 최소 확장안 + AutoFit 레거시 가시성 축소
// Scope: 차량 시각 자산, 기본 레이아웃, Wheel Class 참조, Movement/WheelVisual 최소 슬롯을 Native 타입으로 고정합니다.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/DataAsset.h"
#include "CFVehicleDriveStateConfig.h"
#include "CFVehicleData.generated.h"

class UChaosVehicleWheel;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FCFVehicleWheelLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Legacy|AutoFit", meta=(ToolTip="레거시 AutoFit 사용 여부 필드입니다. 현재 런타임에서는 사용하지 않으며 수동 Wheel Anchor 배치를 기준으로 삼습니다."))
	bool bUseAutoFit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Legacy|AutoFit", meta=(ToolTip="레거시 AutoFit 전륜 축 여백 필드입니다. 현재 런타임에서는 사용하지 않습니다."))
	float FrontAxleMargin = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Legacy|AutoFit", meta=(ToolTip="레거시 AutoFit 후륜 축 여백 필드입니다. 현재 런타임에서는 사용하지 않습니다."))
	float RearAxleMargin = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Legacy|AutoFit", meta=(ToolTip="레거시 AutoFit 높이 오프셋 필드입니다. 현재 런타임에서는 사용하지 않습니다."))
	float HeightOffset = 10.0f;
};

USTRUCT(BlueprintType)
struct FCFVehicleVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 차체 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> ChassisMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="앞왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> WheelMeshFL = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="앞오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshFR = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="뒤왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRL = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="뒤오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRR = nullptr;
};

USTRUCT(BlueprintType)
struct FCFVehicleLayoutConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Legacy|AutoFit", meta=(ToolTip="레거시 AutoFit 레이아웃 데이터입니다. 현재 런타임에서는 사용하지 않습니다."))
	FCFVehicleWheelLayout WheelLayout;
};

USTRUCT(BlueprintType)
struct FCFVehicleMovementConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 이 차량이 별도 VehicleMovement 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseMovementOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="현재 단계에서는 실제 수치 대신 식별용 이름 또는 메모성 구분자로 사용합니다."))
	FName MovementProfileName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 MaxSteerAngle을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelMaxSteerAngle = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 조향각(deg)입니다."))
	float FrontWheelMaxSteerAngle = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 MaxBrakeTorque를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelMaxBrakeTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float FrontWheelMaxBrakeTorque = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 MaxBrakeTorque를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelMaxBrakeTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float RearWheelMaxBrakeTorque = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 MaxHandBrakeTorque를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelMaxHandBrakeTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 핸드브레이크 토크입니다."))
	float RearWheelMaxHandBrakeTorque = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 WheelRadius를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelRadius = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float FrontWheelRadius = 39.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 WheelRadius를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelRadius = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float RearWheelRadius = 39.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 WheelWidth를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelWidth = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float FrontWheelWidth = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 WheelWidth를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelWidth = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float RearWheelWidth = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 FrictionForceMultiplier를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelFrictionForceMultiplier = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float FrontWheelFrictionForceMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 FrictionForceMultiplier를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelFrictionForceMultiplier = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float RearWheelFrictionForceMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 CorneringStiffness를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelCorneringStiffness = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float FrontWheelCorneringStiffness = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 CorneringStiffness를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelCorneringStiffness = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float RearWheelCorneringStiffness = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 WheelLoadRatio를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelLoadRatio = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float FrontWheelLoadRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 WheelLoadRatio를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelLoadRatio = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float RearWheelLoadRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 SpringRate를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSpringRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float FrontWheelSpringRate = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 SpringRate를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSpringRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float RearWheelSpringRate = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 SpringPreload를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSpringPreload = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float FrontWheelSpringPreload = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 SpringPreload를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSpringPreload = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float RearWheelSpringPreload = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 SuspensionMaxRaise를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSuspensionMaxRaise = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float FrontWheelSuspensionMaxRaise = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 SuspensionMaxRaise를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSuspensionMaxRaise = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float RearWheelSuspensionMaxRaise = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 SuspensionMaxDrop를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSuspensionMaxDrop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float FrontWheelSuspensionMaxDrop = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 SuspensionMaxDrop를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSuspensionMaxDrop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float RearWheelSuspensionMaxDrop = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 엔진 영향 여부를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="전륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bFrontWheelAffectedByEngine = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 엔진 영향 여부를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="후륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bRearWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 Wheel Class 기본값의 SweepShape를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSweepShape = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="전륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape FrontWheelSweepShape = ESweepShape::Shapecast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 Wheel Class 기본값의 SweepShape를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSweepShape = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="후륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape RearWheelSweepShape = ESweepShape::Shapecast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 차체 높이를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideChassisHeight = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovementComp의 ChassisHeight 값입니다."))
	float ChassisHeight = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 공기 저항 계수를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideDragCoefficient = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovementComp의 DragCoefficient 값입니다."))
	float DragCoefficient = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 다운포스 계수를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideDownforceCoefficient = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovementComp의 DownforceCoefficient 값입니다."))
	float DownforceCoefficient = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 중심질량 오버라이드 사용 여부를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEnableCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovementComp의 중심질량 오버라이드 사용 여부입니다."))
	bool bEnableCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 중심질량 오프셋을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovementComp의 CenterOfMassOverride 값입니다."))
	FVector CenterOfMassOverride = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 엔진 최대 토크를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineMaxTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="EngineSetup.MaxTorque 값입니다."))
	float EngineMaxTorque = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 엔진 최대 RPM을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineMaxRPM = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="EngineSetup.MaxRPM 값입니다."))
	float EngineMaxRPM = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 엔진 아이들 RPM을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineIdleRPM = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="EngineSetup.EngineIdleRPM 값입니다."))
	float EngineIdleRPM = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 엔진 브레이크 효과를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineBrakeEffect = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="EngineSetup.EngineBrakeEffect 값입니다."))
	float EngineBrakeEffect = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 엔진 회전 상승 MOI를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineRevUpMOI = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="EngineSetup.EngineRevUpMOI 값입니다."))
	float EngineRevUpMOI = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 엔진 회전 하강 속도를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineRevDownRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="EngineSetup.EngineRevDownRate 값입니다."))
	float EngineRevDownRate = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 디퍼렌셜 타입을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideDifferentialType = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="DifferentialSetup.DifferentialType 값입니다."))
	EVehicleDifferential DifferentialType = EVehicleDifferential::RearWheelDrive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전후륜 구동 분배 값을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontRearSplit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="DifferentialSetup.FrontRearSplit 값입니다."))
	float FrontRearSplit = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 조향 타입을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideSteeringType = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="SteeringSetup.SteeringType 값입니다."))
	ESteeringType SteeringType = ESteeringType::AngleRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 조향 AngleRatio를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideSteeringAngleRatio = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ToolTip="SteeringSetup.AngleRatio 값입니다."))
	float SteeringAngleRatio = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 VehicleMovementComp의 bLegacyWheelFrictionPosition 값을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideLegacyWheelFrictionPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovementComp의 bLegacyWheelFrictionPosition 값입니다."))
	bool bLegacyWheelFrictionPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 전륜 WheelSetup AdditionalOffset를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelAdditionalOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="전륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector FrontWheelAdditionalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 후륜 WheelSetup AdditionalOffset를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelAdditionalOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="후륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector RearWheelAdditionalOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCFVehicleWheelVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="True이면 이 차량이 별도 WheelVisual 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseWheelVisualOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0", ToolTip="차량 휠 시각 동기화에서 기대하는 바퀴 개수입니다."))
	int32 ExpectedWheelCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0", ToolTip="차량 휠 시각 동기화에서 조향 대상으로 보는 전륜 개수입니다."))
	int32 FrontWheelCountForSteering = 2;
};

USTRUCT(BlueprintType)
struct FCFVehicleReferenceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 전륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> FrontWheelClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량 후륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> RearWheelClass;
};

UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차체와 휠 시각 자산 참조를 묶은 설정입니다."))
	FCFVehicleVisualConfig VehicleVisualConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Legacy|AutoFit", meta=(ToolTip="레거시 AutoFit / WheelLayout 설정입니다. 현재 런타임에서는 사용하지 않습니다.", AdvancedDisplay))
	FCFVehicleLayoutConfig VehicleLayoutConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="VehicleMovement 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleMovementConfig VehicleMovementConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="WheelSync 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleWheelVisualConfig WheelVisualConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="전륜과 후륜 Wheel Class 등 차량 참조형 자산 설정입니다."))
	FCFVehicleReferenceConfig VehicleReferenceConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ToolTip="차량별 DriveState 판정 임계값과 히스테리시스 설정입니다."))
	FCFVehicleDriveStateConfig DriveStateConfig;
};
