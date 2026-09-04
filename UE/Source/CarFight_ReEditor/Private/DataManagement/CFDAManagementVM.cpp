// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAManagementVM.cpp
// Version: v1.6.0
// Date: 2026-09-04
// Description: CF-FQ-045 DAM-P0-04E 새로고침 후 재검사 필요 상태를 구분하는 ViewModel 구현입니다.
// Changelog:
// - v1.6.0: Refresh 시 이전 loaded 결과값은 폐기하되 검사 이력을 session-local 재검사 필요 상태로 보존하고, 현재 검사 완료 시 해당 표시를 해제.
// - v1.5.0: 검사/참조/에셋 열기/콘텐츠 브라우저 및 Registry 준비 오류의 사용자-facing 문구를 한글 우선으로 교정.
// - v1.4.0: Type/Asset sort state와 selected column + canonical identity tie-breaker deterministic sorting을 구현.
// - v1.3.0: management-universe Overview와 user purpose/usage row/search projection을 구현.
// - v1.2.0: bCanonicalNative/abstract/Coverage 기반 presentation state와 Type/Asset view-specific filter를 구현.
// - v1.1.0: PolicyUnavailable/non-success evaluation을 Validate 완료로 오표시하지 않고 hidden Asset selection을 filter/view context와 동기화.
// - v1.0.0: metadata Refresh, generation-bound loaded/reference cache, search/filter/detail/navigation을 구현.
// Migration:
// - Runtime/Content/Product Asset mutation 없음.

#include "DataManagement/CFDAManagementVM.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "ContentBrowserModule.h"
#include "DataManagement/CFDAAuditService.h"
#include "DataManagement/CFDAHealthService.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/SoftObjectPath.h"

namespace CFDAManagementVMPrivate
{
	bool ContainsSearchText(
		const FString& Haystack,
		const FString& SearchText)
	{
		return SearchText.IsEmpty()
			|| Haystack.Contains(SearchText, ESearchCase::IgnoreCase);
	}

	const TCHAR* DomainTechnicalName(const ECFDADomain Domain)
	{
		switch (Domain)
		{
		case ECFDADomain::Vehicle: return TEXT("Vehicle");
		case ECFDADomain::Combat: return TEXT("Combat");
		case ECFDADomain::Targeting: return TEXT("Targeting");
		case ECFDADomain::UI: return TEXT("UI");
		case ECFDADomain::Authoring: return TEXT("Authoring");
		case ECFDADomain::Unclassified:
		default:
			return TEXT("Unclassified");
		}
	}
}

bool FCFDAManagementVM::Initialize(FString& OutError)
{
	if (!EnsureTypeRegistry(OutError))
	{
		return false;
	}
	return Refresh(OutError);
}

bool FCFDAManagementVM::Refresh(FString& OutError)
{
	if (!EnsureTypeRegistry(OutError))
	{
		return false;
	}

	const uint64 NextGeneration =
		FCFDAAuditService::NextInventoryGeneration(Inventory.InventoryGeneration);
	FCFDAInventoryResult NewInventory =
		FCFDAAuditService::Refresh(TypeRegistry, NextGeneration);

	// 현재 generation에서 실제 검사를 수행했던 ObjectPath는 값 자체를 재사용하지 않고 재검사 필요 표시에만 보존합니다.
	for (const TPair<FString, FCFDALoadedAssetResult>& LoadedResultPair : LoadedResultsByObjectPath)
	{
		NeedsRevalidationObjectPaths.Add(LoadedResultPair.Key);
	}

	if (!NewInventory.IsInventoryComplete())
	{
		Inventory = MoveTemp(NewInventory);
		LoadedResultsByObjectPath.Reset();
		ReferenceResultsByObjectPath.Reset();
		SelectedObjectPath.Reset();
		OutError = Inventory.RegistryMessage.IsEmpty()
			? TEXT("에셋 레지스트리의 데이터 목록이 아직 준비되지 않았습니다.")
			: Inventory.RegistryMessage;
		return false;
	}

	Inventory = MoveTemp(NewInventory);

	// 새 snapshot에 더 이상 존재하지 않는 ObjectPath의 재검사 필요 표시는 제거합니다.
	for (TSet<FString>::TIterator RevalidationIterator = NeedsRevalidationObjectPaths.CreateIterator(); RevalidationIterator; ++RevalidationIterator)
	{
		if (!FindAssetRecord(*RevalidationIterator))
		{
			RevalidationIterator.RemoveCurrent();
		}
	}

	// generation이 바뀌면 이전 loaded/reference 결과값은 stale이므로 현재 결과로 재사용하지 않고 즉시 폐기합니다.
	LoadedResultsByObjectPath.Reset();
	ReferenceResultsByObjectPath.Reset();

	// 같은 ObjectPath가 새 snapshot에도 존재하고 current filter에도 보이는 경우에만 selection을 유지합니다.
	if (!SelectedObjectPath.IsEmpty() && !FindAssetRecord(SelectedObjectPath))
	{
		SelectedObjectPath.Reset();
	}
	ReconcileSelectionWithCurrentAssetFilters();

	OutError.Reset();
	return true;
}

