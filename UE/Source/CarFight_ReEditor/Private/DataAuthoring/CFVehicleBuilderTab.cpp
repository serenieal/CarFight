// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTab.cpp
// Version: v1.25.1
// Date: 2026-09-02
// Description: Guided Vehicle Builder Slate Shell + CF-FQ-042 Vehicle ID/Candidate Quick Start UX 구현입니다.
// Changelog:
// - v1.25.1: P0-07 USER UAT 인식 개선. Hardpoint row의 generic `삭제`를 `장착 위치만 삭제`로 명확화하고, Recipe row만 제거되며 Chassis StaticMesh Socket은 유지된다는 안내를 표/tooltip에 직접 노출. 삭제 로직은 변경하지 않음.
// - v1.25.0: P0-07 USER UAT 2차 피드백. Wheel/Hardpoint `추가 후 편집`이 Socket 생성 직후 cached snapshot 때문에 stale 활성화되던 문제를 live UStaticMesh::FindSocket truth로 교정. 상단 Target/Step/State/Summary/지금 할 일을 한 고정 프레임으로 묶고 Step 3 Socket Preparation을 remaining-height scroll panel로 전환.
// - v1.24.1: P0-07 durable USER Driving PASS가 Recipe non-semantic receipt에 기록되며 auto-save하지 않는 실제 경계를 confirmation/success 문구에 명시. DefinitionHash가 acceptance authority이고 RunId는 diagnostic-only임을 표시.
// - v1.24.0: P0-07 UAT 피드백 반영. Wheel/Hardpoint Chassis 편집 버튼을 하나로 통합하고 missing Wheel/Hardpoint Socket을 exact 이름으로 원점 생성→편집하는 explicit no-auto-save UX를 추가. Hardpoint 이름을 read-only copy slot으로 바꾸고 `기타 조건부 Socket`을 실제 의미인 `파괴 FX Socket`으로 명확화.
// - v1.23.1: VMG-P0-04 코드감사에서 Step 6 scope 주석을 교정. 8영역 Guidance/Socket만 read-only이며 Standard Mount 패널은 explicit typed Recipe write/remove를 소유한다는 실제 구현 경계와 일치시킴.
// - v1.23.0: CF-FQ-043 VMG-P0-04 Step 6 Hardpoint별 Standard Mount transient draft(Type/Size/Preset), explicit commit/remove UI와 Recipe refresh sync를 추가. 기존 8영역 Guidance는 read-only diagnosis authority로 유지.
// - v1.22.2: VMG-P0-03 Step 2 pending Chassis/Wheel Mesh open 버튼, optional FR/RL/RR→pending FL fallback, shared Chassis asset edit warning을 추가하고 Step 2/3 StaticMesh editor backend를 공통화.
// - v1.22.1: VMG-P0-03 Legacy/custom SocketName=None row에서 HP_* 이름을 임의 합성하지 않고 stored truth/LocalTransform 보존 상태를 표시하며 삭제 안내도 실제 Socket binding 유무를 구분.
// - v1.22.0: CF-FQ-043 VMG-P0-03 Step 3에 explicit Hardpoint Plan Mode, Standard category add/remove row, exact Socket status/copy, contextual Wheel/Hardpoint editor entry와 shared Chassis warning을 연결.
// - v1.21.0: Explicit New Vehicle mode에서 Browser refresh가 stale current row를 Slate selection으로 복원하지 않도록 row highlight 복원 조건을 방어적으로 제한.
// - v1.20.0: Step 1 일반 naming을 Vehicle ID 한 칸으로 전환하고 deterministic default identity, 실시간 validation, 접힌 Advanced Asset 경로 override와 Mesh Candidate Quick Start label을 연결.
// - v1.19.0: Explicit New Vehicle Step 1에 Blank/optional StaticMesh picker와 공통 Preview→승인→Commit→exact Browser row highlight/adoption reporting을 연결.
// - v1.18.0: 좌측 작업 대상에 '+ 새 차량 만들기'를 추가하고 BuilderVM transient 신규 제작 상태와 연결. pre-refresh Stable Step 8개를 최초 Slate tree에서 즉시 표시.
// - v1.17.0: Step 1에 Existing Reference Evidence complete replacement R1 전용 검토 버튼/dialog을 추가하고 fingerprint/count/mutation boundary/receipt stale 영향을 USER 승인 전에 표시.
// - v1.16.0: Builder-wide Transmission diagnostic에 상향변속 RPM retention을 함께 표시해 기어 간 spacing sanity review를 명확화.
// - v1.15.0: Step 5 Physics Proposal USER review에 TransmissionProposalHash와 기어별 ChangeUpRPM 예상 차속/post-shift RPM diagnostic을 표시. 값은 runtime shift trigger가 아님을 명시.
// - v1.14.0: Resolver fail-closed로 SelectVehicle가 false를 반환해도 이미 전환된 current selection/Recipe 기준으로 Step 2 pending picker를 재동기화해 이전 차량 표시가 남지 않도록 보강.
// - v1.13.0: 차량 선택/refresh/commit 뒤 Step 2 ObjectPicker subtree를 current Recipe pending state로 재생성해 이전 차량 Chassis/Wheel 표시가 남는 stale presentation을 교정.
// - v1.12.0: actual Wagon Final Review에서 28건 Field Diff가 AutoHeight TextBlock으로 무한 확장되어 하단이 잘리는 문제를 교정. Final Review 상세 Diff 영역만 고정 높이 ScrollBox로 제한하고 Apply/Undo/경계 안내는 항상 노출.
// - v1.11.0: Step 4 Ready/NotCaptured를 Step 7 deferred Apply 정상 상태로 표시하고 Next 버튼이 ViewModel의 단일 forward-progress contract를 사용하도록 교정.
// - v1.10.0: USER 피드백에 따라 작업 대상 색상을 identity hash 팔레트에서 관리 상태 의미 기반 고정색으로 전환하고 같은 분류끼리 모아 정렬. 제목은 중립색, 분류 배지만 의미색을 사용.
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
// - v1.21.0부터 Explicit New Vehicle mode에서는 Browser refresh가 기존 row selection/highlight를 자동 복원하지 않습니다. 새 record adoption 성공 뒤에는 기존 exact row 복원 경로를 그대로 사용합니다.
// - v1.20.0부터 일반 Guided creation은 Vehicle ID가 기본 입력이며 Definition/Recipe package/object 네 칸은 접힌 `고급 Asset 경로 설정` override로 이동합니다. invalid ID는 sanitize하지 않고 fail-closed하며 최종 collision/path/type는 existing Preview authority가 재검증합니다.
// - v1.19.0 Explicit New Vehicle은 P0-02에서 기존 4개 package/name 입력을 임시 재사용하며 Vehicle ID 단일 naming은 P0-03으로 남깁니다. optional Chassis는 사용 중 Mesh도 허용하고 Recipe AssetIntent에만 기록하며 VehicleData Apply/Save는 하지 않습니다.
// - v1.18.0 신규 차량 entry는 UI/BuilderVM transient state만 전환하며 Asset 생성/Save/VehicleData Apply는 수행하지 않습니다. Blank/ChassisMesh 생성 입력은 후속 VBCUX Gate에서 연결합니다.
// - Step 8 benchmark는 existing RunBuilderBench.ps1/VB-P0-08 authority만 사용하며 saved Target을 요구합니다. USER test-drive는 active PIE transient duplicate만 적용하고 Product Asset 자동 Save/Reference threshold 자동 판정은 하지 않습니다.
// - Step 7 Target mutation은 existing ReadBuilderFinalReview → explicit DefinitionApply → ApplyBuilderFinalReview R3만 사용합니다. Undo는 Apply가 발급한 exact guarded token만 사용하며 auto Save/retry는 없습니다.
// - Step 6은 existing ReadBuilderGameplayGuidance R0를 read-only diagnosis로 소비하고 Standard Mount 패널의 explicit typed Recipe write/remove만 추가로 허용합니다. Step 3의 USER explicit `소켓 추가`만 StaticMesh에 exact-name Socket을 원점 생성할 수 있으며 자동 배치/자동 저장은 하지 않습니다. Target VehicleData Apply는 Step 7이 소유합니다.
// - Step 5 persistent mutation은 existing CommitBuilderProfiles typed facade의 explicit USER-approved R1 transaction만 사용합니다. Target VehicleData Apply는 Step 7에 남기고 raw/shared Profile write와 auto Save는 수행하지 않습니다.
// - Step 1 persistent mutation은 existing CreateBuilderCompanions typed facade의 explicit USER-approved R2 transaction만 사용합니다. raw Profile/Evidence write, VehicleData Apply와 auto Save는 수행하지 않습니다.

#include "DataAuthoring/CFVehicleBuilderTab.h"

#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"

