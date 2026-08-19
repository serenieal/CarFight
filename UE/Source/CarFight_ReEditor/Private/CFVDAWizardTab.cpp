// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.8.0
// Date: 2026-08-18
// Description: P0-10 parity 기간 동안 유지되는 Legacy Vehicle DA 입력 보조용 Editor Slate 탭 구현입니다.
// Scope: CFVehicleData 선택, UCFVDAValidator 실행, 리포트 텍스트 표시와 복사를 제공합니다.
// Changelog:
// - v1.8.0: managed Authoring Recipe Target에서는 legacy Layout/Quick Tune/Revert 변경 동작을 Common Authoring facade 판정으로 비활성화하고 handler에서도 재검사.
// - v1.7.0: 차체 소켓 캡처 Wizard 문구와 트랜잭션 이름을 차량 레이아웃 통합 캡처 기준으로 갱신.
// - v1.6.0: 주행감 Quick Tune 되돌리기 버튼과 Target DA 기준값 스냅샷을 추가.
// - v1.5.0: Target DA 선택/로드 시 현재 Movement 값으로 Quick Tune 슬라이더를 역동기화.
// - v1.4.0: 가속감 Quick Tune이 ThrottleInputScale과 더 넓은 토크 범위를 함께 적용하도록 보강.
// - v1.3.0: 차종별 주행감 Quick Tune 프리셋, 슬라이더, DA 적용 버튼을 추가.
// - v1.2.0: Target DA 차체 소켓에서 휠 레이아웃을 캡처하는 Wizard 버튼을 추가.
// - v1.1.0: 검사 항목 선택 목록과 선택 항목 상세 패널을 추가.
// - v1.0.0: Target/Source 선택, 전체 검사 실행, 리포트 복사, DA 에디터 열기 UI를 추가.
// Migration:
// - 기존 CaptureLayoutFromChassisSockets 호출은 유지하며, 함수 내부 통합 캡처 확장을 그대로 사용한다.
// - Target DA를 새로 불러오면 Quick Tune 되돌리기 기준값도 해당 DA의 현재 Movement 값으로 갱신된다.
// - Target DA를 다시 불러오면 슬라이더 임시값은 DA에 저장된 Movement 값 기준으로 갱신된다.
// - 가속감 0%와 100%의 차이를 확인하려면 Quick Tune 적용 후 ThrottleInputScale 미리보기를 확인한다.
// - 기존 VehicleData 선택과 Validator 표시 흐름은 유지하고 Quick Tune 적용 값만 확장한다.
// - DA 값 변경은 사용자가 "DA에 주행감 적용" 버튼을 누를 때만 수행한다.

#include "CFVDAWizardTab.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "AssetRegistry/AssetData.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SCFVDAWizardTab"

namespace
{
	// 초기 상태에서 리포트 영역에 표시할 안내 문구입니다.
	static const TCHAR* InitialReportText = TEXT("아직 검사 결과가 없습니다. Content Browser에서 CFVehicleData 자산을 선택한 뒤 Target으로 지정하고 전체 검사를 실행하세요.");

	// Quick Tune 가속감 0%에 대응하는 엔진 최대 토크입니다.
	constexpr float DrivingFeelEngineMaxTorqueMin = 250.0f;

	// Quick Tune 가속감 100%에 대응하는 엔진 최대 토크입니다.
	constexpr float DrivingFeelEngineMaxTorqueMax = 1600.0f;

	// Quick Tune 가속감 0%에 대응하는 엔진 최대 RPM입니다.
	constexpr float DrivingFeelEngineMaxRPMMin = 4500.0f;

	// Quick Tune 가속감 100%에 대응하는 엔진 최대 RPM입니다.
	constexpr float DrivingFeelEngineMaxRPMMax = 8500.0f;

	// Quick Tune 가속감 0%에 대응하는 스로틀 입력 배율입니다.
	constexpr float DrivingFeelThrottleInputScaleMin = 0.20f;

	// Quick Tune 가속감 100%에 대응하는 스로틀 입력 배율입니다.
	constexpr float DrivingFeelThrottleInputScaleMax = 1.0f;

	// Quick Tune 조향 민첩성 0%에 대응하는 전륜 최대 조향각입니다.
	constexpr float DrivingFeelFrontSteerAngleMin = 26.0f;

	// Quick Tune 조향 민첩성 100%에 대응하는 전륜 최대 조향각입니다.
	constexpr float DrivingFeelFrontSteerAngleMax = 48.0f;

	// Quick Tune 조향 민첩성 0%에 대응하는 조향 AngleRatio입니다.
	constexpr float DrivingFeelSteeringAngleRatioMin = 0.45f;

	// Quick Tune 조향 민첩성 100%에 대응하는 조향 AngleRatio입니다.
	constexpr float DrivingFeelSteeringAngleRatioMax = 0.90f;

	// Quick Tune 접지감 0%에 대응하는 휠 마찰력 배수입니다.
	constexpr float DrivingFeelWheelFrictionMin = 1.2f;

	// Quick Tune 접지감 100%에 대응하는 휠 마찰력 배수입니다.
	constexpr float DrivingFeelWheelFrictionMax = 3.2f;

	// Quick Tune 접지감 0%에 대응하는 코너링 강성입니다.
	constexpr float DrivingFeelCorneringStiffnessMin = 700.0f;

	// Quick Tune 접지감 100%에 대응하는 코너링 강성입니다.
	constexpr float DrivingFeelCorneringStiffnessMax = 1500.0f;

	// Quick Tune 서스펜션 단단함 0%에 대응하는 스프링 강성입니다.
	constexpr float DrivingFeelSpringRateMin = 160.0f;

	// Quick Tune 서스펜션 단단함 100%에 대응하는 스프링 강성입니다.
	constexpr float DrivingFeelSpringRateMax = 500.0f;

	// Quick Tune 서스펜션 단단함 0%에 대응하는 스프링 프리로드입니다.
	constexpr float DrivingFeelSpringPreloadMin = 35.0f;

	// Quick Tune 서스펜션 단단함 100%에 대응하는 스프링 프리로드입니다.
	constexpr float DrivingFeelSpringPreloadMax = 90.0f;

	// 현재 수치가 Quick Tune 범위 안에서 어느 슬라이더 위치에 해당하는지 반환합니다.
	float NormalizeDrivingFeelSliderValue(const float CurrentValue, const float MinValue, const float MaxValue)
	{
		// 0으로 나누기를 피하기 위한 유효 범위 폭입니다.
		const float SafeRange = FMath::Max(MaxValue - MinValue, KINDA_SMALL_NUMBER);

		return FMath::Clamp((CurrentValue - MinValue) / SafeRange, 0.0f, 1.0f);
	}

	// 심각도 값을 리포트 표시용 문자열로 변환합니다.
	FString GetSeverityLabel(const ECFVDASeverity Severity)
	{
		switch (Severity)
		{
		case ECFVDASeverity::Pass:
			return TEXT("정상");
		case ECFVDASeverity::Info:
			return TEXT("정보");
		case ECFVDASeverity::Warning:
			return TEXT("경고");
		case ECFVDASeverity::Error:
			return TEXT("오류");
		case ECFVDASeverity::Blocked:
			return TEXT("보류");
		default:
			return TEXT("알 수 없음");
		}
	}

	// VehicleData 자산의 표시 이름과 경로를 한 줄 문자열로 만듭니다.
	FString GetVehicleDataDisplayText(const UCFVehicleData* VehicleData)
	{
		if (!VehicleData)
		{
			return TEXT("지정되지 않음");
		}

		return FString::Printf(TEXT("%s (%s)"), *VehicleData->GetName(), *VehicleData->GetPathName());
	}

	// VehicleData 자산 경로를 텍스트 박스에 넣기 좋은 Object Path 문자열로 만듭니다.
	FString GetVehicleDataObjectPath(const UCFVehicleData* VehicleData)
	{
		if (!VehicleData)
		{
			return FString();
		}

		return VehicleData->GetPathName();
	}

