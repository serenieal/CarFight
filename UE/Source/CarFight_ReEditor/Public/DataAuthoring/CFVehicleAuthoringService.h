// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringService.h
// Version: v1.3.0
// Date: 2026-08-18
// Description: DAUTH-P0-08I~P0-11 UI/AI가 공유하는 single-Vehicle Authoring facade입니다.
// Scope: Existing Snapshot/Resolver/Import/Adoption/Batch B2/Apply core orchestration과 Frozen 24.90~24.94 Workspace completeness를 제공합니다.
// Changelog:
// - v1.3.0: P0-11 Shared Profile B2 adapter, External Drift 3-way recovery, New Vehicle/Mesh-only two-record creation facade를 추가.
// - v1.2.0: P0-10 managed-target lookup, Reference Compare, Adoption, Wheel Measurement facade를 추가해 Wizard/Workspace가 Core를 직접 호출하지 않게 함.
// - v1.1.0: P0-09 Initial Import reviewed R2 proposal/commit facade를 추가해 Slate가 Import Core를 직접 호출하지 않게 함.
// - v1.0.0: Section 25 Common Authoring Service Foundation 최초 구현.
// Migration:
// - Resolver/validation/source precedence를 재구현하지 않습니다.
// - Target UCFVehicleData write는 ApplyResolvedVehicle에서도 FCFVehicleApplyService만 호출합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleUXTypes.h"

