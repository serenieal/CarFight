// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdProvisioningTests.cpp
// Version: v1.3.0
// Date: 2026-10-06
// Description: CF-FQ-058 CCAS-P0-07 Production Provisioning/Recovery/Publication Bridge focused Automation입니다.
// Changelog:
// - v1.3.0: transaction v3 approval/execution identity와 absent-required CreateNew collision fail-closed 회귀를 추가.
// - v1.2.0: concrete Production target binding이 existing Weapon Guide/Equipment backend fingerprint와 exact 결속되는 회귀를 추가.
// - v1.1.0: F.1 shared single-writer + F.2 multi-Product 선철회/재검증 closure 회귀를 추가.
// - v1.0.2: Production transaction v2의 target object path / impacted composite ProductKey durable binding, file-store roundtrip, path-drift resume 차단 회귀 추가.
// - v1.0.1: 다른 ContentType child가 published Product와 같은 ContentId를 가져도 impact closure가 과잉 withdraw하지 않는 identity 회귀 추가.
// - v1.0.0: ammo schema projection, impact closure/graph fingerprint, partial recovery resume,
//   explicit sortie ammo projection exact4 regression을 최초 추가.
// Migration:
// - 대부분 fixture는 in-memory/fake persistence boundary를 사용하며 Production binding 회귀는 existing backend를 read-only로 호출합니다.
// - Production Workbook, Production DataAsset, canonical publication catalog를 생성·수정하지 않습니다.

