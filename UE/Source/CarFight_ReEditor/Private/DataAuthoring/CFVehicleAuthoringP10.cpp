// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringP10.cpp
// Version: v1.4.0
// Date: 2026-09-11
// Description: DAUTH-P0-10~12 Existing Wizard Migration / Frozen adoption parity page 구현입니다.
// Changelog:
// - v1.4.0: CF-FQ-038 DEL6 closure에 맞춰 제거된 Legacy Wizard 유지 문구를 Current Vehicle Authoring Workspace 기준으로 갱신. 동작 변경 0.
// - v1.3.0: P0-12 UA-07 Technical Readiness에서 Driving Feel의 Recipe→Preview→Target 분리 설명과 4축 저장/프리셋 실패 feedback을 보강. Core/Resolver/Preset semantic 변경 0.
// - v1.2.0: P0-12 UA-01 사용자 피드백에 따라 Assets/Layout, Driving Feel, Mount/Compare 사용자 문구를 한국어 우선으로 정리.
// - v1.1.0: Frozen 24.90 Existing Import completeness를 위해 Handling/Performance Adoption normal Workspace 버튼 추가.
// - v1.0.0: Frozen Section 24 P0-10 parity UI 최초 구현.
// Migration:
// - Authoring 동작은 FCFVehicleAuthoringVM을 통해 Common Authoring facade를 사용합니다.
// - Driving Feel 프리셋은 기존 exact Recipe 4축 shortcut을 유지하며 Target 직접 변경이나 자동 저장을 추가하지 않습니다.
// - Legacy Wizard retirement 이후 기존 parity 기능은 Current Vehicle Authoring Workspace가 소유하며 Batch main page는 추가하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringTab.h"

#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Engine/StaticMesh.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

namespace CFVehicleAuthoringP10Private
{
	// Source enum을 사람이 읽을 수 있는 display text로 변환합니다.
	FString SourceTypeText(const ECFVehicleSourceType SourceType)
	{
		// Reflection display name을 제공하는 enum입니다.
		const UEnum* SourceEnum = StaticEnum<ECFVehicleSourceType>();
		return SourceEnum ? SourceEnum->GetDisplayNameTextByValue(static_cast<int64>(SourceType)).ToString() : FString::FromInt(static_cast<int32>(SourceType));
	}

	// Long fingerprint/hash를 review 화면에서 식별 가능한 prefix로 줄입니다.
	FString ShortHash(const FString& Hash)
	{
		return Hash.Len() > 14 ? Hash.Left(14) + TEXT("…") : Hash;
	}

	// Typed enum input 문자열을 exact enum value로 변환합니다.
	template<typename EnumType>
	bool ParseEnumText(const FString& InputText, EnumType& OutValue)
	{
		// Enum reflection descriptor입니다.
		const UEnum* Enum = StaticEnum<EnumType>();
		if (!Enum)
		{
			return false;
		}
		// Trimmed enum name입니다.
		const FString TrimmedInput = InputText.TrimStartAndEnd();
		// Exact/short enum name lookup 결과입니다.
		int64 EnumValue = Enum->GetValueByNameString(TrimmedInput, EGetByNameFlags::None);
		if (EnumValue == INDEX_NONE)
		{
			for (int32 EnumIndex = 0; EnumIndex < Enum->NumEnums() - 1; ++EnumIndex)
			{
				if (Enum->GetDisplayNameTextByIndex(EnumIndex).ToString().Equals(TrimmedInput, ESearchCase::IgnoreCase))
				{
					EnumValue = Enum->GetValueByIndex(EnumIndex);
					break;
				}
			}
		}
		if (EnumValue == INDEX_NONE)
		{
			return false;
		}
		OutValue = static_cast<EnumType>(EnumValue);
		return true;
	}
}

