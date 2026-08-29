// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleP11UI.cpp
// Version: v1.7.0
// Date: 2026-08-21
// Description: DAUTH-P0-11~12 Frozen 24.91~24.94 normal Workspace Slate UI입니다.
// Scope: 5-domain Profile binding/open, Shared Profile B2 nested editor/impact navigation, External Drift 3-way recovery, Mesh-only Candidate two-record creation을 제공합니다.
// Changelog:
// - v1.7.0: P0-12 UA-06 USER UX remediation으로 Shared Profile 숫자 편집을 raw ColumnId 직접입력에서 Registry 기반 검색/선택 + 표시명/현재값/단위 UI로 전환하고 review에도 current/new 값을 표시.
// - v1.6.0: P0-12 UA-07 baseline-safe Profile 검증 복구를 위해 reviewed Profile 연결 해제 UX와 Bind/Unbind 실패 feedback을 추가.
// - v1.5.0: P0-12 UA-04 USER 피드백에 따라 사용자-facing Authoring/VehicleData/3-way 용어를 제작 기준값/차량 데이터/상세 비교로 현지화.
// - v1.4.0: P0-12 UA-04 edge audit에서 incomplete 3-way review action 차단, mixed AdvancedOverride 불가 묶음 안내, recovery 실패 modal, 기본 승인창 기술 hash 비노출을 추가.
// - v1.3.0: P0-12 UA-04 USER feedback에 따라 External Drift를 목적 중심 macro-flow로 단순화하고 Keep Authoring primary + same-pane final Apply + secondary/detail 영역을 추가.
// - v1.2.0: P0-12 UA-03 USER feedback에 따라 current Profile binding, existing 후보 선택, explicit Recipe-only bind와 Open Profile UX를 추가.
// - v1.1.0: P0-12 UA-01 사용자 피드백에 따라 Shared Profile, affected Vehicle, Drift 3-way, Mesh Create 문구를 한국어 우선으로 정리.
// - v1.0.0: Frozen UX completeness technical route 최초 구현.
// Migration:
// - v1.7.0부터 사람은 stable technical ColumnId를 직접 입력하지 않고 선택한 Profile Domain의 Registry 항목을 검색/선택합니다. 내부 B2 request의 ColumnId authority와 Registry/Apply 계약은 그대로 유지합니다.
// - v1.5.0은 사용자-facing 문구만 현지화하며 ECFVehicleDriftDecision, facade/ViewModel, Recipe/Profile/VehicleData 내부 계약은 변경하지 않습니다.
// - 시각/사용성 최종 판정은 P0-12에 남기며 이 파일은 Common Authoring facade/ViewModel 도달성만 제공합니다.
// - raw UProperty SetField, direct existing Target write, Auto Apply/Save/Retry를 제공하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringTab.h"

#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFBatchColumnRegistry.h"
#include "DataAuthoring/CFVehicleUXTypes.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

namespace CFVehicleP11UIPrivate
{
	// Profile Domain enum을 사람이 읽을 수 있는 display label로 변환합니다.
		FString ProfileDomainText(const ECFVehicleProfileDomain ProfileDomain)
	{
		switch (ProfileDomain)
		{
		case ECFVehicleProfileDomain::VehicleBase: return TEXT("차량 기본");
		case ECFVehicleProfileDomain::Drivetrain: return TEXT("구동계");
		case ECFVehicleProfileDomain::Handling: return TEXT("핸들링");
		case ECFVehicleProfileDomain::Performance: return TEXT("성능");
		case ECFVehicleProfileDomain::DriveState: return TEXT("주행 상태");
		default: return TEXT("알 수 없음");
		}
	}

	// Drift recovery decision을 review dialog label로 변환합니다.
	FString DriftDecisionText(const ECFVehicleDriftDecision Decision)
	{
		switch (Decision)
		{
				case ECFVehicleDriftDecision::KeepAuthoring:
			return TEXT("제작 기준값 유지");
		case ECFVehicleDriftDecision::PreserveRawAsLegacyPin:
			return TEXT("현재 원본값을 레거시 고정값으로 보존");
		case ECFVehicleDriftDecision::PromoteRawToAdvancedOverride:
			return TEXT("현재 원본값을 고급 덮어쓰기로 승격");
		default:
			return TEXT("알 수 없음");
		}
	}

	// Long hash를 review 화면에서 식별 가능한 prefix로 표시합니다.
	FString ShortHash(const FString& Hash)
	{
		return Hash.Len() > 14 ? Hash.Left(14) + TEXT("…") : Hash;
	}
}

