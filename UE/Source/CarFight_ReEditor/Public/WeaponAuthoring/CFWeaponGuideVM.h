// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideVM.h
// Version: v1.4.1
// Date: 2026-09-19
// Description: CF-FQ-055 Weapon Equipment Authoring Guide의 Stable Step projection, durable exact5 graph, completion semantic integrity와 navigation-only handoff ViewModel입니다.
// Changelog:
// - v1.4.1: WEA-P0-04 Mid-review P1 교정으로 P0-03 Bundle fingerprint와 분리된 CompletionSemanticFingerprint를 추가해 persisted exact5 + FireMode/Capability 의미를 fail-closed 검증합니다.
// - v1.4.0: WEA-P0-04 completion Bundle integrity 재검증, USER completion summary와 Equipment Builder navigation-only handoff contract를 추가했습니다.
// - v1.3.0: WEA-P0-03 Damage/Ammo existing provider durable Create, exact5 child fresh preflight, session partial-durable recovery, final reference graph readback과 test-only confirmed interruption seam을 추가했습니다.
// - v1.2.0: WEA-P0-02 Damage/Ammo CreateNew Draft를 기존 CFDADamageProvider/CFDAAmmoProvider typed payload·fingerprint·serializer 계약에 연결하고 mutation0 provider preview를 추가했습니다.
// - v1.1.0: fixed exact8 navigation을 Stable StepId + VisibleSteps projection으로 교체하고 Template/Capability topology, 분리된 Projectile/Damage/Ammo/Launcher/Propulsion/MissileFlight/Guidance/Heat/Charge validation 계약을 추가했습니다.
// - v1.0.0: exact8 Wizard navigation, deterministic naming, existing child selection, fail-closed validation, completion bundle와 exact3 Create-only durable 진입 계약 추가.
// Migration:
// - v1.0.0의 fixed index 증가/감소와 PayloadChain/OptionalFeatures validator는 더 이상 사용하지 않습니다.
// - EquipmentPreset 저장은 수행하지 않습니다. 완료 Bundle만 만들고 최종 EquipmentPreset Review/Apply는 기존 Equipment Builder가 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAuthoring/CFWeaponGuideTypes.h"

// 무장 제작 가이드의 Editor-only transient 상태와 durable child Create를 관리합니다.
class CARFIGHT_REEDITOR_API FCFWeaponGuideVM
{
public:
	// 새 빈 무장 제작 세션으로 모든 transient state를 초기화합니다.
	void Reset();

	// 현재 semantic Stable StepId를 반환합니다.
	ECFWeaponGuideStep GetCurrentStep() const { return CurrentStep; }

	// 현재 Draft/Capability에서 사용자에게 보이는 Step projection을 반환합니다.
	TArray<ECFWeaponGuideStep> GetVisibleSteps() const;

	// 지정한 Stable StepId가 현재 projection에 보이는지 반환합니다.
	bool IsStepVisible(ECFWeaponGuideStep Step) const;

	// 현재 전체 transient draft를 읽기 전용으로 반환합니다.
	const FCFWeaponGuideDraft& GetDraft() const { return Draft; }

	// Slate 입력 위젯이 현재 transient draft의 Guided 값만 수정할 수 있도록 mutable reference를 반환합니다.
	FCFWeaponGuideDraft& GetMutableDraft() { return Draft; }

	// 현재 완료 Bundle을 반환합니다.
	const FCFWeaponGuideResultBundle& GetResultBundle() const { return ResultBundle; }

	// 현재 마지막 상태/실패 메시지를 반환합니다.
	const FString& GetLastStatusMessage() const { return LastStatusMessage; }

	// 사용자 기본 이름을 정규화하고 모든 신규 child target/ID 파생 상태를 갱신합니다.
	void SetBaseAssetName(const FString& BaseAssetName);

	// 게임 표시 이름 제안값만 transient draft에 반영합니다.
	void SetSuggestedDisplayName(const FString& SuggestedDisplayName);

	// 제작 Template을 선택하고 일반적인 Capability 기본값을 제안합니다.
	void SetTemplate(ECFWeaponGuideTemplate Template);

	// RequiredMountType을 변경하고 신규 Weapon의 compatible mount 의미와 동기화합니다.
	void SetRequiredMountType(ECFVehicleMountType RequiredMountType);

