// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdProvisioning.cpp
// Version: v1.2.0
// Date: 2026-10-06
// Description: CF-FQ-058 Typed Production Provisioning, Product Recovery,
// EquipmentPresetAmmoLoads projection, impact withdrawal와 Production Publication Bridge 구현입니다.
// Changelog:
// - v1.2.0: transaction schema v3로 approved Review/Execution identity와 require-absent target evidence를 durable 결속하고 unexpected-existing CreateNew collision을 pre-mutation fail-closed.
// - v1.1.0: F.1 shared single-writer binding validation과 F.2 multi-Product impacted closure revalidation orchestration을 추가.
// - v1.0.2: Product transaction schema v2에서 target object path와 impacted Product composite ContentKey를 durable binding하고 withdrawal intent를 mutation 전에 journal에 고정.
// - v1.0.1: impact closure publication 판정을 Product ContentTypeId까지 결속해 다른 타입의 동일 ContentId가 과잉 withdraw되는 경로를 차단.
// - v1.0.0: frozen Production bridge contract를 existing CFDADurableCore/FCFRuntimeEquipApplyService 위에 최초 구현.
// Migration:
// - 신규 generic DA writer, Runtime Apply subsystem, graph authority를 만들지 않습니다.
// - Product target adapter는 existing typed authoring backend를 호출하는 얇은 boundary만 구현해야 합니다.

#include "DataAuthoring/CFProdProvisioning.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFRuntimeEquipApply.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFDAStagingApply.h"

#include "CFDACommonPrimitives.h"
#include "CFDADurableCore.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"

namespace CFProdProvisioningPrivate
{
	// Canonical generated Production Publication Catalog exact object path입니다.
	const TCHAR* CanonicalCatalogObjectPath =
		TEXT("/Game/CarFight/Weapons/Data/Production/DA_ProdEquipCatalog.DA_ProdEquipCatalog");

	// Product transaction file schema identity입니다.
	const TCHAR* TransactionSchemaId = TEXT("ccas-production-transaction/v3");

	// Catalog payload durable writer가 사용하는 provider-local whole-record payload입니다.
	struct FCatalogPayload
	{
		// Persisted generated Catalog schema revision입니다.
		int32 CatalogSchemaVersion = 1;

		// Stable ContentId order의 published Equipment Product entries입니다.
		TArray<FCFProdEquipCatalogEntry> PublishedEquipment;

		// Published membership canonical fingerprint입니다.
		FString CatalogFingerprint;
	};

