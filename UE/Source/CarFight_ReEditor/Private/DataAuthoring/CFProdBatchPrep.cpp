// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdBatchPrep.cpp
// Version: v1.4.0
// Date: 2026-10-06
// Description: CF-FQ-058 First Production Wave exact34 candidate batch read-only preparation implementation입니다.
// Changelog:
// - v1.4.0: first Production cutover provenance용 ProviderSchemaFingerprint를 reviewed BaseSnapshot에서 결과 evidence로 보존.
// - v1.3.0: exact8 typed execution manifest/desired fingerprints를 immutable Review Package에 결속하고 PFP-complete Guided readiness를 반영.
// - v1.2.0: fresh persisted AssetDump 기준 resource reuse + turret socket capability proof + current turret values를 candidate Review Package에 결속.
// - v1.1.0: Roster 미지정 seed를 기존 typed backend 기본값으로 정렬하고 실제 Projectile/Damage field 의미와 schema 명칭을 교정.
// - v1.0.0: current Roster exact8 balance/naming/topology를 transient Workbook + immutable Review Package로 구성.
// Migration:
// - persistent Workbook/DataAsset/Publication mutation exact0.
// - managed authority cutover 전 First Wave bootstrap/review-only seed이며 cutover 이후 canonical Workbook과 병렬 authoring authority로 사용하지 않음.
// - Required resource/capability proof가 불완전하면 ResourceBindingPending, 모두 충족하면 ReviewReady로 fail-visible 전환.
// - durable apply는 기존 CFProdProvisioning + CFProdTargetBindings만 사용하며 이 파일은 writer를 소유하지 않음.

#include "DataAuthoring/CFProdBatchPrep.h"

#include "DataAuthoring/CFContentCompiler.h"
#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFContentProvider.h"
#include "DataAuthoring/CFContentResource.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFProdProvisioning.h"
#include "EquipmentAuthoring/CFEquipmentDurable.h"
#include "WeaponAuthoring/CFWeaponGuideBackend.h"
#include "CFCombatFxData.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "NiagaraSystem.h"

#if WITH_EDITOR

namespace CFProdBatchPrepPrivate
{
	// First Production Wave의 frozen balance/topology와 typed backend default seed를 함께 보존하는 candidate input입니다.
	struct FProductSpec
	{
		// Canonical Product identity입니다.
		FString ProductId;

		// Product family identity입니다.
		FString FamilyId;

		// Family 안의 variant identity입니다.
		FString VariantId;

		// Gameplay role identity입니다.
		FString GameplayRole;

		// Human-readable design intent입니다.
		FString DesignIntent;

		// 비교 기준 Product identity입니다.
		FString BaseProductId;

		// Wave1A / Wave1B identity입니다.
		FString ProductionWaveId;

		// Product가 사용하는 shared/new Mount identity입니다.
		FString MountId;

		// Product가 사용하는 Projectile identity입니다.
		FString ProjectileId;

		// Product가 사용하는 Damage identity입니다.
		FString DamageId;

		// Product가 사용하는 Ammo identity입니다.
		FString AmmoId;

		// Product가 Mount를 최초 생성하는 shared owner인지 여부입니다.
		bool bCreateSharedMount = false;

		// Product가 Projectile을 최초 생성하는 shared owner인지 여부입니다.
		bool bCreateSharedProjectile = false;

		// Product가 Damage를 최초 생성하는 shared owner인지 여부입니다.
		bool bCreateSharedDamage = false;

		// Product가 Ammo를 최초 생성하는 shared owner인지 여부입니다.
		bool bCreateSharedAmmo = false;

		// Direct-hit BaseDamage입니다.
		double BaseDamage = 0.0;

		// 분당 발사 횟수입니다.
		double FireRatePerMinute = 0.0;

		// Projectile initial speed authored unit m/s입니다.
		double InitialSpeedMetersPerSecond = 0.0;

		// Weapon max range authored unit m입니다.
		double MaxRangeMeters = 0.0;

		// Spread angle authored unit deg입니다.
		double SpreadDeg = 0.0;

		// Magazine capacity입니다.
		int64 MagazineSize = 0;

		// Initial loaded ammo count입니다.
		int64 InitialLoadedAmmoCount = 0;

		// Reload authored unit seconds입니다.
		double ReloadTimeSeconds = 0.0;

		// Weapon mass authored unit kg입니다.
		double WeaponMassKg = 0.0;

		// Ammo maximum loadable count입니다.
		int64 AmmoMaximumLoadableCount = 0;

		// Explicit sortie ammo count입니다.
		int64 DefaultSortieAmmoCount = 0;

		// Ammo unit mass authored unit kg입니다.
		double AmmoUnitMassKg = 0.0;

		// Projectile lifetime authored unit seconds입니다.
		double ProjectileLifeTimeSeconds = 0.0;

		// Launcher fire pattern identity입니다.
		FString LauncherFirePattern;

		// Launcher trigger당 projectile 수입니다.
		int64 ProjectileCountPerTrigger = 1;

		// Salvo simultaneous limit입니다.
		int64 MaximumSimultaneousLaunchCount = 1;

		// Ripple delay authored unit seconds입니다.
		double InterMuzzleDelaySeconds = 0.0;

		// Projectile propulsion 사용 여부입니다.
		bool bUsePropulsion = false;

		// Propulsion ignition delay authored unit seconds입니다.
		double IgnitionDelaySeconds = 0.0;

		// Propulsion burn duration authored unit seconds입니다.
		double BurnDurationSeconds = 0.0;

		// Propulsion acceleration authored unit m/s^2입니다.
		double ThrustAccelerationMetersPerSecondSq = 0.0;

		// Propulsion maximum speed authored unit m/s입니다.
		double MaximumPropelledSpeedMetersPerSecond = 0.0;

		// Launch-axis stabilization 사용 여부입니다.
		bool bUseLaunchAxisStabilization = false;

		// Maximum thrust vector angle authored unit deg입니다.
		double MaximumThrustVectorAngleDeg = 0.0;

		// Launch-axis stabilization response authored unit seconds입니다.
		double LaunchAxisResponseTimeSeconds = 0.0;

		// Guided missile flight profile 사용 여부입니다.
		bool bUseMissileFlight = false;

		// TargetActor guidance 사용 여부입니다.
		bool bUseGuidance = false;
	};

	// ContentType별 frozen schema를 read-only planning layer에 노출하는 provider입니다.
	class FReviewProvider final : public ICFContentProvider
	{
	public:
		// Provider descriptor를 exact1로 보존합니다.
		explicit FReviewProvider(const FCFContentProviderDescriptor& InDescriptor)
			: Descriptor(InDescriptor)
		{
		}

		// Frozen descriptor를 반환합니다.
		virtual bool DescribeSchema(
			FCFContentProviderDescriptor& OutDescriptor,
			FString& OutError) const override
		{
			OutDescriptor = Descriptor;
			OutError.Reset();
			return true;
		}

		// Candidate preparation은 기존 Production record를 current authority로 import하지 않습니다.
		virtual bool ImportExisting(
			FCFContentImportResult& OutImport,
			FString& OutError) const override
		{
			OutImport = FCFContentImportResult();
			OutError.Reset();
			return true;
		}

		// Candidate preparation에서 current persisted Product snapshot은 존재하지 않는 것으로 fail-visible 처리합니다.
		virtual bool BuildCurrentSnapshot(
			const FCFContentKey& Key,
			FCFContentSnapshot& OutSnapshot,
			FString& OutError) const override
		{
			(void)Key;
			OutSnapshot = FCFContentSnapshot();
			OutError = TEXT("Production candidate preparation provider는 persisted current snapshot authority가 아닙니다.");
			return false;
		}

		// ContentType 고유 candidate invariant를 검증합니다.
		virtual bool ValidateRecord(
			const FCFContentRecord& Record,
			TArray<FCFContentValidationIssue>& OutIssues) const override
		{
			OutIssues.Reset();

			if (!(Record.Key.ContentTypeId == Descriptor.ContentTypeId))
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderContentTypeMismatch"),
					Record.Key.ToStableString(),
					TEXT("Candidate record ContentType이 registered provider와 다릅니다."));
				return false;
			}

			if (Descriptor.ContentTypeId.Value.Equals(TEXT("Weapon"), ESearchCase::CaseSensitive))
			{
				const FCFContentCollection* RoleBindings = Record.Collections.Find(TEXT("RoleBindings"));
				if (RoleBindings == nullptr || RoleBindings->Items.Num() != 6)
				{
					AddIssue(
						OutIssues,
						TEXT("ProductionRoleBindingCount"),
						Record.Key.ToStableString(),
						TEXT("First Production Wave Weapon Product는 Required Role binding exact6을 가져야 합니다."));
				}
			}
			else if (Descriptor.ContentTypeId.Value.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
			{
				const FCFContentCollection* AmmoLoads =
					Record.Collections.Find(TEXT("EquipmentPresetAmmoLoads"));
				if (AmmoLoads == nullptr || AmmoLoads->Items.Num() != 1)
				{
					AddIssue(
						OutIssues,
						TEXT("ProductionAmmoLoadCount"),
						Record.Key.ToStableString(),
						TEXT("First Production Wave finite EquipmentPreset는 explicit sortie ammo row exact1을 가져야 합니다."));
				}
			}

			return OutIssues.IsEmpty();
		}

		// Candidate record가 만드는 typed dependency edge를 구성합니다.
		virtual bool BuildDependencyEdges(
			const FCFContentRecord& Record,
			TArray<FCFContentDependencyEdge>& OutEdges,
			FString& OutError) const override
		{
			OutEdges.Reset();
			OutError.Reset();

			for (const TPair<FString, FCFContentValue>& FieldPair : Record.Fields)
			{
				const FCFContentValue& Value = FieldPair.Value;
				if (Value.Type == ECFContentValueType::ContentReference
					&& Value.State == ECFContentValueState::Value)
				{
					FCFContentDependencyEdge Edge;
					Edge.From = Record.Key;
					Edge.To = Value.ContentReference;
					Edge.FieldPath = FieldPair.Key;
					OutEdges.Add(MoveTemp(Edge));
				}
			}

			if (Record.Key.ContentTypeId.Value.Equals(TEXT("Weapon"), ESearchCase::CaseSensitive))
			{
				const FCFContentCollection* RoleBindings = Record.Collections.Find(TEXT("RoleBindings"));
				if (RoleBindings != nullptr)
				{
					for (const FCFContentCollectionItem& Item : RoleBindings->Items)
					{
						const FCFContentValue* RoleValue = Item.Fields.Find(TEXT("RoleId"));
						const FCFContentValue* BoundValue = Item.Fields.Find(TEXT("BoundContentId"));
						if (RoleValue == nullptr
							|| BoundValue == nullptr
							|| RoleValue->State != ECFContentValueState::Value
							|| BoundValue->State != ECFContentValueState::Value)
						{
							continue;
						}

						FCFContentKey TargetKey;
						TargetKey.ContentTypeId.Value = ContentTypeForRole(RoleValue->StringValue);
						TargetKey.ContentId = BoundValue->StringValue;
						if (!TargetKey.IsValid())
						{
							OutError = TEXT("RoleBindings가 invalid target ContentKey를 생성했습니다.");
							return false;
						}

						FCFContentDependencyEdge Edge;
						Edge.From = Record.Key;
						Edge.To = TargetKey;
						Edge.FieldPath = TEXT("RoleBindings.") + RoleValue->StringValue;
						OutEdges.Add(MoveTemp(Edge));
					}
				}
			}
			else if (Record.Key.ContentTypeId.Value.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
			{
				const FCFContentCollection* AmmoLoads =
					Record.Collections.Find(TEXT("EquipmentPresetAmmoLoads"));
				if (AmmoLoads != nullptr)
				{
					for (const FCFContentCollectionItem& Item : AmmoLoads->Items)
					{
						const FCFContentValue* AmmoValue = Item.Fields.Find(TEXT("AmmoRoleContentId"));
						if (AmmoValue == nullptr || AmmoValue->State != ECFContentValueState::Value)
						{
							continue;
						}

						FCFContentDependencyEdge Edge;
						Edge.From = Record.Key;
						Edge.To = MakeKey(TEXT("Ammo"), AmmoValue->StringValue);
						Edge.FieldPath = TEXT("EquipmentPresetAmmoLoads.") + AmmoValue->StringValue;
						OutEdges.Add(MoveTemp(Edge));
					}
				}
			}

			return true;
		}

		// Current/desired record diff를 deterministic semantic fingerprint로 구성합니다.
		virtual bool BuildDiff(
			const FCFContentSnapshot& Current,
			const FCFContentRecord& Desired,
			FCFContentDiff& OutDiff,
			FString& OutError) const override
		{
			OutDiff = FCFContentDiff();
			OutDiff.Key = Desired.Key;

			if (!BuildRecordFingerprint(Current.Record, OutDiff.CurrentFingerprint, OutError)
				|| !BuildRecordFingerprint(Desired, OutDiff.DesiredFingerprint, OutError))
			{
				return false;
			}

			if (!OutDiff.CurrentFingerprint.Equals(
				OutDiff.DesiredFingerprint,
				ESearchCase::CaseSensitive))
			{
				OutDiff.ChangedPaths.Add(TEXT("$record"));
			}
			return true;
		}

		// Immutable review 뒤 전달할 mutation plan shape만 구성합니다.
		virtual bool BuildReviewedMutationPlan(
			const FCFContentDiff& Diff,
			const FString& BaseWorkbookSemanticHash,
			FCFContentReviewedMutationPlan& OutPlan,
			FString& OutError) const override
		{
			OutError.Reset();
			OutPlan = FCFContentReviewedMutationPlan();
			OutPlan.Key = Diff.Key;
			OutPlan.BaseWorkbookSemanticHash = BaseWorkbookSemanticHash;
			OutPlan.DesiredFingerprint = Diff.DesiredFingerprint;
			OutPlan.ChangedPaths = Diff.ChangedPaths;
			return true;
		}

		// Candidate preparation provider는 persistent mutation authority를 절대 열지 않습니다.
		virtual bool ApplyReviewed(
			const FCFContentReviewedMutationPlan& ReviewedPlan,
			FCFContentApplyResult& OutResult) override
		{
			(void)ReviewedPlan;
			OutResult = FCFContentApplyResult();
			OutResult.Error =
				TEXT("Production candidate preparation provider는 read-only입니다. durable mutation은 CFProdProvisioning 경계에서만 수행합니다.");
			return false;
		}

		// Candidate preparation provider는 persisted readback authority가 아닙니다.
		virtual bool ReadbackFingerprint(
			const FCFContentKey& Key,
			FString& OutFingerprint,
			FString& OutError) const override
		{
			(void)Key;
			OutFingerprint.Reset();
			OutError = TEXT("Production candidate preparation provider에는 persisted readback이 없습니다.");
			return false;
		}

	private:
		// Provider validation diagnostic을 추가합니다.
		static void AddIssue(
			TArray<FCFContentValidationIssue>& OutIssues,
			const FString& Code,
			const FString& Path,
			const FString& Message)
		{
			FCFContentValidationIssue Issue;
			Issue.Code = Code;
			Issue.Path = Path;
			Issue.Message = Message;
			Issue.bBlocking = true;
			OutIssues.Add(MoveTemp(Issue));
		}

		// RoleId를 current exact6 target ContentType으로 변환합니다.
		static FString ContentTypeForRole(const FString& RoleId)
		{
			if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
			{
				return TEXT("EquipmentPreset");
			}
			if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
			{
				return TEXT("TurretMount");
			}
			if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
			{
				return TEXT("WeaponDefinition");
			}
			if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
			{
				return TEXT("Projectile");
			}
			if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
			{
				return TEXT("Damage");
			}
			if (RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive))
			{
				return TEXT("Ammo");
			}
			return FString();
		}

		// ContentType + ContentId canonical key를 생성합니다.
		static FCFContentKey MakeKey(
			const FString& ContentTypeId,
			const FString& ContentId)
		{
			FCFContentKey Key;
			Key.ContentTypeId.Value = ContentTypeId;
			Key.ContentId = ContentId;
			return Key;
		}

		// Provider schema + single record를 canonical semantic fingerprint로 변환합니다.
		bool BuildRecordFingerprint(
			const FCFContentRecord& Record,
			FString& OutFingerprint,
			FString& OutError) const
		{
			FCFContentWorkbookModel Projection;
			Projection.WorkbookSourceId = TEXT("CarFight.Content.Production.Review");
			Projection.SchemaRevision = Descriptor.SchemaRevision;
			Projection.Sheets = Descriptor.Sheets;
			Projection.Records.Add(Record);
			return FCFContentSemanticHasher::BuildWorkbookSemanticHash(
				Projection,
				OutFingerprint,
				OutError);
		}

