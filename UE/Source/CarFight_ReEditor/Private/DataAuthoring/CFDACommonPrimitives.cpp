// Copyright (c) CarFight. All Rights Reserved.
// File: CFDACommonPrimitives.cpp
// Version: v1.1.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-02 provider-neutral Authoring primitive implementation입니다.
// Changelog:
// - v1.1.0: common envelope TargetObjectPath canonical validation과 BaseSemanticFingerprint null/string + canonical SHA-256 parse authority를 추가.
// - v1.0.0: shared issue/JSON/Literal FText/fingerprint/SoftObjectPath/Asset Registry metadata-only validation authority를 구현.
// Migration:
// - referenced asset validation은 Asset Registry metadata만 사용하며 LoadObject/ResolveObject를 호출하지 않습니다.
// - 기존 Missile accepted fingerprint token framing과 Literal FText source semantics를 그대로 보존합니다.

#include "CFDACommonPrimitives.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"
#include "Modules/ModuleManager.h"

// Unreal의 namespace UI와 OpenSSL 전역 UI typedef 충돌을 third-party include 구간에서만 격리합니다.
#define UI CF_OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
THIRD_PARTY_INCLUDES_END
#undef UI

namespace CFDACommonPrimitivesPrivate
{
#if WITH_DEV_AUTOMATION_TESTS
	// 현재 thread의 production fingerprint path에서 실제 emit된 token label을 관측할 temporary sink입니다.
	thread_local TArray<FString>* GSemanticTokenLabelProbe = nullptr;
#endif

	// uint32를 network-order big-endian 4-byte sequence로 append합니다.
	void AppendUint32BigEndian(TArray<uint8>& OutBytes, const uint32 Value)
	{
		OutBytes.Add(static_cast<uint8>((Value >> 24) & 0xff));
		OutBytes.Add(static_cast<uint8>((Value >> 16) & 0xff));
		OutBytes.Add(static_cast<uint8>((Value >> 8) & 0xff));
		OutBytes.Add(static_cast<uint8>(Value & 0xff));
	}

	// label + length + raw bytes를 accepted semantic token framing으로 append합니다.
	void AppendRawToken(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const uint8* ValueBytes,
		const int32 ValueByteCount)
	{
#if WITH_DEV_AUTOMATION_TESTS
		if (GSemanticTokenLabelProbe != nullptr)
		{
			GSemanticTokenLabelProbe->Add(Label);
		}
#endif

		// token label의 UTF-8 변환입니다.
		FTCHARToUTF8 LabelUtf8(Label);
		// token label의 byte 수입니다.
		const uint32 LabelByteCount = static_cast<uint32>(LabelUtf8.Length());
		AppendUint32BigEndian(OutBytes, LabelByteCount);
		OutBytes.Append(reinterpret_cast<const uint8*>(LabelUtf8.Get()), static_cast<int32>(LabelByteCount));
		AppendUint32BigEndian(OutBytes, static_cast<uint32>(ValueByteCount));
		if (ValueByteCount > 0)
		{
			OutBytes.Append(ValueBytes, ValueByteCount);
		}
	}
}

namespace CFDACommonPrimitives
{
	// blocking 또는 informational issue를 target 배열에 추가합니다.
	void AddIssue(
		TArray<FCFDAStagingIssue>& OutIssues,
		const ECFDAStagingIssueCode Code,
		const FString& FieldPath,
		const FString& Message,
		const bool bBlocking)
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

	// semantic string token을 accepted length-prefixed byte protocol로 append합니다.
	void AppendStringToken(TArray<uint8>& OutBytes, const TCHAR* Label, const FString& Value)
	{
		// semantic string의 UTF-8 변환입니다.
		FTCHARToUTF8 ValueUtf8(*Value);
		CFDACommonPrimitivesPrivate::AppendRawToken(
			OutBytes,
			Label,
			reinterpret_cast<const uint8*>(ValueUtf8.Get()),
			ValueUtf8.Length());
	}

	// semantic bool token을 accepted byte protocol로 append합니다.
	void AppendBoolToken(TArray<uint8>& OutBytes, const TCHAR* Label, const bool bValue)
	{
		// accepted bool wire value입니다.
		const uint8 BoolByte = bValue ? 1 : 0;
		CFDACommonPrimitivesPrivate::AppendRawToken(OutBytes, Label, &BoolByte, 1);
	}

