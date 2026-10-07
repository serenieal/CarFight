// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentResource.cpp
// Version: v1.3.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS Resource Catalog + Semantic Binding과 P0-06 snapshot fingerprint read-only implementation입니다.
// Changelog:
// - v1.3.0: P0-06 fresh implementation review 교정으로 missing/unready/type-mismatch/capability-failure 자체를 fingerprint state로 보존하고 diagnostics와 분리.
// - v1.2.0: Registered resource existence/type/mesh capability를 deterministic ResourceCatalogFingerprint로 생성.
// - v1.1.0: Fresh review 교정으로 Picker AllowedClass registration fail-closed와 unbound binding source None semantics를 추가.
// - v1.0.0: Stable ResourceId/picker registry, Asset Registry validation, mesh capability scan,
//   Semantic Role/Profile/Override resolve와 Workbook ResourceReference validation을 최초 구현.
// Migration:
// - P0-01/P0-02 Core를 변경하거나 Product/Workbook persistent mutation을 수행하지 않습니다.

#include "DataAuthoring/CFContentResource.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace CFContentResourcePrivate
{
	/** Stable blocking/non-blocking resource diagnostic을 append합니다. */
	void AddIssue(
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& Code,
		const FString& Path,
		const FString& Message,
		const bool bBlocking = true)
	{
		// Append할 typed diagnostic입니다.
		FCFContentValidationIssue Issue;
		Issue.Code = Code;
		Issue.Path = Path;
		Issue.Message = Message;
		Issue.bBlocking = bBlocking;
		OutIssues.Add(MoveTemp(Issue));
	}

	/** Issues 안에 blocking diagnostic이 하나라도 있는지 확인합니다. */
	bool HasBlockingIssues(const TArray<FCFContentValidationIssue>& Issues)
	{
		for (const FCFContentValidationIssue& Issue : Issues)
		{
			if (Issue.bBlocking)
			{
				return true;
			}
		}
		return false;
	}

	/** Resource/Pickertype/Role/Profile에서 사용하는 stable identifier를 검증합니다. */
	bool IsStableSemanticId(const FString& Value)
	{
		return CFIsStableContentId(Value);
	}

	/** SoftObjectPath가 top-level persisted asset을 가리키는 canonical 구조인지 확인합니다. */
	bool IsCanonicalAssetObjectPath(const FSoftObjectPath& ObjectPath)
	{
		// Top-level asset identity입니다.
		const FTopLevelAssetPath AssetPath = ObjectPath.GetAssetPath();
		// Long package name입니다.
		const FString PackageName = AssetPath.GetPackageName().ToString();
		return !ObjectPath.IsNull()
			&& !AssetPath.IsNull()
			&& ObjectPath.GetSubPathString().IsEmpty()
			&& FPackageName::IsValidLongPackageName(PackageName);
	}

	/** Asset Registry에서 exact object path의 persisted metadata를 읽습니다. */
	FAssetData GetAssetData(const FSoftObjectPath& ObjectPath)
	{
		// 현재 Editor read-only Asset Registry입니다.
		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		return AssetRegistry.GetAssetByObjectPath(ObjectPath, false, false);
	}

	/** AssetData가 expected base class 또는 derived class인지 metadata only로 검사합니다. */
	bool IsAssetClassCompatible(
		const FAssetData& AssetData,
		const FTopLevelAssetPath& ExpectedBaseClassPath)
	{
		if (!AssetData.IsValid() || ExpectedBaseClassPath.IsNull())
		{
			return false;
		}

		if (AssetData.AssetClassPath == ExpectedBaseClassPath)
		{
			return true;
		}

		// Derived class query의 base class 목록입니다.
		TArray<FTopLevelAssetPath> BaseClassPaths;
		BaseClassPaths.Add(ExpectedBaseClassPath);
		// Derived class query의 excluded class 집합입니다.
		const TSet<FTopLevelAssetPath> ExcludedClassPaths;
		// Asset Registry가 계산한 derived class set입니다.
		TSet<FTopLevelAssetPath> DerivedClassPaths;

		// 현재 Editor read-only Asset Registry입니다.
		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		AssetRegistry.GetDerivedClassNames(
			BaseClassPaths,
			ExcludedClassPaths,
			DerivedClassPaths);
		return DerivedClassPaths.Contains(AssetData.AssetClassPath);
	}

	/** Top-level class path를 loaded/unloaded 상태와 무관하게 editor-only로 resolve합니다. */
	UClass* ResolveClassPath(const FTopLevelAssetPath& ClassPath)
	{
		// 이미 loaded 된 exact native/generated class입니다.
		UClass* ResolvedClass = FindObject<UClass>(
			ClassPath,
			EFindObjectFlags::None);
		if (ResolvedClass == nullptr)
		{
			// Unloaded class를 soft class path로 resolve할 fallback입니다.
			const FSoftClassPath SoftClassPath(ClassPath.ToString());
			ResolvedClass = SoftClassPath.TryLoadClass<UObject>();
		}
		return ResolvedClass;
	}

	/** Resource descriptor와 picker의 persisted existence/type contract를 검증합니다. */
	bool ValidateResourceDescriptor(
		const FCFResourceDescriptor& Descriptor,
		const FCFResourcePickerRegistry& PickerRegistry,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		// Resource diagnostic path입니다.
		const FString ResourcePath = FString::Printf(TEXT("Resource/%s"), *Descriptor.ResourceId);

		if (!CFIsStableResourceId(Descriptor.ResourceId))
		{
			AddIssue(
				OutIssues,
				TEXT("InvalidResourceId"),
				ResourcePath,
				TEXT("ResourceId가 canonical stable resource ID grammar를 따르지 않습니다."));
			return false;
		}

		// Resource가 참조하는 picker contract입니다.
		const FCFResourcePickerDescriptor* Picker =
			PickerRegistry.FindPicker(Descriptor.PickerTypeId);
		if (Picker == nullptr)
		{
			AddIssue(
				OutIssues,
				TEXT("MissingResourcePicker"),
				ResourcePath,
				TEXT("Resource가 참조하는 PickerTypeId가 등록되어 있지 않습니다."));
			return false;
		}

		if (!IsCanonicalAssetObjectPath(Descriptor.ObjectPath))
		{
			AddIssue(
				OutIssues,
				TEXT("InvalidResourceObjectPath"),
				ResourcePath,
				TEXT("Resource ObjectPath는 canonical top-level Unreal asset path여야 합니다."));
			return false;
		}

		// Persisted Asset Registry metadata입니다.
		const FAssetData AssetData = GetAssetData(Descriptor.ObjectPath);
		if (!AssetData.IsValid())
		{
			AddIssue(
				OutIssues,
				TEXT("MissingResourceAsset"),
				ResourcePath,
				TEXT("Resource ObjectPath가 가리키는 asset metadata를 Asset Registry에서 찾을 수 없습니다."));
			return false;
		}

		if (!IsAssetClassCompatible(AssetData, Picker->AllowedBaseClassPath))
		{
			AddIssue(
				OutIssues,
				TEXT("ResourceAssetTypeMismatch"),
				ResourcePath,
				FString::Printf(
					TEXT("Resource asset class가 picker expected base class와 호환되지 않습니다. Actual=%s ExpectedBase=%s"),
					*AssetData.AssetClassPath.ToString(),
					*Picker->AllowedBaseClassPath.ToString()));
			return false;
		}

		return true;
	}

	/** FName 배열을 case-sensitive lexical order의 unique snapshot으로 정규화합니다. */
	void SortUniqueNames(TArray<FName>& Names)
	{
		// Duplicate 제거용 unique name set입니다.
		TSet<FName> UniqueNames;
		for (const FName Name : Names)
		{
			if (!Name.IsNone())
			{
				UniqueNames.Add(Name);
			}
		}

		Names.Reset();
		for (const FName UniqueName : UniqueNames)
		{
			Names.Add(UniqueName);
		}
		Names.Sort([](const FName Left, const FName Right)
		{
			return Left.ToString() < Right.ToString();
		});
	}

	/** Profile binding에서 exact RoleId를 찾습니다. */
	const FCFResourceBindingValue* FindProfileBinding(
		const FCFResourceProfile& Profile,
		const FString& RoleId)
	{
		return Profile.Bindings.FindByPredicate(
			[&RoleId](const FCFResourceBindingValue& Binding)
			{
				return Binding.RoleId.Equals(RoleId, ESearchCase::CaseSensitive);
			});
	}

	/** Override 집합에서 exact RoleId를 찾습니다. */
	const FCFResourceBindingOverride* FindOverride(
		const TArray<FCFResourceBindingOverride>& Overrides,
		const FString& RoleId)
	{
		return Overrides.FindByPredicate(
			[&RoleId](const FCFResourceBindingOverride& Override)
			{
				return Override.RoleId.Equals(RoleId, ESearchCase::CaseSensitive);
			});
	}

	/** Optional role의 내부 resource validation issues를 non-blocking으로 downgrade합니다. */
	void AppendRoleScopedIssues(
		const FString& RoleId,
		const ECFResourceRequirement Requirement,
		const TArray<FCFContentValidationIssue>& SourceIssues,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		for (const FCFContentValidationIssue& SourceIssue : SourceIssues)
		{
			// Role context를 반영한 copied issue입니다.
			FCFContentValidationIssue Issue = SourceIssue;
			Issue.Path = FString::Printf(
				TEXT("Role/%s/%s"),
				*RoleId,
				*SourceIssue.Path);
			Issue.bBlocking = Requirement == ECFResourceRequirement::Required;
			OutIssues.Add(MoveTemp(Issue));
		}
	}

	/** ResourceReference VALUE 하나를 catalog/asset metadata 기준으로 검증합니다. */
	void ValidateResourceReferenceValue(
		const FCFContentValue& Value,
		const FString& ValuePath,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		if (Value.Type != ECFContentValueType::ResourceReference
			|| Value.State != ECFContentValueState::Value)
		{
			return;
		}

		if (!CFIsStableResourceId(Value.StringValue))
		{
			AddIssue(
				OutIssues,
				TEXT("InvalidResourceReference"),
				ValuePath,
				TEXT("ResourceReference VALUE가 canonical ResourceId가 아닙니다."));
			return;
		}

		if (ResourceCatalog.FindResource(Value.StringValue) == nullptr)
		{
			AddIssue(
				OutIssues,
				TEXT("MissingCatalogResource"),
				ValuePath,
				FString::Printf(
					TEXT("ResourceReference '%s'가 Resource Catalog에 없습니다."),
					*Value.StringValue));
			return;
		}

		// Fresh catalog/Asset Registry validation diagnostics입니다.
		TArray<FCFContentValidationIssue> ResourceIssues;
		ResourceCatalog.ValidateResource(
			Value.StringValue,
			PickerRegistry,
			ResourceIssues);
		for (const FCFContentValidationIssue& ResourceIssue : ResourceIssues)
		{
			// Workbook field path를 보존한 copied issue입니다.
			FCFContentValidationIssue Issue = ResourceIssue;
			Issue.Path = ValuePath;
			OutIssues.Add(MoveTemp(Issue));
		}
	}
}

