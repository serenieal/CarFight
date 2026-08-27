// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRefEvidence.cpp
// Version: v1.2.0
// Date: 2026-08-26
// Description: CF-FQ-040 Vehicle Reference Evidence Unicode NFC canonical semantic payload와 portable SHA-256 fingerprint 구현입니다.
// Changelog:
// - v1.2.0: CFVRN-1 frozen contract의 Unicode NFC를 UE 공급 ICU Normalizer2로 구현하고 stable-ID 정렬과 모든 length-prefixed semantic token에 적용.
// - v1.1.1: Unreal CoreUObject namespace UI와 OpenSSL 전역 UI typedef 충돌을 third-party include 구간의 local macro rename으로 격리.
// - v1.1.0: Windows UE 5.8에서 GenericPlatform SHA-256이 assert-only인 경로를 제거하고 Editor-only OpenSSL EVP SHA-256으로 교체.
// - v1.0.1: UE 5.8에 존재하지 않는 Misc/LexToString.h include 의존을 제거하고 locale-independent printf canonical numeric formatting으로 교체.
// - v1.0.0: VB-P0-01 CFVREF-1 / CFVRN-1 계약에 맞춘 stable-ID sorting, diagnostic exclusion, URL tracking/fragment 제거와 SHA-256 생성 구현.
// Migration:
// - AccessedAtUtc/ResearchNotes/SourceScopeNote/Note/AuthoringRevision/LastResearchAtUtc와 array physical order는 fingerprint를 바꾸지 않습니다.

#include "DataAuthoring/CFVehicleRefEvidence.h"

#include "Containers/StringConv.h"
#include "GenericPlatform/GenericPlatformMisc.h"

// Unreal의 namespace UI와 OpenSSL 1.1.1 전역 UI typedef가 충돌하지 않도록 third-party header 내부에서만 이름을 격리합니다.
#define UI CF_OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
#include <unicode/unorm2.h>
THIRD_PARTY_INCLUDES_END
#undef UI

namespace CFVehicleRefEvidencePrivate
{
	// Evidence fingerprint domain separator입니다.
	static constexpr ANSICHAR EvidenceDomain[] = "CFVehicleRefEvidence";

	// Evidence fingerprint contract revision입니다.
	static constexpr TCHAR EvidenceContractRevision[] = TEXT("CFVREF-1");

	// FString을 CFVRN-1 계약의 Unicode NFC canonical form으로 정규화합니다.
	FString NormalizeNfc(const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return Value;
		}

		// Unreal TCHAR source를 ICU가 소비하는 UTF-16 buffer로 변환합니다.
		FTCHARToUTF16 Utf16Value(*Value);
		// ICU NFC singleton 조회 결과를 받을 error code입니다.
		UErrorCode ErrorCode = U_ZERO_ERROR;
		// Unicode canonical composition(NFC)을 수행하는 immutable ICU normalizer입니다.
		const UNormalizer2* NfcNormalizer = unorm2_getNFCInstance(&ErrorCode);
		if (U_FAILURE(ErrorCode) || !NfcNormalizer)
		{
			ensureMsgf(false, TEXT("CFVRN-1 Unicode NFC normalizer를 초기화하지 못했습니다. ICU error=%d"), static_cast<int32>(ErrorCode));
			return Value;
		}

		// Output buffer에 필요한 UTF-16 code unit 개수입니다.
		ErrorCode = U_ZERO_ERROR;
		const int32 RequiredLength = unorm2_normalize(
			NfcNormalizer,
			reinterpret_cast<const UChar*>(Utf16Value.Get()),
			Utf16Value.Length(),
			nullptr,
			0,
			&ErrorCode);
		if (ErrorCode != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(ErrorCode))
		{
			ensureMsgf(false, TEXT("CFVRN-1 Unicode NFC output 길이를 계산하지 못했습니다. ICU error=%d"), static_cast<int32>(ErrorCode));
			return Value;
		}
		if (RequiredLength <= 0)
		{
			return FString();
		}

		// ICU가 NFC UTF-16 결과를 쓸 exact-size buffer입니다.
		TArray<UChar> NormalizedBuffer;
		NormalizedBuffer.SetNumUninitialized(RequiredLength);
		ErrorCode = U_ZERO_ERROR;
		// 실제 NFC UTF-16 code unit 개수입니다.
		const int32 WrittenLength = unorm2_normalize(
			NfcNormalizer,
			reinterpret_cast<const UChar*>(Utf16Value.Get()),
			Utf16Value.Length(),
			NormalizedBuffer.GetData(),
			NormalizedBuffer.Num(),
			&ErrorCode);
		if (U_FAILURE(ErrorCode) || WrittenLength != RequiredLength)
		{
			ensureMsgf(false, TEXT("CFVRN-1 Unicode NFC 정규화에 실패했습니다. ICU error=%d"), static_cast<int32>(ErrorCode));
			return Value;
		}

