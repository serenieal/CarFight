// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-04
// Description: CF-FQ-041 RTA-P0-03 Equipment Slot Runtime Apply + CF-FQ-047 active Mount handoff 자동화 테스트
// Scope: Catalog exact membership, same-mass 장비 hot apply, mass-change 실패 보상 복구와 candidate active Mount 선택 정책을 검증합니다.
// Changelog:
// - v1.2.0: active weapon 소실 + non-weapon target + weapon-bearing 후보 0개에서도 Prepare 전 explicit Reject가 유지되는 회귀를 추가.
// - v1.1.0: current active weapon Mount 보존 / missing-current 시 USER target handoff / non-weapon target fail-closed pure policy 회귀를 추가.
// - v1.0.0: EquipmentValidation, EquipmentApplySuccess, ApplyFailedRecovery 3개 focused test를 최초 추가.
// Migration:
// - 실제 Persisted SUV VehicleData/Fitting fixture는 읽기 전용으로 사용하고 저장·수정하지 않습니다.
// - 성공 후보는 기존 EquipmentPresetData의 transient 복사본을 사용해 동일 질량 경로를 검증합니다.
// - 실패 복구 후보는 transient TurretMountData의 질량만 변경해 BeginPlay 전 Automation World에서 Chaos hot-reapply 실패를 유도하며 Product test hook을 추가하지 않습니다.

#include "CFRuntimeEquipApply.h"
#include "CFRuntimeEquipApplyPolicy.h"

#include "CFEquipmentPresetData.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFWeaponData.h"
#include "CFTurretMountData.h"

