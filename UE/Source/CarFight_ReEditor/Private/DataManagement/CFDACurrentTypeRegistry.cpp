// Copyright (c) CarFight. All Rights Reserved.
// File: CFDACurrentTypeRegistry.cpp
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 current concrete CarFight DataAsset semantic/policy + Manager user description 등록 구현입니다.
// Changelog:
// - v1.1.0: current 27종에 '어떤 데이터인가 / 어디에 사용되는가' 사용자 설명을 추가.
// - v1.0.3: current descriptor batch 등록을 all-or-nothing으로 교정하고 SourceName을 exact symbol provenance로 정규화.
// - v1.0.2: 공개 read-only UCFVDAValidator::ValidateVehicleData facade를 VehicleData CustomContract policy로 연결.
// - v1.0.1: Plan v0.1.4 기준 Identity Required/N/A와 resolver kind를 분리해 current 27종 mapping을 교정.
// - v1.0.0: current concrete 27종의 Domain, 사용자용 Type 이름, 역할, Identity Policy, Validation Policy를 actual Source audit 기준으로 등록.
// Migration:
// - Runtime/Content/Asset 변경 없음. descriptor는 선언 metadata만 소유하며 Stable ID resolve, Validation 실행, Duplicate ID 검사를 수행하지 않습니다.

#include "DataManagement/CFDATypeRegistry.h"

