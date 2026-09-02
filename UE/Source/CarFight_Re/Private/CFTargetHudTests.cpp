// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-31
// Description: TS-P0-06 TargetSelect Marker-only HUD 저장 에셋·상태 캐시·UISubsystem ownership 자동화 테스트
// Changelog:
// - v1.2.0: Native Pawn의 Legacy TargetSelectWidgetClass가 더 이상 constructor hard-load되지 않고 Null을 유지하는 UISubsystem 단일 ownership 계약을 검증.
// - v1.1.0: UI-P0-05 이후 Marker-only 계약에 맞춰 의미 텍스트 기대값을 제거하고, 기존 WBP_TargetSelect를 재구성·컴파일·저장하지 않는 load-only 검증으로 전환.
// - v1.0.0: TS-P0-06 TargetSelect HUD 에셋과 상태 표시 자동화 테스트.
// Migration:
// - v1.2.0 WBP_TargetSelect Asset 자체는 load-only로 검증하지만 Pawn Legacy Class 필드가 그 GeneratedClass를 기본값으로 소유해야 한다는 옛 기대는 제거합니다.
// - WBP_TargetSelect는 World Marker만 소유하며 이름·거리·관계·TrackState 의미 텍스트는 Production TargetPanel이 소유합니다.
// - 이 테스트는 저장 Asset을 변경하지 않으며 Transient Widget에는 PlayerController/Viewport projection이 없으므로 marker projected visibility를 True로 기대하지 않습니다.

#include "UI/CFTargetSelectWidget.h"

#include "CFTargetSelectComp.h"
#include "CFTargetSelectContractTestTypes.h"
#include "CFVehiclePawn.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/WidgetTree.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "WidgetBlueprint.h"

namespace
{
	// 저장된 WBP_TargetSelect의 고정 ObjectPath입니다.
	const TCHAR* TargetSelectWidgetObjectPath = TEXT("/Game/CarFight/UI/WBP_TargetSelect.WBP_TargetSelect");

	// 저장 Asset을 수정하지 않고 현재 WBP_TargetSelect를 load-only로 반환합니다.
	UWidgetBlueprint* LoadTargetHudWidgetBlueprint()
	{
		return LoadObject<UWidgetBlueprint>(nullptr, TargetSelectWidgetObjectPath);
	}

	// Content Asset 없이 Transient World에 TargetSelectable 계약 Actor를 생성합니다.
	ACFTargetSelectContractActor* SpawnHudTarget(UWorld* TestWorld, const FName ActorName, const FVector& Location, const ECFTargetRelation Relation)
	{
		// 결정적 Actor 이름을 지정할 spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		// Target HUD 상태 캐시 검증에 사용할 transient 대상입니다.
		ACFTargetSelectContractActor* TargetActor = TestWorld ? TestWorld->SpawnActor<ACFTargetSelectContractActor>(
			ACFTargetSelectContractActor::StaticClass(),
			Location,
			FRotator::ZeroRotator,
			SpawnParameters) : nullptr;
		if (TargetActor)
		{
			TargetActor->DisplayInfo.TargetId = ActorName;
			TargetActor->DisplayInfo.DisplayName = FText::FromName(ActorName);
			TargetActor->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
			TargetActor->DisplayInfo.Relation = Relation;
			TargetActor->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
		}
		return TargetActor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetHudTest,
	"CarFight.TargetSelect.TS_P0_06.TargetHud",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 현재 Marker-only Target HUD의 저장 에셋 구조와 TargetSelect 이벤트 캐시 계약을 검증합니다.
bool FCFTargetHudTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 저장된 WBP_TargetSelect를 수정 없이 읽은 WidgetBlueprint입니다.
	UWidgetBlueprint* WidgetBlueprint = LoadTargetHudWidgetBlueprint();
	TestNotNull(TEXT("WBP_TargetSelect load-only 성공"), WidgetBlueprint);
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !WidgetBlueprint->GeneratedClass)
	{
		return false;
	}