#include "CFEquipmentPresetData.h"
#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/MessageDialog.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
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
			return TEXT("메시 후보");
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed: return TEXT("관리 DA");
		case ECFVehicleManageState::PartiallyManaged: return TEXT("부분관리 DA");
		case ECFVehicleManageState::LegacyImported: return TEXT("레거시 DA");
		case ECFVehicleManageState::Unmanaged: return TEXT("미관리 DA");
		default: return TEXT("기타");
		}
	}

	// Vehicle Browser row의 분류가 항상 같은 의미색을 사용하도록 category 기반 accent를 반환합니다.
	FLinearColor VehicleRowAccentColor(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return FLinearColor(0.32f, 0.72f, 1.00f, 1.0f); // 파랑: 아직 VehicleData가 없는 Mesh 후보
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed:
			return FLinearColor(0.38f, 0.86f, 0.48f, 1.0f); // 초록: 정상 관리 DA
		case ECFVehicleManageState::PartiallyManaged:
			return FLinearColor(0.95f, 0.78f, 0.28f, 1.0f); // 노랑: 일부 관리 계약만 갖춘 DA
		case ECFVehicleManageState::LegacyImported:
			return FLinearColor(1.00f, 0.56f, 0.24f, 1.0f); // 주황: 레거시 import DA
		case ECFVehicleManageState::Unmanaged:
			return FLinearColor(0.96f, 0.38f, 0.34f, 1.0f); // 빨강: 아직 Recipe 관리 밖의 DA
		default:
			return FLinearColor(0.70f, 0.70f, 0.74f, 1.0f); // 회색: 기타/알 수 없음
		}
	}

	// 작업 대상 목록을 관리 상태별로 모으기 위한 stable category rank입니다.
	int32 VehicleRowCategorySortRank(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return 40;
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed: return 0;
		case ECFVehicleManageState::PartiallyManaged: return 10;
		case ECFVehicleManageState::LegacyImported: return 20;
		case ECFVehicleManageState::Unmanaged: return 30;
		default: return 50;
		}
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
			? FString::Printf(TEXT("Recipe | %s"), *Entry.RecipePath.ToString())
			: TEXT("Recipe 없음 | 미관리 VehicleData");
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
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 8.0f)
					[
						SNew(SButton)
							.Text(LOCTEXT("BeginNewVehicle", "+ 새 차량 만들기"))
							.ToolTipText(LOCTEXT("BeginNewVehicleTooltip", "기존 목록 선택과 독립적인 신규 차량 제작 모드로 들어갑니다. 이 버튼만으로 Asset을 생성하거나 저장/적용하지 않습니다."))
							.OnClicked(this, &SCFVehicleBuilderTab::HandleBeginNewVehicleEntry)
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
						.Padding(0.0f, 0.0f, 0.0f, 10.0f)
						[
							// USER가 표시한 빨간 영역: Target / 현재 Step / State / Summary / 지금 할 일을 한 프레임으로 고정합니다.
							SNew(SBorder)
							.Padding(12.0f)
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
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetCurrentStepTitle)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetCurrentStepStateText)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetCurrentStepSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SBorder)
									.Padding(10.0f)
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
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(SBorder)
										.Visibility(this, &SCFVehicleBuilderTab::GetNewVehicleCreationOptionsVisibility)
										.Padding(10.0f)
										[
											SNew(SVerticalBox)

											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 6.0f)
											[
												SNew(STextBlock)
													.Text(LOCTEXT("NewVehicleStartModeHeader", "신규 차량 시작 방식"))
													.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
											]

											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 8.0f)
											[
												SNew(STextBlock)
													.Text(LOCTEXT("NewVehicleStartModeIntro", "차체 메시를 비우면 Blank Start입니다. 원하는 StaticMesh를 지정하면 Recipe AssetIntent에만 기록합니다. 이미 다른 차량이 사용하는 Mesh도 허용하며 새 VehicleData에는 지금 자동 적용하지 않습니다."))
													.AutoWrapText(true)
											]

											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 6.0f)
											[
												SNew(SObjectPropertyEntryBox)
													.AllowedClass(UStaticMesh::StaticClass())
													.ObjectPath(this, &SCFVehicleBuilderTab::GetNewVehicleChassisMeshPath)
													.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleNewVehicleChassisMeshChanged)
													.AllowClear(true)
											]

											+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
													.Text_Lambda([this]()
													{
														if (!ViewModel.IsValid() || !ViewModel->GetNewVehicleChassisMeshPath().IsValid())
														{
															return LOCTEXT("NewVehicleBlankMode", "현재 시작 방식: Blank — Chassis Mesh 없음");
														}
														return FText::FromString(FString::Printf(TEXT("현재 시작 방식: Chassis Mesh — %s"), *ViewModel->GetNewVehicleChassisMeshPath().ToString()));
													})
													.AutoWrapText(true)
											]
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 4.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("VehicleIdLabel", "Vehicle ID"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f)
								[
									SAssignNew(NewVehicleIdTextBox, SEditableTextBox)
										.HintText(LOCTEXT("VehicleIdHint", "예: WagonPolice, Wagon_Police, SUV01"))
										.ToolTipText(LOCTEXT("VehicleIdTooltip", "Vehicle ID만 입력하면 VehicleData/Recipe package와 object 이름을 자동 제안합니다. 영문자, 숫자, _만 사용할 수 있습니다."))
										.OnTextChanged(this, &SCFVehicleBuilderTab::HandleVehicleIdTextChanged)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetVehicleIdValidationText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("AdvancedCreateIdentityTitle", "고급 Asset 경로 설정"))
										.BodyContent()
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewDefinitionPackageTextBox, SEditableTextBox)
													.HintText(LOCTEXT("DefinitionPackageHint", "VehicleData package: /Game/.../DA_Vehicle_Name"))
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewDefinitionNameTextBox, SEditableTextBox)
													.HintText(LOCTEXT("DefinitionNameHint", "VehicleData Asset 이름"))
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewRecipePackageTextBox, SEditableTextBox)
													.HintText(LOCTEXT("RecipePackageHint", "Recipe package: /Game/.../DA_Recipe_Name"))
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewRecipeNameTextBox, SEditableTextBox)
													.HintText(LOCTEXT("RecipeNameHint", "Recipe Asset 이름"))
											]
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SButton)
										.Text(this, &SCFVehicleBuilderTab::GetCreateVehicleButtonText)
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

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 8.0f, 0.0f, 0.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("RefreshReferenceEvidence", "Reference Evidence 갱신 검토"))
										.ToolTipText(LOCTEXT("RefreshReferenceEvidenceTooltip", "현재 existing Evidence와 다른 complete ResearchDraft를 mutation0 R1 preview한 뒤, explicit AuthoringWrite 승인 시에만 research payload를 교체합니다. EvidenceId/Recipe/Target binding과 Profile/VehicleData는 유지하고 자동 저장하지 않습니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleReferenceEvidenceRefresh)
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
									SAssignNew(MeshPreparationPickerHost, SBox)
									[
										BuildMeshPreparationPickerFields()
									]
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
						.FillHeight(1.0f)
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetSocketPreparationVisibility)
							.Padding(12.0f)
							[
								SNew(SScrollBox)

								+ SScrollBox::Slot()
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
											"Wheel/Hardpoint Socket 이름은 한 글자라도 다르면 매칭되지 않습니다. 아래 exact 이름을 복사하거나, 누락된 경우 '추가 후 편집'을 눌러 현재 Chassis 원점에 정확한 이름의 Socket을 만들 수 있습니다. 위치·회전·스케일은 Static Mesh Editor에서 USER가 직접 맞추며 Builder는 자동 배치/자동 저장하지 않습니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 6.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("OpenChassisSocketEditor", "Chassis Socket 편집하기"))
										.ToolTipText(LOCTEXT("OpenChassisSocketEditorTooltip", "Wheel과 Hardpoint 모두 같은 current Chassis StaticMesh의 Socket을 사용합니다. 메시를 열어 위치·회전·Scale을 직접 편집하고 저장하세요. +X Forward / +Y Right / +Z Up 기준입니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenChassisSocketEditor)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("SharedChassisSocketWarning", "주의: Chassis Socket은 차량별 데이터가 아니라 StaticMesh 자산에 저장됩니다. 같은 Chassis Mesh를 사용하는 다른 차량도 이 Socket 위치를 공유합니다."))
										.AutoWrapText(true)
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
								.Padding(0.0f, 14.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("HardpointPlanHeader", "장비 장착 위치 / Hardpoint Plan"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SAssignNew(HardpointPlanningHost, SBox)
									[
										BuildHardpointPlanningPanel()
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("HardpointAxisGuide", "Hardpoint Socket 축 기준: +X / Red = Forward·기본 발사 방향, +Y / Green = Right, +Z / Blue = Up. Builder는 Socket Rotation을 자동 추론하지 않습니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 12.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("OptionalSocketHeader", "파괴 FX Socket (선택)"))
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
										.Text(LOCTEXT("CopyOptionalSocketNames", "파괴 FX Socket 이름 복사"))
										.ToolTipText(LOCTEXT("CopyOptionalSocketNamesTooltip", "현재 Recipe가 요구하는 파괴 FX Socket 이름을 복사합니다. 차량 파괴 효과가 별도 Socket 위치를 사용할 때만 필요합니다."))
										.IsEnabled(this, &SCFVehicleBuilderTab::CanCopyOptionalSocketNames)
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyOptionalSocketNames)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"SocketPreparationBoundary",
											"Socket 편집을 마치고 Static Mesh를 직접 저장한 뒤 아래 '현재 상태 다시 확인'을 누르세요. 파괴 FX Socket은 차량이 파괴될 때 효과를 붙일 별도 위치가 필요한 경우에만 사용하면 됩니다."))
										.AutoWrapText(true)
									]
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
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SAssignNew(MountPlanningHost, SBox)
									[
										BuildMountPlanningPanel()
									]
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
											"위 Standard Mount 패널은 explicit '장착 규칙 반영/삭제'에서 Recipe MountIntents만 typed write합니다. 나머지 8영역 Guidance와 Socket 위치는 read-only입니다. Target VehicleData Apply는 Step 7 Final Review가 소유하고 Builder는 자동 Save하지 않습니다."))
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
									// Final Review는 수십 개 Field Diff를 표시할 수 있으므로 상세 본문만 bounded scroll 영역으로 제한합니다.
									SNew(SBox)
									.HeightOverride(430.0f)
									[
										SNew(SScrollBox)

										+ SScrollBox::Slot()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetFinalReviewSummaryText)
												.AutoWrapText(true)
										]
									]
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

	// 같은 관리 분류끼리 모으고, 같은 분류 안에서는 표시 이름으로 정렬해 색상의 의미와 목록 구조를 일치시킵니다.
	VehicleRows.Sort([](const FVehicleRowPtr& Left, const FVehicleRowPtr& Right)
	{
		if (!Left.IsValid() || !Right.IsValid())
		{
			return Left.IsValid();
		}
		const int32 LeftRank = VehicleRowCategorySortRank(*Left);
		const int32 RightRank = VehicleRowCategorySortRank(*Right);
		if (LeftRank != RightRank)
		{
			return LeftRank < RightRank;
		}
		return VehicleRowTitle(*Left) < VehicleRowTitle(*Right);
	});

	if (VehicleListView.IsValid())
	{
		VehicleListView->RequestListRefresh();

		// ViewModel current selection과 같은 fresh Browser row를 다시 찾아 좌측 highlight도 exact identity로 맞춥니다.
		if (!ViewModel->IsNewVehicleEntryActive() && ViewModel->HasSelection())
		{
			// 현재 Builder가 authoritative하게 선택한 exact target입니다.
			const FCFVehicleListEntry& CurrentEntry = ViewModel->GetSelectedEntry();
			// fresh Browser row 중 current target과 exact identity가 같은 row입니다.
			FVehicleRowPtr CurrentRow;
			for (const FVehicleRowPtr& Row : VehicleRows)
			{
				if (!Row.IsValid())
				{
					continue;
				}

				// Managed row는 Definition+Recipe exact pair가 모두 같아야 합니다.
				const bool bSameManagedIdentity = CurrentEntry.DefinitionPath.IsValid()
					&& Row->DefinitionPath == CurrentEntry.DefinitionPath
					&& Row->RecipePath == CurrentEntry.RecipePath;
				// Mesh-only row는 후보 flag와 Chassis exact path가 모두 같아야 합니다.
				const bool bSameMeshCandidate = CurrentEntry.bMeshOnlyCandidate
					&& Row->bMeshOnlyCandidate
					&& Row->ChassisMeshPath == CurrentEntry.ChassisMeshPath;
				if (bSameManagedIdentity || bSameMeshCandidate)
				{
					CurrentRow = Row;
					break;
				}
			}

			if (CurrentRow.IsValid())
			{
				VehicleListView->SetSelection(CurrentRow, ESelectInfo::Direct);
				VehicleListView->RequestScrollIntoView(CurrentRow);
			}
		}
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("대상 %d개를 fresh read했습니다. Product Asset/VehicleData는 변경하지 않았습니다."),
		VehicleRows.Num()));
	return FReply::Handled();
}

