// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringTypes.h
// Version: v1.2.0
// Date: 2026-08-18
// Description: Vehicle Data Authoring의 Editor-only 공용 계약 타입입니다.
// Scope: Recipe intent, source/ownership, stable field path/value, import/applied 상태의 저장 계약을 제공합니다.
// Changelog:
// - v1.2.0: P0-11 External Drift exact 3-way review를 위해 AppliedTrace에 backward-compatible exact LastAppliedValue를 추가.
// - v1.1.0: P0-10 Measurement UX에서 "검토 안 함"과 "Compatibility Default 유지 확인"을 구분하는 Editor-only adoption metadata를 추가.
// - v1.0.0: DAUTH-P0-08A Recipe/Profile Foundation용 공용 타입을 최초 구현.
// Migration:
// - Runtime UCFVehicleData에는 Authoring metadata를 추가하지 않습니다.
// - 이 타입들은 CarFight_ReEditor에만 존재하며 packaged runtime에서 소비하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleWeaponTypes.h"
#include "CFVehicleAuthoringTypes.generated.h"

class UCFCombatFxData;
class UCFDriveStateProfile;
class UCFDrivetrainProfile;
class UCFEquipmentPresetData;
class UCFHandlingProfile;
class UCFPerformanceProfile;
class UCFVehicleBaseProfile;
class UCFVehicleData;
class UCFVehicleDefenseData;
class UStaticMesh;

/** Vehicle Authoring의 5개 고정 Profile Domain입니다. */
UENUM()
enum class ECFVehicleProfileDomain : uint8
{
	None,
	VehicleBase,
	Drivetrain,
	Handling,
	Performance,
	DriveState
};

/** 최종 field value를 제공할 수 있는 Frozen Authoring Source 종류입니다. */
UENUM()
enum class ECFVehicleSourceType : uint8
{
	ProjectCompatibilityDefault,
	VehicleBaseProfile,
	DrivetrainProfile,
	HandlingProfile,
	PerformanceProfile,
	DriveStateProfile,
	RuleDerived,
	AssetDerived,
	RecipeExplicitSemanticInput,
	LegacyImportedPinnedBaseline,
	AdvancedLeafOverride,
	LegacySerializedPassthrough
};

/** Registry descriptor가 사용할 P0 Resolver 생성 규칙입니다. */
UENUM()
enum class ECFVehicleResolveRule : uint8
{
	ProjectCompatibilityDefault,
	RecipeAssetIntent,
	RecipeSemantic,
	RecipeBinding,
	DerivedGate,
	AssetSocketDerived,
	LegacySerializedPassthrough,
	BaseProfileThenRecipeExplicit,
	AccelerationFeelDerived,
	SteeringFeelDerived,
	HandlingProfileDirect,
	AssetMeasurementProposal,
	GripFeelDerived,
	SuspensionFeelDerived,
	DrivetrainProfile,
	VehicleBaseProfile,
	PerformanceProfileDirect,
	CompatibilityDefaultOrAdvanced,
	RecipeWheelVisualPolicy,
	BaseProfileMeasurementPolicy,
	DriveStateProfile,
	BaseProfileThenRecipeAsset,
	ProjectDefaultThenRecipeSemantic
};

/** Existing Definition의 Legacy Pin을 새 Authoring Source로 넘길 사용자 Adoption Group입니다. */
UENUM(BlueprintType)
enum class ECFVehicleAdoptGroup : uint8
{
	VisualAssets UMETA(DisplayName="시각 자산"),
	Layout UMETA(DisplayName="레이아웃"),
	Hardpoints UMETA(DisplayName="하드포인트"),
	Mounts UMETA(DisplayName="장착 프로파일"),
	MassDurability UMETA(DisplayName="질량 / 내구도"),
	DefenseFx UMETA(DisplayName="방어 / 파괴 FX"),
	Drivetrain UMETA(DisplayName="구동계"),
	Handling UMETA(DisplayName="핸들링"),
	Performance UMETA(DisplayName="성능"),
	WheelGeometry UMETA(DisplayName="휠 지오메트리"),
	WheelVisual UMETA(DisplayName="휠 시각"),
	DriveState UMETA(DisplayName="DriveState"),
	TechnicalHandling UMETA(DisplayName="고급 핸들링"),
	LegacyTechnical UMETA(DisplayName="레거시 기술 데이터"),
	DerivedState UMETA(Hidden)
};

