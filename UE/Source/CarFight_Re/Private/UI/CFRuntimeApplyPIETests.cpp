// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-03
// Description: CF-FQ-041 Runtime Apply + CF-FQ-047 actual Wagon Standard Mount PIE E2E 자동화 테스트
// Scope: 기존 Vehicle/Equipment 반복 Apply와 persisted Wagon Mount_Top_01의 RuntimeApply 노출 및 overweight Equipment fail-closed를 실제 PIE World에서 검증합니다.
// Changelog:
// - v1.2.0: Wagon Product acceptance를 Mount_Top_01 실제 UI 노출로 정렬하고, HeavyCannon 2356kg > Wagon 2350kg GrossMassExceeded는 정상 ValidationFailed + state preservation으로 검증.
// - v1.1.0: actual Wagon을 direct source로 초기화한 뒤 RuntimeApply UI에서 Mount_Top_01 + HeavyCannon 첫 적용 handoff 회귀를 추가.
// - v1.0.0: persisted Runtime Catalog와 실제 BP_CFVehiclePawn을 사용한 Vehicle/Equipment 반복 Apply PIE E2E를 최초 추가.
// Migration:
// - 테스트는 /Game/Maps/M_VehicleDefensePIE를 읽기 전용으로 로드하고 PIE 복제 World에 테스트 Pawn만 생성합니다.
// - 실제 RuntimeApply Widget public API와 Product Vehicle/Equipment Apply service만 사용하며 Map/DataAsset/Blueprint를 저장하지 않습니다.
// - P0 RuntimeApply UI에는 별도 Equipment clear/default action이 없으므로 존재하지 않는 UX를 테스트 전용으로 만들지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "UI/CFRuntimeApplyWidget.h"

#include "CFEquipmentPresetData.h"
#include "CFRuntimeEquipApply.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"

