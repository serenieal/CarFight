// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-07-31
// Description: CF-FQ-033 차량 방어 런타임 자동화 테스트
// Scope: VehicleHealth 호환, 6방향 독립 장갑, 쉴드, 관통, Legacy Fallback, 재생, DR-P0-03 통합과 DR-P0-04 Debug·Blueprint 계약을 검증합니다.
// Changelog:
// - v1.2.0: BlueprintAssignable 이벤트 7종, BlueprintPure Debug API, 정상·Legacy 결과 캐시와 Reset 수명 테스트 추가.
// - v1.1.0: Pawn 기본 DefenseComp, 정식 정적 진입점, Integrity 호환 결과와 HitScan·Projectile 결과 동등성 테스트 추가.
// - v1.0.0: DR-P0-02 HealthCompatibility, DirectionalArmor, ShieldArmorPenetration, OverflowLegacyFallback, ShieldRegeneration 테스트 추가.
// Migration:
// - DR-P0-03부터 ACFVehiclePawn은 VehicleHealthComp와 VehicleDefenseComp 기본 서브오브젝트를 함께 소유한다.
// - 실제 에셋 연결과 사용자 PIE는 DR-P0-05~07 범위로 남긴다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFDamageData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleDefenseData.h"

#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/UnrealType.h"

namespace
{
	// [v1.0.0] 자동화 테스트에서 사용할 Actor, HealthComp와 DefenseComp 묶음입니다.
	struct FCFDefenseTestFixture
	{
		// [v1.0.0] 테스트 대상 차량 역할을 하는 일반 Actor입니다.
		AActor* TargetActor = nullptr;

		// [v1.0.0] 테스트 대상의 차량 내구도 컴포넌트입니다.
		UCFVehicleHealthComp* HealthComponent = nullptr;

		// [v1.0.0] 테스트 대상의 차량 방어 컴포넌트입니다.
		UCFVehicleDefenseComp* DefenseComponent = nullptr;

		// [v1.0.0] 최대 차량 내구도와 선택 방어 데이터를 제공하는 VehicleData입니다.
		UCFVehicleData* VehicleData = nullptr;
	};

	// [v1.0.0] 테스트 월드에 HealthComp와 선택적 DefenseComp를 가진 Actor를 생성합니다.
	FCFDefenseTestFixture CreateDefenseTestFixture(
		UWorld* TestWorld,
		const FName ActorName,
		UCFVehicleDefenseData* DefenseData,
		const bool bAddDefenseComponent = true,
		const float MaximumIntegrity = 100.0f)
	{
		// [v1.0.0] 생성한 테스트 Actor와 컴포넌트를 반환할 묶음입니다.
		FCFDefenseTestFixture Fixture;

		if (!TestWorld)
		{
			return Fixture;
		}

		// [v1.0.0] 테스트 Actor에 안정적인 이름을 적용할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		Fixture.TargetActor = TestWorld->SpawnActor<AActor>(
			AActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);

		if (!Fixture.TargetActor)
		{
			return Fixture;
		}

				// [v1.0.0] 테스트 VehicleData 인스턴스에 사용할 안정적인 Object 이름입니다.
		const FName VehicleDataObjectName(*FString::Printf(TEXT("%s_VehicleData"), *ActorName.ToString()));

		Fixture.VehicleData = NewObject<UCFVehicleData>(Fixture.TargetActor, VehicleDataObjectName);
		Fixture.VehicleData->VehicleDurabilityConfig.MaxHealth = MaximumIntegrity;
		Fixture.VehicleData->DefaultDefenseData = DefenseData;

		// [v1.0.0] 테스트 HealthComponent 인스턴스에 사용할 안정적인 Object 이름입니다.
		const FName HealthComponentObjectName(*FString::Printf(TEXT("%s_Health"), *ActorName.ToString()));

		Fixture.HealthComponent = NewObject<UCFVehicleHealthComp>(Fixture.TargetActor, HealthComponentObjectName);
		Fixture.TargetActor->AddInstanceComponent(Fixture.HealthComponent);
		Fixture.HealthComponent->RegisterComponent();
		Fixture.HealthComponent->InitializeFromVehicleData(Fixture.VehicleData);

		if (bAddDefenseComponent)
		{
						// [v1.0.0] 테스트 DefenseComponent 인스턴스에 사용할 안정적인 Object 이름입니다.
			const FName DefenseComponentObjectName(*FString::Printf(TEXT("%s_Defense"), *ActorName.ToString()));

						Fixture.DefenseComponent = NewObject<UCFVehicleDefenseComp>(Fixture.TargetActor, DefenseComponentObjectName);
			Fixture.TargetActor->AddInstanceComponent(Fixture.DefenseComponent);
			Fixture.DefenseComponent->RegisterComponent();
			Fixture.DefenseComponent->Activate(true);
			Fixture.DefenseComponent->InitializeFromVehicleData(Fixture.VehicleData);
		}

		return Fixture;
	}

	// [v1.0.0] 직접 명중 테스트에 사용할 공통 DamageHitContext를 생성합니다.
	FCFDamageHitContext BuildDefenseDamageHitContext(
		AActor* TargetActor,
		AActor* InstigatorActor,
		UCFDamageData* DamageData,
		const FVector ImpactLocation)
	{
		// [v1.0.0] 방어 피해 테스트에 사용할 완성된 명중 컨텍스트입니다.
		FCFDamageHitContext DamageHitContext;
		DamageHitContext.DamageData = DamageData;
		DamageHitContext.HitActor = TargetActor;
		DamageHitContext.InstigatorActor = InstigatorActor;
		DamageHitContext.ImpactLocation = ImpactLocation;
				DamageHitContext.ImpactNormal = ImpactLocation.GetSafeNormal(SMALL_NUMBER, FVector::ForwardVector);
		DamageHitContext.IncomingDirection = -DamageHitContext.ImpactNormal;
		DamageHitContext.bBlockingHit = true;
		return DamageHitContext;
	}

	}

// [v1.0.1] Unreal Automation의 TestNearlyEqual 멤버 오버로드와 이름 충돌 없이 float 근사값을 검사합니다.
#define TestNearlyEqual(TestObject, What, Actual, Expected) \
	(TestObject).TestTrue((What), FMath::IsNearlyEqual((Actual), (Expected), KINDA_SMALL_NUMBER))


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleHealthCompatibilityTest,
	"CarFight.Damage.DR_P0_02.HealthCompatibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 기존 BaseDamage API와 신규 명시 Integrity API가 같은 상태·파괴 계약을 공유하는지 검증합니다.
bool FCFVehicleHealthCompatibilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] ActorComponent 등록과 소유 Actor 검증에 사용할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Health 호환 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 공격 주체 역할을 하는 일반 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();

	// [v1.0.0] DefenseComp 없이 기존 Health API만 검증할 대상 묶음입니다.
	FCFDefenseTestFixture Fixture = CreateDefenseTestFixture(TestWorld, TEXT("HealthCompatibilityTarget"), nullptr, false);
	if (!TestNotNull(TEXT("Health 호환 대상 생성"), Fixture.TargetActor)
		|| !TestNotNull(TEXT("Health 호환 컴포넌트 생성"), Fixture.HealthComponent)
		|| !TestNotNull(TEXT("공격 주체 생성"), InstigatorActor))
	{
		return false;
	}

	// [v1.0.0] 기존 BaseDamage Wrapper 검증에 사용할 피해 데이터입니다.
	UCFDamageData* DamageData = NewObject<UCFDamageData>(Fixture.TargetActor, TEXT("HealthCompatibilityDamage"));
	DamageData->BaseDamage = 25.0f;

	// [v1.0.0] 대상 정면에 명중한 공통 피해 컨텍스트입니다.
	const FCFDamageHitContext DamageHitContext = BuildDefenseDamageHitContext(
		Fixture.TargetActor,
		InstigatorActor,
		DamageData,
		FVector(100.0f, 0.0f, 0.0f));

	// [v1.0.0] 기존 BaseDamage Wrapper가 반환한 피해 적용 결과입니다.
	FCFDamageApplyResult LegacyWrapperResult;
	TestTrue(TEXT("기존 BaseDamage Wrapper 적용 성공"), Fixture.HealthComponent->ApplyDamageFromHitContext(DamageHitContext, LegacyWrapperResult));
	TestNearlyEqual(*this, TEXT("기존 Wrapper 요청 피해 25"), LegacyWrapperResult.RequestedDamage, 25.0f);
	TestNearlyEqual(*this, TEXT("기존 Wrapper 적용 피해 25"), LegacyWrapperResult.AppliedDamage, 25.0f);
	TestNearlyEqual(*this, TEXT("기존 Wrapper 이후 내구도 75"), Fixture.HealthComponent->GetCurrentIntegrity(), 75.0f);

	Fixture.HealthComponent->ResetHealthToMaximum();

	// [v1.0.0] 신규 명시 Integrity API가 반환한 피해 적용 결과입니다.
	FCFDamageApplyResult ExplicitIntegrityResult;
	TestTrue(TEXT("명시 Integrity 피해 적용 성공"), Fixture.HealthComponent->ApplyIntegrityDamageFromHitContext(DamageHitContext, 40.0f, ExplicitIntegrityResult));
	TestNearlyEqual(*this, TEXT("명시 Integrity 요청 피해 40"), ExplicitIntegrityResult.RequestedDamage, 40.0f);
	TestNearlyEqual(*this, TEXT("명시 Integrity 적용 피해 40"), ExplicitIntegrityResult.AppliedDamage, 40.0f);
	TestNearlyEqual(*this, TEXT("명시 Integrity 이후 내구도 60"), Fixture.HealthComponent->GetCurrentIntegrity(), 60.0f);
	TestNearlyEqual(*this, TEXT("Health·Integrity 비율 호환"), Fixture.HealthComponent->GetHealthRatio(), Fixture.HealthComponent->GetIntegrityRatio());

	// [v1.0.0] 남은 내구도를 초과하는 명시 피해의 파괴 결과입니다.
	FCFDamageApplyResult FatalIntegrityResult;
	TestTrue(TEXT("명시 치명 피해 적용 성공"), Fixture.HealthComponent->ApplyIntegrityDamageFromHitContext(DamageHitContext, 200.0f, FatalIntegrityResult));
	TestNearlyEqual(*this, TEXT("치명 피해 실제 적용량은 남은 60"), FatalIntegrityResult.AppliedDamage, 60.0f);
	TestTrue(TEXT("치명 피해 최초 파괴 표시"), FatalIntegrityResult.bDestroyedThisHit);
	TestTrue(TEXT("HealthComp 파괴 상태"), Fixture.HealthComponent->IsDestroyed());

	// [v1.0.0] 이미 파괴된 대상에 추가 피해를 시도한 결과입니다.
	FCFDamageApplyResult DestroyedRejectResult;
	TestFalse(TEXT("파괴 대상 추가 피해 거부"), Fixture.HealthComponent->ApplyIntegrityDamageFromHitContext(DamageHitContext, 10.0f, DestroyedRejectResult));
	TestEqual(TEXT("파괴 대상 거부 사유"), DestroyedRejectResult.RejectReason, ECFDamageApplyRejectReason::TargetDestroyed);

	Fixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDirectionalArmorRuntimeTest,
	"CarFight.Damage.DR_P0_02.DirectionalArmor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 지배 축 판정과 6방향 독립 장갑 Pool·피해 배율을 검증합니다.
