// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderPresent.h
// Version: v1.5.0
// Date: 2026-09-04
// Description: CF-FQ-046 USER presentation + CF-FQ-047 Hardpoint/Mount semantic readback / Step 8 progress·Driving Apply·Recipe Save formatter입니다.
// Changelog:
// - v1.5.0: VBHAI-P0-07E에서 cached 7단계 benchmark progress/elapsed와 exact current Recipe explicit Save preflight/outcome을 USER-facing 한국어로 변환하는 formatter를 추가.
// - v1.4.0: VBHAI-P0-07B에서 typed Driving Apply blocker와 benchmark-running 상태를 raw path/hash 없이 USER가 바로 복구할 수 있는 한국어 안내로 변환하는 formatter를 추가.
// - v1.3.1: CF-FQ-046×047 통합 정리. .h/.cpp 버전·changelog를 동기화하고 USER presentation ownership과 Hardpoint integrity authority 경계를 명시. formatter logic 변경 없음.
// - v1.3.0: CF-FQ-047 Hardpoint/Mount semantic readback formatter를 추가해 Step 6/7/8의 typed 장착 상태를 USER 문장으로 투영.
// - v1.2.0: VBIUX-P0-04. Step 7 Final Review typed diff의 human-readable grouping/field descriptor/unknown fallback과 Step 8 benchmark·주행 checklist formatter를 추가.
// - v1.1.0: VBIUX-P0-03. Step 5 AI Draft와 승인 완료 Builder-private Profile을 명확히 구분하고 엔진/변속기/조향·제동/서스펜션/질량 수치를 값+뜻+주행 영향으로 표시하는 formatter를 추가.
// - v1.0.0: Step 1~4의 목적/상태/다음 행동, Reference/Mesh/Socket/Layout typed truth를 USER 문장으로 변환하는 pure presentation 계약을 추가.
// Migration:
// - v1.5.0 progress/save formatter는 입력받은 typed truth만 표시하며 disk polling, SavePackage, receipt validation authority를 소유하지 않습니다.
// - v1.4.0 Driving Apply readiness는 typed blocker를 표시 문장으로만 변환하며 stable readiness authority는 BuilderVM, benchmark process/wrong-step 상태는 BuilderTab에 남습니다.
// - v1.3.x에서 이 helper는 CF-FQ-047 typed truth를 USER 문장으로 투영만 하며 Hardpoint integrity/receipt validation/mutation authority는 FCFVehicleBuilderHardpointIntegrity/BuilderVM에 남습니다.
// - Persistent state, UObject load, Save, Recipe/VehicleData mutation을 수행하지 않습니다. 입력받은 typed truth만 표시용 문자열로 변환합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "DataAuthoring/CFVehicleBuilderTypes.h"
#include "DataAuthoring/CFVehicleBuilderHardpointIntegrity.h"

class UStaticMesh;

/** Step 5에 표시하는 물리값이 AI 제안인지 승인 완료 제작 프로필인지 구분합니다. */
enum class ECFVehiclePhysicsPresentationSource : uint8
{
	AiDraft,
	ApprovedBuilderProfile
};

/** Step 1 기준 자료를 USER 문장으로 만들 때 필요한 scalar presentation 입력입니다. */
struct FCFVehicleReferencePresentInfo
{
	// Persistent Reference Evidence가 현재 존재하는지 여부입니다.
	bool bHasPersistentEvidence = false;

	// Persistent Evidence가 없을 때 loaded AI Research Draft가 존재하는지 여부입니다.
	bool bHasLoadedDraft = false;

	// Current 기준 자료가 USER review를 통과한 Step Complete 상태인지 여부입니다.
	bool bUserReviewed = false;

	// Primary Reference 제조사입니다.
	FString Manufacturer;

	// Primary Reference 모델명입니다.
	FString Model;

	// Primary Reference 시작 연식입니다.
	int32 ModelYearStart = 0;

	// Primary Reference 트림입니다.
	FString Trim;

	// Primary Reference 파워트레인 설명입니다.
	FString Powertrain;

	// Primary Reference 변속기 설명입니다.
	FString Transmission;

	// 현재 기준 자료가 참고하는 Source 수입니다.
	int32 SourceCount = 0;

	// 전체 conflict 수입니다.
	int32 ConflictCount = 0;

	// 실제 진행을 막는 unresolved conflict 수입니다.
	int32 BlockingConflictCount = 0;

	// 전체 unknown fact 수입니다.
	int32 UnknownCount = 0;

	// Physics proposal 진행을 막는 unknown fact 수입니다.
	int32 BlockingUnknownCount = 0;
};

/** Step 3 required Wheel Socket 상태를 USER 문장으로 만들 때 필요한 presentation 입력입니다. */
struct FCFVehicleSocketPresentInfo
{
	// FL/FR/RL/RR 순서의 effective Wheel Socket 이름입니다.
	TArray<FName> WheelSocketNames;

	// WheelSocketNames와 같은 순서의 current Chassis 존재 여부입니다.
	TArray<bool> WheelSocketFound;

	// Step 3/4가 공유하는 current 차량 배치 사실입니다.
	FCFVehicleBuilderLayoutFacts LayoutFacts;
};

