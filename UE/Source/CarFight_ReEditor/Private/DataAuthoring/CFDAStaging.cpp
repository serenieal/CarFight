// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStaging.cpp
// Version: v1.7.1
// Date: 2026-09-09
// Description: CF-FQ-049 DAS-P0-02 MissileGuidePreset strict JSON parse, canonical SHA-256, read-only current resolve, mutation0 Preview와 BatchPlanHash 구현입니다.
// Changelog:
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

#include "DataAuthoring/CFDAStaging.h"
#include "CFDAContractGuard.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "CFMissileGuidePresetData.h"
#include "Containers/StringConv.h"
#include "DataManagement/CFDATypeRegistry.h"
#include "Dom/JsonObject.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Misc/PackageName.h"
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
	// P0 MissileGuidePreset schema identity입니다.
	static constexpr TCHAR MissilePresetSchemaId[] = TEXT("CarFight.DataAsset.MissileGuidePreset");

	// P0 MissileGuidePreset JSON shape revision입니다.
	static constexpr int32 MissilePresetSchemaRevision = 1;

	// P0 MissileGuidePreset typed adapter semantic revision입니다.
	static constexpr int32 MissilePresetAdapterRevision = 2;

	// P0 MissileGuidePreset exact native class path입니다.
	static constexpr TCHAR MissilePresetClassPath[] = TEXT("/Script/CarFight_Re.CFMissileGuidePresetData");

	// BatchPlanHash domain kind입니다.
	static constexpr TCHAR BatchPlanKind[] = TEXT("CarFightDAStagingBatchPlan");

	// BatchPlanHash binary token format revision입니다.
	static constexpr int32 BatchPlanFormatRevision = 1;

#if WITH_DEV_AUTOMATION_TESTS
	// DACE private probe가 현재 thread의 production fingerprint path에서 실제 emit된 token label을 관측할 temporary sink입니다.
	thread_local TArray<FString>* GSemanticTokenLabelProbe = nullptr;

	// 한 fingerprint probe lifetime 동안 token observation sink를 설치하고 반드시 이전 상태로 복원합니다.
	struct FScopedSemanticTokenProbe
	{
		// Scope 진입 전 현재 thread의 probe sink입니다.
		TArray<FString>* PreviousSink = nullptr;
		// 이번 scope가 실제 sink ownership을 획득했는지 나타냅니다.
		bool bBound = false;

		// 현재 thread에 기존 probe가 없을 때 requested sink를 설치합니다.
		explicit FScopedSemanticTokenProbe(TArray<FString>& RequestedSink)
			: PreviousSink(GSemanticTokenLabelProbe)
			, bBound(PreviousSink == nullptr)
		{
			if (bBound)
			{
				GSemanticTokenLabelProbe = &RequestedSink;
			}
		}

		// Scope 종료 시 성공/실패 경로와 무관하게 이전 sink를 복원합니다.
		~FScopedSemanticTokenProbe()
		{
			if (bBound)
			{
				GSemanticTokenLabelProbe = PreviousSink;
			}
		}

		// 이번 scope가 probe sink ownership을 획득했는지 반환합니다.
		bool IsBound() const
		{
			return bBound;
		}
	};
