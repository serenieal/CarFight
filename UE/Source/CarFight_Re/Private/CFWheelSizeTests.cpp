// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWheelSizeTests.cpp
// Version: v1.3.0
// Date: 2026-09-01
// Description: CF-FQ-040 WSA schema/helper + Runtime Visual/WheelSync + deterministic Right fallback/full-transform focused Automation입니다.
// Scope: additive/legacy 호환, Socket-authored scale, Legacy/Socket/Manual transition, FL fallback, Right source-aware orientation, per-wheel spin handedness, WheelSync transform 회귀를 검증합니다.
// Changelog:
// - v1.3.0: authored full-transform restoration, Legacy→Socket→Manual mode transition, Construction cache invalidation, RR absolute/delta handedness assertion을 추가.
// - v1.2.1: RightFallbackOrientationSpin에 production delta AddLocalRotation 경로의 Left +1 / Right fallback -1 handedness 직접 검증 추가.
// - v1.2.0: WSA-P0-07 RightFallbackOrientationSpin을 추가해 Right Roll180, repeated Apply idempotency, explicit same-pointer override, fallback↔explicit re-init과 spin handedness를 검증.
// - v1.1.0: WSA-P0-03 RuntimeVisualFallback 테스트를 추가해 실제 Pawn WheelVisual/Layout/WheelSync 경계를 transient-only로 검증.
// - v1.0.0: WSA-P0-01 SchemaUtilityLegacy 최초 추가.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFWheelSizeUtils.h"
#include "CFWheelSyncComp.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWheelSizeSchemaUtilityLegacyTest,
	"CarFight.Vehicle.WheelSize.WSA_P0_01.SchemaUtilityLegacy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWheelSizeRuntimeVisualTest,
	"CarFight.Vehicle.WheelSize.WSA_P0_03.RuntimeVisualFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWheelSizeRightFallbackOrientationSpinTest,
	"CarFight.Vehicle.WheelSize.WSA_P0_07.RightFallbackOrientationSpin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace CFWheelSizeTestsPrivate
{
	struct FWheelComponentPair
	{
		USceneComponent* Anchor = nullptr;
		UStaticMeshComponent* Mesh = nullptr;
	};

	// 실제 Pawn의 Component discovery 규칙과 같은 표준 이름으로 transient Wheel Anchor/Mesh pair를 추가합니다.
	FWheelComponentPair AddWheelComponentPair(ACFVehiclePawn& VehiclePawn, const FName AnchorName, const FName MeshName, const FVector& InitialAnchorScale)
	{
		FWheelComponentPair Pair;

		Pair.Anchor = NewObject<USceneComponent>(&VehiclePawn, AnchorName);
		if (!Pair.Anchor)
		{
			return Pair;
		}
		VehiclePawn.AddInstanceComponent(Pair.Anchor);
		Pair.Anchor->SetupAttachment(VehiclePawn.GetRootComponent());
		Pair.Anchor->SetRelativeScale3D(InitialAnchorScale);
		Pair.Anchor->RegisterComponent();

		Pair.Mesh = NewObject<UStaticMeshComponent>(&VehiclePawn, MeshName);
		if (!Pair.Mesh)
		{
			return Pair;
		}
		VehiclePawn.AddInstanceComponent(Pair.Mesh);
		Pair.Mesh->SetupAttachment(Pair.Anchor);
		Pair.Mesh->RegisterComponent();
		return Pair;
	}
}