bool FCFResourcePickerRegistry::RegisterPicker(
	const FCFResourcePickerDescriptor& Descriptor,
	FString& OutError)
{
	OutError.Reset();

	if (!CFContentResourcePrivate::IsStableSemanticId(Descriptor.PickerTypeId))
	{
		OutError = TEXT("PickerTypeId가 canonical stable ID가 아닙니다.");
		return false;
	}
	if (Descriptor.AllowedBaseClassPath.IsNull())
	{
		OutError = TEXT("Picker AllowedBaseClassPath가 비어 있습니다.");
		return false;
	}
	if (CFContentResourcePrivate::ResolveClassPath(
		Descriptor.AllowedBaseClassPath) == nullptr)
	{
		OutError = FString::Printf(
			TEXT("Picker AllowedBaseClassPath를 UClass로 resolve하지 못했습니다: %s"),
			*Descriptor.AllowedBaseClassPath.ToString());
		return false;
	}
	if (PickersById.Contains(Descriptor.PickerTypeId))
	{
		OutError = FString::Printf(
			TEXT("Duplicate PickerTypeId: %s"),
			*Descriptor.PickerTypeId);
		return false;
	}

	// Registered immutable-style descriptor copy입니다.
	FCFResourcePickerDescriptor DescriptorCopy = Descriptor;
	PickersById.Add(Descriptor.PickerTypeId, MoveTemp(DescriptorCopy));
	return true;
}