#endif

	// blocking 또는 informational issue를 target 배열에 추가합니다.
	void AddIssue(
		TArray<FCFDAStagingIssue>& OutIssues,
		const ECFDAStagingIssueCode Code,
		const FString& FieldPath,
		const FString& Message,
		const bool bBlocking = true)
	{
		// 새로 추가할 stable diagnostic입니다.
		FCFDAStagingIssue Issue;
		Issue.Code = Code;
		Issue.FieldPath = FieldPath;
		Issue.Message = Message;
		Issue.bBlocking = bBlocking;
		OutIssues.Add(MoveTemp(Issue));
	}

	// issue 배열에 blocking 진단이 하나라도 존재하는지 확인합니다.
	bool HasBlockingIssue(const TArray<FCFDAStagingIssue>& Issues)
	{
		for (const FCFDAStagingIssue& Issue : Issues)
		{
			if (Issue.bBlocking)
			{
				return true;
			}
		}
		return false;
	}

	// uint32를 platform endian과 무관한 big-endian 4 bytes로 append합니다.
	void AppendUint32BigEndian(TArray<uint8>& OutBytes, const uint32 Value)
	{
		OutBytes.Add(static_cast<uint8>((Value >> 24) & 0xff));
		OutBytes.Add(static_cast<uint8>((Value >> 16) & 0xff));
		OutBytes.Add(static_cast<uint8>((Value >> 8) & 0xff));
		OutBytes.Add(static_cast<uint8>(Value & 0xff));
	}

	// label/value raw bytes를 length-prefixed canonical token으로 append합니다.
	void AppendRawToken(
		TArray<uint8>& OutBytes,
		const FString& Label,
		const uint8* ValueBytes,
		const int32 ValueByteCount)
	{
#if WITH_DEV_AUTOMATION_TESTS
		if (GSemanticTokenLabelProbe != nullptr)
		{
			GSemanticTokenLabelProbe->Add(Label);
		}
#endif
		// token label의 deterministic UTF-8 bytes입니다.
		FTCHARToUTF8 LabelUtf8(*Label);
		AppendUint32BigEndian(OutBytes, static_cast<uint32>(LabelUtf8.Length()));
		OutBytes.Append(reinterpret_cast<const uint8*>(LabelUtf8.Get()), LabelUtf8.Length());
		AppendUint32BigEndian(OutBytes, static_cast<uint32>(ValueByteCount));
		if (ValueByteCount > 0)
		{
			OutBytes.Append(ValueBytes, ValueByteCount);
		}
	}

	// label/FString을 UTF-8 length-prefixed canonical token으로 append합니다.
	void AppendStringToken(TArray<uint8>& OutBytes, const FString& Label, const FString& Value)
	{
		// token value의 deterministic UTF-8 bytes입니다.
		FTCHARToUTF8 ValueUtf8(*Value);
		AppendRawToken(
			OutBytes,
			Label,
			reinterpret_cast<const uint8*>(ValueUtf8.Get()),
			ValueUtf8.Length());
	}

	// bool을 text formatter를 거치지 않는 1-byte canonical token으로 append합니다.
	void AppendBoolToken(TArray<uint8>& OutBytes, const FString& Label, const bool bValue)
	{
		// canonical bool byte입니다.
		const uint8 CanonicalBool = bValue ? 1 : 0;
		AppendRawToken(OutBytes, Label, &CanonicalBool, 1);
	}

	// float를 -0→+0 정규화 후 IEEE-754 32-bit big-endian canonical token으로 append합니다.
	void AppendFloatToken(TArray<uint8>& OutBytes, const FString& Label, const float Value)
	{
		// -0.0f와 +0.0f를 같은 semantic 값으로 만드는 canonical float입니다.
		const float CanonicalValue = Value == 0.0f ? 0.0f : Value;
		// canonical float의 raw IEEE-754 bit pattern입니다.
		uint32 FloatBits = 0;
		FMemory::Memcpy(&FloatBits, &CanonicalValue, sizeof(float));
		// platform endian과 무관한 canonical float bytes입니다.
		uint8 FloatBytes[4] =
		{
			static_cast<uint8>((FloatBits >> 24) & 0xff),
			static_cast<uint8>((FloatBits >> 16) & 0xff),
			static_cast<uint8>((FloatBits >> 8) & 0xff),
			static_cast<uint8>(FloatBits & 0xff)
		};
		AppendRawToken(OutBytes, Label, FloatBytes, UE_ARRAY_COUNT(FloatBytes));
	}

	// FName semantic equality와 맞추기 위해 comparison text를 lowercase로 canonicalize합니다.
	FString CanonicalNameText(const FName Value)
	{
		return Value.ToString().ToLower();
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

	// SHA-256 canonical lowercase `sha256:<64 hex>` 형식인지 확인합니다.
	bool IsCanonicalSha256Fingerprint(const FString& Fingerprint)
	{
		if (Fingerprint.Len() != 71 || !Fingerprint.StartsWith(TEXT("sha256:"), ESearchCase::CaseSensitive))
		{
			return false;
		}

		for (int32 CharacterIndex = 7; CharacterIndex < Fingerprint.Len(); ++CharacterIndex)
		{
			// 현재 SHA-256 hex digit입니다.
			const TCHAR Character = Fingerprint[CharacterIndex];
			// lowercase canonical hex인지 나타냅니다.
			const bool bIsLowerHex = (Character >= TEXT('0') && Character <= TEXT('9'))
				|| (Character >= TEXT('a') && Character <= TEXT('f'));
			if (!bIsLowerHex)
			{
				return false;
			}
		}
		return true;
	}

	// canonical byte stream을 OpenSSL EVP SHA-256으로 hash하고 protocol fingerprint 형식으로 반환합니다.
	bool HashCanonicalBytes(const TArray<uint8>& Bytes, FString& OutFingerprint, FString& OutError)
	{
		// OpenSSL EVP가 채울 32-byte SHA-256 결과입니다.
		FSHA256Signature Signature;
		// OpenSSL EVP가 반환하는 실제 digest byte 수입니다.
		unsigned int DigestLength = 0;
		// empty buffer에서도 안전한 non-null input pointer입니다.
		const uint8 EmptyInputByte = 0;
		// hash 대상 canonical byte pointer입니다.
		const uint8* HashData = Bytes.Num() > 0 ? Bytes.GetData() : &EmptyInputByte;
		// OpenSSL portable SHA-256 실행 결과입니다.
		const int32 DigestResult = EVP_Digest(
			HashData,
			static_cast<size_t>(Bytes.Num()),
			Signature.Signature,
			&DigestLength,
			EVP_sha256(),
			nullptr);
		if (DigestResult != 1 || DigestLength != UE_ARRAY_COUNT(Signature.Signature))
		{
			OutFingerprint.Reset();
			OutError = TEXT("OpenSSL EVP SHA-256 fingerprint 생성에 실패했습니다.");
			return false;
		}

		OutFingerprint = TEXT("sha256:") + Signature.ToString().ToLower();
		OutError.Reset();
		return true;
	}

	// JSON object가 exact required field set만 가지는지 검사합니다.
	bool ValidateExactFields(
		const TSharedPtr<FJsonObject>& Object,
		const TArray<FString>& RequiredFields,
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// allowed/required field membership 검사 집합입니다.
		TSet<FString> RequiredFieldSet;
		for (const FString& RequiredField : RequiredFields)
		{
			RequiredFieldSet.Add(RequiredField);
		}

		for (const auto& Pair : Object->Values)
		{
			// UE 5.8 FJsonObject의 shared-string key를 schema 비교용 owned FString으로 변환합니다.
			const FString ActualFieldName(Pair.Key.ToView());
			if (!RequiredFieldSet.Contains(ActualFieldName))
			{
				AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::UnknownField,
					ObjectPath.IsEmpty() ? ActualFieldName : ObjectPath + TEXT(".") + ActualFieldName,
					TEXT("schema에 없는 field입니다. 오타를 자동 무시하지 않습니다."));
			}
		}

		for (const FString& RequiredField : RequiredFields)
		{
			// UE 5.8 public FStringView existence lookup 결과입니다.
			if (!Object->HasField(RequiredField))
			{
				AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::MissingRequiredField,
					ObjectPath.IsEmpty() ? RequiredField : ObjectPath + TEXT(".") + RequiredField,
					TEXT("whole-record required field가 누락되었습니다."));
			}
		}
		return !HasBlockingIssue(OutIssues);
	}

	// exact field를 찾아 expected JSON type인지 검사합니다.
	TSharedPtr<FJsonValue> RequireField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		const EJson ExpectedType,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// UE 5.8 public FStringView untyped lookup으로 읽은 requested JSON field입니다.
		const TSharedPtr<FJsonValue> FoundValue = Object->GetFieldUntyped(FieldName);
		if (!FoundValue.IsValid())
		{
			return nullptr;
		}
		if (FoundValue->Type != ExpectedType)
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::TypeMismatch, FieldPath, TEXT("JSON field type이 schema와 다릅니다."));
			return nullptr;
		}
		return FoundValue;
	}

	// required JSON string field를 parse합니다.
	bool ParseStringField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		FString& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// exact string JSON field입니다.
		const TSharedPtr<FJsonValue> Value = RequireField(Object, FieldName, FieldPath, EJson::String, OutIssues);
		if (!Value.IsValid())
		{
			return false;
		}
		OutValue = Value->AsString();
		return true;
	}

	// required JSON integer-valued number field를 parse합니다.
	bool ParseRevisionField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		int32& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// exact numeric JSON field입니다.
		const TSharedPtr<FJsonValue> Value = RequireField(Object, FieldName, FieldPath, EJson::Number, OutIssues);
		if (!Value.IsValid())
		{
			return false;
		}
		// JSON number 원본 double 값입니다.
		const double NumberValue = Value->AsNumber();
		if (!FMath::IsFinite(NumberValue)
			|| NumberValue < static_cast<double>(MIN_int32)
			|| NumberValue > static_cast<double>(MAX_int32)
			|| static_cast<double>(static_cast<int32>(NumberValue)) != NumberValue)
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, FieldPath, TEXT("revision은 finite int32 정수여야 합니다."));
			return false;
		}
		OutValue = static_cast<int32>(NumberValue);
		return true;
	}

	// required JSON bool field를 parse합니다.
	bool ParseBoolField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		bool& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// exact boolean JSON field입니다.
		const TSharedPtr<FJsonValue> Value = RequireField(Object, FieldName, FieldPath, EJson::Boolean, OutIssues);
		if (!Value.IsValid())
		{
			return false;
		}
		OutValue = Value->AsBool();
		return true;
	}

	// required JSON number를 current authored range 안의 finite float로 parse합니다.
	bool ParseFloatField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		const double MinimumValue,
		const double MaximumValue,
		float& OutValue,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// exact numeric JSON field입니다.
		const TSharedPtr<FJsonValue> Value = RequireField(Object, FieldName, FieldPath, EJson::Number, OutIssues);
		if (!Value.IsValid())
		{
			return false;
		}
		// JSON number 원본 double 값입니다.
		const double NumberValue = Value->AsNumber();
		if (!FMath::IsFinite(NumberValue) || NumberValue < MinimumValue || NumberValue > MaximumValue)
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				FieldPath,
				FString::Printf(TEXT("raw authored 값이 허용 범위 %.6g..%.6g 밖입니다. clamp하지 않습니다."), MinimumValue, MaximumValue));
			return false;
		}

		// Unreal 저장 타입으로 변환한 authored float 값입니다.
		const float TypedValue = static_cast<float>(NumberValue);
		if (!FMath::IsFinite(TypedValue))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, FieldPath, TEXT("float 변환 결과가 finite 값이 아닙니다."));
			return false;
		}
		OutValue = TypedValue;
		return true;
	}

	// required JSON object field를 parse합니다.
	bool ParseObjectField(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		const FString& FieldPath,
		TSharedPtr<FJsonObject>& OutObject,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// exact object JSON field입니다.
		const TSharedPtr<FJsonValue> Value = RequireField(Object, FieldName, FieldPath, EJson::Object, OutIssues);
		if (!Value.IsValid())
		{
			return false;
		}
		OutObject = Value->AsObject();
		if (!OutObject.IsValid())
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::TypeMismatch, FieldPath, TEXT("유효한 JSON object가 아닙니다."));
			return false;
		}
		return true;
	}

	// P0 Literal FText object를 strict `{Kind,Text}` shape로 parse합니다.
	bool ParseLiteralText(
		const TSharedPtr<FJsonObject>& TextObject,
		const FString& FieldPath,
		FCFDAStagingLiteralText& OutText,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// Literal text object의 exact required fields입니다.
		const TArray<FString> RequiredFields = {TEXT("Kind"), TEXT("Text")};
		ValidateExactFields(TextObject, RequiredFields, FieldPath, OutIssues);

		// interchange text representation kind입니다.
		FString Kind;
		if (ParseStringField(TextObject, TEXT("Kind"), FieldPath + TEXT(".Kind"), Kind, OutIssues)
			&& !Kind.Equals(TEXT("Literal"), ESearchCase::CaseSensitive))
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::UnsupportedTextRepresentation,
				FieldPath + TEXT(".Kind"),
				TEXT("P0는 Literal FText만 지원합니다. StringTable/localization identity를 silent 변환하지 않습니다."));
		}

		ParseStringField(TextObject, TEXT("Text"), FieldPath + TEXT(".Text"), OutText.Text, OutIssues);
		return !HasBlockingIssue(OutIssues);
	}

	// exact full `/Game/.../Asset.Asset` target object path인지 검사합니다.
	bool ValidateTargetObjectPath(const FString& ObjectPath, TArray<FCFDAStagingIssue>& OutIssues)
	{
		if (!ObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
			|| ObjectPath.Contains(TEXT(":"), ESearchCase::CaseSensitive)
			|| ObjectPath.Contains(TEXT("\\"), ESearchCase::CaseSensitive))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("TargetObjectPath"), TEXT("P0 target은 exact `/Game/.../Asset.Asset` 경로여야 합니다."));
			return false;
		}

		// object separator 마지막 dot 위치입니다.
		int32 DotIndex = INDEX_NONE;
		// package path 마지막 slash 위치입니다.
		int32 SlashIndex = INDEX_NONE;
		ObjectPath.FindLastChar(TEXT('.'), DotIndex);
		ObjectPath.FindLastChar(TEXT('/'), SlashIndex);
		if (DotIndex <= SlashIndex + 1 || DotIndex >= ObjectPath.Len() - 1)
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("TargetObjectPath"), TEXT("full object path에는 package와 object 이름이 모두 필요합니다."));
			return false;
		}

		// `.Object` 앞의 long package name입니다.
		const FString PackageName = ObjectPath.Left(DotIndex);
		// package leaf asset name입니다.
		const FString PackageLeaf = ObjectPath.Mid(SlashIndex + 1, DotIndex - SlashIndex - 1);
		// dot 뒤 UObject 이름입니다.
		const FString ObjectName = ObjectPath.Mid(DotIndex + 1);
		if (!FPackageName::IsValidLongPackageName(PackageName)
			|| !PackageLeaf.Equals(ObjectName, ESearchCase::CaseSensitive))
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, TEXT("TargetObjectPath"), TEXT("package/object 이름이 canonical Unreal asset object path 규칙과 다릅니다."));
			return false;
		}
		return true;
	}

	// repository-relative Staging path를 canonical authority 아래의 slash-normalized JSON path로 변환합니다.
	bool NormalizeStagingRelativePath(const FString& InputPath, FString& OutPath)
	{
		// CF-FQ-049가 읽을 수 있는 tracked Staging authority root입니다.
		static constexpr TCHAR StagingRootPrefix[] = TEXT("Authoring/DataAssetStaging/");

		OutPath = InputPath;
		OutPath.TrimStartAndEndInline();
		OutPath.ReplaceInline(TEXT("\\"), TEXT("/"), ESearchCase::CaseSensitive);
		while (OutPath.StartsWith(TEXT("./"), ESearchCase::CaseSensitive))
		{
			OutPath = OutPath.Mid(2);
		}
		if (OutPath.IsEmpty()
			|| OutPath.StartsWith(TEXT("/"), ESearchCase::CaseSensitive)
			|| OutPath.EndsWith(TEXT("/"), ESearchCase::CaseSensitive)
			|| OutPath.Contains(TEXT("//"), ESearchCase::CaseSensitive)
			|| OutPath.Contains(TEXT(":"), ESearchCase::CaseSensitive))
		{
			return false;
		}

		// normalized path segment 목록입니다.
		TArray<FString> Segments;
		OutPath.ParseIntoArray(Segments, TEXT("/"), true);
		for (const FString& Segment : Segments)
		{
			if (Segment.IsEmpty() || Segment == TEXT(".") || Segment == TEXT(".."))
			{
				return false;
			}
		}

		// P0 canonical interchange source는 tracked Staging root 아래의 lowercase .json 파일만 허용합니다.
		if (!OutPath.StartsWith(StagingRootPrefix, ESearchCase::CaseSensitive)
			|| !OutPath.EndsWith(TEXT(".json"), ESearchCase::CaseSensitive)
			|| OutPath.Len() <= UE_ARRAY_COUNT(StagingRootPrefix) - 1 + 5)
		{
			return false;
		}
		return true;
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

	// persisted FText가 P0 Literal interchange로 loss 없이 표현 가능한지 검사하고 source codepoint sequence를 읽습니다.
	bool ReadLiteralTextFromAsset(
		const FText& SourceText,
		const FString& FieldPath,
		FCFDAStagingLiteralText& OutText,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		// persisted FText가 참조하는 StringTable identity입니다.
		FName StringTableId = NAME_None;
		// persisted FText가 참조하는 StringTable key입니다.
		FString StringTableKey;
		if (FTextInspector::GetTableIdAndKey(SourceText, StringTableId, StringTableKey))
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::UnsupportedTextRepresentation,
				FieldPath,
				TEXT("StringTable FText는 P0 Literal Staging 표현으로 localization identity를 보존할 수 없습니다."));
			return false;
		}

		// localization display string이 아닌 authored source string입니다.
		const FString* SourceString = FTextInspector::GetSourceString(SourceText);
		// persisted FText namespace identity입니다.
		const TOptional<FString> TextNamespace = FTextInspector::GetNamespace(SourceText);
		// persisted FText key identity입니다.
		const TOptional<FString> TextKey = FTextInspector::GetKey(SourceText);
		// full namespace에서 UE package namespace component를 제외한 authored namespace입니다.
		FString AuthoredNamespace;
		if (TextNamespace.IsSet() && !TextNamespace.GetValue().IsEmpty())
		{
			AuthoredNamespace = TextNamespaceUtil::StripPackageNamespace(TextNamespace.GetValue());
		}

		// P0 Literal은 source string 의미만 보존하므로 실제 authored namespace가 남는 localization text만 차단합니다.
		if (!AuthoredNamespace.IsEmpty())
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::UnsupportedTextRepresentation,
				FieldPath,
				TEXT("명시적 authored namespace localization identity가 있는 FText는 P0 Literal Staging으로 silent 변환하지 않습니다."));
			return false;
		}

		// empty/package-only namespace에서 UE가 persistence용 stable key를 부여한 경우 key는 P0 semantic fingerprint에서 제외합니다.
		(void)TextKey;
		if (SourceString != nullptr)
		{
			OutText.Text = *SourceString;
			return true;
		}
		if (SourceText.IsEmpty())
		{
			OutText.Text.Reset();
			return true;
		}

		AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::UnsupportedTextRepresentation,
			FieldPath,
			TEXT("source string을 직접 보존할 수 없는 generated/formatted FText는 P0 Literal Staging에서 지원하지 않습니다."));
		return false;
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
		AppendStringToken(OutBytes, TEXT("SchemaId"), MissilePresetSchemaId);
		AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(MissilePresetSchemaRevision));
		AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(MissilePresetAdapterRevision));
		AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), MissilePresetClassPath);
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
		if (!RecomputedFingerprint.Equals(Record.StagingSemanticFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Staging Payload와 StagingSemanticFingerprint가 서로 다른 semantic state를 나타냅니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Create/Update approval candidate의 mutable DTO 구조와 source/current binding을 다시 검증합니다.
	bool ValidateBatchCandidateIntegrity(const FCFDAStagingPreviewRow& Row, FString& OutError)
	{
		if (!Row.Record.SchemaId.Equals(MissilePresetSchemaId, ESearchCase::CaseSensitive)
			|| Row.Record.SchemaRevision != MissilePresetSchemaRevision
			|| Row.Record.AdapterContractRevision != MissilePresetAdapterRevision
			|| !Row.Record.DataAssetTypeClassPath.Equals(MissilePresetClassPath, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Batch candidate의 schema/revision/class가 current P0 계약과 다릅니다.");
			return false;
		}
		if (Row.Record.StableLogicalId.IsNone()
			|| Row.Record.Payload.PresetId.IsNone()
			|| Row.Record.StableLogicalId != Row.Record.Payload.PresetId)
		{
			OutError = TEXT("Batch candidate의 StableLogicalId와 Payload.PresetId identity 계약이 유효하지 않습니다.");
			return false;
		}

		// mutable target path를 canonical Unreal object path 규칙으로 재검증할 임시 진단입니다.
		TArray<FCFDAStagingIssue> TargetPathIssues;
		if (!ValidateTargetObjectPath(Row.Record.TargetObjectPath, TargetPathIssues))
		{
			OutError = TEXT("Batch candidate의 TargetObjectPath가 canonical Unreal asset object path가 아닙니다.");
			return false;
		}

		// mutable source path를 canonical Staging authority로 다시 정규화한 값입니다.
		FString NormalizedStagingPath;
		if (!NormalizeStagingRelativePath(Row.Record.StagingRelativePath, NormalizedStagingPath)
			|| !NormalizedStagingPath.Equals(Row.Record.StagingRelativePath, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Batch candidate의 StagingRelativePath가 canonical `Authoring/DataAssetStaging/.../*.json` source path가 아닙니다.");
			return false;
		}

		// mutable Payload와 cached desired-state fingerprint가 같은 semantic state인지 확인합니다.
		FString StagingFingerprintIntegrityError;
		if (!ValidateStagingFingerprintIntegrity(Row.Record, StagingFingerprintIntegrityError))
		{
			OutError = StagingFingerprintIntegrityError;
			return false;
		}

		if (Row.Kind == ECFDAStagingPreviewKind::Create)
		{
			if (Row.Record.bHasBaseSemanticFingerprint
				|| !Row.Record.BaseSemanticFingerprint.IsEmpty()
				|| !Row.CurrentSemanticFingerprint.IsEmpty())
			{
				OutError = TEXT("Create candidate는 Base가 없어야 하고 Current target fingerprint도 없어야 합니다.");
				return false;
			}
		}
		else if (Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			if (!Row.Record.bHasBaseSemanticFingerprint
				|| !IsCanonicalSha256Fingerprint(Row.Record.BaseSemanticFingerprint)
				|| !IsCanonicalSha256Fingerprint(Row.CurrentSemanticFingerprint))
			{
				OutError = TEXT("Update candidate는 canonical Base/Current semantic fingerprint를 모두 가져야 합니다.");
				return false;
			}
		}
		else
		{
			OutError = TEXT("Batch mutation candidate는 Create 또는 Update여야 합니다.");
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

	// exact BatchPlanHash candidate sorting key를 만듭니다.
	FString BuildBatchSortKey(const FCFDAStagingPreviewRow& Row)
	{
		return Row.Record.DataAssetTypeClassPath.ToLower()
			+ TEXT("\n") + CanonicalNameText(Row.Record.StableLogicalId)
			+ TEXT("\n") + Row.Record.TargetObjectPath.ToLower()
			+ TEXT("\n") + Row.Record.StagingRelativePath.ToLower();
	}

	// 한 Create/Update row의 approval-binding semantic fields를 BatchPlan token stream에 append합니다.
	void AppendBatchTargetTokens(TArray<uint8>& OutBytes, const FCFDAStagingPreviewRow& Row)
	{
		AppendStringToken(OutBytes, TEXT("SchemaId"), Row.Record.SchemaId);
		AppendStringToken(OutBytes, TEXT("SchemaRevision"), FString::FromInt(Row.Record.SchemaRevision));
		AppendStringToken(OutBytes, TEXT("AdapterContractRevision"), FString::FromInt(Row.Record.AdapterContractRevision));
		AppendStringToken(OutBytes, TEXT("DataAssetTypeClassPath"), Row.Record.DataAssetTypeClassPath);
		AppendStringToken(OutBytes, TEXT("IdentityPolicy"), TEXT("Required"));
		AppendStringToken(OutBytes, TEXT("StableLogicalId"), CanonicalNameText(Row.Record.StableLogicalId));
		AppendStringToken(OutBytes, TEXT("TargetObjectPath"), Row.Record.TargetObjectPath);
		AppendStringToken(OutBytes, TEXT("StagingRelativePath"), Row.Record.StagingRelativePath);
		AppendStringToken(OutBytes, TEXT("BaseSemanticFingerprint"), Row.Record.bHasBaseSemanticFingerprint ? Row.Record.BaseSemanticFingerprint : TEXT("<none>"));
		AppendStringToken(OutBytes, TEXT("CurrentSemanticFingerprint"), Row.CurrentSemanticFingerprint.IsEmpty() ? TEXT("<absent-target>") : Row.CurrentSemanticFingerprint);
		AppendStringToken(OutBytes, TEXT("StagingSemanticFingerprint"), Row.Record.StagingSemanticFingerprint);
		AppendStringToken(OutBytes, TEXT("PlannedOperation"), PreviewKindToken(Row.Kind));
	}
}

// P0 MissileGuidePreset schema identity를 반환합니다.
const TCHAR* FCFDAStagingService::GetMissilePresetSchemaId()
{
	return CFDAStagingPrivate::MissilePresetSchemaId;
}

// P0 MissileGuidePreset JSON shape revision을 반환합니다.
int32 FCFDAStagingService::GetMissilePresetSchemaRevision()
{
	return CFDAStagingPrivate::MissilePresetSchemaRevision;
}

// P0 MissileGuidePreset typed adapter semantic revision을 반환합니다.
int32 FCFDAStagingService::GetMissilePresetAdapterRevision()
{
	return CFDAStagingPrivate::MissilePresetAdapterRevision;
}

// P0 MissileGuidePreset exact native class path를 반환합니다.
const TCHAR* FCFDAStagingService::GetMissilePresetClassPath()
{
	return CFDAStagingPrivate::MissilePresetClassPath;
}

// strict whole-record JSON을 typed MissileGuidePreset record로 parse/canonicalize합니다.
FCFDAStagingParseResult FCFDAStagingService::ParseMissilePresetJson(
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

	// BaseSemanticFingerprint JSON field입니다. Create는 required null, Update는 required string입니다.
	const TSharedPtr<FJsonValue> BaseFingerprintValue = RootObject->GetFieldUntyped(TEXT("BaseSemanticFingerprint"));
	if (BaseFingerprintValue.IsValid())
	{
		if (BaseFingerprintValue->Type == EJson::Null)
		{
			Result.Record.bHasBaseSemanticFingerprint = false;
			Result.Record.BaseSemanticFingerprint.Reset();
		}
		else if (BaseFingerprintValue->Type == EJson::String)
		{
			Result.Record.bHasBaseSemanticFingerprint = true;
			Result.Record.BaseSemanticFingerprint = BaseFingerprintValue->AsString();
			if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Result.Record.BaseSemanticFingerprint))
			{
				CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("BaseSemanticFingerprint"), TEXT("BaseSemanticFingerprint는 `sha256:` + 64 lowercase hex 형식이어야 합니다."));
			}
		}
		else
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::TypeMismatch, TEXT("BaseSemanticFingerprint"), TEXT("Create는 null, Update는 canonical sha256 string이어야 합니다."));
		}
	}

	// strict Payload JSON object입니다.
	TSharedPtr<FJsonObject> PayloadObject;
	if (CFDAStagingPrivate::ParseObjectField(RootObject, TEXT("Payload"), TEXT("Payload"), PayloadObject, Result.Issues))
	{
		CFDAStagingPrivate::ParseMissilePresetPayload(PayloadObject, Result.Record.Payload, Result.Issues);
	}

	if (!Result.Record.SchemaId.Equals(CFDAStagingPrivate::MissilePresetSchemaId, ESearchCase::CaseSensitive))
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::SchemaUnsupported, TEXT("SchemaId"), TEXT("지원하지 않는 Staging schema입니다."));
	}
	if (Result.Record.SchemaRevision != CFDAStagingPrivate::MissilePresetSchemaRevision)
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported, TEXT("SchemaRevision"), TEXT("현재 parser와 정확히 같은 SchemaRevision만 지원합니다."));
	}
	if (Result.Record.AdapterContractRevision != CFDAStagingPrivate::MissilePresetAdapterRevision)
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::AdapterRevisionMismatch, TEXT("AdapterContractRevision"), TEXT("현재 typed adapter와 정확히 같은 AdapterContractRevision만 지원합니다."));
	}
	if (!Result.Record.DataAssetTypeClassPath.Equals(CFDAStagingPrivate::MissilePresetClassPath, ESearchCase::CaseSensitive))
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::SchemaUnsupported, TEXT("DataAssetTypeClassPath"), TEXT("P0 Pilot은 CFMissileGuidePresetData exact class만 지원합니다."));
	}
	if (!Result.Record.StableLogicalId.IsNone()
		&& !Result.Record.Payload.PresetId.IsNone()
		&& Result.Record.StableLogicalId != Result.Record.Payload.PresetId)
	{
		CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::StableIdentityMismatch, TEXT("StableLogicalId"), TEXT("StableLogicalId와 Payload.PresetId가 같은 FName identity가 아닙니다."));
	}

	if (!StagingRelativePath.IsEmpty())
	{
		if (!CFDAStagingPrivate::NormalizeStagingRelativePath(StagingRelativePath, Result.Record.StagingRelativePath))
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingRelativePath"), TEXT("Staging source path는 `Authoring/DataAssetStaging/` 아래의 안전한 main_game-relative `.json` 경로여야 합니다."));
		}
	}

	if (!CFDAStagingPrivate::HasBlockingIssue(Result.Issues))
	{
		// typed desired payload의 derived semantic fingerprint 생성 오류입니다.
		FString FingerprintError;
		if (!BuildSemanticFingerprint(Result.Record.Payload, Result.Record.StagingSemanticFingerprint, FingerprintError))
		{
			CFDAStagingPrivate::AddIssue(Result.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("Payload"), FingerprintError);
		}
	}

	Result.bValid = !CFDAStagingPrivate::HasBlockingIssue(Result.Issues);
	return Result;
}