	// semantic float token을 -0→+0 정규화한 IEEE-754 big-endian accepted byte protocol로 append합니다.
	void AppendFloatToken(TArray<uint8>& OutBytes, const TCHAR* Label, const float Value)
	{
		// 기존 Missile accepted semantic과 동일하게 -0.0f와 +0.0f를 하나의 +0.0f 의미로 정규화한 값입니다.
		const float CanonicalValue = Value == 0.0f ? 0.0f : Value;
		// canonical float bit pattern의 raw uint32 view입니다.
		uint32 FloatBits = 0;
		static_assert(sizeof(FloatBits) == sizeof(CanonicalValue), "float semantic token은 32-bit IEEE-754 저장을 요구합니다.");
		FMemory::Memcpy(&FloatBits, &CanonicalValue, sizeof(FloatBits));
		// accepted big-endian float bytes입니다.
		const uint8 FloatBytes[4] =
		{
			static_cast<uint8>((FloatBits >> 24) & 0xff),
			static_cast<uint8>((FloatBits >> 16) & 0xff),
			static_cast<uint8>((FloatBits >> 8) & 0xff),
			static_cast<uint8>(FloatBits & 0xff)
		};
		CFDACommonPrimitivesPrivate::AppendRawToken(OutBytes, Label, FloatBytes, UE_ARRAY_COUNT(FloatBytes));
	}

	// FName semantic equality와 맞추기 위해 comparison text를 lowercase로 canonicalize합니다.
	FString CanonicalNameText(const FName Value)
	{
		return Value.ToString().ToLower();
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

	// canonical byte stream을 SHA-256 protocol fingerprint로 변환합니다.
	bool HashCanonicalBytes(const TArray<uint8>& Bytes, FString& OutFingerprint, FString& OutError)
	{
		// OpenSSL EVP가 채울 32-byte SHA-256 결과입니다.
		FSHA256Signature Signature;
		// OpenSSL EVP가 반환하는 실제 digest byte 수입니다.
		unsigned int DigestLength = 0;
		// empty buffer에서도 안전한 non-null input byte입니다.
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

	// provider-local typed payload에서 다시 계산한 fingerprint와 cached fingerprint의 exact integrity를 검증합니다.
	bool ValidateCachedSemanticFingerprint(
		const FString& CachedFingerprint,
		const FString& RecomputedFingerprint,
		FString& OutError)
	{
		if (!IsCanonicalSha256Fingerprint(CachedFingerprint)
			|| !IsCanonicalSha256Fingerprint(RecomputedFingerprint)
			|| !CachedFingerprint.Equals(RecomputedFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("provider-local typed payload와 cached StagingSemanticFingerprint가 exact 일치하지 않습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// common envelope TargetObjectPath가 exact `/Game/.../Asset.Asset` canonical object path인지 검사합니다.
	bool ValidateTargetObjectPath(
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		if (!ObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
			|| ObjectPath.Contains(TEXT(":"), ESearchCase::CaseSensitive)
			|| ObjectPath.Contains(TEXT("\\"), ESearchCase::CaseSensitive))
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				TEXT("TargetObjectPath"),
				TEXT("P0 target은 exact `/Game/.../Asset.Asset` 경로여야 합니다."));
			return false;
		}

		// Object separator 마지막 dot 위치입니다.
		int32 DotIndex = INDEX_NONE;
		// Package path 마지막 slash 위치입니다.
		int32 SlashIndex = INDEX_NONE;
		ObjectPath.FindLastChar(TEXT('.'), DotIndex);
		ObjectPath.FindLastChar(TEXT('/'), SlashIndex);
		if (DotIndex <= SlashIndex + 1 || DotIndex >= ObjectPath.Len() - 1)
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				TEXT("TargetObjectPath"),
				TEXT("full object path에는 package와 object 이름이 모두 필요합니다."));
			return false;
		}

		// `.Object` 앞의 long package name입니다.
		const FString PackageName = ObjectPath.Left(DotIndex);
		// Package leaf asset name입니다.
		const FString PackageLeaf = ObjectPath.Mid(SlashIndex + 1, DotIndex - SlashIndex - 1);
		// Dot 뒤 UObject 이름입니다.
		const FString ObjectName = ObjectPath.Mid(DotIndex + 1);
		if (!FPackageName::IsValidLongPackageName(PackageName)
			|| !PackageLeaf.Equals(ObjectName, ESearchCase::CaseSensitive))
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				TEXT("TargetObjectPath"),
				TEXT("package/object 이름이 canonical Unreal asset object path 규칙과 다릅니다."));
			return false;
		}
		return true;
	}

