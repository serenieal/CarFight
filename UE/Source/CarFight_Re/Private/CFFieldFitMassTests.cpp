// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-034 FFIT-P0-04 실제 Chaos Vehicle Field Mass Reapply PIE 자동화 테스트
// Scope: 기존 M_VehicleDefensePIE의 저장된 테스트 SUV에서 Movement Mass를 임시 변경·Physics State 재생성·실측 검증한 뒤 같은 PIE 수명 안에서 원래 질량으로 복원합니다.
// Changelog:
// - v1.0.0: 실제 PIE Vehicle의 Configured/Actual Mass, Physics State, Transform, 미세 속도와 Runtime Ready 보존 및 원래 질량 복원을 최초 검증.
// Migration:
// - 테스트는 /Game/Maps/M_VehicleDefensePIE를 읽기 전용으로 사용하며 Map, Blueprint, DataAsset을 저장하거나 수정하지 않습니다.
// - 테스트 질량 변화는 PIE 복제 Actor에만 +50kg을 임시 적용하고 검증 직후 원래 Movement Mass로 복원합니다.
// - SetMassOverrideInKg, Body-only Override, PhysicsAsset·MassScale 저장 수정은 사용하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFieldFitCoordinator.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	/** 실제 PIE World에서 저장된 Defense SUV에 Field Mass를 임시 재적용하고 원래 질량 복원까지 검증합니다. */
	class FCFVerifyFieldMassPIECommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.0.0] 검증 결과를 기록할 Automation Test를 보존하고 PIE 준비 제한시간을 시작합니다.
		explicit FCFVerifyFieldMassPIECommand(FAutomationTestBase* InTest)
			: Test(InTest)
			, StartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// [v1.0.0] 실제 PIE 차량의 질량을 임시 변경·검증·복원한 뒤 latent command를 종료합니다.
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

			// [v1.0.0] PIE World와 대상 차량 초기화를 기다린 누적 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			// [v1.0.0] 맵 또는 차량 초기화 실패를 무한 대기하지 않을 제한 시간입니다.
			constexpr double PIEReadyTimeoutSeconds = 30.0;
			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if (ElapsedSeconds >= PIEReadyTimeoutSeconds)
				{
					Test->AddError(TEXT("FFIT-P0-04 ChaosMassPIE: 30초 안에 PIE World Actor 초기화가 완료되지 않았습니다."));
					return true;
				}
				return false;
			}

			// [v1.0.0] 저장 맵에서 Field Mass 기술 검증 대상으로 사용할 정확한 FittingData 경로입니다.
			const FString ExpectedFittingDataPath = TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV");
			// [v1.0.0] 정확한 FittingData를 가진 실제 PIE 복제 방어 SUV입니다.
			ACFVehiclePawn* TargetVehiclePawn = nullptr;
			for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(PIEWorld); VehiclePawnIterator; ++VehiclePawnIterator)
			{
				// [v1.0.0] 현재 순회 중인 실제 PIE Vehicle Pawn입니다.
				ACFVehiclePawn* CandidateVehiclePawn = *VehiclePawnIterator;
				if (!IsValid(CandidateVehiclePawn))
				{
					continue;
				}

				if (GetPathNameSafe(CandidateVehiclePawn->VehicleFittingData.Get()) == ExpectedFittingDataPath)
				{
					TargetVehiclePawn = CandidateVehiclePawn;
					break;
				}
			}

			if (!TargetVehiclePawn || !TargetVehiclePawn->HasActorBegunPlay())
			{
				if (ElapsedSeconds < PIEReadyTimeoutSeconds)
				{
					return false;
				}

				Test->AddError(TEXT("FFIT-P0-04 ChaosMassPIE: 30초 안에 DA_Fit_DefenseTestSUV PIE 차량을 찾지 못했습니다."));
				return true;
			}

			// [v1.0.0] 초기 질량 검증과 Applied Snapshot 상태를 제공하는 실제 PIE Fitting Component입니다.
			UCFVehicleFittingComp* VehicleFittingComponent = TargetVehiclePawn->GetVehicleFittingComp();
			// [v1.0.0] 실제 Chaos 설정 질량과 Vehicle Simulation Physics State를 제공하는 Movement Component입니다.
			UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(TargetVehiclePawn->GetVehicleMovementComponent());
			// [v1.0.0] 실제 PhysicsAsset 집계 질량·Physics State·속도를 제공하는 inherited VehicleMesh입니다.
			USkeletalMeshComponent* VehicleMeshComponent = TargetVehiclePawn->GetMesh();
			if (!VehicleFittingComponent
				|| !VehicleFittingComponent->HasVerifiedInitialMass()
				|| !VehicleFittingComponent->HasAppliedFittingSnapshot()
				|| !VehicleMovementComponent
				|| !VehicleMeshComponent
				|| !VehicleMovementComponent->HasValidPhysicsState()
				|| !VehicleMeshComponent->IsPhysicsStateCreated()
				|| !VehicleMeshComponent->IsSimulatingPhysics()
				|| !VehicleMeshComponent->GetPhysicsAsset())
			{
				Test->AddError(TEXT("FFIT-P0-04 ChaosMassPIE: 대상 차량의 Initial Mass 또는 Chaos Physics Runtime 선행조건이 준비되지 않았습니다."));
				return true;
			}

			// [v1.0.0] Field Mass 변경 전 Chaos Movement 설정 질량입니다.
			const float OriginalConfiguredMassKg = VehicleMovementComponent->Mass;
			// [v1.0.0] Field Mass 변경 전 PhysicsAsset 보조 Body를 포함한 실제 VehicleMesh 집계 질량입니다.
			const float OriginalActualMassKg = VehicleMeshComponent->GetMass();
			// [v1.0.0] 실제 Field Reapply를 증명하기 위해 원래 설정 질량에 더할 임시 질량입니다.
			constexpr float TemporaryMassDeltaKg = 50.0f;
			// [v1.0.0] 이번 PIE에서 임시 적용할 목표 Chaos Movement 질량입니다.
			const float TemporaryTargetMassKg = OriginalConfiguredMassKg + TemporaryMassDeltaKg;
			// [v1.0.0] PhysicsAsset 보조 Body 오버헤드를 보존했을 때 기대되는 임시 실제 집계 질량입니다.
			const float ExpectedTemporaryActualMassKg = OriginalActualMassKg + TemporaryMassDeltaKg;
			// [v1.0.0] Physics State 재생성 전 반드시 보존해야 할 Actor World Transform입니다.
			const FTransform OriginalActorTransform = TargetVehiclePawn->GetActorTransform();
			// [v1.0.0] Physics State 재생성 전 반드시 보존해야 할 chassis 선속도입니다.
			const FVector OriginalLinearVelocity = VehicleMeshComponent->GetPhysicsLinearVelocity();
			// [v1.0.0] Physics State 재생성 전 반드시 보존해야 할 chassis 각속도입니다.
			const FVector OriginalAngularVelocityDegrees = VehicleMeshComponent->GetPhysicsAngularVelocityInDegrees();
			// [v1.0.0] Field Mass 변경 전 차량 Core Runtime Ready 상태입니다.
			const bool bOriginalCoreRuntimeReady = TargetVehiclePawn->bVehicleCoreRuntimeReady;
			// [v1.0.0] Field Mass 변경 전 차량 Combat Runtime Ready 상태입니다.
			const bool bOriginalCombatRuntimeReady = TargetVehiclePawn->bVehicleCombatRuntimeReady;

			if (!FMath::IsFinite(OriginalConfiguredMassKg)
				|| OriginalConfiguredMassKg <= 0.0f
				|| !FMath::IsFinite(OriginalActualMassKg)
				|| OriginalActualMassKg <= 0.0f)
			{
				Test->AddError(TEXT("FFIT-P0-04 ChaosMassPIE: 원래 Configured/Actual Mass가 유효하지 않습니다."));
				return true;
			}

			// [v1.0.0] 실제 PIE Wheeled Vehicle에 Movement Mass를 재적용할 production Adapter입니다.
			FCFChaosVehicleMassRuntime MassRuntime(TargetVehiclePawn);
			// [v1.0.0] 임시 질량 적용 실패 상세를 받을 진단 문자열입니다.
			FString ApplyFailureSummary;
			// [v1.0.0] PIE 차량에 +50kg 임시 질량을 실제 적용·재생성·검증한 결과입니다.
			const bool bTemporaryMassApplied = MassRuntime.ReapplyVehicleMassKg(TemporaryTargetMassKg, ApplyFailureSummary);

			// [v1.0.0] 임시 적용 뒤 직접 readback한 Chaos 설정 질량입니다.
			const float TemporaryConfiguredMassKg = VehicleMovementComponent->Mass;
			// [v1.0.0] 임시 적용 뒤 직접 readback한 VehicleMesh 실제 집계 질량입니다.
			const float TemporaryActualMassKg = VehicleMeshComponent->GetMass();
			// [v1.0.0] 임시 적용 실제 질량 검증에 사용할 기존 FIT-P0-05 허용 오차입니다.
			const float TemporaryMassToleranceKg = UCFVehicleFittingComp::CalculateInitialMassToleranceKg(TemporaryTargetMassKg);

			if (bTemporaryMassApplied)
			{
				Test->TestTrue(TEXT("ChaosMassPIE 임시 Configured Mass 목표 일치"), FMath::IsNearlyEqual(TemporaryConfiguredMassKg, TemporaryTargetMassKg, 0.01f));
				Test->TestTrue(TEXT("ChaosMassPIE 임시 Actual Mass 변화 반영"), FMath::IsNearlyEqual(TemporaryActualMassKg, ExpectedTemporaryActualMassKg, TemporaryMassToleranceKg));
				Test->TestTrue(TEXT("ChaosMassPIE 임시 Physics State 유지"), VehicleMovementComponent->HasValidPhysicsState() && VehicleMeshComponent->IsPhysicsStateCreated());
				Test->TestTrue(TEXT("ChaosMassPIE 임시 Simulate Physics 유지"), VehicleMeshComponent->IsSimulatingPhysics());
				Test->TestTrue(TEXT("ChaosMassPIE 임시 PhysicsAsset 유지"), VehicleMeshComponent->GetPhysicsAsset() != nullptr);
				Test->TestTrue(TEXT("ChaosMassPIE 임시 Actor Transform 보존"), TargetVehiclePawn->GetActorTransform().Equals(OriginalActorTransform, 0.01f));
				Test->TestTrue(TEXT("ChaosMassPIE 임시 선속도 보존"), VehicleMeshComponent->GetPhysicsLinearVelocity().Equals(OriginalLinearVelocity, 0.1f));
				Test->TestTrue(TEXT("ChaosMassPIE 임시 각속도 보존"), VehicleMeshComponent->GetPhysicsAngularVelocityInDegrees().Equals(OriginalAngularVelocityDegrees, 0.1f));
				Test->TestEqual(TEXT("ChaosMassPIE 임시 Core Runtime Ready 보존"), TargetVehiclePawn->bVehicleCoreRuntimeReady, bOriginalCoreRuntimeReady);
				Test->TestEqual(TEXT("ChaosMassPIE 임시 Combat Runtime Ready 보존"), TargetVehiclePawn->bVehicleCombatRuntimeReady, bOriginalCombatRuntimeReady);
			}
			else
			{
				Test->AddError(FString::Printf(TEXT("FFIT-P0-04 ChaosMassPIE: 임시 질량 적용 실패: %s"), *ApplyFailureSummary));
			}

			// [v1.0.0] 임시 적용 성공/실패와 무관하게 같은 PIE 수명에서 원래 Movement Mass 복원을 시도할 진단 문자열입니다.
			FString RestoreFailureSummary;
			// [v1.0.0] 원래 질량·Physics State로 복구한 결과입니다.
			const bool bOriginalMassRestored = MassRuntime.ReapplyVehicleMassKg(OriginalConfiguredMassKg, RestoreFailureSummary);
			// [v1.0.0] 복원 뒤 직접 readback한 원래 Chaos 설정 질량입니다.
			const float RestoredConfiguredMassKg = VehicleMovementComponent->Mass;
			// [v1.0.0] 복원 뒤 직접 readback한 원래 VehicleMesh 실제 집계 질량입니다.
			const float RestoredActualMassKg = VehicleMeshComponent->GetMass();
			// [v1.0.0] 원래 실제 질량 복원 검증에 사용할 기존 FIT-P0-05 허용 오차입니다.
			const float RestoreMassToleranceKg = UCFVehicleFittingComp::CalculateInitialMassToleranceKg(OriginalConfiguredMassKg);

			Test->TestTrue(TEXT("ChaosMassPIE 원래 질량 복원 성공"), bOriginalMassRestored);
			if (!bOriginalMassRestored)
			{
				Test->AddError(FString::Printf(TEXT("FFIT-P0-04 ChaosMassPIE: 원래 질량 복원 실패: %s"), *RestoreFailureSummary));
			}
			Test->TestTrue(TEXT("ChaosMassPIE 복원 Configured Mass 일치"), FMath::IsNearlyEqual(RestoredConfiguredMassKg, OriginalConfiguredMassKg, 0.01f));
			Test->TestTrue(TEXT("ChaosMassPIE 복원 Actual Mass 일치"), FMath::IsNearlyEqual(RestoredActualMassKg, OriginalActualMassKg, RestoreMassToleranceKg));
			Test->TestTrue(TEXT("ChaosMassPIE 복원 Physics State 유지"), VehicleMovementComponent->HasValidPhysicsState() && VehicleMeshComponent->IsPhysicsStateCreated());
			Test->TestTrue(TEXT("ChaosMassPIE 복원 Simulate Physics 유지"), VehicleMeshComponent->IsSimulatingPhysics());
			Test->TestTrue(TEXT("ChaosMassPIE 복원 Actor Transform 보존"), TargetVehiclePawn->GetActorTransform().Equals(OriginalActorTransform, 0.01f));
			Test->TestEqual(TEXT("ChaosMassPIE 복원 Core Runtime Ready 보존"), TargetVehiclePawn->bVehicleCoreRuntimeReady, bOriginalCoreRuntimeReady);
			Test->TestEqual(TEXT("ChaosMassPIE 복원 Combat Runtime Ready 보존"), TargetVehiclePawn->bVehicleCombatRuntimeReady, bOriginalCombatRuntimeReady);

			Test->AddInfo(FString::Printf(
				TEXT("FFIT-P0-04 ChaosMassPIE | Actor=%s | OriginalConfigured=%.3f | OriginalActual=%.3f | TargetConfigured=%.3f | TargetActual=%.3f | RestoredConfigured=%.3f | RestoredActual=%.3f | Apply=%s | Restore=%s"),
				*TargetVehiclePawn->GetPathName(),
				OriginalConfiguredMassKg,
				OriginalActualMassKg,
				TemporaryConfiguredMassKg,
				TemporaryActualMassKg,
				RestoredConfiguredMassKg,
				RestoredActualMassKg,
				bTemporaryMassApplied ? TEXT("PASS") : TEXT("FAIL"),
				bOriginalMassRestored ? TEXT("PASS") : TEXT("FAIL")));
			return true;
		}

	private:
		// [v1.0.0] Latent 검증 결과를 기록할 현재 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;

		// [v1.0.0] PIE 준비 제한시간을 계산할 latent command 시작 시각입니다.
		double StartTimeSeconds = 0.0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFFieldFitChaosMassPIETest,
	"CarFight.Fitting.FFIT_P0_04.ChaosMassPIE",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 기존 저장 맵 PIE에서 실제 Chaos Vehicle 질량 재적용과 원상복구를 검증합니다.
bool FCFFieldFitChaosMassPIETest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 저장된 방어 SUV와 실제 Physics State를 재사용할 읽기 전용 테스트 맵입니다.
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFVerifyFieldMassPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
