// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleCatalogPromoTests.cpp
// Version: v1.0.0
// Date: 2026-09-02
// Description: CF-FQ-044 VRCP-P0-02 Editor Promotion Service focused Automation입니다.
// Changelog:
// - v1.0.0: Registered/idempotency, invalid catalog ordering, unavailable/invalid target, no-save와 Undo/Redo fresh membership을 isolated Catalog fixture로 검증.
// Migration:
// - Default Product Catalog는 read-only fixture source로만 사용하며 mutation/save하지 않습니다.

#include "DataAuthoring/CFVehicleCatalogPromoService.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"
#include "CFVehicleData.h"
#include "Editor.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

namespace CFVehicleCatalogPromoTests
{
	/** Product Catalog의 persistent references만 복사해 쓰는 isolated unsaved Catalog fixture입니다. */
	struct FCatalogFixture
	{
		// Test-owned unsaved /Game package입니다.
		UPackage* CatalogPackage = nullptr;

		// Test-owned Runtime Catalog object입니다.
		UCFRuntimeTestCatalogData* Catalog = nullptr;

		// Promotion target으로 사용할 persistent Product VehicleData입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// Catalog 초기 seed로 사용할 다른 persistent Product VehicleData입니다.
		UCFVehicleData* SeedVehicleData = nullptr;

		// Catalog validity를 만족시키는 persistent Product EquipmentPresetData입니다.
		UCFEquipmentPresetData* SeedEquipmentPresetData = nullptr;

		// Test마다 unique unsaved package를 만들고 Product assets는 read-only reference로만 연결합니다.
		bool Initialize(FString& OutError)
		{
			// Current Product Default Catalog입니다.
			UCFRuntimeTestCatalogData* ProductCatalog = GetDefault<UCFRuntimeTestSettings>()->LoadDefaultCatalog();
			if (!IsValid(ProductCatalog)
				|| ProductCatalog->AllowedVehicleData.Num() < 2
				|| ProductCatalog->AllowedEquipmentPresetData.IsEmpty())
			{
				OutError = TEXT("Focused Promotion test에는 valid Product Default Catalog의 Vehicle 2개/Equipment 1개 이상이 필요합니다.");
				return false;
			}

			SeedVehicleData = ProductCatalog->AllowedVehicleData[0].Get();
			TargetVehicleData = ProductCatalog->AllowedVehicleData[1].Get();
			SeedEquipmentPresetData = ProductCatalog->AllowedEquipmentPresetData[0].Get();
			if (!IsValid(SeedVehicleData) || !IsValid(TargetVehicleData) || !IsValid(SeedEquipmentPresetData))
			{
				OutError = TEXT("Focused Promotion test의 persistent Product fixture reference가 유효하지 않습니다.");
				return false;
			}

			// Unique test-owned package name입니다.
			const FString PackageName = FString::Printf(
				TEXT("/Game/CarFight/Tests/Transient/DA_Promo_%s"),
				*FGuid::NewGuid().ToString(EGuidFormats::Digits));
			CatalogPackage = CreatePackage(*PackageName);
			if (!CatalogPackage)
			{
				OutError = TEXT("Focused Promotion test package를 만들 수 없습니다.");
				return false;
			}

			Catalog = NewObject<UCFRuntimeTestCatalogData>(
				CatalogPackage,
				UCFRuntimeTestCatalogData::StaticClass(),
				TEXT("DA_PromoCatalog"),
				RF_Transactional);
			if (!Catalog)
			{
				OutError = TEXT("Focused Promotion test Catalog를 만들 수 없습니다.");
				return false;
			}

			Catalog->AllowedVehicleData.Add(SeedVehicleData);
			Catalog->AllowedEquipmentPresetData.Add(SeedEquipmentPresetData);
			CatalogPackage->SetDirtyFlag(false);
			OutError.Reset();
			return true;
		}

		// Test-owned unsaved objects를 garbage 대상으로 돌리고 disk save는 수행하지 않습니다.
		void Cleanup()
		{
			if (CatalogPackage)
			{
				CatalogPackage->SetDirtyFlag(false);
			}
			if (Catalog)
			{
				Catalog->MarkAsGarbage();
			}
			if (CatalogPackage)
			{
				CatalogPackage->MarkAsGarbage();
			}
		}
	};

