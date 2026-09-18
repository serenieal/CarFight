// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.15.1
// Date: 2026-09-18
// Description: Phase 3B-1 Persistent Knowledge Store Runtime / Phase 5 Target Scan-only Cancel
// Scope: 기존 Knowledge/Contact 계약을 유지하면서 Target Scan Attempt만 종료하는 explicit cancel command를 추가합니다.
// Changelog:
// - v1.15.1: CancelTargetScan을 추가해 현재 Scan Attempt actor/contact/progress만 정리하고 broad Active Detection Pulse state/remaining time은 보존.
// - v1.15.0: UCFVehicleSensorComp private TargetEntityId keyed Persistent Knowledge Store, Identified/DetailedScan 즉시 upsert, exact AnalysisCompletionRevision 복원, Contact Removed 이후 reassociation, authoritative Terminal Record와 concrete stable identity conflict fail-closed를 구현. Dynamic Knowledge/Freshness/Rescan은 미구현 유지.
// - v1.14.0: 기존 Sensor TargetSelectable resolver 패턴에 GetTargetEntityId를 추가. 실제 Blueprint override는 Execute 경로를 유지하고 Native C++ 구현은 _Implementation을 직접 호출해 기본 Invalid Guid로 잘못 떨어지는 dispatch를 방지.
// - v1.13.0: TargetEntityId를 Contact 최초 생성 때만 캡처하고 같은 Contact 재관측 중 다른 유효 Entity ID가 관측되면 갱신하지 않고 fail-closed. Identity 미지원 Invalid Guid는 기존 Detection/Selection과 호환.
// - v1.12.0: StartActiveDetectionPulse/StartTargetScan/CancelSensorOperations를 canonical command로 전환하고 기존 ActiveScan 계열 함수를 wrapper로 축소. 전체 Sensor update를 RunSensorUpdate로 명확화하고 Knowledge 승격 helper를 Scan progress 의미로 정리. Contact Legacy AnalysisProgress 제거와 동기화.
// - v1.11.0: Target Scan이 bActiveScanRunning을 사용하지 않도록 독립 duration/state를 추가하고, broad Active Detection과 Target Scan의 동시 실행·독립 만료·단일 Target active-range 관측을 구현.
// - v1.10.0: StartTargetedScan 단일 Target 분석, 독립 Scan Attempt progress/decay, exact-once 완료 전이와 완료 즉시 Scanner Idle 복귀를 구현.
// - v1.9.0: DetailedScan 최초 도달 시 AnalysisCompletionRevision을 정확히 한 번 발급하고 raw progress decay와 독립적으로 보존.
// - v1.8.0: Scanner override > VehicleBase > Fallback source resolver와 ApplyVehicleBaseSensorData를 추가하고 Scanner 제거 시 차량 기본 Sensor로 복귀하도록 수정.
// - v1.7.0: 전체 SensorData 계약 검증을 Runtime Source 선택에 적용하고 RadarDisplayRangePresetsCm/Default index를 Applied copy로 고정·공개하는 read-only Getter를 추가.
// - v1.6.0: ApplySensorData, 적용 Config 사본, 탐지 성능 감소 lifecycle reconcile과 Active Scan remaining clamp를 구현.
// - v1.5.0: 외부 adapter가 선택 Actor를 Snapshot ContactId에만 연결할 수 있는 read-only Actor→ContactId bridge를 추가.
// - v1.4.0: VehicleHealthComp OnVehicleDestroyed/IsDestroyed 기반 DestroyedHold, 독립 보존 타이머, 이벤트 binding 정리와 weak-invalid 비파괴 계약을 구현.
// - v1.3.0: 입력과 분리된 Active Scan 시작/중단, Active range 장거리 전방향 Detection, Analysis gain/decay와 Identified/DetailedScan Knowledge 승격을 구현.
// - v1.2.0: 매 Sensor update Contact freshness 갱신, Live→LastKnown→Lost→Removed, Lost 1회 게시와 Lost 전 재획득 보존을 구현.
// - v1.1.0: MaxActorScansPerUpdate 상한의 공정한 Level Actor cursor, self 제외, Passive range, ECC_Visibility visual fallback과 Actor 중복 방지를 구현.
// - v1.0.0: SEN-P0-01 Sensor Component Foundation을 최초 구현.
// Migration:
// - v1.15.1부터 Target Scan 전용 cancel은 CancelTargetScan을 사용하며 broad CancelSensorOperations와 의미를 분리합니다. CancelTargetScan은 bActiveScanRunning/ActiveScanRemainingSeconds를 변경하지 않습니다.
// - v1.15.0부터 valid TargetEntityId의 Identified/DetailedScan/Terminal Knowledge는 Contact array와 독립된 private actor-free Store가 보존합니다. Contact Removed 뒤 동일 Entity 재발견 시 기존 Knowledge를 복원하되 Scan Attempt/CompletionTransition은 복원하지 않습니다.
// - v1.15.0 ResetSensorRuntime/InitializeSensorRuntime 재호출/EndPlay은 Store를 clear하고 ApplySensorData/ApplyVehicleBaseSensorData hot reapply는 Store를 보존합니다. Identity 미지원 Invalid Guid 대상은 기존 Scan 동작을 유지하지만 Store에는 기록하지 않습니다.
// - v1.15.0 same Entity의 양쪽 concrete Source TargetId/TargetCategory mismatch 또는 Terminal Entity 재등장은 새 Contact admission 전에 fail-closed합니다. Dynamic Knowledge/Freshness/Rescan은 Phase 3B-2 전까지 추가하지 않습니다.
// - v1.14.0부터 Sensor의 TargetEntityId 조회는 기존 TargetSelectable 해석 정책과 동일하게 실제 Blueprint override를 우선하고, 그렇지 않은 Native C++ 구현은 virtual _Implementation을 직접 호출합니다. 기존 Blueprint 계약과 Identity 미지원 Invalid Guid 의미는 유지됩니다.
// - v1.12.0부터 새 제품 코드는 StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations를 사용합니다. StartActiveScan / StartTargetedScan / StopActiveScan은 기존 Blueprint/C++ caller 보존용 wrapper입니다.
// - v1.12.0부터 전체 Sensor update canonical 이름은 RunSensorUpdate이며 RunPassiveDetectionUpdate는 기존 Automation/private caller 호환 wrapper입니다.
// - v1.12.0부터 Contact에는 AnalysisProgress01이 없습니다. 현재 한 번의 Scan 진행률은 CurrentScanAttemptProgress01과 공개 Snapshot.ScanAttempt.Progress01만 소유합니다.
// - v1.11.0부터 bActiveScanRunning/ActiveScanRemainingSeconds는 broad Active Detection Pulse 전용입니다. Target Scan은 bCurrentScanAttemptActive/CurrentScanAttemptRemainingSeconds를 독립 Authority로 사용합니다.
// - v1.11.0 Target Scan은 주변 broad Active Detection을 암묵적으로 켜지 않고 지정 Target 하나만 ActiveScanRange 안에서 관측·분석합니다. 두 Operation은 동시에 실행 가능하며 한쪽 만료/완료가 다른 쪽을 종료하지 않습니다.
// - v1.10.0 StartActiveScan은 전방향 장거리 Contact Detection pulse만 제공하고 Contact Knowledge를 자동 분석하지 않습니다. 지정 분석은 StartTargetedScan(TargetActor)만 수행합니다.
// - v1.10.0 DetailedScan 도달 순간 Contact Knowledge와 최초 획득 Revision을 보존한 뒤 Scanner Runtime은 즉시 Idle, Current Scan Attempt는 0/inactive로 복귀합니다.
// - v1.10.0 이미 DetailedScan인 같은 Contact는 기본적으로 StartTargetedScan을 거부하며 향후 동적 재분석은 별도 Rescan 계약으로 분리합니다.
// - Sensor는 InputAction을 직접 소유하지 않고 명시 command API만 제공합니다. 실제 입력 owner가 선택 Target을 전달합니다.
// - ActiveScanRangeCm은 실행 중 Contact Detection에 사용하며 지정 Scan Attempt gain에는 Active range와 ECC_Visibility 직접 가시가 모두 필요합니다.
// - Scan Attempt 분석이 비유효하면 진행률은 AnalysisDecayPerSec로 서서히 감소하며 획득한 InformationLevel/Known Target Knowledge는 강등하지 않습니다.
// - Identified 이상 승격 때만 private Source Metadata의 TargetId/DisplayName을 공개하고 Source InformationLevel은 Sensor Knowledge로 복사하지 않습니다.
// - P0-03 ContactId/lifecycle/reacquire와 P0-04 Active Scan/Analysis 의미를 보존합니다.
// - 파괴 확정은 VehicleHealthComp의 OnVehicleDestroyed 이벤트와 IsDestroyed 상태만 사용하며 Actor Destroy/weak invalid를 파괴로 추정하지 않습니다.
// - DestroyedHold는 마지막 신뢰 위치, ContactId와 획득 Knowledge를 유지합니다.
// - v1.5.0의 Actor→ContactId bridge는 Snapshot의 Player-facing data를 우회하지 않으며 TargetSelect 선택 의미를 변경하지 않습니다.
// - v1.6.0부터 Runtime Ready 상태는 AppliedSensorConfig 사본을 소비하며 Source UObject의 후속 값 변경만으로 Runtime 의미가 바뀌지 않습니다.
// - ApplySensorData는 invalid explicit Scanner Source를 원자적으로 거부하고, null은 VehicleBaseSensorData가 유효하면 차량 기본 Sensor로, 없으면 검증된 Fallback Source로 적용합니다.
// - 탐지 능력 감소는 Contact를 삭제하지 않고 LastKnown으로 넘기며 Active Scan은 새 장비 적용으로 남은 시간이 늘어나지 않습니다.
// - v1.7.0 Radar Range Profile은 탐지 성능을 변경하지 않는 UI 표시 Source이며 Runtime Ready 상태에서는 Applied copy만 공개합니다. 기존 Scanner-less Fallback은 빈 Profile을 유지합니다.
// - v1.8.0 VehicleBaseSensorData는 VehicleData의 기본 Sensor Source이며 장착 Scanner SensorData가 항상 우선합니다. 둘 다 없을 때만 기존 zero-range Fallback을 사용합니다.

#include "CFVehicleSensorComp.h"

#include "CFTargetSelectable.h"
#include "CFVehicleHealthComp.h"
#include "CFVehicleSensorData.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	// [v1.1.0] BlueprintNativeEvent의 실제 Blueprint 재정의를 구분할 IsTargetSelectable 함수 이름입니다.
	const FName SensorIsTargetSelectableFunctionName(TEXT("IsTargetSelectable"));

	// [v1.1.0] BlueprintNativeEvent의 실제 Blueprint 재정의를 구분할 GetTargetDisplayInfo 함수 이름입니다.
	const FName SensorGetTargetDisplayInfoFunctionName(TEXT("GetTargetDisplayInfo"));

	// [v1.14.0] BlueprintNativeEvent의 실제 Blueprint 재정의를 구분할 GetTargetEntityId 함수 이름입니다.
	const FName SensorGetTargetEntityIdFunctionName(TEXT("GetTargetEntityId"));

	// [v1.1.0] BlueprintNativeEvent의 실제 Blueprint 재정의를 구분할 GetTargetSelectionLocation 함수 이름입니다.
	const FName SensorGetTargetLocationFunctionName(TEXT("GetTargetSelectionLocation"));

	// [v1.1.0] Sensor가 TargetSelectable 자격을 조회할 때 사용하는 컨텍스트 ID입니다.
	const FName SensorPassiveContextId(TEXT("SensorPassive"));

	// [v1.1.0] 월드 위치 벡터의 모든 축이 유한한 값인지 반환합니다.
	bool IsFiniteSensorVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	// [v1.1.0] TargetSelectable BlueprintNativeEvent가 실제 Blueprint에서 재정의됐는지 반환합니다.
	bool HasSensorBlueprintOverride(const AActor* TargetActor, const FName FunctionName)
	{
		if (!IsValid(TargetActor))
		{
			return false;
		}

		// [v1.1.0] Actor class에서 검색된 대상 함수 Reflection 정보입니다.
		const UFunction* TargetFunction = TargetActor->FindFunction(FunctionName);
		return IsValid(TargetFunction)
			&& TargetFunction->GetOuter() != UCFTargetSelectable::StaticClass();
	}

	// [v1.1.0] Native C++과 실제 Blueprint override를 구분해 IsTargetSelectable을 안전하게 호출합니다.
	bool ResolveSensorTargetSelectable(const AActor* TargetActor, const FCFTargetSelectionContext& SelectionContext)
	{
		if (HasSensorBlueprintOverride(TargetActor, SensorIsTargetSelectableFunctionName))
		{
			return ICFTargetSelectable::Execute_IsTargetSelectable(TargetActor, SelectionContext);
		}

		// [v1.1.0] C++ Native TargetSelectable 구현이 있으면 생성 Execute 우회 대신 직접 기본/override 구현을 호출합니다.
		const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor);
		if (NativeTargetSelectable)
		{
			return NativeTargetSelectable->IsTargetSelectable_Implementation(SelectionContext);
		}

		return ICFTargetSelectable::Execute_IsTargetSelectable(TargetActor, SelectionContext);
	}

	// [v1.1.0] Native C++과 실제 Blueprint override를 구분해 Source DisplayInfo를 안전하게 호출합니다.
	FCFTargetDisplayInfo ResolveSensorDisplayInfo(const AActor* TargetActor)
	{
		if (HasSensorBlueprintOverride(TargetActor, SensorGetTargetDisplayInfoFunctionName))
		{
			return ICFTargetSelectable::Execute_GetTargetDisplayInfo(TargetActor);
		}

		// [v1.1.0] C++ Native TargetSelectable 구현의 Source Metadata를 직접 읽을 인터페이스입니다.
		const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor);
		if (NativeTargetSelectable)
		{
			return NativeTargetSelectable->GetTargetDisplayInfo_Implementation();
		}

		return ICFTargetSelectable::Execute_GetTargetDisplayInfo(TargetActor);
	}

	// [v1.14.0] Native C++과 실제 Blueprint override를 구분해 Gameplay Entity ID를 안전하게 호출합니다.
	FGuid ResolveSensorTargetEntityId(const AActor* TargetActor)
	{
		if (!IsValid(TargetActor))
		{
			return FGuid();
		}

		if (HasSensorBlueprintOverride(TargetActor, SensorGetTargetEntityIdFunctionName))
		{
			return ICFTargetSelectable::Execute_GetTargetEntityId(TargetActor);
		}

		// [v1.14.0] Native C++ TargetSelectable 구현의 virtual _Implementation을 직접 호출해 개체별 ID를 보존합니다.
		const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor);
		if (NativeTargetSelectable)
		{
			return NativeTargetSelectable->GetTargetEntityId_Implementation();
		}

		return ICFTargetSelectable::Execute_GetTargetEntityId(TargetActor);
	}

	// [v1.1.0] Native C++과 실제 Blueprint override를 구분해 대표 선택 위치를 안전하게 호출합니다.
	FVector ResolveSensorSelectionLocation(const AActor* TargetActor)
	{
		if (HasSensorBlueprintOverride(TargetActor, SensorGetTargetLocationFunctionName))
		{
			return ICFTargetSelectable::Execute_GetTargetSelectionLocation(TargetActor);
		}

		// [v1.1.0] C++ Native TargetSelectable 구현의 대표 위치를 직접 읽을 인터페이스입니다.
		const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(TargetActor);
		if (NativeTargetSelectable)
		{
			return NativeTargetSelectable->GetTargetSelectionLocation_Implementation();
		}

		return ICFTargetSelectable::Execute_GetTargetSelectionLocation(TargetActor);
	}
}