/** Profile 기본값과 Recipe 명시 수치를 구분하는 입력 방식입니다. */
UENUM(BlueprintType)
enum class ECFAuthoringInputMode : uint8
{
	UseProfile UMETA(DisplayName="프로파일 사용", ToolTip="해당 Domain Profile의 값을 사용합니다."),
	ExplicitValue UMETA(DisplayName="명시 값", ToolTip="Recipe에 명시적으로 입력한 값을 사용합니다.")
};

/** Optional Asset Reference에서 Profile / 명시 Asset / 의도적 None을 구분합니다. */
UENUM(BlueprintType)
enum class ECFAssetIntentMode : uint8
{
	UseProfile UMETA(DisplayName="프로파일 사용"),
	ExplicitAsset UMETA(DisplayName="명시 자산"),
	ExplicitNone UMETA(DisplayName="명시적으로 없음")
};

/** 휠 시각 동작을 Raw flag 대신 표현하는 semantic mode입니다. */
UENUM(BlueprintType)
enum class ECFWheelVisualIntentMode : uint8
{
	UseProfilePolicy UMETA(DisplayName="프로파일 정책 사용"),
	ManualMeshScale UMETA(DisplayName="수동 메시 스케일"),
	AutoScaleToPhysicsRadius UMETA(DisplayName="물리 반지름 자동 스케일")
};

/** DriveState가 Project Default를 쓸지 차량별 Profile을 쓸지 결정합니다. */
UENUM(BlueprintType)
enum class ECFVehicleDriveStateMode : uint8
{
	ProjectDefault UMETA(DisplayName="프로젝트 기본값"),
	VehicleSpecific UMETA(DisplayName="차량별 설정")
};

/** Existing Definition이 새 Authoring Source로 얼마나 전환되었는지 나타냅니다. */
UENUM(BlueprintType)
enum class ECFVehicleManageState : uint8
{
	Unmanaged UMETA(DisplayName="관리 안 됨"),
	LegacyImported UMETA(DisplayName="레거시 가져옴"),
	PartiallyManaged UMETA(DisplayName="부분 관리"),
	Managed UMETA(DisplayName="관리됨")
};

/** Definition leaf diff가 수행할 안정적인 변경 연산입니다. */
UENUM()
enum class ECFVehicleDiffOp : uint8
{
	SetLeaf,
	AddArrayElement,
	RemoveArrayElement,
	MoveArrayElement
};

/** Stale report에서 field가 달라진 원인을 분류합니다. */
UENUM()
enum class ECFVehicleStaleReason : uint8
{
	None,
	EffectiveSourceChanged,
	ShadowSourceChanged,
	ExternalDrift,
	MeasurementFingerprintChanged,
	ResolverContractChanged
};