namespace CFDACurrentTypeRegistryPrivate
{
	// P0-04C Manager가 current 27종을 기술 클래스 지식 없이 설명할 수 있도록 사용자 의미를 채웁니다.
	void ApplyUserFacingDescriptions(FCFDASemanticDescriptor& Descriptor)
	{
#define CF_SET_DA_USER_DESCRIPTION(ClassPathLiteral, PurposeText, UsageText) \
		if (Descriptor.ClassPath == TEXT(ClassPathLiteral)) \
		{ \
			Descriptor.UserPurposeDescription = TEXT(PurposeText); \
			Descriptor.UserUsageDescription = TEXT(UsageText); \
			return; \
		}

		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFAmmoData",
			"탄약 한 종류의 기본 정보와 적재 기준을 정의하는 데이터입니다.",
			"무기와 피팅, HUD가 탄종 이름·계열·질량·최대 적재량을 확인할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFCombatFxData",
			"발사·피격·파괴 상황에서 보여줄 전투 효과를 정의하는 데이터입니다.",
			"무기와 피해 연출이 상황에 맞는 Niagara 효과와 표시 방식을 선택할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFDamageData",
			"공격이 대상에 적용할 피해와 관통·범위 피해 규칙을 정의하는 데이터입니다.",
			"발사체가 명중했을 때 차량과 모듈에 어떤 피해를 적용할지 결정하는 데 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFEquipmentPresetData",
			"차량에 장착할 무기 또는 유틸리티 장비 한 묶음을 정의하는 데이터입니다.",
			"차량의 Mount와 장비 선택 화면이 실제 장착 패키지를 선택하고 표시할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFEquipmentItemData",
			"인벤토리에서 장비 프리셋 하나를 아이템으로 취급하기 위한 정의입니다.",
			"인벤토리와 피팅 시스템이 장비 프리셋을 수량 1의 선택 가능한 장비로 다룰 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFDefenseItemData",
			"인벤토리에서 차량 방어 구성을 아이템으로 취급하기 위한 정의입니다.",
			"인벤토리와 피팅 시스템이 쉴드·장갑 구성을 선택 가능한 방어 장비로 다룰 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFProjectileData",
			"무기가 발사하는 발사체의 비행과 피해 연결을 정의하는 데이터입니다.",
			"WeaponData가 실제 발사체의 추진·비행 효과·유도·피해 데이터를 선택할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFRuntimeTestCatalogData",
			"게임 실행 중 테스트하거나 시연할 차량과 장비 목록을 관리하는 데이터입니다.",
			"PIE와 패키징된 런타임 적용 메뉴가 사용자에게 선택 가능한 차량·장비 목록을 보여줄 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFTargetSelectData",
			"플레이어가 타겟을 찾고 선택하는 방식과 안정성을 조정하는 데이터입니다.",
			"타겟 선택 시스템이 검색 거리·선택 각도·갱신 주기·가림 유예를 판단할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFTurretMountData",
			"터렛의 외형과 회전 범위, 총구 기준을 정의하는 데이터입니다.",
			"장착된 터렛이 Base/Yaw/Pitch 메쉬를 배치하고 회전·조준·발사 방향을 계산할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFVehicleCameraData",
			"차량을 운전할 때 사용할 카메라 위치와 시야 반응을 정의하는 데이터입니다.",
			"차량 카메라가 Arm 길이·FOV·속도 보정·충돌 보정과 기본 조준 시점을 적용할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFVehicleData",
			"차량 한 대의 런타임 구성을 묶는 핵심 차량 데이터입니다.",
			"차량 Pawn이 차체·바퀴·주행·질량·방어·센서·장착점 등 최종 차량 설정을 초기화할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFVehicleDefenseData",
			"차량의 쉴드와 장갑 내구·저항을 정의하는 데이터입니다.",
			"차량 방어 시스템이 피격 시 쉴드·방향별 장갑·저항·재생을 계산할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFVehicleFittingData",
			"차량에 실제로 장착할 장비·탄약·방어 구성을 저장하는 데이터입니다.",
			"출격 전 피팅과 런타임 장비 적용이 어떤 차량에 무엇을 장착할지 재현할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFVehicleSensorData",
			"차량의 탐지 범위와 스캔·레이더 동작을 정의하는 데이터입니다.",
			"센서 시스템이 Passive/Active/Visual 탐지와 Contact 수명, 분석 및 Radar 표시 범위를 계산할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFWeaponData",
			"무기 한 종류의 장착 조건과 발사 성능을 정의하는 데이터입니다.",
			"장착·발사 시스템이 사거리·발사 방식·탄약·열·Charge·Projectile 연결을 적용할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFHUDLayoutData",
			"HUD 각 요소의 화면 배치와 크기 기준을 정의하는 데이터입니다.",
			"HUD가 해상도에 맞춰 Vehicle·Radar·Target·Weapon 등 패널의 위치와 글자 크기를 배치할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFHUDVisualData",
			"HUD가 사용할 텍스처와 머티리얼 자산 연결을 모아 둔 데이터입니다.",
			"각 HUD 패널이 VehiclePanel·Armor·Radar·Target·Weapon용 시각 자산을 선택할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFUIDensityData",
			"UI를 촘촘하게 또는 넓게 표시하는 간격·크기 기준을 정의하는 데이터입니다.",
			"Compact·Standard·Expanded 밀도에 따라 Padding·행 높이·아이콘·텍스트 비율을 조정할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_Re.CFUIStyleData",
			"CarFight UI의 공통 색상·글꼴·간격·형상 규칙을 정의하는 데이터입니다.",
			"여러 HUD와 메뉴가 같은 시각 스타일과 상태 표현을 공유하도록 공통 디자인 토큰을 제공할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFDriveStateProfile",
			"차량 주행 상태를 판정하는 기준값을 묶어 둔 제작용 프로파일입니다.",
			"차량 제작 과정에서 정지·가속·감속 등 DriveState 판정 기준을 VehicleData에 구성할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFDrivetrainProfile",
			"엔진·변속기·구동계 설정을 묶어 둔 차량 제작용 프로파일입니다.",
			"차량 제작 가이드가 엔진 출력과 기어비, 구동계 값을 VehicleData에 작성할 때 기준으로 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFHandlingProfile",
			"조향과 핸들링 특성의 기준값을 묶어 둔 차량 제작용 프로파일입니다.",
			"차량 제작 가이드가 조향 반응과 핸들링 관련 값을 VehicleData에 작성할 때 기준으로 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFPerformanceProfile",
			"차량의 엔진 출력과 RPM 같은 성능 기준값을 묶어 둔 제작용 프로파일입니다.",
			"차량 제작 가이드가 최대 출력·RPM·Redline·TorqueCurve 같은 성능 값을 VehicleData에 작성할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFVehicleBaseProfile",
			"차량의 질량과 내구도 같은 기본 기준값을 묶어 둔 제작용 프로파일입니다.",
			"차량 제작 가이드가 기준 질량·최대 총중량·내구도 등 차량 기본값을 VehicleData에 작성할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFVehicleRecipeData",
			"차량 제작 가이드에서 한 차량을 어떻게 만들었는지 저장하는 제작 레시피입니다.",
			"Vehicle Builder가 Target VehicleData와 프로파일 선택, 주행 설정, 장착점 계획과 적용 기록을 이어서 작업할 때 사용합니다.");
		CF_SET_DA_USER_DESCRIPTION("/Script/CarFight_ReEditor.CFVehicleRefEvidence",
			"실제 차량을 참고해 제작할 때 사용한 근거 자료를 정리하는 데이터입니다.",
			"Vehicle Builder가 차량 제원 출처와 확정·충돌·미확인 정보를 추적하고 제작 수치의 근거를 확인할 때 사용합니다.");

#undef CF_SET_DA_USER_DESCRIPTION
	}

	// current Source audit 결과를 descriptor 한 건으로 조립합니다.
	FCFDASemanticDescriptor MakeDescriptor(
		const TCHAR* ClassPath,
		ECFDADomain Domain,
		const TCHAR* TypeDisplayName,
		const TCHAR* RoleDescription,
		ECFDAIdentityPolicy IdentityPolicy,
		ECFDAIdentityResolverKind IdentityResolverKind,
		const TCHAR* IdentitySourceName,
		ECFDAValidationPolicy ValidationPolicy,
		const TCHAR* ValidationSourceName)
	{
		// current type 한 건의 선언적 semantic/policy metadata입니다.
		FCFDASemanticDescriptor Descriptor;
		Descriptor.ClassPath = ClassPath;
		Descriptor.Domain = Domain;
		Descriptor.TypeDisplayName = TypeDisplayName;
		Descriptor.RoleDescription = RoleDescription;
		ApplyUserFacingDescriptions(Descriptor);
		Descriptor.IdentityPolicy = IdentityPolicy;
		Descriptor.IdentityResolverKind = IdentityResolverKind;
		Descriptor.IdentitySourceName = IdentitySourceName;
		Descriptor.ValidationPolicy = ValidationPolicy;
		Descriptor.ValidationSourceName = ValidationSourceName;
		return Descriptor;
	}
}

