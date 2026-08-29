// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringTab.cpp
// Version: v1.10.0
// Date: 2026-08-21
// Description: DAUTH-P0-09~12 Frozen Section 24 single-Vehicle Authoring Workspace Slate 구현입니다.
// Changelog:
// - v1.10.0: P0-12 UA-06 USER UX remediation으로 final Apply review에 bounded field/current/apply-after 상세를 추가하고 Shared Profile numeric selector 상태를 selection refresh와 동기화.
// - v1.9.0: P0-12 UA-05 USER 피드백에 따라 값 출처 탭을 검색 + 실제 적용 우선 + 비적용 후보 선택 표시 UX로 교정.
// - v1.8.0: P0-12 UA-05 USER 피드백에 따라 문제 탭 검증 이슈를 심각도/코드/필드/내용 의미 단위로 분리하고 문장 사이 줄바꿈을 추가.
// - v1.7.0: P0-12 UA-04 USER 피드백에 따라 사용자-facing Authoring/VehicleData 용어를 `제작 기준값/차량 데이터`로 현지화하고 기본 Apply 대상은 Asset 이름만 표시.
// - v1.6.0: P0-12 UA-05~06 readiness에서 stale Preview의 Header/Overview/Validation/Sync/Bottom cached 결과 노출을 막고 Apply terminal failure를 즉시 표시하도록 보강.
// - v1.5.0: P0-12 UA-04 macro-flow UX로 External Drift read-only auto-review/Sync 안내, 목적 중심 summary와 drift recovery Apply review 문구를 추가.
// - v1.4.0: P0-12 UA-02 피드백에 따라 Initial Import를 '제작 관리 시작' UX로 교정하고 기본 Recipe 이름 제안과 preview/commit 실패 팝업을 추가.
// - v1.3.0: P0-12 UA-01 피드백에 따라 한국어 우선 UI, 테스트/레거시 기본 숨김 Browser, 폐기 DA_PoliceCar 표시 필터를 추가.
// - v1.2.0: Frozen 24.91~24.94 Shared Profile/Mesh Candidate nested Recipe route와 3-way Drift Sync route를 연결.
// - v1.1.0: P0-10 Assets/Layout, Measurement, 4축 Driving Feel/preset, Reference Compare, Mount/Defaults, Adoption, standard Undo page를 연결.
// - v1.0.0: 3-pane Workspace, Vehicle Browser, 8 main navigation, 4 Context tabs, Initial Import, Recipe basic intent, Diff/Trace/Validation, Apply, Raw DA Open 추가.
// Migration:
// - v1.10.0은 fresh FieldDiff projection을 final review에 최대 8건 bounded 표시할 뿐 ApplyService/approval/TOCTOU/no-auto-save 의미를 변경하지 않습니다.
// - v1.9.0은 Resolver SourceTrace 배열과 effective layer 판정을 그대로 사용하며 검색/표시 밀도만 변경합니다.
// - v1.8.0은 Validation issue presentation만 변경하며 issue code/message/field path와 Validation 계약은 변경하지 않습니다.
// - v1.7.0은 사용자-facing 문구만 현지화하며 C++ Authoring/VehicleData 타입명, facade 계약, serialization은 변경하지 않습니다.
// - P0-10 parity page 구현은 CFVehicleAuthoringP10.cpp로 분리하며 공통 ViewModel/facade만 사용합니다.
// - SCFVDAWizardTab은 DG/DEL Gate 전까지 별도 legacy Nomad Tab으로 유지합니다.
// - Batch main page는 추가하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringTab.h"

#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

namespace CFVehicleAuthoringTabPrivate
{
		// Management view를 Header/Browser용 한국어 text로 변환합니다.
	FString ManageText(const ECFWorkspaceManageView View)
	{
		switch (View)
		{
		case ECFWorkspaceManageView::LegacyImported: return TEXT("레거시 가져옴");
		case ECFWorkspaceManageView::PartiallyManaged: return TEXT("부분 관리");
		case ECFWorkspaceManageView::Managed: return TEXT("관리됨");
		default: return TEXT("미관리");
		}
	}

		// Sync view를 Header/Context용 한국어 text로 변환합니다.
	FString SyncText(const ECFWorkspaceSyncView View)
	{
		switch (View)
		{
		case ECFWorkspaceSyncView::InSync: return TEXT("동기화됨");
		case ECFWorkspaceSyncView::EffectiveStale: return TEXT("적용값 오래됨");
		case ECFWorkspaceSyncView::ShadowChanged: return TEXT("비적용 출처 변경");
		case ECFWorkspaceSyncView::ExternalDrift: return TEXT("외부 변경 감지");
		case ECFWorkspaceSyncView::PreviewOutOfDate: return TEXT("미리보기 오래됨");
		default: return TEXT("기준 없음");
		}
	}

		// Validation view를 Header용 한국어 text로 변환합니다.
	FString ValidationText(const ECFWorkspaceValidView View)
	{
		switch (View)
		{
		case ECFWorkspaceValidView::Valid: return TEXT("정상");
		case ECFWorkspaceValidView::Warning: return TEXT("경고");
		case ECFWorkspaceValidView::Blocked: return TEXT("적용 차단");
		default: return TEXT("미검사");
		}
	}

		// Preview view를 Bottom Action Bar용 한국어 text로 변환합니다.
	FString PreviewText(const ECFWorkspacePreviewView View)
	{
		switch (View)
		{
		case ECFWorkspacePreviewView::Fresh: return TEXT("최신");
		case ECFWorkspacePreviewView::Blocked: return TEXT("차단됨");
		case ECFWorkspacePreviewView::OutOfDate: return TEXT("오래됨");
		default: return TEXT("새로고침 필요");
		}
	}

		// Diff operation을 사람이 읽는 짧은 한국어 label로 변환합니다.
	FString DiffOperationText(const ECFVehicleDiffOp Operation)
	{
		switch (Operation)
		{
		case ECFVehicleDiffOp::AddArrayElement: return TEXT("추가");
		case ECFVehicleDiffOp::RemoveArrayElement: return TEXT("제거");
		case ECFVehicleDiffOp::MoveArrayElement: return TEXT("이동");
		default: return TEXT("변경");
		}
	}

		// Validation severity를 사람이 읽는 한국어 label로 변환합니다.
	FString SeverityText(const ECFVehicleValidationSeverity Severity)
	{
		switch (Severity)
		{
		case ECFVehicleValidationSeverity::Warning: return TEXT("경고");
		case ECFVehicleValidationSeverity::Blocked: return TEXT("차단");
		case ECFVehicleValidationSeverity::Error: return TEXT("오류");
		default: return TEXT("정보");
		}
	}

	// SourceType UENUM display name을 안전하게 얻습니다.
	FString SourceTypeText(const ECFVehicleSourceType SourceType)
	{
		// ECFVehicleSourceType reflection enum입니다.
		const UEnum* Enum = StaticEnum<ECFVehicleSourceType>();
		return Enum ? Enum->GetDisplayNameTextByValue(static_cast<int64>(SourceType)).ToString() : FString::FromInt(static_cast<int32>(SourceType));
	}

		// Long hash를 UI에서 식별 가능한 compact prefix로 표시합니다.
	FString ShortHash(const FString& Hash)
	{
		return Hash.Len() > 12 ? Hash.Left(12) + TEXT("…") : Hash;
	}

			// Boolean 상태를 사용자 화면용 한국어로 변환합니다.
	const TCHAR* BooleanText(const bool bValue)
	{
		return bValue ? TEXT("있음") : TEXT("없음");
	}

	// Stable Field Path의 마지막 segment를 macro-flow 사용자 표시용 짧은 이름으로 변환합니다.
	FString FieldDisplayText(const FCFVehicleFieldPath& FieldPath)
	{
		// Canonical Stable Field Path 전체 문자열입니다.
		const FString CanonicalPath = FieldPath.ToCanonicalString(true);
		// 마지막 구조 구분점 위치입니다.
		const int32 LastDotIndex = CanonicalPath.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		return LastDotIndex != INDEX_NONE && LastDotIndex + 1 < CanonicalPath.Len()
			? CanonicalPath.Mid(LastDotIndex + 1)
			: CanonicalPath;
	}

	// 기존 VehicleData 이름에서 사람이 수정할 수 있는 새 Recipe 이름 제안을 만듭니다.
	FString BuildSuggestedRecipeName(const FCFVehicleListEntry& Entry)
	{
		// 선택된 VehicleData asset 이름에서 접두어를 제거한 표시용 stem입니다.
		FString Stem = Entry.DefinitionPath.GetAssetName();
		if (!Stem.RemoveFromStart(TEXT("DA_Vehicle_")))
		{
			if (!Stem.RemoveFromStart(TEXT("DA_Veh_")))
			{
				Stem.RemoveFromStart(TEXT("DA_"));
			}
		}
		if (Stem.IsEmpty())
		{
			Stem = TEXT("Vehicle");
		}
		return TEXT("DA_Recipe_") + Stem;
	}

	// 기본 Vehicle Browser에서 숨길 테스트/레거시 record인지 판정합니다.
	bool IsTechnicalOrRetiredBrowserEntry(const FCFVehicleListEntry& Entry)
	{
		// Definition 또는 Mesh-only Candidate의 canonical identity입니다.
		const FSoftObjectPath IdentityPath = Entry.bMeshOnlyCandidate ? Entry.ChassisMeshPath : Entry.DefinitionPath;
		// UI 표시 필터가 검사할 full object path입니다.
		const FString ObjectPath = IdentityPath.ToString();
		// 프로젝트에서 사용 중단이 명시된 exact DA_PoliceCar를 삭제 없이 숨깁니다.
		if (IdentityPath.GetAssetName().Equals(TEXT("DA_PoliceCar"), ESearchCase::IgnoreCase))
		{
			return true;
		}
		// 테스트/생성물/개발자/레거시 전용 디렉터리는 기본 제작 목록에서 숨깁니다.
		return ObjectPath.Contains(TEXT("/Tests/"), ESearchCase::IgnoreCase)
			|| ObjectPath.Contains(TEXT("/Test/"), ESearchCase::IgnoreCase)
			|| ObjectPath.Contains(TEXT("/_GENERATED/"), ESearchCase::IgnoreCase)
			|| ObjectPath.Contains(TEXT("/Developers/"), ESearchCase::IgnoreCase)
			|| ObjectPath.Contains(TEXT("/Legacy/"), ESearchCase::IgnoreCase);
	}

	// Read-only long text를 scrollable body로 만드는 공용 Slate helper입니다.
	TSharedRef<SWidget> MakeTextBody(const TAttribute<FText>& TextAttribute)
	{
		return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(STextBlock)
			.Text(TextAttribute)
			.AutoWrapText(true)
		];
	}
}

// Slate Widget을 구성하고 initial Browser facade read를 수행합니다.
void SCFVehicleAuthoringTab::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FCFVehicleAuthoringVM>();
	SelectedProfileDomain = ECFVehicleProfileDomain::Handling;
	// Initial Browser facade read diagnostic입니다.
	FString BrowserError;
	ViewModel->RefreshBrowser(FString(), BrowserError);
	RebuildBrowserRows();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(STextBlock)
				.Text(this, &SCFVehicleAuthoringTab::GetHeaderText)
				.AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f, 0.0f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			.Value(0.22f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
										SAssignNew(SearchBox, SSearchBox)
					.HintText(FText::FromString(TEXT("차량 검색")))
					.OnTextChanged(this, &SCFVehicleAuthoringTab::HandleSearchChanged)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
										SNew(SButton)
					.Text(FText::FromString(TEXT("차량 목록 새로고침")))
					.OnClicked(this, &SCFVehicleAuthoringTab::HandleRefreshBrowser)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this]() { return bShowTechnicalBrowserRecords ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
					{
						bShowTechnicalBrowserRecords = NewState == ECheckBoxState::Checked;
						RebuildBrowserRows();
					})
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("테스트/레거시 표시")))
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SAssignNew(VehicleListView, SListView<TSharedPtr<FCFVehicleListEntry>>)
					.ListItemsSource(&BrowserRows)
					.OnGenerateRow(this, &SCFVehicleAuthoringTab::GenerateVehicleRow)
					.OnSelectionChanged(this, &SCFVehicleAuthoringTab::HandleVehicleSelectionChanged)
				]
			]
			+ SSplitter::Slot()
			.Value(0.53f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("개요"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Overview); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("레시피 / 프로필"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Recipe); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("에셋 / 배치"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Assets); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("주행감"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::DrivingFeel); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("장착 / 기본값"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Mounts); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("비교 / 변경점"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Compare); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("검증"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Validation); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("고급"))).OnClicked_Lambda([this](){ return SetMainPage(ECFVehicleAuthoringPage::Advanced); })]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(4.0f)
				[
					SAssignNew(MainPageSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()
					[
						CFVehicleAuthoringTabPrivate::MakeTextBody(TAttribute<FText>::CreateSP(this, &SCFVehicleAuthoringTab::GetOverviewText))
					]
					+ SWidgetSwitcher::Slot()
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
							[
								SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetRecipeText).AutoWrapText(true)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
							[
								SNew(SVerticalBox)
								.Visibility(this, &SCFVehicleAuthoringTab::GetManagedVisibility)
								+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("차량 유형 — 값을 입력한 뒤 Enter로 레시피에 반영")))]
								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(ArchetypeTextBox, SEditableTextBox)
									.OnTextCommitted(this, &SCFVehicleAuthoringTab::HandleArchetypeCommitted)
								]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
							[
								SNew(SVerticalBox)
								.Visibility(this, &SCFVehicleAuthoringTab::GetUnmanagedVisibility)
																+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("제작 관리 시작 — 이미 존재하는 차량 데이터는 그대로 둡니다. 현재 값을 기준으로 편집용 레시피를 새로 만들고, 이후 변경은 레시피/프로필에서 검토한 뒤 차량 데이터에 적용합니다."))).AutoWrapText(true)]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("레시피 저장 폴더")))]
								+ SVerticalBox::Slot().AutoHeight()[SAssignNew(ImportFolderTextBox, SEditableTextBox).Text(FText::FromString(TEXT("/Game/CarFight/Data/Authoring"))).HintText(FText::FromString(TEXT("/Game 이하 저장 폴더")))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("새 레시피 이름")))]
								+ SVerticalBox::Slot().AutoHeight()[SAssignNew(ImportNameTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("예: DA_Recipe_TestSUV")))]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("제작 관리 시작 검토…"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleInitialImport)]
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
							[
								BuildP11RecipePanel()
							]
						]
					]
					+ SWidgetSwitcher::Slot()[BuildAssetsPage()]
					+ SWidgetSwitcher::Slot()[BuildDrivingFeelPage()]
					+ SWidgetSwitcher::Slot()[BuildMountsPage()]
					+ SWidgetSwitcher::Slot()[BuildComparePage()]
					+ SWidgetSwitcher::Slot()[CFVehicleAuthoringTabPrivate::MakeTextBody(TAttribute<FText>::CreateSP(this, &SCFVehicleAuthoringTab::GetValidationText))]
					+ SWidgetSwitcher::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().FillHeight(1.0f)[CFVehicleAuthoringTabPrivate::MakeTextBody(TAttribute<FText>::CreateSP(this, &SCFVehicleAuthoringTab::GetRawDAWarningText))]
						+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("원본 차량 데이터 열기"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleOpenRawDA)]
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
				[
					SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("변경점"))).OnClicked_Lambda([this](){ return SetContextPage(ECFVehicleContextPage::Changes); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("값 출처"))).OnClicked_Lambda([this](){ return SetContextPage(ECFVehicleContextPage::SourceTrace); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("문제"))).OnClicked_Lambda([this](){ return SetContextPage(ECFVehicleContextPage::Issues); })]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("동기화"))).OnClicked_Lambda([this](){ return SetContextPage(ECFVehicleContextPage::Sync); })]
				]
				+ SVerticalBox::Slot().FillHeight(1.0f).Padding(4.0f)
				[
					SAssignNew(ContextPageSwitcher, SWidgetSwitcher)
										+ SWidgetSwitcher::Slot()[CFVehicleAuthoringTabPrivate::MakeTextBody(TAttribute<FText>::CreateSP(this, &SCFVehicleAuthoringTab::GetChangesContextText))]
					+ SWidgetSwitcher::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SNew(SSearchBox)
							.HintText(FText::FromString(TEXT("필드명 / 출처 검색")))
							.OnTextChanged_Lambda([this](const FText& NewText)
							{
								SourceTraceSearchText = NewText.ToString().TrimStartAndEnd();
							})
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SNew(SCheckBox)
							.IsChecked_Lambda([this]() { return bShowSourceTraceCandidates ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
							.OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
							{
								bShowSourceTraceCandidates = NewState == ECheckBoxState::Checked;
							})
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("비적용 후보까지 보기")))
							]
						]
						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							CFVehicleAuthoringTabPrivate::MakeTextBody(TAttribute<FText>::CreateSP(this, &SCFVehicleAuthoringTab::GetSourceTraceText))
						]
					]
					+ SWidgetSwitcher::Slot()[CFVehicleAuthoringTabPrivate::MakeTextBody(TAttribute<FText>::CreateSP(this, &SCFVehicleAuthoringTab::GetIssuesContextText))]
										+ SWidgetSwitcher::Slot()[BuildP11SyncPanel()]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(SBorder)
			.Padding(6.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetBottomStatusText).AutoWrapText(true)]
										+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("미리보기 새로고침"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleRefreshPreview)]
					+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("마지막 작업 되돌리기"))).IsEnabled(this, &SCFVehicleAuthoringTab::IsUndoEnabled).OnClicked(this, &SCFVehicleAuthoringTab::HandleUndoLastAction)]
					+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(this, &SCFVehicleAuthoringTab::GetApplyButtonText).IsEnabled(this, &SCFVehicleAuthoringTab::IsApplyEnabled).OnClicked(this, &SCFVehicleAuthoringTab::HandleApply)]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetOperationMessageText).AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
										SNew(STextBlock).Text(FText::FromString(TEXT("되돌리기: 이 작업창에서 성공한 마지막 변경은 위 버튼으로 되돌릴 수 있습니다. 일반 Ctrl+Z는 Unreal 전체 작업 기록을 따릅니다."))).AutoWrapText(true)
				]
			]
		]
	];
}

// Left Vehicle Browser row widget을 생성합니다.
TSharedRef<ITableRow> SCFVehicleAuthoringTab::GenerateVehicleRow(TSharedPtr<FCFVehicleListEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
		// Row에 표시할 Definition 또는 Mesh-only Candidate identity입니다.
	const FString DefinitionName = Item.IsValid()
		? (Item->bMeshOnlyCandidate ? Item->ChassisMeshPath.GetAssetName() : Item->DefinitionPath.GetAssetName())
		: TEXT("<Invalid>");
	// Row management state입니다.
		const FString ManageState = Item.IsValid() && Item->bMeshOnlyCandidate
		? TEXT("차체 메시 후보")
		: (Item.IsValid() && Item->RecipePath.IsValid() ? TEXT("관리됨") : TEXT("미관리"));
	return SNew(STableRow<TSharedPtr<FCFVehicleListEntry>>, OwnerTable)
	[
		SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%s\n%s"), *DefinitionName, *ManageState)))
	];
}

// Browser row 선택을 ViewModel current single-Vehicle selection으로 반영합니다.
void SCFVehicleAuthoringTab::HandleVehicleSelectionChanged(TSharedPtr<FCFVehicleListEntry> Item, ESelectInfo::Type SelectInfo)
{
	if (!Item.IsValid() || !ViewModel.IsValid())
	{
		return;
	}
		// Selection/initial preview diagnostic입니다.
	FString SelectionError;
	if (!ViewModel->SelectVehicle(*Item, SelectionError))
	{
		return;
	}
		SyncEditableFieldsFromSelection();
	RefreshDriftGuidanceIfNeeded();
	if (ImportNameTextBox.IsValid() && !ViewModel->HasRecipe() && !ViewModel->IsMeshOnlyCandidate())
	{
		// 미관리 VehicleData 선택이 바뀔 때마다 현재 차량 identity에 맞는 새 Recipe 이름을 제안합니다.
		ImportNameTextBox->SetText(FText::FromString(CFVehicleAuthoringTabPrivate::BuildSuggestedRecipeName(ViewModel->GetSelectedEntry())));
	}
}

// Search text를 facade Browser filter에 반영합니다.
void SCFVehicleAuthoringTab::HandleSearchChanged(const FText& NewText)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	// Filtered facade Browser read diagnostic입니다.
	FString SearchError;
	ViewModel->RefreshBrowser(NewText.ToString(), SearchError);
	RebuildBrowserRows();
}

// Browser cache를 fresh facade result로 교체하고 list widget을 갱신합니다.
FReply SCFVehicleAuthoringTab::HandleRefreshBrowser()
{
	// Current search filter text입니다.
	const FString SearchText = SearchBox.IsValid() ? SearchBox->GetText().ToString() : FString();
	// Browser facade read diagnostic입니다.
	FString BrowserError;
	ViewModel->RefreshBrowser(SearchText, BrowserError);
	RebuildBrowserRows();
	return FReply::Handled();
}

// Current selection Resolve/Diff/Trace/Validation을 fresh facade result로 갱신합니다.
FReply SCFVehicleAuthoringTab::HandleRefreshPreview()
{
	if (ViewModel.IsValid() && ViewModel->IsMeshOnlyCandidate())
	{
		return FReply::Handled();
	}
	// Facade refresh diagnostic입니다.
	FString RefreshError;
	if (ViewModel.IsValid() && ViewModel->RefreshPreview(RefreshError))
	{
		RefreshDriftGuidanceIfNeeded();
	}
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Fresh preview에서 External Drift가 확인되면 read-only 3-way를 준비하고 Sync Context로 안내합니다.
void SCFVehicleAuthoringTab::RefreshDriftGuidanceIfNeeded()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !ViewModel->HasExternalDrift())
	{
		return;
	}
	// External Drift 상세 비교를 자동 준비하는 read-only diagnostic입니다.
	FString DriftReviewError;
	ViewModel->RefreshDriftReview(DriftReviewError);
	SetContextPage(ECFVehicleContextPage::Sync);
}

// Recipe Archetype text commit을 reviewed R1 facade transaction으로 반영합니다.
void SCFVehicleAuthoringTab::HandleArchetypeCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType != ETextCommit::OnEnter || !ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return;
	}
	// Reviewed semantic Recipe commit result입니다.
	FCFAuthoringOpResult CommitResult;
	ViewModel->CommitArchetypeIntent(FName(*NewText.ToString().TrimStartAndEnd()), CommitResult);
	SyncEditableFieldsFromSelection();
}

