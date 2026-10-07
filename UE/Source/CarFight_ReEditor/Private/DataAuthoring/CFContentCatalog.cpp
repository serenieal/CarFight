// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCatalog.cpp
// Version: v1.0.1
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-05 read-only Catalog / Compare Presentation ViewModel 구현입니다.
// Changelog:
// - v1.0.1: concrete xlsx adapter가 P2로 미연결된 live Product mode에서 Workbook/Diff 상태를 단순 '미로드'가 아니라 명시적 unavailable 상태로 교정.
// - v1.0.0: Persisted Weapon EquipmentPreset + Vehicle Recipe dual-consumer catalog, canonical workbook preview projection,
//   Family/Variant grouping, search/filter, selected compare, source/dependency/resource/diff/status presentation을 최초 구현.
// Migration:
// - Generic Core와 Product authority는 변경하지 않습니다.
// - Live Catalog는 persisted Product를 read-only로 읽고 P0-04 bridge를 재사용합니다.
// - USER-facing 기본 텍스트에 raw ObjectPath/ResourceId/ContentId를 노출하지 않습니다.

#include "DataAuthoring/CFContentCatalog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFProjectileData.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFContentPilot.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Modules/ModuleManager.h"

namespace CFContentCatalogPrivate
{
	// Exact class 또는 derived class의 persisted assets를 Project Asset Registry에서 read-only로 조회합니다.
	void GetAssetsByClass(const UClass& AssetClass, TArray<FAssetData>& OutAssets)
	{
		// Project Asset Registry module입니다.
		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// ClassPath query를 실행할 Asset Registry입니다.
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		// Exact/derived class 조회 filter입니다.
		FARFilter Filter;
		Filter.ClassPaths.Add(AssetClass.GetClassPathName());
		Filter.bRecursiveClasses = true;

		OutAssets.Reset();
		AssetRegistry.GetAssets(Filter, OutAssets);
		OutAssets.Sort([](const FAssetData& Left, const FAssetData& Right)
		{
			return Left.GetSoftObjectPath().ToString()
				< Right.GetSoftObjectPath().ToString();
		});
	}

	// Stable machine ID를 live Presentation 내부 key로만 사용할 lowercase token으로 정규화합니다.
	FString MakeStableToken(const FString& Source)
	{
		// 정규화된 stable token입니다.
		FString StableToken;
		StableToken.Reserve(Source.Len());

		// 연속 separator 축약 상태입니다.
		bool bPreviousWasSeparator = false;
		for (const TCHAR Character : Source)
		{
			// ASCII alpha-numeric 여부입니다.
			const bool bAsciiAlphaNumeric =
				(Character >= TEXT('a') && Character <= TEXT('z'))
				|| (Character >= TEXT('A') && Character <= TEXT('Z'))
				|| (Character >= TEXT('0') && Character <= TEXT('9'));

			if (bAsciiAlphaNumeric)
			{
				StableToken.AppendChar(FChar::ToLower(Character));
				bPreviousWasSeparator = false;
			}
			else if (!bPreviousWasSeparator && !StableToken.IsEmpty())
			{
				StableToken.AppendChar(TEXT('_'));
				bPreviousWasSeparator = true;
			}
		}

		while (StableToken.EndsWith(TEXT("_"), ESearchCase::CaseSensitive))
		{
			StableToken.LeftChopInline(1);
		}

		return StableToken;
	}

	// Unreal Asset의 기술 prefix를 줄인 USER-facing 이름을 반환합니다.
	FString MakeAssetDisplayName(const FString& AssetName)
	{
		// USER-facing asset 이름입니다.
		FString DisplayName = AssetName;
		const TCHAR* Prefixes[] = {
			TEXT("DA_Recipe_"),
			TEXT("DA_"),
			TEXT("BP_"),
			TEXT("SM_"),
			TEXT("SKM_")
		};

		for (const TCHAR* Prefix : Prefixes)
		{
			if (DisplayName.StartsWith(Prefix, ESearchCase::CaseSensitive))
			{
				DisplayName.RightChopInline(FCString::Strlen(Prefix));
				break;
			}
		}

		return DisplayName.IsEmpty()
			? TEXT("이름 미지정")
			: DisplayName;
	}

	// FSoftObjectPath에서 기술 경로 없이 USER-facing asset 이름만 반환합니다.
	FString MakePathDisplayName(const FSoftObjectPath& ObjectPath)
	{
		if (!ObjectPath.IsValid())
		{
			return TEXT("없음");
		}

		return MakeAssetDisplayName(ObjectPath.GetAssetName());
	}

	// Catalog type의 USER-facing label을 반환합니다.
	FString GetTypeDisplayName(const ECFContentCatalogType CatalogType)
	{
		switch (CatalogType)
		{
		case ECFContentCatalogType::Weapon:
			return TEXT("무장");
		case ECFContentCatalogType::Vehicle:
			return TEXT("차량");
		case ECFContentCatalogType::Other:
			return TEXT("기타");
		case ECFContentCatalogType::All:
		default:
			return TEXT("전체");
		}
	}

	// ContentTypeId를 Presentation consumer type으로만 분류하며 Generic Core에는 분기를 추가하지 않습니다.
	ECFContentCatalogType GetCatalogType(const FCFContentTypeId& ContentTypeId)
	{
		if (ContentTypeId.Value.Contains(TEXT("weapon"), ESearchCase::IgnoreCase))
		{
			return ECFContentCatalogType::Weapon;
		}
		if (ContentTypeId.Value.Contains(TEXT("vehicle"), ESearchCase::IgnoreCase))
		{
			return ECFContentCatalogType::Vehicle;
		}
		return ECFContentCatalogType::Other;
	}

	// Resource binding source를 USER-facing 문자열로 변환합니다.
	FString GetResourceSourceText(const ECFResourceBindingSource Source)
	{
		switch (Source)
		{
		case ECFResourceBindingSource::Profile:
			return TEXT("프로파일");
		case ECFResourceBindingSource::Override:
			return TEXT("개별 설정");
		case ECFResourceBindingSource::None:
		default:
			return TEXT("미연결");
		}
	}

