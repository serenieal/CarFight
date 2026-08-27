// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleFieldRegistry.cpp
// Version: v1.5.0
// Date: 2026-08-26
// Description: Current 127 VehicleData leaf pattern Registry, Builder atomic Transmission ratio-set/wheel fallback dependency metadata와 Reflection coverage 구현입니다.
// Changelog:
// - v1.5.0: VB-P0-05 설계 검수 교정으로 FCFVehicleTransmissionRatios를 atomic leaf로 복원해 Forward/Reverse ratio provenance가 분리되지 않게 하고 coverage를 128→127로 정정.
// - v1.4.0: CF-FQ-040 VB-P0-05 ChassisWidth/Transmission 10개 leaf를 추가하고 wheel geometry를 BaseProfileMeasurementPolicy로 전환해 VehicleBase fallback + AssetDerived 우선 계약을 연결.
// - v1.3.0: UI-P0-06 explicit RedlineStartRPM을 PerformanceProfileDirect descriptor로 추가해 Reflection coverage를 118 leaf로 확장.
// - v1.2.0: DAUTH-P0-08E candidate enforcement를 위해 Section 22.17에 이미 동결된 Project Default/Base/Asset 보조 Source mask 누락을 교정.
// - v1.1.0: DAUTH-P0-08C Section 22.17 Main Dependency를 RequiredDependencies로 확장.
// - v1.0.0: DAUTH-P0-08B 117 descriptor, Stable-ID array pattern, bidirectional coverage를 구현.
// Migration:
// - UCFVehicleData field 추가/삭제/rename 시 coverage Automation이 조용히 통과하지 않습니다.

#include "DataAuthoring/CFVehicleFieldRegistry.h"

#include "CFVehicleData.h"
#include "UObject/UnrealType.h"

namespace CFVehicleFieldRegistryPrivate
{
	/** Descriptor seed를 compact하게 기술하는 내부 상수 데이터입니다. */
	struct FDescriptorSeed
	{
		// Canonical scalar 또는 Stable-ID wildcard path입니다.
		const TCHAR* CanonicalPattern;

		// Primary Profile Domain입니다.
		ECFVehicleProfileDomain Domain;

		// P0 Resolver rule입니다.
		ECFVehicleResolveRule Rule;

		// User Adoption Group입니다.
		ECFVehicleAdoptGroup Group;

		// Advanced Override 허용 여부입니다.
		bool bOverride;

		// Stable identity field 여부입니다.
		bool bIdentity;

		// Hidden legacy serialized field 여부입니다.
		bool bLegacy;
	};

	// Source type 하나를 descriptor bit mask로 변환합니다.
	uint64 SourceBit(const ECFVehicleSourceType SourceType)
	{
		return 1ull << static_cast<uint8>(SourceType);
	}

	// 모든 normal Resolver source와 Legacy Pin/Advanced를 포함하는 첫 Foundation 허용 mask를 만듭니다.
	uint64 BuildAllowedSourceMask(const FDescriptorSeed& Seed)
	{
				// Frozen 일반 precedence의 최저 baseline인 Project Default와 Existing Definition 보존용 Legacy Pin을 normal field에 공통 허용합니다.
		uint64 Mask = SourceBit(ECFVehicleSourceType::ProjectCompatibilityDefault)
			| SourceBit(ECFVehicleSourceType::LegacyImportedPinnedBaseline);

		switch (Seed.Rule)
		{
				case ECFVehicleResolveRule::ProjectCompatibilityDefault:
		case ECFVehicleResolveRule::CompatibilityDefaultOrAdvanced:
			Mask |= SourceBit(ECFVehicleSourceType::ProjectCompatibilityDefault);
			break;
		case ECFVehicleResolveRule::ProjectDefaultThenRecipeSemantic:
			Mask |= SourceBit(ECFVehicleSourceType::ProjectCompatibilityDefault);
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			break;
				case ECFVehicleResolveRule::RecipeAssetIntent:
		case ECFVehicleResolveRule::RecipeSemantic:
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			break;
		case ECFVehicleResolveRule::RecipeBinding:
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			if (FString(Seed.CanonicalPattern).StartsWith(TEXT("VehicleLayoutConfig.BodyWheelSocket")))
			{
				Mask |= SourceBit(ECFVehicleSourceType::ProjectCompatibilityDefault);
			}
			break;
		case ECFVehicleResolveRule::RecipeWheelVisualPolicy:
			Mask |= SourceBit(ECFVehicleSourceType::VehicleBaseProfile);
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			break;
		case ECFVehicleResolveRule::DerivedGate:
			Mask |= SourceBit(ECFVehicleSourceType::RuleDerived);
			break;
		case ECFVehicleResolveRule::AssetSocketDerived:
		case ECFVehicleResolveRule::AssetMeasurementProposal:
			Mask |= SourceBit(ECFVehicleSourceType::AssetDerived);
			break;
		case ECFVehicleResolveRule::LegacySerializedPassthrough:
			Mask = SourceBit(ECFVehicleSourceType::LegacySerializedPassthrough);
			break;
		case ECFVehicleResolveRule::BaseProfileThenRecipeExplicit:
		case ECFVehicleResolveRule::BaseProfileThenRecipeAsset:
			Mask |= SourceBit(ECFVehicleSourceType::VehicleBaseProfile);
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			break;
		case ECFVehicleResolveRule::AccelerationFeelDerived:
		case ECFVehicleResolveRule::PerformanceProfileDirect:
			Mask |= SourceBit(ECFVehicleSourceType::PerformanceProfile);
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			Mask |= SourceBit(ECFVehicleSourceType::RuleDerived);
			break;
		case ECFVehicleResolveRule::SteeringFeelDerived:
		case ECFVehicleResolveRule::GripFeelDerived:
		case ECFVehicleResolveRule::SuspensionFeelDerived:
		case ECFVehicleResolveRule::HandlingProfileDirect:
			Mask |= SourceBit(ECFVehicleSourceType::HandlingProfile);
			Mask |= SourceBit(ECFVehicleSourceType::RecipeExplicitSemanticInput);
			Mask |= SourceBit(ECFVehicleSourceType::RuleDerived);
			break;
		case ECFVehicleResolveRule::DrivetrainProfile:
			Mask |= SourceBit(ECFVehicleSourceType::DrivetrainProfile);
			break;
				case ECFVehicleResolveRule::VehicleBaseProfile:
			Mask |= SourceBit(ECFVehicleSourceType::VehicleBaseProfile);
			break;
		case ECFVehicleResolveRule::BaseProfileMeasurementPolicy:
			Mask |= SourceBit(ECFVehicleSourceType::VehicleBaseProfile);
			Mask |= SourceBit(ECFVehicleSourceType::AssetDerived);
			break;
		case ECFVehicleResolveRule::DriveStateProfile:
			Mask |= SourceBit(ECFVehicleSourceType::DriveStateProfile);
			break;
		default:
			break;
		}

		if (Seed.bOverride)
		{
			Mask |= SourceBit(ECFVehicleSourceType::AdvancedLeafOverride);
		}

				return Mask;
	}

	// Dependency key를 descriptor 목록에 중복 없이 추가합니다.
	void AddDependency(TArray<FName>& Dependencies, const TCHAR* DependencyKey)
	{
		// Frozen dependency contract에서 사용할 stable FName key입니다.
		const FName DependencyName(DependencyKey);
		if (!Dependencies.Contains(DependencyName))
		{
			Dependencies.Add(DependencyName);
		}
	}