	// FString integer token을 shared SHA-256 canonical protocol에 추가합니다.
	void AppendIntegerToken(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const int64 Value)
	{
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			Label,
			LexToString(Value));
	}

	// ContentKey를 shared SHA-256 canonical protocol에 추가합니다.
	void AppendContentKey(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const FCFContentKey& Key)
	{
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			*FString::Printf(TEXT("%s.Type"), Label),
			Key.ContentTypeId.Value);
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			*FString::Printf(TEXT("%s.Id"), Label),
			Key.ContentId);
	}

	// Published entry의 runtime-relevant projection을 canonical bytes에 추가합니다.
	void AppendCatalogEntry(
		TArray<uint8>& OutBytes,
		const FCFProdEquipCatalogEntry& Entry)
	{
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			TEXT("ContentId"),
			Entry.ContentId);
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			TEXT("EquipmentPresetData"),
			GetPathNameSafe(Entry.EquipmentPresetData));
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			TEXT("ProductGraphFingerprint"),
			Entry.ProductGraphFingerprint);

		// Deterministic AmmoId/path order로 fingerprint할 출격 탄약 포인터 목록입니다.
		TArray<const FCFAmmoSortieLoad*> SortedAmmoLoads;
		for (const FCFAmmoSortieLoad& AmmoLoad : Entry.DefaultSortieAmmoLoads)
		{
			SortedAmmoLoads.Add(&AmmoLoad);
		}
		SortedAmmoLoads.Sort(
			[](const FCFAmmoSortieLoad& Left, const FCFAmmoSortieLoad& Right)
			{
				// Left Ammo stable ID입니다.
				const FString LeftAmmoId =
					IsValid(Left.AmmoData) ? Left.AmmoData->AmmoId.ToString() : FString();
				// Right Ammo stable ID입니다.
				const FString RightAmmoId =
					IsValid(Right.AmmoData) ? Right.AmmoData->AmmoId.ToString() : FString();
				if (LeftAmmoId != RightAmmoId)
				{
					return LeftAmmoId < RightAmmoId;
				}
				return GetPathNameSafe(Left.AmmoData) < GetPathNameSafe(Right.AmmoData);
			});

		for (const FCFAmmoSortieLoad* AmmoLoad : SortedAmmoLoads)
		{
			if (AmmoLoad == nullptr)
			{
				continue;
			}

			CFDACommonPrimitives::AppendStringToken(
				OutBytes,
				TEXT("AmmoId"),
				IsValid(AmmoLoad->AmmoData)
					? AmmoLoad->AmmoData->AmmoId.ToString()
					: FString());
			CFDACommonPrimitives::AppendStringToken(
				OutBytes,
				TEXT("AmmoData"),
				GetPathNameSafe(AmmoLoad->AmmoData));
			AppendIntegerToken(
				OutBytes,
				TEXT("InitialSortieAmmoCount"),
				AmmoLoad->InitialSortieAmmoCount);
		}
	}

	// Published membership entries만으로 CatalogFingerprint를 생성합니다.
	bool BuildCatalogMembershipFingerprint(
		const TArray<FCFProdEquipCatalogEntry>& Entries,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// Stable ContentId order의 entry pointer 배열입니다.
		TArray<const FCFProdEquipCatalogEntry*> SortedEntries;
		for (const FCFProdEquipCatalogEntry& Entry : Entries)
		{
			SortedEntries.Add(&Entry);
		}
		SortedEntries.Sort(
			[](const FCFProdEquipCatalogEntry& Left, const FCFProdEquipCatalogEntry& Right)
			{
				return Left.ContentId < Right.ContentId;
			});

		// Catalog membership canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		CFDACommonPrimitives::AppendStringToken(
			CanonicalBytes,
			TEXT("Protocol"),
			TEXT("CarFight.CCAS.ProdEquipCatalogMembership/v1"));

		for (const FCFProdEquipCatalogEntry* Entry : SortedEntries)
		{
			if (Entry != nullptr)
			{
				AppendCatalogEntry(CanonicalBytes, *Entry);
			}
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	// Whole Catalog payload의 durable semantic fingerprint를 생성합니다.
	bool BuildCatalogPayloadFingerprint(
		const FCatalogPayload& Payload,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// Whole generated Catalog canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		CFDACommonPrimitives::AppendStringToken(
			CanonicalBytes,
			TEXT("Protocol"),
			TEXT("CarFight.CCAS.ProdEquipCatalogAsset/v1"));
		AppendIntegerToken(
			CanonicalBytes,
			TEXT("CatalogSchemaVersion"),
			Payload.CatalogSchemaVersion);
		CFDACommonPrimitives::AppendStringToken(
			CanonicalBytes,
			TEXT("CatalogFingerprint"),
			Payload.CatalogFingerprint);

		for (const FCFProdEquipCatalogEntry& Entry : Payload.PublishedEquipment)
		{
			AppendCatalogEntry(CanonicalBytes, Entry);
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	// Durable writer가 generated Catalog payload를 UObject에 materialize합니다.
	void MaterializeCatalogPayload(
		UCFProdEquipCatalogData& Catalog,
		const FCatalogPayload& Payload)
	{
		Catalog.CatalogSchemaVersion = Payload.CatalogSchemaVersion;
		Catalog.PublishedEquipment = Payload.PublishedEquipment;
		Catalog.CatalogFingerprint = Payload.CatalogFingerprint;
	}

	// Persisted generated Catalog UObject를 lossless provider-local payload로 추출합니다.
	bool ExtractCatalogPayload(
		const UCFProdEquipCatalogData& Catalog,
		FCatalogPayload& OutPayload,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutPayload = FCatalogPayload();
		OutIssues.Reset();

		OutPayload.CatalogSchemaVersion = Catalog.CatalogSchemaVersion;
		OutPayload.PublishedEquipment = Catalog.PublishedEquipment;
		OutPayload.CatalogFingerprint = Catalog.CatalogFingerprint;
		return true;
	}

	// Catalog entry 배열을 stable ContentId order로 정렬합니다.
	void SortCatalogEntries(TArray<FCFProdEquipCatalogEntry>& InOutEntries)
	{
		InOutEntries.Sort(
			[](const FCFProdEquipCatalogEntry& Left, const FCFProdEquipCatalogEntry& Right)
			{
				return Left.ContentId < Right.ContentId;
			});
	}

	// Transaction filename에 사용할 slash 없는 stable local ID인지 확인합니다.
	bool IsSafeLocalId(const FString& Value)
	{
		if (Value.IsEmpty() || Value.Len() > 160)
		{
			return false;
		}

		for (const TCHAR Character : Value)
		{
			if (!FChar::IsAlnum(Character)
				&& Character != TEXT('.')
				&& Character != TEXT('_')
				&& Character != TEXT('-'))
			{
				return false;
			}
		}
		return true;
	}

	// Product transaction directory physical path를 반환합니다.
	FString GetTransactionDirectory()
	{
		return FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("CCAS"),
			TEXT("ProductionTransactions"));
	}

	// Exact Product transaction JSON physical path를 반환합니다.
	FString GetTransactionPath(const FString& TransactionId)
	{
		return FPaths::Combine(
			GetTransactionDirectory(),
			TransactionId + TEXT(".json"));
	}

	// UTF-8 JSON을 sibling temp + readback + replace 방식으로 durable 저장합니다.
	bool AtomicWriteText(
		const FString& TargetPath,
		const FString& TextPayload,
		FString& OutError)
	{
		OutError.Reset();

		// Target parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(TargetPath);
		if (!IFileManager::Get().MakeDirectory(*ParentDirectory, true)
			&& !IFileManager::Get().DirectoryExists(*ParentDirectory))
		{
			OutError = TEXT("Production transaction parent directory 생성에 실패했습니다.");
			return false;
		}

		// Controlled sibling temporary file path입니다.
		const FString TempPath = TargetPath + TEXT(".production-writing");
		if (IFileManager::Get().FileExists(*TempPath)
			&& !IFileManager::Get().Delete(*TempPath, false, true, true))
		{
			OutError = TEXT("이전 Production transaction temp 파일을 정리하지 못했습니다: ") + TempPath;
			return false;
		}

		if (!FFileHelper::SaveStringToFile(
			TextPayload,
			*TempPath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			OutError = TEXT("Production transaction temp durable write가 실패했습니다.");
			return false;
		}

		// Temp exact readback payload입니다.
		FString TempReadback;
		if (!FFileHelper::LoadFileToString(TempReadback, *TempPath)
			|| !TempReadback.Equals(TextPayload, ESearchCase::CaseSensitive))
		{
			IFileManager::Get().Delete(*TempPath, false, true, true);
			OutError = TEXT("Production transaction temp readback이 write payload와 다릅니다.");
			return false;
		}

		if (!IFileManager::Get().Move(
			*TargetPath,
			*TempPath,
			true,
			true,
			false,
			true))
		{
			IFileManager::Get().Delete(*TempPath, false, true, true);
			OutError = TEXT("Production transaction durable target replace가 실패했습니다.");
			return false;
		}

		// Final exact readback payload입니다.
		FString FinalReadback;
		if (!FFileHelper::LoadFileToString(FinalReadback, *TargetPath)
			|| !FinalReadback.Equals(TextPayload, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Production transaction final readback이 write payload와 다릅니다.");
			return false;
		}
		return true;
	}

	// ContentKey를 transaction JSON object로 직렬화합니다.
	TSharedRef<FJsonObject> ContentKeyToJson(const FCFContentKey& ContentKey)
	{
		// Serialized ContentKey object입니다.
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField(TEXT("ContentTypeId"), ContentKey.ContentTypeId.Value);
		JsonObject->SetStringField(TEXT("ContentId"), ContentKey.ContentId);
		return JsonObject;
	}

	// Transaction JSON object에서 ContentKey를 역직렬화합니다.
	bool ContentKeyFromJson(
		const TSharedPtr<FJsonObject>& JsonObject,
		FCFContentKey& OutContentKey)
	{
		OutContentKey = FCFContentKey();
		return JsonObject.IsValid()
			&& JsonObject->TryGetStringField(
				TEXT("ContentTypeId"),
				OutContentKey.ContentTypeId.Value)
			&& JsonObject->TryGetStringField(
				TEXT("ContentId"),
				OutContentKey.ContentId)
			&& OutContentKey.IsValid();
	}

	// Product transaction을 deterministic compact JSON으로 직렬화합니다.
	bool SerializeTransaction(
		const FCFProdTransaction& Transaction,
		FString& OutJson,
		FString& OutError)
	{
		OutJson.Reset();
		OutError.Reset();

		if (!IsSafeLocalId(Transaction.TransactionId)
			|| !Transaction.ProductKey.IsValid())
		{
			OutError = TEXT("Production transaction identity가 유효하지 않습니다.");
			return false;
		}

		// Root transaction JSON object입니다.
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("SchemaId"), TransactionSchemaId);
		RootObject->SetStringField(TEXT("TransactionId"), Transaction.TransactionId);
		RootObject->SetObjectField(TEXT("ProductKey"), ContentKeyToJson(Transaction.ProductKey));
		RootObject->SetStringField(TEXT("ReviewPackageFingerprint"), Transaction.ReviewPackageFingerprint);
		RootObject->SetStringField(TEXT("BaseCatalogSnapshotFingerprint"), Transaction.BaseCatalogSnapshotFingerprint);
		RootObject->SetStringField(TEXT("ResourceCatalogFingerprint"), Transaction.ResourceCatalogFingerprint);
		RootObject->SetStringField(TEXT("ExecutionBindingFingerprint"), Transaction.ExecutionBindingFingerprint);
		RootObject->SetStringField(TEXT("WorkbookSemanticHash"), Transaction.WorkbookSemanticHash);
		RootObject->SetStringField(TEXT("RoleTopologyFingerprint"), Transaction.RoleTopologyFingerprint);
		RootObject->SetNumberField(TEXT("State"), static_cast<int32>(Transaction.State));
		RootObject->SetStringField(TEXT("ProductGraphFingerprint"), Transaction.ProductGraphFingerprint);
		RootObject->SetStringField(TEXT("Diagnostic"), Transaction.Diagnostic);

		// Stable composite identity order의 impacted Product pointers입니다.
		TArray<const FCFContentKey*> SortedImpactedProductKeys;
		for (const FCFContentKey& ProductKey : Transaction.ImpactedProductKeys)
		{
			SortedImpactedProductKeys.Add(&ProductKey);
		}
		SortedImpactedProductKeys.Sort(
			[](const FCFContentKey& Left, const FCFContentKey& Right)
			{
				return Left.ToStableString() < Right.ToStableString();
			});

		// Serialized impacted Product composite keys입니다.
		TArray<TSharedPtr<FJsonValue>> ImpactedProductValues;
		for (const FCFContentKey* ProductKey : SortedImpactedProductKeys)
		{
			if (ProductKey != nullptr)
			{
				ImpactedProductValues.Add(
					MakeShared<FJsonValueObject>(ContentKeyToJson(*ProductKey)));
			}
		}
		RootObject->SetArrayField(TEXT("ImpactedProductKeys"), ImpactedProductValues);

		// Stable ContentKey order의 target evidence pointers입니다.
		TArray<const FCFProdTargetEvidence*> SortedTargets;
		for (const FCFProdTargetEvidence& Target : Transaction.Targets)
		{
			SortedTargets.Add(&Target);
		}
		SortedTargets.Sort(
			[](const FCFProdTargetEvidence& Left, const FCFProdTargetEvidence& Right)
			{
				return Left.ContentKey.ToStableString() < Right.ContentKey.ToStableString();
			});

		// Serialized target evidence values입니다.
		TArray<TSharedPtr<FJsonValue>> TargetValues;
		for (const FCFProdTargetEvidence* Target : SortedTargets)
		{
			if (Target == nullptr)
			{
				continue;
			}

			// One target evidence JSON object입니다.
			TSharedRef<FJsonObject> TargetObject = MakeShared<FJsonObject>();
			TargetObject->SetObjectField(TEXT("ContentKey"), ContentKeyToJson(Target->ContentKey));
			TargetObject->SetStringField(TEXT("TargetObjectPath"), Target->TargetObjectPath);
			TargetObject->SetStringField(TEXT("DesiredFingerprint"), Target->DesiredFingerprint);
			TargetObject->SetBoolField(TEXT("RequireAbsentAtReview"), Target->bRequireAbsentAtReview);
			TargetObject->SetStringField(TEXT("PersistedFingerprint"), Target->PersistedFingerprint);
			TargetObject->SetBoolField(TEXT("MutationPerformed"), Target->bMutationPerformed);
			TargetValues.Add(MakeShared<FJsonValueObject>(TargetObject));
		}
		RootObject->SetArrayField(TEXT("Targets"), TargetValues);

		// Compact transaction JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(RootObject, JsonWriter))
		{
			OutError = TEXT("Production transaction JSON serialize에 실패했습니다.");
			return false;
		}
		return true;
	}

	// Durable JSON에서 Product transaction을 역직렬화합니다.
	bool DeserializeTransaction(
		const FString& JsonText,
		FCFProdTransaction& OutTransaction,
		FString& OutError)
	{
		OutTransaction = FCFProdTransaction();
		OutError.Reset();

		// Parsed root transaction JSON object입니다.
		TSharedPtr<FJsonObject> RootObject;
		// Transaction JSON reader입니다.
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(JsonReader, RootObject)
			|| !RootObject.IsValid())
		{
			OutError = TEXT("Production transaction JSON parse에 실패했습니다.");
			return false;
		}

		// Serialized schema identity입니다.
		FString SchemaId;
		if (!RootObject->TryGetStringField(TEXT("SchemaId"), SchemaId)
			|| !SchemaId.Equals(TransactionSchemaId, ESearchCase::CaseSensitive)
			|| !RootObject->TryGetStringField(TEXT("TransactionId"), OutTransaction.TransactionId)
			|| !IsSafeLocalId(OutTransaction.TransactionId))
		{
			OutError = TEXT("Production transaction schema/identity가 유효하지 않습니다.");
			return false;
		}

		// Serialized ProductKey object입니다.
		const TSharedPtr<FJsonObject>* ProductKeyObject = nullptr;
		if (!RootObject->TryGetObjectField(TEXT("ProductKey"), ProductKeyObject)
			|| ProductKeyObject == nullptr
			|| !ContentKeyFromJson(*ProductKeyObject, OutTransaction.ProductKey))
		{
			OutError = TEXT("Production transaction ProductKey가 유효하지 않습니다.");
			return false;
		}

		RootObject->TryGetStringField(TEXT("ReviewPackageFingerprint"), OutTransaction.ReviewPackageFingerprint);
		RootObject->TryGetStringField(TEXT("BaseCatalogSnapshotFingerprint"), OutTransaction.BaseCatalogSnapshotFingerprint);
		RootObject->TryGetStringField(TEXT("ResourceCatalogFingerprint"), OutTransaction.ResourceCatalogFingerprint);
		RootObject->TryGetStringField(TEXT("ExecutionBindingFingerprint"), OutTransaction.ExecutionBindingFingerprint);
		RootObject->TryGetStringField(TEXT("WorkbookSemanticHash"), OutTransaction.WorkbookSemanticHash);
		RootObject->TryGetStringField(TEXT("RoleTopologyFingerprint"), OutTransaction.RoleTopologyFingerprint);
		RootObject->TryGetStringField(TEXT("ProductGraphFingerprint"), OutTransaction.ProductGraphFingerprint);
		RootObject->TryGetStringField(TEXT("Diagnostic"), OutTransaction.Diagnostic);

		// Serialized transaction state number입니다.
		double StateNumber = 0.0;
		if (!RootObject->TryGetNumberField(TEXT("State"), StateNumber)
			|| StateNumber < 0.0
			|| StateNumber > static_cast<double>(ECFProdTxnState::RecoveryRequired))
		{
			OutError = TEXT("Production transaction state가 유효하지 않습니다.");
			return false;
		}
		OutTransaction.State = static_cast<ECFProdTxnState>(static_cast<uint8>(StateNumber));

		// Serialized impacted Product composite key values입니다.
		const TArray<TSharedPtr<FJsonValue>>* ImpactedProductValues = nullptr;
		if (!RootObject->TryGetArrayField(TEXT("ImpactedProductKeys"), ImpactedProductValues)
			|| ImpactedProductValues == nullptr)
		{
			OutError = TEXT("Production transaction ImpactedProductKeys array가 없습니다.");
			return false;
		}
		for (const TSharedPtr<FJsonValue>& Value : *ImpactedProductValues)
		{
			// Serialized impacted Product key object입니다.
			const TSharedPtr<FJsonObject> ImpactedProductKeyObject =
				Value.IsValid() ? Value->AsObject() : nullptr;
			// Parsed impacted Product composite key입니다.
			FCFContentKey ProductKey;
			if (!ContentKeyFromJson(ImpactedProductKeyObject, ProductKey))
			{
				OutError = TEXT("Production transaction impacted Product ContentKey parse에 실패했습니다.");
				return false;
			}
			OutTransaction.ImpactedProductKeys.Add(MoveTemp(ProductKey));
		}

		// Serialized target evidence array입니다.
		const TArray<TSharedPtr<FJsonValue>>* TargetValues = nullptr;
		if (!RootObject->TryGetArrayField(TEXT("Targets"), TargetValues)
			|| TargetValues == nullptr)
		{
			OutError = TEXT("Production transaction Targets array가 없습니다.");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& TargetValue : *TargetValues)
		{
			// Parsed target evidence object입니다.
			const TSharedPtr<FJsonObject> TargetObject =
				TargetValue.IsValid() ? TargetValue->AsObject() : nullptr;
			if (!TargetObject.IsValid())
			{
				OutError = TEXT("Production transaction target object가 유효하지 않습니다.");
				return false;
			}

			// Parsed target evidence입니다.
			FCFProdTargetEvidence TargetEvidence;
			// Serialized target ContentKey object입니다.
			const TSharedPtr<FJsonObject>* TargetKeyObject = nullptr;
			if (!TargetObject->TryGetObjectField(TEXT("ContentKey"), TargetKeyObject)
				|| TargetKeyObject == nullptr
				|| !ContentKeyFromJson(*TargetKeyObject, TargetEvidence.ContentKey)
				|| !TargetObject->TryGetStringField(
					TEXT("TargetObjectPath"),
					TargetEvidence.TargetObjectPath)
				|| !TargetObject->TryGetStringField(
					TEXT("DesiredFingerprint"),
					TargetEvidence.DesiredFingerprint)
				|| !TargetObject->TryGetBoolField(
					TEXT("RequireAbsentAtReview"),
					TargetEvidence.bRequireAbsentAtReview)
				|| !TargetObject->TryGetStringField(
					TEXT("PersistedFingerprint"),
					TargetEvidence.PersistedFingerprint)
				|| !TargetObject->TryGetBoolField(
					TEXT("MutationPerformed"),
					TargetEvidence.bMutationPerformed))
			{
				OutError = TEXT("Production transaction target evidence parse에 실패했습니다.");
				return false;
			}
			OutTransaction.Targets.Add(MoveTemp(TargetEvidence));
		}

		return true;
	}

	// Exact generated Catalog UObject를 persistent disk truth에서 읽습니다.
	bool LoadCatalog(
		const FString& CatalogObjectPath,
		bool& bOutExists,
		UCFProdEquipCatalogData*& OutCatalog,
		FString& OutError)
	{
		bOutExists = false;
		OutCatalog = nullptr;
		OutError.Reset();

		// Canonical Catalog soft object path입니다.
		const FSoftObjectPath ObjectPath(CatalogObjectPath);
		// Canonical Catalog package long name입니다.
		const FString PackageName = FPackageName::ObjectPathToPackageName(CatalogObjectPath);
		if (!ObjectPath.IsValid()
			|| !CatalogObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
			|| !FPackageName::IsValidLongPackageName(PackageName))
		{
			OutError = TEXT("Production Catalog object path가 canonical /Game object path가 아닙니다.");
			return false;
		}

		// Current loaded exact Catalog object입니다.
		OutCatalog = Cast<UCFProdEquipCatalogData>(ObjectPath.ResolveObject());
		if (OutCatalog == nullptr && FPackageName::DoesPackageExist(PackageName))
		{
			OutCatalog = LoadObject<UCFProdEquipCatalogData>(nullptr, *CatalogObjectPath);
		}

		if (OutCatalog == nullptr)
		{
			if (FPackageName::DoesPackageExist(PackageName))
			{
				OutError = TEXT("Production Catalog package가 존재하지만 UCFProdEquipCatalogData exact object를 읽지 못했습니다.");
				return false;
			}
			return true;
		}

		bOutExists = true;

		// Persisted Catalog package입니다.
		UPackage* Package = OutCatalog->GetOutermost();
		if (Package == nullptr || Package->IsDirty())
		{
			OutError = TEXT("Production Catalog package가 dirty/unconfirmed 상태라 publication authority로 사용할 수 없습니다.");
			return false;
		}

		// Persisted Catalog contract diagnostics입니다.
		TArray<FText> ValidationErrors;
		if (!OutCatalog->ValidateProductionCatalog(ValidationErrors))
		{
			OutError = ValidationErrors.IsEmpty()
				? TEXT("Production Catalog validation이 실패했습니다.")
				: ValidationErrors[0].ToString();
			return false;
		}

		return true;
	}

	// Exact generated Catalog entry set을 CFDADurableCore로 create/update/save/reload합니다.
	bool PersistCatalogEntries(
		const FString& CatalogObjectPath,
		TArray<FCFProdEquipCatalogEntry> DesiredEntries,
		FString& OutError)
	{
		OutError.Reset();
		SortCatalogEntries(DesiredEntries);

		// Desired generated Catalog payload입니다.
		FCatalogPayload DesiredPayload;
		DesiredPayload.PublishedEquipment = MoveTemp(DesiredEntries);
		if (!BuildCatalogMembershipFingerprint(
			DesiredPayload.PublishedEquipment,
			DesiredPayload.CatalogFingerprint,
			OutError))
		{
			return false;
		}

		// Desired whole Catalog semantic fingerprint입니다.
		FString DesiredPayloadFingerprint;
		if (!BuildCatalogPayloadFingerprint(
			DesiredPayload,
			DesiredPayloadFingerprint,
			OutError))
		{
			return false;
		}

		// Current Catalog existence/readback입니다.
		bool bCatalogExists = false;
		// Current Catalog UObject입니다.
		UCFProdEquipCatalogData* CurrentCatalog = nullptr;
		if (!LoadCatalog(
			CatalogObjectPath,
			bCatalogExists,
			CurrentCatalog,
			OutError))
		{
			return false;
		}

		// Generated Catalog durable apply row입니다.
		FCFDACommonPreviewRow Row;
		Row.Kind = bCatalogExists
			? ECFDAStagingPreviewKind::Update
			: ECFDAStagingPreviewKind::Create;
		Row.Envelope.TargetObjectPath = CatalogObjectPath;
		Row.Envelope.StagingSemanticFingerprint = DesiredPayloadFingerprint;

		// CFDADurableCore terminal apply report입니다.
		FCFDAStagingTargetApplyReport ApplyReport;
		CFDADurableCore::ApplyTypedTarget<UCFProdEquipCatalogData, FCatalogPayload>(
			Row,
			DesiredPayload,
			&MaterializeCatalogPayload,
			&ExtractCatalogPayload,
			&BuildCatalogPayloadFingerprint,
			ApplyReport);

		if (ApplyReport.Result != ECFDAStagingTargetApplyResult::DurableApplied)
		{
			OutError = ApplyReport.Diagnostic.IsEmpty()
				? TEXT("Production Catalog durable apply가 terminal DurableApplied에 도달하지 못했습니다.")
				: ApplyReport.Diagnostic;
			return false;
		}
		return true;
	}

	// Transaction target evidence에서 exact ContentKey를 찾습니다.
	FCFProdTargetEvidence* FindTargetEvidence(
		FCFProdTransaction& Transaction,
		const FCFContentKey& ContentKey)
	{
		return Transaction.Targets.FindByPredicate(
			[&ContentKey](const FCFProdTargetEvidence& Evidence)
			{
				return Evidence.ContentKey == ContentKey;
			});
	}

	// Const transaction target evidence에서 exact ContentKey를 찾습니다.
	const FCFProdTargetEvidence* FindTargetEvidence(
		const FCFProdTransaction& Transaction,
		const FCFContentKey& ContentKey)
	{
		return Transaction.Targets.FindByPredicate(
			[&ContentKey](const FCFProdTargetEvidence& Evidence)
			{
				return Evidence.ContentKey == ContentKey;
			});
	}

	// Request target 중 exact ContentId를 가진 target을 찾습니다.
	const FCFProdProvisionTarget* FindTargetByContentId(
		const FCFProdProvisionRequest& Request,
		const FString& ContentId)
	{
		return Request.Targets.FindByPredicate(
			[&ContentId](const FCFProdProvisionTarget& Target)
			{
				return Target.ContentKey.ContentId.Equals(
					ContentId,
					ESearchCase::CaseSensitive);
			});
	}

	// Published entry의 sortie ammo projection exact equality를 비교합니다.
	bool AreAmmoLoadsEquivalent(
		const TArray<FCFAmmoSortieLoad>& Left,
		const TArray<FCFAmmoSortieLoad>& Right)
	{
		if (Left.Num() != Right.Num())
		{
			return false;
		}

		// Stable AmmoId -> (object path,count) projection입니다.
		TMap<FString, FString> LeftProjection;
		for (const FCFAmmoSortieLoad& Load : Left)
		{
			// Stable ammo projection key입니다.
			const FString AmmoId = IsValid(Load.AmmoData)
				? Load.AmmoData->AmmoId.ToString()
				: FString();
			LeftProjection.Add(
				AmmoId,
				GetPathNameSafe(Load.AmmoData)
					+ TEXT("|")
					+ LexToString(Load.InitialSortieAmmoCount));
		}

		for (const FCFAmmoSortieLoad& Load : Right)
		{
			// Stable ammo projection key입니다.
			const FString AmmoId = IsValid(Load.AmmoData)
				? Load.AmmoData->AmmoId.ToString()
				: FString();
			// Left side exact projected value입니다.
			const FString* LeftValue = LeftProjection.Find(AmmoId);
			// Right side exact projected value입니다.
			const FString RightValue =
				GetPathNameSafe(Load.AmmoData)
				+ TEXT("|")
				+ LexToString(Load.InitialSortieAmmoCount);
			if (LeftValue == nullptr
				|| !LeftValue->Equals(RightValue, ESearchCase::CaseSensitive))
			{
				return false;
			}
		}
		return true;
	}

	// Existing transaction과 current request가 같은 desired transaction인지 exact 검증합니다.
	bool ValidateResumeIdentity(
		const FCFProdProvisionRequest& Request,
		const FCFProdTransaction& Transaction,
		FString& OutError)
	{
		OutError.Reset();

		if (!Transaction.TransactionId.Equals(Request.TransactionId, ESearchCase::CaseSensitive)
			|| !(Transaction.ProductKey == Request.ProductKey)
			|| !Transaction.ReviewPackageFingerprint.Equals(
				Request.ReviewPackageFingerprint,
				ESearchCase::CaseSensitive)
			|| !Transaction.BaseCatalogSnapshotFingerprint.Equals(
				Request.BaseCatalogSnapshotFingerprint,
				ESearchCase::CaseSensitive)
			|| !Transaction.ResourceCatalogFingerprint.Equals(
				Request.ResourceCatalogFingerprint,
				ESearchCase::CaseSensitive)
			|| !Transaction.ExecutionBindingFingerprint.Equals(
				Request.ExecutionBindingFingerprint,
				ESearchCase::CaseSensitive)
			|| !Transaction.WorkbookSemanticHash.Equals(
				Request.WorkbookSemanticHash,
				ESearchCase::CaseSensitive)
			|| !Transaction.RoleTopologyFingerprint.Equals(
				Request.RoleTopologyFingerprint,
				ESearchCase::CaseSensitive)
			|| Transaction.Targets.Num() != Request.Targets.Num())
		{
			OutError = TEXT("기존 Product transaction의 identity/Workbook/Role topology/target count가 current request와 다릅니다.");
			return false;
		}

		for (const FCFProdProvisionTarget& Target : Request.Targets)
		{
			// Durable transaction에 저장된 same target evidence입니다.
			const FCFProdTargetEvidence* Evidence =
				FindTargetEvidence(Transaction, Target.ContentKey);
			if (Evidence == nullptr
				|| !Evidence->TargetObjectPath.Equals(
					Target.TargetObjectPath,
					ESearchCase::CaseSensitive)
				|| !Evidence->DesiredFingerprint.Equals(
					Target.DesiredFingerprint,
					ESearchCase::CaseSensitive)
				|| Evidence->bRequireAbsentAtReview
					!= Target.bRequireAbsentAtReview)
			{
				OutError = TEXT("기존 Product transaction의 target object path / desired fingerprint set이 current request와 다릅니다.");
				return false;
			}
		}
		return true;
	}

	// Product request의 immutable identity와 target set을 mutation 전에 검증합니다.
	bool ValidateRequest(
		const FCFProdProvisionRequest& Request,
		FString& OutError)
	{
		OutError.Reset();

		if (!IsSafeLocalId(Request.TransactionId)
			|| !Request.ProductKey.IsValid()
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Request.ReviewPackageFingerprint)
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Request.BaseCatalogSnapshotFingerprint)
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Request.ResourceCatalogFingerprint)
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Request.ExecutionBindingFingerprint)
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Request.WorkbookSemanticHash)
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Request.RoleTopologyFingerprint)
			|| Request.EquipmentPresetObjectPath.IsEmpty()
			|| Request.Targets.IsEmpty())
		{
			OutError = TEXT("Production provisioning request identity/hash/EquipmentPreset/target set이 유효하지 않습니다.");
			return false;
		}

		// Duplicate ContentKey 검사용 set입니다.
		TSet<FString> SeenContentKeys;
		// Duplicate target object path 검사용 set입니다.
		TSet<FString> SeenTargetPaths;

		for (const FCFProdProvisionTarget& Target : Request.Targets)
		{
			// Target stable key text입니다.
			const FString TargetKeyText = Target.ContentKey.ToStableString();
			// Shared binding mode와 redundant shared flag가 exact 일치하는지 확인합니다.
			const bool bSharedBinding =
				Target.BindingMode != ECFProdBindingMode::CreateNew;
			if (!Target.ContentKey.IsValid()
				|| Target.RoleId.IsEmpty()
				|| Target.TargetObjectPath.IsEmpty()
				|| !Target.TargetObjectPath.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
				|| Target.bShared != bSharedBinding
				|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
					Target.DesiredFingerprint))
			{
				OutError = TEXT("Production provisioning target identity/path/fingerprint가 유효하지 않습니다.");
				return false;
			}

			if (SeenContentKeys.Contains(TargetKeyText)
				|| SeenTargetPaths.Contains(Target.TargetObjectPath))
			{
				OutError = TEXT("Production provisioning request에 duplicate ContentKey 또는 target object path가 있습니다.");
				return false;
			}
			SeenContentKeys.Add(TargetKeyText);
			SeenTargetPaths.Add(Target.TargetObjectPath);
		}

		return true;
	}

	// Fresh Product request에서 Prepared transaction snapshot을 deterministic하게 구성합니다.
	void InitializePreparedTransaction(
		const FCFProdProvisionRequest& Request,
		FCFProdTransaction& OutTransaction)
	{
		OutTransaction = FCFProdTransaction();
		OutTransaction.TransactionId = Request.TransactionId;
		OutTransaction.ProductKey = Request.ProductKey;
		OutTransaction.ReviewPackageFingerprint = Request.ReviewPackageFingerprint;
		OutTransaction.BaseCatalogSnapshotFingerprint = Request.BaseCatalogSnapshotFingerprint;
		OutTransaction.ResourceCatalogFingerprint = Request.ResourceCatalogFingerprint;
		OutTransaction.ExecutionBindingFingerprint = Request.ExecutionBindingFingerprint;
		OutTransaction.WorkbookSemanticHash = Request.WorkbookSemanticHash;
		OutTransaction.RoleTopologyFingerprint = Request.RoleTopologyFingerprint;
		OutTransaction.State = ECFProdTxnState::Prepared;

		for (const FCFProdProvisionTarget& Target : Request.Targets)
		{
			// Initial durable target evidence입니다.
			FCFProdTargetEvidence Evidence;
			Evidence.ContentKey = Target.ContentKey;
			Evidence.TargetObjectPath = Target.TargetObjectPath;
			Evidence.DesiredFingerprint = Target.DesiredFingerprint;
			Evidence.bRequireAbsentAtReview = Target.bRequireAbsentAtReview;
			OutTransaction.Targets.Add(MoveTemp(Evidence));
		}
	}

	// State를 durable 저장하고 실패 시 caller diagnostic을 보존합니다.
	bool SaveTransactionState(
		ICFProdTransactionStore& Store,
		FCFProdTransaction& Transaction,
		const TCHAR* FailureContext)
	{
		// Durable journal save failure 상세입니다.
		FString SaveError;
		if (Store.SaveTransaction(Transaction, SaveError))
		{
			return true;
		}

		Transaction.Diagnostic = FString::Printf(
			TEXT("%s: %s"),
			FailureContext,
			SaveError.IsEmpty() ? TEXT("unknown durable journal error") : *SaveError);
		return false;
	}
}

