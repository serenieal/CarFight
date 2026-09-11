// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseProvider.cpp
// Version: v1.1.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-01 VehicleDefenseData reviewed durable authoring provider 구현입니다.
// Changelog:
// - v1.1.0: VDR-P0-02 independent VehicleDefense DACE bootstrap acceptance를 반영해 DaceReadiness를 ContractReady로 승격했습니다. authoring behavior는 변경하지 않습니다.
// - v1.0.0: exact17 top-level + nested armor exact12 strict parser/serializer/fingerprint, raw-value preserving extractor/materializer, current resolver와 shared durable callback을 추가했습니다.
// Migration:
// - VehicleDefense runtime getter의 clamp/disabled projection을 authoring source로 사용하지 않습니다. valid authored raw values를 whole-record로 보존합니다.
// - v1.1.0에서 DACE readiness는 ContractReady로 승격됐고 canonical VehicleDefense target set은 explicit exact0을 유지합니다. mixed operational admission은 CFDAStagingOps explicit authority가 별도 소유합니다.

#include "CFDAVehicleDefenseProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDADurableCore.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFVehicleDefenseData.h"
#include "DataManagement/CFDATypeRegistry.h"
#include "Dom/JsonObject.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"

namespace CFDAVehicleDefenseProviderPrivate
{
	// VehicleDefense whole-record top-level exact field set입니다.
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

	// VehicleDefense exact17 top-level payload JSON field set입니다.
	const TArray<FString>& GetPayloadFields()
	{
		// Process lifetime 동안 재사용할 exact payload field 목록입니다.
		static const TArray<FString> PayloadFields =
		{
			TEXT("DefenseId"),
			TEXT("DefenseMassKg"),
			TEXT("bUseShield"),
			TEXT("MaximumShield"),
			TEXT("ShieldRegenerationDelaySeconds"),
			TEXT("ShieldRegenerationPerSecond"),
			TEXT("ArmorType"),
			TEXT("ArmorResistance"),
			TEXT("FrontArmorConfig"),
			TEXT("LeftArmorConfig"),
			TEXT("RightArmorConfig"),
			TEXT("RearArmorConfig"),
			TEXT("TopArmorConfig"),
			TEXT("BottomArmorConfig"),
			TEXT("ShieldComponentDamageScale"),
			TEXT("ArmorComponentDamageScale"),
			TEXT("IntegrityComponentDamageScale")
		};
		return PayloadFields;
	}