// Unmanaged target의 제작 관리 시작 proposal을 review한 뒤 R2 facade commit을 실행합니다.
FReply SCFVehicleAuthoringTab::HandleInitialImport()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	// Requested Recipe package folder입니다.
	const FString PackageFolder = ImportFolderTextBox.IsValid() ? ImportFolderTextBox->GetText().ToString().TrimStartAndEnd() : FString();
	// Requested Recipe asset name입니다.
	const FName AssetName = ImportNameTextBox.IsValid() ? FName(*ImportNameTextBox->GetText().ToString().TrimStartAndEnd()) : NAME_None;
	if (PackageFolder.IsEmpty() || AssetName.IsNone())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(TEXT("제작 관리 시작에는 '레시피 저장 폴더'와 '새 레시피 이름'이 필요합니다.\n\n차량을 다시 선택하면 기본 레시피 이름을 자동으로 제안합니다.")));
		return FReply::Handled();
	}

	// Mutation0 제작 관리 시작 proposal입니다.
	FCFVehicleInitialImportPreviewResult Preview;
	if (!ViewModel->BuildInitialImportPreview(PackageFolder, AssetName, Preview))
	{
		// Preview 실패를 무반응처럼 숨기지 않고 사용자에게 즉시 표시할 메시지입니다.
		const FString PreviewError = ViewModel->GetLastMessage().IsEmpty()
			? TEXT("제작 관리 시작 검토를 준비하지 못했습니다.")
			: ViewModel->GetLastMessage();
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(FString::Printf(TEXT("제작 관리 시작 검토를 열 수 없습니다.\n\n%s"), *PreviewError)));
		return FReply::Handled();
	}

	// Explicit R2 review dialog message입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("제작 관리 시작 검토\n\n기존 차량 데이터는 변경하지 않습니다.\n새로 만들 레시피: %s\n현재 차량 데이터 식별값: %s\n현재 값을 그대로 보존하는 항목: %d\n숨김 상태로 보존하는 항목: %d\n레시피에서 편집 가능한 후보 항목: %d\n자동 저장: 안 함\n\n이 차량을 제작 관리 대상으로 등록하고 새 레시피를 만들까요?"),
		*Preview.ProspectiveRecipePath.ToString(),
		*CFVehicleAuthoringTabPrivate::ShortHash(Preview.Proposal.ExpectedTargetDefinitionHash),
		Preview.ImportSummary.LegacyPinnedFieldCount,
		Preview.ImportSummary.LegacySerializedFieldCount,
		Preview.ImportSummary.SemanticCandidateFieldCount));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	// Explicit approved 제작 관리 시작 commit result입니다.
	FCFVehicleInitialImportResult ImportResult;
	if (!ViewModel->CommitInitialImport(PackageFolder, AssetName, ImportResult))
	{
		// Commit 단계의 stale/conflict/failure도 조용히 삼키지 않는 사용자 오류 메시지입니다.
		const FString CommitError = ImportResult.Operation.Message.IsEmpty()
			? TEXT("새 레시피를 만들지 못했습니다.")
			: ImportResult.Operation.Message;
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(FString::Printf(TEXT("제작 관리 시작에 실패했습니다.\n\n%s\n\n차량 데이터는 변경하지 않았습니다."), *CommitError)));
		return FReply::Handled();
	}
	SyncEditableFieldsFromSelection();
	HandleRefreshBrowser();
	return FReply::Handled();
}

// Current fresh Diff를 reviewed R3 approval로 준비하고 shared Apply lane을 실행합니다.
FReply SCFVehicleAuthoringTab::HandleApply()
{
	if (!ViewModel.IsValid() || !ViewModel->CanApply())
	{
		return FReply::Handled();
	}
	// Keep Authoring review를 거친 External Drift 복구 Apply인지 구분합니다.
	const bool bIsReviewedExternalDriftRecovery = ViewModel->HasExternalDrift() && ViewModel->HasAcceptedDriftKeep();
	// Exact fresh R3 approval preparation result입니다.
	FCFAuthoringOpResult PrepareResult;
	if (!ViewModel->PrepareApply(PrepareResult))
	{
		return FReply::Handled();
	}

	// Final review에서 한 번에 펼쳐 보여줄 최대 변경 상세 건수입니다.
	constexpr int32 MaxReviewDiffRows = 8;
	// PrepareApply 직후에도 authority가 되는 fresh FieldDiff 배열입니다.
	const TArray<FCFVehicleFieldDiff>& FreshDiffs = ViewModel->GetDiffResult().FieldDiff;
	// Final review에 넣을 bounded field/current/apply-after 상세 문자열입니다.
	FString DiffDetailText;
	// 실제 dialog에 펼칠 bounded diff row 수입니다.
	const int32 VisibleDiffCount = FMath::Min(FreshDiffs.Num(), MaxReviewDiffRows);
	for (int32 DiffIndex = 0; DiffIndex < VisibleDiffCount; ++DiffIndex)
	{
		// Final review 한 줄의 exact fresh field diff입니다.
		const FCFVehicleFieldDiff& Diff = FreshDiffs[DiffIndex];
		DiffDetailText += FString::Printf(
			TEXT("%d. %s\n   현재: %s\n   적용 후: %s\n"),
			DiffIndex + 1,
			*Diff.FieldPath.ToCanonicalString(true),
			Diff.bHasBeforeValue ? *Diff.BeforeValue.CanonicalValueText : TEXT("<없음>"),
			Diff.bHasAfterValue ? *Diff.AfterValue.CanonicalValueText : TEXT("<없음>"));
	}
	if (FreshDiffs.Num() > VisibleDiffCount)
	{
		// Bounded dialog가 생략한 나머지 exact 변경 개수입니다.
		const int32 RemainingDiffCount = FreshDiffs.Num() - VisibleDiffCount;
		DiffDetailText += FString::Printf(TEXT("... 외 %d건 — 전체 내용은 오른쪽 '변경점'에서 확인할 수 있습니다.\n"), RemainingDiffCount);
	}
	if (DiffDetailText.IsEmpty())
	{
		DiffDetailText = TEXT("표시할 fresh 변경 상세가 없습니다.");
	}

	// Explicit Definition Apply review dialog입니다.
	const FText ReviewText = bIsReviewedExternalDriftRecovery
		? FText::FromString(FString::Printf(
			TEXT("제작 기준값 복구 적용 검토\n\n처리 방향: 제작 기준값 유지\n복구할 변경: %d개\n경고: %d개\n대상: %s\n자동 저장: 안 함\n\n복구 상세:\n%s\n검토한 제작 기준값을 차량 데이터에 복구 적용하시겠습니까?"),
			ViewModel->GetPendingDiffCount(),
			ViewModel->GetWarningCount(),
			*ViewModel->GetSelectedEntry().DefinitionPath.GetAssetName(),
			*DiffDetailText))
		: FText::FromString(FString::Printf(
			TEXT("차량 데이터 적용 검토\n\n적용할 변경: %d개\n경고: %d개\n외부 변경 감지: %s\n대상: %s\n자동 저장: 안 함\n\n변경 상세:\n%s\n검토한 변경을 차량 데이터에 적용하시겠습니까?"),
			ViewModel->GetPendingDiffCount(),
			ViewModel->GetWarningCount(),
			CFVehicleAuthoringTabPrivate::BooleanText(ViewModel->HasExternalDrift()),
			*ViewModel->GetSelectedEntry().DefinitionPath.GetAssetName(),
			*DiffDetailText));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	// Shared facade Apply terminal result입니다.
	FCFAuthoringOpResult ApplyResult;
	if (!ViewModel->ExecutePreparedApply(ApplyResult))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ApplyResult.Message));
		if (bIsReviewedExternalDriftRecovery)
		{
			SetContextPage(ECFVehicleContextPage::Sync);
		}
		return FReply::Handled();
	}
	if (bIsReviewedExternalDriftRecovery)
	{
		SetContextPage(ECFVehicleContextPage::Sync);
	}
	return FReply::Handled();
}

