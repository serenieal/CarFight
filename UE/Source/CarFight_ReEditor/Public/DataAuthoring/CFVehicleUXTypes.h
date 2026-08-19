// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleUXTypes.h
// Version: v1.0.0
// Date: 2026-08-18
// Description: DAUTH-P0-11 Frozen 24.90~24.94 Workspace completeness용 Common Authoring facade C++ value contract입니다.
// Scope: Shared Profile B2 adapter, External Drift 3-way review/recovery, New Vehicle/Mesh-only record creation을 위한 Editor-only typed value를 제공합니다.
// Changelog:
// - v1.0.0: Frozen 24.91~24.94 public value contract 최초 구현.
// Migration:
// - AI typed semantic contract를 확장하지 않으며 Shared Profile payload write는 SlateUI/Automation에만 허용합니다.
// - Runtime UCFVehicleData schema, Inventory/Fitting, Content Asset 저장 계약을 변경하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFBatchImport.h"
#include "DataAuthoring/CFVehicleAIContract.h"

class UCFVehicleData;
class UCFVehicleRecipeData;
class UStaticMesh;

/** Frozen 24.59 External Drift의 explicit field/group recovery 선택입니다. */
enum class ECFVehicleDriftDecision : uint8
{
	KeepAuthoring,
	PreserveRawAsLegacyPin,
	PromoteRawToAdvancedOverride
};

/** Last Applied / Current Raw / Current Authoring 3-way 한 field row입니다. */
struct FCFVehicleDriftReviewRow
{
	// Review 대상 exact Stable Field Path입니다.
	FCFVehicleFieldPath FieldPath;

	// 기존 asset이 exact Applied value 저장 전 세대인지 여부를 구분합니다.
	bool bHasLastAppliedValue = false;

	// 새 Apply부터 보존되는 exact Last Applied typed value입니다.
	FCFVehicleFieldValue LastAppliedValue;

	// Backward compatibility와 stale authority를 유지하는 Last Applied value hash입니다.
	FString LastAppliedValueHash;

	// Review 시점 Current Target Definition의 exact raw value입니다.
	FCFVehicleFieldValue CurrentRawValue;

	// Review 시점 Current Authoring Resolve의 exact effective value입니다.
	FCFVehicleFieldValue CurrentAuthoringValue;

	// Frozen Registry가 이 field에 Advanced Override를 허용하는지 여부입니다.
	bool bAdvancedOverrideAllowed = false;
};

/** External Drift review를 current hash/source/revision에 binding한 mutation0 결과입니다. */
struct FCFVehicleDriftReviewResult
{
	// Common typed R0 result envelope입니다.
	FCFAuthoringOpResult Operation;

	// Current External Drift field를 canonical path 순서로 보존한 3-way rows입니다.
	TArray<FCFVehicleDriftReviewRow> Rows;

	// Review를 binding하는 current Recipe fingerprint입니다.
	FString ExpectedRecipeFingerprint;

	// Review를 binding하는 current full Target Definition hash입니다.
	FString ExpectedTargetDefinitionHash;

	// Review를 binding하는 current effective Source signature입니다.
	FString ExpectedSourceSignature;

	// Review를 binding하는 Resolver semantic contract revision입니다.
	int32 ResolverContractRevision = 0;
};

/** Keep/Rebase/Advanced Override decision preview/commit 공용 request입니다. */
struct FCFVehicleDriftDecisionRequest
{
	// Current managed Recipe/Target read authority입니다.
	FCFVehicleAuthoringReadRequest ReadRequest;

	// Field decision이면 1개, group decision이면 N개의 exact reviewed field paths입니다.
	TArray<FCFVehicleFieldPath> FieldPaths;

	// 사용자가 explicit하게 선택한 Frozen recovery operation입니다.
	ECFVehicleDriftDecision Decision = ECFVehicleDriftDecision::KeepAuthoring;

	// PromoteRawToAdvancedOverride에서 필수인 사람이 읽는 이유입니다.
	FString OverrideReason;

	// R2 persistent Recipe ownership mutation 승인 여부입니다. KeepAuthoring은 false여도 됩니다.
	bool bOwnershipWriteApproved = false;

	// Preview ProposalHash와 exact 일치해야 하는 approval scope입니다.
	FString ApprovalScopeHash;
};

/** Drift decision의 mutation0 prospective result와 exact approval evidence입니다. */
struct FCFVehicleDriftDecisionPreview
{
	// Fresh 3-way review authority입니다.
	FCFVehicleDriftReviewResult Review;

	// Exact operation/field set/current evidence에 binding된 proposal입니다.
	FCFAuthoringProposal Proposal;

	// Preserve/Override에서 transient Recipe로 다시 계산한 prospective Resolve입니다.
	FCFVehicleResolveResult ProspectiveResolveResult;
};