	// 한 directional armor object의 exact2 child field set입니다.
	const TArray<FString>& GetArmorConfigFields()
	{
		// Process lifetime 동안 재사용할 exact nested field 목록입니다.
		static const TArray<FString> ArmorConfigFields =
		{
			TEXT("MaximumArmor"),
			TEXT("DamageMultiplier")
		};
		return ArmorConfigFields;
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
				TEXT("required VehicleDefense FName identity는 NAME_None/empty일 수 없습니다."));
			return false;
		}
		return true;
	}

	// ArmorType enum 값을 canonical JSON/fingerprint token으로 변환합니다.
	bool TryGetArmorTypeToken(const ECFArmorType ArmorType, FString& OutToken)
	{
		switch (ArmorType)
		{
		case ECFArmorType::Light:
			OutToken = TEXT("Light");
			return true;
		case ECFArmorType::Standard:
			OutToken = TEXT("Standard");
			return true;
		case ECFArmorType::Heavy:
			OutToken = TEXT("Heavy");
			return true;
		default:
			OutToken.Reset();
			return false;
		}
	}

	// Canonical ArmorType JSON token을 exact enum 값으로 strict parse합니다.
	bool TryParseArmorTypeToken(const FString& Token, ECFArmorType& OutArmorType)
	{
		if (Token.Equals(TEXT("Light"), ESearchCase::CaseSensitive))
		{
			OutArmorType = ECFArmorType::Light;
			return true;
		}
		if (Token.Equals(TEXT("Standard"), ESearchCase::CaseSensitive))
		{
			OutArmorType = ECFArmorType::Standard;
			return true;
		}
		if (Token.Equals(TEXT("Heavy"), ESearchCase::CaseSensitive))
		{
			OutArmorType = ECFArmorType::Heavy;
			return true;
		}
		return false;
	}

	// ArmorType JSON field를 exact canonical string enum으로 strict parse합니다.
	bool ParseArmorTypeField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		ECFArmorType& OutArmorType,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// JSON source ArmorType token입니다.
		FString ArmorTypeToken;
		if (!CFDACommonPrimitives::ParseStringField(Object, FieldName, FieldPath, ArmorTypeToken, OutIssues))
		{
			return false;
		}
		if (!TryParseArmorTypeToken(ArmorTypeToken, OutArmorType))
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidEnumValue,
				FieldPath,
				TEXT("ArmorType은 exact `Light`, `Standard`, `Heavy` 중 하나여야 합니다."));
			return false;
		}
		return true;
	}

	// 한 float가 finite nonnegative authoring contract를 만족하는지 검사합니다.
	bool ValidateNonnegativeFloat(const TCHAR* FieldName, const float Value, FString& OutError)
	{
		if (!FMath::IsFinite(Value) || Value < 0.0f)
		{
			OutError = FString::Printf(TEXT("%s는 finite nonnegative float여야 합니다."), FieldName);
			return false;
		}
		return true;
	}

	// 한 directional armor config의 exact2 numeric semantic을 검사합니다.
	bool ValidateArmorConfig(
		const TCHAR* FieldName,
		const FCFDirectionalArmorConfig& ArmorConfig,
		FString& OutError)
	{
		if (!ValidateNonnegativeFloat(*FString::Printf(TEXT("%s.MaximumArmor"), FieldName), ArmorConfig.MaximumArmor, OutError))
		{
			return false;
		}
		if (!ValidateNonnegativeFloat(*FString::Printf(TEXT("%s.DamageMultiplier"), FieldName), ArmorConfig.DamageMultiplier, OutError))
		{
			return false;
		}
		return true;
	}

	// VehicleDefense authored semantic leaf exact23이 frozen authoring contract를 만족하는지 검사합니다.
	bool ValidateTypedPayload(const FCFDAVehicleDefensePayload& Payload, FString& OutError)
	{
		if (Payload.DefenseId.IsNone())
		{
			OutError = TEXT("DefenseId는 required stable identity이며 NAME_None일 수 없습니다.");
			return false;
		}

		// Canonical ArmorType token validation 결과입니다.
		FString ArmorTypeToken;
		if (!TryGetArmorTypeToken(Payload.ArmorType, ArmorTypeToken))
		{
			OutError = TEXT("ArmorType 값이 current ECFArmorType authored 범위 밖입니다.");
			return false;
		}

		if (!ValidateNonnegativeFloat(TEXT("DefenseMassKg"), Payload.DefenseMassKg, OutError)
			|| !ValidateNonnegativeFloat(TEXT("MaximumShield"), Payload.MaximumShield, OutError)
			|| !ValidateNonnegativeFloat(TEXT("ShieldRegenerationDelaySeconds"), Payload.ShieldRegenerationDelaySeconds, OutError)
			|| !ValidateNonnegativeFloat(TEXT("ShieldRegenerationPerSecond"), Payload.ShieldRegenerationPerSecond, OutError)
			|| !ValidateNonnegativeFloat(TEXT("ArmorResistance"), Payload.ArmorResistance, OutError)
			|| !ValidateArmorConfig(TEXT("FrontArmorConfig"), Payload.FrontArmorConfig, OutError)
			|| !ValidateArmorConfig(TEXT("LeftArmorConfig"), Payload.LeftArmorConfig, OutError)
			|| !ValidateArmorConfig(TEXT("RightArmorConfig"), Payload.RightArmorConfig, OutError)
			|| !ValidateArmorConfig(TEXT("RearArmorConfig"), Payload.RearArmorConfig, OutError)
			|| !ValidateArmorConfig(TEXT("TopArmorConfig"), Payload.TopArmorConfig, OutError)
			|| !ValidateArmorConfig(TEXT("BottomArmorConfig"), Payload.BottomArmorConfig, OutError)
			|| !ValidateNonnegativeFloat(TEXT("ShieldComponentDamageScale"), Payload.ShieldComponentDamageScale, OutError)
			|| !ValidateNonnegativeFloat(TEXT("ArmorComponentDamageScale"), Payload.ArmorComponentDamageScale, OutError)
			|| !ValidateNonnegativeFloat(TEXT("IntegrityComponentDamageScale"), Payload.IntegrityComponentDamageScale, OutError))
		{
			return false;
		}

		if (Payload.bUseShield
			&& Payload.MaximumShield <= 0.0f
			&& Payload.ShieldRegenerationPerSecond > 0.0f)
		{
			OutError = TEXT("bUseShield=true이고 MaximumShield<=0이면 ShieldRegenerationPerSecond는 0이어야 합니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// 한 nested directional armor JSON object를 exact2 typed config로 parse합니다.
	void ParseArmorConfigObject(
		const TSharedPtr<FJsonObject>& ArmorObject,
		const FString& FieldPath,
		FCFDirectionalArmorConfig& OutArmorConfig,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		CFDACommonPrimitives::ValidateExactFields(ArmorObject, GetArmorConfigFields(), FieldPath, OutIssues);
		CFDACommonPrimitives::ParseFloatField(
			ArmorObject,
			TEXT("MaximumArmor"),
			FieldPath + TEXT(".MaximumArmor"),
			0.0,
			TNumericLimits<float>::Max(),
			OutArmorConfig.MaximumArmor,
			OutIssues);
		CFDACommonPrimitives::ParseFloatField(
			ArmorObject,
			TEXT("DamageMultiplier"),
			FieldPath + TEXT(".DamageMultiplier"),
			0.0,
			TNumericLimits<float>::Max(),
			OutArmorConfig.DamageMultiplier,
			OutIssues);
	}

	// Payload의 한 named directional armor object를 strict parse합니다.
	void ParseArmorConfigField(
		const TSharedPtr<FJsonObject>& PayloadObject,
		const FString& FieldName,
		FCFDirectionalArmorConfig& OutArmorConfig,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// Strict nested armor object입니다.
		TSharedPtr<FJsonObject> ArmorObject;
		const FString FieldPath = TEXT("Payload.") + FieldName;
		if (CFDACommonPrimitives::ParseObjectField(PayloadObject, FieldName, FieldPath, ArmorObject, OutIssues))
		{
			ParseArmorConfigObject(ArmorObject, FieldPath, OutArmorConfig, OutIssues);
		}
	}

	// VehicleDefense exact17 + nested exact12 Payload object를 strict typed payload로 parse합니다.
	void ParsePayload(
		const TSharedPtr<FJsonObject>& PayloadObject,
		FCFDAVehicleDefensePayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		CFDACommonPrimitives::ValidateExactFields(PayloadObject, GetPayloadFields(), TEXT("Payload"), OutIssues);
		ParseRequiredNameField(PayloadObject, TEXT("DefenseId"), TEXT("Payload.DefenseId"), OutPayload.DefenseId, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("DefenseMassKg"), TEXT("Payload.DefenseMassKg"), 0.0, TNumericLimits<float>::Max(), OutPayload.DefenseMassKg, OutIssues);
		CFDACommonPrimitives::ParseBoolField(PayloadObject, TEXT("bUseShield"), TEXT("Payload.bUseShield"), OutPayload.bUseShield, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("MaximumShield"), TEXT("Payload.MaximumShield"), 0.0, TNumericLimits<float>::Max(), OutPayload.MaximumShield, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ShieldRegenerationDelaySeconds"), TEXT("Payload.ShieldRegenerationDelaySeconds"), 0.0, TNumericLimits<float>::Max(), OutPayload.ShieldRegenerationDelaySeconds, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ShieldRegenerationPerSecond"), TEXT("Payload.ShieldRegenerationPerSecond"), 0.0, TNumericLimits<float>::Max(), OutPayload.ShieldRegenerationPerSecond, OutIssues);
		ParseArmorTypeField(PayloadObject, TEXT("ArmorType"), TEXT("Payload.ArmorType"), OutPayload.ArmorType, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ArmorResistance"), TEXT("Payload.ArmorResistance"), 0.0, TNumericLimits<float>::Max(), OutPayload.ArmorResistance, OutIssues);
		ParseArmorConfigField(PayloadObject, TEXT("FrontArmorConfig"), OutPayload.FrontArmorConfig, OutIssues);
		ParseArmorConfigField(PayloadObject, TEXT("LeftArmorConfig"), OutPayload.LeftArmorConfig, OutIssues);
		ParseArmorConfigField(PayloadObject, TEXT("RightArmorConfig"), OutPayload.RightArmorConfig, OutIssues);
		ParseArmorConfigField(PayloadObject, TEXT("RearArmorConfig"), OutPayload.RearArmorConfig, OutIssues);
		ParseArmorConfigField(PayloadObject, TEXT("TopArmorConfig"), OutPayload.TopArmorConfig, OutIssues);
		ParseArmorConfigField(PayloadObject, TEXT("BottomArmorConfig"), OutPayload.BottomArmorConfig, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ShieldComponentDamageScale"), TEXT("Payload.ShieldComponentDamageScale"), 0.0, TNumericLimits<float>::Max(), OutPayload.ShieldComponentDamageScale, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("ArmorComponentDamageScale"), TEXT("Payload.ArmorComponentDamageScale"), 0.0, TNumericLimits<float>::Max(), OutPayload.ArmorComponentDamageScale, OutIssues);
		CFDACommonPrimitives::ParseFloatField(PayloadObject, TEXT("IntegrityComponentDamageScale"), TEXT("Payload.IntegrityComponentDamageScale"), 0.0, TNumericLimits<float>::Max(), OutPayload.IntegrityComponentDamageScale, OutIssues);

		if (!CFDACommonPrimitives::HasBlockingIssue(OutIssues))
		{
			// Parsed typed payload validation 오류입니다.
			FString TypedPayloadError;
			if (!ValidateTypedPayload(OutPayload, TypedPayloadError))
			{
				CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("Payload"), TypedPayloadError);
			}
		}
	}

	// 한 directional armor config의 exact2 fingerprint token을 append합니다.
	void AppendArmorTokens(
		TArray<uint8>& OutBytes,
		const TCHAR* FieldName,
		const FCFDirectionalArmorConfig& ArmorConfig)
	{
		// MaximumArmor token label입니다.
		const FString MaximumArmorLabel = FString::Printf(TEXT("Payload.%s.MaximumArmor"), FieldName);
		// DamageMultiplier token label입니다.
		const FString DamageMultiplierLabel = FString::Printf(TEXT("Payload.%s.DamageMultiplier"), FieldName);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, *MaximumArmorLabel, ArmorConfig.MaximumArmor);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, *DamageMultiplierLabel, ArmorConfig.DamageMultiplier);
	}

	// VehicleDefense semantic fingerprint의 metadata exact4 + payload semantic leaf exact23 token stream을 append합니다.
	bool AppendPayloadTokens(TArray<uint8>& OutBytes, const FCFDAVehicleDefensePayload& Payload, FString& OutError)
	{
		// Fingerprint에 사용할 canonical ArmorType token입니다.
		FString ArmorTypeToken;
		if (!TryGetArmorTypeToken(Payload.ArmorType, ArmorTypeToken))
		{
			OutError = TEXT("ArmorType fingerprint token을 생성할 수 없습니다.");
			return false;
		}

		// Current trusted VehicleDefense provider descriptor입니다.
		const FCFDATypeProvider& Provider = CFDAVehicleDefenseProvider::GetProvider().Descriptor;
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("SchemaId"), Provider.TypeKey.SchemaId);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(Provider.SchemaRevision));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(Provider.AdapterContractRevision));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), Provider.TypeKey.DataAssetTypeClassPath);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.DefenseId"), CFDACommonPrimitives::CanonicalNameText(Payload.DefenseId));
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.DefenseMassKg"), Payload.DefenseMassKg);
		CFDACommonPrimitives::AppendBoolToken(OutBytes, TEXT("Payload.bUseShield"), Payload.bUseShield);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.MaximumShield"), Payload.MaximumShield);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ShieldRegenerationDelaySeconds"), Payload.ShieldRegenerationDelaySeconds);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ShieldRegenerationPerSecond"), Payload.ShieldRegenerationPerSecond);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.ArmorType"), ArmorTypeToken);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ArmorResistance"), Payload.ArmorResistance);
		AppendArmorTokens(OutBytes, TEXT("FrontArmorConfig"), Payload.FrontArmorConfig);
		AppendArmorTokens(OutBytes, TEXT("LeftArmorConfig"), Payload.LeftArmorConfig);
		AppendArmorTokens(OutBytes, TEXT("RightArmorConfig"), Payload.RightArmorConfig);
		AppendArmorTokens(OutBytes, TEXT("RearArmorConfig"), Payload.RearArmorConfig);
		AppendArmorTokens(OutBytes, TEXT("TopArmorConfig"), Payload.TopArmorConfig);
		AppendArmorTokens(OutBytes, TEXT("BottomArmorConfig"), Payload.BottomArmorConfig);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ShieldComponentDamageScale"), Payload.ShieldComponentDamageScale);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.ArmorComponentDamageScale"), Payload.ArmorComponentDamageScale);
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.IntegrityComponentDamageScale"), Payload.IntegrityComponentDamageScale);
		OutError.Reset();
		return true;
	}

	// 한 directional armor config를 exact2 JSON object로 직렬화합니다.
	TSharedRef<FJsonObject> BuildArmorConfigObject(const FCFDirectionalArmorConfig& ArmorConfig)
	{
		// Serializer가 반환할 nested armor JSON object입니다.
		TSharedRef<FJsonObject> ArmorObject = MakeShared<FJsonObject>();
		ArmorObject->SetNumberField(TEXT("MaximumArmor"), ArmorConfig.MaximumArmor);
		ArmorObject->SetNumberField(TEXT("DamageMultiplier"), ArmorConfig.DamageMultiplier);
		return ArmorObject;
	}
}

