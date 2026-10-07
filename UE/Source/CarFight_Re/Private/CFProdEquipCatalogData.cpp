// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdEquipCatalogData.cpp
// Version: v1.1.0
// Date: 2026-10-06
// Description: CF-FQ-058 CCAS Production Equipment Publication Catalog runtime validation 구현입니다.
// Changelog:
// - v1.1.0: AssetManager stable PrimaryAsset identity 기반 Production Catalog load + published EquipmentPreset exact lookup seam을 추가.
// - v1.0.0: stable PrimaryAsset identity, generated membership 검증과 canonical ContentId 조회를 최초 구현.
// Migration:
// - ProductGraphFingerprint/CatalogFingerprint 생성 권한은 Editor CCAS bridge가 소유합니다. Runtime은 저장된 canonical SHA-256 표기만 검증합니다.

#include "CFProdEquipCatalogData.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFWeaponData.h"
#include "Engine/AssetManager.h"

namespace
{
	// CCAS generated fingerprint가 canonical lowercase sha256 표기인지 문법만 검증합니다.
	bool IsCanonicalSha256Text(const FString& Fingerprint)
	{
		// Canonical fingerprint 접두사입니다.
		static const FString Prefix = TEXT("sha256:");
		if (!Fingerprint.StartsWith(Prefix, ESearchCase::CaseSensitive)
			|| Fingerprint.Len() != Prefix.Len() + 64)
		{
			return false;
		}

		// SHA-256 hex payload 시작 index입니다.
		const int32 HexStartIndex = Prefix.Len();
		for (int32 CharacterIndex = HexStartIndex; CharacterIndex < Fingerprint.Len(); ++CharacterIndex)
		{
			// 현재 fingerprint hex 문자입니다.
			const TCHAR Character = Fingerprint[CharacterIndex];
			const bool bIsDigit = Character >= TEXT('0') && Character <= TEXT('9');
			const bool bIsLowerHexLetter = Character >= TEXT('a') && Character <= TEXT('f');
			if (!bIsDigit && !bIsLowerHexLetter)
			{
				return false;
			}
		}

		return true;
	}

	// Catalog validation 오류를 일관된 FText로 추가합니다.
	void AddCatalogValidationError(TArray<FText>& OutValidationErrors, const FString& Message)
	{
		OutValidationErrors.Add(FText::FromString(Message));
	}
}

// Stable Production publication PrimaryAsset identity를 반환합니다.
FPrimaryAssetId UCFProdEquipCatalogData::GetPrimaryAssetId() const
{
	// CCAS Production Publication Catalog의 stable PrimaryAssetType입니다.
	static const FPrimaryAssetType CatalogAssetType(TEXT("CFProdEquipCatalog"));

	// CCAS Production Publication Catalog의 stable PrimaryAssetName입니다.
	static const FName CatalogAssetName(TEXT("Production"));

	return FPrimaryAssetId(CatalogAssetType, CatalogAssetName);
}

// Stable Production PrimaryAsset identity로 generated Catalog를 load합니다.
UCFProdEquipCatalogData* UCFProdEquipCatalogData::LoadProductionCatalog()
{
	static const FPrimaryAssetId ProductionCatalogId(
		FPrimaryAssetType(TEXT("CFProdEquipCatalog")),
		FName(TEXT("Production")));

	UAssetManager& AssetManager = UAssetManager::Get();
	if (UCFProdEquipCatalogData* LoadedCatalog = Cast<UCFProdEquipCatalogData>(
		AssetManager.GetPrimaryAssetObject(ProductionCatalogId)))
	{
		return LoadedCatalog;
	}

	const FSoftObjectPath CatalogPath =
		AssetManager.GetPrimaryAssetPath(ProductionCatalogId);
	if (CatalogPath.IsValid())
	{
		if (UCFProdEquipCatalogData* LoadedCatalog = Cast<UCFProdEquipCatalogData>(
			CatalogPath.TryLoad()))
		{
			return LoadedCatalog;
		}
	}

	// PrimaryAsset registry가 Editor lifetime 중 아직 refresh되지 않은 경우에도 frozen canonical package를 직접 resolve합니다.
	return LoadObject<UCFProdEquipCatalogData>(
		nullptr,
		TEXT("/Game/CarFight/Weapons/Data/Production/DA_ProdEquipCatalog.DA_ProdEquipCatalog"));
}