#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// [v1.0.0] Equipment exact membership 검증에 사용할 최소 valid transient Runtime Test Catalog를 생성합니다.
	UCFRuntimeTestCatalogData* BuildTransientEquipmentCatalog(
		UCFVehicleData* AllowedVehicleData,
		UCFEquipmentPresetData* AllowedEquipmentPresetData)
	{
		// Equipment authorization 테스트가 소유할 transient Catalog입니다.
		UCFRuntimeTestCatalogData* RuntimeCatalog = NewObject<UCFRuntimeTestCatalogData>(
			GetTransientPackage(),
			MakeUniqueObjectName(
				GetTransientPackage(),
				UCFRuntimeTestCatalogData::StaticClass(),
				TEXT("RTA_P0_03_Catalog")));

		if (RuntimeCatalog && AllowedVehicleData && AllowedEquipmentPresetData)
		{
			RuntimeCatalog->AllowedVehicleData.Add(AllowedVehicleData);
			RuntimeCatalog->AllowedEquipmentPresetData.Add(AllowedEquipmentPresetData);
		}

		return RuntimeCatalog;
	}

	// [v1.0.0] 실제 persisted SUV Snapshot Fitting이 적용된 BP_CFVehiclePawn Runtime fixture를 생성합니다.
	bool BuildEquipmentRuntimePawn(
		FAutomationTestBase& Test,
		UWorld* TestWorld,
		ACFVehiclePawn*& OutVehiclePawn,
		UCFVehicleData*& OutVehicleData,
		UCFVehicleFittingData*& OutVehicleFittingData)
	{
		OutVehiclePawn = nullptr;
		OutVehicleData = nullptr;
		OutVehicleFittingData = nullptr;

		// 실제 Blueprint 컴포넌트 구성으로 Spawn할 VehiclePawn class입니다.
		UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
			nullptr,
			TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));

		// 기존 Fitting Runtime regression이 사용하는 persisted SUV VehicleData입니다.
		UCFVehicleData* VehicleData = LoadObject<UCFVehicleData>(
			nullptr,
			TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"));

		// 기존 1570kg Snapshot mass와 실제 장비 선택을 포함한 persisted Fitting fixture입니다.
		UCFVehicleFittingData* VehicleFittingData = LoadObject<UCFVehicleFittingData>(
			nullptr,
			TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV"));

		if (!Test.TestNotNull(TEXT("RTA-P0-03 BP_CFVehiclePawn class"), VehiclePawnClass)
			|| !Test.TestNotNull(TEXT("RTA-P0-03 VehicleData"), VehicleData)
			|| !Test.TestNotNull(TEXT("RTA-P0-03 VehicleFittingData"), VehicleFittingData))
		{
			return false;
		}

		// actual BP Pawn을 충돌 없이 생성할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Equipment Runtime operation을 수행할 actual VehiclePawn입니다.
		ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
			VehiclePawnClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Test.TestNotNull(TEXT("RTA-P0-03 VehiclePawn spawn"), VehiclePawn))
		{
			return false;
		}

		VehiclePawn->UnregisterAllComponents();
		VehiclePawn->VehicleData = VehicleData;
		VehiclePawn->VehicleFittingData = VehicleFittingData;
		VehiclePawn->bAutoRegisterInputMappingContext = false;
		VehiclePawn->bShowAimReticle = false;
		VehiclePawn->bShowTargetSelectHud = false;

		// persisted Snapshot을 fresh하게 재구성할 실제 Fitting component입니다.
		UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
		if (!Test.TestNotNull(TEXT("RTA-P0-03 VehicleFittingComp"), VehicleFittingComp))
		{
			VehiclePawn->Destroy();
			return false;
		}

		VehicleFittingComp->ResetFittingRuntimeState();
		VehiclePawn->RegisterAllComponents();

		if (!Test.TestTrue(TEXT("RTA-P0-03 Snapshot Initial Mass configured"), VehicleFittingComp->HasConfiguredInitialMass())
			|| !Test.TestTrue(TEXT("RTA-P0-03 initial Runtime initialize"), VehiclePawn->InitializeVehicleRuntime())
			|| !Test.TestTrue(TEXT("RTA-P0-03 initial Core Runtime ready"), VehiclePawn->GetVehicleDebugRuntime().bRuntimeReady)
			|| !Test.TestTrue(TEXT("RTA-P0-03 initial Combat Runtime ready"), VehiclePawn->bVehicleCombatRuntimeReady)
			|| !Test.TestTrue(TEXT("RTA-P0-03 initial Applied Snapshot exists"), VehicleFittingComp->HasAppliedFittingSnapshot()))
		{
			VehiclePawn->Destroy();
			return false;
		}

		OutVehiclePawn = VehiclePawn;
		OutVehicleData = VehicleData;
		OutVehicleFittingData = VehicleFittingData;
		return true;
	}

	// [v1.0.0] Applied Snapshot에서 실제 EquipmentPresetData가 있는 첫 Mount를 찾아 테스트 입력으로 반환합니다.
	bool FindFirstEquippedMount(
		const UCFVehicleFittingComp* VehicleFittingComp,
		FName& OutMountProfileId,
		UCFEquipmentPresetData*& OutEquipmentPresetData)
	{
		OutMountProfileId = NAME_None;
		OutEquipmentPresetData = nullptr;
		if (!VehicleFittingComp || !VehicleFittingComp->HasAppliedFittingSnapshot())
		{
			return false;
		}

		// 현재 실제 적용된 Mount와 Equipment를 결정론적 순서로 보유한 Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehicleFittingComp->GetAppliedFittingSnapshot();
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			if (!ResolvedMount.MountProfileId.IsNone() && IsValid(ResolvedMount.EquipmentPresetData))
			{
				OutMountProfileId = ResolvedMount.MountProfileId;
				OutEquipmentPresetData = ResolvedMount.EquipmentPresetData;
				return true;
			}
		}

		return false;
	}

	// [v1.0.0] Applied Snapshot의 지정 Mount가 exact EquipmentPresetData를 가리키는지 검증합니다.
	bool IsExactEquipmentApplied(
		const UCFVehicleFittingComp* VehicleFittingComp,
		const FName TargetMountProfileId,
		const UCFEquipmentPresetData* ExpectedEquipmentPresetData)
	{
		if (!VehicleFittingComp || !VehicleFittingComp->HasAppliedFittingSnapshot())
		{
			return false;
		}

		// final exact Equipment identity를 검사할 현재 Applied Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehicleFittingComp->GetAppliedFittingSnapshot();
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == TargetMountProfileId)
			{
				return ResolvedMount.EquipmentPresetData == ExpectedEquipmentPresetData;
			}
		}

		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeEquipActiveMountPolicyTest,
	"CarFight.RuntimeApply.CF_FQ_047.ActiveMountHandoffPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] 질량/Chaos와 무관하게 candidate Snapshot의 active weapon Mount 선택과 explicit fail-closed 정책을 결정론적으로 검증합니다.
bool FCFRuntimeEquipActiveMountPolicyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const FName CurrentMountId(TEXT("LegacyActiveMount"));
	const FName TargetMountId(TEXT("Mount_Top_01"));
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage(), TEXT("CF_FQ_047_ActiveMountPolicyWeapon"));
	if (!TestNotNull(TEXT("CF-FQ-047 policy WeaponData"), WeaponData))
	{
		return false;
	}

	FCFResolvedFittingMount CurrentWeaponMount;
	CurrentWeaponMount.MountProfileId = CurrentMountId;
	CurrentWeaponMount.WeaponData = WeaponData;

	FCFResolvedFittingMount TargetWeaponMount;
	TargetWeaponMount.MountProfileId = TargetMountId;
	TargetWeaponMount.WeaponData = WeaponData;

	FCFVehicleFittingSnapshot BothWeaponMounts;
	BothWeaponMounts.ResolvedMounts = {CurrentWeaponMount, TargetWeaponMount};

	// candidate에 current active weapon Mount가 그대로 남아 있는 정책 결과입니다.
	const CFRuntimeEquipApplyPolicy::FCFCandidateActiveMountResolution PreserveResolution =
		CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount(BothWeaponMounts, CurrentMountId, TargetMountId);
	TestTrue(TEXT("CF-FQ-047 current active 보존은 Apply 허용"), PreserveResolution.CanApply());
	TestEqual(TEXT("CF-FQ-047 candidate에 current active weapon Mount가 남아 있으면 보존"), PreserveResolution.RequestedActiveMountProfileId, CurrentMountId);
	TestEqual(TEXT("CF-FQ-047 current active 보존 decision"), PreserveResolution.Decision, CFRuntimeEquipApplyPolicy::ECFCandidateActiveMountDecision::PreserveCurrent);

	FCFVehicleFittingSnapshot TargetOnlySnapshot;
	TargetOnlySnapshot.ResolvedMounts = {TargetWeaponMount};

	// current active가 사라지고 USER target weapon Mount만 남은 handoff 정책 결과입니다.
	const CFRuntimeEquipApplyPolicy::FCFCandidateActiveMountResolution HandoffResolution =
		CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount(TargetOnlySnapshot, CurrentMountId, TargetMountId);
	TestTrue(TEXT("CF-FQ-047 USER target weapon handoff는 Apply 허용"), HandoffResolution.CanApply());
	TestEqual(TEXT("CF-FQ-047 current active가 candidate에서 사라지면 USER target weapon Mount로 handoff"), HandoffResolution.RequestedActiveMountProfileId, TargetMountId);
	TestEqual(TEXT("CF-FQ-047 USER target handoff decision"), HandoffResolution.Decision, CFRuntimeEquipApplyPolicy::ECFCandidateActiveMountDecision::HandoffToTarget);

	FCFResolvedFittingMount TargetNonWeaponMount;
	TargetNonWeaponMount.MountProfileId = TargetMountId;
	FCFVehicleFittingSnapshot NonWeaponTargetSnapshot;
	NonWeaponTargetSnapshot.ResolvedMounts = {TargetNonWeaponMount};

	// 기존 active weapon이 사라지고 target도 비무장인 P1 edge case 정책 결과입니다.
	const CFRuntimeEquipApplyPolicy::FCFCandidateActiveMountResolution RejectResolution =
		CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount(NonWeaponTargetSnapshot, CurrentMountId, TargetMountId);
	TestFalse(TEXT("CF-FQ-047 weapon-bearing 후보 0 + non-weapon target은 Prepare 전에 Reject"), RejectResolution.CanApply());
	TestEqual(TEXT("CF-FQ-047 Reject는 기존 active identity를 진단용으로 보존"), RejectResolution.RequestedActiveMountProfileId, CurrentMountId);
	TestEqual(TEXT("CF-FQ-047 non-weapon target reject decision"), RejectResolution.Decision, CFRuntimeEquipApplyPolicy::ECFCandidateActiveMountDecision::RejectNonWeaponTarget);

	// 원래 active weapon이 없고 target이 weapon-bearing이면 새 active weapon으로 선택하는 정책 결과입니다.
	const CFRuntimeEquipApplyPolicy::FCFCandidateActiveMountResolution NoCurrentWeaponHandoffResolution =
		CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount(TargetOnlySnapshot, NAME_None, TargetMountId);
	TestTrue(TEXT("CF-FQ-047 current active None + weapon target은 Apply 허용"), NoCurrentWeaponHandoffResolution.CanApply());
	TestEqual(TEXT("CF-FQ-047 current active None에서도 valid USER target weapon Mount를 선택"), NoCurrentWeaponHandoffResolution.RequestedActiveMountProfileId, TargetMountId);

	// 원래 active weapon 자체가 없고 target도 비무장이면 weapon identity 없이 정상 진행하는 정책 결과입니다.
	const CFRuntimeEquipApplyPolicy::FCFCandidateActiveMountResolution NoActiveWeaponResolution =
		CFRuntimeEquipApplyPolicy::ResolveCandidateActiveMount(NonWeaponTargetSnapshot, NAME_None, TargetMountId);
	TestTrue(TEXT("CF-FQ-047 원래 active weapon이 없던 non-weapon target은 불필요하게 차단하지 않음"), NoActiveWeaponResolution.CanApply());
	TestEqual(TEXT("CF-FQ-047 active weapon 없음은 NAME_None 유지"), NoActiveWeaponResolution.RequestedActiveMountProfileId, NAME_None);
	TestEqual(TEXT("CF-FQ-047 no active weapon decision"), NoActiveWeaponResolution.Decision, CFRuntimeEquipApplyPolicy::ECFCandidateActiveMountDecision::NoActiveWeapon);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeEquipValidationTest,
	"CarFight.RuntimeApply.RTA_P0_03.EquipmentValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Catalog 전체 계약과 exact EquipmentPresetData membership을 mutation 없이 검증합니다.
