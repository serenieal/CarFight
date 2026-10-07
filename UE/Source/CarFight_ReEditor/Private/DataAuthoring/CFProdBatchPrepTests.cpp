// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdBatchPrepTests.cpp
// Version: v1.2.0
// Date: 2026-10-06
// Description: CF-FQ-058 First Production Wave exact34 candidate preparation focused Automation입니다.
// Changelog:
// - v1.2.0: exact8 typed execution manifest approval binding, exact34 backend desired target identity, Guided PFP-complete readiness와 tamper rejection을 추가.
// - v1.1.0: persisted resource exact12, socket capability exact8, ReviewReady와 current Cannon/Rocket turret values/muzzle order 검증을 추가.
// - v1.0.0: exact8 Product / exact48 Role Binding / exact34 unique target / finite-ammo exact8,
//   immutable Review Package, canonical Production path와 no-write/no-cutover invariant를 최초 검증.
// Migration:
// - 이 테스트는 read-only preparation만 호출하며 Production DataAsset, canonical Workbook, Catalog publication을 생성하지 않습니다.

#include "DataAuthoring/CFProdBatchPrep.h"

#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFProdTargetBindings.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

namespace CFProdBatchPrepTestsPrivate
{
	// exact ContentType + ContentId의 candidate record를 찾습니다.
	const FCFContentRecord* FindRecord(
		const FCFContentWorkbookModel& Workbook,
		const FString& ContentTypeId,
		const FString& ContentId)
	{
		return Workbook.Records.FindByPredicate(
			[&ContentTypeId, &ContentId](const FCFContentRecord& Record)
			{
				return Record.Key.ContentTypeId.Value.Equals(
						ContentTypeId,
						ESearchCase::CaseSensitive)
					&& Record.Key.ContentId.Equals(
						ContentId,
						ESearchCase::CaseSensitive);
			});
	}

	// exact keyed child item을 DomainKey로 찾습니다.
	const FCFContentCollectionItem* FindCollectionItem(
		const FCFContentRecord& Record,
		const FString& CollectionId,
		const FString& DomainKey)
	{
		// 대상 collection입니다.
		const FCFContentCollection* Collection =
			Record.Collections.Find(CollectionId);
		if (Collection == nullptr)
		{
			return nullptr;
		}

		return Collection->Items.FindByPredicate(
			[&DomainKey](const FCFContentCollectionItem& Item)
			{
				return Item.DomainKey.Equals(
					DomainKey,
					ESearchCase::CaseSensitive);
			});
	}

	// child item의 exact string-like VALUE를 읽습니다.
	bool TryGetStringValue(
		const FCFContentCollectionItem& Item,
		const FString& FieldId,
		FString& OutValue)
	{
		// 대상 field value입니다.
		const FCFContentValue* Value = Item.Fields.Find(FieldId);
		if (Value == nullptr
			|| Value->State != ECFContentValueState::Value)
		{
			return false;
		}

		OutValue = Value->StringValue;
		return true;
	}

	// record의 exact string-like VALUE를 읽습니다.
	bool TryGetRecordStringValue(
		const FCFContentRecord& Record,
		const FString& FieldId,
		FString& OutValue)
	{
		// 대상 field value입니다.
		const FCFContentValue* Value = Record.Fields.Find(FieldId);
		if (Value == nullptr
			|| Value->State != ECFContentValueState::Value)
		{
			return false;
		}

		OutValue = Value->StringValue;
		return true;
	}

	// child item의 exact signed integer VALUE를 읽습니다.
	bool TryGetSignedIntegerValue(
		const FCFContentCollectionItem& Item,
		const FString& FieldId,
		int64& OutValue)
	{
		// 대상 field value입니다.
		const FCFContentValue* Value = Item.Fields.Find(FieldId);
		if (Value == nullptr
			|| Value->State != ECFContentValueState::Value
			|| Value->Type != ECFContentValueType::SignedInteger)
		{
			return false;
		}

		OutValue = Value->SignedIntegerValue;
		return true;
	}

	// record의 exact double VALUE를 읽습니다.
	bool TryGetRecordDoubleValue(
		const FCFContentRecord& Record,
		const FString& FieldId,
		double& OutValue)
	{
		// 대상 field value입니다.
		const FCFContentValue* Value = Record.Fields.Find(FieldId);
		if (Value == nullptr
			|| Value->State != ECFContentValueState::Value
			|| Value->Type != ECFContentValueType::Double)
		{
			return false;
		}

		OutValue = Value->FloatingPointValue;
		return true;
	}

	// record의 exact boolean VALUE를 읽습니다.
	bool TryGetRecordBoolValue(
		const FCFContentRecord& Record,
		const FString& FieldId,
		bool& bOutValue)
	{
		// 대상 field value입니다.
		const FCFContentValue* Value = Record.Fields.Find(FieldId);
		if (Value == nullptr
			|| Value->State != ECFContentValueState::Value
			|| Value->Type != ECFContentValueType::Boolean)
		{
			return false;
		}

		bOutValue = Value->bBooleanValue;
		return true;
	}

	// OrderedList collection의 semantic values를 explicit Order 기준으로 읽습니다.
	TArray<FString> ReadOrderedStringValues(
		const FCFContentRecord& Record,
		const FString& CollectionId,
		const FString& FieldId)
	{
		TArray<FString> Values;

		// 대상 ordered collection입니다.
		const FCFContentCollection* Collection =
			Record.Collections.Find(CollectionId);
		if (Collection == nullptr
			|| Collection->Kind != ECFContentCollectionKind::OrderedList)
		{
			return Values;
		}

		// Physical array order와 무관하게 semantic Order로 정렬할 item refs입니다.
		TArray<const FCFContentCollectionItem*> OrderedItems;
		for (const FCFContentCollectionItem& Item : Collection->Items)
		{
			OrderedItems.Add(&Item);
		}
		OrderedItems.Sort(
			[](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
			{
				return Left.Order < Right.Order;
			});

		for (const FCFContentCollectionItem* Item : OrderedItems)
		{
			if (Item == nullptr)
			{
				continue;
			}
			FString Value;
			if (TryGetStringValue(*Item, FieldId, Value))
			{
				Values.Add(Value);
			}
		}
		return Values;
	}

	// exact ContentKey target manifest entry를 찾습니다.
	const FCFProdCandidateTarget* FindTarget(
		const TArray<FCFProdCandidateTarget>& Targets,
		const FString& ContentTypeId,
		const FString& ContentId)
	{
		return Targets.FindByPredicate(
			[&ContentTypeId, &ContentId](const FCFProdCandidateTarget& Target)
			{
				return Target.ContentKey.ContentTypeId.Value.Equals(
						ContentTypeId,
						ESearchCase::CaseSensitive)
					&& Target.ContentKey.ContentId.Equals(
						ContentId,
						ESearchCase::CaseSensitive);
			});
	}

	// Product Role Binding의 exact BindingMode를 읽습니다.
	bool TryGetBindingMode(
		const FCFContentRecord& ProductRecord,
		const FString& RoleId,
		FString& OutBindingMode)
	{
		// RoleBindings에서 exact role item을 찾습니다.
		const FCFContentCollectionItem* RoleItem =
			FindCollectionItem(
				ProductRecord,
				TEXT("RoleBindings"),
				RoleId);
		if (RoleItem == nullptr)
		{
			return false;
		}

		return TryGetStringValue(
			*RoleItem,
			TEXT("BindingMode"),
			OutBindingMode);
	}

	// EquipmentPreset의 explicit sortie ammo count를 읽습니다.
	bool TryGetSortieAmmoCount(
		const FCFContentRecord& EquipmentRecord,
		const FString& AmmoContentId,
		int64& OutCount)
	{
		// exact ammo row입니다.
		const FCFContentCollectionItem* AmmoItem =
			FindCollectionItem(
				EquipmentRecord,
				TEXT("EquipmentPresetAmmoLoads"),
				AmmoContentId);
		if (AmmoItem == nullptr)
		{
			return false;
		}

		return TryGetSignedIntegerValue(
			*AmmoItem,
			TEXT("DefaultSortieAmmoCount"),
			OutCount);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdBatchCandidatePreparationTest,
	"CarFight.ContentAuthoring.P007.ProductionBatch.CandidatePreparation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// First Production Wave exact34 candidate가 current Roster와 no-write review contract를 만족하는지 검증합니다.
bool FCFProdBatchCandidatePreparationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Read-only candidate preparation 결과입니다.
	FCFProdBatchPrepResult Result;

	// Candidate preparation 실패 사유입니다.
	FString Error;

	// Candidate preparation 실행 결과입니다.
	const bool bPrepared =
		FCFProdBatchPrep::PrepareFirstWaveCandidate(Result, Error);
	TestTrue(TEXT("Candidate preparation succeeds"), bPrepared);
	TestTrue(TEXT("Candidate preparation error is empty"), Error.IsEmpty());
	if (!bPrepared)
	{
		return false;
	}

	// Persisted Resource Catalog + socket capability proof까지 완료된 fresh ReviewReady인지 검증합니다.
	TestEqual(
		TEXT("State is ReviewReady after persisted resource proof"),
		Result.State,
		ECFProdBatchPrepState::ReviewReady);

	// Frozen exact count contract를 검증합니다.
	TestEqual(TEXT("Product count exact8"), Result.ProductCount, 8);
	TestEqual(TEXT("Role Binding count exact48"), Result.RoleBindingCount, 48);
	TestEqual(TEXT("Unique Production target count exact34"), Result.UniqueDataAssetCount, 34);
	TestEqual(TEXT("Explicit sortie ammo row count exact8"), Result.AmmoLoadCount, 8);
	TestEqual(
		TEXT("Review Workbook record count exact42 including Product envelopes"),
		Result.ReviewPackage.Preview.Workbook.Records.Num(),
		42);

	// Immutable review identities가 canonical SHA-256인지 검증합니다.
	TestTrue(
		TEXT("ReviewPackageFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Result.ReviewPackage.ReviewPackageFingerprint));
	TestTrue(
		TEXT("CandidateWorkbookSemanticHash canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Result.CandidateWorkbookSemanticHash));
	TestEqual(
		TEXT("Candidate semantic hash equals Review Package expected post hash"),
		Result.CandidateWorkbookSemanticHash,
		Result.ReviewPackage.ExpectedPostSemanticHash);

	// Preparation path가 persistent write/cutover를 열지 않았는지 검증합니다.
	TestTrue(
		TEXT("Review Package no persistent Workbook write guard"),
		Result.ReviewPackage.bNoPersistentWorkbookWriteGuardPassed);
	TestTrue(
		TEXT("Review Package no direct DataAsset write guard"),
		Result.ReviewPackage.bNoDirectDataAssetWriteGuardPassed);
	TestFalse(
		TEXT("Canonical Workbook write exact0"),
		Result.bCanonicalWorkbookWritePerformed);
	TestFalse(
		TEXT("Production DataAsset mutation exact0"),
		Result.bProductionMutationPerformed);
	TestFalse(
		TEXT("Authority cutover exact0"),
		Result.bAuthorityCutoverApplied);
	TestEqual(
		TEXT("Missing required Resource bindings exact0"),
		Result.MissingRequiredResourceBindings.Num(),
		0);
	TestEqual(TEXT("Resolved persisted resource count exact12"), Result.ResolvedResourceCount, 12);
	TestEqual(TEXT("Required socket capability check count exact8"), Result.SocketCapabilityCheckCount, 8);
	TestTrue(
		TEXT("ResourceCatalogFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Result.ResourceCatalogFingerprint));
	TestTrue(
		TEXT("ExecutionBindingFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Result.ExecutionBindingFingerprint));
	TestEqual(
		TEXT("Review Package binds exact execution manifest"),
		Result.ReviewPackage.ExecutionBindingFingerprint,
		Result.ExecutionBindingFingerprint);
	TestEqual(
		TEXT("Prepared Product exact8"),
		Result.PreparedProducts.Num(),
		8);
	TestEqual(
		TEXT("Prepared execution unique target exact34"),
		Result.UniqueExecutionTargetCount,
		34);

	// exact8 request가 같은 approved review/base/resource/execution identity를 소유하는지 검증합니다.
	TSet<FString> PreparedTargetKeys;
	TMap<FString, FString> DesiredFingerprintByKey;
	for (const FCFProdPreparedProduct& Prepared : Result.PreparedProducts)
	{
		TestEqual(
			TEXT("Prepared request binds final ReviewPackageFingerprint"),
			Prepared.Request.ReviewPackageFingerprint,
			Result.ReviewPackage.ReviewPackageFingerprint);
		TestEqual(
			TEXT("Prepared request binds base Catalog snapshot"),
			Prepared.Request.BaseCatalogSnapshotFingerprint,
			Result.ReviewPackage.BaseCatalogSnapshotFingerprint);
		TestEqual(
			TEXT("Prepared request binds Resource Catalog"),
			Prepared.Request.ResourceCatalogFingerprint,
			Result.ResourceCatalogFingerprint);
		TestEqual(
			TEXT("Prepared request binds execution manifest"),
			Prepared.Request.ExecutionBindingFingerprint,
			Result.ExecutionBindingFingerprint);
		TestEqual(
			TEXT("Prepared request binds reviewed post Workbook"),
			Prepared.Request.WorkbookSemanticHash,
			Result.ReviewPackage.ExpectedPostSemanticHash);
		TestEqual(
			TEXT("Prepared Product has exact6 typed targets"),
			Prepared.Request.Targets.Num(),
			6);

		for (const FCFProdProvisionTarget& Target : Prepared.Request.Targets)
		{
			TestTrue(
				TEXT("Prepared target desired fingerprint canonical"),
				CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
					Target.DesiredFingerprint));
			PreparedTargetKeys.Add(Target.ContentKey.ToStableString());

			const FString* ExistingDesired =
				DesiredFingerprintByKey.Find(Target.ContentKey.ToStableString());
			if (ExistingDesired != nullptr)
			{
				TestEqual(
					TEXT("Shared target desired fingerprint is exact single-writer payload"),
					*ExistingDesired,
					Target.DesiredFingerprint);
			}
			else
			{
				DesiredFingerprintByKey.Add(
					Target.ContentKey.ToStableString(),
					Target.DesiredFingerprint);
			}
		}
	}
	TestEqual(
		TEXT("Prepared request target union exact34"),
		PreparedTargetKeys.Num(),
		34);

	// exact8 prepared drafts가 concrete existing typed backend binding으로 unique exact34 target을 그대로 결속하는지 검증합니다.
	FCFProdBoundTargetAdapter BoundAdapter;
	TSet<FString> RegisteredBindingKeys;
	for (const FCFProdPreparedProduct& Prepared : Result.PreparedProducts)
	{
		const FCFProdProvisionTarget* DamageTarget =
			Prepared.Request.Targets.FindByPredicate(
				[](const FCFProdProvisionTarget& Target)
				{
					return Target.RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive);
				});
		const FCFProdProvisionTarget* ProjectileTarget =
			Prepared.Request.Targets.FindByPredicate(
				[](const FCFProdProvisionTarget& Target)
				{
					return Target.RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive);
				});
		const FCFProdProvisionTarget* AmmoTarget =
			Prepared.Request.Targets.FindByPredicate(
				[](const FCFProdProvisionTarget& Target)
				{
					return Target.RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive);
				});

		for (const FCFProdProvisionTarget& Target : Prepared.Request.Targets)
		{
			const FString StableKey = Target.ContentKey.ToStableString();
			if (RegisteredBindingKeys.Contains(StableKey))
			{
				continue;
			}

			FString BindingError;
			bool bBound = false;
			if (Target.RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
			{
				bBound = FCFProdTargetBindings::BindTurret(
					BoundAdapter,
					Target,
					Prepared.WeaponDraft,
					BindingError);
			}
			else if (Target.RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
			{
				bBound = FCFProdTargetBindings::BindDamage(
					BoundAdapter,
					Target,
					Prepared.WeaponDraft,
					BindingError);
			}
			else if (Target.RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive))
			{
				bBound = FCFProdTargetBindings::BindAmmo(
					BoundAdapter,
					Target,
					Prepared.WeaponDraft,
					BindingError);
			}
			else if (Target.RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive)
				&& DamageTarget != nullptr)
			{
				bBound = FCFProdTargetBindings::BindProjectile(
					BoundAdapter,
					Target,
					Prepared.WeaponDraft,
					DamageTarget->TargetObjectPath,
					BindingError);
			}
			else if (Target.RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive)
				&& ProjectileTarget != nullptr
				&& AmmoTarget != nullptr)
			{
				bBound = FCFProdTargetBindings::BindWeapon(
					BoundAdapter,
					Target,
					Prepared.WeaponDraft,
					ProjectileTarget->TargetObjectPath,
					AmmoTarget->TargetObjectPath,
					FName(*AmmoTarget->ContentKey.ContentId),
					BindingError);
			}
			else if (Target.RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
			{
				bBound = FCFProdTargetBindings::BindEquipmentPreset(
					BoundAdapter,
					Target,
					Prepared.EquipmentDraft,
					BindingError);
			}

			TestTrue(
				*FString::Printf(
					TEXT("Prepared typed backend binding succeeds: %s"),
					*StableKey),
				bBound);
			TestTrue(
				*FString::Printf(
					TEXT("Prepared typed backend binding error is empty: %s"),
					*StableKey),
				BindingError.IsEmpty());
			if (bBound)
			{
				RegisteredBindingKeys.Add(StableKey);
			}
		}
	}
	TestEqual(
		TEXT("Prepared concrete typed backend binding exact34"),
		BoundAdapter.GetBindingCount(),
		34);

	// Execution manifest payload를 바꾸면 기존 approved ReviewPackageFingerprint를 재사용할 수 없습니다.
	FCFContentReviewPackage TamperedReview = Result.ReviewPackage;
	TamperedReview.ExecutionBindingFingerprint =
		Result.ResourceCatalogFingerprint;
	FCFContentReviewApproval OriginalApproval;
	OriginalApproval.bApproved = true;
	OriginalApproval.ReviewPackageFingerprint =
		Result.ReviewPackage.ReviewPackageFingerprint;
	TArray<FCFContentValidationIssue> TamperIssues;
	TestFalse(
		TEXT("Execution manifest tamper invalidates original review approval"),
		FCFContentPlanningService::ValidateApproval(
			TamperedReview,
			OriginalApproval,
			TamperIssues));

	// Managed cutover 직전 사용할 approval→typed execution exact binding 재검증입니다.
	FString ApprovalBindingError;
	TestTrue(
		TEXT("Approved Review Package validates exact prepared typed execution batch"),
		FCFProdBatchPrep::ValidatePreparedBatchApproval(
			Result,
			OriginalApproval,
			ApprovalBindingError));
	TestTrue(
		TEXT("Approved prepared batch validation error is empty"),
		ApprovalBindingError.IsEmpty());

	FCFProdBatchPrepResult TamperedExecution = Result;
	TamperedExecution.PreparedProducts[0].Request.Targets[0].DesiredFingerprint =
		Result.ResourceCatalogFingerprint;
	TestFalse(
		TEXT("Typed desired payload tamper is rejected before durable execution"),
		FCFProdBatchPrep::ValidatePreparedBatchApproval(
			TamperedExecution,
			OriginalApproval,
			ApprovalBindingError));

	// exact34 manifest ContentKey uniqueness를 검증합니다.
	TSet<FString> UniqueTargetKeys;

	// exact34 manifest canonical object path uniqueness를 검증합니다.
	TSet<FString> UniqueObjectPaths;
	for (const FCFProdCandidateTarget& Target : Result.CandidateTargets)
	{
		UniqueTargetKeys.Add(Target.ContentKey.ToStableString());
		UniqueObjectPaths.Add(Target.TargetObjectPath);
	}
	TestEqual(TEXT("Unique target ContentKey exact34"), UniqueTargetKeys.Num(), 34);
	TestEqual(TEXT("Unique target object path exact34"), UniqueObjectPaths.Num(), 34);

	// Cannon shared mount target입니다.
	const FCFProdCandidateTarget* CannonMount =
		CFProdBatchPrepTestsPrivate::FindTarget(
			Result.CandidateTargets,
			TEXT("TurretMount"),
			TEXT("Mount_Cannon"));
	TestNotNull(TEXT("Mount_Cannon target exists"), CannonMount);
	if (CannonMount != nullptr)
	{
		TestTrue(TEXT("Mount_Cannon is shared"), CannonMount->bShared);
		TestEqual(
			TEXT("Mount_Cannon canonical Production path"),
			CannonMount->TargetObjectPath,
			FString(TEXT("/Game/CarFight/Weapons/Data/Production/TurretMounts/DA_Mount_Cannon.DA_Mount_Cannon")));
	}

	// Shared Rocket projectile target입니다.
	const FCFProdCandidateTarget* RocketProjectile =
		CFProdBatchPrepTestsPrivate::FindTarget(
			Result.CandidateTargets,
			TEXT("Projectile"),
			TEXT("Projectile_Rocket"));
	TestNotNull(TEXT("Projectile_Rocket target exists"), RocketProjectile);
	if (RocketProjectile != nullptr)
	{
		TestTrue(TEXT("Projectile_Rocket is shared"), RocketProjectile->bShared);
	}

	// Guided missile own projectile target입니다.
	const FCFProdCandidateTarget* GuidedProjectile =
		CFProdBatchPrepTestsPrivate::FindTarget(
			Result.CandidateTargets,
			TEXT("Projectile"),
			TEXT("Projectile_GuidedMissile_Standard"));
	TestNotNull(TEXT("Guided own projectile target exists"), GuidedProjectile);
	if (GuidedProjectile != nullptr)
	{
		TestFalse(TEXT("Guided projectile is variant-owned"), GuidedProjectile->bShared);
	}

	// Review Package Workbook입니다.
	const FCFContentWorkbookModel& Workbook =
		Result.ReviewPackage.Preview.Workbook;

	// Cannon Standard Product envelope입니다.
	const FCFContentRecord* CannonStandard =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("Weapon"),
			TEXT("Cannon_Standard"));

	// Cannon LongRange Product envelope입니다.
	const FCFContentRecord* CannonLongRange =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("Weapon"),
			TEXT("Cannon_LongRange"));

	// Rocket Salvo Product envelope입니다.
	const FCFContentRecord* RocketSalvo =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("Weapon"),
			TEXT("Rocket_Salvo"));

	// Guided missile Product envelope입니다.
	const FCFContentRecord* GuidedProduct =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("Weapon"),
			TEXT("GuidedMissile_Standard"));

	TestNotNull(TEXT("Cannon Standard Product exists"), CannonStandard);
	TestNotNull(TEXT("Cannon LongRange Product exists"), CannonLongRange);
	TestNotNull(TEXT("Rocket Salvo Product exists"), RocketSalvo);
	TestNotNull(TEXT("Guided Product exists"), GuidedProduct);

	if (CannonStandard != nullptr)
	{
		// Cannon Standard Mount binding mode입니다.
		FString MountBindingMode;
		TestTrue(
			TEXT("Cannon Standard Mount binding readable"),
			CFProdBatchPrepTestsPrivate::TryGetBindingMode(
				*CannonStandard,
				TEXT("Mount"),
				MountBindingMode));
		TestEqual(
			TEXT("Cannon Standard owns shared Mount writer"),
			MountBindingMode,
			FString(TEXT("CreateNewShared")));
	}

	if (CannonLongRange != nullptr)
	{
		// Cannon LongRange Damage binding mode입니다.
		FString DamageBindingMode;

		// Cannon LongRange Ammo binding mode입니다.
		FString AmmoBindingMode;
		TestTrue(
			TEXT("Cannon LongRange Damage binding readable"),
			CFProdBatchPrepTestsPrivate::TryGetBindingMode(
				*CannonLongRange,
				TEXT("Damage"),
				DamageBindingMode));
		TestTrue(
			TEXT("Cannon LongRange Ammo binding readable"),
			CFProdBatchPrepTestsPrivate::TryGetBindingMode(
				*CannonLongRange,
				TEXT("Ammo"),
				AmmoBindingMode));
		TestEqual(
			TEXT("Cannon LongRange binds Standard Damage"),
			DamageBindingMode,
			FString(TEXT("BindShared")));
		TestEqual(
			TEXT("Cannon LongRange binds Standard Ammo"),
			AmmoBindingMode,
			FString(TEXT("BindShared")));
	}

	if (RocketSalvo != nullptr)
	{
		// Rocket Salvo Projectile binding mode입니다.
		FString ProjectileBindingMode;
		TestTrue(
			TEXT("Rocket Salvo Projectile binding readable"),
			CFProdBatchPrepTestsPrivate::TryGetBindingMode(
				*RocketSalvo,
				TEXT("Projectile"),
				ProjectileBindingMode));
		TestEqual(
			TEXT("Rocket Salvo binds shared Projectile"),
			ProjectileBindingMode,
			FString(TEXT("BindShared")));
	}

	if (GuidedProduct != nullptr)
	{
		// Guided Mount binding mode입니다.
		FString MountBindingMode;

		// Guided Projectile binding mode입니다.
		FString ProjectileBindingMode;
		TestTrue(
			TEXT("Guided Mount binding readable"),
			CFProdBatchPrepTestsPrivate::TryGetBindingMode(
				*GuidedProduct,
				TEXT("Mount"),
				MountBindingMode));
		TestTrue(
			TEXT("Guided Projectile binding readable"),
			CFProdBatchPrepTestsPrivate::TryGetBindingMode(
				*GuidedProduct,
				TEXT("Projectile"),
				ProjectileBindingMode));
		TestEqual(
			TEXT("Guided binds shared Rocket pod Mount"),
			MountBindingMode,
			FString(TEXT("BindShared")));
		TestEqual(
			TEXT("Guided owns new Projectile"),
			ProjectileBindingMode,
			FString(TEXT("CreateNew")));

		// CF-FQ-056 PFP-P0-03 USER ACCEPTED / COMPLETE가 current candidate planning truth에 반영됐는지 확인합니다.
		FString GuidedReadiness;
		TestTrue(
			TEXT("Guided readiness readable"),
			CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(
				*GuidedProduct,
				TEXT("Readiness"),
				GuidedReadiness));
		TestEqual(
			TEXT("Guided is AuthoringReady after PFP USER acceptance"),
			GuidedReadiness,
			FString(TEXT("AuthoringReady")));
		TestEqual(
			TEXT("Guided typed planning readiness is AuthoringReady"),
			GuidedProduct->AuthoringMetadata.Planning.Readiness,
			ECFContentPlanningReadiness::AuthoringReady);
		TestEqual(
			TEXT("Guided stale PFP dependency exact0"),
			GuidedProduct->AuthoringMetadata.Planning.TechnologyDependencies.Num(),
			0);
	}

	// Product별 current explicit sortie ammo frozen values입니다.
	struct FExpectedAmmoLoad
	{
		// Product identity입니다.
		const TCHAR* ProductId;

		// Ammo identity입니다.
		const TCHAR* AmmoId;

		// Expected sortie count입니다.
		int64 ExpectedCount;
	};

	// Current Roster exact8 sortie ammo expectations입니다.
	const FExpectedAmmoLoad AmmoExpectations[] = {
		{ TEXT("Cannon_Standard"), TEXT("Ammo_Cannon_Standard"), 40 },
		{ TEXT("Cannon_Heavy"), TEXT("Ammo_Cannon_Heavy"), 20 },
		{ TEXT("Cannon_LongRange"), TEXT("Ammo_Cannon_Standard"), 40 },
		{ TEXT("Cannon_Rapid"), TEXT("Ammo_Cannon_Rapid"), 120 },
		{ TEXT("Rocket_Standard"), TEXT("Ammo_Rocket"), 32 },
		{ TEXT("Rocket_Salvo"), TEXT("Ammo_Rocket"), 32 },
		{ TEXT("Rocket_Ripple"), TEXT("Ammo_Rocket"), 32 },
		{ TEXT("GuidedMissile_Standard"), TEXT("Ammo_GuidedMissile_Standard"), 10 }
	};

	for (const FExpectedAmmoLoad& Expectation : AmmoExpectations)
	{
		// Product EquipmentPreset record입니다.
		const FCFContentRecord* EquipmentRecord =
			CFProdBatchPrepTestsPrivate::FindRecord(
				Workbook,
				TEXT("EquipmentPreset"),
				Expectation.ProductId);
		TestNotNull(
			*FString::Printf(
				TEXT("%s EquipmentPreset exists"),
				Expectation.ProductId),
			EquipmentRecord);
		if (EquipmentRecord == nullptr)
		{
			continue;
		}

		// Readback sortie ammo count입니다.
		int64 ReadbackCount = INDEX_NONE;
		TestTrue(
			*FString::Printf(
				TEXT("%s sortie ammo row readable"),
				Expectation.ProductId),
			CFProdBatchPrepTestsPrivate::TryGetSortieAmmoCount(
				*EquipmentRecord,
				Expectation.AmmoId,
				ReadbackCount));
		TestEqual(
			*FString::Printf(
				TEXT("%s current sortie ammo count"),
				Expectation.ProductId),
			ReadbackCount,
			Expectation.ExpectedCount);
	}

	// Persisted current Cannon/Rocket turret resource + gameplay values를 검증합니다.
	const FCFContentRecord* CannonTurret =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("TurretMount"),
			TEXT("Mount_Cannon"));
	const FCFContentRecord* RocketTurret =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("TurretMount"),
			TEXT("Mount_RocketPod"));
	TestNotNull(TEXT("Cannon Production turret record exists"), CannonTurret);
	TestNotNull(TEXT("Rocket Production turret record exists"), RocketTurret);

