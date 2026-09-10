// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStaging.cpp
// Version: v1.12.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-02 payload-free common Preview/Batch orchestration과 Missile provider-local typed compatibility facade 구현입니다.
// Changelog:
// - v1.12.0: Missile compatibility path의 TargetObjectPath와 BaseSemanticFingerprint common envelope 검증을 CFDACommonPrimitives 단일 authority로 rewire.
// - v1.11.0: provider-neutral issue/JSON/Literal FText/fingerprint primitive를 CFDACommonPrimitives authority로 rewire해 Missile accepted semantic bytes/Public API를 보존.
// - v1.10.0: Public Missile Preview row의 typed mutable integrity를 provider-local에서 검증한 뒤 payload-free common Review row로 투영하는 compatibility seam을 추가.
// - v1.9.0: shared Preview/duplicate/BatchPlanHash를 FCFDACommonPreviewRow로 이동하고 raw JSON→common/current provider operation seam을 추가. Public Missile record/payload는 provider-local facade에서만 common row로 투영하며 accepted Missile semantic/hash token 값은 보존.
// - v1.8.0: Editor Private common envelope + trusted Missile provider를 기존 Public compatibility facade 뒤에 연결하고 provider-owned StagingRoot, class-scoped StableLogicalId와 provider-first typed parse를 적용. Missile semantic/hash token 값은 보존.
// - v1.7.1: CF-FQ-050 DACE-P0-02 재검수 교정으로 fingerprint token observation sink를 thread-local + scoped RAII restoration으로 격리해 nested/concurrent dev probe에서 process-global raw state가 누수되지 않도록 보강.
// - v1.7.0: CF-FQ-050 DACE-P0-01 전용 private parser/fingerprint/extractor probe와 fingerprint actual token-label observation hook을 WITH_DEV_AUTOMATION_TESTS 범위에 추가. Production semantic bytes와 Public API는 변경하지 않음.
// - v1.6.0: P0-04 persisted FText canonicalization 의미 변경을 AdapterContractRevision 2로 승격해 revision 1 Staging/approval의 silent reinterpret를 차단.
// - v1.5.1: Product AssetDump가 확인한 NSLOCTEXT("", generated-key, source) persisted 형태를 반영해 source-backed empty-authored-namespace FText의 key를 persistence metadata로 제외하고, authored namespace만 localization 의미 경계로 유지.
// - v1.5.0: DAS-P0-04 persisted round-trip에서 UE stable localization key가 부여한 package-only namespace/key는 source-backed Literal 의미로 canonicalize하고, StringTable/explicit authored namespace/source-less FText는 계속 fail-closed하도록 교정.
// - v1.4.0: MissileGuidePreset UObject→typed whole-record extractor를 public staging service contract로 승격해 current resolver, Apply rollback, post-save persisted validation이 동일 semantic authority를 재사용하도록 정렬.
// - v1.3.0: DAS-P0-03 global preflight prerequisite로 Asset Registry에 아직 등록되지 않은 loaded /Game MissileGuidePreset까지 StableIdentity current truth에 합쳐 duplicate/TargetMoved를 fail-closed하도록 보강.
// - v1.2.1: mutable DTO가 parse 뒤 TargetObjectPath/StagingRelativePath 또는 Create/Update intent binding을 바꾸는 경우 Preview/BatchPlanHash에서 다시 structural integrity를 검증하도록 보강.
// - v1.2.0: StagingRelativePath를 canonical Authoring/DataAssetStaging/*.json authority 아래로 제한하고, Preview/BatchPlanHash가 Payload에서 StagingSemanticFingerprint를 재계산해 split DTO를 fail-closed하도록 교정.
// - v1.1.1: UE 5.8 JSON shared-key API를 public HasField/GetFieldUntyped로 교정하고, ready Asset Registry의 exact-class identity scan 0건을 정상 absent truth로 취급하도록 교정.
// - v1.1.0: Asset Registry exact target/StableIdentity read-only resolver, literal FText source validation, current duplicate identity fail-closed를 추가.
// - v1.0.0: P0-01 frozen schema/revision/identity/value/fingerprint/3-way Preview/batch duplicate/hash 의미를 구현.
// Migration:
// - 이 파일 자체는 UObject/package write API를 호출하지 않습니다. DAS-P0-03 write/save는 별도 CFDAStagingApply service가 소유하며 current resolver는 read-only truth owner를 유지합니다.
// - AdapterContractRevision 1 Staging/approval은 current FText canonicalization 의미와 다르므로 silent migration하지 않고 AdapterRevisionMismatch로 거부합니다. current authoring은 revision 2로 fresh Preview/fingerprint를 생성해야 합니다.
// - v1.7.1의 probe 격리는 WITH_DEV_AUTOMATION_TESTS 전용이며 production fingerprint byte stream, Product Apply/Save와 Public API 의미는 변경하지 않습니다.
// - v1.9.0도 FCFDAStagingRecord/FCFDAStagingService Public Missile 시그니처를 유지합니다. Shared core는 payload-free common row만 보유하며 Ammo typed payload/provider는 DAO-P0-02 이후 추가합니다.

#include "DataAuthoring/CFDAStaging.h"
#include "CFDACommonPrimitives.h"
#include "CFDAContractGuard.h"
#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFMissileGuidePresetData.h"
#include "Containers/StringConv.h"
#include "DataManagement/CFDATypeRegistry.h"
#include "Dom/JsonObject.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectIterator.h"

// Unreal의 namespace UI와 OpenSSL 전역 UI typedef 충돌을 third-party include 구간에서만 격리합니다.
#define UI CF_OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
THIRD_PARTY_INCLUDES_END
#undef UI

namespace CFDAStagingPrivate
{
	// BatchPlanHash domain kind입니다.
	static constexpr TCHAR BatchPlanKind[] = TEXT("CarFightDAStagingBatchPlan");

	// BatchPlanHash binary token format revision입니다.
	static constexpr int32 BatchPlanFormatRevision = 1;

	// blocking 또는 informational issue를 provider-neutral common authority를 통해 추가합니다.
	void AddIssue(
		TArray<FCFDAStagingIssue>& OutIssues,
		const ECFDAStagingIssueCode Code,
		const FString& FieldPath,
		const FString& Message,
		const bool bBlocking = true)
	{
		CFDACommonPrimitives::AddIssue(OutIssues, Code, FieldPath, Message, bBlocking);
	}

	// issue 배열의 blocking 여부를 provider-neutral common authority로 확인합니다.
	bool HasBlockingIssue(const TArray<FCFDAStagingIssue>& Issues)
	{
		return CFDACommonPrimitives::HasBlockingIssue(Issues);
	}

	// label/FString을 accepted provider-neutral token authority로 append합니다.
	void AppendStringToken(TArray<uint8>& OutBytes, const FString& Label, const FString& Value)
	{
		CFDACommonPrimitives::AppendStringToken(OutBytes, *Label, Value);
	}

	// bool을 accepted provider-neutral token authority로 append합니다.
	void AppendBoolToken(TArray<uint8>& OutBytes, const FString& Label, const bool bValue)
	{
		CFDACommonPrimitives::AppendBoolToken(OutBytes, *Label, bValue);
	}

	// float를 accepted provider-neutral token authority로 append합니다.
	void AppendFloatToken(TArray<uint8>& OutBytes, const FString& Label, const float Value)
	{
		CFDACommonPrimitives::AppendFloatToken(OutBytes, *Label, Value);
	}

	// FName canonicalization을 provider-neutral common authority로 위임합니다.
	FString CanonicalNameText(const FName Value)
	{
		return CFDACommonPrimitives::CanonicalNameText(Value);
	}

	// enum typed value를 exact reflected enumerator token으로 변환합니다.
	template <typename TEnum>
	FString EnumToken(const TEnum Value)
	{
		// requested enum의 reflected metadata입니다.
		const UEnum* Enum = StaticEnum<TEnum>();
		return Enum != nullptr
			? Enum->GetNameStringByValue(static_cast<int64>(Value))
			: FString();
	}