const FCFResourcePickerDescriptor* FCFResourcePickerRegistry::FindPicker(
	const FString& PickerTypeId) const
{
	return PickersById.Find(PickerTypeId);
}

UClass* FCFResourcePickerRegistry::ResolveAllowedClass(
	const FString& PickerTypeId,
	FString& OutError) const
{
	OutError.Reset();

	// Requested picker descriptor입니다.
	const FCFResourcePickerDescriptor* Descriptor = FindPicker(PickerTypeId);
	if (Descriptor == nullptr)
	{
		OutError = FString::Printf(
			TEXT("PickerTypeId가 등록되어 있지 않습니다: %s"),
			*PickerTypeId);
		return nullptr;
	}

	// Registered descriptor의 resolved AllowedClass입니다.
	UClass* AllowedClass = CFContentResourcePrivate::ResolveClassPath(
		Descriptor->AllowedBaseClassPath);
	if (AllowedClass == nullptr)
	{
		OutError = FString::Printf(
			TEXT("Picker AllowedBaseClass를 resolve하지 못했습니다: %s"),
			*Descriptor->AllowedBaseClassPath.ToString());
	}
	return AllowedClass;
}

bool FCFResourceCatalog::RegisterResource(
	const FCFResourceDescriptor& Descriptor,
	const FCFResourcePickerRegistry& PickerRegistry,
	FString& OutError)
{
	OutError.Reset();

	if (ResourcesById.Contains(Descriptor.ResourceId))
	{
		OutError = FString::Printf(
			TEXT("Duplicate ResourceId: %s"),
			*Descriptor.ResourceId);
		return false;
	}

	// Registration 전 fresh resource validation diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
	if (!CFContentResourcePrivate::ValidateResourceDescriptor(
		Descriptor,
		PickerRegistry,
		Issues))
	{
		OutError = Issues.Num() > 0
			? FString::Printf(
				TEXT("%s: %s"),
				*Issues[0].Code,
				*Issues[0].Message)
			: TEXT("Resource descriptor validation이 실패했습니다.");
		return false;
	}

	// Registered immutable-style resource descriptor copy입니다.
	FCFResourceDescriptor DescriptorCopy = Descriptor;
	ResourcesById.Add(Descriptor.ResourceId, MoveTemp(DescriptorCopy));
	return true;
}

const FCFResourceDescriptor* FCFResourceCatalog::FindResource(
	const FString& ResourceId) const
{
	return ResourcesById.Find(ResourceId);
}

bool FCFResourceCatalog::ValidateResource(
	const FString& ResourceId,
	const FCFResourcePickerRegistry& PickerRegistry,
	TArray<FCFContentValidationIssue>& OutIssues) const
{
	OutIssues.Reset();

	// Requested catalog descriptor입니다.
	const FCFResourceDescriptor* Descriptor = FindResource(ResourceId);
	if (Descriptor == nullptr)
	{
		CFContentResourcePrivate::AddIssue(
			OutIssues,
			TEXT("MissingCatalogResource"),
			FString::Printf(TEXT("Resource/%s"), *ResourceId),
			TEXT("Requested ResourceId가 catalog에 없습니다."));
		return false;
	}

	CFContentResourcePrivate::ValidateResourceDescriptor(
		*Descriptor,
		PickerRegistry,
		OutIssues);
	return !CFContentResourcePrivate::HasBlockingIssues(OutIssues);
}

