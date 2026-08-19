// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleImportService.h
// Version: v1.2.0
// Date: 2026-08-18
// Description: DAUTH-P0-08G~P0-11 Existing Definition Import / Adoption / Raw Preservation Core public C++ 계약입니다.
// Scope: Definition Snapshot import, Legacy Pin partition, group/field Adoption과 reviewed Raw→Legacy Pin ownership recovery를 제공합니다.
// Changelog:
// - v1.2.0: Frozen 24.59 Preserve Raw As Legacy Pin이 같은 ownership 규칙을 재사용하도록 snapshot/UObject raw pin primitive를 추가.
// - v1.1.0: P0-09 reviewed Initial Import UI가 persistent Recipe mutation 없이 exact import summary를 얻는 PreviewDefinitionImport를 추가.
// - v1.0.0: Section 22.20~22.21 Import/Legacy Pin/Adoption Foundation 최초 구현.
// Migration:
// - Target UCFVehicleData를 직접 수정하는 Apply API를 포함하지 않습니다.
// - Legacy Serialized Mount field는 preview/commit normal adoption 대상에서 제외합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"
#include "CFVehicleImportService.generated.h"

class UCFVehicleRecipeData;

/** Adoption Preview가 group 전체인지 exact field 하나인지 구분합니다. */
UENUM()
enum class ECFVehicleAdoptionScope : uint8
{
	Group,
	Field
};

/** Existing Definition import가 Recipe migration state에 만든 value-copy 결과입니다. */
USTRUCT()
struct FCFVehicleImportResult
{
	GENERATED_BODY()

	// 검증 완료된 imported Definition hash입니다.
	UPROPERTY()
	FString ImportedDefinitionHash;

	// 일반 semantic/technical Definition field로 보존한 Legacy Pin 수입니다.
	UPROPERTY()
	int32 LegacyPinnedFieldCount = 0;

	// Mount hidden serialized compatibility field로 분리한 exact field 수입니다.
	UPROPERTY()
	int32 LegacySerializedFieldCount = 0;

	// Current Definition에서 Recipe semantic candidate로 lossless direct copy한 semantic field 수입니다.
	UPROPERTY()
	int32 SemanticCandidateFieldCount = 0;
};

/** Recipe mutation 전에 Current/Prospective Resolver 결과와 제거할 Legacy Pin 집합을 고정하는 Adoption Preview입니다. */
USTRUCT()
struct FCFVehicleAdoptionPreview
{
	GENERATED_BODY()

	// Group 또는 Field adoption 중 어떤 preview인지 나타냅니다.
	UPROPERTY()
	ECFVehicleAdoptionScope Scope = ECFVehicleAdoptionScope::Group;

	// Group preview에서 선택한 user-facing Adoption Group입니다.
	UPROPERTY()
	ECFVehicleAdoptGroup AdoptionGroup = ECFVehicleAdoptGroup::VisualAssets;

	// Field preview에서 선택한 exact Stable Field Path입니다.
	UPROPERTY()
	FCFVehicleFieldPath AdoptionFieldPath;

	// Preview를 만든 현재 persistent Recipe semantic fingerprint입니다.
	UPROPERTY()
	FString BaselineRecipeFingerprint;

	// 선택 Pin을 가상 제거한 Recipe Snapshot semantic fingerprint입니다.
	UPROPERTY()
	FString ProspectiveRecipeFingerprint;

	// 승인 시 Recipe.LegacyPinnedFields에서 실제 제거해야 하는 exact paths입니다.
	UPROPERTY()
	TArray<FCFVehicleFieldPath> LegacyPinPathsToRemove;

	// Legacy Pin을 유지한 현재 Resolver preview입니다.
	UPROPERTY()
	FCFVehicleResolveResult CurrentResolveResult;

	// 선택 Pin만 가상 제외한 prospective Resolver preview입니다.
	UPROPERTY()
	FCFVehicleResolveResult ProspectiveResolveResult;

	// 선택 field/group이 새 Source로 실제 해석되어 Recipe-only commit 가능한지 여부입니다.
	UPROPERTY()
	bool bCanCommit = false;

	// Commit 불가 시 사람이 읽을 원인입니다.
	UPROPERTY()
	FString BlockReason;
};

/** Existing Vehicle Definition을 Recipe migration state로 가져오고 explicit Adoption을 관리하는 Editor-only 서비스입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleImportService
{
public:
	// Persistent Recipe를 수정하지 않고 exact Legacy partition/direct semantic candidate count를 계산합니다.
	static bool PreviewDefinitionImport(
		const FCFVehicleDefinitionSnapshot& CurrentDefinition,
		FCFVehicleImportResult& OutResult,
		FString& OutError);

	// Current Definition Snapshot 전체를 Legacy Pin으로 보존하고 direct semantic candidate만 Recipe에 복사합니다.
	static bool ImportDefinitionSnapshot(
		const FCFVehicleDefinitionSnapshot& CurrentDefinition,
		UCFVehicleRecipeData& Recipe,
		FCFVehicleImportResult& OutResult,
		FString& OutError);

	// 선택 user-facing group의 Legacy Pin만 가상 제거해 Source/Diff/Validation preview를 계산합니다.
	static bool PreviewGroupAdoption(
		const FCFVehicleResolveRequest& CurrentRequest,
		ECFVehicleAdoptGroup AdoptionGroup,
		FCFVehicleAdoptionPreview& OutPreview,
		FString& OutError);

	// Source Trace에서 선택한 exact Legacy Pin 하나만 가상 제거하는 advanced preview를 계산합니다.
	static bool PreviewFieldAdoption(
		const FCFVehicleResolveRequest& CurrentRequest,
		const FCFVehicleFieldPath& FieldPath,
		FCFVehicleAdoptionPreview& OutPreview,
		FString& OutError);

		// Reviewed raw field/value를 transient Recipe Snapshot의 Legacy Pin ownership으로 upsert하고 ManageState를 재계산합니다.
	static bool ApplyRawPinsToSnapshot(
		FCFVehicleRecipeSnapshot& RecipeSnapshot,
		const TArray<FCFVehicleFieldOverride>& RawPins,
		FString& OutError);

	// Reviewed raw field/value를 persistent Recipe의 Legacy Pin ownership으로 upsert하며 caller transaction 안에서 사용합니다.
	static bool ApplyRawPins(
		UCFVehicleRecipeData& Recipe,
		const TArray<FCFVehicleFieldOverride>& RawPins,
		FString& OutError);

	// Fresh Preview fingerprint를 검증한 뒤 Legacy Pin 제거/AdoptedGroups/revision만 Recipe에 transaction으로 반영합니다.
	static bool CommitAdoption(
		UCFVehicleRecipeData& Recipe,
		const FCFVehicleAdoptionPreview& ApprovedPreview,
		FString& OutError);
};