	// Content Browser에서 선택한 첫 CFVehicleData 자산을 찾습니다.
	UCFVehicleData* GetSelectedVehicleData(FString& OutMessage)
	{
		// Content Browser 모듈 참조입니다.
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		// Content Browser에서 현재 선택된 자산 목록입니다.
		TArray<FAssetData> SelectedAssets;
		ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

		if (SelectedAssets.Num() == 0)
		{
			OutMessage = TEXT("Content Browser에서 CFVehicleData 자산을 먼저 선택하세요.");
			return nullptr;
		}

		for (const FAssetData& SelectedAsset : SelectedAssets)
		{
			// 현재 선택 항목에서 로드한 UObject입니다.
			UObject* SelectedObject = SelectedAsset.GetAsset();

			// 선택 항목을 CFVehicleData로 변환한 결과입니다.
			UCFVehicleData* SelectedVehicleData = Cast<UCFVehicleData>(SelectedObject);
			if (SelectedVehicleData)
			{
				OutMessage = FString::Printf(TEXT("선택된 VehicleData를 사용합니다: %s"), *SelectedVehicleData->GetName());
				return SelectedVehicleData;
			}
		}

		OutMessage = TEXT("선택 항목 중 CFVehicleData 자산을 찾지 못했습니다.");
		return nullptr;
	}
}

// Vehicle DA Wizard 탭의 Slate 레이아웃과 초기 상태를 구성합니다.
void SCFVDAWizardTab::Construct(const FArguments& InArgs)
{
	StatusMessage = TEXT("Vehicle DA Wizard가 준비되었습니다.");
	ReportText = InitialReportText;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(12.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

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
							.Text(LOCTEXT("Title", "Vehicle DA Wizard"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(LOCTEXT("Subtitle", "새 차량 CFVehicleData를 Target으로 지정하고, 필요하면 기준 DA를 Source로 지정한 뒤 검사 리포트를 확인합니다."))
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("TargetTitle", "Target DA"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SCFVDAWizardTab::GetTargetVehicleDataText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SAssignNew(TargetPathTextBox, SEditableTextBox)
							.HintText(LOCTEXT("TargetPathHint", "/Game/.../DA_NewCar.DA_NewCar"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("SetTargetBtn", "선택을 Target으로"))
								.OnClicked(this, &SCFVDAWizardTab::HandleSetTargetFromSelectionClicked)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("OpenTargetBtn", "Target DA 열기"))
								.OnClicked(this, &SCFVDAWizardTab::HandleOpenTargetClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanOpenTarget)
							]
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DrivingFeelTitle", "주행감 Quick Tune"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(LOCTEXT("DrivingFeelHint", "차종별 주행감을 체감 슬라이더로 조절한 뒤 버튼을 눌러 Target DA에 적용합니다. 슬라이더만 움직일 때는 DA 원본이 바뀌지 않습니다."))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("SedanPresetBtn", "세단 프리셋"))
								.OnClicked(this, &SCFVDAWizardTab::HandleSedanPresetClicked)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("SuvPresetBtn", "SUV 프리셋"))
								.OnClicked(this, &SCFVDAWizardTab::HandleSuvPresetClicked)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("SportsPresetBtn", "스포츠 프리셋"))
								.OnClicked(this, &SCFVDAWizardTab::HandleSportsPresetClicked)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("HeavyPresetBtn", "무거운 차량 프리셋"))
								.OnClicked(this, &SCFVDAWizardTab::HandleHeavyPresetClicked)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(this, &SCFVDAWizardTab::GetAccelerationFeelText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(SSlider)
							.Value(this, &SCFVDAWizardTab::GetAccelerationFeelValue)
							.OnValueChanged(this, &SCFVDAWizardTab::HandleAccelerationFeelChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(this, &SCFVDAWizardTab::GetSteeringFeelText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(SSlider)
							.Value(this, &SCFVDAWizardTab::GetSteeringFeelValue)
							.OnValueChanged(this, &SCFVDAWizardTab::HandleSteeringFeelChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(this, &SCFVDAWizardTab::GetGripFeelText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(SSlider)
							.Value(this, &SCFVDAWizardTab::GetGripFeelValue)
							.OnValueChanged(this, &SCFVDAWizardTab::HandleGripFeelChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(this, &SCFVDAWizardTab::GetSuspensionFeelText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 0.0f)
						[
							SNew(SSlider)
							.Value(this, &SCFVDAWizardTab::GetSuspensionFeelValue)
							.OnValueChanged(this, &SCFVDAWizardTab::HandleSuspensionFeelChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SCFVDAWizardTab::GetDrivingFeelPreviewText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("ApplyDrivingFeelBtn", "DA에 주행감 적용"))
								.ToolTipText(LOCTEXT("ApplyDrivingFeelTooltip", "현재 슬라이더 값으로 Target DA의 VehicleMovementConfig 수치를 저장합니다."))
								.OnClicked(this, &SCFVDAWizardTab::HandleApplyDrivingFeelClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanApplyDrivingFeel)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(6.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("RevertDrivingFeelBtn", "되돌리기"))
								.ToolTipText(LOCTEXT("RevertDrivingFeelTooltip", "Target DA를 마지막으로 불러왔을 때의 Quick Tune 기준 수치로 되돌립니다."))
								.OnClicked(this, &SCFVDAWizardTab::HandleRevertDrivingFeelClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanRevertDrivingFeel)
							]
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SourceTitle", "Source DA"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SCFVDAWizardTab::GetSourceVehicleDataText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SAssignNew(SourcePathTextBox, SEditableTextBox)
							.HintText(LOCTEXT("SourcePathHint", "/Game/.../DA_TestSedan.DA_TestSedan"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("SetSourceBtn", "선택을 Source로"))
								.OnClicked(this, &SCFVDAWizardTab::HandleSetSourceFromSelectionClicked)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("OpenSourceBtn", "Source DA 열기"))
								.OnClicked(this, &SCFVDAWizardTab::HandleOpenSourceClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanOpenSource)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("ClearSourceBtn", "Source 비우기"))
								.OnClicked(this, &SCFVDAWizardTab::HandleClearSourceClicked)
							]
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ActionTitle", "검사 작업"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("ValidateBtn", "전체 검사 실행"))
								.OnClicked(this, &SCFVDAWizardTab::HandleValidateClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanValidate)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CaptureLayoutBtn", "차량 레이아웃 캡처"))
								.OnClicked(this, &SCFVDAWizardTab::HandleCaptureLayoutClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanCaptureLayout)
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("CopyReportBtn", "리포트 복사"))
								.OnClicked(this, &SCFVDAWizardTab::HandleCopyReportClicked)
								.IsEnabled(this, &SCFVDAWizardTab::CanCopyReport)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SCFVDAWizardTab::GetStatusMessageText)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SummaryTitle", "검사 요약"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SCFVDAWizardTab::GetReportSummaryText)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ResultListTitle", "검사 항목 선택"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(LOCTEXT("ResultListHint", "검사 실행 후 항목을 클릭하면 어떤 DA 필드를 고쳐야 하는지 아래 상세 패널에서 확인할 수 있습니다."))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SBox)
							.HeightOverride(220.0f)
							[
								SAssignNew(ResultListView, SListView<TSharedPtr<FCFVDARow>>)
								.ListItemsSource(&ResultRows)
								.SelectionMode(ESelectionMode::Single)
								.OnGenerateRow(this, &SCFVDAWizardTab::HandleGenerateResultRow)
								.OnSelectionChanged(this, &SCFVDAWizardTab::HandleResultSelectionChanged)
							]
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SelectedResultTitle", "선택 항목 상세"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(this, &SCFVDAWizardTab::GetSelectedResultDetailText)
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ReportTitle", "검사 리포트"))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(0.0f, 8.0f, 0.0f, 0.0f)
						[
							SNew(SBox)
							.HeightOverride(360.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SNew(STextBlock)
									.AutoWrapText(true)
									.Text(this, &SCFVDAWizardTab::GetReportText)
								]
							]
						]
					]
				]
			]
		]
	];
}

// Content Browser 선택 자산을 대상 DA로 지정합니다.
FReply SCFVDAWizardTab::HandleSetTargetFromSelectionClicked()
{
	// 선택 처리 결과 메시지입니다.
	FString SelectionMessage;

	// Content Browser에서 찾은 VehicleData입니다.
	UCFVehicleData* SelectedVehicleData = GetSelectedVehicleData(SelectionMessage);
	if (!SelectedVehicleData)
	{
		StatusMessage = SelectionMessage;
		return FReply::Handled();
	}

	SetVehicleData(TargetVehicleData, TargetPathTextBox, SelectedVehicleData);
	CaptureDrivingFeelRevertValuesFromTargetData(SelectedVehicleData);
	SyncDrivingFeelSlidersFromTargetData(SelectedVehicleData);
	StatusMessage = FString::Printf(TEXT("Target DA를 지정하고 Quick Tune 슬라이더를 현재값으로 맞췄습니다: %s"), *SelectedVehicleData->GetName());
	return FReply::Handled();
}

