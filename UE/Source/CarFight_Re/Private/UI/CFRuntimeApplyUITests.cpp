// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.1
// Date: 2026-09-03
// Description: CF-FQ-041 Runtime Apply UI + CF-FQ-044 Catalog option synchronization 자동화 테스트
// Scope: RuntimeApply Navigation/Apply 회귀와 isolated Catalog exact-sequence change-aware cache synchronization을 검증합니다.
// Changelog:
// - v1.1.1: Mid-review P1/P2. CatalogOptionSync가 Product Default Catalog 개수에 의존하지 않도록 test-owned transient Vehicle/Equipment fixture를 사용하고, raw null entry가 filtered cache를 반복 invalidation하지 않는 상태를 추가 검증.
// - v1.1.0: VRCP-P0-03 isolated transient Catalog에서 Vehicle/Equipment add 및 same-count reorder를 refresh가 감지하고, rebuild 뒤 exact 선택 identity를 보존하는 회귀를 추가. Product Default Catalog mutation/save 0.
// - v1.0.6: Applied Snapshot이 없는 Legacy Runtime에서 VehicleWeaponComp의 실제 활성 Equipment가 Current Equipment UI에 표시되는 회귀를 추가.
// - v1.0.5: exact Catalog authorization과 downstream Fitting candidate validation을 분리 검증. 현재 Catalog 장비가 필수 질량 계약을 만족하지 못하면 ValidationFailed도 정상 fail-closed 결과로 인정하고 이전 Current/Fitting source 보존과 UI 상태 표시를 검증.
// - v1.0.4: P0-04 단계 경계에 맞춰 Equipment UI 검증을 persisted valid Snapshot + exact Catalog candidate 연결 검증으로 교정. Catalog 장비 데이터가 현재 Fitting 질량 계약을 만족하면 성공 상태/Current 갱신을, 아직 미충족이면 ApplyFailed/기존 Current 보존을 모두 검증하고 P0-05 E2E 데이터 readiness와 분리.
// - v1.0.3: Equipment frontend 성공 검증을 Catalog 밖 persisted Fitting에서 분리. valid mass/mount VehicleData인 DefenseTestSUV에 Vehicle default RocketLauncher를 해석하는 transient Fitting fixture를 구성하고, 다른 Catalog 장비 선택 시 Current 불변 → exact default Catalog 장비 Explicit Apply 성공을 검증.
// - v1.0.2: Equipment UI fixture가 기존 출격 Ammo 계약을 보존하도록 현재 Applied Equipment와 같은 DefaultWeaponData를 사용하는 compatible exact Catalog Equipment만 후보로 선택.
// - v1.0.1: Equipment UI fixture를 현재 Applied Equipment exact identity 가정에서 현재 Mount에 호환되는 exact Catalog Equipment 선택 방식으로 교정해 Catalog authorization 계약을 보존.
// - v1.0.0: PanelNavigation, ExplicitVehicleApply, ExplicitEquipmentApply 3개 focused test를 최초 추가.
// Migration:
// - 테스트는 순수 C++ RuntimeApply Widget을 사용하며 WBP Asset을 생성·수정하지 않습니다.
// - 실제 BP_CFVehiclePawn과 persisted Runtime Catalog/SUV Fitting fixture는 읽기 전용으로 사용합니다.

#include "UI/CFRuntimeApplyWidget.h"
#include "UI/CFVehicleDebugPanelWidget.h"

