// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-036 SEN-P0-02 bounded Passive Detection Automation
// Scope: 자기 차량 제외, Passive range, 직접 가시 Visual fallback, bounded cursor와 동일 Actor 중복 Contact 금지를 asset-free로 검증합니다.
// Changelog:
// - v1.0.0: PassiveRange, VisualFallback, BoundedScan 3개 Automation을 최초 추가.
// Migration:
// - 새 프로젝트 Content Asset을 생성·저장하지 않고 transient editor map과 C++ ACFMissileTestTarget만 사용합니다.
// - Sensor Production Source는 ECC_Visibility만 사용하며 TargetSelect Trace Channel을 사용하지 않습니다.

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
	// [v1.0.0] 이름·위치·TargetId가 명시된 C++ TargetSelectable 테스트 Actor를 transient map에 생성합니다.
	ACFMissileTestTarget* SpawnSensorTestTarget(
		UWorld* TestWorld,
		const FName ActorName,
		const FVector& ActorLocation,
		const FName TargetId)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		// [v1.0.0] 결정적인 Actor 이름으로 Spawn하기 위한 파라미터입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		// [v1.0.0] Sensor TargetSelectable 계약 검증에 사용할 실제 C++ 테스트 Actor입니다.
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

	// [v1.0.0] Actor Owner에 transient Sensor Component를 붙이고 명시 Config로 초기화합니다.
	UCFVehicleSensorComp* AttachSensorForAutomation(
		AActor* SensorOwner,
		const float PassiveRangeCm,
		const float VisualRangeCm,
		const int32 ActorScanBudget)
	{
		if (!SensorOwner)
		{
			return nullptr;
		}

		// [v1.0.0] 저장 Asset 없이 실제 World/Owner를 사용하는 테스트 Sensor Component입니다.
		UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(SensorOwner, TEXT("VehicleSensorComp_Automation"));
		if (!SensorComponent)
		{
			return nullptr;
		}

		SensorOwner->AddInstanceComponent(SensorComponent);
		SensorComponent->RegisterComponent();
		SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = PassiveRangeCm;
		SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 0.0f;
		SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = VisualRangeCm;
		SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
		SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = ActorScanBudget;
		SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = 0.0f;
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
	FCFSensorPassiveRangeTest,
	"CarFight.Sensor.SEN_P0_02.PassiveRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 자기 Actor 제외, Passive range 안/밖과 Source InformationLevel 비누출을 검증합니다.
bool FCFSensorPassiveRangeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 새 프로젝트 Content Asset 없이 Actor와 Component를 생성할 transient editor map입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Passive Range 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 자기 차량 제외 계약을 함께 검증할 Sensor Owner TargetSelectable입니다.
	ACFMissileTestTarget* SensorOwner = SpawnSensorTestTarget(TestWorld, TEXT("SensorRangeOwner"), FVector::ZeroVector, TEXT("SensorRangeOwner"));

	// [v1.0.0] 5,000cm Passive 범위 안에 있는 TargetSelectable입니다.
	ACFMissileTestTarget* NearTarget = SpawnSensorTestTarget(TestWorld, TEXT("SensorRangeNear"), FVector(3000.0f, 0.0f, 0.0f), TEXT("RangeNear"));

	// [v1.0.0] 5,000cm Passive 범위 밖에 있는 TargetSelectable입니다.
	ACFMissileTestTarget* FarTarget = SpawnSensorTestTarget(TestWorld, TEXT("SensorRangeFar"), FVector(8000.0f, 0.0f, 0.0f), TEXT("RangeFar"));
	if (!TestNotNull(TEXT("Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("Passive 안쪽 Target 생성"), NearTarget)
		|| !TestNotNull(TEXT("Passive 바깥 Target 생성"), FarTarget))
	{
		return false;
	}

	// [v1.0.0] 한 update가 작은 테스트 월드 전체를 충분히 검사하도록 큰 bounded budget을 준 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachSensorForAutomation(SensorOwner, 5000.0f, 0.0f, 256);
	if (!TestNotNull(TEXT("Passive Range Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.0.0] World actor cursor가 최초 전체 순회를 마쳤는지 확인할 시작 cycle 값입니다.
	const int32 StartingCycleSerial = SensorComponent->PassiveScanCycleSerial;

	// [v1.0.0] Level actor 수가 테스트 환경에 따라 달라도 무한 반복하지 않도록 제한할 호출 횟수입니다.
	int32 UpdateSafetyCount = 0;
	while (SensorComponent->PassiveScanCycleSerial == StartingCycleSerial && UpdateSafetyCount < 32)
	{
		SensorComponent->RunPassiveDetectionUpdate();
		++UpdateSafetyCount;
	}
		TestTrue(TEXT("Passive Range bounded cursor가 전체 순회를 완료"), SensorComponent->PassiveScanCycleSerial > StartingCycleSerial);
	TestEqual(TEXT("큰 budget도 한 update에서 World를 중복 순회하지 않음"), SensorComponent->PassiveScanCycleSerial, StartingCycleSerial + 1);

	// [v1.0.0] Passive 범위 안 Actor의 private Runtime Contact 인덱스입니다.
	const int32 NearContactIndex = SensorComponent->FindRuntimeContactIndexByActor(NearTarget);

	// [v1.0.0] Passive 범위 밖 Actor의 private Runtime Contact 인덱스입니다.
	const int32 FarContactIndex = SensorComponent->FindRuntimeContactIndexByActor(FarTarget);

	// [v1.0.0] 자기 Actor가 잘못 Contact로 들어갔는지 확인할 private Runtime Contact 인덱스입니다.
	const int32 SelfContactIndex = SensorComponent->FindRuntimeContactIndexByActor(SensorOwner);
	TestTrue(TEXT("Passive 범위 안 Target 탐지"), NearContactIndex != INDEX_NONE);
	TestEqual(TEXT("Passive 범위 밖 Target 미탐지"), FarContactIndex, INDEX_NONE);
	TestEqual(TEXT("Sensor 자기 Actor 제외"), SelfContactIndex, INDEX_NONE);

	// [v1.0.0] 외부 소비자가 읽을 Actor-free Passive Snapshot입니다.
	const FCFSensorSnapshot SensorSnapshot = SensorComponent->GetSensorSnapshot();
	TestEqual(TEXT("Passive Snapshot Contact 1개"), SensorSnapshot.Contacts.Num(), 1);
	TestTrue(TEXT("Passive Snapshot 공개 계약 유효"), SensorSnapshot.IsPublicContractValid());
	if (SensorSnapshot.Contacts.Num() == 1)
	{
		// [v1.0.0] Detected 단계에서 Source의 Identified truth가 새지 않는지 검증할 공개 Contact입니다.
		const FCFSensorContact& PublicContact = SensorSnapshot.Contacts[0];
		TestEqual(TEXT("새 Passive Contact는 Detected로 시작"), PublicContact.InformationLevel, ECFTargetInfoLevel::Detected);
		TestEqual(TEXT("Detected Contact KnownTargetId 비공개"), PublicContact.KnownTargetId, NAME_None);
		TestTrue(TEXT("Detected Contact KnownDisplayName 비공개"), PublicContact.KnownDisplayName.IsEmpty());
		TestEqual(TEXT("Source Relation Metadata는 공용 의미 타입으로 전달"), PublicContact.Relation, ECFTargetRelation::Hostile);
		TestEqual(TEXT("Source Category Metadata는 공용 의미 타입으로 전달"), PublicContact.TargetCategory, ECFTargetCategory::Vehicle);
	}

	if (NearContactIndex != INDEX_NONE)
	{
		// [v1.0.0] private Source Metadata에는 기존 TargetSelectable의 원본 Identified 값이 보존되는지 확인합니다.
		const UCFVehicleSensorComp::FCFSensorContactRuntime& RuntimeContact = SensorComponent->RuntimeContacts[NearContactIndex];
		TestEqual(TEXT("private Source TargetId 보존"), RuntimeContact.SourceDisplayInfo.TargetId, FName(TEXT("RangeNear")));
		TestEqual(TEXT("private Source InformationLevel은 원본 Identified"), RuntimeContact.SourceDisplayInfo.InformationLevel, ECFTargetInfoLevel::Identified);
		TestEqual(TEXT("public Knowledge는 Source InformationLevel을 복사하지 않음"), RuntimeContact.PublicContact.InformationLevel, ECFTargetInfoLevel::Detected);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorVisualFallbackTest,
	"CarFight.Sensor.SEN_P0_02.VisualFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Passive 범위 밖에서 직접 보이는 대상은 탐지하고 Visibility blocker 뒤 대상은 제외하는 최소 가시 계약을 검증합니다.
bool FCFSensorVisualFallbackTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Visibility Trace를 실제 World collision으로 검증할 transient editor map입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Visual Fallback 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Visual LOS 시작점이 될 Sensor Owner입니다.
	ACFMissileTestTarget* SensorOwner = SpawnSensorTestTarget(TestWorld, TEXT("SensorVisualOwner"), FVector::ZeroVector, TEXT("SensorVisualOwner"));

	// [v1.0.0] Passive 1,000cm 밖이지만 Visual 5,000cm 안에서 가리지 않은 Target입니다.
	ACFMissileTestTarget* VisibleTarget = SpawnSensorTestTarget(TestWorld, TEXT("SensorVisualClear"), FVector(3000.0f, 1000.0f, 0.0f), TEXT("VisualClear"));

	// [v1.0.0] Passive 밖·Visual 안이지만 중간 Visibility blocker 뒤에 있는 Target입니다.
	ACFMissileTestTarget* OccludedTarget = SpawnSensorTestTarget(TestWorld, TEXT("SensorVisualBlocked"), FVector(3000.0f, -1000.0f, 0.0f), TEXT("VisualBlocked"));
	if (!TestNotNull(TEXT("Visual Sensor Owner 생성"), SensorOwner)
		|| !TestNotNull(TEXT("가시 Target 생성"), VisibleTarget)
		|| !TestNotNull(TEXT("가림 Target 생성"), OccludedTarget))
	{
		return false;
	}

	// [v1.0.0] OccludedTarget 직선 경로 중간에 놓을 일반 World Actor입니다.
	AActor* VisibilityBlockerActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Visibility blocker Actor 생성"), VisibilityBlockerActor))
	{
		return false;
	}

	// [v1.0.0] TargetSelect 채널 설정 없이 ECC_Visibility만 Block하는 테스트 박스입니다.
	UBoxComponent* VisibilityBlockerComponent = NewObject<UBoxComponent>(VisibilityBlockerActor, TEXT("SensorVisibilityBlocker"));
	if (!TestNotNull(TEXT("Visibility blocker Component 생성"), VisibilityBlockerComponent))
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
	VisibilityBlockerActor->SetActorLocation(FVector(1500.0f, -500.0f, 0.0f));

	// [v1.0.0] Passive보다 긴 Visual 직접 가시 fallback만 확인할 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachSensorForAutomation(SensorOwner, 1000.0f, 5000.0f, 256);
	if (!TestNotNull(TEXT("Visual Fallback Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.0.0] 최초 전체 actor cursor 순회 전 cycle 값입니다.
	const int32 StartingCycleSerial = SensorComponent->PassiveScanCycleSerial;

	// [v1.0.0] 테스트 World actor 수 차이를 허용하는 bounded update 안전 횟수입니다.
	int32 UpdateSafetyCount = 0;

	// [v1.0.0] 전체 순회 동안 누적된 Visual Visibility Trace 수입니다.
	int32 TotalVisibilityTraceCount = 0;
	while (SensorComponent->PassiveScanCycleSerial == StartingCycleSerial && UpdateSafetyCount < 32)
	{
		SensorComponent->RunPassiveDetectionUpdate();
		TotalVisibilityTraceCount += SensorComponent->LastPassiveVisibilityTraceCount;
		++UpdateSafetyCount;
	}
		TestTrue(TEXT("Visual Fallback bounded cursor 전체 순회 완료"), SensorComponent->PassiveScanCycleSerial > StartingCycleSerial);
	TestEqual(TEXT("Visual 큰 budget도 한 update에서 World를 한 번만 순회"), SensorComponent->PassiveScanCycleSerial, StartingCycleSerial + 1);
	TestTrue(TEXT("Visual fallback에서 Visibility Trace 실행"), TotalVisibilityTraceCount >= 2);
	TestTrue(TEXT("Passive 밖 직접 가시 Target 탐지"), SensorComponent->FindRuntimeContactIndexByActor(VisibleTarget) != INDEX_NONE);
	TestEqual(TEXT("Passive 밖 Visibility blocker 뒤 Target 미탐지"), SensorComponent->FindRuntimeContactIndexByActor(OccludedTarget), INDEX_NONE);
	TestEqual(TEXT("Visual Fallback Snapshot Contact 1개"), SensorComponent->GetSensorSnapshot().Contacts.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFSensorBoundedScanTest,
	"CarFight.Sensor.SEN_P0_02.BoundedScan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] update당 Actor 슬롯 상한, cursor 공정성, 전체 순회 후 5개 탐지와 같은 Actor ContactId 비중복을 검증합니다.
bool FCFSensorBoundedScanTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 작은 budget으로 여러 update에 걸친 cursor 진행을 검증할 transient editor map입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Bounded Scan 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] bounded cursor에서 self 제외할 Sensor Owner입니다.
	ACFMissileTestTarget* SensorOwner = SpawnSensorTestTarget(TestWorld, TEXT("SensorBoundedOwner"), FVector::ZeroVector, TEXT("SensorBoundedOwner"));
	if (!TestNotNull(TEXT("Bounded Sensor Owner 생성"), SensorOwner))
	{
		return false;
	}

	// [v1.0.0] 전체 순회 뒤 모두 탐지돼야 할 근거리 Target Actor 목록입니다.
	TArray<ACFMissileTestTarget*> ExpectedTargets;
	for (int32 TargetIndex = 0; TargetIndex < 5; ++TargetIndex)
	{
		// [v1.0.0] 각 Target의 결정적인 Actor 이름입니다.
		const FName ActorName(*FString::Printf(TEXT("SensorBoundedTarget_%d"), TargetIndex));

		// [v1.0.0] 각 Target의 Source Metadata TargetId입니다.
		const FName TargetId(*FString::Printf(TEXT("BoundedTarget_%d"), TargetIndex));

		// [v1.0.0] 10,000cm Passive 범위 안에서 서로 겹치지 않을 테스트 위치입니다.
		const FVector TargetLocation(2000.0f + TargetIndex * 500.0f, TargetIndex * 300.0f, 0.0f);

		// [v1.0.0] 현재 인덱스의 실제 TargetSelectable 테스트 Actor입니다.
		ACFMissileTestTarget* TargetActor = SpawnSensorTestTarget(TestWorld, ActorName, TargetLocation, TargetId);
		if (!TestNotNull(TEXT("Bounded Target 생성"), TargetActor))
		{
			return false;
		}
		ExpectedTargets.Add(TargetActor);
	}

	// [v1.0.0] 한 update에서 Actor 슬롯 2개만 허용하는 매우 작은 bounded budget의 Sensor입니다.
	UCFVehicleSensorComp* SensorComponent = AttachSensorForAutomation(SensorOwner, 10000.0f, 0.0f, 2);
	if (!TestNotNull(TEXT("Bounded Scan Sensor 초기화"), SensorComponent))
	{
		return false;
	}

	// [v1.0.0] 첫 전체 cursor cycle 완료 여부를 판정할 시작값입니다.
	const int32 FirstCycleStartSerial = SensorComponent->PassiveScanCycleSerial;

	// [v1.0.0] Actor 배열이 예상보다 커도 테스트를 무한 반복하지 않게 제한하는 호출 횟수입니다.
	int32 FirstCycleSafetyCount = 0;
	while (SensorComponent->PassiveScanCycleSerial == FirstCycleStartSerial && FirstCycleSafetyCount < 128)
	{
		SensorComponent->RunPassiveDetectionUpdate();
		TestTrue(TEXT("한 update Actor 검사 수가 budget 2 이하"), SensorComponent->LastPassiveScanActorCount <= 2);
		++FirstCycleSafetyCount;
	}
	TestTrue(TEXT("작은 budget도 cursor가 전체 World를 끝까지 순회"), SensorComponent->PassiveScanCycleSerial > FirstCycleStartSerial);
	TestEqual(TEXT("첫 전체 순회 후 Target 5개 Contact"), SensorComponent->RuntimeContacts.Num(), 5);
	TestEqual(TEXT("첫 전체 순회 Snapshot Contact 5개"), SensorComponent->GetSensorSnapshot().Contacts.Num(), 5);
	TestTrue(TEXT("첫 Snapshot ContactId 중복 없음"), SensorComponent->GetSensorSnapshot().HasUniqueContactIds());

	// [v1.0.0] 둘째 순회 뒤 같은 Actor가 새 ContactId를 받지 않았는지 비교할 첫 ContactId 집합입니다.
	TSet<FName> FirstContactIds;
	for (const UCFVehicleSensorComp::FCFSensorContactRuntime& RuntimeContact : SensorComponent->RuntimeContacts)
	{
		FirstContactIds.Add(RuntimeContact.PublicContact.ContactId);
	}

	// [v1.0.0] 둘째 전체 cursor cycle의 시작값입니다.
	const int32 SecondCycleStartSerial = SensorComponent->PassiveScanCycleSerial;

	// [v1.0.0] 둘째 순회도 같은 bounded budget을 지키는지 검증할 안전 횟수입니다.
	int32 SecondCycleSafetyCount = 0;
	while (SensorComponent->PassiveScanCycleSerial == SecondCycleStartSerial && SecondCycleSafetyCount < 128)
	{
		SensorComponent->RunPassiveDetectionUpdate();
		TestTrue(TEXT("둘째 순회도 update당 Actor 검사 수 budget 준수"), SensorComponent->LastPassiveScanActorCount <= 2);
		++SecondCycleSafetyCount;
	}
	TestTrue(TEXT("둘째 bounded 전체 순회 완료"), SensorComponent->PassiveScanCycleSerial > SecondCycleStartSerial);
	TestEqual(TEXT("같은 Actor 재관측 뒤 Contact 수 증가 없음"), SensorComponent->RuntimeContacts.Num(), 5);

	// [v1.0.0] 둘째 순회 후 실제 Runtime ContactId 집합입니다.
	TSet<FName> SecondContactIds;
	for (const UCFVehicleSensorComp::FCFSensorContactRuntime& RuntimeContact : SensorComponent->RuntimeContacts)
	{
		SecondContactIds.Add(RuntimeContact.PublicContact.ContactId);
	}
		TestEqual(TEXT("같은 Actor 재관측에서 ContactId 개수 유지"), SecondContactIds.Num(), FirstContactIds.Num());
	for (const FName FirstContactId : FirstContactIds)
	{
		TestTrue(TEXT("같은 Actor 재관측에서 기존 ContactId 유지"), SecondContactIds.Contains(FirstContactId));
	}
	TestTrue(TEXT("둘째 Snapshot도 ContactId 중복 없음"), SensorComponent->GetSensorSnapshot().HasUniqueContactIds());

	for (ACFMissileTestTarget* ExpectedTarget : ExpectedTargets)
	{
		TestTrue(TEXT("모든 근거리 Target이 bounded cursor로 공정하게 탐지"), SensorComponent->FindRuntimeContactIndexByActor(ExpectedTarget) != INDEX_NONE);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
