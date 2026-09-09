// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractBase.cpp
// Version: v1.0.0
// Date: 2026-09-09
// Description: CF-FQ-050 DACE-P0-01 production-owned append-only accepted contract snapshot history authority입니다.
// Changelog:
// - v1.0.0: MissileGuidePreset SchemaRevision 1 / AdapterContractRevision 2 bootstrap accepted snapshot exact 1 record와 chain signature 계산을 최초 추가.
// Migration:
// - 새 accepted contract는 기존 record를 수정하지 않고 이 history 뒤에 append해야 합니다. Test fixture가 accepted baseline authority를 소유하지 않습니다.

#include "CFDAContractGuard.h"

#include "Containers/StringConv.h"
#include "Misc/SecureHash.h"

#define UI CF_OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
THIRD_PARTY_INCLUDES_END
#undef UI

namespace CFDAContractBasePrivate
{
	// Migration impact enum을 stable snapshot token으로 변환합니다.
	const TCHAR* MigrationImpactToken(const ECFDAContractMigrationImpact Impact)
	{
		switch (Impact)
		{
		case ECFDAContractMigrationImpact::NoMigration: return TEXT("NoMigration");
		case ECFDAContractMigrationImpact::StagingMigrationRequired: return TEXT("StagingMigrationRequired");
		case ECFDAContractMigrationImpact::ProductMigrationReviewRequired: return TEXT("ProductMigrationReviewRequired");
		case ECFDAContractMigrationImpact::StagingAndProductMigrationReviewRequired: return TEXT("StagingAndProductMigrationReviewRequired");
		default: return TEXT("Unknown");
		}
	}

	// Migration resolution enum을 stable snapshot token으로 변환합니다.
	const TCHAR* MigrationResolutionToken(const ECFDAContractMigrationResolution Resolution)
	{
		switch (Resolution)
		{
		case ECFDAContractMigrationResolution::NotRequired: return TEXT("NotRequired");
		case ECFDAContractMigrationResolution::Pending: return TEXT("Pending");
		case ECFDAContractMigrationResolution::Resolved: return TEXT("Resolved");
		default: return TEXT("Unknown");
		}
	}

	// UTF-8 canonical snapshot text를 SHA-256 protocol fingerprint로 변환합니다.
	bool HashSnapshotText(const FString& CanonicalText, FString& OutSignature, FString& OutError)
	{
		// Canonical UTF-8 bytes입니다.
		FTCHARToUTF8 CanonicalUtf8(*CanonicalText);
		// OpenSSL EVP가 채울 SHA-256 결과입니다.
		FSHA256Signature Signature;
		// OpenSSL EVP가 반환하는 digest byte 수입니다.
		unsigned int DigestLength = 0;
		// Empty text에서도 non-null input pointer를 유지하는 sentinel입니다.
		const uint8 EmptyInputByte = 0;
		// 실제 hash input pointer입니다.
		const uint8* HashData = CanonicalUtf8.Length() > 0 ? reinterpret_cast<const uint8*>(CanonicalUtf8.Get()) : &EmptyInputByte;
		// Portable SHA-256 실행 결과입니다.
		const int32 DigestResult = EVP_Digest(HashData, static_cast<size_t>(CanonicalUtf8.Length()), Signature.Signature, &DigestLength, EVP_sha256(), nullptr);
		if (DigestResult != 1 || DigestLength != UE_ARRAY_COUNT(Signature.Signature))
		{
			OutSignature.Reset();
			OutError = TEXT("DACE accepted snapshot SHA-256 signature 생성에 실패했습니다.");
			return false;
		}
		OutSignature = TEXT("sha256:") + Signature.ToString().ToLower();
		OutError.Reset();
		return true;
	}
}

// Production-owned accepted snapshot append-only history를 반환합니다.
const TArray<FCFDAAcceptedContractSnapshot>& FCFDAContractGuard::GetAcceptedSnapshots()
{
	// CF-FQ-049 Current System을 bootstrap으로 동결한 accepted snapshot exact 1 record입니다.
	static const TArray<FCFDAAcceptedContractSnapshot> Snapshots =
	{
		{
			TEXT("DACE-MissileGuidePreset-S1-A2-Bootstrap"),
			TEXT(""),
			TEXT("CarFight.DataAsset.MissileGuidePreset"),
			1,
			2,
			TEXT("/Script/CarFight_Re.CFMissileGuidePresetData"),
			TEXT("sha256:9a01449e0dfe2cf512527eccffa715bf6f62bcd3993e579f00d4cd87c9b0f8ac"),
			TEXT("sha256:87d3997689a44bdfe2ce7c0442d52aa2b14181b4f83f80a5550861ba72869ab9"),
			TEXT("sha256:b9bdaa43e693004b0cadae700c548b4c5552dd7e5b6cbf8e4c0728516ce70f87"),
			TEXT("sha256:8ea27c8e053d89ad6781cd8815a53f5bb6b9a2231b743ce88b2118f9cf25153a"),
			ECFDAContractMigrationImpact::NoMigration,
			ECFDAContractMigrationResolution::NotRequired,
			TEXT(""),
			TEXT("sha256:ec198d5eb5f8199b1ec5a99bbbde80ec5ae30916fee6fea89e72875c710aee69")
		}
	};
	return Snapshots;
}

// Snapshot record의 chain signature를 deterministic하게 계산합니다.
bool FCFDAContractGuard::BuildSnapshotSignature(const FCFDAAcceptedContractSnapshot& Snapshot, FString& OutSignature, FString& OutError)
{
	// Snapshot fields를 fixed-key order로 표현한 canonical text입니다.
	const FString CanonicalText = FString::Printf(
		TEXT("SnapshotId\t%s\n")
		TEXT("PreviousSnapshotSignature\t%s\n")
		TEXT("SchemaId\t%s\n")
		TEXT("SchemaRevision\t%d\n")
		TEXT("AdapterContractRevision\t%d\n")
		TEXT("DataAssetTypeClassPath\t%s\n")
		TEXT("SourceShapeSignature\t%s\n")
		TEXT("AdapterShapeSignature\t%s\n")
		TEXT("SourceAdapterMappingSignature\t%s\n")
		TEXT("SemanticContractSignature\t%s\n")
		TEXT("MigrationImpact\t%s\n")
		TEXT("MigrationResolution\t%s\n")
		TEXT("MigrationEvidenceId\t%s"),
		*Snapshot.SnapshotId,
		*Snapshot.PreviousSnapshotSignature,
		*Snapshot.SchemaId,
		Snapshot.SchemaRevision,
		Snapshot.AdapterContractRevision,
		*Snapshot.DataAssetTypeClassPath,
		*Snapshot.SourceShapeSignature,
		*Snapshot.AdapterShapeSignature,
		*Snapshot.SourceAdapterMappingSignature,
		*Snapshot.SemanticContractSignature,
		CFDAContractBasePrivate::MigrationImpactToken(Snapshot.MigrationImpact),
		CFDAContractBasePrivate::MigrationResolutionToken(Snapshot.MigrationResolution),
		*Snapshot.MigrationEvidenceId);
	return CFDAContractBasePrivate::HashSnapshotText(CanonicalText, OutSignature, OutError);
}