// Exact transaction evidence를 atomic sibling replace 방식으로 저장합니다.
bool FCFProdTransactionFileStore::SaveTransaction(
	const FCFProdTransaction& Transaction,
	FString& OutError)
{
	OutError.Reset();

	// Serialized Product transaction JSON입니다.
	FString JsonText;
	if (!CFProdProvisioningPrivate::SerializeTransaction(
		Transaction,
		JsonText,
		OutError))
	{
		return false;
	}

	return CFProdProvisioningPrivate::AtomicWriteText(
		CFProdProvisioningPrivate::GetTransactionPath(Transaction.TransactionId),
		JsonText,
		OutError);
}

// Exact TransactionId JSON을 읽고 schema/filename identity까지 검증합니다.
bool FCFProdTransactionFileStore::LoadTransaction(
	const FString& TransactionId,
	bool& bOutFound,
	FCFProdTransaction& OutTransaction,
	FString& OutError) const
{
	bOutFound = false;
	OutTransaction = FCFProdTransaction();
	OutError.Reset();

	if (!CFProdProvisioningPrivate::IsSafeLocalId(TransactionId))
	{
		OutError = TEXT("Production transaction lookup ID가 safe local ID가 아닙니다.");
		return false;
	}

	// Exact durable transaction JSON path입니다.
	const FString TransactionPath =
		CFProdProvisioningPrivate::GetTransactionPath(TransactionId);
	if (!IFileManager::Get().FileExists(*TransactionPath))
	{
		return true;
	}

	// Durable transaction JSON text입니다.
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *TransactionPath)
		|| !CFProdProvisioningPrivate::DeserializeTransaction(
			JsonText,
			OutTransaction,
			OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Production transaction durable JSON을 읽지 못했습니다.");
		}
		return false;
	}

	if (!OutTransaction.TransactionId.Equals(
		TransactionId,
		ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Production transaction filename identity와 payload TransactionId가 다릅니다.");
		return false;
	}

	bOutFound = true;
	return true;
}

// Canonical Production Catalog object path를 사용합니다.
FCFProdCatalogStore::FCFProdCatalogStore()
	: CatalogObjectPath(CFProdProvisioningPrivate::CanonicalCatalogObjectPath)
{
}

// Automation/disposable test에서만 별도 Catalog object path를 명시합니다.
FCFProdCatalogStore::FCFProdCatalogStore(const FString& InCatalogObjectPath)
	: CatalogObjectPath(InCatalogObjectPath)
{
}

// 현재 published Product ContentId exact set을 읽습니다.
bool FCFProdCatalogStore::ReadPublishedProductIds(
	TArray<FString>& OutProductContentIds,
	FString& OutError) const
{
	OutProductContentIds.Reset();
	OutError.Reset();

	// Current generated Catalog existence입니다.
	bool bCatalogExists = false;
	// Current generated Catalog UObject입니다.
	UCFProdEquipCatalogData* Catalog = nullptr;
	if (!CFProdProvisioningPrivate::LoadCatalog(
		CatalogObjectPath,
		bCatalogExists,
		Catalog,
		OutError))
	{
		return false;
	}

	if (!bCatalogExists || Catalog == nullptr)
	{
		return true;
	}

	for (const FCFProdEquipCatalogEntry& Entry : Catalog->PublishedEquipment)
	{
		OutProductContentIds.Add(Entry.ContentId);
	}
	OutProductContentIds.Sort();
	return true;
}

