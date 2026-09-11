// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageProvider.cpp
// Version: v1.2.0
// Date: 2026-09-11
// Description: CF-FQ-052 DDO-P0-03 DamageData exact12 Reviewed durable authoring + ContractReady DACE provider 구현입니다.
// Changelog:
// - v1.2.0: dedicated CFDADamageDace descriptor/history candidate와 함께 provider DACE readiness를 ContractReady로 전환했습니다. canonical Product Damage target exact0과 mixed operational admission exact2는 유지합니다.
// - v1.1.0: exact12 materializer와 provider-local Reviewed mutation callback을 추가하고 shared CFDADurableCore를 재사용해 Damage provider를 ReviewedMutationReady로 승격했습니다.
// - v1.0.1: Mid-review P1 교정으로 BaseDamage finite > 0과 radial-enabled ExplosionRadius/ExplosionDamage > 0 Runtime-aligned validation을 추가.
// - v1.0.0: Damage exact12 strict parser/serializer, semantic fingerprint, persisted/current extractor와 ReadOnlyPreviewReady provider를 추가.
// Migration:
// - BaseDamage는 finite > 0만 허용합니다. bUseRadialDamage=true면 ExplosionRadius와 ExplosionDamage도 각각 > 0이어야 합니다.
// - bUseRadialDamage=false에서는 radial numeric authored 값을 자동 zeroing하지 않고 기존 individual range-valid 값을 그대로 보존합니다.
// - ExplosionInnerRadius와 ExplosionRadius 사이에 기존 runtime contract에 없는 교차 제약을 새로 추가하지 않습니다.
// - DDO-P0-03에서 Damage DACE exact4 descriptor + bootstrap exact1이 dedicated authority로 함께 존재하므로 DACE readiness는 ContractReady입니다. canonical Product Damage Staging target set은 explicit exact0이며 mixed operational admission은 MissileGuidePreset+AmmoData exact2를 유지합니다.

#include "CFDADamageProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDADurableCore.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFDamageData.h"
#include "DataManagement/CFDATypeRegistry.h"
#include "Dom/JsonObject.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"

namespace CFDADamageProviderPrivate
{
	// Damage exact12 payload JSON field set입니다.
	const TArray<FString>& GetPayloadFields()
	{
		// Process lifetime 동안 재사용할 exact payload field 목록입니다.
		static const TArray<FString> PayloadFields =
		{
			TEXT("DamageId"),
			TEXT("DamageType"),
			TEXT("BaseDamage"),
			TEXT("bCanDamageSelf"),
			TEXT("ArmorPenetration"),
			TEXT("bUseRadialDamage"),
			TEXT("ExplosionRadius"),
			TEXT("ExplosionInnerRadius"),
			TEXT("ExplosionDamage"),
			TEXT("MinExplosionDamageScale"),
			TEXT("ModuleDamageScale"),
			TEXT("ImpulseStrength")
		};
		return PayloadFields;
	}

	// Damage whole-record top-level exact field set입니다.
	const TArray<FString>& GetTopLevelFields()
	{
		// Process lifetime 동안 재사용할 exact top-level field 목록입니다.
		static const TArray<FString> TopLevelFields =
		{
			TEXT("SchemaId"),
			TEXT("SchemaRevision"),
			TEXT("AdapterContractRevision"),
			TEXT("DataAssetTypeClassPath"),
			TEXT("StableLogicalId"),
			TEXT("TargetObjectPath"),
			TEXT("BaseSemanticFingerprint"),
			TEXT("Payload")
		};
		return TopLevelFields;
	}

	// JSON string field를 required FName semantic으로 parse합니다.
	bool ParseRequiredNameField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		FName& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// JSON source string입니다.
		FString NameText;
		if (!CFDACommonPrimitives::ParseStringField(Object, FieldName, FieldPath, NameText, OutIssues))
		{
			return false;
		}