// 기존 Browser selection과 독립적으로 신규 차량 제작 진입 상태를 시작합니다.
FReply SCFVehicleBuilderTab::HandleBeginNewVehicleEntry()
{
	if (!ViewModel.IsValid())
	{
		LastStatusText = LOCTEXT("BeginNewVehicleMissingVM", "Builder ViewModel이 없습니다.");
		return FReply::Handled();
	}

	ViewModel->BeginNewVehicleEntry();
	ViewModel->SetNewVehicleBlankStart();
	SyncCreationFieldsFromSelection();
	if (VehicleListView.IsValid())
	{
		VehicleListView->ClearSelection();
	}

	LastStatusText = LOCTEXT(
		"BeginNewVehicleReady",
		"새 차량 제작 모드로 들어왔습니다. 아직 Asset 생성/Save/VehicleData Apply는 수행하지 않았습니다.");
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
		// Resolver fail-closed여도 selection/Recipe binding은 current target으로 전환됐을 수 있으므로 이전 차량 picker 표시를 남기지 않습니다.
		SyncCreationFieldsFromSelection();
		SyncMeshPreparationFieldsFromRecipe();
		RefreshMeshPreparationPickerPresentation();
		RefreshHardpointPlanningPresentation();
		SyncMountPlanningDraftsFromRecipe();
		RefreshMountPlanningPresentation();
		LastStatusText = FText::FromString(FString::Printf(TEXT("대상 선택 실패: %s"), *Error));
		return;
	}

	SyncCreationFieldsFromSelection();
	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
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
						.ColorAndOpacity(FLinearColor(0.92f, 0.92f, 0.94f, 1.0f))
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
		// Resolve fail-closed여도 current Recipe 자체는 유효할 수 있으므로 stale ObjectPicker 표시를 남기지 않습니다.
		SyncMeshPreparationFieldsFromRecipe();
		RefreshMeshPreparationPickerPresentation();
		RefreshHardpointPlanningPresentation();
		SyncMountPlanningDraftsFromRecipe();
		RefreshMountPlanningPresentation();
		LastStatusText = FText::FromString(FString::Printf(TEXT("현재 상태 확인 실패: %s"), *Error));
		return FReply::Handled();
	}

	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = LOCTEXT("RefreshPass", "현재 상태를 fresh read했습니다. 자동 저장/적용은 수행하지 않았습니다.");
	return FReply::Handled();
}

// ViewModel forward-progress contract를 만족하는 현재 Step에서 다음 Step으로 이동합니다.
FReply SCFVehicleBuilderTab::HandleNextStep()
{
	if (ViewModel.IsValid() && ViewModel->MoveNextStep())
	{
		LastStatusText = LOCTEXT("MovedNext", "현재 단계의 forward-progress 조건을 확인하고 다음 화면으로 이동했습니다.");
	}
	return FReply::Handled();
}

// Explicit New Vehicle 또는 Mesh-only Quick Start의 VehicleData+Recipe proposal을 검토하고 explicit 승인 뒤 공통 commit/adoption 경로를 실행합니다.
FReply SCFVehicleBuilderTab::HandleCreateVehicleFromMesh()
{
	if (!ViewModel.IsValid()
		|| (!ViewModel->IsNewVehicleEntryActive() && !ViewModel->IsMeshOnlyCandidate())
		|| !NewVehicleIdTextBox.IsValid()
		|| !NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return FReply::Handled();
	}

	// Explicit New Vehicle와 기존 Mesh-only Quick Start를 구분하는 현재 진입 상태입니다.
	const bool bExplicitNewVehicle = ViewModel->IsNewVehicleEntryActive();
	// Preview/build diagnostic입니다.
	FString Error;
	if (!ViewModel->SetVehicleCreationId(NewVehicleIdTextBox->GetText().ToString(), Error))
	{
		VehicleIdValidationText = FText::FromString(Error);
		LastStatusText = FText::FromString(FString::Printf(TEXT("Vehicle ID 오류: %s"), *Error));
		return FReply::Handled();
	}

	// Mutation0 two-record creation preview입니다.
	FCFVehicleRecordCreatePreview Preview;
	// 현재 진입 방식에 맞는 canonical Guided create request를 준비했는지 여부입니다.
	const bool bPreviewReady = bExplicitNewVehicle
		? ViewModel->PrepareNewVehicleRecordCreate(
			NewDefinitionPackageTextBox->GetText().ToString(),
			NewDefinitionNameTextBox->GetText().ToString(),
			NewRecipePackageTextBox->GetText().ToString(),
			NewRecipeNameTextBox->GetText().ToString(),
			Preview,
			Error)
		: ViewModel->PrepareSelectedMeshRecordCreate(
			NewDefinitionPackageTextBox->GetText().ToString(),
			NewDefinitionNameTextBox->GetText().ToString(),
			NewRecipePackageTextBox->GetText().ToString(),
			NewRecipeNameTextBox->GetText().ToString(),
			Preview,
			Error);
	if (!bPreviewReady)
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("생성 검토 실패: %s"), *Error));
		return FReply::Handled();
	}

	// Review dialog에 표시할 optional Chassis exact object path입니다. Empty면 Blank Start입니다.
	const FSoftObjectPath ReviewChassisMeshPath = bExplicitNewVehicle
		? ViewModel->GetNewVehicleChassisMeshPath()
		: ViewModel->GetSelectedEntry().ChassisMeshPath;
	// USER가 실제 생성 범위를 읽고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("Guided Builder — 신규 차량 레코드 생성 검토\n\n시작 방식: %s\n차체 메시: %s\n생성할 VehicleData: %s\n생성할 Recipe: %s\nTransmission 정책: VehicleSpecificRequired\n\nReference Evidence: 아직 생성 안 함\nprivate Profile 4종: 아직 생성 안 함\n차급 자동 추론: 안 함\n물리/밸런스 자동 추론: 안 함\n새 VehicleData Chassis 자동 Apply: 안 함\n자동 저장: 안 함\n\nMesh가 지정된 경우 새 Recipe AssetIntent에만 기록합니다. 이미 다른 차량이 사용하는 Mesh도 허용합니다. 지금은 Builder의 core 2-record만 생성합니다. 계속하시겠습니까?"),
		ReviewChassisMeshPath.IsValid() ? TEXT("Chassis Mesh") : TEXT("Blank"),
		ReviewChassisMeshPath.IsValid() ? *ReviewChassisMeshPath.ToString() : TEXT("없음"),
		*Preview.ProspectiveDefinitionPath.ToString(),
		*Preview.ProspectiveRecipePath.ToString()));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("CreateCancelled", "생성을 취소했습니다. 변경된 Asset은 없습니다.");
		return FReply::Handled();
	}

	// Explicit USER approval이 붙은 exact two-record terminal result입니다.
	FCFVehicleRecordCreateResult Result;
	// Record creation 이후 exact Browser/Builder adoption 성공 여부입니다.
	bool bBuilderAdopted = false;
	// Record creation과 분리해서 보고할 post-create adoption diagnostic입니다.
	FString AdoptionError;
	if (!ViewModel->ExecutePreparedNewVehicleRecordCreate(Result, bBuilderAdopted, AdoptionError, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("VehicleData+Recipe 생성 실패: %s"), *Error));
		return FReply::Handled();
	}

	// 생성된 Asset은 이미 존재합니다. Browser row copy/highlight를 fresh read하되 adoption 실패를 생성 실패로 오인하지 않습니다.
	HandleRefreshVehicles();
	if (!bBuilderAdopted)
	{
		// 생성 성공 뒤 rollback하지 않고 exact Created record와 adoption 오류를 분리해 표시합니다.
		const FString CreatedDefinitionPath = Result.CreatedDefinition ? FSoftObjectPath(Result.CreatedDefinition).ToString() : TEXT("<null>");
		// 생성 성공 뒤 rollback하지 않고 exact Created Recipe를 표시합니다.
		const FString CreatedRecipePath = Result.CreatedRecipe ? FSoftObjectPath(Result.CreatedRecipe).ToString() : TEXT("<null>");
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("VehicleData+Recipe records는 생성됐지만 Builder adoption에 실패했습니다. 생성 Asset은 롤백하지 않았고 자동 저장/VehicleData Apply도 하지 않았습니다.\n%s\nDefinition=%s\nRecipe=%s"),
			*AdoptionError,
			*CreatedDefinitionPath,
			*CreatedRecipePath));
		return FReply::Handled();
	}

	SyncCreationFieldsFromSelection();
	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("VehicleData+Recipe를 생성하고 exact 새 managed 차량을 Builder current target/Browser row로 adoption했습니다. 자동 저장과 VehicleData Chassis Apply는 하지 않았습니다. Step 1 Reference 준비 후 정상 Builder 단계를 진행하세요.\n%s"),
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

// Step 1 loaded ResearchDraft로 Existing Reference Evidence complete replacement R1을 검토하고 explicit USER 승인 뒤 commit합니다.
FReply SCFVehicleBuilderTab::HandleReferenceEvidenceRefresh()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current Evidence + loaded ResearchDraft로 만든 mutation0 R1 preview입니다.
	FCFBuilderEvidenceRefreshPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareReferenceEvidenceRefresh(Preview, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Reference Evidence 갱신 검토 실패: %s"), *Error));
		return FReply::Handled();
	}

	if (Preview.Operation.Status == ECFAuthoringOpStatus::NoChange)
	{
		LastStatusText = LOCTEXT("ReferenceEvidenceRefreshNoChange", "Loaded Research Draft와 current Reference Evidence가 동일합니다. 갱신할 내용이 없습니다.");
		return FReply::Handled();
	}

	// USER가 current→prospective Evidence semantic 변경과 mutation boundary를 확인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"Guided Builder — Reference Evidence 갱신 검토\n\n"
			"Current EvidenceFingerprint:\n%s\n\n"
			"Prospective EvidenceFingerprint:\n%s\n\n"
			"Claim 수: %d → %d\n"
			"Unknown Fact 수: %d → %d\n\n"
			"EvidenceId: 유지\n"
			"Recipe/Target binding: 유지\n"
			"Profile mutation: 안 함\n"
			"Recipe mutation: 안 함\n"
			"VehicleData mutation/Apply: 안 함\n"
			"자동 저장: 안 함\n"
			"자동 재시도: 안 함\n\n"
			"중요: EvidenceFingerprint가 바뀌면 기존 Reference review token과 Builder Profile receipt는 stale이 됩니다.\n"
			"갱신 후 이 Reference Set을 다시 확인하고 Physics Proposal을 새 Evidence 기준으로 다시 검토해야 합니다.\n\n"
			"Loaded Research Draft의 complete research payload로 existing Evidence를 교체하시겠습니까?"),
		*Preview.CurrentEvidenceFingerprint,
		*Preview.ProspectiveEvidenceFingerprint,
		Preview.CurrentClaimCount,
		Preview.ProspectiveClaimCount,
		Preview.CurrentUnknownFactCount,
		Preview.ProspectiveUnknownFactCount));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("ReferenceEvidenceRefreshCancelled", "Reference Evidence 갱신을 취소했습니다. Prepared approval은 실행하지 않았습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved Evidence Refresh terminal result입니다.
	FCFBuilderEvidenceRefreshResult Result;
	if (!ViewModel->ExecutePreparedEvidenceRefresh(Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Reference Evidence 갱신 실패: %s"), *Error));
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("Reference Evidence research payload를 갱신했습니다. EvidenceId/Recipe/Target/Profile/VehicleData는 유지했고 자동 저장하지 않았습니다. 이 Reference Set을 다시 확인한 뒤 새 Physics Proposal을 검토하세요.\n%s"),
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
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("Mesh 설정을 Recipe에 반영했습니다. VehicleData Apply/자동 저장은 하지 않았습니다. %s"),
		*CommitResult.Message));
	return FReply::Handled();
}

