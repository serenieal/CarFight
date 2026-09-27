// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentBuilderTab.h
// Version: v1.4.0
// Date: 2026-09-17
// Description: CF-FQ-054 EBA-P0-05 Guided Equipment Builder USER UX correction + child DataAsset 제작 진입 UX입니다.
// Changelog:
// - v1.4.0: 기존 장비 종류 고정, 신규 에셋 이름 기반 자동 ID/경로, DisplayName 설명, compatibility 결과 인접 배치와 Turret/Weapon/Sensor DataAsset 생성 진입을 추가.
// - v1.3.0: VehicleData exact picker, MountProfile read-only combo, exact3 compatibility summary/refresh와 Vehicle Builder navigation-only action을 추가.
// - v1.2.0: Create target path, Revision 1 검토 상태, explicit `검토`와 one-shot `적용 및 저장` action을 추가.
// - v1.1.0: Weapon TurretMountData/WeaponData, Scanner VehicleSensorData transient selector와 bounded child validation presentation을 추가.
// - v1.0.0: Vehicle Builder UX 계열의 0.27/0.73 shell, EquipmentPreset browser, transient Draft identity/basic mount editor와 common scroll을 추가.
// Migration:
// - child selector는 Draft reference만 변경합니다. child 생성 버튼은 Content Browser의 신규 DataAsset 생성 흐름만 시작하며 EquipmentPreset 저장과 child 저장을 결합하지 않습니다.
// - Product mutation은 explicit `적용 및 저장`에서 선택된 exact EquipmentPreset target 하나에만 발생하며 ViewModel/CFDADurableCore authority를 사용합니다.
// - compatibility UI는 transient advisory only이며 결과를 새로고침 action 바로 아래에 표시합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleWeaponTypes.h"
#include "EquipmentAuthoring/CFEquipmentBuilderTypes.h"
#include "Widgets/SCompoundWidget.h"

class FCFEquipmentBuilderVM;
struct FAssetData;
class ITableRow;
class SScrollBox;
class STableViewBase;
template<typename OptionType> class SComboBox;
template<typename ItemType> class SListView;

// 장비 제작 흐름의 Editor-only Native Slate 진입점입니다.
class CARFIGHT_REEDITOR_API SCFEquipmentBuilderTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCFEquipmentBuilderTab) {}
	SLATE_END_ARGS()

	// Slate widget tree와 transient Equipment Builder ViewModel을 초기화합니다.
	void Construct(const FArguments& InArgs);

