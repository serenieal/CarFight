// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderHardpointIntegrityTests.cpp
// Version: v1.4.2
// Date: 2026-09-04
// Description: CF-FQ-047 Hardpoint authoring integrity + typed Physics receipt compatibility + actual Wagon PostLoad focused Automation입니다.
// Scope: transient Socket/semantic readback, post-commit outcome, legacy receipt-refresh boundary, P0-07A Exact/Equivalent/Stale/Blocked compatibility와 actual Wagon load-only invariant를 검증합니다.
// Changelog:
// - v1.4.2: Actual Wagon fixture를 exact1 개수 고정에서 확장 가능한 Recipe↔VehicleData semantic alignment + Top_01/Mount_Top_01 중복 금지 + legacy injection 금지 계약으로 교정. USER가 정상 추가한 2/2 이상 상태를 false failure로 취급하지 않음.
// - v1.4.1: Step 5 승인 직후 Step 7 Apply Pending 상태(Receipt == Prospective != Target)가 Exact/Complete를 유지하는 회귀 케이스를 추가.
// - v1.4.0: P0-07A Physics receipt compatibility의 Exact, Hardpoint-only, Mount-only, combined equivalent, provenance stale, Target drift, unrelated drift, empty/unknown blocked matrix를 추가.
// - v1.3.0: post-P0-06 중간검수 P1 회귀로 successful persistent mutation 뒤 refresh failure가 operation false-negative로 뒤집히지 않고 warning으로 분리되는 공통 outcome 계약을 추가.
// - v1.2.0: Physics receipt-only refresh가 current Target 불변 + Hardpoint/Mount-only pending diff에서만 허용되고 unrelated drift/no-diff를 차단하는 회귀를 추가.
// - v1.1.0: P0-06 actual DA_Vehicle_Wagon load에서 Standard Mount_Top_01이 존재할 때 legacy RoofTurret_MediumOrLarge가 PostLoad로 중복 생성되지 않는 회귀검증 추가.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "CFVehicleBuilderPresent.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleBuilderHardpointIntegrity.h"
#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleBuilderHardpointIntegrityTestsPrivate
{
	void AddSocket(UStaticMesh& Mesh, const FName SocketName, const FVector& Location)
	{
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(&Mesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = Location;
		Socket->RelativeRotation = FRotator::ZeroRotator;
		Socket->RelativeScale = FVector::OneVector;
		Mesh.AddSocket(Socket);
	}

	TArray<FName> StandardCategories()
	{
		return {
			TEXT("Top"),
			TEXT("Front"),
			TEXT("Back"),
			TEXT("LeftSide"),
			TEXT("RightSide"),
			TEXT("Bottom"),
			TEXT("Internal")
		};
	}

	FCFVehicleFieldDiff MakeStructuralDiff(
		const ECFVehicleDiffOp Operation,
		const FName CollectionName,
		const FName SelectorKey,
		const FName SelectorValue)
	{
		FCFVehicleFieldDiff Diff;
		Diff.Operation = Operation;
		Diff.FieldPath.CollectionPropertyName = CollectionName;
		Diff.FieldPath.SelectorKeyPropertyName = SelectorKey;
		Diff.FieldPath.SelectorKeyValue = SelectorValue;
		return Diff;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHardpointIntegrityInventoryTest,
	"CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.InventoryClassification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHardpointIntegritySemanticReadbackTest,
	"CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.SemanticReadback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHardpointIntegrityPresentationTest,
	"CarFight.DataAuthoring.CF_FQ_047.VBHAI_P0_04.PresentationReadback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHardpointIntegrityPhysicsReceiptBoundaryTest,
	"CarFight.DataAuthoring.CF_FQ_047.P0_06.PhysicsReceiptRefreshBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHardpointIntegrityPhysicsCompatibilityTest,
	"CarFight.DataAuthoring.CF_FQ_047.P0_07A.PhysicsReceiptCompatibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHAIPostCommitRefreshOutcomeTest,
	"CarFight.DataAuthoring.CF_FQ_047.MidReview.PostCommitRefreshOutcome",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBHardpointIntegrityActualWagonPostLoadTest,
	"CarFight.DataAuthoring.CF_FQ_047.P0_06.ActualWagon.PostLoadMountIntegrity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFVBHardpointIntegrityInventoryTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderHardpointIntegrityTestsPrivate;

	UStaticMesh* Mesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), NAME_None, RF_Transient);
	UCFVehicleData* Target = NewObject<UCFVehicleData>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!Mesh || !Recipe || !Target)
	{
		AddError(TEXT("VBHAI transient inventory fixture를 만들 수 없습니다."));
		return false;
	}

	AddSocket(*Mesh, TEXT("HP_Top_01"), FVector(10.0, 0.0, 20.0));
	AddSocket(*Mesh, TEXT("HP_Top_1"), FVector(11.0, 0.0, 20.0));
	AddSocket(*Mesh, TEXT("HP_Front_01"), FVector(20.0, 0.0, 20.0));
	AddSocket(*Mesh, TEXT("HP_Back_01"), FVector(-20.0, 0.0, 20.0));
	AddSocket(*Mesh, TEXT("HP_LeftSide_01"), FVector(0.0, -20.0, 20.0));
	AddSocket(*Mesh, TEXT("Wheel_Anchor_FL"), FVector(100.0, -50.0, 20.0));

	FCFHardpointIntent RecipeBound;
	RecipeBound.LocationSlotId = TEXT("Back_01");
	RecipeBound.LocationCategory = TEXT("Back");
	RecipeBound.SocketName = TEXT("HP_Back_01");
	Recipe->HardpointIntents.Add(RecipeBound);

	FCFHardpointIntent Collision;
	Collision.LocationSlotId = TEXT("Top_01");
	Collision.LocationCategory = TEXT("Top");
	Collision.SocketName = TEXT("HP_OtherTop_01");
	Recipe->HardpointIntents.Add(Collision);

	FCFVehicleHardpointSlot TargetBound;
	TargetBound.LocationSlotId = TEXT("LeftSide_01");
	TargetBound.SocketName = TEXT("HP_LeftSide_01");
	Target->HardpointSlots.Add(TargetBound);

	const int32 SocketCountBefore = Mesh->Sockets.Num();
	const int32 RecipeHardpointCountBefore = Recipe->HardpointIntents.Num();
	const int32 TargetHardpointCountBefore = Target->HardpointSlots.Num();
	const FVector FrontLocationBefore = Mesh->FindSocket(TEXT("HP_Front_01"))->RelativeLocation;

	FCFBuilderChassisSocketInventory Inventory;
	FString Error;
	TestTrue(
		TEXT("VBHAI inventory read succeeds"),
		FCFVehicleBuilderHardpointIntegrity::ReadChassisSocketInventory(
			Mesh,
			Recipe,
			Target,
			StandardCategories(),
			Inventory,
			Error));

	const FCFBuilderChassisSocketInventoryEntry* Front = Inventory.FindBySocketName(TEXT("HP_Front_01"));
	const FCFBuilderChassisSocketInventoryEntry* TopCanonical = Inventory.FindBySocketName(TEXT("HP_Top_01"));
	const FCFBuilderChassisSocketInventoryEntry* TopNonCanonical = Inventory.FindBySocketName(TEXT("HP_Top_1"));
	const FCFBuilderChassisSocketInventoryEntry* Back = Inventory.FindBySocketName(TEXT("HP_Back_01"));
	const FCFBuilderChassisSocketInventoryEntry* Left = Inventory.FindBySocketName(TEXT("HP_LeftSide_01"));
	const FCFBuilderChassisSocketInventoryEntry* Wheel = Inventory.FindBySocketName(TEXT("Wheel_Anchor_FL"));

	TestNotNull(TEXT("VBHAI canonical adoptable entry exists"), Front);
	TestNotNull(TEXT("VBHAI identity collision entry exists"), TopCanonical);
	TestNotNull(TEXT("VBHAI noncanonical entry exists"), TopNonCanonical);
	TestNotNull(TEXT("VBHAI recipe-bound entry exists"), Back);
	TestNotNull(TEXT("VBHAI target-bound entry exists"), Left);
	TestNotNull(TEXT("VBHAI unrelated entry exists"), Wheel);

	if (Front)
	{
		TestEqual(TEXT("VBHAI HP_Front_01 is adoptable"), Front->Classification, ECFBuilderHardpointSocketClassification::StandardAdoptable);
		TestEqual(TEXT("VBHAI HP_Front_01 parses Front_01"), Front->ParsedLocationSlotId, FName(TEXT("Front_01")));
	}
	if (TopCanonical)
	{
		TestEqual(TEXT("VBHAI Top identity collision is fail-closed"), TopCanonical->Classification, ECFBuilderHardpointSocketClassification::IdentityCollision);
	}
	if (TopNonCanonical)
	{
		TestEqual(TEXT("VBHAI HP_Top_1 is noncanonical"), TopNonCanonical->Classification, ECFBuilderHardpointSocketClassification::NonCanonicalHardpointLike);
	}
	if (Back)
	{
		TestEqual(TEXT("VBHAI HP_Back_01 is already recipe-bound"), Back->Classification, ECFBuilderHardpointSocketClassification::AlreadyRecipeBound);
	}
	if (Left)
	{
		TestEqual(TEXT("VBHAI HP_LeftSide_01 is already target-bound"), Left->Classification, ECFBuilderHardpointSocketClassification::AlreadyTargetBound);
	}
	if (Wheel)
	{
		TestEqual(TEXT("VBHAI wheel socket is unrelated"), Wheel->Classification, ECFBuilderHardpointSocketClassification::UnrelatedSocket);
	}

	TestEqual(TEXT("VBHAI inventory read does not add/remove sockets"), Mesh->Sockets.Num(), SocketCountBefore);
	TestEqual(TEXT("VBHAI inventory read does not mutate Recipe Hardpoints"), Recipe->HardpointIntents.Num(), RecipeHardpointCountBefore);
	TestEqual(TEXT("VBHAI inventory read does not mutate Target Hardpoints"), Target->HardpointSlots.Num(), TargetHardpointCountBefore);
	TestEqual(TEXT("VBHAI inventory read preserves existing Socket transform"), Mesh->FindSocket(TEXT("HP_Front_01"))->RelativeLocation, FrontLocationBefore);
	return true;
}