		// ICU UTF-16 결과를 Unreal TCHAR 문자열로 변환합니다.
		FUTF16ToTCHAR NormalizedText(reinterpret_cast<const UTF16CHAR*>(NormalizedBuffer.GetData()), WrittenLength);
		return FString(NormalizedText.Length(), NormalizedText.Get());
	}

	// Canonical payload token 하나를 Unicode NFC + length-prefixed format으로 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		// 길이 계산과 실제 token bytes가 같은 canonical text를 사용하도록 NFC를 먼저 적용합니다.
		const FString CanonicalValue = NormalizeNfc(Value);
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(CanonicalValue.Len());
		OutPayload += TEXT(":");
		OutPayload += CanonicalValue;
		OutPayload += TEXT("\n");
	}

	// FName을 deterministic string으로 변환합니다.
	FString NameString(const FName Value)
	{
		return Value.IsNone() ? FString() : Value.ToString();
	}

	// bool을 locale-independent canonical 값으로 변환합니다.
	FString BoolString(const bool Value)
	{
		return Value ? TEXT("1") : TEXT("0");
	}

	// float를 locale-independent round-trip string으로 변환합니다.
	FString FloatString(const float Value)
	{
		return FString::Printf(TEXT("%.9g"), static_cast<double>(Value));
	}

	// double을 locale-independent round-trip string으로 변환합니다.
	FString DoubleString(const double Value)
	{
		return FString::Printf(TEXT("%.17g"), Value);
	}

	// FName 목록을 stable ID 기준으로 정렬한 canonical 문자열로 반환합니다.
	FString SortedNamesString(const TArray<FName>& Values)
	{
		// 물리 array order를 fingerprint에서 제거할 복사본입니다.
		TArray<FName> SortedValues = Values;
		SortedValues.Sort([](const FName Left, const FName Right)
		{
			return NormalizeNfc(NameString(Left)) < NormalizeNfc(NameString(Right));
		});

		// 구분자 충돌을 막기 위해 각 ID를 length-prefixed token으로 누적합니다.
		FString Result;
		for (const FName Value : SortedValues)
		{
			AppendToken(Result, TEXT("Id"), NameString(Value));
		}
		return Result;
	}

	// Method parameter map을 key 기준으로 정렬해 canonical 문자열로 반환합니다.
	FString SortedParametersString(const TMap<FName, FString>& Parameters)
	{
		// deterministic key order를 만들 key 배열입니다.
		TArray<FName> SortedKeys;
		Parameters.GetKeys(SortedKeys);
		SortedKeys.Sort([](const FName Left, const FName Right)
		{
			return NormalizeNfc(NameString(Left)) < NormalizeNfc(NameString(Right));
		});

		// parameter key/value를 length-prefixed token으로 누적합니다.
		FString Result;
		for (const FName Key : SortedKeys)
		{
			AppendToken(Result, TEXT("ParamKey"), NameString(Key));
			AppendToken(Result, TEXT("ParamValue"), Parameters.FindRef(Key));
		}
		return Result;
	}

	// URL fragment와 tracking query를 제거하고 scheme/host를 소문자로 정규화합니다.
	FString NormalizeUrlForFingerprint(const FString& InputUrl)
	{
		// 앞뒤 공백을 제거한 URL입니다.
		FString Url = InputUrl;
		Url.TrimStartAndEndInline();

		// fragment는 research fact identity에 포함하지 않습니다.
		int32 FragmentIndex = INDEX_NONE;
		if (Url.FindChar(TEXT('#'), FragmentIndex))
		{
			Url.LeftInline(FragmentIndex, EAllowShrinking::No);
		}

		// query와 base URL을 분리합니다.
		FString BaseUrl = Url;
		FString QueryString;
		int32 QueryIndex = INDEX_NONE;
		if (Url.FindChar(TEXT('?'), QueryIndex))
		{
			BaseUrl = Url.Left(QueryIndex);
			QueryString = Url.Mid(QueryIndex + 1);
		}

		// scheme과 host까지만 lowercase 처리합니다.
		int32 SchemeIndex = INDEX_NONE;
		if (BaseUrl.Find(TEXT("://"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 0) != INDEX_NONE)
		{
			SchemeIndex = BaseUrl.Find(TEXT("://"));
			// host 끝 위치입니다.
			const int32 HostStart = SchemeIndex + 3;
			// path 시작 위치입니다.
			int32 PathStart = BaseUrl.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromStart, HostStart);
			if (PathStart == INDEX_NONE)
			{
				PathStart = BaseUrl.Len();
			}
			// lowercase 대상 scheme+host입니다.
			FString Authority = BaseUrl.Left(PathStart).ToLower();
			BaseUrl = Authority + BaseUrl.Mid(PathStart);
		}

		// tracking parameter를 제거하고 나머지 query를 key/value 문자열 기준으로 정렬합니다.
		TArray<FString> QueryParts;
		QueryString.ParseIntoArray(QueryParts, TEXT("&"), true);
		QueryParts.RemoveAll([](const FString& Part)
		{
			// parameter key입니다.
			FString Key = Part;
			// key/value separator 위치입니다.
			int32 EqualsIndex = INDEX_NONE;
			if (Key.FindChar(TEXT('='), EqualsIndex))
			{
				Key.LeftInline(EqualsIndex, EAllowShrinking::No);
			}
			Key = Key.ToLower();
			return Key.StartsWith(TEXT("utm_"))
				|| Key == TEXT("fbclid")
				|| Key == TEXT("gclid")
				|| Key == TEXT("dclid")
				|| Key == TEXT("msclkid");
		});
		QueryParts.Sort();

		if (QueryParts.IsEmpty())
		{
			return BaseUrl;
		}
		return BaseUrl + TEXT("?") + FString::Join(QueryParts, TEXT("&"));
	}

	// Reference Vehicle identity를 semantic payload에 추가합니다.
	void AppendReferenceVehicle(FString& OutPayload, const FCFRefVehicleIdentity& Vehicle)
	{
		AppendToken(OutPayload, TEXT("ReferenceVehicleId"), NameString(Vehicle.ReferenceVehicleId));
		AppendToken(OutPayload, TEXT("Role"), FString::FromInt(static_cast<int32>(Vehicle.Role)));
		AppendToken(OutPayload, TEXT("Manufacturer"), Vehicle.Manufacturer);
		AppendToken(OutPayload, TEXT("Model"), Vehicle.Model);
		AppendToken(OutPayload, TEXT("Generation"), Vehicle.Generation);
		AppendToken(OutPayload, TEXT("ModelYearStart"), FString::FromInt(Vehicle.ModelYearStart));
		AppendToken(OutPayload, TEXT("ModelYearEnd"), FString::FromInt(Vehicle.ModelYearEnd));
		AppendToken(OutPayload, TEXT("ModelYearQualifier"), FString::FromInt(static_cast<int32>(Vehicle.ModelYearQualifier)));
		AppendToken(OutPayload, TEXT("Trim"), Vehicle.Trim);
		AppendToken(OutPayload, TEXT("Powertrain"), Vehicle.Powertrain);
		AppendToken(OutPayload, TEXT("Transmission"), Vehicle.Transmission);
		AppendToken(OutPayload, TEXT("DriveLayout"), Vehicle.DriveLayout);
		AppendToken(OutPayload, TEXT("MarketRegion"), Vehicle.MarketRegion);
		AppendToken(OutPayload, TEXT("BodyVariant"), Vehicle.BodyVariant);
		AppendToken(OutPayload, TEXT("WheelTireVariant"), Vehicle.WheelTireVariant);
		AppendToken(OutPayload, TEXT("IdentityConfidence"), FloatString(Vehicle.IdentityConfidence));
	}

	// Source citation의 semantic fields만 payload에 추가합니다.
	void AppendSource(FString& OutPayload, const FCFRefSourceCitation& Source)
	{
		AppendToken(OutPayload, TEXT("SourceId"), NameString(Source.SourceId));
		AppendToken(OutPayload, TEXT("Tier"), FString::FromInt(static_cast<int32>(Source.Tier)));
		AppendToken(OutPayload, TEXT("SourceKind"), NameString(Source.SourceKind));
		AppendToken(OutPayload, TEXT("Publisher"), Source.Publisher);
		AppendToken(OutPayload, TEXT("DocumentTitle"), Source.DocumentTitle);
		AppendToken(OutPayload, TEXT("CanonicalUrl"), NormalizeUrlForFingerprint(Source.CanonicalUrl));
		AppendToken(OutPayload, TEXT("PublishedDate"), Source.PublishedDate);
		AppendToken(OutPayload, TEXT("UpdatedDate"), Source.UpdatedDate);
		AppendToken(OutPayload, TEXT("Locator"), Source.Locator);
		AppendToken(OutPayload, TEXT("ReferenceVehicleIds"), SortedNamesString(Source.ReferenceVehicleIds));
		AppendToken(OutPayload, TEXT("OriginGroupId"), NameString(Source.OriginGroupId));
		AppendToken(OutPayload, TEXT("OriginIndependence"), FString::FromInt(static_cast<int32>(Source.OriginIndependence)));
	}

	// Atomic claim의 semantic fields만 payload에 추가합니다.
	void AppendClaim(FString& OutPayload, const FCFRefClaim& Claim)
	{
		AppendToken(OutPayload, TEXT("ClaimId"), NameString(Claim.ClaimId));
		AppendToken(OutPayload, TEXT("ReferenceVehicleId"), NameString(Claim.ReferenceVehicleId));
		AppendToken(OutPayload, TEXT("FactKey"), NameString(Claim.FactKey));
		AppendToken(OutPayload, TEXT("ValueKind"), FString::FromInt(static_cast<int32>(Claim.ValueKind)));
		AppendToken(OutPayload, TEXT("NumberValue"), DoubleString(Claim.NumberValue));
		AppendToken(OutPayload, TEXT("IntegerValue"), FString::Printf(TEXT("%lld"), static_cast<long long>(Claim.IntegerValue)));
		AppendToken(OutPayload, TEXT("BooleanValue"), BoolString(Claim.BooleanValue));
		AppendToken(OutPayload, TEXT("TextValue"), Claim.TextValue);
		AppendToken(OutPayload, TEXT("RangeMinValue"), DoubleString(Claim.RangeMinValue));
		AppendToken(OutPayload, TEXT("RangeMaxValue"), DoubleString(Claim.RangeMaxValue));
		AppendToken(OutPayload, TEXT("UnitId"), NameString(Claim.UnitId));
		AppendToken(OutPayload, TEXT("SourceValueText"), Claim.SourceValueText);
		AppendToken(OutPayload, TEXT("Provenance"), FString::FromInt(static_cast<int32>(Claim.Provenance)));
		AppendToken(OutPayload, TEXT("CitationIds"), SortedNamesString(Claim.CitationIds));
		AppendToken(OutPayload, TEXT("InputClaimIds"), SortedNamesString(Claim.InputClaimIds));
		AppendToken(OutPayload, TEXT("MethodId"), NameString(Claim.MethodId));
		AppendToken(OutPayload, TEXT("MethodRevision"), FString::FromInt(Claim.MethodRevision));
		AppendToken(OutPayload, TEXT("MethodParameters"), SortedParametersString(Claim.MethodParameters));
		AppendToken(OutPayload, TEXT("ConfidenceScore"), FloatString(Claim.ConfidenceScore));
		AppendToken(OutPayload, TEXT("ConflictId"), NameString(Claim.ConflictId));
		AppendToken(OutPayload, TEXT("ResolutionState"), FString::FromInt(static_cast<int32>(Claim.ResolutionState)));
	}

	// Conflict의 semantic fields만 payload에 추가합니다.
	void AppendConflict(FString& OutPayload, const FCFRefConflict& Conflict)
	{
		AppendToken(OutPayload, TEXT("ConflictId"), NameString(Conflict.ConflictId));
		AppendToken(OutPayload, TEXT("ReferenceVehicleId"), NameString(Conflict.ReferenceVehicleId));
		AppendToken(OutPayload, TEXT("FactKey"), NameString(Conflict.FactKey));
		AppendToken(OutPayload, TEXT("CandidateClaimIds"), SortedNamesString(Conflict.CandidateClaimIds));
		AppendToken(OutPayload, TEXT("ConflictType"), FString::FromInt(static_cast<int32>(Conflict.ConflictType)));
		AppendToken(OutPayload, TEXT("Severity"), FString::FromInt(static_cast<int32>(Conflict.Severity)));
		AppendToken(OutPayload, TEXT("ResolutionPolicy"), FString::FromInt(static_cast<int32>(Conflict.ResolutionPolicy)));
		AppendToken(OutPayload, TEXT("ResolutionClaimId"), NameString(Conflict.ResolutionClaimId));
		AppendToken(OutPayload, TEXT("ResolutionReasonCode"), NameString(Conflict.ResolutionReasonCode));
	}

	// Unknown fact의 semantic fields만 payload에 추가합니다.
	void AppendUnknown(FString& OutPayload, const FCFRefUnknownFact& Unknown)
	{
		AppendToken(OutPayload, TEXT("UnknownFactId"), NameString(Unknown.UnknownFactId));
		AppendToken(OutPayload, TEXT("ReferenceVehicleId"), NameString(Unknown.ReferenceVehicleId));
		AppendToken(OutPayload, TEXT("FactKey"), NameString(Unknown.FactKey));
		AppendToken(OutPayload, TEXT("SearchedSourceIds"), SortedNamesString(Unknown.SearchedSourceIds));
		AppendToken(OutPayload, TEXT("Reason"), FString::FromInt(static_cast<int32>(Unknown.Reason)));
		AppendToken(OutPayload, TEXT("BlockingUse"), FString::FromInt(static_cast<int32>(Unknown.BlockingUse)));
	}

	// domain separator + semantic UTF-8 payload를 SHA-256 lowercase hex로 변환합니다.
	bool HashSemanticPayload(const FString& Payload, FString& OutFingerprint, FString& OutError)
	{
		// semantic payload의 UTF-8 bytes입니다.
		FTCHARToUTF8 PayloadUtf8(*Payload);
		// domain separator, NUL, semantic payload를 합칠 byte buffer입니다.
		TArray<uint8> HashInput;
		HashInput.Reserve((sizeof(EvidenceDomain) - 1) + 1 + PayloadUtf8.Length());
		HashInput.Append(reinterpret_cast<const uint8*>(EvidenceDomain), sizeof(EvidenceDomain) - 1);
		HashInput.Add(0);
		HashInput.Append(reinterpret_cast<const uint8*>(PayloadUtf8.Get()), PayloadUtf8.Length());

		// OpenSSL EVP가 채울 32-byte SHA-256 결과입니다.
		FSHA256Signature Signature;
		// OpenSSL EVP가 반환하는 실제 digest byte 수입니다.
		unsigned int DigestLength = 0;
		// UE GenericPlatform의 assert-only 구현을 거치지 않는 portable SHA-256 실행 결과입니다.
		const int32 DigestResult = EVP_Digest(
			HashInput.GetData(),
			static_cast<size_t>(HashInput.Num()),
			Signature.Signature,
			&DigestLength,
			EVP_sha256(),
			nullptr);
		if (DigestResult != 1 || DigestLength != UE_ARRAY_COUNT(Signature.Signature))
		{
			OutFingerprint.Reset();
			OutError = TEXT("OpenSSL EVP SHA-256 Evidence fingerprint 생성에 실패했습니다.");
			return false;
		}

		OutFingerprint = Signature.ToString().ToLower();
		OutError.Reset();
		return true;
	}
}

