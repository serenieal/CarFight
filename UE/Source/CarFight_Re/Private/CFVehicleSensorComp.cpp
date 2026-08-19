// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.6.0
// Date: 2026-08-16
// Description: CF-FQ-037 차량 Scanner Runtime Config Apply / SCAN-P0-02 구현
// Scope: Config fallback, non-destructive SensorData apply, private Runtime Contact, bounded Passive/Active Detection, Contact lifetime, DestroyedHold, Tactical Analysis와 Actor-free Snapshot 게시를 구현합니다.
// Changelog:
// - v1.6.0: ApplySensorData, 적용 Config 사본, 탐지 성능 감소 lifecycle reconcile과 Active Scan remaining clamp를 구현.
// - v1.5.0: 외부 adapter가 선택 Actor를 Snapshot ContactId에만 연결할 수 있는 read-only Actor→ContactId bridge를 추가.
// - v1.4.0: VehicleHealthComp OnVehicleDestroyed/IsDestroyed 기반 DestroyedHold, 독립 보존 타이머, 이벤트 binding 정리와 weak-invalid 비파괴 계약을 구현.
// - v1.3.0: 입력과 분리된 Active Scan 시작/중단, Active range 장거리 전방향 Detection, Analysis gain/decay와 Identified/DetailedScan Knowledge 승격을 구현.
// - v1.2.0: 매 Sensor update Contact freshness 갱신, Live→LastKnown→Lost→Removed, Lost 1회 게시와 Lost 전 재획득 보존을 구현.
// - v1.1.0: MaxActorScansPerUpdate 상한의 공정한 Level Actor cursor, self 제외, Passive range, ECC_Visibility visual fallback과 Actor 중복 방지를 구현.
// - v1.0.0: SEN-P0-01 Sensor Component Foundation을 최초 구현.
// Migration:
// - Sensor는 InputAction을 직접 소유하지 않고 StartActiveScan/StopActiveScan 명령 API만 제공합니다. 실제 입력 owner 연결은 후속 공용 Pawn 통합 책임입니다.
// - ActiveScanRangeCm은 실행 중에만 Contact Detection에 사용하며 Active Scan 자체는 전방향입니다. Tactical Analysis gain에는 Active range와 ECC_Visibility 직접 가시가 모두 필요합니다.
// - Analysis가 비유효하면 진행률은 AnalysisDecayPerSec로 서서히 감소하며 즉시 reset하지 않습니다. 획득한 InformationLevel/Known Target Knowledge는 강등하지 않습니다.
// - Identified 이상 승격 때만 private Source Metadata의 TargetId/DisplayName을 공개하고 Source InformationLevel은 Sensor Knowledge로 복사하지 않습니다.
// - P0-03 ContactId/lifecycle/reacquire와 P0-04 Active Scan/Analysis 의미를 보존합니다.
// - 파괴 확정은 VehicleHealthComp의 OnVehicleDestroyed 이벤트와 IsDestroyed 상태만 사용하며 Actor Destroy/weak invalid를 파괴로 추정하지 않습니다.
// - DestroyedHold는 마지막 신뢰 위치, ContactId, Knowledge와 AnalysisProgress를 유지합니다.
// - v1.5.0의 Actor→ContactId bridge는 Snapshot의 Player-facing data를 우회하지 않으며 TargetSelect 선택 의미를 변경하지 않습니다.
// - v1.6.0부터 Runtime Ready 상태는 AppliedSensorConfig 사본을 소비하며 Source UObject의 후속 값 변경만으로 Runtime 의미가 바뀌지 않습니다.
// - ApplySensorData는 invalid explicit Source를 원자적으로 거부하고, null은 검증된 Fallback Source로 적용합니다.
// - 탐지 능력 감소는 Contact를 삭제하지 않고 LastKnown으로 넘기며 Active Scan은 새 장비 적용으로 남은 시간이 늘어나지 않습니다.

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
		|| (!HasConfiguredDetectionWork(SensorConfig) && RuntimeContacts.IsEmpty()))
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
	RunPassiveDetectionUpdate(SensorUpdateDeltaSeconds);
}

// [v1.0.0] Actor 수명이 종료될 때 Sensor Snapshot과 준비 상태를 안전하게 비웁니다.
void UCFVehicleSensorComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetSensorRuntime();
	Super::EndPlay(EndPlayReason);
}

