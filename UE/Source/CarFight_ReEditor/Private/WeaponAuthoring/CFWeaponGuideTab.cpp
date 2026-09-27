// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideTab.cpp
// Version: v1.2.1
// Date: 2026-09-19
// Description: CF-FQ-055 WEA-P0-04 Guided Draft completion summary와 fail-closed Equipment Builder navigation-only handoff UI 구현입니다.
// Changelog:
// - v1.2.1: Mid-review P2 교정으로 IsEnabled 경로의 반복 asset revalidation을 제거하고 integrity 실패 시 stale technical detail을 숨깁니다.
// - v1.2.0: USER completion summary, technical bundle detail, context injection 없는 Equipment Builder 탭 이동 Action을 추가했습니다.
// - v1.1.0: Review 단계의 명시적 무장 생성 Action, partial durable recovery 상태 안내와 exact completion bundle 표시를 활성화했습니다.
// - v1.0.1: Damage provider의 ExplosionInnerRadius/MinExplosionDamageScale, Ammo provider의 Texture2D AmmoIcon 입력을 normal authoring UI에 추가했습니다.
// - v1.0.0: Stable Step rail, single-scroll Page shell, create/reuse asset picker, conditional Capability pages, provider mutation0 review를 추가했습니다.
// Migration:
// - WEA-P0-03에서만 Review Page의 명시적 무장 생성 Action이 durable child 저장을 수행합니다. 중간 성공 child는 자동 rollback/delete하지 않습니다.
// - CF-FQ-054 Equipment Builder state/business logic은 참조하거나 변경하지 않습니다.

#include "WeaponAuthoring/CFWeaponGuideTab.h"

#include "WeaponAuthoring/CFWeaponGuideVM.h"

#include "AssetRegistry/AssetData.h"
#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFProjectileData.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "Engine/Texture2D.h"
#include "Framework/Docking/TabManager.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SCFWeaponGuideTab"

namespace
{
	// 사용자 label과 실제 입력 widget을 같은 가로 행에 배치합니다.
	TSharedRef<SWidget> BuildFieldRow(const FText& Label, const TSharedRef<SWidget>& FieldWidget)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.30f).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Label).AutoWrapText(true)
			]
			+ SHorizontalBox::Slot().FillWidth(0.70f)
			[
				FieldWidget
			];
	}

	// float Draft 값을 Guided Page에서 편집하는 공통 행을 만듭니다.
	TSharedRef<SWidget> BuildFloatRow(
		const FText& Label,
		TFunction<float()> ValueGetter,
		TFunction<void(float)> ValueSetter,
		const float MinimumValue = 0.0f)
	{
		return BuildFieldRow(
			Label,
			SNew(SNumericEntryBox<float>)
				.Value_Lambda([ValueGetter]() -> TOptional<float>
				{
					return ValueGetter();
				})
				.MinValue(MinimumValue)
				.OnValueChanged_Lambda([ValueSetter](const float NewValue)
				{
					ValueSetter(NewValue);
				}));
	}

	// int32 Draft 값을 Guided Page에서 편집하는 공통 행을 만듭니다.
	TSharedRef<SWidget> BuildIntRow(
		const FText& Label,
		TFunction<int32()> ValueGetter,
		TFunction<void(int32)> ValueSetter,
		const int32 MinimumValue = 0)
	{
		return BuildFieldRow(
			Label,
			SNew(SNumericEntryBox<int32>)
				.Value_Lambda([ValueGetter]() -> TOptional<int32>
				{
					return ValueGetter();
				})
				.MinValue(MinimumValue)
				.OnValueChanged_Lambda([ValueSetter](const int32 NewValue)
				{
					ValueSetter(NewValue);
				}));
	}

	// bool Draft 값을 Guided Page에서 편집하는 공통 체크 행을 만듭니다.
	TSharedRef<SWidget> BuildCheckRow(
		const FText& Label,
		TFunction<bool()> ValueGetter,
		TFunction<void(bool)> ValueSetter)
	{
		return BuildFieldRow(
			Label,
			SNew(SCheckBox)
				.IsChecked_Lambda([ValueGetter]()
				{
					return ValueGetter() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([ValueSetter](const ECheckBoxState NewState)
				{
					ValueSetter(NewState == ECheckBoxState::Checked);
				}));
	}

	// Page 맨 위의 제목과 설명을 일관된 구조로 만듭니다.
	TSharedRef<SWidget> BuildPageHeader(const FText& Title, const FText& Description)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock).Text(Title)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock).Text(Description).AutoWrapText(true)
			];
	}
}

// Slate shell과 session-local Weapon Guide ViewModel을 초기화합니다.
void SCFWeaponGuideTab::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FCFWeaponGuideVM>();
	ViewModel->Reset();
	UiStatusMessage.Reset();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(10.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("Title", "CarFight 무장 제작 가이드"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
					.Text(LOCTEXT("Boundary", "WEA-P0-03 — 최종 검토에서 [무장 생성]을 눌렀을 때만 Damage/Ammo + Turret/Projectile/Weapon child를 durable 저장합니다. 중간 성공 child는 실패 시 자동 삭제하지 않고 같은 세션 재시도에서 재사용합니다. Equipment Builder(CF-FQ-054)는 Paused 상태를 유지합니다."))
					.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SSplitter)
				+ SSplitter::Slot().Value(0.24f)
				[
					SAssignNew(StepRailHost, SBox)
				]
				+ SSplitter::Slot().Value(0.76f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(STextBlock).Text(this, &SCFWeaponGuideTab::GetCurrentStepText)
					]
					+ SVerticalBox::Slot().FillHeight(1.0f).Padding(8.0f, 0.0f, 0.0f, 0.0f)
					[
						SAssignNew(PageScrollBox, SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(PageHost, SBox)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 8.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock).Text(this, &SCFWeaponGuideTab::GetStatusText).AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
								.Text(LOCTEXT("Reset", "새 초안"))
								.ToolTipText(LOCTEXT("ResetTip", "저장된 Asset은 건드리지 않고 현재 transient 무장 초안만 초기화합니다."))
								.OnClicked(this, &SCFWeaponGuideTab::HandleResetDraft)
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
								.Text(LOCTEXT("Previous", "이전"))
								.IsEnabled_Lambda([this]() { return ViewModel.IsValid() && ViewModel->CanMovePrevious(); })
								.OnClicked(this, &SCFWeaponGuideTab::HandlePrevious)
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
								.Text(LOCTEXT("Validate", "현재 단계 확인"))
								.OnClicked(this, &SCFWeaponGuideTab::HandleValidateCurrent)
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SButton)
								.Text_Lambda([this]()
								{
									return ViewModel.IsValid() && ViewModel->GetCurrentStep() == ECFWeaponGuideStep::ReviewCreate
										? LOCTEXT("P003Create", "무장 생성")
										: LOCTEXT("Next", "다음");
								})
								.IsEnabled(this, &SCFWeaponGuideTab::CanUseNext)
								.OnClicked(this, &SCFWeaponGuideTab::HandleNext)
						]
					]
				]
			]
		]
	];

	RefreshView();
}

// 현재 visible Stable Step 진행 상황을 왼쪽 rail로 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildStepRail() const
{
	// Stable Step 목록을 담을 세로 컨테이너입니다.
	TSharedRef<SVerticalBox> StepList = SNew(SVerticalBox);

	// Current Draft의 visible Step projection입니다.
	const TArray<ECFWeaponGuideStep> VisibleSteps = ViewModel->GetVisibleSteps();
	// Current visible Step index입니다.
	const int32 CurrentIndex = VisibleSteps.IndexOfByKey(ViewModel->GetCurrentStep());

	for (int32 StepIndex = 0; StepIndex < VisibleSteps.Num(); ++StepIndex)
	{
		// 현재 rail 행의 Stable StepId입니다.
		const ECFWeaponGuideStep Step = VisibleSteps[StepIndex];
		// 완료/현재/예정 상태를 나타내는 사용자 prefix입니다.
		const FString Prefix = StepIndex < CurrentIndex ? TEXT("✓") : (StepIndex == CurrentIndex ? TEXT("●") : TEXT("○"));

		StepList->AddSlot().AutoHeight().Padding(6.0f, 3.0f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s %s"), *Prefix, *FCFWeaponGuideVM::GetStepDisplayName(Step))))
				.AutoWrapText(true)
		];
	}

	return SNew(SBorder)
		.Padding(6.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(6.0f, 3.0f, 6.0f, 6.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("Steps", "제작 단계"))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				StepList
			]
		];
}