void FCFDAManagementVM::SetSearchText(const FString& SearchText)
{
	FilterState.SearchText = SearchText;
	ReconcileSelectionWithCurrentAssetFilters();
}

void FCFDAManagementVM::SetDomainFilter(const TOptional<ECFDADomain>& Domain)
{
	FilterState.Domain = Domain;
	ReconcileSelectionWithCurrentAssetFilters();
}

void FCFDAManagementVM::SetManagementStateFilter(
	const TOptional<ECFDAManagerTypePresentationState>& ManagementState)
{
	FilterState.ManagementState = ManagementState;
}

void FCFDAManagementVM::SetScopeFilter(const TOptional<ECFDAScope>& Scope)
{
	FilterState.Scope = Scope;
	ReconcileSelectionWithCurrentAssetFilters();
}

void FCFDAManagementVM::SetHealthFilter(const TOptional<ECFDAHealthState>& Health)
{
	FilterState.Health = Health;
	ReconcileSelectionWithCurrentAssetFilters();
}

void FCFDAManagementVM::SetShowAllDiscovered(const bool bShowAllDiscovered)
{
	FilterState.bShowAllDiscovered = bShowAllDiscovered;
	ReconcileSelectionWithCurrentAssetFilters();
}

void FCFDAManagementVM::SetViewMode(const ECFDAManagerViewMode NewViewMode)
{
	ViewMode = NewViewMode;
	if (ViewMode == ECFDAManagerViewMode::TypeView)
	{
		ClearSelection();
	}
}

void FCFDAManagementVM::SetTypeSort(
	const ECFDAManagerTypeSortKey SortKey,
	const ECFDAManagerSortDirection SortDirection)
{
	TypeSortKey = SortKey;
	TypeSortDirection = SortDirection;
}

void FCFDAManagementVM::SetAssetSort(
	const ECFDAManagerAssetSortKey SortKey,
	const ECFDAManagerSortDirection SortDirection)
{
	AssetSortKey = SortKey;
	AssetSortDirection = SortDirection;
}

void FCFDAManagementVM::SortTypeRows(
	TArray<FCFDAManagerTypeRow>& Rows,
	const ECFDAManagerTypeSortKey SortKey,
	const ECFDAManagerSortDirection SortDirection)
{
	Rows.Sort([SortKey, SortDirection](
		const FCFDAManagerTypeRow& Left,
		const FCFDAManagerTypeRow& Right)
	{
		int32 PrimaryCompare = 0;
		switch (SortKey)
		{
		case ECFDAManagerTypeSortKey::Domain:
			PrimaryCompare = static_cast<int32>(Left.Domain) - static_cast<int32>(Right.Domain);
			break;
		case ECFDAManagerTypeSortKey::AssetCount:
			PrimaryCompare = Left.AssetCount - Right.AssetCount;
			break;
		case ECFDAManagerTypeSortKey::ManagementState:
			PrimaryCompare = static_cast<int32>(Left.PresentationState) - static_cast<int32>(Right.PresentationState);
			break;
		case ECFDAManagerTypeSortKey::DisplayName:
		default:
			PrimaryCompare = Left.DisplayName.Compare(Right.DisplayName, ESearchCase::IgnoreCase);
			break;
		}

		if (PrimaryCompare != 0)
		{
			return SortDirection == ECFDAManagerSortDirection::Ascending
				? PrimaryCompare < 0
				: PrimaryCompare > 0;
		}

		return Left.ClassPath.Compare(Right.ClassPath, ESearchCase::CaseSensitive) < 0;
	});
}

