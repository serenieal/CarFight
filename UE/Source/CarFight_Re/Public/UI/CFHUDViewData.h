// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-13
// Description: CF-FQ-032 UI-P0-03 + CF-FQ-031 AMMO-P0-06 인게임 HUD 공개 ViewData 계약
// Scope: Vehicle, Weapon·Ammo·Reload, Defense, Target, Radar, Alert의 플레이어 표시용 데이터와 Unknown/Unavailable/KnownZero 구분을 제공합니다.
// Changelog:
// - v1.3.0: Launcher 이벤트에 의한 의미 상태 전이와 Ammo/Timer 부수 Refresh를 구분하도록 LauncherSequenceRevision을 추가. FirePattern과 Presentation 수명 정책을 분리.
// - v1.2.0: WeaponPanel의 Loaded/Capacity + label-less Reserve 계약을 위해 MagazineCapacity를 공개 ViewData에 추가하고 Immediate/CurrentUsable을 내부 판정값으로 명시.
// - v1.1.0: AMMO-P0-06 실제 Ammo Runtime의 장전·예약·예비·사용 가능량, Reload 상태·시간과 Action Lock을 Weapon ViewData에 추가.
// - v1.0.0: UI-P0-03 최초 ViewData 계약과 통합 FCFInGameUIViewData를 추가.
// Migration:
// - Gameplay Runtime 타입을 Widget에서 직접 읽지 않고 UCFHUDDataProvider가 이 타입으로 변환합니다.
// - Radar/Sensor, Heat처럼 실제 Runtime Provider가 없는 채널은 값을 추정하지 않고 Unavailable로 유지하며 finite Ammo는 VehicleAmmoComp Snapshot만 사용합니다.
// - Target Actor 이름이나 Component 이름 같은 내부 식별자는 Player-facing Text로 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFLauncherTypes.h"
#include "CFTargetSelectTypes.h"
#include "CFHUDViewData.generated.h"

/**
 * UI가 값의 의미를 추정하지 않도록 공개 상태를 명시적으로 구분합니다.
 */
UENUM(BlueprintType, meta=(DisplayName="UI 값 가용 상태 (UI Value Availability)", ToolTip="Unknown, Unavailable, KnownZero, Known을 구분해 Widget이 0과 정보 부재를 혼동하지 않게 합니다."))
enum class ECFUIViewAvailability : uint8
{
	Unknown UMETA(DisplayName="미확인 (Unknown)"),
	Unavailable UMETA(DisplayName="제공 불가 (Unavailable)"),
	KnownZero UMETA(DisplayName="확인된 0 (Known Zero)"),
	Known UMETA(DisplayName="확인됨 (Known)")
};