// Content Browser 선택 자산을 기준 DA로 지정합니다.
FReply SCFVDAWizardTab::HandleSetSourceFromSelectionClicked()
{
	// 선택 처리 결과 메시지입니다.
	FString SelectionMessage;

	// Content Browser에서 찾은 VehicleData입니다.
	UCFVehicleData* SelectedVehicleData = GetSelectedVehicleData(SelectionMessage);
	if (!SelectedVehicleData)
	{
		StatusMessage = SelectionMessage;
		return FReply::Handled();
	}

	SetVehicleData(SourceVehicleData, SourcePathTextBox, SelectedVehicleData);
	StatusMessage = FString::Printf(TEXT("Source DA를 지정했습니다: %s"), *SelectedVehicleData->GetName());
	return FReply::Handled();
}

// 기준 DA 선택을 비웁니다.
FReply SCFVDAWizardTab::HandleClearSourceClicked()
{
	SourceVehicleData.Reset();
	if (SourcePathTextBox.IsValid())
	{
		SourcePathTextBox->SetText(FText::GetEmpty());
	}

	StatusMessage = TEXT("Source DA를 비웠습니다. 다음 검사는 기준 비교 없이 실행됩니다.");
	return FReply::Handled();
}

// 대상 DA를 전체 검사하고 결과 텍스트를 갱신합니다.
FReply SCFVDAWizardTab::HandleValidateClicked()
{
	// Target 경로 로드 결과 메시지입니다.
	FString TargetLoadMessage;

	// 검사 대상 VehicleData입니다.
	UCFVehicleData* LoadedTargetVehicleData = LoadVehicleDataFromTextBox(TargetPathTextBox, true, TargetLoadMessage);
	if (!LoadedTargetVehicleData)
	{
		StatusMessage = TargetLoadMessage;
		return FReply::Handled();
	}

	SetVehicleData(TargetVehicleData, TargetPathTextBox, LoadedTargetVehicleData);
	CaptureDrivingFeelRevertValuesFromTargetData(LoadedTargetVehicleData);
	SyncDrivingFeelSlidersFromTargetData(LoadedTargetVehicleData);

	// Source 경로 로드 결과 메시지입니다.
	FString SourceLoadMessage;

	// 비교 기준 VehicleData입니다.
	UCFVehicleData* LoadedSourceVehicleData = LoadVehicleDataFromTextBox(SourcePathTextBox, false, SourceLoadMessage);
	if (LoadedSourceVehicleData)
	{
		SetVehicleData(SourceVehicleData, SourcePathTextBox, LoadedSourceVehicleData);
	}
	else if (SourcePathTextBox.IsValid() && SourcePathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty())
	{
		SourceVehicleData.Reset();
	}
	else if (!SourceLoadMessage.IsEmpty())
	{
		StatusMessage = SourceLoadMessage;
		return FReply::Handled();
	}

	LastReport = UCFVDAValidator::ValidateVehicleData(LoadedTargetVehicleData, LoadedSourceVehicleData);
	ReportText = BuildReportText(LastReport);
	RebuildResultRows(LastReport);
	StatusMessage = FString::Printf(TEXT("검사가 완료되었습니다. 오류 %d개, 경고 %d개, 정보 %d개, 보류 %d개입니다."), LastReport.ErrorCount, LastReport.WarningCount, LastReport.InfoCount, LastReport.BlockedCount);
	return FReply::Handled();
}

// 대상 DA의 차체 소켓에서 차량 레이아웃 값을 캡처합니다.
FReply SCFVDAWizardTab::HandleCaptureLayoutClicked()
{
	// Target 경로 로드 결과 메시지입니다.
	FString TargetLoadMessage;

	// 차량 레이아웃을 캡처할 대상 VehicleData입니다.
	UCFVehicleData* LoadedTargetVehicleData = LoadVehicleDataFromTextBox(TargetPathTextBox, true, TargetLoadMessage);
	if (!LoadedTargetVehicleData)
	{
		StatusMessage = TargetLoadMessage;
		return FReply::Handled();
	}

	SetVehicleData(TargetVehicleData, TargetPathTextBox, LoadedTargetVehicleData);
	// 새 Authoring Recipe가 연결된 Target은 legacy direct Layout mutation 대신 Workspace를 사용합니다.
	FString ManagedTargetMessage;
	if (HasManagedAuthoringRecipe(&ManagedTargetMessage))
	{
		StatusMessage = ManagedTargetMessage;
		return FReply::Handled();
	}
	CaptureDrivingFeelRevertValuesFromTargetData(LoadedTargetVehicleData);
	SyncDrivingFeelSlidersFromTargetData(LoadedTargetVehicleData);

	// Source 경로 로드 결과 메시지입니다.
	FString SourceLoadMessage;

	// 캡처 후 재검사에 사용할 비교 기준 VehicleData입니다.
	UCFVehicleData* LoadedSourceVehicleData = LoadVehicleDataFromTextBox(SourcePathTextBox, false, SourceLoadMessage);
	if (LoadedSourceVehicleData)
	{
		SetVehicleData(SourceVehicleData, SourcePathTextBox, LoadedSourceVehicleData);
	}
	else if (SourcePathTextBox.IsValid() && SourcePathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty())
	{
		SourceVehicleData.Reset();
	}
	else if (!SourceLoadMessage.IsEmpty())
	{
		StatusMessage = SourceLoadMessage;
		return FReply::Handled();
	}

	// 사용자가 Undo로 캡처 전 상태로 되돌릴 수 있게 묶는 에디터 트랜잭션입니다.
	const FScopedTransaction CaptureTransaction(LOCTEXT("CaptureLayoutTransaction", "Capture Vehicle DA Layout From Sockets"));
	LoadedTargetVehicleData->Modify();
	LoadedTargetVehicleData->CaptureLayoutFromChassisSockets();

	LastReport = UCFVDAValidator::ValidateVehicleData(LoadedTargetVehicleData, LoadedSourceVehicleData);
	ReportText = BuildReportText(LastReport);
	RebuildResultRows(LastReport);
	StatusMessage = FString::Printf(TEXT("차량 레이아웃 캡처를 실행하고 재검사를 완료했습니다. 오류 %d개, 경고 %d개, 정보 %d개, 보류 %d개입니다."), LastReport.ErrorCount, LastReport.WarningCount, LastReport.InfoCount, LastReport.BlockedCount);
	return FReply::Handled();
}

// 세단형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
FReply SCFVDAWizardTab::HandleSedanPresetClicked()
{
	ApplyDrivingFeelPreset(0.50f, 0.50f, 0.55f, 0.45f, TEXT("세단"));
	return FReply::Handled();
}

// SUV형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
FReply SCFVDAWizardTab::HandleSuvPresetClicked()
{
	ApplyDrivingFeelPreset(0.38f, 0.34f, 0.50f, 0.38f, TEXT("SUV"));
	return FReply::Handled();
}

// 스포츠형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
FReply SCFVDAWizardTab::HandleSportsPresetClicked()
{
	ApplyDrivingFeelPreset(0.85f, 0.78f, 0.82f, 0.78f, TEXT("스포츠"));
	return FReply::Handled();
}

// 무거운 차량형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
FReply SCFVDAWizardTab::HandleHeavyPresetClicked()
{
	ApplyDrivingFeelPreset(0.28f, 0.25f, 0.45f, 0.55f, TEXT("무거운 차량"));
	return FReply::Handled();
}

