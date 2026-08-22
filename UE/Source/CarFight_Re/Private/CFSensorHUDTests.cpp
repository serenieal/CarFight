// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-20
// Description: CF-FQ-036 Sensor Snapshot HUD + CF-FQ-032 UI-P0-08 Radar Range Foundation asset-free Automation
// Scope: Sensor Knowledge/Radar Snapshot, DestroyedHold와 Scanner Range Profile 기반 Display Range/Zoom/normalized selected-edge 계약을 검증합니다.
// Changelog:
// - v1.1.0: RadarRangeFoundationContract를 추가해 transient Scanner Range Profile의 explicit default, 50m→25m Zoom In, 선택 Contact range-out edge direction, Zoom Out, Applied-copy source isolation을 검증.
// - v1.0.0: HUDSnapshot 단일 계약 Automation을 최초 추가.
// Migration:
// - UI-P0-08 테스트 Range 수치는 transient Automation 전용이며 Production SensorData/Scanner Asset에 저장하지 않습니다.
// - 기존 HUDSnapshot의 scanner-less 빈 Radar Range Profile은 NormalizedPosition Unavailable 회귀를 그대로 검증합니다.
// - transient Editor World, C++ ACFVehiclePawn/ACFMissileTestTarget, transient Health/DamageData만 사용하며 Content Asset을 생성·수정·저장하지 않습니다.
// - TargetSelect는 선택·TrackState owner로 그대로 사용하고 Relation/Category/InformationLevel/Identity/거리와 Radar Contact는 Sensor Snapshot 결과만 검증합니다.
// - DestroyedHold 검증은 실제 VehicleHealth 파괴 이벤트를 사용하고 TargetSelect의 Destroyed 즉시 clear와 Sensor Radar 보존이 동시에 성립하는지 확인합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFDamageData.h"
#include "CFMissileTestTarget.h"
#include "CFTargetSelectComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleSensorComp.h"
#include "CFVehicleSensorData.h"

#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDViewData.h"

namespace
{
	// [v1.0.0] 명시 ID·표시 이름·위치를 가진 C++ TargetSelectable Actor를 transient World에 생성합니다.
	ACFMissileTestTarget* SpawnSensorHUDTarget(
		UWorld* TestWorld,
		const FName ActorName,
		const FVector& ActorLocation,
		const FName TargetId,
		const FText& DisplayName)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		// [v1.0.0] 테스트 Target의 결정적인 Actor 이름을 지정할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		// [v1.0.0] Source DisplayInfo는 Identified지만 Sensor Public Knowledge는 Detected부터 시작하는 실제 C++ 테스트 Target입니다.
		ACFMissileTestTarget* TargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>(
			ACFMissileTestTarget::StaticClass(),
			ActorLocation,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (TargetActor)
		{
			TargetActor->TargetId = TargetId;
			TargetActor->TargetDisplayName = DisplayName;
			TargetActor->bAutoDestroy = false;
			TargetActor->RefreshTestTargetConfig();
		}
		return TargetActor;
	}

	// [v1.0.0] Target에 authoritative destruction truth를 제공할 transient VehicleHealth Component를 추가합니다.
	UCFVehicleHealthComp* AttachSensorHUDHealth(ACFMissileTestTarget* TargetActor)
	{
		if (!TargetActor)
		{
			return nullptr;
		}

		// [v1.0.0] 실제 OnVehicleDestroyed 경로를 발생시킬 Target 소유 Health Component입니다.
		UCFVehicleHealthComp* HealthComponent = NewObject<UCFVehicleHealthComp>(
			TargetActor,
			TEXT("VehicleHealthComp_SensorHUDAutomation"));
		if (!HealthComponent)
		{
			return nullptr;
		}

		TargetActor->AddInstanceComponent(HealthComponent);
		HealthComponent->RegisterComponent();
		HealthComponent->InitializeFromVehicleData(nullptr);
		return HealthComponent;
	}

	// [v1.0.0] Pawn 기본 Sensor를 asset-free Passive Detection/DestroyedHold 계약으로 재초기화합니다.
	bool ConfigureSensorHUDRuntime(UCFVehicleSensorComp* SensorComponent)
	{
		if (!SensorComponent)
		{
			return false;
		}

		SensorComponent->ResetSensorRuntime();
		SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = 5000.0f;
		SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
		SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = 64;
		SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = 5.0f;
		SensorComponent->FallbackSensorConfig.DestroyedHoldTimeSec = 2.0f;
		SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = 0.0f;
		SensorComponent->FallbackSensorConfig.AnalysisGainPerSec = 0.0f;
		SensorComponent->FallbackSensorConfig.AnalysisDecayPerSec = 0.0f;
		SensorComponent->FallbackSensorConfig.IdentifiedThreshold = 0.5f;
		SensorComponent->FallbackSensorConfig.DetailedScanThreshold = 1.0f;
		return SensorComponent->InitializeSensorRuntime();
	}