// 새 Evidence identity와 current schema/policy revision을 초기화합니다.
UCFVehicleRefEvidence::UCFVehicleRefEvidence()
{
	EvidenceId = FGuid::NewGuid();
	SchemaRevision = 1;
	NormalizationPolicyRevision = TEXT("CFVRN-1");
}

// Duplicate가 원본 EvidenceId를 재사용하지 않도록 새 identity를 발급합니다.
void UCFVehicleRefEvidence::PostDuplicate(const bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	if (!bDuplicateForPIE)
	{
		EvidenceId = FGuid::NewGuid();
		// 새 duplicate의 generated fingerprint 계산 오류입니다.
		FString FingerprintError;
		RefreshEvidenceFingerprint(FingerprintError);
	}
}

#if WITH_EDITOR
// Editor에서 semantic field가 바뀌면 generated fingerprint를 즉시 다시 계산합니다.
void UCFVehicleRefEvidence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// 현재 semantic payload fingerprint 갱신 오류입니다.
	FString FingerprintError;
	if (RefreshEvidenceFingerprint(FingerprintError))
	{
		++AuthoringRevision;
	}
}
#endif

// 현재 semantic payload로 deterministic SHA-256 fingerprint를 계산합니다.
bool UCFVehicleRefEvidence::BuildEvidenceFingerprint(FString& OutFingerprint, FString& OutError) const
{
	if (!EvidenceId.IsValid())
	{
		OutFingerprint.Reset();
		OutError = TEXT("EvidenceId가 유효하지 않습니다.");
		return false;
	}
	if (SchemaRevision <= 0 || NormalizationPolicyRevision.IsEmpty())
	{
		OutFingerprint.Reset();
		OutError = TEXT("Evidence schema/normalization revision이 유효하지 않습니다.");
		return false;
	}

	// semantic root payload입니다.
	FString Payload;
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("ContractRevision"), CFVehicleRefEvidencePrivate::EvidenceContractRevision);
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("SchemaRevision"), FString::FromInt(SchemaRevision));
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("NormalizationPolicyRevision"), NormalizationPolicyRevision);
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("EvidenceId"), EvidenceId.ToString(EGuidFormats::DigitsWithHyphensLower));
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("TargetRecipeId"), TargetRecipeId.IsValid() ? TargetRecipeId.ToString(EGuidFormats::DigitsWithHyphensLower) : FString());
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("TargetRecipePath"), TargetRecipePath.ToString());
	CFVehicleRefEvidencePrivate::AppendToken(Payload, TEXT("TargetDefinitionPath"), TargetDefinitionPath.ToString());

	// Reference identity는 stable ID 기준으로 정렬합니다.
	TArray<FCFRefVehicleIdentity> SortedVehicles = ReferenceVehicles;
	SortedVehicles.Sort([](const FCFRefVehicleIdentity& Left, const FCFRefVehicleIdentity& Right)
	{
		return CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Left.ReferenceVehicleId))
			< CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Right.ReferenceVehicleId));
	});
	for (const FCFRefVehicleIdentity& Vehicle : SortedVehicles)
	{
		CFVehicleRefEvidencePrivate::AppendReferenceVehicle(Payload, Vehicle);
	}

	// Source citation은 stable SourceId 기준으로 정렬합니다.
	TArray<FCFRefSourceCitation> SortedSources = Sources;
	SortedSources.Sort([](const FCFRefSourceCitation& Left, const FCFRefSourceCitation& Right)
	{
		return CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Left.SourceId))
			< CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Right.SourceId));
	});
	for (const FCFRefSourceCitation& Source : SortedSources)
	{
		CFVehicleRefEvidencePrivate::AppendSource(Payload, Source);
	}

	// Claim은 stable ClaimId 기준으로 정렬합니다.
	TArray<FCFRefClaim> SortedClaims = Claims;
	SortedClaims.Sort([](const FCFRefClaim& Left, const FCFRefClaim& Right)
	{
		return CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Left.ClaimId))
			< CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Right.ClaimId));
	});
	for (const FCFRefClaim& Claim : SortedClaims)
	{
		CFVehicleRefEvidencePrivate::AppendClaim(Payload, Claim);
	}

	// Conflict는 stable ConflictId 기준으로 정렬합니다.
	TArray<FCFRefConflict> SortedConflicts = Conflicts;
	SortedConflicts.Sort([](const FCFRefConflict& Left, const FCFRefConflict& Right)
	{
		return CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Left.ConflictId))
			< CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Right.ConflictId));
	});
	for (const FCFRefConflict& Conflict : SortedConflicts)
	{
		CFVehicleRefEvidencePrivate::AppendConflict(Payload, Conflict);
	}

	// Unknown fact는 stable UnknownFactId 기준으로 정렬합니다.
	TArray<FCFRefUnknownFact> SortedUnknownFacts = UnknownFacts;
	SortedUnknownFacts.Sort([](const FCFRefUnknownFact& Left, const FCFRefUnknownFact& Right)
	{
		return CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Left.UnknownFactId))
			< CFVehicleRefEvidencePrivate::NormalizeNfc(CFVehicleRefEvidencePrivate::NameString(Right.UnknownFactId));
	});
	for (const FCFRefUnknownFact& Unknown : SortedUnknownFacts)
	{
		CFVehicleRefEvidencePrivate::AppendUnknown(Payload, Unknown);
	}

	return CFVehicleRefEvidencePrivate::HashSemanticPayload(Payload, OutFingerprint, OutError);
}

// generated read-only EvidenceFingerprint를 현재 semantic payload로 갱신합니다.
bool UCFVehicleRefEvidence::RefreshEvidenceFingerprint(FString& OutError)
{
	// 현재 semantic payload에서 새로 계산한 fingerprint입니다.
	FString CurrentFingerprint;
	if (!BuildEvidenceFingerprint(CurrentFingerprint, OutError))
	{
		return false;
	}
	EvidenceFingerprint = MoveTemp(CurrentFingerprint);
	return true;
}
