// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentResource.h
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS Stable Resource Catalog, Asset Picker registration, capability와 P0-06 snapshot fingerprint contract입니다.
// Changelog:
// - v1.2.0: P0-06 Catalog Snapshot용 resource existence/type/capability semantic fingerprint API를 추가.
// - v1.1.0: Fresh review 교정으로 Picker registration 시 AllowedClass resolve를 fail-closed하고 unbound binding source를 None으로 명확화.
// - v1.0.0: ResourceId catalog, picker type registry, Asset Registry validation, mesh Socket/Bone/MaterialSlot
//   capability snapshot, semantic role/profile/override resolver와 Workbook ResourceReference validation을 최초 구현.
// Migration:
// - P0-01 Canonical Content Model의 ResourceReference와 P0-02 Generic Compiler Core를 그대로 재사용합니다.
// - Product Apply, DataAsset Save, Workbook write, CCAS-P0-07 authority cutover를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentTypes.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/TopLevelAssetPath.h"

/** Semantic Role이 resource를 반드시 요구하는지 나타냅니다. */
enum class ECFResourceRequirement : uint8
{
	Optional,
	Required
};

/** Mesh resource에서 semantic role이 요구하는 capability 종류입니다. */
enum class ECFResourceCapabilityKind : uint8
{
	None,
	Socket,
	Bone,
	MaterialSlot
};

/** Per-content override가 profile binding을 교체하거나 비우는 방식입니다. */
enum class ECFResourceOverrideMode : uint8
{
	Replace,
	Clear
};

/** 최종 resolved binding이 어느 authoring layer에서 왔는지 나타냅니다. */
enum class ECFResourceBindingSource : uint8
{
	None,
	Profile,
	Override
};

/** SObjectPropertyEntryBox 같은 Unreal Asset Picker가 소비할 stable picker type registration입니다. */
struct FCFResourcePickerDescriptor
{
	// UI/schema에서 참조하는 stable picker type identity입니다.
	FString PickerTypeId;

	// Unreal Asset Picker의 AllowedClass에 대응하는 expected base class path입니다.
	FTopLevelAssetPath AllowedBaseClassPath;

	// Picker UI에서 빈 값을 허용할 수 있는지 나타냅니다.
	bool bAllowClear = true;
};

/** Stable ResourceId가 가리키는 persisted Unreal asset descriptor입니다. */
struct FCFResourceDescriptor
{
	// Workbook/Content가 raw ObjectPath 대신 참조하는 stable ResourceId입니다.
	FString ResourceId;

	// 이 resource가 사용하는 registered picker type identity입니다.
	FString PickerTypeId;

	// Persisted Unreal asset의 canonical top-level object path입니다.
	FSoftObjectPath ObjectPath;
};

/** Mesh resource 하나에서 발견한 attach/material capability snapshot입니다. */
struct FCFResourceCapabilitySnapshot
{
	// Snapshot 대상 stable ResourceId입니다.
	FString ResourceId;

	// StaticMesh 또는 SkeletalMesh를 실제로 scan했는지 나타냅니다.
	bool bMeshAsset = false;

	// Mesh/Skeleton에서 발견한 unique socket names입니다.
	TArray<FName> SocketNames;

	// SkeletalMesh reference skeleton에서 발견한 unique bone names입니다.
	TArray<FName> BoneNames;

	// Static/Skeletal material slot에서 발견한 unique material slot names입니다.
	TArray<FName> MaterialSlotNames;
};

/** Semantic Role의 required/optional 및 capability contract입니다. */
struct FCFSemanticRoleDescriptor
{
	// Muzzle.Primary 같은 stable semantic role identity입니다.
	FString RoleId;

	// Role binding이 없거나 invalid일 때 fail policy입니다.
	ECFResourceRequirement Requirement = ECFResourceRequirement::Optional;

	// 이 role이 resource 자체 또는 특정 mesh capability를 요구하는지 나타냅니다.
	ECFResourceCapabilityKind CapabilityKind = ECFResourceCapabilityKind::None;
};

/** Profile 또는 Override가 semantic role 하나에 부여하는 resource binding 값입니다. */
struct FCFResourceBindingValue
{
	// Binding 대상 stable semantic role identity입니다.
	FString RoleId;