// 현재 Stable Step에 대응하는 Guided Draft Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildCurrentPage()
{
	switch (ViewModel->GetCurrentStep())
	{
	case ECFWeaponGuideStep::IdentityTemplate: return BuildIdentityPage();
	case ECFWeaponGuideStep::MountCompatibility: return BuildMountCompatibilityPage();
	case ECFWeaponGuideStep::MountGeometry: return BuildMountGeometryPage();
	case ECFWeaponGuideStep::FireBehavior: return BuildFireBehaviorPage();
	case ECFWeaponGuideStep::Projectile: return BuildProjectilePage();
	case ECFWeaponGuideStep::Damage: return BuildDamagePage();
	case ECFWeaponGuideStep::Ammo: return BuildAmmoPage();
	case ECFWeaponGuideStep::Launcher: return BuildLauncherPage();
	case ECFWeaponGuideStep::Propulsion: return BuildPropulsionPage();
	case ECFWeaponGuideStep::MissileFlight: return BuildMissileFlightPage();
	case ECFWeaponGuideStep::Guidance: return BuildGuidancePage();
	case ECFWeaponGuideStep::Heat: return BuildHeatPage();
	case ECFWeaponGuideStep::Charge: return BuildChargePage();
	case ECFWeaponGuideStep::ReviewCreate: return BuildReviewPage();
	case ECFWeaponGuideStep::Complete: return BuildCompletePage();
	default: return SNew(STextBlock).Text(LOCTEXT("UnknownPage", "알 수 없는 제작 단계입니다."));
	}
}

// 기본 이름, 표시 이름, 목적, Template 선택 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildIdentityPage()
{
	// Template 버튼을 담는 세로 컨테이너입니다.
	TSharedRef<SVerticalBox> TemplateButtons = SNew(SVerticalBox);
	// 일반 사용자에게 노출할 제작 Template 선택 목록입니다.
	const ECFWeaponGuideTemplate Templates[] =
	{
		ECFWeaponGuideTemplate::DirectFireCannon,
		ECFWeaponGuideTemplate::AutomaticGun,
		ECFWeaponGuideTemplate::RocketLauncher,
		ECFWeaponGuideTemplate::GuidedMissileLauncher,
		ECFWeaponGuideTemplate::Custom
	};
	// 각 Template 선택 버튼을 Stable 순서로 구성합니다.
	for (const ECFWeaponGuideTemplate Template : Templates)
	{
		TemplateButtons->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(SButton)
				.Text_Lambda([this, Template]()
				{
					// 선택 상태를 나타내는 버튼 prefix입니다.
					const FString Prefix = ViewModel->GetDraft().Template == Template ? TEXT("● ") : TEXT("○ ");
					return FText::FromString(Prefix + GetTemplateLabel(Template).ToString());
				})
				.OnClicked_Lambda([this, Template]()
				{
					ViewModel->SetTemplate(Template);
					UiStatusMessage = TEXT("Template 제안값을 적용했습니다. 각 단계의 실제 값을 계속 확인하세요.");
					RefreshView();
					return FReply::Handled();
				})
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("IdentityTitle", "1. 기본 정보 / Template"),
			LOCTEXT("IdentityHelp", "파일명과 게임 표시 이름을 분리합니다. Template은 Runtime 타입을 고정하지 않고 일반적인 Capability 조합만 제안합니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			BuildFieldRow(
				LOCTEXT("AssetName", "에셋 기본 이름"),
				SNew(SEditableTextBox)
					.Text_Lambda([this]() { return FText::FromString(ViewModel->GetDraft().BaseAssetName); })
					.ToolTipText(LOCTEXT("AssetNameTip", "Damage/Ammo/Projectile/Weapon/Turret child ID와 저장 경로를 자동 파생하는 사용자용 기본 이름입니다."))
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						ViewModel->SetBaseAssetName(NewText.ToString());
					}))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			BuildFieldRow(
				LOCTEXT("DisplayName", "게임 표시 이름"),
				SNew(SEditableTextBox)
					.Text_Lambda([this]() { return FText::FromString(ViewModel->GetDraft().SuggestedDisplayName); })
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						ViewModel->SetSuggestedDisplayName(NewText.ToString());
					}))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			BuildFieldRow(
				LOCTEXT("Concept", "무장 목적 메모"),
				SNew(SEditableTextBox)
					.Text_Lambda([this]() { return FText::FromString(ViewModel->GetDraft().ConceptNote); })
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						ViewModel->GetMutableDraft().ConceptNote = NewText.ToString();
					}))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock).Text(LOCTEXT("TemplateLabel", "제작 Template"))
		]
		+ SVerticalBox::Slot().AutoHeight()[TemplateButtons];
}

// 장착 타입과 무기 크기 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildMountCompatibilityPage()
{
	// MountType 선택 버튼 컨테이너입니다.
	TSharedRef<SHorizontalBox> MountButtons = SNew(SHorizontalBox);
	// 일반 Weapon Equipment 제작에서 선택할 수 있는 MountType 목록입니다.
	const ECFVehicleMountType MountTypes[] =
	{
		ECFVehicleMountType::Fixed,
		ECFVehicleMountType::Gimbal,
		ECFVehicleMountType::Turret,
		ECFVehicleMountType::Launcher
	};
	// 각 MountType 선택 버튼을 Stable 순서로 구성합니다.
	for (const ECFVehicleMountType MountType : MountTypes)
	{
		MountButtons->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
		[
			SNew(SButton)
				.Text_Lambda([this, MountType]()
				{
					// MountType 선택 상태 prefix입니다.
					const FString Prefix = ViewModel->GetDraft().RequiredMountType == MountType ? TEXT("● ") : TEXT("○ ");
					return FText::FromString(Prefix + GetMountTypeLabel(MountType).ToString());
				})
				.OnClicked_Lambda([this, MountType]()
				{
					ViewModel->SetRequiredMountType(MountType);
					RefreshView();
					return FReply::Handled();
				})
		];
	}

	// WeaponSize 선택 버튼 컨테이너입니다.
	TSharedRef<SHorizontalBox> SizeButtons = SNew(SHorizontalBox);
	// 일반 무장 호환성 제작에서 선택할 수 있는 WeaponSize 목록입니다.
	const ECFVehicleWeaponSize WeaponSizes[] =
	{
		ECFVehicleWeaponSize::Small,
		ECFVehicleWeaponSize::Medium,
		ECFVehicleWeaponSize::Large
	};
	// 각 WeaponSize 선택 버튼을 Stable 순서로 구성합니다.
	for (const ECFVehicleWeaponSize WeaponSize : WeaponSizes)
	{
		SizeButtons->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
		[
			SNew(SButton)
				.Text_Lambda([this, WeaponSize]()
				{
					// WeaponSize 선택 상태 prefix입니다.
					const FString Prefix = ViewModel->GetDraft().RequiredWeaponSize == WeaponSize ? TEXT("● ") : TEXT("○ ");
					return FText::FromString(Prefix + GetWeaponSizeLabel(WeaponSize).ToString());
				})
				.OnClicked_Lambda([this, WeaponSize]()
				{
					ViewModel->SetRequiredWeaponSize(WeaponSize);
					RefreshView();
					return FReply::Handled();
				})
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("MountCompatTitle", "2. 장착 호환 규격"),
			LOCTEXT("MountCompatHelp", "EquipmentPreset 조립 전에 이 무장이 요구하는 Mount Type(장착 타입)과 Weapon Size(무기 크기)를 먼저 고정합니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)[BuildFieldRow(LOCTEXT("MountType", "장착 타입"), MountButtons)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)[BuildFieldRow(LOCTEXT("WeaponSize", "무기 크기"), SizeButtons)];
}

// TurretMountData create/reuse와 핵심 기하 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildMountGeometryPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("MountGeoTitle", "3. 장착 / 포탑 구조"),
			LOCTEXT("MountGeoHelp", "기존 TurretMountData를 재사용하거나 신규 Draft를 작성합니다. 기존 자산 재사용은 read-only이며 이 가이드가 수정·저장하지 않습니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Turret.Mode, ECFWeaponGuideChildMode::CreateNew); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Turret.Mode = ECFWeaponGuideChildMode::CreateNew;
						RefreshView();
						return FReply::Handled();
					})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Turret.Mode, ECFWeaponGuideChildMode::ReuseExisting); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Turret.Mode = ECFWeaponGuideChildMode::ReuseExisting;
						RefreshView();
						return FReply::Handled();
					})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Turret.Mode == ECFWeaponGuideChildMode::ReuseExisting ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					BuildFieldRow(
						LOCTEXT("TurretReuse", "기존 TurretMountData"),
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFTurretMountData::StaticClass())
							.ObjectPath(this, &SCFWeaponGuideTab::GetTurretObjectPath)
							.OnObjectChanged(this, &SCFWeaponGuideTab::HandleTurretChanged)
							.AllowClear(true))
				]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Turret.Mode == ECFWeaponGuideChildMode::CreateNew ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("MuzzleSocket", "기본 Muzzle Socket"),
							SNew(SEditableTextBox)
								.Text_Lambda([this]() { return FText::FromName(ViewModel->GetDraft().Turret.PrimaryMuzzleSocketName); })
								.OnTextChanged_Lambda([this](const FText& NewText)
								{
									ViewModel->GetMutableDraft().Turret.PrimaryMuzzleSocketName = FName(*NewText.ToString());
								}))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("MinYaw", "최소 Yaw (deg)"),
						[this]() { return ViewModel->GetDraft().Turret.MinYawDeg; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.MinYawDeg = Value; },
						-360.0f)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("MaxYaw", "최대 Yaw (deg)"),
						[this]() { return ViewModel->GetDraft().Turret.MaxYawDeg; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.MaxYawDeg = Value; },
						-360.0f)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("MinPitch", "최소 Pitch (deg)"),
						[this]() { return ViewModel->GetDraft().Turret.MinPitchDeg; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.MinPitchDeg = Value; },
						-180.0f)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("MaxPitch", "최대 Pitch (deg)"),
						[this]() { return ViewModel->GetDraft().Turret.MaxPitchDeg; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.MaxPitchDeg = Value; },
						-180.0f)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("YawRate", "Yaw 회전 속도 (deg/s)"),
						[this]() { return ViewModel->GetDraft().Turret.YawTurnRateDegPerSec; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.YawTurnRateDegPerSec = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("PitchRate", "Pitch 회전 속도 (deg/s)"),
						[this]() { return ViewModel->GetDraft().Turret.PitchTurnRateDegPerSec; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.PitchTurnRateDegPerSec = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("TurretWeight", "Turret 질량 (kg)"),
						[this]() { return ViewModel->GetDraft().Turret.TurretMountWeightKg; },
						[this](float Value) { ViewModel->GetMutableDraft().Turret.TurretMountWeightKg = Value; })]
				]
		];
}