// [v1.3.0] 유효한 Config로 Runtime을 초기화하고 Passive/Visual 작업이 있으면 기본 Sensor update를 활성화합니다.
bool UCFVehicleSensorComp::InitializeSensorRuntime()
{
		// [v1.6.0] 현재 Source UObject 또는 Fallback에서 초기화 시점에 고정할 실제 Runtime 설정 사본입니다.
	const FCFSensorConfig ResolvedConfig = ResolveConfiguredSensorConfig();

	UnbindAllContactDestroyedEvents();
	RuntimeContacts.Reset();
	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	PassiveUpdateElapsedSeconds = 0.0f;
	ResetPassiveScanCursor();

		if (!ResolvedConfig.IsValid())
	{
		bHasAppliedSensorConfig = false;
		AppliedSensorConfig = FCFSensorConfig();
		bSensorRuntimeReady = false;
		SetComponentTickEnabled(false);
		RebuildFoundationSnapshot(false);
		LastSensorRuntimeSummary = TEXT("SensorRuntime: InvalidConfig");
		return false;
	}

		AppliedSensorConfig = ResolvedConfig;
	bHasAppliedSensorConfig = true;
	bSensorRuntimeReady = true;
	RebuildFoundationSnapshot(true);

	// [v1.3.0] Active Scan 요청이 없어도 상시 수행해야 하는 Passive/Visual Detection 작업 여부입니다.
	const bool bPassiveUpdatesEnabled = HasConfiguredPassiveWork(ResolvedConfig);
	SetComponentTickEnabled(bPassiveUpdatesEnabled);

	// [v1.0.0] 실제 설정이 SensorData에서 왔는지 Fallback에서 왔는지 표시할 출처 문자열입니다.
	const TCHAR* ConfigSource = IsValid(SensorData) && SensorData->IsSensorConfigValid()
		? TEXT("SensorData")
		: TEXT("Fallback");
	LastSensorRuntimeSummary = FString::Printf(
		TEXT("SensorRuntime: Ready Source=%s PassiveUpdates=%s ActiveReady=%s Revision=%d | %s"),
		ConfigSource,
		bPassiveUpdatesEnabled ? TEXT("True") : TEXT("False"),
		(ResolvedConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER && ResolvedConfig.ActiveScanDurationSec > KINDA_SMALL_NUMBER) ? TEXT("True") : TEXT("False"),
		CurrentSensorSnapshot.Revision,
		*ResolvedConfig.BuildDebugSummary());
		return true;
}