#include "CFEquipmentPresetData.h"
#include "CFRuntimeEquipApply.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// [v1.0.0] 실제 BP_CFVehiclePawn을 지정 VehicleData + optional FittingData 상태로 생성하고 Runtime 초기화합니다.
	bool BuildRuntimeApplyUiPawn(
		FAutomationTestBase& Test,
		UWorld* TestWorld,
		UCFVehicleData* VehicleData,
		UCFVehicleFittingData* VehicleFittingData,
		ACFVehiclePawn*& OutVehiclePawn)
	{
		OutVehiclePawn = nullptr;

		// [v1.0.0] 실제 Blueprint 컴포넌트 계층을 가진 VehiclePawn class입니다.
		UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
			nullptr,
			TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));

		if (!Test.TestNotNull(TEXT("RTA-P0-04 BP_CFVehiclePawn class"), VehiclePawnClass)
			|| !Test.TestNotNull(TEXT("RTA-P0-04 fixture VehicleData"), VehicleData))
		{
			return false;
		}

		// [v1.0.0] 테스트 Pawn을 다른 actor와 충돌 없이 생성할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// [v1.0.0] Runtime Apply UI의 실제 mutation/readback 대상 Pawn입니다.
		ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
			VehiclePawnClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Test.TestNotNull(TEXT("RTA-P0-04 VehiclePawn spawn"), VehiclePawn))
		{
			return false;
		}

		VehiclePawn->UnregisterAllComponents();
		VehiclePawn->VehicleData = VehicleData;
		VehiclePawn->VehicleFittingData = VehicleFittingData;
		VehiclePawn->bAutoRegisterInputMappingContext = false;
		VehiclePawn->bShowAimReticle = false;
		VehiclePawn->bShowTargetSelectHud = false;

		// [v1.0.0] fixture Runtime을 매 테스트 fresh 상태로 시작할 Fitting component입니다.
		UCFVehicleFittingComp* VehicleFittingComp =
			VehiclePawn->GetVehicleFittingComp();
		if (!Test.TestNotNull(TEXT("RTA-P0-04 VehicleFittingComp"), VehicleFittingComp))
		{
			VehiclePawn->Destroy();
			return false;
		}

		VehicleFittingComp->ResetFittingRuntimeState();
		VehiclePawn->RegisterAllComponents();

		if (!Test.TestTrue(
			TEXT("RTA-P0-04 initial Vehicle Runtime initialize"),
			VehiclePawn->InitializeVehicleRuntime()))
		{
			VehiclePawn->Destroy();
			return false;
		}

		OutVehiclePawn = VehiclePawn;
		return true;
	}

	// [v1.0.0] Runtime Apply UI test와 같은 packaged-safe 기본 Catalog를 로드합니다.
	UCFRuntimeTestCatalogData* LoadRuntimeApplyUiCatalog(
		FAutomationTestBase& Test)
	{
		// [v1.0.0] Config soft reference loader를 소유하는 Runtime settings CDO입니다.
		const UCFRuntimeTestSettings* RuntimeTestSettings =
			GetDefault<UCFRuntimeTestSettings>();

		// [v1.0.0] 실제 RuntimeApply UI가 사용하는 persisted selection Catalog입니다.
		UCFRuntimeTestCatalogData* RuntimeCatalog =
			RuntimeTestSettings
				? RuntimeTestSettings->LoadDefaultCatalog()
				: nullptr;

		Test.TestNotNull(TEXT("RTA-P0-04 default Runtime Catalog"), RuntimeCatalog);
		if (RuntimeCatalog)
		{
			Test.TestTrue(
				TEXT("RTA-P0-04 default Runtime Catalog usable"),
				RuntimeCatalog->IsRuntimeTestCatalogUsable());
		}
		return RuntimeCatalog;
	}

	// [v1.0.0] RuntimeApply Widget의 Vehicle option에서 exact VehicleData index를 찾아 선택합니다.
	bool SelectExactVehicle(
		UCFRuntimeApplyWidget* RuntimeApplyWidget,
		UCFVehicleData* ExpectedVehicleData)
	{
		if (!RuntimeApplyWidget || !ExpectedVehicleData)
		{
			return false;
		}

		for (int32 VehicleIndex = 0;
			VehicleIndex < RuntimeApplyWidget->GetVehicleOptionCount();
			++VehicleIndex)
		{
			if (RuntimeApplyWidget->SelectVehicleByIndex(VehicleIndex)
				&& RuntimeApplyWidget->GetSelectedVehicleData() == ExpectedVehicleData)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] RuntimeApply Widget의 Mount option에서 exact MountProfileId index를 찾아 선택합니다.
	bool SelectExactMount(
		UCFRuntimeApplyWidget* RuntimeApplyWidget,
		const FName ExpectedMountProfileId)
	{
		if (!RuntimeApplyWidget || ExpectedMountProfileId.IsNone())
		{
			return false;
		}

		for (int32 MountIndex = 0;
			MountIndex < RuntimeApplyWidget->GetMountOptionCount();
			++MountIndex)
		{
			if (RuntimeApplyWidget->SelectMountByIndex(MountIndex)
				&& RuntimeApplyWidget->GetSelectedMountProfileId() == ExpectedMountProfileId)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] RuntimeApply Widget의 Equipment option에서 exact EquipmentPresetData index를 찾아 선택합니다.
	bool SelectExactEquipment(
		UCFRuntimeApplyWidget* RuntimeApplyWidget,
		UCFEquipmentPresetData* ExpectedEquipmentPresetData)
	{
		if (!RuntimeApplyWidget || !ExpectedEquipmentPresetData)
		{
			return false;
		}

		for (int32 EquipmentIndex = 0;
			EquipmentIndex < RuntimeApplyWidget->GetEquipmentOptionCount();
			++EquipmentIndex)
		{
			if (RuntimeApplyWidget->SelectEquipmentByIndex(EquipmentIndex)
				&& RuntimeApplyWidget->GetSelectedEquipmentData() == ExpectedEquipmentPresetData)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] 현재 Applied Snapshot에서 exact Equipment가 존재하는 첫 Mount를 찾습니다.
	bool FindFirstAppliedEquipment(
		const UCFVehicleFittingComp* VehicleFittingComp,
		FName& OutMountProfileId,
		UCFEquipmentPresetData*& OutEquipmentPresetData)
	{
		OutMountProfileId = NAME_None;
		OutEquipmentPresetData = nullptr;

		if (!VehicleFittingComp
			|| !VehicleFittingComp->HasAppliedFittingSnapshot())
		{
			return false;
		}

		// [v1.0.0] 현재 실제 Fitting runtime이 Commit한 장비 readback Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot =
			VehicleFittingComp->GetAppliedFittingSnapshot();

		for (const FCFResolvedFittingMount& ResolvedMount :
			AppliedFittingSnapshot.ResolvedMounts)
		{
			if (!ResolvedMount.MountProfileId.IsNone()
				&& IsValid(ResolvedMount.EquipmentPresetData))
			{
				OutMountProfileId = ResolvedMount.MountProfileId;
				OutEquipmentPresetData = ResolvedMount.EquipmentPresetData;
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeApplyPanelNavTest,
	"CarFight.RuntimeApply.RTA_P0_04.PanelNavigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] VehicleDebug Panel ViewData에 RuntimeApply가 독립 Navigation Section으로 등록되는지 검증합니다.
bool FCFRuntimeApplyPanelNavTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] actual BP Pawn과 C++ Panel을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-04 PanelNavigation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Vehicle pre-register/runtime 계약을 실제 Game World 의미로 실행할 guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// [v1.0.0] 실제 packaged selection source인 기본 Runtime Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog =
		LoadRuntimeApplyUiCatalog(*this);
	if (!RuntimeCatalog
		|| !TestTrue(
			TEXT("RTA-P0-04 PanelNavigation Vehicle option exists"),
			!RuntimeCatalog->AllowedVehicleData.IsEmpty()))
	{
		return false;
	}

	// [v1.0.0] Panel debug source로 사용할 첫 Catalog VehicleData입니다.
	UCFVehicleData* InitialVehicleData =
		RuntimeCatalog->AllowedVehicleData[0].Get();

	// [v1.0.0] Panel이 Snapshot을 읽을 actual VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;
	if (!BuildRuntimeApplyUiPawn(
		*this,
		TestWorld,
		InitialVehicleData,
		nullptr,
		VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] WBP 없이 C++ ViewData/Navigation 계약만 검증할 VehicleDebug Panel입니다.
	UCFVehicleDebugPanelWidget* VehicleDebugPanel =
		CreateWidget<UCFVehicleDebugPanelWidget>(
			TestWorld,
			UCFVehicleDebugPanelWidget::StaticClass());
	if (!TestNotNull(TEXT("RTA-P0-04 C++ VehicleDebug Panel"), VehicleDebugPanel))
	{
		VehiclePawn->Destroy();
		return false;
	}

	VehicleDebugPanel->SetVehiclePawnRef(VehiclePawn);

	// [v1.0.0] 최신 Panel Snapshot에서 RuntimeApply Section 존재 여부를 검사할 ViewData입니다.
	const FCFVehicleDebugPanelViewData& PanelViewData =
		VehicleDebugPanel->GetCachedPanelViewData();

	// [v1.0.0] RuntimeApply Section의 exact ID/title/group을 검증할 발견 상태입니다.
	bool bFoundRuntimeApplySection = false;
	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& SectionViewData :
		PanelViewData.TopLevelSectionArray)
	{
		if (SectionViewData.IsValid()
			&& SectionViewData->SectionId == TEXT("RuntimeApply"))
		{
			bFoundRuntimeApplySection = true;
			TestEqual(
				TEXT("RuntimeApply Navigation title"),
				SectionViewData->TitleText,
				FString(TEXT("런타임 적용")));
			TestEqual(
				TEXT("RuntimeApply Navigation group"),
				SectionViewData->NavigationGroup,
				ECFVehicleDebugNavGroup::Diagnostics);
			TestTrue(
				TEXT("RuntimeApply Navigation visible"),
				SectionViewData->bShowInNavigation);
			break;
		}
	}

	TestTrue(
		TEXT("RuntimeApply top-level Navigation Section exists"),
		bFoundRuntimeApplySection);

	VehicleDebugPanel->SelectSectionById(TEXT("RuntimeApply"));
	TestEqual(
		TEXT("RuntimeApply Section ID can be selected"),
		VehicleDebugPanel->GetSelectedSectionId(),
		FString(TEXT("RuntimeApply")));

	VehicleDebugPanel->SelectSectionById(TEXT("Overview"));
	TestEqual(
		TEXT("Generic Section can be re-selected after RuntimeApply"),
		VehicleDebugPanel->GetSelectedSectionId(),
		FString(TEXT("Overview")));

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeApplyUiVehicleTest,
	"CarFight.RuntimeApply.RTA_P0_04.ExplicitVehicleApply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Selected Vehicle 변경만으로 Current가 변하지 않고 Explicit Apply에서만 service mutation이 발생하는지 검증합니다.
bool FCFRuntimeApplyUiVehicleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] same-Pawn Vehicle UI Apply를 실제 Runtime으로 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-04 VehicleApply World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 실제 Vehicle component register/init 계약을 활성화할 Game World guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// [v1.0.0] RuntimeApply UI와 동일 Config 경로에서 로드한 persisted Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog =
		LoadRuntimeApplyUiCatalog(*this);
	if (!RuntimeCatalog
		|| !TestTrue(
			TEXT("RTA-P0-04 Vehicle catalog has two options"),
			RuntimeCatalog->AllowedVehicleData.Num() >= 2))
	{
		return false;
	}

	// [v1.0.0] UI Apply 전 Current Vehicle로 유지할 첫 Catalog VehicleData입니다.
	UCFVehicleData* InitialVehicleData =
		RuntimeCatalog->AllowedVehicleData[0].Get();

	// [v1.0.0] UI Selected Vehicle로만 먼저 바꿀 두 번째 Catalog VehicleData입니다.
	UCFVehicleData* CandidateVehicleData =
		RuntimeCatalog->AllowedVehicleData[1].Get();

	// [v1.0.0] UI Explicit Apply 대상 actual VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;
	if (!BuildRuntimeApplyUiPawn(
		*this,
		TestWorld,
		InitialVehicleData,
		nullptr,
		VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] WBP Asset 없이 동일 frontend 계약을 검증할 pure C++ RuntimeApply Widget입니다.
	UCFRuntimeApplyWidget* RuntimeApplyWidget =
		CreateWidget<UCFRuntimeApplyWidget>(
			TestWorld,
			UCFRuntimeApplyWidget::StaticClass());
	if (!TestNotNull(TEXT("RTA-P0-04 RuntimeApply Widget"), RuntimeApplyWidget))
	{
		VehiclePawn->Destroy();
		return false;
	}

	RuntimeApplyWidget->SetVehiclePawnRef(VehiclePawn);

	TestEqual(
		TEXT("RuntimeApply vehicle option count matches Catalog"),
		RuntimeApplyWidget->GetVehicleOptionCount(),
		RuntimeCatalog->AllowedVehicleData.Num());

	TestTrue(
		TEXT("Candidate Vehicle can be selected"),
		SelectExactVehicle(RuntimeApplyWidget, CandidateVehicleData));

	// [v1.0.0] Explicit Apply 전 exact Current Vehicle identity입니다.
	UCFVehicleData* CurrentVehicleBeforeApply =
		VehiclePawn->VehicleData.Get();

	TestTrue(
		TEXT("Selected Vehicle differs from Current before Apply"),
		RuntimeApplyWidget->GetSelectedVehicleData() != CurrentVehicleBeforeApply);
	TestTrue(
		TEXT("Selection alone does not mutate Current Vehicle"),
		VehiclePawn->VehicleData.Get() == InitialVehicleData);

	// [v1.0.0] UI public action이 P0-02 Runtime service에 위임한 최종 Vehicle status입니다.
	const ECFRuntimeVehicleApplyStatus ApplyStatus =
		RuntimeApplyWidget->ApplySelectedVehicle();

	TestEqual(
		TEXT("Explicit Vehicle UI Apply succeeds"),
		ApplyStatus,
		ECFRuntimeVehicleApplyStatus::Succeeded);
	TestTrue(
		TEXT("Vehicle Current becomes transient only after Apply"),
		VehiclePawn->VehicleData
			&& VehiclePawn->VehicleData->HasAnyFlags(RF_Transient));
	TestTrue(
		TEXT("Vehicle Current is not persistent candidate pointer"),
		VehiclePawn->VehicleData.Get() != CandidateVehicleData);
	TestTrue(
		TEXT("Selected persistent candidate remains separate"),
		RuntimeApplyWidget->GetSelectedVehicleData() == CandidateVehicleData);
	TestTrue(
		TEXT("Last Result reports success"),
		RuntimeApplyWidget->GetLastResultText().Contains(TEXT("성공")));

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeApplyUiEquipmentTest,
	"CarFight.RuntimeApply.RTA_P0_04.ExplicitEquipmentApply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.4] Mount/Equipment 선택만으로 Current 장비가 변하지 않고 Explicit Apply 결과가 Runtime service 상태와 Current readback에 정확히 반영되는지 검증합니다.
bool FCFRuntimeApplyUiEquipmentTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.4] persisted valid Snapshot Runtime을 실제로 구성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-04 EquipmentApply World"), TestWorld))
	{
		return false;
	}

	// [v1.0.4] Snapshot Initial Mass/Weapon/Defense Runtime을 실제 Game World 의미로 구성할 guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// [v1.0.4] UI exact Equipment authorization에 사용할 실제 persisted Runtime Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog =
		LoadRuntimeApplyUiCatalog(*this);
	if (!RuntimeCatalog)
	{
		return false;
	}

	// [v1.0.4] 실제 Fitting mass/mount 계약을 만족하는 persisted SUV VehicleData입니다.
	UCFVehicleData* VehicleData = LoadObject<UCFVehicleData>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"));

	// [v1.0.4] P0-03에서 이미 성공/복구 Runtime fixture로 검증된 persisted FittingData입니다.
	UCFVehicleFittingData* VehicleFittingData = LoadObject<UCFVehicleFittingData>(
		nullptr,
		TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV"));

	if (!TestNotNull(TEXT("RTA-P0-04 Equipment fixture VehicleData"), VehicleData)
		|| !TestNotNull(TEXT("RTA-P0-04 Equipment fixture FittingData"), VehicleFittingData))
	{
		return false;
	}

	// [v1.0.4] Equipment UI Apply 대상 actual Snapshot VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;
	if (!BuildRuntimeApplyUiPawn(
		*this,
		TestWorld,
		VehicleData,
		VehicleFittingData,
		VehiclePawn))
	{
		return false;
	}

	// [v1.0.4] Current Equipment readback을 제공할 actual Fitting component입니다.
	UCFVehicleFittingComp* VehicleFittingComp =
		VehiclePawn->GetVehicleFittingComp();
	if (!TestNotNull(TEXT("RTA-P0-04 Equipment FittingComp"), VehicleFittingComp))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.4] Apply 전 현재 실제 장착 장비를 가진 MountProfileId입니다.
	FName AppliedMountProfileId = NAME_None;

	// [v1.0.4] Apply 실패 시 exact identity로 유지되어야 하는 현재 EquipmentPresetData입니다.
	UCFEquipmentPresetData* AppliedEquipmentPresetData = nullptr;
	if (!TestTrue(
		TEXT("RTA-P0-04 current Applied Equipment exists"),
		FindFirstAppliedEquipment(
			VehicleFittingComp,
			AppliedMountProfileId,
			AppliedEquipmentPresetData)))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.4] WBP 없이 Equipment selection/apply frontend 연결을 검증할 C++ RuntimeApply Widget입니다.
	UCFRuntimeApplyWidget* RuntimeApplyWidget =
		CreateWidget<UCFRuntimeApplyWidget>(
			TestWorld,
			UCFRuntimeApplyWidget::StaticClass());
	if (!TestNotNull(TEXT("RTA-P0-04 Equipment RuntimeApply Widget"), RuntimeApplyWidget))
	{
		VehiclePawn->Destroy();
		return false;
	}

	RuntimeApplyWidget->SetVehiclePawnRef(VehiclePawn);

	TestTrue(
		TEXT("Applied Mount exists in UI Mount options"),
		SelectExactMount(RuntimeApplyWidget, AppliedMountProfileId));

	// [v1.0.4] 현재 Mount의 타입/크기 호환성 기준을 제공할 VehicleData MountProfile입니다.
	const FCFVehicleMountProfile* AppliedMountProfile = nullptr;
	for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
	{
		if (MountProfile.MountProfileId == AppliedMountProfileId)
		{
			AppliedMountProfile = &MountProfile;
			break;
		}
	}
	if (!TestNotNull(TEXT("RTA-P0-04 applied MountProfile definition"), AppliedMountProfile))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.4] Catalog authorization과 현재 Mount 호환성을 모두 만족하는 UI 선택 후보입니다.
	UCFEquipmentPresetData* CatalogEquipmentCandidate = nullptr;
	for (UCFEquipmentPresetData* CatalogEquipmentPresetData : RuntimeCatalog->AllowedEquipmentPresetData)
	{
		if (IsValid(CatalogEquipmentPresetData)
			&& CatalogEquipmentPresetData->CanUseOnMount(
				AppliedMountProfile->MountType,
				AppliedMountProfile->SizeLimit))
		{
			CatalogEquipmentCandidate = CatalogEquipmentPresetData;
			break;
		}
	}
	if (!TestNotNull(
		TEXT("RTA-P0-04 compatible exact Catalog Equipment"),
		CatalogEquipmentCandidate))
	{
		VehiclePawn->Destroy();
		return false;
	}

	TestTrue(
		TEXT("Compatible exact Catalog Equipment can be selected"),
		SelectExactEquipment(RuntimeApplyWidget, CatalogEquipmentCandidate));

	// [v1.0.4] selection 전후 Current Equipment 불변성을 검사할 Applied Snapshot입니다.
	const FCFVehicleFittingSnapshot SnapshotBeforeApply =
		VehicleFittingComp->GetAppliedFittingSnapshot();

	// [v1.0.4] selection만으로 현재 Applied Equipment가 바뀌지 않았는지 여부입니다.
	bool bCurrentEquipmentUnchangedBeforeApply = false;
	for (const FCFResolvedFittingMount& ResolvedMount : SnapshotBeforeApply.ResolvedMounts)
	{
		if (ResolvedMount.MountProfileId == AppliedMountProfileId)
		{
			bCurrentEquipmentUnchangedBeforeApply =
				ResolvedMount.EquipmentPresetData == AppliedEquipmentPresetData;
			break;
		}
	}
	TestTrue(
		TEXT("Equipment selection alone keeps Current Applied Equipment"),
		bCurrentEquipmentUnchangedBeforeApply);

	// [v1.0.4] Explicit Apply 전 source-level FittingData identity입니다.
	UCFVehicleFittingData* SourceFittingBeforeApply =
		VehiclePawn->VehicleFittingData.Get();
	TestTrue(
		TEXT("Equipment selection alone keeps source Fitting identity"),
		SourceFittingBeforeApply == VehicleFittingData);

	// [v1.0.5] UI Apply 전에 exact Catalog authorization 자체가 통과하는지 downstream Fitting validation과 분리해 확인합니다.
	FString CatalogValidationFailureReason;
	TestTrue(
		TEXT("Exact Catalog candidate passes catalog authorization"),
		FCFRuntimeEquipApplyService::ValidateCatalogEquipmentCandidate(
			RuntimeCatalog,
			CatalogEquipmentCandidate,
			CatalogValidationFailureReason));
	TestTrue(
		TEXT("Exact Catalog authorization has no failure reason"),
		CatalogValidationFailureReason.IsEmpty());

	// [v1.0.5] UI public action이 P0-03 Equipment Runtime service에 위임한 최종 status입니다.
	const ECFRuntimeEquipApplyStatus ApplyStatus =
		RuntimeApplyWidget->ApplySelectedEquipment();

	TestNotEqual(
		TEXT("P0-04 UI path must not end in unrecovered Runtime state"),
		ApplyStatus,
		ECFRuntimeEquipApplyStatus::RecoveryFailed);
	TestTrue(
		TEXT("Selected Equipment remains exact persistent Catalog source"),
		RuntimeApplyWidget->GetSelectedEquipmentData() == CatalogEquipmentCandidate);

	if (ApplyStatus == ECFRuntimeEquipApplyStatus::Succeeded)
	{
		// [v1.0.4] 성공 시 현재 Applied Snapshot이 exact Catalog Equipment로 변경됐는지 여부입니다.
		bool bCandidateEquipmentApplied = false;
		const FCFVehicleFittingSnapshot SnapshotAfterSuccess =
			VehicleFittingComp->GetAppliedFittingSnapshot();
		for (const FCFResolvedFittingMount& ResolvedMount : SnapshotAfterSuccess.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == AppliedMountProfileId)
			{
				bCandidateEquipmentApplied =
					ResolvedMount.EquipmentPresetData == CatalogEquipmentCandidate;
				break;
			}
		}

		TestTrue(
			TEXT("Successful UI Apply updates Current Applied Equipment"),
			bCandidateEquipmentApplied);
		TestTrue(
			TEXT("Successful UI Apply activates transient Fitting source"),
			VehiclePawn->VehicleFittingData
				&& VehiclePawn->VehicleFittingData->HasAnyFlags(RF_Transient));
		TestTrue(
			TEXT("Successful UI Apply reports success"),
			RuntimeApplyWidget->GetLastResultText().Contains(TEXT("성공")));
	}
	else
	{
		// [v1.0.5] 현재 Catalog 장비는 Catalog authorization 이후 Fitting 필수 질량 단계에서 ValidationFailed가 날 수 있고, mutation 이후 실패는 ApplyFailed로 반환됩니다.
		const bool bExpectedFailClosedStatus =
			ApplyStatus == ECFRuntimeEquipApplyStatus::ValidationFailed
			|| ApplyStatus == ECFRuntimeEquipApplyStatus::ApplyFailed;
		TestTrue(
			TEXT("Current Catalog data readiness failure is fail-closed"),
			bExpectedFailClosedStatus);

		// [v1.0.5] fail-closed 뒤 이전 Applied Equipment exact identity가 유지됐는지 여부입니다.
		bool bPreviousEquipmentPreserved = false;
		const FCFVehicleFittingSnapshot SnapshotAfterFailure =
			VehicleFittingComp->GetAppliedFittingSnapshot();
		for (const FCFResolvedFittingMount& ResolvedMount : SnapshotAfterFailure.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == AppliedMountProfileId)
			{
				bPreviousEquipmentPreserved =
					ResolvedMount.EquipmentPresetData == AppliedEquipmentPresetData;
				break;
			}
		}

		TestTrue(
			TEXT("Failed UI Apply preserves previous Current Equipment"),
			bPreviousEquipmentPreserved);
		TestTrue(
			TEXT("Failed UI Apply preserves previous Fitting source"),
			VehiclePawn->VehicleFittingData.Get() == SourceFittingBeforeApply);

		// [v1.0.5] 실제 service status가 UI Last Result에 같은 의미로 표시되는지 확인합니다.
		const bool bExpectedFailureText =
			ApplyStatus == ECFRuntimeEquipApplyStatus::ValidationFailed
				? RuntimeApplyWidget->GetLastResultText().Contains(TEXT("검증 실패"))
				: RuntimeApplyWidget->GetLastResultText().Contains(TEXT("적용 실패"));
		TestTrue(
			TEXT("Failed UI Apply reports exact service failure class"),
			bExpectedFailureText);
	}

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeApplyUiLegacyEquipmentTest,
	"CarFight.RuntimeApply.RTA_P0_04.LegacyEquipmentReadback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.6] Snapshot이 없는 초기 Legacy Runtime에서도 실제 VehicleWeaponComp 활성 장비가 Current Equipment UI에 표시되는지 검증합니다.
