// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTab.cpp
// Version: v1.9.0
// Date: 2026-08-28
// Description: Guided Vehicle Builder Slate Shell 구현입니다.
// Changelog:
// - v1.9.0: E2E에서 발견된 Step 2 Wheel Mesh 지정 UX 공백을 StaticMesh object picker + explicit typed AssetIntent Recipe-only 반영 UI로 교정.
// - v1.8.0: USER 피드백에 따라 작업 대상 row를 identity 기반 고정 accent 색+관리 상태 배지로 구분하고 Step 3을 필수·선택 Socket 이름/복사/차체 메시 열기까지 포함하는 실제 소켓 준비 단계로 강화.
// - v1.7.0: VB-P0-09 Step 8 existing VB-P0-08 benchmark를 non-blocking child process로 실행/회수하고 active PIE transient test-drive + exact USER Driving PASS UX를 연결.
// - v1.6.0: VB-P0-09 Step 7 existing ReadBuilderFinalReview R0 전체 Diff review, explicit DefinitionApply, exact guarded Undo UX를 Guided Shell에 연결.
// - v1.5.0: VB-P0-09 Step 6 existing ReadBuilderGameplayGuidance R0 결과를 8영역 completeness/USER Socket guidance/pending diff 요약으로 Guided Shell에 노출.
// - v1.4.0: VB-P0-09 Step 5 Physics Proposal summary, AI PhysicsDraft load, private 4 Profile mutation0 review→explicit USER commit UX를 추가.
// - v1.3.0: VB-P0-09 Step 1 Reference summary, AI ResearchDraft load, Companion mutation0 review→explicit USER commit, local Reference review token UX를 추가.
// - v1.2.0: navigation을 current StepViews.Num() 기반으로 만들고 Step 1 전용 UI를 fixed index가 아닌 stable StepId로 판정.
// - v1.1.0: Step 1 Mesh-only 후보에 기존 two-record Preview→explicit USER approval→commit UI를 연결. Profile/차급/물리 추론과 Save는 계속 0.
// - v1.0.0: 별도 Guided Shell에서 차량 선택 → 8 Step 상태 확인 → Back/Refresh/Next를 제공하고 Advanced Workspace는 보존.
// Migration:
// - Step 8 benchmark는 existing RunBuilderBench.ps1/VB-P0-08 authority만 사용하며 saved Target을 요구합니다. USER test-drive는 active PIE transient duplicate만 적용하고 Product Asset 자동 Save/Reference threshold 자동 판정은 하지 않습니다.
// - Step 7 Target mutation은 existing ReadBuilderFinalReview → explicit DefinitionApply → ApplyBuilderFinalReview R3만 사용합니다. Undo는 Apply가 발급한 exact guarded token만 사용하며 auto Save/retry는 없습니다.
// - Step 6은 existing ReadBuilderGameplayGuidance R0만 소비하며 Socket 생성·이동, Target VehicleData Apply, Save를 수행하지 않습니다. USER Socket 위치 authority와 Step 7 Apply ownership을 유지합니다.
// - Step 5 persistent mutation은 existing CommitBuilderProfiles typed facade의 explicit USER-approved R1 transaction만 사용합니다. Target VehicleData Apply는 Step 7에 남기고 raw/shared Profile write와 auto Save는 수행하지 않습니다.
// - Step 1 persistent mutation은 existing CreateBuilderCompanions typed facade의 explicit USER-approved R2 transaction만 사용합니다. raw Profile/Evidence write, VehicleData Apply와 auto Save는 수행하지 않습니다.

#include "DataAuthoring/CFVehicleBuilderTab.h"

#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"

#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/MessageDialog.h"
#include "PropertyCustomizationHelpers.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SCFVehicleBuilderTab"

namespace
{
	// Step state를 USER-facing 한국어로 반환합니다.
	FString BuilderStepStateText(const ECFVehicleBuilderStepState State)
	{
		switch (State)
		{
		case ECFVehicleBuilderStepState::Unavailable: return TEXT("사용 불가");
		case ECFVehicleBuilderStepState::Locked: return TEXT("잠김");
		case ECFVehicleBuilderStepState::Ready: return TEXT("진행 가능");
		case ECFVehicleBuilderStepState::Complete: return TEXT("완료");
		case ECFVehicleBuilderStepState::Blocked: return TEXT("막힘");
		case ECFVehicleBuilderStepState::Stale: return TEXT("다시 확인");
		default: return TEXT("알 수 없음");
		}
	}

	// Vehicle Browser row의 관리 유형을 한눈에 구분할 짧은 배지 문구를 반환합니다.
	FString VehicleRowCategoryText(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return TEXT("MESH 후보");
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed: return TEXT("관리됨");
		case ECFVehicleManageState::PartiallyManaged: return TEXT("부분 관리");
		case ECFVehicleManageState::LegacyImported: return TEXT("레거시");
		case ECFVehicleManageState::Unmanaged: return TEXT("미관리");
		default: return TEXT("기타");
		}
	}

	// Vehicle Browser row마다 새로고침해도 바뀌지 않는 identity 기반 accent 색상을 반환합니다.
	FLinearColor VehicleRowAccentColor(const FCFVehicleListEntry& Entry)
	{
		// Mesh 후보는 Chassis path, managed row는 Definition path를 stable identity로 사용합니다.
		const FString StableIdentity = Entry.bMeshOnlyCandidate
			? Entry.ChassisMeshPath.ToString()
			: Entry.DefinitionPath.ToString();
		// 서로 인접한 row를 육안으로 구분하기 위한 고정 accent palette입니다.
		static const FLinearColor AccentPalette[] =
		{
			FLinearColor(0.35f, 0.75f, 1.00f, 1.0f),
			FLinearColor(0.45f, 0.88f, 0.55f, 1.0f),
			FLinearColor(1.00f, 0.72f, 0.30f, 1.0f),
			FLinearColor(0.85f, 0.52f, 0.95f, 1.0f),
			FLinearColor(0.35f, 0.90f, 0.85f, 1.0f),
			FLinearColor(1.00f, 0.52f, 0.52f, 1.0f),
			FLinearColor(0.72f, 0.72f, 1.00f, 1.0f),
			FLinearColor(0.92f, 0.82f, 0.42f, 1.0f)
		};
		// Stable identity hash를 palette index로 축약한 값입니다.
		const uint32 PaletteIndex = GetTypeHash(StableIdentity) % UE_ARRAY_COUNT(AccentPalette);
		return AccentPalette[PaletteIndex];
	}

	// Vehicle Browser row의 가장 읽기 쉬운 primary identity를 반환합니다.
	FString VehicleRowTitle(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return Entry.ChassisMeshPath.GetAssetName();
		}

		return Entry.DefinitionPath.IsValid()
			? Entry.DefinitionPath.GetAssetName()
			: TEXT("<VehicleData 없음>");
	}

	// Vehicle Browser row의 보조 상태를 반환합니다.
	FString VehicleRowSubtitle(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return Entry.ChassisMeshPath.ToString();
		}

		return Entry.RecipePath.IsValid()
			? FString::Printf(TEXT("Managed | %s"), *Entry.RecipePath.ToString())
			: TEXT("Unmanaged VehicleData");
	}
}

// Widget이 닫힐 때 benchmark child process handle만 해제하고 process 자체는 강제 종료하지 않습니다.
SCFVehicleBuilderTab::~SCFVehicleBuilderTab()
{
	if (DrivingBenchmarkProcess.IsValid())
	{
		FPlatformProcess::CloseProc(DrivingBenchmarkProcess);
		DrivingBenchmarkProcess = FProcHandle();
	}
}

// Non-blocking Technical Benchmark child process terminal 상태를 polling하고 current Step 8 result를 fresh 회수합니다.
void SCFVehicleBuilderTab::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!bDrivingBenchmarkRunning || !DrivingBenchmarkProcess.IsValid())
	{
		return;
	}
	if (FPlatformProcess::IsProcRunning(DrivingBenchmarkProcess))
	{
		return;
	}

	// Terminal child process exit code입니다.
	int32 ExitCode = INDEX_NONE;
	// OS가 exact exit code를 회수했는지 여부입니다.
	const bool bHasExitCode = FPlatformProcess::GetProcReturnCode(DrivingBenchmarkProcess, &ExitCode);
	FPlatformProcess::CloseProc(DrivingBenchmarkProcess);
	DrivingBenchmarkProcess = FProcHandle();
	bDrivingBenchmarkRunning = false;

	// 시작했던 exact Step 8 run identity입니다.
	const FString FinishedRunId = RunningDrivingBenchmarkRunId;
	RunningDrivingBenchmarkRunId.Reset();

	if (!ViewModel.IsValid())
	{
		LastStatusText = LOCTEXT("DrivingBenchmarkNoViewModel", "기술 벤치마크 process는 종료됐지만 Builder ViewModel이 없어 결과를 회수하지 못했습니다.");
		return;
	}

	// Fresh result/current Target binding diagnostic입니다.
	FString RefreshError;
	const bool bRefreshSucceeded = ViewModel->RefreshCurrentState(RefreshError);
	const bool bExactRunMatched = bRefreshSucceeded
		&& ViewModel->HasDrivingBenchmarkResult()
		&& ViewModel->GetDrivingBenchmarkResult().RunId == FinishedRunId;

	if (bHasExitCode && ExitCode == 0 && bExactRunMatched)
	{
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("기술 벤치마크가 완료됐고 current Target에 exact binding됐습니다. RunId=%s. 이제 실제 PIE에서 선택 차량을 주행해 USER 판단을 진행하세요."),
			*FinishedRunId));
		return;
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("기술 벤치마크 결과를 current Step 8로 승인하지 못했습니다. ExitCode=%d | RunId=%s | %s"),
		bHasExitCode ? ExitCode : INDEX_NONE,
		*FinishedRunId,
		RefreshError.IsEmpty() ? TEXT("result identity/status를 확인하세요.") : *RefreshError));
}