// P0-10 Assets/Layout page Slate를 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildAssetsPage()
{
	// Page 전체를 담는 scroll container입니다.
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
	// Editable semantic fields와 read-only derived state를 담는 root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	ScrollBox->AddSlot()[Root];

	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetAssetsLayoutText).AutoWrapText(true)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("에셋 / 소켓 설정 — 레시피만 편집하며 실제 배치 좌표는 Asset Reader와 Resolver가 계산합니다."))).AutoWrapText(true)
	];

	ChassisMeshTextBox = SNew(SEditableTextBox).HintText(FText::FromString(TEXT("차체 StaticMesh 경로")));
	Root->AddSlot().AutoHeight().Padding(4.0f)[ChassisMeshTextBox.ToSharedRef()];

	WheelMeshTextBoxes.SetNum(4);
	WheelSocketTextBoxes.SetNum(4);
	// Frozen FL/FR/RL/RR order label입니다.
	static const TCHAR* WheelLabels[4] = { TEXT("FL"), TEXT("FR"), TEXT("RL"), TEXT("RR") };
	for (int32 WheelIndex = 0; WheelIndex < 4; ++WheelIndex)
	{
		WheelMeshTextBoxes[WheelIndex] = SNew(SEditableTextBox)
			.HintText(FText::FromString(FString::Printf(TEXT("휠 %s StaticMesh 경로"), WheelLabels[WheelIndex])));
		WheelSocketTextBoxes[WheelIndex] = SNew(SEditableTextBox)
			.HintText(FText::FromString(FString::Printf(TEXT("휠 %s 차체 소켓 이름"), WheelLabels[WheelIndex])));
		Root->AddSlot().AutoHeight().Padding(4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.65f).Padding(0.0f, 0.0f, 4.0f, 0.0f)[WheelMeshTextBoxes[WheelIndex].ToSharedRef()]
			+ SHorizontalBox::Slot().FillWidth(0.35f)[WheelSocketTextBoxes[WheelIndex].ToSharedRef()]
		];
	}
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SButton).Text(FText::FromString(TEXT("에셋 / 소켓 설정 저장"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleCommitAssetIntent)
	];

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("레거시 값 관리 전환 — 기존 값을 레거시 고정 상태에서 Authoring 관리로 넘길 때만 사용합니다.")))
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("외형 에셋 관리 전환…"))).OnClicked_Lambda([this]() { return HandleAdoptionGroup(ECFVehicleAdoptGroup::VisualAssets); })
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("휠 / 배치 관리 전환…"))).OnClicked_Lambda([this]() { return HandleAdoptionGroup(ECFVehicleAdoptGroup::Layout); })
		]
				+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton).Text(FText::FromString(TEXT("하드포인트 관리 전환…"))).OnClicked_Lambda([this]() { return HandleAdoptionGroup(ECFVehicleAdoptGroup::Hardpoints); })
		]
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("핸들링 관리 전환…"))).OnClicked_Lambda([this]() { return HandleAdoptionGroup(ECFVehicleAdoptGroup::Handling); })
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton).Text(FText::FromString(TEXT("성능 관리 전환…"))).OnClicked_Lambda([this]() { return HandleAdoptionGroup(ECFVehicleAdoptGroup::Performance); })
		]
	];

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("휠 측정값 검토 — 선택한 결과만 레시피의 에셋 채택 기록에 반영됩니다.")))
	];
	// Resolver가 지원하는 최대 네 wheel measurement field UI row입니다.
	for (int32 ProposalIndex = 0; ProposalIndex < 4; ++ProposalIndex)
	{
		Root->AddSlot().AutoHeight().Padding(4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock).Text_Lambda([this, ProposalIndex]()
				{
					if (!ViewModel.IsValid() || !ViewModel->GetMeasurementResult().Proposals.IsValidIndex(ProposalIndex))
					{
						return FText::FromString(TEXT("<measurement 없음>"));
					}
					// Current exact measurement proposal입니다.
					const FCFVehicleMeasurementProposal& Proposal = ViewModel->GetMeasurementResult().Proposals[ProposalIndex];
					return FText::FromString(FString::Printf(
						TEXT("%s = %s | %s | %s"),
						*Proposal.FieldPath.ToCanonicalString(true),
						*Proposal.MeasuredCandidateValue.CanonicalValueText,
						*Proposal.MeasurementRuleId.ToString(),
						*CFVehicleAuthoringP10Private::ShortHash(Proposal.AssetFingerprint)));
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("측정값 사용…")))
				.IsEnabled_Lambda([this, ProposalIndex]() { return ViewModel.IsValid() && ViewModel->GetMeasurementResult().Proposals.IsValidIndex(ProposalIndex); })
				.OnClicked_Lambda([this, ProposalIndex]() { return HandleMeasurementDecision(ProposalIndex, ECFVehicleMeasureDecision::AcceptMeasuredValue); })
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("호환 기본값 유지…")))
				.IsEnabled_Lambda([this, ProposalIndex]() { return ViewModel.IsValid() && ViewModel->GetMeasurementResult().Proposals.IsValidIndex(ProposalIndex); })
				.OnClicked_Lambda([this, ProposalIndex]() { return HandleMeasurementDecision(ProposalIndex, ECFVehicleMeasureDecision::UseCompatibilityDefault); })
			]
		];
	}
	return ScrollBox;
}