// P0-11 Recipe page 안의 Shared Profile Editor / Mesh-only Create panel을 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildP11RecipePanel()
{
	// P0-11 Recipe nested workflow root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)
	[
		SNew(SSeparator)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SCFVehicleAuthoringTab::GetManagedVisibility)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("공유 프로필 편집 — 허용된 숫자 항목만 편집합니다. 원시 UObject 필드를 직접 쓰지 않습니다.")))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("차량 기본"))).OnClicked_Lambda([this]() { return HandleProfileDomainSelected(ECFVehicleProfileDomain::VehicleBase); })]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("구동계"))).OnClicked_Lambda([this]() { return HandleProfileDomainSelected(ECFVehicleProfileDomain::Drivetrain); })]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("핸들링"))).OnClicked_Lambda([this]() { return HandleProfileDomainSelected(ECFVehicleProfileDomain::Handling); })]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("성능"))).OnClicked_Lambda([this]() { return HandleProfileDomainSelected(ECFVehicleProfileDomain::Performance); })]
			+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("주행 상태"))).OnClicked_Lambda([this]() { return HandleProfileDomainSelected(ECFVehicleProfileDomain::DriveState); })]
		]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(FString::Printf(TEXT("선택한 프로필 종류: %s"), *CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain))); })
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetProfileBindingText).AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SAssignNew(ProfileChoiceComboBox, SComboBox<TSharedPtr<FCFProfileListEntry>>)
			.OptionsSource(&ProfileChoiceRows)
			.OnGenerateWidget(this, &SCFVehicleAuthoringTab::GenerateProfileChoiceWidget)
			.OnSelectionChanged(this, &SCFVehicleAuthoringTab::HandleProfileChoiceChanged)
			[
				SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetSelectedProfileChoiceText)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("선택 프로필 연결 검토…")))
				.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasRecipe() && SelectedProfileChoice.IsValid(); })
				.OnClicked(this, &SCFVehicleAuthoringTab::HandleBindSelectedProfile)
						]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("현재 프로필 연결 해제 검토…")))
				.IsEnabled(this, &SCFVehicleAuthoringTab::HasBoundSelectedProfile)
				.OnClicked(this, &SCFVehicleAuthoringTab::HandleUnbindSelectedProfile)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("현재 프로필 열기")))
				.IsEnabled(this, &SCFVehicleAuthoringTab::HasBoundSelectedProfile)
				.OnClicked(this, &SCFVehicleAuthoringTab::HandleOpenBoundProfile)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("공유 프로필 값 편집 — 현재 연결된 프로필의 허용된 숫자 항목만 변경합니다."))).AutoWrapText(true)
		]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SAssignNew(ProfileNumericFieldSearchBox, SSearchBox)
			.HintText(FText::FromString(TEXT("편집할 항목 검색 (이름 / 기술 ID)")))
			.OnTextChanged(this, &SCFVehicleAuthoringTab::HandleProfileNumericFieldSearchChanged)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SAssignNew(ProfileNumericFieldComboBox, SComboBox<TSharedPtr<FCFBatchColumnDescriptor>>)
			.OptionsSource(&ProfileNumericFieldRows)
			.OnGenerateWidget(this, &SCFVehicleAuthoringTab::GenerateProfileNumericFieldWidget)
			.OnSelectionChanged(this, &SCFVehicleAuthoringTab::HandleProfileNumericFieldChanged)
			[
				SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetSelectedProfileNumericFieldText)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(this, &SCFVehicleAuthoringTab::GetSelectedProfileNumericFieldStatusText)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SAssignNew(ProfileNumericValueTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("새 숫자 값")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("공유 프로필 변경 검토…")))
			.IsEnabled_Lambda([this]() { return HasBoundSelectedProfile() && SelectedProfileNumericField.IsValid(); })
			.OnClicked(this, &SCFVehicleAuthoringTab::HandleProfileNumericEdit)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetProfileImpactText).AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(0) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[0].TargetPath, Rows[0].PendingDefinitionChangeCount) : TEXT("영향 차량 1")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(0); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(0); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(1) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[1].TargetPath, Rows[1].PendingDefinitionChangeCount) : TEXT("영향 차량 2")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(1); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(1); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(2) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[2].TargetPath, Rows[2].PendingDefinitionChangeCount) : TEXT("영향 차량 3")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(2); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(2); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(3) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[3].TargetPath, Rows[3].PendingDefinitionChangeCount) : TEXT("영향 차량 4")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(3); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(3); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(4) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[4].TargetPath, Rows[4].PendingDefinitionChangeCount) : TEXT("영향 차량 5")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(4); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(4); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(5) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[5].TargetPath, Rows[5].PendingDefinitionChangeCount) : TEXT("영향 차량 6")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(5); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(5); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(6) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[6].TargetPath, Rows[6].PendingDefinitionChangeCount) : TEXT("영향 차량 7")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(6); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(6); })]
			+ SVerticalBox::Slot().AutoHeight()[SNew(SButton).Text_Lambda([this]() { const auto& Rows = ViewModel->GetProfileImpactPreview().AffectedVehicles; return FText::FromString(Rows.IsValidIndex(7) ? FString::Printf(TEXT("영향 차량: %s (변경 %d개)"), *Rows[7].TargetPath, Rows[7].PendingDefinitionChangeCount) : TEXT("영향 차량 8")); }).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->GetProfileImpactPreview().AffectedVehicles.IsValidIndex(7); }).OnClicked_Lambda([this]() { return HandleNavigateAffectedVehicle(7); })]
		]
	];

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SCFVehicleAuthoringTab::GetMeshCandidateVisibility)
		+ SVerticalBox::Slot().AutoHeight()[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetMeshCandidateText).AutoWrapText(true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SAssignNew(NewDefinitionPackageTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("/Game/.../DA_Vehicle_Name")))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SAssignNew(NewDefinitionNameTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("DA_Vehicle_Name")))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SAssignNew(NewRecipePackageTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("/Game/.../DA_Recipe_Name")))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[SAssignNew(NewRecipeNameTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("DA_Recipe_Name")))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)[SNew(SButton).Text(FText::FromString(TEXT("메시에서 차량 만들기 검토…"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleCreateVehicleFromMesh)]
	];
	return Root;
}

