// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-08-19
// Description: CarFight 차량 무기 DataAsset·피팅 질량·Charge·Heat P0 입력과 CF-FQ-008 정적 DataValidation 계약
// Scope: EquipmentPresetData가 참조할 무기 데이터, 장착 호환성, 피팅 질량, 런처 발사 패턴·Release 설정, 탄약·Charge·Heat 정적 설정과 선택 대상 사용 정책을 제공합니다.
// Changelog:
// - v1.13.0: UI-P0-06 WeaponCharge Runtime용 explicit Maximum/Initial/PerShot/Recovery 입력과 opt-in 활성 조건을 추가. 기존 Asset all-zero는 Disabled 호환.
// - v1.12.1: UE 5.8 UHT가 지원하지 않는 HeatDissipationPerSecond Units 메타데이터를 제거. Heat 값·Runtime 계약 변경 없음.
// - v1.12.0: UI-P0-06 실제 Heat Runtime이 사용할 explicit HeatDissipationPerSecond와 Heat Runtime 활성 조건을 추가. 기존 Asset 기본값은 0으로 발사 동작 변경 없음.
// - v1.11.0: CF-FQ-008 WD-P0-01 WeaponData 정적 계약 검증과 Unreal Data Validation 진입점을 추가. Runtime fallback과 기존 에셋 값은 변경하지 않음.
// - v1.10.0: CF-FQ-031 AMMO-P0-01 DefaultAmmoData, 초기 장전량, 발사당 탄약량, Reload·부분 처리 정책과 기존 무한탄 호환 Getter를 추가.
// - v1.9.0: CF-FQ-034 FIT-P0-02 WeaponMassKg와 안전 Getter·요약 출력을 추가.
// - v1.8.0: Direct·AngledEjection·VerticalEjection Release 설정과 유효값·요약 Getter를 추가.
// - v1.7.0: SingleCycle·Ripple·Salvo 발사 패턴 설정과 유효값·요약 Getter를 추가.
// - v1.6.0: TS-P0-07 장비별 대상 분류·관계·속성·추적 상태 정책을 추가.
// - v1.5.0: MountProfile legacy 직접 WeaponData 슬롯 제거에 맞춰 연결 기준을 EquipmentPresetData.DefaultWeaponData로 갱신.
// - v1.4.0: DamageData 직접 참조를 제거하고 피해 데이터 소유권을 ProjectileData 단일 경로로 정리.
// - v1.3.0: 무기 기본 DamageData 직접 참조를 추가하고 ProjectileData DamageData가 없을 때의 fallback 기준으로 정의.
// - v1.2.0: 발사 간격 입력을 CooldownSeconds에서 분당 발사속도 FireRatePerMinute로 전환하고 기존 저장값 마이그레이션을 추가.
// - v1.1.0: 기본 ProjectileData 직접 참조를 추가하고 기존 ProjectileDataId는 마이그레이션용으로 유지.
// - v1.0.0: WeaponData / ProjectileData / DamageData 분리의 첫 단계로 차량 무기 DataAsset 타입을 추가.
// Migration:
// - v1.13.0 WeaponCharge는 MaximumWeaponCharge/WeaponChargePerShot/WeaponChargeRecoveryPerSecond가 유효한 양수이고 InitialWeaponCharge가 0~Maximum 범위일 때만 활성화한다. 기존 네 값 all-zero Asset은 발사를 제한하지 않는다.
// - v1.12.0 Heat Runtime은 HeatPerShot, MaxHeat, HeatDissipationPerSecond가 모두 유효한 양수일 때만 활성화한다. 기존 Asset의 HeatDissipationPerSecond 기본 0은 Heat 비활성으로 현재 발사 결과를 유지한다.
// - v1.11.0 DataValidation은 정적 모순만 Invalid로 보고하며 FireRate=0, MaxRange=0, WeaponMass=0, DefaultProjectileData=None과 기존 무한탄 호환 조합은 허용한다.
// - WeaponFire·Launcher·Ammo의 현재 Runtime 상태와 Scheduler 정책은 이 DataAsset 검증이 중복 소유하지 않는다.
// - 기존 WeaponData는 bUseInfiniteAmmoForDebug=true와 DefaultAmmoData=None 기본값으로 현재 무한탄 발사 결과를 유지한다.
// - MagazineSize·ReloadTimeSeconds·AmmoTypeId는 삭제하거나 리네이밍하지 않고 신규 Ammo Runtime의 호환 입력으로 유지한다.
// - 기존 WeaponData는 WeaponMassKg=0 기본값으로 현재 발사와 차량 주행 결과를 유지한다.
// - 피팅에 사용할 WeaponData만 실제 무기 질량을 명시하며 0은 미설정 질량으로 후속 피팅 검증에서 처리한다.
// - 기존 WeaponData는 LauncherReleaseConfig 기본값 Direct / CarrierVelocityRatio 0으로 기존 Projectile 방향·속도를 유지한다.
// - HitScan 무기는 비Direct Release 설정이 저장돼 있어도 런타임 유효값을 Direct / 차량 속도 상속 0으로 보정한다.
// - 기존 WeaponData는 LauncherFirePatternConfig 기본값 SingleCycle / 1발을 사용해 기존 단발 발사와 쿨다운 결과를 유지한다.
// - LM-P0-03 Ripple·Salvo Scheduler는 적용됐으며 LM-P0-04는 각 내부 Projectile의 Launch Context를 Release 설정으로 생성한다.
// - EquipmentPresetData.DefaultWeaponData에 이 DataAsset을 연결한다.
// - 기존 CooldownSeconds 저장값은 로드 시 FireRatePerMinute로 환산하고, 런타임 검증은 60 / FireRatePerMinute 발사 간격을 사용한다.
// - DefaultProjectileData가 비어 있어도 기존 Dummy HitScan / FireOrigin / 발사 간격 검증 흐름은 유지한다.
// - ProjectileDataId는 기존 에셋 호환과 로그 확인용으로 유지한다.
// - HitScan / Laser처럼 실제 Actor를 스폰하지 않는 무기도 DamageData가 필요하면 가상 ProjectileData를 DefaultProjectileData에 연결한다.
// - BaseDamage와 DamageProfileId는 기존 에셋 확인용 레거시 값이며, 런타임 DamageData 해석 경로에는 사용하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFTargetUseTypes.h"
#include "CFLauncherTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "CFWeaponData.generated.h"

