// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-10-06
// Description: Phase 4 Vehicle Target Lock Runtime + Phase 5 Selection/Lock/Scan Gameplay Command Boundary focused Automation
// Scope: 기존 Targeting Runtime 계약과 public RequestLock 경로를 보존하면서 Pawn command facade의 Selection/Lock/Scan 독립성, Lock Input same-target toggle과 cross-target replacement를 검증합니다.
// Changelog:
// - v1.3.0: 실제 HandleTargetLockStarted 입력 경로에서 Locked A + Selected B가 A를 clear하지 않고 B Acquiring으로 replacement되는지, 같은 Selected B 재입력은 Lock-only clear로 토글되는지 회귀 검증 추가.
// - v1.2.0: TGT-P0-05 GameplayCommandBoundary를 추가해 Selected Target→Lock, Selection 변경/해제 뒤 Lock/Scan 비자동 추종, Lock-only clear, Scan-only cancel과 broad Active Detection 보존을 실제 public API로 검증.
// - v1.1.0: TGT-P0-04 RequestLockPublicPath를 추가해 Live 수락, 동일 대상 중복, Locked 중복, A→B 교체, LastKnown/NoContact/Invalid 거부와 replacement non-break를 실제 public RequestLock 경로로 검증.
// - v1.0.0: TGT-P0-01~03 Phase 4 focused regression을 최초 추가.
// Migration:
// - Test-only 변경입니다. HUD/Input/Guided Weapon source와 Content Asset은 생성·수정·저장하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFTargetingTypes.h"
#include "CFMissileTestTarget.h"
#include "CFTargetSelectComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleSensorComp.h"
#include "CFVehicleTargetingComp.h"
#include "CFSensorTypes.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// 지정 ContactId와 ContactState 하나만 가진 결정적 Sensor Snapshot을 만듭니다.
	FCFSensorSnapshot MakeTargetingSensorSnapshot(
		const FName ContactId,
		const ECFSensorContactState ContactState)
	{
		// Vehicle Targeting Runtime이 상태 전이에 사용할 단일 Contact입니다.
		FCFSensorContact SensorContact;
		SensorContact.ContactId = ContactId;
		SensorContact.ContactState = ContactState;

		// 다른 Sensor 의미를 섞지 않고 exact Contact 한 건만 제공할 Snapshot입니다.
		FCFSensorSnapshot SensorSnapshot;
		SensorSnapshot.Contacts.Add(SensorContact);
		return SensorSnapshot;
	}

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetingTypesTest,
	"CarFight.Targeting.TGT_P0_01.TypesContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Phase 4의 독립 Config와 actor-free Snapshot 상태별 공개 계약을 검증합니다.
