// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageDaceTests.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-052 DDO-P0-03 Damage DACE descriptor/production probe/bootstrap focused Automation입니다.
// Changelog:
// - v1.0.0: exact4 descriptor signatures, immutable Damage bootstrap exact1, production fingerprint token exact16, serialize/parse/materialize/extract behavior, negative revision/value probes와 no-delta exact0 migration isolation을 최초 추가했습니다.
// Migration:
// - 모든 P0-03 fixture는 memory/transient-only입니다. Product Damage Staging/asset Save, Missile/Ammo accepted history와 shared FCFDAContractGuard를 수정하지 않습니다.
// - fingerprint coverage expected manifest는 production descriptor/token implementation에서 생성하지 않고 이 test-owned literal sequence로 독립 유지합니다.

#include "CFDADamageDace.h"
#include "CFDADamageProvider.h"
#include "CFDAAmmoDace.h"
#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDAContractGuard.h"
#include "CFDamageData.h"

#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDADamageDaceTestsPrivate
{
	// Production probe에서 공통으로 사용할 exact12 Damage payload를 생성합니다.
	FCFDADamagePayload BuildProbePayload(const bool bUseRadialDamage = true)
	{
		// DACE production probe exact12 payload입니다.
		FCFDADamagePayload Payload;
		Payload.DamageId = FName(TEXT("DaceProbeDamage"));
		Payload.DamageType = ECFDamageType::Explosive;
		Payload.BaseDamage = 45.0f;
		Payload.bCanDamageSelf = false;
		Payload.ArmorPenetration = 32.0f;
		Payload.bUseRadialDamage = bUseRadialDamage;
		Payload.ExplosionRadius = 240.0f;
		Payload.ExplosionInnerRadius = 300.0f;
		Payload.ExplosionDamage = 120.0f;
		Payload.MinExplosionDamageScale = 0.25f;
		Payload.ModuleDamageScale = 1.25f;
		Payload.ImpulseStrength = 650.0f;
		return Payload;
	}

	// Production Damage fingerprint가 emit해야 하는 exact16 token label sequence를 test-owned independent manifest로 반환합니다.
	TArray<FString> BuildExpectedFingerprintLabels()
	{
		// Schema/revision/class exact4 + Damage authored exact12 token label sequence입니다.
		TArray<FString> Labels =
		{
			TEXT("SchemaId"),
			TEXT("SchemaRevision"),
			TEXT("AdapterContractRevision"),
			TEXT("DataAssetTypeClassPath"),
			TEXT("Payload.DamageId"),
			TEXT("Payload.DamageType"),
			TEXT("Payload.BaseDamage"),
			TEXT("Payload.bCanDamageSelf"),
			TEXT("Payload.ArmorPenetration"),
			TEXT("Payload.bUseRadialDamage"),
			TEXT("Payload.ExplosionRadius"),
			TEXT("Payload.ExplosionInnerRadius"),
			TEXT("Payload.ExplosionDamage"),
			TEXT("Payload.MinExplosionDamageScale"),
			TEXT("Payload.ModuleDamageScale"),
			TEXT("Payload.ImpulseStrength")
		};
		return Labels;
	}

	// Observed production token labels가 independent expected manifest와 순서/중복까지 exact 일치하는지 검증합니다.
	bool ValidateFingerprintLabels(
		FAutomationTestBase& Test,
		const TArray<FString>& ObservedLabels,
		const TArray<FString>& ExpectedLabels)
	{
		Test.TestEqual(TEXT("Damage fingerprint token label count must remain exact16"), ObservedLabels.Num(), ExpectedLabels.Num());
		if (ObservedLabels.Num() != ExpectedLabels.Num())
		{
			return false;
		}
		// exact sequence를 순서대로 비교할 현재 token label index입니다.
		for (int32 LabelIndex = 0; LabelIndex < ExpectedLabels.Num(); ++LabelIndex)
		{
			Test.TestEqual(
				*FString::Printf(TEXT("Damage fingerprint token label[%d] must remain exact"), LabelIndex),
				ObservedLabels[LabelIndex],
				ExpectedLabels[LabelIndex]);
		}
		return true;
	}

	// JSON root를 memory-only로 parse합니다.
	bool ParseRootJson(const FString& JsonText, TSharedPtr<FJsonObject>& OutRoot)
	{
		// Memory JSON reader입니다.
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
	}

	// Memory-only JSON root를 다시 text로 직렬화합니다.
	bool SerializeRootJson(const TSharedRef<FJsonObject>& RootObject, FString& OutJsonText)
	{
		OutJsonText.Reset();
		// Memory JSON writer입니다.
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonText);
		// JSON serializer 실행 결과입니다.
		const bool bSerialized = FJsonSerializer::Serialize(RootObject, Writer);
		Writer->Close();
		return bSerialized;
	}

	// Current valid JSON을 새 mutable root로 복제해 negative fixture의 shared 시작점을 만듭니다.
	TSharedPtr<FJsonObject> ParseMutableFixture(FAutomationTestBase& Test, const FString& CurrentJson, const TCHAR* Context)
	{
		// Caller가 독립적으로 mutate할 fresh JSON root입니다.
		TSharedPtr<FJsonObject> RootObject;
		Test.TestTrue(Context, ParseRootJson(CurrentJson, RootObject));
		return RootObject;
	}

	// Mutable negative root를 strict Damage parser 입력 text로 변환합니다.
	FCFDADamageParseResult ParseNegativeFixture(
		FAutomationTestBase& Test,
		const TSharedPtr<FJsonObject>& RootObject,
		const TCHAR* SerializeContext)
	{
		// Negative fixture serialized JSON text입니다.
		FString JsonText;
		if (!RootObject.IsValid() || !Test.TestTrue(SerializeContext, SerializeRootJson(RootObject.ToSharedRef(), JsonText)))
		{
			return FCFDADamageParseResult();
		}
		return CFDADamageProviderImpl::ParseJson(JsonText);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageDaceDescriptorTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceDescriptorBootstrap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageDaceProductionProbeTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceProductionProbe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageDaceNegativeProbeTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceNegativeProbe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDADamageDaceMigrationTest,
	"CarFight.DataManagement.CF_FQ_052.DDO_P0_03.DamageDaceBootstrapMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Current Damage exact4 descriptors가 native Source와 bootstrap exact1에 동일하게 결속되는지 검증합니다.
bool FCFDADamageDaceDescriptorTest::RunTest(const FString& Parameters)
{
	// Current Damage DACE aggregate contract 검증 결과입니다.
	const FCFDAContractGuardResult ContractResult = FCFDADamageDace::ValidateCurrentContract();
	TestTrue(TEXT("Current Damage DACE descriptor/history/revision contract must pass"), ContractResult.bPassed);
	TestEqual(TEXT("Damage SourceShape descriptor must remain exact12"), FCFDADamageDace::GetSourceShapeDescriptor().Num(), 12);
	TestEqual(TEXT("Damage AdapterShape descriptor must remain exact20"), FCFDADamageDace::GetAdapterShapeDescriptor().Num(), 20);
	TestEqual(TEXT("Damage SourceAdapterMapping descriptor must remain exact19"), FCFDADamageDace::GetSourceAdapterMappingDescriptor().Num(), 19);
	TestEqual(TEXT("Damage SemanticContract descriptor must remain exact14"), FCFDADamageDace::GetSemanticContractDescriptor().Num(), 14);

	// Independent accepted Damage history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& History = FCFDADamageDace::GetAcceptedSnapshots();
	TestEqual(TEXT("Damage accepted history must start at exact1"), History.Num(), 1);
	if (History.Num() != 1)
	{
		return false;
	}

	// Dedicated append-only owner가 보존해야 하는 immutable bootstrap exact1입니다.
	const FCFDAAcceptedContractSnapshot& Bootstrap = History[0];
	TestEqual(TEXT("Damage bootstrap SnapshotId must remain immutable"), Bootstrap.SnapshotId, FString(TEXT("DACE-DamageData-S1-A1-Bootstrap")));
	TestTrue(TEXT("Damage bootstrap previous signature must remain empty"), Bootstrap.PreviousSnapshotSignature.IsEmpty());
	TestEqual(TEXT("Damage bootstrap SchemaId must remain immutable"), Bootstrap.SchemaId, FString(TEXT("CarFight.DataAsset.DamageData")));
	TestEqual(TEXT("Damage bootstrap SchemaRevision must remain 1"), Bootstrap.SchemaRevision, 1);
	TestEqual(TEXT("Damage bootstrap AdapterContractRevision must remain 1"), Bootstrap.AdapterContractRevision, 1);
	TestEqual(TEXT("Damage bootstrap class path must remain immutable"), Bootstrap.DataAssetTypeClassPath, FString(TEXT("/Script/CarFight_Re.CFDamageData")));
	TestEqual(TEXT("Damage bootstrap Source signature must remain immutable"), Bootstrap.SourceShapeSignature, FString(TEXT("sha256:1082759fbf56bcd3a1fa7e1f2beab95a2fdc08738eaec81fe589e8c8294db663")));
	TestEqual(TEXT("Damage bootstrap Adapter signature must remain immutable"), Bootstrap.AdapterShapeSignature, FString(TEXT("sha256:da4d501862867d6b5da4adf6f233d2cd107b39073efe1061850d42e6537e888a")));
	TestEqual(TEXT("Damage bootstrap Mapping signature must remain immutable"), Bootstrap.SourceAdapterMappingSignature, FString(TEXT("sha256:ad3edca41d0d1083b8795c1afc07d84bf826457202e161dc9b73f6a1d8f09db7")));
	TestEqual(TEXT("Damage bootstrap Semantic signature must remain immutable"), Bootstrap.SemanticContractSignature, FString(TEXT("sha256:3f6d76d67ac3749ab3abd9e4a8fccdd4ed242d44202ed8a71f7203e5e69a159a")));
	TestEqual(TEXT("Damage bootstrap migration impact must remain NoMigration"), Bootstrap.MigrationImpact, ECFDAContractMigrationImpact::NoMigration);
	TestEqual(TEXT("Damage bootstrap migration resolution must remain NotRequired"), Bootstrap.MigrationResolution, ECFDAContractMigrationResolution::NotRequired);
	TestTrue(TEXT("Damage bootstrap migration evidence must remain empty"), Bootstrap.MigrationEvidenceId.IsEmpty());
	TestEqual(TEXT("Damage bootstrap chain signature must remain immutable"), Bootstrap.SnapshotSignature, FString(TEXT("sha256:d3bc8e2c004d5e86a4339f3869badd9a146af5e0bdbe7faf476f14914dfea7ee")));

	// Current descriptor exact4 signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Signature failure diagnostic입니다.
	FString SignatureError;
	TestTrue(TEXT("Current Damage signatures must build"), FCFDADamageDace::BuildCurrentSignatures(CurrentSignatures, SignatureError));
	if (!SignatureError.IsEmpty())
	{
		AddError(SignatureError);
		return false;
	}
	TestEqual(TEXT("Damage SourceShape signature must equal accepted bootstrap"), CurrentSignatures.SourceShapeSignature, Bootstrap.SourceShapeSignature);
	TestEqual(TEXT("Damage AdapterShape signature must equal accepted bootstrap"), CurrentSignatures.AdapterShapeSignature, Bootstrap.AdapterShapeSignature);
	TestEqual(TEXT("Damage Mapping signature must equal accepted bootstrap"), CurrentSignatures.SourceAdapterMappingSignature, Bootstrap.SourceAdapterMappingSignature);
	TestEqual(TEXT("Damage Semantic signature must equal accepted bootstrap"), CurrentSignatures.SemanticContractSignature, Bootstrap.SemanticContractSignature);

	// Bootstrap chain signature readback입니다.
	FString RebuiltSnapshotSignature;
	// Snapshot signature build failure diagnostic입니다.
	FString SnapshotError;
	TestTrue(TEXT("Damage bootstrap snapshot signature must rebuild through shared authority"), FCFDAContractGuard::BuildSnapshotSignature(Bootstrap, RebuiltSnapshotSignature, SnapshotError));
	TestEqual(TEXT("Damage bootstrap stored chain signature must be exact"), RebuiltSnapshotSignature, Bootstrap.SnapshotSignature);
	return true;
}

// 실제 Damage production serializer/parser/fingerprint/materializer-extractor와 exact16 token contract를 검증합니다.
bool FCFDADamageDaceProductionProbeTest::RunTest(const FString& Parameters)
{
	// Radial enabled + InnerRadius > Radius를 포함한 positive production probe payload입니다.
	const FCFDADamagePayload ProbePayload = CFDADamageDaceTestsPrivate::BuildProbePayload(true);
	// Production payload fingerprint입니다.
	FString SourceFingerprint;
	// 실제 production token append path에서 관측한 exact16 label sequence입니다.
	TArray<FString> ObservedFingerprintLabels;
	// Fingerprint failure diagnostic입니다.
	FString FingerprintError;
	{
		// Production common token framing의 thread-local observation sink입니다.
		CFDACommonPrimitives::FScopedSemanticTokenProbe TokenProbe(ObservedFingerprintLabels);
		TestTrue(TEXT("Damage production fingerprint token probe must bind"), TokenProbe.IsBound());
		TestTrue(TEXT("Damage production fingerprint must succeed"), CFDADamageProviderImpl::BuildSemanticFingerprint(ProbePayload, SourceFingerprint, FingerprintError));
	}
	// Test-owned independent exact16 expected token manifest입니다.
	const TArray<FString> ExpectedFingerprintLabels = CFDADamageDaceTestsPrivate::BuildExpectedFingerprintLabels();
	CFDADamageDaceTestsPrivate::ValidateFingerprintLabels(*this, ObservedFingerprintLabels, ExpectedFingerprintLabels);

	// Production serializer output입니다.
	FString SerializedJson;
	// Serializer failure diagnostic입니다.
	FString SerializerError;
	TestTrue(TEXT("Damage production serializer must succeed"), CFDADamageProviderImpl::SerializeStagingJson(
		ProbePayload,
		TEXT("/Game/Test/CarFight/DDODamageDace/DA_DaceProbe.DA_DaceProbe"),
		FString(),
		SerializedJson,
		SerializerError));
	if (!SerializerError.IsEmpty())
	{
		AddError(SerializerError);
		return false;
	}

	// DACE adapter descriptor와 실제 serialized physical shape의 coverage 결과입니다.
	const FCFDAContractGuardResult AdapterCoverage = FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(
		SerializedJson,
		FCFDADamageDace::GetAdapterShapeDescriptor());
	TestTrue(TEXT("Production Damage serialized JSON must cover exact20 AdapterShape"), AdapterCoverage.bPassed);

	// Production strict parser exact result입니다.
	const FCFDADamageParseResult ParseResult = CFDADamageProviderImpl::ParseJson(
		SerializedJson,
		TEXT("Authoring/DataAssetStaging/DamageData/__AutomationDace__/DaceProbeDamage.json"));
	TestTrue(TEXT("Damage serializer output must parse through strict production parser"), ParseResult.bValid);
	if (!ParseResult.bValid)
	{
		return false;
	}
	TestEqual(TEXT("Serialize→parse fingerprint must remain exact"), ParseResult.Record.StagingSemanticFingerprint, SourceFingerprint);
	TestTrue(TEXT("Normative Damage contract must allow ExplosionInnerRadius > ExplosionRadius"), ParseResult.Record.Payload.ExplosionInnerRadius > ParseResult.Record.Payload.ExplosionRadius);

	// Transient-only materializer probe target입니다.
	UCFDamageData* const TransientDamage = NewObject<UCFDamageData>(GetTransientPackage());
	TestNotNull(TEXT("Transient Damage probe object must be created"), TransientDamage);
	if (TransientDamage == nullptr)
	{
		return false;
	}
	CFDADamageProviderImpl::MaterializePayload(*TransientDamage, ParseResult.Record.Payload);
	// Materialized transient UObject에서 다시 추출한 exact12 payload입니다.
	FCFDADamagePayload ExtractedPayload;
	// Extractor diagnostics입니다.
	TArray<FCFDAStagingIssue> ExtractIssues;
	TestTrue(TEXT("Materialized Damage must extract through production extractor"), CFDADamageProviderImpl::ExtractPayload(*TransientDamage, ExtractedPayload, ExtractIssues));
	// Extracted semantic fingerprint입니다.
	FString ExtractedFingerprint;
	// Extracted fingerprint failure diagnostic입니다.
	FString ExtractedFingerprintError;
	TestTrue(TEXT("Extracted Damage fingerprint must succeed"), CFDADamageProviderImpl::BuildSemanticFingerprint(ExtractedPayload, ExtractedFingerprint, ExtractedFingerprintError));
	TestEqual(TEXT("Materialize→extract semantic fingerprint must remain exact"), ExtractedFingerprint, SourceFingerprint);

	// Radial disabled 상태에서도 valid nonzero radial authored values를 보존해야 하는 payload입니다.
	FCFDADamagePayload RadialDisabledPayload = CFDADamageDaceTestsPrivate::BuildProbePayload(false);
	RadialDisabledPayload.ExplosionRadius = 125.0f;
	RadialDisabledPayload.ExplosionInnerRadius = 300.0f;
	RadialDisabledPayload.ExplosionDamage = 75.0f;
	// Radial-disabled transient materializer target입니다.
	UCFDamageData* const RadialDisabledDamage = NewObject<UCFDamageData>(GetTransientPackage());
	TestNotNull(TEXT("Radial-disabled transient Damage must be created"), RadialDisabledDamage);
	if (RadialDisabledDamage == nullptr)
	{
		return false;
	}
	CFDADamageProviderImpl::MaterializePayload(*RadialDisabledDamage, RadialDisabledPayload);
	// Radial-disabled materialized payload readback입니다.
	FCFDADamagePayload RadialDisabledReadback;
	// Radial-disabled extractor diagnostics입니다.
	TArray<FCFDAStagingIssue> RadialDisabledIssues;
	TestTrue(TEXT("Radial-disabled Damage readback must succeed"), CFDADamageProviderImpl::ExtractPayload(*RadialDisabledDamage, RadialDisabledReadback, RadialDisabledIssues));
	TestFalse(TEXT("Radial-disabled flag must remain false"), RadialDisabledReadback.bUseRadialDamage);
	TestEqual(TEXT("Radial-disabled ExplosionRadius must not auto-zero"), RadialDisabledReadback.ExplosionRadius, 125.0f);
	TestEqual(TEXT("Radial-disabled ExplosionInnerRadius must not auto-zero"), RadialDisabledReadback.ExplosionInnerRadius, 300.0f);
	TestEqual(TEXT("Radial-disabled ExplosionDamage must not auto-zero"), RadialDisabledReadback.ExplosionDamage, 75.0f);

	// FName case-only identity variation payload입니다.
	FCFDADamagePayload CaseEquivalentPayload = ProbePayload;
	CaseEquivalentPayload.DamageId = FName(TEXT("daceprobedamage"));
	// Case-only variation semantic fingerprint입니다.
	FString CaseEquivalentFingerprint;
	// Case-only variation fingerprint error입니다.
	FString CaseEquivalentError;
	TestTrue(TEXT("FName case-only Damage identity must fingerprint"), CFDADamageProviderImpl::BuildSemanticFingerprint(CaseEquivalentPayload, CaseEquivalentFingerprint, CaseEquivalentError));
	TestEqual(TEXT("FName case-only Damage identity must remain semantic-equivalent"), CaseEquivalentFingerprint, SourceFingerprint);
	return true;
}

// Damage wrong enum/value/radial/revision inputs가 strict production path에서 fail-closed되는지 검증합니다.
bool FCFDADamageDaceNegativeProbeTest::RunTest(const FString& Parameters)
{
	// Current valid exact12 source fixture입니다.
	const FCFDADamagePayload ProbePayload = CFDADamageDaceTestsPrivate::BuildProbePayload(true);
	// Current valid production JSON입니다.
	FString CurrentJson;
	// Current serializer failure diagnostic입니다.
	FString SerializerError;
	TestTrue(TEXT("Damage negative fixture base serializer must succeed"), CFDADamageProviderImpl::SerializeStagingJson(
		ProbePayload,
		TEXT("/Game/Test/CarFight/DDODamageDace/DA_DaceNegative.DA_DaceNegative"),
		FString(),
		CurrentJson,
		SerializerError));
	if (!SerializerError.IsEmpty())
	{
		AddError(SerializerError);
		return false;
	}

	// Wrong-case DamageType fixture root입니다.
	TSharedPtr<FJsonObject> WrongCaseRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Wrong-case DamageType fixture must parse in memory"));
	// Wrong-case DamageType payload object입니다.
	TSharedPtr<FJsonObject> WrongCasePayload = WrongCaseRoot.IsValid() ? WrongCaseRoot->GetObjectField(TEXT("Payload")) : nullptr;
	if (WrongCasePayload.IsValid())
	{
		WrongCasePayload->SetStringField(TEXT("DamageType"), TEXT("explosive"));
	}
	// Wrong-case strict parse result입니다.
	const FCFDADamageParseResult WrongCaseParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, WrongCaseRoot, TEXT("Wrong-case DamageType fixture must serialize"));
	TestFalse(TEXT("Wrong-case DamageType token must fail strict production parser"), WrongCaseParse.bValid);

	// BaseDamage exact-zero fixture root입니다.
	TSharedPtr<FJsonObject> ZeroBaseRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Zero BaseDamage fixture must parse in memory"));
	// BaseDamage exact-zero payload object입니다.
	TSharedPtr<FJsonObject> ZeroBasePayload = ZeroBaseRoot.IsValid() ? ZeroBaseRoot->GetObjectField(TEXT("Payload")) : nullptr;
	if (ZeroBasePayload.IsValid())
	{
		ZeroBasePayload->SetNumberField(TEXT("BaseDamage"), 0.0);
	}
	// BaseDamage zero strict parse result입니다.
	const FCFDADamageParseResult ZeroBaseParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, ZeroBaseRoot, TEXT("Zero BaseDamage fixture must serialize"));
	TestFalse(TEXT("BaseDamage == 0 must fail strict production parser"), ZeroBaseParse.bValid);

	// Radial-enabled ExplosionRadius exact-zero fixture root입니다.
	TSharedPtr<FJsonObject> ZeroRadiusRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Zero ExplosionRadius fixture must parse in memory"));
	// Zero radius payload object입니다.
	TSharedPtr<FJsonObject> ZeroRadiusPayload = ZeroRadiusRoot.IsValid() ? ZeroRadiusRoot->GetObjectField(TEXT("Payload")) : nullptr;
	if (ZeroRadiusPayload.IsValid())
	{
		ZeroRadiusPayload->SetNumberField(TEXT("ExplosionRadius"), 0.0);
	}
	// Zero radius strict parse result입니다.
	const FCFDADamageParseResult ZeroRadiusParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, ZeroRadiusRoot, TEXT("Zero ExplosionRadius fixture must serialize"));
	TestFalse(TEXT("Radial-enabled ExplosionRadius == 0 must fail"), ZeroRadiusParse.bValid);

	// Radial-enabled ExplosionDamage exact-zero fixture root입니다.
	TSharedPtr<FJsonObject> ZeroExplosionDamageRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Zero ExplosionDamage fixture must parse in memory"));
	// Zero explosion damage payload object입니다.
	TSharedPtr<FJsonObject> ZeroExplosionDamagePayload = ZeroExplosionDamageRoot.IsValid() ? ZeroExplosionDamageRoot->GetObjectField(TEXT("Payload")) : nullptr;
	if (ZeroExplosionDamagePayload.IsValid())
	{
		ZeroExplosionDamagePayload->SetNumberField(TEXT("ExplosionDamage"), 0.0);
	}
	// Zero explosion damage strict parse result입니다.
	const FCFDADamageParseResult ZeroExplosionDamageParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, ZeroExplosionDamageRoot, TEXT("Zero ExplosionDamage fixture must serialize"));
	TestFalse(TEXT("Radial-enabled ExplosionDamage == 0 must fail"), ZeroExplosionDamageParse.bValid);

	// MinExplosionDamageScale below-range fixture root입니다.
	TSharedPtr<FJsonObject> LowScaleRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Low MinExplosionDamageScale fixture must parse in memory"));
	// Below-range min scale payload object입니다.
	TSharedPtr<FJsonObject> LowScalePayload = LowScaleRoot.IsValid() ? LowScaleRoot->GetObjectField(TEXT("Payload")) : nullptr;
	if (LowScalePayload.IsValid())
	{
		LowScalePayload->SetNumberField(TEXT("MinExplosionDamageScale"), -0.1);
	}
	// Below-range min scale strict parse result입니다.
	const FCFDADamageParseResult LowScaleParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, LowScaleRoot, TEXT("Low MinExplosionDamageScale fixture must serialize"));
	TestFalse(TEXT("MinExplosionDamageScale < 0 must fail"), LowScaleParse.bValid);

	// MinExplosionDamageScale above-range fixture root입니다.
	TSharedPtr<FJsonObject> HighScaleRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("High MinExplosionDamageScale fixture must parse in memory"));
	// Above-range min scale payload object입니다.
	TSharedPtr<FJsonObject> HighScalePayload = HighScaleRoot.IsValid() ? HighScaleRoot->GetObjectField(TEXT("Payload")) : nullptr;
	if (HighScalePayload.IsValid())
	{
		HighScalePayload->SetNumberField(TEXT("MinExplosionDamageScale"), 1.1);
	}
	// Above-range min scale strict parse result입니다.
	const FCFDADamageParseResult HighScaleParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, HighScaleRoot, TEXT("High MinExplosionDamageScale fixture must serialize"));
	TestFalse(TEXT("MinExplosionDamageScale > 1 must fail"), HighScaleParse.bValid);

	// Old SchemaRevision fixture root입니다.
	TSharedPtr<FJsonObject> OldSchemaRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Old SchemaRevision fixture must parse in memory"));
	if (OldSchemaRoot.IsValid())
	{
		OldSchemaRoot->SetNumberField(TEXT("SchemaRevision"), 0.0);
	}
	// Old SchemaRevision strict parse result입니다.
	const FCFDADamageParseResult OldSchemaParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, OldSchemaRoot, TEXT("Old SchemaRevision fixture must serialize"));
	TestFalse(TEXT("Old Damage SchemaRevision must fail strict current provider compatibility"), OldSchemaParse.bValid);

	// Old AdapterContractRevision fixture root입니다.
	TSharedPtr<FJsonObject> OldAdapterRoot = CFDADamageDaceTestsPrivate::ParseMutableFixture(*this, CurrentJson, TEXT("Old AdapterContractRevision fixture must parse in memory"));
	if (OldAdapterRoot.IsValid())
	{
		OldAdapterRoot->SetNumberField(TEXT("AdapterContractRevision"), 0.0);
	}
	// Old AdapterContractRevision strict parse result입니다.
	const FCFDADamageParseResult OldAdapterParse = CFDADamageDaceTestsPrivate::ParseNegativeFixture(*this, OldAdapterRoot, TEXT("Old AdapterContractRevision fixture must serialize"));
	TestFalse(TEXT("Old Damage AdapterContractRevision must fail strict current provider compatibility"), OldAdapterParse.bValid);
	return true;
}

