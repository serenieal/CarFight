// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADaceCorrTests.cpp
// Version: v1.1.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-04 per-TypeKey DACE boundary와 generic observation focused Automation입니다.
// Changelog:
// - v1.1.0: Ammo independent bootstrap 이후 ContractReady/current migration 상태로 regression을 전진시키고 DACE-AmmoData history authority를 직접 확인합니다.
// - v1.0.1: ContractNotReady Ammo가 exact0 canonical compatibility만으로 accepted append/Current promotion을 통과하지 못하는 provider-aware migration gate regression을 추가했습니다.
// - v1.0.0: DACE readiness/target-set isolation, Ammo exact0, cross-TypeKey history 차단, Ammo exact8 direct Reflection, AmmoTags[] element observation, AmmoIcon SoftObject target-class observation을 추가했습니다.
// Migration:
// - 이 파일은 memory-only descriptor/JSON fixture와 native class Reflection/accepted constant만 사용합니다. Product Staging/UObject, HeavyFinite/RocketFinite를 생성·수정·저장하지 않습니다.

#include "CFDAContractGuard.h"
#include "CFDAAmmoDace.h"
#include "CFDAAmmoProvider.h"
#include "CFDAMissileProvider.h"
#include "CFAmmoData.h"

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADaceTypeBoundaryCorrectionTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_04.DaceTypeBoundaryCorrection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADaceObservationCorrectionTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_04.DaceObservationCorrection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Missile/Ammo DACE readiness와 canonical target set이 exact TypeKey별로 독립이며 Ammo exact0이 valid empty set인지 검증합니다.
bool FCFDADaceTypeBoundaryCorrectionTest::RunTest(const FString& Parameters)
{
	// Existing accepted Missile provider입니다.
	const FCFDATypeProviderEntry& MissileProvider = CFDAMissileProvider::GetProvider();
	// Independent accepted bootstrap을 보유한 current Ammo provider입니다.
	const FCFDATypeProviderEntry& AmmoProvider = CFDAAmmoProvider::GetProvider();

	TestEqual(TEXT("Missile DACE must remain ContractReady"), MissileProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestTrue(TEXT("Missile canonical target set must be explicitly declared"), MissileProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Missile canonical target set must remain exact3"), MissileProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 3);
	TestEqual(TEXT("Missile accepted history namespace must remain unchanged"), MissileProvider.Descriptor.DaceAcceptedHistoryNamespace, FString(TEXT("DACE-MissileGuidePreset")));

	TestEqual(TEXT("Ammo authoring must remain ReviewedMutationReady"), AmmoProvider.Readiness, ECFDAProviderReadiness::ReviewedMutationReady);
	TestEqual(TEXT("Ammo DACE must be ContractReady after independent bootstrap"), AmmoProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestEqual(TEXT("Ammo DACE owner must be independent"), AmmoProvider.Descriptor.DaceContractOwnerName, FName(TEXT("CFDAAmmoDace")));
	TestTrue(TEXT("Ammo canonical target set must be explicitly declared"), AmmoProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Ammo Product canonical target set must remain exact0"), AmmoProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);
	TestEqual(TEXT("Ammo accepted history namespace must remain independent"), AmmoProvider.Descriptor.DaceAcceptedHistoryNamespace, FString(TEXT("DACE-AmmoData")));

	// Declared empty exact0 target set의 read-only compatibility 결과입니다.
	const FCFDAContractGuardResult AmmoExactZeroCompatibility = FCFDAContractGuard::ValidateCanonicalStagingCompatibilityForProvider(AmmoProvider);
	TestTrue(TEXT("Declared Ammo exact0 canonical target set must be valid compatibility state"), AmmoExactZeroCompatibility.bPassed);
	// Accepted baseline과 current descriptor가 동일한 Ammo provider-aware migration gate입니다.
	const FCFDAMigrationGateResult AmmoCurrentGate = FCFDAAmmoDace::EvaluateCurrentMigrationGate();
	TestTrue(TEXT("Current Ammo DACE migration gate must pass with exact0 canonical set"), AmmoCurrentGate.Validation.bPassed);
	TestFalse(TEXT("No-delta current Ammo contract must not append duplicate accepted history"), AmmoCurrentGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Accepted current Ammo DACE contract may project Current state"), AmmoCurrentGate.bCurrentSystemPromotionAllowed);
	TestEqual(TEXT("Ammo independent accepted history must remain bootstrap exact1"), FCFDAAmmoDace::GetAcceptedSnapshots().Num(), 1);

	// Existing protected Missile accepted chain을 provider-local boundary로 검증한 결과입니다.
	const FCFDAContractGuardResult MissileHistoryResult = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(MissileProvider.Descriptor, FCFDAContractGuard::GetAcceptedSnapshots());
	TestTrue(TEXT("Existing Missile accepted history must remain valid under provider-local boundary"), MissileHistoryResult.bPassed);

	// Missile records를 current Ammo provider history로 잘못 전달한 결과입니다.
	const FCFDAContractGuardResult CrossTypeHistoryResult = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(AmmoProvider.Descriptor, FCFDAContractGuard::GetAcceptedSnapshots());
	TestFalse(TEXT("Missile accepted records must never validate as Ammo history"), CrossTypeHistoryResult.bPassed);
	TestTrue(TEXT("Cross-TypeKey history contamination must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(CrossTypeHistoryResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));
	return true;
}

// Ammo direct Reflection과 Array element JSON observer가 P1-2/P1-3 correction contract를 실제로 관측하는지 검증합니다.
bool FCFDADaceObservationCorrectionTest::RunTest(const FString& Parameters)
{
	// CFAmmoData direct authored Reflection exact8입니다.
	TArray<FCFDASourceFieldDescriptor> AmmoSourceDescriptors;
	// Reflection 실패 사유입니다.
	FString ReflectionError;
	TestTrue(TEXT("Ammo direct Reflection must succeed"), FCFDAContractGuard::BuildDirectReflectedSourceShapeDescriptor(*UCFAmmoData::StaticClass(), AmmoSourceDescriptors, ReflectionError));
	if (!ReflectionError.IsEmpty())
	{
		AddError(ReflectionError);
		return false;
	}
	TestEqual(TEXT("CFAmmoData direct authored Reflection must remain exact8"), AmmoSourceDescriptors.Num(), 8);

	// AmmoTags reflected descriptor입니다.
	const FCFDASourceFieldDescriptor* AmmoTagsDescriptor = AmmoSourceDescriptors.FindByPredicate([](const FCFDASourceFieldDescriptor& Descriptor)
	{
		return Descriptor.SourcePropertyPath.Equals(TEXT("AmmoTags"), ESearchCase::CaseSensitive);
	});
	TestNotNull(TEXT("AmmoTags reflected descriptor must exist"), AmmoTagsDescriptor);
	if (AmmoTagsDescriptor != nullptr)
	{
		TestEqual(TEXT("AmmoTags property kind must be Array"), AmmoTagsDescriptor->PropertyKind, FString(TEXT("Array")));
		TestEqual(TEXT("AmmoTags inner reflected type must remain FName"), AmmoTagsDescriptor->ReflectedTypePath, FString(TEXT("Array<Name>")));
		TestEqual(TEXT("AmmoTags container kind must be Array"), AmmoTagsDescriptor->ContainerKind, FString(TEXT("Array")));
	}

	// AmmoIcon reflected descriptor입니다.
	const FCFDASourceFieldDescriptor* AmmoIconDescriptor = AmmoSourceDescriptors.FindByPredicate([](const FCFDASourceFieldDescriptor& Descriptor)
	{
		return Descriptor.SourcePropertyPath.Equals(TEXT("AmmoIcon"), ESearchCase::CaseSensitive);
	});
	TestNotNull(TEXT("AmmoIcon reflected descriptor must exist"), AmmoIconDescriptor);
	if (AmmoIconDescriptor != nullptr)
	{
		TestEqual(TEXT("AmmoIcon property kind must expose SoftObject semantics"), AmmoIconDescriptor->PropertyKind, FString(TEXT("SoftObject")));
		TestEqual(TEXT("AmmoIcon target class must remain UTexture2D"), AmmoIconDescriptor->ReflectedTypePath, FString(TEXT("/Script/Engine.Texture2D")));
		TestEqual(TEXT("AmmoIcon must remain scalar"), AmmoIconDescriptor->ContainerKind, FString(TEXT("Scalar")));
	}

	// Array element synthesized observation용 minimal adapter descriptor입니다.
	const TArray<FCFDAAdapterFieldDescriptor> ArrayAdapterDescriptors =
	{
		{TEXT("Payload"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("WholeRecordObject"), TEXT("PayloadContainer")},
		{TEXT("Payload.AmmoTags"), TEXT("Array"), TEXT("Required"), TEXT("NonNull"), TEXT("FNameTokenArray"), TEXT("PayloadContainer")},
		{TEXT("Payload.AmmoTags[]"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("FNameToken"), TEXT("Payload")}
	};
	// Homogeneous non-empty String array fixture입니다.
	const FCFDAContractGuardResult StringArrayResult = FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(TEXT("{\"Payload\":{\"AmmoTags\":[\"AP\",\"HE\"]}}"), ArrayAdapterDescriptors);
	TestTrue(TEXT("Non-empty AmmoTags String array must expose matching [] element observation"), StringArrayResult.bPassed);
	// Number element로 drift한 memory-only fixture입니다.
	const FCFDAContractGuardResult WrongElementTypeResult = FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(TEXT("{\"Payload\":{\"AmmoTags\":[1,2]}}"), ArrayAdapterDescriptors);
	TestFalse(TEXT("AmmoTags numeric elements must fail String element descriptor"), WrongElementTypeResult.bPassed);
	TestTrue(TEXT("Wrong AmmoTags element type must report SerializerCoverageMismatch"), FCFDAContractGuard::HasIssueCode(WrongElementTypeResult, ECFDAContractIssueCode::SerializerCoverageMismatch));
	// Empty array는 required element shape 증거를 제공하지 못하는 fixture입니다.
	const FCFDAContractGuardResult EmptyArrayResult = FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(TEXT("{\"Payload\":{\"AmmoTags\":[]}}"), ArrayAdapterDescriptors);
	TestFalse(TEXT("Empty array cannot satisfy required [] element observation probe"), EmptyArrayResult.bPassed);
	return true;
}

#endif