		OutValue = FName(*NameText);
		if (OutValue.IsNone())
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::StableIdentityMissing,
				FieldPath,
				TEXT("required Damage FName identity는 NAME_None/empty일 수 없습니다."));
			return false;
		}
		return true;
	}

	// DamageType enum 값을 canonical JSON/fingerprint token으로 변환합니다.
	bool TryGetDamageTypeToken(const ECFDamageType DamageType, FString& OutToken)
	{
		switch (DamageType)
		{
		case ECFDamageType::None:
			OutToken = TEXT("None");
			return true;
		case ECFDamageType::Kinetic:
			OutToken = TEXT("Kinetic");
			return true;
		case ECFDamageType::Explosive:
			OutToken = TEXT("Explosive");
			return true;
		case ECFDamageType::Energy:
			OutToken = TEXT("Energy");
			return true;
		default:
			OutToken.Reset();
			return false;
		}
	}

	// Canonical DamageType JSON token을 exact enum 값으로 strict parse합니다.
	bool TryParseDamageTypeToken(const FString& Token, ECFDamageType& OutDamageType)
	{
		if (Token.Equals(TEXT("None"), ESearchCase::CaseSensitive))
		{
			OutDamageType = ECFDamageType::None;
			return true;
		}
		if (Token.Equals(TEXT("Kinetic"), ESearchCase::CaseSensitive))
		{
			OutDamageType = ECFDamageType::Kinetic;
			return true;
		}
		if (Token.Equals(TEXT("Explosive"), ESearchCase::CaseSensitive))
		{
			OutDamageType = ECFDamageType::Explosive;
			return true;
		}
		if (Token.Equals(TEXT("Energy"), ESearchCase::CaseSensitive))
		{
			OutDamageType = ECFDamageType::Energy;
			return true;
		}
		return false;
	}

	// DamageType JSON field를 exact canonical string enum으로 strict parse합니다.
	bool ParseDamageTypeField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		ECFDamageType& OutDamageType,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// JSON source DamageType token입니다.
		FString DamageTypeToken;
		if (!CFDACommonPrimitives::ParseStringField(Object, FieldName, FieldPath, DamageTypeToken, OutIssues))
		{
			return false;
		}
		if (!TryParseDamageTypeToken(DamageTypeToken, OutDamageType))
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidEnumValue,
				FieldPath,
				TEXT("DamageType은 exact `None`, `Kinetic`, `Explosive`, `Energy` 중 하나여야 합니다."));
			return false;
		}
		return true;
	}

	// Damage exact12 typed payload가 current authored field 제약을 만족하는지 검사합니다.
	bool ValidateTypedPayload(const FCFDADamagePayload& Payload, FString& OutError)
	{
		if (Payload.DamageId.IsNone())
		{
			OutError = TEXT("DamageId는 required stable identity이며 NAME_None일 수 없습니다.");
			return false;
		}

		// Canonical DamageType token validation 결과입니다.
		FString DamageTypeToken;
		if (!TryGetDamageTypeToken(Payload.DamageType, DamageTypeToken))
		{
			OutError = TEXT("DamageType 값이 current ECFDamageType authored 범위 밖입니다.");
			return false;
		}

		if (!FMath::IsFinite(Payload.BaseDamage) || Payload.BaseDamage <= 0.0f)
		{
			OutError = TEXT("BaseDamage는 finite positive float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.ArmorPenetration) || Payload.ArmorPenetration < 0.0f)
		{
			OutError = TEXT("ArmorPenetration은 finite nonnegative float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.ExplosionRadius) || Payload.ExplosionRadius < 0.0f)
		{
			OutError = TEXT("ExplosionRadius는 finite nonnegative float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.ExplosionInnerRadius) || Payload.ExplosionInnerRadius < 0.0f)
		{
			OutError = TEXT("ExplosionInnerRadius는 finite nonnegative float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.ExplosionDamage) || Payload.ExplosionDamage < 0.0f)
		{
			OutError = TEXT("ExplosionDamage는 finite nonnegative float여야 합니다.");
			return false;
		}
		if (Payload.bUseRadialDamage
			&& (Payload.ExplosionRadius <= 0.0f || Payload.ExplosionDamage <= 0.0f))
		{
			OutError = TEXT("bUseRadialDamage=true이면 ExplosionRadius와 ExplosionDamage가 각각 finite positive float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.MinExplosionDamageScale)
			|| Payload.MinExplosionDamageScale < 0.0f
			|| Payload.MinExplosionDamageScale > 1.0f)
		{
			OutError = TEXT("MinExplosionDamageScale은 finite 0..1 float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.ModuleDamageScale) || Payload.ModuleDamageScale < 0.0f)
		{
			OutError = TEXT("ModuleDamageScale은 finite nonnegative float여야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.ImpulseStrength) || Payload.ImpulseStrength < 0.0f)
		{
			OutError = TEXT("ImpulseStrength는 finite nonnegative float여야 합니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Damage exact12 Payload object를 strict typed payload로 parse합니다.
	void ParsePayload(
		const TSharedPtr<FJsonObject>& PayloadObject,
		FCFDADamagePayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		CFDACommonPrimitives::ValidateExactFields(
			PayloadObject,
			GetPayloadFields(),
			TEXT("Payload"),
			OutIssues);

		ParseRequiredNameField(PayloadObject, TEXT("DamageId"), TEXT("Payload.DamageId"), OutPayload.DamageId, OutIssues);
		ParseDamageTypeField(PayloadObject, TEXT("DamageType"), TEXT("Payload.DamageType"), OutPayload.DamageType, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("BaseDamage"), TEXT("Payload.BaseDamage"), 0.0, TNumericLimits<float>::Max(), OutPayload.BaseDamage, OutIssues);
		CFDACommonPrimitives::ParseBoolField(PayloadObject, TEXT("bCanDamageSelf"), TEXT("Payload.bCanDamageSelf"), OutPayload.bCanDamageSelf, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ArmorPenetration"), TEXT("Payload.ArmorPenetration"), 0.0, TNumericLimits<float>::Max(), OutPayload.ArmorPenetration, OutIssues);
		CFDACommonPrimitives::ParseBoolField(PayloadObject, TEXT("bUseRadialDamage"), TEXT("Payload.bUseRadialDamage"), OutPayload.bUseRadialDamage, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ExplosionRadius"), TEXT("Payload.ExplosionRadius"), 0.0, TNumericLimits<float>::Max(), OutPayload.ExplosionRadius, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ExplosionInnerRadius"), TEXT("Payload.ExplosionInnerRadius"), 0.0, TNumericLimits<float>::Max(), OutPayload.ExplosionInnerRadius, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ExplosionDamage"), TEXT("Payload.ExplosionDamage"), 0.0, TNumericLimits<float>::Max(), OutPayload.ExplosionDamage, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("MinExplosionDamageScale"), TEXT("Payload.MinExplosionDamageScale"), 0.0, 1.0, OutPayload.MinExplosionDamageScale, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ModuleDamageScale"), TEXT("Payload.ModuleDamageScale"), 0.0, TNumericLimits<float>::Max(), OutPayload.ModuleDamageScale, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ImpulseStrength"), TEXT("Payload.ImpulseStrength"), 0.0, TNumericLimits<float>::Max(), OutPayload.ImpulseStrength, OutIssues);

		if (!CFDACommonPrimitives::HasBlockingIssue(OutIssues))
		{
			// Parsed exact12 typed payload validation 오류입니다.
			FString TypedPayloadError;
			if (!ValidateTypedPayload(OutPayload, TypedPayloadError))
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("Payload"),
					TypedPayloadError);
			}
		}
	}

	// Damage semantic fingerprint가 사용할 exact schema/revision/class + exact12 payload token stream을 append합니다.
	bool AppendPayloadTokens(TArray<uint8>& OutBytes, const FCFDADamagePayload& Payload, FString& OutError)
	{
		// Fingerprint에 사용할 canonical DamageType token입니다.
		FString DamageTypeToken;
		if (!TryGetDamageTypeToken(Payload.DamageType, DamageTypeToken))
		{
			OutError = TEXT("DamageType fingerprint token을 생성할 수 없습니다.");
			return false;
		}

		// Current trusted Damage provider descriptor입니다.
		const FCFDATypeProvider& Provider = CFDADamageProvider::GetProvider().Descriptor;
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("SchemaId"), Provider.TypeKey.SchemaId);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(Provider.SchemaRevision));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(Provider.AdapterContractRevision));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), Provider.TypeKey.DataAssetTypeClassPath);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.DamageId"), CFDACommonPrimitives::CanonicalNameText(Payload.DamageId));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.DamageType"), DamageTypeToken);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.BaseDamage"), Payload.BaseDamage);
		CFDACommonPrimitives::AppendBoolToken(OutBytes, TEXT("Payload.bCanDamageSelf"), Payload.bCanDamageSelf);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ArmorPenetration"), Payload.ArmorPenetration);
		CFDACommonPrimitives::AppendBoolToken(OutBytes, TEXT("Payload.bUseRadialDamage"), Payload.bUseRadialDamage);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ExplosionRadius"), Payload.ExplosionRadius);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ExplosionInnerRadius"), Payload.ExplosionInnerRadius);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ExplosionDamage"), Payload.ExplosionDamage);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.MinExplosionDamageScale"), Payload.MinExplosionDamageScale);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ModuleDamageScale"), Payload.ModuleDamageScale);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ImpulseStrength"), Payload.ImpulseStrength);
		OutError.Reset();
		return true;
	}
}

