// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderHardpointIntegrity.h
// Version: v1.3.1
// Date: 2026-09-04
// Description: CF-FQ-047 Builder-private Socket/semantic readback + typed Physics receipt compatibility/drift boundary contract입니다.
// Scope: Resolver fingerprint와 분리된 Editor-only read-only Chassis Socket inventory 및 mutation0 Physics structural compatibility만 소유합니다.
// Changelog:
// - v1.3.1: Step 5 승인 직후 Step 7 Apply 전 정상 상태를 보존하도록 ReceiptResolvedDefinitionHash == CurrentProspectiveResolvedDefinitionHash를 Exact로 명시합니다. Current Target hash 일치는 Exact의 필수조건이 아닙니다.
// - v1.3.0: Step 5 Physics receipt를 Exact/Hardpoint-Mount-only equivalent/Stale/Blocked로 분류하는 typed compatibility와 단일 structural drift boundary owner를 추가.
// Migration:
// - v1.3.1부터 Step 5 Exact는 승인된 prospective Definition identity를 기준으로 합니다. Current Target과의 차이는 Step 7 Apply Pending일 수 있으므로 stale로 해석하지 않습니다.
// - 기존 ValidatePhysicsReceiptRefreshBoundary bool API는 typed structural evaluator의 projection으로 유지합니다. Hardpoint/Mount-only equivalence는 receipt를 자동 재작성하거나 Save하지 않습니다.

#pragma once

#include "CoreMinimal.h"

class UCFVehicleData;
class UCFVehicleRecipeData;
class UStaticMesh;
struct FCFBuilderFinalReviewResult;
struct FCFVehicleFieldDiff;

/** Physics receipt 대비 current/prospective Definition structural drift가 물리적으로 동등한지 표현합니다. */
enum class ECFBuilderPhysicsStructuralDriftBoundary : uint8
{
	Equivalent,
	NotEquivalent,
	Blocked
};

/** Physics structural drift 판정과 사람이 읽을 진단을 함께 반환합니다. */
struct FCFBuilderPhysicsStructuralDriftBoundaryResult
{
	// Physics 관점 structural drift 판정입니다.
	ECFBuilderPhysicsStructuralDriftBoundary Boundary = ECFBuilderPhysicsStructuralDriftBoundary::Blocked;

	// 판정 근거 또는 fail-closed 이유입니다.
	FString Diagnostic;
};

/** Current persistent Physics receipt가 Step 5 진행에 계속 유효한지 표현합니다. */
enum class ECFBuilderPhysicsReceiptCompatibility : uint8
{
	Exact,
	PhysicsEquivalentStructuralDrift,
	PhysicsStale,
	Blocked
};

/** Step 5 Physics receipt compatibility와 USER/diagnostic projection의 근거를 함께 반환합니다. */
struct FCFBuilderPhysicsReceiptCompatibilityResult
{
	// Current Step 5 Physics receipt compatibility 판정입니다.
	ECFBuilderPhysicsReceiptCompatibility Compatibility = ECFBuilderPhysicsReceiptCompatibility::Blocked;

	// 판정 근거 또는 복구가 필요한 이유입니다.
	FString Diagnostic;

	// Step 5를 다시 작성하지 않고 완료 상태로 유지할 수 있는지 반환합니다.
	bool IsComplete() const
	{
		return Compatibility == ECFBuilderPhysicsReceiptCompatibility::Exact
			|| Compatibility == ECFBuilderPhysicsReceiptCompatibility::PhysicsEquivalentStructuralDrift;
	}
};

/** Chassis Socket 하나가 current Recipe/Target Hardpoint semantic과 어떤 관계인지 표현합니다. */
enum class ECFBuilderHardpointSocketClassification : uint8
{
	StandardAdoptable,
	NonCanonicalHardpointLike,
	AlreadyRecipeBound,
	AlreadyTargetBound,
	IdentityCollision,
	UnrelatedSocket
};

/** Resolver AssetSnapshot과 독립적으로 읽은 Chassis Socket 진단 한 행입니다. */
struct FCFBuilderChassisSocketInventoryEntry
{
	FName SocketName = NAME_None;
	FName ParsedLocationCategory = NAME_None;
	FName ParsedLocationSlotId = NAME_None;
	int32 ParsedStandardIndex = 0;
	FVector RelativeLocation = FVector::ZeroVector;
	FRotator RelativeRotation = FRotator::ZeroRotator;
	FVector RelativeScale = FVector::OneVector;
	ECFBuilderHardpointSocketClassification Classification = ECFBuilderHardpointSocketClassification::UnrelatedSocket;
	FString Diagnostic;
};

/** Current Chassis의 Builder-private read-only Socket inventory입니다. */
struct FCFBuilderChassisSocketInventory
{
	FSoftObjectPath ChassisMeshPath;
	TArray<FCFBuilderChassisSocketInventoryEntry> Entries;

