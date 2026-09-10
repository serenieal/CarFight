// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoProvider.cpp
// Version: v1.4.0
// Date: 2026-09-10
// Description: CF-FQ-051 AmmoData strict typed authoring provider + accepted per-TypeKey DACE production path 구현입니다.
// Changelog:
// - v1.4.0: DAO-P0-04에서 deterministic Ammo whole-record production serializer를 추가하고 independent DACE bootstrap 승인에 맞춰 owner를 CFDAAmmoDace, readiness를 ContractReady로 승격했습니다. Product canonical exact0은 유지합니다.
// - v1.3.0: DAO-P0-04 correction에서 Ammo DACE를 ContractNotReady로 명시하고 Product canonical exact0을 explicit empty target set으로 동결했습니다. accepted bootstrap은 추가하지 않습니다.
// - v1.2.1: ReviewedMutationReady current 상태와 어긋나던 provider initialization/current resolver의 stale read-only 주석을 교정.
// - v1.2.0: Ammo exact8 deterministic materializer와 Reviewed Apply callback을 추가하고 provider-neutral CFDADurableCore를 통해 exact Create/Update/Save/reload/readback을 실행.
// - v1.1.0: TargetObjectPath canonical validation과 BaseSemanticFingerprint physical parse를 CFDACommonPrimitives 단일 authority로 rewire하고 Ammo read-only boundary는 유지.
// - v1.0.0: Ammo exact8 parser, set-like AmmoTags, SoftObjectPath metadata validation, semantic fingerprint, UObject extractor/current resolver와 ReadOnlyPreviewReady provider를 구현.
// Migration:
// - Ammo authoring numeric 값은 strict reject/no-clamp이며 UCFAmmoData runtime defensive clamp를 변경하지 않습니다.
// - AmmoIcon referenced asset 검증은 Asset Registry metadata만 사용하고 referenced Texture를 LoadObject/ResolveObject/GetAsset 하지 않습니다.
// - v1.2.1은 동작 변경 없이 readiness 설명만 current ReviewedMutationReady 계약에 맞춥니다.
// - v1.2.0부터 Reviewed mutation이 열리지만 exact Review/TOCTOU gate를 통과한 candidate만 shared durable core로 진입합니다.
// - v1.3.0의 DACE boundary는 authoring ReviewedMutationReady와 독립입니다. accepted DACE history가 생기기 전까지 ContractNotReady이며 Product Ammo exact0을 유지합니다.
// - v1.4.0부터 DACE-AmmoData bootstrap exact1과 current descriptor/probe가 승인된 상태에서 ContractReady입니다. serializer는 memory JSON만 생성하며 Staging/Product 파일을 쓰지 않습니다.

#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDADurableCore.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFAmmoData.h"
#include "DataManagement/CFDATypeRegistry.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectIterator.h"

namespace CFDAAmmoProviderPrivate
{
	// Ammo exact8 payload JSON field set입니다.
	const TArray<FString>& GetPayloadFields()
	{
		// Process lifetime 동안 재사용할 exact payload field 목록입니다.
		static const TArray<FString> PayloadFields =
		{
			TEXT("AmmoId"),
			TEXT("AmmoDisplayName"),
			TEXT("AmmoFamilyId"),
			TEXT("UnitMassKg"),
			TEXT("AmmoTags"),
			TEXT("AmmoIcon"),
			TEXT("MaximumLoadableAmmoCount"),
			TEXT("bCanBeResupplied")
		};
		return PayloadFields;
	}

	// Ammo whole-record top-level exact field set입니다.
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

	// AmmoIcon이 참조해야 하는 exact base class path입니다.
	FTopLevelAssetPath GetAmmoIconBaseClassPath()
	{
		return UTexture2D::StaticClass()->GetClassPathName();
	}

	// P0 target canonical validation을 provider-neutral common authority로 위임합니다.
	bool ValidateTargetObjectPath(
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ValidateTargetObjectPath(ObjectPath, OutIssues);
	}