	// 공통 fixture 초기화 실패를 Automation error로 보고합니다.
	bool InitializeFixture(
		FAutomationTestBase& Test,
		FCatalogFixture& OutFixture)
	{
		// Fixture initialization diagnostic입니다.
		FString FixtureError;
		if (!OutFixture.Initialize(FixtureError))
		{
			Test.AddError(FixtureError);
			return false;
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCatalogPromoRegisterTest,
	"CarFight.DataAuthoring.CF_FQ_044.VRCP_P0_02.RegisterIdempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCatalogPromoInvalidCatalogTest,
	"CarFight.DataAuthoring.CF_FQ_044.VRCP_P0_02.InvalidCatalogBeforeMembership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCatalogPromoUnavailableTest,
	"CarFight.DataAuthoring.CF_FQ_044.VRCP_P0_02.UnavailableAndInvalidTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCatalogPromoUndoRedoTest,
	"CarFight.DataAuthoring.CF_FQ_044.VRCP_P0_02.UndoRedoFreshMembership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Valid isolated Catalog에 exact target을 1회만 등록하고 second call이 no-op success인지 검증합니다.
bool FCFVehicleCatalogPromoRegisterTest::RunTest(const FString& Parameters)
{
	// Isolated Catalog fixture입니다.
	CFVehicleCatalogPromoTests::FCatalogFixture Fixture;
	if (!CFVehicleCatalogPromoTests::InitializeFixture(*this, Fixture))
	{
		return false;
	}

	// Mutation 전 vehicle count입니다.
	const int32 InitialVehicleCount = Fixture.Catalog->AllowedVehicleData.Num();
	// First promotion terminal result입니다.
	const FCFVehicleCatalogPromoResult FirstResult =
		FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(Fixture.TargetVehicleData, Fixture.Catalog);
	TestEqual(TEXT("First promotion outcome"), FirstResult.Outcome, ECFVehicleCatalogPromoOutcome::Registered);
	TestTrue(TEXT("First promotion exact membership"), FirstResult.bIsMember);
	TestFalse(TEXT("First promotion auto-save"), FirstResult.bSavePerformed);
	TestTrue(TEXT("First promotion leaves Catalog dirty"), FirstResult.bCatalogPackageDirty);
	TestEqual(TEXT("First promotion increments count once"), Fixture.Catalog->AllowedVehicleData.Num(), InitialVehicleCount + 1);

	// Second idempotent promotion terminal result입니다.
	const FCFVehicleCatalogPromoResult SecondResult =
		FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(Fixture.TargetVehicleData, Fixture.Catalog);
	TestEqual(TEXT("Second promotion outcome"), SecondResult.Outcome, ECFVehicleCatalogPromoOutcome::AlreadyRegistered);
	TestTrue(TEXT("Second promotion exact membership"), SecondResult.bIsMember);
	TestFalse(TEXT("Second promotion auto-save"), SecondResult.bSavePerformed);
	TestEqual(TEXT("Second promotion duplicate zero"), Fixture.Catalog->AllowedVehicleData.Num(), InitialVehicleCount + 1);

	Fixture.Cleanup();
	return true;
}

// Invalid Catalog가 target membership을 이미 포함해도 AlreadyRegistered로 우회되지 않는지 검증합니다.
bool FCFVehicleCatalogPromoInvalidCatalogTest::RunTest(const FString& Parameters)
{
	// Isolated Catalog fixture입니다.
	CFVehicleCatalogPromoTests::FCatalogFixture Fixture;
	if (!CFVehicleCatalogPromoTests::InitializeFixture(*this, Fixture))
	{
		return false;
	}

	Fixture.Catalog->AllowedVehicleData.Add(Fixture.TargetVehicleData);
	Fixture.Catalog->AllowedVehicleData.Add(Fixture.TargetVehicleData);

	// Invalid pre-validation 결과입니다.
	const FCFVehicleCatalogPromoResult Result =
		FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(Fixture.TargetVehicleData, Fixture.Catalog);
	TestEqual(TEXT("Invalid Catalog wins before membership"), Result.Outcome, ECFVehicleCatalogPromoOutcome::CatalogInvalid);
	TestFalse(TEXT("Invalid Catalog auto-save"), Result.bSavePerformed);

	Fixture.Cleanup();
	return true;
}

// Missing DefaultCatalog raw resolve와 null/transient VehicleData 차단을 검증합니다.
bool FCFVehicleCatalogPromoUnavailableTest::RunTest(const FString& Parameters)
{
	// DefaultCatalog가 비어 있는 test-owned Settings object입니다.
	UCFRuntimeTestSettings* EmptySettings = NewObject<UCFRuntimeTestSettings>(GetTransientPackage());
	// Config class CDO에서 복사된 project DefaultCatalog를 제거해 exact unavailable fixture를 만듭니다.
	EmptySettings->DefaultCatalog.Reset();
	// Raw resolver output Catalog입니다.
	UCFRuntimeTestCatalogData* ResolvedCatalog = nullptr;
	// Raw resolver failure result입니다.
	FCFVehicleCatalogPromoResult ResolveFailure;
	const bool bResolved = FCFVehicleCatalogPromoService::ResolveRuntimeCatalogRaw(
		EmptySettings,
		ResolvedCatalog,
		ResolveFailure);
	TestFalse(TEXT("Empty DefaultCatalog raw resolve fails"), bResolved);
	TestEqual(TEXT("Empty DefaultCatalog outcome"), ResolveFailure.Outcome, ECFVehicleCatalogPromoOutcome::CatalogUnavailable);
	TestNull(TEXT("Empty DefaultCatalog output"), ResolvedCatalog);

	// Null target result입니다.
	const FCFVehicleCatalogPromoResult NullTargetResult =
		FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(nullptr, nullptr);
	TestEqual(TEXT("Null target outcome"), NullTargetResult.Outcome, ECFVehicleCatalogPromoOutcome::InvalidTarget);

	// Transient VehicleData fixture입니다.
	UCFVehicleData* TransientVehicleData = NewObject<UCFVehicleData>(
		GetTransientPackage(),
		UCFVehicleData::StaticClass(),
		TEXT("PromoTransientVehicle"),
		RF_Transient);
	// Transient target result입니다.
	const FCFVehicleCatalogPromoResult TransientTargetResult =
		FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(TransientVehicleData, nullptr);
	TestEqual(TEXT("Transient target outcome"), TransientTargetResult.Outcome, ECFVehicleCatalogPromoOutcome::InvalidTarget);
	return true;
}

// Standard Editor Undo/Redo가 exact membership을 제거/복원하고 fresh status가 stale cache 없이 따라가는지 검증합니다.
bool FCFVehicleCatalogPromoUndoRedoTest::RunTest(const FString& Parameters)
{
	// Isolated Catalog fixture입니다.
	CFVehicleCatalogPromoTests::FCatalogFixture Fixture;
	if (!CFVehicleCatalogPromoTests::InitializeFixture(*this, Fixture))
	{
		return false;
	}
	if (!GEditor)
	{
		AddError(TEXT("Undo/Redo focused test에 GEditor가 필요합니다."));
		Fixture.Cleanup();
		return false;
	}

	// Registered transaction result입니다.
	const FCFVehicleCatalogPromoResult RegisterResult =
		FCFVehicleCatalogPromoService::PromoteVehicleToCatalog(Fixture.TargetVehicleData, Fixture.Catalog);
	TestEqual(TEXT("Undo/Redo setup registered"), RegisterResult.Outcome, ECFVehicleCatalogPromoOutcome::Registered);

	GEditor->UndoTransaction();

	// Undo 뒤 fresh membership result입니다.
	const FCFVehicleCatalogPromoResult UndoReadback =
		FCFVehicleCatalogPromoService::ReadPromotionMembership(Fixture.TargetVehicleData, Fixture.Catalog);
	TestFalse(TEXT("Undo removes exact membership"), UndoReadback.bIsMember);
	TestEqual(TEXT("Undo restores initial vehicle count"), Fixture.Catalog->AllowedVehicleData.Num(), 1);

	GEditor->RedoTransaction();

	// Redo 뒤 fresh membership result입니다.
	const FCFVehicleCatalogPromoResult RedoReadback =
		FCFVehicleCatalogPromoService::ReadPromotionMembership(Fixture.TargetVehicleData, Fixture.Catalog);
	TestTrue(TEXT("Redo restores exact membership"), RedoReadback.bIsMember);
	TestEqual(TEXT("Redo membership outcome"), RedoReadback.Outcome, ECFVehicleCatalogPromoOutcome::AlreadyRegistered);
	TestEqual(TEXT("Redo restores promoted vehicle count"), Fixture.Catalog->AllowedVehicleData.Num(), 2);
	TestFalse(TEXT("Redo readback auto-save"), RedoReadback.bSavePerformed);

	Fixture.Cleanup();
	return true;
}

#endif