/** Slate/UI/AI/Automation이 같은 Authoring Core를 호출하게 만드는 Editor-only facade입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleAuthoringService
{
public:
	// Project Asset Registry에서 Vehicle Definition/Recipe identity를 read-only로 나열합니다.
	static bool ListVehicles(const FCFVehicleListRequest& Request, FCFVehicleListResult& OutResult);

	// Project Asset Registry에서 Frozen 5-domain Profile을 read-only로 나열합니다.
	static bool ListProfiles(const FCFProfileListRequest& Request, FCFProfileListResult& OutResult);

	// Exact existing Profile을 shared SnapshotBuilder로 read-only snapshot합니다.
	static bool ReadProfile(const FCFProfileReadRequest& Request, FCFProfileReadResult& OutResult);

	// Write-before-read authority에 사용할 bounded Vehicle/Recipe context를 반환합니다.
	static bool ReadVehicleContext(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleContextReadResult& OutResult);

	// Live UObject를 shared SnapshotBuilder/AssetReader로 끊은 뒤 기존 Pure Resolver를 호출합니다.
	static bool ResolveVehiclePreview(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleResolveReadResult& OutResult);

	// Shared Resolver FieldDiff를 별도 비교 로직 없이 반환합니다.
	static bool ReadPendingDiff(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleDiffReadResult& OutResult);

	// Shared Resolver SourceTrace를 optional exact Stable Field Path로 projection합니다.
	static bool ReadSourceTrace(
		const FCFVehicleAuthoringReadRequest& Request,
		const TArray<FCFVehicleFieldPath>& OptionalFieldPaths,
		FCFVehicleTraceReadResult& OutResult);

	// Recipe/Resolver/Definition/Sync validation layer를 shared Resolver result에서 반환합니다.
	static bool ReadValidation(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleValidationReadResult& OutResult);

		// Shared R16 StaleReport를 External Drift read contract로 반환합니다.
	static bool ReviewExternalDrift(const FCFVehicleAuthoringReadRequest& Request, FCFVehicleDriftReadResult& OutResult);

	// Target을 binding한 managed/imported Recipe 존재 여부를 Editor-only Registry에서 read-only로 조회합니다.
	static bool ReadManagedTarget(UCFVehicleData* TargetVehicleData, FCFVehicleManagedReadResult& OutResult);

	// 117 Stable Field Registry projection으로 Current/Resolved A-B Reference Compare를 read-only 생성합니다.
	static bool CompareReferenceVehicles(
		const FCFVehicleReferenceCompareRequest& Request,
		FCFVehicleReferenceCompareResult& OutResult);

	// Existing Import Core를 사용해 group/field Legacy Pin Adoption prospective result와 exact R2 proposal을 만듭니다.
	static bool PreviewAdoption(
		const FCFVehicleAdoptionRequest& Request,
		FCFVehicleAdoptionPreviewResult& OutResult);

	// Fresh adoption preview/approval을 재검사한 뒤 Recipe-only Adoption transaction을 existing Import Core에 위임합니다.
	static bool CommitAdoption(
		const FCFVehicleAdoptionRequest& Request,
		FCFAuthoringOpResult& OutResult);

	// Shared Resolver가 만든 current Wheel measurement proposals를 read-only 반환합니다.
	static bool ReadMeasurementProposals(
		const FCFVehicleMeasurementReadRequest& Request,
		FCFVehicleMeasurementReadResult& OutResult);

		// Existing B2 ProfileNumericEdit pipeline으로 Shared Profile numeric edit와 affected Vehicle impact를 mutation0 preview합니다.
	static bool PreviewProfileNumericEdit(
		const FCFProfileNumericEditRequest& Request,
		FCFProfileNumericEditPreview& OutPreview);

	// UI가 review한 exact B2 Session/Approval만 existing Batch source commit에 위임하며 AI caller는 거부합니다.
	static bool CommitProfileNumericEdit(
		const FCFProfileNumericEditRequest& Request,
		const FCFProfileNumericEditPreview& ApprovedPreview,
		FCFProfileNumericEditResult& OutResult);

	// Current External Drift를 Last Applied / Current Raw / Current Authoring 3-way rows로 read-only 구성합니다.
	static bool BuildDriftReview(
		const FCFVehicleAuthoringReadRequest& Request,
		FCFVehicleDriftReviewResult& OutReview);

	// Keep/Rebase/Advanced Override decision을 current evidence에 binding하고 persistent mutation 없이 prospective resolve합니다.
	static bool PreviewDriftDecision(
		const FCFVehicleDriftDecisionRequest& Request,
		FCFVehicleDriftDecisionPreview& OutPreview);

	// Fresh preview/approval을 재검사하고 Preserve/Override만 Recipe transaction으로 commit하며 Keep은 mutation0 review token만 반환합니다.
	static bool CommitDriftDecision(
		const FCFVehicleDriftDecisionRequest& Request,
		FCFAuthoringOpResult& OutResult);

	// New Vehicle/Mesh-only request의 two-record path/collision/type를 검증하고 exact R2 proposal을 만듭니다.
	static bool PreviewVehicleRecords(
		const FCFVehicleRecordCreateRequest& Request,
		FCFVehicleRecordCreatePreview& OutPreview);

	// Fresh R2 proposal을 재검사한 뒤 Definition+Recipe만 한 transaction으로 생성하며 Apply/Save/Profile inference는 하지 않습니다.
	static bool CreateVehicleRecords(
		const FCFVehicleRecordCreateRequest& Request,
		FCFVehicleRecordCreateResult& OutResult);

	// Measurement decision을 transient Recipe Snapshot에만 적용해 prospective resolve와 exact R2 proposal을 만듭니다.
	static bool PreviewMeasurementDecision(
		const FCFVehicleMeasurementRequest& Request,
		FCFVehicleMeasurementPreviewResult& OutResult);

	// Fresh proposal/asset fingerprint/approval을 재검사한 뒤 Recipe AssetAdoption만 transaction mutation합니다.
	static bool CommitMeasurementDecision(
		const FCFVehicleMeasurementRequest& Request,
		FCFAuthoringOpResult& OutResult);

	// Existing unmanaged Definition을 수정하지 않고 exact Initial Import summary/approval proposal을 만듭니다.
	static bool BuildInitialImportProposal(
		const FCFVehicleInitialImportRequest& Request,
		FCFVehicleInitialImportPreviewResult& OutResult);

	// Exact OwnershipWrite approval/current Target hash를 재검사한 뒤 새 Recipe record만 만들고 Existing Definition을 import합니다.
	static bool ImportExistingDefinition(
		const FCFVehicleInitialImportRequest& Request,
		FCFVehicleInitialImportResult& OutResult);

	// Typed semantic command를 transient Recipe Snapshot에만 적용해 prospective Resolve/Proposal을 생성합니다.
	static bool PreviewRecipeChange(const FCFVehicleRecipeChangeRequest& Request, FCFVehicleRecipePreviewResult& OutResult);

	// Fresh ProposalHash/expected fingerprint/approval을 재검사한 뒤 Recipe만 transaction mutation합니다.
	static bool CommitRecipeChange(const FCFVehicleRecipeChangeRequest& Request, FCFAuthoringOpResult& OutResult);

	// Reviewed shared ApplyRequest에서 exact R3 DefinitionApply approval proposal을 생성합니다.
	static bool BuildApplyApprovalProposal(
		const FCFVehicleApplyRequest& ApplyRequest,
		FCFAuthoringProposal& OutProposal,
		FCFAuthoringOpResult& OutResult);

	// R3 approval/precondition을 확인한 뒤 Target writer를 새로 만들지 않고 FCFVehicleApplyService를 호출합니다.
	static bool ApplyResolvedVehicle(const FCFVehicleApplyOpRequest& Request, FCFAuthoringOpResult& OutResult);

	// Shared Resolver FieldDiff exact rows의 deterministic hash를 반환합니다.
	static FString BuildDiffHash(const TArray<FCFVehicleFieldDiff>& FieldDiff);
};
