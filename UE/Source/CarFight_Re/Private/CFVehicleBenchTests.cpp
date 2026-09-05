// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleBenchTests.cpp
// Version: v1.18.1
// Date: 2026-09-04
// Description: CF-FQ-040 VB-P0-08 saved VehicleData Technical Driving Benchmark와 ESH-04 dedicated high-speed benchmark authority를 제공합니다.
// Scope: M_VehicleBenchmark production no-fitting Legacy Mass straight-line authority와 선택적 transient ChangeUpRPM A/B/C를 제공하며 Product Asset/Map 저장 mutation은 수행하지 않습니다.
// Changelog:
// - v1.18.1: P0-07E 중간검수 교정. command-line progress write target을 ProjectSaved/CarFight canonical sidecar exact path로 제한하고 temp write 실패 시 잔여 임시 파일을 best-effort 정리합니다.
// - v1.18.0: VBHAI-P0-07E에서 optional exact RunId + absolute progress sidecar path를 받아 VB-P0-08 내부 16 phase를 USER-facing 7단계 coarse progress로 투영합니다. 각 coarse 단계는 run당 최대 1회만 temp→replace write를 시도하고 write 실패는 warning만 남겨 benchmark terminal PASS/FAIL authority와 분리합니다.
// - v1.17.0: fixed ChangeUpRPM A/B 판정용 0→100/150/200 km/h first-reach time telemetry를 추가. 최고속/사용 gear만으로 shift 후보를 오판하지 않고 WOT acceleration evidence를 함께 비교합니다.
// - v1.16.0: ESH-03 후보를 Product mutation 없이 검증할 optional CFHighSpeedChangeUpRPMOverride를 추가. saved VehicleData를 transient duplicate한 뒤 duplicate의 ChangeUpRPM만 변경하며 effective ChangeUp/Down/AutomaticGears를 summary에 기록.
// - v1.15.0: ESH-04가 VehicleFittingData=None production 경로와 달리 BaseVehicleMassKg=1886을 BeginPlay 뒤 강제 Reapply하던 fixture divergence를 제거. fresh BeginPlay가 확정한 Legacy configured/actual mass를 그대로 보존하고 summary에 기록.
// - v1.14.0: zero-steering 150s run에서 누적 yaw 76.2deg가 longitudinal speed를 오염한 것을 확인해 handling과 분리된 straight-line fixture용 world-Z yaw-rate lock을 추가. steering은 계속 0이며 매 frame clamp 전 최대 yaw rate를 evidence에 기록.
// - v1.13.0: steering intervention 자체를 제거. PlayerStart의 longitudinal 위치/rotation은 보존하고 start를 persisted Floor의 lateral centerline으로 투영해 약 ±3.9km 횡여유를 확보한 뒤 Steering=0으로 순수 종방향 성능을 계측. centerline shift를 evidence에 기록.
// - v1.12.1: 첫 heading-hold 관측의 max steering 0.0615가 high-speed 성능을 오염할 가능성을 제거하기 위해 0.25deg deadzone + gain 0.005/deg + clamp 0.02의 low-intervention pulse controller로 축소.
// - v1.12.0: zero-steering long run의 누적 yaw drift로 Floor side bound를 소모하는 문제를 교정. PlayerStart initial heading만 유지하는 P-only controller(gain 0.015/deg, clamp 0.12)를 추가하고 최대 steering intervention을 evidence에 기록.
// - v1.11.1: 임의 50m lateral corridor를 hard blocker에서 제거하고 persisted Floor bounds 이탈만 fail-closed하도록 교정. 최대 lateral offset과 최대 heading deviation을 authority evidence로 기록.
// - v1.11.0: ESH-04 HighSpeedBenchmark Automation을 추가. exact PlayerStart forward axis + Floor AABB exit을 start/end authority로 사용하고 collision obstacle preflight, 150s bounded observation, 10s×2 stability window, gear별 entry/dwell/speed gain을 기록.
// - v1.10.1: ESH-04 map actor inventory에 Rotation을 추가해 PlayerStart forward-axis authority를 직접 판정할 수 있게 함.
// - v1.10.0: ESH-04 첫 Gate로 M_VehicleBenchmark actor label/class/location/scale/tag, StaticMesh world bounds, WorldSettings KillZ를 mutation0로 출력하는 BenchmarkMapInventory Automation을 additive 추가.
// - v1.9.1: 고속 진단 중 발견한 destroyed UObject raw-pointer access를 IsValid fail-closed로 보강하고, BP_CFVehiclePawn inherited Chaos TorqueCurve를 Product mutation 없이 읽는 TorqueCurveInspect Automation을 additive 추가. 기존 M_VehicleDefensePIE benchmark fixture/metric 계약은 보존.
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
// - v1.18.1부터 progress path 인자가 canonical ProjectSaved/CarFight sidecar와 exact 일치하지 않으면 progress 표시만 비활성화하고 benchmark 본체는 계속합니다.
// - v1.18.0 progress writer는 `CFBuilderBenchmarkRunId`와 `CFBuilderBenchmarkProgressPath`가 둘 다 유효할 때만 활성화됩니다. 기존 direct Automation 호출은 두 인자를 생략하면 progress write 없이 이전 동작을 유지합니다.
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
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
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

	/** VB-P0-08 내부 phase를 USER-facing 7단계 progress로 묶는 Runtime-private group입니다. */
	enum class ECFBuilderBenchmarkProgressGroup : uint8
	{
		None,
		Preparing,
		Acceleration,
		TopSpeed,
		Braking,
		Steering,
		TurningRadius,
		Finalizing
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
			const FString& InLabel,
			const FString& InRunId,
			const FString& InProgressFilePath)
			: Test(InTest)
			, VehicleDataPath(InVehicleDataPath)
			, FittingDataPath(InFittingDataPath)
			, Label(InLabel)
			, RunId(InRunId)
			, ProgressFilePath(InProgressFilePath)
			, CommandStartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// fresh PIE를 한 프레임씩 진행하며 benchmark state machine을 실행합니다.
		virtual bool Update() override
		{
			AttemptCurrentProgressWrite();

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

			if (!PIEWorld
				|| !IsValid(TargetVehiclePawn)
				|| !IsValid(VehicleDriveComponent)
				|| !IsValid(VehicleMovementComponent)
				|| !IsValid(VehicleMeshComponent))
			{
				FailBenchmark(TEXT("VB-P0-08: benchmark 도중 PIE Vehicle Runtime 참조가 파괴되었거나 유실되었습니다."));
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
		// Current internal benchmark phase를 USER-facing coarse progress group으로 변환합니다.
		ECFBuilderBenchmarkProgressGroup ResolveCurrentProgressGroup() const
		{
			switch (Phase)
			{
			case ECFBuilderDrivingBenchmarkPhase::WaitForPIE:
			case ECFBuilderDrivingBenchmarkPhase::SettleBeforeAcceleration:
				return ECFBuilderBenchmarkProgressGroup::Preparing;
			case ECFBuilderDrivingBenchmarkPhase::MeasureAcceleration:
				return ECFBuilderBenchmarkProgressGroup::Acceleration;
			case ECFBuilderDrivingBenchmarkPhase::MeasureTopSpeed:
				return ECFBuilderBenchmarkProgressGroup::TopSpeed;
			case ECFBuilderDrivingBenchmarkPhase::PrepareBraking:
			case ECFBuilderDrivingBenchmarkPhase::ReachBrakingSpeed:
			case ECFBuilderDrivingBenchmarkPhase::MeasureBraking:
				return ECFBuilderBenchmarkProgressGroup::Braking;
			case ECFBuilderDrivingBenchmarkPhase::PrepareYaw:
			case ECFBuilderDrivingBenchmarkPhase::SettleBeforeYaw:
			case ECFBuilderDrivingBenchmarkPhase::ReachYawSpeed:
			case ECFBuilderDrivingBenchmarkPhase::MeasureYaw:
				return ECFBuilderBenchmarkProgressGroup::Steering;
			case ECFBuilderDrivingBenchmarkPhase::PrepareTurning:
			case ECFBuilderDrivingBenchmarkPhase::SettleBeforeTurning:
			case ECFBuilderDrivingBenchmarkPhase::ReachTurningSpeed:
			case ECFBuilderDrivingBenchmarkPhase::MeasureTurning:
				return ECFBuilderBenchmarkProgressGroup::TurningRadius;
			case ECFBuilderDrivingBenchmarkPhase::Finish:
				return ECFBuilderBenchmarkProgressGroup::Finalizing;
			case ECFBuilderDrivingBenchmarkPhase::Done:
			default:
				return ECFBuilderBenchmarkProgressGroup::None;
			}
		}

		// Coarse progress group의 stable JSON key와 1-based index를 반환합니다.
		bool ResolveProgressDescriptor(
			const ECFBuilderBenchmarkProgressGroup ProgressGroup,
			const TCHAR*& OutPhaseKey,
			int32& OutPhaseIndex) const
		{
			OutPhaseKey = TEXT("");
			OutPhaseIndex = 0;
			switch (ProgressGroup)
			{
			case ECFBuilderBenchmarkProgressGroup::Preparing:
				OutPhaseKey = TEXT("preparing");
				OutPhaseIndex = 1;
				return true;
			case ECFBuilderBenchmarkProgressGroup::Acceleration:
				OutPhaseKey = TEXT("acceleration");
				OutPhaseIndex = 2;
				return true;
			case ECFBuilderBenchmarkProgressGroup::TopSpeed:
				OutPhaseKey = TEXT("top_speed");
				OutPhaseIndex = 3;
				return true;
			case ECFBuilderBenchmarkProgressGroup::Braking:
				OutPhaseKey = TEXT("braking");
				OutPhaseIndex = 4;
				return true;
			case ECFBuilderBenchmarkProgressGroup::Steering:
				OutPhaseKey = TEXT("steering");
				OutPhaseIndex = 5;
				return true;
			case ECFBuilderBenchmarkProgressGroup::TurningRadius:
				OutPhaseKey = TEXT("turning_radius");
				OutPhaseIndex = 6;
				return true;
			case ECFBuilderBenchmarkProgressGroup::Finalizing:
				OutPhaseKey = TEXT("finalizing");
				OutPhaseIndex = 7;
				return true;
			case ECFBuilderBenchmarkProgressGroup::None:
			default:
				return false;
			}
		}

		// 한 coarse progress snapshot을 same-directory temp file 뒤 destination replace로 best-effort 기록합니다.
		bool WriteProgressSnapshot(
			const ECFBuilderBenchmarkProgressGroup ProgressGroup,
			FString& OutError) const
		{
			OutError.Reset();

			// JSON에 기록할 stable phase key입니다.
			const TCHAR* PhaseKey = TEXT("");
			// JSON에 기록할 1-based phase index입니다.
			int32 PhaseIndex = 0;
			if (!ResolveProgressDescriptor(ProgressGroup, PhaseKey, PhaseIndex))
			{
				OutError = TEXT("VB-P0-08 progress group을 JSON descriptor로 변환할 수 없습니다.");
				return false;
			}

			// Progress sidecar root JSON object입니다.
			const TSharedRef<FJsonObject> ProgressObject = MakeShared<FJsonObject>();
			ProgressObject->SetStringField(TEXT("schema_version"), TEXT("carfight_vehicle_builder_benchmark_progress_v1"));
			ProgressObject->SetStringField(TEXT("run_id"), RunId);
			ProgressObject->SetStringField(TEXT("phase_key"), PhaseKey);
			ProgressObject->SetNumberField(TEXT("phase_index"), PhaseIndex);
			ProgressObject->SetNumberField(TEXT("phase_count"), 7);

			// UTF-8로 저장할 serialized progress JSON입니다.
			FString ProgressJson;
			// Compact/default JSON writer입니다.
			const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ProgressJson);
			if (!FJsonSerializer::Serialize(ProgressObject, JsonWriter))
			{
				OutError = TEXT("VB-P0-08 progress JSON serialize에 실패했습니다.");
				return false;
			}

			// Destination과 같은 directory에 두는 unique temporary file입니다.
			const FString TemporaryProgressPath = FString::Printf(
				TEXT("%s.%s.tmp"),
				*ProgressFilePath,
				*FGuid::NewGuid().ToString(EGuidFormats::Digits));
			if (!FFileHelper::SaveStringToFile(
				ProgressJson,
				*TemporaryProgressPath,
				FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				// Partial temp가 남았을 가능성까지 best-effort 정리합니다. 실패해도 benchmark terminal authority에는 영향이 없습니다.
				IFileManager::Get().Delete(*TemporaryProgressPath, false, true, true);
				OutError = FString::Printf(TEXT("VB-P0-08 progress temp write에 실패했습니다: %s"), *TemporaryProgressPath);
				return false;
			}

			// Same-directory temp를 canonical destination으로 replace하는 terminal file operation 결과입니다.
			const bool bMoved = IFileManager::Get().Move(
				*ProgressFilePath,
				*TemporaryProgressPath,
				true,
				true,
				false,
				true);
			if (!bMoved)
			{
				IFileManager::Get().Delete(*TemporaryProgressPath, false, true, true);
				OutError = FString::Printf(TEXT("VB-P0-08 progress temp→destination replace에 실패했습니다: %s"), *ProgressFilePath);
				return false;
			}
			return true;
		}

		// Current coarse group 진입 시 run당 한 번만 progress write를 시도하고 실패를 benchmark terminal 결과와 분리합니다.
		void AttemptCurrentProgressWrite()
		{
			if (RunId.IsEmpty() || ProgressFilePath.IsEmpty())
			{
				return;
			}

			// Current internal phase가 속한 coarse USER progress group입니다.
			const ECFBuilderBenchmarkProgressGroup CurrentProgressGroup = ResolveCurrentProgressGroup();
			if (CurrentProgressGroup == ECFBuilderBenchmarkProgressGroup::None
				|| CurrentProgressGroup == LastAttemptedProgressGroup)
			{
				return;
			}

			// 실패하더라도 같은 group에서 per-frame retry하지 않도록 write 전에 attempted state를 먼저 갱신합니다.
			LastAttemptedProgressGroup = CurrentProgressGroup;
			++ProgressWriteAttemptCount;
			if (ProgressWriteAttemptCount > 7)
			{
				if (Test)
				{
					Test->AddWarning(TEXT("VB-P0-08 progress write attempt가 7회를 초과해 추가 sidecar write를 생략했습니다."));
				}
				return;
			}

			// Best-effort sidecar write diagnostic입니다.
			FString ProgressWriteError;
			if (!WriteProgressSnapshot(CurrentProgressGroup, ProgressWriteError) && Test)
			{
				Test->AddWarning(FString::Printf(
					TEXT("VB-P0-08 progress sidecar write warning: %s"),
					*ProgressWriteError));
			}
		}

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
			if (IsValid(VehicleDriveComponent))
			{
				VehicleDriveComponent->ClearDriveInputs();
			}
			if (IsValid(TargetVehiclePawn))
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
		// Guided/runner가 전달한 exact benchmark run identity입니다. 비어 있으면 progress writer를 사용하지 않습니다.
		FString RunId;
		// Guided Step 8이 polling할 canonical absolute progress sidecar path입니다. 비어 있으면 progress writer를 사용하지 않습니다.
		FString ProgressFilePath;
		// 동일 coarse group에서 write failure가 나도 per-frame retry하지 않기 위한 마지막 attempted group입니다.
		ECFBuilderBenchmarkProgressGroup LastAttemptedProgressGroup = ECFBuilderBenchmarkProgressGroup::None;
		// 한 benchmark run에서 실제 sidecar write를 시도한 coarse group 수입니다.
		int32 ProgressWriteAttemptCount = 0;
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

	/** ESH-04 persisted M_VehicleBenchmark의 actor/geometry authority를 read-only로 기록합니다. */
	class FCFVehicleBenchmarkMapInventoryCommand final : public IAutomationLatentCommand
	{
	public:
		// 결과를 기록할 Automation Test를 보존합니다.
		explicit FCFVehicleBenchmarkMapInventoryCommand(FAutomationTestBase* InTest)
			: Test(InTest)
		{
		}

		// 현재 Editor World의 benchmark actor inventory를 한 번 읽고 종료합니다.
		virtual bool Update() override
		{
			// 현재 Editor World입니다.
			UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
			if (!EditorWorld)
			{
				if (Test)
				{
					Test->AddError(TEXT("ESH-04: M_VehicleBenchmark Editor World를 찾을 수 없습니다."));
				}
				return true;
			}

			// 현재 map의 WorldSettings입니다.
			const AWorldSettings* WorldSettings = EditorWorld->GetWorldSettings();
			// 현재 map 전체 actor 개수입니다.
			int32 ActorCount = 0;
			// VehicleBenchmarkStart tag actor 개수입니다.
			int32 StartMarkerCount = 0;
			// VehicleBenchmarkEnd 또는 EndPreview tag actor 개수입니다.
			int32 EndMarkerCount = 0;
			// StaticMesh actor 개수입니다.
			int32 StaticMeshActorCount = 0;

			for (TActorIterator<AActor> ActorIterator(EditorWorld); ActorIterator; ++ActorIterator)
			{
				// 현재 inventory actor입니다.
				AActor* Actor = *ActorIterator;
				if (!IsValid(Actor))
				{
					continue;
				}
				++ActorCount;

				// 현재 actor의 label입니다.
				const FString ActorLabel = Actor->GetActorLabel();
				// 현재 actor의 class 이름입니다.
				const FString ActorClass = GetNameSafe(Actor->GetClass());
				// 현재 actor의 world location입니다.
				const FVector ActorLocation = Actor->GetActorLocation();
				// 현재 actor의 world rotation입니다.
				const FRotator ActorRotation = Actor->GetActorRotation();
				// 현재 actor의 world scale입니다.
				const FVector ActorScale = Actor->GetActorScale3D();
				// 현재 actor tag들의 bounded 문자열입니다.
				FString TagList;
				for (const FName ActorTag : Actor->Tags)
				{
					if (!TagList.IsEmpty())
					{
						TagList += TEXT(",");
					}
					TagList += ActorTag.ToString();
				}

				if (Actor->ActorHasTag(TEXT("VehicleBenchmarkStart")))
				{
					++StartMarkerCount;
				}
				if (Actor->ActorHasTag(TEXT("VehicleBenchmarkEnd")) || Actor->ActorHasTag(TEXT("VehicleBenchmarkEndPreview")))
				{
					++EndMarkerCount;
				}

				if (Test)
				{
					Test->AddInfo(FString::Printf(
						TEXT("ESH04_MAP_ACTOR | Label=%s | Class=%s | LocationCm=(%.3f,%.3f,%.3f) | RotationDeg=(P=%.3f,Y=%.3f,R=%.3f) | Scale=(%.6f,%.6f,%.6f) | Tags=%s"),
						*ActorLabel,
						*ActorClass,
						ActorLocation.X,
						ActorLocation.Y,
						ActorLocation.Z,
						ActorRotation.Pitch,
						ActorRotation.Yaw,
						ActorRotation.Roll,
						ActorScale.X,
						ActorScale.Y,
						ActorScale.Z,
						TagList.IsEmpty() ? TEXT("None") : *TagList));
				}

				// StaticMesh actor면 실제 world bounds도 함께 기록합니다.
				AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
				if (!StaticMeshActor || !StaticMeshActor->GetStaticMeshComponent())
				{
					continue;
				}
				++StaticMeshActorCount;

				// current StaticMesh component의 world-space axis-aligned bounds입니다.
				const FBoxSphereBounds WorldBounds = StaticMeshActor->GetStaticMeshComponent()->Bounds;
				// world bounds full size입니다.
				const FVector WorldBoundsSizeCm = WorldBounds.BoxExtent * 2.0;
				if (Test)
				{
					Test->AddInfo(FString::Printf(
						TEXT("ESH04_MAP_MESH | Label=%s | Mesh=%s | BoundsSizeCm=(%.3f,%.3f,%.3f) | BoundsOriginCm=(%.3f,%.3f,%.3f)"),
						*ActorLabel,
						*GetPathNameSafe(StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh()),
						WorldBoundsSizeCm.X,
						WorldBoundsSizeCm.Y,
						WorldBoundsSizeCm.Z,
						WorldBounds.Origin.X,
						WorldBounds.Origin.Y,
						WorldBounds.Origin.Z));
				}
			}

			if (Test)
			{
				Test->AddInfo(FString::Printf(
					TEXT("ESH04_MAP_SUMMARY | Map=%s | ActorCount=%d | StaticMeshActors=%d | StartMarkers=%d | EndMarkers=%d | KillZCm=%.3f"),
					*EditorWorld->GetPathName(),
					ActorCount,
					StaticMeshActorCount,
					StartMarkerCount,
					EndMarkerCount,
					WorldSettings ? WorldSettings->KillZ : 0.0f));
			}
			return true;
		}

	private:
		// 결과/오류를 기록할 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
	};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBenchmarkMapInventoryTest,
	"CarFight.VehicleBuilder.CF_FQ_040.ESH_04.BenchmarkMapInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Dedicated M_VehicleBenchmark persisted map의 actor/geometry authority inventory를 mutation0로 기록합니다.
bool FCFVehicleBenchmarkMapInventoryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// ESH-04 dedicated benchmark map exact path입니다.
	const FString BenchmarkMapPath = TEXT("/Game/Maps/M_VehicleBenchmark");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(BenchmarkMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FCFVehicleBenchmarkMapInventoryCommand(this));
	return true;
}

/** ESH-04 한 forward gear의 실제 high-speed 사용 구간 관측값입니다. */
struct FCFHighSpeedGearObservation
{
	// 실제 Chaos forward gear 번호입니다.
	int32 GearNumber = 0;
	// 이 gear에 최초 진입한 simulation time입니다.
	float EntryTimeSeconds = 0.0f;
	// 이 gear 최초 진입 속도입니다.
	float EntrySpeedKmh = 0.0f;
	// 이 gear 최초 진입 engine RPM입니다.
	float EntryEngineRpm = 0.0f;
	// 이 gear에서 마지막으로 관측된 simulation time입니다.
	float LastTimeSeconds = 0.0f;
	// 이 gear에서 관측한 최고 속도입니다.
	float PeakSpeedKmh = 0.0f;
	// 이 gear에서 관측한 최고 engine RPM입니다.
	float PeakEngineRpm = 0.0f;
};

/** ESH-04 dedicated M_VehicleBenchmark에서 한 saved VehicleData의 long full-throttle trajectory를 측정합니다. */
class FCFVehicleHighSpeedBenchmarkCommand final : public IAutomationLatentCommand
{
public:
	// exact VehicleData/label을 보존하고 fresh PIE 준비 시간을 시작합니다.
	FCFVehicleHighSpeedBenchmarkCommand(
		FAutomationTestBase* InTest,
		const FString& InVehicleDataPath,
		const FString& InLabel,
		const float InChangeUpRpmOverride)
		: Test(InTest)
		, VehicleDataPath(InVehicleDataPath)
		, Label(InLabel)
		, RequestedChangeUpRpmOverride(InChangeUpRpmOverride)
		, CommandStartTimeSeconds(FPlatformTime::Seconds())
	{
	}

	// fresh PIE를 진행하며 authority setup → settle → long full-throttle observation을 수행합니다.
	virtual bool Update() override
	{
		if (bDone)
		{
			return true;
		}

		if (!PIEWorld)
		{
			PIEWorld = FindPIEWorld();
		}

		if (!bSetupComplete)
		{
			UpdateSetup();
			return bDone;
		}

		if (!PIEWorld
			|| !IsValid(TargetVehiclePawn)
			|| !IsValid(VehicleDriveComponent)
			|| !IsValid(VehicleMovementComponent)
			|| !IsValid(VehicleMeshComponent))
		{
			FailBenchmark(TEXT("ESH-04: high-speed trajectory 중 required Vehicle runtime이 소실됐습니다."));
			return true;
		}

		if (!bDriveStarted)
		{
			UpdateSettle();
			return bDone;
		}

		UpdateDrive();
		return bDone;
	}

private:
	// current process의 fresh PIE World를 찾습니다.
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

	// M_VehicleBenchmark의 exact PlayerStart/Floor/obstacle geometry authority를 확정합니다.
	bool ResolveMapAuthority(FString& OutError)
	{
		if (!PIEWorld)
		{
			OutError = TEXT("ESH-04 PIE World가 없습니다.");
			return false;
		}

		// exact PlayerStart 후보 개수입니다.
		int32 PlayerStartCount = 0;
		// exact benchmark start PlayerStart입니다.
		APlayerStart* BenchmarkStart = nullptr;
		for (TActorIterator<APlayerStart> StartIterator(PIEWorld); StartIterator; ++StartIterator)
		{
			if (IsValid(*StartIterator))
			{
				++PlayerStartCount;
				BenchmarkStart = *StartIterator;
			}
		}
		if (PlayerStartCount != 1 || !BenchmarkStart)
		{
			OutError = FString::Printf(TEXT("ESH-04는 exact PlayerStart 1개가 필요합니다. current=%d"), PlayerStartCount);
			return false;
		}

		// exact Floor StaticMesh 후보 개수입니다.
		int32 FloorCount = 0;
		// 도로 authority를 소유할 exact Floor actor입니다.
		AStaticMeshActor* FloorActor = nullptr;
		// Floor 외 collision-enabled StaticMesh obstacle 개수입니다.
		int32 CollisionObstacleCount = 0;
		for (TActorIterator<AStaticMeshActor> MeshIterator(PIEWorld); MeshIterator; ++MeshIterator)
		{
			// current persisted StaticMesh actor입니다.
			AStaticMeshActor* MeshActor = *MeshIterator;
			if (!IsValid(MeshActor) || !MeshActor->GetStaticMeshComponent())
			{
				continue;
			}
			if (MeshActor->GetActorLabel() == TEXT("Floor"))
			{
				++FloorCount;
				FloorActor = MeshActor;
				continue;
			}
			if (MeshActor->GetStaticMeshComponent()->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			{
				++CollisionObstacleCount;
			}
		}
		if (FloorCount != 1 || !FloorActor || !FloorActor->GetStaticMeshComponent())
		{
			OutError = FString::Printf(TEXT("ESH-04는 exact Floor StaticMeshActor 1개가 필요합니다. current=%d"), FloorCount);
			return false;
		}
		if (FloorActor->GetStaticMeshComponent()->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			OutError = TEXT("ESH-04 Floor collision이 꺼져 있습니다.");
			return false;
		}
		if (CollisionObstacleCount != 0)
		{
			OutError = FString::Printf(TEXT("ESH-04 benchmark lane에 Floor 외 collision StaticMesh가 있습니다. count=%d"), CollisionObstacleCount);
			return false;
		}

		// persisted PlayerStart의 원래 world location입니다.
		const FVector PlayerStartLocation = BenchmarkStart->GetActorLocation();
		FloorBounds = FloorActor->GetStaticMeshComponent()->Bounds.GetBox();
		ForwardAxis = BenchmarkStart->GetActorForwardVector();
		ForwardAxis.Z = 0.0f;
		if (!ForwardAxis.Normalize())
		{
			OutError = TEXT("ESH-04 PlayerStart horizontal forward axis가 유효하지 않습니다.");
			return false;
		}
		RightAxis = FVector::CrossProduct(FVector::UpVector, ForwardAxis).GetSafeNormal();
		if (RightAxis.IsNearlyZero())
		{
			OutError = TEXT("ESH-04 PlayerStart right axis를 만들 수 없습니다.");
			return false;
		}

		if (PlayerStartLocation.X < FloorBounds.Min.X || PlayerStartLocation.X > FloorBounds.Max.X
			|| PlayerStartLocation.Y < FloorBounds.Min.Y || PlayerStartLocation.Y > FloorBounds.Max.Y)
		{
			OutError = TEXT("ESH-04 PlayerStart가 Floor horizontal bounds 밖에 있습니다.");
			return false;
		}

		// persisted PlayerStart 기준 Floor 네 corner의 right-axis projection 최소값입니다.
		float MinimumLateralProjectionCm = MAX_flt;
		// persisted PlayerStart 기준 Floor 네 corner의 right-axis projection 최대값입니다.
		float MaximumLateralProjectionCm = -MAX_flt;
		const FVector FloorCorners[4] = {
			FVector(FloorBounds.Min.X, FloorBounds.Min.Y, PlayerStartLocation.Z),
			FVector(FloorBounds.Min.X, FloorBounds.Max.Y, PlayerStartLocation.Z),
			FVector(FloorBounds.Max.X, FloorBounds.Min.Y, PlayerStartLocation.Z),
			FVector(FloorBounds.Max.X, FloorBounds.Max.Y, PlayerStartLocation.Z)};
		for (const FVector& FloorCorner : FloorCorners)
		{
			// persisted PlayerStart 기준 current floor corner의 lateral projection입니다.
			const float LateralProjectionCm = FVector::DotProduct(FloorCorner - PlayerStartLocation, RightAxis);
			MinimumLateralProjectionCm = FMath::Min(MinimumLateralProjectionCm, LateralProjectionCm);
			MaximumLateralProjectionCm = FMath::Max(MaximumLateralProjectionCm, LateralProjectionCm);
		}

		// PlayerStart longitudinal 위치를 보존하면서 Floor lateral centerline으로 옮길 signed offset입니다.
		StartCenterlineShiftCm = 0.5f * (MinimumLateralProjectionCm + MaximumLateralProjectionCm);
		StartTransform = BenchmarkStart->GetActorTransform();
		StartLocation = PlayerStartLocation + (RightAxis * StartCenterlineShiftCm);
		StartTransform.SetLocation(StartLocation);

		// centerline-derived start 기준 lateral floor edge 여유입니다.
		MinimumLateralProjectionCm -= StartCenterlineShiftCm;
		MaximumLateralProjectionCm -= StartCenterlineShiftCm;
		LateralAvailableCm = FMath::Min(-MinimumLateralProjectionCm, MaximumLateralProjectionCm);
		if (!FMath::IsFinite(LateralAvailableCm) || LateralAvailableCm <= 10000.0f)
		{
			OutError = FString::Printf(TEXT("ESH-04 centered lateral road authority가 100m 이하입니다. available_m=%.1f"), LateralAvailableCm * 0.01f);
			return false;
		}

		// centerline-derived start의 forward ray가 X edge와 만나는 positive distance입니다.
		float ForwardToXEdgeCm = MAX_flt;
		if (FMath::Abs(ForwardAxis.X) > KINDA_SMALL_NUMBER)
		{
			const float XEdge = ForwardAxis.X > 0.0f ? FloorBounds.Max.X : FloorBounds.Min.X;
			ForwardToXEdgeCm = (XEdge - StartLocation.X) / ForwardAxis.X;
		}
		// centerline-derived start의 forward ray가 Y edge와 만나는 positive distance입니다.
		float ForwardToYEdgeCm = MAX_flt;
		if (FMath::Abs(ForwardAxis.Y) > KINDA_SMALL_NUMBER)
		{
			const float YEdge = ForwardAxis.Y > 0.0f ? FloorBounds.Max.Y : FloorBounds.Min.Y;
			ForwardToYEdgeCm = (YEdge - StartLocation.Y) / ForwardAxis.Y;
		}
		ForwardAvailableCm = FMath::Min(ForwardToXEdgeCm, ForwardToYEdgeCm);
		if (!FMath::IsFinite(ForwardAvailableCm) || ForwardAvailableCm <= 50000.0f)
		{
			OutError = FString::Printf(TEXT("ESH-04 forward road authority가 500m 이하입니다. available_m=%.1f"), ForwardAvailableCm * 0.01f);
			return false;
		}

		// map obstacle preflight 결과입니다.
		MapCollisionObstacleCount = CollisionObstacleCount;
		return true;
	}

	// saved VehicleData를 exact benchmark start에 deferred-spawn하고 production Chaos runtime을 준비합니다.
	bool SpawnBenchmarkVehicle(FString& OutError)
	{
		// benchmark target saved VehicleData입니다.
		UCFVehicleData* VehicleDataAsset = LoadObject<UCFVehicleData>(nullptr, *VehicleDataPath);
		if (!VehicleDataAsset)
		{
			OutError = FString::Printf(TEXT("ESH-04 VehicleData를 load할 수 없습니다: %s"), *VehicleDataPath);
			return false;
		}

		// benchmark runtime에 실제 할당할 VehicleData입니다. 기본은 persisted asset 그대로입니다.
		UCFVehicleData* RuntimeVehicleData = VehicleDataAsset;
		if (RequestedChangeUpRpmOverride > 0.0f)
		{
			if (!FMath::IsFinite(RequestedChangeUpRpmOverride)
				|| RequestedChangeUpRpmOverride > VehicleDataAsset->VehicleMovementConfig.EngineMaxRPM)
			{
				OutError = FString::Printf(
					TEXT("ESH-04 transient ChangeUpRPM override가 유효 범위를 벗어났습니다. Override=%.3f EngineMaxRPM=%.3f"),
					RequestedChangeUpRpmOverride,
					VehicleDataAsset->VehicleMovementConfig.EngineMaxRPM);
				return false;
			}

			// persisted VehicleData 원본을 수정하지 않기 위한 transient duplicate입니다.
			RuntimeVehicleData = DuplicateObject<UCFVehicleData>(VehicleDataAsset, GetTransientPackage());
			if (!RuntimeVehicleData)
			{
				OutError = TEXT("ESH-04 transient VehicleData duplicate 생성에 실패했습니다.");
				return false;
			}
			RuntimeVehicleData->VehicleMovementConfig.ChangeUpRPM = RequestedChangeUpRpmOverride;
		}

		// production Vehicle Pawn Blueprint class입니다.
		UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
			nullptr,
			TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));
		if (!VehiclePawnClass)
		{
			OutError = TEXT("ESH-04 BP_CFVehiclePawn class를 load할 수 없습니다.");
			return false;
		}

		MeasurementPlayerController = PIEWorld->GetFirstPlayerController();
		if (!MeasurementPlayerController)
		{
			OutError = TEXT("ESH-04 PIE PlayerController를 찾을 수 없습니다.");
			return false;
		}
		OriginalPlayerPawn = MeasurementPlayerController->GetPawn();
		if (IsValid(OriginalPlayerPawn))
		{
			OriginalPlayerPawn->SetActorEnableCollision(false);
			OriginalPlayerPawn->SetActorHiddenInGame(true);
			OriginalPlayerPawn->SetActorLocation(StartLocation + FVector(0.0f, 0.0f, 10000.0f), false, nullptr, ETeleportType::TeleportPhysics);
		}

		TargetVehiclePawn = PIEWorld->SpawnActorDeferred<ACFVehiclePawn>(
			VehiclePawnClass,
			StartTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!TargetVehiclePawn)
		{
			OutError = TEXT("ESH-04 benchmark Vehicle Pawn deferred spawn에 실패했습니다.");
			return false;
		}
		TargetVehiclePawn->VehicleData = RuntimeVehicleData;
		TargetVehiclePawn->VehicleFittingData = nullptr;
		TargetVehiclePawn->bAutoInitializeOnBeginPlay = true;
		TargetVehiclePawn->bAutoRegisterInputMappingContext = false;
		TargetVehiclePawn->bShowAimReticle = false;
		TargetVehiclePawn->bShowTargetSelectHud = false;
		UGameplayStatics::FinishSpawningActor(TargetVehiclePawn, StartTransform);
		return true;
	}

	// PIE/map/vehicle/physics/possession 선행조건을 준비합니다.
	void UpdateSetup()
	{
		// setup을 무한 대기하지 않을 wall-clock timeout입니다.
		constexpr double ReadyTimeoutSeconds = 30.0;
		// command 시작 이후 wall-clock 경과 시간입니다.
		const double ElapsedWallSeconds = FPlatformTime::Seconds() - CommandStartTimeSeconds;
		if (!PIEWorld || !PIEWorld->AreActorsInitialized())
		{
			if (ElapsedWallSeconds >= ReadyTimeoutSeconds)
			{
				FailBenchmark(TEXT("ESH-04: 30초 안에 PIE World가 준비되지 않았습니다."));
			}
			return;
		}

		if (ForwardAvailableCm <= 0.0f)
		{
			// map authority preflight 실패 diagnostic입니다.
			FString AuthorityError;
			if (!ResolveMapAuthority(AuthorityError))
			{
				FailBenchmark(AuthorityError);
				return;
			}
		}

		if (!TargetVehiclePawn)
		{
			// benchmark target spawn 실패 diagnostic입니다.
			FString SpawnError;
			if (!SpawnBenchmarkVehicle(SpawnError))
			{
				FailBenchmark(SpawnError);
				return;
			}
		}

		if (!TargetVehiclePawn->HasActorBegunPlay())
		{
			if (ElapsedWallSeconds >= ReadyTimeoutSeconds)
			{
				FailBenchmark(TEXT("ESH-04: benchmark Pawn BeginPlay가 준비되지 않았습니다."));
			}
			return;
		}

		VehicleDriveComponent = TargetVehiclePawn->GetVehicleDriveComp();
		VehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(TargetVehiclePawn->GetVehicleMovementComponent());
		VehicleMeshComponent = TargetVehiclePawn->GetMesh();
		if (!VehicleDriveComponent || !VehicleMovementComponent || !VehicleMeshComponent
			|| !VehicleMovementComponent->HasValidPhysicsState()
			|| !VehicleMeshComponent->IsPhysicsStateCreated()
			|| !VehicleMeshComponent->IsSimulatingPhysics())
		{
			if (ElapsedWallSeconds >= ReadyTimeoutSeconds)
			{
				FailBenchmark(TEXT("ESH-04: actual Chaos physics runtime가 준비되지 않았습니다."));
			}
			return;
		}

		MeasurementPlayerController->Possess(TargetVehiclePawn);
		if (TargetVehiclePawn->GetController() != MeasurementPlayerController || !TargetVehiclePawn->IsLocallyControlled())
		{
			FailBenchmark(TEXT("ESH-04: benchmark Pawn 임시 Possess가 성립하지 않았습니다."));
			return;
		}
		TargetVehiclePawn->SetActorTickEnabled(false);

		// BeginPlay가 실제 Chaos TransmissionSetup에 materialize한 상향 변속 RPM입니다.
		EffectiveChangeUpRpm = VehicleMovementComponent->TransmissionSetup.ChangeUpRPM;
		// BeginPlay가 실제 Chaos TransmissionSetup에 materialize한 하향 변속 RPM입니다.
		EffectiveChangeDownRpm = VehicleMovementComponent->TransmissionSetup.ChangeDownRPM;
		// BeginPlay가 실제 Chaos TransmissionSetup에 materialize한 자동 변속 사용 여부입니다.
		bEffectiveAutomaticGears = VehicleMovementComponent->TransmissionSetup.bUseAutomaticGears;
		if (RequestedChangeUpRpmOverride > 0.0f
			&& !FMath::IsNearlyEqual(EffectiveChangeUpRpm, RequestedChangeUpRpmOverride, 0.1f))
		{
			FailBenchmark(FString::Printf(
				TEXT("ESH-04 transient ChangeUpRPM override가 runtime에 materialize되지 않았습니다. Requested=%.3f Effective=%.3f"),
				RequestedChangeUpRpmOverride,
				EffectiveChangeUpRpm));
			return;
		}

		// VehicleFittingData=None production BeginPlay가 확정한 Legacy Chaos configured mass입니다.
		BenchmarkConfiguredMassKg = VehicleMovementComponent->Mass;
		// 같은 fresh Physics State의 actual VehicleMesh aggregate mass입니다.
		BenchmarkActualMassKg = VehicleMeshComponent->GetMass();
		if (!FMath::IsFinite(BenchmarkConfiguredMassKg)
			|| BenchmarkConfiguredMassKg <= 0.0f
			|| !FMath::IsFinite(BenchmarkActualMassKg)
			|| BenchmarkActualMassKg <= 0.0f)
		{
			FailBenchmark(FString::Printf(
				TEXT("ESH-04: production Legacy mass readback이 유효하지 않습니다. Configured=%.3f Actual=%.3f"),
				BenchmarkConfiguredMassKg,
				BenchmarkActualMassKg));
			return;
		}

		ResetVehicleKinematics();
		SettleStartWorldTimeSeconds = PIEWorld->GetTimeSeconds();
		bSetupComplete = true;
	}

	// benchmark vehicle을 exact start/neutral/zero velocity 상태로 되돌립니다.
	void ResetVehicleKinematics()
	{
		VehicleDriveComponent->ClearDriveInputs();
		VehicleMovementComponent->SetTargetGear(0, true);
		TargetVehiclePawn->SetActorTransform(StartTransform, false, nullptr, ETeleportType::TeleportPhysics);
		VehicleMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
		VehicleMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
	}

	// setup 뒤 1초 settle을 보장하고 exact 1단/full-throttle trajectory를 시작합니다.
	void UpdateSettle()
	{
		// mass/physics settle simulation time입니다.
		constexpr float SettleSeconds = 1.0f;
		if (PIEWorld->GetTimeSeconds() - SettleStartWorldTimeSeconds < SettleSeconds)
		{
			return;
		}
		ResetVehicleKinematics();
		VehicleMovementComponent->SetTargetGear(1, true);
		DriveStartWorldTimeSeconds = PIEWorld->GetTimeSeconds();
		LastObservedGear = 0;
		bDriveStarted = true;
		VehicleDriveComponent->ApplyThrottleInput(1.0f);
	}

	// current actor forward-axis 속도를 km/h로 반환합니다.
	float GetForwardSpeedKmh() const
	{
		if (!VehicleMeshComponent)
		{
			return 0.0f;
		}
		// chassis world linear velocity입니다.
		const FVector LinearVelocity = VehicleMeshComponent->GetPhysicsLinearVelocity();
		return FMath::Max(0.0f, FVector::DotProduct(LinearVelocity, ForwardAxis) * 0.036f);
	}

	// current gear의 first-entry/dwell/peak telemetry를 갱신합니다.
	void UpdateGearObservation(const float ElapsedSeconds, const float SpeedKmh, const float EngineRpm)
	{
		// actual Chaos current gear입니다.
		const int32 CurrentGear = VehicleMovementComponent->GetCurrentGear();
		if (CurrentGear <= 0)
		{
			return;
		}
		if (CurrentGear != LastObservedGear)
		{
			// 새 gear 관측 row입니다.
			FCFHighSpeedGearObservation NewObservation;
			NewObservation.GearNumber = CurrentGear;
			NewObservation.EntryTimeSeconds = ElapsedSeconds;
			NewObservation.EntrySpeedKmh = SpeedKmh;
			NewObservation.EntryEngineRpm = EngineRpm;
			NewObservation.LastTimeSeconds = ElapsedSeconds;
			NewObservation.PeakSpeedKmh = SpeedKmh;
			NewObservation.PeakEngineRpm = EngineRpm;
			GearObservations.Add(NewObservation);
			LastObservedGear = CurrentGear;
		}
		if (GearObservations.Num() > 0 && GearObservations.Last().GearNumber == CurrentGear)
		{
			// current gear의 mutable telemetry row입니다.
			FCFHighSpeedGearObservation& CurrentObservation = GearObservations.Last();
			CurrentObservation.LastTimeSeconds = ElapsedSeconds;
			CurrentObservation.PeakSpeedKmh = FMath::Max(CurrentObservation.PeakSpeedKmh, SpeedKmh);
			CurrentObservation.PeakEngineRpm = FMath::Max(CurrentObservation.PeakEngineRpm, EngineRpm);
		}
	}

	// full-throttle long trajectory를 계측하고 stable/timeout/road-end 조건에서 종료합니다.
	void UpdateDrive()
	{
		// 충분히 긴 high-speed 관측 상한입니다. 성능 PASS threshold가 아니라 무한 실행 방지 bound입니다.
		constexpr float MaximumObservationSeconds = 150.0f;
		// 안정화 판정을 시작할 최소 주행 시간입니다.
		constexpr float MinimumStabilityStartSeconds = 60.0f;
		// 한 번의 top-speed stability 비교 window입니다.
		constexpr float StabilityWindowSeconds = 10.0f;
		// 같은 gear 10초 동안 이 이하 speed gain이면 한 stability window를 통과합니다.
		constexpr float StabilitySpeedGainToleranceKmh = 0.5f;
		// top-speed stable 판정에 필요한 연속 window 수입니다.
		constexpr int32 RequiredStableWindowCount = 2;
		// Floor end 전에 남길 안전 여유입니다.
		constexpr float RoadEndSafetyMarginCm = 10000.0f;
		VehicleDriveComponent->ApplyBrakeInput(0.0f);
		VehicleDriveComponent->ApplyThrottleInput(1.0f);
		// straight-line drivetrain fixture에서 clamp 전 chassis world angular velocity입니다.
		FVector CurrentAngularVelocityDegPerSec = VehicleMeshComponent->GetPhysicsAngularVelocityInDegrees();
		MaximumSuppressedYawRateDegPerSec = FMath::Max(MaximumSuppressedYawRateDegPerSec, FMath::Abs(CurrentAngularVelocityDegPerSec.Z));
		// handling을 평가하지 않는 ESH-04 longitudinal fixture는 world-Z yaw rate만 0으로 억제합니다.
		CurrentAngularVelocityDegPerSec.Z = 0.0f;
		VehicleMeshComponent->SetPhysicsAngularVelocityInDegrees(CurrentAngularVelocityDegPerSec, false);

		// current trajectory simulation time입니다.
		const float ElapsedSeconds = PIEWorld->GetTimeSeconds() - DriveStartWorldTimeSeconds;
		// current vehicle location입니다.
		const FVector CurrentLocation = TargetVehiclePawn->GetActorLocation();
		// start 기준 current forward progress입니다.
		const float ForwardProgressCm = FVector::DotProduct(CurrentLocation - StartLocation, ForwardAxis);
		// start 기준 current lateral offset absolute입니다.
		const float LateralOffsetCm = FMath::Abs(FVector::DotProduct(CurrentLocation - StartLocation, RightAxis));
		// current forward speed입니다.
		const float SpeedKmh = GetForwardSpeedKmh();
		// current engine RPM입니다.
		const float EngineRpm = VehicleMovementComponent->GetEngineRotationSpeed();
		// current actual forward gear입니다.
		const int32 CurrentGear = VehicleMovementComponent->GetCurrentGear();
		// current vehicle horizontal forward vector입니다.
		FVector CurrentForwardAxis = TargetVehiclePawn->GetActorForwardVector();
		CurrentForwardAxis.Z = 0.0f;
		CurrentForwardAxis.Normalize();
		// benchmark initial forward에서 current heading까지 absolute horizontal deviation입니다.
		const float HeadingDeviationDegrees = FMath::RadiansToDegrees(FMath::Acos(
			FMath::Clamp(FVector::DotProduct(ForwardAxis, CurrentForwardAxis), -1.0f, 1.0f)));
		// ESH-04 pure longitudinal authority는 steering intervention을 사용하지 않습니다.
		VehicleDriveComponent->ApplySteeringInput(0.0f);

		PeakSpeedKmh = FMath::Max(PeakSpeedKmh, SpeedKmh);
		PeakEngineRpm = FMath::Max(PeakEngineRpm, EngineRpm);
		if (TimeTo100KmhSeconds < 0.0f && SpeedKmh >= 100.0f)
		{
			TimeTo100KmhSeconds = ElapsedSeconds;
		}
		if (TimeTo150KmhSeconds < 0.0f && SpeedKmh >= 150.0f)
		{
			TimeTo150KmhSeconds = ElapsedSeconds;
		}
		if (TimeTo200KmhSeconds < 0.0f && SpeedKmh >= 200.0f)
		{
			TimeTo200KmhSeconds = ElapsedSeconds;
		}
		MaximumForwardProgressCm = FMath::Max(MaximumForwardProgressCm, ForwardProgressCm);
		MaximumLateralOffsetCm = FMath::Max(MaximumLateralOffsetCm, LateralOffsetCm);
		MaximumHeadingDeviationDegrees = FMath::Max(MaximumHeadingDeviationDegrees, HeadingDeviationDegrees);
		FinalSpeedKmh = SpeedKmh;
		FinalGear = CurrentGear;
		UpdateGearObservation(ElapsedSeconds, SpeedKmh, EngineRpm);

		if (LateralOffsetCm >= LateralAvailableCm)
		{
			FailBenchmark(FString::Printf(TEXT("ESH-04: high-speed trajectory가 persisted Floor lateral bounds를 벗어났습니다. lateral_m=%.2f floor_margin_m=%.2f"), LateralOffsetCm * 0.01f, LateralAvailableCm * 0.01f));
			return;
		}
		if (CurrentLocation.Z < FloorBounds.Min.Z - 500.0f)
		{
			FailBenchmark(TEXT("ESH-04: high-speed vehicle이 Floor 아래로 이탈했습니다."));
			return;
		}
		if (ForwardProgressCm >= ForwardAvailableCm - RoadEndSafetyMarginCm)
		{
			bRoadEndReached = true;
			FinishBenchmark(ElapsedSeconds);
			return;
		}

		if (ElapsedSeconds >= MinimumStabilityStartSeconds)
		{
			if (StabilityWindowStartSeconds < 0.0f)
			{
				StabilityWindowStartSeconds = ElapsedSeconds;
				StabilityWindowStartSpeedKmh = SpeedKmh;
				StabilityWindowStartGear = CurrentGear;
			}
			else if (ElapsedSeconds - StabilityWindowStartSeconds >= StabilityWindowSeconds)
			{
				// current stability window의 forward speed gain입니다.
				const float WindowSpeedGainKmh = SpeedKmh - StabilityWindowStartSpeedKmh;
				LastStabilityWindowSpeedGainKmh = WindowSpeedGainKmh;
				if (CurrentGear > 0
					&& CurrentGear == StabilityWindowStartGear
					&& FMath::Abs(WindowSpeedGainKmh) <= StabilitySpeedGainToleranceKmh)
				{
					++ConsecutiveStableWindowCount;
				}
				else
				{
					ConsecutiveStableWindowCount = 0;
				}
				StabilityWindowStartSeconds = ElapsedSeconds;
				StabilityWindowStartSpeedKmh = SpeedKmh;
				StabilityWindowStartGear = CurrentGear;
				if (ConsecutiveStableWindowCount >= RequiredStableWindowCount)
				{
					bTopSpeedStable = true;
					FinishBenchmark(ElapsedSeconds);
					return;
				}
			}
		}

		if (ElapsedSeconds >= MaximumObservationSeconds)
		{
			FinishBenchmark(ElapsedSeconds);
		}
	}

	// authority/result/gear telemetry를 machine-readable log로 기록하고 transient control을 정리합니다.
	void FinishBenchmark(const float ElapsedSeconds)
	{
		VehicleDriveComponent->ClearDriveInputs();
		if (Test)
		{
			Test->AddInfo(FString::Printf(
				TEXT("ESH04_HIGH_SPEED_SUMMARY | Label=%s | VehicleData=%s | DurationSec=%.3f | ConfiguredMassKg=%.3f | ActualMassKg=%.3f | EffectiveChangeUpRPM=%.3f | EffectiveChangeDownRPM=%.3f | AutomaticGears=%s | ChangeUpOverride=%s | TimeTo100KmhSec=%.3f | TimeTo150KmhSec=%.3f | TimeTo200KmhSec=%.3f | ForwardAvailableM=%.3f | CenterlineShiftM=%.3f | TravelM=%.3f | MaxLateralM=%.3f | MaxHeadingDeg=%.3f | MaxSteeringInput=%.6f | YawRateLock=True | MaxSuppressedYawRateDegPerSec=%.6f | PeakSpeedKmh=%.3f | FinalSpeedKmh=%.3f | PeakRPM=%.3f | FinalGear=%d | TopSpeedStable=%s | LastStableWindowGainKmh=%.3f | RoadEndReached=%s | CollisionObstacles=%d"),
				*Label,
				*VehicleDataPath,
				ElapsedSeconds,
				BenchmarkConfiguredMassKg,
				BenchmarkActualMassKg,
				EffectiveChangeUpRpm,
				EffectiveChangeDownRpm,
				bEffectiveAutomaticGears ? TEXT("True") : TEXT("False"),
				RequestedChangeUpRpmOverride > 0.0f ? TEXT("True") : TEXT("False"),
				TimeTo100KmhSeconds,
				TimeTo150KmhSeconds,
				TimeTo200KmhSeconds,
				ForwardAvailableCm * 0.01f,
				StartCenterlineShiftCm * 0.01f,
				MaximumForwardProgressCm * 0.01f,
				MaximumLateralOffsetCm * 0.01f,
				MaximumHeadingDeviationDegrees,
				MaximumHeadingHoldSteeringInput,
				MaximumSuppressedYawRateDegPerSec,
				PeakSpeedKmh,
				FinalSpeedKmh,
				PeakEngineRpm,
				FinalGear,
				bTopSpeedStable ? TEXT("True") : TEXT("False"),
				LastStabilityWindowSpeedGainKmh,
				bRoadEndReached ? TEXT("True") : TEXT("False"),
				MapCollisionObstacleCount));

			for (const FCFHighSpeedGearObservation& GearObservation : GearObservations)
			{
				Test->AddInfo(FString::Printf(
					TEXT("ESH04_HIGH_SPEED_GEAR | Gear=%d | EntryTimeSec=%.3f | EntrySpeedKmh=%.3f | EntryRPM=%.3f | DwellSec=%.3f | PeakSpeedKmh=%.3f | SpeedGainKmh=%.3f | PeakRPM=%.3f"),
					GearObservation.GearNumber,
					GearObservation.EntryTimeSeconds,
					GearObservation.EntrySpeedKmh,
					GearObservation.EntryEngineRpm,
					GearObservation.LastTimeSeconds - GearObservation.EntryTimeSeconds,
					GearObservation.PeakSpeedKmh,
					GearObservation.PeakSpeedKmh - GearObservation.EntrySpeedKmh,
					GearObservation.PeakEngineRpm));
			}
		}
		RestoreMeasurementControl();
		bDone = true;
	}

	// benchmark failure를 기록하고 transient control을 복원합니다.
	void FailBenchmark(const FString& FailureMessage)
	{
		if (Test)
		{
			Test->AddError(FailureMessage);
		}
		RestoreMeasurementControl();
		bDone = true;
	}

	// benchmark 종료 시 input/possession을 원래 PIE 상태로 복원합니다.
	void RestoreMeasurementControl()
	{
		if (bControlRestored)
		{
			return;
		}
		if (IsValid(VehicleDriveComponent))
		{
			VehicleDriveComponent->ClearDriveInputs();
		}
		if (MeasurementPlayerController && MeasurementPlayerController->GetPawn() == TargetVehiclePawn)
		{
			MeasurementPlayerController->UnPossess();
		}
		if (MeasurementPlayerController && IsValid(OriginalPlayerPawn))
		{
			OriginalPlayerPawn->SetActorHiddenInGame(false);
			OriginalPlayerPawn->SetActorEnableCollision(true);
			MeasurementPlayerController->Possess(OriginalPlayerPawn);
		}
		bControlRestored = true;
	}

	// 결과/오류를 기록할 Automation Test입니다.
	FAutomationTestBase* Test = nullptr;
	// saved target VehicleData exact object path입니다.
	FString VehicleDataPath;
	// 사람이 식별할 benchmark label입니다.
	FString Label;
	// optional transient ChangeUpRPM override입니다. 0 이하이면 persisted baseline을 그대로 사용합니다.
	float RequestedChangeUpRpmOverride = -1.0f;
	// command wall-clock 시작 시각입니다.
	double CommandStartTimeSeconds = 0.0;
	// current PIE World입니다.
	UWorld* PIEWorld = nullptr;
	// benchmark target vehicle입니다.
	ACFVehiclePawn* TargetVehiclePawn = nullptr;
	// benchmark input possession controller입니다.
	APlayerController* MeasurementPlayerController = nullptr;
	// benchmark 전 원래 Player Pawn입니다.
	APawn* OriginalPlayerPawn = nullptr;
	// production drive input component입니다.
	UCFVehicleDriveComp* VehicleDriveComponent = nullptr;
	// actual Chaos Movement runtime입니다.
	UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = nullptr;
	// actual chassis physics source입니다.
	USkeletalMeshComponent* VehicleMeshComponent = nullptr;
	// production no-fitting BeginPlay가 확정한 Chaos configured mass입니다.
	float BenchmarkConfiguredMassKg = 0.0f;
	// production no-fitting BeginPlay가 확정한 VehicleMesh actual aggregate mass입니다.
	float BenchmarkActualMassKg = 0.0f;
	// actual Chaos runtime에 materialize된 ChangeUpRPM입니다.
	float EffectiveChangeUpRpm = 0.0f;
	// actual Chaos runtime에 materialize된 ChangeDownRPM입니다.
	float EffectiveChangeDownRpm = 0.0f;
	// actual Chaos runtime에 materialize된 automatic gear control mode입니다.
	bool bEffectiveAutomaticGears = false;
	// exact PlayerStart spawn transform입니다.
	FTransform StartTransform = FTransform::Identity;
	// exact benchmark start world location입니다.
	FVector StartLocation = FVector::ZeroVector;
	// PlayerStart horizontal forward authority입니다.
	FVector ForwardAxis = FVector::ForwardVector;
	// PlayerStart horizontal right authority입니다.
	FVector RightAxis = FVector::RightVector;
	// exact Floor world bounds입니다.
	FBox FloorBounds = FBox(ForceInit);
	// start에서 forward floor edge까지 usable distance입니다.
	float ForwardAvailableCm = 0.0f;
	// start에서 가까운 lateral floor edge까지 usable distance입니다.
	float LateralAvailableCm = 0.0f;
	// persisted PlayerStart에서 derived Floor centerline start까지 signed lateral shift입니다.
	float StartCenterlineShiftCm = 0.0f;
	// Floor 외 collision-enabled StaticMesh obstacle 개수입니다.
	int32 MapCollisionObstacleCount = 0;
	// setup 완료 여부입니다.
	bool bSetupComplete = false;
	// full-throttle trajectory 시작 여부입니다.
	bool bDriveStarted = false;
	// command 종료 여부입니다.
	bool bDone = false;
	// transient possession/input 복원 완료 여부입니다.
	bool bControlRestored = false;
	// road end safety margin에 도달했는지 여부입니다.
	bool bRoadEndReached = false;
	// two-window top speed stability를 만족했는지 여부입니다.
	bool bTopSpeedStable = false;
	// setup settle 시작 simulation time입니다.
	float SettleStartWorldTimeSeconds = 0.0f;
	// full-throttle trajectory 시작 simulation time입니다.
	float DriveStartWorldTimeSeconds = 0.0f;
	// 현재 stability window 시작 trajectory time입니다.
	float StabilityWindowStartSeconds = -1.0f;
	// 현재 stability window 시작 speed입니다.
	float StabilityWindowStartSpeedKmh = 0.0f;
	// 현재 stability window 시작 actual gear입니다.
	int32 StabilityWindowStartGear = 0;
	// 연속 stability window PASS 개수입니다.
	int32 ConsecutiveStableWindowCount = 0;
	// 마지막 완료 stability window의 speed gain입니다.
	float LastStabilityWindowSpeedGainKmh = 0.0f;
	// trajectory peak forward speed입니다.
	float PeakSpeedKmh = 0.0f;
	// trajectory 마지막 forward speed입니다.
	float FinalSpeedKmh = 0.0f;
	// trajectory peak engine RPM입니다.
	float PeakEngineRpm = 0.0f;
	// trajectory 마지막 actual gear입니다.
	int32 FinalGear = 0;
	// full-throttle trajectory에서 100km/h에 처음 도달한 시간입니다. 미도달이면 -1입니다.
	float TimeTo100KmhSeconds = -1.0f;
	// full-throttle trajectory에서 150km/h에 처음 도달한 시간입니다. 미도달이면 -1입니다.
	float TimeTo150KmhSeconds = -1.0f;
	// full-throttle trajectory에서 200km/h에 처음 도달한 시간입니다. 미도달이면 -1입니다.
	float TimeTo200KmhSeconds = -1.0f;
	// trajectory 최대 forward progress입니다.
	float MaximumForwardProgressCm = 0.0f;
	// trajectory 최대 absolute lateral offset입니다.
	float MaximumLateralOffsetCm = 0.0f;
	// benchmark forward authority 대비 최대 horizontal heading deviation입니다.
	float MaximumHeadingDeviationDegrees = 0.0f;
	// ESH-04 pure longitudinal run의 최대 absolute steering input입니다. v1.14.0에서도 항상 0입니다.
	float MaximumHeadingHoldSteeringInput = 0.0f;
	// straight-line fixture가 매 frame 억제한 최대 absolute world-Z yaw rate입니다.
	float MaximumSuppressedYawRateDegPerSec = 0.0f;
	// 직전 frame에서 관측한 actual forward gear입니다.
	int32 LastObservedGear = 0;
	// 실제 진입한 forward gear별 telemetry입니다.
	TArray<FCFHighSpeedGearObservation> GearObservations;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleHighSpeedBenchmarkTest,
	"CarFight.VehicleBuilder.CF_FQ_040.ESH_04.HighSpeedBenchmark",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// command-line saved VehicleData를 dedicated M_VehicleBenchmark의 long full-throttle authority trajectory로 계측합니다.
bool FCFVehicleHighSpeedBenchmarkTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// runner가 전달한 exact VehicleData object path입니다.
	FString VehicleDataPath;
	if (!FParse::Value(FCommandLine::Get(), TEXT("CFHighSpeedVehicleData="), VehicleDataPath) || VehicleDataPath.IsEmpty())
	{
		AddError(TEXT("ESH-04: -CFHighSpeedVehicleData=<ObjectPath>가 필요합니다."));
		return false;
	}
	// optional human-readable label입니다.
	FString Label;
	if (!FParse::Value(FCommandLine::Get(), TEXT("CFHighSpeedLabel="), Label) || Label.IsEmpty())
	{
		Label = TEXT("HighSpeedVehicle");
	}

	// optional transient ChangeUpRPM A/B override입니다. 지정하지 않으면 persisted baseline을 그대로 사용합니다.
	float ChangeUpRpmOverride = -1.0f;
	const bool bHasChangeUpRpmOverride = FParse::Value(
		FCommandLine::Get(),
		TEXT("CFHighSpeedChangeUpRPMOverride="),
		ChangeUpRpmOverride);
	if (bHasChangeUpRpmOverride && (!FMath::IsFinite(ChangeUpRpmOverride) || ChangeUpRpmOverride <= 0.0f))
	{
		AddError(TEXT("ESH-04: -CFHighSpeedChangeUpRPMOverride는 finite 양수 RPM이어야 합니다."));
		return false;
	}

	// ESH-04 dedicated benchmark map exact path입니다.
	const FString BenchmarkMapPath = TEXT("/Game/Maps/M_VehicleBenchmark");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(BenchmarkMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFVehicleHighSpeedBenchmarkCommand(
		this,
		VehicleDataPath,
		Label,
		bHasChangeUpRpmOverride ? ChangeUpRpmOverride : -1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleBuilderTorqueCurveInspectTest,
	"CarFight.VehicleBuilder.CF_FQ_040.VB_P0_08.TorqueCurveInspect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// BP_CFVehiclePawn CDO의 inherited Chaos TorqueCurve가 Wagon vehicle-specific engine authoring 없이 실제로 어떤 shape를 제공하는지 read-only로 기록합니다.
bool FCFVehicleBuilderTorqueCurveInspectTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Production Vehicle Pawn Blueprint class입니다.
	UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));
	if (!TestNotNull(TEXT("VB-P0-08 TorqueCurveInspect BP_CFVehiclePawn class"), VehiclePawnClass))
	{
		return false;
	}

	// Blueprint CDO입니다. Product Asset은 수정하지 않습니다.
	ACFVehiclePawn* VehiclePawnCDO = Cast<ACFVehiclePawn>(VehiclePawnClass->GetDefaultObject());
	if (!TestNotNull(TEXT("VB-P0-08 TorqueCurveInspect CDO"), VehiclePawnCDO))
	{
		return false;
	}

	// CDO의 inherited Chaos Vehicle Movement Component입니다.
	UChaosWheeledVehicleMovementComponent* MovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawnCDO->GetVehicleMovementComponent());
	if (!TestNotNull(TEXT("VB-P0-08 TorqueCurveInspect Movement"), MovementComponent))
	{
		return false;
	}

	// RuntimeFloatCurve raw RichCurve입니다. X축은 실제 RPM입니다.
	const FRichCurve* TorqueCurve = MovementComponent->EngineSetup.TorqueCurve.GetRichCurveConst();
	if (!TestNotNull(TEXT("VB-P0-08 TorqueCurveInspect Curve"), TorqueCurve))
	{
		return false;
	}

	// Chaos가 normalized torque multiplier를 만들 때 사용하는 curve maximum입니다.
	float MinimumCurveValue = 0.0f;
	float MaximumCurveValue = 0.0f;
	TorqueCurve->GetValueRange(MinimumCurveValue, MaximumCurveValue);
	if (!TestTrue(TEXT("VB-P0-08 TorqueCurveInspect positive curve max"), MaximumCurveValue > KINDA_SMALL_NUMBER))
	{
		return false;
	}

	// Wagon current authored MaxRPM과 user-observed 200km/h/6th 근처 RPM을 함께 샘플링합니다.
	const float WagonMaxRpm = 6500.0f;
	const float Wagon200KmhSixthRpm = 4244.0f;
	auto NormalizeTorqueAtRpm = [TorqueCurve, MaximumCurveValue](const float EngineRpm)
	{
		return TorqueCurve->Eval(EngineRpm) / MaximumCurveValue;
	};

	AddInfo(FString::Printf(
		TEXT("VB-P0-08 TorqueCurveInspect | CurveMin=%.6f | CurveMax=%.6f | Rpm0=%.6f | Rpm1800=%.6f | Rpm3250=%.6f | Rpm4244=%.6f | Rpm4800=%.6f | Rpm5400=%.6f | Rpm5700=%.6f | Rpm6500=%.6f"),
		MinimumCurveValue,
		MaximumCurveValue,
		NormalizeTorqueAtRpm(0.0f),
		NormalizeTorqueAtRpm(1800.0f),
		NormalizeTorqueAtRpm(0.5f * WagonMaxRpm),
		NormalizeTorqueAtRpm(Wagon200KmhSixthRpm),
		NormalizeTorqueAtRpm(4800.0f),
		NormalizeTorqueAtRpm(5400.0f),
		NormalizeTorqueAtRpm(5700.0f),
		NormalizeTorqueAtRpm(WagonMaxRpm)));

	return true;
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

	// Optional Guided progress exact RunId입니다. standalone direct Automation 호출은 생략할 수 있습니다.
	FString RunId;
	// Optional Guided progress canonical absolute sidecar path입니다.
	FString ProgressFilePath;
	// RunId 인자 존재 여부입니다.
	const bool bHasRunId = FParse::Value(FCommandLine::Get(), TEXT("CFBuilderBenchmarkRunId="), RunId) && !RunId.IsEmpty();
	// Progress path 인자 존재 여부입니다.
	const bool bHasProgressPath = FParse::Value(FCommandLine::Get(), TEXT("CFBuilderBenchmarkProgressPath="), ProgressFilePath) && !ProgressFilePath.IsEmpty();
	if (bHasRunId != bHasProgressPath)
	{
		AddWarning(TEXT("VB-P0-08 progress transport는 RunId와 ProgressPath가 함께 필요합니다. 진행 표시만 비활성화하고 benchmark는 계속합니다."));
		RunId.Reset();
		ProgressFilePath.Reset();
	}
	else if (bHasRunId && bHasProgressPath)
	{
		// Canonical lowercase-hyphenated RunId로 normalize할 parsed GUID입니다.
		FGuid ParsedRunId;
		if (!FGuid::Parse(RunId, ParsedRunId))
		{
			AddWarning(TEXT("VB-P0-08 progress RunId가 GUID가 아닙니다. 진행 표시만 비활성화하고 benchmark는 계속합니다."));
			RunId.Reset();
			ProgressFilePath.Reset();
		}
		else if (FPaths::IsRelative(ProgressFilePath))
		{
			AddWarning(TEXT("VB-P0-08 progress path는 absolute path여야 합니다. 진행 표시만 비활성화하고 benchmark는 계속합니다."));
			RunId.Reset();
			ProgressFilePath.Reset();
		}
		else
		{
			// Runtime writer가 허용할 ProjectSaved/CarFight canonical sidecar exact path입니다.
			FString CanonicalProgressFilePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
				FPaths::ProjectSavedDir(),
				TEXT("CarFight"),
				TEXT("VehicleBuilderBenchmarkProgress.json")));
			// Command-line에서 받은 absolute path의 normalized 비교본입니다.
			FString NormalizedRequestedProgressPath = FPaths::ConvertRelativePathToFull(ProgressFilePath);
			FPaths::NormalizeFilename(CanonicalProgressFilePath);
			FPaths::NormalizeFilename(NormalizedRequestedProgressPath);
			if (!NormalizedRequestedProgressPath.Equals(CanonicalProgressFilePath, ESearchCase::IgnoreCase))
			{
				AddWarning(TEXT("VB-P0-08 progress path가 canonical ProjectSaved/CarFight sidecar와 다릅니다. 진행 표시만 비활성화하고 benchmark는 계속합니다."));
				RunId.Reset();
				ProgressFilePath.Reset();
			}
			else
			{
				RunId = ParsedRunId.ToString(EGuidFormats::DigitsWithHyphensLower);
				ProgressFilePath = MoveTemp(CanonicalProgressFilePath);
			}
		}
	}

	// 기존 production test map을 읽기 전용 technical road/physics fixture로 재사용합니다.
	const FString BenchmarkMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(BenchmarkMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFBuilderDrivingBenchmarkCommand(
		this,
		VehicleDataPath,
		FittingDataPath,
		Label,
		RunId,
		ProgressFilePath));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