// P0-10 4-axis Driving Feel page Slate를 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildDrivingFeelPage()
{
	// Driving Feel page root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetDrivingFeelText).AutoWrapText(true)
	];

	// 공통 4축 slider row 생성 helper입니다.
	auto AddFeelSlider = [&Root](const FString& Label, TSharedPtr<SSlider>& Slider, float* PendingValue)
	{
		Slider = SNew(SSlider)
			.Value_Lambda([PendingValue]() { return *PendingValue; })
			.OnValueChanged_Lambda([PendingValue](const float NewValue) { *PendingValue = NewValue; });
		Root->AddSlot().AutoHeight().Padding(4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.28f)
			[
				SNew(STextBlock).Text_Lambda([Label, PendingValue]() { return FText::FromString(FString::Printf(TEXT("%s  %.2f"), *Label, *PendingValue)); })
			]
			+ SHorizontalBox::Slot().FillWidth(0.72f)[Slider.ToSharedRef()]
		];
	};
	AddFeelSlider(TEXT("가속 반응"), AccelerationSlider, &PendingAccelerationFeel);
	AddFeelSlider(TEXT("조향 민첩성"), SteeringSlider, &PendingSteeringAgility);
	AddFeelSlider(TEXT("접지감"), GripSlider, &PendingGripFeel);
	AddFeelSlider(TEXT("서스펜션 단단함"), SuspensionSlider, &PendingSuspensionFirmness);

				Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SButton).Text(FText::FromString(TEXT("4축 주행감 저장"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleCommitDrivingFeel)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("차량 유형 프리셋 — 레시피의 4축 값만 한 번에 바꿉니다. 공유 프로필과 차량 데이터는 자동으로 바뀌지 않습니다."))).AutoWrapText(true)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("세단"))).OnClicked_Lambda([this]() { return HandleDrivingFeelPreset(TEXT("Sedan")); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("SUV"))).OnClicked_Lambda([this]() { return HandleDrivingFeelPreset(TEXT("SUV")); })]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("스포츠"))).OnClicked_Lambda([this]() { return HandleDrivingFeelPreset(TEXT("Sports")); })]
		+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("중량형"))).OnClicked_Lambda([this]() { return HandleDrivingFeelPreset(TEXT("Heavy")); })]
	];
	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("기존 Quick Tune의 원시 Movement 값 매핑과 원시값→4축 역추론은 이 작업창에서 사용하지 않습니다."))).AutoWrapText(true)
	];
	return Root;
}

// P0-10 Mounts & Defaults migration page Slate를 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildMountsPage()
{
	// Mount/Defaults page scroll container입니다.
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
	// Stable-ID authoring controls root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	ScrollBox->AddSlot()[Root];
	Root->AddSlot().AutoHeight().Padding(4.0f)[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetMountsText).AutoWrapText(true)];

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("하드포인트 설정 — 고정 LocationSlotId 기준")))];
	HardpointIdTextBox = SNew(SEditableTextBox).HintText(FText::FromString(TEXT("LocationSlotId")));
	HardpointCategoryTextBox = SNew(SEditableTextBox).HintText(FText::FromString(TEXT("위치 분류 (예: Front/Top)")));
	HardpointSocketTextBox = SNew(SEditableTextBox).HintText(FText::FromString(TEXT("차체 소켓 이름")));
	Root->AddSlot().AutoHeight().Padding(4.0f)[HardpointIdTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[HardpointCategoryTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[HardpointSocketTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("하드포인트 저장"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleUpsertHardpoint)];

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("장착점 설정 — 고정 MountProfileId / LocationSlotRef 기준")))];
	MountIdTextBox = SNew(SEditableTextBox).HintText(FText::FromString(TEXT("MountProfileId")));
	MountLocationTextBox = SNew(SEditableTextBox).HintText(FText::FromString(TEXT("LocationSlotRef")));
	MountTypeTextBox = SNew(SEditableTextBox).Text(FText::FromString(TEXT("Turret"))).HintText(FText::FromString(TEXT("장착 종류 enum 이름 (예: Turret)")));
	MountSizeTextBox = SNew(SEditableTextBox).Text(FText::FromString(TEXT("Medium"))).HintText(FText::FromString(TEXT("무기 크기 제한 enum 이름 (예: Medium)")));
	Root->AddSlot().AutoHeight().Padding(4.0f)[MountIdTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[MountLocationTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[MountTypeTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[MountSizeTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("장착점 저장"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleUpsertMount)];

	Root->AddSlot().AutoHeight().Padding(4.0f, 12.0f, 4.0f, 4.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("기본 데이터 설정")))];
	DestroyedFxSocketTextBox = SNew(SEditableTextBox).Text(FText::FromString(TEXT("FX_Destroyed"))).HintText(FText::FromString(TEXT("파괴 FX 차체 소켓 이름")));
	Root->AddSlot().AutoHeight().Padding(4.0f)[DestroyedFxSocketTextBox.ToSharedRef()];
	Root->AddSlot().AutoHeight().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("기본 데이터 저장"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleCommitDefaultIntent)];
	return ScrollBox;
}

