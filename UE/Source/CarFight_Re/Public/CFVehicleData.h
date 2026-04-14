// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-04-01
// Description: CarFight 차량 루트 DataAsset VehicleMovement 기본값 재정렬 및 레거시 실험값 자동 마이그레이션 추가
// Scope: 차량 시각 자산, Wheel Class 참조, VehicleMovement/WheelVisual 소비 구조에서 DA 기본값과 레거시 자산 보정을 함께 다룹니다.

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
struct FCFVehicleVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 메쉬 (ChassisMesh)", ToolTip="차량 차체 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> ChassisMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="앞왼쪽 휠 메쉬 (WheelMeshFL)", ToolTip="앞왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다."))
	TObjectPtr<UStaticMesh> WheelMeshFL = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="앞오른쪽 휠 메쉬 (WheelMeshFR)", ToolTip="앞오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshFR = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="뒤왼쪽 휠 메쉬 (WheelMeshRL)", ToolTip="뒤왼쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRL = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="뒤오른쪽 휠 메쉬 (WheelMeshRR)", ToolTip="뒤오른쪽 바퀴 시각 표현에 사용할 Static Mesh 입니다. 비어 있으면 FL 재사용을 권장합니다."))
	TObjectPtr<UStaticMesh> WheelMeshRR = nullptr;
};

USTRUCT(BlueprintType)
struct FCFVehicleMovementConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="VehicleMovement 설정 사용 (bUseMovementOverrides)", ToolTip="True이면 이 차량 DataAsset의 VehicleMovementConfig 값을 차량 기본 Movement/Wheel 설정에 일괄 적용합니다."))
	bool bUseMovementOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="이동 프로필 이름 (MovementProfileName)", ToolTip="현재 단계에서는 실제 수치 대신 식별용 이름 또는 메모성 구분자로 사용합니다."))
	FName MovementProfileName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 최대 조향각 (FrontWheelMaxSteerAngle)", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 조향각(deg)입니다."))
	float FrontWheelMaxSteerAngle = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 최대 브레이크 토크 (FrontWheelMaxBrakeTorque)", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float FrontWheelMaxBrakeTorque = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 최대 브레이크 토크 (RearWheelMaxBrakeTorque)", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float RearWheelMaxBrakeTorque = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 최대 핸드브레이크 토크 (RearWheelMaxHandBrakeTorque)", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 핸드브레이크 토크입니다."))
	float RearWheelMaxHandBrakeTorque = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 휠 반지름 (FrontWheelRadius)", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float FrontWheelRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 휠 반지름 (RearWheelRadius)", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float RearWheelRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 휠 폭 (FrontWheelWidth)", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float FrontWheelWidth = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 휠 폭 (RearWheelWidth)", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float RearWheelWidth = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 마찰력 배수 (FrontWheelFrictionForceMultiplier)", ToolTip="전륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float FrontWheelFrictionForceMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 마찰력 배수 (RearWheelFrictionForceMultiplier)", ToolTip="후륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float RearWheelFrictionForceMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 코너링 강성 (FrontWheelCorneringStiffness)", ToolTip="전륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float FrontWheelCorneringStiffness = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 코너링 강성 (RearWheelCorneringStiffness)", ToolTip="후륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float RearWheelCorneringStiffness = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 Wheel Load Ratio (FrontWheelLoadRatio)", ToolTip="전륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float FrontWheelLoadRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 Wheel Load Ratio (RearWheelLoadRatio)", ToolTip="후륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float RearWheelLoadRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 스프링 강성 (FrontWheelSpringRate)", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float FrontWheelSpringRate = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 스프링 강성 (RearWheelSpringRate)", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float RearWheelSpringRate = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 스프링 프리로드 (FrontWheelSpringPreload)", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float FrontWheelSpringPreload = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 스프링 프리로드 (RearWheelSpringPreload)", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float RearWheelSpringPreload = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 서스펜션 최대 상승 (FrontWheelSuspensionMaxRaise)", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float FrontWheelSuspensionMaxRaise = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 서스펜션 최대 상승 (RearWheelSuspensionMaxRaise)", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float RearWheelSuspensionMaxRaise = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 서스펜션 최대 하강 (FrontWheelSuspensionMaxDrop)", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float FrontWheelSuspensionMaxDrop = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 서스펜션 최대 하강 (RearWheelSuspensionMaxDrop)", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float RearWheelSuspensionMaxDrop = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 엔진 영향 여부 (bFrontWheelAffectedByEngine)", ToolTip="전륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bFrontWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 엔진 영향 여부 (bRearWheelAffectedByEngine)", ToolTip="후륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bRearWheelAffectedByEngine = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Sweep Shape (FrontWheelSweepShape)", ToolTip="전륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape FrontWheelSweepShape = ESweepShape::Raycast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Sweep Shape (RearWheelSweepShape)", ToolTip="후륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape RearWheelSweepShape = ESweepShape::Raycast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 높이 (ChassisHeight)", ToolTip="VehicleMovementComp의 ChassisHeight 값입니다."))
	float ChassisHeight = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="공기 저항 계수 (DragCoefficient)", ToolTip="VehicleMovementComp의 DragCoefficient 값입니다."))
	float DragCoefficient = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="다운포스 계수 (DownforceCoefficient)", ToolTip="VehicleMovementComp의 DownforceCoefficient 값입니다."))
	float DownforceCoefficient = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오버라이드 사용 여부 (bEnableCenterOfMassOverride)", ToolTip="VehicleMovementComp의 중심질량 오버라이드 사용 여부입니다."))
	bool bEnableCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오프셋 (CenterOfMassOverride)", ToolTip="VehicleMovementComp의 CenterOfMassOverride 값입니다."))
	FVector CenterOfMassOverride = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 최대 토크 (EngineMaxTorque)", ToolTip="EngineSetup.MaxTorque 값입니다."))
	float EngineMaxTorque = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 최대 RPM (EngineMaxRPM)", ToolTip="EngineSetup.MaxRPM 값입니다."))
	float EngineMaxRPM = 7000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 아이들 RPM (EngineIdleRPM)", ToolTip="EngineSetup.EngineIdleRPM 값입니다."))
	float EngineIdleRPM = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 브레이크 효과 (EngineBrakeEffect)", ToolTip="EngineSetup.EngineBrakeEffect 값입니다."))
	float EngineBrakeEffect = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 회전 상승 MOI (EngineRevUpMOI)", ToolTip="EngineSetup.EngineRevUpMOI 값입니다."))
	float EngineRevUpMOI = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 회전 하강 속도 (EngineRevDownRate)", ToolTip="EngineSetup.EngineRevDownRate 값입니다."))
	float EngineRevDownRate = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="디퍼렌셜 타입 (DifferentialType)", ToolTip="DifferentialSetup.DifferentialType 값입니다."))
	EVehicleDifferential DifferentialType = EVehicleDifferential::RearWheelDrive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="전후륜 구동 분배 (FrontRearSplit)", ToolTip="DifferentialSetup.FrontRearSplit 값입니다."))
	float FrontRearSplit = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="조향 타입 (SteeringType)", ToolTip="SteeringSetup.SteeringType 값입니다."))
	ESteeringType SteeringType = ESteeringType::AngleRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="조향 Angle Ratio (SteeringAngleRatio)", ToolTip="SteeringSetup.AngleRatio 값입니다."))
	float SteeringAngleRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="레거시 휠 마찰 위치 사용 여부 (bLegacyWheelFrictionPosition)", ToolTip="VehicleMovementComp의 bLegacyWheelFrictionPosition 값입니다."))
	bool bLegacyWheelFrictionPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Additional Offset (FrontWheelAdditionalOffset)", ToolTip="전륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector FrontWheelAdditionalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Additional Offset (RearWheelAdditionalOffset)", ToolTip="후륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector RearWheelAdditionalOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCFVehicleWheelVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="WheelVisual 덮어쓰기 사용 (bUseWheelVisualOverrides)", ToolTip="True이면 이 차량이 별도 WheelVisual 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseWheelVisualOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0", DisplayName="예상 휠 개수 (ExpectedWheelCount)", ToolTip="차량 휠 시각 동기화에서 기대하는 바퀴 개수입니다."))
	int32 ExpectedWheelCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0", DisplayName="조향 전륜 개수 (FrontWheelCountForSteering)", ToolTip="차량 휠 시각 동기화에서 조향 대상으로 보는 전륜 개수입니다."))
	int32 FrontWheelCountForSteering = 2;
};

USTRUCT(BlueprintType)
struct FCFVehicleReferenceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Wheel Class (FrontWheelClass)", ToolTip="차량 전륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> FrontWheelClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Wheel Class (RearWheelClass)", ToolTip="차량 후륜에 사용할 Wheel Class 입니다."))
	TSubclassOf<UChaosVehicleWheel> RearWheelClass;
};

UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 레거시 VehicleMovement 실험값 세트를 현재 프로젝트 기준값으로 보정합니다.
	virtual void PostLoad() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 시각 설정 (VehicleVisualConfig)", ToolTip="차체와 휠 시각 자산 참조를 묶은 설정입니다."))
	FCFVehicleVisualConfig VehicleVisualConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 이동 설정 (VehicleMovementConfig)", ToolTip="VehicleMovement 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleMovementConfig VehicleMovementConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="휠 시각 설정 (WheelVisualConfig)", ToolTip="WheelSync 계열 데이터를 나중에 확장하기 위한 최소 슬롯입니다."))
	FCFVehicleWheelVisualConfig WheelVisualConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차량 참조 설정 (VehicleReferenceConfig)", ToolTip="전륜과 후륜 Wheel Class 등 차량 참조형 자산 설정입니다."))
	FCFVehicleReferenceConfig VehicleReferenceConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="DriveState 설정 (DriveStateConfig)", ToolTip="차량별 DriveState 판정 임계값과 히스테리시스 설정입니다."))
	FCFVehicleDriveStateConfig DriveStateConfig;
};