// WeaponData create/reuse와 발사/탄약/선택 Capability Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildFireBehaviorPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("FireTitle", "4. 무기 발사 동작"),
			LOCTEXT("FireHelp", "기존 WeaponData를 통째로 재사용하거나 신규 Weapon Draft를 작성합니다. 유한 탄약·Launcher·Heat·Charge를 켜면 뒤의 조건부 Page가 자동으로 나타납니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Weapon.Mode, ECFWeaponGuideChildMode::CreateNew); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Weapon.Mode = ECFWeaponGuideChildMode::CreateNew;
						RefreshView();
						return FReply::Handled();
					})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Weapon.Mode, ECFWeaponGuideChildMode::ReuseExisting); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Weapon.Mode = ECFWeaponGuideChildMode::ReuseExisting;
						RefreshView();
						return FReply::Handled();
					})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Weapon.Mode == ECFWeaponGuideChildMode::ReuseExisting ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					BuildFieldRow(
						LOCTEXT("WeaponReuse", "기존 WeaponData"),
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFWeaponData::StaticClass())
							.ObjectPath(this, &SCFWeaponGuideTab::GetWeaponObjectPath)
							.OnObjectChanged(this, &SCFWeaponGuideTab::HandleWeaponChanged)
							.AllowClear(true))
				]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Weapon.Mode == ECFWeaponGuideChildMode::CreateNew ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("FireMode", "발사 방식"),
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
							[
								SNew(SButton)
									.Text_Lambda([this]() { return FText::FromString(ViewModel->GetDraft().Weapon.FireMode == ECFWeaponFireMode::Projectile ? TEXT("● Projectile") : TEXT("○ Projectile")); })
									.OnClicked_Lambda([this]()
									{
										ViewModel->GetMutableDraft().Weapon.FireMode = ECFWeaponFireMode::Projectile;
										ViewModel->NotifyDraftTopologyChanged();
										RefreshView();
										return FReply::Handled();
									})
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SButton)
									.Text_Lambda([this]() { return FText::FromString(ViewModel->GetDraft().Weapon.FireMode == ECFWeaponFireMode::HitScan ? TEXT("● HitScan") : TEXT("○ HitScan")); })
									.OnClicked_Lambda([this]()
									{
										ViewModel->GetMutableDraft().Weapon.FireMode = ECFWeaponFireMode::HitScan;
										ViewModel->NotifyDraftTopologyChanged();
										RefreshView();
										return FReply::Handled();
									})
							])
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("FireRate", "발사 속도 (RPM)"),
						[this]() { return ViewModel->GetDraft().Weapon.FireRatePerMinute; },
						[this](float Value) { ViewModel->GetMutableDraft().Weapon.FireRatePerMinute = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("Range", "유효 사거리"),
						[this]() { return ViewModel->GetDraft().Weapon.MaxRange; },
						[this](float Value) { ViewModel->GetMutableDraft().Weapon.MaxRange = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("Spread", "탄퍼짐 (deg)"),
						[this]() { return ViewModel->GetDraft().Weapon.SpreadDeg; },
						[this](float Value) { ViewModel->GetMutableDraft().Weapon.SpreadDeg = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("WeaponMass", "무기 본체 질량 (kg)"),
						[this]() { return ViewModel->GetDraft().Weapon.WeaponMassKg; },
						[this](float Value) { ViewModel->GetMutableDraft().Weapon.WeaponMassKg = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("FiniteAmmo", "유한 탄약 사용"),
						[this]() { return ViewModel->GetDraft().Weapon.bUseFiniteAmmo; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Weapon.bUseFiniteAmmo = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("UseLauncher", "Launcher 설정 사용"),
						[this]() { return ViewModel->GetDraft().Weapon.bUseLauncher; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Weapon.bUseLauncher = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("UseHeat", "Heat 사용"),
						[this]() { return ViewModel->GetDraft().Weapon.bUseHeat; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Weapon.bUseHeat = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("UseCharge", "Charge 사용"),
						[this]() { return ViewModel->GetDraft().Weapon.bUseCharge; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Weapon.bUseCharge = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						SNew(SBox)
							.Visibility_Lambda([this]() { return ViewModel->GetDraft().Weapon.bUseFiniteAmmo ? EVisibility::Visible : EVisibility::Collapsed; })
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()[BuildIntRow(
									LOCTEXT("Magazine", "탄창 용량"),
									[this]() { return ViewModel->GetDraft().Weapon.MagazineSize; },
									[this](int32 Value) { ViewModel->GetMutableDraft().Weapon.MagazineSize = Value; },
									1)]
								+ SVerticalBox::Slot().AutoHeight()[BuildIntRow(
									LOCTEXT("InitialAmmo", "초기 장전량"),
									[this]() { return ViewModel->GetDraft().Weapon.InitialLoadedAmmoCount; },
									[this](int32 Value) { ViewModel->GetMutableDraft().Weapon.InitialLoadedAmmoCount = Value; })]
								+ SVerticalBox::Slot().AutoHeight()[BuildIntRow(
									LOCTEXT("AmmoPerShot", "1회 소비량"),
									[this]() { return ViewModel->GetDraft().Weapon.AmmoUnitsPerShot; },
									[this](int32 Value) { ViewModel->GetMutableDraft().Weapon.AmmoUnitsPerShot = Value; },
									1)]
								+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
									LOCTEXT("Reload", "재장전 시간 (s)"),
									[this]() { return ViewModel->GetDraft().Weapon.ReloadTimeSeconds; },
									[this](float Value) { ViewModel->GetMutableDraft().Weapon.ReloadTimeSeconds = Value; })]
							]
					]
				]
		];
}

// ProjectileData create/reuse와 핵심 비행값 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildProjectilePage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("ProjectileTitle", "5. Projectile 데이터"),
			LOCTEXT("ProjectileHelp", "Projectile 발사 모드는 실제 Projectile Actor Class를 사용하고, HitScan은 가상 ProjectileData를 통해 DamageData 단일 소유 규칙을 유지합니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Projectile.Mode, ECFWeaponGuideChildMode::CreateNew); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Projectile.Mode = ECFWeaponGuideChildMode::CreateNew;
						RefreshView();
						return FReply::Handled();
					})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Projectile.Mode, ECFWeaponGuideChildMode::ReuseExisting); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Projectile.Mode = ECFWeaponGuideChildMode::ReuseExisting;
						RefreshView();
						return FReply::Handled();
					})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					BuildFieldRow(
						LOCTEXT("ProjectileReuse", "기존 ProjectileData"),
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFProjectileData::StaticClass())
							.ObjectPath(this, &SCFWeaponGuideTab::GetProjectileObjectPath)
							.OnObjectChanged(this, &SCFWeaponGuideTab::HandleProjectileChanged)
							.AllowClear(true))
				]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Projectile.Mode == ECFWeaponGuideChildMode::CreateNew ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						SNew(STextBlock)
							.Text_Lambda([this]()
							{
								return FText::FromString(FString::Printf(
									TEXT("Projectile Actor Class: %s"),
									*ViewModel->GetDraft().Projectile.ProjectileActorClassPath.ToString()));
							})
							.AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("InitialSpeed", "초기 속도"),
						[this]() { return ViewModel->GetDraft().Projectile.InitialSpeed; },
						[this](float Value) { ViewModel->GetMutableDraft().Projectile.InitialSpeed = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("Lifetime", "수명 (s)"),
						[this]() { return ViewModel->GetDraft().Projectile.LifeTimeSeconds; },
						[this](float Value) { ViewModel->GetMutableDraft().Projectile.LifeTimeSeconds = Value; },
						0.01f)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("CollisionRadius", "충돌 반경"),
						[this]() { return ViewModel->GetDraft().Projectile.CollisionRadius; },
						[this](float Value) { ViewModel->GetMutableDraft().Projectile.CollisionRadius = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("Gravity", "중력 영향"),
						[this]() { return ViewModel->GetDraft().Projectile.bAffectedByGravity; },
						[this](bool bValue) { ViewModel->GetMutableDraft().Projectile.bAffectedByGravity = bValue; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("Sweep", "Sweep 충돌 사용"),
						[this]() { return ViewModel->GetDraft().Projectile.bUseSweepCollision; },
						[this](bool bValue) { ViewModel->GetMutableDraft().Projectile.bUseSweepCollision = bValue; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("Propulsion", "자체 추진 사용"),
						[this]() { return ViewModel->GetDraft().Projectile.PropulsionConfig.bUsePropulsion; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Projectile.PropulsionConfig.bUsePropulsion = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("MissileFlight", "Missile Flight 사용"),
						[this]() { return ViewModel->GetDraft().Projectile.MissileFlightConfig.bUseMissileFlight; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Projectile.MissileFlightConfig.bUseMissileFlight = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("Guidance", "Guidance 사용"),
						[this]() { return ViewModel->GetDraft().Projectile.MissileGuideConfig.bUseGuidance; },
						[this](bool bValue)
						{
							ViewModel->GetMutableDraft().Projectile.MissileGuideConfig.bUseGuidance = bValue;
							ViewModel->NotifyDraftTopologyChanged();
							RefreshView();
						})]
				]
		];
}