// [v1.1.0] bounded Passive Detection Tick은 Config에 실제 탐지 거리가 있을 때만 활성화됩니다.
UCFVehicleSensorComp::UCFVehicleSensorComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(false);
}

// [v1.3.0] UpdateIntervalSec마다 Contact 수명, bounded Detection과 Tactical Analysis를 한 번 갱신합니다.
void UCFVehicleSensorComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bSensorRuntimeReady)
	{
		return;
	}

	// [v1.3.0] 현재 Tick에서 사용할 유효 Sensor 설정 사본입니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	if (!SensorConfig.IsValid()
		|| (!HasConfiguredDetectionWork(SensorConfig)
			&& !bCurrentScanAttemptActive
			&& RuntimeContacts.IsEmpty()))
	{
		return;
	}

	// [v1.1.0] 비유한 값과 음수 DeltaTime을 제거한 실제 누적 시간입니다.
	const float SafeDeltaTime = FMath::IsFinite(DeltaTime)
		? FMath::Max(DeltaTime, 0.0f)
		: 0.0f;
	PassiveUpdateElapsedSeconds += SafeDeltaTime;
	if (PassiveUpdateElapsedSeconds < SensorConfig.UpdateIntervalSec)
	{
		return;
	}

	// [v1.3.0] hitch catch-up burst는 만들지 않되 이번 한 번의 Sensor update가 실제로 대표할 누적 시간입니다.
	const float SensorUpdateDeltaSeconds = PassiveUpdateElapsedSeconds;
	PassiveUpdateElapsedSeconds = 0.0f;
	RunSensorUpdate(SensorUpdateDeltaSeconds);
}

// [v1.0.0] Actor 수명이 종료될 때 Sensor Snapshot과 준비 상태를 안전하게 비웁니다.
void UCFVehicleSensorComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetSensorRuntime();
	Super::EndPlay(EndPlayReason);
}

// [v1.8.0] Scanner override > VehicleBase > Fallback 우선순위의 유효 Config로 Runtime을 초기화합니다.
bool UCFVehicleSensorComp::InitializeSensorRuntime()
{
	// [v1.8.0] 초기화 시점에 실제로 선택된 Scanner 또는 VehicleBase SensorData Source입니다. Fallback이면 nullptr입니다.
	const UCFVehicleSensorData* EffectiveSensorData = ResolveConfiguredSensorData();

	// [v1.8.0] 선택된 Source 또는 zero-range Fallback에서 초기화 시점에 고정할 실제 Runtime 설정 사본입니다.
	const FCFSensorConfig ResolvedConfig = ResolveConfiguredSensorConfig();

	UnbindAllContactDestroyedEvents();
	RuntimeContacts.Reset();
	// [v1.15.0] InitializeSensorRuntime 재호출은 Hard Reinitialize이므로 Contact lifetime 독립 Store도 함께 비웁니다.
	PersistentKnowledgeStore.Reset();
	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	ResetCurrentScanAttempt();
	LastCompletedScanContactId = NAME_None;
	LastScanCompletionTransitionRevision = 0;
	PassiveUpdateElapsedSeconds = 0.0f;
	ResetPassiveScanCursor();

	if (!ResolvedConfig.IsValid())
	{
		bHasAppliedSensorConfig = false;
		AppliedSensorConfig = FCFSensorConfig();
		AppliedRadarDisplayRangePresetsCm.Reset();
		AppliedDefaultRadarDisplayRangePresetIndex = INDEX_NONE;
		bSensorRuntimeReady = false;
		SetComponentTickEnabled(false);
		RebuildFoundationSnapshot(false);
		LastSensorRuntimeSummary = TEXT("SensorRuntime: InvalidConfig");
		return false;
	}

	AppliedSensorConfig = ResolvedConfig;
	AppliedRadarDisplayRangePresetsCm = EffectiveSensorData
		? EffectiveSensorData->RadarDisplayRangePresetsCm
		: TArray<float>();
	AppliedDefaultRadarDisplayRangePresetIndex = EffectiveSensorData
		? EffectiveSensorData->DefaultRadarDisplayRangePresetIndex
		: INDEX_NONE;
	bHasAppliedSensorConfig = true;
	bSensorRuntimeReady = true;
	RebuildFoundationSnapshot(true);

	// [v1.3.0] Active Scan 요청이 없어도 상시 수행해야 하는 Passive/Visual Detection 작업 여부입니다.
	const bool bPassiveUpdatesEnabled = HasConfiguredPassiveWork(ResolvedConfig);
	SetComponentTickEnabled(bPassiveUpdatesEnabled);

	// [v1.8.0] 실제 설정이 Scanner override, VehicleBase 또는 Fallback 중 어디에서 왔는지 표시할 출처 문자열입니다.
	const TCHAR* ConfigSource = IsValid(SensorData) && SensorData->IsSensorDataContractValid()
		? TEXT("ScannerOverride")
		: (EffectiveSensorData != nullptr && EffectiveSensorData == VehicleBaseSensorData.Get()
			? TEXT("VehicleBase")
			: TEXT("Fallback"));
	LastSensorRuntimeSummary = FString::Printf(
		TEXT("SensorRuntime: Ready Source=%s PassiveUpdates=%s ActiveReady=%s Revision=%d | %s"),
		ConfigSource,
		bPassiveUpdatesEnabled ? TEXT("True") : TEXT("False"),
		(ResolvedConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER && ResolvedConfig.ActiveScanDurationSec > KINDA_SMALL_NUMBER) ? TEXT("True") : TEXT("False"),
		CurrentSensorSnapshot.Revision,
		*ResolvedConfig.BuildDebugSummary());
	return true;
}

// [v1.8.0] 장착 Scanner SensorData를 적용하거나 None이면 VehicleBase/Fallback Source로 복귀합니다.
bool UCFVehicleSensorComp::ApplySensorData(UCFVehicleSensorData* NewSensorData)
{
	if (NewSensorData != nullptr
		&& (!IsValid(NewSensorData) || !NewSensorData->IsSensorDataContractValid()))
	{
		return false;
	}

	// [v1.8.0] Scanner가 없을 때 사용할 유효 VehicleBase SensorData입니다. 없거나 계약이 무효하면 nullptr입니다.
	const UCFVehicleSensorData* ValidVehicleBaseSensorData = IsValid(VehicleBaseSensorData)
		&& VehicleBaseSensorData->IsSensorDataContractValid()
		? VehicleBaseSensorData.Get()
		: nullptr;

	// [v1.8.0] 이번 요청에서 실제로 적용될 Scanner override 또는 VehicleBase SensorData입니다. 둘 다 없으면 nullptr입니다.
	const UCFVehicleSensorData* RequestedEffectiveSensorData = IsValid(NewSensorData)
		? NewSensorData
		: ValidVehicleBaseSensorData;

	// [v1.8.0] 실제 선택 Source가 없을 때만 기존 zero-range Fallback을 사용하는 적용 후보 Config입니다.
	const FCFSensorConfig RequestedSensorConfig = RequestedEffectiveSensorData
		? RequestedEffectiveSensorData->SensorConfig
		: FallbackSensorConfig;

	// [v1.8.0] 실제 선택 Source가 명시한 Radar 표시 Range Preset 적용 후보 사본입니다.
	const TArray<float> RequestedRadarDisplayRangePresetsCm = RequestedEffectiveSensorData
		? RequestedEffectiveSensorData->RadarDisplayRangePresetsCm
		: TArray<float>();

	// [v1.8.0] 실제 선택 Source가 명시한 Radar 초기 표시 Range Preset 인덱스입니다.
	const int32 RequestedDefaultRadarDisplayRangePresetIndex = RequestedEffectiveSensorData
		? RequestedEffectiveSensorData->DefaultRadarDisplayRangePresetIndex
		: INDEX_NONE;
	if (!RequestedSensorConfig.IsValid())
	{
		return false;
	}

	// [v1.8.0] 초기화 전 Source 선택은 Runtime을 암묵적으로 시작하지 않고 다음 InitializeSensorRuntime 입력만 교체합니다.
	if (!bSensorRuntimeReady)
	{
		SensorData = NewSensorData;
		bHasAppliedSensorConfig = false;
		AppliedSensorConfig = FCFSensorConfig();
		AppliedRadarDisplayRangePresetsCm.Reset();
		AppliedDefaultRadarDisplayRangePresetIndex = INDEX_NONE;

		// [v1.8.0] 초기화 전 실제 적용 후보 Source를 구분하는 진단 문자열입니다.
		const TCHAR* ConfigSource = IsValid(NewSensorData)
			? TEXT("ScannerOverride")
			: (RequestedEffectiveSensorData == VehicleBaseSensorData.Get() && RequestedEffectiveSensorData != nullptr
				? TEXT("VehicleBase")
				: TEXT("Fallback"));
		LastSensorRuntimeSummary = FString::Printf(
			TEXT("SensorRuntime: Configured Source=%s RuntimeReady=False | %s"),
			ConfigSource,
			*RequestedSensorConfig.BuildDebugSummary());
		return true;
	}

	// [v1.6.0] hot reapply 전 실제 Runtime이 소비하던 적용 Config 사본입니다.
	const FCFSensorConfig PreviousSensorConfig = GetResolvedSensorConfig();

	// [v1.6.0] Passive 또는 Visual 관측 능력이 이전 적용값보다 줄어 기존 Live를 즉시 신뢰할 수 없는지 여부입니다.
	const bool bBaselineDetectionReduced = RequestedSensorConfig.PassiveDetectionRangeCm + KINDA_SMALL_NUMBER < PreviousSensorConfig.PassiveDetectionRangeCm
		|| RequestedSensorConfig.VisualDetectionRangeCm + KINDA_SMALL_NUMBER < PreviousSensorConfig.VisualDetectionRangeCm;

	// [v1.6.0] 실행 중 Active-only Contact를 이전 범위 그대로 신뢰할 수 없게 만드는 Active 거리 감소 여부입니다.
	const bool bActiveDetectionReduced = RequestedSensorConfig.ActiveScanRangeCm + KINDA_SMALL_NUMBER < PreviousSensorConfig.ActiveScanRangeCm;

	SensorData = NewSensorData;
	AppliedSensorConfig = RequestedSensorConfig;
	AppliedRadarDisplayRangePresetsCm = RequestedRadarDisplayRangePresetsCm;
	AppliedDefaultRadarDisplayRangePresetIndex = RequestedDefaultRadarDisplayRangePresetIndex;
	bHasAppliedSensorConfig = true;
	PassiveUpdateElapsedSeconds = 0.0f;

	if (bBaselineDetectionReduced)
	{
		MarkAllLiveContactsLastKnown();
	}
	else if (bActiveDetectionReduced)
	{
		MarkActiveOnlyContactsLastKnown();
	}

	// [v1.11.0] Scanner/기본 Sensor Source가 바뀌는 동안 진행 중 Target Scan Attempt는 다른 장비 의미로 이어받지 않되 broad Active Detection state는 독립적으로 보존합니다.
	if (bCurrentScanAttemptActive)
	{
		MarkCurrentScanTargetLastKnownIfNeeded();
		ResetCurrentScanAttempt();
	}

	if (bActiveScanRunning)
	{
		// [v1.11.0] 새 Config가 broad Active Detection Pulse를 계속 실행할 최소 거리와 지속 시간을 모두 제공하는지 여부입니다.
		const bool bActiveScanSupported = RequestedSensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER
			&& RequestedSensorConfig.ActiveScanDurationSec > KINDA_SMALL_NUMBER;
		if (!bActiveScanSupported)
		{
			bActiveScanRunning = false;
			ActiveScanRemainingSeconds = 0.0f;
			MarkActiveOnlyContactsLastKnown();
		}
		else
		{
			ActiveScanRemainingSeconds = FMath::Min(
				FMath::Max(ActiveScanRemainingSeconds, 0.0f),
				RequestedSensorConfig.ActiveScanDurationSec);
		}
	}

	SetComponentTickEnabled(
		HasConfiguredPassiveWork(RequestedSensorConfig)
		|| bActiveScanRunning
		|| bCurrentScanAttemptActive
		|| !RuntimeContacts.IsEmpty());
	PublishRuntimeSnapshot();

	// [v1.8.0] hot reapply 결과의 실제 Source를 구분하는 진단 문자열입니다.
	const TCHAR* ConfigSource = IsValid(NewSensorData)
		? TEXT("ScannerOverride")
		: (RequestedEffectiveSensorData == VehicleBaseSensorData.Get() && RequestedEffectiveSensorData != nullptr
			? TEXT("VehicleBase")
			: TEXT("Fallback"));
	LastSensorRuntimeSummary = FString::Printf(
		TEXT("SensorRuntime: Applied Source=%s Contacts=%d Active=%s Remaining=%.3f Revision=%d | %s"),
		ConfigSource,
		RuntimeContacts.Num(),
		bActiveScanRunning ? TEXT("True") : TEXT("False"),
		ActiveScanRemainingSeconds,
		CurrentSensorSnapshot.Revision,
		*RequestedSensorConfig.BuildDebugSummary());
	return true;
}