	// Semantic Role ID를 technical token 노출 없이 USER-facing 역할명으로 변환합니다.
	FString GetRoleDisplayName(const FCFResolvedResourceBinding& Binding)
	{
		if (Binding.RoleId.Contains(TEXT("muzzle"), ESearchCase::IgnoreCase))
		{
			return TEXT("발사구");
		}
		if (Binding.RoleId.Contains(TEXT("yaw_pivot"), ESearchCase::IgnoreCase))
		{
			return TEXT("좌우 회전 기준");
		}
		if (Binding.RoleId.Contains(TEXT("pitch_pivot"), ESearchCase::IgnoreCase))
		{
			return TEXT("상하 회전 기준");
		}
		if (Binding.RoleId.Contains(TEXT("hardpoint"), ESearchCase::IgnoreCase))
		{
			return TEXT("장착 위치");
		}
		if (Binding.RoleId.Contains(TEXT("wheel_fl"), ESearchCase::IgnoreCase))
		{
			return Binding.CapabilityKind == ECFResourceCapabilityKind::Socket
				? TEXT("앞왼쪽 휠 위치")
				: TEXT("앞왼쪽 휠");
		}
		if (Binding.RoleId.Contains(TEXT("wheel_fr"), ESearchCase::IgnoreCase))
		{
			return Binding.CapabilityKind == ECFResourceCapabilityKind::Socket
				? TEXT("앞오른쪽 휠 위치")
				: TEXT("앞오른쪽 휠");
		}
		if (Binding.RoleId.Contains(TEXT("wheel_rl"), ESearchCase::IgnoreCase))
		{
			return Binding.CapabilityKind == ECFResourceCapabilityKind::Socket
				? TEXT("뒤왼쪽 휠 위치")
				: TEXT("뒤왼쪽 휠");
		}
		if (Binding.RoleId.Contains(TEXT("wheel_rr"), ESearchCase::IgnoreCase))
		{
			return Binding.CapabilityKind == ECFResourceCapabilityKind::Socket
				? TEXT("뒤오른쪽 휠 위치")
				: TEXT("뒤오른쪽 휠");
		}
		if (Binding.RoleId.Contains(TEXT("chassis"), ESearchCase::IgnoreCase))
		{
			return TEXT("차체");
		}
		if (Binding.RoleId.Contains(TEXT("base"), ESearchCase::IgnoreCase))
		{
			return TEXT("터렛 베이스");
		}
		if (Binding.RoleId.Contains(TEXT("yaw"), ESearchCase::IgnoreCase))
		{
			return TEXT("터렛 좌우부");
		}
		if (Binding.RoleId.Contains(TEXT("pitch"), ESearchCase::IgnoreCase))
		{
			return TEXT("터렛 포신부");
		}
		return TEXT("리소스");
	}

	// Resource binding result를 USER-facing resource/socket rows로 투영합니다.
	void AppendResourceRows(
		const FCFResourceBindingResult& BindingResult,
		TArray<FCFContentCatalogResource>& OutResources)
	{
		for (const FCFResolvedResourceBinding& Binding : BindingResult.Bindings)
		{
			if (!Binding.bBound)
			{
				continue;
			}

			// USER-facing resource row입니다.
			FCFContentCatalogResource ResourceRow;
			ResourceRow.RoleDisplayName = GetRoleDisplayName(Binding);
			ResourceRow.ResourceDisplayName = MakePathDisplayName(Binding.ObjectPath);
			ResourceRow.CapabilityDisplayName = Binding.CapabilityName.IsNone()
				? TEXT("")
				: Binding.CapabilityName.ToString();
			ResourceRow.SourceText = GetResourceSourceText(Binding.Source);
			ResourceRow.ResourceObjectPath = Binding.ObjectPath;
			OutResources.Add(MoveTemp(ResourceRow));
		}
	}

	// USER-facing numeric value row를 append합니다.
	void AddNumericValue(
		TArray<FCFContentCatalogValue>& OutValues,
		const FString& FieldKey,
		const FString& DisplayName,
		const double NumericValue,
		const FString& UnitText,
		const FString& SourceText,
		const int32 FractionalDigits = 2)
	{
		// UE 5.8 checked-format contract에 맞춘 bounded precision입니다.
		const int32 SafeFractionalDigits =
			FMath::Clamp(FractionalDigits, 0, 4);

		// USER-facing value row입니다.
		FCFContentCatalogValue Row;
		Row.DisplayName = DisplayName;
		Row.FieldKey = FieldKey;
		Row.NumericValue = NumericValue;
		Row.bNumeric = true;
		Row.UnitText = UnitText;
		Row.SourceText = SourceText;
		switch (SafeFractionalDigits)
		{
		case 0:
			Row.ValueText = FString::Printf(TEXT("%.0f"), NumericValue);
			break;
		case 1:
			Row.ValueText = FString::Printf(TEXT("%.1f"), NumericValue);
			break;
		case 2:
			Row.ValueText = FString::Printf(TEXT("%.2f"), NumericValue);
			break;
		case 3:
			Row.ValueText = FString::Printf(TEXT("%.3f"), NumericValue);
			break;
		case 4:
		default:
			Row.ValueText = FString::Printf(TEXT("%.4f"), NumericValue);
			break;
		}
		if (!UnitText.IsEmpty())
		{
			Row.ValueText += TEXT(" ") + UnitText;
		}
		OutValues.Add(MoveTemp(Row));
	}

	// USER-facing text value row를 append합니다.
	void AddTextValue(
		TArray<FCFContentCatalogValue>& OutValues,
		const FString& FieldKey,
		const FString& DisplayName,
		const FString& ValueText,
		const FString& SourceText)
	{
		// USER-facing text value row입니다.
		FCFContentCatalogValue Row;
		Row.DisplayName = DisplayName;
		Row.FieldKey = FieldKey;
		Row.ValueText = ValueText;
		Row.SourceText = SourceText;
		OutValues.Add(MoveTemp(Row));
	}

	// Dependency row를 raw path/Stable ID 없이 USER-facing asset 이름으로 append합니다.
	void AddPathDependency(
		TArray<FCFContentCatalogDependency>& OutDependencies,
		const FString& RelationDisplayName,
		const FSoftObjectPath& ObjectPath)
	{
		if (!ObjectPath.IsValid())
		{
			return;
		}

		// USER-facing dependency row입니다.
		FCFContentCatalogDependency Dependency;
		Dependency.RelationDisplayName = RelationDisplayName;
		Dependency.TargetDisplayName = MakePathDisplayName(ObjectPath);
		OutDependencies.Add(MoveTemp(Dependency));
	}

	// UObject dependency를 raw ObjectPath 없이 USER-facing asset 이름으로 append합니다.
	void AddObjectDependency(
		TArray<FCFContentCatalogDependency>& OutDependencies,
		const FString& RelationDisplayName,
		const UObject* Object)
	{
		if (Object == nullptr)
		{
			return;
		}

		AddPathDependency(
			OutDependencies,
			RelationDisplayName,
			FSoftObjectPath(Object));
	}

	// Canonical content value의 USER-facing authored source를 반환합니다.
	FString GetCanonicalSourceText(const FCFContentValue& Value)
	{
		switch (Value.State)
		{
		case ECFContentValueState::Value:
			return TEXT("직접 입력");
		case ECFContentValueState::None:
			return TEXT("명시적 없음");
		case ECFContentValueState::Inherit:
		default:
			return TEXT("프로파일 / 상속");
		}
	}

