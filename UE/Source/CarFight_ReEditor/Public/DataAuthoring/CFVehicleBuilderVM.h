// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderVM.h
// Version: v1.35.0
// Date: 2026-09-05
// Description: Guided Vehicle Builder Shell + CF-FQ-047 Step 7 durable final commit / Step 8 fresh Target preflight seam입니다.
// Changelog:
// - v1.35.0: CF-FQ-047 최종 재감사 교정. guarded Undo 성공도 P0-07H durable handoff의 일부로 취급해 reverted Target/Recipe exact pair persistence를 요구하며 partial save failure는 dirty state를 보존한 채 fail-closed하는 계약으로 갱신.
// - v1.34.0: CF-FQ-047 최종감사 교정. P0-07H durable writer를 Product Save 없이 직접 회귀검증할 Automation-only package persistence seam을 추가하고 SavePackage true 뒤 clean/persisted 확인 실패를 SaveStateUnconfirmed로 구분할 exact save-state 결과를 추가.
// - v1.33.1: VBHAI-P0-07H fresh restart에서 Step 7이 Step 8 benchmark cache 복원보다 먼저 평가되어 post-driving receipt pending을 놓치는 순환을 막기 위해 benchmark-independent downstream receipt candidate helper를 추가.
// - v1.33.0: VBHAI-P0-07H에서 Final Review semantic truth와 exact Target/Recipe durable handoff를 합성하는 typed preflight, Apply/finalize/dirty-pair persistence orchestration, post-driving receipt pending phase를 추가.
// - v1.32.1: P0-07E Source 중간검수 교정. 과거 "새 writer 없음" migration 문구를 현재 exact Recipe single-package explicit Save writer 경계와 일치하도록 갱신.
// - v1.32.0: VBHAI-P0-07E에서 exact RunId benchmark progress sidecar reader와 persistent USER Driving receipt 기반 exact current Recipe explicit Save preflight/writer를 추가.
// - v1.31.1: VBHAI-P0-07B 재검수에서 stable preflight가 AuthoringVM cached DefinitionHash에 의존하지 않도록 live Target UObject에서 fresh Definition Snapshot identity를 만드는 private helper를 추가.
// - v1.31.0: VBHAI-P0-07B에서 cached benchmark + persisted Recipe/Target + AppliedState identity를 한 stable read-only preflight로 평가해 Step 8 버튼과 production Apply guard가 같은 blocker authority를 사용하도록 추가.
// - v1.30.0: Step 5 persistent receipt를 full commit transaction과 분리된 Physics compatibility로 읽어 Hardpoint/Mount-only structural drift에서는 Complete를 유지하는 read-only helper를 추가.
// - v1.29.0: CF-FQ-047 post-P0-06 중간검수 P1 교정. successful Recipe/Profile receipt mutation 뒤 post-commit refresh 실패를 operation failure로 뒤집지 않고 transient warning으로 분리.
// - v1.28.0: CF-FQ-047 P0-06에서 downstream Hardpoint/Mount 변경이 전체 resolved hash만 바꾼 경우 private 4 Profile payload 변경 0을 증명한 뒤 persistent Physics receipt만 explicit rebind하는 prepared path를 추가.
// - v1.27.0: CF-FQ-047 VBHAI-P0-02. Resolver AssetSnapshot과 분리된 Chassis Socket inventory read seam과 exact existing Standard Socket adoption typed wrapper를 추가.
// - v1.26.0: CF-FQ-046 VBIUX-P0-03에서 Step 5가 승인된 Builder-private 4 Profile의 실제 typed payload를 읽도록 ReadCurrentPrivateProfilePayload read-only seam을 추가.
// - v1.25.0: CF-FQ-046 VBIUX-P0-02에서 Step 3/4 USER presentation이 evaluator와 동일 current Socket/Target truth를 읽도록 ReadCurrentLayoutFacts read-only seam을 추가.
// - v1.24.0: P0-07 UAT에서 USER Driving PASS를 persistent Recipe receipt로 승격. same Target DefinitionHash에서는 benchmark RunId가 바뀌어도 PASS를 유지하고 Target identity/hash drift에서만 stale 처리하는 contract로 전환.
// - v1.23.0: P0-07 UAT 회귀 교정. local Reference review token이 유실돼도 exact persistent BuilderCommitReceipt가 current EvidenceId/Fingerprint를 증명하면 완료 차량의 Step 1/5 forward-progress를 복원하는 durable acceptance helper를 추가.
// - v1.22.1: CF-FQ-043 VMG-P0-06 Automation이 invalid Hardpoint identity의 Step 3 fail-closed projection을 production debug API 추가 없이 직접 재평가할 수 있도록 test-only friend seam을 추가.
// - v1.22.0: CF-FQ-043 VMG-P0-04 Standard 1:1 Mount commit API를 추가. new Mount_<LocationSlotId> collision fail-closed, existing MountProfileId/bExposedModule stable 보존, MountType/Size/Preset compatibility를 typed commit 전에 검증.
// - v1.21.1: VMG-P0-03 작성 중 exact Hardpoint Socket 미생성 상태만 Builder diagnostic read로 허용하는 narrow blocked-read gate를 추가. Resolver/R3 Apply blocker는 유지.
// - v1.21.0: CF-FQ-043 VMG-P0-03 Standard Hardpoint category→stable LocationSlotId/HP_* 생성, current Recipe+Target collision-aware numbering, Step 3 conditional non-Hardpoint Socket projection을 추가.
// - v1.20.0: CF-FQ-043 Recipe-owned HardpointPlanMode read/transaction, Guided creation explicit-plan binding과 dependency-safe typed remove Builder wrapper를 추가. Mode 변경은 prepared Final Apply를 폐기하고 no-save/no-target을 유지.
// - v1.19.0: Explicit New Vehicle 진입 계약에 이전 Authoring selection 해제를 포함해 Browser refresh가 old row를 재선택하지 못하도록 고정.
// - v1.18.0: VBCUX-P0-04 focused regression이 production behavior 변경 없이 private post-create adoption failure/no-rollback 계약을 직접 검증할 수 있도록 Automation 전용 friend seam을 추가.
// - v1.17.0: Vehicle ID 영문자/숫자/_ validation과 deterministic Definition/Recipe default identity builder를 추가해 Explicit New Vehicle/Mesh Candidate가 같은 naming owner를 사용하도록 연결.
// - v1.16.0: Blank/Arbitrary/Reused Chassis가 하나의 Guided create request를 사용하고, 생성 성공 뒤 exact Browser target adoption을 별도 결과로 검증하도록 추가.
// - v1.15.0: Stable 8-Step을 Browser refresh 전에 생성하고, selection-independent 신규 차량 진입 상태를 BuilderVM이 소유하도록 추가.
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
// - v1.35.0부터 Step 7 guarded Undo는 USER 승인 뒤 backend Undo가 실제 성공하면 reverted Target→Recipe exact pair를 dirty-needed 범위로 즉시 저장합니다. 저장 실패 시 Undo 자체를 자동 rollback/retry하지 않으며 current dirty/partial persisted state를 그대로 보존해 후속 Step 7 복구가 가능하게 합니다. Save All/StaticMesh/Profile/Evidence/Catalog 권한은 추가하지 않습니다.
// - v1.34.0 test persistence seam은 WITH_DEV_AUTOMATION_TESTS에서만 활성화되며 Shipping/production package persistence authority를 변경하지 않습니다. Production은 계속 exact /Game package + UPackage::SavePackage + clean/disk 확인을 사용합니다.
// - v1.33.1 fresh-restart 복원은 persistent USER Driving receipt + semantic/Target/AppliedState candidate가 모두 exact일 때만 existing benchmark result JSON을 transient VM cache로 다시 검증합니다. Asset mutation/Save/benchmark 실행 권한은 추가하지 않습니다.
// - v1.33.0 Step 7은 USER 확인 뒤 exact current Target/Recipe package만 dirty-needed 범위로 저장하며 둘 다 필요할 때 Target→Recipe 순서를 사용합니다. Save All/StaticMesh/Profile/Evidence/Catalog writer authority는 추가하지 않습니다.
// - v1.32.1의 새 writer는 `SaveCurrentRecipeAfterDrivingAcceptance()` 하나이며 persistent USER Driving receipt가 fresh current Target에 exact binding된 current Recipe package 한 개만 USER click 뒤 저장합니다. 기존 Authoring facade의 Profile/VehicleData/StaticMesh/Catalog writer authority는 변경하지 않습니다.
// - v1.31.1 fresh Target identity는 live VehicleData를 immutable Definition Snapshot으로 읽기만 하며 Authoring selection refresh, JSON reread, Asset mutation/Save를 수행하지 않습니다.
// - v1.31.0 Driving Apply preflight는 RefreshDrivingBenchmarkState가 검증해 둔 cached result만 읽으며 Slate enable 평가 중 JSON을 다시 읽지 않습니다. PIE lifecycle/Save/Product mutation 권한은 추가하지 않습니다.
// - v1.30.0 Physics compatibility는 current receipt/provenance/pending diff를 읽기만 하며 receipt 자동 재작성/Save 권한을 추가하지 않습니다. Step 8 DefinitionHash 계약은 변경하지 않습니다.
// - v1.29.0 GetLastPostCommitRefreshWarning은 Editor-only transient diagnostic이며 persistent writer/Save 권한을 추가하지 않습니다. 기존 mutation signature와 authority는 유지합니다.
// - v1.26.0 ReadCurrentPrivateProfilePayload는 기존 Recipe binding의 Builder-private Profile Data를 읽기만 하며 Profile/Recipe/VehicleData mutation 권한을 추가하지 않습니다.
// - v1.25.0 ReadCurrentLayoutFacts는 current evaluator truth의 transient read-only projection이며 Layout Apply/Save/Socket mutation 권한을 추가하지 않습니다.
// - v1.19.0부터 BeginNewVehicleEntry는 이전 Authoring selection을 해제하되 Browser cache와 persistent Asset은 유지합니다. 신규 차량 입력은 refresh 뒤에도 selection-independent 상태를 유지합니다.
// - v1.18.0은 WITH_DEV_AUTOMATION_TESTS에서만 private adoption helper 접근을 허용하는 test seam이며 Runtime/Slate public API와 shipping behavior는 변경하지 않습니다.
// - v1.17.0부터 일반 Guided 신규 차량 naming은 Vehicle ID 한 칸이 기본 owner입니다. 기본 identity는 /Game/CarFight/Data/Authoring/DA_Vehicle_<Id>, DA_Recipe_<Id>로 deterministic 제안하며 collision/path/type 최종 판정은 기존 PreviewVehicleRecords/ValidateNewAssetIdentity가 계속 authority입니다. Advanced override는 UI에서만 별도 노출합니다.
// - v1.16.0부터 Blank/Arbitrary/Mesh-only Guided creation은 동일 Builder request helper를 사용하며 모두 VehicleSpecificRequired입니다. 생성 직후 Chassis는 Recipe AssetIntent에만 기록하고 VehicleData Apply/Save는 하지 않습니다. record creation 성공과 Builder adoption 실패는 별도 결과로 보고합니다.
// - v1.15.0부터 Step definition은 BuilderVM 생성 직후 존재하며 Asset Registry refresh 성공 여부와 분리됩니다. 신규 차량 진입은 transient state만 바꾸며 Asset 생성/Save/Apply를 수행하지 않습니다.
// - Recipe-only AuthoringRevision은 NewVehicle lifecycle을 소모하지 않으며, private Profile bootstrap은 기존 prospective Resolver/Validator를 반드시 통과해야 합니다.
// - P0-07E 이전 Authoring mutation authority는 기존 Authoring facade가 계속 소유하며, P0-07E는 Step 8 explicit Recipe single-package Save authority만 별도로 추가합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleBuilderTypes.h"

