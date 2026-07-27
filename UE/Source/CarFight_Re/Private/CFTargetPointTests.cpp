// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-24
// Description: TS-P0-02 타겟 포인트와 위치 Fallback 자동화 테스트
// Scope: TargetPoint 우선, Bounds 중심, Actor 위치, Null 안전성과 차량 기본 서브오브젝트를 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFTargetPointComp.h"
#include "CFVehiclePawn.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetPointResolutionTest,
	"CarFight.TargetSelect.TS_P0_02.TargetPoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetPointResolutionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Automation 테스트 월드가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	AActor* TargetActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("타겟 포인트 테스트 Actor 생성"), TargetActor))
	{
		return false;
	}

	USceneComponent* RootComponent = NewObject<USceneComponent>(TargetActor, TEXT("TargetPointTestRoot"));
	if (!TestNotNull(TEXT("테스트 RootComponent 생성"), RootComponent))
	{
		return false;
	}

	TargetActor->AddInstanceComponent(RootComponent);
	TargetActor->SetRootComponent(RootComponent);
	RootComponent->RegisterComponent();
	TargetActor->SetActorLocation(FVector(1000.0f, 2000.0f, 3000.0f));

	UBoxComponent* BoundsComponent = NewObject<UBoxComponent>(TargetActor, TEXT("TargetPointTestBounds"));
	if (!TestNotNull(TEXT("테스트 BoundsComponent 생성"), BoundsComponent))
	{
		return false;
	}

	TargetActor->AddInstanceComponent(BoundsComponent);
	BoundsComponent->SetupAttachment(RootComponent);
	BoundsComponent->InitBoxExtent(FVector(100.0f, 50.0f, 25.0f));
	BoundsComponent->SetRelativeLocation(FVector(25.0f, -10.0f, 50.0f));
	BoundsComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoundsComponent->RegisterComponent();

	UCFTargetPointComp* TargetPointComponent = NewObject<UCFTargetPointComp>(TargetActor, TEXT("TargetPointTestComponent"));
	if (!TestNotNull(TEXT("테스트 TargetPointComponent 생성"), TargetPointComponent))
	{
		return false;
	}

	TargetActor->AddInstanceComponent(TargetPointComponent);
	TargetPointComponent->SetupAttachment(RootComponent);
	TargetPointComponent->SetRelativeLocation(FVector(200.0f, 20.0f, 120.0f));
	TargetPointComponent->RegisterComponent();

	const FCFTargetPointResult BoundsResult = UCFTargetPointComp::ResolveTargetPoint(TargetActor);
	TestTrue(TEXT("비활성 TargetPoint가 있으면 Bounds Fallback 유효"), BoundsResult.bIsValid);
	TestEqual(TEXT("비활성 TargetPoint 위치 출처는 ActorBounds"), BoundsResult.Source, ECFTargetPointSource::ActorBounds);
	TestTrue(TEXT("Bounds 중심은 Primitive 상대 위치를 반영"), BoundsResult.WorldLocation.Equals(FVector(1025.0f, 1990.0f, 3050.0f), 0.1f));

			TargetPointComponent->bUseAsTargetPoint = true;
	TargetPointComponent->Deactivate();
	const FCFTargetPointResult InactiveTargetPointResult = UCFTargetPointComp::ResolveTargetPoint(TargetActor);
	TestEqual(TEXT("비활성 TargetPoint는 ActorBounds로 Fallback"), InactiveTargetPointResult.Source, ECFTargetPointSource::ActorBounds);

	TargetPointComponent->Activate();
	const FCFTargetPointResult TargetPointResult = UCFTargetPointComp::ResolveTargetPoint(TargetActor);
	TestTrue(TEXT("활성 TargetPoint 위치 유효"), TargetPointResult.bIsValid);
	TestEqual(TEXT("활성 TargetPoint가 Bounds보다 우선"), TargetPointResult.Source, ECFTargetPointSource::TargetPoint);
	TestTrue(TEXT("TargetPoint 상대 위치를 월드 위치로 해석"), TargetPointResult.WorldLocation.Equals(FVector(1200.0f, 2020.0f, 3120.0f), 0.1f));
	TestEqual(TEXT("컴포넌트 getter도 같은 TargetPoint 월드 위치 반환"), TargetPointComponent->GetTargetPointWorldLocation(), TargetPointResult.WorldLocation);
		TestTrue(TEXT("디버그 요약은 TargetPoint 출처 포함"), TargetPointComponent->BuildTargetPointDebugSummary().Contains(TEXT("Source=TargetPoint")));

	TargetPointComponent->bAutoAlignToPreferredBounds = true;
	TargetPointComponent->PreferredBoundsComponentName = BoundsComponent->GetFName();
	TargetPointComponent->PreferredBoundsLocalOffset = FVector(0.0f, 0.0f, 15.0f);
	TestTrue(TEXT("선호 Bounds 중심 자동 정렬 성공"), TargetPointComponent->AlignToPreferredBoundsComponent());
	TestEqual(TEXT("자동 정렬 후 Bounds 컴포넌트에 부착"), TargetPointComponent->GetAttachParent(), static_cast<USceneComponent*>(BoundsComponent));
	TestTrue(TEXT("Bounds 중심과 로컬 오프셋을 월드 위치에 반영"), TargetPointComponent->GetComponentLocation().Equals(FVector(1025.0f, 1990.0f, 3065.0f), 0.1f));
	TargetPointComponent->bAutoAlignToPreferredBounds = false;
	TargetPointComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	TargetPointComponent->bUseAsTargetPoint = false;
	BoundsComponent->DestroyComponent();
	const FCFTargetPointResult ActorLocationResult = UCFTargetPointComp::ResolveTargetPoint(TargetActor);
	TestTrue(TEXT("Primitive Bounds가 없으면 Actor 위치 유효"), ActorLocationResult.bIsValid);
	TestEqual(TEXT("Primitive Bounds가 없으면 ActorLocation 출처"), ActorLocationResult.Source, ECFTargetPointSource::ActorLocation);
	TestTrue(TEXT("ActorLocation Fallback은 실제 Actor 위치"), ActorLocationResult.WorldLocation.Equals(TargetActor->GetActorLocation(), 0.1f));

	const FCFTargetPointResult NullResult = UCFTargetPointComp::ResolveTargetPoint(nullptr);
	TestFalse(TEXT("Null Actor 위치는 무효"), NullResult.bIsValid);
	TestEqual(TEXT("Null Actor 출처는 Invalid"), NullResult.Source, ECFTargetPointSource::Invalid);
	TestEqual(TEXT("Null Actor 위치는 ZeroVector"), NullResult.WorldLocation, FVector::ZeroVector);

	const ACFVehiclePawn* VehicleClassDefaultObject = GetDefault<ACFVehiclePawn>();
	if (TestNotNull(TEXT("ACFVehiclePawn CDO 존재"), VehicleClassDefaultObject))
	{
		UCFTargetPointComp* VehicleTargetPoint = VehicleClassDefaultObject->GetTargetPointComp();
		if (TestNotNull(TEXT("차량은 TargetPoint 기본 서브오브젝트 보유"), VehicleTargetPoint))
		{
						TestTrue(TEXT("차량은 명시 TargetPoint를 기본 사용"), VehicleTargetPoint->bUseAsTargetPoint);
			TestTrue(TEXT("차량 TargetPoint는 선호 Bounds 자동 정렬 사용"), VehicleTargetPoint->bAutoAlignToPreferredBounds);
			TestEqual(TEXT("차량 TargetPoint 선호 Bounds는 SM_Body"), VehicleTargetPoint->PreferredBoundsComponentName, FName(TEXT("SM_Body")));
		}
	}

	TargetActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
