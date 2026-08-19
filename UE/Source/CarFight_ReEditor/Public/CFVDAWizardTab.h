// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.8.0
// Date: 2026-08-18
// Description: P0-10 parity 기간 동안 유지되는 Legacy Vehicle DA 입력 보조용 Editor Slate 탭입니다.
// Scope: Content Browser 선택 CFVehicleData를 Target/Source로 지정하고 검증 리포트를 표시합니다.
// Changelog:
// - v1.8.0: Authoring Recipe가 연결된 Target에서는 legacy Layout/Quick Tune/Revert 변경 동작을 비활성화하고 read-only 기능은 유지.
// - v1.7.0: 차체 소켓 캡처 Wizard 문구를 차량 레이아웃 통합 캡처 기준으로 갱신.
// - v1.6.0: 주행감 Quick Tune 되돌리기 버튼과 Target DA 기준값 스냅샷을 추가.
// - v1.5.0: Target DA 선택/로드 시 현재 Movement 값으로 Quick Tune 슬라이더를 역동기화.
// - v1.4.0: 가속감 Quick Tune이 ThrottleInputScale을 함께 계산하도록 확장.
// - v1.3.0: 차종별 주행감 Quick Tune 프리셋, 슬라이더, DA 적용 버튼을 추가.
// - v1.2.0: Target DA 차체 소켓에서 휠 레이아웃을 캡처하는 Wizard 버튼을 추가.
// - v1.1.0: 검사 항목 선택 목록과 선택 항목 상세 패널을 추가.
// - v1.0.0: Target/Source 선택, 전체 검사 실행, 리포트 복사, DA 에디터 열기 UI를 추가.
// Migration:
// - EUW 자산 없이 C++ Editor 탭으로 먼저 제공한다.
// - 향후 EUW_VDAWizard를 만들 때도 검증 로직은 UCFVDAValidator를 계속 사용한다.

#pragma once

#include "CoreMinimal.h"
#include "CFVDAValidator.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class SEditableTextBox;
class UCFVehicleData;

struct FCFVDARow
{
	// 검사 리포트 안에서 표시할 1 기반 순번입니다.
	int32 ItemNumber = 0;

	// 선택 목록에 표시할 실제 Validator 검사 항목입니다.
	FCFVDAValidationItem Item;
};

struct FCFDrivingFeelValues
{
	// 적용 예정 VehicleMovementConfig 사용 여부입니다.
	bool bUseMovementOverrides = true;

	// 적용 예정 엔진 최대 토크입니다.
	float EngineMaxTorque = 750.0f;

	// 적용 예정 엔진 최대 RPM입니다.
	float EngineMaxRPM = 7000.0f;

	// 적용 예정 스로틀 입력 배율입니다.
	float ThrottleInputScale = 1.0f;

	// 적용 예정 전륜 최대 조향각입니다.
	float FrontWheelMaxSteerAngle = 35.0f;

	// 적용 예정 조향 Angle Ratio입니다.
	float SteeringAngleRatio = 0.7f;

	// 적용 예정 전륜 마찰력 배수입니다.
	float FrontWheelFrictionForceMultiplier = 2.0f;

	// 적용 예정 후륜 마찰력 배수입니다.
	float RearWheelFrictionForceMultiplier = 2.0f;

	// 적용 예정 전륜 코너링 강성입니다.
	float FrontWheelCorneringStiffness = 1000.0f;

	// 적용 예정 후륜 코너링 강성입니다.
	float RearWheelCorneringStiffness = 1000.0f;

	// 적용 예정 전륜 스프링 강성입니다.
	float FrontWheelSpringRate = 250.0f;

	// 적용 예정 후륜 스프링 강성입니다.
	float RearWheelSpringRate = 250.0f;

	// 적용 예정 전륜 스프링 프리로드입니다.
	float FrontWheelSpringPreload = 50.0f;

	// 적용 예정 후륜 스프링 프리로드입니다.
	float RearWheelSpringPreload = 50.0f;
};

class SCFVDAWizardTab : public SCompoundWidget
{
	friend class FCFVDAWizardTestAccess;

public:
	SLATE_BEGIN_ARGS(SCFVDAWizardTab) {}
	SLATE_END_ARGS()

	// Vehicle DA Wizard 탭의 Slate 레이아웃과 초기 상태를 구성합니다.
	void Construct(const FArguments& InArgs);

private:
	// 검사 대상 VehicleData 약한 참조입니다.
	TWeakObjectPtr<UCFVehicleData> TargetVehicleData;

	// 비교 기준 VehicleData 약한 참조입니다.
	TWeakObjectPtr<UCFVehicleData> SourceVehicleData;

	// 대상 DA 경로를 표시하고 수동 입력을 받을 텍스트 박스입니다.
	TSharedPtr<SEditableTextBox> TargetPathTextBox;

	// 기준 DA 경로를 표시하고 수동 입력을 받을 텍스트 박스입니다.
	TSharedPtr<SEditableTextBox> SourcePathTextBox;