	// [v1.0.0] 실제 VehicleHealth 피해 경로로 테스트 Target을 한 번에 파괴합니다.
	bool DestroySensorHUDTarget(
		ACFMissileTestTarget* TargetActor,
		UCFVehicleHealthComp* HealthComponent,
		AActor* DamageInstigator,
		FCFDamageApplyResult& OutDamageResult)
	{
		if (!TargetActor || !HealthComponent)
		{
			return false;
		}

		// [v1.0.0] 저장 Asset 없이 파괴 이벤트를 발생시킬 transient DamageData입니다.
		UCFDamageData* DamageData = NewObject<UCFDamageData>(
			GetTransientPackage(),
			TEXT("DA_SensorHUDDestroyedAutomation"));
		if (!DamageData)
		{
			return false;
		}
		DamageData->DamageId = TEXT("SensorHUDDestroyedAutomationDamage");
		DamageData->BaseDamage = 1000.0f;
		DamageData->bCanDamageSelf = false;

		// [v1.0.0] VehicleHealth 최초 파괴 이벤트까지 전달할 실제 Blocking Hit Context입니다.
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
	FCFSensorHUDSnapshotTest,
	"CarFight.Sensor.SEN_P0_06.HUDSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] TargetSelect 선택 의미와 Sensor Snapshot Knowledge/Radar 의미가 분리된 채 Provider ViewData에 연결되는지 검증합니다.
bool FCFSensorHUDSnapshotTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Content Asset 없이 Pawn, Target, Sensor와 Provider를 함께 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("SEN-P0-06 HUDSnapshot 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on SensorHUDVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	// [v1.0.0] TargetSelect, Sensor와 HUD Provider의 실제 source가 될 Player 차량 Pawn Spawn 설정입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("SensorHUDVehiclePawn");

	// [v1.0.0] P0-06 통합 경계를 실제 기본 Component 조합으로 제공할 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);

	// [v1.0.0] 차량 앞 30m·오른쪽 10m에 배치할 Source Identified TargetSelectable입니다.
	ACFMissileTestTarget* TargetActor = SpawnSensorHUDTarget(
		TestWorld,
		TEXT("SensorHUDTargetActor"),
		FVector(3000.0f, 1000.0f, 0.0f),
		TEXT("SensorHUDTarget"),
		FText::FromString(TEXT("SENSOR HUD TARGET")));
	if (!TestNotNull(TEXT("SEN-P0-06 Vehicle Pawn 생성"), VehiclePawn)
		|| !TestNotNull(TEXT("SEN-P0-06 Target 생성"), TargetActor))
	{
		return false;
	}

	// [v1.0.0] 파괴 확정과 TargetSelect Destroyed 즉시 clear에 함께 사용할 authoritative Health Runtime입니다.
	UCFVehicleHealthComp* TargetHealthComponent = AttachSensorHUDHealth(TargetActor);

	// [v1.0.0] Detection/Knowledge와 actor-free Snapshot을 소유할 Pawn 기본 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = VehiclePawn->GetVehicleSensorComp();

	// [v1.0.0] 후보 검색·선택·TrackState를 계속 소유할 Pawn 기본 TargetSelect Component입니다.
	UCFTargetSelectComp* TargetSelectComponent = VehiclePawn->GetTargetSelectComp();
	if (!TestNotNull(TEXT("SEN-P0-06 Target Health 생성"), TargetHealthComponent)
		|| !TestNotNull(TEXT("SEN-P0-06 Sensor Component"), SensorComponent)
		|| !TestNotNull(TEXT("SEN-P0-06 TargetSelect Component"), TargetSelectComponent))
	{
		return false;
	}

	TestTrue(TEXT("SEN-P0-06 Sensor asset-free Runtime 초기화"), ConfigureSensorHUDRuntime(SensorComponent));
	TargetSelectComponent->bAutoRefreshCandidate = false;

	// [v1.0.0] Widget/Presenter 대신 Provider ViewData 계약 자체를 검증할 transient HUD Provider입니다.
	UCFHUDDataProvider* HUDDataProvider = NewObject<UCFHUDDataProvider>(
		GetTransientPackage(),
		TEXT("HUDDataProvider_SensorAutomation"));
	if (!TestNotNull(TEXT("SEN-P0-06 HUD Provider 생성"), HUDDataProvider))
	{
		return false;
	}
	HUDDataProvider->RebindCurrentPawn(VehiclePawn);

	// [v1.0.0] Sensor Runtime은 준비됐지만 Contact가 아직 0개인 초기 HUD ViewData입니다.
	const FCFInGameUIViewData EmptySensorViewData = HUDDataProvider->GetCurrentViewData();
	TestEqual(TEXT("준비된 Sensor + Contact 0은 Radar KnownZero"), EmptySensorViewData.Radar.Availability, ECFUIViewAvailability::KnownZero);
	TestTrue(TEXT("초기 Radar Contact 없음"), EmptySensorViewData.Radar.Contacts.IsEmpty());

	// [v1.0.0] 실제 Sensor 탐지 경로로 Target을 최초 Live/Detected Contact로 만든 결과입니다.
	const bool bInitialDetectionAccepted = SensorComponent->ProcessPassiveScanActor(
		TargetActor,
		SensorComponent->FallbackSensorConfig);
	TestTrue(TEXT("Sensor가 Target을 최초 Contact로 탐지"), bInitialDetectionAccepted);
	SensorComponent->PublishRuntimeSnapshot();

	// [v1.0.0] TargetSelect의 기존 public selection API가 사용할 현재 기본 선택 Context입니다.
	const FCFTargetSelectionContext SelectionContext = TargetSelectComponent->GetDefaultSelectionContext();
	TestTrue(TEXT("TargetSelect가 Target 선택을 계속 소유"), TargetSelectComponent->SetSelectedTarget(TargetActor, SelectionContext));
	HUDDataProvider->RefreshViewData();

	// [v1.0.0] Provider가 Player-facing data source로 사용해야 하는 최초 Actor-free Sensor Snapshot입니다.
	const FCFSensorSnapshot DetectedSnapshot = SensorComponent->GetSensorSnapshot();
	TestTrue(TEXT("Detected Snapshot 공개 계약 유효"), DetectedSnapshot.IsPublicContractValid());
	TestEqual(TEXT("Detected Snapshot Contact 1개"), DetectedSnapshot.Contacts.Num(), 1);
	if (DetectedSnapshot.Contacts.Num() != 1)
	{
		HUDDataProvider->ShutdownProvider();
		return false;
	}

	// [v1.0.0] Source는 Identified지만 Sensor가 Player에게 처음 공개한 Detected Contact입니다.
	const FCFSensorContact& DetectedContact = DetectedSnapshot.Contacts[0];
	TestEqual(TEXT("Source Identified가 Sensor Knowledge로 누출되지 않음"), DetectedContact.InformationLevel, ECFTargetInfoLevel::Detected);
	TestEqual(TEXT("Detected KnownTargetId 숨김"), DetectedContact.KnownTargetId, NAME_None);
	TestTrue(TEXT("Detected KnownDisplayName 숨김"), DetectedContact.KnownDisplayName.IsEmpty());

	// [v1.0.0] TargetSelect selection 상태와 Sensor Detected Knowledge가 분리 합성된 HUD ViewData입니다.
	const FCFInGameUIViewData DetectedHUDViewData = HUDDataProvider->GetCurrentViewData();
	TestEqual(TEXT("선택 Target HUD 채널 Known"), DetectedHUDViewData.Target.Availability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("TargetSelect 선택 기록 유지"), DetectedHUDViewData.Target.bHasSelectedTarget);
	TestEqual(TEXT("선택 Contact Sensor 연결 Known"), DetectedHUDViewData.Target.SensorContactAvailability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("HUD Target ContactId는 Snapshot ContactId"), DetectedHUDViewData.Target.ContactId, DetectedContact.ContactId);
	TestEqual(TEXT("HUD Target 정보 단계는 Sensor Detected"), DetectedHUDViewData.Target.InformationLevel, ECFTargetInfoLevel::Detected);
	TestEqual(TEXT("HUD TargetId는 Detected에서 숨김"), DetectedHUDViewData.Target.TargetId, NAME_None);
	TestEqual(TEXT("HUD Identity는 Detected에서 Unknown"), DetectedHUDViewData.Target.IdentityAvailability, ECFUIViewAvailability::Unknown);
	TestEqual(TEXT("HUD Relation은 Sensor Snapshot source"), DetectedHUDViewData.Target.Relation, DetectedContact.Relation);
	TestEqual(TEXT("HUD Category는 Sensor Snapshot source"), DetectedHUDViewData.Target.Category, DetectedContact.TargetCategory);
	TestEqual(TEXT("HUD Target ContactState Live"), DetectedHUDViewData.Target.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("HUD TrackState는 TargetSelect source"), DetectedHUDViewData.Target.TrackState, TargetSelectComponent->GetSelectedTargetTrackState());
	TestEqual(TEXT("HUD Target 거리 제공"), DetectedHUDViewData.Target.DistanceAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("HUD Target 거리 약 31.62m"), FMath::IsNearlyEqual(DetectedHUDViewData.Target.DistanceMeters, FMath::Sqrt(1000.0f), 0.05f));

	TestEqual(TEXT("Detected Contact가 있으면 Radar Known"), DetectedHUDViewData.Radar.Availability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("Radar Contact 1개"), DetectedHUDViewData.Radar.Contacts.Num(), 1);
	if (DetectedHUDViewData.Radar.Contacts.Num() == 1)
	{
		// [v1.0.0] Snapshot origin/forward와 last-known만으로 변환된 최초 Radar Contact입니다.
		const FCFRadarContactHUDData& RadarContact = DetectedHUDViewData.Radar.Contacts[0];
		TestEqual(TEXT("Radar ContactId 보존"), RadarContact.ContactId, DetectedContact.ContactId);
		TestEqual(TEXT("Radar InformationLevel Detected"), RadarContact.InformationLevel, ECFTargetInfoLevel::Detected);
		TestEqual(TEXT("Radar ContactState Live"), RadarContact.ContactState, ECFSensorContactState::Live);
		TestEqual(TEXT("Radar 상대 위치 제공"), RadarContact.RelativePositionAvailability, ECFUIViewAvailability::Known);
		TestTrue(TEXT("Radar 전방 상대 위치 약 30m"), FMath::IsNearlyEqual(RadarContact.RelativePositionMeters.X, 30.0f, 0.05f));
		TestTrue(TEXT("Radar 우측 상대 위치 약 10m"), FMath::IsNearlyEqual(RadarContact.RelativePositionMeters.Y, 10.0f, 0.05f));
		TestTrue(TEXT("Radar 거리 약 31.62m"), FMath::IsNearlyEqual(RadarContact.DistanceMeters, FMath::Sqrt(1000.0f), 0.05f));
		TestEqual(TEXT("Radar Range/Zoom 미확정이라 정규화 위치 Unavailable"), RadarContact.NormalizedPositionAvailability, ECFUIViewAvailability::Unavailable);
		TestTrue(TEXT("현재 선택 Contact만 bSelected true"), RadarContact.bSelected);
	}

	// [v1.0.0] Sensor Analysis가 Identified를 획득한 상태를 asset-free로 준비할 private Runtime Contact 인덱스입니다.
	const int32 RuntimeContactIndex = SensorComponent->FindRuntimeContactIndexByActor(TargetActor);
	TestTrue(TEXT("Sensor Runtime Contact 인덱스 존재"), SensorComponent->RuntimeContacts.IsValidIndex(RuntimeContactIndex));
	if (!SensorComponent->RuntimeContacts.IsValidIndex(RuntimeContactIndex))
	{
		HUDDataProvider->ShutdownProvider();
		return false;
	}

	// [v1.0.0] P0-04가 정의한 Knowledge 승격 결과와 같은 Player-facing Identified 상태를 검증하기 위한 Runtime Contact입니다.
	UCFVehicleSensorComp::FCFSensorContactRuntime& IdentifiedRuntimeContact = SensorComponent->RuntimeContacts[RuntimeContactIndex];
	IdentifiedRuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::Identified;
	IdentifiedRuntimeContact.PublicContact.KnownTargetId = TEXT("SensorHUDTarget");
	IdentifiedRuntimeContact.PublicContact.KnownDisplayName = FText::FromString(TEXT("SENSOR HUD TARGET"));
	IdentifiedRuntimeContact.PublicContact.AnalysisProgress01 = 0.60f;
	SensorComponent->PublishRuntimeSnapshot();
	HUDDataProvider->RefreshViewData();

	// [v1.0.0] Identified Knowledge가 Snapshot에서 Provider Target ViewData로 그대로 전달된 결과입니다.
	const FCFInGameUIViewData IdentifiedHUDViewData = HUDDataProvider->GetCurrentViewData();
	TestEqual(TEXT("HUD Identified 정보 단계"), IdentifiedHUDViewData.Target.InformationLevel, ECFTargetInfoLevel::Identified);
	TestEqual(TEXT("HUD KnownTargetId 공개"), IdentifiedHUDViewData.Target.TargetId, FName(TEXT("SensorHUDTarget")));
	TestEqual(TEXT("HUD Identity Known"), IdentifiedHUDViewData.Target.IdentityAvailability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("HUD KnownDisplayName 공개"), IdentifiedHUDViewData.Target.DisplayName.ToString(), FString(TEXT("SENSOR HUD TARGET")));
	TestTrue(TEXT("HUD AnalysisProgress Snapshot 값 보존"), FMath::IsNearlyEqual(IdentifiedHUDViewData.Target.AnalysisProgress01, 0.60f, KINDA_SMALL_NUMBER));

	// [v1.0.0] DestroyedHold 뒤에도 같은 Contact인지 확인할 승격 완료 ContactId입니다.
	const FName IdentifiedContactId = IdentifiedHUDViewData.Target.ContactId;

	// [v1.0.0] VehicleHealth의 실제 최초 파괴 경로 결과입니다.
	FCFDamageApplyResult DamageResult;
	const bool bDamageApplied = DestroySensorHUDTarget(
		TargetActor,
		TargetHealthComponent,
		VehiclePawn,
		DamageResult);
	TestTrue(TEXT("VehicleHealth 파괴 피해 적용"), bDamageApplied);
	TestTrue(TEXT("이번 피해에서 최초 파괴 확정"), DamageResult.bDestroyedThisHit);

	// Delegate 순서와 무관하게 최종 TargetSelect/Sensor 상태를 한 번의 Provider refresh로 함께 읽습니다.
	HUDDataProvider->RefreshViewData();

	TestTrue(TEXT("TargetSelect는 Destroyed 확인 즉시 선택 clear"), !TargetSelectComponent->HasSelectedTarget());

	// [v1.0.0] 선택은 사라졌지만 Sensor DestroyedHold Contact가 Radar에 독립 보존된 최종 ViewData입니다.
	const FCFInGameUIViewData DestroyedHUDViewData = HUDDataProvider->GetCurrentViewData();
	TestEqual(TEXT("선택 clear 뒤 Target HUD KnownZero"), DestroyedHUDViewData.Target.Availability, ECFUIViewAvailability::KnownZero);
	TestTrue(TEXT("선택 clear 뒤 bHasSelectedTarget false"), !DestroyedHUDViewData.Target.bHasSelectedTarget);
	TestEqual(TEXT("DestroyedHold Contact 때문에 Radar Known 유지"), DestroyedHUDViewData.Radar.Availability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("DestroyedHold Radar Contact 1개 유지"), DestroyedHUDViewData.Radar.Contacts.Num(), 1);
	if (DestroyedHUDViewData.Radar.Contacts.Num() == 1)
	{
		// [v1.0.0] TargetSelect 수명과 독립적으로 Sensor가 보존한 DestroyedHold Radar Contact입니다.
		const FCFRadarContactHUDData& DestroyedRadarContact = DestroyedHUDViewData.Radar.Contacts[0];
		TestEqual(TEXT("DestroyedHold 같은 ContactId 유지"), DestroyedRadarContact.ContactId, IdentifiedContactId);
		TestEqual(TEXT("DestroyedHold 상태 전달"), DestroyedRadarContact.ContactState, ECFSensorContactState::DestroyedHold);
		TestTrue(TEXT("DestroyedHold bDestroyedConfirmed 전달"), DestroyedRadarContact.bDestroyedConfirmed);
		TestEqual(TEXT("DestroyedHold Knowledge Identified 유지"), DestroyedRadarContact.InformationLevel, ECFTargetInfoLevel::Identified);
		TestTrue(TEXT("DestroyedHold AnalysisProgress 유지"), FMath::IsNearlyEqual(DestroyedRadarContact.AnalysisProgress01, 0.60f, KINDA_SMALL_NUMBER));
		TestTrue(TEXT("선택은 clear됐으므로 DestroyedHold bSelected false"), !DestroyedRadarContact.bSelected);
		TestTrue(TEXT("DestroyedHold 마지막 신뢰 전방 위치 약 30m"), FMath::IsNearlyEqual(DestroyedRadarContact.RelativePositionMeters.X, 30.0f, 0.05f));
		TestTrue(TEXT("DestroyedHold 마지막 신뢰 우측 위치 약 10m"), FMath::IsNearlyEqual(DestroyedRadarContact.RelativePositionMeters.Y, 10.0f, 0.05f));
		}

	HUDDataProvider->ShutdownProvider();
	TargetActor->Destroy();
	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRadarRangeFoundationTest,
	"CarFight.UI.UI_P0_08.RadarRangeFoundationContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] Scanner-owned Range Profile이 Sensor 탐지 성능과 분리된 Provider-local Radar Zoom/정규화 ViewData로 변환되는지 검증합니다.
bool FCFRadarRangeFoundationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] Content Asset 없이 차량, Sensor, TargetSelect와 HUD Provider를 함께 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-08 Radar Range 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.1.0] Radar Range Profile과 실제 Sensor Runtime을 소유할 transient 차량 Pawn Spawn 설정입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("RadarRangeVehiclePawn");

