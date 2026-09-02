// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTab.h
// Version: v1.11.0
// Date: 2026-08-31
// Description: CF-FQ-040 Guided Vehicle Builder의 Editor Slate Shell입니다.
// Changelog:
// - v1.11.0: Step 1 Existing Reference Evidence complete replacement R1의 별도 검토 버튼/handler를 추가해 Companion 생성과 Evidence 갱신 UX를 분리.
// - v1.10.0: Step 2 picker presentation 재생성 host/helper를 추가해 차량 selection 전환 시 이전 ObjectPicker 표시 캐시가 current Recipe로 교체되도록 보강.
// - v1.9.0: E2E에서 발견된 Step 2 Wheel Mesh 지정 UX 공백을 StaticMesh object picker + explicit Recipe-only 반영 UI로 교정.
// - v1.8.0: USER 피드백에 따라 작업 대상 row를 identity 기반 고정 accent 색+관리 상태 배지로 구분하고 Step 3에 필수·선택 Socket 이름/복사/차체 메시 열기 작업 패널을 추가.
// - v1.7.0: VB-P0-09 Step 8 existing VB-P0-08 benchmark를 non-blocking child process로 실행/회수하고 active PIE transient test-drive + exact USER Driving PASS UX를 연결.
// - v1.6.0: VB-P0-09 Step 7 existing ReadBuilderFinalReview R0 전체 Diff review, explicit DefinitionApply, exact guarded Undo UX를 Guided Shell에 연결.
// - v1.5.0: VB-P0-09 Step 6 existing ReadBuilderGameplayGuidance R0 결과를 8영역 completeness/USER Socket guidance/pending diff 요약으로 Guided Shell에 노출.
// - v1.4.0: VB-P0-09 Step 5 AI PhysicsDraft load, private 4 Profile mutation0 review→explicit USER R1 commit, persistent receipt 상태 요약 UI를 추가.
// - v1.3.0: VB-P0-09 Step 1 AI ResearchDraft load, Companion mutation0 review→explicit USER R2 commit, Reference summary와 fingerprint review token UI를 추가.
// - v1.2.0: navigation/Step-specific visibility가 fixed count/index 대신 ViewModel projection과 stable StepId를 사용하도록 전환.
// - v1.1.0: Step 1 Mesh-only 후보에 기존 safe VehicleData+Recipe Preview→명시 승인 생성 UI를 연결.
// - v1.0.0: 차량/메시 후보 목록, 고정 8-step navigation, current-step 단일 content, Back/Refresh/Next를 추가.
// Migration:
// - 기존 Vehicle Authoring Advanced Workspace를 대체하지 않고 별도 Guided entry로 병행합니다.

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;
struct FCFVehicleListEntry;
class FCFVehicleBuilderVM;
template<typename ItemType> class SListView;
class SBox;
class SEditableTextBox;
class SVerticalBox;

/** P0-09 USER Acceptance에서 사용할 Guided Vehicle Builder Slate Shell입니다. */
class CARFIGHT_REEDITOR_API SCFVehicleBuilderTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCFVehicleBuilderTab) {}
	SLATE_END_ARGS()

	// Slate widget tree와 transient Builder ViewModel을 초기화합니다.
	void Construct(const FArguments& InArgs);

	// Widget이 닫힐 때 benchmark child process handle만 해제하고 process 자체는 강제 종료하지 않습니다.
	virtual ~SCFVehicleBuilderTab() override;

	// Non-blocking Technical Benchmark child process terminal 상태를 polling하고 current Step 8 result를 fresh 회수합니다.
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;