// Explicit New Vehicle의 optional Chassis StaticMesh picker 변경을 BuilderVM canonical creation state에 반영합니다.
void SCFVehicleBuilderTab::HandleNewVehicleChassisMeshChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid() || !ViewModel->IsNewVehicleEntryActive())
	{
		return;
	}

	ViewModel->SetNewVehicleChassisMeshPath(
		AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath());
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

// Step 2에서 explicit commit 전 pending Chassis StaticMesh를 Asset Editor에서 직접 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenPendingChassisMesh()
{
	return OpenStaticMeshAssetPath(
		PendingChassisMeshPath,
		TEXT("Step 2 pending Chassis Mesh"),
		TEXT("아직 'Mesh 설정 반영' 전의 pending 선택값을 열었습니다. Chassis 자산 편집은 같은 StaticMesh를 쓰는 다른 차량에도 영향을 줄 수 있습니다."));
}

// Step 2에서 role별 pending Wheel StaticMesh를 열고 optional role이 비었으면 pending FL fallback을 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenPendingWheelMesh(const int32 WheelRoleIndex)
{
	const FSoftObjectPath EffectiveMeshPath = GetEffectivePendingWheelMeshPath(WheelRoleIndex);
	const bool bUsesFLFallback = WheelRoleIndex > 0
		&& PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex)
		&& !PendingWheelMeshPaths[WheelRoleIndex].IsValid()
		&& PendingWheelMeshPaths.IsValidIndex(0)
		&& PendingWheelMeshPaths[0].IsValid();

	const TCHAR* RoleLabels[] = {TEXT("FL"), TEXT("FR"), TEXT("RL"), TEXT("RR")};
	const FString RoleLabel = WheelRoleIndex >= 0 && WheelRoleIndex < UE_ARRAY_COUNT(RoleLabels)
		? FString(RoleLabels[WheelRoleIndex])
		: FString::Printf(TEXT("Role%d"), WheelRoleIndex);
	const FString CompletionNote = bUsesFLFallback
		? FString::Printf(TEXT("%s pending 값이 비어 있어 current pending FL Mesh fallback을 열었습니다. 아직 Recipe commit은 수행하지 않았습니다."), *RoleLabel)
		: FString::Printf(TEXT("%s role의 pending Mesh를 열었습니다. 아직 Recipe commit은 수행하지 않았습니다."), *RoleLabel);

	return OpenStaticMeshAssetPath(
		EffectiveMeshPath,
		FString::Printf(TEXT("Step 2 pending %s Wheel Mesh"), *RoleLabel),
		CompletionNote);
}

// Step 2 pending Chassis Mesh open 버튼 활성 여부입니다.
bool SCFVehicleBuilderTab::CanOpenPendingChassisMesh() const
{
	return PendingChassisMeshPath.IsValid();
}

// Step 2 role별 effective pending Wheel Mesh open 버튼 활성 여부입니다.
bool SCFVehicleBuilderTab::CanOpenPendingWheelMesh(const int32 WheelRoleIndex) const
{
	return GetEffectivePendingWheelMeshPath(WheelRoleIndex).IsValid();
}

// Step 2 optional Wheel fallback 여부를 포함하는 동적 open tooltip입니다.
FText SCFVehicleBuilderTab::GetPendingWheelMeshOpenTooltip(const int32 WheelRoleIndex) const
{
	if (!PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex))
	{
		return LOCTEXT("PendingWheelOpenInvalidRole", "유효하지 않은 Wheel role입니다.");
	}
	if (PendingWheelMeshPaths[WheelRoleIndex].IsValid())
	{
		return LOCTEXT("PendingWheelOpenExact", "현재 Object Picker에 보이는 pending Wheel StaticMesh를 직접 엽니다. 'Mesh 설정 반영' 전이어도 이 pending 자산을 엽니다.");
	}
	if (WheelRoleIndex > 0 && PendingWheelMeshPaths.IsValidIndex(0) && PendingWheelMeshPaths[0].IsValid())
	{
		return LOCTEXT("PendingWheelOpenFallback", "이 optional Wheel role은 비어 있으므로 current pending FL Wheel Mesh fallback을 엽니다. Runtime/Builder fallback 의미를 그대로 보여 줍니다.");
	}
	return LOCTEXT("PendingWheelOpenMissing", "열 수 있는 pending Wheel StaticMesh가 없습니다.");
}

// Step 2 direct role path 또는 optional role의 pending FL fallback path를 반환합니다.
FSoftObjectPath SCFVehicleBuilderTab::GetEffectivePendingWheelMeshPath(const int32 WheelRoleIndex) const
{
	if (!PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex))
	{
		return FSoftObjectPath();
	}
	if (PendingWheelMeshPaths[WheelRoleIndex].IsValid())
	{
		return PendingWheelMeshPaths[WheelRoleIndex];
	}
	if (WheelRoleIndex > 0 && PendingWheelMeshPaths.IsValidIndex(0))
	{
		return PendingWheelMeshPaths[0];
	}
	return FSoftObjectPath();
}

// Step 2 Chassis + FL/FR/RL/RR StaticMesh picker 묶음을 current pending state로 새로 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildMeshPreparationPickerFields()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
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
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SObjectPropertyEntryBox)
					.AllowedClass(UStaticMesh::StaticClass())
					.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingChassisMeshPath)
					.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleChassisMeshChanged)
					.AllowClear(false)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("OpenPendingChassisMesh", "열기"))
					.ToolTipText(LOCTEXT("OpenPendingChassisMeshTooltip", "현재 Object Picker의 pending Chassis StaticMesh를 직접 엽니다. 'Mesh 설정 반영' 전이어도 pending 자산을 엽니다."))
					.IsEnabled(this, &SCFVehicleBuilderTab::CanOpenPendingChassisMesh)
					.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenPendingChassisMesh)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(150.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("Step2SharedChassisWarning", "주의: Chassis StaticMesh 자산을 직접 편집하면 같은 Chassis Mesh를 사용하는 다른 차량에도 Mesh/Socket 변경이 공유됩니다. Builder는 차량별 복사본을 자동 생성하지 않습니다."))
				.AutoWrapText(true)
		]

		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(0, LOCTEXT("WheelMeshRoleFL", "FL / 앞왼쪽"), true)]
		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(1, LOCTEXT("WheelMeshRoleFR", "FR / 앞오른쪽"), false)]
		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(2, LOCTEXT("WheelMeshRoleRL", "RL / 뒤왼쪽"), false)]
		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(3, LOCTEXT("WheelMeshRoleRR", "RR / 뒤오른쪽"), false)];
}

// 차량 선택/refresh/commit 뒤 ObjectPicker presentation을 current Recipe pending state로 재생성합니다.
void SCFVehicleBuilderTab::RefreshMeshPreparationPickerPresentation()
{
	if (MeshPreparationPickerHost.IsValid())
	{
		MeshPreparationPickerHost->SetContent(BuildMeshPreparationPickerFields());
	}
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
		.Padding(0.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(SObjectPropertyEntryBox)
				.AllowedClass(UStaticMesh::StaticClass())
				.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingWheelMeshPath, WheelRoleIndex)
				.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleWheelMeshChanged, WheelRoleIndex)
				.AllowClear(!bRequired)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("OpenPendingWheelMesh", "열기"))
				.ToolTipText(this, &SCFVehicleBuilderTab::GetPendingWheelMeshOpenTooltip, WheelRoleIndex)
				.IsEnabled(this, &SCFVehicleBuilderTab::CanOpenPendingWheelMesh, WheelRoleIndex)
				.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenPendingWheelMesh, WheelRoleIndex)
		];
}

// Step 3 Wheel/Hardpoint가 공유하는 current Chassis StaticMesh를 하나의 Socket 편집 진입으로 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenChassisSocketEditor()
{
	return OpenCurrentChassisMeshForSocketEditing();
}

// Step 3 exact missing SocketName을 current Chassis StaticMesh 원점에 explicit 생성하고 Asset Editor를 엽니다.
FReply SCFVehicleBuilderTab::HandleAddChassisSocket(const FName SocketName)
{
	return AddChassisSocketAtOriginAndOpen(SocketName);
}

// Current Chassis StaticMesh를 공통 Socket 편집 문맥으로 엽니다.
FReply SCFVehicleBuilderTab::OpenCurrentChassisMeshForSocketEditing()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	return OpenStaticMeshAssetPath(
		ViewModel->GetCurrentChassisMeshPath(),
		TEXT("Chassis Socket"),
		TEXT("Wheel과 Hardpoint는 같은 committed Chassis StaticMesh의 Socket을 사용합니다. 같은 Chassis Mesh를 쓰는 다른 차량과 Socket 위치를 공유하며 Builder는 자동 배치/자동 저장하지 않습니다."));
}

// Exact missing Socket을 current Chassis에 transaction으로 추가하고 편집기를 여는 공통 backend입니다.
FReply SCFVehicleBuilderTab::AddChassisSocketAtOriginAndOpen(const FName SocketName)
{
	if (!ViewModel.IsValid() || SocketName.IsNone())
	{
		LastStatusText = LOCTEXT("AddChassisSocketInvalidName", "추가할 exact SocketName이 없습니다.");
		return FReply::Handled();
	}

	const FSoftObjectPath MeshPath = ViewModel->GetCurrentChassisMeshPath();
	UObject* MeshObject = MeshPath.ResolveObject();
	if (!MeshObject)
	{
		MeshObject = MeshPath.TryLoad();
	}
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject);
	if (!StaticMesh)
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Socket을 추가할 current Chassis StaticMesh를 읽을 수 없습니다: %s"), *MeshPath.ToString()));
		return FReply::Handled();
	}

	// Exact Socket이 이미 있으면 중복 생성하지 않고 편집기만 엽니다.
	if (StaticMesh->FindSocket(SocketName))
	{
		return OpenStaticMeshAssetPath(
			MeshPath,
			TEXT("Chassis Socket"),
			FString::Printf(TEXT("Socket %s는 이미 존재합니다. 위치/회전만 확인하세요."), *SocketName.ToString()));
	}

	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "AddBuilderChassisSocket", "차량 Builder Chassis Socket 추가"));
	StaticMesh->Modify();
	UStaticMeshSocket* NewSocket = NewObject<UStaticMeshSocket>(StaticMesh, NAME_None, RF_Transactional);
	if (!NewSocket)
	{
		Transaction.Cancel();
		LastStatusText = LOCTEXT("AddChassisSocketCreateFailed", "StaticMesh Socket object를 만들 수 없습니다.");
		return FReply::Handled();
	}

	NewSocket->SocketName = SocketName;
	NewSocket->RelativeLocation = FVector::ZeroVector;
	NewSocket->RelativeRotation = FRotator::ZeroRotator;
	NewSocket->RelativeScale = FVector::OneVector;
	StaticMesh->Sockets.Add(NewSocket);
	StaticMesh->MarkPackageDirty();
	StaticMesh->PostEditChange();

	// Fresh AssetSnapshot으로 Socket found-state를 다시 읽되, Resolver 상태 오류가 Socket 생성 자체를 rollback시키지는 않습니다.
	FString RefreshError;
	ViewModel->RefreshCurrentState(RefreshError);
	RefreshHardpointPlanningPresentation();

	const FString RefreshSuffix = RefreshError.IsEmpty()
		? FString()
		: FString::Printf(TEXT(" | 상태 재확인: %s"), *RefreshError);
	return OpenStaticMeshAssetPath(
		MeshPath,
		TEXT("Chassis Socket"),
		FString::Printf(
			TEXT("Socket %s를 원점에 exact 이름으로 추가했습니다. 위치/회전/Scale을 직접 맞춘 뒤 StaticMesh를 저장하세요. 자동 저장은 하지 않았습니다.%s"),
			*SocketName.ToString(),
			*RefreshSuffix));
}

