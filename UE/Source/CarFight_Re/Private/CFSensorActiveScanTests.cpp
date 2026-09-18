// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-09-17
// Description: Active Detection / Target Scan 독립 Runtime state 회귀
// Scope: broad Active Detection, 단일 Target Scan Attempt gain/decay, 두 Operation 독립 실행/만료, Knowledge 승격과 canonical Basic Sensor 1회 완료를 검증합니다.
// Changelog:
// - v1.4.0: Targeted-only Scan이 broad Active Detection을 켜거나 주변 Contact를 탐지하지 않는지, broad Pulse와 Target Scan 동시 실행 및 broad 만료 뒤 Target Scan 독립 지속을 검증하는 OperationSplit 회귀를 추가.
// - v1.3.0: 분석 테스트를 StartTargetedScan + Snapshot.ScanAttempt authority로 전환하고 DetailedScan 완료 즉시 Scanner Idle/Attempt 0, Contact Knowledge 유지, 같은 완료 대상 즉시 재스캔 거부를 검증.
// - v1.2.0: DetailedScan 최초 완료 Revision 발급과 scan 중단 후 raw progress decay에서도 같은 Revision/공개 계약이 유지되는 회귀를 추가.
// - v1.1.0: 실제 DA_VehicleSensor_Basic을 읽어 정상 LOS/범위에서 Active Scan 1회로 DetailedScan 100%에 도달하고 종료 후 Knowledge가 유지되는 회귀를 추가.
// - v1.0.0: ActiveRange, AnalysisProgress, AnalysisDecay 3개 Automation을 최초 추가.
// Migration:
// - v1.4.0부터 Snapshot.bActiveScanRunning/IsActiveScanRunning은 broad Active Detection만 검증하고 지정 Target Scan 상태는 Snapshot.ScanAttempt/IsTargetedScanRunning으로 독립 검증합니다.
// - v1.3.0부터 분석 진행률은 Contact.AnalysisProgress01이 아니라 Snapshot.ScanAttempt.Progress01을 검증합니다. Contact의 Legacy progress decay 회귀는 폐기합니다.
// - v1.3.0 StartActiveScan은 broad detection 전용 회귀에만 사용하고 Target Knowledge 분석 테스트는 StartTargetedScan(TargetActor)을 사용합니다.
// - v1.2.0부터 DetailedScan 회귀는 완료 Revision이 최초 발급되는지 검증하며 v1.3.0부터 완료 뒤 Attempt 자체는 즉시 종료/0으로 복귀합니다.
// - v1.1.0의 Basic Sensor 회귀는 canonical Content Asset을 read-only로 로드하며 생성·수정·저장하지 않습니다.
// - 기존 transient Editor World 테스트는 C++ ACFMissileTestTarget만 사용하며 프로젝트 Content Asset을 생성·수정·저장하지 않습니다.
// - Sensor InputAction은 생성하지 않고 StartActiveScan/StartTargetedScan/StopActiveScan Runtime API를 직접 호출합니다.
// - Active Detection은 LOS 없이 허용하지만 Tactical Analysis gain은 ECC_Visibility 직접 가시 조건을 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileTestTarget.h"
#include "CFVehicleSensorComp.h"
#include "CFVehicleSensorData.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] 이름·위치·TargetId가 명시된 TargetSelectable C++ 테스트 Actor를 transient World에 생성합니다.
	ACFMissileTestTarget* SpawnActiveScanTarget(
		UWorld* TestWorld,
		const FName ActorName,
		const FVector& ActorLocation,
		const FName TargetId)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		// [v1.0.0] 테스트마다 결정적인 Actor 이름을 사용하기 위한 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		// [v1.0.0] Sensor Active Scan과 Knowledge 획득에 사용할 실제 C++ TargetSelectable입니다.
		ACFMissileTestTarget* TargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>(
			ACFMissileTestTarget::StaticClass(),
			ActorLocation,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (TargetActor)
		{
			TargetActor->TargetId = TargetId;
			TargetActor->TargetDisplayName = FText::FromName(TargetId);
			TargetActor->bAutoDestroy = false;
			TargetActor->RefreshTestTargetConfig();
		}

		return TargetActor;
	}

	// [v1.0.0] plain Actor Owner에 transient Sensor Component를 붙이고 P0-04 Active Scan 테스트 Config로 초기화합니다.
	UCFVehicleSensorComp* AttachActiveScanSensor(
		AActor* SensorOwner,
		const float ActiveScanDurationSec,
		const float AnalysisGainPerSec,
		const float AnalysisDecayPerSec,
		const float IdentifiedThreshold,
		const float DetailedScanThreshold)
	{
		if (!SensorOwner)
		{
			return nullptr;
		}

		// [v1.0.0] Content Asset 없이 실제 World Runtime을 사용하는 테스트 Sensor Component입니다.
		UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(SensorOwner, TEXT("VehicleSensorComp_ActiveScanAutomation"));
		if (!SensorComponent)
		{
			return nullptr;
		}

		SensorOwner->AddInstanceComponent(SensorComponent);
		SensorComponent->RegisterComponent();
		SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = 1000.0f;
		SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 5000.0f;
		SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
		SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = 256;
		SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = 10.0f;
		SensorComponent->FallbackSensorConfig.DestroyedHoldTimeSec = 0.0f;
		SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = ActiveScanDurationSec;
		SensorComponent->FallbackSensorConfig.AnalysisGainPerSec = AnalysisGainPerSec;
		SensorComponent->FallbackSensorConfig.AnalysisDecayPerSec = AnalysisDecayPerSec;
		SensorComponent->FallbackSensorConfig.IdentifiedThreshold = IdentifiedThreshold;
		SensorComponent->FallbackSensorConfig.DetailedScanThreshold = DetailedScanThreshold;
		return SensorComponent->InitializeSensorRuntime() ? SensorComponent : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorActiveRangeTest,
	"CarFight.Sensor.SEN_P0_04.ActiveRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] ActiveScanRangeCm이 실행 중에만 장거리 전방향 Detection에 적용되고 종료 뒤 active-only Contact가 LastKnown이 되는지 검증합니다.
bool FCFSensorActiveRangeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Active range와 scan duration을 실제 Actor 위치로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Active Range 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Sensor 원점이 될 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] Passive 1,000cm 안이라 Active Scan과 무관하게 계속 Live여야 할 대상입니다.
	ACFMissileTestTarget* PassiveTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorActivePassiveTarget"),
		FVector(500.0f, 0.0f, 0.0f),
		TEXT("ActivePassiveTarget"));

	// [v1.0.0] 차량 뒤쪽 3,000cm에 있어 Active Scan의 전방향 장거리 계약으로만 탐지될 대상입니다.
	ACFMissileTestTarget* ActiveBehindTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorActiveBehindTarget"),
		FVector(-3000.0f, 0.0f, 0.0f),
		TEXT("ActiveBehindTarget"));

	// [v1.0.0] Active 5,000cm 밖이라 실행 중에도 탐지되지 않아야 할 대상입니다.
	ACFMissileTestTarget* OutsideActiveTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorActiveOutsideTarget"),
		FVector(7000.0f, 0.0f, 0.0f),
		TEXT("ActiveOutsideTarget"));
	if (!TestNotNull(TEXT("Active Range Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Passive Target 생성"), PassiveTarget)
		|| !TestNotNull(TEXT("Active 뒤쪽 Target 생성"), ActiveBehindTarget)
		|| !TestNotNull(TEXT("Active 범위 밖 Target 생성"), OutsideActiveTarget))
	{
		return false;
	}

	// [v1.0.0] 0.2초 Active Scan으로 자동 종료까지 두 update에 검증할 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachActiveScanSensor(
		SensorOwner,
		0.2f,
		0.0f,
		0.0f,
		0.5f,
		1.0f);
	if (!TestNotNull(TEXT("Active Range Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestTrue(TEXT("Active 시작 전 Passive Target 탐지"), SensorComponent->FindRuntimeContactIndexByActor(PassiveTarget) != INDEX_NONE);
	TestEqual(TEXT("Active 시작 전 뒤쪽 장거리 Target 미탐지"), SensorComponent->FindRuntimeContactIndexByActor(ActiveBehindTarget), INDEX_NONE);
	TestEqual(TEXT("Active 시작 전 Active 범위 밖 Target 미탐지"), SensorComponent->FindRuntimeContactIndexByActor(OutsideActiveTarget), INDEX_NONE);

	TestTrue(TEXT("유효 Config에서 Active Scan 시작 성공"), SensorComponent->StartActiveScan());
	TestTrue(TEXT("Active Scan Runtime 상태 True"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("시작 직후 Snapshot Active 상태 True"), SensorComponent->GetSensorSnapshot().bActiveScanRunning);
	TestFalse(TEXT("실행 중 중복 Active Scan 시작 거부"), SensorComponent->StartActiveScan());

	SensorComponent->RunPassiveDetectionUpdate(0.1f);

	// [v1.0.0] 첫 Active update에서 새로 생성된 뒤쪽 장거리 Contact 인덱스입니다.
	const int32 ActiveBehindContactIndex = SensorComponent->FindRuntimeContactIndexByActor(ActiveBehindTarget);
	TestTrue(TEXT("차량 뒤쪽 장거리 Target도 Active Scan 전방향 탐지"), ActiveBehindContactIndex != INDEX_NONE);
	TestEqual(TEXT("Active 범위 밖 Target은 실행 중에도 미탐지"), SensorComponent->FindRuntimeContactIndexByActor(OutsideActiveTarget), INDEX_NONE);
	TestTrue(TEXT("첫 Active update 뒤 아직 실행 중"), SensorComponent->IsActiveScanRunning());
	if (ActiveBehindContactIndex != INDEX_NONE)
	{
		TestFalse(
			TEXT("뒤쪽 장거리 Contact는 Active-only 관측으로 기록"),
			SensorComponent->RuntimeContacts[ActiveBehindContactIndex].bBaselineDetectionValidAtLastObservation);
		TestEqual(
			TEXT("Active-only Contact도 Source Identified를 직접 복사하지 않고 Detected"),
			SensorComponent->RuntimeContacts[ActiveBehindContactIndex].PublicContact.InformationLevel,
			ECFTargetInfoLevel::Detected);
	}

	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestFalse(TEXT("ActiveScanDuration 0.2초 자동 만료"), SensorComponent->IsActiveScanRunning());
	TestFalse(TEXT("자동 만료 Snapshot Active 상태 False"), SensorComponent->GetSensorSnapshot().bActiveScanRunning);
	TestFalse(TEXT("자동 만료 뒤 StopActiveScan은 중복 중단 거부"), SensorComponent->StopActiveScan());

	// [v1.0.0] 자동 종료 뒤 Passive Target 상태를 확인할 Runtime Contact 인덱스입니다.
	const int32 PassiveContactIndex = SensorComponent->FindRuntimeContactIndexByActor(PassiveTarget);

	// [v1.0.0] 자동 종료 뒤 Active-only Target 상태를 확인할 Runtime Contact 인덱스입니다.
	const int32 ExpiredActiveContactIndex = SensorComponent->FindRuntimeContactIndexByActor(ActiveBehindTarget);
	if (PassiveContactIndex != INDEX_NONE)
	{
		TestEqual(
			TEXT("Active 종료 뒤 baseline Passive Contact는 Live 유지"),
			SensorComponent->RuntimeContacts[PassiveContactIndex].PublicContact.ContactState,
			ECFSensorContactState::Live);
	}
	if (ExpiredActiveContactIndex != INDEX_NONE)
	{
		TestEqual(
			TEXT("Active 종료 뒤 active-only Contact는 LastKnown"),
			SensorComponent->RuntimeContacts[ExpiredActiveContactIndex].PublicContact.ContactState,
			ECFSensorContactState::LastKnown);
	}
	TestTrue(TEXT("Active Range Snapshot 공개 계약 유효"), SensorComponent->GetSensorSnapshot().IsPublicContractValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorOpSplitTest,
	"CarFight.Sensor.SEN_P0_04.OperationSplit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.4.0] Target Scan 단독 실행이 주변 broad Detection을 만들지 않고 broad Pulse와 동시 실행한 뒤 한쪽 만료가 다른 쪽을 종료하지 않는지 검증합니다.
bool FCFSensorOpSplitTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] 두 Sensor Operation의 독립 상태를 실제 World Actor로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Operation Split 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.4.0] Sensor 원점이 될 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.4.0] Target Scan이 단독으로 관측할 Passive 밖·Active 안 지정 대상입니다.
	ACFMissileTestTarget* ScanTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorOpSplitScanTarget"),
		FVector(3000.0f, 0.0f, 0.0f),
		TEXT("OpSplitScanTarget"));

	// [v1.4.0] Target Scan 단독 실행에서는 탐지되지 않고 broad Active Detection에서만 발견돼야 할 주변 대상입니다.
	ACFMissileTestTarget* NearbyTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorOpSplitNearbyTarget"),
		FVector(-3000.0f, 0.0f, 0.0f),
		TEXT("OpSplitNearbyTarget"));
	if (!TestNotNull(TEXT("Operation Split Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Operation Split Scan Target 생성"), ScanTarget)
		|| !TestNotNull(TEXT("Operation Split Nearby Target 생성"), NearbyTarget))
	{
		return false;
	}

	// [v1.4.0] broad Pulse를 먼저 일부 소비한 뒤 Target Scan을 시작해 두 독립 만료 시점을 만들 Sensor입니다. gain 4.0은 0.4초 단발 완료 예산 계약을 만족하면서 0.2초 동시 실행 구간에서는 DetailedScan 직전 0.8에 머물게 합니다.
	UCFVehicleSensorComp* SensorComponent = AttachActiveScanSensor(
		SensorOwner,
		0.4f,
		4.0f,
		0.1f,
		0.8f,
		1.0f);
	if (!TestNotNull(TEXT("Operation Split Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	TestTrue(TEXT("Targeted-only Scan 시작"), SensorComponent->StartTargetedScan(ScanTarget));
	TestFalse(TEXT("Targeted-only Scan은 broad Active Detection을 켜지 않음"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("Targeted-only Scan Attempt 실행"), SensorComponent->IsTargetedScanRunning());
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestTrue(TEXT("Targeted-only 지정 대상 Contact 생성"), SensorComponent->FindRuntimeContactIndexByActor(ScanTarget) != INDEX_NONE);
	TestEqual(TEXT("Targeted-only는 주변 Active-range 대상을 broad 탐지하지 않음"), SensorComponent->FindRuntimeContactIndexByActor(NearbyTarget), INDEX_NONE);
	TestFalse(TEXT("Targeted-only Snapshot broad Active False"), SensorComponent->GetSensorSnapshot().bActiveScanRunning);
	TestTrue(TEXT("Targeted-only Snapshot ScanAttempt True"), SensorComponent->GetSensorSnapshot().ScanAttempt.bScanning);
	TestTrue(TEXT("Targeted-only 호환 Stop 승인"), SensorComponent->StopActiveScan());
	TestFalse(TEXT("Targeted-only Stop 뒤 Scan Attempt 종료"), SensorComponent->IsTargetedScanRunning());

	TestTrue(TEXT("broad Active Detection 시작"), SensorComponent->StartActiveScan());
	SensorComponent->RunPassiveDetectionUpdate(0.2f);
	TestTrue(TEXT("broad Pulse 첫 구간 뒤 실행 유지"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("broad Pulse가 주변 Active-range Contact 탐지"), SensorComponent->FindRuntimeContactIndexByActor(NearbyTarget) != INDEX_NONE);

	TestTrue(TEXT("broad Pulse 실행 중 Target Scan 동시 시작 허용"), SensorComponent->StartTargetedScan(ScanTarget));
	TestTrue(TEXT("동시 실행 broad Active True"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("동시 실행 Target Scan True"), SensorComponent->IsTargetedScanRunning());
	SensorComponent->RunPassiveDetectionUpdate(0.2f);
	TestFalse(TEXT("먼저 시작한 broad Pulse만 독립 만료"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("broad 만료 뒤 Target Scan은 계속 실행"), SensorComponent->IsTargetedScanRunning());
	TestTrue(TEXT("broad 만료 뒤 Target Scan progress 유지"), SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 > 0.0f);

	// [v1.4.0] broad Pulse 종료 뒤 주변 active-only Contact의 lifecycle을 확인할 Runtime Contact 인덱스입니다.
	const int32 NearbyContactIndex = SensorComponent->FindRuntimeContactIndexByActor(NearbyTarget);
	if (NearbyContactIndex != INDEX_NONE)
	{
		TestEqual(
			TEXT("broad 만료 뒤 주변 active-only Contact는 LastKnown"),
			SensorComponent->RuntimeContacts[NearbyContactIndex].PublicContact.ContactState,
			ECFSensorContactState::LastKnown);
	}

	TestTrue(TEXT("남은 Target Scan 호환 Stop 승인"), SensorComponent->StopActiveScan());
	TestFalse(TEXT("최종 Stop 뒤 broad Active False"), SensorComponent->IsActiveScanRunning());
	TestFalse(TEXT("최종 Stop 뒤 Target Scan False"), SensorComponent->IsTargetedScanRunning());
	TestTrue(TEXT("Operation Split Snapshot 공개 계약 유효"), SensorComponent->GetSensorSnapshot().IsPublicContractValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorAnalysisProgressTest,
	"CarFight.Sensor.SEN_P0_04.AnalysisProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 직접 가시 Active Analysis가 progress를 누적하고 threshold에서 Identified/DetailedScan Knowledge만 공개하는지 검증합니다.
bool FCFSensorAnalysisProgressTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Tactical Analysis와 Knowledge 승격을 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Analysis Progress 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Analysis Visibility 시작점이 될 Sensor Owner입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] Passive 밖·Active 안에서 직접 가시인 Tactical Analysis Target입니다.
	ACFMissileTestTarget* AnalysisTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorAnalysisProgressTarget"),
		FVector(3000.0f, 0.0f, 0.0f),
		TEXT("AnalysisProgressTarget"));
	if (!TestNotNull(TEXT("Analysis Progress Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Analysis Progress Target 생성"), AnalysisTarget))
	{
		return false;
	}

	// [v1.0.0] 초당 1.0 gain, Identified 0.25, Detailed 0.55로 짧은 update에서 두 승격을 검증할 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachActiveScanSensor(
		SensorOwner,
		2.0f,
		1.0f,
		0.2f,
		0.25f,
		0.55f);
	if (!TestNotNull(TEXT("Analysis Progress Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	TestTrue(TEXT("Analysis용 지정 Target Scan 시작"), SensorComponent->StartTargetedScan(AnalysisTarget));
	TestFalse(TEXT("지정 Target Scan은 broad Active Detection을 켜지 않음"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("지정 Target Scan Attempt 활성"), SensorComponent->GetSensorSnapshot().ScanAttempt.bScanning);
	SensorComponent->RunPassiveDetectionUpdate(0.1f);

	// [v1.0.0] 첫 Active update에서 생성된 분석 대상 Runtime Contact 인덱스입니다.
	const int32 AnalysisContactIndex = SensorComponent->FindRuntimeContactIndexByActor(AnalysisTarget);
	TestTrue(TEXT("Active range Target Contact 생성"), AnalysisContactIndex != INDEX_NONE);
	if (AnalysisContactIndex == INDEX_NONE)
	{
		return false;
	}

	// [v1.0.0] 모든 후속 Knowledge 승격에서도 유지돼야 할 최초 Sensor ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId;
	TestTrue(
		TEXT("첫 0.1초 Scan Attempt progress 약 0.1"),
		FMath::IsNearlyEqual(
			SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01,
			0.1f,
			KINDA_SMALL_NUMBER));
	TestEqual(
		TEXT("Source가 Identified여도 threshold 전 공개 Knowledge는 Detected"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.InformationLevel,
		ECFTargetInfoLevel::Detected);
	TestEqual(
		TEXT("Detected 단계 KnownTargetId 비공개"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.KnownTargetId,
		NAME_None);
	TestTrue(
		TEXT("Detected 단계 KnownDisplayName 비공개"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.KnownDisplayName.IsEmpty());
	TestEqual(
		TEXT("private Source InformationLevel은 원본 Identified 유지"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].SourceDisplayInfo.InformationLevel,
		ECFTargetInfoLevel::Identified);
	TestTrue(TEXT("유효 Analysis에서 ECC_Visibility Trace 실행"), SensorComponent->GetLastActiveAnalysisTraceCount() > 0);

	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	SensorComponent->RunPassiveDetectionUpdate(0.1f);

	// [v1.0.0] 0.3 progress에서 Identified로 승격된 공개 Contact입니다.
	const FCFSensorContact& IdentifiedContact = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact;
	TestEqual(TEXT("IdentifiedThreshold 통과 시 Identified 승격"), IdentifiedContact.InformationLevel, ECFTargetInfoLevel::Identified);
	TestEqual(TEXT("Identified에서 KnownTargetId 공개"), IdentifiedContact.KnownTargetId, AnalysisTarget->TargetId);
	TestEqual(
		TEXT("Identified에서 KnownDisplayName 공개"),
		IdentifiedContact.KnownDisplayName.ToString(),
		AnalysisTarget->TargetDisplayName.ToString());
	TestEqual(TEXT("Identified 승격 후 ContactId 유지"), IdentifiedContact.ContactId, OriginalContactId);

	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	SensorComponent->RunPassiveDetectionUpdate(0.1f);

	// [v1.2.0] 0.6 progress에서 DetailedScan까지 승격된 공개 Contact입니다.
	const FCFSensorContact& DetailedContact = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact;
	TestEqual(TEXT("DetailedScanThreshold 통과 시 DetailedScan 승격"), DetailedContact.InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestEqual(TEXT("DetailedScan에서도 KnownTargetId 유지"), DetailedContact.KnownTargetId, AnalysisTarget->TargetId);
	TestEqual(TEXT("DetailedScan에서도 ContactId 유지"), DetailedContact.ContactId, OriginalContactId);
	TestTrue(TEXT("DetailedScan 최초 완료 Revision 발급"), DetailedContact.AnalysisCompletionRevision > 0);
	// [v1.2.0] progress decay 뒤에도 그대로 유지돼야 할 terminal 분석 완료 Revision입니다.
	const int32 DetailedCompletionRevision = DetailedContact.AnalysisCompletionRevision;
	TestTrue(TEXT("DetailedScan 공개 Contact 계약 유효"), DetailedContact.IsPublicContractValid());

	TestFalse(TEXT("DetailedScan 완료 즉시 Scanner Runtime Idle"), SensorComponent->IsActiveScanRunning());
	TestFalse(TEXT("DetailedScan 완료 즉시 Scan Attempt 비활성"), SensorComponent->GetSensorSnapshot().ScanAttempt.bScanning);
	TestTrue(TEXT("DetailedScan 완료 즉시 Scan Attempt progress 0"), FMath::IsNearlyZero(SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01));
	TestTrue(TEXT("DetailedScan 완료 전이 Revision 발급"), SensorComponent->GetSensorSnapshot().ScanAttempt.CompletionTransitionRevision > 0);
	TestEqual(TEXT("DetailedScan 완료 전이 ContactId 일치"), SensorComponent->GetSensorSnapshot().ScanAttempt.LastCompletedContactId, OriginalContactId);
	TestFalse(TEXT("완료 뒤 StopActiveScan 중복 중단 거부"), SensorComponent->StopActiveScan());
	TestFalse(TEXT("이미 DetailedScan 완료된 동일 Target 즉시 재스캔 거부"), SensorComponent->StartTargetedScan(AnalysisTarget));
	TestEqual(
		TEXT("Attempt 종료 후에도 DetailedScan Knowledge 강등 없음"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.InformationLevel,
		ECFTargetInfoLevel::DetailedScan);
	TestEqual(
		TEXT("Attempt 종료 후에도 KnownTargetId 보존"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.KnownTargetId,
		AnalysisTarget->TargetId);
	TestEqual(
		TEXT("Attempt 종료 후에도 Knowledge 완료 Revision 유지"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisCompletionRevision,
		DetailedCompletionRevision);
	TestTrue(
		TEXT("Attempt 종료 후에도 공개 Contact 계약 유효"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.IsPublicContractValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorAnalysisDecayTest,
	"CarFight.Sensor.SEN_P0_04.AnalysisDecay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.3.0] 진행 중 Scan Attempt가 LOS·범위 이탈에서는 서서히 decay하고 재획득 시 남은 progress에서 재개되며 명시 중단에서는 Attempt만 즉시 0/비활성화되는지 검증합니다.
bool FCFSensorAnalysisDecayTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.3.0] Visibility blocker와 범위 이동을 실제 collision으로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Analysis Decay 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.3.0] Targeted Analysis LOS 시작점이 될 Sensor Owner입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.3.0] Passive 밖·Active 안에서 지정 Scan Attempt gain/decay를 반복할 대상입니다.
	ACFMissileTestTarget* AnalysisTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("SensorAnalysisDecayTarget"),
		FVector(3000.0f, 0.0f, 0.0f),
		TEXT("AnalysisDecayTarget"));
	if (!TestNotNull(TEXT("Analysis Decay Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Analysis Decay Target 생성"), AnalysisTarget))
	{
		return false;
	}

	// [v1.3.0] 초당 gain 1.0, decay 0.5로 0.1초마다 +0.1/-0.05 변화를 검증할 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachActiveScanSensor(
		SensorOwner,
		5.0f,
		1.0f,
		0.5f,
		0.8f,
		1.0f);
	if (!TestNotNull(TEXT("Analysis Decay Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	TestTrue(TEXT("Decay 테스트 지정 Target Scan 시작"), SensorComponent->StartTargetedScan(AnalysisTarget));
	for (int32 GainUpdateIndex = 0; GainUpdateIndex < 4; ++GainUpdateIndex)
	{
		SensorComponent->RunPassiveDetectionUpdate(0.1f);
	}

	// [v1.3.0] 4번의 유효 분석 update 뒤 대상 Runtime Contact 인덱스입니다.
	const int32 AnalysisContactIndex = SensorComponent->FindRuntimeContactIndexByActor(AnalysisTarget);
	TestTrue(TEXT("Decay 대상 Contact 생성"), AnalysisContactIndex != INDEX_NONE);
	if (AnalysisContactIndex == INDEX_NONE)
	{
		return false;
	}

	// [v1.3.0] 가림·범위 이탈·재획득 전체에서 유지돼야 할 Sensor ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId;

	// [v1.3.0] Visibility 가림 직전 현재 Scan Attempt에 남아 있는 진행률입니다.
	const float ProgressBeforeOcclusion = SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01;
	TestTrue(TEXT("가림 전 Scan Attempt progress 약 0.4"), FMath::IsNearlyEqual(ProgressBeforeOcclusion, 0.4f, KINDA_SMALL_NUMBER));

	// [v1.4.0] Target Scan의 단일 Target 관측은 유지하면서 Analysis LOS만 막을 World Actor입니다.
	AActor* VisibilityBlockerActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Analysis Visibility blocker Actor 생성"), VisibilityBlockerActor))
	{
		return false;
	}

	// [v1.3.0] TargetSelect 채널과 무관하게 ECC_Visibility만 Block할 분석 가림 박스입니다.
	UBoxComponent* VisibilityBlockerComponent = NewObject<UBoxComponent>(VisibilityBlockerActor, TEXT("SensorAnalysisVisibilityBlocker"));
	if (!TestNotNull(TEXT("Analysis Visibility blocker Component 생성"), VisibilityBlockerComponent))
	{
		return false;
	}
	VisibilityBlockerActor->SetRootComponent(VisibilityBlockerComponent);
	VisibilityBlockerActor->AddInstanceComponent(VisibilityBlockerComponent);
	VisibilityBlockerComponent->SetBoxExtent(FVector(300.0f, 300.0f, 500.0f));
	VisibilityBlockerComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisibilityBlockerComponent->SetCollisionObjectType(ECC_WorldStatic);
	VisibilityBlockerComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	VisibilityBlockerComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	VisibilityBlockerComponent->RegisterComponent();
	VisibilityBlockerActor->SetActorLocation(FVector(1500.0f, 0.0f, 0.0f));

	SensorComponent->RunPassiveDetectionUpdate(0.1f);

	// [v1.4.0] Target Scan의 단일 Active-range 관측은 LOS를 요구하지 않아 가림 중에도 유지돼야 할 Runtime Contact입니다.
	const UCFVehicleSensorComp::FCFSensorContactRuntime& OccludedRuntimeContact = SensorComponent->RuntimeContacts[AnalysisContactIndex];
	TestEqual(TEXT("가림 중 Active Contact는 Live 유지"), OccludedRuntimeContact.PublicContact.ContactState, ECFSensorContactState::Live);
	TestTrue(TEXT("가림 중 Targeted Analysis Visibility Trace 실행"), SensorComponent->GetLastActiveAnalysisTraceCount() > 0);
	TestTrue(TEXT("가림 시 Scan Attempt progress 감소"), SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 < ProgressBeforeOcclusion);
	TestTrue(TEXT("가림 시 Scan Attempt progress 즉시 0 reset 금지"), SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 > 0.0f);

	// [v1.3.0] 가림을 해제한 직후 남아 있는 Scan Attempt progress에서 다시 증가하는지 비교할 값입니다.
	const float ProgressAfterOcclusion = SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01;
	VisibilityBlockerComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestTrue(
		TEXT("가림 해제 후 남은 Scan Attempt progress에서 증가 재개"),
		SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 > ProgressAfterOcclusion);

	// [v1.3.0] ActiveScanRangeCm 밖으로 이동하기 직전 재개된 Scan Attempt progress입니다.
	const float ProgressBeforeRangeLoss = SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01;
	AnalysisTarget->SetActorLocation(FVector(7000.0f, 0.0f, 0.0f));
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestEqual(
		TEXT("Active range 이탈 시 P0-03 lifecycle로 LastKnown"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactState,
		ECFSensorContactState::LastKnown);
	TestTrue(
		TEXT("range 이탈 시 Scan Attempt progress 서서히 감소"),
		SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 < ProgressBeforeRangeLoss);
	TestTrue(
		TEXT("range 이탈 시 Scan Attempt progress 즉시 reset 금지"),
		SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 > 0.0f);

	// [v1.3.0] 동일 Actor를 Lost 전에 Active range 안으로 재획득해 같은 ContactId와 남은 Attempt progress를 재사용합니다.
	const float ProgressBeforeReacquire = SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01;
	AnalysisTarget->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestEqual(TEXT("Active range 재획득 후 Live 복귀"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("Active range 재획득 후 동일 ContactId"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId, OriginalContactId);
	TestTrue(TEXT("재획득 후 남은 Scan Attempt progress에서 증가"), SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01 > ProgressBeforeReacquire);

	TestTrue(TEXT("Decay 테스트 지정 Scan 수동 중단"), SensorComponent->StopActiveScan());
	TestEqual(
		TEXT("active-only Contact는 수동 중단 시 LastKnown"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactState,
		ECFSensorContactState::LastKnown);
	TestFalse(TEXT("StopActiveScan 뒤 Scan Attempt 비활성"), SensorComponent->GetSensorSnapshot().ScanAttempt.bScanning);
	TestTrue(TEXT("StopActiveScan 뒤 Scan Attempt progress 즉시 0"), FMath::IsNearlyZero(SensorComponent->GetSensorSnapshot().ScanAttempt.Progress01));
	TestEqual(TEXT("StopActiveScan 뒤 Scan Attempt TargetContactId 제거"), SensorComponent->GetSensorSnapshot().ScanAttempt.TargetContactId, NAME_None);
	TestEqual(TEXT("decay 전체 과정에서 ContactId 유지"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId, OriginalContactId);
	TestTrue(TEXT("중단 후 Snapshot 공개 계약 유효"), SensorComponent->GetSensorSnapshot().IsPublicContractValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorBasicSingleScanCompletionTest,
	"CarFight.Sensor.SEN_P0_04.BasicSensorSingleScanCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.3.0] canonical Basic Sensor가 정상 LOS/범위에서 지정 Target Scan Attempt 1회로 DetailedScan에 도달하고 완료 즉시 Idle/0으로 복귀하면서 Knowledge를 유지하는지 검증합니다.
bool FCFSensorBasicSingleScanCompletionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.3.0] 실제 차량 기본 Sensor DataAsset의 canonical object path입니다.
	const FString BasicSensorObjectPath = TEXT("/Game/CarFight/Vehicles/Data/Sensor/DA_VehicleSensor_Basic.DA_VehicleSensor_Basic");
	// [v1.3.0] 제품 기본값 자체를 회귀에 포함할 read-only canonical SensorData입니다.
	UCFVehicleSensorData* BasicSensorData = LoadObject<UCFVehicleSensorData>(nullptr, *BasicSensorObjectPath);
	if (!TestNotNull(TEXT("canonical Basic SensorData 로드"), BasicSensorData))
	{
		return false;
	}

	TestTrue(TEXT("canonical Basic SensorData 전체 계약 유효"), BasicSensorData->IsSensorDataContractValid());
	TestTrue(TEXT("canonical Basic Sensor 단발 완료 시간 예산 유효"), BasicSensorData->SensorConfig.IsSingleScanAnalysisBudgetValid());
	TestTrue(TEXT("canonical Basic Sensor AnalysisGain 0.40"), FMath::IsNearlyEqual(BasicSensorData->SensorConfig.AnalysisGainPerSec, 0.40f, KINDA_SMALL_NUMBER));

	// [v1.3.0] 실제 Basic Sensor behavior를 World Runtime으로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Basic Sensor 단발 완료 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.3.0] Basic Sensor 원점이 될 transient Owner Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);
	// [v1.3.0] Passive 20m/Visual 30m 밖이면서 Active 40m 안인 직접 가시 대상입니다.
	ACFMissileTestTarget* AnalysisTarget = SpawnActiveScanTarget(
		TestWorld,
		TEXT("BasicSensorSingleScanTarget"),
		FVector(3500.0f, 0.0f, 0.0f),
		TEXT("BasicSensorSingleScanTarget"));
	if (!TestNotNull(TEXT("Basic Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Basic Sensor Analysis Target 생성"), AnalysisTarget))
	{
		return false;
	}

	// [v1.3.0] canonical Vehicle Basic Sensor를 실제 Runtime 우선순위 경로로 적용할 Component입니다.
	UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(SensorOwner, TEXT("VehicleSensorComp_BasicSingleScanAutomation"));
	if (!TestNotNull(TEXT("Basic Sensor Runtime Component 생성"), SensorComponent))
	{
		return false;
	}
	SensorOwner->AddInstanceComponent(SensorComponent);
	SensorComponent->RegisterComponent();
	TestTrue(TEXT("canonical Basic Sensor를 Vehicle Base Source로 적용"), SensorComponent->ApplyVehicleBaseSensorData(BasicSensorData));
	TestTrue(TEXT("canonical Basic Sensor Source로 Runtime 초기화"), SensorComponent->InitializeSensorRuntime());
	TestTrue(TEXT("Basic Sensor 지정 Target Scan 시작"), SensorComponent->StartTargetedScan(AnalysisTarget));

	// [v1.3.0] 실제 DataAsset UpdateInterval로 단발 완료 시간 예산 안에서 필요한 bounded update 횟수입니다.
	const int32 ScanUpdateCount = FMath::CeilToInt(BasicSensorData->SensorConfig.ActiveScanDurationSec / BasicSensorData->SensorConfig.UpdateIntervalSec);
	// [v1.3.0] DetailedScan 완료와 즉시 Idle/0 전이가 같은 단발 Scan Attempt 안에서 발생했는지 기록합니다.
	bool bReachedDetailedAndReturnedIdle = false;
	for (int32 UpdateIndex = 0; UpdateIndex < ScanUpdateCount; ++UpdateIndex)
	{
		SensorComponent->RunPassiveDetectionUpdate(BasicSensorData->SensorConfig.UpdateIntervalSec);

		// [v1.3.0] 현재 public Snapshot에서 canonical Target의 공개 Contact와 Scan Attempt 완료 전이를 확인합니다.
		const FCFSensorSnapshot CurrentSnapshot = SensorComponent->GetSensorSnapshot();
		// [v1.3.0] DetailedScan 이후 KnownTargetId가 공개된 canonical Target Contact입니다.
		const FCFSensorContact* CurrentContact = CurrentSnapshot.Contacts.FindByPredicate([AnalysisTarget](const FCFSensorContact& Contact)
		{
			return Contact.KnownTargetId == AnalysisTarget->TargetId;
		});
		if (CurrentContact
			&& CurrentContact->InformationLevel == ECFTargetInfoLevel::DetailedScan
			&& CurrentSnapshot.ScanAttempt.CompletionTransitionRevision > 0)
		{
			bReachedDetailedAndReturnedIdle = !SensorComponent->IsActiveScanRunning()
				&& !CurrentSnapshot.ScanAttempt.bScanning
				&& FMath::IsNearlyZero(CurrentSnapshot.ScanAttempt.Progress01)
				&& CurrentSnapshot.ScanAttempt.LastCompletedContactId == CurrentContact->ContactId;
			break;
		}
	}

	TestTrue(TEXT("Basic Sensor는 지정 Scan Attempt 1회에서 DetailedScan 완료 후 즉시 Idle/0 복귀"), bReachedDetailedAndReturnedIdle);
	if (!bReachedDetailedAndReturnedIdle)
	{
		return false;
	}

	// [v1.3.0] 완료 직후 영구 Knowledge와 일시 Scan Attempt 상태를 함께 확인할 public Snapshot입니다.
	const FCFSensorSnapshot CompletedSnapshot = SensorComponent->GetSensorSnapshot();
	// [v1.3.0] 완료 직후 DetailedScan Knowledge가 남아 있어야 할 canonical Target Contact입니다.
	const FCFSensorContact* CompletedContact = CompletedSnapshot.Contacts.FindByPredicate([AnalysisTarget](const FCFSensorContact& Contact)
	{
		return Contact.KnownTargetId == AnalysisTarget->TargetId;
	});
	if (!TestNotNull(TEXT("Basic Sensor 완료 후 Target Contact 보존"), CompletedContact))
	{
		return false;
	}
	TestEqual(TEXT("Basic Sensor 완료 후 DetailedScan Knowledge 유지"), CompletedContact->InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestTrue(TEXT("Basic Sensor 완료 후 Knowledge Revision 발급 유지"), CompletedContact->AnalysisCompletionRevision > 0);
	TestFalse(TEXT("Basic Sensor 완료 후 Scan Attempt 비활성"), CompletedSnapshot.ScanAttempt.bScanning);
	TestTrue(TEXT("Basic Sensor 완료 후 Scan Attempt progress 0"), FMath::IsNearlyZero(CompletedSnapshot.ScanAttempt.Progress01));
	TestTrue(TEXT("Basic Sensor 완료 전이 Revision 발급"), CompletedSnapshot.ScanAttempt.CompletionTransitionRevision > 0);

	// [v1.3.0] 후속 update 이후에도 유지돼야 할 canonical Basic Sensor Knowledge 완료 Revision입니다.
	const int32 CompletionRevisionAtFinish = CompletedContact->AnalysisCompletionRevision;
	SensorComponent->RunPassiveDetectionUpdate(BasicSensorData->SensorConfig.UpdateIntervalSec);

	// [v1.3.0] 완료 다음 update에서 Knowledge 보존과 Attempt Idle 상태를 확인할 public Snapshot입니다.
	const FCFSensorSnapshot FollowupSnapshot = SensorComponent->GetSensorSnapshot();
	// [v1.3.0] 완료 다음 update에서도 같은 Knowledge를 보존해야 할 canonical Target Contact입니다.
	const FCFSensorContact* FollowupContact = FollowupSnapshot.Contacts.FindByPredicate([AnalysisTarget](const FCFSensorContact& Contact)
	{
		return Contact.KnownTargetId == AnalysisTarget->TargetId;
	});
	if (!TestNotNull(TEXT("Basic Sensor 완료 다음 update에도 Target Contact 보존"), FollowupContact))
	{
		return false;
	}
	TestFalse(TEXT("Basic Sensor 다음 update에서도 Scan Attempt 비활성 유지"), FollowupSnapshot.ScanAttempt.bScanning);
	TestTrue(TEXT("Basic Sensor 다음 update에서도 Scan Attempt progress 0 유지"), FMath::IsNearlyZero(FollowupSnapshot.ScanAttempt.Progress01));
	TestEqual(TEXT("Attempt 종료 후에도 DetailedScan Knowledge 강등 금지"), FollowupContact->InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestEqual(TEXT("Basic Sensor Attempt 종료 후에도 Knowledge 완료 Revision 유지"), FollowupContact->AnalysisCompletionRevision, CompletionRevisionAtFinish);
	TestFalse(TEXT("Basic Sensor 완료 Target 즉시 재스캔 거부"), SensorComponent->StartTargetedScan(AnalysisTarget));
	TestTrue(TEXT("Basic Sensor 완료 후 공개 Snapshot 계약 유효"), FollowupSnapshot.IsPublicContractValid());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