/** Step 8 saved-state와 benchmark/user-drive 상태를 pure formatter에 전달하는 presentation 입력입니다. */
struct FCFVehicleDrivingPresentInfo
{
	// Current Recipe package에 저장되지 않은 변경이 있는지 여부입니다.
	bool bRecipeDirty = false;

	// Current Target VehicleData package에 저장되지 않은 변경이 있는지 여부입니다.
	bool bTargetDirty = false;

	// Current Target에 유효하게 binding된 benchmark 결과가 있는지 여부입니다.
	bool bHasBenchmarkResult = false;

	// Current session PIE에 선택 차량을 적용해 직접 주행할 준비가 되었는지 여부입니다.
	bool bUserDrivePrepared = false;

	// Current Target 상태에 대한 USER 주행 PASS가 이미 존재하는지 여부입니다.
	bool bUserDrivingAccepted = false;

	// 유효할 때 표시할 current benchmark typed result입니다.
	FCFVehicleBuilderBenchmarkResult BenchmarkResult;
};

/** Guided Vehicle Builder USER-facing 문자열 변환만 소유하는 Editor-private pure helper입니다. */
class FCFVehicleBuilderPresentation
{
public:
	// Stable StepId를 초보자용 단계 이름으로 변환합니다.
	static FText GetStepTitle(ECFVehicleBuilderStepId StepId, const FText& FallbackTitle);

	// Backend Step state를 USER 상태 표현으로 변환합니다.
	static FString GetStepStateLabel(ECFVehicleBuilderStepState State);

	// Step 1~8이 무엇을 확인하는 단계인지 기본 목적 설명을 반환합니다.
	static FText GetStepPurpose(ECFVehicleBuilderStepId StepId, const FText& FallbackSummary);

	// Step 1~4의 현재 state에 맞는 USER next action을 반환합니다.
	static FText GetStepNextAction(const FCFVehicleBuilderStepView& Step, const FText& FallbackResolution);

	// Step 1 persistent Evidence 또는 loaded Draft scalar truth를 사용자용 기준 자료 요약으로 변환합니다.
	static FText BuildReferenceSummary(const FCFVehicleReferencePresentInfo& Info);

	// Step 2 Recipe AssetIntent를 사용자용 Chassis/Wheel Mesh 요약으로 변환합니다.
	static FText BuildMeshSummary(const FCFVehicleAssetIntent& AssetIntent);

	// Step 3 required Wheel Socket + shared layout facts를 사용자용 요약으로 변환합니다.
	static FText BuildSocketSummary(const FCFVehicleSocketPresentInfo& Info);

	// Step 4 current Socket/VehicleData layout facts를 사용자용 배치 요약으로 변환합니다.
	static FText BuildLayoutSummary(
		const FCFVehicleBuilderLayoutFacts& Facts,
		ECFVehicleBuilderStepState State);

	// Step 5 AI Draft 또는 승인 완료 Builder-private Profile을 값+뜻+주행 영향 구조로 변환합니다.
	static FText BuildPhysicsSummary(
		const FCFBuilderPrivateProfilePayload& ProfilePayload,
		ECFVehiclePhysicsPresentationSource Source);

	// Step 6 Recipe authored Hardpoint↔Mount 관계를 completion authority와 분리된 USER 참고 정보로 변환합니다.
	static FText BuildMountSemanticSummary(const FCFBuilderHardpointSemanticFacts& Facts);

	// Step 7 current Final Review의 typed diff를 Before→After + 의미/영향 구조로 변환합니다.
	static FText BuildFinalReviewSummary(const FCFBuilderFinalReviewResult& Review);

	// Step 7 current Target vs fresh prospective Resolve의 Hardpoint/Mount count를 분리 표시합니다.
	static FText BuildFinalReviewHardpointReadback(const FCFBuilderHardpointSemanticFacts& Facts);

	// Step 8 saved-state/benchmark/checklist를 USER 판단용 정보로 변환합니다.
	static FText BuildDrivingSummary(const FCFVehicleDrivingPresentInfo& Info);

	// Step 8 PIE Apply preflight blocker를 raw hash/path 없이 USER recovery 문장으로 변환합니다.
	static FText BuildDrivingApplyReadiness(
		const FCFVehicleDrivingApplyPreflight& Preflight,
		bool bBenchmarkRunning);

	// Running benchmark의 cached coarse phase와 monotonic elapsed seconds를 가짜 ETA 없이 USER 문장으로 변환합니다.
	static FText BuildDrivingBenchmarkProgress(
		bool bRunning,
		bool bHasProgress,
		const FCFVehicleBuilderBenchmarkProgress& Progress,
		double ElapsedSeconds);

	// Current persistent acceptance receipt/Recipe dirty preflight를 exact Save scope가 드러나는 USER 문장으로 변환합니다.
	static FText BuildDrivingRecipeSaveStatus(const FCFVehicleRecipeSavePreflight& Preflight);

	// Exact current Recipe Save terminal outcome을 USER success/warning/recovery 문장으로 변환합니다.
	static FText BuildDrivingRecipeSaveResult(const FCFVehicleRecipeSaveResult& Result);

	// Step 8 current Target VehicleData의 실제 Hardpoint/Mount readback만 표시합니다.
	static FText BuildDrivingHardpointReadback(const FCFBuilderHardpointSemanticFacts& Facts);
};