	// Raw ObjectPath가 아닌 stable ResourceId입니다.
	FString ResourceId;

	// Socket/Bone/MaterialSlot role일 때 요구하는 exact capability name입니다.
	FName CapabilityName = NAME_None;
};

/** 반복되는 resource bindings를 재사용하는 stable profile입니다. */
struct FCFResourceProfile
{
	// Profile authoring identity입니다.
	FString ProfileId;

	// Profile이 제공하는 unique role bindings입니다.
	TArray<FCFResourceBindingValue> Bindings;
};

/** Profile binding 위에 적용되는 per-content explicit override입니다. */
struct FCFResourceBindingOverride
{
	// Override 대상 stable semantic role identity입니다.
	FString RoleId;

	// Profile 값을 Replace하거나 optional role을 Clear하는 방식입니다.
	ECFResourceOverrideMode Mode = ECFResourceOverrideMode::Replace;

	// Replace일 때 사용할 stable ResourceId입니다.
	FString ResourceId;

	// Replace일 때 사용할 exact Socket/Bone/MaterialSlot capability name입니다.
	FName CapabilityName = NAME_None;
};

/** 특정 content가 resolve하려는 semantic role/profile/override 입력입니다. */
struct FCFResourceBindingRequest
{
	// 이 content가 실제로 resolve해야 하는 unique semantic role IDs입니다.
	TArray<FString> RequestedRoleIds;

	// Optional base profile identity입니다. 비어 있으면 profile layer를 사용하지 않습니다.
	FString ProfileId;

	// Profile 이후에 적용할 per-content overrides입니다.
	TArray<FCFResourceBindingOverride> Overrides;
};

/** Semantic role 하나의 최종 resolved resource binding입니다. */
struct FCFResolvedResourceBinding
{
	// Resolved stable semantic role identity입니다.
	FString RoleId;

	// Required/Optional contract입니다.
	ECFResourceRequirement Requirement = ECFResourceRequirement::Optional;

	// 실제 resource binding이 존재하는지 나타냅니다.
	bool bBound = false;

	// Bound일 때 profile 또는 override 중 실제 source layer입니다.
	ECFResourceBindingSource Source = ECFResourceBindingSource::None;

	// Bound일 때 최종 stable ResourceId입니다.
	FString ResourceId;

	// Bound resource의 resolved canonical asset object path입니다.
	FSoftObjectPath ObjectPath;

	// Role이 요구한 capability kind입니다.
	ECFResourceCapabilityKind CapabilityKind = ECFResourceCapabilityKind::None;

	// Capability role일 때 검증된 exact capability name입니다.
	FName CapabilityName = NAME_None;
};

/** Profile + Override semantic binding 전체 resolve 결과입니다. */
struct FCFResourceBindingResult
{
	// Blocking issue 없이 requested roles 전체를 resolve했는지 나타냅니다.
	bool bSucceeded = false;

	// Stable RoleId 순서의 resolved bindings입니다.
	TArray<FCFResolvedResourceBinding> Bindings;

	// Required/Optional policy를 포함한 typed diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
};

/** Stable picker type -> Unreal AllowedClass registration을 관리합니다. */
class FCFResourcePickerRegistry
{
public:
	// Picker descriptor를 duplicate 없이 등록합니다.
	bool RegisterPicker(
		const FCFResourcePickerDescriptor& Descriptor,
		FString& OutError);

	// Stable picker type의 descriptor를 반환하며 없으면 nullptr입니다.
	const FCFResourcePickerDescriptor* FindPicker(const FString& PickerTypeId) const;

	// SObjectPropertyEntryBox AllowedClass에서 사용할 loaded UClass를 resolve합니다.
	UClass* ResolveAllowedClass(
		const FString& PickerTypeId,
		FString& OutError) const;

	// Registered picker type 수를 반환합니다.
	int32 Num() const
	{
		return PickersById.Num();
	}

private:
	// Stable PickerTypeId -> picker descriptor map입니다.
	TMap<FString, FCFResourcePickerDescriptor> PickersById;
};

/** Stable ResourceId -> persisted Unreal asset mapping을 관리합니다. */
class FCFResourceCatalog
{
public:
	// Resource descriptor를 picker contract와 Asset Registry metadata로 검증한 뒤 등록합니다.
	bool RegisterResource(
		const FCFResourceDescriptor& Descriptor,
		const FCFResourcePickerRegistry& PickerRegistry,
		FString& OutError);