	// exact case-sensitive enumerator token을 typed enum value로 parse합니다.
	template <typename TEnum>
	bool ParseEnumToken(
		const FString& Token,
		const FString& FieldPath,
		TEnum& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// requested enum의 reflected metadata입니다.
		const UEnum* Enum = StaticEnum<TEnum>();
		if (Enum == nullptr)
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidEnumValue, FieldPath, TEXT("enum metadata를 확인할 수 없습니다."));
			return false;
		}

		// reflected enumerator 개수입니다.
		const int32 EnumCount = Enum->NumEnums();
		for (int32 EnumIndex = 0; EnumIndex < EnumCount; ++EnumIndex)
		{
			// 현재 reflected enumerator의 source token입니다.
			const FString CandidateToken = Enum->GetNameStringByIndex(EnumIndex);
			if (CandidateToken.EndsWith(TEXT("_MAX"), ESearchCase::CaseSensitive))
			{
				continue;
			}
			if (CandidateToken.Equals(Token, ESearchCase::CaseSensitive))
			{
				OutValue = static_cast<TEnum>(Enum->GetValueByIndex(EnumIndex));
				return true;
			}
		}

		AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidEnumValue,
			FieldPath,
			FString::Printf(TEXT("지원하지 않는 enum token입니다: %s"), *Token));
		return false;
	}

	// SHA-256 canonical 형식 검사를 provider-neutral common authority로 위임합니다.
	bool IsCanonicalSha256Fingerprint(const FString& Fingerprint)
	{
		return CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Fingerprint);
	}

	// canonical byte stream hash를 provider-neutral common authority로 위임합니다.
	bool HashCanonicalBytes(const TArray<uint8>& Bytes, FString& OutFingerprint, FString& OutError)
	{
		return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
	}

	// JSON exact-field 검사를 provider-neutral common authority로 위임합니다.
	bool ValidateExactFields(
		const TSharedPtr<FJsonObject>& Object,
		const TArray<FString>& RequiredFields,
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ValidateExactFields(Object, RequiredFields, ObjectPath, OutIssues);
	}

	// JSON exact field/type 검사를 provider-neutral common authority로 위임합니다.
	TSharedPtr<FJsonValue> RequireField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		const EJson ExpectedType,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::RequireField(Object, FieldName, FieldPath, ExpectedType, OutIssues);
	}

	// required JSON string parsing을 provider-neutral common authority로 위임합니다.
	bool ParseStringField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		FString& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ParseStringField(Object, FieldName, FieldPath, OutValue, OutIssues);
	}

	// required JSON revision parsing을 provider-neutral common authority로 위임합니다.
	bool ParseRevisionField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		int32& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ParseRevisionField(Object, FieldName, FieldPath, OutValue, OutIssues);
	}

	// required JSON bool parsing을 provider-neutral common authority로 위임합니다.
	bool ParseBoolField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		bool& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ParseBoolField(Object, FieldName, FieldPath, OutValue, OutIssues);
	}

	// required JSON finite float/range parsing을 provider-neutral common authority로 위임합니다.
	bool ParseFloatField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		const double MinimumValue,
		const double MaximumValue,
		float& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ParseFloatField(Object, FieldName, FieldPath, MinimumValue, MaximumValue, OutValue, OutIssues);
	}

	// required JSON object parsing을 provider-neutral common authority로 위임합니다.
	bool ParseObjectField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		TSharedPtr<FJsonObject>& OutObject,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ParseObjectField(Object, FieldName, FieldPath, OutObject, OutIssues);
	}

	// Literal FText parsing을 provider-neutral common authority로 위임합니다.
	bool ParseLiteralText(
		const TSharedPtr<FJsonObject>& TextObject,
		const FString& FieldPath,
		FCFDAStagingLiteralText& OutText,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ParseLiteralText(TextObject, FieldPath, OutText, OutIssues);
	}

	// exact full `/Game/.../Asset.Asset` target object path 검증을 provider-neutral common authority로 위임합니다.
	bool ValidateTargetObjectPath(const FString& ObjectPath, TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ValidateTargetObjectPath(ObjectPath, OutIssues);
	}

	// repository-relative Missile Staging path를 provider-owned canonical root 아래의 slash-normalized JSON path로 변환합니다.
	bool NormalizeStagingRelativePath(const FString& InputPath, FString& OutPath)
	{
		// Current compatibility facade가 사용하는 trusted Missile provider입니다.
		const FCFDATypeProvider& MissileProvider = CFDATypeDispatch::GetMissilePresetProvider();
		return CFDATypeDispatch::NormalizeProviderStagingPath(MissileProvider, InputPath, OutPath);
	}

	// Config enum string field를 strict typed enum으로 parse합니다.
	template <typename TEnum>
	bool ParseEnumField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		TEnum& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// JSON enum source token입니다.
		FString EnumText;
		if (!ParseStringField(Object, FieldName, FieldPath, EnumText, OutIssues))
		{
			return false;
		}
		return ParseEnumToken(EnumText, FieldPath, OutValue, OutIssues);
	}

	// MissileGuideConfig strict whole-record object를 typed struct로 parse합니다.
	bool ParseMissileGuideConfig(
		const TSharedPtr<FJsonObject>& ConfigObject,
		FCFMissileGuideConfig& OutConfig,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// MissileGuideConfig의 exact P0 writable field set입니다.
		const TArray<FString> RequiredFields =
		{
			TEXT("bUseGuidance"),
			TEXT("GuideMode"),
			TEXT("LostTargetPolicy"),
			TEXT("NavigationConstant"),
			TEXT("MaximumTurnRateDegPerSec"),
			TEXT("MaximumLateralAccelerationCmPerSecSq"),
			TEXT("GuidanceResponseTimeSeconds"),
			TEXT("MinimumGuidanceSpeedCmPerSec"),
			TEXT("SeekerFieldOfViewDeg"),
			TEXT("LockBreakAngleDeg"),
			TEXT("TargetLostGraceTimeSeconds"),
			TEXT("SeekerModel"),
			TEXT("TargetObservationMode"),
			TEXT("GuidanceLaw"),
			TEXT("GuidanceActivationMode"),
			TEXT("GuidanceActivationDelaySeconds"),
			TEXT("GuidanceActivationDistanceCm"),
			TEXT("LeadTimeSeconds"),
			TEXT("MaxLeadDistanceCm"),
			TEXT("ReacquisitionMode"),
			TEXT("TargetObservationIntervalSeconds"),
			TEXT("TargetVelocityEstimateResponseTimeSeconds"),
			TEXT("AcquisitionConeHalfAngleDeg"),
			TEXT("TrackingConeHalfAngleDeg"),
			TEXT("ReacquisitionConeHalfAngleDeg"),
			TEXT("ReacquisitionTimeSeconds")
		};
		ValidateExactFields(ConfigObject, RequiredFields, TEXT("Payload.MissileGuideConfig"), OutIssues);

		ParseBoolField(ConfigObject, TEXT("bUseGuidance"), TEXT("Payload.MissileGuideConfig.bUseGuidance"), OutConfig.bUseGuidance, OutIssues);
		ParseEnumField(ConfigObject, TEXT("GuideMode"), TEXT("Payload.MissileGuideConfig.GuideMode"), OutConfig.GuideMode, OutIssues);
		ParseEnumField(ConfigObject, TEXT("LostTargetPolicy"), TEXT("Payload.MissileGuideConfig.LostTargetPolicy"), OutConfig.LostTargetPolicy, OutIssues);
		ParseFloatField(ConfigObject, TEXT("NavigationConstant"), TEXT("Payload.MissileGuideConfig.NavigationConstant"), 0.0, 10.0, OutConfig.NavigationConstant, OutIssues);
		ParseFloatField(ConfigObject, TEXT("MaximumTurnRateDegPerSec"), TEXT("Payload.MissileGuideConfig.MaximumTurnRateDegPerSec"), 0.0, 720.0, OutConfig.MaximumTurnRateDegPerSec, OutIssues);
		ParseFloatField(ConfigObject, TEXT("MaximumLateralAccelerationCmPerSecSq"), TEXT("Payload.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq"), 0.0, 1000000.0, OutConfig.MaximumLateralAccelerationCmPerSecSq, OutIssues);
		ParseFloatField(ConfigObject, TEXT("GuidanceResponseTimeSeconds"), TEXT("Payload.MissileGuideConfig.GuidanceResponseTimeSeconds"), 0.001, 10.0, OutConfig.GuidanceResponseTimeSeconds, OutIssues);
		ParseFloatField(ConfigObject, TEXT("MinimumGuidanceSpeedCmPerSec"), TEXT("Payload.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec"), 0.0, 1000000.0, OutConfig.MinimumGuidanceSpeedCmPerSec, OutIssues);
		ParseFloatField(ConfigObject, TEXT("SeekerFieldOfViewDeg"), TEXT("Payload.MissileGuideConfig.SeekerFieldOfViewDeg"), 0.0, 360.0, OutConfig.SeekerFieldOfViewDeg, OutIssues);
		ParseFloatField(ConfigObject, TEXT("LockBreakAngleDeg"), TEXT("Payload.MissileGuideConfig.LockBreakAngleDeg"), 0.0, 180.0, OutConfig.LockBreakAngleDeg, OutIssues);
		ParseFloatField(ConfigObject, TEXT("TargetLostGraceTimeSeconds"), TEXT("Payload.MissileGuideConfig.TargetLostGraceTimeSeconds"), 0.0, 30.0, OutConfig.TargetLostGraceTimeSeconds, OutIssues);
		ParseEnumField(ConfigObject, TEXT("SeekerModel"), TEXT("Payload.MissileGuideConfig.SeekerModel"), OutConfig.SeekerModel, OutIssues);
		ParseEnumField(ConfigObject, TEXT("TargetObservationMode"), TEXT("Payload.MissileGuideConfig.TargetObservationMode"), OutConfig.TargetObservationMode, OutIssues);
		ParseEnumField(ConfigObject, TEXT("GuidanceLaw"), TEXT("Payload.MissileGuideConfig.GuidanceLaw"), OutConfig.GuidanceLaw, OutIssues);
		ParseEnumField(ConfigObject, TEXT("GuidanceActivationMode"), TEXT("Payload.MissileGuideConfig.GuidanceActivationMode"), OutConfig.GuidanceActivationMode, OutIssues);
		ParseFloatField(ConfigObject, TEXT("GuidanceActivationDelaySeconds"), TEXT("Payload.MissileGuideConfig.GuidanceActivationDelaySeconds"), 0.0, 30.0, OutConfig.GuidanceActivationDelaySeconds, OutIssues);
		ParseFloatField(ConfigObject, TEXT("GuidanceActivationDistanceCm"), TEXT("Payload.MissileGuideConfig.GuidanceActivationDistanceCm"), 0.0, 1000000.0, OutConfig.GuidanceActivationDistanceCm, OutIssues);
		ParseFloatField(ConfigObject, TEXT("LeadTimeSeconds"), TEXT("Payload.MissileGuideConfig.LeadTimeSeconds"), 0.0, 10.0, OutConfig.LeadTimeSeconds, OutIssues);
		ParseFloatField(ConfigObject, TEXT("MaxLeadDistanceCm"), TEXT("Payload.MissileGuideConfig.MaxLeadDistanceCm"), 0.0, 1000000.0, OutConfig.MaxLeadDistanceCm, OutIssues);
		ParseEnumField(ConfigObject, TEXT("ReacquisitionMode"), TEXT("Payload.MissileGuideConfig.ReacquisitionMode"), OutConfig.ReacquisitionMode, OutIssues);
		ParseFloatField(ConfigObject, TEXT("TargetObservationIntervalSeconds"), TEXT("Payload.MissileGuideConfig.TargetObservationIntervalSeconds"), 0.001, 10.0, OutConfig.TargetObservationIntervalSeconds, OutIssues);
		ParseFloatField(ConfigObject, TEXT("TargetVelocityEstimateResponseTimeSeconds"), TEXT("Payload.MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds"), 0.001, 10.0, OutConfig.TargetVelocityEstimateResponseTimeSeconds, OutIssues);
		ParseFloatField(ConfigObject, TEXT("AcquisitionConeHalfAngleDeg"), TEXT("Payload.MissileGuideConfig.AcquisitionConeHalfAngleDeg"), 0.0, 180.0, OutConfig.AcquisitionConeHalfAngleDeg, OutIssues);
		ParseFloatField(ConfigObject, TEXT("TrackingConeHalfAngleDeg"), TEXT("Payload.MissileGuideConfig.TrackingConeHalfAngleDeg"), 0.0, 180.0, OutConfig.TrackingConeHalfAngleDeg, OutIssues);
		ParseFloatField(ConfigObject, TEXT("ReacquisitionConeHalfAngleDeg"), TEXT("Payload.MissileGuideConfig.ReacquisitionConeHalfAngleDeg"), 0.0, 180.0, OutConfig.ReacquisitionConeHalfAngleDeg, OutIssues);
		ParseFloatField(ConfigObject, TEXT("ReacquisitionTimeSeconds"), TEXT("Payload.MissileGuideConfig.ReacquisitionTimeSeconds"), 0.0, 30.0, OutConfig.ReacquisitionTimeSeconds, OutIssues);
		return !HasBlockingIssue(OutIssues);
	}

	// MissileGuidePreset Payload strict whole-record object를 typed DTO로 parse합니다.
	bool ParseMissilePresetPayload(
		const TSharedPtr<FJsonObject>& PayloadObject,
		FCFDAMissilePresetPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// MissileGuidePreset whole-record exact writable field set입니다.
		const TArray<FString> RequiredFields =
		{
			TEXT("PresetId"),
			TEXT("PresetDisplayName"),
			TEXT("PresetDescription"),
			TEXT("MissileGuideConfig")
		};
		ValidateExactFields(PayloadObject, RequiredFields, TEXT("Payload"), OutIssues);

		// payload PresetId source text입니다.
		FString PresetIdText;
		if (ParseStringField(PayloadObject, TEXT("PresetId"), TEXT("Payload.PresetId"), PresetIdText, OutIssues))
		{
			OutPayload.PresetId = FName(*PresetIdText);
			if (OutPayload.PresetId.IsNone())
			{
				AddIssue(OutIssues, ECFDAStagingIssueCode::StableIdentityMissing, TEXT("Payload.PresetId"), TEXT("Required identity PresetId는 NAME_None/empty일 수 없습니다."));
			}
		}

		// PresetDisplayName JSON object입니다.
		TSharedPtr<FJsonObject> DisplayNameObject;
		if (ParseObjectField(PayloadObject, TEXT("PresetDisplayName"), TEXT("Payload.PresetDisplayName"), DisplayNameObject, OutIssues))
		{
			ParseLiteralText(DisplayNameObject, TEXT("Payload.PresetDisplayName"), OutPayload.PresetDisplayName, OutIssues);
		}

		// PresetDescription JSON object입니다.
		TSharedPtr<FJsonObject> DescriptionObject;
		if (ParseObjectField(PayloadObject, TEXT("PresetDescription"), TEXT("Payload.PresetDescription"), DescriptionObject, OutIssues))
		{
			ParseLiteralText(DescriptionObject, TEXT("Payload.PresetDescription"), OutPayload.PresetDescription, OutIssues);
		}

		// nested MissileGuideConfig JSON object입니다.
		TSharedPtr<FJsonObject> ConfigObject;
		if (ParseObjectField(PayloadObject, TEXT("MissileGuideConfig"), TEXT("Payload.MissileGuideConfig"), ConfigObject, OutIssues))
		{
			ParseMissileGuideConfig(ConfigObject, OutPayload.MissileGuideConfig, OutIssues);
		}
		return !HasBlockingIssue(OutIssues);
	}

	// fingerprint authority에 들어오는 typed MissileGuidePreset payload도 raw authored schema 범위를 만족하는지 검증합니다.
	bool ValidateTypedMissilePresetPayload(const FCFDAMissilePresetPayload& Payload, FString& OutError)
	{
		if (Payload.PresetId.IsNone())
		{
			OutError = TEXT("PresetId가 NAME_None입니다.");
			return false;
		}

		// fingerprint 대상 authored config입니다.
		const FCFMissileGuideConfig& Config = Payload.MissileGuideConfig;
		if (EnumToken(Config.GuideMode).IsEmpty()
			|| EnumToken(Config.LostTargetPolicy).IsEmpty()
			|| EnumToken(Config.SeekerModel).IsEmpty()
			|| EnumToken(Config.TargetObservationMode).IsEmpty()
			|| EnumToken(Config.GuidanceLaw).IsEmpty()
			|| EnumToken(Config.GuidanceActivationMode).IsEmpty()
			|| EnumToken(Config.ReacquisitionMode).IsEmpty())
		{
			OutError = TEXT("typed MissileGuideConfig에 지원하지 않는 enum 값이 있습니다.");
			return false;
		}

		// 한 authored float가 finite하고 frozen schema 범위 안인지 검사하는 local helper입니다.
		auto IsInRange = [](const float Value, const float MinimumValue, const float MaximumValue)
		{
			return FMath::IsFinite(Value) && Value >= MinimumValue && Value <= MaximumValue;
		};
		if (!IsInRange(Config.NavigationConstant, 0.0f, 10.0f)
			|| !IsInRange(Config.MaximumTurnRateDegPerSec, 0.0f, 720.0f)
			|| !IsInRange(Config.MaximumLateralAccelerationCmPerSecSq, 0.0f, 1000000.0f)
			|| !IsInRange(Config.GuidanceResponseTimeSeconds, 0.001f, 10.0f)
			|| !IsInRange(Config.MinimumGuidanceSpeedCmPerSec, 0.0f, 1000000.0f)
			|| !IsInRange(Config.SeekerFieldOfViewDeg, 0.0f, 360.0f)
			|| !IsInRange(Config.LockBreakAngleDeg, 0.0f, 180.0f)
			|| !IsInRange(Config.TargetLostGraceTimeSeconds, 0.0f, 30.0f)
			|| !IsInRange(Config.GuidanceActivationDelaySeconds, 0.0f, 30.0f)
			|| !IsInRange(Config.GuidanceActivationDistanceCm, 0.0f, 1000000.0f)
			|| !IsInRange(Config.LeadTimeSeconds, 0.0f, 10.0f)
			|| !IsInRange(Config.MaxLeadDistanceCm, 0.0f, 1000000.0f)
			|| !IsInRange(Config.TargetObservationIntervalSeconds, 0.001f, 10.0f)
			|| !IsInRange(Config.TargetVelocityEstimateResponseTimeSeconds, 0.001f, 10.0f)
			|| !IsInRange(Config.AcquisitionConeHalfAngleDeg, 0.0f, 180.0f)
			|| !IsInRange(Config.TrackingConeHalfAngleDeg, 0.0f, 180.0f)
			|| !IsInRange(Config.ReacquisitionConeHalfAngleDeg, 0.0f, 180.0f)
			|| !IsInRange(Config.ReacquisitionTimeSeconds, 0.0f, 30.0f))
		{
			OutError = TEXT("typed MissileGuideConfig의 raw authored float가 frozen schema 범위를 벗어나거나 finite 값이 아닙니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// persisted Literal FText readback을 provider-neutral common authority로 위임합니다.
	bool ReadLiteralTextFromAsset(
		const FText& SourceText,
		const FString& FieldPath,
		FCFDAStagingLiteralText& OutText,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		return CFDACommonPrimitives::ReadLiteralTextFromAsset(SourceText, FieldPath, OutText, OutIssues);
	}

	// persisted MissileGuidePreset UObject를 fingerprint와 동일한 typed whole-record payload로 read-only 변환합니다.
	bool BuildMissilePresetPayloadFromAsset(
		const UCFMissileGuidePresetData& PresetAsset,
		FCFDAMissilePresetPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutPayload = FCFDAMissilePresetPayload();
		OutPayload.PresetId = PresetAsset.PresetId;
		if (OutPayload.PresetId.IsNone())
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::StableIdentityMissing,
				TEXT("Current.Payload.PresetId"),
				TEXT("current CFMissileGuidePresetData의 required PresetId가 NAME_None입니다."));
		}

		ReadLiteralTextFromAsset(
			PresetAsset.PresetDisplayName,
			TEXT("Current.Payload.PresetDisplayName"),
			OutPayload.PresetDisplayName,
			OutIssues);
		ReadLiteralTextFromAsset(
			PresetAsset.PresetDescription,
			TEXT("Current.Payload.PresetDescription"),
			OutPayload.PresetDescription,
			OutIssues);
		OutPayload.MissileGuideConfig = PresetAsset.MissileGuideConfig;

		if (!HasBlockingIssue(OutIssues))
		{
			// current persisted typed payload가 frozen authored range를 만족하는지 확인할 오류입니다.
			FString TypedPayloadError;
			if (!ValidateTypedMissilePresetPayload(OutPayload, TypedPayloadError))
			{
				AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("Current.Payload"),
					TypedPayloadError);
			}
		}
		return !HasBlockingIssue(OutIssues);
	}

	// typed MissileGuidePreset payload의 exact semantic token stream을 작성합니다.
	void AppendMissilePresetPayloadTokens(TArray<uint8>& OutBytes, const FCFDAMissilePresetPayload& Payload)
	{
		// Accepted Missile semantic header authority입니다.
		const FCFDATypeProvider& MissileProvider = CFDATypeDispatch::GetMissilePresetProvider();
		AppendStringToken(OutBytes, TEXT("SchemaId"), MissileProvider.TypeKey.SchemaId);
		AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(MissileProvider.SchemaRevision));
		AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(MissileProvider.AdapterContractRevision));
		AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), MissileProvider.TypeKey.DataAssetTypeClassPath);
		AppendStringToken(OutBytes, TEXT("Payload.PresetId"), CanonicalNameText(Payload.PresetId));
		AppendStringToken(OutBytes, TEXT("Payload.PresetDisplayName"), Payload.PresetDisplayName.Text);
		AppendStringToken(OutBytes, TEXT("Payload.PresetDescription"), Payload.PresetDescription.Text);

		// shorter alias for the authored config being canonicalized입니다.
		const FCFMissileGuideConfig& Config = Payload.MissileGuideConfig;
		AppendBoolToken(OutBytes, TEXT("Config.bUseGuidance"), Config.bUseGuidance);
		AppendStringToken(OutBytes, TEXT("Config.GuideMode"), EnumToken(Config.GuideMode));
		AppendStringToken(OutBytes, TEXT("Config.LostTargetPolicy"), EnumToken(Config.LostTargetPolicy));
		AppendFloatToken(OutBytes, TEXT("Config.NavigationConstant"), Config.NavigationConstant);
		AppendFloatToken(OutBytes, TEXT("Config.MaximumTurnRateDegPerSec"), Config.MaximumTurnRateDegPerSec);
		AppendFloatToken(OutBytes, TEXT("Config.MaximumLateralAccelerationCmPerSecSq"), Config.MaximumLateralAccelerationCmPerSecSq);
		AppendFloatToken(OutBytes, TEXT("Config.GuidanceResponseTimeSeconds"), Config.GuidanceResponseTimeSeconds);
		AppendFloatToken(OutBytes, TEXT("Config.MinimumGuidanceSpeedCmPerSec"), Config.MinimumGuidanceSpeedCmPerSec);
		AppendFloatToken(OutBytes, TEXT("Config.SeekerFieldOfViewDeg"), Config.SeekerFieldOfViewDeg);
		AppendFloatToken(OutBytes, TEXT("Config.LockBreakAngleDeg"), Config.LockBreakAngleDeg);
		AppendFloatToken(OutBytes, TEXT("Config.TargetLostGraceTimeSeconds"), Config.TargetLostGraceTimeSeconds);
		AppendStringToken(OutBytes, TEXT("Config.SeekerModel"), EnumToken(Config.SeekerModel));
		AppendStringToken(OutBytes, TEXT("Config.TargetObservationMode"), EnumToken(Config.TargetObservationMode));
		AppendStringToken(OutBytes, TEXT("Config.GuidanceLaw"), EnumToken(Config.GuidanceLaw));
		AppendStringToken(OutBytes, TEXT("Config.GuidanceActivationMode"), EnumToken(Config.GuidanceActivationMode));
		AppendFloatToken(OutBytes, TEXT("Config.GuidanceActivationDelaySeconds"), Config.GuidanceActivationDelaySeconds);
		AppendFloatToken(OutBytes, TEXT("Config.GuidanceActivationDistanceCm"), Config.GuidanceActivationDistanceCm);
		AppendFloatToken(OutBytes, TEXT("Config.LeadTimeSeconds"), Config.LeadTimeSeconds);
		AppendFloatToken(OutBytes, TEXT("Config.MaxLeadDistanceCm"), Config.MaxLeadDistanceCm);
		AppendStringToken(OutBytes, TEXT("Config.ReacquisitionMode"), EnumToken(Config.ReacquisitionMode));
		AppendFloatToken(OutBytes, TEXT("Config.TargetObservationIntervalSeconds"), Config.TargetObservationIntervalSeconds);
		AppendFloatToken(OutBytes, TEXT("Config.TargetVelocityEstimateResponseTimeSeconds"), Config.TargetVelocityEstimateResponseTimeSeconds);
		AppendFloatToken(OutBytes, TEXT("Config.AcquisitionConeHalfAngleDeg"), Config.AcquisitionConeHalfAngleDeg);
		AppendFloatToken(OutBytes, TEXT("Config.TrackingConeHalfAngleDeg"), Config.TrackingConeHalfAngleDeg);
		AppendFloatToken(OutBytes, TEXT("Config.ReacquisitionConeHalfAngleDeg"), Config.ReacquisitionConeHalfAngleDeg);
		AppendFloatToken(OutBytes, TEXT("Config.ReacquisitionTimeSeconds"), Config.ReacquisitionTimeSeconds);
	}

	// record Payload와 cached StagingSemanticFingerprint가 동일 semantic state인지 재계산해 검증합니다.
	bool ValidateStagingFingerprintIntegrity(const FCFDAStagingRecord& Record, FString& OutError)
	{
		// current typed Payload에서 새로 계산한 semantic fingerprint입니다.
		FString RecomputedFingerprint;
		// fingerprint 재계산 실패 원인입니다.
		FString FingerprintError;
		if (!FCFDAStagingService::BuildSemanticFingerprint(Record.Payload, RecomputedFingerprint, FingerprintError))
		{
			OutError = FString::Printf(TEXT("Staging Payload semantic fingerprint 재계산에 실패했습니다: %s"), *FingerprintError);
			return false;
		}
		return CFDACommonPrimitives::ValidateCachedSemanticFingerprint(
			Record.StagingSemanticFingerprint,
			RecomputedFingerprint,
			OutError);
	}

	// Public Missile facade Create/Update candidate의 typed payload와 common binding을 다시 검증합니다.
	bool ValidateMissileBatchCandidateIntegrity(const FCFDAStagingPreviewRow& Row, FString& OutError)
	{
		// Candidate exact TypeKey에 등록된 complete provider entry입니다.
		const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
			Row.Record.SchemaId,
			Row.Record.DataAssetTypeClassPath,
			&OutError);
		// Public Missile facade가 허용하는 exact first provider entry입니다.
		const FCFDATypeProviderEntry& MissileProviderEntry = CFDATypeDispatch::GetMissilePresetProviderEntry();
		if (ProviderEntry == nullptr || ProviderEntry != &MissileProviderEntry)
		{
			OutError = TEXT("Batch candidate가 Missile compatibility facade의 exact trusted provider가 아닙니다.");
			return false;
		}

		// Provider-local typed record projection으로 만든 payload-free common orchestration envelope입니다.
		const FCFDACommonEnvelope Envelope = CFDAMissileProviderImpl::BuildCommonEnvelope(
			Row.Record,
			Row.CurrentSemanticFingerprint,
			Row.Kind);
		if (!CFDATypeDispatch::ValidateProviderContract(Envelope, ProviderEntry->Descriptor, OutError))
		{
			return false;
		}
		if (Row.Record.StableLogicalId.IsNone()
			|| Row.Record.Payload.PresetId.IsNone()
			|| Row.Record.StableLogicalId != Row.Record.Payload.PresetId)
		{
			OutError = TEXT("Batch candidate의 StableLogicalId와 Payload.PresetId identity 계약이 유효하지 않습니다.");
			return false;
		}

		// Mutable target path를 canonical Unreal object path 규칙으로 재검증할 임시 진단입니다.
		TArray<FCFDAStagingIssue> TargetPathIssues;
		if (!ValidateTargetObjectPath(Row.Record.TargetObjectPath, TargetPathIssues))
		{
			OutError = TEXT("Batch candidate의 TargetObjectPath가 canonical Unreal asset object path가 아닙니다.");
			return false;
		}

		// Mutable Payload와 cached desired-state fingerprint가 같은 semantic state인지 확인합니다.
		FString StagingFingerprintIntegrityError;
		if (!ValidateStagingFingerprintIntegrity(Row.Record, StagingFingerprintIntegrityError))
		{
			OutError = StagingFingerprintIntegrityError;
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Preview kind를 BatchPlanHash canonical token으로 변환합니다.
	FString PreviewKindToken(const ECFDAStagingPreviewKind Kind)
	{
		switch (Kind)
		{
		case ECFDAStagingPreviewKind::Create:
			return TEXT("Create");
		case ECFDAStagingPreviewKind::Update:
			return TEXT("Update");
		case ECFDAStagingPreviewKind::NoChange:
			return TEXT("NoChange");
		case ECFDAStagingPreviewKind::Conflict:
			return TEXT("Conflict");
		case ECFDAStagingPreviewKind::Invalid:
		default:
			return TEXT("Invalid");
		}
	}

	// exact BatchPlanHash candidate sorting key를 payload-free common row 기준으로 만듭니다.
	FString BuildBatchSortKey(const FCFDACommonPreviewRow& Row)
	{
		return Row.Envelope.DataAssetTypeClassPath.ToLower()
			+ TEXT("\n") + CanonicalNameText(Row.Envelope.StableLogicalId)
			+ TEXT("\n") + Row.Envelope.TargetObjectPath.ToLower()
			+ TEXT("\n") + Row.Envelope.StagingRelativePath.ToLower();
	}

	// 한 Create/Update common row의 payload-free envelope를 accepted BatchPlan token stream에 append합니다.
	void AppendBatchTargetTokens(TArray<uint8>& OutBytes, const FCFDACommonPreviewRow& Row)
	{
		// Existing approval hash bytes와 1:1로 대응하는 payload-free common envelope입니다.
		const FCFDACommonEnvelope& Envelope = Row.Envelope;
		AppendStringToken(OutBytes, TEXT("SchemaId"), Envelope.SchemaId);
		AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(Envelope.SchemaRevision));
		AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(Envelope.AdapterContractRevision));
		AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), Envelope.DataAssetTypeClassPath);
		AppendStringToken(OutBytes, TEXT("IdentityPolicy"), TEXT("Required"));
		AppendStringToken(OutBytes, TEXT("StableLogicalId"), CanonicalNameText(Envelope.StableLogicalId));
		AppendStringToken(OutBytes, TEXT("TargetObjectPath"), Envelope.TargetObjectPath);
		AppendStringToken(OutBytes, TEXT("StagingRelativePath"), Envelope.StagingRelativePath);
		AppendStringToken(OutBytes, TEXT("BaseSemanticFingerprint"), Envelope.bHasBaseSemanticFingerprint ? Envelope.BaseSemanticFingerprint : TEXT("<none>"));
		AppendStringToken(OutBytes, TEXT("CurrentSemanticFingerprint"), Envelope.CurrentSemanticFingerprint.IsEmpty() ? TEXT("<absent-target>") : Envelope.CurrentSemanticFingerprint);
		AppendStringToken(OutBytes, TEXT("StagingSemanticFingerprint"), Envelope.StagingSemanticFingerprint);
		AppendStringToken(OutBytes, TEXT("PlannedOperation"), PreviewKindToken(Envelope.PlannedOperation));
	}
}