// P0-11 Sync context의 External Drift 복구를 목적 중심 macro-flow로 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildP11SyncPanel()
{
	// External Drift macro-flow root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(STextBlock)
		.Text(this, &SCFVehicleAuthoringTab::GetSyncContextText)
		.AutoWrapText(true)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("제작 기준값으로 복구…")))
		.Visibility_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift() && !ViewModel->HasAcceptedDriftKeep() ? EVisibility::Visible : EVisibility::Collapsed; })
				.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift() && ViewModel->HasRecipe() && !ViewModel->GetDriftReview().Rows.IsEmpty(); })
		.OnClicked_Lambda([this]() { return HandleDriftDecision(ECFVehicleDriftDecision::KeepAuthoring); })
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("제작 기준값으로 복구 적용…")))
		.Visibility_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasAcceptedDriftKeep() ? EVisibility::Visible : EVisibility::Collapsed; })
		.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasAcceptedDriftKeep() && ViewModel->CanApply(); })
		.OnClicked(this, &SCFVehicleAuthoringTab::HandleApply)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.Visibility_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift() ? EVisibility::Visible : EVisibility::Collapsed; })
		.HeaderContent()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("상세 비교")))
		]
		.BodyContent()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text(this, &SCFVehicleAuthoringTab::GetDriftReviewText)
				.AutoWrapText(true)
			]
		]
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.Visibility_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift() && !ViewModel->HasAcceptedDriftKeep() ? EVisibility::Visible : EVisibility::Collapsed; })
		.HeaderContent()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("다른 처리 방법")))
		]
		.BodyContent()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("현재 원본값을 레거시 고정값으로 보존…")))
								.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift() && !ViewModel->GetDriftReview().Rows.IsEmpty(); })
				.OnClicked_Lambda([this]() { return HandleDriftDecision(ECFVehicleDriftDecision::PreserveRawAsLegacyPin); })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 2.0f)
			[
								SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!ViewModel.IsValid())
					{
						return FText::GetEmpty();
					}
					// 현재 3-way 묶음에 Advanced Override 불가 field가 하나라도 있는지 여부입니다.
					const bool bHasUnsupportedField = ViewModel->GetDriftReview().Rows.ContainsByPredicate([](const FCFVehicleDriftReviewRow& Row)
					{
						return !Row.bAdvancedOverrideAllowed;
					});
					return FText::FromString(bHasUnsupportedField
						? TEXT("현재 외부 변경 묶음에는 고급 덮어쓰기를 허용하지 않는 항목이 포함되어 있어 묶음 승격을 사용할 수 없습니다. '상세 비교'에서 허용 여부를 확인하세요.")
						: TEXT("고급 덮어쓰기는 현재 원본값을 제작 기준의 명시적 예외값으로 승격합니다. 이유 입력이 필수입니다."));
				})
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				SAssignNew(DriftOverrideReasonTextBox, SEditableTextBox)
				.HintText(FText::FromString(TEXT("고급 덮어쓰기 이유 — 원본값 승격 시 필수")))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("현재 원본값을 고급 덮어쓰기로 승격…")))
								.IsEnabled_Lambda([this]()
				{
					if (!ViewModel.IsValid() || !ViewModel->HasExternalDrift() || ViewModel->GetDriftReview().Rows.IsEmpty())
					{
						return false;
					}
					return !ViewModel->GetDriftReview().Rows.ContainsByPredicate([](const FCFVehicleDriftReviewRow& Row)
					{
						return !Row.bAdvancedOverrideAllowed;
					});
				})
				.OnClicked_Lambda([this]() { return HandleDriftDecision(ECFVehicleDriftDecision::PromoteRawToAdvancedOverride); })
			]
		]
	];
	return Root;
}

// Profile Domain 선택을 바꾸고 해당 Domain의 existing Profile 후보를 read-only 갱신합니다.
FReply SCFVehicleAuthoringTab::HandleProfileDomainSelected(const ECFVehicleProfileDomain ProfileDomain)
{
	SelectedProfileDomain = ProfileDomain;
	ProfileNumericFieldSearchText.Reset();
	SelectedProfileNumericField.Reset();
	SelectedProfileNumericCurrentValue.Reset();
	SelectedProfileNumericCurrentValueError.Reset();
	if (ProfileNumericFieldSearchBox.IsValid())
	{
		ProfileNumericFieldSearchBox->SetText(FText::GetEmpty());
	}
	if (ProfileNumericValueTextBox.IsValid())
	{
		ProfileNumericValueTextBox->SetText(FText::GetEmpty());
	}
	RefreshProfileChoiceRows();
	RefreshProfileNumericFieldRows();
	return FReply::Handled();
}

// Selected Domain의 facade Profile 후보 cache를 Slate Combo rows로 재구성합니다.
void SCFVehicleAuthoringTab::RefreshProfileChoiceRows()
{
	ProfileChoiceRows.Reset();
	SelectedProfileChoice.Reset();
	if (ViewModel.IsValid() && ViewModel->HasRecipe())
	{
		// Selected Domain candidate read diagnostic입니다.
		FString ProfileError;
		if (ViewModel->RefreshProfileChoices(SelectedProfileDomain, ProfileError))
		{
			for (const FCFProfileListEntry& Entry : ViewModel->GetProfileChoices())
			{
				ProfileChoiceRows.Add(MakeShared<FCFProfileListEntry>(Entry));
			}

						// Current bound identity와 exact 일치하는 후보가 있으면 표시 selection만 복원합니다.
			const FSoftObjectPath BoundPath = ViewModel->GetBoundProfilePath(SelectedProfileDomain);
			// Current bound Profile과 exact 일치하는 Combo row pointer입니다.
			const TSharedPtr<FCFProfileListEntry>* BoundChoice = ProfileChoiceRows.FindByPredicate([&BoundPath](const TSharedPtr<FCFProfileListEntry>& Entry)
			{
				return Entry.IsValid() && Entry->ProfilePath == BoundPath;
			});
			SelectedProfileChoice = BoundChoice ? *BoundChoice : nullptr;
		}
	}

	if (ProfileChoiceComboBox.IsValid())
	{
		ProfileChoiceComboBox->RefreshOptions();
		if (SelectedProfileChoice.IsValid())
		{
			ProfileChoiceComboBox->SetSelectedItem(SelectedProfileChoice);
		}
		else
		{
			ProfileChoiceComboBox->ClearSelection();
		}
	}
}

