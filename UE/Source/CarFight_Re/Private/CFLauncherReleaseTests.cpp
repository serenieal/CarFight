// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-29
// Description: CF-FQ-029 LM-P0-04 런처 Release 계산 자동화 테스트
// Scope: Direct 호환, Angled·Vertical 방향, 사출 속력 fallback, 차량 속도 상속과 WeaponData 유효값 계약을 검증합니다.
// Changelog:
// - v1.0.0: Launcher Release RuntimeContract 최초 추가.
// Migration:
// - 실제 Muzzle 소켓 방향, 차량 이동과 Projectile 궤적은 Editor PIE 대상으로 남기고 이 테스트는 결정적 데이터·벡터 계산을 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFLauncherTypes.h"
#include "CFProjectileData.h"
#include "CFWeaponData.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFLauncherReleaseContractTest,
	"CarFight.Launcher.LM_P0_04.ReleaseContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Release Mode별 방향·속도와 기존 WeaponData 호환 계약을 검증합니다.
bool FCFLauncherReleaseContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기존 WeaponData가 사용하는 Direct / 차량 속도 미상속 기본 설정입니다.
	const FCFLauncherReleaseConfig DefaultConfig;
	const FVector DirectAimDirection = FVector(1.0f, 1.0f, 0.0f).GetSafeNormal();
	const FTransform IdentityMuzzleTransform = FTransform::Identity;
	TestEqual(TEXT("기본 Release Mode는 Direct"), DefaultConfig.ReleaseMode, ECFProjectileReleaseMode::Direct);
	TestTrue(TEXT("Direct 방향은 AimDirection 유지"), DefaultConfig.ResolveInitialLaunchDirection(IdentityMuzzleTransform, DirectAimDirection).Equals(DirectAimDirection, KINDA_SMALL_NUMBER));
	TestEqual(TEXT("Direct 속력은 Projectile InitialSpeed 유지"), DefaultConfig.GetEffectiveReleaseSpeed(3210.0f), 3210.0f);
	TestEqual(TEXT("기본 차량 속도 상속 비율 0"), DefaultConfig.GetEffectiveCarrierVelocityRatio(), 0.0f);
	TestTrue(TEXT("기본 차량 속도 상속 성분 Zero"), DefaultConfig.ResolveInheritedCarrierVelocity(FVector(1000.0f, 200.0f, 0.0f)).IsNearlyZero());

	// [v1.0.0] Muzzle 로컬 대각 방향을 월드로 변환하고 차량 속도를 절반 상속할 설정입니다.
	FCFLauncherReleaseConfig AngledConfig;
	AngledConfig.ReleaseMode = ECFProjectileReleaseMode::AngledEjection;
	AngledConfig.LocalEjectionDirection = FVector(1.0f, 0.0f, 1.0f);
	AngledConfig.EjectionSpeed = 1200.0f;
	AngledConfig.CarrierVelocityRatio = 0.5f;
	AngledConfig.LauncherClearanceTraceDistanceCm = 250.0f;

	const FTransform AngledMuzzleTransform(FRotator(0.0f, 90.0f, 0.0f));
	const FVector ExpectedAngledDirection = AngledMuzzleTransform
		.TransformVectorNoScale(AngledConfig.LocalEjectionDirection.GetSafeNormal())
		.GetSafeNormal();
	const FVector ResolvedAngledDirection = AngledConfig.ResolveInitialLaunchDirection(AngledMuzzleTransform, DirectAimDirection);
	TestTrue(TEXT("Angled 방향은 Muzzle Transform 기준"), ResolvedAngledDirection.Equals(ExpectedAngledDirection, KINDA_SMALL_NUMBER));
	TestEqual(TEXT("Angled 사출 속력 설정값 사용"), AngledConfig.GetEffectiveReleaseSpeed(3000.0f), 1200.0f);
	TestEqual(TEXT("Angled 안전 검사 거리 보존"), AngledConfig.GetEffectiveLauncherClearanceTraceDistanceCm(), 250.0f);

	const FVector CarrierWorldVelocity(1000.0f, 200.0f, 0.0f);
	const FVector ExpectedInheritedVelocity = CarrierWorldVelocity * 0.5f;
	const FVector ExpectedAngledInitialVelocity = ResolvedAngledDirection * 1200.0f + ExpectedInheritedVelocity;
	TestTrue(TEXT("차량 속도 상속 성분 계산"), AngledConfig.ResolveInheritedCarrierVelocity(CarrierWorldVelocity).Equals(ExpectedInheritedVelocity, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("사출 Velocity와 차량 Velocity 합산"), AngledConfig.ResolveInitialLaunchVelocity(ResolvedAngledDirection, 3000.0f, CarrierWorldVelocity).Equals(ExpectedAngledInitialVelocity, KINDA_SMALL_NUMBER));

	// [v1.0.0] Muzzle +X축이 World Up을 향하는 수직 발사관 Transform입니다.
	const FQuat VerticalMuzzleRotation = FQuat::FindBetweenNormals(FVector::ForwardVector, FVector::UpVector);
	const FTransform VerticalMuzzleTransform(VerticalMuzzleRotation, FVector::ZeroVector);
	FCFLauncherReleaseConfig VerticalConfig;
	VerticalConfig.ReleaseMode = ECFProjectileReleaseMode::VerticalEjection;
	VerticalConfig.EjectionSpeed = 900.0f;
	const FVector ResolvedVerticalDirection = VerticalConfig.ResolveInitialLaunchDirection(VerticalMuzzleTransform, DirectAimDirection);
	TestTrue(TEXT("Vertical 방향은 Muzzle X축"), ResolvedVerticalDirection.Equals(FVector::UpVector, KINDA_SMALL_NUMBER));
	TestFalse(TEXT("Vertical 방향은 Command Aim과 독립"), ResolvedVerticalDirection.Equals(DirectAimDirection, KINDA_SMALL_NUMBER));

	// [v1.0.0] 잘못된 설정이 안전한 런타임 값으로 복구되는지 검증합니다.
	FCFLauncherReleaseConfig InvalidConfig;
	InvalidConfig.ReleaseMode = ECFProjectileReleaseMode::AngledEjection;
	InvalidConfig.LocalEjectionDirection = FVector::ZeroVector;
	InvalidConfig.EjectionSpeed = 0.0f;
	InvalidConfig.CarrierVelocityRatio = -1.0f;
	InvalidConfig.LauncherClearanceTraceDistanceCm = -10.0f;
	TestTrue(TEXT("0 로컬 방향은 +X fallback"), InvalidConfig.GetEffectiveLocalEjectionDirection().Equals(FVector::ForwardVector));
	TestEqual(TEXT("0 EjectionSpeed는 Projectile InitialSpeed fallback"), InvalidConfig.GetEffectiveReleaseSpeed(2500.0f), 2500.0f);
	TestEqual(TEXT("음수 차량 속도 상속은 0"), InvalidConfig.GetEffectiveCarrierVelocityRatio(), 0.0f);
	TestEqual(TEXT("음수 안전 검사 거리는 0"), InvalidConfig.GetEffectiveLauncherClearanceTraceDistanceCm(), 0.0f);

	// [v1.0.0] WeaponData 기본값과 Projectile 전환 후 유효 Release 설정을 검증합니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>();
	if (!TestNotNull(TEXT("WeaponData 생성"), WeaponData))
	{
		return false;
	}

	const FCFLauncherReleaseConfig DefaultWeaponRelease = WeaponData->GetEffectiveLauncherReleaseConfig();
	TestEqual(TEXT("기본 HitScan WeaponData는 Direct"), DefaultWeaponRelease.ReleaseMode, ECFProjectileReleaseMode::Direct);
	TestEqual(TEXT("기본 HitScan 차량 속도 상속 0"), DefaultWeaponRelease.CarrierVelocityRatio, 0.0f);

	UCFProjectileData* ProjectileData = NewObject<UCFProjectileData>(WeaponData);
	if (!TestNotNull(TEXT("ProjectileData 생성"), ProjectileData))
	{
		return false;
	}
	ProjectileData->InitialSpeed = 3000.0f;
	WeaponData->FireMode = ECFWeaponFireMode::Projectile;
	WeaponData->DefaultProjectileData = ProjectileData;
	WeaponData->LauncherReleaseConfig = InvalidConfig;

	const FCFLauncherReleaseConfig EffectiveProjectileRelease = WeaponData->GetEffectiveLauncherReleaseConfig();
	TestEqual(TEXT("Projectile WeaponData Angled Mode 보존"), EffectiveProjectileRelease.ReleaseMode, ECFProjectileReleaseMode::AngledEjection);
	TestEqual(TEXT("EjectionSpeed 0은 ProjectileData InitialSpeed 사용"), EffectiveProjectileRelease.EjectionSpeed, 3000.0f);
	TestTrue(TEXT("WeaponData 로컬 방향 정규화"), EffectiveProjectileRelease.LocalEjectionDirection.Equals(FVector::ForwardVector));
	TestTrue(TEXT("Release 전용 요약 생성"), WeaponData->BuildLauncherReleaseSummary().Contains(TEXT("AngledEjection")));
	TestTrue(TEXT("Weapon 통합 요약에 Release Mode 포함"), WeaponData->BuildWeaponSummary().Contains(TEXT("ReleaseMode=")));

	WeaponData->FireMode = ECFWeaponFireMode::HitScan;
	WeaponData->LauncherReleaseConfig.ReleaseMode = ECFProjectileReleaseMode::VerticalEjection;
	WeaponData->LauncherReleaseConfig.CarrierVelocityRatio = 1.0f;
	const FCFLauncherReleaseConfig HitScanRelease = WeaponData->GetEffectiveLauncherReleaseConfig();
	TestEqual(TEXT("HitScan은 저장된 Vertical 값을 Direct로 보정"), HitScanRelease.ReleaseMode, ECFProjectileReleaseMode::Direct);
	TestEqual(TEXT("HitScan은 차량 속도 상속을 0으로 보정"), HitScanRelease.CarrierVelocityRatio, 0.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
