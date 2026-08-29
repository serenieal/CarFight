// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.15.0
// Date: 2026-08-25
// Description: CF-FQ-039 차량별 HUD silhouette identity + 기존 HUD ViewData 계약
// Scope: VehicleData visual identity, Vehicle, ViewMode, Weapon, Defense, Target, Radar Range·Contact 표시 상태와 Alert의 Player-facing ViewData 계약을 제공합니다.
// Changelog:
// - v1.15.0: 현재 차량의 VehicleData Soft identity를 Vehicle HUD Data에 additive 전달해 Presenter가 HUD Visual catalog에서 차종별 Armor Body Map silhouette를 선택할 수 있게 함. Texture와 Gameplay 설정은 ViewData에 넣지 않음.
// - v1.14.0: 기존 VehicleCamera/Aim Runtime을 재계산하지 않고 Camera Mode, 차량 Heading, 카메라·터렛의 차량 기준 상대 Yaw/Pitch를 전달하는 ViewMode HUD 계약을 추가.
// - v1.13.0: Radar에 Display/Maximum Range, Preset index/count/zoom 가능 상태와 Contact의 range-inside/selected-edge 방향 계약을 추가. NormalizedPosition은 명시 Range가 있을 때만 사용.
// - v1.12.0: UI-P0-06 실제 WeaponCharge Runtime의 Current/Maximum/Ratio/Insufficient를 ViewData로 전달하는 계약을 추가. VehicleBattery는 계속 Unavailable.
// - v1.11.0: Applied Fitting 고정 표시 순서 기반 Player-facing Weapon Selection 목록과 SelectedWeaponIndex를 추가. 각 항목은 EquipmentPresetData.DisplayName만 노출하고 내부 MountProfileId/WeaponId를 포함하지 않음.
// - v1.10.0: UI-P0-06 실제 Weapon Heat Runtime의 Current/Maximum/Ratio/Overheated를 ViewData로 전달하는 계약을 추가. Battery·Charge는 계속 Unavailable.
// - v1.9.0: UI-P0-06 RPM Gauge가 Current Chaos RPM과 별개로 explicit RedlineStartRPM/EngineMaxRPM source를 전달받도록 Vehicle HUD 계약을 확장. Redline 미설정은 Unavailable이며 자동 추정 금지.
// - v1.8.0: UI-P0-06 Player-facing Weapon DisplayName Source를 호환되는 활성 EquipmentPresetData.DisplayName으로 명확화. 내부 ID/AssetName fallback 금지.
// - v1.7.0: UI-P0-06 기존 실제 Ammo·Reserve·Cooldown·Reload·LauncherSequence를 additive 공통 Weapon Resource Channel로 투영하는 타입과 ResourceChannels 계약을 추가. Battery·Charge·Heat는 실제 Runtime 전 채널 생성 금지.
// - v1.6.0: UI-P0-06 실제 Chaos Vehicle Runtime의 Engine RPM과 Current Gear가 기존 Vehicle HUD 필드를 채우는 계약을 활성화. RPM Gauge 정규화/Redline 추정은 추가하지 않음.
// - v1.5.0: UI-P0-06 착수 감사에 따라 실제 Provider가 없는 VehicleBattery·WeaponCharge 채널을 명시적 Unavailable 계약으로 추가하고 값 추정을 금지.
// - v1.4.0: Target/Radar가 Sensor Snapshot의 ContactId, ContactState, Knowledge, 상대 위치·거리, freshness, analysis와 DestroyedHold 의미를 그대로 전달하도록 확장.
// - v1.3.0: Launcher 이벤트에 의한 의미 상태 전이와 Ammo/Timer 부수 Refresh를 구분하도록 LauncherSequenceRevision을 추가. FirePattern과 Presentation 수명 정책을 분리.
// - v1.2.0: WeaponPanel의 Loaded/Capacity + label-less Reserve 계약을 위해 MagazineCapacity를 공개 ViewData에 추가하고 Immediate/CurrentUsable을 내부 판정값으로 명시.
// - v1.1.0: AMMO-P0-06 실제 Ammo Runtime의 장전·예약·예비·사용 가능량, Reload 상태·시간과 Action Lock을 Weapon ViewData에 추가.
// - v1.0.0: UI-P0-03 최초 ViewData 계약과 통합 FCFInGameUIViewData를 추가.
// Migration:
// - v1.12.0 WeaponChargeAvailability이 Known/KnownZero일 때만 CurrentWeaponCharge/MaximumWeaponCharge/WeaponChargeRatio/bWeaponChargeInsufficient를 소비하며 VehicleBattery나 정적 설정으로 현재 Charge를 추정하지 않습니다.
// - v1.11.0 Weapon Selection은 Provider가 Applied Fitting 고정 순서를 표시 순서로 전달하며 내부 MountProfileId, WeaponId, EquipmentId와 Asset 이름을 선택 항목에 넣지 않습니다. Production Rail Visual은 별도 consumer migration 전까지 Collapsed를 유지합니다.
// - v1.10.0 HeatAvailability이 Known/KnownZero일 때만 CurrentHeat/MaximumHeat/HeatRatio/bWeaponOverheated를 소비하며 정적 WeaponData만으로 현재 Heat를 만들지 않습니다.
// - Gameplay Runtime 타입을 Widget에서 직접 읽지 않고 UCFHUDDataProvider가 이 타입으로 변환합니다.
// - v1.4.0부터 Target Knowledge와 Radar Contact는 FCFSensorSnapshot만 의미 source로 사용합니다. TargetSelect는 선택 상태와 TrackState만 제공합니다.
// - v1.13.0부터 Radar 표시 Range는 Scanner가 명시한 Range Preset만 사용합니다. Preset이 없으면 기존처럼 Range/NormalizedPosition을 Unavailable로 유지하며 Sensor 탐지 거리로 임의 대체하지 않습니다.
// - Radar NormalizedPosition은 차량 Heading-Up 평면의 `X=전방, Y=우측` 의미를 보존합니다. 범위 밖 Contact는 unit edge로 clamp하되 bInsideDisplayRange로 구분하고 선택 Contact만 bShowSelectedEdgeMarker를 사용할 수 있습니다.
// - EngineRpm은 Chaos Runtime 현재값이고 EngineRedlineStartRpm/EngineMaximumRpm은 VehicleData의 명시 authored 값입니다. RedlineStartRPM=0은 Unavailable이며 EngineMaxRPM 또는 변속값으로 추정하지 않습니다.
// - Heat처럼 실제 Runtime Provider가 없는 채널은 값을 추정하지 않고 Unavailable로 유지하며 finite Ammo는 VehicleAmmoComp Snapshot만 사용합니다.
// - Target Actor 이름이나 Component 이름 같은 내부 식별자는 Player-facing Text로 사용하지 않습니다.
// - v1.15.0 VehicleDataAsset은 HUD Visual 선택용 identity일 뿐 Player-facing Text가 아니며 Presenter가 Gameplay VehicleData 필드를 읽는 용도로 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFLauncherTypes.h"
#include "CFSensorTypes.h"
#include "CFVehicleCameraTypes.h"
#include "CFTargetSelectTypes.h"
#include "CFHUDViewData.generated.h"

