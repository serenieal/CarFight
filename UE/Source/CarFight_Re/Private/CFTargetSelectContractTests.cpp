// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-07-23
// Description: TS-P0-01 타겟 선택 상태 컴포넌트 런타임 계약 자동화 테스트
// Scope: 입력 거부, Native/Blueprint 인터페이스 디스패치, 필터, 이벤트 중복 억제, Fallback, 상태 전이와 약한 참조 무효화를 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFTargetSelectComp.h"
#include "CFTargetSelectData.h"
#include "CFTargetSelectContractTestTypes.h"

#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetSelectRuntimeContractTest,
	"CarFight.TargetSelect.TS_P0_01.RuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetSelectRuntimeContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Automation 테스트 월드가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	AActor* OwnerActor = TestWorld->SpawnActor<AActor>();
	AActor* NonSelectableActor = TestWorld->SpawnActor<AActor>();
	ACFTargetSelectContractActor* TargetA = TestWorld->SpawnActor<ACFTargetSelectContractActor>();
	ACFTargetSelectContractActor* TargetB = TestWorld->SpawnActor<ACFTargetSelectContractActor>();
	ACFTargetSelectContractActor* WeakTarget = TestWorld->SpawnActor<ACFTargetSelectContractActor>();

	if (!TestNotNull(TEXT("Owner Actor 생성"), OwnerActor)
		|| !TestNotNull(TEXT("인터페이스 미구현 Actor 생성"), NonSelectableActor)
		|| !TestNotNull(TEXT("Target A 생성"), TargetA)
		|| !TestNotNull(TEXT("Target B 생성"), TargetB)
		|| !TestNotNull(TEXT("Weak Target 생성"), WeakTarget))
	{
		return false;
	}

	TargetA->DisplayInfo.TargetId = TEXT("TargetA");
	TargetA->DisplayInfo.DisplayName = FText::FromString(TEXT("Target A"));
	TargetA->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
	TargetA->DisplayInfo.Relation = ECFTargetRelation::Unknown;
	TargetA->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
	TargetA->DisplayInfo.AttributeTags = { TEXT("Vehicle"), TEXT("Repairable") };

	TargetB->DisplayInfo.TargetId = TEXT("TargetB");
	TargetB->DisplayInfo.DisplayName = FText::FromString(TEXT("Target B"));
	TargetB->DisplayInfo.TargetCategory = ECFTargetCategory::Device;
	TargetB->DisplayInfo.Relation = ECFTargetRelation::Hostile;
	TargetB->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Detected;
	TargetB->DisplayInfo.AttributeTags = { TEXT("Device"), TEXT("Hackable") };

	WeakTarget->DisplayInfo.TargetId = TEXT("WeakTarget");
	WeakTarget->DisplayInfo.DisplayName = FText::FromString(TEXT("Weak Target"));
	WeakTarget->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
	WeakTarget->DisplayInfo.Relation = ECFTargetRelation::Neutral;
	WeakTarget->DisplayInfo.AttributeTags = { TEXT("Vehicle") };

	UCFTargetSelectComp* TargetSelectComp = NewObject<UCFTargetSelectComp>(OwnerActor, TEXT("TargetSelectContractComp"));
	if (!TestNotNull(TEXT("TargetSelectComp 생성"), TargetSelectComp))
	{
		return false;
	}

	OwnerActor->AddInstanceComponent(TargetSelectComp);
	TargetSelectComp->RegisterComponent();

	TStrongObjectPtr<UCFTargetSelectContractProbe> EventProbe(NewObject<UCFTargetSelectContractProbe>());
	if (!TestNotNull(TEXT("이벤트 Probe 생성"), EventProbe.Get()))
	{
		return false;
	}

	TargetSelectComp->OnTargetCandidateChanged.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleCandidateChanged);
	TargetSelectComp->OnSelectedTargetChanged.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetChanged);
	TargetSelectComp->OnSelectedTargetCleared.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetCleared);
	TargetSelectComp->OnSelectedTargetValidityChanged.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetValidityChanged);
	TargetSelectComp->OnSelectedTargetTrackStateChanged.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetTrackChanged);

		FCFTargetSelectionContext DefaultContext;

	TestTrue(TEXT("Target A는 유효한 Actor"), IsValid(TargetA));
	TestTrue(TEXT("Target A 클래스는 TargetSelectable 인터페이스 구현"), TargetA->GetClass()->ImplementsInterface(UCFTargetSelectable::StaticClass()));
	TestTrue(TEXT("Target A 직접 C++ 구현은 선택 허용"), TargetA->IsTargetSelectable_Implementation(DefaultContext));
		TestNotNull(TEXT("Target A Native 인터페이스 주소를 확인할 수 있어야 함"), Cast<ICFTargetSelectable>(TargetA));

	TestFalse(TEXT("Null Actor는 선택 불가"), TargetSelectComp->CanUseActorAsTarget(nullptr, DefaultContext));
	TestFalse(TEXT("인터페이스 미구현 Actor는 선택 불가"), TargetSelectComp->CanUseActorAsTarget(NonSelectableActor, DefaultContext));
	TestFalse(TEXT("기본 컨텍스트에서 Owner 자기 자신은 선택 불가"), TargetSelectComp->CanUseActorAsTarget(OwnerActor, DefaultContext));
	TestTrue(TEXT("빈 필터는 선택 가능 인터페이스 Actor를 허용"), TargetSelectComp->CanUseActorAsTarget(TargetA, DefaultContext));

	TargetA->bSelectable = false;
	TestFalse(TEXT("대상 자체가 거부하면 선택 불가"), TargetSelectComp->CanUseActorAsTarget(TargetA, DefaultContext));
	TargetA->bSelectable = true;

	FCFTargetSelectionContext FilterContext;
	FilterContext.AllowedCategories = { ECFTargetCategory::Vehicle };
	TestTrue(TEXT("허용 분류 Vehicle 통과"), TargetSelectComp->CanUseActorAsTarget(TargetA, FilterContext));
	TestFalse(TEXT("허용 분류 밖 Device 거부"), TargetSelectComp->CanUseActorAsTarget(TargetB, FilterContext));

	FilterContext = FCFTargetSelectionContext();
	FilterContext.AllowedRelations = { ECFTargetRelation::Unknown };
	TestTrue(TEXT("허용 관계 Unknown 통과"), TargetSelectComp->CanUseActorAsTarget(TargetA, FilterContext));
	TestFalse(TEXT("허용 관계 밖 Hostile 거부"), TargetSelectComp->CanUseActorAsTarget(TargetB, FilterContext));

	FilterContext = FCFTargetSelectionContext();
	FilterContext.RequiredAttributeTags = { TEXT("Vehicle"), TEXT("Repairable") };
	TestTrue(TEXT("모든 필수 태그 보유 시 통과"), TargetSelectComp->CanUseActorAsTarget(TargetA, FilterContext));
	FilterContext.RequiredAttributeTags.Add(TEXT("MissingTag"));
	TestFalse(TEXT("필수 태그 하나라도 누락 시 거부"), TargetSelectComp->CanUseActorAsTarget(TargetA, FilterContext));

	FilterContext = FCFTargetSelectionContext();
	FilterContext.ExcludedAttributeTags = { TEXT("Vehicle") };
	TestFalse(TEXT("제외 태그 보유 시 거부"), TargetSelectComp->CanUseActorAsTarget(TargetA, FilterContext));
	FilterContext.ExcludedAttributeTags = { TEXT("NotPresent") };
	TestTrue(TEXT("제외 태그가 없으면 통과"), TargetSelectComp->CanUseActorAsTarget(TargetA, FilterContext));

	EventProbe->ResetCounts();
	FCFTargetCandidate CandidateA;
	CandidateA.TargetActor = TargetA;
	CandidateA.DisplayInfo = TargetA->DisplayInfo;
	CandidateA.WorldDistanceCm = 100.0f;

	TestTrue(TEXT("첫 후보 설정 성공"), TargetSelectComp->SetCurrentCandidate(CandidateA, DefaultContext));
	TestEqual(TEXT("첫 후보 설정 이벤트 1회"), EventProbe->CandidateChangedCount, 1);
	TestEqual(TEXT("후보 상태 CandidateAvailable"), TargetSelectComp->GetTargetCoreState(), ECFTargetCoreState::CandidateAvailable);

	CandidateA.WorldDistanceCm = 250.0f;
	TestTrue(TEXT("같은 Actor 후보 데이터 갱신 성공"), TargetSelectComp->SetCurrentCandidate(CandidateA, DefaultContext));
	TestEqual(TEXT("같은 Actor 재설정은 후보 변경 이벤트 없음"), EventProbe->CandidateChangedCount, 1);
	TestEqual(TEXT("같은 Actor라도 후보 데이터는 최신값으로 갱신"), TargetSelectComp->GetCurrentCandidateData().WorldDistanceCm, 250.0f);

	TargetSelectComp->ClearCurrentCandidate();
	TestEqual(TEXT("후보 해제 이벤트 1회 추가"), EventProbe->CandidateChangedCount, 2);
	TargetSelectComp->ClearCurrentCandidate();
	TestEqual(TEXT("빈 후보 재해제는 이벤트 없음"), EventProbe->CandidateChangedCount, 2);
	TestEqual(TEXT("후보 해제 후 NoCandidate"), TargetSelectComp->GetTargetCoreState(), ECFTargetCoreState::NoCandidate);

	EventProbe->ResetCounts();
	TestTrue(TEXT("첫 선택 대상 설정 성공"), TargetSelectComp->SetSelectedTarget(TargetA, DefaultContext));
	TestEqual(TEXT("첫 선택 변경 이벤트 1회"), EventProbe->SelectedTargetChangedCount, 1);
	TestTrue(TEXT("선택 대상 유효"), TargetSelectComp->IsSelectedTargetValid());
	TestEqual(TEXT("선택 상태 TargetSelected"), TargetSelectComp->GetTargetCoreState(), ECFTargetCoreState::TargetSelected);
	TestEqual(TEXT("선택 표시 정보 캐시"), TargetSelectComp->GetSelectedTargetDisplayInfo().TargetId, FName(TEXT("TargetA")));

	TestTrue(TEXT("같은 대상 재선택 성공 No-op"), TargetSelectComp->SetSelectedTarget(TargetA, DefaultContext));
	TestEqual(TEXT("같은 대상 재선택은 이벤트 없음"), EventProbe->SelectedTargetChangedCount, 1);

	TestTrue(TEXT("다른 대상 선택 성공"), TargetSelectComp->SetSelectedTarget(TargetB, DefaultContext));
	TestEqual(TEXT("다른 대상 선택 시 이벤트 1회 추가"), EventProbe->SelectedTargetChangedCount, 2);
	TestEqual(TEXT("새 대상 표시 정보 캐시"), TargetSelectComp->GetSelectedTargetDisplayInfo().TargetId, FName(TEXT("TargetB")));

	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	TestEqual(TEXT("선택 해제 이벤트 1회"), EventProbe->SelectedTargetClearedCount, 1);
	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	TestEqual(TEXT("빈 선택 재해제는 이벤트 없음"), EventProbe->SelectedTargetClearedCount, 1);

	EventProbe->ResetCounts();
	TestTrue(TEXT("유효성·추적 검증 대상 설정"), TargetSelectComp->SetSelectedTarget(TargetA, DefaultContext));
	TestEqual(TEXT("대상 초기 추적 상태 Visible"), TargetSelectComp->GetSelectedTargetTrackState(), ECFTargetTrackState::Visible);

	TestTrue(TEXT("같은 유효성 True 통지 성공"), TargetSelectComp->NotifySelectedTargetValidityChanged(TargetA, true));
	TestEqual(TEXT("같은 유효성은 이벤트 없음"), EventProbe->SelectedTargetValidityChangedCount, 0);
	TestFalse(TEXT("다른 대상의 유효성 통지는 거부"), TargetSelectComp->NotifySelectedTargetValidityChanged(TargetB, false));
	TestTrue(TEXT("선택 대상 무효 통지 성공"), TargetSelectComp->NotifySelectedTargetValidityChanged(TargetA, false));
	TestEqual(TEXT("유효성 변경 이벤트 1회"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestFalse(TEXT("무효 통지 후 선택 대상 무효"), TargetSelectComp->IsSelectedTargetValid());
	TestEqual(TEXT("무효 통지 후 SelectedTargetInvalid"), TargetSelectComp->GetTargetCoreState(), ECFTargetCoreState::SelectedTargetInvalid);
	TestEqual(TEXT("무효 선택 대상 추적 상태 조회는 Invalid"), TargetSelectComp->GetSelectedTargetTrackState(), ECFTargetTrackState::Invalid);

	TestTrue(TEXT("같은 무효 상태 재통지 성공"), TargetSelectComp->NotifySelectedTargetValidityChanged(TargetA, false));
	TestEqual(TEXT("같은 무효 상태 재통지는 이벤트 없음"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestTrue(TEXT("선택 대상 유효 복구 통지 성공"), TargetSelectComp->NotifySelectedTargetValidityChanged(TargetA, true));
	TestEqual(TEXT("유효 복구 이벤트 1회 추가"), EventProbe->SelectedTargetValidityChangedCount, 2);

	TestTrue(TEXT("같은 추적 상태 Visible 설정 성공"), TargetSelectComp->SetSelectedTargetTrackState(TargetA, ECFTargetTrackState::Visible));
	TestEqual(TEXT("같은 추적 상태는 이벤트 없음"), EventProbe->SelectedTargetTrackChangedCount, 0);
	TestFalse(TEXT("다른 대상의 추적 상태 변경은 거부"), TargetSelectComp->SetSelectedTargetTrackState(TargetB, ECFTargetTrackState::Occluded));
	TestTrue(TEXT("선택 대상 Occluded 전환 성공"), TargetSelectComp->SetSelectedTargetTrackState(TargetA, ECFTargetTrackState::Occluded));
	TestEqual(TEXT("추적 상태 변경 이벤트 1회"), EventProbe->SelectedTargetTrackChangedCount, 1);
	TestEqual(TEXT("선택 대상 추적 상태 Occluded"), TargetSelectComp->GetSelectedTargetTrackState(), ECFTargetTrackState::Occluded);
	TestTrue(TEXT("같은 Occluded 재설정 성공"), TargetSelectComp->SetSelectedTargetTrackState(TargetA, ECFTargetTrackState::Occluded));
	TestEqual(TEXT("같은 추적 상태 재설정은 이벤트 없음"), EventProbe->SelectedTargetTrackChangedCount, 1);

	TargetSelectComp->FallbackTargetSelectConfig.DirectSelectMaxDistanceCm = 111.0f;
	TargetSelectComp->TargetSelectData = nullptr;
	TestEqual(TEXT("DataAsset 없음 Fallback 사용"), TargetSelectComp->GetResolvedTargetSelectConfig().DirectSelectMaxDistanceCm, 111.0f);

	UCFTargetSelectData* ValidData = NewObject<UCFTargetSelectData>(TargetSelectComp, TEXT("ValidTargetSelectData"));
	ValidData->TargetSelectConfig.DirectSelectMaxDistanceCm = 222.0f;
	TargetSelectComp->TargetSelectData = ValidData;
	TestTrue(TEXT("유효 DataAsset 설정 검증 통과"), ValidData->IsTargetSelectConfigValid());
	TestEqual(TEXT("유효 DataAsset이 Fallback보다 우선"), TargetSelectComp->GetResolvedTargetSelectConfig().DirectSelectMaxDistanceCm, 222.0f);

	ValidData->TargetSelectConfig.CandidateRefreshIntervalSec = 0.0f;
	TestFalse(TEXT("0초 갱신 간격 DataAsset은 무효"), ValidData->IsTargetSelectConfigValid());
	TestEqual(TEXT("무효 DataAsset은 Fallback 사용"), TargetSelectComp->GetResolvedTargetSelectConfig().DirectSelectMaxDistanceCm, 111.0f);

	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	FCFTargetCandidate WeakCandidate;
	WeakCandidate.TargetActor = WeakTarget;
	WeakCandidate.DisplayInfo = WeakTarget->DisplayInfo;
	TestTrue(TEXT("약한 참조 후보 설정"), TargetSelectComp->SetCurrentCandidate(WeakCandidate, DefaultContext));
	TestTrue(TEXT("약한 참조 선택 대상 설정"), TargetSelectComp->SetSelectedTarget(WeakTarget, DefaultContext));
	TestTrue(TEXT("선택 기록 존재"), TargetSelectComp->HasSelectedTarget());

		TestTrue(TEXT("Weak Target Destroy 성공"), WeakTarget->Destroy());
	TestFalse(TEXT("Destroy된 후보는 유효 후보로 반환하지 않음"), TargetSelectComp->HasCurrentCandidate());
	TestNull(TEXT("Destroy된 후보 Actor getter는 Null"), TargetSelectComp->GetCurrentCandidateActor());
	TestFalse(TEXT("Destroy된 선택 대상은 수명 구독으로 자동 해제"), TargetSelectComp->HasSelectedTarget());
	TestFalse(TEXT("Destroy된 선택 대상은 유효하지 않음"), TargetSelectComp->IsSelectedTargetValid());
	TestNull(TEXT("Destroy된 선택 대상 Actor getter는 Null"), TargetSelectComp->GetSelectedTargetActor());
	TestEqual(TEXT("Destroy 후 후보와 선택이 없으면 NoCandidate"), TargetSelectComp->GetTargetCoreState(), ECFTargetCoreState::NoCandidate);
	TestEqual(TEXT("Destroy 자동 해제 사유"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::Destroyed);
	TestEqual(TEXT("Destroy된 선택 대상 추적 상태는 Invalid"), TargetSelectComp->GetSelectedTargetTrackState(), ECFTargetTrackState::Invalid);

	TargetSelectComp->OnTargetCandidateChanged.RemoveAll(EventProbe.Get());
	TargetSelectComp->OnSelectedTargetChanged.RemoveAll(EventProbe.Get());
	TargetSelectComp->OnSelectedTargetCleared.RemoveAll(EventProbe.Get());
	TargetSelectComp->OnSelectedTargetValidityChanged.RemoveAll(EventProbe.Get());
	TargetSelectComp->OnSelectedTargetTrackStateChanged.RemoveAll(EventProbe.Get());

	TargetSelectComp->ClearCurrentCandidate();
	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	NonSelectableActor->Destroy();
	TargetA->Destroy();
	TargetB->Destroy();
	OwnerActor->Destroy();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