// Slate widget tree와 transient Builder ViewModel을 초기화합니다.
void SCFVehicleBuilderTab::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FCFVehicleBuilderVM>();
	// Step 2 FL/FR/RL/RR pending Wheel Mesh picker storage를 고정 4-role로 초기화합니다.
	PendingWheelMeshPaths.SetNum(4);
	LastStatusText = LOCTEXT("InitialStatus", "대상 목록을 읽는 중입니다.");

	ChildSlot
	[
		SNew(SBorder)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "CarFight 차량 제작 가이드"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(
					"Subtitle",
					"실존 차량 Reference → Mesh/Socket → Physics Proposal → Gameplay → Final Review → Driving 순서로 한 단계씩 진행합니다. "
					"Wheel/Hardpoint Socket은 자동 생성하지 않으며, 자동 저장하지 않습니다."))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SSplitter)

				+ SSplitter::Slot()
				.Value(0.27f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 6.0f)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("VehicleListHeader", "1. 작업 대상"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]

					+ SVerticalBox::Slot()
					.FillHeight(0.45f)
					.Padding(0.0f, 0.0f, 6.0f, 10.0f)
					[
						SAssignNew(VehicleListView, SListView<FVehicleRowPtr>)
							.ListItemsSource(&VehicleRows)
							.OnGenerateRow(this, &SCFVehicleBuilderTab::HandleGenerateVehicleRow)
							.OnSelectionChanged(this, &SCFVehicleBuilderTab::HandleVehicleSelectionChanged)
							.SelectionMode(ESelectionMode::Single)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 10.0f)
					[
						SNew(SButton)
							.Text(LOCTEXT("RefreshVehicles", "대상 목록 새로고침"))
							.OnClicked(this, &SCFVehicleBuilderTab::HandleRefreshVehicles)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 6.0f)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("StepHeader", "2. 제작 단계"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]

					+ SVerticalBox::Slot()
					.FillHeight(0.55f)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						BuildStepNavigation()
					]
				]

				+ SSplitter::Slot()
				.Value(0.73f)
				[
					SNew(SBorder)
					.Padding(14.0f)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SNew(STextBlock)
								.Text(this, &SCFVehicleBuilderTab::GetSelectionText)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 12.0f)
						[
							SNew(STextBlock)
								.Text(this, &SCFVehicleBuilderTab::GetCurrentStepTitle)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 12.0f)
						[
							SNew(STextBlock)
								.Text(this, &SCFVehicleBuilderTab::GetCurrentStepStateText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SNew(SScrollBox)

							+ SScrollBox::Slot()
							.Padding(0.0f, 0.0f, 0.0f, 14.0f)
							[
								SNew(STextBlock)
									.Text(this, &SCFVehicleBuilderTab::GetCurrentStepSummaryText)
									.AutoWrapText(true)
							]

							+ SScrollBox::Slot()
							[
								SNew(SBorder)
								.Padding(12.0f)
								[
									SNew(SVerticalBox)

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(STextBlock)
											.Text(LOCTEXT("NextActionHeader", "지금 할 일"))
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
									]

									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.0f, 6.0f, 0.0f, 0.0f)
									[
										SNew(STextBlock)
											.Text(this, &SCFVehicleBuilderTab::GetCurrentStepResolutionText)
											.AutoWrapText(true)
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 12.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetMeshCreationVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("MeshCreateHeader", "VehicleData + Recipe 만들기"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f)
								[
									SAssignNew(NewDefinitionPackageTextBox, SEditableTextBox)
										.HintText(LOCTEXT("DefinitionPackageHint", "VehicleData package: /Game/.../DA_Vehicle_Name"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f)
								[
									SAssignNew(NewDefinitionNameTextBox, SEditableTextBox)
										.HintText(LOCTEXT("DefinitionNameHint", "VehicleData Asset 이름"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f)
								[
									SAssignNew(NewRecipePackageTextBox, SEditableTextBox)
										.HintText(LOCTEXT("RecipePackageHint", "Recipe package: /Game/.../DA_Recipe_Name"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 8.0f)
								[
									SAssignNew(NewRecipeNameTextBox, SEditableTextBox)
										.HintText(LOCTEXT("RecipeNameHint", "Recipe Asset 이름"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SButton)
										.Text(LOCTEXT("CreateFromMesh", "생성 내용 검토"))
										.ToolTipText(LOCTEXT("CreateFromMeshTooltip", "먼저 mutation0 Preview를 만들고, 별도 확인창에서 승인할 때만 VehicleData+Recipe를 생성합니다. 자동 저장하지 않습니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCreateVehicleFromMesh)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetReferenceFlowVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("ReferenceFlowHeader", "Reference Evidence / Companion"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetReferenceSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text_Lambda([this]()
										{
											return ViewModel.IsValid()
												? FText::FromString(FString::Printf(TEXT("AI Draft 경로: %s"), *ViewModel->GetResearchDraftPath()))
												: FText::GetEmpty();
										})
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("LoadResearchDraft", "AI Research Draft 불러오기"))
											.ToolTipText(LOCTEXT("LoadResearchDraftTooltip", "Project Saved의 typed ResearchDraft.json을 current RecipeId/TargetDefinitionPath와 exact 비교해 mutation 없이 읽습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleLoadResearchDraft)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ReviewResearchCompanion", "Companion 생성 내용 검토"))
											.ToolTipText(LOCTEXT("ReviewResearchCompanionTooltip", "먼저 mutation0 R2 proposal을 만들고 확인창에서 승인할 때만 Missing Evidence/private Profile을 생성합니다. 자동 저장/Apply는 하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleResearchCompanionReview)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("AcceptReferenceSet", "이 Reference Set으로 진행"))
											.ToolTipText(LOCTEXT("AcceptReferenceSetTooltip", "Current Evidence 요약과 exact fingerprint를 다시 확인한 뒤 local navigation review token만 기록합니다. Profile/Recipe write, Apply, Save 권한은 부여하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleAcceptReferenceSet)
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetMeshPreparationVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("MeshPreparationHeader", "Mesh 준비 / Assets"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"MeshPreparationIntro",
											"차체와 Wheel StaticMesh를 지정합니다. FL Wheel Mesh는 필수입니다. FR/RL/RR을 비우면 기존 계약대로 FL을 재사용합니다. Builder는 Wheel을 자동 탐지하거나 위치를 계산하지 않습니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 6.0f)
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(0.0f, 0.0f, 8.0f, 0.0f)
									[
										SNew(SBox)
										.WidthOverride(150.0f)
										[
											SNew(STextBlock)
												.Text(LOCTEXT("ChassisMeshPickerLabel", "차체 메시 [필수]"))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
										]
									]

									+ SHorizontalBox::Slot()
									.FillWidth(1.0f)
									[
										SNew(SObjectPropertyEntryBox)
											.AllowedClass(UStaticMesh::StaticClass())
											.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingChassisMeshPath)
											.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleChassisMeshChanged)
											.AllowClear(false)
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildWheelMeshPickerRow(0, LOCTEXT("WheelMeshRoleFL", "FL / 앞왼쪽"), true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildWheelMeshPickerRow(1, LOCTEXT("WheelMeshRoleFR", "FR / 앞오른쪽"), false)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildWheelMeshPickerRow(2, LOCTEXT("WheelMeshRoleRL", "RL / 뒤왼쪽"), false)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildWheelMeshPickerRow(3, LOCTEXT("WheelMeshRoleRR", "RR / 뒤오른쪽"), false)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 10.0f, 0.0f, 8.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("CommitMeshPreparation", "Mesh 설정 반영"))
										.ToolTipText(LOCTEXT("CommitMeshPreparationTooltip", "현재 picker 선택값만 existing typed AssetIntent Recipe-only 경로로 반영합니다. VehicleData Apply와 자동 저장은 수행하지 않습니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCommitMeshPreparation)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"MeshPreparationBoundary",
											"반영 후 Step 2는 fresh AssetSnapshot으로 다시 검사됩니다. FR/RL/RR 선택은 선택사항이며, 비어 있으면 FL Wheel Mesh를 재사용합니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetSocketPreparationVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("SocketPreparationHeader", "소켓 준비 / Naming"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"SocketPreparationIntro",
											"필수 Wheel Socket 4개는 현재 Recipe binding과 이름이 정확히 일치해야 합니다. 위치·회전·스케일은 차체 메시를 보고 USER가 직접 작성합니다. Builder는 Socket을 자동 생성하거나 이동하지 않습니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("OpenCurrentChassisMesh", "차체 메시 열기"))
										.ToolTipText(LOCTEXT("OpenCurrentChassisMeshTooltip", "현재 선택 차량의 Chassis StaticMesh를 Static Mesh 에디터에서 엽니다. 소켓 추가·이동·저장은 USER가 직접 수행합니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenCurrentChassisMesh)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("RequiredSocketHeader", "필수 Wheel Socket — 4개 모두 필요"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(0, LOCTEXT("WheelRoleFL", "FL / 앞왼쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(1, LOCTEXT("WheelRoleFR", "FR / 앞오른쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(2, LOCTEXT("WheelRoleRL", "RL / 뒤왼쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(3, LOCTEXT("WheelRoleRR", "RR / 뒤오른쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 12.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("OptionalSocketHeader", "선택 / 조건부 Socket"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetOptionalSocketGuideText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("CopyOptionalSocketNames", "선택 Socket 이름 전체 복사"))
										.ToolTipText(LOCTEXT("CopyOptionalSocketNamesTooltip", "현재 Recipe가 사용하는 선택/조건부 Hardpoint 및 파괴 FX Socket 이름만 줄바꿈 목록으로 복사합니다."))
										.IsEnabled(this, &SCFVehicleBuilderTab::CanCopyOptionalSocketNames)
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyOptionalSocketNames)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"SocketPreparationBoundary",
											"Socket 편집을 마치고 Static Mesh를 저장한 뒤 아래 '현재 상태 다시 확인'을 누르세요. 선택/조건부 Socket은 현재 Recipe가 실제로 요구할 때만 추가하면 됩니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetPhysicsProposalVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("PhysicsProposalHeader", "Physics Proposal / private 4 Profile"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetPhysicsProposalSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text_Lambda([this]()
										{
											return ViewModel.IsValid()
												? FText::FromString(FString::Printf(TEXT("AI Physics Draft 경로: %s"), *ViewModel->GetPhysicsProposalDraftPath()))
												: FText::GetEmpty();
										})
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("LoadPhysicsProposalDraft", "AI Physics Proposal 불러오기"))
											.ToolTipText(LOCTEXT("LoadPhysicsProposalDraftTooltip", "Project Saved의 typed PhysicsDraft.json을 current Recipe/Target/USER-reviewed Evidence/private Profile binding과 exact 비교해 mutation 없이 읽습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleLoadPhysicsProposalDraft)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("ReviewPhysicsProposal", "Physics Proposal 검토 후 반영"))
											.ToolTipText(LOCTEXT("ReviewPhysicsProposalTooltip", "먼저 private 4 Profile complete payload를 mutation0 preview하고 확인창에서 승인할 때만 Builder-private Profile + Recipe receipt를 commit합니다. VehicleData Apply는 Step 7, Save는 자동 수행하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandlePhysicsProposalReview)
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetGameplaySetupVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("GameplaySetupHeader", "Gameplay Setup / 8영역 점검"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetGameplaySetupSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"GameplaySetupBoundary",
											"이 단계는 읽기 전용입니다. USER Socket 위치는 Static Mesh 에디터의 소켓 매니저(Socket Manager)에서 직접 관리하고, 변경 뒤 아래 '현재 상태 다시 확인'을 누르세요. Gameplay pending diff의 실제 VehicleData Apply는 Step 7 Final Review에서 별도 승인합니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetFinalReviewVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("FinalReviewHeader", "Final Review / Definition Apply"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetFinalReviewSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ApplyFinalReview", "Final Review 검토 후 적용"))
											.ToolTipText(LOCTEXT(
												"ApplyFinalReviewTooltip",
												"클릭 시 fresh R0 Final Review를 다시 만들고 USER 확인창에서 승인한 exact ProposalHash가 Apply 직전 fresh review와 같을 때만 기존 R3 DefinitionApply를 실행합니다. 자동 저장/자동 재시도는 하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanApplyFinalReview)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleFinalReviewApply)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("UndoFinalReview", "마지막 Builder Apply 되돌리기"))
											.ToolTipText(LOCTEXT(
												"UndoFinalReviewTooltip",
												"이 Builder Apply가 발급한 exact UE transaction이 현재 Undo stack top이고 post-Apply Target/Recipe 상태가 그대로일 때만 guarded Undo합니다. 다른 작업을 임의로 Undo하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanUndoFinalReview)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleFinalReviewUndo)
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"FinalReviewBoundary",
											"Apply는 Target VehicleData와 Recipe AppliedState를 기존 R3 transaction으로 변경할 수 있습니다. Apply 직전 다시 fresh review하며 stale approval은 차단됩니다. 자동 저장은 하지 않습니다. Undo는 이 Builder가 만든 마지막 exact Apply transaction만 대상으로 합니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetDrivingTestVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("DrivingTestHeader", "Technical Driving / USER Driving"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetDrivingTestSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("RunDrivingBenchmark", "기술 벤치마크 실행"))
											.ToolTipText(LOCTEXT(
												"RunDrivingBenchmarkTooltip",
												"현재 저장된 Target VehicleData를 기존 VB-P0-08 fixed-60Hz fresh PIE benchmark runner로 실행합니다. Recipe/Target이 Dirty면 실행하지 않으며 Builder는 자동 저장하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanRunDrivingBenchmark)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleRunDrivingBenchmark)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ApplyDrivingTargetToPIE", "현재 PIE에 선택 차량 적용"))
											.ToolTipText(LOCTEXT(
												"ApplyDrivingTargetToPIETooltip",
												"현재 benchmark와 exact 일치하는 saved Target VehicleData를 transient duplicate해 active PIE Player VehiclePawn에만 적용합니다. Persistent Asset은 수정하거나 저장하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanApplyDrivingTargetToPIE)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleApplyDrivingTargetToPIE)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("AcceptUserDriving", "USER 주행 PASS"))
											.ToolTipText(LOCTEXT(
												"AcceptUserDrivingTooltip",
												"선택 차량을 current PIE에 적용해 직접 주행한 뒤, 현재 Target DefinitionHash와 benchmark RunId에 exact binding된 USER Driving PASS를 기록합니다. 수치 benchmark가 이 판단을 대신하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanAcceptUserDriving)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleAcceptUserDriving)
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"DrivingTestBoundary",
											"Technical Benchmark는 기존 VB-P0-08 authority를 재사용하며 Reference 성능 threshold나 USER 주행감을 임의 판정하지 않습니다. Benchmark는 saved VehicleData만 읽습니다. USER test-drive는 transient VehicleData만 PIE Pawn에 적용하고 Persistent Asset/Map/Config를 저장하지 않습니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 6.0f)
						[
							SNew(STextBlock)
								.Text_Lambda([this]() { return LastStatusText; })
								.AutoWrapText(true)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
									.Text(LOCTEXT("Previous", "이전"))
									.IsEnabled(this, &SCFVehicleBuilderTab::CanMovePrevious)
									.OnClicked(this, &SCFVehicleBuilderTab::HandlePreviousStep)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
									.Text(LOCTEXT("RefreshCurrent", "현재 상태 다시 확인"))
									.OnClicked(this, &SCFVehicleBuilderTab::HandleRefreshCurrentStep)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
									.Text(LOCTEXT("Next", "다음"))
									.IsEnabled(this, &SCFVehicleBuilderTab::CanMoveNext)
									.OnClicked(this, &SCFVehicleBuilderTab::HandleNextStep)
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SSpacer)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
									.Text(LOCTEXT("OpenAdvanced", "고급 차량 데이터 제작 열기"))
									.ToolTipText(LOCTEXT("OpenAdvancedTooltip", "기존 Advanced Workspace를 별도 탭으로 엽니다. Guided Builder 상태를 우회해서 자동 적용하지 않습니다."))
									.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenAdvancedWorkspace)
							]
						]
					]
				]
			]
		]
	];

	HandleRefreshVehicles();
}

// Vehicle Browser cache를 fresh read하고 list row를 다시 만듭니다.
FReply SCFVehicleBuilderTab::HandleRefreshVehicles()
{
	if (!ViewModel.IsValid())
	{
		LastStatusText = LOCTEXT("MissingVM", "Builder ViewModel이 없습니다.");
		return FReply::Handled();
	}

	FString Error;
	if (!ViewModel->RefreshVehicles(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("대상 목록 읽기 실패: %s"), *Error));
		return FReply::Handled();
	}

	VehicleRows.Reset();
	for (const FCFVehicleListEntry& Entry : ViewModel->GetVehicleEntries())
	{
		VehicleRows.Add(MakeShared<FCFVehicleListEntry>(Entry));
	}

	if (VehicleListView.IsValid())
	{
		VehicleListView->RequestListRefresh();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("대상 %d개를 fresh read했습니다. Product Asset/VehicleData는 변경하지 않았습니다."),
		VehicleRows.Num()));
	return FReply::Handled();
}

// 선택 row를 current Builder target으로 전환합니다.
void SCFVehicleBuilderTab::HandleVehicleSelectionChanged(FVehicleRowPtr SelectedItem, const ESelectInfo::Type SelectInfo)
{
	if (!SelectedItem.IsValid() || !ViewModel.IsValid())
	{
		return;
	}

	FString Error;
	if (!ViewModel->SelectVehicle(*SelectedItem, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("대상 선택 실패: %s"), *Error));
		return;
	}

	SyncCreationFieldsFromSelection();
	SyncMeshPreparationFieldsFromRecipe();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("선택: %s | 현재 Step 상태를 authoritative truth에서 다시 계산했습니다."),
		*VehicleRowTitle(*SelectedItem)));
}

// Browser row 하나의 Slate 표현을 생성합니다.
TSharedRef<ITableRow> SCFVehicleBuilderTab::HandleGenerateVehicleRow(FVehicleRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 현재 row의 management/candidate 유형을 구분하는 accent 색상입니다.
	const FLinearColor AccentColor = Item.IsValid()
		? VehicleRowAccentColor(*Item)
		: FLinearColor::White;
	// 현재 row 유형을 짧게 표시하는 badge text입니다.
	const FString CategoryText = Item.IsValid()
		? VehicleRowCategoryText(*Item)
		: TEXT("INVALID");

	return SNew(STableRow<FVehicleRowPtr>, OwnerTable)
		.Padding(5.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(CategoryText))
						.ColorAndOpacity(AccentColor)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Item.IsValid() ? VehicleRowTitle(*Item) : TEXT("<invalid>")))
						.ColorAndOpacity(AccentColor)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
					.Text(FText::FromString(Item.IsValid() ? VehicleRowSubtitle(*Item) : FString()))
					.ColorAndOpacity(FLinearColor(0.72f, 0.72f, 0.76f, 1.0f))
					.AutoWrapText(true)
			]
		];
}

// current Step Definition projection을 그대로 사용하는 navigation UI를 생성합니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildStepNavigation()
{
	SAssignNew(StepNavigationBox, SVerticalBox);

	// 현재 ViewModel definition에서 만들어진 Step 개수입니다.
	const int32 StepCount = ViewModel.IsValid() ? ViewModel->GetStepViews().Num() : 0;
	for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
	{
		StepNavigationBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(SButton)
				.Text(this, &SCFVehicleBuilderTab::GetStepButtonText, StepIndex)
				.IsEnabled(this, &SCFVehicleBuilderTab::IsStepButtonEnabled, StepIndex)
				.OnClicked(this, &SCFVehicleBuilderTab::HandleSelectStep, StepIndex)
		];
	}

	return StepNavigationBox.ToSharedRef();
}

// Step button을 눌렀을 때 허용된 visible page로 이동합니다.
FReply SCFVehicleBuilderTab::HandleSelectStep(const int32 StepIndex)
{
	if (ViewModel.IsValid() && ViewModel->SelectVisibleStep(StepIndex))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("%d단계 화면을 열었습니다."), StepIndex + 1));
	}
	return FReply::Handled();
}

// 이전 Step으로 이동합니다.
FReply SCFVehicleBuilderTab::HandlePreviousStep()
{
	if (ViewModel.IsValid())
	{
		ViewModel->MovePreviousStep();
	}
	return FReply::Handled();
}

// 현재 authoritative truth를 다시 읽고 page 상태를 갱신합니다.
FReply SCFVehicleBuilderTab::HandleRefreshCurrentStep()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FString Error;
	if (!ViewModel->RefreshCurrentState(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("현재 상태 확인 실패: %s"), *Error));
		return FReply::Handled();
	}

	SyncMeshPreparationFieldsFromRecipe();
	LastStatusText = LOCTEXT("RefreshPass", "현재 상태를 fresh read했습니다. 자동 저장/적용은 수행하지 않았습니다.");
	return FReply::Handled();
}

// Complete인 현재 Step에서만 다음 Step으로 이동합니다.
FReply SCFVehicleBuilderTab::HandleNextStep()
{
	if (ViewModel.IsValid() && ViewModel->MoveNextStep())
	{
		LastStatusText = LOCTEXT("MovedNext", "완료된 단계의 다음 화면으로 이동했습니다.");
	}
	return FReply::Handled();
}

// Mesh-only 후보에서 VehicleData+Recipe 생성 proposal을 검토하고 explicit 승인 뒤 commit합니다.
FReply SCFVehicleBuilderTab::HandleCreateVehicleFromMesh()
{
	if (!ViewModel.IsValid() || !ViewModel->IsMeshOnlyCandidate()
		|| !NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return FReply::Handled();
	}

	// Mutation0 two-record creation preview입니다.
	FCFVehicleRecordCreatePreview Preview;
	// Preview/build diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareSelectedMeshRecordCreate(
		NewDefinitionPackageTextBox->GetText().ToString(),
		NewDefinitionNameTextBox->GetText().ToString(),
		NewRecipePackageTextBox->GetText().ToString(),
		NewRecipeNameTextBox->GetText().ToString(),
		Preview,
		Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("생성 검토 실패: %s"), *Error));
		return FReply::Handled();
	}

	const FCFVehicleListEntry& SelectedEntry = ViewModel->GetSelectedEntry();
	// USER가 실제 생성 범위를 읽고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("Guided Builder — 신규 차량 레코드 생성 검토\n\n차체 메시: %s\n생성할 VehicleData: %s\n생성할 Recipe: %s\n\nReference Evidence: 아직 생성 안 함\nprivate Profile 4종: 아직 생성 안 함\n차급 자동 추론: 안 함\n물리/밸런스 자동 추론: 안 함\nVehicleData 자동 Apply: 안 함\n자동 저장: 안 함\n\n지금은 Builder의 core 2-record만 생성합니다. 계속하시겠습니까?"),
		*SelectedEntry.ChassisMeshPath.ToString(),
		*Preview.ProspectiveDefinitionPath.ToString(),
		*Preview.ProspectiveRecipePath.ToString()));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("CreateCancelled", "생성을 취소했습니다. 변경된 Asset은 없습니다.");
		return FReply::Handled();
	}

	// Explicit USER approval이 붙은 exact two-record terminal result입니다.
	FCFVehicleRecordCreateResult Result;
	if (!ViewModel->ExecutePreparedMeshRecordCreate(Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("VehicleData+Recipe 생성 실패: %s"), *Error));
		return FReply::Handled();
	}

	HandleRefreshVehicles();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("VehicleData+Recipe를 생성했습니다. 자동 저장은 하지 않았습니다. 새 managed 차량을 선택한 뒤 Step 2 Mesh 준비 → Step 3 소켓 준비를 확인하세요. Reference Evidence/Companion은 차량 / Reference 단계에서 별도로 진행합니다.\n%s"),
		*Result.Operation.Message));
	return FReply::Handled();
}