struct FCFBuilderChassisSocketInventory;
struct FCFBuilderPhysicsReceiptCompatibilityResult;
class UPackage;

/** P0-09 USER Acceptance용 Guided Builder Shell transient ViewModel입니다. */
class CARFIGHT_REEDITOR_API FCFVehicleBuilderVM
{
public:
	// Browser refresh 전에도 Stable Step 8개가 존재하도록 transient Builder 상태를 초기화합니다.
	FCFVehicleBuilderVM();

	// Asset Registry 기반 차량/메시 후보 목록을 기존 Authoring VM으로 새로 읽습니다.
	bool RefreshVehicles(FString& OutError);

	// 목록 row 하나를 current Builder target으로 선택하고 fresh authoring context를 읽습니다.
	bool SelectVehicle(const FCFVehicleListEntry& Entry, FString& OutError);

	// 현재 선택과 authoritative truth를 다시 읽고 Step 상태를 재평가합니다.
	bool RefreshCurrentState(FString& OutError);

	// 기존 Authoring selection을 해제하고 Browser row와 독립적인 신규 차량 제작 진입 상태를 시작합니다.
	void BeginNewVehicleEntry();

	// 현재 Builder가 명시적 신규 차량 제작 진입 상태인지 반환합니다.
	bool IsNewVehicleEntryActive() const;

