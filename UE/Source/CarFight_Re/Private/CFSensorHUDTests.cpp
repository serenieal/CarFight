// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-036 SEN-P0-06 Public Sensor Snapshot → HUD Provider asset-free Automation
// Scope: TargetSelect 선택 소유권, Sensor Knowledge/Radar Snapshot 소비, Detected identity 비누출, 상대 위치·거리와 DestroyedHold 독립 수명을 검증합니다.
// Changelog:
// - v1.0.0: HUDSnapshot 단일 계약 Automation을 최초 추가.
// Migration:
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

#endif // WITH_DEV_AUTOMATION_TESTS