// Current Target VehicleData를 Unreal 표준 Raw Asset Editor에 엽니다.
FReply SCFVehicleAuthoringTab::HandleOpenRawDA()
{
	// Raw editor navigation diagnostic입니다.
	FString OpenError;
	if (ViewModel.IsValid())
	{
		ViewModel->OpenRawVehicleData(OpenError);
	}
	return FReply::Handled();
}

// Center page selection을 바꿉니다.
FReply SCFVehicleAuthoringTab::SetMainPage(ECFVehicleAuthoringPage NewPage)
{
	CurrentMainPage = NewPage;
	if (MainPageSwitcher.IsValid())
	{
		MainPageSwitcher->SetActiveWidgetIndex(static_cast<int32>(NewPage));
	}
	return FReply::Handled();
}

// Context page selection을 바꿉니다.
FReply SCFVehicleAuthoringTab::SetContextPage(ECFVehicleContextPage NewPage)
{
	CurrentContextPage = NewPage;
	if (ContextPageSwitcher.IsValid())
	{
		ContextPageSwitcher->SetActiveWidgetIndex(static_cast<int32>(NewPage));
	}
	return FReply::Handled();
}

// Header의 current selection/state summary text를 만듭니다.
FText SCFVehicleAuthoringTab::GetHeaderText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
				return FText::FromString(TEXT("차량 데이터 제작 — 왼쪽 차량 목록에서 작업할 차량을 선택하세요."));
	}
		if (ViewModel->IsMeshOnlyCandidate())
	{
				return FText::FromString(FString::Printf(TEXT("차량 데이터 제작 — 차체 메시 후보\n메시: %s\n차량 데이터: 아직 생성되지 않음\n레시피: 아직 생성되지 않음"), *ViewModel->GetSelectedEntry().ChassisMeshPath.ToString()));
	}
	// Selected Definition path입니다.
	const FString DefinitionPath = ViewModel->GetSelectedEntry().DefinitionPath.ToString();
		// Selected Recipe path입니다.
	const FString RecipePath = ViewModel->HasRecipe() ? ViewModel->GetSelectedEntry().RecipePath.ToString() : TEXT("<none>");
	if (ViewModel->HasRecipe() && !ViewModel->IsPreviewFresh())
	{
		return FText::FromString(FString::Printf(
			TEXT("%s\n차량 데이터: %s\n레시피: %s\n관리 상태: %s | 동기화: %s | 검증: 재계산 필요 | 적용 대기 변경: 재계산 필요"),
			*ViewModel->GetSelectedEntry().DefinitionPath.GetAssetName(),
			*DefinitionPath,
			*RecipePath,
			*CFVehicleAuthoringTabPrivate::ManageText(ViewModel->GetManagementView()),
			*CFVehicleAuthoringTabPrivate::SyncText(ViewModel->GetSyncView())));
	}
	return FText::FromString(FString::Printf(
				TEXT("%s\n차량 데이터: %s\n레시피: %s\n관리 상태: %s | 동기화: %s | 검증: %s | 적용 대기 변경: %d"),
		*ViewModel->GetSelectedEntry().DefinitionPath.GetAssetName(),
		*DefinitionPath,
		*RecipePath,
		*CFVehicleAuthoringTabPrivate::ManageText(ViewModel->GetManagementView()),
		*CFVehicleAuthoringTabPrivate::SyncText(ViewModel->GetSyncView()),
		*CFVehicleAuthoringTabPrivate::ValidationText(ViewModel->GetValidationView()),
		ViewModel->GetPendingDiffCount()));
}

// Center Overview/Resolve Preview text를 만듭니다.
FText SCFVehicleAuthoringTab::GetOverviewText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
				return FText::FromString(TEXT("왼쪽 차량 목록에서 차량 데이터 또는 차체 메시 후보를 선택하세요."));
	}
		if (ViewModel->IsMeshOnlyCandidate())
	{
				return FText::FromString(TEXT("이 항목은 아직 차량 데이터가 없는 차체 메시 후보입니다. '레시피 / 프로필'에서 차량 데이터 + 레시피 생성을 검토할 수 있습니다."));
	}
				if (!ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("이미 존재하는 차량 데이터이지만 아직 제작용 레시피가 연결되지 않은 차량입니다.\n\n권장 순서:\n1. 원본 차량 데이터 확인\n2. '레시피 / 프로필'에서 제작 관리 시작\n3. 현재 차량 데이터 값을 기준으로 새 레시피 생성\n4. 미리보기 새로고침\n5. 변경점 / 값 출처 / 검증 확인\n6. 필요한 변경만 차량 데이터에 적용"));
	}
	if (!ViewModel->IsPreviewFresh())
	{
		return FText::FromString(TEXT("해석 미리보기가 오래되었거나 아직 준비되지 않았습니다.\n\n'미리보기 새로고침'을 실행한 뒤 변경점 → 값 출처 → 검증을 같은 최신 상태에서 확인하세요."));
	}
	// Current facade resolve result입니다.
	const FCFVehicleResolveReadResult& Resolve = ViewModel->GetResolveResult();
	return FText::FromString(FString::Printf(
				TEXT("해석 미리보기\n\n레시피 식별값: %s\n값 출처 서명: %s\n현재 차량 데이터 해시: %s\n예상 결과 해시: %s\n해석기 버전: %d\n적용 대기 변경: %d\n경고: %d\n차단 문제: %d\n외부 변경 감지: %s\n\n미리보기만으로 차량 데이터는 변경되지 않습니다."),
		*CFVehicleAuthoringTabPrivate::ShortHash(Resolve.ResolveRequest.Recipe.RecipeFingerprint),
		*CFVehicleAuthoringTabPrivate::ShortHash(Resolve.ResolveResult.SourceSignature),
		*CFVehicleAuthoringTabPrivate::ShortHash(Resolve.ResolveRequest.CurrentDefinition.DefinitionHash),
		*CFVehicleAuthoringTabPrivate::ShortHash(Resolve.ResolveResult.ResolvedDefinitionHash),
		Resolve.ResolveResult.ResolverContractRevision,
		ViewModel->GetPendingDiffCount(),
				ViewModel->GetWarningCount(),
		ViewModel->GetBlockingIssueCount(),
		CFVehicleAuthoringTabPrivate::BooleanText(ViewModel->HasExternalDrift())));
}

// Recipe/Profile basic summary text를 만듭니다.
FText SCFVehicleAuthoringTab::GetRecipeText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
		return FText::FromString(TEXT("차량을 선택하세요."));
	}
		if (ViewModel->IsMeshOnlyCandidate())
	{
				return FText::FromString(TEXT("레시피 없음 — 차체 메시 후보입니다. 아래 '메시에서 차량 만들기'에서 차량 데이터 + 레시피 생성을 검토하세요."));
	}
		if (!ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("레시피 없음 — 이 차량 데이터를 제작 관리 대상으로 사용하려면 아래 '제작 관리 시작'에서 편집용 레시피를 만드세요."));
	}
	// Facade context result입니다.
	const FCFVehicleContextReadResult& Context = ViewModel->GetContextResult();
	return FText::FromString(FString::Printf(
				TEXT("레시피 기본 정보\n레시피 ID: %s\n관리 상태: %s\n레거시 고정값: %d\n숨김 보존값: %d\n고급 덮어쓰기: %d\n\n아래에서 차량 유형과 공유 프로필을 편집할 수 있습니다. 다른 설정은 상단의 에셋/배치, 주행감, 장착/기본값 페이지를 사용하세요."),
		*Context.RecipeId.ToString(EGuidFormats::DigitsWithHyphens),
		*CFVehicleAuthoringTabPrivate::ManageText(ViewModel->GetManagementView()),
		Context.LegacyPinnedFieldCount,
		Context.LegacySerializedFieldCount,
		Context.AdvancedOverrideCount));
}

