// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoDace.cpp
// Version: v1.1.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-04 AmmoData independent DACE current descriptor/migration authority 구현입니다.
// Changelog:
// - v1.1.0: mutable current descriptor와 accepted history의 accidental rebaseline 결합을 제거하기 위해 GetAcceptedSnapshots 구현을 CFDAAmmoDaceBase.cpp로 분리했습니다.
// - v1.0.0: Ammo exact8 SourceShape, strict AdapterShape, SourceAdapterMapping, SemanticContract와 DACE-AmmoData bootstrap exact1/current migration gate를 구현.
// Migration:
// - accepted Ammo history implementation은 dedicated CFDAAmmoDaceBase.cpp가 소유하며 기존 bootstrap 값/signature는 변경하지 않습니다.
// - accepted Ammo history는 CFDAContractBase.cpp의 Missile history와 물리적으로 분리합니다.
// - Product canonical Ammo exact0은 valid empty target set이며 fake Product asset/Staging 생성 없이 compatibility PASS가 가능합니다.

#include "CFDAAmmoDace.h"

#include "CFAmmoData.h"
#include "CFDAAmmoProvider.h"

namespace CFDAAmmoDacePrivate
{
	// Ammo SourceShape canonical root class path입니다.
	static constexpr TCHAR SourceRootClassPath[] = TEXT("/Script/CarFight_Re.CFAmmoData");

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

// Current AmmoData direct authored SourceShape exact8 descriptor를 반환합니다.
const TArray<FCFDASourceFieldDescriptor>& FCFDAAmmoDace::GetSourceShapeDescriptor()
{
	// Ammo authored exact8 immutable Source descriptor입니다.
	static const TArray<FCFDASourceFieldDescriptor> Descriptors =
	{
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("AmmoId"), TEXT("Name"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("AmmoDisplayName"), TEXT("Text"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("AmmoFamilyId"), TEXT("Name"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("UnitMassKg"), TEXT("Float"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("AmmoTags"), TEXT("Array"), TEXT("Array<Name>"), TEXT("Array"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("AmmoIcon"), TEXT("SoftObject"), TEXT("/Script/Engine.Texture2D"), TEXT("Scalar"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("MaximumLoadableAmmoCount"), TEXT("Int"), TEXT(""), TEXT("Scalar"), TEXT("")},
		{CFDAAmmoDacePrivate::SourceRootClassPath, TEXT("bCanBeResupplied"), TEXT("Bool"), TEXT(""), TEXT("Scalar"), TEXT("")}
	};
	return Descriptors;
}

// Current AmmoData strict whole-record AdapterShape descriptor를 반환합니다.
const TArray<FCFDAAdapterFieldDescriptor>& FCFDAAmmoDace::GetAdapterShapeDescriptor()
{
	// Root common envelope + Ammo payload physical contract exact19입니다.
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
		{TEXT("Payload.AmmoId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Payload")},
		{TEXT("Payload.AmmoDisplayName"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralFTextObject"), TEXT("PayloadContainer")},
		{TEXT("Payload.AmmoDisplayName.Kind"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralKind"), TEXT("RepresentationConstant")},
		{TEXT("Payload.AmmoDisplayName.Text"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralText"), TEXT("Payload")},
		{TEXT("Payload.AmmoFamilyId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Payload")},
		{TEXT("Payload.UnitMassKg"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Float"), TEXT("Payload")},
		{TEXT("Payload.AmmoTags"), TEXT("Array"), TEXT("Required"), TEXT("NonNull"), TEXT("FNameTokenArray"), TEXT("PayloadContainer")},
		{TEXT("Payload.AmmoTags[]"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("FNameToken"), TEXT("Payload")},
		{TEXT("Payload.AmmoIcon"), TEXT("StringOrNull"), TEXT("Required"), TEXT("Nullable"), TEXT("SoftObjectPath"), TEXT("Payload")},
		{TEXT("Payload.MaximumLoadableAmmoCount"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("Int32"), TEXT("Payload")},
		{TEXT("Payload.bCanBeResupplied"), TEXT("Boolean"), TEXT("Required"), TEXT("NonNull"), TEXT("Bool"), TEXT("Payload")}
	};
	return Descriptors;
}

// Current AmmoData Source↔Adapter exact mapping descriptor를 반환합니다.
const TArray<FCFDASourceAdapterMappingDescriptor>& FCFDAAmmoDace::GetSourceAdapterMappingDescriptor()
{
	// Ammo source leaf exact8과 adapter terminal field를 exact cover하는 mapping입니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor> Descriptors =
	{
		{TEXT(""), TEXT("SchemaId"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("SchemaRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("AdapterContractRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("DataAssetTypeClassPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("TargetObjectPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT(""), TEXT("BaseSemanticFingerprint"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")},
		{TEXT("AmmoId"), TEXT("StableLogicalId"), TEXT("NameToken"), TEXT("SourceToAdapter")},
		{TEXT("AmmoId"), TEXT("Payload.AmmoId"), TEXT("NameToken"), TEXT("SourceToAdapter")},
		{TEXT("AmmoDisplayName"), TEXT("Payload.AmmoDisplayName.Kind"), TEXT("LiteralKind"), TEXT("AdapterOnlyConstant")},
		{TEXT("AmmoDisplayName"), TEXT("Payload.AmmoDisplayName.Text"), TEXT("LiteralText"), TEXT("SourceToAdapter")},
		{TEXT("AmmoFamilyId"), TEXT("Payload.AmmoFamilyId"), TEXT("NameToken"), TEXT("SourceToAdapter")},
		{TEXT("UnitMassKg"), TEXT("Payload.UnitMassKg"), TEXT("Float"), TEXT("SourceToAdapter")},
		{TEXT("AmmoTags"), TEXT("Payload.AmmoTags[]"), TEXT("FNameToken"), TEXT("SourceToAdapter")},
		{TEXT("AmmoIcon"), TEXT("Payload.AmmoIcon"), TEXT("SoftObjectPath"), TEXT("SourceToAdapter")},
		{TEXT("MaximumLoadableAmmoCount"), TEXT("Payload.MaximumLoadableAmmoCount"), TEXT("Int32"), TEXT("SourceToAdapter")},
		{TEXT("bCanBeResupplied"), TEXT("Payload.bCanBeResupplied"), TEXT("Bool"), TEXT("SourceToAdapter")}
	};
	return Descriptors;
}

// Current AmmoData authored semantic policy descriptor를 반환합니다.
const TArray<FCFDASemanticRuleDescriptor>& FCFDAAmmoDace::GetSemanticContractDescriptor()
{
	// Reflection만으로 추론할 수 없는 Ammo exact semantic rules입니다.
	static const TArray<FCFDASemanticRuleDescriptor> Descriptors =
	{
		{TEXT("FText.LiteralRepresentation"), TEXT("SourceStringOnly;Kind=Literal;PackageOnlyNamespaceKeyIgnored;StringTableOrAuthoredNamespaceRejected")},
		{TEXT("FName.CasePolicy"), TEXT("SemanticComparisonCaseInsensitive;FingerprintLowercase")},
		{TEXT("Identity.StableLogicalId"), TEXT("StableLogicalIdEqualsPayload.AmmoIdByFNameSemantics")},
		{TEXT("Numeric.UnitMassKg"), TEXT("FiniteFloat32;Minimum=0;NoClampOnParseFingerprintOrMaterialize")},
		{TEXT("Numeric.MaximumLoadableAmmoCount"), TEXT("ExactInt32;Minimum=0;NoClampOnParseOrMaterialize")},
		{TEXT("AmmoTags.ContainerSemantic"), TEXT("ArrayStringPhysical;SetLikeFNameSemantic;NonNone;SemanticDuplicateRejected;CanonicalSortedMaterialize")},
		{TEXT("AmmoTags.FingerprintPolicy"), TEXT("OrderIndependent;CaseInsensitiveFNameCanonicalText;DeterministicCaseSensitiveByteSort")},
		{TEXT("AmmoIcon.PhysicalSemantic"), TEXT("NullOrCanonicalTopLevelSoftObjectPath;SubobjectRejected;NoGameMountRestriction")},
		{TEXT("AmmoIcon.TargetClass"), TEXT("AssetRegistryReadOnly;/Script/Engine.Texture2DCompatible;ReferencedAssetLoadForbidden")},
		{TEXT("AmmoIcon.FingerprintPolicy"), TEXT("NullFlagAndCanonicalPathOnly;ReferencedAssetStateExcluded")},
		{TEXT("Fingerprint.Inclusion"), TEXT("SchemaId;SchemaRevision;AdapterContractRevision;DataAssetTypeClassPath;PayloadWritableSemanticLeaves")},
		{TEXT("Fingerprint.FloatPolicy"), TEXT("IEEE754Float32;NegativeZeroCanonicalizedToPositiveZero")},
		{TEXT("Resolver.TargetPathPolicy"), TEXT("ExactGameObjectPath;PackageLeafEqualsObjectName")},
		{TEXT("Runtime.AuthoringBoundary"), TEXT("AuthoringStrictRejectNoClamp;RuntimeDefensiveClampAndAmmoIdOnlyValidationPreserved")}
	};
	return Descriptors;
}

// Current Ammo descriptor exact4 component signatures를 계산합니다.
bool FCFDAAmmoDace::BuildCurrentSignatures(
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

// Current accepted baseline 대비 미승인 contract delta declaration을 반환합니다. 현재 baseline과 동일하므로 nullptr입니다.
const FCFDACurrentChangeDeclaration* FCFDAAmmoDace::GetCurrentChangeDeclaration()
{
	return nullptr;
}

// Reflection/mapping/accepted-chain/revision을 결합해 current Ammo DACE contract를 fail-closed 검증합니다.
FCFDAContractGuardResult FCFDAAmmoDace::ValidateCurrentContract()
{
	// Aggregate current contract validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Native CFAmmoData direct authored Reflection 결과입니다.
	TArray<FCFDASourceFieldDescriptor> ReflectedSourceDescriptors;
	// Reflection 실행 실패 원인입니다.
	FString ReflectionError;
	if (!FCFDAContractGuard::BuildDirectReflectedSourceShapeDescriptor(
		*UCFAmmoData::StaticClass(),
		ReflectedSourceDescriptors,
		ReflectionError))
	{
		CFDAAmmoDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::SourceAuthoringContractDrift,
			TEXT("SourceShape"),
			ReflectionError);
		return Result;
	}
	CFDAAmmoDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateSourceShapeCoverage(GetSourceShapeDescriptor(), ReflectedSourceDescriptors));
	CFDAAmmoDacePrivate::AppendGuardResult(
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
		CFDAAmmoDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::AcceptedSnapshotChainInvalid,
			TEXT("CurrentSignatures"),
			SignatureError);
		return Result;
	}

	// Current trusted Ammo provider entry입니다.
	const FCFDAAmmoTypeProvider& ProviderEntry = CFDAAmmoProvider::GetProvider();
	// Independent Ammo accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& AcceptedSnapshots = GetAcceptedSnapshots();
	CFDAAmmoDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
			ProviderEntry.Descriptor,
			AcceptedSnapshots));
	if (AcceptedSnapshots.IsEmpty())
	{
		CFDAAmmoDacePrivate::AddExecutionIssue(
			Result,
			ECFDAContractIssueCode::AcceptedSnapshotChainInvalid,
			TEXT("AcceptedHistory"),
			TEXT("DACE-AmmoData accepted history가 비었습니다."));
		return Result;
	}

	CFDAAmmoDacePrivate::AppendGuardResult(
		Result,
		FCFDAContractGuard::ValidateRevisionGuardForProvider(
			ProviderEntry.Descriptor,
			AcceptedSnapshots.Last(),
			CurrentSignatures,
			GetCurrentChangeDeclaration()));
	return Result;
}

// Product canonical exact0 compatibility와 migration state를 결합해 append/promotion gate를 평가합니다.
FCFDAMigrationGateResult FCFDAAmmoDace::EvaluateCurrentMigrationGate()
{
	// Current trusted Ammo provider entry입니다.
	const FCFDAAmmoTypeProvider& ProviderEntry = CFDAAmmoProvider::GetProvider();
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