bool FCFVBHardpointIntegritySemanticReadbackTest::RunTest(const FString& Parameters)
{
	using namespace CFVehicleBuilderHardpointIntegrityTestsPrivate;

	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), NAME_None, RF_Transient);
	UCFVehicleData* Target = NewObject<UCFVehicleData>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!Recipe || !Target)
	{
		AddError(TEXT("VBHAI semantic fixture를 만들 수 없습니다."));
		return false;
	}

	FCFHardpointIntent AuthoredHardpoint;
	AuthoredHardpoint.LocationSlotId = TEXT("Top_01");
	AuthoredHardpoint.LocationCategory = TEXT("Top");
	AuthoredHardpoint.SocketName = TEXT("HP_Top_01");
	Recipe->HardpointIntents.Add(AuthoredHardpoint);

	FCFMountIntent AuthoredMount;
	AuthoredMount.MountProfileId = TEXT("Mount_Top_01");
	AuthoredMount.LocationSlotRef = TEXT("Top_01");
	AuthoredMount.MountType = ECFVehicleMountType::Turret;
	AuthoredMount.SizeLimit = ECFVehicleWeaponSize::Medium;
	Recipe->MountIntents.Add(AuthoredMount);

	FCFVehicleHardpointSlot CurrentHardpoint;
	CurrentHardpoint.LocationSlotId = TEXT("Legacy_01");
	CurrentHardpoint.SocketName = TEXT("HP_Legacy_01");
	Target->HardpointSlots.Add(CurrentHardpoint);

	FCFVehicleMountProfile CurrentMount;
	CurrentMount.MountProfileId = TEXT("Mount_Legacy_01");
	CurrentMount.LocationSlotRef = TEXT("Legacy_01");
	CurrentMount.MountType = ECFVehicleMountType::Fixed;
	CurrentMount.SizeLimit = ECFVehicleWeaponSize::Small;
	Target->MountProfiles.Add(CurrentMount);

	FCFBuilderFinalReviewResult Review;
	Review.Operation.Status = ECFAuthoringOpStatus::Succeeded;
	Review.FieldDiff.Add(MakeStructuralDiff(
		ECFVehicleDiffOp::AddArrayElement,
		TEXT("HardpointSlots"),
		TEXT("LocationSlotId"),
		TEXT("Top_01")));
	Review.FieldDiff.Add(MakeStructuralDiff(
		ECFVehicleDiffOp::AddArrayElement,
		TEXT("MountProfiles"),
		TEXT("MountProfileId"),
		TEXT("Mount_Top_01")));

	// 같은 element child diff는 prospective count를 중복 증가시키면 안 됩니다.
	FCFVehicleFieldDiff ChildDiff;
	ChildDiff.Operation = ECFVehicleDiffOp::SetLeaf;
	ChildDiff.FieldPath.CollectionPropertyName = TEXT("HardpointSlots");
	ChildDiff.FieldPath.SelectorKeyPropertyName = TEXT("LocationSlotId");
	ChildDiff.FieldPath.SelectorKeyValue = TEXT("Top_01");
	ChildDiff.FieldPath.PropertyChain = {TEXT("SocketName")};
	Review.FieldDiff.Add(ChildDiff);

	FCFBuilderHardpointSemanticFacts Facts;
	FString Error;
	TestTrue(
		TEXT("VBHAI semantic readback succeeds"),
		FCFVehicleBuilderHardpointIntegrity::BuildHardpointSemanticFacts(
			Recipe,
			Target,
			&Review,
			Facts,
			Error));

	TestTrue(TEXT("VBHAI semantic facts available"), Facts.bAvailable);
	TestTrue(TEXT("VBHAI current Target present"), Facts.bHasCurrentTarget);
	TestEqual(TEXT("VBHAI authored Hardpoint count"), Facts.AuthoredHardpointCount, 1);
	TestEqual(TEXT("VBHAI authored Mount count"), Facts.AuthoredMountCount, 1);
	TestEqual(TEXT("VBHAI current Target Hardpoint count"), Facts.CurrentTargetHardpointCount, 1);
	TestEqual(TEXT("VBHAI current Target Mount count"), Facts.CurrentTargetMountCount, 1);
	TestTrue(TEXT("VBHAI prospective counts available"), Facts.bHasProspectiveTargetCounts);
	TestEqual(TEXT("VBHAI prospective Hardpoint count"), Facts.ProspectiveTargetHardpointCount, 2);
	TestEqual(TEXT("VBHAI prospective Mount count"), Facts.ProspectiveTargetMountCount, 2);
	TestEqual(TEXT("VBHAI one authored relation"), Facts.AuthoredRelations.Num(), 1);
	if (Facts.AuthoredRelations.Num() == 1)
	{
		TestEqual(TEXT("VBHAI relation stable slot"), Facts.AuthoredRelations[0].LocationSlotId, FName(TEXT("Top_01")));
		TestEqual(TEXT("VBHAI relation Socket"), Facts.AuthoredRelations[0].SocketName, FName(TEXT("HP_Top_01")));
		TestEqual(TEXT("VBHAI relation mount count"), Facts.AuthoredRelations[0].MountProfileIds.Num(), 1);
		if (Facts.AuthoredRelations[0].MountProfileIds.Num() == 1)
		{
			TestEqual(TEXT("VBHAI relation mount id"), Facts.AuthoredRelations[0].MountProfileIds[0], FName(TEXT("Mount_Top_01")));
		}
	}

	FCFBuilderHardpointSemanticFacts NoTargetFacts;
	TestTrue(
		TEXT("VBHAI no-target Recipe facts still read"),
		FCFVehicleBuilderHardpointIntegrity::BuildHardpointSemanticFacts(
			Recipe,
			nullptr,
			nullptr,
			NoTargetFacts,
			Error));
	TestFalse(TEXT("VBHAI no-target does not imply 0/0 current Target"), NoTargetFacts.bHasCurrentTarget);
	return true;
}

