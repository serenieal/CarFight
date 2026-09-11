// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageDace.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-052 DDO-P0-03 DamageData independent DACE current descriptor/migration authority 구현입니다.
// Changelog:
// - v1.0.0: Damage SourceShape exact12, AdapterShape exact20, SourceAdapterMapping exact19, SemanticContract exact14와 provider-bound current contract/migration gate를 최초 구현했습니다.
// Migration:
// - Accepted Damage history는 dedicated CFDADamageDaceBase.cpp가 소유합니다.
// - shared FCFDAContractGuard, Missile/Ammo DACE history와 mixed operational admission은 변경하지 않습니다.
// - Product canonical Damage target exact0은 valid explicit empty set이며 persisted protected Damage exact2를 canonical DACE target으로 편입하지 않습니다.

#include "CFDADamageDace.h"

#include "CFDADamageProvider.h"
#include "CFDamageData.h"

namespace CFDADamageDacePrivate
{
	// Damage SourceShape canonical root class path입니다.
	static constexpr TCHAR SourceRootClassPath[] = TEXT("/Script/CarFight_Re.CFDamageData");

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

// Current DamageData direct authored SourceShape exact12 descriptor를 반환합니다.
const TArray<FCFDASourceFieldDescriptor>& FCFDADamageDace::GetSourceShapeDescriptor()
{
	// Damage authored exact12 immutable Source descriptor입니다.
	static const TArray<FCFDASourceFieldDescriptor> Descriptors =
	{
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("DamageId"), TEXT("Name"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("DamageType"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFDamageType"), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("BaseDamage"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("bCanDamageSelf"), TEXT("Bool"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("ArmorPenetration"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("bUseRadialDamage"), TEXT("Bool"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("ExplosionRadius"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("ExplosionInnerRadius"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("ExplosionDamage"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("MinExplosionDamageScale"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("ModuleDamageScale"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDADamageDacePrivate::SourceRootClassPath, TEXT("ImpulseStrength"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")}
	};
	return Descriptors;
}

// Current DamageData strict whole-record AdapterShape exact20 descriptor를 반환합니다.
const TArray<FCFDAAdapterFieldDescriptor>& FCFDADamageDace::GetAdapterShapeDescriptor()
{
	// Root common envelope exact8 + Damage payload exact12 physical contract입니다.
	static const TArray<FCFDAAdapterFieldDescriptor> Descriptors =
	{
		{TEXT("SchemaId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("SchemaId"), TEXT("Metadata")},
		{TEXT("SchemaRevision"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("IntegerRevision"), TEXT("Metadata")},
		{TEXT("AdapterContractRevision"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("IntegerRevision"), TEXT("Metadata")},
		{TEXT("DataAssetTypeClassPath"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("ClassPath"), TEXT("Metadata")},
		{TEXT("StableLogicalId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Identity")},
		{TEXT("TargetObjectPath"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("ObjectPath"), TEXT("Metadata")},
		{TEXT("BaseSemanticFingerprint"), TEXT("StringOrNull"), TEXT("Required"), TEXT("Nullable"), TEXT("Sha256OrNull"), TEXT("Metadata")},
		{TEXT("Payload"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("WholeRecordObject"), TEXT("PayloadContainer")},
		{TEXT("Payload.DamageId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Payload")},
		{TEXT("Payload.DamageType"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("EnumNameToken"), TEXT("Payload")},
		{TEXT("Payload.BaseDamage"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.bCanDamageSelf"), TEXT("Boolean"), TEXT("Required"), TEXT("NonNull"), TEXT("Bool"), TEXT("Payload")},
		{TEXT("Payload.ArmorPenetration"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.bUseRadialDamage"), TEXT("Boolean"), TEXT("Required"), TEXT("NonNull"), TEXT("Bool"), TEXT("Payload")},
		{TEXT("Payload.ExplosionRadius"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.ExplosionInnerRadius"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.ExplosionDamage"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.MinExplosionDamageScale"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.ModuleDamageScale"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.ImpulseStrength"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")}
	};
	return Descriptors;
}

// Current DamageData Source↔Adapter exact19 mapping descriptor를 반환합니다.
const TArray<FCFDASourceAdapterMappingDescriptor>& FCFDADamageDace::GetSourceAdapterMappingDescriptor()
{
	// Adapter-only metadata exact6 + DamageId dual projection exact2 + remaining authored exact11 mapping입니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor> Descriptors =
	{
		{TEXT(""), TEXT("SchemaId"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("SchemaRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("AdapterContractRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("DataAssetTypeClassPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("TargetObjectPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("BaseSemanticFingerprint"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT("DamageId"), TEXT("StableLogicalId"), TEXT("NameToken"), TEXT("SourceToAdapter")},
		{TEXT("DamageId"), TEXT("Payload.DamageId"), TEXT("NameToken"), TEXT("SourceToAdapter")},
		{TEXT("DamageType"), TEXT("Payload.DamageType"), TEXT("EnumNameToken"), TEXT("SourceToAdapter")},
		{TEXT("BaseDamage"), TEXT("Payload.BaseDamage"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("bCanDamageSelf"), TEXT("Payload.bCanDamageSelf"), TEXT("Bool"), TEXT("SourceToAdapter")},
		{TEXT("ArmorPenetration"), TEXT("Payload.ArmorPenetration"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("bUseRadialDamage"), TEXT("Payload.bUseRadialDamage"), TEXT("Bool"), TEXT("SourceToAdapter")},
		{TEXT("ExplosionRadius"), TEXT("Payload.ExplosionRadius"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("ExplosionInnerRadius"), TEXT("Payload.ExplosionInnerRadius"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("ExplosionDamage"), TEXT("Payload.ExplosionDamage"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("MinExplosionDamageScale"), TEXT("Payload.MinExplosionDamageScale"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("ModuleDamageScale"), TEXT("Payload.ModuleDamageScale"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("ImpulseStrength"), TEXT("Payload.ImpulseStrength"), TEXT("Float"), TEXT("SourceToAdapter")}
	};
	return Descriptors;
}

// Current DamageData authored SemanticContract exact14 descriptor를 반환합니다.
const TArray<FCFDASemanticRuleDescriptor>& FCFDADamageDace::GetSemanticContractDescriptor()
{
	// Reflection만으로 추론할 수 없는 Damage exact semantic rules입니다.
	static const TArray<FCFDASemanticRuleDescriptor> Descriptors =
	{
		{TEXT("FName.CasePolicy"), TEXT("SemanticComparisonCaseInsensitive;FingerprintLowercase")},
		{TEXT("Enum.DamageType.TokenPolicy"), TEXT("ExactCaseSensitiveTokens=None|Kinetic|Explosive|Energy;NumericAliasUnknownRejected")},
		{TEXT("Identity.StableLogicalId"), TEXT("StableLogicalIdEqualsPayload.DamageIdByFNameSemantics")},
		{TEXT("Numeric.FinitePolicy"), TEXT("AuthoredFloatExact8AllFinite;NoClampOnParseFingerprintOrMaterialize")},
		{TEXT("Numeric.BaseDamage"), TEXT("FiniteFloat32;StrictGreaterThanZero;NoClamp")},
		{TEXT("Numeric.NonNegative"), TEXT("ArmorPenetration|ExplosionRadius|ExplosionInnerRadius|ExplosionDamage|ModuleDamageScale|ImpulseStrength;Minimum=0;NoClamp")},
		{TEXT("Numeric.MinExplosionDamageScale"), TEXT("FiniteFloat32;RangeInclusive=0..1;NoClamp")},
		{TEXT("Radial.EnabledReadiness"), TEXT("bUseRadialDamageTrueRequiresExplosionRadiusGreaterThanZeroAndExplosionDamageGreaterThanZero")},
		{TEXT("Radial.DisabledPreservation"), TEXT("FalsePreservesIndividuallyValidRadialValues;AutoZeroForbidden")},
		{TEXT("Radial.InnerRadiusPolicy"), TEXT("NoExplosionInnerRadiusLessOrEqualExplosionRadiusInvariant")},
		{TEXT("Fingerprint.Inclusion"), TEXT("SchemaId;SchemaRevision;AdapterContractRevision;DataAssetTypeClassPath;PayloadExact12AuthoredSemanticLeaves")},
		{TEXT("Fingerprint.FloatPolicy"), TEXT("IEEE754Float32;NegativeZeroCanonicalizedToPositiveZero")},
		{TEXT("Resolver.TargetPathPolicy"), TEXT("ExactGameObjectPath;PackageLeafEqualsObjectName")},
		{TEXT("Runtime.AuthoringBoundary"), TEXT("AuthoringStrictRejectNoClamp;ExistingRuntimeDefensiveSemanticsPreserved;NoNewDamageFormula")}
	};
	return Descriptors;
}

// Current Damage descriptor exact4 component signatures를 shared canonical algorithm으로 계산합니다.
bool FCFDADamageDace::BuildCurrentSignatures(
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
const FCFDACurrentChangeDeclaration* FCFDADamageDace::GetCurrentChangeDeclaration()
{
	return nullptr;
}

// Reflection/mapping/accepted-chain/revision을 결합해 current Damage DACE contract를 fail-closed 검증합니다.
FCFDAContractGuardResult FCFDADamageDace::ValidateCurrentContract()
{
	// Aggregate current contract validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Native CFDamageData direct authored Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> ReflectedSourceDescriptors;
	// Reflection 실행 실패 원인입니다.
	FString ReflectionError;
	if (!FCFDAContractGuard::BuildDirectReflectedSourceShapeDescriptor(
		*UCFDamageData::StaticClass(),
		ReflectedSourceDescriptors,
		ReflectionError))
	{
		CFDADamageDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::SourceAuthoringContractDrift,
			TEXT("SourceShape"),
			ReflectionError);
		return Result;
	}
	CFDADamageDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateSourceShapeCoverage(GetSourceShapeDescriptor(), ReflectedSourceDescriptors));
	CFDADamageDacePrivate::AppendGuardResult(
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
		CFDADamageDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::AcceptedSnapshotChainInvalid,
			TEXT("CurrentSignatures"),
			SignatureError);
		return Result;
	}

	// Current trusted Damage provider entry입니다.
	const FCFDADamageTypeProvider& ProviderEntry = CFDADamageProvider::GetProvider();
	// Independent Damage accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& AcceptedSnapshots = GetAcceptedSnapshots();
	CFDADamageDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
			ProviderEntry.Descriptor,
			AcceptedSnapshots));
	if (AcceptedSnapshots.IsEmpty())
	{
		CFDADamageDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::AcceptedSnapshotChainInvalid,
			TEXT("AcceptedHistory"),
			TEXT("DACE-DamageData accepted history가 비었습니다."));
		return Result;
	}

	CFDADamageDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateRevisionGuardForProvider(
			ProviderEntry.Descriptor,
			AcceptedSnapshots.Last(),
			CurrentSignatures,
			GetCurrentChangeDeclaration()));
	return Result;
}

// Product canonical exact0 compatibility와 no-delta migration state를 결합해 append/promotion gate를 평가합니다.
FCFDAMigrationGateResult FCFDADamageDace::EvaluateCurrentMigrationGate()
{
	// Current trusted Damage provider entry입니다.
	const FCFDADamageTypeProvider& ProviderEntry = CFDADamageProvider::GetProvider();
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