// Current committed Chassis StaticMesh object의 live FindSocket truth를 반환합니다.
bool SCFVehicleBuilderTab::IsCurrentChassisSocketPresentLive(const FName SocketName) const
{
	if (!ViewModel.IsValid() || SocketName.IsNone())
	{
		return false;
	}

	const FSoftObjectPath MeshPath = ViewModel->GetCurrentChassisMeshPath();
	if (!MeshPath.IsValid())
	{
		return false;
	}

	UObject* MeshObject = MeshPath.ResolveObject();
	if (!MeshObject)
	{
		MeshObject = MeshPath.TryLoad();
	}
	const UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject);
	return StaticMesh && StaticMesh->FindSocket(SocketName) != nullptr;
}

// Exact Socket이 current Chassis에 없을 때만 `추가 후 편집` 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanAddCurrentChassisSocket(const FName SocketName) const
{
	return !SocketName.IsNone() && !IsCurrentChassisSocketPresentLive(SocketName);
}

// Step 2/3에서 exact StaticMesh path를 공통 Asset Editor backend로 엽니다.
FReply SCFVehicleBuilderTab::OpenStaticMeshAssetPath(
	const FSoftObjectPath& MeshPath,
	const FString& ContextLabel,
	const FString& CompletionNote)
{
	if (!MeshPath.IsValid())
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("%s: 열 수 있는 StaticMesh 경로가 없습니다."), *ContextLabel));
		return FReply::Handled();
	}

	UObject* MeshObject = MeshPath.ResolveObject();
	if (!MeshObject)
	{
		MeshObject = MeshPath.TryLoad();
	}
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject);
	if (!StaticMesh)
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("%s StaticMesh를 열 수 없습니다: %s"), *ContextLabel, *MeshPath.ToString()));
		return FReply::Handled();
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor
		? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()
		: nullptr;
	if (!AssetEditorSubsystem)
	{
		LastStatusText = LOCTEXT("OpenStaticMeshMissingSubsystem", "Asset Editor subsystem을 찾을 수 없습니다.");
		return FReply::Handled();
	}

	AssetEditorSubsystem->OpenEditorForAsset(StaticMesh);
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("%s를 열었습니다: %s | %s"),
		*ContextLabel,
		*MeshPath.ToString(),
		*CompletionNote));
	return FReply::Handled();
}

// Step 3 Hardpoint Plan Mode를 explicit USER action으로 기록합니다.
FReply SCFVehicleBuilderTab::HandleSetHardpointPlanMode(const ECFBuilderHardpointPlanMode PlanMode)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->CommitHardpointPlanMode(PlanMode, Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Hardpoint 계획 변경 실패: %s"), *Error));
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(Result.Message.IsEmpty()
		? TEXT("Hardpoint 계획을 Recipe에 반영했습니다. 자동 저장/VehicleData Apply는 수행하지 않았습니다.")
		: Result.Message);
	return FReply::Handled();
}

// Step 3 Standard physical category에 deterministic stable Hardpoint row를 추가합니다.
FReply SCFVehicleBuilderTab::HandleAddStandardHardpoint(const FName LocationCategory)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FCFHardpointIntent CreatedIntent;
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->AddStandardHardpoint(LocationCategory, CreatedIntent, Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Hardpoint 추가 실패: %s"), *Error));
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("장착 위치를 추가했습니다: %s / Socket %s. StaticMesh Socket은 자동 생성하지 않았습니다."),
		*CreatedIntent.LocationSlotId.ToString(),
		*CreatedIntent.SocketName.ToString()));
	return FReply::Handled();
}

// Step 3 exact Recipe Hardpoint row를 typed no-cascade remove lane으로 제거합니다.
FReply SCFVehicleBuilderTab::HandleRemoveHardpoint(const FName LocationSlotId)
{
	if (!ViewModel.IsValid() || LocationSlotId.IsNone())
	{
		return FReply::Handled();
	}

	// 삭제 전 USER에게 남은 StaticMesh Socket을 정확히 안내할 current Recipe binding입니다.
	FName SocketName = NAME_None;
	if (const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe())
	{
		if (const FCFHardpointIntent* ExistingIntent = Recipe->HardpointIntents.FindByPredicate([LocationSlotId](const FCFHardpointIntent& Intent)
		{
			return Intent.LocationSlotId == LocationSlotId;
		}))
		{
			SocketName = ExistingIntent->SocketName;
		}
	}
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->RemoveHardpointIntent(LocationSlotId, Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Hardpoint 제거 실패: %s"), *Error));
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = SocketName.IsNone()
		? FText::FromString(FString::Printf(
			TEXT("%s Recipe 장착 위치를 제거했습니다. 이 row의 stored SocketName은 None이었으며 Builder는 StaticMesh를 변경하지 않았습니다."),
			*LocationSlotId.ToString()))
		: FText::FromString(FString::Printf(
			TEXT("%s Recipe 장착 위치를 제거했습니다. StaticMesh Socket %s는 삭제하지 않았습니다. 필요하면 Static Mesh Editor에서 직접 관리하세요."),
			*LocationSlotId.ToString(),
			*SocketName.ToString()));
	return FReply::Handled();
}

// Step 3 Hardpoint row의 exact SocketName을 클립보드에 복사합니다.
FReply SCFVehicleBuilderTab::HandleCopyHardpointSocketName(const FName SocketName)
{
	if (!SocketName.IsNone())
	{
		const FString SocketNameText = SocketName.ToString();
		FPlatformApplicationMisc::ClipboardCopy(*SocketNameText);
		LastStatusText = FText::FromString(FString::Printf(TEXT("Hardpoint Socket 이름을 복사했습니다: %s"), *SocketNameText));
	}
	return FReply::Handled();
}

// Current Recipe/Mode에서 Step 3 Hardpoint planning subtree를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildHardpointPlanningPanel()
{
	TSharedRef<SVerticalBox> Panel = SNew(SVerticalBox);
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("HardpointPlanNoRecipe", "Hardpoint 계획을 표시할 managed Recipe가 없습니다."))
		];
		return Panel;
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const ECFBuilderHardpointPlanMode PlanMode = ViewModel->GetHardpointPlanMode();
	const TCHAR* ModeText = TEXT("알 수 없음");
	switch (PlanMode)
	{
	case ECFBuilderHardpointPlanMode::LegacyCompatible: ModeText = TEXT("기존 호환 모드"); break;
	case ECFBuilderHardpointPlanMode::Unspecified: ModeText = TEXT("미결정"); break;
	case ECFBuilderHardpointPlanMode::NoHardpoints: ModeText = TEXT("장착점 없음"); break;
	case ECFBuilderHardpointPlanMode::UseHardpoints: ModeText = TEXT("장착 위치 사용"); break;
	default: break;
	}

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
	[
		SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("현재 계획: %s | Recipe Hardpoint %d개 / Mount %d개"), ModeText, Recipe->HardpointIntents.Num(), Recipe->MountIntents.Num())))
			.AutoWrapText(true)
	];

	if (PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible)
	{
		Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
					.Text(LOCTEXT("LegacyHardpointPlanNotice", "기존 차량은 현재 Hardpoint/Mount 구조를 자동 migration하지 않습니다. 새 Standard Guided 계획을 사용하려면 아래 버튼으로 명시적으로 opt-in하세요."))
					.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SButton)
					.Text(LOCTEXT("OptInUseHardpoints", "장착 위치 사용으로 전환"))
					.OnClicked(this, &SCFVehicleBuilderTab::HandleSetHardpointPlanMode, ECFBuilderHardpointPlanMode::UseHardpoints)
			]
		];
	}
	else
	{
		const bool bCanChooseNoHardpoints = Recipe->HardpointIntents.IsEmpty() && Recipe->MountIntents.IsEmpty();
		Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("ChooseNoHardpoints", "장착점 없음"))
					.ToolTipText(LOCTEXT("ChooseNoHardpointsTooltip", "Hardpoint/Mount가 모두 비어 있을 때만 차량에 장착 위치가 없음을 명시합니다. 기존 항목을 자동 삭제하지 않습니다."))
					.IsEnabled(bCanChooseNoHardpoints && PlanMode != ECFBuilderHardpointPlanMode::NoHardpoints)
					.OnClicked(this, &SCFVehicleBuilderTab::HandleSetHardpointPlanMode, ECFBuilderHardpointPlanMode::NoHardpoints)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(6.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("ChooseUseHardpoints", "장착 위치 사용"))
					.IsEnabled(PlanMode != ECFBuilderHardpointPlanMode::UseHardpoints)
					.OnClicked(this, &SCFVehicleBuilderTab::HandleSetHardpointPlanMode, ECFBuilderHardpointPlanMode::UseHardpoints)
			]
		];
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::UseHardpoints)
	{
		Panel->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("StandardHardpointAddGuide", "추가할 물리 위치를 선택하세요. 생성 후 category/LocationSlotId는 Standard에서 고정되며 바꾸려면 삭제 후 다시 추가합니다."))
				.AutoWrapText(true)
		];

		TSharedRef<SHorizontalBox> CategoryButtons = SNew(SHorizontalBox);
		for (const FName Category : ViewModel->GetStandardHardpointCategories())
		{
			FString Label = Category.ToString();
			if (Category == TEXT("Top")) Label = TEXT("위");
			else if (Category == TEXT("Front")) Label = TEXT("앞");
			else if (Category == TEXT("Back")) Label = TEXT("뒤");
			else if (Category == TEXT("LeftSide")) Label = TEXT("왼쪽");
			else if (Category == TEXT("RightSide")) Label = TEXT("오른쪽");
			else if (Category == TEXT("Bottom")) Label = TEXT("아래");
			else if (Category == TEXT("Internal")) Label = TEXT("내부");

			CategoryButtons->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
					.Text(FText::FromString(Label))
					.ToolTipText(FText::FromString(FString::Printf(TEXT("%s category의 다음 stable ID를 max-used+1로 생성합니다."), *Category.ToString())))
					.OnClicked(this, &SCFVehicleBuilderTab::HandleAddStandardHardpoint, Category)
			];
		}
		Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)[CategoryButtons];
	}

	if (Recipe->HardpointIntents.IsEmpty())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(PlanMode == ECFBuilderHardpointPlanMode::UseHardpoints
					? LOCTEXT("HardpointRowsEmptyUse", "아직 장착 위치가 없습니다. 하나 이상 추가해야 Step 3을 완료할 수 있습니다.")
					: LOCTEXT("HardpointRowsEmpty", "현재 Recipe Hardpoint row가 없습니다."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	Panel->AddSlot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 2.0f)
	[
		SNew(STextBlock).Text(LOCTEXT("HardpointRowsHeader", "현재 Recipe 장착 위치"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
	];

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
			.Text(LOCTEXT("HardpointDeleteBoundary", "삭제는 Recipe의 장착 위치 row만 제거합니다. Chassis StaticMesh의 Socket은 그대로 유지됩니다."))
			.AutoWrapText(true)
	];

	for (const FCFHardpointIntent& Intent : Recipe->HardpointIntents)
	{
		// UI는 creation-time suggestion을 다시 추론하지 않고 Recipe에 실제 저장된 SocketName truth만 표시합니다.
		const FName StoredSocketName = Intent.SocketName;
		const FString SocketDisplayText = StoredSocketName.IsNone() ? TEXT("(없음)") : StoredSocketName.ToString();
		const bool bCanDelete = PlanMode != ECFBuilderHardpointPlanMode::LegacyCompatible;

		Panel->AddSlot().AutoHeight().Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(74.0f)[SNew(STextBlock).Text(FText::FromName(Intent.LocationCategory))]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(110.0f)[SNew(STextBlock).Text(FText::FromName(Intent.LocationSlotId))]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SEditableTextBox)
					.Text(FText::FromString(SocketDisplayText))
					.IsReadOnly(true)
					.ToolTipText(LOCTEXT("HardpointSocketExactNameTooltip", "이 문자열이 StaticMesh SocketName과 한 글자까지 정확히 일치해야 합니다."))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
					.Text_Lambda([this, StoredSocketName, PlanMode]()
					{
						if (StoredSocketName.IsNone())
						{
							return FText::FromString(PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible
								? TEXT("stored LocalTransform 보존")
								: TEXT("SocketName 없음 — 구조 교정 필요"));
						}
						return IsCurrentChassisSocketPresentLive(StoredSocketName)
							? FText::FromString(TEXT("현재 있음"))
							: FText::FromString(TEXT("없음 — 추가 필요"));
					})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("AddHardpointSocketAndEdit", "추가 후 편집"))
					.ToolTipText(LOCTEXT("AddHardpointSocketAndEditTooltip", "현재 Chassis에 이 exact 이름의 Socket이 없을 때 원점에 생성하고 Static Mesh Editor를 엽니다. 위치/회전은 직접 맞추고 저장하세요."))
					.IsEnabled_Lambda([this, StoredSocketName]() { return CanAddCurrentChassisSocket(StoredSocketName); })
					.OnClicked(this, &SCFVehicleBuilderTab::HandleAddChassisSocket, StoredSocketName)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("CopyHardpointSocket", "복사"))
					.IsEnabled(!StoredSocketName.IsNone())
					.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyHardpointSocketName, StoredSocketName)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("RemoveHardpointRow", "장착 위치만 삭제"))
					.ToolTipText(LOCTEXT("RemoveHardpointRowTooltip", "Recipe의 이 Hardpoint/장착 위치 row만 삭제합니다. Chassis StaticMesh Socket은 그대로 유지됩니다. 이 Hardpoint를 참조하는 Mount가 남아 있으면 삭제는 차단됩니다."))
					.IsEnabled(bCanDelete && !Intent.LocationSlotId.IsNone())
					.OnClicked(this, &SCFVehicleBuilderTab::HandleRemoveHardpoint, Intent.LocationSlotId)
			]
		];
	}

	return Panel;
}