// Pending Authoring Changes authority table을 text presentation으로 만듭니다.
FText SCFVehicleAuthoringTab::GetDiffText() const
{
	if (!ViewModel.IsValid() || !ViewModel->IsPreviewFresh())
	{
				return FText::FromString(TEXT("최신 해석 미리보기가 없습니다. '미리보기 새로고침'을 먼저 실행하세요."));
	}
	// Shared Resolver FieldDiff authority입니다.
	const TArray<FCFVehicleFieldDiff>& Diffs = ViewModel->GetDiffResult().FieldDiff;
	if (Diffs.IsEmpty())
	{
				return FText::FromString(TEXT("적용 대기 변경: 0\n현재 차량 데이터와 제작 예상 결과가 같습니다."));
	}
	// Full deterministic diff presentation입니다.
		FString Text = FString::Printf(TEXT("적용 대기 변경: %d\n변경 묶음 식별값: %s\n\n"), Diffs.Num(), *CFVehicleAuthoringTabPrivate::ShortHash(ViewModel->GetDiffResult().DiffHash));
	for (const FCFVehicleFieldDiff& Diff : Diffs)
	{
		Text += FString::Printf(
						TEXT("[%s] %s\n  현재: %s\n  적용 후: %s\n\n"),
			*CFVehicleAuthoringTabPrivate::DiffOperationText(Diff.Operation),
			*Diff.FieldPath.ToCanonicalString(true),
			Diff.bHasBeforeValue ? *Diff.BeforeValue.CanonicalValueText : TEXT("<none>"),
			Diff.bHasAfterValue ? *Diff.AfterValue.CanonicalValueText : TEXT("<none>"));
	}
	return FText::FromString(Text);
}

// Validation layer issue들을 persistent text presentation으로 만듭니다.
FText SCFVehicleAuthoringTab::GetValidationText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
				return FText::FromString(TEXT("관리 중인 레시피 차량을 선택하면 레시피 / 해석 결과 / 차량 데이터 검증 문제가 표시됩니다."));
	}
	if (!ViewModel->IsPreviewFresh())
	{
		return FText::FromString(TEXT("최신 미리보기가 없어 검증 결과를 표시할 수 없습니다. '미리보기 새로고침'을 먼저 실행하세요."));
	}
	// Fresh facade Validation projection입니다.
	const FCFVehicleValidationReadResult& Validation = ViewModel->GetValidationResult();
		// Layer별 issue를 사람이 의미 단위로 끊어 읽을 수 있게 append하는 local helper입니다.
	auto AppendIssues = [](FString& InOutText, const TCHAR* LayerName, const TArray<FCFVehicleValidationIssue>& Issues)
	{
		InOutText += FString::Printf(TEXT("[%s] %d\n\n"), LayerName, Issues.Num());
		for (int32 IssueIndex = 0; IssueIndex < Issues.Num(); ++IssueIndex)
		{
			// 현재 표시할 검증 이슈입니다.
			const FCFVehicleValidationIssue& Issue = Issues[IssueIndex];
			// 비어 있지 않을 때만 별도 필드 줄을 표시하기 위한 canonical field path입니다.
			const FString FieldPathText = Issue.FieldPath.ToCanonicalString(true);
			// 원본 검증 메시지는 바꾸지 않고 UI presentation에서만 문장 사이를 줄바꿈한 문자열입니다.
			FString MessageText = Issue.Message;
			MessageText.ReplaceInline(TEXT(". "), TEXT(".\n    "));

			InOutText += FString::Printf(
				TEXT("이슈 %d\n  심각도: %s\n  코드: %s\n"),
				IssueIndex + 1,
				*CFVehicleAuthoringTabPrivate::SeverityText(Issue.Severity),
				*Issue.IssueCode.ToString());
			if (!FieldPathText.IsEmpty())
			{
				InOutText += FString::Printf(TEXT("  필드: %s\n"), *FieldPathText);
			}
			InOutText += FString::Printf(TEXT("  내용:\n    %s\n\n"), *MessageText);
		}
	};
	// Validation full presentation입니다.
	FString Text;
		AppendIssues(Text, TEXT("레시피"), Validation.RecipeValidation);
	AppendIssues(Text, TEXT("해석 결과"), Validation.ResolverValidation);
	AppendIssues(Text, TEXT("차량 데이터 검증"), Validation.DefinitionValidation);
	Text += FString::Printf(TEXT("요약: 경고=%d / 차단=%d / 오류=%d"), Validation.Operation.ValidationSummary.WarningCount, Validation.Operation.ValidationSummary.BlockedCount, Validation.Operation.ValidationSummary.ErrorCount);
	return FText::FromString(Text);
}

// Current right Changes context text를 만듭니다.
FText SCFVehicleAuthoringTab::GetChangesContextText() const
{
	return GetDiffText();
}

// Current Source Trace stack text를 검색/상세 표시 상태에 맞춰 만듭니다.
FText SCFVehicleAuthoringTab::GetSourceTraceText() const
{
	if (!ViewModel.IsValid() || !ViewModel->IsPreviewFresh())
	{
		return FText::FromString(TEXT("최신 미리보기가 없어 값 출처를 표시할 수 없습니다. '미리보기 새로고침'을 먼저 실행하세요."));
	}
	// Resolver Source Trace authority입니다.
	const TArray<FCFVehicleSourceTrace>& Traces = ViewModel->GetTraceResult().SourceTrace;
	// 현재 검색 조건과 일치해 화면에 표시된 Trace 수입니다.
	int32 VisibleTraceCount = 0;
	// 검색 조건을 적용한 source stack presentation입니다.
	FString TraceBodyText;
	for (const FCFVehicleSourceTrace& Trace : Traces)
	{
		// Stable Field Path 전체 문자열입니다.
		const FString CanonicalFieldPath = Trace.FieldPath.ToCanonicalString(true);
		// Field Path와 모든 Source layer를 함께 검색하기 위한 비교 문자열입니다.
		FString SearchableTraceText = CanonicalFieldPath;
				for (const FCFVehicleSourceLayer& Layer : Trace.Layers)
		{
			SearchableTraceText += TEXT(" ");
			SearchableTraceText += CFVehicleAuthoringTabPrivate::SourceTypeText(Layer.SourceType);
			SearchableTraceText += TEXT(" ");
			SearchableTraceText += Layer.SourceId;
		}
		if (!SourceTraceSearchText.IsEmpty() && !SearchableTraceText.Contains(SourceTraceSearchText, ESearchCase::IgnoreCase))
		{
			continue;
		}

		++VisibleTraceCount;
		TraceBodyText += FString::Printf(TEXT("%s\n"), *CanonicalFieldPath);
		if (Trace.Layers.IsValidIndex(Trace.EffectiveLayerIndex))
		{
			// Resolver가 실제 적용으로 판정한 source layer입니다.
			const FCFVehicleSourceLayer& EffectiveLayer = Trace.Layers[Trace.EffectiveLayerIndex];
			TraceBodyText += FString::Printf(
				TEXT("  실제 적용: %s — %s\n"),
				*CFVehicleAuthoringTabPrivate::SourceTypeText(EffectiveLayer.SourceType),
				*EffectiveLayer.SourceId);
		}
		else
		{
			TraceBodyText += TEXT("  실제 적용: 없음\n");
		}

		if (bShowSourceTraceCandidates)
		{
			for (int32 LayerIndex = 0; LayerIndex < Trace.Layers.Num(); ++LayerIndex)
			{
				if (LayerIndex == Trace.EffectiveLayerIndex)
				{
					continue;
				}
				// 실제 적용되지 않았지만 Resolver가 추적한 후보 source layer입니다.
				const FCFVehicleSourceLayer& CandidateLayer = Trace.Layers[LayerIndex];
				TraceBodyText += FString::Printf(
					TEXT("  비적용 후보: %s — %s\n"),
					*CFVehicleAuthoringTabPrivate::SourceTypeText(CandidateLayer.SourceType),
					*CandidateLayer.SourceId);
			}
		}
		TraceBodyText += TEXT("\n");
	}

	// 검색과 상세 표시 상태를 한눈에 확인할 수 있는 summary입니다.
	FString Text = FString::Printf(
		TEXT("전체 추적 항목: %d | 현재 표시: %d\n기본 화면은 실제 적용 출처만 표시합니다. 필요한 필드를 검색하고, 출처 경쟁 관계를 확인할 때만 '비적용 후보까지 보기'를 켜세요.\n\n"),
		Traces.Num(),
		VisibleTraceCount);
	if (!SourceTraceSearchText.IsEmpty())
	{
		Text += FString::Printf(TEXT("검색: %s\n\n"), *SourceTraceSearchText);
	}
	if (VisibleTraceCount == 0)
	{
		Text += TEXT("검색 조건과 일치하는 값 출처가 없습니다.");
		return FText::FromString(Text);
	}
	Text += TraceBodyText;
	return FText::FromString(Text);
}