bool FCFVBHardpointIntegrityPresentationTest::RunTest(const FString& Parameters)
{
	FCFBuilderHardpointSemanticFacts Facts;
	Facts.bAvailable = true;
	Facts.bHasCurrentTarget = true;
	Facts.AuthoredHardpointCount = 1;
	Facts.AuthoredMountCount = 1;
	Facts.CurrentTargetHardpointCount = 0;
	Facts.CurrentTargetMountCount = 0;
	Facts.bHasProspectiveTargetCounts = true;
	Facts.ProspectiveTargetHardpointCount = 1;
	Facts.ProspectiveTargetMountCount = 1;

	FCFBuilderHardpointMountRelationFact Relation;
	Relation.LocationSlotId = TEXT("Top_01");
	Relation.SocketName = TEXT("HP_Top_01");
	Relation.MountProfileIds.Add(TEXT("Mount_Top_01"));
	Facts.AuthoredRelations.Add(Relation);

	const FString Step6 = FCFVehicleBuilderPresentation::BuildMountSemanticSummary(Facts).ToString();
	const FString Step7 = FCFVehicleBuilderPresentation::BuildFinalReviewHardpointReadback(Facts).ToString();
	const FString Step8 = FCFVehicleBuilderPresentation::BuildDrivingHardpointReadback(Facts).ToString();

	TestTrue(TEXT("VBHAI Step6 shows exact relation"), Step6.Contains(TEXT("Top_01 → HP_Top_01 → Mount_Top_01")));
	TestTrue(TEXT("VBHAI Step6 says authored record is not VehicleData"), Step6.Contains(TEXT("실제 VehicleData 반영 여부")));
	TestTrue(TEXT("VBHAI Step7 shows current 0/0"), Step7.Contains(TEXT("현재 VehicleData: 장착 위치 0 / 장착 규칙 0")));
	TestTrue(TEXT("VBHAI Step7 shows prospective 1/1"), Step7.Contains(TEXT("변경 적용 후: 장착 위치 1 / 장착 규칙 1")));
	TestTrue(TEXT("VBHAI Step7 explains prospective is not yet saved current"), Step7.Contains(TEXT("VehicleData에 저장된 값이 아닙니다")));
	TestTrue(TEXT("VBHAI Step8 shows current Target only"), Step8.Contains(TEXT("장착 위치 0 / 장착 규칙 0")));
	TestTrue(TEXT("VBHAI Step8 explains current Target authority"), Step8.Contains(TEXT("현재 VehicleData에서 읽은 값")));
	TestFalse(TEXT("VBHAI Step8 does not show prospective 1/1"), Step8.Contains(TEXT("장착 위치 1 / 장착 규칙 1")));

	FCFBuilderHardpointSemanticFacts MissingTargetFacts;
	MissingTargetFacts.bAvailable = true;
	const FString MissingStep8 = FCFVehicleBuilderPresentation::BuildDrivingHardpointReadback(MissingTargetFacts).ToString();
	TestTrue(TEXT("VBHAI missing Target is not rendered as zero"), MissingStep8.Contains(TEXT("읽을 수 없습니다")));
	return true;
}