	// [v1.1.0] Heading-Up Radar 원점과 전방 방향을 제공할 transient 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);

	// [v1.1.0] 전방 30m·우측 10m에 배치해 50m 범위 안/25m 범위 밖을 모두 검증할 Target입니다.
	ACFMissileTestTarget* TargetActor = SpawnSensorHUDTarget(
		TestWorld,
		TEXT("RadarRangeTargetActor"),
		FVector(3000.0f, 1000.0f, 0.0f),
		TEXT("RadarRangeTarget"),
		FText::FromString(TEXT("RADAR RANGE TARGET")));
	if (!TestNotNull(TEXT("UI-P0-08 Radar 차량 Pawn 생성"), VehiclePawn)
		|| !TestNotNull(TEXT("UI-P0-08 Radar Target 생성"), TargetActor))
	{
		return false;
	}

	// [v1.1.0] 실제 Sensor Contact Runtime과 Applied Radar Range Profile을 소유할 차량 기본 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = VehiclePawn->GetVehicleSensorComp();
	// [v1.1.0] 현재 선택 Contact를 Radar bSelected 의미에 연결할 기존 TargetSelect Component입니다.
	UCFTargetSelectComp* TargetSelectComponent = VehiclePawn->GetTargetSelectComp();
	if (!TestNotNull(TEXT("UI-P0-08 Radar Sensor Component"), SensorComponent)
		|| !TestNotNull(TEXT("UI-P0-08 Radar TargetSelect Component"), TargetSelectComponent))
	{
		return false;
	}