// Current VehicleDefenseData ReviewedMutationReady trusted provider authority를 반환합니다.
const FCFDAVehicleDefenseProvider& CFDAVehicleDefenseProvider::GetProvider()
{
	// CF-FQ-053 frozen schema/class/revision/root/identity + reserved DACE boundary authority입니다.
	static const FCFDAVehicleDefenseProvider VehicleDefenseProvider = []()
	{
		// Current Gate 2 mutation-ready VehicleDefense provider entry입니다.
		FCFDAVehicleDefenseProvider Provider;
		Provider.Descriptor.TypeKey.SchemaId = TEXT("CarFight.DataAsset.VehicleDefenseData");
		Provider.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFVehicleDefenseData");
		Provider.Descriptor.SchemaRevision = 1;
		Provider.Descriptor.AdapterContractRevision = 1;
		Provider.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging/VehicleDefenseData");
		Provider.Descriptor.StableIdentityPolicy = ECFDAStableIdentityPolicy::Required;
		Provider.Descriptor.StableIdentityResolver = ECFDAStableIdentityResolver::ExplicitFName;
		Provider.Descriptor.StableIdentitySourceName = FName(TEXT("DefenseId"));
		Provider.Descriptor.DaceContractOwnerName = FName(TEXT("CFDAVehicleDefenseDace"));
		Provider.Descriptor.DaceAcceptedHistoryNamespace = TEXT("DACE-VehicleDefenseData");
		Provider.Descriptor.DaceReadiness = ECFDADaceReadiness::ContractReady;
		Provider.Descriptor.bDaceCanonicalStagingTargetSetDeclared = true;
		Provider.Descriptor.DaceCanonicalStagingRelativePaths.Reset();
		Provider.Readiness = ECFDAProviderReadiness::ReviewedMutationReady;
		Provider.Operations.ParseCommonCandidate = &CFDAVehicleDefenseProviderImpl::ParseCommonCandidate;
		Provider.Operations.ResolveCommonCurrentState = &CFDAVehicleDefenseProviderImpl::ResolveCommonCurrentState;
		Provider.Operations.ApplyReviewedMutation = &CFDAVehicleDefenseProviderImpl::ApplyReviewedMutation;
		return Provider;
	}();
	return VehicleDefenseProvider;
}