	// 명시적 신규 차량을 Chassis 없는 Blank Start로 전환합니다.
	void SetNewVehicleBlankStart();

	// 명시적 신규 차량의 optional Chassis StaticMesh exact object path를 설정합니다. Invalid/empty path는 Blank Start로 되돌립니다.
	void SetNewVehicleChassisMeshPath(const FSoftObjectPath& ChassisMeshPath);

	// Explicit New Vehicle/Mesh Candidate Quick Start가 공유하는 Vehicle ID transient 입력을 저장하고 Unreal Asset-safe 규칙을 검증합니다.
	bool SetVehicleCreationId(const FString& VehicleId, FString& OutError);

	// 현재 Guided 신규 차량 creation Vehicle ID transient 입력을 반환합니다.
	const FString& GetVehicleCreationId() const;

	// Vehicle ID 하나에서 canonical default Definition/Recipe package/object identity 네 값을 deterministic하게 만듭니다.
	bool BuildDefaultVehicleRecordIdentity(
		const FString& VehicleId,
		FString& OutDefinitionPackageName,
		FString& OutDefinitionAssetName,
		FString& OutRecipePackageName,
		FString& OutRecipeAssetName,
		FString& OutError) const;

	// 명시적 신규 차량이 현재 사용할 optional Chassis StaticMesh exact object path를 반환합니다.
	const FSoftObjectPath& GetNewVehicleChassisMeshPath() const;