// P0-10 Pending/Reference Compare page Slate를 구성합니다.
TSharedRef<SWidget> SCFVehicleAuthoringTab::BuildComparePage()
{
	// Compare page root입니다.
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
	Root->AddSlot().FillHeight(0.45f).Padding(4.0f)
	[
		SNew(SScrollBox) + SScrollBox::Slot()[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetDiffText).AutoWrapText(true)]
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("비교 차량 — 117개 안정 필드를 읽기 전용으로 비교합니다. 비교 차량의 값을 자동 복사하지 않습니다."))).AutoWrapText(true)
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SAssignNew(ReferenceComboBox, SComboBox<TSharedPtr<FCFVehicleListEntry>>)
		.OptionsSource(&BrowserRows)
		.OnGenerateWidget_Lambda([](TSharedPtr<FCFVehicleListEntry> Item)
		{
			// Reference option display label입니다.
			const FString Label = Item.IsValid() ? Item->DefinitionPath.GetAssetName() : TEXT("<Invalid>");
			return SNew(STextBlock).Text(FText::FromString(Label));
		})
		.OnSelectionChanged(this, &SCFVehicleAuthoringTab::HandleReferenceSelectionChanged)
		[
			SNew(STextBlock).Text_Lambda([this]()
			{
				return FText::FromString(SelectedReferenceRow.IsValid() ? SelectedReferenceRow->DefinitionPath.GetAssetName() : TEXT("비교 차량 선택"));
			})
		]
	];
	Root->AddSlot().AutoHeight().Padding(4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("전체 비교"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleReferenceCompareAll)]
		+ SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("차이만 보기"))).OnClicked(this, &SCFVehicleAuthoringTab::HandleReferenceCompareChanged)]
	];
	Root->AddSlot().FillHeight(0.55f).Padding(4.0f)
	[
		SNew(SScrollBox) + SScrollBox::Slot()[SNew(STextBlock).Text(this, &SCFVehicleAuthoringTab::GetReferenceCompareText).AutoWrapText(true)]
	];
	return Root;
}