// typed MissileGuidePreset whole-record payload를 deterministic SHA-256 semantic fingerprint로 변환합니다.
bool FCFDAStagingService::BuildSemanticFingerprint(
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
bool FCFDAStagingService::ExtractMissilePresetPayload(
	const UCFMissileGuidePresetData& PresetAsset,
	FCFDAMissilePresetPayload& OutPayload,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutIssues.Reset();
	return CFDAStagingPrivate::BuildMissilePresetPayloadFromAsset(PresetAsset, OutPayload, OutIssues);
}

// Asset Registry와 typed MissileGuidePreset payload만 사용해 exact target/StableIdentity/current fingerprint를 read-only로 해석합니다.
bool FCFDAStagingService::ResolveMissilePresetCurrentState(
	const FCFDAStagingRecord& Record,
	FCFDAStagingCurrentState& OutCurrentState,
	TArray<FCFDAStagingIssue>& OutIssues)
{
	OutCurrentState = FCFDAStagingCurrentState();
	OutIssues.Reset();

	if (!Record.SchemaId.Equals(CFDAStagingPrivate::MissilePresetSchemaId, ESearchCase::CaseSensitive)
		|| Record.SchemaRevision != CFDAStagingPrivate::MissilePresetSchemaRevision
		|| Record.AdapterContractRevision != CFDAStagingPrivate::MissilePresetAdapterRevision
		|| !Record.DataAssetTypeClassPath.Equals(CFDAStagingPrivate::MissilePresetClassPath, ESearchCase::CaseSensitive))
	{
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::SchemaUnsupported,
			TEXT("DataAssetTypeClassPath"),
			TEXT("current resolver는 current P0 CFMissileGuidePresetData schema/adapter/class 계약만 지원합니다."));
		return false;
	}
	if (Record.StableLogicalId.IsNone())
	{
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::StableIdentityMissing,
			TEXT("StableLogicalId"),
			TEXT("current resolver에는 required StableLogicalId가 필요합니다."));
		return false;
	}
	if (!CFDAStagingPrivate::ValidateTargetObjectPath(Record.TargetObjectPath, OutIssues))
	{
		return false;
	}

	// CF-FQ-045 public Registry coverage를 current resolver가 요구하는 metadata authority로 준비합니다.
	FCFDATypeRegistry TypeRegistry;
	// current concrete descriptor 등록 실패 원인입니다.
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
	const FCFDASemanticDescriptor* Descriptor = TypeRegistry.FindDescriptor(Record.DataAssetTypeClassPath);
	if (Descriptor == nullptr
		|| Descriptor->IdentityPolicy != ECFDAIdentityPolicy::Required
		|| Descriptor->IdentityResolverKind != ECFDAIdentityResolverKind::ExplicitFName
		|| Descriptor->IdentitySourceName != TEXT("PresetId"))
	{
		CFDAStagingPrivate::AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::InvalidValue,
			TEXT("Registry.Identity"),
			TEXT("CFMissileGuidePresetData Registry identity 계약이 Required/ExplicitFName/PresetId와 다릅니다."));
		return false;
	}

	// read-only exact target/identity 조회에 사용할 Project Asset Registry입니다.
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	// exact requested target object path입니다.
	const FSoftObjectPath TargetObjectPath(Record.TargetObjectPath);
	// 이미 메모리에 존재하는 exact target UObject입니다. ResolveObject는 load/mutation을 수행하지 않습니다.
	UObject* ResolvedTargetObject = TargetObjectPath.ResolveObject();
	// persisted/registered exact target metadata입니다.
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

		if (OutCurrentState.RequestedTargetClassPath.Equals(CFDAStagingPrivate::MissilePresetClassPath, ESearchCase::CaseSensitive))
		{
			// current exact class target를 semantic readback하기 위해 load한 read-only UObject입니다.
			UObject* LoadedTargetObject = ResolvedTargetObject != nullptr ? ResolvedTargetObject : TargetAssetData.GetAsset();
			// exact expected DataAsset 타입으로 확인한 current target입니다.
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
			// current exact target를 소유하는 package입니다.
			const UPackage* CurrentPackage = CurrentPreset->GetOutermost();
			OutCurrentState.bRequestedTargetDirty = CurrentPackage != nullptr && CurrentPackage->IsDirty();

			// current persisted/loaded asset의 raw whole-record typed payload입니다.
			FCFDAMissilePresetPayload CurrentPayload;
			if (!ExtractMissilePresetPayload(*CurrentPreset, CurrentPayload, OutIssues))
			{
				return false;
			}
			// current semantic fingerprint 생성 오류입니다.
			FString CurrentFingerprintError;
			if (!BuildSemanticFingerprint(CurrentPayload, OutCurrentState.CurrentSemanticFingerprint, CurrentFingerprintError))
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
			|| LoadedPreset->PresetId != Record.StableLogicalId)
		{
			continue;
		}

		// loaded object의 exact soft object identity입니다.
		const FString LoadedObjectPath = FSoftObjectPath(LoadedPreset).ToString();
		if (!LoadedObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive))
		{
			continue;
		}
		StableIdentityObjectPaths.Add(LoadedObjectPath);
	}

	// current Project 안의 exact CFMissileGuidePresetData 전체에서 StableLogicalId를 찾는 metadata+typed read-only filter입니다.
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

	// current exact Pilot class asset metadata 목록입니다. 0건은 valid absent current truth입니다.
	TArray<FAssetData> MissilePresetAssets;
	// filter는 StaticClass 기반으로 내부 고정되므로 query result bool과 무관하게 empty result를 정상 absent로 소비합니다.
	(void)AssetRegistry.GetAssets(IdentityFilter, MissilePresetAssets, false);

	for (const FAssetData& MissilePresetAssetData : MissilePresetAssets)
	{
		// StableLogicalId 비교를 위해 read-only load한 exact class DataAsset입니다.
		const UCFMissileGuidePresetData* CandidatePreset = Cast<UCFMissileGuidePresetData>(MissilePresetAssetData.GetAsset());
		if (CandidatePreset == nullptr || CandidatePreset->PresetId != Record.StableLogicalId)
		{
			continue;
		}

		StableIdentityObjectPaths.Add(MissilePresetAssetData.GetSoftObjectPath().ToString());
	}

	OutCurrentState.StableIdentityMatchCount = StableIdentityObjectPaths.Num();
	OutCurrentState.bStableIdentityExists = OutCurrentState.StableIdentityMatchCount > 0;
	if (OutCurrentState.StableIdentityMatchCount == 1)
	{
		// single identity match의 exact current object path입니다.
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

// valid typed record와 current truth를 P0 exact 3-way 규칙으로 mutation0 분류합니다.
FCFDAStagingPreviewRow FCFDAStagingService::BuildPreview(
	const FCFDAStagingRecord& Record,
	const FCFDAStagingCurrentState& CurrentState)
{
	// 반환할 mutation0 Preview row입니다.
	FCFDAStagingPreviewRow Row;
	Row.Record = Record;
	Row.CurrentSemanticFingerprint = CurrentState.CurrentSemanticFingerprint;

	if (!Record.SchemaId.Equals(CFDAStagingPrivate::MissilePresetSchemaId, ESearchCase::CaseSensitive)
		|| Record.SchemaRevision != CFDAStagingPrivate::MissilePresetSchemaRevision
		|| Record.AdapterContractRevision != CFDAStagingPrivate::MissilePresetAdapterRevision
		|| !Record.DataAssetTypeClassPath.Equals(CFDAStagingPrivate::MissilePresetClassPath, ESearchCase::CaseSensitive))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::SchemaUnsupported, TEXT("SchemaId"), TEXT("Preview input record가 current P0 MissileGuidePreset schema/adapter/class 계약과 다릅니다."));
		return Row;
	}
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
	if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Record.StagingSemanticFingerprint))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingSemanticFingerprint"), TEXT("Preview input record의 StagingSemanticFingerprint가 canonical SHA-256이 아닙니다."));
		return Row;
	}
	if (!CFDAStagingPrivate::ValidateTargetObjectPath(Record.TargetObjectPath, Row.Issues))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		return Row;
	}

	// mutable DTO의 Payload와 cached fingerprint가 갈라진 상태를 approval 이전에 fail-closed합니다.
	FString StagingFingerprintIntegrityError;
	if (!CFDAStagingPrivate::ValidateStagingFingerprintIntegrity(Record, StagingFingerprintIntegrityError))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("StagingSemanticFingerprint"), StagingFingerprintIntegrityError);
		return Row;
	}

	if (CurrentState.StableIdentityMatchCount > 1)
	{
		Row.Kind = ECFDAStagingPreviewKind::Conflict;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::DuplicateStableIdentity, TEXT("StableLogicalId"), TEXT("current Project에 같은 StableLogicalId의 CFMissileGuidePresetData가 둘 이상 존재합니다."));
		return Row;
	}

	if (CurrentState.bStableIdentityExists
		&& !CurrentState.StableIdentityObjectPath.IsEmpty()
		&& !CurrentState.StableIdentityObjectPath.Equals(Record.TargetObjectPath, ESearchCase::IgnoreCase))
	{
		Row.Kind = ECFDAStagingPreviewKind::Conflict;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::TargetMoved, TEXT("TargetObjectPath"), TEXT("같은 StableLogicalId가 다른 current object path에 존재합니다."));
		return Row;
	}

	if (CurrentState.bRequestedTargetExists)
	{
		if (!CurrentState.RequestedTargetClassPath.Equals(Record.DataAssetTypeClassPath, ESearchCase::CaseSensitive)
			|| CurrentState.RequestedTargetStableLogicalId != Record.StableLogicalId)
		{
			Row.Kind = ECFDAStagingPreviewKind::Conflict;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::PathCollision, TEXT("TargetObjectPath"), TEXT("requested path에 다른 class 또는 stable identity의 object가 존재합니다."));
			return Row;
		}
		if (CurrentState.bRequestedTargetDirty)
		{
			Row.Kind = ECFDAStagingPreviewKind::Conflict;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::TargetDirtyUnowned, TEXT("TargetObjectPath"), TEXT("pre-existing dirty target은 CF-FQ-049 save ownership 밖이므로 Preview에서 차단합니다."));
			return Row;
		}
	}

	if (!Record.bHasBaseSemanticFingerprint)
	{
		if (CurrentState.bRequestedTargetExists || CurrentState.bStableIdentityExists)
		{
			Row.Kind = ECFDAStagingPreviewKind::Conflict;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::UnexpectedExistingTarget, TEXT("BaseSemanticFingerprint"), TEXT("Create-intent record인데 target 또는 identity가 이미 존재합니다."));
			return Row;
		}
		Row.Kind = ECFDAStagingPreviewKind::Create;
		Row.CurrentSemanticFingerprint.Reset();
		return Row;
	}

	if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(Record.BaseSemanticFingerprint))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::BaselineMissing, TEXT("BaseSemanticFingerprint"), TEXT("Update-intent record에는 canonical BaseSemanticFingerprint가 필요합니다."));
		return Row;
	}
	if (!CurrentState.bRequestedTargetExists)
	{
		Row.Kind = ECFDAStagingPreviewKind::Conflict;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::TargetMissing, TEXT("TargetObjectPath"), TEXT("Update baseline은 존재하지만 requested current target이 없습니다."));
		return Row;
	}
	if (!CFDAStagingPrivate::IsCanonicalSha256Fingerprint(CurrentState.CurrentSemanticFingerprint))
	{
		Row.Kind = ECFDAStagingPreviewKind::Invalid;
		CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::InvalidValue, TEXT("CurrentSemanticFingerprint"), TEXT("current target fingerprint가 canonical SHA-256이 아닙니다."));
		return Row;
	}

	// Update Staging을 만들 때 캡처한 semantic baseline입니다.
	const FString& BaseFingerprint = Record.BaseSemanticFingerprint;
	// Preview 직전 current Unreal semantic state입니다.
	const FString& CurrentFingerprint = CurrentState.CurrentSemanticFingerprint;
	// Staging JSON desired semantic state입니다.
	const FString& StagingFingerprint = Record.StagingSemanticFingerprint;

	if (BaseFingerprint == CurrentFingerprint)
	{
		Row.Kind = StagingFingerprint == CurrentFingerprint
			? ECFDAStagingPreviewKind::NoChange
			: ECFDAStagingPreviewKind::Update;
		return Row;
	}
	if (CurrentFingerprint == StagingFingerprint)
	{
		Row.Kind = ECFDAStagingPreviewKind::NoChange;
		CFDAStagingPrivate::AddIssue(
			Row.Issues,
			ECFDAStagingIssueCode::BaselineRebaseRequired,
			TEXT("BaseSemanticFingerprint"),
			TEXT("current가 이미 Staging desired state로 수렴했습니다. mutation은 0이지만 다음 편집 전 explicit baseline rebase가 필요합니다."),
			false);
		return Row;
	}

	Row.Kind = ECFDAStagingPreviewKind::Conflict;
	CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::BaselineMismatch, TEXT("BaseSemanticFingerprint"), TEXT("Base와 Current가 달라 외부 current drift를 overwrite하지 않습니다."));
	return Row;
}