	TestTrue(TEXT("WBP_TargetSelect 부모 클래스"), WidgetBlueprint->GeneratedClass->IsChildOf(UCFTargetSelectWidget::StaticClass()));
	TestNotNull(TEXT("후보 Root"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("VerticalBox_CandidateRoot")));
	TestNotNull(TEXT("후보 형태"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_CandidateMarker")));
	TestNotNull(TEXT("후보 정보 호환 위젯"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_CandidateInfo")));
	TestNotNull(TEXT("선택 Root"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("VerticalBox_SelectedRoot")));
	TestNotNull(TEXT("선택 형태"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_SelectedMarker")));
	TestNotNull(TEXT("선택 정보 호환 위젯"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_SelectedInfo")));
	TestNotNull(TEXT("선택 추적 상태 호환 위젯"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_SelectedTrackState")));

	// Content Asset을 저장하지 않고 TargetSelect 이벤트 캐시를 검증할 transient World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("Target HUD 테스트 월드"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}

	AddExpectedError(TEXT("VehicleVisualHitCollision: SM_Body component missing on TargetHudVehiclePawn."), EAutomationExpectedErrorFlags::Contains, 1);

	// 실제 C++ TargetSelectComp를 소유하는 transient VehiclePawn의 spawn 설정입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("TargetHudVehiclePawn");

	// Target HUD 이벤트 source가 될 transient 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	// 후보·선택 상태 A를 검증할 적대 Targetable Actor입니다.
	ACFTargetSelectContractActor* TargetA = SpawnHudTarget(
		TestWorld,
		TEXT("HudTargetA"),
		FVector(3000.0f, 0.0f, 100.0f),
		ECFTargetRelation::Hostile);
	// 선택과 다른 후보 B를 검증할 중립 Targetable Actor입니다.
	ACFTargetSelectContractActor* TargetB = SpawnHudTarget(
		TestWorld,
		TEXT("HudTargetB"),
		FVector(4500.0f, 200.0f, 100.0f),
		ECFTargetRelation::Neutral);

	TestNotNull(TEXT("Target HUD 차량 Pawn"), VehiclePawn);
	TestNotNull(TEXT("Target HUD 대상 A"), TargetA);
	TestNotNull(TEXT("Target HUD 대상 B"), TargetB);
	if (!VehiclePawn || !TargetA || !TargetB)
	{
		return false;
	}

	TestNull(TEXT("Pawn Legacy TargetSelect HUD 클래스는 runtime 기본값 Null"), VehiclePawn->TargetSelectWidgetClass.Get());
	TestTrue(TEXT("Pawn TargetSelect HUD 기본 표시 정책 허용"), VehiclePawn->bShowTargetSelectHud);
	TestEqual(TEXT("Pawn Legacy TargetSelect HUD ZOrder 저장 호환값 유지"), VehiclePawn->TargetSelectHudZOrder, 20);

	// 실제 선택 상태 owner인 차량 TargetSelect 컴포넌트입니다.
	UCFTargetSelectComp* TargetSelectComp = VehiclePawn->GetTargetSelectComp();
	// Viewport projection 없이 이벤트 캐시 의미만 검증할 transient Marker Widget입니다.
	UCFTargetSelectWidget* TargetWidget = NewObject<UCFTargetSelectWidget>(GetTransientPackage());
	TestNotNull(TEXT("TargetSelectComp"), TargetSelectComp);
	TestNotNull(TEXT("TargetSelectWidget 테스트 인스턴스"), TargetWidget);
	if (!TargetSelectComp || !TargetWidget)
	{
		return false;
	}

	TargetSelectComp->bAutoRefreshCandidate = false;
	TargetWidget->SetVehiclePawnRef(VehiclePawn);

	// 현재 TargetSelectComp의 기본 선택 필터 계약입니다.
	const FCFTargetSelectionContext SelectionContext = TargetSelectComp->GetDefaultSelectionContext();

	// Target A를 현재 후보로 전달할 후보 데이터입니다.
	FCFTargetCandidate CandidateA;
	CandidateA.TargetActor = TargetA;
	CandidateA.TargetWorldLocation = TargetA->GetActorLocation();
	CandidateA.DisplayInfo = TargetA->DisplayInfo;
	CandidateA.WorldDistanceCm = 3000.0f;

	TestTrue(TEXT("후보 A 설정"), TargetSelectComp->SetCurrentCandidate(CandidateA, SelectionContext));
	TestEqual(TEXT("후보 A 캐시"), TargetWidget->GetCachedCandidateActor(), static_cast<AActor*>(TargetA));
	TestTrue(TEXT("Marker-only 후보 의미 텍스트 비움"), TargetWidget->GetCandidateInfoText().IsEmpty());
	TestFalse(TEXT("Transient Widget은 projection context 없이 후보 Marker 숨김"), TargetWidget->IsCandidateMarkerVisible());

	TestTrue(TEXT("대상 A 선택"), TargetSelectComp->SetSelectedTarget(TargetA, SelectionContext));
	TestEqual(TEXT("선택 A 캐시"), TargetWidget->GetCachedSelectedActor(), static_cast<AActor*>(TargetA));
	TestTrue(TEXT("Marker-only 선택 의미 텍스트 비움"), TargetWidget->GetSelectedInfoText().IsEmpty());
	TestTrue(TEXT("Marker-only 선택 상태 텍스트 비움"), TargetWidget->GetSelectedTrackStateText().IsEmpty());
	TestEqual(TEXT("선택 기본 형태"), TargetWidget->GetSelectedMarkerGlyphText().ToString(), FString(TEXT("▣")));
	TestFalse(TEXT("Transient Widget은 projection context 없이 선택 Marker 숨김"), TargetWidget->IsSelectedMarkerVisible());

	// 선택 A와 동시에 다른 후보 B를 캐시할 후보 데이터입니다.
	FCFTargetCandidate CandidateB;
	CandidateB.TargetActor = TargetB;
	CandidateB.TargetWorldLocation = TargetB->GetActorLocation();
	CandidateB.DisplayInfo = TargetB->DisplayInfo;
	CandidateB.WorldDistanceCm = 4500.0f;

	TestTrue(TEXT("후보 B 설정"), TargetSelectComp->SetCurrentCandidate(CandidateB, SelectionContext));
	TestEqual(TEXT("후보 B 캐시"), TargetWidget->GetCachedCandidateActor(), static_cast<AActor*>(TargetB));
	TestTrue(TEXT("후보 B 의미 텍스트도 Marker에서 비움"), TargetWidget->GetCandidateInfoText().IsEmpty());

	TestTrue(TEXT("선택 A 가림"), TargetSelectComp->SetSelectedTargetTrackState(TargetA, ECFTargetTrackState::Occluded));
	TestTrue(TEXT("가림 상태 의미 텍스트도 Marker에서 비움"), TargetWidget->GetSelectedTrackStateText().IsEmpty());
	TestEqual(TEXT("가림 형태"), TargetWidget->GetSelectedMarkerGlyphText().ToString(), FString(TEXT("▧")));

	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	TestNull(TEXT("Manual 해제 뒤 선택 캐시 Null"), TargetWidget->GetCachedSelectedActor());
	TestEqual(TEXT("Manual 해제 뒤 후보 B 캐시 유지"), TargetWidget->GetCachedCandidateActor(), static_cast<AActor*>(TargetB));
	TestFalse(TEXT("Transient Widget 후보는 projection context 없이 계속 숨김"), TargetWidget->IsCandidateMarkerVisible());

	TargetB->Destroy();
	TargetSelectComp->ClearCurrentCandidate();
	TargetWidget->RefreshFromTargetSelect();
	TestNull(TEXT("후보 파괴 뒤 캐시 Null"), TargetWidget->GetCachedCandidateActor());
	TestFalse(TEXT("후보 파괴 뒤 숨김"), TargetWidget->IsCandidateMarkerVisible());

	TargetWidget->SetVehiclePawnRef(nullptr);
	TargetA->Destroy();
	VehiclePawn->Destroy();
	return true;
}

#endif