bool FCFTargetingTypesTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 기본 Phase 4 fallback Targeting 설정입니다.
	FCFTargetingConfig TargetingConfig;
	TestTrue(TEXT("기본 Targeting Config는 유효해야 함"), TargetingConfig.IsValid());

	// Acquire 완료가 불가능한 무효 설정 복사본입니다.
	FCFTargetingConfig InvalidTargetingConfig = TargetingConfig;
	InvalidTargetingConfig.LockAcquireGainPerSec = 0.0f;
	TestFalse(TEXT("AcquireGain 0은 무효 Targeting Config"), InvalidTargetingConfig.IsValid());

	// 초기 Idle 공개 Snapshot입니다.
	FCFTargetingSnapshot IdleSnapshot;
	TestTrue(TEXT("기본 Idle Snapshot은 유효해야 함"), IdleSnapshot.IsPublicContractValid());

	// 유효한 Acquiring 공개 Snapshot입니다.
	FCFTargetingSnapshot AcquiringSnapshot;
	AcquiringSnapshot.State = ECFTargetLockState::Acquiring;
	AcquiringSnapshot.TargetContactId = TEXT("Contact_A");
	AcquiringSnapshot.LockProgress01 = 0.4f;
	TestTrue(TEXT("Contact와 0~1 미완료 progress가 있는 Acquiring Snapshot은 유효"), AcquiringSnapshot.IsPublicContractValid());

	// Acquiring에서 1.0 progress를 잘못 유지하는 무효 Snapshot입니다.
	FCFTargetingSnapshot InvalidAcquiringSnapshot = AcquiringSnapshot;
	InvalidAcquiringSnapshot.LockProgress01 = 1.0f;
	TestFalse(TEXT("Acquiring 상태에서 progress 1.0은 무효"), InvalidAcquiringSnapshot.IsPublicContractValid());

	// Break transition evidence를 보존한 유효 Idle Snapshot입니다.
	FCFTargetingSnapshot BrokenToIdleSnapshot;
	BrokenToIdleSnapshot.BreakTransitionRevision = 1;
	BrokenToIdleSnapshot.LastBreakReason = ECFTargetLockBreakReason::ContactLost;
	TestTrue(TEXT("Break evidence를 보존한 Idle Snapshot은 유효"), BrokenToIdleSnapshot.IsPublicContractValid());

	// Break Revision 없이 reason만 남은 무효 Snapshot입니다.
	FCFTargetingSnapshot InvalidBreakEvidenceSnapshot;
	InvalidBreakEvidenceSnapshot.LastBreakReason = ECFTargetLockBreakReason::ContactLost;
	TestFalse(TEXT("Revision 0에서 Break reason만 존재하면 무효"), InvalidBreakEvidenceSnapshot.IsPublicContractValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetingRuntimeTest,
	"CarFight.Targeting.TGT_P0_02.RuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Live/LastKnown/Lost/DestroyedHold Contact 변화가 Vehicle Target Lock 상태와 Break evidence에 정확히 반영되는지 검증합니다.
bool FCFTargetingRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Private 상태 전이를 public API 확장 없이 결정적으로 검증할 transient Targeting Component입니다.
	UCFVehicleTargetingComp* TargetingComponent = NewObject<UCFVehicleTargetingComp>();
	if (!TestNotNull(TEXT("Targeting Component 생성"), TargetingComponent))
	{
		return false;
	}

	TestTrue(TEXT("기본 Config로 Targeting Runtime 초기화 성공"), TargetingComponent->InitializeTargetingRuntime());

	// 이번 테스트에서 고정해 사용할 원래 ContactId입니다.
	const FName OriginalContactId(TEXT("Contact_A"));

	// Live Contact 한 건을 가진 Sensor Snapshot입니다.
	const FCFSensorSnapshot LiveSnapshot = MakeTargetingSensorSnapshot(
		OriginalContactId,
		ECFSensorContactState::Live);

	TargetingComponent->CurrentTargetingSnapshot.State = ECFTargetLockState::Acquiring;
	TargetingComponent->CurrentTargetingSnapshot.TargetContactId = OriginalContactId;
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 0.0f;
	TargetingComponent->CurrentTargetingSnapshot.LockQuality01 = 0.0f;
	TargetingComponent->AdvanceTargetingState(0.5f, LiveSnapshot);

	// Live 0.5초 뒤 공개 상태입니다.
	FCFTargetingSnapshot TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Acquire 0.5초 뒤 상태는 Acquiring"), TargetingSnapshot.State, ECFTargetLockState::Acquiring);
	TestTrue(TEXT("Acquire 0.5초 뒤 progress는 0.5"), FMath::IsNearlyEqual(TargetingSnapshot.LockProgress01, 0.5f));

	TargetingComponent->AdvanceTargetingState(0.5f, LiveSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Acquire 1초 누적 뒤 Locked"), TargetingSnapshot.State, ECFTargetLockState::Locked);
	TestTrue(TEXT("Locked 진입 시 progress 1"), FMath::IsNearlyEqual(TargetingSnapshot.LockProgress01, 1.0f));
	TestTrue(TEXT("Locked 진입 시 quality 1"), FMath::IsNearlyEqual(TargetingSnapshot.LockQuality01, 1.0f));

	// 같은 ContactId가 LastKnown으로 전환된 Sensor Snapshot입니다.
	const FCFSensorSnapshot LastKnownSnapshot = MakeTargetingSensorSnapshot(
		OriginalContactId,
		ECFSensorContactState::LastKnown);

	TargetingComponent->AdvanceTargetingState(1.0f, LastKnownSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("LastKnown 중 Quality가 남아 있으면 Locked 유지"), TargetingSnapshot.State, ECFTargetLockState::Locked);
	TestTrue(TEXT("LastKnown 1초 뒤 quality 0.5"), FMath::IsNearlyEqual(TargetingSnapshot.LockQuality01, 0.5f));

	TargetingComponent->AdvanceTargetingState(0.5f, LiveSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestTrue(TEXT("Live 복귀 0.5초 뒤 quality 1 회복"), FMath::IsNearlyEqual(TargetingSnapshot.LockQuality01, 1.0f));

	TargetingComponent->AdvanceTargetingState(2.0f, LastKnownSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Quality 소진 즉시 Idle"), TargetingSnapshot.State, ECFTargetLockState::Idle);
	TestEqual(TEXT("첫 Break Revision 1"), TargetingSnapshot.BreakTransitionRevision, 1);
	TestEqual(TEXT("첫 Break 사유 QualityDepleted"), TargetingSnapshot.LastBreakReason, ECFTargetLockBreakReason::QualityDepleted);
	TestTrue(TEXT("Break 뒤 공개 Snapshot 계약 유효"), TargetingSnapshot.IsPublicContractValid());

	// 기존 Break evidence를 유지한 채 다시 Locked 상태를 구성합니다.
	TargetingComponent->CurrentTargetingSnapshot.State = ECFTargetLockState::Locked;
	TargetingComponent->CurrentTargetingSnapshot.TargetContactId = OriginalContactId;
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 1.0f;
	TargetingComponent->CurrentTargetingSnapshot.LockQuality01 = 1.0f;

	// 같은 ContactId가 Lost 상태인 Sensor Snapshot입니다.
	const FCFSensorSnapshot LostSnapshot = MakeTargetingSensorSnapshot(
		OriginalContactId,
		ECFSensorContactState::Lost);

	TargetingComponent->AdvanceTargetingState(0.1f, LostSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Lost는 즉시 Idle"), TargetingSnapshot.State, ECFTargetLockState::Idle);
	TestEqual(TEXT("두 번째 Break Revision 2"), TargetingSnapshot.BreakTransitionRevision, 2);
	TestEqual(TEXT("Lost Break 사유 ContactLost"), TargetingSnapshot.LastBreakReason, ECFTargetLockBreakReason::ContactLost);

	TargetingComponent->CurrentTargetingSnapshot.State = ECFTargetLockState::Locked;
	TargetingComponent->CurrentTargetingSnapshot.TargetContactId = OriginalContactId;
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 1.0f;
	TargetingComponent->CurrentTargetingSnapshot.LockQuality01 = 1.0f;

	// 같은 ContactId가 DestroyedHold인 Sensor Snapshot입니다.
	const FCFSensorSnapshot DestroyedSnapshot = MakeTargetingSensorSnapshot(
		OriginalContactId,
		ECFSensorContactState::DestroyedHold);

	TargetingComponent->AdvanceTargetingState(0.1f, DestroyedSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("DestroyedHold는 즉시 Idle"), TargetingSnapshot.State, ECFTargetLockState::Idle);
	TestEqual(TEXT("세 번째 Break Revision 3"), TargetingSnapshot.BreakTransitionRevision, 3);
	TestEqual(TEXT("Destroyed Break 사유 TargetDestroyed"), TargetingSnapshot.LastBreakReason, ECFTargetLockBreakReason::TargetDestroyed);

	TargetingComponent->CurrentTargetingSnapshot.State = ECFTargetLockState::Locked;
	TargetingComponent->CurrentTargetingSnapshot.TargetContactId = OriginalContactId;
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 1.0f;
	TargetingComponent->CurrentTargetingSnapshot.LockQuality01 = 1.0f;

	// Lost 뒤 재발견을 흉내 내는 서로 다른 새 ContactId Snapshot입니다.
	const FCFSensorSnapshot NewContactSnapshot = MakeTargetingSensorSnapshot(
		TEXT("Contact_B"),
		ECFSensorContactState::Live);

	TargetingComponent->AdvanceTargetingState(0.1f, NewContactSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("새 ContactId는 기존 Lock에 자동 재연결하지 않고 Idle"), TargetingSnapshot.State, ECFTargetLockState::Idle);
	TestEqual(TEXT("자동 재연결 금지 경계는 ContactLost Break를 1회 기록"), TargetingSnapshot.BreakTransitionRevision, 4);
	TestEqual(TEXT("새 ContactId 미일치 Break 사유 ContactLost"), TargetingSnapshot.LastBreakReason, ECFTargetLockBreakReason::ContactLost);

	TargetingComponent->CurrentTargetingSnapshot.State = ECFTargetLockState::Locked;
	TargetingComponent->CurrentTargetingSnapshot.TargetContactId = OriginalContactId;
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 1.0f;
	TargetingComponent->CurrentTargetingSnapshot.LockQuality01 = 1.0f;

	// 수동 Clear 전 Break Revision입니다.
	const int32 BreakRevisionBeforeManualClear = TargetingComponent->GetTargetingSnapshot().BreakTransitionRevision;
	TestTrue(TEXT("Locked 상태 수동 Clear 성공"), TargetingComponent->ClearLock());
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("수동 Clear 뒤 Idle"), TargetingSnapshot.State, ECFTargetLockState::Idle);
	TestEqual(TEXT("수동 Clear는 Break Revision을 증가시키지 않음"), TargetingSnapshot.BreakTransitionRevision, BreakRevisionBeforeManualClear);
	TestFalse(TEXT("이미 Idle이면 추가 Clear는 false"), TargetingComponent->ClearLock());

	TargetingComponent->CurrentTargetingSnapshot.State = ECFTargetLockState::Acquiring;
	TargetingComponent->CurrentTargetingSnapshot.TargetContactId = OriginalContactId;
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 0.6f;
	TargetingComponent->CurrentTargetingSnapshot.LockQuality01 = 0.0f;
	TargetingComponent->AdvanceTargetingState(0.1f, LostSnapshot);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Acquiring 중 Lost는 Break 없이 Idle"), TargetingSnapshot.State, ECFTargetLockState::Idle);
	TestEqual(TEXT("Acquiring 취소는 기존 Break Revision 유지"), TargetingSnapshot.BreakTransitionRevision, BreakRevisionBeforeManualClear);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetingRequestLockTest,
	"CarFight.Targeting.TGT_P0_04.RequestLockPublicPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 실제 Sensor Actor→ContactId bridge와 public RequestLock API를 통해 Live/duplicate/replacement/rejection 계약을 검증합니다.
bool FCFTargetingRequestLockTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Sensor와 Targeting Component를 실제 등록 상태로 보유할 transient World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RequestLock 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// Sensor와 Targeting Component를 함께 소유할 테스트 Actor입니다.
	AActor* TargetingOwner = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("RequestLock 테스트 Owner 생성"), TargetingOwner))
	{
		return false;
	}

	// RequestLock이 실제로 조회할 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(TargetingOwner, TEXT("TargetingRequestSensor"));
	// public RequestLock API를 실행할 Targeting Component입니다.
	UCFVehicleTargetingComp* TargetingComponent = NewObject<UCFVehicleTargetingComp>(TargetingOwner, TEXT("TargetingRequestRuntime"));
	if (!TestNotNull(TEXT("RequestLock Sensor Component 생성"), SensorComponent)
		|| !TestNotNull(TEXT("RequestLock Targeting Component 생성"), TargetingComponent))
	{
		return false;
	}

	TargetingOwner->AddInstanceComponent(SensorComponent);
	SensorComponent->RegisterComponent();
	TargetingOwner->AddInstanceComponent(TargetingComponent);
	TargetingComponent->RegisterComponent();

	TestTrue(TEXT("RequestLock 테스트 Sensor Runtime 초기화"), SensorComponent->InitializeSensorRuntime());
	TestTrue(TEXT("RequestLock 테스트 Targeting Runtime 초기화"), TargetingComponent->InitializeTargetingRuntime());

	// 실제 RequestLock 입력으로 사용할 Live Contact A 대상입니다.
	AActor* TargetActorA = TestWorld->SpawnActor<AActor>();
	// 단일 Lock replacement를 검증할 Live Contact B 대상입니다.
	AActor* TargetActorB = TestWorld->SpawnActor<AActor>();
	// non-Live 거부를 검증할 LastKnown Contact C 대상입니다.
	AActor* TargetActorC = TestWorld->SpawnActor<AActor>();
	// Sensor Contact가 없는 거부 경로를 검증할 대상입니다.
	AActor* NoContactActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("RequestLock Target A 생성"), TargetActorA)
		|| !TestNotNull(TEXT("RequestLock Target B 생성"), TargetActorB)
		|| !TestNotNull(TEXT("RequestLock Target C 생성"), TargetActorC)
		|| !TestNotNull(TEXT("RequestLock NoContact Target 생성"), NoContactActor))
	{
		return false;
	}

	// Sensor private Runtime Contact와 public Snapshot을 같은 exact ContactId/Actor로 구성하는 테스트 전용 helper입니다.
	auto AddSensorContact = [SensorComponent](
		AActor* TargetActor,
		const FName ContactId,
		const ECFSensorContactState ContactState)
	{
		// Actor→ContactId bridge의 private truth를 구성할 Runtime Contact입니다.
		UCFVehicleSensorComp::FCFSensorContactRuntime& RuntimeContact = SensorComponent->RuntimeContacts.AddDefaulted_GetRef();
		RuntimeContact.TargetActor = TargetActor;
		RuntimeContact.PublicContact.ContactId = ContactId;
		RuntimeContact.PublicContact.ContactState = ContactState;
	};

	AddSensorContact(TargetActorA, TEXT("Contact_A"), ECFSensorContactState::Live);
	AddSensorContact(TargetActorB, TEXT("Contact_B"), ECFSensorContactState::Live);
	AddSensorContact(TargetActorC, TEXT("Contact_C"), ECFSensorContactState::LastKnown);
	SensorComponent->PublishRuntimeSnapshot();

	TestEqual(
		TEXT("Live Contact A RequestLock은 Accepted"),
		TargetingComponent->RequestLock(TargetActorA),
		ECFTargetLockRequestResult::Accepted);

	// 첫 public RequestLock 직후 상태입니다.
	FCFTargetingSnapshot TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Live A 수락 뒤 Acquiring"), TargetingSnapshot.State, ECFTargetLockState::Acquiring);
	TestEqual(TEXT("Live A 수락 뒤 Contact_A 소유"), TargetingSnapshot.TargetContactId, FName(TEXT("Contact_A")));

	// 동일 A 중복 요청이 진행률을 재시작하지 않는지 확인하기 위해 미완료 진행률을 결정적으로 부여합니다.
	TargetingComponent->CurrentTargetingSnapshot.LockProgress01 = 0.4f;
	TestEqual(
		TEXT("Acquiring A 중복 RequestLock은 AlreadyAcquiring"),
		TargetingComponent->RequestLock(TargetActorA),
		ECFTargetLockRequestResult::AlreadyAcquiring);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestTrue(TEXT("Acquiring A 중복 요청은 progress 유지"), FMath::IsNearlyEqual(TargetingSnapshot.LockProgress01, 0.4f));

	// public RequestLock으로 시작된 A를 정상 상태 머신을 통해 Locked로 진행합니다.
	TargetingComponent->AdvanceTargetingState(1.0f, SensorComponent->GetSensorSnapshot());
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("A acquire 진행 뒤 Locked"), TargetingSnapshot.State, ECFTargetLockState::Locked);
	TestEqual(
		TEXT("Locked A 중복 RequestLock은 AlreadyLocked"),
		TargetingComponent->RequestLock(TargetActorA),
		ECFTargetLockRequestResult::AlreadyLocked);

	// 명시적 A→B 교체는 유지조건 붕괴 Break가 아니므로 기존 Break Revision을 증가시키지 않아야 합니다.
	const int32 BreakRevisionBeforeReplacement = TargetingComponent->GetTargetingSnapshot().BreakTransitionRevision;
	TestEqual(
		TEXT("Locked A에서 Live B RequestLock은 Accepted"),
		TargetingComponent->RequestLock(TargetActorB),
		ECFTargetLockRequestResult::Accepted);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("A→B 교체 뒤 새 상태는 Acquiring"), TargetingSnapshot.State, ECFTargetLockState::Acquiring);
	TestEqual(TEXT("A→B 교체 뒤 Contact_B 소유"), TargetingSnapshot.TargetContactId, FName(TEXT("Contact_B")));
	TestTrue(TEXT("A→B 교체는 progress 0부터 시작"), FMath::IsNearlyZero(TargetingSnapshot.LockProgress01));
	TestEqual(TEXT("A→B 교체는 Break Revision 비증가"), TargetingSnapshot.BreakTransitionRevision, BreakRevisionBeforeReplacement);

	TestEqual(
		TEXT("LastKnown Contact C RequestLock은 ContactNotLive"),
		TargetingComponent->RequestLock(TargetActorC),
		ECFTargetLockRequestResult::ContactNotLive);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("LastKnown 거부 뒤 기존 Contact_B Acquire 유지"), TargetingSnapshot.TargetContactId, FName(TEXT("Contact_B")));

	TestEqual(
		TEXT("Sensor Contact 없는 Actor RequestLock은 NoSensorContact"),
		TargetingComponent->RequestLock(NoContactActor),
		ECFTargetLockRequestResult::NoSensorContact);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("NoContact 거부 뒤 기존 Contact_B Acquire 유지"), TargetingSnapshot.TargetContactId, FName(TEXT("Contact_B")));

	TestEqual(
		TEXT("nullptr RequestLock은 InvalidTarget"),
		TargetingComponent->RequestLock(nullptr),
		ECFTargetLockRequestResult::InvalidTarget);
	TargetingSnapshot = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("InvalidTarget 거부 뒤 기존 Contact_B Acquire 유지"), TargetingSnapshot.TargetContactId, FName(TEXT("Contact_B")));
	TestTrue(TEXT("RequestLock public-path 최종 Snapshot 계약 유효"), TargetingSnapshot.IsPublicContractValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetingPawnFoundationTest,
	"CarFight.Targeting.TGT_P0_03.PawnFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// VehiclePawn CDO에 TargetSelect/Sensor와 독립적인 VehicleTargetingComp 기본 서브오브젝트가 실제 존재하는지 검증합니다.
bool FCFTargetingPawnFoundationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Blueprint/instance 생성 없이 기본 서브오브젝트 구성을 검증할 VehiclePawn CDO입니다.
	const ACFVehiclePawn* VehiclePawnCDO = GetDefault<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("VehiclePawn CDO 생성"), VehiclePawnCDO))
	{
		return false;
	}

	TestNotNull(TEXT("기존 TargetSelectComp 보존"), VehiclePawnCDO->GetTargetSelectComp());
	TestNotNull(TEXT("기존 VehicleSensorComp 보존"), VehiclePawnCDO->GetVehicleSensorComp());
	TestNotNull(TEXT("신규 VehicleTargetingComp 기본 서브오브젝트 존재"), VehiclePawnCDO->GetVehicleTargetingComp());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetingGameplayCommandBoundaryTest,
	"CarFight.Targeting.TGT_P0_05.GameplayCommandBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Phase 5 Pawn facade가 Selection/Lock/Scan/Active Detection을 암묵적으로 결합하지 않는지 public API만으로 검증합니다.
bool FCFTargetingGameplayCommandBoundaryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Phase 5 command boundary를 실제 Pawn public API로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Phase 5 Gameplay Command 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// BP 조립 없이 C++ 기본 Pawn만 생성하므로 SM_Body 누락 로그 1회는 이 테스트의 의도된 전제입니다.
	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on Phase5CommandVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	// TargetSelect/Sensor/Targeting 기본 서브오브젝트와 Pawn facade를 함께 검증할 차량입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("Phase5CommandVehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	if (!TestNotNull(TEXT("Phase 5 Gameplay Command 차량 Pawn 생성"), VehiclePawn))
	{
		return false;
	}

	// Selection Authority인 Pawn 기본 TargetSelect Component입니다.
	UCFTargetSelectComp* TargetSelectComponent = VehiclePawn->GetTargetSelectComp();
	// Contact/Scan/Detection Authority인 Pawn 기본 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = VehiclePawn->GetVehicleSensorComp();
	// Vehicle Lock Authority인 Pawn 기본 Targeting Component입니다.
	UCFVehicleTargetingComp* TargetingComponent = VehiclePawn->GetVehicleTargetingComp();
	if (!TestNotNull(TEXT("Phase 5 TargetSelect Component 존재"), TargetSelectComponent)
		|| !TestNotNull(TEXT("Phase 5 Sensor Component 존재"), SensorComponent)
		|| !TestNotNull(TEXT("Phase 5 Targeting Component 존재"), TargetingComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// Scan/Detection command를 asset-free로 실행할 유효 Fallback Sensor Config입니다.
	SensorComponent->ResetSensorRuntime();
	SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = 1000.0f;
	SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 5000.0f;
	SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 1000.0f;
	SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
	SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = 64;
	SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = 10.0f;
	SensorComponent->FallbackSensorConfig.DestroyedHoldTimeSec = 1.0f;
	SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = 3.0f;
	SensorComponent->FallbackSensorConfig.AnalysisGainPerSec = 0.5f;
	SensorComponent->FallbackSensorConfig.AnalysisDecayPerSec = 0.2f;
	SensorComponent->FallbackSensorConfig.IdentifiedThreshold = 0.5f;
	SensorComponent->FallbackSensorConfig.DetailedScanThreshold = 1.0f;
	TestTrue(TEXT("Phase 5 Fallback Sensor Config 유효"), SensorComponent->FallbackSensorConfig.IsValid());
	TestTrue(TEXT("Phase 5 Sensor Runtime 명시 초기화"), SensorComponent->InitializeSensorRuntime());

	TargetingComponent->ResetTargetingRuntime();
	TestTrue(TEXT("Phase 5 Targeting Runtime 명시 초기화"), TargetingComponent->InitializeTargetingRuntime());

	// 첫 Selection/Lock/Scan command가 캡처할 Target A입니다.
	FActorSpawnParameters TargetASpawnParameters;
	TargetASpawnParameters.Name = TEXT("Phase5CommandTargetA");
	ACFMissileTestTarget* TargetActorA = TestWorld->SpawnActor<ACFMissileTestTarget>(
		ACFMissileTestTarget::StaticClass(),
		FVector(3000.0f, 0.0f, 0.0f),
		FRotator::ZeroRotator,
		TargetASpawnParameters);

	// Selection만 변경했을 때 기존 Lock/Scan이 자동 추종하지 않는지 검증할 Target B입니다.
	FActorSpawnParameters TargetBSpawnParameters;
	TargetBSpawnParameters.Name = TEXT("Phase5CommandTargetB");
	ACFMissileTestTarget* TargetActorB = TestWorld->SpawnActor<ACFMissileTestTarget>(
		ACFMissileTestTarget::StaticClass(),
		FVector(3200.0f, 300.0f, 0.0f),
		FRotator::ZeroRotator,
		TargetBSpawnParameters);
	if (!TestNotNull(TEXT("Phase 5 Target A 생성"), TargetActorA)
		|| !TestNotNull(TEXT("Phase 5 Target B 생성"), TargetActorB))
	{
		VehiclePawn->Destroy();
		return false;
	}

	TargetActorA->TargetId = TEXT("Phase5CommandTargetA");
	TargetActorA->TargetDisplayName = FText::FromString(TEXT("Phase 5 Command Target A"));
	TargetActorA->RefreshTestTargetConfig();
	TargetActorB->TargetId = TEXT("Phase5CommandTargetB");
	TargetActorB->TargetDisplayName = FText::FromString(TEXT("Phase 5 Command Target B"));
	TargetActorB->RefreshTestTargetConfig();

	TestTrue(
		TEXT("Phase 5 Target A 선택"),
		TargetSelectComponent->SetSelectedTarget(TargetActorA, TargetSelectComponent->GetDefaultSelectionContext()));
	TestEqual(TEXT("Phase 5 Selected Target A 확인"), TargetSelectComponent->GetSelectedTargetActor(), static_cast<AActor*>(TargetActorA));

	// Scan-only cancel이 broad Detection을 보존하는지 함께 검증하기 위해 두 Operation을 독립적으로 시작합니다.
	TestTrue(TEXT("Phase 5 broad Active Detection 시작"), SensorComponent->StartActiveDetectionPulse());
	TestTrue(TEXT("Phase 5 Selected Target A Scan 시작"), VehiclePawn->RequestStartTargetScan());
	TestTrue(TEXT("Phase 5 Target Scan 실행 상태"), SensorComponent->IsTargetScanRunning());
	TestTrue(TEXT("Phase 5 broad Active Detection 실행 상태"), SensorComponent->IsActiveDetectionPulseRunning());

	// Scan 시작 순간 Sensor가 캡처한 Target A ContactId입니다.
	const FName CapturedScanContactId = SensorComponent->GetSensorSnapshot().ScanAttempt.TargetContactId;
	TestFalse(TEXT("Phase 5 Scan ContactId 유효"), CapturedScanContactId.IsNone());

	TestEqual(
		TEXT("Phase 5 Selected Target A Lock 요청 Accepted"),
		VehiclePawn->RequestLockSelectedTarget(),
		ECFTargetLockRequestResult::Accepted);
	// Lock command가 선택 A를 캡처한 뒤 소유하는 ContactId입니다.
	const FCFTargetingSnapshot TargetingSnapshotAfterLock = TargetingComponent->GetTargetingSnapshot();
	TestEqual(TEXT("Phase 5 Lock ContactId와 Scan ContactId 일치"), TargetingSnapshotAfterLock.TargetContactId, CapturedScanContactId);
	TestEqual(TEXT("Phase 5 Lock 요청 뒤 Acquiring"), TargetingSnapshotAfterLock.State, ECFTargetLockState::Acquiring);

	TestTrue(
		TEXT("Phase 5 Selection을 Target B로 변경"),
		TargetSelectComponent->SetSelectedTarget(TargetActorB, TargetSelectComponent->GetDefaultSelectionContext()));
	TestEqual(TEXT("Phase 5 현재 Selection은 Target B"), TargetSelectComponent->GetSelectedTargetActor(), static_cast<AActor*>(TargetActorB));
	TestEqual(TEXT("Selection 변경 뒤 Lock은 Target A 유지"), TargetingComponent->GetTargetingSnapshot().TargetContactId, CapturedScanContactId);
	TestEqual(TEXT("Selection 변경 뒤 Scan은 Target A 유지"), SensorComponent->GetSensorSnapshot().ScanAttempt.TargetContactId, CapturedScanContactId);

	TestTrue(TEXT("Phase 5 Selection 수동 해제"), VehiclePawn->ClearSelectedTargetManually());
	TestNull(TEXT("Phase 5 Selection 수동 해제 확인"), TargetSelectComponent->GetSelectedTargetActor());
	TestEqual(TEXT("Selection 해제 뒤 Lock은 Target A 유지"), TargetingComponent->GetTargetingSnapshot().TargetContactId, CapturedScanContactId);
	TestEqual(TEXT("Selection 해제 뒤 Scan은 Target A 유지"), SensorComponent->GetSensorSnapshot().ScanAttempt.TargetContactId, CapturedScanContactId);

	// Manual Lock clear는 Break가 아니며 Scan/Detection에는 side effect를 만들지 않아야 합니다.
	const int32 BreakRevisionBeforeManualClear = TargetingComponent->GetTargetingSnapshot().BreakTransitionRevision;
	TestTrue(TEXT("Phase 5 Lock-only 수동 해제 승인"), VehiclePawn->RequestClearTargetLock());
	TestEqual(TEXT("Phase 5 Lock-only 해제 뒤 Idle"), TargetingComponent->GetTargetingSnapshot().State, ECFTargetLockState::Idle);
	TestEqual(TEXT("Phase 5 Lock-only 해제는 Break Revision 비증가"), TargetingComponent->GetTargetingSnapshot().BreakTransitionRevision, BreakRevisionBeforeManualClear);
	TestTrue(TEXT("Lock-only 해제 뒤 Target Scan 유지"), SensorComponent->IsTargetScanRunning());
	TestTrue(TEXT("Lock-only 해제 뒤 Active Detection 유지"), SensorComponent->IsActiveDetectionPulseRunning());

	TestEqual(
		TEXT("Selection 없는 Lock command는 InvalidTarget"),
		VehiclePawn->RequestLockSelectedTarget(),
		ECFTargetLockRequestResult::InvalidTarget);
	TestEqual(TEXT("Selection 없는 Lock 거부 뒤 Idle 유지"), TargetingComponent->GetTargetingSnapshot().State, ECFTargetLockState::Idle);

	TestTrue(TEXT("Phase 5 Target Scan-only 취소 승인"), VehiclePawn->RequestCancelTargetScan());
	TestFalse(TEXT("Scan-only 취소 뒤 Target Scan 비활성"), SensorComponent->IsTargetScanRunning());
	TestTrue(TEXT("Scan-only 취소 뒤 broad Active Detection 유지"), SensorComponent->IsActiveDetectionPulseRunning());
	TestNull(TEXT("Scan-only 취소가 Selection을 만들지 않음"), TargetSelectComponent->GetSelectedTargetActor());
	TestEqual(TEXT("Scan-only 취소가 Lock을 만들지 않음"), TargetingComponent->GetTargetingSnapshot().State, ECFTargetLockState::Idle);
	TestFalse(TEXT("활성 Target Scan 없을 때 중복 Scan-only 취소 거부"), VehiclePawn->RequestCancelTargetScan());

	TestTrue(TEXT("Phase 5 broad Sensor Operation 정리 승인"), VehiclePawn->RequestCancelSensorOperations());
	TestFalse(TEXT("broad 취소 뒤 Active Detection 비활성"), SensorComponent->IsActiveDetectionPulseRunning());

	// [v1.3.0] Lock 입력이 같은 대상 토글과 다른 대상 replacement를 구분하는지 실제 handler로 검증합니다.
	TestTrue(
		TEXT("Lock input retarget 준비 Target A 재선택"),
		TargetSelectComponent->SetSelectedTarget(TargetActorA, TargetSelectComponent->GetDefaultSelectionContext()));
	TestTrue(TEXT("Lock input retarget 준비 Target A Scan 시작"), VehiclePawn->RequestStartTargetScan());
	TestEqual(
		TEXT("Lock input retarget 준비 Target A Lock Accepted"),
		VehiclePawn->RequestLockSelectedTarget(),
		ECFTargetLockRequestResult::Accepted);
	TestEqual(TEXT("Lock input retarget 준비 Target A Acquiring"), TargetingComponent->GetTargetingSnapshot().State, ECFTargetLockState::Acquiring);
	TestTrue(TEXT("Lock input retarget 준비 Target A Scan 취소"), VehiclePawn->RequestCancelTargetScan());

	TestTrue(
		TEXT("Lock input retarget Target B 선택"),
		TargetSelectComponent->SetSelectedTarget(TargetActorB, TargetSelectComponent->GetDefaultSelectionContext()));
	TestTrue(TEXT("Lock input retarget Target B Scan 시작"), VehiclePawn->RequestStartTargetScan());
	// Target B Target Scan이 확보한 실제 Sensor ContactId입니다.
	const FName TargetBContactId = SensorComponent->GetSensorSnapshot().ScanAttempt.TargetContactId;
	TestFalse(TEXT("Lock input retarget Target B ContactId 유효"), TargetBContactId.IsNone());

	VehiclePawn->HandleTargetLockStarted(FInputActionValue());
	TestEqual(TEXT("다른 Selected Target Lock 입력은 B Acquiring"), TargetingComponent->GetTargetingSnapshot().State, ECFTargetLockState::Acquiring);
	TestEqual(TEXT("다른 Selected Target Lock 입력은 B Contact로 replacement"), TargetingComponent->GetTargetingSnapshot().TargetContactId, TargetBContactId);

	VehiclePawn->HandleTargetLockStarted(FInputActionValue());
	TestEqual(TEXT("같은 Selected Target 재입력은 Lock-only clear"), TargetingComponent->GetTargetingSnapshot().State, ECFTargetLockState::Idle);
	TestTrue(TEXT("Lock input retarget Target B Scan 정리"), VehiclePawn->RequestCancelTargetScan());

	TargetActorB->Destroy();
	TargetActorA->Destroy();
	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
