// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 탄종 정적 DataAsset
// Scope: 탄종 식별, 표시 이름, 계열, 탄당 질량, 피팅 적재 한도와 UI 메타데이터를 소유합니다.
// Changelog:
// - v1.0.0: AMMO-P0-01 UCFAmmoData 최초 추가.
// Migration:
// - ProjectileData와 DamageData 소유 경로는 변경하지 않습니다.
// - MaximumLoadableAmmoCount는 피팅 한도이며 인게임 현재 탄약 수량으로 직접 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFAmmoData.generated.h"

class UTexture2D;

/**
 * 차량이 출격 시 적재할 수 있는 탄종의 정적 정의입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFAmmoData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 음수나 비유한 값을 제거한 탄약 한 단위 질량을 kg으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|AmmoData", meta=(DisplayName="유효 탄약 단위 질량 반환", ToolTip="피팅 질량 계산에서 사용할 0 이상의 유한한 탄약 한 단위 질량을 kg으로 반환합니다."))
	float GetEffectiveUnitMassKg() const;

	// [v1.0.0] 음수 적재 한도를 0으로 보정한 피팅 최대 적재량을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|AmmoData", meta=(DisplayName="유효 최대 적재 탄약량 반환", ToolTip="피팅 화면에서 사용할 0 이상의 최대 적재 한도입니다. 인게임 현재 탄약 수량이 아닙니다."))
	int32 GetEffectiveMaximumLoadableAmmoCount() const;

	// [v1.0.0] Runtime 식별에 필요한 AmmoId가 유효한지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|AmmoData", meta=(DisplayName="탄약 데이터 유효 여부", ToolTip="AmmoId가 비어 있지 않아 Runtime과 저장 데이터에서 안정적으로 식별할 수 있으면 True입니다."))
	bool IsAmmoDataValid() const;

	// [v1.0.0] 로그와 Debug에서 사용할 탄종 정적 설정 요약을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|AmmoData", meta=(DisplayName="탄약 데이터 요약 생성", ToolTip="AmmoId, 계열, 질량, 최대 적재량과 재보급 정책을 한 줄로 반환합니다."))
	FString BuildAmmoSummary() const;

	// [v1.0.0] 탄종을 저장·Runtime·UI에서 안정적으로 식별할 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Identity", meta=(DisplayName="탄약 ID (AmmoId)", ToolTip="저장, Runtime과 피팅에서 이 탄종을 식별할 안정 ID입니다. 서로 다른 탄종은 같은 ID를 사용하지 않습니다."))
	FName AmmoId = NAME_None;

	// [v1.0.0] 플레이어 UI에서 표시할 탄종 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Identity", meta=(DisplayName="탄약 표시 이름 (AmmoDisplayName)", ToolTip="내부 Asset 이름 대신 HUD와 피팅 화면에 표시할 플레이어용 탄종 이름입니다."))
	FText AmmoDisplayName;

	// [v1.0.0] 상호 호환 가능한 탄약 계열을 분류할 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Identity", meta=(DisplayName="탄약 계열 ID (AmmoFamilyId)", ToolTip="탄종 교환이나 호환성 확장 시 같은 계열을 분류할 ID입니다. P0에서는 식별 메타데이터로만 사용합니다."))
	FName AmmoFamilyId = NAME_None;

	// [v1.0.0] 탄약 한 단위가 차량 피팅 질량에 더하는 kg 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Mass", meta=(ClampMin="0.0", Units="kg", DisplayName="탄약 단위 질량 kg (UnitMassKg)", ToolTip="탄약 한 단위가 출격 초기 차량 총중량에 더하는 질량입니다."))
	float UnitMassKg = 0.0f;

	// [v1.0.0] 탄종 검색·분류·후속 규칙에서 사용할 확장 태그 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Identity", meta=(DisplayName="탄약 태그 (AmmoTags)", ToolTip="AP, HE, Missile 같은 프로젝트 내부 분류 태그를 FName으로 저장합니다. P0에서는 검색·표시 메타데이터입니다."))
	TArray<FName> AmmoTags;

	// [v1.0.0] HUD와 피팅 화면에서 사용할 선택적 탄약 아이콘입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|UI", meta=(DisplayName="탄약 아이콘 (AmmoIcon)", ToolTip="HUD와 피팅 화면에서 사용할 선택적 탄약 아이콘 Soft Reference입니다."))
	TSoftObjectPtr<UTexture2D> AmmoIcon;

	// [v1.0.0] 피팅에서 이번 출격에 선택할 수 있는 해당 탄종 최대 한도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Fitting", meta=(ClampMin="0", DisplayName="최대 적재 가능 탄약량 (MaximumLoadableAmmoCount)", ToolTip="피팅 단계에서 선택 가능한 최대 한도입니다. 인게임 HUD의 현재 탄약 수량으로 사용하지 않습니다."))
	int32 MaximumLoadableAmmoCount = 0;

	// [v1.0.0] 후속 보급 시스템에서 이 탄종을 재보급할 수 있는지 나타냅니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|AmmoData|Fitting", meta=(DisplayName="재보급 가능 여부 (bCanBeResupplied)", ToolTip="후속 보급 시스템에서 이 탄종을 다시 채울 수 있으면 True입니다."))
	bool bCanBeResupplied = true;
};