// Physics receipt-only refresh가 current Target 불변 + Hardpoint/Mount-only pending diff에만 허용되는지 pure boundary로 검증합니다.
bool FCFVBHardpointIntegrityPhysicsReceiptBoundaryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFVehicleBuilderHardpointIntegrityTestsPrivate;

	TArray<FCFVehicleFieldDiff> HardpointMountDiff;
	HardpointMountDiff.Add(MakeStructuralDiff(
		ECFVehicleDiffOp::AddArrayElement,
		TEXT("HardpointSlots"),
		TEXT("LocationSlotId"),
		TEXT("Top_01")));
	HardpointMountDiff.Add(MakeStructuralDiff(
		ECFVehicleDiffOp::AddArrayElement,
		TEXT("MountProfiles"),
		TEXT("MountProfileId"),
		TEXT("Mount_Top_01")));

	FString Error;
	TestTrue(
		TEXT("P0-06 receipt refresh allows exact Target + Hardpoint/Mount-only drift"),
		FCFVehicleBuilderHardpointIntegrity::ValidatePhysicsReceiptRefreshBoundary(
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			HardpointMountDiff,
			Error));
	TestTrue(TEXT("P0-06 allowed receipt refresh has no error"), Error.IsEmpty());

	TestFalse(
		TEXT("P0-06 current Target drift blocks receipt-only refresh"),
		FCFVehicleBuilderHardpointIntegrity::ValidatePhysicsReceiptRefreshBoundary(
			TEXT("old_hash"),
			TEXT("different_target_hash"),
			TEXT("new_hash"),
			HardpointMountDiff,
			Error));
	TestTrue(TEXT("P0-06 Target drift diagnostic"), Error.Contains(TEXT("Current Target Definition")));

	TArray<FCFVehicleFieldDiff> UnrelatedDiff = HardpointMountDiff;
	FCFVehicleFieldDiff DurabilityDiff;
	DurabilityDiff.Operation = ECFVehicleDiffOp::SetLeaf;
	DurabilityDiff.FieldPath.PropertyChain = {TEXT("VehicleDurabilityConfig"), TEXT("MaxHealth")};
	UnrelatedDiff.Add(DurabilityDiff);
	TestFalse(
		TEXT("P0-06 unrelated Definition diff blocks receipt-only refresh"),
		FCFVehicleBuilderHardpointIntegrity::ValidatePhysicsReceiptRefreshBoundary(
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			UnrelatedDiff,
			Error));
	TestTrue(TEXT("P0-06 unrelated drift diagnostic"), Error.Contains(TEXT("Hardpoint/Mount 이외")));

	TestFalse(
		TEXT("P0-06 no resolved drift does not refresh receipt"),
		FCFVehicleBuilderHardpointIntegrity::ValidatePhysicsReceiptRefreshBoundary(
			TEXT("same_hash"),
			TEXT("same_hash"),
			TEXT("same_hash"),
			HardpointMountDiff,
			Error));
	TestTrue(TEXT("P0-06 no-drift diagnostic"), Error.Contains(TEXT("재검증이 필요하지 않습니다")));

	TArray<FCFVehicleFieldDiff> EmptyDiff;
	TestFalse(
		TEXT("P0-06 hash drift without pending diff is fail-closed"),
		FCFVehicleBuilderHardpointIntegrity::ValidatePhysicsReceiptRefreshBoundary(
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			EmptyDiff,
			Error));
	TestTrue(TEXT("P0-06 empty diff diagnostic"), Error.Contains(TEXT("pending FieldDiff")));
	return true;
}