// Impact closure Product들을 durable Catalog update로 withdraw합니다.
bool FCFProdCatalogStore::WithdrawProducts(
	const TArray<FString>& ProductContentIds,
	FString& OutError)
{
	OutError.Reset();

	if (ProductContentIds.IsEmpty())
	{
		return true;
	}

	// Current generated Catalog existence입니다.
	bool bCatalogExists = false;
	// Current generated Catalog UObject입니다.
	UCFProdEquipCatalogData* Catalog = nullptr;
	if (!CFProdProvisioningPrivate::LoadCatalog(
		CatalogObjectPath,
		bCatalogExists,
		Catalog,
		OutError))
	{
		return false;
	}

	if (!bCatalogExists || Catalog == nullptr)
	{
		return true;
	}

	// Withdrawal lookup set입니다.
	TSet<FString> WithdrawSet;
	for (const FString& ProductContentId : ProductContentIds)
	{
		WithdrawSet.Add(ProductContentId);
	}

	// Desired post-withdraw Catalog entries입니다.
	TArray<FCFProdEquipCatalogEntry> DesiredEntries;
	for (const FCFProdEquipCatalogEntry& Entry : Catalog->PublishedEquipment)
	{
		if (!WithdrawSet.Contains(Entry.ContentId))
		{
			DesiredEntries.Add(Entry);
		}
	}

	if (DesiredEntries.Num() == Catalog->PublishedEquipment.Num())
	{
		return true;
	}

	return CFProdProvisioningPrivate::PersistCatalogEntries(
		CatalogObjectPath,
		MoveTemp(DesiredEntries),
		OutError);
}

// RuntimeVerified Product exact1을 durable Catalog create/update로 publish합니다.
bool FCFProdCatalogStore::PublishProduct(
	const FCFProdEquipCatalogEntry& Entry,
	FString& OutError)
{
	OutError.Reset();

	if (!CFIsStableContentId(Entry.ContentId)
		|| !IsValid(Entry.EquipmentPresetData)
		|| !Entry.EquipmentPresetData->HasCompleteEquipmentData()
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Entry.ProductGraphFingerprint))
	{
		OutError = TEXT("Publish Product entry identity/EquipmentPreset/ProductGraphFingerprint가 유효하지 않습니다.");
		return false;
	}

	// Current generated Catalog existence입니다.
	bool bCatalogExists = false;
	// Current generated Catalog UObject입니다.
	UCFProdEquipCatalogData* Catalog = nullptr;
	if (!CFProdProvisioningPrivate::LoadCatalog(
		CatalogObjectPath,
		bCatalogExists,
		Catalog,
		OutError))
	{
		return false;
	}

	// Desired post-publish Catalog entries입니다.
	TArray<FCFProdEquipCatalogEntry> DesiredEntries;
	if (bCatalogExists && Catalog != nullptr)
	{
		DesiredEntries = Catalog->PublishedEquipment;
	}

	// Existing same Product entry index입니다.
	const int32 ExistingIndex = DesiredEntries.IndexOfByPredicate(
		[&Entry](const FCFProdEquipCatalogEntry& ExistingEntry)
		{
			return ExistingEntry.ContentId.Equals(
				Entry.ContentId,
				ESearchCase::CaseSensitive);
		});
	if (ExistingIndex == INDEX_NONE)
	{
		DesiredEntries.Add(Entry);
	}
	else
	{
		DesiredEntries[ExistingIndex] = Entry;
	}

	return CFProdProvisioningPrivate::PersistCatalogEntries(
		CatalogObjectPath,
		MoveTemp(DesiredEntries),
		OutError);
}

// Publication 뒤 동일 canonical Product entry를 persisted Catalog에서 fresh readback합니다.
bool FCFProdCatalogStore::ReadPublishedEntry(
	const FString& ProductContentId,
	FCFProdEquipCatalogEntry& OutEntry,
	FString& OutError) const
{
	OutEntry = FCFProdEquipCatalogEntry();
	OutError.Reset();

	// Current generated Catalog existence입니다.
	bool bCatalogExists = false;
	// Current generated Catalog UObject입니다.
	UCFProdEquipCatalogData* Catalog = nullptr;
	if (!CFProdProvisioningPrivate::LoadCatalog(
		CatalogObjectPath,
		bCatalogExists,
		Catalog,
		OutError))
	{
		return false;
	}

	if (!bCatalogExists || Catalog == nullptr)
	{
		OutError = TEXT("Production Catalog가 아직 존재하지 않습니다.");
		return false;
	}

	// Same canonical Product published entry입니다.
	const FCFProdEquipCatalogEntry* Entry =
		Catalog->FindEntryByContentId(ProductContentId);
	if (Entry == nullptr)
	{
		OutError = TEXT("Production Catalog에서 requested canonical Product를 찾지 못했습니다.");
		return false;
	}

	OutEntry = *Entry;
	return true;
}

// 기존 Runtime Apply service를 통해 technical apply/readback까지 검증합니다.
bool FCFProdRuntimeVerifier::VerifyRuntime(
	ACFVehiclePawn* VehiclePawn,
	FName TargetMountProfileId,
	UCFEquipmentPresetData* EquipmentPresetData,
	const TArray<FCFAmmoSortieLoad>& InitialSortieAmmoLoads,
	FString& OutError)
{
	OutError.Reset();

	if (!IsValid(VehiclePawn)
		|| TargetMountProfileId.IsNone()
		|| !IsValid(EquipmentPresetData))
	{
		OutError = TEXT("Production Runtime proof에 Vehicle/Mount/EquipmentPreset 입력이 부족합니다.");
		return false;
	}

	// Existing Fitting/Runtime authority가 반환한 direct technical apply 결과입니다.
	const FCFRuntimeEquipApplyResult Result =
		FCFRuntimeEquipApplyService::ApplyEquipmentRuntimeWithAmmoLoads(
			VehiclePawn,
			TargetMountProfileId,
			EquipmentPresetData,
			InitialSortieAmmoLoads);

	if (!Result.IsSuccessful()
		|| !Result.bRuntimeReady
		|| !Result.CurrentEquipmentPath.Equals(
			GetPathNameSafe(EquipmentPresetData),
			ESearchCase::CaseSensitive))
	{
		OutError = Result.Message.IsEmpty()
			? TEXT("Production direct Runtime technical apply/readback이 실패했습니다.")
			: Result.Message;
		return false;
	}

	return true;
}

// EquipmentPresetAmmoLoads child sheet descriptor를 frozen P0-07 schema로 구성합니다.
FCFContentSheetDescriptor FCFProdWeaponSchema::BuildAmmoLoadSheetDescriptor()
{
	// Weapon Provider-owned ammo-load child sheet descriptor입니다.
	FCFContentSheetDescriptor Sheet;
	Sheet.SheetId = TEXT("EquipmentPresetAmmoLoads");
	Sheet.SchemaRevision = 1;
	Sheet.ContentTypeId.Value = TEXT("Weapon");
	Sheet.ParentSheetId = TEXT("EquipmentPresets");
	Sheet.CollectionId = TEXT("EquipmentPresetAmmoLoads");
	Sheet.CollectionKind = ECFContentCollectionKind::KeyedCollection;

	// AmmoRoleContentId canonical field descriptor입니다.
	FCFContentFieldDescriptor AmmoRoleField;
	AmmoRoleField.ColumnId = TEXT("AmmoRoleContentId");
	AmmoRoleField.DisplayLabel = TEXT("Ammo Role Content ID");
	AmmoRoleField.ValueType = ECFContentValueType::NameId;
	AmmoRoleField.Ownership = ECFContentFieldOwnership::CCASManaged;
	AmmoRoleField.bRequired = true;
	Sheet.Fields.Add(MoveTemp(AmmoRoleField));

	// DefaultSortieAmmoCount canonical field descriptor입니다.
	FCFContentFieldDescriptor SortieCountField;
	SortieCountField.ColumnId = TEXT("DefaultSortieAmmoCount");
	SortieCountField.DisplayLabel = TEXT("Default Sortie Ammo Count");
	SortieCountField.ValueType = ECFContentValueType::SignedInteger;
	SortieCountField.Ownership = ECFContentFieldOwnership::CCASManaged;
	SortieCountField.bRequired = true;
	Sheet.Fields.Add(MoveTemp(SortieCountField));

	return Sheet;
}

// EquipmentPreset record의 EquipmentPresetAmmoLoads collection을 typed rows로 projection합니다.
bool FCFProdWeaponSchema::ProjectAmmoLoadRows(
	const FCFContentRecord& EquipmentPresetRecord,
	TArray<FCFProdAmmoLoadRow>& OutRows,
	FString& OutError)
{
	OutRows.Reset();
	OutError.Reset();

	if (!EquipmentPresetRecord.Key.IsValid())
	{
		OutError = TEXT("EquipmentPresetAmmoLoads parent record ContentKey가 유효하지 않습니다.");
		return false;
	}

	// Provider-owned ammo load collection입니다.
	const FCFContentCollection* Collection =
		EquipmentPresetRecord.Collections.Find(TEXT("EquipmentPresetAmmoLoads"));
	if (Collection == nullptr)
	{
		return true;
	}

	if (Collection->Kind != ECFContentCollectionKind::KeyedCollection)
	{
		OutError = TEXT("EquipmentPresetAmmoLoads collection은 KeyedCollection이어야 합니다.");
		return false;
	}

	// Duplicate AmmoRoleContentId 검사용 set입니다.
	TSet<FString> SeenAmmoRoleContentIds;

	for (const FCFContentCollectionItem& Item : Collection->Items)
	{
		// Ammo Role ContentId canonical cell입니다.
		const FCFContentValue* AmmoRoleValue =
			Item.Fields.Find(TEXT("AmmoRoleContentId"));
		// Explicit sortie count canonical cell입니다.
		const FCFContentValue* SortieCountValue =
			Item.Fields.Find(TEXT("DefaultSortieAmmoCount"));

		if (AmmoRoleValue == nullptr
			|| AmmoRoleValue->Type != ECFContentValueType::NameId
			|| AmmoRoleValue->State != ECFContentValueState::Value
			|| SortieCountValue == nullptr
			|| SortieCountValue->Type != ECFContentValueType::SignedInteger
			|| SortieCountValue->State != ECFContentValueState::Value)
		{
			OutError = TEXT("EquipmentPresetAmmoLoads row의 AmmoRoleContentId/DefaultSortieAmmoCount typed VALUE가 부족합니다.");
			return false;
		}

		// Projected Ammo Role ContentId입니다.
		const FString AmmoRoleContentId = AmmoRoleValue->StringValue;
		if (!CFIsStableContentId(AmmoRoleContentId)
			|| !Item.DomainKey.Equals(
				AmmoRoleContentId,
				ESearchCase::CaseSensitive))
		{
			OutError = TEXT("EquipmentPresetAmmoLoads DomainKey와 AmmoRoleContentId가 canonical exact match가 아닙니다.");
			return false;
		}

		if (SeenAmmoRoleContentIds.Contains(AmmoRoleContentId))
		{
			OutError = TEXT("EquipmentPresetAmmoLoads에 duplicate AmmoRoleContentId가 있습니다.");
			return false;
		}
		SeenAmmoRoleContentIds.Add(AmmoRoleContentId);

		if (SortieCountValue->SignedIntegerValue < 0
			|| SortieCountValue->SignedIntegerValue > MAX_int32)
		{
			OutError = TEXT("DefaultSortieAmmoCount가 int32 non-negative 범위를 벗어났습니다.");
			return false;
		}

		// Typed Product ammo-load row projection입니다.
		FCFProdAmmoLoadRow Row;
		Row.ProductContentId = EquipmentPresetRecord.Key.ContentId;
		Row.AmmoRoleContentId = AmmoRoleContentId;
		Row.DefaultSortieAmmoCount =
			static_cast<int32>(SortieCountValue->SignedIntegerValue);
		OutRows.Add(MoveTemp(Row));
	}

	OutRows.Sort(
		[](const FCFProdAmmoLoadRow& Left, const FCFProdAmmoLoadRow& Right)
		{
			return Left.AmmoRoleContentId < Right.AmmoRoleContentId;
		});
	return true;
}