// P0 MissileGuidePreset schema identity를 compatibility facade로 반환합니다.
const TCHAR* FCFDAStagingService::GetMissilePresetSchemaId()
{
	return *CFDATypeDispatch::GetMissilePresetProvider().TypeKey.SchemaId;
}

// P0 MissileGuidePreset JSON shape revision을 compatibility facade로 반환합니다.
int32 FCFDAStagingService::GetMissilePresetSchemaRevision()
{
	return CFDATypeDispatch::GetMissilePresetProvider().SchemaRevision;
}

// P0 MissileGuidePreset typed adapter semantic revision을 compatibility facade로 반환합니다.
int32 FCFDAStagingService::GetMissilePresetAdapterRevision()
{
	return CFDATypeDispatch::GetMissilePresetProvider().AdapterContractRevision;
}

// P0 MissileGuidePreset exact native class path를 compatibility facade로 반환합니다.
const TCHAR* FCFDAStagingService::GetMissilePresetClassPath()
{
	return *CFDATypeDispatch::GetMissilePresetProvider().TypeKey.DataAssetTypeClassPath;
}

// Existing Missile typed record를 payload-free common envelope로 투영하는 provider-local adapter입니다.
FCFDACommonEnvelope CFDAMissileProviderImpl::BuildCommonEnvelope(
	const FCFDAStagingRecord& Record,
	const FString& CurrentSemanticFingerprint,
	const ECFDAStagingPreviewKind PlannedOperation)
{
	// Shared orchestration으로 전달할 payload-free common envelope입니다.
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

// Existing Public Missile Preview row의 typed mutable integrity를 검증한 뒤 payload-free common row로 투영합니다.
bool CFDAMissileProviderImpl::ProjectCompatibilityPreviewRow(
	const FCFDAStagingPreviewRow& PreviewRow,
	FCFDACommonPreviewRow& OutCommonRow,
	FString& OutError)
{
	OutCommonRow = FCFDACommonPreviewRow();
	OutError.Reset();

	if (PreviewRow.Kind == ECFDAStagingPreviewKind::Create
		|| PreviewRow.Kind == ECFDAStagingPreviewKind::Update)
	{
		if (!CFDAStagingPrivate::ValidateMissileBatchCandidateIntegrity(PreviewRow, OutError))
		{
			return false;
		}
	}

	OutCommonRow.Kind = PreviewRow.Kind;
	OutCommonRow.Envelope = BuildCommonEnvelope(
		PreviewRow.Record,
		PreviewRow.CurrentSemanticFingerprint,
		PreviewRow.Kind);
	OutCommonRow.Issues = PreviewRow.Issues;
	return true;
}

// Raw JSON을 typed Missile parser로 검증한 뒤 payload-free common candidate만 shared core에 반환합니다.
bool CFDAMissileProviderImpl::ParseCommonCandidate(
	const FString& JsonText,
	const FString& StagingRelativePath,
	FCFDACommonEnvelope& OutEnvelope,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Provider-local strict typed parse 결과입니다.
	const FCFDAStagingParseResult ParseResult = ParseJson(JsonText, StagingRelativePath);
	OutIssues = ParseResult.Issues;
	if (!ParseResult.bValid)
	{
		OutEnvelope = FCFDACommonEnvelope();
		return false;
	}

	OutEnvelope = BuildCommonEnvelope(ParseResult.Record);
	return true;
}

// strict whole-record JSON을 typed MissileGuidePreset record로 parse/canonicalize합니다.
FCFDAStagingParseResult CFDAMissileProviderImpl::ParseJson(
	const FString& JsonText,
	const FString& StagingRelativePath)
{
	// parse/validation 결과입니다.
	FCFDAStagingParseResult Result;
	// JSON text reader입니다.
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	// JSON root object입니다.
	TSharedPtr<FJsonObject> RootObject;
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::MalformedJson, TEXT("$"), TEXT("유효한 JSON object를 parse할 수 없습니다."));
		return Result;
	}

	// top-level exact whole-record field set입니다.
	const TArray<FString> TopLevelFields =
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
	CFDAStagingPrivate::ValidateExactFields(RootObject, TopLevelFields, FString(), Result.Issues);

	CFDAStagingPrivate::ParseStringField(RootObject, TEXT("SchemaId"), TEXT("SchemaId"), Result.Record.SchemaId, Result.Issues);
	CFDAStagingPrivate::ParseRevisionField(RootObject, TEXT("SchemaRevision"), TEXT("SchemaRevision"), Result.Record.SchemaRevision, Result.Issues);
	CFDAStagingPrivate::ParseRevisionField(RootObject, TEXT("AdapterContractRevision"), TEXT("AdapterContractRevision"), Result.Record.AdapterContractRevision, Result.Issues);
	CFDAStagingPrivate::ParseStringField(RootObject, TEXT("DataAssetTypeClassPath"), TEXT("DataAssetTypeClassPath"), Result.Record.DataAssetTypeClassPath, Result.Issues);

	// top-level StableLogicalId source text입니다.
	FString StableLogicalIdText;
	if (CFDAStagingPrivate::ParseStringField(RootObject, TEXT("StableLogicalId"), TEXT("StableLogicalId"), StableLogicalIdText, Result.Issues))
	{
		Result.Record.StableLogicalId = FName(*StableLogicalIdText);
		if (Result.Record.StableLogicalId.IsNone())
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::StableIdentityMissing, TEXT("StableLogicalId"), TEXT("Required StableLogicalId는 NAME_None/empty일 수 없습니다."));
		}
	}

	CFDAStagingPrivate::ParseStringField(RootObject, TEXT("TargetObjectPath"), TEXT("TargetObjectPath"), Result.Record.TargetObjectPath, Result.Issues);
	if (!Result.Record.TargetObjectPath.IsEmpty())
	{
		CFDAStagingPrivate::ValidateTargetObjectPath(Result.Record.TargetObjectPath, Result.Issues);
	}

	// BaseSemanticFingerprint common envelope physical contract를 provider-neutral authority로 parse합니다.
	CFDACommonPrimitives::ParseBaseSemanticFingerprint(
		RootObject,
		Result.Record.bHasBaseSemanticFingerprint,
		Result.Record.BaseSemanticFingerprint,
		Result.Issues);

	// Common fields가 선택한 exact complete trusted provider entry입니다. JSON/DTO는 provider implementation을 직접 지정하지 못합니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* SelectedProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		Result.Record.SchemaId,
		Result.Record.DataAssetTypeClassPath,
		&ProviderLookupError);
	// Missile compatibility parser가 허용하는 exact first provider entry입니다.
	const FCFDATypeProviderEntry& MissileProviderEntry = CFDATypeDispatch::GetMissilePresetProviderEntry();
	if (SelectedProviderEntry == nullptr || SelectedProviderEntry != &MissileProviderEntry)
	{
		CFDAStagingPrivate::AddIssue(
			Result.Issues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderLookupError.IsEmpty() ? TEXT("Missile compatibility parser에 등록된 exact trusted TypeKey provider가 아닙니다.") : ProviderLookupError);
	}
	else
	{
		// Selected complete entry의 structural descriptor입니다.
		const FCFDATypeProvider& SelectedProvider = SelectedProviderEntry->Descriptor;
		if (Result.Record.SchemaRevision != SelectedProvider.SchemaRevision)
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported, TEXT("SchemaRevision"), TEXT("Selected provider와 정확히 같은 SchemaRevision만 지원합니다."));
		}
		if (Result.Record.AdapterContractRevision != SelectedProvider.AdapterContractRevision)
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::AdapterRevisionMismatch, TEXT("AdapterContractRevision"), TEXT("Selected provider typed adapter와 정확히 같은 AdapterContractRevision만 지원합니다."));
		}
		if (!StagingRelativePath.IsEmpty())
		{
			if (!CFDATypeDispatch::NormalizeProviderStagingPath(SelectedProvider, StagingRelativePath, Result.Record.StagingRelativePath))
			{
				CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingRelativePath"), TEXT("Staging source path가 selected provider의 canonical StagingRoot 아래 lowercase `.json` 경로가 아닙니다."));
			}
		}
	}

	// Provider TypeKey/revision/root 검증을 통과한 뒤에만 Missile typed payload adapter로 진입합니다.
	if (!CFDAStagingPrivate::HasBlockingIssue(Result.Issues))
	{
		// strict Payload JSON object입니다.
		TSharedPtr<FJsonObject> PayloadObject;
		if (CFDAStagingPrivate::ParseObjectField(RootObject, TEXT("Payload"), TEXT("Payload"), PayloadObject, Result.Issues))
		{
			CFDAStagingPrivate::ParseMissilePresetPayload(PayloadObject, Result.Record.Payload, Result.Issues);
		}
	}

	if (!Result.Record.StableLogicalId.IsNone()
		&& !Result.Record.Payload.PresetId.IsNone()
		&& Result.Record.StableLogicalId != Result.Record.Payload.PresetId)
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::StableIdentityMismatch, TEXT("StableLogicalId"), TEXT("StableLogicalId와 Payload.PresetId가 같은 FName identity가 아닙니다."));
	}

	if (!CFDAStagingPrivate::HasBlockingIssue(Result.Issues))
	{
		// typed desired payload의 derived semantic fingerprint 생성 오류입니다.
		FString FingerprintError;
		if (!CFDAMissileProviderImpl::BuildSemanticFingerprint(Result.Record.Payload, Result.Record.StagingSemanticFingerprint, FingerprintError))
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("Payload"), FingerprintError);
		}
	}

	Result.bValid = !CFDAStagingPrivate::HasBlockingIssue(Result.Issues);
	return Result;
}

