// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-16
// Description: CF-FQ-037 SCAN-P0-02 Runtime Config Apply asset-free Automation
// Scope: SensorData 명시 적용, applied-config 고정, invalid 원자 거부, Contact/Knowledge 보존, range 감소 reconcile과 Active Scan 상태 조정을 검증합니다.
// Changelog:
// - v1.0.0: SCAN-P0-02 ConfigApply Automation을 최초 추가.
// Migration:
// - transient Editor World와 transient VehicleSensorData만 사용하며 프로젝트 Content Asset, Blueprint와 InputAction을 생성·수정·저장하지 않습니다.
// - 실제 FittingSnapshot 연결과 Field Fitting transaction은 SCAN-P0-04 범위이므로 이 테스트에서 수행하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleSensorComp.h"
#include "CFVehicleSensorData.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFScannerRuntimeConfigTest,
	"CarFight.Scanner.SCAN_P0_02.ConfigApply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] SensorData apply/reapply가 Runtime Contact와 Knowledge를 보존하면서 Config 감소와 Active Scan 상태만 안전하게 조정하는지 검증합니다.
bool FCFScannerRuntimeConfigTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 등록된 Sensor Component의 Tick 상태까지 asset-free로 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("SCAN-P0-02 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] plain Actor에 등록된 테스트 Sensor Component를 생성하는 공통 helper입니다.
	auto CreateSensorComponent = [TestWorld](const FName OwnerName, const FName ComponentName) -> UCFVehicleSensorComp*
	{
		// [v1.0.0] 테스트별 Owner 이름을 결정적으로 지정할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = OwnerName;

		// [v1.0.0] Sensor Component를 실제 등록 상태로 보유할 테스트 Owner Actor입니다.
		AActor* SensorOwner = TestWorld->SpawnActor<AActor>(
			AActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!SensorOwner)
		{
			return nullptr;
		}

		// [v1.0.0] Content Asset 없이 Runtime Config Apply를 검증할 실제 등록 Sensor Component입니다.
		UCFVehicleSensorComp* SensorComponent = NewObject<UCFVehicleSensorComp>(SensorOwner, ComponentName);
		if (!SensorComponent)
		{
			return nullptr;
		}

		SensorOwner->AddInstanceComponent(SensorComponent);
		SensorComponent->RegisterComponent();
		return SensorComponent;
	};

	// [v1.0.0] transient VehicleSensorData에 유효한 P0 SensorConfig를 채우는 공통 helper입니다.
	auto ConfigureSensorData = [](
		UCFVehicleSensorData* SensorData,
		const float PassiveRangeCm,
		const float ActiveRangeCm,
		const float VisualRangeCm,
		const float ActiveDurationSec)
	{
		if (!SensorData)
		{
			return;
		}

		SensorData->SensorConfig.PassiveDetectionRangeCm = PassiveRangeCm;
		SensorData->SensorConfig.ActiveScanRangeCm = ActiveRangeCm;
		SensorData->SensorConfig.VisualDetectionRangeCm = VisualRangeCm;
		SensorData->SensorConfig.UpdateIntervalSec = 0.1f;
		SensorData->SensorConfig.MaxActorScansPerUpdate = 64;
		SensorData->SensorConfig.ContactMemoryTimeSec = 10.0f;
		SensorData->SensorConfig.DestroyedHoldTimeSec = 1.0f;
		SensorData->SensorConfig.ActiveScanDurationSec = ActiveDurationSec;
		SensorData->SensorConfig.AnalysisGainPerSec = 0.5f;
		SensorData->SensorConfig.AnalysisDecayPerSec = 0.2f;
		SensorData->SensorConfig.IdentifiedThreshold = 0.5f;
		SensorData->SensorConfig.DetailedScanThreshold = 1.0f;
	};

	// [v1.0.0] private RuntimeContacts에 Knowledge가 이미 획득된 Live Contact를 추가하는 helper입니다.
	auto AddKnownLiveContact = [](
		UCFVehicleSensorComp* SensorComponent,
		const FName ContactId,
		const FName TargetId,
		const bool bBaselineDetectionValid,
		const float AnalysisProgress) -> int32
	{
		if (!SensorComponent)
		{
			return INDEX_NONE;
		}

		// [v1.0.0] hot reapply 전후 identity, Knowledge와 Analysis를 비교할 새 private Runtime Contact입니다.
		UCFVehicleSensorComp::FCFSensorContactRuntime& RuntimeContact = SensorComponent->RuntimeContacts.AddDefaulted_GetRef();
		RuntimeContact.SourceDisplayInfo.TargetId = TargetId;
		RuntimeContact.SourceDisplayInfo.DisplayName = FText::FromName(TargetId);
		RuntimeContact.SourceDisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
		RuntimeContact.SourceDisplayInfo.Relation = ECFTargetRelation::Hostile;
		RuntimeContact.PublicContact.ContactId = ContactId;
		RuntimeContact.PublicContact.KnownTargetId = TargetId;
		RuntimeContact.PublicContact.KnownDisplayName = FText::FromName(TargetId);
		RuntimeContact.PublicContact.TargetCategory = ECFTargetCategory::Vehicle;
		RuntimeContact.PublicContact.Relation = ECFTargetRelation::Hostile;
		RuntimeContact.PublicContact.InformationLevel = ECFTargetInfoLevel::DetailedScan;
		RuntimeContact.PublicContact.ContactState = ECFSensorContactState::Live;
		RuntimeContact.PublicContact.LastKnownWorldLocation = FVector(1000.0f, 200.0f, 50.0f);
		RuntimeContact.PublicContact.LastObservedWorldTimeSeconds = 0.0;
		RuntimeContact.PublicContact.FreshnessSeconds = 0.0f;
		RuntimeContact.PublicContact.AnalysisProgress01 = AnalysisProgress;
		RuntimeContact.PublicContact.bDestroyedConfirmed = false;
		RuntimeContact.bBaselineDetectionValidAtLastObservation = bBaselineDetectionValid;
		return SensorComponent->RuntimeContacts.Num() - 1;
	};

	// [v1.0.0] 초기화 전 Source 선택과 InitializeSensorRuntime 입력 연결을 검증할 SensorData입니다.
	UCFVehicleSensorData* InitialSensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	ConfigureSensorData(InitialSensorData, 3000.0f, 6000.0f, 2000.0f, 5.0f);
	if (!TestNotNull(TEXT("초기 적용 SensorData 생성"), InitialSensorData)
		|| !TestTrue(TEXT("초기 적용 SensorData 계약 유효"), InitialSensorData->IsSensorConfigValid()))
	{
		return false;
	}

	// [v1.0.0] Runtime 초기화 전에 ApplySensorData가 암묵적으로 Runtime을 시작하지 않는지 검증할 Component입니다.
	UCFVehicleSensorComp* PreInitializeSensorComponent = CreateSensorComponent(
		TEXT("ScannerPreInitializeOwner"),
		TEXT("ScannerPreInitializeSensor"));
	if (!TestNotNull(TEXT("초기화 전 Sensor Component 생성"), PreInitializeSensorComponent))
	{
		return false;
	}

	TestTrue(TEXT("초기화 전 valid SensorData Source 적용"), PreInitializeSensorComponent->ApplySensorData(InitialSensorData));
	TestFalse(TEXT("Source 적용만으로 Runtime 자동 초기화 금지"), PreInitializeSensorComponent->IsSensorRuntimeReady());
	TestEqual(TEXT("초기화 전 선택 Source 보존"), PreInitializeSensorComponent->SensorData.Get(), InitialSensorData);
	TestTrue(TEXT("선택 Source로 Runtime 초기화"), PreInitializeSensorComponent->InitializeSensorRuntime());
	TestTrue(TEXT("초기화 후 Applied Config Passive 3000"), FMath::IsNearlyEqual(PreInitializeSensorComponent->GetResolvedSensorConfig().PassiveDetectionRangeCm, 3000.0f));

	// [v1.0.0] hot reapply 전체 계약을 한 Runtime 수명에서 검증할 Sensor Component입니다.
	UCFVehicleSensorComp* SensorComponent = CreateSensorComponent(
		TEXT("ScannerRuntimeConfigOwner"),
		TEXT("ScannerRuntimeConfigSensor"));
	if (!TestNotNull(TEXT("Runtime Config Sensor Component 생성"), SensorComponent))
	{
		return false;
	}

	TestTrue(TEXT("scanner-less Fallback으로 Runtime 초기화"), SensorComponent->InitializeSensorRuntime());
	TestFalse(TEXT("0-range Fallback 초기 Tick 비활성"), SensorComponent->IsComponentTickEnabled());

	// [v1.0.0] 최초 hot apply에서 사용할 유효 Scanner SensorData입니다.
	UCFVehicleSensorData* FullSensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	ConfigureSensorData(FullSensorData, 3000.0f, 6000.0f, 2000.0f, 5.0f);
	if (!TestNotNull(TEXT("Full Scanner SensorData 생성"), FullSensorData)
		|| !TestTrue(TEXT("Full Scanner SensorData 계약 유효"), FullSensorData->IsSensorConfigValid()))
	{
		return false;
	}

	// [v1.0.0] 첫 successful apply 전 Snapshot Revision입니다.
	const int32 RevisionBeforeFullApply = SensorComponent->GetSensorSnapshot().Revision;
	TestTrue(TEXT("Runtime Ready 상태 valid SensorData 적용"), SensorComponent->ApplySensorData(FullSensorData));
	TestEqual(TEXT("적용 Source 포인터 갱신"), SensorComponent->SensorData.Get(), FullSensorData);
	TestTrue(TEXT("적용 Config Passive 3000"), FMath::IsNearlyEqual(SensorComponent->GetResolvedSensorConfig().PassiveDetectionRangeCm, 3000.0f));
	TestTrue(TEXT("Passive work가 생기면 Tick 활성"), SensorComponent->IsComponentTickEnabled());
	TestTrue(TEXT("successful apply가 Snapshot Revision 갱신"), SensorComponent->GetSensorSnapshot().Revision > RevisionBeforeFullApply);

	// [v1.0.0] Source UObject가 Runtime 중 직접 변해도 Apply 전까지 유지돼야 할 적용 Passive 값입니다.
	const float AppliedPassiveRangeBeforeSourceMutation = SensorComponent->GetResolvedSensorConfig().PassiveDetectionRangeCm;
	FullSensorData->SensorConfig.PassiveDetectionRangeCm = 3500.0f;
	TestTrue(TEXT("Source UObject 직접 변경은 Applied Config를 우회하지 않음"), FMath::IsNearlyEqual(SensorComponent->GetResolvedSensorConfig().PassiveDetectionRangeCm, AppliedPassiveRangeBeforeSourceMutation));
	FullSensorData->SensorConfig.PassiveDetectionRangeCm = 3000.0f;

	// [v1.0.0] Passive/Visual로 마지막 관측된 Knowledge 보유 Contact 인덱스입니다.
	const int32 BaselineContactIndex = AddKnownLiveContact(
		SensorComponent,
		TEXT("Contact_700001"),
		TEXT("ScannerBaselineTarget"),
		true,
		0.65f);

	// [v1.0.0] Active Scan으로만 마지막 관측된 Knowledge 보유 Contact 인덱스입니다.
	const int32 ActiveOnlyContactIndex = AddKnownLiveContact(
		SensorComponent,
		TEXT("Contact_700002"),
		TEXT("ScannerActiveOnlyTarget"),
		false,
		0.45f);
	if (!TestTrue(TEXT("Baseline Contact 생성"), BaselineContactIndex != INDEX_NONE)
		|| !TestTrue(TEXT("Active-only Contact 생성"), ActiveOnlyContactIndex != INDEX_NONE))
	{
		return false;
	}

	SensorComponent->bActiveScanRunning = true;
	SensorComponent->ActiveScanRemainingSeconds = 4.0f;
	SensorComponent->SetComponentTickEnabled(true);
	SensorComponent->PublishRuntimeSnapshot();

	// [v1.0.0] invalid Source 거부가 Snapshot을 갱신하지 않는지 비교할 Revision입니다.
	const int32 RevisionBeforeInvalidApply = SensorComponent->GetSensorSnapshot().Revision;

	// [v1.0.0] invalid Source 거부 뒤 그대로 유지돼야 할 기존 적용 SensorData입니다.
	UCFVehicleSensorData* AppliedSourceBeforeInvalidApply = SensorComponent->SensorData.Get();

	// [v1.0.0] 명시적 invalid Source의 원자 거부를 검증할 transient SensorData입니다.
	UCFVehicleSensorData* InvalidSensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	ConfigureSensorData(InvalidSensorData, 3000.0f, 6000.0f, 2000.0f, 5.0f);
	InvalidSensorData->SensorConfig.UpdateIntervalSec = 0.0f;
	TestFalse(TEXT("invalid SensorData 계약"), InvalidSensorData->IsSensorConfigValid());
	TestFalse(TEXT("invalid SensorData apply 원자 거부"), SensorComponent->ApplySensorData(InvalidSensorData));
	TestEqual(TEXT("invalid 거부 후 Source 포인터 유지"), SensorComponent->SensorData.Get(), AppliedSourceBeforeInvalidApply);
	TestEqual(TEXT("invalid 거부 후 Snapshot Revision 유지"), SensorComponent->GetSensorSnapshot().Revision, RevisionBeforeInvalidApply);
	TestTrue(TEXT("invalid 거부 후 Active Scan 유지"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("invalid 거부 후 Remaining 유지"), FMath::IsNearlyEqual(SensorComponent->GetActiveScanRemainingSeconds(), 4.0f));
	TestEqual(TEXT("invalid 거부 후 Contact 수 유지"), SensorComponent->RuntimeContacts.Num(), 2);
	TestEqual(TEXT("invalid 거부 후 Baseline Live 유지"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("invalid 거부 후 Active-only Live 유지"), SensorComponent->RuntimeContacts[ActiveOnlyContactIndex].PublicContact.ContactState, ECFSensorContactState::Live);

	// [v1.0.0] Passive/Visual은 유지하고 Active range와 duration만 줄이는 SensorData입니다.
	UCFVehicleSensorData* ReducedActiveSensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	ConfigureSensorData(ReducedActiveSensorData, 3000.0f, 4000.0f, 2000.0f, 2.5f);
	TestTrue(TEXT("Active 감소 SensorData 계약 유효"), ReducedActiveSensorData->IsSensorConfigValid());
	TestTrue(TEXT("Active 감소 SensorData 적용"), SensorComponent->ApplySensorData(ReducedActiveSensorData));
	TestEqual(TEXT("Active 감소에서도 Contact 수 보존"), SensorComponent->RuntimeContacts.Num(), 2);
	TestEqual(TEXT("Baseline 능력 유지 Contact는 Live 유지"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.ContactState, ECFSensorContactState::Live);
	TestEqual(TEXT("Active range 감소 시 Active-only Contact는 LastKnown"), SensorComponent->RuntimeContacts[ActiveOnlyContactIndex].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestTrue(TEXT("지원되는 새 Config에서 Active Scan 계속 실행"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("Active remaining은 새 duration으로 clamp"), FMath::IsNearlyEqual(SensorComponent->GetActiveScanRemainingSeconds(), 2.5f));
	TestEqual(TEXT("Active 감소 후 ContactId 보존"), SensorComponent->RuntimeContacts[ActiveOnlyContactIndex].PublicContact.ContactId, FName(TEXT("Contact_700002")));
	TestEqual(TEXT("Active 감소 후 획득 Knowledge 보존"), SensorComponent->RuntimeContacts[ActiveOnlyContactIndex].PublicContact.KnownTargetId, FName(TEXT("ScannerActiveOnlyTarget")));
	TestTrue(TEXT("Active 감소 후 Analysis 보존"), FMath::IsNearlyEqual(SensorComponent->RuntimeContacts[ActiveOnlyContactIndex].PublicContact.AnalysisProgress01, 0.45f));

	// [v1.0.0] Passive/Visual 능력을 줄이되 Active duration은 크게 늘려 remaining 비증가를 함께 검증할 SensorData입니다.
	UCFVehicleSensorData* ReducedBaselineSensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	ConfigureSensorData(ReducedBaselineSensorData, 1000.0f, 4000.0f, 500.0f, 10.0f);
	TestTrue(TEXT("Baseline 감소 SensorData 계약 유효"), ReducedBaselineSensorData->IsSensorConfigValid());

	// [v1.0.0] Baseline 감소 apply 전 보존돼야 할 Knowledge/Analysis 비교값입니다.
	const float BaselineAnalysisBeforeReduction = SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.AnalysisProgress01;
	TestTrue(TEXT("Baseline 감소 SensorData 적용"), SensorComponent->ApplySensorData(ReducedBaselineSensorData));
	TestEqual(TEXT("Baseline 감소에서도 Contact 즉시 삭제 금지"), SensorComponent->RuntimeContacts.Num(), 2);
	TestEqual(TEXT("Baseline 감소 시 기존 Live는 LastKnown"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestEqual(TEXT("Baseline 감소 후 ContactId 보존"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.ContactId, FName(TEXT("Contact_700001")));
	TestEqual(TEXT("Baseline 감소 후 Detailed Knowledge 보존"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.InformationLevel, ECFTargetInfoLevel::DetailedScan);
	TestEqual(TEXT("Baseline 감소 후 KnownTargetId 보존"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.KnownTargetId, FName(TEXT("ScannerBaselineTarget")));
	TestTrue(TEXT("Baseline 감소 후 AnalysisProgress 보존"), FMath::IsNearlyEqual(SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.AnalysisProgress01, BaselineAnalysisBeforeReduction));
	TestTrue(TEXT("새 duration이 길어도 Active remaining 증가 금지"), FMath::IsNearlyEqual(SensorComponent->GetActiveScanRemainingSeconds(), 2.5f));

	// [v1.0.0] null Apply가 사용해야 할 현재 Component의 scanner-less 0-range Fallback Config입니다.
	const FCFSensorConfig ScannerlessFallbackConfig = SensorComponent->FallbackSensorConfig;
	TestTrue(TEXT("scanner-less Fallback Config 유효"), ScannerlessFallbackConfig.IsValid());
	TestTrue(TEXT("null SensorData를 Fallback Source로 적용"), SensorComponent->ApplySensorData(nullptr));
	TestNull(TEXT("null Apply 뒤 SensorData Source 제거"), SensorComponent->SensorData.Get());
	TestTrue(TEXT("null Apply 뒤 Fallback Passive 0"), FMath::IsNearlyZero(SensorComponent->GetResolvedSensorConfig().PassiveDetectionRangeCm));
	TestFalse(TEXT("Active 미지원 Fallback 적용 시 실행 중 Scan 종료"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("Active 미지원 Fallback 적용 시 Remaining 0"), FMath::IsNearlyZero(SensorComponent->GetActiveScanRemainingSeconds()));
	TestEqual(TEXT("scanner-less 전환에서도 Contact 즉시 삭제 금지"), SensorComponent->RuntimeContacts.Num(), 2);
	TestEqual(TEXT("scanner-less 전환 Baseline Contact LastKnown 유지"), SensorComponent->RuntimeContacts[BaselineContactIndex].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestEqual(TEXT("scanner-less 전환 Active-only Contact LastKnown 유지"), SensorComponent->RuntimeContacts[ActiveOnlyContactIndex].PublicContact.ContactState, ECFSensorContactState::LastKnown);
	TestTrue(TEXT("0-range라도 남은 Contact lifecycle 때문에 Tick 유지"), SensorComponent->IsComponentTickEnabled());
	TestEqual(TEXT("공개 Snapshot에도 Contact 2개 보존"), SensorComponent->GetSensorSnapshot().Contacts.Num(), 2);
	TestTrue(TEXT("최종 Snapshot 공개 계약 유효"), SensorComponent->GetSensorSnapshot().IsPublicContractValid());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
