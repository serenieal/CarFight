// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWheelSizeTests.cpp
// Version: v1.1.0
// Date: 2026-08-28
// Description: CF-FQ-040 WSA-P0-01 schema/helper + WSA-P0-03 Runtime Visual/WheelSync focused Automation입니다.
// Scope: additive/legacy 호환, Socket-authored Wheel_Mesh exact scale, FL mesh fallback, Wheel_Anchor scale 비소유와 WheelSync transform 회귀를 검증합니다.
// Changelog:
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

#endif // WITH_DEV_AUTOMATION_TESTS