// VehicleDefense typed record를 payload-free shared envelope로 투영합니다.
FCFDACommonEnvelope CFDAVehicleDefenseProviderImpl::BuildCommonEnvelope(
	const FCFDAVehicleDefenseRecord& Record,
	const FString& CurrentSemanticFingerprint,
	const ECFDAStagingPreviewKind PlannedOperation)
{
	// Shared orchestration으로 전달할 payload-free VehicleDefense envelope입니다.
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

// Raw JSON을 strict VehicleDefense typed parser까지 통과시킨 뒤 common candidate만 반환합니다.
bool CFDAVehicleDefenseProviderImpl::ParseCommonCandidate(
	const FString& JsonText,
	const FString& StagingRelativePath,
	FCFDACommonEnvelope& OutEnvelope,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Provider-local strict VehicleDefense parse 결과입니다.
	const FCFDAVehicleDefenseParseResult ParseResult = ParseJson(JsonText, StagingRelativePath);
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
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingSemanticFingerprint"), IntegrityError);
		OutEnvelope = FCFDACommonEnvelope();
		return false;
	}
	OutEnvelope = BuildCommonEnvelope(ParseResult.Record);
	return true;
}

// Strict whole-record VehicleDefense JSON을 provider-local typed record로 parse합니다.
FCFDAVehicleDefenseParseResult CFDAVehicleDefenseProviderImpl::ParseJson(
	const FString& JsonText,
	const FString& StagingRelativePath)
{
	// Parse/validation 결과입니다.
	FCFDAVehicleDefenseParseResult Result;
	// JSON text reader입니다.
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	// JSON root object입니다.
	TSharedPtr<FJsonObject> RootObject;
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::MalformedJson, TEXT("$"), TEXT("유효한 VehicleDefense whole-record JSON object를 parse할 수 없습니다."));
		return Result;
	}

	CFDACommonPrimitives::ValidateExactFields(RootObject, CFDAVehicleDefenseProviderPrivate::GetTopLevelFields(), FString(), Result.Issues);
	CFDACommonPrimitives::ParseStringField(RootObject, TEXT("SchemaId"), TEXT("SchemaId"), Result.Record.SchemaId, Result.Issues);
	CFDACommonPrimitives::ParseRevisionField(RootObject, TEXT("SchemaRevision"), TEXT("SchemaRevision"), Result.Record.SchemaRevision, Result.Issues);
	CFDACommonPrimitives::ParseRevisionField(RootObject, TEXT("AdapterContractRevision"), TEXT("AdapterContractRevision"), Result.Record.AdapterContractRevision, Result.Issues);
	CFDACommonPrimitives::ParseStringField(RootObject, TEXT("DataAssetTypeClassPath"), TEXT("DataAssetTypeClassPath"), Result.Record.DataAssetTypeClassPath, Result.Issues);

	// Top-level StableLogicalId source text입니다.
	FString StableLogicalIdText;
	if (CFDACommonPrimitives::ParseStringField(RootObject, TEXT("StableLogicalId"), TEXT("StableLogicalId"), StableLogicalIdText, Result.Issues))
	{
		Result.Record.StableLogicalId = FName(*StableLogicalIdText);
		if (Result.Record.StableLogicalId.IsNone())
		{
			CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::StableIdentityMissing, TEXT("StableLogicalId"), TEXT("Required StableLogicalId는 NAME_None/empty일 수 없습니다."));
		}
	}

	CFDACommonPrimitives::ParseStringField(RootObject, TEXT("TargetObjectPath"), TEXT("TargetObjectPath"), Result.Record.TargetObjectPath, Result.Issues);
	if (!Result.Record.TargetObjectPath.IsEmpty())
	{
		CFDACommonPrimitives::ValidateTargetObjectPath(Result.Record.TargetObjectPath, Result.Issues);
	}
	CFDACommonPrimitives::ParseBaseSemanticFingerprint(RootObject, Result.Record.bHasBaseSemanticFingerprint, Result.Record.BaseSemanticFingerprint, Result.Issues);

	// JSON TypeKey가 선택한 production provider entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* SelectedProviderEntry = CFDATypeDispatch::FindExactProviderEntry(Result.Record.SchemaId, Result.Record.DataAssetTypeClassPath, &ProviderLookupError);
	// VehicleDefense parser가 허용하는 exact production entry입니다.
	const FCFDATypeProviderEntry& VehicleDefenseProviderEntry = CFDAVehicleDefenseProvider::GetProvider();
	if (SelectedProviderEntry == nullptr || SelectedProviderEntry != &VehicleDefenseProviderEntry)
	{
		CFDACommonPrimitives::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderLookupError.IsEmpty() ? TEXT("VehicleDefense parser에 등록된 exact trusted TypeKey provider가 아닙니다.") : ProviderLookupError);
	}
	else
	{
		// Selected VehicleDefense structural descriptor입니다.
		const FCFDATypeProvider& Provider = SelectedProviderEntry->Descriptor;
		if (Result.Record.SchemaRevision != Provider.SchemaRevision)
		{
			CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported, TEXT("SchemaRevision"), TEXT("Selected VehicleDefense provider와 정확히 같은 SchemaRevision만 지원합니다."));
		}
		if (Result.Record.AdapterContractRevision != Provider.AdapterContractRevision)
		{
			CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::AdapterRevisionMismatch, TEXT("AdapterContractRevision"), TEXT("Selected VehicleDefense typed adapter와 정확히 같은 AdapterContractRevision만 지원합니다."));
		}
		if (!StagingRelativePath.IsEmpty()
			&& !CFDATypeDispatch::NormalizeProviderStagingPath(Provider, StagingRelativePath, Result.Record.StagingRelativePath))
		{
			CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingRelativePath"), TEXT("Staging source path가 VehicleDefense provider canonical StagingRoot 아래 `.json` 경로가 아닙니다."));
		}
	}

	if (!CFDACommonPrimitives::HasBlockingIssue(Result.Issues))
	{
		// Strict VehicleDefense Payload JSON object입니다.
		TSharedPtr<FJsonObject> PayloadObject;
		if (CFDACommonPrimitives::ParseObjectField(RootObject, TEXT("Payload"), TEXT("Payload"), PayloadObject, Result.Issues))
		{
			CFDAVehicleDefenseProviderPrivate::ParsePayload(PayloadObject, Result.Record.Payload, Result.Issues);
		}
	}

	if (!Result.Record.StableLogicalId.IsNone()
		&& !Result.Record.Payload.DefenseId.IsNone()
		&& Result.Record.StableLogicalId != Result.Record.Payload.DefenseId)
	{
		CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::StableIdentityMismatch, TEXT("StableLogicalId"), TEXT("StableLogicalId와 Payload.DefenseId가 같은 FName semantic identity가 아닙니다."));
	}

	if (!CFDACommonPrimitives::HasBlockingIssue(Result.Issues))
	{
		// Typed desired VehicleDefense payload fingerprint 생성 오류입니다.
		FString FingerprintError;
		if (!BuildSemanticFingerprint(Result.Record.Payload, Result.Record.StagingSemanticFingerprint, FingerprintError))
		{
			CFDACommonPrimitives::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("Payload"), FingerprintError);
		}
	}

	Result.bValid = !CFDACommonPrimitives::HasBlockingIssue(Result.Issues);
	return Result;
}