// DamageData create/reuse와 existing provider exact12 Draft Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildDamagePage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("DamageTitle", "6. Damage 데이터"),
			LOCTEXT("DamageHelp", "기존 DamageData를 선택하거나 신규 Damage Draft를 작성합니다. 신규 Draft는 CFDADamageProvider의 exact typed payload/fingerprint/serializer 계약으로 검토되며 P0-02에서는 저장하지 않습니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Damage.Mode, ECFWeaponGuideChildMode::CreateNew); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Damage.Mode = ECFWeaponGuideChildMode::CreateNew;
						RefreshView();
						return FReply::Handled();
					})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Damage.Mode, ECFWeaponGuideChildMode::ReuseExisting); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Damage.Mode = ECFWeaponGuideChildMode::ReuseExisting;
						RefreshView();
						return FReply::Handled();
					})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Damage.Mode == ECFWeaponGuideChildMode::ReuseExisting ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					BuildFieldRow(
						LOCTEXT("DamageReuse", "기존 DamageData"),
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFDamageData::StaticClass())
							.ObjectPath(this, &SCFWeaponGuideTab::GetDamageObjectPath)
							.OnObjectChanged(this, &SCFWeaponGuideTab::HandleDamageChanged)
							.AllowClear(true))
				]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Damage.Mode == ECFWeaponGuideChildMode::CreateNew ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("DamageType", "피해 종류"),
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
							[
								SNew(SButton).Text(LOCTEXT("Kinetic", "Kinetic"))
								.OnClicked_Lambda([this]()
								{
									ViewModel->GetMutableDraft().Damage.DamageType = ECFDamageType::Kinetic;
									return FReply::Handled();
								})
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
							[
								SNew(SButton).Text(LOCTEXT("Explosive", "Explosive"))
								.OnClicked_Lambda([this]()
								{
									ViewModel->GetMutableDraft().Damage.DamageType = ECFDamageType::Explosive;
									return FReply::Handled();
								})
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SButton).Text(LOCTEXT("Energy", "Energy"))
								.OnClicked_Lambda([this]()
								{
									ViewModel->GetMutableDraft().Damage.DamageType = ECFDamageType::Energy;
									return FReply::Handled();
								})
							])
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("BaseDamage", "기본 피해량"),
						[this]() { return ViewModel->GetDraft().Damage.BaseDamage; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.BaseDamage = Value; },
						0.01f)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("ArmorPen", "장갑 관통"),
						[this]() { return ViewModel->GetDraft().Damage.ArmorPenetration; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.ArmorPenetration = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("SelfDamage", "자기 피해 허용"),
						[this]() { return ViewModel->GetDraft().Damage.bCanDamageSelf; },
						[this](bool bValue) { ViewModel->GetMutableDraft().Damage.bCanDamageSelf = bValue; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("Radial", "범위 피해 사용"),
						[this]() { return ViewModel->GetDraft().Damage.bUseRadialDamage; },
						[this](bool bValue) { ViewModel->GetMutableDraft().Damage.bUseRadialDamage = bValue; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("ExplosionRadius", "폭발 반경"),
						[this]() { return ViewModel->GetDraft().Damage.ExplosionRadius; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.ExplosionRadius = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("ExplosionInnerRadius", "폭발 최대 피해 반경"),
						[this]() { return ViewModel->GetDraft().Damage.ExplosionInnerRadius; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.ExplosionInnerRadius = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("ExplosionDamage", "폭발 피해량"),
						[this]() { return ViewModel->GetDraft().Damage.ExplosionDamage; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.ExplosionDamage = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("MinExplosionScale", "최소 폭발 피해 배율"),
						[this]() { return ViewModel->GetDraft().Damage.MinExplosionDamageScale; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.MinExplosionDamageScale = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("ModuleDamageScale", "모듈 피해 배율"),
						[this]() { return ViewModel->GetDraft().Damage.ModuleDamageScale; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.ModuleDamageScale = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("Impulse", "충격량"),
						[this]() { return ViewModel->GetDraft().Damage.ImpulseStrength; },
						[this](float Value) { ViewModel->GetMutableDraft().Damage.ImpulseStrength = Value; })]
				]
		];
}

// AmmoData create/reuse와 existing provider exact8 Draft Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildAmmoPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("AmmoTitle", "7. Ammo 데이터"),
			LOCTEXT("AmmoHelp", "유한 탄약 무기에서 기존 AmmoData를 재사용하거나 신규 Draft를 작성합니다. 신규 Draft는 CFDAAmmoProvider exact8 계약으로 mutation0 검토됩니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 5.0f, 0.0f)
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Ammo.Mode, ECFWeaponGuideChildMode::CreateNew); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Ammo.Mode = ECFWeaponGuideChildMode::CreateNew;
						if (ViewModel->GetDraft().Ammo.AmmoDisplayName.IsEmpty())
						{
							ViewModel->GetMutableDraft().Ammo.AmmoDisplayName = ViewModel->GetDraft().SuggestedDisplayName + TEXT(" 탄약");
						}
						RefreshView();
						return FReply::Handled();
					})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text_Lambda([this]() { return GetChildModeLabel(ViewModel->GetDraft().Ammo.Mode, ECFWeaponGuideChildMode::ReuseExisting); })
					.OnClicked_Lambda([this]()
					{
						ViewModel->GetMutableDraft().Ammo.Mode = ECFWeaponGuideChildMode::ReuseExisting;
						RefreshView();
						return FReply::Handled();
					})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Ammo.Mode == ECFWeaponGuideChildMode::ReuseExisting ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					BuildFieldRow(
						LOCTEXT("AmmoReuse", "기존 AmmoData"),
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFAmmoData::StaticClass())
							.ObjectPath(this, &SCFWeaponGuideTab::GetAmmoObjectPath)
							.OnObjectChanged(this, &SCFWeaponGuideTab::HandleAmmoChanged)
							.AllowClear(true))
				]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
		[
			SNew(SBox)
				.Visibility_Lambda([this]() { return ViewModel->GetDraft().Ammo.Mode == ECFWeaponGuideChildMode::CreateNew ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("AmmoDisplay", "게임 표시 이름"),
							SNew(SEditableTextBox)
								.Text_Lambda([this]() { return FText::FromString(ViewModel->GetDraft().Ammo.AmmoDisplayName); })
								.OnTextChanged_Lambda([this](const FText& NewText)
								{
									ViewModel->GetMutableDraft().Ammo.AmmoDisplayName = NewText.ToString();
								}))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("AmmoFamily", "탄약 계열 ID"),
							SNew(SEditableTextBox)
								.Text_Lambda([this]() { return FText::FromName(ViewModel->GetDraft().Ammo.AmmoFamilyId); })
								.OnTextChanged_Lambda([this](const FText& NewText)
								{
									ViewModel->GetMutableDraft().Ammo.AmmoFamilyId = FName(*NewText.ToString());
								}))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildFloatRow(
						LOCTEXT("UnitMass", "탄약 1단위 질량 (kg)"),
						[this]() { return ViewModel->GetDraft().Ammo.UnitMassKg; },
						[this](float Value) { ViewModel->GetMutableDraft().Ammo.UnitMassKg = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("AmmoTags", "분류 태그 (쉼표 구분)"),
							SNew(SEditableTextBox)
								.Text_Lambda([this]()
								{
									// 현재 AmmoTags를 사용자 편집용 문자열로 변환합니다.
									TArray<FString> Tags;
									for (const FName Tag : ViewModel->GetDraft().Ammo.AmmoTags)
									{
										Tags.Add(Tag.ToString());
									}
									return FText::FromString(FString::Join(Tags, TEXT(", ")));
								})
								.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
								{
									// 쉼표 기반 사용자 입력 token 목록입니다.
									TArray<FString> TagTokens;
									NewText.ToString().ParseIntoArray(TagTokens, TEXT(","), true);
									// provider set-like 의미로 저장할 새로운 FName 목록입니다.
									TArray<FName> NewTags;
									for (FString& Token : TagTokens)
									{
										Token.TrimStartAndEndInline();
										if (!Token.IsEmpty())
										{
											NewTags.Add(FName(*Token));
										}
									}
									ViewModel->GetMutableDraft().Ammo.AmmoTags = MoveTemp(NewTags);
								}))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
					[
						BuildFieldRow(
							LOCTEXT("AmmoIcon", "탄약 아이콘 (Texture2D)"),
							SNew(SObjectPropertyEntryBox)
								.AllowedClass(UTexture2D::StaticClass())
								.ObjectPath(this, &SCFWeaponGuideTab::GetAmmoIconObjectPath)
								.OnObjectChanged(this, &SCFWeaponGuideTab::HandleAmmoIconChanged)
								.AllowClear(true))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildIntRow(
						LOCTEXT("MaxLoad", "최대 적재량"),
						[this]() { return ViewModel->GetDraft().Ammo.MaximumLoadableAmmoCount; },
						[this](int32 Value) { ViewModel->GetMutableDraft().Ammo.MaximumLoadableAmmoCount = Value; })]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)[BuildCheckRow(
						LOCTEXT("Resupply", "재보급 가능"),
						[this]() { return ViewModel->GetDraft().Ammo.bCanBeResupplied; },
						[this](bool bValue) { ViewModel->GetMutableDraft().Ammo.bCanBeResupplied = bValue; })]
				]
		];
}

