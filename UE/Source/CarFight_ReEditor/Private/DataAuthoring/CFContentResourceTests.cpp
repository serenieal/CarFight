// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentResourceTests.cpp
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS-P0-03 Resource Catalog + Semantic Binding focused Automation입니다.
// Changelog:
// - v1.2.0: P0-06 fresh implementation review 회귀로 ResourceCatalogFingerprint가 missing/unready state도 fingerprint + diagnostic으로 보존하는지 AssetValidation에 추가.
// - v1.1.0: Fresh review correction의 unbound Source=None semantics를 RequiredOptionalPolicy에 고정.
// - v1.0.0: StableResourceId/PickerRegistry/AssetValidation/MeshCapabilities/SemanticRoleBinding/
//   ProfileOverride/RequiredOptionalPolicy/WorkbookResourceValidation exact8 최초 추가.
// Migration:
// - /Engine/BasicShapes/Cube.Cube read-only fixture와 in-memory registries만 사용하며 Product asset을 저장하지 않습니다.

#include "DataAuthoring/CFContentResource.h"
#include "DataAuthoring/CFDACommonPrimitives.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Misc/AutomationTest.h"

namespace CFContentResourceTestsPrivate
{
	/** StaticMesh Asset Picker registration을 생성합니다. */
	bool RegisterStaticMeshPicker(
		FCFResourcePickerRegistry& OutRegistry,
		FString& OutError)
	{
		// StaticMesh picker descriptor입니다.
		FCFResourcePickerDescriptor Picker;
		Picker.PickerTypeId = TEXT("picker.static_mesh");
		Picker.AllowedBaseClassPath =
			UStaticMesh::StaticClass()->GetClassPathName();
		Picker.bAllowClear = true;
		return OutRegistry.RegisterPicker(Picker, OutError);
	}

	/** SkeletalMesh Asset Picker registration을 생성합니다. */
	bool RegisterSkeletalMeshPicker(
		FCFResourcePickerRegistry& OutRegistry,
		FString& OutError)
	{
		// SkeletalMesh picker descriptor입니다.
		FCFResourcePickerDescriptor Picker;
		Picker.PickerTypeId = TEXT("picker.skeletal_mesh");
		Picker.AllowedBaseClassPath =
			USkeletalMesh::StaticClass()->GetClassPathName();
		Picker.bAllowClear = true;
		return OutRegistry.RegisterPicker(Picker, OutError);
	}

	/** Engine Cube를 Stable ResourceId로 catalog에 등록합니다. */
	bool RegisterCubeResource(
		const FString& ResourceId,
		const FString& PickerTypeId,
		const FCFResourcePickerRegistry& PickerRegistry,
		FCFResourceCatalog& OutCatalog,
		FString& OutError)
	{
		// Engine Cube resource descriptor입니다.
		FCFResourceDescriptor Resource;
		Resource.ResourceId = ResourceId;
		Resource.PickerTypeId = PickerTypeId;
		Resource.ObjectPath =
			FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube"));
		return OutCatalog.RegisterResource(
			Resource,
			PickerRegistry,
			OutError);
	}

	/** Required/Optional semantic role를 registry에 등록합니다. */
	bool RegisterRole(
		const FString& RoleId,
		const ECFResourceRequirement Requirement,
		const ECFResourceCapabilityKind CapabilityKind,
		FCFSemanticRoleRegistry& OutRegistry,
		FString& OutError)
	{
		// Semantic role descriptor입니다.
		FCFSemanticRoleDescriptor Role;
		Role.RoleId = RoleId;
		Role.Requirement = Requirement;
		Role.CapabilityKind = CapabilityKind;
		return OutRegistry.RegisterRole(Role, OutError);
	}

	/** Role 하나를 resource에 매핑하는 profile을 등록합니다. */
	bool RegisterProfile(
		const FString& ProfileId,
		const FString& RoleId,
		const FString& ResourceId,
		const FName CapabilityName,
		const FCFSemanticRoleRegistry& RoleRegistry,
		FCFResourceProfileRegistry& OutRegistry,
		FString& OutError)
	{
		// Profile binding입니다.
		FCFResourceBindingValue Binding;
		Binding.RoleId = RoleId;
		Binding.ResourceId = ResourceId;
		Binding.CapabilityName = CapabilityName;

		// Resource profile입니다.
		FCFResourceProfile Profile;
		Profile.ProfileId = ProfileId;
		Profile.Bindings.Add(Binding);
		return OutRegistry.RegisterProfile(
			Profile,
			RoleRegistry,
			OutError);
	}

