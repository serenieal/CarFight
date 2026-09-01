// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-09-01
// Description: CF-FQ-041 런타임 테스트용 명시 Asset Catalog 계약
// Scope: Packaged Runtime에서 노출을 허용한 VehicleData와 EquipmentPresetData의 hard reference 목록만 소유합니다.
// Changelog:
// - v1.0.1: DataAsset instance에서 수동 Catalog 등록이 가능하도록 Vehicle/Equipment 배열을 EditAnywhere로 교정.
// - v1.0.0: RTA-P0-01용 Vehicle/Equipment hard reference 목록, runtime 검증과 요약 API를 추가.
// Migration:
// - 프로젝트 전체 Asset Registry 자동 검색은 사용하지 않습니다.
// - 새 테스트/시연 대상 Asset은 이 Catalog 인스턴스에 명시적으로 등록합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFRuntimeTestCatalogData.generated.h"

class UCFEquipmentPresetData;
class UCFVehicleData;

/**
 * 개발·시연 Runtime 메뉴가 노출할 기존 차량과 장비 Asset의 명시 목록입니다.
 */
UCLASS(BlueprintType, meta=(DisplayName="런타임 테스트 Catalog 데이터 (Runtime Test Catalog Data)", ToolTip="PIE와 패키징된 시연 빌드에서 선택 가능하도록 명시 등록한 VehicleData와 EquipmentPresetData만 제공합니다."))
class CARFIGHT_RE_API UCFRuntimeTestCatalogData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] Catalog가 Runtime 선택 목록으로 사용 가능한 최소 계약을 만족하는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeTest|Catalog", meta=(DisplayName="런타임 테스트 Catalog 사용 가능 여부 (Is Runtime Test Catalog Usable)", ToolTip="차량과 장비 목록이 비어 있지 않고 Null 또는 중복 Asset 참조가 없는지 확인합니다."))
	bool IsRuntimeTestCatalogUsable() const;

	// [v1.0.0] Catalog의 모든 계약 오류를 호출자에게 반환합니다.
	bool ValidateRuntimeTestCatalog(TArray<FText>& OutValidationErrors) const;

	// [v1.0.0] 디버그 UI와 로그에 사용할 Catalog 등록 개수와 유효성 요약을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeTest|Catalog", meta=(DisplayName="런타임 테스트 Catalog 요약 생성 (Build Runtime Test Catalog Summary)", ToolTip="등록된 차량 수, 장비 수와 Catalog 유효성 상태를 한 줄 문자열로 반환합니다."))
	FString BuildRuntimeTestCatalogSummary() const;

	// [v1.0.0] Runtime 메뉴에 노출할 VehicleData hard reference 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeTest|Catalog", meta=(DisplayName="허용 차량 데이터 목록 (Allowed Vehicle Data)", ToolTip="Runtime 테스트 메뉴에 표시할 VehicleData만 명시적으로 등록합니다. Tests/Legacy를 자동 검색하지 않습니다."))
	TArray<TObjectPtr<UCFVehicleData>> AllowedVehicleData;

	// [v1.0.0] Runtime 메뉴에 노출할 EquipmentPresetData hard reference 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|RuntimeTest|Catalog", meta=(DisplayName="허용 장비 프리셋 목록 (Allowed Equipment Preset Data)", ToolTip="Runtime 테스트 메뉴에 표시할 EquipmentPresetData만 명시적으로 등록합니다. Inventory 소유권 목록과는 독립적입니다."))
	TArray<TObjectPtr<UCFEquipmentPresetData>> AllowedEquipmentPresetData;
};