// Step 5 Physics receipt가 full Definition drift와 Physics semantic drift를 분리해 판정하는지 pure typed contract로 검증합니다.
bool FCFVBHardpointIntegrityPhysicsCompatibilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFVehicleBuilderHardpointIntegrityTestsPrivate;

	// HardpointSlots만 추가되는 current structural diff입니다.
	TArray<FCFVehicleFieldDiff> HardpointOnlyDiff;
	HardpointOnlyDiff.Add(MakeStructuralDiff(
		ECFVehicleDiffOp::AddArrayElement,
		TEXT("HardpointSlots"),
		TEXT("LocationSlotId"),
		TEXT("Top_01")));

	// MountProfiles만 추가되는 current structural diff입니다.
	TArray<FCFVehicleFieldDiff> MountOnlyDiff;
	MountOnlyDiff.Add(MakeStructuralDiff(
		ECFVehicleDiffOp::AddArrayElement,
		TEXT("MountProfiles"),
		TEXT("MountProfileId"),
		TEXT("Mount_Top_01")));

	// HardpointSlots와 MountProfiles가 함께 변경되는 current structural diff입니다.
	TArray<FCFVehicleFieldDiff> HardpointMountDiff = HardpointOnlyDiff;
	HardpointMountDiff.Append(MountOnlyDiff);

	// Full Definition identity까지 모두 exact인 Step 5 compatibility입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult Exact =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("same_hash"),
			TEXT("same_hash"),
			TEXT("same_hash"),
			TArray<FCFVehicleFieldDiff>());
	TestEqual(TEXT("P0-07A exact receipt classification"), Exact.Compatibility, ECFBuilderPhysicsReceiptCompatibility::Exact);
	TestTrue(TEXT("P0-07A exact receipt keeps Step5 complete"), Exact.IsComplete());

	// Step 5 승인 직후 Target Apply 전에는 receipt/prospective만 같고 current Target은 이전 hash일 수 있습니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult ApplyPendingExact =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("approved_new_hash"),
			TEXT("old_target_hash"),
			TEXT("approved_new_hash"),
			TArray<FCFVehicleFieldDiff>());
	TestEqual(TEXT("P0-07A Step7 apply-pending receipt remains exact"), ApplyPendingExact.Compatibility, ECFBuilderPhysicsReceiptCompatibility::Exact);
	TestTrue(TEXT("P0-07A Step7 apply-pending receipt keeps Step5 complete"), ApplyPendingExact.IsComplete());

	// Hardpoint-only downstream structural drift compatibility입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult HardpointEquivalent =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			HardpointOnlyDiff);
	TestEqual(TEXT("P0-07A Hardpoint-only drift is equivalent"), HardpointEquivalent.Compatibility, ECFBuilderPhysicsReceiptCompatibility::PhysicsEquivalentStructuralDrift);
	TestTrue(TEXT("P0-07A Hardpoint-only drift keeps Step5 complete"), HardpointEquivalent.IsComplete());

	// Mount-only downstream structural drift compatibility입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult MountEquivalent =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			MountOnlyDiff);
	TestEqual(TEXT("P0-07A Mount-only drift is equivalent"), MountEquivalent.Compatibility, ECFBuilderPhysicsReceiptCompatibility::PhysicsEquivalentStructuralDrift);
	TestTrue(TEXT("P0-07A Mount-only drift keeps Step5 complete"), MountEquivalent.IsComplete());

	// Hardpoint+Mount downstream structural drift compatibility입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult CombinedEquivalent =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			HardpointMountDiff);
	TestEqual(TEXT("P0-07A Hardpoint+Mount drift is equivalent"), CombinedEquivalent.Compatibility, ECFBuilderPhysicsReceiptCompatibility::PhysicsEquivalentStructuralDrift);

	// Evidence/Profile/Transmission/Engine/Resolver provenance가 달라진 fail-closed 결과입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult ProvenanceStale =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			false,
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			HardpointOnlyDiff,
			TEXT("provenance drift"));
	TestEqual(TEXT("P0-07A provenance drift is stale"), ProvenanceStale.Compatibility, ECFBuilderPhysicsReceiptCompatibility::PhysicsStale);
	TestFalse(TEXT("P0-07A provenance drift does not keep Step5 complete"), ProvenanceStale.IsComplete());
	TestEqual(TEXT("P0-07A provenance diagnostic preserved"), ProvenanceStale.Diagnostic, FString(TEXT("provenance drift")));

	// Receipt baseline과 current Target이 달라진 결과입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult TargetStale =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("old_hash"),
			TEXT("different_target_hash"),
			TEXT("new_hash"),
			HardpointOnlyDiff);
	TestEqual(TEXT("P0-07A Target drift is stale"), TargetStale.Compatibility, ECFBuilderPhysicsReceiptCompatibility::PhysicsStale);

	// Physics 비영향 allowlist 밖의 Definition 변경입니다.
	TArray<FCFVehicleFieldDiff> UnrelatedDiff = HardpointOnlyDiff;
	// 비-Hardpoint/Mount top-level leaf 변경 fixture입니다.
	FCFVehicleFieldDiff DurabilityDiff;
	DurabilityDiff.Operation = ECFVehicleDiffOp::SetLeaf;
	DurabilityDiff.FieldPath.PropertyChain = {TEXT("VehicleDurabilityConfig"), TEXT("MaxHealth")};
	UnrelatedDiff.Add(DurabilityDiff);
	// Unrelated Definition drift의 compatibility입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult UnrelatedStale =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			UnrelatedDiff);
	TestEqual(TEXT("P0-07A unrelated Definition drift is stale"), UnrelatedStale.Compatibility, ECFBuilderPhysicsReceiptCompatibility::PhysicsStale);

	// Hash는 달라졌지만 authoritative pending diff가 없는 fail-closed 결과입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult MissingDiffBlocked =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			TArray<FCFVehicleFieldDiff>());
	TestEqual(TEXT("P0-07A missing structural evidence is blocked"), MissingDiffBlocked.Compatibility, ECFBuilderPhysicsReceiptCompatibility::Blocked);

	// Definition identity 자체가 비어 있는 fail-closed 결과입니다.
	const FCFBuilderPhysicsReceiptCompatibilityResult MissingIdentityBlocked =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsReceiptCompatibility(
			true,
			FString(),
			TEXT("target_hash"),
			TEXT("new_hash"),
			HardpointOnlyDiff);
	TestEqual(TEXT("P0-07A missing identity is blocked"), MissingIdentityBlocked.Compatibility, ECFBuilderPhysicsReceiptCompatibility::Blocked);

	// Legacy bool refresh와 Step5 compatibility가 공유하는 single structural boundary 결과입니다.
	const FCFBuilderPhysicsStructuralDriftBoundaryResult StructuralEquivalent =
		FCFVehicleBuilderHardpointIntegrity::EvaluatePhysicsStructuralDriftBoundary(
			TEXT("old_hash"),
			TEXT("old_hash"),
			TEXT("new_hash"),
			HardpointMountDiff);
	TestEqual(TEXT("P0-07A structural boundary recognizes Hardpoint+Mount only"), StructuralEquivalent.Boundary, ECFBuilderPhysicsStructuralDriftBoundary::Equivalent);

	return true;
}