	// Frozen Section 22.17 Main Dependency를 Resolver 이전 단계에서 사용할 stable key 목록으로 확장합니다.
	TArray<FName> BuildRequiredDependencies(const FDescriptorSeed& Seed)
	{
		// 최종 descriptor에 기록할 deterministic dependency key 목록입니다.
		TArray<FName> Dependencies;
		// Path-specific dependency 분기에 사용할 canonical Registry pattern입니다.
		const FString Pattern(Seed.CanonicalPattern);

		switch (Seed.Rule)
		{
		case ECFVehicleResolveRule::ProjectCompatibilityDefault:
			AddDependency(Dependencies, TEXT("Project.CompatibilityDefaults"));
			if (Pattern == TEXT("VehicleMovementConfig.MovementProfileName"))
			{
				AddDependency(Dependencies, TEXT("Recipe.ImportState.LegacyPins"));
			}
			break;

		case ECFVehicleResolveRule::RecipeAssetIntent:
			AddDependency(Dependencies, TEXT("Recipe.AssetIntent"));
			break;

		case ECFVehicleResolveRule::RecipeSemantic:
			if (Pattern.StartsWith(TEXT("HardpointSlots")))
			{
				AddDependency(Dependencies, TEXT("Recipe.HardpointIntents"));
			}
			else
			{
				AddDependency(Dependencies, TEXT("Recipe.MountIntents"));
				if (Pattern.Contains(TEXT(".LocationSlotRef")))
				{
					AddDependency(Dependencies, TEXT("Recipe.HardpointIntents"));
				}
			}
			break;

		case ECFVehicleResolveRule::RecipeBinding:
			if (Pattern.StartsWith(TEXT("VehicleLayoutConfig")))
			{
				AddDependency(Dependencies, TEXT("Project.CompatibilityDefaults"));
				AddDependency(Dependencies, TEXT("Recipe.AssetIntent"));
			}
			else
			{
				AddDependency(Dependencies, TEXT("Recipe.HardpointIntents"));
			}
			break;

		case ECFVehicleResolveRule::DerivedGate:
			if (Pattern == TEXT("VehicleLayoutConfig.bUseLayoutOverrides"))
			{
				AddDependency(Dependencies, TEXT("Resolved.LayoutState"));
			}
			else if (Pattern == TEXT("VehicleMovementConfig.bUseMovementOverrides"))
			{
				AddDependency(Dependencies, TEXT("Resolved.WheelDetailOrThrottleState"));
			}
			else if (Pattern == TEXT("VehicleMovementConfig.bEnableCenterOfMassOverride"))
			{
				AddDependency(Dependencies, TEXT("Resolved.CenterOfMassSource"));
			}
			else if (Pattern == TEXT("WheelVisualConfig.bUseWheelVisualOverrides"))
			{
				AddDependency(Dependencies, TEXT("Resolved.WheelVisualState"));
			}
			else if (Pattern == TEXT("DriveStateConfig.bUseDriveStateOverrides"))
			{
				AddDependency(Dependencies, TEXT("Recipe.DriveStateMode"));
			}
			break;

		case ECFVehicleResolveRule::AssetSocketDerived:
			AddDependency(Dependencies, TEXT("Asset.ChassisSockets"));
			if (Pattern.StartsWith(TEXT("VehicleLayoutConfig")))
			{
				AddDependency(Dependencies, TEXT("Recipe.AssetIntent"));
			}
			else
			{
				AddDependency(Dependencies, TEXT("Recipe.HardpointIntents"));
			}
			break;

		case ECFVehicleResolveRule::LegacySerializedPassthrough:
			AddDependency(Dependencies, TEXT("Recipe.ImportState.LegacySerialized"));
			break;

		case ECFVehicleResolveRule::BaseProfileThenRecipeExplicit:
			AddDependency(Dependencies, TEXT("Profile.VehicleBase"));
			if (Pattern == TEXT("VehicleDurabilityConfig.MaxHealth"))
			{
				AddDependency(Dependencies, TEXT("Recipe.DurabilityIntent"));
			}
			else
			{
				AddDependency(Dependencies, TEXT("Recipe.MassIntent"));
			}
			break;

		case ECFVehicleResolveRule::AccelerationFeelDerived:
			AddDependency(Dependencies, TEXT("Profile.Performance"));
			AddDependency(Dependencies, TEXT("Recipe.DrivingFeelIntent"));
			AddDependency(Dependencies, TEXT("Resolver.OptionalBaseMassContext"));
			break;

		case ECFVehicleResolveRule::SteeringFeelDerived:
		case ECFVehicleResolveRule::GripFeelDerived:
			AddDependency(Dependencies, TEXT("Profile.Handling"));
			AddDependency(Dependencies, TEXT("Recipe.DrivingFeelIntent"));
			break;

		case ECFVehicleResolveRule::HandlingProfileDirect:
			AddDependency(Dependencies, TEXT("Profile.Handling"));
			break;

		case ECFVehicleResolveRule::AssetMeasurementProposal:
			AddDependency(Dependencies, TEXT("Asset.WheelBounds"));
			break;

		case ECFVehicleResolveRule::SuspensionFeelDerived:
			AddDependency(Dependencies, TEXT("Profile.Handling"));
			AddDependency(Dependencies, TEXT("Recipe.DrivingFeelIntent"));
			AddDependency(Dependencies, TEXT("Resolver.OptionalBaseMassContext"));
			break;

		case ECFVehicleResolveRule::DrivetrainProfile:
			AddDependency(Dependencies, TEXT("Profile.Drivetrain"));
			break;

		case ECFVehicleResolveRule::VehicleBaseProfile:
			AddDependency(Dependencies, TEXT("Profile.VehicleBase"));
			break;

		case ECFVehicleResolveRule::PerformanceProfileDirect:
			AddDependency(Dependencies, TEXT("Profile.Performance"));
			break;

		case ECFVehicleResolveRule::CompatibilityDefaultOrAdvanced:
			AddDependency(Dependencies, TEXT("Project.CompatibilityDefaults"));
			AddDependency(Dependencies, TEXT("Recipe.AdvancedOverrides"));
			AddDependency(Dependencies, TEXT("Recipe.ImportState.LegacyPins"));
			break;

		case ECFVehicleResolveRule::RecipeWheelVisualPolicy:
			AddDependency(Dependencies, TEXT("Profile.VehicleBase"));
			AddDependency(Dependencies, TEXT("Recipe.WheelVisualIntent"));
			break;

		case ECFVehicleResolveRule::BaseProfileMeasurementPolicy:
			AddDependency(Dependencies, TEXT("Profile.VehicleBase"));
			AddDependency(Dependencies, TEXT("Asset.WheelBounds"));
			break;

		case ECFVehicleResolveRule::DriveStateProfile:
			AddDependency(Dependencies, TEXT("Recipe.DriveStateMode"));
			AddDependency(Dependencies, TEXT("Profile.DriveState"));
			break;

		case ECFVehicleResolveRule::BaseProfileThenRecipeAsset:
			AddDependency(Dependencies, TEXT("Profile.VehicleBase"));
			AddDependency(Dependencies, TEXT("Recipe.DefaultDataIntent"));
			break;

		case ECFVehicleResolveRule::ProjectDefaultThenRecipeSemantic:
			AddDependency(Dependencies, TEXT("Project.CompatibilityDefaults"));
			AddDependency(Dependencies, TEXT("Recipe.DefaultDataIntent"));
			break;

		default:
			break;
		}

		Dependencies.Sort(FNameLexicalLess());
		return Dependencies;
	}

