// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-09-03
// Description: CF-FQ-041 Runtime Apply UI + CF-FQ-044 Runtime Catalog change-aware option synchronization
// Scope: Runtime Test Catalog 선택 상태, Current/Selected 분리, Explicit Vehicle/Equipment Apply와 Catalog option cache 동기화를 소유합니다.
// Changelog:
// - v1.1.0: VRCP-P0-03. cached RuntimeApply Widget에서 Catalog Vehicle/Equipment exact sequence가 실제 변경된 경우에만 option을 재구성하도록 change-aware sync 계약을 추가.
// - v1.0.0: Asset 없는 C++ WidgetTree, Vehicle/Mount/Equipment 선택, 명시 Apply와 Runtime service 위임을 최초 추가.
// Migration:
// - v1.1.0부터 RefreshRuntimeApplyState는 Catalog Vehicle/Equipment exact sequence가 cached option과 다를 때만 해당 ComboBox를 rebuild합니다. 일반 Tick ClearOptions는 하지 않습니다.
// - 이 Widget은 Runtime 적용 규칙을 직접 구현하지 않고 FCFRuntimeVehicleApplyService / FCFRuntimeEquipApplyService만 호출합니다.
// - Equipment DisplayName이 비어 있으면 EquipmentId/AssetName으로 공개 이름을 대체하지 않고 '(공개 이름 미지정)'으로 표시합니다.
// - 향후 WBP 스타일 파생을 추가해도 선택 상태와 Explicit Apply API는 유지합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "CFRuntimeEquipApply.h"
#include "CFRuntimeVehicleApply.h"
#include "CFRuntimeApplyWidget.generated.h"

class ACFVehiclePawn;
class UButton;
class UCFEquipmentPresetData;
class UCFRuntimeTestCatalogData;
class UCFVehicleData;
class UComboBoxString;
class UTextBlock;
class UVerticalBox;