	// Canonical content value를 raw ResourceId/ContentId 노출 없이 USER-facing text로 변환합니다.
	FString FormatCanonicalValue(const FCFContentValue& Value)
	{
		if (Value.State == ECFContentValueState::None)
		{
			return TEXT("없음");
		}
		if (Value.State == ECFContentValueState::Inherit)
		{
			return TEXT("상속");
		}

		switch (Value.Type)
		{
		case ECFContentValueType::Boolean:
			return Value.bBooleanValue ? TEXT("켜짐") : TEXT("꺼짐");
		case ECFContentValueType::SignedInteger:
			return FString::Printf(TEXT("%lld"), Value.SignedIntegerValue);
		case ECFContentValueType::UnsignedInteger:
			return FString::Printf(TEXT("%llu"), Value.UnsignedIntegerValue);
		case ECFContentValueType::Float:
		case ECFContentValueType::Double:
			return FString::Printf(TEXT("%.4g"), Value.FloatingPointValue);
		case ECFContentValueType::Text:
			return Value.TextValue.SourceString;
		case ECFContentValueType::ContentReference:
			return TEXT("다른 콘텐츠에 연결됨");
		case ECFContentValueType::ResourceReference:
			return TEXT("리소스에 연결됨");
		case ECFContentValueType::Enum:
		case ECFContentValueType::NameId:
		case ECFContentValueType::String:
		default:
			return Value.StringValue;
		}
	}

	// Canonical content value가 relative compare 가능한 numeric인지 판정합니다.
	bool TryGetCanonicalNumeric(
		const FCFContentValue& Value,
		double& OutNumericValue)
	{
		if (Value.State != ECFContentValueState::Value)
		{
			return false;
		}

		switch (Value.Type)
		{
		case ECFContentValueType::SignedInteger:
			OutNumericValue = static_cast<double>(Value.SignedIntegerValue);
			return true;
		case ECFContentValueType::UnsignedInteger:
			OutNumericValue = static_cast<double>(Value.UnsignedIntegerValue);
			return true;
		case ECFContentValueType::Float:
		case ECFContentValueType::Double:
			OutNumericValue = Value.FloatingPointValue;
			return true;
		default:
			return false;
		}
	}

	// Canonical record type에 대응하는 primary sheet descriptor를 찾습니다.
	const FCFContentSheetDescriptor* FindPrimarySheet(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentTypeId& ContentTypeId)
	{
		return Workbook.Sheets.FindByPredicate(
			[&ContentTypeId](const FCFContentSheetDescriptor& Sheet)
			{
				return !Sheet.IsChildSheet()
					&& Sheet.ContentTypeId == ContentTypeId;
			});
	}

	// Stable ColumnId에 대응하는 field descriptor를 찾습니다.
	const FCFContentFieldDescriptor* FindFieldDescriptor(
		const FCFContentSheetDescriptor* Sheet,
		const FString& ColumnId)
	{
		if (Sheet == nullptr)
		{
			return nullptr;
		}

		return Sheet->Fields.FindByPredicate(
			[&ColumnId](const FCFContentFieldDescriptor& Field)
			{
				return Field.ColumnId.Equals(
					ColumnId,
					ESearchCase::CaseSensitive);
			});
	}

	// Canonical record에서 USER-facing display name 후보 field를 찾습니다.
	FString FindCanonicalDisplayName(
		const FCFContentRecord& Record,
		const FCFContentSheetDescriptor* Sheet)
	{
		// Display name 후보 ColumnId 우선순위입니다.
		const TCHAR* CandidateIds[] = {
			TEXT("display_name"),
			TEXT("displayname"),
			TEXT("name"),
			TEXT("title"),
			TEXT("label")
		};

		for (const TCHAR* CandidateId : CandidateIds)
		{
			for (const TPair<FString, FCFContentValue>& Pair : Record.Fields)
			{
				if (!Pair.Key.Equals(CandidateId, ESearchCase::IgnoreCase)
					|| Pair.Value.State != ECFContentValueState::Value)
				{
					continue;
				}

				// USER-facing candidate value입니다.
				const FString CandidateValue = FormatCanonicalValue(Pair.Value);
				if (!CandidateValue.IsEmpty())
				{
					return CandidateValue;
				}
			}
		}

		if (Sheet != nullptr)
		{
			for (const FCFContentFieldDescriptor& Descriptor : Sheet->Fields)
			{
				if (!Descriptor.DisplayLabel.Contains(TEXT("이름"))
					&& !Descriptor.DisplayLabel.Contains(TEXT("Name"), ESearchCase::IgnoreCase))
				{
					continue;
				}

				// Display label 기반 candidate field입니다.
				const FCFContentValue* CandidateValue =
					Record.Fields.Find(Descriptor.ColumnId);
				if (CandidateValue == nullptr
					|| CandidateValue->State != ECFContentValueState::Value)
				{
					continue;
				}

				// Display label 기반 USER-facing value입니다.
				const FString CandidateText = FormatCanonicalValue(*CandidateValue);
				if (!CandidateText.IsEmpty())
				{
					return CandidateText;
				}
			}
		}

		return TEXT("표시 이름 미지정");
	}

	// Canonical management state를 USER-facing 상태로 변환합니다.
	FString GetManagementText(const ECFContentManagementState ManagementState)
	{
		return ManagementState == ECFContentManagementState::Managed
			? TEXT("CCAS 관리 대상")
			: TEXT("외부 읽기 전용");
	}

	// Compile diff kind를 USER-facing 상태로 변환합니다.
	FString GetDiffKindText(const ECFContentCompileDiffKind DiffKind)
	{
		switch (DiffKind)
		{
		case ECFContentCompileDiffKind::Added:
			return TEXT("추가 예정");
		case ECFContentCompileDiffKind::Removed:
			return TEXT("Retire 예정");
		case ECFContentCompileDiffKind::Modified:
			return TEXT("변경 예정");
		case ECFContentCompileDiffKind::Unchanged:
		default:
			return TEXT("변경 없음");
		}
	}

	// Content key가 exact 같은지 비교합니다.
	bool AreKeysEqual(
		const FCFContentKey& Left,
		const FCFContentKey& Right)
	{
		return Left == Right;
	}

	// USER-facing resource binding rows를 deterministic하게 정렬합니다.
	void SortResourceRows(TArray<FCFContentCatalogResource>& ResourceRows)
	{
		ResourceRows.Sort(
			[](const FCFContentCatalogResource& Left, const FCFContentCatalogResource& Right)
			{
				const FString LeftKey =
					Left.RoleDisplayName + TEXT("|")
					+ Left.ResourceDisplayName + TEXT("|")
					+ Left.CapabilityDisplayName;
				const FString RightKey =
					Right.RoleDisplayName + TEXT("|")
					+ Right.ResourceDisplayName + TEXT("|")
					+ Right.CapabilityDisplayName;
				return LeftKey < RightKey;
			});
	}
}