class UCFVehicleData;

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
 * WeaponPanel 공통 자원 채널이 표현할 Gameplay 자원 종류입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="무기 자원 채널 종류 (Weapon Resource Channel Type)", ToolTip="Ammo, Reserve, Battery, Charge, Heat, Cooldown, Reload, LauncherSequence를 같은 ViewData 경계에서 구분합니다. 실제 Runtime이 없는 종류는 채널을 생성하지 않습니다."))
enum class ECFWeaponResourceChannelType : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	Ammo UMETA(DisplayName="탄약 (Ammo)"),
	ReserveAmmo UMETA(DisplayName="예비 탄약 (Reserve Ammo)"),
	VehicleBattery UMETA(DisplayName="차량 배터리 (Vehicle Battery)"),
	WeaponCharge UMETA(DisplayName="무기 충전 (Weapon Charge)"),
	Heat UMETA(DisplayName="열 (Heat)"),
	Cooldown UMETA(DisplayName="쿨다운 (Cooldown)"),
	Reload UMETA(DisplayName="재장전 (Reload)"),
	LauncherSequence UMETA(DisplayName="런처 시퀀스 (Launcher Sequence)")
};

/**
 * 공통 자원 채널을 어떤 표시 형태로 해석할지 Presenter에 전달하는 의미 타입입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="무기 자원 표시 방식 (Weapon Resource Display Mode)", ToolTip="Count, CountPair, Ratio, Time, Progress, Sequence처럼 자원 의미에 맞는 표시 방식을 구분합니다. Widget이 Gameplay 원본을 보고 형태를 추정하지 않습니다."))
enum class ECFWeaponResourceDisplayMode : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	Count UMETA(DisplayName="단일 수량 (Count)"),
	CountPair UMETA(DisplayName="현재/최대 수량 (Count Pair)"),
	RatioBar UMETA(DisplayName="비율 바 (Ratio Bar)"),
	Percent UMETA(DisplayName="퍼센트 (Percent)"),
	TimeRemaining UMETA(DisplayName="남은 시간 (Time Remaining)"),
	Progress UMETA(DisplayName="진행률 (Progress)"),
	Sequence UMETA(DisplayName="시퀀스 (Sequence)"),
	TextOnly UMETA(DisplayName="텍스트 전용 (Text Only)")
};

/**
 * 실제 Gameplay Runtime 한 채널을 WeaponPanel 공통 표현 계층에 전달하는 읽기 전용 ViewData입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 자원 HUD 데이터 (Weapon Resource HUD Data)", ToolTip="실제 Gameplay Runtime에서 확인된 한 자원의 현재값, 최대값, 진행률과 활성·표시 상태를 전달합니다. 값 존재 여부를 별도 플래그로 명시해 0과 데이터 부재를 혼동하지 않습니다."))
struct CARFIGHT_RE_API FCFWeaponResourceHUDData
{
	GENERATED_BODY()

	// [v1.7.0] 이 공통 채널이 표현하는 자원 종류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="자원 채널 종류", ToolTip="Ammo, ReserveAmmo, VehicleBattery, WeaponCharge, Heat, Cooldown, Reload 또는 LauncherSequence 중 실제 Source가 있는 종류입니다."))
	ECFWeaponResourceChannelType ChannelType = ECFWeaponResourceChannelType::None;

	// [v1.7.0] Presenter가 이 채널 값을 표시할 기본 형태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="자원 표시 방식", ToolTip="수량, 현재/최대, 비율, 남은 시간, 진행률 또는 시퀀스 중 이 채널의 의미에 맞는 표시 방식입니다."))
	ECFWeaponResourceDisplayMode DisplayMode = ECFWeaponResourceDisplayMode::None;

	// [v1.7.0] 이 채널의 실제 값 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="자원 데이터 상태", ToolTip="실제 Provider가 제공한 Known/KnownZero 상태를 보존합니다. Provider가 없는 채널은 ResourceChannels 배열에 만들지 않습니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.7.0] CurrentValue가 실제 의미를 가지는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="현재값 존재", ToolTip="True일 때만 CurrentValue를 실제 자원 값으로 사용합니다."))
	bool bHasCurrentValue = false;

	// [v1.7.0] 실제 Runtime이 제공한 현재 자원 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="현재값", ToolTip="bHasCurrentValue가 True일 때만 유효합니다. 수량·시간·진행 단위는 ChannelType과 DisplayMode가 결정합니다."))
	float CurrentValue = 0.0f;

	// [v1.7.0] MaximumValue가 실제 의미를 가지는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="최대값 존재", ToolTip="True일 때만 MaximumValue를 실제 최대값으로 사용합니다."))
	bool bHasMaximumValue = false;

	// [v1.7.0] 실제 Runtime이 제공한 최대 자원 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="최대값", ToolTip="bHasMaximumValue가 True일 때만 유효합니다."))
	float MaximumValue = 0.0f;

	// [v1.7.0] NormalizedValue가 실제 계산 가능한지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="정규화 값 존재", ToolTip="실제 현재/최대 또는 Runtime 진행 정보로 0~1 비율을 안전하게 만들 수 있을 때만 True입니다."))
	bool bHasNormalizedValue = false;

	// [v1.7.0] 실제 Runtime 의미를 보존한 0~1 정규화 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="정규화 값", ToolTip="bHasNormalizedValue가 True일 때만 Bar/Progress에 사용합니다."))
	float NormalizedValue = 0.0f;

	// [v1.7.0] RemainingTimeSeconds가 실제 의미를 가지는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="남은 시간 존재", ToolTip="Cooldown이나 Reload처럼 실제 Runtime이 남은 시간을 제공할 때만 True입니다."))
	bool bHasRemainingTimeSeconds = false;

	// [v1.7.0] 실제 Runtime이 제공한 남은 게임 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(Units="s", DisplayName="남은 시간 초", ToolTip="bHasRemainingTimeSeconds가 True일 때만 유효합니다. Pause 중 진행 여부는 상위 Runtime 의미를 그대로 따릅니다."))
	float RemainingTimeSeconds = 0.0f;

	// [v1.7.0] 이 채널에 대응하는 실제 행동 상태가 현재 진행 중인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="자원 상태 활성", ToolTip="Cooldown, Reload, LauncherSequence처럼 시간/행동 상태가 현재 진행 중일 때 True입니다. Ammo/Reserve 같은 상시 수량 채널은 True입니다."))
	bool bIsActive = false;

	// [v1.7.0] 현재 기본 WeaponPanel 정책에서 이 채널을 표시할지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="자원 채널 표시", ToolTip="지원되지만 현재 비활성인 Cooldown/Reload/LauncherSequence는 False일 수 있습니다. Provider가 없는 채널은 배열 자체에 생성하지 않습니다."))
	bool bIsVisible = false;

	// [v1.7.0] 이 채널에 대응하는 실제 Runtime 상태가 현재 신규 발사를 막는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Resource", meta=(DisplayName="발사 차단", ToolTip="실제 Ammo/ActionLock/Cooldown 상태가 발사를 막는 경우만 True입니다. 자원 이름으로 추정하지 않습니다."))
	bool bBlocksFire = false;
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

	// [v1.15.0] HUD Visual catalog가 차종별 실루엣을 선택할 현재 VehicleData Asset identity입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="차량 Visual Identity", ToolTip="현재 Pawn이 사용하는 VehicleData Asset의 Soft identity입니다. Presenter는 이 값으로 HUD 실루엣만 선택하며 VehicleData Gameplay 설정을 조회하지 않습니다."))
	TSoftObjectPtr<UCFVehicleData> VehicleDataAsset;

	// [v1.0.0] 실제 차량 속도 값의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="속도 데이터 상태", ToolTip="현재 속도가 실제 Runtime에서 제공됐는지 나타냅니다."))
	ECFUIViewAvailability SpeedAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 실제 VehicleDriveComp 기준 차량 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(Units="km/h", DisplayName="차량 속도 km/h", ToolTip="Vehicle Runtime에서 읽은 현재 차량 속도입니다."))
	float SpeedKmh = 0.0f;

			// [v1.6.0] 실제 Engine RPM 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="엔진 RPM 데이터 상태", ToolTip="VehicleDriveComp가 실제 UE 5.8 Chaos Movement를 제공할 때만 Known/KnownZero입니다. 속도로 RPM을 추정하지 않습니다."))
	ECFUIViewAvailability EngineRpmAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.6.0] 실제 Chaos Vehicle Mechanical Simulation의 현재 엔진 RPM 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(Units="rpm", DisplayName="엔진 RPM", ToolTip="UE 5.8 Chaos Wheeled Vehicle Movement의 실제 현재 Engine RPM입니다. 물리 상한이나 변속값에서 추정하지 않습니다."))
	float EngineRpm = 0.0f;

	// [v1.9.0] 차량별 명시 RedlineStartRPM을 HUD에 전달할 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="레드라인 시작 RPM 데이터 상태", ToolTip="VehicleData에 RedlineStartRPM이 명시됐는지 나타냅니다. 0은 미설정이며 EngineMaxRPM이나 변속값으로 보정하지 않습니다."))
	ECFUIViewAvailability EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.9.0] HUD Tachometer 85% Red Zone 시작점에 매핑할 실제 차량별 RedlineStartRPM입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(Units="rpm", DisplayName="레드라인 시작 RPM", ToolTip="VehicleData가 명시한 실제 RedlineStartRPM입니다. HUD Presenter가 이 값을 85% 화면 위치에 매핑합니다."))
	float EngineRedlineStartRpm = 0.0f;

	// [v1.9.0] VehicleData에서 실제 Chaos EngineSetup.MaxRPM으로 적용되는 물리 최대 RPM의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="엔진 최대 RPM 데이터 상태", ToolTip="VehicleData의 실제 EngineMaxRPM을 HUD 최대 눈금 source로 사용할 수 있는지 나타냅니다."))
	ECFUIViewAvailability EngineMaximumRpmAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.9.0] HUD Tachometer 100% 끝점에 매핑할 실제 물리 EngineMaxRPM입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(Units="rpm", DisplayName="엔진 최대 RPM", ToolTip="VehicleData의 실제 EngineMaxRPM입니다. Chaos EngineSetup.MaxRPM과 같은 source이며 Redline 시작값이 아닙니다."))
	float EngineMaximumRpm = 0.0f;

	// [v1.6.0] 실제 Chaos Transmission Gear 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Vehicle", meta=(DisplayName="기어 데이터 상태", ToolTip="VehicleDriveComp가 실제 UE 5.8 Chaos Movement를 제공할 때 Known입니다. 차량 속도나 입력으로 단수를 추정하지 않습니다."))
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
 * 외부 3인칭 HUD가 차체·카메라·터렛 방향을 같은 기준으로 표시할 수 있게 하는 읽기 전용 View Mode 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="뷰 모드 HUD 데이터 (View Mode HUD Data)", ToolTip="기존 VehicleCamera/Aim Runtime이 계산한 카메라 모드와 차체 기준 방향 차이를 UI에 전달합니다. 새 카메라 모드나 터렛 동작을 만들지 않습니다."))
struct CARFIGHT_RE_API FCFViewModeHUDData
{
	GENERATED_BODY()

	// [v1.14.0] 현재 View Mode 방향 데이터의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(DisplayName="뷰 모드 데이터 상태", ToolTip="현재 차량과 VehicleCameraComp에서 실제 방향 정보를 읽을 수 있을 때 Known입니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.14.0] VehicleCameraComp가 이미 판정한 현재 대표 카메라 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(DisplayName="현재 카메라 모드", ToolTip="Normal, Combat, Reverse, Airborne, Destroyed 또는 Spectate 중 VehicleCameraComp가 실제 판정한 현재 모드입니다."))
	ECFVehicleCameraMode CameraMode = ECFVehicleCameraMode::Normal;

	// [v1.14.0] 월드 +X를 0도로 한 현재 차량 차체의 수평 Heading입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(Units="deg", DisplayName="차량 Heading", ToolTip="현재 차량 Actor Forward의 월드 수평 Heading을 0~360도로 정규화한 값입니다."))
	float VehicleHeadingDegrees = 0.0f;

	// [v1.14.0] 차체 전방을 0도로 한 실제 카메라 시선의 좌우 상대각입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(Units="deg", DisplayName="카메라 상대 Yaw", ToolTip="차체 전방 대비 현재 VehicleCameraComp 시선의 좌우 상대각입니다. 음수는 좌측, 양수는 우측입니다."))
	float CameraRelativeYawDegrees = 0.0f;

	// [v1.14.0] 현재 VehicleCameraComp 시선의 수평 기준 상하 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(Units="deg", DisplayName="카메라 Pitch", ToolTip="현재 카메라 시선의 상하 각도입니다. 양수는 위쪽입니다."))
	float CameraPitchDegrees = 0.0f;

	// [v1.14.0] 현재 활성 Weapon Aim Solution이 실제 Muzzle 방향을 제공하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(DisplayName="터렛 방향 사용 가능", ToolTip="유효한 Weapon Aim Solution의 CurrentMuzzleDirection을 읽을 수 있을 때 True입니다."))
	bool bTurretDirectionAvailable = false;

	// [v1.14.0] 차체 전방을 0도로 한 현재 Muzzle/터렛의 좌우 상대각입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(Units="deg", DisplayName="터렛 상대 Yaw", ToolTip="차체 전방 대비 현재 Muzzle 방향의 좌우 상대각입니다. bTurretDirectionAvailable이 True일 때만 유효합니다."))
	float TurretRelativeYawDegrees = 0.0f;

	// [v1.14.0] 현재 Muzzle/터렛 방향의 수평 기준 상하 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(Units="deg", DisplayName="터렛 Pitch", ToolTip="현재 Muzzle 방향의 상하 각도입니다. bTurretDirectionAvailable이 True일 때만 유효합니다."))
	float TurretPitchDegrees = 0.0f;

	// [v1.14.0] 기존 Weapon Aim Solution이 터렛 정렬 진행 중으로 판정했는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|ViewMode", meta=(DisplayName="터렛 정렬 중", ToolTip="VehicleAimComp가 제공한 bTurretAligning을 그대로 전달합니다. UI가 정렬 여부를 재계산하지 않습니다."))
	bool bTurretAligning = false;
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
 * Applied Fitting의 Player-facing 고정 표시 순서에서 한 무기 슬롯의 공개 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 선택 HUD 항목 (Weapon Selection HUD Item)", ToolTip="Applied Fitting에서 실제 장착된 무기의 고정 표시 순서, DisplayName 가용 상태와 현재 선택 여부만 UI에 전달합니다. 내부 MountProfileId나 WeaponId는 포함하지 않습니다."))
struct CARFIGHT_RE_API FCFWeaponSelectionHUDItem
{
	GENERATED_BODY()

	// [v1.11.0] 이 항목의 Player-facing DisplayName 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Selection", meta=(DisplayName="무기 선택 표시 이름 상태", ToolTip="실제 EquipmentPresetData.DisplayName이 비어 있지 않을 때만 Known입니다. 내부 ID나 Asset 이름으로 fallback하지 않습니다."))
	ECFUIViewAvailability DisplayNameAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.11.0] 실제 EquipmentPresetData가 명시한 Player-facing 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Selection", meta=(DisplayName="무기 선택 표시 이름", ToolTip="Applied Fitting의 실제 EquipmentPresetData.DisplayName만 전달합니다. 내부 MountProfileId, WeaponId, EquipmentId와 Asset 이름은 전달하지 않습니다."))
	FText DisplayName;

	// [v1.11.0] 이 고정 표시 순번이 현재 활성 무기인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Selection", meta=(DisplayName="현재 선택 무기", ToolTip="Provider 고정 표시 순서에서 현재 WeaponComp SelectedWeaponIndex와 같은 항목이면 True입니다."))
	bool bSelected = false;
};

/**
 * 현재 활성 무기와 Launcher 진행 상태의 표시 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="무기 HUD 데이터 (Weapon HUD Data)", ToolTip="현재 활성 무기, 실제 선택 가능 무기 목록, 쿨다운과 Launcher 시퀀스 상태를 UI에 전달합니다."))
struct CARFIGHT_RE_API FCFWeaponHUDData
{
	GENERATED_BODY()

	// [v1.7.0] 기존 실제 Weapon 필드를 additive 공통 ResourceChannels 배열로 다시 투영합니다.
	void RebuildResourceChannelsFromCurrentFields();

	// [v1.0.0] 활성 무기 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 데이터 상태", ToolTip="VehicleWeapon Runtime과 활성 WeaponData 존재 상태를 구분합니다."))
	ECFUIViewAvailability Availability = ECFUIViewAvailability::Unavailable;

	// [v1.0.0] 내부 비교와 Provider 진단에 사용할 안정 Weapon ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 ID", ToolTip="Provider 내부 안정 식별용 WeaponId입니다. Player-facing 이름으로 직접 표시하지 않습니다."))
	FName WeaponId = NAME_None;

		// [v1.8.0] Player-facing 무기 이름 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 표시 이름 상태", ToolTip="호환되는 현재 활성 EquipmentPresetData에 비어 있지 않은 Player-facing DisplayName이 있을 때만 Known입니다. 내부 ID나 Asset 이름으로 추정하지 않습니다."))
	ECFUIViewAvailability DisplayNameAvailability = ECFUIViewAvailability::Unavailable;

		// [v1.8.0] 현재 활성 EquipmentPresetData가 명시한 Player-facing 장비 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 표시 이름", ToolTip="EquipmentPresetData.DisplayName을 그대로 사용하는 Player-facing 장비 이름입니다. WeaponId, EquipmentId, MountProfileId 또는 Asset 이름을 fallback으로 사용하지 않습니다."))
	FText DisplayName;

	// [v1.11.0] 실제 Applied Fitting 기반 Weapon Selection 목록의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Selection", meta=(DisplayName="무기 선택 목록 상태", ToolTip="WeaponComp가 Applied Fitting의 실제 weapon-bearing 고정 순서를 선택 Runtime으로 보유하면 Known입니다. Legacy 단일 무기 경로는 Unavailable입니다."))
	ECFUIViewAvailability WeaponSelectionAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.11.0] SelectableWeapons 고정 표시 순서에서 현재 활성 무기의 0-based 인덱스입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Selection", meta=(DisplayName="현재 선택 무기 인덱스", ToolTip="Applied Fitting 기반 고정 표시 순서의 0-based SelectedWeaponIndex입니다. 내부 MountProfileId를 UI 그룹으로 사용하지 않습니다."))
	int32 SelectedWeaponIndex = INDEX_NONE;

	// [v1.11.0] Applied Fitting의 실제 weapon-bearing mount 고정 순서로 만든 Player-facing 무기 선택 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon|Selection", meta=(DisplayName="선택 가능 무기 목록", ToolTip="각 항목은 DisplayName과 선택 여부만 포함합니다. 내부 MountProfileId, WeaponId, EquipmentId 또는 Asset 이름은 포함하지 않습니다."))
	TArray<FCFWeaponSelectionHUDItem> SelectableWeapons;

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

		// [v1.5.0] 차량 배터리 Runtime 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="차량 배터리 데이터 상태", ToolTip="실제 VehicleBattery Runtime Provider가 없으면 Unavailable입니다. 무기 종류나 정적 설정만으로 배터리 값을 추정하지 않습니다."))
	ECFUIViewAvailability VehicleBatteryAvailability = ECFUIViewAvailability::Unavailable;

		// [v1.12.0] 무기 내부 Charge Runtime 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 충전 데이터 상태", ToolTip="실제 활성 WeaponCharge Runtime이 있으면 Known 또는 KnownZero입니다. 차량 배터리나 쿨다운을 충전량으로 대신 사용하지 않습니다."))
	ECFUIViewAvailability WeaponChargeAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.12.0] 현재 활성 무기에 실제 남아 있는 내부 Charge입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="현재 무기 Charge", ToolTip="WeaponComp의 실제 per-weapon Charge Runtime에서 읽은 현재 내부 충전량입니다."))
	float CurrentWeaponCharge = 0.0f;

	// [v1.12.0] 현재 활성 무기의 명시된 최대 내부 Charge입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="최대 무기 Charge", ToolTip="현재 활성 WeaponCharge Runtime의 최대 내부 충전량입니다. Charge Runtime이 없으면 0입니다."))
	float MaximumWeaponCharge = 0.0f;

	// [v1.12.0] 현재 Charge를 최대 Charge 기준으로 정규화한 0~1 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="무기 Charge 비율", ToolTip="현재 WeaponCharge를 MaximumWeaponCharge로 나눈 실제 Runtime 비율입니다."))
	float WeaponChargeRatio = 0.0f;

	// [v1.12.0] 현재 활성 무기의 Charge가 다음 표준 한 발 소비량보다 부족한지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 Charge 부족 상태", ToolTip="현재 활성 무기의 실제 내부 Charge가 다음 표준 한 발의 명시 소비량보다 부족해 발사가 차단되면 True입니다."))
	bool bWeaponChargeInsufficient = false;

		// [v1.10.0] Heat Runtime 채널의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="열 데이터 상태", ToolTip="실제 활성 Weapon Heat Runtime이 있으면 Known 또는 KnownZero입니다. 정적 WeaponData 설정만으로 현재 열을 추정하지 않습니다."))
	ECFUIViewAvailability HeatAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.10.0] 현재 활성 무기에 실제 누적된 Heat입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="현재 무기 Heat", ToolTip="WeaponComp의 실제 per-weapon Heat Runtime에서 읽은 현재 누적 열량입니다."))
	float CurrentHeat = 0.0f;

	// [v1.10.0] 현재 활성 무기의 명시된 최대 Heat입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="최대 무기 Heat", ToolTip="현재 활성 Heat Runtime의 MaxHeat입니다. Heat Runtime이 없으면 0입니다."))
	float MaximumHeat = 0.0f;

	// [v1.10.0] 현재 Heat를 최대 Heat 기준으로 정규화한 0~1 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="무기 Heat 비율", ToolTip="현재 Heat를 MaximumHeat로 나눈 0부터 1까지의 실제 Runtime 비율입니다."))
	float HeatRatio = 0.0f;

	// [v1.10.0] 현재 활성 무기가 MaxHeat 도달 후 냉각 대기 중인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 과열 상태", ToolTip="현재 활성 무기가 과열되어 다음 표준 한 발을 위한 Heat 여유가 생길 때까지 발사가 차단되면 True입니다."))
	bool bWeaponOverheated = false;

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

				// [v1.12.0] 실제 Gameplay Source가 존재하는 자원만 담는 additive 공통 Weapon Resource 채널 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Weapon", meta=(DisplayName="무기 자원 채널 목록", ToolTip="현재 실제 Runtime이 제공하는 Ammo, ReserveAmmo, WeaponCharge, Cooldown, Reload, LauncherSequence와 Heat를 투영합니다. VehicleBattery는 실제 Runtime이 생기기 전 항목을 만들지 않습니다."))
	TArray<FCFWeaponResourceHUDData> ResourceChannels;
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

	// [v1.4.0] 현재 선택 Target에 대응하는 Sensor Contact가 공개 Snapshot에 존재하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="Sensor Contact 상태", ToolTip="선택 Target과 연결된 ContactId가 현재 FCFSensorSnapshot에 있으면 Known이며, Sensor가 준비됐지만 Contact가 없으면 Unknown입니다."))
	ECFUIViewAvailability SensorContactAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.4.0] 선택 Target과 연결된 Sensor Contact의 독립 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="Sensor Contact ID", ToolTip="TargetId와 별개인 Sensor ContactId입니다. 선택 Actor의 metadata 대신 Sensor bridge로 연결한 뒤 Snapshot에서 읽습니다."))
		FName ContactId = NAME_None;

	// [v1.0.0] 현재 선택 Target의 논리 유효성입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="선택 타겟 유효", ToolTip="현재 선택 기록과 약한 Actor 참조가 실제 유효하면 True입니다."))
	bool bSelectedTargetValid = false;

			// [v1.4.0] Sensor Knowledge가 Identified 이상에서 공개한 안정 Target ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 ID", ToolTip="Sensor Contact의 KnownTargetId입니다. Detected 단계나 Sensor Contact 부재에서는 None입니다."))
	FName TargetId = NAME_None;

	// [v1.0.0] Target identity 공개 수준을 반영한 이름 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 이름 상태", ToolTip="Identified 이상에서만 Known이 되며 Detected 단계에서는 Unknown입니다."))
	ECFUIViewAvailability IdentityAvailability = ECFUIViewAvailability::Unknown;

			// [v1.4.0] Sensor Knowledge가 공개를 허용할 때 표시할 Target 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 표시 이름", ToolTip="Sensor Contact의 KnownDisplayName입니다. TargetSelectable 원본 이름을 직접 사용하지 않습니다."))
	FText DisplayName;

	// [v1.4.0] Sensor Contact가 공개한 관계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 관계", ToolTip="FCFSensorSnapshot Contact가 공개한 미확인, 아군, 중립 또는 적대 관계입니다."))
	ECFTargetRelation Relation = ECFTargetRelation::Unknown;

	// [v1.4.0] Sensor Contact가 공개한 큰 분류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 분류", ToolTip="FCFSensorSnapshot Contact가 공개한 차량, 장치, 환경 또는 미분류 상태입니다."))
	ECFTargetCategory Category = ECFTargetCategory::Unknown;

			// [v1.4.0] Sensor Knowledge가 현재 공개하는 Target 정보 단계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="타겟 정보 단계", ToolTip="FCFSensorSnapshot Contact의 Detected, Identified, DetailedScan 정보 단계입니다."))
	ECFTargetInfoLevel InformationLevel = ECFTargetInfoLevel::None;

	// [v1.4.0] 선택 Target과 연결된 Sensor Contact의 수명 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="Sensor Contact 수명 상태", ToolTip="Live, LastKnown, Lost 또는 DestroyedHold의 Sensor lifecycle 상태입니다. TargetSelect TrackState와 별개입니다."))
	ECFSensorContactState ContactState = ECFSensorContactState::Invalid;

	// [v1.4.0] 마지막 Sensor 신뢰 관측 이후 경과 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(Units="s", DisplayName="Contact 정보 경과 시간", ToolTip="Sensor Snapshot Contact의 FreshnessSeconds를 그대로 전달합니다."))
	float FreshnessSeconds = 0.0f;

	// [v1.4.0] Sensor Tactical Analysis의 현재 0~1 진행률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="분석 진행률", ToolTip="Sensor Snapshot Contact의 AnalysisProgress01을 재계산 없이 전달합니다."))
	float AnalysisProgress01 = 0.0f;

	// [v1.4.0] Sensor가 대상 파괴를 확정했는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Target", meta=(DisplayName="파괴 확인", ToolTip="Sensor Snapshot Contact가 DestroyedHold이면 True입니다. TargetSelect 선택 수명과 별개입니다."))
	bool bDestroyedConfirmed = false;

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

			// [v1.4.0] Contact 관계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact 관계", ToolTip="FCFSensorSnapshot Contact가 공개한 미확인, 아군, 중립 또는 적대 관계입니다."))
	ECFTargetRelation Relation = ECFTargetRelation::Unknown;

	// [v1.4.0] Sensor Contact의 큰 기능 분류입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact 분류", ToolTip="FCFSensorSnapshot Contact가 공개한 차량, 장치, 환경 또는 미분류 상태입니다."))
	ECFTargetCategory Category = ECFTargetCategory::Unknown;

	// [v1.4.0] Sensor Contact의 Player Knowledge 단계입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact 정보 단계", ToolTip="Sensor Knowledge가 공개한 Detected, Identified 또는 DetailedScan 단계입니다."))
	ECFTargetInfoLevel InformationLevel = ECFTargetInfoLevel::None;

	// [v1.4.0] Sensor Contact 자체의 수명 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact 수명 상태", ToolTip="Live, LastKnown, Lost 또는 DestroyedHold 상태를 Snapshot에서 그대로 전달합니다."))
	ECFSensorContactState ContactState = ECFSensorContactState::Invalid;

	// [v1.4.0] Sensor 전방/우측 기준 상대 평면 위치의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="상대 위치 상태", ToolTip="Snapshot 기준 위치·전방과 Contact 마지막 신뢰 위치가 유효하면 Known입니다."))
	ECFUIViewAvailability RelativePositionAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.4.0] Sensor 전방을 +X, 우측을 +Y로 한 실제 상대 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(Units="m", DisplayName="상대 위치 m", ToolTip="Snapshot의 SensorOriginWorldLocation, SensorForwardWorldDirection과 Contact LastKnownWorldLocation만으로 계산한 실제 평면 상대 위치입니다."))
	FVector2D RelativePositionMeters = FVector2D::ZeroVector;

	// [v1.4.0] Sensor 기준점에서 마지막 신뢰 위치까지의 실제 거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(Units="m", DisplayName="Contact 거리 m", ToolTip="Snapshot의 SensorOriginWorldLocation과 Contact LastKnownWorldLocation 사이 거리입니다."))
	float DistanceMeters = 0.0f;

	// [v1.4.0] 마지막 Sensor 신뢰 관측 이후 경과 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(Units="s", DisplayName="Contact 정보 경과 시간", ToolTip="Snapshot Contact의 FreshnessSeconds를 그대로 전달합니다."))
	float FreshnessSeconds = 0.0f;

	// [v1.4.0] Tactical Analysis의 현재 진행률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="Contact 분석 진행률", ToolTip="Snapshot Contact의 AnalysisProgress01을 그대로 전달합니다."))
	float AnalysisProgress01 = 0.0f;

	// [v1.4.0] Sensor가 대상 파괴를 확정했는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Contact 파괴 확인", ToolTip="DestroyedHold Contact이면 True입니다."))
	bool bDestroyedConfirmed = false;

	// [v1.4.0] Radar 정규화 표시 위치가 실제 확정된 UI Range/Zoom 계약으로 계산됐는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 정규화 위치 상태", ToolTip="Radar 표시 반경/Zoom 계약이 확정되기 전에는 Unavailable이며 Sensor 사거리로 임의 추정하지 않습니다."))
	ECFUIViewAvailability NormalizedPositionAvailability = ECFUIViewAvailability::Unavailable;

			// [v1.13.0] Radar 중심 기준 unit disk 안의 Heading-Up 정규화 평면 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 정규화 위치", ToolTip="명시적 DisplayRangeMeters가 있을 때 사용하는 위치입니다. X는 차량 전방, Y는 차량 우측이며 범위 밖 Contact는 방향을 보존한 unit edge 위치로 clamp됩니다."))
	FVector2D NormalizedPosition = FVector2D::ZeroVector;

	// [v1.13.0] 현재 Contact가 선택된 Radar 표시 범위 안에 있는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 표시 범위 안", ToolTip="실제 평면 상대 거리가 현재 DisplayRangeMeters 이하이면 True입니다. Range Profile이 없으면 False이며 NormalizedPositionAvailability가 Unavailable입니다."))
	bool bInsideDisplayRange = false;

	// [v1.13.0] 선택 Contact가 현재 Radar 표시 범위 밖이라 Radar 외곽 방향 표식이 필요한지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="선택 Contact Radar 외곽 표식", ToolTip="현재 선택 Contact가 DisplayRangeMeters 밖에 있을 때만 True입니다. 일반 Contact는 범위 밖에서 숨기고 이 표식을 만들지 않습니다."))
	bool bShowSelectedEdgeMarker = false;

	// [v1.13.0] 범위 밖 선택 Contact가 위치한 Heading-Up Radar 외곽 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="선택 Contact 외곽 방향", ToolTip="X는 차량 전방, Y는 차량 우측인 unit 방향 벡터입니다. bShowSelectedEdgeMarker가 True일 때 2-Corner Open Edge Bracket 방향에 사용합니다."))
	FVector2D SelectedEdgeDirection = FVector2D::ZeroVector;

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

	// [v1.13.0] 현재 Radar UI 표시 범위의 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 현재 표시 범위 상태", ToolTip="적용 Scanner Profile에 유효 Radar Range Preset이 있을 때만 Known입니다. Sensor 탐지 거리로 임의 추정하지 않습니다."))
	ECFUIViewAvailability DisplayRangeAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.13.0] 현재 선택된 Radar 표시 범위입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(Units="m", DisplayName="Radar 현재 표시 범위 m", ToolTip="현재 Radar Range Preset이 정의한 UI 표시 반경입니다. 이 값은 Passive/Active Sensor 탐지 성능을 변경하지 않습니다."))
	float DisplayRangeMeters = 0.0f;

		// [v1.13.0] 현재 Scanner의 실제 최대 탐지거리 채널 가용 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 최대 탐지 범위 상태", ToolTip="현재 적용 SensorConfig의 ActiveScanRangeCm이 0보다 큰 실제 Scanner 최대 탐지거리로 제공될 때 Known입니다. Radar 표시 Range Profile 존재 여부와는 독립입니다."))
	ECFUIViewAvailability MaximumDetectionRangeAvailability = ECFUIViewAvailability::Unavailable;

	// [v1.13.0] 현재 Scanner가 Active Scan으로 도달 가능한 실제 최대 탐지거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(Units="m", DisplayName="Radar 최대 탐지 범위 m", ToolTip="현재 적용 SensorConfig의 ActiveScanRangeCm을 m로 변환한 Scanner/Sensor 실제 성능 상한입니다. 현재 DisplayRangeMeters와 별개이며 Radar Zoom이 이 값을 변경하지 않습니다."))
	float MaximumDetectionRangeMeters = 0.0f;

	// [v1.13.0] 현재 선택된 Scanner Radar Range Preset의 0-based 인덱스입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 범위 프리셋 인덱스", ToolTip="현재 UI가 선택한 RadarDisplayRangePresetsCm의 0-based 인덱스입니다. Range Profile이 없으면 -1입니다."))
	int32 RangePresetIndex = INDEX_NONE;

	// [v1.13.0] 현재 Scanner가 제공하는 Radar 표시 Range Preset 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar 범위 프리셋 개수", ToolTip="현재 적용 Scanner Profile이 제공하는 단계식 Radar 표시 범위 개수입니다."))
	int32 RangePresetCount = 0;

	// [v1.13.0] 현재 Preset에서 한 단계 더 작은 표시 범위로 Zoom In할 수 있는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar Zoom In 가능", ToolTip="현재 RangePresetIndex보다 작은 범위 Preset이 존재할 때 True입니다."))
	bool bCanZoomIn = false;

	// [v1.13.0] 현재 Preset에서 한 단계 더 큰 표시 범위로 Zoom Out할 수 있는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="Radar Zoom Out 가능", ToolTip="현재 RangePresetIndex보다 큰 범위 Preset이 존재할 때 True입니다."))
	bool bCanZoomOut = false;

	// [v1.0.0] 실제 Sensor Provider가 공개한 Contact 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD|Radar", meta=(DisplayName="레이더 Contact 목록", ToolTip="Sensor Provider가 만든 Contact만 포함합니다. 범위 밖 일반 Contact도 lifecycle 데이터로 남을 수 있으므로 bInsideDisplayRange를 확인해 표시 여부를 결정합니다."))
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
USTRUCT(BlueprintType, meta=(DisplayName="인게임 UI ViewData (InGame UI View Data)", ToolTip="Vehicle, ViewMode, Weapon, Defense, Target, Radar와 Alert ViewData를 한 번에 전달합니다."))
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

	// [v1.14.0] 외부 3인칭 차체·카메라·터렛 방향 ViewData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|UI|HUD", meta=(DisplayName="뷰 모드 HUD 데이터"))
	FCFViewModeHUDData ViewMode;

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