void FCFDAManagementVM::SortAssetRows(
	TArray<FCFDAManagerAssetRow>& Rows,
	const ECFDAManagerAssetSortKey SortKey,
	const ECFDAManagerSortDirection SortDirection)
{
	Rows.Sort([SortKey, SortDirection](
		const FCFDAManagerAssetRow& Left,
		const FCFDAManagerAssetRow& Right)
	{
		int32 PrimaryCompare = 0;
		switch (SortKey)
		{
		case ECFDAManagerAssetSortKey::TypeDisplayName:
			PrimaryCompare = Left.TypeDisplayName.Compare(Right.TypeDisplayName, ESearchCase::IgnoreCase);
			break;
		case ECFDAManagerAssetSortKey::Domain:
			PrimaryCompare = static_cast<int32>(Left.Domain) - static_cast<int32>(Right.Domain);
			break;
		case ECFDAManagerAssetSortKey::Scope:
			PrimaryCompare = static_cast<int32>(Left.Scope) - static_cast<int32>(Right.Scope);
			break;
		case ECFDAManagerAssetSortKey::Health:
			PrimaryCompare = static_cast<int32>(Left.HealthState) - static_cast<int32>(Right.HealthState);
			break;
		case ECFDAManagerAssetSortKey::AssetName:
		default:
			PrimaryCompare = Left.AssetName.Compare(Right.AssetName, ESearchCase::IgnoreCase);
			break;
		}

		if (PrimaryCompare != 0)
		{
			return SortDirection == ECFDAManagerSortDirection::Ascending
				? PrimaryCompare < 0
				: PrimaryCompare > 0;
		}

		return Left.ObjectPath.Compare(Right.ObjectPath, ESearchCase::CaseSensitive) < 0;
	});
}

FCFDAManagerOverview FCFDAManagementVM::BuildOverview() const
{
	FCFDAManagerOverview Overview;
	Overview.TypeCount = Inventory.TypeRecords.Num();
	Overview.AssetCount = Inventory.AssetRecords.Num();

	for (const FCFDATypeRecord& TypeRecord : Inventory.TypeRecords)
	{
		if (TypeRecord.CoverageState == ECFDACoverageState::Registered)
		{
			++Overview.RegisteredTypeCount;
		}
		else
		{
			++Overview.UnregisteredTypeCount;
		}

		if (TypeRecord.TypeInstanceState == ECFDATypeInstanceState::NoAssetInstance)
		{
			++Overview.ZeroInstanceTypeCount;
		}

		if (ResolvePresentationState(TypeRecord) == ECFDAManagerTypePresentationState::NeedsManagementRule)
		{
			++Overview.NeedsManagementRuleTypeCount;
		}
	}

	for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
	{
		const FCFDAManagerAssetRow AssetRow = BuildAssetRow(AssetRecord);
		if (IsManagementUniverseState(AssetRow.PresentationState))
		{
			++Overview.ManagedAssetCount;
			if (AssetRow.HealthState == ECFDAHealthState::Error || AssetRow.HealthState == ECFDAHealthState::Warning)
			{
				++Overview.ProblemAssetCount;
			}
			if (AssetRow.HealthState == ECFDAHealthState::NotValidated)
			{
				++Overview.ManagedNotValidatedAssetCount;
			}
		}

		switch (AssetRow.HealthState)
		{
		case ECFDAHealthState::Error:
			++Overview.ErrorAssetCount;
			break;
		case ECFDAHealthState::Warning:
			++Overview.WarningAssetCount;
			break;
		case ECFDAHealthState::NotValidated:
			++Overview.NotValidatedAssetCount;
			break;
		default:
			break;
		}
	}

	return Overview;
}

