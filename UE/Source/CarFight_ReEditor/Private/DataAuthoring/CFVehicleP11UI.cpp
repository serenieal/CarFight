// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleP11UI.cpp
// Version: v1.2.0
// Date: 2026-08-18
// Description: DAUTH-P0-11~12 Frozen 24.91~24.94 normal Workspace Slate UI입니다.
// Scope: 5-domain Profile binding/open, Shared Profile B2 nested editor/impact navigation, External Drift 3-way recovery, Mesh-only Candidate two-record creation을 제공합니다.
// Changelog:
// - v1.2.0: P0-12 UA-03 USER feedback에 따라 current Profile binding, existing 후보 선택, explicit Recipe-only bind와 Open Profile UX를 추가.
// - v1.1.0: P0-12 UA-01 사용자 피드백에 따라 Shared Profile, affected Vehicle, Drift 3-way, Mesh Create 문구를 한국어 우선으로 정리.
// - v1.0.0: Frozen UX completeness technical route 최초 구현.
// Migration:
// - 시각/사용성 최종 판정은 P0-12에 남기며 이 파일은 Common Authoring facade/ViewModel 도달성만 제공합니다.
// - raw UProperty SetField, direct existing Target write, Auto Apply/Save/Retry를 제공하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringTab.h"

#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleUXTypes.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
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
			return TEXT("Authoring 값 유지");
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
			SAssignNew(ProfileColumnIdTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("편집할 항목 ID (ColumnId)")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SAssignNew(ProfileNumericValueTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("새 숫자 값")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
				[
			SNew(SButton)
			.Text(FText::FromString(TEXT("공유 프로필 변경 검토…")))
			.IsEnabled(this, &SCFVehicleAuthoringTab::HasBoundSelectedProfile)
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

// P0-11 Sync context의 External Drift 3-way review/recovery panel을 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildP11SyncPanel()
{
	// External Drift review/recovery root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	Root->AddSlot().AutoHeight().Padding(4.0f)[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetSyncContextText).AutoWrapText(true)];
	Root->AddSlot().AutoHeight().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("외부 변경 3-way 검토"))).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift() && ViewModel->HasRecipe(); }).OnClicked(this, &SCFVehicleAuthoringTab::HandleRefreshDriftReview)];
	Root->AddSlot().FillHeight(1.0f).Padding(4.0f)[SNew(SScrollBox) + SScrollBox::Slot()[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetDriftReviewText).AutoWrapText(true)]];
	Root->AddSlot().AutoHeight().Padding(4.0f)[SAssignNew(DriftOverrideReasonTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("고급 덮어쓰기 이유 — 원본값 승격 시 필수")))];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("Authoring 값 유지…"))).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift(); }).OnClicked_Lambda([this]() { return HandleDriftDecision(ECFVehicleDriftDecision::KeepAuthoring); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("원본값을 레거시 고정값으로 보존…"))).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift(); }).OnClicked_Lambda([this]() { return HandleDriftDecision(ECFVehicleDriftDecision::PreserveRawAsLegacyPin); })]
		+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("원본값을 고급 덮어쓰기로 승격…"))).IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->HasExternalDrift(); }).OnClicked_Lambda([this]() { return HandleDriftDecision(ECFVehicleDriftDecision::PromoteRawToAdvancedOverride); })]
	];
	return Root;
}

