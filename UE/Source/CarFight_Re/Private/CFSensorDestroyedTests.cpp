// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-036 SEN-P0-05 Destroyed Contact asset-free Automation
// Scope: VehicleHealth authoritative 파괴 확정, DestroyedHold 보존·만료와 weak Actor invalid 비파괴 계약을 검증합니다.
// Changelog:
// - v1.0.0: DestroyedHold와 InvalidActor 2개 Automation을 최초 추가.
// Migration:
// - transient Editor World, C++ ACFMissileTestTarget, transient VehicleHealthComp/DamageData만 사용하며 Content Asset을 생성·수정·저장하지 않습니다.
// - 파괴 확정은 실제 UCFVehicleHealthComp::ApplyIntegrityDamageFromHitContext → OnVehicleDestroyed 경로로 발생시킵니다.
// - Actor Destroy/weak invalid는 Sensor 파괴 확정으로 사용하지 않고 기존 LastKnown/Lost 의미를 유지하는지 별도로 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFDamageData.h"
#include "CFMissileTestTarget.h"
#include "CFVehicleHealthComp.h"
#include "CFVehicleSensorComp.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] 명시 이름·위치·TargetId를 가진 C++ TargetSelectable Actor를 transient World에 생성합니다.
	ACFMissileTestTarget* SpawnDestroyedSensorTarget(
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

		// [v1.0.0] Sensor Destroyed lifecycle의 TargetSelectable source로 사용할 경량 테스트 Actor입니다.
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

	// [v1.0.0] 테스트 Target에 실제 VehicleHealth authoritative 파괴 owner를 transient Component로 추가합니다.
	UCFVehicleHealthComp* AttachDestroyedSensorHealth(
		ACFMissileTestTarget* TargetActor,
		const FName ComponentName)
	{
		if (!TargetActor)
		{
			return nullptr;
		}

		// [v1.0.0] Content Asset 없이 실제 파괴 이벤트를 발생시킬 VehicleHealth Component입니다.
		UCFVehicleHealthComp* HealthComponent = NewObject<UCFVehicleHealthComp>(TargetActor, ComponentName);
		if (!HealthComponent)
		{
			return nullptr;
		}

		TargetActor->AddInstanceComponent(HealthComponent);
		HealthComponent->RegisterComponent();
		HealthComponent->InitializeFromVehicleData(nullptr);
		return HealthComponent;
	}

	// [v1.0.0] plain Actor Owner에 transient Sensor Component를 붙이고 DestroyedHold 검증용 설정으로 초기화합니다.
	UCFVehicleSensorComp* AttachDestroyedSensorComponent(
		AActor* SensorOwner,
		const float DestroyedHoldTimeSec)
	{
		if (!SensorOwner)
		{
			return nullptr;
		}

		// [v1.0.0] 저장 Asset 없이 실제 Sensor Runtime을 사용하는 테스트 Component입니다.
		UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(SensorOwner, TEXT("VehicleSensorComp_DestroyedAutomation"));
		if (!SensorComponent)
		{
			return nullptr;
		}

		SensorOwner->AddInstanceComponent(SensorComponent);
		SensorComponent->RegisterComponent();
		SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = 5000.0f;
		SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
		SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = 1;
		SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = 10.0f;
		SensorComponent->FallbackSensorConfig.DestroyedHoldTimeSec = DestroyedHoldTimeSec;
		SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = 0.0f;
		SensorComponent->FallbackSensorConfig.AnalysisGainPerSec = 0.0f;
		SensorComponent->FallbackSensorConfig.AnalysisDecayPerSec = 0.0f;
		SensorComponent->FallbackSensorConfig.IdentifiedThreshold = 0.5f;
		SensorComponent->FallbackSensorConfig.DetailedScanThreshold = 1.0f;
		return SensorComponent->InitializeSensorRuntime() ? SensorComponent : nullptr;
	}

	// [v1.0.0] VehicleHealth의 실제 피해 경로로 Target을 한 번에 파괴하고 결과를 반환합니다.
	bool DestroySensorTargetThroughHealth(
		ACFMissileTestTarget* TargetActor,
		UCFVehicleHealthComp* HealthComponent,
		AActor* DamageInstigator,
		FCFDamageApplyResult& OutDamageResult)
	{
		if (!TargetActor || !HealthComponent)
		{
			return false;
		}

		// [v1.0.0] 저장 Asset 없이 VehicleHealth 피해 검증에 사용할 transient DamageData입니다.
		UCFDamageData* DamageData = NewObject<UCFDamageData>(GetTransientPackage(), TEXT("DA_SensorDestroyedAutomation"));
		if (!DamageData)
		{
			return false;
		}
		DamageData->DamageId = TEXT("SensorDestroyedAutomationDamage");
		DamageData->BaseDamage = 1000.0f;
		DamageData->bCanDamageSelf = false;

		// [v1.0.0] VehicleHealth authoritative 파괴 이벤트까지 도달할 실제 Blocking Hit 입력입니다.
		FCFDamageHitContext DamageHitContext;
		DamageHitContext.DamageData = DamageData;
		DamageHitContext.DamageId = DamageData->DamageId;
		DamageHitContext.HitActor = TargetActor;
		DamageHitContext.ImpactLocation = TargetActor->GetActorLocation();
		DamageHitContext.ImpactNormal = FVector::UpVector;
		DamageHitContext.IncomingDirection = FVector::ForwardVector;
		DamageHitContext.InstigatorActor = DamageInstigator;
		DamageHitContext.bBlockingHit = true;

		return HealthComponent->ApplyIntegrityDamageFromHitContext(
			DamageHitContext,
			1000.0f,
			OutDamageResult);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorDestroyedHoldTest,
	"CarFight.Sensor.SEN_P0_05.DestroyedHold",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] VehicleHealth 최초 파괴 이벤트가 기존 Contact를 같은 ID·Knowledge·마지막 신뢰 위치의 DestroyedHold로 만들고 별도 시간 뒤 제거하는지 검증합니다.
bool FCFSensorDestroyedHoldTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 저장 Asset 없이 DestroyedHold를 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("DestroyedHold 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Sensor Component를 소유할 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] 최초 2,000cm에서 신뢰 관측할 TargetSelectable Actor입니다.
	ACFMissileTestTarget* TargetActor = SpawnDestroyedSensorTarget(
		TestWorld,
		TEXT("SensorDestroyedHoldTarget"),
		FVector(2000.0f, 0.0f, 0.0f),
		TEXT("DestroyedHoldTarget"));
	if (!TestNotNull(TEXT("DestroyedHold Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("DestroyedHold Target 생성"), TargetActor))
	{
		return false;
	}

	// [v1.0.0] Target의 authoritative destruction truth와 이벤트를 소유할 Health Component입니다.
	UCFVehicleHealthComp* HealthComponent = AttachDestroyedSensorHealth(
		TargetActor,
		TEXT("VehicleHealthComp_DestroyedHoldAutomation"));

	// [v1.0.0] DestroyedHold를 1초 유지하도록 설정한 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = AttachDestroyedSensorComponent(SensorOwner, 1.0f);
	if (!TestNotNull(TEXT("DestroyedHold Health Component 생성"), HealthComponent)
		|| !TestNotNull(TEXT("DestroyedHold Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.0.0] 실제 Sensor 탐지 경로로 최초 Live Contact와 VehicleHealth event binding을 만든 결과입니다.
	const bool bInitialDetectionAccepted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("최초 Live Contact 생성"), bInitialDetectionAccepted);
	TestEqual(TEXT("최초 Runtime Contact 1개"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.0.0] 파괴 뒤에도 동일해야 하는 최초 ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[0].PublicContact.ContactId;

	// [v1.0.0] 파괴 이벤트가 실제 위치나 Impact 위치로 덮어쓰면 안 되는 마지막 신뢰 관측 위치입니다.
	const FVector OriginalLastKnownWorldLocation = SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation;

	// [v1.0.0] 파괴 이벤트가 변경하면 안 되는 마지막 Sensor 관측 시각입니다.
	const double OriginalLastObservedWorldTimeSeconds = SensorComponent->RuntimeContacts[0].PublicContact.LastObservedWorldTimeSeconds;

	// [v1.0.0] 이미 획득한 Sensor Knowledge 보존을 검증할 Identified Target ID입니다.
	SensorComponent->RuntimeContacts[0].PublicContact.InformationLevel = ECFTargetInfoLevel::Identified;
	SensorComponent->RuntimeContacts[0].PublicContact.KnownTargetId = TEXT("DestroyedHoldTarget");
	SensorComponent->RuntimeContacts[0].PublicContact.KnownDisplayName = FText::FromString(TEXT("파괴 보존 대상"));
	SensorComponent->RuntimeContacts[0].PublicContact.AnalysisProgress01 = 0.65f;

	// [v1.0.0] 파괴 뒤에도 유지돼야 할 획득 정보 단계입니다.
	const ECFTargetInfoLevel OriginalInformationLevel = SensorComponent->RuntimeContacts[0].PublicContact.InformationLevel;

	// [v1.0.0] 파괴 뒤에도 유지돼야 할 획득 Target ID입니다.
	const FName OriginalKnownTargetId = SensorComponent->RuntimeContacts[0].PublicContact.KnownTargetId;

	// [v1.0.0] 파괴 뒤에도 유지돼야 할 분석 진행률입니다.
	const float OriginalAnalysisProgress = SensorComponent->RuntimeContacts[0].PublicContact.AnalysisProgress01;

	// [v1.0.0] 마지막 Sensor 관측 이후 실제 Actor가 이동해도 파괴 확정 위치로 사용하지 않도록 만드는 미관측 이동입니다.
	TargetActor->SetActorLocation(FVector(3000.0f, 500.0f, 0.0f));

	// [v1.0.0] 실제 Health 경로의 파괴 전환 결과입니다.
	FCFDamageApplyResult DamageResult;
	const bool bDamageApplied = DestroySensorTargetThroughHealth(
		TargetActor,
		HealthComponent,
		SensorOwner,
		DamageResult);
	TestTrue(TEXT("VehicleHealth 파괴 피해 적용"), bDamageApplied);
	TestTrue(TEXT("이번 피해가 최초 파괴를 확정"), DamageResult.bDestroyedThisHit);
	TestTrue(TEXT("VehicleHealth authoritative 상태 Destroyed"), HealthComponent->IsDestroyed());

	TestEqual(TEXT("파괴 이벤트 뒤 Runtime Contact 1개 유지"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.0.0] 이벤트가 즉시 전환한 private Runtime Contact입니다.
	const UCFVehicleSensorComp::FCFSensorContactRuntime& DestroyedRuntimeContact = SensorComponent->RuntimeContacts[0];
	TestEqual(TEXT("ContactState DestroyedHold"), DestroyedRuntimeContact.PublicContact.ContactState, ECFSensorContactState::DestroyedHold);
	TestTrue(TEXT("bDestroyedConfirmed true"), DestroyedRuntimeContact.PublicContact.bDestroyedConfirmed);
	TestEqual(TEXT("DestroyedHold에서 같은 ContactId 보존"), DestroyedRuntimeContact.PublicContact.ContactId, OriginalContactId);
	TestEqual(TEXT("DestroyedHold에서 InformationLevel 보존"), DestroyedRuntimeContact.PublicContact.InformationLevel, OriginalInformationLevel);
	TestEqual(TEXT("DestroyedHold에서 KnownTargetId 보존"), DestroyedRuntimeContact.PublicContact.KnownTargetId, OriginalKnownTargetId);
	TestTrue(
		TEXT("DestroyedHold에서 AnalysisProgress 보존"),
		FMath::IsNearlyEqual(DestroyedRuntimeContact.PublicContact.AnalysisProgress01, OriginalAnalysisProgress, KINDA_SMALL_NUMBER));
	TestTrue(
		TEXT("DestroyedHold 위치는 파괴 시 실제 위치가 아니라 마지막 신뢰 위치 보존"),
		DestroyedRuntimeContact.PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));
	TestEqual(
		TEXT("파괴 이벤트는 마지막 Sensor 관측 시각을 변경하지 않음"),
		DestroyedRuntimeContact.PublicContact.LastObservedWorldTimeSeconds,
		OriginalLastObservedWorldTimeSeconds);

	// [v1.0.0] Health 이벤트 handler가 bounded cursor 대기 없이 즉시 게시한 공개 Snapshot입니다.
	const FCFSensorSnapshot ImmediateDestroyedSnapshot = SensorComponent->GetSensorSnapshot();
	TestEqual(TEXT("즉시 Destroyed Snapshot Contact 1개"), ImmediateDestroyedSnapshot.Contacts.Num(), 1);
	if (ImmediateDestroyedSnapshot.Contacts.Num() == 1)
	{
		TestEqual(TEXT("즉시 Snapshot 상태 DestroyedHold"), ImmediateDestroyedSnapshot.Contacts[0].ContactState, ECFSensorContactState::DestroyedHold);
		TestTrue(TEXT("즉시 Snapshot bDestroyedConfirmed true"), ImmediateDestroyedSnapshot.Contacts[0].bDestroyedConfirmed);
		TestEqual(TEXT("즉시 Snapshot ContactId 유지"), ImmediateDestroyedSnapshot.Contacts[0].ContactId, OriginalContactId);
		TestTrue(TEXT("즉시 Snapshot 공개 계약 유효"), ImmediateDestroyedSnapshot.IsPublicContractValid());
	}

	// [v1.0.0] bounded cursor와 독립된 DestroyedHold 수명 시작 시각입니다.
	const double DestroyedConfirmedWorldTimeSeconds = SensorComponent->RuntimeContacts[0].DestroyedConfirmedWorldTimeSeconds;

	// [v1.0.0] Hold 절반이 지난 합성 Sensor update 시각입니다.
	const double HalfHoldWorldTimeSeconds = DestroyedConfirmedWorldTimeSeconds + 0.5;
	SensorComponent->AdvanceContactLifetimes(
		HalfHoldWorldTimeSeconds,
		SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("0.5초 후 DestroyedHold 유지"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() == 1)
	{
		TestEqual(TEXT("0.5초 후 상태 DestroyedHold"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::DestroyedHold);
		TestTrue(
			TEXT("Hold 진행 중 마지막 신뢰 위치 고정"),
			SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));
	}

	// [v1.0.0] 파괴된 Actor를 다시 bounded 후보로 평가해도 Live로 되살아나거나 위치가 갱신되지 않는지 확인합니다.
	const bool bDestroyedCandidateHandled = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("이미 파괴된 기존 Contact 상태 확인 처리"), bDestroyedCandidateHandled);
	TestEqual(TEXT("재평가 뒤 Runtime Contact 1개 유지"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() == 1)
	{
		TestEqual(TEXT("재평가 뒤 DestroyedHold 유지"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::DestroyedHold);
		TestTrue(
			TEXT("재평가로 마지막 신뢰 위치를 갱신하지 않음"),
			SensorComponent->RuntimeContacts[0].PublicContact.LastKnownWorldLocation.Equals(OriginalLastKnownWorldLocation, KINDA_SMALL_NUMBER));
	}

	// [v1.0.0] 정확히 DestroyedHoldTimeSec가 만료된 합성 Sensor update 시각입니다.
	const double ExpiredHoldWorldTimeSeconds = DestroyedConfirmedWorldTimeSeconds + 1.0;
	SensorComponent->AdvanceContactLifetimes(
		ExpiredHoldWorldTimeSeconds,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("DestroyedHoldTimeSec 만료 뒤 Runtime Contact 제거"), SensorComponent->RuntimeContacts.IsEmpty());

	SensorComponent->PublishRuntimeSnapshot();
	TestTrue(TEXT("DestroyedHold 만료 뒤 공개 Snapshot Contact 없음"), SensorComponent->GetSensorSnapshot().Contacts.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorDestroyedInvalidActorTest,
	"CarFight.Sensor.SEN_P0_05.InvalidActor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Actor Destroy/weak invalid만으로 DestroyedHold가 되지 않고, 미관측 상태에서 이미 파괴된 새 Actor도 새 Destroyed Contact를 만들지 않는지 검증합니다.
bool FCFSensorDestroyedInvalidActorTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] weak invalid와 pre-destroyed 미탐지 계약을 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("InvalidActor 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Sensor Component를 소유할 일반 Actor입니다.
	AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
		AActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator);

	// [v1.0.0] Health 파괴 신호 없이 Actor Destroy만 수행할 Target입니다.
	ACFMissileTestTarget* InvalidTargetActor = SpawnDestroyedSensorTarget(
		TestWorld,
		TEXT("SensorInvalidActorTarget"),
		FVector(1500.0f, 0.0f, 0.0f),
		TEXT("InvalidActorTarget"));
	if (!TestNotNull(TEXT("InvalidActor Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("InvalidActor Target 생성"), InvalidTargetActor))
	{
		return false;
	}

	// [v1.0.0] 일반 Contact memory와 DestroyedHold를 함께 가진 테스트 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachDestroyedSensorComponent(SensorOwner, 1.0f);
	if (!TestNotNull(TEXT("InvalidActor Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.0.0] Health가 없는 Target도 정상 Live Contact로 탐지되는 기존 P0-02 계약 결과입니다.
	const bool bInitialDetectionAccepted = SensorComponent->ProcessPassiveScanActor(
		InvalidTargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("Health 없는 Target 최초 Live 탐지"), bInitialDetectionAccepted);
	TestEqual(TEXT("InvalidActor 최초 Contact 1개"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() != 1)
	{
		return false;
	}

	// [v1.0.0] Actor invalid 뒤에도 LastKnown으로 유지할 최초 ContactId입니다.
	const FName OriginalContactId = SensorComponent->RuntimeContacts[0].PublicContact.ContactId;

	// [v1.0.0] weak invalid 후 freshness를 진행할 기준 마지막 관측 시각입니다.
	const double OriginalObservedWorldTimeSeconds = SensorComponent->RuntimeContacts[0].PublicContact.LastObservedWorldTimeSeconds;

	// [v1.0.0] VehicleHealth authoritative signal 없이 Actor lifecycle만 종료한 결과입니다.
	const bool bActorDestroyRequested = InvalidTargetActor->Destroy();
	TestTrue(TEXT("테스트 Actor Destroy 요청 성공"), bActorDestroyRequested);

	SensorComponent->AdvanceContactLifetimes(
		OriginalObservedWorldTimeSeconds + 0.1,
		SensorComponent->FallbackSensorConfig);
	TestEqual(TEXT("weak invalid 뒤 Contact 1개 유지"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() == 1)
	{
		TestEqual(TEXT("weak invalid는 LastKnown 전환"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::LastKnown);
		TestTrue(TEXT("weak invalid는 bDestroyedConfirmed false"), !SensorComponent->RuntimeContacts[0].PublicContact.bDestroyedConfirmed);
		TestEqual(TEXT("weak invalid에서도 같은 ContactId"), SensorComponent->RuntimeContacts[0].PublicContact.ContactId, OriginalContactId);
	}

	SensorComponent->PublishRuntimeSnapshot();
	TestTrue(TEXT("weak invalid LastKnown Snapshot 공개 계약 유효"), SensorComponent->GetSensorSnapshot().IsPublicContractValid());

	// [v1.0.0] Sensor가 한 번도 Contact로 만든 적 없는 이미 파괴된 Target입니다.
	ACFMissileTestTarget* PreDestroyedTargetActor = SpawnDestroyedSensorTarget(
		TestWorld,
		TEXT("SensorPreDestroyedTarget"),
		FVector(2500.0f, 0.0f, 0.0f),
		TEXT("PreDestroyedTarget"));
	if (!TestNotNull(TEXT("PreDestroyed Target 생성"), PreDestroyedTargetActor))
	{
		return false;
	}

	// [v1.0.0] 미관측 Target의 authoritative 파괴 상태를 만들 Health Component입니다.
	UCFVehicleHealthComp* PreDestroyedHealthComponent = AttachDestroyedSensorHealth(
		PreDestroyedTargetActor,
		TEXT("VehicleHealthComp_PreDestroyedAutomation"));
	if (!TestNotNull(TEXT("PreDestroyed Health Component 생성"), PreDestroyedHealthComponent))
	{
		return false;
	}

	// [v1.0.0] Sensor Contact 생성 전에 실제 VehicleHealth 경로로 먼저 파괴한 결과입니다.
	FCFDamageApplyResult PreDestroyedDamageResult;
	const bool bPreDestroyDamageApplied = DestroySensorTargetThroughHealth(
		PreDestroyedTargetActor,
		PreDestroyedHealthComponent,
		SensorOwner,
		PreDestroyedDamageResult);
	TestTrue(TEXT("미관측 Target VehicleHealth 파괴 성공"), bPreDestroyDamageApplied);
	TestTrue(TEXT("미관측 Target authoritative Destroyed"), PreDestroyedHealthComponent->IsDestroyed());

	// [v1.0.0] 이미 파괴된 새 Actor를 처음 발견했다고 새로운 Destroyed Contact를 만들어서는 안 되는 평가 결과입니다.
	const bool bPreDestroyedCandidateAccepted = SensorComponent->ProcessPassiveScanActor(
		PreDestroyedTargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("이미 파괴된 미관측 Target은 새 Contact로 수락하지 않음"), !bPreDestroyedCandidateAccepted);
	TestEqual(TEXT("미관측 파괴 Target 평가 뒤 기존 Contact 수 유지"), SensorComponent->RuntimeContacts.Num(), 1);
	if (SensorComponent->RuntimeContacts.Num() == 1)
	{
		TestEqual(TEXT("기존 weak-invalid Contact는 계속 LastKnown"), SensorComponent->RuntimeContacts[0].PublicContact.ContactState, ECFSensorContactState::LastKnown);
		TestTrue(TEXT("기존 Contact에 파괴 확정 오염 없음"), !SensorComponent->RuntimeContacts[0].PublicContact.bDestroyedConfirmed);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