// Persisted Vehicle Recipe + Weapon EquipmentPreset을 읽고 P0-04 shared typed bridge로 live Catalog를 구성합니다.
bool FCFContentCatalogVM::RefreshLiveCatalog(FString& OutError)
{
	OutError.Reset();
	Entries.Reset();
	FilteredEntries.Reset();
	Groups.Reset();
	SelectedEntryIndex = INDEX_NONE;
	CompareEntryIndex = INDEX_NONE;

	// Weapon/Vehicle가 exact same P0-04 Resource Core context를 공유하는 live catalog context입니다.
	FCFContentPilotContext SharedPilotContext;
	if (!SharedPilotContext.Initialize(OutError))
	{
		return false;
	}

	// Provider-local bridge가 보고한 issue 수입니다.
	int32 TotalIssueCount = 0;

	// Persisted Vehicle Recipe assets입니다.
	TArray<FAssetData> VehicleRecipeAssets;
	CFContentCatalogPrivate::GetAssetsByClass(
		*UCFVehicleRecipeData::StaticClass(),
		VehicleRecipeAssets);

	for (const FAssetData& AssetData : VehicleRecipeAssets)
	{
		// Persisted Editor-only Vehicle Recipe입니다.
		const UCFVehicleRecipeData* Recipe =
			Cast<UCFVehicleRecipeData>(AssetData.GetAsset());
		if (Recipe == nullptr)
		{
			++TotalIssueCount;
			continue;
		}

		// P0-04 existing Vehicle backend + shared Resource Core projection 결과입니다.
		FCFVehiclePilotResult PilotResult;
		const bool bPilotSucceeded =
			FCFVehicleResourcePilot::Build(
				*Recipe,
				SharedPilotContext,
				PilotResult);
		TotalIssueCount += PilotResult.Issues.Num();

		// USER-facing Vehicle catalog row입니다.
		FCFContentCatalogEntry Entry;
		Entry.Key.ContentTypeId.Value = TEXT("vehicle");
		Entry.Key.ContentId =
			FString::Printf(
				TEXT("vehicle_%s"),
				*Recipe->RecipeId.ToString(EGuidFormats::Digits).ToLower());
		Entry.CatalogType = ECFContentCatalogType::Vehicle;

		// Target VehicleData가 있으면 그 이름을 우선하고 없으면 Recipe 이름을 사용합니다.
		const FSoftObjectPath TargetVehiclePath =
			Recipe->TargetVehicleData.ToSoftObjectPath();
		Entry.DisplayName = TargetVehiclePath.IsValid()
			? CFContentCatalogPrivate::MakePathDisplayName(TargetVehiclePath)
			: CFContentCatalogPrivate::MakeAssetDisplayName(AssetData.AssetName.ToString());
		Entry.SecondaryText = bPilotSucceeded
			? TEXT("차량 · 현재 Product 읽기 전용")
			: TEXT("차량 · 검증 이슈 확인 필요");
		Entry.FamilyDisplayName = TEXT("Family 미지정");
		Entry.VariantDisplayName = Recipe->VehicleArchetypeId.IsNone()
			? TEXT("현재 차량")
			: FString::Printf(
				TEXT("Archetype: %s"),
				*Recipe->VehicleArchetypeId.ToString());
		Entry.ValidationIssueCount = PilotResult.Issues.Num();
		Entry.EditTabId = FName(TEXT("CarFight.VehicleAuthoring"));
		Entry.SourceAssetPath = AssetData.GetSoftObjectPath();

		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.feel.acceleration"),
			TEXT("가속감"),
			Recipe->DrivingFeelIntent.AccelerationFeel,
			TEXT(""),
			TEXT("Vehicle Recipe"));
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.feel.steering"),
			TEXT("조향 민첩성"),
			Recipe->DrivingFeelIntent.SteeringAgility,
			TEXT(""),
			TEXT("Vehicle Recipe"));
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.feel.grip"),
			TEXT("접지감"),
			Recipe->DrivingFeelIntent.GripFeel,
			TEXT(""),
			TEXT("Vehicle Recipe"));
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.feel.suspension"),
			TEXT("서스펜션 단단함"),
			Recipe->DrivingFeelIntent.SuspensionFirmness,
			TEXT(""),
			TEXT("Vehicle Recipe"));
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.authoring_revision"),
			TEXT("제작 리비전"),
			static_cast<double>(Recipe->AuthoringRevision),
			TEXT(""),
			TEXT("Vehicle Recipe"),
			0);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.hardpoint_count"),
			TEXT("장착 위치 수"),
			static_cast<double>(Recipe->HardpointIntents.Num()),
			TEXT("개"),
			TEXT("Vehicle Recipe"),
			0);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("vehicle.mount_count"),
			TEXT("장비 마운트 수"),
			static_cast<double>(Recipe->MountIntents.Num()),
			TEXT("개"),
			TEXT("Vehicle Recipe"),
			0);

		CFContentCatalogPrivate::AppendResourceRows(
			PilotResult.ResourceBindings,
			Entry.Resources);
		CFContentCatalogPrivate::SortResourceRows(Entry.Resources);

		CFContentCatalogPrivate::AddPathDependency(
			Entry.Dependencies,
			TEXT("런타임 차량 데이터"),
			TargetVehiclePath);
		for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
		{
			CFContentCatalogPrivate::AddPathDependency(
				Entry.Dependencies,
				TEXT("기본 장비"),
				MountIntent.DefaultEquipmentPresetData.ToSoftObjectPath());
		}

		Entries.Add(MoveTemp(Entry));
	}

	// Persisted Weapon package assets입니다.
	TArray<FAssetData> EquipmentPresetAssets;
	CFContentCatalogPrivate::GetAssetsByClass(
		*UCFEquipmentPresetData::StaticClass(),
		EquipmentPresetAssets);

	// 동일 WeaponData + TurretMountData 조합의 P0-04 bridge 중복 등록을 막는 read-only result cache입니다.
	TMap<FString, FCFWeaponPilotResult> WeaponPilotCache;

	for (const FAssetData& AssetData : EquipmentPresetAssets)
	{
		// Persisted EquipmentPresetData입니다.
		const UCFEquipmentPresetData* EquipmentPreset =
			Cast<UCFEquipmentPresetData>(AssetData.GetAsset());
		if (EquipmentPreset == nullptr
			|| EquipmentPreset->DefaultWeaponData == nullptr
			|| EquipmentPreset->DefaultTurretMountData == nullptr)
		{
			continue;
		}

		// Existing CF-FQ-055 WeaponData입니다.
		const UCFWeaponData* WeaponData =
			EquipmentPreset->DefaultWeaponData.Get();

		// Existing CF-FQ-055 TurretMountData입니다.
		const UCFTurretMountData* TurretMountData =
			EquipmentPreset->DefaultTurretMountData.Get();

		// Same typed pair를 exact1 bridge result로 재사용할 cache key입니다.
		const FString PairCacheKey =
			FSoftObjectPath(WeaponData).ToString()
			+ TEXT("|")
			+ FSoftObjectPath(TurretMountData).ToString();

		// P0-04 shared Resource Core projection 결과입니다.
		const FCFWeaponPilotResult* CachedPilotResult =
			WeaponPilotCache.Find(PairCacheKey);
		if (CachedPilotResult == nullptr)
		{
			// 새 typed pair의 P0-04 projection 결과입니다.
			FCFWeaponPilotResult NewPilotResult;
			FCFWeaponResourcePilot::Build(
				*WeaponData,
				*TurretMountData,
				SharedPilotContext,
				NewPilotResult);
			TotalIssueCount += NewPilotResult.Issues.Num();
			CachedPilotResult =
				&WeaponPilotCache.Add(
					PairCacheKey,
					MoveTemp(NewPilotResult));
		}

		// USER-facing Weapon catalog row입니다.
		FCFContentCatalogEntry Entry;
		Entry.Key.ContentTypeId.Value = TEXT("weapon");
		Entry.Key.ContentId =
			TEXT("weapon_")
			+ CFContentCatalogPrivate::MakeStableToken(
				EquipmentPreset->EquipmentId.ToString());
		if (Entry.Key.ContentId.Equals(TEXT("weapon_"), ESearchCase::CaseSensitive))
		{
			Entry.Key.ContentId =
				TEXT("weapon_")
				+ CFContentCatalogPrivate::MakeStableToken(
					AssetData.AssetName.ToString());
		}
		Entry.CatalogType = ECFContentCatalogType::Weapon;

		// Player-facing Equipment display name을 우선합니다.
		const FString EquipmentDisplayName =
			EquipmentPreset->DisplayName.IsEmpty()
			? FString()
			: EquipmentPreset->DisplayName.ToString();
		Entry.DisplayName = !EquipmentDisplayName.IsEmpty()
			? EquipmentDisplayName
			: CFContentCatalogPrivate::MakeAssetDisplayName(
				WeaponData->GetName());
		Entry.SecondaryText = CachedPilotResult->bWeaponContractValidated
			? TEXT("무장 · 현재 Product 읽기 전용")
			: TEXT("무장 · 검증 이슈 확인 필요");
		Entry.FamilyDisplayName = TEXT("Family 미지정");
		Entry.VariantDisplayName =
			CFContentCatalogPrivate::MakeAssetDisplayName(
				TurretMountData->GetName());
		Entry.ValidationIssueCount = CachedPilotResult->Issues.Num();
		Entry.EditTabId = FName(TEXT("CarFight.WeaponGuide"));
		Entry.SourceAssetPath = AssetData.GetSoftObjectPath();

		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("weapon.fire_rate"),
			TEXT("분당 발사속도"),
			WeaponData->FireRatePerMinute,
			TEXT("RPM"),
			TEXT("WeaponData"),
			1);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("weapon.max_range"),
			TEXT("최대 사거리"),
			WeaponData->MaxRange,
			TEXT("cm"),
			TEXT("WeaponData"),
			0);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("weapon.mass"),
			TEXT("무기 질량"),
			WeaponData->WeaponMassKg,
			TEXT("kg"),
			TEXT("WeaponData"),
			1);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("weapon.spread"),
			TEXT("탄퍼짐"),
			WeaponData->SpreadDeg,
			TEXT("deg"),
			TEXT("WeaponData"),
			2);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("weapon.magazine"),
			TEXT("탄창 크기"),
			static_cast<double>(WeaponData->MagazineSize),
			TEXT("발"),
			TEXT("WeaponData"),
			0);
		CFContentCatalogPrivate::AddNumericValue(
			Entry.Values,
			TEXT("weapon.reload"),
			TEXT("재장전 시간"),
			WeaponData->ReloadTimeSeconds,
			TEXT("s"),
			TEXT("WeaponData"),
			2);

		CFContentCatalogPrivate::AppendResourceRows(
			CachedPilotResult->ResourceBindings,
			Entry.Resources);
		CFContentCatalogPrivate::SortResourceRows(Entry.Resources);

		CFContentCatalogPrivate::AddObjectDependency(
			Entry.Dependencies,
			TEXT("무기 데이터"),
			WeaponData);
		CFContentCatalogPrivate::AddObjectDependency(
			Entry.Dependencies,
			TEXT("터렛 구성"),
			TurretMountData);
		CFContentCatalogPrivate::AddObjectDependency(
			Entry.Dependencies,
			TEXT("발사체"),
			WeaponData->DefaultProjectileData.Get());
		CFContentCatalogPrivate::AddObjectDependency(
			Entry.Dependencies,
			TEXT("탄약"),
			WeaponData->DefaultAmmoData.Get());

		Entries.Add(MoveTemp(Entry));
	}

	Entries.Sort(
		[](const FCFContentCatalogEntry& Left, const FCFContentCatalogEntry& Right)
		{
			if (Left.CatalogType != Right.CatalogType)
			{
				return static_cast<uint8>(Left.CatalogType)
					< static_cast<uint8>(Right.CatalogType);
			}
			return Left.DisplayName < Right.DisplayName;
		});

	Status.CatalogSourceText =
		TEXT("현재 Persisted Product · 읽기 전용");
	Status.WorkbookAuthorityText =
		TEXT("Workbook 권한 미전환 · CCAS-P0-07 대기");
	Status.WorkbookRevisionText =
		TEXT("Workbook Preview 사용 불가 · xlsx adapter P2 미연결");
	Status.ValidationText =
		TotalIssueCount == 0
			? TEXT("Live typed validation 이슈 없음")
			: FString::Printf(
				TEXT("Live typed validation 이슈 %d건"),
				TotalIssueCount);
	Status.DiffPreviewText =
		TEXT("Diff Preview 사용 불가 · xlsx adapter P2 미연결");
	Status.ApplyStateText =
		TEXT("Product Apply 비활성 · 읽기/비교만 가능");

	RebuildFilteredEntries();
	return !Entries.IsEmpty();
}