	// JSON string field를 FName semantic으로 parse합니다.
	bool ParseNameField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		const bool bAllowNone,
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
		if (!bAllowNone && OutValue.IsNone())
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::StableIdentityMissing,
				FieldPath,
				TEXT("required FName identity는 NAME_None/empty일 수 없습니다."));
			return false;
		}
		return true;
	}

	// JSON number field를 exact nonnegative int32로 strict parse합니다.
	bool ParseNonNegativeInt32Field(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		int32& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// Exact numeric JSON field입니다.
		const TSharedPtr<FJsonValue> Value = CFDACommonPrimitives::RequireField(
			Object,
			FieldName,
			FieldPath,
			EJson::Number,
			OutIssues);
		if (!Value.IsValid())
		{
			return false;
		}

		// JSON number 원본 double 값입니다.
		const double NumberValue = Value->AsNumber();
		if (!FMath::IsFinite(NumberValue)
			|| NumberValue < 0.0
			|| NumberValue > static_cast<double>(MAX_int32)
			|| static_cast<double>(static_cast<int32>(NumberValue)) != NumberValue)
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				FieldPath,
				TEXT("authored 값은 0 이상의 exact finite int32 JSON number여야 하며 clamp하지 않습니다."));
			return false;
		}

		OutValue = static_cast<int32>(NumberValue);
		return true;
	}

	// AmmoTags를 set-like FName semantic으로 parse하고 deterministic canonical order로 정렬합니다.
	bool ParseAmmoTags(
		const TSharedPtr<FJsonObject>& PayloadObject,
		FCFDAAmmoPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// AmmoTags exact JSON array field입니다.
		const TSharedPtr<FJsonValue> TagsValue = CFDACommonPrimitives::RequireField(
			PayloadObject,
			TEXT("AmmoTags"),
			TEXT("Payload.AmmoTags"),
			EJson::Array,
			OutIssues);
		if (!TagsValue.IsValid())
		{
			return false;
		}

		OutPayload.AmmoTags.Reset();
		// 이미 관측한 case-insensitive FName semantic key 집합입니다.
		TSet<FString> SeenCanonicalTags;
		// Physical JSON tag element 목록입니다.
		const TArray<TSharedPtr<FJsonValue>>& TagValues = TagsValue->AsArray();
		for (int32 TagIndex = 0; TagIndex < TagValues.Num(); ++TagIndex)
		{
			// 현재 tag JSON value입니다.
			const TSharedPtr<FJsonValue>& TagValue = TagValues[TagIndex];
			// Stable diagnostic용 element field path입니다.
			const FString FieldPath = FString::Printf(TEXT("Payload.AmmoTags[%d]"), TagIndex);
			if (!TagValue.IsValid() || TagValue->Type != EJson::String)
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::TypeMismatch,
					FieldPath,
					TEXT("AmmoTags element는 JSON string이어야 합니다."));
				continue;
			}

			// JSON string에서 변환한 typed FName tag입니다.
			const FName TagName(*TagValue->AsString());
			if (TagName.IsNone())
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					FieldPath,
					TEXT("AmmoTags element는 NAME_None/empty일 수 없습니다."));
				continue;
			}

			// FName semantic equality에 맞춘 lowercase canonical key입니다.
			const FString CanonicalTag = CFDACommonPrimitives::CanonicalNameText(TagName);
			if (SeenCanonicalTags.Contains(CanonicalTag))
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					FieldPath,
					TEXT("AmmoTags에 FName semantic duplicate tag가 있습니다."));
				continue;
			}

			SeenCanonicalTags.Add(CanonicalTag);
			OutPayload.AmmoTags.Add(TagName);
		}

		// Physical input order와 무관한 deterministic canonical materialize order입니다.
		OutPayload.AmmoTags.Sort([](const FName Left, const FName Right)
		{
			return CFDACommonPrimitives::CanonicalNameText(Left) < CFDACommonPrimitives::CanonicalNameText(Right);
		});
		return !CFDACommonPrimitives::HasBlockingIssue(OutIssues);
	}

	// AmmoIcon을 JSON null 또는 canonical SoftObjectPath string으로 strict parse합니다.
	bool ParseAmmoIcon(
		const TSharedPtr<FJsonObject>& PayloadObject,
		FCFDAAmmoPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// AmmoIcon required physical JSON field입니다.
		const TSharedPtr<FJsonValue> IconValue = PayloadObject.IsValid()
			? PayloadObject->GetFieldUntyped(TEXT("AmmoIcon"))
			: nullptr;
		if (!IconValue.IsValid())
		{
			return false;
		}

		OutPayload.AmmoIcon.Reset();
		if (IconValue->Type == EJson::Null)
		{
			return true;
		}
		if (IconValue->Type != EJson::String)
		{
			CFDACommonPrimitives::AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::TypeMismatch,
				TEXT("Payload.AmmoIcon"),
				TEXT("AmmoIcon은 null 또는 canonical SoftObjectPath JSON string이어야 합니다."));
			return false;
		}

		// Non-null icon string physical representation입니다.
		const FString IconPathText = IconValue->AsString();
		return CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
			IconPathText,
			TEXT("Payload.AmmoIcon"),
			false,
			OutPayload.AmmoIcon,
			OutIssues);
	}

	// Ammo exact8 payload JSON object를 typed DTO로 strict parse합니다.
	bool ParsePayload(
		const TSharedPtr<FJsonObject>& PayloadObject,
		FCFDAAmmoPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutPayload = FCFDAAmmoPayload();
		CFDACommonPrimitives::ValidateExactFields(
			PayloadObject,
			GetPayloadFields(),
			TEXT("Payload"),
			OutIssues);

		ParseNameField(
			PayloadObject,
			TEXT("AmmoId"),
			TEXT("Payload.AmmoId"),
			false,
			OutPayload.AmmoId,
			OutIssues);

		// AmmoDisplayName Literal FText JSON object입니다.
		TSharedPtr<FJsonObject> DisplayNameObject;
		if (CFDACommonPrimitives::ParseObjectField(
			PayloadObject,
			TEXT("AmmoDisplayName"),
			TEXT("Payload.AmmoDisplayName"),
			DisplayNameObject,
			OutIssues))
		{
			CFDACommonPrimitives::ParseLiteralText(
				DisplayNameObject,
				TEXT("Payload.AmmoDisplayName"),
				OutPayload.AmmoDisplayName,
				OutIssues);
		}

		ParseNameField(
			PayloadObject,
			TEXT("AmmoFamilyId"),
			TEXT("Payload.AmmoFamilyId"),
			true,
			OutPayload.AmmoFamilyId,
			OutIssues);

		CFDACommonPrimitives::ParseFloatField(
			PayloadObject,
			TEXT("UnitMassKg"),
			TEXT("Payload.UnitMassKg"),
			0.0,
			static_cast<double>(TNumericLimits<float>::Max()),
			OutPayload.UnitMassKg,
			OutIssues);

		ParseAmmoTags(PayloadObject, OutPayload, OutIssues);
		ParseAmmoIcon(PayloadObject, OutPayload, OutIssues);
		ParseNonNegativeInt32Field(
			PayloadObject,
			TEXT("MaximumLoadableAmmoCount"),
			TEXT("Payload.MaximumLoadableAmmoCount"),
			OutPayload.MaximumLoadableAmmoCount,
			OutIssues);
		CFDACommonPrimitives::ParseBoolField(
			PayloadObject,
			TEXT("bCanBeResupplied"),
			TEXT("Payload.bCanBeResupplied"),
			OutPayload.bCanBeResupplied,
			OutIssues);
		return !CFDACommonPrimitives::HasBlockingIssue(OutIssues);
	}

	// Typed Ammo payload가 frozen authored semantic 범위를 만족하는지 검증합니다.
	bool ValidateTypedPayload(
		const FCFDAAmmoPayload& Payload,
		FString& OutError)
	{
		if (Payload.AmmoId.IsNone())
		{
			OutError = TEXT("AmmoId가 NAME_None입니다.");
			return false;
		}
		if (!FMath::IsFinite(Payload.UnitMassKg) || Payload.UnitMassKg < 0.0f)
		{
			OutError = TEXT("UnitMassKg는 0 이상의 finite authored float여야 하며 clamp하지 않습니다.");
			return false;
		}
		if (Payload.MaximumLoadableAmmoCount < 0)
		{
			OutError = TEXT("MaximumLoadableAmmoCount는 0 이상의 authored int32여야 하며 clamp하지 않습니다.");
			return false;
		}

		// AmmoTags semantic duplicate 검사 집합입니다.
		TSet<FString> SeenCanonicalTags;
		for (const FName TagName : Payload.AmmoTags)
		{
			if (TagName.IsNone())
			{
				OutError = TEXT("AmmoTags에 NAME_None element가 있습니다.");
				return false;
			}
			// Current tag의 case-insensitive semantic key입니다.
			const FString CanonicalTag = CFDACommonPrimitives::CanonicalNameText(TagName);
			if (SeenCanonicalTags.Contains(CanonicalTag))
			{
				OutError = TEXT("AmmoTags에 FName semantic duplicate element가 있습니다.");
				return false;
			}
			SeenCanonicalTags.Add(CanonicalTag);
		}

		if (!Payload.AmmoIcon.IsNull())
		{
			// Typed icon path가 canonical physical representation인지 재검증할 diagnostics입니다.
			TArray<FCFDAStagingIssue> IconPathIssues;
			// Typed icon path의 canonical source text입니다.
			const FString IconPathText = Payload.AmmoIcon.ToString();
			// Round-trip parse 결과입니다.
			FSoftObjectPath RoundTripPath;
			if (!CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
				IconPathText,
				TEXT("Payload.AmmoIcon"),
				false,
				RoundTripPath,
				IconPathIssues)
				|| RoundTripPath != Payload.AmmoIcon)
			{
				OutError = TEXT("AmmoIcon typed value가 canonical top-level SoftObjectPath가 아닙니다.");
				return false;
			}
		}

		OutError.Reset();
		return true;
	}

	// AmmoIcon desired/current reference를 metadata-only로 existence/class 검증합니다.
	bool ValidateAmmoIconReference(
		const FCFDAAmmoPayload& Payload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		if (Payload.AmmoIcon.IsNull())
		{
			return true;
		}
		return CFDACommonPrimitives::ValidateAssetReferenceMetadata(
			Payload.AmmoIcon,
			GetAmmoIconBaseClassPath(),
			TEXT("Payload.AmmoIcon"),
			OutIssues);
	}

	// Typed Ammo payload의 exact deterministic semantic token stream을 작성합니다.
	void AppendPayloadTokens(
		TArray<uint8>& OutBytes,
		const FCFDAAmmoPayload& Payload)
	{
		// Current Ammo provider structural authority입니다.
		const FCFDATypeProvider& Provider = CFDAAmmoProvider::GetProvider().Descriptor;
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("SchemaId"), Provider.TypeKey.SchemaId);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(Provider.SchemaRevision));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(Provider.AdapterContractRevision));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), Provider.TypeKey.DataAssetTypeClassPath);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.AmmoId"), CFDACommonPrimitives::CanonicalNameText(Payload.AmmoId));
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.AmmoDisplayName"), Payload.AmmoDisplayName.Text);
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.AmmoFamilyId"), CFDACommonPrimitives::CanonicalNameText(Payload.AmmoFamilyId));
		CFDACommonPrimitives::AppendFloatToken(OutBytes, TEXT("Payload.UnitMassKg"), Payload.UnitMassKg);

		// Fingerprint 전용 canonical sorted tag text 목록입니다.
		TArray<FString> CanonicalTags;
		CanonicalTags.Reserve(Payload.AmmoTags.Num());
		for (const FName TagName : Payload.AmmoTags)
		{
			CanonicalTags.Add(CFDACommonPrimitives::CanonicalNameText(TagName));
		}
		CanonicalTags.Sort([](const FString& Left, const FString& Right)
		{
			return Left < Right;
		});
		CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.AmmoTags.Count"), FString::FromInt(CanonicalTags.Num()));
		for (const FString& CanonicalTag : CanonicalTags)
		{
			CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.AmmoTags[]"), CanonicalTag);
		}

		CFDACommonPrimitives::AppendBoolToken(OutBytes, TEXT("Payload.AmmoIcon.IsNull"), Payload.AmmoIcon.IsNull());
		if (!Payload.AmmoIcon.IsNull())
		{
			CFDACommonPrimitives::AppendStringToken(OutBytes, TEXT("Payload.AmmoIcon.Path"), Payload.AmmoIcon.ToString());
		}
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			TEXT("Payload.MaximumLoadableAmmoCount"),
			FString::FromInt(Payload.MaximumLoadableAmmoCount));
		CFDACommonPrimitives::AppendBoolToken(OutBytes, TEXT("Payload.bCanBeResupplied"), Payload.bCanBeResupplied);
	}

	// BaseSemanticFingerprint common envelope physical contract를 provider-neutral authority로 parse합니다.
	void ParseBaseFingerprint(
		const TSharedPtr<FJsonObject>& RootObject,
		FCFDAAmmoRecord& OutRecord,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		CFDACommonPrimitives::ParseBaseSemanticFingerprint(
			RootObject,
			OutRecord.bHasBaseSemanticFingerprint,
			OutRecord.BaseSemanticFingerprint,
			OutIssues);
	}

	// Persisted/loaded Ammo payload에서 canonical tag order와 value validity를 확정합니다.
	bool CanonicalizeExtractedTags(
		FCFDAAmmoPayload& InOutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// Current persisted tags의 semantic duplicate 검사 집합입니다.
		TSet<FString> SeenCanonicalTags;
		for (int32 TagIndex = 0; TagIndex < InOutPayload.AmmoTags.Num(); ++TagIndex)
		{
			// Current persisted tag value입니다.
			const FName TagName = InOutPayload.AmmoTags[TagIndex];
			// Current persisted tag diagnostic path입니다.
			const FString FieldPath = FString::Printf(TEXT("Current.Payload.AmmoTags[%d]"), TagIndex);
			if (TagName.IsNone())
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					FieldPath,
					TEXT("current AmmoTags에 NAME_None element가 있습니다."));
				continue;
			}
			// Current tag semantic key입니다.
			const FString CanonicalTag = CFDACommonPrimitives::CanonicalNameText(TagName);
			if (SeenCanonicalTags.Contains(CanonicalTag))
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					FieldPath,
					TEXT("current AmmoTags에 FName semantic duplicate가 있습니다."));
				continue;
			}
			SeenCanonicalTags.Add(CanonicalTag);
		}

		InOutPayload.AmmoTags.Sort([](const FName Left, const FName Right)
		{
			return CFDACommonPrimitives::CanonicalNameText(Left) < CFDACommonPrimitives::CanonicalNameText(Right);
		});
		return !CFDACommonPrimitives::HasBlockingIssue(OutIssues);
	}
}

// Current AmmoData ReviewedMutationReady trusted provider authority를 반환합니다.
const FCFDAAmmoTypeProvider& CFDAAmmoProvider::GetProvider()
{
	// CF-FQ-051 frozen Ammo schema/class/revision/root/identity authority입니다.
	static const FCFDAAmmoTypeProvider AmmoProvider = []()
	{
		// Current ReviewedMutationReady Ammo provider entry입니다.
		FCFDAAmmoTypeProvider Provider;
		Provider.Descriptor.TypeKey.SchemaId = TEXT("CarFight.DataAsset.AmmoData");
		Provider.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFAmmoData");
		Provider.Descriptor.SchemaRevision = 1;
		Provider.Descriptor.AdapterContractRevision = 1;
		Provider.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging/AmmoData");
		Provider.Descriptor.StableIdentityPolicy = ECFDAStableIdentityPolicy::Required;
		Provider.Descriptor.StableIdentityResolver = ECFDAStableIdentityResolver::ExplicitFName;
		Provider.Descriptor.StableIdentitySourceName = FName(TEXT("AmmoId"));
		Provider.Descriptor.DaceContractOwnerName = FName(TEXT("CFDAAmmoDace"));
		Provider.Descriptor.DaceAcceptedHistoryNamespace = TEXT("DACE-AmmoData");
		Provider.Descriptor.DaceReadiness = ECFDADaceReadiness::ContractReady;
		Provider.Descriptor.bDaceCanonicalStagingTargetSetDeclared = true;
		Provider.Descriptor.DaceCanonicalStagingRelativePaths.Reset();
		Provider.Readiness = ECFDAProviderReadiness::ReviewedMutationReady;
		Provider.Operations.ParseCommonCandidate = &CFDAAmmoProviderImpl::ParseCommonCandidate;
		Provider.Operations.ResolveCommonCurrentState = &CFDAAmmoProviderImpl::ResolveCommonCurrentState;
		Provider.Operations.ApplyReviewedMutation = &CFDAAmmoProviderImpl::ApplyReviewedMutation;
		return Provider;
	}();
	return AmmoProvider;
}