// VehicleDefense typed payload를 current schema whole-record JSON으로 deterministic 직렬화합니다. 빈 BaseSemanticFingerprint는 JSON null을 뜻합니다.
bool CFDAVehicleDefenseProviderImpl::SerializeStagingJson(
	const FCFDAVehicleDefensePayload& Payload,
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
		OutError = TargetPathIssues.IsEmpty() ? TEXT("VehicleDefense serializer TargetObjectPath가 canonical object path가 아닙니다.") : TargetPathIssues[0].Message;
		return false;
	}
	if (!BaseSemanticFingerprint.IsEmpty() && !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(BaseSemanticFingerprint))
	{
		OutError = TEXT("VehicleDefense serializer BaseSemanticFingerprint는 empty(null) 또는 canonical sha256:<64 lowercase hex>여야 합니다.");
		return false;
	}

	// Serializer에 사용할 canonical ArmorType token입니다.
	FString ArmorTypeToken;
	if (!CFDAVehicleDefenseProviderPrivate::TryGetArmorTypeToken(Payload.ArmorType, ArmorTypeToken))
	{
		OutError = TEXT("VehicleDefense serializer가 ArmorType token을 생성할 수 없습니다.");
		return false;
	}

	// Exact VehicleDefense top-level exact17 + nested exact12 payload object입니다.
	TSharedRef<FJsonObject> PayloadObject = MakeShared<FJsonObject>();
	PayloadObject->SetStringField(TEXT("DefenseId"), Payload.DefenseId.ToString());
	PayloadObject->SetNumberField(TEXT("DefenseMassKg"), Payload.DefenseMassKg);
	PayloadObject->SetBoolField(TEXT("bUseShield"), Payload.bUseShield);
	PayloadObject->SetNumberField(TEXT("MaximumShield"), Payload.MaximumShield);
	PayloadObject->SetNumberField(TEXT("ShieldRegenerationDelaySeconds"), Payload.ShieldRegenerationDelaySeconds);
	PayloadObject->SetNumberField(TEXT("ShieldRegenerationPerSecond"), Payload.ShieldRegenerationPerSecond);
	PayloadObject->SetStringField(TEXT("ArmorType"), ArmorTypeToken);
	PayloadObject->SetNumberField(TEXT("ArmorResistance"), Payload.ArmorResistance);
	PayloadObject->SetObjectField(TEXT("FrontArmorConfig"), CFDAVehicleDefenseProviderPrivate::BuildArmorConfigObject(Payload.FrontArmorConfig));
	PayloadObject->SetObjectField(TEXT("LeftArmorConfig"), CFDAVehicleDefenseProviderPrivate::BuildArmorConfigObject(Payload.LeftArmorConfig));
	PayloadObject->SetObjectField(TEXT("RightArmorConfig"), CFDAVehicleDefenseProviderPrivate::BuildArmorConfigObject(Payload.RightArmorConfig));
	PayloadObject->SetObjectField(TEXT("RearArmorConfig"), CFDAVehicleDefenseProviderPrivate::BuildArmorConfigObject(Payload.RearArmorConfig));
	PayloadObject->SetObjectField(TEXT("TopArmorConfig"), CFDAVehicleDefenseProviderPrivate::BuildArmorConfigObject(Payload.TopArmorConfig));
	PayloadObject->SetObjectField(TEXT("BottomArmorConfig"), CFDAVehicleDefenseProviderPrivate::BuildArmorConfigObject(Payload.BottomArmorConfig));
	PayloadObject->SetNumberField(TEXT("ShieldComponentDamageScale"), Payload.ShieldComponentDamageScale);
	PayloadObject->SetNumberField(TEXT("ArmorComponentDamageScale"), Payload.ArmorComponentDamageScale);
	PayloadObject->SetNumberField(TEXT("IntegrityComponentDamageScale"), Payload.IntegrityComponentDamageScale);

	// Current trusted VehicleDefense provider descriptor입니다.
	const FCFDATypeProvider& Provider = CFDAVehicleDefenseProvider::GetProvider().Descriptor;
	// Strict 8-field whole-record root object입니다.
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("SchemaId"), Provider.TypeKey.SchemaId);
	RootObject->SetNumberField(TEXT("SchemaRevision"), Provider.SchemaRevision);
	RootObject->SetNumberField(TEXT("AdapterContractRevision"), Provider.AdapterContractRevision);
	RootObject->SetStringField(TEXT("DataAssetTypeClassPath"), Provider.TypeKey.DataAssetTypeClassPath);
	RootObject->SetStringField(TEXT("StableLogicalId"), Payload.DefenseId.ToString());
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
		OutError = TEXT("VehicleDefense whole-record JSON serialization에 실패했습니다.");
		return false;
	}
	JsonWriter->Close();
	OutJsonText += LINE_TERMINATOR;
	OutError.Reset();
	return true;
}