// Step 1 current Recipe에 exact binding된 AI Research Draft를 Project Saved에서 읽습니다.
FReply SCFVehicleBuilderTab::HandleLoadResearchDraft()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Draft load/binding validation diagnostic입니다.
	FString Error;
	if (!ViewModel->LoadResearchDraft(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("AI Research Draft 불러오기 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = LOCTEXT(
		"ResearchDraftLoaded",
		"AI Research Draft를 current Recipe/Target에 exact binding해 읽었습니다. Product Asset/VehicleData는 변경하지 않았습니다.");
	return FReply::Handled();
}

// Step 1 Evidence + Missing private Profiles의 R2 proposal을 검토하고 explicit USER 승인 뒤 commit합니다.
FReply SCFVehicleBuilderTab::HandleResearchCompanionReview()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Mutation0 Companion proposal입니다.
	FCFBuilderCompanionPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareResearchCompanions(Preview, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Companion 생성 검토 실패: %s"), *Error));
		return FReply::Handled();
	}

	// Preview 시점에 Evidence가 이미 persistent하게 존재했는지 여부입니다.
	const bool bEvidenceAlreadyExists = ViewModel->GetCurrentReferenceEvidencePath().IsValid();
	// Current Recipe의 persistent private Profile binding입니다.
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	// USER review에 표시할 VehicleBase create 여부입니다.
	const bool bCreateVehicleBase = Recipe && Recipe->ProfileBindings.VehicleBaseProfile.IsNull();
	// USER review에 표시할 Drivetrain create 여부입니다.
	const bool bCreateDrivetrain = Recipe && Recipe->ProfileBindings.DrivetrainProfile.IsNull();
	// USER review에 표시할 Handling create 여부입니다.
	const bool bCreateHandling = Recipe && Recipe->ProfileBindings.HandlingProfile.IsNull();
	// USER review에 표시할 Performance create 여부입니다.
	const bool bCreatePerformance = Recipe && Recipe->ProfileBindings.PerformanceProfile.IsNull();

	// Existing/생성 예정 표기 helper입니다.
	const auto BuildCompanionLine = [](const TCHAR* Label, const bool bCreate, const FSoftObjectPath& Path)
	{
		return FString::Printf(
			TEXT("%s: %s | %s"),
			Label,
			bCreate ? TEXT("새로 생성") : TEXT("기존 보존"),
			*Path.ToString());
	};

	// USER가 exact R2 mutation 범위와 fingerprint를 읽고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — Reference / Companion 생성 검토\n\n"
			"%s\n%s\n%s\n%s\n%s\n\n"
			"Prospective EvidenceFingerprint:\n%s\n\n"
			"Current ResolvedDefinitionHash:\n%s\n"
			"Prospective ResolvedDefinitionHash:\n%s\n\n"
			"Existing companion 덮어쓰기: 안 함\n"
			"shared/foreign Profile 수정: 안 함\n"
			"VehicleData mutation: 안 함\n"
			"VehicleData Apply: 안 함\n"
			"자동 저장: 안 함\n"
			"자동 재시도: 안 함\n\n"
			"승인하면 Missing Evidence/private Profile만 하나의 Undo 가능한 R2 transaction으로 생성·Recipe binding합니다. 계속하시겠습니까?"),
		*BuildCompanionLine(TEXT("Reference Evidence"), !bEvidenceAlreadyExists, Preview.EvidencePath),
		*BuildCompanionLine(TEXT("VehicleBase Profile"), bCreateVehicleBase, Preview.VehicleBasePath),
		*BuildCompanionLine(TEXT("Drivetrain Profile"), bCreateDrivetrain, Preview.DrivetrainPath),
		*BuildCompanionLine(TEXT("Handling Profile"), bCreateHandling, Preview.HandlingPath),
		*BuildCompanionLine(TEXT("Performance Profile"), bCreatePerformance, Preview.PerformancePath),
		*Preview.ProspectiveEvidenceFingerprint,
		*Preview.CurrentResolvedDefinitionHash,
		*Preview.ProspectiveResolvedDefinitionHash));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("ResearchCompanionCancelled", "Companion 생성을 취소했습니다. Prepared approval은 실행하지 않았습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved R2 terminal result입니다.
	FCFBuilderCompanionResult Result;
	if (!ViewModel->ExecutePreparedResearchCompanions(Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Companion 생성 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("Reference/Companion transaction이 완료됐습니다. 자동 저장/VehicleData Apply는 수행하지 않았습니다.\n%s"),
		*Result.Operation.Message));
	return FReply::Handled();
}