/** Source Trace / Override / Diff가 공유하는 구조화된 Stable Field Path입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleFieldPath
{
	GENERATED_BODY()

	// Scalar/Nested property를 바깥쪽부터 leaf까지 표현하는 property 이름 체인입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="프로퍼티 체인"))
	TArray<FName> PropertyChain;

	// Stable-ID collection field 이름이며 scalar path에서는 None입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="컬렉션 프로퍼티"))
	FName CollectionPropertyName = NAME_None;

	// Array element를 식별할 stable selector property 이름입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="선택자 키 프로퍼티"))
	FName SelectorKeyPropertyName = NAME_None;

	// 실제 element identity이며 Registry wildcard pattern에서는 None입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="선택자 키 값"))
	FName SelectorKeyValue = NAME_None;

	// Stable Field Path를 로그/정렬/Hash용 canonical 문자열로 반환합니다.
	FString ToCanonicalString(const bool bUseWildcardForMissingSelector = false) const
	{
		// 최종 canonical 문자열을 누적합니다.
		FString Result;

		if (!CollectionPropertyName.IsNone())
		{
			Result = CollectionPropertyName.ToString();
			if (!SelectorKeyPropertyName.IsNone())
			{
				// 실제 selector가 없고 Registry pattern을 출력할 때 사용할 wildcard 문자열입니다.
				const FString SelectorValue = SelectorKeyValue.IsNone() && bUseWildcardForMissingSelector
					? TEXT("*")
					: SelectorKeyValue.ToString();
				Result += FString::Printf(TEXT("[%s=%s]"), *SelectorKeyPropertyName.ToString(), *SelectorValue);
			}
		}

		for (const FName PropertyName : PropertyChain)
		{
			if (!Result.IsEmpty())
			{
				Result += TEXT(".");
			}
			Result += PropertyName.ToString();
		}

		return Result;
	}
};

/** Reflection leaf를 type-safe canonical text로 보존하는 generic value입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleFieldValue
{
	GENERATED_BODY()

	// Struct/Enum/Object/Class constraint까지 포함하는 reflection type signature입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="프로퍼티 타입 서명"))
	FString PropertyTypeSignature;

	// Unreal Reflection Export/Import 규칙으로 만든 canonical value text입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="Canonical 값"))
	FString CanonicalValueText;
};

/** Legacy Pin과 Advanced Override가 공유하는 field/value/reason 저장 단위입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleFieldOverride
{
	GENERATED_BODY()

	// Override 또는 Legacy Pin이 가리키는 exact stable field입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="필드 경로"))
	FCFVehicleFieldPath FieldPath;

	// Type signature가 포함된 exact field value입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="필드 값"))
	FCFVehicleFieldValue OverrideValue;

	// Advanced Override 또는 migration 보존 이유입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring", meta=(DisplayName="이유"))
	FString Reason;
};

/** 차량의 시각 Asset 선택과 wheel socket binding만 보관하는 Recipe intent입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleAssetIntent
{
	GENERATED_BODY()

	// 차량 차체로 사용할 Static Mesh입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="차체 메시"))
	TSoftObjectPtr<UStaticMesh> ChassisMesh;

	// 앞왼쪽 휠 메시입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="앞왼쪽 휠 메시"))
	TSoftObjectPtr<UStaticMesh> WheelMeshFL;

	// 앞오른쪽 휠 메시입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="앞오른쪽 휠 메시"))
	TSoftObjectPtr<UStaticMesh> WheelMeshFR;

	// 뒤왼쪽 휠 메시입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="뒤왼쪽 휠 메시"))
	TSoftObjectPtr<UStaticMesh> WheelMeshRL;

	// 뒤오른쪽 휠 메시입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="뒤오른쪽 휠 메시"))
	TSoftObjectPtr<UStaticMesh> WheelMeshRR;

	// 앞왼쪽 휠 위치를 추출할 차체 socket 이름입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="앞왼쪽 휠 소켓"))
	FName BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");

	// 앞오른쪽 휠 위치를 추출할 차체 socket 이름입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="앞오른쪽 휠 소켓"))
	FName BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");

	// 뒤왼쪽 휠 위치를 추출할 차체 socket 이름입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="뒤왼쪽 휠 소켓"))
	FName BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");

	// 뒤오른쪽 휠 위치를 추출할 차체 socket 이름입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Assets", meta=(DisplayName="뒤오른쪽 휠 소켓"))
	FName BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
};

/** Recipe가 선택한 5개 flat Profile Asset reference입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleProfileBindings
{
	GENERATED_BODY()

	// Vehicle Base Domain Profile입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profiles", meta=(DisplayName="차량 기본 프로파일"))
	TSoftObjectPtr<UCFVehicleBaseProfile> VehicleBaseProfile;

	// Drivetrain Domain Profile입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profiles", meta=(DisplayName="구동계 프로파일"))
	TSoftObjectPtr<UCFDrivetrainProfile> DrivetrainProfile;

	// Handling Domain Profile입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profiles", meta=(DisplayName="핸들링 프로파일"))
	TSoftObjectPtr<UCFHandlingProfile> HandlingProfile;

	// Performance Domain Profile입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profiles", meta=(DisplayName="성능 프로파일"))
	TSoftObjectPtr<UCFPerformanceProfile> PerformanceProfile;

	// DriveState Domain Profile입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profiles", meta=(DisplayName="DriveState 프로파일"))
	TSoftObjectPtr<UCFDriveStateProfile> DriveStateProfile;
};

/** Raw Movement 값을 복제하지 않는 Frozen 4축 Driving Feel intent입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleFeelIntent
{
	GENERATED_BODY()

	// 가속 체감의 0~1 semantic intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Feel", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="가속감"))
	float AccelerationFeel = 0.5f;

	// 조향 민첩성의 0~1 semantic intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Feel", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="조향 민첩성"))
	float SteeringAgility = 0.5f;

	// 접지 체감의 0~1 semantic intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Feel", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="접지감"))
	float GripFeel = 0.5f;

	// 서스펜션 단단함의 0~1 semantic intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Feel", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="서스펜션 단단함"))
	float SuspensionFirmness = 0.5f;
};

/** Base/Gross mass의 Profile/Explicit ownership과 explicit 값입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleMassIntent
{
	GENERATED_BODY()

	// 기준 차량 질량의 source mode입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mass", meta=(DisplayName="기준 질량 입력 방식"))
	ECFAuthoringInputMode BaseMassMode = ECFAuthoringInputMode::UseProfile;

	// Explicit 모드에서 사용할 기준 차량 질량 kg입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mass", meta=(ClampMin="0.0", Units="kg", DisplayName="명시 기준 질량 kg"))
	float ExplicitBaseMassKg = 0.0f;

	// 최대 총중량의 source mode입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mass", meta=(DisplayName="최대 총중량 입력 방식"))
	ECFAuthoringInputMode GrossMassMode = ECFAuthoringInputMode::UseProfile;

	// Explicit 모드에서 사용할 최대 허용 총중량 kg입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mass", meta=(ClampMin="0.0", Units="kg", DisplayName="명시 최대 총중량 kg"))
	float ExplicitGrossMassKg = 0.0f;
};

/** Vehicle durability MaxHealth의 Profile/Explicit ownership과 explicit 값입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleDurabilityIntent
{
	GENERATED_BODY()

	// 최대 내구도의 source mode입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Durability", meta=(DisplayName="최대 내구도 입력 방식"))
	ECFAuthoringInputMode MaxHealthMode = ECFAuthoringInputMode::UseProfile;

	// Explicit 모드에서 사용할 최대 내구도입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Durability", meta=(ClampMin="0.0", DisplayName="명시 최대 내구도"))
	float ExplicitMaxHealth = 0.0f;
};

/** Hardpoint의 stable identity와 Asset binding만 저장하는 semantic intent입니다. */
USTRUCT(BlueprintType)
struct FCFHardpointIntent
{
	GENERATED_BODY()

