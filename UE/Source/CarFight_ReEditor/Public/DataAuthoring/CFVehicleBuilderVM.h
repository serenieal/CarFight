// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderVM.h
// Version: v1.14.0
// Date: 2026-08-31
// Description: Guided Vehicle Builder Shell + WSA-P0-04 blocked-preview diagnostics transient ViewModel입니다.
// Changelog:
// - v1.14.0: Existing Reference Evidence complete replacement R1의 Preview→explicit AuthoringWrite Commit prepared state를 Step 1 ViewModel에 추가하고, differing ResearchDraft를 refresh candidate로 load할 수 있게 Companion 동일성 검증과 분리.
// - v1.13.0: actual Wagon E2E에서 발견된 Step 4 Ready/NotCaptured navigation deadlock을 교정. Target Apply는 Step 7에 유지하면서 deferred Layout Apply가 가능한 Ready 상태만 forward-progress prerequisite로 인정.
// - v1.12.0: actual Wagon E2E의 NewVehicle private Profile bootstrap을 위해 Recipe-only revision과 lifecycle을 분리하고 expected Resolver Blocked read 허용 helper를 추가.
// - v1.11.0: 전체 Preview가 Blocked여도 이번 호출의 fresh Resolve read가 성공했다면 Step 2~4가 최신 Asset/Socket/Layout truth를 진단할 수 있는 내부 read-state flag 추가.
// - v1.10.0: E2E에서 발견된 Step 2 Wheel Mesh 지정 UX 공백을 기존 typed AssetIntent Recipe-only commit으로 연결하는 Builder wrapper를 추가.
// - v1.9.0: USER 피드백에 따라 Step 3 소켓 준비 UI가 소비할 current Chassis path, effective Wheel Socket 4종, optional Gameplay Socket, found-state read-only helper를 추가.
// - v1.8.0: VB-P0-09 Step 8에서 existing VB-P0-08 saved VehicleData benchmark result, active PIE transient USER test-drive, exact benchmark-bound USER Driving acceptance를 Guided Shell VM에 연결.
// - v1.7.0: VB-P0-09 Step 7에서 existing ReadBuilderFinalReview R0 → explicit DefinitionApply → exact Builder-owned guarded Undo를 Guided Shell VM에 연결.
// - v1.6.0: VB-P0-09 Step 6에서 existing ReadBuilderGameplayGuidance R0 authority를 8영역 completeness/USER Socket guidance/current pending diff projection으로 Guided Shell에 연결.
// - v1.5.0: VB-P0-09 Step 5 PhysicsDraft load, accepted Evidence/private 4 Profile typed Preview→explicit USER Commit, persistent receipt 기반 Complete/Stale evaluator를 연결.
// - v1.4.0: VB-P0-09 Step 1 ResearchDraft load, Evidence discovery, baseline-preserving Companion Preview/Commit, Reference review token/resume을 Guided Shell VM에 연결.
// - v1.3.1: current Step Definition 순서가 evaluator dispatch 순서까지 소유하도록 StepId dispatcher를 추가해 reorder owner를 단일화.
// - v1.3.0: StepId 기반 lookup/setter와 Step별 evaluator를 추가해 순서/index 의미 결합을 제거하고 후속 Step 추가·제거·재배치를 국소화.
// - v1.2.0: MeshPrep/SocketGuide/LayoutCapture가 existing Authoring VM fresh AssetSnapshot/Target readback에서 상태를 파생하도록 연결.
// - v1.1.0: Mesh-only 후보의 기존 two-record VehicleData+Recipe Preview/Commit 경로를 Guided Shell에 재노출.
// - v1.0.0: 기존 FCFVehicleAuthoringVM을 재사용해 차량 목록/선택과 8-step 상태 projection을 제공.
// Migration:
// - Recipe-only AuthoringRevision은 NewVehicle lifecycle을 소모하지 않으며, private Profile bootstrap은 기존 prospective Resolver/Validator를 반드시 통과해야 합니다.
// - 새 writer를 만들지 않습니다. persistent truth와 mutation authority는 기존 Authoring facade가 계속 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleBuilderTypes.h"