// Selected Domain의 Registry editable numeric fields를 검색 조건에 맞는 Slate rows로 재구성합니다.
void SCFVehicleAuthoringTab::RefreshProfileNumericFieldRows()
{
	// Refresh 전 사용자가 선택했던 stable technical ColumnId입니다.
	const FString PreviousColumnId = SelectedProfileNumericField.IsValid() ? SelectedProfileNumericField->ColumnId : FString();
	ProfileNumericFieldRows.Reset();
	SelectedProfileNumericField.Reset();
	SelectedProfileNumericCurrentValue.Reset();
	SelectedProfileNumericCurrentValueError.Reset();

	// Registry가 selected Profile Domain에 노출하는 exact editable numeric descriptor 목록입니다.
	TArray<FCFBatchColumnDescriptor> RegistryColumns;
	// Registry projection validation diagnostic입니다.
	TArray<FString> RegistryErrors;
	if (SelectedProfileDomain != ECFVehicleProfileDomain::None
		&& FCFBatchColumnRegistry::GetDatasetColumns(
			ECFBatchDatasetKind::ProfileNumericEdit,
			SelectedProfileDomain,
			RegistryColumns,
			RegistryErrors))
	{
		for (const FCFBatchColumnDescriptor& Descriptor : RegistryColumns)
		{
			// 사람이 검색할 표시명/기술 ID/단위를 합친 비교 문자열입니다.
			const FString SearchableText = FString::Printf(
				TEXT("%s %s %s"),
				*Descriptor.DisplayLabel.ToString(),
				*Descriptor.ColumnId,
				*Descriptor.Unit);
			if (!ProfileNumericFieldSearchText.IsEmpty()
				&& !SearchableText.Contains(ProfileNumericFieldSearchText, ESearchCase::IgnoreCase))
			{
				continue;
			}
			ProfileNumericFieldRows.Add(MakeShared<FCFBatchColumnDescriptor>(Descriptor));
		}
	}
	else if (!RegistryErrors.IsEmpty())
	{
		SelectedProfileNumericCurrentValueError = RegistryErrors[0];
	}

	if (!PreviousColumnId.IsEmpty())
	{
		// Filtered 결과 안에서 기존 selection을 exact ColumnId로 복원할 row입니다.
		const TSharedPtr<FCFBatchColumnDescriptor>* PreviousSelection = ProfileNumericFieldRows.FindByPredicate([&PreviousColumnId](const TSharedPtr<FCFBatchColumnDescriptor>& Row)
		{
			return Row.IsValid() && Row->ColumnId == PreviousColumnId;
		});
		SelectedProfileNumericField = PreviousSelection ? *PreviousSelection : nullptr;
	}

	if (ProfileNumericFieldComboBox.IsValid())
	{
		ProfileNumericFieldComboBox->RefreshOptions();
		if (SelectedProfileNumericField.IsValid())
		{
			ProfileNumericFieldComboBox->SetSelectedItem(SelectedProfileNumericField);
		}
		else
		{
			ProfileNumericFieldComboBox->ClearSelection();
		}
	}
	RefreshSelectedProfileNumericCurrentValue();
}

// Shared Profile numeric field 검색어를 갱신하고 filtered field rows를 다시 만듭니다.
void SCFVehicleAuthoringTab::HandleProfileNumericFieldSearchChanged(const FText& NewText)
{
	ProfileNumericFieldSearchText = NewText.ToString().TrimStartAndEnd();
	RefreshProfileNumericFieldRows();
}

// Shared Profile numeric field ComboBox 한 후보의 사용자 표시 widget을 생성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::GenerateProfileNumericFieldWidget(TSharedPtr<FCFBatchColumnDescriptor> Item) const
{
	// 단위가 존재할 때만 표시명 뒤에 붙일 사용자 unit label입니다.
	const FString UnitSuffix = Item.IsValid() && !Item->Unit.IsEmpty() ? FString::Printf(TEXT(" [%s]"), *Item->Unit) : FString();
	// 표시명은 primary, stable ColumnId는 secondary technical identity입니다.
	const FString Label = Item.IsValid()
		? FString::Printf(TEXT("%s%s | %s"), *Item->DisplayLabel.ToString(), *UnitSuffix, *Item->ColumnId)
		: TEXT("<잘못된 항목>");
	return SNew(STextBlock).Text(FText::FromString(Label));
}

// Shared Profile numeric field selection을 보존하고 current authored value를 read-only 갱신합니다.
void SCFVehicleAuthoringTab::HandleProfileNumericFieldChanged(TSharedPtr<FCFBatchColumnDescriptor> Item, ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;
	SelectedProfileNumericField = Item;
	if (ProfileNumericValueTextBox.IsValid())
	{
		ProfileNumericValueTextBox->SetText(FText::GetEmpty());
	}
	RefreshSelectedProfileNumericCurrentValue();
}