	// [v1.1.0] Production Asset을 건드리지 않고 Range Profile 전체 계약을 검증할 transient Scanner Data입니다.
	UCFVehicleSensorData* RadarSensorData = NewObject<UCFVehicleSensorData>(
		GetTransientPackage(),
		TEXT("DA_RadarRangeFoundationTransient"));
	if (!TestNotNull(TEXT("UI-P0-08 transient Radar SensorData"), RadarSensorData))
	{
		return false;
	}

	RadarSensorData->SensorConfig.PassiveDetectionRangeCm = 5000.0f;
	RadarSensorData->SensorConfig.ActiveScanRangeCm = 10000.0f;
	RadarSensorData->SensorConfig.VisualDetectionRangeCm = 0.0f;
	RadarSensorData->SensorConfig.UpdateIntervalSec = 0.1f;
	RadarSensorData->SensorConfig.MaxActorScansPerUpdate = 64;
	RadarSensorData->SensorConfig.ContactMemoryTimeSec = 5.0f;
	RadarSensorData->SensorConfig.DestroyedHoldTimeSec = 2.0f;
	RadarSensorData->SensorConfig.ActiveScanDurationSec = 1.0f;
	RadarSensorData->SensorConfig.AnalysisGainPerSec = 0.0f;
	RadarSensorData->SensorConfig.AnalysisDecayPerSec = 0.0f;
	RadarSensorData->SensorConfig.IdentifiedThreshold = 0.5f;
	RadarSensorData->SensorConfig.DetailedScanThreshold = 1.0f;
	RadarSensorData->RadarDisplayRangePresetsCm.Add(2500.0f);
	RadarSensorData->RadarDisplayRangePresetsCm.Add(5000.0f);
	RadarSensorData->RadarDisplayRangePresetsCm.Add(10000.0f);
	RadarSensorData->DefaultRadarDisplayRangePresetIndex = 1;
	TestTrue(TEXT("UI-P0-08 transient Radar Range Profile 전체 계약 유효"), RadarSensorData->IsSensorDataContractValid());