// Step 1 current Evidence summary/fingerprint를 USER가 확인한 뒤 local Reference review token을 갱신합니다.
FReply SCFVehicleBuilderTab::HandleAcceptReferenceSet()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	if (!ViewModel->GetCurrentReferenceEvidence())
	{
		LastStatusText = LOCTEXT("ReferenceAcceptMissingEvidence", "승인할 persistent Reference Evidence가 없습니다.");
		return FReply::Handled();
	}

	// USER가 exact current persistent Reference state를 다시 읽는 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — Reference Set 확인\n\n%s\n\n"
			"이 확인은 local workflow navigation review token만 기록합니다.\n"
			"Profile write 권한: 없음\n"
			"Recipe write 권한: 없음\n"
			"VehicleData Apply 권한: 없음\n"
			"Save 권한: 없음\n\n"
			"현재 Evidence identity/fingerprint를 Step 1의 확인 기준으로 사용하시겠습니까?"),
		*ViewModel->BuildReferenceSummary()));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("ReferenceAcceptCancelled", "Reference Set 확인을 취소했습니다. 기존 review token은 변경하지 않았습니다.");
		return FReply::Handled();
	}

	// Local Reference review token update diagnostic입니다.
	FString Error;
	if (!ViewModel->AcceptCurrentReferenceSet(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Reference Set 확인 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = LOCTEXT(
		"ReferenceAcceptPass",
		"Current EvidenceId/EvidenceFingerprint를 USER Reference review token으로 확인했습니다. Product Asset/Apply/Save 권한은 변경되지 않았습니다.");
	return FReply::Handled();
}


// Step 2 current Recipe의 Chassis/Wheel Mesh pending 선택값을 typed AssetIntent Recipe-only commit으로 반영합니다.
FReply SCFVehicleBuilderTab::HandleCommitMeshPreparation()
{
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		LastStatusText = LOCTEXT("MeshPrepMissingRecipe", "Mesh 설정을 반영할 current managed Recipe가 없습니다.");
		return FReply::Handled();
	}

	if (!PendingChassisMeshPath.IsValid())
	{
		LastStatusText = LOCTEXT("MeshPrepMissingChassis", "차체 메시가 필요합니다.");
		return FReply::Handled();
	}
	if (!PendingWheelMeshPaths.IsValidIndex(0) || !PendingWheelMeshPaths[0].IsValid())
	{
		LastStatusText = LOCTEXT("MeshPrepMissingWheelFL", "FL / 앞왼쪽 Wheel Mesh는 필수입니다.");
		return FReply::Handled();
	}

	// Current Recipe의 Socket binding 등 Step 2가 소유하지 않는 AssetIntent 값을 보존한 채 Mesh reference만 교체합니다.
	FCFVehicleAssetIntent AssetIntent = ViewModel->GetRecipe()->AssetIntent;
	AssetIntent.ChassisMesh = TSoftObjectPtr<UStaticMesh>(PendingChassisMeshPath);
	AssetIntent.WheelMeshFL = TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[0]);
	AssetIntent.WheelMeshFR = PendingWheelMeshPaths.IsValidIndex(1)
		? TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[1])
		: TSoftObjectPtr<UStaticMesh>();
	AssetIntent.WheelMeshRL = PendingWheelMeshPaths.IsValidIndex(2)
		? TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[2])
		: TSoftObjectPtr<UStaticMesh>();
	AssetIntent.WheelMeshRR = PendingWheelMeshPaths.IsValidIndex(3)
		? TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[3])
		: TSoftObjectPtr<UStaticMesh>();

	// Existing typed Authoring facade의 Recipe-only terminal result입니다.
	FCFAuthoringOpResult CommitResult;
	// Commit/fresh re-evaluation diagnostic입니다.
	FString Error;
	if (!ViewModel->CommitMeshPreparation(AssetIntent, CommitResult, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Mesh 설정 반영 실패: %s"), *Error));
		return FReply::Handled();
	}

	SyncMeshPreparationFieldsFromRecipe();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("Mesh 설정을 Recipe에 반영했습니다. VehicleData Apply/자동 저장은 하지 않았습니다. %s"),
		*CommitResult.Message));
	return FReply::Handled();
}