// Current DamageData ReviewedMutationReady trusted provider authority를 반환합니다.
const FCFDADamageTypeProvider& CFDADamageProvider::GetProvider()
{
	// CF-FQ-052 frozen Damage schema/class/revision/root/identity + reserved DACE boundary authority입니다.
	static const FCFDADamageTypeProvider DamageProvider = []()
	{
		// Current ReviewedMutationReady Damage provider entry입니다.
		FCFDADamageTypeProvider Provider;
		Provider.Descriptor.TypeKey.SchemaId = TEXT("CarFight.DataAsset.DamageData");
		Provider.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFDamageData");
		Provider.Descriptor.SchemaRevision = 1;
		Provider.Descriptor.AdapterContractRevision = 1;
		Provider.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging/DamageData");
		Provider.Descriptor.StableIdentityPolicy = ECFDAStableIdentityPolicy::Required;
		Provider.Descriptor.StableIdentityResolver = ECFDAStableIdentityResolver::ExplicitFName;
		Provider.Descriptor.StableIdentitySourceName = FName(TEXT("DamageId"));
		Provider.Descriptor.DaceContractOwnerName = FName(TEXT("CFDADamageDace"));
		Provider.Descriptor.DaceAcceptedHistoryNamespace = TEXT("DACE-DamageData");
		Provider.Descriptor.DaceReadiness = ECFDADaceReadiness::ContractReady;
		Provider.Descriptor.bDaceCanonicalStagingTargetSetDeclared = true;
		Provider.Descriptor.DaceCanonicalStagingRelativePaths.Reset();
		Provider.Readiness = ECFDAProviderReadiness::ReviewedMutationReady;
		Provider.Operations.ParseCommonCandidate = &CFDADamageProviderImpl::ParseCommonCandidate;
		Provider.Operations.ResolveCommonCurrentState = &CFDADamageProviderImpl::ResolveCommonCurrentState;
		Provider.Operations.ApplyReviewedMutation = &CFDADamageProviderImpl::ApplyReviewedMutation;
		return Provider;
	}();
	return DamageProvider;
}

// Damage typed record를 payload-free shared envelope로 투영합니다.
FCFDACommonEnvelope CFDADamageProviderImpl::BuildCommonEnvelope(
	const FCFDADamageRecord& Record,
	const FString& CurrentSemanticFingerprint,
	const ECFDAStagingPreviewKind PlannedOperation)
{
	// Shared orchestration으로 전달할 payload-free Damage envelope입니다.
	FCFDACommonEnvelope Envelope;
	Envelope.SchemaId = Record.SchemaId;
	Envelope.SchemaRevision = Record.SchemaRevision;
	Envelope.AdapterContractRevision = Record.AdapterContractRevision;
	Envelope.DataAssetTypeClassPath = Record.DataAssetTypeClassPath;
	Envelope.StableLogicalId = Record.StableLogicalId;
	Envelope.TargetObjectPath = Record.TargetObjectPath;
	Envelope.StagingRelativePath = Record.StagingRelativePath;
	Envelope.bHasBaseSemanticFingerprint = Record.bHasBaseSemanticFingerprint;
	Envelope.BaseSemanticFingerprint = Record.BaseSemanticFingerprint;
	Envelope.CurrentSemanticFingerprint = CurrentSemanticFingerprint;
	Envelope.StagingSemanticFingerprint = Record.StagingSemanticFingerprint;
	Envelope.PlannedOperation = PlannedOperation;
	return Envelope;
}