bool FCFRuntimeEquipValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Catalog 전체 계약을 valid로 만들 최소 transient VehicleData입니다.
	UCFVehicleData* AllowedVehicleData = NewObject<UCFVehicleData>(
		GetTransientPackage(),
		TEXT("RTA_P0_03_AllowedVehicle"));

	// exact membership PASS 대상 transient EquipmentPresetData입니다.
	UCFEquipmentPresetData* AllowedEquipmentPresetData = NewObject<UCFEquipmentPresetData>(
		GetTransientPackage(),
		TEXT("RTA_P0_03_AllowedEquipment"));

	// Catalog에 등록하지 않아 exact membership FAIL이어야 하는 장비입니다.
	UCFEquipmentPresetData* UnregisteredEquipmentPresetData = NewObject<UCFEquipmentPresetData>(
		GetTransientPackage(),
		TEXT("RTA_P0_03_UnregisteredEquipment"));

	// Vehicle 1개 + Equipment 1개로 Catalog 전체 hard-reference 계약을 만족시킬 transient source입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = BuildTransientEquipmentCatalog(
		AllowedVehicleData,
		AllowedEquipmentPresetData);

	if (!TestNotNull(TEXT("RTA-P0-03 Allowed VehicleData"), AllowedVehicleData)
		|| !TestNotNull(TEXT("RTA-P0-03 Allowed EquipmentPresetData"), AllowedEquipmentPresetData)
		|| !TestNotNull(TEXT("RTA-P0-03 Unregistered EquipmentPresetData"), UnregisteredEquipmentPresetData)
		|| !TestNotNull(TEXT("RTA-P0-03 transient Catalog"), RuntimeCatalog))
	{
		return false;
	}

	// Catalog validation 실패 이유를 확인할 출력 문자열입니다.
	FString ValidationError;
	TestTrue(
		TEXT("등록 EquipmentPresetData validation PASS"),
		FCFRuntimeEquipApplyService::ValidateCatalogEquipmentCandidate(
			RuntimeCatalog,
			AllowedEquipmentPresetData,
			ValidationError));
	TestTrue(TEXT("등록 EquipmentPresetData PASS error empty"), ValidationError.IsEmpty());

	TestFalse(
		TEXT("미등록 EquipmentPresetData validation FAIL"),
		FCFRuntimeEquipApplyService::ValidateCatalogEquipmentCandidate(
			RuntimeCatalog,
			UnregisteredEquipmentPresetData,
			ValidationError));
	TestTrue(TEXT("미등록 Equipment 오류에 허용 목록 설명 포함"), ValidationError.Contains(TEXT("허용 목록")));

	RuntimeCatalog->AllowedVehicleData.Reset();
	TestFalse(
		TEXT("전체 Catalog 계약 invalid면 Equipment candidate도 FAIL"),
		FCFRuntimeEquipApplyService::ValidateCatalogEquipmentCandidate(
			RuntimeCatalog,
			AllowedEquipmentPresetData,
			ValidationError));
	TestTrue(TEXT("invalid Catalog 오류에 계약 설명 포함"), ValidationError.Contains(TEXT("Catalog 계약")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeEquipSuccessTest,
	"CarFight.RuntimeApply.RTA_P0_03.EquipmentApplySuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 Snapshot 차량에서 같은 질량의 transient Equipment 사본을 한 Mount에 hot apply하는지 검증합니다.
bool FCFRuntimeEquipSuccessTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 실제 BP VehiclePawn Runtime을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-03 Success World"), TestWorld))
	{
		return false;
	}

	// PreRegister/Chaos physics 경로를 실제 Game World 의미로 실행하고 종료 시 원래 WorldType을 복원합니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// 실제 Runtime fixture로 생성된 VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;

	// 실제 Runtime fixture가 사용하는 VehicleData입니다.
	UCFVehicleData* VehicleData = nullptr;

	// apply 이전 source pointer identity를 검증할 persisted VehicleFittingData입니다.
	UCFVehicleFittingData* PreviousVehicleFittingData = nullptr;
	if (!BuildEquipmentRuntimePawn(
		*this,
		TestWorld,
		VehiclePawn,
		VehicleData,
		PreviousVehicleFittingData))
	{
		return false;
	}

	// 현재 Applied Snapshot을 readback할 실제 Fitting component입니다.
	UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();

	// same-mass 교체 대상으로 사용할 실제 MountProfileId입니다.
	FName TargetMountProfileId = NAME_None;

	// same-mass 후보의 원본이 될 현재 실제 EquipmentPresetData입니다.
	UCFEquipmentPresetData* PreviousEquipmentPresetData = nullptr;
	if (!TestTrue(
		TEXT("RTA-P0-03 success equipped mount fixture"),
		FindFirstEquippedMount(
			VehicleFittingComp,
			TargetMountProfileId,
			PreviousEquipmentPresetData)))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// 저장 Asset을 수정하지 않고 동일 payload/질량으로 exact identity만 다른 후보 Equipment입니다.
	UCFEquipmentPresetData* CandidateEquipmentPresetData = DuplicateObject<UCFEquipmentPresetData>(
		PreviousEquipmentPresetData,
		GetTransientPackage(),
		MakeUniqueObjectName(
			GetTransientPackage(),
			UCFEquipmentPresetData::StaticClass(),
			TEXT("RTA_P0_03_SameMassEquipment")));
	if (!TestNotNull(TEXT("RTA-P0-03 same-mass candidate Equipment"), CandidateEquipmentPresetData))
	{
		VehiclePawn->Destroy();
		return false;
	}

	CandidateEquipmentPresetData->EquipmentId = TEXT("RTA_P0_03_SameMass");
	CandidateEquipmentPresetData->DisplayName = FText::FromString(TEXT("RTA P0-03 Same Mass Candidate"));

	// 단일 Mount transient fitting + 기존 Fitting Runtime authority를 수행한 결과입니다.
	const FCFRuntimeEquipApplyResult ApplyResult =
		FCFRuntimeEquipApplyService::ApplyEquipmentRuntime(
			VehiclePawn,
			TargetMountProfileId,
			CandidateEquipmentPresetData);

	TestEqual(TEXT("Equipment Apply 성공 상태"), ApplyResult.Status, ECFRuntimeEquipApplyStatus::Succeeded);
	TestTrue(TEXT("Equipment Apply combat runtime ready"), ApplyResult.bRuntimeReady);
	TestTrue(TEXT("Equipment Apply transient Fitting active"), ApplyResult.bAppliedTransientFittingActive);
	TestFalse(TEXT("same-mass candidate는 Mass reapply 불필요"), ApplyResult.bMassReapplyRequired);
	TestFalse(TEXT("Equipment Apply recovery 미시도"), ApplyResult.bRecoveryAttempted);
	TestTrue(
		TEXT("Applied Snapshot exact candidate Equipment"),
		IsExactEquipmentApplied(
			VehicleFittingComp,
			TargetMountProfileId,
			CandidateEquipmentPresetData));
	TestTrue(TEXT("source VehicleFittingData는 이전 persisted pointer와 다름"), VehiclePawn->VehicleFittingData.Get() != PreviousVehicleFittingData);
	TestTrue(TEXT("source VehicleFittingData transient flag"), VehiclePawn->VehicleFittingData && VehiclePawn->VehicleFittingData->HasAnyFlags(RF_Transient));
	TestTrue(TEXT("source VehicleFittingData outer TransientPackage"), VehiclePawn->VehicleFittingData && VehiclePawn->VehicleFittingData->GetOutermost() == GetTransientPackage());
	TestEqual(TEXT("previous Fitting path readback"), ApplyResult.PreviousVehicleFittingDataPath, GetPathNameSafe(PreviousVehicleFittingData));
	TestTrue(
		TEXT("same-mass configured mass unchanged"),
		FMath::IsNearlyEqual(
			ApplyResult.PreviousConfiguredMassKg,
			ApplyResult.CurrentConfiguredMassKg,
			0.01f));

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeEquipRecoveryTest,
	"CarFight.RuntimeApply.RTA_P0_03.ApplyFailedRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 후보 Fitting Commit 뒤 Chaos mass hot-reapply가 실패하면 이전 Fitting/source/mass Runtime을 보상 복구하는지 검증합니다.
bool FCFRuntimeEquipRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// BeginPlay 전 실제 BP fixture에서 Mass Runtime failure를 재현할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-03 Recovery World"), TestWorld))
	{
		return false;
	}

	// Physics component 등록은 수행하되 World BeginPlay 전 상태를 유지할 Game World type guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// 실제 Runtime fixture로 생성된 VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;

	// 실제 Runtime fixture가 사용하는 VehicleData입니다.
	UCFVehicleData* VehicleData = nullptr;

	// 실패 보상 뒤 exact source pointer로 돌아와야 하는 persisted FittingData입니다.
	UCFVehicleFittingData* PreviousVehicleFittingData = nullptr;
	if (!BuildEquipmentRuntimePawn(
		*this,
		TestWorld,
		VehiclePawn,
		VehicleData,
		PreviousVehicleFittingData))
	{
		return false;
	}

	// 이전 Applied Snapshot과 final recovery를 readback할 Fitting component입니다.
	UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();

	// mass-change 후보를 적용할 실제 MountProfileId입니다.
	FName TargetMountProfileId = NAME_None;

	// 복구 뒤 exact identity로 다시 Applied 상태여야 하는 이전 EquipmentPresetData입니다.
	UCFEquipmentPresetData* PreviousEquipmentPresetData = nullptr;
	if (!TestTrue(
		TEXT("RTA-P0-03 recovery equipped mount fixture"),
		FindFirstEquippedMount(
			VehicleFittingComp,
			TargetMountProfileId,
			PreviousEquipmentPresetData)))
	{
		VehiclePawn->Destroy();
		return false;
	}

	if (!TestNotNull(
		TEXT("RTA-P0-03 recovery source TurretMountData"),
		PreviousEquipmentPresetData ? PreviousEquipmentPresetData->DefaultTurretMountData.Get() : nullptr))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// 저장 Asset은 건드리지 않고 payload를 복사할 candidate EquipmentPresetData입니다.
	UCFEquipmentPresetData* CandidateEquipmentPresetData = DuplicateObject<UCFEquipmentPresetData>(
		PreviousEquipmentPresetData,
		GetTransientPackage(),
		MakeUniqueObjectName(
			GetTransientPackage(),
			UCFEquipmentPresetData::StaticClass(),
			TEXT("RTA_P0_03_MassChangeEquipment")));
	if (!TestNotNull(TEXT("RTA-P0-03 mass-change candidate Equipment"), CandidateEquipmentPresetData))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// candidate Equipment가 별도 질량을 갖도록 복사한 transient TurretMountData입니다.
	UCFTurretMountData* CandidateTurretMountData = DuplicateObject<UCFTurretMountData>(
		PreviousEquipmentPresetData->DefaultTurretMountData,
		CandidateEquipmentPresetData,
		MakeUniqueObjectName(
			CandidateEquipmentPresetData,
			UCFTurretMountData::StaticClass(),
			TEXT("RTA_P0_03_MassChangeMount")));
	if (!TestNotNull(TEXT("RTA-P0-03 mass-change candidate TurretMount"), CandidateTurretMountData))
	{
		VehiclePawn->Destroy();
		return false;
	}

	CandidateEquipmentPresetData->EquipmentId = TEXT("RTA_P0_03_MassChange");
	CandidateEquipmentPresetData->DisplayName = FText::FromString(TEXT("RTA P0-03 Mass Change Candidate"));
	CandidateTurretMountData->TurretMountWeightKg += 10.0f;
	CandidateEquipmentPresetData->DefaultTurretMountData = CandidateTurretMountData;

	// Fitting Commit은 성공하되 BeginPlay 전 Chaos mass hot-reapply가 실패해 보상 경로로 진입해야 하는 결과입니다.
	const FCFRuntimeEquipApplyResult ApplyResult =
		FCFRuntimeEquipApplyService::ApplyEquipmentRuntime(
			VehiclePawn,
			TargetMountProfileId,
			CandidateEquipmentPresetData);

	TestTrue(TEXT("mass-change candidate는 Mass reapply 필요"), ApplyResult.bMassReapplyRequired);
	TestEqual(TEXT("Mass Apply 실패는 ApplyFailed/복구성공"), ApplyResult.Status, ECFRuntimeEquipApplyStatus::ApplyFailed);
	TestTrue(TEXT("Mass Apply 실패 뒤 recovery 시도"), ApplyResult.bRecoveryAttempted);
	TestTrue(TEXT("Mass Apply 실패 뒤 recovery 성공"), ApplyResult.bRecoverySucceeded);
	TestTrue(TEXT("Recovery final combat runtime ready"), ApplyResult.bRuntimeReady);
	TestTrue(TEXT("Recovery source Fitting pointer exact 복원"), VehiclePawn->VehicleFittingData.Get() == PreviousVehicleFittingData);
	TestTrue(
		TEXT("Recovery Applied Snapshot exact 이전 Equipment"),
		IsExactEquipmentApplied(
			VehicleFittingComp,
			TargetMountProfileId,
			PreviousEquipmentPresetData));
	TestTrue(
		TEXT("Recovery configured mass exact 이전 값"),
		FMath::IsNearlyEqual(
			ApplyResult.PreviousConfiguredMassKg,
			ApplyResult.CurrentConfiguredMassKg,
			0.01f));
	TestFalse(TEXT("Recovery final transient Fitting inactive"), ApplyResult.bAppliedTransientFittingActive);

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