// [v1.8.0] VehicleData 기본 Sensor Source를 교체하고 Scanner override가 없으면 같은 non-destructive 적용 경로를 실행합니다.
bool UCFVehicleSensorComp::ApplyVehicleBaseSensorData(UCFVehicleSensorData* NewVehicleBaseSensorData)
{
	if (NewVehicleBaseSensorData != nullptr
		&& (!IsValid(NewVehicleBaseSensorData) || !NewVehicleBaseSensorData->IsSensorDataContractValid()))
	{
		return false;
	}

	// [v1.8.0] 실패 시 원자적으로 되돌릴 기존 차량 기본 Sensor Source입니다.
	UCFVehicleSensorData* PreviousVehicleBaseSensorData = VehicleBaseSensorData.Get();
	VehicleBaseSensorData = NewVehicleBaseSensorData;

	// [v1.8.0] 유효한 장착 Scanner override가 있으면 실제 Runtime Source가 변하지 않으므로 기본 Source만 저장합니다.
	const bool bHasValidScannerOverride = IsValid(SensorData) && SensorData->IsSensorDataContractValid();
	if (bHasValidScannerOverride)
	{
		return true;
	}

	if (ApplySensorData(nullptr))
	{
		return true;
	}

	VehicleBaseSensorData = PreviousVehicleBaseSensorData;
	return false;
}

// [v1.3.0] Runtime Contact, Active Scan, scan cursor와 공개 Snapshot을 초기화하고 Tick을 비활성화합니다.
void UCFVehicleSensorComp::ResetSensorRuntime()
{
	SetComponentTickEnabled(false);
	bSensorRuntimeReady = false;
	bHasAppliedSensorConfig = false;
	AppliedSensorConfig = FCFSensorConfig();
	AppliedRadarDisplayRangePresetsCm.Reset();
	AppliedDefaultRadarDisplayRangePresetIndex = INDEX_NONE;
	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	ResetCurrentScanAttempt();
	LastCompletedScanContactId = NAME_None;
	LastScanCompletionTransitionRevision = 0;
	UnbindAllContactDestroyedEvents();
	RuntimeContacts.Reset();
	// [v1.15.0] 명시 Runtime Reset은 같은 World라도 이전 Entity Knowledge를 유지하지 않는 Hard Reset입니다.
	PersistentKnowledgeStore.Reset();
	PassiveUpdateElapsedSeconds = 0.0f;
	ResetPassiveScanCursor();
	RebuildFoundationSnapshot(false);
	LastSensorRuntimeSummary = FString::Printf(
		TEXT("SensorRuntime: Reset Revision=%d"),
		CurrentSensorSnapshot.Revision);
}

// [v1.12.0] 주변 Contact 탐지 강화를 위한 broad Active Detection Pulse를 시작하고 Target Scan 상태는 변경하지 않습니다.
bool UCFVehicleSensorComp::StartActiveDetectionPulse()
{
	if (!bSensorRuntimeReady || bActiveScanRunning)
	{
		return false;
	}

	// [v1.10.0] Active Sensor pulse 시작 조건과 실행 시간을 제공할 현재 Sensor 설정입니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	if (!SensorConfig.IsValid()
		|| SensorConfig.ActiveScanRangeCm <= KINDA_SMALL_NUMBER
		|| SensorConfig.ActiveScanDurationSec <= KINDA_SMALL_NUMBER
		|| !GetWorld()
		|| !IsValid(GetOwner()))
	{
		return false;
	}

	bActiveScanRunning = true;
	ActiveScanRemainingSeconds = SensorConfig.ActiveScanDurationSec;
	PassiveUpdateElapsedSeconds = 0.0f;
	SetComponentTickEnabled(true);
	PublishRuntimeSnapshot();
	return true;
}

// [v1.12.0] 기존 Active Scan 시작 caller를 canonical Active Detection Pulse로 전달하는 compatibility wrapper입니다.
bool UCFVehicleSensorComp::StartActiveScan()
{
	return StartActiveDetectionPulse();
}

// [v1.12.0] 지정 Actor 하나를 broad Active Detection과 독립된 Target Scan Attempt 대상으로 시작하고 이미 DetailedScan 완료된 대상은 기본 정책상 거부합니다.
bool UCFVehicleSensorComp::StartTargetScan(AActor* TargetActor)
{
	if (!bSensorRuntimeReady
		|| bCurrentScanAttemptActive
		|| !IsValid(TargetActor)
		|| !GetWorld()
		|| !IsValid(GetOwner())
		|| !IsSensorCandidateEligible(TargetActor))
	{
		return false;
	}

	// [v1.10.0] 지정 Scan Attempt의 거리·시간·진행률 정책을 제공할 현재 적용 Sensor 설정입니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	if (!SensorConfig.IsValid()
		|| SensorConfig.ActiveScanRangeCm <= KINDA_SMALL_NUMBER
		|| SensorConfig.ActiveScanDurationSec <= KINDA_SMALL_NUMBER
		|| SensorConfig.AnalysisGainPerSec <= 0.0f)
	{
		return false;
	}

	// [v1.10.0] 시작 시 Active Scan 범위 확인에 사용할 지정 Target의 현재 대표 위치입니다.
	const FVector TargetWorldLocation = ResolveSensorTargetLocation(TargetActor);
	if (!IsFiniteSensorVector(TargetWorldLocation))
	{
		return false;
	}

	// [v1.10.0] 시작 시점에 지정 Target이 현재 Scanner Active range 안에 있는지 확인할 실제 거리입니다.
	const float DistanceToTargetCm = FVector::Distance(GetOwner()->GetActorLocation(), TargetWorldLocation);
	if (!FMath::IsFinite(DistanceToTargetCm) || DistanceToTargetCm > SensorConfig.ActiveScanRangeCm)
	{
		return false;
	}

	// [v1.10.0] 이미 존재하는 Contact가 DetailedScan Knowledge를 획득했는지 확인할 현재 인덱스입니다.
	const int32 ExistingContactIndex = FindRuntimeContactIndexByActor(TargetActor);
	if (RuntimeContacts.IsValidIndex(ExistingContactIndex)
		&& RuntimeContacts[ExistingContactIndex].PublicContact.InformationLevel == ECFTargetInfoLevel::DetailedScan)
	{
		return false;
	}

	// [v1.11.0] broad Active Detection을 켜지 않고 지정 Target 하나에만 ActiveScanRange 관측을 허용해 Contact를 확보합니다.
	if (!ProcessPassiveScanActor(TargetActor, SensorConfig, true))
	{
		SetComponentTickEnabled(
			HasConfiguredPassiveWork(SensorConfig)
			|| bActiveScanRunning
			|| !RuntimeContacts.IsEmpty());
		PublishRuntimeSnapshot();
		return false;
	}

	// [v1.11.0] 지정 Target 단일 Active-range 관측으로 확보한 Runtime Contact 인덱스입니다.
	const int32 TargetContactIndex = FindRuntimeContactIndexByActor(TargetActor);
	if (!RuntimeContacts.IsValidIndex(TargetContactIndex)
		|| RuntimeContacts[TargetContactIndex].PublicContact.ContactState != ECFSensorContactState::Live
		|| RuntimeContacts[TargetContactIndex].PublicContact.InformationLevel == ECFTargetInfoLevel::DetailedScan)
	{
		SetComponentTickEnabled(
			HasConfiguredPassiveWork(SensorConfig)
			|| bActiveScanRunning
			|| !RuntimeContacts.IsEmpty());
		PublishRuntimeSnapshot();
		return false;
	}

	bCurrentScanAttemptActive = true;
	CurrentScanAttemptRemainingSeconds = SensorConfig.ActiveScanDurationSec;
	CurrentScanTargetActor = TargetActor;
	CurrentScanTargetContactId = RuntimeContacts[TargetContactIndex].PublicContact.ContactId;
	CurrentScanAttemptProgress01 = 0.0f;
	PassiveUpdateElapsedSeconds = 0.0f;
	SetComponentTickEnabled(true);
	PublishRuntimeSnapshot();
	return true;
}

// [v1.12.0] 기존 Targeted Scan 시작 caller를 canonical Target Scan command로 전달하는 compatibility wrapper입니다.
bool UCFVehicleSensorComp::StartTargetedScan(AActor* TargetActor)
{
	return StartTargetScan(TargetActor);
}

// [v1.15.1] 현재 Target Scan Attempt만 명시 취소하고 broad Active Detection Pulse와 획득 Knowledge는 그대로 유지합니다.
bool UCFVehicleSensorComp::CancelTargetScan()
{
	if (!bCurrentScanAttemptActive)
	{
		return false;
	}

	MarkCurrentScanTargetLastKnownIfNeeded();
	ResetCurrentScanAttempt();

	// [v1.15.1] Target Scan 취소 뒤 broad Detection 또는 Passive/Contact lifetime 작업이 필요하면 Tick을 계속 유지합니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	SetComponentTickEnabled(
		SensorConfig.IsValid()
		&& (HasConfiguredPassiveWork(SensorConfig) || bActiveScanRunning || !RuntimeContacts.IsEmpty()));
	PublishRuntimeSnapshot();
	return true;
}

// [v1.12.0] 실행 중인 broad Active Detection Pulse와 Target Scan Attempt를 모두 명시 취소하고 Contact Knowledge는 유지합니다.
bool UCFVehicleSensorComp::CancelSensorOperations()
{
	if (!bActiveScanRunning && !bCurrentScanAttemptActive)
	{
		return false;
	}

	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	PassiveUpdateElapsedSeconds = 0.0f;
	ResetCurrentScanAttempt();
	MarkActiveOnlyContactsLastKnown();

	// [v1.11.0] 두 Operation 명시 취소 뒤에도 Passive/Visual Detection 또는 남은 Contact lifetime 작업이 필요한지 여부입니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	SetComponentTickEnabled(
		SensorConfig.IsValid()
		&& (HasConfiguredPassiveWork(SensorConfig) || !RuntimeContacts.IsEmpty()));
	PublishRuntimeSnapshot();
	return true;
}

// [v1.12.0] 기존 Active Scan 중단 caller를 canonical Sensor Operation cancel command로 전달하는 compatibility wrapper입니다.
bool UCFVehicleSensorComp::StopActiveScan()
{
	return CancelSensorOperations();
}

// [v1.6.0] Runtime Ready이면 실제 적용된 Sensor Config를, 초기화 전이면 현재 Source 또는 Fallback 설정을 반환합니다.
FCFSensorConfig UCFVehicleSensorComp::GetResolvedSensorConfig() const
{
	return bHasAppliedSensorConfig
		? AppliedSensorConfig
		: ResolveConfiguredSensorConfig();
}

// [v1.8.0] Runtime에 실제 적용됐거나 초기화 전 유효 Scanner/VehicleBase Source가 제공하는 Radar 표시 Range Preset 사본을 반환합니다.
TArray<float> UCFVehicleSensorComp::GetResolvedRadarDisplayRangePresetsCm() const
{
	if (bHasAppliedSensorConfig)
	{
		return AppliedRadarDisplayRangePresetsCm;
	}

	// [v1.8.0] 초기화 전 Scanner override > VehicleBase 우선순위로 선택된 유효 SensorData Source입니다.
	const UCFVehicleSensorData* EffectiveSensorData = ResolveConfiguredSensorData();
	return EffectiveSensorData
		? EffectiveSensorData->RadarDisplayRangePresetsCm
		: TArray<float>();
}

// [v1.8.0] Runtime에 실제 적용됐거나 초기화 전 유효 Scanner/VehicleBase Source가 제공하는 Radar 기본 표시 Range Preset 인덱스를 반환합니다.
int32 UCFVehicleSensorComp::GetResolvedDefaultRadarDisplayRangePresetIndex() const
{
	if (bHasAppliedSensorConfig)
	{
		return AppliedDefaultRadarDisplayRangePresetIndex;
	}

	// [v1.8.0] 초기화 전 Scanner override > VehicleBase 우선순위로 선택된 유효 SensorData Source입니다.
	const UCFVehicleSensorData* EffectiveSensorData = ResolveConfiguredSensorData();
	return EffectiveSensorData
		? EffectiveSensorData->DefaultRadarDisplayRangePresetIndex
		: INDEX_NONE;
}