// typed MissileGuidePreset whole-record payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
bool CFDAMissileProviderImpl::BuildSemanticFingerprint(
	const FCFDAMissilePresetPayload& Payload,
	FString& OutFingerprint,
	FString& OutError)
{
	if (!CFDAStagingPrivate::ValidateTypedMissilePresetPayload(Payload, OutError))
	{
		OutFingerprint.Reset();
		return false;
	}

	// exact schema/revision/class + typed payload canonical token stream입니다.
	TArray<uint8> CanonicalBytes;
	CanonicalBytes.Reserve(2048);
	CFDAStagingPrivate::AppendMissilePresetPayloadTokens(CanonicalBytes, Payload);
	return CFDAStagingPrivate::HashCanonicalBytes(CanonicalBytes, OutFingerprint, OutError);
}

// exact MissileGuidePreset UObject를 current fingerprint와 동일한 P0 Literal whole-record semantic payload로 lossless 추출합니다.
bool CFDAMissileProviderImpl::ExtractPayload(
	const UCFMissileGuidePresetData& PresetAsset,
	FCFDAMissilePresetPayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutIssues.Reset();
	return CFDAStagingPrivate::BuildMissilePresetPayloadFromAsset(PresetAsset, OutPayload, OutIssues);
}

// Payload-free common candidate를 Asset Registry + Missile typed readback으로 exact current truth에 해석합니다.
bool CFDAMissileProviderImpl::ResolveCommonCurrentState(
	const FCFDACommonEnvelope& Envelope,
	FCFDACommonCurrentState& OutCurrentState,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutCurrentState = FCFDACommonCurrentState();
	OutIssues.Reset();

	// Envelope exact TypeKey에 등록된 complete provider entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		Envelope.SchemaId,
		Envelope.DataAssetTypeClassPath,
		&ProviderLookupError);
	// Missile current-state resolver가 허용하는 exact first provider entry입니다.
	const FCFDATypeProviderEntry& MissileProviderEntry = CFDATypeDispatch::GetMissilePresetProviderEntry();
	// Provider 계약 검증 실패 상세입니다.
	FString ProviderContractError;
	if (ProviderEntry == nullptr
		|| ProviderEntry != &MissileProviderEntry
		|| !CFDATypeDispatch::ValidateProviderContract(Envelope, ProviderEntry->Descriptor, ProviderContractError))
	{
		// Fail-closed 원인을 보존하는 current-state provider diagnostic입니다.
		const FString ProviderDiagnostic = ProviderEntry == nullptr
			? ProviderLookupError
			: (ProviderEntry != &MissileProviderEntry
				? TEXT("Missile current-state resolver에 등록된 typed provider가 아닙니다.")
				: ProviderContractError);
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderDiagnostic);
		return false;
	}

	// Current resolver가 사용할 selected structural descriptor입니다.
	const FCFDATypeProvider& Provider = ProviderEntry->Descriptor;
	if (!CFDAStagingPrivate::ValidateTargetObjectPath(Envelope.TargetObjectPath, OutIssues))
	{
		return false;
	}

	// CF-FQ-045 public Registry coverage를 current resolver가 요구하는 metadata authority로 준비합니다.
	FCFDATypeRegistry TypeRegistry;
	// Current concrete descriptor 등록 실패 원인입니다.
	FString RegistryError;
	if (!TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistryError))
	{
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry"),
			FString::Printf(TEXT("CF-FQ-045 current descriptor 등록에 실패했습니다: %s"), *RegistryError));
		return false;
	}
	// Pilot exact class의 public semantic descriptor입니다.
	const FCFDASemanticDescriptor* Descriptor = TypeRegistry.FindDescriptor(Envelope.DataAssetTypeClassPath);
	if (Descriptor == nullptr
		|| Descriptor->IdentityPolicy != ECFDAIdentityPolicy::Required
		|| Descriptor->IdentityResolverKind != ECFDAIdentityResolverKind::ExplicitFName
		|| Descriptor->IdentitySourceName != Provider.StableIdentitySourceName.ToString())
	{
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry.Identity"),
			TEXT("CFMissileGuidePresetData Registry identity 계약이 Required/ExplicitFName/PresetId와 다릅니다."));
		return false;
	}

	// Read-only exact target/identity 조회에 사용할 Project Asset Registry입니다.
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	// Exact requested target object path입니다.
	const FSoftObjectPath TargetObjectPath(Envelope.TargetObjectPath);
	// 이미 메모리에 존재하는 exact target UObject입니다. ResolveObject는 load/mutation을 수행하지 않습니다.
	UObject* ResolvedTargetObject = TargetObjectPath.ResolveObject();
	// Persisted/registered exact target metadata입니다.
	const FAssetData TargetAssetData = AssetRegistry.GetAssetByObjectPath(TargetObjectPath, false, false);
	OutCurrentState.bRequestedTargetExists = ResolvedTargetObject != nullptr || TargetAssetData.IsValid();

	if (OutCurrentState.bRequestedTargetExists)
	{
		if (ResolvedTargetObject != nullptr)
		{
			OutCurrentState.RequestedTargetClassPath = ResolvedTargetObject->GetClass()->GetClassPathName().ToString();
		}
		else
		{
			OutCurrentState.RequestedTargetClassPath = TargetAssetData.AssetClassPath.ToString();
		}

		if (OutCurrentState.RequestedTargetClassPath.Equals(Provider.TypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
		{
			// Current exact class target를 semantic readback하기 위해 load한 read-only UObject입니다.
			UObject* LoadedTargetObject = ResolvedTargetObject != nullptr ? ResolvedTargetObject : TargetAssetData.GetAsset();
			// Exact expected DataAsset 타입으로 확인한 current target입니다.
			const UCFMissileGuidePresetData* CurrentPreset = Cast<UCFMissileGuidePresetData>(LoadedTargetObject);
			if (CurrentPreset == nullptr)
			{
				CFDAStagingPrivate::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("Current.TargetObjectPath"),
					TEXT("Asset Registry에는 expected class target이 있지만 UObject semantic readback에 실패했습니다."));
				return false;
			}

			OutCurrentState.RequestedTargetStableLogicalId = CurrentPreset->PresetId;
			// Current exact target를 소유하는 package입니다.
			const UPackage* CurrentPackage = CurrentPreset->GetOutermost();
			OutCurrentState.bRequestedTargetDirty = CurrentPackage != nullptr && CurrentPackage->IsDirty();

			// Current persisted/loaded asset의 raw whole-record typed payload입니다.
			FCFDAMissilePresetPayload CurrentPayload;
			if (!CFDAMissileProviderImpl::ExtractPayload(*CurrentPreset, CurrentPayload, OutIssues))
			{
				return false;
			}
			// Current semantic fingerprint 생성 오류입니다.
			FString CurrentFingerprintError;
			if (!CFDAMissileProviderImpl::BuildSemanticFingerprint(CurrentPayload, OutCurrentState.CurrentSemanticFingerprint, CurrentFingerprintError))
			{
				CFDAStagingPrivate::AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("CurrentSemanticFingerprint"),
					CurrentFingerprintError);
				return false;
			}
		}
	}

	// Asset Registry와 loaded-but-unregistered UObject 양쪽을 합친 exact StableIdentity object path 집합입니다.
	TSet<FString> StableIdentityObjectPaths;

	// 아직 Asset Registry에 등록되지 않은 /Game loaded object도 Apply preflight current truth에서 누락하지 않습니다.
	for (TObjectIterator<UCFMissileGuidePresetData> LoadedPresetIterator; LoadedPresetIterator; ++LoadedPresetIterator)
	{
		// 현재 process에 살아 있는 exact Pilot DataAsset 후보입니다.
		const UCFMissileGuidePresetData* LoadedPreset = *LoadedPresetIterator;
		if (LoadedPreset == nullptr
			|| LoadedPreset->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient)
			|| LoadedPreset->PresetId != Envelope.StableLogicalId)
		{
			continue;
		}

		// Loaded object의 exact soft object identity입니다.
		const FString LoadedObjectPath = FSoftObjectPath(LoadedPreset).ToString();
		if (!LoadedObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive))
		{
			continue;
		}
		StableIdentityObjectPaths.Add(LoadedObjectPath);
	}

	// Current Project 안의 exact CFMissileGuidePresetData 전체에서 StableLogicalId를 찾는 metadata+typed read-only filter입니다.
	FARFilter IdentityFilter;
	IdentityFilter.ClassPaths.Add(UCFMissileGuidePresetData::StaticClass()->GetClassPathName());
	IdentityFilter.bRecursiveClasses = false;
	if (AssetRegistry.IsLoadingAssets())
	{
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry.IdentityScan"),
			TEXT("Asset Registry가 아직 loading 중이라 StableIdentity current truth를 확정할 수 없습니다."));
		return false;
	}

	// Current exact Pilot class asset metadata 목록입니다. 0건은 valid absent current truth입니다.
	TArray<FAssetData> MissilePresetAssets;
	// Filter는 StaticClass 기반으로 내부 고정되므로 query result bool과 무관하게 empty result를 정상 absent로 소비합니다.
	(void)AssetRegistry.GetAssets(IdentityFilter, MissilePresetAssets, false);

	for (const FAssetData& MissilePresetAssetData : MissilePresetAssets)
	{
		// StableLogicalId 비교를 위해 read-only load한 exact class DataAsset입니다.
		const UCFMissileGuidePresetData* CandidatePreset = Cast<UCFMissileGuidePresetData>(MissilePresetAssetData.GetAsset());
		if (CandidatePreset == nullptr || CandidatePreset->PresetId != Envelope.StableLogicalId)
		{
			continue;
		}

		StableIdentityObjectPaths.Add(MissilePresetAssetData.GetSoftObjectPath().ToString());
	}

	OutCurrentState.StableIdentityMatchCount = StableIdentityObjectPaths.Num();
	OutCurrentState.bStableIdentityExists = OutCurrentState.StableIdentityMatchCount > 0;
	if (OutCurrentState.StableIdentityMatchCount == 1)
	{
		// Single identity match의 exact current object path입니다.
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
	return !CFDAStagingPrivate::HasBlockingIssue(OutIssues);
}