bool FCFWheelSizeSchemaUtilityLegacyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 새 runtime schema의 additive 기본값이 기존 VehicleData 동작을 바꾸지 않는지 검증합니다.
	const FCFWheelAnchorPose DefaultAnchorPose;
	const FCFVehicleWheelVisualConfig DefaultWheelVisualConfig;
	TestTrue(TEXT("WheelAnchor RelativeScale default is OneVector"), DefaultAnchorPose.RelativeScale.Equals(FVector::OneVector));
	TestFalse(TEXT("Wheel socket size mode default is false"), DefaultWheelVisualConfig.bUseWheelSocketScale);

	// Canonical 100x25x100cm Wheel Bounds contract 자체를 공용 helper로 검증합니다.
	FString CanonicalBoundsError;
	TestTrue(
		TEXT("Canonical 100x25x100 centered bounds validate"),
		FCFWheelSizeUtils::ValidateCanonicalWheelBounds(FVector::ZeroVector, FVector(50.0, 12.5, 50.0), CanonicalBoundsError));
	FString NonCanonicalBoundsError;
	TestFalse(
		TEXT("Non-canonical cube wheel bounds are rejected"),
		FCFWheelSizeUtils::ValidateCanonicalWheelBounds(FVector::ZeroVector, FVector(50.0, 50.0, 50.0), NonCanonicalBoundsError));

	// Canonical 100x25x100cm Wheel의 BoundsExtent와 USER Scale 0.72/1.12/0.72입니다.
	const FVector CanonicalBoundsExtent(50.0, 12.5, 50.0);
	const FVector AuthoredSocketScale(0.72, 1.12, 0.72);
	FCFDerivedWheelSize DerivedWheelSize;
	FString DeriveError;
	TestTrue(
		TEXT("Canonical wheel size derive succeeds"),
		FCFWheelSizeUtils::DeriveWheelSizeFromBoundsAndScale(
			CanonicalBoundsExtent,
			AuthoredSocketScale,
			DerivedWheelSize,
			DeriveError));
	TestTrue(TEXT("Derived radius is 36cm"), FMath::IsNearlyEqual(DerivedWheelSize.RadiusCm, 36.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Derived width is 28cm"), FMath::IsNearlyEqual(DerivedWheelSize.WidthCm, 28.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Visual scale preserves exact socket scale"), DerivedWheelSize.VisualScale.Equals(AuthoredSocketScale));

	// X/Z가 다른 USER Scale은 타원형 Wheel이 되므로 fail-closed해야 합니다.
	FString InvalidScaleError;
	TestFalse(
		TEXT("Non-circular socket scale is rejected"),
		FCFWheelSizeUtils::ValidateWheelSocketScale(FVector(0.72, 1.12, 0.70), InvalidScaleError));
	TestTrue(TEXT("Invalid socket scale reports reason"), !InvalidScaleError.IsEmpty());

	FString ZeroScaleError;
	TestFalse(
		TEXT("Zero socket scale is rejected"),
		FCFWheelSizeUtils::ValidateWheelSocketScale(FVector(0.0, 1.0, 0.0), ZeroScaleError));
	TestTrue(TEXT("Zero socket scale reports reason"), !ZeroScaleError.IsEmpty());

	// X/Z Bounds 자체가 원형 계약을 벗어나면 base radius를 임의 평균해 숨기지 않습니다.
	float InvalidBaseRadiusCm = 0.0f;
	FString InvalidBoundsError;
	TestFalse(
		TEXT("Non-circular wheel bounds are rejected"),
		FCFWheelSizeUtils::MeasureBaseWheelRadius(FVector(50.0, 12.5, 49.0), InvalidBaseRadiusCm, InvalidBoundsError));
	TestTrue(TEXT("Invalid bounds reports reason"), !InvalidBoundsError.IsEmpty());

	// 같은 axle의 동일 derived size는 통과하고 폭이 다른 결과는 실패해야 합니다.
	FCFDerivedWheelSize MatchingWheelSize = DerivedWheelSize;
	TestTrue(TEXT("Matching axle wheel sizes are compatible"), FCFWheelSizeUtils::AreWheelSizesCompatible(DerivedWheelSize, MatchingWheelSize));
	MatchingWheelSize.WidthCm += 1.0f;
	TestFalse(TEXT("Mismatched axle wheel width is rejected"), FCFWheelSizeUtils::AreWheelSizesCompatible(DerivedWheelSize, MatchingWheelSize));

	// WSA 이전에 저장된 대표 SUV VehicleData는 새 property가 직렬화돼 있지 않아도 C++ additive default로 복원되어야 합니다.
	const UCFVehicleData* LegacyVehicleData = LoadObject<UCFVehicleData>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV.DA_TestSUV"));
	if (!TestNotNull(TEXT("Legacy DA_TestSUV loads"), LegacyVehicleData))
	{
		return false;
	}

	TestFalse(TEXT("Legacy VehicleData keeps socket size mode disabled"), LegacyVehicleData->WheelVisualConfig.bUseWheelSocketScale);
	TestFalse(TEXT("Legacy VehicleData keeps existing visual auto-scale false path"), LegacyVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius);
	TestTrue(TEXT("Legacy FL RelativeScale defaults to OneVector"), LegacyVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeScale.Equals(FVector::OneVector));
	TestTrue(TEXT("Legacy FR RelativeScale defaults to OneVector"), LegacyVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeScale.Equals(FVector::OneVector));
	TestTrue(TEXT("Legacy RL RelativeScale defaults to OneVector"), LegacyVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeScale.Equals(FVector::OneVector));
	TestTrue(TEXT("Legacy RR RelativeScale defaults to OneVector"), LegacyVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeScale.Equals(FVector::OneVector));

	return true;
}