bool FCFResourceCatalog::BuildSemanticFingerprint(
	const FCFResourcePickerRegistry& PickerRegistry,
	FString& OutFingerprint,
	TArray<FCFContentValidationIssue>& OutIssues) const
{
	OutFingerprint.Reset();
	OutIssues.Reset();

	// Deterministic resource catalog semantic byte stream입니다.
	TArray<uint8> CanonicalBytes;
	CFDACommonPrimitives::AppendStringToken(
		CanonicalBytes,
		TEXT("Protocol"),
		TEXT("CarFight.CCAS.ResourceCatalog/v2"));

	// Physical registration order와 독립적인 stable ResourceId 목록입니다.
	TArray<FString> ResourceIds;
	ResourcesById.GetKeys(ResourceIds);
	ResourceIds.Sort();

	for (const FString& ResourceId : ResourceIds)
	{
		// Fingerprint 대상 resource descriptor입니다.
		const FCFResourceDescriptor* Descriptor = ResourcesById.Find(ResourceId);
		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ResourceKey"), ResourceId);
		CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("DescriptorPresent"), Descriptor != nullptr);
		if (Descriptor == nullptr)
		{
			CFContentResourcePrivate::AddIssue(
				OutIssues,
				TEXT("MissingCatalogResource"),
				FString::Printf(TEXT("Resource/%s"), *ResourceId),
				TEXT("Fingerprint 생성 중 registered resource descriptor를 찾지 못했습니다."));
			continue;
		}

		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ResourceId"), Descriptor->ResourceId);
		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("PickerTypeId"), Descriptor->PickerTypeId);
		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ObjectPath"), Descriptor->ObjectPath.ToString());

		// Resource가 결속된 picker descriptor입니다.
		const FCFResourcePickerDescriptor* Picker = PickerRegistry.FindPicker(Descriptor->PickerTypeId);
		CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("PickerPresent"), Picker != nullptr);
		if (Picker == nullptr)
		{
			CFContentResourcePrivate::AddIssue(
				OutIssues,
				TEXT("MissingResourcePicker"),
				FString::Printf(TEXT("Resource/%s"), *ResourceId),
				TEXT("Fingerprint 생성 시 PickerTypeId가 현재 registry에 없습니다."));
			continue;
		}

		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("AllowedBaseClass"), Picker->AllowedBaseClassPath.ToString());
		CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("AllowClear"), Picker->bAllowClear);

		// Persisted Asset Registry metadata입니다.
		const FAssetData AssetData = CFContentResourcePrivate::GetAssetData(Descriptor->ObjectPath);
		const bool bAssetPresent = AssetData.IsValid();
		CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("AssetPresent"), bAssetPresent);
		if (!bAssetPresent)
		{
			CFContentResourcePrivate::AddIssue(
				OutIssues,
				TEXT("MissingResourceAsset"),
				FString::Printf(TEXT("Resource/%s"), *ResourceId),
				TEXT("Registered Resource의 persisted asset metadata가 현재 Asset Registry에 없습니다."));
			continue;
		}

		CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("ActualAssetClass"), AssetData.AssetClassPath.ToString());

		// Current picker type contract compatibility state입니다.
		const bool bTypeCompatible = CFContentResourcePrivate::IsAssetClassCompatible(
			AssetData,
			Picker->AllowedBaseClassPath);
		CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("TypeCompatible"), bTypeCompatible);
		if (!bTypeCompatible)
		{
			CFContentResourcePrivate::AddIssue(
				OutIssues,
				TEXT("ResourceAssetTypeMismatch"),
				FString::Printf(TEXT("Resource/%s"), *ResourceId),
				FString::Printf(
					TEXT("Resource asset class가 picker expected base class와 호환되지 않습니다. Actual=%s ExpectedBase=%s"),
					*AssetData.AssetClassPath.ToString(),
					*Picker->AllowedBaseClassPath.ToString()));
			continue;
		}

		// Socket/Bone/MaterialSlot이 planning truth인 mesh resource인지 확인합니다.
		const bool bStaticMeshResource = CFContentResourcePrivate::IsAssetClassCompatible(
			AssetData,
			UStaticMesh::StaticClass()->GetClassPathName());
		const bool bSkeletalMeshResource = CFContentResourcePrivate::IsAssetClassCompatible(
			AssetData,
			USkeletalMesh::StaticClass()->GetClassPathName());
		const bool bMeshResource = bStaticMeshResource || bSkeletalMeshResource;
		CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("MeshResource"), bMeshResource);

		if (bMeshResource)
		{
			// Current persisted mesh capability snapshot입니다.
			FCFResourceCapabilitySnapshot CapabilitySnapshot;
			// Capability scan typed diagnostics입니다.
			TArray<FCFContentValidationIssue> CapabilityIssues;
			const bool bCapabilityReady = FCFMeshCapabilityScanner::ScanResource(
				ResourceId,
				*this,
				PickerRegistry,
				CapabilitySnapshot,
				CapabilityIssues);
			CFDACommonPrimitives::AppendBoolToken(CanonicalBytes, TEXT("CapabilityReady"), bCapabilityReady);
			OutIssues.Append(CapabilityIssues);
			if (!bCapabilityReady)
			{
				continue;
			}

			for (const FName SocketName : CapabilitySnapshot.SocketNames)
			{
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("Socket"), SocketName.ToString());
			}
			for (const FName BoneName : CapabilitySnapshot.BoneNames)
			{
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("Bone"), BoneName.ToString());
			}
			for (const FName MaterialSlotName : CapabilitySnapshot.MaterialSlotNames)
			{
				CFDACommonPrimitives::AppendStringToken(CanonicalBytes, TEXT("MaterialSlot"), MaterialSlotName.ToString());
			}
		}
	}

	// Missing/unready/type-mismatch 역시 planning truth이므로 blocking diagnostics와 별개로 fingerprint는 반드시 생성합니다.
	FString HashError;
	if (!CFDACommonPrimitives::HashCanonicalBytes(CanonicalBytes, OutFingerprint, HashError))
	{
		CFContentResourcePrivate::AddIssue(
			OutIssues,
			TEXT("ResourceCatalogFingerprintFailed"),
			TEXT("ResourceCatalog"),
			HashError.IsEmpty() ? TEXT("Resource Catalog fingerprint 생성에 실패했습니다.") : HashError);
		return false;
	}
	return true;
}