// VehicleDefense typed payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
bool CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(
	const FCFDAVehicleDefensePayload& Payload,
	FString& OutFingerprint,
	FString& OutError)
{
	if (!CFDAVehicleDefenseProviderPrivate::ValidateTypedPayload(Payload, OutError))
	{
		OutFingerprint.Reset();
		return false;
	}

	// Exact metadata4 + VehicleDefense semantic leaf exact23 canonical token stream입니다.
	TArray<uint8> CanonicalBytes;
	CanonicalBytes.Reserve(1536);
	if (!CFDAVehicleDefenseProviderPrivate::AppendPayloadTokens(CanonicalBytes, Payload, OutError))
	{
		OutFingerprint.Reset();
		return false;
	}
	return CFDACommonPrimitives::HashCanonicalBytes(CanonicalBytes, OutFingerprint, OutError);
}

// Exact UCFVehicleDefenseData UObject를 authored whole-record payload로 read-only 추출합니다.
bool CFDAVehicleDefenseProviderImpl::ExtractPayload(
	const UCFVehicleDefenseData& DefenseAsset,
	FCFDAVehicleDefensePayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutPayload = FCFDAVehicleDefensePayload();
	OutIssues.Reset();
	OutPayload.DefenseId = DefenseAsset.DefenseId;
	OutPayload.DefenseMassKg = DefenseAsset.DefenseMassKg;
	OutPayload.bUseShield = DefenseAsset.bUseShield;
	OutPayload.MaximumShield = DefenseAsset.MaximumShield;
	OutPayload.ShieldRegenerationDelaySeconds = DefenseAsset.ShieldRegenerationDelaySeconds;
	OutPayload.ShieldRegenerationPerSecond = DefenseAsset.ShieldRegenerationPerSecond;
	OutPayload.ArmorType = DefenseAsset.ArmorType;
	OutPayload.ArmorResistance = DefenseAsset.ArmorResistance;
	OutPayload.FrontArmorConfig = DefenseAsset.FrontArmorConfig;
	OutPayload.LeftArmorConfig = DefenseAsset.LeftArmorConfig;
	OutPayload.RightArmorConfig = DefenseAsset.RightArmorConfig;
	OutPayload.RearArmorConfig = DefenseAsset.RearArmorConfig;
	OutPayload.TopArmorConfig = DefenseAsset.TopArmorConfig;
	OutPayload.BottomArmorConfig = DefenseAsset.BottomArmorConfig;
	OutPayload.ShieldComponentDamageScale = DefenseAsset.ShieldComponentDamageScale;
	OutPayload.ArmorComponentDamageScale = DefenseAsset.ArmorComponentDamageScale;
	OutPayload.IntegrityComponentDamageScale = DefenseAsset.IntegrityComponentDamageScale;

	if (OutPayload.DefenseId.IsNone())
	{
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::StableIdentityMissing, TEXT("Current.Payload.DefenseId"), TEXT("current CFVehicleDefenseData의 required DefenseId가 NAME_None입니다."));
	}
	if (!CFDACommonPrimitives::HasBlockingIssue(OutIssues))
	{
		// Current raw authored payload validation 오류입니다.
		FString TypedPayloadError;
		if (!CFDAVehicleDefenseProviderPrivate::ValidateTypedPayload(OutPayload, TypedPayloadError))
		{
			CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("Current.Payload"), TypedPayloadError);
		}
	}
	return !CFDACommonPrimitives::HasBlockingIssue(OutIssues);
}