// Shared Profile numeric field ComboBox의 current selection summary를 반환합니다.
FText SCFVehicleAuthoringTab::GetSelectedProfileNumericFieldText() const
{
	if (SelectedProfileNumericField.IsValid())
	{
		// 단위가 존재할 때만 Combo current label에 표시합니다.
		const FString UnitSuffix = SelectedProfileNumericField->Unit.IsEmpty()
			? FString()
			: FString::Printf(TEXT(" [%s]"), *SelectedProfileNumericField->Unit);
		return FText::FromString(SelectedProfileNumericField->DisplayLabel.ToString() + UnitSuffix);
	}
	return FText::FromString(ProfileNumericFieldRows.IsEmpty() ? TEXT("일치하는 편집 항목 없음") : TEXT("편집할 항목 선택…"));
}

// Shared Profile numeric field의 표시명/current value/unit/technical ID를 반환합니다.
FText SCFVehicleAuthoringTab::GetSelectedProfileNumericFieldStatusText() const
{
	if (!HasBoundSelectedProfile())
	{
		return FText::FromString(TEXT("현재 선택한 프로필 종류에 연결된 Shared Profile이 없습니다. 프로필을 먼저 연결하세요."));
	}
	if (!SelectedProfileNumericField.IsValid())
	{
		return FText::FromString(TEXT("항목을 검색하거나 목록에서 선택하세요. 기술 ID를 직접 입력할 필요가 없습니다."));
	}
	if (!SelectedProfileNumericCurrentValueError.IsEmpty())
	{
		return FText::FromString(FString::Printf(TEXT("현재 값을 읽지 못했습니다: %s"), *SelectedProfileNumericCurrentValueError));
	}
	// 단위가 존재할 때 current value 뒤에 붙일 사용자 unit label입니다.
	const FString UnitSuffix = SelectedProfileNumericField->Unit.IsEmpty()
		? FString()
		: FString::Printf(TEXT(" %s"), *SelectedProfileNumericField->Unit);
	return FText::FromString(FString::Printf(
		TEXT("선택 항목: %s\n현재 값: %s%s\n기술 ID: %s"),
		*SelectedProfileNumericField->DisplayLabel.ToString(),
		*SelectedProfileNumericCurrentValue,
		*UnitSuffix,
		*SelectedProfileNumericField->ColumnId));
}

// Selected Shared Profile numeric field의 current authored value cache를 read-only 갱신합니다.
void SCFVehicleAuthoringTab::RefreshSelectedProfileNumericCurrentValue()
{
	SelectedProfileNumericCurrentValue.Reset();
	SelectedProfileNumericCurrentValueError.Reset();
	if (!ViewModel.IsValid() || !SelectedProfileNumericField.IsValid() || !HasBoundSelectedProfile())
	{
		return;
	}
	ViewModel->ReadBoundProfileNumericValue(
		SelectedProfileDomain,
		SelectedProfileNumericField->ColumnId,
		SelectedProfileNumericCurrentValue,
		SelectedProfileNumericCurrentValueError);
}

// Shared Profile ComboBox 한 후보의 표시 widget을 생성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::GenerateProfileChoiceWidget(TSharedPtr<FCFProfileListEntry> Item) const
{
	// Combo row에서 DisplayName과 exact asset identity를 함께 보여줄 label입니다.
	const FString Label = Item.IsValid()
		? FString::Printf(TEXT("%s | %s"), Item->DisplayName.IsEmpty() ? *Item->ProfilePath.GetAssetName() : *Item->DisplayName.ToString(), *Item->ProfilePath.ToString())
		: TEXT("<잘못된 프로필>");
	return SNew(STextBlock).Text(FText::FromString(Label));
}

// Shared Profile ComboBox selection을 current explicit binding 후보로 보존합니다.
void SCFVehicleAuthoringTab::HandleProfileChoiceChanged(TSharedPtr<FCFProfileListEntry> Item, ESelectInfo::Type SelectInfo)
{
	(void)SelectInfo;
	SelectedProfileChoice = Item;
}

// Shared Profile ComboBox의 current selection summary를 반환합니다.
FText SCFVehicleAuthoringTab::GetSelectedProfileChoiceText() const
{
	if (SelectedProfileChoice.IsValid())
	{
		return FText::FromString(SelectedProfileChoice->DisplayName.IsEmpty()
			? SelectedProfileChoice->ProfilePath.GetAssetName()
			: SelectedProfileChoice->DisplayName.ToString());
	}
	return FText::FromString(ProfileChoiceRows.IsEmpty() ? TEXT("사용 가능한 프로필 없음") : TEXT("연결할 프로필 선택…"));
}

// Current Recipe의 selected Domain Profile binding과 후보 상태를 표시합니다.
FText SCFVehicleAuthoringTab::GetProfileBindingText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("관리 중인 Recipe를 선택하면 5개 Profile binding을 설정할 수 있습니다."));
	}
		// Selected Domain의 current persistent Recipe binding identity입니다.
	const FSoftObjectPath BoundPath = ViewModel->GetBoundProfilePath(SelectedProfileDomain);
	return FText::FromString(FString::Printf(
		TEXT("현재 연결: %s\n사용 가능한 %s 프로필: %d\n프로필 연결은 레시피만 변경하며 차량 데이터와 공유 프로필 값은 직접 변경하지 않습니다."),
		BoundPath.IsValid() ? *BoundPath.ToString() : TEXT("없음"),
		*CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain),
		ProfileChoiceRows.Num()));
}