	// RequiredWeaponSize를 변경하고 신규 WeaponData의 WeaponSize 의미와 동기화합니다.
	void SetRequiredWeaponSize(ECFVehicleWeaponSize RequiredWeaponSize);

	// Capability나 FireMode 등 visible topology를 바꾸는 Draft 수정 뒤 current Step을 안전한 visible Step에 수렴시킵니다.
	void NotifyDraftTopologyChanged();

	// 기존 TurretMountData exact object path를 read-only reuse 대상으로 선택합니다.
	bool SetExistingTurretPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// 기존 WeaponData exact object path를 read-only reuse 대상으로 선택합니다.
	bool SetExistingWeaponPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// 기존 ProjectileData exact object path를 read-only reuse 대상으로 선택합니다.
	bool SetExistingProjectilePath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// 기존 DamageData exact object path를 Projectile damage source로 선택합니다.
	bool SetDamageDataPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// 기존 AmmoData exact object path를 finite ammo source로 선택합니다.
	bool SetAmmoDataPath(const FSoftObjectPath& ObjectPath, FString& OutError);

	// 이전 visible Step으로 이동할 수 있는지 반환합니다.
	bool CanMovePrevious() const;

	// 현재 Step validation을 통과해 다음 visible Step으로 이동할 수 있는지 반환합니다.
	bool CanMoveNext(FString& OutError) const;

	// 이전 visible Step으로 이동합니다.
	bool MovePrevious();

	// 현재 Step validation을 통과한 뒤 다음 visible Step으로 이동합니다.
	bool MoveNext(FString& OutError);

