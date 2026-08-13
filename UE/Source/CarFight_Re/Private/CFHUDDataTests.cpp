// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-13
// Description: CF-FQ-032 UI-P0-03 HUD ViewData/Provider Automation
// Scope: 명시 가용 상태, OnCurrentPawnChanged Provider Rebind와 공통 Weapon/Launcher Presentation lifecycle 계약을 검증합니다.
// Changelog:
// - v1.4.0: Salvo 전용 Hold 회귀를 제거하고 LauncherSequenceRevision 기반 Ripple·Salvo 공통 Active→Terminal→Cooldown/READY lifecycle 및 HeavyCannon SingleCycle 상태 경로를 검증.
// - v1.3.0: [폐기 이력] 같은 프레임에 완료되는 Salvo Hold가 사용할 terminal Snapshot 변환 계약을 추가.
// - v1.2.0: 정상 LauncherSequence를 Alert가 아닌 WeaponPanel 진행 상태로 표현하는 RIPPLE/SALVO 문구·진행률 회귀를 추가하고 구형 AlertSemanticRouting 회귀를 제거.
// - v1.1.0: LauncherSequence AlertKey가 단독/혼합 상태에서 전용 Launcher 슬롯으로 결정되는 semantic routing 회귀를 추가.
// - v1.0.2: Transient ULocalPlayer Outer를 실제 ClassWithin 계약에 맞게 GEngine 아래 생성하도록 교정.
// - v1.0.1: Provider Rebind Fixture의 UCFUISubsystem Outer를 실제 ClassWithin 계약인 ULocalPlayer로 교정.
// - v1.0.0: ViewData Availability와 Provider Rebind Generation/Old Pawn 해제 계약 테스트를 최초 추가.
// Migration:
// - 테스트는 Transient ULocalPlayer/UObject와 Automation World만 사용하며 Unreal Asset을 생성하거나 저장하지 않습니다.
// - FirePattern은 RIPPLE/SALVO 문구 선택에만 사용하며 Presentation 수명 전이는 LauncherSequenceRevision과 Active 상태로 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehiclePawn.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDPresenter.h"
#include "UI/CFHUDViewData.h"
#include "UI/CFUISubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDViewDataAvailabilityTest,
	"CarFight.UI.UI_P0_03.ViewDataAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 기본 ViewData가 실제 Provider 없는 채널을 Known 값으로 위조하지 않는지 검증합니다.
bool FCFHUDViewDataAvailabilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기본 생성 상태의 전체 HUD ViewData입니다.
	const FCFInGameUIViewData ViewData;
	TestEqual(TEXT("Vehicle 기본 상태 Unavailable"), ViewData.Vehicle.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Defense 기본 상태 Unavailable"), ViewData.Defense.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Weapon 기본 상태 Unavailable"), ViewData.Weapon.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Target 기본 상태 Unavailable"), ViewData.Target.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Radar 기본 상태 Unavailable"), ViewData.Radar.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Alert 기본 상태 Unavailable"), ViewData.Alerts.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Engine RPM 기본 상태 Unavailable"), ViewData.Vehicle.EngineRpmAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Gear 기본 상태 Unavailable"), ViewData.Vehicle.GearAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Ammo 기본 상태 Unavailable"), ViewData.Weapon.AmmoAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Heat 기본 상태 Unavailable"), ViewData.Weapon.HeatAvailability, ECFUIViewAvailability::Unavailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDProviderPawnRebindTest,
	"CarFight.UI.UI_P0_03.ProviderPawnRebind",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] UISubsystem OnCurrentPawnChanged가 Provider의 Old Pawn 해제와 새 Pawn Rebind를 정확히 수행하는지 검증합니다.
bool FCFHUDProviderPawnRebindTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 서로 다른 두 차량 Pawn을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("HUD Provider Rebind Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 첫 번째 Provider Source 차량입니다.
	ACFVehiclePawn* FirstVehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	// [v1.0.0] 두 번째 Provider Source 차량입니다.
	ACFVehiclePawn* SecondVehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("첫 번째 HUD Source 차량"), FirstVehiclePawn)
		|| !TestNotNull(TEXT("두 번째 HUD Source 차량"), SecondVehiclePawn))
	{
		return false;
	}

	// [v1.0.2] ULocalPlayer의 Engine ClassWithin과 ULocalPlayerSubsystem의 LocalPlayer ClassWithin을 모두 만족하는 테스트 Outer입니다.
	ULocalPlayer* TestLocalPlayer = GEngine ? NewObject<ULocalPlayer>(GEngine) : nullptr;
	// [v1.0.1] Current Pawn 이벤트만 제공할 Transient UISubsystem입니다.
	UCFUISubsystem* UISubsystem = TestLocalPlayer ? NewObject<UCFUISubsystem>(TestLocalPlayer) : nullptr;
	// [v1.0.0] Rebind 수명 계약을 검증할 Transient HUD Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient LocalPlayer"), TestLocalPlayer)
		|| !TestNotNull(TEXT("Transient UISubsystem"), UISubsystem)
		|| !TestNotNull(TEXT("Transient HUD Data Provider"), DataProvider))
	{
		return false;
	}

	TestTrue(TEXT("HUD Provider 초기화"), DataProvider->InitializeProvider(UISubsystem));
	// [v1.0.0] 초기 Null Source 다음 첫 차량 Rebind 직전 Generation입니다.
	const int32 InitialGeneration = DataProvider->GetBindingGeneration();

	UISubsystem->OnCurrentPawnChanged.Broadcast(nullptr, FirstVehiclePawn);
	TestTrue(TEXT("첫 차량 Rebind"), DataProvider->GetBoundVehiclePawn() == FirstVehiclePawn);
	TestTrue(TEXT("첫 차량 Rebind Generation 증가"), DataProvider->GetBindingGeneration() > InitialGeneration);

	// [v1.0.0] 두 번째 차량 Rebind 직전 Generation입니다.
	const int32 FirstPawnGeneration = DataProvider->GetBindingGeneration();
	UISubsystem->OnCurrentPawnChanged.Broadcast(FirstVehiclePawn, SecondVehiclePawn);
	TestTrue(TEXT("두 번째 차량 Rebind"), DataProvider->GetBoundVehiclePawn() == SecondVehiclePawn);
	TestTrue(TEXT("두 번째 차량 Rebind Generation 증가"), DataProvider->GetBindingGeneration() > FirstPawnGeneration);

	UISubsystem->OnCurrentPawnChanged.Broadcast(SecondVehiclePawn, nullptr);
	TestNull(TEXT("Pawn 해제 후 Provider Source 없음"), DataProvider->GetBoundVehiclePawn());
	TestEqual(TEXT("Pawn 해제 후 Vehicle ViewData Unavailable"), DataProvider->GetCurrentViewData().Vehicle.Availability, ECFUIViewAvailability::Unavailable);

	DataProvider->ShutdownProvider();
	FirstVehiclePawn->Destroy();
	SecondVehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDWeaponLauncherPresentationTest,
	"CarFight.UI.UI_P0_03.WeaponLauncherSequencePresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.4.0] HeavyCannon SingleCycle, Ripple과 Salvo가 무기별 예외 없이 공통 Presentation lifecycle을 사용하는지 검증합니다.
bool FCFHUDWeaponLauncherPresentationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] 상태를 실제 Presenter 인스턴스처럼 순차적으로 보존할 Transient Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient HUD Presenter"), Presenter))
	{
		return false;
	}

	// [v1.4.0] 공통 Launcher Sequence 테스트에 사용할 WeaponId입니다.
	const FName LauncherWeaponId(TEXT("LauncherWeapon"));

	// [v1.4.0] Ripple 4발 중 실제 승인 2발 상태를 재현하는 Active ViewData입니다.
	FCFWeaponHUDData RippleActiveViewData;
	RippleActiveViewData.WeaponId = LauncherWeaponId;
	RippleActiveViewData.LauncherAvailability = ECFUIViewAvailability::Known;
	RippleActiveViewData.LauncherSequenceRevision = 10;
	RippleActiveViewData.bLauncherSequenceActive = true;
	RippleActiveViewData.LauncherPattern = ECFLauncherFirePattern::Ripple;
	RippleActiveViewData.LauncherTotalProjectileCount = 4;
	RippleActiveViewData.LauncherAcceptedProjectileCount = 2;
	RippleActiveViewData.LauncherRemainingProjectileCount = 2;

	// [v1.4.0] Presenter가 만들어야 하는 Player-facing Sequence 문구입니다.
	FText SequenceText;
	// [v1.4.0] Presenter가 만들어야 하는 0~1 진행률입니다.
	float SequenceProgress = 0.0f;
	TestTrue(
		TEXT("Active Ripple은 공통 Launcher lifecycle로 표시"),
		Presenter->ResolveLauncherSequenceDisplay(RippleActiveViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Ripple 표시 문구"), SequenceText.ToString(), FString(TEXT("RIPPLE 2 / 4")));
	TestTrue(TEXT("Ripple 진행률 0.5"), FMath::IsNearlyEqual(SequenceProgress, 0.5f));

	// [v1.4.0] Runtime은 terminal이지만 정식 Launcher 이벤트 Revision은 아직 증가하지 않은 Ammo 부수 Refresh입니다.
	FCFWeaponHUDData RipplePreTerminalRefresh = RippleActiveViewData;
	RipplePreTerminalRefresh.LauncherAvailability = ECFUIViewAvailability::KnownZero;
	RipplePreTerminalRefresh.bLauncherSequenceActive = false;
	RipplePreTerminalRefresh.LauncherAcceptedProjectileCount = 4;
	RipplePreTerminalRefresh.LauncherRemainingProjectileCount = 0;
	TestTrue(
		TEXT("Ripple terminal 전 Ammo Refresh는 마지막 Active Presentation 유지"),
		Presenter->ResolveLauncherSequenceDisplay(RipplePreTerminalRefresh, SequenceText, SequenceProgress));
	TestEqual(TEXT("정식 terminal 이벤트 전 Ripple 문구 유지"), SequenceText.ToString(), FString(TEXT("RIPPLE 2 / 4")));

	// [v1.4.0] 실제 Launcher Completed 이벤트가 Revision을 증가시킨 최종 Snapshot입니다.
	FCFWeaponHUDData RippleTerminalViewData = RipplePreTerminalRefresh;
	RippleTerminalViewData.LauncherSequenceRevision = 11;
	TestTrue(
		TEXT("Ripple terminal 이벤트는 최종 Snapshot 1회 표시"),
		Presenter->ResolveLauncherSequenceDisplay(RippleTerminalViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Ripple terminal 문구"), SequenceText.ToString(), FString(TEXT("RIPPLE 4 / 4")));
	TestTrue(TEXT("Ripple terminal 진행률 1.0"), FMath::IsNearlyEqual(SequenceProgress, 1.0f));
	TestFalse(
		TEXT("Ripple terminal 다음 ViewData는 Weapon Status로 전환"),
		Presenter->ResolveLauncherSequenceDisplay(RippleTerminalViewData, SequenceText, SequenceProgress));

	// [v1.4.0] 같은 lifecycle을 Salvo에 그대로 적용하고 FirePattern은 문구만 SALVO로 바꿉니다.
	FCFWeaponHUDData SalvoActiveViewData = RippleActiveViewData;
	SalvoActiveViewData.LauncherSequenceRevision = 20;
	SalvoActiveViewData.LauncherPattern = ECFLauncherFirePattern::Salvo;
	SalvoActiveViewData.LauncherAcceptedProjectileCount = 3;
	SalvoActiveViewData.LauncherRemainingProjectileCount = 1;
	TestTrue(
		TEXT("Active Salvo도 Ripple과 동일 lifecycle로 표시"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoActiveViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Salvo 표시 문구"), SequenceText.ToString(), FString(TEXT("SALVO 3 / 4")));
	TestTrue(TEXT("Salvo 진행률 0.75"), FMath::IsNearlyEqual(SequenceProgress, 0.75f));

	// [v1.4.0] Salvo도 Revision이 바뀌기 전 부수 Refresh에서는 마지막 Active Presentation을 그대로 유지합니다.
	FCFWeaponHUDData SalvoPreTerminalRefresh = SalvoActiveViewData;
	SalvoPreTerminalRefresh.LauncherAvailability = ECFUIViewAvailability::KnownZero;
	SalvoPreTerminalRefresh.bLauncherSequenceActive = false;
	SalvoPreTerminalRefresh.LauncherAcceptedProjectileCount = 4;
	SalvoPreTerminalRefresh.LauncherRemainingProjectileCount = 0;
	TestTrue(
		TEXT("Salvo terminal 전 Ammo Refresh도 마지막 Active Presentation 유지"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoPreTerminalRefresh, SequenceText, SequenceProgress));
	TestEqual(TEXT("정식 terminal 이벤트 전 Salvo 문구 유지"), SequenceText.ToString(), FString(TEXT("SALVO 3 / 4")));

	FCFWeaponHUDData SalvoTerminalViewData = SalvoPreTerminalRefresh;
	SalvoTerminalViewData.LauncherSequenceRevision = 21;
	TestTrue(
		TEXT("Salvo terminal 이벤트도 공통 최종 Snapshot 1회 표시"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoTerminalViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Salvo terminal 문구"), SequenceText.ToString(), FString(TEXT("SALVO 4 / 4")));
	TestTrue(TEXT("Salvo terminal 진행률 1.0"), FMath::IsNearlyEqual(SequenceProgress, 1.0f));
	TestFalse(
		TEXT("Salvo terminal 다음 ViewData도 Weapon Status로 전환"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoTerminalViewData, SequenceText, SequenceProgress));

	// [v1.4.0] HeavyCannon SingleCycle은 Launcher Sequence를 만들지 않고 같은 Weapon Status 경로에서 Cooldown을 표시합니다.
	FCFWeaponHUDData HeavyCannonViewData;
	HeavyCannonViewData.WeaponId = FName(TEXT("HeavyCannon"));
	HeavyCannonViewData.LauncherAvailability = ECFUIViewAvailability::KnownZero;
	HeavyCannonViewData.LauncherPattern = ECFLauncherFirePattern::SingleCycle;
	HeavyCannonViewData.bLauncherSequenceActive = false;
	HeavyCannonViewData.CooldownAvailability = ECFUIViewAvailability::Known;
	HeavyCannonViewData.CooldownDurationSeconds = 2.0f;
	HeavyCannonViewData.RemainingCooldownSeconds = 1.5f;
	TestFalse(
		TEXT("HeavyCannon SingleCycle은 별도 Launcher Presentation 예외를 만들지 않음"),
		Presenter->ResolveLauncherSequenceDisplay(HeavyCannonViewData, SequenceText, SequenceProgress));

	// [v1.4.0] HeavyCannon과 Launcher terminal 이후가 함께 사용하는 공통 Weapon Status 출력입니다.
	FText WeaponStatusText;
	float WeaponStatusProgress = 0.0f;
	TestTrue(
		TEXT("HeavyCannon Cooldown은 공통 Weapon Status로 표시"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			HeavyCannonViewData,
			false,
			WeaponStatusText,
			WeaponStatusProgress));
	TestEqual(TEXT("HeavyCannon Cooldown 문구"), WeaponStatusText.ToString(), FString(TEXT("1.5 s")));
	TestTrue(TEXT("HeavyCannon Cooldown 진행률 0.25"), FMath::IsNearlyEqual(WeaponStatusProgress, 0.25f));

	HeavyCannonViewData.RemainingCooldownSeconds = 0.0f;
	HeavyCannonViewData.CooldownAvailability = ECFUIViewAvailability::KnownZero;
	TestTrue(
		TEXT("HeavyCannon Cooldown 종료는 같은 Weapon Status에서 READY"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			HeavyCannonViewData,
			false,
			WeaponStatusText,
			WeaponStatusProgress));
	TestEqual(TEXT("HeavyCannon READY 문구"), WeaponStatusText.ToString(), FString(TEXT("READY")));
	TestTrue(TEXT("HeavyCannon READY 진행률 1.0"), FMath::IsNearlyEqual(WeaponStatusProgress, 1.0f));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS