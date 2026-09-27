// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentBuilderVM.h
// Version: v1.6.0
// Date: 2026-09-17
// Description: CF-FQ-054 EquipmentPreset durable authoring, USER UX correction과 read-only Vehicle Mount compatibility ViewModel입니다.
// Changelog:
// - v1.6.0: 기존 EquipmentPreset 장비 종류 변경을 차단하고, 신규 에셋 이름에서 EquipmentId와 canonical 저장 경로를 자동 파생하는 Guided identity UX를 추가.
// - v1.5.0: localized DisplayName preserve-only 편집 보호, Scanner Utility 고정과 Weapon mount-type session cache를 추가.
// - v1.4.0: transient VehicleDataObjectPath + MountProfileId, exact3 compatibility result, fresh MountProfile lookup과 durable gate 분리 API를 추가.
// - v1.3.0: Create target, ReviewProposalDigest, complete identity union, one-shot approval와 CFDADurableCore apply orchestration을 추가.
// - v1.2.0: Draft mode별 transient selection cache를 추가해 Weapon↔Scanner 전환 뒤 이전 선택을 복원하고, active mode payload만 Draft에 projection하도록 보강.
// - v1.1.0: TurretMountData/WeaponData/VehicleSensorData transient selector setter, mode-safe payload 정리와 child validation chain을 추가.
// - v1.0.0: Data Asset Manager inventory 재사용, exact-class EquipmentPreset projection, existing read-only load, new transient draft와 identity/basic mount edit state를 추가.
// Migration:
// - explicit `적용 및 저장`만 exact EquipmentPreset target mutation을 요청하며 durable transaction sequencing은 CFDADurableCore만 사용합니다.
// - localized DisplayName preserve mode는 해당 필드를 다시 쓰지 않으며 Runtime/Product schema를 변경하지 않습니다.
// - Scanner Draft는 Builder policy상 Utility mount + size None으로 고정되며 기존 persisted Asset의 Weapon/Scanner 종류는 변경할 수 없습니다.
// - 신규 EquipmentPreset의 EquipmentId와 Product target은 사용자가 입력한 에셋 이름에서 자동 파생하며 기존 low-level SetCreateTargetObjectPath API는 Automation/durable 경로 호환용으로 유지합니다.
// - compatibility result는 authored semantic/review digest/apply readiness에 들어가지 않으며 VehicleData/Fitting/RuntimeApply mutation authority를 추가하지 않습니다.
// - child 선택과 validation은 EquipmentPreset durable authority와 분리됩니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementVM.h"
#include "EquipmentAuthoring/CFEquipmentBuilderTypes.h"

// Equipment Builder 탭 lifetime 안에서만 browser와 transient draft 상태를 소유합니다.
class CARFIGHT_REEDITOR_API FCFEquipmentBuilderVM
{
public:
	// Data Asset Manager의 metadata-only inventory를 준비하고 EquipmentPreset browser projection을 구성합니다.
	bool Initialize(FString& OutError);

	// fresh metadata inventory를 읽어 EquipmentPreset browser projection만 갱신합니다.
	bool RefreshBrowser(FString& OutError);

	// 저장된 exact EquipmentPreset 하나를 read-only load해 transient draft와 source snapshot으로 복사합니다.
	bool LoadExistingPreset(const FString& ObjectPath, FString& OutError);

	// Product target을 만들지 않고 새 EquipmentPreset transient draft를 시작합니다.
	void BeginNewPreset();

	// 현재 draft를 source snapshot 또는 새 draft 최초값으로 되돌립니다.
	void ResetDraftToSource();

	// current draft의 EquipmentId만 변경하고 Product UObject는 수정하지 않습니다.
	void SetEquipmentId(FName EquipmentId);

	// current draft의 DisplayName만 변경하고 Product UObject는 수정하지 않습니다.
	void SetDisplayName(const FText& DisplayName);

	// current draft의 DisplayName을 Equipment Builder에서 lossless 편집할 수 있는지 반환합니다.
	bool CanEditDisplayName() const { return bHasActiveDraft && !Draft.bPreserveExistingDisplayName; }

	// 신규 Draft에서만 Weapon/Scanner Draft Mode를 변경하고 Scanner의 Utility/None 고정 계약을 transient state에 반영합니다.
	void SetDraftMode(ECFEquipmentDraftMode DraftMode);