// 기존 Public Missile current-state typed record를 common resolver에 투영하는 compatibility implementation입니다.
bool CFDAMissileProviderImpl::ResolveCurrentState(
	const FCFDAStagingRecord& Record,
	FCFDAStagingCurrentState& OutCurrentState,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Public Missile typed record에서 만든 provider-local common envelope입니다.
	const FCFDACommonEnvelope Envelope = BuildCommonEnvelope(Record);
	// Shared current-state resolver 결과입니다.
	FCFDACommonCurrentState CommonCurrentState;
	if (!ResolveCommonCurrentState(Envelope, CommonCurrentState, OutIssues))
	{
		OutCurrentState = FCFDAStagingCurrentState();
		return false;
	}

	OutCurrentState = FCFDAStagingCurrentState();
	OutCurrentState.bRequestedTargetExists = CommonCurrentState.bRequestedTargetExists;
	OutCurrentState.bRequestedTargetDirty = CommonCurrentState.bRequestedTargetDirty;
	OutCurrentState.RequestedTargetClassPath = CommonCurrentState.RequestedTargetClassPath;
	OutCurrentState.RequestedTargetStableLogicalId = CommonCurrentState.RequestedTargetStableLogicalId;
	OutCurrentState.CurrentSemanticFingerprint = CommonCurrentState.CurrentSemanticFingerprint;
	OutCurrentState.bStableIdentityExists = CommonCurrentState.bStableIdentityExists;
	OutCurrentState.StableIdentityMatchCount = CommonCurrentState.StableIdentityMatchCount;
	OutCurrentState.StableIdentityObjectPath = CommonCurrentState.StableIdentityObjectPath;
	return true;
}