// Ammo typed record를 payload-free shared envelope로 투영합니다.
FCFDACommonEnvelope CFDAAmmoProviderImpl::BuildCommonEnvelope(
	const FCFDAAmmoRecord& Record,
	const FString& CurrentSemanticFingerprint,
	const ECFDAStagingPreviewKind PlannedOperation)
{
	// Shared orchestration으로 전달할 payload-free Ammo envelope입니다.
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

// Raw JSON을 strict Ammo typed parser + reference validation까지 통과시킨 뒤 common candidate만 반환합니다.
bool CFDAAmmoProviderImpl::ParseCommonCandidate(
	const FString& JsonText,
	const FString& StagingRelativePath,
	FCFDACommonEnvelope& OutEnvelope,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Provider-local strict Ammo parse 결과입니다.
	const FCFDAAmmoParseResult ParseResult = ParseJson(JsonText, StagingRelativePath);
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

	if (!CFDAAmmoProviderPrivate::ValidateAmmoIconReference(ParseResult.Record.Payload, OutIssues))
	{
		OutEnvelope = FCFDACommonEnvelope();
		return false;
	}

	OutEnvelope = BuildCommonEnvelope(ParseResult.Record);
	return true;
}

// Strict whole-record Ammo JSON을 provider-local typed record로 parse/canonicalize합니다.
FCFDAAmmoParseResult CFDAAmmoProviderImpl::ParseJson(
	const FString& JsonText,
	const FString& StagingRelativePath)
{
	// Parse/validation 결과입니다.
	FCFDAAmmoParseResult Result;
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
			TEXT("유효한 Ammo whole-record JSON object를 parse할 수 없습니다."));
		return Result;
	}

	CFDACommonPrimitives::ValidateExactFields(
		RootObject,
		CFDAAmmoProviderPrivate::GetTopLevelFields(),
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
		CFDAAmmoProviderPrivate::ValidateTargetObjectPath(Result.Record.TargetObjectPath, Result.Issues);
	}
	CFDAAmmoProviderPrivate::ParseBaseFingerprint(RootObject, Result.Record, Result.Issues);

	// JSON TypeKey가 선택한 production provider entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* SelectedProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		Result.Record.SchemaId,
		Result.Record.DataAssetTypeClassPath,
		&ProviderLookupError);
	// Ammo compatibility가 허용하는 exact production entry입니다.
	const FCFDATypeProviderEntry& AmmoProviderEntry = CFDAAmmoProvider::GetProvider();
	if (SelectedProviderEntry == nullptr || SelectedProviderEntry != &AmmoProviderEntry)
	{
		CFDACommonPrimitives::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderLookupError.IsEmpty()
				? TEXT("Ammo parser에 등록된 exact trusted TypeKey provider가 아닙니다.")
				: ProviderLookupError);
	}
	else
	{
		// Selected Ammo structural descriptor입니다.
		const FCFDATypeProvider& Provider = SelectedProviderEntry->Descriptor;
		if (Result.Record.SchemaRevision != Provider.SchemaRevision)
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::SchemaRevisionUnsupported,
				TEXT("SchemaRevision"),
				TEXT("Selected Ammo provider와 정확히 같은 SchemaRevision만 지원합니다."));
		}
		if (Result.Record.AdapterContractRevision != Provider.AdapterContractRevision)
		{
			CFDACommonPrimitives::AddIssue(
				Result.Issues,
				ECFDAStagingIssueCode::AdapterRevisionMismatch,
				TEXT("AdapterContractRevision"),
				TEXT("Selected Ammo typed adapter와 정확히 같은 AdapterContractRevision만 지원합니다."));
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
				TEXT("Staging source path가 Ammo provider canonical StagingRoot 아래 `.json` 경로가 아닙니다."));
		}
	}

	if (!CFDACommonPrimitives::HasBlockingIssue(Result.Issues))
	{
		// Strict Ammo Payload JSON object입니다.
		TSharedPtr<FJsonObject> PayloadObject;
		if (CFDACommonPrimitives::ParseObjectField(
			RootObject,
			TEXT("Payload"),
			TEXT("Payload"),
			PayloadObject,
			Result.Issues))
		{
			CFDAAmmoProviderPrivate::ParsePayload(PayloadObject, Result.Record.Payload, Result.Issues);
		}
	}

	if (!Result.Record.StableLogicalId.IsNone()
		&& !Result.Record.Payload.AmmoId.IsNone()
		&& Result.Record.StableLogicalId != Result.Record.Payload.AmmoId)
	{
		CFDACommonPrimitives::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::StableIdentityMismatch,
			TEXT("StableLogicalId"),
			TEXT("StableLogicalId와 Payload.AmmoId가 같은 FName semantic identity가 아닙니다."));
	}

	if (!CFDACommonPrimitives::HasBlockingIssue(Result.Issues))
	{
		// Typed desired Ammo payload fingerprint 생성 오류입니다.
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

// Ammo exact8 typed payload를 current schema whole-record JSON으로 deterministic 직렬화합니다. 빈 BaseSemanticFingerprint는 JSON null을 뜻합니다.
bool CFDAAmmoProviderImpl::SerializeStagingJson(
	const FCFDAAmmoPayload& Payload,
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
	if (!CFDAAmmoProviderPrivate::ValidateTargetObjectPath(TargetObjectPath, TargetPathIssues))
	{
		OutError = TargetPathIssues.IsEmpty()
			? TEXT("Ammo serializer TargetObjectPath가 canonical object path가 아닙니다.")
			: TargetPathIssues[0].Message;
		return false;
	}
	if (!BaseSemanticFingerprint.IsEmpty()
		&& !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(BaseSemanticFingerprint))
	{
		OutError = TEXT("Ammo serializer BaseSemanticFingerprint는 empty(null) 또는 canonical sha256:<64 lowercase hex>여야 합니다.");
		return false;
	}

	// Literal AmmoDisplayName physical object입니다.
	TSharedRef<FJsonObject> DisplayNameObject = MakeShared<FJsonObject>();
	DisplayNameObject->SetStringField(TEXT("Kind"), TEXT("Literal"));
	DisplayNameObject->SetStringField(TEXT("Text"), Payload.AmmoDisplayName.Text);

	// Deterministic canonical AmmoTags JSON array입니다.
	TArray<FString> CanonicalTagTexts;
	CanonicalTagTexts.Reserve(Payload.AmmoTags.Num());
	for (const FName TagName : Payload.AmmoTags)
	{
		CanonicalTagTexts.Add(CFDACommonPrimitives::CanonicalNameText(TagName));
	}
	CanonicalTagTexts.Sort([](const FString& Left, const FString& Right)
	{
		return Left.Compare(Right, ESearchCase::CaseSensitive) < 0;
	});
	// JSON writer에 전달할 tag value array입니다.
	TArray<TSharedPtr<FJsonValue>> TagValues;
	TagValues.Reserve(CanonicalTagTexts.Num());
	for (const FString& TagText : CanonicalTagTexts)
	{
		TagValues.Add(MakeShared<FJsonValueString>(TagText));
	}

	// Exact Ammo payload object입니다.
	TSharedRef<FJsonObject> PayloadObject = MakeShared<FJsonObject>();
	PayloadObject->SetStringField(TEXT("AmmoId"), Payload.AmmoId.ToString());
	PayloadObject->SetObjectField(TEXT("AmmoDisplayName"), DisplayNameObject);
	PayloadObject->SetStringField(TEXT("AmmoFamilyId"), Payload.AmmoFamilyId.IsNone() ? FString() : Payload.AmmoFamilyId.ToString());
	PayloadObject->SetNumberField(TEXT("UnitMassKg"), Payload.UnitMassKg);
	PayloadObject->SetArrayField(TEXT("AmmoTags"), TagValues);
	if (Payload.AmmoIcon.IsNull())
	{
		PayloadObject->SetField(TEXT("AmmoIcon"), MakeShared<FJsonValueNull>());
	}
	else
	{
		PayloadObject->SetStringField(TEXT("AmmoIcon"), Payload.AmmoIcon.ToString());
	}
	PayloadObject->SetNumberField(TEXT("MaximumLoadableAmmoCount"), Payload.MaximumLoadableAmmoCount);
	PayloadObject->SetBoolField(TEXT("bCanBeResupplied"), Payload.bCanBeResupplied);

	// Current trusted Ammo provider descriptor입니다.
	const FCFDATypeProvider& Provider = CFDAAmmoProvider::GetProvider().Descriptor;
	// Strict 8-field whole-record root object입니다.
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
	RootObject->SetStringField(TEXT("SchemaId"), Provider.TypeKey.SchemaId);
	RootObject->SetNumberField(TEXT("SchemaRevision"), Provider.SchemaRevision);
	RootObject->SetNumberField(TEXT("AdapterContractRevision"), Provider.AdapterContractRevision);
	RootObject->SetStringField(TEXT("DataAssetTypeClassPath"), Provider.TypeKey.DataAssetTypeClassPath);
	RootObject->SetStringField(TEXT("StableLogicalId"), Payload.AmmoId.ToString());
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
		OutError = TEXT("Ammo whole-record JSON serialization에 실패했습니다.");
		return false;
	}
	JsonWriter->Close();
	OutJsonText += LINE_TERMINATOR;
	OutError.Reset();
	return true;
}