// Profile Domain 선택을 바꾸고 해당 Domain의 existing Profile 후보를 read-only 갱신합니다.
FReply SCFVehicleAuthoringTab::HandleProfileDomainSelected(const ECFVehicleProfileDomain ProfileDomain)
{
	SelectedProfileDomain = ProfileDomain;
	RefreshProfileChoiceRows();
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
		TEXT("현재 연결: %s\n사용 가능한 %s 프로필: %d\n프로필 연결은 Recipe만 변경하며 VehicleData와 Shared Profile payload는 직접 변경하지 않습니다."),
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
		TEXT("프로필 연결 검토\n\n프로필 종류: %s\n연결할 프로필: %s\n변경 대상: 현재 Recipe의 Profile Binding\nVehicleData 직접 변경: 없음\nShared Profile 값 변경: 없음\n자동 저장: 안 함\n\n이 프로필을 현재 차량 Recipe에 연결하시겠습니까?"),
		*CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain),
		*SelectedProfileChoice->ProfilePath.ToString()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	// Existing BindVehicleProfile R1 semantic transaction 결과입니다.
	FCFAuthoringOpResult BindResult;
	ViewModel->CommitProfileBinding(SelectedProfileDomain, SelectedProfileChoice->ProfilePath, BindResult);
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
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !ProfileColumnIdTextBox.IsValid() || !ProfileNumericValueTextBox.IsValid())
	{
		return FReply::Handled();
	}
	// User-entered stable editable ColumnId입니다.
	const FString ColumnId = ProfileColumnIdTextBox->GetText().ToString().TrimStartAndEnd();
	// User-entered canonical numeric value입니다.
	const FString NumericValue = ProfileNumericValueTextBox->GetText().ToString().TrimStartAndEnd();
	// Existing B2 mutation0 preview/impact입니다.
	FCFProfileNumericEditPreview Preview;
	if (!ViewModel->PrepareProfileNumericEdit(SelectedProfileDomain, ColumnId, NumericValue, Preview))
	{
		return FReply::Handled();
	}
	// Explicit Shared Profile B2 review dialog입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("공유 프로필 변경 검토\n\n프로필 종류: %s\n항목 ID: %s\n새 값: %s\n영향받는 차량: %d\n예상 VehicleData 변경: %d\nVehicleData 자동 적용: 안 함\n자동 저장: 안 함\n\n공유 프로필 값을 변경하시겠습니까?"),
		*CFVehicleP11UIPrivate::ProfileDomainText(SelectedProfileDomain),
		*ColumnId,
		*NumericValue,
		Preview.AffectedVehicleCount,
		Preview.PendingDefinitionChangeCount));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Existing B2 terminal result입니다.
		FCFProfileNumericEditResult Result;
		ViewModel->ExecutePreparedProfileEdit(Result);
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
		return FReply::Handled();
	}
	// Explicit R2 drift recovery review dialog입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("외부 변경 복구 검토\n\n선택: %s\n대상 항목: %d\n레시피 식별값: %s\nVehicleData 식별값: %s\n예상 변경: %d\n이 단계의 VehicleData 직접 변경: 없음\n자동 저장: 안 함\n\n이 복구 결정을 반영하시겠습니까?"),
		*CFVehicleP11UIPrivate::DriftDecisionText(Decision),
		Preview.Review.Rows.Num(),
		*CFVehicleP11UIPrivate::ShortHash(Preview.Review.ExpectedRecipeFingerprint),
		*CFVehicleP11UIPrivate::ShortHash(Preview.Review.ExpectedTargetDefinitionHash),
		Preview.ProspectiveResolveResult.FieldDiff.Num()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Reviewed Drift decision terminal result입니다.
		FCFAuthoringOpResult Result;
		ViewModel->ExecutePreparedDriftDecision(Result);
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
		TEXT("메시에서 차량 만들기 검토\n\n차체 메시: %s\n생성할 VehicleData: %s\n생성할 레시피: %s\n프로필 자동 추론: 안 함\n차급 자동 추론: 안 함\n물리/밸런스 자동 추론: 안 함\nVehicleData 자동 적용: 안 함\n자동 저장: 안 함\n\nC++ 기본값을 사용하는 VehicleData + Editor 전용 레시피를 생성하시겠습니까?"),
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
		return FText::FromString(TEXT("영향 미리보기가 없습니다. 항목 ID와 숫자 값을 입력한 뒤 변경을 검토하세요."));
	}
	return FText::FromString(FString::Printf(
		TEXT("공유 프로필 변경 미리보기 — 영향받는 차량: %d | 예상 VehicleData 변경: %d\n프로필 저장 뒤 관련 VehicleData는 자동 적용되지 않으며 새로 해석해야 합니다."),
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
		return FText::FromString(TEXT("외부 변경이 있습니다. '외부 변경 3-way 검토'를 실행해 마지막 적용값 / 현재 원본값 / Authoring 예상값을 비교하세요."));
	}
	// Deterministic review presentation입니다.
	FString Text = FString::Printf(TEXT("3-way 외부 변경 검토 — %d개 항목 | Authoring 유지 승인: %s\n\n"), Review.Rows.Num(), ViewModel->HasAcceptedDriftKeep() ? TEXT("있음") : TEXT("없음"));
	for (const FCFVehicleDriftReviewRow& Row : Review.Rows)
	{
		// Old Recipe generation에서는 exact last value 대신 hash-only fallback을 표시합니다.
		const FString LastAppliedText = Row.bHasLastAppliedValue
			? Row.LastAppliedValue.CanonicalValueText
			: FString::Printf(TEXT("<값 기록 없음 / 해시 %s>"), *CFVehicleP11UIPrivate::ShortHash(Row.LastAppliedValueHash));
		Text += FString::Printf(
			TEXT("%s\n  마지막 적용값      : %s\n  현재 원본값        : %s\n  Authoring 예상값   : %s\n  고급 덮어쓰기 허용 : %s\n\n"),
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
		TEXT("차체 메시 후보\n메시: %s\n\n이 항목은 아직 VehicleData가 없는 차체 메시입니다. '메시에서 차량 만들기'는 VehicleData + 레시피만 생성하며 프로필/차급/물리/밸런스 값을 자동 추론하지 않습니다."),
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