// Step 2 Chassis StaticMesh object picker 변경을 pending 값에만 반영합니다.
void SCFVehicleBuilderTab::HandleChassisMeshChanged(const FAssetData& AssetData)
{
	PendingChassisMeshPath = AssetData.IsValid()
		? AssetData.GetSoftObjectPath()
		: FSoftObjectPath();
}

// Step 2 Wheel StaticMesh object picker 변경을 role별 pending 값에만 반영합니다.
void SCFVehicleBuilderTab::HandleWheelMeshChanged(const FAssetData& AssetData, const int32 WheelRoleIndex)
{
	if (!PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex))
	{
		return;
	}

	PendingWheelMeshPaths[WheelRoleIndex] = AssetData.IsValid()
		? AssetData.GetSoftObjectPath()
		: FSoftObjectPath();
}

// Step 2 Wheel Mesh role 한 행의 StaticMesh picker UI를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildWheelMeshPickerRow(
	const int32 WheelRoleIndex,
	const FText& RoleLabel,
	const bool bRequired)
{
	// USER-facing 필수/선택 구분을 role label에 붙인 문구입니다.
	const FText RequirementLabel = FText::Format(
		bRequired
			? LOCTEXT("WheelMeshRequiredFormat", "{0} [필수]")
			: LOCTEXT("WheelMeshOptionalFormat", "{0} [선택]"),
		RoleLabel);

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(SBox)
			.WidthOverride(150.0f)
			[
				SNew(STextBlock)
					.Text(RequirementLabel)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 2.0f)
		[
			SNew(SObjectPropertyEntryBox)
				.AllowedClass(UStaticMesh::StaticClass())
				.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingWheelMeshPath, WheelRoleIndex)
				.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleWheelMeshChanged, WheelRoleIndex)
				.AllowClear(!bRequired)
		];
}

// Step 3 current Chassis StaticMesh를 Asset Editor에서 바로 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenCurrentChassisMesh()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current selected vehicle의 exact Chassis StaticMesh object path입니다.
	const FSoftObjectPath ChassisMeshPath = ViewModel->GetCurrentChassisMeshPath();
	if (!ChassisMeshPath.IsValid())
	{
		LastStatusText = LOCTEXT("OpenChassisMissingPath", "현재 선택 차량에 열 수 있는 Chassis StaticMesh가 없습니다.");
		return FReply::Handled();
	}

	// 이미 load된 Chassis UObject 또는 exact path에서 load한 object입니다.
	UObject* ChassisObject = ChassisMeshPath.ResolveObject();
	if (!ChassisObject)
	{
		ChassisObject = ChassisMeshPath.TryLoad();
	}
	// Static Mesh Editor로 열 exact Chassis asset입니다.
	UStaticMesh* ChassisMesh = Cast<UStaticMesh>(ChassisObject);
	if (!ChassisMesh)
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Chassis StaticMesh를 열 수 없습니다: %s"), *ChassisMeshPath.ToString()));
		return FReply::Handled();
	}

	// Unreal Editor의 공식 Asset Editor subsystem입니다.
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor
		? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()
		: nullptr;
	if (!AssetEditorSubsystem)
	{
		LastStatusText = LOCTEXT("OpenChassisMissingSubsystem", "Asset Editor subsystem을 찾을 수 없습니다.");
		return FReply::Handled();
	}

	AssetEditorSubsystem->OpenEditorForAsset(ChassisMesh);
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("차체 메시를 열었습니다: %s | Builder는 Socket을 생성·이동·저장하지 않습니다."),
		*ChassisMeshPath.ToString()));
	return FReply::Handled();
}

// Step 3 required Wheel Socket 한 개의 exact current 이름을 클립보드에 복사합니다.
FReply SCFVehicleBuilderTab::HandleCopyRequiredWheelSocketName(const int32 WheelRoleIndex)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current Recipe binding을 반영한 required Wheel Socket 4종입니다.
	const TArray<FName> RequiredSocketNames = ViewModel->GetRequiredWheelSocketNames();
	if (!RequiredSocketNames.IsValidIndex(WheelRoleIndex))
	{
		LastStatusText = LOCTEXT("CopyRequiredSocketInvalidIndex", "복사할 Wheel Socket role을 찾을 수 없습니다.");
		return FReply::Handled();
	}

	// Clipboard에 기록할 exact Socket 이름입니다.
	const FString SocketNameText = RequiredSocketNames[WheelRoleIndex].ToString();
	FPlatformApplicationMisc::ClipboardCopy(*SocketNameText);
	LastStatusText = FText::FromString(FString::Printf(TEXT("Socket 이름을 복사했습니다: %s"), *SocketNameText));
	return FReply::Handled();
}

// Step 3 current Recipe의 optional Gameplay Socket 이름 전체를 줄바꿈 목록으로 복사합니다.
FReply SCFVehicleBuilderTab::HandleCopyOptionalSocketNames()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current Recipe에서 실제 파생한 optional/conditional Socket 이름입니다.
	const TArray<FName> OptionalSocketNames = ViewModel->GetOptionalSocketNames();
	if (OptionalSocketNames.IsEmpty())
	{
		LastStatusText = LOCTEXT("CopyOptionalSocketEmpty", "현재 Recipe가 요구하는 선택/조건부 Socket 이름이 없습니다.");
		return FReply::Handled();
	}

	// Clipboard에 복사할 줄바꿈 exact Socket name 목록입니다.
	TArray<FString> OptionalSocketNameTexts;
	OptionalSocketNameTexts.Reserve(OptionalSocketNames.Num());
	for (const FName SocketName : OptionalSocketNames)
	{
		OptionalSocketNameTexts.Add(SocketName.ToString());
	}

	// 여러 이름을 한 번에 붙여넣기 쉬운 줄바꿈 텍스트입니다.
	const FString ClipboardText = FString::Join(OptionalSocketNameTexts, LINE_TERMINATOR);
	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("선택/조건부 Socket 이름 %d개를 줄바꿈 목록으로 복사했습니다."),
		OptionalSocketNames.Num()));
	return FReply::Handled();
}

// Step 3 required Wheel Socket 한 행의 역할/이름/현재 상태/복사 UI를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildRequiredWheelSocketRow(const int32 WheelRoleIndex, const FText& RoleLabel)
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(SBox)
			.WidthOverride(100.0f)
			[
				SNew(STextBlock)
					.Text(RoleLabel)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(SEditableTextBox)
				.Text(this, &SCFVehicleBuilderTab::GetRequiredWheelSocketNameText, WheelRoleIndex)
				.IsReadOnly(true)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
				.Text(this, &SCFVehicleBuilderTab::GetRequiredWheelSocketStatusText, WheelRoleIndex)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 2.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("CopyRequiredSocketName", "복사"))
				.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyRequiredWheelSocketName, WheelRoleIndex)
		];
}