	// common envelope BaseSemanticFingerprint의 required null/string physical contract와 canonical SHA-256을 parse합니다.
	void ParseBaseSemanticFingerprint(
		const TSharedPtr<FJsonObject>& RootObject,
		bool& bOutHasBaseSemanticFingerprint,
		FString& OutBaseSemanticFingerprint,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		bOutHasBaseSemanticFingerprint = false;
		OutBaseSemanticFingerprint.Reset();
		if (!RootObject.IsValid())
		{
			return;
		}

		// Required BaseSemanticFingerprint physical JSON value입니다.
		const TSharedPtr<FJsonValue> BaseFingerprintValue = RootObject->GetFieldUntyped(TEXT("BaseSemanticFingerprint"));
		if (!BaseFingerprintValue.IsValid())
		{
			return;
		}
		if (BaseFingerprintValue->Type == EJson::Null)
		{
			return;
		}
		if (BaseFingerprintValue->Type == EJson::String)
		{
			bOutHasBaseSemanticFingerprint = true;
			OutBaseSemanticFingerprint = BaseFingerprintValue->AsString();
			if (!IsCanonicalSha256Fingerprint(OutBaseSemanticFingerprint))
			{
				AddIssue(
					OutIssues,
					ECFDAStagingIssueCode::InvalidValue,
					TEXT("BaseSemanticFingerprint"),
					TEXT("BaseSemanticFingerprint는 `sha256:` + 64 lowercase hex 형식이어야 합니다."));
			}
			return;
		}

		AddIssue(
			OutIssues,
			ECFDAStagingIssueCode::TypeMismatch,
			TEXT("BaseSemanticFingerprint"),
			TEXT("Create는 null, Update는 canonical sha256 string이어야 합니다."));
	}