// P0-01 Canonical Workbook + P0-02 Compile Preview를 USER-facing Catalog/Diff/Family projection에 반영합니다.
void FCFContentCatalogVM::ApplyCompilePreview(
	const FCFContentCompileResult& CompileResult)
{
	Entries.Reset();
	SelectedEntryIndex = INDEX_NONE;
	CompareEntryIndex = INDEX_NONE;

	for (const FCFContentRecord& Record : CompileResult.Workbook.Records)
	{
		Entries.Add(
			BuildCanonicalEntry(
				Record,
				CompileResult.Workbook));
	}

	for (FCFContentCatalogEntry& Entry : Entries)
	{
		// Exact record의 compiler Diff입니다.
		const FCFContentCompileDiff* Diff =
			CompileResult.Diffs.FindByPredicate(
				[&Entry](const FCFContentCompileDiff& Candidate)
				{
					return CFContentCatalogPrivate::AreKeysEqual(
						Candidate.Key,
						Entry.Key);
				});
		if (Diff != nullptr)
		{
			Entry.Diff = *Diff;
			Entry.bHasDiff = true;
		}

		for (const FCFContentDependencyEdge& Edge : CompileResult.DependencyEdges)
		{
			if (!CFContentCatalogPrivate::AreKeysEqual(
				Edge.From,
				Entry.Key))
			{
				continue;
			}

			// Dependency target의 USER-facing display name을 찾습니다.
			const FCFContentRecord* TargetRecord =
				CompileResult.Workbook.Records.FindByPredicate(
					[&Edge](const FCFContentRecord& Candidate)
					{
						return CFContentCatalogPrivate::AreKeysEqual(
							Candidate.Key,
							Edge.To);
					});

			// USER-facing dependency row입니다.
			FCFContentCatalogDependency Dependency;
			Dependency.RelationDisplayName =
				TEXT("콘텐츠 의존성");
			if (TargetRecord != nullptr)
			{
				// Target primary sheet입니다.
				const FCFContentSheetDescriptor* TargetSheet =
					CFContentCatalogPrivate::FindPrimarySheet(
						CompileResult.Workbook,
						TargetRecord->Key.ContentTypeId);
				Dependency.TargetDisplayName =
					CFContentCatalogPrivate::FindCanonicalDisplayName(
						*TargetRecord,
						TargetSheet);
			}
			else
			{
				Dependency.TargetDisplayName =
					TEXT("외부 콘텐츠");
			}
			Entry.Dependencies.Add(MoveTemp(Dependency));
		}

		for (const FCFContentValidationIssue& Issue : CompileResult.Issues)
		{
			if (Issue.Path.Contains(
				Entry.Key.ContentId,
				ESearchCase::CaseSensitive))
			{
				++Entry.ValidationIssueCount;
			}
		}
	}

	Entries.Sort(
		[](const FCFContentCatalogEntry& Left, const FCFContentCatalogEntry& Right)
		{
			if (Left.FamilyDisplayName != Right.FamilyDisplayName)
			{
				return Left.FamilyDisplayName < Right.FamilyDisplayName;
			}
			return Left.DisplayName < Right.DisplayName;
		});

	// Blocking validation issue 수입니다.
	int32 BlockingIssueCount = 0;
	// Non-blocking validation issue 수입니다.
	int32 WarningIssueCount = 0;
	for (const FCFContentValidationIssue& Issue : CompileResult.Issues)
	{
		if (Issue.bBlocking)
		{
			++BlockingIssueCount;
		}
		else
		{
			++WarningIssueCount;
		}
	}

	// Changed diff 수입니다.
	int32 ChangedDiffCount = 0;
	for (const FCFContentCompileDiff& Diff : CompileResult.Diffs)
	{
		if (Diff.Kind != ECFContentCompileDiffKind::Unchanged)
		{
			++ChangedDiffCount;
		}
	}

	Status.CatalogSourceText =
		TEXT("Canonical Workbook Preview · 읽기 전용");
	Status.WorkbookAuthorityText =
		TEXT("Workbook 권한 미전환 · CCAS-P0-07 대기");
	Status.WorkbookRevisionText =
		FString::Printf(
			TEXT("Workbook Schema Revision %d"),
			CompileResult.Workbook.SchemaRevision);
	Status.ValidationText =
		FString::Printf(
			TEXT("Blocking %d · Warning %d"),
			BlockingIssueCount,
			WarningIssueCount);
	Status.DiffPreviewText =
		FString::Printf(
			TEXT("변경 대상 %d / 전체 Diff %d"),
			ChangedDiffCount,
			CompileResult.Diffs.Num());
	Status.ApplyStateText =
		TEXT("Product Apply 비활성 · Preview만 가능");

	RebuildFilteredEntries();
}

