// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.0
// Date: 2026-04-01
// Description: CarFight 차량 루트 DataAsset 최소 확장안 + AutoFit 레거시 구조 삭제
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="VehicleMovement 덮어쓰기 사용 (bUseMovementOverrides)", ToolTip="True이면 이 차량이 별도 VehicleMovement 설정 구조를 사용할 준비가 되어 있음을 뜻합니다."))
	bool bUseMovementOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="이동 프로필 이름 (MovementProfileName)", ToolTip="현재 단계에서는 실제 수치 대신 식별용 이름 또는 메모성 구분자로 사용합니다."))
	FName MovementProfileName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 최대 조향각 덮어쓰기 (bOverrideFrontWheelMaxSteerAngle)", ToolTip="True이면 전륜 Wheel Class 기본값의 MaxSteerAngle을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelMaxSteerAngle = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 최대 조향각 (FrontWheelMaxSteerAngle)", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 조향각(deg)입니다."))
	float FrontWheelMaxSteerAngle = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 최대 브레이크 토크 덮어쓰기 (bOverrideFrontWheelMaxBrakeTorque)", ToolTip="True이면 전륜 Wheel Class 기본값의 MaxBrakeTorque를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelMaxBrakeTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 최대 브레이크 토크 (FrontWheelMaxBrakeTorque)", ToolTip="전륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float FrontWheelMaxBrakeTorque = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 최대 브레이크 토크 덮어쓰기 (bOverrideRearWheelMaxBrakeTorque)", ToolTip="True이면 후륜 Wheel Class 기본값의 MaxBrakeTorque를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelMaxBrakeTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 최대 브레이크 토크 (RearWheelMaxBrakeTorque)", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 브레이크 토크입니다."))
	float RearWheelMaxBrakeTorque = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 최대 핸드브레이크 토크 덮어쓰기 (bOverrideRearWheelMaxHandBrakeTorque)", ToolTip="True이면 후륜 Wheel Class 기본값의 MaxHandBrakeTorque를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelMaxHandBrakeTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 최대 핸드브레이크 토크 (RearWheelMaxHandBrakeTorque)", ToolTip="후륜 Wheel Class 기본값에 적용할 최대 핸드브레이크 토크입니다."))
	float RearWheelMaxHandBrakeTorque = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 휠 반지름 덮어쓰기 (bOverrideFrontWheelRadius)", ToolTip="True이면 전륜 Wheel Class 기본값의 WheelRadius를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelRadius = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 휠 반지름 (FrontWheelRadius)", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float FrontWheelRadius = 39.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 휠 반지름 덮어쓰기 (bOverrideRearWheelRadius)", ToolTip="True이면 후륜 Wheel Class 기본값의 WheelRadius를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelRadius = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 휠 반지름 (RearWheelRadius)", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 반지름(cm)입니다."))
	float RearWheelRadius = 39.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 휠 폭 덮어쓰기 (bOverrideFrontWheelWidth)", ToolTip="True이면 전륜 Wheel Class 기본값의 WheelWidth를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelWidth = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 휠 폭 (FrontWheelWidth)", ToolTip="전륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float FrontWheelWidth = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 휠 폭 덮어쓰기 (bOverrideRearWheelWidth)", ToolTip="True이면 후륜 Wheel Class 기본값의 WheelWidth를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelWidth = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 휠 폭 (RearWheelWidth)", ToolTip="후륜 Wheel Class 기본값에 적용할 바퀴 폭(cm)입니다."))
	float RearWheelWidth = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 마찰력 배수 덮어쓰기 (bOverrideFrontWheelFrictionForceMultiplier)", ToolTip="True이면 전륜 Wheel Class 기본값의 FrictionForceMultiplier를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelFrictionForceMultiplier = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 마찰력 배수 (FrontWheelFrictionForceMultiplier)", ToolTip="전륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float FrontWheelFrictionForceMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 마찰력 배수 덮어쓰기 (bOverrideRearWheelFrictionForceMultiplier)", ToolTip="True이면 후륜 Wheel Class 기본값의 FrictionForceMultiplier를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelFrictionForceMultiplier = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 마찰력 배수 (RearWheelFrictionForceMultiplier)", ToolTip="후륜 Wheel Class 기본값에 적용할 마찰력 배수입니다."))
	float RearWheelFrictionForceMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 코너링 강성 덮어쓰기 (bOverrideFrontWheelCorneringStiffness)", ToolTip="True이면 전륜 Wheel Class 기본값의 CorneringStiffness를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelCorneringStiffness = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 코너링 강성 (FrontWheelCorneringStiffness)", ToolTip="전륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float FrontWheelCorneringStiffness = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 코너링 강성 덮어쓰기 (bOverrideRearWheelCorneringStiffness)", ToolTip="True이면 후륜 Wheel Class 기본값의 CorneringStiffness를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelCorneringStiffness = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 코너링 강성 (RearWheelCorneringStiffness)", ToolTip="후륜 Wheel Class 기본값에 적용할 CorneringStiffness 입니다."))
	float RearWheelCorneringStiffness = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Wheel Load Ratio 덮어쓰기 (bOverrideFrontWheelLoadRatio)", ToolTip="True이면 전륜 Wheel Class 기본값의 WheelLoadRatio를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelLoadRatio = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 Wheel Load Ratio (FrontWheelLoadRatio)", ToolTip="전륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float FrontWheelLoadRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Wheel Load Ratio 덮어쓰기 (bOverrideRearWheelLoadRatio)", ToolTip="True이면 후륜 Wheel Class 기본값의 WheelLoadRatio를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelLoadRatio = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 Wheel Load Ratio (RearWheelLoadRatio)", ToolTip="후륜 Wheel Class 기본값에 적용할 WheelLoadRatio 입니다."))
	float RearWheelLoadRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 스프링 강성 덮어쓰기 (bOverrideFrontWheelSpringRate)", ToolTip="True이면 전륜 Wheel Class 기본값의 SpringRate를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSpringRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 스프링 강성 (FrontWheelSpringRate)", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float FrontWheelSpringRate = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 스프링 강성 덮어쓰기 (bOverrideRearWheelSpringRate)", ToolTip="True이면 후륜 Wheel Class 기본값의 SpringRate를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSpringRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 스프링 강성 (RearWheelSpringRate)", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringRate 입니다."))
	float RearWheelSpringRate = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 스프링 프리로드 덮어쓰기 (bOverrideFrontWheelSpringPreload)", ToolTip="True이면 전륜 Wheel Class 기본값의 SpringPreload를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSpringPreload = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 스프링 프리로드 (FrontWheelSpringPreload)", ToolTip="전륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float FrontWheelSpringPreload = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 스프링 프리로드 덮어쓰기 (bOverrideRearWheelSpringPreload)", ToolTip="True이면 후륜 Wheel Class 기본값의 SpringPreload를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSpringPreload = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 스프링 프리로드 (RearWheelSpringPreload)", ToolTip="후륜 Wheel Class 기본값에 적용할 SpringPreload 입니다."))
	float RearWheelSpringPreload = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 서스펜션 최대 상승 덮어쓰기 (bOverrideFrontWheelSuspensionMaxRaise)", ToolTip="True이면 전륜 Wheel Class 기본값의 SuspensionMaxRaise를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSuspensionMaxRaise = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 서스펜션 최대 상승 (FrontWheelSuspensionMaxRaise)", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float FrontWheelSuspensionMaxRaise = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 서스펜션 최대 상승 덮어쓰기 (bOverrideRearWheelSuspensionMaxRaise)", ToolTip="True이면 후륜 Wheel Class 기본값의 SuspensionMaxRaise를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSuspensionMaxRaise = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 서스펜션 최대 상승 (RearWheelSuspensionMaxRaise)", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxRaise 입니다."))
	float RearWheelSuspensionMaxRaise = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 서스펜션 최대 하강 덮어쓰기 (bOverrideFrontWheelSuspensionMaxDrop)", ToolTip="True이면 전륜 Wheel Class 기본값의 SuspensionMaxDrop를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSuspensionMaxDrop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="전륜 서스펜션 최대 하강 (FrontWheelSuspensionMaxDrop)", ToolTip="전륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float FrontWheelSuspensionMaxDrop = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 서스펜션 최대 하강 덮어쓰기 (bOverrideRearWheelSuspensionMaxDrop)", ToolTip="True이면 후륜 Wheel Class 기본값의 SuspensionMaxDrop를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSuspensionMaxDrop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="후륜 서스펜션 최대 하강 (RearWheelSuspensionMaxDrop)", ToolTip="후륜 Wheel Class 기본값에 적용할 SuspensionMaxDrop 입니다."))
	float RearWheelSuspensionMaxDrop = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 엔진 영향 여부 덮어쓰기 (bOverrideFrontWheelAffectedByEngine)", ToolTip="True이면 전륜 Wheel Class 기본값의 엔진 영향 여부를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 엔진 영향 여부 (bFrontWheelAffectedByEngine)", ToolTip="전륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bFrontWheelAffectedByEngine = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 엔진 영향 여부 덮어쓰기 (bOverrideRearWheelAffectedByEngine)", ToolTip="True이면 후륜 Wheel Class 기본값의 엔진 영향 여부를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 엔진 영향 여부 (bRearWheelAffectedByEngine)", ToolTip="후륜 Wheel Class 기본값의 엔진 영향 여부입니다."))
	bool bRearWheelAffectedByEngine = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Sweep Shape 덮어쓰기 (bOverrideFrontWheelSweepShape)", ToolTip="True이면 전륜 Wheel Class 기본값의 SweepShape를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelSweepShape = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Sweep Shape (FrontWheelSweepShape)", ToolTip="전륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape FrontWheelSweepShape = ESweepShape::Shapecast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Sweep Shape 덮어쓰기 (bOverrideRearWheelSweepShape)", ToolTip="True이면 후륜 Wheel Class 기본값의 SweepShape를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelSweepShape = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Sweep Shape (RearWheelSweepShape)", ToolTip="후륜 Wheel Class 기본값에 적용할 SweepShape 입니다."))
	ESweepShape RearWheelSweepShape = ESweepShape::Shapecast;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 높이 덮어쓰기 (bOverrideChassisHeight)", ToolTip="True이면 차체 높이를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideChassisHeight = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="차체 높이 (ChassisHeight)", ToolTip="VehicleMovementComp의 ChassisHeight 값입니다."))
	float ChassisHeight = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="공기 저항 계수 덮어쓰기 (bOverrideDragCoefficient)", ToolTip="True이면 공기 저항 계수를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideDragCoefficient = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="공기 저항 계수 (DragCoefficient)", ToolTip="VehicleMovementComp의 DragCoefficient 값입니다."))
	float DragCoefficient = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="다운포스 계수 덮어쓰기 (bOverrideDownforceCoefficient)", ToolTip="True이면 다운포스 계수를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideDownforceCoefficient = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="다운포스 계수 (DownforceCoefficient)", ToolTip="VehicleMovementComp의 DownforceCoefficient 값입니다."))
	float DownforceCoefficient = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오버라이드 사용 덮어쓰기 (bOverrideEnableCenterOfMassOverride)", ToolTip="True이면 중심질량 오버라이드 사용 여부를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEnableCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오버라이드 사용 여부 (bEnableCenterOfMassOverride)", ToolTip="VehicleMovementComp의 중심질량 오버라이드 사용 여부입니다."))
	bool bEnableCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오프셋 덮어쓰기 (bOverrideCenterOfMassOverride)", ToolTip="True이면 중심질량 오프셋을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideCenterOfMassOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="중심질량 오프셋 (CenterOfMassOverride)", ToolTip="VehicleMovementComp의 CenterOfMassOverride 값입니다."))
	FVector CenterOfMassOverride = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="엔진 최대 토크 덮어쓰기 (bOverrideEngineMaxTorque)", ToolTip="True이면 엔진 최대 토크를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineMaxTorque = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 최대 토크 (EngineMaxTorque)", ToolTip="EngineSetup.MaxTorque 값입니다."))
	float EngineMaxTorque = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="엔진 최대 RPM 덮어쓰기 (bOverrideEngineMaxRPM)", ToolTip="True이면 엔진 최대 RPM을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineMaxRPM = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 최대 RPM (EngineMaxRPM)", ToolTip="EngineSetup.MaxRPM 값입니다."))
	float EngineMaxRPM = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="엔진 아이들 RPM 덮어쓰기 (bOverrideEngineIdleRPM)", ToolTip="True이면 엔진 아이들 RPM을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineIdleRPM = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 아이들 RPM (EngineIdleRPM)", ToolTip="EngineSetup.EngineIdleRPM 값입니다."))
	float EngineIdleRPM = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="엔진 브레이크 효과 덮어쓰기 (bOverrideEngineBrakeEffect)", ToolTip="True이면 엔진 브레이크 효과를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineBrakeEffect = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 브레이크 효과 (EngineBrakeEffect)", ToolTip="EngineSetup.EngineBrakeEffect 값입니다."))
	float EngineBrakeEffect = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="엔진 회전 상승 MOI 덮어쓰기 (bOverrideEngineRevUpMOI)", ToolTip="True이면 엔진 회전 상승 MOI를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineRevUpMOI = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 회전 상승 MOI (EngineRevUpMOI)", ToolTip="EngineSetup.EngineRevUpMOI 값입니다."))
	float EngineRevUpMOI = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="엔진 회전 하강 속도 덮어쓰기 (bOverrideEngineRevDownRate)", ToolTip="True이면 엔진 회전 하강 속도를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideEngineRevDownRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="엔진 회전 하강 속도 (EngineRevDownRate)", ToolTip="EngineSetup.EngineRevDownRate 값입니다."))
	float EngineRevDownRate = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="디퍼렌셜 타입 덮어쓰기 (bOverrideDifferentialType)", ToolTip="True이면 디퍼렌셜 타입을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideDifferentialType = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="디퍼렌셜 타입 (DifferentialType)", ToolTip="DifferentialSetup.DifferentialType 값입니다."))
	EVehicleDifferential DifferentialType = EVehicleDifferential::RearWheelDrive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전후륜 구동 분배 덮어쓰기 (bOverrideFrontRearSplit)", ToolTip="True이면 전후륜 구동 분배 값을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontRearSplit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="전후륜 구동 분배 (FrontRearSplit)", ToolTip="DifferentialSetup.FrontRearSplit 값입니다."))
	float FrontRearSplit = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="조향 타입 덮어쓰기 (bOverrideSteeringType)", ToolTip="True이면 조향 타입을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideSteeringType = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="조향 타입 (SteeringType)", ToolTip="SteeringSetup.SteeringType 값입니다."))
	ESteeringType SteeringType = ESteeringType::AngleRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="조향 Angle Ratio 덮어쓰기 (bOverrideSteeringAngleRatio)", ToolTip="True이면 조향 AngleRatio를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideSteeringAngleRatio = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(ClampMin="0.0", DisplayName="조향 Angle Ratio (SteeringAngleRatio)", ToolTip="SteeringSetup.AngleRatio 값입니다."))
	float SteeringAngleRatio = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="레거시 휠 마찰 위치 덮어쓰기 (bOverrideLegacyWheelFrictionPosition)", ToolTip="True이면 VehicleMovementComp의 bLegacyWheelFrictionPosition 값을 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideLegacyWheelFrictionPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="레거시 휠 마찰 위치 사용 여부 (bLegacyWheelFrictionPosition)", ToolTip="VehicleMovementComp의 bLegacyWheelFrictionPosition 값입니다."))
	bool bLegacyWheelFrictionPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Additional Offset 덮어쓰기 (bOverrideFrontWheelAdditionalOffset)", ToolTip="True이면 전륜 WheelSetup AdditionalOffset를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideFrontWheelAdditionalOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="전륜 Additional Offset (FrontWheelAdditionalOffset)", ToolTip="전륜 WheelSetup에 적용할 AdditionalOffset 값입니다."))
	FVector FrontWheelAdditionalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Data", meta=(DisplayName="후륜 Additional Offset 덮어쓰기 (bOverrideRearWheelAdditionalOffset)", ToolTip="True이면 후륜 WheelSetup AdditionalOffset를 이 차량 데이터 기준 값으로 덮어씁니다."))
	bool bOverrideRearWheelAdditionalOffset = false;

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
