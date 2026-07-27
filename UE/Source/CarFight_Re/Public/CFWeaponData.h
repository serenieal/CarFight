// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-07-24
// Description: CarFight 차량 무기 DataAsset
// Scope: EquipmentPresetData가 참조할 무기 데이터, 장착 호환성과 선택 대상 사용 정책을 제공합니다.
// Changelog:
// - v1.6.0: TS-P0-07 장비별 대상 분류·관계·속성·추적 상태 정책을 추가.
// - v1.5.0: MountProfile legacy 직접 WeaponData 슬롯 제거에 맞춰 연결 기준을 EquipmentPresetData.DefaultWeaponData로 갱신.
// - v1.4.0: DamageData 직접 참조를 제거하고 피해 데이터 소유권을 ProjectileData 단일 경로로 정리.
// - v1.3.0: 무기 기본 DamageData 직접 참조를 추가하고 ProjectileData DamageData가 없을 때의 fallback 기준으로 정의.
// - v1.2.0: 발사 간격 입력을 CooldownSeconds에서 분당 발사속도 FireRatePerMinute로 전환하고 기존 저장값 마이그레이션을 추가.
// - v1.1.0: 기본 ProjectileData 직접 참조를 추가하고 기존 ProjectileDataId는 마이그레이션용으로 유지.
// - v1.0.0: WeaponData / ProjectileData / DamageData 분리의 첫 단계로 차량 무기 DataAsset 타입을 추가.
// Migration:
// - EquipmentPresetData.DefaultWeaponData에 이 DataAsset을 연결한다.
// - 기존 CooldownSeconds 저장값은 로드 시 FireRatePerMinute로 환산하고, 런타임 검증은 60 / FireRatePerMinute 발사 간격을 사용한다.
// - DefaultProjectileData가 비어 있어도 기존 Dummy HitScan / FireOrigin / 발사 간격 검증 흐름은 유지한다.
// - ProjectileDataId는 기존 에셋 호환과 로그 확인용으로 유지한다.
// - HitScan / Laser처럼 실제 Actor를 스폰하지 않는 무기도 DamageData가 필요하면 가상 ProjectileData를 DefaultProjectileData에 연결한다.
// - BaseDamage와 DamageProfileId는 기존 에셋 확인용 레거시 값이며, 런타임 DamageData 해석 경로에는 사용하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetUseTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "CFWeaponData.generated.h"

class UCFProjectileData;