// Existing dependency graph의 reverse closure에서 현재 published Product만 계산합니다.
bool FCFProdProvisioning::BuildPublishedImpactClosure(
	const FCFProdProvisionRequest& Request,
	const TArray<FCFContentKey>& ChangedContentKeys,
	const TArray<FString>& PublishedProductContentIds,
	TArray<FString>& OutImpactedProductContentIds,
	FString& OutError)
{
	OutImpactedProductContentIds.Reset();
	OutError.Reset();

	if (!Request.ProductKey.IsValid())
	{
		OutError = TEXT("Impact closure ProductKey가 유효하지 않습니다.");
		return false;
	}

	// Published Product ContentId lookup set입니다.
	TSet<FString> PublishedSet;
	for (const FString& ProductContentId : PublishedProductContentIds)
	{
		PublishedSet.Add(ProductContentId);
	}

	// Reverse dependency adjacency: target stable key -> direct consumer ContentKeys입니다.
	TMap<FString, TArray<FCFContentKey>> ReverseAdjacency;
	for (const FCFContentDependencyEdge& Edge : Request.DependencyEdges)
	{
		if (!Edge.From.IsValid() || !Edge.To.IsValid())
		{
			OutError = TEXT("Impact closure dependency edge endpoint가 유효하지 않습니다.");
			return false;
		}
		ReverseAdjacency.FindOrAdd(Edge.To.ToStableString()).Add(Edge.From);
	}

	// Reverse traversal queue입니다.
	TArray<FCFContentKey> Queue = ChangedContentKeys;
	// Reverse traversal visited stable key set입니다.
	TSet<FString> VisitedKeys;
	// Impacted published Product ID set입니다.
	TSet<FString> ImpactedSet;

	for (int32 QueueIndex = 0; QueueIndex < Queue.Num(); ++QueueIndex)
	{
		// Current reverse traversal key입니다.
		const FCFContentKey CurrentKey = Queue[QueueIndex];
		// Current reverse traversal stable key text입니다.
		const FString CurrentStableKey = CurrentKey.ToStableString();
		if (VisitedKeys.Contains(CurrentStableKey))
		{
			continue;
		}
		VisitedKeys.Add(CurrentStableKey);

		// Publication membership의 ContentId는 Product type 안에서만 의미가 있으므로 child type의 동일 ID를 Product로 오인하지 않습니다.
		if (CurrentKey.ContentTypeId == Request.ProductKey.ContentTypeId
			&& PublishedSet.Contains(CurrentKey.ContentId))
		{
			ImpactedSet.Add(CurrentKey.ContentId);
		}

		// Direct consumer list입니다.
		const TArray<FCFContentKey>* Consumers =
			ReverseAdjacency.Find(CurrentStableKey);
		if (Consumers == nullptr)
		{
			continue;
		}

		for (const FCFContentKey& Consumer : *Consumers)
		{
			Queue.Add(Consumer);
		}
	}

	for (const FString& ProductContentId : ImpactedSet)
	{
		OutImpactedProductContentIds.Add(ProductContentId);
	}
	OutImpactedProductContentIds.Sort();
	return true;
}

// Required transitive target persisted fingerprints를 포함하는 ProductGraphFingerprint를 생성합니다.
bool FCFProdProvisioning::BuildProductGraphFingerprint(
	const FCFProdProvisionRequest& Request,
	const TArray<FCFProdTargetEvidence>& TargetEvidence,
	FString& OutFingerprint,
	FString& OutError)
{
	OutFingerprint.Reset();
	OutError.Reset();

	if (!Request.ProductKey.IsValid()
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Request.WorkbookSemanticHash)
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Request.RoleTopologyFingerprint))
	{
		OutError = TEXT("Product graph fingerprint input identity/hash가 유효하지 않습니다.");
		return false;
	}

	// Required target pointer set입니다.
	TArray<const FCFProdProvisionTarget*> RequiredTargets;
	for (const FCFProdProvisionTarget& Target : Request.Targets)
	{
		if (Target.bRequired)
		{
			RequiredTargets.Add(&Target);
		}
	}
	RequiredTargets.Sort(
		[](const FCFProdProvisionTarget& Left, const FCFProdProvisionTarget& Right)
		{
			if (Left.RoleId != Right.RoleId)
			{
				return Left.RoleId < Right.RoleId;
			}
			return Left.ContentKey.ToStableString()
				< Right.ContentKey.ToStableString();
		});

	// Product graph canonical byte stream입니다.
	TArray<uint8> CanonicalBytes;
	CFDACommonPrimitives::AppendStringToken(
		CanonicalBytes,
		TEXT("Protocol"),
		TEXT("CarFight.CCAS.ProductGraph/v1"));
	CFProdProvisioningPrivate::AppendContentKey(
		CanonicalBytes,
		TEXT("ProductKey"),
		Request.ProductKey);
	CFDACommonPrimitives::AppendStringToken(
		CanonicalBytes,
		TEXT("WorkbookSemanticHash"),
		Request.WorkbookSemanticHash);
	CFDACommonPrimitives::AppendStringToken(
		CanonicalBytes,
		TEXT("RoleTopologyFingerprint"),
		Request.RoleTopologyFingerprint);

	for (const FCFProdProvisionTarget* Target : RequiredTargets)
	{
		if (Target == nullptr)
		{
			continue;
		}

		// Matching persisted target evidence입니다.
		const FCFProdTargetEvidence* Evidence =
			TargetEvidence.FindByPredicate(
				[Target](const FCFProdTargetEvidence& Candidate)
				{
					return Candidate.ContentKey == Target->ContentKey;
				});
		if (Evidence == nullptr
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Evidence->PersistedFingerprint)
			|| !Evidence->PersistedFingerprint.Equals(
				Target->DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Required Product Role target의 persisted fingerprint가 desired state와 exact 일치하지 않습니다.");
			return false;
		}

		CFDACommonPrimitives::AppendStringToken(
			CanonicalBytes,
			TEXT("RoleId"),
			Target->RoleId);
		CFProdProvisioningPrivate::AppendContentKey(
			CanonicalBytes,
			TEXT("TargetKey"),
			Target->ContentKey);
		CFDACommonPrimitives::AppendStringToken(
			CanonicalBytes,
			TEXT("TargetObjectPath"),
			Target->TargetObjectPath);
		CFDACommonPrimitives::AppendStringToken(
			CanonicalBytes,
			TEXT("TargetFingerprint"),
			Evidence->PersistedFingerprint);
	}

	return CFDACommonPrimitives::HashCanonicalBytes(
		CanonicalBytes,
		OutFingerprint,
		OutError);
}

// Workbook explicit ammo rows와 current fitting을 merge해 Runtime candidate InitialSortieAmmoLoads를 구성합니다.
bool FCFProdProvisioning::BuildRuntimeAmmoLoads(
	const FCFProdProvisionRequest& Request,
	const UCFEquipmentPresetData& EquipmentPresetData,
	const ACFVehiclePawn* VehiclePawn,
	TArray<FCFAmmoSortieLoad>& OutAmmoLoads,
	FString& OutError)
{
	OutAmmoLoads.Reset();
	OutError.Reset();

	if (!EquipmentPresetData.EquipmentId.ToString().Equals(
		Request.ProductKey.ContentId,
		ESearchCase::CaseSensitive))
	{
		OutError = TEXT("EquipmentPresetData.EquipmentId가 canonical Product ContentId와 다릅니다.");
		return false;
	}

	// Current fitting이 있으면 다른 Mount의 출격 탄약 load를 보존할 initial merge base입니다.
	if (IsValid(VehiclePawn) && IsValid(VehiclePawn->VehicleFittingData))
	{
		OutAmmoLoads = VehiclePawn->VehicleFittingData->InitialSortieAmmoLoads;
	}

	// Product row duplicate 검사용 Ammo Role set입니다.
	TSet<FString> SeenAmmoRoleContentIds;
	// Product explicit row로 resolve된 AmmoData set입니다.
	TSet<FName> ExplicitAmmoIds;

	for (const FCFProdAmmoLoadRow& Row : Request.AmmoLoadRows)
	{
		if (!Row.ProductContentId.Equals(
			Request.ProductKey.ContentId,
			ESearchCase::CaseSensitive)
			|| !CFIsStableContentId(Row.AmmoRoleContentId)
			|| Row.DefaultSortieAmmoCount < 0
			|| SeenAmmoRoleContentIds.Contains(Row.AmmoRoleContentId))
		{
			OutError = TEXT("EquipmentPresetAmmoLoads Product/AmmoRole/count identity가 유효하지 않거나 중복됐습니다.");
			return false;
		}
		SeenAmmoRoleContentIds.Add(Row.AmmoRoleContentId);

		// Ammo Role ContentId가 resolve한 exact provisioning target입니다.
		const FCFProdProvisionTarget* AmmoTarget =
			CFProdProvisioningPrivate::FindTargetByContentId(
				Request,
				Row.AmmoRoleContentId);
		if (AmmoTarget == nullptr)
		{
			OutError = TEXT("EquipmentPresetAmmoLoads의 AmmoRoleContentId를 provisioning target으로 resolve하지 못했습니다.");
			return false;
		}

		// Persisted resolved AmmoData입니다.
		UCFAmmoData* AmmoData =
			LoadObject<UCFAmmoData>(nullptr, *AmmoTarget->TargetObjectPath);
		if (!IsValid(AmmoData) || !AmmoData->IsAmmoDataValid())
		{
			OutError = TEXT("EquipmentPresetAmmoLoads가 resolve한 persisted AmmoData가 유효하지 않습니다.");
			return false;
		}

		if (Row.DefaultSortieAmmoCount
			> AmmoData->GetEffectiveMaximumLoadableAmmoCount())
		{
			OutError = FString::Printf(
				TEXT("DefaultSortieAmmoCount %d가 AmmoData 최대 적재량 %d를 초과합니다: %s"),
				Row.DefaultSortieAmmoCount,
				AmmoData->GetEffectiveMaximumLoadableAmmoCount(),
				*AmmoData->AmmoId.ToString());
			return false;
		}

		ExplicitAmmoIds.Add(AmmoData->AmmoId);

		// Existing same AmmoId load index입니다.
		const int32 ExistingLoadIndex = OutAmmoLoads.IndexOfByPredicate(
			[AmmoData](const FCFAmmoSortieLoad& ExistingLoad)
			{
				return IsValid(ExistingLoad.AmmoData)
					&& ExistingLoad.AmmoData->AmmoId == AmmoData->AmmoId;
			});

		// Explicit Workbook default sortie load입니다.
		FCFAmmoSortieLoad ProjectedLoad;
		ProjectedLoad.AmmoData = AmmoData;
		ProjectedLoad.InitialSortieAmmoCount = Row.DefaultSortieAmmoCount;
		if (ExistingLoadIndex == INDEX_NONE)
		{
			OutAmmoLoads.Add(ProjectedLoad);
		}
		else
		{
			OutAmmoLoads[ExistingLoadIndex] = ProjectedLoad;
		}
	}

	// EquipmentPreset이 가리키는 Product WeaponData입니다.
	const UCFWeaponData* WeaponData = EquipmentPresetData.DefaultWeaponData.Get();
	if (!IsValid(WeaponData))
	{
		OutError = TEXT("Production EquipmentPreset에 persisted WeaponData가 없습니다.");
		return false;
	}

	if (!WeaponData->WeaponId.ToString().Equals(
		Request.ProductKey.ContentId,
		ESearchCase::CaseSensitive))
	{
		OutError = TEXT("WeaponData.WeaponId가 canonical Product ContentId와 다릅니다.");
		return false;
	}

	if (WeaponData->UsesFiniteAmmoRuntime())
	{
		// Product finite Weapon이 요구하는 exact default AmmoData입니다.
		const UCFAmmoData* RequiredAmmoData = WeaponData->DefaultAmmoData.Get();
		if (!IsValid(RequiredAmmoData)
			|| !RequiredAmmoData->IsAmmoDataValid()
			|| !ExplicitAmmoIds.Contains(RequiredAmmoData->AmmoId))
		{
			OutError = TEXT("Finite Product Weapon의 DefaultAmmoData에 대응하는 EquipmentPresetAmmoLoads row exact1이 없습니다.");
			return false;
		}

		// Required AmmoData용 final sortie load입니다.
		const FCFAmmoSortieLoad* RequiredLoad =
			OutAmmoLoads.FindByPredicate(
				[RequiredAmmoData](const FCFAmmoSortieLoad& Candidate)
				{
					return IsValid(Candidate.AmmoData)
						&& Candidate.AmmoData->AmmoId == RequiredAmmoData->AmmoId;
				});
		if (RequiredLoad == nullptr
			|| RequiredLoad->InitialSortieAmmoCount
				< WeaponData->GetEffectiveInitialLoadedAmmoCount())
		{
			OutError = TEXT("DefaultSortieAmmoCount가 finite Weapon의 InitialLoadedAmmoCount를 만족하지 않습니다.");
			return false;
		}
	}

	OutAmmoLoads.Sort(
		[](const FCFAmmoSortieLoad& Left, const FCFAmmoSortieLoad& Right)
		{
			// Left stable AmmoId입니다.
			const FString LeftId =
				IsValid(Left.AmmoData) ? Left.AmmoData->AmmoId.ToString() : FString();
			// Right stable AmmoId입니다.
			const FString RightId =
				IsValid(Right.AmmoData) ? Right.AmmoData->AmmoId.ToString() : FString();
			return LeftId < RightId;
		});
	return true;
}

