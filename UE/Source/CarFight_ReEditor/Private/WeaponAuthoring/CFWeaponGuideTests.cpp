// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideTests.cpp
// Version: v1.3.1
// Date: 2026-09-19
// Description: CF-FQ-055 WEA-P0-01~04 Stable Step, durable exact5 graph, partial recovery와 completion semantic handoff focused Automation입니다.
// Changelog:
// - v1.3.1: WEA-P0-04 Mid-review P1 회귀로 FireMode/Launcher/Guidance stale semantic 차단과 P0-03 Bundle fingerprint 불변을 P004에 추가했습니다.
// - v1.3.0: WEA-P0-04 completion USER summary, deterministic fingerprint integrity와 Equipment Builder navigation-only handoff contract 테스트를 추가했습니다.
// - v1.2.0: WEA-P0-03 disposable exact5 durable graph Create와 Damage+Ammo confirmed partial durable interruption→same-session recovery 테스트를 추가했습니다.
// - v1.1.1: Damage radial non-default field와 Ammo non-null Texture2D icon까지 provider mapping round-trip에 포함해 P0-02 normal authoring payload coverage를 보강했습니다.
// - v1.1.0: WEA-P0-02 Damage/Ammo CreateNew Draft가 existing provider payload/fingerprint/serializer/parser 계약을 mutation0으로 통과하는지 검증하도록 provider boundary 테스트를 전진.
// - v1.0.0: VisibleSteps, Template topology, deterministic naming, Capability validation, Damage/Ammo provider boundary 테스트 추가.
// Migration:
// - durable Automation은 `/Game/CarFight/Tests/WeaponGuide/P003` disposable namespace만 실제 저장하며 시작/종료 residue exact0을 강제합니다.
// - Product Asset은 생성/수정/저장하지 않습니다.
// - naming/provider preview 검증은 Product mutation 없이 disposable target만 사용합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "WeaponAuthoring/CFWeaponGuideVM.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFProjectileData.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace
{
	// visible Step 목록에서 지정 Step의 index를 반환합니다.
	int32 FindVisibleStepIndex(const TArray<ECFWeaponGuideStep>& VisibleSteps, const ECFWeaponGuideStep Step)
	{
		return VisibleSteps.IndexOfByKey(Step);
	}

	// visible Step 목록에 지정 Step이 포함됐는지 반환합니다.
	bool ContainsVisibleStep(const TArray<ECFWeaponGuideStep>& VisibleSteps, const ECFWeaponGuideStep Step)
	{
		return VisibleSteps.Contains(Step);
	}

	// WEA-P0-03 durable Automation 전용 package root입니다.
	const FString WeaponGuideP003FixtureRoot(TEXT("/Game/CarFight/Tests/WeaponGuide/P003"));

	// 지정 object path가 WEA-P0-03 disposable namespace 안에 있는지 반환합니다.
	bool IsP003FixtureObjectPath(const FString& ObjectPath)
	{
		return ObjectPath.StartsWith(WeaponGuideP003FixtureRoot + TEXT("/"), ESearchCase::CaseSensitive);
	}

	// WEA-P0-03 fixture root에 registry/disk residue가 exact0인지 검증합니다.
	bool VerifyP003FixtureRootResidueFree(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Registry-visible disposable assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FName(*WeaponGuideP003FixtureRoot), FixtureAssets, true, true);
		if (!FixtureAssets.IsEmpty())
		{
			OutError = FString::Printf(TEXT("WEA-P0-03 fixture registry residue가 %d개 남아 있습니다."), FixtureAssets.Num());
			return false;
		}

		// Exact disposable physical Content root입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), TEXT("CarFight/Tests/WeaponGuide/P003")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory))
		{
			OutError = FString::Printf(TEXT("WEA-P0-03 fixture disk residue가 남아 있습니다: %s"), *ContentDirectory);
			return false;
		}
		OutError.Reset();
		return true;
	}

	// WEA-P0-03 exact disposable Content root만 unload→GC→disk delete→registry refresh 순서로 정리합니다.
	bool CleanupP003FixtureRoot(FString& OutError)
	{
		// Current Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// Registry-visible disposable assets입니다.
		TArray<FAssetData> FixtureAssets;
		AssetRegistry.GetAssetsByPath(FName(*WeaponGuideP003FixtureRoot), FixtureAssets, true, true);
		// Unload할 unique package 목록입니다.
		TArray<UPackage*> PackagesToUnload;
		// Disk delete 뒤 registry refresh에 사용할 exact filenames입니다.
		TArray<FString> DeletedFilenames;

		for (const FAssetData& FixtureAssetData : FixtureAssets)
		{
			// Loaded 또는 persisted disposable object입니다.
			UObject* FixtureObject = FixtureAssetData.GetSoftObjectPath().ResolveObject();
			if (FixtureObject == nullptr)
			{
				FixtureObject = FixtureAssetData.GetAsset();
			}
			// Fixture owning package입니다.
			UPackage* FixturePackage = FixtureObject != nullptr ? FixtureObject->GetOutermost() : nullptr;
			if (FixturePackage != nullptr)
			{
				FixturePackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(FixturePackage);
			}
			// Expected persisted .uasset filename입니다.
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(
				FixtureAssetData.PackageName.ToString(),
				FPackageName::GetAssetPackageExtension());
			DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(PackageFilename));
		}

		for (TObjectIterator<UObject> ObjectIterator; ObjectIterator; ++ObjectIterator)
		{
			// Loaded-only disposable object candidate입니다.
			UObject* LoadedObject = *ObjectIterator;
			if (LoadedObject == nullptr || LoadedObject->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject | RF_Transient))
			{
				continue;
			}
			if (!IsP003FixtureObjectPath(LoadedObject->GetPathName()))
			{
				continue;
			}
			// Loaded fixture owning package입니다.
			UPackage* LoadedPackage = LoadedObject->GetOutermost();
			if (LoadedPackage != nullptr)
			{
				LoadedPackage->SetDirtyFlag(false);
				PackagesToUnload.AddUnique(LoadedPackage);
				// Loaded-only package도 registry refresh candidate로 포함합니다.
				const FString LoadedFilename = FPackageName::LongPackageNameToFilename(
					LoadedPackage->GetName(),
					FPackageName::GetAssetPackageExtension());
				DeletedFilenames.AddUnique(FPaths::ConvertRelativePathToFull(LoadedFilename));
			}
			LoadedObject->ClearFlags(RF_Standalone);
		}

		if (!PackagesToUnload.IsEmpty() && !UPackageTools::UnloadPackages(PackagesToUnload))
		{
			OutError = TEXT("WEA-P0-03 fixture package unload에 실패했습니다.");
			return false;
		}
		CollectGarbage(RF_NoFlags);

		// Exact disposable physical Content root입니다.
		const FString ContentDirectory = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectContentDir(), TEXT("CarFight/Tests/WeaponGuide/P003")));
		if (IFileManager::Get().DirectoryExists(*ContentDirectory)
			&& !IFileManager::Get().DeleteDirectory(*ContentDirectory, false, true))
		{
			OutError = FString::Printf(TEXT("WEA-P0-03 fixture Content root 삭제에 실패했습니다: %s"), *ContentDirectory);
			return false;
		}
		if (!DeletedFilenames.IsEmpty())
		{
			AssetRegistry.ScanModifiedAssetFiles(DeletedFilenames);
			AssetRegistry.WaitForCompletion();
		}
		return VerifyP003FixtureRootResidueFree(OutError);
	}

	// P0-03 exact5 신규 child durable 생성에 사용할 정상 finite-ammo Draft를 구성합니다.
	void ConfigureP003CreateDraft(
		FCFWeaponGuideVM& ViewModel,
		const FString& FixtureSubRoot,
		const FString& BaseAssetName)
	{
		ViewModel.Reset();
		ViewModel.SetDisposableTestRoot(WeaponGuideP003FixtureRoot + TEXT("/") + FixtureSubRoot);
		ViewModel.SetBaseAssetName(BaseAssetName);
		ViewModel.SetSuggestedDisplayName(TEXT("WEA P0-03 자동화 무장"));

		// exact5 신규 child intent를 가진 mutable Draft입니다.
		FCFWeaponGuideDraft& Draft = ViewModel.GetMutableDraft();
		Draft.Turret.Mode = ECFWeaponGuideChildMode::CreateNew;
		Draft.Weapon.Mode = ECFWeaponGuideChildMode::CreateNew;
		Draft.Projectile.Mode = ECFWeaponGuideChildMode::CreateNew;
		Draft.Damage.Mode = ECFWeaponGuideChildMode::CreateNew;
		Draft.Damage.DamageType = ECFDamageType::Kinetic;
		Draft.Damage.BaseDamage = 120.0f;
		Draft.Damage.ArmorPenetration = 15.0f;
		Draft.Damage.ModuleDamageScale = 1.0f;
		Draft.Weapon.bUseFiniteAmmo = true;
		Draft.Weapon.MagazineSize = 12;
		Draft.Weapon.InitialLoadedAmmoCount = 6;
		Draft.Weapon.AmmoUnitsPerShot = 1;
		Draft.Weapon.ReloadTimeSeconds = 2.0f;
		Draft.Ammo.Mode = ECFWeaponGuideChildMode::CreateNew;
		Draft.Ammo.AmmoDisplayName = TEXT("WEA P0-03 Test Ammo");
		Draft.Ammo.AmmoFamilyId = TEXT("WEAP003");
		Draft.Ammo.UnitMassKg = 1.25f;
		Draft.Ammo.AmmoTags = {TEXT("Shell"), TEXT("WeaponGuideP003")};
		Draft.Ammo.AmmoIconPath = FSoftObjectPath(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
		Draft.Ammo.MaximumLoadableAmmoCount = 120;
		ViewModel.NotifyDraftTopologyChanged();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideVisibleStepsTest,
	"CarFight.WeaponAuthoring.P001.VisibleSteps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Common Step과 Capability 조건부 Step projection이 deterministic 순서를 유지하는지 검증합니다.
bool FCFWeaponGuideVisibleStepsTest::RunTest(const FString& Parameters)
{
	// focused test용 session-local Weapon Guide ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ViewModel.Reset();

	// 기본 Direct Fire 형태에서 계산한 visible Step 목록입니다.
	const TArray<ECFWeaponGuideStep> DefaultSteps = ViewModel.GetVisibleSteps();
	TestTrue(TEXT("IdentityTemplate is visible"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::IdentityTemplate));
	TestTrue(TEXT("Projectile is visible for default Projectile FireMode"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Projectile));
	TestTrue(TEXT("Damage is always visible"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Damage));
	TestFalse(TEXT("Ammo is hidden for infinite ammo"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Ammo));
	TestFalse(TEXT("Launcher is hidden by default"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Launcher));
	TestFalse(TEXT("Propulsion is hidden by default"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Propulsion));
	TestFalse(TEXT("MissileFlight is hidden by default"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::MissileFlight));
	TestFalse(TEXT("Guidance is hidden by default"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Guidance));
	TestFalse(TEXT("Heat is hidden by default"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Heat));
	TestFalse(TEXT("Charge is hidden by default"), ContainsVisibleStep(DefaultSteps, ECFWeaponGuideStep::Charge));

	// 모든 conditional Capability를 활성화할 mutable Draft입니다.
	FCFWeaponGuideDraft& Draft = ViewModel.GetMutableDraft();
	Draft.Weapon.bUseFiniteAmmo = true;
	Draft.Weapon.bUseLauncher = true;
	Draft.Projectile.PropulsionConfig.bUsePropulsion = true;
	Draft.Projectile.MissileFlightConfig.bUseMissileFlight = true;
	Draft.Projectile.MissileGuideConfig.bUseGuidance = true;
	Draft.Projectile.MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
	Draft.Weapon.bUseHeat = true;
	Draft.Weapon.bUseCharge = true;
	ViewModel.NotifyDraftTopologyChanged();

	// 모든 conditional Capability 활성화 후 visible Step 목록입니다.
	const TArray<ECFWeaponGuideStep> FullSteps = ViewModel.GetVisibleSteps();
	TestTrue(TEXT("Ammo becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::Ammo));
	TestTrue(TEXT("Launcher becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::Launcher));
	TestTrue(TEXT("Propulsion becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::Propulsion));
	TestTrue(TEXT("MissileFlight becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::MissileFlight));
	TestTrue(TEXT("Guidance becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::Guidance));
	TestTrue(TEXT("Heat becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::Heat));
	TestTrue(TEXT("Charge becomes visible"), ContainsVisibleStep(FullSteps, ECFWeaponGuideStep::Charge));
	TestTrue(
		TEXT("Projectile precedes Damage"),
		FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Projectile) < FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Damage));
	TestTrue(
		TEXT("Damage precedes Ammo"),
		FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Damage) < FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Ammo));
	TestTrue(
		TEXT("Guidance precedes Heat"),
		FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Guidance) < FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Heat));
	TestTrue(
		TEXT("ReviewCreate precedes Complete"),
		FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::ReviewCreate) < FindVisibleStepIndex(FullSteps, ECFWeaponGuideStep::Complete));

	Draft.Weapon.FireMode = ECFWeaponFireMode::HitScan;
	ViewModel.NotifyDraftTopologyChanged();

	// HitScan 전환 뒤 physical Projectile 계열 Step이 제거된 projection입니다.
	const TArray<ECFWeaponGuideStep> HitScanSteps = ViewModel.GetVisibleSteps();
	TestFalse(TEXT("Physical Projectile page is hidden for HitScan"), ContainsVisibleStep(HitScanSteps, ECFWeaponGuideStep::Projectile));
	TestFalse(TEXT("Propulsion page is hidden for HitScan"), ContainsVisibleStep(HitScanSteps, ECFWeaponGuideStep::Propulsion));
	TestFalse(TEXT("MissileFlight page is hidden for HitScan"), ContainsVisibleStep(HitScanSteps, ECFWeaponGuideStep::MissileFlight));
	TestFalse(TEXT("Guidance page is hidden for HitScan"), ContainsVisibleStep(HitScanSteps, ECFWeaponGuideStep::Guidance));
	TestTrue(TEXT("Damage remains visible for virtual ProjectileData"), ContainsVisibleStep(HitScanSteps, ECFWeaponGuideStep::Damage));
	TestTrue(TEXT("Ammo remains visible when finite ammo is enabled"), ContainsVisibleStep(HitScanSteps, ECFWeaponGuideStep::Ammo));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideTemplateTest,
	"CarFight.WeaponAuthoring.P001.TemplateTopology",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 대표 Template이 Runtime hard class가 아니라 일반적인 Capability 기본값만 제안하는지 검증합니다.
bool FCFWeaponGuideTemplateTest::RunTest(const FString& Parameters)
{
	// Template 제안 검증용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ViewModel.Reset();

	ViewModel.SetTemplate(ECFWeaponGuideTemplate::GuidedMissileLauncher);

	// Guided Missile Template 적용 후 Draft입니다.
	const FCFWeaponGuideDraft& GuidedDraft = ViewModel.GetDraft();
	TestEqual(TEXT("Guided template keeps Projectile FireMode"), GuidedDraft.Weapon.FireMode, ECFWeaponFireMode::Projectile);
	TestTrue(TEXT("Guided template enables Launcher"), GuidedDraft.Weapon.bUseLauncher);
	TestTrue(TEXT("Guided template enables Propulsion"), GuidedDraft.Projectile.PropulsionConfig.bUsePropulsion);
	TestTrue(TEXT("Guided template enables MissileFlight"), GuidedDraft.Projectile.MissileFlightConfig.bUseMissileFlight);
	TestTrue(TEXT("Guided template enables Guidance"), GuidedDraft.Projectile.MissileGuideConfig.bUseGuidance);
	TestEqual(TEXT("Guided template proposes TargetActor"), GuidedDraft.Projectile.MissileGuideConfig.GuideMode, ECFMissileGuideMode::TargetActor);

	ViewModel.SetTemplate(ECFWeaponGuideTemplate::RocketLauncher);

	// Rocket Template 적용 후 Draft입니다.
	const FCFWeaponGuideDraft& RocketDraft = ViewModel.GetDraft();
	TestTrue(TEXT("Rocket template enables Launcher"), RocketDraft.Weapon.bUseLauncher);
	TestTrue(TEXT("Rocket template enables Propulsion"), RocketDraft.Projectile.PropulsionConfig.bUsePropulsion);
	TestFalse(TEXT("Rocket template disables MissileFlight"), RocketDraft.Projectile.MissileFlightConfig.bUseMissileFlight);
	TestFalse(TEXT("Rocket template disables Guidance"), RocketDraft.Projectile.MissileGuideConfig.bUseGuidance);

	ViewModel.SetTemplate(ECFWeaponGuideTemplate::DirectFireCannon);

	// Direct Fire Template 적용 후 Draft입니다.
	const FCFWeaponGuideDraft& DirectDraft = ViewModel.GetDraft();
	TestFalse(TEXT("Direct template disables Launcher"), DirectDraft.Weapon.bUseLauncher);
	TestFalse(TEXT("Direct template disables Propulsion"), DirectDraft.Projectile.PropulsionConfig.bUsePropulsion);
	TestFalse(TEXT("Direct template disables MissileFlight"), DirectDraft.Projectile.MissileFlightConfig.bUseMissileFlight);
	TestFalse(TEXT("Direct template disables Guidance"), DirectDraft.Projectile.MissileGuideConfig.bUseGuidance);
	TestFalse(TEXT("Direct template disables Heat"), DirectDraft.Weapon.bUseHeat);
	TestFalse(TEXT("Direct template disables Charge"), DirectDraft.Weapon.bUseCharge);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideNamingTest,
	"CarFight.WeaponAuthoring.P001.Naming",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 사용자 BaseAssetName에서 exact child path와 logical ID가 deterministic하게 파생되는지 검증합니다.
bool FCFWeaponGuideNamingTest::RunTest(const FString& Parameters)
{
	// naming 검증용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ViewModel.Reset();
	ViewModel.SetDisposableTestRoot(TEXT("/Game/CarFight/Tests/WeaponGuide/P001"));
	ViewModel.SetBaseAssetName(TEXT("WG_P001_Cannon"));
	ViewModel.SetSuggestedDisplayName(TEXT("P001 테스트 캐논"));

	TestEqual(
		TEXT("Turret object path is deterministic"),
		ViewModel.GetDerivedTurretObjectPath(),
		FString(TEXT("/Game/CarFight/Tests/WeaponGuide/P001/TurretMounts/DA_WG_P001_Cannon_Mount.DA_WG_P001_Cannon_Mount")));
	TestEqual(
		TEXT("Weapon object path is deterministic"),
		ViewModel.GetDerivedWeaponObjectPath(),
		FString(TEXT("/Game/CarFight/Tests/WeaponGuide/P001/WeaponDefs/DA_WG_P001_Cannon_Weapon.DA_WG_P001_Cannon_Weapon")));
	TestEqual(
		TEXT("Projectile object path is deterministic"),
		ViewModel.GetDerivedProjectileObjectPath(),
		FString(TEXT("/Game/CarFight/Tests/WeaponGuide/P001/ProjectileDefs/DA_WG_P001_Cannon_Projectile.DA_WG_P001_Cannon_Projectile")));
	TestEqual(
		TEXT("Damage object path is deterministic"),
		ViewModel.GetDerivedDamageObjectPath(),
		FString(TEXT("/Game/CarFight/Tests/WeaponGuide/P001/DamageDefs/DA_WG_P001_Cannon_Damage.DA_WG_P001_Cannon_Damage")));
	TestEqual(
		TEXT("Ammo object path is deterministic"),
		ViewModel.GetDerivedAmmoObjectPath(),
		FString(TEXT("/Game/CarFight/Tests/WeaponGuide/P001/AmmoDefs/DA_WG_P001_Cannon_Ammo.DA_WG_P001_Cannon_Ammo")));

	TestEqual(TEXT("Turret logical ID is deterministic"), ViewModel.GetDerivedTurretId(), FName(TEXT("WG_P001_Cannon_Mount")));
	TestEqual(TEXT("Weapon logical ID is deterministic"), ViewModel.GetDerivedWeaponId(), FName(TEXT("WG_P001_Cannon")));
	TestEqual(TEXT("Projectile logical ID is deterministic"), ViewModel.GetDerivedProjectileId(), FName(TEXT("WG_P001_Cannon_Projectile")));
	TestEqual(TEXT("Damage logical ID is deterministic"), ViewModel.GetDerivedDamageId(), FName(TEXT("WG_P001_Cannon_Damage")));
	TestEqual(TEXT("Ammo logical ID is deterministic"), ViewModel.GetDerivedAmmoId(), FName(TEXT("WG_P001_Cannon_Ammo")));

	// valid Identity validation 결과입니다.
	TArray<FCFWeaponGuideIssue> ValidIssues;
	TestTrue(TEXT("Valid identity passes"), ViewModel.ValidateStep(ECFWeaponGuideStep::IdentityTemplate, ValidIssues));

	ViewModel.SetBaseAssetName(TEXT("Bad Name"));

	// invalid Identity validation 결과입니다.
	TArray<FCFWeaponGuideIssue> InvalidIssues;
	TestFalse(TEXT("Name containing space fails visibly"), ViewModel.ValidateStep(ECFWeaponGuideStep::IdentityTemplate, InvalidIssues));
	TestTrue(TEXT("Invalid name produces at least one issue"), InvalidIssues.Num() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideCapabilityTest,
	"CarFight.WeaponAuthoring.P001.CapabilityValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Propulsion/Guidance/Heat/Charge의 조건부 validation이 잘못된 값을 fail-visible하는지 검증합니다.
bool FCFWeaponGuideCapabilityTest::RunTest(const FString& Parameters)
{
	// Capability validation용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ViewModel.Reset();

	// Capability 입력을 직접 구성할 transient Draft입니다.
	FCFWeaponGuideDraft& Draft = ViewModel.GetMutableDraft();

	Draft.Projectile.PropulsionConfig.bUsePropulsion = true;
	Draft.Projectile.PropulsionConfig.IgnitionDelaySeconds = 0.0f;
	Draft.Projectile.PropulsionConfig.BurnDurationSeconds = 2.0f;
	Draft.Projectile.PropulsionConfig.ThrustAccelerationCmPerSecSq = 1000.0f;
	Draft.Projectile.PropulsionConfig.MaximumPropelledSpeed = 0.0f;

	// invalid Propulsion validation 결과입니다.
	TArray<FCFWeaponGuideIssue> PropulsionIssues;
	TestFalse(TEXT("Propulsion blocks zero maximum speed"), ViewModel.ValidateStep(ECFWeaponGuideStep::Propulsion, PropulsionIssues));
	Draft.Projectile.PropulsionConfig.MaximumPropelledSpeed = 50000.0f;
	PropulsionIssues.Reset();
	TestTrue(TEXT("Propulsion passes valid values"), ViewModel.ValidateStep(ECFWeaponGuideStep::Propulsion, PropulsionIssues));

	Draft.Projectile.MissileGuideConfig.bUseGuidance = true;
	Draft.Projectile.MissileGuideConfig.GuideMode = ECFMissileGuideMode::None;

	// invalid Guidance validation 결과입니다.
	TArray<FCFWeaponGuideIssue> GuidanceIssues;
	TestFalse(TEXT("Guidance blocks GuideMode None"), ViewModel.ValidateStep(ECFWeaponGuideStep::Guidance, GuidanceIssues));
	Draft.Projectile.MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
	GuidanceIssues.Reset();
	TestTrue(TEXT("Guidance passes default valid core values with TargetActor"), ViewModel.ValidateStep(ECFWeaponGuideStep::Guidance, GuidanceIssues));

	Draft.Weapon.bUseHeat = true;
	Draft.Weapon.HeatPerShot = 10.0f;
	Draft.Weapon.MaxHeat = 100.0f;
	Draft.Weapon.HeatDissipationPerSecond = 0.0f;

	// invalid Heat validation 결과입니다.
	TArray<FCFWeaponGuideIssue> HeatIssues;
	TestFalse(TEXT("Heat blocks zero dissipation"), ViewModel.ValidateStep(ECFWeaponGuideStep::Heat, HeatIssues));
	Draft.Weapon.HeatDissipationPerSecond = 5.0f;
	HeatIssues.Reset();
	TestTrue(TEXT("Heat passes fully positive runtime values"), ViewModel.ValidateStep(ECFWeaponGuideStep::Heat, HeatIssues));

	Draft.Weapon.bUseCharge = true;
	Draft.Weapon.MaximumWeaponCharge = 100.0f;
	Draft.Weapon.InitialWeaponCharge = 100.0f;
	Draft.Weapon.WeaponChargePerShot = 10.0f;
	Draft.Weapon.WeaponChargeRecoveryPerSecond = 0.0f;

	// invalid Charge validation 결과입니다.
	TArray<FCFWeaponGuideIssue> ChargeIssues;
	TestFalse(TEXT("Charge blocks zero recovery"), ViewModel.ValidateStep(ECFWeaponGuideStep::Charge, ChargeIssues));
	Draft.Weapon.WeaponChargeRecoveryPerSecond = 5.0f;
	ChargeIssues.Reset();
	TestTrue(TEXT("Charge passes fully positive runtime values"), ViewModel.ValidateStep(ECFWeaponGuideStep::Charge, ChargeIssues));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideProviderBoundaryTest,
	"CarFight.WeaponAuthoring.P002.ProviderDraftMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Damage/Ammo 신규 Draft가 별도 writer 없이 existing typed provider의 mutation0 mapping 계약을 통과하는지 검증합니다.
bool FCFWeaponGuideProviderBoundaryTest::RunTest(const FString& Parameters)
{
	// provider mapping 검증용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ViewModel.Reset();
	ViewModel.SetDisposableTestRoot(TEXT("/Game/CarFight/Tests/WeaponGuide/P002"));
	ViewModel.SetBaseAssetName(TEXT("WG_P002_Provider"));

	// Damage/Ammo 신규 생성 intent를 가진 transient Draft입니다.
	FCFWeaponGuideDraft& Draft = ViewModel.GetMutableDraft();
	Draft.Damage.Mode = ECFWeaponGuideChildMode::CreateNew;
	Draft.Damage.DamageType = ECFDamageType::Kinetic;
	Draft.Damage.BaseDamage = 125.0f;
	Draft.Damage.ArmorPenetration = 20.0f;
	Draft.Damage.bUseRadialDamage = true;
	Draft.Damage.ExplosionRadius = 450.0f;
	Draft.Damage.ExplosionInnerRadius = 125.0f;
	Draft.Damage.ExplosionDamage = 85.0f;
	Draft.Damage.MinExplosionDamageScale = 0.25f;
	Draft.Damage.ModuleDamageScale = 1.0f;

	// Existing CFDADamageProvider의 serializer/parser/fingerprint round-trip 결과입니다.
	FString DamageProviderSummary;
	// Damage provider mapping 실패 상세입니다.
	FString DamageProviderError;
	TestTrue(
		TEXT("Damage CreateNew maps through existing provider"),
		ViewModel.BuildDamageProviderPreview(DamageProviderSummary, DamageProviderError));
	TestTrue(TEXT("Damage provider preview names existing authority"), DamageProviderSummary.Contains(TEXT("CFDADamageProvider")));

	// Damage Step validation이 provider mapping 성공 뒤 통과하는지 확인합니다.
	TArray<FCFWeaponGuideIssue> DamageIssues;
	TestTrue(TEXT("Damage CreateNew validation passes provider contract"), ViewModel.ValidateStep(ECFWeaponGuideStep::Damage, DamageIssues));

	Draft.Weapon.bUseFiniteAmmo = true;
	Draft.Ammo.Mode = ECFWeaponGuideChildMode::CreateNew;
	Draft.Ammo.AmmoDisplayName = TEXT("P002 Provider Ammo");
	Draft.Ammo.AmmoFamilyId = TEXT("P002Family");
	Draft.Ammo.UnitMassKg = 2.5f;
	Draft.Ammo.AmmoTags = {TEXT("Shell"), TEXT("ProviderMapped")};
	// Engine-owned Texture2D를 사용해 non-null AmmoIcon provider mapping을 Product mutation 없이 검증합니다.
	Draft.Ammo.AmmoIconPath = FSoftObjectPath(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
	Draft.Ammo.MaximumLoadableAmmoCount = 48;

	// Existing CFDAAmmoProvider의 serializer/parser/fingerprint round-trip 결과입니다.
	FString AmmoProviderSummary;
	// Ammo provider mapping 실패 상세입니다.
	FString AmmoProviderError;
	TestTrue(
		TEXT("Ammo CreateNew maps through existing provider"),
		ViewModel.BuildAmmoProviderPreview(AmmoProviderSummary, AmmoProviderError));
	TestTrue(TEXT("Ammo provider preview names existing authority"), AmmoProviderSummary.Contains(TEXT("CFDAAmmoProvider")));

	// Ammo Step validation이 provider mapping 성공 뒤 통과하는지 확인합니다.
	TArray<FCFWeaponGuideIssue> AmmoIssues;
	TestTrue(TEXT("Ammo CreateNew validation passes provider contract"), ViewModel.ValidateStep(ECFWeaponGuideStep::Ammo, AmmoIssues));

	// ProviderDraftMapping 테스트 자체는 explicit Create action을 호출하지 않아 mutation0 경계를 유지합니다.
	TestFalse(TEXT("Provider mapping alone does not produce a completion bundle"), ViewModel.GetResultBundle().bComplete);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideDurableGraphTest,
	"CarFight.WeaponAuthoring.P003.DurableCreateGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Damage/Ammo provider exact2와 Guide-local exact3을 하나의 disposable graph로 durable 생성하고 exact reference readback을 검증합니다.
bool FCFWeaponGuideDurableGraphTest::RunTest(const FString& Parameters)
{
	// 시작 residue exact0 검증 오류입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("P003 durable pre-clean leaves residue exact0"), CleanupP003FixtureRoot(CleanupError)))
	{
		AddError(CleanupError);
		return true;
	}

	// Full exact5 durable graph용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ConfigureP003CreateDraft(ViewModel, TEXT("Full"), TEXT("WG_P003_Full"));

	// Mutation 전 전체 Draft validation 진단입니다.
	TArray<FCFWeaponGuideIssue> ValidationIssues;
	TestTrue(TEXT("P003 full Draft validation passes"), ViewModel.ValidateAll(ValidationIssues));

	// exact5 durable Create 실패 상세입니다.
	FString CreateError;
	const bool bCreated = ViewModel.CreateChildrenAndBuildResult(CreateError);
	TestTrue(TEXT("P003 exact5 durable Create succeeds"), bCreated);
	if (!bCreated)
	{
		AddError(CreateError);
	}
	else
	{
		// Exact graph readback 뒤 확정된 completion Bundle입니다.
		const FCFWeaponGuideResultBundle& Bundle = ViewModel.GetResultBundle();
		TestTrue(TEXT("P003 completion bundle is complete"), Bundle.bComplete);
		TestEqual(TEXT("P003 Damage path exact"), Bundle.DamageDataPath.ToString(), ViewModel.GetDerivedDamageObjectPath());
		TestEqual(TEXT("P003 Ammo path exact"), Bundle.AmmoDataPath.ToString(), ViewModel.GetDerivedAmmoObjectPath());
		TestEqual(TEXT("P003 Turret path exact"), Bundle.TurretMountDataPath.ToString(), ViewModel.GetDerivedTurretObjectPath());
		TestEqual(TEXT("P003 Projectile path exact"), Bundle.ProjectileDataPath.ToString(), ViewModel.GetDerivedProjectileObjectPath());
		TestEqual(TEXT("P003 Weapon path exact"), Bundle.WeaponDataPath.ToString(), ViewModel.GetDerivedWeaponObjectPath());
		TestFalse(TEXT("P003 bundle fingerprint exists"), Bundle.DeterministicBundleFingerprint.IsEmpty());

		// Persisted exact ProjectileData입니다.
		const UCFProjectileData* ProjectileData = LoadObject<UCFProjectileData>(nullptr, *Bundle.ProjectileDataPath.ToString());
		// Persisted exact WeaponData입니다.
		const UCFWeaponData* WeaponData = LoadObject<UCFWeaponData>(nullptr, *Bundle.WeaponDataPath.ToString());
		TestNotNull(TEXT("P003 persisted ProjectileData loads"), ProjectileData);
		TestNotNull(TEXT("P003 persisted WeaponData loads"), WeaponData);
		if (ProjectileData != nullptr)
		{
			TestEqual(
				TEXT("P003 Projectile->Damage exact reference"),
				FSoftObjectPath(ProjectileData->DefaultDamageData.Get()).ToString(),
				Bundle.DamageDataPath.ToString());
		}
		if (WeaponData != nullptr)
		{
			TestEqual(
				TEXT("P003 Weapon->Projectile exact reference"),
				FSoftObjectPath(WeaponData->DefaultProjectileData.Get()).ToString(),
				Bundle.ProjectileDataPath.ToString());
			TestEqual(
				TEXT("P003 Weapon->Ammo exact reference"),
				FSoftObjectPath(WeaponData->DefaultAmmoData.Get()).ToString(),
				Bundle.AmmoDataPath.ToString());
		}
	}

	// 종료 residue exact0 검증 오류입니다.
	FString TeardownError;
	if (!TestTrue(TEXT("P003 durable teardown leaves residue exact0"), CleanupP003FixtureRoot(TeardownError)))
	{
		AddError(TeardownError);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuidePartialRecoveryTest,
	"CarFight.WeaponAuthoring.P003.PartialRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Damage+Ammo+Turret confirmed durable 직후 interruption에서 stale Draft를 막고 같은 ViewModel retry로 나머지 graph를 완성하는지 검증합니다.
bool FCFWeaponGuidePartialRecoveryTest::RunTest(const FString& Parameters)
{
	// 시작 residue exact0 검증 오류입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("P003 recovery pre-clean leaves residue exact0"), CleanupP003FixtureRoot(CleanupError)))
	{
		AddError(CleanupError);
		return true;
	}

	// Partial durable recovery용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ConfigureP003CreateDraft(ViewModel, TEXT("Recovery"), TEXT("WG_P003_Recovery"));
	ViewModel.SetDurableInterruptionStepForTests(ECFWeaponGuideStep::MountGeometry);

	// Confirmed Damage+Ammo+Turret durable 직후 intentional interruption 결과입니다.
	FString FirstCreateError;
	TestFalse(TEXT("P003 intentional interruption stops first Create"), ViewModel.CreateChildrenAndBuildResult(FirstCreateError));
	TestTrue(TEXT("P003 interruption is fail-visible"), FirstCreateError.Contains(TEXT("Automation")));

	// 첫 시도에서 durable 확인되어야 하는 DamageData입니다.
	UCFDamageData* PersistedDamage = LoadObject<UCFDamageData>(nullptr, *ViewModel.GetDerivedDamageObjectPath());
	// 첫 시도에서 durable 확인되어야 하는 AmmoData입니다.
	UCFAmmoData* PersistedAmmo = LoadObject<UCFAmmoData>(nullptr, *ViewModel.GetDerivedAmmoObjectPath());
	// 첫 시도에서 durable 확인되어야 하는 TurretMountData입니다.
	UCFTurretMountData* PersistedTurret = LoadObject<UCFTurretMountData>(nullptr, *ViewModel.GetDerivedTurretObjectPath());
	TestNotNull(TEXT("P003 interruption preserves confirmed DamageData"), PersistedDamage);
	TestNotNull(TEXT("P003 interruption preserves confirmed AmmoData"), PersistedAmmo);
	TestNotNull(TEXT("P003 interruption preserves confirmed TurretMountData"), PersistedTurret);

	// 이미 durable 저장한 Turret과 다른 current Draft를 의도적으로 만들어 stale retry를 검증합니다.
	FCFWeaponGuideDraft& MutableDraft = ViewModel.GetMutableDraft();
	MutableDraft.Turret.MaxYawDeg = 90.0f;
	// Stale retry 차단 상세입니다.
	FString StaleRetryError;
	TestFalse(TEXT("P003 stale same-session Turret Draft is blocked"), ViewModel.CreateChildrenAndBuildResult(StaleRetryError));
	TestTrue(TEXT("P003 stale durable mismatch is fail-visible"), StaleRetryError.Contains(TEXT("TurretMountData")));

	// Durable 저장 당시 authored 값으로 복원하고 intentional interruption도 해제합니다.
	MutableDraft.Turret.MaxYawDeg = 180.0f;
	ViewModel.ClearDurableInterruptionForTests();

	// 같은 ViewModel safe retry 결과입니다.
	FString RetryError;
	const bool bRetrySucceeded = ViewModel.CreateChildrenAndBuildResult(RetryError);
	TestTrue(TEXT("P003 same-session retry completes remaining graph"), bRetrySucceeded);
	if (!bRetrySucceeded)
	{
		AddError(RetryError);
	}
	else
	{
		// Recovery 뒤 확정된 exact completion Bundle입니다.
		const FCFWeaponGuideResultBundle& Bundle = ViewModel.GetResultBundle();
		TestTrue(TEXT("P003 recovery bundle is complete"), Bundle.bComplete);
		TestEqual(TEXT("P003 recovery reuses same Damage path"), Bundle.DamageDataPath.ToString(), ViewModel.GetDerivedDamageObjectPath());
		TestEqual(TEXT("P003 recovery reuses same Ammo path"), Bundle.AmmoDataPath.ToString(), ViewModel.GetDerivedAmmoObjectPath());
		TestEqual(TEXT("P003 recovery reuses same Turret path"), Bundle.TurretMountDataPath.ToString(), ViewModel.GetDerivedTurretObjectPath());
		TestEqual(TEXT("P003 recovery completes Weapon path"), Bundle.WeaponDataPath.ToString(), ViewModel.GetDerivedWeaponObjectPath());
	}

	// 종료 residue exact0 검증 오류입니다.
	FString TeardownError;
	if (!TestTrue(TEXT("P003 recovery teardown leaves residue exact0"), CleanupP003FixtureRoot(TeardownError)))
	{
		AddError(TeardownError);
	}
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponGuideCompletionHandoffTest,
	"CarFight.WeaponAuthoring.P004.CompletionHandoff",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Completion Bundle USER summary와 navigation-only Equipment Builder handoff가 P0-03 fingerprint를 보존하는지 검증합니다.
bool FCFWeaponGuideCompletionHandoffTest::RunTest(const FString& Parameters)
{
	// 시작 residue exact0 검증 오류입니다.
	FString CleanupError;
	if (!TestTrue(TEXT("P004 handoff pre-clean leaves residue exact0"), CleanupP003FixtureRoot(CleanupError)))
	{
		AddError(CleanupError);
		return true;
	}

	// Navigation-only handoff가 가리키는 기존 Equipment Builder Nomad Tab identity입니다.
	const FName EquipmentBuilderTabId(TEXT("CarFight.EquipmentBuilder"));
	TestTrue(
		TEXT("P004 Equipment Builder Nomad Tab spawner is registered"),
		FGlobalTabmanager::Get()->HasTabSpawner(EquipmentBuilderTabId));

	// Completion/handoff 검증용 session-local ViewModel입니다.
	FCFWeaponGuideVM ViewModel;
	ConfigureP003CreateDraft(ViewModel, TEXT("P004Handoff"), TEXT("WG_P004_Handoff"));
	ViewModel.SetSuggestedDisplayName(TEXT("P004 Handoff Weapon"));

	// 완료 전 USER summary는 fail-closed되어야 합니다.
	FString PrematureSummary;
	// 완료 전 summary 실패 상세입니다.
	FString PrematureError;
	TestFalse(
		TEXT("P004 summary is blocked before completion"),
		ViewModel.BuildCompletionUserSummary(PrematureSummary, PrematureError));

	// 완료 전 handoff 계약도 fail-closed되어야 합니다.
	FCFWeaponGuideEquipmentHandoff PrematureHandoff;
	// 완료 전 handoff 실패 상세입니다.
	FString PrematureHandoffError;
	TestFalse(
		TEXT("P004 handoff is blocked before completion"),
		ViewModel.BuildEquipmentBuilderHandoff(PrematureHandoff, PrematureHandoffError));

	// exact5 durable graph + completion Bundle 생성 실패 상세입니다.
	FString CreateError;
	const bool bCreated = ViewModel.CreateChildrenAndBuildResult(CreateError);
	TestTrue(TEXT("P004 exact5 completion graph succeeds"), bCreated);
	if (!bCreated)
	{
		AddError(CreateError);
	}
	else
	{
		// P0-03에서 확정된 completion Bundle입니다.
		const FCFWeaponGuideResultBundle& Bundle = ViewModel.GetResultBundle();
		// USER summary/handoff 전 보존할 deterministic fingerprint입니다.
		const FString OriginalFingerprint = Bundle.DeterministicBundleFingerprint;
		TestTrue(TEXT("P004 completion bundle is complete"), Bundle.bComplete);
		TestFalse(TEXT("P004 completion fingerprint exists"), OriginalFingerprint.IsEmpty());

		// 사용자 중심 완료 요약입니다.
		FString UserSummary;
		// USER summary 생성 실패 상세입니다.
		FString SummaryError;
		TestTrue(
			TEXT("P004 completion USER summary builds"),
			ViewModel.BuildCompletionUserSummary(UserSummary, SummaryError));
		TestTrue(TEXT("P004 summary shows suggested display name"), UserSummary.Contains(TEXT("P004 Handoff Weapon")));
		TestTrue(TEXT("P004 summary shows mount requirement"), UserSummary.Contains(TEXT("장착 요구")));
		TestTrue(TEXT("P004 summary explains next Equipment Builder task"), UserSummary.Contains(TEXT("장비 제작 가이드")));
		TestTrue(TEXT("P004 summary explains no automatic context transfer"), UserSummary.Contains(TEXT("자동 전달")));

		// Navigation-only Equipment Builder handoff 계약입니다.
		FCFWeaponGuideEquipmentHandoff Handoff;
		// Handoff 생성 실패 상세입니다.
		FString HandoffError;
		TestTrue(
			TEXT("P004 navigation-only handoff builds"),
			ViewModel.BuildEquipmentBuilderHandoff(Handoff, HandoffError));
		TestEqual(TEXT("P004 target tab id exact"), Handoff.TargetTabId, EquipmentBuilderTabId);
		TestTrue(TEXT("P004 handoff is navigation-only"), Handoff.bNavigationOnly);
		TestFalse(TEXT("P004 handoff does not inject context"), Handoff.bInjectContext);
		TestEqual(TEXT("P004 source fingerprint exact"), Handoff.SourceBundleFingerprint, OriginalFingerprint);
		TestEqual(
			TEXT("P004 summary/handoff preserve completion fingerprint"),
			ViewModel.GetResultBundle().DeterministicBundleFingerprint,
			OriginalFingerprint);

		// 완료 뒤 Draft를 변조해 stale handoff integrity 차단을 검증합니다.
		ViewModel.GetMutableDraft().SuggestedDisplayName = TEXT("P004 Stale DisplayName");
		// stale handoff 계약입니다.
		FCFWeaponGuideEquipmentHandoff StaleHandoff;
		// stale handoff 차단 상세입니다.
		FString StaleError;
		TestFalse(
			TEXT("P004 stale Draft blocks handoff"),
			ViewModel.BuildEquipmentBuilderHandoff(StaleHandoff, StaleError));
		TestTrue(TEXT("P004 stale handoff reports fingerprint mismatch"), StaleError.Contains(TEXT("fingerprint")));

		// 원래 DisplayName으로 복원해 completion Bundle identity를 다시 일치시킵니다.
		ViewModel.GetMutableDraft().SuggestedDisplayName = TEXT("P004 Handoff Weapon");

		// P0-03 Bundle fingerprint에는 직접 포함되지 않던 FireMode를 변경해 P0-04 semantic integrity를 검증합니다.
		const ECFWeaponFireMode OriginalFireMode = ViewModel.GetDraft().Weapon.FireMode;
		ViewModel.GetMutableDraft().Weapon.FireMode = OriginalFireMode == ECFWeaponFireMode::Projectile
			? ECFWeaponFireMode::HitScan
			: ECFWeaponFireMode::Projectile;
		FCFWeaponGuideEquipmentHandoff FireModeStaleHandoff;
		FString FireModeStaleError;
		TestFalse(
			TEXT("P004 stale FireMode blocks handoff outside P0-03 bundle fingerprint"),
			ViewModel.BuildEquipmentBuilderHandoff(FireModeStaleHandoff, FireModeStaleError));
		TestEqual(
			TEXT("P004 FireMode stale check does not rewrite P0-03 bundle fingerprint"),
			ViewModel.GetResultBundle().DeterministicBundleFingerprint,
			OriginalFingerprint);
		ViewModel.GetMutableDraft().Weapon.FireMode = OriginalFireMode;

		// Launcher intent는 persisted Weapon payload의 별도 flag가 아니므로 P0-04 semantic snapshot이 직접 보호해야 합니다.
		const bool bOriginalUseLauncher = ViewModel.GetDraft().Weapon.bUseLauncher;
		ViewModel.GetMutableDraft().Weapon.bUseLauncher = !bOriginalUseLauncher;
		FCFWeaponGuideEquipmentHandoff LauncherStaleHandoff;
		FString LauncherStaleError;
		TestFalse(
			TEXT("P004 stale Launcher intent blocks handoff"),
			ViewModel.BuildEquipmentBuilderHandoff(LauncherStaleHandoff, LauncherStaleError));
		ViewModel.GetMutableDraft().Weapon.bUseLauncher = bOriginalUseLauncher;

		// Projectile Guidance capability도 P0-04 semantic integrity가 보호해야 합니다.
		const bool bOriginalUseGuidance = ViewModel.GetDraft().Projectile.MissileGuideConfig.bUseGuidance;
		ViewModel.GetMutableDraft().Projectile.MissileGuideConfig.bUseGuidance = !bOriginalUseGuidance;
		FCFWeaponGuideEquipmentHandoff GuidanceStaleHandoff;
		FString GuidanceStaleError;
		TestFalse(
			TEXT("P004 stale Guidance capability blocks handoff"),
			ViewModel.BuildEquipmentBuilderHandoff(GuidanceStaleHandoff, GuidanceStaleError));
		ViewModel.GetMutableDraft().Projectile.MissileGuideConfig.bUseGuidance = bOriginalUseGuidance;

		// 모든 completion semantic을 원래 값으로 복원한 뒤 navigation-only handoff가 다시 성립해야 합니다.
		FCFWeaponGuideEquipmentHandoff RestoredHandoff;
		FString RestoredError;
		TestTrue(
			TEXT("P004 restored Draft rebuilds navigation-only handoff"),
			ViewModel.BuildEquipmentBuilderHandoff(RestoredHandoff, RestoredError));
		TestEqual(TEXT("P004 restored fingerprint exact"), RestoredHandoff.SourceBundleFingerprint, OriginalFingerprint);
	}

	// 종료 residue exact0 검증 오류입니다.
	FString TeardownError;
	if (!TestTrue(TEXT("P004 handoff teardown leaves residue exact0"), CleanupP003FixtureRoot(TeardownError)))
	{
		AddError(TeardownError);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