		// 이 read-only provider가 소유하는 frozen schema입니다.
		FCFContentProviderDescriptor Descriptor;
	};

	// Persistent write를 제공하지 않는 in-memory Workbook adapter입니다.
	class FMemoryWorkbookAdapter final : public ICFContentWorkbookAdapter
	{
	public:
		// Read-only source Workbook을 보존합니다.
		explicit FMemoryWorkbookAdapter(const FCFContentWorkbookModel& InWorkbook)
			: Workbook(InWorkbook)
		{
		}

		// Candidate preparation adapter identity를 반환합니다.
		virtual FCFWorkbookAdapterInfo DescribeAdapter() const override
		{
			FCFWorkbookAdapterInfo Info;
			Info.AdapterId = TEXT("CFProdBatchPrep.Memory");
			Info.AdapterVersion = TEXT("1.0");
			return Info;
		}

		// In-memory Workbook을 반환합니다.
		virtual bool ReadWorkbook(
			const FString& WorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			if (WorkbookPath.IsEmpty())
			{
				OutError = TEXT("Candidate preparation Workbook path token이 비었습니다.");
				return false;
			}
			OutWorkbook = Workbook;
			OutError.Reset();
			return true;
		}

		// Candidate preparation에서 persistent Workbook write를 차단합니다.
		virtual bool WriteStagedWorkbook(
			const FString& BaseWorkbookPath,
			const FString& StagedWorkbookPath,
			const FCFContentWorkbookModel& ReviewedWorkbook,
			FString& OutError) override
		{
			(void)BaseWorkbookPath;
			(void)StagedWorkbookPath;
			(void)ReviewedWorkbook;
			OutError = TEXT("Candidate preparation은 persistent Workbook write를 수행하지 않습니다.");
			return false;
		}

		// Candidate preparation에서 disk reopen을 차단합니다.
		virtual bool ReopenWorkbook(
			const FString& StagedWorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			(void)StagedWorkbookPath;
			(void)OutWorkbook;
			OutError = TEXT("Candidate preparation은 disk Workbook reopen을 수행하지 않습니다.");
			return false;
		}

	private:
		// Read-only source Workbook입니다.
		FCFContentWorkbookModel Workbook;
	};

	// Canonical ContentKey를 생성합니다.
	FCFContentKey MakeKey(
		const FString& ContentTypeId,
		const FString& ContentId)
	{
		FCFContentKey Key;
		Key.ContentTypeId.Value = ContentTypeId;
		Key.ContentId = ContentId;
		return Key;
	}

	// Boolean canonical VALUE를 생성합니다.
	FCFContentValue BoolValue(const bool bValue)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::Boolean;
		Value.State = ECFContentValueState::Value;
		Value.bBooleanValue = bValue;
		return Value;
	}

	// Signed integer canonical VALUE를 생성합니다.
	FCFContentValue IntValue(const int64 Number)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::SignedInteger;
		Value.State = ECFContentValueState::Value;
		Value.SignedIntegerValue = Number;
		return Value;
	}

	// Double canonical VALUE를 생성합니다.
	FCFContentValue DoubleValue(const double Number)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::Double;
		Value.State = ECFContentValueState::Value;
		Value.FloatingPointValue = Number;
		return Value;
	}

	// Enum canonical VALUE를 생성합니다.
	FCFContentValue EnumValue(const FString& Token)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::Enum;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = Token;
		return Value;
	}

	// NameId canonical VALUE를 생성합니다.
	FCFContentValue NameValue(const FString& Token)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::NameId;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = Token;
		return Value;
	}

	// String canonical VALUE를 생성합니다.
	FCFContentValue StringValue(const FString& Text)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::String;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = Text;
		return Value;
	}

	// Explicit NONE NameId를 생성합니다.
	FCFContentValue NoneNameValue()
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::NameId;
		Value.State = ECFContentValueState::None;
		return Value;
	}

	// Explicit NONE ResourceReference를 생성합니다.
	FCFContentValue NoneResourceValue()
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::ResourceReference;
		Value.State = ECFContentValueState::None;
		return Value;
	}

	// Stable ResourceId canonical VALUE를 생성합니다.
	FCFContentValue ResourceValue(const FString& ResourceId)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::ResourceReference;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = ResourceId;
		return Value;
	}

	// ContentReference canonical VALUE를 생성합니다.
	FCFContentValue ReferenceValue(
		const FString& ContentTypeId,
		const FString& ContentId)
	{
		FCFContentValue Value;
		Value.Type = ECFContentValueType::ContentReference;
		Value.State = ECFContentValueState::Value;
		Value.ContentReference = MakeKey(ContentTypeId, ContentId);
		return Value;
	}

	// CCAS-managed field descriptor를 생성합니다.
	FCFContentFieldDescriptor Field(
		const FString& ColumnId,
		const ECFContentValueType ValueType,
		const bool bRequired = true,
		const bool bAllowNone = false,
		const FString& CanonicalUnitId = FString())
	{
		FCFContentFieldDescriptor Descriptor;
		Descriptor.ColumnId = ColumnId;
		Descriptor.DisplayLabel = ColumnId;
		Descriptor.ValueType = ValueType;
		Descriptor.Ownership = ECFContentFieldOwnership::CCASManaged;
		Descriptor.CanonicalUnitId = CanonicalUnitId;
		Descriptor.bRequired = bRequired;
		Descriptor.bAllowNone = bAllowNone;
		return Descriptor;
	}

	// Primary sheet descriptor를 생성합니다.
	FCFContentSheetDescriptor PrimarySheet(
		const FString& SheetId,
		const FString& ContentTypeId,
		const TArray<FCFContentFieldDescriptor>& Fields)
	{
		FCFContentSheetDescriptor Sheet;
		Sheet.SheetId = SheetId;
		Sheet.SchemaRevision = 1;
		Sheet.ContentTypeId.Value = ContentTypeId;
		Sheet.Fields = Fields;
		return Sheet;
	}

	// Keyed child sheet descriptor를 생성합니다.
	FCFContentSheetDescriptor KeyedChildSheet(
		const FString& SheetId,
		const FString& ContentTypeId,
		const FString& ParentSheetId,
		const FString& CollectionId,
		const TArray<FCFContentFieldDescriptor>& Fields)
	{
		FCFContentSheetDescriptor Sheet;
		Sheet.SheetId = SheetId;
		Sheet.SchemaRevision = 1;
		Sheet.ContentTypeId.Value = ContentTypeId;
		Sheet.ParentSheetId = ParentSheetId;
		Sheet.CollectionId = CollectionId;
		Sheet.CollectionKind = ECFContentCollectionKind::KeyedCollection;
		Sheet.Fields = Fields;
		return Sheet;
	}

	// First Production Wave exact7 + Guided exact1 frozen specs를 생성합니다.
	TArray<FProductSpec> BuildProductSpecs()
	{
		TArray<FProductSpec> Specs;

		FProductSpec CannonStandard;
		CannonStandard.ProductId = TEXT("Cannon_Standard");
		CannonStandard.FamilyId = TEXT("Cannon");
		CannonStandard.VariantId = TEXT("Standard");
		CannonStandard.GameplayRole = TEXT("GeneralPurpose");
		CannonStandard.DesignIntent = TEXT("Cannon family baseline with balanced damage, accuracy, range, reload and finite ammo.");
		CannonStandard.ProductionWaveId = TEXT("Wave1A");
		CannonStandard.MountId = TEXT("Mount_Cannon");
		CannonStandard.ProjectileId = TEXT("Projectile_Cannon_Standard");
		CannonStandard.DamageId = TEXT("Damage_Cannon_Standard");
		CannonStandard.AmmoId = TEXT("Ammo_Cannon_Standard");
		CannonStandard.bCreateSharedMount = true;
		CannonStandard.bCreateSharedDamage = true;
		CannonStandard.bCreateSharedAmmo = true;
		CannonStandard.BaseDamage = 25.0;
		CannonStandard.FireRatePerMinute = 60.0;
		CannonStandard.InitialSpeedMetersPerSecond = 90.0;
		CannonStandard.MaxRangeMeters = 120.0;
		CannonStandard.SpreadDeg = 0.35;
		CannonStandard.MagazineSize = 8;
		CannonStandard.InitialLoadedAmmoCount = 8;
		CannonStandard.ReloadTimeSeconds = 3.0;
		CannonStandard.WeaponMassKg = 650.0;
		CannonStandard.AmmoMaximumLoadableCount = 40;
		CannonStandard.DefaultSortieAmmoCount = 40;
		CannonStandard.AmmoUnitMassKg = 12.0;
		CannonStandard.ProjectileLifeTimeSeconds = 2.0;
		CannonStandard.LauncherFirePattern = TEXT("SingleCycle");
		Specs.Add(CannonStandard);

		FProductSpec CannonHeavy = CannonStandard;
		CannonHeavy.ProductId = TEXT("Cannon_Heavy");
		CannonHeavy.VariantId = TEXT("Heavy");
		CannonHeavy.GameplayRole = TEXT("HighDamage");
		CannonHeavy.DesignIntent = TEXT("Higher per-shot damage with slower fire rate, heavier weapon and heavier finite ammo.");
		CannonHeavy.BaseProductId = TEXT("Cannon_Standard");
		CannonHeavy.ProjectileId = TEXT("Projectile_Cannon_Heavy");
		CannonHeavy.DamageId = TEXT("Damage_Cannon_Heavy");
		CannonHeavy.AmmoId = TEXT("Ammo_Cannon_Heavy");
		CannonHeavy.bCreateSharedMount = false;
		CannonHeavy.bCreateSharedDamage = false;
		CannonHeavy.bCreateSharedAmmo = false;
		CannonHeavy.BaseDamage = 45.0;
		CannonHeavy.FireRatePerMinute = 30.0;
		CannonHeavy.InitialSpeedMetersPerSecond = 75.0;
		CannonHeavy.MaxRangeMeters = 100.0;
		CannonHeavy.SpreadDeg = 0.50;
		CannonHeavy.MagazineSize = 4;
		CannonHeavy.InitialLoadedAmmoCount = 4;
		CannonHeavy.ReloadTimeSeconds = 4.5;
		CannonHeavy.WeaponMassKg = 950.0;
		CannonHeavy.AmmoMaximumLoadableCount = 20;
		CannonHeavy.DefaultSortieAmmoCount = 20;
		CannonHeavy.AmmoUnitMassKg = 22.0;
		Specs.Add(CannonHeavy);

		FProductSpec CannonLongRange = CannonStandard;
		CannonLongRange.ProductId = TEXT("Cannon_LongRange");
		CannonLongRange.VariantId = TEXT("LongRange");
		CannonLongRange.GameplayRole = TEXT("LongRangePrecision");
		CannonLongRange.DesignIntent = TEXT("Higher projectile speed, lower spread and longer range while sharing Standard damage and ammo identity.");
		CannonLongRange.BaseProductId = TEXT("Cannon_Standard");
		CannonLongRange.ProjectileId = TEXT("Projectile_Cannon_LongRange");
		CannonLongRange.bCreateSharedMount = false;
		CannonLongRange.bCreateSharedDamage = false;
		CannonLongRange.bCreateSharedAmmo = false;
		CannonLongRange.FireRatePerMinute = 45.0;
		CannonLongRange.InitialSpeedMetersPerSecond = 140.0;
		CannonLongRange.MaxRangeMeters = 180.0;
		CannonLongRange.SpreadDeg = 0.10;
		CannonLongRange.MagazineSize = 6;
		CannonLongRange.InitialLoadedAmmoCount = 6;
		CannonLongRange.ReloadTimeSeconds = 3.5;
		CannonLongRange.WeaponMassKg = 750.0;
		Specs.Add(CannonLongRange);

		FProductSpec CannonRapid = CannonStandard;
		CannonRapid.ProductId = TEXT("Cannon_Rapid");
		CannonRapid.VariantId = TEXT("Rapid");
		CannonRapid.GameplayRole = TEXT("SustainedPressure");
		CannonRapid.DesignIntent = TEXT("High fire rate and ammo consumption with low per-shot damage, shorter range and wider spread.");
		CannonRapid.BaseProductId = TEXT("Cannon_Standard");
		CannonRapid.ProjectileId = TEXT("Projectile_Cannon_Rapid");
		CannonRapid.DamageId = TEXT("Damage_Cannon_Rapid");
		CannonRapid.AmmoId = TEXT("Ammo_Cannon_Rapid");
		CannonRapid.bCreateSharedMount = false;
		CannonRapid.bCreateSharedDamage = false;
		CannonRapid.bCreateSharedAmmo = false;
		CannonRapid.BaseDamage = 8.0;
		CannonRapid.FireRatePerMinute = 240.0;
		CannonRapid.InitialSpeedMetersPerSecond = 85.0;
		CannonRapid.MaxRangeMeters = 90.0;
		CannonRapid.SpreadDeg = 0.85;
		CannonRapid.MagazineSize = 24;
		CannonRapid.InitialLoadedAmmoCount = 24;
		CannonRapid.ReloadTimeSeconds = 3.8;
		CannonRapid.WeaponMassKg = 700.0;
		CannonRapid.AmmoMaximumLoadableCount = 120;
		CannonRapid.DefaultSortieAmmoCount = 120;
		CannonRapid.AmmoUnitMassKg = 5.0;
		CannonRapid.ProjectileLifeTimeSeconds = 1.6;
		Specs.Add(CannonRapid);

		FProductSpec RocketStandard;
		RocketStandard.ProductId = TEXT("Rocket_Standard");
		RocketStandard.FamilyId = TEXT("UnguidedRocket");
		RocketStandard.VariantId = TEXT("Standard");
		RocketStandard.GameplayRole = TEXT("SingleRocket");
		RocketStandard.DesignIntent = TEXT("Single-cycle direct-hit unguided rocket using shared Rocket pod, projectile, warhead and ammo.");
		RocketStandard.ProductionWaveId = TEXT("Wave1A");
		RocketStandard.MountId = TEXT("Mount_RocketPod");
		RocketStandard.ProjectileId = TEXT("Projectile_Rocket");
		RocketStandard.DamageId = TEXT("Damage_Rocket");
		RocketStandard.AmmoId = TEXT("Ammo_Rocket");
		RocketStandard.bCreateSharedMount = true;
		RocketStandard.bCreateSharedProjectile = true;
		RocketStandard.bCreateSharedDamage = true;
		RocketStandard.bCreateSharedAmmo = true;
		RocketStandard.BaseDamage = 25.0;
		RocketStandard.FireRatePerMinute = 30.0;
		RocketStandard.InitialSpeedMetersPerSecond = 22.0;
		RocketStandard.MaxRangeMeters = 140.0;
		RocketStandard.SpreadDeg = 0.35;
		RocketStandard.MagazineSize = 6;
		RocketStandard.InitialLoadedAmmoCount = 6;
		RocketStandard.ReloadTimeSeconds = 4.0;
		RocketStandard.WeaponMassKg = 500.0;
		RocketStandard.AmmoMaximumLoadableCount = 32;
		RocketStandard.DefaultSortieAmmoCount = 32;
		RocketStandard.AmmoUnitMassKg = 18.0;
		RocketStandard.ProjectileLifeTimeSeconds = 4.0;
		RocketStandard.LauncherFirePattern = TEXT("SingleCycle");
		RocketStandard.ProjectileCountPerTrigger = 1;
		RocketStandard.MaximumSimultaneousLaunchCount = 1;
		RocketStandard.bUsePropulsion = true;
		RocketStandard.IgnitionDelaySeconds = 0.05;
		RocketStandard.BurnDurationSeconds = 1.40;
		RocketStandard.ThrustAccelerationMetersPerSecondSq = 70.0;
		RocketStandard.MaximumPropelledSpeedMetersPerSecond = 70.0;
		RocketStandard.bUseLaunchAxisStabilization = true;
		RocketStandard.MaximumThrustVectorAngleDeg = 12.0;
		RocketStandard.LaunchAxisResponseTimeSeconds = 0.25;
		Specs.Add(RocketStandard);

		FProductSpec RocketSalvo = RocketStandard;
		RocketSalvo.ProductId = TEXT("Rocket_Salvo");
		RocketSalvo.VariantId = TEXT("Salvo");
		RocketSalvo.GameplayRole = TEXT("BurstVolley");
		RocketSalvo.DesignIntent = TEXT("Four-projectile simultaneous salvo while sharing Rocket payload data with Standard and Ripple.");
		RocketSalvo.BaseProductId = TEXT("Rocket_Standard");
		RocketSalvo.bCreateSharedMount = false;
		RocketSalvo.bCreateSharedProjectile = false;
		RocketSalvo.bCreateSharedDamage = false;
		RocketSalvo.bCreateSharedAmmo = false;
		RocketSalvo.FireRatePerMinute = 12.0;
		RocketSalvo.SpreadDeg = 0.55;
		RocketSalvo.MagazineSize = 8;
		RocketSalvo.InitialLoadedAmmoCount = 8;
		RocketSalvo.ReloadTimeSeconds = 5.0;
		RocketSalvo.WeaponMassKg = 600.0;
		RocketSalvo.LauncherFirePattern = TEXT("Salvo");
		RocketSalvo.ProjectileCountPerTrigger = 4;
		RocketSalvo.MaximumSimultaneousLaunchCount = 4;
		Specs.Add(RocketSalvo);

		FProductSpec RocketRipple = RocketStandard;
		RocketRipple.ProductId = TEXT("Rocket_Ripple");
		RocketRipple.VariantId = TEXT("Ripple");
		RocketRipple.GameplayRole = TEXT("TimedVolley");
		RocketRipple.DesignIntent = TEXT("Four-projectile sequential ripple while sharing Rocket payload data with Standard and Salvo.");
		RocketRipple.BaseProductId = TEXT("Rocket_Standard");
		RocketRipple.bCreateSharedMount = false;
		RocketRipple.bCreateSharedProjectile = false;
		RocketRipple.bCreateSharedDamage = false;
		RocketRipple.bCreateSharedAmmo = false;
		RocketRipple.FireRatePerMinute = 15.0;
		RocketRipple.SpreadDeg = 0.45;
		RocketRipple.MagazineSize = 8;
		RocketRipple.InitialLoadedAmmoCount = 8;
		RocketRipple.ReloadTimeSeconds = 5.0;
		RocketRipple.WeaponMassKg = 575.0;
		RocketRipple.LauncherFirePattern = TEXT("Ripple");
		RocketRipple.ProjectileCountPerTrigger = 4;
		RocketRipple.MaximumSimultaneousLaunchCount = 1;
		RocketRipple.InterMuzzleDelaySeconds = 0.18;
		Specs.Add(RocketRipple);

		FProductSpec Guided;
		Guided.ProductId = TEXT("GuidedMissile_Standard");
		Guided.FamilyId = TEXT("GuidedMissile");
		Guided.VariantId = TEXT("Standard");
		Guided.GameplayRole = TEXT("TargetLockedGuided");
		Guided.DesignIntent = TEXT("TargetActor lock guided missile with independent projectile, warhead and ammo while initially sharing Rocket pod mount.");
		Guided.ProductionWaveId = TEXT("Wave1B");
		Guided.MountId = TEXT("Mount_RocketPod");
		Guided.ProjectileId = TEXT("Projectile_GuidedMissile_Standard");
		Guided.DamageId = TEXT("Damage_GuidedMissile_Standard");
		Guided.AmmoId = TEXT("Ammo_GuidedMissile_Standard");
		Guided.BaseDamage = 40.0;
		Guided.FireRatePerMinute = 15.0;
		Guided.InitialSpeedMetersPerSecond = 18.0;
		Guided.MaxRangeMeters = 180.0;
		Guided.SpreadDeg = 0.10;
		Guided.MagazineSize = 2;
		Guided.InitialLoadedAmmoCount = 2;
		Guided.ReloadTimeSeconds = 6.0;
		Guided.WeaponMassKg = 650.0;
		Guided.AmmoMaximumLoadableCount = 10;
		Guided.DefaultSortieAmmoCount = 10;
		Guided.AmmoUnitMassKg = 25.0;
		Guided.ProjectileLifeTimeSeconds = 5.0;
		Guided.LauncherFirePattern = TEXT("SingleCycle");
		Guided.ProjectileCountPerTrigger = 1;
		Guided.MaximumSimultaneousLaunchCount = 1;
		Guided.bUsePropulsion = true;
		Guided.IgnitionDelaySeconds = 0.15;
		Guided.BurnDurationSeconds = 2.0;
		Guided.ThrustAccelerationMetersPerSecondSq = 85.0;
		Guided.MaximumPropelledSpeedMetersPerSecond = 80.0;
		Guided.bUseMissileFlight = true;
		Guided.bUseGuidance = true;
		Specs.Add(Guided);

		return Specs;
	}

	// Weapon Product + RoleBindings schema를 생성합니다.
	FCFContentProviderDescriptor BuildWeaponProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.WeaponReview");
		Descriptor.ContentTypeId.Value = TEXT("Weapon");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("Weapons"),
				TEXT("Weapon"),
				{
					Field(TEXT("DisplayName"), ECFContentValueType::String),
					Field(TEXT("FamilyId"), ECFContentValueType::NameId),
					Field(TEXT("VariantId"), ECFContentValueType::NameId),
					Field(TEXT("GameplayRole"), ECFContentValueType::NameId),
					Field(TEXT("DesignIntent"), ECFContentValueType::String),
					Field(TEXT("RelativeIntent"), ECFContentValueType::String),
					Field(TEXT("BaseContentId"), ECFContentValueType::NameId, true, true),
					Field(TEXT("ProductionWave"), ECFContentValueType::NameId),
					Field(TEXT("Readiness"), ECFContentValueType::Enum),
					Field(TEXT("Lifecycle"), ECFContentValueType::Enum)
				}));
		Descriptor.Sheets.Add(
			KeyedChildSheet(
				TEXT("RoleBindings"),
				TEXT("Weapon"),
				TEXT("Weapons"),
				TEXT("RoleBindings"),
				{
					Field(TEXT("RoleId"), ECFContentValueType::NameId),
					Field(TEXT("Requirement"), ECFContentValueType::Enum),
					Field(TEXT("Cardinality"), ECFContentValueType::Enum),
					Field(TEXT("Ownership"), ECFContentValueType::Enum),
					Field(TEXT("BindingMode"), ECFContentValueType::Enum),
					Field(TEXT("BoundContentId"), ECFContentValueType::NameId),
					Field(TEXT("ParentRoleId"), ECFContentValueType::NameId, true, true),
					Field(TEXT("ActivationCondition"), ECFContentValueType::String),
					Field(TEXT("AuthoringProviderId"), ECFContentValueType::NameId),
					Field(TEXT("State"), ECFContentValueType::Enum)
				}));
		return Descriptor;
	}

	// EquipmentPreset + explicit sortie ammo child schema를 생성합니다.
	FCFContentProviderDescriptor BuildEquipmentProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.EquipmentReview");
		Descriptor.ContentTypeId.Value = TEXT("EquipmentPreset");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("EquipmentPresets"),
				TEXT("EquipmentPreset"),
				{
					Field(TEXT("EquipmentId"), ECFContentValueType::NameId),
					Field(TEXT("DisplayName"), ECFContentValueType::String),
					Field(TEXT("RequiredMountType"), ECFContentValueType::Enum),
					Field(TEXT("RequiredWeaponSize"), ECFContentValueType::Enum),
					Field(TEXT("MountRoleContentId"), ECFContentValueType::ContentReference),
					Field(TEXT("WeaponRoleContentId"), ECFContentValueType::ContentReference),
					Field(TEXT("SensorRoleContentId"), ECFContentValueType::NameId, true, true)
				}));

		FCFContentSheetDescriptor AmmoSheet =
			FCFProdWeaponSchema::BuildAmmoLoadSheetDescriptor();
		AmmoSheet.ContentTypeId.Value = TEXT("EquipmentPreset");
		Descriptor.Sheets.Add(MoveTemp(AmmoSheet));
		return Descriptor;
	}

	// WeaponDefinition schema를 생성합니다.
	FCFContentProviderDescriptor BuildWeaponDefinitionProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.WeaponDefinitionReview");
		Descriptor.ContentTypeId.Value = TEXT("WeaponDefinition");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("WeaponDefinitions"),
				TEXT("WeaponDefinition"),
				{
					Field(TEXT("WeaponId"), ECFContentValueType::NameId),
					Field(TEXT("WeaponSize"), ECFContentValueType::Enum),
					Field(TEXT("CompatibleMountTypes"), ECFContentValueType::String),
					Field(TEXT("WeaponMassKg"), ECFContentValueType::Double, true, false, TEXT("kg")),
					Field(TEXT("FireMode"), ECFContentValueType::Enum),
					Field(TEXT("FireRateRPM"), ECFContentValueType::Double, true, false, TEXT("rpm")),
					Field(TEXT("MaxRangeM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("SpreadDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("MagazineSize"), ECFContentValueType::SignedInteger),
					Field(TEXT("InitialLoadedAmmoCount"), ECFContentValueType::SignedInteger),
					Field(TEXT("AmmoUnitsPerShot"), ECFContentValueType::SignedInteger),
					Field(TEXT("ReloadTimeSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("ReloadMode"), ECFContentValueType::Enum),
					Field(TEXT("AutoReloadWhenEmpty"), ECFContentValueType::Boolean),
					Field(TEXT("AllowPartialReload"), ECFContentValueType::Boolean),
					Field(TEXT("AllowPartialSequence"), ECFContentValueType::Boolean),
					Field(TEXT("UseInfiniteAmmoForDebug"), ECFContentValueType::Boolean),
					Field(TEXT("LauncherPattern"), ECFContentValueType::Enum),
					Field(TEXT("ProjectileCountPerTrigger"), ECFContentValueType::SignedInteger),
					Field(TEXT("InterMuzzleDelaySeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("MaximumSimultaneousLaunchCount"), ECFContentValueType::SignedInteger),
					Field(TEXT("SequenceFailurePolicy"), ECFContentValueType::Enum),
					Field(TEXT("CooldownStartPolicy"), ECFContentValueType::Enum),
					Field(TEXT("LauncherReleaseMode"), ECFContentValueType::Enum),
					Field(TEXT("EjectionSpeedMps"), ECFContentValueType::Double, true, false, TEXT("mps")),
					Field(TEXT("CarrierVelocityRatio"), ECFContentValueType::Double),
					Field(TEXT("LauncherClearanceDistanceM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("MaximumWeaponCharge"), ECFContentValueType::Double),
					Field(TEXT("InitialWeaponCharge"), ECFContentValueType::Double),
					Field(TEXT("ChargeCostPerShot"), ECFContentValueType::Double),
					Field(TEXT("MaximumHeat"), ECFContentValueType::Double),
					Field(TEXT("HeatPerShot"), ECFContentValueType::Double),
					Field(TEXT("ProjectileRoleContentId"), ECFContentValueType::ContentReference),
					Field(TEXT("AmmoRoleContentId"), ECFContentValueType::ContentReference),
					Field(TEXT("FireFxResourceId"), ECFContentValueType::ResourceReference, true, true)
				}));
		return Descriptor;
	}

	// Projectile schema를 생성합니다.
	FCFContentProviderDescriptor BuildProjectileProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.ProjectileReview");
		Descriptor.ContentTypeId.Value = TEXT("Projectile");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("Projectiles"),
				TEXT("Projectile"),
				{
					Field(TEXT("ProjectileId"), ECFContentValueType::NameId),
					Field(TEXT("InitialSpeedMps"), ECFContentValueType::Double, true, false, TEXT("mps")),
					Field(TEXT("AffectedByGravity"), ECFContentValueType::Boolean),
					Field(TEXT("GravityScale"), ECFContentValueType::Double),
					Field(TEXT("LifeTimeSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("CollisionRadiusCm"), ECFContentValueType::Double, true, false, TEXT("cm")),
					Field(TEXT("UseSweepCollision"), ECFContentValueType::Boolean),
					Field(TEXT("CanBeIntercepted"), ECFContentValueType::Boolean),
					Field(TEXT("DetonateWhenIntercepted"), ECFContentValueType::Boolean),
					Field(TEXT("UsePropulsion"), ECFContentValueType::Boolean),
					Field(TEXT("IgnitionDelaySeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("BurnDurationSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("ThrustAccelerationMps2"), ECFContentValueType::Double, true, false, TEXT("mps2")),
					Field(TEXT("MaximumPropelledSpeedMps"), ECFContentValueType::Double, true, false, TEXT("mps")),
					Field(TEXT("UseLaunchAxisStabilization"), ECFContentValueType::Boolean),
					Field(TEXT("MaximumThrustVectorAngleDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("LaunchAxisStabilizationResponseTimeSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("UseMissileFlight"), ECFContentValueType::Boolean),
					Field(TEXT("AttackProfile"), ECFContentValueType::Enum),
					Field(TEXT("MinimumClearanceTimeSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("MinimumClearanceDistanceM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("TransitionDurationSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("UseTerminalPhase"), ECFContentValueType::Boolean),
					Field(TEXT("TerminalPhaseStartDistanceM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("UseGuidance"), ECFContentValueType::Boolean),
					Field(TEXT("GuideMode"), ECFContentValueType::Enum),
					Field(TEXT("LostTargetPolicy"), ECFContentValueType::Enum),
					Field(TEXT("GuidanceLaw"), ECFContentValueType::Enum),
					Field(TEXT("NavigationConstant"), ECFContentValueType::Double),
					Field(TEXT("MaximumTurnRateDegPerSec"), ECFContentValueType::Double, true, false, TEXT("degps")),
					Field(TEXT("MaximumLateralAccelerationMps2"), ECFContentValueType::Double, true, false, TEXT("mps2")),
					Field(TEXT("GuidanceResponseTimeSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("MinimumGuidanceSpeedMps"), ECFContentValueType::Double, true, false, TEXT("mps")),
					Field(TEXT("SeekerModel"), ECFContentValueType::Enum),
					Field(TEXT("TargetObservationMode"), ECFContentValueType::Enum),
					Field(TEXT("GuidanceActivationMode"), ECFContentValueType::Enum),
					Field(TEXT("GuidanceActivationDelaySeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("GuidanceActivationDistanceM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("SeekerFieldOfViewDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("LockBreakAngleDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("TargetLostGraceTimeSeconds"), ECFContentValueType::Double, true, false, TEXT("s")),
					Field(TEXT("ProjectileActorResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("ProjectileMeshResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("TrailFxResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("ThrusterFxResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("ImpactFxResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("DamageRoleContentId"), ECFContentValueType::ContentReference)
				}));
		return Descriptor;
	}

	// Damage schema를 생성합니다.
	FCFContentProviderDescriptor BuildDamageProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.DamageReview");
		Descriptor.ContentTypeId.Value = TEXT("Damage");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("Damage"),
				TEXT("Damage"),
				{
					Field(TEXT("DamageId"), ECFContentValueType::NameId),
					Field(TEXT("DamageType"), ECFContentValueType::Enum),
					Field(TEXT("BaseDamage"), ECFContentValueType::Double),
					Field(TEXT("CanDamageSelf"), ECFContentValueType::Boolean),
					Field(TEXT("ArmorPenetration"), ECFContentValueType::Double),
					Field(TEXT("UseRadialDamage"), ECFContentValueType::Boolean),
					Field(TEXT("ExplosionRadiusM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("ExplosionInnerRadiusM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("ExplosionDamage"), ECFContentValueType::Double),
					Field(TEXT("MinExplosionDamageScale"), ECFContentValueType::Double),
					Field(TEXT("ModuleDamageScale"), ECFContentValueType::Double),
					Field(TEXT("ImpulseStrength"), ECFContentValueType::Double)
				}));
		return Descriptor;
	}

	// Ammo schema를 생성합니다.
	FCFContentProviderDescriptor BuildAmmoProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.AmmoReview");
		Descriptor.ContentTypeId.Value = TEXT("Ammo");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("Ammo"),
				TEXT("Ammo"),
				{
					Field(TEXT("AmmoId"), ECFContentValueType::NameId),
					Field(TEXT("DisplayName"), ECFContentValueType::String),
					Field(TEXT("AmmoFamilyId"), ECFContentValueType::NameId, true, true),
					Field(TEXT("UnitMassKg"), ECFContentValueType::Double, true, false, TEXT("kg")),
					Field(TEXT("MaximumLoadableAmmoCount"), ECFContentValueType::SignedInteger),
					Field(TEXT("CanBeResupplied"), ECFContentValueType::Boolean),
					Field(TEXT("AmmoTags"), ECFContentValueType::String),
					Field(TEXT("AmmoIconResourceId"), ECFContentValueType::ResourceReference, true, true)
				}));
		return Descriptor;
	}

	// TurretMount schema를 생성합니다.
	FCFContentProviderDescriptor BuildTurretProviderDescriptor()
	{
		FCFContentProviderDescriptor Descriptor;
		Descriptor.ProviderId = TEXT("CarFight.Production.TurretReview");
		Descriptor.ContentTypeId.Value = TEXT("TurretMount");
		Descriptor.SchemaRevision = 1;
		Descriptor.Sheets.Add(
			PrimarySheet(
				TEXT("TurretMounts"),
				TEXT("TurretMount"),
				{
					Field(TEXT("TurretMountId"), ECFContentValueType::NameId),
					Field(TEXT("BaseMeshResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("YawMeshResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("PitchMeshResourceId"), ECFContentValueType::ResourceReference, true, true),
					Field(TEXT("YawPivotSemantic"), ECFContentValueType::NameId),
					Field(TEXT("PitchPivotSemantic"), ECFContentValueType::NameId),
					Field(TEXT("MuzzleSemantic"), ECFContentValueType::NameId),
					Field(TEXT("RequireAllMuzzles"), ECFContentValueType::Boolean),
					Field(TEXT("MinYawDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("MaxYawDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("MinPitchDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("MaxPitchDeg"), ECFContentValueType::Double, true, false, TEXT("deg")),
					Field(TEXT("YawTurnRateDegPerSec"), ECFContentValueType::Double, true, false, TEXT("degps")),
					Field(TEXT("PitchTurnRateDegPerSec"), ECFContentValueType::Double, true, false, TEXT("degps")),
					Field(TEXT("AllowFireWhileAligning"), ECFContentValueType::Boolean),
					Field(TEXT("MuzzleClearanceDistanceM"), ECFContentValueType::Double, true, false, TEXT("m")),
					Field(TEXT("MountWeightKg"), ECFContentValueType::Double, true, false, TEXT("kg"))
				}));
		// Muzzle 발사 순서를 보존해야 하므로 OrderedList child sheet를 사용합니다.
		FCFContentSheetDescriptor MuzzleSemanticsSheet =
			KeyedChildSheet(
				TEXT("TurretMuzzleSemantics"),
				TEXT("TurretMount"),
				TEXT("TurretMounts"),
				TEXT("MuzzleSemantics"),
				{
					Field(TEXT("SocketSemantic"), ECFContentValueType::NameId)
				});
		MuzzleSemanticsSheet.CollectionKind = ECFContentCollectionKind::OrderedList;
		Descriptor.Sheets.Add(MoveTemp(MuzzleSemanticsSheet));
		return Descriptor;
	}

	// exact7 provider descriptors를 생성합니다.
	TArray<FCFContentProviderDescriptor> BuildProviderDescriptors()
	{
		return {
			BuildWeaponProviderDescriptor(),
			BuildEquipmentProviderDescriptor(),
			BuildWeaponDefinitionProviderDescriptor(),
			BuildProjectileProviderDescriptor(),
			BuildDamageProviderDescriptor(),
			BuildAmmoProviderDescriptor(),
			BuildTurretProviderDescriptor()
		};
	}

	// One persisted asset type의 Asset Picker contract를 등록합니다.
	bool RegisterResourcePicker(
		const FString& PickerTypeId,
		const UClass& AllowedBaseClass,
		FCFResourcePickerRegistry& OutPickerRegistry,
		FString& OutError)
	{
		FCFResourcePickerDescriptor Picker;
		Picker.PickerTypeId = PickerTypeId;
		Picker.AllowedBaseClassPath = AllowedBaseClass.GetClassPathName();
		Picker.bAllowClear = true;
		return OutPickerRegistry.RegisterPicker(Picker, OutError);
	}

	// Stable ResourceId -> persisted Unreal asset exact mapping을 등록합니다.
	bool RegisterResource(
		const FString& ResourceId,
		const FString& PickerTypeId,
		const FString& ObjectPath,
		const FCFResourcePickerRegistry& PickerRegistry,
		FCFResourceCatalog& OutResourceCatalog,
		FString& OutError)
	{
		FCFResourceDescriptor Resource;
		Resource.ResourceId = ResourceId;
		Resource.PickerTypeId = PickerTypeId;
		Resource.ObjectPath = FSoftObjectPath(ObjectPath);
		return OutResourceCatalog.RegisterResource(
			Resource,
			PickerRegistry,
			OutError);
	}

	// Fresh persisted AssetDump에서 확인한 First Wave reusable resource exact12를 등록합니다.
	bool BuildProductionResourceCatalog(
		FCFResourcePickerRegistry& OutPickerRegistry,
		FCFResourceCatalog& OutResourceCatalog,
		FString& OutError)
	{
		OutError.Reset();

		if (!RegisterResourcePicker(
			TEXT("picker.static_mesh"),
			*UStaticMesh::StaticClass(),
			OutPickerRegistry,
			OutError)
			|| !RegisterResourcePicker(
				TEXT("picker.blueprint"),
				*UBlueprint::StaticClass(),
				OutPickerRegistry,
				OutError)
			|| !RegisterResourcePicker(
				TEXT("picker.combat_fx"),
				*UCFCombatFxData::StaticClass(),
				OutPickerRegistry,
				OutError)
			|| !RegisterResourcePicker(
				TEXT("picker.niagara"),
				*UNiagaraSystem::StaticClass(),
				OutPickerRegistry,
				OutError))
		{
			return false;
		}

		struct FResourceSeed
		{
			// Stable ResourceId입니다.
			const TCHAR* ResourceId;

			// Registered picker type입니다.
			const TCHAR* PickerTypeId;

			// Persisted top-level Unreal asset object path입니다.
			const TCHAR* ObjectPath;
		};

		// Legacy Product identity는 재사용하지 않고 binary resource만 stable ResourceId로 재사용합니다.
		const FResourceSeed ResourceSeeds[] = {
			{ TEXT("mesh.turret.base.standard"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Turrets/_Shared/Meshes/Base_Standard.Base_Standard") },
			{ TEXT("mesh.turret.cannon.yaw"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Turrets/TRT_RoofHeavyCannon/Meshes/RoofHeavyCannon_YawMesh.RoofHeavyCannon_YawMesh") },
			{ TEXT("mesh.turret.cannon.pitch"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Turrets/TRT_RoofHeavyCannon/Meshes/RoofHeavyCannon_PivotMesh.RoofHeavyCannon_PivotMesh") },
			{ TEXT("mesh.turret.rocket.yaw"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Turrets/RocketLauncher/Meshes/RocketLauncherYaw.RocketLauncherYaw") },
			{ TEXT("mesh.turret.rocket.pitch"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Turrets/RocketLauncher/Meshes/RocketLauncherPitch.RocketLauncherPitch") },
			{ TEXT("blueprint.projectile.standard"), TEXT("picker.blueprint"), TEXT("/Game/CarFight/Weapons/Projectiles/Blueprints/BP_CFProjectile.BP_CFProjectile") },
			{ TEXT("mesh.projectile.cannon"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Projectiles/PRJ_HeavyShell/Meshes/PRJ_HeavyShell.PRJ_HeavyShell") },
			{ TEXT("mesh.projectile.rocket"), TEXT("picker.static_mesh"), TEXT("/Game/CarFight/Weapons/Projectiles/Rocket/Meshes/Rocket.Rocket") },
			{ TEXT("fx.weapon.fire.proto"), TEXT("picker.combat_fx"), TEXT("/Game/CarFight/FX/Data/DA_FX_ProtoWeaponFire.DA_FX_ProtoWeaponFire") },
			{ TEXT("fx.projectile.trail.ribbon"), TEXT("picker.niagara"), TEXT("/Game/sA_Megapack_v1/sA_StylizedAttacksPack/FX/NiagaraSystems/NS_RibbonTrail.NS_RibbonTrail") },
			{ TEXT("fx.projectile.thruster.rocket"), TEXT("picker.niagara"), TEXT("/Game/RocketThrusterExhaustFX/FX/NS_RocketExhaust_Realistic.NS_RocketExhaust_Realistic") },
			{ TEXT("fx.projectile.impact.proto"), TEXT("picker.combat_fx"), TEXT("/Game/CarFight/FX/Data/DA_FX_ProtoShellImpact.DA_FX_ProtoShellImpact") }
		};

		for (const FResourceSeed& Seed : ResourceSeeds)
		{
			if (!RegisterResource(
				Seed.ResourceId,
				Seed.PickerTypeId,
				Seed.ObjectPath,
				OutPickerRegistry,
				OutResourceCatalog,
				OutError))
			{
				return false;
			}
		}
		return true;
	}

	// Persisted turret mesh가 exact required socket semantic을 제공하는지 검증합니다.
	bool ValidateSocketCapability(
		const FString& ResourceId,
		const FName SocketName,
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		int32& InOutCheckCount,
		FString& OutError)
	{
		FCFResourceCapabilitySnapshot Snapshot;
		TArray<FCFContentValidationIssue> Issues;
		if (!FCFMeshCapabilityScanner::ScanResource(
			ResourceId,
			ResourceCatalog,
			PickerRegistry,
			Snapshot,
			Issues))
		{
			OutError = Issues.IsEmpty()
				? TEXT("Required turret mesh capability scan에 실패했습니다.")
				: Issues[0].Code + TEXT(": ") + Issues[0].Message;
			return false;
		}

		if (!FCFMeshCapabilityScanner::HasCapability(
			Snapshot,
			ECFResourceCapabilityKind::Socket,
			SocketName))
		{
			OutError = FString::Printf(
				TEXT("Resource %s 에 required socket %s 가 없습니다."),
				*ResourceId,
				*SocketName.ToString());
			return false;
		}

		++InOutCheckCount;
		return true;
	}

	// Cannon/Rocket persisted turret socket exact8 capability proof를 수행합니다.
	bool ValidateProductionSocketCapabilities(
		const FCFResourceCatalog& ResourceCatalog,
		const FCFResourcePickerRegistry& PickerRegistry,
		int32& OutCheckCount,
		FString& OutError)
	{
		OutCheckCount = 0;
		OutError.Reset();

		struct FSocketRequirement
		{
			// Socket을 소유하는 stable ResourceId입니다.
			const TCHAR* ResourceId;

			// Required socket semantic입니다.
			const TCHAR* SocketName;
		};

		// Fresh persisted AssetDump에서 실제 존재를 확인한 First Wave socket semantics입니다.
		const FSocketRequirement Requirements[] = {
			{ TEXT("mesh.turret.base.standard"), TEXT("YawPivot") },
			{ TEXT("mesh.turret.cannon.yaw"), TEXT("PitchPivot") },
			{ TEXT("mesh.turret.cannon.pitch"), TEXT("Muzzle") },
			{ TEXT("mesh.turret.rocket.yaw"), TEXT("PitchPivot") },
			{ TEXT("mesh.turret.rocket.pitch"), TEXT("Muzzle_1") },
			{ TEXT("mesh.turret.rocket.pitch"), TEXT("Muzzle_4") },
			{ TEXT("mesh.turret.rocket.pitch"), TEXT("Muzzle_2") },
			{ TEXT("mesh.turret.rocket.pitch"), TEXT("Muzzle_3") }
		};

		for (const FSocketRequirement& Requirement : Requirements)
		{
			if (!ValidateSocketCapability(
				Requirement.ResourceId,
				FName(Requirement.SocketName),
				ResourceCatalog,
				PickerRegistry,
				OutCheckCount,
				OutError))
			{
				return false;
			}
		}
		return OutCheckCount == 8;
	}

	// Provider descriptors를 하나의 frozen Workbook schema로 결합합니다.
	FCFContentWorkbookModel BuildEmptyWorkbook(
		const TArray<FCFContentProviderDescriptor>& Descriptors)
	{
		FCFContentWorkbookModel Workbook;
		Workbook.WorkbookSourceId = TEXT("CarFight.Content.Production");
		Workbook.SchemaRevision = 1;
		for (const FCFContentProviderDescriptor& Descriptor : Descriptors)
		{
			Workbook.Sheets.Append(Descriptor.Sheets);
		}
		return Workbook;
	}

	// Product role가 shared target인지 current exact8 specs 전체에서 계산합니다.
	bool IsSharedRoleTarget(
		const TArray<FProductSpec>& Specs,
		const FString& RoleId,
		const FString& ContentId)
	{
		int32 ReferenceCount = 0;
		for (const FProductSpec& Spec : Specs)
		{
			FString CandidateId;
			if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
			{
				CandidateId = Spec.MountId;
			}
			else if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
			{
				CandidateId = Spec.ProjectileId;
			}
			else if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
			{
				CandidateId = Spec.DamageId;
			}
			else if (RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive))
			{
				CandidateId = Spec.AmmoId;
			}

			if (CandidateId.Equals(ContentId, ESearchCase::CaseSensitive))
			{
				++ReferenceCount;
			}
		}
		return ReferenceCount > 1;
	}

	// Product/Role 조합의 binding mode를 frozen single-writer 계약으로 계산합니다.
	FString BindingModeForRole(
		const FProductSpec& Spec,
		const FString& RoleId,
		const bool bShared)
	{
		if (!bShared)
		{
			return TEXT("CreateNew");
		}

		if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive) && Spec.bCreateSharedMount)
		{
			return TEXT("CreateNewShared");
		}
		if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive) && Spec.bCreateSharedProjectile)
		{
			return TEXT("CreateNewShared");
		}
		if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive) && Spec.bCreateSharedDamage)
		{
			return TEXT("CreateNewShared");
		}
		if (RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive) && Spec.bCreateSharedAmmo)
		{
			return TEXT("CreateNewShared");
		}
		return TEXT("BindShared");
	}

	// RoleId의 BoundContentId를 Product spec에서 반환합니다.
	FString ContentIdForRole(
		const FProductSpec& Spec,
		const FString& RoleId)
	{
		if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
		{
			return Spec.ProductId;
		}
		if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
		{
			return Spec.MountId;
		}
		if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
		{
			return Spec.ProductId;
		}
		if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
		{
			return Spec.ProjectileId;
		}
		if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
		{
			return Spec.DamageId;
		}
		if (RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive))
		{
			return Spec.AmmoId;
		}
		return FString();
	}

	// RoleId의 authoring provider identity를 반환합니다.
	FString ProviderIdForRole(const FString& RoleId)
	{
		if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
		{
			return TEXT("CarFight.EquipmentBuilder");
		}
		if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
		{
			return TEXT("CarFight.WeaponGuide.Turret");
		}
		if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
		{
			return TEXT("CarFight.WeaponGuide.Weapon");
		}
		if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
		{
			return TEXT("CarFight.WeaponGuide.Projectile");
		}
		if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
		{
			return TEXT("CarFight.DamageProvider");
		}
		if (RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive))
		{
			return TEXT("CarFight.AmmoProvider");
		}
		return TEXT("CarFight.Unknown");
	}

	// Product RoleBindings exact6 collection을 생성합니다.
	FCFContentCollection BuildRoleBindings(
		const TArray<FProductSpec>& Specs,
		const FProductSpec& Spec)
	{
		FCFContentCollection Collection;
		Collection.CollectionId = TEXT("RoleBindings");
		Collection.Kind = ECFContentCollectionKind::KeyedCollection;

		const TArray<FString> RoleIds = {
			TEXT("EquipmentPreset"),
			TEXT("Mount"),
			TEXT("WeaponDefinition"),
			TEXT("Projectile"),
			TEXT("Damage"),
			TEXT("Ammo")
		};

		for (const FString& RoleId : RoleIds)
		{
			const FString BoundContentId = ContentIdForRole(Spec, RoleId);
			const bool bShared =
				RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive)
				|| RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive)
					? false
					: IsSharedRoleTarget(Specs, RoleId, BoundContentId);

			FCFContentCollectionItem Item;
			Item.ChildItemId = TEXT("Role.") + RoleId;
			Item.DomainKey = RoleId;
			Item.Fields.Add(TEXT("RoleId"), NameValue(RoleId));
			Item.Fields.Add(TEXT("Requirement"), EnumValue(TEXT("Required")));
			Item.Fields.Add(TEXT("Cardinality"), EnumValue(TEXT("Single")));
			Item.Fields.Add(
				TEXT("Ownership"),
				EnumValue(bShared ? TEXT("Shared") : TEXT("VariantOwned")));
			Item.Fields.Add(
				TEXT("BindingMode"),
				EnumValue(BindingModeForRole(Spec, RoleId, bShared)));
			Item.Fields.Add(TEXT("BoundContentId"), NameValue(BoundContentId));
			Item.Fields.Add(TEXT("ParentRoleId"), NoneNameValue());
			Item.Fields.Add(TEXT("ActivationCondition"), StringValue(TEXT("Always")));
			Item.Fields.Add(TEXT("AuthoringProviderId"), NameValue(ProviderIdForRole(RoleId)));
			Item.Fields.Add(TEXT("State"), EnumValue(TEXT("Active")));
			Collection.Items.Add(MoveTemp(Item));
		}
		return Collection;
	}

	// Product explicit sortie ammo collection exact1을 생성합니다.
	FCFContentCollection BuildAmmoLoads(const FProductSpec& Spec)
	{
		FCFContentCollection Collection;
		Collection.CollectionId = TEXT("EquipmentPresetAmmoLoads");
		Collection.Kind = ECFContentCollectionKind::KeyedCollection;

		FCFContentCollectionItem Item;
		Item.ChildItemId = TEXT("AmmoLoad.") + Spec.AmmoId;
		Item.DomainKey = Spec.AmmoId;
		Item.Fields.Add(TEXT("AmmoRoleContentId"), NameValue(Spec.AmmoId));
		Item.Fields.Add(TEXT("DefaultSortieAmmoCount"), IntValue(Spec.DefaultSortieAmmoCount));
		Collection.Items.Add(MoveTemp(Item));
		return Collection;
	}

	// Product planning metadata를 current Roster 의미로 생성합니다.
	FCFContentAuthoringMetadata BuildProductMetadata(
		const TArray<FProductSpec>& Specs,
		const FProductSpec& Spec)
	{
		FCFContentAuthoringMetadata Metadata;
		Metadata.FamilyId = Spec.FamilyId;
		Metadata.BaseContentId = Spec.BaseProductId;
		Metadata.DesignIntent = Spec.DesignIntent;
		Metadata.Planning.VariantId = Spec.VariantId;
		Metadata.Planning.RoleId = Spec.GameplayRole;
		Metadata.Planning.ProductionWaveId = Spec.ProductionWaveId;
		// CF-FQ-056 PFP-P0-03은 USER ACCEPTED / COMPLETE이므로 First Wave exact8 모두 authoring-ready입니다.
		Metadata.Planning.Readiness = ECFContentPlanningReadiness::AuthoringReady;

		const TArray<FString> RoleIds = {
			TEXT("EquipmentPreset"),
			TEXT("Mount"),
			TEXT("WeaponDefinition"),
			TEXT("Projectile"),
			TEXT("Damage"),
			TEXT("Ammo")
		};
		for (const FString& RoleId : RoleIds)
		{
			const FString BoundContentId = ContentIdForRole(Spec, RoleId);
			const bool bShared =
				RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive)
				|| RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive)
					? false
					: IsSharedRoleTarget(Specs, RoleId, BoundContentId);
			if (bShared)
			{
				Metadata.Planning.SharedDataRoleIds.Add(RoleId);
			}
			else
			{
				Metadata.Planning.VariantOwnedDataRoleIds.Add(RoleId);
			}
		}

		return Metadata;
	}

	// Weapon Product top-level record를 생성합니다.
	FCFContentRecord BuildWeaponRecord(
		const TArray<FProductSpec>& Specs,
		const FProductSpec& Spec)
	{
		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("Weapon"), Spec.ProductId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.AuthoringMetadata = BuildProductMetadata(Specs, Spec);
		Record.Fields.Add(TEXT("DisplayName"), StringValue(Spec.ProductId));
		Record.Fields.Add(TEXT("FamilyId"), NameValue(Spec.FamilyId));
		Record.Fields.Add(TEXT("VariantId"), NameValue(Spec.VariantId));
		Record.Fields.Add(TEXT("GameplayRole"), NameValue(Spec.GameplayRole));
		Record.Fields.Add(TEXT("DesignIntent"), StringValue(Spec.DesignIntent));
		Record.Fields.Add(TEXT("RelativeIntent"), StringValue(TEXT("See typed planning metadata and initial balance baseline.")));
		Record.Fields.Add(
			TEXT("BaseContentId"),
			Spec.BaseProductId.IsEmpty() ? NoneNameValue() : NameValue(Spec.BaseProductId));
		Record.Fields.Add(TEXT("ProductionWave"), NameValue(Spec.ProductionWaveId));
		Record.Fields.Add(TEXT("Readiness"), EnumValue(TEXT("AuthoringReady")));
		Record.Fields.Add(TEXT("Lifecycle"), EnumValue(TEXT("Active")));
		Record.Collections.Add(TEXT("RoleBindings"), BuildRoleBindings(Specs, Spec));
		return Record;
	}

	// EquipmentPreset top-level record를 생성합니다.
	FCFContentRecord BuildEquipmentRecord(const FProductSpec& Spec)
	{
		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("EquipmentPreset"), Spec.ProductId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("EquipmentId"), NameValue(Spec.ProductId));
		Record.Fields.Add(TEXT("DisplayName"), StringValue(Spec.ProductId));
		Record.Fields.Add(TEXT("RequiredMountType"), EnumValue(TEXT("Turret")));
		Record.Fields.Add(TEXT("RequiredWeaponSize"), EnumValue(TEXT("Large")));
		Record.Fields.Add(TEXT("MountRoleContentId"), ReferenceValue(TEXT("TurretMount"), Spec.MountId));
		Record.Fields.Add(TEXT("WeaponRoleContentId"), ReferenceValue(TEXT("WeaponDefinition"), Spec.ProductId));
		Record.Fields.Add(TEXT("SensorRoleContentId"), NoneNameValue());
		Record.Collections.Add(TEXT("EquipmentPresetAmmoLoads"), BuildAmmoLoads(Spec));
		return Record;
	}

	// WeaponDefinition top-level record를 생성합니다.
	FCFContentRecord BuildWeaponDefinitionRecord(const FProductSpec& Spec)
	{
		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("WeaponDefinition"), Spec.ProductId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("WeaponId"), NameValue(Spec.ProductId));
		Record.Fields.Add(TEXT("WeaponSize"), EnumValue(TEXT("Large")));
		Record.Fields.Add(TEXT("CompatibleMountTypes"), StringValue(TEXT("Turret")));
		Record.Fields.Add(TEXT("WeaponMassKg"), DoubleValue(Spec.WeaponMassKg));
		Record.Fields.Add(TEXT("FireMode"), EnumValue(TEXT("Projectile")));
		Record.Fields.Add(TEXT("FireRateRPM"), DoubleValue(Spec.FireRatePerMinute));
		Record.Fields.Add(TEXT("MaxRangeM"), DoubleValue(Spec.MaxRangeMeters));
		Record.Fields.Add(TEXT("SpreadDeg"), DoubleValue(Spec.SpreadDeg));
		Record.Fields.Add(TEXT("MagazineSize"), IntValue(Spec.MagazineSize));
		Record.Fields.Add(TEXT("InitialLoadedAmmoCount"), IntValue(Spec.InitialLoadedAmmoCount));
		Record.Fields.Add(TEXT("AmmoUnitsPerShot"), IntValue(1));
		Record.Fields.Add(TEXT("ReloadTimeSeconds"), DoubleValue(Spec.ReloadTimeSeconds));
		Record.Fields.Add(TEXT("ReloadMode"), EnumValue(TEXT("FullMagazine")));
		Record.Fields.Add(TEXT("AutoReloadWhenEmpty"), BoolValue(true));
		Record.Fields.Add(TEXT("AllowPartialReload"), BoolValue(true));
		Record.Fields.Add(TEXT("AllowPartialSequence"), BoolValue(true));
		Record.Fields.Add(TEXT("UseInfiniteAmmoForDebug"), BoolValue(false));
		Record.Fields.Add(TEXT("LauncherPattern"), EnumValue(Spec.LauncherFirePattern));
		Record.Fields.Add(TEXT("ProjectileCountPerTrigger"), IntValue(Spec.ProjectileCountPerTrigger));
		Record.Fields.Add(TEXT("InterMuzzleDelaySeconds"), DoubleValue(Spec.InterMuzzleDelaySeconds));
		Record.Fields.Add(TEXT("MaximumSimultaneousLaunchCount"), IntValue(Spec.MaximumSimultaneousLaunchCount));
		Record.Fields.Add(TEXT("SequenceFailurePolicy"), EnumValue(TEXT("ContinueRemaining")));
		Record.Fields.Add(TEXT("CooldownStartPolicy"), EnumValue(TEXT("FirstAcceptedProjectile")));
		Record.Fields.Add(TEXT("LauncherReleaseMode"), EnumValue(TEXT("Direct")));
		Record.Fields.Add(TEXT("EjectionSpeedMps"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("CarrierVelocityRatio"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("LauncherClearanceDistanceM"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("MaximumWeaponCharge"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("InitialWeaponCharge"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("ChargeCostPerShot"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("MaximumHeat"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("HeatPerShot"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("ProjectileRoleContentId"), ReferenceValue(TEXT("Projectile"), Spec.ProjectileId));
		Record.Fields.Add(TEXT("AmmoRoleContentId"), ReferenceValue(TEXT("Ammo"), Spec.AmmoId));
		Record.Fields.Add(TEXT("FireFxResourceId"), ResourceValue(TEXT("fx.weapon.fire.proto")));
		return Record;
	}

	// Projectile top-level record를 생성합니다.
	FCFContentRecord BuildProjectileRecord(const FProductSpec& Spec)
	{
		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("Projectile"), Spec.ProjectileId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("ProjectileId"), NameValue(Spec.ProjectileId));
		Record.Fields.Add(TEXT("InitialSpeedMps"), DoubleValue(Spec.InitialSpeedMetersPerSecond));
		Record.Fields.Add(TEXT("AffectedByGravity"), BoolValue(true));
		Record.Fields.Add(TEXT("GravityScale"), DoubleValue(1.0));
		Record.Fields.Add(TEXT("LifeTimeSeconds"), DoubleValue(Spec.ProjectileLifeTimeSeconds));
		Record.Fields.Add(TEXT("CollisionRadiusCm"), DoubleValue(8.0));
		Record.Fields.Add(TEXT("UseSweepCollision"), BoolValue(true));
		Record.Fields.Add(TEXT("CanBeIntercepted"), BoolValue(true));
		Record.Fields.Add(TEXT("DetonateWhenIntercepted"), BoolValue(true));
		Record.Fields.Add(TEXT("UsePropulsion"), BoolValue(Spec.bUsePropulsion));
		Record.Fields.Add(TEXT("IgnitionDelaySeconds"), DoubleValue(Spec.IgnitionDelaySeconds));
		Record.Fields.Add(TEXT("BurnDurationSeconds"), DoubleValue(Spec.BurnDurationSeconds));
		Record.Fields.Add(TEXT("ThrustAccelerationMps2"), DoubleValue(Spec.ThrustAccelerationMetersPerSecondSq));
		Record.Fields.Add(TEXT("MaximumPropelledSpeedMps"), DoubleValue(Spec.MaximumPropelledSpeedMetersPerSecond));
		Record.Fields.Add(TEXT("UseLaunchAxisStabilization"), BoolValue(Spec.bUseLaunchAxisStabilization));
		Record.Fields.Add(TEXT("MaximumThrustVectorAngleDeg"), DoubleValue(Spec.MaximumThrustVectorAngleDeg));
		Record.Fields.Add(TEXT("LaunchAxisStabilizationResponseTimeSeconds"), DoubleValue(Spec.LaunchAxisResponseTimeSeconds));
		Record.Fields.Add(TEXT("UseMissileFlight"), BoolValue(Spec.bUseMissileFlight));
		Record.Fields.Add(TEXT("AttackProfile"), EnumValue(TEXT("Direct")));
		Record.Fields.Add(TEXT("MinimumClearanceTimeSeconds"), DoubleValue(Spec.bUseMissileFlight ? 0.15 : 0.0));
		Record.Fields.Add(TEXT("MinimumClearanceDistanceM"), DoubleValue(Spec.bUseMissileFlight ? 3.0 : 0.0));
		Record.Fields.Add(TEXT("TransitionDurationSeconds"), DoubleValue(Spec.bUseMissileFlight ? 0.35 : 0.0));
		Record.Fields.Add(TEXT("UseTerminalPhase"), BoolValue(false));
		Record.Fields.Add(TEXT("TerminalPhaseStartDistanceM"), DoubleValue(15.0));
		Record.Fields.Add(TEXT("UseGuidance"), BoolValue(Spec.bUseGuidance));
		Record.Fields.Add(TEXT("GuideMode"), EnumValue(Spec.bUseGuidance ? TEXT("TargetActor") : TEXT("None")));
		Record.Fields.Add(TEXT("LostTargetPolicy"), EnumValue(TEXT("ContinueStraight")));
		Record.Fields.Add(TEXT("GuidanceLaw"), EnumValue(TEXT("ProportionalNavigation")));
		Record.Fields.Add(TEXT("NavigationConstant"), DoubleValue(Spec.bUseGuidance ? 3.0 : 0.0));
		Record.Fields.Add(TEXT("MaximumTurnRateDegPerSec"), DoubleValue(Spec.bUseGuidance ? 70.0 : 0.0));
		Record.Fields.Add(TEXT("MaximumLateralAccelerationMps2"), DoubleValue(Spec.bUseGuidance ? 90.0 : 0.0));
		Record.Fields.Add(TEXT("GuidanceResponseTimeSeconds"), DoubleValue(Spec.bUseGuidance ? 0.15 : 0.0));
		Record.Fields.Add(TEXT("MinimumGuidanceSpeedMps"), DoubleValue(Spec.bUseGuidance ? 8.0 : 0.0));
		Record.Fields.Add(TEXT("SeekerModel"), EnumValue(TEXT("LegacySingleGate")));
		Record.Fields.Add(TEXT("TargetObservationMode"), EnumValue(TEXT("DirectActorKinematics")));
		Record.Fields.Add(TEXT("GuidanceActivationMode"), EnumValue(TEXT("FollowFlightGuidanceWindow")));
		Record.Fields.Add(TEXT("GuidanceActivationDelaySeconds"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("GuidanceActivationDistanceM"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("SeekerFieldOfViewDeg"), DoubleValue(Spec.bUseGuidance ? 60.0 : 0.0));
		Record.Fields.Add(TEXT("LockBreakAngleDeg"), DoubleValue(Spec.bUseGuidance ? 85.0 : 0.0));
		Record.Fields.Add(TEXT("TargetLostGraceTimeSeconds"), DoubleValue(Spec.bUseGuidance ? 0.25 : 0.0));
		Record.Fields.Add(TEXT("ProjectileActorResourceId"), ResourceValue(TEXT("blueprint.projectile.standard")));
		Record.Fields.Add(
			TEXT("ProjectileMeshResourceId"),
			ResourceValue(
				Spec.FamilyId.Equals(TEXT("Cannon"), ESearchCase::CaseSensitive)
					? TEXT("mesh.projectile.cannon")
					: TEXT("mesh.projectile.rocket")));
		Record.Fields.Add(TEXT("TrailFxResourceId"), ResourceValue(TEXT("fx.projectile.trail.ribbon")));
		Record.Fields.Add(
			TEXT("ThrusterFxResourceId"),
			Spec.bUsePropulsion
				? ResourceValue(TEXT("fx.projectile.thruster.rocket"))
				: NoneResourceValue());
		Record.Fields.Add(TEXT("ImpactFxResourceId"), ResourceValue(TEXT("fx.projectile.impact.proto")));
		Record.Fields.Add(TEXT("DamageRoleContentId"), ReferenceValue(TEXT("Damage"), Spec.DamageId));
		return Record;
	}

	// Damage top-level record를 생성합니다.
	FCFContentRecord BuildDamageRecord(
		const FString& DamageId,
		const double BaseDamage)
	{
		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("Damage"), DamageId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("DamageId"), NameValue(DamageId));
		Record.Fields.Add(TEXT("DamageType"), EnumValue(TEXT("Kinetic")));
		Record.Fields.Add(TEXT("BaseDamage"), DoubleValue(BaseDamage));
		Record.Fields.Add(TEXT("CanDamageSelf"), BoolValue(false));
		Record.Fields.Add(TEXT("ArmorPenetration"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("UseRadialDamage"), BoolValue(false));
		Record.Fields.Add(TEXT("ExplosionRadiusM"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("ExplosionInnerRadiusM"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("ExplosionDamage"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("MinExplosionDamageScale"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("ModuleDamageScale"), DoubleValue(0.0));
		Record.Fields.Add(TEXT("ImpulseStrength"), DoubleValue(0.0));
		return Record;
	}

	// Ammo top-level record를 생성합니다.
	FCFContentRecord BuildAmmoRecord(
		const FString& AmmoId,
		const double UnitMassKg,
		const int64 MaximumLoadableAmmoCount)
	{
		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("Ammo"), AmmoId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("AmmoId"), NameValue(AmmoId));
		Record.Fields.Add(TEXT("DisplayName"), StringValue(AmmoId));
		Record.Fields.Add(
			TEXT("AmmoFamilyId"),
			AmmoId.Equals(TEXT("Ammo_Rocket"), ESearchCase::CaseSensitive)
				? NameValue(TEXT("Rocket_Standard_Warhead"))
				: NoneNameValue());
		Record.Fields.Add(TEXT("UnitMassKg"), DoubleValue(UnitMassKg));
		Record.Fields.Add(TEXT("MaximumLoadableAmmoCount"), IntValue(MaximumLoadableAmmoCount));
		Record.Fields.Add(TEXT("CanBeResupplied"), BoolValue(true));
		Record.Fields.Add(TEXT("AmmoTags"), StringValue(TEXT("Production")));
		Record.Fields.Add(TEXT("AmmoIconResourceId"), NoneResourceValue());
		return Record;
	}

	// Persisted current Cannon/Rocket turret evidence로 Production TurretMount candidate를 생성합니다.
	FCFContentRecord BuildTurretRecord(const FString& MountId)
	{
		// Cannon/Rocket pod 구분입니다.
		const bool bCannonMount =
			MountId.Equals(TEXT("Mount_Cannon"), ESearchCase::CaseSensitive);

		FCFContentRecord Record;
		Record.Key = MakeKey(TEXT("TurretMount"), MountId);
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("TurretMountId"), NameValue(MountId));
		Record.Fields.Add(TEXT("BaseMeshResourceId"), ResourceValue(TEXT("mesh.turret.base.standard")));
		Record.Fields.Add(
			TEXT("YawMeshResourceId"),
			ResourceValue(
				bCannonMount
					? TEXT("mesh.turret.cannon.yaw")
					: TEXT("mesh.turret.rocket.yaw")));
		Record.Fields.Add(
			TEXT("PitchMeshResourceId"),
			ResourceValue(
				bCannonMount
					? TEXT("mesh.turret.cannon.pitch")
					: TEXT("mesh.turret.rocket.pitch")));
		Record.Fields.Add(TEXT("YawPivotSemantic"), NameValue(TEXT("YawPivot")));
		Record.Fields.Add(TEXT("PitchPivotSemantic"), NameValue(TEXT("PitchPivot")));
		Record.Fields.Add(
			TEXT("MuzzleSemantic"),
			NameValue(bCannonMount ? TEXT("Muzzle") : TEXT("Muzzle_1")));
		Record.Fields.Add(TEXT("RequireAllMuzzles"), BoolValue(!bCannonMount));
		Record.Fields.Add(TEXT("MinYawDeg"), DoubleValue(-180.0));
		Record.Fields.Add(TEXT("MaxYawDeg"), DoubleValue(180.0));
		Record.Fields.Add(TEXT("MinPitchDeg"), DoubleValue(-8.0));
		Record.Fields.Add(TEXT("MaxPitchDeg"), DoubleValue(25.0));
		Record.Fields.Add(TEXT("YawTurnRateDegPerSec"), DoubleValue(35.0));
		Record.Fields.Add(TEXT("PitchTurnRateDegPerSec"), DoubleValue(20.0));
		Record.Fields.Add(TEXT("AllowFireWhileAligning"), BoolValue(true));
		Record.Fields.Add(TEXT("MuzzleClearanceDistanceM"), DoubleValue(1.5));
		Record.Fields.Add(TEXT("MountWeightKg"), DoubleValue(bCannonMount ? 200.0 : 150.0));

		// Persisted current turret의 semantic muzzle order를 보존합니다.
		FCFContentCollection MuzzleSemantics;
		MuzzleSemantics.CollectionId = TEXT("MuzzleSemantics");
		MuzzleSemantics.Kind = ECFContentCollectionKind::OrderedList;

		// Current Production candidate가 사용할 exact muzzle semantic order입니다.
		const TArray<FString> MuzzleOrder = bCannonMount
			? TArray<FString>{ TEXT("Muzzle") }
			: TArray<FString>{ TEXT("Muzzle_1"), TEXT("Muzzle_4"), TEXT("Muzzle_2"), TEXT("Muzzle_3") };
		for (int32 MuzzleIndex = 0; MuzzleIndex < MuzzleOrder.Num(); ++MuzzleIndex)
		{
			FCFContentCollectionItem MuzzleItem;
			MuzzleItem.ChildItemId = FString::Printf(TEXT("Muzzle%d"), MuzzleIndex + 1);
			MuzzleItem.Order = MuzzleIndex;
			MuzzleItem.Fields.Add(
				TEXT("SocketSemantic"),
				NameValue(MuzzleOrder[MuzzleIndex]));
			MuzzleSemantics.Items.Add(MoveTemp(MuzzleItem));
		}
		Record.Collections.Add(TEXT("MuzzleSemantics"), MoveTemp(MuzzleSemantics));
		return Record;
	}

	// Specs에서 exact8 Product + unique exact34 DA target canonical Workbook을 구성합니다.
	FCFContentWorkbookModel BuildCandidateWorkbook(
		const FCFContentWorkbookModel& EmptyWorkbook,
		const TArray<FProductSpec>& Specs)
	{
		FCFContentWorkbookModel Workbook = EmptyWorkbook;

		TSet<FString> AddedTargetKeys;

		for (const FProductSpec& Spec : Specs)
		{
			Workbook.Records.Add(BuildWeaponRecord(Specs, Spec));

			const FCFContentRecord EquipmentRecord = BuildEquipmentRecord(Spec);
			if (!AddedTargetKeys.Contains(EquipmentRecord.Key.ToStableString()))
			{
				Workbook.Records.Add(EquipmentRecord);
				AddedTargetKeys.Add(EquipmentRecord.Key.ToStableString());
			}

			const FCFContentRecord WeaponRecord = BuildWeaponDefinitionRecord(Spec);
			if (!AddedTargetKeys.Contains(WeaponRecord.Key.ToStableString()))
			{
				Workbook.Records.Add(WeaponRecord);
				AddedTargetKeys.Add(WeaponRecord.Key.ToStableString());
			}

			const FCFContentRecord ProjectileRecord = BuildProjectileRecord(Spec);
			if (!AddedTargetKeys.Contains(ProjectileRecord.Key.ToStableString()))
			{
				Workbook.Records.Add(ProjectileRecord);
				AddedTargetKeys.Add(ProjectileRecord.Key.ToStableString());
			}

			const FCFContentRecord DamageRecord =
				BuildDamageRecord(Spec.DamageId, Spec.BaseDamage);
			if (!AddedTargetKeys.Contains(DamageRecord.Key.ToStableString()))
			{
				Workbook.Records.Add(DamageRecord);
				AddedTargetKeys.Add(DamageRecord.Key.ToStableString());
			}

			const FCFContentRecord AmmoRecord =
				BuildAmmoRecord(
					Spec.AmmoId,
					Spec.AmmoUnitMassKg,
					Spec.AmmoMaximumLoadableCount);
			if (!AddedTargetKeys.Contains(AmmoRecord.Key.ToStableString()))
			{
				Workbook.Records.Add(AmmoRecord);
				AddedTargetKeys.Add(AmmoRecord.Key.ToStableString());
			}

			const FCFContentRecord MountRecord = BuildTurretRecord(Spec.MountId);
			if (!AddedTargetKeys.Contains(MountRecord.Key.ToStableString()))
			{
				Workbook.Records.Add(MountRecord);
				AddedTargetKeys.Add(MountRecord.Key.ToStableString());
			}
		}

		Workbook.Records.Sort(
			[](const FCFContentRecord& Left, const FCFContentRecord& Right)
			{
				return Left.Key.ToStableString() < Right.Key.ToStableString();
			});
		return Workbook;
	}

	// ContentType에 대응하는 primary SheetId를 찾습니다.
	const FCFContentSheetDescriptor* FindPrimarySheet(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentTypeId& ContentTypeId)
	{
		return Workbook.Sheets.FindByPredicate(
			[&ContentTypeId](const FCFContentSheetDescriptor& Sheet)
			{
				return !Sheet.IsChildSheet() && Sheet.ContentTypeId == ContentTypeId;
			});
	}

	// Record collection에 대응하는 child SheetId를 찾습니다.
	const FCFContentSheetDescriptor* FindChildSheet(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentTypeId& ContentTypeId,
		const FString& ParentSheetId,
		const FString& CollectionId)
	{
		return Workbook.Sheets.FindByPredicate(
			[&ContentTypeId, &ParentSheetId, &CollectionId](const FCFContentSheetDescriptor& Sheet)
			{
				return Sheet.IsChildSheet()
					&& Sheet.ContentTypeId == ContentTypeId
					&& Sheet.ParentSheetId.Equals(ParentSheetId, ESearchCase::CaseSensitive)
					&& Sheet.CollectionId.Equals(CollectionId, ESearchCase::CaseSensitive);
			});
	}

	// Candidate Workbook 전체를 empty snapshot에 적용할 typed Change Proposal로 변환합니다.
	bool BuildAddProposal(
		const FCFContentCatalogSnapshot& BaseSnapshot,
		const FCFContentWorkbookModel& CandidateWorkbook,
		FCFContentChangeProposal& OutProposal,
		FString& OutError)
	{
		OutProposal = FCFContentChangeProposal();
		OutError.Reset();
		OutProposal.ChangeSet.ChangeSetId =
			TEXT("CF.FQ.058.ProductionWave1.Exact34Candidate.v1");
		OutProposal.ChangeSet.BaseWorkbookSemanticHash =
			BaseSnapshot.WorkbookSemanticHash;
		OutProposal.ChangeSet.BaseCatalogSnapshotFingerprint =
			BaseSnapshot.SnapshotFingerprint;
		OutProposal.ChangeSet.WorkbookSchemaVersion =
			BaseSnapshot.WorkbookSchemaVersion;
		OutProposal.ChangeSet.Reason =
			TEXT("Prepare the first Production exact8/exact34 candidate without persistent mutation.");
		OutProposal.ChangeSet.DesignIntent =
			TEXT("Freeze current Roster topology, balance and naming into a deterministic review package before USER cutover acceptance.");
		OutProposal.BaseCatalogSnapshotFingerprint =
			BaseSnapshot.SnapshotFingerprint;

		TMap<FString, TArray<int32>> OperationIndexesByRecord;

		for (const FCFContentRecord& Record : CandidateWorkbook.Records)
		{
			const FCFContentSheetDescriptor* Primary =
				FindPrimarySheet(CandidateWorkbook, Record.Key.ContentTypeId);
			if (Primary == nullptr)
			{
				OutError =
					TEXT("Candidate record ContentType에 대응하는 primary sheet가 없습니다: ")
					+ Record.Key.ToStableString();
				return false;
			}

			FCFContentChangeOperation AddOperation;
			AddOperation.OperationType = ECFContentChangeOperationType::AddRecord;
			AddOperation.SheetId = Primary->SheetId;
			AddOperation.ContentKey = Record.Key;
			AddOperation.RowId = Record.RowId;
			OperationIndexesByRecord.FindOrAdd(Record.Key.ToStableString())
				.Add(OutProposal.ChangeSet.Operations.Add(MoveTemp(AddOperation)));

			TArray<FString> FieldIds;
			Record.Fields.GetKeys(FieldIds);
			FieldIds.Sort();
			for (const FString& FieldId : FieldIds)
			{
				const FCFContentValue* Value = Record.Fields.Find(FieldId);
				if (Value == nullptr)
				{
					continue;
				}

				FCFContentChangeOperation FieldOperation;
				FieldOperation.OperationType = ECFContentChangeOperationType::UpdateField;
				FieldOperation.SheetId = Primary->SheetId;
				FieldOperation.ContentKey = Record.Key;
				FieldOperation.RowId = Record.RowId;
				FieldOperation.ColumnId = FieldId;
				FieldOperation.Value = *Value;
				OperationIndexesByRecord.FindOrAdd(Record.Key.ToStableString())
					.Add(OutProposal.ChangeSet.Operations.Add(MoveTemp(FieldOperation)));
			}

			const bool bHasPlanningMetadata =
				!Record.AuthoringMetadata.FamilyId.IsEmpty()
				|| !Record.AuthoringMetadata.BaseContentId.IsEmpty()
				|| !Record.AuthoringMetadata.DesignIntent.IsEmpty()
				|| !Record.AuthoringMetadata.Planning.VariantId.IsEmpty()
				|| !Record.AuthoringMetadata.Planning.RoleId.IsEmpty()
				|| !Record.AuthoringMetadata.Planning.ProductionWaveId.IsEmpty()
				|| !Record.AuthoringMetadata.Planning.SharedDataRoleIds.IsEmpty()
				|| !Record.AuthoringMetadata.Planning.VariantOwnedDataRoleIds.IsEmpty()
				|| !Record.AuthoringMetadata.Planning.TechnologyDependencies.IsEmpty();
			if (bHasPlanningMetadata)
			{
				FCFContentChangeOperation PlanningOperation;
				PlanningOperation.OperationType =
					ECFContentChangeOperationType::SetPlanningMetadata;
				PlanningOperation.SheetId = Primary->SheetId;
				PlanningOperation.ContentKey = Record.Key;
				PlanningOperation.RowId = Record.RowId;
				PlanningOperation.PlanningFamilyId =
					Record.AuthoringMetadata.FamilyId;
				PlanningOperation.PlanningBaseContentId =
					Record.AuthoringMetadata.BaseContentId;
				PlanningOperation.PlanningDesignIntent =
					Record.AuthoringMetadata.DesignIntent;
				PlanningOperation.PlanningMetadata =
					Record.AuthoringMetadata.Planning;
				OperationIndexesByRecord.FindOrAdd(Record.Key.ToStableString())
					.Add(OutProposal.ChangeSet.Operations.Add(MoveTemp(PlanningOperation)));
			}

			TArray<FString> CollectionIds;
			Record.Collections.GetKeys(CollectionIds);
			CollectionIds.Sort();
			for (const FString& CollectionId : CollectionIds)
			{
				const FCFContentCollection* Collection =
					Record.Collections.Find(CollectionId);
				const FCFContentSheetDescriptor* Child =
					FindChildSheet(
						CandidateWorkbook,
						Record.Key.ContentTypeId,
						Primary->SheetId,
						CollectionId);
				if (Collection == nullptr || Child == nullptr)
				{
					OutError =
						TEXT("Candidate collection child schema를 찾지 못했습니다: ")
						+ Record.Key.ToStableString()
						+ TEXT(".")
						+ CollectionId;
					return false;
				}

				TArray<const FCFContentCollectionItem*> Items;
				for (const FCFContentCollectionItem& Item : Collection->Items)
				{
					Items.Add(&Item);
				}
				Items.Sort(
					[](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
					{
						return Left.ChildItemId < Right.ChildItemId;
					});

				for (const FCFContentCollectionItem* Item : Items)
				{
					if (Item == nullptr)
					{
						continue;
					}

					TArray<FString> ChildFieldIds;
					Item->Fields.GetKeys(ChildFieldIds);
					ChildFieldIds.Sort();
					for (const FString& ChildFieldId : ChildFieldIds)
					{
						const FCFContentValue* ChildValue =
							Item->Fields.Find(ChildFieldId);
						if (ChildValue == nullptr)
						{
							continue;
						}

						FCFContentChangeOperation ChildOperation;
						ChildOperation.OperationType =
							ECFContentChangeOperationType::UpsertChild;
						ChildOperation.SheetId = Child->SheetId;
						ChildOperation.ContentKey = Record.Key;
						ChildOperation.RowId = Record.RowId;
						ChildOperation.ColumnId = ChildFieldId;
						ChildOperation.ChildItemId = Item->ChildItemId;
						ChildOperation.ChildDomainKey = Item->DomainKey;
						ChildOperation.ChildOrder = Item->Order;
						ChildOperation.Value = *ChildValue;
						OperationIndexesByRecord.FindOrAdd(
							Record.Key.ToStableString())
							.Add(OutProposal.ChangeSet.Operations.Add(
								MoveTemp(ChildOperation)));
					}
				}
			}
		}

		TArray<FString> RationaleKeys;
		OperationIndexesByRecord.GetKeys(RationaleKeys);
		RationaleKeys.Sort();
		for (const FString& RationaleKey : RationaleKeys)
		{
			const TArray<int32>* OperationIndexes =
				OperationIndexesByRecord.Find(RationaleKey);
			if (OperationIndexes == nullptr || OperationIndexes->IsEmpty())
			{
				continue;
			}

			const FCFContentChangeOperation& FirstOperation =
				OutProposal.ChangeSet.Operations[(*OperationIndexes)[0]];

			FCFContentRecordRationale Rationale;
			Rationale.ContentKey = FirstOperation.ContentKey;
			Rationale.Reason =
				TEXT("First Production Wave candidate record.");
			Rationale.DesignIntentDelta =
				TEXT("Create reviewed Production identity/topology/balance candidate from current Roster baseline.");
			Rationale.RelatedOperationIndexes = *OperationIndexes;
			OutProposal.RecordRationales.Add(MoveTemp(Rationale));
		}
		return true;
	}

	// Role target의 canonical Production object path를 생성합니다.
	FString BuildTargetObjectPath(
		const FString& RoleId,
		const FString& ContentId)
	{
		FString Folder;
		FString AssetName;
		if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
		{
			Folder = TEXT("EquipmentPresets");
			AssetName = TEXT("DA_Equip_") + ContentId;
		}
		else if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
		{
			Folder = TEXT("TurretMounts");
			AssetName = TEXT("DA_") + ContentId;
		}
		else if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
		{
			Folder = TEXT("WeaponDefs");
			AssetName = TEXT("DA_Weapon_") + ContentId;
		}
		else if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
		{
			Folder = TEXT("Projectiles");
			AssetName = TEXT("DA_") + ContentId;
		}
		else if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
		{
			Folder = TEXT("Damage");
			AssetName = TEXT("DA_") + ContentId;
		}
		else
		{
			Folder = TEXT("Ammo");
			AssetName = TEXT("DA_") + ContentId;
		}

		return TEXT("/Game/CarFight/Weapons/Data/Production/")
			+ Folder
			+ TEXT("/")
			+ AssetName
			+ TEXT(".")
			+ AssetName;
	}

	// RoleId의 target ContentType을 반환합니다.
	FString ContentTypeForRole(const FString& RoleId)
	{
		if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
		{
			return TEXT("EquipmentPreset");
		}
		if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
		{
			return TEXT("TurretMount");
		}
		if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
		{
			return TEXT("WeaponDefinition");
		}
		if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
		{
			return TEXT("Projectile");
		}
		if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
		{
			return TEXT("Damage");
		}
		return TEXT("Ammo");
	}

	// Specs에서 unique exact34 Provision target manifest를 생성합니다.
	TArray<FCFProdCandidateTarget> BuildCandidateTargets(
		const TArray<FProductSpec>& Specs)
	{
		TArray<FCFProdCandidateTarget> Targets;
		TSet<FString> SeenKeys;
		const TArray<FString> RoleIds = {
			TEXT("EquipmentPreset"),
			TEXT("Mount"),
			TEXT("WeaponDefinition"),
			TEXT("Projectile"),
			TEXT("Damage"),
			TEXT("Ammo")
		};

		for (const FProductSpec& Spec : Specs)
		{
			for (const FString& RoleId : RoleIds)
			{
				const FString ContentId = ContentIdForRole(Spec, RoleId);
				const FString ContentTypeId = ContentTypeForRole(RoleId);
				const FCFContentKey Key = MakeKey(ContentTypeId, ContentId);
				const FString StableKey = Key.ToStableString();
				if (SeenKeys.Contains(StableKey))
				{
					continue;
				}

				FCFProdCandidateTarget Target;
				Target.ContentKey = Key;
				Target.RoleId = RoleId;
				Target.TargetObjectPath =
					BuildTargetObjectPath(RoleId, ContentId);
				Target.bShared =
					RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive)
					|| RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive)
						? false
						: IsSharedRoleTarget(Specs, RoleId, ContentId);
				Targets.Add(MoveTemp(Target));
				SeenKeys.Add(StableKey);
			}
		}

		Targets.Sort(
			[](const FCFProdCandidateTarget& Left, const FCFProdCandidateTarget& Right)
			{
				return Left.ContentKey.ToStableString()
					< Right.ContentKey.ToStableString();
			});
		return Targets;
	}


	// Registered ResourceId의 persisted exact object path를 반환합니다.
	bool ResolveResourceObjectPath(
		const FCFResourceCatalog& ResourceCatalog,
		const FString& ResourceId,
		FSoftObjectPath& OutObjectPath,
		FString& OutError)
	{
		OutObjectPath.Reset();
		const FCFResourceDescriptor* Descriptor =
			ResourceCatalog.FindResource(ResourceId);
		if (Descriptor == nullptr || !Descriptor->ObjectPath.IsValid())
		{
			OutError = TEXT("Production execution draft가 요구하는 ResourceId를 resolve하지 못했습니다: ")
				+ ResourceId;
			return false;
		}
		OutObjectPath = Descriptor->ObjectPath;
		return true;
	}

	// Workbook BindingMode token을 Production execution enum으로 변환합니다.
	ECFProdBindingMode BindingModeEnumForRole(
		const FProductSpec& Spec,
		const FString& RoleId,
		const bool bShared)
	{
		const FString BindingMode =
			BindingModeForRole(Spec, RoleId, bShared);
		if (BindingMode.Equals(TEXT("CreateNewShared"), ESearchCase::CaseSensitive))
		{
			return ECFProdBindingMode::CreateNewShared;
		}
		if (BindingMode.Equals(TEXT("BindShared"), ESearchCase::CaseSensitive))
		{
			return ECFProdBindingMode::BindShared;
		}
		return ECFProdBindingMode::CreateNew;
	}

	// Role target의 dependency-first durable execution order를 반환합니다.
	int32 DependencyOrderForRole(const FString& RoleId)
	{
		if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
		{
			return 1;
		}
		if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
		{
			return 2;
		}
		if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
		{
			return 3;
		}
		return 0;
	}

	// Launcher pattern token을 actual WeaponData enum으로 변환합니다.
	ECFLauncherFirePattern LauncherPatternForSpec(const FProductSpec& Spec)
	{
		if (Spec.LauncherFirePattern.Equals(TEXT("Salvo"), ESearchCase::CaseSensitive))
		{
			return ECFLauncherFirePattern::Salvo;
		}
		if (Spec.LauncherFirePattern.Equals(TEXT("Ripple"), ESearchCase::CaseSensitive))
		{
			return ECFLauncherFirePattern::Ripple;
		}
		return ECFLauncherFirePattern::SingleCycle;
	}

	// One Product spec를 existing Weapon Guide / Equipment Builder durable backend가 직접 소비할 typed draft로 투영합니다.
	bool BuildTypedDrafts(
		const FProductSpec& Spec,
		const FCFResourceCatalog& ResourceCatalog,
		FCFWeaponGuideDraft& OutWeaponDraft,
		FCFEquipmentPresetDraft& OutEquipmentDraft,
		FString& OutError)
	{
		OutWeaponDraft = FCFWeaponGuideDraft();
		OutEquipmentDraft = FCFEquipmentPresetDraft();
		OutError.Reset();

		// Reusable persisted binary resources입니다.
		FSoftObjectPath BaseTurretMeshPath;
		FSoftObjectPath YawTurretMeshPath;
		FSoftObjectPath PitchTurretMeshPath;
		FSoftObjectPath ProjectileMeshPath;
		FSoftObjectPath FireFxPath;
		FSoftObjectPath TrailFxPath;
		FSoftObjectPath ThrusterFxPath;
		FSoftObjectPath ImpactFxPath;

		const bool bCannon =
			Spec.FamilyId.Equals(TEXT("Cannon"), ESearchCase::CaseSensitive);
		const bool bGuided =
			Spec.FamilyId.Equals(TEXT("GuidedMissile"), ESearchCase::CaseSensitive);

		if (!ResolveResourceObjectPath(
				ResourceCatalog,
				TEXT("mesh.turret.base.standard"),
				BaseTurretMeshPath,
				OutError)
			|| !ResolveResourceObjectPath(
				ResourceCatalog,
				bCannon ? TEXT("mesh.turret.cannon.yaw") : TEXT("mesh.turret.rocket.yaw"),
				YawTurretMeshPath,
				OutError)
			|| !ResolveResourceObjectPath(
				ResourceCatalog,
				bCannon ? TEXT("mesh.turret.cannon.pitch") : TEXT("mesh.turret.rocket.pitch"),
				PitchTurretMeshPath,
				OutError)
			|| !ResolveResourceObjectPath(
				ResourceCatalog,
				bCannon ? TEXT("mesh.projectile.cannon") : TEXT("mesh.projectile.rocket"),
				ProjectileMeshPath,
				OutError)
			|| !ResolveResourceObjectPath(
				ResourceCatalog,
				TEXT("fx.weapon.fire.proto"),
				FireFxPath,
				OutError)
			|| !ResolveResourceObjectPath(
				ResourceCatalog,
				TEXT("fx.projectile.trail.ribbon"),
				TrailFxPath,
				OutError)
			|| !ResolveResourceObjectPath(
				ResourceCatalog,
				TEXT("fx.projectile.impact.proto"),
				ImpactFxPath,
				OutError))
		{
			return false;
		}

		if (Spec.bUsePropulsion
			&& !ResolveResourceObjectPath(
				ResourceCatalog,
				TEXT("fx.projectile.thruster.rocket"),
				ThrusterFxPath,
				OutError))
		{
			return false;
		}

		OutWeaponDraft.BaseAssetName = Spec.ProductId;
		OutWeaponDraft.SuggestedDisplayName = Spec.ProductId;
		OutWeaponDraft.ConceptNote = Spec.DesignIntent;
		OutWeaponDraft.Template = bGuided
			? ECFWeaponGuideTemplate::GuidedMissileLauncher
			: (bCannon
				? ECFWeaponGuideTemplate::DirectFireCannon
				: ECFWeaponGuideTemplate::RocketLauncher);
		OutWeaponDraft.RequiredMountType = ECFVehicleMountType::Turret;
		OutWeaponDraft.RequiredWeaponSize = ECFVehicleWeaponSize::Large;

		// Turret typed draft는 Workbook의 resource/semantic/current persisted values와 exact 대응합니다.
		OutWeaponDraft.Turret.Mode = ECFWeaponGuideChildMode::CreateNew;
		OutWeaponDraft.Turret.TurretBaseMeshPath = BaseTurretMeshPath;
		OutWeaponDraft.Turret.TurretYawMeshPath = YawTurretMeshPath;
		OutWeaponDraft.Turret.TurretPitchMeshPath = PitchTurretMeshPath;
		OutWeaponDraft.Turret.YawPivotSocketName = TEXT("YawPivot");
		OutWeaponDraft.Turret.PitchPivotSocketName = TEXT("PitchPivot");
		OutWeaponDraft.Turret.PrimaryMuzzleSocketName =
			bCannon ? FName(TEXT("Muzzle")) : FName(TEXT("Muzzle_1"));
		OutWeaponDraft.Turret.AdditionalMuzzleSocketNames =
			bCannon
				? TArray<FName>()
				: TArray<FName>{
					FName(TEXT("Muzzle_4")),
					FName(TEXT("Muzzle_2")),
					FName(TEXT("Muzzle_3")) };
		OutWeaponDraft.Turret.bRequireAllMuzzles = !bCannon;
		OutWeaponDraft.Turret.MinYawDeg = -180.0f;
		OutWeaponDraft.Turret.MaxYawDeg = 180.0f;
		OutWeaponDraft.Turret.MinPitchDeg = -8.0f;
		OutWeaponDraft.Turret.MaxPitchDeg = 25.0f;
		OutWeaponDraft.Turret.YawTurnRateDegPerSec = 35.0f;
		OutWeaponDraft.Turret.PitchTurnRateDegPerSec = 20.0f;
		OutWeaponDraft.Turret.bAllowFireWhileAligning = true;
		OutWeaponDraft.Turret.MuzzleClearanceDistanceCm = 150.0f;
		OutWeaponDraft.Turret.TurretMountWeightKg = bCannon ? 200.0f : 150.0f;

		// WeaponData typed draft입니다. Workbook m 단위를 Runtime cm 단위로 명시 변환합니다.
		OutWeaponDraft.Weapon.Mode = ECFWeaponGuideChildMode::CreateNew;
		OutWeaponDraft.Weapon.FireMode = ECFWeaponFireMode::Projectile;
		OutWeaponDraft.Weapon.FireRatePerMinute =
			static_cast<float>(Spec.FireRatePerMinute);
		OutWeaponDraft.Weapon.MaxRange =
			static_cast<float>(Spec.MaxRangeMeters * 100.0);
		OutWeaponDraft.Weapon.SpreadDeg =
			static_cast<float>(Spec.SpreadDeg);
		OutWeaponDraft.Weapon.WeaponMassKg =
			static_cast<float>(Spec.WeaponMassKg);
		OutWeaponDraft.Weapon.MagazineSize =
			static_cast<int32>(Spec.MagazineSize);
		OutWeaponDraft.Weapon.InitialLoadedAmmoCount =
			static_cast<int32>(Spec.InitialLoadedAmmoCount);
		OutWeaponDraft.Weapon.AmmoUnitsPerShot = 1;
		OutWeaponDraft.Weapon.ReloadTimeSeconds =
			static_cast<float>(Spec.ReloadTimeSeconds);
		OutWeaponDraft.Weapon.ReloadMode = ECFWeaponReloadMode::FullMagazine;
		OutWeaponDraft.Weapon.bAutoReloadWhenEmpty = true;
		OutWeaponDraft.Weapon.bAllowPartialReload = true;
		OutWeaponDraft.Weapon.bAllowPartialSequence = true;
		OutWeaponDraft.Weapon.bUseFiniteAmmo = true;
		OutWeaponDraft.Weapon.AmmoTypeId = FName(*Spec.AmmoId);
		OutWeaponDraft.Weapon.DefaultFireFxDataPath = FireFxPath;
		OutWeaponDraft.Weapon.bUseLauncher = !bCannon;
		OutWeaponDraft.Weapon.LauncherFirePatternConfig.FirePattern =
			LauncherPatternForSpec(Spec);
		OutWeaponDraft.Weapon.LauncherFirePatternConfig.ProjectileCountPerTrigger =
			static_cast<int32>(Spec.ProjectileCountPerTrigger);
		OutWeaponDraft.Weapon.LauncherFirePatternConfig.InterMuzzleDelaySeconds =
			static_cast<float>(Spec.InterMuzzleDelaySeconds);
		OutWeaponDraft.Weapon.LauncherFirePatternConfig.MaximumSimultaneousLaunchCount =
			static_cast<int32>(Spec.MaximumSimultaneousLaunchCount);
		OutWeaponDraft.Weapon.LauncherFirePatternConfig.SequenceFailurePolicy =
			ECFLauncherSequenceFailurePolicy::ContinueRemaining;
		OutWeaponDraft.Weapon.LauncherFirePatternConfig.CooldownStartPolicy =
			ECFLauncherCooldownStartPolicy::FirstAcceptedProjectile;
		OutWeaponDraft.Weapon.LauncherReleaseConfig.ReleaseMode =
			ECFProjectileReleaseMode::Direct;
		OutWeaponDraft.Weapon.LauncherReleaseConfig.EjectionSpeed = 0.0f;
		OutWeaponDraft.Weapon.LauncherReleaseConfig.CarrierVelocityRatio = 0.0f;
		OutWeaponDraft.Weapon.LauncherReleaseConfig.LauncherClearanceTraceDistanceCm = 0.0f;

		// Projectile typed draft입니다.
		OutWeaponDraft.Projectile.Mode = ECFWeaponGuideChildMode::CreateNew;
		OutWeaponDraft.Projectile.ProjectileActorClassPath =
			FSoftClassPath(
				TEXT("/Game/CarFight/Weapons/Projectiles/Blueprints/BP_CFProjectile.BP_CFProjectile_C"));
		OutWeaponDraft.Projectile.ProjectileStaticMeshPath = ProjectileMeshPath;
		OutWeaponDraft.Projectile.InitialSpeed =
			static_cast<float>(Spec.InitialSpeedMetersPerSecond * 100.0);
		OutWeaponDraft.Projectile.LifeTimeSeconds =
			static_cast<float>(Spec.ProjectileLifeTimeSeconds);
		OutWeaponDraft.Projectile.bAffectedByGravity = true;
		OutWeaponDraft.Projectile.GravityScale = 1.0f;
		OutWeaponDraft.Projectile.CollisionRadius = 8.0f;
		OutWeaponDraft.Projectile.bUseSweepCollision = true;
		OutWeaponDraft.Projectile.bCanBeIntercepted = true;
		OutWeaponDraft.Projectile.bDetonateWhenIntercepted = true;
		OutWeaponDraft.Projectile.TrailFxNiagaraPath = TrailFxPath;
		OutWeaponDraft.Projectile.ThrusterFxNiagaraPath =
			Spec.bUsePropulsion ? ThrusterFxPath : FSoftObjectPath();
		OutWeaponDraft.Projectile.DefaultImpactFxDataPath = ImpactFxPath;
		OutWeaponDraft.Projectile.PropulsionConfig.bUsePropulsion =
			Spec.bUsePropulsion;
		OutWeaponDraft.Projectile.PropulsionConfig.IgnitionDelaySeconds =
			static_cast<float>(Spec.IgnitionDelaySeconds);
		OutWeaponDraft.Projectile.PropulsionConfig.BurnDurationSeconds =
			static_cast<float>(Spec.BurnDurationSeconds);
		OutWeaponDraft.Projectile.PropulsionConfig.ThrustAccelerationCmPerSecSq =
			static_cast<float>(Spec.ThrustAccelerationMetersPerSecondSq * 100.0);
		OutWeaponDraft.Projectile.PropulsionConfig.MaximumPropelledSpeed =
			static_cast<float>(Spec.MaximumPropelledSpeedMetersPerSecond * 100.0);
		OutWeaponDraft.Projectile.PropulsionConfig.bUseLaunchAxisStabilization =
			Spec.bUseLaunchAxisStabilization;
		OutWeaponDraft.Projectile.PropulsionConfig.MaximumThrustVectorAngleDeg =
			static_cast<float>(Spec.MaximumThrustVectorAngleDeg);
		OutWeaponDraft.Projectile.PropulsionConfig.LaunchAxisStabilizationResponseTimeSeconds =
			static_cast<float>(Spec.LaunchAxisResponseTimeSeconds);

		OutWeaponDraft.Projectile.MissileFlightConfig.bUseMissileFlight =
			Spec.bUseMissileFlight;
		OutWeaponDraft.Projectile.MissileFlightConfig.AttackProfile =
			ECFMissileAttackProfile::Direct;
		OutWeaponDraft.Projectile.MissileFlightConfig.MinimumClearanceTimeSeconds =
			Spec.bUseMissileFlight ? 0.15f : 0.0f;
		OutWeaponDraft.Projectile.MissileFlightConfig.MinimumClearanceDistanceCm =
			Spec.bUseMissileFlight ? 300.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileFlightConfig.TransitionDurationSeconds =
			Spec.bUseMissileFlight ? 0.35f : 0.0f;
		OutWeaponDraft.Projectile.MissileFlightConfig.bUseTerminalPhase = false;
		OutWeaponDraft.Projectile.MissileFlightConfig.TerminalPhaseStartDistanceCm = 1500.0f;

		OutWeaponDraft.Projectile.MissileGuideConfig.bUseGuidance =
			Spec.bUseGuidance;
		OutWeaponDraft.Projectile.MissileGuideConfig.GuideMode =
			Spec.bUseGuidance
				? ECFMissileGuideMode::TargetActor
				: ECFMissileGuideMode::None;
		OutWeaponDraft.Projectile.MissileGuideConfig.LostTargetPolicy =
			ECFMissileLostTargetPolicy::ContinueStraight;
		OutWeaponDraft.Projectile.MissileGuideConfig.GuidanceLaw =
			ECFMissileGuidanceLaw::ProportionalNavigation;
		OutWeaponDraft.Projectile.MissileGuideConfig.NavigationConstant =
			Spec.bUseGuidance ? 3.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.MaximumTurnRateDegPerSec =
			Spec.bUseGuidance ? 70.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq =
			Spec.bUseGuidance ? 9000.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.GuidanceResponseTimeSeconds =
			Spec.bUseGuidance ? 0.15f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec =
			Spec.bUseGuidance ? 800.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.SeekerModel =
			ECFMissileSeekerModel::LegacySingleGate;
		OutWeaponDraft.Projectile.MissileGuideConfig.TargetObservationMode =
			ECFMissileTargetObservationMode::DirectActorKinematics;
		OutWeaponDraft.Projectile.MissileGuideConfig.GuidanceActivationMode =
			ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow;
		OutWeaponDraft.Projectile.MissileGuideConfig.GuidanceActivationDelaySeconds = 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.GuidanceActivationDistanceCm = 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.SeekerFieldOfViewDeg =
			Spec.bUseGuidance ? 60.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.LockBreakAngleDeg =
			Spec.bUseGuidance ? 85.0f : 0.0f;
		OutWeaponDraft.Projectile.MissileGuideConfig.TargetLostGraceTimeSeconds =
			Spec.bUseGuidance ? 0.25f : 0.0f;

		// Damage/Ammo typed provider draft입니다.
		OutWeaponDraft.Damage.Mode = ECFWeaponGuideChildMode::CreateNew;
		OutWeaponDraft.Damage.DamageType = ECFDamageType::Kinetic;
		OutWeaponDraft.Damage.BaseDamage =
			static_cast<float>(Spec.BaseDamage);
		OutWeaponDraft.Damage.bCanDamageSelf = false;
		OutWeaponDraft.Damage.ArmorPenetration = 0.0f;
		OutWeaponDraft.Damage.bUseRadialDamage = false;
		OutWeaponDraft.Damage.ExplosionRadius = 0.0f;
		OutWeaponDraft.Damage.ExplosionInnerRadius = 0.0f;
		OutWeaponDraft.Damage.ExplosionDamage = 0.0f;
		OutWeaponDraft.Damage.MinExplosionDamageScale = 0.0f;
		OutWeaponDraft.Damage.ModuleDamageScale = 0.0f;
		OutWeaponDraft.Damage.ImpulseStrength = 0.0f;

		OutWeaponDraft.Ammo.Mode = ECFWeaponGuideChildMode::CreateNew;
		OutWeaponDraft.Ammo.AmmoDisplayName = Spec.AmmoId;
		OutWeaponDraft.Ammo.AmmoFamilyId =
			Spec.AmmoId.Equals(TEXT("Ammo_Rocket"), ESearchCase::CaseSensitive)
				? FName(TEXT("Rocket_Standard_Warhead"))
				: NAME_None;
		OutWeaponDraft.Ammo.UnitMassKg =
			static_cast<float>(Spec.AmmoUnitMassKg);
		OutWeaponDraft.Ammo.AmmoTags = { FName(TEXT("Production")) };
		OutWeaponDraft.Ammo.MaximumLoadableAmmoCount =
			static_cast<int32>(Spec.AmmoMaximumLoadableCount);
		OutWeaponDraft.Ammo.bCanBeResupplied = true;

		// EquipmentPreset typed draft입니다.
		OutEquipmentDraft.EquipmentId = FName(*Spec.ProductId);
		OutEquipmentDraft.DisplayName = FText::FromString(Spec.ProductId);
		OutEquipmentDraft.bPreserveExistingDisplayName = false;
		OutEquipmentDraft.RequiredMountType = ECFVehicleMountType::Turret;
		OutEquipmentDraft.RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		OutEquipmentDraft.DefaultTurretMountData =
			TSoftObjectPtr<UCFTurretMountData>(
				FSoftObjectPath(BuildTargetObjectPath(TEXT("Mount"), Spec.MountId)));
		OutEquipmentDraft.DefaultWeaponData =
			TSoftObjectPtr<UCFWeaponData>(
				FSoftObjectPath(BuildTargetObjectPath(TEXT("WeaponDefinition"), Spec.ProductId)));
		OutEquipmentDraft.DefaultSensorData.Reset();
		OutEquipmentDraft.DraftMode = ECFEquipmentDraftMode::Weapon;
		return true;
	}

	// One role target의 actual typed durable backend desired fingerprint를 계산합니다.
	bool BuildTypedDesiredFingerprint(
		const FProductSpec& Spec,
		const FString& RoleId,
		const FCFWeaponGuideDraft& WeaponDraft,
		const FCFEquipmentPresetDraft& EquipmentDraft,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		if (RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
		{
			FCFEquipmentSemanticSnapshot Snapshot;
			return FCFEquipmentDurable::BuildSnapshotFromDraft(
					EquipmentDraft,
					Snapshot,
					OutError)
				&& FCFEquipmentDurable::BuildSemanticFingerprint(
					Snapshot,
					OutFingerprint,
					OutError);
		}
		if (RoleId.Equals(TEXT("Mount"), ESearchCase::CaseSensitive))
		{
			return CFWeaponGuideBackend::BuildTurretDesiredFingerprint(
				WeaponDraft,
				FName(*Spec.MountId),
				OutFingerprint,
				OutError);
		}
		if (RoleId.Equals(TEXT("Damage"), ESearchCase::CaseSensitive))
		{
			return CFWeaponGuideBackend::BuildDamageDesiredFingerprint(
				WeaponDraft,
				FName(*Spec.DamageId),
				OutFingerprint,
				OutError);
		}
		if (RoleId.Equals(TEXT("Ammo"), ESearchCase::CaseSensitive))
		{
			return CFWeaponGuideBackend::BuildAmmoDesiredFingerprint(
				WeaponDraft,
				FName(*Spec.AmmoId),
				OutFingerprint,
				OutError);
		}
		if (RoleId.Equals(TEXT("Projectile"), ESearchCase::CaseSensitive))
		{
			return CFWeaponGuideBackend::BuildProjectileDesiredFingerprint(
				WeaponDraft,
				FName(*Spec.ProjectileId),
				FSoftObjectPath(BuildTargetObjectPath(TEXT("Damage"), Spec.DamageId)),
				OutFingerprint,
				OutError);
		}
		if (RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive))
		{
			return CFWeaponGuideBackend::BuildWeaponDesiredFingerprint(
				WeaponDraft,
				FName(*Spec.ProductId),
				FSoftObjectPath(BuildTargetObjectPath(TEXT("Projectile"), Spec.ProjectileId)),
				FSoftObjectPath(BuildTargetObjectPath(TEXT("Ammo"), Spec.AmmoId)),
				FName(*Spec.AmmoId),
				OutFingerprint,
				OutError);
		}

		OutError = TEXT("Unknown Production role의 typed desired fingerprint를 만들 수 없습니다: ")
			+ RoleId;
		return false;
	}

	// Product exact6 Role topology identity를 typed execution 관점에서 계산합니다.
	bool BuildRoleTopologyFingerprint(
		const FCFProdProvisionRequest& Request,
		FString& OutFingerprint,
		FString& OutError)
	{
		TArray<uint8> Bytes;
		CFDACommonPrimitives::AppendStringToken(
			Bytes,
			TEXT("Protocol"),
			TEXT("CarFight.CCAS.ProdRoleTopology/v1"));
		CFDACommonPrimitives::AppendStringToken(
			Bytes,
			TEXT("Product"),
			Request.ProductKey.ToStableString());

		TArray<const FCFProdProvisionTarget*> SortedTargets;
		for (const FCFProdProvisionTarget& Target : Request.Targets)
		{
			SortedTargets.Add(&Target);
		}
		SortedTargets.Sort(
			[](const FCFProdProvisionTarget& Left, const FCFProdProvisionTarget& Right)
			{
				return Left.RoleId < Right.RoleId;
			});

		for (const FCFProdProvisionTarget* Target : SortedTargets)
		{
			if (Target == nullptr)
			{
				continue;
			}
			CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Role"), Target->RoleId);
			CFDACommonPrimitives::AppendStringToken(
				Bytes,
				TEXT("ContentKey"),
				Target->ContentKey.ToStableString());
			CFDACommonPrimitives::AppendStringToken(
				Bytes,
				TEXT("ObjectPath"),
				Target->TargetObjectPath);
			CFDACommonPrimitives::AppendStringToken(
				Bytes,
				TEXT("BindingMode"),
				LexToString(static_cast<int32>(Target->BindingMode)));
			CFDACommonPrimitives::AppendBoolToken(
				Bytes,
				TEXT("Shared"),
				Target->bShared);
			CFDACommonPrimitives::AppendBoolToken(
				Bytes,
				TEXT("Required"),
				Target->bRequired);
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			Bytes,
			OutFingerprint,
			OutError);
	}

	// Review Package와 같은 Product spec에서 exact8 typed execution requests/drafts를 구성합니다.
	bool BuildPreparedProducts(
		const TArray<FProductSpec>& Specs,
		const FCFContentReviewPackage& ReviewPackage,
		const FString& ResourceCatalogFingerprint,
		const FCFResourceCatalog& ResourceCatalog,
		TArray<FCFProdPreparedProduct>& OutPreparedProducts,
		FString& OutError)
	{
		OutPreparedProducts.Reset();
		OutError.Reset();

		const TArray<FString> RoleIds = {
			TEXT("EquipmentPreset"),
			TEXT("Mount"),
			TEXT("WeaponDefinition"),
			TEXT("Projectile"),
			TEXT("Damage"),
			TEXT("Ammo")
		};

		for (const FProductSpec& Spec : Specs)
		{
			FCFProdPreparedProduct Prepared;
			if (!BuildTypedDrafts(
				Spec,
				ResourceCatalog,
				Prepared.WeaponDraft,
				Prepared.EquipmentDraft,
				OutError))
			{
				return false;
			}

			Prepared.Request.TransactionId =
				TEXT("CF058_W1_") + Spec.ProductId;
			Prepared.Request.ProductKey =
				MakeKey(TEXT("Weapon"), Spec.ProductId);
			Prepared.Request.BaseCatalogSnapshotFingerprint =
				ReviewPackage.BaseCatalogSnapshotFingerprint;
			Prepared.Request.ResourceCatalogFingerprint =
				ResourceCatalogFingerprint;
			Prepared.Request.WorkbookSemanticHash =
				ReviewPackage.ExpectedPostSemanticHash;
			Prepared.Request.EquipmentPresetObjectPath =
				BuildTargetObjectPath(TEXT("EquipmentPreset"), Spec.ProductId);
			Prepared.Request.DependencyEdges =
				ReviewPackage.Preview.DependencyEdges;

			for (const FString& RoleId : RoleIds)
			{
				const FString ContentId =
					ContentIdForRole(Spec, RoleId);
				const bool bShared =
					RoleId.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive)
					|| RoleId.Equals(TEXT("WeaponDefinition"), ESearchCase::CaseSensitive)
						? false
						: IsSharedRoleTarget(Specs, RoleId, ContentId);

				FCFProdProvisionTarget Target;
				Target.ContentKey =
					MakeKey(ContentTypeForRole(RoleId), ContentId);
				Target.RoleId = RoleId;
				Target.TargetObjectPath =
					BuildTargetObjectPath(RoleId, ContentId);
				Target.BindingMode =
					BindingModeEnumForRole(Spec, RoleId, bShared);
				Target.bShared = bShared;
				Target.bRequired = true;
				Target.bRequireAbsentAtReview =
					Target.BindingMode != ECFProdBindingMode::BindShared;
				Target.DependencyOrder =
					DependencyOrderForRole(RoleId);

				if (!BuildTypedDesiredFingerprint(
					Spec,
					RoleId,
					Prepared.WeaponDraft,
					Prepared.EquipmentDraft,
					Target.DesiredFingerprint,
					OutError))
				{
					return false;
				}

				Prepared.Request.Targets.Add(MoveTemp(Target));
			}

			FCFProdAmmoLoadRow AmmoLoad;
			AmmoLoad.ProductContentId = Spec.ProductId;
			AmmoLoad.AmmoRoleContentId = Spec.AmmoId;
			AmmoLoad.DefaultSortieAmmoCount =
				static_cast<int32>(Spec.DefaultSortieAmmoCount);
			Prepared.Request.AmmoLoadRows.Add(MoveTemp(AmmoLoad));

			if (!BuildRoleTopologyFingerprint(
				Prepared.Request,
				Prepared.Request.RoleTopologyFingerprint,
				OutError))
			{
				return false;
			}

			OutPreparedProducts.Add(MoveTemp(Prepared));
		}

		return OutPreparedProducts.Num() == 8;
	}

	// exact8 typed Product execution manifest 전체를 canonical approval binding fingerprint로 계산합니다.
	bool BuildExecutionBindingFingerprint(
		const FCFContentReviewPackage& ReviewPackage,
		const FString& ResourceCatalogFingerprint,
		const TArray<FCFProdPreparedProduct>& PreparedProducts,
		FString& OutFingerprint,
		int32& OutUniqueTargetCount,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutUniqueTargetCount = 0;
		OutError.Reset();

		TArray<uint8> Bytes;
		CFDACommonPrimitives::AppendStringToken(
			Bytes,
			TEXT("Protocol"),
			TEXT("CarFight.CCAS.ProductionExecutionBinding/v1"));
		CFDACommonPrimitives::AppendStringToken(
			Bytes,
			TEXT("BaseCatalogSnapshotFingerprint"),
			ReviewPackage.BaseCatalogSnapshotFingerprint);
		CFDACommonPrimitives::AppendStringToken(
			Bytes,
			TEXT("ExpectedPostSemanticHash"),
			ReviewPackage.ExpectedPostSemanticHash);
		CFDACommonPrimitives::AppendStringToken(
			Bytes,
			TEXT("ResourceCatalogFingerprint"),
			ResourceCatalogFingerprint);

		TArray<const FCFProdPreparedProduct*> SortedProducts;
		for (const FCFProdPreparedProduct& Prepared : PreparedProducts)
		{
			SortedProducts.Add(&Prepared);
		}
		SortedProducts.Sort(
			[](const FCFProdPreparedProduct& Left, const FCFProdPreparedProduct& Right)
			{
				return Left.Request.ProductKey.ToStableString()
					< Right.Request.ProductKey.ToStableString();
			});

		// Shared target가 Product별로 같은 path/desired payload를 요구하는지 검증할 canonical projection입니다.
		TMap<FString, FString> CanonicalTargetPayloadByKey;
		for (const FCFProdPreparedProduct* Prepared : SortedProducts)
		{
			if (Prepared == nullptr)
			{
				continue;
			}

			const FCFProdProvisionRequest& Request = Prepared->Request;
			CFDACommonPrimitives::AppendStringToken(
				Bytes,
				TEXT("Product"),
				Request.ProductKey.ToStableString());
			CFDACommonPrimitives::AppendStringToken(
				Bytes,
				TEXT("RoleTopologyFingerprint"),
				Request.RoleTopologyFingerprint);
			CFDACommonPrimitives::AppendStringToken(
				Bytes,
				TEXT("EquipmentPresetObjectPath"),
				Request.EquipmentPresetObjectPath);

			TArray<const FCFProdProvisionTarget*> SortedTargets;
			for (const FCFProdProvisionTarget& Target : Request.Targets)
			{
				SortedTargets.Add(&Target);
			}
			SortedTargets.Sort(
				[](const FCFProdProvisionTarget& Left, const FCFProdProvisionTarget& Right)
				{
					return Left.RoleId < Right.RoleId;
				});

			for (const FCFProdProvisionTarget* Target : SortedTargets)
			{
				if (Target == nullptr)
				{
					continue;
				}
				CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Role"), Target->RoleId);
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("TargetKey"),
					Target->ContentKey.ToStableString());
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("TargetObjectPath"),
					Target->TargetObjectPath);
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("DesiredFingerprint"),
					Target->DesiredFingerprint);
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("BindingMode"),
					LexToString(static_cast<int32>(Target->BindingMode)));
				CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("Shared"), Target->bShared);
				CFDACommonPrimitives::AppendBoolToken(
					Bytes,
					TEXT("RequireAbsentAtReview"),
					Target->bRequireAbsentAtReview);
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("DependencyOrder"),
					LexToString(Target->DependencyOrder));

				const FString StableKey =
					Target->ContentKey.ToStableString();
				const FString CanonicalPayload =
					Target->TargetObjectPath
					+ TEXT("|")
					+ Target->DesiredFingerprint
					+ TEXT("|")
					+ (Target->bShared ? TEXT("Shared") : TEXT("VariantOwned"));
				if (const FString* ExistingPayload =
					CanonicalTargetPayloadByKey.Find(StableKey))
				{
					if (!ExistingPayload->Equals(
						CanonicalPayload,
						ESearchCase::CaseSensitive))
					{
						OutError =
							TEXT("Shared/duplicate Production target의 typed desired payload가 Product별로 다릅니다: ")
							+ StableKey;
						return false;
					}
				}
				else
				{
					CanonicalTargetPayloadByKey.Add(
						StableKey,
						CanonicalPayload);
				}
			}

			for (const FCFProdAmmoLoadRow& AmmoLoad : Request.AmmoLoadRows)
			{
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("AmmoLoadProduct"),
					AmmoLoad.ProductContentId);
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("AmmoLoadRole"),
					AmmoLoad.AmmoRoleContentId);
				CFDACommonPrimitives::AppendStringToken(
					Bytes,
					TEXT("AmmoLoadCount"),
					LexToString(AmmoLoad.DefaultSortieAmmoCount));
			}
		}

		OutUniqueTargetCount = CanonicalTargetPayloadByKey.Num();
		if (OutUniqueTargetCount != 34)
		{
			OutError = FString::Printf(
				TEXT("Typed Production execution unique target count가 exact34가 아닙니다: %d"),
				OutUniqueTargetCount);
			return false;
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			Bytes,
			OutFingerprint,
			OutError);
	}

	// ExecutionBindingFingerprint를 Review Package fingerprint와 exact8 requests에 최종 결속합니다.
	bool FinalizeExecutionApprovalBinding(
		FCFProdBatchPrepResult& OutResult,
		FString& OutError)
	{
		OutResult.ReviewPackage.ExecutionBindingFingerprint =
			OutResult.ExecutionBindingFingerprint;

		FString FinalReviewFingerprint;
		if (!FCFContentPlanningService::ComputeReviewPackageFingerprint(
			OutResult.ReviewPackage,
			FinalReviewFingerprint,
			OutError))
		{
			return false;
		}
		OutResult.ReviewPackage.ReviewPackageFingerprint =
			FinalReviewFingerprint;

		for (FCFProdPreparedProduct& Prepared : OutResult.PreparedProducts)
		{
			Prepared.Request.ReviewPackageFingerprint =
				FinalReviewFingerprint;
			Prepared.Request.BaseCatalogSnapshotFingerprint =
				OutResult.ReviewPackage.BaseCatalogSnapshotFingerprint;
			Prepared.Request.ResourceCatalogFingerprint =
				OutResult.ResourceCatalogFingerprint;
			Prepared.Request.ExecutionBindingFingerprint =
				OutResult.ExecutionBindingFingerprint;
			Prepared.Request.WorkbookSemanticHash =
				OutResult.ReviewPackage.ExpectedPostSemanticHash;
		}
		return true;
	}

	// Vehicle-ready apply에 필요한 Required ResourceReference 중 unresolved field만 수집합니다.
	TArray<FString> CollectMissingRequiredResourceBindings(
		const FCFContentWorkbookModel& Workbook)
	{
		TArray<FString> Missing;
		for (const FCFContentRecord& Record : Workbook.Records)
		{
			// Projectile propulsion 상태입니다.
			bool bUsePropulsion = false;
			if (const FCFContentValue* UsePropulsionValue =
				Record.Fields.Find(TEXT("UsePropulsion")))
			{
				bUsePropulsion =
					UsePropulsionValue->Type == ECFContentValueType::Boolean
					&& UsePropulsionValue->State == ECFContentValueState::Value
					&& UsePropulsionValue->bBooleanValue;
			}

			for (const TPair<FString, FCFContentValue>& FieldPair : Record.Fields)
			{
				if (FieldPair.Value.Type != ECFContentValueType::ResourceReference)
				{
					continue;
				}

				// Ammo icon과 non-propelled projectile thruster는 gameplay apply에 optional입니다.
				const bool bOptionalAmmoIcon =
					FieldPair.Key.Equals(TEXT("AmmoIconResourceId"), ESearchCase::CaseSensitive);
				const bool bOptionalThruster =
					FieldPair.Key.Equals(TEXT("ThrusterFxResourceId"), ESearchCase::CaseSensitive)
					&& !bUsePropulsion;
				if (bOptionalAmmoIcon || bOptionalThruster)
				{
					continue;
				}

				if (FieldPair.Value.State != ECFContentValueState::Value
					|| FieldPair.Value.StringValue.IsEmpty())
				{
					Missing.Add(
						Record.Key.ToStableString()
						+ TEXT(".")
						+ FieldPair.Key);
				}
			}
		}
		Missing.Sort();
		return Missing;
	}

	// Candidate Workbook exact counts를 검증합니다.
	bool ValidateCandidateCounts(
		const FCFContentWorkbookModel& Workbook,
		const TArray<FCFProdCandidateTarget>& Targets,
		FString& OutError)
	{
		int32 ProductCount = 0;
		int32 RoleBindingCount = 0;
		int32 AmmoLoadCount = 0;
		for (const FCFContentRecord& Record : Workbook.Records)
		{
			if (Record.Key.ContentTypeId.Value.Equals(TEXT("Weapon"), ESearchCase::CaseSensitive))
			{
				++ProductCount;
				const FCFContentCollection* Roles =
					Record.Collections.Find(TEXT("RoleBindings"));
				RoleBindingCount += Roles != nullptr ? Roles->Items.Num() : 0;
			}
			else if (Record.Key.ContentTypeId.Value.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
			{
				const FCFContentCollection* AmmoLoads =
					Record.Collections.Find(TEXT("EquipmentPresetAmmoLoads"));
				AmmoLoadCount += AmmoLoads != nullptr ? AmmoLoads->Items.Num() : 0;
			}
		}

		if (ProductCount != 8
			|| RoleBindingCount != 48
			|| Targets.Num() != 34
			|| AmmoLoadCount != 8
			|| Workbook.Records.Num() != 42)
		{
			OutError = FString::Printf(
				TEXT("Production candidate exact count mismatch: Product=%d RoleBindings=%d Targets=%d AmmoLoads=%d Records=%d"),
				ProductCount,
				RoleBindingCount,
				Targets.Num(),
				AmmoLoadCount,
				Workbook.Records.Num());
			return false;
		}
		return true;
	}

	// Review Package preview에서 exact counts를 읽어 결과에 기록합니다.
	void FillCounts(
		const FCFContentWorkbookModel& Workbook,
		FCFProdBatchPrepResult& OutResult)
	{
		for (const FCFContentRecord& Record : Workbook.Records)
		{
			if (Record.Key.ContentTypeId.Value.Equals(TEXT("Weapon"), ESearchCase::CaseSensitive))
			{
				++OutResult.ProductCount;
				const FCFContentCollection* Roles =
					Record.Collections.Find(TEXT("RoleBindings"));
				OutResult.RoleBindingCount += Roles != nullptr ? Roles->Items.Num() : 0;
			}
			else if (Record.Key.ContentTypeId.Value.Equals(TEXT("EquipmentPreset"), ESearchCase::CaseSensitive))
			{
				const FCFContentCollection* AmmoLoads =
					Record.Collections.Find(TEXT("EquipmentPresetAmmoLoads"));
				OutResult.AmmoLoadCount += AmmoLoads != nullptr ? AmmoLoads->Items.Num() : 0;
			}
		}
		OutResult.UniqueDataAssetCount = OutResult.CandidateTargets.Num();
	}
}

// Current Roster baseline으로 exact8/exact34 transient candidate와 immutable Review Package를 생성합니다.
bool FCFProdBatchPrep::PrepareFirstWaveCandidate(
	FCFProdBatchPrepResult& OutResult,
	FString& OutError)
{
	using namespace CFProdBatchPrepPrivate;

	OutResult = FCFProdBatchPrepResult();
	OutError.Reset();

	const TArray<FProductSpec> Specs = BuildProductSpecs();
	if (Specs.Num() != 8)
	{
		OutError = TEXT("First Production Wave Product spec count가 exact8이 아닙니다.");
		return false;
	}

	const TArray<FCFContentProviderDescriptor> Descriptors =
		BuildProviderDescriptors();

	FCFContentProviderRegistry ProviderRegistry;
	for (const FCFContentProviderDescriptor& Descriptor : Descriptors)
	{
		const TSharedRef<FReviewProvider> Provider =
			MakeShared<FReviewProvider>(Descriptor);
		if (!ProviderRegistry.RegisterProvider(Provider, OutError))
		{
			return false;
		}
	}

	const FCFContentWorkbookModel EmptyWorkbook =
		BuildEmptyWorkbook(Descriptors);

	FMemoryWorkbookAdapter BaseAdapter(EmptyWorkbook);
	FCFContentCompileResult BaseCompileResult;
	if (!FCFContentCompiler::CompilePreview(
		TEXT("Memory://CFProdBatchPrep/Base"),
		BaseAdapter,
		ProviderRegistry,
		BaseCompileResult))
	{
		OutError = BaseCompileResult.Error.IsEmpty()
			? TEXT("Empty Production baseline compile에 실패했습니다.")
			: BaseCompileResult.Error;
		return false;
	}

	FCFResourceCatalog ResourceCatalog;
	FCFResourcePickerRegistry PickerRegistry;
	if (!BuildProductionResourceCatalog(
		PickerRegistry,
		ResourceCatalog,
		OutError))
	{
		return false;
	}

	if (ResourceCatalog.Num() != 12)
	{
		OutError = FString::Printf(
			TEXT("First Production Wave Resource Catalog count가 exact12가 아닙니다: %d"),
			ResourceCatalog.Num());
		return false;
	}

	FCFContentCatalogSnapshot BaseSnapshot;
	if (!FCFContentPlanningService::BuildCatalogSnapshot(
		BaseCompileResult,
		ProviderRegistry,
		ResourceCatalog,
		PickerRegistry,
		BaseSnapshot,
		OutError))
	{
		return false;
	}

	const FCFContentWorkbookModel CandidateWorkbook =
		BuildCandidateWorkbook(EmptyWorkbook, Specs);
	OutResult.CandidateTargets = BuildCandidateTargets(Specs);

	// Candidate ResourceReference가 모두 registered persisted resource를 가리키는지 fail-closed 검증합니다.
	TArray<FCFContentValidationIssue> ResourceIssues;
	if (!FCFContentResourceValidator::ValidateWorkbook(
		CandidateWorkbook,
		ResourceCatalog,
		PickerRegistry,
		ResourceIssues))
	{
		OutError = ResourceIssues.IsEmpty()
			? TEXT("Production candidate ResourceReference validation에 실패했습니다.")
			: ResourceIssues[0].Code + TEXT(": ") + ResourceIssues[0].Message;
		return false;
	}

	// Persisted mesh의 required semantic socket exact8을 직접 증명합니다.
	if (!ValidateProductionSocketCapabilities(
		ResourceCatalog,
		PickerRegistry,
		OutResult.SocketCapabilityCheckCount,
		OutError))
	{
		return false;
	}

	OutResult.ResolvedResourceCount = ResourceCatalog.Num();
	OutResult.ResourceCatalogFingerprint =
		BaseSnapshot.ResourceCatalogFingerprint;
	OutResult.ProviderSchemaFingerprint =
		BaseSnapshot.ProviderSchemaFingerprint;

	if (!ValidateCandidateCounts(
		CandidateWorkbook,
		OutResult.CandidateTargets,
		OutError))
	{
		return false;
	}

	FCFContentChangeProposal Proposal;
	if (!BuildAddProposal(
		BaseSnapshot,
		CandidateWorkbook,
		Proposal,
		OutError))
	{
		return false;
	}

	TArray<FCFContentValidationIssue> ReviewIssues;
	if (!FCFContentPlanningService::BuildReviewPackage(
		BaseSnapshot,
		Proposal,
		ProviderRegistry,
		ResourceCatalog,
		PickerRegistry,
		OutResult.ReviewPackage,
		ReviewIssues,
		OutError))
	{
		if (OutError.IsEmpty() && !ReviewIssues.IsEmpty())
		{
			OutError =
				ReviewIssues[0].Code
				+ TEXT(": ")
				+ ReviewIssues[0].Message;
		}
		return false;
	}

	FString CandidateHashError;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		OutResult.ReviewPackage.Preview.Workbook,
		OutResult.CandidateWorkbookSemanticHash,
		CandidateHashError))
	{
		OutError = CandidateHashError;
		return false;
	}

	FString DirectCandidateHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		CandidateWorkbook,
		DirectCandidateHash,
		CandidateHashError))
	{
		OutError = CandidateHashError;
		return false;
	}
	if (!DirectCandidateHash.Equals(
		OutResult.CandidateWorkbookSemanticHash,
		ESearchCase::CaseSensitive))
	{
		OutError =
			TEXT("Review Package post Workbook과 direct exact34 candidate semantic hash가 일치하지 않습니다.");
		return false;
	}

	// 같은 Product spec / Resource Catalog에서 실제 durable backend typed drafts와 exact8 requests를 생성합니다.
	if (!BuildPreparedProducts(
		Specs,
		OutResult.ReviewPackage,
		OutResult.ResourceCatalogFingerprint,
		ResourceCatalog,
		OutResult.PreparedProducts,
		OutError))
	{
		return false;
	}

	// exact8 / exact34 typed execution payload 전체를 USER approval에 포함할 manifest fingerprint로 고정합니다.
	if (!BuildExecutionBindingFingerprint(
		OutResult.ReviewPackage,
		OutResult.ResourceCatalogFingerprint,
		OutResult.PreparedProducts,
		OutResult.ExecutionBindingFingerprint,
		OutResult.UniqueExecutionTargetCount,
		OutError))
	{
		return false;
	}

	// Execution manifest를 immutable ReviewPackageFingerprint 안에 넣고 동일 final identity를 exact8 requests에 복제합니다.
	if (!FinalizeExecutionApprovalBinding(
		OutResult,
		OutError))
	{
		return false;
	}

	OutResult.MissingRequiredResourceBindings =
		CollectMissingRequiredResourceBindings(OutResult.ReviewPackage.Preview.Workbook);
	OutResult.State =
		OutResult.MissingRequiredResourceBindings.IsEmpty()
			&& OutResult.ResolvedResourceCount == 12
			&& OutResult.SocketCapabilityCheckCount == 8
			? ECFProdBatchPrepState::ReviewReady
			: ECFProdBatchPrepState::ResourceBindingPending;

	FillCounts(OutResult.ReviewPackage.Preview.Workbook, OutResult);

	if (OutResult.ProductCount != 8
		|| OutResult.RoleBindingCount != 48
		|| OutResult.UniqueDataAssetCount != 34
		|| OutResult.AmmoLoadCount != 8
		|| OutResult.ResolvedResourceCount != 12
		|| OutResult.SocketCapabilityCheckCount != 8
		|| OutResult.PreparedProducts.Num() != 8
		|| OutResult.UniqueExecutionTargetCount != 34
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			OutResult.ExecutionBindingFingerprint)
		|| OutResult.State != ECFProdBatchPrepState::ReviewReady)
	{
		OutError = TEXT("Review Package readback exact counts가 frozen First Wave contract와 다릅니다.");
		return false;
	}

	if (!OutResult.ReviewPackage.bNoPersistentWorkbookWriteGuardPassed
		|| !OutResult.ReviewPackage.bNoDirectDataAssetWriteGuardPassed)
	{
		OutError =
			TEXT("Candidate preparation Review Package의 no-write guard가 통과하지 못했습니다.");
		return false;
	}

	return true;
}


// USER approval과 exact8 typed execution manifest가 durable mutation 직전에도 같은 immutable package인지 재검증합니다.
bool FCFProdBatchPrep::ValidatePreparedBatchApproval(
	const FCFProdBatchPrepResult& PreparedBatch,
	const FCFContentReviewApproval& Approval,
	FString& OutError)
{
	using namespace CFProdBatchPrepPrivate;

	OutError.Reset();

	if (PreparedBatch.State != ECFProdBatchPrepState::ReviewReady
		|| PreparedBatch.PreparedProducts.Num() != 8
		|| PreparedBatch.UniqueExecutionTargetCount != 34
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			PreparedBatch.ExecutionBindingFingerprint))
	{
		OutError = TEXT("Prepared Production batch가 ReviewReady exact8/exact34 execution contract를 만족하지 않습니다.");
		return false;
	}

	// USER approval이 current immutable Review Package payload와 exact 일치하는지 generic planning guard를 재사용합니다.
	TArray<FCFContentValidationIssue> ApprovalIssues;
	if (!FCFContentPlanningService::ValidateApproval(
		PreparedBatch.ReviewPackage,
		Approval,
		ApprovalIssues))
	{
		OutError = ApprovalIssues.IsEmpty()
			? TEXT("Production Review Package USER approval validation에 실패했습니다.")
			: ApprovalIssues[0].Code + TEXT(": ") + ApprovalIssues[0].Message;
		return false;
	}

	// PreparedProducts가 review 이후 바뀌지 않았는지 typed execution manifest를 fresh 재계산합니다.
	FString FreshExecutionBindingFingerprint;
	int32 FreshUniqueTargetCount = 0;
	if (!BuildExecutionBindingFingerprint(
		PreparedBatch.ReviewPackage,
		PreparedBatch.ResourceCatalogFingerprint,
		PreparedBatch.PreparedProducts,
		FreshExecutionBindingFingerprint,
		FreshUniqueTargetCount,
		OutError))
	{
		return false;
	}

	if (FreshUniqueTargetCount != 34
		|| !FreshExecutionBindingFingerprint.Equals(
			PreparedBatch.ExecutionBindingFingerprint,
			ESearchCase::CaseSensitive)
		|| !FreshExecutionBindingFingerprint.Equals(
			PreparedBatch.ReviewPackage.ExecutionBindingFingerprint,
			ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Approved Review Package와 fresh typed execution manifest fingerprint가 다릅니다.");
		return false;
	}

	// exact8 Product request가 final approval identity를 모두 그대로 소유하는지 확인합니다.
	for (const FCFProdPreparedProduct& Prepared : PreparedBatch.PreparedProducts)
	{
		const FCFProdProvisionRequest& Request = Prepared.Request;
		if (!Request.ReviewPackageFingerprint.Equals(
				PreparedBatch.ReviewPackage.ReviewPackageFingerprint,
				ESearchCase::CaseSensitive)
			|| !Request.BaseCatalogSnapshotFingerprint.Equals(
				PreparedBatch.ReviewPackage.BaseCatalogSnapshotFingerprint,
				ESearchCase::CaseSensitive)
			|| !Request.ResourceCatalogFingerprint.Equals(
				PreparedBatch.ResourceCatalogFingerprint,
				ESearchCase::CaseSensitive)
			|| !Request.ExecutionBindingFingerprint.Equals(
				PreparedBatch.ExecutionBindingFingerprint,
				ESearchCase::CaseSensitive)
			|| !Request.WorkbookSemanticHash.Equals(
				PreparedBatch.ReviewPackage.ExpectedPostSemanticHash,
				ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Prepared Product request의 review/base/resource/execution/workbook approval identity가 batch와 다릅니다.");
			return false;
		}
	}

	return true;
}

#endif // WITH_EDITOR