// Step 3 소켓 준비 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetSocketPreparationVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::SocketGuide
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 3 required Wheel Socket exact 이름을 표시합니다.
FText SCFVehicleBuilderTab::GetRequiredWheelSocketNameText(const int32 WheelRoleIndex) const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Current Recipe binding을 반영한 required Socket 목록입니다.
	const TArray<FName> RequiredSocketNames = ViewModel->GetRequiredWheelSocketNames();
	return RequiredSocketNames.IsValidIndex(WheelRoleIndex)
		? FText::FromName(RequiredSocketNames[WheelRoleIndex])
		: FText::FromString(TEXT("-"));
}

// Step 3 required Wheel Socket이 current Chassis에 존재하는지 표시합니다.
FText SCFVehicleBuilderTab::GetRequiredWheelSocketStatusText(const int32 WheelRoleIndex) const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Current Recipe binding을 반영한 required Socket 목록입니다.
	const TArray<FName> RequiredSocketNames = ViewModel->GetRequiredWheelSocketNames();
	if (!RequiredSocketNames.IsValidIndex(WheelRoleIndex))
	{
		return FText::FromString(TEXT("확인 불가"));
	}

	return ViewModel->IsCurrentChassisSocketFound(RequiredSocketNames[WheelRoleIndex])
		? FText::FromString(TEXT("현재 있음"))
		: FText::FromString(TEXT("없음 — 추가 필요"));
}

// Step 3 current Recipe의 optional Hardpoint/Destroyed FX Socket 상태를 표시합니다.
FText SCFVehicleBuilderTab::GetOptionalSocketGuideText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Current Recipe가 실제로 가리키는 optional/conditional Socket 이름입니다.
	const TArray<FName> OptionalSocketNames = ViewModel->GetOptionalSocketNames();
	if (OptionalSocketNames.IsEmpty())
	{
		return FText::FromString(
			TEXT("현재 Recipe가 요구하는 선택/조건부 Socket은 없습니다. Hardpoint가 필요해지면 Step 6 Gameplay Setup의 current Recipe 기준으로 정확한 HP_<LocationSlotId> 이름을 안내합니다."));
	}

	// USER-facing optional Socket 상태 행입니다.
	TArray<FString> SocketLines;
	SocketLines.Reserve(OptionalSocketNames.Num());
	for (const FName SocketName : OptionalSocketNames)
	{
		// Socket prefix로 현재 역할을 읽기 쉽게 분류합니다.
		const FString SocketNameText = SocketName.ToString();
		// Current fresh Chassis에 이 Socket이 존재하는지 여부입니다.
		const bool bSocketFound = ViewModel->IsCurrentChassisSocketFound(SocketName);
		// Hardpoint/FX/기타 current binding의 USER-facing role입니다.
		const TCHAR* SocketRole = SocketNameText.StartsWith(TEXT("HP_"))
			? TEXT("Hardpoint")
			: (SocketNameText.StartsWith(TEXT("FX_")) ? TEXT("파괴 FX") : TEXT("조건부"));

		SocketLines.Add(FString::Printf(
			TEXT("[선택/조건부 · %s] %s | %s"),
			SocketRole,
			*SocketNameText,
			bSocketFound ? TEXT("현재 있음") : TEXT("현재 없음")));
	}

	SocketLines.Add(TEXT("선택/조건부 Socket은 현재 Recipe/Gameplay 요구에 맞을 때만 추가하세요. Wheel Socket 4개와 달리 무조건 생성하는 항목이 아닙니다."));
	return FText::FromString(FString::Join(SocketLines, LINE_TERMINATOR));
}

// Step 3에서 복사할 optional Socket 이름이 하나 이상 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanCopyOptionalSocketNames() const
{
	return ViewModel.IsValid() && !ViewModel->GetOptionalSocketNames().IsEmpty();
}

// Step 5 current Recipe/Target/accepted Evidence에 exact binding된 AI Physics Draft를 Project Saved에서 읽습니다.
FReply SCFVehicleBuilderTab::HandleLoadPhysicsProposalDraft()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Physics Draft load/binding validation diagnostic입니다.
	FString Error;
	if (!ViewModel->LoadPhysicsProposalDraft(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("AI Physics Proposal 불러오기 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = LOCTEXT(
		"PhysicsDraftLoaded",
		"AI Physics Proposal을 current Recipe/Target/USER-reviewed Evidence/private Profile binding에 exact 연결해 읽었습니다. 아직 Profile/VehicleData는 변경하지 않았습니다.");
	return FReply::Handled();
}

// Step 5 private 4 Profile mutation0 proposal을 검토하고 explicit USER 승인 뒤 existing typed commit을 실행합니다.
FReply SCFVehicleBuilderTab::HandlePhysicsProposalReview()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Existing typed facade가 만든 exact mutation0 private 4 Profile proposal입니다.
	FCFBuilderProfileCommitPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PreparePhysicsProposal(Preview, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Physics Proposal 검토 실패: %s"), *Error));
		return FReply::Handled();
	}

	// USER가 이해할 AI proposal metadata입니다.
	const FCFBuilderPhysicsDraft& Draft = ViewModel->GetLoadedPhysicsProposalDraft();

	// Profile domain별 actual payload 변화 여부를 fingerprint로 표시하는 helper입니다.
	const auto BuildProfileChangeLine = [](const TCHAR* Label, const FString& CurrentFingerprint, const FString& ProspectiveFingerprint)
	{
		return FString::Printf(
			TEXT("%s: %s"),
			Label,
			CurrentFingerprint == ProspectiveFingerprint ? TEXT("현재 값 유지") : TEXT("변경 예정"));
	};

	// USER가 읽을 consumed canonical Claim ID 목록입니다.
	FString ConsumedClaimList;
	for (const FName ClaimId : Draft.ConsumedClaimIds)
	{
		if (!ConsumedClaimList.IsEmpty())
		{
			ConsumedClaimList += TEXT(", ");
		}
		ConsumedClaimList += ClaimId.ToString();
	}

	// USER가 숫자 세부 구조를 직접 이해하지 않아도 proposal 의미와 exact mutation boundary를 확인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — Physics Proposal 검토\n\n"
			"%s\n\n"
			"%s\n\n"
			"근거 Claim: %s\n"
			"EvidenceFingerprint:\n%s\n\n"
			"%s\n%s\n%s\n%s\n\n"
			"Prospective ResolvedDefinitionHash:\n%s\n\n"
			"실제 변경 범위:\n"
			"- current Recipe에 exact owner로 binding된 Builder-private 4 Profile만 대상\n"
			"- Recipe BuilderCommitReceipt provenance metadata는 갱신될 수 있음\n"
			"- shared/foreign Profile 덮어쓰기: 안 함\n"
			"- Target VehicleData mutation: 안 함\n"
			"- Definition Apply: 안 함 (Step 7에서 별도 승인)\n"
			"- 자동 저장: 안 함\n"
			"- 자동 재시도: 안 함\n\n"
			"승인하면 위 complete typed Profile Proposal을 기존 R1 AuthoringWrite transaction으로 반영합니다. 계속하시겠습니까?"),
		Draft.ProposalLabel.IsEmpty() ? TEXT("<Proposal 제목 없음>") : *Draft.ProposalLabel,
		Draft.UserFacingSummary.IsEmpty() ? TEXT("<차량 특성 설명 없음>") : *Draft.UserFacingSummary,
		ConsumedClaimList.IsEmpty() ? TEXT("<없음>") : *ConsumedClaimList,
		*Preview.EvidenceFingerprint,
		*BuildProfileChangeLine(TEXT("VehicleBase"), Preview.CurrentFingerprints.VehicleBaseFingerprint, Preview.ProspectiveFingerprints.VehicleBaseFingerprint),
		*BuildProfileChangeLine(TEXT("Drivetrain"), Preview.CurrentFingerprints.DrivetrainFingerprint, Preview.ProspectiveFingerprints.DrivetrainFingerprint),
		*BuildProfileChangeLine(TEXT("Handling"), Preview.CurrentFingerprints.HandlingFingerprint, Preview.ProspectiveFingerprints.HandlingFingerprint),
		*BuildProfileChangeLine(TEXT("Performance"), Preview.CurrentFingerprints.PerformanceFingerprint, Preview.ProspectiveFingerprints.PerformanceFingerprint),
		*Preview.ProspectiveResolvedDefinitionHash));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("PhysicsProposalCancelled", "Physics Proposal 반영을 취소했습니다. Profile/Recipe/VehicleData는 변경하지 않았습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved R1 private Profile commit 결과입니다.
	FCFAuthoringOpResult Result;
	if (!ViewModel->ExecutePreparedPhysicsProposal(Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Physics Proposal 반영 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("Physics Proposal을 Builder-private 4 Profile typed lane으로 반영했습니다. Target VehicleData Apply/자동 Save는 수행하지 않았습니다.\n%s"),
		*Result.Message));
	return FReply::Handled();
}

