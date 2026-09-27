// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentBuilderTypes.h
// Version: v1.4.0
// Date: 2026-09-17
// Description: CF-FQ-054 EBA-P0-05 USER UX correction을 포함한 transient draft, durable Review와 Vehicle Mount compatibility probe DTO입니다.
// Changelog:
// - v1.4.0: 기존 localized DisplayName을 손실 없이 보존하는 session-local preserve flag를 Draft/Semantic snapshot에 추가.
// - v1.3.0: VehicleDataObjectPath + MountProfileId transient probe identity, Compatible/Incompatible/CannotEvaluate exact3와 read-only Mount option DTO를 추가.
// - v1.2.0: exact7 semantic snapshot, one-shot approval lifecycle, Review evidence와 durable apply report DTO를 추가.
// - v1.1.0: Weapon/Scanner child 조립 검증을 위한 Editor-only validation severity/item/result DTO를 추가.
// - v1.0.0: EquipmentPreset browser row, UI-only Weapon/Scanner Draft Mode와 exact7 authored field transient draft를 추가.
// Migration:
// - Runtime UCFEquipmentPresetData와 Product Asset 형식에는 변경이 없습니다.
// - Review/approval DTO와 ECFEquipmentDraftMode는 Editor UI 전용이며 Product Asset에 저장하지 않습니다.
// - compatibility probe identity/result는 Editor session-local transient이며 semantic fingerprint, ReviewProposalDigest와 durable readiness에 포함하지 않습니다.
// - 기존 localized DisplayName은 preserve flag가 true일 때 Equipment Builder가 다시 쓰지 않으며, 다른 authored field만 안전하게 변경할 수 있습니다.
// - durable write authority는 DTO가 아니라 existing CFDADurableCore가 계속 단독 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleWeaponTypes.h"
#include "UObject/SoftObjectPtr.h"

class UCFTurretMountData;
class UCFVehicleSensorData;
class UCFWeaponData;

// Equipment Builder에서만 사용하는 transient 조립 모드입니다.
enum class ECFEquipmentDraftMode : uint8
{
	Weapon,
	Scanner
};

// Equipment Builder가 child 조립 상태를 사용자에게 표시할 Editor-only 진단 단계입니다.
enum class ECFEquipmentValidationSeverity : uint8
{
	Pass,
	Limited,
	Warning,
	Error
};

// child 조립/검증 결과에서 사용자에게 표시할 한 항목입니다.
struct FCFEquipmentValidationItem
{
	// 항목의 차단 여부와 표현 수준입니다.
	ECFEquipmentValidationSeverity Severity = ECFEquipmentValidationSeverity::Pass;

	// 사용자에게 표시할 짧은 검증 항목 이름입니다.
	FString Title;

	// 검증 결과와 필요한 조치를 설명하는 사용자 문장입니다.
	FString Message;
};

// current transient Draft 전체의 bounded child validation 결과입니다.
struct FCFEquipmentDraftValidation
{
	// Runtime EquipmentPreset의 단일 payload 완성 규칙을 만족하는지 나타냅니다.
	bool bPackageComplete = false;

	// Apply/Save 전 반드시 해결해야 할 blocking error가 하나라도 있는지 나타냅니다.
	bool bHasBlockingErrors = false;

	// Package/child/reference chain을 계층별로 설명하는 결과 목록입니다.
	TArray<FCFEquipmentValidationItem> Items;
};

// 특정 Vehicle Mount에 대한 Editor-only compatibility probe의 exact3 결과입니다.
enum class ECFEquipmentCompatibilityState : uint8
{
	CannotEvaluate,
	Compatible,
	Incompatible
};

// compatibility probe가 exact3 결과를 선택한 bounded 이유입니다.
enum class ECFEquipmentCompatibilityReason : uint8
{
	None,
	NoVehicleSelected,
	VehicleSourceUnresolved,
	VehicleSourceUnpersisted,
	VehicleSourceDirty,
	MountProfileMissing,
	MountProfileIdNone,
	MountProfileIdDuplicate,
	MountTypeNone,
	EquipmentIntrinsicInvalid,
	RequiredMountTypeMismatch,
	RequiredWeaponSizeTooLarge,
	WeaponMountUnsupported,
	WeaponSizeUnsupported,
	RuntimeContractRejected
};