/**
 * Alert Feed가 같은 의미의 현재 상태를 안정적으로 정렬할 우선순위입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="HUD 경고 우선순위 (HUD Alert Priority)", ToolTip="현재 상태 기반 HUD Alert의 정보, 주의, 위험 우선순위를 나타냅니다."))
enum class ECFHUDAlertPriority : uint8
{
	Info UMETA(DisplayName="정보 (Info)"),
	Caution UMETA(DisplayName="주의 (Caution)"),
	Critical UMETA(DisplayName="위험 (Critical)")
};

/**
 * 플레이어 차량의 주행 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="차량 HUD 데이터 (Vehicle HUD Data)", ToolTip="현재 플레이어 차량의 속도와 런타임 준비 상태를 UI에 전달합니다."))
struct CARFIGHT_RE_API FCFVehicleHUDData
{
	GENERATED_BODY()

	// [v1.0.0] 차량 채널 자체의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="차량 데이터 상태", ToolTip="현재 UI Pawn이 지원 차량인지 나타냅니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 차량 속도 값의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="속도 데이터 상태", ToolTip="현재 속도가 실제 Runtime에서 제공됐는지 나타냅니다."))
	ECFUIViewAvailability SpeedAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 VehicleDriveComp 기준 차량 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(Units="km/h", DisplayName="차량 속도 km/h", ToolTip="Vehicle Runtime에서 읽은 현재 차량 속도입니다."))
	float SpeedKmh = 0.0f;

	// [v1.0.0] 실제 Engine RPM 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="엔진 RPM 데이터 상태", ToolTip="실제 Engine RPM Provider가 없으면 Unavailable입니다. 속도로 RPM을 추정하지 않습니다."))
	ECFUIViewAvailability EngineRpmAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 Provider가 제공할 때 사용할 엔진 RPM 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="엔진 RPM", ToolTip="실제 Engine RPM Provider가 제공한 값만 사용합니다."))
	float EngineRpm = 0.0f;

	// [v1.0.0] 실제 Gear 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="기어 데이터 상태", ToolTip="실제 Gear Provider가 없으면 Unavailable이며 단수를 추정하지 않습니다."))
	ECFUIViewAvailability GearAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 Provider가 제공할 때 표시할 Gear 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="기어 표시", ToolTip="R, N 또는 실제 전진 기어 단수를 표시할 Player-facing 문자열입니다."))
	FText GearText;

	// [v1.0.0] 차량 코어 Runtime 준비 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="차량 코어 Runtime 준비", ToolTip="주행, 내구도와 기본 차량 Runtime이 준비됐는지 나타냅니다."))
	bool bVehicleCoreRuntimeReady = false;

	// [v1.0.0] 차량 전투 Runtime 준비 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="차량 전투 Runtime 준비", ToolTip="Aim, Weapon, Launcher와 TargetSelect를 포함한 전투 Runtime이 준비됐는지 나타냅니다."))
	bool bVehicleCombatRuntimeReady = false;
};

/**
 * 플레이어 차량의 쉴드, 6방향 장갑과 Integrity 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="방어 HUD 데이터 (Defense HUD Data)", ToolTip="VehicleDefense와 VehicleHealth의 실제 Runtime 값을 UI 표시용으로 전달합니다."))
struct CARFIGHT_RE_API FCFDefenseHUDData
{
	GENERATED_BODY()

	// [v1.0.0] 정식 VehicleDefense 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="방어 데이터 상태", ToolTip="유효 VehicleDefenseData로 정식 방어 Runtime이 준비됐는지 나타냅니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] DefenseData 없이 Legacy Integrity 직접 피해 경로를 사용하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="Legacy 방어 Fallback", ToolTip="True이면 Shield와 Armor 없이 Vehicle Integrity 직접 피해 호환 경로를 사용합니다."))
	bool bUsingLegacyFallback = false;

	// [v1.0.0] Shield 현재값의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="쉴드 데이터 상태", ToolTip="현재 Shield 값이 실제 방어 Runtime에서 제공됐는지 나타냅니다."))
	ECFUIViewAvailability ShieldAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 현재 Shield입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="현재 쉴드", ToolTip="현재 차량에 남은 Shield 값입니다."))
	float CurrentShield = 0.0f;

	// [v1.0.0] 최대 Shield입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="최대 쉴드", ToolTip="현재 방어 데이터가 정의한 최대 Shield 값입니다."))
	float MaximumShield = 0.0f;

	// [v1.0.0] 현재 Shield의 0~1 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="쉴드 비율", ToolTip="현재 Shield를 최대 Shield로 나눈 표시용 비율입니다."))
	float ShieldRatio = 0.0f;

	// [v1.0.0] Shield가 실제 재생 중인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="쉴드 재생 중", ToolTip="VehicleDefense Runtime이 실제 Shield 재생 중일 때 True입니다."))
	bool bShieldRegenerating = false;

	// [v1.0.0] 6방향 Armor 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="장갑 데이터 상태", ToolTip="정식 6방향 Armor Runtime이 준비됐는지 나타냅니다."))
	ECFUIViewAvailability ArmorAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] Front Armor의 현재 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="전방 장갑 비율", ToolTip="Front Armor 현재/최대 비율입니다."))
	float FrontArmorRatio = 0.0f;

	// [v1.0.0] Right Armor의 현재 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="우측 장갑 비율", ToolTip="Right Armor 현재/최대 비율입니다."))
	float RightArmorRatio = 0.0f;

	// [v1.0.0] Rear Armor의 현재 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="후방 장갑 비율", ToolTip="Rear Armor 현재/최대 비율입니다."))
	float RearArmorRatio = 0.0f;

	// [v1.0.0] Left Armor의 현재 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="좌측 장갑 비율", ToolTip="Left Armor 현재/최대 비율입니다."))
	float LeftArmorRatio = 0.0f;

	// [v1.0.0] Top Armor의 현재 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="상부 장갑 비율", ToolTip="Top Armor 현재/최대 비율입니다."))
	float TopArmorRatio = 0.0f;

	// [v1.0.0] Bottom Armor의 현재 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="하부 장갑 비율", ToolTip="Bottom Armor 현재/최대 비율입니다."))
	float BottomArmorRatio = 0.0f;

	// [v1.0.0] Vehicle Integrity 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="차량 내구도 데이터 상태", ToolTip="VehicleHealth Runtime에서 Integrity를 실제 제공하는지 나타냅니다."))
	ECFUIViewAvailability IntegrityAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 현재 Vehicle Integrity입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="현재 차량 내구도", ToolTip="Shield와 Armor를 통과한 피해가 적용되는 현재 Vehicle Integrity입니다."))
	float CurrentIntegrity = 0.0f;

	// [v1.0.0] 최대 Vehicle Integrity입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="최대 차량 내구도", ToolTip="현재 차량의 최대 Vehicle Integrity입니다."))
	float MaximumIntegrity = 0.0f;

	// [v1.0.0] Vehicle Integrity의 0~1 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="차량 내구도 비율", ToolTip="현재 Vehicle Integrity를 최대값으로 나눈 표시용 비율입니다."))
	float IntegrityRatio = 0.0f;

	// [v1.0.0] 차량이 실제 Destroyed 상태인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Defense", meta=(DisplayName="차량 파괴 상태", ToolTip="VehicleHealth Runtime이 최초 파괴 상태로 전환됐으면 True입니다."))
	bool bDestroyed = false;
};

/**
 * 현재 활성 무기와 Launcher 진행 상태의 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 HUD 데이터 (Weapon HUD Data)", ToolTip="현재 활성 무기, 쿨다운과 Launcher 시퀀스 상태를 UI에 전달합니다."))
struct CARFIGHT_RE_API FCFWeaponHUDData
{
	GENERATED_BODY()

	// [v1.0.0] 활성 무기 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 데이터 상태", ToolTip="VehicleWeapon Runtime과 활성 WeaponData 존재 상태를 구분합니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 내부 비교와 Provider 진단에 사용할 안정 Weapon ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 ID", ToolTip="Provider 내부 안정 식별용 WeaponId입니다. Player-facing 이름으로 직접 표시하지 않습니다."))
	FName WeaponId = NAME_None;

	// [v1.0.0] Player-facing 무기 이름 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 표시 이름 상태", ToolTip="실제 Player-facing DisplayName Provider가 없으면 Unavailable입니다."))
	ECFUIViewAvailability DisplayNameAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 Provider가 제공할 때 사용할 Player-facing 무기 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 표시 이름", ToolTip="내부 Asset Name이 아닌 Player-facing 무기 이름입니다."))
	FText DisplayName;

	// [v1.0.0] 발사 쿨다운 값의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="쿨다운 데이터 상태", ToolTip="실제 Weapon Runtime의 발사 간격과 남은 시간이 제공됐는지 나타냅니다."))
	ECFUIViewAvailability CooldownAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 전체 발사 간격입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(Units="s", DisplayName="전체 쿨다운 초", ToolTip="현재 활성 무기의 실제 발사 간격입니다."))
	float CooldownDurationSeconds = 0.0f;

	// [v1.0.0] 현재 남은 발사 쿨다운입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(Units="s", DisplayName="남은 쿨다운 초", ToolTip="현재 월드 시간 기준 실제 남은 무기 쿨다운입니다."))
	float RemainingCooldownSeconds = 0.0f;

		// [v1.1.0] 실제 finite Ammo Runtime Snapshot 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="탄약 데이터 상태", ToolTip="실제 finite Ammo Runtime Snapshot이 있으면 Known 또는 KnownZero입니다. InfiniteCompatibility 또는 미초기화 상태는 Unavailable이며 MagazineSize나 최대 적재량으로 가짜 수치를 만들지 않습니다."))
	ECFUIViewAvailability AmmoAvailability = ECFUIViewAvailability::Unavailable;

		// [v1.1.0] 현재 무기에 실제 장전되어 있는 탄약 단위 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="현재 장전 탄약량", ToolTip="VehicleAmmoComp Snapshot의 실제 LoadedAmmoCount입니다. WeaponPanel Primary Ammo의 왼쪽 값입니다."))
	int32 LoadedAmmoCount = 0;

	// [v1.2.0] 현재 무기 탄창이 보유할 수 있는 최대 탄약 단위 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="탄창 총 용량", ToolTip="VehicleAmmoComp Snapshot의 MagazineCapacity입니다. WeaponPanel Primary Ammo의 오른쪽 값이며 피팅 MaximumLoadableAmmoCount와 다릅니다."))
	int32 MagazineCapacity = 0;

	// [v1.1.0] 현재 Launcher Sequence에 예약되어 신규 행동에 사용할 수 없는 장전 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 예약 탄약량", ToolTip="진행 중 Ripple·Salvo에 예약된 실제 탄약 단위 수입니다."))
	int32 ReservedSequenceAmmoCount = 0;

		// [v1.1.0] 현재 신규 발사 행동에 즉시 자유롭게 사용할 수 있는 장전 탄약 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="즉시 사용 가능 탄약량", ToolTip="Launcher 예약과 단발 Transaction 예약을 제외한 실제 장전량입니다. 발사 가능·예약·회귀 판정용 내부 값이며 WeaponPanel Primary Ammo 숫자로 직접 표시하지 않습니다."))
	int32 ImmediateUsableAmmoCount = 0;

	// [v1.1.0] 차량 탄약고에 남은 같은 탄종의 실제 공유 예비량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="예비 탄약량", ToolTip="현재 무기의 AmmoId와 같은 탄종으로 차량에 남은 실제 ReserveAmmoCount입니다."))
	int32 ReserveAmmoCount = 0;

		// [v1.1.0] 현재 무기가 신규 행동으로 자유롭게 사용할 수 있는 장전+예비 탄약 총량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="현재 사용 가능 탄약량", ToolTip="피팅 최대 적재량이 아니라 실제 현재 사용 가능량입니다. NoAmmo·발사 가능·회귀 판정용 내부 값이며 WeaponPanel Primary Ammo 숫자로 직접 표시하지 않습니다."))
	int32 CurrentUsableAmmoCount = 0;

	// [v1.1.0] 같은 탄종의 모든 무기 장전량과 차량 Reserve를 합친 실제 차량 보유량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="현재 차량 보유 탄약량", ToolTip="NoAmmo 판정과 Debug에 사용하는 실제 CurrentOnboardAmmoCount입니다."))
	int32 CurrentOnboardAmmoCount = 0;

	// [v1.1.0] 현재 Ammo Runtime의 FullMagazine Reload 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="재장전 상태", ToolTip="NotInitialized, Ready, Reloading, Empty, NoReserveAmmo 또는 Disabled 상태입니다."))
	ECFWeaponReloadState ReloadState = ECFWeaponReloadState::NotInitialized;

	// [v1.1.0] 현재 Reload 한 사이클의 전체 소요 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(Units="s", DisplayName="전체 재장전 시간", ToolTip="Reload 진행률 계산에 사용할 Runtime 제공 전체 시간입니다."))
	float ReloadDurationSeconds = 0.0f;

	// [v1.1.0] 현재 Reload 완료까지 남은 실제 게임 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(Units="s", DisplayName="남은 재장전 시간", ToolTip="Pause 중 진행되지 않는 Ammo Runtime의 실제 남은 Reload 시간입니다."))
	float RemainingReloadTimeSeconds = 0.0f;

	// [v1.1.0] 현재 Ammo Runtime에서 무기 행동이 잠긴 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 행동 잠김", ToolTip="Launcher Sequence, Reloading, VehicleDestroyed 등 Ammo Runtime Action Lock이 활성화됐으면 True입니다."))
	bool bWeaponActionLocked = false;

	// [v1.1.0] 현재 Ammo Runtime Action Lock의 대표 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 행동 잠금 사유", ToolTip="현재 무기 행동을 제한하는 Ammo Runtime의 실제 Action Lock 이유입니다."))
	ECFWeaponActionLockReason WeaponActionLockReason = ECFWeaponActionLockReason::None;

	// [v1.0.0] Heat Runtime 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="열 데이터 상태", ToolTip="실제 Heat Runtime Provider가 없으면 Unavailable입니다. WeaponData 설정값으로 현재 열을 추정하지 않습니다."))
	ECFUIViewAvailability HeatAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] Launcher 시퀀스 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 시퀀스 데이터 상태", ToolTip="LauncherComp가 있으면 실제 시퀀스 상태를 제공합니다."))
	ECFUIViewAvailability LauncherAvailability = ECFUIViewAvailability::Unavailable;

		// [v1.3.0] 실제 OnLauncherSequenceChanged 이벤트가 발생할 때만 증가하는 의미 상태 Revision입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 시퀀스 Revision", ToolTip="Launcher 시작·진행·완료·취소 이벤트에서만 증가합니다. Ammo 변경이나 10Hz 연속 갱신은 이 값을 증가시키지 않아 Presentation 수명 정책이 FirePattern이나 임의 시간 Hold에 의존하지 않게 합니다."))
	int32 LauncherSequenceRevision = 0;

	// [v1.0.0] Ripple 또는 Salvo 후속 발사가 현재 진행 중인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 시퀀스 진행 중", ToolTip="LauncherComp의 실제 Active 시퀀스가 존재하면 True입니다."))
	bool bLauncherSequenceActive = false;

	// [v1.0.0] 현재 또는 마지막 Launcher 발사 패턴입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 발사 패턴", ToolTip="SingleCycle, Ripple 또는 Salvo의 실제 시퀀스 패턴입니다."))
	ECFLauncherFirePattern LauncherPattern = ECFLauncherFirePattern::SingleCycle;

	// [v1.0.0] 현재 Launcher 시퀀스 전체 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 전체 발사체 수", ToolTip="현재 시퀀스가 처리할 전체 Projectile 수입니다."))
	int32 LauncherTotalProjectileCount = 0;

	// [v1.0.0] 현재 Launcher 시퀀스 승인 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 승인 발사체 수", ToolTip="현재 시퀀스에서 실제 승인된 Projectile 수입니다."))
	int32 LauncherAcceptedProjectileCount = 0;

	// [v1.0.0] 현재 Launcher 시퀀스 남은 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="런처 남은 발사체 수", ToolTip="아직 Dispatch하지 않은 Projectile 수입니다."))
	int32 LauncherRemainingProjectileCount = 0;
};

/**
 * 선택 Target을 Gameplay Target Data보다 좁은 Player-facing Knowledge로 변환한 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 HUD 데이터 (Target HUD Data)", ToolTip="현재 선택 Target의 공개 가능한 표시 정보와 추적 상태만 UI에 전달합니다."))
struct CARFIGHT_RE_API FCFTargetHUDData
{
	GENERATED_BODY()

	// [v1.0.0] TargetSelect 채널의 가용 상태 또는 선택 없음 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 데이터 상태", ToolTip="TargetSelect Provider 부재, 선택 없음 또는 실제 선택 상태를 구분합니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 명시적인 선택 기록이 존재하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="선택 타겟 존재", ToolTip="TargetSelectComp에 현재 선택 기록이 있으면 True입니다."))
	bool bHasSelectedTarget = false;

	// [v1.0.0] 현재 선택 Target의 논리 유효성입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="선택 타겟 유효", ToolTip="현재 선택 기록과 약한 Actor 참조가 실제 유효하면 True입니다."))
	bool bSelectedTargetValid = false;

	// [v1.0.0] 공개 DisplayInfo가 제공한 안정 Target ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 ID", ToolTip="TargetSelectable이 공개한 안정적인 TargetId입니다."))
	FName TargetId = NAME_None;

	// [v1.0.0] Target identity 공개 수준을 반영한 이름 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 이름 상태", ToolTip="Identified 이상에서만 Known이 되며 Detected 단계에서는 Unknown입니다."))
	ECFUIViewAvailability IdentityAvailability = ECFUIViewAvailability::Unknown;

	// [v1.0.0] 공개가 허용될 때 표시할 Target 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 표시 이름", ToolTip="TargetSelectable이 제공한 Player-facing DisplayName입니다."))
	FText DisplayName;

	// [v1.0.0] TargetSelectable이 공개한 관계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 관계", ToolTip="미확인, 아군, 중립 또는 적대 관계입니다."))
	ECFTargetRelation Relation = ECFTargetRelation::Unknown;

	// [v1.0.0] TargetSelectable이 공개한 큰 분류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 분류", ToolTip="차량, 장치, 환경 또는 미분류 상태입니다."))
	ECFTargetCategory Category = ECFTargetCategory::Unknown;

	// [v1.0.0] 현재 공개 가능한 Target 정보 단계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 정보 단계", ToolTip="Detected, Identified, DetailedScan 등 현재 공개 수준입니다."))
	ECFTargetInfoLevel InformationLevel = ECFTargetInfoLevel::None;

	// [v1.0.0] 현재 선택 Target의 추적 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 추적 상태", ToolTip="Visible, Occluded, Estimated, SignalLost 또는 Invalid 상태입니다."))
	ECFTargetTrackState TrackState = ECFTargetTrackState::Invalid;

	// [v1.0.0] 거리 표시 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 거리 상태", ToolTip="Sensor/Knowledge Provider가 Player-facing 거리를 제공하기 전에는 Unavailable입니다."))
	ECFUIViewAvailability DistanceAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 Sensor/Knowledge Provider가 제공할 때 사용할 거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(Units="m", DisplayName="타겟 거리 m", ToolTip="Player-facing Sensor/Knowledge Provider가 제공한 거리만 사용합니다."))
	float DistanceMeters = 0.0f;
};

/**
 * Sensor Provider가 후속으로 채울 Radar Contact 한 건의 공개 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="레이더 Contact HUD 데이터 (Radar Contact HUD Data)", ToolTip="Sensor Provider가 공개한 Contact ID, 관계와 Radar 정규화 위치를 전달합니다."))
struct CARFIGHT_RE_API FCFRadarContactHUDData
{
	GENERATED_BODY()

	// [v1.0.0] Sensor Contact의 안정적인 표시 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact ID", ToolTip="Radar Contact를 프레임 사이에서 안정적으로 식별할 ID입니다."))
	FName ContactId = NAME_None;

	// [v1.0.0] Contact 관계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact 관계", ToolTip="미확인, 아군, 중립 또는 적대 관계입니다."))
	ECFTargetRelation Relation = ECFTargetRelation::Unknown;

	// [v1.0.0] Radar 중심 기준 -1~1 정규화 평면 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 정규화 위치", ToolTip="Radar 중심을 0,0으로 한 -1~1 범위 표시 위치입니다."))
	FVector2D NormalizedPosition = FVector2D::ZeroVector;

	// [v1.0.0] 현재 선택 Target과 같은 Contact인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="선택 Contact", ToolTip="현재 TargetSelect의 선택 대상과 같은 Sensor Contact이면 True입니다."))
	bool bSelected = false;
};

/**
 * Radar/Sensor 채널의 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="레이더 HUD 데이터 (Radar HUD Data)", ToolTip="실제 Sensor Provider의 Radar 가용 상태와 Contact 목록을 전달합니다."))
struct CARFIGHT_RE_API FCFRadarHUDData
{
	GENERATED_BODY()

	// [v1.0.0] 실제 Sensor/Radar Provider 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="레이더 데이터 상태", ToolTip="Sensor Runtime Provider가 없으면 Unavailable이며 Target 후보로 가짜 Contact를 만들지 않습니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 Sensor Provider가 공개한 Contact 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="레이더 Contact 목록", ToolTip="Sensor Provider가 만든 Contact만 포함합니다."))
	TArray<FCFRadarContactHUDData> Contacts;
};

/**
 * Alert Feed에 표시할 현재 상태 기반 한 항목입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="HUD 경고 항목 (HUD Alert Item)", ToolTip="AlertKey, 우선순위, 제목과 보조 문구를 전달합니다."))
struct CARFIGHT_RE_API FCFHUDAlertItem
{
	GENERATED_BODY()

	// [v1.0.0] 같은 의미 Alert의 중복 제거에 사용할 안정 키입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Alert", meta=(DisplayName="경고 키", ToolTip="같은 의미의 현재 상태 Alert를 중복 표시하지 않기 위한 안정 키입니다."))
	FName AlertKey = NAME_None;

	// [v1.0.0] Alert 표시 우선순위입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Alert", meta=(DisplayName="경고 우선순위", ToolTip="정보, 주의 또는 위험 우선순위입니다."))
	ECFHUDAlertPriority Priority = ECFHUDAlertPriority::Info;

	// [v1.0.0] Player-facing Alert 제목입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Alert", meta=(DisplayName="경고 제목", ToolTip="Gameplay 내부 이름을 포함하지 않는 Player-facing Alert 제목입니다."))
	FText Title;

	// [v1.0.0] 선택적인 Player-facing Alert 보조 문구입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Alert", meta=(DisplayName="경고 보조 문구", ToolTip="수량이나 상태를 간단히 보충할 Player-facing 문구입니다."))
	FText Detail;
};

/**
 * Alert Provider가 만든 현재 활성 Alert 목록입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="전투 경고 HUD 데이터 (Combat Alert HUD Data)", ToolTip="실제 Gameplay 상태에서 파생된 중복 없는 현재 Alert 목록을 전달합니다."))
struct CARFIGHT_RE_API FCFCombatAlertViewData
{
	GENERATED_BODY()

	// [v1.0.0] Alert Provider 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Alert", meta=(DisplayName="경고 데이터 상태", ToolTip="현재 플레이어 차량 Runtime이 없으면 Unavailable입니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 우선순위와 안정 AlertKey를 가진 현재 활성 Alert 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Alert", meta=(DisplayName="활성 경고 목록", ToolTip="현재 실제 Gameplay 상태로부터 생성된 Alert만 포함합니다."))
	TArray<FCFHUDAlertItem> ActiveAlerts;
};

/**
 * Production HUD 한 프레임이 소비할 전체 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="인게임 UI ViewData (InGame UI View Data)", ToolTip="Vehicle, Weapon, Defense, Target, Radar와 Alert ViewData를 한 번에 전달합니다."))
struct CARFIGHT_RE_API FCFInGameUIViewData
{
	GENERATED_BODY()

	// [v1.0.0] Provider가 새 ViewData를 만든 순번입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="ViewData Revision", ToolTip="Provider가 ViewData를 새로 계산할 때 증가하는 Revision입니다."))
	int32 Revision = 0;

	// [v1.0.0] Current Pawn Rebind가 실제 발생한 순번입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="Pawn Binding Generation", ToolTip="OnCurrentPawnChanged 기반 Rebind가 발생할 때 증가하며 Old Pawn 데이터 잔류를 진단할 수 있습니다."))
	int32 BindingGeneration = 0;

	// [v1.0.0] 플레이어 차량 주행 ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="차량 HUD 데이터"))
	FCFVehicleHUDData Vehicle;

	// [v1.0.0] 플레이어 차량 무기 ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="무기 HUD 데이터"))
	FCFWeaponHUDData Weapon;

	// [v1.0.0] 플레이어 차량 방어 ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="방어 HUD 데이터"))
	FCFDefenseHUDData Defense;

	// [v1.0.0] 현재 선택 Target ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="타겟 HUD 데이터"))
	FCFTargetHUDData Target;

	// [v1.0.0] Radar/Sensor ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="레이더 HUD 데이터"))
	FCFRadarHUDData Radar;

	// [v1.0.0] 현재 전투 Alert ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="전투 경고 HUD 데이터"))
	FCFCombatAlertViewData Alerts;
};
