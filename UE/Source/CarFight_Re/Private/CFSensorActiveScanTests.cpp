// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-036 SEN-P0-04 Active Scan Analysis asset-free Automation
// Scope: Active 장거리 전방향 Detection, scan duration, Analysis gain/decay, Knowledge 승격과 same-ID 재획득을 검증합니다.
// Changelog:
// - v1.0.0: ActiveRange, AnalysisProgress, AnalysisDecay 3개 Automation을 최초 추가.
// Migration:
// - transient Editor World와 C++ ACFMissileTestTarget만 사용하며 프로젝트 Content Asset을 생성·수정·저장하지 않습니다.
// - Sensor InputAction은 생성하지 않고 StartActiveScan/StopActiveScan Runtime API를 직접 호출합니다.
// - Active Detection은 LOS 없이 허용하지만 Tactical Analysis gain은 ECC_Visibility 직접 가시 조건을 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileTestTarget.h"
#include "CFVehicleSensorComp.h"

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

	TestTrue(TEXT("Analysis용 Active Scan 시작"), SensorComponent->StartActiveScan());
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
		TEXT("첫 0.1초 Analysis progress 약 0.1"),
		FMath::IsNearlyEqual(
			SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01,
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

	// [v1.0.0] 0.6 progress에서 DetailedScan까지 승격된 공개 Contact입니다.
	const FCFSensorContact& DetailedContact = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact;
	TestEqual(TEXT("DetailedScanThreshold 통과 시 DetailedScan 승격"), DetailedContact.InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestEqual(TEXT("DetailedScan에서도 KnownTargetId 유지"), DetailedContact.KnownTargetId, AnalysisTarget->TargetId);
	TestEqual(TEXT("DetailedScan에서도 ContactId 유지"), DetailedContact.ContactId, OriginalContactId);
	TestTrue(TEXT("DetailedScan 공개 Contact 계약 유효"), DetailedContact.IsPublicContractValid());

	TestTrue(TEXT("Active Scan 수동 중단 성공"), SensorComponent->StopActiveScan());

	// [v1.0.0] Active Scan 중단 뒤 progress 감소로 threshold 아래가 되어도 획득 Knowledge가 강등되지 않는지 검증합니다.
	SensorComponent->AdvanceContactAnalysis(
		2.0f,
		0.0f,
		SensorComponent->FallbackSensorConfig);
	TestTrue(
		TEXT("중단 뒤 Analysis progress는 감소"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 < 0.6f);
	TestEqual(
		TEXT("progress 감소 후에도 DetailedScan Knowledge 강등 없음"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.InformationLevel,
		ECFTargetInfoLevel::DetailedScan);
	TestEqual(
		TEXT("progress 감소 후에도 KnownTargetId 보존"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.KnownTargetId,
		AnalysisTarget->TargetId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorAnalysisDecayTest,
	"CarFight.Sensor.SEN_P0_04.AnalysisDecay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 가림·범위 이탈·Active 중단에서 progress가 즉시 reset되지 않고 decay하며 재획득 시 남은 progress와 ContactId를 유지하는지 검증합니다.
bool FCFSensorAnalysisDecayTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Visibility blocker와 범위 이동을 실제 collision으로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Analysis Decay 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Active Analysis LOS 시작점이 될 Sensor Owner입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] Passive 밖·Active 안에서 progress gain/decay를 반복할 대상입니다.
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

	// [v1.0.0] 초당 gain 1.0, decay 0.5로 0.1초마다 ±0.1/0.05 변화를 검증할 Sensor입니다.
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

	TestTrue(TEXT("Decay 테스트 Active Scan 시작"), SensorComponent->StartActiveScan());
	for (int32 GainUpdateIndex = 0; GainUpdateIndex < 4; ++GainUpdateIndex)
	{
		SensorComponent->RunPassiveDetectionUpdate(0.1f);
	}

	// [v1.0.0] 4번의 유효 분석 update 뒤 대상 Runtime Contact 인덱스입니다.
		const int32 AnalysisContactIndex = SensorComponent->FindRuntimeContactIndexByActor(AnalysisTarget);
	TestTrue(TEXT("Decay 대상 Contact 생성"), AnalysisContactIndex != INDEX_NONE);
	if (AnalysisContactIndex == INDEX_NONE)
	{
		return false;
	}

	// [v1.0.0] 가림·범위 이탈·재획득 전체에서 유지돼야 할 Sensor ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId;

	// [v1.0.0] Visibility 가림 직전 남아 있는 분석 진행률입니다.
	const float ProgressBeforeOcclusion = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01;
	TestTrue(TEXT("가림 전 progress 약 0.4"), FMath::IsNearlyEqual(ProgressBeforeOcclusion, 0.4f, KINDA_SMALL_NUMBER));

	// [v1.0.0] Active Detection 자체는 유지하면서 Tactical Analysis LOS만 막을 World Actor입니다.
	AActor* VisibilityBlockerActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Analysis Visibility blocker Actor 생성"), VisibilityBlockerActor))
	{
		return false;
	}

	// [v1.0.0] TargetSelect 채널과 무관하게 ECC_Visibility만 Block할 분석 가림 박스입니다.
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

	// [v1.0.0] Active Detection은 LOS를 요구하지 않아 가림 중에도 유지돼야 할 Runtime Contact입니다.
	const UCFVehicleSensorComp::FCFSensorContactRuntime& OccludedRuntimeContact = SensorComponent->RuntimeContacts[AnalysisContactIndex];
	TestEqual(TEXT("가림 중 Active Contact는 Live 유지"), OccludedRuntimeContact.PublicContact.ContactState, ECFSensorContactState::Live);
	TestTrue(TEXT("가림 중 Active Analysis Visibility Trace 실행"), SensorComponent->GetLastActiveAnalysisTraceCount() > 0);
	TestTrue(TEXT("가림 시 progress 감소"), OccludedRuntimeContact.PublicContact.AnalysisProgress01 < ProgressBeforeOcclusion);
	TestTrue(TEXT("가림 시 progress 즉시 0 reset 금지"), OccludedRuntimeContact.PublicContact.AnalysisProgress01 > 0.0f);

	// [v1.0.0] 가림을 해제한 직후 남아 있는 progress에서 다시 증가하는지 비교할 값입니다.
	const float ProgressAfterOcclusion = OccludedRuntimeContact.PublicContact.AnalysisProgress01;
	VisibilityBlockerComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestTrue(
		TEXT("가림 해제 후 남은 progress에서 증가 재개"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 > ProgressAfterOcclusion);

	// [v1.0.0] ActiveScanRangeCm 밖으로 이동하기 직전 재개된 progress입니다.
	const float ProgressBeforeRangeLoss = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01;
	AnalysisTarget->SetActorLocation(FVector(7000.0f, 0.0f, 0.0f));
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestEqual(
		TEXT("Active range 이탈 시 P0-03 lifecycle로 LastKnown"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactState,
		ECFSensorContactState::LastKnown);
	TestTrue(
		TEXT("range 이탈 시 progress 서서히 감소"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 < ProgressBeforeRangeLoss);
	TestTrue(
		TEXT("range 이탈 시 progress 즉시 reset 금지"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 > 0.0f);

	// [v1.0.0] 동일 Actor를 Lost 전에 Active range 안으로 재획득해 같은 ContactId와 남은 progress를 재사용합니다.
	const float ProgressBeforeReacquire = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01;
	AnalysisTarget->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));
	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestEqual(TEXT("Active range 재획득 후 Live 복귀"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("Active range 재획득 후 동일 ContactId"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId, OriginalContactId);
	TestTrue(TEXT("재획득 후 남은 progress에서 증가"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 > ProgressBeforeReacquire);

	// [v1.0.0] 수동 중단 직전 progress와 비교해 Active Scan이 없어도 즉시 0으로 지우지 않는지 확인합니다.
	const float ProgressBeforeStop = SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01;
	TestTrue(TEXT("Decay 테스트 Active Scan 수동 중단"), SensorComponent->StopActiveScan());
	TestEqual(
		TEXT("active-only Contact는 수동 중단 시 LastKnown"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactState,
		ECFSensorContactState::LastKnown);
	TestTrue(
		TEXT("StopActiveScan 자체는 progress를 즉시 reset하지 않음"),
		FMath::IsNearlyEqual(
			SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01,
			ProgressBeforeStop,
			KINDA_SMALL_NUMBER));

	SensorComponent->RunPassiveDetectionUpdate(0.1f);
	TestTrue(
		TEXT("Active 중단 뒤 다음 Sensor update에서 progress decay"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 < ProgressBeforeStop);
	TestTrue(
		TEXT("Active 중단 뒤에도 progress 즉시 0 reset 금지"),
		SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.AnalysisProgress01 > 0.0f);
	TestEqual(TEXT("decay 전체 과정에서 ContactId 유지"), SensorComponent->RuntimeContacts[AnalysisContactIndex].PublicContact.ContactId, OriginalContactId);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