class UCFAmmoData;
class UCFCombatFxData;
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

		// [v1.9.0] 음수나 비유한 값을 제거한 피팅용 유효 무기 질량을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Mass", meta=(DisplayName="유효 무기 질량 반환 (Get Effective Weapon Mass)", ToolTip="피팅 질량 계산에서 사용할 0 이상의 유한한 무기 질량을 kg 단위로 반환합니다."))
	float GetEffectiveWeaponMassKg() const;

	// [v1.10.0] 기존 MagazineSize를 신규 Runtime의 MagazineCapacity 의미로 안전하게 보정해 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="유효 탄창 용량 반환", ToolTip="기존 MagazineSize 저장값을 0 이상의 신규 Ammo Runtime 탄창 용량으로 반환합니다."))
	int32 GetEffectiveMagazineCapacity() const;

	// [v1.10.0] 출격 초기 장전량을 유효 탄창 용량 범위로 보정해 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="유효 초기 장전량 반환", ToolTip="InitialLoadedAmmoCount를 0부터 유효 탄창 용량 사이로 보정해 반환합니다."))
	int32 GetEffectiveInitialLoadedAmmoCount() const;

	// [v1.10.0] 한 번의 정상 발사에 필요한 탄약 단위를 최소 1로 보정해 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="유효 발사당 탄약량 반환", ToolTip="AmmoUnitsPerShot를 최소 1로 보정해 한 번의 정상 발사가 소비할 탄약 단위를 반환합니다."))
	int32 GetEffectiveAmmoUnitsPerShot() const;

	// [v1.10.0] 기존 ReloadTimeSeconds를 0 이상의 유한한 재장전 시간으로 보정해 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="유효 재장전 시간 반환", ToolTip="ReloadTimeSeconds를 0 이상의 유한한 값으로 보정해 반환합니다."))
	float GetEffectiveReloadTimeSeconds() const;

		// [v1.10.0] 이 WeaponData가 명시적인 유한 탄약 Runtime을 사용하도록 설정됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="유한 탄약 Runtime 사용 여부", ToolTip="무한탄 Debug 호환이 꺼지고 DefaultAmmoData와 유효 탄창 용량이 있을 때만 True입니다."))
	bool UsesFiniteAmmoRuntime() const;

		// [v1.13.0] 네 explicit Charge 입력이 유효해 실제 무기 내부 Charge Runtime을 사용할지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Charge", meta=(DisplayName="무기 Charge Runtime 사용 여부", ToolTip="최대 충전량, 발사당 소비량, 초당 회복량이 유효한 양수이고 초기 충전량이 0부터 최대 충전량 사이일 때만 True입니다. 기존 기본값 0 조합은 Charge Runtime을 사용하지 않습니다."))
	bool UsesWeaponChargeRuntime() const;

	// [v1.12.0] 세 explicit Heat 입력이 모두 유효해 실제 무기 Heat Runtime을 사용할지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Heat", meta=(DisplayName="무기 Heat Runtime 사용 여부", ToolTip="발사 열량, 최대 열량, 초당 자연 냉각량이 모두 유효한 양수일 때만 True입니다. 하나라도 0이면 기존 무기처럼 Heat Runtime을 사용하지 않습니다."))
	bool UsesWeaponHeatRuntime() const;

	// [v1.10.0] 로그와 Debug에서 사용할 탄약 정적 설정 요약을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="탄약 설정 요약 생성", ToolTip="유한탄 사용 여부, AmmoData, 탄창 용량, 초기 장전량, 발사당 소비량과 Reload 정책을 한 줄로 반환합니다."))
	FString BuildAmmoConfigSummary() const;

		// [v1.0.0] 디버그 패널에 표시할 무기 데이터 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="무기 요약 생성 (Build Weapon Summary)", ToolTip="디버그 패널과 로그에 표시할 핵심 무기 데이터 요약 문자열을 생성합니다."))
	FString BuildWeaponSummary() const;

	// [v1.11.0] DataValidation과 Automation이 공유할 WeaponData 정적 계약 오류 목록을 생성합니다.
	bool ValidateWeaponDataContract(TArray<FText>& OutValidationErrors) const;