// Asset/Socket text를 typed Recipe AssetIntent로 반영합니다.
FReply SCFVehicleAuthoringTab::HandleCommitAssetIntent()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FReply::Handled();
	}
	// Current semantic state를 기반으로 수정할 AssetIntent입니다.
	FCFVehicleAssetIntent AssetIntent = ViewModel->GetRecipe()->AssetIntent;
	AssetIntent.ChassisMesh = ParseStaticMeshPath(ChassisMeshTextBox);
	if (WheelMeshTextBoxes.Num() == 4)
	{
		AssetIntent.WheelMeshFL = ParseStaticMeshPath(WheelMeshTextBoxes[0]);
		AssetIntent.WheelMeshFR = ParseStaticMeshPath(WheelMeshTextBoxes[1]);
		AssetIntent.WheelMeshRL = ParseStaticMeshPath(WheelMeshTextBoxes[2]);
		AssetIntent.WheelMeshRR = ParseStaticMeshPath(WheelMeshTextBoxes[3]);
	}
	if (WheelSocketTextBoxes.Num() == 4)
	{
		AssetIntent.BodyWheelSocketFL = FName(*WheelSocketTextBoxes[0]->GetText().ToString().TrimStartAndEnd());
		AssetIntent.BodyWheelSocketFR = FName(*WheelSocketTextBoxes[1]->GetText().ToString().TrimStartAndEnd());
		AssetIntent.BodyWheelSocketRL = FName(*WheelSocketTextBoxes[2]->GetText().ToString().TrimStartAndEnd());
		AssetIntent.BodyWheelSocketRR = FName(*WheelSocketTextBoxes[3]->GetText().ToString().TrimStartAndEnd());
	}
	// Typed Recipe semantic result입니다.
	FCFAuthoringOpResult CommitResult;
	ViewModel->CommitAssetIntent(AssetIntent, CommitResult);
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Frozen Driving Feel 4축 local values를 한 typed Recipe transaction으로 반영합니다.
FReply SCFVehicleAuthoringTab::HandleCommitDrivingFeel()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FReply::Handled();
	}
	// Frozen all-axis semantic patch입니다.
	FCFDrivingFeelPatch Patch;
	Patch.bSetAccelerationFeel = true;
	Patch.AccelerationFeel = PendingAccelerationFeel;
	Patch.bSetSteeringAgility = true;
	Patch.SteeringAgility = PendingSteeringAgility;
	Patch.bSetGripFeel = true;
	Patch.GripFeel = PendingGripFeel;
	Patch.bSetSuspensionFirmness = true;
	Patch.SuspensionFirmness = PendingSuspensionFirmness;
	// Typed Recipe semantic result입니다.
	FCFAuthoringOpResult CommitResult;
	if (!ViewModel->CommitDrivingFeel(Patch, CommitResult))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(FString::Printf(
				TEXT("4축 주행감 저장에 실패했습니다.\n\n%s"),
				CommitResult.Message.IsEmpty() ? TEXT("현재 레시피 상태와 적용 가능한 프로필을 확인하세요.") : *CommitResult.Message)));
		return FReply::Handled();
	}
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Named Frozen migration preset을 exact 4축 Recipe intent로 반영합니다.
FReply SCFVehicleAuthoringTab::HandleDrivingFeelPreset(const FName PresetId)
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FReply::Handled();
	}
	// Preset semantic result입니다.
	FCFAuthoringOpResult CommitResult;
	if (!ViewModel->CommitDrivingFeelPreset(PresetId, CommitResult))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(FString::Printf(
				TEXT("차량 유형 프리셋 적용에 실패했습니다.\n\n%s"),
				CommitResult.Message.IsEmpty() ? TEXT("현재 레시피 상태와 적용 가능한 프로필을 확인하세요.") : *CommitResult.Message)));
		return FReply::Handled();
	}
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Stable-ID Hardpoint text input을 typed Recipe intent로 upsert합니다.
FReply SCFVehicleAuthoringTab::HandleUpsertHardpoint()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !HardpointIdTextBox.IsValid() || !HardpointSocketTextBox.IsValid())
	{
		return FReply::Handled();
	}
	// Stable-ID Hardpoint semantic intent입니다.
	FCFHardpointIntent Intent;
	Intent.LocationSlotId = FName(*HardpointIdTextBox->GetText().ToString().TrimStartAndEnd());
	Intent.LocationCategory = HardpointCategoryTextBox.IsValid() ? FName(*HardpointCategoryTextBox->GetText().ToString().TrimStartAndEnd()) : NAME_None;
	Intent.SocketName = FName(*HardpointSocketTextBox->GetText().ToString().TrimStartAndEnd());
	// Typed Recipe semantic result입니다.
	FCFAuthoringOpResult CommitResult;
	ViewModel->UpsertHardpointIntent(Intent, CommitResult);
	return FReply::Handled();
}

// Stable-ID Mount text input을 typed Recipe intent로 upsert합니다.
FReply SCFVehicleAuthoringTab::HandleUpsertMount()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !MountIdTextBox.IsValid() || !MountLocationTextBox.IsValid() || !MountTypeTextBox.IsValid() || !MountSizeTextBox.IsValid())
	{
		return FReply::Handled();
	}
	// Parsed Mount type입니다.
	ECFVehicleMountType MountType = ECFVehicleMountType::None;
	// Parsed weapon size limit입니다.
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::None;
	if (!CFVehicleAuthoringP10Private::ParseEnumText(MountTypeTextBox->GetText().ToString(), MountType)
		|| !CFVehicleAuthoringP10Private::ParseEnumText(MountSizeTextBox->GetText().ToString(), SizeLimit))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("장착 종류 또는 무기 크기 제한 값을 확인하세요. 예: Turret / Medium")));
		return FReply::Handled();
	}
	// Stable-ID Mount semantic intent입니다.
	FCFMountIntent Intent;
	Intent.MountProfileId = FName(*MountIdTextBox->GetText().ToString().TrimStartAndEnd());
	Intent.LocationSlotRef = FName(*MountLocationTextBox->GetText().ToString().TrimStartAndEnd());
	Intent.MountType = MountType;
	Intent.SizeLimit = SizeLimit;
	Intent.bExposedModule = true;
	// Typed Recipe semantic result입니다.
	FCFAuthoringOpResult CommitResult;
	ViewModel->UpsertMountIntent(Intent, CommitResult);
	return FReply::Handled();
}