bool FCFMeshCapabilityScanner::ScanResource(
	const FString& ResourceId,
	const FCFResourceCatalog& ResourceCatalog,
	const FCFResourcePickerRegistry& PickerRegistry,
	FCFResourceCapabilitySnapshot& OutSnapshot,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutSnapshot = FCFResourceCapabilitySnapshot();
	OutSnapshot.ResourceId = ResourceId;
	OutIssues.Reset();

	// Fresh resource metadata validation issues입니다.
	TArray<FCFContentValidationIssue> ResourceIssues;
	if (!ResourceCatalog.ValidateResource(
		ResourceId,
		PickerRegistry,
		ResourceIssues))
	{
		OutIssues.Append(ResourceIssues);
		return false;
	}

	// Scan 대상 catalog descriptor입니다.
	const FCFResourceDescriptor* Descriptor =
		ResourceCatalog.FindResource(ResourceId);
	if (Descriptor == nullptr)
	{
		CFContentResourcePrivate::AddIssue(
			OutIssues,
			TEXT("MissingCatalogResource"),
			FString::Printf(TEXT("Resource/%s"), *ResourceId),
			TEXT("Capability scan 대상 ResourceId가 catalog에 없습니다."));
		return false;
	}

	// Capability scan을 위해 read-only로 load한 asset입니다.
	UObject* AssetObject = Descriptor->ObjectPath.TryLoad();
	if (AssetObject == nullptr)
	{
		CFContentResourcePrivate::AddIssue(
			OutIssues,
			TEXT("ResourceLoadFailed"),
			FString::Printf(TEXT("Resource/%s"), *ResourceId),
			TEXT("Mesh capability scan을 위해 resource asset을 load하지 못했습니다."));
		return false;
	}

	if (const UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetObject))
	{
		OutSnapshot.bMeshAsset = true;

		for (const TObjectPtr<UStaticMeshSocket>& Socket : StaticMesh->Sockets)
		{
			if (Socket != nullptr)
			{
				OutSnapshot.SocketNames.Add(Socket->SocketName);
			}
		}

		for (const FStaticMaterial& Material : StaticMesh->GetStaticMaterials())
		{
			if (!Material.MaterialSlotName.IsNone())
			{
				OutSnapshot.MaterialSlotNames.Add(Material.MaterialSlotName);
			}
		}
	}
	else if (const USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(AssetObject))
	{
		OutSnapshot.bMeshAsset = true;

		// Mesh sockets + Skeleton nonduplicate sockets를 합친 active socket snapshot입니다.
		const TArray<USkeletalMeshSocket*> ActiveSockets =
			SkeletalMesh->GetActiveSocketList();
		for (const USkeletalMeshSocket* Socket : ActiveSockets)
		{
			if (Socket != nullptr)
			{
				OutSnapshot.SocketNames.Add(Socket->SocketName);
			}
		}

		// SkeletalMesh reference skeleton의 deterministic bone identity source입니다.
		const FReferenceSkeleton& ReferenceSkeleton =
			SkeletalMesh->GetRefSkeleton();
		for (int32 BoneIndex = 0;
			BoneIndex < ReferenceSkeleton.GetNum();
			++BoneIndex)
		{
			OutSnapshot.BoneNames.Add(
				ReferenceSkeleton.GetBoneName(BoneIndex));
		}

		for (const FSkeletalMaterial& Material : SkeletalMesh->GetMaterials())
		{
			if (!Material.MaterialSlotName.IsNone())
			{
				OutSnapshot.MaterialSlotNames.Add(Material.MaterialSlotName);
			}
		}
	}
	else
	{
		CFContentResourcePrivate::AddIssue(
			OutIssues,
			TEXT("UnsupportedMeshResource"),
			FString::Printf(TEXT("Resource/%s"), *ResourceId),
			TEXT("Socket/Bone/MaterialSlot capability scan은 StaticMesh 또는 SkeletalMesh resource만 지원합니다."));
		return false;
	}

	CFContentResourcePrivate::SortUniqueNames(OutSnapshot.SocketNames);
	CFContentResourcePrivate::SortUniqueNames(OutSnapshot.BoneNames);
	CFContentResourcePrivate::SortUniqueNames(OutSnapshot.MaterialSlotNames);
	return true;
}

bool FCFMeshCapabilityScanner::HasCapability(
	const FCFResourceCapabilitySnapshot& Snapshot,
	const ECFResourceCapabilityKind CapabilityKind,
	const FName CapabilityName)
{
	switch (CapabilityKind)
	{
	case ECFResourceCapabilityKind::None:
		return true;
	case ECFResourceCapabilityKind::Socket:
		return !CapabilityName.IsNone()
			&& Snapshot.SocketNames.Contains(CapabilityName);
	case ECFResourceCapabilityKind::Bone:
		return !CapabilityName.IsNone()
			&& Snapshot.BoneNames.Contains(CapabilityName);
	case ECFResourceCapabilityKind::MaterialSlot:
		return !CapabilityName.IsNone()
			&& Snapshot.MaterialSlotNames.Contains(CapabilityName);
	default:
		return false;
	}
}