// successful persistent mutation 뒤 refresh failure가 operation false-negative로 바뀌지 않는 공통 outcome 계약을 검증합니다.
bool FCFVBHAIPostCommitRefreshOutcomeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// existing Socket adoption의 forced refresh failure 목록입니다.
	TArray<FString> AdoptionRefreshFailures;
	AdoptionRefreshFailures.Add(TEXT("현재 상태 다시 읽기: forced adoption refresh failure"));
	// stale error 초기값이 successful outcome에서 제거되는지 확인합니다.
	FString Error = TEXT("stale error");
	// non-fatal warning 결과입니다.
	FString Warning;
	// existing Socket adoption와 동일한 successful-mutation terminal outcome입니다.
	const bool bAdoptionOutcome = FCFVehicleBuilderVM::FinalizeSuccessfulMutationRefresh(
		AdoptionRefreshFailures,
		Error,
		Warning);

	TestTrue(TEXT("Mid-review adoption persistent success stays true after refresh failure"), bAdoptionOutcome);
	TestTrue(TEXT("Mid-review adoption success clears OutError"), Error.IsEmpty());
	TestTrue(TEXT("Mid-review adoption refresh failure becomes warning"), Warning.Contains(TEXT("forced adoption refresh failure")));

	// Physics/Profile commit의 forced follow-up refresh failures입니다.
	TArray<FString> PhysicsRefreshFailures;
	PhysicsRefreshFailures.Add(TEXT("Vehicle preview 다시 읽기: forced physics preview failure"));
	PhysicsRefreshFailures.Add(TEXT("기준 정보 다시 읽기: forced evidence refresh failure"));
	Error = TEXT("stale error");
	Warning.Reset();
	// Profile/receipt commit와 동일한 successful-mutation terminal outcome입니다.
	const bool bPhysicsOutcome = FCFVehicleBuilderVM::FinalizeSuccessfulMutationRefresh(
		PhysicsRefreshFailures,
		Error,
		Warning);

	TestTrue(TEXT("Mid-review physics persistent success stays true after refresh failures"), bPhysicsOutcome);
	TestTrue(TEXT("Mid-review physics success clears OutError"), Error.IsEmpty());
	TestTrue(TEXT("Mid-review physics warning keeps preview failure"), Warning.Contains(TEXT("forced physics preview failure")));
	TestTrue(TEXT("Mid-review physics warning keeps evidence failure"), Warning.Contains(TEXT("forced evidence refresh failure")));

	// refresh failure가 없는 clean success 경우입니다.
	TArray<FString> NoRefreshFailures;
	Error = TEXT("stale error");
	Warning = TEXT("stale warning");
	// clean success terminal outcome입니다.
	const bool bCleanOutcome = FCFVehicleBuilderVM::FinalizeSuccessfulMutationRefresh(
		NoRefreshFailures,
		Error,
		Warning);

	TestTrue(TEXT("Mid-review clean success remains true"), bCleanOutcome);
	TestTrue(TEXT("Mid-review clean success clears OutError"), Error.IsEmpty());
	TestTrue(TEXT("Mid-review clean success clears stale warning"), Warning.IsEmpty());
	return true;
}

