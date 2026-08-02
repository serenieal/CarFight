// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-01
// Description: CarFight 출격 전 차량 피팅 선택과 결정론적 Snapshot DataAsset
// Scope: 기준 VehicleData, 장착 선택, 방어 선택, 누락 장착 정책, 데이터 계약 검증과 Pawn 없는 Snapshot 생성을 제공합니다.
// Changelog:
// - v1.1.0: CF-FQ-034 FIT-P0-03 Compatibility Validation과 결정론적 Mass Snapshot 생성을 추가.
// - v1.0.0: CF-FQ-034 FIT-P0-02 VehicleFittingData Foundation과 DataValidation 계약을 최초 추가.
// Migration:
// - 기존 차량은 VehicleFittingData를 지정하지 않으므로 현재 VehicleData 기본 장비·방어 동작을 그대로 유지한다.
// - BuildFittingSnapshot은 UObject 데이터만 읽는 순수 해석이며 VehiclePawn, VehicleMovement와 Chaos 상태를 수정하지 않는다.
// - Ammo 선택 배열은 CF-FQ-031의 AmmoData 계약이 구현된 뒤 중복 타입 없이 추가한다.
// - Snapshot의 차량 런타임 적용은 FIT-P0-04 이후 범위다.

#pragma once

#include "CoreMinimal.h"
#include "CFFittingTypes.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "CFVehicleFittingData.generated.h"

/**
 * 플레이어가 출격 전에 선택한 차량 장비·방어 구성을 저장하는 DataAsset입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleFittingData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] P0 피팅 데이터의 안전한 기본값을 초기화합니다.
	UCFVehicleFittingData();

	// [v1.0.0] DataValidation과 Automation이 공유할 피팅 데이터 계약 오류 목록을 생성합니다.
	bool ValidateFittingDataContract(TArray<FText>& OutValidationErrors) const;

		// [v1.1.0] 차량 플랫폼과 피팅 선택을 검증해 Pawn 없이 결정론적 Snapshot을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleFittingData", meta=(DisplayName="차량 피팅 Snapshot 생성 (Build Vehicle Fitting Snapshot)", ToolTip="VehicleData의 MountProfile 순서로 하드포인트, 장비 프리셋, 무기, 방어와 질량을 검증해 결정론적인 VehicleFittingSnapshot을 반환합니다. 차량 Pawn과 물리 상태는 변경하지 않습니다."))
	FCFVehicleFittingSnapshot BuildFittingSnapshot() const;

	// [v1.0.0] 디버그와 로그에서 사용할 피팅 선택 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleFittingData", meta=(DisplayName="차량 피팅 요약 생성 (Build Vehicle Fitting Summary)", ToolTip="피팅 ID, 기준 차량, 장착 선택 수, 누락 장착 정책과 방어 선택 방식을 한 줄로 반환합니다."))
	FString BuildVehicleFittingSummary() const;

#if WITH_EDITOR
	// [v1.0.0] Unreal Data Validation에서 잘못된 피팅 선택과 질량 기준을 보고합니다.
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	// [v1.0.0] 로그, UI와 저장 데이터에서 이 피팅을 식별할 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Identity", meta=(DisplayName="피팅 ID (FittingId)", ToolTip="출격 피팅을 로그, UI와 저장 데이터에서 식별할 이름입니다. 예: TestSUV_Default"))
	FName FittingId = TEXT("ProtoVehicleFitting");

	// [v1.0.0] 에디터와 피팅 UI에서 표시할 사람이 읽기 쉬운 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Identity", meta=(DisplayName="표시 이름 (DisplayName)", ToolTip="에디터와 피팅 화면에서 사용자에게 표시할 이름입니다."))
	FText DisplayName;

	// [v1.0.0] 이 피팅이 적용될 차량 플랫폼 원본 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Vehicle", meta=(DisplayName="차량 데이터 (VehicleData)", ToolTip="이 피팅의 하드포인트, 장착 프로파일, 기본 장비·방어와 질량 기준을 제공할 VehicleData입니다."))
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	// [v1.0.0] MountProfileId별 장비 프리셋 또는 명시적 빈 장착 선택입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Mount", meta=(DisplayName="장착 선택 목록 (MountSelections)", ToolTip="VehicleData.MountProfiles의 ID별로 적용할 EquipmentPresetData 또는 명시적 빈 장착을 저장합니다."))
	TArray<FCFVehicleMountSelection> MountSelections;

	// [v1.0.0] MountSelections에 없는 차량 장착 프로파일을 해석할 정책입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Mount", meta=(DisplayName="누락 장착 선택 정책 (MissingMountSelectionPolicy)", ToolTip="피팅에 선택이 없는 MountProfile을 차량 기본 장비, 빈 장착 또는 오류 중 어떻게 해석할지 결정합니다."))
	ECFMissingMountPolicy MissingMountSelectionPolicy = ECFMissingMountPolicy::UseVehicleDefault;

	// [v1.0.0] 차량 기본 방어, 방어 없음 또는 별도 방어 덮어쓰기 선택입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Defense", meta=(DisplayName="방어 선택 (DefenseSelection)", ToolTip="VehicleData 기본 방어 사용, 명시적 방어 없음 또는 별도 VehicleDefenseData Override를 선택합니다."))
	FCFVehicleDefenseSelection DefenseSelection;

	// [v1.0.0] UI 필터와 후속 저장·해금 연동에서 사용할 선택적 분류 태그입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleFittingData|Identity", meta=(DisplayName="피팅 태그 (FittingTags)", ToolTip="피팅을 분류하기 위한 선택적 이름 태그 목록입니다. P0 런타임 판정에는 사용하지 않습니다."))
	TArray<FName> FittingTags;
};
