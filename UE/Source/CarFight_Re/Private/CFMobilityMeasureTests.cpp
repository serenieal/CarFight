// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-17
// Description: CF-FQ-034 FIT-P0-07C 공식 Light / Default / Heavy 동일 조건 Chaos Mobility 정량 계측
// Scope: M_VehicleDefensePIE의 같은 DA_VehicleDefense_TestSUV 플랫폼을 Fixture별 fresh PIE lifetime으로 격리해 0→30km/h 가속, 30km/h→Idle 진입 속도 제동, 30km/h Steering 0.5 / 2초 Yaw를 계측합니다.
// Changelog:
// - v1.2.0: Light 완료 뒤 같은 Chaos Vehicle을 Default로 재사용할 때 drivetrain/physics 상태가 오염되는 실측 RCA를 반영해 Fixture마다 Map reload + fresh PIE lifetime을 사용하도록 격리했습니다.
// - v1.2.0: 각 fresh PIE에서 기존 PlayerController를 같은 공식 SUV 플랫폼 차량에 임시 Possess하고 Pawn 사용자 조향 Tick만 중지하며 측정 종료 전 원래 Possession을 복원합니다.
// - v1.1.0: 비소유 차량 Drive 입력 문제 대응을 위해 임시 Possess, 연속 DriveComp 입력과 PeakForward 진단을 추가했습니다.
// - v1.0.0: 공식 Fixture 3종의 동일 플랫폼 정량 Mobility PIE 계측과 machine-readable MobilityMetric 로그를 최초 추가했습니다.
// Migration:
// - 30km/h, Steering 0.5, 2초는 PASS 임계값이 아니라 세 Fixture에 동일하게 적용하는 측정 조건입니다.
// - 결과의 Light/Default/Heavy 순서나 최소 차이를 Automation PASS 조건으로 사용하지 않습니다.
// - 각 Fixture는 fresh PIE 복제 World에서만 변경되며 VehicleData, VehicleFittingData, Map, PhysicsAsset 또는 Production 질량값을 저장·변경하지 않습니다.
// - USER 주행감 PASS를 대체하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFFieldFitCoordinator.h"
#include "CFVehicleData.h"
#include "CFVehicleDriveComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	/** FIT-P0-07C가 한 fresh PIE Fixture에서 수집하는 정량 Mobility 결과입니다. */
	struct FCFMobilityMeasurementResult
	{
		// [v1.2.0] 사람이 식별할 공식 Fixture 구분 이름입니다.
		FString FixtureLabel;
		// [v1.2.0] Fixture Snapshot이 요구한 Chaos Movement 설정 질량입니다.
		float ConfiguredMassKg = 0.0f;
		// [v1.2.0] PhysicsAsset 보조 Body를 포함한 VehicleMesh 실제 집계 질량입니다.
		float ActualMassKg = 0.0f;
		// [v1.2.0] 정지 상태에서 30km/h에 처음 도달한 World simulation 시간입니다.
		float AccelerationTimeToReferenceSeconds = 0.0f;
		// [v1.2.0] 정지 상태에서 30km/h에 도달할 때까지 이동한 평면 거리(m)입니다.
		float AccelerationDistanceToReferenceMeters = 0.0f;
		// [v1.2.0] 제동 입력을 시작한 실제 프레임의 평면 속도(km/h)입니다.
		float BrakingStartSpeedKmh = 0.0f;
		// [v1.2.0] Brake 1.0 입력 뒤 DriveState Idle 진입 속도까지 걸린 시간입니다.
		float BrakingTimeSeconds = 0.0f;
		// [v1.2.0] Brake 1.0 입력 뒤 DriveState Idle 진입 속도까지 이동한 평면 거리(m)입니다.
		float BrakingDistanceMeters = 0.0f;
		// [v1.2.0] 조향 계측을 시작한 실제 프레임의 평면 속도(km/h)입니다.
		float SteeringStartSpeedKmh = 0.0f;
		// [v1.2.0] Steering 0.5를 2초 적용하는 동안 누적된 절대 Yaw 변화량(deg)입니다.
		float SteeringYawResponseDegrees = 0.0f;
		// [v1.2.0] 2초 조향 계측 종료 시점의 평면 속도(km/h)입니다.
		float SteeringEndSpeedKmh = 0.0f;
	};

	/** 한 fresh PIE lifetime 안에서 수행할 Mobility 계측 단계입니다. */
	enum class ECFMobilityMeasurePhase : uint8
	{
		WaitForPIE,
		SettleBeforeAcceleration,
		MeasureAcceleration,
		PrepareBraking,
		SettleBeforeBraking,
		ReachBrakingReferenceSpeed,
		MeasureBraking,
		PrepareSteering,
		SettleBeforeSteering,
		ReachSteeringReferenceSpeed,
		MeasureSteering,
		FinishMeasurement,
		Done
	};

	/** 한 공식 Fixture만 fresh PIE lifetime에서 실제 Chaos 주행으로 계측합니다. */
	class FCFMeasureSingleMobilityPIECommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.2.0] Automation Test, 공식 Fixture 이름과 저장 경로를 보존하고 PIE 준비 제한시간을 시작합니다.
		FCFMeasureSingleMobilityPIECommand(
			FAutomationTestBase* InTest,
			const FString& InFixtureLabel,
			const FString& InFittingPath)
			: Test(InTest)
			, FixtureLabel(InFixtureLabel)
			, FittingPath(InFittingPath)
			, CommandStartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// [v1.2.0] 현재 fresh PIE World를 한 프레임씩 진행하며 한 Fixture의 가속·제동·조향 수치를 계측합니다.
		virtual bool Update() override
		{
			if (Phase == ECFMobilityMeasurePhase::Done)
			{
				return true;
			}

			if (!PIEWorld)
			{
				PIEWorld = FindPIEWorld();
			}

			if (Phase == ECFMobilityMeasurePhase::WaitForPIE)
			{
				UpdateWaitForPIE();
				return Phase == ECFMobilityMeasurePhase::Done;
			}

			if (!PIEWorld || !TargetVehiclePawn || !VehicleDriveComponent || !VehicleMovementComponent || !VehicleMeshComponent)
			{
				FailMeasurement(TEXT("FIT-P0-07C: 계측 도중 PIE Vehicle Runtime 참조가 유실되었습니다."));
				return true;
			}

			switch (Phase)
			{
			case ECFMobilityMeasurePhase::SettleBeforeAcceleration:
				UpdateSettleBeforeAcceleration();
				break;
			case ECFMobilityMeasurePhase::MeasureAcceleration:
				UpdateMeasureAcceleration();
				break;
			case ECFMobilityMeasurePhase::PrepareBraking:
				UpdatePrepareBraking();
				break;
			case ECFMobilityMeasurePhase::SettleBeforeBraking:
				UpdateSettleBeforeBraking();
				break;
			case ECFMobilityMeasurePhase::ReachBrakingReferenceSpeed:
				UpdateReachBrakingReferenceSpeed();
				break;
			case ECFMobilityMeasurePhase::MeasureBraking:
				UpdateMeasureBraking();
				break;
			case ECFMobilityMeasurePhase::PrepareSteering:
				UpdatePrepareSteering();
				break;
			case ECFMobilityMeasurePhase::SettleBeforeSteering:
				UpdateSettleBeforeSteering();
				break;
			case ECFMobilityMeasurePhase::ReachSteeringReferenceSpeed:
				UpdateReachSteeringReferenceSpeed();
				break;
			case ECFMobilityMeasurePhase::MeasureSteering:
				UpdateMeasureSteering();
				break;
			case ECFMobilityMeasurePhase::FinishMeasurement:
				UpdateFinishMeasurement();
				break;
			case ECFMobilityMeasurePhase::WaitForPIE:
			case ECFMobilityMeasurePhase::Done:
			default:
				break;
			}

			return Phase == ECFMobilityMeasurePhase::Done;
		}

	private:
		// [v1.2.0] 현재 Engine Context에서 실제 PIE World를 찾습니다.
		UWorld* FindPIEWorld() const
		{
			if (!GEngine)
			{
				return nullptr;
			}

			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
				{
					return WorldContext.World();
				}
			}
			return nullptr;
		}

		// [v1.2.0] 저장 M_VehicleDefensePIE에서 실제 DA_VehicleDefense_TestSUV + DA_Fit_DefenseTestSUV 기준 차량을 찾습니다.
		ACFVehiclePawn* FindTargetVehiclePawn() const
		{
			if (!PIEWorld)
			{
				return nullptr;
			}

			// [v1.2.0] 저장 맵의 기존 동일 플랫폼 차량을 식별할 FittingData 경로입니다.
			const FString ExistingMapFittingPath = TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV");
			for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(PIEWorld); VehiclePawnIterator; ++VehiclePawnIterator)
			{
				// [v1.2.0] 현재 순회 중인 실제 PIE Vehicle Pawn입니다.
				ACFVehiclePawn* CandidateVehiclePawn = *VehiclePawnIterator;
				if (!IsValid(CandidateVehiclePawn))
				{
					continue;
				}
				if (GetPathNameSafe(CandidateVehiclePawn->VehicleFittingData.Get()) == ExistingMapFittingPath)
				{
					return CandidateVehiclePawn;
				}
			}
			return nullptr;
		}

		// [v1.2.0] 현재 Fixture 저장 자산에서 유효 Snapshot을 만들고 대상 차량과 동일 VehicleData 플랫폼인지 확인합니다.
		bool LoadCurrentFixtureSnapshot()
		{
			// [v1.2.0] 현재 fresh PIE에서 적용할 공식 VehicleFittingData입니다.
			UCFVehicleFittingData* FittingData = LoadObject<UCFVehicleFittingData>(nullptr, *FittingPath);
			if (!FittingData)
			{
				return false;
			}

			FittingSnapshot = FittingData->BuildFittingSnapshot();
			if (!FittingSnapshot.IsValid() || !FittingSnapshot.VehicleData || !TargetVehiclePawn || !TargetVehiclePawn->VehicleData)
			{
				return false;
			}
			return FittingSnapshot.VehicleData == TargetVehiclePawn->VehicleData;
		}

		// [v1.2.0] fresh PIE World·차량·Fixture·Possession·Chaos 질량 선행조건을 준비합니다.
		void UpdateWaitForPIE()
		{
			// [v1.2.0] PIE 준비를 기다린 실제 wall-clock 누적 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - CommandStartTimeSeconds;
			// [v1.2.0] World 또는 차량 준비 실패를 무한 대기하지 않을 기술 제한시간입니다.
			constexpr double PIEReadyTimeoutSeconds = 30.0;
			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if (ElapsedSeconds >= PIEReadyTimeoutSeconds)
				{
					FailMeasurement(TEXT("FIT-P0-07C: 30초 안에 PIE World Actor 초기화가 완료되지 않았습니다."));
				}
				return;
			}

			if (!TargetVehiclePawn)
			{
				TargetVehiclePawn = FindTargetVehiclePawn();
			}
			if (!TargetVehiclePawn || !TargetVehiclePawn->HasActorBegunPlay())
			{
				if (ElapsedSeconds >= PIEReadyTimeoutSeconds)
				{
					FailMeasurement(TEXT("FIT-P0-07C: 30초 안에 M_VehicleDefensePIE 기준 차량을 찾지 못했습니다."));
				}
				return;
			}

			VehicleDriveComponent = TargetVehiclePawn->GetVehicleDriveComp();
			VehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(TargetVehiclePawn->GetVehicleMovementComponent());
			VehicleMeshComponent = TargetVehiclePawn->GetMesh();
			if (!VehicleDriveComponent || !VehicleMovementComponent || !VehicleMeshComponent
				|| !VehicleMovementComponent->HasValidPhysicsState()
				|| !VehicleMeshComponent->IsPhysicsStateCreated()
				|| !VehicleMeshComponent->IsSimulatingPhysics()
				|| !VehicleMeshComponent->GetPhysicsAsset())
			{
				if (ElapsedSeconds >= PIEReadyTimeoutSeconds)
				{
					FailMeasurement(TEXT("FIT-P0-07C: 실제 Chaos Vehicle Physics Runtime 선행조건이 준비되지 않았습니다."));
				}
				return;
			}

			if (!LoadCurrentFixtureSnapshot())
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s 공식 Fixture Snapshot 또는 동일 VehicleData 플랫폼 계약을 준비하지 못했습니다."), *FixtureLabel));
				return;
			}

			OriginalActorTransform = TargetVehiclePawn->GetActorTransform();
			// [v1.2.0] 제동 종료를 판정할 현재 VehicleData의 기존 DriveState Idle 진입 속도입니다.
			StopSpeedThresholdKmh = TargetVehiclePawn->VehicleData->DriveStateConfig.IdleEnterSpeedThresholdKmh;
			if (!FMath::IsFinite(StopSpeedThresholdKmh) || StopSpeedThresholdKmh < 0.0f)
			{
				FailMeasurement(TEXT("FIT-P0-07C: DriveState Idle 진입 속도 기준이 유효하지 않습니다."));
				return;
			}

			// [v1.2.0] 비소유 동일 플랫폼 차량을 실제 Chaos 주행 입력이 가능한 로컬 소유 상태로 만들 기존 PIE PlayerController입니다.
			MeasurementPlayerController = PIEWorld->GetFirstPlayerController();
			if (!MeasurementPlayerController)
			{
				FailMeasurement(TEXT("FIT-P0-07C: PIE PlayerController를 찾지 못했습니다."));
				return;
			}
			// [v1.2.0] 계측 종료 전 되돌릴 원래 PIE Player Pawn입니다.
			OriginalPlayerPawn = MeasurementPlayerController->GetPawn();
			// [v1.2.0] 계측 대상 Pawn의 원래 Actor Tick 활성 상태입니다.
			bOriginalTargetActorTickEnabled = TargetVehiclePawn->IsActorTickEnabled();
			MeasurementPlayerController->Possess(TargetVehiclePawn);
			bMeasurementPossessionApplied = TargetVehiclePawn->GetController() == MeasurementPlayerController;
			if (!bMeasurementPossessionApplied || !TargetVehiclePawn->IsLocallyControlled())
			{
				FailMeasurement(TEXT("FIT-P0-07C: 동일 플랫폼 대상 차량의 임시 Possess가 성립하지 않았습니다."));
				return;
			}
			// [v1.2.0] Pawn Tick의 사용자 조향 보정만 멈추고 Chaos Movement Component Tick은 유지합니다.
			TargetVehiclePawn->SetActorTickEnabled(false);

			MassRuntime = MakeUnique<FCFChaosVehicleMassRuntime>(TargetVehiclePawn);
			ResetVehicleForMeasurement();
			// [v1.2.0] production Mass Runtime 실패 원인을 보존할 진단 문자열입니다.
			FString FailureSummary;
			if (!MassRuntime->ReapplyVehicleMassKg(FittingSnapshot.TotalVehicleMassKg, FailureSummary))
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s Fixture Mass 적용 실패: %s"), *FixtureLabel, *FailureSummary));
				return;
			}
			ResetVehicleForMeasurement();

			Result.FixtureLabel = FixtureLabel;
			Result.ConfiguredMassKg = VehicleMovementComponent->Mass;
			Result.ActualMassKg = VehicleMeshComponent->GetMass();
			BeginPhaseMeasurement();
			Phase = ECFMobilityMeasurePhase::SettleBeforeAcceleration;
		}

		// [v1.2.0] 현재 차량의 평면 속도를 km/h로 반환합니다.
		float GetPlanarSpeedKmh() const
		{
			return VehicleMeshComponent ? VehicleMeshComponent->GetPhysicsLinearVelocity().Size2D() * 0.036f : 0.0f;
		}

		// [v1.2.0] 현재 차량 Forward 축 방향의 양수 속도를 km/h로 반환합니다.
		float GetForwardSpeedKmh() const
		{
			if (!VehicleMeshComponent || !TargetVehiclePawn)
			{
				return 0.0f;
			}
			// [v1.2.0] 현재 chassis 선속도입니다.
			const FVector LinearVelocity = VehicleMeshComponent->GetPhysicsLinearVelocity();
			// [v1.2.0] 현재 Actor Forward 축에 투영한 전진 속도(cm/s)입니다.
			const float ForwardSpeedCmPerSec = FVector::DotProduct(LinearVelocity, TargetVehiclePawn->GetActorForwardVector());
			return FMath::Max(0.0f, ForwardSpeedCmPerSec * 0.036f);
		}

		// [v1.2.0] 각 측정 구간 시작 전에 위치·속도·입력을 같은 원점 기준으로 초기화합니다.
		void ResetVehicleForMeasurement()
		{
			if (!TargetVehiclePawn || !VehicleMeshComponent || !VehicleDriveComponent)
			{
				return;
			}
			VehicleDriveComponent->ClearDriveInputs();
			TargetVehiclePawn->SetActorTransform(OriginalActorTransform, false, nullptr, ETeleportType::TeleportPhysics);
			VehicleMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
			VehicleMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
		}

		// [v1.2.0] 현재 World simulation time을 반환합니다.
		float GetWorldTimeSeconds() const
		{
			return PIEWorld ? PIEWorld->GetTimeSeconds() : 0.0f;
		}

		// [v1.2.0] 현재 Phase의 시작 시각과 시작 위치를 기록합니다.
		void BeginPhaseMeasurement()
		{
			PhaseStartWorldTimeSeconds = GetWorldTimeSeconds();
			PhaseStartLocation = TargetVehiclePawn ? TargetVehiclePawn->GetActorLocation() : FVector::ZeroVector;
		}

		// [v1.2.0] 현재 Phase 시작 후 경과한 World simulation time입니다.
		float GetPhaseElapsedSeconds() const
		{
			return GetWorldTimeSeconds() - PhaseStartWorldTimeSeconds;
		}

		// [v1.2.0] 현재 Phase 시작 위치에서 평면으로 이동한 거리(m)를 반환합니다.
		float GetPhaseDistanceMeters() const
		{
			if (!TargetVehiclePawn)
			{
				return 0.0f;
			}
			return FVector::Dist2D(PhaseStartLocation, TargetVehiclePawn->GetActorLocation()) * 0.01f;
		}

		// [v1.2.0] 질량 재생성 뒤 1초 안정화 후 정확한 정지 상태에서 0→30km/h 가속 계측을 시작합니다.
		void UpdateSettleBeforeAcceleration()
		{
			// [v1.2.0] 질량 적용과 서스펜션 초기화 뒤 공통으로 기다릴 안정화 시간입니다.
			constexpr float SettleDurationSeconds = 1.0f;
			if (GetPhaseElapsedSeconds() < SettleDurationSeconds)
			{
				return;
			}
			ResetVehicleForMeasurement();
			BeginPhaseMeasurement();
			PeakForwardSpeedKmh = 0.0f;
			VehicleDriveComponent->ApplyThrottleInput(1.0f);
			Phase = ECFMobilityMeasurePhase::MeasureAcceleration;
		}

		// [v1.2.0] 정지 상태에서 공통 30km/h 기준 속도까지 걸린 시간과 거리를 계측합니다.
		void UpdateMeasureAcceleration()
		{
			// [v1.2.0] 가속·제동·조향의 공통 시작 속도로 사용할 측정 조건이며 PASS 임계값이 아닙니다.
			constexpr float ReferenceSpeedKmh = 30.0f;
			// [v1.2.0] 계측 환경 이상으로 기준 속도에 도달하지 못할 때 무한 대기하지 않을 기술 제한시간입니다.
			constexpr float AccelerationTimeoutSeconds = 20.0f;
			VehicleDriveComponent->ApplyThrottleInput(1.0f);
			// [v1.2.0] 현재 Forward 기준 속도입니다.
			const float CurrentForwardSpeedKmh = GetForwardSpeedKmh();
			PeakForwardSpeedKmh = FMath::Max(PeakForwardSpeedKmh, CurrentForwardSpeedKmh);
			if (CurrentForwardSpeedKmh >= ReferenceSpeedKmh)
			{
				Result.AccelerationTimeToReferenceSeconds = GetPhaseElapsedSeconds();
				Result.AccelerationDistanceToReferenceMeters = GetPhaseDistanceMeters();
				VehicleDriveComponent->ClearDriveInputs();
				Phase = ECFMobilityMeasurePhase::PrepareBraking;
				return;
			}
			if (GetPhaseElapsedSeconds() >= AccelerationTimeoutSeconds)
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s Fixture가 20초 안에 30km/h에 도달하지 못했습니다. PeakForward=%.6fkm/h, Local=%s, Controller=%s"), *FixtureLabel, PeakForwardSpeedKmh, TargetVehiclePawn->IsLocallyControlled() ? TEXT("Yes") : TEXT("No"), *GetNameSafe(TargetVehiclePawn->GetController())));
			}
		}

		// [v1.2.0] 제동 계측을 위해 같은 fresh PIE Fixture·같은 시작 Transform·0 속도로 차량을 재설정합니다.
		void UpdatePrepareBraking()
		{
			ResetVehicleForMeasurement();
			BeginPhaseMeasurement();
			Phase = ECFMobilityMeasurePhase::SettleBeforeBraking;
		}

		// [v1.2.0] 제동 준비 가속 전에 0.5초 안정화해 이전 가속 구간의 물리 잔류를 제거합니다.
		void UpdateSettleBeforeBraking()
		{
			// [v1.2.0] 재측정 구간 사이 공통 짧은 안정화 시간입니다.
			constexpr float SettleDurationSeconds = 0.5f;
			if (GetPhaseElapsedSeconds() < SettleDurationSeconds)
			{
				return;
			}
			ResetVehicleForMeasurement();
			BeginPhaseMeasurement();
			VehicleDriveComponent->ApplyThrottleInput(1.0f);
			Phase = ECFMobilityMeasurePhase::ReachBrakingReferenceSpeed;
		}

		// [v1.2.0] 제동 시작 조건을 동일한 30km/h로 맞춥니다.
		void UpdateReachBrakingReferenceSpeed()
		{
			// [v1.2.0] 제동 시작에 사용할 공통 측정 속도입니다.
			constexpr float ReferenceSpeedKmh = 30.0f;
			// [v1.2.0] 기준 속도 준비 실패를 무한 대기하지 않을 기술 제한시간입니다.
			constexpr float ReferenceSpeedTimeoutSeconds = 20.0f;
			VehicleDriveComponent->ApplyThrottleInput(1.0f);
			// [v1.2.0] 현재 Forward 기준 속도입니다.
			const float CurrentForwardSpeedKmh = GetForwardSpeedKmh();
			if (CurrentForwardSpeedKmh >= ReferenceSpeedKmh)
			{
				VehicleDriveComponent->ApplyThrottleInput(0.0f);
				VehicleDriveComponent->ApplyBrakeInput(1.0f);
				Result.BrakingStartSpeedKmh = GetPlanarSpeedKmh();
				BeginPhaseMeasurement();
				Phase = ECFMobilityMeasurePhase::MeasureBraking;
				return;
			}
			if (GetPhaseElapsedSeconds() >= ReferenceSpeedTimeoutSeconds)
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s Fixture가 제동 준비 중 30km/h에 도달하지 못했습니다."), *FixtureLabel));
			}
		}

		// [v1.2.0] Brake 1.0 입력부터 VehicleData의 기존 Idle 진입 속도까지 시간·거리를 측정합니다.
		void UpdateMeasureBraking()
		{
			// [v1.2.0] 제동 구간이 비정상적으로 끝나지 않을 때 무한 대기하지 않을 기술 제한시간입니다.
			constexpr float BrakingTimeoutSeconds = 15.0f;
			VehicleDriveComponent->ApplyThrottleInput(0.0f);
			VehicleDriveComponent->ApplyBrakeInput(1.0f);
			// [v1.2.0] 현재 평면 속도입니다.
			const float CurrentPlanarSpeedKmh = GetPlanarSpeedKmh();
			if (CurrentPlanarSpeedKmh <= StopSpeedThresholdKmh)
			{
				Result.BrakingTimeSeconds = GetPhaseElapsedSeconds();
				Result.BrakingDistanceMeters = GetPhaseDistanceMeters();
				VehicleDriveComponent->ClearDriveInputs();
				Phase = ECFMobilityMeasurePhase::PrepareSteering;
				return;
			}
			if (GetPhaseElapsedSeconds() >= BrakingTimeoutSeconds)
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s Fixture가 15초 안에 Idle 진입 속도 %.3fkm/h까지 감속하지 못했습니다."), *FixtureLabel, StopSpeedThresholdKmh));
			}
		}

		// [v1.2.0] 조향 계측을 위해 같은 fresh PIE Fixture·같은 시작 Transform·0 속도로 차량을 재설정합니다.
		void UpdatePrepareSteering()
		{
			ResetVehicleForMeasurement();
			BeginPhaseMeasurement();
			Phase = ECFMobilityMeasurePhase::SettleBeforeSteering;
		}

		// [v1.2.0] 조향 준비 가속 전에 0.5초 안정화해 이전 제동 구간의 물리 잔류를 제거합니다.
		void UpdateSettleBeforeSteering()
		{
			// [v1.2.0] 재측정 구간 사이 공통 짧은 안정화 시간입니다.
			constexpr float SettleDurationSeconds = 0.5f;
			if (GetPhaseElapsedSeconds() < SettleDurationSeconds)
			{
				return;
			}
			ResetVehicleForMeasurement();
			BeginPhaseMeasurement();
			VehicleDriveComponent->ApplyThrottleInput(1.0f);
			Phase = ECFMobilityMeasurePhase::ReachSteeringReferenceSpeed;
		}

		// [v1.2.0] 조향 시작 조건을 동일한 30km/h로 맞춥니다.
		void UpdateReachSteeringReferenceSpeed()
		{
			// [v1.2.0] 조향 시작에 사용할 공통 측정 속도입니다.
			constexpr float ReferenceSpeedKmh = 30.0f;
			// [v1.2.0] 기준 속도 준비 실패를 무한 대기하지 않을 기술 제한시간입니다.
			constexpr float ReferenceSpeedTimeoutSeconds = 20.0f;
			VehicleDriveComponent->ApplyThrottleInput(1.0f);
			// [v1.2.0] 현재 Forward 기준 속도입니다.
			const float CurrentForwardSpeedKmh = GetForwardSpeedKmh();
			if (CurrentForwardSpeedKmh >= ReferenceSpeedKmh)
			{
				VehicleDriveComponent->ApplyThrottleInput(0.0f);
				VehicleDriveComponent->ApplyBrakeInput(0.0f);
				VehicleDriveComponent->ApplySteeringInput(0.5f);
				Result.SteeringStartSpeedKmh = GetPlanarSpeedKmh();
				LastSteeringYawDegrees = TargetVehiclePawn->GetActorRotation().Yaw;
				AccumulatedSteeringYawDegrees = 0.0f;
				BeginPhaseMeasurement();
				Phase = ECFMobilityMeasurePhase::MeasureSteering;
				return;
			}
			if (GetPhaseElapsedSeconds() >= ReferenceSpeedTimeoutSeconds)
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s Fixture가 조향 준비 중 30km/h에 도달하지 못했습니다."), *FixtureLabel));
			}
		}

		// [v1.2.0] 30km/h에서 Throttle 0 / Steering 0.5를 2초 적용해 누적 Yaw 반응을 계측합니다.
		void UpdateMeasureSteering()
		{
			// [v1.2.0] 세 Fixture에 동일 적용하는 조향 계측 시간이며 PASS 임계값이 아닙니다.
			constexpr float SteeringMeasurementDurationSeconds = 2.0f;
			VehicleDriveComponent->ApplyThrottleInput(0.0f);
			VehicleDriveComponent->ApplyBrakeInput(0.0f);
			VehicleDriveComponent->ApplySteeringInput(0.5f);
			// [v1.2.0] 현재 Actor Yaw 각도입니다.
			const float CurrentYawDegrees = TargetVehiclePawn->GetActorRotation().Yaw;
			// [v1.2.0] -180/180 wrap을 안전하게 통과하는 이번 프레임 Signed Yaw 변화량입니다.
			const float FrameYawDeltaDegrees = FMath::FindDeltaAngleDegrees(LastSteeringYawDegrees, CurrentYawDegrees);
			AccumulatedSteeringYawDegrees += FrameYawDeltaDegrees;
			LastSteeringYawDegrees = CurrentYawDegrees;
			if (GetPhaseElapsedSeconds() < SteeringMeasurementDurationSeconds)
			{
				return;
			}

			Result.SteeringYawResponseDegrees = FMath::Abs(AccumulatedSteeringYawDegrees);
			Result.SteeringEndSpeedKmh = GetPlanarSpeedKmh();
			VehicleDriveComponent->ClearDriveInputs();
			Phase = ECFMobilityMeasurePhase::FinishMeasurement;
		}

		// [v1.2.0] 현재 Fixture 수치가 유한한지 기술 검증하고 machine-readable 한 줄 로그를 기록합니다.
		void UpdateFinishMeasurement()
		{
			// [v1.2.0] 결과에 NaN/Inf가 들어왔는지 확인하는 기술 유효성입니다.
			const bool bMetricsFinite =
				FMath::IsFinite(Result.ConfiguredMassKg)
				&& FMath::IsFinite(Result.ActualMassKg)
				&& FMath::IsFinite(Result.AccelerationTimeToReferenceSeconds)
				&& FMath::IsFinite(Result.AccelerationDistanceToReferenceMeters)
				&& FMath::IsFinite(Result.BrakingStartSpeedKmh)
				&& FMath::IsFinite(Result.BrakingTimeSeconds)
				&& FMath::IsFinite(Result.BrakingDistanceMeters)
				&& FMath::IsFinite(Result.SteeringStartSpeedKmh)
				&& FMath::IsFinite(Result.SteeringYawResponseDegrees)
				&& FMath::IsFinite(Result.SteeringEndSpeedKmh);
			if (!bMetricsFinite)
			{
				FailMeasurement(FString::Printf(TEXT("FIT-P0-07C: %s Fixture 계측 결과에 비유한 수치가 포함됐습니다."), *FixtureLabel));
				return;
			}

			if (Test)
			{
				Test->AddInfo(FString::Printf(
					TEXT("FIT-P0-07C MobilityMetric | Fixture=%s | ConfiguredMassKg=%.3f | ActualMassKg=%.3f | AccelReferenceKmh=30.000 | AccelTimeSec=%.6f | AccelDistanceM=%.6f | BrakeStartKmh=%.6f | BrakeStopKmh=%.6f | BrakeTimeSec=%.6f | BrakeDistanceM=%.6f | SteeringStartKmh=%.6f | SteeringInput=0.500 | SteeringDurationSec=2.000 | SteeringYawDeg=%.6f | SteeringEndKmh=%.6f"),
					*Result.FixtureLabel,
					Result.ConfiguredMassKg,
					Result.ActualMassKg,
					Result.AccelerationTimeToReferenceSeconds,
					Result.AccelerationDistanceToReferenceMeters,
					Result.BrakingStartSpeedKmh,
					StopSpeedThresholdKmh,
					Result.BrakingTimeSeconds,
					Result.BrakingDistanceMeters,
					Result.SteeringStartSpeedKmh,
					Result.SteeringYawResponseDegrees,
					Result.SteeringEndSpeedKmh));
			}
			RestoreMeasurementControl();
			Phase = ECFMobilityMeasurePhase::Done;
		}

		// [v1.2.0] 기술 실패를 기록하고 transient Drive 입력·Possession을 복원한 뒤 현재 fresh PIE 측정을 종료합니다.
		void FailMeasurement(const FString& FailureMessage)
		{
			if (Test)
			{
				Test->AddError(FailureMessage);
			}
			RestoreMeasurementControl();
			Phase = ECFMobilityMeasurePhase::Done;
		}

		// [v1.2.0] 계측용 임시 로컬 소유권과 Pawn Tick 상태를 현재 fresh PIE의 원래 상태로 되돌립니다.
		void RestoreMeasurementControl()
		{
			if (bMeasurementControlRestored)
			{
				return;
			}
			if (VehicleDriveComponent)
			{
				VehicleDriveComponent->ClearDriveInputs();
			}
			if (TargetVehiclePawn)
			{
				TargetVehiclePawn->SetActorTickEnabled(bOriginalTargetActorTickEnabled);
			}
			if (bMeasurementPossessionApplied && MeasurementPlayerController)
			{
				MeasurementPlayerController->UnPossess();
				if (IsValid(OriginalPlayerPawn))
				{
					MeasurementPlayerController->Possess(OriginalPlayerPawn);
				}
			}
			bMeasurementPossessionApplied = false;
			bMeasurementControlRestored = true;
		}

		// [v1.2.0] 결과와 오류를 기록할 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
		// [v1.2.0] 현재 fresh PIE가 계측하는 공식 Fixture 이름입니다.
		FString FixtureLabel;
		// [v1.2.0] 현재 fresh PIE가 로드할 공식 VehicleFittingData 저장 경로입니다.
		FString FittingPath;
		// [v1.2.0] PIE 준비 wall-clock 제한시간의 기준 시각입니다.
		double CommandStartTimeSeconds = 0.0;
		// [v1.2.0] 현재 실제 Play-In-Editor World입니다.
		UWorld* PIEWorld = nullptr;
		// [v1.2.0] M_VehicleDefensePIE에서 계측할 실제 동일 플랫폼 Chaos Vehicle Pawn입니다.
		ACFVehiclePawn* TargetVehiclePawn = nullptr;
		// [v1.2.0] 비소유 대상 차량을 계측 동안만 로컬 소유하게 할 기존 PIE PlayerController입니다.
		APlayerController* MeasurementPlayerController = nullptr;
		// [v1.2.0] 계측 종료 뒤 PlayerController가 다시 소유할 원래 Player Pawn입니다.
		APawn* OriginalPlayerPawn = nullptr;
		// [v1.2.0] Gameplay Drive 입력을 production 경로로 전달할 VehicleDriveComp입니다.
		UCFVehicleDriveComp* VehicleDriveComponent = nullptr;
		// [v1.2.0] Configured Mass와 실제 Chaos Vehicle Simulation을 제공하는 Movement Component입니다.
		UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = nullptr;
		// [v1.2.0] 실제 속도·집계 질량·Physics State를 제공하는 chassis VehicleMesh입니다.
		USkeletalMeshComponent* VehicleMeshComponent = nullptr;
		// [v1.2.0] 현재 공식 Fixture에서 직접 생성한 결정론적 Fitting Snapshot입니다.
		FCFVehicleFittingSnapshot FittingSnapshot;
		// [v1.2.0] 동일 PIE Vehicle의 질량을 저장하지 않고 transient하게 재적용하는 production Adapter입니다.
		TUniquePtr<FCFChaosVehicleMassRuntime> MassRuntime;
		// [v1.2.0] 현재 fresh PIE Fixture에서 작성 중인 정량 계측 결과입니다.
		FCFMobilityMeasurementResult Result;
		// [v1.2.0] 현재 latent 계측 진행 단계입니다.
		ECFMobilityMeasurePhase Phase = ECFMobilityMeasurePhase::WaitForPIE;
		// [v1.2.0] 현재 측정 Phase 시작 World simulation time입니다.
		float PhaseStartWorldTimeSeconds = 0.0f;
		// [v1.2.0] 현재 측정 Phase 시작 Actor 위치입니다.
		FVector PhaseStartLocation = FVector::ZeroVector;
		// [v1.2.0] 제동 종료 판정에 사용할 VehicleData DriveState Idle 진입 속도입니다.
		float StopSpeedThresholdKmh = 0.0f;
		// [v1.2.0] Steering Yaw 누적 계산에서 직전 프레임 Yaw입니다.
		float LastSteeringYawDegrees = 0.0f;
		// [v1.2.0] Steering 계측 2초 동안 wrap-safe로 누적한 Signed Yaw 변화량입니다.
		float AccumulatedSteeringYawDegrees = 0.0f;
		// [v1.2.0] 현재 Fixture 가속 구간에서 관측한 최대 Forward 속도로 실패 RCA에만 사용합니다.
		float PeakForwardSpeedKmh = 0.0f;
		// [v1.2.0] 현재 fresh PIE 시작 시 동일 플랫폼 차량의 원래 Actor Transform입니다.
		FTransform OriginalActorTransform = FTransform::Identity;
		// [v1.2.0] 계측 대상 Pawn의 원래 Actor Tick 활성 상태입니다.
		bool bOriginalTargetActorTickEnabled = true;
		// [v1.2.0] 테스트가 대상 차량에 임시 Possession을 실제 적용했는지 여부입니다.
		bool bMeasurementPossessionApplied = false;
		// [v1.2.0] 현재 fresh PIE의 임시 Drive/Possession 복원을 이미 수행했는지 여부입니다.
		bool bMeasurementControlRestored = false;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFQuantitativeMobilityPIETest,
	"CarFight.Fitting.FIT_P0_07C.QuantitativeMobility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] 공식 Light·Default·Heavy를 각각 fresh PIE lifetime에서 같은 측정 프로토콜로 순차 계측합니다.
bool FCFQuantitativeMobilityPIETest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] 세 fresh PIE lifetime 모두 재사용할 읽기 전용 기술 계측 맵입니다.
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFMeasureSingleMobilityPIECommand(
		this,
		TEXT("Light"),
		TEXT("/Game/CarFight/Tests/Fitting/DA_Fit_MobilityLight.DA_Fit_MobilityLight")));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFMeasureSingleMobilityPIECommand(
		this,
		TEXT("Default"),
		TEXT("/Game/CarFight/Tests/Fitting/DA_Fit_MobilityDefault.DA_Fit_MobilityDefault")));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFMeasureSingleMobilityPIECommand(
		this,
		TEXT("Heavy"),
		TEXT("/Game/CarFight/Tests/Fitting/DA_Fit_MobilityHeavy.DA_Fit_MobilityHeavy")));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