	// Canonical registry pattern을 structured FCFVehicleFieldPath로 변환합니다.
	FCFVehicleFieldPath ParsePattern(const FString& CanonicalPattern)
	{
		// 완성할 structured stable path입니다.
		FCFVehicleFieldPath Result;

		// Stable collection selector 시작 위치입니다.
		int32 OpenBracketIndex = INDEX_NONE;
		if (CanonicalPattern.FindChar(TEXT('['), OpenBracketIndex))
		{
			// Stable selector 종료 위치입니다.
			int32 CloseBracketIndex = INDEX_NONE;
			CanonicalPattern.FindChar(TEXT(']'), CloseBracketIndex);

			Result.CollectionPropertyName = FName(*CanonicalPattern.Left(OpenBracketIndex));

			// 대괄호 안의 key=value 표현입니다.
			const FString SelectorExpression = CanonicalPattern.Mid(OpenBracketIndex + 1, CloseBracketIndex - OpenBracketIndex - 1);
			// Selector key/value 구분 위치입니다.
			int32 EqualsIndex = INDEX_NONE;
			SelectorExpression.FindChar(TEXT('='), EqualsIndex);
			Result.SelectorKeyPropertyName = FName(*SelectorExpression.Left(EqualsIndex));

			// Registry pattern은 selector value를 비워 wildcard identity로 유지합니다.
			Result.SelectorKeyValue = NAME_None;

			// ]. 뒤에 남은 element property chain 문자열입니다.
			const FString Tail = CanonicalPattern.Mid(CloseBracketIndex + 1).TrimStartAndEnd().Replace(TEXT("."), TEXT(" "), ESearchCase::CaseSensitive);
			// 공백으로 분리할 property 이름 목록입니다.
			TArray<FString> TailParts;
			Tail.ParseIntoArrayWS(TailParts);
			for (const FString& TailPart : TailParts)
			{
				Result.PropertyChain.Add(FName(*TailPart));
			}
			return Result;
		}

		// Scalar/Nested path를 점 기준으로 분리한 property 이름 목록입니다.
		TArray<FString> ScalarParts;
		CanonicalPattern.ParseIntoArray(ScalarParts, TEXT("."), true);
		for (const FString& ScalarPart : ScalarParts)
		{
			Result.PropertyChain.Add(FName(*ScalarPart));
		}
		return Result;
	}

	// FVector/FRotator/FTransform과 complete Transmission ratio-set은 Registry에서 내부 component가 아니라 하나의 atomic leaf value로 취급합니다.
	bool IsAtomicStruct(const UScriptStruct* Struct)
	{
		return Struct == TBaseStructure<FVector>::Get()
			|| Struct == TBaseStructure<FRotator>::Get()
			|| Struct == TBaseStructure<FTransform>::Get()
			|| Struct == FCFVehicleTransmissionRatios::StaticStruct();
	}