/**
 * 무기가 발사 결과를 처리하는 기본 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFWeaponFireMode : uint8
{
	HitScan UMETA(DisplayName="HitScan"),
	Projectile UMETA(DisplayName="Projectile")
};

/**
 * 차량 장착 프로파일에서 참조할 최소 무기 데이터입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 무기 데이터 값을 초기화합니다.
	UCFWeaponData();

	// [v1.2.0] 기존 CooldownSeconds 저장값을 FireRatePerMinute로 변환합니다.
	virtual void PostLoad() override;

	// [v1.0.0] 지정한 장착 타입을 이 무기가 지원하는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="장착 타입 지원 여부 (Supports Mount Type)", ToolTip="이 무기 데이터가 지정한 차량 장착 타입에서 사용할 수 있는지 반환합니다."))
	bool SupportsMountType(ECFVehicleMountType InMountType) const;

	// [v1.0.0] 지정한 장착 크기 제한 안에 이 무기가 들어가는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="무기 크기 지원 여부 (Supports Weapon Size)", ToolTip="이 무기의 크기가 장착 프로파일의 최대 크기 제한 안에 들어가는지 반환합니다."))
	bool SupportsWeaponSize(ECFVehicleWeaponSize InMountSizeLimit) const;

	// [v1.0.0] 지정한 장착 타입과 크기 제한에서 이 무기를 사용할 수 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="장착 가능 여부 (Can Use On Mount)", ToolTip="장착 타입과 크기 제한을 함께 검사해 이 무기를 해당 장착 프로파일에서 사용할 수 있는지 반환합니다."))
	bool CanUseOnMount(ECFVehicleMountType InMountType, ECFVehicleWeaponSize InMountSizeLimit) const;

	// [v1.0.0] 디버그 패널에 표시할 무기 데이터 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="무기 요약 생성 (Build Weapon Summary)", ToolTip="디버그 패널과 로그에 표시할 핵심 무기 데이터 요약 문자열을 생성합니다."))
	FString BuildWeaponSummary() const;

	// [v1.2.0] 음수를 제거한 유효 분당 발사속도를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="분당 발사속도 반환 (Get Effective Fire Rate Per Minute)", ToolTip="음수를 제거한 유효 분당 발사속도를 반환합니다. 60이면 1초마다 1발입니다."))
	float GetEffectiveFireRatePerMinute() const;

	// [v1.2.0] 분당 발사속도를 실제 발사 간격 초로 환산합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="발사 간격 초 반환 (Get Fire Interval Seconds)", ToolTip="분당 발사속도를 실제 발사 제한에 사용할 초 단위 발사 간격으로 환산합니다. 60이면 1초입니다."))
	float GetFireIntervalSeconds() const;

	// [v1.0.0] 무기 데이터를 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Identity", meta=(DisplayName="무기 ID (WeaponId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 무기를 식별할 이름입니다. 예: Proto_TurretCannon"))
	FName WeaponId = TEXT("Proto_TurretCannon");

	// [v1.0.0] 이 무기가 차지하는 장착 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Mount", meta=(DisplayName="무기 크기 (WeaponSize)", ToolTip="이 무기가 요구하는 장착 크기입니다. 장착 프로파일의 SizeLimit보다 작거나 같아야 합니다."))
	ECFVehicleWeaponSize WeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.0.0] 이 무기가 호환되는 장착 타입 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Mount", meta=(DisplayName="호환 장착 타입 (CompatibleMountTypes)", ToolTip="이 무기를 사용할 수 있는 차량 장착 타입 목록입니다. 예: Turret"))
	TArray<ECFVehicleMountType> CompatibleMountTypes;

	// [v1.0.0] 이 무기의 기본 발사 처리 방식입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Fire", meta=(DisplayName="발사 모드 (FireMode)", ToolTip="현재 무기가 HitScan 방식인지 Projectile 방식인지 구분합니다."))
	ECFWeaponFireMode FireMode = ECFWeaponFireMode::HitScan;

	// [v1.2.0] 1분 동안 발사 가능한 탄 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Fire", meta=(ClampMin="0.0", UIMin="0.0", DisplayName="분당 발사속도 (FireRatePerMinute)", ToolTip="1분 동안 발사 가능한 탄 수입니다. 예: 60이면 1초마다 1발, 120이면 0.5초마다 1발입니다."))
	float FireRatePerMinute = 75.0f;

	// [v1.0.0] 이 무기의 기본 유효 사거리입니다.
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Fire", meta=(ClampMin="0.0", DisplayName="최대 사거리 (MaxRange)", ToolTip="HitScan Trace 또는 Projectile 기준으로 사용할 기본 최대 사거리입니다."))
float MaxRange = 10000.0f;

// [v1.6.0] 이 무기가 현재 선택 대상에 사용될 수 있는지 평가할 대상 정책입니다.
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|TargetUse", meta=(DisplayName="타겟 사용 정책 (TargetUsePolicy)", ToolTip="허용 대상 분류, 관계, 필수·제외 속성 태그와 추적 상태를 정의합니다. 모든 배열이 비어 있으면 유효한 선택 대상을 제한 없이 허용합니다."))
FCFTargetUsePolicy TargetUsePolicy;

	// [v1.4.0] 기존 에셋 확인용으로만 유지하는 레거시 기준 피해량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Legacy", meta=(ClampMin="0.0", DisplayName="레거시 기본 피해량 (LegacyBaseDamage)", ToolTip="DamageData 분리 전 사용하던 확인용 피해량입니다. 실제 DamageData는 ProjectileData에서만 참조합니다."))
	float BaseDamage = 25.0f;

	// [v1.0.0] 발사 방향에 적용할 기본 탄퍼짐 각도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Fire", meta=(ClampMin="0.0", DisplayName="탄퍼짐 각도 (SpreadDeg)", ToolTip="발사 방향에 적용할 기본 탄퍼짐 각도입니다. 0이면 탄퍼짐을 적용하지 않습니다."))
	float SpreadDeg = 0.0f;

	// [v1.0.0] 탄창 시스템을 사용할 때 한 탄창에 들어가는 발수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(ClampMin="0", DisplayName="탄창 크기 (MagazineSize)", ToolTip="탄창 시스템을 사용할 때 한 탄창에 들어가는 발수입니다. 0이면 현재 P0 단계에서 사용하지 않습니다."))
	int32 MagazineSize = 0;

	// [v1.0.0] 탄창 재장전에 필요한 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(ClampMin="0.0", DisplayName="재장전 시간 초 (ReloadTimeSeconds)", ToolTip="탄창 재장전에 필요한 시간입니다. MagazineSize가 0이면 현재 P0 단계에서 사용하지 않습니다."))
	float ReloadTimeSeconds = 0.0f;

	// [v1.0.0] 한 발 발사 시 누적할 열량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Heat", meta=(ClampMin="0.0", DisplayName="발사 열량 (HeatPerShot)", ToolTip="과열 시스템을 사용할 때 한 발 발사마다 누적할 열량입니다. 0이면 현재 P0 단계에서 사용하지 않습니다."))
	float HeatPerShot = 0.0f;

	// [v1.0.0] 과열 시스템에서 허용할 최대 열량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Heat", meta=(ClampMin="0.0", DisplayName="최대 열량 (MaxHeat)", ToolTip="과열 시스템에서 허용할 최대 열량입니다. 0이면 현재 P0 단계에서 사용하지 않습니다."))
	float MaxHeat = 0.0f;

	// [v1.0.0] 후속 AmmoData 분리 전 탄종을 구분할 임시 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Refs", meta=(DisplayName="탄종 ID (AmmoTypeId)", ToolTip="후속 AmmoData 분리 전 탄종을 구분할 임시 ID입니다."))
	FName AmmoTypeId = TEXT("ProtoShell");

	// [v1.1.0] 이 무기가 기본으로 사용할 발사체 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Refs", meta=(DisplayName="기본 발사체 데이터 (DefaultProjectileData)", ToolTip="이 무기가 기본으로 발사할 ProjectileData입니다. 비어 있으면 기존 Dummy HitScan fallback을 유지합니다."))
	TObjectPtr<UCFProjectileData> DefaultProjectileData = nullptr;

	// [v1.0.0] 기존 에셋 호환과 로그 확인용으로 유지하는 발사체 데이터 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Refs", meta=(DisplayName="발사체 데이터 ID (ProjectileDataId)", ToolTip="후속 ProjectileData 분리에서 실제 발사체 DataAsset을 연결하기 전 사용할 식별자입니다."))
	FName ProjectileDataId = NAME_None;

	// [v1.4.0] 기존 에셋 확인용으로만 유지하는 레거시 피해 프로파일 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Legacy", meta=(DisplayName="레거시 피해 프로파일 ID (LegacyDamageProfileId)", ToolTip="DamageData 분리 전 사용하던 확인용 피해 프로파일 ID입니다. 실제 DamageData는 ProjectileData에서만 참조합니다."))
	FName DamageProfileId = TEXT("ProtoDirectHit");

private:
	// [v1.2.0] 저장된 기존 CooldownSeconds 값을 FireRatePerMinute로 1회 변환합니다.
	void MigrateLegacyCooldownSeconds();

	// [v1.2.0] 기존 에셋의 CooldownSeconds 저장값을 읽기 위한 숨김 마이그레이션 값입니다.
	UPROPERTY()
	float CooldownSeconds = 0.0f;

	// [v1.2.0] 기존 CooldownSeconds 저장값을 FireRatePerMinute로 변환했는지 여부입니다.
	UPROPERTY()
	bool bMigratedCooldownSecondsToFireRate = false;
};