// [v1.8.0] Scanner override > VehicleBase 순서로 현재 유효한 SensorData Source를 반환합니다.
const UCFVehicleSensorData* UCFVehicleSensorComp::ResolveConfiguredSensorData() const
{
	if (IsValid(SensorData) && SensorData->IsSensorDataContractValid())
	{
		return SensorData.Get();
	}

	if (IsValid(VehicleBaseSensorData) && VehicleBaseSensorData->IsSensorDataContractValid())
	{
		return VehicleBaseSensorData.Get();
	}

	return nullptr;
}

// [v1.8.0] Scanner override > VehicleBase > Fallback 순서로 실제 Sensor Config를 반환합니다.
FCFSensorConfig UCFVehicleSensorComp::ResolveConfiguredSensorConfig() const
{
	// [v1.8.0] 현재 우선순위에서 선택된 유효 Scanner 또는 VehicleBase SensorData Source입니다.
	const UCFVehicleSensorData* EffectiveSensorData = ResolveConfiguredSensorData();
	return EffectiveSensorData
		? EffectiveSensorData->SensorConfig
		: FallbackSensorConfig;
}

// [v1.0.0] 현재 Actor-free Sensor Snapshot 사본을 반환합니다.
FCFSensorSnapshot UCFVehicleSensorComp::GetSensorSnapshot() const
{
	return CurrentSensorSnapshot;
}

// [v1.5.0] 이미 존재하는 Sensor Contact와 지정 Actor의 연결을 ContactId만으로 읽고 Actor truth나 private Runtime data는 공개하지 않습니다.
bool UCFVehicleSensorComp::TryGetContactIdForActor(const AActor* TargetActor, FName& OutContactId) const
{
	OutContactId = NAME_None;
	if (!IsValid(TargetActor))
	{
		return false;
	}

	// [v1.5.0] 이미 Sensor Runtime에 존재하는 지정 Actor의 Contact 인덱스입니다.
	const int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(TargetActor);
	if (!RuntimeContacts.IsValidIndex(RuntimeContactIndex))
	{
		return false;
	}

	// [v1.5.0] 외부 adapter가 Actor metadata 대신 공개 Snapshot에서 다시 찾을 안정 ContactId입니다.
	const FName ResolvedContactId = RuntimeContacts[RuntimeContactIndex].PublicContact.ContactId;
	if (ResolvedContactId.IsNone())
	{
		return false;
	}

	OutContactId = ResolvedContactId;
	return true;
}

// [v1.0.0] TargetId와 독립된 새 ContactId를 증가형 serial로 발급합니다.
FName UCFVehicleSensorComp::AllocateContactId()
{
	// [v1.0.0] 잘못된 외부 메모리 오염에도 1 이상의 serial로 복구한 현재 발급 번호입니다.
	const int32 ContactSerial = FMath::Max(NextContactSerial, 1);
	NextContactSerial = ContactSerial + 1;
	return FName(*FString::Printf(TEXT("Contact_%06d"), ContactSerial));
}

// [v1.1.0] Passive 또는 Visual 탐지 거리가 실제로 설정돼 기본 bounded update가 필요한지 반환합니다.
bool UCFVehicleSensorComp::HasConfiguredPassiveWork(const FCFSensorConfig& SensorConfig) const
{
	return SensorConfig.PassiveDetectionRangeCm > KINDA_SMALL_NUMBER
		|| SensorConfig.VisualDetectionRangeCm > KINDA_SMALL_NUMBER;
}

// [v1.11.0] broad Active Detection Pulse 실행 상태까지 포함해 이번 update에 bounded World Detection이 필요한지 반환합니다.
bool UCFVehicleSensorComp::HasConfiguredDetectionWork(const FCFSensorConfig& SensorConfig) const
{
	return HasConfiguredPassiveWork(SensorConfig)
		|| (bActiveScanRunning && SensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER);
}

// [v1.12.0] Contact lifetime, bounded Detection, Target Scan progression, Operation duration과 Snapshot을 한 번 갱신합니다.
void UCFVehicleSensorComp::RunSensorUpdate(const float SensorUpdateDeltaSeconds)
{
	LastPassiveScanActorCount = 0;
	LastPassiveVisibilityTraceCount = 0;
	LastActiveAnalysisTraceCount = 0;

	if (!bSensorRuntimeReady)
	{
		return;
	}

	// [v1.3.0] bounded Detection과 현재 World time을 제공할 월드입니다.
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		return;
	}

	// [v1.3.0] 이번 update에서 사용할 검증된 Sensor 설정 사본입니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	if (!SensorConfig.IsValid())
	{
		return;
	}

	// [v1.3.0] Tick에서는 실제 누적 시간을, asset-free Automation 직접 호출에서는 UpdateInterval을 사용하는 결정적 update 시간입니다.
	const float ResolvedSensorUpdateDeltaSeconds = FMath::IsFinite(SensorUpdateDeltaSeconds)
		&& SensorUpdateDeltaSeconds >= 0.0f
		? SensorUpdateDeltaSeconds
		: SensorConfig.UpdateIntervalSec;

	// [v1.11.0] 이번 update 구간 중 broad Active Detection Pulse가 실제로 유효한 시간입니다.
	const float ActiveScanTimeThisUpdate = bActiveScanRunning
		? FMath::Min(FMath::Max(ActiveScanRemainingSeconds, 0.0f), ResolvedSensorUpdateDeltaSeconds)
		: 0.0f;

	// [v1.11.0] broad Active Detection과 독립된 현재 Target Scan Attempt가 이번 update에서 실제로 사용할 수 있는 시간입니다.
	const float TargetScanTimeThisUpdate = bCurrentScanAttemptActive
		? FMath::Min(FMath::Max(CurrentScanAttemptRemainingSeconds, 0.0f), ResolvedSensorUpdateDeltaSeconds)
		: 0.0f;

	// [v1.2.0] bounded Actor cursor와 독립적으로 모든 기존 Contact 수명을 진행할 현재 World 시간입니다.
	const double CurrentWorldTimeSeconds = FMath::Max(
		0.0,
		static_cast<double>(CurrentWorld->GetTimeSeconds()));
	AdvanceContactLifetimes(CurrentWorldTimeSeconds, SensorConfig);

	if (HasConfiguredDetectionWork(SensorConfig))
	{
		// [v1.1.0] 현재 World에 로드된 Level 배열입니다. 각 Level의 Actors 배열을 persistent cursor로 이어서 읽습니다.
		const TArray<ULevel*>& WorldLevels = CurrentWorld->GetLevels();
		if (!WorldLevels.IsEmpty())
		{
			if (PassiveScanLevelIndex < 0 || PassiveScanLevelIndex >= WorldLevels.Num())
			{
				PassiveScanLevelIndex = 0;
				PassiveScanActorIndex = 0;
			}

			// [v1.1.0] 이번 update에서 소비할 수 있는 Actor 슬롯 예산입니다.
			int32 RemainingActorScanBudget = FMath::Clamp(SensorConfig.MaxActorScansPerUpdate, 1, 4096);

			// [v1.1.0] 작은 World에서 남은 budget 때문에 같은 Actor를 한 update 안에 다시 검사하지 않도록 고정할 시작 cycle 값입니다.
			const int32 StartingScanCycleSerial = PassiveScanCycleSerial;

			// [v1.1.0] Actor 슬롯을 하나도 소비하지 못하는 빈 Level 연속 구간에서 무한 순회를 막는 Level 이동 횟수입니다.
			int32 ConsecutiveLevelAdvanceCount = 0;
			while (RemainingActorScanBudget > 0
				&& ConsecutiveLevelAdvanceCount < WorldLevels.Num()
				&& PassiveScanCycleSerial == StartingScanCycleSerial)
			{
				// [v1.1.0] 현재 cursor가 가리키는 Level입니다.
				ULevel* CurrentLevel = WorldLevels[PassiveScanLevelIndex];
				if (!CurrentLevel || CurrentLevel->Actors.IsEmpty() || PassiveScanActorIndex >= CurrentLevel->Actors.Num())
				{
					AdvancePassiveScanLevel(WorldLevels.Num());
					++ConsecutiveLevelAdvanceCount;
					continue;
				}

				ConsecutiveLevelAdvanceCount = 0;

				// [v1.1.0] 현재 bounded cursor가 실제로 검사할 Actor 슬롯의 Actor입니다. null 슬롯도 예산을 소비합니다.
				AActor* CandidateActor = CurrentLevel->Actors[PassiveScanActorIndex].Get();
				++PassiveScanActorIndex;
				++LastPassiveScanActorCount;
				--RemainingActorScanBudget;

				ProcessPassiveScanActor(CandidateActor, SensorConfig);

				if (PassiveScanActorIndex >= CurrentLevel->Actors.Num())
				{
					AdvancePassiveScanLevel(WorldLevels.Num());
				}
			}
		}
	}

	// [v1.11.0] broad Pulse 만료를 먼저 반영해도 진행 중 Target Scan은 독립 상태로 남습니다.
	AdvanceActiveScanDuration(ActiveScanTimeThisUpdate, SensorConfig);

	if (bCurrentScanAttemptActive)
	{
		// [v1.11.0] bounded World cursor와 무관하게 지정 Target 하나를 매 update 직접 재관측해 Target Scan 자체가 broad Detection에 의존하지 않도록 합니다.
		ProcessPassiveScanActor(CurrentScanTargetActor.Get(), SensorConfig, true);
	}

	AdvanceCurrentScanAttempt(
		ResolvedSensorUpdateDeltaSeconds,
		TargetScanTimeThisUpdate,
		SensorConfig);
	AdvanceCurrentScanDuration(TargetScanTimeThisUpdate, SensorConfig);
	PublishRuntimeSnapshot();

	if (!HasConfiguredDetectionWork(SensorConfig)
		&& !bCurrentScanAttemptActive
		&& RuntimeContacts.IsEmpty())
	{
		SetComponentTickEnabled(false);
	}
}

// [v1.12.0] 기존 Automation/private caller를 전체 Sensor update로 전달하는 compatibility wrapper입니다.
void UCFVehicleSensorComp::RunPassiveDetectionUpdate(const float SensorUpdateDeltaSeconds)
{
	RunSensorUpdate(SensorUpdateDeltaSeconds);
}

// [v1.11.0] 하나의 Actor를 Sensor 자격·대표 위치·Passive/Visual/Active 거리 순서로 평가하고 지정 Target Scan 호출에는 그 Actor 하나만 Active-range 관측을 허용합니다.
bool UCFVehicleSensorComp::ProcessPassiveScanActor(
	AActor* CandidateActor,
	const FCFSensorConfig& SensorConfig,
	const bool bAllowTargetScanActiveRange)
{
	// [v1.4.0] 이미 알고 있는 Contact의 authoritative VehicleHealth 상태가 파괴라면 일반 탐지보다 먼저 DestroyedHold로 고정합니다.
	UCFVehicleHealthComp* CandidateHealthComponent = IsValid(CandidateActor)
		? CandidateActor->FindComponentByClass<UCFVehicleHealthComp>()
		: nullptr;
	if (IsValid(CandidateHealthComponent) && CandidateHealthComponent->IsDestroyed())
	{
		return ConfirmDestroyedContactForActor(CandidateActor);
	}

	if (!IsSensorCandidateEligible(CandidateActor))
	{
		return MarkContactNotObservedForActor(CandidateActor);
	}

	// [v1.1.0] ICFTargetSelectable이 제공한 Sensor 후보 대표 월드 위치입니다.
	const FVector TargetWorldLocation = ResolveSensorTargetLocation(CandidateActor);
	if (!IsFiniteSensorVector(TargetWorldLocation))
	{
		return MarkContactNotObservedForActor(CandidateActor);
	}

	// [v1.1.0] Sensor 탐지 원점으로 사용할 Component Owner Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	// [v1.1.0] Sensor Owner에서 대상 대표 위치까지의 실제 월드 거리입니다.
	const float DistanceToTargetCm = FVector::Distance(OwnerActor->GetActorLocation(), TargetWorldLocation);
	if (!FMath::IsFinite(DistanceToTargetCm))
	{
		return MarkContactNotObservedForActor(CandidateActor);
	}

	// [v1.1.0] Passive Tracking 근거리 전방향 범위 안에 들어온 대상인지 여부입니다.
	const bool bInsidePassiveRange = SensorConfig.PassiveDetectionRangeCm > KINDA_SMALL_NUMBER
		&& DistanceToTargetCm <= SensorConfig.PassiveDetectionRangeCm;

	// [v1.1.0] Passive 범위 밖에서도 직접 가시 대상 최소 계약을 적용할 Visual 거리 안인지 여부입니다.
	const bool bInsideVisualRange = SensorConfig.VisualDetectionRangeCm > KINDA_SMALL_NUMBER
		&& DistanceToTargetCm <= SensorConfig.VisualDetectionRangeCm;

	// [v1.3.0] Passive가 이미 성립하지 않을 때만 Visual fallback Trace를 수행한 직접 가시 탐지 결과입니다.
	const bool bVisualDetected = !bInsidePassiveRange
		&& bInsideVisualRange
		&& HasDirectSensorVisibility(CandidateActor, TargetWorldLocation, false);

	// [v1.3.0] Active Scan이 없어도 유지될 수 있는 기존 Passive/Visual 탐지 계약입니다.
	const bool bBaselineDetected = bInsidePassiveRange || bVisualDetected;

	// [v1.11.0] broad Active Detection Pulse 또는 명시 Target Scan 단일 Actor 관측에서만 허용되는 Active range Detection 결과입니다. LOS는 Contact 생성 조건이 아닙니다.
	const bool bActiveRangeDetected = (bActiveScanRunning || bAllowTargetScanActiveRange)
		&& SensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER
		&& DistanceToTargetCm <= SensorConfig.ActiveScanRangeCm;

	if (!bBaselineDetected && !bActiveRangeDetected)
	{
		return MarkContactNotObservedForActor(CandidateActor);
	}

	// [v1.1.0] 대상 자격·대표 위치와 별도로 읽는 TargetSelectable 원본 Metadata입니다.
	const FCFTargetDisplayInfo SourceDisplayInfo = ResolveSensorSourceDisplayInfo(CandidateActor);
	return UpsertLiveContact(
		CandidateActor,
		SourceDisplayInfo,
		TargetWorldLocation,
		bBaselineDetected);
}