// Product-level transaction을 withdrawal→typed persist→graph verify→Runtime proof→publish→VehicleReady 순으로 실행/재개합니다.
bool FCFProdProvisioning::ExecuteProductTransaction(
	const FCFProdProvisionRequest& Request,
	ICFProdTypedTargetAdapter& TypedTargetAdapter,
	ICFProdTransactionStore& TransactionStore,
	ICFProdPublicationStore& PublicationStore,
	ICFProdRuntimeVerifier& RuntimeVerifier,
	ACFVehiclePawn* VehiclePawn,
	FName TargetMountProfileId,
	FCFProdTransaction& OutTransaction)
{
	OutTransaction = FCFProdTransaction();

	// Pre-mutation request validation error입니다.
	FString ValidationError;
	if (!CFProdProvisioningPrivate::ValidateRequest(
		Request,
		ValidationError))
	{
		OutTransaction.TransactionId = Request.TransactionId;
		OutTransaction.ProductKey = Request.ProductKey;
		OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = ValidationError;
		return false;
	}

	// Existing durable transaction existence입니다.
	bool bExistingTransactionFound = false;
	// Existing durable transaction load error입니다.
	FString LoadError;
	if (!TransactionStore.LoadTransaction(
		Request.TransactionId,
		bExistingTransactionFound,
		OutTransaction,
		LoadError))
	{
		OutTransaction.TransactionId = Request.TransactionId;
		OutTransaction.ProductKey = Request.ProductKey;
		OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = LoadError;
		return false;
	}

	if (bExistingTransactionFound)
	{
		if (!CFProdProvisioningPrivate::ValidateResumeIdentity(
			Request,
			OutTransaction,
			ValidationError))
		{
			OutTransaction.Diagnostic = ValidationError;
			return false;
		}
	}
	else
	{
		CFProdProvisioningPrivate::InitializePreparedTransaction(
			Request,
			OutTransaction);

		if (!CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Prepared transaction durable save 실패")))
		{
			OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
			return false;
		}
	}

	// Dependency-first deterministic target pointers입니다.
	TArray<const FCFProdProvisionTarget*> OrderedTargets;
	for (const FCFProdProvisionTarget& Target : Request.Targets)
	{
		OrderedTargets.Add(&Target);
	}
	OrderedTargets.Sort(
		[](const FCFProdProvisionTarget& Left, const FCFProdProvisionTarget& Right)
		{
			if (Left.DependencyOrder != Right.DependencyOrder)
			{
				return Left.DependencyOrder < Right.DependencyOrder;
			}
			return Left.ContentKey.ToStableString()
				< Right.ContentKey.ToStableString();
		});

	// Preflight 결과 실제 persistent mutation이 필요한 ContentKeys입니다.
	TArray<FCFContentKey> ChangedContentKeys;

	for (const FCFProdProvisionTarget* Target : OrderedTargets)
	{
		if (Target == nullptr)
		{
			continue;
		}

		// Fresh target persisted existence입니다.
		bool bExists = false;
		// Fresh target persisted fingerprint입니다.
		FString CurrentFingerprint;
		// Fresh target read error입니다.
		FString ReadError;
		if (!TypedTargetAdapter.ReadPersistedFingerprint(
			*Target,
			bExists,
			CurrentFingerprint,
			ReadError))
		{
			OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic = ReadError;
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("BlockedBeforeMutation journal save 실패"));
			return false;
		}

		// Durable transaction same target evidence입니다.
		FCFProdTargetEvidence* Evidence =
			CFProdProvisioningPrivate::FindTargetEvidence(
				OutTransaction,
				Target->ContentKey);
		if (Evidence == nullptr)
		{
			OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic = TEXT("Product transaction target evidence가 누락됐습니다.");
			return false;
		}

		if (bExists
			&& CurrentFingerprint.Equals(
				Target->DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			Evidence->PersistedFingerprint = CurrentFingerprint;
			continue;
		}

		// First Wave처럼 review에서 absent를 요구한 신규 target이 다른 persisted payload로 선점됐으면 stale review입니다.
		if (!bExistingTransactionFound
			&& Target->bRequireAbsentAtReview
			&& bExists)
		{
			OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic =
				TEXT("Review 당시 absent를 요구한 CreateNew target이 execution 전에 다른 persisted payload로 나타나 stale review를 차단했습니다.");
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("Unexpected-existing CreateNew collision journal save 실패"));
			return false;
		}

		// BindShared consumer는 absent shared target의 생성 authority가 없습니다.
		if (!bExists && Target->BindingMode == ECFProdBindingMode::BindShared)
		{
			OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic = TEXT("BindShared target이 absent인데 이 Product에는 shared target 생성 authority가 없습니다.");
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("BindShared absence journal save 실패"));
			return false;
		}

		// 이전 run에서 confirmed target이 이후 drift한 경우 동일 transaction auto-resume을 금지합니다.
		if (bExistingTransactionFound
			&& (Evidence->bMutationPerformed
				|| !Evidence->PersistedFingerprint.IsEmpty()))
		{
			OutTransaction.State = ECFProdTxnState::RecoveryRequired;
			OutTransaction.Diagnostic = TEXT("이전 Product transaction에서 confirmed된 target이 desired fingerprint와 달라져 동일 transaction 자동 재개를 차단했습니다.");
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("RecoveryRequired drift journal save 실패"));
			return false;
		}

		ChangedContentKeys.Add(Target->ContentKey);
	}

	// Current published Product exact set입니다.
	TArray<FString> PublishedProductContentIds;
	// Publication read error입니다.
	FString PublicationError;
	if (!PublicationStore.ReadPublishedProductIds(
		PublishedProductContentIds,
		PublicationError))
	{
		OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = PublicationError;
		return false;
	}

	// Current mutation set이 영향을 주는 published Product ContentId closure입니다.
	TArray<FString> ImpactedPublishedProducts;
	// Existing non-terminal recovery transaction이 이전 withdrawal scope를 이미 durable하게 결속했는지 여부입니다.
	const bool bReuseDurableImpactScope =
		bExistingTransactionFound
		&& OutTransaction.State != ECFProdTxnState::VehicleReady
		&& !OutTransaction.ImpactedProductKeys.IsEmpty();

	if (bReuseDurableImpactScope)
	{
		for (const FCFContentKey& ImpactedProductKey : OutTransaction.ImpactedProductKeys)
		{
			if (!(ImpactedProductKey.ContentTypeId == Request.ProductKey.ContentTypeId))
			{
				OutTransaction.State = ECFProdTxnState::RecoveryRequired;
				OutTransaction.Diagnostic = TEXT("Durable impacted Product key type이 current Product type과 달라 recovery를 차단했습니다.");
				return false;
			}
			ImpactedPublishedProducts.AddUnique(ImpactedProductKey.ContentId);
		}
		ImpactedPublishedProducts.Sort();
	}
	else if (!BuildPublishedImpactClosure(
		Request,
		ChangedContentKeys,
		PublishedProductContentIds,
		ImpactedPublishedProducts,
		ValidationError))
	{
		OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = ValidationError;
		return false;
	}

	// Product/package/Catalog 중 하나라도 durable mutation이 시작됐는지 나타냅니다.
	bool bAnyDurableMutation = false;

	if (!ImpactedPublishedProducts.IsEmpty())
	{
		if (!bReuseDurableImpactScope)
		{
			OutTransaction.ImpactedProductKeys.Reset();
			for (const FString& ProductContentId : ImpactedPublishedProducts)
			{
				// Publication Catalog ContentId를 current Product type과 결합한 durable composite identity입니다.
				FCFContentKey ImpactedProductKey;
				ImpactedProductKey.ContentTypeId = Request.ProductKey.ContentTypeId;
				ImpactedProductKey.ContentId = ProductContentId;
				OutTransaction.ImpactedProductKeys.Add(MoveTemp(ImpactedProductKey));
			}
			OutTransaction.ImpactedProductKeys.Sort(
				[](const FCFContentKey& Left, const FCFContentKey& Right)
				{
					return Left.ToStableString() < Right.ToStableString();
				});

			// Catalog visibility mutation 전에 exact impacted Product scope를 먼저 durable journal에 고정합니다.
			if (!CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("Pre-withdraw impact scope journal save 실패")))
			{
				OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
				return false;
			}
		}

		// Closure orchestrator가 VisibilityWithdrawn을 이미 durable하게 확정한 recovery transaction은 같은 withdraw를 반복하지 않습니다.
		const bool bVisibilityAlreadyWithdrawn =
			bReuseDurableImpactScope
			&& OutTransaction.State != ECFProdTxnState::Prepared
			&& OutTransaction.State != ECFProdTxnState::BlockedBeforeMutation;
		if (!bVisibilityAlreadyWithdrawn)
		{
			if (!PublicationStore.WithdrawProducts(
				ImpactedPublishedProducts,
				PublicationError))
			{
				OutTransaction.State = ECFProdTxnState::BlockedBeforeMutation;
				OutTransaction.Diagnostic = PublicationError;
				return false;
			}

			OutTransaction.State = ECFProdTxnState::VisibilityWithdrawn;
			if (!CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("VisibilityWithdrawn journal save 실패")))
			{
				OutTransaction.State = ECFProdTxnState::RecoveryRequired;
				return false;
			}
		}

		bAnyDurableMutation = true;
	}

	for (const FCFProdProvisionTarget* Target : OrderedTargets)
	{
		if (Target == nullptr)
		{
			continue;
		}

		// Fresh per-target TOCTOU persisted existence입니다.
		bool bExists = false;
		// Fresh per-target TOCTOU persisted fingerprint입니다.
		FString CurrentFingerprint;
		// Fresh per-target TOCTOU read error입니다.
		FString ReadError;
		if (!TypedTargetAdapter.ReadPersistedFingerprint(
			*Target,
			bExists,
			CurrentFingerprint,
			ReadError))
		{
			OutTransaction.State = bAnyDurableMutation
				? ECFProdTxnState::RecoveryRequired
				: ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic = ReadError;
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("Target read failure journal save 실패"));
			return false;
		}

		// Same target durable evidence입니다.
		FCFProdTargetEvidence* Evidence =
			CFProdProvisioningPrivate::FindTargetEvidence(
				OutTransaction,
				Target->ContentKey);
		if (Evidence == nullptr)
		{
			OutTransaction.State = bAnyDurableMutation
				? ECFProdTxnState::RecoveryRequired
				: ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic = TEXT("Product target durable evidence가 누락됐습니다.");
			return false;
		}

		if (bExists
			&& CurrentFingerprint.Equals(
				Target->DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			Evidence->PersistedFingerprint = CurrentFingerprint;
			continue;
		}

		// Preflight 이후 absent-required target이 다른 persisted payload로 나타난 TOCTOU collision을 차단합니다.
		if (Target->bRequireAbsentAtReview && bExists)
		{
			OutTransaction.State = bAnyDurableMutation
				? ECFProdTxnState::RecoveryRequired
				: ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic =
				TEXT("Review 당시 absent-required target이 mutation 직전에 다른 persisted payload로 나타나 stale execution을 차단했습니다.");
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("CreateNew TOCTOU collision journal save 실패"));
			return false;
		}

		// Preflight 이후 target이 사라져도 BindShared consumer가 absent shared target을 생성하지 못하게 TOCTOU를 재차 차단합니다.
		if (!bExists && Target->BindingMode == ECFProdBindingMode::BindShared)
		{
			OutTransaction.State = bAnyDurableMutation
				? ECFProdTxnState::RecoveryRequired
				: ECFProdTxnState::BlockedBeforeMutation;
			OutTransaction.Diagnostic = TEXT("BindShared target이 mutation 직전에 absent 상태가 되어 생성 authority 없이 진행할 수 없습니다.");
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("BindShared TOCTOU journal save 실패"));
			return false;
		}

		if (bExistingTransactionFound
			&& (Evidence->bMutationPerformed
				|| !Evidence->PersistedFingerprint.IsEmpty()))
		{
			OutTransaction.State = ECFProdTxnState::RecoveryRequired;
			OutTransaction.Diagnostic = TEXT("Resume 중 previously confirmed target drift를 발견해 자동 mutation을 차단했습니다.");
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("RecoveryRequired journal save 실패"));
			return false;
		}

		// Existing typed backend가 반환한 post-write persisted fingerprint입니다.
		FString PersistedFingerprint;
		// Existing typed backend apply failure 상세입니다.
		FString ApplyError;
		if (!TypedTargetAdapter.ApplyReviewedTarget(
			*Target,
			PersistedFingerprint,
			ApplyError)
			|| !PersistedFingerprint.Equals(
				Target->DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			OutTransaction.State = ECFProdTxnState::RecoveryRequired;
			OutTransaction.Diagnostic = ApplyError.IsEmpty()
				? TEXT("Typed Production target durable apply/readback이 desired fingerprint와 일치하지 않습니다.")
				: ApplyError;
			CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				OutTransaction,
				TEXT("RecoveryRequired journal save 실패"));
			return false;
		}

		bAnyDurableMutation = true;
		Evidence->PersistedFingerprint = PersistedFingerprint;
		Evidence->bMutationPerformed = true;

		if (!CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Per-target durable progress journal save 실패")))
		{
			OutTransaction.State = ECFProdTxnState::RecoveryRequired;
			return false;
		}
	}

	OutTransaction.State = ECFProdTxnState::ChildAssetsPersisted;
	if (!CFProdProvisioningPrivate::SaveTransactionState(
		TransactionStore,
		OutTransaction,
		TEXT("ChildAssetsPersisted journal save 실패")))
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		return false;
	}

	if (!BuildProductGraphFingerprint(
		Request,
		OutTransaction.Targets,
		OutTransaction.ProductGraphFingerprint,
		ValidationError))
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = ValidationError;
		CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Graph failure journal save 실패"));
		return false;
	}

	OutTransaction.State = ECFProdTxnState::ProductGraphVerified;
	if (!CFProdProvisioningPrivate::SaveTransactionState(
		TransactionStore,
		OutTransaction,
		TEXT("ProductGraphVerified journal save 실패")))
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		return false;
	}

	// Persisted canonical Product EquipmentPresetData입니다.
	UCFEquipmentPresetData* EquipmentPresetData =
		LoadObject<UCFEquipmentPresetData>(
			nullptr,
			*Request.EquipmentPresetObjectPath);
	if (!IsValid(EquipmentPresetData)
		|| !EquipmentPresetData->HasCompleteEquipmentData())
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = TEXT("Persisted Product EquipmentPresetData graph가 complete하지 않습니다.");
		CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("EquipmentPreset graph failure journal save 실패"));
		return false;
	}

	// Runtime proof에 전달할 merged explicit sortie ammo loads입니다.
	TArray<FCFAmmoSortieLoad> RuntimeAmmoLoads;
	if (!BuildRuntimeAmmoLoads(
		Request,
		*EquipmentPresetData,
		VehiclePawn,
		RuntimeAmmoLoads,
		ValidationError))
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = ValidationError;
		CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Ammo projection failure journal save 실패"));
		return false;
	}

	// Direct Runtime technical proof failure 상세입니다.
	FString RuntimeError;
	if (!RuntimeVerifier.VerifyRuntime(
		VehiclePawn,
		TargetMountProfileId,
		EquipmentPresetData,
		RuntimeAmmoLoads,
		RuntimeError))
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		OutTransaction.Diagnostic = RuntimeError;
		CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Runtime verification failure journal save 실패"));
		return false;
	}

	OutTransaction.State = ECFProdTxnState::RuntimeVerified;
	if (!CFProdProvisioningPrivate::SaveTransactionState(
		TransactionStore,
		OutTransaction,
		TEXT("RuntimeVerified journal save 실패")))
	{
		OutTransaction.State = bAnyDurableMutation
			? ECFProdTxnState::RecoveryRequired
			: ECFProdTxnState::BlockedBeforeMutation;
		return false;
	}

	// Generated Production Publication Catalog entry입니다.
	FCFProdEquipCatalogEntry PublishEntry;
	PublishEntry.ContentId = Request.ProductKey.ContentId;
	PublishEntry.EquipmentPresetData = EquipmentPresetData;
	PublishEntry.DefaultSortieAmmoLoads = RuntimeAmmoLoads;
	PublishEntry.ProductGraphFingerprint = OutTransaction.ProductGraphFingerprint;

	if (!PublicationStore.PublishProduct(
		PublishEntry,
		PublicationError))
	{
		OutTransaction.State = ECFProdTxnState::RecoveryRequired;
		OutTransaction.Diagnostic = PublicationError;
		CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Publication failure journal save 실패"));
		return false;
	}
	bAnyDurableMutation = true;

	OutTransaction.State = ECFProdTxnState::Published;
	if (!CFProdProvisioningPrivate::SaveTransactionState(
		TransactionStore,
		OutTransaction,
		TEXT("Published journal save 실패")))
	{
		OutTransaction.State = ECFProdTxnState::RecoveryRequired;
		return false;
	}

	// Fresh persisted Publication Catalog readback entry입니다.
	FCFProdEquipCatalogEntry ReadbackEntry;
	if (!PublicationStore.ReadPublishedEntry(
		Request.ProductKey.ContentId,
		ReadbackEntry,
		PublicationError)
		|| !ReadbackEntry.ContentId.Equals(
			PublishEntry.ContentId,
			ESearchCase::CaseSensitive)
		|| !GetPathNameSafe(ReadbackEntry.EquipmentPresetData).Equals(
			GetPathNameSafe(PublishEntry.EquipmentPresetData),
			ESearchCase::CaseSensitive)
		|| !ReadbackEntry.ProductGraphFingerprint.Equals(
			PublishEntry.ProductGraphFingerprint,
			ESearchCase::CaseSensitive)
		|| !CFProdProvisioningPrivate::AreAmmoLoadsEquivalent(
			ReadbackEntry.DefaultSortieAmmoLoads,
			PublishEntry.DefaultSortieAmmoLoads))
	{
		OutTransaction.State = ECFProdTxnState::RecoveryRequired;
		OutTransaction.Diagnostic = PublicationError.IsEmpty()
			? TEXT("Publication Catalog fresh readback이 canonical Product publish payload와 다릅니다.")
			: PublicationError;
		CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			OutTransaction,
			TEXT("Publication readback failure journal save 실패"));
		return false;
	}

	OutTransaction.State = ECFProdTxnState::VehicleReady;
	OutTransaction.Diagnostic.Reset();
	if (!CFProdProvisioningPrivate::SaveTransactionState(
		TransactionStore,
		OutTransaction,
		TEXT("VehicleReady journal save 실패")))
	{
		OutTransaction.State = ECFProdTxnState::RecoveryRequired;
		return false;
	}

	return true;
}