// Default Destroyed FX socket semantic intent를 Recipe에 반영합니다.
FReply SCFVehicleAuthoringTab::HandleCommitDefaultIntent()
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe() || !DestroyedFxSocketTextBox.IsValid())
	{
		return FReply::Handled();
	}
	// Existing DefaultData 의미를 유지하면서 socket만 편집하는 semantic intent입니다.
	FCFVehicleDefaultIntent DefaultIntent = ViewModel->GetRecipe()->DefaultDataIntent;
	DefaultIntent.DestroyedFxSocketName = FName(*DestroyedFxSocketTextBox->GetText().ToString().TrimStartAndEnd());
	// Typed Recipe semantic result입니다.
	FCFAuthoringOpResult CommitResult;
	ViewModel->CommitDefaultDataIntent(DefaultIntent, CommitResult);
	SyncEditableFieldsFromSelection();
	return FReply::Handled();
}

// Browser row를 Reference side로 선택합니다.
void SCFVehicleAuthoringTab::HandleReferenceSelectionChanged(TSharedPtr<FCFVehicleListEntry> Item, ESelectInfo::Type SelectInfo)
{
	SelectedReferenceRow = Item;
	if (!Item.IsValid() || !ViewModel.IsValid())
	{
		return;
	}
	// Read-only Reference selection diagnostic입니다.
	FString CompareError;
	ViewModel->SelectReferenceVehicle(*Item, CompareError);
}

// Reference Compare를 Same 포함 모드로 갱신합니다.
FReply SCFVehicleAuthoringTab::HandleReferenceCompareAll()
{
	if (ViewModel.IsValid())
	{
		// Read-only compare diagnostic입니다.
		FString CompareError;
		ViewModel->RefreshReferenceCompare(false, CompareError);
	}
	return FReply::Handled();
}

// Reference Compare를 Different only 모드로 갱신합니다.
FReply SCFVehicleAuthoringTab::HandleReferenceCompareChanged()
{
	if (ViewModel.IsValid())
	{
		// Read-only compare diagnostic입니다.
		FString CompareError;
		ViewModel->RefreshReferenceCompare(true, CompareError);
	}
	return FReply::Handled();
}