	// 검사 결과 항목을 클릭 가능한 목록으로 표시하는 Slate 리스트입니다.
	TSharedPtr<SListView<TSharedPtr<FCFVDARow>>> ResultListView;

	// 마지막으로 실행한 검증 리포트입니다.
	FCFVDAValidationReport LastReport;

	// 검사 결과 리스트에 표시할 행 데이터입니다.
	TArray<TSharedPtr<FCFVDARow>> ResultRows;

	// 현재 사용자가 선택한 검사 결과 행입니다.
	TSharedPtr<FCFVDARow> SelectedResultRow;

	// 가속감 슬라이더의 현재 임시값입니다.
	float AccelerationFeelValue = 0.5f;

	// 조향 민첩성 슬라이더의 현재 임시값입니다.
	float SteeringFeelValue = 0.5f;

	// 접지감 슬라이더의 현재 임시값입니다.
	float GripFeelValue = 0.55f;

	// 서스펜션 단단함 슬라이더의 현재 임시값입니다.
	float SuspensionFeelValue = 0.45f;

	// Quick Tune 되돌리기에 사용할 Target DA 기준 Movement 수치입니다.
	FCFDrivingFeelValues DrivingFeelRevertValues;

	// 되돌리기에 사용할 기준 Movement 수치가 준비되어 있는지 여부입니다.
	bool bHasDrivingFeelRevertValues = false;

	// 되돌리기 기준 수치를 캡처한 Target DA 자산 경로입니다.
	FString DrivingFeelRevertTargetPath;

	// 탭 상단 상태 안내 문구입니다.
	FString StatusMessage;

	// 리포트 영역에 표시할 줄바꿈 포함 텍스트입니다.
	FString ReportText;

	// Content Browser 선택 자산을 대상 DA로 지정합니다.
	FReply HandleSetTargetFromSelectionClicked();

	// Content Browser 선택 자산을 기준 DA로 지정합니다.
	FReply HandleSetSourceFromSelectionClicked();

	// 기준 DA 선택을 비웁니다.
	FReply HandleClearSourceClicked();

	// 대상 DA를 전체 검사하고 결과 텍스트를 갱신합니다.
	FReply HandleValidateClicked();

	// 대상 DA의 차체 소켓에서 차량 레이아웃 값을 캡처합니다.
	FReply HandleCaptureLayoutClicked();

	// 세단형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
	FReply HandleSedanPresetClicked();

	// SUV형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
	FReply HandleSuvPresetClicked();

	// 스포츠형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
	FReply HandleSportsPresetClicked();

	// 무거운 차량형 주행감 프리셋을 임시 슬라이더 값에 적용합니다.
	FReply HandleHeavyPresetClicked();

	// 임시 주행감 슬라이더 값을 Target DA의 MovementConfig에 적용합니다.
	FReply HandleApplyDrivingFeelClicked();

	// Target DA를 Quick Tune 기준 수치로 되돌립니다.
	FReply HandleRevertDrivingFeelClicked();

	// 마지막 검사 리포트 텍스트를 클립보드에 복사합니다.
	FReply HandleCopyReportClicked();

	// 대상 DA 자산 에디터를 엽니다.
	FReply HandleOpenTargetClicked();

	// 기준 DA 자산 에디터를 엽니다.
	FReply HandleOpenSourceClicked();

	// 검사 결과 행 Slate 위젯을 생성합니다.
	TSharedRef<ITableRow> HandleGenerateResultRow(TSharedPtr<FCFVDARow> RowItem, const TSharedRef<STableViewBase>& OwnerTable);

	// 검사 결과 목록에서 선택한 항목을 상세 패널 상태로 반영합니다.
	void HandleResultSelectionChanged(TSharedPtr<FCFVDARow> SelectedRow, ESelectInfo::Type SelectInfo);

	// 가속감 슬라이더 변경값을 임시 상태에 반영합니다.
	void HandleAccelerationFeelChanged(float NewValue);

	// 조향 민첩성 슬라이더 변경값을 임시 상태에 반영합니다.
	void HandleSteeringFeelChanged(float NewValue);

	// 접지감 슬라이더 변경값을 임시 상태에 반영합니다.
	void HandleGripFeelChanged(float NewValue);

	// 서스펜션 단단함 슬라이더 변경값을 임시 상태에 반영합니다.
	void HandleSuspensionFeelChanged(float NewValue);

	// 상태 안내 문구를 Slate 표시용 텍스트로 반환합니다.
	FText GetStatusMessageText() const;

	// 대상 DA 표시 문자열을 반환합니다.
	FText GetTargetVehicleDataText() const;

	// 기준 DA 표시 문자열을 반환합니다.
	FText GetSourceVehicleDataText() const;

	// 마지막 리포트 요약 문자열을 반환합니다.
	FText GetReportSummaryText() const;

	// 마지막 리포트 본문 문자열을 반환합니다.
	FText GetReportText() const;

	// 현재 선택한 검사 항목의 상세 설명을 반환합니다.
	FText GetSelectedResultDetailText() const;

