// File: CFVehicleCameraFXTests.cpp
// Version: v1.3.1
// Date: 2026-10-02
// Description: VCFX normalized contract와 P0-03 Camera collision recovery seed까지 기존 exact test set에서 검증합니다.
// Changelog:
// - v1.3.1: test count를 늘리지 않고 FrozenDefaults에 collision Clear hold/padding/recovery interpolation seed 검증을 추가.
// - v1.3.0: 기존 0~1 SpeedRatio parity, explicit 1.5 overspeed ratio/phase, overspeed CameraData seed와 AccelerationRearKickScale을 검증.
// - v1.2.0: 실제 차체 Motion intensity, signed free-look ViewAlignment, 최종 Lateral Presentation scale pure math 검증 추가.
// - v1.1.1: 비율 나눗셈의 정상적인 float 반올림을 허용하도록 normalized ratio assertion tolerance를 1e-4로 명시.
// - v1.1.0: 런타임과 동일한 Private pure helper를 사용해 차량별 25/50/75/100% SpeedRatio, Accel/Brake/Lateral rate, clamp, invalid reference/input exact0을 직접 검증.
// - v1.0.0: normalized 기본 OFF, Curve Seed, Clamp/Attenuation Seed, Aim Presentation 기본 OFF를 검증.
// Migration:
// - Product Asset을 수정하지 않는 source-only focused Automation입니다.
// - Public Runtime/Blueprint debug surface를 추가하지 않고 production normalized math authority를 직접 호출합니다.

#include "Misc/AutomationTest.h"
#include "CFVehicleCameraData.h"
#include "CFVehicleCameraMath.h"
#include "CFVehicleCameraTypes.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCameraFXDefaultsTest,
	"CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_02.FrozenDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 동결된 DrivingFX 기본값과 legacy-disabled parity gate를 검증합니다.
bool FCFVehicleCameraFXDefaultsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// C++ 기본 Camera tuning에서 신규 DrivingFX contract를 읽습니다.
	const FCFVehicleCameraTuningConfig TuningConfig;

	// 검증 대상 normalized DrivingFX 설정입니다.
	const FCFVehicleDrivingFXConfig& DrivingFXConfig = TuningConfig.DrivingFXConfig;

	TestFalse(TEXT("Normalized Driving FX defaults disabled for legacy parity"), DrivingFXConfig.bUseNormalizedDrivingFX);
	TestTrue(TEXT("Max speed presentation ratio seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxSpeedPresentationRatio, 1.5f));
	TestTrue(TEXT("Max overspeed FOV bonus seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxOverspeedFOVBonusDeg, 4.0f));
	TestTrue(TEXT("Max overspeed Arm bonus seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxOverspeedArmBonusCm, 30.0f));
	TestTrue(TEXT("Acceleration rear kick scale seed"), FMath::IsNearlyEqual(DrivingFXConfig.AccelerationRearKickScale, 1.25f));
	TestTrue(TEXT("Motion interp speed seed"), FMath::IsNearlyEqual(DrivingFXConfig.MotionRateInterpSpeed, 10.0f));
	TestTrue(TEXT("Lateral body motion interp seed"), FMath::IsNearlyEqual(DrivingFXConfig.LateralBodyMotionInterpSpeed, 6.0f));
	TestTrue(TEXT("Lateral body roll angle full-intensity seed"), FMath::IsNearlyEqual(DrivingFXConfig.LateralBodyRollAngleForFullIntensityDeg, 7.5f));
	TestTrue(TEXT("Lateral body roll rate full-intensity seed"), FMath::IsNearlyEqual(DrivingFXConfig.LateralBodyRollRateForFullIntensityDegPerSec, 45.0f));
	TestTrue(TEXT("Lateral body yaw rate full-intensity seed"), FMath::IsNearlyEqual(DrivingFXConfig.LateralBodyYawRateForFullIntensityDegPerSec, 90.0f));
	TestTrue(TEXT("Max speed FOV clamp seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxSpeedFOVOffsetDeg, 12.0f));
	TestTrue(TEXT("Max speed Arm clamp seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxSpeedArmOffsetCm, 120.0f));
	TestTrue(TEXT("Max accel kick seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxAccelerationArmKickCm, 35.0f));
	TestTrue(TEXT("Max brake kick seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxBrakingArmKickCm, 35.0f));
	TestTrue(TEXT("Max lateral roll seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxLateralRollDeg, 3.0f));
	TestTrue(TEXT("Presentation FOV min seed"), FMath::IsNearlyEqual(DrivingFXConfig.MinPresentationFOVDeg, 60.0f));
	TestTrue(TEXT("Presentation FOV max seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxPresentationFOVDeg, 120.0f));
	TestTrue(TEXT("Presentation Arm max seed"), FMath::IsNearlyEqual(DrivingFXConfig.MaxPresentationArmLengthCm, 900.0f));
	TestTrue(TEXT("Collision release hold seed"), FMath::IsNearlyEqual(TuningConfig.CollisionReleaseHoldTimeSec, 0.10f));
	TestTrue(TEXT("Collision release probe padding seed"), FMath::IsNearlyEqual(TuningConfig.CollisionReleaseProbePaddingCm, 4.0f));
	TestTrue(TEXT("Collision recovery interp seed"), FMath::IsNearlyEqual(TuningConfig.CollisionRecoveryInterpSpeed, 6.0f));

	// Speed FOV Curve Seed입니다.
	const FRichCurve* SpeedFOVCurve = DrivingFXConfig.SpeedFOVOffsetCurve.GetRichCurveConst();

	// Speed Arm Curve Seed입니다.
	const FRichCurve* SpeedArmCurve = DrivingFXConfig.SpeedArmOffsetCurve.GetRichCurveConst();

	// Acceleration Kick Curve Seed입니다.
	const FRichCurve* AccelCurve = DrivingFXConfig.AccelerationArmKickCurve.GetRichCurveConst();

	// Braking Kick Curve Seed입니다.
	const FRichCurve* BrakeCurve = DrivingFXConfig.BrakingArmKickCurve.GetRichCurveConst();

	// Lateral Roll Curve Seed입니다.
	const FRichCurve* LateralCurve = DrivingFXConfig.LateralRollCurve.GetRichCurveConst();

	if (TestNotNull(TEXT("Speed FOV curve exists"), SpeedFOVCurve))
	{
		TestTrue(TEXT("Speed FOV 50 percent seed"), FMath::IsNearlyEqual(SpeedFOVCurve->Eval(0.50f), 1.0f));
		TestTrue(TEXT("Speed FOV 100 percent seed"), FMath::IsNearlyEqual(SpeedFOVCurve->Eval(1.00f), 6.0f));
	}
	if (TestNotNull(TEXT("Speed Arm curve exists"), SpeedArmCurve))
	{
		TestTrue(TEXT("Speed Arm 50 percent seed"), FMath::IsNearlyEqual(SpeedArmCurve->Eval(0.50f), 10.0f));
		TestTrue(TEXT("Speed Arm 100 percent seed"), FMath::IsNearlyEqual(SpeedArmCurve->Eval(1.00f), 45.0f));
	}
	if (TestNotNull(TEXT("Acceleration curve exists"), AccelCurve))
	{
		TestTrue(TEXT("Acceleration 30 percent seed"), FMath::IsNearlyEqual(AccelCurve->Eval(0.30f), 20.0f));
	}
	if (TestNotNull(TEXT("Braking curve exists"), BrakeCurve))
	{
		TestTrue(TEXT("Braking 30 percent seed"), FMath::IsNearlyEqual(BrakeCurve->Eval(0.30f), 18.0f));
	}
	if (TestNotNull(TEXT("Lateral curve exists"), LateralCurve))
	{
		TestTrue(TEXT("Lateral negative full seed"), FMath::IsNearlyEqual(LateralCurve->Eval(-1.00f), -2.5f));
		TestTrue(TEXT("Lateral positive full seed"), FMath::IsNearlyEqual(LateralCurve->Eval(1.00f), 2.5f));
	}

	TestTrue(TEXT("Combat speed attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.CombatSpeedFXScale, 0.50f));
	TestTrue(TEXT("Combat motion attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.CombatMotionFXScale, 0.25f));
	TestTrue(TEXT("Aim speed attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.AimSpeedFXScale, 0.50f));
	TestTrue(TEXT("Aim motion attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.AimMotionFXScale, 0.20f));
	TestTrue(TEXT("Airborne speed attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.AirborneSpeedFXScale, 0.25f));
	TestTrue(TEXT("Airborne motion disabled seed"), FMath::IsNearlyZero(DrivingFXConfig.AirborneMotionFXScale));
	TestTrue(TEXT("Reverse speed attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.ReverseSpeedFXScale, 0.50f));
	TestTrue(TEXT("Reverse motion attenuation seed"), FMath::IsNearlyEqual(DrivingFXConfig.ReverseMotionFXScale, 0.35f));

	// Aim state는 explicit producer가 연결하기 전 false이며 AimProfile에서 추론하지 않습니다.
	const FCFVehicleCameraModeFlags DefaultModeFlags;
	TestFalse(TEXT("Aim presentation modifier defaults false"), DefaultModeFlags.bAimPresentation);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCameraFXMathTest,
	"CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_02.NormalizedMotionMath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Production normalized motion helper의 차량별 ratio와 invalid-input fail-safe를 직접 검증합니다.
bool FCFVehicleCameraFXMathTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 100 km/h를 Unreal velocity 단위(cm/s)로 환산한 차량 A ReferenceSpeed입니다.
	const float VehicleAReferenceSpeedCmPerSec = 100.0f / 0.036f;

	// 200 km/h를 Unreal velocity 단위(cm/s)로 환산한 차량 B ReferenceSpeed입니다.
	const float VehicleBReferenceSpeedCmPerSec = 200.0f / 0.036f;

	// invalid finite 검증에 사용할 quiet NaN입니다.
	const float QuietNaN = std::numeric_limits<float>::quiet_NaN();

	// invalid finite 검증에 사용할 positive infinity입니다.
	const float PositiveInfinity = std::numeric_limits<float>::infinity();

	// normalized ratio 계산의 정상적인 float division round-off를 허용하는 검증 오차입니다.
	constexpr float RatioTolerance = 0.0001f;

	TestTrue(TEXT("100 km/h reference is valid"), CFVehicleCameraMath::IsValidReferenceSpeed(VehicleAReferenceSpeedCmPerSec));
	TestTrue(TEXT("200 km/h reference is valid"), CFVehicleCameraMath::IsValidReferenceSpeed(VehicleBReferenceSpeedCmPerSec));

	TestTrue(TEXT("Vehicle A 25 percent speed"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleAReferenceSpeedCmPerSec * 0.25f, VehicleAReferenceSpeedCmPerSec),
		0.25f,
		RatioTolerance));
	TestTrue(TEXT("Vehicle A 50 percent speed"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleAReferenceSpeedCmPerSec * 0.50f, VehicleAReferenceSpeedCmPerSec),
		0.50f,
		RatioTolerance));
	TestTrue(TEXT("Vehicle A 75 percent speed"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleAReferenceSpeedCmPerSec * 0.75f, VehicleAReferenceSpeedCmPerSec),
		0.75f,
		RatioTolerance));
	TestTrue(TEXT("Vehicle A 100 percent speed"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleAReferenceSpeedCmPerSec, VehicleAReferenceSpeedCmPerSec),
		1.0f,
		RatioTolerance));

	TestTrue(TEXT("Vehicle B same 50 percent phase despite different absolute speed"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleBReferenceSpeedCmPerSec * 0.50f, VehicleBReferenceSpeedCmPerSec),
		0.50f,
		RatioTolerance));
	TestTrue(TEXT("Speed ratio over reference clamps to one"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleAReferenceSpeedCmPerSec * 1.25f, VehicleAReferenceSpeedCmPerSec),
		1.0f,
		RatioTolerance));
	TestTrue(TEXT("Explicit overspeed ratio preserves 125 percent"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(
			VehicleAReferenceSpeedCmPerSec * 1.25f,
			VehicleAReferenceSpeedCmPerSec,
			1.5f),
		1.25f,
		RatioTolerance));
	TestTrue(TEXT("Explicit overspeed ratio clamps at 150 percent"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeSpeedRatio(
			VehicleAReferenceSpeedCmPerSec * 1.75f,
			VehicleAReferenceSpeedCmPerSec,
			1.5f),
		1.5f,
		RatioTolerance));
	TestTrue(TEXT("Invalid max presentation ratio exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeSpeedRatio(
			VehicleAReferenceSpeedCmPerSec,
			VehicleAReferenceSpeedCmPerSec,
			0.75f)));
	TestTrue(TEXT("Overspeed phase below reference exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::CalculateOverspeedPhase(0.75f, 1.5f)));
	TestTrue(TEXT("Overspeed phase at 125 percent is half"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateOverspeedPhase(1.25f, 1.5f),
		0.5f,
		RatioTolerance));
	TestTrue(TEXT("Overspeed phase at max is one"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateOverspeedPhase(1.5f, 1.5f),
		1.0f,
		RatioTolerance));
	TestTrue(TEXT("Overspeed phase above max clamps one"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateOverspeedPhase(2.0f, 1.5f),
		1.0f,
		RatioTolerance));
	TestTrue(TEXT("Overspeed phase invalid max exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::CalculateOverspeedPhase(1.25f, 1.0f)));
	TestTrue(TEXT("Negative speed magnitude fails safe to zero"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeSpeedRatio(-VehicleAReferenceSpeedCmPerSec, VehicleAReferenceSpeedCmPerSec)));

	TestTrue(TEXT("Positive 20 percent rate becomes acceleration"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeAccelerationRate(VehicleAReferenceSpeedCmPerSec * 0.20f, VehicleAReferenceSpeedCmPerSec),
		0.20f,
		RatioTolerance));
	TestTrue(TEXT("Positive rate does not become braking"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeBrakingRate(VehicleAReferenceSpeedCmPerSec * 0.20f, VehicleAReferenceSpeedCmPerSec)));

	TestTrue(TEXT("Negative 30 percent rate becomes braking"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeBrakingRate(-VehicleAReferenceSpeedCmPerSec * 0.30f, VehicleAReferenceSpeedCmPerSec),
		0.30f,
		RatioTolerance));
	TestTrue(TEXT("Negative rate does not become acceleration"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeAccelerationRate(-VehicleAReferenceSpeedCmPerSec * 0.30f, VehicleAReferenceSpeedCmPerSec)));

	TestTrue(TEXT("Positive lateral rate preserves sign"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeLateralRate(VehicleAReferenceSpeedCmPerSec * 0.40f, VehicleAReferenceSpeedCmPerSec),
		0.40f,
		RatioTolerance));
	TestTrue(TEXT("Negative lateral rate preserves sign"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeLateralRate(-VehicleAReferenceSpeedCmPerSec * 0.40f, VehicleAReferenceSpeedCmPerSec),
		-0.40f,
		RatioTolerance));
	TestTrue(TEXT("Lateral rate clamps positive overshoot"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeLateralRate(VehicleAReferenceSpeedCmPerSec * 2.0f, VehicleAReferenceSpeedCmPerSec),
		1.0f,
		RatioTolerance));
	TestTrue(TEXT("Lateral rate clamps negative overshoot"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::NormalizeLateralRate(-VehicleAReferenceSpeedCmPerSec * 2.0f, VehicleAReferenceSpeedCmPerSec),
		-1.0f,
		RatioTolerance));

	TestFalse(TEXT("Zero reference is invalid"), CFVehicleCameraMath::IsValidReferenceSpeed(0.0f));
	TestFalse(TEXT("Negative reference is invalid"), CFVehicleCameraMath::IsValidReferenceSpeed(-1.0f));
	TestFalse(TEXT("NaN reference is invalid"), CFVehicleCameraMath::IsValidReferenceSpeed(QuietNaN));
	TestFalse(TEXT("Infinite reference is invalid"), CFVehicleCameraMath::IsValidReferenceSpeed(PositiveInfinity));

	TestTrue(TEXT("Zero reference speed ratio exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeSpeedRatio(VehicleAReferenceSpeedCmPerSec, 0.0f)));
	TestTrue(TEXT("Negative reference acceleration exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeAccelerationRate(VehicleAReferenceSpeedCmPerSec, -1.0f)));
	TestTrue(TEXT("NaN reference braking exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeBrakingRate(-VehicleAReferenceSpeedCmPerSec, QuietNaN)));
	TestTrue(TEXT("Infinite reference lateral exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeLateralRate(VehicleAReferenceSpeedCmPerSec, PositiveInfinity)));

	TestTrue(TEXT("NaN speed input exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeSpeedRatio(QuietNaN, VehicleAReferenceSpeedCmPerSec)));
	TestTrue(TEXT("Infinite acceleration input exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeAccelerationRate(PositiveInfinity, VehicleAReferenceSpeedCmPerSec)));
	TestTrue(TEXT("NaN braking input exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeBrakingRate(QuietNaN, VehicleAReferenceSpeedCmPerSec)));
	TestTrue(TEXT("Infinite lateral input exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeLateralRate(PositiveInfinity, VehicleAReferenceSpeedCmPerSec)));

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleCameraLateralPresentationMathTest,
	"CarFight.VehicleCamera.CF_FQ_057.VCFX_P0_03.LateralPresentationMath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 실제 차체 움직임과 자유시점 정렬이 Lateral Presentation Roll 강도/방향을 올바르게 조절하는지 검증합니다.
bool FCFVehicleCameraLateralPresentationMathTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// float ratio 비교에서 허용할 검증 오차입니다.
	constexpr float Tolerance = 0.0001f;

	// invalid-input fail-safe 검증에 사용할 quiet NaN입니다.
	const float QuietNaN = std::numeric_limits<float>::quiet_NaN();

	TestTrue(TEXT("Calm body produces zero intensity"), FMath::IsNearlyZero(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			0.0f,
			0.0f,
			0.0f,
			7.5f,
			45.0f,
			90.0f)));

	TestTrue(TEXT("Half roll angle produces half intensity"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			3.75f,
			0.0f,
			0.0f,
			7.5f,
			45.0f,
			90.0f),
		0.5f,
		Tolerance));

	TestTrue(TEXT("Full roll rate produces full intensity"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			0.0f,
			45.0f,
			0.0f,
			7.5f,
			45.0f,
			90.0f),
		1.0f,
		Tolerance));

	TestTrue(TEXT("Half yaw rate produces half intensity"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			0.0f,
			0.0f,
			45.0f,
			7.5f,
			45.0f,
			90.0f),
		0.5f,
		Tolerance));

	TestTrue(TEXT("Strongest body signal wins without additive overshoot"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			1.875f,
			11.25f,
			67.5f,
			7.5f,
			45.0f,
			90.0f),
		0.75f,
		Tolerance));

	TestTrue(TEXT("Negative body motion uses magnitude"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			-3.75f,
			0.0f,
			0.0f,
			7.5f,
			45.0f,
			90.0f),
		0.5f,
		Tolerance));

	TestTrue(TEXT("Invalid body component contributes zero while valid signals remain"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralBodyMotionIntensity(
			QuietNaN,
			22.5f,
			0.0f,
			7.5f,
			45.0f,
			90.0f),
		0.5f,
		Tolerance));

	TestTrue(TEXT("Invalid full-intensity threshold fails that signal safe to zero"), FMath::IsNearlyZero(
		CFVehicleCameraMath::NormalizeMotionMagnitude(10.0f, 0.0f)));

	TestTrue(TEXT("Front view alignment is positive one"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateSignedViewAlignment(
			FVector::ForwardVector,
			FVector::ForwardVector),
		1.0f,
		Tolerance));

	TestTrue(TEXT("Side view alignment suppresses roll"), FMath::IsNearlyZero(
		CFVehicleCameraMath::CalculateSignedViewAlignment(
			FVector::ForwardVector,
			FVector::RightVector),
		Tolerance));

	TestTrue(TEXT("Rear view alignment reverses roll"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateSignedViewAlignment(
			FVector::ForwardVector,
			-FVector::ForwardVector),
		-1.0f,
		Tolerance));

	TestTrue(TEXT("Diagonal view alignment is continuous cosine scale"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateSignedViewAlignment(
			FVector::ForwardVector,
			FVector(1.0f, 1.0f, 0.0f)),
		0.70710678f,
		Tolerance));

	TestTrue(TEXT("View pitch does not reduce horizontal alignment"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateSignedViewAlignment(
			FVector::ForwardVector,
			FVector(1.0f, 0.0f, 1.0f)),
		1.0f,
		Tolerance));

	TestTrue(TEXT("Zero view direction fails alignment safe to zero"), FMath::IsNearlyZero(
		CFVehicleCameraMath::CalculateSignedViewAlignment(
			FVector::ForwardVector,
			FVector::ZeroVector)));

	TestTrue(TEXT("Body view and mode scales multiply"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralPresentationScale(
			0.5f,
			0.5f,
			0.25f),
		0.0625f,
		Tolerance));

	TestTrue(TEXT("Rear alignment preserves signed inversion"), FMath::IsNearlyEqual(
		CFVehicleCameraMath::CalculateLateralPresentationScale(
			0.5f,
			-1.0f,
			1.0f),
		-0.5f,
		Tolerance));

	TestTrue(TEXT("Invalid presentation scale input exact0"), FMath::IsNearlyZero(
		CFVehicleCameraMath::CalculateLateralPresentationScale(
			QuietNaN,
			1.0f,
			1.0f)));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