// Selected Domain에 current bound Profile이 존재하는지 반환합니다.
bool SCFVehicleAuthoringTab::HasBoundSelectedProfile() const
{
	return ViewModel.IsValid() && ViewModel->HasRecipe() && ViewModel->GetBoundProfilePath(SelectedProfileDomain).IsValid();
}

// User-selected existing Profile을 reviewed Recipe-only BindVehicleProfile transaction으로 연결합니다.
FReply SCFVehicleAuthoringTab::HandleBindSelectedProfile()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !SelectedProfileChoice.IsValid())
	{
		return FReply::Handled();
	}

		// Recipe-only Profile binding을 사용자가 명시적으로 확인할 review dialog입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("프로필 연결 검토\n\n프로필 종류: %s\n연결할 프로필: %s\n변경 대상: 현재 레시피의 프로필 연결\n차량 데이터 직접 변경: 없음\n공유 프로필 값 변경: 없음\n자동 저장: 안 함\n\n이 프로필을 현재 차량 레시피에 연결하시겠습니까?"),
		*CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain),
		*SelectedProfileChoice->ProfilePath.ToString()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

		// Existing BindVehicleProfile R1 semantic transaction 결과입니다.
	FCFAuthoringOpResult BindResult;
	if (!ViewModel->CommitProfileBinding(SelectedProfileDomain, SelectedProfileChoice->ProfilePath, BindResult))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(BindResult.Message.IsEmpty() ? TEXT("프로필 연결에 실패했습니다.") : BindResult.Message));
		return FReply::Handled();
	}
	RefreshProfileChoiceRows();
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Current selected Domain의 bound Profile을 reviewed Recipe-only UnbindVehicleProfile transaction으로 해제합니다.
FReply SCFVehicleAuthoringTab::HandleUnbindSelectedProfile()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FReply::Handled();
	}

	// Review 전에 고정할 current selected Domain binding identity입니다.
	const FSoftObjectPath BoundProfilePath = ViewModel->GetBoundProfilePath(SelectedProfileDomain);
	if (!BoundProfilePath.IsValid())
	{
		return FReply::Handled();
	}

	// Recipe-only Profile unbind의 삭제/Target/Save 경계를 사용자에게 명시하는 review dialog입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("프로필 연결 해제 검토\n\n프로필 종류: %s\n현재 연결: %s\n변경 대상: 현재 레시피의 프로필 연결\n차량 데이터 직접 변경: 없음\n공유 프로필 값 변경: 없음\n프로필 자산 삭제: 없음\n자동 저장: 안 함\n\n이 프로필 연결을 현재 차량 레시피에서 해제하시겠습니까?"),
		*CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain),
		*BoundProfilePath.ToString()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	// Explicit UnbindVehicleProfile R1 Recipe-only semantic transaction 결과입니다.
	FCFAuthoringOpResult UnbindResult;
	if (!ViewModel->CommitProfileUnbinding(SelectedProfileDomain, UnbindResult))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(UnbindResult.Message.IsEmpty() ? TEXT("프로필 연결 해제에 실패했습니다.") : UnbindResult.Message));
		return FReply::Handled();
	}
	RefreshProfileChoiceRows();
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Current Recipe에 bound된 selected Domain Shared Profile을 Unreal 표준 Asset Editor로 엽니다.
FReply SCFVehicleAuthoringTab::HandleOpenBoundProfile()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}
	// Profile navigation failure diagnostic입니다.
	FString OpenError;
	if (!ViewModel->OpenBoundProfile(SelectedProfileDomain, OpenError) && !OpenError.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(OpenError));
	}
	return FReply::Handled();
}