// Raw JSON을 strict Damage typed parser까지 통과시킨 뒤 common candidate만 반환합니다.
bool CFDADamageProviderImpl::ParseCommonCandidate(
	const FString& JsonText,
	const FString& StagingRelativePath,
	FCFDACommonEnvelope& OutEnvelope,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Provider-local strict Damage parse 결과입니다.
	const FCFDADamageParseResult ParseResult = ParseJson(JsonText, StagingRelativePath);
	OutIssues = ParseResult.Issues;
	if (!ParseResult.bValid)
	{
		OutEnvelope = FCFDACommonEnvelope();
		return false;
	}

	// Provider-local typed payload/cached fingerprint split-state 검증 오류입니다.
	FString IntegrityError;
	if (!ValidateRecordIntegrity(ParseResult.Record, IntegrityError))
	{
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("StagingSemanticFingerprint"),
			IntegrityError);
		OutEnvelope = FCFDACommonEnvelope();
		return false;
	}

	OutEnvelope = BuildCommonEnvelope(ParseResult.Record);
	return true;
}

// Strict whole-record Damage JSON을 provider-local typed record로 parse합니다.
FCFDADamageParseResult CFDADamageProviderImpl::ParseJson(
	const FString& JsonText,
	const FString& StagingRelativePath)
{
	// Parse/validation 결과입니다.
	FCFDADamageParseResult Result;
	// JSON text reader입니다.
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	// JSON root object입니다.
	TSharedPtr<FJsonObject> RootObject;
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		CFDACommonPrimitives::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::MalformedJson,
			TEXT("$"),
			TEXT("유효한 Damage whole-record JSON object를 parse할 수 없습니다."));
		return Result;
	}

	CFDACommonPrimitives::ValidateExactFields(
		RootObject,
		CFDADamageProviderPrivate::GetTopLevelFields(),
		FString(),
		Result.Issues);
	CFDACommonPrimitives::ParseStringField(RootObject, TEXT("SchemaId"), TEXT("SchemaId"), Result.Record.SchemaId, Result.Issues);
	CFDACommonPrimitives::ParseRevisionField(RootObject, TEXT("SchemaRevision"), TEXT("SchemaRevision"), Result.Record.SchemaRevision, Result.Issues);
	CFDACommonPrimitives::ParseRevisionField(RootObject, TEXT("AdapterContractRevision"), TEXT("AdapterContractRevision"), Result.Record.AdapterContractRevision, Result.Issues);
	CFDACommonPrimitives::ParseStringField(RootObject, TEXT("DataAssetTypeClassPath"), TEXT("DataAssetTypeClassPath"), Result.Record.DataAssetTypeClassPath, Result.Issues);

	// Top-level StableLogicalId source text입니다.
	FString StableLogicalIdText;
	if (CFDACommonPrimitives::ParseStringField(
		RootObject,
		TEXT("StableLogicalId"),
		TEXT("StableLogicalId"),
		StableLogicalIdText,
		Result.Issues))
	{
		Result.Record.StableLogicalId = FName(*StableLogicalIdText);
		if (Result.Record.StableLogicalId.IsNone())
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::StableIdentityMissing,
				TEXT("StableLogicalId"),
				TEXT("Required StableLogicalId는 NAME_None/empty일 수 없습니다."));
		}
	}

	CFDACommonPrimitives::ParseStringField(
		RootObject,
		TEXT("TargetObjectPath"),
		TEXT("TargetObjectPath"),
		Result.Record.TargetObjectPath,
		Result.Issues);
	if (!Result.Record.TargetObjectPath.IsEmpty())
	{
		CFDACommonPrimitives::ValidateTargetObjectPath(Result.Record.TargetObjectPath, Result.Issues);
	}
	CFDACommonPrimitives::ParseBaseSemanticFingerprint(
		RootObject,
		Result.Record.bHasBaseSemanticFingerprint,
		Result.Record.BaseSemanticFingerprint,
		Result.Issues);

	// JSON TypeKey가 선택한 production provider entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* SelectedProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		Result.Record.SchemaId,
		Result.Record.DataAssetTypeClassPath,
		&ProviderLookupError);
	// Damage parser가 허용하는 exact production entry입니다.
	const FCFDATypeProviderEntry& DamageProviderEntry = CFDADamageProvider::GetProvider();
	if (SelectedProviderEntry == nullptr || SelectedProviderEntry != &DamageProviderEntry)
	{
		CFDACommonPrimitives::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderLookupError.IsEmpty()
				? TEXT("Damage parser에 등록된 exact trusted TypeKey provider가 아닙니다.")
				: ProviderLookupError);
	}
	else
	{
		// Selected Damage structural descriptor입니다.
		const FCFDATypeProvider& Provider = SelectedProviderEntry->Descriptor;
		if (Result.Record.SchemaRevision != Provider.SchemaRevision)
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::SchemaRevisionUnsupported,
				TEXT("SchemaRevision"),
				TEXT("Selected Damage provider와 정확히 같은 SchemaRevision만 지원합니다."));
		}
		if (Result.Record.AdapterContractRevision != Provider.AdapterContractRevision)
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::AdapterRevisionMismatch,
				TEXT("AdapterContractRevision"),
				TEXT("Selected Damage typed adapter와 정확히 같은 AdapterContractRevision만 지원합니다."));
		}
		if (!StagingRelativePath.IsEmpty()
			&& !CFDATypeDispatch::NormalizeProviderStagingPath(
				Provider,
				StagingRelativePath,
				Result.Record.StagingRelativePath))
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::InvalidValue,
				TEXT("StagingRelativePath"),
				TEXT("Staging source path가 Damage provider canonical StagingRoot 아래 `.json` 경로가 아닙니다."));
		}
	}

	if (!CFDACommonPrimitives::HasBlockingIssue(Result.Issues))
	{
		// Strict Damage Payload JSON object입니다.
		TSharedPtr<FJsonObject> PayloadObject;
		if (CFDACommonPrimitives::ParseObjectField(
			RootObject,
			TEXT("Payload"),
			TEXT("Payload"),
			PayloadObject,
			Result.Issues))
		{
			CFDADamageProviderPrivate::ParsePayload(PayloadObject, Result.Record.Payload, Result.Issues);
		}
	}

	if (!Result.Record.StableLogicalId.IsNone()
		&& !Result.Record.Payload.DamageId.IsNone()
		&& Result.Record.StableLogicalId != Result.Record.Payload.DamageId)
	{
		CFDACommonPrimitives::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::StableIdentityMismatch,
			TEXT("StableLogicalId"),
			TEXT("StableLogicalId와 Payload.DamageId가 같은 FName semantic identity가 아닙니다."));
	}

	if (!CFDACommonPrimitives::HasBlockingIssue(Result.Issues))
	{
		// Typed desired Damage payload fingerprint 생성 오류입니다.
		FString FingerprintError;
		if (!BuildSemanticFingerprint(
			Result.Record.Payload,
			Result.Record.StagingSemanticFingerprint,
			FingerprintError))
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::InvalidValue,
				TEXT("Payload"),
				FingerprintError);
		}
	}

	Result.bValid = !CFDACommonPrimitives::HasBlockingIssue(Result.Issues);
	return Result;
}

