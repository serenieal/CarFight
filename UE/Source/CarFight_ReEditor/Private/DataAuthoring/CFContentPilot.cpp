// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentPilot.cpp
// Version: v1.1.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-04 Weapon + Vehicle Dual Consumer Pilot read-only bridge 구현입니다.
// Changelog:
// - v1.1.0: Fresh review 교정으로 Weapon/Vehicle Profile capability의 fixture-name hardcode를 제거하고 typed backend socket intent를 그대로 사용.
// - v1.0.0: CF-FQ-055 WeaponData/TurretMountData와 Vehicle SnapshotBuilder/AssetReader를 동일한
//   Resource Picker/Catalog/Semantic Role/Profile/Override Core로 연결.
// Migration:
// - Generic CFContentResource Core는 수정하지 않습니다.
// - Product Apply/Save, Workbook write, writer activation, P0-07 cutover를 수행하지 않습니다.

#include "DataAuthoring/CFContentPilot.h"

#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotTypes.h"
#include "DataAuthoring/CFVehicleAssetReader.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

namespace CFContentPilotPrivate
{
	// Pilot provider-local blocking diagnostic을 append합니다.
	void AddIssue(
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& Code,
		const FString& Path,
		const FString& Message)
	{
		// Append할 blocking issue입니다.
		FCFContentValidationIssue Issue;
		Issue.Code = Code;
		Issue.Path = Path;
		Issue.Message = Message;
		Issue.bBlocking = true;
		OutIssues.Add(MoveTemp(Issue));
	}

	// Shared Core diagnostic을 pilot result에 복사합니다.
	void AppendIssues(
		const TArray<FCFContentValidationIssue>& SourceIssues,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		for (const FCFContentValidationIssue& Issue : SourceIssues)
		{
			OutIssues.Add(Issue);
		}
	}

	// Blocking diagnostic이 존재하는지 확인합니다.
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

	// Domain identity를 Resource/Profile ID suffix로 사용할 stable lowercase token으로 정규화합니다.
	FString MakeStableToken(const FString& Source)
	{
		// 정규화된 stable token입니다.
		FString StableToken;
		StableToken.Reserve(Source.Len());

		// 연속 separator를 하나로 축약하기 위한 상태입니다.
		bool bPreviousWasSeparator = false;
		for (const TCHAR Character : Source)
		{
			// ASCII alpha-numeric 여부입니다.
			const bool bAsciiAlphaNumeric =
				(Character >= TEXT('a') && Character <= TEXT('z'))
				|| (Character >= TEXT('A') && Character <= TEXT('Z'))
				|| (Character >= TEXT('0') && Character <= TEXT('9'));

			if (bAsciiAlphaNumeric)
			{
				StableToken.AppendChar(FChar::ToLower(Character));
				bPreviousWasSeparator = false;
			}
			else if (!bPreviousWasSeparator && !StableToken.IsEmpty())
			{
				StableToken.AppendChar(TEXT('_'));
				bPreviousWasSeparator = true;
			}
		}

		while (StableToken.EndsWith(TEXT("_"), ESearchCase::CaseSensitive))
		{
			StableToken.LeftChopInline(1);
		}

		return StableToken;
	}

	// FName domain identity를 stable lowercase token으로 정규화합니다.
	FString MakeStableToken(const FName Source)
	{
		return MakeStableToken(Source.ToString());
	}

	// Recipe GUID를 stable Resource/Profile ID suffix로 변환합니다.
	FString MakeRecipeToken(const FGuid& RecipeId)
	{
		return RecipeId.ToString(EGuidFormats::Digits).ToLower();
	}