// Current right Issues context text를 만듭니다.
FText SCFVehicleAuthoringTab::GetIssuesContextText() const
{
	return GetValidationText();
}

// Current stale/drift/freshness sync text를 만듭니다.
FText SCFVehicleAuthoringTab::GetSyncContextText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
		return FText::FromString(TEXT("차량을 선택하세요."));
	}
	if (ViewModel->IsMeshOnlyCandidate())
	{
		return FText::FromString(TEXT("차체 메시 후보 / 차량 데이터 없음\n'메시에서 차량 만들기' 전에는 적용 기준값, 검증, 외부 변경 감지, 적용 대상이 없습니다."));
	}
		if (!ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("적용 기준 없음 / 미관리\n'제작 관리 시작'으로 레시피를 만들기 전에는 제작 관리 기준값과 값 출처 기록이 없습니다."));
	}
	if (!ViewModel->IsPreviewFresh())
	{
		return FText::FromString(TEXT("현재 미리보기가 오래되었습니다. 이전 동기화/외부 변경 정보는 현재 판단에 사용하지 않습니다.\n\n'미리보기 새로고침'으로 현재 레시피와 차량 데이터를 다시 읽어주세요."));
	}
	// Current R16 stale report입니다.
	const FCFVehicleStaleReport& Stale = ViewModel->GetResolveResult().ResolveResult.StaleReport;
	if (Stale.bHasExternalDrift)
	{
		// Macro-flow 첫 화면에 표시할 External Drift 요약입니다.
		FString Text = FString::Printf(TEXT("외부 변경 %d건 발견\n\n차량 데이터 원본이 마지막 제작 기준값 적용 이후 바뀌었습니다. 아래에서 처리 방향을 선택하기 전에는 변경 적용이 차단됩니다.\n\n"), Stale.Fields.Num());
		// 자동 준비된 read-only 3-way 비교입니다.
		const FCFVehicleDriftReviewResult& Review = ViewModel->GetDriftReview();
		for (const FCFVehicleDriftReviewRow& Row : Review.Rows)
		{
			Text += FString::Printf(
				TEXT("- %s: 제작 기준값 %s → 현재 원본 %s\n"),
				*CFVehicleAuthoringTabPrivate::FieldDisplayText(Row.FieldPath),
				*Row.CurrentAuthoringValue.CanonicalValueText,
				*Row.CurrentRawValue.CanonicalValueText);
		}
		if (Review.Rows.IsEmpty())
		{
			Text += TEXT("- 변경값 상세 비교를 아직 준비하지 못했습니다. 미리보기 새로고침으로 현재 값을 다시 읽어주세요.\n");
		}
		Text += ViewModel->HasAcceptedDriftKeep()
			? TEXT("\n처리 방향: 제작 기준값을 유지하기로 승인했습니다.\n다음 단계: '제작 기준값으로 복구 적용…'을 눌러 차량 데이터에 복구하세요.\n")
			: TEXT("\n권장 처리: '제작 기준값으로 복구…'에서 관리 기준값을 유지할지 먼저 검토하세요.\n다른 소유권 처리는 '다른 처리 방법'에서 선택할 수 있습니다.\n");
		Text += TEXT("자동 저장은 하지 않습니다. 정확한 마지막 적용값 / 현재 원본값 / 현재 제작 기준값은 '상세 비교'에서 확인할 수 있습니다.");
		return FText::FromString(Text);
	}
	return FText::FromString(FString::Printf(
		TEXT("미리보기: %s\n동기화: %s\n실제 적용값 오래됨: %s\n비적용 출처 변경: %s\n외부 변경 감지: 없음\n영향 항목: %d"),
		*CFVehicleAuthoringTabPrivate::PreviewText(ViewModel->GetPreviewView()),
		*CFVehicleAuthoringTabPrivate::SyncText(ViewModel->GetSyncView()),
		CFVehicleAuthoringTabPrivate::BooleanText(Stale.bHasEffectiveStale),
		CFVehicleAuthoringTabPrivate::BooleanText(Stale.bHasShadowSourceChange),
		Stale.Fields.Num()));
}

// Bottom Action Bar summary text를 만듭니다.
FText SCFVehicleAuthoringTab::GetBottomStatusText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
				return FText::FromString(TEXT("미리보기: 차량 선택 필요"));
	}
				if (ViewModel->IsMeshOnlyCandidate())
	{
				return FText::FromString(FString::Printf(TEXT("차체 메시 후보 | %s | 차량 데이터/레시피 아직 생성되지 않음"), *ViewModel->GetSelectedEntry().ChassisMeshPath.GetAssetName()));
	}
	if (ViewModel->HasRecipe() && !ViewModel->IsPreviewFresh())
	{
		return FText::FromString(FString::Printf(TEXT("미리보기: %s | 변경/검증 재계산 필요 | 대상: %s"), *CFVehicleAuthoringTabPrivate::PreviewText(ViewModel->GetPreviewView()), *ViewModel->GetSelectedEntry().DefinitionPath.GetAssetName()));
	}
	return FText::FromString(FString::Printf(
				TEXT("미리보기: %s | 변경: %d | 경고: %d | 차단: %d | 외부 변경: %s | 대상: %s"),
		*CFVehicleAuthoringTabPrivate::PreviewText(ViewModel->GetPreviewView()),
		ViewModel->GetPendingDiffCount(),
				ViewModel->GetWarningCount(),
		ViewModel->GetBlockingIssueCount(),
		CFVehicleAuthoringTabPrivate::BooleanText(ViewModel->HasExternalDrift()),
		*ViewModel->GetSelectedEntry().DefinitionPath.GetAssetName()));
}