	// Builder-owned 신규 차량 state로 기존 two-record 생성 proposal을 mutation0 준비합니다.
	bool PrepareNewVehicleRecordCreate(
		const FString& DefinitionPackageName,
		const FString& DefinitionAssetName,
		const FString& RecipePackageName,
		const FString& RecipeAssetName,
		FCFVehicleRecordCreatePreview& OutPreview,
		FString& OutError);

	// 직전 exact 신규 차량 proposal을 commit하고 record creation과 post-create exact Builder adoption 결과를 분리해 반환합니다.
	bool ExecutePreparedNewVehicleRecordCreate(
		FCFVehicleRecordCreateResult& OutResult,
		bool& bOutBuilderAdopted,
		FString& OutAdoptionError,
		FString& OutError);

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

	// Current persistent Physics receipt와 private 4 Profile이 exact 유지되고 downstream resolved hash만 달라진 경우 receipt-only rebind preview를 준비합니다.
	bool PrepareCurrentPhysicsReceiptRefresh(FCFBuilderProfileCommitPreview& OutPreview, FString& OutError);

	// 직전 exact Physics Proposal preview에 USER AuthoringWrite approval을 붙여 private 4 Profile + Builder receipt를 commit합니다.
	bool ExecutePreparedPhysicsProposal(FCFAuthoringOpResult& OutResult, FString& OutError);

	// AI가 작성해야 하는 current Project Saved PhysicsDraft path를 반환합니다.
	FString GetPhysicsProposalDraftPath() const;

	// Current Step 5에 load된 PhysicsDraft가 있는지 반환합니다.
	bool HasLoadedPhysicsProposalDraft() const { return bHasLoadedPhysicsProposalDraft; }

	// Current Step 5에 load된 PhysicsDraft를 반환합니다.
	const FCFBuilderPhysicsDraft& GetLoadedPhysicsProposalDraft() const { return LoadedPhysicsProposalDraft; }

	// Current Recipe에 binding된 승인 완료 Builder-private 4 Profile의 typed payload를 read-only로 반환합니다.
	bool ReadCurrentPrivateProfilePayload(FCFBuilderPrivateProfilePayload& OutPayload, FString& OutError) const;

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

	// USER-facing Step 7 validation/drift/provenance/diff/durable handoff readiness 요약을 만듭니다.
	FString BuildFinalReviewSummary() const;

	// Current Step 7이 마지막으로 평가한 semantic + durable final commit state를 반환합니다.
	const FCFBuilderFinalCommitPreflight& GetFinalCommitPreflight() const { return FinalCommitPreflight; }

	// Step 7의 current Final Review와 exact Target/Recipe durable action을 USER dialog 직전 fresh 평가해 prepared state로 보관합니다.
	bool PrepareFinalReviewCommit(
		FCFBuilderFinalCommitPreflight& OutPreflight,
		FCFBuilderFinalReviewResult& OutReview,
		FString& OutError);

	// USER가 확인한 exact final-commit scope를 fresh 재검증한 뒤 Apply/finalize/Target→Recipe persistence를 실행합니다.
	bool ExecutePreparedFinalReviewCommit(FCFBuilderFinalCommitResult& OutResult, FString& OutError);

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

	// Step 8 exact RunId coarse progress sidecar의 canonical JSON path를 반환합니다.
	FString GetDrivingBenchmarkProgressPath() const;

	// Canonical progress sidecar를 읽어 typed coarse progress로 변환합니다. RunId current-match 판정은 Tab process owner가 수행합니다.
	bool ReadDrivingBenchmarkProgress(FCFVehicleBuilderBenchmarkProgress& OutProgress, FString& OutError) const;

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

	// Step 8 PIE Apply 버튼과 production Apply가 공유하는 current saved-state/benchmark stable preflight를 mutation 없이 읽습니다.
	FCFVehicleDrivingApplyPreflight ReadDrivingApplyPreflight() const;

	// Active PIE player VehiclePawn에 current selected saved VehicleData의 transient duplicate를 적용해 USER test-drive를 준비합니다.
	bool ApplySelectedVehicleToActivePIE(FString& OutError);

	// Current saved Target Definition을 USER Driving PASS persistent Recipe receipt + legacy local token으로 기록합니다. benchmark RunId는 진단용으로만 보존합니다.
	bool AcceptCurrentUserDriving(FString& OutError);