// Automation 또는 후속 provider adapter가 이미 만든 entries를 read-only Presentation state로 주입합니다.
void FCFContentCatalogVM::SetEntriesForPresentation(
	const TArray<FCFContentCatalogEntry>& InEntries,
	const FCFContentCatalogStatus& InStatus)
{
	Entries = InEntries;
	Status = InStatus;
	SelectedEntryIndex = INDEX_NONE;
	CompareEntryIndex = INDEX_NONE;
	RebuildFilteredEntries();
}

// Search text를 갱신하고 filtered rows/family groups를 다시 계산합니다.
void FCFContentCatalogVM::SetSearchText(
	const FString& InSearchText)
{
	SearchText = InSearchText.TrimStartAndEnd();
	RebuildFilteredEntries();
}

// All/Weapon/Vehicle type filter를 갱신하고 filtered rows/family groups를 다시 계산합니다.
void FCFContentCatalogVM::SetTypeFilter(
	const ECFContentCatalogType InTypeFilter)
{
	TypeFilter = InTypeFilter;
	RebuildFilteredEntries();
}

// Exact logical key를 current selected row로 설정합니다.
bool FCFContentCatalogVM::SelectEntry(
	const FCFContentKey& Key)
{
	SelectedEntryIndex = FindEntryIndex(Key);
	if (SelectedEntryIndex == INDEX_NONE)
	{
		CompareEntryIndex = INDEX_NONE;
		return false;
	}

	if (CompareEntryIndex == SelectedEntryIndex)
	{
		CompareEntryIndex = INDEX_NONE;
	}
	return true;
}

// Exact logical key를 current compare row로 설정합니다.
bool FCFContentCatalogVM::SelectCompareEntry(
	const FCFContentKey& Key)
{
	// Compare candidate index입니다.
	const int32 CandidateIndex = FindEntryIndex(Key);
	if (CandidateIndex == INDEX_NONE
		|| CandidateIndex == SelectedEntryIndex)
	{
		return false;
	}

	CompareEntryIndex = CandidateIndex;
	return true;
}

// Current selected/compare row를 함께 지웁니다.
void FCFContentCatalogVM::ClearSelection()
{
	SelectedEntryIndex = INDEX_NONE;
	CompareEntryIndex = INDEX_NONE;
}

// Compare row만 지웁니다.
void FCFContentCatalogVM::ClearCompareEntry()
{
	CompareEntryIndex = INDEX_NONE;
}

// Current selected row입니다. 선택이 없으면 nullptr입니다.
const FCFContentCatalogEntry* FCFContentCatalogVM::GetSelectedEntry() const
{
	return Entries.IsValidIndex(SelectedEntryIndex)
		? &Entries[SelectedEntryIndex]
		: nullptr;
}

// Current compare row입니다. 선택이 없으면 nullptr입니다.
const FCFContentCatalogEntry* FCFContentCatalogVM::GetCompareEntry() const
{
	return Entries.IsValidIndex(CompareEntryIndex)
		? &Entries[CompareEntryIndex]
		: nullptr;
}