// 임시 주행감 슬라이더 값을 Target DA의 MovementConfig에 적용합니다.
FReply SCFVDAWizardTab::HandleApplyDrivingFeelClicked()
{
	// Target 경로 로드 결과 메시지입니다.
	FString TargetLoadMessage;

	// 주행감 수치를 적용할 대상 VehicleData입니다.
	UCFVehicleData* LoadedTargetVehicleData = LoadVehicleDataFromTextBox(TargetPathTextBox, true, TargetLoadMessage);
	if (!LoadedTargetVehicleData)
	{
		StatusMessage = TargetLoadMessage;
		return FReply::Handled();
	}

	SetVehicleData(TargetVehicleData, TargetPathTextBox, LoadedTargetVehicleData);
	// 새 Authoring Recipe에 연결된 Target은 Vehicle Authoring Workspace에서 조정합니다.
	FString ManagedTargetMessage;
	if (HasManagedAuthoringRecipe(&ManagedTargetMessage))
	{
		StatusMessage = ManagedTargetMessage;
		return FReply::Handled();
	}

	// Source 경로 로드 결과 메시지입니다.
	FString SourceLoadMessage;

	// 적용 후 재검사에 사용할 비교 기준 VehicleData입니다.
	UCFVehicleData* LoadedSourceVehicleData = LoadVehicleDataFromTextBox(SourcePathTextBox, false, SourceLoadMessage);
	if (LoadedSourceVehicleData)
	{
		SetVehicleData(SourceVehicleData, SourcePathTextBox, LoadedSourceVehicleData);
	}
	else if (SourcePathTextBox.IsValid() && SourcePathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty())
	{
		SourceVehicleData.Reset();
	}
	else if (!SourceLoadMessage.IsEmpty())
	{
		StatusMessage = SourceLoadMessage;
		return FReply::Handled();
	}

	// 현재 슬라이더 값으로 계산한 적용 예정 Movement 수치입니다.
	const FCFDrivingFeelValues FeelValues = BuildDrivingFeelValues();

	// 사용자가 Undo로 Quick Tune 적용 전 상태로 되돌릴 수 있게 묶는 에디터 트랜잭션입니다.
	const FScopedTransaction DrivingFeelTransaction(LOCTEXT("ApplyDrivingFeelTransaction", "Apply Vehicle DA Driving Feel Quick Tune"));
	LoadedTargetVehicleData->Modify();
	ApplyDrivingFeelValuesToData(LoadedTargetVehicleData, FeelValues);
	LoadedTargetVehicleData->MarkPackageDirty();

	LastReport = UCFVDAValidator::ValidateVehicleData(LoadedTargetVehicleData, LoadedSourceVehicleData);
	ReportText = BuildReportText(LastReport);
	RebuildResultRows(LastReport);
	StatusMessage = FString::Printf(TEXT("주행감 Quick Tune을 Target DA에 적용하고 재검사를 완료했습니다. 오류 %d개, 경고 %d개, 정보 %d개, 보류 %d개입니다."), LastReport.ErrorCount, LastReport.WarningCount, LastReport.InfoCount, LastReport.BlockedCount);
	return FReply::Handled();
}

// Target DA를 Quick Tune 기준 수치로 되돌립니다.
FReply SCFVDAWizardTab::HandleRevertDrivingFeelClicked()
{
	if (!bHasDrivingFeelRevertValues)
	{
		StatusMessage = TEXT("되돌릴 Quick Tune 기준값이 없습니다. Target DA를 먼저 불러오세요.");
		return FReply::Handled();
	}

	// Target 경로 로드 결과 메시지입니다.
	FString TargetLoadMessage;

	// 되돌리기 값을 적용할 대상 VehicleData입니다.
	UCFVehicleData* LoadedTargetVehicleData = LoadVehicleDataFromTextBox(TargetPathTextBox, true, TargetLoadMessage);
	if (!LoadedTargetVehicleData)
	{
		StatusMessage = TargetLoadMessage;
		return FReply::Handled();
	}

	// 현재 Target DA가 되돌리기 기준값을 캡처한 DA와 같은지 확인하기 위한 자산 경로입니다.
	const FString CurrentTargetPath = LoadedTargetVehicleData->GetPathName();
	if (CurrentTargetPath != DrivingFeelRevertTargetPath)
	{
		StatusMessage = TEXT("현재 Target DA와 되돌리기 기준 DA가 다릅니다. Target DA를 다시 불러와 기준값을 갱신하세요.");
		return FReply::Handled();
	}

	SetVehicleData(TargetVehicleData, TargetPathTextBox, LoadedTargetVehicleData);
	// 새 Authoring Recipe에 연결된 Target은 Workspace의 Unreal 표준 Undo를 사용합니다.
	FString ManagedTargetMessage;
	if (HasManagedAuthoringRecipe(&ManagedTargetMessage))
	{
		StatusMessage = ManagedTargetMessage;
		return FReply::Handled();
	}

	// Source 경로 로드 결과 메시지입니다.
	FString SourceLoadMessage;

	// 되돌린 뒤 재검사에 사용할 비교 기준 VehicleData입니다.
	UCFVehicleData* LoadedSourceVehicleData = LoadVehicleDataFromTextBox(SourcePathTextBox, false, SourceLoadMessage);
	if (LoadedSourceVehicleData)
	{
		SetVehicleData(SourceVehicleData, SourcePathTextBox, LoadedSourceVehicleData);
	}
	else if (SourcePathTextBox.IsValid() && SourcePathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty())
	{
		SourceVehicleData.Reset();
	}
	else if (!SourceLoadMessage.IsEmpty())
	{
		StatusMessage = SourceLoadMessage;
		return FReply::Handled();
	}

	// 사용자가 에디터 Undo로도 되돌리기 작업 자체를 취소할 수 있게 묶는 트랜잭션입니다.
	const FScopedTransaction RevertDrivingFeelTransaction(LOCTEXT("RevertDrivingFeelTransaction", "Revert Vehicle DA Driving Feel Quick Tune"));
	LoadedTargetVehicleData->Modify();
	ApplyDrivingFeelValuesToData(LoadedTargetVehicleData, DrivingFeelRevertValues);
	LoadedTargetVehicleData->MarkPackageDirty();
	SyncDrivingFeelSlidersFromTargetData(LoadedTargetVehicleData);

	LastReport = UCFVDAValidator::ValidateVehicleData(LoadedTargetVehicleData, LoadedSourceVehicleData);
	ReportText = BuildReportText(LastReport);
	RebuildResultRows(LastReport);
	StatusMessage = FString::Printf(TEXT("Target DA를 Quick Tune 기준값으로 되돌리고 재검사를 완료했습니다. 오류 %d개, 경고 %d개, 정보 %d개, 보류 %d개입니다."), LastReport.ErrorCount, LastReport.WarningCount, LastReport.InfoCount, LastReport.BlockedCount);
	return FReply::Handled();
}

// 마지막 검사 리포트 텍스트를 클립보드에 복사합니다.
FReply SCFVDAWizardTab::HandleCopyReportClicked()
{
	if (ReportText.IsEmpty() || ReportText == InitialReportText)
	{
		StatusMessage = TEXT("복사할 검사 리포트가 아직 없습니다.");
		return FReply::Handled();
	}

	FPlatformApplicationMisc::ClipboardCopy(*ReportText);
	StatusMessage = TEXT("검사 리포트를 클립보드에 복사했습니다.");
	return FReply::Handled();
}

// 대상 DA 자산 에디터를 엽니다.
FReply SCFVDAWizardTab::HandleOpenTargetClicked()
{
	// Target 경로 로드 결과 메시지입니다.
	FString TargetLoadMessage;

	// 열 대상 VehicleData입니다.
	UCFVehicleData* LoadedTargetVehicleData = LoadVehicleDataFromTextBox(TargetPathTextBox, true, TargetLoadMessage);
	OpenVehicleDataEditor(LoadedTargetVehicleData, TargetLoadMessage);
	return FReply::Handled();
}

// 기준 DA 자산 에디터를 엽니다.
FReply SCFVDAWizardTab::HandleOpenSourceClicked()
{
	// Source 경로 로드 결과 메시지입니다.
	FString SourceLoadMessage;

	// 열 기준 VehicleData입니다.
	UCFVehicleData* LoadedSourceVehicleData = LoadVehicleDataFromTextBox(SourcePathTextBox, true, SourceLoadMessage);
	OpenVehicleDataEditor(LoadedSourceVehicleData, SourceLoadMessage);
	return FReply::Handled();
}

// 검사 결과 행 Slate 위젯을 생성합니다.
TSharedRef<ITableRow> SCFVDAWizardTab::HandleGenerateResultRow(TSharedPtr<FCFVDARow> RowItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 목록에 표시할 행 텍스트입니다.
	const FString RowText = RowItem.IsValid() ? BuildResultRowText(*RowItem) : FString(TEXT("알 수 없는 검사 항목"));

	return SNew(STableRow<TSharedPtr<FCFVDARow>>, OwnerTable)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(RowText))
		];
}