// Launcher conditional 설정 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildLauncherPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("LauncherTitle", "Launcher 설정"),
			LOCTEXT("LauncherHelp", "다중 Muzzle 발사 패턴과 Projectile 분리 속도를 설정합니다."))]
		+ SVerticalBox::Slot().AutoHeight()[BuildIntRow(
			LOCTEXT("ProjectileCount", "Trigger당 발사 수"),
			[this]() { return ViewModel->GetDraft().Weapon.LauncherFirePatternConfig.ProjectileCountPerTrigger; },
			[this](int32 Value) { ViewModel->GetMutableDraft().Weapon.LauncherFirePatternConfig.ProjectileCountPerTrigger = Value; },
			1)]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("InterMuzzleDelay", "Muzzle 간 지연 (s)"),
			[this]() { return ViewModel->GetDraft().Weapon.LauncherFirePatternConfig.InterMuzzleDelaySeconds; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.LauncherFirePatternConfig.InterMuzzleDelaySeconds = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildIntRow(
			LOCTEXT("MaxSimultaneous", "최대 동시 발사 수"),
			[this]() { return ViewModel->GetDraft().Weapon.LauncherFirePatternConfig.MaximumSimultaneousLaunchCount; },
			[this](int32 Value) { ViewModel->GetMutableDraft().Weapon.LauncherFirePatternConfig.MaximumSimultaneousLaunchCount = Value; },
			1)]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("EjectionSpeed", "분리 속도"),
			[this]() { return ViewModel->GetDraft().Weapon.LauncherReleaseConfig.EjectionSpeed; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.LauncherReleaseConfig.EjectionSpeed = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("CarrierRatio", "차량 속도 상속 비율"),
			[this]() { return ViewModel->GetDraft().Weapon.LauncherReleaseConfig.CarrierVelocityRatio; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.LauncherReleaseConfig.CarrierVelocityRatio = Value; })];
}

// Propulsion conditional 설정 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildPropulsionPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("PropulsionTitle", "Propulsion / 자체 추진"),
			LOCTEXT("PropulsionHelp", "Rocket/Missile의 점화 지연, 연소 시간, 가속도와 최대 추진 속도를 설정합니다."))]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("IgnitionDelay", "점화 지연 (s)"),
			[this]() { return ViewModel->GetDraft().Projectile.PropulsionConfig.IgnitionDelaySeconds; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.PropulsionConfig.IgnitionDelaySeconds = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("BurnDuration", "연소 시간 (s)"),
			[this]() { return ViewModel->GetDraft().Projectile.PropulsionConfig.BurnDurationSeconds; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.PropulsionConfig.BurnDurationSeconds = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("ThrustAccel", "추진 가속도"),
			[this]() { return ViewModel->GetDraft().Projectile.PropulsionConfig.ThrustAccelerationCmPerSecSq; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.PropulsionConfig.ThrustAccelerationCmPerSecSq = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("MaxPropelledSpeed", "최대 추진 속도"),
			[this]() { return ViewModel->GetDraft().Projectile.PropulsionConfig.MaximumPropelledSpeed; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.PropulsionConfig.MaximumPropelledSpeed = Value; })];
}

// Missile Flight conditional 설정 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildMissileFlightPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("MissileFlightTitle", "Missile Flight / 비행 단계"),
			LOCTEXT("MissileFlightHelp", "발사 직후 clearance와 transition, terminal 전환 거리를 설정합니다. Attack Profile은 선택 Template의 제안값을 유지합니다."))]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("ClearanceTime", "최소 Clearance 시간 (s)"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileFlightConfig.MinimumClearanceTimeSeconds; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileFlightConfig.MinimumClearanceTimeSeconds = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("ClearanceDistance", "최소 Clearance 거리"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileFlightConfig.MinimumClearanceDistanceCm; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileFlightConfig.MinimumClearanceDistanceCm = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("TransitionDuration", "Transition 시간 (s)"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileFlightConfig.TransitionDurationSeconds; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileFlightConfig.TransitionDurationSeconds = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("TerminalDistance", "Terminal 시작 거리"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileFlightConfig.TerminalPhaseStartDistanceCm; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileFlightConfig.TerminalPhaseStartDistanceCm = Value; })];
}