// Current Selected vs Compare absolute/relative/source rows를 deterministic 순서로 만듭니다.
TArray<FCFContentCompareRow> FCFContentCatalogVM::BuildCompareRows() const
{
	// 결과 compare rows입니다.
	TArray<FCFContentCompareRow> Rows;

	// Current selected row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		GetSelectedEntry();

	// Current compare row입니다.
	const FCFContentCatalogEntry* CompareEntry =
		GetCompareEntry();
	if (SelectedEntry == nullptr || CompareEntry == nullptr)
	{
		return Rows;
	}

	// Selected/Compare 양쪽의 unique field keys입니다.
	TSet<FString> FieldKeys;
	for (const FCFContentCatalogValue& Value : SelectedEntry->Values)
	{
		FieldKeys.Add(Value.FieldKey);
	}
	for (const FCFContentCatalogValue& Value : CompareEntry->Values)
	{
		FieldKeys.Add(Value.FieldKey);
	}

	// Deterministic field key order입니다.
	TArray<FString> SortedFieldKeys = FieldKeys.Array();
	SortedFieldKeys.Sort();

	for (const FString& FieldKey : SortedFieldKeys)
	{
		// Selected side value입니다.
		const FCFContentCatalogValue* SelectedValue =
			SelectedEntry->Values.FindByPredicate(
				[&FieldKey](const FCFContentCatalogValue& Candidate)
				{
					return Candidate.FieldKey.Equals(
						FieldKey,
						ESearchCase::CaseSensitive);
				});

		// Compare side value입니다.
		const FCFContentCatalogValue* CompareValue =
			CompareEntry->Values.FindByPredicate(
				[&FieldKey](const FCFContentCatalogValue& Candidate)
				{
					return Candidate.FieldKey.Equals(
						FieldKey,
						ESearchCase::CaseSensitive);
				});

		// USER-facing compare row입니다.
		FCFContentCompareRow Row;
		Row.DisplayName =
			SelectedValue != nullptr
				? SelectedValue->DisplayName
				: (CompareValue != nullptr
					? CompareValue->DisplayName
					: TEXT("항목"));
		Row.SelectedValue =
			SelectedValue != nullptr
				? SelectedValue->ValueText
				: TEXT("—");
		Row.CompareValue =
			CompareValue != nullptr
				? CompareValue->ValueText
				: TEXT("—");
		Row.SelectedSource =
			SelectedValue != nullptr
				? SelectedValue->SourceText
				: TEXT("—");
		Row.CompareSource =
			CompareValue != nullptr
				? CompareValue->SourceText
				: TEXT("—");

		if (SelectedValue != nullptr
			&& CompareValue != nullptr
			&& SelectedValue->bNumeric
			&& CompareValue->bNumeric
			&& SelectedValue->UnitText.Equals(
				CompareValue->UnitText,
				ESearchCase::CaseSensitive))
		{
			// Compare baseline 대비 selected absolute delta입니다.
			const double Delta =
				SelectedValue->NumericValue
				- CompareValue->NumericValue;

			// 표시 단위 suffix입니다.
			const FString UnitSuffix =
				SelectedValue->UnitText.IsEmpty()
					? TEXT("")
					: TEXT(" ") + SelectedValue->UnitText;

			if (!FMath::IsNearlyZero(CompareValue->NumericValue))
			{
				// Compare baseline 대비 selected percentage delta입니다.
				const double Percent =
					(Delta / CompareValue->NumericValue) * 100.0;
				Row.RelativeDelta =
					FString::Printf(
						TEXT("%+.3f%s (%+.1f%%)"),
						Delta,
						*UnitSuffix,
						Percent);
			}
			else
			{
				Row.RelativeDelta =
					FString::Printf(
						TEXT("%+.3f%s"),
						Delta,
						*UnitSuffix);
			}
		}
		else
		{
			Row.RelativeDelta =
				Row.SelectedValue.Equals(
					Row.CompareValue,
					ESearchCase::CaseSensitive)
					? TEXT("동일")
					: TEXT("다름");
		}

		Rows.Add(MoveTemp(Row));
	}

	return Rows;
}

// Current selected row의 dependency view를 USER-facing multi-line text로 만듭니다.
FString FCFContentCatalogVM::BuildDependencyText() const
{
	// Current selected row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		GetSelectedEntry();
	if (SelectedEntry == nullptr)
	{
		return TEXT("콘텐츠를 선택하면 의존성을 표시합니다.");
	}
	if (SelectedEntry->Dependencies.IsEmpty())
	{
		return TEXT("표시할 의존성이 없습니다.");
	}

	// USER-facing dependency lines입니다.
	TArray<FString> Lines;
	for (const FCFContentCatalogDependency& Dependency : SelectedEntry->Dependencies)
	{
		Lines.Add(
			FString::Printf(
				TEXT("• %s: %s"),
				*Dependency.RelationDisplayName,
				*Dependency.TargetDisplayName));
	}
	return FString::Join(Lines, TEXT("\n"));
}

// Current selected row의 resource/socket view를 USER-facing multi-line text로 만듭니다.
FString FCFContentCatalogVM::BuildResourceText() const
{
	// Current selected row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		GetSelectedEntry();
	if (SelectedEntry == nullptr)
	{
		return TEXT("콘텐츠를 선택하면 리소스와 소켓을 표시합니다.");
	}
	if (SelectedEntry->Resources.IsEmpty())
	{
		return TEXT("표시할 Resource / Socket binding이 없습니다.");
	}

	// USER-facing resource lines입니다.
	TArray<FString> Lines;
	for (const FCFContentCatalogResource& Resource : SelectedEntry->Resources)
	{
		// Optional capability suffix입니다.
		const FString CapabilitySuffix =
			Resource.CapabilityDisplayName.IsEmpty()
				? TEXT("")
				: FString::Printf(
					TEXT(" · %s"),
					*Resource.CapabilityDisplayName);
		Lines.Add(
			FString::Printf(
				TEXT("• %s: %s%s · 출처 %s"),
				*Resource.RoleDisplayName,
				*Resource.ResourceDisplayName,
				*CapabilitySuffix,
				*Resource.SourceText));
	}
	return FString::Join(Lines, TEXT("\n"));
}

// Current selected row의 Diff Preview를 USER-facing multi-line text로 만듭니다.
FString FCFContentCatalogVM::BuildSelectedDiffText() const
{
	// Current selected row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		GetSelectedEntry();
	if (SelectedEntry == nullptr)
	{
		return TEXT("콘텐츠를 선택하면 Diff Preview를 표시합니다.");
	}
	if (!SelectedEntry->bHasDiff)
	{
		return TEXT("현재 Workbook Diff Preview가 없습니다.");
	}

	// Diff kind summary입니다.
	FString Result =
		CFContentCatalogPrivate::GetDiffKindText(
			SelectedEntry->Diff.Kind);
	if (SelectedEntry->Diff.ChangedPaths.IsEmpty())
	{
		return Result;
	}

	// USER-facing changed path count만 기본 표시하며 raw stable path는 숨깁니다.
	Result += FString::Printf(
		TEXT("\n변경 항목 %d개"),
		SelectedEntry->Diff.ChangedPaths.Num());
	return Result;
}

// USER가 technical ObjectPath/Stable ID를 보지 않고 기존 전문 authoring UI로 이동할 TabId를 반환합니다.
FName FCFContentCatalogVM::GetSelectedEditTabId() const
{
	// Current selected row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		GetSelectedEntry();
	return SelectedEntry != nullptr
		? SelectedEntry->EditTabId
		: NAME_None;
}