// Damage exact12 typed payload를 current schema whole-record JSON으로 deterministic 직렬화합니다. 빈 BaseSemanticFingerprint는 JSON null을 뜻합니다.
bool CFDADamageProviderImpl::SerializeStagingJson(
	const FCFDADamagePayload& Payload,
	const FString& TargetObjectPath,
	const FString& BaseSemanticFingerprint,
	FString& OutJsonText,
	FString& OutError)
{
	OutJsonText.Reset();
	// Serializer 진입 시 typed payload 전체 semantic을 검증하기 위한 fingerprint입니다.
	FString ValidationFingerprint;
	if (!BuildSemanticFingerprint(Payload, ValidationFingerprint, OutError))
	{
		return false;
	}

	// TargetObjectPath canonical contract 검증 결과입니다.
	TArray<FCFDAStagingIssue> TargetPathIssues;
	if (!CFDACommonPrimitives::ValidateTargetObjectPath(TargetObjectPath, TargetPathIssues))
	{
		OutError = TargetPathIssues.IsEmpty()
			? TEXT("Damage serializer TargetObjectPath가 canonical object path가 아닙니다.")
			: TargetPathIssues[0].Message;
		return false;
	}
	if (!BaseSemanticFingerprint.IsEmpty()
		&& !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(BaseSemanticFingerprint))
	{
		OutError = TEXT("Damage serializer BaseSemanticFingerprint는 empty(null) 또는 canonical sha256:<64 lowercase hex>여야 합니다.");
		return false;
	}

	// Serializer에 사용할 canonical DamageType token입니다.
	FString DamageTypeToken;
	if (!CFDADamageProviderPrivate::TryGetDamageTypeToken(Payload.DamageType, DamageTypeToken))
	{
		OutError = TEXT("Damage serializer가 DamageType token을 생성할 수 없습니다.");
		return false;
	}

	// Exact Damage exact12 payload object입니다.
	TSharedRef<FJsonObject> PayloadObject = MakeShared<FJsonObject>();
	PayloadObject->SetStringField(TEXT("DamageId"), Payload.DamageId.ToString());
	PayloadObject->SetStringField(TEXT("DamageType"), DamageTypeToken);
	PayloadObject->SetNumberField(TEXT("BaseDamage"), Payload.BaseDamage);
	PayloadObject->SetBoolField(TEXT("bCanDamageSelf"), Payload.bCanDamageSelf);
	PayloadObject->SetNumberField(TEXT("ArmorPenetration"), Payload.ArmorPenetration);
	PayloadObject->SetBoolField(TEXT("bUseRadialDamage"), Payload.bUseRadialDamage);
	PayloadObject->SetNumberField(TEXT("ExplosionRadius"), Payload.ExplosionRadius);
	PayloadObject->SetNumberField(TEXT("ExplosionInnerRadius"), Payload.ExplosionInnerRadius);
	PayloadObject->SetNumberField(TEXT("ExplosionDamage"), Payload.ExplosionDamage);
	PayloadObject->SetNumberField(TEXT("MinExplosionDamageScale"), Payload.MinExplosionDamageScale);
	PayloadObject->SetNumberField(TEXT("ModuleDamageScale"), Payload.ModuleDamageScale);
	PayloadObject->SetNumberField(TEXT("ImpulseStrength"), Payload.ImpulseStrength);

	// Current trusted Damage provider descriptor입니다.
	const FCFDATypeProvider& Provider = CFDADamageProvider::GetProvider().Descriptor;
	// Strict 8-field whole-record root object입니다.
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("SchemaId"), Provider.TypeKey.SchemaId);
	RootObject->SetNumberField(TEXT("SchemaRevision"), Provider.SchemaRevision);
	RootObject->SetNumberField(TEXT("AdapterContractRevision"), Provider.AdapterContractRevision);
	RootObject->SetStringField(TEXT("DataAssetTypeClassPath"), Provider.TypeKey.DataAssetTypeClassPath);
	RootObject->SetStringField(TEXT("StableLogicalId"), Payload.DamageId.ToString());
	RootObject->SetStringField(TEXT("TargetObjectPath"), TargetObjectPath);
	if (BaseSemanticFingerprint.IsEmpty())
	{
		RootObject->SetField(TEXT("BaseSemanticFingerprint"), MakeShared<FJsonValueNull>());
	}
	else
	{
		RootObject->SetStringField(TEXT("BaseSemanticFingerprint"), BaseSemanticFingerprint);
	}
	RootObject->SetObjectField(TEXT("Payload"), PayloadObject);

	// Deterministic JSON text writer입니다.
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJsonText);
	if (!FJsonSerializer::Serialize(RootObject, JsonWriter))
	{
		OutJsonText.Reset();
		OutError = TEXT("Damage whole-record JSON serialization에 실패했습니다.");
		return false;
	}
	JsonWriter->Close();
	OutJsonText += LINE_TERMINATOR;
	OutError.Reset();
	return true;
}