private:
	using FVehicleRowPtr = TSharedPtr<FCFVehicleListEntry>;

	// Vehicle Browser cache를 fresh read하고 list row를 다시 만듭니다.
	FReply HandleRefreshVehicles();

	// 선택 row를 current Builder target으로 전환합니다.
	void HandleVehicleSelectionChanged(FVehicleRowPtr SelectedItem, ESelectInfo::Type SelectInfo);

	// Browser row 하나의 Slate 표현을 생성합니다.
	TSharedRef<ITableRow> HandleGenerateVehicleRow(FVehicleRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	// 현재 Step Definition projection 기반 navigation UI를 생성합니다.
	TSharedRef<SWidget> BuildStepNavigation();

	// Step button을 눌렀을 때 허용된 visible page로 이동합니다.
	FReply HandleSelectStep(int32 StepIndex);

	// 이전 Step으로 이동합니다.
	FReply HandlePreviousStep();

	// 현재 authoritative truth를 다시 읽고 page 상태를 갱신합니다.
	FReply HandleRefreshCurrentStep();

	// Complete인 현재 Step에서만 다음 Step으로 이동합니다.
	FReply HandleNextStep();

	// Mesh-only 후보에서 VehicleData+Recipe 생성 proposal을 검토하고 explicit 승인 뒤 commit합니다.
	FReply HandleCreateVehicleFromMesh();

	// Step 1 current Recipe에 exact binding된 AI Research Draft를 Project Saved에서 읽습니다.
	FReply HandleLoadResearchDraft();

	// Step 1 Evidence + Missing private Profiles의 R2 proposal을 검토하고 explicit USER 승인 뒤 commit합니다.
	FReply HandleResearchCompanionReview();

	// Step 1 loaded ResearchDraft로 Existing Reference Evidence complete replacement R1을 검토하고 explicit USER 승인 뒤 commit합니다.
	FReply HandleReferenceEvidenceRefresh();

	// Step 1 current Evidence summary/fingerprint를 USER가 확인한 뒤 local Reference review token을 갱신합니다.
	FReply HandleAcceptReferenceSet();

	// Step 2 current Recipe의 Chassis/Wheel Mesh pending 선택값을 typed AssetIntent Recipe-only commit으로 반영합니다.
	FReply HandleCommitMeshPreparation();

	// Step 2 Chassis StaticMesh object picker 변경을 pending 값에만 반영합니다.
	void HandleChassisMeshChanged(const FAssetData& AssetData);

	// Step 2 Wheel StaticMesh object picker 변경을 role별 pending 값에만 반영합니다.
	void HandleWheelMeshChanged(const FAssetData& AssetData, int32 WheelRoleIndex);

	// Step 2 Chassis + FL/FR/RL/RR StaticMesh picker 묶음을 current pending state로 새로 만듭니다.
	TSharedRef<SWidget> BuildMeshPreparationPickerFields();

	// Step 2 Wheel Mesh role 한 행의 StaticMesh picker UI를 만듭니다.
	TSharedRef<SWidget> BuildWheelMeshPickerRow(int32 WheelRoleIndex, const FText& RoleLabel, bool bRequired);

	// 차량 선택/refresh/commit 뒤 ObjectPicker presentation을 current Recipe pending state로 재생성합니다.
	void RefreshMeshPreparationPickerPresentation();


	// Step 2 Mesh 준비 전용 UI 표시 조건입니다.
	EVisibility GetMeshPreparationVisibility() const;

	// Step 2 pending Chassis StaticMesh object path 문자열을 반환합니다.
	FString GetPendingChassisMeshPath() const;

	// Step 2 pending Wheel StaticMesh role별 object path 문자열을 반환합니다.
	FString GetPendingWheelMeshPath(int32 WheelRoleIndex) const;

	// Current Recipe AssetIntent를 Step 2 pending picker 값으로 동기화합니다.
	void SyncMeshPreparationFieldsFromRecipe();

	// Step 3 current Chassis StaticMesh를 Asset Editor에서 바로 엽니다.
	FReply HandleOpenCurrentChassisMesh();

	// Step 3 required Wheel Socket 한 개의 exact current 이름을 클립보드에 복사합니다.
	FReply HandleCopyRequiredWheelSocketName(int32 WheelRoleIndex);

	// Step 3 current Recipe의 optional Gameplay Socket 이름 전체를 줄바꿈 목록으로 복사합니다.
	FReply HandleCopyOptionalSocketNames();

	// Step 3 required Wheel Socket 한 행의 역할/이름/현재 상태/복사 UI를 만듭니다.
	TSharedRef<SWidget> BuildRequiredWheelSocketRow(int32 WheelRoleIndex, const FText& RoleLabel);

	// Step 3 소켓 준비 전용 UI 표시 조건입니다.
	EVisibility GetSocketPreparationVisibility() const;

	// Step 3 required Wheel Socket exact 이름을 표시합니다.
	FText GetRequiredWheelSocketNameText(int32 WheelRoleIndex) const;

	// Step 3 required Wheel Socket이 current Chassis에 존재하는지 표시합니다.
	FText GetRequiredWheelSocketStatusText(int32 WheelRoleIndex) const;

	// Step 3 current Recipe의 optional Hardpoint/Destroyed FX Socket 상태를 표시합니다.
	FText GetOptionalSocketGuideText() const;

	// Step 3에서 복사할 optional Socket 이름이 하나 이상 있는지 반환합니다.
	bool CanCopyOptionalSocketNames() const;

	// Step 5 current Recipe/Target/accepted Evidence에 exact binding된 AI Physics Draft를 Project Saved에서 읽습니다.
	FReply HandleLoadPhysicsProposalDraft();

	// Step 5 private 4 Profile mutation0 proposal을 검토하고 explicit USER 승인 뒤 existing typed commit을 실행합니다.
	FReply HandlePhysicsProposalReview();

	// Step 7 fresh Final Review 전체 Diff/provenance를 검토하고 explicit USER DefinitionApply 승인 뒤 existing R3 Apply를 실행합니다.
	FReply HandleFinalReviewApply();

	// Step 7 마지막 successful Builder Apply가 발급한 exact guarded Undo token을 USER 승인 뒤 실행합니다.
	FReply HandleFinalReviewUndo();

	// Step 8 current saved Target을 existing VB-P0-08 fixed-60Hz benchmark runner로 non-blocking 실행합니다.
	FReply HandleRunDrivingBenchmark();

	// Step 8 current selected VehicleData transient duplicate를 active PIE Player VehiclePawn에 적용합니다.
	FReply HandleApplyDrivingTargetToPIE();

	// Step 8 exact current benchmark/Target에 USER Driving PASS를 명시적으로 기록합니다.
	FReply HandleAcceptUserDriving();

	// 선택한 Mesh 후보 이름으로 사람이 수정할 수 있는 package/name 제안값을 채웁니다.
	void SyncCreationFieldsFromSelection();

	// Step 1의 Mesh-only 생성 UI 표시 조건입니다.
	EVisibility GetMeshCreationVisibility() const;

	// Step 1 managed Recipe의 Reference/Companion UI 표시 조건입니다.
	EVisibility GetReferenceFlowVisibility() const;

	// Step 1 persistent Evidence 또는 loaded Draft의 USER-facing 요약을 표시합니다.
	FText GetReferenceSummaryText() const;

	// Step 5 Physics Proposal 전용 UI 표시 조건입니다.
	EVisibility GetPhysicsProposalVisibility() const;

	// Step 5 loaded Draft/current receipt를 USER-facing 요약으로 표시합니다.
	FText GetPhysicsProposalSummaryText() const;

	// Step 6 Gameplay Setup 전용 R0 guidance UI 표시 조건입니다.
	EVisibility GetGameplaySetupVisibility() const;

	// Step 6의 8영역 completeness/USER Socket/pending diff를 USER-facing 요약으로 표시합니다.
	FText GetGameplaySetupSummaryText() const;

	// Step 7 Final Review 전용 R0 review/apply/undo UI 표시 조건입니다.
	EVisibility GetFinalReviewVisibility() const;

	// Step 7 validation/drift/provenance/전체 Field Diff/apply readiness를 USER-facing 요약으로 표시합니다.
	FText GetFinalReviewSummaryText() const;

	// Current fresh Final Review가 explicit DefinitionApply 가능한지 반환합니다.
	bool CanApplyFinalReview() const;

	// Current Editor lifetime에 exact Builder guarded Undo token이 있는지 반환합니다.
	bool CanUndoFinalReview() const;

	// Step 8 Technical Driving / USER Driving 전용 UI 표시 조건입니다.
	EVisibility GetDrivingTestVisibility() const;

	// Step 8 saved-state/technical metrics/USER checklist를 USER-facing 요약으로 표시합니다.
	FText GetDrivingTestSummaryText() const;

	// Current Step 8에서 benchmark child process를 새로 시작할 수 있는지 반환합니다.
	bool CanRunDrivingBenchmark() const;

	// Current benchmark가 exact current Target에 binding됐을 때 PIE transient 적용 버튼을 활성화합니다.
	bool CanApplyDrivingTargetToPIE() const;

	// Current benchmark가 exact current Target에 binding됐을 때 USER explicit PASS 버튼을 활성화합니다.
	bool CanAcceptUserDriving() const;

	// 기존 Advanced Vehicle Authoring Workspace를 별도 탭으로 엽니다.
	FReply HandleOpenAdvancedWorkspace();

	// 현재 선택을 USER-facing 한 줄로 표시합니다.
	FText GetSelectionText() const;

	// 현재 Step 제목을 표시합니다.
	FText GetCurrentStepTitle() const;

	// 현재 Step 상태를 표시합니다.
	FText GetCurrentStepStateText() const;

	// 현재 Step 설명을 표시합니다.
	FText GetCurrentStepSummaryText() const;

	// 현재 Step 해결/다음 행동을 표시합니다.
	FText GetCurrentStepResolutionText() const;

	// Step button label을 상태와 함께 표시합니다.
	FText GetStepButtonText(int32 StepIndex) const;

	// Step button을 현재 state에서 클릭할 수 있는지 반환합니다.
	bool IsStepButtonEnabled(int32 StepIndex) const;

	// Previous 버튼 활성 조건입니다.
	bool CanMovePrevious() const;

	// Next 버튼 활성 조건입니다.
	bool CanMoveNext() const;

	// 내부 Builder ViewModel입니다.
	TSharedPtr<FCFVehicleBuilderVM> ViewModel;

	// SListView가 소유할 lightweight Vehicle row copies입니다.
	TArray<FVehicleRowPtr> VehicleRows;

	// Vehicle Browser list widget입니다.
	TSharedPtr<SListView<FVehicleRowPtr>> VehicleListView;

	// current definition-driven Step button container입니다.
	TSharedPtr<SVerticalBox> StepNavigationBox;

	// 신규 VehicleData package 경로 입력입니다.
	TSharedPtr<SEditableTextBox> NewDefinitionPackageTextBox;

	// 신규 VehicleData Asset 이름 입력입니다.
	TSharedPtr<SEditableTextBox> NewDefinitionNameTextBox;

	// 신규 Recipe package 경로 입력입니다.
	TSharedPtr<SEditableTextBox> NewRecipePackageTextBox;

	// 신규 Recipe Asset 이름 입력입니다.
	TSharedPtr<SEditableTextBox> NewRecipeNameTextBox;

	// Step 2에서 explicit 반영 전까지 보관하는 pending Chassis StaticMesh path입니다.
	FSoftObjectPath PendingChassisMeshPath;

	// Step 2에서 explicit 반영 전까지 FL/FR/RL/RR 순서로 보관하는 pending Wheel StaticMesh path입니다.
	TArray<FSoftObjectPath> PendingWheelMeshPaths;

	// Step 2 ObjectPicker subtree를 current Recipe 기준으로 교체할 host입니다.
	TSharedPtr<SBox> MeshPreparationPickerHost;


	// 마지막 read/action 결과를 USER에게 보여주는 status text입니다.
	FText LastStatusText;

	// Step 8 existing benchmark runner를 실행 중일 때만 보관하는 OS process handle입니다.
	FProcHandle DrivingBenchmarkProcess;

	// 현재 실행 중인 benchmark result와 USER acceptance를 binding할 exact RunId입니다.
	FString RunningDrivingBenchmarkRunId;

	// Step 8 benchmark child process가 terminal 회수 전인지 여부입니다.
	bool bDrivingBenchmarkRunning = false;
};