namespace
{
	// [v1.0.0] RuntimeApply Widget Vehicle option에서 exact VehicleData를 찾아 선택합니다.
	bool SelectExactPIEVehicle(
		UCFRuntimeApplyWidget* RuntimeApplyWidget,
		UCFVehicleData* ExpectedVehicleData)
	{
		if (!RuntimeApplyWidget || !ExpectedVehicleData)
		{
			return false;
		}

		// [v1.0.0] RuntimeApply Widget에 현재 노출된 Vehicle option 개수입니다.
		const int32 VehicleOptionCount = RuntimeApplyWidget->GetVehicleOptionCount();
		for (int32 VehicleIndex = 0; VehicleIndex < VehicleOptionCount; ++VehicleIndex)
		{
			if (RuntimeApplyWidget->SelectVehicleByIndex(VehicleIndex)
				&& RuntimeApplyWidget->GetSelectedVehicleData() == ExpectedVehicleData)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] RuntimeApply Widget Mount option에서 exact MountProfileId를 찾아 선택합니다.
	bool SelectExactPIEMount(
		UCFRuntimeApplyWidget* RuntimeApplyWidget,
		const FName ExpectedMountProfileId)
	{
		if (!RuntimeApplyWidget || ExpectedMountProfileId.IsNone())
		{
			return false;
		}

		// [v1.0.0] RuntimeApply Widget에 현재 노출된 Mount option 개수입니다.
		const int32 MountOptionCount = RuntimeApplyWidget->GetMountOptionCount();
		for (int32 MountIndex = 0; MountIndex < MountOptionCount; ++MountIndex)
		{
			if (RuntimeApplyWidget->SelectMountByIndex(MountIndex)
				&& RuntimeApplyWidget->GetSelectedMountProfileId() == ExpectedMountProfileId)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] RuntimeApply Widget Equipment option에서 exact EquipmentPresetData를 찾아 선택합니다.
	bool SelectExactPIEEquipment(
		UCFRuntimeApplyWidget* RuntimeApplyWidget,
		UCFEquipmentPresetData* ExpectedEquipmentPresetData)
	{
		if (!RuntimeApplyWidget || !ExpectedEquipmentPresetData)
		{
			return false;
		}

		// [v1.0.0] RuntimeApply Widget에 현재 노출된 Equipment option 개수입니다.
		const int32 EquipmentOptionCount = RuntimeApplyWidget->GetEquipmentOptionCount();
		for (int32 EquipmentIndex = 0; EquipmentIndex < EquipmentOptionCount; ++EquipmentIndex)
		{
			if (RuntimeApplyWidget->SelectEquipmentByIndex(EquipmentIndex)
				&& RuntimeApplyWidget->GetSelectedEquipmentData() == ExpectedEquipmentPresetData)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] Catalog Vehicle 목록에서 Asset 이름이 일치하는 exact VehicleData를 찾습니다.
	UCFVehicleData* FindCatalogVehicleByAssetName(
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		const FName ExpectedAssetName)
	{
		if (!RuntimeCatalog || ExpectedAssetName.IsNone())
		{
			return nullptr;
		}

		for (UCFVehicleData* CandidateVehicleData : RuntimeCatalog->AllowedVehicleData)
		{
			if (IsValid(CandidateVehicleData)
				&& CandidateVehicleData->GetFName() == ExpectedAssetName)
			{
				return CandidateVehicleData;
			}
		}
		return nullptr;
	}

	// [v1.0.0] Catalog Equipment 목록에서 Asset 이름이 일치하는 exact EquipmentPresetData를 찾습니다.
	UCFEquipmentPresetData* FindCatalogEquipmentByAssetName(
		const UCFRuntimeTestCatalogData* RuntimeCatalog,
		const FName ExpectedAssetName)
	{
		if (!RuntimeCatalog || ExpectedAssetName.IsNone())
		{
			return nullptr;
		}

		for (UCFEquipmentPresetData* CandidateEquipmentPresetData : RuntimeCatalog->AllowedEquipmentPresetData)
		{
			if (IsValid(CandidateEquipmentPresetData)
				&& CandidateEquipmentPresetData->GetFName() == ExpectedAssetName)
			{
				return CandidateEquipmentPresetData;
			}
		}
		return nullptr;
	}

	// [v1.0.0] 현재 Applied Snapshot에서 exact MountProfileId에 적용된 EquipmentPresetData를 반환합니다.
	UCFEquipmentPresetData* FindAppliedEquipmentAtMount(
		const UCFVehicleFittingComp* VehicleFittingComp,
		const FName MountProfileId)
	{
		if (!VehicleFittingComp
			|| MountProfileId.IsNone()
			|| !VehicleFittingComp->HasAppliedFittingSnapshot())
		{
			return nullptr;
		}

		// [v1.0.0] 현재 실제 Fitting runtime이 Commit한 read-only Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedSnapshot =
			VehicleFittingComp->GetAppliedFittingSnapshot();
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedSnapshot.ResolvedMounts)
		{
			if (ResolvedMount.MountProfileId == MountProfileId)
			{
				return ResolvedMount.EquipmentPresetData;
			}
		}
		return nullptr;
	}

	// [v1.0.0] 실제 PIE World에서 RTA-P0-05 Vehicle/Equipment 연쇄를 한 번 실행하고 모든 Runtime readback을 검증합니다.
	class FCFRuntimeApplyPIEE2ECommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.0.0] PIE World 준비 제한시간과 결과 기록 대상을 초기화합니다.
		explicit FCFRuntimeApplyPIEE2ECommand(FAutomationTestBase* InTest)
			: Test(InTest)
			, StartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// [v1.0.0] 실제 PIE World가 준비되면 같은 Pawn에서 Vehicle/Equipment E2E를 실행하고 종료합니다.
		virtual bool Update() override
		{
			// [v1.0.0] 현재 Engine World Context에서 찾은 실제 PIE World입니다.
			UWorld* PIEWorld = nullptr;
			if (GEngine)
			{
				for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
				{
					if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
					{
						PIEWorld = WorldContext.World();
						break;
					}
				}
			}

			// [v1.0.0] PIE World 준비를 기다린 누적 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			// [v1.0.0] PIE World 준비 실패를 무한 대기하지 않을 제한 시간입니다.
			constexpr double PIEReadyTimeoutSeconds = 30.0;
			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if (ElapsedSeconds >= PIEReadyTimeoutSeconds)
				{
					Test->AddError(TEXT("RTA-P0-05 PIEE2E: 30초 안에 PIE World Actor 초기화가 완료되지 않았습니다."));
					return true;
				}
				return false;
			}

			// [v1.0.0] 실제 RuntimeApply UI와 동일한 Config soft reference loader를 소유하는 settings CDO입니다.
			const UCFRuntimeTestSettings* RuntimeTestSettings = GetDefault<UCFRuntimeTestSettings>();
			// [v1.0.0] P0-05 Vehicle/Equipment 선택 authorization에 사용할 persisted Runtime Catalog입니다.
			UCFRuntimeTestCatalogData* RuntimeCatalog =
				RuntimeTestSettings ? RuntimeTestSettings->LoadDefaultCatalog() : nullptr;
			if (!Test->TestNotNull(TEXT("RTA-P0-05 default Runtime Catalog"), RuntimeCatalog))
			{
				return true;
			}
			if (!Test->TestTrue(
				TEXT("RTA-P0-05 default Runtime Catalog usable"),
				RuntimeCatalog->IsRuntimeTestCatalogUsable()))
			{
				return true;
			}

			// [v1.0.0] A→B→A의 A이자 Equipment Snapshot 전환 기준으로 사용할 fitting-ready Catalog VehicleData입니다.
			UCFVehicleData* FittingReadyVehicleData = FindCatalogVehicleByAssetName(
				RuntimeCatalog,
				TEXT("DA_VehicleDefense_TestSUV"));
			// [v1.0.0] A→B→A의 B로 사용할 기존 legacy Catalog VehicleData입니다.
			UCFVehicleData* AlternateVehicleData = FindCatalogVehicleByAssetName(
				RuntimeCatalog,
				TEXT("DA_TestSUV"));
			// [v1.0.0] 첫 Snapshot Equipment 적용 후보입니다.
			UCFEquipmentPresetData* HeavyCannonEquipment = FindCatalogEquipmentByAssetName(
				RuntimeCatalog,
				TEXT("HeavyCannon"));
			// [v1.0.0] 두 번째 Snapshot Equipment 적용 후보입니다.
			UCFEquipmentPresetData* RocketLauncherEquipment = FindCatalogEquipmentByAssetName(
				RuntimeCatalog,
				TEXT("RocketLauncher"));

			if (!Test->TestNotNull(TEXT("RTA-P0-05 fitting-ready Catalog Vehicle"), FittingReadyVehicleData)
				|| !Test->TestNotNull(TEXT("RTA-P0-05 alternate Catalog Vehicle"), AlternateVehicleData)
				|| !Test->TestNotNull(TEXT("RTA-P0-05 HeavyCannon Catalog Equipment"), HeavyCannonEquipment)
				|| !Test->TestNotNull(TEXT("RTA-P0-05 RocketLauncher Catalog Equipment"), RocketLauncherEquipment))
			{
				return true;
			}

			// [v1.0.0] 실제 Blueprint 컴포넌트 계층을 가진 VehiclePawn class입니다.
			UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
				nullptr,
				TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));
			if (!Test->TestNotNull(TEXT("RTA-P0-05 BP_CFVehiclePawn class"), VehiclePawnClass))
			{
				return true;
			}

			// [v1.0.0] PIE 복제 World에 테스트 Pawn을 다른 actor와 겹치지 않게 둘 위치/회전입니다.
			const FTransform SpawnTransform(
				FRotator::ZeroRotator,
				FVector(0.0, 0.0, 500.0),
				FVector::OneVector);
			// [v1.0.0] BeginPlay 전에 initial legacy VehicleData를 주입할 deferred 실제 VehiclePawn입니다.
			ACFVehiclePawn* VehiclePawn = PIEWorld->SpawnActorDeferred<ACFVehiclePawn>(
				VehiclePawnClass,
				SpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (!Test->TestNotNull(TEXT("RTA-P0-05 deferred VehiclePawn spawn"), VehiclePawn))
			{
				return true;
			}

			VehiclePawn->VehicleData = AlternateVehicleData;
			VehiclePawn->VehicleFittingData = nullptr;
			VehiclePawn->bAutoRegisterInputMappingContext = false;
			VehiclePawn->bShowAimReticle = false;
			VehiclePawn->bShowTargetSelectHud = false;
			VehiclePawn->FinishSpawning(SpawnTransform);

			// [v1.0.0] Vehicle/Equipment runtime 상태 전이를 readback할 실제 Fitting component입니다.
			UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
			if (!Test->TestNotNull(TEXT("RTA-P0-05 VehicleFittingComp"), VehicleFittingComp))
			{
				VehiclePawn->Destroy();
				return true;
			}

			VehicleFittingComp->ResetFittingRuntimeState();
			if (!Test->TestTrue(
				TEXT("RTA-P0-05 initial legacy Vehicle Runtime initialize"),
				VehiclePawn->InitializeVehicleRuntime()))
			{
				VehiclePawn->Destroy();
				return true;
			}

			Test->TestTrue(
				TEXT("RTA-P0-05 initial legacy Fitting Runtime committed"),
				VehicleFittingComp->HasAppliedRuntimeInput());
			Test->TestFalse(
				TEXT("RTA-P0-05 initial legacy Runtime has no Applied Snapshot"),
				VehicleFittingComp->HasAppliedFittingSnapshot());

			// [v1.0.0] 실제 RuntimeApply frontend public action을 검증할 pure C++ Widget입니다.
			UCFRuntimeApplyWidget* RuntimeApplyWidget =
				CreateWidget<UCFRuntimeApplyWidget>(
					PIEWorld,
					UCFRuntimeApplyWidget::StaticClass());
			if (!Test->TestNotNull(TEXT("RTA-P0-05 RuntimeApply Widget"), RuntimeApplyWidget))
			{
				VehiclePawn->Destroy();
				return true;
			}
			RuntimeApplyWidget->SetVehiclePawnRef(VehiclePawn);

			// [v1.0.0] A→B→A 첫 A 선택 결과입니다.
			const bool bSelectedFirstVehicleA =
				SelectExactPIEVehicle(RuntimeApplyWidget, FittingReadyVehicleData);
			Test->TestTrue(TEXT("RTA-P0-05 Vehicle A first selection"), bSelectedFirstVehicleA);
			// [v1.0.0] 첫 A explicit apply 결과입니다.
			const ECFRuntimeVehicleApplyStatus FirstVehicleAStatus =
				bSelectedFirstVehicleA
					? RuntimeApplyWidget->ApplySelectedVehicle()
					: ECFRuntimeVehicleApplyStatus::NotAttempted;
			Test->TestEqual(
				TEXT("RTA-P0-05 Vehicle A first Apply succeeds"),
				FirstVehicleAStatus,
				ECFRuntimeVehicleApplyStatus::Succeeded);

			// [v1.0.0] A→B→A의 B 선택 결과입니다.
			const bool bSelectedVehicleB =
				SelectExactPIEVehicle(RuntimeApplyWidget, AlternateVehicleData);
			Test->TestTrue(TEXT("RTA-P0-05 Vehicle B selection"), bSelectedVehicleB);
			// [v1.0.0] Vehicle B explicit apply 결과입니다.
			const ECFRuntimeVehicleApplyStatus VehicleBStatus =
				bSelectedVehicleB
					? RuntimeApplyWidget->ApplySelectedVehicle()
					: ECFRuntimeVehicleApplyStatus::NotAttempted;
			Test->TestEqual(
				TEXT("RTA-P0-05 Vehicle B Apply succeeds"),
				VehicleBStatus,
				ECFRuntimeVehicleApplyStatus::Succeeded);

			// [v1.0.0] A→B→A 마지막 A 선택 결과입니다.
			const bool bSelectedFinalVehicleA =
				SelectExactPIEVehicle(RuntimeApplyWidget, FittingReadyVehicleData);
			Test->TestTrue(TEXT("RTA-P0-05 Vehicle A final selection"), bSelectedFinalVehicleA);
			// [v1.0.0] 마지막 A explicit apply 결과입니다.
			const ECFRuntimeVehicleApplyStatus FinalVehicleAStatus =
				bSelectedFinalVehicleA
					? RuntimeApplyWidget->ApplySelectedVehicle()
					: ECFRuntimeVehicleApplyStatus::NotAttempted;
			Test->TestEqual(
				TEXT("RTA-P0-05 Vehicle A final Apply succeeds"),
				FinalVehicleAStatus,
				ECFRuntimeVehicleApplyStatus::Succeeded);
			Test->TestTrue(
				TEXT("RTA-P0-05 Vehicle A final Runtime ready"),
				VehiclePawn->bVehicleCoreRuntimeReady);
			Test->TestTrue(
				TEXT("RTA-P0-05 Vehicle A final source is transient"),
				VehiclePawn->VehicleData
					&& VehiclePawn->VehicleData->HasAnyFlags(RF_Transient));
			Test->TestTrue(
				TEXT("RTA-P0-05 Vehicle A final legacy Fitting Runtime committed"),
				VehicleFittingComp->HasAppliedRuntimeInput());
			Test->TestFalse(
				TEXT("RTA-P0-05 Vehicle A final still has no Applied Snapshot"),
				VehicleFittingComp->HasAppliedFittingSnapshot());
			Test->TestTrue(
				TEXT("RTA-P0-05 Vehicle A final Fitting source is cleared"),
				VehiclePawn->VehicleFittingData == nullptr);

			RuntimeApplyWidget->RefreshRuntimeApplyState();

			// [v1.0.0] DefenseSUV Equipment target으로 사용할 exact MountProfileId입니다.
			const FName TargetMountProfileId(TEXT("RoofTurret_MediumOrLarge"));
			// [v1.0.0] 최종 Vehicle A 기준 UI Mount option에서 exact target을 선택한 결과입니다.
			const bool bSelectedTargetMount =
				SelectExactPIEMount(RuntimeApplyWidget, TargetMountProfileId);
			Test->TestTrue(TEXT("RTA-P0-05 target Mount selection"), bSelectedTargetMount);

			// [v1.0.0] 첫 Equipment 후보 HeavyCannon을 UI에서 선택한 결과입니다.
			const bool bSelectedHeavyCannon =
				SelectExactPIEEquipment(RuntimeApplyWidget, HeavyCannonEquipment);
			Test->TestTrue(TEXT("RTA-P0-05 HeavyCannon selection"), bSelectedHeavyCannon);
			// [v1.0.0] Legacy→Snapshot 전환을 일으킬 첫 Equipment explicit apply 결과입니다.
			const ECFRuntimeEquipApplyStatus HeavyCannonStatus =
				bSelectedHeavyCannon
					? RuntimeApplyWidget->ApplySelectedEquipment()
					: ECFRuntimeEquipApplyStatus::NotAttempted;
			Test->TestEqual(
				TEXT("RTA-P0-05 HeavyCannon Apply succeeds"),
				HeavyCannonStatus,
				ECFRuntimeEquipApplyStatus::Succeeded);
			Test->TestTrue(
				TEXT("RTA-P0-05 Equipment Apply promotes to Applied Snapshot"),
				VehicleFittingComp->HasAppliedFittingSnapshot());
			Test->TestTrue(
				TEXT("RTA-P0-05 Equipment Apply keeps Applied Runtime input"),
				VehicleFittingComp->HasAppliedRuntimeInput());
			Test->TestTrue(
				TEXT("RTA-P0-05 transient Fitting source becomes active"),
				VehiclePawn->VehicleFittingData
					&& VehiclePawn->VehicleFittingData->HasAnyFlags(RF_Transient));
			Test->TestTrue(
				TEXT("RTA-P0-05 HeavyCannon applied at target Mount"),
				FindAppliedEquipmentAtMount(VehicleFittingComp, TargetMountProfileId)
					== HeavyCannonEquipment);
			Test->TestTrue(
				TEXT("RTA-P0-05 HeavyCannon UI Last Result success"),
				RuntimeApplyWidget->GetLastResultText().Contains(TEXT("성공")));

			// [v1.0.0] 실제 Chaos Movement의 final configured mass를 readback할 component입니다.
			UChaosWheeledVehicleMovementComponent* VehicleMovementComponent =
				Cast<UChaosWheeledVehicleMovementComponent>(
					VehiclePawn->GetVehicleMovementComponent());
			if (Test->TestNotNull(TEXT("RTA-P0-05 Chaos VehicleMovement"), VehicleMovementComponent))
			{
				// [v1.0.0] Vehicle 1000 + Turret 350 + Weapon 120 + Defense 100의 승인된 P0-05 Snapshot 총질량입니다.
				constexpr float ExpectedSnapshotMassKg = 1570.0f;
				Test->TestTrue(
					TEXT("RTA-P0-05 HeavyCannon Snapshot configured mass = 1570kg"),
					FMath::IsNearlyEqual(
						VehicleMovementComponent->Mass,
						ExpectedSnapshotMassKg,
						0.01f));
			}

			// [v1.0.0] 두 번째 Equipment 후보 RocketLauncher를 UI에서 선택한 결과입니다.
			const bool bSelectedRocketLauncher =
				SelectExactPIEEquipment(RuntimeApplyWidget, RocketLauncherEquipment);
			Test->TestTrue(TEXT("RTA-P0-05 RocketLauncher selection"), bSelectedRocketLauncher);
			// [v1.0.0] HeavyCannon→RocketLauncher explicit apply 결과입니다.
			const ECFRuntimeEquipApplyStatus RocketLauncherStatus =
				bSelectedRocketLauncher
					? RuntimeApplyWidget->ApplySelectedEquipment()
					: ECFRuntimeEquipApplyStatus::NotAttempted;
			Test->TestEqual(
				TEXT("RTA-P0-05 RocketLauncher Apply succeeds"),
				RocketLauncherStatus,
				ECFRuntimeEquipApplyStatus::Succeeded);
			Test->TestTrue(
				TEXT("RTA-P0-05 RocketLauncher applied at target Mount"),
				FindAppliedEquipmentAtMount(VehicleFittingComp, TargetMountProfileId)
					== RocketLauncherEquipment);

			// [v1.0.0] 반복 교체의 마지막 HeavyCannon을 다시 선택한 결과입니다.
			const bool bReselectedHeavyCannon =
				SelectExactPIEEquipment(RuntimeApplyWidget, HeavyCannonEquipment);
			Test->TestTrue(TEXT("RTA-P0-05 HeavyCannon reselection"), bReselectedHeavyCannon);
			// [v1.0.0] RocketLauncher→HeavyCannon 재적용 결과입니다.
			const ECFRuntimeEquipApplyStatus HeavyCannonReapplyStatus =
				bReselectedHeavyCannon
					? RuntimeApplyWidget->ApplySelectedEquipment()
					: ECFRuntimeEquipApplyStatus::NotAttempted;
			Test->TestEqual(
				TEXT("RTA-P0-05 HeavyCannon reapply succeeds"),
				HeavyCannonReapplyStatus,
				ECFRuntimeEquipApplyStatus::Succeeded);
			Test->TestTrue(
				TEXT("RTA-P0-05 HeavyCannon reapplied at target Mount"),
				FindAppliedEquipmentAtMount(VehicleFittingComp, TargetMountProfileId)
					== HeavyCannonEquipment);

			// [v1.0.0] invalid candidate fail-closed 전 exact Fitting source identity checkpoint입니다.
			UCFVehicleFittingData* FittingSourceBeforeInvalidApply =
				VehiclePawn->VehicleFittingData.Get();
			// [v1.0.0] invalid candidate fail-closed 전 실제 Chaos 설정 질량 checkpoint입니다.
			const float ConfiguredMassBeforeInvalidApply =
				VehicleMovementComponent ? VehicleMovementComponent->Mass : 0.0f;
			// [v1.0.0] Catalog에 등록되지 않아 authorization에서 거부되어야 하는 transient Equipment 후보입니다.
			UCFEquipmentPresetData* UnauthorizedEquipment =
				NewObject<UCFEquipmentPresetData>(GetTransientPackage());

			// [v1.0.0] Catalog authorization fail-closed 결과입니다.
			const FCFRuntimeEquipApplyResult InvalidApplyResult =
				FCFRuntimeEquipApplyService::ApplyCatalogEquipment(
					VehiclePawn,
					RuntimeCatalog,
					TargetMountProfileId,
					UnauthorizedEquipment);
			Test->TestEqual(
				TEXT("RTA-P0-05 unauthorized Equipment is ValidationFailed"),
				InvalidApplyResult.Status,
				ECFRuntimeEquipApplyStatus::ValidationFailed);
			Test->TestFalse(
				TEXT("RTA-P0-05 authorization failure does not start recovery"),
				InvalidApplyResult.bRecoveryAttempted);
			Test->TestTrue(
				TEXT("RTA-P0-05 invalid Apply preserves Fitting source identity"),
				VehiclePawn->VehicleFittingData.Get() == FittingSourceBeforeInvalidApply);
			Test->TestTrue(
				TEXT("RTA-P0-05 invalid Apply preserves current Equipment"),
				FindAppliedEquipmentAtMount(VehicleFittingComp, TargetMountProfileId)
					== HeavyCannonEquipment);
			if (VehicleMovementComponent)
			{
				Test->TestTrue(
					TEXT("RTA-P0-05 invalid Apply preserves configured mass"),
					FMath::IsNearlyEqual(
						VehicleMovementComponent->Mass,
						ConfiguredMassBeforeInvalidApply,
						0.01f));
			}

			RuntimeApplyWidget->RefreshRuntimeApplyState();
			Test->TestTrue(
				TEXT("RTA-P0-05 UI remains usable after invalid service request"),
				RuntimeApplyWidget->GetMountOptionCount() > 0
					&& RuntimeApplyWidget->GetEquipmentOptionCount() == 2);

			Test->AddInfo(
				TEXT("RTA-P0-05 PIEE2E PASS path executed: Vehicle DefenseSUV→TestSUV→DefenseSUV, Legacy→Snapshot, HeavyCannon→RocketLauncher→HeavyCannon, invalid Catalog Equipment fail-closed."));
			VehiclePawn->Destroy();
			return true;
		}

	private:
		// [v1.0.0] 검증 결과를 기록할 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;

		// [v1.0.0] PIE World 준비 제한시간을 계산할 latent command 시작 시각입니다.
		double StartTimeSeconds = 0.0;
	};

	// [v1.1.0] persisted Wagon Standard Mount를 RuntimeApply UI 첫 장비 적용까지 실제 PIE에서 검증합니다.
	class FCFWagonMountRuntimeApplyPIECommand final : public IAutomationLatentCommand
	{
	public:
		explicit FCFWagonMountRuntimeApplyPIECommand(FAutomationTestBase* InTest)
			: Test(InTest)
			, StartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		virtual bool Update() override
		{
			UWorld* PIEWorld = nullptr;
			if (GEngine)
			{
				for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
				{
					if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
					{
						PIEWorld = WorldContext.World();
						break;
					}
				}
			}

			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if ((FPlatformTime::Seconds() - StartTimeSeconds) >= 30.0)
				{
					Test->AddError(TEXT("CF-FQ-047 Wagon RuntimeApply PIE: 30초 안에 PIE World가 준비되지 않았습니다."));
					return true;
				}
				return false;
			}

			const UCFRuntimeTestSettings* RuntimeTestSettings = GetDefault<UCFRuntimeTestSettings>();
			UCFRuntimeTestCatalogData* RuntimeCatalog = RuntimeTestSettings ? RuntimeTestSettings->LoadDefaultCatalog() : nullptr;
			UCFVehicleData* Wagon = FindCatalogVehicleByAssetName(RuntimeCatalog, TEXT("DA_Vehicle_Wagon"));
			UCFEquipmentPresetData* HeavyCannon = FindCatalogEquipmentByAssetName(RuntimeCatalog, TEXT("HeavyCannon"));
			if (!Test->TestNotNull(TEXT("CF-FQ-047 default Runtime Catalog"), RuntimeCatalog)
				|| !Test->TestNotNull(TEXT("CF-FQ-047 catalog Wagon"), Wagon)
				|| !Test->TestNotNull(TEXT("CF-FQ-047 catalog HeavyCannon"), HeavyCannon))
			{
				return true;
			}

			Test->TestEqual(TEXT("CF-FQ-047 persisted Wagon Mount exact1"), Wagon->MountProfiles.Num(), 1);
			if (Wagon->MountProfiles.Num() != 1)
			{
				return true;
			}
			const FName TargetMountProfileId(TEXT("Mount_Top_01"));
			Test->TestEqual(TEXT("CF-FQ-047 persisted Wagon Mount ID"), Wagon->MountProfiles[0].MountProfileId, TargetMountProfileId);
			Test->TestTrue(
				TEXT("CF-FQ-047 HeavyCannon compatible with Wagon Mount"),
				HeavyCannon->CanUseOnMount(Wagon->MountProfiles[0].MountType, Wagon->MountProfiles[0].SizeLimit));

			UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
				nullptr,
				TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));
			if (!Test->TestNotNull(TEXT("CF-FQ-047 BP_CFVehiclePawn class"), VehiclePawnClass))
			{
				return true;
			}

			const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0.0, 500.0, 500.0), FVector::OneVector);
			ACFVehiclePawn* VehiclePawn = PIEWorld->SpawnActorDeferred<ACFVehiclePawn>(
				VehiclePawnClass,
				SpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (!Test->TestNotNull(TEXT("CF-FQ-047 Wagon Pawn spawn"), VehiclePawn))
			{
				return true;
			}

			VehiclePawn->VehicleData = Wagon;
			VehiclePawn->VehicleFittingData = nullptr;
			VehiclePawn->bAutoRegisterInputMappingContext = false;
			VehiclePawn->bShowAimReticle = false;
			VehiclePawn->bShowTargetSelectHud = false;
			VehiclePawn->FinishSpawning(SpawnTransform);

			UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
			UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
			if (!Test->TestNotNull(TEXT("CF-FQ-047 Wagon FittingComp"), VehicleFittingComp)
				|| !Test->TestNotNull(TEXT("CF-FQ-047 Wagon WeaponComp"), VehicleWeaponComp))
			{
				VehiclePawn->Destroy();
				return true;
			}

			VehicleFittingComp->ResetFittingRuntimeState();
			Test->TestTrue(TEXT("CF-FQ-047 Wagon core Runtime initializes"), VehiclePawn->InitializeVehicleRuntime());
			Test->TestTrue(TEXT("CF-FQ-047 Wagon legacy Fitting Runtime committed"), VehicleFittingComp->HasAppliedRuntimeInput());
			const FName ActiveMountBeforeEquipmentApply = VehicleWeaponComp->GetActiveMountProfileId();
			Test->TestNotEqual(
				TEXT("CF-FQ-047 pre-equipment active Mount is not yet Standard target"),
				ActiveMountBeforeEquipmentApply,
				TargetMountProfileId);

			UCFRuntimeApplyWidget* RuntimeApplyWidget = CreateWidget<UCFRuntimeApplyWidget>(
				PIEWorld,
				UCFRuntimeApplyWidget::StaticClass());
			if (!Test->TestNotNull(TEXT("CF-FQ-047 RuntimeApply Widget"), RuntimeApplyWidget))
			{
				VehiclePawn->Destroy();
				return true;
			}
			RuntimeApplyWidget->SetVehiclePawnRef(VehiclePawn);

			Test->TestEqual(TEXT("CF-FQ-047 Wagon UI Mount option exact1"), RuntimeApplyWidget->GetMountOptionCount(), 1);
			const bool bMountSelected = SelectExactPIEMount(RuntimeApplyWidget, TargetMountProfileId);
			const bool bEquipmentSelected = SelectExactPIEEquipment(RuntimeApplyWidget, HeavyCannon);
			Test->TestTrue(TEXT("CF-FQ-047 Mount_Top_01 selectable"), bMountSelected);
			Test->TestTrue(TEXT("CF-FQ-047 HeavyCannon selectable"), bEquipmentSelected);

			const ECFRuntimeEquipApplyStatus ApplyStatus =
				bMountSelected && bEquipmentSelected
					? RuntimeApplyWidget->ApplySelectedEquipment()
					: ECFRuntimeEquipApplyStatus::NotAttempted;
			Test->AddInfo(FString::Printf(
				TEXT("CF-FQ-047 Wagon Equipment diagnostic: Status=%d | UI=%s | Fitting=%s | ActiveMount=%s"),
				static_cast<int32>(ApplyStatus),
				*RuntimeApplyWidget->GetLastResultText(),
				*VehicleFittingComp->GetLastFittingRuntimeSummary(),
				*VehicleWeaponComp->GetActiveMountProfileId().ToString()));
			Test->TestEqual(
				TEXT("CF-FQ-047 Wagon overweight Equipment is validation-blocked"),
				ApplyStatus,
				ECFRuntimeEquipApplyStatus::ValidationFailed);
			Test->TestFalse(
				TEXT("CF-FQ-047 validation failure does not create Applied Snapshot"),
				VehicleFittingComp->HasAppliedFittingSnapshot());
			Test->TestTrue(
				TEXT("CF-FQ-047 validation failure leaves target Mount unequipped"),
				FindAppliedEquipmentAtMount(VehicleFittingComp, TargetMountProfileId) == nullptr);
			Test->TestEqual(
				TEXT("CF-FQ-047 validation failure preserves pre-apply active Mount"),
				VehicleWeaponComp->GetActiveMountProfileId(),
				ActiveMountBeforeEquipmentApply);
			Test->TestTrue(
				TEXT("CF-FQ-047 RuntimeApply reports GrossMass validation cause"),
				RuntimeApplyWidget->GetLastResultText().Contains(TEXT("최대 허용 총중량"))
					|| RuntimeApplyWidget->GetLastResultText().Contains(TEXT("검증 실패")));

			Test->AddInfo(TEXT("CF-FQ-047 Wagon RuntimeApply PIE PASS: persisted Mount_Top_01 is visible/selectable; production HeavyCannon is correctly blocked by GrossMassExceeded without mutating runtime state."));
			VehiclePawn->Destroy();
			return true;
		}

	private:
		FAutomationTestBase* Test = nullptr;
		double StartTimeSeconds = 0.0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeApplyPIEE2ETest,
	"CarFight.RuntimeApply.RTA_P0_05.PIEE2E",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 저장된 테스트 맵의 실제 PIE World에서 Runtime Apply 전체 연쇄를 검증합니다.
bool FCFRuntimeApplyPIEE2ETest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 PIE lifecycle을 시작할 읽기 전용 테스트 맵입니다.
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFRuntimeApplyPIEE2ECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWagonMountRuntimeApplyPIETest,
	"CarFight.RuntimeApply.CF_FQ_047.WagonMountEquipmentPIE",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFWagonMountRuntimeApplyPIETest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFWagonMountRuntimeApplyPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