// 기존 Public Missile parser API를 first typed provider callback으로 전달하는 compatibility facade입니다.
FCFDAStagingParseResult FCFDAStagingService::ParseMissilePresetJson(
	const FString& JsonText,
	const FString& StagingRelativePath)
{
	// Current Missile typed provider callback table입니다.
	const FCFDAMissileProviderCallbacks& Callbacks = CFDAMissileProvider::GetProvider().Callbacks;
	checkf(Callbacks.ParseJson != nullptr, TEXT("Missile provider ParseJson callback이 등록되지 않았습니다."));
	return Callbacks.ParseJson(JsonText, StagingRelativePath);
}

// 기존 Public Missile fingerprint API를 first typed provider callback으로 전달하는 compatibility facade입니다.
bool FCFDAStagingService::BuildSemanticFingerprint(
	const FCFDAMissilePresetPayload& Payload,
	FString& OutFingerprint,
	FString& OutError)
{
	// Current Missile typed provider callback table입니다.
	const FCFDAMissileProviderCallbacks& Callbacks = CFDAMissileProvider::GetProvider().Callbacks;
	checkf(Callbacks.BuildSemanticFingerprint != nullptr, TEXT("Missile provider fingerprint callback이 등록되지 않았습니다."));
	return Callbacks.BuildSemanticFingerprint(Payload, OutFingerprint, OutError);
}

// 기존 Public Missile extractor API를 first typed provider callback으로 전달하는 compatibility facade입니다.
bool FCFDAStagingService::ExtractMissilePresetPayload(
	const UCFMissileGuidePresetData& PresetAsset,
	FCFDAMissilePresetPayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Current Missile typed provider callback table입니다.
	const FCFDAMissileProviderCallbacks& Callbacks = CFDAMissileProvider::GetProvider().Callbacks;
	checkf(Callbacks.ExtractPayload != nullptr, TEXT("Missile provider extractor callback이 등록되지 않았습니다."));
	return Callbacks.ExtractPayload(PresetAsset, OutPayload, OutIssues);
}

// 기존 Public Missile current-state API를 first typed provider callback으로 전달하는 compatibility facade입니다.
bool FCFDAStagingService::ResolveMissilePresetCurrentState(
	const FCFDAStagingRecord& Record,
	FCFDAStagingCurrentState& OutCurrentState,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	// Current Missile typed provider callback table입니다.
	const FCFDAMissileProviderCallbacks& Callbacks = CFDAMissileProvider::GetProvider().Callbacks;
	checkf(Callbacks.ResolveCurrentState != nullptr, TEXT("Missile provider current-state callback이 등록되지 않았습니다."));
	return Callbacks.ResolveCurrentState(Record, OutCurrentState, OutIssues);
}