TArray<FCFDAManagerTypeRow> FCFDAManagementVM::BuildFilteredTypeRows() const
{
	TArray<FCFDAManagerTypeRow> Rows;
	for (const FCFDATypeRecord& TypeRecord : Inventory.TypeRecords)
	{
		const FCFDAResolvedSemantic Semantic =
			TypeRegistry.ResolveSemantic(
				TypeRecord.ClassPath,
				TypeRecord.TechnicalClassName);

		FCFDAManagerTypeRow Row;
		Row.ClassPath = TypeRecord.ClassPath;
		Row.TechnicalClassName = TypeRecord.TechnicalClassName;
		Row.DisplayName = Semantic.Descriptor.TypeDisplayName.IsEmpty()
			? TypeRecord.TechnicalClassName
			: Semantic.Descriptor.TypeDisplayName;
		Row.UserPurposeDescription = Semantic.Descriptor.UserPurposeDescription;
		Row.UserUsageDescription = Semantic.Descriptor.UserUsageDescription;
		Row.RoleDescription = Semantic.Descriptor.RoleDescription;
		Row.Domain = Semantic.Descriptor.Domain;
		Row.CoverageState = Semantic.CoverageState;
		Row.PresentationState = ResolvePresentationState(TypeRecord);
		Row.TypeInstanceState = TypeRecord.TypeInstanceState;
		Row.bCanonicalNative = TypeRecord.bCanonicalNative;
		Row.bAbstract = TypeRecord.bAbstract;
		Row.AssetCount = TypeRecord.AssetCount;

		if (!TypeMatchesCurrentFilters(Row))
		{
			continue;
		}

		if (!MatchesTypeSearch(Row))
		{
			continue;
		}

		Rows.Add(MoveTemp(Row));
	}

	SortTypeRows(Rows, TypeSortKey, TypeSortDirection);
	return Rows;
}

TArray<FCFDAManagerAssetRow> FCFDAManagementVM::BuildFilteredAssetRows() const
{
	TArray<FCFDAManagerAssetRow> Rows;
	for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
	{
		FCFDAManagerAssetRow Row = BuildAssetRow(AssetRecord);
		if (!AssetMatchesCurrentFilters(Row))
		{
			continue;
		}
		Rows.Add(MoveTemp(Row));
	}

	SortAssetRows(Rows, AssetSortKey, AssetSortDirection);
	return Rows;
}

TArray<ECFDADomain> FCFDAManagementVM::GetAvailableDomains() const
{
	TSet<ECFDADomain> PresentDomains;
	for (const FCFDATypeRecord& TypeRecord : Inventory.TypeRecords)
	{
		const ECFDAManagerTypePresentationState PresentationState = ResolvePresentationState(TypeRecord);
		if (!FilterState.bShowAllDiscovered && !IsManagementUniverseState(PresentationState))
		{
			continue;
		}

		const FCFDAResolvedSemantic Semantic =
			TypeRegistry.ResolveSemantic(
				TypeRecord.ClassPath,
				TypeRecord.TechnicalClassName);
		PresentDomains.Add(Semantic.Descriptor.Domain);
	}

	const ECFDADomain CanonicalOrder[] =
	{
		ECFDADomain::Vehicle,
		ECFDADomain::Combat,
		ECFDADomain::Targeting,
		ECFDADomain::UI,
		ECFDADomain::Authoring,
		ECFDADomain::Unclassified
	};

	TArray<ECFDADomain> Domains;
	for (const ECFDADomain Domain : CanonicalOrder)
	{
		if (PresentDomains.Contains(Domain))
		{
			Domains.Add(Domain);
		}
	}
	return Domains;
}

bool FCFDAManagementVM::SelectAsset(const FString& ObjectPath)
{
	const FCFDAAssetRecord* AssetRecord = FindAssetRecord(ObjectPath);
	if (!AssetRecord || !AssetMatchesCurrentFilters(BuildAssetRow(*AssetRecord)))
	{
		return false;
	}
	SelectedObjectPath = ObjectPath;
	return true;
}

void FCFDAManagementVM::ClearSelection()
{
	SelectedObjectPath.Reset();
}

bool FCFDAManagementVM::IsConclusiveValidationResult(
	const FCFDALoadedAssetResult& Result,
	FString& OutMessage)
{
	if (Result.EvaluationState == ECFDAEvaluationState::Succeeded)
	{
		OutMessage.Reset();
		return true;
	}

	switch (Result.EvaluationState)
	{
	case ECFDAEvaluationState::PolicyUnavailable:
		OutMessage = TEXT("이 DataAsset 타입은 아직 관리 검증 정책이 등록되지 않아 검사를 완료하지 않았습니다.");
		return false;
	case ECFDAEvaluationState::InvalidRequest:
		OutMessage = TEXT("현재 데이터 목록 상태에서는 이 DataAsset 검사를 실행할 수 없습니다.");
		return false;
	case ECFDAEvaluationState::LoadFailed:
		OutMessage = TEXT("DataAsset을 열지 못해 검사를 완료하지 않았습니다.");
		return false;
	case ECFDAEvaluationState::ClassMismatch:
		OutMessage = TEXT("목록에 기록된 타입과 실제 DataAsset 타입이 달라 검사를 완료하지 않았습니다.");
		return false;
	case ECFDAEvaluationState::AdapterUnavailable:
		OutMessage = TEXT("이 DataAsset 타입의 검사 기능을 현재 사용할 수 없습니다.");
		return false;
	case ECFDAEvaluationState::NotRequested:
	default:
		if (!Result.Messages.IsEmpty())
		{
			OutMessage = Result.Messages[0];
		}
		else
		{
			OutMessage = TEXT("DataAsset 검사가 아직 완료되지 않았습니다.");
		}
		return false;
	}
}