// Damage exact12 typed payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
bool CFDADamageProviderImpl::BuildSemanticFingerprint(
	const FCFDADamagePayload& Payload,
	FString& OutFingerprint,
	FString& OutError)
{
	if (!CFDADamageProviderPrivate::ValidateTypedPayload(Payload, OutError))
	{
		OutFingerprint.Reset();
		return false;
	}

	// Exact schema/revision/class + Damage exact12 typed payload canonical token stream입니다.
	TArray<uint8> CanonicalBytes;
	CanonicalBytes.Reserve(1024);
	if (!CFDADamageProviderPrivate::AppendPayloadTokens(CanonicalBytes, Payload, OutError))
	{
		OutFingerprint.Reset();
		return false;
	}
	return CFDACommonPrimitives::HashCanonicalBytes(CanonicalBytes, OutFingerprint, OutError);
}

// Exact UCFDamageData UObject를 authored whole-record payload로 read-only 추출합니다.
bool CFDADamageProviderImpl::ExtractPayload(
	const UCFDamageData& DamageAsset,
	FCFDADamagePayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutPayload = FCFDADamagePayload();
	OutIssues.Reset();
	OutPayload.DamageId = DamageAsset.DamageId;
	OutPayload.DamageType = DamageAsset.DamageType;
	OutPayload.BaseDamage = DamageAsset.BaseDamage;
	OutPayload.bCanDamageSelf = DamageAsset.bCanDamageSelf;
	OutPayload.ArmorPenetration = DamageAsset.ArmorPenetration;
	OutPayload.bUseRadialDamage = DamageAsset.bUseRadialDamage;
	OutPayload.ExplosionRadius = DamageAsset.ExplosionRadius;
	OutPayload.ExplosionInnerRadius = DamageAsset.ExplosionInnerRadius;
	OutPayload.ExplosionDamage = DamageAsset.ExplosionDamage;
	OutPayload.MinExplosionDamageScale = DamageAsset.MinExplosionDamageScale;
	OutPayload.ModuleDamageScale = DamageAsset.ModuleDamageScale;
	OutPayload.ImpulseStrength = DamageAsset.ImpulseStrength;

	if (OutPayload.DamageId.IsNone())
	{
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::StableIdentityMissing,
			TEXT("Current.Payload.DamageId"),
			TEXT("current CFDamageData의 required DamageId가 NAME_None입니다."));
	}
	if (!CFDACommonPrimitives::HasBlockingIssue(OutIssues))
	{
		// Current raw authored payload validation 오류입니다.
		FString TypedPayloadError;
		if (!CFDADamageProviderPrivate::ValidateTypedPayload(OutPayload, TypedPayloadError))
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				TEXT("Current.Payload"),
				TypedPayloadError);
		}
	}
	return !CFDACommonPrimitives::HasBlockingIssue(OutIssues);
}

// Mutable provider-local record의 payload와 cached fingerprint가 exact 일치하는지 검증합니다.
bool CFDADamageProviderImpl::ValidateRecordIntegrity(
	const FCFDADamageRecord& Record,
	FString& OutError)
{
	// Mutable typed payload에서 fresh 재계산한 semantic fingerprint입니다.
	FString RecomputedFingerprint;
	// Fresh fingerprint 생성 오류입니다.
	FString FingerprintError;
	if (!BuildSemanticFingerprint(Record.Payload, RecomputedFingerprint, FingerprintError))
	{
		OutError = FingerprintError;
		return false;
	}
	return CFDACommonPrimitives::ValidateCachedSemanticFingerprint(
		Record.StagingSemanticFingerprint,
		RecomputedFingerprint,
		OutError);
}

// Damage exact12 typed payload를 exact UCFDamageData UObject에 deterministic whole-record로 materialize합니다.
void CFDADamageProviderImpl::MaterializePayload(
	UCFDamageData& TargetAsset,
	const FCFDADamagePayload& Payload)
{
	TargetAsset.DamageId = Payload.DamageId;
	TargetAsset.DamageType = Payload.DamageType;
	TargetAsset.BaseDamage = Payload.BaseDamage;
	TargetAsset.bCanDamageSelf = Payload.bCanDamageSelf;
	TargetAsset.ArmorPenetration = Payload.ArmorPenetration;
	TargetAsset.bUseRadialDamage = Payload.bUseRadialDamage;
	TargetAsset.ExplosionRadius = Payload.ExplosionRadius;
	TargetAsset.ExplosionInnerRadius = Payload.ExplosionInnerRadius;
	TargetAsset.ExplosionDamage = Payload.ExplosionDamage;
	TargetAsset.MinExplosionDamageScale = Payload.MinExplosionDamageScale;
	TargetAsset.ModuleDamageScale = Payload.ModuleDamageScale;
	TargetAsset.ImpulseStrength = Payload.ImpulseStrength;
}