	// 현재 Draft의 Weapon/Scanner 종류를 변경할 수 있는지 반환합니다. 기존 persisted EquipmentPreset은 false입니다.
	bool CanChangeDraftMode() const { return bHasActiveDraft && bIsNewDraft; }

	// current draft의 RequiredMountType만 변경합니다. Scanner Draft는 Utility로 고정됩니다.
	void SetRequiredMountType(ECFVehicleMountType MountType);

	// current draft의 RequiredWeaponSize만 변경합니다.
	void SetRequiredWeaponSize(ECFVehicleWeaponSize WeaponSize);

	// Weapon Draft의 TurretMountData reference를 exact typed read-only selection으로 갱신합니다.
	bool SetTurretMountDataPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// Weapon Draft의 WeaponData reference를 exact typed read-only selection으로 갱신합니다.
	bool SetWeaponDataPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// Scanner Draft의 VehicleSensorData reference를 exact typed read-only selection으로 갱신합니다.
	bool SetSensorDataPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// 신규 Draft의 사용자용 에셋 이름을 받아 EquipmentId와 canonical Product target을 함께 자동 갱신합니다.
	void SetCreateAssetName(const FString& AssetName);

	// 신규 Draft의 low-level canonical Product target object path를 transient하게 갱신합니다. Automation/durable 호환용입니다.
	void SetCreateTargetObjectPath(const FString& TargetObjectPath);

	// current Draft/target/identity/child truth를 fresh 검증해 one-shot Reviewed approval을 생성합니다.
	bool ReviewCurrentDraft(FString& OutError);

	// exact Reviewed approval을 one-shot 소비하고 immediate no-yield fresh TOCTOU 뒤 CFDADurableCore로 적용 및 저장합니다.
	bool ApplyReviewedDraft(FCFEquipmentApplyReport& OutReport);

	// 신규 Draft에서 사용자가 입력한 에셋 이름을 반환합니다.
	const FString& GetCreateAssetName() const { return CreateAssetName; }

	// 신규 Draft의 transient Create target object path를 반환합니다.
	const FString& GetCreateTargetObjectPath() const { return CreateTargetObjectPath; }

	// current Create/Update Review target object path를 반환합니다.
	FString GetTargetObjectPathForReview() const;

	// current one-shot Review state를 반환합니다.
	const FCFEquipmentReviewState& GetReviewState() const { return ReviewState; }

	// current transient Draft의 package/child/reference chain을 bounded read-only validation으로 평가합니다.
	FCFEquipmentDraftValidation BuildCurrentValidation() const;

	// read-only compatibility 대상 VehicleData object path를 변경하고 이전 Mount 선택을 폐기합니다.
	void SetCompatibilityVehicleDataPath(const FSoftObjectPath& VehicleDataObjectPath);

	// current VehicleData 안에서 평가할 transient MountProfileId를 선택합니다.
	void SetCompatibilityMountProfileId(FName MountProfileId);

	// current Vehicle/Mount/Draft source를 fresh 재해석해 advisory probe를 갱신합니다.
	void RefreshCompatibilityProbe();

	// selected VehicleData의 current MountProfiles read-only projection을 반환합니다.
	const TArray<FCFEquipmentMountOption>& GetCompatibilityMountOptions() const { return CompatibilityMountOptions; }

	// 현재 transient VehicleData object path를 반환합니다.
	const FSoftObjectPath& GetCompatibilityVehicleDataPath() const { return CompatibilityVehicleDataObjectPath; }

	// 현재 transient MountProfileId를 반환합니다.
	FName GetCompatibilityMountProfileId() const { return CompatibilityMountProfileId; }

	// 마지막 fresh exact3 compatibility advisory 결과를 반환합니다.
	const FCFEquipmentCompatibilityResult& GetCompatibilityResult() const { return CompatibilityResult; }

	// 임의 transient Draft를 동일한 EBA-P0-02 validation hierarchy로 평가합니다.
	FCFEquipmentDraftValidation BuildValidationForDraft(const FCFEquipmentPresetDraft& DraftToValidate) const;

	// metadata-only EquipmentPreset browser rows를 반환합니다.
	const TArray<FCFEquipmentPresetListEntry>& GetPresetEntries() const { return PresetEntries; }

	// current transient draft를 반환합니다.
	const FCFEquipmentPresetDraft& GetDraft() const { return Draft; }