// WSA-P0-03 실제 Pawn 경계에서 Socket Scale visual authority, FL mesh fallback과 WheelSync transform ownership 회귀를 검증합니다.
bool FCFWheelSizeRuntimeVisualTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("WSA-P0-03 transient world exists"), TestWorld))
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("WSAP003VehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!TestNotNull(TEXT("WSA-P0-03 VehiclePawn exists"), VehiclePawn))
	{
		return false;
	}

	const FVector InitialAnchorScale(1.25, 1.25, 1.25);
	CFWheelSizeTestsPrivate::FWheelComponentPair Wheels[4];
	Wheels[0] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_FL"), TEXT("Wheel_Mesh_FL"), InitialAnchorScale);
	Wheels[1] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_FR"), TEXT("Wheel_Mesh_FR"), InitialAnchorScale);
	Wheels[2] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_RL"), TEXT("Wheel_Mesh_RL"), InitialAnchorScale);
	Wheels[3] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_RR"), TEXT("Wheel_Mesh_RR"), InitialAnchorScale);

	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(Wheels); ++WheelIndex)
	{
		if (!TestNotNull(*FString::Printf(TEXT("Wheel %d anchor exists"), WheelIndex), Wheels[WheelIndex].Anchor)
			|| !TestNotNull(*FString::Printf(TEXT("Wheel %d mesh component exists"), WheelIndex), Wheels[WheelIndex].Mesh))
		{
			VehiclePawn->Destroy();
			return false;
		}
	}

	UStaticMesh* SharedWheelMesh = LoadObject<UStaticMesh>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL"));
	if (!TestNotNull(TEXT("Canonical shared Wheel_FL loads"), SharedWheelMesh))
	{
		VehiclePawn->Destroy();
		return false;
	}

	UCFVehicleData* RuntimeVehicleData = NewObject<UCFVehicleData>(VehiclePawn, TEXT("DA_WSA_P0_03_Runtime"));
	if (!TestNotNull(TEXT("WSA runtime VehicleData exists"), RuntimeVehicleData))
	{
		VehiclePawn->Destroy();
		return false;
	}

	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFL = SharedWheelMesh;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFR = nullptr;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshRL = nullptr;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshRR = nullptr;

	RuntimeVehicleData->WheelVisualConfig.bUseWheelVisualOverrides = true;
	RuntimeVehicleData->WheelVisualConfig.ExpectedWheelCount = 4;
	RuntimeVehicleData->WheelVisualConfig.FrontWheelCountForSteering = 2;
	RuntimeVehicleData->WheelVisualConfig.bUseWheelSocketScale = true;
	// 잘못된 persisted conflict가 있더라도 Runtime에서 이중 scale하지 않고 Socket authority가 우선하는지 검증합니다.
	RuntimeVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = true;
	RuntimeVehicleData->VehicleMovementConfig.FrontWheelRadius = 10.0f;
	RuntimeVehicleData->VehicleMovementConfig.RearWheelRadius = 10.0f;

	RuntimeVehicleData->VehicleLayoutConfig.bUseLayoutOverrides = true;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation = FVector(100.0, -50.0, 20.0);
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeLocation = FVector(100.0, 50.0, 20.0);
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeLocation = FVector(-100.0, -50.0, 20.0);
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeLocation = FVector(-100.0, 50.0, 20.0);
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeRotation = FRotator::ZeroRotator;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeRotation = FRotator::ZeroRotator;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeRotation = FRotator::ZeroRotator;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeRotation = FRotator::ZeroRotator;

	const FVector FrontSocketScale(0.72, 1.12, 0.72);
	const FVector RearSocketScale(0.80, 1.00, 0.80);
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeScale = FrontSocketScale;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeScale = FrontSocketScale;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeScale = RearSocketScale;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeScale = RearSocketScale;

	VehiclePawn->VehicleData = RuntimeVehicleData;
	VehiclePawn->ApplyVehicleWheelVisualConfig();

	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(Wheels); ++WheelIndex)
	{
		TestTrue(*FString::Printf(TEXT("Wheel %d uses shared FL fallback mesh"), WheelIndex), Wheels[WheelIndex].Mesh->GetStaticMesh() == SharedWheelMesh);
	}
	TestTrue(TEXT("FL visual scale exact Socket authored value"), Wheels[0].Mesh->GetRelativeScale3D().Equals(FrontSocketScale));
	TestTrue(TEXT("FR visual scale exact Socket authored value"), Wheels[1].Mesh->GetRelativeScale3D().Equals(FrontSocketScale));
	TestTrue(TEXT("RL visual scale exact Socket authored value"), Wheels[2].Mesh->GetRelativeScale3D().Equals(RearSocketScale));
	TestTrue(TEXT("RR visual scale exact Socket authored value"), Wheels[3].Mesh->GetRelativeScale3D().Equals(RearSocketScale));

	// Runtime layout 적용은 Location/Rotation만 소유하며 Socket size scale을 Wheel_Anchor parent transform으로 전달하지 않아야 합니다.
	VehiclePawn->ApplyVehicleLayoutConfig();
	TestTrue(TEXT("FL anchor scale unchanged by layout apply"), Wheels[0].Anchor->GetRelativeScale3D().Equals(InitialAnchorScale));
	TestTrue(TEXT("FR anchor scale unchanged by layout apply"), Wheels[1].Anchor->GetRelativeScale3D().Equals(InitialAnchorScale));
	TestTrue(TEXT("RL anchor scale unchanged by layout apply"), Wheels[2].Anchor->GetRelativeScale3D().Equals(InitialAnchorScale));
	TestTrue(TEXT("RR anchor scale unchanged by layout apply"), Wheels[3].Anchor->GetRelativeScale3D().Equals(InitialAnchorScale));
	TestTrue(TEXT("FL layout location applied"), Wheels[0].Anchor->GetRelativeLocation().Equals(RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation));

	UCFWheelSyncComp* WheelSyncComp = VehiclePawn->GetWheelSyncComp();
	if (!TestNotNull(TEXT("WheelSync component exists"), WheelSyncComp))
	{
		VehiclePawn->Destroy();
		return false;
	}

	WheelSyncComp->bEnableApplyTransformsInCpp = true;
	WheelSyncComp->bApplySteeringYawInCpp = true;
	WheelSyncComp->bApplySuspensionZInCpp = true;
	WheelSyncComp->bApplySpinPitchInCpp = true;
	if (!TestTrue(TEXT("WheelSync prepares with WSA visual components"), WheelSyncComp->TryPrepareWheelSync()))
	{
		AddError(WheelSyncComp->LastValidationSummary);
		VehiclePawn->Destroy();
		return false;
	}

	TArray<FCFWheelVisualInput> WheelInputs;
	WheelInputs.SetNum(4);
	for (int32 WheelIndex = 0; WheelIndex < WheelInputs.Num(); ++WheelIndex)
	{
		WheelInputs[WheelIndex].WheelIndex = WheelIndex;
		WheelInputs[WheelIndex].bIsValidInput = true;
		WheelInputs[WheelIndex].SteeringYawDeg = WheelIndex < 2 ? 7.0f : 0.0f;
		WheelInputs[WheelIndex].SuspensionOffsetZ = 3.0f + static_cast<float>(WheelIndex);
		WheelInputs[WheelIndex].SpinPitchDeg = 15.0f + static_cast<float>(WheelIndex);
		WheelInputs[WheelIndex].SpinPitchDeltaDeg = 0.0f;
		WheelInputs[WheelIndex].bApplySpinPitchAsDelta = false;
	}

	TestTrue(TEXT("WheelSync steering/suspension/spin apply succeeds"), WheelSyncComp->ApplyWheelVisualInputsPhase2(WheelInputs));
	TestTrue(TEXT("WheelSync steering yaw still applies"), FMath::IsNearlyEqual(Wheels[0].Anchor->GetRelativeRotation().Yaw, 7.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("WheelSync suspension Z still applies"), FMath::IsNearlyEqual(Wheels[0].Anchor->GetRelativeLocation().Z, 23.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("WheelSync spin pitch still applies"), FMath::IsNearlyEqual(Wheels[0].Mesh->GetRelativeRotation().Pitch, 15.0f, KINDA_SMALL_NUMBER));

	// WheelSync는 Location/Rotation만 갱신하므로 WSA Wheel_Mesh Scale은 transform update 후에도 exact 유지되어야 합니다.
	TestTrue(TEXT("FL Socket scale survives WheelSync"), Wheels[0].Mesh->GetRelativeScale3D().Equals(FrontSocketScale));
	TestTrue(TEXT("FR Socket scale survives WheelSync"), Wheels[1].Mesh->GetRelativeScale3D().Equals(FrontSocketScale));
	TestTrue(TEXT("RL Socket scale survives WheelSync"), Wheels[2].Mesh->GetRelativeScale3D().Equals(RearSocketScale));
	TestTrue(TEXT("RR Socket scale survives WheelSync"), Wheels[3].Mesh->GetRelativeScale3D().Equals(RearSocketScale));
	TestTrue(TEXT("FL anchor scale survives WheelSync"), Wheels[0].Anchor->GetRelativeScale3D().Equals(InitialAnchorScale));

	VehiclePawn->Destroy();
	return true;
}

// WSA-P0-07 FL-only shared Wheel fallback의 Right orientation, idempotency, explicit override와 spin handedness를 검증합니다.
bool FCFWheelSizeRightFallbackOrientationSpinTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Right fallback 시각 계약을 검증할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("WSA-P0-07 transient world exists"), TestWorld))
	{
		return false;
	}

	// 테스트 Pawn의 고정 이름을 포함하는 spawn 설정입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("WSAP007RightFallbackVehiclePawn");

	// 실제 ACFVehiclePawn runtime 경계를 사용할 transient Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!TestNotNull(TEXT("WSA-P0-07 VehiclePawn exists"), VehiclePawn))
	{
		return false;
	}

	// Wheel_Anchor가 Socket Scale을 소유하지 않는지 함께 보호할 초기 parent scale입니다.
	const FVector InitialAnchorScale(1.15, 1.15, 1.15);

	// 실제 Pawn component discovery 이름 계약과 동일한 네 Wheel pair입니다.
	CFWheelSizeTestsPrivate::FWheelComponentPair Wheels[4];
	Wheels[0] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_FL"), TEXT("Wheel_Mesh_FL"), InitialAnchorScale);
	Wheels[1] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_FR"), TEXT("Wheel_Mesh_FR"), InitialAnchorScale);
	Wheels[2] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_RL"), TEXT("Wheel_Mesh_RL"), InitialAnchorScale);
	Wheels[3] = CFWheelSizeTestsPrivate::AddWheelComponentPair(*VehiclePawn, TEXT("Wheel_Anchor_RR"), TEXT("Wheel_Mesh_RR"), InitialAnchorScale);

	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(Wheels); ++WheelIndex)
	{
		if (!TestNotNull(*FString::Printf(TEXT("WSA-P0-07 Wheel %d anchor exists"), WheelIndex), Wheels[WheelIndex].Anchor)
			|| !TestNotNull(*FString::Printf(TEXT("WSA-P0-07 Wheel %d mesh exists"), WheelIndex), Wheels[WheelIndex].Mesh))
		{
			VehiclePawn->Destroy();
			return false;
		}
	}

	// BP-authored full transform 보존까지 확인할 각 Wheel_Mesh 기준 회전입니다.
	const FRotator AuthoredBaseRotations[4] =
	{
		FRotator(0.0f, 2.0f, 4.0f),
		FRotator(0.0f, 3.0f, 6.0f),
		FRotator(0.0f, -2.0f, -4.0f),
		FRotator(0.0f, -3.0f, -6.0f)
	};

	// BP-authored RelativeLocation 복원을 검증할 각 Wheel_Mesh 기준 위치입니다.
	const FVector AuthoredBaseLocations[4] =
	{
		FVector(1.0, 2.0, 3.0),
		FVector(4.0, 5.0, 6.0),
		FVector(-1.0, -2.0, -3.0),
		FVector(-4.0, -5.0, -6.0)
	};

	// Manual scale authority로 돌아왔을 때 복원되어야 할 각 Wheel_Mesh authored base scale입니다.
	const FVector AuthoredBaseScales[4] =
	{
		FVector(1.01, 1.02, 1.03),
		FVector(1.04, 1.05, 1.06),
		FVector(0.99, 0.98, 0.97),
		FVector(0.96, 0.95, 0.94)
	};

	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(Wheels); ++WheelIndex)
	{
		Wheels[WheelIndex].Mesh->SetRelativeLocation(AuthoredBaseLocations[WheelIndex]);
		Wheels[WheelIndex].Mesh->SetRelativeRotation(AuthoredBaseRotations[WheelIndex]);
		Wheels[WheelIndex].Mesh->SetRelativeScale3D(AuthoredBaseScales[WheelIndex]);
	}

	// FL-only fallback source로 사용할 canonical shared Wheel Mesh입니다.
	UStaticMesh* SharedWheelMesh = LoadObject<UStaticMesh>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL"));
	if (!TestNotNull(TEXT("WSA-P0-07 canonical shared Wheel_FL loads"), SharedWheelMesh))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// actual Wagon과 같은 FL-only fallback 구조를 구성할 transient VehicleData입니다.
	UCFVehicleData* RuntimeVehicleData = NewObject<UCFVehicleData>(VehiclePawn, TEXT("DA_WSA_P0_07_RightFallbackRuntime"));
	if (!TestNotNull(TEXT("WSA-P0-07 runtime VehicleData exists"), RuntimeVehicleData))
	{
		VehiclePawn->Destroy();
		return false;
	}

	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFL = SharedWheelMesh;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFR = nullptr;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshRL = nullptr;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshRR = nullptr;
	RuntimeVehicleData->WheelVisualConfig.bUseWheelVisualOverrides = true;
	RuntimeVehicleData->WheelVisualConfig.ExpectedWheelCount = 4;
	RuntimeVehicleData->WheelVisualConfig.FrontWheelCountForSteering = 2;
	RuntimeVehicleData->WheelVisualConfig.bUseWheelSocketScale = true;
	RuntimeVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = false;

	// actual Wagon WSA authority와 동일한 Socket-authored visual scale입니다.
	const FVector WagonSocketScale(0.8, 1.0, 0.8);
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeScale = WagonSocketScale;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeScale = WagonSocketScale;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeScale = WagonSocketScale;
	RuntimeVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeScale = WagonSocketScale;

	VehiclePawn->VehicleData = RuntimeVehicleData;
	VehiclePawn->ApplyVehicleWheelVisualConfig();

	// Right fallback에만 적용할 local Roll180 compensation Quaternion입니다.
	const FQuat RightFallbackCompensation = FRotator(0.0f, 0.0f, 180.0f).Quaternion();

	// 첫 fallback Apply 뒤 기대하는 FR orientation입니다.
	const FQuat ExpectedFrontRightFallbackRotation = (AuthoredBaseRotations[1].Quaternion() * RightFallbackCompensation).GetNormalized();

	// 첫 fallback Apply 뒤 기대하는 RR orientation입니다.
	const FQuat ExpectedRearRightFallbackRotation = (AuthoredBaseRotations[3].Quaternion() * RightFallbackCompensation).GetNormalized();

	TestTrue(TEXT("FL fallback keeps authored orientation"), Wheels[0].Mesh->GetRelativeRotation().Quaternion().Equals(AuthoredBaseRotations[0].Quaternion(), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("RL fallback keeps authored orientation"), Wheels[2].Mesh->GetRelativeRotation().Quaternion().Equals(AuthoredBaseRotations[2].Quaternion(), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("FL SocketScale restores authored location"), Wheels[0].Mesh->GetRelativeLocation().Equals(AuthoredBaseLocations[0]));
	TestTrue(TEXT("FR SocketScale restores authored location"), Wheels[1].Mesh->GetRelativeLocation().Equals(AuthoredBaseLocations[1]));
	TestTrue(TEXT("FR FL-fallback gets local Roll180"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontRightFallbackRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("RR FL-fallback gets local Roll180"), Wheels[3].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedRearRightFallbackRotation, KINDA_SMALL_NUMBER));

	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(Wheels); ++WheelIndex)
	{
		TestTrue(*FString::Printf(TEXT("WSA-P0-07 Wheel %d uses shared FL mesh"), WheelIndex), Wheels[WheelIndex].Mesh->GetStaticMesh() == SharedWheelMesh);
		TestTrue(*FString::Printf(TEXT("WSA-P0-07 Wheel %d keeps Wagon Socket Scale"), WheelIndex), Wheels[WheelIndex].Mesh->GetRelativeScale3D().Equals(WagonSocketScale));
	}

	// 같은 VehicleData를 반복 Apply해도 Right orientation이 180→360으로 누적되지 않아야 합니다.
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestTrue(TEXT("FR repeated fallback Apply is idempotent"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontRightFallbackRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("RR repeated fallback Apply is idempotent"), Wheels[3].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedRearRightFallbackRotation, KINDA_SMALL_NUMBER));

	// 실제 WheelSync가 source-aware handedness를 소비하는지 검증할 component입니다.
	UCFWheelSyncComp* WheelSyncComp = VehiclePawn->GetWheelSyncComp();
	if (!TestNotNull(TEXT("WSA-P0-07 WheelSync component exists"), WheelSyncComp))
	{
		VehiclePawn->Destroy();
		return false;
	}

	WheelSyncComp->bEnableApplyTransformsInCpp = true;
	WheelSyncComp->bApplySpinPitchInCpp = true;
	if (!TestTrue(TEXT("WSA-P0-07 WheelSync prepares in fallback state"), WheelSyncComp->TryPrepareWheelSync()))
	{
		AddError(WheelSyncComp->LastValidationSummary);
		VehiclePawn->Destroy();
		return false;
	}

	// 네 바퀴에 동일한 vehicle-side absolute spin을 주는 입력입니다.
	TArray<FCFWheelVisualInput> WheelInputs;
	WheelInputs.SetNum(4);
	for (int32 WheelIndex = 0; WheelIndex < WheelInputs.Num(); ++WheelIndex)
	{
		WheelInputs[WheelIndex].WheelIndex = WheelIndex;
		WheelInputs[WheelIndex].bIsValidInput = true;
		WheelInputs[WheelIndex].SpinPitchDeg = 20.0f;
		WheelInputs[WheelIndex].SpinPitchDeltaDeg = 0.0f;
		WheelInputs[WheelIndex].bApplySpinPitchAsDelta = false;
	}

	TestTrue(TEXT("Fallback spin inputs apply"), WheelSyncComp->ApplyWheelVisualInputsPhase2(WheelInputs));

	// Left wheel의 +20 local spin 기대 회전입니다.
	const FQuat ExpectedFrontLeftSpinRotation = (AuthoredBaseRotations[0].Quaternion() * FRotator(20.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();

	// Right fallback은 Roll180 base 뒤 -20 local spin을 사용해야 같은 차량 기준 굴림 방향이 됩니다.
	const FQuat ExpectedFrontRightSpinRotation = (ExpectedFrontRightFallbackRotation * FRotator(-20.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();

	// Right rear fallback absolute spin도 Front와 동일한 -1 handedness를 사용해야 합니다.
	const FQuat ExpectedRearRightSpinRotation = (ExpectedRearRightFallbackRotation * FRotator(-20.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();

	TestTrue(TEXT("FL spin keeps +1 handedness"), Wheels[0].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontLeftSpinRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("FR fallback spin uses -1 handedness"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontRightSpinRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("RR fallback spin uses -1 handedness"), Wheels[3].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedRearRightSpinRotation, KINDA_SMALL_NUMBER));

	// production delta-spin 경로를 authored/fallback base에서 다시 검증하기 위해 Wheel Visual을 deterministic하게 재적용합니다.
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	if (!TestTrue(TEXT("WSA-P0-07 WheelSync re-prepares for delta spin"), WheelSyncComp->TryPrepareWheelSync()))
	{
		AddError(WheelSyncComp->LastValidationSummary);
		VehiclePawn->Destroy();
		return false;
	}

	for (int32 WheelIndex = 0; WheelIndex < WheelInputs.Num(); ++WheelIndex)
	{
		WheelInputs[WheelIndex].SpinPitchDeg = 0.0f;
		WheelInputs[WheelIndex].SpinPitchDeltaDeg = 7.0f;
		WheelInputs[WheelIndex].bApplySpinPitchAsDelta = true;
	}

	TestTrue(TEXT("Fallback delta spin inputs apply"), WheelSyncComp->ApplyWheelVisualInputsPhase2(WheelInputs));

	// production local AddLocalRotation에서 기대하는 Left +7도 결과입니다.
	const FQuat ExpectedFrontLeftDeltaSpinRotation = (AuthoredBaseRotations[0].Quaternion() * FRotator(7.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();

	// production local AddLocalRotation에서 기대하는 Right fallback -7도 결과입니다.
	const FQuat ExpectedFrontRightDeltaSpinRotation = (ExpectedFrontRightFallbackRotation * FRotator(-7.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();

	// production local AddLocalRotation에서 기대하는 Right rear fallback -7도 결과입니다.
	const FQuat ExpectedRearRightDeltaSpinRotation = (ExpectedRearRightFallbackRotation * FRotator(-7.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();

	TestTrue(TEXT("FL production delta spin keeps +1 handedness"), Wheels[0].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontLeftDeltaSpinRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("FR production delta spin uses -1 handedness"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontRightDeltaSpinRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("RR production delta spin uses -1 handedness"), Wheels[3].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedRearRightDeltaSpinRotation, KINDA_SMALL_NUMBER));

	// 이후 explicit Right 검증은 absolute 경로로 되돌립니다.
	for (int32 WheelIndex = 0; WheelIndex < WheelInputs.Num(); ++WheelIndex)
	{
		WheelInputs[WheelIndex].SpinPitchDeg = 20.0f;
		WheelInputs[WheelIndex].SpinPitchDeltaDeg = 0.0f;
		WheelInputs[WheelIndex].bApplySpinPitchAsDelta = false;
	}

	// explicit Right Mesh가 FL과 같은 pointer여도 source가 explicit이면 자동 반전/handedness 보정을 해제해야 합니다.
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFR = SharedWheelMesh;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshRR = SharedWheelMesh;
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestTrue(TEXT("Explicit FR same-pointer restores authored base"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(AuthoredBaseRotations[1].Quaternion(), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Explicit RR same-pointer restores authored base"), Wheels[3].Mesh->GetRelativeRotation().Quaternion().Equals(AuthoredBaseRotations[3].Quaternion(), KINDA_SMALL_NUMBER));

	if (!TestTrue(TEXT("WheelSync re-prepares after explicit Right transition"), WheelSyncComp->TryPrepareWheelSync()))
	{
		AddError(WheelSyncComp->LastValidationSummary);
		VehiclePawn->Destroy();
		return false;
	}
	TestTrue(TEXT("Explicit Right spin inputs apply"), WheelSyncComp->ApplyWheelVisualInputsPhase2(WheelInputs));

	// explicit FR은 authored base 뒤 +20 local spin을 사용해야 합니다.
	const FQuat ExpectedExplicitFrontRightSpinRotation = (AuthoredBaseRotations[1].Quaternion() * FRotator(20.0f, 0.0f, 0.0f).Quaternion()).GetNormalized();
	TestTrue(TEXT("Explicit FR uses +1 handedness"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedExplicitFrontRightSpinRotation, KINDA_SMALL_NUMBER));

	// explicit → fallback 재초기화에서도 authored base에서 정확히 1회만 Right compensation을 다시 적용해야 합니다.
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFR = nullptr;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshRR = nullptr;
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestTrue(TEXT("FR explicit-to-fallback reapplies exactly one Roll180"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedFrontRightFallbackRotation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("RR explicit-to-fallback reapplies exactly one Roll180"), Wheels[3].Mesh->GetRelativeRotation().Quaternion().Equals(ExpectedRearRightFallbackRotation, KINDA_SMALL_NUMBER));

	// Legacy AutoScale/AutoCenter가 authored location/scale을 변경하는 이전 차량 상태를 구성합니다.
	RuntimeVehicleData->WheelVisualConfig.bUseWheelSocketScale = false;
	RuntimeVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = true;
	RuntimeVehicleData->WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin = true;
	RuntimeVehicleData->VehicleMovementConfig.FrontWheelRadius = 30.0f;
	RuntimeVehicleData->VehicleMovementConfig.RearWheelRadius = 30.0f;
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestFalse(TEXT("Legacy AutoCenter changes FL authored location"), Wheels[0].Mesh->GetRelativeLocation().Equals(AuthoredBaseLocations[0]));

	// Legacy → SocketScale 전환 시 이전 center correction을 남기지 않고 authored location에서 현재 SocketScale만 적용해야 합니다.
	RuntimeVehicleData->WheelVisualConfig.bUseWheelSocketScale = true;
	RuntimeVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = false;
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestTrue(TEXT("Legacy-to-Socket restores FL authored location"), Wheels[0].Mesh->GetRelativeLocation().Equals(AuthoredBaseLocations[0]));
	TestTrue(TEXT("Legacy-to-Socket applies current Wagon scale"), Wheels[0].Mesh->GetRelativeScale3D().Equals(WagonSocketScale));

	// SocketScale → Manual 전환에서는 이전 0.8 scale이 남지 않고 authored base scale로 돌아가야 합니다.
	RuntimeVehicleData->WheelVisualConfig.bUseWheelSocketScale = false;
	RuntimeVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = false;
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestTrue(TEXT("Socket-to-Manual restores FL authored scale"), Wheels[0].Mesh->GetRelativeScale3D().Equals(AuthoredBaseScales[0]));
	TestTrue(TEXT("Socket-to-Manual keeps FL authored location"), Wheels[0].Mesh->GetRelativeLocation().Equals(AuthoredBaseLocations[0]));

	// Construction fresh recapture를 모사하기 위해 authored cache를 invalidate하고 새 SCS authored transform을 만든 뒤 재캡처합니다.
	VehiclePawn->InvalidateWheelVisualAuthoredBaseTransforms();
	// Construction 재실행 후 새로 authoring됐다고 가정할 FR transform입니다.
	const FTransform FreshConstructionFrontRightTransform(FRotator(0.0f, 11.0f, 13.0f), FVector(14.0, 15.0, 16.0), FVector(1.1, 1.2, 1.3));
	Wheels[1].Mesh->SetRelativeTransform(FreshConstructionFrontRightTransform);
	VehiclePawn->CaptureWheelVisualAuthoredBaseTransformsIfNeeded();
	RuntimeVehicleData->WheelVisualConfig.bUseWheelSocketScale = false;
	RuntimeVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = false;
	RuntimeVehicleData->VehicleVisualConfig.WheelMeshFR = SharedWheelMesh;
	VehiclePawn->ApplyVehicleWheelVisualConfig();
	TestTrue(TEXT("Construction recapture preserves fresh FR location"), Wheels[1].Mesh->GetRelativeLocation().Equals(FreshConstructionFrontRightTransform.GetLocation()));
	TestTrue(TEXT("Construction recapture preserves fresh FR rotation"), Wheels[1].Mesh->GetRelativeRotation().Quaternion().Equals(FreshConstructionFrontRightTransform.GetRotation(), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Construction recapture preserves fresh FR scale"), Wheels[1].Mesh->GetRelativeScale3D().Equals(FreshConstructionFrontRightTransform.GetScale3D()));

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