	// Fixed/dynamic semantic role을 동일 descriptor로 재사용하거나 신규 등록합니다.
	bool EnsureRole(
		FCFSemanticRoleRegistry& RoleRegistry,
		const FString& RoleId,
		const ECFResourceRequirement Requirement,
		const ECFResourceCapabilityKind CapabilityKind,
		FString& OutError)
	{
		// 이미 등록된 semantic role입니다.
		const FCFSemanticRoleDescriptor* ExistingRole = RoleRegistry.FindRole(RoleId);
		if (ExistingRole != nullptr)
		{
			if (ExistingRole->Requirement != Requirement
				|| ExistingRole->CapabilityKind != CapabilityKind)
			{
				OutError = FString::Printf(
					TEXT("Semantic role contract가 기존 등록과 다릅니다: %s"),
					*RoleId);
				return false;
			}
			return true;
		}

		// 새 semantic role descriptor입니다.
		FCFSemanticRoleDescriptor Role;
		Role.RoleId = RoleId;
		Role.Requirement = Requirement;
		Role.CapabilityKind = CapabilityKind;
		return RoleRegistry.RegisterRole(Role, OutError);
	}

	// StaticMesh object path를 Stable ResourceId로 shared catalog에 등록합니다.
	bool RegisterStaticMeshResource(
		const FString& ResourceId,
		const FSoftObjectPath& ObjectPath,
		FCFContentPilotContext& SharedContext,
		FString& OutError)
	{
		if (!ObjectPath.IsValid())
		{
			OutError = FString::Printf(
				TEXT("StaticMesh Resource path가 비어 있습니다: %s"),
				*ResourceId);
			return false;
		}

		// Shared Resource Catalog에 등록할 descriptor입니다.
		FCFResourceDescriptor Resource;
		Resource.ResourceId = ResourceId;
		Resource.PickerTypeId = TEXT("picker.static_mesh");
		Resource.ObjectPath = ObjectPath;
		return SharedContext.ResourceCatalog.RegisterResource(
			Resource,
			SharedContext.PickerRegistry,
			OutError);
	}

	// Hard UObject StaticMesh reference를 Stable ResourceId로 shared catalog에 등록합니다.
	bool RegisterStaticMeshResource(
		const FString& ResourceId,
		const UStaticMesh* Mesh,
		FCFContentPilotContext& SharedContext,
		FString& OutError)
	{
		if (Mesh == nullptr)
		{
			OutError = FString::Printf(
				TEXT("StaticMesh Resource object가 비어 있습니다: %s"),
				*ResourceId);
			return false;
		}

		// Persisted StaticMesh object path입니다.
		const FSoftObjectPath ObjectPath(Mesh->GetPathName());
		return RegisterStaticMeshResource(
			ResourceId,
			ObjectPath,
			SharedContext,
			OutError);
	}

	// Profile binding 하나를 append합니다.
	void AddProfileBinding(
		FCFResourceProfile& Profile,
		TArray<FString>& RequestedRoleIds,
		const FString& RoleId,
		const FString& ResourceId,
		const FName CapabilityName = NAME_None)
	{
		// Profile에 추가할 typed binding입니다.
		FCFResourceBindingValue Binding;
		Binding.RoleId = RoleId;
		Binding.ResourceId = ResourceId;
		Binding.CapabilityName = CapabilityName;
		Profile.Bindings.Add(MoveTemp(Binding));

		if (!RequestedRoleIds.Contains(RoleId))
		{
			RequestedRoleIds.Add(RoleId);
		}
	}

	// Per-content capability override 하나를 append합니다.
	void AddOverride(
		FCFResourceBindingRequest& Request,
		const FString& RoleId,
		const FString& ResourceId,
		const FName CapabilityName)
	{
		// Typed Replace override입니다.
		FCFResourceBindingOverride Override;
		Override.RoleId = RoleId;
		Override.Mode = ECFResourceOverrideMode::Replace;
		Override.ResourceId = ResourceId;
		Override.CapabilityName = CapabilityName;
		Request.Overrides.Add(MoveTemp(Override));
	}