	// Hardpoint element stable identity입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Hardpoint", meta=(DisplayName="위치 슬롯 ID"))
	FName LocationSlotId = NAME_None;

	// Front/Top 같은 위치 분류입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Hardpoint", meta=(DisplayName="위치 분류"))
	FName LocationCategory = NAME_None;

	// Asset Derived transform을 읽을 socket binding입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Hardpoint", meta=(DisplayName="캡처 소켓 이름"))
	FName SocketName = NAME_None;
};

/** Current active Mount contract만 저장하고 hidden legacy field를 복제하지 않는 semantic intent입니다. */
USTRUCT(BlueprintType)
struct FCFMountIntent
{
	GENERATED_BODY()

	// Mount element stable identity입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mount", meta=(DisplayName="장착 프로파일 ID"))
	FName MountProfileId = NAME_None;

	// 이 Mount가 참조할 Hardpoint stable ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mount", meta=(DisplayName="위치 슬롯 참조"))
	FName LocationSlotRef = NAME_None;

	// 장착 규칙 타입입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mount", meta=(DisplayName="장착 타입"))
	ECFVehicleMountType MountType = ECFVehicleMountType::None;

	// 허용 최대 무기 크기입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mount", meta=(DisplayName="무기 크기 제한"))
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::None;

	// 기본 장비 조합을 제공하는 EquipmentPresetData입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mount", meta=(DisplayName="기본 장비 프리셋"))
	TSoftObjectPtr<UCFEquipmentPresetData> DefaultEquipmentPresetData;

