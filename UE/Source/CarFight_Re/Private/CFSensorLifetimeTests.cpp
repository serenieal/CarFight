// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-036 SEN-P0-03 Contact Lifetime asset-free Automation
// Scope: Live→LastKnown→Lost→Removed, frozen LastKnown 위치, ContactMemory/Freshness, Lost 최소 1 Snapshot과 Lost 전 재획득 보존을 검증합니다.
// Changelog:
// - v1.0.0: ContactLifetime과 Reacquire 2개 Automation을 최초 추가.
// Migration:
// - transient Editor World와 C++ ACFMissileTestTarget만 사용하며 프로젝트 Content Asset을 생성·수정·저장하지 않습니다.
// - lifecycle 시간은 실제 대기 대신 private AdvanceContactLifetimes에 명시 시각을 전달해 결정적으로 검증합니다.
// - DestroyedHold, Active Scan, TargetSelect와 HUD 계약은 이 테스트 범위에 포함하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileTestTarget.h"
#include "CFVehicleSensorComp.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] 명시 이름·위치·TargetId를 가진 C++ TargetSelectable Actor를 transient World에 생성합니다.
	ACFMissileTestTarget* SpawnSensorLifetimeTarget(
		UWorld* TestWorld,
		const FName ActorName,
		const FVector& ActorLocation,
		const FName TargetId)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		// [v1.0.0] 테스트 Actor의 결정적인 이름을 지정할 Spawn 파라미터입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		// [v1.0.0] Sensor Contact lifecycle의 실제 TargetSelectable source로 사용할 테스트 Actor입니다.
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

	// [v1.0.0] plain Actor Owner에 transient Sensor Component를 붙이고 P0-03 lifecycle 테스트 설정으로 초기화합니다.
	UCFVehicleSensorComp* AttachSensorLifetimeComponent(
		AActor* SensorOwner,
		const float PassiveRangeCm,
		const float ContactMemoryTimeSec)
	{
		if (!SensorOwner)
		{
			return nullptr;
		}

		// [v1.0.0] 저장 Asset 없이 실제 World/Owner lifecycle을 사용하는 테스트 Sensor Component입니다.
		UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(SensorOwner, TEXT("VehicleSensorComp_LifetimeAutomation"));
		if (!SensorComponent)
		{
			return nullptr;
		}

		SensorOwner->AddInstanceComponent(SensorComponent);
		SensorComponent->RegisterComponent();
		SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = PassiveRangeCm;
		SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
		SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = 1;
		SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = ContactMemoryTimeSec;
		SensorComponent->FallbackSensorConfig.DestroyedHoldTimeSec = 0.0f;
		SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = 0.0f;
		SensorComponent->FallbackSensorConfig.AnalysisGainPerSec = 0.0f;
		SensorComponent->FallbackSensorConfig.AnalysisDecayPerSec = 0.0f;
		SensorComponent->FallbackSensorConfig.IdentifiedThreshold = 0.5f;
		SensorComponent->FallbackSensorConfig.DetailedScanThreshold = 1.0f;
		return SensorComponent->InitializeSensorRuntime() ? SensorComponent : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorContactLifetimeTest,
	"CarFight.Sensor.SEN_P0_03.ContactLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Live→LastKnown→Lost→Removed와 frozen 위치, Freshness, Lost 1회 게시가 bounded cursor 재방문 없이 진행되는지 검증합니다.
bool FCFSensorContactLifetimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 저장 Asset 없이 lifecycle을 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Contact Lifetime 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Sensor Component를 소유할 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] 최초 2,000cm Passive 범위 안에서 Live Contact가 될 Target입니다.
	ACFMissileTestTarget* TargetActor = SpawnSensorLifetimeTarget(
		TestWorld,
		TEXT("SensorLifetimeTarget"),
		FVector(2000.0f, 0.0f, 0.0f),
		TEXT("LifetimeTarget"));
	if (!TestNotNull(TEXT("Contact Lifetime Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Contact Lifetime Target 생성"), TargetActor))
	{
		return false;
	}

	// [v1.0.0] Passive 5,000cm, Contact Memory 1초로 구성한 테스트 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachSensorLifetimeComponent(SensorOwner, 5000.0f, 1.0f);
	if (!TestNotNull(TEXT("Contact Lifetime Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.0.0] 실제 Sensor 후보 판정 경로로 최초 Live Contact를 생성했는지 여부입니다.
	const bool bInitialDetectionAccepted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("최초 Passive 탐지 Live Contact 생성"), bInitialDetectionAccepted);
	TestEqual(TEXT("최초 Runtime Contact 1개"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.0.0] 상태 전이 전체에서 유지돼야 할 최초 ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[0].PublicContact.ContactId;

	// [v1.0.0] LastKnown에서 절대로 실제 Actor 이동을 따라가면 안 되는 마지막 신뢰 위치입니다.
	const FVector OriginalLastKnownWorldLocation = SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation;

	// [v1.0.0] Freshness 계산의 기준이 되는 마지막 유효 관측 월드 시각입니다.
	const double OriginalLastObservedWorldTimeSeconds = SensorComponent->RuntimeContacts[0].PublicContact.LastObservedWorldTimeSeconds;
	TestEqual(TEXT("최초 Contact 상태 Live"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("최초 Freshness 0"), SensorComponent->RuntimeContacts[0].PublicContact.FreshnessSeconds, 0.0f);

	TargetActor->SetActorLocation(FVector(9000.0f, 0.0f, 0.0f));

	// [v1.0.0] 같은 Actor를 실제로 재평가해 Passive 범위 이탈을 관측했는지 여부입니다.
	const bool bLossTransitionApplied = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("범위 이탈 기존 Contact를 LastKnown으로 전환"), bLossTransitionApplied);
	TestEqual(TEXT("탐지 상실 직후 LastKnown"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestTrue(
		TEXT("LastKnown 위치는 마지막 신뢰 위치로 고정"),
		SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));
	TestEqual(
		TEXT("LastKnown 전환은 마지막 관측 시각을 변경하지 않음"),
		SensorComponent->RuntimeContacts[0].PublicContact.LastObservedWorldTimeSeconds,
		OriginalLastObservedWorldTimeSeconds);

	// [v1.0.0] bounded Actor cursor를 다시 방문하지 않고 ContactMemory 중간 시점을 직접 진행할 합성 월드 시각입니다.
	const double QuarterSecondWorldTime = OriginalLastObservedWorldTimeSeconds + 0.25;
	SensorComponent->AdvanceContactLifetimes(
		QuarterSecondWorldTime,
		SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("0.25초 후에도 LastKnown 유지"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestTrue(
		TEXT("FreshnessSeconds가 Sensor update 시간으로 0.25초 진행"),
		FMath::IsNearlyEqual(SensorComponent->RuntimeContacts[0].PublicContact.FreshnessSeconds, 0.25f, KINDA_SMALL_NUMBER));
	TestTrue(
		TEXT("Freshness 진행 중에도 LastKnown 위치 고정"),
		SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));

	SensorComponent->PublishRuntimeSnapshot();

	// [v1.0.0] ContactMemory 만료 전 LastKnown 상태가 공개된 Actor-free Snapshot입니다.
	const FCFSensorSnapshot LastKnownSnapshot = SensorComponent->GetSensorSnapshot();
	TestEqual(TEXT("LastKnown Snapshot Contact 1개"), LastKnownSnapshot.Contacts.Num(), 1);
	if (LastKnownSnapshot.Contacts.Num() == 1)
	{
		TestEqual(TEXT("공개 Snapshot 상태 LastKnown"), LastKnownSnapshot.Contacts[0].ContactState, ECFSensorContactState::LastKnown);
		TestEqual(TEXT("LastKnown Snapshot ContactId 유지"), LastKnownSnapshot.Contacts[0].ContactId, OriginalContactId);
		TestTrue(TEXT("LastKnown Snapshot 공개 계약 유효"), LastKnownSnapshot.IsPublicContractValid());
	}

	// [v1.0.0] ContactMemoryTimeSec 1초를 정확히 만료시키는 합성 월드 시각입니다.
	const double LostWorldTime = OriginalLastObservedWorldTimeSeconds + 1.0;
	SensorComponent->AdvanceContactLifetimes(
		LostWorldTime,
		SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("ContactMemory 만료 시 Lost 전환"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::Lost);
	TestTrue(
		TEXT("Lost에서도 마지막 신뢰 위치 고정"),
		SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Lost는 게시 전 cleanup 대기"), !SensorComponent->RuntimeContacts[0].bLostSnapshotPublished);

	SensorComponent->PublishRuntimeSnapshot();

	// [v1.0.0] 최소 한 Snapshot에 반드시 노출돼야 하는 Lost 공개 결과입니다.
	const FCFSensorSnapshot LostSnapshot = SensorComponent->GetSensorSnapshot();
	TestEqual(TEXT("Lost Snapshot Contact 1개"), LostSnapshot.Contacts.Num(), 1);
	if (LostSnapshot.Contacts.Num() == 1)
	{
		TestEqual(TEXT("Lost 상태가 최소 한 Snapshot에 노출"), LostSnapshot.Contacts[0].ContactState, ECFSensorContactState::Lost);
		TestEqual(TEXT("Lost Snapshot도 같은 ContactId"), LostSnapshot.Contacts[0].ContactId, OriginalContactId);
		TestTrue(TEXT("Lost Snapshot 공개 계약 유효"), LostSnapshot.IsPublicContractValid());
	}
	TestTrue(TEXT("Lost Snapshot 게시 완료 flag 설정"), SensorComponent->RuntimeContacts[0].bLostSnapshotPublished);

	// [v1.0.0] Actor cursor 재방문 없이 다음 Sensor update에서 Lost cleanup만 진행할 합성 월드 시각입니다.
	const double CleanupWorldTime = OriginalLastObservedWorldTimeSeconds + 1.1;
	SensorComponent->AdvanceContactLifetimes(
		CleanupWorldTime,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("Lost 1회 게시 다음 update에서 Runtime Contact 제거"), SensorComponent->RuntimeContacts.IsEmpty());

	SensorComponent->PublishRuntimeSnapshot();
	TestTrue(TEXT("Lost cleanup 뒤 공개 Snapshot Contact 없음"), SensorComponent->GetSensorSnapshot().Contacts.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorContactReacquireTest,
	"CarFight.Sensor.SEN_P0_03.Reacquire",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Lost 게시 전 같은 Actor 재획득 시 ContactId와 기존 Sensor Knowledge를 보존한 Live로 복귀하는지 검증합니다.
bool FCFSensorContactReacquireTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 재획득 계약을 저장 Asset 없이 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Reacquire 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 재획득 Sensor Component를 소유할 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] 최초 Live와 이후 동일 Actor 재획득에 사용할 Target입니다.
	ACFMissileTestTarget* TargetActor = SpawnSensorLifetimeTarget(
		TestWorld,
		TEXT("SensorReacquireTarget"),
		FVector(2000.0f, 0.0f, 0.0f),
		TEXT("ReacquireTarget"));
	if (!TestNotNull(TEXT("Reacquire Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Reacquire Target 생성"), TargetActor))
	{
		return false;
	}

	// [v1.0.0] 재획득 여유를 충분히 두기 위해 Contact Memory 2초로 초기화한 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachSensorLifetimeComponent(SensorOwner, 5000.0f, 2.0f);
	if (!TestNotNull(TEXT("Reacquire Sensor 초기화"), SensorComponent))
	{
		return false;
	}

			SensorComponent->ProcessPassiveScanActor(TargetActor, SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("Reacquire 최초 Runtime Contact 1개"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.0.0] P0-04 이전에도 P0-03가 기존 Sensor Knowledge를 지우지 않는지 검증하기 위해 주입할 Runtime Contact입니다.
	UCFVehicleSensorComp::FCFSensorContactRuntime& InitialRuntimeContact = SensorComponent->RuntimeContacts[0];
	InitialRuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::Identified;
	InitialRuntimeContact.PublicContact.KnownTargetId = TargetActor->TargetId;
	InitialRuntimeContact.PublicContact.KnownDisplayName = TargetActor->TargetDisplayName;
	InitialRuntimeContact.PublicContact.AnalysisProgress01 = 0.4f;

	// [v1.0.0] 재획득 뒤 동일해야 할 ContactId입니다.
	const FName OriginalContactId = InitialRuntimeContact.PublicContact.ContactId;

	// [v1.0.0] LastKnown 동안 고정돼야 할 최초 마지막 신뢰 위치입니다.
	const FVector OriginalLastKnownWorldLocation = InitialRuntimeContact.PublicContact.LastKnownWorldLocation;

	// [v1.0.0] 재획득 전 Freshness 계산 기준이 되는 최초 관측 시각입니다.
	const double OriginalLastObservedWorldTimeSeconds = InitialRuntimeContact.PublicContact.LastObservedWorldTimeSeconds;

	TargetActor->SetActorLocation(FVector(9000.0f, 0.0f, 0.0f));
	SensorComponent->ProcessPassiveScanActor(TargetActor, SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("재획득 전 범위 이탈 LastKnown"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::LastKnown);

	// [v1.0.0] Lost 전에 0.5초만 경과시켜 기존 Contact를 유지할 합성 월드 시각입니다.
	const double ReacquireMemoryWorldTime = OriginalLastObservedWorldTimeSeconds + 0.5;
	SensorComponent->AdvanceContactLifetimes(
		ReacquireMemoryWorldTime,
		SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("ContactMemory 안에서는 LastKnown 유지"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestEqual(TEXT("LastKnown 중 ContactId 유지"), SensorComponent->RuntimeContacts[0].PublicContact.ContactId, OriginalContactId);
	TestEqual(TEXT("LastKnown 중 InformationLevel 유지"), SensorComponent->RuntimeContacts[0].PublicContact.InformationLevel, ECFTargetInfoLevel::Identified);
	TestEqual(TEXT("LastKnown 중 KnownTargetId 유지"), SensorComponent->RuntimeContacts[0].PublicContact.KnownTargetId, TargetActor->TargetId);
	TestTrue(
		TEXT("LastKnown 중 마지막 신뢰 위치 유지"),
		SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));

	TargetActor->SetActorLocation(FVector(2500.0f, 500.0f, 0.0f));

	// [v1.0.0] 같은 Actor를 Passive 범위 안에서 다시 관측해 기존 Contact를 Live로 되돌렸는지 여부입니다.
	const bool bReacquired = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("Lost 전 같은 Actor 재획득 성공"), bReacquired);
	TestEqual(TEXT("재획득 후 Runtime Contact 수 증가 없음"), SensorComponent->RuntimeContacts.Num(), 1);

	// [v1.0.0] 재획득 결과를 검증할 기존 private Runtime Contact입니다.
	const UCFVehicleSensorComp::FCFSensorContactRuntime& ReacquiredRuntimeContact = SensorComponent->RuntimeContacts[0];
	TestEqual(TEXT("재획득 후 Live 복귀"), ReacquiredRuntimeContact.PublicContact.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("재획득 후 동일 ContactId"), ReacquiredRuntimeContact.PublicContact.ContactId, OriginalContactId);
	TestEqual(TEXT("재획득 후 InformationLevel 보존"), ReacquiredRuntimeContact.PublicContact.InformationLevel, ECFTargetInfoLevel::Identified);
	TestEqual(TEXT("재획득 후 KnownTargetId 보존"), ReacquiredRuntimeContact.PublicContact.KnownTargetId, TargetActor->TargetId);
	TestEqual(
		TEXT("재획득 후 KnownDisplayName 보존"),
		ReacquiredRuntimeContact.PublicContact.KnownDisplayName.ToString(),
		TargetActor->TargetDisplayName.ToString());
	TestTrue(
		TEXT("재획득 후 AnalysisProgress 보존"),
		FMath::IsNearlyEqual(ReacquiredRuntimeContact.PublicContact.AnalysisProgress01, 0.4f, KINDA_SMALL_NUMBER));
	TestEqual(TEXT("재획득 후 Freshness 0으로 갱신"), ReacquiredRuntimeContact.PublicContact.FreshnessSeconds, 0.0f);
	TestTrue(
		TEXT("재획득 후 마지막 신뢰 위치를 새 실제 관측 위치로 갱신"),
		ReacquiredRuntimeContact.PublicContact.LastKnownWorldLocation.Equals(TargetActor->GetActorLocation(), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("재획득은 Lost 게시 flag 초기화"), !ReacquiredRuntimeContact.bLostSnapshotPublished);

	SensorComponent->PublishRuntimeSnapshot();

	// [v1.0.0] 재획득 후 외부 소비자가 읽을 Actor-free Live Snapshot입니다.
	const FCFSensorSnapshot ReacquiredSnapshot = SensorComponent->GetSensorSnapshot();
	TestEqual(TEXT("재획득 Snapshot Contact 1개"), ReacquiredSnapshot.Contacts.Num(), 1);
	if (ReacquiredSnapshot.Contacts.Num() == 1)
	{
		TestEqual(TEXT("재획득 Snapshot Live"), ReacquiredSnapshot.Contacts[0].ContactState, ECFSensorContactState::Live);
		TestEqual(TEXT("재획득 Snapshot 동일 ContactId"), ReacquiredSnapshot.Contacts[0].ContactId, OriginalContactId);
		TestTrue(TEXT("재획득 Snapshot 공개 계약 유효"), ReacquiredSnapshot.IsPublicContractValid());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