bool FCFDAManagementVM::ValidateSelectedAsset(FString& OutError)
{
	if (SelectedObjectPath.IsEmpty())
	{
		OutError = TEXT("검사할 데이터 에셋을 먼저 선택하세요.");
		return false;
	}

	const FString ValidatedObjectPath = SelectedObjectPath;
	const FCFDALoadedAssetResult Result =
		FCFDAHealthService::ValidateAsset(
			Inventory,
			ValidatedObjectPath,
			TypeRegistry);
	LoadedResultsByObjectPath.Add(ValidatedObjectPath, Result);
	NeedsRevalidationObjectPaths.Remove(ValidatedObjectPath);

	// Validate 결과가 current Health filter에서 row를 숨기게 만들 수 있으므로 action context도 즉시 재조정합니다.
	ReconcileSelectionWithCurrentAssetFilters();

	// DA Health가 Error여도 evaluation 자체가 성공했다면 Validate action은 정상 완료입니다.
	// 반대로 PolicyUnavailable/operational failure는 Health를 NotValidated로 보존하고 완료로 오표시하지 않습니다.
	return IsConclusiveValidationResult(Result, OutError);
}

bool FCFDAManagementVM::QuerySelectedReferences(FString& OutError)
{
	if (SelectedObjectPath.IsEmpty())
	{
		OutError = TEXT("참조 관계를 조회할 데이터 에셋을 먼저 선택하세요.");
		return false;
	}

	const FCFDAReferenceResult Result =
		FCFDAReferenceService::Query(
			Inventory,
			SelectedObjectPath);
	ReferenceResultsByObjectPath.Add(SelectedObjectPath, Result);

	if (Result.QueryState != ECFDAReferenceQueryState::Succeeded)
	{
		OutError = Result.Message.IsEmpty()
			? TEXT("참조 대상 / 사용처 조회를 완료하지 못했습니다.")
			: Result.Message;
		return false;
	}

	OutError.Reset();
	return true;
}

bool FCFDAManagementVM::OpenSelectedAsset(FString& OutError) const
{
	const FCFDAAssetRecord* AssetRecord = GetSelectedAssetRecord();
	if (!AssetRecord)
	{
		OutError = TEXT("열 데이터 에셋을 먼저 선택하세요.");
		return false;
	}

	if (!GEditor)
	{
		OutError = TEXT("언리얼 에디터 기능이 준비되지 않았습니다.");
		return false;
	}

	const FSoftObjectPath ObjectPath(AssetRecord->ObjectPath);
	UObject* Asset = ObjectPath.TryLoad();
	if (!Asset)
	{
		OutError = TEXT("선택한 데이터 에셋을 불러오지 못했습니다.");
		return false;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem =
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		OutError = TEXT("에셋 편집기 기능을 사용할 수 없습니다.");
		return false;
	}

	AssetEditorSubsystem->OpenEditorForAsset(Asset);
	OutError.Reset();
	return true;
}