// Generated Catalog 전체 membership과 finite-ammo projection 계약을 검증합니다.
bool UCFProdEquipCatalogData::ValidateProductionCatalog(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (CatalogSchemaVersion != 1)
	{
		AddCatalogValidationError(
			OutValidationErrors,
			FString::Printf(
				TEXT("Production CatalogSchemaVersion이 지원값 1이 아닙니다: %d"),
				CatalogSchemaVersion));
	}

	if (!IsCanonicalSha256Text(CatalogFingerprint))
	{
		AddCatalogValidationError(
			OutValidationErrors,
			TEXT("Production CatalogFingerprint가 canonical lowercase sha256 형식이 아닙니다."));
	}

	// Catalog 안에서 중복 ContentId를 차단할 집합입니다.
	TSet<FString> SeenContentIds;

	// Deterministic published membership 순서를 검증하기 위한 직전 ContentId입니다.
	FString PreviousContentId;

	for (int32 EntryIndex = 0; EntryIndex < PublishedEquipment.Num(); ++EntryIndex)
	{
		// 현재 검증 중인 published Product entry입니다.
		const FCFProdEquipCatalogEntry& Entry = PublishedEquipment[EntryIndex];

		if (Entry.ContentId.IsEmpty())
		{
			AddCatalogValidationError(
				OutValidationErrors,
				FString::Printf(TEXT("PublishedEquipment[%d] ContentId가 비어 있습니다."), EntryIndex));
			continue;
		}

		if (SeenContentIds.Contains(Entry.ContentId))
		{
			AddCatalogValidationError(
				OutValidationErrors,
				FString::Printf(TEXT("중복 Production ContentId가 존재합니다: %s"), *Entry.ContentId));
		}
		SeenContentIds.Add(Entry.ContentId);

		if (!PreviousContentId.IsEmpty()
			&& Entry.ContentId.Compare(PreviousContentId, ESearchCase::CaseSensitive) <= 0)
		{
			AddCatalogValidationError(
				OutValidationErrors,
				TEXT("PublishedEquipment는 ContentId 오름차순 exact deterministic order여야 합니다."));
		}
		PreviousContentId = Entry.ContentId;

		if (!IsValid(Entry.EquipmentPresetData))
		{
			AddCatalogValidationError(
				OutValidationErrors,
				FString::Printf(
					TEXT("Product %s의 EquipmentPresetData가 유효하지 않습니다."),
					*Entry.ContentId));
			continue;
		}

		if (!Entry.EquipmentPresetData->HasCompleteEquipmentData())
		{
			AddCatalogValidationError(
				OutValidationErrors,
				FString::Printf(
					TEXT("Product %s의 EquipmentPresetData payload가 완성되지 않았습니다."),
					*Entry.ContentId));
		}

		if (!IsCanonicalSha256Text(Entry.ProductGraphFingerprint))
		{
			AddCatalogValidationError(
				OutValidationErrors,
				FString::Printf(
					TEXT("Product %s의 ProductGraphFingerprint가 canonical lowercase sha256 형식이 아닙니다."),
					*Entry.ContentId));
		}

		// 한 Product 안에서 같은 AmmoId 출격 load 중복을 차단할 집합입니다.
		TSet<FName> SeenAmmoIds;
		for (int32 AmmoLoadIndex = 0; AmmoLoadIndex < Entry.DefaultSortieAmmoLoads.Num(); ++AmmoLoadIndex)
		{
			// 현재 검증 중인 explicit sortie ammo projection입니다.
			const FCFAmmoSortieLoad& AmmoLoad = Entry.DefaultSortieAmmoLoads[AmmoLoadIndex];

			if (!IsValid(AmmoLoad.AmmoData) || !AmmoLoad.AmmoData->IsAmmoDataValid())
			{
				AddCatalogValidationError(
					OutValidationErrors,
					FString::Printf(
						TEXT("Product %s의 DefaultSortieAmmoLoads[%d] AmmoData가 유효하지 않습니다."),
						*Entry.ContentId,
						AmmoLoadIndex));
				continue;
			}

			if (SeenAmmoIds.Contains(AmmoLoad.AmmoData->AmmoId))
			{
				AddCatalogValidationError(
					OutValidationErrors,
					FString::Printf(
						TEXT("Product %s에 중복 AmmoId 출격 load가 존재합니다: %s"),
						*Entry.ContentId,
						*AmmoLoad.AmmoData->AmmoId.ToString()));
			}
			SeenAmmoIds.Add(AmmoLoad.AmmoData->AmmoId);

			if (AmmoLoad.InitialSortieAmmoCount < 0
				|| AmmoLoad.InitialSortieAmmoCount > AmmoLoad.AmmoData->GetEffectiveMaximumLoadableAmmoCount())
			{
				AddCatalogValidationError(
					OutValidationErrors,
					FString::Printf(
						TEXT("Product %s의 AmmoId %s 출격 수량 %d가 허용 범위 0..%d를 벗어났습니다."),
						*Entry.ContentId,
						*AmmoLoad.AmmoData->AmmoId.ToString(),
						AmmoLoad.InitialSortieAmmoCount,
						AmmoLoad.AmmoData->GetEffectiveMaximumLoadableAmmoCount()));
			}
		}

		// 현재 EquipmentPreset이 보유한 weapon payload입니다.
		const UCFWeaponData* WeaponData = Entry.EquipmentPresetData->DefaultWeaponData.Get();
		if (IsValid(WeaponData) && WeaponData->UsesFiniteAmmoRuntime())
		{
			// Finite weapon이 요구하는 exact AmmoData입니다.
			const UCFAmmoData* RequiredAmmoData = WeaponData->DefaultAmmoData.Get();
			if (!IsValid(RequiredAmmoData) || !RequiredAmmoData->IsAmmoDataValid())
			{
				AddCatalogValidationError(
					OutValidationErrors,
					FString::Printf(
						TEXT("Product %s의 finite WeaponData가 유효한 DefaultAmmoData를 갖지 않습니다."),
						*Entry.ContentId));
				continue;
			}

			// DefaultSortieAmmoLoads 안에서 required AmmoId를 exact1 찾은 결과입니다.
			const FCFAmmoSortieLoad* RequiredAmmoLoad = Entry.DefaultSortieAmmoLoads.FindByPredicate(
				[RequiredAmmoData](const FCFAmmoSortieLoad& CandidateLoad)
				{
					return CandidateLoad.AmmoData.Get() == RequiredAmmoData;
				});

			if (RequiredAmmoLoad == nullptr)
			{
				AddCatalogValidationError(
					OutValidationErrors,
					FString::Printf(
						TEXT("Product %s의 finite WeaponData용 explicit DefaultSortieAmmoLoad가 없습니다: AmmoId=%s"),
						*Entry.ContentId,
						*RequiredAmmoData->AmmoId.ToString()));
			}
			else if (RequiredAmmoLoad->InitialSortieAmmoCount < WeaponData->GetEffectiveInitialLoadedAmmoCount())
			{
				AddCatalogValidationError(
					OutValidationErrors,
					FString::Printf(
						TEXT("Product %s의 DefaultSortieAmmoCount %d가 Weapon 초기 장전량 %d보다 작습니다."),
						*Entry.ContentId,
						RequiredAmmoLoad->InitialSortieAmmoCount,
						WeaponData->GetEffectiveInitialLoadedAmmoCount()));
			}
		}
	}

	return OutValidationErrors.IsEmpty();
}

// Canonical ContentId로 published entry를 exact1 조회합니다.
const FCFProdEquipCatalogEntry* UCFProdEquipCatalogData::FindEntryByContentId(const FString& ContentId) const
{
	if (ContentId.IsEmpty())
	{
		return nullptr;
	}

	return PublishedEquipment.FindByPredicate(
		[&ContentId](const FCFProdEquipCatalogEntry& Entry)
		{
			return Entry.ContentId == ContentId;
		});
}

// Exact persisted EquipmentPresetData identity로 published entry를 exact1 조회합니다.
const FCFProdEquipCatalogEntry* UCFProdEquipCatalogData::FindEntryByEquipmentPreset(
	const UCFEquipmentPresetData* EquipmentPresetData) const
{
	if (!IsValid(EquipmentPresetData))
	{
		return nullptr;
	}

	return PublishedEquipment.FindByPredicate(
		[EquipmentPresetData](const FCFProdEquipCatalogEntry& Entry)
		{
			return Entry.EquipmentPresetData.Get() == EquipmentPresetData;
		});
}
