// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringService.h
// Version: v1.10.0
// Date: 2026-09-04
// Description: DAUTH Common Authoring facade + CF-FQ-047 Physics receipt 및 durable AppliedState finalize seam입니다.
// Scope: Existing Snapshot/Resolver/Import/Adoption/Batch B2/Apply core orchestration과 Frozen 24.90~24.94 Workspace completeness를 제공합니다.
// Changelog:
// - v1.10.0: VBHAI-P0-07H에서 Final Review semantic no-diff를 fresh 재검사하고 exact DefinitionApply approval로 Recipe AppliedState만 finalize하는 service facade/scope를 추가.
// - v1.9.0: generic Builder Profile full-exact transaction 의미를 유지하면서 resolved DefinitionHash를 제외한 Physics provenance exactness를 read-only로 조회하는 facade를 추가.
// - v1.8.0: Builder Final Review가 one fresh Resolve를 Validation/Drift/Gameplay projection에 재사용하고 provenance receipt/post-state Undo hardening을 지원.
// - v1.7.0: existing Validation/Drift/Gameplay/Diff/Evidence provenance를 aggregate하는 Final Review R0, exact DefinitionApply delegation과 Builder-owned top transaction guarded Undo facade를 추가.
// - v1.6.0: current Recipe/Target/Resolver/Asset truth를 재사용해 Gameplay Setup completeness와 USER Socket/Fitting 안내를 mutation0로 반환하는 R0 facade를 추가.
// - v1.5.0: 기존 Definition+Recipe에 Reference Evidence + Builder-private 4 Profile을 R2 approval로 생성·binding하는 preview/commit facade를 추가.
// - v1.4.0: VB-P0-05 Builder-private VehicleBase/Drivetrain/Handling/Performance complete payload의 mutation0 preview와 one-transaction typed commit facade를 추가.
// - v1.3.0: P0-11 Shared Profile B2 adapter, External Drift 3-way recovery, New Vehicle/Mesh-only two-record creation facade를 추가.
// - v1.2.0: P0-10 managed-target lookup, Reference Compare, Adoption, Wheel Measurement facade를 추가해 Wizard/Workspace가 Core를 직접 호출하지 않게 함.
// - v1.1.0: P0-09 Initial Import reviewed R2 proposal/commit facade를 추가해 Slate가 Import Core를 직접 호출하지 않게 함.
// - v1.0.0: Section 25 Common Authoring Service Foundation 최초 구현.
// Migration:
// - v1.10.0 AppliedState finalize는 Target field를 수정하거나 Save하지 않으며, 기존 ApplyBuilderFinalReview의 actual Diff Apply 의미를 변경하지 않습니다.
// - v1.9.0 Physics provenance read는 persistent receipt를 수정/저장하지 않으며 PreviewBuilderProfiles/CommitBuilderProfiles의 full resolved DefinitionHash exact comparator는 그대로 유지합니다.
// - Builder companion creation은 기존 CreateVehicleRecords의 Definition+Recipe core 계약을 변경하지 않고 후속 R2 operation으로 Evidence + private 4 Profile만 생성합니다.
// - Builder-private Profile commit은 Recipe에 exact binding되고 Meta.OwnerRecipeId==RecipeId인 4 Profile만 허용하며 기존 Shared Profile B2 mutation을 대체하거나 우회하지 않습니다.
// - Resolver/validation/source precedence를 재구현하지 않습니다.
// - Target UCFVehicleData write는 ApplyResolvedVehicle에서도 FCFVehicleApplyService만 호출합니다.
// - Builder Gameplay Guidance는 Hardpoint/Destroyed FX Socket을 생성·이동하지 않고 기존 Resolver/Asset/Target을 read-only로만 관측합니다.
// - Builder Final Review Apply는 기존 BuildApplyApprovalProposal/ApplyResolvedVehicle/FCFVehicleApplyService만 사용하며 새 Target writer를 만들지 않습니다. Undo는 이 Builder가 만든 exact UE transaction token이 current stack top일 때만 수행합니다.

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

	// Existing Definition+Recipe에 Reference Evidence + private 4 Profile을 만들 exact R2 companion plan을 mutation0 preview합니다.
	static bool PreviewBuilderCompanions(
		const FCFBuilderCompanionRequest& Request,
		FCFBuilderCompanionPreview& OutPreview);

	// Fresh OwnershipWrite approval 뒤 Evidence + private 4 Profile 생성과 Recipe binding을 한 transaction으로 commit합니다.
	static bool CreateBuilderCompanions(
		const FCFBuilderCompanionRequest& Request,
		FCFBuilderCompanionResult& OutResult);

	// Existing Reference Evidence complete research payload replacement를 current fingerprint/Recipe/Target에 binding해 mutation0 R1 preview합니다.
	static bool PreviewBuilderEvidenceRefresh(
		const FCFBuilderEvidenceRefreshRequest& Request,
		FCFBuilderEvidenceRefreshPreview& OutPreview);

	// Fresh preview + AuthoringWrite approval 뒤 exact existing Evidence research payload만 transaction commit하고 Save는 수행하지 않습니다.
	static bool CommitBuilderEvidenceRefresh(
		const FCFBuilderEvidenceRefreshRequest& Request,
		const FCFBuilderEvidenceRefreshPreview& ApprovedPreview,
		FCFBuilderEvidenceRefreshResult& OutResult);

	// Gameplay Setup의 Durability/Defense/DestroyedFx/Hardpoint/Mount/DriveState/WheelVisual/Fitting completeness와 USER 수동 Socket 안내를 R0로 반환합니다.
	static bool ReadBuilderGameplayGuidance(
		const FCFBuilderGameplayGuidanceRequest& Request,
		FCFBuilderGameplayGuidanceResult& OutResult);

	// Final Review Step의 existing Validation/External Drift/Gameplay/Diff/consumed Evidence provenance를 한 번에 mutation0로 집계합니다.
	static bool ReadBuilderFinalReview(
		const FCFBuilderFinalReviewRequest& Request,
		FCFBuilderFinalReviewResult& OutResult);

	// Final Review가 만든 exact DefinitionApply scope를 fresh 재검사한 뒤 existing ApplyResolvedVehicle lane으로 explicit Apply합니다.
	static bool ApplyBuilderFinalReview(
		const FCFBuilderFinalApplyRequest& Request,
		FCFBuilderFinalApplyResult& OutResult);

	// semantic no-diff Final Review의 fresh Recipe/Target/Resolve identity를 binding한 AppliedState finalize approval scope를 만듭니다.
	static FString BuildBuilderAppliedStateFinalizeScope(const FCFBuilderFinalReviewResult& Review);

	// Exact DefinitionApply approval을 fresh Final Review와 재검사한 뒤 Target mutation 없이 Recipe AppliedState만 authoritative state로 finalize합니다.
	static bool FinalizeBuilderAppliedState(
		const FCFBuilderFinalReviewRequest& ReviewRequest,
		const FCFAuthoringCallContext& CallContext,
		FCFAuthoringOpResult& OutResult);

	// ApplyBuilderFinalReview가 발급한 exact Builder-owned UE transaction이 current Undo stack top일 때만 Unreal standard Undo를 수행합니다.
	static bool UndoBuilderFinalApply(
		const FCFBuilderUndoRequest& Request,
		FCFAuthoringOpResult& OutResult);

	// Builder-private 4 Profile complete payload를 current owner/fingerprint/Recipe/Target/Resolver/upstream proposal에 binding해 mutation0 preview합니다.
	static bool PreviewBuilderProfiles(
		const FCFBuilderProfileCommitRequest& Request,
		FCFBuilderProfileCommitPreview& OutPreview);

	// Fresh Builder Profile preview 기준으로 full resolved DefinitionHash를 제외한 Evidence/Claim/Profile/Transmission/Engine/Resolver Physics provenance가 receipt와 exact 같은지 확인합니다.
	static bool IsBuilderPhysicsReceiptProvenanceCurrent(
		const FCFBuilderProfileCommitRequest& Request,
		const FCFBuilderProfileCommitPreview& Preview,
		FString& OutDiagnostic);

	// Fresh preview와 AuthoringWrite approval을 재검사한 뒤 Recipe에 binding된 private 4 Profile complete payload를 한 transaction으로 commit합니다.
	static bool CommitBuilderProfiles(
		const FCFBuilderProfileCommitRequest& Request,
		const FCFBuilderProfileCommitPreview& ApprovedPreview,
		FCFAuthoringOpResult& OutResult);

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

private:
	// 이미 계산한 one fresh Resolve result를 재사용해 Builder Gameplay Guidance projection만 구성합니다.
	static bool BuildBuilderGameplayGuidanceFromResolve(
		const FCFBuilderGameplayGuidanceRequest& Request,
		const FCFVehicleResolveReadResult& ResolveRead,
		FCFBuilderGameplayGuidanceResult& OutResult);
};