// Ammo exact8 typed payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
bool CFDAAmmoProviderImpl::BuildSemanticFingerprint(
	const FCFDAAmmoPayload& Payload,
	FString& OutFingerprint,
	FString& OutError)
{
	if (!CFDAAmmoProviderPrivate::ValidateTypedPayload(Payload, OutError))
	{
		OutFingerprint.Reset();
		return false;
	}

	// Exact schema/revision/class + Ammo typed payload canonical token stream입니다.
	TArray<uint8> CanonicalBytes;
	CanonicalBytes.Reserve(1024);
	CFDAAmmoProviderPrivate::AppendPayloadTokens(CanonicalBytes, Payload);
	return CFDACommonPrimitives::HashCanonicalBytes(CanonicalBytes, OutFingerprint, OutError);
}

// Exact CFAmmoData UObject를 authored whole-record payload로 read-only 추출합니다.
bool CFDAAmmoProviderImpl::ExtractPayload(
	const UCFAmmoData& AmmoAsset,
	FCFDAAmmoPayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutPayload = FCFDAAmmoPayload();
	OutIssues.Reset();
	OutPayload.AmmoId = AmmoAsset.AmmoId;
	if (OutPayload.AmmoId.IsNone())
	{
		CFDACommonPrimitives::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::StableIdentityMissing,
			TEXT("Current.Payload.AmmoId"),
			TEXT("current CFAmmoData의 required AmmoId가 NAME_None입니다."));
	}

	CFDACommonPrimitives::ReadLiteralTextFromAsset(
		AmmoAsset.AmmoDisplayName,
		TEXT("Current.Payload.AmmoDisplayName"),
		OutPayload.AmmoDisplayName,
		OutIssues);
	OutPayload.AmmoFamilyId = AmmoAsset.AmmoFamilyId;
	OutPayload.UnitMassKg = AmmoAsset.UnitMassKg;
	OutPayload.AmmoTags = AmmoAsset.AmmoTags;
	OutPayload.AmmoIcon = AmmoAsset.AmmoIcon.ToSoftObjectPath();
	OutPayload.MaximumLoadableAmmoCount = AmmoAsset.MaximumLoadableAmmoCount;
	OutPayload.bCanBeResupplied = AmmoAsset.bCanBeResupplied;

	CFDAAmmoProviderPrivate::CanonicalizeExtractedTags(OutPayload, OutIssues);
	if (!CFDACommonPrimitives::HasBlockingIssue(OutIssues))
	{
		// Current raw authored payload validation 오류입니다.
		FString TypedPayloadError;
		if (!CFDAAmmoProviderPrivate::ValidateTypedPayload(OutPayload, TypedPayloadError))
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
bool CFDAAmmoProviderImpl::ValidateRecordIntegrity(
	const FCFDAAmmoRecord& Record,
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

// Ammo exact8 typed payload를 CFAmmoData UObject에 deterministic whole-record로 materialize합니다.
void CFDAAmmoProviderImpl::MaterializePayload(
	UCFAmmoData& TargetAsset,
	const FCFDAAmmoPayload& Payload)
{
	TargetAsset.AmmoId = Payload.AmmoId;
	TargetAsset.AmmoDisplayName = FText::FromString(Payload.AmmoDisplayName.Text);
	TargetAsset.AmmoFamilyId = Payload.AmmoFamilyId;
	TargetAsset.UnitMassKg = Payload.UnitMassKg;
	TargetAsset.AmmoTags = Payload.AmmoTags;
	TargetAsset.AmmoTags.Sort([](const FName Left, const FName Right)
	{
		return CFDACommonPrimitives::CanonicalNameText(Left) < CFDACommonPrimitives::CanonicalNameText(Right);
	});
	TargetAsset.AmmoIcon = TSoftObjectPtr<UTexture2D>(Payload.AmmoIcon);
	TargetAsset.MaximumLoadableAmmoCount = Payload.MaximumLoadableAmmoCount;
	TargetAsset.bCanBeResupplied = Payload.bCanBeResupplied;
}

// Fresh reviewed JSON을 재parse/rebind한 뒤 shared durable core로 exact Ammo Create/Update를 실행합니다.
void CFDAAmmoProviderImpl::ApplyReviewedMutation(
	const FString& JsonText,
	const FCFDACommonPreviewRow& FreshRow,
	FCFDAStagingTargetApplyReport& OutTargetReport)
{
	// Mutation 직전 fresh source를 provider-local typed record로 다시 parse한 결과입니다.
	const FCFDAAmmoParseResult ParseResult = ParseJson(JsonText, FreshRow.Envelope.StagingRelativePath);
	if (!ParseResult.bValid)
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		OutTargetReport.Issues = ParseResult.Issues;
		OutTargetReport.Diagnostic = TEXT("Ammo provider mutation callback에서 fresh typed Staging 재parse가 실패했습니다.");
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
	if (!CFDAAmmoProviderPrivate::ValidateAmmoIconReference(ParseResult.Record.Payload, OutTargetReport.Issues))
	{
		OutTargetReport.Result = ECFDAStagingTargetApplyResult::BlockedBeforeMutation;
		OutTargetReport.Diagnostic = TEXT("Ammo provider mutation callback에서 AmmoIcon referenced metadata validation이 실패했습니다.");
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
			TEXT("Ammo typed mutation record가 immediate fresh common evidence와 다릅니다."));
		OutTargetReport.Diagnostic = TEXT("Ammo typed mutation record/common evidence mismatch입니다.");
		return;
	}

	CFDADurableCore::ApplyTypedTarget<UCFAmmoData, FCFDAAmmoPayload>(
		FreshRow,
		ParseResult.Record.Payload,
		&CFDAAmmoProviderImpl::MaterializePayload,
		&CFDAAmmoProviderImpl::ExtractPayload,
		&CFDAAmmoProviderImpl::BuildSemanticFingerprint,
		OutTargetReport);
}

// Payload-free Ammo candidate를 exact CFAmmoData current truth로 read-only 해석합니다.
bool CFDAAmmoProviderImpl::ResolveCommonCurrentState(
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
	// Ammo current resolver가 허용하는 exact production provider entry입니다.
	const FCFDATypeProviderEntry& AmmoProviderEntry = CFDAAmmoProvider::GetProvider();
	// Provider contract validation 오류입니다.
	FString ProviderContractError;
	if (ProviderEntry == nullptr
		|| ProviderEntry != &AmmoProviderEntry
		|| !CFDATypeDispatch::ValidateProviderContract(
			Envelope,
			ProviderEntry->Descriptor,
			ProviderContractError))
	{
		// Fail-closed provider diagnostic입니다.
		const FString ProviderDiagnostic = ProviderEntry == nullptr
			? ProviderLookupError
			: (ProviderEntry != &AmmoProviderEntry
				? TEXT("Ammo current-state resolver에 등록된 typed provider가 아닙니다.")
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
	if (!CFDAAmmoProviderPrivate::ValidateTargetObjectPath(Envelope.TargetObjectPath, OutIssues))
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

	// CFAmmoData public semantic descriptor입니다.
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
			TEXT("CFAmmoData Registry identity 계약이 Required/ExplicitFName/AmmoId와 다릅니다."));
		return false;
	}

	// Read-only exact target/identity 조회에 사용할 Project Asset Registry입니다.
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	// Exact requested target path입니다.
	const FSoftObjectPath TargetObjectPath(Envelope.TargetObjectPath);
	// 이미 메모리에 존재하는 exact target UObject입니다. Target 자체의 ResolveObject는 referenced AmmoIcon validation과 별개입니다.
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
			// Exact expected target UObject semantic readback입니다. AmmoIcon referenced texture는 여기서 load하지 않습니다.
			UObject* LoadedTargetObject = ResolvedTargetObject != nullptr
				? ResolvedTargetObject
				: TargetAssetData.GetAsset();
			// Exact CFAmmoData current target입니다.
			const UCFAmmoData* CurrentAmmo = Cast<UCFAmmoData>(LoadedTargetObject);
			if (CurrentAmmo == nullptr)
			{
				CFDACommonPrimitives::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("Current.TargetObjectPath"),
					TEXT("Asset Registry에는 expected CFAmmoData target이 있지만 UObject semantic readback에 실패했습니다."));
				return false;
			}

			OutCurrentState.RequestedTargetStableLogicalId = CurrentAmmo->AmmoId;
			// Current exact target package입니다.
			const UPackage* CurrentPackage = CurrentAmmo->GetOutermost();
			OutCurrentState.bRequestedTargetDirty = CurrentPackage != nullptr && CurrentPackage->IsDirty();

			// Current persisted/loaded Ammo exact8 payload입니다.
			FCFDAAmmoPayload CurrentPayload;
			if (!ExtractPayload(*CurrentAmmo, CurrentPayload, OutIssues))
			{
				return false;
			}
			if (!CFDAAmmoProviderPrivate::ValidateAmmoIconReference(CurrentPayload, OutIssues))
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

	// Asset Registry와 loaded-but-unregistered UObject를 합친 exact class-scoped AmmoId path 집합입니다.
	TSet<FString> StableIdentityObjectPaths;
	for (TObjectIterator<UCFAmmoData> LoadedAmmoIterator; LoadedAmmoIterator; ++LoadedAmmoIterator)
	{
		// 현재 process에 살아 있는 exact CFAmmoData 후보입니다.
		const UCFAmmoData* LoadedAmmo = *LoadedAmmoIterator;
		if (LoadedAmmo == nullptr
			|| LoadedAmmo->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
			|| LoadedAmmo->AmmoId != Envelope.StableLogicalId)
		{
			continue;
		}

		// Loaded Ammo UObject exact object identity입니다.
		const FString LoadedObjectPath = FSoftObjectPath(LoadedAmmo).ToString();
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
			TEXT("Asset Registry가 아직 loading 중이라 Ammo StableIdentity current truth를 확정할 수 없습니다."));
		return false;
	}

	// Current Project exact CFAmmoData metadata filter입니다.
	FARFilter IdentityFilter;
	IdentityFilter.ClassPaths.Add(UCFAmmoData::StaticClass()->GetClassPathName());
	IdentityFilter.bRecursiveClasses = false;
	// Current exact CFAmmoData metadata 목록입니다.
	TArray<FAssetData> AmmoAssets;
	(void)AssetRegistry.GetAssets(IdentityFilter, AmmoAssets, false);
	for (const FAssetData& AmmoAssetData : AmmoAssets)
	{
		// AmmoId semantic 비교를 위해 read-only load한 exact DataAsset입니다. AmmoIcon reference는 load하지 않습니다.
		const UCFAmmoData* CandidateAmmo = Cast<UCFAmmoData>(AmmoAssetData.GetAsset());
		if (CandidateAmmo == nullptr || CandidateAmmo->AmmoId != Envelope.StableLogicalId)
		{
			continue;
		}
		StableIdentityObjectPaths.Add(AmmoAssetData.GetSoftObjectPath().ToString());
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