	// Reflection struct를 재귀 순회해 scalar leaf pattern을 추가합니다.
	void DiscoverStructLeaves(const UStruct* Struct, const FString& Prefix, TSet<FString>& OutPatterns)
	{
		for (TFieldIterator<FProperty> PropertyIt(Struct); PropertyIt; ++PropertyIt)
		{
			// 현재 struct가 직접 선언한 property만 처리해 상위 UObject field를 배제합니다.
			FProperty* Property = *PropertyIt;
			if (!Property || Property->GetOwnerStruct() != Struct)
			{
				continue;
			}

			// 현재 property까지 포함한 canonical path입니다.
			const FString PropertyPath = Prefix.IsEmpty()
				? Property->GetName()
				: Prefix + TEXT(".") + Property->GetName();

			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (!IsAtomicStruct(StructProperty->Struct))
				{
					DiscoverStructLeaves(StructProperty->Struct, PropertyPath, OutPatterns);
					continue;
				}
			}

			OutPatterns.Add(PropertyPath);
		}
	}

	// Stable-ID top-level array를 wildcard selector pattern으로 flatten합니다.
	void DiscoverArrayLeaves(const FArrayProperty& ArrayProperty, const FName SelectorKey, TSet<FString>& OutPatterns)
	{
		// VehicleData stable collections는 struct element만 허용합니다.
		const FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty.Inner);
		if (!InnerStructProperty || !InnerStructProperty->Struct)
		{
			return;
		}

		// Array element leaf에 붙일 canonical stable selector prefix입니다.
		const FString Prefix = FString::Printf(TEXT("%s[%s=*]"), *ArrayProperty.GetName(), *SelectorKey.ToString());
		DiscoverStructLeaves(InnerStructProperty->Struct, Prefix, OutPatterns);
	}

	// Current exact 127 leaf descriptor를 생성합니다.
	TArray<FCFVehicleFieldDescriptor> BuildDescriptors()
	{
		// Current 127 leaf seed입니다. 숫자값은 없고 ownership/rule metadata만 기술합니다.
		static const FDescriptorSeed Seeds[] =
		{
			{TEXT("VehicleVisualConfig.ChassisMesh"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeAssetIntent, ECFVehicleAdoptGroup::VisualAssets, false, false, false},
			{TEXT("VehicleVisualConfig.WheelMeshFL"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeAssetIntent, ECFVehicleAdoptGroup::VisualAssets, false, false, false},
			{TEXT("VehicleVisualConfig.WheelMeshFR"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeAssetIntent, ECFVehicleAdoptGroup::VisualAssets, false, false, false},
			{TEXT("VehicleVisualConfig.WheelMeshRL"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeAssetIntent, ECFVehicleAdoptGroup::VisualAssets, false, false, false},
			{TEXT("VehicleVisualConfig.WheelMeshRR"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeAssetIntent, ECFVehicleAdoptGroup::VisualAssets, false, false, false},

			{TEXT("VehicleLayoutConfig.bUseLayoutOverrides"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::DerivedGate, ECFVehicleAdoptGroup::Layout, false, false, false},
			{TEXT("VehicleLayoutConfig.BodyWheelSocketFL"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeBinding, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.BodyWheelSocketFR"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeBinding, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.BodyWheelSocketRL"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeBinding, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.BodyWheelSocketRR"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeBinding, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeLocation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeRotation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorFR.RelativeLocation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorFR.RelativeRotation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorRL.RelativeLocation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorRL.RelativeRotation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorRR.RelativeLocation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},
			{TEXT("VehicleLayoutConfig.WheelAnchorRR.RelativeRotation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Layout, true, false, false},

			{TEXT("HardpointSlots[LocationSlotId=*].LocationSlotId"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Hardpoints, false, true, false},
			{TEXT("HardpointSlots[LocationSlotId=*].LocationCategory"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Hardpoints, false, false, false},
			{TEXT("HardpointSlots[LocationSlotId=*].SocketName"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeBinding, ECFVehicleAdoptGroup::Hardpoints, false, false, false},
			{TEXT("HardpointSlots[LocationSlotId=*].LocalLocation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Hardpoints, true, false, false},
			{TEXT("HardpointSlots[LocationSlotId=*].LocalRotation"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::AssetSocketDerived, ECFVehicleAdoptGroup::Hardpoints, true, false, false},

			{TEXT("MountProfiles[MountProfileId=*].MountProfileId"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Mounts, false, true, false},
			{TEXT("MountProfiles[MountProfileId=*].LocationSlotRef"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Mounts, false, false, false},
			{TEXT("MountProfiles[MountProfileId=*].MountType"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Mounts, true, false, false},
			{TEXT("MountProfiles[MountProfileId=*].SizeLimit"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Mounts, true, false, false},
			{TEXT("MountProfiles[MountProfileId=*].DefaultEquipmentPresetData"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Mounts, true, false, false},
			{TEXT("MountProfiles[MountProfileId=*].TurretYawMesh"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].TurretPitchMesh"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].TurretYawRelativeTransform"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].PitchPivotSocketName"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].TurretPitchRelativeTransform"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].YawTurnRateDegPerSec"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].PitchTurnRateDegPerSec"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].StabilizationToleranceDeg"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].AimSettleTimeSeconds"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].MountWeightKg"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::LegacySerializedPassthrough, ECFVehicleAdoptGroup::LegacyTechnical, false, false, true},
			{TEXT("MountProfiles[MountProfileId=*].bExposedModule"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::RecipeSemantic, ECFVehicleAdoptGroup::Mounts, true, false, false},

			{TEXT("BaseVehicleMassKg"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileThenRecipeExplicit, ECFVehicleAdoptGroup::MassDurability, false, false, false},
			{TEXT("MaximumGrossMassKg"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileThenRecipeExplicit, ECFVehicleAdoptGroup::MassDurability, true, false, false},

			{TEXT("VehicleMovementConfig.bUseMovementOverrides"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::DerivedGate, ECFVehicleAdoptGroup::DerivedState, false, false, false},
			{TEXT("VehicleMovementConfig.MovementProfileName"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::ProjectCompatibilityDefault, ECFVehicleAdoptGroup::LegacyTechnical, false, false, false},
			{TEXT("VehicleMovementConfig.ThrottleInputScale"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::AccelerationFeelDerived, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::SteeringFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelMaxBrakeTorque"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelMaxBrakeTorque"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelMaxHandBrakeTorque"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelRadius"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileMeasurementPolicy, ECFVehicleAdoptGroup::WheelGeometry, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelRadius"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileMeasurementPolicy, ECFVehicleAdoptGroup::WheelGeometry, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelWidth"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileMeasurementPolicy, ECFVehicleAdoptGroup::WheelGeometry, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelWidth"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileMeasurementPolicy, ECFVehicleAdoptGroup::WheelGeometry, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelFrictionForceMultiplier"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::GripFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelFrictionForceMultiplier"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::GripFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelCorneringStiffness"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::GripFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelCorneringStiffness"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::GripFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelLoadRatio"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelLoadRatio"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelSpringRate"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::SuspensionFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelSpringRate"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::SuspensionFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelSpringPreload"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::SuspensionFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelSpringPreload"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::SuspensionFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelSuspensionMaxRaise"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelSuspensionMaxRaise"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelSuspensionMaxDrop"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelSuspensionMaxDrop"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.bFrontWheelAffectedByEngine"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.bRearWheelAffectedByEngine"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelSweepShape"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelSweepShape"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.ChassisWidth"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::MassDurability, true, false, false},
			{TEXT("VehicleMovementConfig.ChassisHeight"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::MassDurability, true, false, false},
			{TEXT("VehicleMovementConfig.DragCoefficient"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.DownforceCoefficient"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.bEnableCenterOfMassOverride"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::DerivedGate, ECFVehicleAdoptGroup::TechnicalHandling, false, false, false},
			{TEXT("VehicleMovementConfig.CenterOfMassOverride"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::CompatibilityDefaultOrAdvanced, ECFVehicleAdoptGroup::TechnicalHandling, true, false, false},
						{TEXT("VehicleMovementConfig.EngineMaxTorque"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::AccelerationFeelDerived, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.EngineMaxRPM"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::AccelerationFeelDerived, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.RedlineStartRPM"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.EngineIdleRPM"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.EngineBrakeEffect"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.EngineRevUpMOI"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.EngineRevDownRate"), ECFVehicleProfileDomain::Performance, ECFVehicleResolveRule::PerformanceProfileDirect, ECFVehicleAdoptGroup::Performance, true, false, false},
			{TEXT("VehicleMovementConfig.DifferentialType"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.FrontRearSplit"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.bUseAutomaticGears"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.bUseAutoReverse"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.TransmissionRatios"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.FinalRatio"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.ChangeUpRPM"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.ChangeDownRPM"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.GearChangeTime"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.TransmissionEfficiency"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleMovementConfig.SteeringType"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::HandlingProfileDirect, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.SteeringAngleRatio"), ECFVehicleProfileDomain::Handling, ECFVehicleResolveRule::SteeringFeelDerived, ECFVehicleAdoptGroup::Handling, true, false, false},
			{TEXT("VehicleMovementConfig.bLegacyWheelFrictionPosition"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::ProjectCompatibilityDefault, ECFVehicleAdoptGroup::LegacyTechnical, true, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelAdditionalOffset"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::CompatibilityDefaultOrAdvanced, ECFVehicleAdoptGroup::TechnicalHandling, true, false, false},
			{TEXT("VehicleMovementConfig.RearWheelAdditionalOffset"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::CompatibilityDefaultOrAdvanced, ECFVehicleAdoptGroup::TechnicalHandling, true, false, false},

			{TEXT("WheelVisualConfig.bUseWheelVisualOverrides"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::DerivedGate, ECFVehicleAdoptGroup::WheelVisual, false, false, false},
			{TEXT("WheelVisualConfig.ExpectedWheelCount"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::WheelVisual, true, false, false},
			{TEXT("WheelVisualConfig.FrontWheelCountForSteering"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::WheelVisual, true, false, false},
			{TEXT("WheelVisualConfig.bAutoScaleWheelMeshToRadius"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::RecipeWheelVisualPolicy, ECFVehicleAdoptGroup::WheelVisual, false, false, false},
			{TEXT("WheelVisualConfig.WheelMeshRadiusMeasureMode"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileMeasurementPolicy, ECFVehicleAdoptGroup::WheelGeometry, true, false, false},
			{TEXT("WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::WheelVisual, true, false, false},
			{TEXT("WheelVisualConfig.WheelMeshScaleClampMin"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::WheelVisual, true, false, false},
			{TEXT("WheelVisualConfig.WheelMeshScaleClampMax"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::VehicleBaseProfile, ECFVehicleAdoptGroup::WheelVisual, true, false, false},

			{TEXT("VehicleReferenceConfig.FrontWheelClass"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},
			{TEXT("VehicleReferenceConfig.RearWheelClass"), ECFVehicleProfileDomain::Drivetrain, ECFVehicleResolveRule::DrivetrainProfile, ECFVehicleAdoptGroup::Drivetrain, true, false, false},

			{TEXT("VehicleDurabilityConfig.MaxHealth"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileThenRecipeExplicit, ECFVehicleAdoptGroup::MassDurability, true, false, false},
			{TEXT("DefaultDefenseData"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileThenRecipeAsset, ECFVehicleAdoptGroup::DefenseFx, true, false, false},
			{TEXT("DefaultDestroyedFxData"), ECFVehicleProfileDomain::VehicleBase, ECFVehicleResolveRule::BaseProfileThenRecipeAsset, ECFVehicleAdoptGroup::DefenseFx, true, false, false},
			{TEXT("DestroyedFxSocketName"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::ProjectDefaultThenRecipeSemantic, ECFVehicleAdoptGroup::DefenseFx, true, false, false},

			{TEXT("DriveStateConfig.bUseDriveStateOverrides"), ECFVehicleProfileDomain::None, ECFVehicleResolveRule::DerivedGate, ECFVehicleAdoptGroup::DriveState, false, false, false},
			{TEXT("DriveStateConfig.bEnableDriveStateHysteresis"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.bUsePerStateHoldTimes"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.DriveStateMinimumHoldTimeSeconds"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.IdleStateMinimumHoldTimeSeconds"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.ReversingStateMinimumHoldTimeSeconds"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.AirborneStateMinimumHoldTimeSeconds"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.IdleEnterSpeedThresholdKmh"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.IdleExitSpeedThresholdKmh"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.ReverseEnterSpeedThresholdKmh"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.ReverseExitSpeedThresholdKmh"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.AirborneMinSpeedThresholdKmh"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.AirborneVerticalSpeedThresholdCmPerSec"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.ActiveInputThreshold"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false},
			{TEXT("DriveStateConfig.bTreatOppositeThrottleAsBrake"), ECFVehicleProfileDomain::DriveState, ECFVehicleResolveRule::DriveStateProfile, ECFVehicleAdoptGroup::DriveState, true, false, false}
		};

		// 최종 immutable-style descriptor 배열입니다.
		TArray<FCFVehicleFieldDescriptor> Descriptors;
		Descriptors.Reserve(UE_ARRAY_COUNT(Seeds));
		for (const FDescriptorSeed& Seed : Seeds)
		{
			// Seed에서 확장할 descriptor입니다.
			FCFVehicleFieldDescriptor Descriptor;
			Descriptor.StablePathPattern = ParsePattern(Seed.CanonicalPattern);
			Descriptor.PrimaryProfileDomain = Seed.Domain;
			Descriptor.ResolveRule = Seed.Rule;
			Descriptor.AdoptionGroup = Seed.Group;
			Descriptor.AllowedSourceMask = BuildAllowedSourceMask(Seed);
						Descriptor.bAdvancedOverrideAllowed = Seed.bOverride;
			Descriptor.bIdentityField = Seed.bIdentity;
			Descriptor.bLegacySerialized = Seed.bLegacy;
			Descriptor.RequiredDependencies = BuildRequiredDependencies(Seed);
			Descriptors.Add(MoveTemp(Descriptor));
		}

		Descriptors.Sort([](const FCFVehicleFieldDescriptor& Left, const FCFVehicleFieldDescriptor& Right)
		{
			return Left.GetCanonicalPattern() < Right.GetCanonicalPattern();
		});
		return Descriptors;
	}
}

// Current P0 descriptor 127개를 canonical 순서로 반환합니다.
const TArray<FCFVehicleFieldDescriptor>& FCFVehicleFieldRegistry::GetDescriptors()
{
	// 최초 호출 시 한 번 구성되는 immutable descriptor cache입니다.
	static const TArray<FCFVehicleFieldDescriptor> Descriptors = CFVehicleFieldRegistryPrivate::BuildDescriptors();
	return Descriptors;
}

// Current UCFVehicleData Reflection에서 실제 leaf pattern을 재발견합니다.
TSet<FString> FCFVehicleFieldRegistry::DiscoverDefinitionLeafPatterns()
{
	// Reflection으로 발견한 canonical leaf pattern 집합입니다.
	TSet<FString> Patterns;

	for (TFieldIterator<FProperty> PropertyIt(UCFVehicleData::StaticClass()); PropertyIt; ++PropertyIt)
	{
		// UCFVehicleData가 직접 선언한 top-level UPROPERTY만 처리합니다.
		FProperty* Property = *PropertyIt;
		if (!Property || Property->GetOwnerStruct() != UCFVehicleData::StaticClass())
		{
			continue;
		}

		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UCFVehicleData, HardpointSlots))
			{
				CFVehicleFieldRegistryPrivate::DiscoverArrayLeaves(*ArrayProperty, TEXT("LocationSlotId"), Patterns);
				continue;
			}
			if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UCFVehicleData, MountProfiles))
			{
				CFVehicleFieldRegistryPrivate::DiscoverArrayLeaves(*ArrayProperty, TEXT("MountProfileId"), Patterns);
				continue;
			}
		}

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (!CFVehicleFieldRegistryPrivate::IsAtomicStruct(StructProperty->Struct))
			{
				CFVehicleFieldRegistryPrivate::DiscoverStructLeaves(StructProperty->Struct, Property->GetName(), Patterns);
				continue;
			}
		}

		Patterns.Add(Property->GetName());
	}

	return Patterns;
}

// Registry와 Current Reflection leaf를 양방향 비교하고 문제 목록을 반환합니다.
bool FCFVehicleFieldRegistry::ValidateCoverage(TArray<FString>& OutErrors)
{
	OutErrors.Reset();

	// Frozen Registry descriptor 목록입니다.
	const TArray<FCFVehicleFieldDescriptor>& Descriptors = GetDescriptors();
	if (Descriptors.Num() != ExpectedLeafPatternCount)
	{
		OutErrors.Add(FString::Printf(TEXT("Registry descriptor count mismatch. Expected=%d Actual=%d"), ExpectedLeafPatternCount, Descriptors.Num()));
	}

	// Duplicate descriptor를 찾기 위한 canonical pattern 집합입니다.
	TSet<FString> RegistryPatterns;
	for (const FCFVehicleFieldDescriptor& Descriptor : Descriptors)
	{
		// Descriptor의 canonical wildcard identity입니다.
		const FString Pattern = Descriptor.GetCanonicalPattern();
		if (RegistryPatterns.Contains(Pattern))
		{
			OutErrors.Add(FString::Printf(TEXT("Duplicate Registry descriptor: %s"), *Pattern));
			continue;
		}
		RegistryPatterns.Add(Pattern);
	}

	// Current runtime UCFVehicleData source에서 reflection으로 발견한 leaf pattern입니다.
	const TSet<FString> CurrentPatterns = DiscoverDefinitionLeafPatterns();
	if (CurrentPatterns.Num() != ExpectedLeafPatternCount)
	{
		OutErrors.Add(FString::Printf(TEXT("Current UCFVehicleData leaf count mismatch. Expected=%d Actual=%d"), ExpectedLeafPatternCount, CurrentPatterns.Num()));
	}

	for (const FString& CurrentPattern : CurrentPatterns)
	{
		if (!RegistryPatterns.Contains(CurrentPattern))
		{
			OutErrors.Add(FString::Printf(TEXT("Current field has no Registry descriptor: %s"), *CurrentPattern));
		}
	}

	for (const FString& RegistryPattern : RegistryPatterns)
	{
		if (!CurrentPatterns.Contains(RegistryPattern))
		{
			OutErrors.Add(FString::Printf(TEXT("Registry descriptor has no Current field: %s"), *RegistryPattern));
		}
	}

	return OutErrors.IsEmpty();
}
