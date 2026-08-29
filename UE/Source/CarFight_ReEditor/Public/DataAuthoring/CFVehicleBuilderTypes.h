// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTypes.h
// Version: v1.2.0
// Date: 2026-08-27
// Description: CF-FQ-040 Guided Vehicle Builder의 Editor-only 8-step presentation types입니다.
// Changelog:
// - v1.2.0: VB-P0-09 Step 8에서 기존 VB-P0-08 runner JSON을 Guided Shell이 읽기 위한 transient benchmark identity/metric/result projection을 추가.
// - v1.1.0: StepId를 stable semantic identity로 명시하고 current 8-step baseline의 순서/개수를 presentation definition에서 분리.
// - v1.0.0: 고정 8 Step ID, 6-state, USER-facing step projection을 추가.
// Migration:
// - Persistent authoring truth를 저장하지 않습니다. 모든 상태는 current Recipe/Asset truth에서 다시 파생합니다.

#pragma once

#include "CoreMinimal.h"

/** Guided Vehicle Builder Step의 stable semantic identity입니다. 현재 baseline은 8개지만 presentation 순서/개수와 분리합니다. */
enum class ECFVehicleBuilderStepId : uint8
{
	IdentityReference,
	MeshPrep,
	SocketGuide,
	LayoutCapture,
	PhysicsProposal,
	GameplaySetup,
	FinalReview,
	DrivingTest
};

/** Builder Step의 transient derived 상태입니다. */
enum class ECFVehicleBuilderStepState : uint8
{
	Unavailable,
	Locked,
	Ready,
	Complete,
	Blocked,
	Stale
};

/** 한 Builder Step을 Slate에 표시하기 위한 transient projection입니다. */
/** 기존 VB-P0-08 Technical Driving Benchmark가 출력한 차량별 runtime metric projection입니다. */
struct FCFVehicleBuilderDrivingMetric
{
	// 사람이 benchmark run을 식별할 label입니다.
	FString Label;

	// 실제 benchmark target VehicleData exact object path입니다.
	FString VehicleDataPath;

	// optional FittingData exact object path입니다. 현재 Guided Step 8 기본 flow에서는 비어 있습니다.
	FString FittingDataPath;

	// Runtime에 적용된 configured vehicle mass입니다.
	double ConfiguredMassKg = 0.0;

	// 실제 physics body가 보고한 mass입니다.
	double ActualMassKg = 0.0;

	// 정지에서 50km/h 최초 도달 시간입니다.
	double Acceleration0To50Seconds = -1.0;

	// 정지에서 100km/h 최초 도달 시간입니다. 미도달이면 -1입니다.
	double Acceleration0To100Seconds = -1.0;

	// Full throttle 관측 구간의 peak forward speed입니다.
	double PeakSpeedKmh = 0.0;

	// Top-speed 후반 관측 구간이 stable condition을 만족했는지 여부입니다.
	bool bTopSpeedStable = false;

	// 전체 benchmark에서 관측한 peak engine RPM입니다.
	double PeakEngineRpm = 0.0;

	// Peak speed 시점의 actual Chaos current gear입니다.
	int32 PeakSpeedGear = 0;

	// 100km/h braking metric을 실제 수행할 수 있었는지 여부입니다.
	bool bBraking100Available = false;

	// Brake 1.0 입력 직전 실제 시작 속도입니다.
	double BrakingStartKmh = 0.0;

	// 100km/h 부근에서 Idle threshold까지 감속한 시간입니다.
	double Braking100ToIdleSeconds = -1.0;

	// 같은 braking 구간의 planar distance입니다.
	double Braking100ToIdleDistanceMeters = -1.0;

	// 30km/h / steering 0.5 기준 steady-turn yaw response입니다.
	double SteadyYawDegrees = 0.0;

	// 저속 full-steer effective turning radius입니다.
	double EffectiveTurningRadiusMeters = -1.0;

	// Turning radius 계측 구간의 평균 속도입니다.
	double TurningAverageSpeedKmh = 0.0;
};

/** Step 8이 current Target과 exact binding해 소비하는 VB-P0-08 result envelope입니다. */
struct FCFVehicleBuilderBenchmarkResult
{
	// Runner JSON schema identity입니다.
	FString SchemaVersion;

	// Runner terminal status입니다.
	FString Status;

	// Shell/runner 한 회 실행을 식별하는 GUID 문자열입니다.
	FString RunId;

	// Shell launch 당시 current Target semantic hash입니다.
	FString ExpectedTargetDefinitionHash;

	// Benchmark process가 끝나 결과 JSON을 쓴 UTC timestamp입니다.
	FString CompletedUtc;

	// 기존 runner가 Reference threshold를 임의 판정하지 않았음을 나타냅니다.
	bool bReferenceThresholdAsserted = false;

	// 기존 runner가 USER Driving Feel을 대신 판정하지 않았음을 나타냅니다.
	bool bUserDrivingFeelAsserted = false;

	// Exact one metric이 존재하는지 여부입니다.
	bool bHasMetric = false;

	// Parsed technical mobility metric입니다.
	FCFVehicleBuilderDrivingMetric Metric;
};

/** 한 Builder Step을 Slate에 표시하기 위한 transient projection입니다. */
struct FCFVehicleBuilderStepView
{
	// 고정 Step identity입니다.
	ECFVehicleBuilderStepId StepId = ECFVehicleBuilderStepId::IdentityReference;

	// 화면에 표시할 순서입니다.
	int32 StepNumber = 1;

	// USER-facing 단계 이름입니다.
	FText Title;

	// 현재 authoritative truth에서 파생한 상태입니다.
	ECFVehicleBuilderStepState State = ECFVehicleBuilderStepState::Locked;

	// 현재 단계가 무엇을 확인하는지 설명합니다.
	FText Summary;

	// 막힘 또는 미완료 시 USER가 해야 할 구체적인 다음 행동입니다.
	FText Resolution;

	// 현재 shell slice에서 해당 단계가 실제 provider에 연결됐는지 여부입니다.
	bool bProviderConnected = false;
};