// Mode/add/remove/selection/refresh 뒤 Hardpoint planning subtree를 current truth로 교체합니다.
void SCFVehicleBuilderTab::RefreshHardpointPlanningPresentation()
{
	if (HardpointPlanningHost.IsValid())
	{
		HardpointPlanningHost->SetContent(BuildHardpointPlanningPanel());
	}
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

	// Hardpoint 표와 중복되지 않는 current Recipe의 파괴 FX Socket 이름입니다.
	const TArray<FName> OptionalSocketNames = ViewModel->GetConditionalNonHardpointSocketNames();
	if (OptionalSocketNames.IsEmpty())
	{
		LastStatusText = LOCTEXT("CopyOptionalSocketEmpty", "현재 Recipe가 요구하는 별도 파괴 FX Socket 이름이 없습니다.");
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
		TEXT("파괴 FX Socket 이름 %d개를 줄바꿈 목록으로 복사했습니다."),
		OptionalSocketNames.Num()));
	return FReply::Handled();
}

// Step 3 required Wheel Socket 한 행의 역할/이름/현재 상태/추가/복사 UI를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildRequiredWheelSocketRow(const int32 WheelRoleIndex, const FText& RoleLabel)
{
	const TArray<FName> RequiredSocketNames = ViewModel.IsValid() ? ViewModel->GetRequiredWheelSocketNames() : TArray<FName>();
	const FName SocketName = RequiredSocketNames.IsValidIndex(WheelRoleIndex) ? RequiredSocketNames[WheelRoleIndex] : NAME_None;

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
				.ToolTipText(LOCTEXT("WheelSocketExactNameTooltip", "이 문자열이 StaticMesh SocketName과 한 글자까지 정확히 일치해야 합니다."))
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
		.Padding(0.0f, 2.0f, 4.0f, 2.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("AddWheelSocketAndEdit", "추가 후 편집"))
				.ToolTipText(LOCTEXT("AddWheelSocketAndEditTooltip", "현재 Chassis에 이 exact Wheel Socket이 없을 때 원점에 생성하고 Static Mesh Editor를 엽니다. 위치/회전/Scale은 직접 맞추고 저장하세요."))
				.IsEnabled_Lambda([this, SocketName]() { return CanAddCurrentChassisSocket(SocketName); })
				.OnClicked(this, &SCFVehicleBuilderTab::HandleAddChassisSocket, SocketName)
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

	return IsCurrentChassisSocketPresentLive(RequiredSocketNames[WheelRoleIndex])
		? FText::FromString(TEXT("현재 있음"))
		: FText::FromString(TEXT("없음 — 추가 필요"));
}

// Step 3 current Recipe가 별도 위치를 요구하는 파괴 FX Socket 상태를 표시합니다.
FText SCFVehicleBuilderTab::GetOptionalSocketGuideText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Hardpoint와 중복되지 않는 current Recipe의 non-Hardpoint Socket은 현재 계약상 Destroyed FX Socket입니다.
	const TArray<FName> OptionalSocketNames = ViewModel->GetConditionalNonHardpointSocketNames();
	if (OptionalSocketNames.IsEmpty())
	{
		return FText::FromString(TEXT("현재 차량은 별도 파괴 FX Socket을 요구하지 않습니다. 이 항목은 무시해도 됩니다."));
	}

	TArray<FString> SocketLines;
	SocketLines.Reserve(OptionalSocketNames.Num() + 1);
	for (const FName SocketName : OptionalSocketNames)
	{
		const bool bSocketFound = ViewModel->IsCurrentChassisSocketFound(SocketName);
		SocketLines.Add(FString::Printf(
			TEXT("[파괴 FX] %s | %s"),
			*SocketName.ToString(),
			bSocketFound ? TEXT("현재 있음") : TEXT("현재 없음 — 효과를 이 위치에 붙일 경우 추가 필요")));
	}

	SocketLines.Add(TEXT("이 Socket은 차량이 파괴될 때 폭발/잔해 같은 FX를 차체의 특정 위치에 붙이기 위한 것입니다. 별도 위치가 필요하지 않다면 만들지 않아도 됩니다."));
	return FText::FromString(FString::Join(SocketLines, LINE_TERMINATOR));
}

// Step 3에서 복사할 optional Socket 이름이 하나 이상 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanCopyOptionalSocketNames() const
{
	return ViewModel.IsValid() && !ViewModel->GetConditionalNonHardpointSocketNames().IsEmpty();
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

	// USER가 확인할 fixed-shift Transmission diagnostic 요약입니다.
	FString TransmissionDiagnosticText = FString::Printf(
		TEXT("Transmission ProposalHash: %s\n"),
		Preview.TransmissionProposalHash.IsEmpty() ? TEXT("<Legacy/미검토>") : *Preview.TransmissionProposalHash);
	for (const FCFBuilderTransmissionGearDiagnostic& GearDiagnostic : Preview.TransmissionDiagnostic.Gears)
	{
		// WSA-owned current wheel radius로 계산한 이론 shift-speed 표현입니다.
		const FString ShiftSpeedText = GearDiagnostic.bShiftSpeedAvailable
			? (FMath::IsNearlyEqual(GearDiagnostic.ShiftSpeedMinKmh, GearDiagnostic.ShiftSpeedMaxKmh, 0.01f)
				? FString::Printf(TEXT("%.1f km/h"), GearDiagnostic.ShiftSpeedMinKmh)
				: FString::Printf(TEXT("%.1f~%.1f km/h"), GearDiagnostic.ShiftSpeedMinKmh, GearDiagnostic.ShiftSpeedMaxKmh))
			: TEXT("Unavailable");
		// 다음 기어가 존재할 때만 계산하는 post-shift RPM 표현입니다.
		const FString PostShiftText = GearDiagnostic.bPostShiftAvailable
			? FString::Printf(TEXT("%.0f RPM / Retention %.3f / Down margin %.0f"), GearDiagnostic.PostShiftRPM, GearDiagnostic.RpmRetention, GearDiagnostic.DownshiftMarginRPM)
			: TEXT("N/A");
		TransmissionDiagnosticText += FString::Printf(
			TEXT("%d단 | Ratio %.4f | Overall %.4f | Shift@UpRPM %s | 변속 후 %s\n"),
			GearDiagnostic.GearNumber,
			GearDiagnostic.GearRatio,
			GearDiagnostic.OverallRatio,
			*ShiftSpeedText,
			*PostShiftText);
	}
	for (const FString& TransmissionWarning : Preview.TransmissionDiagnostic.Warnings)
	{
		TransmissionDiagnosticText += FString::Printf(TEXT("Warning: %s\n"), *TransmissionWarning);
	}
	TransmissionDiagnosticText += TEXT("※ 예상 차속은 runtime 변속 조건이 아니라 고정 ChangeUpRPM 조합의 무슬립 sanity diagnostic입니다.\n");

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
			"Transmission diagnostic:\n%s\n"
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
		*Preview.ProspectiveResolvedDefinitionHash,
		*TransmissionDiagnosticText));

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
			"PASS는 이 Target DefinitionHash에 persistent binding됩니다. Benchmark RunId는 진단용이며 같은 DefinitionHash에서 새 benchmark를 실행해도 PASS는 유지됩니다.\n"
			"Builder는 자동 저장하지 않으므로 재기동 후에도 유지하려면 승인 뒤 Recipe Asset을 직접 저장해야 합니다.\n\n"
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
		TEXT("USER Driving PASS를 exact Target DefinitionHash에 persistent Recipe receipt로 기록했습니다. Current RunId=%s는 진단용입니다. 자동 저장하지 않았으므로 재기동 후 유지하려면 Recipe Asset을 직접 저장하세요."),
		*ViewModel->GetDrivingBenchmarkResult().RunId));
	return FReply::Handled();
}

// Vehicle ID 입력 변경을 BuilderVM canonical transient state에 반영하고 valid할 때 default package/object identity를 갱신합니다.
void SCFVehicleBuilderTab::HandleVehicleIdTextChanged(const FText& NewText)
{
	if (!ViewModel.IsValid())
	{
		ClearCreationIdentityFields();
		VehicleIdValidationText = LOCTEXT("VehicleIdMissingVM", "Builder ViewModel이 없어 Vehicle ID를 검증할 수 없습니다.");
		return;
	}

	// USER가 입력한 exact Vehicle ID 문자열입니다. Invalid 문자를 임의 sanitize하지 않습니다.
	const FString VehicleId = NewText.ToString();
	// Vehicle ID validation diagnostic입니다.
	FString Error;
	if (!ViewModel->SetVehicleCreationId(VehicleId, Error))
	{
		ClearCreationIdentityFields();
		VehicleIdValidationText = FText::FromString(Error);
		return;
	}

	ApplyDefaultCreationIdentityFromVehicleId();
}

// Current Vehicle ID로 deterministic default Definition/Recipe package/object 네 값을 Advanced 입력란에 채웁니다.
void SCFVehicleBuilderTab::ApplyDefaultCreationIdentityFromVehicleId()
{
	if (!ViewModel.IsValid()
		|| !NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return;
	}

	// Default VehicleData package identity입니다.
	FString DefinitionPackageName;
	// Default VehicleData object name입니다.
	FString DefinitionAssetName;
	// Default Recipe package identity입니다.
	FString RecipePackageName;
	// Default Recipe object name입니다.
	FString RecipeAssetName;
	// Default identity build diagnostic입니다.
	FString Error;
	if (!ViewModel->BuildDefaultVehicleRecordIdentity(
		ViewModel->GetVehicleCreationId(),
		DefinitionPackageName,
		DefinitionAssetName,
		RecipePackageName,
		RecipeAssetName,
		Error))
	{
		ClearCreationIdentityFields();
		VehicleIdValidationText = FText::FromString(Error);
		return;
	}

	NewDefinitionPackageTextBox->SetText(FText::FromString(DefinitionPackageName));
	NewDefinitionNameTextBox->SetText(FText::FromString(DefinitionAssetName));
	NewRecipePackageTextBox->SetText(FText::FromString(RecipePackageName));
	NewRecipeNameTextBox->SetText(FText::FromString(RecipeAssetName));
	VehicleIdValidationText = FText::FromString(FString::Printf(
		TEXT("기본 경로 제안: %s / %s. 충돌과 최종 경로 유효성은 생성 검토 Preview가 다시 확인합니다."),
		*DefinitionPackageName,
		*RecipePackageName));
}

// Advanced Definition/Recipe package/object 입력 네 값을 모두 비웁니다.
void SCFVehicleBuilderTab::ClearCreationIdentityFields()
{
	if (NewDefinitionPackageTextBox.IsValid())
	{
		NewDefinitionPackageTextBox->SetText(FText::GetEmpty());
	}
	if (NewDefinitionNameTextBox.IsValid())
	{
		NewDefinitionNameTextBox->SetText(FText::GetEmpty());
	}
	if (NewRecipePackageTextBox.IsValid())
	{
		NewRecipePackageTextBox->SetText(FText::GetEmpty());
	}
	if (NewRecipeNameTextBox.IsValid())
	{
		NewRecipeNameTextBox->SetText(FText::GetEmpty());
	}
}

// Current Vehicle ID validation 안내를 표시합니다.
FText SCFVehicleBuilderTab::GetVehicleIdValidationText() const
{
	return VehicleIdValidationText;
}

// Explicit New Vehicle과 Mesh Candidate Quick Start를 구분하는 생성 검토 버튼 문구를 반환합니다.
FText SCFVehicleBuilderTab::GetCreateVehicleButtonText() const
{
	return ViewModel.IsValid() && !ViewModel->IsNewVehicleEntryActive() && ViewModel->IsMeshOnlyCandidate()
		? LOCTEXT("CreateFromMeshCandidate", "이 Mesh로 새 차량 만들기 — 검토")
		: LOCTEXT("CreateNewVehicleReview", "생성 내용 검토");
}

// Explicit New Vehicle은 빈 Vehicle ID로 초기화하고 Mesh Candidate는 Mesh stem 기반 Vehicle ID 제안값을 채웁니다.
void SCFVehicleBuilderTab::SyncCreationFieldsFromSelection()
{
	if (!NewVehicleIdTextBox.IsValid())
	{
		return;
	}

	if (!ViewModel.IsValid())
	{
		NewVehicleIdTextBox->SetText(FText::GetEmpty());
		ClearCreationIdentityFields();
		return;
	}

	if (ViewModel->IsNewVehicleEntryActive())
	{
		NewVehicleIdTextBox->SetText(FText::GetEmpty());
		ClearCreationIdentityFields();
		VehicleIdValidationText = LOCTEXT("VehicleIdEmptyGuide", "Vehicle ID를 입력하세요. 영문자, 숫자, _만 사용할 수 있습니다.");
		return;
	}

	if (!ViewModel->IsMeshOnlyCandidate())
	{
		NewVehicleIdTextBox->SetText(FText::GetEmpty());
		ClearCreationIdentityFields();
		VehicleIdValidationText = FText::GetEmpty();
		return;
	}

	// Mesh 이름에서 semantic inference 없이 Vehicle ID 제안용 파일명 stem만 가져옵니다.
	FString CandidateVehicleId = ViewModel->GetSelectedEntry().ChassisMeshPath.GetAssetName();
	CandidateVehicleId.RemoveFromStart(TEXT("SM_"));
	if (CandidateVehicleId.IsEmpty())
	{
		CandidateVehicleId = TEXT("Vehicle");
	}

	NewVehicleIdTextBox->SetText(FText::FromString(CandidateVehicleId));
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

// Step 1의 Explicit New Vehicle 또는 Mesh-only Quick Start 생성 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetMeshCreationVisibility() const
{
	return ViewModel.IsValid()
		&& (ViewModel->IsNewVehicleEntryActive() || ViewModel->IsMeshOnlyCandidate())
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::IdentityReference
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Explicit New Vehicle 전용 Blank/optional Chassis picker 영역 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetNewVehicleCreationOptionsVisibility() const
{
	return ViewModel.IsValid() && ViewModel->IsNewVehicleEntryActive()
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Explicit New Vehicle의 current optional Chassis StaticMesh object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetNewVehicleChassisMeshPath() const
{
	return ViewModel.IsValid() && ViewModel->GetNewVehicleChassisMeshPath().IsValid()
		? ViewModel->GetNewVehicleChassisMeshPath().ToString()
		: FString();
}

// Step 1 managed Recipe의 Reference/Companion UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetReferenceFlowVisibility() const
{
	return ViewModel.IsValid()
		&& !ViewModel->IsNewVehicleEntryActive()
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

// Step 6 Hardpoint별 Standard MountType transient draft를 변경합니다. Recipe mutation은 하지 않습니다.
FReply SCFVehicleBuilderTab::HandleSelectMountDraftType(
	const FName LocationSlotId,
	const ECFVehicleMountType MountType)
{
	if (!LocationSlotId.IsNone())
	{
		PendingMountTypes.Add(LocationSlotId, MountType);
		RefreshMountPlanningPresentation();
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("%s MountType draft를 변경했습니다. 아직 Recipe에는 반영하지 않았습니다."),
			*LocationSlotId.ToString()));
	}
	return FReply::Handled();
}

// Step 6 Hardpoint별 Standard SizeLimit transient draft를 변경합니다. Recipe mutation은 하지 않습니다.
FReply SCFVehicleBuilderTab::HandleSelectMountDraftSize(
	const FName LocationSlotId,
	const ECFVehicleWeaponSize SizeLimit)
{
	if (!LocationSlotId.IsNone())
	{
		PendingMountSizes.Add(LocationSlotId, SizeLimit);
		RefreshMountPlanningPresentation();
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("%s SizeLimit draft를 변경했습니다. 아직 Recipe에는 반영하지 않았습니다."),
			*LocationSlotId.ToString()));
	}
	return FReply::Handled();
}

// Step 6 Hardpoint별 optional EquipmentPreset pending picker를 변경합니다. Recipe mutation은 하지 않습니다.
void SCFVehicleBuilderTab::HandleMountPresetChanged(
	const FAssetData& AssetData,
	const FName LocationSlotId)
{
	if (LocationSlotId.IsNone())
	{
		return;
	}

	PendingMountPresetPaths.Add(
		LocationSlotId,
		AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath());
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("%s EquipmentPreset draft를 변경했습니다. 아직 Recipe에는 반영하지 않았습니다."),
		*LocationSlotId.ToString()));
}

// Step 6 Hardpoint 하나의 complete transient draft를 Standard 1:1 typed Recipe write로 반영합니다.
FReply SCFVehicleBuilderTab::HandleCommitStandardMount(const FName LocationSlotId)
{
	if (!ViewModel.IsValid() || LocationSlotId.IsNone())
	{
		return FReply::Handled();
	}

	const ECFVehicleMountType MountType = PendingMountTypes.FindRef(LocationSlotId);
	const ECFVehicleWeaponSize SizeLimit = PendingMountSizes.FindRef(LocationSlotId);
	const FSoftObjectPath PresetPath = PendingMountPresetPaths.FindRef(LocationSlotId);

	FCFMountIntent CommittedIntent;
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->CommitStandardMountIntent(
		LocationSlotId,
		MountType,
		SizeLimit,
		PresetPath,
		CommittedIntent,
		Result,
		Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Mount 반영 실패: %s"), *Error));
		RefreshMountPlanningPresentation();
		return FReply::Handled();
	}

	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("장착 규칙을 Recipe에 반영했습니다: %s → %s. Target VehicleData Apply/자동 Save는 수행하지 않았습니다."),
		*CommittedIntent.LocationSlotRef.ToString(),
		*CommittedIntent.MountProfileId.ToString()));
	return FReply::Handled();
}