// 검사 결과 목록에서 선택한 항목을 상세 패널 상태로 반영합니다.
void SCFVDAWizardTab::HandleResultSelectionChanged(TSharedPtr<FCFVDARow> SelectedRow, ESelectInfo::Type SelectInfo)
{
	SelectedResultRow = SelectedRow;

	if (SelectedResultRow.IsValid())
	{
		StatusMessage = FString::Printf(TEXT("검사 항목 %d번을 선택했습니다. 상세 패널에서 필드와 조치 내용을 확인하세요."), SelectedResultRow->ItemNumber);
	}
}

// 가속감 슬라이더 변경값을 임시 상태에 반영합니다.
void SCFVDAWizardTab::HandleAccelerationFeelChanged(float NewValue)
{
	AccelerationFeelValue = NewValue;
}

// 조향 민첩성 슬라이더 변경값을 임시 상태에 반영합니다.
void SCFVDAWizardTab::HandleSteeringFeelChanged(float NewValue)
{
	SteeringFeelValue = NewValue;
}

// 접지감 슬라이더 변경값을 임시 상태에 반영합니다.
void SCFVDAWizardTab::HandleGripFeelChanged(float NewValue)
{
	GripFeelValue = NewValue;
}

// 서스펜션 단단함 슬라이더 변경값을 임시 상태에 반영합니다.
void SCFVDAWizardTab::HandleSuspensionFeelChanged(float NewValue)
{
	SuspensionFeelValue = NewValue;
}

// 상태 안내 문구를 Slate 표시용 텍스트로 반환합니다.
FText SCFVDAWizardTab::GetStatusMessageText() const
{
	return FText::FromString(StatusMessage);
}

// 대상 DA 표시 문자열을 반환합니다.
FText SCFVDAWizardTab::GetTargetVehicleDataText() const
{
	return FText::FromString(GetVehicleDataDisplayText(TargetVehicleData.Get()));
}

// 기준 DA 표시 문자열을 반환합니다.
FText SCFVDAWizardTab::GetSourceVehicleDataText() const
{
	return FText::FromString(GetVehicleDataDisplayText(SourceVehicleData.Get()));
}

// 마지막 리포트 요약 문자열을 반환합니다.
FText SCFVDAWizardTab::GetReportSummaryText() const
{
	if (LastReport.SummaryText.IsEmpty())
	{
		return LOCTEXT("NoSummaryText", "아직 실행된 검사가 없습니다.");
	}

	return FText::FromString(LastReport.SummaryText);
}

// 마지막 리포트 본문 문자열을 반환합니다.
FText SCFVDAWizardTab::GetReportText() const
{
	return FText::FromString(ReportText.IsEmpty() ? FString(InitialReportText) : ReportText);
}

// 현재 선택한 검사 항목의 상세 설명을 반환합니다.
FText SCFVDAWizardTab::GetSelectedResultDetailText() const
{
	if (!SelectedResultRow.IsValid())
	{
		return LOCTEXT("NoSelectedResultDetail", "선택된 검사 항목이 없습니다. 검사를 실행한 뒤 목록에서 항목을 클릭하세요.");
	}

	// 선택한 검사 항목입니다.
	const FCFVDAValidationItem& Item = SelectedResultRow->Item;

	// 선택 상세 패널에 표시할 문자열입니다.
	FString DetailText;
	DetailText += FString::Printf(TEXT("번호: %d\n"), SelectedResultRow->ItemNumber);
	DetailText += FString::Printf(TEXT("심각도: %s\n"), *GetSeverityLabel(Item.Severity));
	DetailText += FString::Printf(TEXT("그룹: %s\n"), *Item.GroupName.ToString());
	DetailText += FString::Printf(TEXT("필드 경로: %s\n"), *Item.FieldPath);
	DetailText += FString::Printf(TEXT("표시 이름: %s\n"), *Item.DisplayName.ToString());
	DetailText += FString::Printf(TEXT("상태 설명: %s\n"), *Item.Message.ToString());
	DetailText += FString::Printf(TEXT("권장 조치: %s"), *Item.RecommendedAction.ToString());
	return FText::FromString(DetailText);
}

// 가속감 슬라이더 값을 반환합니다.
float SCFVDAWizardTab::GetAccelerationFeelValue() const
{
	return AccelerationFeelValue;
}

// 조향 민첩성 슬라이더 값을 반환합니다.
float SCFVDAWizardTab::GetSteeringFeelValue() const
{
	return SteeringFeelValue;
}

// 접지감 슬라이더 값을 반환합니다.
float SCFVDAWizardTab::GetGripFeelValue() const
{
	return GripFeelValue;
}

// 서스펜션 단단함 슬라이더 값을 반환합니다.
float SCFVDAWizardTab::GetSuspensionFeelValue() const
{
	return SuspensionFeelValue;
}

// 가속감 슬라이더 표시 텍스트를 반환합니다.
FText SCFVDAWizardTab::GetAccelerationFeelText() const
{
	return FText::FromString(FString::Printf(TEXT("가속감: %.0f%%"), AccelerationFeelValue * 100.0f));
}

// 조향 민첩성 슬라이더 표시 텍스트를 반환합니다.
FText SCFVDAWizardTab::GetSteeringFeelText() const
{
	return FText::FromString(FString::Printf(TEXT("조향 민첩성: %.0f%%"), SteeringFeelValue * 100.0f));
}

// 접지감 슬라이더 표시 텍스트를 반환합니다.
FText SCFVDAWizardTab::GetGripFeelText() const
{
	return FText::FromString(FString::Printf(TEXT("접지감: %.0f%%"), GripFeelValue * 100.0f));
}

// 서스펜션 단단함 슬라이더 표시 텍스트를 반환합니다.
FText SCFVDAWizardTab::GetSuspensionFeelText() const
{
	return FText::FromString(FString::Printf(TEXT("서스펜션 단단함: %.0f%%"), SuspensionFeelValue * 100.0f));
}

// 주행감 슬라이더가 만들 적용 예정 Movement 수치 텍스트를 반환합니다.
FText SCFVDAWizardTab::GetDrivingFeelPreviewText() const
{
	// 현재 슬라이더 값으로 계산한 적용 예정 Movement 수치입니다.
	const FCFDrivingFeelValues FeelValues = BuildDrivingFeelValues();

	// Quick Tune 미리보기 표시 문자열입니다.
	FString PreviewText;
	PreviewText += TEXT("적용 예정 수치\n");
	PreviewText += FString::Printf(TEXT("- bUseMovementOverrides: %s\n"), FeelValues.bUseMovementOverrides ? TEXT("true") : TEXT("false"));
	PreviewText += FString::Printf(TEXT("- EngineMaxTorque: %.0f\n"), FeelValues.EngineMaxTorque);
	PreviewText += FString::Printf(TEXT("- EngineMaxRPM: %.0f\n"), FeelValues.EngineMaxRPM);
	PreviewText += FString::Printf(TEXT("- ThrottleInputScale: %.2f\n"), FeelValues.ThrottleInputScale);
	PreviewText += FString::Printf(TEXT("- FrontWheelMaxSteerAngle: %.1f\n"), FeelValues.FrontWheelMaxSteerAngle);
	PreviewText += FString::Printf(TEXT("- SteeringAngleRatio: %.2f\n"), FeelValues.SteeringAngleRatio);
	PreviewText += FString::Printf(TEXT("- FrictionMultiplier F/R: %.2f / %.2f\n"), FeelValues.FrontWheelFrictionForceMultiplier, FeelValues.RearWheelFrictionForceMultiplier);
	PreviewText += FString::Printf(TEXT("- CorneringStiffness F/R: %.0f / %.0f\n"), FeelValues.FrontWheelCorneringStiffness, FeelValues.RearWheelCorneringStiffness);
	PreviewText += FString::Printf(TEXT("- SpringRate F/R: %.0f / %.0f\n"), FeelValues.FrontWheelSpringRate, FeelValues.RearWheelSpringRate);
	PreviewText += FString::Printf(TEXT("- SpringPreload F/R: %.0f / %.0f"), FeelValues.FrontWheelSpringPreload, FeelValues.RearWheelSpringPreload);
	return FText::FromString(PreviewText);
}