	// JSON object가 exact required field set만 가지는지 검사합니다.
	bool ValidateExactFields(
		const TSharedPtr<FJsonObject>& Object,
		const TArray<FString>& RequiredFields,
		const FString& ObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		if (!Object.IsValid())
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::TypeMismatch, ObjectPath, TEXT("유효한 JSON object가 아닙니다."));
			return false;
		}

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
		if (!Object.IsValid())
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::TypeMismatch, FieldPath, TEXT("JSON object가 유효하지 않습니다."));
			return nullptr;
		}

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

	// required JSON number를 requested finite float range로 strict parse합니다.
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

	// Literal FText object를 strict `{Kind,Text}` shape로 parse합니다.
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
				TEXT("Literal FText만 지원합니다. StringTable/localization identity를 silent 변환하지 않습니다."));
		}

		ParseStringField(TextObject, TEXT("Text"), FieldPath + TEXT(".Text"), OutText.Text, OutIssues);
		return !HasBlockingIssue(OutIssues);
	}

	// persisted FText가 Literal interchange로 loss 없이 표현 가능한지 검사하고 source codepoint sequence를 읽습니다.
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
				TEXT("StringTable FText는 Literal Staging 표현으로 localization identity를 보존할 수 없습니다."));
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

		if (!AuthoredNamespace.IsEmpty())
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::UnsupportedTextRepresentation,
				FieldPath,
				TEXT("명시적 authored namespace localization identity가 있는 FText는 Literal Staging으로 silent 변환하지 않습니다."));
			return false;
		}

		// empty/package-only namespace에서 UE가 persistence용 stable key를 부여한 경우 key는 semantic fingerprint에서 제외합니다.
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
			TEXT("source string을 직접 보존할 수 없는 generated/formatted FText는 Literal Staging에서 지원하지 않습니다."));
		return false;
	}

	// nullable string을 load 없이 canonical top-level SoftObjectPath로 strict parse합니다.
	bool ParseCanonicalSoftObjectPath(
		const FString& PathText,
		const FString& FieldPath,
		const bool bAllowNull,
		FSoftObjectPath& OutObjectPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutObjectPath.Reset();
		if (PathText.IsEmpty())
		{
			if (bAllowNull)
			{
				return true;
			}
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, FieldPath, TEXT("required SoftObjectPath가 비어 있습니다."));
			return false;
		}

		// 입력 string으로 구성한 candidate SoftObjectPath입니다.
		const FSoftObjectPath CandidatePath(PathText);
		// candidate의 top-level asset path입니다.
		const FTopLevelAssetPath CandidateAssetPath = CandidatePath.GetAssetPath();
		// candidate가 가리키는 long package name입니다.
		const FString PackageName = CandidateAssetPath.GetPackageName().ToString();
		if (CandidatePath.IsNull()
			|| CandidateAssetPath.IsNull()
			|| !CandidatePath.GetSubPathString().IsEmpty()
			|| !FPackageName::IsValidLongPackageName(PackageName)
			|| !CandidatePath.ToString().Equals(PathText, ESearchCase::CaseSensitive))
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				FieldPath,
				TEXT("SoftObjectPath는 valid long package의 canonical top-level asset object path여야 하며 subobject를 허용하지 않습니다."));
			return false;
		}

		OutObjectPath = CandidatePath;
		return true;
	}

	// SoftObjectPath referenced asset의 존재와 base-class compatibility를 Asset Registry metadata만으로 검사합니다.
	bool ValidateAssetReferenceMetadata(
		const FSoftObjectPath& ObjectPath,
		const FTopLevelAssetPath& ExpectedBaseClassPath,
		const FString& FieldPath,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		if (ObjectPath.IsNull())
		{
			return true;
		}
		if (ExpectedBaseClassPath.IsNull())
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, FieldPath, TEXT("reference validation expected class path가 유효하지 않습니다."));
			return false;
		}

		// 현재 Editor의 read-only Asset Registry입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// exact object path의 persisted metadata입니다. bIncludeOnlyOnDiskAssets=false여도 object load는 수행하지 않습니다.
		const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(ObjectPath, false, false);
		if (!AssetData.IsValid())
		{
			AddIssue(OutIssues, ECFDAStagingIssueCode::InvalidValue, FieldPath, TEXT("SoftObjectPath가 가리키는 asset metadata를 Asset Registry에서 찾을 수 없습니다."));
			return false;
		}

		// expected base class 하나를 시작점으로 사용할 hierarchy query 입력입니다.
		TArray<FTopLevelAssetPath> BaseClassPaths;
		BaseClassPaths.Add(ExpectedBaseClassPath);
		// 제외할 derived class가 없는 empty set입니다.
		const TSet<FTopLevelAssetPath> ExcludedClassPaths;
		// Asset Registry가 계산한 expected base class의 derived class set입니다.
		TSet<FTopLevelAssetPath> DerivedClassPaths;
		AssetRegistry.GetDerivedClassNames(BaseClassPaths, ExcludedClassPaths, DerivedClassPaths);

		// exact base class 또는 그 derived class인지 나타냅니다.
		const bool bCompatibleClass = AssetData.AssetClassPath == ExpectedBaseClassPath
			|| DerivedClassPaths.Contains(AssetData.AssetClassPath);
		if (!bCompatibleClass)
		{
			AddIssue(
				OutIssues,
				ECFDAStagingIssueCode::InvalidValue,
				FieldPath,
				FString::Printf(
					TEXT("referenced asset class가 expected base class와 호환되지 않습니다. Actual=%s ExpectedBase=%s"),
					*AssetData.AssetClassPath.ToString(),
					*ExpectedBaseClassPath.ToString()));
			return false;
		}
		return true;
	}

#if WITH_DEV_AUTOMATION_TESTS
	// 현재 thread에 기존 probe가 없을 때 requested sink를 설치합니다.
	FScopedSemanticTokenProbe::FScopedSemanticTokenProbe(TArray<FString>& RequestedSink)
		: PreviousSink(CFDACommonPrimitivesPrivate::GSemanticTokenLabelProbe)
		, bBound(PreviousSink == nullptr)
	{
		if (bBound)
		{
			CFDACommonPrimitivesPrivate::GSemanticTokenLabelProbe = &RequestedSink;
		}
	}

	// Scope 종료 시 이전 sink를 복원합니다.
	FScopedSemanticTokenProbe::~FScopedSemanticTokenProbe()
	{
		if (bBound)
		{
			CFDACommonPrimitivesPrivate::GSemanticTokenLabelProbe = PreviousSink;
		}
	}

	// 이번 scope가 probe sink ownership을 획득했는지 반환합니다.
	bool FScopedSemanticTokenProbe::IsBound() const
	{
		return bBound;
	}
#endif
}