/**
 * VehicleDebug Panel 안에서 사용할 개발·시연용 Runtime Content Apply 조작 Widget입니다.
 * C++는 Catalog/선택 상태와 서비스 호출만 소유하고, 실제 Vehicle/Fitting Runtime 규칙은 Runtime Apply service에 위임합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFRuntimeApplyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] Widget 초기화 시 C++ WidgetTree와 기본 Runtime Catalog를 준비합니다.
	virtual void NativeOnInitialized() override;

	// [v1.0.0] Widget 파괴 시 ComboBox/Button delegate 바인딩을 정리합니다.
	virtual void NativeDestruct() override;

	// [v1.0.0] Runtime Apply 대상 차량 Pawn을 설정하고 Current 상태와 Mount 선택 목록을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="Runtime Apply 차량 Pawn 설정 (Set Runtime Apply Vehicle Pawn)", ToolTip="Runtime Apply UI가 현재 차량, 장착 프로파일과 적용 결과를 읽고 변경할 CFVehiclePawn을 설정합니다."))
	void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);

	// [v1.0.0] 현재 Pawn/Catalog 상태를 읽어 표시 텍스트와 버튼 가능 상태를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="Runtime Apply UI 갱신 (Refresh Runtime Apply UI)", ToolTip="현재 차량, 선택 차량, 장착 프로파일, 현재/선택 장비와 마지막 적용 결과를 다시 표시합니다."))
	void RefreshRuntimeApplyState();

	// [v1.0.0] Asset 없이 사용할 최소 Runtime Apply WidgetTree를 중복 없이 생성하고 준비 여부를 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="Runtime Apply UI 트리 보장 (Ensure Runtime Apply Tree)", ToolTip="Vehicle/Equipment 선택 ComboBox와 명시 Apply 버튼을 포함하는 C++ WidgetTree를 준비합니다."))
	bool EnsureRuntimeApplyTree();

	// [v1.0.0] Catalog Vehicle 목록에서 index 항목을 Selected Vehicle로 지정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="선택 차량 인덱스 설정 (Select Vehicle Index)", ToolTip="Runtime Catalog 차량 목록의 지정 인덱스를 Selected Vehicle로 설정합니다. 선택만 바꾸며 실제 차량에는 적용하지 않습니다."))
	bool SelectVehicleByIndex(int32 VehicleIndex);

	// [v1.0.0] 현재 차량 MountProfiles에서 index 항목을 Target Mount/Profile로 지정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="대상 Mount 인덱스 설정 (Select Mount Index)", ToolTip="현재 VehicleData의 MountProfiles 중 지정 인덱스를 Equipment Apply 대상 Mount/Profile로 선택합니다."))
	bool SelectMountByIndex(int32 MountIndex);

	// [v1.0.0] Catalog Equipment 목록에서 index 항목을 Selected Equipment로 지정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="선택 장비 인덱스 설정 (Select Equipment Index)", ToolTip="Runtime Catalog 장비 목록의 지정 인덱스를 Selected Equipment로 설정합니다. 선택만 바꾸며 실제 장비에는 적용하지 않습니다."))
	bool SelectEquipmentByIndex(int32 EquipmentIndex);

	// [v1.0.0] Selected Vehicle을 현재 Pawn에 명시적으로 Runtime Apply합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="선택 차량 적용 (Apply Selected Vehicle)", ToolTip="현재 Selected Vehicle을 Runtime Test Catalog 검증 후 같은 Pawn에 적용합니다."))
	ECFRuntimeVehicleApplyStatus ApplySelectedVehicle();

	// [v1.0.0] Selected Equipment를 Target Mount/Profile에 명시적으로 Runtime Apply합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="선택 장비 적용 (Apply Selected Equipment)", ToolTip="현재 Selected Equipment를 선택한 Mount/Profile에 Runtime Test Catalog 검증 후 적용합니다."))
	ECFRuntimeEquipApplyStatus ApplySelectedEquipment();

	// [v1.0.0] 현재 Catalog Vehicle 선택 후보 개수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="차량 선택 후보 수 (Get Vehicle Option Count)", ToolTip="현재 Runtime Test Catalog에 등록되어 UI가 선택할 수 있는 VehicleData 수입니다."))
	int32 GetVehicleOptionCount() const { return VehicleOptionDataArray.Num(); }

	// [v1.0.0] 현재 차량 Mount/Profile 선택 후보 개수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="Mount 선택 후보 수 (Get Mount Option Count)", ToolTip="현재 VehicleData에 선언되어 UI가 선택할 수 있는 MountProfile 수입니다."))
	int32 GetMountOptionCount() const { return MountOptionIdArray.Num(); }

	// [v1.0.0] 현재 Catalog Equipment 선택 후보 개수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="장비 선택 후보 수 (Get Equipment Option Count)", ToolTip="현재 Runtime Test Catalog에 등록되어 UI가 선택할 수 있는 EquipmentPresetData 수입니다."))
	int32 GetEquipmentOptionCount() const { return EquipmentOptionDataArray.Num(); }

	// [v1.0.0] 현재 Selected Vehicle 원본 Catalog Asset을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="선택 차량 반환 (Get Selected Vehicle Data)", ToolTip="아직 Apply되지 않았을 수 있는 현재 Selected VehicleData Catalog Asset을 반환합니다."))
	UCFVehicleData* GetSelectedVehicleData() const { return SelectedVehicleData.Get(); }

	// [v1.0.0] 현재 Equipment Apply 대상 MountProfileId를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="대상 Mount ID 반환 (Get Selected Mount Profile Id)", ToolTip="현재 Equipment Apply 대상으로 선택된 VehicleData MountProfileId를 반환합니다."))
	FName GetSelectedMountProfileId() const { return SelectedMountProfileId; }

	// [v1.0.0] 현재 Selected Equipment 원본 Catalog Asset을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="선택 장비 반환 (Get Selected Equipment Data)", ToolTip="아직 Apply되지 않았을 수 있는 현재 Selected EquipmentPresetData Catalog Asset을 반환합니다."))
	UCFEquipmentPresetData* GetSelectedEquipmentData() const { return SelectedEquipmentData.Get(); }

	// [v1.0.0] 마지막 Vehicle 또는 Equipment Explicit Apply 결과를 표시 문자열로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|RuntimeApply|UI", meta=(DisplayName="마지막 Runtime Apply 결과 (Get Last Runtime Apply Result)", ToolTip="가장 최근 Explicit Apply의 성공, 검증 실패, 적용 실패 또는 복구 실패 결과와 메시지를 반환합니다."))
	FString GetLastResultText() const { return LastResultText; }

	// [v1.0.0] 마지막 Vehicle Apply 결과 상태를 반환합니다.
	ECFRuntimeVehicleApplyStatus GetLastVehicleApplyStatus() const { return LastVehicleApplyResult.Status; }

	// [v1.0.0] 마지막 Equipment Apply 결과 상태를 반환합니다.
	ECFRuntimeEquipApplyStatus GetLastEquipmentApplyStatus() const { return LastEquipmentApplyResult.Status; }

private:
#if WITH_DEV_AUTOMATION_TESTS
	// VRCP-P0-03 isolated Catalog cache-sync Automation이 Product Default Catalog mutation 없이 private cache seam을 검증할 수 있게 합니다.
	friend class FCFRuntimeApplyCatalogSyncTest;
#endif

	// [v1.0.0] Config soft reference에서 기본 Runtime Test Catalog를 로드하고 선택 옵션 캐시를 준비합니다.
	bool LoadDefaultRuntimeCatalog();

	// [v1.0.0] 현재 Catalog AllowedVehicleData를 Vehicle ComboBox와 내부 option 배열에 반영합니다.
	void RebuildVehicleOptions();

	// [v1.0.0] 현재 Pawn.VehicleData의 MountProfiles를 Target Mount ComboBox와 내부 option 배열에 반영합니다.
	void RebuildMountOptions();

	// [v1.0.0] 현재 Catalog AllowedEquipmentPresetData를 Equipment ComboBox와 내부 option 배열에 반영합니다.
	void RebuildEquipmentOptions();

	// [v1.0.0] 현재 선택 MountProfileId에 해당하는 VehicleData MountProfile을 찾습니다.
	const struct FCFVehicleMountProfile* FindSelectedMountProfile() const;

	// [v1.0.0] 현재 Applied Fitting Snapshot에서 선택 Mount에 실제 적용된 EquipmentPresetData를 찾습니다.
	UCFEquipmentPresetData* FindCurrentAppliedEquipment() const;

	// [v1.0.0] VehicleData를 Runtime Apply UI의 사람이 읽을 표시 문자열로 변환합니다.
	FString BuildVehicleDisplayText(const UCFVehicleData* VehicleData) const;

	// [v1.0.0] EquipmentPresetData의 Player-facing DisplayName 계약을 지키는 표시 문자열을 생성합니다.
	FString BuildEquipmentDisplayText(const UCFEquipmentPresetData* EquipmentPresetData) const;

	// [v1.0.0] 현재 Target Mount의 ID와 LocationSlot을 함께 보여주는 표시 문자열을 생성합니다.
	FString BuildMountDisplayText(const struct FCFVehicleMountProfile& MountProfile) const;

	// [v1.0.0] 마지막 Vehicle/Equipment 결과 상태를 짧은 한국어 문자열로 변환합니다.
	FString BuildResultStatusText(bool bVehicleResult) const;

	// [v1.0.0] 현재 Current/Selected/Catalog/Compatibility/Last Result TextBlock 값을 갱신합니다.
	void RefreshDisplayTexts();

	// [v1.0.0] 현재 선택과 Catalog/Pawn 유효성에 따라 Explicit Apply 버튼 활성 상태를 갱신합니다.
	void RefreshApplyButtonStates();

	// [v1.0.0] Vehicle ComboBox 선택 변경을 SelectedVehicleData 변경으로만 반영합니다.
	UFUNCTION()
	void HandleVehicleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// [v1.0.0] Mount ComboBox 선택 변경을 SelectedMountProfileId 변경으로만 반영합니다.
	UFUNCTION()
	void HandleMountSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// [v1.0.0] Equipment ComboBox 선택 변경을 SelectedEquipmentData 변경으로만 반영합니다.
	UFUNCTION()
	void HandleEquipmentSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// [v1.0.0] Vehicle Apply 버튼 클릭을 ApplySelectedVehicle로 전달합니다.
	UFUNCTION()
	void HandleApplyVehicleClicked();

	// [v1.0.0] Equipment Apply 버튼 클릭을 ApplySelectedEquipment로 전달합니다.
	UFUNCTION()
	void HandleApplyEquipmentClicked();

	// [v1.0.0] Runtime Apply 대상 CFVehiclePawn입니다.
	UPROPERTY(Transient)
	TObjectPtr<ACFVehiclePawn> VehiclePawnRef = nullptr;

	// [v1.0.0] Config에서 로드한 명시 허용 Vehicle/Equipment Catalog입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFRuntimeTestCatalogData> RuntimeCatalog = nullptr;

	// [v1.0.0] Vehicle ComboBox index와 exact Catalog VehicleData identity를 연결하는 배열입니다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCFVehicleData>> VehicleOptionDataArray;

	// [v1.0.0] Vehicle ComboBox의 중복 없는 화면 표시 문자열 배열입니다.
	TArray<FString> VehicleOptionLabelArray;

	// [v1.0.0] Mount ComboBox index와 current VehicleData MountProfileId를 연결하는 배열입니다.
	TArray<FName> MountOptionIdArray;

	// [v1.0.0] Mount ComboBox의 중복 없는 화면 표시 문자열 배열입니다.
	TArray<FString> MountOptionLabelArray;

	// [v1.0.0] Equipment ComboBox index와 exact Catalog EquipmentPresetData identity를 연결하는 배열입니다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCFEquipmentPresetData>> EquipmentOptionDataArray;

	// [v1.0.0] Equipment ComboBox의 중복 없는 화면 표시 문자열 배열입니다.
	TArray<FString> EquipmentOptionLabelArray;

	// [v1.0.0] Apply 버튼을 누르기 전 사용자가 선택한 Catalog VehicleData입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleData> SelectedVehicleData = nullptr;

	// [v1.0.0] Equipment Apply 대상으로 사용자가 선택한 current VehicleData MountProfileId입니다.
	FName SelectedMountProfileId = NAME_None;

	// [v1.0.0] Apply 버튼을 누르기 전 사용자가 선택한 Catalog EquipmentPresetData입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFEquipmentPresetData> SelectedEquipmentData = nullptr;

	// [v1.0.0] Mount option 재생성이 필요한지 판단할 마지막 관측 Pawn.VehicleData identity입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleData> LastObservedVehicleData = nullptr;

	// [v1.0.0] UI에서 마지막으로 성공 적용한 persistent Catalog VehicleData 원본입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleData> LastAppliedVehicleSourceData = nullptr;

	// [v1.0.0] UI에서 마지막 Vehicle Apply 후 Pawn이 실제 사용하는 transient VehicleData 사본입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleData> LastAppliedVehicleRuntimeData = nullptr;

	// [v1.0.0] 가장 최근 Vehicle Explicit Apply의 구조화 결과입니다.
	FCFRuntimeVehicleApplyResult LastVehicleApplyResult;

	// [v1.0.0] 가장 최근 Equipment Explicit Apply의 구조화 결과입니다.
	FCFRuntimeEquipApplyResult LastEquipmentApplyResult;

	// [v1.0.0] 화면에 표시할 마지막 Explicit Apply 결과 문자열입니다.
	FString LastResultText = TEXT("아직 적용하지 않음");

	// [v1.0.0] Runtime Apply 조작을 세로로 배치하는 C++ WidgetTree root입니다.
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RootVerticalBox = nullptr;

	// [v1.0.0] Catalog 상태 요약을 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CatalogSummaryText = nullptr;

	// [v1.0.0] 현재 Pawn VehicleData와 transient 여부를 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentVehicleText = nullptr;

	// [v1.0.0] 아직 Apply되지 않을 수 있는 Selected Vehicle을 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectedVehicleText = nullptr;

	// [v1.0.0] Catalog Vehicle 선택용 ComboBox입니다.
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> VehicleComboBox = nullptr;

	// [v1.0.0] Selected Vehicle을 명시 적용하는 버튼입니다.
	UPROPERTY(Transient)
	TObjectPtr<UButton> ApplyVehicleButton = nullptr;

	// [v1.0.0] 현재 Equipment Apply 대상 Mount/Profile을 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentMountText = nullptr;

	// [v1.0.0] 현재 VehicleData Mount/Profile 선택용 ComboBox입니다.
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> MountComboBox = nullptr;

	// [v1.0.0] 현재 Applied Snapshot의 선택 Mount 장비를 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurrentEquipmentText = nullptr;

	// [v1.0.0] 아직 Apply되지 않을 수 있는 Selected Equipment를 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectedEquipmentText = nullptr;

	// [v1.0.0] Catalog Equipment 선택용 ComboBox입니다.
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> EquipmentComboBox = nullptr;

	// [v1.0.0] Selected Equipment가 현재 Mount 규칙과 호환되는지 미리 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EquipmentCompatibilityText = nullptr;

	// [v1.0.0] Selected Equipment를 Target Mount에 명시 적용하는 버튼입니다.
	UPROPERTY(Transient)
	TObjectPtr<UButton> ApplyEquipmentButton = nullptr;

	// [v1.0.0] 마지막 Explicit Apply 결과와 복구 상태를 표시하는 Text입니다.
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LastResultTextBlock = nullptr;
};