// 대상 DA가 지정되어 검사를 시작할 수 있는지 반환합니다.
bool SCFVDAWizardTab::CanValidate() const
{
	if (TargetVehicleData.IsValid())
	{
		return true;
	}

	if (!TargetPathTextBox.IsValid())
	{
		return false;
	}

	return !TargetPathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty();
}

// 대상 DA가 지정되어 소켓 캡처를 시도할 수 있는지 반환합니다.
bool SCFVDAWizardTab::CanCaptureLayout() const
{
	return CanValidate() && !HasManagedAuthoringRecipe();
}

// 대상 DA가 지정되어 주행감 적용을 시도할 수 있는지 반환합니다.
bool SCFVDAWizardTab::CanApplyDrivingFeel() const
{
	return CanValidate() && !HasManagedAuthoringRecipe();
}

// Target DA를 Quick Tune 기준 수치로 되돌릴 수 있는지 반환합니다.
bool SCFVDAWizardTab::CanRevertDrivingFeel() const
{
	return CanValidate() && !HasManagedAuthoringRecipe() && bHasDrivingFeelRevertValues && !DrivingFeelRevertTargetPath.IsEmpty();
}

// Current Target이 새 Authoring Recipe에 연결되어 있는지 Common Authoring facade로 확인합니다.
bool SCFVDAWizardTab::HasManagedAuthoringRecipe(FString* OutMessage) const
{
	// Current weak Target 또는 입력 경로에서 resolve한 조회 대상입니다.
	UCFVehicleData* GuardTarget = TargetVehicleData.Get();
	// Path-only 상태의 optional load diagnostic입니다.
	FString LoadMessage;
	if (!GuardTarget && TargetPathTextBox.IsValid() && !TargetPathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty())
	{
		GuardTarget = LoadVehicleDataFromTextBox(TargetPathTextBox, true, LoadMessage);
	}
	if (!GuardTarget)
	{
		return false;
	}
	// Common Authoring facade의 managed-target read result입니다.
	FCFVehicleManagedReadResult ManagedResult;
	if (!FCFVehicleAuthoringService::ReadManagedTarget(GuardTarget, ManagedResult))
	{
		if (OutMessage)
		{
			*OutMessage = TEXT("Authoring 관리 상태를 확인하지 못했습니다. CarFight Vehicle Authoring Workspace에서 다시 확인하세요.");
		}
		return true;
	}
	if (ManagedResult.bManaged && OutMessage)
	{
		*OutMessage = FString::Printf(
			TEXT("이 Target은 Vehicle Authoring Recipe에 연결되어 있습니다: %s\nLayout/Driving Feel 변경은 CarFight Vehicle Authoring의 Preview/Apply/Undo를 사용하세요."),
			*ManagedResult.RecipePath.ToString());
	}
	return ManagedResult.bManaged;
}

// 복사할 리포트 텍스트가 있는지 반환합니다.
bool SCFVDAWizardTab::CanCopyReport() const
{
	return !ReportText.IsEmpty() && ReportText != InitialReportText;
}

// 대상 DA 에디터를 열 수 있는지 반환합니다.
bool SCFVDAWizardTab::CanOpenTarget() const
{
	return CanValidate();
}

// 기준 DA 에디터를 열 수 있는지 반환합니다.
bool SCFVDAWizardTab::CanOpenSource() const
{
	if (SourceVehicleData.IsValid())
	{
		return true;
	}

	if (!SourcePathTextBox.IsValid())
	{
		return false;
	}

	return !SourcePathTextBox->GetText().ToString().TrimStartAndEnd().IsEmpty();
}

// 대상 또는 기준 DA 약한 참조와 경로 입력칸을 함께 갱신합니다.
void SCFVDAWizardTab::SetVehicleData(TWeakObjectPtr<UCFVehicleData>& InOutVehicleData, const TSharedPtr<SEditableTextBox>& PathTextBox, UCFVehicleData* NewVehicleData)
{
	InOutVehicleData = NewVehicleData;
	if (PathTextBox.IsValid())
	{
		PathTextBox->SetText(FText::FromString(GetVehicleDataObjectPath(NewVehicleData)));
	}
}

// 텍스트 박스 경로를 기준으로 VehicleData 자산을 로드합니다.
UCFVehicleData* SCFVDAWizardTab::LoadVehicleDataFromTextBox(const TSharedPtr<SEditableTextBox>& PathTextBox, const bool bIsRequired, FString& OutMessage) const
{
	if (!PathTextBox.IsValid())
	{
		OutMessage = TEXT("VehicleData 경로 입력칸을 찾지 못했습니다.");
		return nullptr;
	}

	// 텍스트 박스에 입력된 Object Path입니다.
	const FString ObjectPathText = PathTextBox->GetText().ToString().TrimStartAndEnd();
	if (ObjectPathText.IsEmpty())
	{
		if (bIsRequired)
		{
			OutMessage = TEXT("필수 VehicleData 경로가 비어 있습니다.");
		}
		else
		{
			OutMessage.Reset();
		}

		return nullptr;
	}

	// 입력 문자열에서 만든 Soft Object Path입니다.
	const FSoftObjectPath ObjectPath(ObjectPathText);
	if (!ObjectPath.IsValid())
	{
		OutMessage = FString::Printf(TEXT("VehicleData 경로 형식이 올바르지 않습니다: %s"), *ObjectPathText);
		return nullptr;
	}

	// Object Path로 로드한 UObject입니다.
	UObject* LoadedObject = ObjectPath.TryLoad();

	// 로드된 UObject를 VehicleData로 변환한 결과입니다.
	UCFVehicleData* LoadedVehicleData = Cast<UCFVehicleData>(LoadedObject);
	if (!LoadedVehicleData)
	{
		OutMessage = FString::Printf(TEXT("경로의 자산이 CFVehicleData가 아닙니다: %s"), *ObjectPathText);
		return nullptr;
	}

	OutMessage = FString::Printf(TEXT("VehicleData를 로드했습니다: %s"), *LoadedVehicleData->GetName());
	return LoadedVehicleData;
}

// 마지막 리포트를 UI 표시용 줄바꿈 텍스트로 변환합니다.
FString SCFVDAWizardTab::BuildReportText(const FCFVDAValidationReport& InReport) const
{
	// 최종 표시 문자열입니다.
	FString ResultText;
	ResultText += FString::Printf(TEXT("Target: %s\n"), *InReport.TargetDAName);
	ResultText += FString::Printf(TEXT("Source: %s\n"), *InReport.SourceDAName);
	ResultText += FString::Printf(TEXT("Overall: %s\n"), *GetSeverityLabel(InReport.OverallSeverity));
	ResultText += FString::Printf(TEXT("Error=%d, Warning=%d, Info=%d, Blocked=%d\n\n"), InReport.ErrorCount, InReport.WarningCount, InReport.InfoCount, InReport.BlockedCount);

	if (InReport.Items.Num() == 0)
	{
		ResultText += TEXT("검사 항목이 없습니다. 현재 Validator 기준으로는 수정할 항목이 발견되지 않았습니다.\n");
		return ResultText;
	}

	for (int32 ItemIndex = 0; ItemIndex < InReport.Items.Num(); ++ItemIndex)
	{
		// 현재 표시할 검사 항목입니다.
		const FCFVDAValidationItem& Item = InReport.Items[ItemIndex];

		ResultText += FString::Printf(TEXT("[%02d] %s / %s\n"), ItemIndex + 1, *GetSeverityLabel(Item.Severity), *Item.GroupName.ToString());
		ResultText += FString::Printf(TEXT("필드: %s\n"), *Item.FieldPath);
		ResultText += FString::Printf(TEXT("이름: %s\n"), *Item.DisplayName.ToString());
		ResultText += FString::Printf(TEXT("내용: %s\n"), *Item.Message.ToString());
		ResultText += FString::Printf(TEXT("조치: %s\n\n"), *Item.RecommendedAction.ToString());
	}

	return ResultText;
}

// 검사 리포트 항목으로 클릭 가능한 결과 행 목록을 다시 구성합니다.
void SCFVDAWizardTab::RebuildResultRows(const FCFVDAValidationReport& InReport)
{
	ResultRows.Reset();
	SelectedResultRow.Reset();

	for (int32 ItemIndex = 0; ItemIndex < InReport.Items.Num(); ++ItemIndex)
	{
		// 새로 추가할 목록 행 데이터입니다.
		TSharedPtr<FCFVDARow> NewRow = MakeShared<FCFVDARow>();
		NewRow->ItemNumber = ItemIndex + 1;
		NewRow->Item = InReport.Items[ItemIndex];
		ResultRows.Add(NewRow);
	}

	if (ResultListView.IsValid())
	{
		ResultListView->RequestListRefresh();
	}
}