// selected VehicleData의 current MountProfiles에서 read-only로 투영한 한 행입니다.
struct FCFEquipmentMountOption
{
	// transient compatibility identity의 MountProfileId입니다.
	FName MountProfileId = NAME_None;

	// current VehicleData가 선언한 장착 타입입니다.
	ECFVehicleMountType MountType = ECFVehicleMountType::None;

	// current VehicleData가 선언한 장착 크기 제한입니다.
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::None;
};

// VehicleDataObjectPath + MountProfileId를 fresh 평가한 Editor-only advisory 결과입니다.
struct FCFEquipmentCompatibilityResult
{
	// Compatible/Incompatible/CannotEvaluate exact3 상태입니다.
	ECFEquipmentCompatibilityState State = ECFEquipmentCompatibilityState::CannotEvaluate;

	// exact3 상태를 선택한 bounded 이유입니다.
	ECFEquipmentCompatibilityReason Reason = ECFEquipmentCompatibilityReason::NoVehicleSelected;

	// 평가에 사용한 transient VehicleData object path입니다.
	FSoftObjectPath VehicleDataObjectPath;

	// 평가에 사용한 transient MountProfileId입니다.
	FName MountProfileId = NAME_None;

	// exact1 Mount가 확인된 경우 현재 장착 타입입니다.
	ECFVehicleMountType MountType = ECFVehicleMountType::None;

	// exact1 Mount가 확인된 경우 현재 장착 크기 제한입니다.
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::None;

	// 사용자에게 현재 결과와 조치를 설명하는 bounded diagnostic입니다.
	FString Diagnostic;
};

// Equipment Builder Review가 승인하는 durable operation 종류입니다.
enum class ECFEquipmentReviewOperation : uint8
{
	None,
	Create,
	Update
};

// Equipment Builder one-shot approval의 lifecycle입니다.
enum class ECFEquipmentApprovalState : uint8
{
	Unreviewed,
	Reviewed,
	ApplyAttempted,
	Consumed
};

// Equipment Builder가 사용자에게 반환하는 durable apply terminal 결과입니다.
enum class ECFEquipmentApplyResult : uint8
{
	NotRun,
	BlockedBeforeMutation,
	FailedBeforeDurableWrite,
	InMemoryStateUnconfirmed,
	DurableApplied,
	SaveStateUnconfirmed
};

// EquipmentPreset authored exact7을 lossless durable semantic으로 정규화한 snapshot입니다.
struct FCFEquipmentSemanticSnapshot
{
	// EquipmentPreset stable identity입니다.
	FName EquipmentId = NAME_None;

	// 기존 localized DisplayName을 그대로 보존하고 Equipment Builder가 다시 쓰지 않는지 나타냅니다.
	bool bPreserveExistingDisplayName = false;

	// Literal FText source 또는 preserve-mode stable sentinel입니다.
	FString DisplayNameSource;

	// EquipmentPreset authored mount requirement입니다.
	ECFVehicleMountType RequiredMountType = ECFVehicleMountType::None;

	// EquipmentPreset authored size requirement입니다.
	ECFVehicleWeaponSize RequiredWeaponSize = ECFVehicleWeaponSize::None;

	// TurretMountData canonical object path입니다.
	FSoftObjectPath DefaultTurretMountDataPath;

	// WeaponData canonical object path입니다.
	FSoftObjectPath DefaultWeaponDataPath;

	// VehicleSensorData canonical object path입니다.
	FSoftObjectPath DefaultSensorDataPath;

	// 위 exact7 semantic에서 계산한 deterministic SHA-256 fingerprint입니다.
	FString SemanticFingerprint;
};