// Fresh reviewed JSON을 재parse/rebind한 뒤 shared durable core로 exact Damage Create/Update를 실행합니다.
void CFDADamageProviderImpl::ApplyReviewedMutation(
	const FString& JsonText,
	const FCFDACommonPreviewRow& FreshRow,
	FCFDAStagingTargetApplyReport& OutTargetReport)
{
	// Mutation 직전 fresh source를 provider-local typed record로 다시 parse한 결과입니다.
	const FCFDADamageParseResult ParseResult = ParseJson(JsonText, FreshRow.Envelope.StagingRelativePath);
	if (!ParseResult.bValid)
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		OutTargetReport.Issues = ParseResult.Issues;
		OutTargetReport.Diagnostic = TEXT("Damage provider mutation callback에서 fresh typed Staging 재parse가 실패했습니다.");
		return;
	}

	// Fresh typed record의 cached semantic fingerprint integrity 실패 상세입니다.
	FString IntegrityError;
	if (!ValidateRecordIntegrity(ParseResult.Record, IntegrityError))
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		CFDACommonPrimitives::AddIssue(
			OutTargetReport.Issues,
			ECFDAStagingIssueCode::ApprovalStale,
			TEXT("StagingSemanticFingerprint"),
			IntegrityError);
		OutTargetReport.Diagnostic = IntegrityError;
		return;
	}

	// Provider-local typed record를 shared durable row와 대조할 payload-free projection입니다.
	const FCFDACommonEnvelope ParsedEnvelope = BuildCommonEnvelope(
		ParseResult.Record,
		FreshRow.Envelope.CurrentSemanticFingerprint,
		FreshRow.Kind);
	if (!CFDATypeDispatch::AreCommonEnvelopesEquivalent(ParsedEnvelope, FreshRow.Envelope))
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		CFDACommonPrimitives::AddIssue(
			OutTargetReport.Issues,
			ECFDAStagingIssueCode::ApprovalStale,
			TEXT("Approval"),
			TEXT("Damage typed mutation record가 immediate fresh common evidence와 다릅니다."));
		OutTargetReport.Diagnostic = TEXT("Damage typed mutation record/common evidence mismatch입니다.");
		return;
	}

	CFDADurableCore::ApplyTypedTarget<UCFDamageData, FCFDADamagePayload>(
		FreshRow,
		ParseResult.Record.Payload,
		&CFDADamageProviderImpl::MaterializePayload,
		&CFDADamageProviderImpl::ExtractPayload,
		&CFDADamageProviderImpl::BuildSemanticFingerprint,
		OutTargetReport);
}