// Wheel measurement proposal의 reviewed decision을 Recipe AssetAdoption에 반영합니다.
FReply SCFVehicleAuthoringTab::HandleMeasurementDecision(const int32 ProposalIndex, const ECFVehicleMeasureDecision Decision)
{
	if (!ViewModel.IsValid() || !ViewModel->GetMeasurementResult().Proposals.IsValidIndex(ProposalIndex))
	{
		return FReply::Handled();
	}
	// UI가 실제 검토할 exact current Resolver proposal입니다.
	const FCFVehicleMeasurementProposal& Proposal = ViewModel->GetMeasurementResult().Proposals[ProposalIndex];
	// Mutation0 prospective decision result입니다.
	FCFVehicleMeasurementPreviewResult Preview;
	if (!ViewModel->PrepareMeasurementDecision(Proposal, Decision, Preview))
	{
		return FReply::Handled();
	}
		// Explicit R2 review action label입니다.
	const TCHAR* DecisionLabel = Decision == ECFVehicleMeasureDecision::AcceptMeasuredValue ? TEXT("측정값 사용") : TEXT("호환 기본값 유지");
	// Exact proposal-bound review text입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("휠 측정값 결정 검토\n\n항목: %s\n후보값: %s\n측정 규칙: %s\n에셋 식별값: %s\n선택: %s\n예상 변경: %d\n자동 저장: 안 함\n\n이 결정을 레시피에 반영하시겠습니까?"),
		*Proposal.FieldPath.ToCanonicalString(true),
		*Proposal.MeasuredCandidateValue.CanonicalValueText,
		*Proposal.MeasurementRuleId.ToString(),
		*CFVehicleAuthoringP10Private::ShortHash(Proposal.AssetFingerprint),
		DecisionLabel,
		Preview.ProspectiveResolveResult.FieldDiff.Num()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Reviewed R2 Recipe semantic result입니다.
		FCFAuthoringOpResult CommitResult;
		ViewModel->ExecutePreparedMeasurement(CommitResult);
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Legacy Pin ownership group Adoption을 review/commit합니다.
FReply SCFVehicleAuthoringTab::HandleAdoptionGroup(const ECFVehicleAdoptGroup AdoptionGroup)
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FReply::Handled();
	}
	// Mutation0 Existing Import Core prospective adoption입니다.
	FCFVehicleAdoptionPreviewResult Preview;
	if (!ViewModel->PrepareGroupAdoption(AdoptionGroup, Preview))
	{
		return FReply::Handled();
	}
	// Exact R2 ownership transition review text입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("레거시 값 관리 전환 검토\n\n그룹: %d\n해제할 레거시 고정값: %d\n예상 변경: %d\n레시피 식별값: %s\n자동 저장: 안 함\n\n이 관리 전환을 반영하시겠습니까?"),
		static_cast<int32>(AdoptionGroup),
		Preview.Preview.LegacyPinPathsToRemove.Num(),
		Preview.Preview.ProspectiveResolveResult.FieldDiff.Num(),
		*CFVehicleAuthoringP10Private::ShortHash(Preview.Proposal.ProspectiveRecipeFingerprint)));
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) == EAppReturnType::Yes)
	{
		// Reviewed R2 Recipe semantic result입니다.
		FCFAuthoringOpResult CommitResult;
		ViewModel->ExecutePreparedAdoption(CommitResult);
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Workspace가 소유한 마지막 transaction을 Unreal standard Undo로 되돌립니다.
FReply SCFVehicleAuthoringTab::HandleUndoLastAction()
{
	if (ViewModel.IsValid())
	{
		// Standard Unreal transaction Undo diagnostic입니다.
		FString UndoError;
		ViewModel->UndoLastWorkspaceAction(UndoError);
		SyncEditableFieldsFromSelection();
	}
	return FReply::Handled();
}

// Assets/Layout intent + derived sockets + measurement proposals를 read-only summary로 만듭니다.
FText SCFVehicleAuthoringTab::GetAssetsLayoutText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("관리 중인 레시피 차량을 선택하면 에셋/배치 설정과 자동 계산된 에셋 정보를 표시합니다."));
	}
	// Current persistent Recipe semantic intent입니다.
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	// Current fresh asset snapshot입니다.
	const FCFVehicleAssetSnapshot& Assets = ViewModel->GetResolveResult().ResolveRequest.Assets;
	// Read-only Assets/Layout summary입니다.
	FString Text = FString::Printf(
		TEXT("에셋 / 배치\n차체 메시: %s\n휠 FL/FR/RL/RR: %s | %s | %s | %s\n소켓 FL/FR/RL/RR: %s | %s | %s | %s\n\n자동 인식된 차체 소켓: %d\n측정값 제안: %d\n\n자동 계산된 위치와 휠 경계값은 읽기 전용입니다."),
		*Recipe->AssetIntent.ChassisMesh.ToSoftObjectPath().ToString(),
		*Recipe->AssetIntent.WheelMeshFL.ToSoftObjectPath().ToString(),
		*Recipe->AssetIntent.WheelMeshFR.ToSoftObjectPath().ToString(),
		*Recipe->AssetIntent.WheelMeshRL.ToSoftObjectPath().ToString(),
		*Recipe->AssetIntent.WheelMeshRR.ToSoftObjectPath().ToString(),
		*Recipe->AssetIntent.BodyWheelSocketFL.ToString(),
		*Recipe->AssetIntent.BodyWheelSocketFR.ToString(),
		*Recipe->AssetIntent.BodyWheelSocketRL.ToString(),
		*Recipe->AssetIntent.BodyWheelSocketRR.ToString(),
		Assets.ChassisSockets.Num(),
		ViewModel->GetMeasurementResult().Proposals.Num());
	for (const FCFVehicleSocketSnapshot& Socket : Assets.ChassisSockets)
	{
				Text += FString::Printf(TEXT("\n- 소켓 %s @ %s"), *Socket.SocketName.ToString(), *Socket.RelativeLocation.ToCompactString());
	}
	return FText::FromString(Text);
}

// 4축 Driving Feel current semantic state를 표시합니다.
FText SCFVehicleAuthoringTab::GetDrivingFeelText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("관리 중인 레시피 차량을 선택하세요."));
	}
	// Current persistent 4-axis semantic intent입니다.
	const FCFVehicleFeelIntent& Feel = ViewModel->GetRecipe()->DrivingFeelIntent;
	return FText::FromString(FString::Printf(
		TEXT("4축 주행감\n가속 반응 %.2f | 조향 민첩성 %.2f | 접지감 %.2f | 서스펜션 단단함 %.2f\n\n슬라이더를 움직이는 동안에는 임시값입니다. '4축 주행감 저장'을 누르면 레시피만 변경되고 새 미리보기의 변경점이 갱신됩니다. 차량 데이터는 별도 적용 전까지 바뀌지 않습니다."),
		Feel.AccelerationFeel, Feel.SteeringAgility, Feel.GripFeel, Feel.SuspensionFirmness));
}