	int32 CountByClassification(ECFBuilderHardpointSocketClassification Classification) const;
	const FCFBuilderChassisSocketInventoryEntry* FindBySocketName(FName SocketName) const;
};

/** Step 6에서 USER가 확인할 authored Hardpoint와 연결된 Mount stable identity 묶음입니다. */
struct FCFBuilderHardpointMountRelationFact
{
	FName LocationSlotId = NAME_None;
	FName SocketName = NAME_None;
	TArray<FName> MountProfileIds;
};

/** Step 6/7/8이 공유하는 Hardpoint/Mount semantic readback입니다. */
struct FCFBuilderHardpointSemanticFacts
{
	bool bAvailable = false;

	// Recipe authored semantic입니다. Runtime/current Target authority가 아닙니다.
	int32 AuthoredHardpointCount = 0;
	int32 AuthoredMountCount = 0;
	TArray<FCFBuilderHardpointMountRelationFact> AuthoredRelations;

	// 현재 Target VehicleData를 실제로 읽었는지 여부입니다. false일 때 0개로 해석하지 않습니다.
	bool bHasCurrentTarget = false;

	// 현재 Target VehicleData readback입니다.
	int32 CurrentTargetHardpointCount = 0;
	int32 CurrentTargetMountCount = 0;

	// fresh Final Review FieldDiff에서 current Target에 Add/Remove를 적용했을 prospective count입니다.
	bool bHasProspectiveTargetCounts = false;
	int32 ProspectiveTargetHardpointCount = 0;
	int32 ProspectiveTargetMountCount = 0;

	FString Diagnostic;
};

/**
 * CF-FQ-047 Hardpoint authoring integrity helper.
 *
 * 중요:
 * - 이 helper의 inventory는 FCFVehicleAssetSnapshot에 합치지 않습니다.
 * - ChassisLayoutFingerprint / ResolvedDefinitionHash / DefinitionApply / Driving acceptance authority에 참여하지 않습니다.
 * - mutation authority를 갖지 않습니다.
 */
class CARFIGHT_REEDITOR_API FCFVehicleBuilderHardpointIntegrity
{
public:
	// HP_<KnownCategory>_<Index>를 current Standard 생성 규칙(%02d)과 exact round-trip해 canonical identity만 반환합니다.
	static bool TryParseCanonicalStandardSocketName(
		FName SocketName,
		const TArray<FName>& StandardCategories,
		FName& OutLocationCategory,
		FName& OutLocationSlotId,
		int32& OutIndex);

	// Current Chassis 전체 Socket을 read-only로 읽고 Recipe/Target semantic과 deterministic하게 분류합니다.
	static bool ReadChassisSocketInventory(
		const UStaticMesh* ChassisMesh,
		const UCFVehicleRecipeData* Recipe,
		const UCFVehicleData* TargetVehicleData,
		const TArray<FName>& StandardCategories,
		FCFBuilderChassisSocketInventory& OutInventory,
		FString& OutError);

	// Recipe authored relation + current Target + optional fresh Final Review prospective count를 mutation0으로 projection합니다.
	static bool BuildHardpointSemanticFacts(
		const UCFVehicleRecipeData* Recipe,
		const UCFVehicleData* TargetVehicleData,
		const FCFBuilderFinalReviewResult* FinalReview,
		FCFBuilderHardpointSemanticFacts& OutFacts,
		FString& OutError);

	// Receipt baseline/current Target/prospective Definition/pending diff를 읽어 Hardpoint/Mount-only structural equivalence를 단일 typed owner에서 판정합니다.
	static FCFBuilderPhysicsStructuralDriftBoundaryResult EvaluatePhysicsStructuralDriftBoundary(
		const FString& ReceiptResolvedDefinitionHash,
		const FString& CurrentTargetDefinitionHash,
		const FString& CurrentProspectiveResolvedDefinitionHash,
		const TArray<FCFVehicleFieldDiff>& CurrentPendingDiff);

	// Fresh Physics provenance 판정과 structural boundary를 결합해 Step 5 receipt compatibility를 mutation0으로 반환합니다.
	static FCFBuilderPhysicsReceiptCompatibilityResult EvaluatePhysicsReceiptCompatibility(
		bool bPhysicsProvenanceCurrent,
		const FString& ReceiptResolvedDefinitionHash,
		const FString& CurrentTargetDefinitionHash,
		const FString& CurrentProspectiveResolvedDefinitionHash,
		const TArray<FCFVehicleFieldDiff>& CurrentPendingDiff,
		const FString& ProvenanceDiagnostic = FString());

	// Legacy receipt-only refresh bool API를 typed structural boundary의 Equivalent projection으로 유지합니다.
	static bool ValidatePhysicsReceiptRefreshBoundary(
		const FString& ReceiptResolvedDefinitionHash,
		const FString& CurrentTargetDefinitionHash,
		const FString& CurrentProspectiveResolvedDefinitionHash,
		const TArray<FCFVehicleFieldDiff>& CurrentPendingDiff,
		FString& OutError);
};