	// 후속 module damage에서 외부 노출 장비로 분류할지 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Mount", meta=(DisplayName="노출 모듈 여부"))
	bool bExposedModule = true;
};

/** Defense/Destroyed FX의 Profile/Explicit/None 의도를 저장합니다. */
USTRUCT(BlueprintType)
struct FCFVehicleDefaultIntent
{
	GENERATED_BODY()

	// 기본 Defense source mode입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Defaults", meta=(DisplayName="기본 방어 입력 방식"))
	ECFAssetIntentMode DefenseMode = ECFAssetIntentMode::UseProfile;

	// ExplicitAsset 모드의 VehicleDefenseData입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Defaults", meta=(DisplayName="명시 기본 방어 데이터"))
	TSoftObjectPtr<UCFVehicleDefenseData> DefaultDefenseData;

	// 기본 Destroyed FX source mode입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Defaults", meta=(DisplayName="파괴 FX 입력 방식"))
	ECFAssetIntentMode DestroyedFxMode = ECFAssetIntentMode::UseProfile;

	// ExplicitAsset 모드의 CombatFxData입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Defaults", meta=(DisplayName="명시 기본 파괴 FX"))
	TSoftObjectPtr<UCFCombatFxData> DefaultDestroyedFxData;

	// 파괴 FX를 배치할 차체 socket semantic input입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Defaults", meta=(DisplayName="파괴 FX 소켓 이름"))
	FName DestroyedFxSocketName = TEXT("FX_Destroyed");
};

/** WheelVisual raw flag 대신 저장하는 semantic policy입니다. */
USTRUCT(BlueprintType)
struct FCFWheelVisualIntent
{
	GENERATED_BODY()

	// Resolver가 WheelVisual raw fields를 만들 때 사용할 semantic mode입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|WheelVisual", meta=(DisplayName="휠 시각 정책"))
	ECFWheelVisualIntentMode Mode = ECFWheelVisualIntentMode::UseProfilePolicy;
};

/** Measurement 결과를 명시적으로 채택했는지와 그때의 Asset fingerprint를 기록합니다. */
USTRUCT(BlueprintType)
struct FCFVehicleAssetAdoption
{
	GENERATED_BODY()

	// 측정된 전륜 반지름 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bUseMeasuredFrontRadius = false;

	// 측정된 후륜 반지름 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bUseMeasuredRearRadius = false;

	// 측정된 전륜 폭 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bUseMeasuredFrontWidth = false;

	// 측정된 후륜 폭 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bUseMeasuredRearWidth = false;

		// 제안된 radius measure mode 사용 여부입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bUseSuggestedRadiusMeasureMode = false;

	// 전륜 반지름 measurement 대신 현재 Compatibility Default를 reviewed decision으로 유지했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bConfirmedFrontRadiusCompatibilityDefault = false;

