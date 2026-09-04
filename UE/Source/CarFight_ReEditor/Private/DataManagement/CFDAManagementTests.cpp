// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAManagementTests.cpp
// Version: v1.6.0
// Date: 2026-09-04
// Description: CF-FQ-045 DAM-P0-03~04E 사용자 정보 Manager Automation입니다.
// Changelog:
// - v1.6.0: P0-04E Refresh 후 재검사/재확인 필요 상태, 재검사 완료 시 stale 표시 해제, 직접 에셋 생성 가능 여부 표현 회귀를 추가.
// - v1.5.0: P0-04E feedback correction의 기술 정보/참조 관계 한글 우선 표현과 영어 상태 레이블 비노출 회귀를 추가.
// - v1.4.0: P0-04D Stable ID 사용자 상태, Referencer 삭제 안전 경고, InventoryGeneration 비노출, 정렬 후 Slate selection/Detail 동기화 회귀를 추가.
// - v1.3.0: Multi-column presentation의 management universe, presentation state, view-specific filter 회귀를 추가.
// - v1.2.0: Asset Registry query failure와 successful-empty Reference evidence 구분 회귀를 추가.
// - v1.1.0: PolicyUnavailable non-conclusive Validate와 filter/view hidden selection cleanup 회귀를 추가.
// - v1.0.0: VM inventory/filter, cache lifecycle/reference, Nomad Tab construction을 검증.
// Migration:
// - Product Asset mutation/save 없음. navigation action은 Automation에서 실행하지 않습니다.

#include "DataManagement/CFDAManagementTab.h"
#include "DataManagement/CFDAManagementVM.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Framework/Docking/TabManager.h"
#include "Misc/AutomationTest.h"
#include "Widgets/Docking/SDockTab.h"

namespace CFDAManagementTestsPrivate
{
	const FCFDAAssetRecord* FindFirstAssetByClass(
		const FCFDAInventoryResult& Inventory,
		const FString& ClassPath)
	{
		for (const FCFDAAssetRecord& AssetRecord : Inventory.AssetRecords)
		{
			if (AssetRecord.ClassPath == ClassPath)
			{
				return &AssetRecord;
			}
		}
		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAManagerInventoryFilterTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_03.InventoryFilter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAManagerCacheReferenceTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_03.CacheReference",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAManagerTabTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_03.TabSurface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAManagerTablePresentationTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_04B.TablePresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAManagerInformationHierarchyTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_04C.InformationHierarchy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAManagerP004DCorrectionTest,
	"CarFight.DataManagement.CF_FQ_045.DAM_P0_04D.PresentationCorrection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFDAManagerInventoryFilterTest::RunTest(const FString& Parameters)
{
	FCFDAManagementVM ViewModel;
	FString Error;
	if (!TestTrue(TEXT("Manager VM initializes"), ViewModel.Initialize(Error)))
	{
		AddError(Error);
		return false;
	}

	TestTrue(TEXT("Manager inventory is complete"), ViewModel.GetInventory().IsInventoryComplete());
	TestTrue(TEXT("Manager generation is non-zero"), ViewModel.GetInventoryGeneration() > 0);

	const FCFDAManagerOverview Overview = ViewModel.BuildOverview();
	TestTrue(TEXT("Current type baseline is present"), Overview.TypeCount >= 28);
	TestTrue(TEXT("Current asset baseline is present"), Overview.AssetCount >= 53);
	TestTrue(TEXT("Registered current concrete types are present"), Overview.RegisteredTypeCount >= 27);
	TestTrue(TEXT("At least one zero-instance type is visible"), Overview.ZeroInstanceTypeCount >= 1);
	TestEqual(TEXT("Fresh metadata inventory starts NotValidated"), Overview.NotValidatedAssetCount, Overview.AssetCount);

	const TArray<ECFDADomain> Domains = ViewModel.GetAvailableDomains();
	TestTrue(TEXT("Vehicle domain is available"), Domains.Contains(ECFDADomain::Vehicle));
	TestTrue(TEXT("Combat domain is available"), Domains.Contains(ECFDADomain::Combat));
	TestTrue(TEXT("Targeting domain is available"), Domains.Contains(ECFDADomain::Targeting));
	TestTrue(TEXT("UI domain is available"), Domains.Contains(ECFDADomain::UI));
	TestTrue(TEXT("Authoring domain is available"), Domains.Contains(ECFDADomain::Authoring));

	ViewModel.SetDomainFilter(TOptional<ECFDADomain>(ECFDADomain::Vehicle));
	const TArray<FCFDAManagerTypeRow> VehicleTypes = ViewModel.BuildFilteredTypeRows();
	TestTrue(TEXT("Vehicle domain filter returns type rows"), !VehicleTypes.IsEmpty());
	for (const FCFDAManagerTypeRow& Row : VehicleTypes)
	{
		TestEqual(TEXT("Vehicle type row respects Domain filter"), Row.Domain, ECFDADomain::Vehicle);
	}

	ViewModel.SetDomainFilter(TOptional<ECFDADomain>());
	ViewModel.SetSearchText(TEXT("Vehicle"));
	const TArray<FCFDAManagerTypeRow> SearchTypes = ViewModel.BuildFilteredTypeRows();
	const TArray<FCFDAManagerAssetRow> SearchAssets = ViewModel.BuildFilteredAssetRows();
	TestTrue(TEXT("Search finds type rows"), !SearchTypes.IsEmpty());
	TestTrue(TEXT("Search finds asset rows"), !SearchAssets.IsEmpty());

	ViewModel.SetSearchText(TEXT(""));
	const TArray<FCFDAManagerAssetRow> DefaultManagedRows = ViewModel.BuildFilteredAssetRows();
	TestTrue(TEXT("Default management universe contains assets"), !DefaultManagedRows.IsEmpty());
	TestTrue(TEXT("Default management universe does not exceed full inventory"), DefaultManagedRows.Num() <= Overview.AssetCount);

	ViewModel.SetHealthFilter(TOptional<ECFDAHealthState>(ECFDAHealthState::NotValidated));
	const TArray<FCFDAManagerAssetRow> NotValidatedRows = ViewModel.BuildFilteredAssetRows();
	TestEqual(TEXT("Initial Health filter sees all default management-universe metadata assets"), NotValidatedRows.Num(), DefaultManagedRows.Num());

	// filter가 selected Asset을 숨기면 Detail/action 대상 selection도 같이 해제되어야 합니다.
	if (!NotValidatedRows.IsEmpty())
	{
		const FString SelectedPath = NotValidatedRows[0].ObjectPath;
		TestTrue(TEXT("Visible asset can be selected before filter cleanup"), ViewModel.SelectAsset(SelectedPath));
		TestFalse(TEXT("Selection exists before filter cleanup"), ViewModel.GetSelectedObjectPath().IsEmpty());
		ViewModel.SetSearchText(TEXT("__DAM_NO_MATCH_SELECTION_TEST__"));
		TestTrue(TEXT("Hidden asset selection is cleared by search filter"), ViewModel.GetSelectedObjectPath().IsEmpty());

		ViewModel.SetSearchText(TEXT(""));
		TestTrue(TEXT("Asset can be selected again after search reset"), ViewModel.SelectAsset(SelectedPath));
		ViewModel.SetViewMode(ECFDAManagerViewMode::AssetView);
		TestFalse(TEXT("Asset View keeps explicit asset selection"), ViewModel.GetSelectedObjectPath().IsEmpty());
		ViewModel.SetViewMode(ECFDAManagerViewMode::TypeView);
		TestTrue(TEXT("Type View clears previous asset action context"), ViewModel.GetSelectedObjectPath().IsEmpty());
	}

	// Unregistered/PolicyUnavailable는 Health Error가 아니며 Validate 완료로 표현할 수 없는 non-conclusive 결과입니다.
	// Health filter가 NotValidated인 상태에서 Validate가 conclusive Health로 바뀌면 selection도 hidden row와 함께 해제되어야 합니다.
	const FCFDAAssetRecord* CatalogForFilterCleanup =
		CFDAManagementTestsPrivate::FindFirstAssetByClass(
			ViewModel.GetInventory(),
			TEXT("/Script/CarFight_Re.CFRuntimeTestCatalogData"));
	TestNotNull(TEXT("Catalog exists for validation-driven selection cleanup"), CatalogForFilterCleanup);
	if (CatalogForFilterCleanup)
	{
		TestTrue(TEXT("Catalog is selectable under NotValidated filter before validation"), ViewModel.SelectAsset(CatalogForFilterCleanup->ObjectPath));
		FString ValidationActionMessage;
		TestTrue(TEXT("Catalog validation completes for filter cleanup"), ViewModel.ValidateSelectedAsset(ValidationActionMessage));
		TestTrue(TEXT("Validation-driven hidden row clears asset selection"), ViewModel.GetSelectedObjectPath().IsEmpty());
	}

	FCFDALoadedAssetResult PolicyUnavailableResult;
	PolicyUnavailableResult.EvaluationState = ECFDAEvaluationState::PolicyUnavailable;
	PolicyUnavailableResult.HealthState = ECFDAHealthState::NotValidated;
	PolicyUnavailableResult.Messages.Add(TEXT("Typed Semantic descriptor가 등록되지 않았습니다."));
	FString ValidationMessage;
	TestFalse(
		TEXT("PolicyUnavailable validation is not conclusive"),
		FCFDAManagementVM::IsConclusiveValidationResult(PolicyUnavailableResult, ValidationMessage));
	TestFalse(TEXT("PolicyUnavailable provides user-facing explanation"), ValidationMessage.IsEmpty());

	FCFDALoadedAssetResult InvalidDataResult;
	InvalidDataResult.EvaluationState = ECFDAEvaluationState::Succeeded;
	InvalidDataResult.HealthState = ECFDAHealthState::Error;
	TestTrue(
		TEXT("Succeeded evaluation remains conclusive even when DA Health is Error"),
		FCFDAManagementVM::IsConclusiveValidationResult(InvalidDataResult, ValidationMessage));
	return true;
}

bool FCFDAManagerTablePresentationTest::RunTest(const FString& Parameters)
{
	FCFDATypeRecord RegisteredType;
	RegisteredType.CoverageState = ECFDACoverageState::Registered;
	TestEqual(
		TEXT("Registered type is Managed regardless of discovery provenance"),
		FCFDAManagementVM::ResolvePresentationState(RegisteredType),
		ECFDAManagerTypePresentationState::Managed);

	FCFDATypeRecord NativeConcreteType;
	NativeConcreteType.CoverageState = ECFDACoverageState::Unregistered;
	NativeConcreteType.bCanonicalNative = true;
	NativeConcreteType.bAbstract = false;
	TestEqual(
		TEXT("Unregistered canonical native concrete type needs management rule"),
		FCFDAManagementVM::ResolvePresentationState(NativeConcreteType),
		ECFDAManagerTypePresentationState::NeedsManagementRule);

	FCFDATypeRecord NativeAbstractType = NativeConcreteType;
	NativeAbstractType.bAbstract = true;
	TestEqual(
		TEXT("Unregistered canonical native abstract type is Framework"),
		FCFDAManagementVM::ResolvePresentationState(NativeAbstractType),
		ECFDAManagerTypePresentationState::Framework);

	FCFDATypeRecord ExternalAbstractType = NativeAbstractType;
	ExternalAbstractType.bCanonicalNative = false;
	TestEqual(
		TEXT("Non-CarFight type stays Auxiliary even when abstract"),
		FCFDAManagementVM::ResolvePresentationState(ExternalAbstractType),
		ECFDAManagerTypePresentationState::AuxiliaryDataAsset);

	TestTrue(
		TEXT("Managed is in management universe"),
		FCFDAManagementVM::IsManagementUniverseState(ECFDAManagerTypePresentationState::Managed));
	TestTrue(
		TEXT("NeedsManagementRule is in management universe"),
		FCFDAManagementVM::IsManagementUniverseState(ECFDAManagerTypePresentationState::NeedsManagementRule));
	TestFalse(
		TEXT("Framework is outside default management universe"),
		FCFDAManagementVM::IsManagementUniverseState(ECFDAManagerTypePresentationState::Framework));
	TestFalse(
		TEXT("AuxiliaryDataAsset is outside default management universe"),
		FCFDAManagementVM::IsManagementUniverseState(ECFDAManagerTypePresentationState::AuxiliaryDataAsset));

	FCFDAManagementVM ViewModel;
	FString Error;
	if (!TestTrue(TEXT("Manager VM initializes for table presentation"), ViewModel.Initialize(Error)))
	{
		AddError(Error);
		return false;
	}

	const TArray<FCFDAManagerTypeRow> DefaultTypes = ViewModel.BuildFilteredTypeRows();
	const TArray<FCFDAManagerAssetRow> DefaultAssets = ViewModel.BuildFilteredAssetRows();
	TestTrue(TEXT("Default Type table has managed rows"), !DefaultTypes.IsEmpty());
	TestTrue(TEXT("Default Asset table has managed rows"), !DefaultAssets.IsEmpty());
	for (const FCFDAManagerTypeRow& Row : DefaultTypes)
	{
		TestTrue(
			TEXT("Default Type table excludes Framework/Auxiliary"),
			FCFDAManagementVM::IsManagementUniverseState(Row.PresentationState));
	}
	for (const FCFDAManagerAssetRow& Row : DefaultAssets)
	{
		TestTrue(
			TEXT("Default Asset table excludes Framework/Auxiliary"),
			FCFDAManagementVM::IsManagementUniverseState(Row.PresentationState));
	}

	// Asset-only Scope/Health filter must not change Type View rows.
	ViewModel.SetScopeFilter(TOptional<ECFDAScope>(ECFDAScope::Test));
	ViewModel.SetHealthFilter(TOptional<ECFDAHealthState>(ECFDAHealthState::NotValidated));
	const TArray<FCFDAManagerTypeRow> TypesWithAssetFilters = ViewModel.BuildFilteredTypeRows();
	TestEqual(TEXT("Type View ignores Asset-only Scope/Health filter count"), TypesWithAssetFilters.Num(), DefaultTypes.Num());
	for (int32 Index = 0; Index < DefaultTypes.Num() && Index < TypesWithAssetFilters.Num(); ++Index)
	{
		TestEqual(TEXT("Type View ignores Asset-only Scope/Health filter identity"), TypesWithAssetFilters[Index].ClassPath, DefaultTypes[Index].ClassPath);
	}
	ViewModel.SetScopeFilter(TOptional<ECFDAScope>());
	ViewModel.SetHealthFilter(TOptional<ECFDAHealthState>());

	// Type-only ManagementState filter must not change Asset View rows.
	ViewModel.SetManagementStateFilter(TOptional<ECFDAManagerTypePresentationState>(ECFDAManagerTypePresentationState::Managed));
	const TArray<FCFDAManagerTypeRow> ManagedTypes = ViewModel.BuildFilteredTypeRows();
	TestTrue(TEXT("Managed Type filter returns rows"), !ManagedTypes.IsEmpty());
	for (const FCFDAManagerTypeRow& Row : ManagedTypes)
	{
		TestEqual(TEXT("Managed Type filter is exact"), Row.PresentationState, ECFDAManagerTypePresentationState::Managed);
	}
	const TArray<FCFDAManagerAssetRow> AssetsWithTypeFilter = ViewModel.BuildFilteredAssetRows();
	TestEqual(TEXT("Asset View ignores Type-only ManagementState filter"), AssetsWithTypeFilter.Num(), DefaultAssets.Num());

	ViewModel.SetManagementStateFilter(TOptional<ECFDAManagerTypePresentationState>());
	ViewModel.SetShowAllDiscovered(true);
	const TArray<FCFDAManagerTypeRow> AllTypes = ViewModel.BuildFilteredTypeRows();
	const TArray<FCFDAManagerAssetRow> AllAssets = ViewModel.BuildFilteredAssetRows();
	TestTrue(TEXT("Show all discovery does not reduce Type rows"), AllTypes.Num() >= DefaultTypes.Num());
	TestTrue(TEXT("Show all discovery does not reduce Asset rows"), AllAssets.Num() >= DefaultAssets.Num());

	bool bFoundFramework = false;
	bool bFoundAuxiliary = false;
	for (const FCFDAManagerTypeRow& Row : AllTypes)
	{
		bFoundFramework |= Row.PresentationState == ECFDAManagerTypePresentationState::Framework;
		bFoundAuxiliary |= Row.PresentationState == ECFDAManagerTypePresentationState::AuxiliaryDataAsset;
	}
	TestTrue(TEXT("Show all discovery exposes current Framework type"), bFoundFramework);
	TestTrue(TEXT("Show all discovery exposes current Auxiliary DataAsset type"), bFoundAuxiliary);

	ViewModel.SetManagementStateFilter(TOptional<ECFDAManagerTypePresentationState>(ECFDAManagerTypePresentationState::Framework));
	const TArray<FCFDAManagerTypeRow> FrameworkTypes = ViewModel.BuildFilteredTypeRows();
	TestTrue(TEXT("Framework filter works when full discovery is visible"), !FrameworkTypes.IsEmpty());
	for (const FCFDAManagerTypeRow& Row : FrameworkTypes)
	{
		TestEqual(TEXT("Framework filter is exact"), Row.PresentationState, ECFDAManagerTypePresentationState::Framework);
	}

	// Type table sort는 selected column 방향을 따르되 동일 primary 값은 canonical ClassPath로 결정론적으로 정렬해야 합니다.
	TArray<FCFDAManagerTypeRow> SyntheticTypeRows;
	FCFDAManagerTypeRow TypeBravoB;
	TypeBravoB.DisplayName = TEXT("Bravo");
	TypeBravoB.ClassPath = TEXT("/Script/Test.B");
	SyntheticTypeRows.Add(TypeBravoB);
	FCFDAManagerTypeRow TypeAlpha;
	TypeAlpha.DisplayName = TEXT("Alpha");
	TypeAlpha.ClassPath = TEXT("/Script/Test.C");
	SyntheticTypeRows.Add(TypeAlpha);
	FCFDAManagerTypeRow TypeBravoA;
	TypeBravoA.DisplayName = TEXT("Bravo");
	TypeBravoA.ClassPath = TEXT("/Script/Test.A");
	SyntheticTypeRows.Add(TypeBravoA);

	FCFDAManagementVM::SortTypeRows(
		SyntheticTypeRows,
		ECFDAManagerTypeSortKey::DisplayName,
		ECFDAManagerSortDirection::Ascending);
	TestEqual(TEXT("Type ascending primary sort"), SyntheticTypeRows[0].DisplayName, FString(TEXT("Alpha")));
	TestEqual(TEXT("Type canonical tie-breaker A before B"), SyntheticTypeRows[1].ClassPath, FString(TEXT("/Script/Test.A")));
	TestEqual(TEXT("Type canonical tie-breaker B after A"), SyntheticTypeRows[2].ClassPath, FString(TEXT("/Script/Test.B")));

	FCFDAManagementVM::SortTypeRows(
		SyntheticTypeRows,
		ECFDAManagerTypeSortKey::DisplayName,
		ECFDAManagerSortDirection::Descending);
	TestEqual(TEXT("Type descending primary sort"), SyntheticTypeRows[0].DisplayName, FString(TEXT("Bravo")));
	TestEqual(TEXT("Type descending retains canonical tie-breaker"), SyntheticTypeRows[0].ClassPath, FString(TEXT("/Script/Test.A")));
	TestEqual(TEXT("Type descending puts Alpha last"), SyntheticTypeRows[2].DisplayName, FString(TEXT("Alpha")));

	// Asset table도 동일 primary 값에서 canonical ObjectPath tie-breaker를 사용해야 합니다.
	TArray<FCFDAManagerAssetRow> SyntheticAssetRows;
	FCFDAManagerAssetRow AssetBravoB;
	AssetBravoB.AssetName = TEXT("Bravo");
	AssetBravoB.ObjectPath = TEXT("/Game/Test/B.B");
	SyntheticAssetRows.Add(AssetBravoB);
	FCFDAManagerAssetRow AssetAlpha;
	AssetAlpha.AssetName = TEXT("Alpha");
	AssetAlpha.ObjectPath = TEXT("/Game/Test/C.C");
	SyntheticAssetRows.Add(AssetAlpha);
	FCFDAManagerAssetRow AssetBravoA;
	AssetBravoA.AssetName = TEXT("Bravo");
	AssetBravoA.ObjectPath = TEXT("/Game/Test/A.A");
	SyntheticAssetRows.Add(AssetBravoA);

	FCFDAManagementVM::SortAssetRows(
		SyntheticAssetRows,
		ECFDAManagerAssetSortKey::AssetName,
		ECFDAManagerSortDirection::Ascending);
	TestEqual(TEXT("Asset ascending primary sort"), SyntheticAssetRows[0].AssetName, FString(TEXT("Alpha")));
	TestEqual(TEXT("Asset canonical tie-breaker A before B"), SyntheticAssetRows[1].ObjectPath, FString(TEXT("/Game/Test/A.A")));
	TestEqual(TEXT("Asset canonical tie-breaker B after A"), SyntheticAssetRows[2].ObjectPath, FString(TEXT("/Game/Test/B.B")));

	FCFDAManagementVM::SortAssetRows(
		SyntheticAssetRows,
		ECFDAManagerAssetSortKey::AssetName,
		ECFDAManagerSortDirection::Descending);
	TestEqual(TEXT("Asset descending primary sort"), SyntheticAssetRows[0].AssetName, FString(TEXT("Bravo")));
	TestEqual(TEXT("Asset descending retains canonical tie-breaker"), SyntheticAssetRows[0].ObjectPath, FString(TEXT("/Game/Test/A.A")));
	TestEqual(TEXT("Asset descending puts Alpha last"), SyntheticAssetRows[2].AssetName, FString(TEXT("Alpha")));

	ViewModel.SetTypeSort(ECFDAManagerTypeSortKey::AssetCount, ECFDAManagerSortDirection::Descending);
	TestEqual(TEXT("VM stores Type sort key"), ViewModel.GetTypeSortKey(), ECFDAManagerTypeSortKey::AssetCount);
	TestEqual(TEXT("VM stores Type sort direction"), ViewModel.GetTypeSortDirection(), ECFDAManagerSortDirection::Descending);
	ViewModel.SetAssetSort(ECFDAManagerAssetSortKey::Health, ECFDAManagerSortDirection::Descending);
	TestEqual(TEXT("VM stores Asset sort key"), ViewModel.GetAssetSortKey(), ECFDAManagerAssetSortKey::Health);
	TestEqual(TEXT("VM stores Asset sort direction"), ViewModel.GetAssetSortDirection(), ECFDAManagerSortDirection::Descending);

	return true;
}

bool FCFDAManagerInformationHierarchyTest::RunTest(const FString& Parameters)
{
	FCFDAManagementVM ViewModel;
	FString Error;
	if (!TestTrue(TEXT("Manager VM initializes for information hierarchy"), ViewModel.Initialize(Error)))
	{
		AddError(Error);
		return false;
	}

	const FCFDAManagerOverview Overview = ViewModel.BuildOverview();
	const TArray<FCFDAManagerAssetRow> ManagedAssets = ViewModel.BuildFilteredAssetRows();
	TestEqual(TEXT("Overview managed Asset count matches default management universe"), Overview.ManagedAssetCount, ManagedAssets.Num());

	int32 ExpectedProblems = 0;
	int32 ExpectedNotValidated = 0;
	for (const FCFDAManagerAssetRow& Row : ManagedAssets)
	{
		TestTrue(TEXT("Managed Asset has user purpose"), !Row.UserPurposeDescription.IsEmpty());
		TestTrue(TEXT("Managed Asset has user usage"), !Row.UserUsageDescription.IsEmpty());
		if (Row.HealthState == ECFDAHealthState::Error || Row.HealthState == ECFDAHealthState::Warning)
		{
			++ExpectedProblems;
		}
		if (Row.HealthState == ECFDAHealthState::NotValidated)
		{
			++ExpectedNotValidated;
		}
	}
	TestEqual(TEXT("Overview problem count uses Error+Warning only"), Overview.ProblemAssetCount, ExpectedProblems);
	TestEqual(TEXT("Overview NotValidated is separate from problem count"), Overview.ManagedNotValidatedAssetCount, ExpectedNotValidated);

	int32 ExpectedNeedsRuleTypes = 0;
	for (const FCFDATypeRecord& TypeRecord : ViewModel.GetInventory().TypeRecords)
	{
		if (FCFDAManagementVM::ResolvePresentationState(TypeRecord) == ECFDAManagerTypePresentationState::NeedsManagementRule)
		{
			++ExpectedNeedsRuleTypes;
		}
	}
	TestEqual(TEXT("Overview needs-management-rule unit is Type count"), Overview.NeedsManagementRuleTypeCount, ExpectedNeedsRuleTypes);

	const TArray<FCFDAManagerTypeRow> ManagedTypes = ViewModel.BuildFilteredTypeRows();
	for (const FCFDAManagerTypeRow& Row : ManagedTypes)
	{
		TestTrue(TEXT("Managed Type has user purpose"), !Row.UserPurposeDescription.IsEmpty());
		TestTrue(TEXT("Managed Type has user usage"), !Row.UserUsageDescription.IsEmpty());
	}

	// UserUsageDescription에만 존재하는 문구도 검색 owner가 찾을 수 있어야 합니다.
	ViewModel.SetSearchText(TEXT("수량 1의 선택 가능한 장비"));
	const TArray<FCFDAManagerTypeRow> UsageSearchRows = ViewModel.BuildFilteredTypeRows();
	TestTrue(TEXT("Search includes user usage description"), !UsageSearchRows.IsEmpty());

	FCFDATypeRegistry FallbackRegistry;
	const FCFDAResolvedSemantic Fallback = FallbackRegistry.ResolveSemantic(
		TEXT("/Script/Example.FutureData"),
		TEXT("FutureData"));
	TestEqual(TEXT("Fallback remains Unregistered"), Fallback.CoverageState, ECFDACoverageState::Unregistered);
	TestFalse(TEXT("Fallback purpose is user-readable"), Fallback.Descriptor.UserPurposeDescription.IsEmpty());
	TestFalse(TEXT("Fallback usage is user-readable"), Fallback.Descriptor.UserUsageDescription.IsEmpty());
	TestFalse(TEXT("Fallback policy remains non-authoritative"), Fallback.HasAuthoritativePolicy());
	return true;
}

bool FCFDAManagerP004DCorrectionTest::RunTest(const FString& Parameters)
{
	TestEqual(
		TEXT("Stable ID NotResolved is user-facing unknown"),
		SCFDAManagementTab::BuildStableIdUserText(ECFDAStableIdState::NotResolved, FString()),
		FString(TEXT("미확인")));
	TestEqual(
		TEXT("Stable ID NotApplicable is explicit"),
		SCFDAManagementTab::BuildStableIdUserText(ECFDAStableIdState::NotApplicable, FString()),
		FString(TEXT("해당 없음")));
	TestEqual(
		TEXT("Stable ID MissingRequired is explicit"),
		SCFDAManagementTab::BuildStableIdUserText(ECFDAStableIdState::MissingRequired, FString()),
		FString(TEXT("필수 ID 누락")));
	TestEqual(
		TEXT("Stable ID Resolved shows actual value"),
		SCFDAManagementTab::BuildStableIdUserText(ECFDAStableIdState::Resolved, TEXT("DAM_Test_Id")),
		FString(TEXT("DAM_Test_Id")));
	TestEqual(
		TEXT("Never validated Health remains explicit"),
		SCFDAManagementTab::BuildHealthUserText(ECFDAHealthState::NotValidated, false),
		FString(TEXT("미검사")));
	TestEqual(
		TEXT("Stale Health requests revalidation"),
		SCFDAManagementTab::BuildHealthUserText(ECFDAHealthState::NotValidated, true),
		FString(TEXT("재검사 필요")));
	TestEqual(
		TEXT("Stale Stable ID requests recheck"),
		SCFDAManagementTab::BuildStableIdUserText(ECFDAStableIdState::NotResolved, FString(), true),
		FString(TEXT("재확인 필요")));

	// NoKnownReferencer가 deletion-safe 판정으로 해석되지 않도록 노출되는 사용자 경고입니다.
	const FString NoKnownReferencerSafetyText =
		SCFDAManagementTab::BuildReferencerSafetyText(ECFDAReferencerState::NoKnownReferencer);
	TestTrue(
		TEXT("No Known Referencer includes deletion safety warning"),
		NoKnownReferencerSafetyText.Contains(TEXT("삭제"))
			&& NoKnownReferencerSafetyText.Contains(TEXT("보장")));
	TestTrue(
		TEXT("Referenced state does not add deletion warning"),
		SCFDAManagementTab::BuildReferencerSafetyText(ECFDAReferencerState::Referenced).IsEmpty());

	// 실제 Slate surface와 ViewModel을 함께 구성해 presentation state를 검증합니다.
	TSharedRef<SCFDAManagementTab> Surface = SNew(SCFDAManagementTab);

	// surface가 소유한 current Data Management ViewModel입니다.
	TSharedPtr<FCFDAManagementVM> ViewModel = Surface->GetViewModelForTesting();
	if (!TestTrue(TEXT("P0-04D correction surface owns a VM"), ViewModel.IsValid()))
	{
		return false;
	}

	TestFalse(
		TEXT("Initial user status hides InventoryGeneration"),
		Surface->GetStatusMessageForTesting().Contains(TEXT("Generation")));

	// InventoryGeneration은 내부 freshness authority로 계속 존재해야 합니다.
	const uint64 GenerationBeforeRefresh = ViewModel->GetInventoryGeneration();
	TestTrue(TEXT("Internal InventoryGeneration remains active"), GenerationBeforeRefresh > 0);

	Surface->ExecuteRefreshForTesting();
	TestFalse(
		TEXT("Refresh user status hides InventoryGeneration"),
		Surface->GetStatusMessageForTesting().Contains(TEXT("Generation")));
	TestTrue(
		TEXT("Refresh still advances internal generation"),
		ViewModel->GetInventoryGeneration() != GenerationBeforeRefresh);

	ViewModel->SetViewMode(ECFDAManagerViewMode::AssetView);
	Surface->RefreshListItemsForTesting();

	// Asset selection synchronization 검증에 사용할 현재 visible Asset rows입니다.
	const TArray<FCFDAManagerAssetRow> AssetRows = ViewModel->BuildFilteredAssetRows();
	if (!TestTrue(TEXT("Asset rows exist for Slate selection sync"), !AssetRows.IsEmpty()))
	{
		return false;
	}

	// 정렬 전후 동일해야 하는 Asset ObjectPath입니다.
	const FString SelectedAssetObjectPath = AssetRows[0].ObjectPath;

	// Detail이 같은 대상을 계속 가리키는지 확인할 Asset 이름입니다.
	const FString SelectedAssetName = AssetRows[0].AssetName;
	if (!TestTrue(TEXT("VM selects Asset for Slate selection sync"), ViewModel->SelectAsset(SelectedAssetObjectPath)))
	{
		return false;
	}

	Surface->RefreshListItemsForTesting();
	TestEqual(
		TEXT("Slate Asset highlight follows VM selection"),
		Surface->GetSlateSelectedAssetPathForTesting(),
		SelectedAssetObjectPath);
	TestTrue(
		TEXT("Detail follows selected Asset"),
		Surface->GetDetailTextForTesting().ToString().Contains(SelectedAssetName));

	// P0-04E USER feedback correction: 기술 정보는 기술 식별값을 보존하되 레이블/상태 의미를 한글 우선으로 보여야 합니다.
	const FString AssetTechnicalDetail = Surface->GetTechnicalDetailTextForTesting().ToString();
	TestTrue(TEXT("Asset technical detail labels Object Path in Korean"), AssetTechnicalDetail.Contains(TEXT("오브젝트 경로 (Object Path):")));
	TestTrue(TEXT("Asset technical detail labels Class Path in Korean"), AssetTechnicalDetail.Contains(TEXT("클래스 경로 (Class Path):")));
	TestTrue(TEXT("Asset technical detail labels coverage in Korean"), AssetTechnicalDetail.Contains(TEXT("관리 정보 등록:")));
	TestTrue(TEXT("Asset technical detail labels evaluation in Korean"), AssetTechnicalDetail.Contains(TEXT("검사 상태:")));
	TestTrue(TEXT("Asset technical detail labels duplicate state in Korean"), AssetTechnicalDetail.Contains(TEXT("중복 상태:")));
	TestFalse(TEXT("Asset technical detail does not expose old Coverage label"), AssetTechnicalDetail.Contains(TEXT("Coverage:")));
	TestFalse(TEXT("Asset technical detail does not expose old Evaluation label"), AssetTechnicalDetail.Contains(TEXT("Evaluation:")));
	TestFalse(TEXT("Asset technical detail does not expose old Duplicate label"), AssetTechnicalDetail.Contains(TEXT("Duplicate:")));

	// 조회 전 참조 관계도 Unknown / Not Queried 같은 내부 표현 대신 사용자가 이해할 문구를 사용해야 합니다.
	const FString ReferenceTextBeforeQuery = Surface->GetReferenceTextForTesting().ToString();
	TestTrue(TEXT("Reference text explains not queried in Korean"), ReferenceTextBeforeQuery.Contains(TEXT("아직 조회하지 않음")));
	TestFalse(TEXT("Reference text hides old Unknown / Not Queried label"), ReferenceTextBeforeQuery.Contains(TEXT("Unknown / Not Queried")));

	ViewModel->SetAssetSort(
		ECFDAManagerAssetSortKey::Health,
		ECFDAManagerSortDirection::Descending);
	Surface->RefreshListItemsForTesting();
	TestEqual(
		TEXT("Slate Asset highlight survives row regeneration after sort"),
		Surface->GetSlateSelectedAssetPathForTesting(),
		SelectedAssetObjectPath);
	TestTrue(
		TEXT("Detail remains synchronized with Asset after sort"),
		Surface->GetDetailTextForTesting().ToString().Contains(SelectedAssetName));

	ViewModel->SetViewMode(ECFDAManagerViewMode::TypeView);
	Surface->RefreshListItemsForTesting();

	// Type selection synchronization 검증에 사용할 현재 visible Type rows입니다.
	const TArray<FCFDAManagerTypeRow> TypeRows = ViewModel->BuildFilteredTypeRows();
	if (!TestTrue(TEXT("Type rows exist for Slate selection sync"), !TypeRows.IsEmpty()))
	{
		return false;
	}

	// 정렬 전후 동일해야 하는 Type ClassPath입니다.
	const FString SelectedTypeClassPath = TypeRows[0].ClassPath;

	// Detail이 같은 Type을 계속 가리키는지 확인할 표시 이름입니다.
	const FString SelectedTypeDisplayName = TypeRows[0].DisplayName;
	if (!TestTrue(
		TEXT("Slate Type row can be selected through real callback path"),
		Surface->SelectTypeForTesting(SelectedTypeClassPath)))
	{
		return false;
	}

	ViewModel->SetTypeSort(
		ECFDAManagerTypeSortKey::AssetCount,
		ECFDAManagerSortDirection::Descending);
	Surface->RefreshListItemsForTesting();
	TestEqual(
		TEXT("Slate Type highlight survives row regeneration after sort"),
		Surface->GetSlateSelectedTypePathForTesting(),
		SelectedTypeClassPath);
	TestTrue(
		TEXT("Detail remains synchronized with Type after sort"),
		Surface->GetDetailTextForTesting().ToString().Contains(SelectedTypeDisplayName));

	// Type 기술 정보는 abstract 같은 구현 용어 대신 사용자가 판단할 수 있는 직접 생성 가능 여부를 보여줍니다.
	const FString TypeTechnicalDetail = Surface->GetTechnicalDetailTextForTesting().ToString();
	TestTrue(TEXT("Type technical detail explains direct asset creation"), TypeTechnicalDetail.Contains(TEXT("직접 에셋 생성:")));
	TestFalse(TEXT("Type technical detail hides old abstract wording"), TypeTechnicalDetail.Contains(TEXT("추상 유형:")));
	TestTrue(TEXT("Type technical detail labels canonical native state in Korean"), TypeTechnicalDetail.Contains(TEXT("CarFight 기본 C++ 유형:")));
	TestTrue(TEXT("Type technical detail labels persisted assets in Korean"), TypeTechnicalDetail.Contains(TEXT("저장된 에셋 수:")));
	TestFalse(TEXT("Type technical detail hides old Abstract label"), TypeTechnicalDetail.Contains(TEXT("Abstract:")));
	TestFalse(TEXT("Type technical detail hides old Canonical Native label"), TypeTechnicalDetail.Contains(TEXT("Canonical Native:")));
	TestFalse(TEXT("Type technical detail hides old Persisted Asset label"), TypeTechnicalDetail.Contains(TEXT("Persisted Asset:")));
	TestFalse(TEXT("Type technical detail hides old Has Assets state"), TypeTechnicalDetail.Contains(TEXT("Has Assets")));
	TestFalse(TEXT("Type technical detail hides old No Asset Instance state"), TypeTechnicalDetail.Contains(TEXT("No Asset Instance")));

	return true;
}

bool FCFDAManagerCacheReferenceTest::RunTest(const FString& Parameters)
{
	// successful-empty만 NoKnownReferencer가 될 수 있고 query failure는 반드시 QueryFailed여야 합니다.
	FCFDAReferenceResult EmptyEvidenceResult;
	FCFDAReferenceService::FinalizeQueryEvidence(true, true, EmptyEvidenceResult);
	TestEqual(TEXT("Successful empty Reference query succeeds"), EmptyEvidenceResult.QueryState, ECFDAReferenceQueryState::Succeeded);
	TestEqual(TEXT("Successful empty Referencer evidence is NoKnownReferencer"), EmptyEvidenceResult.ReferencerState, ECFDAReferencerState::NoKnownReferencer);
	TestTrue(TEXT("Successful empty Reference evidence is available"), EmptyEvidenceResult.bReferenceEvidenceAvailable);
	TestTrue(TEXT("Successful empty Referencer evidence is available"), EmptyEvidenceResult.bReferencerEvidenceAvailable);

	FCFDAReferenceResult MissingReferencerEvidenceResult;
	FCFDAReferenceService::FinalizeQueryEvidence(true, false, MissingReferencerEvidenceResult);
	TestEqual(TEXT("Missing Referencer node is QueryFailed"), MissingReferencerEvidenceResult.QueryState, ECFDAReferenceQueryState::QueryFailed);
	TestEqual(TEXT("Missing Referencer node cannot claim NoKnownReferencer"), MissingReferencerEvidenceResult.ReferencerState, ECFDAReferencerState::QueryFailed);
	TestTrue(TEXT("Reference side evidence can still be available"), MissingReferencerEvidenceResult.bReferenceEvidenceAvailable);
	TestFalse(TEXT("Referencer side evidence is unavailable"), MissingReferencerEvidenceResult.bReferencerEvidenceAvailable);

	FCFDAReferenceResult ReferencedEvidenceResult;
	ReferencedEvidenceResult.Referencers.Add(TEXT("/Game/CarFight/Tests/Referencer"));
	FCFDAReferenceService::FinalizeQueryEvidence(true, true, ReferencedEvidenceResult);
	TestEqual(TEXT("Non-empty successful Referencer evidence is Referenced"), ReferencedEvidenceResult.ReferencerState, ECFDAReferencerState::Referenced);

	FCFDAManagementVM ViewModel;
	FString Error;
	if (!TestTrue(TEXT("Manager VM initializes for cache test"), ViewModel.Initialize(Error)))
	{
		AddError(Error);
		return false;
	}

	const FCFDAAssetRecord* CatalogRecord =
		CFDAManagementTestsPrivate::FindFirstAssetByClass(
			ViewModel.GetInventory(),
			TEXT("/Script/CarFight_Re.CFRuntimeTestCatalogData"));
	TestNotNull(TEXT("RuntimeTestCatalog exists for Manager detail test"), CatalogRecord);
	if (!CatalogRecord)
	{
		return false;
	}

	// Refresh 이후에도 안전하게 사용할 선택 대상 ObjectPath 사본입니다.
	const FString CatalogObjectPath = CatalogRecord->ObjectPath;

	TestTrue(TEXT("Catalog can be selected"), ViewModel.SelectAsset(CatalogObjectPath));
	TestEqual(TEXT("Loaded cache starts empty"), ViewModel.GetLoadedCacheCount(), 0);
	TestEqual(TEXT("Reference cache starts empty"), ViewModel.GetReferenceCacheCount(), 0);
	TestNull(TEXT("Loaded detail starts unknown"), ViewModel.GetSelectedLoadedResult());
	TestNull(TEXT("References start Unknown / Not Queried"), ViewModel.GetSelectedReferenceResult());

	if (!TestTrue(TEXT("Selected catalog validates"), ViewModel.ValidateSelectedAsset(Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Loaded cache contains selected result"), ViewModel.GetLoadedCacheCount(), 1);
	TestEqual(TEXT("Fresh validation has no revalidation marker"), ViewModel.GetNeedsRevalidationCount(), 0);
	const FCFDALoadedAssetResult* LoadedResult = ViewModel.GetSelectedLoadedResult();
	TestNotNull(TEXT("Loaded result is available"), LoadedResult);
	if (LoadedResult)
	{
		TestTrue(TEXT("Loaded result is current generation"), LoadedResult->IsFreshForInventoryGeneration(ViewModel.GetInventoryGeneration()));
		TestEqual(TEXT("Catalog loaded evaluation succeeds"), LoadedResult->EvaluationState, ECFDAEvaluationState::Succeeded);
	}

	if (!TestTrue(TEXT("Selected catalog Reference query succeeds"), ViewModel.QuerySelectedReferences(Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Reference cache contains selected result"), ViewModel.GetReferenceCacheCount(), 1);
	const FCFDAReferenceResult* ReferenceResult = ViewModel.GetSelectedReferenceResult();
	TestNotNull(TEXT("Reference result is available"), ReferenceResult);
	if (ReferenceResult)
	{
		TestEqual(TEXT("Reference query state is Succeeded"), ReferenceResult->QueryState, ECFDAReferenceQueryState::Succeeded);
		TestTrue(
			TEXT("Referencer state is conservative conclusive state"),
			ReferenceResult->ReferencerState == ECFDAReferencerState::Referenced
				|| ReferenceResult->ReferencerState == ECFDAReferencerState::NoKnownReferencer);
	}

	const uint64 PreviousGeneration = ViewModel.GetInventoryGeneration();
	if (!TestTrue(TEXT("Manager metadata Refresh succeeds"), ViewModel.Refresh(Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(TEXT("Refresh advances generation"), ViewModel.GetInventoryGeneration() != PreviousGeneration);
	TestEqual(TEXT("Refresh invalidates loaded cache"), ViewModel.GetLoadedCacheCount(), 0);
	TestEqual(TEXT("Refresh invalidates reference cache"), ViewModel.GetReferenceCacheCount(), 0);
	TestEqual(TEXT("Refresh retains one revalidation marker without old result values"), ViewModel.GetNeedsRevalidationCount(), 1);
	TestNull(TEXT("Old loaded result is unavailable after Refresh"), ViewModel.GetSelectedLoadedResult());
	TestNull(TEXT("Old reference result is unavailable after Refresh"), ViewModel.GetSelectedReferenceResult());
	TestNotNull(TEXT("Selection persists when ObjectPath still exists"), ViewModel.GetSelectedAssetRecord());

	// Refresh 후 current inventory에서 같은 ObjectPath를 가리키는 사용자 row입니다.
	const TArray<FCFDAManagerAssetRow> RefreshedRows = ViewModel.BuildFilteredAssetRows();

	// 과거 검사값 대신 stale 상태만 투영하는 선택 대상 row입니다.
	const FCFDAManagerAssetRow* RefreshedCatalogRow = nullptr;
	for (const FCFDAManagerAssetRow& Row : RefreshedRows)
	{
		if (Row.ObjectPath == CatalogObjectPath)
		{
			RefreshedCatalogRow = &Row;
			break;
		}
	}
	TestNotNull(TEXT("Refreshed selected row remains visible"), RefreshedCatalogRow);
	if (RefreshedCatalogRow)
	{
		TestTrue(TEXT("Refreshed selected row requires revalidation"), RefreshedCatalogRow->bNeedsRevalidation);
		TestEqual(
			TEXT("Refreshed Health is shown as revalidation required"),
			SCFDAManagementTab::BuildHealthUserText(
				RefreshedCatalogRow->HealthState,
				RefreshedCatalogRow->bNeedsRevalidation),
			FString(TEXT("재검사 필요")));
		TestEqual(
			TEXT("Refreshed Stable ID is shown as recheck required"),
			SCFDAManagementTab::BuildStableIdUserText(
				RefreshedCatalogRow->StableIdState,
				RefreshedCatalogRow->StableId,
				RefreshedCatalogRow->bNeedsRevalidation),
			FString(TEXT("재확인 필요")));
	}

	if (!TestTrue(TEXT("Selected catalog revalidates after Refresh"), ViewModel.ValidateSelectedAsset(Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Revalidation restores current loaded result"), ViewModel.GetLoadedCacheCount(), 1);
	TestEqual(TEXT("Revalidation clears stale marker"), ViewModel.GetNeedsRevalidationCount(), 0);
	TestNotNull(TEXT("Current loaded result is available after revalidation"), ViewModel.GetSelectedLoadedResult());
	return true;
}

bool FCFDAManagerTabTest::RunTest(const FString& Parameters)
{
	const FName DataAssetManagerTabName(TEXT("CarFight.DataAssetManager"));
	TestTrue(
		TEXT("Data Asset Manager Nomad Tab spawner is registered"),
		FGlobalTabmanager::Get()->HasTabSpawner(DataAssetManagerTabName));

	TSharedPtr<SDockTab> SpawnedTab =
		FGlobalTabmanager::Get()->TryInvokeTab(DataAssetManagerTabName);
	TestTrue(TEXT("Data Asset Manager tab can be invoked"), SpawnedTab.IsValid());

	TSharedRef<SCFDAManagementTab> Surface = SNew(SCFDAManagementTab);
	TestTrue(TEXT("Data Asset Manager surface owns a VM"), Surface->GetViewModelForTesting().IsValid());
	if (Surface->GetViewModelForTesting().IsValid())
	{
		TestTrue(
			TEXT("Data Asset Manager surface has current metadata inventory"),
			Surface->GetViewModelForTesting()->GetInventory().IsInventoryComplete());
		TestEqual(
			TEXT("Surface starts with no loaded cache"),
			Surface->GetViewModelForTesting()->GetLoadedCacheCount(),
			0);
		TestEqual(
			TEXT("Surface starts with no Reference cache"),
			Surface->GetViewModelForTesting()->GetReferenceCacheCount(),
			0);
	}

	if (SpawnedTab.IsValid())
	{
		SpawnedTab->RequestCloseTab();
	}
	return true;
}

#endif