// Step 7 fresh Final Review 전체 Diff/provenance를 검토하고 explicit USER DefinitionApply 승인 뒤 existing R3 Apply를 실행합니다.
FReply SCFVehicleBuilderTab::HandleFinalReviewApply()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// USER dialog 직전 existing R0 facade가 만든 fresh Final Review입니다.
	FCFBuilderFinalReviewResult Review;
	// Fresh review / explicit Apply diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareFinalReviewApply(Review, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Final Review 적용 검토 실패: %s"), *Error));
		return FReply::Handled();
	}

	// USER가 exact current Diff/provenance/mutation boundary를 읽고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — Final Review / Definition Apply\n\n"
			"%s\n\n"
			"실제 승인 범위:\n"
			"- Target VehicleData: current resolved Field Diff를 기존 R3 Apply lane으로 반영\n"
			"- Recipe AppliedState: current Apply evidence로 갱신 가능\n"
			"- UE transaction: 생성 가능\n"
			"- Package Dirty: 될 수 있음\n"
			"- 자동 저장: 안 함\n"
			"- 자동 재시도: 안 함\n"
			"- Apply 직전 Final Review fresh 재검사: 함\n"
			"- ProposalHash가 달라졌으면 stale approval 차단\n\n"
			"현재 exact ProposalHash:\n%s\n\n"
			"위 Field Diff 전체와 Reference provenance를 확인했습니다. DefinitionApply를 실행하시겠습니까?"),
		*ViewModel->BuildFinalReviewSummary(),
		*Review.ApplyProposal.ProposalHash));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		// USER 취소 시 prepared approval을 남기지 않도록 read-only refresh로 폐기합니다.
		FString RefreshError;
		ViewModel->RefreshCurrentState(RefreshError);
		LastStatusText = LOCTEXT(
			"FinalReviewApplyCancelled",
			"DefinitionApply를 취소했습니다. Target/Recipe는 변경하지 않았고 prepared approval은 폐기했습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved DefinitionApply terminal result입니다.
	FCFBuilderFinalApplyResult Result;
	if (!ViewModel->ExecutePreparedFinalReviewApply(Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("DefinitionApply 실패/차단: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT(
			"DefinitionApply가 완료됐습니다. 자동 저장은 하지 않았습니다.\n"
			"Target 변경: %s | Recipe 변경: %s | guarded Undo: %s\n%s"),
		Result.Operation.Mutation.bTargetChanged ? TEXT("예") : TEXT("아니오"),
		Result.Operation.Mutation.bRecipeChanged ? TEXT("예") : TEXT("아니오"),
		Result.bUndoAvailable ? TEXT("사용 가능") : TEXT("없음"),
		*Result.Operation.Message));
	return FReply::Handled();
}

// Step 7 마지막 successful Builder Apply가 발급한 exact guarded Undo token을 USER 승인 뒤 실행합니다.
FReply SCFVehicleBuilderTab::HandleFinalReviewUndo()
{
	if (!ViewModel.IsValid() || !ViewModel->HasFinalReviewUndoToken())
	{
		LastStatusText = LOCTEXT("FinalReviewUndoMissing", "현재 Builder가 안전하게 되돌릴 Final Apply transaction이 없습니다.");
		return FReply::Handled();
	}

	// USER가 확인할 exact current guarded Undo token입니다.
	const FCFBuilderUndoToken& UndoToken = ViewModel->GetFinalReviewUndoToken();
	// USER가 arbitrary Editor Undo가 아님을 확인할 explicit guarded Undo 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — 마지막 Definition Apply 되돌리기\n\n"
			"TransactionId: %s\n"
			"Recipe: %s\n"
			"Target VehicleData: %s\n\n"
			"안전 조건:\n"
			"- 이 Builder가 현재 Editor lifetime에서 만든 exact Apply transaction이어야 함\n"
			"- 이 transaction이 현재 Unreal Undo stack top이어야 함\n"
			"- Apply 뒤 Target/Recipe/AppliedState가 transaction 밖에서 바뀌지 않았어야 함\n"
			"- 조건이 하나라도 달라지면 아무 Undo도 실행하지 않고 차단\n"
			"- 자동 저장: 안 함\n\n"
			"이 exact Builder Apply만 되돌리시겠습니까?"),
		*UndoToken.TransactionId.ToString(EGuidFormats::DigitsWithHyphensLower),
		*UndoToken.RecipePath.ToString(),
		*UndoToken.TargetDefinitionPath.ToString()));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("FinalReviewUndoCancelled", "guarded Undo를 취소했습니다. 현재 Apply 상태와 Undo token은 유지합니다.");
		return FReply::Handled();
	}

	// Exact guarded Undo terminal result입니다.
	FCFAuthoringOpResult Result;
	// Guarded Undo diagnostic입니다.
	FString Error;
	if (!ViewModel->ExecuteFinalReviewUndo(Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("guarded Undo가 안전 조건에서 차단/실패했습니다: %s"),
			*Error));
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("마지막 Builder DefinitionApply를 exact guarded Undo로 되돌렸습니다. 자동 저장은 하지 않았습니다.\n%s"),
		*Result.Message));
	return FReply::Handled();
}

// Step 8 current saved Target을 existing VB-P0-08 fixed-60Hz benchmark runner로 non-blocking 실행합니다.
FReply SCFVehicleBuilderTab::HandleRunDrivingBenchmark()
{
	if (!ViewModel.IsValid() || bDrivingBenchmarkRunning)
	{
		return FReply::Handled();
	}

	// Existing runner를 시작할 executable입니다.
	FString Executable;
	// Existing runner의 exact command-line arguments입니다.
	FString Arguments;
	// Runner working directory입니다.
	FString WorkingDirectory;
	// 이번 run의 exact result/USER acceptance identity입니다.
	FString RunId;
	// Saved-state / target identity / runner preparation diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareDrivingBenchmarkLaunch(Executable, Arguments, WorkingDirectory, RunId, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("기술 벤치마크 실행 준비 실패: %s"), *Error));
		return FReply::Handled();
	}

	// UI thread를 막지 않는 detached benchmark child process입니다.
	DrivingBenchmarkProcess = FPlatformProcess::CreateProc(
		*Executable,
		*Arguments,
		true,
		true,
		true,
		nullptr,
		0,
		*WorkingDirectory,
		nullptr,
		nullptr);
	if (!DrivingBenchmarkProcess.IsValid())
	{
		LastStatusText = LOCTEXT("DrivingBenchmarkLaunchFailed", "기술 벤치마크 child process를 시작하지 못했습니다. Target/Recipe는 변경하지 않았습니다.");
		return FReply::Handled();
	}

	RunningDrivingBenchmarkRunId = RunId;
	bDrivingBenchmarkRunning = true;
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("기술 벤치마크를 기존 VB-P0-08 fixed-60Hz runner로 실행 중입니다. UI를 막지 않으며 자동 저장하지 않습니다. RunId=%s"),
		*RunId));
	return FReply::Handled();
}

// Step 8 current selected VehicleData transient duplicate를 active PIE Player VehiclePawn에 적용합니다.
FReply SCFVehicleBuilderTab::HandleApplyDrivingTargetToPIE()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Active PIE transient test-drive preparation diagnostic입니다.
	FString Error;
	if (!ViewModel->ApplySelectedVehicleToActivePIE(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("선택 차량 PIE 적용 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = LOCTEXT(
		"DrivingTargetAppliedToPIE",
		"선택 차량의 saved VehicleData를 transient duplicate해 현재 PIE Player VehiclePawn에 적용했습니다. Persistent Asset은 변경하지 않았습니다. 이제 직접 가속/조향/제동/고속 반응을 주행 확인하세요.");
	return FReply::Handled();
}

// Step 8 exact current benchmark/Target에 USER Driving PASS를 명시적으로 기록합니다.
FReply SCFVehicleBuilderTab::HandleAcceptUserDriving()
{
	if (!ViewModel.IsValid() || !ViewModel->HasDrivingBenchmarkResult())
	{
		return FReply::Handled();
	}

	// USER가 benchmark 수치가 아니라 실제 주행 체감에 대해 명시 판정할 confirmation입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — USER Driving Acceptance\n\n"
			"Benchmark RunId: %s\n"
			"Target DefinitionHash: %s\n\n"
			"현재 PIE에서 선택 차량을 직접 주행했고 아래를 확인했습니까?\n"
			"- 출발/가속 반응이 Reference와 의도한 차량 성격에 어울림\n"
			"- 조향 반응과 회전반경이 차량 크기/성격에 어울림\n"
			"- 제동이 정상적이고 조작 가능함\n"
			"- RPM/변속/고속 반응에 명백한 이상이 없음\n"
			"- Wheel/차체 물리에 플레이를 막는 이상이 없음\n\n"
			"Technical benchmark 수치만 보고 PASS하는 것이 아니라 실제 주행 체감에 대한 USER 판단입니다.\n"
			"이 exact 차량 상태를 USER Driving PASS로 승인하시겠습니까?"),
		*ViewModel->GetDrivingBenchmarkResult().RunId,
		*ViewModel->GetDrivingBenchmarkResult().ExpectedTargetDefinitionHash));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("UserDrivingCancelled", "USER Driving PASS를 승인하지 않았습니다. 차량 데이터와 benchmark 결과는 변경하지 않습니다.");
		return FReply::Handled();
	}

	// Exact benchmark/Target-bound local USER Driving acceptance diagnostic입니다.
	FString Error;
	if (!ViewModel->AcceptCurrentUserDriving(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("USER Driving PASS 기록 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("USER Driving PASS를 exact Target DefinitionHash + Benchmark RunId에 기록했습니다. RunId=%s. Step 8이 Complete이면 VB-P0-09 신규 차량 E2E USER Acceptance를 닫을 수 있습니다."),
		*ViewModel->GetDrivingBenchmarkResult().RunId));
	return FReply::Handled();
}