// Selected Shared Profile Domain의 allowlisted numeric edit를 B2 preview/review/commit합니다.
FReply SCFVehicleAuthoringTab::HandleProfileNumericEdit()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !SelectedProfileNumericField.IsValid() || !ProfileNumericValueTextBox.IsValid())
	{
		return FReply::Handled();
	}
	RefreshSelectedProfileNumericCurrentValue();
	if (!SelectedProfileNumericCurrentValueError.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(SelectedProfileNumericCurrentValueError));
		return FReply::Handled();
	}
	// User가 목록에서 선택한 Registry authority stable editable ColumnId입니다.
	const FString ColumnId = SelectedProfileNumericField->ColumnId;
	// User-entered canonical numeric 새 값입니다.
	const FString NumericValue = ProfileNumericValueTextBox->GetText().ToString().TrimStartAndEnd();
	// 단위가 존재할 때 review value 뒤에 붙일 사용자 unit label입니다.
	const FString UnitSuffix = SelectedProfileNumericField->Unit.IsEmpty()
		? FString()
		: FString::Printf(TEXT(" %s"), *SelectedProfileNumericField->Unit);
	// Existing B2 mutation0 preview/impact입니다.
	FCFProfileNumericEditPreview Preview;
	if (!ViewModel->PrepareProfileNumericEdit(SelectedProfileDomain, ColumnId, NumericValue, Preview))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ViewModel->GetLastMessage()));
		return FReply::Handled();
	}
	// Explicit Shared Profile B2 review dialog입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("공유 프로필 변경 검토\n\n프로필 종류: %s\n항목: %s\n현재 값: %s%s\n새 값: %s%s\n기술 ID: %s\n영향받는 차량: %d\n예상 차량 데이터 변경: %d\n차량 데이터 자동 적용: 안 함\n자동 저장: 안 함\n\n공유 프로필 값을 변경하시겠습니까?"),
		*CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain),
		*SelectedProfileNumericField->DisplayLabel.ToString(),
		*SelectedProfileNumericCurrentValue,
		*UnitSuffix,
		*NumericValue,
		*UnitSuffix,
		*ColumnId,
		Preview.AffectedVehicleCount,
		Preview.PendingDefinitionChangeCount));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Existing B2 terminal result입니다.
		FCFProfileNumericEditResult Result;
		if (!ViewModel->ExecutePreparedProfileEdit(Result))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result.Operation.Message));
			return FReply::Handled();
		}
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Profile impact row의 affected Vehicle로 Browser selection을 이동합니다.
FReply SCFVehicleAuthoringTab::HandleNavigateAffectedVehicle(const int32 ImpactIndex)
{
	if (ViewModel.IsValid())
	{
		// Affected Vehicle navigation diagnostic입니다.
		FString Error;
		ViewModel->NavigateToAffectedVehicle(ImpactIndex, Error);
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Current External Drift 3-way rows를 fresh review합니다.
FReply SCFVehicleAuthoringTab::HandleRefreshDriftReview()
{
	if (ViewModel.IsValid())
	{
		// Read-only 3-way review diagnostic입니다.
		FString Error;
		ViewModel->RefreshDriftReview(Error);
	}
	return FReply::Handled();
}

// Current External Drift 전체 group에 explicit recovery decision을 preview/review/commit합니다.
FReply SCFVehicleAuthoringTab::HandleDriftDecision(const ECFVehicleDriftDecision Decision)
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FReply::Handled();
	}
	// Advanced Override일 때만 사용되는 explicit reason입니다.
	const FString OverrideReason = DriftOverrideReasonTextBox.IsValid() ? DriftOverrideReasonTextBox->GetText().ToString().TrimStartAndEnd() : FString();
	// Empty field subset은 current External Drift 전체 group을 의미합니다.
	TArray<FCFVehicleFieldPath> AllCurrentDriftFields;
	// Exact mutation0 reviewed decision입니다.
	FCFVehicleDriftDecisionPreview Preview;
		if (!ViewModel->PrepareDriftDecision(AllCurrentDriftFields, Decision, OverrideReason, Preview))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ViewModel->GetLastMessage()));
		return FReply::Handled();
	}
	// 기본 USER review에는 내부 fingerprint/hash를 노출하지 않고 처리 의미와 mutation boundary만 표시합니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("외부 변경 처리 검토\n\n선택: %s\n대상 항목: %d\n결정 후 예상 차량 데이터 변경: %d\n이 단계의 차량 데이터 직접 변경: 없음\n자동 저장: 안 함\n\n%s\n\n이 처리 방향을 승인하시겠습니까?"),
		*CFVehicleP11UIPrivate::DriftDecisionText(Decision),
		Preview.Review.Rows.Num(),
		Preview.ProspectiveResolveResult.FieldDiff.Num(),
		Decision == ECFVehicleDriftDecision::KeepAuthoring
			? TEXT("제작 기준값 유지는 처리 방향만 승인합니다. 실제 차량 데이터 복구 적용은 다음 단계에서 다시 확인합니다.")
			: TEXT("이 결정은 레시피의 값 출처 소유권만 변경합니다. 차량 데이터 적용은 별도 단계입니다.")));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Reviewed Drift decision terminal result입니다.
		FCFAuthoringOpResult Result;
		if (!ViewModel->ExecutePreparedDriftDecision(Result))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result.Message));
			return FReply::Handled();
		}
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Mesh-only Candidate에서 Definition+Recipe 생성 proposal을 review/commit합니다.
FReply SCFVehicleAuthoringTab::HandleCreateVehicleFromMesh()
{
	if (!ViewModel.IsValid() || !ViewModel->IsMeshOnlyCandidate()
		|| !NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return FReply::Handled();
	}
	// Explicit two-record creation request입니다.
	FCFVehicleRecordCreateRequest Request;
	Request.DefinitionPackageName = NewDefinitionPackageTextBox->GetText().ToString().TrimStartAndEnd();
	Request.DefinitionAssetName = FName(*NewDefinitionNameTextBox->GetText().ToString().TrimStartAndEnd());
	Request.RecipePackageName = NewRecipePackageTextBox->GetText().ToString().TrimStartAndEnd();
	Request.RecipeAssetName = FName(*NewRecipeNameTextBox->GetText().ToString().TrimStartAndEnd());
	Request.ChassisMesh = TSoftObjectPtr<UStaticMesh>(ViewModel->GetSelectedEntry().ChassisMeshPath);
	// No Profile inference: Request.ProfileBindings는 의도적으로 default empty입니다.
	// Mutation0 creation proposal입니다.
	FCFVehicleRecordCreatePreview Preview;
	if (!ViewModel->PrepareVehicleRecordCreate(Request, Preview))
	{
		return FReply::Handled();
	}
	// Explicit R2 record creation review dialog입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("메시에서 차량 만들기 검토\n\n차체 메시: %s\n생성할 차량 데이터: %s\n생성할 레시피: %s\n프로필 자동 추론: 안 함\n차급 자동 추론: 안 함\n물리/밸런스 자동 추론: 안 함\n차량 데이터 자동 적용: 안 함\n자동 저장: 안 함\n\nC++ 기본값을 사용하는 차량 데이터 + Editor 전용 레시피를 생성하시겠습니까?"),
		*Request.ChassisMesh.ToSoftObjectPath().ToString(),
		*Preview.ProspectiveDefinitionPath.ToString(),
		*Preview.ProspectiveRecipePath.ToString()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Exact two-record terminal result입니다.
		FCFVehicleRecordCreateResult Result;
		ViewModel->ExecutePreparedVehicleCreate(Result);
		HandleRefreshBrowser();
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Last Shared Profile preview의 affected Vehicle impact를 표시합니다.
FText SCFVehicleAuthoringTab::GetProfileImpactText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}
	// Last reviewed Profile impact snapshot입니다.
	const FCFProfileNumericEditPreview& Impact = ViewModel->GetProfileImpactPreview();
	if (Impact.AffectedVehicles.IsEmpty())
	{
				return FText::FromString(TEXT("영향 미리보기가 없습니다. 편집할 항목을 선택하고 새 숫자 값을 입력한 뒤 변경을 검토하세요."));
	}
	return FText::FromString(FString::Printf(
		TEXT("공유 프로필 변경 미리보기 — 영향받는 차량: %d | 예상 차량 데이터 변경: %d\n프로필 저장 뒤 관련 차량 데이터는 자동 적용되지 않으며 새로 해석해야 합니다."),
		Impact.AffectedVehicleCount,
		Impact.PendingDefinitionChangeCount));
}

