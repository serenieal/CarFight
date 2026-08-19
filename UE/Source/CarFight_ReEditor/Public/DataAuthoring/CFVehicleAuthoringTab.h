// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringTab.h
// Version: v1.4.0
// Date: 2026-08-18
// Description: DAUTH-P0-09~12 single-Vehicle Authoring Nomad Workspace Slate widget입니다.
// Changelog:
// - v1.4.0: P0-12 UA-03 USER feedback에 따라 5-domain Profile 후보 선택/Recipe binding/Open Profile UI state를 추가.
// - v1.3.0: P0-12 UA-01 한국어 우선 UI와 기본 Browser 테스트/레거시 숨김 표시 상태 추가.
// - v1.2.0: Frozen 24.90~24.94 Handling/Performance Adoption, Shared Profile impact, Drift 3-way recovery, Mesh-only Create flow UI 추가.
// - v1.1.0: P0-10 Assets/Layout, Wheel Measurement, 4축 Driving Feel/preset, Reference Compare, Mount/Default, Adoption, standard Undo parity UI 추가.
// - v1.0.0: Frozen Section 24 P0-09 Browser/Overview/Recipe/Preview/Diff/Trace/Validation/Apply/Raw DA 최소 UI 최초 구현.
// Migration:
// - SCFVDAWizardTab은 P0-10 Technical parity 후에도 DG/DEL Gate 전까지 별도 legacy Nomad Tab으로 유지합니다.
// - Batch main page는 이 Workspace에 추가하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FCFVehicleAuthoringVM;
class SEditableTextBox;
class SSearchBox;
class SSlider;
class SWidgetSwitcher;
template<typename OptionType> class SComboBox;
template<typename ItemType> class SListView;
struct FCFVehicleListEntry;
struct FCFProfileListEntry;
enum class ECFVehicleMeasureDecision : uint8;
enum class ECFVehicleAdoptGroup : uint8;
enum class ECFVehicleProfileDomain : uint8;
enum class ECFVehicleDriftDecision : uint8;


/** P0-09 center Main Authoring View의 최소 page입니다. */
enum class ECFVehicleAuthoringPage : uint8
{
	Overview,
	Recipe,
	Assets,
	DrivingFeel,
	Mounts,
	Compare,
	Validation,
	Advanced
};

/** P0-09 right Context Pane의 고정 tab입니다. */
enum class ECFVehicleContextPage : uint8
{
	Changes,
	SourceTrace,
	Issues,
	Sync
};

/** Frozen Section 24.3 3-pane + Bottom Action Bar를 구현하는 single-Vehicle Editor Workspace입니다. */
class CARFIGHT_REEDITOR_API SCFVehicleAuthoringTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCFVehicleAuthoringTab) {}
	SLATE_END_ARGS()

	// Slate Widget을 구성하고 initial Browser facade read를 수행합니다.
	void Construct(const FArguments& InArgs);