bool FCFDirectionalArmorRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TestEqual(TEXT("+X는 Front"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(1.0f, 0.0f, 0.0f)), ECFArmorDirection::Front);
	TestEqual(TEXT("-X는 Rear"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(-1.0f, 0.0f, 0.0f)), ECFArmorDirection::Rear);
	TestEqual(TEXT("+Y는 Right"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(0.0f, 1.0f, 0.0f)), ECFArmorDirection::Right);
	TestEqual(TEXT("-Y는 Left"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(0.0f, -1.0f, 0.0f)), ECFArmorDirection::Left);
	TestEqual(TEXT("+Z는 Top"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(0.0f, 0.0f, 1.0f)), ECFArmorDirection::Top);
	TestEqual(TEXT("-Z는 Bottom"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(0.0f, 0.0f, -1.0f)), ECFArmorDirection::Bottom);
	TestEqual(TEXT("X·Y 동률은 X 우선"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(1.0f, 1.0f, 0.0f)), ECFArmorDirection::Front);
	TestEqual(TEXT("Z·X 동률은 X 우선"), UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(FVector(1.0f, 0.0f, 1.0f)), ECFArmorDirection::Front);

	// [v1.0.0] 6방향 장갑 실제 감소를 검증할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("6방향 장갑 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 쉴드 없이 방향 장갑만 검증할 방어 데이터입니다.
	UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>();
	DefenseData->bUseShield = false;
	DefenseData->ArmorResistance = 100.0f;
	DefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.0.0] 6방향 장갑을 독립적으로 피격할 대상 묶음입니다.
	FCFDefenseTestFixture Fixture = CreateDefenseTestFixture(TestWorld, TEXT("DirectionalArmorTarget"), DefenseData);

	// [v1.0.0] 공격 주체 역할을 하는 일반 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();

	// [v1.0.0] 방향 배율만큼 장갑을 감소시킬 기본 피해 데이터입니다.
	UCFDamageData* DamageData = NewObject<UCFDamageData>(Fixture.TargetActor, TEXT("DirectionalArmorDamage"));
	DamageData->BaseDamage = 10.0f;
	DamageData->ArmorPenetration = 0.0f;

	if (!TestNotNull(TEXT("6방향 방어 컴포넌트 생성"), Fixture.DefenseComponent)
		|| !TestNotNull(TEXT("6방향 Health 컴포넌트 생성"), Fixture.HealthComponent)
		|| !TestNotNull(TEXT("6방향 공격 주체 생성"), InstigatorActor))
	{
		return false;
	}

	// [v1.0.0] 방향, 피격 위치와 기대 장갑 감소량을 묶은 테스트 행입니다.
	struct FDirectionalArmorTestCase
	{
		// [v1.0.0] 이번 행에서 기대하는 피격 장갑 방향입니다.
		ECFArmorDirection ArmorDirection;

		// [v1.0.0] Actor Bounds 중심에서 해당 방향으로 떨어진 피격 위치입니다.
		FVector ImpactLocation;

		// [v1.0.0] 방향 피해 배율이 반영된 기대 장갑 감소량입니다.
		float ExpectedArmorDamage;
	};

	// [v1.0.0] Front, Left, Right, Rear, Top, Bottom 전체 런타임 테스트 행입니다.
	const TArray<FDirectionalArmorTestCase> TestCases =
	{
		{ ECFArmorDirection::Front, FVector(100.0f, 0.0f, 0.0f), 10.0f },
		{ ECFArmorDirection::Left, FVector(0.0f, -100.0f, 0.0f), 12.0f },
		{ ECFArmorDirection::Right, FVector(0.0f, 100.0f, 0.0f), 12.0f },
		{ ECFArmorDirection::Rear, FVector(-100.0f, 0.0f, 0.0f), 15.0f },
		{ ECFArmorDirection::Top, FVector(0.0f, 0.0f, 100.0f), 13.0f },
		{ ECFArmorDirection::Bottom, FVector(0.0f, 0.0f, -100.0f), 16.0f }
	};

	for (const FDirectionalArmorTestCase& TestCase : TestCases)
	{
		// [v1.0.0] 이번 방향 피해 적용 전 장갑 내구도입니다.
		const float ArmorBefore = Fixture.DefenseComponent->GetCurrentArmor(TestCase.ArmorDirection);

		// [v1.0.0] 이번 방향에 적용할 직접 명중 컨텍스트입니다.
		const FCFDamageHitContext DamageHitContext = BuildDefenseDamageHitContext(
			Fixture.TargetActor,
			InstigatorActor,
			DamageData,
			TestCase.ImpactLocation);

		// [v1.0.0] 이번 방향 피해 분배 결과입니다.
		FCFVehicleDamageResult DamageResult;
		TestTrue(TEXT("방향 장갑 피해 적용 성공"), Fixture.DefenseComponent->ApplyDamageFromHitContext(DamageHitContext, DamageResult));
		TestEqual(TEXT("판정 장갑 방향 일치"), DamageResult.ArmorDirection, TestCase.ArmorDirection);
		TestNearlyEqual(*this, TEXT("방향 장갑 흡수량 일치"), DamageResult.DamageAbsorbedByArmor, TestCase.ExpectedArmorDamage);
		TestNearlyEqual(*this, TEXT("방향 장갑 현재값 독립 감소"), Fixture.DefenseComponent->GetCurrentArmor(TestCase.ArmorDirection), ArmorBefore - TestCase.ExpectedArmorDamage);
		TestNearlyEqual(*this, TEXT("장갑이 모두 흡수해 내구도 유지"), Fixture.HealthComponent->GetCurrentIntegrity(), 100.0f);
	}

	Fixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFShieldArmorPenetrationTest,
	"CarFight.Damage.DR_P0_02.ShieldArmorPenetration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 쉴드 선흡수, 장갑 흡수와 50% 관통 피해 분배 불변식을 검증합니다.
bool FCFShieldArmorPenetrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 쉴드·장갑·내구도 3층 분배를 검증할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("쉴드·관통 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 최대 쉴드 50과 표준 장갑 저항 100을 가진 방어 데이터입니다.
	UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>();
	DefenseData->MaximumShield = 50.0f;
	DefenseData->ArmorResistance = 100.0f;
	DefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.0.0] 3층 피해 분배를 받을 대상 묶음입니다.
	FCFDefenseTestFixture Fixture = CreateDefenseTestFixture(TestWorld, TEXT("ShieldPenetrationTarget"), DefenseData);

	// [v1.0.0] 공격 주체 역할을 하는 일반 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();

	// [v1.0.0] 단계별 BaseDamage와 ArmorPenetration을 변경할 피해 데이터입니다.
	UCFDamageData* DamageData = NewObject<UCFDamageData>(Fixture.TargetActor, TEXT("ShieldPenetrationDamage"));

	// [v1.0.0] 대상 정면에 명중시키는 공통 위치입니다.
	const FVector FrontImpactLocation(100.0f, 0.0f, 0.0f);

	DamageData->BaseDamage = 30.0f;
	DamageData->ArmorPenetration = 0.0f;

	// [v1.0.0] 첫 피해가 쉴드에서 완전히 끝나는 결과입니다.
	FCFVehicleDamageResult ShieldOnlyResult;
	TestTrue(TEXT("쉴드 완전 흡수 피해 적용"), Fixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(Fixture.TargetActor, InstigatorActor, DamageData, FrontImpactLocation),
		ShieldOnlyResult));
	TestNearlyEqual(*this, TEXT("첫 피해 쉴드 흡수 30"), ShieldOnlyResult.DamageAbsorbedByShield, 30.0f);
	TestNearlyEqual(*this, TEXT("첫 피해 후 쉴드 20"), Fixture.DefenseComponent->GetCurrentShield(), 20.0f);
	TestNearlyEqual(*this, TEXT("첫 피해 장갑 미변경"), Fixture.DefenseComponent->GetCurrentArmor(ECFArmorDirection::Front), 100.0f);
	TestNearlyEqual(*this, TEXT("첫 피해 내구도 미변경"), Fixture.HealthComponent->GetCurrentIntegrity(), 100.0f);

	DamageData->BaseDamage = 40.0f;

	// [v1.0.0] 남은 쉴드 20과 정면 장갑 20이 나눠 흡수하는 결과입니다.
	FCFVehicleDamageResult ShieldBreakArmorResult;
	TestTrue(TEXT("쉴드 파괴 후 장갑 흡수 피해 적용"), Fixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(Fixture.TargetActor, InstigatorActor, DamageData, FrontImpactLocation),
		ShieldBreakArmorResult));
	TestNearlyEqual(*this, TEXT("두 번째 피해 쉴드 흡수 20"), ShieldBreakArmorResult.DamageAbsorbedByShield, 20.0f);
	TestTrue(TEXT("두 번째 피해 쉴드 파괴"), ShieldBreakArmorResult.bShieldBrokenThisHit);
	TestNearlyEqual(*this, TEXT("두 번째 피해 장갑 흡수 20"), ShieldBreakArmorResult.DamageAbsorbedByArmor, 20.0f);
	TestNearlyEqual(*this, TEXT("두 번째 피해 후 정면 장갑 80"), Fixture.DefenseComponent->GetCurrentArmor(ECFArmorDirection::Front), 80.0f);
	TestNearlyEqual(*this, TEXT("두 번째 피해 내구도 미변경"), Fixture.HealthComponent->GetCurrentIntegrity(), 100.0f);

	DamageData->BaseDamage = 50.0f;
	DamageData->ArmorPenetration = 50.0f;

	// [v1.0.0] 50% 관통으로 장갑 25와 내구도 25에 나뉘는 결과입니다.
	FCFVehicleDamageResult PenetrationResult;
	TestTrue(TEXT("50% 관통 피해 적용"), Fixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(Fixture.TargetActor, InstigatorActor, DamageData, FrontImpactLocation),
		PenetrationResult));
	TestNearlyEqual(*this, TEXT("관통 비율 0.5"), PenetrationResult.ArmorPenetrationRatio, 0.5f);
	TestNearlyEqual(*this, TEXT("관통 피해 장갑 흡수 25"), PenetrationResult.DamageAbsorbedByArmor, 25.0f);
	TestNearlyEqual(*this, TEXT("관통 피해 내구도 요청 25"), PenetrationResult.DamageRequestedForIntegrity, 25.0f);
	TestNearlyEqual(*this, TEXT("관통 피해 내구도 적용 25"), PenetrationResult.DamageAppliedToIntegrity, 25.0f);
	TestNearlyEqual(*this, TEXT("관통 피해 후 정면 장갑 55"), Fixture.DefenseComponent->GetCurrentArmor(ECFArmorDirection::Front), 55.0f);
	TestNearlyEqual(*this, TEXT("관통 피해 후 내구도 75"), Fixture.HealthComponent->GetCurrentIntegrity(), 75.0f);
	TestNearlyEqual(*this, TEXT("원본 피해 불변식"), PenetrationResult.RequestedDamage, PenetrationResult.DamageAbsorbedByShield + PenetrationResult.DamageAfterShield);
	TestNearlyEqual(*this, TEXT("방향 피해 불변식"), PenetrationResult.DirectionalDamage, PenetrationResult.DamageAbsorbedByArmor + PenetrationResult.DamageRequestedForIntegrity);
	TestNearlyEqual(*this, TEXT("내구도 피해 불변식"), PenetrationResult.DamageRequestedForIntegrity, PenetrationResult.DamageAppliedToIntegrity + PenetrationResult.IntegrityOverkillDamage);

	Fixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFArmorOverflowLegacyTest,
	"CarFight.Damage.DR_P0_02.OverflowLegacyFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 장갑 부족 초과 피해, 저항 0 완전 관통과 DefenseComp 없는 Legacy Fallback을 검증합니다.
bool FCFArmorOverflowLegacyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 장갑 초과·완전 관통·Legacy Fallback을 검증할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("장갑 초과·Legacy 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 공격 주체 역할을 하는 일반 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();

	// [v1.0.0] 정면 장갑 10만 가진 장갑 초과 피해 검증 데이터입니다.
	UCFVehicleDefenseData* OverflowDefenseData = NewObject<UCFVehicleDefenseData>();
	OverflowDefenseData->bUseShield = false;
	OverflowDefenseData->ArmorResistance = 100.0f;
	OverflowDefenseData->FrontArmorConfig.MaximumArmor = 10.0f;
	OverflowDefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.0.0] 장갑 초과 피해를 받을 대상 묶음입니다.
	FCFDefenseTestFixture OverflowFixture = CreateDefenseTestFixture(TestWorld, TEXT("ArmorOverflowTarget"), OverflowDefenseData);

	// [v1.0.0] 장갑 10을 초과하는 40 피해 데이터입니다.
	UCFDamageData* OverflowDamageData = NewObject<UCFDamageData>(OverflowFixture.TargetActor, TEXT("ArmorOverflowDamage"));
	OverflowDamageData->BaseDamage = 40.0f;
	OverflowDamageData->ArmorPenetration = 0.0f;

	// [v1.0.0] 장갑 10 흡수와 초과 30 내구도 피해 결과입니다.
	FCFVehicleDamageResult OverflowResult;
	TestTrue(TEXT("장갑 초과 피해 적용"), OverflowFixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(OverflowFixture.TargetActor, InstigatorActor, OverflowDamageData, FVector(100.0f, 0.0f, 0.0f)),
		OverflowResult));
	TestNearlyEqual(*this, TEXT("장갑 초과 피해 흡수 10"), OverflowResult.DamageAbsorbedByArmor, 10.0f);
	TestNearlyEqual(*this, TEXT("장갑 초과 내구도 요청 30"), OverflowResult.DamageRequestedForIntegrity, 30.0f);
	TestNearlyEqual(*this, TEXT("장갑 초과 후 내구도 70"), OverflowFixture.HealthComponent->GetCurrentIntegrity(), 70.0f);
	TestTrue(TEXT("장갑 초과 타격으로 정면 장갑 파괴"), OverflowResult.bArmorBrokenThisHit);

	// [v1.0.0] 장갑 저항 0으로 모든 피해가 관통하는 검증 데이터입니다.
	UCFVehicleDefenseData* ZeroResistanceDefenseData = NewObject<UCFVehicleDefenseData>();
	ZeroResistanceDefenseData->bUseShield = false;
	ZeroResistanceDefenseData->ArmorResistance = 0.0f;
	ZeroResistanceDefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.0.0] 완전 관통 피해를 받을 대상 묶음입니다.
	FCFDefenseTestFixture ZeroResistanceFixture = CreateDefenseTestFixture(TestWorld, TEXT("ZeroResistanceTarget"), ZeroResistanceDefenseData);

	// [v1.0.0] 저항 0에서 완전 관통할 20 피해 데이터입니다.
	UCFDamageData* FullPenetrationDamageData = NewObject<UCFDamageData>(ZeroResistanceFixture.TargetActor, TEXT("FullPenetrationDamage"));
	FullPenetrationDamageData->BaseDamage = 20.0f;
	FullPenetrationDamageData->ArmorPenetration = 0.0f;

	// [v1.0.0] 장갑을 소모하지 않고 내구도에 20을 적용한 완전 관통 결과입니다.
	FCFVehicleDamageResult FullPenetrationResult;
	TestTrue(TEXT("저항 0 완전 관통 피해 적용"), ZeroResistanceFixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(ZeroResistanceFixture.TargetActor, InstigatorActor, FullPenetrationDamageData, FVector(100.0f, 0.0f, 0.0f)),
		FullPenetrationResult));
	TestNearlyEqual(*this, TEXT("저항 0 관통 비율 1"), FullPenetrationResult.ArmorPenetrationRatio, 1.0f);
	TestNearlyEqual(*this, TEXT("저항 0 장갑 흡수 0"), FullPenetrationResult.DamageAbsorbedByArmor, 0.0f);
	TestNearlyEqual(*this, TEXT("저항 0 정면 장갑 유지"), ZeroResistanceFixture.DefenseComponent->GetCurrentArmor(ECFArmorDirection::Front), 100.0f);
	TestNearlyEqual(*this, TEXT("저항 0 내구도 80"), ZeroResistanceFixture.HealthComponent->GetCurrentIntegrity(), 80.0f);

	// [v1.0.0] DefenseComp 없이 기존 HealthComp만 가진 Legacy 대상 묶음입니다.
	FCFDefenseTestFixture LegacyFixture = CreateDefenseTestFixture(TestWorld, TEXT("LegacyHealthTarget"), nullptr, false);

	// [v1.0.0] Legacy BaseDamage 직접 적용을 검증할 피해 데이터입니다.
	UCFDamageData* LegacyDamageData = NewObject<UCFDamageData>(LegacyFixture.TargetActor, TEXT("LegacyFallbackDamage"));
	LegacyDamageData->BaseDamage = 25.0f;

	// [v1.0.0] 정식 TryApplyDamageToActor가 HealthComp로 Fallback한 전체 결과입니다.
	FCFVehicleDamageResult LegacyFallbackResult;
	TestTrue(TEXT("DefenseComp 없는 Legacy Fallback 적용"), UCFVehicleDefenseComp::TryApplyDamageToActor(
		BuildDefenseDamageHitContext(LegacyFixture.TargetActor, InstigatorActor, LegacyDamageData, FVector(100.0f, 0.0f, 0.0f)),
		LegacyFallbackResult));
	TestTrue(TEXT("Legacy Fallback 사용 표시"), LegacyFallbackResult.bUsedLegacyHealthFallback);
	TestNearlyEqual(*this, TEXT("Legacy 내구도 적용 25"), LegacyFallbackResult.DamageAppliedToIntegrity, 25.0f);
	TestNearlyEqual(*this, TEXT("Legacy 대상 내구도 75"), LegacyFixture.HealthComponent->GetCurrentIntegrity(), 75.0f);

	OverflowFixture.TargetActor->Destroy();
	ZeroResistanceFixture.TargetActor->Destroy();
	LegacyFixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFShieldRegenerationRuntimeTest,
	"CarFight.Damage.DR_P0_02.ShieldRegeneration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 유효 피해 후 재생 지연, 잔여 Delta 사용, 최대 쉴드 제한과 Tick 중지를 검증합니다.
