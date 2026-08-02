// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-28
// Description: CF-FQ-029 LM-P0-03 런처 발사 패턴 데이터 계약 자동화 테스트
// Scope: SingleCycle·Ripple·Salvo 기본값, 유효값 보정, 실패·쿨다운 정책과 WeaponData 요약을 검증합니다.
// Changelog:
// - v1.0.0: Launcher Fire Pattern RuntimeContract 최초 추가.
// Migration:
// - 이 테스트는 발사 스케줄러를 실행하지 않으며 LM-P0-03 데이터 계약만 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFLauncherTypes.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFLauncherFirePatternContractTest,
	"CarFight.Launcher.LM_P0_03.FirePatternContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 런처 발사 패턴 기본값과 안전 보정·정책 보존 계약을 검증합니다.
bool FCFLauncherFirePatternContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기존 WeaponData가 자동으로 사용하는 단발 호환 기본 설정입니다.
	const FCFLauncherFirePatternConfig DefaultConfig;
	TestEqual(TEXT("기본 패턴은 SingleCycle"), DefaultConfig.FirePattern, ECFLauncherFirePattern::SingleCycle);
	TestEqual(TEXT("SingleCycle 유효 발사 수는 항상 1"), DefaultConfig.GetEffectiveProjectileCount(), 1);
	TestEqual(TEXT("SingleCycle 유효 Ripple 간격은 0"), DefaultConfig.GetEffectiveInterMuzzleDelaySeconds(), 0.0f);
	TestEqual(TEXT("SingleCycle 유효 동시 발사 한도는 1"), DefaultConfig.GetEffectiveMaximumSimultaneousLaunchCount(), 1);
	TestEqual(TEXT("기본 실패 정책은 남은 발사 계속"), DefaultConfig.SequenceFailurePolicy, ECFLauncherSequenceFailurePolicy::ContinueRemaining);
	TestEqual(TEXT("기본 쿨다운은 첫 승인 발사에서 시작"), DefaultConfig.CooldownStartPolicy, ECFLauncherCooldownStartPolicy::FirstAcceptedProjectile);

	// [v1.0.0] Ripple 수량과 간격의 정상 입력을 검증할 설정입니다.
	FCFLauncherFirePatternConfig RippleConfig;
	RippleConfig.FirePattern = ECFLauncherFirePattern::Ripple;
	RippleConfig.ProjectileCountPerTrigger = 4;
	RippleConfig.InterMuzzleDelaySeconds = 0.12f;
	RippleConfig.MaximumSimultaneousLaunchCount = 8;
	RippleConfig.SequenceFailurePolicy = ECFLauncherSequenceFailurePolicy::StopSequence;
	RippleConfig.CooldownStartPolicy = ECFLauncherCooldownStartPolicy::SequenceCompleted;
	TestEqual(TEXT("Ripple 유효 발사 수 4"), RippleConfig.GetEffectiveProjectileCount(), 4);
	TestEqual(TEXT("Ripple 유효 간격 0.12초"), RippleConfig.GetEffectiveInterMuzzleDelaySeconds(), 0.12f);
	TestEqual(TEXT("Ripple 동시 발사 한도는 1"), RippleConfig.GetEffectiveMaximumSimultaneousLaunchCount(), 1);
	TestEqual(TEXT("Ripple 실패 정책 보존"), RippleConfig.SequenceFailurePolicy, ECFLauncherSequenceFailurePolicy::StopSequence);
	TestEqual(TEXT("Ripple 쿨다운 정책 보존"), RippleConfig.CooldownStartPolicy, ECFLauncherCooldownStartPolicy::SequenceCompleted);

	// [v1.0.0] Salvo 동시 처리 한도가 총 발사 수를 초과하지 않는지 검증할 설정입니다.
	FCFLauncherFirePatternConfig SalvoConfig;
	SalvoConfig.FirePattern = ECFLauncherFirePattern::Salvo;
	SalvoConfig.ProjectileCountPerTrigger = 6;
	SalvoConfig.MaximumSimultaneousLaunchCount = 12;
	TestEqual(TEXT("Salvo 유효 발사 수 6"), SalvoConfig.GetEffectiveProjectileCount(), 6);
	TestEqual(TEXT("Salvo 유효 Ripple 간격은 0"), SalvoConfig.GetEffectiveInterMuzzleDelaySeconds(), 0.0f);
	TestEqual(TEXT("Salvo 동시 처리 한도는 발사 수 6으로 제한"), SalvoConfig.GetEffectiveMaximumSimultaneousLaunchCount(), 6);

	// [v1.0.0] 잘못된 수량·간격 입력을 안전 범위로 보정하는지 검증할 설정입니다.
	FCFLauncherFirePatternConfig InvalidConfig;
	InvalidConfig.FirePattern = ECFLauncherFirePattern::Ripple;
	InvalidConfig.ProjectileCountPerTrigger = -5;
	InvalidConfig.InterMuzzleDelaySeconds = -3.0f;
	InvalidConfig.MaximumSimultaneousLaunchCount = 0;
	TestEqual(TEXT("음수 발사 수는 1로 보정"), InvalidConfig.GetEffectiveProjectileCount(), 1);
	TestEqual(TEXT("음수 Ripple 간격은 0으로 보정"), InvalidConfig.GetEffectiveInterMuzzleDelaySeconds(), 0.0f);
	TestEqual(TEXT("Ripple 동시 처리 한도는 1"), InvalidConfig.GetEffectiveMaximumSimultaneousLaunchCount(), 1);

	// [v1.0.0] WeaponData Getter가 원본을 변경하지 않고 안전한 복사본을 반환하는지 검증할 임시 DataAsset입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>();
	if (!TestNotNull(TEXT("테스트 WeaponData 생성"), WeaponData))
	{
		return false;
	}

	TestEqual(TEXT("기본 WeaponData 패턴은 SingleCycle"), WeaponData->GetEffectiveLauncherFirePatternConfig().FirePattern, ECFLauncherFirePattern::SingleCycle);
	TestEqual(TEXT("기본 WeaponData 유효 발사 수 1"), WeaponData->GetEffectiveLauncherFirePatternConfig().ProjectileCountPerTrigger, 1);

	WeaponData->LauncherFirePatternConfig = SalvoConfig;
	const FCFLauncherFirePatternConfig EffectiveWeaponConfig = WeaponData->GetEffectiveLauncherFirePatternConfig();
	TestEqual(TEXT("WeaponData Salvo 유효 발사 수 6"), EffectiveWeaponConfig.ProjectileCountPerTrigger, 6);
	TestEqual(TEXT("WeaponData Salvo 유효 동시 처리 한도 6"), EffectiveWeaponConfig.MaximumSimultaneousLaunchCount, 6);
	TestEqual(TEXT("원본 Salvo 동시 처리 값은 자동 변경하지 않음"), WeaponData->LauncherFirePatternConfig.MaximumSimultaneousLaunchCount, 12);
	TestTrue(TEXT("전용 요약에 Salvo 패턴 포함"), WeaponData->BuildLauncherFirePatternSummary().Contains(TEXT("Salvo")));
	TestTrue(TEXT("통합 WeaponData 요약에 LauncherProjectiles=6 포함"), WeaponData->BuildWeaponSummary().Contains(TEXT("LauncherProjectiles=6")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