bool FCFDAManagementVM::SyncSelectedAssetToContentBrowser(FString& OutError) const
{
	const FCFDAAssetRecord* AssetRecord = GetSelectedAssetRecord();
	if (!AssetRecord)
	{
		OutError = TEXT("콘텐츠 브라우저에서 찾을 데이터 에셋을 먼저 선택하세요.");
		return false;
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetData AssetData =
		AssetRegistryModule.Get().GetAssetByObjectPath(
			FSoftObjectPath(AssetRecord->ObjectPath),
			false,
			false);
	if (!AssetData.IsValid())
	{
		OutError = TEXT("선택한 데이터 에셋의 에셋 레지스트리 정보를 찾지 못했습니다.");
		return false;
	}

	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FAssetData> AssetsToSync;
	AssetsToSync.Add(AssetData);
	ContentBrowserModule.Get().SyncBrowserToAssets(
		AssetsToSync,
		false,
		true,
		NAME_None,
		false);

	OutError.Reset();
	return true;
}

const FCFDAAssetRecord* FCFDAManagementVM::GetSelectedAssetRecord() const
{
	return FindAssetRecord(SelectedObjectPath);
}

const FCFDALoadedAssetResult* FCFDAManagementVM::GetSelectedLoadedResult() const
{
	return FindLoadedResult(SelectedObjectPath);
}

const FCFDAReferenceResult* FCFDAManagementVM::GetSelectedReferenceResult() const
{
	return FindReferenceResult(SelectedObjectPath);
}

bool FCFDAManagementVM::EnsureTypeRegistry(FString& OutError)
{
	if (bTypeRegistryReady)
	{
		OutError.Reset();
		return true;
	}

	FString RegistrationError;
	if (!TypeRegistry.RegisterCurrentCarFightDescriptors(&RegistrationError))
	{
		OutError = RegistrationError.IsEmpty()
			? TEXT("현재 CarFight 데이터 에셋의 설명 정보를 준비하지 못했습니다.")
			: RegistrationError;
		return false;
	}

	bTypeRegistryReady = true;
	OutError.Reset();
	return true;
}

const FCFDAAssetRecord* FCFDAManagementVM::FindAssetRecord(
	const FString& ObjectPath) const
{
	for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
	{
		if (AssetRecord.ObjectPath == ObjectPath)
		{
			return &AssetRecord;
		}
	}
	return nullptr;
}

const FCFDATypeRecord* FCFDAManagementVM::FindTypeRecord(
	const FString& ClassPath) const
{
	for (const FCFDATypeRecord& TypeRecord : Inventory.TypeRecords)
	{
		if (TypeRecord.ClassPath == ClassPath)
		{
			return &TypeRecord;
		}
	}
	return nullptr;
}

ECFDAManagerTypePresentationState FCFDAManagementVM::ResolvePresentationState(
	const FCFDATypeRecord& TypeRecord)
{
	if (TypeRecord.CoverageState == ECFDACoverageState::Registered)
	{
		return ECFDAManagerTypePresentationState::Managed;
	}
	if (!TypeRecord.bCanonicalNative)
	{
		return ECFDAManagerTypePresentationState::AuxiliaryDataAsset;
	}
	if (TypeRecord.bAbstract)
	{
		return ECFDAManagerTypePresentationState::Framework;
	}
	return ECFDAManagerTypePresentationState::NeedsManagementRule;
}

bool FCFDAManagementVM::IsManagementUniverseState(
	const ECFDAManagerTypePresentationState PresentationState)
{
	return PresentationState == ECFDAManagerTypePresentationState::Managed
		|| PresentationState == ECFDAManagerTypePresentationState::NeedsManagementRule;
}

const FCFDALoadedAssetResult* FCFDAManagementVM::FindLoadedResult(
	const FString& ObjectPath) const
{
	const FCFDALoadedAssetResult* Result =
		LoadedResultsByObjectPath.Find(ObjectPath);
	if (!Result || !Result->IsFreshForInventoryGeneration(Inventory.InventoryGeneration))
	{
		return nullptr;
	}
	return Result;
}

const FCFDAReferenceResult* FCFDAManagementVM::FindReferenceResult(
	const FString& ObjectPath) const
{
	const FCFDAReferenceResult* Result =
		ReferenceResultsByObjectPath.Find(ObjectPath);
	if (!Result || !Result->IsFreshForInventoryGeneration(Inventory.InventoryGeneration))
	{
		return nullptr;
	}
	return Result;
}

FCFDAManagerAssetRow FCFDAManagementVM::BuildAssetRow(
	const FCFDAAssetRecord& AssetRecord) const
{
	const FCFDAResolvedSemantic Semantic =
		TypeRegistry.ResolveSemantic(
			AssetRecord.ClassPath,
			AssetRecord.ClassPath);

	FCFDAManagerAssetRow Row;
	Row.AssetName = AssetRecord.AssetName;
	Row.ObjectPath = AssetRecord.ObjectPath;
	Row.PackagePath = AssetRecord.PackagePath;
	Row.ClassPath = AssetRecord.ClassPath;
	Row.TypeDisplayName = Semantic.Descriptor.TypeDisplayName.IsEmpty()
		? AssetRecord.ClassPath
		: Semantic.Descriptor.TypeDisplayName;
	Row.UserPurposeDescription = Semantic.Descriptor.UserPurposeDescription;
	Row.UserUsageDescription = Semantic.Descriptor.UserUsageDescription;
	Row.RoleDescription = Semantic.Descriptor.RoleDescription;
	Row.Domain = Semantic.Descriptor.Domain;
	Row.Scope = AssetRecord.Scope;
	Row.CoverageState = Semantic.CoverageState;
	if (const FCFDATypeRecord* TypeRecord = FindTypeRecord(AssetRecord.ClassPath))
	{
		Row.PresentationState = ResolvePresentationState(*TypeRecord);
	}
	Row.HealthState = AssetRecord.HealthState;
	Row.StableIdState = AssetRecord.StableIdState;
	Row.bNeedsRevalidation = NeedsRevalidationObjectPaths.Contains(AssetRecord.ObjectPath);

	if (const FCFDALoadedAssetResult* LoadedResult =
		FindLoadedResult(AssetRecord.ObjectPath))
	{
		Row.bNeedsRevalidation = false;
		Row.HealthState = LoadedResult->HealthState;
		Row.StableIdState = LoadedResult->StableIdState;
		Row.EvaluationState = LoadedResult->EvaluationState;
		Row.DuplicateState = LoadedResult->DuplicateState;
		Row.StableId = LoadedResult->StableId;
	}
	return Row;
}

bool FCFDAManagementVM::AssetMatchesCurrentFilters(
	const FCFDAManagerAssetRow& Row) const
{
	if (!FilterState.bShowAllDiscovered && !IsManagementUniverseState(Row.PresentationState))
	{
		return false;
	}
	if (FilterState.Domain.IsSet() && Row.Domain != FilterState.Domain.GetValue())
	{
		return false;
	}
	if (FilterState.Scope.IsSet() && Row.Scope != FilterState.Scope.GetValue())
	{
		return false;
	}
	if (FilterState.Health.IsSet() && Row.HealthState != FilterState.Health.GetValue())
	{
		return false;
	}
	return MatchesAssetSearch(Row);
}

void FCFDAManagementVM::ReconcileSelectionWithCurrentAssetFilters()
{
	if (SelectedObjectPath.IsEmpty())
	{
		return;
	}

	const FCFDAAssetRecord* SelectedAssetRecord = FindAssetRecord(SelectedObjectPath);
	if (!SelectedAssetRecord || !AssetMatchesCurrentFilters(BuildAssetRow(*SelectedAssetRecord)))
	{
		ClearSelection();
	}
}

bool FCFDAManagementVM::TypeMatchesCurrentFilters(
	const FCFDAManagerTypeRow& Row) const
{
	if (!FilterState.bShowAllDiscovered && !IsManagementUniverseState(Row.PresentationState))
	{
		return false;
	}
	if (FilterState.Domain.IsSet() && Row.Domain != FilterState.Domain.GetValue())
	{
		return false;
	}
	if (FilterState.ManagementState.IsSet()
		&& Row.PresentationState != FilterState.ManagementState.GetValue())
	{
		return false;
	}
	return true;
}

bool FCFDAManagementVM::MatchesTypeSearch(
	const FCFDAManagerTypeRow& Row) const
{
	const FString& SearchText = FilterState.SearchText;
	return SearchText.IsEmpty()
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.DisplayName, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.TechnicalClassName, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.ClassPath, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.UserPurposeDescription, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.UserUsageDescription, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.RoleDescription, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(
			CFDAManagementVMPrivate::DomainTechnicalName(Row.Domain),
			SearchText);
}

bool FCFDAManagementVM::MatchesAssetSearch(
	const FCFDAManagerAssetRow& Row) const
{
	const FString& SearchText = FilterState.SearchText;
	return SearchText.IsEmpty()
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.AssetName, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.ObjectPath, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.TypeDisplayName, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.ClassPath, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.UserPurposeDescription, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.UserUsageDescription, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(Row.RoleDescription, SearchText)
		|| CFDAManagementVMPrivate::ContainsSearchText(
			CFDAManagementVMPrivate::DomainTechnicalName(Row.Domain),
			SearchText);
}