	// Current Step 8 USER Driving PASS가 persistent Target Definition receipt 또는 legacy local token으로 current Target에 일치하는지 반환합니다.
	bool HasCurrentUserDrivingAcceptance() const;

	// Persistent USER Driving receipt가 fresh current Target에 exact binding된 Recipe 하나를 explicit Save할 수 있는지 mutation 없이 읽습니다. Legacy local token은 Save authority가 아닙니다.
	FCFVehicleRecipeSavePreflight ReadCurrentRecipeSavePreflight() const;

	// USER click 뒤 exact current Recipe package 하나만 저장하고 SavePackage/dirty/post-refresh outcome을 typed result로 반환합니다.
	FCFVehicleRecipeSaveResult SaveCurrentRecipeAfterDrivingAcceptance();

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

	// Current Recipe의 persistent Builder Hardpoint 계획 mode를 반환합니다. Recipe가 없으면 LegacyCompatible을 반환합니다.
	ECFBuilderHardpointPlanMode GetHardpointPlanMode() const;

	// USER가 선택한 Guided Hardpoint 계획 mode를 Builder-owned Recipe transaction으로 기록하고 stale workflow approval을 폐기합니다.
	bool CommitHardpointPlanMode(ECFBuilderHardpointPlanMode PlanMode, FCFAuthoringOpResult& OutResult, FString& OutError);

	// Exact MountProfile stable identity 하나를 existing R1 typed remove lane으로 제거하고 Builder state를 fresh 재평가합니다.
	bool RemoveMountIntent(FName MountProfileId, FCFAuthoringOpResult& OutResult, FString& OutError);

	// Standard Hardpoint 하나의 1:1 Mount rule을 complete draft로 검증하고 stable MountProfileId를 보존/생성해 existing typed R1 lane으로 commit합니다.
	bool CommitStandardMountIntent(
		FName LocationSlotId,
		ECFVehicleMountType MountType,
		ECFVehicleWeaponSize SizeLimit,
		const FSoftObjectPath& DefaultEquipmentPresetPath,
		FCFMountIntent& OutIntent,
		FCFAuthoringOpResult& OutResult,
		FString& OutError);

	// Exact Hardpoint stable identity 하나를 dependency-safe existing R1 typed remove lane으로 제거하고 Builder state를 fresh 재평가합니다.
	bool RemoveHardpointIntent(FName LocationSlotId, FCFAuthoringOpResult& OutResult, FString& OutError);

	// Standard category 하나를 stable <Category>_<NN> / HP_<LocationSlotId> Hardpoint intent로 생성해 existing typed R1 lane으로 commit합니다.
	bool AddStandardHardpoint(FName LocationCategory, FCFHardpointIntent& OutIntent, FCFAuthoringOpResult& OutResult, FString& OutError);

	// Step 3 Standard UI가 노출하는 physical Hardpoint category 목록을 deterministic 순서로 반환합니다.
	TArray<FName> GetStandardHardpointCategories() const;

	// Step 2의 USER 선택 Chassis/Wheel Mesh를 existing typed AssetIntent Recipe-only lane으로 반영하고 Builder state를 fresh 재평가합니다.
	bool CommitMeshPreparation(const FCFVehicleAssetIntent& AssetIntent, FCFAuthoringOpResult& OutResult, FString& OutError);

	// 현재 선택이 사용하는 Chassis StaticMesh exact object path를 반환합니다.
	FSoftObjectPath GetCurrentChassisMeshPath() const;

	// Step 3에서 반드시 준비해야 하는 effective FL/FR/RL/RR Wheel Socket 이름 4개를 current Recipe binding 기준으로 반환합니다.
	TArray<FName> GetRequiredWheelSocketNames() const;

	// Step 3에서 선택적으로 준비할 current Recipe Hardpoint/Destroyed FX Socket 이름을 중복 없이 반환합니다.
	TArray<FName> GetOptionalSocketNames() const;

	// Step 3 Hardpoint 표와 중복되지 않는 Destroyed FX Socket 이름만 반환합니다.
	TArray<FName> GetConditionalNonHardpointSocketNames() const;

	// Fresh AssetSnapshot에서 exact Chassis Socket이 현재 존재하는지 read-only로 반환합니다.
	bool IsCurrentChassisSocketFound(FName SocketName) const;

	// Step 3/4 USER presentation이 evaluator와 동일 current Socket/Target truth를 읽도록 차량 배치 사실을 반환합니다.
	bool ReadCurrentLayoutFacts(FCFVehicleBuilderLayoutFacts& OutFacts, FString& OutError) const;

	// Resolver AssetSnapshot과 분리된 Editor-only current Chassis Socket inventory를 read-only로 반환합니다.
	bool ReadCurrentChassisSocketInventory(FCFBuilderChassisSocketInventory& OutInventory, FString& OutError) const;