// Payload-free Damage candidate를 exact UCFDamageData current truth로 read-only 해석합니다.
bool CFDADamageProviderImpl::ResolveCommonCurrentState(
	const FCFDACommonEnvelope& Envelope,
	FCFDACommonCurrentState& OutCurrentState,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutCurrentState = FCFDACommonCurrentState();
	OutIssues.Reset();

	// Envelope TypeKey에 등록된 exact production entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		Envelope.SchemaId,
		Envelope.DataAssetTypeClassPath,
		&ProviderLookupError);
	// Damage current resolver가 허용하는 exact production provider entry입니다.
	const FCFDATypeProviderEntry& DamageProviderEntry = CFDADamageProvider::GetProvider();
	// Provider contract validation 오류입니다.
	FString ProviderContractError;
	if (ProviderEntry == nullptr
		|| ProviderEntry != &DamageProviderEntry
		|| !CFDATypeDispatch::ValidateProviderContract(
			Envelope,
			ProviderEntry->Descriptor,
			ProviderContractError))
	{
		// Fail-closed provider diagnostic입니다.
		const FString ProviderDiagnostic = ProviderEntry == nullptr
			? ProviderLookupError
			: (ProviderEntry != &DamageProviderEntry
				? TEXT("Damage current-state resolver에 등록된 typed provider가 아닙니다.")
				: ProviderContractError);
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderDiagnostic);
		return false;
	}

	// Current resolver가 사용할 selected structural descriptor입니다.
	const FCFDATypeProvider& Provider = ProviderEntry->Descriptor;
	if (!CFDACommonPrimitives::ValidateTargetObjectPath(Envelope.TargetObjectPath, OutIssues))
	{
		return false;
	}

	// CF-FQ-045 current DataAsset descriptor authority입니다.
	FCFDATypeRegistry TypeRegistry;
	// Descriptor registration 실패 원인입니다.
	FString RegistryError;
	if (!TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistryError))
	{
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry"),
			FString::Printf(TEXT("CF-FQ-045 current descriptor 등록에 실패했습니다: %s"), *RegistryError));
		return false;
	}

	// CFDamageData public semantic descriptor입니다.
	const FCFDASemanticDescriptor* Descriptor = TypeRegistry.FindDescriptor(Envelope.DataAssetTypeClassPath);
	if (Descriptor == nullptr
		|| Descriptor->IdentityPolicy != ECFDAIdentityPolicy::Required
		|| Descriptor->IdentityResolverKind != ECFDAIdentityResolverKind::ExplicitFName
		|| Descriptor->IdentitySourceName != Provider.StableIdentitySourceName.ToString())
	{
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry.Identity"),
			TEXT("CFDamageData Registry identity 계약이 Required/ExplicitFName/DamageId와 다릅니다."));
		return false;
	}

	// Read-only exact target/identity 조회에 사용할 Project Asset Registry입니다.
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	// Exact requested target path입니다.
	const FSoftObjectPath TargetObjectPath(Envelope.TargetObjectPath);
	// 이미 메모리에 존재하는 exact target UObject입니다.
	UObject* ResolvedTargetObject = TargetObjectPath.ResolveObject();
	// Persisted/registered exact target metadata입니다.
	const FAssetData TargetAssetData = AssetRegistry.GetAssetByObjectPath(TargetObjectPath, false, false);
	OutCurrentState.bRequestedTargetExists = ResolvedTargetObject != nullptr || TargetAssetData.IsValid();

	if (OutCurrentState.bRequestedTargetExists)
	{
		OutCurrentState.RequestedTargetClassPath = ResolvedTargetObject != nullptr
			? ResolvedTargetObject->GetClass()->GetClassPathName().ToString()
			: TargetAssetData.AssetClassPath.ToString();

		if (OutCurrentState.RequestedTargetClassPath.Equals(
			Provider.TypeKey.DataAssetTypeClassPath,
			ESearchCase::CaseSensitive))
		{
			// Exact expected target UObject semantic readback입니다.
			UObject* LoadedTargetObject = ResolvedTargetObject != nullptr
				? ResolvedTargetObject
				: TargetAssetData.GetAsset();
			// Exact UCFDamageData current target입니다.
			const UCFDamageData* CurrentDamage = Cast<UCFDamageData>(LoadedTargetObject);
			if (CurrentDamage == nullptr)
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("Current.TargetObjectPath"),
					TEXT("Asset Registry에는 expected CFDamageData target이 있지만 UObject semantic readback에 실패했습니다."));
				return false;
			}

			OutCurrentState.RequestedTargetStableLogicalId = CurrentDamage->DamageId;
			// Current exact target package입니다.
			const UPackage* CurrentPackage = CurrentDamage->GetOutermost();
			OutCurrentState.bRequestedTargetDirty = CurrentPackage != nullptr && CurrentPackage->IsDirty();

			// Current persisted/loaded Damage exact12 payload입니다.
			FCFDADamagePayload CurrentPayload;
			if (!ExtractPayload(*CurrentDamage, CurrentPayload, OutIssues))
			{
				return false;
			}

			// Current semantic fingerprint 생성 오류입니다.
			FString CurrentFingerprintError;
			if (!BuildSemanticFingerprint(
				CurrentPayload,
				OutCurrentState.CurrentSemanticFingerprint,
				CurrentFingerprintError))
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("CurrentSemanticFingerprint"),
					CurrentFingerprintError);
				return false;
			}
		}
	}

	// Asset Registry와 loaded-but-unregistered UObject를 합친 exact class-scoped DamageId path 집합입니다.
	TSet<FString> StableIdentityObjectPaths;
	for (TObjectIterator<UCFDamageData> LoadedDamageIterator; LoadedDamageIterator; ++LoadedDamageIterator)
	{
		// 현재 process에 살아 있는 exact UCFDamageData 후보입니다.
		const UCFDamageData* LoadedDamage = *LoadedDamageIterator;
		if (LoadedDamage == nullptr
			|| LoadedDamage->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
			|| LoadedDamage->DamageId != Envelope.StableLogicalId)
		{
			continue;
		}

		// Loaded Damage UObject exact object identity입니다.
		const FString LoadedObjectPath = FSoftObjectPath(LoadedDamage).ToString();
		if (!LoadedObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive))
		{
			continue;
		}
		StableIdentityObjectPaths.Add(LoadedObjectPath);
	}

	if (AssetRegistry.IsLoadingAssets())
	{
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry.IdentityScan"),
			TEXT("Asset Registry가 아직 loading 중이라 Damage StableIdentity current truth를 확정할 수 없습니다."));
		return false;
	}

	// Current Project exact UCFDamageData metadata filter입니다.
	FARFilter IdentityFilter;
	IdentityFilter.ClassPaths.Add(UCFDamageData::StaticClass()->GetClassPathName());
	IdentityFilter.bRecursiveClasses = false;
	// Current exact UCFDamageData metadata 목록입니다.
	TArray<FAssetData> DamageAssets;
	(void)AssetRegistry.GetAssets(IdentityFilter, DamageAssets, false);
	for (const FAssetData& DamageAssetData : DamageAssets)
	{
		// DamageId semantic 비교를 위해 read-only load한 exact DataAsset입니다.
		const UCFDamageData* CandidateDamage = Cast<UCFDamageData>(DamageAssetData.GetAsset());
		if (CandidateDamage == nullptr || CandidateDamage->DamageId != Envelope.StableLogicalId)
		{
			continue;
		}
		StableIdentityObjectPaths.Add(DamageAssetData.GetSoftObjectPath().ToString());
	}

	OutCurrentState.StableIdentityMatchCount = StableIdentityObjectPaths.Num();
	OutCurrentState.bStableIdentityExists = OutCurrentState.StableIdentityMatchCount > 0;
	if (OutCurrentState.StableIdentityMatchCount == 1)
	{
		for (const FString& IdentityObjectPath : StableIdentityObjectPaths)
		{
			OutCurrentState.StableIdentityObjectPath = IdentityObjectPath;
			break;
		}
	}
	else
	{
		OutCurrentState.StableIdentityObjectPath.Reset();
	}
	return !CFDACommonPrimitives::HasBlockingIssue(OutIssues);
}