#include "DataAuthoring/CFProdProvisioning.h"
#include "DataAuthoring/CFProdTargetBindings.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "EquipmentAuthoring/CFEquipmentDurable.h"
#include "WeaponAuthoring/CFWeaponGuideBackend.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFProdProvisioningTestsPrivate
{
	// Fixture text를 canonical SHA-256 fingerprint로 변환합니다.
	FString HashText(const FString& Text)
	{
		// UTF-8 hash source입니다.
		FTCHARToUTF8 Utf8(*Text);

		// Canonical byte buffer입니다.
		TArray<uint8> Bytes;
		Bytes.Append(
			reinterpret_cast<const uint8*>(Utf8.Get()),
			Utf8.Length());

		// Generated canonical fingerprint입니다.
		FString Fingerprint;

		// Hash diagnostic입니다.
		FString Error;
		return CFDACommonPrimitives::HashCanonicalBytes(
			Bytes,
			Fingerprint,
			Error)
				? Fingerprint
				: FString();
	}

	// Stable ContentKey fixture를 생성합니다.
	FCFContentKey MakeKey(
		const FString& ContentTypeId,
		const FString& ContentId)
	{
		// Generated fixture key입니다.
		FCFContentKey Key;
		Key.ContentTypeId.Value = ContentTypeId;
		Key.ContentId = ContentId;
		return Key;
	}

	// Execution test request에 canonical synthetic USER-review identity exact4를 결속합니다.
	void BindSyntheticApprovalIdentity(
		FCFProdProvisionRequest& Request,
		const FString& Seed)
	{
		Request.ReviewPackageFingerprint =
			HashText(Seed + TEXT("-review"));
		Request.BaseCatalogSnapshotFingerprint =
			HashText(Seed + TEXT("-base"));
		Request.ResourceCatalogFingerprint =
			HashText(Seed + TEXT("-resources"));
		Request.ExecutionBindingFingerprint =
			HashText(Seed + TEXT("-execution"));
	}

	// Canonical NameId VALUE fixture를 생성합니다.
	FCFContentValue MakeNameValue(const FString& ValueText)
	{
		// Generated canonical value입니다.
		FCFContentValue Value;
		Value.Type = ECFContentValueType::NameId;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = ValueText;
		return Value;
	}

	// Canonical signed integer VALUE fixture를 생성합니다.
	FCFContentValue MakeIntValue(const int64 Number)
	{
		// Generated canonical value입니다.
		FCFContentValue Value;
		Value.Type = ECFContentValueType::SignedInteger;
		Value.State = ECFContentValueState::Value;
		Value.SignedIntegerValue = Number;
		return Value;
	}

	// Product transaction fixture가 사용할 complete infinite-ammo EquipmentPreset을 in-memory /Game package에 생성합니다.
	UCFEquipmentPresetData* MakeInfiniteEquipmentPreset(
		const FString& AssetName)
	{
		// Disposable in-memory package name입니다.
		const FString PackageName =
			TEXT("/Game/CarFight/Tests/CCASP007/") + AssetName;

		// Disposable in-memory package입니다.
		UPackage* Package = CreatePackage(*PackageName);
		if (Package == nullptr)
		{
			return nullptr;
		}

		// Complete weapon package용 TurretMountData입니다.
		UCFTurretMountData* TurretMountData =
			NewObject<UCFTurretMountData>(
				Package,
				*(AssetName + TEXT("_Mount")),
				RF_Transient);

		// Infinite-compatibility WeaponData입니다.
		UCFWeaponData* WeaponData =
			NewObject<UCFWeaponData>(
				Package,
				*(AssetName + TEXT("_Weapon")),
				RF_Transient);

		// Product-level EquipmentPreset fixture입니다.
		UCFEquipmentPresetData* EquipmentPresetData =
			NewObject<UCFEquipmentPresetData>(
				Package,
				*AssetName,
				RF_Public | RF_Standalone);
		if (EquipmentPresetData == nullptr)
		{
			return nullptr;
		}

		EquipmentPresetData->EquipmentId = FName(*AssetName);
		EquipmentPresetData->DefaultTurretMountData = TurretMountData;
		EquipmentPresetData->DefaultWeaponData = WeaponData;
		EquipmentPresetData->DefaultSensorData = nullptr;
		return EquipmentPresetData;
	}

	// Product transaction fixture가 사용할 in-memory transaction store입니다.
	class FMemoryTransactionStore final : public ICFProdTransactionStore
	{
	public:
		// TransactionId -> latest durable-style snapshot입니다.
		TMap<FString, FCFProdTransaction> Transactions;

		// Save 호출 수입니다.
		int32 SaveCount = 0;

		// Exact transaction snapshot을 memory map에 저장합니다.
		virtual bool SaveTransaction(
			const FCFProdTransaction& Transaction,
			FString& OutError) override
		{
			OutError.Reset();
			Transactions.Add(Transaction.TransactionId, Transaction);
			++SaveCount;
			return true;
		}

		// Exact transaction snapshot을 memory map에서 읽습니다.
		virtual bool LoadTransaction(
			const FString& TransactionId,
			bool& bOutFound,
			FCFProdTransaction& OutTransaction,
			FString& OutError) const override
		{
			OutError.Reset();

			// Existing transaction snapshot입니다.
			const FCFProdTransaction* Existing = Transactions.Find(TransactionId);
			bOutFound = Existing != nullptr;
			OutTransaction = Existing != nullptr
				? *Existing
				: FCFProdTransaction();
			return true;
		}
	};

	// Existing typed durable backend를 흉내내는 Product target adapter fixture입니다.
	class FMemoryTypedAdapter final : public ICFProdTypedTargetAdapter
	{
	public:
		// Stable ContentKey -> persisted fingerprint state입니다.
		TMap<FString, FString> PersistedFingerprints;

		// Stable ContentId -> successful mutation count입니다.
		TMap<FString, int32> SuccessfulApplyCount;

		// 이 ContentId Apply에서 deterministic failure를 강제합니다.
		FString FailContentId;

		// Withdrawal/apply ordering을 확인할 shared event log입니다.
		TArray<FString>* EventLog = nullptr;

		// Current in-memory persisted fingerprint를 mutation 없이 읽습니다.
		virtual bool ReadPersistedFingerprint(
			const FCFProdProvisionTarget& Target,
			bool& bOutExists,
			FString& OutFingerprint,
			FString& OutError) const override
		{
			OutError.Reset();

			// Current target fingerprint입니다.
			const FString* Persisted =
				PersistedFingerprints.Find(Target.ContentKey.ToStableString());
			bOutExists = Persisted != nullptr;
			OutFingerprint = Persisted != nullptr ? *Persisted : FString();
			return true;
		}

		// Existing typed durable writer success/failure를 deterministic하게 모사합니다.
		virtual bool ApplyReviewedTarget(
			const FCFProdProvisionTarget& Target,
			FString& OutPersistedFingerprint,
			FString& OutError) override
		{
			if (EventLog != nullptr)
			{
				EventLog->Add(TEXT("apply:") + Target.ContentKey.ContentId);
			}

			if (Target.ContentKey.ContentId.Equals(
				FailContentId,
				ESearchCase::CaseSensitive))
			{
				OutPersistedFingerprint.Reset();
				OutError = TEXT("Fixture forced typed target failure.");
				return false;
			}

			OutError.Reset();
			OutPersistedFingerprint = Target.DesiredFingerprint;
			PersistedFingerprints.Add(
				Target.ContentKey.ToStableString(),
				Target.DesiredFingerprint);
			SuccessfulApplyCount.FindOrAdd(Target.ContentKey.ContentId) += 1;
			return true;
		}
	};

	// Production publication authority를 in-memory로 모사하는 fixture입니다.
	class FMemoryPublicationStore final : public ICFProdPublicationStore
	{
	public:
		// Current published Product ContentId set입니다.
		TSet<FString> PublishedProductIds;

		// Published Product entry readback map입니다.
		TMap<FString, FCFProdEquipCatalogEntry> EntriesByContentId;

		// Withdrawal/apply ordering을 확인할 shared event log입니다.
		TArray<FString>* EventLog = nullptr;

		// Current publication membership을 stable order로 반환합니다.
		virtual bool ReadPublishedProductIds(
			TArray<FString>& OutProductContentIds,
			FString& OutError) const override
		{
			OutError.Reset();
			OutProductContentIds = PublishedProductIds.Array();
			OutProductContentIds.Sort();
			return true;
		}

		// Impact closure를 publication membership에서 제거합니다.
		virtual bool WithdrawProducts(
			const TArray<FString>& ProductContentIds,
			FString& OutError) override
		{
			OutError.Reset();
			for (const FString& ProductContentId : ProductContentIds)
			{
				if (EventLog != nullptr)
				{
					EventLog->Add(TEXT("withdraw:") + ProductContentId);
				}
				PublishedProductIds.Remove(ProductContentId);
				EntriesByContentId.Remove(ProductContentId);
			}
			return true;
		}

		// RuntimeVerified Product를 current publication membership에 추가합니다.
		virtual bool PublishProduct(
			const FCFProdEquipCatalogEntry& Entry,
			FString& OutError) override
		{
			OutError.Reset();
			if (EventLog != nullptr)
			{
				EventLog->Add(TEXT("publish:") + Entry.ContentId);
			}
			PublishedProductIds.Add(Entry.ContentId);
			EntriesByContentId.Add(Entry.ContentId, Entry);
			return true;
		}

		// Same canonical Product publication entry를 readback합니다.
		virtual bool ReadPublishedEntry(
			const FString& ProductContentId,
			FCFProdEquipCatalogEntry& OutEntry,
			FString& OutError) const override
		{
			// Current Product publication entry입니다.
			const FCFProdEquipCatalogEntry* Entry =
				EntriesByContentId.Find(ProductContentId);
			if (Entry == nullptr)
			{
				OutEntry = FCFProdEquipCatalogEntry();
				OutError = TEXT("Fixture publication entry missing.");
				return false;
			}

			OutEntry = *Entry;
			OutError.Reset();
			return true;
		}
	};

	// Existing FCFRuntimeEquipApplyService technical proof boundary를 모사하는 fixture입니다.
	class FPassRuntimeVerifier final : public ICFProdRuntimeVerifier
	{
	public:
		// Runtime verification 호출 수입니다.
		int32 VerifyCount = 0;

		// Runtime proof 호출 여부를 확인할 shared event log입니다.
		TArray<FString>* EventLog = nullptr;

		// Runtime technical proof를 deterministic success 처리합니다.
		virtual bool VerifyRuntime(
			ACFVehiclePawn* VehiclePawn,
			FName TargetMountProfileId,
			UCFEquipmentPresetData* EquipmentPresetData,
			const TArray<FCFAmmoSortieLoad>& InitialSortieAmmoLoads,
			FString& OutError) override
		{
			(void)VehiclePawn;
			(void)TargetMountProfileId;
			(void)InitialSortieAmmoLoads;

			if (!IsValid(EquipmentPresetData)
				|| !EquipmentPresetData->HasCompleteEquipmentData())
			{
				OutError = TEXT("Fixture EquipmentPresetData incomplete.");
				return false;
			}

			if (EventLog != nullptr)
			{
				EventLog->Add(TEXT("runtime"));
			}
			++VerifyCount;
			OutError.Reset();
			return true;
		}
	};

	// Product transaction recovery fixture request를 생성합니다.
	FCFProdProvisionRequest MakeRecoveryRequest(
		UCFEquipmentPresetData& EquipmentPresetData)
	{
		// Generated Product transaction request입니다.
		FCFProdProvisionRequest Request;
		Request.TransactionId = TEXT("CCASP007Recovery");
		Request.ProductKey = MakeKey(
			TEXT("EquipmentPreset"),
			TEXT("ProdCannonStandard"));
		BindSyntheticApprovalIdentity(Request, TEXT("recovery"));
		Request.WorkbookSemanticHash = HashText(TEXT("workbook"));
		Request.RoleTopologyFingerprint = HashText(TEXT("roles"));
		EquipmentPresetData.EquipmentId = TEXT("ProdCannonStandard");
		if (IsValid(EquipmentPresetData.DefaultWeaponData))
		{
			EquipmentPresetData.DefaultWeaponData->WeaponId = TEXT("ProdCannonStandard");
		}
		Request.EquipmentPresetObjectPath =
			FSoftObjectPath(&EquipmentPresetData).ToString();

		// First required child target입니다.
		FCFProdProvisionTarget FirstTarget;
		FirstTarget.ContentKey = MakeKey(
			TEXT("Weapon"),
			TEXT("ProdChildA"));
		FirstTarget.RoleId = TEXT("Weapon");
		FirstTarget.TargetObjectPath =
			TEXT("/Game/CarFight/Tests/CCASP007/DA_ProdChildA.DA_ProdChildA");
		FirstTarget.DesiredFingerprint = HashText(TEXT("child-a-v1"));
		FirstTarget.bRequired = true;
		FirstTarget.DependencyOrder = 0;

		// Second required child target입니다.
		FCFProdProvisionTarget SecondTarget;
		SecondTarget.ContentKey = MakeKey(
			TEXT("Projectile"),
			TEXT("ProdChildB"));
		SecondTarget.RoleId = TEXT("Projectile");
		SecondTarget.TargetObjectPath =
			TEXT("/Game/CarFight/Tests/CCASP007/DA_ProdChildB.DA_ProdChildB");
		SecondTarget.DesiredFingerprint = HashText(TEXT("child-b-v1"));
		SecondTarget.bRequired = true;
		SecondTarget.DependencyOrder = 1;

		// Product 자체 persisted EquipmentPreset target입니다.
		FCFProdProvisionTarget ProductTarget;
		ProductTarget.ContentKey = Request.ProductKey;
		ProductTarget.RoleId = TEXT("Product");
		ProductTarget.TargetObjectPath = Request.EquipmentPresetObjectPath;
		ProductTarget.DesiredFingerprint = HashText(TEXT("product-v1"));
		ProductTarget.bRequired = true;
		ProductTarget.DependencyOrder = 2;

		Request.Targets = {FirstTarget, SecondTarget, ProductTarget};

		// Product -> Weapon dependency입니다.
		FCFContentDependencyEdge ProductToWeapon;
		ProductToWeapon.From = Request.ProductKey;
		ProductToWeapon.To = FirstTarget.ContentKey;
		ProductToWeapon.FieldPath = TEXT("RoleBindings.Weapon");

		// Product -> Projectile dependency입니다.
		FCFContentDependencyEdge ProductToProjectile;
		ProductToProjectile.From = Request.ProductKey;
		ProductToProjectile.To = SecondTarget.ContentKey;
		ProductToProjectile.FieldPath = TEXT("RoleBindings.Projectile");

		Request.DependencyEdges = {
			ProductToWeapon,
			ProductToProjectile
		};
		return Request;
	}

	// Shared Production target exact1을 포함하는 closure Product request fixture를 생성합니다.
	FCFProdProvisionRequest MakeSharedClosureRequest(
		UCFEquipmentPresetData& EquipmentPresetData,
		const FString& ProductContentId,
		const FString& TransactionId,
		const ECFProdBindingMode SharedBindingMode)
	{
		// Generated closure Product request입니다.
		FCFProdProvisionRequest Request;
		Request.TransactionId = TransactionId;
		Request.ProductKey = MakeKey(TEXT("EquipmentPreset"), ProductContentId);
		BindSyntheticApprovalIdentity(Request, ProductContentId);
		Request.WorkbookSemanticHash = HashText(TEXT("closure-workbook"));
		Request.RoleTopologyFingerprint = HashText(TEXT("closure-roles"));
		EquipmentPresetData.EquipmentId = FName(*ProductContentId);
		if (IsValid(EquipmentPresetData.DefaultWeaponData))
		{
			EquipmentPresetData.DefaultWeaponData->WeaponId = FName(*ProductContentId);
		}
		Request.EquipmentPresetObjectPath =
			FSoftObjectPath(&EquipmentPresetData).ToString();

		// 두 Product가 공유하는 canonical shared Mount target입니다.
		FCFProdProvisionTarget SharedTarget;
		SharedTarget.ContentKey = MakeKey(TEXT("TurretMount"), TEXT("SharedMount_W1"));
		SharedTarget.RoleId = TEXT("TurretMount");
		SharedTarget.TargetObjectPath =
			TEXT("/Game/CarFight/Tests/CCASP007/DA_SharedMount_W1.DA_SharedMount_W1");
		SharedTarget.DesiredFingerprint = HashText(TEXT("shared-mount-v2"));
		SharedTarget.bRequired = true;
		SharedTarget.BindingMode = SharedBindingMode;
		SharedTarget.bShared = true;
		SharedTarget.DependencyOrder = 0;

		// Product 자체 persisted EquipmentPreset target입니다.
		FCFProdProvisionTarget ProductTarget;
		ProductTarget.ContentKey = Request.ProductKey;
		ProductTarget.RoleId = TEXT("Product");
		ProductTarget.TargetObjectPath = Request.EquipmentPresetObjectPath;
		ProductTarget.DesiredFingerprint = HashText(ProductContentId + TEXT("-product-v1"));
		ProductTarget.bRequired = true;
		ProductTarget.DependencyOrder = 1;

		Request.Targets = {SharedTarget, ProductTarget};

		// Product -> shared target canonical dependency입니다.
		FCFContentDependencyEdge ProductToShared;
		ProductToShared.From = Request.ProductKey;
		ProductToShared.To = SharedTarget.ContentKey;
		ProductToShared.FieldPath = TEXT("RoleBindings.TurretMount");
		Request.DependencyEdges = {ProductToShared};
		return Request;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdAmmoSchemaProjectionTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.AmmoSchemaProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// EquipmentPresetAmmoLoads provider-owned child schema와 typed row projection을 검증합니다.
bool FCFProdAmmoSchemaProjectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Frozen Production ammo child sheet schema입니다.
	const FCFContentSheetDescriptor Sheet =
		FCFProdWeaponSchema::BuildAmmoLoadSheetDescriptor();

	TestEqual(
		TEXT("Ammo load sheet stable ID"),
		Sheet.SheetId,
		FString(TEXT("EquipmentPresetAmmoLoads")));
	TestEqual(
		TEXT("Ammo load parent sheet"),
		Sheet.ParentSheetId,
		FString(TEXT("EquipmentPresets")));
	TestEqual(
		TEXT("Ammo load keyed collection"),
		Sheet.CollectionKind,
		ECFContentCollectionKind::KeyedCollection);
	TestEqual(
		TEXT("Ammo load provider field count exact2"),
		Sheet.Fields.Num(),
		2);

	// Parent EquipmentPreset canonical record입니다.
	FCFContentRecord Record;
	Record.Key =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("EquipmentPreset"),
			TEXT("ProdCannonStandard"));

	// Provider-owned ammo child collection입니다.
	FCFContentCollection Collection;
	Collection.Kind = ECFContentCollectionKind::KeyedCollection;

	// Canonical explicit sortie ammo child row입니다.
	FCFContentCollectionItem Item;
	Item.ChildItemId = TEXT("AmmoLoad.Main");
	Item.DomainKey = TEXT("AmmoMain");
	Item.Fields.Add(
		TEXT("AmmoRoleContentId"),
		CFProdProvisioningTestsPrivate::MakeNameValue(TEXT("AmmoMain")));
	Item.Fields.Add(
		TEXT("DefaultSortieAmmoCount"),
		CFProdProvisioningTestsPrivate::MakeIntValue(40));
	Collection.Items.Add(Item);
	Record.Collections.Add(TEXT("EquipmentPresetAmmoLoads"), Collection);

	// Typed projection rows입니다.
	TArray<FCFProdAmmoLoadRow> Rows;

	// Projection diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Ammo load canonical projection succeeds"),
		FCFProdWeaponSchema::ProjectAmmoLoadRows(
			Record,
			Rows,
			Error));
	TestEqual(TEXT("Ammo load row count exact1"), Rows.Num(), 1);
	if (Rows.Num() == 1)
	{
		TestEqual(
			TEXT("Ammo row parent ProductContentId"),
			Rows[0].ProductContentId,
			FString(TEXT("ProdCannonStandard")));
		TestEqual(
			TEXT("Ammo row role ContentId"),
			Rows[0].AmmoRoleContentId,
			FString(TEXT("AmmoMain")));
		TestEqual(
			TEXT("Ammo row explicit sortie count"),
			Rows[0].DefaultSortieAmmoCount,
			40);
	}

	// DomainKey drift fixture입니다.
	FCFContentRecord DriftedRecord = Record;
	DriftedRecord.Collections[TEXT("EquipmentPresetAmmoLoads")]
		.Items[0]
		.DomainKey = TEXT("AmmoOther");
	TestFalse(
		TEXT("Ammo row DomainKey drift fails closed"),
		FCFProdWeaponSchema::ProjectAmmoLoadRows(
			DriftedRecord,
			Rows,
			Error));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdImpactGraphTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.ImpactGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Published reverse impact closure와 transitive required fingerprint 전파를 검증합니다.