// 검사 결과 행 하나를 목록 표시용 한 줄 문자열로 변환합니다.
FString SCFVDAWizardTab::BuildResultRowText(const FCFVDARow& Row) const
{
	return FString::Printf(
		TEXT("[%02d] %s / %s / %s - %s"),
		Row.ItemNumber,
		*GetSeverityLabel(Row.Item.Severity),
		*Row.Item.GroupName.ToString(),
		*Row.Item.FieldPath,
		*Row.Item.Message.ToString());
}

// 현재 슬라이더 값으로 적용 예정 Movement 수치를 계산합니다.
FCFDrivingFeelValues SCFVDAWizardTab::BuildDrivingFeelValues() const
{
	// 계산에 사용할 0~1 가속감 값입니다.
	const float ClampedAccelerationValue = FMath::Clamp(AccelerationFeelValue, 0.0f, 1.0f);

	// 계산에 사용할 0~1 조향 민첩성 값입니다.
	const float ClampedSteeringValue = FMath::Clamp(SteeringFeelValue, 0.0f, 1.0f);

	// 계산에 사용할 0~1 접지감 값입니다.
	const float ClampedGripValue = FMath::Clamp(GripFeelValue, 0.0f, 1.0f);

	// 계산에 사용할 0~1 서스펜션 단단함 값입니다.
	const float ClampedSuspensionValue = FMath::Clamp(SuspensionFeelValue, 0.0f, 1.0f);

	// 최종 적용 예정 수치를 담을 구조체입니다.
	FCFDrivingFeelValues FeelValues;
	FeelValues.bUseMovementOverrides = true;
	FeelValues.EngineMaxTorque = FMath::Lerp(DrivingFeelEngineMaxTorqueMin, DrivingFeelEngineMaxTorqueMax, ClampedAccelerationValue);
	FeelValues.EngineMaxRPM = FMath::Lerp(DrivingFeelEngineMaxRPMMin, DrivingFeelEngineMaxRPMMax, ClampedAccelerationValue);
	FeelValues.ThrottleInputScale = FMath::Lerp(DrivingFeelThrottleInputScaleMin, DrivingFeelThrottleInputScaleMax, ClampedAccelerationValue);
	FeelValues.FrontWheelMaxSteerAngle = FMath::Lerp(DrivingFeelFrontSteerAngleMin, DrivingFeelFrontSteerAngleMax, ClampedSteeringValue);
	FeelValues.SteeringAngleRatio = FMath::Lerp(DrivingFeelSteeringAngleRatioMin, DrivingFeelSteeringAngleRatioMax, ClampedSteeringValue);
	FeelValues.FrontWheelFrictionForceMultiplier = FMath::Lerp(DrivingFeelWheelFrictionMin, DrivingFeelWheelFrictionMax, ClampedGripValue);
	FeelValues.RearWheelFrictionForceMultiplier = FMath::Lerp(DrivingFeelWheelFrictionMin, DrivingFeelWheelFrictionMax, ClampedGripValue);
	FeelValues.FrontWheelCorneringStiffness = FMath::Lerp(DrivingFeelCorneringStiffnessMin, DrivingFeelCorneringStiffnessMax, ClampedGripValue);
	FeelValues.RearWheelCorneringStiffness = FMath::Lerp(DrivingFeelCorneringStiffnessMin, DrivingFeelCorneringStiffnessMax, ClampedGripValue);
	FeelValues.FrontWheelSpringRate = FMath::Lerp(DrivingFeelSpringRateMin, DrivingFeelSpringRateMax, ClampedSuspensionValue);
	FeelValues.RearWheelSpringRate = FMath::Lerp(DrivingFeelSpringRateMin, DrivingFeelSpringRateMax, ClampedSuspensionValue);
	FeelValues.FrontWheelSpringPreload = FMath::Lerp(DrivingFeelSpringPreloadMin, DrivingFeelSpringPreloadMax, ClampedSuspensionValue);
	FeelValues.RearWheelSpringPreload = FMath::Lerp(DrivingFeelSpringPreloadMin, DrivingFeelSpringPreloadMax, ClampedSuspensionValue);
	return FeelValues;
}

// Target DA의 현재 Movement 값을 Quick Tune 수치 묶음으로 읽어옵니다.
FCFDrivingFeelValues SCFVDAWizardTab::BuildDrivingFeelValuesFromData(const UCFVehicleData* VehicleData) const
{
	// DA를 읽을 수 없을 때 반환할 기본 Quick Tune 수치입니다.
	FCFDrivingFeelValues FeelValues;
	if (!VehicleData)
	{
		return FeelValues;
	}

	// Target DA에서 읽은 현재 Movement 설정입니다.
	const FCFVehicleMovementConfig& MovementConfig = VehicleData->VehicleMovementConfig;
	FeelValues.bUseMovementOverrides = MovementConfig.bUseMovementOverrides;
	FeelValues.EngineMaxTorque = MovementConfig.EngineMaxTorque;
	FeelValues.EngineMaxRPM = MovementConfig.EngineMaxRPM;
	FeelValues.ThrottleInputScale = MovementConfig.ThrottleInputScale;
	FeelValues.FrontWheelMaxSteerAngle = MovementConfig.FrontWheelMaxSteerAngle;
	FeelValues.SteeringAngleRatio = MovementConfig.SteeringAngleRatio;
	FeelValues.FrontWheelFrictionForceMultiplier = MovementConfig.FrontWheelFrictionForceMultiplier;
	FeelValues.RearWheelFrictionForceMultiplier = MovementConfig.RearWheelFrictionForceMultiplier;
	FeelValues.FrontWheelCorneringStiffness = MovementConfig.FrontWheelCorneringStiffness;
	FeelValues.RearWheelCorneringStiffness = MovementConfig.RearWheelCorneringStiffness;
	FeelValues.FrontWheelSpringRate = MovementConfig.FrontWheelSpringRate;
	FeelValues.RearWheelSpringRate = MovementConfig.RearWheelSpringRate;
	FeelValues.FrontWheelSpringPreload = MovementConfig.FrontWheelSpringPreload;
	FeelValues.RearWheelSpringPreload = MovementConfig.RearWheelSpringPreload;
	return FeelValues;
}

// Target DA의 현재 Movement 값을 Quick Tune 되돌리기 기준값으로 저장합니다.
void SCFVDAWizardTab::CaptureDrivingFeelRevertValuesFromTargetData(const UCFVehicleData* VehicleData)
{
	if (!VehicleData)
	{
		bHasDrivingFeelRevertValues = false;
		DrivingFeelRevertTargetPath.Reset();
		return;
	}

	DrivingFeelRevertValues = BuildDrivingFeelValuesFromData(VehicleData);
	DrivingFeelRevertTargetPath = VehicleData->GetPathName();
	bHasDrivingFeelRevertValues = true;
}

// Target DA의 현재 Movement 값을 Quick Tune 슬라이더 값으로 역산해 반영합니다.
void SCFVDAWizardTab::SyncDrivingFeelSlidersFromTargetData(const UCFVehicleData* VehicleData)
{
	if (!VehicleData)
	{
		return;
	}

	// Target DA에서 읽은 현재 Quick Tune 수치 묶음입니다.
	const FCFDrivingFeelValues FeelValues = BuildDrivingFeelValuesFromData(VehicleData);

	SyncDrivingFeelSlidersFromValues(FeelValues);
}