// Shared single-writer topology를 mutation0 검증하고 impacted published closure 전체를 deterministic하게 재실행합니다.
bool FCFProdProvisioning::ExecuteProductClosure(
	const TArray<FCFProdProductExecution>& ProductExecutions,
	ICFProdTypedTargetAdapter& TypedTargetAdapter,
	ICFProdTransactionStore& TransactionStore,
	ICFProdPublicationStore& PublicationStore,
	ICFProdRuntimeVerifier& RuntimeVerifier,
	FCFProdClosureExecutionReport& OutReport)
{
	OutReport = FCFProdClosureExecutionReport();

	if (ProductExecutions.IsEmpty())
	{
		OutReport.Diagnostic = TEXT("Production closure execution에는 Product exact1 이상이 필요합니다.");
		return false;
	}

	// Closure 안에서 허용할 Product ContentType입니다.
	const FCFContentTypeId ProductContentType =
		ProductExecutions[0].Request.ProductKey.ContentTypeId;
	if (!ProductContentType.IsValid())
	{
		OutReport.Diagnostic = TEXT("Production closure Product ContentTypeId가 유효하지 않습니다.");
		return false;
	}

	// 한 target의 Product별 binding occurrence입니다.
	struct FTargetOccurrence
	{
		// Product execution array index입니다.
		int32 ExecutionIndex = INDEX_NONE;

		// 해당 Product request 안의 exact target descriptor입니다.
		const FCFProdProvisionTarget* Target = nullptr;
	};

	// Product composite identity 중복 검사용 set입니다.
	TSet<FString> SeenProductKeys;
	// Product transaction identity 중복 검사용 set입니다.
	TSet<FString> SeenTransactionIds;
	// Canonical target ContentKey -> Product별 occurrence 집합입니다.
	TMap<FString, TArray<FTargetOccurrence>> TargetOccurrencesByKey;
	// Union dependency edge set을 만들기 위한 stable edge identity set입니다.
	TSet<FString> SeenDependencyEdges;
	// 모든 Product request가 공유할 closure-wide dependency graph입니다.
	TArray<FCFContentDependencyEdge> CombinedDependencyEdges;

	for (int32 ExecutionIndex = 0; ExecutionIndex < ProductExecutions.Num(); ++ExecutionIndex)
	{
		// Current Product execution입니다.
		const FCFProdProductExecution& Execution =
			ProductExecutions[ExecutionIndex];
		// Current request validation failure 상세입니다.
		FString ValidationError;
		if (!CFProdProvisioningPrivate::ValidateRequest(
			Execution.Request,
			ValidationError))
		{
			OutReport.Diagnostic = ValidationError;
			return false;
		}

		if (!(Execution.Request.ProductKey.ContentTypeId == ProductContentType))
		{
			OutReport.Diagnostic = TEXT("한 Production closure 안의 Product는 동일 ContentTypeId를 사용해야 합니다.");
			return false;
		}

		// Current Product stable composite key입니다.
		const FString ProductStableKey =
			Execution.Request.ProductKey.ToStableString();
		if (SeenProductKeys.Contains(ProductStableKey))
		{
			OutReport.Diagnostic = TEXT("Production closure에 duplicate Product ContentKey가 있습니다: ")
				+ ProductStableKey;
			return false;
		}
		SeenProductKeys.Add(ProductStableKey);

		if (SeenTransactionIds.Contains(Execution.Request.TransactionId))
		{
			OutReport.Diagnostic = TEXT("Production closure에 duplicate TransactionId가 있습니다: ")
				+ Execution.Request.TransactionId;
			return false;
		}
		SeenTransactionIds.Add(Execution.Request.TransactionId);

		for (const FCFProdProvisionTarget& Target : Execution.Request.Targets)
		{
			// Current target occurrence입니다.
			FTargetOccurrence Occurrence;
			Occurrence.ExecutionIndex = ExecutionIndex;
			Occurrence.Target = &Target;
			TargetOccurrencesByKey.FindOrAdd(
				Target.ContentKey.ToStableString()).Add(Occurrence);
		}

		for (const FCFContentDependencyEdge& Edge : Execution.Request.DependencyEdges)
		{
			if (!Edge.From.IsValid() || !Edge.To.IsValid())
			{
				OutReport.Diagnostic = TEXT("Production closure dependency edge endpoint가 유효하지 않습니다.");
				return false;
			}

			// Dependency edge duplicate 제거용 stable identity입니다.
			const FString EdgeStableKey =
				Edge.From.ToStableString()
				+ TEXT("->")
				+ Edge.To.ToStableString()
				+ TEXT("|")
				+ Edge.FieldPath;
			if (!SeenDependencyEdges.Contains(EdgeStableKey))
			{
				SeenDependencyEdges.Add(EdgeStableKey);
				CombinedDependencyEdges.Add(Edge);
			}
		}
	}

	CombinedDependencyEdges.Sort(
		[](const FCFContentDependencyEdge& Left, const FCFContentDependencyEdge& Right)
		{
			// Left stable edge identity입니다.
			const FString LeftKey =
				Left.From.ToStableString()
					+ TEXT("->")
					+ Left.To.ToStableString()
					+ TEXT("|")
					+ Left.FieldPath;
			// Right stable edge identity입니다.
			const FString RightKey =
				Right.From.ToStableString()
					+ TEXT("->")
					+ Right.To.ToStableString()
					+ TEXT("|")
					+ Right.FieldPath;
			return LeftKey < RightKey;
		});

	// Product execution dependency adjacency입니다. Absent shared target creator가 consumer보다 먼저 실행되도록 사용합니다.
	TArray<TArray<int32>> DependentExecutionIndices;
	DependentExecutionIndices.SetNum(ProductExecutions.Num());
	// Product execution별 incoming creator dependency 수입니다.
	TArray<int32> IncomingDependencyCounts;
	IncomingDependencyCounts.Init(0, ProductExecutions.Num());
	// Duplicate execution dependency edge를 차단할 set입니다.
	TSet<FString> SeenExecutionDependencies;
	// Current desired state와 다른 unique target ContentKeys입니다.
	TArray<FCFContentKey> ChangedContentKeys;
	// Changed target 중복 제거용 set입니다.
	TSet<FString> ChangedContentKeySet;

	for (const TPair<FString, TArray<FTargetOccurrence>>& Pair : TargetOccurrencesByKey)
	{
		const TArray<FTargetOccurrence>& Occurrences = Pair.Value;
		if (Occurrences.IsEmpty()
			|| Occurrences[0].Target == nullptr)
		{
			OutReport.Diagnostic = TEXT("Production closure target occurrence가 비어 있습니다.");
			return false;
		}

		// Canonical authored target descriptor 기준입니다.
		const FCFProdProvisionTarget& CanonicalTarget =
			*Occurrences[0].Target;
		// Shared creator binding count입니다.
		int32 SharedCreatorCount = 0;
		// Shared consumer binding count입니다.
		int32 SharedConsumerCount = 0;
		// Absent shared target creator Product execution index입니다.
		int32 SharedCreatorExecutionIndex = INDEX_NONE;

		for (const FTargetOccurrence& Occurrence : Occurrences)
		{
			if (Occurrence.Target == nullptr
				|| Occurrence.ExecutionIndex < 0
				|| Occurrence.ExecutionIndex >= ProductExecutions.Num())
			{
				OutReport.Diagnostic = TEXT("Production closure target occurrence identity가 유효하지 않습니다.");
				return false;
			}

			if (!Occurrence.Target->TargetObjectPath.Equals(
					CanonicalTarget.TargetObjectPath,
					ESearchCase::CaseSensitive)
				|| !Occurrence.Target->DesiredFingerprint.Equals(
					CanonicalTarget.DesiredFingerprint,
					ESearchCase::CaseSensitive)
				|| Occurrence.Target->bShared != CanonicalTarget.bShared)
			{
				OutReport.Diagnostic =
					TEXT("같은 shared/content target의 object path / desired fingerprint / shared 의미가 Product마다 갈라졌습니다: ")
					+ CanonicalTarget.ContentKey.ToStableString();
				return false;
			}

			switch (Occurrence.Target->BindingMode)
			{
			case ECFProdBindingMode::CreateNewShared:
				++SharedCreatorCount;
				SharedCreatorExecutionIndex = Occurrence.ExecutionIndex;
				break;

			case ECFProdBindingMode::BindShared:
				++SharedConsumerCount;
				break;

			case ECFProdBindingMode::CreateNew:
				if (CanonicalTarget.bShared)
				{
					OutReport.Diagnostic = TEXT("Shared target group 안에 CreateNew variant-owned binding이 섞였습니다.");
					return false;
				}
				break;

			default:
				OutReport.Diagnostic = TEXT("Production target BindingMode enum이 유효하지 않습니다.");
				return false;
			}
		}

		if (!CanonicalTarget.bShared && Occurrences.Num() != 1)
		{
			OutReport.Diagnostic =
				TEXT("Variant-owned target ContentKey가 여러 Product에 중복됐습니다. Shared target은 명시적 shared binding을 사용해야 합니다: ")
				+ CanonicalTarget.ContentKey.ToStableString();
			return false;
		}

		if (CanonicalTarget.bShared
			&& (SharedCreatorCount > 1 || SharedConsumerCount < 1))
		{
			OutReport.Diagnostic =
				TEXT("Shared target은 CreateNewShared creator 0..1 및 BindShared consumer 1..N 계약을 만족해야 합니다: ")
				+ CanonicalTarget.ContentKey.ToStableString();
			return false;
		}

		// Canonical shared/variant target persisted 존재 여부입니다.
		bool bTargetExists = false;
		// Canonical target fresh persisted fingerprint입니다.
		FString CurrentFingerprint;
		// Canonical target read failure 상세입니다.
		FString ReadError;
		if (!TypedTargetAdapter.ReadPersistedFingerprint(
			CanonicalTarget,
			bTargetExists,
			CurrentFingerprint,
			ReadError))
		{
			OutReport.Diagnostic = ReadError.IsEmpty()
				? TEXT("Production closure target persisted state read가 실패했습니다.")
				: ReadError;
			return false;
		}

		if (bTargetExists
			&& !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				CurrentFingerprint))
		{
			OutReport.Diagnostic =
				TEXT("Existing Production target persisted fingerprint가 canonical SHA-256이 아닙니다: ")
				+ CanonicalTarget.ContentKey.ToStableString();
			return false;
		}

		if (CanonicalTarget.bShared && !bTargetExists)
		{
			if (SharedCreatorCount != 1
				|| SharedCreatorExecutionIndex == INDEX_NONE)
			{
				OutReport.Diagnostic =
					TEXT("Absent shared target은 CreateNewShared creator exact1이 필요합니다: ")
					+ CanonicalTarget.ContentKey.ToStableString();
				return false;
			}

			for (const FTargetOccurrence& Occurrence : Occurrences)
			{
				if (Occurrence.Target == nullptr
					|| Occurrence.Target->BindingMode != ECFProdBindingMode::BindShared)
				{
					continue;
				}

				// Creator→consumer execution dependency stable key입니다.
				const FString ExecutionDependencyKey =
					LexToString(SharedCreatorExecutionIndex)
						+ TEXT("->")
						+ LexToString(Occurrence.ExecutionIndex);
				if (Occurrence.ExecutionIndex != SharedCreatorExecutionIndex
					&& !SeenExecutionDependencies.Contains(
						ExecutionDependencyKey))
				{
					SeenExecutionDependencies.Add(ExecutionDependencyKey);
					DependentExecutionIndices[SharedCreatorExecutionIndex].Add(
						Occurrence.ExecutionIndex);
					++IncomingDependencyCounts[Occurrence.ExecutionIndex];
				}
			}
		}

		if (!bTargetExists
			|| !CurrentFingerprint.Equals(
				CanonicalTarget.DesiredFingerprint,
				ESearchCase::CaseSensitive))
		{
			const FString ChangedKey =
				CanonicalTarget.ContentKey.ToStableString();
			if (!ChangedContentKeySet.Contains(ChangedKey))
			{
				ChangedContentKeySet.Add(ChangedKey);
				ChangedContentKeys.Add(CanonicalTarget.ContentKey);
			}
		}
	}

	ChangedContentKeys.Sort(
		[](const FCFContentKey& Left, const FCFContentKey& Right)
		{
			return Left.ToStableString() < Right.ToStableString();
		});

	// Current publication membership before any Product/child mutation입니다.
	TArray<FString> PublishedProductContentIds;
	// Publication membership read error입니다.
	FString PublicationError;
	if (!PublicationStore.ReadPublishedProductIds(
		PublishedProductContentIds,
		PublicationError))
	{
		OutReport.Diagnostic = PublicationError;
		return false;
	}

	// Closure-wide dependency graph을 가진 synthetic impact request입니다.
	FCFProdProvisionRequest ImpactRequest =
		ProductExecutions[0].Request;
	ImpactRequest.DependencyEdges = CombinedDependencyEdges;

	// All changed targets로부터 계산한 currently published reverse consumer closure입니다.
	TArray<FString> ImpactedPublishedProductIds;
	// Closure impact calculation failure 상세입니다.
	FString ImpactError;
	if (!BuildPublishedImpactClosure(
		ImpactRequest,
		ChangedContentKeys,
		PublishedProductContentIds,
		ImpactedPublishedProductIds,
		ImpactError))
	{
		OutReport.Diagnostic = ImpactError;
		return false;
	}

	// Interrupted closure recovery를 위해 non-terminal Product transaction의 기존 durable impact scope도 합칩니다.
	for (const FCFProdProductExecution& Execution : ProductExecutions)
	{
		// Existing transaction 존재 여부입니다.
		bool bExistingTransactionFound = false;
		// Existing transaction snapshot입니다.
		FCFProdTransaction ExistingTransaction;
		// Existing transaction load failure 상세입니다.
		FString TransactionLoadError;
		if (!TransactionStore.LoadTransaction(
			Execution.Request.TransactionId,
			bExistingTransactionFound,
			ExistingTransaction,
			TransactionLoadError))
		{
			OutReport.Diagnostic = TransactionLoadError;
			return false;
		}

		if (!bExistingTransactionFound
			|| ExistingTransaction.State == ECFProdTxnState::VehicleReady)
		{
			continue;
		}

		// Existing recovery transaction identity가 current request와 exact 일치해야 합니다.
		FString ResumeValidationError;
		if (!CFProdProvisioningPrivate::ValidateResumeIdentity(
			Execution.Request,
			ExistingTransaction,
			ResumeValidationError))
		{
			OutReport.Diagnostic = ResumeValidationError;
			return false;
		}

		for (const FCFContentKey& ImpactedProductKey : ExistingTransaction.ImpactedProductKeys)
		{
			if (!(ImpactedProductKey.ContentTypeId == ProductContentType))
			{
				OutReport.Diagnostic = TEXT("Recovery transaction의 impacted Product type이 closure Product type과 다릅니다.");
				return false;
			}
			ImpactedPublishedProductIds.AddUnique(ImpactedProductKey.ContentId);
		}
	}
	ImpactedPublishedProductIds.Sort();

	// Impact closure의 exact execution request 존재 여부를 mutation 전에 확인합니다.
	for (const FString& ImpactedProductContentId : ImpactedPublishedProductIds)
	{
		const bool bExecutionProvided = ProductExecutions.ContainsByPredicate(
			[&ProductContentType, &ImpactedProductContentId](
				const FCFProdProductExecution& Execution)
			{
				return Execution.Request.ProductKey.ContentTypeId
						== ProductContentType
					&& Execution.Request.ProductKey.ContentId.Equals(
						ImpactedProductContentId,
						ESearchCase::CaseSensitive);
			});
		if (!bExecutionProvided)
		{
			OutReport.Diagnostic =
				TEXT("Impacted published Product closure 전체를 재검증할 execution request가 제공되지 않았습니다: ")
				+ ImpactedProductContentId;
			return false;
		}
	}

	// Durable journal에 기록할 composite impacted Product key 집합입니다.
	TArray<FCFContentKey> DurableImpactedProductKeys;
	for (const FString& ImpactedProductContentId : ImpactedPublishedProductIds)
	{
		// Publication ContentId를 closure Product type과 결합한 exact composite identity입니다.
		FCFContentKey ImpactedProductKey;
		ImpactedProductKey.ContentTypeId = ProductContentType;
		ImpactedProductKey.ContentId = ImpactedProductContentId;
		DurableImpactedProductKeys.Add(MoveTemp(ImpactedProductKey));
	}

	// ProductContentId -> pre-withdraw transaction snapshot입니다.
	TMap<FString, FCFProdTransaction> ImpactTransactionsByProductId;
	for (const FCFContentKey& ImpactedProductKey : DurableImpactedProductKeys)
	{
		// Impacted Product execution descriptor입니다.
		const FCFProdProductExecution* ImpactedExecution =
			ProductExecutions.FindByPredicate(
				[&ImpactedProductKey](const FCFProdProductExecution& Execution)
				{
					return Execution.Request.ProductKey == ImpactedProductKey;
				});
		if (ImpactedExecution == nullptr)
		{
			OutReport.Diagnostic = TEXT("Impacted Product execution lookup이 mutation 전에 실패했습니다.");
			return false;
		}

		// Existing transaction 존재 여부입니다.
		bool bExistingTransactionFound = false;
		// Pre-withdraw transaction snapshot입니다.
		FCFProdTransaction PreparedTransaction;
		// Transaction load failure 상세입니다.
		FString TransactionLoadError;
		if (!TransactionStore.LoadTransaction(
			ImpactedExecution->Request.TransactionId,
			bExistingTransactionFound,
			PreparedTransaction,
			TransactionLoadError))
		{
			OutReport.Diagnostic = TransactionLoadError;
			return false;
		}

		if (bExistingTransactionFound)
		{
			// Completed transaction identity를 새 mutation closure에 재사용하지 않습니다.
			if (PreparedTransaction.State == ECFProdTxnState::VehicleReady)
			{
				OutReport.Diagnostic =
					TEXT("이미 VehicleReady로 완료된 TransactionId를 새 impacted closure mutation에 재사용할 수 없습니다: ")
					+ ImpactedExecution->Request.TransactionId;
				return false;
			}

			// Existing transaction이 current request와 exact 동일한지 확인합니다.
			FString ResumeValidationError;
			if (!CFProdProvisioningPrivate::ValidateResumeIdentity(
				ImpactedExecution->Request,
				PreparedTransaction,
				ResumeValidationError))
			{
				OutReport.Diagnostic = ResumeValidationError;
				return false;
			}
		}
		else
		{
			CFProdProvisioningPrivate::InitializePreparedTransaction(
				ImpactedExecution->Request,
				PreparedTransaction);
		}

		PreparedTransaction.ImpactedProductKeys = DurableImpactedProductKeys;
		if (!CFProdProvisioningPrivate::SaveTransactionState(
			TransactionStore,
			PreparedTransaction,
			TEXT("Closure pre-withdraw impact journal save 실패")))
		{
			OutReport.Diagnostic = PreparedTransaction.Diagnostic;
			return false;
		}

		ImpactTransactionsByProductId.Add(
			ImpactedProductKey.ContentId,
			MoveTemp(PreparedTransaction));
	}

	// 모든 impacted Product visibility를 어떤 child/Product mutation보다 먼저 한 번에 durable withdraw합니다.
	if (!ImpactedPublishedProductIds.IsEmpty())
	{
		if (!PublicationStore.WithdrawProducts(
			ImpactedPublishedProductIds,
			PublicationError))
		{
			OutReport.Diagnostic = PublicationError;
			return false;
		}

		for (TPair<FString, FCFProdTransaction>& Pair : ImpactTransactionsByProductId)
		{
			// RecoveryRequired 등 더 진행된 state는 보존하고 fresh Prepared만 VisibilityWithdrawn으로 전진시킵니다.
			if (Pair.Value.State == ECFProdTxnState::Prepared
				|| Pair.Value.State == ECFProdTxnState::BlockedBeforeMutation)
			{
				Pair.Value.State = ECFProdTxnState::VisibilityWithdrawn;
			}
			if (!CFProdProvisioningPrivate::SaveTransactionState(
				TransactionStore,
				Pair.Value,
				TEXT("Closure VisibilityWithdrawn journal save 실패")))
			{
				OutReport.Diagnostic = Pair.Value.Diagnostic;
				return false;
			}
		}
	}

	// Topological/deterministic execution order입니다.
	TArray<int32> ExecutionOrder;
	// 이미 order에 선택된 execution index 집합입니다.
	TSet<int32> SelectedExecutionIndices;

	while (ExecutionOrder.Num() < ProductExecutions.Num())
	{
		// 이번 iteration에서 선택할 zero-indegree execution index입니다.
		int32 SelectedIndex = INDEX_NONE;
		// Selected candidate의 stable Product key입니다.
		FString SelectedProductKey;

		for (int32 ExecutionIndex = 0; ExecutionIndex < ProductExecutions.Num(); ++ExecutionIndex)
		{
			if (SelectedExecutionIndices.Contains(ExecutionIndex)
				|| IncomingDependencyCounts[ExecutionIndex] != 0)
			{
				continue;
			}

			// Candidate stable Product identity입니다.
			const FString CandidateProductKey =
				ProductExecutions[ExecutionIndex].Request.ProductKey.ToStableString();
			if (SelectedIndex == INDEX_NONE
				|| CandidateProductKey < SelectedProductKey)
			{
				SelectedIndex = ExecutionIndex;
				SelectedProductKey = CandidateProductKey;
			}
		}

		if (SelectedIndex == INDEX_NONE)
		{
			OutReport.Diagnostic =
				TEXT("Absent shared target creator/consumer Product execution dependency에 cycle이 있습니다.");
			return false;
		}

		SelectedExecutionIndices.Add(SelectedIndex);
		ExecutionOrder.Add(SelectedIndex);
		for (const int32 DependentIndex :
			DependentExecutionIndices[SelectedIndex])
		{
			if (DependentIndex >= 0
				&& DependentIndex < IncomingDependencyCounts.Num())
			{
				--IncomingDependencyCounts[DependentIndex];
			}
		}
	}

	// Closure 전체 Product 중 하나라도 terminal VehicleReady가 아니었는지 여부입니다.
	bool bAllVehicleReady = true;

	for (const int32 ExecutionIndex : ExecutionOrder)
	{
		// Closure-wide dependency graph을 사용하는 exact Product execution copy입니다.
		FCFProdProductExecution Execution =
			ProductExecutions[ExecutionIndex];
		Execution.Request.DependencyEdges =
			CombinedDependencyEdges;

		// Current Product terminal transaction입니다.
		FCFProdTransaction ProductTransaction;
		const bool bProductSucceeded = ExecuteProductTransaction(
			Execution.Request,
			TypedTargetAdapter,
			TransactionStore,
			PublicationStore,
			RuntimeVerifier,
			Execution.VehiclePawn,
			Execution.TargetMountProfileId,
			ProductTransaction);
		OutReport.Transactions.Add(ProductTransaction);

		if (bProductSucceeded
			&& ProductTransaction.State == ECFProdTxnState::VehicleReady)
		{
			OutReport.VehicleReadyProductKeys.Add(
				Execution.Request.ProductKey);
		}
		else
		{
			bAllVehicleReady = false;
			OutReport.RecoveryRequiredProductKeys.Add(
				Execution.Request.ProductKey);
		}
	}

	OutReport.VehicleReadyProductKeys.Sort(
		[](const FCFContentKey& Left, const FCFContentKey& Right)
		{
			return Left.ToStableString() < Right.ToStableString();
		});
	OutReport.RecoveryRequiredProductKeys.Sort(
		[](const FCFContentKey& Left, const FCFContentKey& Right)
		{
			return Left.ToStableString() < Right.ToStableString();
		});

	if (!bAllVehicleReady)
	{
		OutReport.Diagnostic = FString::Printf(
			TEXT("Production closure partial failure: VehicleReady=%d, RecoveryRequired=%d"),
			OutReport.VehicleReadyProductKeys.Num(),
			OutReport.RecoveryRequiredProductKeys.Num());
		return false;
	}

	OutReport.Diagnostic.Reset();
	return true;
}