// Guidance conditional 설정 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildGuidancePage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("GuidanceTitle", "Guidance / 유도"),
			LOCTEXT("GuidanceHelp", "TargetActor 등 유도 정보 출처의 물리 반응 한계를 설정합니다. GuideMode/Law 기본값은 Template 제안을 유지하고 세부 확장은 후속 Advanced UX에서 다룹니다."))]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("TurnRate", "최대 선회율 (deg/s)"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileGuideConfig.MaximumTurnRateDegPerSec; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileGuideConfig.MaximumTurnRateDegPerSec = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("LateralAccel", "최대 횡가속도"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("ResponseTime", "유도 응답 시간 (s)"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileGuideConfig.GuidanceResponseTimeSeconds; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileGuideConfig.GuidanceResponseTimeSeconds = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("MinGuideSpeed", "최소 유도 속도"),
			[this]() { return ViewModel->GetDraft().Projectile.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec; },
			[this](float Value) { ViewModel->GetMutableDraft().Projectile.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = Value; })];
}

// Heat conditional 설정 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildHeatPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("HeatTitle", "Heat / 열 관리"),
			LOCTEXT("HeatHelp", "발사 열 누적과 최대 열, 초당 냉각량을 설정합니다."))]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("HeatPerShot", "발사당 열량"),
			[this]() { return ViewModel->GetDraft().Weapon.HeatPerShot; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.HeatPerShot = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("MaxHeat", "최대 열량"),
			[this]() { return ViewModel->GetDraft().Weapon.MaxHeat; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.MaxHeat = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("HeatDissipation", "초당 냉각량"),
			[this]() { return ViewModel->GetDraft().Weapon.HeatDissipationPerSecond; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.HeatDissipationPerSecond = Value; })];
}

// Charge conditional 설정 Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildChargePage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("ChargeTitle", "Charge / 에너지 충전"),
			LOCTEXT("ChargeHelp", "최대 Charge, 초기값, 발사당 소비와 초당 회복량을 설정합니다."))]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("MaxCharge", "최대 Charge"),
			[this]() { return ViewModel->GetDraft().Weapon.MaximumWeaponCharge; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.MaximumWeaponCharge = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("InitialCharge", "초기 Charge"),
			[this]() { return ViewModel->GetDraft().Weapon.InitialWeaponCharge; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.InitialWeaponCharge = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("ChargePerShot", "발사당 소비 Charge"),
			[this]() { return ViewModel->GetDraft().Weapon.WeaponChargePerShot; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.WeaponChargePerShot = Value; })]
		+ SVerticalBox::Slot().AutoHeight()[BuildFloatRow(
			LOCTEXT("ChargeRecovery", "초당 Charge 회복"),
			[this]() { return ViewModel->GetDraft().Weapon.WeaponChargeRecoveryPerSecond; },
			[this](float Value) { ViewModel->GetMutableDraft().Weapon.WeaponChargeRecoveryPerSecond = Value; })];
}

// 전체 Draft validation과 provider mutation0 preview Page를 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildReviewPage()
{
	// 전체 Draft validation 사용자 요약입니다.
	const FString ValidationSummary = ViewModel->BuildValidationSummary();

	// 신규 Damage provider preview 결과입니다.
	FString DamageProviderSummary = TEXT("DamageData: 기존 persisted 자산 재사용");
	if (ViewModel->GetDraft().Damage.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		// 신규 Damage provider preview 실패 상세입니다.
		FString DamageProviderError;
		if (!ViewModel->BuildDamageProviderPreview(DamageProviderSummary, DamageProviderError))
		{
			DamageProviderSummary = TEXT("Damage provider 검토 실패: ") + DamageProviderError;
		}
	}

	// 신규 Ammo provider preview 결과입니다.
	FString AmmoProviderSummary = ViewModel->GetDraft().Weapon.bUseFiniteAmmo
		? TEXT("AmmoData: 기존 persisted 자산 재사용")
		: TEXT("AmmoData: 무한 탄약 구성으로 생략");
	if (ViewModel->GetDraft().Weapon.bUseFiniteAmmo && ViewModel->GetDraft().Ammo.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		// 신규 Ammo provider preview 실패 상세입니다.
		FString AmmoProviderError;
		if (!ViewModel->BuildAmmoProviderPreview(AmmoProviderSummary, AmmoProviderError))
		{
			AmmoProviderSummary = TEXT("Ammo provider 검토 실패: ") + AmmoProviderError;
		}
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("ReviewTitle", "최종 검토 / 생성 — WEA-P0-03"),
			LOCTEXT("ReviewHelp", "현재 Draft와 provider/current truth를 fresh 검증합니다. [무장 생성]을 누르기 전에는 mutation 0이며, 누른 뒤에는 Damage → Ammo → Turret → Projectile → Weapon 순으로 durable 저장합니다. 중간 실패 시 이미 저장된 child를 자동 삭제하지 않습니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBorder).Padding(8.0f)
			[
				SNew(STextBlock).Text(FText::FromString(ValidationSummary)).AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBorder).Padding(8.0f)
			[
				SNew(STextBlock).Text(FText::FromString(DamageProviderSummary)).AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBorder).Padding(8.0f)
			[
				SNew(STextBlock).Text(FText::FromString(AmmoProviderSummary)).AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("P003Boundary", "실행 전 확인: [무장 생성]은 새 child DataAsset을 실제 저장합니다. 실패한 경우 상태 메시지에 이미 생성 완료된 child와 재시도 경계를 표시하며 자동 rollback/delete는 수행하지 않습니다. EquipmentPreset과 CF-FQ-054는 변경하지 않습니다."))
				.AutoWrapText(true)
		];
}

// P0-04 completion USER summary와 navigation-only handoff Action을 구성합니다.
TSharedRef<SWidget> SCFWeaponGuideTab::BuildCompletePage()
{
	// P0-03에서 확정된 completion Bundle입니다.
	const FCFWeaponGuideResultBundle& Bundle = ViewModel->GetResultBundle();

	// Bundle fingerprint와 persisted graph를 다시 확인한 사용자 중심 완료 요약입니다.
	FString UserSummary;
	// 완료 요약 생성 실패 상세입니다.
	FString SummaryError;
	const bool bSummaryValid = ViewModel->BuildCompletionUserSummary(UserSummary, SummaryError);
	bCompletionHandoffReady = bSummaryValid;
	if (!bSummaryValid)
	{
		UserSummary = TEXT("완료 결과를 다시 확인하지 못했습니다. 장비 제작 가이드로 이동하지 말고 현재 상태를 재검증하세요.\n") + SummaryError;
	}

	// Advanced 성격의 exact child path/fingerprint 기술 세부 정보입니다. integrity 실패 시 stale 확정값처럼 보이지 않도록 숨깁니다.
	const FString TechnicalSummary = bSummaryValid
		? FString::Printf(
			TEXT("[기술 세부 정보]\nTurretMountData: %s\nDamageData: %s\nAmmoData: %s\nProjectileData: %s\nWeaponData: %s\nDeterministicBundleFingerprint: %s"),
			*Bundle.TurretMountDataPath.ToString(),
			*Bundle.DamageDataPath.ToString(),
			Bundle.AmmoDataPath.IsNull() ? TEXT("(없음 / AmmoTypeId fallback 사용)") : *Bundle.AmmoDataPath.ToString(),
			*Bundle.ProjectileDataPath.ToString(),
			*Bundle.WeaponDataPath.ToString(),
			*Bundle.DeterministicBundleFingerprint)
		: TEXT("[기술 세부 정보]\n현재 completion integrity 검증이 실패하여 이전 exact path/fingerprint 표시를 숨겼습니다.");

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildPageHeader(
			LOCTEXT("CompleteTitle", "무장 제작 완료"),
			LOCTEXT("CompleteHelp", "WEA-P0-03 durable exact5 graph와 DeterministicBundleFingerprint를 보존한 완료 결과입니다. EquipmentPreset은 아직 생성/수정하지 않았고 CF-FQ-054 Equipment Builder는 Paused 상태입니다."))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f)
		[
			SNew(SBorder).Padding(10.0f)
			[
				SNew(STextBlock).Text(FText::FromString(UserSummary)).AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBorder).Padding(8.0f)
			[
				SNew(STextBlock).Text(FText::FromString(TechnicalSummary)).AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 4.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("OpenEquipmentBuilder", "장비 제작 가이드 열기 (이동만)"))
				.ToolTipText(LOCTEXT("OpenEquipmentBuilderTip", "CarFight 장비 제작 가이드 탭만 엽니다. 완료 Bundle, child reference, DisplayName을 자동 입력하지 않으며 CF-FQ-054 lifecycle을 재개하지 않습니다."))
				.IsEnabled(this, &SCFWeaponGuideTab::CanOpenEquipmentBuilder)
				.OnClicked(this, &SCFWeaponGuideTab::HandleOpenEquipmentBuilder)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("NavigationOnlyNotice", "Navigation-only 계약: 버튼은 Equipment Builder 탭을 여는 것만 수행합니다. 정확한 context injection은 CF-FQ-054를 USER가 명시적으로 재개한 뒤 별도 Handoff Contract에서 다룹니다."))
				.AutoWrapText(true)
		];
}