// Quick Tune 수치 묶음을 슬라이더 값으로 역산해 반영합니다.
void SCFVDAWizardTab::SyncDrivingFeelSlidersFromValues(const FCFDrivingFeelValues& FeelValues)
{
	// 엔진 최대 토크 기준으로 역산한 가속감 슬라이더 값입니다.
	const float TorqueAccelerationValue = NormalizeDrivingFeelSliderValue(FeelValues.EngineMaxTorque, DrivingFeelEngineMaxTorqueMin, DrivingFeelEngineMaxTorqueMax);

	// 엔진 최대 RPM 기준으로 역산한 가속감 슬라이더 값입니다.
	const float RpmAccelerationValue = NormalizeDrivingFeelSliderValue(FeelValues.EngineMaxRPM, DrivingFeelEngineMaxRPMMin, DrivingFeelEngineMaxRPMMax);

	// 스로틀 입력 배율 기준으로 역산한 가속감 슬라이더 값입니다.
	const float ThrottleAccelerationValue = NormalizeDrivingFeelSliderValue(FeelValues.ThrottleInputScale, DrivingFeelThrottleInputScaleMin, DrivingFeelThrottleInputScaleMax);

	// 전륜 최대 조향각 기준으로 역산한 조향 민첩성 슬라이더 값입니다.
	const float SteerAngleValue = NormalizeDrivingFeelSliderValue(FeelValues.FrontWheelMaxSteerAngle, DrivingFeelFrontSteerAngleMin, DrivingFeelFrontSteerAngleMax);

	// 조향 AngleRatio 기준으로 역산한 조향 민첩성 슬라이더 값입니다.
	const float SteeringRatioValue = NormalizeDrivingFeelSliderValue(FeelValues.SteeringAngleRatio, DrivingFeelSteeringAngleRatioMin, DrivingFeelSteeringAngleRatioMax);

	// 전륜 마찰력 기준으로 역산한 접지감 슬라이더 값입니다.
	const float FrontFrictionValue = NormalizeDrivingFeelSliderValue(FeelValues.FrontWheelFrictionForceMultiplier, DrivingFeelWheelFrictionMin, DrivingFeelWheelFrictionMax);

	// 후륜 마찰력 기준으로 역산한 접지감 슬라이더 값입니다.
	const float RearFrictionValue = NormalizeDrivingFeelSliderValue(FeelValues.RearWheelFrictionForceMultiplier, DrivingFeelWheelFrictionMin, DrivingFeelWheelFrictionMax);

	// 전륜 코너링 강성 기준으로 역산한 접지감 슬라이더 값입니다.
	const float FrontCorneringValue = NormalizeDrivingFeelSliderValue(FeelValues.FrontWheelCorneringStiffness, DrivingFeelCorneringStiffnessMin, DrivingFeelCorneringStiffnessMax);

	// 후륜 코너링 강성 기준으로 역산한 접지감 슬라이더 값입니다.
	const float RearCorneringValue = NormalizeDrivingFeelSliderValue(FeelValues.RearWheelCorneringStiffness, DrivingFeelCorneringStiffnessMin, DrivingFeelCorneringStiffnessMax);

	// 전륜 스프링 강성 기준으로 역산한 서스펜션 단단함 슬라이더 값입니다.
	const float FrontSpringRateValue = NormalizeDrivingFeelSliderValue(FeelValues.FrontWheelSpringRate, DrivingFeelSpringRateMin, DrivingFeelSpringRateMax);

	// 후륜 스프링 강성 기준으로 역산한 서스펜션 단단함 슬라이더 값입니다.
	const float RearSpringRateValue = NormalizeDrivingFeelSliderValue(FeelValues.RearWheelSpringRate, DrivingFeelSpringRateMin, DrivingFeelSpringRateMax);

	// 전륜 스프링 프리로드 기준으로 역산한 서스펜션 단단함 슬라이더 값입니다.
	const float FrontSpringPreloadValue = NormalizeDrivingFeelSliderValue(FeelValues.FrontWheelSpringPreload, DrivingFeelSpringPreloadMin, DrivingFeelSpringPreloadMax);

	// 후륜 스프링 프리로드 기준으로 역산한 서스펜션 단단함 슬라이더 값입니다.
	const float RearSpringPreloadValue = NormalizeDrivingFeelSliderValue(FeelValues.RearWheelSpringPreload, DrivingFeelSpringPreloadMin, DrivingFeelSpringPreloadMax);

	AccelerationFeelValue = FMath::Clamp((TorqueAccelerationValue + RpmAccelerationValue + ThrottleAccelerationValue) / 3.0f, 0.0f, 1.0f);
	SteeringFeelValue = FMath::Clamp((SteerAngleValue + SteeringRatioValue) / 2.0f, 0.0f, 1.0f);
	GripFeelValue = FMath::Clamp((FrontFrictionValue + RearFrictionValue + FrontCorneringValue + RearCorneringValue) / 4.0f, 0.0f, 1.0f);
	SuspensionFeelValue = FMath::Clamp((FrontSpringRateValue + RearSpringRateValue + FrontSpringPreloadValue + RearSpringPreloadValue) / 4.0f, 0.0f, 1.0f);
}

// 차종 프리셋 값을 임시 슬라이더에 적용합니다.
void SCFVDAWizardTab::ApplyDrivingFeelPreset(float NewAccelerationValue, float NewSteeringValue, float NewGripValue, float NewSuspensionValue, const FString& PresetName)
{
	AccelerationFeelValue = FMath::Clamp(NewAccelerationValue, 0.0f, 1.0f);
	SteeringFeelValue = FMath::Clamp(NewSteeringValue, 0.0f, 1.0f);
	GripFeelValue = FMath::Clamp(NewGripValue, 0.0f, 1.0f);
	SuspensionFeelValue = FMath::Clamp(NewSuspensionValue, 0.0f, 1.0f);
	StatusMessage = FString::Printf(TEXT("%s 프리셋을 임시 슬라이더에 적용했습니다. DA 원본은 아직 변경되지 않았습니다."), *PresetName);
}

// 계산된 Movement 수치를 Target DA에 실제로 씁니다.
void SCFVDAWizardTab::ApplyDrivingFeelValuesToData(UCFVehicleData* VehicleData, const FCFDrivingFeelValues& FeelValues)
{
	if (!VehicleData)
	{
		return;
	}

	// 실제로 수정할 VehicleMovement 설정입니다.
	FCFVehicleMovementConfig& MutableMovementConfig = VehicleData->VehicleMovementConfig;
	MutableMovementConfig.bUseMovementOverrides = FeelValues.bUseMovementOverrides;
	MutableMovementConfig.EngineMaxTorque = FeelValues.EngineMaxTorque;
	MutableMovementConfig.EngineMaxRPM = FeelValues.EngineMaxRPM;
	MutableMovementConfig.ThrottleInputScale = FeelValues.ThrottleInputScale;
	MutableMovementConfig.FrontWheelMaxSteerAngle = FeelValues.FrontWheelMaxSteerAngle;
	MutableMovementConfig.SteeringAngleRatio = FeelValues.SteeringAngleRatio;
	MutableMovementConfig.FrontWheelFrictionForceMultiplier = FeelValues.FrontWheelFrictionForceMultiplier;
	MutableMovementConfig.RearWheelFrictionForceMultiplier = FeelValues.RearWheelFrictionForceMultiplier;
	MutableMovementConfig.FrontWheelCorneringStiffness = FeelValues.FrontWheelCorneringStiffness;
	MutableMovementConfig.RearWheelCorneringStiffness = FeelValues.RearWheelCorneringStiffness;
	MutableMovementConfig.FrontWheelSpringRate = FeelValues.FrontWheelSpringRate;
	MutableMovementConfig.RearWheelSpringRate = FeelValues.RearWheelSpringRate;
	MutableMovementConfig.FrontWheelSpringPreload = FeelValues.FrontWheelSpringPreload;
	MutableMovementConfig.RearWheelSpringPreload = FeelValues.RearWheelSpringPreload;
}

// 에디터에서 지정된 VehicleData 자산을 엽니다.
void SCFVDAWizardTab::OpenVehicleDataEditor(UCFVehicleData* VehicleData, const FString& MissingMessage)
{
	if (!VehicleData)
	{
		StatusMessage = MissingMessage.IsEmpty() ? FString(TEXT("열 VehicleData가 없습니다.")) : MissingMessage;
		return;
	}

	if (!GEditor)
	{
		StatusMessage = TEXT("GEditor가 없어 자산 에디터를 열 수 없습니다.");
		return;
	}

	// Unreal 자산 에디터 서브시스템입니다.
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		StatusMessage = TEXT("AssetEditorSubsystem을 찾지 못했습니다.");
		return;
	}

	AssetEditorSubsystem->OpenEditorForAsset(VehicleData);
	StatusMessage = FString::Printf(TEXT("VehicleData 에디터를 열었습니다: %s"), *VehicleData->GetName());
}

#undef LOCTEXT_NAMESPACE