	SensorComponent->ResetSensorRuntime();
	TestTrue(TEXT("UI-P0-08 Radar SensorData pre-runtime 적용"), SensorComponent->ApplySensorData(RadarSensorData));
	TestTrue(TEXT("UI-P0-08 Radar Sensor Runtime 초기화"), SensorComponent->InitializeSensorRuntime());
	TestEqual(TEXT("UI-P0-08 Applied Radar Range Preset 3개"), SensorComponent->GetResolvedRadarDisplayRangePresetsCm().Num(), 3);
	TestEqual(TEXT("UI-P0-08 Applied 기본 Radar Range Index 1"), SensorComponent->GetResolvedDefaultRadarDisplayRangePresetIndex(), 1);

	// [v1.1.0] Target을 실제 Sensor Contact로 만들 때 사용할 현재 Applied Sensor Config입니다.
	const FCFSensorConfig AppliedSensorConfigBeforeZoom = SensorComponent->GetResolvedSensorConfig();
	TestTrue(TEXT("UI-P0-08 Target 실제 Passive Contact 탐지"), SensorComponent->ProcessPassiveScanActor(TargetActor, AppliedSensorConfigBeforeZoom));
	SensorComponent->PublishRuntimeSnapshot();

	TargetSelectComponent->bAutoRefreshCandidate = false;
	// [v1.1.0] 기존 TargetSelect public selection API가 사용할 현재 기본 선택 Context입니다.
	const FCFTargetSelectionContext SelectionContext = TargetSelectComponent->GetDefaultSelectionContext();
	TestTrue(TEXT("UI-P0-08 TargetSelect가 Radar 선택 Contact 소유"), TargetSelectComponent->SetSelectedTarget(TargetActor, SelectionContext));