// Step 6 exact Recipe MountProfileId 하나를 typed remove lane으로 제거합니다.
FReply SCFVehicleBuilderTab::HandleRemoveMount(const FName MountProfileId)
{
	if (!ViewModel.IsValid() || MountProfileId.IsNone())
	{
		return FReply::Handled();
	}

	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->RemoveMountIntent(MountProfileId, Result, Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("Mount 제거 실패: %s"), *Error));
		RefreshMountPlanningPresentation();
		return FReply::Handled();
	}

	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("%s Recipe Mount를 제거했습니다. Hardpoint/StaticMesh/Target VehicleData는 변경하지 않았고 자동 Save도 하지 않았습니다."),
		*MountProfileId.ToString()));
	return FReply::Handled();
}

// Step 6 current Recipe/HardpointPlanMode에서 Standard Mount planning subtree를 생성합니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildMountPlanningPanel()
{
	TSharedRef<SVerticalBox> Panel = SNew(SVerticalBox);
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("MountPlanNoRecipe", "Mount 규칙을 표시할 managed Recipe가 없습니다."))
		];
		return Panel;
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const ECFBuilderHardpointPlanMode PlanMode = ViewModel->GetHardpointPlanMode();

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
	[
		SNew(STextBlock)
			.Text(LOCTEXT("StandardMountHeader", "Standard 1:1 Mount 규칙"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
	];

	if (PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible)
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(
					TEXT("기존 호환 모드입니다. Recipe Mount %d개 / Target 기존 구조를 custom·multi-Mount 의미 그대로 보존하며 Standard 1:1 편집을 강제하지 않습니다."),
					Recipe->MountIntents.Num())))
				.AutoWrapText(true)
		];
		return Panel;
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::Unspecified)
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("MountPlanUnspecified", "Step 3 Hardpoint Plan이 아직 미결정입니다. 먼저 '장착점 없음' 또는 '장착 위치 사용'을 명시적으로 선택하세요."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::NoHardpoints)
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(Recipe->HardpointIntents.IsEmpty() && Recipe->MountIntents.IsEmpty()
					? LOCTEXT("MountPlanIntentionalZero", "장착점 없음이 명시되어 있어 Mount 0개가 정상 완료 상태입니다.")
					: LOCTEXT("MountPlanNoHardpointConflict", "NoHardpoints인데 Hardpoint/Mount semantic row가 남아 있습니다. Mount → Hardpoint 순서로 제거해야 합니다."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	if (Recipe->HardpointIntents.IsEmpty())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("MountPlanNoHardpointsYet", "UseHardpoints를 선택했지만 Hardpoint가 아직 없습니다. Step 3에서 장착 위치를 하나 이상 추가하세요."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
			.Text(LOCTEXT("MountPlanDraftBoundary", "아래 Type/Size/Preset 선택은 transient draft입니다. '장착 규칙 반영'을 눌러야 Recipe가 바뀝니다. 새 MountProfileId는 Mount_<LocationSlotId>이며 기존 1:1 Mount는 현재 ID를 유지합니다."))
			.AutoWrapText(true)
	];

	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		const FName LocationSlotId = HardpointIntent.LocationSlotId;
		if (LocationSlotId.IsNone())
		{
			continue;
		}

		TArray<const FCFMountIntent*> MountsForHardpoint;
		for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
		{
			if (MountIntent.LocationSlotRef == LocationSlotId)
			{
				MountsForHardpoint.Add(&MountIntent);
			}
		}

		const FCFMountIntent* ExistingMount = MountsForHardpoint.Num() == 1 ? MountsForHardpoint[0] : nullptr;
		const FName DisplayMountProfileId = ExistingMount
			? ExistingMount->MountProfileId
			: FName(*FString::Printf(TEXT("Mount_%s"), *LocationSlotId.ToString()));
		const ECFVehicleMountType DraftMountType = PendingMountTypes.FindRef(LocationSlotId);
		const ECFVehicleWeaponSize DraftSizeLimit = PendingMountSizes.FindRef(LocationSlotId);
		const bool bDraftRuleComplete = DraftMountType != ECFVehicleMountType::None
			&& (DraftMountType == ECFVehicleMountType::Utility || DraftSizeLimit != ECFVehicleWeaponSize::None);
		const bool bStandardStructureEditable = MountsForHardpoint.Num() <= 1;

		const auto BuildTypeButton = [this, LocationSlotId, DraftMountType](const ECFVehicleMountType Type, const TCHAR* Label)
		{
			const FString ButtonLabel = DraftMountType == Type
				? FString::Printf(TEXT("[%s]"), Label)
				: FString(Label);
			return SNew(SButton)
				.Text(FText::FromString(ButtonLabel))
				.OnClicked(this, &SCFVehicleBuilderTab::HandleSelectMountDraftType, LocationSlotId, Type);
		};
		const auto BuildSizeButton = [this, LocationSlotId, DraftSizeLimit](const ECFVehicleWeaponSize Size, const TCHAR* Label)
		{
			const FString ButtonLabel = DraftSizeLimit == Size
				? FString::Printf(TEXT("[%s]"), Label)
				: FString(Label);
			return SNew(SButton)
				.Text(FText::FromString(ButtonLabel))
				.OnClicked(this, &SCFVehicleBuilderTab::HandleSelectMountDraftSize, LocationSlotId, Size);
		};

		Panel->AddSlot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(
							TEXT("%s → %s | 현재 연결 Mount %d개"),
							*LocationSlotId.ToString(),
							*DisplayMountProfileId.ToString(),
							MountsForHardpoint.Num())))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock)
						.Text(MountsForHardpoint.Num() > 1
							? LOCTEXT("MountMultiConflict", "이 Hardpoint는 multi-Mount 구조입니다. Standard 편집이 자동으로 축소하지 않습니다. Advanced에서 먼저 구조를 정리하세요.")
							: LOCTEXT("MountTypeDraftLabel", "MountType draft"))
						.AutoWrapText(true)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Fixed, TEXT("Fixed"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Gimbal, TEXT("Gimbal"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Turret, TEXT("Turret"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Launcher, TEXT("Launcher"))]
					+ SHorizontalBox::Slot().AutoWidth()[BuildTypeButton(ECFVehicleMountType::Utility, TEXT("Utility"))]
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("MountSizeDraftLabel", "SizeLimit draft — Fixed/Gimbal/Turret/Launcher는 Small 이상 필수, Utility는 None 허용"))
						.AutoWrapText(true)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildSizeButton(ECFVehicleWeaponSize::None, TEXT("None"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildSizeButton(ECFVehicleWeaponSize::Small, TEXT("Small"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildSizeButton(ECFVehicleWeaponSize::Medium, TEXT("Medium"))]
					+ SHorizontalBox::Slot().AutoWidth()[BuildSizeButton(ECFVehicleWeaponSize::Large, TEXT("Large"))]
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(145.0f)
						[
							SNew(STextBlock).Text(LOCTEXT("MountPresetLabel", "기본 EquipmentPreset"))
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFEquipmentPresetData::StaticClass())
							.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingMountPresetPath, LocationSlotId)
							.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleMountPresetChanged, LocationSlotId)
							.AllowClear(true)
					]
				]

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						SNew(SButton)
							.Text(ExistingMount ? LOCTEXT("UpdateStandardMount", "장착 규칙 반영") : LOCTEXT("CreateStandardMount", "장착 규칙 생성"))
							.ToolTipText(LOCTEXT("CommitStandardMountTooltip", "complete transient draft를 검증한 뒤 Recipe MountIntent만 typed write합니다. 새 ID 충돌은 자동 suffix하지 않고 실패합니다."))
							.IsEnabled(bStandardStructureEditable && bDraftRuleComplete)
							.OnClicked(this, &SCFVehicleBuilderTab::HandleCommitStandardMount, LocationSlotId)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
							.Text(LOCTEXT("RemoveStandardMount", "Mount 삭제"))
							.ToolTipText(LOCTEXT("RemoveStandardMountTooltip", "exact Recipe MountProfileId 하나만 제거합니다. Hardpoint/StaticMesh/Target VehicleData는 건드리지 않습니다."))
							.IsEnabled(ExistingMount && !ExistingMount->MountProfileId.IsNone())
							.OnClicked(this, &SCFVehicleBuilderTab::HandleRemoveMount, ExistingMount ? ExistingMount->MountProfileId : NAME_None)
					]
				]
			]
		];
	}

	return Panel;
}

// Step 6 persistent Recipe의 existing 1:1 Mount 값을 transient draft로 다시 동기화합니다.
void SCFVehicleBuilderTab::SyncMountPlanningDraftsFromRecipe()
{
	PendingMountTypes.Reset();
	PendingMountSizes.Reset();
	PendingMountPresetPaths.Reset();

	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		return;
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		if (HardpointIntent.LocationSlotId.IsNone())
		{
			continue;
		}

		TArray<const FCFMountIntent*> MountsForHardpoint;
		for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
		{
			if (MountIntent.LocationSlotRef == HardpointIntent.LocationSlotId)
			{
				MountsForHardpoint.Add(&MountIntent);
			}
		}

		if (MountsForHardpoint.Num() == 1)
		{
			const FCFMountIntent& ExistingMount = *MountsForHardpoint[0];
			PendingMountTypes.Add(HardpointIntent.LocationSlotId, ExistingMount.MountType);
			PendingMountSizes.Add(HardpointIntent.LocationSlotId, ExistingMount.SizeLimit);
			PendingMountPresetPaths.Add(HardpointIntent.LocationSlotId, ExistingMount.DefaultEquipmentPresetData.ToSoftObjectPath());
		}
		else
		{
			PendingMountTypes.Add(HardpointIntent.LocationSlotId, ECFVehicleMountType::None);
			PendingMountSizes.Add(HardpointIntent.LocationSlotId, ECFVehicleWeaponSize::None);
			PendingMountPresetPaths.Add(HardpointIntent.LocationSlotId, FSoftObjectPath());
		}
	}
}

// Step 6 transient/persistent 변경 뒤 Mount planning subtree를 current truth로 교체합니다.
void SCFVehicleBuilderTab::RefreshMountPlanningPresentation()
{
	if (MountPlanningHost.IsValid())
	{
		MountPlanningHost->SetContent(BuildMountPlanningPanel());
	}
}

// Step 6 Hardpoint별 pending EquipmentPreset object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetPendingMountPresetPath(const FName LocationSlotId) const
{
	const FSoftObjectPath* PresetPath = PendingMountPresetPaths.Find(LocationSlotId);
	return PresetPath && PresetPath->IsValid() ? PresetPath->ToString() : FString();
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
	if (ViewModel.IsValid() && ViewModel->IsNewVehicleEntryActive())
	{
		return LOCTEXT("NewVehicleSelection", "대상: 새 차량 만들기 (신규 제작 시작)");
	}

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

	return ViewModel->CanAdvanceFromCurrentStep();
}

#undef LOCTEXT_NAMESPACE