	// UseHardpoints 상태에서 canonical existing HP_* Socket 하나를 exact parsed stable identity로 Recipe HardpointIntent에 채택합니다. StaticMesh/Target/Save mutation은 없습니다.
	bool AdoptExistingStandardHardpointSocket(
		FName SocketName,
		FCFHardpointIntent& OutIntent,
		FCFAuthoringOpResult& OutResult,
		FString& OutError);

	// 마지막 successful Builder mutation 뒤 상태 재조회만 실패한 경우의 non-fatal warning을 반환합니다.
	const FString& GetLastPostCommitRefreshWarning() const { return LastPostCommitRefreshWarning; }

	// 현재 선택이 아직 VehicleData가 없는 Mesh-only 후보인지 반환합니다.
	bool IsMeshOnlyCandidate() const;

private:
#if WITH_DEV_AUTOMATION_TESTS
	// VBCUX-P0-04 Automation이 post-create adoption failure partial-success 계약을 production API 공개 없이 직접 검증할 수 있도록 허용합니다.
	friend class FCFVBCUXP004FocusedRegressionTest;
	// VMG-P0-02 Automation이 prepared Final Apply invalidation을 production debug API 추가 없이 검증할 수 있도록 허용합니다.
	friend class FCFVMGP002RecipeStateTest;
	// VMG-P0-06 Automation이 invalid Hardpoint identity 상태의 Step 3 evaluator를 last valid AssetSnapshot 위에서 직접 검증할 수 있도록 허용합니다.
	friend class FCFVMGP006ContractMatrixTest;
	// Step 8 Automation이 production AcceptCurrentUserDriving의 실제 persistent receipt write를 PIE runtime 없이 검증할 수 있도록 test-only preparation flag에 접근합니다.
	friend class FCFVehicleBuilderStep8DrivingTest;
	// CF-FQ-047 중간검수 Automation이 successful mutation 뒤 refresh warning outcome을 production write API 추가 없이 검증하도록 허용합니다.
	friend class FCFVBHAIPostCommitRefreshOutcomeTest;
	// CF-FQ-047 P0-07H 최종감사 Automation이 exact /Game identity를 유지한 채 Product disk write 없이 durable final commit persistence/failure taxonomy를 직접 검증하도록 허용합니다.
	friend class FCFVBHAIP007HDurableCommitTest;
#endif

	// Cached benchmark result가 current Target identity에 대해 마지막 refresh에서 어떤 검증 상태였는지 표현합니다.
	enum class EDrivingBenchmarkValidationState : uint8
	{
		Unavailable,
		Current,
		StaleOrInvalid
	};

	// Driving Apply stable blocker evaluator가 소비하는 current read-only facts입니다.
	struct FDrivingApplyPreflightFacts
	{
		// Current Recipe를 읽을 수 있는지 여부입니다.
		bool bRecipeAvailable = false;

		// Current Target VehicleData를 읽을 수 있는지 여부입니다.
		bool bTargetAvailable = false;

		// Current Recipe package에 저장되지 않은 변경이 있는지 여부입니다.
		bool bRecipeDirty = false;

		// Current Target package에 저장되지 않은 변경이 있는지 여부입니다.
		bool bTargetDirty = false;

		// Current Recipe package가 disk에 실제 존재하는지 여부입니다.
		bool bRecipePersisted = false;

		// Current Target package가 disk에 실제 존재하는지 여부입니다.
		bool bTargetPersisted = false;

		// Current Target path/hash identity를 fresh read하는 데 성공했는지 여부입니다.
		bool bStateReadSucceeded = false;

		// Recipe AppliedState가 fresh current Target DefinitionHash와 exact 일치하는지 여부입니다.
		bool bAppliedStateCurrent = false;

		// 마지막 benchmark refresh가 분류한 cached result validation 상태입니다.
		EDrivingBenchmarkValidationState BenchmarkState = EDrivingBenchmarkValidationState::Unavailable;

		// Cached benchmark path/hash가 fresh current Target identity와 exact 일치하는지 여부입니다.
		bool bBenchmarkIdentityCurrent = false;

		// Raw path/hash/error를 포함할 수 있는 내부 진단 정보입니다.
		FString Diagnostic;
	};

	// Stable facts를 deterministic blocker 우선순위의 Driving Apply preflight로 변환합니다.
	static FCFVehicleDrivingApplyPreflight EvaluateDrivingApplyPreflight(const FDrivingApplyPreflightFacts& Facts);

	// 신규 차량 제작이 어느 시작 방식을 사용할지 표현하는 transient 모드입니다.
	enum class ENewVehicleStartMode : uint8
	{
		Blank,
		ChassisMesh
	};

