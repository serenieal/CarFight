// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTab.h
// Version: v1.18.0
// Date: 2026-09-02
// Description: Guided Vehicle Builder + CF-FQ-042 Vehicle ID/Candidate Quick Start Editor Slate Shell입니다.
// Changelog:
// - v1.18.0: P0-07 USER UAT 2차 피드백. Wheel/Hardpoint `추가 후 편집` enable을 cached row snapshot이 아니라 live current Chassis FindSocket truth로 통일하고, Step 3 상단 status/next-action 고정 프레임 + Socket Preparation 내부 scroll 레이아웃을 지원.
// - v1.17.0: P0-07 UAT 피드백 반영. Step 3 Wheel/Hardpoint 동일 Chassis 편집 버튼을 하나로 통합하고 exact missing Socket을 원점에 explicit 생성 후 편집하는 handler를 추가. SocketName copy/read-only slot UX와 파괴 FX Socket 명칭을 명확화.
// - v1.16.0: CF-FQ-043 VMG-P0-04 Step 6 Standard 1:1 Mount transient draft(Type/Size/Preset) + explicit typed commit/remove panel과 presentation sync 계약을 추가.
// - v1.15.1: VMG-P0-03 Step 2 pending Chassis/FL/FR/RL/RR Mesh 직접 열기와 optional Wheel→pending FL fallback, shared Chassis edit warning용 Slate helper 계약을 추가.
// - v1.15.0: CF-FQ-043 VMG-P0-03 Step 3 explicit Hardpoint Plan Mode, Standard category add/remove rows, contextual Wheel/Hardpoint Socket editor entry와 shared Chassis warning presentation을 추가.
// - v1.14.0: Step 1 일반 naming을 Vehicle ID 한 칸으로 전환하고 deterministic default identity와 접힌 Advanced Asset 경로 override, Mesh Candidate Quick Start label/flow를 추가.
// - v1.13.0: 신규 제작 Step 1에 optional Chassis StaticMesh picker/Blank Start와 공통 Preview→승인→Commit→exact row highlight flow를 연결.
// - v1.12.0: 작업 대상 영역에 selection-independent '+ 새 차량 만들기' entry를 추가하고 BuilderVM transient 신규 제작 상태와 연결.
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
// - v1.14.0부터 일반 Guided creation은 Vehicle ID 한 칸이 기본 입력이며 4개 package/name은 `고급 Asset 경로 설정` 접힘 영역의 override로 이동합니다. invalid Vehicle ID는 sanitize하지 않고 즉시 설명하며 최종 collision/path/type는 existing Preview authority가 판정합니다.
// - v1.13.0 Explicit New Vehicle은 기존 4개 package/name 입력을 P0-02 임시 identity UI로 재사용합니다. Vehicle ID 단일 naming은 P0-03이 소유합니다. Chassis picker는 optional이며 사용 중 Mesh도 허용하고 생성 시 Recipe AssetIntent만 기록합니다.
// - v1.12.0 신규 차량 버튼은 transient 진입만 수행하며 Asset 생성/Save/VehicleData Apply는 하지 않습니다. 기존 Mesh-only Quick Start도 그대로 유지됩니다.
// - 기존 Vehicle Authoring Advanced Workspace를 대체하지 않고 별도 Guided entry로 병행합니다.

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;
struct FCFVehicleListEntry;
enum class ECFBuilderHardpointPlanMode : uint8;
enum class ECFVehicleMountType : uint8;
enum class ECFVehicleWeaponSize : uint8;
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

	// 기존 Browser selection과 독립적으로 신규 차량 제작 진입 상태를 시작합니다.
	FReply HandleBeginNewVehicleEntry();

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

	// Explicit New Vehicle 또는 Mesh-only Quick Start의 VehicleData+Recipe proposal을 검토하고 explicit 승인 뒤 공통 commit/adoption 경로를 실행합니다.
	FReply HandleCreateVehicleFromMesh();

	// Vehicle ID 입력 변경을 BuilderVM canonical transient state에 반영하고 valid할 때 default package/object identity를 갱신합니다.
	void HandleVehicleIdTextChanged(const FText& NewText);

	// Current Vehicle ID로 deterministic default Definition/Recipe package/object 네 값을 Advanced 입력란에 채웁니다.
	void ApplyDefaultCreationIdentityFromVehicleId();

	// Advanced Definition/Recipe package/object 입력 네 값을 모두 비웁니다.
	void ClearCreationIdentityFields();

	// Current Vehicle ID validation 안내를 표시합니다.
	FText GetVehicleIdValidationText() const;

	// Explicit New Vehicle과 Mesh Candidate Quick Start를 구분하는 생성 검토 버튼 문구를 반환합니다.
	FText GetCreateVehicleButtonText() const;

	// Explicit New Vehicle의 optional Chassis StaticMesh picker 변경을 BuilderVM canonical creation state에 반영합니다.
	void HandleNewVehicleChassisMeshChanged(const FAssetData& AssetData);

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

	// Step 2에서 explicit commit 전 pending Chassis StaticMesh를 Asset Editor에서 직접 엽니다.
	FReply HandleOpenPendingChassisMesh();

	// Step 2에서 role별 pending Wheel StaticMesh를 열고 optional role이 비었으면 pending FL fallback을 엽니다.
	FReply HandleOpenPendingWheelMesh(int32 WheelRoleIndex);

	// Step 2 pending Chassis Mesh open 버튼 활성 여부입니다.
	bool CanOpenPendingChassisMesh() const;

	// Step 2 role별 effective pending Wheel Mesh open 버튼 활성 여부입니다.
	bool CanOpenPendingWheelMesh(int32 WheelRoleIndex) const;

	// Step 2 optional Wheel fallback 여부를 포함하는 동적 open tooltip입니다.
	FText GetPendingWheelMeshOpenTooltip(int32 WheelRoleIndex) const;

	// Step 2 direct role path 또는 optional role의 pending FL fallback path를 반환합니다.
	FSoftObjectPath GetEffectivePendingWheelMeshPath(int32 WheelRoleIndex) const;

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

	// Step 3 Wheel/Hardpoint가 공유하는 current Chassis StaticMesh를 하나의 Socket 편집 진입으로 엽니다.
	FReply HandleOpenChassisSocketEditor();

	// Step 3 exact missing SocketName을 current Chassis StaticMesh 원점에 explicit 생성하고 Asset Editor를 엽니다. 자동 저장/배치는 하지 않습니다.
	FReply HandleAddChassisSocket(FName SocketName);

	// Current Chassis StaticMesh를 공통 Socket 편집 문맥으로 엽니다.
	FReply OpenCurrentChassisMeshForSocketEditing();

	// Exact missing Socket을 current Chassis에 transaction으로 추가하고 편집기를 여는 공통 backend입니다.
	FReply AddChassisSocketAtOriginAndOpen(FName SocketName);

	// Current committed Chassis StaticMesh object의 live FindSocket truth를 반환합니다. UI button enable/status는 cached AssetSnapshot 대신 이 값을 사용합니다.
	bool IsCurrentChassisSocketPresentLive(FName SocketName) const;

	// Exact Socket이 현재 Chassis에 없을 때만 `추가 후 편집` 버튼을 활성화합니다.
	bool CanAddCurrentChassisSocket(FName SocketName) const;

	// Step 2/3에서 exact StaticMesh path를 공통 Asset Editor backend로 엽니다.
	FReply OpenStaticMeshAssetPath(const FSoftObjectPath& MeshPath, const FString& ContextLabel, const FString& CompletionNote);

	// Step 3 Hardpoint Plan Mode를 explicit USER action으로 기록합니다.
	FReply HandleSetHardpointPlanMode(ECFBuilderHardpointPlanMode PlanMode);

	// Step 3 Standard physical category에 deterministic stable Hardpoint row를 추가합니다.
	FReply HandleAddStandardHardpoint(FName LocationCategory);

	// Step 3 exact Recipe Hardpoint row를 typed no-cascade remove lane으로 제거합니다.
	FReply HandleRemoveHardpoint(FName LocationSlotId);

	// Step 3 Hardpoint row의 exact SocketName을 클립보드에 복사합니다.
	FReply HandleCopyHardpointSocketName(FName SocketName);

	// Current Recipe/Mode에서 Step 3 Hardpoint planning subtree를 만듭니다.
	TSharedRef<SWidget> BuildHardpointPlanningPanel();

	// Mode/add/remove/selection/refresh 뒤 Hardpoint planning subtree를 current truth로 교체합니다.
	void RefreshHardpointPlanningPresentation();

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

	// Explicit New Vehicle은 빈 Vehicle ID로 초기화하고 Mesh Candidate는 Mesh stem 기반 Vehicle ID 제안값을 채웁니다.
	void SyncCreationFieldsFromSelection();

	// Step 1의 Explicit New Vehicle 또는 Mesh-only Quick Start 생성 UI 표시 조건입니다.
	EVisibility GetMeshCreationVisibility() const;

	// Explicit New Vehicle 전용 Blank/optional Chassis picker 영역 표시 조건입니다.
	EVisibility GetNewVehicleCreationOptionsVisibility() const;

	// Explicit New Vehicle의 current optional Chassis StaticMesh object path 문자열을 반환합니다.
	FString GetNewVehicleChassisMeshPath() const;

	// Step 1 managed Recipe의 Reference/Companion UI 표시 조건입니다.
	EVisibility GetReferenceFlowVisibility() const;

	// Step 1 persistent Evidence 또는 loaded Draft의 USER-facing 요약을 표시합니다.
	FText GetReferenceSummaryText() const;

	// Step 5 Physics Proposal 전용 UI 표시 조건입니다.
	EVisibility GetPhysicsProposalVisibility() const;

	// Step 5 loaded Draft/current receipt를 USER-facing 요약으로 표시합니다.
	FText GetPhysicsProposalSummaryText() const;

	// Step 6 Hardpoint별 Standard MountType transient draft를 변경합니다. Recipe mutation은 하지 않습니다.
	FReply HandleSelectMountDraftType(FName LocationSlotId, ECFVehicleMountType MountType);

	// Step 6 Hardpoint별 Standard SizeLimit transient draft를 변경합니다. Recipe mutation은 하지 않습니다.
	FReply HandleSelectMountDraftSize(FName LocationSlotId, ECFVehicleWeaponSize SizeLimit);

	// Step 6 Hardpoint별 optional EquipmentPreset pending picker를 변경합니다. Recipe mutation은 하지 않습니다.
	void HandleMountPresetChanged(const FAssetData& AssetData, FName LocationSlotId);

	// Step 6 Hardpoint 하나의 complete transient draft를 Standard 1:1 typed Recipe write로 반영합니다.
	FReply HandleCommitStandardMount(FName LocationSlotId);

	// Step 6 exact Recipe MountProfileId 하나를 typed remove lane으로 제거합니다.
	FReply HandleRemoveMount(FName MountProfileId);

	// Step 6 current Recipe/HardpointPlanMode에서 Standard Mount planning subtree를 생성합니다.
	TSharedRef<SWidget> BuildMountPlanningPanel();

	// Step 6 persistent Recipe의 existing 1:1 Mount 값을 transient draft로 다시 동기화합니다.
	void SyncMountPlanningDraftsFromRecipe();

	// Step 6 transient/persistent 변경 뒤 Mount planning subtree를 current truth로 교체합니다.
	void RefreshMountPlanningPresentation();

	// Step 6 Hardpoint별 pending EquipmentPreset object path 문자열을 반환합니다.
	FString GetPendingMountPresetPath(FName LocationSlotId) const;

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

	// Guided 신규 차량 일반 naming의 단일 Vehicle ID 입력입니다.
	TSharedPtr<SEditableTextBox> NewVehicleIdTextBox;

	// 신규 VehicleData package 고급 override 입력입니다.
	TSharedPtr<SEditableTextBox> NewDefinitionPackageTextBox;

	// 신규 VehicleData Asset 이름 고급 override 입력입니다.
	TSharedPtr<SEditableTextBox> NewDefinitionNameTextBox;

	// 신규 Recipe package 경로 고급 override 입력입니다.
	TSharedPtr<SEditableTextBox> NewRecipePackageTextBox;

	// 신규 Recipe Asset 이름 고급 override 입력입니다.
	TSharedPtr<SEditableTextBox> NewRecipeNameTextBox;

	// Step 2에서 explicit 반영 전까지 보관하는 pending Chassis StaticMesh path입니다.
	FSoftObjectPath PendingChassisMeshPath;

	// Step 2에서 explicit 반영 전까지 FL/FR/RL/RR 순서로 보관하는 pending Wheel StaticMesh path입니다.
	TArray<FSoftObjectPath> PendingWheelMeshPaths;

	// Step 2 ObjectPicker subtree를 current Recipe 기준으로 교체할 host입니다.
	TSharedPtr<SBox> MeshPreparationPickerHost;

	// Step 3 Hardpoint Mode/row subtree를 current Recipe 기준으로 교체할 host입니다.
	TSharedPtr<SBox> HardpointPlanningHost;

	// Step 6 Standard Mount planning subtree를 current Recipe/draft 기준으로 교체할 host입니다.
	TSharedPtr<SBox> MountPlanningHost;

	// Step 6 Hardpoint별 pending MountType draft입니다. explicit 반영 전에는 persistent Recipe를 변경하지 않습니다.
	TMap<FName, ECFVehicleMountType> PendingMountTypes;

	// Step 6 Hardpoint별 pending SizeLimit draft입니다. explicit 반영 전에는 persistent Recipe를 변경하지 않습니다.
	TMap<FName, ECFVehicleWeaponSize> PendingMountSizes;

	// Step 6 Hardpoint별 pending optional EquipmentPresetData path입니다.
	TMap<FName, FSoftObjectPath> PendingMountPresetPaths;

	// Vehicle ID 실시간 validation/default naming 상태를 USER에게 설명합니다.
	FText VehicleIdValidationText;

	// 마지막 read/action 결과를 USER에게 보여주는 status text입니다.
	FText LastStatusText;

	// Step 8 existing benchmark runner를 실행 중일 때만 보관하는 OS process handle입니다.
	FProcHandle DrivingBenchmarkProcess;

	// 현재 실행 중인 benchmark result와 USER acceptance를 binding할 exact RunId입니다.
	FString RunningDrivingBenchmarkRunId;

	// Step 8 benchmark child process가 terminal 회수 전인지 여부입니다.
	bool bDrivingBenchmarkRunning = false;
};