// Mutable provider-local record의 payload와 cached fingerprint가 exact 일치하는지 검증합니다.
bool CFDAVehicleDefenseProviderImpl::ValidateRecordIntegrity(
	const FCFDAVehicleDefenseRecord& Record,
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
	return CFDACommonPrimitives::ValidateCachedSemanticFingerprint(Record.StagingSemanticFingerprint, RecomputedFingerprint, OutError);
}

// VehicleDefense typed payload를 exact UCFVehicleDefenseData UObject에 deterministic whole-record로 materialize합니다.
void CFDAVehicleDefenseProviderImpl::MaterializePayload(
	UCFVehicleDefenseData& TargetAsset,
	const FCFDAVehicleDefensePayload& Payload)
{
	TargetAsset.DefenseId = Payload.DefenseId;
	TargetAsset.DefenseMassKg = Payload.DefenseMassKg;
	TargetAsset.bUseShield = Payload.bUseShield;
	TargetAsset.MaximumShield = Payload.MaximumShield;
	TargetAsset.ShieldRegenerationDelaySeconds = Payload.ShieldRegenerationDelaySeconds;
	TargetAsset.ShieldRegenerationPerSecond = Payload.ShieldRegenerationPerSecond;
	TargetAsset.ArmorType = Payload.ArmorType;
	TargetAsset.ArmorResistance = Payload.ArmorResistance;
	TargetAsset.FrontArmorConfig = Payload.FrontArmorConfig;
	TargetAsset.LeftArmorConfig = Payload.LeftArmorConfig;
	TargetAsset.RightArmorConfig = Payload.RightArmorConfig;
	TargetAsset.RearArmorConfig = Payload.RearArmorConfig;
	TargetAsset.TopArmorConfig = Payload.TopArmorConfig;
	TargetAsset.BottomArmorConfig = Payload.BottomArmorConfig;
	TargetAsset.ShieldComponentDamageScale = Payload.ShieldComponentDamageScale;
	TargetAsset.ArmorComponentDamageScale = Payload.ArmorComponentDamageScale;
	TargetAsset.IntegrityComponentDamageScale = Payload.IntegrityComponentDamageScale;
}

// Fresh reviewed JSON을 재parse/rebind한 뒤 shared durable core로 exact VehicleDefense Create/Update를 실행합니다.
void CFDAVehicleDefenseProviderImpl::ApplyReviewedMutation(
	const FString& JsonText,
	const FCFDACommonPreviewRow& FreshRow,
	FCFDAStagingTargetApplyReport& OutTargetReport)
{
	// Mutation 직전 fresh source를 provider-local typed record로 다시 parse한 결과입니다.
	const FCFDAVehicleDefenseParseResult ParseResult = ParseJson(JsonText, FreshRow.Envelope.StagingRelativePath);
	if (!ParseResult.bValid)
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		OutTargetReport.Issues = ParseResult.Issues;
		OutTargetReport.Diagnostic = TEXT("VehicleDefense provider mutation callback에서 fresh typed Staging 재parse가 실패했습니다.");
		return;
	}

	// Fresh typed record의 cached semantic fingerprint integrity 실패 상세입니다.
	FString IntegrityError;
	if (!ValidateRecordIntegrity(ParseResult.Record, IntegrityError))
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		CFDACommonPrimitives::AddIssue(OutTargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("StagingSemanticFingerprint"), IntegrityError);
		OutTargetReport.Diagnostic = IntegrityError;
		return;
	}

	// Provider-local typed record를 shared durable row와 대조할 payload-free projection입니다.
	const FCFDACommonEnvelope ParsedEnvelope = BuildCommonEnvelope(ParseResult.Record, FreshRow.Envelope.CurrentSemanticFingerprint, FreshRow.Kind);
	if (!CFDATypeDispatch::AreCommonEnvelopesEquivalent(ParsedEnvelope, FreshRow.Envelope))
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		CFDACommonPrimitives::AddIssue(OutTargetReport.Issues, ECFDAStagingIssueCode::ApprovalStale, TEXT("Approval"), TEXT("VehicleDefense typed mutation record가 immediate fresh common evidence와 다릅니다."));
		OutTargetReport.Diagnostic = TEXT("VehicleDefense typed mutation record/common evidence mismatch입니다.");
		return;
	}

	CFDADurableCore::ApplyTypedTarget<UCFVehicleDefenseData, FCFDAVehicleDefensePayload>(
		FreshRow,
		ParseResult.Record.Payload,
		&CFDAVehicleDefenseProviderImpl::MaterializePayload,
		&CFDAVehicleDefenseProviderImpl::ExtractPayload,
		&CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint,
		OutTargetReport);
}