// 같은 batch 안의 duplicate stable identity/target path를 fail-closed Invalid로 승격합니다.
void FCFDAStagingService::ApplyBatchDuplicateValidation(TArray<FCFDAStagingPreviewRow>& InOutRows)
{
	// canonical stable identity별 row index 목록입니다.
	TMap<FString, TArray<int32>> IdentityRows;
	// case-insensitive target object path별 row index 목록입니다.
	TMap<FString, TArray<int32>> TargetPathRows;

	for (int32 RowIndex = 0; RowIndex < InOutRows.Num(); ++RowIndex)
	{
		// 현재 Preview row입니다.
		const FCFDAStagingPreviewRow& Row = InOutRows[RowIndex];
		if (!Row.Record.StableLogicalId.IsNone())
		{
			IdentityRows.FindOrAdd(CFDAStagingPrivate::CanonicalNameText(Row.Record.StableLogicalId)).Add(RowIndex);
		}
		if (!Row.Record.TargetObjectPath.IsEmpty())
		{
			TargetPathRows.FindOrAdd(Row.Record.TargetObjectPath.ToLower()).Add(RowIndex);
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
			FCFDAStagingPreviewRow& Row = InOutRows[RowIndex];
			Row.Kind = ECFDAStagingPreviewKind::Invalid;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::DuplicateStableIdentity, TEXT("StableLogicalId"), TEXT("같은 batch 안에 duplicate StableLogicalId가 있습니다."));
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
			FCFDAStagingPreviewRow& Row = InOutRows[RowIndex];
			Row.Kind = ECFDAStagingPreviewKind::Invalid;
			CFDAStagingPrivate::AddIssue(Row.Issues, ECFDAStagingIssueCode::DuplicateTargetPath, TEXT("TargetObjectPath"), TEXT("같은 batch 안에 duplicate TargetObjectPath가 있습니다."));
		}
	}
}

