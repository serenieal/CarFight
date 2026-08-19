// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-15
// Description: CF-FQ-036 차량 Sensor 설정 DataAsset / SEN-P0-02 bounded Passive 설정 검증
// Scope: FCFSensorConfig 조정값, 계약 검증과 디버그 요약을 제공합니다.
// Changelog:
// - v1.1.0: Active Scan 0=비활성 계약과 MaxActorScansPerUpdate bounded budget 검증을 반영.
// - v1.0.0: SEN-P0-01 UCFVehicleSensorData와 DataValidation 계약을 최초 추가.
// Migration:
// - SEN-P0-02에서도 DataAsset 클래스만 변경하며 Content .uasset은 생성하거나 저장하지 않습니다.
// - 실제 SensorData가 없거나 유효하지 않으면 VehicleSensorComp의 FallbackSensorConfig를 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFSensorTypes.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "CFVehicleSensorData.generated.h"

/**
 * 차량 Sensor Runtime이 사용할 조정 가능한 정적 설정 DataAsset입니다.
 */
UCLASS(BlueprintType, meta=(DisplayName="차량 센서 데이터 (Vehicle Sensor Data)", ToolTip="Passive/Active 탐지, Contact 기억과 Tactical Analysis 조정값을 제공합니다."))
class CARFIGHT_RE_API UCFVehicleSensorData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 현재 SensorConfig가 유한하고 허용 범위 안에 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Data", meta=(DisplayName="센서 설정 유효 여부", ToolTip="탐지 거리, 갱신 간격, Contact 수명, 분석율과 정보 단계 임계값이 유효한지 검사합니다."))
	bool IsSensorConfigValid() const;

	// [v1.0.0] DataValidation과 Automation이 공유할 SensorData 계약 오류 목록을 생성합니다.
	bool ValidateSensorDataContract(TArray<FText>& OutValidationErrors) const;

	// [v1.0.0] 디버그와 로그에 사용할 SensorData 설정 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Data", meta=(DisplayName="센서 데이터 요약 생성", ToolTip="현재 SensorConfig 값과 유효성 결과를 한 줄 문자열로 생성합니다."))
	FString BuildSensorDataSummary() const;

#if WITH_EDITOR
	// [v1.0.0] Unreal Data Validation에서 잘못된 Sensor 설정을 보고합니다.
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	// [v1.0.0] 차량 Sensor Runtime이 사용할 조정 가능한 P0 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Data", meta=(DisplayName="센서 설정", ToolTip="Passive/Active/Visual 탐지 거리, Contact 수명과 Tactical Analysis 조정값입니다."))
	FCFSensorConfig SensorConfig;
};