// [v1.1.0] ICFTargetSelectable 대상 자격만 재사용해 Sensor 후보로 사용할 수 있는지 반환합니다.
bool UCFVehicleSensorComp::IsSensorCandidateEligible(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor))
	{
		return false;
	}

	// [v1.1.0] Sensor를 소유한 차량/Actor 자신은 Contact 후보에서 항상 제외합니다.
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor && CandidateActor == OwnerActor)
	{
		return false;
	}

	if (!CandidateActor->GetClass()->ImplementsInterface(UCFTargetSelectable::StaticClass()))
	{
		return false;
	}

	// [v1.1.0] TargetSelectable의 자체 자격 판정에 전달할 Sensor 전용 최소 컨텍스트입니다.
	FCFTargetSelectionContext SensorSelectionContext;
	SensorSelectionContext.ContextId = SensorPassiveContextId;
	SensorSelectionContext.bAllowSelfTarget = false;
	SensorSelectionContext.bRequireLineOfSightForNewSelection = false;
	return ResolveSensorTargetSelectable(CandidateActor, SensorSelectionContext);
}

// [v1.1.0] ICFTargetSelectable의 BlueprintNativeEvent 안전 디스패치로 대표 위치를 읽습니다.
FVector UCFVehicleSensorComp::ResolveSensorTargetLocation(AActor* CandidateActor) const
{
	return IsValid(CandidateActor)
		? ResolveSensorSelectionLocation(CandidateActor)
		: FVector::ZeroVector;
}

// [v1.1.0] ICFTargetSelectable의 Source Metadata를 읽되 InformationLevel을 Player Knowledge로 자동 승격하지 않습니다.
FCFTargetDisplayInfo UCFVehicleSensorComp::ResolveSensorSourceDisplayInfo(AActor* CandidateActor) const
{
	return IsValid(CandidateActor)
		? ResolveSensorDisplayInfo(CandidateActor)
		: FCFTargetDisplayInfo();
}

// [v1.3.0] Owner→Target 대표 위치를 ECC_Visibility로 확인하고 호출 목적에 맞는 Trace 진단값을 증가시킵니다.
bool UCFVehicleSensorComp::HasDirectSensorVisibility(
	AActor* CandidateActor,
	const FVector& TargetWorldLocation,
	const bool bActiveAnalysisTrace)
{
	if (!IsValid(CandidateActor) || !IsFiniteSensorVector(TargetWorldLocation))
	{
		return false;
	}

	// [v1.1.0] Visibility Trace를 실행할 현재 World입니다.
	UWorld* CurrentWorld = GetWorld();

	// [v1.1.0] Visibility 시작점을 제공하고 자기 충돌을 무시할 Sensor Owner입니다.
	AActor* OwnerActor = GetOwner();
	if (!CurrentWorld || !IsValid(OwnerActor))
	{
		return false;
	}

	// [v1.1.0] Sensor 시야 판정이 시작되는 Owner 월드 위치입니다.
	const FVector VisibilityStart = OwnerActor->GetActorLocation();
	if (!IsFiniteSensorVector(VisibilityStart))
	{
		return false;
	}

	if (VisibilityStart.Equals(TargetWorldLocation, KINDA_SMALL_NUMBER))
	{
		return true;
	}

	// [v1.3.0] Passive Visual과 Active Analysis를 로그에서 구분하되 둘 다 TargetSelect와 무관한 ECC_Visibility만 사용합니다.
	const FName VisibilityTraceName = bActiveAnalysisTrace
		? FName(TEXT("CFSensorActiveAnalysisVisibility"))
		: FName(TEXT("CFSensorPassiveVisibility"));

	// [v1.1.0] TargetSelect 전용 채널과 분리된 일반 직접 가시성용 Query 설정입니다.
	FCollisionQueryParams VisibilityQueryParams(VisibilityTraceName, false, OwnerActor);

	// [v1.1.0] ECC_Visibility Trace의 최초 blocking hit 결과입니다.
	FHitResult VisibilityHit;
	if (bActiveAnalysisTrace)
	{
		++LastActiveAnalysisTraceCount;
	}
	else
	{
		++LastPassiveVisibilityTraceCount;
	}

	// [v1.1.0] World geometry 또는 대상 자체에 최초 blocking hit가 있었는지 여부입니다.
	const bool bBlockingHit = CurrentWorld->LineTraceSingleByChannel(
		VisibilityHit,
		VisibilityStart,
		TargetWorldLocation,
		ECC_Visibility,
		VisibilityQueryParams);

	return !bBlockingHit || VisibilityHit.GetActor() == CandidateActor;
}

// [v1.15.0] 같은 Actor Contact를 갱신하거나 새 Contact를 admit하기 전에 Persistent Entity identity/terminal 계약을 확인하고 동일 Entity Knowledge를 복원합니다.
bool UCFVehicleSensorComp::UpsertLiveContact(
	AActor* CandidateActor,
	const FCFTargetDisplayInfo& SourceDisplayInfo,
	const FVector& TargetWorldLocation,
	const bool bBaselineDetectionValidAtObservation)
{
	if (!IsValid(CandidateActor))
	{
		return false;
	}

	// [v1.14.0] ContactId·표시 TargetId와 독립적으로 Native/Blueprint provider를 안전하게 해석한 Gameplay Entity Identity입니다.
	const FGuid ObservedTargetEntityId = CandidateActor->GetClass()->ImplementsInterface(UCFTargetSelectable::StaticClass())
		? ResolveSensorTargetEntityId(CandidateActor)
		: FGuid();

	// [v1.1.0] 같은 Actor에 이미 존재하는 Runtime Contact 인덱스입니다.
	int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(CandidateActor);
	if (RuntimeContacts.IsValidIndex(RuntimeContactIndex))
	{
		// [v1.15.0] 기존 Contact가 최초 캡처한 Entity ID를 같은 Contact lifetime 동안 바꾸지 않기 위한 현재 Identity입니다.
		const FGuid ExistingContactEntityId = RuntimeContacts[RuntimeContactIndex].PublicContact.TargetEntityId;
		if (ExistingContactEntityId.IsValid()
			&& ObservedTargetEntityId.IsValid()
			&& ExistingContactEntityId != ObservedTargetEntityId)
		{
			return false;
		}
	}

	// [v1.15.0] 기존 Contact가 유효 Identity를 이미 소유하면 provider가 일시 Invalid를 반환해도 최초 캡처 Identity를 Knowledge lookup 기준으로 유지합니다.
	const FGuid KnowledgeTargetEntityId = RuntimeContacts.IsValidIndex(RuntimeContactIndex)
		&& RuntimeContacts[RuntimeContactIndex].PublicContact.TargetEntityId.IsValid()
		? RuntimeContacts[RuntimeContactIndex].PublicContact.TargetEntityId
		: ObservedTargetEntityId;

	// [v1.15.0] 동일 Entity에 이미 Persistent Knowledge가 있으면 새 Contact admission 전에 consistency/terminal을 확인할 Record입니다.
	FCFSensorKnowledgeRecord* ExistingKnowledgeRecord = KnowledgeTargetEntityId.IsValid()
		? PersistentKnowledgeStore.Find(KnowledgeTargetEntityId)
		: nullptr;
	if (ExistingKnowledgeRecord)
	{
		if (!IsPersistentKnowledgeIdentityCompatible(*ExistingKnowledgeRecord, SourceDisplayInfo)
			|| ExistingKnowledgeRecord->bTerminalDestroyed)
		{
			return false;
		}

		FillPersistentKnowledgeIdentity(*ExistingKnowledgeRecord, SourceDisplayInfo);
	}

	if (RuntimeContactIndex == INDEX_NONE)
	{
		// [v1.15.0] Store consistency/terminal 검사를 통과한 뒤에만 새 Runtime Contact를 admit합니다.
		FCFSensorContactRuntime NewRuntimeContact;
		NewRuntimeContact.TargetActor = CandidateActor;
		NewRuntimeContact.SourceDisplayInfo = SourceDisplayInfo;
		NewRuntimeContact.PublicContact.ContactId = AllocateContactId();
		// [v1.13.0] Entity Identity는 Contact 최초 생성 때만 캡처하며 이후 재관측으로 같은 Contact의 ID를 바꾸지 않습니다.
		NewRuntimeContact.PublicContact.TargetEntityId = ObservedTargetEntityId;
		NewRuntimeContact.PublicContact.TargetCategory = SourceDisplayInfo.TargetCategory;
		NewRuntimeContact.PublicContact.Relation = SourceDisplayInfo.Relation;
		NewRuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::Detected;
		RuntimeContactIndex = RuntimeContacts.Add(MoveTemp(NewRuntimeContact));
	}

	// [v1.15.0] 새로 생성했거나 기존 Actor에서 찾아낸 유일한 Runtime Contact입니다.
	FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
	if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::DestroyedHold)
	{
		return false;
	}

	RuntimeContact.TargetActor = CandidateActor;
	RuntimeContact.SourceDisplayInfo = SourceDisplayInfo;
	RuntimeContact.bLostSnapshotPublished = false;
	RuntimeContact.bBaselineDetectionValidAtLastObservation = bBaselineDetectionValidAtObservation;

	// [v1.15.0] Contact가 제거됐다가 같은 valid Entity로 새로 admit된 경우에만 Persistent Knowledge Projection을 복원합니다.
	if (ExistingKnowledgeRecord
		&& RuntimeContact.PublicContact.TargetEntityId == ExistingKnowledgeRecord->TargetEntityId)
	{
		RestorePersistentKnowledgeToContact(RuntimeContact, *ExistingKnowledgeRecord);
	}

	BindContactDestroyedEvent(RuntimeContact);

	// [v1.4.0] event binding 직전 이미 파괴가 확정된 극단적 순서에서도 기존 Contact를 Live로 되돌리지 않는 상태 확인입니다.
	UCFVehicleHealthComp* BoundHealthComponent = RuntimeContact.BoundVehicleHealthComponent.Get();
	if (IsValid(BoundHealthComponent) && BoundHealthComponent->IsDestroyed())
	{
		return ConfirmDestroyedContactForActor(CandidateActor);
	}

	// [v1.1.0] 외부 Actor 포인터 없이 갱신할 공개 Contact 사본 참조입니다.
	FCFSensorContact& PublicContact = RuntimeContact.PublicContact;
	// [v1.15.0] 현재 Source가 concrete category를 제공하면 사용하고, Unknown이면 Store에서 복원한 안정 분류를 유지합니다.
	if (SourceDisplayInfo.TargetCategory != ECFTargetCategory::Unknown
		|| PublicContact.TargetCategory == ECFTargetCategory::Unknown)
	{
		PublicContact.TargetCategory = SourceDisplayInfo.TargetCategory;
	}
	PublicContact.Relation = SourceDisplayInfo.Relation;
	if (PublicContact.InformationLevel == ECFTargetInfoLevel::None)
	{
		PublicContact.InformationLevel = ECFTargetInfoLevel::Detected;
	}
	PublicContact.ContactState = ECFSensorContactState::Live;
	PublicContact.LastKnownWorldLocation = TargetWorldLocation;

	// [v1.1.0] 마지막 유효 관측 시각을 기록할 현재 World입니다.
	const UWorld* CurrentWorld = GetWorld();
	PublicContact.LastObservedWorldTimeSeconds = CurrentWorld
		? FMath::Max(0.0, static_cast<double>(CurrentWorld->GetTimeSeconds()))
		: 0.0;
	PublicContact.FreshnessSeconds = 0.0f;
	PublicContact.bDestroyedConfirmed = false;

	// SourceDisplayInfo.InformationLevel은 Actor truth이므로 Sensor Player Knowledge로 복사하지 않습니다.
	return true;
}

// [v1.4.0] 기존 Runtime Contact의 VehicleHealth 파괴 이벤트 구독을 현재 Actor 컴포넌트에 맞춰 연결합니다.
void UCFVehicleSensorComp::BindContactDestroyedEvent(FCFSensorContactRuntime& RuntimeContact)
{
	// [v1.4.0] 파괴 확정 owner를 찾을 현재 Contact Actor입니다.
	AActor* TargetActor = RuntimeContact.TargetActor.Get();
	UCFVehicleHealthComp* VehicleHealthComponent = IsValid(TargetActor)
		? TargetActor->FindComponentByClass<UCFVehicleHealthComp>()
		: nullptr;

	if (RuntimeContact.BoundVehicleHealthComponent.Get() == VehicleHealthComponent)
	{
		return;
	}

	UnbindContactDestroyedEvent(RuntimeContact);
	if (!IsValid(VehicleHealthComponent))
	{
		return;
	}

	VehicleHealthComponent->OnVehicleDestroyed.AddUniqueDynamic(
		this,
		&UCFVehicleSensorComp::HandleObservedVehicleDestroyed);
	RuntimeContact.BoundVehicleHealthComponent = VehicleHealthComponent;
}