	// [v1.1.0] Scanner Range Profile 선택과 Radar ViewData 정규화를 소유할 transient HUD Provider입니다.
	UCFHUDDataProvider* HUDDataProvider = NewObject<UCFHUDDataProvider>(
		GetTransientPackage(),
		TEXT("HUDDataProvider_RadarRangeAutomation"));
	if (!TestNotNull(TEXT("UI-P0-08 Radar HUD Provider"), HUDDataProvider))
	{
		return false;
	}
	HUDDataProvider->RebindCurrentPawn(VehiclePawn);

	// [v1.1.0] Scanner가 명시한 기본 Index 1=50m를 처음 적용한 Radar ViewData입니다.
	const FCFInGameUIViewData DefaultRangeViewData = HUDDataProvider->GetCurrentViewData();
	TestEqual(TEXT("UI-P0-08 기본 Radar Display Range Known"), DefaultRangeViewData.Radar.DisplayRangeAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("UI-P0-08 기본 Radar Display Range 50m"), FMath::IsNearlyEqual(DefaultRangeViewData.Radar.DisplayRangeMeters, 50.0f));
	TestEqual(TEXT("UI-P0-08 Radar 최대 탐지 Range Known"), DefaultRangeViewData.Radar.MaximumDetectionRangeAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("UI-P0-08 Radar 최대 탐지 Range 100m"), FMath::IsNearlyEqual(DefaultRangeViewData.Radar.MaximumDetectionRangeMeters, 100.0f));
	TestEqual(TEXT("UI-P0-08 기본 Radar Preset Index 1"), DefaultRangeViewData.Radar.RangePresetIndex, 1);
	TestEqual(TEXT("UI-P0-08 Radar Preset Count 3"), DefaultRangeViewData.Radar.RangePresetCount, 3);
	TestTrue(TEXT("UI-P0-08 기본 Range에서 Zoom In 가능"), DefaultRangeViewData.Radar.bCanZoomIn);
	TestTrue(TEXT("UI-P0-08 기본 Range에서 Zoom Out 가능"), DefaultRangeViewData.Radar.bCanZoomOut);
	TestEqual(TEXT("UI-P0-08 기본 Radar Contact 1개"), DefaultRangeViewData.Radar.Contacts.Num(), 1);
	if (DefaultRangeViewData.Radar.Contacts.Num() != 1)
	{
		HUDDataProvider->ShutdownProvider();
		return false;
	}