// Independent Damage bootstrap, canonical exact0와 no-delta migration gate가 Missile/Ammo TypeKey와 분리되는지 검증합니다.
bool FCFDADamageDaceMigrationTest::RunTest(const FString& Parameters)
{
	// Current trusted Damage provider입니다.
	const FCFDADamageTypeProvider& DamageProvider = CFDADamageProvider::GetProvider();
	TestEqual(TEXT("Damage DACE must be ContractReady"), DamageProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestEqual(TEXT("Damage DACE owner must be CFDADamageDace"), DamageProvider.Descriptor.DaceContractOwnerName, FName(TEXT("CFDADamageDace")));
	TestEqual(TEXT("Damage accepted history namespace must remain isolated"), DamageProvider.Descriptor.DaceAcceptedHistoryNamespace, FString(TEXT("DACE-DamageData")));
	TestTrue(TEXT("Damage canonical target set must be explicitly declared"), DamageProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Product canonical Damage DACE target set must remain exact0"), DamageProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);
	TestTrue(TEXT("Damage no-delta declaration must remain nullptr"), FCFDADamageDace::GetCurrentChangeDeclaration() == nullptr);

	// Provider-owned canonical exact0 compatibility result입니다.
	const FCFDAContractGuardResult CanonicalCompatibility = FCFDAContractGuard::ValidateCanonicalStagingCompatibilityForProvider(DamageProvider);
	TestTrue(TEXT("Explicit Damage canonical exact0 must pass compatibility without fake Product Staging"), CanonicalCompatibility.bPassed);
	// Current no-delta migration gate입니다.
	const FCFDAMigrationGateResult CurrentGate = FCFDADamageDace::EvaluateCurrentMigrationGate();
	TestTrue(TEXT("Accepted Damage bootstrap + exact0 compatibility must pass migration validation"), CurrentGate.Validation.bPassed);
	TestFalse(TEXT("No-delta Damage state must reject duplicate accepted append"), CurrentGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Accepted Damage current contract may promote Current projection"), CurrentGate.bCurrentSystemPromotionAllowed);

	// Missile accepted history를 Damage provider에 잘못 공급한 cross-TypeKey contamination 결과입니다.
	const FCFDAContractGuardResult MissileAsDamageHistory = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		DamageProvider.Descriptor,
		FCFDAContractGuard::GetAcceptedSnapshots());
	TestFalse(TEXT("Missile accepted chain must never validate as DACE-DamageData history"), MissileAsDamageHistory.bPassed);

	// Ammo accepted history를 Damage provider에 잘못 공급한 cross-TypeKey contamination 결과입니다.
	const FCFDAContractGuardResult AmmoAsDamageHistory = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		DamageProvider.Descriptor,
		FCFDAAmmoDace::GetAcceptedSnapshots());
	TestFalse(TEXT("Ammo accepted chain must never validate as DACE-DamageData history"), AmmoAsDamageHistory.bPassed);

	// Damage accepted history를 Ammo provider에 잘못 공급한 reverse contamination 결과입니다.
	const FCFDAContractGuardResult DamageAsAmmoHistory = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		CFDAAmmoProvider::GetProvider().Descriptor,
		FCFDADamageDace::GetAcceptedSnapshots());
	TestFalse(TEXT("Damage accepted chain must never validate as DACE-AmmoData history"), DamageAsAmmoHistory.bPassed);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