bool FCFProdImpactGraphTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable complete EquipmentPreset fixture입니다.
	UCFEquipmentPresetData* EquipmentPresetData =
		CFProdProvisioningTestsPrivate::MakeInfiniteEquipmentPreset(
			TEXT("DA_ImpactGraphPreset"));
	TestNotNull(TEXT("ImpactGraph EquipmentPreset fixture"), EquipmentPresetData);
	if (EquipmentPresetData == nullptr)
	{
		return false;
	}

	// Recovery fixture와 동일한 Product dependency graph request입니다.
	const FCFProdProvisionRequest Request =
		CFProdProvisioningTestsPrivate::MakeRecoveryRequest(
			*EquipmentPresetData);

	// Current published Product membership입니다.
	TArray<FString> PublishedProductIds = {
		TEXT("ProdCannonStandard"),
		TEXT("UnrelatedProduct")
	};

	// Reverse impact closure result입니다.
	TArray<FString> ImpactedProductIds;

	// Impact closure diagnostic입니다.
	FString Error;
	// Shared child mutation이 시작점인 changed key set입니다.
	TArray<FCFContentKey> ChangedContentKeys = {
		Request.Targets[0].ContentKey
	};
	TestTrue(
		TEXT("Published impact closure builds"),
		FCFProdProvisioning::BuildPublishedImpactClosure(
			Request,
			ChangedContentKeys,
			PublishedProductIds,
			ImpactedProductIds,
			Error));
	TestEqual(
		TEXT("Impact closure exact1"),
		ImpactedProductIds.Num(),
		1);
	if (ImpactedProductIds.Num() == 1)
	{
		TestEqual(
			TEXT("Product depending on changed child is withdrawn"),
			ImpactedProductIds[0],
			FString(TEXT("ProdCannonStandard")));
	}

	// Published Product와 ContentId만 같은 다른 타입 child key입니다.
	FCFContentKey CrossTypeCollisionKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Ammo"),
			TEXT("ProdCannonStandard"));
	// Cross-type collision만 시작점으로 쓸 때 Product로 오인하면 안 됩니다.
	TArray<FCFContentKey> CrossTypeCollisionKeys = {CrossTypeCollisionKey};
	TArray<FString> CollisionImpactedProductIds;
	TestTrue(
		TEXT("Cross-type ContentId collision impact closure builds"),
		FCFProdProvisioning::BuildPublishedImpactClosure(
			Request,
			CrossTypeCollisionKeys,
			PublishedProductIds,
			CollisionImpactedProductIds,
			Error));
	TestEqual(
		TEXT("Cross-type same ContentId is not mistaken for published Product"),
		CollisionImpactedProductIds.Num(),
		0);

	// First required target persisted evidence입니다.
	FCFProdTargetEvidence FirstEvidence;
	FirstEvidence.ContentKey = Request.Targets[0].ContentKey;
	FirstEvidence.DesiredFingerprint = Request.Targets[0].DesiredFingerprint;
	FirstEvidence.PersistedFingerprint = Request.Targets[0].DesiredFingerprint;

	// Second required target persisted evidence입니다.
	FCFProdTargetEvidence SecondEvidence;
	SecondEvidence.ContentKey = Request.Targets[1].ContentKey;
	SecondEvidence.DesiredFingerprint = Request.Targets[1].DesiredFingerprint;
	SecondEvidence.PersistedFingerprint = Request.Targets[1].DesiredFingerprint;

	// Product 자체 required target persisted evidence입니다.
	FCFProdTargetEvidence ProductEvidence;
	ProductEvidence.ContentKey = Request.Targets[2].ContentKey;
	ProductEvidence.DesiredFingerprint = Request.Targets[2].DesiredFingerprint;
	ProductEvidence.PersistedFingerprint = Request.Targets[2].DesiredFingerprint;

	// Exact required graph evidence set입니다.
	TArray<FCFProdTargetEvidence> Evidence = {
		FirstEvidence,
		SecondEvidence,
		ProductEvidence
	};

	// Baseline ProductGraphFingerprint입니다.
	FString BaselineFingerprint;
	TestTrue(
		TEXT("ProductGraphFingerprint builds"),
		FCFProdProvisioning::BuildProductGraphFingerprint(
			Request,
			Evidence,
			BaselineFingerprint,
			Error));

	// Required transitive target의 legitimate desired/persisted fingerprint가 함께 바뀐 request입니다.
	FCFProdProvisionRequest UpdatedRequest = Request;
	UpdatedRequest.Targets[1].DesiredFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("child-b-v2"));
	Evidence[1].DesiredFingerprint =
		UpdatedRequest.Targets[1].DesiredFingerprint;
	Evidence[1].PersistedFingerprint =
		UpdatedRequest.Targets[1].DesiredFingerprint;

	// Updated ProductGraphFingerprint입니다.
	FString UpdatedFingerprint;
	TestTrue(
		TEXT("Updated required target graph fingerprint builds"),
		FCFProdProvisioning::BuildProductGraphFingerprint(
			UpdatedRequest,
			Evidence,
			UpdatedFingerprint,
			Error));
	TestNotEqual(
		TEXT("Required transitive target fingerprint change propagates to ProductGraphFingerprint"),
		UpdatedFingerprint,
		BaselineFingerprint);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdRecoveryResumeTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.RecoveryResume",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Withdrawal-before-mutation, partial failure RecoveryRequired, exact-fingerprint resume skip과 VehicleReady를 검증합니다.
bool FCFProdRecoveryResumeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable complete Product EquipmentPreset fixture입니다.
	UCFEquipmentPresetData* EquipmentPresetData =
		CFProdProvisioningTestsPrivate::MakeInfiniteEquipmentPreset(
			TEXT("DA_RecoveryPreset"));
	TestNotNull(TEXT("Recovery EquipmentPreset fixture"), EquipmentPresetData);
	if (EquipmentPresetData == nullptr)
	{
		return false;
	}

	// Production transaction request입니다.
	const FCFProdProvisionRequest Request =
		CFProdProvisioningTestsPrivate::MakeRecoveryRequest(
			*EquipmentPresetData);

	// Review 당시 absent-required였던 CreateNew target이 다른 persisted payload로 선점된 stale-review fixture입니다.
	FCFProdProvisionRequest CollisionRequest = Request;
	CollisionRequest.TransactionId = TEXT("CCASP007Collision");
	CollisionRequest.Targets[0].bRequireAbsentAtReview = true;
	CFProdProvisioningTestsPrivate::FMemoryTransactionStore CollisionTransactionStore;
	CFProdProvisioningTestsPrivate::FMemoryTypedAdapter CollisionTypedAdapter;
	CFProdProvisioningTestsPrivate::FMemoryPublicationStore CollisionPublicationStore;
	CFProdProvisioningTestsPrivate::FPassRuntimeVerifier CollisionRuntimeVerifier;
	CollisionTypedAdapter.PersistedFingerprints.Add(
		CollisionRequest.Targets[0].ContentKey.ToStableString(),
		CFProdProvisioningTestsPrivate::HashText(TEXT("foreign-existing-payload")));
	FCFProdTransaction CollisionTransaction;
	TestFalse(
		TEXT("Unexpected-existing absent-required CreateNew target blocks before mutation"),
		FCFProdProvisioning::ExecuteProductTransaction(
			CollisionRequest,
			CollisionTypedAdapter,
			CollisionTransactionStore,
			CollisionPublicationStore,
			CollisionRuntimeVerifier,
			nullptr,
			NAME_None,
			CollisionTransaction));
	TestEqual(
		TEXT("Unexpected-existing collision is BlockedBeforeMutation"),
		CollisionTransaction.State,
		ECFProdTxnState::BlockedBeforeMutation);
	TestEqual(
		TEXT("Unexpected-existing collision performs target mutation exact0"),
		CollisionTypedAdapter.SuccessfulApplyCount.Num(),
		0);
	TestEqual(
		TEXT("Unexpected-existing collision leaves publication entries exact0"),
		CollisionPublicationStore.EntriesByContentId.Num(),
		0);
	TestEqual(
		TEXT("Unexpected-existing collision performs runtime proof exact0"),
		CollisionRuntimeVerifier.VerifyCount,
		0);

	// Durable-style in-memory transaction store입니다.
	CFProdProvisioningTestsPrivate::FMemoryTransactionStore TransactionStore;

	// Existing typed backend adapter fixture입니다.
	CFProdProvisioningTestsPrivate::FMemoryTypedAdapter TypedAdapter;
	TypedAdapter.FailContentId = TEXT("ProdChildB");

	// Publication store fixture입니다.
	CFProdProvisioningTestsPrivate::FMemoryPublicationStore PublicationStore;
	PublicationStore.PublishedProductIds.Add(TEXT("ProdCannonStandard"));

	// Runtime technical proof fixture입니다.
	CFProdProvisioningTestsPrivate::FPassRuntimeVerifier RuntimeVerifier;

	// Shared operation ordering event log입니다.
	TArray<FString> EventLog;
	TypedAdapter.EventLog = &EventLog;
	PublicationStore.EventLog = &EventLog;
	RuntimeVerifier.EventLog = &EventLog;

	// First partial-failure transaction result입니다.
	FCFProdTransaction FirstTransaction;
	TestFalse(
		TEXT("First partial transaction fails for recovery"),
		FCFProdProvisioning::ExecuteProductTransaction(
			Request,
			TypedAdapter,
			TransactionStore,
			PublicationStore,
			RuntimeVerifier,
			nullptr,
			NAME_None,
			FirstTransaction));
	TestEqual(
		TEXT("Partial durable mutation enters RecoveryRequired"),
		FirstTransaction.State,
		ECFProdTxnState::RecoveryRequired);
	TestEqual(
		TEXT("Impacted Product composite key is durable-bound exact1"),
		FirstTransaction.ImpactedProductKeys.Num(),
		1);
	if (FirstTransaction.ImpactedProductKeys.Num() == 1)
	{
		TestTrue(
			TEXT("Impacted Product durable key keeps full ContentKey identity"),
			FirstTransaction.ImpactedProductKeys[0] == Request.ProductKey);
	}
	TestEqual(
		TEXT("Transaction target evidence count matches request"),
		FirstTransaction.Targets.Num(),
		Request.Targets.Num());
	for (const FCFProdProvisionTarget& RequestTarget : Request.Targets)
	{
		// Matching durable evidence row입니다.
		const FCFProdTargetEvidence* Evidence = FirstTransaction.Targets.FindByPredicate(
			[&RequestTarget](const FCFProdTargetEvidence& Candidate)
			{
				return Candidate.ContentKey == RequestTarget.ContentKey;
			});
		TestNotNull(TEXT("Each request target has durable evidence"), Evidence);
		if (Evidence != nullptr)
		{
			TestEqual(
				TEXT("Durable target evidence binds exact object path"),
				Evidence->TargetObjectPath,
				RequestTarget.TargetObjectPath);
		}
	}
	TestEqual(
		TEXT("First target mutated exact1 before failure"),
		TypedAdapter.SuccessfulApplyCount.FindRef(TEXT("ProdChildA")),
		1);
	TestEqual(
		TEXT("Failed second target has no successful mutation"),
		TypedAdapter.SuccessfulApplyCount.FindRef(TEXT("ProdChildB")),
		0);

	TestTrue(
		TEXT("Operation log contains withdrawal and apply"),
		EventLog.Num() >= 2);
	if (EventLog.Num() >= 2)
	{
		TestEqual(
			TEXT("Published impact closure withdrawal occurs before child mutation"),
			EventLog[0],
			FString(TEXT("withdraw:ProdCannonStandard")));
		TestEqual(
			TEXT("First child mutation follows withdrawal"),
			EventLog[1],
			FString(TEXT("apply:ProdChildA")));
	}

	// 동일 ContentKey/fingerprint라도 target object path가 바뀐 resume request는 차단해야 합니다.
	FCFProdProvisionRequest PathDriftRequest = Request;
	PathDriftRequest.Targets[0].TargetObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/DA_ProdChildA_Moved.DA_ProdChildA_Moved");
	// Path drift resume result입니다.
	FCFProdTransaction PathDriftTransaction;
	TestFalse(
		TEXT("Resume blocks same identity/fingerprint when target object path drifted"),
		FCFProdProvisioning::ExecuteProductTransaction(
			PathDriftRequest,
			TypedAdapter,
			TransactionStore,
			PublicationStore,
			RuntimeVerifier,
			nullptr,
			NAME_None,
			PathDriftTransaction));

	// Resume에서는 failed target을 정상화합니다.
	TypedAdapter.FailContentId.Reset();

	// Resume terminal transaction result입니다.
	FCFProdTransaction ResumedTransaction;
	TestTrue(
		TEXT("Exact transaction resumes to VehicleReady"),
		FCFProdProvisioning::ExecuteProductTransaction(
			Request,
			TypedAdapter,
			TransactionStore,
			PublicationStore,
			RuntimeVerifier,
			nullptr,
			NAME_None,
			ResumedTransaction));
	TestEqual(
		TEXT("Resumed transaction reaches VehicleReady"),
		ResumedTransaction.State,
		ECFProdTxnState::VehicleReady);
	TestEqual(
		TEXT("Already persisted exact fingerprint target is not mutated twice"),
		TypedAdapter.SuccessfulApplyCount.FindRef(TEXT("ProdChildA")),
		1);
	TestEqual(
		TEXT("Previously failed target is mutated exact1 on resume"),
		TypedAdapter.SuccessfulApplyCount.FindRef(TEXT("ProdChildB")),
		1);
	TestEqual(
		TEXT("Product EquipmentPreset target is mutated exact1 after children complete"),
		TypedAdapter.SuccessfulApplyCount.FindRef(TEXT("ProdCannonStandard")),
		1);
	TestEqual(
		TEXT("Runtime technical proof executes once after graph complete"),
		RuntimeVerifier.VerifyCount,
		1);

	// VehicleReady publication same-canonical readback입니다.
	FCFProdEquipCatalogEntry PublishedEntry;

	// Publication readback diagnostic입니다.
	FString PublishReadbackError;
	TestTrue(
		TEXT("VehicleReady Product is published"),
		PublicationStore.ReadPublishedEntry(
			TEXT("ProdCannonStandard"),
			PublishedEntry,
			PublishReadbackError));
	TestEqual(
		TEXT("Published graph fingerprint equals transaction graph fingerprint"),
		PublishedEntry.ProductGraphFingerprint,
		ResumedTransaction.ProductGraphFingerprint);

	// VehicleReady 이후 previously-mutated target drift fixture입니다.
	TypedAdapter.PersistedFingerprints.Add(
		Request.Targets[0].ContentKey.ToStableString(),
		CFProdProvisioningTestsPrivate::HashText(TEXT("external-drift")));

	// Drifted resume result입니다.
	FCFProdTransaction DriftedTransaction;
	TestFalse(
		TEXT("Previously mutated target drift fails closed"),
		FCFProdProvisioning::ExecuteProductTransaction(
			Request,
			TypedAdapter,
			TransactionStore,
			PublicationStore,
			RuntimeVerifier,
			nullptr,
			NAME_None,
			DriftedTransaction));
	TestEqual(
		TEXT("VehicleReady transaction drift is RecoveryRequired"),
		DriftedTransaction.State,
		ECFProdTxnState::RecoveryRequired);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdAmmoDefaultTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.AmmoDefault",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Finite ammo Runtime projection이 MaximumLoadableAmmoCount가 아니라 explicit DefaultSortieAmmoCount를 사용함을 검증합니다.
bool FCFProdAmmoDefaultTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable in-memory package입니다.
	UPackage* Package = CreatePackage(
		TEXT("/Game/CarFight/Tests/CCASP007/DA_AmmoProjection"));
	TestNotNull(TEXT("Ammo projection package"), Package);
	if (Package == nullptr)
	{
		return false;
	}

	// Explicit finite AmmoData fixture입니다.
	UCFAmmoData* AmmoData =
		NewObject<UCFAmmoData>(
			Package,
			TEXT("DA_TestAmmo"),
			RF_Public | RF_Standalone);
	AmmoData->AmmoId = TEXT("AmmoMain");
	AmmoData->MaximumLoadableAmmoCount = 99;

	// Finite WeaponData fixture입니다.
	UCFWeaponData* WeaponData =
		NewObject<UCFWeaponData>(
			Package,
			TEXT("DA_TestWeapon"),
			RF_Transient);
	WeaponData->bUseInfiniteAmmoForDebug = false;
	WeaponData->DefaultAmmoData = AmmoData;
	WeaponData->MagazineSize = 5;
	WeaponData->InitialLoadedAmmoCount = 2;

	// Complete weapon package용 TurretMountData입니다.
	UCFTurretMountData* TurretMountData =
		NewObject<UCFTurretMountData>(
			Package,
			TEXT("DA_TestMount"),
			RF_Transient);

	// Product EquipmentPreset fixture입니다.
	UCFEquipmentPresetData* EquipmentPresetData =
		NewObject<UCFEquipmentPresetData>(
			Package,
			TEXT("DA_TestPreset"),
			RF_Transient);
	EquipmentPresetData->EquipmentId = TEXT("ProdCannonStandard");
	EquipmentPresetData->DefaultTurretMountData = TurretMountData;
	EquipmentPresetData->DefaultWeaponData = WeaponData;
	WeaponData->WeaponId = TEXT("ProdCannonStandard");

	// Ammo target resolution을 포함하는 Product request입니다.
	FCFProdProvisionRequest Request;
	Request.ProductKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("EquipmentPreset"),
			TEXT("ProdCannonStandard"));

	// Resolved Ammo Role target입니다.
	FCFProdProvisionTarget AmmoTarget;
	AmmoTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Ammo"),
			TEXT("AmmoMain"));
	AmmoTarget.RoleId = TEXT("Ammo");
	AmmoTarget.TargetObjectPath =
		FSoftObjectPath(AmmoData).ToString();
	AmmoTarget.DesiredFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("ammo"));
	Request.Targets.Add(AmmoTarget);

	// Workbook explicit default sortie ammo projection입니다.
	FCFProdAmmoLoadRow AmmoRow;
	AmmoRow.ProductContentId = TEXT("ProdCannonStandard");
	AmmoRow.AmmoRoleContentId = TEXT("AmmoMain");
	AmmoRow.DefaultSortieAmmoCount = 7;
	Request.AmmoLoadRows.Add(AmmoRow);

	// Runtime candidate sortie ammo loads입니다.
	TArray<FCFAmmoSortieLoad> RuntimeAmmoLoads;

	// Ammo projection diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Finite ammo explicit projection succeeds"),
		FCFProdProvisioning::BuildRuntimeAmmoLoads(
			Request,
			*EquipmentPresetData,
			nullptr,
			RuntimeAmmoLoads,
			Error));
	TestEqual(
		TEXT("Finite ammo projection exact1"),
		RuntimeAmmoLoads.Num(),
		1);
	if (RuntimeAmmoLoads.Num() == 1)
	{
		TestEqual(
			TEXT("Projection uses explicit DefaultSortieAmmoCount"),
			RuntimeAmmoLoads[0].InitialSortieAmmoCount,
			7);
		TestNotEqual(
			TEXT("Projection does not infer current sortie amount from MaximumLoadableAmmoCount"),
			RuntimeAmmoLoads[0].InitialSortieAmmoCount,
			AmmoData->MaximumLoadableAmmoCount);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdSharedClosureTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.SharedClosure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Shared single-writer와 impacted published Product 전체 선철회 후 deterministic revalidation/republish를 검증합니다.
bool FCFProdSharedClosureTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 첫 번째 Product의 disposable complete EquipmentPreset입니다.
	UCFEquipmentPresetData* ProductAData =
		CFProdProvisioningTestsPrivate::MakeInfiniteEquipmentPreset(
			TEXT("ProdClosureA"));
	// 두 번째 Product의 disposable complete EquipmentPreset입니다.
	UCFEquipmentPresetData* ProductBData =
		CFProdProvisioningTestsPrivate::MakeInfiniteEquipmentPreset(
			TEXT("ProdClosureB"));
	TestNotNull(TEXT("Closure Product A fixture"), ProductAData);
	TestNotNull(TEXT("Closure Product B fixture"), ProductBData);
	if (ProductAData == nullptr || ProductBData == nullptr)
	{
		return false;
	}

	// Shared creator Product request입니다.
	FCFProdProvisionRequest ProductARequest =
		CFProdProvisioningTestsPrivate::MakeSharedClosureRequest(
			*ProductAData,
			TEXT("ProdClosureA"),
			TEXT("P007ClosureA"),
			ECFProdBindingMode::CreateNewShared);
	// Shared consumer Product request입니다.
	FCFProdProvisionRequest ProductBRequest =
		CFProdProvisioningTestsPrivate::MakeSharedClosureRequest(
			*ProductBData,
			TEXT("ProdClosureB"),
			TEXT("P007ClosureB"),
			ECFProdBindingMode::BindShared);

	// Closure-wide durable transaction fixture입니다.
	CFProdProvisioningTestsPrivate::FMemoryTransactionStore TransactionStore;
	// Existing typed backend fixture입니다.
	CFProdProvisioningTestsPrivate::FMemoryTypedAdapter TypedAdapter;
	// Publication catalog fixture입니다.
	CFProdProvisioningTestsPrivate::FMemoryPublicationStore PublicationStore;
	// Runtime proof fixture입니다.
	CFProdProvisioningTestsPrivate::FPassRuntimeVerifier RuntimeVerifier;
	// Cross-boundary operation order log입니다.
	TArray<FString> EventLog;
	TypedAdapter.EventLog = &EventLog;
	PublicationStore.EventLog = &EventLog;
	RuntimeVerifier.EventLog = &EventLog;

	// 두 Product는 현재 publish된 상태입니다.
	PublicationStore.PublishedProductIds.Add(TEXT("ProdClosureA"));
	PublicationStore.PublishedProductIds.Add(TEXT("ProdClosureB"));

	// Shared target만 desired state와 달라 closure 전체 revalidation이 필요합니다.
	TypedAdapter.PersistedFingerprints.Add(
		ProductARequest.Targets[0].ContentKey.ToStableString(),
		CFProdProvisioningTestsPrivate::HashText(TEXT("shared-mount-v1")));
	// Product A 자체 target은 이미 desired state입니다.
	TypedAdapter.PersistedFingerprints.Add(
		ProductARequest.Targets[1].ContentKey.ToStableString(),
		ProductARequest.Targets[1].DesiredFingerprint);
	// Product B 자체 target은 이미 desired state입니다.
	TypedAdapter.PersistedFingerprints.Add(
		ProductBRequest.Targets[1].ContentKey.ToStableString(),
		ProductBRequest.Targets[1].DesiredFingerprint);

	// Shared closure execution A입니다.
	FCFProdProductExecution ProductAExecution;
	ProductAExecution.Request = ProductARequest;
	// Shared closure execution B입니다.
	FCFProdProductExecution ProductBExecution;
	ProductBExecution.Request = ProductBRequest;
	// Multi-Product closure execution set입니다.
	TArray<FCFProdProductExecution> Executions = {
		ProductAExecution,
		ProductBExecution
	};

	// Closure terminal report입니다.
	FCFProdClosureExecutionReport Report;
	TestTrue(
		TEXT("Shared closure reaches VehicleReady for all impacted Products"),
		FCFProdProvisioning::ExecuteProductClosure(
			Executions,
			TypedAdapter,
			TransactionStore,
			PublicationStore,
			RuntimeVerifier,
			Report));
	TestEqual(
		TEXT("Shared closure VehicleReady exact2"),
		Report.VehicleReadyProductKeys.Num(),
		2);
	TestEqual(
		TEXT("Shared closure RecoveryRequired exact0"),
		Report.RecoveryRequiredProductKeys.Num(),
		0);
	TestEqual(
		TEXT("Shared target durable mutation occurs exact1"),
		TypedAdapter.SuccessfulApplyCount.FindRef(TEXT("SharedMount_W1")),
		1);
	TestEqual(
		TEXT("Both impacted Products receive Runtime proof"),
		RuntimeVerifier.VerifyCount,
		2);

	TestTrue(TEXT("Closure event log contains withdrawal and mutation"), EventLog.Num() >= 3);
	if (EventLog.Num() >= 3)
	{
		TestEqual(
			TEXT("First impacted Product is withdrawn before any target mutation"),
			EventLog[0],
			FString(TEXT("withdraw:ProdClosureA")));
		TestEqual(
			TEXT("Second impacted Product is withdrawn before any target mutation"),
			EventLog[1],
			FString(TEXT("withdraw:ProdClosureB")));
		TestEqual(
			TEXT("Shared target mutation begins only after full closure withdrawal"),
			EventLog[2],
			FString(TEXT("apply:SharedMount_W1")));
	}
	TestTrue(
		TEXT("Product A is republished after proof"),
		PublicationStore.PublishedProductIds.Contains(TEXT("ProdClosureA")));
	TestTrue(
		TEXT("Product B is republished after proof"),
		PublicationStore.PublishedProductIds.Contains(TEXT("ProdClosureB")));

	// Product A terminal transaction snapshot입니다.
	const FCFProdTransaction* ProductATransaction =
		TransactionStore.Transactions.Find(TEXT("P007ClosureA"));
	// Product B terminal transaction snapshot입니다.
	const FCFProdTransaction* ProductBTransaction =
		TransactionStore.Transactions.Find(TEXT("P007ClosureB"));
	TestNotNull(TEXT("Product A closure transaction exists"), ProductATransaction);
	TestNotNull(TEXT("Product B closure transaction exists"), ProductBTransaction);
	if (ProductATransaction != nullptr && ProductBTransaction != nullptr)
	{
		TestEqual(
			TEXT("Product A journal binds whole impacted closure exact2"),
			ProductATransaction->ImpactedProductKeys.Num(),
			2);
		TestEqual(
			TEXT("Product B journal binds whole impacted closure exact2"),
			ProductBTransaction->ImpactedProductKeys.Num(),
			2);
	}

	// Absent shared target인데 creator 없이 BindShared consumer만 있는 invalid topology fixture입니다.
	CFProdProvisioningTestsPrivate::FMemoryTransactionStore InvalidTransactionStore;
	CFProdProvisioningTestsPrivate::FMemoryTypedAdapter InvalidTypedAdapter;
	CFProdProvisioningTestsPrivate::FMemoryPublicationStore InvalidPublicationStore;
	CFProdProvisioningTestsPrivate::FPassRuntimeVerifier InvalidRuntimeVerifier;
	TArray<FString> InvalidEventLog;
	InvalidTypedAdapter.EventLog = &InvalidEventLog;
	InvalidPublicationStore.EventLog = &InvalidEventLog;
	InvalidRuntimeVerifier.EventLog = &InvalidEventLog;
	InvalidPublicationStore.PublishedProductIds.Add(TEXT("ProdClosureA"));
	InvalidPublicationStore.PublishedProductIds.Add(TEXT("ProdClosureB"));

	FCFProdProvisionRequest InvalidARequest =
		CFProdProvisioningTestsPrivate::MakeSharedClosureRequest(
			*ProductAData,
			TEXT("ProdClosureA"),
			TEXT("P007ClosureInvalidA"),
			ECFProdBindingMode::BindShared);
	FCFProdProvisionRequest InvalidBRequest =
		CFProdProvisioningTestsPrivate::MakeSharedClosureRequest(
			*ProductBData,
			TEXT("ProdClosureB"),
			TEXT("P007ClosureInvalidB"),
			ECFProdBindingMode::BindShared);
	FCFProdProductExecution InvalidAExecution;
	InvalidAExecution.Request = InvalidARequest;
	FCFProdProductExecution InvalidBExecution;
	InvalidBExecution.Request = InvalidBRequest;
	TArray<FCFProdProductExecution> InvalidExecutions = {
		InvalidAExecution,
		InvalidBExecution
	};
	FCFProdClosureExecutionReport InvalidReport;
	TestFalse(
		TEXT("Absent shared target without CreateNewShared creator fails before mutation"),
		FCFProdProvisioning::ExecuteProductClosure(
			InvalidExecutions,
			InvalidTypedAdapter,
			InvalidTransactionStore,
			InvalidPublicationStore,
			InvalidRuntimeVerifier,
			InvalidReport));
	TestEqual(
		TEXT("Invalid shared topology performs no withdraw/apply/runtime/publish"),
		InvalidEventLog.Num(),
		0);
	TestTrue(
		TEXT("Invalid topology keeps Product A publication visible"),
		InvalidPublicationStore.PublishedProductIds.Contains(TEXT("ProdClosureA")));
	TestTrue(
		TEXT("Invalid topology keeps Product B publication visible"),
		InvalidPublicationStore.PublishedProductIds.Contains(TEXT("ProdClosureB")));
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdConcreteBindingTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.ConcreteBindings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Production target exact contract가 existing Weapon Guide / Equipment backend fingerprint와 결속되는지 검증합니다.
bool FCFProdConcreteBindingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Existing Weapon Guide backend 입력으로 사용할 minimal typed Draft입니다.
	FCFWeaponGuideDraft WeaponDraft;
	WeaponDraft.RequiredMountType = ECFVehicleMountType::Turret;
	WeaponDraft.RequiredWeaponSize = ECFVehicleWeaponSize::Large;
	WeaponDraft.Weapon.bUseFiniteAmmo = true;
	WeaponDraft.Weapon.AmmoTypeId = TEXT("BindingAmmo");
	// Existing Damage provider가 요구하는 finite positive BaseDamage입니다.
	WeaponDraft.Damage.BaseDamage = 100.0f;
	WeaponDraft.Ammo.AmmoDisplayName = TEXT("Binding Ammo");
	WeaponDraft.Ammo.MaximumLoadableAmmoCount = 32;

	// Resolved child object paths는 실제 asset을 만들지 않는 read-only binding fixture입니다.
	const FString DamageObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingDamage.DA_BindingDamage");
	const FString AmmoObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingAmmo.DA_BindingAmmo");
	const FString ProjectileObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingProjectile.DA_BindingProjectile");
	const FString WeaponObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingWeapon.DA_BindingWeapon");
	const FString TurretObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingTurret.DA_BindingTurret");
	const FString EquipmentObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingEquip.DA_BindingEquip");

	// Turret target backend desired fingerprint입니다.
	FString TurretFingerprint;
	// Damage target backend desired fingerprint입니다.
	FString DamageFingerprint;
	// Ammo target backend desired fingerprint입니다.
	FString AmmoFingerprint;
	// Projectile target backend desired fingerprint입니다.
	FString ProjectileFingerprint;
	// Weapon target backend desired fingerprint입니다.
	FString WeaponFingerprint;
	// Backend fingerprint diagnostic입니다.
	FString BackendError;

	TestTrue(
		TEXT("Turret backend desired fingerprint builds"),
		CFWeaponGuideBackend::BuildTurretDesiredFingerprint(
			WeaponDraft,
			TEXT("BindingTurret"),
			TurretFingerprint,
			BackendError));
	TestTrue(
		TEXT("Damage backend desired fingerprint builds"),
		CFWeaponGuideBackend::BuildDamageDesiredFingerprint(
			WeaponDraft,
			TEXT("BindingDamage"),
			DamageFingerprint,
			BackendError));
	TestTrue(
		TEXT("Ammo backend desired fingerprint builds"),
		CFWeaponGuideBackend::BuildAmmoDesiredFingerprint(
			WeaponDraft,
			TEXT("BindingAmmo"),
			AmmoFingerprint,
			BackendError));
	TestTrue(
		TEXT("Projectile backend desired fingerprint builds"),
		CFWeaponGuideBackend::BuildProjectileDesiredFingerprint(
			WeaponDraft,
			TEXT("BindingProjectile"),
			FSoftObjectPath(DamageObjectPath),
			ProjectileFingerprint,
			BackendError));
	TestTrue(
		TEXT("Weapon backend desired fingerprint builds"),
		CFWeaponGuideBackend::BuildWeaponDesiredFingerprint(
			WeaponDraft,
			TEXT("BindingWeapon"),
			FSoftObjectPath(ProjectileObjectPath),
			FSoftObjectPath(AmmoObjectPath),
			TEXT("BindingAmmo"),
			WeaponFingerprint,
			BackendError));

	// Equipment durable backend 입력으로 사용할 minimal exact7 Draft입니다.
	FCFEquipmentPresetDraft EquipmentDraft;
	EquipmentDraft.EquipmentId = TEXT("BindingEquip");
	EquipmentDraft.DisplayName = FText::FromString(TEXT("Binding Equip"));
	EquipmentDraft.RequiredMountType = ECFVehicleMountType::Turret;
	EquipmentDraft.RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// Existing Equipment durable backend desired snapshot입니다.
	FCFEquipmentSemanticSnapshot EquipmentSnapshot;
	TestTrue(
		TEXT("Equipment backend desired fingerprint builds"),
		FCFEquipmentDurable::BuildSnapshotFromDraft(
			EquipmentDraft,
			EquipmentSnapshot,
			BackendError));

	// Concrete Production adapter입니다.
	FCFProdBoundTargetAdapter Adapter;
	// Binding diagnostic입니다.
	FString BindingError;

	// Turret reviewed target contract입니다.
	FCFProdProvisionTarget TurretTarget;
	TurretTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("TurretMount"),
			TEXT("BindingTurret"));
	TurretTarget.RoleId = TEXT("TurretMount");
	TurretTarget.TargetObjectPath = TurretObjectPath;
	TurretTarget.DesiredFingerprint = TurretFingerprint;
	TurretTarget.BindingMode = ECFProdBindingMode::CreateNew;
	TestTrue(
		TEXT("Turret target binds to existing Weapon Guide backend"),
		FCFProdTargetBindings::BindTurret(
			Adapter,
			TurretTarget,
			WeaponDraft,
			BindingError));

	// Damage reviewed target contract입니다.
	FCFProdProvisionTarget DamageTarget;
	DamageTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Damage"),
			TEXT("BindingDamage"));
	DamageTarget.RoleId = TEXT("Damage");
	DamageTarget.TargetObjectPath = DamageObjectPath;
	DamageTarget.DesiredFingerprint = DamageFingerprint;
	DamageTarget.BindingMode = ECFProdBindingMode::CreateNew;
	TestTrue(
		TEXT("Damage target binds to existing Damage provider"),
		FCFProdTargetBindings::BindDamage(
			Adapter,
			DamageTarget,
			WeaponDraft,
			BindingError));

	// Ammo reviewed target contract입니다.
	FCFProdProvisionTarget AmmoTarget;
	AmmoTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Ammo"),
			TEXT("BindingAmmo"));
	AmmoTarget.RoleId = TEXT("Ammo");
	AmmoTarget.TargetObjectPath = AmmoObjectPath;
	AmmoTarget.DesiredFingerprint = AmmoFingerprint;
	AmmoTarget.BindingMode = ECFProdBindingMode::CreateNew;
	TestTrue(
		TEXT("Ammo target binds to existing Ammo provider"),
		FCFProdTargetBindings::BindAmmo(
			Adapter,
			AmmoTarget,
			WeaponDraft,
			BindingError));

	// Projectile reviewed target contract입니다.
	FCFProdProvisionTarget ProjectileTarget;
	ProjectileTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Projectile"),
			TEXT("BindingProjectile"));
	ProjectileTarget.RoleId = TEXT("Projectile");
	ProjectileTarget.TargetObjectPath = ProjectileObjectPath;
	ProjectileTarget.DesiredFingerprint = ProjectileFingerprint;
	ProjectileTarget.BindingMode = ECFProdBindingMode::CreateNew;
	TestTrue(
		TEXT("Projectile target binds to existing Weapon Guide backend"),
		FCFProdTargetBindings::BindProjectile(
			Adapter,
			ProjectileTarget,
			WeaponDraft,
			DamageObjectPath,
			BindingError));

	// Weapon reviewed target contract입니다.
	FCFProdProvisionTarget WeaponTarget;
	WeaponTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Weapon"),
			TEXT("BindingWeapon"));
	WeaponTarget.RoleId = TEXT("Weapon");
	WeaponTarget.TargetObjectPath = WeaponObjectPath;
	WeaponTarget.DesiredFingerprint = WeaponFingerprint;
	WeaponTarget.BindingMode = ECFProdBindingMode::CreateNew;
	TestTrue(
		TEXT("Weapon target binds to existing Weapon Guide backend"),
		FCFProdTargetBindings::BindWeapon(
			Adapter,
			WeaponTarget,
			WeaponDraft,
			ProjectileObjectPath,
			AmmoObjectPath,
			TEXT("BindingAmmo"),
			BindingError));

	// EquipmentPreset reviewed target contract입니다.
	FCFProdProvisionTarget EquipmentTarget;
	EquipmentTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("EquipmentPreset"),
			TEXT("BindingEquip"));
	EquipmentTarget.RoleId = TEXT("Product");
	EquipmentTarget.TargetObjectPath = EquipmentObjectPath;
	EquipmentTarget.DesiredFingerprint =
		EquipmentSnapshot.SemanticFingerprint;
	EquipmentTarget.BindingMode = ECFProdBindingMode::CreateNew;
	TestTrue(
		TEXT("EquipmentPreset target binds to existing Equipment durable backend"),
		FCFProdTargetBindings::BindEquipmentPreset(
			Adapter,
			EquipmentTarget,
			EquipmentDraft,
			BindingError));

	TestEqual(
		TEXT("Concrete backend binding exact6"),
		Adapter.GetBindingCount(),
		6);

	// 아직 durable asset을 만들지 않았으므로 registered backend read는 absent를 성공적으로 보고해야 합니다.
	TArray<FCFProdProvisionTarget> BoundTargets = {
		TurretTarget,
		DamageTarget,
		AmmoTarget,
		ProjectileTarget,
		WeaponTarget,
		EquipmentTarget
	};
	for (const FCFProdProvisionTarget& BoundTarget : BoundTargets)
	{
		// Persisted target 존재 여부입니다.
		bool bExists = true;
		// Persisted target fingerprint입니다.
		FString PersistedFingerprint;
		// Read callback diagnostic입니다.
		FString ReadError;
		TestTrue(
			*FString::Printf(
				TEXT("Registered backend read succeeds for %s"),
				*BoundTarget.ContentKey.ToStableString()),
			Adapter.ReadPersistedFingerprint(
				BoundTarget,
				bExists,
				PersistedFingerprint,
				ReadError));
		TestFalse(
			*FString::Printf(
				TEXT("Binding fixture remains absent for %s"),
				*BoundTarget.ContentKey.ToStableString()),
			bExists);
	}

	// Reviewed desired fingerprint가 backend truth와 다르면 binding 자체가 mutation 전에 거부되어야 합니다.
	FCFProdProvisionTarget MismatchedDamageTarget = DamageTarget;
	MismatchedDamageTarget.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Damage"),
			TEXT("BindingDamageMismatch"));
	MismatchedDamageTarget.TargetObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_BindingDamageMismatch.DA_BindingDamageMismatch");
	MismatchedDamageTarget.DesiredFingerprint =
		CFProdProvisioningTestsPrivate::HashText(
			TEXT("intentionally-mismatched"));
	TestFalse(
		TEXT("Mismatched reviewed fingerprint is rejected before binding"),
		FCFProdTargetBindings::BindDamage(
			Adapter,
			MismatchedDamageTarget,
			WeaponDraft,
			BindingError));
	TestEqual(
		TEXT("Rejected binding does not change registry size"),
		Adapter.GetBindingCount(),
		6);

	// Registered target의 object path가 drift하면 same ContentKey라도 callback을 호출하지 않습니다.
	FCFProdProvisionTarget DriftedWeaponTarget = WeaponTarget;
	DriftedWeaponTarget.TargetObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/Bindings/DA_DriftedWeapon.DA_DriftedWeapon");
	bool bDriftExists = false;
	FString DriftFingerprint;
	FString DriftError;
	TestFalse(
		TEXT("Target object-path drift is rejected before backend read"),
		Adapter.ReadPersistedFingerprint(
			DriftedWeaponTarget,
			bDriftExists,
			DriftFingerprint,
			DriftError));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdTransactionFileStoreTest,
	"CarFight.ContentAuthoring.P007.ProductionBridge.TransactionFileStore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Production transaction v3가 approval/execution identity, composite impacted Product key와 exact target evidence를 durable JSON round-trip하는지 검증합니다.
bool FCFProdTransactionFileStoreTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Disposable Saved transaction identity입니다.
	const FString TransactionId = TEXT("P007ProdTxnFileStoreTest");
	// Disposable transaction physical path입니다.
	const FString TransactionPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS"),
		TEXT("ProductionTransactions"),
		TransactionId + TEXT(".json"));
	// Production file store입니다.
	FCFProdTransactionFileStore FileStore;

	// 이전 interrupted test residue를 현재 disposable exact file에 한해 정리합니다.
	IFileManager::Get().Delete(*TransactionPath, false, true, true);

	// Durable round-trip source transaction입니다.
	FCFProdTransaction SourceTransaction;
	SourceTransaction.TransactionId = TransactionId;
	SourceTransaction.ProductKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("EquipmentPreset"),
			TEXT("ProdCannonStandard"));
	SourceTransaction.ReviewPackageFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-review"));
	SourceTransaction.BaseCatalogSnapshotFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-base"));
	SourceTransaction.ResourceCatalogFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-resources"));
	SourceTransaction.ExecutionBindingFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-execution"));
	SourceTransaction.WorkbookSemanticHash =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-workbook"));
	SourceTransaction.RoleTopologyFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-roles"));
	SourceTransaction.State = ECFProdTxnState::RecoveryRequired;
	SourceTransaction.ProductGraphFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-graph"));
	SourceTransaction.Diagnostic = TEXT("fixture recovery");
	SourceTransaction.ImpactedProductKeys.Add(SourceTransaction.ProductKey);

	// Durable target evidence fixture입니다.
	FCFProdTargetEvidence TargetEvidence;
	TargetEvidence.ContentKey =
		CFProdProvisioningTestsPrivate::MakeKey(
			TEXT("Weapon"),
			TEXT("ProdChildA"));
	TargetEvidence.TargetObjectPath =
		TEXT("/Game/CarFight/Tests/CCASP007/DA_ProdChildA.DA_ProdChildA");
	TargetEvidence.DesiredFingerprint =
		CFProdProvisioningTestsPrivate::HashText(TEXT("file-store-child"));
	TargetEvidence.bRequireAbsentAtReview = true;
	TargetEvidence.PersistedFingerprint = TargetEvidence.DesiredFingerprint;
	TargetEvidence.bMutationPerformed = true;
	SourceTransaction.Targets.Add(TargetEvidence);

	// Durable save/load diagnostics입니다.
	FString Error;
	TestTrue(
		TEXT("Production transaction v3 durable save succeeds"),
		FileStore.SaveTransaction(SourceTransaction, Error));

	// Reload existence flag입니다.
	bool bFound = false;
	// Reloaded transaction evidence입니다.
	FCFProdTransaction ReloadedTransaction;
	TestTrue(
		TEXT("Production transaction v3 durable load succeeds"),
		FileStore.LoadTransaction(
			TransactionId,
			bFound,
			ReloadedTransaction,
			Error));
	TestTrue(TEXT("Production transaction v3 file is found"), bFound);
	TestTrue(
		TEXT("Product composite ContentKey round-trips"),
		ReloadedTransaction.ProductKey == SourceTransaction.ProductKey);
	TestEqual(
		TEXT("Impacted Product composite key count exact1"),
		ReloadedTransaction.ImpactedProductKeys.Num(),
		1);
	if (ReloadedTransaction.ImpactedProductKeys.Num() == 1)
	{
		TestTrue(
			TEXT("Impacted Product composite ContentKey round-trips"),
			ReloadedTransaction.ImpactedProductKeys[0] == SourceTransaction.ProductKey);
	}
	TestEqual(
		TEXT("Target evidence count exact1"),
		ReloadedTransaction.Targets.Num(),
		1);
	if (ReloadedTransaction.Targets.Num() == 1)
	{
		TestEqual(
			TEXT("Target object path round-trips exactly"),
			ReloadedTransaction.Targets[0].TargetObjectPath,
			TargetEvidence.TargetObjectPath);
		TestEqual(
			TEXT("Target desired fingerprint round-trips exactly"),
			ReloadedTransaction.Targets[0].DesiredFingerprint,
			TargetEvidence.DesiredFingerprint);
		TestEqual(
			TEXT("Target persisted fingerprint round-trips exactly"),
			ReloadedTransaction.Targets[0].PersistedFingerprint,
			TargetEvidence.PersistedFingerprint);
		TestTrue(
			TEXT("Target mutation evidence round-trips"),
			ReloadedTransaction.Targets[0].bMutationPerformed);
	}

	// Disposable test file만 정리합니다.
	TestTrue(
		TEXT("Production transaction disposable file cleanup succeeds"),
		!IFileManager::Get().FileExists(*TransactionPath)
			|| IFileManager::Get().Delete(*TransactionPath, false, true, true));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
