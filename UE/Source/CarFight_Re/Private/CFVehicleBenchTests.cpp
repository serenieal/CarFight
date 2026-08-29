// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBenchTests.cpp
// Version: v1.9.0
// Date: 2026-08-27
// Description: CF-FQ-040 VB-P0-08 saved VehicleData를 fresh PIE 실제 Chaos Vehicle로 계측하는 Technical Driving Benchmark입니다.
// Scope: Runtime Mass, 0→50/100km/h, peak/top-speed stability, 100→Idle braking, steady-turn yaw, effective turning radius, peak RPM/gear를 측정합니다.
// Changelog:
// - v1.9.0: initial 0→100을 달성한 차량은 같은 top-speed trajectory에서 현재 속도가 100 아래면 throttle로 재확보하고, 위면 coast로 접근해 100km/h 부근에서 Brake 1.0을 적용한다. top-speed 종료 순간 속도만으로 braking unavailable을 오판하던 조건을 제거.
// - v1.8.0: 100→Idle braking은 별도 reset/re-acceleration을 제거하고 동일 top-speed trajectory에서 throttle 0으로 100km/h까지 coast한 뒤 Brake 1.0을 적용해 측정한다. 100km/h 도달 가능한 차량의 braking phase state divergence를 제거.
// - v1.7.0: Heavy PhysicsState reset과 Light kinematic reset을 분리해 각 metric을 Heavy reset → settle → Light reset → drive 순서로 실행한다. settle 뒤 PhysicsState를 다시 만들어 안정화 효과를 무효화하던 구조를 교정.
// - v1.6.0: test map의 기존 Defense SUV는 spawn transform을 제공한 뒤 PIE lifetime에서 collision/physics/tick을 비활성화하고 격리해 benchmark 차량과의 환경 간섭을 제거.
// - v1.5.0: PhysicsState 재생성/Teleport 직후 transient velocity가 PeakSpeed로 오염되지 않도록 Peak Speed/RPM/Gear telemetry를 acceleration/top-speed active drive phase로만 제한.
// - v1.4.0: 각 독립 metric reset에서 production FCFChaosVehicleMassRuntime으로 동일 질량을 재적용해 Chaos PhysicsState를 재생성하고, top-speed 이후 wheel/transmission 내부 상태가 후속 yaw/turning metric에 잔류하지 않도록 결정론적 reset을 강화.
// - v1.3.0: 공통 reset에서 Chaos TargetGear를 Neutral(0)로 즉시 초기화하고, 모든 forward phase는 explicit 1단에서 시작한다. 이후 Automatic은 Chaos auto-shift, Manual은 configured ChangeUpRPM/ChangeDownRPM test-only driver로 제어해 phase 간 gear 잔류와 최초 전진 타이밍 편차를 방지.
// - v1.2.0: braking/yaw/turning 진입 전 0.5초 settle + second reset을 추가해 이전 phase drivetrain/physics 잔류가 후속 metric을 흔드는 반복 계측 편차를 줄임.
// - v1.1.0: Fitting 미지정 시 invalid transient Fitting Snapshot을 만들지 않고 VehicleData.BaseVehicleMassKg를 explicit runtime mass target으로 사용하며, 50/100km/h 미도달은 차량 성능 관측 결과(-1)로 남기고 기술 실패로 오판하지 않도록 교정.
// - v1.0.0: arbitrary saved VehicleData + optional FittingData command-line target을 deferred-spawn하는 Builder benchmark Automation 최초 구현.
// Migration:
// - Reference fact와의 PASS/FAIL threshold를 이 테스트가 임의 생성하지 않습니다. 이 테스트는 기술적으로 유효한 runtime metric만 기록합니다.
// - Product VehicleData/Fitting/Map/PhysicsAsset을 저장하거나 수정하지 않습니다. 모든 runtime 변경은 fresh PIE lifetime에만 존재합니다.
// - USER Driving feel PASS를 대체하지 않습니다.

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
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	/** VB-P0-08 한 VehicleData의 실제 Chaos runtime metric 결과입니다. */
	struct FCFBuilderDrivingBenchmarkResult
	{
		// 사람이 결과를 식별할 benchmark label입니다.
		FString Label;
		// 실제 benchmark target VehicleData path입니다.
		FString VehicleDataPath;
		// optional FittingData path입니다. 비어 있으면 VehicleData.BaseVehicleMassKg를 runtime mass target으로 사용합니다.
		FString FittingDataPath;
		// Movement Component에 적용된 configured mass입니다.
		float ConfiguredMassKg = 0.0f;
		// Skeletal VehicleMesh가 보고한 actual physics mass입니다.
		float ActualMassKg = 0.0f;
		// 정지 상태에서 50km/h에 처음 도달한 시간입니다.
		float Acceleration0To50Seconds = -1.0f;
		// 정지 상태에서 100km/h에 처음 도달한 시간입니다. 미도달이면 -1입니다.
		float Acceleration0To100Seconds = -1.0f;
		// Full throttle 관측 구간의 peak forward speed입니다.
		float PeakSpeedKmh = 0.0f;
		// Top-speed 후반 5초 변화량이 작았는지 여부입니다.
		bool bTopSpeedStable = false;
		// 100km/h braking 계측이 실제 수행됐는지 여부입니다.
		bool bBraking100Available = false;
		// Brake 1.0 입력 직전 실제 시작 속도입니다.
		float BrakingStartSpeedKmh = 0.0f;
		// 100km/h 근처에서 DriveState Idle threshold까지 감속한 시간입니다.
		float Braking100ToIdleSeconds = -1.0f;
		// 같은 braking 구간의 planar 이동 거리입니다.
		float Braking100ToIdleDistanceMeters = -1.0f;
		// 30km/h, Steering 0.5, 2초 구간의 누적 yaw 반응입니다.
		float SteadyYawResponseDegrees = 0.0f;
		// 저속 full-steer 구간의 effective turning radius입니다.
		float EffectiveTurningRadiusMeters = -1.0f;
		// Turning radius 측정 구간의 평균 planar speed입니다.
		float TurningAverageSpeedKmh = 0.0f;
		// 전체 benchmark에서 관측한 peak engine RPM입니다.
		float PeakEngineRpm = 0.0f;
		// Peak speed가 갱신된 시점의 actual Chaos current gear입니다.
		int32 PeakSpeedGear = 0;
	};

	/** VB-P0-08 fresh PIE benchmark phase입니다. */
	enum class ECFBuilderDrivingBenchmarkPhase : uint8
	{
		WaitForPIE,
		SettleBeforeAcceleration,
		MeasureAcceleration,
		MeasureTopSpeed,
		PrepareBraking,
		ReachBrakingSpeed,
		MeasureBraking,
		PrepareYaw,
		SettleBeforeYaw,
		ReachYawSpeed,
		MeasureYaw,
		PrepareTurning,
		SettleBeforeTurning,
		ReachTurningSpeed,
		MeasureTurning,
		Finish,
		Done
	};

	/** 한 saved VehicleData를 fresh PIE의 새 BP_CFVehiclePawn에 주입해 실제 Chaos mobility를 계측합니다. */
	class FCFBuilderDrivingBenchmarkCommand final : public IAutomationLatentCommand
	{
	public:
		// Test와 exact target object path/label을 보존하고 benchmark 준비 시간을 시작합니다.
		FCFBuilderDrivingBenchmarkCommand(
			FAutomationTestBase* InTest,
			const FString& InVehicleDataPath,
			const FString& InFittingDataPath,
			const FString& InLabel)
			: Test(InTest)
			, VehicleDataPath(InVehicleDataPath)
			, FittingDataPath(InFittingDataPath)
			, Label(InLabel)
			, CommandStartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// fresh PIE를 한 프레임씩 진행하며 benchmark state machine을 실행합니다.
		virtual bool Update() override
		{
			if (Phase == ECFBuilderDrivingBenchmarkPhase::Done)
			{
				return true;
			}

			if (!PIEWorld)
			{
				PIEWorld = FindPIEWorld();
			}

			if (Phase == ECFBuilderDrivingBenchmarkPhase::WaitForPIE)
			{
				UpdateWaitForPIE();
				return Phase == ECFBuilderDrivingBenchmarkPhase::Done;
			}

			if (!PIEWorld || !TargetVehiclePawn || !VehicleDriveComponent || !VehicleMovementComponent || !VehicleMeshComponent)
			{
				FailBenchmark(TEXT("VB-P0-08: benchmark 도중 PIE Vehicle Runtime 참조가 유실되었습니다."));
				return true;
			}

			if (Phase == ECFBuilderDrivingBenchmarkPhase::MeasureAcceleration
				|| Phase == ECFBuilderDrivingBenchmarkPhase::MeasureTopSpeed)
			{
				UpdatePeakRuntimeTelemetry();
			}

			switch (Phase)
			{
			case ECFBuilderDrivingBenchmarkPhase::SettleBeforeAcceleration:
				UpdateSettleBeforeAcceleration();
				break;
			case ECFBuilderDrivingBenchmarkPhase::MeasureAcceleration:
				UpdateMeasureAcceleration();
				break;
			case ECFBuilderDrivingBenchmarkPhase::MeasureTopSpeed:
				UpdateMeasureTopSpeed();
				break;
			case ECFBuilderDrivingBenchmarkPhase::PrepareBraking:
				UpdatePrepareBraking();
				break;
			case ECFBuilderDrivingBenchmarkPhase::ReachBrakingSpeed:
				UpdateReachBrakingSpeed();
				break;
			case ECFBuilderDrivingBenchmarkPhase::MeasureBraking:
				UpdateMeasureBraking();
				break;
			case ECFBuilderDrivingBenchmarkPhase::PrepareYaw:
				UpdatePrepareYaw();
				break;
			case ECFBuilderDrivingBenchmarkPhase::SettleBeforeYaw:
				UpdateSettleBeforeYaw();
				break;
			case ECFBuilderDrivingBenchmarkPhase::ReachYawSpeed:
				UpdateReachYawSpeed();
				break;
			case ECFBuilderDrivingBenchmarkPhase::MeasureYaw:
				UpdateMeasureYaw();
				break;
			case ECFBuilderDrivingBenchmarkPhase::PrepareTurning:
				UpdatePrepareTurning();
				break;
			case ECFBuilderDrivingBenchmarkPhase::SettleBeforeTurning:
				UpdateSettleBeforeTurning();
				break;
			case ECFBuilderDrivingBenchmarkPhase::ReachTurningSpeed:
				UpdateReachTurningSpeed();
				break;
			case ECFBuilderDrivingBenchmarkPhase::MeasureTurning:
				UpdateMeasureTurning();
				break;
			case ECFBuilderDrivingBenchmarkPhase::Finish:
				UpdateFinish();
				break;
			case ECFBuilderDrivingBenchmarkPhase::WaitForPIE:
			case ECFBuilderDrivingBenchmarkPhase::Done:
			default:
				break;
			}
			return Phase == ECFBuilderDrivingBenchmarkPhase::Done;
		}

	private:
		// 현재 Engine Context에서 actual PIE World를 찾습니다.
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

		// Test map의 검증된 Chaos vehicle spawn transform을 기존 defense SUV actor에서 읽고 actor는 PIE 안에서 멀리 격리합니다.
		bool ResolveBenchmarkSpawnTransform(FTransform& OutTransform)
		{
			if (!PIEWorld)
			{
				return false;
			}
			for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(PIEWorld); VehiclePawnIterator; ++VehiclePawnIterator)
			{
				// 현재 순회 중인 map vehicle candidate입니다.
				ACFVehiclePawn* CandidateVehiclePawn = *VehiclePawnIterator;
				if (!IsValid(CandidateVehiclePawn))
				{
					continue;
				}
				// P0 benchmark 전용 test map에서 기존 physics-safe spawn transform을 제공하는 fitting path입니다.
				const FString ExistingMapFittingPath = TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV");
				if (GetPathNameSafe(CandidateVehiclePawn->VehicleFittingData.Get()) != ExistingMapFittingPath)
				{
					continue;
				}
				OutTransform = CandidateVehiclePawn->GetActorTransform();
				// benchmark spawn transform만 사용하고 기존 map fixture가 다시 낙하·충돌하지 않도록 PIE lifetime에서 완전 격리합니다.
				CandidateVehiclePawn->SetActorEnableCollision(false);
				CandidateVehiclePawn->SetActorTickEnabled(false);
				CandidateVehiclePawn->SetActorHiddenInGame(true);
				if (USkeletalMeshComponent* ExistingFixtureMesh = CandidateVehiclePawn->GetMesh())
				{
					ExistingFixtureMesh->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
					ExistingFixtureMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
					ExistingFixtureMesh->SetSimulatePhysics(false);
				}
				CandidateVehiclePawn->SetActorLocation(OutTransform.GetLocation() + FVector(0.0, 0.0, 10000.0), false, nullptr, ETeleportType::TeleportPhysics);
				return true;
			}
			return false;
		}

		// saved VehicleData와 optional Fitting을 BeginPlay 전에 주입한 benchmark Pawn을 deferred-spawn합니다.
		bool SpawnBenchmarkVehicle(FString& OutError)
		{
			// benchmark target saved VehicleData입니다.
			UCFVehicleData* VehicleDataAsset = LoadObject<UCFVehicleData>(nullptr, *VehicleDataPath);
			if (!VehicleDataAsset)
			{
				OutError = FString::Printf(TEXT("VB-P0-08 VehicleData를 load할 수 없습니다: %s"), *VehicleDataPath);
				return false;
			}

			// benchmark에 사용할 optional saved fitting입니다. 미지정이면 VehicleData Base mass를 직접 runtime target으로 사용합니다.
			UCFVehicleFittingData* FittingData = nullptr;
			if (!FittingDataPath.IsEmpty())
			{
				FittingData = LoadObject<UCFVehicleFittingData>(nullptr, *FittingDataPath);
				if (!FittingData || FittingData->VehicleData != VehicleDataAsset)
				{
					OutError = TEXT("VB-P0-08 FittingData가 없거나 target VehicleData와 일치하지 않습니다.");
					return false;
				}
				// Fitting이 지정된 경우 장비 포함 총중량의 authoritative snapshot입니다.
				FittingSnapshot = FittingData->BuildFittingSnapshot();
				if (!FittingSnapshot.IsValid() || FittingSnapshot.VehicleData != VehicleDataAsset)
				{
					OutError = TEXT("VB-P0-08 Fitting Snapshot이 target VehicleData 기준으로 유효하지 않습니다.");
					return false;
				}
				BenchmarkTargetMassKg = FittingSnapshot.TotalVehicleMassKg;
			}
			else
			{
				// Builder 신규 차량의 no-fitting benchmark는 VehicleData의 validated base vehicle mass를 사용합니다.
				BenchmarkTargetMassKg = VehicleDataAsset->BaseVehicleMassKg;
				if (!FMath::IsFinite(BenchmarkTargetMassKg) || BenchmarkTargetMassKg <= 0.0f)
				{
					OutError = TEXT("VB-P0-08 Fitting 미지정 benchmark에는 유효한 VehicleData.BaseVehicleMassKg가 필요합니다.");
					return false;
				}
			}

			// 실제 Builder benchmark가 사용할 공용 production Vehicle Pawn Blueprint class입니다.
			UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
				nullptr,
				TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));
			if (!VehiclePawnClass)
			{
				OutError = TEXT("VB-P0-08 BP_CFVehiclePawn class를 load할 수 없습니다.");
				return false;
			}

			// test map의 검증된 road/physics 시작 transform입니다.
			FTransform SpawnTransform;
			if (!ResolveBenchmarkSpawnTransform(SpawnTransform))
			{
				OutError = TEXT("VB-P0-08 test map에서 benchmark spawn transform을 찾을 수 없습니다.");
				return false;
			}

			// BeginPlay 전에 VehicleData/Fitting을 설정할 deferred Pawn입니다.
			TargetVehiclePawn = PIEWorld->SpawnActorDeferred<ACFVehiclePawn>(
				VehiclePawnClass,
				SpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (!TargetVehiclePawn)
			{
				OutError = TEXT("VB-P0-08 benchmark Vehicle Pawn deferred spawn에 실패했습니다.");
				return false;
			}

			TargetVehiclePawn->VehicleData = VehicleDataAsset;
			TargetVehiclePawn->VehicleFittingData = FittingData;
			TargetVehiclePawn->bAutoInitializeOnBeginPlay = true;
			TargetVehiclePawn->bAutoRegisterInputMappingContext = false;
			TargetVehiclePawn->bShowAimReticle = false;
			TargetVehiclePawn->bShowTargetSelectHud = false;
			UGameplayStatics::FinishSpawningActor(TargetVehiclePawn, SpawnTransform);

			return true;
		}

		// PIE/vehicle/physics/possession 선행조건을 준비합니다.
		void UpdateWaitForPIE()
		{
			// benchmark 준비를 무한 대기하지 않을 wall-clock 제한입니다.
			constexpr double ReadyTimeoutSeconds = 30.0;
			// command 시작 뒤 실제 wall-clock 경과 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - CommandStartTimeSeconds;
			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if (ElapsedSeconds >= ReadyTimeoutSeconds)
				{
					FailBenchmark(TEXT("VB-P0-08: 30초 안에 PIE World가 준비되지 않았습니다."));
				}
				return;
			}

			if (!TargetVehiclePawn)
			{
				// target pawn spawn diagnostic입니다.
				FString SpawnError;
				if (!SpawnBenchmarkVehicle(SpawnError))
				{
					FailBenchmark(SpawnError);
					return;
				}
			}

			if (!TargetVehiclePawn->HasActorBegunPlay())
			{
				if (ElapsedSeconds >= ReadyTimeoutSeconds)
				{
					FailBenchmark(TEXT("VB-P0-08: benchmark Pawn BeginPlay가 준비되지 않았습니다."));
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
				if (ElapsedSeconds >= ReadyTimeoutSeconds)
				{
					FailBenchmark(TEXT("VB-P0-08: actual Chaos physics runtime가 준비되지 않았습니다."));
				}
				return;
			}

			MeasurementPlayerController = PIEWorld->GetFirstPlayerController();
			if (!MeasurementPlayerController)
			{
				FailBenchmark(TEXT("VB-P0-08: PIE PlayerController를 찾을 수 없습니다."));
				return;
			}
			OriginalPlayerPawn = MeasurementPlayerController->GetPawn();
			bOriginalTargetActorTickEnabled = TargetVehiclePawn->IsActorTickEnabled();
			MeasurementPlayerController->Possess(TargetVehiclePawn);
			bMeasurementPossessionApplied = TargetVehiclePawn->GetController() == MeasurementPlayerController;
			if (!bMeasurementPossessionApplied || !TargetVehiclePawn->IsLocallyControlled())
			{
				FailBenchmark(TEXT("VB-P0-08: benchmark Pawn 임시 Possess가 성립하지 않았습니다."));
				return;
			}
			TargetVehiclePawn->SetActorTickEnabled(false);

			MassRuntime = MakeUnique<FCFChaosVehicleMassRuntime>(TargetVehiclePawn);
			// production mass apply 실패 diagnostic입니다.
			FString MassError;
			if (!MassRuntime->ReapplyVehicleMassKg(BenchmarkTargetMassKg, MassError))
			{
				FailBenchmark(FString::Printf(TEXT("VB-P0-08 mass apply 실패: %s"), *MassError));
				return;
			}

			OriginalActorTransform = TargetVehiclePawn->GetActorTransform();
			StopSpeedThresholdKmh = TargetVehiclePawn->VehicleData->DriveStateConfig.IdleEnterSpeedThresholdKmh;
			if (!FMath::IsFinite(StopSpeedThresholdKmh) || StopSpeedThresholdKmh < 0.0f)
			{
				FailBenchmark(TEXT("VB-P0-08 DriveState Idle threshold가 유효하지 않습니다."));
				return;
			}

			Result.Label = Label;
			Result.VehicleDataPath = VehicleDataPath;
			Result.FittingDataPath = FittingDataPath;
			Result.ConfiguredMassKg = VehicleMovementComponent->Mass;
			Result.ActualMassKg = VehicleMeshComponent->GetMass();
			if (!ResetVehicleKinematics())
			{
				return;
			}
			BeginPhase();
			Phase = ECFBuilderDrivingBenchmarkPhase::SettleBeforeAcceleration;
		}

		// 현재 chassis planar speed를 km/h로 반환합니다.
		float GetPlanarSpeedKmh() const
		{
			return VehicleMeshComponent ? VehicleMeshComponent->GetPhysicsLinearVelocity().Size2D() * 0.036f : 0.0f;
		}

		// 현재 chassis velocity의 Actor Forward 양수 성분을 km/h로 반환합니다.
		float GetForwardSpeedKmh() const
		{
			if (!VehicleMeshComponent || !TargetVehiclePawn)
			{
				return 0.0f;
			}
			// current chassis linear velocity입니다.
			const FVector LinearVelocity = VehicleMeshComponent->GetPhysicsLinearVelocity();
			// Actor Forward 축에 projection한 cm/s입니다.
			const float ForwardSpeedCmPerSec = FVector::DotProduct(LinearVelocity, TargetVehiclePawn->GetActorForwardVector());
			return FMath::Max(0.0f, ForwardSpeedCmPerSec * 0.036f);
		}

		// phase 시작 후 World simulation time입니다.
		float GetPhaseElapsedSeconds() const
		{
			return PIEWorld ? PIEWorld->GetTimeSeconds() - PhaseStartWorldTimeSeconds : 0.0f;
		}

		// phase 시작 위치로부터 planar 직선 거리를 m로 반환합니다.
		float GetPhaseDistanceMeters() const
		{
			return TargetVehiclePawn
				? FVector::Dist2D(PhaseStartLocation, TargetVehiclePawn->GetActorLocation()) * 0.01f
				: 0.0f;
		}

		// current phase time/location 기준을 갱신합니다.
		void BeginPhase()
		{
			PhaseStartWorldTimeSeconds = PIEWorld ? PIEWorld->GetTimeSeconds() : 0.0f;
			PhaseStartLocation = TargetVehiclePawn ? TargetVehiclePawn->GetActorLocation() : FVector::ZeroVector;
		}

		// PhysicsState를 건드리지 않고 입력·gear·Transform·선속도·각속도만 benchmark 공통 정지 상태로 되돌립니다.
		bool ResetVehicleKinematics()
		{
			if (!TargetVehiclePawn || !VehicleMeshComponent || !VehicleDriveComponent || !VehicleMovementComponent)
			{
				FailBenchmark(TEXT("VB-P0-08: kinematic reset에 필요한 Vehicle Runtime 참조가 없습니다."));
				return false;
			}

			VehicleDriveComponent->ClearDriveInputs();
			VehicleMovementComponent->SetTargetGear(0, true);
			TargetVehiclePawn->SetActorTransform(OriginalActorTransform, false, nullptr, ETeleportType::TeleportPhysics);
			VehicleMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
			VehicleMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
			return true;
		}

		// 독립 metric 시작 전에 production mass adapter로 Chaos PhysicsState를 새로 만들고 공통 kinematic 상태를 다시 확정합니다.
		bool RecreateBenchmarkPhysicsState()
		{
			if (!MassRuntime)
			{
				FailBenchmark(TEXT("VB-P0-08: PhysicsState reset에 필요한 Mass Runtime이 없습니다."));
				return false;
			}
			if (!ResetVehicleKinematics())
			{
				return false;
			}

			// top-speed/제동 이후 wheel·engine·transmission 내부 simulation state까지 새로 만들기 위해 동일 질량을 production lifecycle 경로로 재적용합니다.
			FString PhysicsResetError;
			if (!MassRuntime->ReapplyVehicleMassKg(BenchmarkTargetMassKg, PhysicsResetError))
			{
				FailBenchmark(FString::Printf(TEXT("VB-P0-08 metric PhysicsState reset 실패: %s"), *PhysicsResetError));
				return false;
			}

			return ResetVehicleKinematics();
		}

		// 모든 benchmark forward phase를 explicit 1단에서 시작하고, 이후 Automatic은 Chaos auto-shift에 맡기며 Manual만 configured shift RPM으로 제어합니다.
		void ApplyBenchmarkForwardThrottle(const float ThrottleValue)
		{
			if (!VehicleDriveComponent || !VehicleMovementComponent)
			{
				return;
			}

			// 현재 configured gearbox의 forward gear 개수입니다.
			const int32 MaximumForwardGear = VehicleMovementComponent->TransmissionSetup.ForwardGearRatios.Num();
			// 현재 Chaos target gear입니다.
			const int32 CurrentTargetGear = VehicleMovementComponent->GetTargetGear();
			if (MaximumForwardGear > 0 && CurrentTargetGear <= 0)
			{
				// Neutral reset 직후 Automatic/Manual 모두 첫 forward frame을 결정론적인 1단에서 시작합니다.
				VehicleMovementComponent->SetTargetGear(1, true);
			}
			else if (!VehicleMovementComponent->TransmissionSetup.bUseAutomaticGears && CurrentTargetGear > 0)
			{
				// 현재 engine RPM입니다.
				const float CurrentEngineRpm = VehicleMovementComponent->GetEngineRotationSpeed();
				if (CurrentTargetGear < MaximumForwardGear
					&& VehicleMovementComponent->TransmissionSetup.ChangeUpRPM > 0.0f
					&& CurrentEngineRpm >= VehicleMovementComponent->TransmissionSetup.ChangeUpRPM)
				{
					VehicleMovementComponent->SetTargetGear(CurrentTargetGear + 1, false);
				}
				else if (CurrentTargetGear > 1
					&& VehicleMovementComponent->TransmissionSetup.ChangeDownRPM > 0.0f
					&& CurrentEngineRpm <= VehicleMovementComponent->TransmissionSetup.ChangeDownRPM)
				{
					VehicleMovementComponent->SetTargetGear(CurrentTargetGear - 1, false);
				}
			}

			VehicleDriveComponent->ApplyThrottleInput(ThrottleValue);
		}

		// acceleration/top-speed active drive phase의 RPM/current gear/peak speed telemetry만 갱신합니다.
		void UpdatePeakRuntimeTelemetry()
		{
			if (!VehicleMovementComponent)
			{
				return;
			}
			Result.PeakEngineRpm = FMath::Max(Result.PeakEngineRpm, VehicleMovementComponent->GetEngineRotationSpeed());
			// current forward speed입니다.
			const float ForwardSpeedKmh = GetForwardSpeedKmh();
			if (ForwardSpeedKmh > Result.PeakSpeedKmh)
			{
				Result.PeakSpeedKmh = ForwardSpeedKmh;
				Result.PeakSpeedGear = VehicleMovementComponent->GetCurrentGear();
			}
		}

		// physics settle 뒤 0→50/100 acceleration phase를 시작합니다.
		void UpdateSettleBeforeAcceleration()
		{
			// mass/physics settle 시간입니다.
			constexpr float SettleSeconds = 1.0f;
			if (GetPhaseElapsedSeconds() < SettleSeconds)
			{
				return;
			}
			if (!ResetVehicleKinematics())
			{
				return;
			}
			BeginPhase();
			ApplyBenchmarkForwardThrottle(1.0f);
			Phase = ECFBuilderDrivingBenchmarkPhase::MeasureAcceleration;
		}

		// 0→50/100 도달 시간을 기록하고 100 도달 또는 bounded observation 종료 뒤 top-speed phase로 전진합니다.
		void UpdateMeasureAcceleration()
		{
			// 0→50/100 관측을 무한 실행하지 않을 공통 bounded observation 시간입니다. 성능 PASS 임계값이 아닙니다.
			constexpr float AccelerationObservationSeconds = 30.0f;
			ApplyBenchmarkForwardThrottle(1.0f);
			// current forward speed입니다.
			const float SpeedKmh = GetForwardSpeedKmh();
			if (Result.Acceleration0To50Seconds < 0.0f && SpeedKmh >= 50.0f)
			{
				Result.Acceleration0To50Seconds = GetPhaseElapsedSeconds();
			}
			if (Result.Acceleration0To100Seconds < 0.0f && SpeedKmh >= 100.0f)
			{
				Result.Acceleration0To100Seconds = GetPhaseElapsedSeconds();
				BeginPhase();
				TopSpeedStabilityReferenceKmh = -1.0f;
				Phase = ECFBuilderDrivingBenchmarkPhase::MeasureTopSpeed;
				return;
			}
			if (GetPhaseElapsedSeconds() >= AccelerationObservationSeconds)
			{
				BeginPhase();
				TopSpeedStabilityReferenceKmh = -1.0f;
				Phase = ECFBuilderDrivingBenchmarkPhase::MeasureTopSpeed;
			}
		}

		// Full throttle 추가 20초에서 peak speed와 후반 5초 안정 여부를 측정합니다.
		void UpdateMeasureTopSpeed()
		{
			// top-speed observation duration입니다.
			constexpr float TopSpeedMeasureSeconds = 20.0f;
			// stability reference를 잡을 후반 시작 시각입니다.
			constexpr float StabilityReferenceSeconds = 15.0f;
			ApplyBenchmarkForwardThrottle(1.0f);
			// current speed입니다.
			const float SpeedKmh = GetForwardSpeedKmh();
			if (TopSpeedStabilityReferenceKmh < 0.0f && GetPhaseElapsedSeconds() >= StabilityReferenceSeconds)
			{
				TopSpeedStabilityReferenceKmh = SpeedKmh;
			}
			if (GetPhaseElapsedSeconds() < TopSpeedMeasureSeconds)
			{
				return;
			}
			if (TopSpeedStabilityReferenceKmh >= 0.0f)
			{
				Result.bTopSpeedStable = FMath::Abs(SpeedKmh - TopSpeedStabilityReferenceKmh) <= 2.0f;
			}
			VehicleDriveComponent->ClearDriveInputs();
			Phase = ECFBuilderDrivingBenchmarkPhase::PrepareBraking;
		}

		// initial trajectory가 100km/h에 도달한 차량만 같은 주행 상태에서 100km/h braking entry를 준비합니다.
		void UpdatePrepareBraking()
		{
			VehicleDriveComponent->ClearDriveInputs();
			if (Result.Acceleration0To100Seconds < 0.0f)
			{
				Result.bBraking100Available = false;
				Phase = ECFBuilderDrivingBenchmarkPhase::PrepareYaw;
				return;
			}
			BeginPhase();
			Phase = ECFBuilderDrivingBenchmarkPhase::ReachBrakingSpeed;
		}

		// 같은 top-speed trajectory에서 100km/h를 재확보하거나 coast로 접근한 뒤 Brake 1.0을 적용하고 100→Idle 측정을 시작합니다.
		void UpdateReachBrakingSpeed()
		{
			// 100km/h braking entry를 무한 대기하지 않을 기술 제한시간입니다.
			constexpr float ReachBrakingEntryTimeoutSeconds = 30.0f;
			// 100km/h braking entry를 frame 단위로 판정할 허용 폭입니다.
			constexpr float BrakingEntryToleranceKmh = 0.75f;
			// current planar speed입니다.
			const float SpeedKmh = GetPlanarSpeedKmh();

			if (FMath::Abs(SpeedKmh - 100.0f) <= BrakingEntryToleranceKmh)
			{
				Result.bBraking100Available = true;
				Result.BrakingStartSpeedKmh = SpeedKmh;
				VehicleDriveComponent->ApplyThrottleInput(0.0f);
				VehicleDriveComponent->ApplyBrakeInput(1.0f);
				BeginPhase();
				Phase = ECFBuilderDrivingBenchmarkPhase::MeasureBraking;
				return;
			}

			if (SpeedKmh < 100.0f)
			{
				VehicleDriveComponent->ApplyBrakeInput(0.0f);
				ApplyBenchmarkForwardThrottle(1.0f);
			}
			else
			{
				VehicleDriveComponent->ApplyThrottleInput(0.0f);
				VehicleDriveComponent->ApplyBrakeInput(0.0f);
			}

			if (GetPhaseElapsedSeconds() >= ReachBrakingEntryTimeoutSeconds)
			{
				FailBenchmark(TEXT("VB-P0-08: 같은 trajectory에서 30초 안에 100km/h braking entry를 확보하지 못했습니다."));
			}
		}

		// Brake 1.0부터 DriveState Idle threshold까지 time/distance를 측정합니다.
		void UpdateMeasureBraking()
		{
			// braking technical timeout입니다.
			constexpr float BrakingTimeoutSeconds = 20.0f;
			VehicleDriveComponent->ApplyThrottleInput(0.0f);
			VehicleDriveComponent->ApplyBrakeInput(1.0f);
			if (GetPlanarSpeedKmh() <= StopSpeedThresholdKmh)
			{
				Result.Braking100ToIdleSeconds = GetPhaseElapsedSeconds();
				Result.Braking100ToIdleDistanceMeters = GetPhaseDistanceMeters();
				VehicleDriveComponent->ClearDriveInputs();
				Phase = ECFBuilderDrivingBenchmarkPhase::PrepareYaw;
				return;
			}
			if (GetPhaseElapsedSeconds() >= BrakingTimeoutSeconds)
			{
				FailBenchmark(TEXT("VB-P0-08: 100km/h braking이 20초 안에 Idle threshold까지 끝나지 않았습니다."));
			}
		}

		// steady yaw benchmark를 fresh PhysicsState reset에서 준비하고 이전 phase 잔류가 빠질 settle 단계로 전진합니다.
		void UpdatePrepareYaw()
		{
			if (!RecreateBenchmarkPhysicsState())
			{
				return;
			}
			BeginPhase();
			Phase = ECFBuilderDrivingBenchmarkPhase::SettleBeforeYaw;
		}

		// yaw 준비 전에 0.5초 무입력 settle 뒤 다시 reset해 동일 시작 상태를 만듭니다.
		void UpdateSettleBeforeYaw()
		{
			// 기존 FIT mobility에서 검증한 phase 간 공통 settle 시간입니다.
			constexpr float SettleSeconds = 0.5f;
			VehicleDriveComponent->ClearDriveInputs();
			if (GetPhaseElapsedSeconds() < SettleSeconds)
			{
				return;
			}
			if (!ResetVehicleKinematics())
			{
				return;
			}
			BeginPhase();
			ApplyBenchmarkForwardThrottle(1.0f);
			Phase = ECFBuilderDrivingBenchmarkPhase::ReachYawSpeed;
		}

		// steady-yaw 시작 기준 30km/h를 준비합니다.
		void UpdateReachYawSpeed()
		{
			// 30km/h reach timeout입니다.
			constexpr float Reach30TimeoutSeconds = 20.0f;
			ApplyBenchmarkForwardThrottle(1.0f);
			if (GetForwardSpeedKmh() >= 30.0f)
			{
				VehicleDriveComponent->ApplyThrottleInput(0.0f);
				VehicleDriveComponent->ApplySteeringInput(0.5f);
				LastYawDegrees = TargetVehiclePawn->GetActorRotation().Yaw;
				AccumulatedYawDegrees = 0.0f;
				BeginPhase();
				Phase = ECFBuilderDrivingBenchmarkPhase::MeasureYaw;
				return;
			}
			if (GetPhaseElapsedSeconds() >= Reach30TimeoutSeconds)
			{
				FailBenchmark(TEXT("VB-P0-08: steady-yaw 준비 중 30km/h에 도달하지 못했습니다."));
			}
		}

		// Steering 0.5를 2초 적용한 누적 yaw response를 측정합니다.
		void UpdateMeasureYaw()
		{
			// yaw measurement duration입니다.
			constexpr float YawMeasureSeconds = 2.0f;
			VehicleDriveComponent->ApplyThrottleInput(0.0f);
			VehicleDriveComponent->ApplyBrakeInput(0.0f);
			VehicleDriveComponent->ApplySteeringInput(0.5f);
			// current actor yaw입니다.
			const float CurrentYawDegrees = TargetVehiclePawn->GetActorRotation().Yaw;
			AccumulatedYawDegrees += FMath::FindDeltaAngleDegrees(LastYawDegrees, CurrentYawDegrees);
			LastYawDegrees = CurrentYawDegrees;
			if (GetPhaseElapsedSeconds() < YawMeasureSeconds)
			{
				return;
			}
			Result.SteadyYawResponseDegrees = FMath::Abs(AccumulatedYawDegrees);
			VehicleDriveComponent->ClearDriveInputs();
			Phase = ECFBuilderDrivingBenchmarkPhase::PrepareTurning;
		}

		// effective turning radius benchmark를 fresh PhysicsState reset에서 준비하고 이전 phase 잔류가 빠질 settle 단계로 전진합니다.
		void UpdatePrepareTurning()
		{
			if (!RecreateBenchmarkPhysicsState())
			{
				return;
			}
			BeginPhase();
			Phase = ECFBuilderDrivingBenchmarkPhase::SettleBeforeTurning;
		}

		// turning 준비 전에 0.5초 무입력 settle 뒤 다시 reset해 동일 시작 상태를 만듭니다.
		void UpdateSettleBeforeTurning()
		{
			// 기존 FIT mobility에서 검증한 phase 간 공통 settle 시간입니다.
			constexpr float SettleSeconds = 0.5f;
			VehicleDriveComponent->ClearDriveInputs();
			if (GetPhaseElapsedSeconds() < SettleSeconds)
			{
				return;
			}
			if (!ResetVehicleKinematics())
			{
				return;
			}
			BeginPhase();
			ApplyBenchmarkForwardThrottle(1.0f);
			Phase = ECFBuilderDrivingBenchmarkPhase::ReachTurningSpeed;
		}

		// full-steer turning 시작 기준 15km/h를 준비합니다.
		void UpdateReachTurningSpeed()
		{
			// 15km/h reach timeout입니다.
			constexpr float Reach15TimeoutSeconds = 15.0f;
			ApplyBenchmarkForwardThrottle(1.0f);
			if (GetForwardSpeedKmh() >= 15.0f)
			{
				LastTurningLocation = TargetVehiclePawn->GetActorLocation();
				LastYawDegrees = TargetVehiclePawn->GetActorRotation().Yaw;
				AccumulatedTurningPathMeters = 0.0f;
				AccumulatedTurningYawDegrees = 0.0f;
				BeginPhase();
				Phase = ECFBuilderDrivingBenchmarkPhase::MeasureTurning;
				return;
			}
			if (GetPhaseElapsedSeconds() >= Reach15TimeoutSeconds)
			{
				FailBenchmark(TEXT("VB-P0-08: turning radius 준비 중 15km/h에 도달하지 못했습니다."));
			}
		}

		// Steering 1.0 + low throttle 4초의 path/yaw로 effective radius를 계산합니다.
		void UpdateMeasureTurning()
		{
			// turning measurement duration입니다.
			constexpr float TurningMeasureSeconds = 4.0f;
			// turning 중 속도 유지용 bounded throttle입니다.
			constexpr float TurningThrottle = 0.20f;
			ApplyBenchmarkForwardThrottle(TurningThrottle);
			VehicleDriveComponent->ApplyBrakeInput(0.0f);
			VehicleDriveComponent->ApplySteeringInput(1.0f);

			// current location입니다.
			const FVector CurrentLocation = TargetVehiclePawn->GetActorLocation();
			// current yaw입니다.
			const float CurrentYawDegrees = TargetVehiclePawn->GetActorRotation().Yaw;
			AccumulatedTurningPathMeters += FVector::Dist2D(LastTurningLocation, CurrentLocation) * 0.01f;
			AccumulatedTurningYawDegrees += FMath::Abs(FMath::FindDeltaAngleDegrees(LastYawDegrees, CurrentYawDegrees));
			LastTurningLocation = CurrentLocation;
			LastYawDegrees = CurrentYawDegrees;

			if (GetPhaseElapsedSeconds() < TurningMeasureSeconds)
			{
				return;
			}

			// accumulated yaw radians입니다.
			const float TurningYawRadians = FMath::DegreesToRadians(AccumulatedTurningYawDegrees);
			if (TurningYawRadians > KINDA_SMALL_NUMBER && AccumulatedTurningPathMeters > KINDA_SMALL_NUMBER)
			{
				Result.EffectiveTurningRadiusMeters = AccumulatedTurningPathMeters / TurningYawRadians;
				Result.TurningAverageSpeedKmh = (AccumulatedTurningPathMeters / TurningMeasureSeconds) * 3.6f;
			}
			VehicleDriveComponent->ClearDriveInputs();
			Phase = ECFBuilderDrivingBenchmarkPhase::Finish;
		}

		// metric 유한성/invariant를 확인하고 machine-readable 한 줄을 기록합니다.
		void UpdateFinish()
		{
			// 필수 metric이 finite인지 확인합니다.
			const bool bFinite =
				FMath::IsFinite(Result.ConfiguredMassKg)
				&& FMath::IsFinite(Result.ActualMassKg)
				&& FMath::IsFinite(Result.Acceleration0To50Seconds)
				&& FMath::IsFinite(Result.PeakSpeedKmh)
				&& FMath::IsFinite(Result.SteadyYawResponseDegrees)
				&& FMath::IsFinite(Result.PeakEngineRpm);
			if (!bFinite || Result.PeakSpeedKmh <= KINDA_SMALL_NUMBER)
			{
				FailBenchmark(TEXT("VB-P0-08: runtime metric이 유효하지 않거나 full-throttle 관측에서 차량이 실제로 이동하지 않았습니다."));
				return;
			}

			if (Test)
			{
				Test->AddInfo(FString::Printf(
					TEXT("VB-P0-08 DrivingMetric | Label=%s | VehicleData=%s | FittingData=%s | ConfiguredMassKg=%.3f | ActualMassKg=%.3f | Accel0To50Sec=%.6f | Accel0To100Sec=%.6f | PeakSpeedKmh=%.6f | TopSpeedStable=%s | PeakRPM=%.6f | PeakGear=%d | Braking100Available=%s | BrakeStartKmh=%.6f | Brake100ToIdleSec=%.6f | Brake100ToIdleDistanceM=%.6f | SteadyYawDeg=%.6f | TurningRadiusM=%.6f | TurningAverageSpeedKmh=%.6f"),
					*Result.Label,
					*Result.VehicleDataPath,
					Result.FittingDataPath.IsEmpty() ? TEXT("BaseMass") : *Result.FittingDataPath,
					Result.ConfiguredMassKg,
					Result.ActualMassKg,
					Result.Acceleration0To50Seconds,
					Result.Acceleration0To100Seconds,
					Result.PeakSpeedKmh,
					Result.bTopSpeedStable ? TEXT("True") : TEXT("False"),
					Result.PeakEngineRpm,
					Result.PeakSpeedGear,
					Result.bBraking100Available ? TEXT("True") : TEXT("False"),
					Result.BrakingStartSpeedKmh,
					Result.Braking100ToIdleSeconds,
					Result.Braking100ToIdleDistanceMeters,
					Result.SteadyYawResponseDegrees,
					Result.EffectiveTurningRadiusMeters,
					Result.TurningAverageSpeedKmh));
			}
			RestoreMeasurementControl();
			Phase = ECFBuilderDrivingBenchmarkPhase::Done;
		}

		// benchmark failure를 기록하고 transient drive/possession을 복원합니다.
		void FailBenchmark(const FString& FailureMessage)
		{
			if (Test)
			{
				Test->AddError(FailureMessage);
			}
			RestoreMeasurementControl();
			Phase = ECFBuilderDrivingBenchmarkPhase::Done;
		}

		// benchmark 중 임시 입력/possession/tick 상태를 원래 PIE 상태로 복원합니다.
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

		// 결과/오류를 기록할 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
		// saved target VehicleData object path입니다.
		FString VehicleDataPath;
		// optional saved FittingData object path입니다.
		FString FittingDataPath;
		// 사람이 식별할 benchmark label입니다.
		FString Label;
		// benchmark 준비 wall-clock 시작 시각입니다.
		double CommandStartTimeSeconds = 0.0;
		// current PIE world입니다.
		UWorld* PIEWorld = nullptr;
		// deferred-spawn한 benchmark vehicle입니다.
		ACFVehiclePawn* TargetVehiclePawn = nullptr;
		// benchmark 동안 target vehicle을 possess할 PlayerController입니다.
		APlayerController* MeasurementPlayerController = nullptr;
		// benchmark 종료 뒤 복원할 원래 Player Pawn입니다.
		APawn* OriginalPlayerPawn = nullptr;
		// production drive input path입니다.
		UCFVehicleDriveComp* VehicleDriveComponent = nullptr;
		// actual Chaos movement runtime입니다.
		UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = nullptr;
		// actual chassis physics velocity/mass source입니다.
		USkeletalMeshComponent* VehicleMeshComponent = nullptr;
		// optional FittingData가 지정된 경우의 exact benchmark fitting snapshot입니다.
		FCFVehicleFittingSnapshot FittingSnapshot;
		// Fitting total mass 또는 no-fitting VehicleData base mass에서 결정한 explicit runtime mass target입니다.
		float BenchmarkTargetMassKg = 0.0f;
		// runtime mass reapply production adapter입니다.
		TUniquePtr<FCFChaosVehicleMassRuntime> MassRuntime;
		// current benchmark result입니다.
		FCFBuilderDrivingBenchmarkResult Result;
		// current benchmark phase입니다.
		ECFBuilderDrivingBenchmarkPhase Phase = ECFBuilderDrivingBenchmarkPhase::WaitForPIE;
		// current phase start world time입니다.
		float PhaseStartWorldTimeSeconds = 0.0f;
		// current phase start location입니다.
		FVector PhaseStartLocation = FVector::ZeroVector;
		// braking stop threshold입니다.
		float StopSpeedThresholdKmh = 0.0f;
		// common reset transform입니다.
		FTransform OriginalActorTransform = FTransform::Identity;
		// top-speed stability 후반 기준 속도입니다.
		float TopSpeedStabilityReferenceKmh = -1.0f;
		// yaw/turning 직전 frame yaw입니다.
		float LastYawDegrees = 0.0f;
		// steady-yaw accumulated signed degrees입니다.
		float AccumulatedYawDegrees = 0.0f;
		// turning path 직전 location입니다.
		FVector LastTurningLocation = FVector::ZeroVector;
		// turning path accumulated meters입니다.
		float AccumulatedTurningPathMeters = 0.0f;
		// turning accumulated absolute yaw degrees입니다.
		float AccumulatedTurningYawDegrees = 0.0f;
		// target pawn 원래 actor tick 상태입니다.
		bool bOriginalTargetActorTickEnabled = true;
		// temporary possession을 적용했는지 여부입니다.
		bool bMeasurementPossessionApplied = false;
		// restoration 중복 방지 flag입니다.
		bool bMeasurementControlRestored = false;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderDrivingBenchmarkTest,
	"CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TechnicalDrivingBenchmark",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// command-line target VehicleData를 fresh PIE 실제 Chaos vehicle로 한 번 계측합니다.
bool FCFVehicleBuilderDrivingBenchmarkTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// runner가 전달한 exact VehicleData object path입니다.
	FString VehicleDataPath;
	if (!FParse::Value(FCommandLine::Get(), TEXT("CFBuilderBenchmarkVehicleData="), VehicleDataPath) || VehicleDataPath.IsEmpty())
	{
		AddError(TEXT("VB-P0-08: -CFBuilderBenchmarkVehicleData=<ObjectPath>가 필요합니다."));
		return false;
	}

	// optional exact FittingData object path입니다.
	FString FittingDataPath;
	FParse::Value(FCommandLine::Get(), TEXT("CFBuilderBenchmarkFittingData="), FittingDataPath);

	// optional human-readable label입니다.
	FString Label;
	if (!FParse::Value(FCommandLine::Get(), TEXT("CFBuilderBenchmarkLabel="), Label) || Label.IsEmpty())
	{
		Label = TEXT("BuilderVehicle");
	}

	// 기존 production test map을 읽기 전용 technical road/physics fixture로 재사용합니다.
	const FString BenchmarkMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(BenchmarkMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFBuilderDrivingBenchmarkCommand(this, VehicleDataPath, FittingDataPath, Label));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