/** P0-09 USER Acceptance용 Guided Builder Shell transient ViewModel입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleBuilderVM
{
public:
	// Asset Registry 기반 차량/메시 후보 목록을 기존 Authoring VM으로 새로 읽습니다.
	bool RefreshVehicles(FString& OutError);

	// 목록 row 하나를 current Builder target으로 선택하고 fresh authoring context를 읽습니다.
	bool SelectVehicle(const FCFVehicleListEntry& Entry, FString& OutError);

	// 현재 선택과 authoritative truth를 다시 읽고 Step 상태를 재평가합니다.
	bool RefreshCurrentState(FString& OutError);

	// 선택된 Mesh-only 후보에 대해 기존 two-record 생성 proposal을 mutation0으로 준비합니다.
	bool PrepareSelectedMeshRecordCreate(
		const FString& DefinitionPackageName,
		const FString& DefinitionAssetName,
		const FString& RecipePackageName,
		const FString& RecipeAssetName,
		FCFVehicleRecordCreatePreview& OutPreview,
		FString& OutError);

	// 직전 exact proposal에 explicit OwnershipWrite approval을 붙여 기존 two-record 생성 경로로 commit합니다.
	bool ExecutePreparedMeshRecordCreate(FCFVehicleRecordCreateResult& OutResult, FString& OutError);

	// Project Saved의 AI ResearchDraft JSON을 current Recipe/Target에 exact binding해 mutation 없이 읽습니다.
	bool LoadResearchDraft(FString& OutError);

	// Loaded ResearchDraft와 current companion truth로 baseline-preserving R2 Companion proposal을 mutation 없이 준비합니다.
	bool PrepareResearchCompanions(FCFBuilderCompanionPreview& OutPreview, FString& OutError);

	// 직전 exact Companion proposal에 USER OwnershipWrite approval을 붙여 Evidence/Missing private Profiles를 commit합니다.
	bool ExecutePreparedResearchCompanions(FCFBuilderCompanionResult& OutResult, FString& OutError);

	// Loaded ResearchDraft를 current Existing Evidence complete replacement로 적용할 R1 proposal을 mutation 없이 준비합니다.
	bool PrepareReferenceEvidenceRefresh(FCFBuilderEvidenceRefreshPreview& OutPreview, FString& OutError);

	// 직전 exact Evidence Refresh proposal에 USER AuthoringWrite approval을 붙여 existing Evidence research payload만 commit합니다.
	bool ExecutePreparedEvidenceRefresh(FCFBuilderEvidenceRefreshResult& OutResult, FString& OutError);

	// Current Evidence fingerprint와 RecipeId를 USER-reviewed Reference token으로 local Editor settings에 기록합니다.
	bool AcceptCurrentReferenceSet(FString& OutError);

	// AI가 작성해야 하는 current Project Saved ResearchDraft path를 반환합니다.
	FString GetResearchDraftPath() const;

	// Current managed Step 1에 load된 ResearchDraft가 있는지 반환합니다.
	bool HasLoadedResearchDraft() const { return bHasLoadedResearchDraft; }

	// Current managed Step 1에 load된 ResearchDraft를 반환합니다.
	const FCFBuilderResearchDraft& GetLoadedResearchDraft() const { return LoadedResearchDraft; }

	// Current Recipe에 exact binding된 Reference Evidence를 반환합니다.
	UCFVehicleRefEvidence* GetCurrentReferenceEvidence() const { return CurrentReferenceEvidence.Get(); }

	// Current Recipe에 exact binding된 Reference Evidence object path를 반환합니다.
	const FSoftObjectPath& GetCurrentReferenceEvidencePath() const { return CurrentReferenceEvidencePath; }

	// USER-facing Step 1 Reference identity/source/claim/conflict/unknown 요약을 만듭니다.
	FString BuildReferenceSummary() const;

	// Project Saved의 AI PhysicsDraft JSON을 current Recipe/Target/accepted Evidence에 exact binding해 mutation 없이 읽습니다.
	bool LoadPhysicsProposalDraft(FString& OutError);

	// Loaded PhysicsDraft의 complete private 4 Profile payload를 existing typed facade로 mutation0 preview합니다.
	bool PreparePhysicsProposal(FCFBuilderProfileCommitPreview& OutPreview, FString& OutError);

	// 직전 exact Physics Proposal preview에 USER AuthoringWrite approval을 붙여 private 4 Profile + Builder receipt를 commit합니다.
	bool ExecutePreparedPhysicsProposal(FCFAuthoringOpResult& OutResult, FString& OutError);

	// AI가 작성해야 하는 current Project Saved PhysicsDraft path를 반환합니다.
	FString GetPhysicsProposalDraftPath() const;

	// Current Step 5에 load된 PhysicsDraft가 있는지 반환합니다.
	bool HasLoadedPhysicsProposalDraft() const { return bHasLoadedPhysicsProposalDraft; }

	// Current Step 5에 load된 PhysicsDraft를 반환합니다.
	const FCFBuilderPhysicsDraft& GetLoadedPhysicsProposalDraft() const { return LoadedPhysicsProposalDraft; }

	// USER-facing Step 5 Proposal/receipt 상태 요약을 만듭니다.
	FString BuildPhysicsProposalSummary() const;

	// Current Step 6의 existing R0 Gameplay Guidance 결과가 fresh projection에 존재하는지 반환합니다.
	bool HasGameplayGuidanceResult() const { return bHasGameplayGuidanceResult; }

	// Current Step 6의 existing R0 Gameplay Guidance fresh 결과를 반환합니다.
	const FCFBuilderGameplayGuidanceResult& GetGameplayGuidanceResult() const { return GameplayGuidanceResult; }

	// USER-facing Step 6 8영역 completeness/manual Socket/pending diff 요약을 만듭니다.
	FString BuildGameplayGuidanceSummary() const;

	// Current Step 7의 existing R0 Final Review 결과가 fresh projection에 존재하는지 반환합니다.
	bool HasFinalReviewResult() const { return bHasFinalReviewResult; }

	// Current Step 7의 existing R0 Final Review fresh 결과를 반환합니다.
	const FCFBuilderFinalReviewResult& GetFinalReviewResult() const { return FinalReviewResult; }

	// USER-facing Step 7 validation/drift/provenance/diff/apply readiness 요약을 만듭니다.
	FString BuildFinalReviewSummary() const;

	// Step 7의 current Final Review를 fresh mutation0로 다시 읽고 exact DefinitionApply proposal을 USER dialog 직전 prepared state로 보관합니다.
	bool PrepareFinalReviewApply(FCFBuilderFinalReviewResult& OutReview, FString& OutError);

	// 직전 exact Final Review proposal에 USER DefinitionApply approval을 붙여 existing R3 Apply lane을 실행합니다.
	bool ExecutePreparedFinalReviewApply(FCFBuilderFinalApplyResult& OutResult, FString& OutError);

	// Current Editor lifetime에서 마지막 Builder Final Apply가 발급한 guarded Undo token이 있는지 반환합니다.
	bool HasFinalReviewUndoToken() const { return bHasFinalReviewUndoToken; }

	// USER confirmation dialog에 표시할 current guarded Undo token을 반환합니다.
	const FCFBuilderUndoToken& GetFinalReviewUndoToken() const { return FinalReviewUndoToken; }

	// USER explicit DefinitionApply approval로 exact Builder-owned top transaction guarded Undo를 실행합니다.
	bool ExecuteFinalReviewUndo(FCFAuthoringOpResult& OutResult, FString& OutError);

	// Step 8이 사용하는 existing VB-P0-08 benchmark result JSON path를 반환합니다.
	FString GetDrivingBenchmarkResultPath() const;

	// Current saved Target exact path/hash를 binding한 VB-P0-08 runner process launch 정보를 만듭니다.
	bool PrepareDrivingBenchmarkLaunch(
		FString& OutExecutable,
		FString& OutArguments,
		FString& OutWorkingDirectory,
		FString& OutRunId,
		FString& OutError);

	// Current Step 8에 exact Target binding된 technical benchmark result가 있는지 반환합니다.
	bool HasDrivingBenchmarkResult() const { return bHasDrivingBenchmarkResult; }

	// Current Step 8 technical benchmark result를 반환합니다.
	const FCFVehicleBuilderBenchmarkResult& GetDrivingBenchmarkResult() const { return DrivingBenchmarkResult; }

	// USER-facing Step 8 technical metric / saved-state / USER Driving acceptance 요약을 만듭니다.
	FString BuildDrivingTestSummary() const;

	// Active PIE player VehiclePawn에 current selected saved VehicleData의 transient duplicate를 적용해 USER test-drive를 준비합니다.
	bool ApplySelectedVehicleToActivePIE(FString& OutError);

	// Current benchmark run과 exact Target hash를 USER Driving PASS local token으로 기록합니다.
	bool AcceptCurrentUserDriving(FString& OutError);

	// Current Step 8 USER Driving PASS token이 exact benchmark/Target에 일치하는지 반환합니다.
	bool HasCurrentUserDrivingAcceptance() const;

	// Current Editor session에서 선택 차량이 active PIE에 transient 적용돼 실제 USER test-drive 준비됐는지 반환합니다.
	bool IsUserTestDrivePreparedThisSession() const { return bUserTestDrivePreparedThisSession; }

	// Current page를 이전 Step으로 이동합니다.
	void MovePreviousStep();

	// Current Step이 workflow forward-progress 조건을 만족할 때 다음 Step으로 이동합니다. LayoutCapture Ready/NotCaptured는 Step 7 deferred Apply 조건에서만 허용합니다.
	bool MoveNextStep();

	// 현재 Step이 다음 단계로 진행 가능한지 단일 contract로 반환합니다.
	bool CanAdvanceFromCurrentStep() const;

	// USER가 상태를 확인할 목적으로 visible Step을 선택합니다. Locked/Unavailable은 이동하지 않습니다.
	bool SelectVisibleStep(int32 StepIndex);

	// 현재 Browser row입니다.
	const TArray<FCFVehicleListEntry>& GetVehicleEntries() const;

	// 현재 definition-driven Step projection입니다.
	const TArray<FCFVehicleBuilderStepView>& GetStepViews() const { return StepViews; }

	// Stable StepId로 현재 projection을 찾습니다. 순서가 바뀌어도 semantic lookup은 유지됩니다.
	const FCFVehicleBuilderStepView* FindStepView(ECFVehicleBuilderStepId StepId) const;

	// 현재 page index입니다.
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	// 현재 Step projection입니다.
	const FCFVehicleBuilderStepView& GetCurrentStep() const;

	// 선택된 차량이 있는지 반환합니다.
	bool HasSelection() const;

	// 현재 선택 row를 반환합니다.
	const FCFVehicleListEntry& GetSelectedEntry() const;

	// 현재 persistent Recipe를 반환합니다.
	UCFVehicleRecipeData* GetRecipe() const;

	// Step 2의 USER 선택 Chassis/Wheel Mesh를 existing typed AssetIntent Recipe-only lane으로 반영하고 Builder state를 fresh 재평가합니다.
	bool CommitMeshPreparation(const FCFVehicleAssetIntent& AssetIntent, FCFAuthoringOpResult& OutResult, FString& OutError);

	// 현재 선택이 사용하는 Chassis StaticMesh exact object path를 반환합니다.
	FSoftObjectPath GetCurrentChassisMeshPath() const;

	// Step 3에서 반드시 준비해야 하는 effective FL/FR/RL/RR Wheel Socket 이름 4개를 current Recipe binding 기준으로 반환합니다.
	TArray<FName> GetRequiredWheelSocketNames() const;

	// Step 3에서 선택적으로 준비할 current Recipe Hardpoint/Destroyed FX Socket 이름을 중복 없이 반환합니다.
	TArray<FName> GetOptionalSocketNames() const;

	// Fresh AssetSnapshot에서 exact Chassis Socket이 현재 존재하는지 read-only로 반환합니다.
	bool IsCurrentChassisSocketFound(FName SocketName) const;

	// 현재 선택이 아직 VehicleData가 없는 Mesh-only 후보인지 반환합니다.
	bool IsMeshOnlyCandidate() const;

private:
	// 단일 Step Definition 목록에서 현재 presentation projection을 생성합니다.
	void InitializeSteps();

	// 현재 Authoring VM selection에서 각 Step evaluator를 순서대로 실행합니다.
	void RebuildStepStates();

	// Stable StepId를 해당 Step evaluator로 dispatch합니다.
	void EvaluateStepById(ECFVehicleBuilderStepId StepId);

	// 차량/Reference 단계의 current state를 평가합니다.
	void EvaluateIdentityReferenceStep();

	// Mesh 준비 단계의 current state를 fresh AssetSnapshot에서 평가합니다.
	void EvaluateMeshPrepStep();

	// 소켓 준비 단계의 current state를 fresh Chassis socket truth에서 평가합니다.
	void EvaluateSocketGuideStep();

	// Layout Capture 단계의 current persisted/current socket equality를 평가합니다.
	void EvaluateLayoutCaptureStep();

	// Physics Proposal 단계의 현재 Shell 연결 상태를 평가합니다.
	void EvaluatePhysicsProposalStep();

	// Step 하나가 workflow prerequisite/forward navigation을 만족하는지 공통 판정합니다.
	bool IsStepSatisfiedForForwardProgress(const FCFVehicleBuilderStepView& Step) const;

	// Gameplay Setup 단계의 현재 Shell 연결 상태를 평가합니다.
	void EvaluateGameplaySetupStep();

	// Final Review 단계의 현재 Shell 연결 상태를 평가합니다.
	void EvaluateFinalReviewStep();

	// Driving Test 단계의 current benchmark / USER acceptance state를 평가합니다.
	void EvaluateDrivingTestStep();

	// Step 8이 요구하는 Step 7 current Complete prerequisite를 검사합니다.
	bool AreDrivingTestPrerequisitesComplete(FString& OutError) const;

	// Current saved Target object path와 exact current DefinitionHash를 Step 8 identity로 읽습니다.
	bool BuildDrivingTargetIdentity(
		FSoftObjectPath& OutTargetPath,
		FString& OutTargetDefinitionHash,
		FString& OutError) const;

	// Existing VB-P0-08 JSON을 읽고 current Target path/hash에 exact binding된 result만 current로 인정합니다.
	bool RefreshDrivingBenchmarkState(FString& OutError);

	// Current Recipe별 USER Driving PASS local token을 EditorPerProject settings에서 복원합니다.
	void LoadDrivingAcceptanceToken();

	// Current benchmark run + Target hash USER Driving PASS token을 EditorPerProject settings에 저장합니다.
	void SaveDrivingAcceptanceToken() const;

	// Selection 전환에서 transient USER Driving token state를 비웁니다.
	void ClearDrivingAcceptanceToken();

	// Current Recipe에 binding된 Evidence를 Asset Registry에서 찾아 exact binding/fingerprint를 재검사합니다.
	bool RefreshReferenceEvidenceState(FString& OutError);

	// Loaded Draft의 Evidence payload가 current existing Evidence와 semantic exact 동일한지 확인합니다.
	bool ValidateDraftAgainstCurrentEvidence(FString& OutError) const;

	// Current Recipe가 companion-only baseline preservation을 요구하는 Existing 차량인지, pristine core NewVehicle인지 파생합니다.
	ECFBuilderCompanionMode DeriveCompanionMode() const;

	// NewVehicle가 필수 private Profile을 아직 만들지 않아 current Resolver만 Blocked인 bootstrap read인지 판정합니다.
	bool CanUseNewVehicleProfileBootstrapRead() const;

	// Current Recipe name/RecipeId에서 deterministic missing companion asset identity 5종을 만듭니다.
	void FillCompanionAssetIdentities(FCFBuilderCompanionRequest& InOutRequest) const;

	// Current Recipe의 private 4 Profile이 모두 exact OwnerRecipeId인지 검사하고 missing/foreign 상태를 구분합니다.
	bool ReadPrivateProfileCompleteness(int32& OutMissingCount, FString& OutError) const;

	// Current Recipe에 exact owner로 binding된 private 4 Profile path/data를 complete typed payload로 읽습니다.
	bool BuildCurrentPrivateProfilePayload(FCFBuilderPrivateProfilePayload& OutPayload, FString& OutError) const;

	// Loaded PhysicsDraft를 current Recipe/Evidence/private Profile authority에 binding한 typed commit request로 만듭니다.
	bool BuildPhysicsProposalRequest(FCFBuilderProfileCommitRequest& OutRequest, FString& OutError) const;

	// Current private Profile payload와 persistent Builder receipt를 existing PreviewBuilderProfiles로 fresh 검증할 read-only request를 만듭니다.
	bool BuildCurrentPhysicsReceiptRequest(FCFBuilderProfileCommitRequest& OutRequest, FString& OutError) const;

	// Step 5가 요구하는 Step 1~4가 모두 current Complete인지 검사합니다.
	bool ArePhysicsProposalPrerequisitesComplete(FString& OutError) const;

	// Step 6이 요구하는 Step 5 current Complete prerequisite를 검사합니다.
	bool AreGameplaySetupPrerequisitesComplete(FString& OutError) const;

	// Current managed Recipe/Target을 existing Gameplay Guidance R0 request로 구성합니다.
	bool BuildGameplayGuidanceRequest(FCFBuilderGameplayGuidanceRequest& OutRequest, FString& OutError) const;

	// Step 7이 요구하는 Step 6 current Complete prerequisite를 검사합니다.
	bool AreFinalReviewPrerequisitesComplete(FString& OutError) const;

	// Current managed Recipe/Target/Evidence/receipt를 existing Final Review R0 request로 구성합니다.
	bool BuildFinalReviewRequest(FCFBuilderFinalReviewRequest& OutRequest, FString& OutError) const;

	// Selection/refresh에서 직전 Final Review DefinitionApply prepared approval을 폐기합니다.
	void ClearPreparedFinalReviewApply();

	// Selection이 바뀌거나 successful Undo 뒤 current lifetime guarded Undo token을 폐기합니다.
	void ClearFinalReviewUndoToken();

	// Current Evidence의 unresolved Block conflict가 존재하는지 반환합니다.
	bool HasBlockingReferenceConflict() const;

	// Current RecipeId에 저장된 USER Reference review token을 EditorPerProject settings에서 복원합니다.
	void LoadReferenceReviewToken();

	// Current USER Reference review token을 EditorPerProject settings에 저장합니다.
	void SaveReferenceReviewToken() const;

	// Selection 전환/새 draft load에서 이전 prepared Companion approval을 폐기합니다.
	void ClearPreparedResearchCompanion();

	// Selection/refresh/draft 변경에서 이전 prepared Evidence Refresh approval을 폐기합니다.
	void ClearPreparedEvidenceRefresh();

	// Selection/refresh/draft 변경에서 이전 prepared Physics Proposal approval을 폐기합니다.
	void ClearPreparedPhysicsProposal();

	// Stable StepId의 current presentation index를 찾습니다.
	int32 FindStepIndexById(ECFVehicleBuilderStepId StepId) const;

	// Stable StepId로 Step 하나의 상태/설명을 갱신합니다.
	void SetStep(
		ECFVehicleBuilderStepId StepId,
		ECFVehicleBuilderStepState State,
		const FString& Summary,
		const FString& Resolution,
		bool bProviderConnected);

	// 기존 Advanced Authoring Core를 재사용하는 내부 ViewModel입니다.
	TSharedPtr<FCFVehicleAuthoringVM> AuthoringViewModel = MakeShared<FCFVehicleAuthoringVM>();

	// 이번 selection/refresh에서 ResolveVehiclePreview read 자체가 성공해 current ResolveRequest.Assets를 Step 2~4 진단에 안전하게 사용할 수 있는지 여부입니다.
	// 전체 Preview/Apply 권한과는 별개이며 실제 read 실패 전에는 반드시 false로 초기화해 이전 snapshot 재사용을 막습니다.
	bool bHasCurrentResolveReadForStepDiagnostics = false;

	// 단일 Step Definition 목록 순서를 반영하는 presentation state입니다.
	TArray<FCFVehicleBuilderStepView> StepViews;

	// 현재 표시하는 Step index입니다.
	int32 CurrentStepIndex = 0;

	// Current Step 1에 load된 AI Research Draft transient payload입니다.
	FCFBuilderResearchDraft LoadedResearchDraft;

	// LoadedResearchDraft가 current Recipe/Target exact binding validation을 통과했는지 여부입니다.
	bool bHasLoadedResearchDraft = false;

	// Current Recipe에 exact binding된 Reference Evidence입니다.
	TWeakObjectPtr<UCFVehicleRefEvidence> CurrentReferenceEvidence;

	// CurrentReferenceEvidence의 stable object path입니다.
	FSoftObjectPath CurrentReferenceEvidencePath;

	// Duplicate/stale ownership 등 current Evidence discovery blocker입니다.
	FString ReferenceEvidenceStateError;

	// USER confirm dialog에 표시한 직전 exact Companion request입니다.
	FCFBuilderCompanionRequest PreparedResearchCompanionRequest;

	// USER confirm dialog에 표시한 직전 exact Companion preview입니다.
	FCFBuilderCompanionPreview PreparedResearchCompanionPreview;

	// 직전 Companion proposal이 USER approval 직전까지 유효한 transient prepared state인지 여부입니다.
	bool bHasPreparedResearchCompanion = false;

	// USER confirm dialog에 표시한 직전 exact Existing Evidence Refresh request입니다.
	FCFBuilderEvidenceRefreshRequest PreparedEvidenceRefreshRequest;

	// USER confirm dialog에 표시한 직전 exact Existing Evidence Refresh preview입니다.
	FCFBuilderEvidenceRefreshPreview PreparedEvidenceRefreshPreview;

	// 직전 Evidence Refresh proposal이 USER approval 직전까지 유효한 transient prepared state인지 여부입니다.
	bool bHasPreparedEvidenceRefresh = false;

	// Current Step 5에 load된 AI Physics Proposal transient payload입니다.
	FCFBuilderPhysicsDraft LoadedPhysicsProposalDraft;

	// LoadedPhysicsProposalDraft가 current Recipe/Target/accepted Evidence binding validation을 통과했는지 여부입니다.
	bool bHasLoadedPhysicsProposalDraft = false;

	// USER confirm dialog에 표시한 직전 exact private 4 Profile typed request입니다.
	FCFBuilderProfileCommitRequest PreparedPhysicsProposalRequest;

	// USER confirm dialog에 표시한 직전 exact private 4 Profile preview입니다.
	FCFBuilderProfileCommitPreview PreparedPhysicsProposalPreview;

	// 직전 Physics Proposal이 USER AuthoringWrite approval 직전까지 유효한 transient prepared state인지 여부입니다.
	bool bHasPreparedPhysicsProposal = false;

	// Current Step 6에서 existing ReadBuilderGameplayGuidance가 반환한 fresh R0 projection입니다.
	FCFBuilderGameplayGuidanceResult GameplayGuidanceResult;

	// GameplayGuidanceResult가 current selection/refresh에서 실제 R0 read된 결과인지 여부입니다.
	bool bHasGameplayGuidanceResult = false;

	// Current Step 7에서 existing ReadBuilderFinalReview가 반환한 fresh R0 projection입니다.
	FCFBuilderFinalReviewResult FinalReviewResult;

	// FinalReviewResult가 current selection/refresh에서 실제 R0 read된 결과인지 여부입니다.
	bool bHasFinalReviewResult = false;

	// USER confirm dialog에 표시한 직전 exact Final Review R0 request입니다.
	FCFBuilderFinalReviewRequest PreparedFinalReviewRequest;

	// USER confirm dialog에 표시한 직전 exact DefinitionApply proposal을 포함하는 Final Review result입니다.
	FCFBuilderFinalReviewResult PreparedFinalReviewResult;

	// 직전 Final Review proposal이 USER DefinitionApply approval 직전까지 유효한 transient prepared state인지 여부입니다.
	bool bHasPreparedFinalReviewApply = false;

	// 마지막 successful Builder Final Apply가 current Editor lifetime에서 발급한 guarded Undo token입니다.
	FCFBuilderUndoToken FinalReviewUndoToken;

	// FinalReviewUndoToken이 current lifetime의 explicit Undo 후보로 존재하는지 여부입니다.
	bool bHasFinalReviewUndoToken = false;

	// Current Step 8에서 current Target path/hash binding을 통과한 VB-P0-08 technical benchmark result입니다.
	FCFVehicleBuilderBenchmarkResult DrivingBenchmarkResult;

	// DrivingBenchmarkResult가 current Target exact identity에 일치하는지 여부입니다.
	bool bHasDrivingBenchmarkResult = false;

	// Benchmark result가 없거나 stale/invalid일 때 Step 8에 표시할 current diagnostic입니다.
	FString DrivingBenchmarkStateError;

	// Current Editor session에서 exact selected Target을 active PIE Pawn에 transient 적용해 USER test-drive가 준비됐는지 여부입니다.
	bool bUserTestDrivePreparedThisSession = false;

	// USER가 직접 주행 후 승인한 exact Recipe identity입니다.
	FGuid AcceptedDrivingRecipeId;

	// USER Driving PASS가 binding된 exact Target DefinitionHash입니다.
	FString AcceptedDrivingTargetDefinitionHash;

	// USER Driving PASS가 binding된 exact benchmark RunId입니다.
	FString AcceptedDrivingBenchmarkRunId;

	// USER가 승인한 exact Recipe identity입니다.
	FGuid AcceptedReferenceRecipeId;

	// USER가 승인한 exact Evidence identity입니다.
	FGuid AcceptedReferenceEvidenceId;

	// USER가 승인한 exact Evidence semantic fingerprint입니다.
	FString AcceptedReferenceEvidenceFingerprint;
};
