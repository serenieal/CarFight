// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseDace.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-02 VehicleDefenseData independent DACE current descriptor/migration authority 구현입니다.
// Changelog:
// - v1.0.0: VehicleDefense SourceShape exact29, AdapterShape exact37, SourceAdapterMapping exact30, SemanticContract exact14와 provider-bound current contract/migration gate를 최초 구현했습니다.
// Migration:
// - Accepted VehicleDefense history는 dedicated CFDAVehicleDefenseDaceBase.cpp가 소유합니다.
// - shared FCFDAContractGuard와 Missile/Ammo/Damage DACE histories는 변경하지 않습니다.
// - Product canonical VehicleDefense target exact0은 valid explicit empty set이며 persisted protected VehicleDefense exact1을 canonical DACE target으로 편입하지 않습니다.

#include "CFDAVehicleDefenseDace.h"

#include "CFDAVehicleDefenseProvider.h"
#include "CFVehicleDefenseData.h"

namespace CFDAVehicleDefenseDacePrivate
{
	// VehicleDefense SourceShape canonical root class path입니다.
	static constexpr TCHAR SourceRootClassPath[] = TEXT("/Script/CarFight_Re.CFVehicleDefenseData");

	// Directional armor nested USTRUCT canonical reflected path입니다.
	static constexpr TCHAR DirectionalArmorStructPath[] = TEXT("/Script/CarFight_Re.CFDirectionalArmorConfig");

	// VehicleDefense가 소유하는 exact six armor direction property prefix입니다.
	const TArray<FString>& GetArmorDirections()
	{
		// Descriptor/mapping을 같은 authoritative direction set으로 생성합니다.
		static const TArray<FString> Directions =
		{
			TEXT("Front"),
			TEXT("Left"),
			TEXT("Right"),
			TEXT("Rear"),
			TEXT("Top"),
			TEXT("Bottom")
		};
		return Directions;
	}

	// 하나의 guard 결과를 aggregate validation에 보존합니다.
	void AppendGuardResult(
		FCFDAContractGuardResult& InOutResult,
		const FCFDAContractGuardResult& ChildResult)
	{
		if (!ChildResult.bPassed)
		{
			InOutResult.bPassed = false;
		}
		InOutResult.Issues.Append(ChildResult.Issues);
	}

	// 직접 실행 실패를 stable DACE issue로 기록합니다.
	void AddExecutionIssue(
		FCFDAContractGuardResult& InOutResult,
		const ECFDAContractIssueCode Code,
		const FString& FieldPath,
		const FString& Message)
	{
		InOutResult.bPassed = false;
		InOutResult.Issues.Add({Code, FieldPath, Message});
	}
}

