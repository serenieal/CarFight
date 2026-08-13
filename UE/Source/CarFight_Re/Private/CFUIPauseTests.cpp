// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-06
// Description: CF-FQ-032 UI-P0-02 싱글플레이 Pause 자동화 테스트
// Scope: C++ Pause Menu 상호작용, 차량 입력 중립화와 Launcher·Projectile·Timer 게임 시간 정지 전제 계약을 검증합니다.
// Changelog:
// - v1.1.0: Continue 버튼의 활성·표시·콘텐츠 계약을 추가.
// - v1.0.0: PauseMenuContract, InputNeutralContract와 ProgressFreezeContract를 최초 추가.
// Migration:
// - 테스트는 Transient Widget·Component·Automation World만 사용하며 Unreal Asset과 피팅 데이터를 생성하거나 수정하지 않는다.
// - 실제 Pause 입력·해제와 화면 Focus 체감은 사용자 PIE 항목으로 별도 유지한다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFLauncherComp.h"
#include "CFProjectileActor.h"
#include "CFProjectileMotorComp.h"
#include "CFVehicleDriveComp.h"
#include "UI/CFPauseMenuWidget.h"

#include "Components/Button.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "TimerManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIPauseMenuContractTest,
	"CarFight.UI.UI_P0_02.PauseMenuContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] C++ Pause Menu가 에셋 없이 WidgetTree와 기본 Continue 버튼을 생성하는지 검증합니다.
bool FCFUIPauseMenuContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 에셋 없이 최소 Pause Menu 트리를 검증할 Transient Widget입니다.
	UCFPauseMenuWidget* PauseMenuWidget = NewObject<UCFPauseMenuWidget>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient Pause Menu"), PauseMenuWidget))
	{
		return false;
	}

		TestTrue(TEXT("Pause Menu 초기화"), PauseMenuWidget->Initialize());
	TestTrue(TEXT("Pause Menu 트리 보장"), PauseMenuWidget->EnsurePauseMenuTree());

	// [v1.1.0] 실제 Pause 해제 상호작용 계약을 검증할 Continue 버튼입니다.
	UButton* ContinueButton = PauseMenuWidget->GetContinueButton();
	if (!TestNotNull(TEXT("Continue 버튼 생성"), ContinueButton))
	{
		return false;
	}
	TestTrue(TEXT("Continue 버튼 활성"), ContinueButton->GetIsEnabled());
	TestEqual(TEXT("Continue 버튼 표시 상태"), ContinueButton->GetVisibility(), ESlateVisibility::Visible);
	TestNotNull(TEXT("Continue 버튼 콘텐츠 생성"), ContinueButton->GetContent());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIPauseInputNeutralTest,
	"CarFight.UI.UI_P0_02.InputNeutralContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Pause 진입용 Drive 입력 초기화가 모든 잔류 입력을 중립값으로 되돌리는지 검증합니다.
bool FCFUIPauseInputNeutralTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Owner와 Chaos Movement 없이도 입력 상태 계약을 검증할 Transient DriveComp입니다.
	UCFVehicleDriveComp* VehicleDriveComp = NewObject<UCFVehicleDriveComp>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient VehicleDriveComp"), VehicleDriveComp))
	{
		return false;
	}

	VehicleDriveComp->ApplyThrottleInput(0.75f);
	VehicleDriveComp->ApplySteeringInput(-0.50f);
	VehicleDriveComp->ApplyBrakeInput(0.35f);
	VehicleDriveComp->ApplyHandbrakeInput(true);

	// [v1.0.0] 입력 초기화 전 잔류값이 실제 기록됐는지 확인하는 상태입니다.
	const FCFVehicleInputState BeforeClearInputState = VehicleDriveComp->CurrentInputState;
	TestEqual(TEXT("초기화 전 Throttle 기록"), BeforeClearInputState.ThrottleInput, 0.75f);
	TestEqual(TEXT("초기화 전 Steering 기록"), BeforeClearInputState.SteeringInput, -0.50f);
	TestEqual(TEXT("초기화 전 Brake 기록"), BeforeClearInputState.BrakeInput, 0.35f);
	TestTrue(TEXT("초기화 전 Handbrake 기록"), BeforeClearInputState.bHandbrakePressed);

	VehicleDriveComp->ClearDriveInputs();

	// [v1.0.0] Pause 진입 후 기대하는 완전 중립 입력 상태입니다.
	const FCFVehicleInputState AfterClearInputState = VehicleDriveComp->CurrentInputState;
	TestEqual(TEXT("Pause 후 Throttle 중립"), AfterClearInputState.ThrottleInput, 0.0f);
	TestEqual(TEXT("Pause 후 Steering 중립"), AfterClearInputState.SteeringInput, 0.0f);
	TestEqual(TEXT("Pause 후 Brake 중립"), AfterClearInputState.BrakeInput, 0.0f);
	TestFalse(TEXT("Pause 후 Handbrake 해제"), AfterClearInputState.bHandbrakePressed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIPauseProgressFreezeTest,
	"CarFight.UI.UI_P0_02.ProgressFreezeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Launcher·Projectile·Motor가 Pause 중 별도 Tick을 허용하지 않고 Projectile 수명이 표준 World Timer를 사용하는지 검증합니다.
bool FCFUIPauseProgressFreezeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Ripple·Salvo 진행을 소유하는 LauncherComp CDO입니다.
	const UCFLauncherComp* LauncherDefaults = GetDefault<UCFLauncherComp>();
	if (!TestNotNull(TEXT("LauncherComp CDO"), LauncherDefaults))
	{
		return false;
	}
	TestFalse(TEXT("LauncherComp는 Pause 중 Tick하지 않음"), LauncherDefaults->PrimaryComponentTick.bTickEvenWhenPaused);

	// [v1.0.0] Projectile Actor와 기본 서브오브젝트 Tick 정책을 확인할 Automation 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Pause 진행 정지 테스트 월드"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Actor·Movement·Motor의 Pause Tick 정책을 확인할 Transient Projectile입니다.
	ACFProjectileActor* ProjectileActor = TestWorld->SpawnActor<ACFProjectileActor>();
	if (!TestNotNull(TEXT("Pause 진행 정지 테스트 Projectile"), ProjectileActor))
	{
		return false;
	}

	UProjectileMovementComponent* ProjectileMovementComp = ProjectileActor->FindComponentByClass<UProjectileMovementComponent>();
	UCFProjectileMotorComp* ProjectileMotorComp = ProjectileActor->FindComponentByClass<UCFProjectileMotorComp>();
	if (!TestNotNull(TEXT("ProjectileMovementComponent"), ProjectileMovementComp)
		|| !TestNotNull(TEXT("ProjectileMotorComp"), ProjectileMotorComp))
	{
		ProjectileActor->Destroy();
		return false;
	}

	TestFalse(TEXT("Projectile Actor는 Pause 중 Tick하지 않음"), ProjectileActor->PrimaryActorTick.bTickEvenWhenPaused);
	TestFalse(TEXT("ProjectileMovement는 Pause 중 Tick하지 않음"), ProjectileMovementComp->PrimaryComponentTick.bTickEvenWhenPaused);
	TestFalse(TEXT("ProjectileMotor는 Pause 중 Tick하지 않음"), ProjectileMotorComp->PrimaryComponentTick.bTickEvenWhenPaused);

		// [v1.0.0] Pause에 종속되는 표준 World TimerManager 계약을 확인할 임시 게임 시간 Timer입니다.
	FTimerHandle GameTimeTimerHandle;
	bool bGameTimeTimerFired = false;
	TestWorld->GetTimerManager().SetTimer(
		GameTimeTimerHandle,
		FTimerDelegate::CreateLambda([&bGameTimeTimerFired]()
		{
			bGameTimeTimerFired = true;
		}),
		1.0f,
		false);
	TestTrue(TEXT("게임 진행 Timer는 World TimerManager에 등록됨"), TestWorld->GetTimerManager().IsTimerActive(GameTimeTimerHandle));
	TestFalse(TEXT("World Tick 전 Timer는 실행되지 않음"), bGameTimeTimerFired);
	TestWorld->GetTimerManager().ClearTimer(GameTimeTimerHandle);

	ProjectileActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
