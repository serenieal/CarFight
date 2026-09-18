// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-09-18
// Description: Contact Lifetime + Phase 3B-1 Persistent Knowledge reassociation Automation
// Scope: 기존 Contact lifecycle과 함께 valid TargetEntityId의 DetailedScan Knowledge가 Removed 뒤 새 Contact에 exact revision으로 복원되고 identity conflict/reset 경계가 지켜지는지 검증합니다.
// Changelog:
// - v1.4.0: Phase 3B-1 PersistentKnowledge 테스트를 추가해 Detected-only no-record, Identified/DetailedScan 즉시 Store upsert, Scanner/Vehicle base hot reapply 보존, Removed 뒤 새 Contact reassociation, exact AnalysisCompletionRevision/allocator 보존, concrete identity conflict와 Terminal Entity 재등장 fail-closed, Initialize/Reset hard clear를 검증.
// - v1.3.0: C++ 테스트 Target provider의 직접 virtual implementation 안정성과 Sensor native/Blueprint resolver를 통한 Contact TargetEntityId 캡처·보존을 고정.
// - v1.2.0: ACFMissileTestTarget의 per-instance TargetEntityId가 최초 Contact에 캡처되고 LastKnown/Lost/재획득 Snapshot까지 불변 유지되는 회귀를 추가.
// - v1.1.0: 재획득 fixture/assertion의 제거된 Contact AnalysisProgress를 삭제하고 Knowledge/Identity 보존 의미만 검증.
// - v1.0.0: ContactLifetime과 Reacquire 2개 Automation을 최초 추가.
// Migration:
// - v1.4.0은 test-only이며 Phase 3B-1 private Store를 친구 테스트로 검증합니다. Product Asset 저장/수정은 없습니다.
// - v1.3.0은 test-only이며 C++ fixture의 Entity ID provider와 Sensor resolver 결과를 검증합니다. Product Asset migration은 없습니다.
// - v1.2.0은 test-only이며 Contact lifecycle/reacquire에서 TargetEntityId 불변성만 추가 검증합니다. Content Asset migration은 없습니다.
// - v1.1.0부터 Contact lifecycle은 Scan Attempt progress를 보존하지 않습니다. 재획득은 ContactId와 획득 Knowledge를 보존하는 것으로 검증합니다.
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

	// [v1.3.0] C++ 테스트 Target provider가 같은 Actor lifetime 동안 반환하는 최초 Gameplay Entity Identity입니다.
	const FGuid FirstTargetEntityId = TargetActor->GetTargetEntityId_Implementation();
	// [v1.3.0] 같은 native provider를 반복 호출했을 때의 Entity Identity입니다.
	const FGuid RepeatedTargetEntityId = TargetActor->GetTargetEntityId_Implementation();
	TestTrue(TEXT("테스트 TargetEntityId 유효"), FirstTargetEntityId.IsValid());
	TestTrue(TEXT("같은 Actor 반복 Entity ID 조회는 동일"), FirstTargetEntityId == RepeatedTargetEntityId);

	// [v1.0.0] 상태 전이 전체에서 유지돼야 할 최초 ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[0].PublicContact.ContactId;
	// [v1.2.0] Contact 최초 생성 때 TargetSelectable provider에서 캡처되어 수명 전체에 유지돼야 할 Entity ID입니다.
	const FGuid OriginalTargetEntityId = SensorComponent->RuntimeContacts[0].PublicContact.TargetEntityId;
	TestTrue(TEXT("최초 Contact TargetEntityId 유효"), OriginalTargetEntityId.IsValid());
	TestTrue(TEXT("최초 Contact Entity ID는 Target provider와 동일"), OriginalTargetEntityId == FirstTargetEntityId);

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
		TestTrue(TEXT("LastKnown Snapshot TargetEntityId 유지"), LastKnownSnapshot.Contacts[0].TargetEntityId == OriginalTargetEntityId);
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
		TestTrue(TEXT("Lost Snapshot도 같은 TargetEntityId"), LostSnapshot.Contacts[0].TargetEntityId == OriginalTargetEntityId);
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
	InitialRuntimeContact.PublicContact.AnalysisCompletionRevision = 0;

	// [v1.0.0] 재획득 뒤 동일해야 할 ContactId입니다.
	const FName OriginalContactId = InitialRuntimeContact.PublicContact.ContactId;
	// [v1.2.0] 재획득 뒤에도 같은 Gameplay Entity임을 증명해야 할 최초 Entity ID입니다.
	const FGuid OriginalTargetEntityId = InitialRuntimeContact.PublicContact.TargetEntityId;
	TestTrue(TEXT("재획득 최초 TargetEntityId 유효"), OriginalTargetEntityId.IsValid());
	TestTrue(
		TEXT("재획득 최초 Contact Entity ID는 Target native provider와 동일"),
		OriginalTargetEntityId == TargetActor->GetTargetEntityId_Implementation());

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
	TestTrue(TEXT("재획득 후 동일 TargetEntityId"), ReacquiredRuntimeContact.PublicContact.TargetEntityId == OriginalTargetEntityId);
	TestEqual(TEXT("재획득 후 InformationLevel 보존"), ReacquiredRuntimeContact.PublicContact.InformationLevel, ECFTargetInfoLevel::Identified);
	TestEqual(TEXT("재획득 후 KnownTargetId 보존"), ReacquiredRuntimeContact.PublicContact.KnownTargetId, TargetActor->TargetId);
	TestEqual(
		TEXT("재획득 후 KnownDisplayName 보존"),
		ReacquiredRuntimeContact.PublicContact.KnownDisplayName.ToString(),
		TargetActor->TargetDisplayName.ToString());
	TestEqual(TEXT("재획득 후 Identified 완료 Revision 0 유지"), ReacquiredRuntimeContact.PublicContact.AnalysisCompletionRevision, 0);
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
		TestTrue(TEXT("재획득 Snapshot 동일 TargetEntityId"), ReacquiredSnapshot.Contacts[0].TargetEntityId == OriginalTargetEntityId);
		TestTrue(TEXT("재획득 Snapshot 공개 계약 유효"), ReacquiredSnapshot.IsPublicContractValid());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorPersistentKnowledgeTest,
	"CarFight.Sensor.SEN_P0_03.PersistentKnowledge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.4.0] Persistent Knowledge가 Contact 제거와 재생성을 넘어 동일 Entity에 복원되고 identity/reset 경계를 지키는지 검증합니다.
bool FCFSensorPersistentKnowledgeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] 저장 Asset 없이 Store/Reassociation 계약을 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("PersistentKnowledge 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.4.0] Persistent Knowledge Sensor를 소유할 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.4.0] Contact Removed 뒤에도 같은 Gameplay Entity ID를 유지할 테스트 Target입니다.
	ACFMissileTestTarget* TargetActor = SpawnSensorLifetimeTarget(
		TestWorld,
		TEXT("SensorPersistentKnowledgeTarget"),
		FVector(2000.0f, 0.0f, 0.0f),
		TEXT("PersistentKnowledgeTarget"));
	if (!TestNotNull(TEXT("PersistentKnowledge Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("PersistentKnowledge Target 생성"), TargetActor))
	{
		return false;
	}

	// [v1.4.0] Contact 제거를 결정적으로 진행할 Contact Memory 1초 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachSensorLifetimeComponent(SensorOwner, 5000.0f, 1.0f);
	if (!TestNotNull(TEXT("PersistentKnowledge Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.4.0] 최초 Detected Contact를 생성한 실제 Sensor 후보 평가 결과입니다.
	const bool bInitialDetectionAccepted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("PersistentKnowledge 최초 Contact 생성"), bInitialDetectionAccepted);
	TestEqual(TEXT("PersistentKnowledge 최초 Contact 1개"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.4.0] Store exact key와 재획득 동일성에 사용할 Gameplay Entity ID입니다.
	const FGuid TargetEntityId = SensorComponent->RuntimeContacts[0].PublicContact.TargetEntityId;
	TestTrue(TEXT("PersistentKnowledge TargetEntityId 유효"), TargetEntityId.IsValid());
	TestTrue(TEXT("Detected-only non-terminal은 Store Record를 만들지 않음"), SensorComponent->PersistentKnowledgeStore.IsEmpty());

	// [v1.4.0] Identified 임계값을 직접 통과시켜 즉시 Store upsert를 검증합니다.
	SensorComponent->PromoteContactKnowledgeFromScanProgress(
		SensorComponent->RuntimeContacts[0],
		SensorComponent->FallbackSensorConfig.IdentifiedThreshold,
		SensorComponent->FallbackSensorConfig);
	// [v1.4.0] Identified 승격 직후 생성돼야 하는 같은 Entity의 Persistent Knowledge Record입니다.
	const UCFVehicleSensorComp::FCFSensorKnowledgeRecord* IdentifiedRecord = SensorComponent->PersistentKnowledgeStore.Find(TargetEntityId);
	if (!TestNotNull(TEXT("Identified 즉시 Persistent Record 생성"), IdentifiedRecord))
	{
		return false;
	}
	TestEqual(TEXT("Identified Store 최고 Knowledge"), IdentifiedRecord->HighestKnowledgeTier, ECFTargetInfoLevel::Identified);
	TestEqual(TEXT("Identified Store KnownTargetId"), IdentifiedRecord->KnownTargetId, TargetActor->TargetId);
	TestTrue(TEXT("Identified Store는 Detailed revision 없음"), IdentifiedRecord->DetailedScanAnalysisRevision == 0);

	// [v1.4.0] DetailedScan 임계값을 통과시켜 양수 AnalysisCompletionRevision과 Store exact 보존을 만듭니다.
	SensorComponent->PromoteContactKnowledgeFromScanProgress(
		SensorComponent->RuntimeContacts[0],
		SensorComponent->FallbackSensorConfig.DetailedScanThreshold,
		SensorComponent->FallbackSensorConfig);
	// [v1.4.0] 최초 DetailedScan 획득 결과와 exact revision을 보존할 공개 Contact 사본입니다.
	const FCFSensorContact DetailedContact = SensorComponent->RuntimeContacts[0].PublicContact;
	TestEqual(TEXT("DetailedScan Contact 승격"), DetailedContact.InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestTrue(TEXT("DetailedScan AnalysisCompletionRevision 양수"), DetailedContact.AnalysisCompletionRevision > 0);

	// [v1.4.0] DetailedScan 최고 Knowledge와 exact Analysis revision을 저장한 Persistent Record입니다.
	const UCFVehicleSensorComp::FCFSensorKnowledgeRecord* DetailedRecord = SensorComponent->PersistentKnowledgeStore.Find(TargetEntityId);
	if (!TestNotNull(TEXT("DetailedScan Persistent Record 유지"), DetailedRecord))
	{
		return false;
	}
	TestEqual(TEXT("Detailed Store 최고 Knowledge"), DetailedRecord->HighestKnowledgeTier, ECFTargetInfoLevel::DetailedScan);
	TestEqual(TEXT("Detailed Store exact Analysis revision"), DetailedRecord->DetailedScanAnalysisRevision, DetailedContact.AnalysisCompletionRevision);
	TestTrue(TEXT("Detailed Store Terminal false"), !DetailedRecord->bTerminalDestroyed);

	// [v1.4.0] Contact 제거/재생성 전후 exact하게 보존할 최초 ContactId입니다.
	const FName OriginalContactId = DetailedContact.ContactId;
	// [v1.4.0] Reassociation이 새 completion으로 오인되지 않는지 확인할 allocator 값입니다.
	const int32 NextAnalysisRevisionBeforeReassociation = SensorComponent->NextAnalysisCompletionRevision;
	// [v1.4.0] Reassociation이 HUD 완료 event를 재발행하지 않는지 확인할 transient transition revision입니다.
	const int32 CompletionTransitionBeforeReassociation = SensorComponent->LastScanCompletionTransitionRevision;

	// [v1.4.0] Scanner hot reapply 의미의 ApplySensorData(nullptr)가 기존 Store를 보존하는지 확인합니다.
	const bool bHotReapplyAccepted = SensorComponent->ApplySensorData(nullptr);
	TestTrue(TEXT("ApplySensorData hot reapply 성공"), bHotReapplyAccepted);
	TestTrue(TEXT("Scanner hot reapply 뒤 Persistent Record 유지"), SensorComponent->PersistentKnowledgeStore.Contains(TargetEntityId));

	// [v1.4.0] VehicleData 기본 Sensor Source를 재적용해도 획득 Knowledge가 유지되는지 확인할 결과입니다.
	const bool bVehicleBaseReapplyAccepted = SensorComponent->ApplyVehicleBaseSensorData(nullptr);
	TestTrue(TEXT("ApplyVehicleBaseSensorData hot reapply 성공"), bVehicleBaseReapplyAccepted);
	TestTrue(TEXT("Vehicle base hot reapply 뒤 Persistent Record 유지"), SensorComponent->PersistentKnowledgeStore.Contains(TargetEntityId));

	// [v1.4.0] 실제 lifecycle로 Contact를 LastKnown→Lost→Removed까지 진행합니다.
	TargetActor->SetActorLocation(FVector(9000.0f, 0.0f, 0.0f));
	SensorComponent->ProcessPassiveScanActor(TargetActor, SensorComponent->FallbackSensorConfig);
	const double LastObservedWorldTimeSeconds = SensorComponent->RuntimeContacts[0].PublicContact.LastObservedWorldTimeSeconds;
	SensorComponent->AdvanceContactLifetimes(
		LastObservedWorldTimeSeconds + SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec,
		SensorComponent->FallbackSensorConfig);
	SensorComponent->PublishRuntimeSnapshot();
	SensorComponent->AdvanceContactLifetimes(
		LastObservedWorldTimeSeconds + SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec + 0.1,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("PersistentKnowledge 기존 Contact Removed"), SensorComponent->RuntimeContacts.IsEmpty());
	TestTrue(TEXT("Contact Removed 뒤 Store 유지"), SensorComponent->PersistentKnowledgeStore.Contains(TargetEntityId));

	TargetActor->SetActorLocation(FVector(2200.0f, 300.0f, 0.0f));
	const bool bReassociated = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("동일 Entity 새 Contact reassociation 성공"), bReassociated);
	TestEqual(TEXT("Reassociation 뒤 Contact 1개"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.4.0] Store에서 Persistent Fact를 복원한 새 Contact입니다.
	const FCFSensorContact& ReassociatedContact = SensorComponent->RuntimeContacts[0].PublicContact;
	TestTrue(TEXT("Reassociation은 새 ContactId 발급"), ReassociatedContact.ContactId != OriginalContactId);
	TestTrue(TEXT("Reassociation TargetEntityId exact 유지"), ReassociatedContact.TargetEntityId == TargetEntityId);
	TestEqual(TEXT("Reassociation DetailedScan 복원"), ReassociatedContact.InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestEqual(TEXT("Reassociation KnownTargetId 복원"), ReassociatedContact.KnownTargetId, DetailedContact.KnownTargetId);
	TestEqual(TEXT("Reassociation KnownDisplayName 복원"), ReassociatedContact.KnownDisplayName.ToString(), DetailedContact.KnownDisplayName.ToString());
	TestEqual(TEXT("Reassociation Analysis revision exact 복원"), ReassociatedContact.AnalysisCompletionRevision, DetailedContact.AnalysisCompletionRevision);
	TestEqual(TEXT("Reassociation allocator 증가 없음"), SensorComponent->NextAnalysisCompletionRevision, NextAnalysisRevisionBeforeReassociation);
	TestEqual(TEXT("Reassociation completion transition 증가 없음"), SensorComponent->LastScanCompletionTransitionRevision, CompletionTransitionBeforeReassociation);
	TestTrue(TEXT("Reassociation 뒤 Scan Attempt inactive"), !SensorComponent->bCurrentScanAttemptActive);
	TestTrue(TEXT("복원 DetailedScan은 기본 정책상 재스캔 거부"), !SensorComponent->StartTargetScan(TargetActor));

	// [v1.4.0] 동일 Entity에서 concrete Source TargetId가 달라진 provider 위반을 주입합니다.
	TargetActor->TargetId = TEXT("PersistentKnowledgeTargetConflict");
	TargetActor->TargetDisplayName = FText::FromString(TEXT("충돌 대상"));
	TargetActor->RefreshTestTargetConfig();
	// [v1.4.0] 같은 Entity에서 concrete stable TargetId가 충돌할 때 새 관측을 거부하는 fail-closed 결과입니다.
	const bool bIdentityConflictAccepted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("같은 Entity concrete TargetId 충돌은 fail-closed"), !bIdentityConflictAccepted);
	// [v1.4.0] Identity conflict가 기존 Stable Identity나 Knowledge를 덮어쓰지 않았는지 확인할 Record입니다.
	const UCFVehicleSensorComp::FCFSensorKnowledgeRecord* UnchangedRecord = SensorComponent->PersistentKnowledgeStore.Find(TargetEntityId);
	if (TestNotNull(TEXT("Identity conflict 뒤 Store Record 유지"), UnchangedRecord))
	{
		TestEqual(TEXT("Identity conflict가 StableTargetId를 덮어쓰지 않음"), UnchangedRecord->StableTargetId, DetailedContact.KnownTargetId);
	}

	// [v1.4.0] Terminal 재등장 검증 전에 원래 Source metadata로 되돌려 같은 Entity의 정상 stable metadata를 복구합니다.
	TargetActor->TargetId = TEXT("PersistentKnowledgeTarget");
	TargetActor->TargetDisplayName = FText::FromName(TargetActor->TargetId);
	TargetActor->RefreshTestTargetConfig();

	// [v1.4.0] 같은 Entity의 기존 Detailed Knowledge를 authoritative Terminal 상태로 전환하는 test-only Store 전이 결과입니다.
	const bool bTerminalUpsertAccepted = SensorComponent->UpsertPersistentKnowledgeRecord(
		TargetEntityId,
		SensorComponent->RuntimeContacts[0].SourceDisplayInfo,
		&SensorComponent->RuntimeContacts[0].PublicContact,
		true);
	TestTrue(TEXT("기존 Entity Persistent Record Terminal 전환 성공"), bTerminalUpsertAccepted);

	// [v1.4.0] Contact lifetime만 종료하고 Persistent Terminal Record는 남겨 실제 Removed 이후 재등장 전제와 같은 상태를 만듭니다.
	SensorComponent->UnbindAllContactDestroyedEvents();
	SensorComponent->RuntimeContacts.Reset();
	TestTrue(TEXT("Terminal 재등장 전 Runtime Contact 없음"), SensorComponent->RuntimeContacts.IsEmpty());
	TestTrue(TEXT("Terminal 재등장 전 Store 유지"), SensorComponent->PersistentKnowledgeStore.Contains(TargetEntityId));

	// [v1.4.0] 같은 healthy Actor/Entity가 다시 관측돼도 Terminal Store가 Contact admission 전에 차단하는 결과입니다.
	const bool bTerminalEntityReadmitted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("Terminal Entity 재등장은 fail-closed"), !bTerminalEntityReadmitted);
	TestTrue(TEXT("Terminal Entity 재등장 차단 뒤 Runtime Contact 없음"), SensorComponent->RuntimeContacts.IsEmpty());

	// [v1.4.0] Hard Reinitialize이 Persistent Store까지 clear하는지 확인하는 Runtime 재초기화 결과입니다.
	const bool bReinitializeAccepted = SensorComponent->InitializeSensorRuntime();
	TestTrue(TEXT("InitializeSensorRuntime 재호출 성공"), bReinitializeAccepted);
	TestTrue(TEXT("InitializeSensorRuntime은 Persistent Store clear"), SensorComponent->PersistentKnowledgeStore.IsEmpty());

	// [v1.4.0] Reset 경계를 별도로 검증하기 위해 Hard Reinitialize 이후 다시 생성하는 Detected Contact 결과입니다.
	const bool bPostReinitializeDetectionAccepted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("Reinitialize 뒤 같은 Entity 새 Contact 생성 가능"), bPostReinitializeDetectionAccepted);
	if (!SensorComponent->RuntimeContacts.IsEmpty())
	{
		SensorComponent->PromoteContactKnowledgeFromScanProgress(
			SensorComponent->RuntimeContacts[0],
			SensorComponent->FallbackSensorConfig.IdentifiedThreshold,
			SensorComponent->FallbackSensorConfig);
	}
	TestTrue(TEXT("Reset 검증 전 Persistent Record 재생성"), SensorComponent->PersistentKnowledgeStore.Contains(TargetEntityId));

	SensorComponent->ResetSensorRuntime();
	TestTrue(TEXT("ResetSensorRuntime은 Persistent Store clear"), SensorComponent->PersistentKnowledgeStore.IsEmpty());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