bool FCFRuntimeApplyUiLegacyEquipmentTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.6] Legacy Runtime 장비 readback을 실제 컴포넌트 초기화로 검증할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-04 LegacyEquipment World"), TestWorld))
	{
		return false;
	}

	// [v1.0.6] 실제 Vehicle component register/init 계약을 활성화할 Game World guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// [v1.0.6] 기본 RocketLauncher를 가진 persisted fitting-ready VehicleData입니다.
	UCFVehicleData* VehicleData = LoadObject<UCFVehicleData>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"));
	if (!TestNotNull(TEXT("RTA-P0-04 LegacyEquipment VehicleData"), VehicleData))
	{
		return false;
	}

	// [v1.0.6] VehicleFittingData 없이 Legacy/default Runtime으로 초기화할 actual VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;
	if (!BuildRuntimeApplyUiPawn(
		*this,
		TestWorld,
		VehicleData,
		nullptr,
		VehiclePawn))
	{
		return false;
	}

	// [v1.0.6] 초기 Legacy Runtime에서 실제 활성 Equipment를 소유하는 Weapon component입니다.
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("RTA-P0-04 LegacyEquipment VehicleWeaponComp"), VehicleWeaponComp))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.6] Snapshot이 없는 Legacy 경로임을 보장할 Fitting component입니다.
	UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
	if (!TestNotNull(TEXT("RTA-P0-04 LegacyEquipment VehicleFittingComp"), VehicleFittingComp))
	{
		VehiclePawn->Destroy();
		return false;
	}

	TestFalse(
		TEXT("Legacy Runtime has no Applied Snapshot"),
		VehicleFittingComp->HasAppliedFittingSnapshot());

	// [v1.0.6] 실제 Legacy Weapon Runtime이 활성화한 persisted EquipmentPresetData입니다.
	UCFEquipmentPresetData* ActiveLegacyEquipment =
		VehicleWeaponComp->GetActiveEquipmentPresetData();
	if (!TestNotNull(TEXT("Legacy Runtime active Equipment exists"), ActiveLegacyEquipment))
	{
		VehiclePawn->Destroy();
		return false;
	}

	TestEqual(
		TEXT("Legacy Runtime active Equipment is RocketLauncher"),
		ActiveLegacyEquipment->GetFName(),
		FName(TEXT("RocketLauncher")));

	// [v1.0.6] 실제 RuntimeApply Current Equipment 표시를 검증할 pure C++ Widget입니다.
	UCFRuntimeApplyWidget* RuntimeApplyWidget =
		CreateWidget<UCFRuntimeApplyWidget>(
			TestWorld,
			UCFRuntimeApplyWidget::StaticClass());
	if (!TestNotNull(TEXT("RTA-P0-04 LegacyEquipment RuntimeApply Widget"), RuntimeApplyWidget))
	{
		VehiclePawn->Destroy();
		return false;
	}

	RuntimeApplyWidget->SetVehiclePawnRef(VehiclePawn);

	// [v1.0.6] WidgetTree의 Current Equipment TextBlock입니다.
	UTextBlock* CurrentEquipmentTextBlock = Cast<UTextBlock>(
		RuntimeApplyWidget->GetWidgetFromName(TEXT("Text_CurrentEquipment")));
	if (!TestNotNull(TEXT("RTA-P0-04 Current Equipment TextBlock"), CurrentEquipmentTextBlock))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.6] 초기 Legacy Runtime readback이 실제 RocketLauncher DisplayName을 표시하는지 확인할 문자열입니다.
	const FString CurrentEquipmentText = CurrentEquipmentTextBlock->GetText().ToString();
	TestTrue(
		TEXT("Legacy Current Equipment UI shows Prototype Rocket Launcher Kit"),
		CurrentEquipmentText.Contains(TEXT("Prototype Rocket Launcher Kit")));
	TestFalse(
		TEXT("Legacy Current Equipment UI no longer reports Snapshot missing"),
		CurrentEquipmentText.Contains(TEXT("Snapshot 없음")));

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeApplyCatalogSyncTest,
	"CarFight.RuntimeApply.CF_FQ_044.VRCP_P0_03.CatalogOptionSync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] Product Catalog를 수정하지 않고 isolated Catalog exact sequence 변화가 cached RuntimeApply options에 bounded 반영되는지 검증합니다.
bool FCFRuntimeApplyCatalogSyncTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] RuntimeApply Widget 생성에 사용할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("VRCP-P0-03 CatalogSync World"), TestWorld))
	{
		return false;
	}

	// [v1.1.1] Product Default Catalog 내용/개수와 독립된 test-owned transient fixture입니다.
	UCFRuntimeTestCatalogData* TestCatalog = NewObject<UCFRuntimeTestCatalogData>(
		GetTransientPackage(),
		UCFRuntimeTestCatalogData::StaticClass());
	UCFVehicleData* InitialVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	UCFVehicleData* AddedVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	UCFEquipmentPresetData* InitialEquipmentData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	UCFEquipmentPresetData* AddedEquipmentData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	if (!TestNotNull(TEXT("VRCP-P0-03 isolated Catalog"), TestCatalog)
		|| !TestNotNull(TEXT("VRCP-P0-03 initial Vehicle fixture"), InitialVehicleData)
		|| !TestNotNull(TEXT("VRCP-P0-03 added Vehicle fixture"), AddedVehicleData)
		|| !TestNotNull(TEXT("VRCP-P0-03 initial Equipment fixture"), InitialEquipmentData)
		|| !TestNotNull(TEXT("VRCP-P0-03 added Equipment fixture"), AddedEquipmentData))
	{
		return false;
	}

	TestCatalog->AllowedVehicleData.Add(InitialVehicleData);
	TestCatalog->AllowedEquipmentPresetData.Add(InitialEquipmentData);

	// [v1.1.0] WBP 없이 current RuntimeApply cache behavior를 직접 검증할 C++ Widget입니다.
	UCFRuntimeApplyWidget* RuntimeApplyWidget = CreateWidget<UCFRuntimeApplyWidget>(
		TestWorld,
		UCFRuntimeApplyWidget::StaticClass());
	if (!TestNotNull(TEXT("VRCP-P0-03 RuntimeApply Widget"), RuntimeApplyWidget))
	{
		return false;
	}

	RuntimeApplyWidget->RuntimeCatalog = TestCatalog;
	RuntimeApplyWidget->RebuildVehicleOptions();
	RuntimeApplyWidget->RebuildEquipmentOptions();

	TestEqual(TEXT("VRCP-P0-03 initial Vehicle option count"), RuntimeApplyWidget->GetVehicleOptionCount(), 1);
	TestEqual(TEXT("VRCP-P0-03 initial Equipment option count"), RuntimeApplyWidget->GetEquipmentOptionCount(), 1);
	TestTrue(TEXT("VRCP-P0-03 initial Vehicle selection"), RuntimeApplyWidget->SelectVehicleByIndex(0));
	TestTrue(TEXT("VRCP-P0-03 initial Equipment selection"), RuntimeApplyWidget->SelectEquipmentByIndex(0));

	// [v1.1.0] Builder promotion과 같은 same-object Catalog append를 모사합니다.
	TestCatalog->AllowedVehicleData.Add(AddedVehicleData);
	TestCatalog->AllowedEquipmentPresetData.Add(AddedEquipmentData);
	RuntimeApplyWidget->RefreshRuntimeApplyState();

	TestEqual(TEXT("VRCP-P0-03 appended Vehicle appears on first refresh"), RuntimeApplyWidget->GetVehicleOptionCount(), 2);
	TestEqual(TEXT("VRCP-P0-03 appended Equipment appears on first refresh"), RuntimeApplyWidget->GetEquipmentOptionCount(), 2);
	TestTrue(TEXT("VRCP-P0-03 Vehicle selection preserved after append"), RuntimeApplyWidget->GetSelectedVehicleData() == InitialVehicleData);
	TestTrue(TEXT("VRCP-P0-03 Equipment selection preserved after append"), RuntimeApplyWidget->GetSelectedEquipmentData() == InitialEquipmentData);

	// [v1.1.0] Count가 같아도 exact sequence reorder는 변경으로 감지되어 cached order를 따라야 합니다.
	TestCatalog->AllowedVehicleData.Swap(0, 1);
	TestCatalog->AllowedEquipmentPresetData.Swap(0, 1);
	RuntimeApplyWidget->RefreshRuntimeApplyState();

	TestTrue(TEXT("VRCP-P0-03 same-count Vehicle reorder is synchronized"), RuntimeApplyWidget->VehicleOptionDataArray[0].Get() == AddedVehicleData);
	TestTrue(TEXT("VRCP-P0-03 same-count Equipment reorder is synchronized"), RuntimeApplyWidget->EquipmentOptionDataArray[0].Get() == AddedEquipmentData);
	TestTrue(TEXT("VRCP-P0-03 Vehicle selection survives reorder"), RuntimeApplyWidget->GetSelectedVehicleData() == InitialVehicleData);
	TestTrue(TEXT("VRCP-P0-03 Equipment selection survives reorder"), RuntimeApplyWidget->GetSelectedEquipmentData() == InitialEquipmentData);

	// [v1.1.0] 변화가 없는 후속 refresh는 cached exact sequence/selection을 그대로 유지해야 합니다.
	RuntimeApplyWidget->RefreshRuntimeApplyState();
	TestEqual(TEXT("VRCP-P0-03 unchanged Vehicle option count stable"), RuntimeApplyWidget->GetVehicleOptionCount(), 2);
	TestEqual(TEXT("VRCP-P0-03 unchanged Equipment option count stable"), RuntimeApplyWidget->GetEquipmentOptionCount(), 2);
	TestTrue(TEXT("VRCP-P0-03 unchanged Vehicle first identity stable"), RuntimeApplyWidget->VehicleOptionDataArray[0].Get() == AddedVehicleData);
	TestTrue(TEXT("VRCP-P0-03 unchanged Equipment first identity stable"), RuntimeApplyWidget->EquipmentOptionDataArray[0].Get() == AddedEquipmentData);
	TestTrue(TEXT("VRCP-P0-03 unchanged Vehicle selection stable"), RuntimeApplyWidget->GetSelectedVehicleData() == InitialVehicleData);
	TestTrue(TEXT("VRCP-P0-03 unchanged Equipment selection stable"), RuntimeApplyWidget->GetSelectedEquipmentData() == InitialEquipmentData);

	// [v1.1.1] Raw Catalog에 null entry가 끼어도 rebuild가 사용하는 valid-entry filtered cache는 같은 sequence로 유지되어야 합니다.
	TestCatalog->AllowedVehicleData.Insert(nullptr, 1);
	TestCatalog->AllowedEquipmentPresetData.Insert(nullptr, 1);
	RuntimeApplyWidget->RefreshRuntimeApplyState();
	TestEqual(TEXT("VRCP-P0-03 null Vehicle entry filtered from options"), RuntimeApplyWidget->GetVehicleOptionCount(), 2);
	TestEqual(TEXT("VRCP-P0-03 null Equipment entry filtered from options"), RuntimeApplyWidget->GetEquipmentOptionCount(), 2);
	TestTrue(TEXT("VRCP-P0-03 null Vehicle entry preserves first valid identity"), RuntimeApplyWidget->VehicleOptionDataArray[0].Get() == AddedVehicleData);
	TestTrue(TEXT("VRCP-P0-03 null Equipment entry preserves first valid identity"), RuntimeApplyWidget->EquipmentOptionDataArray[0].Get() == AddedEquipmentData);
	TestTrue(TEXT("VRCP-P0-03 null Vehicle entry preserves selection"), RuntimeApplyWidget->GetSelectedVehicleData() == InitialVehicleData);
	TestTrue(TEXT("VRCP-P0-03 null Equipment entry preserves selection"), RuntimeApplyWidget->GetSelectedEquipmentData() == InitialEquipmentData);

	// [v1.1.1] 같은 invalid raw state의 재-refresh에서도 filtered cache identity/count/selection이 안정적으로 유지됩니다.
	RuntimeApplyWidget->RefreshRuntimeApplyState();
	TestEqual(TEXT("VRCP-P0-03 repeated null Vehicle refresh count stable"), RuntimeApplyWidget->GetVehicleOptionCount(), 2);
	TestEqual(TEXT("VRCP-P0-03 repeated null Equipment refresh count stable"), RuntimeApplyWidget->GetEquipmentOptionCount(), 2);
	TestTrue(TEXT("VRCP-P0-03 repeated null Vehicle refresh identity stable"), RuntimeApplyWidget->VehicleOptionDataArray[0].Get() == AddedVehicleData);
	TestTrue(TEXT("VRCP-P0-03 repeated null Equipment refresh identity stable"), RuntimeApplyWidget->EquipmentOptionDataArray[0].Get() == AddedEquipmentData);
	TestTrue(TEXT("VRCP-P0-03 repeated null Vehicle refresh selection stable"), RuntimeApplyWidget->GetSelectedVehicleData() == InitialVehicleData);
	TestTrue(TEXT("VRCP-P0-03 repeated null Equipment refresh selection stable"), RuntimeApplyWidget->GetSelectedEquipmentData() == InitialEquipmentData);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
