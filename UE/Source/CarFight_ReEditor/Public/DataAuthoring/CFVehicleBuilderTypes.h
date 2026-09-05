// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTypes.h
// Version: v1.6.0
// Date: 2026-09-04
// Description: Guided Vehicle Builder의 Editor-only Step/Benchmark progress + Step 7 durable handoff + Step 8 Driving Apply/Recipe Save typed state입니다.
// Changelog:
// - v1.6.0: VBHAI-P0-07H에서 Step 7 semantic review와 durable Target/Recipe handoff를 분리하는 saved-handoff/final-commit typed state와 post-driving receipt save pending phase를 추가.
// - v1.5.0: VBHAI-P0-07E에서 exact RunId benchmark progress projection과 persistent USER Driving receipt 기반 current Recipe explicit Save preflight/outcome을 추가.
// - v1.4.0: CF-FQ-047 VBHAI-P0-07B에서 Step 8 PIE Apply 버튼과 production Apply guard가 공유하는 typed blocker/preflight projection을 추가.
// - v1.3.0: CF-FQ-046 VBIUX-P0-02에서 Step 3/4가 동일 current Socket/VehicleData truth를 공유하도록 read-only FCFVehicleBuilderLayoutFacts를 추가.
// - v1.2.0: VB-P0-09 Step 8에서 기존 VB-P0-08 runner JSON을 Guided Shell이 읽기 위한 transient benchmark identity/metric/result projection을 추가.
// - v1.1.0: StepId를 stable semantic identity로 명시하고 current 8-step baseline의 순서/개수를 presentation definition에서 분리.
// - v1.0.0: 고정 8 Step ID, 6-state, USER-facing step projection을 추가.
// Migration:
// - v1.6.0 Final Commit state는 transient orchestration contract입니다. Final Review의 기존 semantic bCanCompleteFinalReview 의미를 변경하지 않으며 Save All/StaticMesh/Profile/Evidence/Catalog authority를 추가하지 않습니다.
// - v1.5.0 progress type은 Editor reader/presentation 전용이며 Runtime benchmark writer와 module dependency를 공유하지 않습니다. Recipe Save type은 exact current Recipe package 1개의 USER-click Save만 표현하며 Save All/Target/StaticMesh/Catalog authority를 추가하지 않습니다.
// - v1.4.0 Driving Apply preflight는 transient read-only projection이며 PIE lifecycle, VehicleData mutation, Save authority를 추가하지 않습니다.
// - v1.3.0 LayoutFacts는 transient read-only projection이며 Layout 계산/Apply/Save authority를 추가하지 않습니다.
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

/** Step 8 current saved VehicleData를 active PIE에 transient 적용하기 전의 stable blocker입니다. */
enum class ECFVehicleDrivingApplyBlocker : uint8
{
	None,
	BenchmarkUnavailable,
	BenchmarkStale,
	RecipeUnavailable,
	TargetUnavailable,
	RecipeUnsaved,
	TargetUnsaved,
	RecipeNotPersisted,
	TargetNotPersisted,
	AppliedStateStale,
	StateReadFailed
};

/** Step 8 PIE Apply 버튼과 production Apply guard가 공유하는 read-only stable preflight입니다. */
struct FCFVehicleDrivingApplyPreflight
{
	// 현재 stable apply blocker입니다.
	ECFVehicleDrivingApplyBlocker Blocker = ECFVehicleDrivingApplyBlocker::StateReadFailed;

	// Raw path/hash/error를 포함할 수 있는 진단 정보입니다. USER 기본 문구는 Presentation formatter가 별도로 만듭니다.
	FString Diagnostic;

	// Stable persisted/benchmark prerequisite가 모두 충족됐는지 반환합니다.
	bool CanApply() const { return Blocker == ECFVehicleDrivingApplyBlocker::None; }
};

/** Step 7 durable Target/Recipe handoff를 읽을 때 identity/package 안전 문제를 표현합니다. */
enum class ECFBuilderSavedHandoffBlocker : uint8
{
	None,
	RecipeUnavailable,
	TargetUnavailable,
	SelectionMismatch,
	PackageInvalid,
	TargetDefinitionStale,
	StateReadFailed
};

/** Step 7/8이 공유하는 fresh Target + fresh Resolve + AppliedState + package 상태입니다. */
struct FCFBuilderSavedHandoffPreflight
{
	// Identity/package 안전성 blocker입니다. Dirty/persistence pending 자체는 commit action이 처리하므로 blocker와 분리합니다.
	ECFBuilderSavedHandoffBlocker Blocker = ECFBuilderSavedHandoffBlocker::StateReadFailed;

	// Current Recipe exact object path입니다.
	FSoftObjectPath RecipePath;

	// Current Target VehicleData exact object path입니다.
	FSoftObjectPath TargetPath;

	// Fresh Recipe semantic fingerprint입니다.
	FString RecipeFingerprint;

	// Fresh Resolver source signature입니다.
	FString SourceSignature;