	// existing load 시점 또는 new draft 시작 시점의 source snapshot을 반환합니다.
	const FCFEquipmentPresetDraft& GetSourceDraft() const { return SourceDraft; }

	// 현재 편집할 transient draft가 준비됐는지 반환합니다.
	bool HasActiveDraft() const { return bHasActiveDraft; }

	// 현재 draft가 persisted Asset이 아닌 신규 transient draft인지 반환합니다.
	bool IsNewDraft() const { return bIsNewDraft; }

	// source snapshot 이후 transient 입력이 달라졌는지 반환합니다. durable semantic fingerprint authority가 아닙니다.
	bool HasDraftChanges() const { return bDraftDirty; }

	// current existing target object path를 반환합니다. 신규 draft에서는 비어 있습니다.
	const FString& GetSelectedObjectPath() const { return SelectedObjectPath; }

	// current draft/source 상태를 사용자용 한 줄 요약으로 반환합니다.
	FString BuildDraftStateSummary() const;

#if WITH_DEV_AUTOMATION_TESTS
	// Automation disposable root를 Product path policy 예외로 허용합니다.
	void EnableDisposableTestTargetsForAutomation(bool bEnabled) { bAllowDisposableTestTargets = bEnabled; }

	// exact child path의 persisted/clean truth를 mutation 없이 fail-closed하도록 fault injection합니다.
	void SetForcedChildStateUnconfirmedForAutomation(const FString& ObjectPath) { ForcedChildStateUnconfirmedObjectPath = ObjectPath; }

	// Reviewed digest를 corruption해 final fresh TOCTOU stale branch를 mutation 없이 검증합니다.
	void CorruptReviewedDigestForAutomation() { ReviewState.ReviewProposalDigest += TEXT("-stale"); }
#endif

private:
	// current Data Asset Manager inventory에서 exact CFEquipmentPresetData class rows만 browser DTO로 투영합니다.
	bool RebuildPresetEntries(FString& OutError);

	// exact object path가 current EquipmentPreset browser에 존재하는지 확인합니다.
	bool HasPresetEntry(const FString& ObjectPath) const;

	// persisted EquipmentPreset payload를 UI-only Weapon/Scanner mode로 해석합니다.
	static ECFEquipmentDraftMode InferDraftMode(const class UCFEquipmentPresetData& Preset);

	// current transient path를 exact UCFVehicleData로 read-only resolve합니다.
	class UCFVehicleData* ResolveCompatibilityVehicleData() const;

	// selected VehicleData의 current MountProfiles를 read-only option projection으로 다시 구성합니다.
	void RebuildCompatibilityMountOptions();

	// current Vehicle/Mount/Draft source를 exact3 compatibility result로 평가합니다.
	FCFEquipmentCompatibilityResult EvaluateCompatibilityProbe() const;

	// current Equipment Draft를 runtime EquipmentPreset mount 의미와 동일하게 read-only 평가합니다.
	bool EvaluateDraftMountContract(
		ECFVehicleMountType MountType,
		ECFVehicleWeaponSize SizeLimit,
		ECFEquipmentCompatibilityReason& OutReason,
		FString& OutDiagnostic) const;

	// validation result에 새 항목을 추가하고 Error severity면 blocking 상태도 함께 올립니다.
	static void AddValidationItem(
		FCFEquipmentDraftValidation& Validation,
		ECFEquipmentValidationSeverity Severity,
		const FString& Title,
		const FString& Message);

	// source/current draft의 edit state가 같은지 비교합니다. 이 비교는 Review semantic fingerprint를 대체하지 않습니다.
	static bool AreDraftsEquivalentForEditing(const FCFEquipmentPresetDraft& Left, const FCFEquipmentPresetDraft& Right);

	// current Draft의 active/inactive payload를 mode별 transient cache에 동기화합니다.
	void RefreshModeCachesFromDraft();

	// current draft와 source snapshot을 비교해 presentation dirty state를 갱신합니다.
	void RecomputeDraftDirty();

	// current selection과 transient draft state를 모두 해제합니다.
	void ClearCurrentDraft();

	// Draft/target semantic 변경 뒤 기존 Reviewed approval을 즉시 무효화합니다.
	void InvalidateReviewedApproval();

	// current Draft에 대해 fresh target/semantic/identity/child evidence와 current contract digest를 생성합니다.
	bool BuildFreshReviewEvidence(FCFEquipmentReviewState& OutEvidence, FString& OutError);