// [v1.6.0] 새 SensorData 또는 scanner-less Fallback Source를 기존 Contact/Knowledge를 지우지 않고 적용합니다.
bool UCFVehicleSensorComp::ApplySensorData(UCFVehicleSensorData* NewSensorData)
{
	if (NewSensorData != nullptr
		&& (!IsValid(NewSensorData) || !NewSensorData->IsSensorConfigValid()))
	{
		return false;
	}

	// [v1.6.0] 요청된 SensorData가 null이면 scanner-less Fallback을 사용하는 적용 후보 Config입니다.
	const FCFSensorConfig RequestedSensorConfig = IsValid(NewSensorData)
		? NewSensorData->SensorConfig
		: FallbackSensorConfig;
	if (!RequestedSensorConfig.IsValid())
	{
		return false;
	}

	// [v1.6.0] 초기화 전 Source 선택은 Runtime을 암묵적으로 시작하지 않고 다음 InitializeSensorRuntime의 입력만 교체합니다.
	if (!bSensorRuntimeReady)
	{
		SensorData = NewSensorData;
		bHasAppliedSensorConfig = false;
		AppliedSensorConfig = FCFSensorConfig();

		// [v1.6.0] 초기화 전 적용 결과를 구분해 표시할 Config Source 문자열입니다.
		const TCHAR* ConfigSource = IsValid(NewSensorData) ? TEXT("SensorData") : TEXT("Fallback");
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

	// [v1.6.0] 새 Config가 Active Scan을 계속 실행할 최소 거리와 지속 시간을 모두 제공하는지 여부입니다.
	const bool bActiveScanSupported = RequestedSensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER
		&& RequestedSensorConfig.ActiveScanDurationSec > KINDA_SMALL_NUMBER;
	if (bActiveScanRunning)
	{
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
		|| !RuntimeContacts.IsEmpty());
	PublishRuntimeSnapshot();

	// [v1.6.0] hot reapply 결과를 구분해 표시할 Config Source 문자열입니다.
	const TCHAR* ConfigSource = IsValid(NewSensorData) ? TEXT("SensorData") : TEXT("Fallback");
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

// [v1.3.0] Runtime Contact, Active Scan, scan cursor와 공개 Snapshot을 초기화하고 Tick을 비활성화합니다.
void UCFVehicleSensorComp::ResetSensorRuntime()
{
	SetComponentTickEnabled(false);
		bSensorRuntimeReady = false;
	bHasAppliedSensorConfig = false;
	AppliedSensorConfig = FCFSensorConfig();
	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	UnbindAllContactDestroyedEvents();
	RuntimeContacts.Reset();
	PassiveUpdateElapsedSeconds = 0.0f;
	ResetPassiveScanCursor();
	RebuildFoundationSnapshot(false);
	LastSensorRuntimeSummary = FString::Printf(
		TEXT("SensorRuntime: Reset Revision=%d"),
		CurrentSensorSnapshot.Revision);
}

// [v1.3.0] 현재 Config의 Active Scan을 한 번 시작하고 성공 여부를 반환합니다.
bool UCFVehicleSensorComp::StartActiveScan()
{
	if (!bSensorRuntimeReady || bActiveScanRunning)
	{
		return false;
	}

	// [v1.3.0] Active Scan 시작 조건과 실행 시간을 제공할 현재 Sensor 설정입니다.
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

// [v1.3.0] 실행 중인 Active Scan을 중단하고 실제 중단이 발생했는지 반환합니다.
bool UCFVehicleSensorComp::StopActiveScan()
{
	if (!bActiveScanRunning)
	{
		return false;
	}

	bActiveScanRunning = false;
	ActiveScanRemainingSeconds = 0.0f;
	PassiveUpdateElapsedSeconds = 0.0f;
	MarkActiveOnlyContactsLastKnown();

	// [v1.3.0] Active Scan 종료 뒤에도 Passive/Visual Detection 또는 남은 Contact lifetime/decay 작업이 필요한지 여부입니다.
	const FCFSensorConfig SensorConfig = GetResolvedSensorConfig();
	SetComponentTickEnabled(
		SensorConfig.IsValid()
		&& (HasConfiguredPassiveWork(SensorConfig) || !RuntimeContacts.IsEmpty()));
	PublishRuntimeSnapshot();
	return true;
}

// [v1.6.0] Runtime Ready이면 실제 적용된 Sensor Config를, 초기화 전이면 현재 Source 또는 Fallback 설정을 반환합니다.
FCFSensorConfig UCFVehicleSensorComp::GetResolvedSensorConfig() const
{
	return bHasAppliedSensorConfig
		? AppliedSensorConfig
		: ResolveConfiguredSensorConfig();
}

// [v1.6.0] 현재 SensorData가 유효하면 해당 설정을, 아니면 FallbackSensorConfig를 반환하는 Source 해석 전용 함수입니다.
FCFSensorConfig UCFVehicleSensorComp::ResolveConfiguredSensorConfig() const
{
	if (IsValid(SensorData) && SensorData->IsSensorConfigValid())
	{
		return SensorData->SensorConfig;
	}

	return FallbackSensorConfig;
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

// [v1.3.0] 현재 Active Scan 실행 상태까지 포함해 이번 update에 bounded World Detection이 필요한지 반환합니다.
bool UCFVehicleSensorComp::HasConfiguredDetectionWork(const FCFSensorConfig& SensorConfig) const
{
	return HasConfiguredPassiveWork(SensorConfig)
		|| (bActiveScanRunning && SensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER);
}

// [v1.3.0] 현재 cursor부터 bounded Actor 슬롯을 검사하고 Contact lifetime, Detection, Analysis와 Snapshot을 한 번 갱신합니다.
void UCFVehicleSensorComp::RunPassiveDetectionUpdate(const float SensorUpdateDeltaSeconds)
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

	// [v1.3.0] 이번 update 구간 중 Active Scan이 실제로 유효한 시간입니다. scan duration 끝을 넘겨 gain하지 않습니다.
	const float ActiveScanTimeThisUpdate = bActiveScanRunning
		? FMath::Min(FMath::Max(ActiveScanRemainingSeconds, 0.0f), ResolvedSensorUpdateDeltaSeconds)
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

	AdvanceContactAnalysis(
		ResolvedSensorUpdateDeltaSeconds,
		ActiveScanTimeThisUpdate,
		SensorConfig);
	AdvanceActiveScanDuration(ActiveScanTimeThisUpdate, SensorConfig);
	PublishRuntimeSnapshot();

	if (!HasConfiguredDetectionWork(SensorConfig) && RuntimeContacts.IsEmpty())
	{
		SetComponentTickEnabled(false);
	}
}

// [v1.3.0] 하나의 Actor를 Sensor 자격·대표 위치·Passive/Visual/Active 거리 순서로 평가하고 Runtime Contact를 갱신합니다.
bool UCFVehicleSensorComp::ProcessPassiveScanActor(AActor* CandidateActor, const FCFSensorConfig& SensorConfig)
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

	// [v1.3.0] Active Scan 실행 중에만 적용되는 장거리 전방향 Contact Detection 결과입니다. LOS는 Contact 생성 조건이 아닙니다.
	const bool bActiveScanDetected = bActiveScanRunning
		&& SensorConfig.ActiveScanRangeCm > KINDA_SMALL_NUMBER
		&& DistanceToTargetCm <= SensorConfig.ActiveScanRangeCm;

	if (!bBaselineDetected && !bActiveScanDetected)
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

// [v1.3.0] 같은 Actor Runtime Contact를 갱신하거나 새 ContactId를 한 번만 발급하고 마지막 관측이 baseline 탐지로 유지 가능한지도 기록합니다.
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

	// [v1.1.0] 같은 Actor에 이미 존재하는 Runtime Contact 인덱스입니다.
	int32 RuntimeContactIndex = FindRuntimeContactIndexByActor(CandidateActor);
	if (RuntimeContactIndex == INDEX_NONE)
	{
		// [v1.1.0] 동일 Actor에 최초 탐지 때만 추가하는 새 private Runtime Contact입니다.
		FCFSensorContactRuntime NewRuntimeContact;
		NewRuntimeContact.TargetActor = CandidateActor;
		NewRuntimeContact.SourceDisplayInfo = SourceDisplayInfo;
		NewRuntimeContact.PublicContact.ContactId = AllocateContactId();
		NewRuntimeContact.PublicContact.TargetCategory = SourceDisplayInfo.TargetCategory;
		NewRuntimeContact.PublicContact.Relation = SourceDisplayInfo.Relation;
		NewRuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::Detected;
		RuntimeContactIndex = RuntimeContacts.Add(MoveTemp(NewRuntimeContact));
	}

		// [v1.3.0] 새로 생성했거나 기존 Actor에서 찾아낸 유일한 Runtime Contact입니다.
	FCFSensorContactRuntime& RuntimeContact = RuntimeContacts[RuntimeContactIndex];
	if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::DestroyedHold)
	{
		return false;
	}

	RuntimeContact.TargetActor = CandidateActor;
	RuntimeContact.SourceDisplayInfo = SourceDisplayInfo;
	RuntimeContact.bLostSnapshotPublished = false;
	RuntimeContact.bBaselineDetectionValidAtLastObservation = bBaselineDetectionValidAtObservation;
	BindContactDestroyedEvent(RuntimeContact);

	// [v1.4.0] event binding 직전 이미 파괴가 확정된 극단적 순서에서도 기존 Contact를 Live로 되돌리지 않는 상태 확인입니다.
	UCFVehicleHealthComp* BoundHealthComponent = RuntimeContact.BoundVehicleHealthComponent.Get();
	if (IsValid(BoundHealthComponent) && BoundHealthComponent->IsDestroyed())
	{
		return ConfirmDestroyedContactForActor(CandidateActor);
	}

	// [v1.1.0] 외부 Actor 포인터 없이 갱신할 공개 Contact 사본 참조입니다.
	FCFSensorContact& PublicContact = RuntimeContact.PublicContact;
	PublicContact.TargetCategory = SourceDisplayInfo.TargetCategory;
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

// [v1.4.0] authoritative VehicleHealth가 파괴 상태인 기존 Contact만 DestroyedHold로 전환합니다.
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

	// [v1.4.0] 새 파괴 Contact를 생성하지 않고 이미 Sensor가 알고 있던 Contact만 찾습니다.
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

// [v1.3.0] 모든 Runtime Contact의 Tactical Analysis를 active-valid 시간에는 증가시키고 그 외 시간에는 설정된 감소율로 서서히 감소시킵니다.
void UCFVehicleSensorComp::AdvanceContactAnalysis(
	const float SensorUpdateDeltaSeconds,
	const float ActiveScanTimeThisUpdate,
	const FCFSensorConfig& SensorConfig)
{
	if (!SensorConfig.IsValid()
		|| !FMath::IsFinite(SensorUpdateDeltaSeconds)
		|| SensorUpdateDeltaSeconds < 0.0f
		|| !FMath::IsFinite(ActiveScanTimeThisUpdate)
		|| ActiveScanTimeThisUpdate < 0.0f)
	{
		return;
	}

	// [v1.3.0] 이번 Sensor update가 실제로 대표하는 유한한 경과 시간입니다.
	const float SafeSensorUpdateDeltaSeconds = FMath::Max(SensorUpdateDeltaSeconds, 0.0f);

	// [v1.3.0] update 시간보다 길게 gain하지 않도록 제한한 실제 Active Scan 유효 시간입니다.
	const float SafeActiveScanTimeThisUpdate = FMath::Clamp(
		ActiveScanTimeThisUpdate,
		0.0f,
		SafeSensorUpdateDeltaSeconds);

	// [v1.3.0] Active Scan이 update 도중 만료된 경우 같은 update 후반에 decay로 반영할 비활성 시간입니다.
	const float InactiveTimeThisUpdate = FMath::Max(
		0.0f,
		SafeSensorUpdateDeltaSeconds - SafeActiveScanTimeThisUpdate);

	for (FCFSensorContactRuntime& RuntimeContact : RuntimeContacts)
	{
		// [v1.3.0] 비정상 값이 유입돼도 공개 계약을 깨지 않도록 정규화한 기존 Analysis progress입니다.
		const float CurrentAnalysisProgress = FMath::IsFinite(RuntimeContact.PublicContact.AnalysisProgress01)
			? FMath::Clamp(RuntimeContact.PublicContact.AnalysisProgress01, 0.0f, 1.0f)
			: 0.0f;
		RuntimeContact.PublicContact.AnalysisProgress01 = CurrentAnalysisProgress;

		if (RuntimeContact.PublicContact.ContactState == ECFSensorContactState::DestroyedHold)
		{
			continue;
		}

		// [v1.3.0] 이번 Active Scan 구간에서 해당 Contact가 실제 Tactical Analysis gain 조건을 만족하는지 여부입니다.
		const bool bAnalysisValid = SafeActiveScanTimeThisUpdate > KINDA_SMALL_NUMBER
			&& IsContactValidForActiveAnalysis(RuntimeContact, SensorConfig);

		if (bAnalysisValid)
		{
			// [v1.3.0] 유효 분석 구간 동안 누적할 progress 증가량입니다.
			const float AnalysisGain = SensorConfig.AnalysisGainPerSec * SafeActiveScanTimeThisUpdate;
			RuntimeContact.PublicContact.AnalysisProgress01 = FMath::Clamp(
				RuntimeContact.PublicContact.AnalysisProgress01 + AnalysisGain,
				0.0f,
				1.0f);
			PromoteContactKnowledgeFromAnalysis(RuntimeContact, SensorConfig);

			if (InactiveTimeThisUpdate > KINDA_SMALL_NUMBER)
			{
				// [v1.3.0] scan이 update 도중 끝난 뒤 남은 시간에만 적용할 서서히 감소하는 progress 양입니다.
				const float AnalysisDecay = SensorConfig.AnalysisDecayPerSec * InactiveTimeThisUpdate;
				RuntimeContact.PublicContact.AnalysisProgress01 = FMath::Clamp(
					RuntimeContact.PublicContact.AnalysisProgress01 - AnalysisDecay,
					0.0f,
					1.0f);
			}
		}
		else if (SafeSensorUpdateDeltaSeconds > KINDA_SMALL_NUMBER)
		{
			// [v1.3.0] 가림·범위 이탈·LastKnown·Active Scan 중단 상태에서 즉시 reset하지 않고 적용할 전체 update decay 양입니다.
			const float AnalysisDecay = SensorConfig.AnalysisDecayPerSec * SafeSensorUpdateDeltaSeconds;
			RuntimeContact.PublicContact.AnalysisProgress01 = FMath::Clamp(
				RuntimeContact.PublicContact.AnalysisProgress01 - AnalysisDecay,
				0.0f,
				1.0f);
		}
	}
}

// [v1.3.0] 현재 Runtime Contact가 Active Tactical Analysis 증가 조건을 만족하는지 판정합니다.
bool UCFVehicleSensorComp::IsContactValidForActiveAnalysis(
	FCFSensorContactRuntime& RuntimeContact,
	const FCFSensorConfig& SensorConfig)
{
	if (!bActiveScanRunning
		|| SensorConfig.ActiveScanRangeCm <= KINDA_SMALL_NUMBER
		|| RuntimeContact.PublicContact.ContactState != ECFSensorContactState::Live)
	{
		return false;
	}

	// [v1.3.0] 분석 대상 자격·거리·가시성을 실제 Actor에서 확인할 private weak Actor입니다.
	AActor* TargetActor = RuntimeContact.TargetActor.Get();
	if (!IsValid(TargetActor) || !IsSensorCandidateEligible(TargetActor))
	{
		return false;
	}

	// [v1.3.0] Active Analysis 거리와 LOS의 끝점으로 사용할 TargetSelectable 대표 위치입니다.
	const FVector TargetWorldLocation = ResolveSensorTargetLocation(TargetActor);
	if (!IsFiniteSensorVector(TargetWorldLocation))
	{
		return false;
	}

	// [v1.3.0] Active Analysis 기준 원점을 소유한 Sensor Owner입니다.
	const AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	// [v1.3.0] 현재 실제 위치 기준 Active Scan 분석 거리입니다. LastKnown 위치를 현재 위치처럼 사용하지 않습니다.
	const float DistanceToTargetCm = FVector::Distance(
		OwnerActor->GetActorLocation(),
		TargetWorldLocation);
	if (!FMath::IsFinite(DistanceToTargetCm)
		|| DistanceToTargetCm > SensorConfig.ActiveScanRangeCm)
	{
		return false;
	}

	return HasDirectSensorVisibility(TargetActor, TargetWorldLocation, true);
}

// [v1.3.0] 분석 임계값을 통과한 Contact의 Sensor Knowledge를 단방향으로 승격하고 Identified 이상에서만 TargetId/Name을 공개합니다.
void UCFVehicleSensorComp::PromoteContactKnowledgeFromAnalysis(
	FCFSensorContactRuntime& RuntimeContact,
	const FCFSensorConfig& SensorConfig)
{
	if (RuntimeContact.SourceDisplayInfo.TargetId.IsNone())
	{
		return;
	}

	// [v1.3.0] Sensor가 직접 획득한 현재 Tactical Analysis progress입니다.
	const float AnalysisProgress = RuntimeContact.PublicContact.AnalysisProgress01;

	if (AnalysisProgress >= SensorConfig.DetailedScanThreshold)
	{
		RuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::DetailedScan;
		RuntimeContact.PublicContact.KnownTargetId = RuntimeContact.SourceDisplayInfo.TargetId;
		RuntimeContact.PublicContact.KnownDisplayName = RuntimeContact.SourceDisplayInfo.DisplayName;
		return;
	}

	if (AnalysisProgress >= SensorConfig.IdentifiedThreshold
		&& (RuntimeContact.PublicContact.InformationLevel == ECFTargetInfoLevel::None
			|| RuntimeContact.PublicContact.InformationLevel == ECFTargetInfoLevel::Detected))
	{
		RuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::Identified;
		RuntimeContact.PublicContact.KnownTargetId = RuntimeContact.SourceDisplayInfo.TargetId;
		RuntimeContact.PublicContact.KnownDisplayName = RuntimeContact.SourceDisplayInfo.DisplayName;
	}
}

// [v1.3.0] 이번 Sensor update가 소비한 Active Scan 시간을 반영하고 만료되면 Active-only Contact를 LastKnown으로 전환합니다.
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