private:
	// EquipmentPreset browser row shared pointer 타입입니다.
	using FPresetRowPtr = TSharedPtr<FCFEquipmentPresetListEntry>;

	// VehicleData MountProfile read-only combo row shared pointer 타입입니다.
	using FMountRowPtr = TSharedPtr<FCFEquipmentMountOption>;

	// 왼쪽 작업 대상 browser와 제작 단계 영역을 구성합니다.
	TSharedRef<SWidget> BuildBrowserPanel();

	// 오른쪽 고정 상태 header + common scroll draft 영역을 구성합니다.
	TSharedRef<SWidget> BuildDraftPanel();

	// EBA P0 전체 흐름을 보여주는 단계 안내 영역을 구성합니다.
	TSharedRef<SWidget> BuildStepList() const;

	// 현재 Gate에서 편집 가능한 transient identity/basic mount 필드를 구성합니다.
	TSharedRef<SWidget> BuildDraftFields();

	// current Draft의 Weapon/Scanner child reference selector와 bounded validation 결과를 구성합니다.
	TSharedRef<SWidget> BuildReferenceSummary();

	// transient VehicleData + MountProfile compatibility probe와 navigation-only Cross-Builder UX를 구성합니다.
	TSharedRef<SWidget> BuildCompatibilityPanel();

	// Browser row 하나의 Slate 표현을 생성합니다.
	TSharedRef<ITableRow> HandleGeneratePresetRow(FPresetRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	// Browser에서 existing EquipmentPreset을 선택해 read-only source를 transient draft로 load합니다.
	void HandlePresetSelectionChanged(FPresetRowPtr SelectedItem, ESelectInfo::Type SelectInfo);

	// Product Asset을 만들지 않고 새 EquipmentPreset transient draft를 시작합니다.
	FReply HandleBeginNewPreset();

	// Data Asset Manager metadata inventory를 fresh 읽어 EquipmentPreset browser만 갱신합니다.
	FReply HandleRefreshBrowser();

	// current transient draft를 source snapshot으로 되돌립니다.
	FReply HandleResetDraft();

	// Weapon/Scanner UI-only Draft Mode를 변경합니다.
	FReply HandleSelectDraftMode(ECFEquipmentDraftMode DraftMode);

	// DisplayName 입력을 transient draft에 반영합니다.
	void HandleDisplayNameChanged(const FText& NewText);

	// 신규 Draft의 사용자용 에셋 이름을 자동 EquipmentId/Product target state에 반영합니다.
	void HandleCreateAssetNameChanged(const FText& NewText);

	// TurretMountData 신규 DataAsset 생성 흐름을 Content Browser에서 시작합니다.
	FReply HandleCreateTurretMountData();

	// WeaponData 신규 DataAsset 생성 흐름을 Content Browser에서 시작합니다.
	FReply HandleCreateWeaponData();

	// VehicleSensorData 신규 DataAsset 생성 흐름을 Content Browser에서 시작합니다.
	FReply HandleCreateSensorData();

	// 지정한 child DataAsset class의 표준 Content Browser 신규 생성 흐름을 시작합니다.
	FReply BeginChildDataAssetCreation(UClass* DataAssetClass, const FString& DefaultAssetName, const FString& PackagePath, const FString& UserLabel);

	// current Draft를 fresh 검증하고 Revision 1 Reviewed approval을 생성합니다.
	FReply HandleReviewDraft();

	// current one-shot Reviewed approval을 소비해 exact EquipmentPreset을 적용 및 저장합니다.
	FReply HandleApplyDraft();

	// RequiredMountType 선택을 transient draft에 반영합니다.
	void HandleMountTypeChanged(TSharedPtr<ECFVehicleMountType> NewValue, ESelectInfo::Type SelectInfo);

	// RequiredWeaponSize 선택을 transient draft에 반영합니다.
	void HandleWeaponSizeChanged(TSharedPtr<ECFVehicleWeaponSize> NewValue, ESelectInfo::Type SelectInfo);

	// Weapon Draft의 TurretMountData picker 변경을 transient reference에만 반영합니다.
	void HandleTurretMountChanged(const FAssetData& AssetData);

	// Weapon Draft의 WeaponData picker 변경을 transient reference에만 반영합니다.
	void HandleWeaponDataChanged(const FAssetData& AssetData);

	// Scanner Draft의 VehicleSensorData picker 변경을 transient reference에만 반영합니다.
	void HandleSensorDataChanged(const FAssetData& AssetData);

	// compatibility VehicleData exact picker 변경을 transient identity에만 반영합니다.
	void HandleCompatibilityVehicleChanged(const FAssetData& AssetData);

	// compatibility MountProfile combo 변경을 transient identity에만 반영합니다.
	void HandleCompatibilityMountChanged(FMountRowPtr NewValue, ESelectInfo::Type SelectInfo);

	// current Vehicle/Mount/Draft source를 fresh read-only compatibility로 다시 평가합니다.
	FReply HandleRefreshCompatibility();

	// registered Vehicle Builder Nomad Tab만 열고 context/business state는 전달하지 않습니다.
	FReply HandleOpenVehicleBuilder();

	// MountType combo option 한 행을 생성합니다.
	TSharedRef<SWidget> GenerateMountTypeOption(TSharedPtr<ECFVehicleMountType> Value) const;

	// WeaponSize combo option 한 행을 생성합니다.
	TSharedRef<SWidget> GenerateWeaponSizeOption(TSharedPtr<ECFVehicleWeaponSize> Value) const;

	// compatibility MountProfile combo option 한 행을 생성합니다.
	TSharedRef<SWidget> GenerateCompatibilityMountOption(FMountRowPtr Value) const;

	// browser shared rows를 ViewModel metadata projection에서 다시 만듭니다.
	void RefreshPresetItemsFromViewModel();

	// compatibility MountProfile shared rows를 ViewModel read-only projection에서 다시 만듭니다.
	void RefreshCompatibilityMountItemsFromViewModel();

	// current target/draft 전환 시 오른쪽 common scroll을 상단으로 되돌립니다.
	void ResetDraftScrollToStart();

	// current 작업 대상을 사용자용 문장으로 반환합니다.
	FText GetTargetText() const;

	// current transient draft 상태를 사용자용 문장으로 반환합니다.
	FText GetDraftStateText() const;

	// current Gate에서 사용자가 할 일을 반환합니다.
	FText GetCurrentActionText() const;

	// Product Asset mutation 여부와 draft/review 상태를 명확히 표시합니다.
	FText GetMutationSafetyText() const;

	// current Revision 1 Review/approval 상태를 사용자용 문장으로 반환합니다.
	FText GetReviewSummaryText() const;

	// current Draft가 Apply 가능한 Reviewed 상태인지 반환합니다.
	bool CanApplyReviewedDraft() const;

	// UI-only Draft Mode 버튼 label을 반환합니다.
	FText GetDraftModeButtonText(ECFEquipmentDraftMode DraftMode) const;

	// current MountType label을 반환합니다.
	FText GetCurrentMountTypeText() const;

	// current WeaponSize label을 반환합니다.
	FText GetCurrentWeaponSizeText() const;

	// current Draft의 TurretMountData object path를 picker attribute용 문자열로 반환합니다.
	FString GetTurretMountObjectPath() const;

	// current Draft의 WeaponData object path를 picker attribute용 문자열로 반환합니다.
	FString GetWeaponDataObjectPath() const;

	// current Draft의 VehicleSensorData object path를 picker attribute용 문자열로 반환합니다.
	FString GetSensorDataObjectPath() const;

	// current Draft child validation 전체를 사용자용 다중 행 텍스트로 반환합니다.
	FText GetValidationSummaryText() const;

	// compatibility VehicleData object path를 picker attribute용 문자열로 반환합니다.
	FString GetCompatibilityVehicleObjectPath() const;

	// current compatibility MountProfile combo label을 반환합니다.
	FText GetCurrentCompatibilityMountText() const;

	// exact3 compatibility state/reason/diagnostic을 사용자용 문장으로 반환합니다.
	FText GetCompatibilitySummaryText() const;

	// validation severity를 사용자용 prefix로 변환합니다.
	static FString GetValidationSeverityPrefix(ECFEquipmentValidationSeverity Severity);

	// MountType enum을 사용자용 한글 우선 label로 변환합니다.
	static FText GetMountTypeLabel(ECFVehicleMountType MountType);

	// WeaponSize enum을 사용자용 한글 우선 label로 변환합니다.
	static FText GetWeaponSizeLabel(ECFVehicleWeaponSize WeaponSize);

	// soft object path를 비어 있음/실제 path 사용자 표시로 변환합니다.
	static FString FormatReferencePath(const FSoftObjectPath& ObjectPath);

	// Equipment Builder session-local ViewModel입니다.
	TSharedPtr<FCFEquipmentBuilderVM> ViewModel;

	// Browser가 사용하는 shared row cache입니다.
	TArray<FPresetRowPtr> PresetItems;

	// Browser list widget입니다.
	TSharedPtr<SListView<FPresetRowPtr>> PresetListView;

	// compatibility MountProfile combo가 사용하는 read-only row cache입니다.
	TArray<FMountRowPtr> CompatibilityMountItems;

	// compatibility MountProfile read-only combo widget입니다.
	TSharedPtr<SComboBox<FMountRowPtr>> CompatibilityMountCombo;

	// 오른쪽 Page body의 단일 세로 scroll owner입니다.
	TSharedPtr<SScrollBox> DraftScrollBox;

	// 프로그램이 selection을 복원할 때 source draft를 다시 load하지 않도록 막는 local guard입니다.
	bool bSynchronizingSelection = false;

	// 마지막 Initialize/Refresh/Selection 결과를 사용자에게 표시합니다.
	FString LastStatusMessage;

	// MountType combo가 사용하는 전체 enum option입니다.
	TArray<TSharedPtr<ECFVehicleMountType>> MountTypeOptions;

	// WeaponSize combo가 사용하는 전체 enum option입니다.
	TArray<TSharedPtr<ECFVehicleWeaponSize>> WeaponSizeOptions;
};