bool FCFSemanticRoleRegistry::RegisterRole(
	const FCFSemanticRoleDescriptor& Descriptor,
	FString& OutError)
{
	OutError.Reset();

	if (!CFContentResourcePrivate::IsStableSemanticId(Descriptor.RoleId))
	{
		OutError = TEXT("RoleId가 canonical stable ID가 아닙니다.");
		return false;
	}
	if (RolesById.Contains(Descriptor.RoleId))
	{
		OutError = FString::Printf(
			TEXT("Duplicate RoleId: %s"),
			*Descriptor.RoleId);
		return false;
	}

	// Registered immutable-style role descriptor copy입니다.
	FCFSemanticRoleDescriptor DescriptorCopy = Descriptor;
	RolesById.Add(Descriptor.RoleId, MoveTemp(DescriptorCopy));
	return true;
}

const FCFSemanticRoleDescriptor* FCFSemanticRoleRegistry::FindRole(
	const FString& RoleId) const
{
	return RolesById.Find(RoleId);
}

bool FCFResourceProfileRegistry::RegisterProfile(
	const FCFResourceProfile& Profile,
	const FCFSemanticRoleRegistry& RoleRegistry,
	FString& OutError)
{
	OutError.Reset();

	if (!CFContentResourcePrivate::IsStableSemanticId(Profile.ProfileId))
	{
		OutError = TEXT("ProfileId가 canonical stable ID가 아닙니다.");
		return false;
	}
	if (ProfilesById.Contains(Profile.ProfileId))
	{
		OutError = FString::Printf(
			TEXT("Duplicate ProfileId: %s"),
			*Profile.ProfileId);
		return false;
	}

	// Profile 내 duplicate role 검사용 set입니다.
	TSet<FString> SeenRoleIds;
	for (const FCFResourceBindingValue& Binding : Profile.Bindings)
	{
		if (!CFContentResourcePrivate::IsStableSemanticId(Binding.RoleId)
			|| RoleRegistry.FindRole(Binding.RoleId) == nullptr)
		{
			OutError = FString::Printf(
				TEXT("Profile binding RoleId가 등록되지 않았습니다: %s"),
				*Binding.RoleId);
			return false;
		}
		if (SeenRoleIds.Contains(Binding.RoleId))
		{
			OutError = FString::Printf(
				TEXT("Profile에 duplicate RoleId binding이 있습니다: %s"),
				*Binding.RoleId);
			return false;
		}
		SeenRoleIds.Add(Binding.RoleId);

		if (!CFIsStableResourceId(Binding.ResourceId))
		{
			OutError = FString::Printf(
				TEXT("Profile binding ResourceId가 invalid입니다: %s"),
				*Binding.ResourceId);
			return false;
		}

		// Binding role contract입니다.
		const FCFSemanticRoleDescriptor* Role =
			RoleRegistry.FindRole(Binding.RoleId);
		if (Role != nullptr)
		{
			if (Role->CapabilityKind == ECFResourceCapabilityKind::None
				&& !Binding.CapabilityName.IsNone())
			{
				OutError = FString::Printf(
					TEXT("Capability를 요구하지 않는 Role에 raw capability name이 있습니다: %s"),
					*Binding.RoleId);
				return false;
			}
			if (Role->CapabilityKind != ECFResourceCapabilityKind::None
				&& Binding.CapabilityName.IsNone())
			{
				OutError = FString::Printf(
					TEXT("Capability Role binding에는 exact capability name이 필요합니다: %s"),
					*Binding.RoleId);
				return false;
			}
		}
	}

	// Registered immutable-style profile copy입니다.
	FCFResourceProfile ProfileCopy = Profile;
	ProfilesById.Add(Profile.ProfileId, MoveTemp(ProfileCopy));
	return true;
}

const FCFResourceProfile* FCFResourceProfileRegistry::FindProfile(
	const FString& ProfileId) const
{
	return ProfilesById.Find(ProfileId);
}