bool FCFShieldRegenerationRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 쉴드 재생 상태 진행을 검증할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("쉴드 재생 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 최대 쉴드 100, 지연 2초와 초당 재생 10을 가진 방어 데이터입니다.
	UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>();
	DefenseData->MaximumShield = 100.0f;
	DefenseData->ShieldRegenerationDelaySeconds = 2.0f;
	DefenseData->ShieldRegenerationPerSecond = 10.0f;

	// [v1.0.0] 쉴드 피해와 재생을 받을 대상 묶음입니다.
	FCFDefenseTestFixture Fixture = CreateDefenseTestFixture(TestWorld, TEXT("ShieldRegenerationTarget"), DefenseData);

	// [v1.0.0] 공격 주체 역할을 하는 일반 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();

	// [v1.0.0] 쉴드를 20 감소시킬 피해 데이터입니다.
	UCFDamageData* DamageData = NewObject<UCFDamageData>(Fixture.TargetActor, TEXT("ShieldRegenerationDamage"));
	DamageData->BaseDamage = 20.0f;
	DamageData->ArmorPenetration = 0.0f;

	// [v1.0.0] 재생 지연을 시작할 쉴드 피해 결과입니다.
	FCFVehicleDamageResult DamageResult;
	TestTrue(TEXT("재생 검증용 쉴드 피해 적용"), Fixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(Fixture.TargetActor, InstigatorActor, DamageData, FVector(100.0f, 0.0f, 0.0f)),
		DamageResult));
	TestNearlyEqual(*this, TEXT("피해 후 쉴드 80"), Fixture.DefenseComponent->GetCurrentShield(), 80.0f);
	TestNearlyEqual(*this, TEXT("피해 후 재생 지연 2초"), Fixture.DefenseComponent->GetRemainingShieldRegenerationDelaySeconds(), 2.0f);
	TestFalse(TEXT("지연 중 실제 재생 아님"), Fixture.DefenseComponent->IsShieldRegenerating());
	TestTrue(TEXT("지연 대기 중 Tick 활성"), Fixture.DefenseComponent->IsComponentTickEnabled());

	Fixture.DefenseComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
	TestNearlyEqual(*this, TEXT("1초 지연 후 쉴드 유지"), Fixture.DefenseComponent->GetCurrentShield(), 80.0f);
	TestNearlyEqual(*this, TEXT("1초 지연 후 남은 1초"), Fixture.DefenseComponent->GetRemainingShieldRegenerationDelaySeconds(), 1.0f);

	Fixture.DefenseComponent->TickComponent(0.5f, LEVELTICK_All, nullptr);
	TestNearlyEqual(*this, TEXT("1.5초 지연 후 쉴드 유지"), Fixture.DefenseComponent->GetCurrentShield(), 80.0f);
	TestNearlyEqual(*this, TEXT("1.5초 지연 후 남은 0.5초"), Fixture.DefenseComponent->GetRemainingShieldRegenerationDelaySeconds(), 0.5f);

	Fixture.DefenseComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
	TestNearlyEqual(*this, TEXT("지연 초과 0.5초 재생으로 쉴드 85"), Fixture.DefenseComponent->GetCurrentShield(), 85.0f);
	TestTrue(TEXT("지연 종료 후 실제 재생 중"), Fixture.DefenseComponent->IsShieldRegenerating());
	TestNearlyEqual(*this, TEXT("재생 시작 후 남은 지연 0"), Fixture.DefenseComponent->GetRemainingShieldRegenerationDelaySeconds(), 0.0f);

	Fixture.DefenseComponent->TickComponent(10.0f, LEVELTICK_All, nullptr);
	TestNearlyEqual(*this, TEXT("최대 쉴드 100 제한"), Fixture.DefenseComponent->GetCurrentShield(), 100.0f);
	TestFalse(TEXT("완전 회복 후 재생 상태 중지"), Fixture.DefenseComponent->IsShieldRegenerating());
	TestFalse(TEXT("완전 회복 후 Tick 중지"), Fixture.DefenseComponent->IsComponentTickEnabled());

	Fixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDefenseRuntimeIntegrationTest,
	"CarFight.Damage.DR_P0_03.RuntimeIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] Pawn 기본 컴포넌트, 정식 피해 진입점, 기존 Integrity 결과와 HitScan·Projectile 동등성을 검증합니다.
bool FCFDefenseRuntimeIntegrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] 기본 서브오브젝트 구성 검증에 사용할 VehiclePawn 클래스 기본 객체입니다.
	const ACFVehiclePawn* VehiclePawnClassDefaultObject = GetDefault<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("VehiclePawn CDO 생성"), VehiclePawnClassDefaultObject))
	{
		return false;
	}

	TestNotNull(TEXT("VehiclePawn 기본 VehicleHealthComp 존재"), VehiclePawnClassDefaultObject->GetVehicleHealthComp());
	TestNotNull(TEXT("VehiclePawn 기본 VehicleDefenseComp 존재"), VehiclePawnClassDefaultObject->GetVehicleDefenseComp());

	// [v1.1.0] 정식 방어 진입점과 전달 경로 동등성을 검증할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("DR-P0-03 통합 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.1.0] 테스트 피해의 발사 주체 역할을 하는 일반 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("DR-P0-03 공격 주체 생성"), InstigatorActor))
	{
		return false;
	}

	// [v1.1.0] 쉴드 전흡수와 기존 Integrity 결과 변환을 검증할 방어 데이터입니다.
	UCFVehicleDefenseData* ShieldDefenseData = NewObject<UCFVehicleDefenseData>();
	ShieldDefenseData->MaximumShield = 50.0f;
	ShieldDefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.1.0] 정식 정적 피해 진입점을 받을 쉴드 대상 묶음입니다.
	FCFDefenseTestFixture ShieldFixture = CreateDefenseTestFixture(
		TestWorld,
		TEXT("DRP003ShieldTarget"),
		ShieldDefenseData);

	// [v1.1.0] 쉴드에서 완전히 흡수될 직접 피해 데이터입니다.
	UCFDamageData* ShieldDamageData = NewObject<UCFDamageData>(ShieldFixture.TargetActor, TEXT("DRP003ShieldDamage"));
	ShieldDamageData->BaseDamage = 25.0f;
	ShieldDamageData->ArmorPenetration = 0.0f;

	// [v1.1.0] HitScan 형식으로 정식 진입점에 전달할 공용 명중 컨텍스트입니다.
	FCFDamageHitContext ShieldHitContext = BuildDefenseDamageHitContext(
		ShieldFixture.TargetActor,
		InstigatorActor,
		ShieldDamageData,
		FVector(100.0f, 0.0f, 0.0f));
	ShieldHitContext.bFromProjectileActor = false;

	// [v1.1.0] 정식 VehicleDefenseComp 진입점이 반환한 쉴드 전흡수 전체 결과입니다.
	FCFVehicleDamageResult ShieldVehicleDamageResult;
	TestTrue(TEXT("정식 방어 진입점 쉴드 피해 적용"), UCFVehicleDefenseComp::TryApplyDamageToActor(
		ShieldHitContext,
		ShieldVehicleDamageResult));
	TestNearlyEqual(*this, TEXT("정식 진입점 쉴드 흡수 25"), ShieldVehicleDamageResult.DamageAbsorbedByShield, 25.0f);
	TestNearlyEqual(*this, TEXT("쉴드 전흡수 후 내구도 유지"), ShieldFixture.HealthComponent->GetCurrentIntegrity(), 100.0f);

	// [v1.1.0] 기존 VehicleDebug와 Pool 복사 경로에 전달할 Integrity 호환 결과입니다.
	const FCFDamageApplyResult ShieldCompatibilityResult = UCFVehicleDefenseComp::BuildIntegrityCompatibilityResult(
		ShieldVehicleDamageResult);
	TestFalse(TEXT("쉴드 전흡수는 내구도 적용 아님"), ShieldCompatibilityResult.bApplied);
	TestEqual(TEXT("쉴드 전흡수 호환 대상 유지"), ShieldCompatibilityResult.TargetActor.Get(), ShieldFixture.TargetActor);
	TestNearlyEqual(*this, TEXT("쉴드 전흡수 내구도 요청 0"), ShieldCompatibilityResult.RequestedDamage, 0.0f);
	TestNearlyEqual(*this, TEXT("쉴드 전흡수 내구도 100→100"), ShieldCompatibilityResult.HealthAfter, 100.0f);

	// [v1.1.0] HitScan과 Projectile 전달 플래그만 다르게 적용할 첫 번째 방어 데이터입니다.
	UCFVehicleDefenseData* HitScanDefenseData = NewObject<UCFVehicleDefenseData>();
	HitScanDefenseData->bUseShield = false;
	HitScanDefenseData->ArmorResistance = 100.0f;
	HitScanDefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.1.0] Projectile 전달 경로에 동일한 초기값을 제공할 두 번째 방어 데이터입니다.
	UCFVehicleDefenseData* ProjectileDefenseData = NewObject<UCFVehicleDefenseData>();
	ProjectileDefenseData->bUseShield = false;
	ProjectileDefenseData->ArmorResistance = 100.0f;
	ProjectileDefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.1.0] HitScan 형식의 피해를 받을 독립 대상 묶음입니다.
	FCFDefenseTestFixture HitScanFixture = CreateDefenseTestFixture(
		TestWorld,
		TEXT("DRP003HitScanTarget"),
		HitScanDefenseData);

	// [v1.1.0] Projectile 형식의 피해를 받을 독립 대상 묶음입니다.
	FCFDefenseTestFixture ProjectileFixture = CreateDefenseTestFixture(
		TestWorld,
		TEXT("DRP003ProjectileTarget"),
		ProjectileDefenseData);

	// [v1.1.0] 두 전달 경로에 동일하게 사용할 40 피해와 50 관통 데이터입니다.
	UCFDamageData* ParityDamageData = NewObject<UCFDamageData>(HitScanFixture.TargetActor, TEXT("DRP003ParityDamage"));
	ParityDamageData->BaseDamage = 40.0f;
	ParityDamageData->ArmorPenetration = 50.0f;

	// [v1.1.0] HitScan 전달 방식으로 표시한 명중 컨텍스트입니다.
	FCFDamageHitContext HitScanDamageHitContext = BuildDefenseDamageHitContext(
		HitScanFixture.TargetActor,
		InstigatorActor,
		ParityDamageData,
		FVector(100.0f, 0.0f, 0.0f));
	HitScanDamageHitContext.bFromProjectileActor = false;

	// [v1.1.0] Projectile 전달 방식으로 표시한 동일 핵심값의 명중 컨텍스트입니다.
	FCFDamageHitContext ProjectileDamageHitContext = BuildDefenseDamageHitContext(
		ProjectileFixture.TargetActor,
		InstigatorActor,
		ParityDamageData,
		FVector(100.0f, 0.0f, 0.0f));
	ProjectileDamageHitContext.bFromProjectileActor = true;

	// [v1.1.0] HitScan 형식 정식 방어 결과입니다.
	FCFVehicleDamageResult HitScanVehicleDamageResult;

	// [v1.1.0] Projectile 형식 정식 방어 결과입니다.
	FCFVehicleDamageResult ProjectileVehicleDamageResult;

	TestTrue(TEXT("HitScan 형식 정식 방어 적용"), UCFVehicleDefenseComp::TryApplyDamageToActor(
		HitScanDamageHitContext,
		HitScanVehicleDamageResult));
	TestTrue(TEXT("Projectile 형식 정식 방어 적용"), UCFVehicleDefenseComp::TryApplyDamageToActor(
		ProjectileDamageHitContext,
		ProjectileVehicleDamageResult));
	TestEqual(TEXT("HitScan·Projectile 장갑 방향 동등"), HitScanVehicleDamageResult.ArmorDirection, ProjectileVehicleDamageResult.ArmorDirection);
	TestNearlyEqual(*this, TEXT("HitScan·Projectile 장갑 흡수 동등"), HitScanVehicleDamageResult.DamageAbsorbedByArmor, ProjectileVehicleDamageResult.DamageAbsorbedByArmor);
	TestNearlyEqual(*this, TEXT("HitScan·Projectile 내구도 요청 동등"), HitScanVehicleDamageResult.DamageRequestedForIntegrity, ProjectileVehicleDamageResult.DamageRequestedForIntegrity);
	TestNearlyEqual(*this, TEXT("HitScan·Projectile 내구도 적용 동등"), HitScanVehicleDamageResult.DamageAppliedToIntegrity, ProjectileVehicleDamageResult.DamageAppliedToIntegrity);
	TestNearlyEqual(*this, TEXT("HitScan·Projectile 최종 내구도 동등"), HitScanFixture.HealthComponent->GetCurrentIntegrity(), ProjectileFixture.HealthComponent->GetCurrentIntegrity());

	ShieldFixture.TargetActor->Destroy();
	HitScanFixture.TargetActor->Destroy();
	ProjectileFixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDefenseDebugBlueprintTest,
	"CarFight.Damage.DR_P0_04.DebugBlueprintContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] BlueprintAssignable 이벤트 7종과 정상·Legacy 마지막 전체 피해 Debug 캐시·Reset 계약을 검증합니다.
bool FCFDefenseDebugBlueprintTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] Blueprint 이벤트와 Debug 함수 Reflection 계약을 확인할 DefenseComp 클래스입니다.
	const UClass* DefenseComponentClass = UCFVehicleDefenseComp::StaticClass();
	if (!TestNotNull(TEXT("VehicleDefenseComp 클래스 존재"), DefenseComponentClass))
	{
		return false;
	}

	// [v1.2.0] Blueprint에서 구독 가능해야 하는 P0 방어 이벤트 프로퍼티 이름입니다.
	const TArray<FName> BlueprintEventPropertyNames =
	{
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnVehicleDamageResolved),
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnShieldChanged),
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnShieldBroken),
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnArmorChanged),
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnArmorBroken),
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnShieldRegenerationStarted),
		GET_MEMBER_NAME_CHECKED(UCFVehicleDefenseComp, OnShieldFullyRestored)
	};

	for (const FName BlueprintEventPropertyName : BlueprintEventPropertyNames)
	{
		// [v1.2.0] 현재 이벤트 이름으로 찾은 Reflection 프로퍼티입니다.
		FProperty* BlueprintEventProperty = FindFProperty<FProperty>(DefenseComponentClass, BlueprintEventPropertyName);
		TestNotNull(*FString::Printf(TEXT("이벤트 프로퍼티 존재: %s"), *BlueprintEventPropertyName.ToString()), BlueprintEventProperty);
		if (BlueprintEventProperty)
		{
			TestTrue(*FString::Printf(TEXT("BlueprintAssignable 플래그: %s"), *BlueprintEventPropertyName.ToString()), BlueprintEventProperty->HasAnyPropertyFlags(CPF_BlueprintAssignable));
			TestTrue(*FString::Printf(TEXT("Multicast Delegate 타입: %s"), *BlueprintEventPropertyName.ToString()), BlueprintEventProperty->IsA<FMulticastDelegateProperty>());
		}
	}

	// [v1.2.0] Blueprint와 VehicleDebug가 사용할 공개 Debug 함수 이름입니다.
	const TArray<FName> BlueprintDebugFunctionNames =
	{
		GET_FUNCTION_NAME_CHECKED(UCFVehicleDefenseComp, BuildVehicleDefenseSummary),
		GET_FUNCTION_NAME_CHECKED(UCFVehicleDefenseComp, BuildVehicleDamageResultSummary),
		GET_FUNCTION_NAME_CHECKED(UCFVehicleDefenseComp, HasLastVehicleDamageResult),
		GET_FUNCTION_NAME_CHECKED(UCFVehicleDefenseComp, GetLastVehicleDamageResult),
		GET_FUNCTION_NAME_CHECKED(UCFVehicleDefenseComp, GetLastVehicleDamageResultSummary)
	};

	for (const FName BlueprintDebugFunctionName : BlueprintDebugFunctionNames)
	{
		// [v1.2.0] 현재 함수 이름으로 찾은 UFunction Reflection 정보입니다.
		const UFunction* BlueprintDebugFunction = DefenseComponentClass->FindFunctionByName(BlueprintDebugFunctionName);
		TestNotNull(*FString::Printf(TEXT("Blueprint Debug 함수 존재: %s"), *BlueprintDebugFunctionName.ToString()), BlueprintDebugFunction);
		if (BlueprintDebugFunction)
		{
			TestTrue(*FString::Printf(TEXT("BlueprintCallable 함수: %s"), *BlueprintDebugFunctionName.ToString()), BlueprintDebugFunction->HasAnyFunctionFlags(FUNC_BlueprintCallable));
			TestTrue(*FString::Printf(TEXT("BlueprintPure 함수: %s"), *BlueprintDebugFunctionName.ToString()), BlueprintDebugFunction->HasAnyFunctionFlags(FUNC_BlueprintPure));
		}
	}

	// [v1.2.0] 정상 방어 결과와 Legacy Fallback 캐시 수명을 검증할 자동화 테스트 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("DR-P0-04 Debug 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v1.2.0] Shield 10과 Front Armor 10 뒤에 Integrity 피해가 이어지는 정상 방어 데이터입니다.
	UCFVehicleDefenseData* DefenseData = NewObject<UCFVehicleDefenseData>();
	DefenseData->MaximumShield = 10.0f;
	DefenseData->FrontArmorConfig.MaximumArmor = 10.0f;
	DefenseData->ArmorResistance = 100.0f;
	DefenseData->ShieldRegenerationPerSecond = 0.0f;

	// [v1.2.0] 정상 방어 전체 결과를 저장할 대상 Actor·컴포넌트 묶음입니다.
	FCFDefenseTestFixture DefenseFixture = CreateDefenseTestFixture(TestWorld, TEXT("DRP004DefenseTarget"), DefenseData);

	// [v1.2.0] DefenseComp는 있지만 DefenseData가 없어 Legacy Integrity Fallback을 사용할 대상 묶음입니다.
	FCFDefenseTestFixture LegacyFixture = CreateDefenseTestFixture(TestWorld, TEXT("DRP004LegacyTarget"), nullptr, true);

	// [v1.2.0] 두 대상에 피해를 전달할 공격 주체 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("DR-P0-04 정상 DefenseComp 생성"), DefenseFixture.DefenseComponent)
		|| !TestNotNull(TEXT("DR-P0-04 Legacy DefenseComp 생성"), LegacyFixture.DefenseComponent)
		|| !TestNotNull(TEXT("DR-P0-04 공격 주체 생성"), InstigatorActor))
	{
		return false;
	}

	// [v1.2.0] Shield 10, Front Armor 10과 Integrity 10으로 분배될 30 피해 데이터입니다.
	UCFDamageData* DefenseDamageData = NewObject<UCFDamageData>(DefenseFixture.TargetActor, TEXT("DRP004DefenseDamage"));
	DefenseDamageData->BaseDamage = 30.0f;
	DefenseDamageData->ArmorPenetration = 0.0f;

	// [v1.2.0] 정상 정면 피해의 전체 결과입니다.
	FCFVehicleDamageResult AppliedDefenseResult;
	TestTrue(TEXT("DR-P0-04 정상 전체 방어 피해 적용"), DefenseFixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(DefenseFixture.TargetActor, InstigatorActor, DefenseDamageData, FVector(100.0f, 0.0f, 0.0f)),
		AppliedDefenseResult));
	TestTrue(TEXT("정상 마지막 전체 방어 결과 저장됨"), DefenseFixture.DefenseComponent->HasLastVehicleDamageResult());

	// [v1.2.0] VehicleDefenseComp가 보존한 정상 마지막 전체 피해 결과 복사본입니다.
	const FCFVehicleDamageResult CachedDefenseResult = DefenseFixture.DefenseComponent->GetLastVehicleDamageResult();
	TestEqual(TEXT("캐시 정면 방향"), CachedDefenseResult.ArmorDirection, ECFArmorDirection::Front);
	TestNearlyEqual(*this, TEXT("캐시 Shield 흡수 10"), CachedDefenseResult.DamageAbsorbedByShield, 10.0f);
	TestNearlyEqual(*this, TEXT("캐시 Armor 흡수 10"), CachedDefenseResult.DamageAbsorbedByArmor, 10.0f);
	TestNearlyEqual(*this, TEXT("캐시 Integrity 적용 10"), CachedDefenseResult.DamageAppliedToIntegrity, 10.0f);
	TestFalse(TEXT("현재 방어 상태 요약 비어 있지 않음"), DefenseFixture.DefenseComponent->BuildVehicleDefenseSummary().IsEmpty());
	TestFalse(TEXT("정상 마지막 전체 피해 요약 비어 있지 않음"), DefenseFixture.DefenseComponent->GetLastVehicleDamageResultSummary().IsEmpty());

	// [v1.2.0] Legacy Fallback에서 직접 Integrity에 적용할 15 피해 데이터입니다.
	UCFDamageData* LegacyDamageData = NewObject<UCFDamageData>(LegacyFixture.TargetActor, TEXT("DRP004LegacyDamage"));
	LegacyDamageData->BaseDamage = 15.0f;

	// [v1.2.0] DefenseComp 내부 Legacy Fallback 경로가 반환한 전체 결과입니다.
	FCFVehicleDamageResult LegacyDamageResult;
	TestTrue(TEXT("DR-P0-04 Legacy Fallback 피해 적용"), LegacyFixture.DefenseComponent->ApplyDamageFromHitContext(
		BuildDefenseDamageHitContext(LegacyFixture.TargetActor, InstigatorActor, LegacyDamageData, FVector(100.0f, 0.0f, 0.0f)),
		LegacyDamageResult));
	TestTrue(TEXT("Legacy Fallback 결과 표시"), LegacyDamageResult.bUsedLegacyHealthFallback);
	TestTrue(TEXT("Legacy 마지막 전체 결과 저장됨"), LegacyFixture.DefenseComponent->HasLastVehicleDamageResult());
	TestTrue(TEXT("Legacy 캐시 결과 표시"), LegacyFixture.DefenseComponent->GetLastVehicleDamageResult().bUsedLegacyHealthFallback);
	TestFalse(TEXT("Legacy 마지막 전체 피해 요약 비어 있지 않음"), LegacyFixture.DefenseComponent->GetLastVehicleDamageResultSummary().IsEmpty());

	DefenseFixture.DefenseComponent->ResetDefenseToMaximum();
	TestFalse(TEXT("Reset 후 마지막 전체 결과 제거"), DefenseFixture.DefenseComponent->HasLastVehicleDamageResult());
	TestEqual(TEXT("Reset 후 결과 요약 기본값"), DefenseFixture.DefenseComponent->GetLastVehicleDamageResultSummary(), FString(TEXT("차량 방어 피해 기록 없음")));

	DefenseFixture.TargetActor->Destroy();
	LegacyFixture.TargetActor->Destroy();
	InstigatorActor->Destroy();
	return true;
}

#undef TestNearlyEqual

#endif // WITH_DEV_AUTOMATION_TESTS