	// 후륜 반지름 measurement 대신 현재 Compatibility Default를 reviewed decision으로 유지했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bConfirmedRearRadiusCompatibilityDefault = false;

	// 전륜 폭 measurement 대신 현재 Compatibility Default를 reviewed decision으로 유지했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bConfirmedFrontWidthCompatibilityDefault = false;

	// 후륜 폭 measurement 대신 현재 Compatibility Default를 reviewed decision으로 유지했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	bool bConfirmedRearWidthCompatibilityDefault = false;

	// 전륜 반지름을 채택했을 때의 관련 Asset fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	FString FrontRadiusAssetFingerprint;

	// 후륜 반지름을 채택했을 때의 관련 Asset fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	FString RearRadiusAssetFingerprint;

	// 전륜 폭을 채택했을 때의 관련 Asset fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	FString FrontWidthAssetFingerprint;

	// 후륜 폭을 채택했을 때의 관련 Asset fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	FString RearWidthAssetFingerprint;

	// measure mode 제안을 채택했을 때의 관련 Asset fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Adoption")
	FString RadiusModeAssetFingerprint;
};

/** 마지막 Apply에서 field 하나가 어떤 Source와 value였는지 보존합니다. */
USTRUCT(BlueprintType)
struct FCFVehicleAppliedTrace
{
	GENERATED_BODY()

	// 마지막 Apply의 stable field identity입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FCFVehicleFieldPath FieldPath;

	// 마지막 Apply의 effective source type입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	ECFVehicleSourceType EffectiveSourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;

	// 마지막 Apply의 effective source asset/id 문자열입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString EffectiveSourceId;

	// 마지막 Apply source payload의 deterministic signature입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString EffectiveSourceSignature;

	// effective 아래 shadow source들의 combined signature입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString ShadowSourceSignature;

		// 새 Apply부터 3-way Drift review를 위해 함께 보존하는 exact typed Last Applied value입니다. 기존 asset은 빈 값일 수 있습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FCFVehicleFieldValue LastAppliedValue;

	// 마지막 Apply exact field value hash입니다. 기존 stale/drift authority와 backward compatibility를 유지합니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString LastAppliedValueHash;
};

/** Target Definition과 Authoring Source의 마지막 성공 Apply baseline입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleAppliedState
{
	GENERATED_BODY()

	// 마지막 Apply 때의 Recipe diagnostic revision입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	int32 AppliedRecipeRevision = 0;

	// 마지막 Apply 때의 full Recipe fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString AppliedRecipeFingerprint;

	// 마지막 Apply 때의 effective Source set signature입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString AppliedSourceSignature;

	// 마지막 Apply가 만든 exact Definition hash입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	FString AppliedDefinitionHash;

	// 마지막 Apply에 사용한 Resolver semantic contract revision입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	int32 ResolverContractRevision = 0;

	// Field-level stale/drift 비교에 사용할 마지막 applied trace입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied")
	TArray<FCFVehicleAppliedTrace> FieldTraces;
};

/** Existing Definition import의 Legacy Pin과 점진적 Adoption 상태입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleImportState
{
	GENERATED_BODY()

	// 현재 Definition이 어느 관리 단계인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Import")
	ECFVehicleManageState ManageState = ECFVehicleManageState::Managed;

	// Initial Import 시점의 exact Definition hash입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Import")
	FString ImportedDefinitionHash;

	// 일반 semantic field의 Legacy Imported Pinned Baseline입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Import")
	TArray<FCFVehicleFieldOverride> LegacyPinnedFields;

	// 일반 Adoption 대상이 아닌 hidden legacy serialized field입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Import")
	TArray<FCFVehicleFieldOverride> LegacySerializedFields;

	// Legacy Pin에서 새 source ownership으로 넘긴 user-facing group 집합입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Import")
	TSet<ECFVehicleAdoptGroup> AdoptedGroups;
};