// Payload-free common candidate와 current truth를 P0 exact 3-way 규칙으로 mutation0 분류합니다.
FCFDACommonPreviewRow CFDATypeDispatch::BuildCommonPreview(
	const FCFDACommonEnvelope& Envelope,
	const FCFDACommonCurrentState& CurrentState)
{
	// 반환할 payload-free Preview row입니다.
	FCFDACommonPreviewRow Row;
	Row.Envelope = Envelope;
	Row.Envelope.CurrentSemanticFingerprint = CurrentState.CurrentSemanticFingerprint;

	// Preview kind와 immutable approval operation을 항상 함께 갱신하는 finalizer입니다.
	const auto FinalizeRow = [&Row](const ECFDAStagingPreviewKind Kind)
	{
		Row.Kind = Kind;
		Row.Envelope.PlannedOperation = Kind;
		return Row;
	};

	// Preview input exact TypeKey에 등록된 complete provider entry입니다.
	FString ProviderLookupError;
	const FCFDATypeProviderEntry* ProviderEntry = FindExactProviderEntry(
		Envelope.SchemaId,
		Envelope.DataAssetTypeClassPath,
		&ProviderLookupError);
	// Provider 계약 검증 실패 상세입니다.
	FString ProviderContractError;
	if (ProviderEntry == nullptr
		|| !ValidateProviderContract(Row.Envelope, ProviderEntry->Descriptor, ProviderContractError))
	{
		CFDAStagingPrivate::AddIssue(
			Row.Issues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("TypeKey"),
			ProviderEntry == nullptr ? ProviderLookupError : ProviderContractError);
		return FinalizeRow(ECFDAStagingPreviewKind::Invalid);
	}
	if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Envelope.StagingSemanticFingerprint))
	{
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingSemanticFingerprint"), TEXT("Preview input의 StagingSemanticFingerprint가 canonical SHA-256이 아닙니다."));
		return FinalizeRow(ECFDAStagingPreviewKind::Invalid);
	}
	if (!CFDAStagingPrivate::ValidateTargetObjectPath(Envelope.TargetObjectPath, Row.Issues))
	{
		return FinalizeRow(ECFDAStagingPreviewKind::Invalid);
	}

	if (CurrentState.StableIdentityMatchCount > 1)
	{
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::DuplicateStableIdentity, TEXT("StableLogicalId"), TEXT("current Project에 같은 class-scoped StableLogicalId가 둘 이상 존재합니다."));
		return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
	}
	if (CurrentState.bStableIdentityExists
		&& !CurrentState.StableIdentityObjectPath.IsEmpty()
		&& !CurrentState.StableIdentityObjectPath.Equals(Envelope.TargetObjectPath, ESearchCase::IgnoreCase))
	{
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::TargetMoved, TEXT("TargetObjectPath"), TEXT("같은 class-scoped StableLogicalId가 다른 current object path에 존재합니다."));
		return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
	}

	if (CurrentState.bRequestedTargetExists)
	{
		if (!CurrentState.RequestedTargetClassPath.Equals(Envelope.DataAssetTypeClassPath, ESearchCase::CaseSensitive)
			|| CurrentState.RequestedTargetStableLogicalId != Envelope.StableLogicalId)
		{
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::PathCollision, TEXT("TargetObjectPath"), TEXT("requested path에 다른 class 또는 stable identity의 object가 존재합니다."));
			return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
		}
		if (CurrentState.bRequestedTargetDirty)
		{
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::TargetDirtyUnowned, TEXT("TargetObjectPath"), TEXT("pre-existing dirty target은 authoring save ownership 밖이므로 Preview에서 차단합니다."));
			return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
		}
	}

	if (!Envelope.bHasBaseSemanticFingerprint)
	{
		if (CurrentState.bRequestedTargetExists || CurrentState.bStableIdentityExists)
		{
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::UnexpectedExistingTarget, TEXT("BaseSemanticFingerprint"), TEXT("Create-intent candidate인데 target 또는 identity가 이미 존재합니다."));
			return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
		}
		Row.Envelope.CurrentSemanticFingerprint.Reset();
		return FinalizeRow(ECFDAStagingPreviewKind::Create);
	}

	if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Envelope.BaseSemanticFingerprint))
	{
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::BaselineMissing, TEXT("BaseSemanticFingerprint"), TEXT("Update-intent candidate에는 canonical BaseSemanticFingerprint가 필요합니다."));
		return FinalizeRow(ECFDAStagingPreviewKind::Invalid);
	}
	if (!CurrentState.bRequestedTargetExists)
	{
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::TargetMissing, TEXT("TargetObjectPath"), TEXT("Update baseline은 존재하지만 requested current target이 없습니다."));
		return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
	}
	if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(CurrentState.CurrentSemanticFingerprint))
	{
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("CurrentSemanticFingerprint"), TEXT("current target fingerprint가 canonical SHA-256이 아닙니다."));
		return FinalizeRow(ECFDAStagingPreviewKind::Invalid);
	}

	// Update Staging을 만들 때 캡처한 semantic baseline입니다.
	const FString& BaseFingerprint = Envelope.BaseSemanticFingerprint;
	// Preview 직전 current Unreal semantic state입니다.
	const FString& CurrentFingerprint = CurrentState.CurrentSemanticFingerprint;
	// Staging desired semantic state입니다.
	const FString& StagingFingerprint = Envelope.StagingSemanticFingerprint;

	if (BaseFingerprint == CurrentFingerprint)
	{
		return FinalizeRow(StagingFingerprint == CurrentFingerprint
			? ECFDAStagingPreviewKind::NoChange
			: ECFDAStagingPreviewKind::Update);
	}
	if (CurrentFingerprint == StagingFingerprint)
	{
		CFDAStagingPrivate::AddIssue(
			Row.Issues,
			ECFDAStagingIssueCode::BaselineRebaseRequired,
			TEXT("BaseSemanticFingerprint"),
			TEXT("current가 이미 Staging desired state로 수렴했습니다. mutation은 0이지만 다음 편집 전 explicit baseline rebase가 필요합니다."),
			false);
		return FinalizeRow(ECFDAStagingPreviewKind::NoChange);
	}

	CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::BaselineMismatch, TEXT("BaseSemanticFingerprint"), TEXT("Base와 Current가 달라 외부 current drift를 overwrite하지 않습니다."));
	return FinalizeRow(ECFDAStagingPreviewKind::Conflict);
}

// 기존 Public Missile typed record/current truth를 common Preview로 투영하는 compatibility facade입니다.
FCFDAStagingPreviewRow FCFDAStagingService::BuildPreview(
	const FCFDAStagingRecord& Record,
	const FCFDAStagingCurrentState& CurrentState)
{
	// 반환할 existing Public Missile Preview row입니다.
	FCFDAStagingPreviewRow Row;
	Row.Record = Record;
	Row.CurrentSemanticFingerprint = CurrentState.CurrentSemanticFingerprint;

	if (Record.StableLogicalId.IsNone() || Record.Payload.PresetId.IsNone())
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::StableIdentityMissing, TEXT("StableLogicalId"), TEXT("Preview input record의 required stable identity가 비어 있습니다."));
		return Row;
	}
	if (Record.StableLogicalId != Record.Payload.PresetId)
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::StableIdentityMismatch, TEXT("StableLogicalId"), TEXT("Preview input record의 StableLogicalId와 Payload.PresetId가 다릅니다."));
		return Row;
	}

	// Mutable Public DTO의 Payload와 cached fingerprint가 갈라진 상태를 approval 이전에 fail-closed합니다.
	FString StagingFingerprintIntegrityError;
	if (!CFDAStagingPrivate::ValidateStagingFingerprintIntegrity(Record, StagingFingerprintIntegrityError))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingSemanticFingerprint"), StagingFingerprintIntegrityError);
		return Row;
	}

	// Public current-state DTO를 shared payload-free current truth로 투영합니다.
	FCFDACommonCurrentState CommonCurrentState;
	CommonCurrentState.bRequestedTargetExists = CurrentState.bRequestedTargetExists;
	CommonCurrentState.bRequestedTargetDirty = CurrentState.bRequestedTargetDirty;
	CommonCurrentState.RequestedTargetClassPath = CurrentState.RequestedTargetClassPath;
	CommonCurrentState.RequestedTargetStableLogicalId = CurrentState.RequestedTargetStableLogicalId;
	CommonCurrentState.CurrentSemanticFingerprint = CurrentState.CurrentSemanticFingerprint;
	CommonCurrentState.bStableIdentityExists = CurrentState.bStableIdentityExists;
	CommonCurrentState.StableIdentityMatchCount = CurrentState.StableIdentityMatchCount;
	CommonCurrentState.StableIdentityObjectPath = CurrentState.StableIdentityObjectPath;

	// Provider-local typed record projection으로 만든 payload-free common envelope입니다.
	const FCFDACommonEnvelope Envelope = CFDAMissileProviderImpl::BuildCommonEnvelope(
		Record,
		CurrentState.CurrentSemanticFingerprint);
	// Shared 3-way state machine 결과입니다.
	const FCFDACommonPreviewRow CommonRow = CFDATypeDispatch::BuildCommonPreview(Envelope, CommonCurrentState);
	Row.Kind = CommonRow.Kind;
	Row.CurrentSemanticFingerprint = CommonRow.Envelope.CurrentSemanticFingerprint;
	Row.Issues = CommonRow.Issues;
	return Row;
}

// 같은 batch 안의 class-scoped StableLogicalId/global TargetObjectPath duplicate를 payload-free rows에서 fail-closed합니다.
void CFDATypeDispatch::ApplyCommonBatchDuplicateValidation(TArray<FCFDACommonPreviewRow>& InOutRows)
{
	// canonical class-scoped stable identity별 row index 목록입니다.
	TMap<FString, TArray<int32>> IdentityRows;
	// case-insensitive global target object path별 row index 목록입니다.
	TMap<FString, TArray<int32>> TargetPathRows;

	for (int32 RowIndex = 0; RowIndex < InOutRows.Num(); ++RowIndex)
	{
		// 현재 payload-free Preview row입니다.
		const FCFDACommonPreviewRow& Row = InOutRows[RowIndex];
		if (!Row.Envelope.StableLogicalId.IsNone())
		{
			// StableLogicalId duplicate scope를 exact DataAsset class + canonical FName semantic으로 만든 key입니다.
			const FString ClassScopedIdentityKey = BuildClassScopedStableIdentityKey(
				Row.Envelope.DataAssetTypeClassPath,
				Row.Envelope.StableLogicalId);
			if (!ClassScopedIdentityKey.IsEmpty())
			{
				IdentityRows.FindOrAdd(ClassScopedIdentityKey).Add(RowIndex);
			}
		}
		if (!Row.Envelope.TargetObjectPath.IsEmpty())
		{
			TargetPathRows.FindOrAdd(Row.Envelope.TargetObjectPath.ToLower()).Add(RowIndex);
		}
	}

	for (const TPair<FString, TArray<int32>>& Pair : IdentityRows)
	{
		if (Pair.Value.Num() < 2)
		{
			continue;
		}
		for (const int32 RowIndex : Pair.Value)
		{
			// Duplicate class-scoped identity로 invalid 승격할 common row입니다.
			FCFDACommonPreviewRow& Row = InOutRows[RowIndex];
			Row.Kind = ECFDAStagingPreviewKind::Invalid;
			Row.Envelope.PlannedOperation = ECFDAStagingPreviewKind::Invalid;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::DuplicateStableIdentity, TEXT("StableLogicalId"), TEXT("같은 batch 안에 동일 DataAsset class scope의 duplicate StableLogicalId가 있습니다."));
		}
	}

	for (const TPair<FString, TArray<int32>>& Pair : TargetPathRows)
	{
		if (Pair.Value.Num() < 2)
		{
			continue;
		}
		for (const int32 RowIndex : Pair.Value)
		{
			// Duplicate global target path로 invalid 승격할 common row입니다.
			FCFDACommonPreviewRow& Row = InOutRows[RowIndex];
			Row.Kind = ECFDAStagingPreviewKind::Invalid;
			Row.Envelope.PlannedOperation = ECFDAStagingPreviewKind::Invalid;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::DuplicateTargetPath, TEXT("TargetObjectPath"), TEXT("같은 batch 안에 duplicate TargetObjectPath가 있습니다."));
		}
	}
}