// Actual Wagon persisted Standard Mount가 load-time legacy migration으로 중복되지 않는지 Save 0으로 검증합니다.
bool FCFVBHardpointIntegrityActualWagonPostLoadTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	const TCHAR* WagonPath = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon");
	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");
	UCFVehicleData* Wagon = LoadObject<UCFVehicleData>(nullptr, WagonPath);
	UCFVehicleRecipeData* WagonRecipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipePath);
	if (!TestNotNull(TEXT("P0-06 actual Wagon loads"), Wagon)
		|| !TestNotNull(TEXT("P0-06 actual Wagon Recipe loads"), WagonRecipe))
	{
		return false;
	}

	TestTrue(TEXT("P0-06 Wagon keeps at least one authored hardpoint after PostLoad"), Wagon->HardpointSlots.Num() >= 1);
	TestTrue(TEXT("P0-06 Wagon keeps at least one authored mount after PostLoad"), Wagon->MountProfiles.Num() >= 1);
	TestEqual(TEXT("P0-06 Wagon Recipe/Target hardpoint counts stay aligned"), WagonRecipe->HardpointIntents.Num(), Wagon->HardpointSlots.Num());
	TestEqual(TEXT("P0-06 Wagon Recipe/Target mount counts stay aligned"), WagonRecipe->MountIntents.Num(), Wagon->MountProfiles.Num());

	// Historical canonical Top_01 hardpoint가 중복 없이 몇 개 존재하는지 셉니다.
	int32 CanonicalTopHardpointCount = 0;
	for (const FCFVehicleHardpointSlot& Hardpoint : Wagon->HardpointSlots)
	{
		if (Hardpoint.LocationSlotId == TEXT("Top_01") && Hardpoint.SocketName == TEXT("HP_Top_01"))
		{
			++CanonicalTopHardpointCount;
		}
	}
	TestEqual(TEXT("P0-06 canonical Top_01 -> HP_Top_01 remains exactly once"), CanonicalTopHardpointCount, 1);

	// Historical canonical Mount_Top_01 mount가 중복 없이 몇 개 존재하는지 셉니다.
	int32 CanonicalTopMountCount = 0;
	for (const FCFVehicleMountProfile& Mount : Wagon->MountProfiles)
	{
		if (Mount.MountProfileId == TEXT("Mount_Top_01")
			&& Mount.LocationSlotRef == TEXT("Top_01")
			&& Mount.MountType == ECFVehicleMountType::Turret
			&& Mount.SizeLimit == ECFVehicleWeaponSize::Large)
		{
			++CanonicalTopMountCount;
		}
	}
	TestEqual(TEXT("P0-06 canonical Mount_Top_01 remains exactly once"), CanonicalTopMountCount, 1);

	// Recipe의 모든 HardpointIntent가 current Target의 동일 ID/Socket hardpoint로 실제 적용됐는지 확인합니다.
	for (const FCFHardpointIntent& Intent : WagonRecipe->HardpointIntents)
	{
		// 현재 Intent와 semantic이 같은 Target hardpoint 개수입니다.
		int32 MatchingTargetCount = 0;
		for (const FCFVehicleHardpointSlot& Hardpoint : Wagon->HardpointSlots)
		{
			if (Hardpoint.LocationSlotId == Intent.LocationSlotId && Hardpoint.SocketName == Intent.SocketName)
			{
				++MatchingTargetCount;
			}
		}
		TestEqual(*FString::Printf(TEXT("P0-06 Recipe hardpoint %s resolves exactly once in Target"), *Intent.LocationSlotId.ToString()), MatchingTargetCount, 1);
	}

	// Recipe의 모든 MountIntent가 current Target의 동일 semantic mount로 실제 적용됐는지 확인합니다.
	for (const FCFMountIntent& Intent : WagonRecipe->MountIntents)
	{
		// 현재 Intent와 semantic이 같은 Target mount 개수입니다.
		int32 MatchingTargetCount = 0;
		for (const FCFVehicleMountProfile& Mount : Wagon->MountProfiles)
		{
			if (Mount.MountProfileId == Intent.MountProfileId
				&& Mount.LocationSlotRef == Intent.LocationSlotRef
				&& Mount.MountType == Intent.MountType
				&& Mount.SizeLimit == Intent.SizeLimit)
			{
				++MatchingTargetCount;
			}
		}
		TestEqual(*FString::Printf(TEXT("P0-06 Recipe mount %s resolves exactly once in Target"), *Intent.MountProfileId.ToString()), MatchingTargetCount, 1);
	}

	TestFalse(
		TEXT("P0-06 legacy RoofTurret migration is not injected beside Standard mount"),
		Wagon->MountProfiles.ContainsByPredicate([](const FCFVehicleMountProfile& Mount)
		{
			return Mount.MountProfileId == TEXT("RoofTurret_MediumOrLarge");
		}));

	TestEqual(TEXT("P0-06 Wagon Recipe mode is UseHardpoints"), WagonRecipe->BuilderHardpointPlanMode, ECFBuilderHardpointPlanMode::UseHardpoints);

	FCFVehicleDefinitionSnapshot CurrentTargetSnapshot;
	FString SnapshotError;
	if (TestTrue(
		TEXT("P0-06 actual Wagon full Target snapshot builds"),
		FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Wagon, CurrentTargetSnapshot, SnapshotError)))
	{
		TestFalse(TEXT("P0-06 actual Wagon current DefinitionHash is non-empty"), CurrentTargetSnapshot.DefinitionHash.IsEmpty());
		TestEqual(
			TEXT("P0-06 Recipe AppliedState matches current full Target DefinitionHash"),
			WagonRecipe->AppliedState.AppliedDefinitionHash,
			CurrentTargetSnapshot.DefinitionHash);
	}
	else
	{
		AddError(SnapshotError);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