	// 신규 차량 제작 입력의 canonical transient state입니다. P0-01은 진입 상태만 사용하고 나머지 필드는 후속 생성 UX가 이어서 채웁니다.
	struct FNewVehicleEntryState
	{
		// 명시적 신규 차량 제작 진입이 활성 상태인지 여부입니다.
		bool bActive = false;

		// 빈 차량 또는 Chassis Mesh 시작 방식입니다.
		ENewVehicleStartMode StartMode = ENewVehicleStartMode::Blank;

		// 후속 Vehicle ID naming 입력을 보관할 canonical 문자열입니다.
		FString VehicleId;

		// Chassis Mesh 시작 방식에서 사용할 optional exact object path입니다.
		FSoftObjectPath OptionalChassisMeshPath;
	};

	// Vehicle ID가 기본 naming에 사용할 영문자/숫자/_ 전용 Unreal Asset-safe identifier인지 검사합니다.
	bool ValidateVehicleCreationId(const FString& VehicleId, FString& OutError) const;

	// 이미 성공한 persistent mutation 뒤 refresh 실패를 operation failure로 뒤집지 않고 warning으로 확정합니다.
	static bool FinalizeSuccessfulMutationRefresh(
		const TArray<FString>& RefreshFailures,
		FString& OutError,
		FString& OutWarning);

	// 신규 차량 제작 transient state를 기본 비활성 상태로 되돌립니다.
	void ResetNewVehicleEntryState();

	// Blank/Arbitrary/Mesh-only Quick Start가 공유하는 Guided two-record creation request를 기존 Authoring facade에 준비합니다.
	bool PrepareGuidedVehicleRecordCreate(
		const FString& DefinitionPackageName,
		const FString& DefinitionAssetName,
		const FString& RecipePackageName,
		const FString& RecipeAssetName,
		const FSoftObjectPath& OptionalChassisMeshPath,
		FCFVehicleRecordCreatePreview& OutPreview,
		FString& OutError);

	// 생성된 exact Definition/Recipe를 Browser fresh row로 다시 찾아 Builder current target으로 adoption합니다.
	bool AdoptCreatedVehicleRecords(
		const FCFVehicleRecordCreateResult& CreateResult,
		FString& OutError);

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

	// Driving Apply stable guard용으로 live Target UObject 자체에서 fresh Definition Snapshot path/hash identity를 만듭니다.
	static bool BuildFreshDrivingTargetIdentity(
		const UCFVehicleData& TargetVehicleData,
		FSoftObjectPath& OutTargetPath,
		FString& OutTargetDefinitionHash,
		FString& OutError);

	// Existing VB-P0-08 JSON을 읽고 current Target path/hash에 exact binding된 result만 current로 인정합니다.
	bool RefreshDrivingBenchmarkState(FString& OutError);

	// Legacy 호환용 Current Recipe별 USER Driving PASS local token을 EditorPerProject settings에서 복원합니다. 새 durable authority는 Recipe receipt입니다.
	void LoadDrivingAcceptanceToken();

	// Legacy 호환용 current benchmark run + Target hash USER Driving PASS token을 EditorPerProject settings에 저장합니다.
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

	// UseHardpoints 작성 중 exact Socket 미생성으로 HardpointSocketMissing 계열 blocker만 존재하는 diagnostic read인지 판정합니다.
	bool CanUseHardpointSocketDraftRead() const;

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

	// Current receipt/fresh preview와 pending structural diff를 single authority로 읽어 Step 5 Physics compatibility를 반환합니다.
	FCFBuilderPhysicsReceiptCompatibilityResult EvaluateCurrentPhysicsReceiptCompatibility(
		const FCFBuilderProfileCommitRequest& ReceiptRequest,
		const FCFBuilderProfileCommitPreview& ReceiptPreview) const;

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

	// Fresh Final Review identity + live Target snapshot에서 exact Recipe/Target durable handoff facts를 읽습니다.
	FCFBuilderSavedHandoffPreflight ReadSavedHandoffPreflight(const FCFBuilderFinalReviewResult& FreshReview) const;

	// Fresh semantic Final Review와 Saved Handoff를 합성해 Step 7 exact action을 파생합니다.
	FCFBuilderFinalCommitPreflight EvaluateFinalCommitPreflight(const FCFBuilderFinalReviewResult& FreshReview) const;

	// Benchmark cache와 무관하게 persistent USER Driving receipt-only downstream dirty 후보가 exact인지 먼저 판정합니다.
	bool IsPostDrivingReceiptCandidate(
		const FCFBuilderFinalReviewResult& FreshReview,
		const FCFBuilderSavedHandoffPreflight& SavedHandoff) const;