bool FCFResourceBindingResolver::Resolve(
	const FCFResourceBindingRequest& Request,
	const FCFSemanticRoleRegistry& RoleRegistry,
	const FCFResourceProfileRegistry& ProfileRegistry,
	const FCFResourceCatalog& ResourceCatalog,
	const FCFResourcePickerRegistry& PickerRegistry,
	FCFResourceBindingResult& OutResult)
{
	OutResult = FCFResourceBindingResult();

	// Requested unique RoleId set입니다.
	TSet<FString> RequestedRoleSet;
	for (const FString& RoleId : Request.RequestedRoleIds)
	{
		if (!CFContentResourcePrivate::IsStableSemanticId(RoleId))
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("InvalidRequestedRole"),
				FString::Printf(TEXT("Role/%s"), *RoleId),
				TEXT("Requested RoleId가 canonical stable ID가 아닙니다."));
			continue;
		}
		if (RequestedRoleSet.Contains(RoleId))
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("DuplicateRequestedRole"),
				FString::Printf(TEXT("Role/%s"), *RoleId),
				TEXT("RequestedRoleIds에 duplicate RoleId가 있습니다."));
			continue;
		}
		if (RoleRegistry.FindRole(RoleId) == nullptr)
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("UnknownSemanticRole"),
				FString::Printf(TEXT("Role/%s"), *RoleId),
				TEXT("Requested semantic RoleId가 registry에 없습니다."));
			continue;
		}
		RequestedRoleSet.Add(RoleId);
	}

	// Optional base profile입니다.
	const FCFResourceProfile* Profile = nullptr;
	if (!Request.ProfileId.IsEmpty())
	{
		if (!CFContentResourcePrivate::IsStableSemanticId(Request.ProfileId))
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("InvalidProfileId"),
				TEXT("Profile"),
				TEXT("Request ProfileId가 canonical stable ID가 아닙니다."));
		}
		else
		{
			Profile = ProfileRegistry.FindProfile(Request.ProfileId);
			if (Profile == nullptr)
			{
				CFContentResourcePrivate::AddIssue(
					OutResult.Issues,
					TEXT("MissingProfile"),
					TEXT("Profile"),
					TEXT("Request ProfileId가 registry에 없습니다."));
			}
		}
	}

	// Override duplicate RoleId 검사용 set입니다.
	TSet<FString> OverrideRoleIds;
	for (const FCFResourceBindingOverride& Override : Request.Overrides)
	{
		if (!RequestedRoleSet.Contains(Override.RoleId))
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("OverrideRoleNotRequested"),
				FString::Printf(TEXT("Role/%s"), *Override.RoleId),
				TEXT("Override는 RequestedRoleIds 안의 semantic role만 대상으로 할 수 있습니다."));
			continue;
		}
		if (OverrideRoleIds.Contains(Override.RoleId))
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("DuplicateRoleOverride"),
				FString::Printf(TEXT("Role/%s"), *Override.RoleId),
				TEXT("동일 semantic role에 override가 두 번 이상 지정됐습니다."));
			continue;
		}
		OverrideRoleIds.Add(Override.RoleId);

		// Override 대상 role contract입니다.
		const FCFSemanticRoleDescriptor* Role =
			RoleRegistry.FindRole(Override.RoleId);
		if (Role == nullptr)
		{
			continue;
		}

		if (Override.Mode == ECFResourceOverrideMode::Clear)
		{
			if (Role->Requirement == ECFResourceRequirement::Required)
			{
				CFContentResourcePrivate::AddIssue(
					OutResult.Issues,
					TEXT("RequiredRoleClear"),
					FString::Printf(TEXT("Role/%s"), *Override.RoleId),
					TEXT("Required semantic role은 override로 Clear할 수 없습니다."));
			}
			continue;
		}

		if (!CFIsStableResourceId(Override.ResourceId))
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("InvalidOverrideResourceId"),
				FString::Printf(TEXT("Role/%s"), *Override.RoleId),
				TEXT("Replace override에는 canonical ResourceId가 필요합니다."));
		}
		if (Role->CapabilityKind == ECFResourceCapabilityKind::None
			&& !Override.CapabilityName.IsNone())
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("UnexpectedOverrideCapability"),
				FString::Printf(TEXT("Role/%s"), *Override.RoleId),
				TEXT("Capability를 요구하지 않는 role override에 raw capability name을 둘 수 없습니다."));
		}
		if (Role->CapabilityKind != ECFResourceCapabilityKind::None
			&& Override.CapabilityName.IsNone())
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("MissingOverrideCapability"),
				FString::Printf(TEXT("Role/%s"), *Override.RoleId),
				TEXT("Capability role Replace override에는 exact capability name이 필요합니다."));
		}
	}

	if (CFContentResourcePrivate::HasBlockingIssues(OutResult.Issues))
	{
		OutResult.bSucceeded = false;
		return false;
	}

	// Deterministic RoleId resolve order입니다.
	TArray<FString> SortedRoleIds = Request.RequestedRoleIds;
	SortedRoleIds.Sort();

	for (const FString& RoleId : SortedRoleIds)
	{
		// Exact semantic role contract입니다.
		const FCFSemanticRoleDescriptor* Role =
			RoleRegistry.FindRole(RoleId);
		if (Role == nullptr)
		{
			continue;
		}

		// 이번 role의 final resolved output입니다.
		FCFResolvedResourceBinding Resolved;
		Resolved.RoleId = RoleId;
		Resolved.Requirement = Role->Requirement;
		Resolved.CapabilityKind = Role->CapabilityKind;

		// Profile layer의 base binding입니다.
		const FCFResourceBindingValue* ProfileBinding =
			Profile != nullptr
				? CFContentResourcePrivate::FindProfileBinding(*Profile, RoleId)
				: nullptr;
		// Per-content override layer입니다.
		const FCFResourceBindingOverride* Override =
			CFContentResourcePrivate::FindOverride(
				Request.Overrides,
				RoleId);

		// Final binding source data입니다.
		FString ResourceId;
		// Final capability name입니다.
		FName CapabilityName = NAME_None;
		// Final source layer입니다.
		ECFResourceBindingSource Source =
			ECFResourceBindingSource::None;
		// Clear override로 explicit unbound가 됐는지 여부입니다.
		bool bExplicitlyCleared = false;

		if (ProfileBinding != nullptr)
		{
			Source = ECFResourceBindingSource::Profile;
			ResourceId = ProfileBinding->ResourceId;
			CapabilityName = ProfileBinding->CapabilityName;
		}
		if (Override != nullptr)
		{
			Source = ECFResourceBindingSource::Override;
			if (Override->Mode == ECFResourceOverrideMode::Clear)
			{
				ResourceId.Reset();
				CapabilityName = NAME_None;
				bExplicitlyCleared = true;
			}
			else
			{
				ResourceId = Override->ResourceId;
				CapabilityName = Override->CapabilityName;
			}
		}

		if (ResourceId.IsEmpty())
		{
			if (Role->Requirement == ECFResourceRequirement::Required)
			{
				CFContentResourcePrivate::AddIssue(
					OutResult.Issues,
					TEXT("MissingRequiredResource"),
					FString::Printf(TEXT("Role/%s"), *RoleId),
					TEXT("Required semantic role에 resolved ResourceId가 없습니다."));
			}
			else
			{
				CFContentResourcePrivate::AddIssue(
					OutResult.Issues,
					bExplicitlyCleared
						? TEXT("OptionalResourceCleared")
						: TEXT("OptionalResourceUnbound"),
					FString::Printf(TEXT("Role/%s"), *RoleId),
					TEXT("Optional semantic role은 resource 없이 resolve됐습니다."),
					false);
			}

			Resolved.bBound = false;
			Resolved.Source = Source;
			OutResult.Bindings.Add(MoveTemp(Resolved));
			continue;
		}

		// Final ResourceId fresh validation issues입니다.
		TArray<FCFContentValidationIssue> ResourceIssues;
		if (!ResourceCatalog.ValidateResource(
			ResourceId,
			PickerRegistry,
			ResourceIssues))
		{
			CFContentResourcePrivate::AppendRoleScopedIssues(
				RoleId,
				Role->Requirement,
				ResourceIssues,
				OutResult.Issues);
			Resolved.bBound = false;
			Resolved.Source = Source;
			OutResult.Bindings.Add(MoveTemp(Resolved));
			continue;
		}

		if (Role->CapabilityKind != ECFResourceCapabilityKind::None)
		{
			// Mesh capability snapshot입니다.
			FCFResourceCapabilitySnapshot CapabilitySnapshot;
			// Mesh capability scan diagnostics입니다.
			TArray<FCFContentValidationIssue> CapabilityIssues;
			if (!FCFMeshCapabilityScanner::ScanResource(
				ResourceId,
				ResourceCatalog,
				PickerRegistry,
				CapabilitySnapshot,
				CapabilityIssues))
			{
				CFContentResourcePrivate::AppendRoleScopedIssues(
					RoleId,
					Role->Requirement,
					CapabilityIssues,
					OutResult.Issues);
				Resolved.bBound = false;
				Resolved.Source = Source;
				OutResult.Bindings.Add(MoveTemp(Resolved));
				continue;
			}

			if (!FCFMeshCapabilityScanner::HasCapability(
				CapabilitySnapshot,
				Role->CapabilityKind,
				CapabilityName))
			{
				CFContentResourcePrivate::AddIssue(
					OutResult.Issues,
					Role->Requirement == ECFResourceRequirement::Required
						? TEXT("MissingRequiredCapability")
						: TEXT("OptionalCapabilityUnavailable"),
					FString::Printf(TEXT("Role/%s"), *RoleId),
					FString::Printf(
						TEXT("Resource '%s'에 semantic role이 요구하는 capability '%s'가 없습니다."),
						*ResourceId,
						*CapabilityName.ToString()),
					Role->Requirement == ECFResourceRequirement::Required);
				Resolved.bBound = false;
				Resolved.Source = Source;
				OutResult.Bindings.Add(MoveTemp(Resolved));
				continue;
			}
		}

		// Final catalog descriptor입니다.
		const FCFResourceDescriptor* Resource =
			ResourceCatalog.FindResource(ResourceId);
		if (Resource == nullptr)
		{
			CFContentResourcePrivate::AddIssue(
				OutResult.Issues,
				TEXT("MissingCatalogResource"),
				FString::Printf(TEXT("Role/%s"), *RoleId),
				TEXT("Validation 직후 Resource Catalog lookup이 일치하지 않습니다."));
			OutResult.Bindings.Add(MoveTemp(Resolved));
			continue;
		}

		Resolved.bBound = true;
		Resolved.Source = Source;
		Resolved.ResourceId = ResourceId;
		Resolved.ObjectPath = Resource->ObjectPath;
		Resolved.CapabilityName = CapabilityName;
		OutResult.Bindings.Add(MoveTemp(Resolved));
	}

	OutResult.bSucceeded =
		!CFContentResourcePrivate::HasBlockingIssues(OutResult.Issues);
	return OutResult.bSucceeded;
}