// Mount/Default current intent를 Stable-ID 중심으로 표시합니다.
FText SCFVehicleAuthoringTab::GetMountsText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasRecipe())
	{
		return FText::FromString(TEXT("관리 중인 레시피 차량을 선택하세요."));
	}
	// Current persistent Recipe입니다.
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	// Stable-ID current authoring state summary입니다.
	FString Text = FString::Printf(
		TEXT("장착 / 기본값\n하드포인트 설정: %d\n장착점 설정: %d\n파괴 FX 소켓: %s\n\n"),
		Recipe->HardpointIntents.Num(), Recipe->MountIntents.Num(), *Recipe->DefaultDataIntent.DestroyedFxSocketName.ToString());
	for (const FCFHardpointIntent& Hardpoint : Recipe->HardpointIntents)
	{
				Text += FString::Printf(TEXT("하드포인트 %s | 위치 분류=%s | 소켓=%s\n"), *Hardpoint.LocationSlotId.ToString(), *Hardpoint.LocationCategory.ToString(), *Hardpoint.SocketName.ToString());
	}
	for (const FCFMountIntent& Mount : Recipe->MountIntents)
	{
				Text += FString::Printf(TEXT("장착점 %s -> %s | 종류=%d | 크기 제한=%d\n"), *Mount.MountProfileId.ToString(), *Mount.LocationSlotRef.ToString(), static_cast<int32>(Mount.MountType), static_cast<int32>(Mount.SizeLimit));
	}
	return FText::FromString(Text);
}

// Reference Vehicle Compare 117-field rows를 text presentation으로 만듭니다.
FText SCFVehicleAuthoringTab::GetReferenceCompareText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasReferenceVehicle())
	{
		return FText::FromString(TEXT("비교할 차량을 선택하세요."));
	}
	// Current facade read-only compare result입니다.
	const FCFVehicleReferenceCompareResult& Compare = ViewModel->GetReferenceCompareResult();
	// Deterministic text presentation입니다.
		FString Text = FString::Printf(TEXT("비교 차량: %s\n항목: %d | 차이: %d\n\n"), *ViewModel->GetReferenceEntry().DefinitionPath.GetAssetName(), Compare.Rows.Num(), Compare.DifferentCount);
	for (const FCFVehicleReferenceCompareRow& Row : Compare.Rows)
	{
		// A-side source label입니다.
		const FString CurrentSource = Row.CurrentSourceId.StartsWith(TEXT("Current Definition:")) ? Row.CurrentSourceId : FString::Printf(TEXT("%s / %s"), *CFVehicleAuthoringP10Private::SourceTypeText(Row.CurrentSourceType), *Row.CurrentSourceId);
		// B-side source label입니다.
		const FString ReferenceSource = Row.ReferenceSourceId.StartsWith(TEXT("Current Definition:")) ? Row.ReferenceSourceId : FString::Printf(TEXT("%s / %s"), *CFVehicleAuthoringP10Private::SourceTypeText(Row.ReferenceSourceType), *Row.ReferenceSourceId);
				Text += FString::Printf(
			TEXT("[%s] %s\n  현재 차량: %s\n     값 출처: %s\n  비교 차량: %s\n     값 출처: %s\n\n"),
			Row.bSame ? TEXT("같음") : TEXT("다름"),
			*Row.FieldPath.ToCanonicalString(true),
			Row.bHasCurrentValue ? *Row.CurrentValue.CanonicalValueText : TEXT("<없음>"),
			*CurrentSource,
			Row.bHasReferenceValue ? *Row.ReferenceValue.CanonicalValueText : TEXT("<없음>"),
			*ReferenceSource);
	}
	return FText::FromString(Text);
}

// Workspace-owned last transaction Undo 버튼 활성 조건입니다.
bool SCFVehicleAuthoringTab::IsUndoEnabled() const
{
	return ViewModel.IsValid() && ViewModel->CanUndoLastWorkspaceAction();
}

// Soft asset path text를 typed soft object pointer로 변환합니다.
TSoftObjectPtr<UStaticMesh> SCFVehicleAuthoringTab::ParseStaticMeshPath(const TSharedPtr<SEditableTextBox>& TextBox) const
{
	if (!TextBox.IsValid())
	{
		return TSoftObjectPtr<UStaticMesh>();
	}
	// User-entered trimmed soft object path입니다.
	const FString AssetPath = TextBox->GetText().ToString().TrimStartAndEnd();
	return AssetPath.IsEmpty() ? TSoftObjectPtr<UStaticMesh>() : TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(AssetPath));
}