// current concrete CarFight DataAsset 전체의 audited semantic/policy descriptor를 이 Registry에 등록합니다.
bool FCFDATypeRegistry::RegisterCurrentCarFightDescriptors(FString* OutError)
{
	if (OutError)
	{
		OutError->Reset();
	}

	// P0-02B current Source audit에서 확정한 concrete descriptor 27종입니다.
	TArray<FCFDASemanticDescriptor> CurrentDescriptors;
	CurrentDescriptors.Reserve(27);

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFAmmoData"),
		ECFDADomain::Combat,
		TEXT("탄약 데이터"),
		TEXT("탄종의 안정 ID, 표시 이름, 계열, 단위 질량, 출격 최대 적재량과 UI 메타데이터를 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("AmmoId"),
		ECFDAValidationPolicy::CustomContract,
		TEXT("IsAmmoDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFCombatFxData"),
		ECFDADomain::Combat,
		TEXT("전투 FX 데이터"),
		TEXT("발사·피격·파괴에 사용할 Niagara 시스템, 스케일 적용 방식, 회전 보정과 수명 안전장치를 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("CombatFxId"),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFDamageData"),
		ECFDADomain::Combat,
		TEXT("피해 데이터"),
		TEXT("ProjectileData가 참조하는 직접 피해, 관통, 범위 피해, 모듈 피해와 물리 반응 후보 규칙을 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("DamageId"),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFEquipmentPresetData"),
		ECFDADomain::Combat,
		TEXT("장비 프리셋 데이터"),
		TEXT("차량 MountProfile이 사용할 무장 패키지 또는 Utility Scanner 패키지와 플레이어 표시 이름을 묶습니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("EquipmentId"),
		ECFDAValidationPolicy::CustomContract,
		TEXT("HasCompleteEquipmentData")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFEquipmentItemData"),
		ECFDADomain::Combat,
		TEXT("장비 인벤토리 Definition"),
		TEXT("EquipmentPresetData를 강타입으로 참조하는 Quantity 1 인벤토리 장비 Definition입니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::PrimaryAssetId,
		TEXT("ItemDefinitionId"),
		ECFDAValidationPolicy::NativeDataValidation,
		TEXT("IsDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFDefenseItemData"),
		ECFDADomain::Vehicle,
		TEXT("방어 인벤토리 Definition"),
		TEXT("VehicleDefenseData를 강타입으로 참조하는 Quantity 1 인벤토리 방어 Definition입니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::PrimaryAssetId,
		TEXT("ItemDefinitionId"),
		ECFDAValidationPolicy::NativeDataValidation,
		TEXT("IsDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFProjectileData"),
		ECFDADomain::Combat,
		TEXT("발사체 데이터"),
		TEXT("무기가 참조할 발사체, 추진, 비행 FX, 요격, 피해 연결과 선택적 미사일 비행·유도 기반 설정을 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("ProjectileId"),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFRuntimeTestCatalogData"),
		ECFDADomain::Authoring,
		TEXT("런타임 테스트 Catalog"),
		TEXT("PIE와 패키징 시연 메뉴에 노출할 VehicleData와 EquipmentPresetData hard reference 허용 목록을 관리합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::CustomContract,
		TEXT("ValidateRuntimeTestCatalog")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFTargetSelectData"),
		ECFDADomain::Targeting,
		TEXT("타겟 선택 데이터"),
		TEXT("타겟 후보 검색 거리, 선택 각도, 갱신 주기, 가림 유예와 선택 안정화 튜닝을 정의합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::CustomContract,
		TEXT("IsTargetSelectConfigValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFTurretMountData"),
		ECFDADomain::Combat,
		TEXT("터렛 마운트 데이터"),
		TEXT("터렛 Base/Yaw/Pitch 메쉬, 피벗·총구 소켓, 회전 제한·속도와 발사 정렬 정책을 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("TurretMountId"),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFVehicleCameraData"),
		ECFDADomain::Vehicle,
		TEXT("차량 카메라 데이터"),
		TEXT("차량 공통 카메라 피벗, Arm/FOV, 속도·충돌 보정과 기본 Aim Profile을 정의합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFVehicleData"),
		ECFDADomain::Vehicle,
		TEXT("차량 데이터"),
		TEXT("차체·휠 시각, 차량 레이아웃, 하드포인트·Mount, 질량, 주행, 내구도, 방어와 DriveState를 묶는 차량 Runtime canonical definition입니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::PrimaryAssetId,
		TEXT("GetPrimaryAssetId"),
		ECFDAValidationPolicy::CustomContract,
		TEXT("UCFVDAValidator::ValidateVehicleData")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFVehicleDefenseData"),
		ECFDADomain::Vehicle,
		TEXT("차량 방어 데이터"),
		TEXT("쉴드, 재생, 장갑 분류·저항, 6방향 장갑과 피팅 방어 질량을 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("DefenseId"),
		ECFDAValidationPolicy::NativeDataValidation,
		TEXT("IsDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFVehicleFittingData"),
		ECFDADomain::Vehicle,
		TEXT("차량 피팅 데이터"),
		TEXT("출격 전 기준 차량, 장착·Scanner·탄약·방어 선택과 결정론적 피팅 Snapshot 구성을 저장합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("FittingId"),
		ECFDAValidationPolicy::NativeDataValidation,
		TEXT("IsDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFVehicleSensorData"),
		ECFDADomain::Targeting,
		TEXT("차량 센서 데이터"),
		TEXT("Passive/Active/Visual 탐지, Contact 수명, Tactical Analysis와 Radar 표시 범위 Profile을 정의합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::NativeDataValidation,
		TEXT("IsDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFWeaponData"),
		ECFDADomain::Combat,
		TEXT("무기 데이터"),
		TEXT("무기 장착 크기·호환성, 발사, 사거리, 런처 패턴, 탄약, Charge, Heat와 Projectile 연결을 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("WeaponId"),
		ECFDAValidationPolicy::NativeDataValidation,
		TEXT("IsDataValid")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFHUDLayoutData"),
		ECFDADomain::UI,
		TEXT("HUD 레이아웃 데이터"),
		TEXT("HUD 7개 Slot의 배치와 해상도별 Geometry·Typography scale 및 판독성 하한을 정의합니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitFName,
		TEXT("ProfileId"),
		ECFDAValidationPolicy::CustomContract,
		TEXT("ValidateLayoutData")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFHUDVisualData"),
		ECFDADomain::UI,
		TEXT("HUD 시각 자산 데이터"),
		TEXT("VehiclePanel, Armor, Defense, Speed, Radar, Target, Weapon HUD가 사용할 전용 Texture·Material soft reference를 관리합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFUIDensityData"),
		ECFDADomain::UI,
		TEXT("UI 밀도 데이터"),
		TEXT("Compact·Standard·Expanded UI의 Padding, 간격, 행 높이, Icon·Text 비율과 Caption 정책을 정의합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::CustomContract,
		TEXT("ValidateDensityData")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_Re.CFUIStyleData"),
		ECFDADomain::UI,
		TEXT("UI 스타일 데이터"),
		TEXT("CarFight UI 공통 색상, Typography, Font, Spacing, Shape, Motion, Panel·Button·Status와 Semantic Icon token을 정의합니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::CustomContract,
		TEXT("ValidateStyleData")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFDriveStateProfile"),
		ECFDADomain::Authoring,
		TEXT("DriveState 제작 프로파일"),
		TEXT("Editor-only Vehicle Authoring에서 DriveState 판정 임계값과 히스테리시스 behavior payload를 재사용하기 위한 프로파일입니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFDrivetrainProfile"),
		ECFDADomain::Authoring,
		TEXT("구동계 제작 프로파일"),
		TEXT("Editor-only Vehicle Authoring에서 엔진·변속기·구동계 typed balance payload를 재사용하기 위한 프로파일입니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFHandlingProfile"),
		ECFDADomain::Authoring,
		TEXT("핸들링 제작 프로파일"),
		TEXT("Editor-only Vehicle Authoring에서 조향과 핸들링 typed balance payload를 재사용하기 위한 프로파일입니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFPerformanceProfile"),
		ECFDADomain::Authoring,
		TEXT("성능 제작 프로파일"),
		TEXT("Editor-only Vehicle Authoring에서 차량 성능, RPM/Redline과 선택적 엔진 TorqueCurve typed payload를 재사용하기 위한 프로파일입니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFVehicleBaseProfile"),
		ECFDADomain::Authoring,
		TEXT("차량 기본 제작 프로파일"),
		TEXT("Editor-only Vehicle Authoring에서 기준 질량, 최대 총중량, 내구도와 차량 기본 typed payload를 재사용하기 위한 프로파일입니다."),
		ECFDAIdentityPolicy::NotApplicable,
		ECFDAIdentityResolverKind::None,
		TEXT(""),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFVehicleRecipeData"),
		ECFDADomain::Authoring,
		TEXT("차량 제작 Recipe"),
		TEXT("Guided Vehicle Builder가 Target VehicleData, Profile binding, 주행감, 질량, Hardpoint·Mount와 적용 provenance를 persistent하게 보존하는 Editor-only Authoring SSOT입니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitGuid,
		TEXT("RecipeId"),
		ECFDAValidationPolicy::None,
		TEXT("")));

	CurrentDescriptors.Add(CFDACurrentTypeRegistryPrivate::MakeDescriptor(
		TEXT("/Script/CarFight_ReEditor.CFVehicleRefEvidence"),
		ECFDADomain::Authoring,
		TEXT("차량 Reference Evidence"),
		TEXT("Guided Vehicle Builder가 실차 Reference identity, 출처, atomic claim, conflict·unknown과 deterministic fingerprint를 보존하는 Editor-only research companion입니다."),
		ECFDAIdentityPolicy::Required,
		ECFDAIdentityResolverKind::ExplicitGuid,
		TEXT("EvidenceId"),
		ECFDAValidationPolicy::None,
		TEXT("")));

	// batch 실패 시 caller Registry를 부분 변경하지 않도록 현재 상태를 복제한 candidate Registry입니다.
	FCFDATypeRegistry CandidateRegistry = *this;

	// current descriptor를 exact class path 기준으로 candidate Registry에 순서대로 등록합니다.
	for (const FCFDASemanticDescriptor& Descriptor : CurrentDescriptors)
	{
		// 현재 descriptor 등록 실패 이유를 보존할 문자열입니다.
		FString RegistrationError;
		if (!CandidateRegistry.RegisterDescriptor(Descriptor, &RegistrationError))
		{
			if (OutError)
			{
				*OutError = FString::Printf(
					TEXT("Current CarFight semantic descriptor 등록 실패: %s / %s"),
					*Descriptor.ClassPath,
					*RegistrationError);
			}
			return false;
		}
	}

	// 27종 모두 성공한 경우에만 caller Registry 전체 상태를 한 번에 교체합니다.
	*this = MoveTemp(CandidateRegistry);
	return true;
}