	// Stable ResourceId descriptor를 반환하며 없으면 nullptr입니다.
	const FCFResourceDescriptor* FindResource(const FString& ResourceId) const;

	// 현재 Asset Registry 기준으로 resource 존재/type contract를 fresh 검증합니다.
	bool ValidateResource(
		const FString& ResourceId,
		const FCFResourcePickerRegistry& PickerRegistry,
		TArray<FCFContentValidationIssue>& OutIssues) const;

	// Registered resource existence/type와 mesh capability를 canonical SHA-256 fingerprint로 생성합니다.
	bool BuildSemanticFingerprint(
		const FCFResourcePickerRegistry& PickerRegistry,
		FString& OutFingerprint,
		TArray<FCFContentValidationIssue>& OutIssues) const;

	// Registered ResourceId 수를 반환합니다.
	int32 Num() const
	{
		return ResourcesById.Num();
	}

private:
	// Stable ResourceId -> resource descriptor map입니다.
	TMap<FString, FCFResourceDescriptor> ResourcesById;
};

/** StaticMesh/SkeletalMesh의 Socket/Bone/MaterialSlot capability를 read-only로 scan합니다. */
class FCFMeshCapabilityScanner
{
public:
	// Catalog resource asset을 load-only scan해서 deterministic capability snapshot을 만듭니다.
	static bool ScanResource(
		const FString& ResourceId,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		FCFResourceCapabilitySnapshot& OutSnapshot,
		TArray<FCFContentValidationIssue>& OutIssues);

	// Snapshot이 exact requested capability를 제공하는지 확인합니다.
	static bool HasCapability(
		const FCFResourceCapabilitySnapshot& Snapshot,
		ECFResourceCapabilityKind CapabilityKind,
		FName CapabilityName);
};

/** Stable Semantic Role descriptor registry입니다. */
class FCFSemanticRoleRegistry
{
public:
	// Role descriptor를 duplicate 없이 등록합니다.
	bool RegisterRole(
		const FCFSemanticRoleDescriptor& Descriptor,
		FString& OutError);

	// Stable RoleId descriptor를 반환하며 없으면 nullptr입니다.
	const FCFSemanticRoleDescriptor* FindRole(const FString& RoleId) const;

	// Registered role 수를 반환합니다.
	int32 Num() const
	{
		return RolesById.Num();
	}

private:
	// Stable RoleId -> semantic role descriptor map입니다.
	TMap<FString, FCFSemanticRoleDescriptor> RolesById;
};

/** Stable resource binding profile registry입니다. */
class FCFResourceProfileRegistry
{
public:
	// Profile의 role/resource/capability token을 검증한 뒤 duplicate 없이 등록합니다.
	bool RegisterProfile(
		const FCFResourceProfile& Profile,
		const FCFSemanticRoleRegistry& RoleRegistry,
		FString& OutError);

	// Stable ProfileId를 반환하며 없으면 nullptr입니다.
	const FCFResourceProfile* FindProfile(const FString& ProfileId) const;

	// Registered profile 수를 반환합니다.
	int32 Num() const
	{
		return ProfilesById.Num();
	}

private:
	// Stable ProfileId -> profile map입니다.
	TMap<FString, FCFResourceProfile> ProfilesById;
};

/** Requested semantic roles에 Profile + Override precedence를 적용하고 resource/capability를 검증합니다. */
class FCFResourceBindingResolver
{
public:
	// Override > Profile precedence로 requested roles를 deterministic하게 resolve합니다.
	static bool Resolve(
		const FCFResourceBindingRequest& Request,
		const FCFSemanticRoleRegistry& RoleRegistry,
		const FCFResourceProfileRegistry& ProfileRegistry,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		FCFResourceBindingResult& OutResult);
};

/** Canonical Workbook의 ResourceReference를 Resource Catalog에 연결해 검증합니다. */
class FCFContentResourceValidator
{
public:
	// Top-level/child ResourceReference VALUE가 catalog에 존재하고 현재 asset type contract를 만족하는지 검증합니다.
	static bool ValidateWorkbook(
		const FCFContentWorkbookModel& Workbook,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		TArray<FCFContentValidationIssue>& OutIssues);
};