#if WITH_EDITOR
	// [v1.11.0] Unreal Data Validation에서 잘못된 WeaponData 정적 설정을 보고합니다.
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	// [v1.2.0] 음수를 제거한 유효 분당 발사속도를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="분당 발사속도 반환 (Get Effective Fire Rate Per Minute)", ToolTip="음수를 제거한 유효 분당 발사속도를 반환합니다. 60이면 1초마다 1발입니다."))
	float GetEffectiveFireRatePerMinute() const;

			// [v1.2.0] 분당 발사속도를 실제 발사 간격 초로 환산합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData", meta=(DisplayName="발사 간격 초 반환 (Get Fire Interval Seconds)", ToolTip="분당 발사속도를 실제 발사 제한에 사용할 초 단위 발사 간격으로 환산합니다. 60이면 1초입니다."))
	float GetFireIntervalSeconds() const;

	// [v1.7.0] 음수·0·패턴 비적용 값을 안전하게 보정한 런처 발사 패턴 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Launcher", meta=(DisplayName="유효 런처 발사 패턴 반환 (Get Effective Launcher Fire Pattern Config)", ToolTip="SingleCycle은 항상 1발, Ripple은 안전한 간격, Salvo는 실제 발사 수 이내의 동시 처리 한도로 보정한 설정을 반환합니다."))
	FCFLauncherFirePatternConfig GetEffectiveLauncherFirePatternConfig() const;

		// [v1.7.0] 현재 런처 발사 패턴의 유효 수량·간격·실패·쿨다운 정책을 한 줄로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Launcher", meta=(DisplayName="런처 발사 패턴 요약 생성 (Build Launcher Fire Pattern Summary)", ToolTip="현재 WeaponData의 발사 패턴, 유효 발사 수, Ripple 간격, Salvo 동시 처리 한도와 정책을 문자열로 반환합니다."))
	FString BuildLauncherFirePatternSummary() const;

	// [v1.8.0] 현재 무기·ProjectileData 기준으로 안전하게 보정된 런처 Release 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Launcher", meta=(DisplayName="유효 런처 Release 설정 반환 (Get Effective Launcher Release Config)", ToolTip="Direct 호환, 사출 방향, 사출 속력, 차량 속도 상속 비율과 안전 검사 거리를 보정해 반환합니다. HitScan은 Direct로 강제합니다."))
	FCFLauncherReleaseConfig GetEffectiveLauncherReleaseConfig() const;

	// [v1.8.0] 현재 런처 Release 모드와 사출·차량 속도 상속 설정을 한 줄로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|WeaponData|Launcher", meta=(DisplayName="런처 Release 요약 생성 (Build Launcher Release Summary)", ToolTip="Release Mode, 로컬 사출 방향, 유효 사출 속력, 차량 속도 상속 비율과 안전 검사 거리를 문자열로 반환합니다."))
	FString BuildLauncherReleaseSummary() const;

	// [v1.0.0] 무기 데이터를 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Identity", meta=(DisplayName="무기 ID (WeaponId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 무기를 식별할 이름입니다. 예: Proto_TurretCannon"))
	FName WeaponId = TEXT("Proto_TurretCannon");

	// [v1.0.0] 이 무기가 차지하는 장착 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Mount", meta=(DisplayName="무기 크기 (WeaponSize)", ToolTip="이 무기가 요구하는 장착 크기입니다. 장착 프로파일의 SizeLimit보다 작거나 같아야 합니다."))
	ECFVehicleWeaponSize WeaponSize = ECFVehicleWeaponSize::Large;

		// [v1.0.0] 이 무기가 호환되는 장착 타입 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Mount", meta=(DisplayName="호환 장착 타입 (CompatibleMountTypes)", ToolTip="이 무기를 사용할 수 있는 차량 장착 타입 목록입니다. 예: Turret"))
	TArray<ECFVehicleMountType> CompatibleMountTypes;

	// [v1.9.0] 이 무기 본체가 차량 피팅 총중량에 기여하는 질량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Mass", meta=(ClampMin="0.0", Units="kg", DisplayName="무기 질량 kg (WeaponMassKg)", ToolTip="이 무기 본체가 차량 피팅 총중량에 더하는 질량입니다. 0은 미설정이며 기존 발사 동작에는 영향을 주지 않습니다."))
	float WeaponMassKg = 0.0f;

	// [v1.0.0] 이 무기의 기본 발사 처리 방식입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Fire", meta=(DisplayName="발사 모드 (FireMode)", ToolTip="현재 무기가 HitScan 방식인지 Projectile 방식인지 구분합니다."))
	ECFWeaponFireMode FireMode = ECFWeaponFireMode::HitScan;

			// [v1.2.0] 1분 동안 발사 가능한 탄 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Fire", meta=(ClampMin="0.0", UIMin="0.0", DisplayName="분당 발사속도 (FireRatePerMinute)", ToolTip="1분 동안 발사 가능한 탄 수입니다. 예: 60이면 1초마다 1발, 120이면 0.5초마다 1발입니다."))
	float FireRatePerMinute = 75.0f;

		// [v1.7.0] 플레이어 입력 한 번을 SingleCycle·Ripple·Salvo 중 어떤 발사 시퀀스로 처리할지 정의합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Launcher", meta=(DisplayName="런처 발사 패턴 설정 (LauncherFirePatternConfig)", ToolTip="입력당 발사체 수, Ripple 간격, Salvo 동시 처리 한도와 실패·쿨다운 정책을 정의합니다. 기본값은 기존 동작과 같은 SingleCycle 1발입니다."))
	FCFLauncherFirePatternConfig LauncherFirePatternConfig;

	// [v1.8.0] 각 Projectile이 Muzzle에서 분리될 방향·속력과 차량 속도 상속을 정의합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Launcher", meta=(DisplayName="런처 Release 설정 (LauncherReleaseConfig)", ToolTip="Direct는 기존 직사 결과를 유지하고, AngledEjection과 VerticalEjection은 사출 속력·차량 속도 상속·실제 사출 방향 안전 검사를 사용합니다."))
	FCFLauncherReleaseConfig LauncherReleaseConfig;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(ClampMin="0.0", DisplayName="재장전 시간 초 (ReloadTimeSeconds)", ToolTip="탄창 재장전에 필요한 시간입니다. 신규 Ammo Runtime에서도 같은 저장 필드를 사용합니다."))
	float ReloadTimeSeconds = 0.0f;

	// [v1.10.0] 이 무기가 기본으로 사용하는 탄종의 정적 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="기본 탄약 데이터 (DefaultAmmoData)", ToolTip="유한 탄약 Runtime에서 이 무기가 기본으로 사용할 탄종입니다. 비어 있으면 기존 무한탄 호환 경로를 유지합니다."))
	TObjectPtr<UCFAmmoData> DefaultAmmoData = nullptr;

	// [v1.10.0] 출격 시작 시 이 WeaponInstance 탄창에 우선 장전할 탄약 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(ClampMin="0", DisplayName="출격 초기 장전량 (InitialLoadedAmmoCount)", ToolTip="출격 시작 시 이 무기 탄창에 장전할 수량입니다. 유효값은 MagazineSize를 넘지 않게 보정됩니다."))
	int32 InitialLoadedAmmoCount = 0;

	// [v1.10.0] 정상 발사 한 번이 소비하는 탄약 단위 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(ClampMin="1", DisplayName="발사당 탄약량 (AmmoUnitsPerShot)", ToolTip="정상 발사 한 번이 소비하는 탄약 단위 수입니다. P0 기본값은 1입니다."))
	int32 AmmoUnitsPerShot = 1;

	// [v1.10.0] 이 무기의 재장전 방식입니다. P0 Runtime은 FullMagazine만 실제 실행합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="재장전 방식 (ReloadMode)", ToolTip="P0 실제 Runtime은 FullMagazine을 사용하고 PerRound는 후속 확장 계약으로 보존합니다."))
	ECFWeaponReloadMode ReloadMode = ECFWeaponReloadMode::FullMagazine;

	// [v1.10.0] 탄창이 발사 최소 요구량보다 부족해졌을 때 자동 재장전을 시도할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="빈 탄창 자동 재장전", ToolTip="Launcher Sequence가 끝나고 탄창이 발사 최소 요구량보다 부족하며 예비 탄약이 있으면 자동 재장전을 시작할 수 있게 합니다."))
	bool bAutoReloadWhenEmpty = true;

	// [v1.10.0] 예비 탄약이 탄창 전체 요구량보다 적어도 가능한 만큼 부분 재장전을 허용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="부분 재장전 허용", ToolTip="예비 탄약이 부족할 때 탄창을 완전히 채우지 못해도 남은 예비량만큼 재장전할 수 있으면 True입니다."))
	bool bAllowPartialReload = true;

	// [v1.10.0] 요청한 Ripple·Salvo 전체 발수를 충족하지 못해도 가능한 발수만 예약해 시퀀스를 시작할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="부분 시퀀스 허용", ToolTip="요청 발수보다 즉시 사용 가능한 탄약이 적을 때 가능한 발수만으로 Ripple·Salvo를 시작할 수 있으면 True입니다."))
	bool bAllowPartialSequence = true;

	// [v1.10.0] 기존 에셋과 Debug 환경에서 탄약 수량 검증 없이 현재 발사 동작을 유지할 호환 플래그입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Ammo", meta=(DisplayName="무한 탄약 Debug 호환", ToolTip="True이면 신규 Ammo Runtime이 이 WeaponData의 발사를 제한하지 않습니다. 기존 WeaponData 호환을 위해 기본값은 True입니다."))
	bool bUseInfiniteAmmoForDebug = true;

		// [v1.13.0] 이 무기 내부 축전기가 보유할 수 있는 최대 Charge입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Charge", meta=(ClampMin="0.0", DisplayName="최대 무기 충전량 (MaximumWeaponCharge)", ToolTip="무기 자체 내부 축전기의 최대 충전량입니다. 0이면 Charge Runtime을 활성화하지 않습니다. VehicleBattery 용량과는 별개입니다."))
	float MaximumWeaponCharge = 0.0f;

	// [v1.13.0] 무기 Runtime 초기화 시 시작할 내부 Charge입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Charge", meta=(ClampMin="0.0", DisplayName="초기 무기 충전량 (InitialWeaponCharge)", ToolTip="출격 또는 무기 Runtime 초기화 시 시작할 내부 Charge입니다. 0부터 최대 무기 충전량 사이의 명시값만 사용합니다."))
	float InitialWeaponCharge = 0.0f;

	// [v1.13.0] 실제 승인된 한 발이 소비할 무기 내부 Charge입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Charge", meta=(ClampMin="0.0", DisplayName="발사당 무기 충전 소비량 (WeaponChargePerShot)", ToolTip="실제 승인된 한 발마다 무기 내부 Charge에서 소비할 양입니다. 0이면 Charge Runtime을 활성화하지 않습니다."))
	float WeaponChargePerShot = 0.0f;

	// [v1.13.0] 실제 Game-Time 1초마다 자연 회복할 무기 내부 Charge입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Charge", meta=(ClampMin="0.0", DisplayName="초당 무기 충전 회복량 (WeaponChargeRecoveryPerSecond)", ToolTip="게임이 진행되는 동안 1초마다 자연 회복할 무기 내부 Charge입니다. 0이면 Charge Runtime을 활성화하지 않습니다."))
	float WeaponChargeRecoveryPerSecond = 0.0f;

	// [v1.0.0] 한 발 발사 시 누적할 열량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Heat", meta=(ClampMin="0.0", DisplayName="발사 열량 (HeatPerShot)", ToolTip="과열 시스템을 사용할 때 한 발 발사마다 누적할 열량입니다. 0이면 현재 P0 단계에서 사용하지 않습니다."))
	float HeatPerShot = 0.0f;

		// [v1.0.0] 과열 시스템에서 허용할 최대 열량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Heat", meta=(ClampMin="0.0", DisplayName="최대 열량 (MaxHeat)", ToolTip="과열 시스템에서 허용할 최대 열량입니다. 0이면 현재 P0 단계에서 사용하지 않습니다."))
	float MaxHeat = 0.0f;

	// [v1.12.0] 발사하지 않는 동안 1초마다 자연 감소할 Heat 양입니다.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Heat", meta=(ClampMin="0.0", DisplayName="초당 자연 냉각량 (HeatDissipationPerSecond)", ToolTip="발사하지 않는 동안 1초마다 감소할 열량입니다. 0이면 Heat Runtime을 활성화하지 않아 기존 무기 동작을 유지합니다."))
	float HeatDissipationPerSecond = 0.0f;

	// [v1.0.0] 후속 AmmoData 분리 전 탄종을 구분할 임시 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|Refs", meta=(DisplayName="탄종 ID (AmmoTypeId)", ToolTip="후속 AmmoData 분리 전 탄종을 구분할 임시 ID입니다."))
	FName AmmoTypeId = TEXT("ProtoShell");

	// [v1.1.0] 이 무기가 기본으로 사용할 발사체 DataAsset입니다.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|WeaponData|FX", meta=(DisplayName="기본 발사 FX 데이터", ToolTip="승인된 발사 위치와 방향에서 재생할 CombatFxData입니다. 비어 있어도 발사 판정은 유지합니다."))
	TObjectPtr<UCFCombatFxData> DefaultFireFxData = nullptr;

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