	// Vehicle AssetReader가 읽은 requested socket fact가 실제 persisted chassis에서 존재하는지 확인합니다.
	bool ValidateVehicleSocketFact(
		const FCFVehicleAssetSnapshot& AssetSnapshot,
		const FName SocketName,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		if (SocketName.IsNone())
		{
			return true;
		}

		// Existing Vehicle AssetReader가 반환한 exact socket fact입니다.
		const FCFVehicleSocketSnapshot* SocketSnapshot =
			AssetSnapshot.FindChassisSocket(SocketName);
		if (SocketSnapshot == nullptr || !SocketSnapshot->bFound)
		{
			AddIssue(
				OutIssues,
				TEXT("VehicleBackendSocketMissing"),
				FString::Printf(TEXT("Vehicle/Socket/%s"), *SocketName.ToString()),
				TEXT("VehicleAssetReader가 typed Recipe가 요청한 chassis socket을 persisted StaticMesh에서 찾지 못했습니다."));
			return false;
		}

		return true;
	}
}

// Static/Skeletal Mesh picker와 fixed shared role schema를 exact1로 초기화합니다.
bool FCFContentPilotContext::Initialize(FString& OutError)
{
	OutError.Reset();

	if (PickerRegistry.Num() != 0
		|| ResourceCatalog.Num() != 0
		|| RoleRegistry.Num() != 0
		|| ProfileRegistry.Num() != 0)
	{
		OutError = TEXT("Content Pilot context는 fresh empty state에서만 Initialize할 수 있습니다.");
		return false;
	}

	// StaticMesh Asset Picker registration입니다.
	FCFResourcePickerDescriptor StaticMeshPicker;
	StaticMeshPicker.PickerTypeId = TEXT("picker.static_mesh");
	StaticMeshPicker.AllowedBaseClassPath =
		UStaticMesh::StaticClass()->GetClassPathName();
	StaticMeshPicker.bAllowClear = true;
	if (!PickerRegistry.RegisterPicker(StaticMeshPicker, OutError))
	{
		return false;
	}

	// SkeletalMesh evidence와 future consumer에서 공유할 Asset Picker registration입니다.
	FCFResourcePickerDescriptor SkeletalMeshPicker;
	SkeletalMeshPicker.PickerTypeId = TEXT("picker.skeletal_mesh");
	SkeletalMeshPicker.AllowedBaseClassPath =
		USkeletalMesh::StaticClass()->GetClassPathName();
	SkeletalMeshPicker.bAllowClear = true;
	if (!PickerRegistry.RegisterPicker(SkeletalMeshPicker, OutError))
	{
		return false;
	}

	// Explicit Weapon resource/socket roles입니다.
	struct FRoleSeed
	{
		// Stable role identity입니다.
		const TCHAR* RoleId;

		// Explicit binding 실패 시 pilot을 차단할 requirement입니다.
		ECFResourceRequirement Requirement;

		// Role이 요구하는 mesh capability입니다.
		ECFResourceCapabilityKind CapabilityKind;
	};

	// Weapon/Vehicle fixed semantic role seed입니다.
	const FRoleSeed RoleSeeds[] = {
		{ TEXT("weapon.visual.base"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("weapon.visual.yaw"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("weapon.visual.pitch"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("weapon.socket.yaw_pivot"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket },
		{ TEXT("weapon.socket.pitch_pivot"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket },
		{ TEXT("weapon.socket.muzzle_primary"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket },
		{ TEXT("vehicle.visual.chassis"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("vehicle.visual.wheel_fl"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("vehicle.visual.wheel_fr"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("vehicle.visual.wheel_rl"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("vehicle.visual.wheel_rr"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::None },
		{ TEXT("vehicle.socket.wheel_fl"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket },
		{ TEXT("vehicle.socket.wheel_fr"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket },
		{ TEXT("vehicle.socket.wheel_rl"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket },
		{ TEXT("vehicle.socket.wheel_rr"), ECFResourceRequirement::Required, ECFResourceCapabilityKind::Socket }
	};

	for (const FRoleSeed& Seed : RoleSeeds)
	{
		if (!CFContentPilotPrivate::EnsureRole(
			RoleRegistry,
			Seed.RoleId,
			Seed.Requirement,
			Seed.CapabilityKind,
			OutError))
		{
			return false;
		}
	}

	return true;
}

// Product mutation 없이 WeaponData contract와 Turret resource/socket binding을 resolve합니다.
bool FCFWeaponResourcePilot::Build(
	const UCFWeaponData& WeaponData,
	const UCFTurretMountData& TurretMountData,
	FCFContentPilotContext& SharedContext,
	FCFWeaponPilotResult& OutResult)
{
	OutResult = FCFWeaponPilotResult();
	OutResult.WeaponId = WeaponData.WeaponId;
	OutResult.TurretMountId = TurretMountData.TurretMountId;

	// Existing WeaponData typed static-contract validation errors입니다.
	TArray<FText> WeaponValidationErrors;
	OutResult.bWeaponContractValidated =
		WeaponData.ValidateWeaponDataContract(WeaponValidationErrors);
	if (!OutResult.bWeaponContractValidated)
	{
		for (const FText& ValidationError : WeaponValidationErrors)
		{
			CFContentPilotPrivate::AddIssue(
				OutResult.Issues,
				TEXT("WeaponBackendContractInvalid"),
				TEXT("Weapon/WeaponData"),
				ValidationError.ToString());
		}
		return false;
	}

	if (TurretMountData.TurretMountId.IsNone())
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("MissingTurretMountId"),
			TEXT("Weapon/TurretMountData"),
			TEXT("TurretMountData의 stable domain identity가 None입니다."));
		return false;
	}

	// Stable resource/profile namespace에 사용할 typed TurretMountId token입니다.
	const FString TurretToken =
		CFContentPilotPrivate::MakeStableToken(TurretMountData.TurretMountId);
	if (TurretToken.IsEmpty())
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("InvalidTurretMountId"),
			TEXT("Weapon/TurretMountData"),
			TEXT("TurretMountId를 stable ResourceId token으로 정규화할 수 없습니다."));
		return false;
	}

	// Weapon pilot profile입니다.
	FCFResourceProfile Profile;
	Profile.ProfileId = FString::Printf(
		TEXT("profile.weapon.%s"),
		*TurretToken);

	// Shared resolver가 이번 Weapon에서 실제 resolve할 roles입니다.
	TArray<FString> RequestedRoleIds;

	// Base/Yaw/Pitch resource identities입니다.
	const FString BaseResourceId =
		FString::Printf(TEXT("weapon.%s.base"), *TurretToken);
	const FString YawResourceId =
		FString::Printf(TEXT("weapon.%s.yaw"), *TurretToken);
	const FString PitchResourceId =
		FString::Printf(TEXT("weapon.%s.pitch"), *TurretToken);

	// Registry/registration 실패 메시지입니다.
	FString Error;

	if (TurretMountData.TurretBaseMesh != nullptr)
	{
		if (!CFContentPilotPrivate::RegisterStaticMeshResource(
			BaseResourceId,
			TurretMountData.TurretBaseMesh.Get(),
			SharedContext,
			Error))
		{
			CFContentPilotPrivate::AddIssue(
				OutResult.Issues,
				TEXT("WeaponResourceRegistrationFailed"),
				TEXT("Weapon/Resource/Base"),
				Error);
			return false;
		}

		CFContentPilotPrivate::AddProfileBinding(
			Profile,
			RequestedRoleIds,
			TEXT("weapon.visual.base"),
			BaseResourceId);

		if (!TurretMountData.YawPivotSocketName.IsNone())
		{
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				TEXT("weapon.socket.yaw_pivot"),
				BaseResourceId,
				TurretMountData.YawPivotSocketName);
		}
	}

	if (TurretMountData.TurretYawMesh != nullptr)
	{
		if (!CFContentPilotPrivate::RegisterStaticMeshResource(
			YawResourceId,
			TurretMountData.TurretYawMesh.Get(),
			SharedContext,
			Error))
		{
			CFContentPilotPrivate::AddIssue(
				OutResult.Issues,
				TEXT("WeaponResourceRegistrationFailed"),
				TEXT("Weapon/Resource/Yaw"),
				Error);
			return false;
		}

		CFContentPilotPrivate::AddProfileBinding(
			Profile,
			RequestedRoleIds,
			TEXT("weapon.visual.yaw"),
			YawResourceId);

		if (!TurretMountData.PitchPivotSocketName.IsNone())
		{
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				TEXT("weapon.socket.pitch_pivot"),
				YawResourceId,
				TurretMountData.PitchPivotSocketName);
		}
	}

	if (TurretMountData.TurretPitchMesh != nullptr)
	{
		if (!CFContentPilotPrivate::RegisterStaticMeshResource(
			PitchResourceId,
			TurretMountData.TurretPitchMesh.Get(),
			SharedContext,
			Error))
		{
			CFContentPilotPrivate::AddIssue(
				OutResult.Issues,
				TEXT("WeaponResourceRegistrationFailed"),
				TEXT("Weapon/Resource/Pitch"),
				Error);
			return false;
		}

		CFContentPilotPrivate::AddProfileBinding(
			Profile,
			RequestedRoleIds,
			TEXT("weapon.visual.pitch"),
			PitchResourceId);
	}

	// Existing TurretMountData가 실제 사용하는 primary muzzle socket입니다.
	FName PrimaryMuzzleSocketName = TurretMountData.MuzzleSocketName;
	if (!TurretMountData.MuzzleSocketNames.IsEmpty()
		&& !TurretMountData.MuzzleSocketNames[0].IsNone())
	{
		PrimaryMuzzleSocketName = TurretMountData.MuzzleSocketNames[0];
	}

	if (TurretMountData.TurretPitchMesh != nullptr
		&& !PrimaryMuzzleSocketName.IsNone())
	{
		CFContentPilotPrivate::AddProfileBinding(
			Profile,
			RequestedRoleIds,
			TEXT("weapon.socket.muzzle_primary"),
			PitchResourceId,
			PrimaryMuzzleSocketName);
	}

	if (Profile.Bindings.IsEmpty())
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("WeaponPilotHasNoResources"),
			TEXT("Weapon/TurretMountData"),
			TEXT("Dual Consumer Pilot에서 검증할 Turret mesh resource가 없습니다."));
		return false;
	}

	if (!SharedContext.ProfileRegistry.RegisterProfile(
		Profile,
		SharedContext.RoleRegistry,
		Error))
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("WeaponProfileRegistrationFailed"),
			TEXT("Weapon/Profile"),
			Error);
		return false;
	}

	// Profile + typed per-content override를 shared resolver에 전달할 request입니다.
	FCFResourceBindingRequest Request;
	Request.RequestedRoleIds = RequestedRoleIds;
	Request.ProfileId = Profile.ProfileId;

	if (TurretMountData.TurretBaseMesh != nullptr
		&& !TurretMountData.YawPivotSocketName.IsNone())
	{
		CFContentPilotPrivate::AddOverride(
			Request,
			TEXT("weapon.socket.yaw_pivot"),
			BaseResourceId,
			TurretMountData.YawPivotSocketName);
	}

	if (TurretMountData.TurretYawMesh != nullptr
		&& !TurretMountData.PitchPivotSocketName.IsNone())
	{
		CFContentPilotPrivate::AddOverride(
			Request,
			TEXT("weapon.socket.pitch_pivot"),
			YawResourceId,
			TurretMountData.PitchPivotSocketName);
	}

	if (TurretMountData.TurretPitchMesh != nullptr
		&& !PrimaryMuzzleSocketName.IsNone())
	{
		CFContentPilotPrivate::AddOverride(
			Request,
			TEXT("weapon.socket.muzzle_primary"),
			PitchResourceId,
			PrimaryMuzzleSocketName);
	}

	// Shared Resource Core의 final resolve 성공 여부입니다.
	const bool bResolved = FCFResourceBindingResolver::Resolve(
		Request,
		SharedContext.RoleRegistry,
		SharedContext.ProfileRegistry,
		SharedContext.ResourceCatalog,
		SharedContext.PickerRegistry,
		OutResult.ResourceBindings);
	CFContentPilotPrivate::AppendIssues(
		OutResult.ResourceBindings.Issues,
		OutResult.Issues);
	return bResolved
		&& !CFContentPilotPrivate::HasBlockingIssues(OutResult.Issues);
}

// Product mutation 없이 Recipe Snapshot/Asset Snapshot과 resource/socket binding을 resolve합니다.
bool FCFVehicleResourcePilot::Build(
	const UCFVehicleRecipeData& Recipe,
	FCFContentPilotContext& SharedContext,
	FCFVehiclePilotResult& OutResult)
{
	OutResult = FCFVehiclePilotResult();
	OutResult.RecipeId = Recipe.RecipeId;

	if (!Recipe.RecipeId.IsValid())
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("MissingVehicleRecipeId"),
			TEXT("Vehicle/Recipe"),
			TEXT("VehicleRecipeData의 persistent RecipeId가 invalid입니다."));
		return false;
	}

	// Existing Vehicle SnapshotBuilder가 생성하는 typed immutable Recipe snapshot입니다.
	FCFVehicleRecipeSnapshot RecipeSnapshot;
	// Existing Vehicle SnapshotBuilder의 diagnostic입니다.
	FString Error;
	OutResult.bRecipeSnapshotBuilt =
		FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(
			Recipe,
			RecipeSnapshot,
			Error);
	if (!OutResult.bRecipeSnapshotBuilt)
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("VehicleRecipeSnapshotFailed"),
			TEXT("Vehicle/Recipe"),
			Error);
		return false;
	}
	OutResult.RecipeFingerprint = RecipeSnapshot.RecipeFingerprint;

	// Existing Vehicle AssetReader가 생성하는 read-only StaticMesh/socket snapshot입니다.
	FCFVehicleAssetSnapshot AssetSnapshot;
	OutResult.bAssetSnapshotBuilt =
		FCFVehicleAssetReader::BuildAssetSnapshot(
			RecipeSnapshot,
			AssetSnapshot,
			Error);
	if (!OutResult.bAssetSnapshotBuilt)
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("VehicleAssetSnapshotFailed"),
			TEXT("Vehicle/AssetIntent"),
			Error);
		return false;
	}
	OutResult.ChassisLayoutFingerprint =
		AssetSnapshot.ChassisLayoutFingerprint;

	// Explicit typed socket intent가 Existing AssetReader에서도 실제 found인지 확인합니다.
	CFContentPilotPrivate::ValidateVehicleSocketFact(
		AssetSnapshot,
		RecipeSnapshot.AssetIntent.BodyWheelSocketFL,
		OutResult.Issues);
	CFContentPilotPrivate::ValidateVehicleSocketFact(
		AssetSnapshot,
		RecipeSnapshot.AssetIntent.BodyWheelSocketFR,
		OutResult.Issues);
	CFContentPilotPrivate::ValidateVehicleSocketFact(
		AssetSnapshot,
		RecipeSnapshot.AssetIntent.BodyWheelSocketRL,
		OutResult.Issues);
	CFContentPilotPrivate::ValidateVehicleSocketFact(
		AssetSnapshot,
		RecipeSnapshot.AssetIntent.BodyWheelSocketRR,
		OutResult.Issues);
	for (const FCFHardpointIntent& Hardpoint : RecipeSnapshot.HardpointIntents)
	{
		CFContentPilotPrivate::ValidateVehicleSocketFact(
			AssetSnapshot,
			Hardpoint.SocketName,
			OutResult.Issues);
	}
	if (CFContentPilotPrivate::HasBlockingIssues(OutResult.Issues))
	{
		return false;
	}

	// Recipe GUID 기반 stable Resource/Profile namespace입니다.
	const FString RecipeToken =
		CFContentPilotPrivate::MakeRecipeToken(Recipe.RecipeId);

	// Chassis/wheel stable ResourceIds입니다.
	const FString ChassisResourceId =
		FString::Printf(TEXT("vehicle.%s.chassis"), *RecipeToken);
	const FString WheelFLResourceId =
		FString::Printf(TEXT("vehicle.%s.wheel_fl"), *RecipeToken);
	const FString WheelFRResourceId =
		FString::Printf(TEXT("vehicle.%s.wheel_fr"), *RecipeToken);
	const FString WheelRLResourceId =
		FString::Printf(TEXT("vehicle.%s.wheel_rl"), *RecipeToken);
	const FString WheelRRResourceId =
		FString::Printf(TEXT("vehicle.%s.wheel_rr"), *RecipeToken);

	// Vehicle provider-local profile입니다.
	FCFResourceProfile Profile;
	Profile.ProfileId =
		FString::Printf(TEXT("profile.vehicle.%s"), *RecipeToken);

	// Shared resolver가 이번 Vehicle에서 실제 resolve할 roles입니다.
	TArray<FString> RequestedRoleIds;

	if (RecipeSnapshot.AssetIntent.ChassisMesh.ToSoftObjectPath().IsValid())
	{
		if (!CFContentPilotPrivate::RegisterStaticMeshResource(
			ChassisResourceId,
			RecipeSnapshot.AssetIntent.ChassisMesh.ToSoftObjectPath(),
			SharedContext,
			Error))
		{
			CFContentPilotPrivate::AddIssue(
				OutResult.Issues,
				TEXT("VehicleResourceRegistrationFailed"),
				TEXT("Vehicle/Resource/Chassis"),
				Error);
			return false;
		}

		CFContentPilotPrivate::AddProfileBinding(
			Profile,
			RequestedRoleIds,
			TEXT("vehicle.visual.chassis"),
			ChassisResourceId);
	}

	// Typed Wheel resource registration을 반복 적용할 helper lambda입니다.
	const auto RegisterWheel =
		[&SharedContext, &Profile, &RequestedRoleIds, &OutResult, &Error](
			const FString& ResourceId,
			const FSoftObjectPath& ObjectPath,
			const FString& RoleId,
			const FString& IssuePath) -> bool
		{
			if (!ObjectPath.IsValid())
			{
				return true;
			}
			if (!CFContentPilotPrivate::RegisterStaticMeshResource(
				ResourceId,
				ObjectPath,
				SharedContext,
				Error))
			{
				CFContentPilotPrivate::AddIssue(
					OutResult.Issues,
					TEXT("VehicleResourceRegistrationFailed"),
					IssuePath,
					Error);
				return false;
			}
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				RoleId,
				ResourceId);
			return true;
		};

	if (!RegisterWheel(
		WheelFLResourceId,
		RecipeSnapshot.AssetIntent.WheelMeshFL.ToSoftObjectPath(),
		TEXT("vehicle.visual.wheel_fl"),
		TEXT("Vehicle/Resource/WheelFL"))
		|| !RegisterWheel(
			WheelFRResourceId,
			RecipeSnapshot.AssetIntent.WheelMeshFR.ToSoftObjectPath(),
			TEXT("vehicle.visual.wheel_fr"),
			TEXT("Vehicle/Resource/WheelFR"))
		|| !RegisterWheel(
			WheelRLResourceId,
			RecipeSnapshot.AssetIntent.WheelMeshRL.ToSoftObjectPath(),
			TEXT("vehicle.visual.wheel_rl"),
			TEXT("Vehicle/Resource/WheelRL"))
		|| !RegisterWheel(
			WheelRRResourceId,
			RecipeSnapshot.AssetIntent.WheelMeshRR.ToSoftObjectPath(),
			TEXT("vehicle.visual.wheel_rr"),
			TEXT("Vehicle/Resource/WheelRR")))
	{
		return false;
	}

	if (RecipeSnapshot.AssetIntent.ChassisMesh.ToSoftObjectPath().IsValid())
	{
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketFL.IsNone())
		{
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				TEXT("vehicle.socket.wheel_fl"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketFL);
		}
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketFR.IsNone())
		{
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				TEXT("vehicle.socket.wheel_fr"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketFR);
		}
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketRL.IsNone())
		{
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				TEXT("vehicle.socket.wheel_rl"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketRL);
		}
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketRR.IsNone())
		{
			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				TEXT("vehicle.socket.wheel_rr"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketRR);
		}

		for (const FCFHardpointIntent& Hardpoint : RecipeSnapshot.HardpointIntents)
		{
			if (Hardpoint.LocationSlotId.IsNone()
				|| Hardpoint.SocketName.IsNone())
			{
				continue;
			}

			// Stable hardpoint semantic suffix입니다.
			const FString HardpointToken =
				CFContentPilotPrivate::MakeStableToken(
					Hardpoint.LocationSlotId);
			if (HardpointToken.IsEmpty())
			{
				CFContentPilotPrivate::AddIssue(
					OutResult.Issues,
					TEXT("InvalidHardpointRoleId"),
					TEXT("Vehicle/Hardpoint"),
					TEXT("Hardpoint LocationSlotId를 stable semantic role token으로 변환할 수 없습니다."));
				return false;
			}

			// Provider-local dynamic hardpoint semantic role입니다.
			const FString HardpointRoleId =
				FString::Printf(
					TEXT("vehicle.socket.hardpoint.%s"),
					*HardpointToken);
			if (!CFContentPilotPrivate::EnsureRole(
				SharedContext.RoleRegistry,
				HardpointRoleId,
				ECFResourceRequirement::Required,
				ECFResourceCapabilityKind::Socket,
				Error))
			{
				CFContentPilotPrivate::AddIssue(
					OutResult.Issues,
					TEXT("VehicleRoleRegistrationFailed"),
					TEXT("Vehicle/Hardpoint"),
					Error);
				return false;
			}

			CFContentPilotPrivate::AddProfileBinding(
				Profile,
				RequestedRoleIds,
				HardpointRoleId,
				ChassisResourceId,
				Hardpoint.SocketName);
		}
	}

	if (Profile.Bindings.IsEmpty())
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("VehiclePilotHasNoResources"),
			TEXT("Vehicle/AssetIntent"),
			TEXT("Dual Consumer Pilot에서 검증할 Vehicle resource가 없습니다."));
		return false;
	}

	if (!SharedContext.ProfileRegistry.RegisterProfile(
		Profile,
		SharedContext.RoleRegistry,
		Error))
	{
		CFContentPilotPrivate::AddIssue(
			OutResult.Issues,
			TEXT("VehicleProfileRegistrationFailed"),
			TEXT("Vehicle/Profile"),
			Error);
		return false;
	}

	// Profile + typed per-Recipe override를 shared resolver에 전달할 request입니다.
	FCFResourceBindingRequest Request;
	Request.RequestedRoleIds = RequestedRoleIds;
	Request.ProfileId = Profile.ProfileId;

	if (RecipeSnapshot.AssetIntent.ChassisMesh.ToSoftObjectPath().IsValid())
	{
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketFL.IsNone())
		{
			CFContentPilotPrivate::AddOverride(
				Request,
				TEXT("vehicle.socket.wheel_fl"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketFL);
		}
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketFR.IsNone())
		{
			CFContentPilotPrivate::AddOverride(
				Request,
				TEXT("vehicle.socket.wheel_fr"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketFR);
		}
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketRL.IsNone())
		{
			CFContentPilotPrivate::AddOverride(
				Request,
				TEXT("vehicle.socket.wheel_rl"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketRL);
		}
		if (!RecipeSnapshot.AssetIntent.BodyWheelSocketRR.IsNone())
		{
			CFContentPilotPrivate::AddOverride(
				Request,
				TEXT("vehicle.socket.wheel_rr"),
				ChassisResourceId,
				RecipeSnapshot.AssetIntent.BodyWheelSocketRR);
		}
	}

	// Shared Resource Core의 final resolve 성공 여부입니다.
	const bool bResolved = FCFResourceBindingResolver::Resolve(
		Request,
		SharedContext.RoleRegistry,
		SharedContext.ProfileRegistry,
		SharedContext.ResourceCatalog,
		SharedContext.PickerRegistry,
		OutResult.ResourceBindings);
	CFContentPilotPrivate::AppendIssues(
		OutResult.ResourceBindings.Issues,
		OutResult.Issues);
	return bResolved
		&& !CFContentPilotPrivate::HasBlockingIssues(OutResult.Issues);
}