// 선택한 Mesh 후보 이름으로 사람이 수정할 수 있는 package/name 제안값을 채웁니다.
void SCFVehicleBuilderTab::SyncCreationFieldsFromSelection()
{
	if (!NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return;
	}

	if (!ViewModel.IsValid() || !ViewModel->IsMeshOnlyCandidate())
	{
		NewDefinitionPackageTextBox->SetText(FText::GetEmpty());
		NewDefinitionNameTextBox->SetText(FText::GetEmpty());
		NewRecipePackageTextBox->SetText(FText::GetEmpty());
		NewRecipeNameTextBox->SetText(FText::GetEmpty());
		return;
	}

	// Mesh 이름에서 semantic inference 없이 파일명 stem만 가져옵니다.
	FString Stem = ViewModel->GetSelectedEntry().ChassisMeshPath.GetAssetName();
	Stem.RemoveFromStart(TEXT("SM_"));
	if (Stem.IsEmpty())
	{
		Stem = TEXT("Vehicle");
	}

	// USER가 수정 가능한 VehicleData 이름 제안입니다.
	const FString DefinitionName = TEXT("DA_Vehicle_") + Stem;
	// USER가 수정 가능한 Recipe 이름 제안입니다.
	const FString RecipeName = TEXT("DA_Recipe_") + Stem;
	NewDefinitionPackageTextBox->SetText(FText::FromString(TEXT("/Game/CarFight/Data/Authoring/") + DefinitionName));
	NewDefinitionNameTextBox->SetText(FText::FromString(DefinitionName));
	NewRecipePackageTextBox->SetText(FText::FromString(TEXT("/Game/CarFight/Data/Authoring/") + RecipeName));
	NewRecipeNameTextBox->SetText(FText::FromString(RecipeName));
}

// Current Recipe AssetIntent를 Step 2 pending picker 값으로 동기화합니다.
void SCFVehicleBuilderTab::SyncMeshPreparationFieldsFromRecipe()
{
	PendingChassisMeshPath = FSoftObjectPath();
	PendingWheelMeshPaths.SetNum(4);
	for (FSoftObjectPath& PendingWheelMeshPath : PendingWheelMeshPaths)
	{
		PendingWheelMeshPath = FSoftObjectPath();
	}

	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		return;
	}

	// Current persistent Recipe의 typed AssetIntent입니다.
	const FCFVehicleAssetIntent& AssetIntent = ViewModel->GetRecipe()->AssetIntent;
	PendingChassisMeshPath = AssetIntent.ChassisMesh.ToSoftObjectPath();
	PendingWheelMeshPaths[0] = AssetIntent.WheelMeshFL.ToSoftObjectPath();
	PendingWheelMeshPaths[1] = AssetIntent.WheelMeshFR.ToSoftObjectPath();
	PendingWheelMeshPaths[2] = AssetIntent.WheelMeshRL.ToSoftObjectPath();
	PendingWheelMeshPaths[3] = AssetIntent.WheelMeshRR.ToSoftObjectPath();
}

// Step 2 Mesh 준비 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetMeshPreparationVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::MeshPrep
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 2 pending Chassis StaticMesh object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetPendingChassisMeshPath() const
{
	return PendingChassisMeshPath.IsValid() ? PendingChassisMeshPath.ToString() : FString();
}

// Step 2 pending Wheel StaticMesh role별 object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetPendingWheelMeshPath(const int32 WheelRoleIndex) const
{
	return PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex) && PendingWheelMeshPaths[WheelRoleIndex].IsValid()
		? PendingWheelMeshPaths[WheelRoleIndex].ToString()
		: FString();
}

// Step 1의 Mesh-only 생성 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetMeshCreationVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->IsMeshOnlyCandidate()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::IdentityReference
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 1 managed Recipe의 Reference/Companion UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetReferenceFlowVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::IdentityReference
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 1 persistent Evidence 또는 loaded Draft의 USER-facing 요약을 표시합니다.
FText SCFVehicleBuilderTab::GetReferenceSummaryText() const
{
	return ViewModel.IsValid()
		? FText::FromString(ViewModel->BuildReferenceSummary())
		: FText::GetEmpty();
}


// Step 5 Physics Proposal 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetPhysicsProposalVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::PhysicsProposal
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 5 loaded Draft/current receipt를 USER-facing 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetPhysicsProposalSummaryText() const
{
	return ViewModel.IsValid()
		? FText::FromString(ViewModel->BuildPhysicsProposalSummary())
		: FText::GetEmpty();
}

// Step 6 Gameplay Setup 전용 R0 guidance UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetGameplaySetupVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::GameplaySetup
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 6의 8영역 completeness/USER Socket/pending diff를 USER-facing 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetGameplaySetupSummaryText() const
{
	return ViewModel.IsValid()
		? FText::FromString(ViewModel->BuildGameplayGuidanceSummary())
		: FText::GetEmpty();
}

// Step 7 Final Review 전용 R0 review/apply/undo UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetFinalReviewVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::FinalReview
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 7 validation/drift/provenance/전체 Field Diff/apply readiness를 USER-facing 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetFinalReviewSummaryText() const
{
	return ViewModel.IsValid()
		? FText::FromString(ViewModel->BuildFinalReviewSummary())
		: FText::GetEmpty();
}

// Current fresh Final Review가 explicit DefinitionApply 가능한지 반환합니다.
bool SCFVehicleBuilderTab::CanApplyFinalReview() const
{
	return ViewModel.IsValid()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::FinalReview
		&& ViewModel->HasFinalReviewResult()
		&& ViewModel->GetFinalReviewResult().bCanApply;
}

// Current Editor lifetime에 exact Builder guarded Undo token이 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanUndoFinalReview() const
{
	return ViewModel.IsValid()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::FinalReview
		&& ViewModel->HasFinalReviewUndoToken();
}

// Step 8 Technical Driving / USER Driving 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetDrivingTestVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 8 saved-state/technical metrics/USER checklist를 USER-facing 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetDrivingTestSummaryText() const
{
	return ViewModel.IsValid()
		? FText::FromString(ViewModel->BuildDrivingTestSummary())
		: FText::GetEmpty();
}

// Current Step 8에서 benchmark child process를 새로 시작할 수 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanRunDrivingBenchmark() const
{
	return ViewModel.IsValid()
		&& !bDrivingBenchmarkRunning
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		&& ViewModel->GetCurrentStep().State != ECFVehicleBuilderStepState::Locked
		&& ViewModel->GetCurrentStep().State != ECFVehicleBuilderStepState::Blocked;
}

// Current benchmark가 exact current Target에 binding됐을 때 PIE transient 적용 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanApplyDrivingTargetToPIE() const
{
	return ViewModel.IsValid()
		&& !bDrivingBenchmarkRunning
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		&& ViewModel->HasDrivingBenchmarkResult();
}

// Current benchmark가 exact current Target에 binding됐을 때 USER explicit PASS 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanAcceptUserDriving() const
{
	return ViewModel.IsValid()
		&& !bDrivingBenchmarkRunning
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		&& ViewModel->HasDrivingBenchmarkResult()
		&& ViewModel->IsUserTestDrivePreparedThisSession()
		&& !ViewModel->HasCurrentUserDrivingAcceptance();
}

// 기존 Advanced Vehicle Authoring Workspace를 별도 탭으로 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenAdvancedWorkspace()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("CarFight.VehicleAuthoring")));
	return FReply::Handled();
}

// 현재 선택을 USER-facing 한 줄로 표시합니다.
FText SCFVehicleBuilderTab::GetSelectionText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
		return LOCTEXT("NoSelection", "대상: 선택 안 됨");
	}

	return FText::FromString(FString::Printf(
		TEXT("대상: %s"),
		*VehicleRowTitle(ViewModel->GetSelectedEntry())));
}

// 현재 Step 제목을 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepTitle() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return LOCTEXT("NoStep", "단계 없음");
	}

	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	return FText::FromString(FString::Printf(TEXT("%d. %s"), Step.StepNumber, *Step.Title.ToString()));
}

// 현재 Step 상태를 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepStateText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return FText::GetEmpty();
	}

	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	return FText::FromString(FString::Printf(
		TEXT("상태: %s | Provider 연결: %s"),
		*BuilderStepStateText(Step.State),
		Step.bProviderConnected ? TEXT("예") : TEXT("아직 아님")));
}

// 현재 Step 설명을 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepSummaryText() const
{
	return ViewModel.IsValid() && !ViewModel->GetStepViews().IsEmpty()
		? ViewModel->GetCurrentStep().Summary
		: FText::GetEmpty();
}

// 현재 Step 해결/다음 행동을 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepResolutionText() const
{
	return ViewModel.IsValid() && !ViewModel->GetStepViews().IsEmpty()
		? ViewModel->GetCurrentStep().Resolution
		: FText::GetEmpty();
}

// Step button label을 상태와 함께 표시합니다.
FText SCFVehicleBuilderTab::GetStepButtonText(const int32 StepIndex) const
{
	if (!ViewModel.IsValid() || !ViewModel->GetStepViews().IsValidIndex(StepIndex))
	{
		return FText::FromString(FString::Printf(TEXT("%d. -"), StepIndex + 1));
	}

	const FCFVehicleBuilderStepView& Step = ViewModel->GetStepViews()[StepIndex];
	return FText::FromString(FString::Printf(
		TEXT("%d. %s  [%s]"),
		Step.StepNumber,
		*Step.Title.ToString(),
		*BuilderStepStateText(Step.State)));
}

// Step button을 현재 state에서 클릭할 수 있는지 반환합니다.
bool SCFVehicleBuilderTab::IsStepButtonEnabled(const int32 StepIndex) const
{
	if (!ViewModel.IsValid() || !ViewModel->GetStepViews().IsValidIndex(StepIndex))
	{
		return false;
	}

	const ECFVehicleBuilderStepState State = ViewModel->GetStepViews()[StepIndex].State;
	return State != ECFVehicleBuilderStepState::Locked && State != ECFVehicleBuilderStepState::Unavailable;
}

// Previous 버튼 활성 조건입니다.
bool SCFVehicleBuilderTab::CanMovePrevious() const
{
	return ViewModel.IsValid() && ViewModel->GetCurrentStepIndex() > 0;
}

// Next 버튼 활성 조건입니다.
bool SCFVehicleBuilderTab::CanMoveNext() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return false;
	}

	return ViewModel->GetCurrentStep().State == ECFVehicleBuilderStepState::Complete
		&& ViewModel->GetStepViews().IsValidIndex(ViewModel->GetCurrentStepIndex() + 1);
}

#undef LOCTEXT_NAMESPACE