// [v1.4.0] Runtime Contact가 구독 중인 VehicleHealth 파괴 이벤트를 안전하게 해제합니다.
void UCFVehicleSensorComp::UnbindContactDestroyedEvent(FCFSensorContactRuntime& RuntimeContact)
{
	UCFVehicleHealthComp* BoundVehicleHealthComponent = RuntimeContact.BoundVehicleHealthComponent.Get();
	if (IsValid(BoundVehicleHealthComponent))
	{
		BoundVehicleHealthComponent->OnVehicleDestroyed.RemoveDynamic(
			this,
			&UCFVehicleSensorComp::HandleObservedVehicleDestroyed);
	}

	RuntimeContact.BoundVehicleHealthComponent.Reset();
}

// [v1.4.0] Reset 또는 재초기화 전에 모든 Runtime Contact의 VehicleHealth 파괴 이벤트를 안전하게 해제합니다.
void UCFVehicleSensorComp::UnbindAllContactDestroyedEvents()
{
	for (FCFSensorContactRuntime& RuntimeContact : RuntimeContacts)
	{
		UnbindContactDestroyedEvent(RuntimeContact);
	}
}

// [v1.15.0] authoritative VehicleHealth가 파괴 상태인 기존 Contact를 Terminal Store에 먼저 기록한 뒤 DestroyedHold로 전환합니다.
bool UCFVehicleSensorComp::ConfirmDestroyedContactForActor(AActor* CandidateActor)
{
	if (!IsValid(CandidateActor))
	{
		return false;
	}

	// [v1.4.0] Sensor가 파괴 truth로 인정하는 기존 VehicleHealth Runtime입니다.
	UCFVehicleHealthComp* VehicleHealthComponent = CandidateActor->FindComponentByClass<UCFVehicleHealthComp>();
	if (!IsValid(VehicleHealthComponent) || !VehicleHealthComponent->IsDestroyed())
	{
		return false;
	}

	// [v1.15.0] 월드의 미관측 파괴 Actor로 ghost Knowledge를 만들지 않고 이미 Sensor가 알고 있던 Contact만 terminalize합니다.
	const int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(CandidateActor);
	if (RuntimeContactIndex == INDEX_NONE)
	{
		return false;
	}

	FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
	if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::DestroyedHold)
	{
		return true;
	}

	// [v1.15.0] Detected-only Contact라도 valid Entity이면 최소 Terminal Record를 만들며 identity conflict에서는 Contact 상태까지 변경하지 않습니다.
	if (!UpsertPersistentKnowledgeRecord(
		RuntimeContact.PublicContact.TargetEntityId,
		RuntimeContact.SourceDisplayInfo,
		&RuntimeContact.PublicContact,
		true))
	{
		return false;
	}

	RuntimeContact.PublicContact.ContactState = ECFSensorContactState::DestroyedHold;
	RuntimeContact.PublicContact.bDestroyedConfirmed = true;
	RuntimeContact.bLostSnapshotPublished = false;
	RuntimeContact.bBaselineDetectionValidAtLastObservation = false;

	// [v1.4.0] 마지막 관측 시각과 별개로 DestroyedHold 보존 수명만 재는 파괴 확정 시각입니다.
	const UWorld* CurrentWorld = GetWorld();
	RuntimeContact.DestroyedConfirmedWorldTimeSeconds = CurrentWorld
		? FMath::Max(0.0, static_cast<double>(CurrentWorld->GetTimeSeconds()))
		: 0.0;

	// [v1.4.0] Passive/Active 탐지가 꺼져 있어도 DestroyedHold 만료 update가 계속 실행돼야 합니다.
	if (bSensorRuntimeReady)
	{
		SetComponentTickEnabled(true);
	}

	return true;
}

// [v1.4.0] VehicleHealthComp의 최초 파괴 이벤트를 받아 기존 Sensor Contact를 즉시 DestroyedHold로 전환합니다.
void UCFVehicleSensorComp::HandleObservedVehicleDestroyed(FCFDamageHitContext DamageHitContext)
{
	AActor* DestroyedActor = DamageHitContext.HitActor.Get();
	if (!IsValid(DestroyedActor) || !ConfirmDestroyedContactForActor(DestroyedActor))
	{
		return;
	}

	// [v1.4.0] bounded cursor를 기다리지 않고 파괴 확정을 같은 이벤트 처리에서 Actor-free Snapshot으로 게시합니다.
	PublishRuntimeSnapshot();
}

// [v1.2.0] 이번 후보 평가에서 탐지 조건을 잃은 기존 Live Contact를 LastKnown으로 전환하되 마지막 신뢰 위치·관측 시각은 변경하지 않습니다.
bool UCFVehicleSensorComp::MarkContactNotObservedForActor(const AActor* CandidateActor)
{
	// [v1.2.0] 탐지 상실을 기록할 Actor의 기존 Runtime Contact 인덱스입니다.
	const int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(CandidateActor);
	if (RuntimeContactIndex == INDEX_NONE)
	{
		return false;
	}

	// [v1.2.0] 마지막 신뢰 위치와 Sensor Knowledge를 그대로 보존할 private Runtime Contact입니다.
	FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
	if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Live)
	{
		RuntimeContact.PublicContact.ContactState = ECFSensorContactState::LastKnown;
		RuntimeContact.bLostSnapshotPublished = false;
	}

	return true;
}

// [v1.6.0] Passive/Visual 탐지 능력 감소 시 기존 Live Contact를 삭제하지 않고 LastKnown lifecycle로 넘깁니다.
void UCFVehicleSensorComp::MarkAllLiveContactsLastKnown()
{
	for (FCFSensorContactRuntime& RuntimeContact : RuntimeContacts)
	{
		if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Live)
		{
			RuntimeContact.PublicContact.ContactState = ECFSensorContactState::LastKnown;
			RuntimeContact.bLostSnapshotPublished = false;
		}
	}
}

// [v1.3.0] Active Scan 종료 시 마지막 관측이 Active-only였던 Live Contact를 LastKnown으로 전환합니다.
void UCFVehicleSensorComp::MarkActiveOnlyContactsLastKnown()
{
	for (FCFSensorContactRuntime& RuntimeContact : RuntimeContacts)
	{
		if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Live
			&& !RuntimeContact.bBaselineDetectionValidAtLastObservation)
		{
			RuntimeContact.PublicContact.ContactState = ECFSensorContactState::LastKnown;
			RuntimeContact.bLostSnapshotPublished = false;
		}
	}
}

// [v1.4.0] 모든 Runtime Contact의 Freshness, LastKnown→Lost→Removed와 DestroyedHold 만료를 bounded cursor와 독립적으로 현재 Sensor update 시각까지 진행합니다.
void UCFVehicleSensorComp::AdvanceContactLifetimes(
	const double CurrentWorldTimeSeconds,
	const FCFSensorConfig& SensorConfig)
{
	if (!FMath::IsFinite(CurrentWorldTimeSeconds) || !SensorConfig.IsValid())
	{
		return;
	}

	// [v1.2.0] World 시간 역행이나 음수 입력에서도 freshness가 음수가 되지 않도록 보정한 현재 시간입니다.
	const double SafeCurrentWorldTimeSeconds = FMath::Max(0.0, CurrentWorldTimeSeconds);

	for (int32 RuntimeContactIndex = RuntimeContacts.Num() - 1; RuntimeContactIndex >= 0; --RuntimeContactIndex)
	{
		// [v1.2.0] 이번 update에서 freshness와 lifecycle을 진행할 private Runtime Contact입니다.
		FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];

		// [v1.2.0] 공개 Snapshot에 최소 한 번 노출된 Lost Contact는 다음 Sensor update 시작 시 제거합니다.
				if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Lost
			&& RuntimeContact.bLostSnapshotPublished)
		{
			UnbindContactDestroyedEvent(RuntimeContact);
			RuntimeContacts.RemoveAtSwap(RuntimeContactIndex, 1, EAllowShrinking::No);
			continue;
		}

		// [v1.4.0] DestroyedHold는 마지막 신뢰 관측 freshness를 계속 진행하되 별도 파괴 확정 시각으로 보존 수명을 계산합니다.
		if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::DestroyedHold)
		{
			const double DestroyedFreshnessSeconds = FMath::Max(
				0.0,
				SafeCurrentWorldTimeSeconds - RuntimeContact.PublicContact.LastObservedWorldTimeSeconds);
			RuntimeContact.PublicContact.FreshnessSeconds = static_cast<float>(FMath::Min(
				DestroyedFreshnessSeconds,
				static_cast<double>(TNumericLimits<float>::Max())));

			// [v1.4.0] bounded Actor cursor와 무관하게 현재 Sensor update 시각으로 계산한 DestroyedHold 경과 시간입니다.
			const double DestroyedHoldElapsedSeconds = FMath::Max(
				0.0,
				SafeCurrentWorldTimeSeconds - RuntimeContact.DestroyedConfirmedWorldTimeSeconds);
			if (DestroyedHoldElapsedSeconds >= static_cast<double>(SensorConfig.DestroyedHoldTimeSec))
			{
				UnbindContactDestroyedEvent(RuntimeContact);
				RuntimeContacts.RemoveAtSwap(RuntimeContactIndex, 1, EAllowShrinking::No);
			}
			continue;
		}

		// [v1.2.0] Actor가 사라진 Live Contact도 cursor 재방문을 기다리지 않고 관측 상실로 전환했는지 여부입니다.
		bool bEnteredLastKnownThisUpdate = false;
		if (!RuntimeContact.TargetActor.IsValid()
			&& RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Live)
		{
			RuntimeContact.PublicContact.ContactState = ECFSensorContactState::LastKnown;
			RuntimeContact.bLostSnapshotPublished = false;
			bEnteredLastKnownThisUpdate = true;
		}

		// [v1.2.0] 마지막 유효 관측 이후 현재 update까지 경과한 고정밀 시간입니다.
		const double FreshnessSeconds = FMath::Max(
			0.0,
			SafeCurrentWorldTimeSeconds - RuntimeContact.PublicContact.LastObservedWorldTimeSeconds);

		// [v1.2.0] 공개 float 범위를 넘는 비정상 장시간 세션에서도 유한값을 유지할 freshness입니다.
		RuntimeContact.PublicContact.FreshnessSeconds = static_cast<float>(FMath::Min(
			FreshnessSeconds,
			static_cast<double>(TNumericLimits<float>::Max())));

		if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::LastKnown
			&& !bEnteredLastKnownThisUpdate
			&& RuntimeContact.PublicContact.FreshnessSeconds >= SensorConfig.ContactMemoryTimeSec)
		{
			RuntimeContact.PublicContact.ContactState = ECFSensorContactState::Lost;
			RuntimeContact.bLostSnapshotPublished = false;
		}
		}
}

// [v1.11.0] 현재 지정 Target Scan Attempt 하나만 독립 Target Scan 유효 시간에는 증가시키고 LOS/거리 조건 상실에서는 감소시킵니다.
void UCFVehicleSensorComp::AdvanceCurrentScanAttempt(
	const float SensorUpdateDeltaSeconds,
	const float TargetScanTimeThisUpdate,
	const FCFSensorConfig& SensorConfig)
{
	if (!bCurrentScanAttemptActive
		|| !SensorConfig.IsValid()
		|| !FMath::IsFinite(SensorUpdateDeltaSeconds)
		|| SensorUpdateDeltaSeconds < 0.0f
		|| !FMath::IsFinite(TargetScanTimeThisUpdate)
		|| TargetScanTimeThisUpdate < 0.0f)
	{
		return;
	}

	// [v1.10.0] 이번 Sensor update가 실제로 대표하는 유한한 경과 시간입니다.
	const float SafeSensorUpdateDeltaSeconds = FMath::Max(SensorUpdateDeltaSeconds, 0.0f);
	// [v1.11.0] update 시간보다 길게 gain하지 않도록 제한한 현재 Target Scan Attempt 유효 시간입니다.
	const float SafeTargetScanTimeThisUpdate = FMath::Clamp(TargetScanTimeThisUpdate, 0.0f, SafeSensorUpdateDeltaSeconds);
	// [v1.11.0] Target Scan duration이 update 도중 만료된 경우 후반 decay에 사용할 비활성 시간입니다.
	const float InactiveTimeThisUpdate = FMath::Max(0.0f, SafeSensorUpdateDeltaSeconds - SafeTargetScanTimeThisUpdate);

	// [v1.10.0] 현재 Scan Attempt의 private Target Actor입니다.
	AActor* TargetActor = CurrentScanTargetActor.Get();
	// [v1.10.0] 현재 Target Actor와 연결된 Runtime Contact 인덱스입니다.
	const int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(TargetActor);
	if (!IsValid(TargetActor)
		|| !RuntimeContacts.IsValidIndex(RuntimeContactIndex)
		|| RuntimeContacts[RuntimeContactIndex].PublicContact.ContactId != CurrentScanTargetContactId)
	{
		// [v1.10.0] Target/Contact 연결을 일시 잃은 동안 적용할 현재 Attempt 진행률 감소량입니다.
		const float AnalysisDecay = SensorConfig.AnalysisDecayPerSec * SafeSensorUpdateDeltaSeconds;
		CurrentScanAttemptProgress01 = FMath::Clamp(CurrentScanAttemptProgress01 - AnalysisDecay, 0.0f, 1.0f);
		return;
	}

	// [v1.10.0] 현재 Scan Attempt가 소유하는 유일한 Runtime Contact입니다.
	FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
	// [v1.11.0] 이번 독립 Target Scan 구간에서 지정 Contact가 실제 Analysis gain 조건을 만족하는지 여부입니다.
	const bool bAnalysisValid = SafeTargetScanTimeThisUpdate > KINDA_SMALL_NUMBER
		&& IsCurrentScanAttemptValidForAnalysis(RuntimeContact, SensorConfig);

	if (bAnalysisValid)
	{
		// [v1.11.0] 유효 분석 구간 동안 현재 Attempt에 누적할 progress 증가량입니다.
		const float AnalysisGain = SensorConfig.AnalysisGainPerSec * SafeTargetScanTimeThisUpdate;
		CurrentScanAttemptProgress01 = FMath::Clamp(CurrentScanAttemptProgress01 + AnalysisGain, 0.0f, 1.0f);
		PromoteContactKnowledgeFromScanProgress(RuntimeContact, CurrentScanAttemptProgress01, SensorConfig);

		if (RuntimeContact.PublicContact.InformationLevel == ECFTargetInfoLevel::DetailedScan)
		{
			CompleteCurrentScanAttempt(RuntimeContact, SensorConfig);
			return;
		}

		if (InactiveTimeThisUpdate > KINDA_SMALL_NUMBER)
		{
			// [v1.10.0] Scanner duration이 update 도중 끝난 뒤 남은 구간에만 적용할 progress 감소량입니다.
			const float AnalysisDecay = SensorConfig.AnalysisDecayPerSec * InactiveTimeThisUpdate;
			CurrentScanAttemptProgress01 = FMath::Clamp(CurrentScanAttemptProgress01 - AnalysisDecay, 0.0f, 1.0f);
		}
	}
	else if (SafeSensorUpdateDeltaSeconds > KINDA_SMALL_NUMBER)
	{
		// [v1.10.0] LOS·거리·Live 조건 상실 동안 현재 Attempt에 적용할 전체 update progress 감소량입니다.
		const float AnalysisDecay = SensorConfig.AnalysisDecayPerSec * SafeSensorUpdateDeltaSeconds;
		CurrentScanAttemptProgress01 = FMath::Clamp(CurrentScanAttemptProgress01 - AnalysisDecay, 0.0f, 1.0f);
	}
}