	// 가속감 슬라이더 값을 반환합니다.
	float GetAccelerationFeelValue() const;

	// 조향 민첩성 슬라이더 값을 반환합니다.
	float GetSteeringFeelValue() const;

	// 접지감 슬라이더 값을 반환합니다.
	float GetGripFeelValue() const;

	// 서스펜션 단단함 슬라이더 값을 반환합니다.
	float GetSuspensionFeelValue() const;

	// 가속감 슬라이더 표시 텍스트를 반환합니다.
	FText GetAccelerationFeelText() const;

	// 조향 민첩성 슬라이더 표시 텍스트를 반환합니다.
	FText GetSteeringFeelText() const;

	// 접지감 슬라이더 표시 텍스트를 반환합니다.
	FText GetGripFeelText() const;

	// 서스펜션 단단함 슬라이더 표시 텍스트를 반환합니다.
	FText GetSuspensionFeelText() const;

	// 주행감 슬라이더가 만들 적용 예정 Movement 수치 텍스트를 반환합니다.
	FText GetDrivingFeelPreviewText() const;

	// 대상 DA가 지정되어 검사를 시작할 수 있는지 반환합니다.
	bool CanValidate() const;

	// 대상 DA가 지정되어 소켓 캡처를 시도할 수 있는지 반환합니다.
	bool CanCaptureLayout() const;

	// 대상 DA가 지정되어 주행감 적용을 시도할 수 있는지 반환합니다.
	bool CanApplyDrivingFeel() const;

		// Target DA를 Quick Tune 기준 수치로 되돌릴 수 있는지 반환합니다.
	bool CanRevertDrivingFeel() const;

	// Current Target이 새 Authoring Recipe에 연결되어 있는지 Common Authoring facade로 확인합니다.
	bool HasManagedAuthoringRecipe(FString* OutMessage = nullptr) const;


	// 복사할 리포트 텍스트가 있는지 반환합니다.
	bool CanCopyReport() const;

	// 대상 DA 에디터를 열 수 있는지 반환합니다.
	bool CanOpenTarget() const;

	// 기준 DA 에디터를 열 수 있는지 반환합니다.
	bool CanOpenSource() const;

	// 대상 또는 기준 DA 약한 참조와 경로 입력칸을 함께 갱신합니다.
	void SetVehicleData(TWeakObjectPtr<UCFVehicleData>& InOutVehicleData, const TSharedPtr<SEditableTextBox>& PathTextBox, UCFVehicleData* NewVehicleData);

	// 텍스트 박스 경로를 기준으로 VehicleData 자산을 로드합니다.
	UCFVehicleData* LoadVehicleDataFromTextBox(const TSharedPtr<SEditableTextBox>& PathTextBox, const bool bIsRequired, FString& OutMessage) const;

	// 마지막 리포트를 UI 표시용 줄바꿈 텍스트로 변환합니다.
	FString BuildReportText(const FCFVDAValidationReport& InReport) const;

	// 검사 리포트 항목으로 클릭 가능한 결과 행 목록을 다시 구성합니다.
	void RebuildResultRows(const FCFVDAValidationReport& InReport);

	// 검사 결과 행 하나를 목록 표시용 한 줄 문자열로 변환합니다.
	FString BuildResultRowText(const FCFVDARow& Row) const;

	// 현재 슬라이더 값으로 적용 예정 Movement 수치를 계산합니다.
	FCFDrivingFeelValues BuildDrivingFeelValues() const;

	// Target DA의 현재 Movement 값을 Quick Tune 수치 묶음으로 읽어옵니다.
	FCFDrivingFeelValues BuildDrivingFeelValuesFromData(const UCFVehicleData* VehicleData) const;

	// Target DA의 현재 Movement 값을 Quick Tune 되돌리기 기준값으로 저장합니다.
	void CaptureDrivingFeelRevertValuesFromTargetData(const UCFVehicleData* VehicleData);

	// Target DA의 현재 Movement 값을 Quick Tune 슬라이더 값으로 역산해 반영합니다.
	void SyncDrivingFeelSlidersFromTargetData(const UCFVehicleData* VehicleData);

	// Quick Tune 수치 묶음을 슬라이더 값으로 역산해 반영합니다.
	void SyncDrivingFeelSlidersFromValues(const FCFDrivingFeelValues& FeelValues);

	// 차종 프리셋 값을 임시 슬라이더에 적용합니다.
	void ApplyDrivingFeelPreset(float NewAccelerationValue, float NewSteeringValue, float NewGripValue, float NewSuspensionValue, const FString& PresetName);

	// 계산된 Movement 수치를 Target DA에 실제로 씁니다.
	void ApplyDrivingFeelValuesToData(UCFVehicleData* VehicleData, const FCFDrivingFeelValues& FeelValues);

	// 에디터에서 지정된 VehicleData 자산을 엽니다.
	void OpenVehicleDataEditor(UCFVehicleData* VehicleData, const FString& MissingMessage);
};