	if (CannonTurret != nullptr)
	{
		FString BaseResourceId;
		FString YawResourceId;
		FString PitchResourceId;
		double YawRate = 0.0;
		double PitchRate = 0.0;
		double MinPitch = 0.0;
		double MaxPitch = 0.0;
		double ClearanceM = 0.0;
		double MountWeightKg = 0.0;
		bool bAllowFireWhileAligning = false;

		TestTrue(TEXT("Cannon base resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*CannonTurret, TEXT("BaseMeshResourceId"), BaseResourceId));
		TestTrue(TEXT("Cannon yaw resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*CannonTurret, TEXT("YawMeshResourceId"), YawResourceId));
		TestTrue(TEXT("Cannon pitch resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*CannonTurret, TEXT("PitchMeshResourceId"), PitchResourceId));
		TestEqual(TEXT("Cannon base resource"), BaseResourceId, FString(TEXT("mesh.turret.base.standard")));
		TestEqual(TEXT("Cannon yaw resource"), YawResourceId, FString(TEXT("mesh.turret.cannon.yaw")));
		TestEqual(TEXT("Cannon pitch resource"), PitchResourceId, FString(TEXT("mesh.turret.cannon.pitch")));

		TestTrue(TEXT("Cannon yaw rate readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*CannonTurret, TEXT("YawTurnRateDegPerSec"), YawRate));
		TestTrue(TEXT("Cannon pitch rate readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*CannonTurret, TEXT("PitchTurnRateDegPerSec"), PitchRate));
		TestTrue(TEXT("Cannon min pitch readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*CannonTurret, TEXT("MinPitchDeg"), MinPitch));
		TestTrue(TEXT("Cannon max pitch readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*CannonTurret, TEXT("MaxPitchDeg"), MaxPitch));
		TestTrue(TEXT("Cannon clearance readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*CannonTurret, TEXT("MuzzleClearanceDistanceM"), ClearanceM));
		TestTrue(TEXT("Cannon mount weight readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*CannonTurret, TEXT("MountWeightKg"), MountWeightKg));
		TestTrue(TEXT("Cannon fire while aligning readable"), CFProdBatchPrepTestsPrivate::TryGetRecordBoolValue(*CannonTurret, TEXT("AllowFireWhileAligning"), bAllowFireWhileAligning));
		TestEqual(TEXT("Cannon persisted yaw rate"), YawRate, 35.0);
		TestEqual(TEXT("Cannon persisted pitch rate"), PitchRate, 20.0);
		TestEqual(TEXT("Cannon persisted min pitch"), MinPitch, -8.0);
		TestEqual(TEXT("Cannon persisted max pitch"), MaxPitch, 25.0);
		TestEqual(TEXT("Cannon persisted clearance m"), ClearanceM, 1.5);
		TestEqual(TEXT("Cannon persisted mount weight kg"), MountWeightKg, 200.0);
		TestTrue(TEXT("Cannon persisted allow fire while aligning"), bAllowFireWhileAligning);

		const TArray<FString> CannonMuzzles =
			CFProdBatchPrepTestsPrivate::ReadOrderedStringValues(
				*CannonTurret,
				TEXT("MuzzleSemantics"),
				TEXT("SocketSemantic"));
		TestEqual(TEXT("Cannon muzzle semantic count exact1"), CannonMuzzles.Num(), 1);
		if (CannonMuzzles.Num() == 1)
		{
			TestEqual(TEXT("Cannon muzzle semantic"), CannonMuzzles[0], FString(TEXT("Muzzle")));
		}
	}

	if (RocketTurret != nullptr)
	{
		FString YawResourceId;
		FString PitchResourceId;
		double MountWeightKg = 0.0;
		bool bRequireAllMuzzles = false;

		TestTrue(TEXT("Rocket yaw resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*RocketTurret, TEXT("YawMeshResourceId"), YawResourceId));
		TestTrue(TEXT("Rocket pitch resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*RocketTurret, TEXT("PitchMeshResourceId"), PitchResourceId));
		TestEqual(TEXT("Rocket yaw resource"), YawResourceId, FString(TEXT("mesh.turret.rocket.yaw")));
		TestEqual(TEXT("Rocket pitch resource"), PitchResourceId, FString(TEXT("mesh.turret.rocket.pitch")));
		TestTrue(TEXT("Rocket mount weight readable"), CFProdBatchPrepTestsPrivate::TryGetRecordDoubleValue(*RocketTurret, TEXT("MountWeightKg"), MountWeightKg));
		TestEqual(TEXT("Rocket persisted mount weight kg"), MountWeightKg, 150.0);
		TestTrue(TEXT("Rocket RequireAllMuzzles readable"), CFProdBatchPrepTestsPrivate::TryGetRecordBoolValue(*RocketTurret, TEXT("RequireAllMuzzles"), bRequireAllMuzzles));
		TestTrue(TEXT("Rocket requires all persisted muzzles"), bRequireAllMuzzles);

		const TArray<FString> RocketMuzzles =
			CFProdBatchPrepTestsPrivate::ReadOrderedStringValues(
				*RocketTurret,
				TEXT("MuzzleSemantics"),
				TEXT("SocketSemantic"));
		const TArray<FString> ExpectedRocketMuzzles = {
			TEXT("Muzzle_1"),
			TEXT("Muzzle_4"),
			TEXT("Muzzle_2"),
			TEXT("Muzzle_3")
		};
		TestEqual(TEXT("Rocket muzzle semantic count exact4"), RocketMuzzles.Num(), 4);
		if (RocketMuzzles.Num() == ExpectedRocketMuzzles.Num())
		{
			for (int32 MuzzleIndex = 0; MuzzleIndex < ExpectedRocketMuzzles.Num(); ++MuzzleIndex)
			{
				TestEqual(
					*FString::Printf(TEXT("Rocket muzzle semantic order %d"), MuzzleIndex),
					RocketMuzzles[MuzzleIndex],
					ExpectedRocketMuzzles[MuzzleIndex]);
			}
		}
	}

	// Representative Product/Projectile resource IDs가 stable reusable resource를 사용함을 검증합니다.
	const FCFContentRecord* CannonWeaponDefinition =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("WeaponDefinition"),
			TEXT("Cannon_Standard"));
	const FCFContentRecord* RocketProjectileRecord =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("Projectile"),
			TEXT("Projectile_Rocket"));
	TestNotNull(TEXT("Cannon WeaponDefinition exists"), CannonWeaponDefinition);
	TestNotNull(TEXT("Rocket Projectile exists"), RocketProjectileRecord);
	if (CannonWeaponDefinition != nullptr)
	{
		FString FireFxResourceId;
		TestTrue(TEXT("Cannon Fire FX resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*CannonWeaponDefinition, TEXT("FireFxResourceId"), FireFxResourceId));
		TestEqual(TEXT("Cannon Fire FX resource"), FireFxResourceId, FString(TEXT("fx.weapon.fire.proto")));
	}
	if (RocketProjectileRecord != nullptr)
	{
		FString ActorResourceId;
		FString MeshResourceId;
		FString ThrusterResourceId;
		TestTrue(TEXT("Rocket actor resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*RocketProjectileRecord, TEXT("ProjectileActorResourceId"), ActorResourceId));
		TestTrue(TEXT("Rocket mesh resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*RocketProjectileRecord, TEXT("ProjectileMeshResourceId"), MeshResourceId));
		TestTrue(TEXT("Rocket thruster resource readable"), CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(*RocketProjectileRecord, TEXT("ThrusterFxResourceId"), ThrusterResourceId));
		TestEqual(TEXT("Rocket actor resource"), ActorResourceId, FString(TEXT("blueprint.projectile.standard")));
		TestEqual(TEXT("Rocket mesh resource"), MeshResourceId, FString(TEXT("mesh.projectile.rocket")));
		TestEqual(TEXT("Rocket thruster resource"), ThrusterResourceId, FString(TEXT("fx.projectile.thruster.rocket")));
	}

	// Shared Rocket Ammo payload입니다.
	const FCFContentRecord* RocketAmmo =
		CFProdBatchPrepTestsPrivate::FindRecord(
			Workbook,
			TEXT("Ammo"),
			TEXT("Ammo_Rocket"));
	TestNotNull(TEXT("Shared Rocket Ammo record exists"), RocketAmmo);
	if (RocketAmmo != nullptr)
	{
		// Frozen Rocket ammo family identity입니다.
		FString RocketAmmoFamily;
		TestTrue(
			TEXT("Rocket AmmoFamilyId readable"),
			CFProdBatchPrepTestsPrivate::TryGetRecordStringValue(
				*RocketAmmo,
				TEXT("AmmoFamilyId"),
				RocketAmmoFamily));
		TestEqual(
			TEXT("Rocket AmmoFamilyId matches current Roster"),
			RocketAmmoFamily,
			FString(TEXT("Rocket_Standard_Warhead")));
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("CCAS_PROD_REVIEW|ReviewPackageFingerprint=%s|ExpectedPostSemanticHash=%s|ResourceCatalogFingerprint=%s|ExecutionBindingFingerprint=%s|Products=%d|RoleBindings=%d|Targets=%d|AmmoLoads=%d|Resources=%d|SocketChecks=%d|State=ReviewReady"),
		*Result.ReviewPackage.ReviewPackageFingerprint,
		*Result.ReviewPackage.ExpectedPostSemanticHash,
		*Result.ResourceCatalogFingerprint,
		*Result.ExecutionBindingFingerprint,
		Result.ProductCount,
		Result.RoleBindingCount,
		Result.UniqueDataAssetCount,
		Result.AmmoLoadCount,
		Result.ResolvedResourceCount,
		Result.SocketCapabilityCheckCount);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