// Payload-free VehicleDefense candidate를 exact UCFVehicleDefenseData current truth로 read-only 해석합니다.
bool CFDAVehicleDefenseProviderImpl::ResolveCommonCurrentState(
	const FCFDACommonEnvelope& Envelope,
	FCFDACommonCurrentState& OutCurrentState,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutCurrentState = FCFDACommonCurrentState();
	OutIssues.Reset();

	// Envelope TypeKey에 등록된 exact production entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(Envelope.SchemaId, Envelope.DataAssetTypeClassPath, &ProviderLookupError);
	// VehicleDefense current resolver가 허용하는 exact production provider entry입니다.
	const FCFDATypeProviderEntry& VehicleDefenseProviderEntry = CFDAVehicleDefenseProvider::GetProvider();
	// Provider contract validation 오류입니다.
	FString ProviderContractError;
	if (ProviderEntry == nullptr
		|| ProviderEntry != &VehicleDefenseProviderEntry
		|| !CFDATypeDispatch::ValidateProviderContract(Envelope, ProviderEntry->Descriptor, ProviderContractError))
	{
		// Fail-closed provider diagnostic입니다.
		const FString ProviderDiagnostic = ProviderEntry == nullptr
			? ProviderLookupError
			: (ProviderEntry != &VehicleDefenseProviderEntry ? TEXT("VehicleDefense current-state resolver에 등록된 typed provider가 아닙니다.") : ProviderContractError);
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::SchemaUnsupported, TEXT("TypeKey"), ProviderDiagnostic);
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
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("Registry"), FString::Printf(TEXT("CF-FQ-045 current descriptor 등록에 실패했습니다: %s"), *RegistryError));
		return false;
	}

	// CFVehicleDefenseData public semantic descriptor입니다.
	const FCFDASemanticDescriptor* Descriptor = TypeRegistry.FindDescriptor(Envelope.DataAssetTypeClassPath);
	if (Descriptor == nullptr
		|| Descriptor->IdentityPolicy != ECFDAIdentityPolicy::Required
		|| Descriptor->IdentityResolverKind != ECFDAIdentityResolverKind::ExplicitFName
		|| Descriptor->IdentitySourceName != Provider.StableIdentitySourceName.ToString())
	{
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("Registry.Identity"), TEXT("CFVehicleDefenseData Registry identity 계약이 Required/ExplicitFName/DefenseId와 다릅니다."));
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

		if (OutCurrentState.RequestedTargetClassPath.Equals(Provider.TypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
		{
			// Exact expected target UObject semantic readback입니다.
			UObject* LoadedTargetObject = ResolvedTargetObject != nullptr ? ResolvedTargetObject : TargetAssetData.GetAsset();
			// Exact UCFVehicleDefenseData current target입니다.
			const UCFVehicleDefenseData* CurrentDefense = Cast<UCFVehicleDefenseData>(LoadedTargetObject);
			if (CurrentDefense == nullptr)
			{
				CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("Current.TargetObjectPath"), TEXT("Asset Registry에는 expected CFVehicleDefenseData target이 있지만 UObject semantic readback에 실패했습니다."));
				return false;
			}

			OutCurrentState.RequestedTargetStableLogicalId = CurrentDefense->DefenseId;
			// Current exact target package입니다.
			const UPackage* CurrentPackage = CurrentDefense->GetOutermost();
			OutCurrentState.bRequestedTargetDirty = CurrentPackage != nullptr && CurrentPackage->IsDirty();

			// Current persisted/loaded VehicleDefense whole-record payload입니다.
			FCFDAVehicleDefensePayload CurrentPayload;
			if (!ExtractPayload(*CurrentDefense, CurrentPayload, OutIssues))
			{
				return false;
			}

			// Current semantic fingerprint 생성 오류입니다.
			FString CurrentFingerprintError;
			if (!BuildSemanticFingerprint(CurrentPayload, OutCurrentState.CurrentSemanticFingerprint, CurrentFingerprintError))
			{
				CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("CurrentSemanticFingerprint"), CurrentFingerprintError);
				return false;
			}
		}
	}

	// Asset Registry와 loaded-but-unregistered UObject를 합친 exact class-scoped DefenseId path 집합입니다.
	TSet<FString> StableIdentityObjectPaths;
	for (TObjectIterator<UCFVehicleDefenseData> LoadedDefenseIterator; LoadedDefenseIterator; ++LoadedDefenseIterator)
	{
		// 현재 process에 살아 있는 exact UCFVehicleDefenseData 후보입니다.
		const UCFVehicleDefenseData* LoadedDefense = *LoadedDefenseIterator;
		if (LoadedDefense == nullptr
			|| LoadedDefense->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
			|| LoadedDefense->DefenseId != Envelope.StableLogicalId)
		{
			continue;
		}

		// Loaded VehicleDefense UObject exact object identity입니다.
		const FString LoadedObjectPath = FSoftObjectPath(LoadedDefense).ToString();
		if (!LoadedObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive))
		{
			continue;
		}
		StableIdentityObjectPaths.Add(LoadedObjectPath);
	}

	if (AssetRegistry.IsLoadingAssets())
	{
		CFDACommonPrimitives::AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("Registry.IdentityScan"), TEXT("Asset Registry가 아직 loading 중이라 VehicleDefense StableIdentity current truth를 확정할 수 없습니다."));
		return false;
	}

	// Current Project exact UCFVehicleDefenseData metadata filter입니다.
	FARFilter IdentityFilter;
	IdentityFilter.ClassPaths.Add(UCFVehicleDefenseData::StaticClass()->GetClassPathName());
	IdentityFilter.bRecursiveClasses = false;
	// Current exact UCFVehicleDefenseData metadata 목록입니다.
	TArray<FAssetData> DefenseAssets;
	(void)AssetRegistry.GetAssets(IdentityFilter, DefenseAssets, false);
	for (const FAssetData& DefenseAssetData : DefenseAssets)
	{
		// DefenseId semantic 비교를 위해 read-only load한 exact DataAsset입니다.
		const UCFVehicleDefenseData* CandidateDefense = Cast<UCFVehicleDefenseData>(DefenseAssetData.GetAsset());
		if (CandidateDefense == nullptr || CandidateDefense->DefenseId != Envelope.StableLogicalId)
		{
			continue;
		}
		StableIdentityObjectPaths.Add(DefenseAssetData.GetSoftObjectPath().ToString());
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