// 기존 Public Missile Preview rows를 common duplicate validator에 투영하는 compatibility facade입니다.
void FCFDAStagingService::ApplyBatchDuplicateValidation(TArray<FCFDAStagingPreviewRow>& InOutRows)
{
	// Shared duplicate validation에 전달할 payload-free rows입니다.
	TArray<FCFDACommonPreviewRow> CommonRows;
	CommonRows.Reserve(InOutRows.Num());
	for (const FCFDAStagingPreviewRow& Row : InOutRows)
	{
		// 한 Public Missile row를 provider-local common envelope로 투영한 row입니다.
		FCFDACommonPreviewRow CommonRow;
		CommonRow.Kind = Row.Kind;
		CommonRow.Envelope = CFDAMissileProviderImpl::BuildCommonEnvelope(
			Row.Record,
			Row.CurrentSemanticFingerprint,
			Row.Kind);
		CommonRow.Issues = Row.Issues;
		CommonRows.Add(MoveTemp(CommonRow));
	}

	CFDATypeDispatch::ApplyCommonBatchDuplicateValidation(CommonRows);
	for (int32 RowIndex = 0; RowIndex < InOutRows.Num(); ++RowIndex)
	{
		InOutRows[RowIndex].Kind = CommonRows[RowIndex].Kind;
		InOutRows[RowIndex].Issues = CommonRows[RowIndex].Issues;
	}
}

// Payload-free exact Create/Update candidate set을 deterministic order로 묶어 accepted SHA-256 BatchPlanHash를 계산합니다.
bool CFDATypeDispatch::BuildCommonBatchPlanHash(
	const TArray<FCFDACommonPreviewRow>& PreviewRows,
	FString& OutBatchPlanHash,
	FString& OutError)
{
	// Caller가 별도 duplicate-validation 호출을 빼먹어도 hash authority가 fail-closed하도록 복사한 common rows입니다.
	TArray<FCFDACommonPreviewRow> ValidatedRows = PreviewRows;
	ApplyCommonBatchDuplicateValidation(ValidatedRows);

	for (const FCFDACommonPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Conflict || Row.Kind == ECFDAStagingPreviewKind::Invalid)
		{
			OutBatchPlanHash.Reset();
			OutError = TEXT("Conflict/Invalid가 포함된 Preview set에는 approval BatchPlanHash를 만들 수 없습니다.");
			return false;
		}

		if (Row.Kind == ECFDAStagingPreviewKind::Create || Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			// Provider parse/current 경계를 이미 통과한 common row가 hash authority에 필요한 payload-free structural binding을 갖는지 검증합니다.
			if (Row.Envelope.SchemaId.IsEmpty()
				|| Row.Envelope.SchemaRevision <= 0
				|| Row.Envelope.AdapterContractRevision <= 0
				|| Row.Envelope.DataAssetTypeClassPath.IsEmpty()
				|| Row.Envelope.StableLogicalId.IsNone()
				|| Row.Envelope.StagingRelativePath.IsEmpty()
				|| !Row.Envelope.StagingRelativePath.EndsWith(TEXT(".json"), ESearchCase::CaseSensitive)
				|| Row.Envelope.PlannedOperation != Row.Kind)
			{
				OutBatchPlanHash.Reset();
				OutError = TEXT("Common candidate의 payload-free structural/operation binding이 유효하지 않습니다.");
				return false;
			}
			if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Row.Envelope.StagingSemanticFingerprint))
			{
				OutBatchPlanHash.Reset();
				OutError = TEXT("Common candidate의 StagingSemanticFingerprint가 canonical SHA-256이 아닙니다.");
				return false;
			}
			// Target path canonicality 검증용 임시 진단입니다.
			TArray<FCFDAStagingIssue> TargetPathIssues;
			if (!CFDAStagingPrivate::ValidateTargetObjectPath(Row.Envelope.TargetObjectPath, TargetPathIssues))
			{
				OutBatchPlanHash.Reset();
				OutError = TEXT("Common candidate의 TargetObjectPath가 canonical Unreal asset object path가 아닙니다.");
				return false;
			}

			if (Row.Kind == ECFDAStagingPreviewKind::Create)
			{
				if (Row.Envelope.bHasBaseSemanticFingerprint
					|| !Row.Envelope.BaseSemanticFingerprint.IsEmpty()
					|| !Row.Envelope.CurrentSemanticFingerprint.IsEmpty())
				{
					OutBatchPlanHash.Reset();
					OutError = TEXT("Create common candidate는 Base가 없어야 하고 Current target fingerprint도 없어야 합니다.");
					return false;
				}
			}
			else if (!Row.Envelope.bHasBaseSemanticFingerprint
				|| !CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Row.Envelope.BaseSemanticFingerprint)
				|| !CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Row.Envelope.CurrentSemanticFingerprint))
			{
				OutBatchPlanHash.Reset();
				OutError = TEXT("Update common candidate는 canonical Base/Current semantic fingerprint를 모두 가져야 합니다.");
				return false;
			}
		}
	}

	// Create/Update mutation candidate만 복사한 exact included common target set입니다.
	TArray<FCFDACommonPreviewRow> CandidateRows;
	for (const FCFDACommonPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Create || Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			CandidateRows.Add(Row);
		}
	}
	if (CandidateRows.IsEmpty())
	{
		OutBatchPlanHash.Reset();
		OutError = TEXT("Create/Update mutation candidate가 없어 Batch approval이 필요하지 않습니다.");
		return false;
	}

	CandidateRows.Sort([](const FCFDACommonPreviewRow& Left, const FCFDACommonPreviewRow& Right)
	{
		return CFDAStagingPrivate::BuildBatchSortKey(Left) < CFDAStagingPrivate::BuildBatchSortKey(Right);
	});

	// Exact sorted target-plan canonical token stream입니다. 기존 Public Missile BatchPlanHash byte format을 그대로 유지합니다.
	TArray<uint8> CanonicalBytes;
	CanonicalBytes.Reserve(CandidateRows.Num() * 1024);
	CFDAStagingPrivate::AppendStringToken(CanonicalBytes, TEXT("Kind"), CFDAStagingPrivate::BatchPlanKind);
	CFDAStagingPrivate::AppendStringToken(CanonicalBytes, TEXT("FormatRevision"), FString::FromInt(CFDAStagingPrivate::BatchPlanFormatRevision));
	CFDAStagingPrivate::AppendStringToken(CanonicalBytes, TEXT("IncludedTargetCount"), FString::FromInt(CandidateRows.Num()));

	for (const FCFDACommonPreviewRow& Row : CandidateRows)
	{
		CFDAStagingPrivate::AppendBatchTargetTokens(CanonicalBytes, Row);
	}
	return CFDAStagingPrivate::HashCanonicalBytes(CanonicalBytes, OutBatchPlanHash, OutError);
}

// 기존 Public Missile Preview set을 common BatchPlanHash authority에 투영하는 compatibility facade입니다.
bool FCFDAStagingService::BuildBatchPlanHash(
	const TArray<FCFDAStagingPreviewRow>& PreviewRows,
	FString& OutBatchPlanHash,
	FString& OutError)
{
	// Public mutable typed DTO split-state를 provider-local facade에서 먼저 검증합니다.
	for (const FCFDAStagingPreviewRow& Row : PreviewRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Create || Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			FString CandidateIntegrityError;
			if (!CFDAStagingPrivate::ValidateMissileBatchCandidateIntegrity(Row, CandidateIntegrityError))
			{
				OutBatchPlanHash.Reset();
				OutError = CandidateIntegrityError;
				return false;
			}
		}
	}

	// Shared hash authority에 전달할 payload-free rows입니다.
	TArray<FCFDACommonPreviewRow> CommonRows;
	CommonRows.Reserve(PreviewRows.Num());
	for (const FCFDAStagingPreviewRow& Row : PreviewRows)
	{
		// 한 Public Missile row를 common row로 투영합니다.
		FCFDACommonPreviewRow CommonRow;
		CommonRow.Kind = Row.Kind;
		CommonRow.Envelope = CFDAMissileProviderImpl::BuildCommonEnvelope(
			Row.Record,
			Row.CurrentSemanticFingerprint,
			Row.Kind);
		CommonRow.Issues = Row.Issues;
		CommonRows.Add(MoveTemp(CommonRow));
	}
	return CFDATypeDispatch::BuildCommonBatchPlanHash(CommonRows, OutBatchPlanHash, OutError);
}

// diagnostic 배열에 특정 machine code가 존재하는지 확인합니다.
bool FCFDAStagingService::HasIssueCode(
	const TArray<FCFDAStagingIssue>& Issues,
	const ECFDAStagingIssueCode Code)
{
	for (const FCFDAStagingIssue& Issue : Issues)
	{
		if (Issue.Code == Code)
		{
			return true;
		}
	}
	return false;
}

#if WITH_DEV_AUTOMATION_TESTS
// Production strict parser implementation을 CF-FQ-050 private probe로 직접 호출합니다.
FCFDAStagingParseResult CFDAContractProbeParse(const FString& JsonText, const FString& StagingRelativePath)
{
	return FCFDAStagingService::ParseMissilePresetJson(JsonText, StagingRelativePath);
}

// Production fingerprint implementation을 호출하면서 실제 semantic token label sequence를 관측합니다.
bool CFDAContractProbeFingerprint(
	const FCFDAMissilePresetPayload& Payload,
	FString& OutFingerprint,
	TArray<FString>& OutTokenLabels,
	FString& OutError)
{
	OutTokenLabels.Reset();
	// 현재 thread에서 token sink 설치/복원을 소유하는 scoped probe입니다.
	CFDACommonPrimitives::FScopedSemanticTokenProbe ScopedProbe(OutTokenLabels);
	if (!ScopedProbe.IsBound())
	{
		OutFingerprint.Reset();
		OutError = TEXT("DACE fingerprint probe가 같은 thread에서 중첩 호출되었습니다.");
		return false;
	}
	// Production fingerprint path의 실제 결과이며 scope destructor가 성공/실패와 무관하게 sink를 복원합니다.
	return FCFDAStagingService::BuildSemanticFingerprint(Payload, OutFingerprint, OutError);
}

// Production extractor implementation을 CF-FQ-050 private probe로 직접 호출합니다.
bool CFDAContractProbeExtract(
	const UCFMissileGuidePresetData& Asset,
	FCFDAMissilePresetPayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	return FCFDAStagingService::ExtractMissilePresetPayload(Asset, OutPayload, OutIssues);
}
#endif
