// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-20
// Description: CF-FQ-036 Sensor 설정 + CF-FQ-032 UI-P0-08 Radar 표시 Range Profile 계약
// Scope: FCFSensorConfig와 Scanner 소유 Radar 표시 Range Preset, 계약 검증과 디버그 요약을 제공합니다.
// Changelog:
// - v1.2.0: Radar Zoom을 탐지 성능과 분리하기 위해 Scanner Data가 소유하는 오름차순 RadarDisplayRangePresetsCm과 명시 Default index를 추가. 기본 배열은 비어 있어 기존 Content 동작 0변경.
// - v1.1.0: Active Scan 0=비활성 계약과 MaxActorScansPerUpdate bounded budget 검증을 반영.
// - v1.0.0: SEN-P0-01 UCFVehicleSensorData와 DataValidation 계약을 최초 추가.
// Migration:
// - 기존 SensorData는 RadarDisplayRangePresetsCm 기본 빈 배열로 직렬화 호환되며 Radar Range/Zoom은 Unavailable을 유지합니다.
// - Radar Preset을 작성할 때는 모든 값이 0 초과·엄격 오름차순이고 ActiveScanRangeCm 이하이어야 하며 DefaultRadarDisplayRangePresetIndex를 명시해야 합니다.
// - 이 변경은 Content .uasset을 생성하거나 저장하지 않습니다.

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
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Data", meta=(DisplayName="센서 설정 유효 여부", ToolTip="탐지 거리, 갱신 간격, Contact 수명, 분석율과 정보 단계 임계값이 유효한지 검사합니다. Radar 표시 Range Profile은 전체 센서 데이터 유효 여부에서 별도로 검사합니다."))
	bool IsSensorConfigValid() const;

	// [v1.2.0] SensorConfig와 Radar 표시 Range Profile을 합친 전체 DataAsset 계약이 유효한지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Sensor|Data", meta=(DisplayName="전체 센서 데이터 유효 여부", ToolTip="SensorConfig와 Radar 표시 Range Preset·기본 Preset 인덱스를 함께 검사합니다. Runtime에 SensorData를 적용할 때 사용하는 전체 계약입니다."))
	bool IsSensorDataContractValid() const;

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

	// [v1.2.0] Scanner가 허용하는 Radar 표시 범위 목록입니다. 작은 값부터 큰 값 순서이며 탐지 성능 자체는 변경하지 않습니다.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Radar", meta=(DisplayName="Radar 표시 범위 프리셋", ToolTip="Mouse Wheel Radar Zoom이 선택할 cm 단위 표시 범위 목록입니다. 0보다 큰 값을 엄격 오름차순으로 입력하고 모든 값은 Active Scan 유효 최대 거리 이하이어야 합니다. 비어 있으면 Radar Range/Zoom을 제공하지 않습니다."))
	TArray<float> RadarDisplayRangePresetsCm;

	// [v1.2.0] Radar 표시 Range Profile을 처음 사용할 때 선택할 명시 기본 Preset 인덱스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Sensor|Radar", meta=(DisplayName="기본 Radar 범위 프리셋 인덱스", ToolTip="RadarDisplayRangePresetsCm이 비어 있지 않을 때 처음 사용할 0-based Preset 인덱스입니다. -1은 Radar Range Profile 미설정을 뜻하며 빈 배열에서만 허용됩니다."))
	int32 DefaultRadarDisplayRangePresetIndex = INDEX_NONE;
};