	// Step 8 USER Driving receipt write만 남은 downstream Recipe dirty가 Step 7을 retroactive incomplete로 만들지 않는지 판정합니다.
	bool IsPostDrivingReceiptSavePending(
		const FCFBuilderFinalReviewResult& FreshReview,
		const FCFBuilderSavedHandoffPreflight& SavedHandoff) const;

	// USER dialog approval을 exact action/Recipe/Target/fresh provenance/package state에 binding한 scope hash를 생성합니다.
	static FString BuildFinalCommitApprovalScope(const FCFBuilderFinalCommitPreflight& Preflight);

	// Exact package가 persisted state인지 production disk 또는 Automation override에서 확인합니다.
	bool DoesBuilderPackageExist(const FString& PackageName) const;

	// Exact /Game package 하나만 저장하고 SavePackage success + clean/persisted를 확인합니다. Policy/ordering은 caller가 소유합니다.
	bool SaveBuilderPackage(
		UPackage* Package,
		UObject* Asset,
		bool& bOutSaved,
		bool& bOutSaveStateUnconfirmed,
		FString& OutError);

	// Selection/refresh에서 직전 Final Review DefinitionApply/final-commit prepared approval을 폐기합니다.
	void ClearPreparedFinalReviewApply();

	// Selection이 바뀌거나 successful Undo 뒤 current lifetime guarded Undo token을 폐기합니다.
	void ClearFinalReviewUndoToken();

	// Current Evidence의 unresolved Block conflict가 존재하는지 반환합니다.
	bool HasBlockingReferenceConflict() const;

	// Current local USER review token 또는 exact persistent Builder receipt가 current Reference Evidence 승인 provenance를 증명하는지 반환합니다.
	bool IsCurrentReferenceAcceptedForProgress() const;

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

	// selection과 독립적으로 Builder가 소유하는 신규 차량 제작 transient state입니다.
	FNewVehicleEntryState NewVehicleEntryState;

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

	// Current Step 7 semantic review와 exact package state를 합성한 final commit projection입니다.
	FCFBuilderFinalCommitPreflight FinalCommitPreflight;

	// FinalReviewResult가 current selection/refresh에서 실제 R0 read된 결과인지 여부입니다.
	bool bHasFinalReviewResult = false;

	// USER confirm dialog에 표시한 직전 exact Final Review R0 request입니다.
	FCFBuilderFinalReviewRequest PreparedFinalReviewRequest;

	// USER confirm dialog에 표시한 직전 exact DefinitionApply proposal을 포함하는 Final Review result입니다.
	FCFBuilderFinalReviewResult PreparedFinalReviewResult;

	// 직전 Final Review proposal이 USER DefinitionApply approval 직전까지 유효한 transient prepared state인지 여부입니다.
	bool bHasPreparedFinalReviewApply = false;

	// USER dialog가 확인한 exact durable final commit action/scope입니다.
	FCFBuilderFinalCommitPreflight PreparedFinalCommitPreflight;

	// 직전 durable final commit approval이 mutation/save 직전 fresh revalidation 후보인지 여부입니다.
	bool bHasPreparedFinalCommit = false;

	// 마지막 successful Builder Final Apply가 current Editor lifetime에서 발급한 guarded Undo token입니다.
	FCFBuilderUndoToken FinalReviewUndoToken;

	// FinalReviewUndoToken이 current lifetime의 explicit Undo 후보로 존재하는지 여부입니다.
	bool bHasFinalReviewUndoToken = false;

	// Current Step 8에서 current Target path/hash binding을 통과한 VB-P0-08 technical benchmark result입니다.
	FCFVehicleBuilderBenchmarkResult DrivingBenchmarkResult;

	// DrivingBenchmarkResult가 current Target exact identity에 일치하는지 여부입니다.
	bool bHasDrivingBenchmarkResult = false;

	// RefreshDrivingBenchmarkState가 마지막으로 검증한 cached benchmark 상태입니다. ReadDrivingApplyPreflight는 JSON을 다시 읽지 않고 이 값만 소비합니다.
	EDrivingBenchmarkValidationState DrivingBenchmarkValidationState = EDrivingBenchmarkValidationState::Unavailable;

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

	// 마지막 successful persistent Builder mutation 뒤 발생한 non-fatal 화면/참조 재조회 warning입니다.
	FString LastPostCommitRefreshWarning;

#if WITH_DEV_AUTOMATION_TESTS
	// P0-07H Automation이 /Game identity를 유지하면서 실제 disk 존재 여부 대신 isolated persisted state를 공급하는 callback입니다.
	TFunction<bool(const FString&)> TestBuilderPackageExistsOverride;

	// P0-07H Automation이 Product Asset write 없이 production SaveBuilderPackage의 success/failure/clean 확인을 검증하는 callback입니다.
	TFunction<bool(UPackage*, UObject*, FString&)> TestBuilderPackageSaveOverride;
#endif
};