	// Live Target UObject에서 다시 계산한 current DefinitionHash입니다.
	FString TargetDefinitionHash;

	// Fresh Resolve가 산출한 prospective DefinitionHash입니다.
	FString ResolvedDefinitionHash;

	// Fresh Resolver contract revision입니다.
	int32 ResolverContractRevision = 0;

	// Current Recipe package에 미저장 변경이 있는지 여부입니다.
	bool bRecipeDirty = false;

	// Current Target package에 미저장 변경이 있는지 여부입니다.
	bool bTargetDirty = false;

	// Current Recipe package가 disk에 존재하는지 여부입니다.
	bool bRecipePersisted = false;

	// Current Target package가 disk에 존재하는지 여부입니다.
	bool bTargetPersisted = false;

	// Recipe AppliedState 4개 identity가 fresh Recipe/Resolve/Target과 exact 일치하는지 여부입니다.
	bool bAppliedStateCurrent = false;

	// USER 기본 문구와 분리할 내부 진단 정보입니다.
	FString Diagnostic;

	// Step 8 pre-benchmark handoff가 실제 durable ready인지 반환합니다.
	bool IsStrictReady() const
	{
		return Blocker == ECFBuilderSavedHandoffBlocker::None
			&& !TargetDefinitionHash.IsEmpty()
			&& TargetDefinitionHash == ResolvedDefinitionHash
			&& !bRecipeDirty
			&& !bTargetDirty
			&& bRecipePersisted
			&& bTargetPersisted
			&& bAppliedStateCurrent;
	}
};

/** Step 7 primary action이 현재 semantic/durable 상태에서 수행할 exact 동작입니다. */
enum class ECFBuilderFinalCommitAction : uint8
{
	None,
	ApplyAndPersist,
	PersistDirtyPair,
	FinalizeAppliedStateAndPersist,
	PostDrivingReceiptSavePending,
	Blocked
};

/** Step 7 USER confirmation과 writer가 공유하는 current final-commit preflight입니다. */
struct FCFBuilderFinalCommitPreflight
{
	// Current Step 7 action입니다.
	ECFBuilderFinalCommitAction Action = ECFBuilderFinalCommitAction::Blocked;

	// Final Review와 동일 fresh identity에서 읽은 durable handoff facts입니다.
	FCFBuilderSavedHandoffPreflight SavedHandoff;

	// USER가 확인한 exact action/identity를 mutation 직전 다시 비교할 transient approval scope입니다.
	FString ApprovalScopeHash;

	// USER 기본 문구와 분리할 내부 진단 정보입니다.
	FString Diagnostic;

	// USER primary action이 실제 mutation/save를 수행할 수 있는 상태인지 반환합니다.
	bool CanExecute() const
	{
		return Action == ECFBuilderFinalCommitAction::ApplyAndPersist
			|| Action == ECFBuilderFinalCommitAction::PersistDirtyPair
			|| Action == ECFBuilderFinalCommitAction::FinalizeAppliedStateAndPersist;
	}

	// Step 7 자체는 durable handoff가 이미 성립했거나 downstream USER receipt save만 남았는지 반환합니다.
	bool IsStepComplete() const
	{
		return Action == ECFBuilderFinalCommitAction::None
			|| Action == ECFBuilderFinalCommitAction::PostDrivingReceiptSavePending;
	}
};

/** Step 7 durable final commit의 terminal outcome입니다. */
enum class ECFBuilderFinalCommitOutcome : uint8
{
	Committed,
	NoChange,
	PostDrivingReceiptSavePending,
	Blocked,
	ApplyFailed,
	FinalizeAppliedStateFailed,
	TargetSaveFailed,
	TargetSavedRecipeSaveFailed,
	SaveStateUnconfirmed,
	CommittedRefreshWarning
};

/** Step 7 apply/finalize/pair-save terminal result입니다. */
struct FCFBuilderFinalCommitResult
{
	// Writer가 확정한 terminal outcome입니다.
	ECFBuilderFinalCommitOutcome Outcome = ECFBuilderFinalCommitOutcome::Blocked;

	// 기존 DefinitionApply가 Target을 실제 변경했는지 여부입니다.
	bool bTargetApplied = false;

	// no-diff repair가 Recipe AppliedState만 실제 갱신했는지 여부입니다.
	bool bAppliedStateFinalized = false;

	// Exact Target package를 이번 operation에서 저장했는지 여부입니다.
	bool bTargetSaved = false;

	// Exact Recipe package를 이번 operation에서 저장했는지 여부입니다.
	bool bRecipeSaved = false;

	// Apply가 guarded Undo token을 발급했는지 여부입니다.
	bool bUndoAvailable = false;

	// Apply가 발급한 exact guarded Undo token identity는 VM의 existing token owner가 보존하므로 여기에는 기술 진단만 기록합니다.
	FString Diagnostic;