// [v1.10.0] 현재 지정 Scan Attempt의 단 하나의 Runtime Contact가 Active Analysis 증가 조건을 만족하는지 판정합니다.
bool UCFVehicleSensorComp::IsCurrentScanAttemptValidForAnalysis(
	FCFSensorContactRuntime& RuntimeContact,
	const FCFSensorConfig& SensorConfig)
{
	if (!bCurrentScanAttemptActive
		|| SensorConfig.ActiveScanRangeCm <= KINDA_SMALL_NUMBER
		|| RuntimeContact.PublicContact.ContactId != CurrentScanTargetContactId
		|| RuntimeContact.PublicContact.ContactState != ECFSensorContactState::Live)
	{
		return false;
	}

	// [v1.10.0] 현재 Attempt target과 Runtime Contact가 같은 Actor인지 확인할 private Actor입니다.
	AActor* TargetActor = RuntimeContact.TargetActor.Get();
	if (!IsValid(TargetActor)
		|| TargetActor != CurrentScanTargetActor.Get()
		|| !IsSensorCandidateEligible(TargetActor))
	{
		return false;
	}

	// [v1.10.0] Active Analysis 거리와 LOS 끝점으로 사용할 TargetSelectable 대표 위치입니다.
	const FVector TargetWorldLocation = ResolveSensorTargetLocation(TargetActor);
	if (!IsFiniteSensorVector(TargetWorldLocation))
	{
		return false;
	}

	// [v1.10.0] Active Analysis 기준 원점을 소유한 Sensor Owner입니다.
	const AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	// [v1.10.0] 현재 실제 위치 기준 지정 Target 분석 거리입니다.
	const float DistanceToTargetCm = FVector::Distance(OwnerActor->GetActorLocation(), TargetWorldLocation);
	if (!FMath::IsFinite(DistanceToTargetCm) || DistanceToTargetCm > SensorConfig.ActiveScanRangeCm)
	{
		return false;
	}

	return HasDirectSensorVisibility(TargetActor, TargetWorldLocation, true);
}

// [v1.15.0] Target Scan Attempt 진행률이 임계값을 통과하면 Contact와 Persistent Store Knowledge를 같은 전이에서 단방향 승격합니다.
void UCFVehicleSensorComp::PromoteContactKnowledgeFromScanProgress(
	FCFSensorContactRuntime& RuntimeContact,
	const float ScanAttemptProgress01,
	const FCFSensorConfig& SensorConfig)
{
	if (RuntimeContact.SourceDisplayInfo.TargetId.IsNone() || !FMath::IsFinite(ScanAttemptProgress01))
	{
		return;
	}

	// [v1.12.0] Contact Knowledge 승격 판정에 사용할 0~1 현재 Target Scan Attempt 진행률입니다.
	const float ScanProgress = FMath::Clamp(ScanAttemptProgress01, 0.0f, 1.0f);
	if (ScanProgress >= SensorConfig.DetailedScanThreshold)
	{
		// [v1.15.0] Store upsert가 실패해도 기존 Contact/allocator를 부분 승격하지 않도록 준비하는 후보 Projection입니다.
		FCFSensorContact PromotedContact = RuntimeContact.PublicContact;
		// [v1.15.0] 기존 DetailedScan Revision이 없을 때만 Component 수명 단조 증가 allocator에서 예약할 후보 Revision입니다.
		const bool bNeedsNewAnalysisRevision = PromotedContact.InformationLevel != ECFTargetInfoLevel::DetailedScan
			|| PromotedContact.AnalysisCompletionRevision <= 0;
		// [v1.15.0] 최초 DetailedScan Knowledge 획득에 사용할 양수 Revision이며 Store 성공 전에는 allocator를 진행하지 않습니다.
		const int32 CompletionRevision = bNeedsNewAnalysisRevision
			? FMath::Max(NextAnalysisCompletionRevision, 1)
			: PromotedContact.AnalysisCompletionRevision;
		PromotedContact.AnalysisCompletionRevision = CompletionRevision;
		PromotedContact.InformationLevel = ECFTargetInfoLevel::DetailedScan;
		PromotedContact.KnownTargetId = RuntimeContact.SourceDisplayInfo.TargetId;
		PromotedContact.KnownDisplayName = RuntimeContact.SourceDisplayInfo.DisplayName;

		if (!UpsertPersistentKnowledgeRecord(
			PromotedContact.TargetEntityId,
			RuntimeContact.SourceDisplayInfo,
			&PromotedContact,
			false))
		{
			return;
		}

		RuntimeContact.PublicContact = PromotedContact;
		if (bNeedsNewAnalysisRevision)
		{
			NextAnalysisCompletionRevision = CompletionRevision + 1;
		}
		return;
	}

	if (ScanProgress >= SensorConfig.IdentifiedThreshold
		&& (RuntimeContact.PublicContact.InformationLevel == ECFTargetInfoLevel::None
			|| RuntimeContact.PublicContact.InformationLevel == ECFTargetInfoLevel::Detected))
	{
		// [v1.15.0] Identified Store upsert와 Contact 승격을 원자적으로 맞추기 위한 후보 Projection입니다.
		FCFSensorContact PromotedContact = RuntimeContact.PublicContact;
		PromotedContact.InformationLevel = ECFTargetInfoLevel::Identified;
		PromotedContact.KnownTargetId = RuntimeContact.SourceDisplayInfo.TargetId;
		PromotedContact.KnownDisplayName = RuntimeContact.SourceDisplayInfo.DisplayName;
		if (!UpsertPersistentKnowledgeRecord(
			PromotedContact.TargetEntityId,
			RuntimeContact.SourceDisplayInfo,
			&PromotedContact,
			false))
		{
			return;
		}

		RuntimeContact.PublicContact = PromotedContact;
	}
}

// [v1.15.0] 기존 Store의 concrete stable identity와 현재 Source Metadata가 모순되지 않는지 판정합니다.
bool UCFVehicleSensorComp::IsPersistentKnowledgeIdentityCompatible(
	const FCFSensorKnowledgeRecord& KnowledgeRecord,
	const FCFTargetDisplayInfo& SourceDisplayInfo) const
{
	// [v1.15.0] None은 아직 확정되지 않은 Source TargetId이므로 concrete mismatch에만 충돌을 선언합니다.
	const bool bTargetIdConflict = !KnowledgeRecord.StableTargetId.IsNone()
		&& !SourceDisplayInfo.TargetId.IsNone()
		&& KnowledgeRecord.StableTargetId != SourceDisplayInfo.TargetId;
	// [v1.15.0] Unknown은 아직 확정되지 않은 분류이므로 양쪽 concrete category mismatch에만 충돌을 선언합니다.
	const bool bTargetCategoryConflict = KnowledgeRecord.StableTargetCategory != ECFTargetCategory::Unknown
		&& SourceDisplayInfo.TargetCategory != ECFTargetCategory::Unknown
		&& KnowledgeRecord.StableTargetCategory != SourceDisplayInfo.TargetCategory;
	return !bTargetIdConflict && !bTargetCategoryConflict;
}

// [v1.15.0] Store의 미확정 Stable Identity만 현재 concrete Source Metadata로 최초 보강합니다.
void UCFVehicleSensorComp::FillPersistentKnowledgeIdentity(
	FCFSensorKnowledgeRecord& KnowledgeRecord,
	const FCFTargetDisplayInfo& SourceDisplayInfo)
{
	if (KnowledgeRecord.StableTargetId.IsNone() && !SourceDisplayInfo.TargetId.IsNone())
	{
		KnowledgeRecord.StableTargetId = SourceDisplayInfo.TargetId;
	}

	if (KnowledgeRecord.StableTargetCategory == ECFTargetCategory::Unknown
		&& SourceDisplayInfo.TargetCategory != ECFTargetCategory::Unknown)
	{
		KnowledgeRecord.StableTargetCategory = SourceDisplayInfo.TargetCategory;
	}
}

// [v1.15.0] 동일 Entity Store Knowledge를 새 Contact Projection에 복원하되 Scan Operation state와 allocator는 건드리지 않습니다.
void UCFVehicleSensorComp::RestorePersistentKnowledgeToContact(
	FCFSensorContactRuntime& RuntimeContact,
	const FCFSensorKnowledgeRecord& KnowledgeRecord)
{
	if (!RuntimeContact.PublicContact.TargetEntityId.IsValid()
		|| RuntimeContact.PublicContact.TargetEntityId != KnowledgeRecord.TargetEntityId
		|| KnowledgeRecord.bTerminalDestroyed)
	{
		return;
	}

	if (KnowledgeRecord.StableTargetCategory != ECFTargetCategory::Unknown)
	{
		RuntimeContact.PublicContact.TargetCategory = KnowledgeRecord.StableTargetCategory;
	}

	if (KnowledgeRecord.HighestKnowledgeTier == ECFTargetInfoLevel::DetailedScan)
	{
		// [v1.15.0] 손상된 Store가 공개 Contact의 DetailedScan↔positive revision 계약을 깨뜨리지 않도록 양수 Revision이 있을 때만 DetailedScan을 복원합니다.
		if (KnowledgeRecord.DetailedScanAnalysisRevision <= 0)
		{
			return;
		}

		RuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::DetailedScan;
		RuntimeContact.PublicContact.KnownTargetId = KnowledgeRecord.KnownTargetId;
		RuntimeContact.PublicContact.KnownDisplayName = KnowledgeRecord.KnownDisplayName;
		// [v1.15.0] 과거 획득 Revision을 exact 복원하며 NextAnalysisCompletionRevision은 증가시키거나 재계산하지 않습니다.
		RuntimeContact.PublicContact.AnalysisCompletionRevision = KnowledgeRecord.DetailedScanAnalysisRevision;
		return;
	}

	if (KnowledgeRecord.HighestKnowledgeTier == ECFTargetInfoLevel::Identified)
	{
		RuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::Identified;
		RuntimeContact.PublicContact.KnownTargetId = KnowledgeRecord.KnownTargetId;
		RuntimeContact.PublicContact.KnownDisplayName = KnowledgeRecord.KnownDisplayName;
	}
}

// [v1.15.0] valid Entity의 Identified/DetailedScan 또는 Terminal Knowledge를 private Store에 즉시 반영합니다.
bool UCFVehicleSensorComp::UpsertPersistentKnowledgeRecord(
	const FGuid& TargetEntityId,
	const FCFTargetDisplayInfo& SourceDisplayInfo,
	const FCFSensorContact* PublicContact,
	const bool bTerminalDestroyed)
{
	// [v1.15.0] Identity 미지원 대상은 기존 Contact/Scan 호환을 유지하되 Persistent Store에는 추측 key를 만들지 않습니다.
	if (!TargetEntityId.IsValid())
	{
		return true;
	}

	// [v1.15.0] DetailedScan Record는 기존 공개 계약과 동일하게 양수 AnalysisCompletionRevision을 반드시 가져야 합니다.
	if (PublicContact
		&& PublicContact->InformationLevel == ECFTargetInfoLevel::DetailedScan
		&& PublicContact->AnalysisCompletionRevision <= 0)
	{
		return false;
	}

	// [v1.15.0] 같은 Gameplay Entity에 이미 축적된 Persistent Knowledge Record입니다.
	FCFSensorKnowledgeRecord* KnowledgeRecord = PersistentKnowledgeStore.Find(TargetEntityId);
	if (KnowledgeRecord)
	{
		if (!IsPersistentKnowledgeIdentityCompatible(*KnowledgeRecord, SourceDisplayInfo)
			|| (KnowledgeRecord->bTerminalDestroyed && !bTerminalDestroyed))
		{
			return false;
		}
	}
	else
	{
		// [v1.15.0] non-terminal Detected-only 상태는 장기 Record를 만들지 않습니다.
		const bool bHasPersistentKnowledge = PublicContact
			&& (PublicContact->InformationLevel == ECFTargetInfoLevel::Identified
				|| PublicContact->InformationLevel == ECFTargetInfoLevel::DetailedScan);
		if (!bTerminalDestroyed && !bHasPersistentKnowledge)
		{
			return true;
		}

		// [v1.15.0] actor-free value만 가진 새 Entity Knowledge Record입니다.
		FCFSensorKnowledgeRecord NewKnowledgeRecord;
		NewKnowledgeRecord.TargetEntityId = TargetEntityId;
		PersistentKnowledgeStore.Add(TargetEntityId, MoveTemp(NewKnowledgeRecord));
		KnowledgeRecord = PersistentKnowledgeStore.Find(TargetEntityId);
	}

	if (!KnowledgeRecord)
	{
		return false;
	}

	FillPersistentKnowledgeIdentity(*KnowledgeRecord, SourceDisplayInfo);

	if (PublicContact
		&& (PublicContact->InformationLevel == ECFTargetInfoLevel::Identified
			|| PublicContact->InformationLevel == ECFTargetInfoLevel::DetailedScan))
	{
		if (PublicContact->InformationLevel == ECFTargetInfoLevel::DetailedScan)
		{
			KnowledgeRecord->HighestKnowledgeTier = ECFTargetInfoLevel::DetailedScan;
			KnowledgeRecord->DetailedScanAnalysisRevision = PublicContact->AnalysisCompletionRevision;
		}
		else if (KnowledgeRecord->HighestKnowledgeTier != ECFTargetInfoLevel::DetailedScan)
		{
			KnowledgeRecord->HighestKnowledgeTier = ECFTargetInfoLevel::Identified;
		}

		if (!PublicContact->KnownTargetId.IsNone())
		{
			KnowledgeRecord->KnownTargetId = PublicContact->KnownTargetId;
		}
		if (!PublicContact->KnownDisplayName.IsEmpty())
		{
			KnowledgeRecord->KnownDisplayName = PublicContact->KnownDisplayName;
		}
	}

	if (bTerminalDestroyed)
	{
		KnowledgeRecord->bTerminalDestroyed = true;
	}
	return true;
}