// Persistent operation result/status message를 반환합니다.
FText SCFVehicleAuthoringTab::GetOperationMessageText() const
{
	return FText::FromString(ViewModel.IsValid() ? ViewModel->GetLastMessage() : FString());
}

// Current Apply button label을 pending count/warnings와 함께 만듭니다.
FText SCFVehicleAuthoringTab::GetApplyButtonText() const
{
	if (!ViewModel.IsValid())
	{
				return FText::FromString(TEXT("변경 적용"));
	}
	return FText::FromString(FString::Printf(TEXT("변경 %d개 적용"), ViewModel->GetPendingDiffCount()));
}

// Normal Apply 활성 조건을 ViewModel에서 읽습니다.
bool SCFVehicleAuthoringTab::IsApplyEnabled() const
{
	return ViewModel.IsValid() && ViewModel->CanApply();
}

// Managed Recipe editor section visibility입니다.
EVisibility SCFVehicleAuthoringTab::GetManagedVisibility() const
{
	return ViewModel.IsValid() && ViewModel->HasRecipe() ? EVisibility::Visible : EVisibility::Collapsed;
}

// Unmanaged Initial Import section visibility입니다.
EVisibility SCFVehicleAuthoringTab::GetUnmanagedVisibility() const
{
		return ViewModel.IsValid() && ViewModel->HasSelection() && !ViewModel->HasRecipe() && !ViewModel->IsMeshOnlyCandidate() ? EVisibility::Visible : EVisibility::Collapsed;
}

// Managed/PartiallyManaged Raw DA escape hatch warning text를 반환합니다.
FText SCFVehicleAuthoringTab::GetRawDAWarningText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
		return FText::FromString(TEXT("차량을 선택하세요."));
	}
	if (ViewModel->IsMeshOnlyCandidate())
	{
		return FText::FromString(TEXT("차체 메시 후보에는 아직 차량 데이터가 없습니다. '메시에서 차량 만들기' 전에는 원본 데이터 에디터를 열 수 없습니다."));
	}
	if (!ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("원본 차량 데이터 열기\n\n아직 제작 관리 대상이 아닌 차량 데이터입니다. 원본 DataAsset을 확인하는 것만으로 값이 변경되지는 않습니다."));
	}
	return FText::FromString(TEXT("원본 차량 데이터 직접 편집 주의\n\n이 차량은 제작 관리 대상입니다. 원본 차량 데이터를 직접 수정하면 값 출처 추적을 우회하고 다음 새로고침에서 외부 변경으로 감지될 수 있습니다.\n\n원본 수정값이 자동으로 고급 덮어쓰기나 레시피 설정으로 변환되지는 않습니다."));
}

// Browser UObject rows를 Slate shared rows로 재구성합니다.
void SCFVehicleAuthoringTab::RebuildBrowserRows()
{
	BrowserRows.Reset();
		if (ViewModel.IsValid())
	{
		for (const FCFVehicleListEntry& Entry : ViewModel->GetBrowserEntries())
		{
			if (!bShowTechnicalBrowserRecords && CFVehicleAuthoringTabPrivate::IsTechnicalOrRetiredBrowserEntry(Entry))
			{
				continue;
			}
			BrowserRows.Add(MakeShared<FCFVehicleListEntry>(Entry));
		}
	}
	if (VehicleListView.IsValid())
	{
		VehicleListView->RequestListRefresh();
	}
}

// Current selection의 Recipe semantic edit fields를 persistent Recipe value에 맞춥니다.
void SCFVehicleAuthoringTab::SyncEditableFieldsFromSelection()
{
	SyncP11FieldsFromSelection();
	RefreshProfileNumericFieldRows();
	// Current persistent Recipe selection입니다.
	UCFVehicleRecipeData* CurrentRecipe = ViewModel.IsValid() ? ViewModel->GetRecipe() : nullptr;
	if (ArchetypeTextBox.IsValid())
	{
		ArchetypeTextBox->SetText(CurrentRecipe ? FText::FromName(CurrentRecipe->VehicleArchetypeId) : FText::GetEmpty());
	}
			if (!CurrentRecipe)
	{
		if (ChassisMeshTextBox.IsValid())
		{
			ChassisMeshTextBox->SetText(FText::GetEmpty());
		}
		for (const TSharedPtr<SEditableTextBox>& WheelMeshTextBox : WheelMeshTextBoxes)
		{
			if (WheelMeshTextBox.IsValid())
			{
				WheelMeshTextBox->SetText(FText::GetEmpty());
			}
		}
		for (const TSharedPtr<SEditableTextBox>& WheelSocketTextBox : WheelSocketTextBoxes)
		{
			if (WheelSocketTextBox.IsValid())
			{
				WheelSocketTextBox->SetText(FText::GetEmpty());
			}
		}
		if (DestroyedFxSocketTextBox.IsValid())
		{
			DestroyedFxSocketTextBox->SetText(FText::GetEmpty());
		}
		return;
	}

	if (ChassisMeshTextBox.IsValid())
	{
		ChassisMeshTextBox->SetText(FText::FromString(CurrentRecipe->AssetIntent.ChassisMesh.ToSoftObjectPath().ToString()));
	}
	if (WheelMeshTextBoxes.Num() == 4)
	{
		if (WheelMeshTextBoxes[0].IsValid()) WheelMeshTextBoxes[0]->SetText(FText::FromString(CurrentRecipe->AssetIntent.WheelMeshFL.ToSoftObjectPath().ToString()));
		if (WheelMeshTextBoxes[1].IsValid()) WheelMeshTextBoxes[1]->SetText(FText::FromString(CurrentRecipe->AssetIntent.WheelMeshFR.ToSoftObjectPath().ToString()));
		if (WheelMeshTextBoxes[2].IsValid()) WheelMeshTextBoxes[2]->SetText(FText::FromString(CurrentRecipe->AssetIntent.WheelMeshRL.ToSoftObjectPath().ToString()));
		if (WheelMeshTextBoxes[3].IsValid()) WheelMeshTextBoxes[3]->SetText(FText::FromString(CurrentRecipe->AssetIntent.WheelMeshRR.ToSoftObjectPath().ToString()));
	}
	if (WheelSocketTextBoxes.Num() == 4)
	{
		if (WheelSocketTextBoxes[0].IsValid()) WheelSocketTextBoxes[0]->SetText(FText::FromName(CurrentRecipe->AssetIntent.BodyWheelSocketFL));
		if (WheelSocketTextBoxes[1].IsValid()) WheelSocketTextBoxes[1]->SetText(FText::FromName(CurrentRecipe->AssetIntent.BodyWheelSocketFR));
		if (WheelSocketTextBoxes[2].IsValid()) WheelSocketTextBoxes[2]->SetText(FText::FromName(CurrentRecipe->AssetIntent.BodyWheelSocketRL));
		if (WheelSocketTextBoxes[3].IsValid()) WheelSocketTextBoxes[3]->SetText(FText::FromName(CurrentRecipe->AssetIntent.BodyWheelSocketRR));
	}

	PendingAccelerationFeel = CurrentRecipe->DrivingFeelIntent.AccelerationFeel;
	PendingSteeringAgility = CurrentRecipe->DrivingFeelIntent.SteeringAgility;
	PendingGripFeel = CurrentRecipe->DrivingFeelIntent.GripFeel;
	PendingSuspensionFirmness = CurrentRecipe->DrivingFeelIntent.SuspensionFirmness;

	if (DestroyedFxSocketTextBox.IsValid())
	{
		DestroyedFxSocketTextBox->SetText(FText::FromName(CurrentRecipe->DefaultDataIntent.DestroyedFxSocketName));
	}
}