// successful Review가 Apply에 넘기는 immutable current-contract approval evidence입니다.
struct FCFEquipmentReviewState
{
	// 현재 one-shot approval lifecycle state입니다.
	ECFEquipmentApprovalState ApprovalState = ECFEquipmentApprovalState::Unreviewed;

	// Reviewed Create 또는 Update operation입니다.
	ECFEquipmentReviewOperation Operation = ECFEquipmentReviewOperation::None;

	// Reviewed exact target object path입니다.
	FString TargetObjectPath;

	// Reviewed exact EquipmentPreset native class path입니다.
	FString TargetClassPath;

	// Update current semantic fingerprint 또는 Create의 stable AbsentTarget sentinel입니다.
	FString CurrentSemanticFingerprint;

	// Reviewed prospective exact7 semantic fingerprint입니다.
	FString ProspectiveSemanticFingerprint;

	// Equipment Authoring durable semantic contract revision입니다.
	int32 ContractRevision = 0;

	// blocking validation이 모두 통과했음을 나타내는 stable canonical token입니다.
	FString ValidationState;

	// current-contract exact7 approval binding의 deterministic SHA-256 digest입니다.
	FString ReviewProposalDigest;

	// Update에서 Before semantic snapshot이 존재하는지 나타냅니다.
	bool bHasBeforeSnapshot = false;

	// Update Review의 persisted/current Before snapshot입니다.
	FCFEquipmentSemanticSnapshot BeforeSnapshot;

	// Create/Update Review의 prospective After snapshot입니다.
	FCFEquipmentSemanticSnapshot AfterSnapshot;
};

// explicit 적용 및 저장 한 번의 durable terminal report입니다.
struct FCFEquipmentApplyReport
{
	// durable core 결과를 Equipment Builder 의미로 투영한 terminal result입니다.
	ECFEquipmentApplyResult Result = ECFEquipmentApplyResult::NotRun;

	// Apply attempt에서 one-shot approval이 소비됐는지 나타냅니다.
	bool bApprovalConsumed = false;

	// exact reviewed target object path입니다.
	FString TargetObjectPath;

	// 사용자와 Automation이 확인할 bounded terminal diagnostic입니다.
	FString Diagnostic;
};

// metadata-only EquipmentPreset browser가 표시하는 한 행입니다.
struct FCFEquipmentPresetListEntry
{
	// 저장된 Asset 이름입니다.
	FString AssetName;

	// exact persisted EquipmentPreset object path입니다.
	FString ObjectPath;

	// Asset이 속한 package path입니다.
	FString PackagePath;
};

// UCFEquipmentPresetData authored exact7을 UObject mutation 없이 편집하는 transient 초안입니다.
struct FCFEquipmentPresetDraft
{
	// 장비 프리셋의 stable identity 초안입니다.
	FName EquipmentId = NAME_None;

	// 플레이어/에디터에 표시할 이름 초안입니다.
	FText DisplayName;

	// 기존 localized DisplayName을 손실 없이 보존하기 위해 이 초안에서 표시 이름 편집을 잠그는지 나타냅니다.
	bool bPreserveExistingDisplayName = false;

	// 프리셋 자체가 요구하는 장착 타입 초안입니다.
	ECFVehicleMountType RequiredMountType = ECFVehicleMountType::Turret;

	// 프리셋 자체가 요구하는 장착 크기 초안입니다.
	ECFVehicleWeaponSize RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// 무장 패키지의 터렛 마운트 reference 초안입니다.
	TSoftObjectPtr<UCFTurretMountData> DefaultTurretMountData;

	// 무장 패키지의 무기 reference 초안입니다.
	TSoftObjectPtr<UCFWeaponData> DefaultWeaponData;

	// 스캐너 패키지의 센서 reference 초안입니다.
	TSoftObjectPtr<UCFVehicleSensorData> DefaultSensorData;

	// persisted enum이 아닌 Equipment Builder UI-only 조립 모드입니다.
	ECFEquipmentDraftMode DraftMode = ECFEquipmentDraftMode::Weapon;
};