bool FCFContentResourceValidator::ValidateWorkbook(
	const FCFContentWorkbookModel& Workbook,
	const FCFResourceCatalog& ResourceCatalog,
	const FCFResourcePickerRegistry& PickerRegistry,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	for (const FCFContentRecord& Record : Workbook.Records)
	{
		// Stable record path입니다.
		const FString RecordPath = Record.Key.ToStableString();
		for (const TPair<FString, FCFContentValue>& FieldPair : Record.Fields)
		{
			CFContentResourcePrivate::ValidateResourceReferenceValue(
				FieldPair.Value,
				FString::Printf(
					TEXT("%s/%s"),
					*RecordPath,
					*FieldPair.Key),
				ResourceCatalog,
				PickerRegistry,
				OutIssues);
		}

		for (const TPair<FString, FCFContentCollection>& CollectionPair : Record.Collections)
		{
			for (const FCFContentCollectionItem& Item : CollectionPair.Value.Items)
			{
				for (const TPair<FString, FCFContentValue>& FieldPair : Item.Fields)
				{
					CFContentResourcePrivate::ValidateResourceReferenceValue(
						FieldPair.Value,
						FString::Printf(
							TEXT("%s/%s/%s/%s"),
							*RecordPath,
							*CollectionPair.Key,
							*Item.ChildItemId,
							*FieldPair.Key),
						ResourceCatalog,
						PickerRegistry,
						OutIssues);
				}
			}
		}
	}

	return !CFContentResourcePrivate::HasBlockingIssues(OutIssues);
}