	// 지정한 Stable StepId에 필요한 fail-closed validation을 수행합니다.
	bool ValidateStep(ECFWeaponGuideStep Step, TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// 완료 전에 현재 visible workflow와 final reference graph를 fresh 검증합니다.
	bool ValidateAll(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// 현재 validation을 사용자용 여러 줄 문자열로 구성합니다.
	FString BuildValidationSummary() const;

	// 신규 Damage Draft를 existing CFDADamageProvider의 typed payload/serializer/fingerprint 계약으로 mutation0 검토합니다.
	bool BuildDamageProviderPreview(FString& OutSummary, FString& OutError) const;

	// 신규 Ammo Draft를 existing CFDAAmmoProvider의 typed payload/serializer/fingerprint 계약으로 mutation0 검토합니다.
	bool BuildAmmoProviderPreview(FString& OutSummary, FString& OutError) const;

	// 현재 draft에서 신규 child를 exact Create-only durable 적용하고 완료 Bundle을 확정합니다.
	bool CreateChildrenAndBuildResult(FString& OutError);

	// 확정된 completion Bundle을 다시 fingerprint 검증하고 사용자 중심 결과 요약을 구성합니다.
	bool BuildCompletionUserSummary(FString& OutSummary, FString& OutError) const;

	// 확정된 completion Bundle에서 Equipment Builder navigation-only 인계 계약을 구성합니다.
	bool BuildEquipmentBuilderHandoff(FCFWeaponGuideEquipmentHandoff& OutHandoff, FString& OutError) const;

	// Product 신규 TurretMountData object path를 반환합니다.
	FString GetDerivedTurretObjectPath() const;

	// Product 신규 WeaponData object path를 반환합니다.
	FString GetDerivedWeaponObjectPath() const;

	// Product 신규 ProjectileData object path를 반환합니다.
	FString GetDerivedProjectileObjectPath() const;

	// Product 신규 DamageData object path를 반환합니다.
	FString GetDerivedDamageObjectPath() const;

	// Product 신규 AmmoData object path를 반환합니다.
	FString GetDerivedAmmoObjectPath() const;

	// 신규 TurretMountData의 자동 logical ID를 반환합니다.
	FName GetDerivedTurretId() const;

	// 신규 WeaponData의 자동 logical ID를 반환합니다.
	FName GetDerivedWeaponId() const;

	// 신규 ProjectileData의 자동 logical ID를 반환합니다.
	FName GetDerivedProjectileId() const;

	// 신규 DamageData의 자동 logical ID를 반환합니다.
	FName GetDerivedDamageId() const;

	// 신규 AmmoData의 자동 logical ID를 반환합니다.
	FName GetDerivedAmmoId() const;

	// 현재 단계의 사용자 표시 이름을 반환합니다.
	static FString GetStepDisplayName(ECFWeaponGuideStep Step);

#if WITH_DEV_AUTOMATION_TESTS
	// Automation fixture가 Product namespace 대신 disposable root를 사용하도록 설정합니다.
	void SetDisposableTestRoot(const FString& TestRoot);

	// Confirmed durable child 직후 의도적 중단을 주입해 same-session partial recovery를 검증합니다.
	void SetDurableInterruptionStepForTests(ECFWeaponGuideStep Step);

	// Automation partial-recovery 중단 주입을 해제합니다.
	void ClearDurableInterruptionForTests();
#endif

private:
	// current Draft에서 visible semantic StepId를 deterministic 순서로 계산합니다.
	void BuildVisibleSteps(TArray<ECFWeaponGuideStep>& OutSteps) const;

	// current Step이 projection에서 사라졌을 때 nearest visible Step으로 수렴시킵니다.
	void NormalizeCurrentStepToVisible();

	// 사용자 입력 기본 이름이 Unreal asset leaf name으로 안전한지 검증합니다.
	bool ValidateBaseAssetName(FString& OutError) const;

	// exact typed existing child가 persisted clean state인지 검증합니다.
	bool ValidateExistingChild(
		const FSoftObjectPath& ObjectPath,
		UClass* ExpectedClass,
		const FString& UserLabel,
		FString& OutError) const;

	// 신규 child target이 memory/disk 어디에도 없는지 fail-closed 검증합니다.
	bool ValidateCreateTargetAbsent(const FString& ObjectPath, FString& OutError) const;

	// 현재 Turret 선택/신규 draft를 검증합니다.
	bool ValidateTurretDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// 현재 Weapon 선택/신규 draft와 mount/size/finite-ammo 계약을 검증합니다.
	bool ValidateWeaponDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// physical 또는 virtual ProjectileData draft를 검증합니다.
	bool ValidateProjectileDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// DamageData create/reuse draft를 검증합니다.
	bool ValidateDamageDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// finite ammo에서 AmmoData create/reuse draft를 검증합니다.
	bool ValidateAmmoDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// Launcher conditional draft를 검증합니다.
	bool ValidateLauncherDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// Projectile propulsion conditional draft를 검증합니다.
	bool ValidatePropulsionDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// Missile flight conditional draft를 검증합니다.
	bool ValidateMissileFlightDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// Missile guidance conditional draft를 검증합니다.
	bool ValidateGuidanceDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// Weapon heat conditional draft를 검증합니다.
	bool ValidateHeatDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// Weapon charge conditional draft를 검증합니다.
	bool ValidateChargeDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// existing Weapon/Projectile가 Guide에서 선택한 child reference와 exact 일치하는지 검증합니다.
	bool ValidateExistingReferenceConsistency(TArray<FCFWeaponGuideIssue>& OutIssues) const;

	// 현재 draft의 resolved final child path를 사용해 deterministic completion Bundle을 계산합니다.
	bool BuildResultBundle(FCFWeaponGuideResultBundle& OutBundle, FString& OutError) const;

	// 현재 draft 의미와 final child references를 canonical SHA-256 Bundle fingerprint로 계산합니다.
	bool BuildBundleFingerprint(const FCFWeaponGuideResultBundle& Bundle, FString& OutFingerprint, FString& OutError) const;

	// P0-03 Bundle fingerprint를 변경하지 않고 persisted exact5 + 완료 USER 의미를 별도 semantic fingerprint로 계산합니다.
	bool BuildCompletionSemanticFingerprint(
		const FCFWeaponGuideResultBundle& Bundle,
		FString& OutFingerprint,
		FString& OutError) const;

	// completion summary/handoff 전에 Bundle identity, persisted graph와 P0-04 semantic fingerprint 불변성을 fail-closed 검증합니다.
	bool ValidateCompletionBundleIntegrity(FString& OutError) const;

	// Multi-asset mutation 전에 모든 새 target과 이미 confirmed된 session child를 fresh fail-closed 검증합니다.
	bool ValidateFreshDurablePreflight(FString& OutError) const;

	// 모든 durable child의 persisted clean state와 Projectile→Damage / Weapon→Projectile/Ammo exact graph를 readback합니다.
	bool ValidateDurableReferenceGraph(
		const FSoftObjectPath& TurretPath,
		const FSoftObjectPath& DamagePath,
		const FSoftObjectPath& AmmoPath,
		const FSoftObjectPath& ProjectilePath,
		const FSoftObjectPath& WeaponPath,
		FString& OutError) const;

	// 신규 DamageData를 existing reviewed-mutation provider authority로 exact Create합니다.
	bool CreateDamageAsset(FSoftObjectPath& OutObjectPath, FString& OutError) const;

	// 신규 finite AmmoData를 existing reviewed-mutation provider authority로 exact Create합니다.
	bool CreateAmmoAsset(FSoftObjectPath& OutObjectPath, FString& OutError) const;

	// 신규 TurretMountData를 existing shared durable core로 exact Create합니다.
	bool CreateTurretAsset(FSoftObjectPath& OutObjectPath, FString& OutError) const;

	// 신규 ProjectileData를 existing shared durable core로 exact Create합니다.
	bool CreateProjectileAsset(const FSoftObjectPath& DamagePath, FSoftObjectPath& OutObjectPath, FString& OutError) const;

	// 신규 WeaponData를 existing shared durable core로 exact Create합니다.
	bool CreateWeaponAsset(const FSoftObjectPath& ProjectilePath, const FSoftObjectPath& AmmoPath, FSoftObjectPath& OutObjectPath, FString& OutError) const;

	// current naming root가 Product 또는 approved disposable test namespace인지 반환합니다.
	FString GetAssetRoot() const;

	// validation issue를 사용자용 메시지 배열에 추가합니다.
	static void AddIssue(
		TArray<FCFWeaponGuideIssue>& OutIssues,
		ECFWeaponGuideIssueSeverity Severity,
		ECFWeaponGuideStep Step,
		const FString& Message);

	// blocking Error issue가 하나라도 있는지 반환합니다.
	static bool HasBlockingIssue(const TArray<FCFWeaponGuideIssue>& Issues);

	// 현재 semantic Stable StepId입니다.
	ECFWeaponGuideStep CurrentStep = ECFWeaponGuideStep::IdentityTemplate;

	// 현재 Weapon Guide 세션의 transient draft입니다.
	FCFWeaponGuideDraft Draft;

	// durable Create와 full validation을 통과한 마지막 완료 Bundle입니다.
	FCFWeaponGuideResultBundle ResultBundle;

	// P0-03 Bundle fingerprint와 별도로 persisted exact5 및 완료 USER 의미를 고정하는 P0-04 semantic fingerprint입니다.
	FString CompletionSemanticFingerprint;

	// 마지막 navigation/validation/durable 결과를 사용자에게 보여줄 메시지입니다.
	FString LastStatusMessage;

	// 현재 세션에서 provider durable Create가 확인된 DamageData exact path입니다. 후속 child 실패 뒤 same-session retry에 재사용합니다.
	FSoftObjectPath SessionCreatedDamagePath;

	// 현재 세션에서 provider durable Create가 확인된 AmmoData exact path입니다. 후속 child 실패 뒤 same-session retry에 재사용합니다.
	FSoftObjectPath SessionCreatedAmmoPath;

	// 현재 세션에서 durable Create가 확인된 TurretMountData exact path입니다. 후속 child 실패 뒤 same-session retry에 재사용합니다.
	FSoftObjectPath SessionCreatedTurretPath;

	// 현재 세션에서 durable Create가 확인된 ProjectileData exact path입니다. 후속 child 실패 뒤 same-session retry에 재사용합니다.
	FSoftObjectPath SessionCreatedProjectilePath;

	// 현재 세션에서 durable Create가 확인된 WeaponData exact path입니다. 후속 처리 실패 뒤 same-session retry에 재사용합니다.
	FSoftObjectPath SessionCreatedWeaponPath;

#if WITH_DEV_AUTOMATION_TESTS
	// Automation에서만 사용할 disposable asset root입니다. 비어 있으면 Product root를 사용합니다.
	FString DisposableTestRoot;

	// Confirmed durable write 직후 deterministic interruption을 주입할 Stable Step입니다.
	TOptional<ECFWeaponGuideStep> TestDurableInterruptionStep;
#endif
};