	// Durable pair commit이 persistent success인지 반환합니다.
	bool IsPersistentCommitSuccess() const
	{
		return Outcome == ECFBuilderFinalCommitOutcome::Committed
			|| Outcome == ECFBuilderFinalCommitOutcome::CommittedRefreshWarning
			|| Outcome == ECFBuilderFinalCommitOutcome::NoChange
			|| Outcome == ECFBuilderFinalCommitOutcome::PostDrivingReceiptSavePending;
	}
};

/** Step 8 benchmark runtime이 USER에게 노출할 coarse progress 단계입니다. */
enum class ECFVehicleBuilderBenchmarkProgressPhase : uint8
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

/** Step 8이 progress sidecar에서 읽은 exact RunId-bound coarse 진행 상태입니다. */
struct FCFVehicleBuilderBenchmarkProgress
{
	// Progress sidecar schema identity입니다.
	FString SchemaVersion;

	// 이 progress가 속한 exact benchmark run identity입니다.
	FString RunId;

	// USER-facing coarse phase입니다.
	ECFVehicleBuilderBenchmarkProgressPhase Phase = ECFVehicleBuilderBenchmarkProgressPhase::None;

	// 1부터 시작하는 current coarse phase index입니다.
	int32 PhaseIndex = 0;

	// 현재 progress schema의 전체 coarse phase 수입니다.
	int32 PhaseCount = 7;
};

/** Step 8 explicit current Recipe Save를 막는 stable blocker입니다. */
enum class ECFVehicleRecipeSaveBlocker : uint8
{
	None,
	RecipeUnavailable,
	SelectionMismatch,
	TargetUnavailable,
	AcceptanceUnavailableOrStale,
	PackageInvalid,
	StateReadFailed
};

/** Step 8 USER-click Recipe Save 버튼과 writer가 공유하는 current exact preflight입니다. */
struct FCFVehicleRecipeSavePreflight
{
	// Current Recipe Save blocker입니다.
	ECFVehicleRecipeSaveBlocker Blocker = ECFVehicleRecipeSaveBlocker::StateReadFailed;

	// Current Recipe package에 미저장 변경이 있는지 여부입니다.
	bool bRecipeDirty = false;

	// USER 기본 문구와 분리할 내부 진단 정보입니다.
	FString Diagnostic;

	// Exact current Recipe를 저장할 수 있고 실제 dirty인지 반환합니다.
	bool CanSave() const { return Blocker == ECFVehicleRecipeSaveBlocker::None && bRecipeDirty; }

	// Exact current acceptance는 유효하지만 package가 이미 clean인지 반환합니다.
	bool IsNoSaveNeeded() const { return Blocker == ECFVehicleRecipeSaveBlocker::None && !bRecipeDirty; }
};

/** Step 8 exact current Recipe explicit Save terminal outcome입니다. */
enum class ECFVehicleRecipeSaveOutcome : uint8
{
	Saved,
	NoSaveNeeded,
	Blocked,
	SaveFailed,
	SaveStateUnconfirmed,
	SavedRefreshWarning
};

/** Step 8 explicit Recipe Save의 terminal outcome과 진단을 함께 보존합니다. */
struct FCFVehicleRecipeSaveResult
{
	// Writer가 확정한 terminal outcome입니다.
	ECFVehicleRecipeSaveOutcome Outcome = ECFVehicleRecipeSaveOutcome::Blocked;

	// Save 또는 post-save refresh의 내부 진단 정보입니다.
	FString Diagnostic;

	// SavePackage 성공 + package clean이 성립한 persistent write success인지 반환합니다.
	bool IsPersistentSaveSuccess() const
	{
		return Outcome == ECFVehicleRecipeSaveOutcome::Saved
			|| Outcome == ECFVehicleRecipeSaveOutcome::SavedRefreshWarning;
	}
};

/** Step 3/4가 동일 current Socket/VehicleData truth에서 읽는 transient 차량 배치 사실입니다. */
struct FCFVehicleBuilderLayoutFacts
{
	// 현재 Recipe/Asset/Target에서 배치 사실을 정상적으로 읽었는지 여부입니다.
	bool bAvailable = false;

	// 네 Wheel Socket fact가 모두 current Chassis에 존재하는지 여부입니다.
	bool bAllWheelSocketsFound = false;

	// +X 전방 기준 앞/뒤 차축 중점 사이 거리 cm입니다.
	float WheelbaseCm = 0.0f;

	// 앞바퀴 좌우 Socket 사이 거리 cm입니다.
	float FrontTrackCm = 0.0f;

	// 뒷바퀴 좌우 Socket 사이 거리 cm입니다.
	float RearTrackCm = 0.0f;

	// Current Target VehicleData가 persisted Layout override를 사용 중인지 여부입니다.
	bool bTargetUsesLayoutOverrides = false;

	// Persisted VehicleData Wheel layout과 current Socket truth 사이의 mismatch 수입니다.
	int32 WheelLayoutMismatchCount = 0;

	// Persisted Hardpoint 위치와 current Socket truth 사이의 non-blocking warning 수입니다.
	int32 HardpointLayoutWarningCount = 0;
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