// 현재 ViewModel 상태를 반영해 Step rail과 Page host를 재구성합니다.
void SCFWeaponGuideTab::RefreshView()
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	if (StepRailHost.IsValid())
	{
		StepRailHost->SetContent(BuildStepRail());
	}
	if (PageHost.IsValid())
	{
		PageHost->SetContent(BuildCurrentPage());
	}
	if (PageScrollBox.IsValid())
	{
		PageScrollBox->ScrollToStart();
	}
}

// 이전 visible Step으로 이동합니다.
FReply SCFWeaponGuideTab::HandlePrevious()
{
	if (ViewModel.IsValid() && ViewModel->MovePrevious())
	{
		UiStatusMessage.Reset();
		RefreshView();
	}
	return FReply::Handled();
}

// 현재 Step을 검증하고 다음 visible Step으로 이동하거나 Review에서 explicit durable Create를 실행합니다.
FReply SCFWeaponGuideTab::HandleNext()
{
	// 다음 Step 이동 또는 durable Create 실패 상세입니다.
	FString Error;
	if (!ViewModel.IsValid())
	{
		UiStatusMessage = TEXT("Weapon Guide ViewModel이 준비되지 않았습니다.");
		return FReply::Handled();
	}

	if (ViewModel->GetCurrentStep() == ECFWeaponGuideStep::ReviewCreate)
	{
		if (ViewModel->CreateChildrenAndBuildResult(Error))
		{
			UiStatusMessage.Reset();
			RefreshView();
		}
		else
		{
			UiStatusMessage = Error;
			RefreshView();
		}
		return FReply::Handled();
	}

	if (ViewModel->MoveNext(Error))
	{
		UiStatusMessage.Reset();
		RefreshView();
	}
	else
	{
		UiStatusMessage = Error;
	}
	return FReply::Handled();
}

// 현재 Step만 fresh 검증해 사용자 상태 메시지를 갱신합니다.
FReply SCFWeaponGuideTab::HandleValidateCurrent()
{
	// 현재 Step 검증 진단 목록입니다.
	TArray<FCFWeaponGuideIssue> Issues;
	const bool bValid = ViewModel.IsValid() && ViewModel->ValidateStep(ViewModel->GetCurrentStep(), Issues);
	if (bValid)
	{
		UiStatusMessage = TEXT("현재 단계 검증 PASS");
	}
	else if (!Issues.IsEmpty())
	{
		UiStatusMessage = TEXT("현재 단계 검증 실패: ") + Issues[0].Message;
	}
	else
	{
		UiStatusMessage = TEXT("현재 단계 검증에 실패했습니다.");
	}
	RefreshView();
	return FReply::Handled();
}

// 모든 transient Draft를 새 세션 기본값으로 되돌립니다.
FReply SCFWeaponGuideTab::HandleResetDraft()
{
	ViewModel->Reset();
	bCompletionHandoffReady = false;
	UiStatusMessage = TEXT("새 transient 무장 초안을 시작했습니다. 저장된 Product Asset은 변경하지 않았습니다.");
	RefreshView();
	return FReply::Handled();
}

// completion Bundle을 다시 검증한 뒤 Equipment Builder Nomad Tab만 열고 context는 전달하지 않습니다.
FReply SCFWeaponGuideTab::HandleOpenEquipmentBuilder()
{
	if (!ViewModel.IsValid())
	{
		UiStatusMessage = TEXT("Weapon Guide ViewModel이 준비되지 않아 장비 제작 가이드를 열 수 없습니다.");
		return FReply::Handled();
	}

	// P0-04 navigation-only handoff 계약입니다.
	FCFWeaponGuideEquipmentHandoff Handoff;
	// Handoff 무결성 검증 실패 상세입니다.
	FString HandoffError;
	if (!ViewModel->BuildEquipmentBuilderHandoff(Handoff, HandoffError))
	{
		UiStatusMessage = TEXT("장비 제작 가이드 이동을 차단했습니다. ") + HandoffError;
		RefreshView();
		return FReply::Handled();
	}
	if (!Handoff.bNavigationOnly || Handoff.bInjectContext)
	{
		UiStatusMessage = TEXT("WEA-P0-04 navigation-only 계약이 깨져 장비 제작 가이드 이동을 차단했습니다.");
		return FReply::Handled();
	}

	// 등록된 Equipment Builder 탭만 열며 Bundle/Draft/context는 전달하지 않습니다.
	const TSharedPtr<SDockTab> EquipmentBuilderTab = FGlobalTabmanager::Get()->TryInvokeTab(Handoff.TargetTabId);
	if (!EquipmentBuilderTab.IsValid())
	{
		UiStatusMessage = TEXT("장비 제작 가이드 탭을 열지 못했습니다. 완료 Bundle은 변경하지 않았습니다.");
		return FReply::Handled();
	}

	UiStatusMessage = TEXT("장비 제작 가이드 탭을 열었습니다. 값은 자동 전달하지 않았으며 CF-FQ-054 lifecycle도 변경하지 않았습니다.");
	return FReply::Handled();
}

// Complete Page 진입 시 1회 검증한 lightweight cache만 읽습니다. 클릭 직전 HandleOpenEquipmentBuilder가 authoritative integrity를 다시 검증합니다.
bool SCFWeaponGuideTab::CanOpenEquipmentBuilder() const
{
	return ViewModel.IsValid() && bCompletionHandoffReady;
}

// Existing TurretMountData picker 선택을 ViewModel에 반영합니다.
void SCFWeaponGuideTab::HandleTurretChanged(const FAssetData& AssetData)
{
	// picker에서 선택한 exact object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	// ViewModel selection validation 실패 상세입니다.
	FString Error;
	if (!SelectedPath.IsNull() && !ViewModel->SetExistingTurretPath(SelectedPath, Error))
	{
		UiStatusMessage = Error;
	}
	else
	{
		UiStatusMessage = SelectedPath.IsNull() ? TEXT("기존 TurretMountData 선택을 비웠습니다.") : TEXT("기존 TurretMountData를 read-only 재사용 대상으로 선택했습니다.");
		if (SelectedPath.IsNull())
		{
			ViewModel->GetMutableDraft().Turret.ExistingAssetPath.Reset();
		}
	}
	RefreshView();
}

// Existing WeaponData picker 선택을 ViewModel에 반영합니다.
void SCFWeaponGuideTab::HandleWeaponChanged(const FAssetData& AssetData)
{
	// picker에서 선택한 exact object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	// ViewModel selection validation 실패 상세입니다.
	FString Error;
	if (!SelectedPath.IsNull() && !ViewModel->SetExistingWeaponPath(SelectedPath, Error))
	{
		UiStatusMessage = Error;
	}
	else
	{
		UiStatusMessage = SelectedPath.IsNull() ? TEXT("기존 WeaponData 선택을 비웠습니다.") : TEXT("기존 WeaponData와 연결 child reference를 read-only 상태로 불러왔습니다.");
		if (SelectedPath.IsNull())
		{
			ViewModel->GetMutableDraft().Weapon.ExistingAssetPath.Reset();
		}
	}
	RefreshView();
}

// Existing ProjectileData picker 선택을 ViewModel에 반영합니다.
void SCFWeaponGuideTab::HandleProjectileChanged(const FAssetData& AssetData)
{
	// picker에서 선택한 exact object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	// ViewModel selection validation 실패 상세입니다.
	FString Error;
	if (!SelectedPath.IsNull() && !ViewModel->SetExistingProjectilePath(SelectedPath, Error))
	{
		UiStatusMessage = Error;
	}
	else
	{
		UiStatusMessage = SelectedPath.IsNull() ? TEXT("기존 ProjectileData 선택을 비웠습니다.") : TEXT("기존 ProjectileData와 연결 DamageData를 read-only 상태로 불러왔습니다.");
		if (SelectedPath.IsNull())
		{
			ViewModel->GetMutableDraft().Projectile.ExistingAssetPath.Reset();
		}
	}
	RefreshView();
}