	/** Exact RoleId의 resolved binding을 찾습니다. */
	const FCFResolvedResourceBinding* FindResolved(
		const FCFResourceBindingResult& Result,
		const FString& RoleId)
	{
		return Result.Bindings.FindByPredicate(
			[&RoleId](const FCFResolvedResourceBinding& Binding)
			{
				return Binding.RoleId.Equals(
					RoleId,
					ESearchCase::CaseSensitive);
			});
	}

	/** Exact diagnostic code 존재 여부를 확인합니다. */
	bool HasIssueCode(
		const TArray<FCFContentValidationIssue>& Issues,
		const FString& Code,
		const bool bBlocking)
	{
		return Issues.ContainsByPredicate(
			[&Code, bBlocking](const FCFContentValidationIssue& Issue)
			{
				return Issue.Code.Equals(
					Code,
					ESearchCase::CaseSensitive)
					&& Issue.bBlocking == bBlocking;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourceStableIdTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.StableResourceId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourcePickerRegistryTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.PickerRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourceAssetValidationTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.AssetValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourceMeshCapabilitiesTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.MeshCapabilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourceSemanticBindingTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.SemanticRoleBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourceProfileOverrideTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.ProfileOverride",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFResourceRequiredOptionalTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.RequiredOptionalPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWorkbookResourceValidationTest,
	"CarFight.CCAS.CF_FQ_058.P0_03.WorkbookResourceValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/** Stable ResourceId가 raw Unreal ObjectPath와 분리된 machine identity인지 검증합니다. */
bool FCFResourceStableIdTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TestTrue(
		TEXT("Stable ResourceId accepts machine identity"),
		CFIsStableResourceId(TEXT("mesh.vehicle.body")));
	TestTrue(
		TEXT("Nested stable ResourceId accepts slash inside identity"),
		CFIsStableResourceId(TEXT("mesh/vehicle/body")));
	TestFalse(
		TEXT("Raw Unreal ObjectPath must not be accepted as ResourceId"),
		CFIsStableResourceId(TEXT("/Engine/BasicShapes/Cube.Cube")));
	TestFalse(
		TEXT("Colon token must fail stable ResourceId grammar"),
		CFIsStableResourceId(TEXT("mesh:vehicle:body")));
	return true;
}

/** Asset Picker registration의 duplicate guard와 AllowedClass resolve를 검증합니다. */
bool FCFResourcePickerRegistryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Registry;
	// Registration error입니다.
	FString Error;
	TestTrue(
		TEXT("StaticMesh picker registration succeeds"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Registry,
			Error));
	TestEqual(
		TEXT("Picker registry count exact1"),
		Registry.Num(),
		1);
	TestFalse(
		TEXT("Duplicate picker registration fails closed"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Registry,
			Error));

	// Resolved Unreal class입니다.
	UClass* AllowedClass =
		Registry.ResolveAllowedClass(
			TEXT("picker.static_mesh"),
			Error);
	TestEqual(
		TEXT("Picker AllowedClass resolves to UStaticMesh"),
		AllowedClass,
		UStaticMesh::StaticClass());
	return true;
}

/** Resource Catalog가 Asset Registry existence/type mismatch를 fail-closed하는지 검증합니다. */
bool FCFResourceAssetValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Pickers;
	// Registration error입니다.
	FString Error;
	TestTrue(
		TEXT("StaticMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Pickers,
			Error));
	TestTrue(
		TEXT("SkeletalMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterSkeletalMeshPicker(
			Pickers,
			Error));

	// Valid resource catalog입니다.
	FCFResourceCatalog Catalog;
	TestTrue(
		TEXT("Engine Cube registers as StaticMesh resource"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.engine.cube"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));

	// Wrong class resource descriptor입니다.
	FCFResourceDescriptor WrongClassResource;
	WrongClassResource.ResourceId = TEXT("mesh.engine.cube_as_skeletal");
	WrongClassResource.PickerTypeId = TEXT("picker.skeletal_mesh");
	WrongClassResource.ObjectPath =
		FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube"));
	TestFalse(
		TEXT("StaticMesh cannot register under SkeletalMesh picker"),
		Catalog.RegisterResource(
			WrongClassResource,
			Pickers,
			Error));

	// Missing asset resource descriptor입니다.
	FCFResourceDescriptor MissingResource;
	MissingResource.ResourceId = TEXT("mesh.missing.fixture");
	MissingResource.PickerTypeId = TEXT("picker.static_mesh");
	MissingResource.ObjectPath =
		FSoftObjectPath(TEXT("/Game/CarFight/DoesNotExist.DoesNotExist"));
	TestFalse(
		TEXT("Missing asset metadata fails resource registration"),
		Catalog.RegisterResource(
			MissingResource,
			Pickers,
			Error));
	TestEqual(
		TEXT("Only valid resource remains registered"),
		Catalog.Num(),
		1);

	// Ready Resource Catalog fingerprint입니다.
	FString ReadyFingerprint;
	// Ready fingerprint diagnostics입니다.
	TArray<FCFContentValidationIssue> ReadyFingerprintIssues;
	TestTrue(
		TEXT("Ready Resource Catalog fingerprint succeeds"),
		Catalog.BuildSemanticFingerprint(
			Pickers,
			ReadyFingerprint,
			ReadyFingerprintIssues));
	TestTrue(
		TEXT("Ready Resource Catalog fingerprint is canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ReadyFingerprint));

	// 등록 뒤 picker가 사라진 상태를 모사하는 empty registry입니다.
	FCFResourcePickerRegistry MissingPickerRegistry;
	// Missing/unready state fingerprint입니다.
	FString MissingPickerFingerprint;
	// Missing/unready state diagnostics입니다.
	TArray<FCFContentValidationIssue> MissingPickerIssues;
	TestTrue(
		TEXT("Missing picker state still produces a snapshot fingerprint"),
		Catalog.BuildSemanticFingerprint(
			MissingPickerRegistry,
			MissingPickerFingerprint,
			MissingPickerIssues));
	TestTrue(
		TEXT("Missing picker state fingerprint is canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(MissingPickerFingerprint));
	TestNotEqual(
		TEXT("Resource readiness change alters fingerprint"),
		MissingPickerFingerprint,
		ReadyFingerprint);
	TestTrue(
		TEXT("Missing picker diagnostic remains blocking evidence"),
		CFContentResourceTestsPrivate::HasIssueCode(
			MissingPickerIssues,
			TEXT("MissingResourcePicker"),
			true));
	return true;
}

/** StaticMesh scan과 Socket/Bone/MaterialSlot capability lookup contract를 검증합니다. */
bool FCFResourceMeshCapabilitiesTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Pickers;
	// Disposable resource catalog입니다.
	FCFResourceCatalog Catalog;
	// Registration error입니다.
	FString Error;
	TestTrue(
		TEXT("StaticMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Pickers,
			Error));
	TestTrue(
		TEXT("Cube resource registration"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.engine.cube"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));

	// Fresh Engine Cube mesh capability snapshot입니다.
	FCFResourceCapabilitySnapshot Snapshot;
	// Capability scan diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(
		TEXT("StaticMesh capability scan succeeds"),
		FCFMeshCapabilityScanner::ScanResource(
			TEXT("mesh.engine.cube"),
			Catalog,
			Pickers,
			Snapshot,
			Issues));
	TestTrue(
		TEXT("Cube is recognized as mesh asset"),
		Snapshot.bMeshAsset);
	TestTrue(
		TEXT("Cube exposes at least one material slot"),
		Snapshot.MaterialSlotNames.Num() > 0);

	// Synthetic exact capability snapshot으로 three-kind lookup semantics를 고정합니다.
	FCFResourceCapabilitySnapshot CapabilityFixture;
	CapabilityFixture.ResourceId = TEXT("mesh.fixture");
	CapabilityFixture.bMeshAsset = true;
	CapabilityFixture.SocketNames.Add(FName(TEXT("Muzzle")));
	CapabilityFixture.BoneNames.Add(FName(TEXT("root")));
	CapabilityFixture.MaterialSlotNames.Add(FName(TEXT("Body")));

	TestTrue(
		TEXT("Socket capability lookup"),
		FCFMeshCapabilityScanner::HasCapability(
			CapabilityFixture,
			ECFResourceCapabilityKind::Socket,
			FName(TEXT("Muzzle"))));
	TestTrue(
		TEXT("Bone capability lookup"),
		FCFMeshCapabilityScanner::HasCapability(
			CapabilityFixture,
			ECFResourceCapabilityKind::Bone,
			FName(TEXT("root"))));
	TestTrue(
		TEXT("MaterialSlot capability lookup"),
		FCFMeshCapabilityScanner::HasCapability(
			CapabilityFixture,
			ECFResourceCapabilityKind::MaterialSlot,
			FName(TEXT("Body"))));
	TestFalse(
		TEXT("Unknown Socket capability fails"),
		FCFMeshCapabilityScanner::HasCapability(
			CapabilityFixture,
			ECFResourceCapabilityKind::Socket,
			FName(TEXT("Unknown"))));
	return true;
}

/** Profile semantic role binding이 Stable ResourceId를 canonical asset path로 resolve하는지 검증합니다. */
bool FCFResourceSemanticBindingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Pickers;
	// Disposable resource catalog입니다.
	FCFResourceCatalog Catalog;
	// Disposable semantic role registry입니다.
	FCFSemanticRoleRegistry Roles;
	// Disposable profile registry입니다.
	FCFResourceProfileRegistry Profiles;
	// Registration error입니다.
	FString Error;

	TestTrue(
		TEXT("StaticMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Pickers,
			Error));
	TestTrue(
		TEXT("Cube resource registration"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.vehicle.body"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));
	TestTrue(
		TEXT("Required body role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("visual.body"),
			ECFResourceRequirement::Required,
			ECFResourceCapabilityKind::None,
			Roles,
			Error));
	TestTrue(
		TEXT("Body profile registration"),
		CFContentResourceTestsPrivate::RegisterProfile(
			TEXT("profile.vehicle.default"),
			TEXT("visual.body"),
			TEXT("mesh.vehicle.body"),
			NAME_None,
			Roles,
			Profiles,
			Error));

	// One-role profile resolution request입니다.
	FCFResourceBindingRequest Request;
	Request.RequestedRoleIds.Add(TEXT("visual.body"));
	Request.ProfileId = TEXT("profile.vehicle.default");

	// Resolution result입니다.
	FCFResourceBindingResult Result;
	TestTrue(
		TEXT("Required profile binding resolves"),
		FCFResourceBindingResolver::Resolve(
			Request,
			Roles,
			Profiles,
			Catalog,
			Pickers,
			Result));

	// Resolved body role입니다.
	const FCFResolvedResourceBinding* Binding =
		CFContentResourceTestsPrivate::FindResolved(
			Result,
			TEXT("visual.body"));
	TestNotNull(
		TEXT("Body role has resolved row"),
		Binding);
	if (Binding != nullptr)
	{
		TestTrue(
			TEXT("Body role is bound"),
			Binding->bBound);
		TestEqual(
			TEXT("Body role keeps stable ResourceId"),
			Binding->ResourceId,
			FString(TEXT("mesh.vehicle.body")));
		TestEqual(
			TEXT("Body role resolves canonical object path"),
			Binding->ObjectPath.ToString(),
			FString(TEXT("/Engine/BasicShapes/Cube.Cube")));
		TestEqual(
			TEXT("Binding source is Profile"),
			static_cast<uint8>(Binding->Source),
			static_cast<uint8>(ECFResourceBindingSource::Profile));
	}
	return true;
}

/** Per-content override가 Profile보다 우선하고 optional Clear가 허용되는지 검증합니다. */
bool FCFResourceProfileOverrideTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Pickers;
	// Disposable resource catalog입니다.
	FCFResourceCatalog Catalog;
	// Disposable role registry입니다.
	FCFSemanticRoleRegistry Roles;
	// Disposable profile registry입니다.
	FCFResourceProfileRegistry Profiles;
	// Registration error입니다.
	FString Error;

	TestTrue(
		TEXT("StaticMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Pickers,
			Error));
	TestTrue(
		TEXT("Base cube resource registration"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.vehicle.base"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));
	TestTrue(
		TEXT("Override cube resource registration"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.vehicle.override"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));
	TestTrue(
		TEXT("Required body role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("visual.body"),
			ECFResourceRequirement::Required,
			ECFResourceCapabilityKind::None,
			Roles,
			Error));
	TestTrue(
		TEXT("Optional decal role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("visual.decal"),
			ECFResourceRequirement::Optional,
			ECFResourceCapabilityKind::None,
			Roles,
			Error));

	// Profile base body binding입니다.
	FCFResourceBindingValue BodyBinding;
	BodyBinding.RoleId = TEXT("visual.body");
	BodyBinding.ResourceId = TEXT("mesh.vehicle.base");
	// Profile base decal binding입니다.
	FCFResourceBindingValue DecalBinding;
	DecalBinding.RoleId = TEXT("visual.decal");
	DecalBinding.ResourceId = TEXT("mesh.vehicle.base");
	// Two-role profile입니다.
	FCFResourceProfile Profile;
	Profile.ProfileId = TEXT("profile.vehicle.shared");
	Profile.Bindings.Add(BodyBinding);
	Profile.Bindings.Add(DecalBinding);
	TestTrue(
		TEXT("Shared profile registration"),
		Profiles.RegisterProfile(
			Profile,
			Roles,
			Error));

	// Required body Replace override입니다.
	FCFResourceBindingOverride BodyOverride;
	BodyOverride.RoleId = TEXT("visual.body");
	BodyOverride.Mode = ECFResourceOverrideMode::Replace;
	BodyOverride.ResourceId = TEXT("mesh.vehicle.override");
	// Optional decal Clear override입니다.
	FCFResourceBindingOverride DecalOverride;
	DecalOverride.RoleId = TEXT("visual.decal");
	DecalOverride.Mode = ECFResourceOverrideMode::Clear;

	// Profile + override request입니다.
	FCFResourceBindingRequest Request;
	Request.RequestedRoleIds = {
		TEXT("visual.body"),
		TEXT("visual.decal")
	};
	Request.ProfileId = Profile.ProfileId;
	Request.Overrides = {
		BodyOverride,
		DecalOverride
	};

	// Resolution result입니다.
	FCFResourceBindingResult Result;
	TestTrue(
		TEXT("Profile + override resolves without blocking issue"),
		FCFResourceBindingResolver::Resolve(
			Request,
			Roles,
			Profiles,
			Catalog,
			Pickers,
			Result));

	// Overridden body result입니다.
	const FCFResolvedResourceBinding* Body =
		CFContentResourceTestsPrivate::FindResolved(
			Result,
			TEXT("visual.body"));
	TestNotNull(
		TEXT("Body result exists"),
		Body);
	if (Body != nullptr)
	{
		TestEqual(
			TEXT("Override wins over profile"),
			Body->ResourceId,
			FString(TEXT("mesh.vehicle.override")));
		TestEqual(
			TEXT("Body source is Override"),
			static_cast<uint8>(Body->Source),
			static_cast<uint8>(ECFResourceBindingSource::Override));
	}

	// Cleared optional decal result입니다.
	const FCFResolvedResourceBinding* Decal =
		CFContentResourceTestsPrivate::FindResolved(
			Result,
			TEXT("visual.decal"));
	TestNotNull(
		TEXT("Optional decal result exists"),
		Decal);
	if (Decal != nullptr)
	{
		TestFalse(
			TEXT("Optional Clear produces unbound role"),
			Decal->bBound);
	}
	TestTrue(
		TEXT("Optional Clear is preserved as non-blocking diagnostic"),
		CFContentResourceTestsPrivate::HasIssueCode(
			Result.Issues,
			TEXT("OptionalResourceCleared"),
			false));
	return true;
}

/** Required role은 fail-closed, Optional role은 unavailable resource/capability를 non-blocking 처리하는지 검증합니다. */
bool FCFResourceRequiredOptionalTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Pickers;
	// Disposable resource catalog입니다.
	FCFResourceCatalog Catalog;
	// Disposable role registry입니다.
	FCFSemanticRoleRegistry Roles;
	// Empty profile registry입니다.
	FCFResourceProfileRegistry Profiles;
	// Registration error입니다.
	FString Error;

	TestTrue(
		TEXT("StaticMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Pickers,
			Error));
	TestTrue(
		TEXT("Cube resource registration"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.weapon.fixture"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));
	TestTrue(
		TEXT("Required body role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("visual.required"),
			ECFResourceRequirement::Required,
			ECFResourceCapabilityKind::None,
			Roles,
			Error));
	TestTrue(
		TEXT("Optional FX role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("visual.optional"),
			ECFResourceRequirement::Optional,
			ECFResourceCapabilityKind::None,
			Roles,
			Error));
	TestTrue(
		TEXT("Required socket role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("mount.required_socket"),
			ECFResourceRequirement::Required,
			ECFResourceCapabilityKind::Socket,
			Roles,
			Error));
	TestTrue(
		TEXT("Optional socket role registration"),
		CFContentResourceTestsPrivate::RegisterRole(
			TEXT("mount.optional_socket"),
			ECFResourceRequirement::Optional,
			ECFResourceCapabilityKind::Socket,
			Roles,
			Error));

	{
		// Missing required resource request입니다.
		FCFResourceBindingRequest Request;
		Request.RequestedRoleIds.Add(TEXT("visual.required"));
		// Required policy result입니다.
		FCFResourceBindingResult Result;
		TestFalse(
			TEXT("Missing required role fails closed"),
			FCFResourceBindingResolver::Resolve(
				Request,
				Roles,
				Profiles,
				Catalog,
				Pickers,
				Result));
		TestTrue(
			TEXT("Missing required role emits blocking diagnostic"),
			CFContentResourceTestsPrivate::HasIssueCode(
				Result.Issues,
				TEXT("MissingRequiredResource"),
				true));
	}

	{
		// Missing optional resource request입니다.
		FCFResourceBindingRequest Request;
		Request.RequestedRoleIds.Add(TEXT("visual.optional"));
		// Optional policy result입니다.
		FCFResourceBindingResult Result;
		TestTrue(
			TEXT("Missing optional role remains successful"),
			FCFResourceBindingResolver::Resolve(
				Request,
				Roles,
				Profiles,
				Catalog,
				Pickers,
				Result));
		TestTrue(
			TEXT("Missing optional role emits non-blocking diagnostic"),
			CFContentResourceTestsPrivate::HasIssueCode(
				Result.Issues,
				TEXT("OptionalResourceUnbound"),
				false));
		// Unbound optional role result입니다.
		const FCFResolvedResourceBinding* Binding =
			CFContentResourceTestsPrivate::FindResolved(
				Result,
				TEXT("visual.optional"));
		TestNotNull(
			TEXT("Optional unbound role has resolved row"),
			Binding);
		if (Binding != nullptr)
		{
			TestEqual(
				TEXT("Optional unbound role source is None"),
				static_cast<uint8>(Binding->Source),
				static_cast<uint8>(ECFResourceBindingSource::None));
		}
	}

	{
		// Required socket Replace override입니다.
		FCFResourceBindingOverride Override;
		Override.RoleId = TEXT("mount.required_socket");
		Override.Mode = ECFResourceOverrideMode::Replace;
		Override.ResourceId = TEXT("mesh.weapon.fixture");
		Override.CapabilityName = FName(TEXT("SocketThatDoesNotExist"));
		// Required socket request입니다.
		FCFResourceBindingRequest Request;
		Request.RequestedRoleIds.Add(TEXT("mount.required_socket"));
		Request.Overrides.Add(Override);
		// Required socket result입니다.
		FCFResourceBindingResult Result;
		TestFalse(
			TEXT("Missing required mesh capability fails closed"),
			FCFResourceBindingResolver::Resolve(
				Request,
				Roles,
				Profiles,
				Catalog,
				Pickers,
				Result));
		TestTrue(
			TEXT("Missing required capability is blocking"),
			CFContentResourceTestsPrivate::HasIssueCode(
				Result.Issues,
				TEXT("MissingRequiredCapability"),
				true));
	}

	{
		// Optional socket Replace override입니다.
		FCFResourceBindingOverride Override;
		Override.RoleId = TEXT("mount.optional_socket");
		Override.Mode = ECFResourceOverrideMode::Replace;
		Override.ResourceId = TEXT("mesh.weapon.fixture");
		Override.CapabilityName = FName(TEXT("SocketThatDoesNotExist"));
		// Optional socket request입니다.
		FCFResourceBindingRequest Request;
		Request.RequestedRoleIds.Add(TEXT("mount.optional_socket"));
		Request.Overrides.Add(Override);
		// Optional socket result입니다.
		FCFResourceBindingResult Result;
		TestTrue(
			TEXT("Missing optional mesh capability remains successful"),
			FCFResourceBindingResolver::Resolve(
				Request,
				Roles,
				Profiles,
				Catalog,
				Pickers,
				Result));
		TestTrue(
			TEXT("Missing optional capability is non-blocking"),
			CFContentResourceTestsPrivate::HasIssueCode(
				Result.Issues,
				TEXT("OptionalCapabilityUnavailable"),
				false));
	}

	return true;
}

/** Canonical Workbook ResourceReference VALUE가 Resource Catalog를 통해 검증되는지 확인합니다. */
bool FCFWorkbookResourceValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable picker registry입니다.
	FCFResourcePickerRegistry Pickers;
	// Disposable resource catalog입니다.
	FCFResourceCatalog Catalog;
	// Registration error입니다.
	FString Error;
	TestTrue(
		TEXT("StaticMesh picker registration"),
		CFContentResourceTestsPrivate::RegisterStaticMeshPicker(
			Pickers,
			Error));
	TestTrue(
		TEXT("Cube resource registration"),
		CFContentResourceTestsPrivate::RegisterCubeResource(
			TEXT("mesh.vehicle.body"),
			TEXT("picker.static_mesh"),
			Pickers,
			Catalog,
			Error));

	// Canonical ResourceReference VALUE입니다.
	FCFContentValue ResourceValue;
	ResourceValue.Type = ECFContentValueType::ResourceReference;
	ResourceValue.State = ECFContentValueState::Value;
	ResourceValue.StringValue = TEXT("mesh.vehicle.body");

	// Minimal record fixture입니다.
	FCFContentRecord Record;
	Record.Key.ContentTypeId.Value = TEXT("Vehicle");
	Record.Key.ContentId = TEXT("TestVehicle");
	Record.RowId = Record.Key.ToStableString();
	Record.ManagementState = ECFContentManagementState::Managed;
	Record.Fields.Add(TEXT("BodyResource"), ResourceValue);

	// Minimal workbook fixture입니다.
	FCFContentWorkbookModel Workbook;
	Workbook.WorkbookSourceId = TEXT("CarFight.Content");
	Workbook.SchemaRevision = 1;
	Workbook.Records.Add(Record);

	// Workbook resource diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(
		TEXT("Known ResourceReference validates through catalog"),
		FCFContentResourceValidator::ValidateWorkbook(
			Workbook,
			Catalog,
			Pickers,
			Issues));
	TestEqual(
		TEXT("Known ResourceReference has no issue"),
		Issues.Num(),
		0);

	Workbook.Records[0].Fields[TEXT("BodyResource")].StringValue =
		TEXT("mesh.vehicle.missing");
	TestFalse(
		TEXT("Unknown ResourceReference fails catalog validation"),
		FCFContentResourceValidator::ValidateWorkbook(
			Workbook,
			Catalog,
			Pickers,
			Issues));
	TestTrue(
		TEXT("Missing catalog resource diagnostic is blocking"),
		CFContentResourceTestsPrivate::HasIssueCode(
			Issues,
			TEXT("MissingCatalogResource"),
			true));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