// [v1.11.0] Target Scan만 유지하던 현재 Contact가 다른 Detection으로 유지되지 않는 경우 LastKnown으로 전환합니다.
void UCFVehicleSensorComp::MarkCurrentScanTargetLastKnownIfNeeded()
{
	if (!bCurrentScanAttemptActive || bActiveScanRunning)
	{
		return;
	}

	// [v1.11.0] Target Scan 종료 직전 현재 Attempt Actor와 연결된 Runtime Contact 인덱스입니다.
	const int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(CurrentScanTargetActor.Get());
	if (!RuntimeContacts.IsValidIndex(RuntimeContactIndex))
	{
		return;
	}

	// [v1.11.0] Passive/Visual 관측이 아니라 Target Scan 단일 Active-range 관측만으로 Live였던 Contact입니다.
	FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
	if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Live
		&& !RuntimeContact.bBaselineDetectionValidAtLastObservation)
	{
		RuntimeContact.PublicContact.ContactState = ECFSensorContactState::LastKnown;
		RuntimeContact.bLostSnapshotPublished = false;
	}
}

// [v1.11.0] 현재 지정 Scan Attempt를 Idle/0 상태로 되돌리되 Contact Knowledge와 완료 전이 이력은 유지합니다.
void UCFVehicleSensorComp::ResetCurrentScanAttempt()
{
	bCurrentScanAttemptActive = false;
	CurrentScanAttemptRemainingSeconds = 0.0f;
	CurrentScanTargetActor.Reset();
	CurrentScanTargetContactId = NAME_None;
	CurrentScanAttemptProgress01 = 0.0f;
}

// [v1.11.0] DetailedScan 완료 전이를 정확히 한 번 기록하고 Target Scan Attempt만 즉시 Idle로 복귀시킵니다.
void UCFVehicleSensorComp::CompleteCurrentScanAttempt(
	FCFSensorContactRuntime& RuntimeContact,
	const FCFSensorConfig& SensorConfig)
{
	if (!bCurrentScanAttemptActive
		|| RuntimeContact.PublicContact.ContactId != CurrentScanTargetContactId
		|| RuntimeContact.PublicContact.InformationLevel != ECFTargetInfoLevel::DetailedScan)
	{
		return;
	}

	// [v1.10.0] HUD 등 전이 소비자가 정확히 한 번 관측할 Component 수명 단조 증가 완료 Revision입니다.
	const int32 CompletionTransitionRevision = FMath::Max(NextScanCompletionTransitionRevision, 1);
	LastScanCompletionTransitionRevision = CompletionTransitionRevision;
	NextScanCompletionTransitionRevision = CompletionTransitionRevision + 1;
	LastCompletedScanContactId = RuntimeContact.PublicContact.ContactId;

	MarkCurrentScanTargetLastKnownIfNeeded();
	ResetCurrentScanAttempt();
	SetComponentTickEnabled(
		HasConfiguredPassiveWork(SensorConfig)
		|| bActiveScanRunning
		|| !RuntimeContacts.IsEmpty());
}

// [v1.11.0] 이번 Sensor update가 소비한 Target Scan 시간을 반영하고 만료되면 Target Scan Attempt만 종료합니다.
void UCFVehicleSensorComp::AdvanceCurrentScanDuration(
	const float TargetScanTimeThisUpdate,
	const FCFSensorConfig& SensorConfig)
{
	if (!bCurrentScanAttemptActive)
	{
		return;
	}

	// [v1.11.0] 잘못된 외부 호출에서도 남은 시간을 역으로 늘리지 않을 안전한 Target Scan 소비 시간입니다.
	const float SafeTargetScanTimeThisUpdate = FMath::IsFinite(TargetScanTimeThisUpdate)
		? FMath::Max(TargetScanTimeThisUpdate, 0.0f)
		: 0.0f;
	CurrentScanAttemptRemainingSeconds = FMath::Max(
		0.0f,
		CurrentScanAttemptRemainingSeconds - SafeTargetScanTimeThisUpdate);

	if (CurrentScanAttemptRemainingSeconds > KINDA_SMALL_NUMBER)
	{
		return;
	}

	MarkCurrentScanTargetLastKnownIfNeeded();
	ResetCurrentScanAttempt();
	SetComponentTickEnabled(
		HasConfiguredPassiveWork(SensorConfig)
		|| bActiveScanRunning
		|| !RuntimeContacts.IsEmpty());
}

// [v1.11.0] 이번 Sensor update가 소비한 broad Active Detection 시간을 반영하고 만료되면 Active-only Contact를 LastKnown으로 전환합니다.
void UCFVehicleSensorComp::AdvanceActiveScanDuration(
	const float ActiveScanTimeThisUpdate,
	const FCFSensorConfig& SensorConfig)
{
	if (!bActiveScanRunning)
	{
		return;
	}

	// [v1.3.0] 잘못된 외부 호출에서도 남은 시간을 역으로 늘리지 않을 안전한 scan 소비 시간입니다.
	const float SafeActiveScanTimeThisUpdate = FMath::IsFinite(ActiveScanTimeThisUpdate)
		? FMath::Max(ActiveScanTimeThisUpdate, 0.0f)
		: 0.0f;
	ActiveScanRemainingSeconds = FMath::Max(
		0.0f,
		ActiveScanRemainingSeconds - SafeActiveScanTimeThisUpdate);

	if (ActiveScanRemainingSeconds > KINDA_SMALL_NUMBER)
	{
		return;
	}

	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	MarkActiveOnlyContactsLastKnown();
	SetComponentTickEnabled(
		HasConfiguredPassiveWork(SensorConfig)
		|| bCurrentScanAttemptActive
		|| !RuntimeContacts.IsEmpty());
}

// [v1.1.0] 같은 Actor에 이미 연결된 Runtime Contact 인덱스를 찾고 없으면 INDEX_NONE을 반환합니다.
int32 UCFVehicleSensorComp::FindRuntimeContactIndexByActor(const AActor* CandidateActor) const
{
	if (!CandidateActor)
	{
		return INDEX_NONE;
	}

	for (int32 RuntimeContactIndex = 0; RuntimeContactIndex < RuntimeContacts.Num(); ++RuntimeContactIndex)
	{
		// [v1.1.0] 현재 비교할 private Runtime Contact입니다.
		const FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
		if (RuntimeContact.TargetActor.Get() == CandidateActor)
		{
			return RuntimeContactIndex;
		}
	}

	return INDEX_NONE;
}

// [v1.3.0] Runtime Contact의 Actor-free PublicContact만 복사해 결정 정렬된 Snapshot을 게시하고 현재 Active Scan 상태를 함께 공개합니다.
void UCFVehicleSensorComp::PublishRuntimeSnapshot()
{
	CurrentSensorSnapshot = FCFSensorSnapshot();
	CurrentSensorSnapshot.Revision = NextSnapshotRevision++;
	CurrentSensorSnapshot.bRuntimeReady = bSensorRuntimeReady;
	CurrentSensorSnapshot.bActiveScanRunning = bActiveScanRunning;
	CurrentSensorSnapshot.ScanAttempt.bScanning = bCurrentScanAttemptActive;
	CurrentSensorSnapshot.ScanAttempt.TargetContactId = bCurrentScanAttemptActive ? CurrentScanTargetContactId : NAME_None;
	CurrentSensorSnapshot.ScanAttempt.Progress01 = bCurrentScanAttemptActive ? FMath::Clamp(CurrentScanAttemptProgress01, 0.0f, 1.0f) : 0.0f;
	CurrentSensorSnapshot.ScanAttempt.CompletionTransitionRevision = LastScanCompletionTransitionRevision;
	CurrentSensorSnapshot.ScanAttempt.LastCompletedContactId = LastCompletedScanContactId;

	// [v1.1.0] Snapshot 생성 시각을 제공할 현재 World입니다.
	const UWorld* CurrentWorld = GetWorld();
	CurrentSensorSnapshot.SnapshotWorldTimeSeconds = CurrentWorld
		? FMath::Max(0.0, static_cast<double>(CurrentWorld->GetTimeSeconds()))
		: 0.0;

	// [v1.1.0] Snapshot의 Sensor 원점과 전방을 제공할 Owner Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		CurrentSensorSnapshot.SensorOriginWorldLocation = OwnerActor->GetActorLocation();
		CurrentSensorSnapshot.SensorForwardWorldDirection = OwnerActor->GetActorForwardVector().GetSafeNormal();
	}

					CurrentSensorSnapshot.Contacts.Reserve(RuntimeContacts.Num());
	for (FCFSensorContactRuntime& RuntimeContact : RuntimeContacts)
	{
		CurrentSensorSnapshot.Contacts.Add(RuntimeContact.PublicContact);
		if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::Lost)
		{
			RuntimeContact.bLostSnapshotPublished = true;
		}
	}
	CurrentSensorSnapshot.SortContactsDeterministically();
}

// [v1.1.0] bounded world scan cursor와 진단값을 초기 위치로 되돌립니다.
void UCFVehicleSensorComp::ResetPassiveScanCursor()
{
	PassiveScanLevelIndex = 0;
	PassiveScanActorIndex = 0;
		PassiveScanCycleSerial = 0;
	LastPassiveScanActorCount = 0;
	LastPassiveVisibilityTraceCount = 0;
	LastActiveAnalysisTraceCount = 0;
}

// [v1.1.0] 다음 World Level로 cursor를 이동하고 끝에서 처음으로 돌아오면 cycle serial을 증가시킵니다.
void UCFVehicleSensorComp::AdvancePassiveScanLevel(const int32 LevelCount)
{
	PassiveScanActorIndex = 0;
	if (LevelCount <= 0)
	{
		PassiveScanLevelIndex = 0;
		return;
	}

	++PassiveScanLevelIndex;
	if (PassiveScanLevelIndex >= LevelCount)
	{
		PassiveScanLevelIndex = 0;
		++PassiveScanCycleSerial;
	}
}

// [v1.3.0] 현재 Owner Transform과 Runtime 준비·Active Scan 상태로 Contact가 없는 Foundation Snapshot을 다시 만듭니다.
void UCFVehicleSensorComp::RebuildFoundationSnapshot(const bool bRuntimeReady)
{
	CurrentSensorSnapshot = FCFSensorSnapshot();
	CurrentSensorSnapshot.Revision = NextSnapshotRevision++;
	CurrentSensorSnapshot.bRuntimeReady = bRuntimeReady;
	CurrentSensorSnapshot.bActiveScanRunning = bActiveScanRunning;
	CurrentSensorSnapshot.ScanAttempt.bScanning = bCurrentScanAttemptActive;
	CurrentSensorSnapshot.ScanAttempt.TargetContactId = bCurrentScanAttemptActive ? CurrentScanTargetContactId : NAME_None;
	CurrentSensorSnapshot.ScanAttempt.Progress01 = bCurrentScanAttemptActive ? FMath::Clamp(CurrentScanAttemptProgress01, 0.0f, 1.0f) : 0.0f;
	CurrentSensorSnapshot.ScanAttempt.CompletionTransitionRevision = LastScanCompletionTransitionRevision;
	CurrentSensorSnapshot.ScanAttempt.LastCompletedContactId = LastCompletedScanContactId;

	// [v1.0.0] Snapshot 생성 시점에 사용할 현재 World입니다.
	const UWorld* CurrentWorld = GetWorld();
	CurrentSensorSnapshot.SnapshotWorldTimeSeconds = CurrentWorld
		? FMath::Max(0.0, static_cast<double>(CurrentWorld->GetTimeSeconds()))
		: 0.0;

	// [v1.0.0] Sensor 기준 위치와 전방을 제공할 현재 Owner Actor입니다.
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		CurrentSensorSnapshot.SensorOriginWorldLocation = OwnerActor->GetActorLocation();
		CurrentSensorSnapshot.SensorForwardWorldDirection = OwnerActor->GetActorForwardVector().GetSafeNormal();
	}

	CurrentSensorSnapshot.Contacts.Reset();
}