// Current VehicleDefenseData recursive authored SourceShape exact29 descriptor를 반환합니다.
const TArray<FCFDASourceFieldDescriptor>& FCFDAVehicleDefenseDace::GetSourceShapeDescriptor()
{
	// VehicleDefense top-level exact17 + nested armor exact12 immutable Source descriptor입니다.
	static const TArray<FCFDASourceFieldDescriptor> Descriptors = []()
	{
		// 반환할 deterministic Source descriptor 배열입니다.
		TArray<FCFDASourceFieldDescriptor> Result;
		Result.Reserve(29);
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("DefenseId"), TEXT("Name"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("DefenseMassKg"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("bUseShield"), TEXT("Bool"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("MaximumShield"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("ShieldRegenerationDelaySeconds"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("ShieldRegenerationPerSecond"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("ArmorType"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFArmorType"), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("ArmorResistance"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		for (const FString& Direction : CFDAVehicleDefenseDacePrivate::GetArmorDirections())
		{
			// Current directional armor top-level property path입니다.
			const FString ArmorPropertyPath = Direction + TEXT("ArmorConfig");
			Result.Add({
				CFDAVehicleDefenseDacePrivate::SourceRootClassPath,
				ArmorPropertyPath,
				TEXT("Struct"),
				CFDAVehicleDefenseDacePrivate::DirectionalArmorStructPath,
				TEXT("Struct"),
				CFDAVehicleDefenseDacePrivate::DirectionalArmorStructPath});
		}
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("ShieldComponentDamageScale"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("ArmorComponentDamageScale"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, TEXT("IntegrityComponentDamageScale"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")});
		for (const FString& Direction : CFDAVehicleDefenseDacePrivate::GetArmorDirections())
		{
			// Current directional armor nested property prefix입니다.
			const FString ArmorPropertyPath = Direction + TEXT("ArmorConfig.");
			Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, ArmorPropertyPath + TEXT("MaximumArmor"), TEXT("Float"), TEXT(""), TEXT("Scalar"), CFDAVehicleDefenseDacePrivate::DirectionalArmorStructPath});
			Result.Add({CFDAVehicleDefenseDacePrivate::SourceRootClassPath, ArmorPropertyPath + TEXT("DamageMultiplier"), TEXT("Float"), TEXT(""), TEXT("Scalar"), CFDAVehicleDefenseDacePrivate::DirectionalArmorStructPath});
		}
		return Result;
	}();
	return Descriptors;
}

// Current VehicleDefenseData strict whole-record AdapterShape exact37 descriptor를 반환합니다.
const TArray<FCFDAAdapterFieldDescriptor>& FCFDAVehicleDefenseDace::GetAdapterShapeDescriptor()
{
	// Root common envelope exact8 + VehicleDefense top-level exact17 + nested armor exact12 physical contract입니다.
	static const TArray<FCFDAAdapterFieldDescriptor> Descriptors = []()
	{
		// 반환할 deterministic Adapter descriptor 배열입니다.
		TArray<FCFDAAdapterFieldDescriptor> Result;
		Result.Reserve(37);
		Result.Add({TEXT("SchemaId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("SchemaId"), TEXT("Metadata")});
		Result.Add({TEXT("SchemaRevision"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("IntegerRevision"), TEXT("Metadata")});
		Result.Add({TEXT("AdapterContractRevision"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("IntegerRevision"), TEXT("Metadata")});
		Result.Add({TEXT("DataAssetTypeClassPath"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("ClassPath"), TEXT("Metadata")});
		Result.Add({TEXT("StableLogicalId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Identity")});
		Result.Add({TEXT("TargetObjectPath"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("ObjectPath"), TEXT("Metadata")});
		Result.Add({TEXT("BaseSemanticFingerprint"), TEXT("StringOrNull"), TEXT("Required"), TEXT("Nullable"), TEXT("Sha256OrNull"), TEXT("Metadata")});
		Result.Add({TEXT("Payload"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("WholeRecordObject"), TEXT("PayloadContainer")});
		Result.Add({TEXT("Payload.DefenseId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Payload")});
		Result.Add({TEXT("Payload.DefenseMassKg"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		Result.Add({TEXT("Payload.bUseShield"), TEXT("Boolean"), TEXT("Required"), TEXT("NonNull"), TEXT("Bool"), TEXT("Payload")});
		Result.Add({TEXT("Payload.MaximumShield"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		Result.Add({TEXT("Payload.ShieldRegenerationDelaySeconds"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		Result.Add({TEXT("Payload.ShieldRegenerationPerSecond"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		Result.Add({TEXT("Payload.ArmorType"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("EnumNameToken"), TEXT("Payload")});
		Result.Add({TEXT("Payload.ArmorResistance"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		for (const FString& Direction : CFDAVehicleDefenseDacePrivate::GetArmorDirections())
		{
			Result.Add({TEXT("Payload.") + Direction + TEXT("ArmorConfig"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("WholeRecordObject"), TEXT("PayloadContainer")});
		}
		Result.Add({TEXT("Payload.ShieldComponentDamageScale"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		Result.Add({TEXT("Payload.ArmorComponentDamageScale"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		Result.Add({TEXT("Payload.IntegrityComponentDamageScale"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		for (const FString& Direction : CFDAVehicleDefenseDacePrivate::GetArmorDirections())
		{
			// Current nested Adapter path prefix입니다.
			const FString ArmorAdapterPath = TEXT("Payload.") + Direction + TEXT("ArmorConfig.");
			Result.Add({ArmorAdapterPath + TEXT("MaximumArmor"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
			Result.Add({ArmorAdapterPath + TEXT("DamageMultiplier"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")});
		}
		return Result;
	}();
	return Descriptors;
}

// Current VehicleDefenseData Source↔Adapter exact30 mapping descriptor를 반환합니다.
const TArray<FCFDASourceAdapterMappingDescriptor>& FCFDAVehicleDefenseDace::GetSourceAdapterMappingDescriptor()
{
	// Adapter-only metadata exact6 + DefenseId dual projection exact2 + remaining semantic leaf exact22 mapping입니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor> Descriptors = []()
	{
		// 반환할 deterministic Source↔Adapter mapping 배열입니다.
		TArray<FCFDASourceAdapterMappingDescriptor> Result;
		Result.Reserve(30);
		Result.Add({TEXT(""), TEXT("SchemaId"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("SchemaRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("AdapterContractRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("DataAssetTypeClassPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("TargetObjectPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("BaseSemanticFingerprint"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT("DefenseId"), TEXT("StableLogicalId"), TEXT("NameToken"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("DefenseId"), TEXT("Payload.DefenseId"), TEXT("NameToken"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("DefenseMassKg"), TEXT("Payload.DefenseMassKg"), TEXT("Float"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("bUseShield"), TEXT("Payload.bUseShield"), TEXT("Bool"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("MaximumShield"), TEXT("Payload.MaximumShield"), TEXT("Float"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("ShieldRegenerationDelaySeconds"), TEXT("Payload.ShieldRegenerationDelaySeconds"), TEXT("Float"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("ShieldRegenerationPerSecond"), TEXT("Payload.ShieldRegenerationPerSecond"), TEXT("Float"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("ArmorType"), TEXT("Payload.ArmorType"), TEXT("EnumNameToken"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("ArmorResistance"), TEXT("Payload.ArmorResistance"), TEXT("Float"), TEXT("SourceToAdapter")});
		for (const FString& Direction : CFDAVehicleDefenseDacePrivate::GetArmorDirections())
		{
			// Current nested Source/Adapter path prefix입니다.
			const FString ArmorSourcePath = Direction + TEXT("ArmorConfig.");
			// Current nested Adapter path prefix입니다.
			const FString ArmorAdapterPath = TEXT("Payload.") + Direction + TEXT("ArmorConfig.");
			Result.Add({ArmorSourcePath + TEXT("MaximumArmor"), ArmorAdapterPath + TEXT("MaximumArmor"), TEXT("Float"), TEXT("SourceToAdapter")});
			Result.Add({ArmorSourcePath + TEXT("DamageMultiplier"), ArmorAdapterPath + TEXT("DamageMultiplier"), TEXT("Float"), TEXT("SourceToAdapter")});
		}
		Result.Add({TEXT("ShieldComponentDamageScale"), TEXT("Payload.ShieldComponentDamageScale"), TEXT("Float"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("ArmorComponentDamageScale"), TEXT("Payload.ArmorComponentDamageScale"), TEXT("Float"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("IntegrityComponentDamageScale"), TEXT("Payload.IntegrityComponentDamageScale"), TEXT("Float"), TEXT("SourceToAdapter")});
		return Result;
	}();
	return Descriptors;
}

// Current VehicleDefenseData authored SemanticContract exact14 descriptor를 반환합니다.
const TArray<FCFDASemanticRuleDescriptor>& FCFDAVehicleDefenseDace::GetSemanticContractDescriptor()
{
	// Reflection만으로 추론할 수 없는 VehicleDefense exact semantic rules입니다.
	static const TArray<FCFDASemanticRuleDescriptor> Descriptors =
	{
		{TEXT("FName.CasePolicy"), TEXT("SemanticComparisonCaseInsensitive;FingerprintLowercase")},
		{TEXT("Enum.ArmorType.TokenPolicy"), TEXT("ExactCaseSensitiveTokens=Light|Standard|Heavy;NumericAliasUnknownRejected")},
		{TEXT("Identity.StableLogicalId"), TEXT("StableLogicalIdEqualsPayload.DefenseIdByFNameSemantics")},
		{TEXT("Numeric.FinitePolicy"), TEXT("AuthoredFloatExact20AllFinite;NoClampOnParseFingerprintOrMaterialize")},
		{TEXT("Numeric.NonNegative"), TEXT("AuthoredFloatExact20Minimum=0;NoClamp")},
		{TEXT("Shield.EnabledReadiness"), TEXT("bUseShieldTrueAndMaximumShieldLessOrEqualZeroRequiresShieldRegenerationPerSecondEqualZero")},
		{TEXT("Shield.DisabledPreservation"), TEXT("FalsePreservesIndividuallyValidMaximumShield|ShieldRegenerationDelaySeconds|ShieldRegenerationPerSecond;AutoZeroForbidden")},
		{TEXT("Armor.DirectionalShape"), TEXT("ExactDirections=Front|Left|Right|Rear|Top|Bottom;EachConfigExact2=MaximumArmor|DamageMultiplier;StructContainerNotSemanticLeaf")},
		{TEXT("ComponentScale.Preservation"), TEXT("ShieldComponentDamageScale|ArmorComponentDamageScale|IntegrityComponentDamageScalePreserveAuthoredRawNonNegativeValues")},
		{TEXT("Fingerprint.Inclusion"), TEXT("SchemaId;SchemaRevision;AdapterContractRevision;DataAssetTypeClassPath;PayloadSemanticLeafExact23")},
		{TEXT("Fingerprint.FloatPolicy"), TEXT("IEEE754Float32;NegativeZeroCanonicalizedToPositiveZero")},
		{TEXT("Resolver.TargetPathPolicy"), TEXT("ExactGameObjectPath;PackageLeafEqualsObjectName")},
		{TEXT("Runtime.AuthoringBoundary"), TEXT("AuthoringStrictRejectNoClamp;RuntimeDefensiveGetterProjectionNotMaterializerSource;NoGameplayFormulaChange")},
		{TEXT("Numeric.ZeroPolicy"), TEXT("DefenseMassKg|MaximumShield|ShieldRegenerationDelaySeconds|ShieldRegenerationPerSecond|ArmorResistance|MaximumArmor|DamageMultiplier|ComponentDamageScalesMayBeZeroSubjectToShieldInvariant")}
	};
	return Descriptors;
}

// Current VehicleDefense descriptor exact4 component signatures를 shared canonical algorithm으로 계산합니다.
bool FCFDAVehicleDefenseDace::BuildCurrentSignatures(
	FCFDAContractSignatures& OutSignatures,
	FString& OutError)
{
	return FCFDAContractGuard::BuildSignaturesFromDescriptors(
		GetSourceShapeDescriptor(),
		GetAdapterShapeDescriptor(),
		GetSourceAdapterMappingDescriptor(),
		GetSemanticContractDescriptor(),
		OutSignatures,
		OutError);
}

// Current accepted baseline 대비 미승인 contract delta declaration을 반환합니다. Bootstrap과 동일한 current 상태에서는 nullptr입니다.
const FCFDACurrentChangeDeclaration* FCFDAVehicleDefenseDace::GetCurrentChangeDeclaration()
{
	return nullptr;
}

// Recursive Reflection/mapping/accepted-chain/revision을 결합해 current VehicleDefense DACE contract를 fail-closed 검증합니다.
FCFDAContractGuardResult FCFDAVehicleDefenseDace::ValidateCurrentContract()
{
	// Aggregate current contract validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Native CFVehicleDefenseData recursive authored Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> ReflectedSourceDescriptors;
	// Reflection 실행 실패 원인입니다.
	FString ReflectionError;
	if (!FCFDAContractGuard::BuildRecursiveReflectedSourceShapeDescriptor(
		*UCFVehicleDefenseData::StaticClass(),
		ReflectedSourceDescriptors,
		ReflectionError))
	{
		CFDAVehicleDefenseDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::SourceAuthoringContractDrift,
			TEXT("SourceShape"),
			ReflectionError);
		return Result;
	}
	CFDAVehicleDefenseDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateSourceShapeCoverage(GetSourceShapeDescriptor(), ReflectedSourceDescriptors));
	CFDAVehicleDefenseDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateSourceAdapterMappingCoverage(
			GetSourceShapeDescriptor(),
			GetAdapterShapeDescriptor(),
			GetSourceAdapterMappingDescriptor()));

	// Current exact4 descriptor signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Signature 계산 실패 원인입니다.
	FString SignatureError;
	if (!BuildCurrentSignatures(CurrentSignatures, SignatureError))
	{
		CFDAVehicleDefenseDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::AcceptedSnapshotChainInvalid,
			TEXT("CurrentSignatures"),
			SignatureError);
		return Result;
	}

	// Current trusted VehicleDefense provider entry입니다.
	const FCFDAVehicleDefenseProvider& ProviderEntry = CFDAVehicleDefenseProvider::GetProvider();
	// Independent VehicleDefense accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& AcceptedSnapshots = GetAcceptedSnapshots();
	CFDAVehicleDefenseDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
			ProviderEntry.Descriptor,
			AcceptedSnapshots));
	if (AcceptedSnapshots.IsEmpty())
	{
		CFDAVehicleDefenseDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::AcceptedSnapshotChainInvalid,
			TEXT("AcceptedHistory"),
			TEXT("DACE-VehicleDefenseData accepted history가 비었습니다."));
		return Result;
	}

	CFDAVehicleDefenseDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateRevisionGuardForProvider(
			ProviderEntry.Descriptor,
			AcceptedSnapshots.Last(),
			CurrentSignatures,
			GetCurrentChangeDeclaration()));
	return Result;
}

// Product canonical exact0 compatibility와 no-delta migration state를 결합해 append/promotion gate를 평가합니다.
FCFDAMigrationGateResult FCFDAVehicleDefenseDace::EvaluateCurrentMigrationGate()
{
	// Current trusted VehicleDefense provider entry입니다.
	const FCFDAVehicleDefenseProvider& ProviderEntry = CFDAVehicleDefenseProvider::GetProvider();
	// Provider-owned Product canonical exact0 compatibility 결과입니다.
	const FCFDAContractGuardResult CanonicalStagingResult =
		FCFDAContractGuard::ValidateCanonicalStagingCompatibilityForProvider(ProviderEntry);
	// Provider readiness + no-delta declaration + exact0 compatibility를 결합한 migration gate입니다.
	FCFDAMigrationGateResult GateResult = FCFDAContractGuard::EvaluateMigrationGateForProvider(
		ProviderEntry,
		GetCurrentChangeDeclaration(),
		CanonicalStagingResult);
	// Descriptor/history/revision current contract validation입니다.
	const FCFDAContractGuardResult CurrentContractResult = ValidateCurrentContract();
	if (!CurrentContractResult.bPassed)
	{
		GateResult.Validation.bPassed = false;
		GateResult.Validation.Issues.Append(CurrentContractResult.Issues);
		GateResult.bAcceptedSnapshotAppendAllowed = false;
		GateResult.bCurrentSystemPromotionAllowed = false;
	}
	return GateResult;
}