	// [v1.1.0] 50m 기본 Range 안에서 실제 30m 전방·10m 우측 Contact의 정규화 결과입니다.
	const FCFRadarContactHUDData& DefaultRangeContact = DefaultRangeViewData.Radar.Contacts[0];
	TestEqual(TEXT("UI-P0-08 기본 Contact Normalized Known"), DefaultRangeContact.NormalizedPositionAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("UI-P0-08 기본 Contact Range 안"), DefaultRangeContact.bInsideDisplayRange);
	TestTrue(TEXT("UI-P0-08 기본 Contact Normalized Forward 0.6"), FMath::IsNearlyEqual(DefaultRangeContact.NormalizedPosition.X, 0.6f, 0.001f));
	TestTrue(TEXT("UI-P0-08 기본 Contact Normalized Right 0.2"), FMath::IsNearlyEqual(DefaultRangeContact.NormalizedPosition.Y, 0.2f, 0.001f));
	TestTrue(TEXT("UI-P0-08 기본 Contact Selected"), DefaultRangeContact.bSelected);
	TestTrue(TEXT("UI-P0-08 기본 Contact Edge Marker 없음"), !DefaultRangeContact.bShowSelectedEdgeMarker);

	TestTrue(TEXT("UI-P0-08 Radar Zoom In 50m→25m"), HUDDataProvider->RequestRadarZoomIn());
	// [v1.1.0] Zoom In 뒤 25m 표시 범위에서 같은 선택 Contact가 범위 밖으로 나간 ViewData입니다.
	const FCFInGameUIViewData ZoomedInViewData = HUDDataProvider->GetCurrentViewData();
	TestTrue(TEXT("UI-P0-08 Zoom In Display Range 25m"), FMath::IsNearlyEqual(ZoomedInViewData.Radar.DisplayRangeMeters, 25.0f));
	TestEqual(TEXT("UI-P0-08 Zoom In Preset Index 0"), ZoomedInViewData.Radar.RangePresetIndex, 0);
	TestTrue(TEXT("UI-P0-08 최소 Range에서 추가 Zoom In 불가"), !ZoomedInViewData.Radar.bCanZoomIn);
	TestEqual(TEXT("UI-P0-08 Zoom In Contact 1개"), ZoomedInViewData.Radar.Contacts.Num(), 1);
	if (ZoomedInViewData.Radar.Contacts.Num() != 1)
	{
		HUDDataProvider->ShutdownProvider();
		return false;
	}