/** Profile Editor에서 허용된 B2 numeric column 하나를 바꾸기 위한 typed request입니다. */
struct FCFProfileNumericEditRequest
{
	// Frozen 5-domain Shared Profile UObject입니다.
	UObject* ProfileObject = nullptr;

	// Profile class와 exact 일치해야 하는 Frozen Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// CFBatchColumnRegistry ProfileNumericEdit allowlist의 stable ColumnId입니다.
	FString ColumnId;

	// Batch codec가 property type에 맞게 검증할 canonical numeric text입니다.
	FString CanonicalNumericValue;

	// SlateUI 또는 Automation만 허용하며 AI는 P0 contract상 거부합니다.
	ECFAuthoringCallerKind CallerKind = ECFAuthoringCallerKind::Unknown;
};

/** Shared Profile edit가 영향을 주는 한 Recipe/Vehicle navigation row입니다. */
struct FCFProfileImpactVehicle
{
	// Shared Profile을 참조하는 affected Recipe identity입니다.
	FString RecipePath;

	// 해당 Recipe의 Runtime canonical Vehicle Definition identity입니다.
	FString TargetPath;

	// Profile change 뒤 prospective Definition pending field 수입니다.
	int32 PendingDefinitionChangeCount = 0;

	// Affected Vehicle prospective Resolver terminal status입니다.
	ECFVehicleResolveStatus ResolveStatus = ECFVehicleResolveStatus::Error;
};

/** Existing B2 Preview/Approval을 normal Profile Editor route에 투영한 mutation0 result입니다. */
struct FCFProfileNumericEditPreview
{
	// Common typed operation result입니다.
	FCFAuthoringOpResult Operation;

	// Commit 시 그대로 넘길 exact B2 transient Session입니다.
	FCFBatchImportSession Session;

	// Commit 시 그대로 넘길 exact B2 approval evidence입니다.
	FCFBatchCommitApproval Approval;

	// Profile impact navigation rows입니다.
	TArray<FCFProfileImpactVehicle> AffectedVehicles;

	// 기존 B2 summary의 affected Vehicle 수입니다.
	int32 AffectedVehicleCount = 0;

	// 기존 B2 summary의 prospective pending Definition field 총수입니다.
	int32 PendingDefinitionChangeCount = 0;
};

/** Existing B2 source commit terminal result를 normal Profile Editor에 반환합니다. */
struct FCFProfileNumericEditResult
{
	// Common typed operation result입니다.
	FCFAuthoringOpResult Operation;

	// Existing B2 machine-readable terminal result입니다.
	FCFBatchAuthoringCommitResult BatchCommitResult;
};

/** New Vehicle 또는 Mesh-only Candidate에서 Definition+Recipe record를 만들기 위한 explicit request입니다. */
struct FCFVehicleRecordCreateRequest
{
	// New UCFVehicleData package long name입니다.
	FString DefinitionPackageName;

	// New UCFVehicleData object name입니다.
	FName DefinitionAssetName = NAME_None;

	// New UCFVehicleRecipeData package long name입니다.
	FString RecipePackageName;

	// New UCFVehicleRecipeData object name입니다.
	FName RecipeAssetName = NAME_None;

	// Mesh-only Candidate에서 explicit하게 선택한 optional Chassis StaticMesh입니다.
	TSoftObjectPtr<UStaticMesh> ChassisMesh;

	// 사용자가 explicit하게 지정한 5 Profile bindings입니다. 빈 binding은 추론하지 않습니다.
	FCFVehicleProfileBindings ProfileBindings;

	// SlateUI 또는 Automation만 허용하며 P0 AI record-creation contract는 새로 열지 않습니다.
	ECFAuthoringCallerKind CallerKind = ECFAuthoringCallerKind::Unknown;

	// R2 record creation explicit approval 여부입니다.
	bool bOwnershipWriteApproved = false;

	// Preview ProposalHash와 exact 일치해야 하는 approval scope입니다.
	FString ApprovalScopeHash;
};

/** Definition+Recipe record creation의 mutation0 validation/proposal입니다. */
struct FCFVehicleRecordCreatePreview
{
	// Common typed R2 preview result입니다.
	FCFAuthoringOpResult Operation;

	// Exact two-record creation approval evidence입니다.
	FCFAuthoringProposal Proposal;

	// 생성될 Definition object path입니다.
	FSoftObjectPath ProspectiveDefinitionPath;

	// 생성될 Recipe object path입니다.
	FSoftObjectPath ProspectiveRecipePath;
};

/** Definition+Recipe record creation terminal result입니다. */
struct FCFVehicleRecordCreateResult
{
	// Common typed terminal result입니다.
	FCFAuthoringOpResult Operation;

	// 성공 시 생성된 canonical Definition입니다.
	UCFVehicleData* CreatedDefinition = nullptr;

	// 성공 시 생성된 Editor-only Recipe입니다.
	UCFVehicleRecipeData* CreatedRecipe = nullptr;
};