// Current External Drift의 Last Applied / Raw / Authoring 3-way rows를 표시합니다.
FText SCFVehicleAuthoringTab::GetDriftReviewText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasExternalDrift())
	{
		return FText::FromString(TEXT("현재 외부 변경이 없습니다."));
	}
	// Current read-only 3-way review rows입니다.
	const FCFVehicleDriftReviewResult& Review = ViewModel->GetDriftReview();
		if (Review.Rows.IsEmpty())
	{
		return FText::FromString(TEXT("외부 변경 상세 비교가 아직 준비되지 않았습니다. '미리보기 새로고침'으로 현재 값을 다시 읽어주세요."));
	}
	// Deterministic review presentation입니다.
	FString Text = FString::Printf(TEXT("외부 변경 상세 비교 — %d개 항목 | 제작 기준값 유지 승인: %s\n\n"), Review.Rows.Num(), ViewModel->HasAcceptedDriftKeep() ? TEXT("있음") : TEXT("없음"));
	for (const FCFVehicleDriftReviewRow& Row : Review.Rows)
	{
		// Old Recipe generation에서는 exact last value 대신 hash-only fallback을 표시합니다.
		const FString LastAppliedText = Row.bHasLastAppliedValue
			? Row.LastAppliedValue.CanonicalValueText
			: FString::Printf(TEXT("<값 기록 없음 / 해시 %s>"), *CFVehicleP11UIPrivate::ShortHash(Row.LastAppliedValueHash));
		Text += FString::Printf(
			TEXT("%s\n  마지막 적용값       : %s\n  현재 원본값         : %s\n  현재 제작 기준값    : %s\n  고급 덮어쓰기 허용  : %s\n\n"),
			*Row.FieldPath.ToCanonicalString(true),
			*LastAppliedText,
			*Row.CurrentRawValue.CanonicalValueText,
			*Row.CurrentAuthoringValue.CanonicalValueText,
			Row.bAdvancedOverrideAllowed ? TEXT("허용") : TEXT("허용 안 함"));
	}
	return FText::FromString(Text);
}

// Current Mesh-only Candidate creation summary를 표시합니다.
FText SCFVehicleAuthoringTab::GetMeshCandidateText() const
{
	if (!ViewModel.IsValid() || !ViewModel->IsMeshOnlyCandidate())
	{
		return FText::GetEmpty();
	}
	return FText::FromString(FString::Printf(
		TEXT("차체 메시 후보\n메시: %s\n\n이 항목은 아직 차량 데이터가 없는 차체 메시입니다. '메시에서 차량 만들기'는 차량 데이터 + 레시피만 생성하며 프로필/차급/물리/밸런스 값을 자동 추론하지 않습니다."),
		*ViewModel->GetSelectedEntry().ChassisMeshPath.ToString()));
}

// Definition이 아닌 Mesh-only Candidate creation section visibility입니다.
EVisibility SCFVehicleAuthoringTab::GetMeshCandidateVisibility() const
{
	return ViewModel.IsValid() && ViewModel->IsMeshOnlyCandidate() ? EVisibility::Visible : EVisibility::Collapsed;
}

// Current Mesh-only Candidate selection에 따라 P0-11 creation input suggestions를 갱신합니다.
void SCFVehicleAuthoringTab::SyncP11FieldsFromSelection()
{
	if (!NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid() || !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
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
	// Candidate asset name에서 사람이 편집할 record name suggestion을 만듭니다. Semantic inference가 아닙니다.
	FString Stem = ViewModel->GetSelectedEntry().ChassisMeshPath.GetAssetName();
	Stem.RemoveFromStart(TEXT("SM_"));
	if (Stem.IsEmpty())
	{
		Stem = TEXT("Vehicle");
	}
	// Suggested Definition object name입니다.
	const FString DefinitionName = TEXT("DA_Vehicle_") + Stem;
	// Suggested Recipe object name입니다.
	const FString RecipeName = TEXT("DA_Recipe_") + Stem;
	NewDefinitionPackageTextBox->SetText(FText::FromString(TEXT("/Game/CarFight/Data/Authoring/") + DefinitionName));
	NewDefinitionNameTextBox->SetText(FText::FromString(DefinitionName));
	NewRecipePackageTextBox->SetText(FText::FromString(TEXT("/Game/CarFight/Data/Authoring/") + RecipeName));
	NewRecipeNameTextBox->SetText(FText::FromString(RecipeName));
}