// Existing DamageData picker 선택을 ViewModel에 반영합니다.
void SCFWeaponGuideTab::HandleDamageChanged(const FAssetData& AssetData)
{
	// picker에서 선택한 exact object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	// ViewModel selection validation 실패 상세입니다.
	FString Error;
	if (!SelectedPath.IsNull() && !ViewModel->SetDamageDataPath(SelectedPath, Error))
	{
		UiStatusMessage = Error;
	}
	else
	{
		UiStatusMessage = SelectedPath.IsNull() ? TEXT("기존 DamageData 선택을 비웠습니다.") : TEXT("기존 DamageData를 read-only 재사용 대상으로 선택했습니다.");
		if (SelectedPath.IsNull())
		{
			ViewModel->GetMutableDraft().Damage.ExistingAssetPath.Reset();
		}
	}
	RefreshView();
}

// Existing AmmoData picker 선택을 ViewModel에 반영합니다.
void SCFWeaponGuideTab::HandleAmmoChanged(const FAssetData& AssetData)
{
	// picker에서 선택한 exact object path입니다.
	const FSoftObjectPath SelectedPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	// ViewModel selection validation 실패 상세입니다.
	FString Error;
	if (!SelectedPath.IsNull() && !ViewModel->SetAmmoDataPath(SelectedPath, Error))
	{
		UiStatusMessage = Error;
	}
	else
	{
		UiStatusMessage = SelectedPath.IsNull() ? TEXT("기존 AmmoData 선택을 비웠습니다.") : TEXT("기존 AmmoData를 read-only 재사용 대상으로 선택했습니다.");
		if (SelectedPath.IsNull())
		{
			ViewModel->GetMutableDraft().Ammo.ExistingAssetPath.Reset();
		}
	}
	RefreshView();
}

// Turret picker가 표시할 exact object path를 반환합니다.
FString SCFWeaponGuideTab::GetTurretObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetDraft().Turret.ExistingAssetPath.ToString() : FString();
}

// Weapon picker가 표시할 exact object path를 반환합니다.
FString SCFWeaponGuideTab::GetWeaponObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetDraft().Weapon.ExistingAssetPath.ToString() : FString();
}

// Projectile picker가 표시할 exact object path를 반환합니다.
FString SCFWeaponGuideTab::GetProjectileObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetDraft().Projectile.ExistingAssetPath.ToString() : FString();
}

// Damage picker가 표시할 exact object path를 반환합니다.
FString SCFWeaponGuideTab::GetDamageObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetDraft().Damage.ExistingAssetPath.ToString() : FString();
}

// Ammo picker가 표시할 exact object path를 반환합니다.
FString SCFWeaponGuideTab::GetAmmoObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetDraft().Ammo.ExistingAssetPath.ToString() : FString();
}

// Ammo Icon picker가 표시할 Texture2D object path를 반환합니다.
FString SCFWeaponGuideTab::GetAmmoIconObjectPath() const
{
	return ViewModel.IsValid() ? ViewModel->GetDraft().Ammo.AmmoIconPath.ToString() : FString();
}

// Ammo Icon picker 선택을 신규 Ammo Draft에 반영합니다.
void SCFWeaponGuideTab::HandleAmmoIconChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	// picker에서 선택한 Texture2D exact object path입니다.
	const FSoftObjectPath SelectedIconPath = AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath();
	ViewModel->GetMutableDraft().Ammo.AmmoIconPath = SelectedIconPath;
	UiStatusMessage = SelectedIconPath.IsNull()
		? TEXT("Ammo Icon을 비웠습니다.")
		: TEXT("Ammo Icon을 신규 Ammo Draft에 연결했습니다.");
}

// 현재 Step과 visible index를 사용자용 문자열로 반환합니다.
FText SCFWeaponGuideTab::GetCurrentStepText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("NoViewModel", "현재 단계: 준비 중");
	}
	// Current Draft visible Step projection입니다.
	const TArray<ECFWeaponGuideStep> VisibleSteps = ViewModel->GetVisibleSteps();
	// 사용자용 1-based current Step index입니다.
	const int32 CurrentIndex = VisibleSteps.IndexOfByKey(ViewModel->GetCurrentStep()) + 1;
	return FText::FromString(FString::Printf(
		TEXT("현재 단계 %d / %d — %s"),
		CurrentIndex,
		VisibleSteps.Num(),
		*FCFWeaponGuideVM::GetStepDisplayName(ViewModel->GetCurrentStep())));
}

// ViewModel 또는 UI action의 마지막 사용자 상태를 반환합니다.
FText SCFWeaponGuideTab::GetStatusText() const
{
	if (!UiStatusMessage.IsEmpty())
	{
		return FText::FromString(UiStatusMessage);
	}
	return ViewModel.IsValid() ? FText::FromString(ViewModel->GetLastStatusMessage()) : FText::GetEmpty();
}

// 현재 Step이 P0-03 UI에서 다음 또는 explicit Create를 실행할 수 있는지 반환합니다.
bool SCFWeaponGuideTab::CanUseNext() const
{
	if (!ViewModel.IsValid())
	{
		return false;
	}
	if (ViewModel->GetCurrentStep() == ECFWeaponGuideStep::Complete)
	{
		return false;
	}
	// current Step validation 실패 상세은 버튼 enabled 판정에서 사용자에게 노출하지 않습니다.
	FString Error;
	return ViewModel->CanMoveNext(Error);
}

// Child mode 버튼의 current 선택 상태를 포함한 label을 반환합니다.
FText SCFWeaponGuideTab::GetChildModeLabel(const ECFWeaponGuideChildMode CurrentMode, const ECFWeaponGuideChildMode ButtonMode)
{
	// 선택 상태를 표현하는 prefix입니다.
	const FString Prefix = CurrentMode == ButtonMode ? TEXT("● ") : TEXT("○ ");
	// 사용자에게 보여줄 mode label입니다.
	const FString ModeLabel = ButtonMode == ECFWeaponGuideChildMode::CreateNew ? TEXT("새로 만들기") : TEXT("기존 데이터 재사용");
	return FText::FromString(Prefix + ModeLabel);
}

// Template enum을 사용자용 한글 label로 변환합니다.
FText SCFWeaponGuideTab::GetTemplateLabel(const ECFWeaponGuideTemplate Template)
{
	switch (Template)
	{
	case ECFWeaponGuideTemplate::DirectFireCannon: return LOCTEXT("TemplateCannon", "직사포 (Direct Fire Cannon)");
	case ECFWeaponGuideTemplate::AutomaticGun: return LOCTEXT("TemplateAuto", "자동화기 (Automatic Gun)");
	case ECFWeaponGuideTemplate::RocketLauncher: return LOCTEXT("TemplateRocket", "로켓 런처 (Rocket Launcher)");
	case ECFWeaponGuideTemplate::GuidedMissileLauncher: return LOCTEXT("TemplateMissile", "유도미사일 런처 (Guided Missile Launcher)");
	case ECFWeaponGuideTemplate::Custom: return LOCTEXT("TemplateCustom", "사용자 정의 (Custom)");
	default: return LOCTEXT("TemplateUnknown", "알 수 없음");
	}
}

// MountType enum을 사용자용 한글/영문 label로 변환합니다.
FText SCFWeaponGuideTab::GetMountTypeLabel(const ECFVehicleMountType MountType)
{
	switch (MountType)
	{
	case ECFVehicleMountType::Fixed: return LOCTEXT("MountFixed", "고정형 (Fixed)");
	case ECFVehicleMountType::Gimbal: return LOCTEXT("MountGimbal", "짐벌 (Gimbal)");
	case ECFVehicleMountType::Turret: return LOCTEXT("MountTurret", "포탑 (Turret)");
	case ECFVehicleMountType::Launcher: return LOCTEXT("MountLauncher", "런처 (Launcher)");
	case ECFVehicleMountType::Utility: return LOCTEXT("MountUtility", "유틸리티 (Utility)");
	default: return LOCTEXT("MountNone", "없음 (None)");
	}
}

// WeaponSize enum을 사용자용 label로 변환합니다.
FText SCFWeaponGuideTab::GetWeaponSizeLabel(const ECFVehicleWeaponSize WeaponSize)
{
	switch (WeaponSize)
	{
	case ECFVehicleWeaponSize::Small: return LOCTEXT("SizeSmall", "소형 (Small)");
	case ECFVehicleWeaponSize::Medium: return LOCTEXT("SizeMedium", "중형 (Medium)");
	case ECFVehicleWeaponSize::Large: return LOCTEXT("SizeLarge", "대형 (Large)");
	default: return LOCTEXT("SizeNone", "없음 (None)");
	}
}

#undef LOCTEXT_NAMESPACE