private:
	// Left Vehicle Browser row widget을 생성합니다.
	TSharedRef<ITableRow> GenerateVehicleRow(TSharedPtr<FCFVehicleListEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// Browser row 선택을 ViewModel current single-Vehicle selection으로 반영합니다.
	void HandleVehicleSelectionChanged(TSharedPtr<FCFVehicleListEntry> Item, ESelectInfo::Type SelectInfo);

	// Search text를 facade Browser filter에 반영합니다.
	void HandleSearchChanged(const FText& NewText);

	// Browser cache를 fresh facade result로 교체하고 list widget을 갱신합니다.
	FReply HandleRefreshBrowser();

	// Current selection Resolve/Diff/Trace/Validation을 fresh facade result로 갱신합니다.
	FReply HandleRefreshPreview();

	// Recipe Archetype text commit을 reviewed R1 facade transaction으로 반영합니다.
	void HandleArchetypeCommitted(const FText& NewText, ETextCommit::Type CommitType);

	// Unmanaged target의 Initial Import exact proposal을 review한 뒤 R2 facade commit을 실행합니다.
	FReply HandleInitialImport();

	// Current fresh Diff를 reviewed R3 approval로 준비하고 shared Apply lane을 실행합니다.
	FReply HandleApply();

		// Current Target VehicleData를 Unreal 표준 Raw Asset Editor에 엽니다.
	FReply HandleOpenRawDA();

	// P0-10 Assets/Layout page Slate를 구성합니다.
	TSharedRef<SWidget> BuildAssetsPage();

	// P0-10 4-axis Driving Feel page Slate를 구성합니다.
	TSharedRef<SWidget> BuildDrivingFeelPage();

	// P0-10 Mounts & Defaults migration page Slate를 구성합니다.
	TSharedRef<SWidget> BuildMountsPage();

		// P0-10 Pending/Reference Compare page Slate를 구성합니다.
	TSharedRef<SWidget> BuildComparePage();

	// P0-11 Recipe page 안의 Shared Profile Editor / Mesh-only Create panel을 구성합니다.
	TSharedRef<SWidget> BuildP11RecipePanel();

	// P0-11 Sync context의 External Drift 3-way review/recovery panel을 구성합니다.
	TSharedRef<SWidget> BuildP11SyncPanel();

	// Asset/Socket text를 typed Recipe AssetIntent로 commit합니다.
	FReply HandleCommitAssetIntent();

	// Frozen Driving Feel 4축 local values를 한 typed Recipe transaction으로 commit합니다.
	FReply HandleCommitDrivingFeel();

	// Named Frozen migration preset을 exact 4축 Recipe intent로 commit합니다.
	FReply HandleDrivingFeelPreset(FName PresetId);

	// Stable-ID Hardpoint text input을 typed Recipe intent로 upsert합니다.
	FReply HandleUpsertHardpoint();

	// Stable-ID Mount text input을 typed Recipe intent로 upsert합니다.
	FReply HandleUpsertMount();

	// Default Destroyed FX socket semantic intent를 Recipe-only commit합니다.
	FReply HandleCommitDefaultIntent();

	// Browser row를 Reference side로 선택합니다.
	void HandleReferenceSelectionChanged(TSharedPtr<FCFVehicleListEntry> Item, ESelectInfo::Type SelectInfo);

	// Reference Compare를 Same 포함 모드로 갱신합니다.
	FReply HandleReferenceCompareAll();

	// Reference Compare를 Different only 모드로 갱신합니다.
	FReply HandleReferenceCompareChanged();

	// Wheel measurement proposal의 measured candidate 채택을 review/commit합니다.
	FReply HandleMeasurementDecision(int32 ProposalIndex, ECFVehicleMeasureDecision Decision);

		// Legacy Pin ownership group Adoption을 review/commit합니다.
	FReply HandleAdoptionGroup(ECFVehicleAdoptGroup AdoptionGroup);

		// Profile Domain 선택을 바꾸고 해당 Domain의 existing Profile 후보를 read-only 갱신합니다.
	FReply HandleProfileDomainSelected(ECFVehicleProfileDomain ProfileDomain);

	// Selected Domain의 facade Profile 후보 cache를 Slate Combo rows로 재구성합니다.
	void RefreshProfileChoiceRows();

	// Shared Profile ComboBox 한 후보의 표시 widget을 생성합니다.
	TSharedRef<SWidget> GenerateProfileChoiceWidget(TSharedPtr<FCFProfileListEntry> Item) const;

	// Shared Profile ComboBox selection을 current explicit binding 후보로 보존합니다.
	void HandleProfileChoiceChanged(TSharedPtr<FCFProfileListEntry> Item, ESelectInfo::Type SelectInfo);

	// Shared Profile ComboBox의 current selection summary를 반환합니다.
	FText GetSelectedProfileChoiceText() const;

	// Current Recipe의 selected Domain Profile binding과 후보 상태를 표시합니다.
	FText GetProfileBindingText() const;

	// Selected Domain에 current bound Profile이 존재하는지 반환합니다.
	bool HasBoundSelectedProfile() const;

	// User-selected existing Profile을 reviewed Recipe-only BindVehicleProfile transaction으로 연결합니다.
	FReply HandleBindSelectedProfile();

	// Current Recipe에 bound된 selected Domain Shared Profile을 Unreal 표준 Asset Editor로 엽니다.
	FReply HandleOpenBoundProfile();

	// Selected Shared Profile Domain의 allowlisted numeric edit를 B2 preview/review/commit합니다.
	FReply HandleProfileNumericEdit();


	// Profile impact row의 affected Vehicle로 Browser selection을 이동합니다.
	FReply HandleNavigateAffectedVehicle(int32 ImpactIndex);

	// Current External Drift 3-way rows를 fresh review합니다.
	FReply HandleRefreshDriftReview();

	// Current External Drift 전체 group에 explicit recovery decision을 preview/review/commit합니다.
	FReply HandleDriftDecision(ECFVehicleDriftDecision Decision);

	// Mesh-only Candidate에서 Definition+Recipe 생성 proposal을 review/commit합니다.
	FReply HandleCreateVehicleFromMesh();

	// Workspace가 소유한 마지막 transaction을 Unreal standard Undo로 되돌립니다.
	FReply HandleUndoLastAction();


	// Center page selection을 바꿉니다.
	FReply SetMainPage(ECFVehicleAuthoringPage NewPage);

	// Context page selection을 바꿉니다.
	FReply SetContextPage(ECFVehicleContextPage NewPage);

	// Header의 current selection/state summary text를 만듭니다.
	FText GetHeaderText() const;

	// Center Overview/Resolve Preview text를 만듭니다.
	FText GetOverviewText() const;

	// Recipe/Profile basic summary text를 만듭니다.
	FText GetRecipeText() const;

		// Pending Authoring Changes authority table을 text presentation으로 만듭니다.
	FText GetDiffText() const;

	// Assets/Layout intent + derived sockets + measurement proposals를 read-only/text summary로 만듭니다.
	FText GetAssetsLayoutText() const;

	// 4축 Driving Feel current semantic state를 표시합니다.
	FText GetDrivingFeelText() const;

	// Mount/Default current intent를 Stable-ID 중심으로 표시합니다.
	FText GetMountsText() const;

	// Reference Vehicle Compare 117-field rows를 text presentation으로 만듭니다.
	FText GetReferenceCompareText() const;


	// Validation layer issue들을 persistent text presentation으로 만듭니다.
	FText GetValidationText() const;

	// Current right Changes context text를 만듭니다.
	FText GetChangesContextText() const;

	// Current Source Trace stack text를 만듭니다.
	FText GetSourceTraceText() const;

	// Current right Issues context text를 만듭니다.
	FText GetIssuesContextText() const;

		// Current stale/drift/freshness sync text를 만듭니다.
	FText GetSyncContextText() const;

	// Last Shared Profile preview의 affected Vehicle impact를 표시합니다.
	FText GetProfileImpactText() const;

	// Current External Drift의 Last Applied / Raw / Authoring 3-way rows를 표시합니다.
	FText GetDriftReviewText() const;

	// Current Mesh-only Candidate creation summary를 표시합니다.
	FText GetMeshCandidateText() const;

	// Bottom Action Bar summary text를 만듭니다.
	FText GetBottomStatusText() const;

	// Persistent operation result/status message를 반환합니다.
	FText GetOperationMessageText() const;

	// Current Apply button label을 pending count/warnings와 함께 만듭니다.
	FText GetApplyButtonText() const;

		// Normal Apply 활성 조건을 ViewModel에서 읽습니다.
	bool IsApplyEnabled() const;

	// Workspace-owned last transaction Undo 버튼 활성 조건입니다.
	bool IsUndoEnabled() const;


	// Managed Recipe editor section visibility입니다.
	EVisibility GetManagedVisibility() const;

		// Unmanaged Initial Import section visibility입니다.
	EVisibility GetUnmanagedVisibility() const;

	// Definition이 아닌 Mesh-only Candidate creation section visibility입니다.
	EVisibility GetMeshCandidateVisibility() const;

	// Managed/PartiallyManaged Raw DA escape hatch warning text를 반환합니다.
	FText GetRawDAWarningText() const;

	// Browser UObject rows를 Slate shared rows로 재구성합니다.
	void RebuildBrowserRows();

			// Current selection의 Recipe semantic edit fields를 persistent Recipe value에 맞춥니다.
	void SyncEditableFieldsFromSelection();

	// Current Mesh-only Candidate selection에 따라 P0-11 creation input suggestions를 갱신합니다.
	void SyncP11FieldsFromSelection();

	// Soft asset path text를 typed soft object pointer로 변환합니다.
	TSoftObjectPtr<UStaticMesh> ParseStaticMeshPath(const TSharedPtr<SEditableTextBox>& TextBox) const;


	// Transient Workspace ViewModel입니다.
	TSharedPtr<FCFVehicleAuthoringVM> ViewModel;

	// Slate list가 참조하는 Vehicle Browser rows입니다.
	TArray<TSharedPtr<FCFVehicleListEntry>> BrowserRows;

	// Left Vehicle Browser list widget입니다.
	TSharedPtr<SListView<TSharedPtr<FCFVehicleListEntry>>> VehicleListView;

		// Vehicle Browser 검색 입력입니다.
	TSharedPtr<SSearchBox> SearchBox;

	// 기본 Browser에서 테스트/레거시 record를 숨기고 필요할 때만 표시할지 결정합니다.
	bool bShowTechnicalBrowserRecords = false;

	// Managed Recipe basic Archetype semantic input입니다.
	TSharedPtr<SEditableTextBox> ArchetypeTextBox;

	// Unmanaged Initial Import destination folder input입니다.
	TSharedPtr<SEditableTextBox> ImportFolderTextBox;

		// Unmanaged Initial Import Recipe asset name input입니다.
	TSharedPtr<SEditableTextBox> ImportNameTextBox;

	// Chassis StaticMesh soft path semantic input입니다.
	TSharedPtr<SEditableTextBox> ChassisMeshTextBox;

	// FL/FR/RL/RR Wheel StaticMesh soft path semantic inputs입니다.
	TArray<TSharedPtr<SEditableTextBox>> WheelMeshTextBoxes;

	// FL/FR/RL/RR wheel socket semantic inputs입니다.
	TArray<TSharedPtr<SEditableTextBox>> WheelSocketTextBoxes;

	// Driving Feel Acceleration semantic local slider입니다.
	TSharedPtr<SSlider> AccelerationSlider;

	// Driving Feel Steering semantic local slider입니다.
	TSharedPtr<SSlider> SteeringSlider;

	// Driving Feel Grip semantic local slider입니다.
	TSharedPtr<SSlider> GripSlider;

	// Driving Feel Suspension semantic local slider입니다.
	TSharedPtr<SSlider> SuspensionSlider;

	// Commit 전 local Acceleration Feel 0~1 값입니다.
	float PendingAccelerationFeel = 0.5f;

	// Commit 전 local Steering Agility 0~1 값입니다.
	float PendingSteeringAgility = 0.5f;

	// Commit 전 local Grip Feel 0~1 값입니다.
	float PendingGripFeel = 0.5f;

	// Commit 전 local Suspension Firmness 0~1 값입니다.
	float PendingSuspensionFirmness = 0.5f;

	// Reference Vehicle selector입니다.
	TSharedPtr<SComboBox<TSharedPtr<FCFVehicleListEntry>>> ReferenceComboBox;

	// Reference selector에 표시할 current selected row입니다.
	TSharedPtr<FCFVehicleListEntry> SelectedReferenceRow;

	// Stable-ID Hardpoint LocationSlotId input입니다.
	TSharedPtr<SEditableTextBox> HardpointIdTextBox;

	// Hardpoint category input입니다.
	TSharedPtr<SEditableTextBox> HardpointCategoryTextBox;

	// Hardpoint chassis socket input입니다.
	TSharedPtr<SEditableTextBox> HardpointSocketTextBox;

	// Stable-ID MountProfileId input입니다.
	TSharedPtr<SEditableTextBox> MountIdTextBox;

		// Mount LocationSlotRef input입니다.
	TSharedPtr<SEditableTextBox> MountLocationTextBox;

	// MountType enum display/name input입니다.
	TSharedPtr<SEditableTextBox> MountTypeTextBox;

	// Weapon SizeLimit enum display/name input입니다.
	TSharedPtr<SEditableTextBox> MountSizeTextBox;

		// Default Destroyed FX socket semantic input입니다.
	TSharedPtr<SEditableTextBox> DestroyedFxSocketTextBox;

		// Shared Profile nested editor에서 선택한 Frozen Profile Domain입니다.
	ECFVehicleProfileDomain SelectedProfileDomain;

	// Selected Domain의 existing Shared Profile 후보를 ComboBox용 shared rows로 보존합니다.
	TArray<TSharedPtr<FCFProfileListEntry>> ProfileChoiceRows;

	// Existing Shared Profile 후보 선택 ComboBox입니다.
	TSharedPtr<SComboBox<TSharedPtr<FCFProfileListEntry>>> ProfileChoiceComboBox;

	// User가 explicit binding 후보로 선택한 Shared Profile row입니다.
	TSharedPtr<FCFProfileListEntry> SelectedProfileChoice;

	// Shared Profile editable Registry stable ColumnId input입니다.
	TSharedPtr<SEditableTextBox> ProfileColumnIdTextBox;

	// Shared Profile canonical numeric value input입니다.
	TSharedPtr<SEditableTextBox> ProfileNumericValueTextBox;

	// Advanced Drift Override 이유 input입니다.
	TSharedPtr<SEditableTextBox> DriftOverrideReasonTextBox;

	// Mesh-only Candidate가 생성할 Definition package input입니다.
	TSharedPtr<SEditableTextBox> NewDefinitionPackageTextBox;

	// Mesh-only Candidate가 생성할 Definition asset name input입니다.
	TSharedPtr<SEditableTextBox> NewDefinitionNameTextBox;

	// Mesh-only Candidate가 생성할 Recipe package input입니다.
	TSharedPtr<SEditableTextBox> NewRecipePackageTextBox;

	// Mesh-only Candidate가 생성할 Recipe asset name input입니다.
	TSharedPtr<SEditableTextBox> NewRecipeNameTextBox;


	// Center page switcher입니다.
	TSharedPtr<SWidgetSwitcher> MainPageSwitcher;

	// Right context switcher입니다.
	TSharedPtr<SWidgetSwitcher> ContextPageSwitcher;

	// Current center page입니다.
	ECFVehicleAuthoringPage CurrentMainPage = ECFVehicleAuthoringPage::Overview;

	// Current context page입니다.
	ECFVehicleContextPage CurrentContextPage = ECFVehicleContextPage::Changes;
};
