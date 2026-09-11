// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseDaceTests.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-02 VehicleDefense DACE descriptor/production probe/bootstrap focused Automation입니다.
// Changelog:
// - v1.0.0: exact4 descriptor signatures, immutable VehicleDefense bootstrap exact1, production fingerprint token exact27, serialize/parse/materialize/extract behavior와 no-delta exact0 migration/cross-TypeKey isolation을 최초 추가했습니다.
// Migration:
// - 모든 fixture는 memory/transient-only입니다. Product VehicleDefense Staging/asset Save, protected DA_VehicleDefense_Test와 predecessor accepted histories를 수정하지 않습니다.
// - strict negative authoring behavior는 VDR-P0-01 Gate 2 accepted tests를 재사용하고 Gate 3에서는 DACE-specific evidence만 추가합니다.

#include "CFDAVehicleDefenseDace.h"
#include "CFDAVehicleDefenseProvider.h"
#include "CFDAAmmoDace.h"
#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDAContractGuard.h"
#include "CFDADamageDace.h"
#include "CFDADamageProvider.h"
#include "CFVehicleDefenseData.h"

#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAVehicleDefenseDaceTestsPrivate
{
	// Production probe에 사용할 valid directional armor config를 생성합니다.
	FCFDirectionalArmorConfig BuildArmorConfig(const float MaximumArmor, const float DamageMultiplier)
	{
		// 반환할 exact2 directional armor config입니다.
		FCFDirectionalArmorConfig Config;
		Config.MaximumArmor = MaximumArmor;
		Config.DamageMultiplier = DamageMultiplier;
		return Config;
	}

	// Production probe에서 semantic leaf exact23을 모두 채운 VehicleDefense payload를 생성합니다.
	FCFDAVehicleDefensePayload BuildProbePayload(const bool bUseShield = true)
	{
		// DACE production probe authored payload입니다.
		FCFDAVehicleDefensePayload Payload;
		Payload.DefenseId = FName(TEXT("DaceProbeVehicleDefense"));
		Payload.DefenseMassKg = 145.0f;
		Payload.bUseShield = bUseShield;
		Payload.MaximumShield = 640.0f;
		Payload.ShieldRegenerationDelaySeconds = 4.5f;
		Payload.ShieldRegenerationPerSecond = 22.0f;
		Payload.ArmorType = ECFArmorType::Heavy;
		Payload.ArmorResistance = 95.0f;
		Payload.FrontArmorConfig = BuildArmorConfig(330.0f, 0.72f);
		Payload.LeftArmorConfig = BuildArmorConfig(225.0f, 0.91f);
		Payload.RightArmorConfig = BuildArmorConfig(226.0f, 0.92f);
		Payload.RearArmorConfig = BuildArmorConfig(175.0f, 1.08f);
		Payload.TopArmorConfig = BuildArmorConfig(115.0f, 1.18f);
		Payload.BottomArmorConfig = BuildArmorConfig(135.0f, 1.12f);
		Payload.ShieldComponentDamageScale = 0.20f;
		Payload.ArmorComponentDamageScale = 0.35f;
		Payload.IntegrityComponentDamageScale = 0.55f;
		return Payload;
	}

	// Production VehicleDefense fingerprint가 emit해야 하는 exact27 token label sequence를 test-owned manifest로 반환합니다.
	TArray<FString> BuildExpectedFingerprintLabels()
	{
		// Schema/revision/class exact4 + semantic leaf exact23 token label sequence입니다.
		TArray<FString> Labels =
		{
			TEXT("SchemaId"),
			TEXT("SchemaRevision"),
			TEXT("AdapterContractRevision"),
			TEXT("DataAssetTypeClassPath"),
			TEXT("Payload.DefenseId"),
			TEXT("Payload.DefenseMassKg"),
			TEXT("Payload.bUseShield"),
			TEXT("Payload.MaximumShield"),
			TEXT("Payload.ShieldRegenerationDelaySeconds"),
			TEXT("Payload.ShieldRegenerationPerSecond"),
			TEXT("Payload.ArmorType"),
			TEXT("Payload.ArmorResistance"),
			TEXT("Payload.FrontArmorConfig.MaximumArmor"),
			TEXT("Payload.FrontArmorConfig.DamageMultiplier"),
			TEXT("Payload.LeftArmorConfig.MaximumArmor"),
			TEXT("Payload.LeftArmorConfig.DamageMultiplier"),
			TEXT("Payload.RightArmorConfig.MaximumArmor"),
			TEXT("Payload.RightArmorConfig.DamageMultiplier"),
			TEXT("Payload.RearArmorConfig.MaximumArmor"),
			TEXT("Payload.RearArmorConfig.DamageMultiplier"),
			TEXT("Payload.TopArmorConfig.MaximumArmor"),
			TEXT("Payload.TopArmorConfig.DamageMultiplier"),
			TEXT("Payload.BottomArmorConfig.MaximumArmor"),
			TEXT("Payload.BottomArmorConfig.DamageMultiplier"),
			TEXT("Payload.ShieldComponentDamageScale"),
			TEXT("Payload.ArmorComponentDamageScale"),
			TEXT("Payload.IntegrityComponentDamageScale")
		};
		return Labels;
	}

	// Observed production token labels가 independent expected manifest와 순서/중복까지 exact 일치하는지 검증합니다.
	bool ValidateFingerprintLabels(
		FAutomationTestBase& Test,
		const TArray<FString>& ObservedLabels,
		const TArray<FString>& ExpectedLabels)
	{
		Test.TestEqual(TEXT("VehicleDefense fingerprint token label count must remain exact27"), ObservedLabels.Num(), ExpectedLabels.Num());
		if (ObservedLabels.Num() != ExpectedLabels.Num())
		{
			return false;
		}
		for (int32 LabelIndex = 0; LabelIndex < ExpectedLabels.Num(); ++LabelIndex)
		{
			Test.TestEqual(
				*FString::Printf(TEXT("VehicleDefense fingerprint token label[%d] must remain exact"), LabelIndex),
				ObservedLabels[LabelIndex],
				ExpectedLabels[LabelIndex]);
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefDaceDescriptorTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.VehicleDefenseDaceDescriptorBootstrap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefDaceProductionTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.VehicleDefenseDaceProductionProbe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAVDefDaceMigrationTest,
	"CarFight.DataManagement.CF_FQ_053.VDR_P0_02.VehicleDefenseDaceBootstrapMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Current VehicleDefense exact4 descriptors가 recursive native Source와 bootstrap exact1에 동일하게 결속되는지 검증합니다.
bool FCFDAVDefDaceDescriptorTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Current VehicleDefense DACE aggregate contract 검증 결과입니다.
	const FCFDAContractGuardResult ContractResult = FCFDAVehicleDefenseDace::ValidateCurrentContract();
	TestTrue(TEXT("Current VehicleDefense DACE descriptor/history/revision contract must pass"), ContractResult.bPassed);
	TestEqual(TEXT("VehicleDefense SourceShape descriptor must remain exact29"), FCFDAVehicleDefenseDace::GetSourceShapeDescriptor().Num(), 29);
	TestEqual(TEXT("VehicleDefense AdapterShape descriptor must remain exact37"), FCFDAVehicleDefenseDace::GetAdapterShapeDescriptor().Num(), 37);
	TestEqual(TEXT("VehicleDefense SourceAdapterMapping descriptor must remain exact30"), FCFDAVehicleDefenseDace::GetSourceAdapterMappingDescriptor().Num(), 30);
	TestEqual(TEXT("VehicleDefense SemanticContract descriptor must remain exact14"), FCFDAVehicleDefenseDace::GetSemanticContractDescriptor().Num(), 14);

	// Independent accepted VehicleDefense history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& History = FCFDAVehicleDefenseDace::GetAcceptedSnapshots();
	TestEqual(TEXT("VehicleDefense accepted history must start at exact1"), History.Num(), 1);
	if (History.Num() != 1)
	{
		return false;
	}

	// Dedicated append-only owner가 보존해야 하는 immutable bootstrap exact1입니다.
	const FCFDAAcceptedContractSnapshot& Bootstrap = History[0];
	TestEqual(TEXT("VehicleDefense bootstrap SnapshotId"), Bootstrap.SnapshotId, FString(TEXT("DACE-VehicleDefenseData-S1-A1-Bootstrap")));
	TestTrue(TEXT("VehicleDefense bootstrap previous signature must remain empty"), Bootstrap.PreviousSnapshotSignature.IsEmpty());
	TestEqual(TEXT("VehicleDefense bootstrap SchemaId"), Bootstrap.SchemaId, FString(TEXT("CarFight.DataAsset.VehicleDefenseData")));
	TestEqual(TEXT("VehicleDefense bootstrap SchemaRevision"), Bootstrap.SchemaRevision, 1);
	TestEqual(TEXT("VehicleDefense bootstrap AdapterContractRevision"), Bootstrap.AdapterContractRevision, 1);
	TestEqual(TEXT("VehicleDefense bootstrap class path"), Bootstrap.DataAssetTypeClassPath, FString(TEXT("/Script/CarFight_Re.CFVehicleDefenseData")));
	TestEqual(TEXT("VehicleDefense bootstrap Source signature"), Bootstrap.SourceShapeSignature, FString(TEXT("sha256:d96af45c383bee9ba47d4942a29895c13031218142736965dd7173134ea027f4")));
	TestEqual(TEXT("VehicleDefense bootstrap Adapter signature"), Bootstrap.AdapterShapeSignature, FString(TEXT("sha256:5e394b556ebd9fb94c94c22d729bb0d24acb8dd9770f42cc117652fc68a9f221")));
	TestEqual(TEXT("VehicleDefense bootstrap Mapping signature"), Bootstrap.SourceAdapterMappingSignature, FString(TEXT("sha256:f60ae70c33594342803824d9db714b69d4951e0fbd3a325bea18ef19f2ffd176")));
	TestEqual(TEXT("VehicleDefense bootstrap Semantic signature"), Bootstrap.SemanticContractSignature, FString(TEXT("sha256:480551a50d3ea141a691efc694eb7541bb081bb60c7e95e73bc484adc581fe5e")));
	TestEqual(TEXT("VehicleDefense bootstrap migration impact"), Bootstrap.MigrationImpact, ECFDAContractMigrationImpact::NoMigration);
	TestEqual(TEXT("VehicleDefense bootstrap migration resolution"), Bootstrap.MigrationResolution, ECFDAContractMigrationResolution::NotRequired);
	TestTrue(TEXT("VehicleDefense bootstrap migration evidence must remain empty"), Bootstrap.MigrationEvidenceId.IsEmpty());
	TestEqual(TEXT("VehicleDefense bootstrap chain signature"), Bootstrap.SnapshotSignature, FString(TEXT("sha256:56b9c2905e025df97ba61934bc9c5fd31a7b3da7e7c902db9baa1b326c92fd60")));

	// Current descriptor exact4 signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Signature 계산 실패 원인입니다.
	FString SignatureError;
	TestTrue(TEXT("Current VehicleDefense signatures must build"), FCFDAVehicleDefenseDace::BuildCurrentSignatures(CurrentSignatures, SignatureError));
	if (!SignatureError.IsEmpty())
	{
		AddError(SignatureError);
		return false;
	}
	TestEqual(TEXT("VehicleDefense Source signature equals bootstrap"), CurrentSignatures.SourceShapeSignature, Bootstrap.SourceShapeSignature);
	TestEqual(TEXT("VehicleDefense Adapter signature equals bootstrap"), CurrentSignatures.AdapterShapeSignature, Bootstrap.AdapterShapeSignature);
	TestEqual(TEXT("VehicleDefense Mapping signature equals bootstrap"), CurrentSignatures.SourceAdapterMappingSignature, Bootstrap.SourceAdapterMappingSignature);
	TestEqual(TEXT("VehicleDefense Semantic signature equals bootstrap"), CurrentSignatures.SemanticContractSignature, Bootstrap.SemanticContractSignature);

	// Bootstrap chain signature readback입니다.
	FString RebuiltSnapshotSignature;
	// Snapshot signature build failure diagnostic입니다.
	FString SnapshotError;
	TestTrue(TEXT("VehicleDefense bootstrap snapshot signature must rebuild"), FCFDAContractGuard::BuildSnapshotSignature(Bootstrap, RebuiltSnapshotSignature, SnapshotError));
	TestEqual(TEXT("VehicleDefense bootstrap stored chain signature exact"), RebuiltSnapshotSignature, Bootstrap.SnapshotSignature);
	return true;
}

// 실제 VehicleDefense serializer/parser/fingerprint/materializer-extractor와 exact27 token contract를 검증합니다.
bool FCFDAVDefDaceProductionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// All semantic leaves populated positive production probe payload입니다.
	const FCFDAVehicleDefensePayload ProbePayload = CFDAVehicleDefenseDaceTestsPrivate::BuildProbePayload(true);
	// Production payload fingerprint입니다.
	FString SourceFingerprint;
	// Production token append path에서 관측한 exact27 label sequence입니다.
	TArray<FString> ObservedFingerprintLabels;
	// Fingerprint failure diagnostic입니다.
	FString FingerprintError;
	{
		// Production common token framing의 thread-local observation sink입니다.
		CFDACommonPrimitives::FScopedSemanticTokenProbe TokenProbe(ObservedFingerprintLabels);
		TestTrue(TEXT("VehicleDefense production fingerprint token probe must bind"), TokenProbe.IsBound());
		TestTrue(TEXT("VehicleDefense production fingerprint must succeed"), CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(ProbePayload, SourceFingerprint, FingerprintError));
	}
	// Test-owned independent expected token manifest입니다.
	const TArray<FString> ExpectedFingerprintLabels = CFDAVehicleDefenseDaceTestsPrivate::BuildExpectedFingerprintLabels();
	CFDAVehicleDefenseDaceTestsPrivate::ValidateFingerprintLabels(*this, ObservedFingerprintLabels, ExpectedFingerprintLabels);

	// Production serializer output입니다.
	FString SerializedJson;
	// Serializer failure diagnostic입니다.
	FString SerializerError;
	TestTrue(TEXT("VehicleDefense production serializer must succeed"), CFDAVehicleDefenseProviderImpl::SerializeStagingJson(
		ProbePayload,
		TEXT("/Game/Test/CarFight/VDRDace/DA_DaceProbe.DA_DaceProbe"),
		FString(),
		SerializedJson,
		SerializerError));
	if (!SerializerError.IsEmpty())
	{
		AddError(SerializerError);
		return false;
	}

	// DACE Adapter descriptor와 actual serialized recursive physical shape coverage입니다.
	const FCFDAContractGuardResult AdapterCoverage = FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(
		SerializedJson,
		FCFDAVehicleDefenseDace::GetAdapterShapeDescriptor());
	TestTrue(TEXT("Production VehicleDefense serialized JSON must cover exact37 AdapterShape"), AdapterCoverage.bPassed);

	// Production strict parser result입니다.
	const FCFDAVehicleDefenseParseResult ParseResult = CFDAVehicleDefenseProviderImpl::ParseJson(
		SerializedJson,
		TEXT("Authoring/DataAssetStaging/VehicleDefenseData/__AutomationDace__/DaceProbeVehicleDefense.json"));
	TestTrue(TEXT("VehicleDefense serializer output must parse through production parser"), ParseResult.bValid);
	if (!ParseResult.bValid)
	{
		return false;
	}
	TestEqual(TEXT("VehicleDefense serialize-parse fingerprint remains exact"), ParseResult.Record.StagingSemanticFingerprint, SourceFingerprint);

	// Transient-only materializer probe target입니다.
	UCFVehicleDefenseData* const TransientDefense = NewObject<UCFVehicleDefenseData>(GetTransientPackage());
	TestNotNull(TEXT("Transient VehicleDefense probe object must be created"), TransientDefense);
	if (TransientDefense == nullptr)
	{
		return false;
	}
	CFDAVehicleDefenseProviderImpl::MaterializePayload(*TransientDefense, ParseResult.Record.Payload);
	// Materialized transient UObject에서 다시 추출한 exact17/nested payload입니다.
	FCFDAVehicleDefensePayload ExtractedPayload;
	// Extractor diagnostics입니다.
	TArray<FCFDAStagingIssue> ExtractIssues;
	TestTrue(TEXT("Materialized VehicleDefense must extract through production extractor"), CFDAVehicleDefenseProviderImpl::ExtractPayload(*TransientDefense, ExtractedPayload, ExtractIssues));
	// Extracted semantic fingerprint입니다.
	FString ExtractedFingerprint;
	// Extracted fingerprint failure diagnostic입니다.
	FString ExtractedFingerprintError;
	TestTrue(TEXT("Extracted VehicleDefense fingerprint must succeed"), CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(ExtractedPayload, ExtractedFingerprint, ExtractedFingerprintError));
	TestEqual(TEXT("VehicleDefense materialize-extract fingerprint remains exact"), ExtractedFingerprint, SourceFingerprint);
	TestEqual(TEXT("Nested front MaximumArmor survives production round-trip"), ExtractedPayload.FrontArmorConfig.MaximumArmor, 330.0f);
	TestEqual(TEXT("Nested bottom DamageMultiplier survives production round-trip"), ExtractedPayload.BottomArmorConfig.DamageMultiplier, 1.12f);

	// Disabled shield 상태에서도 valid raw shield authored values를 보존해야 하는 payload입니다.
	FCFDAVehicleDefensePayload DisabledShieldPayload = CFDAVehicleDefenseDaceTestsPrivate::BuildProbePayload(false);
	DisabledShieldPayload.MaximumShield = 777.0f;
	DisabledShieldPayload.ShieldRegenerationDelaySeconds = 8.0f;
	DisabledShieldPayload.ShieldRegenerationPerSecond = 31.0f;
	// Disabled shield transient materializer target입니다.
	UCFVehicleDefenseData* const DisabledDefense = NewObject<UCFVehicleDefenseData>(GetTransientPackage());
	TestNotNull(TEXT("Disabled-shield transient VehicleDefense must be created"), DisabledDefense);
	if (DisabledDefense == nullptr)
	{
		return false;
	}
	CFDAVehicleDefenseProviderImpl::MaterializePayload(*DisabledDefense, DisabledShieldPayload);
	// Disabled shield materialized payload readback입니다.
	FCFDAVehicleDefensePayload DisabledReadback;
	// Disabled shield extractor diagnostics입니다.
	TArray<FCFDAStagingIssue> DisabledIssues;
	TestTrue(TEXT("Disabled-shield VehicleDefense readback must succeed"), CFDAVehicleDefenseProviderImpl::ExtractPayload(*DisabledDefense, DisabledReadback, DisabledIssues));
	TestFalse(TEXT("Disabled-shield flag remains false"), DisabledReadback.bUseShield);
	TestEqual(TEXT("Disabled MaximumShield raw value preserved"), DisabledReadback.MaximumShield, 777.0f);
	TestEqual(TEXT("Disabled regen delay raw value preserved"), DisabledReadback.ShieldRegenerationDelaySeconds, 8.0f);
	TestEqual(TEXT("Disabled regen rate raw value preserved"), DisabledReadback.ShieldRegenerationPerSecond, 31.0f);

	// FName case-only identity variation payload입니다.
	FCFDAVehicleDefensePayload CaseEquivalentPayload = ProbePayload;
	CaseEquivalentPayload.DefenseId = FName(TEXT("daceprobevehicledefense"));
	// Case-only variation semantic fingerprint입니다.
	FString CaseEquivalentFingerprint;
	// Case-only variation fingerprint error입니다.
	FString CaseEquivalentError;
	TestTrue(TEXT("FName case-only VehicleDefense identity must fingerprint"), CFDAVehicleDefenseProviderImpl::BuildSemanticFingerprint(CaseEquivalentPayload, CaseEquivalentFingerprint, CaseEquivalentError));
	TestEqual(TEXT("FName case-only VehicleDefense identity remains semantic-equivalent"), CaseEquivalentFingerprint, SourceFingerprint);
	return true;
}

// VehicleDefense bootstrap, canonical exact0와 no-delta migration gate가 predecessor TypeKey histories와 분리되는지 검증합니다.
bool FCFDAVDefDaceMigrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	// Current trusted VehicleDefense provider입니다.
	const FCFDAVehicleDefenseProvider& VehicleDefenseProvider = CFDAVehicleDefenseProvider::GetProvider();
	TestEqual(TEXT("VehicleDefense DACE must be ContractReady"), VehicleDefenseProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestEqual(TEXT("VehicleDefense DACE owner"), VehicleDefenseProvider.Descriptor.DaceContractOwnerName, FName(TEXT("CFDAVehicleDefenseDace")));
	TestEqual(TEXT("VehicleDefense accepted history namespace"), VehicleDefenseProvider.Descriptor.DaceAcceptedHistoryNamespace, FString(TEXT("DACE-VehicleDefenseData")));
	TestTrue(TEXT("VehicleDefense canonical target set explicitly declared"), VehicleDefenseProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Product canonical VehicleDefense DACE target remains exact0"), VehicleDefenseProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);
	TestTrue(TEXT("VehicleDefense no-delta declaration remains nullptr"), FCFDAVehicleDefenseDace::GetCurrentChangeDeclaration() == nullptr);

	// Provider-owned canonical exact0 compatibility result입니다.
	const FCFDAContractGuardResult CanonicalCompatibility = FCFDAContractGuard::ValidateCanonicalStagingCompatibilityForProvider(VehicleDefenseProvider);
	TestTrue(TEXT("Explicit VehicleDefense canonical exact0 passes compatibility"), CanonicalCompatibility.bPassed);
	// Current no-delta migration gate입니다.
	const FCFDAMigrationGateResult CurrentGate = FCFDAVehicleDefenseDace::EvaluateCurrentMigrationGate();
	TestTrue(TEXT("Accepted VehicleDefense bootstrap + exact0 compatibility passes migration validation"), CurrentGate.Validation.bPassed);
	TestFalse(TEXT("No-delta VehicleDefense state rejects duplicate accepted append"), CurrentGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Accepted VehicleDefense current contract may promote Current projection"), CurrentGate.bCurrentSystemPromotionAllowed);

	// Missile accepted history를 VehicleDefense provider에 잘못 공급한 contamination 결과입니다.
	const FCFDAContractGuardResult MissileAsVehicleDefense = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		VehicleDefenseProvider.Descriptor,
		FCFDAContractGuard::GetAcceptedSnapshots());
	TestFalse(TEXT("Missile accepted chain never validates as VehicleDefense history"), MissileAsVehicleDefense.bPassed);
	// Ammo accepted history contamination 결과입니다.
	const FCFDAContractGuardResult AmmoAsVehicleDefense = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		VehicleDefenseProvider.Descriptor,
		FCFDAAmmoDace::GetAcceptedSnapshots());
	TestFalse(TEXT("Ammo accepted chain never validates as VehicleDefense history"), AmmoAsVehicleDefense.bPassed);
	// Damage accepted history contamination 결과입니다.
	const FCFDAContractGuardResult DamageAsVehicleDefense = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		VehicleDefenseProvider.Descriptor,
		FCFDADamageDace::GetAcceptedSnapshots());
	TestFalse(TEXT("Damage accepted chain never validates as VehicleDefense history"), DamageAsVehicleDefense.bPassed);
	// VehicleDefense history를 Damage provider에 잘못 공급한 reverse contamination 결과입니다.
	const FCFDAContractGuardResult VehicleDefenseAsDamage = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		CFDADamageProvider::GetProvider().Descriptor,
		FCFDAVehicleDefenseDace::GetAcceptedSnapshots());
	TestFalse(TEXT("VehicleDefense accepted chain never validates as Damage history"), VehicleDefenseAsDamage.bPassed);
	// VehicleDefense history를 Ammo provider에 잘못 공급한 reverse contamination 결과입니다.
	const FCFDAContractGuardResult VehicleDefenseAsAmmo = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		CFDAAmmoProvider::GetProvider().Descriptor,
		FCFDAVehicleDefenseDace::GetAcceptedSnapshots());
	TestFalse(TEXT("VehicleDefense accepted chain never validates as Ammo history"), VehicleDefenseAsAmmo.bPassed);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
