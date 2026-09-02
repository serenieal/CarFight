// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleFieldRegistry.h
// Version: v1.6.0
// Date: 2026-09-01
// Description: UCFVehicleData 134 leaf pattern의 P0 Authoring Registry 계약입니다.
// Scope: Resolver owner/rule/adoption/override/dependency metadata와 Reflection coverage 검증을 제공합니다.
// Changelog:
// - v1.6.0: ESH-01 bUseEngineTorqueCurve + atomic EngineTorqueCurve 2개 leaf를 Performance owner로 추가해 coverage를 132→134로 확장.
// - v1.5.0: WSA-P0-01 WheelAnchor RelativeScale 4개와 bUseWheelSocketScale 1개를 추가해 Registry/Reflection coverage를 127→132로 확장.
// - v1.4.0: VB-P0-05 설계 검수 교정으로 TransmissionRatios를 atomic typed ratio-set 1개로 복원해 coverage를 128→127로 정정.
// - v1.3.0: CF-FQ-040 VB-P0-05 ChassisWidth + UE 5.8 Transmission 9개 leaf를 추가하고 wheel geometry를 VehicleBase fallback+AssetDerived policy로 승격해 coverage를 118→128로 확장.
// - v1.2.0: UI-P0-06 explicit RedlineStartRPM schema 추가에 맞춰 bidirectional Registry coverage를 117→118로 확장.
// - v1.1.0: DAUTH-P0-08C Frozen Section 22.17 RequiredDependencies metadata를 활성화.
// - v1.0.0: DAUTH-P0-08B 117 leaf Registry와 bidirectional coverage API 최초 구현.
// Migration:
// - WSA-P0-01 additive Runtime schema까지 Reflection으로 current source coverage를 검사하며 기존 field rename/delete 없이 leaf를 확장합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"

/** Registry leaf 하나의 stable path와 P0 Resolver ownership metadata입니다. */
struct FCFVehicleFieldDescriptor
{
	// Scalar 또는 Stable-ID wildcard collection의 structured path pattern입니다.
	FCFVehicleFieldPath StablePathPattern;

	// 이 leaf의 primary Profile Domain이며 Profile 비소유 field는 None입니다.
	ECFVehicleProfileDomain PrimaryProfileDomain = ECFVehicleProfileDomain::None;

	// 이 leaf를 생성할 P0 Resolver rule입니다.
	ECFVehicleResolveRule ResolveRule = ECFVehicleResolveRule::ProjectCompatibilityDefault;

	// 이 leaf가 속한 user-facing Adoption Group입니다.
	ECFVehicleAdoptGroup AdoptionGroup = ECFVehicleAdoptGroup::LegacyTechnical;

	// P0 precedence에서 허용하는 ECFVehicleSourceType bit mask입니다.
	uint64 AllowedSourceMask = 0;

	// Advanced Leaf Override가 허용되는 field인지 여부입니다.
	bool bAdvancedOverrideAllowed = false;

	// Stable array identity 자체를 구성하는 field인지 여부입니다.
	bool bIdentityField = false;

	// 일반 Authoring이 아니라 serialized compatibility 보존 전용인지 여부입니다.
	bool bLegacySerialized = false;

	// Resolver가 이 field를 계산할 때 필요한 dependency key 목록입니다.
	TArray<FName> RequiredDependencies;

	// Registry 정렬/비교용 canonical wildcard path를 반환합니다.
	FString GetCanonicalPattern() const
	{
		return StablePathPattern.ToCanonicalString(true);
	}
};

/** Current UCFVehicleData schema와 Current 134 Resolver Map을 연결하는 정적 Registry입니다. */
class FCFVehicleFieldRegistry
{
public:
	// Current P0 descriptor 134개를 canonical 순서로 반환합니다.
	static const TArray<FCFVehicleFieldDescriptor>& GetDescriptors();

	// Current UCFVehicleData Reflection에서 실제 leaf pattern을 재발견합니다.
	static TSet<FString> DiscoverDefinitionLeafPatterns();

	// Registry와 Current Reflection leaf를 양방향 비교하고 문제 목록을 반환합니다.
	static bool ValidateCoverage(TArray<FString>& OutErrors);

	// Registry가 가져야 하는 Current P0 leaf pattern 개수입니다.
	static constexpr int32 ExpectedLeafPatternCount = 134;
};