// Current raw entries와 search/type filter에서 filtered list를 재구성합니다.
void FCFContentCatalogVM::RebuildFilteredEntries()
{
	FilteredEntries.Reset();

	for (const FCFContentCatalogEntry& Entry : Entries)
	{
		if (TypeFilter != ECFContentCatalogType::All
			&& Entry.CatalogType != TypeFilter)
		{
			continue;
		}

		// USER-facing searchable presentation text입니다.
		FString SearchableText =
			Entry.DisplayName + TEXT(" ")
			+ Entry.SecondaryText + TEXT(" ")
			+ Entry.FamilyDisplayName + TEXT(" ")
			+ Entry.VariantDisplayName + TEXT(" ")
			+ Entry.DesignIntent;

		for (const FCFContentCatalogValue& Value : Entry.Values)
		{
			SearchableText += TEXT(" ")
				+ Value.DisplayName
				+ TEXT(" ")
				+ Value.ValueText
				+ TEXT(" ")
				+ Value.SourceText;
		}

		if (!SearchText.IsEmpty()
			&& !SearchableText.Contains(
				SearchText,
				ESearchCase::IgnoreCase))
		{
			continue;
		}

		FilteredEntries.Add(Entry);
	}

	FilteredEntries.Sort(
		[](const FCFContentCatalogEntry& Left, const FCFContentCatalogEntry& Right)
		{
			if (Left.CatalogType != Right.CatalogType)
			{
				return static_cast<uint8>(Left.CatalogType)
					< static_cast<uint8>(Right.CatalogType);
			}
			if (Left.FamilyDisplayName != Right.FamilyDisplayName)
			{
				return Left.FamilyDisplayName
					< Right.FamilyDisplayName;
			}
			return Left.DisplayName < Right.DisplayName;
		});

	RebuildGroups();
}

// Current filtered entries를 consumer + Family 기준 group으로 재구성합니다.
void FCFContentCatalogVM::RebuildGroups()
{
	Groups.Reset();

	for (const FCFContentCatalogEntry& Entry : FilteredEntries)
	{
		// Consumer + Family group identity입니다.
		const FString GroupKey =
			CFContentCatalogPrivate::GetTypeDisplayName(
				Entry.CatalogType)
			+ TEXT("|")
			+ Entry.FamilyDisplayName;

		// Existing group입니다.
		FCFContentCatalogGroup* Group =
			Groups.FindByPredicate(
				[&GroupKey](const FCFContentCatalogGroup& Candidate)
				{
					return Candidate.GroupKey.Equals(
						GroupKey,
						ESearchCase::CaseSensitive);
				});

		if (Group == nullptr)
		{
			// 새 USER-facing family group입니다.
			FCFContentCatalogGroup NewGroup;
			NewGroup.GroupKey = GroupKey;
			NewGroup.DisplayName =
				FString::Printf(
					TEXT("%s · %s"),
					*CFContentCatalogPrivate::GetTypeDisplayName(
						Entry.CatalogType),
					*Entry.FamilyDisplayName);
			Groups.Add(MoveTemp(NewGroup));
			Group = &Groups.Last();
		}

		Group->EntryKeys.Add(Entry.Key);
	}

	Groups.Sort(
		[](const FCFContentCatalogGroup& Left, const FCFContentCatalogGroup& Right)
		{
			return Left.DisplayName < Right.DisplayName;
		});
}

// Canonical workbook record를 generic Presentation entry로 투영합니다.
FCFContentCatalogEntry FCFContentCatalogVM::BuildCanonicalEntry(
	const FCFContentRecord& Record,
	const FCFContentWorkbookModel& Workbook) const
{
	// Record type의 primary sheet descriptor입니다.
	const FCFContentSheetDescriptor* PrimarySheet =
		CFContentCatalogPrivate::FindPrimarySheet(
			Workbook,
			Record.Key.ContentTypeId);

	// USER-facing canonical entry입니다.
	FCFContentCatalogEntry Entry;
	Entry.Key = Record.Key;
	Entry.CatalogType =
		CFContentCatalogPrivate::GetCatalogType(
			Record.Key.ContentTypeId);
	Entry.DisplayName =
		CFContentCatalogPrivate::FindCanonicalDisplayName(
			Record,
			PrimarySheet);
	Entry.SecondaryText =
		CFContentCatalogPrivate::GetTypeDisplayName(
			Entry.CatalogType)
		+ TEXT(" · ")
		+ CFContentCatalogPrivate::GetManagementText(
			Record.ManagementState);
	Entry.FamilyDisplayName =
		Record.AuthoringMetadata.FamilyId.IsEmpty()
			? TEXT("Family 미지정")
			: TEXT("Family 지정됨");
	Entry.VariantDisplayName =
		Record.AuthoringMetadata.BaseContentId.IsEmpty()
			? TEXT("기준 / 독립 Variant")
			: TEXT("파생 Variant");
	Entry.DesignIntent =
		Record.AuthoringMetadata.DesignIntent;

	// Deterministic field key order입니다.
	TArray<FString> FieldKeys;
	Record.Fields.GetKeys(FieldKeys);
	FieldKeys.Sort();

	for (const FString& FieldKey : FieldKeys)
	{
		// Canonical field value입니다.
		const FCFContentValue* Value =
			Record.Fields.Find(FieldKey);
		if (Value == nullptr)
		{
			continue;
		}

		// Schema field descriptor입니다.
		const FCFContentFieldDescriptor* Descriptor =
			CFContentCatalogPrivate::FindFieldDescriptor(
				PrimarySheet,
				FieldKey);

		// USER-facing value row입니다.
		FCFContentCatalogValue Row;
		Row.FieldKey = FieldKey;
		Row.DisplayName =
			Descriptor != nullptr
				&& !Descriptor->DisplayLabel.IsEmpty()
					? Descriptor->DisplayLabel
					: TEXT("콘텐츠 값");
		Row.ValueText =
			CFContentCatalogPrivate::FormatCanonicalValue(
				*Value);
		Row.SourceText =
			CFContentCatalogPrivate::GetCanonicalSourceText(
				*Value);
		Row.UnitText =
			Descriptor != nullptr
				? Descriptor->CanonicalUnitId
				: FString();

		// Numeric canonical value입니다.
		double NumericValue = 0.0;
		if (CFContentCatalogPrivate::TryGetCanonicalNumeric(
			*Value,
			NumericValue))
		{
			Row.bNumeric = true;
			Row.NumericValue = NumericValue;
			if (!Row.UnitText.IsEmpty()
				&& !Row.ValueText.Equals(TEXT("상속"))
				&& !Row.ValueText.Equals(TEXT("없음")))
			{
				Row.ValueText += TEXT(" ")
					+ Row.UnitText;
			}
		}

		Entry.Values.Add(MoveTemp(Row));
	}

	return Entry;
}

// Exact key의 raw entry index를 반환하며 없으면 INDEX_NONE입니다.
int32 FCFContentCatalogVM::FindEntryIndex(
	const FCFContentKey& Key) const
{
	return Entries.IndexOfByPredicate(
		[&Key](const FCFContentCatalogEntry& Candidate)
		{
			return CFContentCatalogPrivate::AreKeysEqual(
				Candidate.Key,
				Key);
		});
}