// exact Create/Update candidate set을 deterministic order로 묶어 SHA-256 BatchPlanHash를 계산합니다.
bool FCFDAStagingService::BuildBatchPlanHash(
	const TArray<FCFDAStagingPreviewRow>& PreviewRows,
	FString& OutBatchPlanHash,
	FString& OutError)
{
	// caller가 별도 duplicate-validation 호출을 빼먹어도 hash authority가 fail-closed하도록 복사한 rows입니다.
	TArray<FCFDAStagingPreviewRow> ValidatedRows = PreviewRows;
	ApplyBatchDuplicateValidation(ValidatedRows);

	for (const FCFDAStagingPreviewRow& Row : ValidatedRows)
	{
		if (Row.Kind == ECFDAStagingPreviewKind::Conflict || Row.Kind == ECFDAStagingPreviewKind::Invalid)
		{
			OutBatchPlanHash.Reset();
			OutError = TEXT("Conflict/Invalid가 포함된 Preview set에는 approval BatchPlanHash를 만들 수 없습니다.");
			return false;
		}

		if (Row.Kind == ECFDAStagingPreviewKind::Create || Row.Kind == ECFDAStagingPreviewKind::Update)
		{
			// Preview 이후 mutable DTO의 payload/path/intent가 변조된 split state를 approval hash가 수용하지 않게 합니다.
			FString CandidateIntegrityError;
			if (!CFDAStagingPrivate::ValidateBatchCandidateIntegrity(Row, CandidateIntegrityError))
			{
				OutBatchPlanHash.Reset();
				OutError = CandidateIntegrityError;
				return false;
			}
		}
	}

	// Create/Update mutation candidate만 복사한 exact included target set입니다.
	TArray<FCFDAStagingPreviewRow> CandidateRows;
	for (const FCFDAStagingPreviewRow& Row : ValidatedRows)
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

	CandidateRows.Sort([](const FCFDAStagingPreviewRow& Left, const FCFDAStagingPreviewRow& Right)
	{
		return CFDAStagingPrivate::BuildBatchSortKey(Left) < CFDAStagingPrivate::BuildBatchSortKey(Right);
	});

	// exact sorted target-plan canonical token stream입니다.
	TArray<uint8> CanonicalBytes;
	CanonicalBytes.Reserve(CandidateRows.Num() * 1024);
	CFDAStagingPrivate::AppendStringToken(CanonicalBytes, TEXT("Kind"), CFDAStagingPrivate::BatchPlanKind);
	CFDAStagingPrivate::AppendStringToken(CanonicalBytes, TEXT("FormatRevision"), FString::FromInt(CFDAStagingPrivate::BatchPlanFormatRevision));
	CFDAStagingPrivate::AppendStringToken(CanonicalBytes, TEXT("IncludedTargetCount"), FString::FromInt(CandidateRows.Num()));

	for (const FCFDAStagingPreviewRow& Row : CandidateRows)
	{
		CFDAStagingPrivate::AppendBatchTargetTokens(CanonicalBytes, Row);
	}
	return CFDAStagingPrivate::HashCanonicalBytes(CanonicalBytes, OutBatchPlanHash, OutError);
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
	CFDAStagingPrivate::FScopedSemanticTokenProbe ScopedProbe(OutTokenLabels);
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