	// [v1.1.0] 범위 밖 선택 Contact가 Radar unit edge에 유지할 방향 계약입니다.
	const FCFRadarContactHUDData& ZoomedInContact = ZoomedInViewData.Radar.Contacts[0];
	TestTrue(TEXT("UI-P0-08 25m에서 Contact Range 밖"), !ZoomedInContact.bInsideDisplayRange);
	TestTrue(TEXT("UI-P0-08 범위 밖 선택 Contact Edge Marker 필요"), ZoomedInContact.bShowSelectedEdgeMarker);
	TestTrue(TEXT("UI-P0-08 Edge Forward 방향 약 0.9487"), FMath::IsNearlyEqual(ZoomedInContact.SelectedEdgeDirection.X, 0.948683f, 0.001f));
	TestTrue(TEXT("UI-P0-08 Edge Right 방향 약 0.3162"), FMath::IsNearlyEqual(ZoomedInContact.SelectedEdgeDirection.Y, 0.316228f, 0.001f));
	TestTrue(TEXT("UI-P0-08 range-out Normalized unit edge"), FMath::IsNearlyEqual(ZoomedInContact.NormalizedPosition.Size(), 1.0f, 0.001f));

	// [v1.1.0] Radar Zoom은 UI 표시만 바꾸고 실제 Sensor Active Scan 성능을 변경하면 안 됩니다.
	const FCFSensorConfig AppliedSensorConfigAfterZoom = SensorComponent->GetResolvedSensorConfig();
	TestTrue(TEXT("UI-P0-08 Zoom 뒤 ActiveScanRange 100m 유지"), FMath::IsNearlyEqual(AppliedSensorConfigAfterZoom.ActiveScanRangeCm, 10000.0f));

	TestTrue(TEXT("UI-P0-08 Radar Zoom Out 25m→50m"), HUDDataProvider->RequestRadarZoomOut());
	// [v1.1.0] 50m로 복귀한 현재 Radar ViewData입니다.
	const FCFInGameUIViewData ZoomedOutViewData = HUDDataProvider->GetCurrentViewData();
	TestTrue(TEXT("UI-P0-08 Zoom Out Display Range 50m 복귀"), FMath::IsNearlyEqual(ZoomedOutViewData.Radar.DisplayRangeMeters, 50.0f));
	TestTrue(TEXT("UI-P0-08 Zoom Out 뒤 Contact 다시 Range 안"), ZoomedOutViewData.Radar.Contacts.Num() == 1 && ZoomedOutViewData.Radar.Contacts[0].bInsideDisplayRange);

	// [v1.1.0] Runtime Ready 뒤 Source UObject를 직접 바꿔도 Applied Radar Profile 사본이 즉시 변하지 않는지 확인합니다.
	RadarSensorData->RadarDisplayRangePresetsCm[1] = 4000.0f;
	RadarSensorData->SensorConfig.ActiveScanRangeCm = 20000.0f;
	HUDDataProvider->RefreshViewData();
	// [v1.1.0] Source 직접 편집 뒤에도 기존 Applied 50m/100m 계약을 유지한 ViewData입니다.
	const FCFInGameUIViewData AppliedCopyViewData = HUDDataProvider->GetCurrentViewData();
	TestTrue(TEXT("UI-P0-08 Source 직접 편집 뒤 Applied Display Range 50m 유지"), FMath::IsNearlyEqual(AppliedCopyViewData.Radar.DisplayRangeMeters, 50.0f));
	TestTrue(TEXT("UI-P0-08 Source 직접 편집 뒤 Applied Maximum 100m 유지"), FMath::IsNearlyEqual(AppliedCopyViewData.Radar.MaximumDetectionRangeMeters, 100.0f));

	HUDDataProvider->ShutdownProvider();
	TargetActor->Destroy();
	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