	// exact class inventory와 loaded-only EquipmentPreset union으로 prospective EquipmentId uniqueness를 fail-closed 검증합니다.
	bool ValidateProspectiveIdentityUniqueness(
		FName ProspectiveEquipmentId,
		const FString& TargetObjectPath,
		ECFEquipmentReviewOperation Operation,
		FString& OutError);

	// target path가 Product 또는 Automation disposable policy에 맞는 canonical path인지 검증합니다.
	bool ValidateReviewTargetPath(const FString& TargetObjectPath, FString& OutError) const;

	// shared durable core terminal result를 Equipment Builder public report로 투영합니다.
	static ECFEquipmentApplyResult MapDurableResult(uint8 DurableResultValue);

	// read-first discovery authority를 재사용하는 session-local Data Asset Manager VM입니다.
	FCFDAManagementVM DataManagementViewModel;

	// exact CFEquipmentPresetData persisted rows만 담는 browser projection입니다.
	TArray<FCFEquipmentPresetListEntry> PresetEntries;

	// 사용자가 현재 편집하는 transient exact7+UI-mode draft입니다.
	FCFEquipmentPresetDraft Draft;

	// draft가 시작된 시점의 source snapshot입니다.
	FCFEquipmentPresetDraft SourceDraft;

	// Weapon mode에서 마지막으로 선택한 TurretMountData를 session-local로 보존합니다.
	TSoftObjectPtr<class UCFTurretMountData> WeaponModeTurretMountCache;

	// Weapon mode에서 마지막으로 선택한 WeaponData를 session-local로 보존합니다.
	TSoftObjectPtr<class UCFWeaponData> WeaponModeWeaponDataCache;

	// Weapon mode에서 마지막으로 사용한 RequiredMountType을 session-local로 보존합니다.
	ECFVehicleMountType WeaponModeMountTypeCache = ECFVehicleMountType::Turret;

	// Weapon mode에서 마지막으로 사용한 RequiredWeaponSize를 session-local로 보존합니다.
	ECFVehicleWeaponSize WeaponModeSizeCache = ECFVehicleWeaponSize::Large;

	// Scanner mode에서 마지막으로 선택한 VehicleSensorData를 session-local로 보존합니다.
	TSoftObjectPtr<class UCFVehicleSensorData> ScannerModeSensorDataCache;

	// existing draft가 binding된 exact persisted object path입니다.
	FString SelectedObjectPath;

	// 신규 Draft에서 사용자가 입력하는 사람 관리용 에셋 이름입니다.
	FString CreateAssetName;

	// 신규 Draft에서 에셋 이름으로부터 자동 파생하거나 Automation이 직접 지정한 exact Create target object path입니다.
	FString CreateTargetObjectPath;

	// current one-shot Review/approval evidence입니다.
	FCFEquipmentReviewState ReviewState;

	// session-local read-only VehicleData compatibility identity입니다.
	FSoftObjectPath CompatibilityVehicleDataObjectPath;

	// session-local read-only MountProfile compatibility identity입니다.
	FName CompatibilityMountProfileId = NAME_None;

	// selected VehicleData의 current MountProfiles read-only projection입니다.
	TArray<FCFEquipmentMountOption> CompatibilityMountOptions;

	// 마지막 fresh exact3 compatibility advisory result입니다.
	FCFEquipmentCompatibilityResult CompatibilityResult;

#if WITH_DEV_AUTOMATION_TESTS
	// Product VehicleData mutation 없이 bounded size compatibility helper를 직접 검증하는 focused Automation friend입니다.
	friend class FCFEquipmentSizeMismatchTest;

	// Automation에서 exact disposable test namespace를 허용하는 명시적 opt-in입니다.
	bool bAllowDisposableTestTargets = false;

	// child state fail-closed branch를 persisted child mutation 없이 검증하는 exact test fault path입니다.
	FString ForcedChildStateUnconfirmedObjectPath;
#endif

	// current draft가 사용 가능한지 나타냅니다.
	bool bHasActiveDraft = false;

	// current draft가 신규 transient draft인지 나타냅니다.
	bool bIsNewDraft = false;

	// current draft가 source snapshot 이후 달라졌는지 나타내는 presentation-only 상태입니다.
	bool bDraftDirty = false;
};
