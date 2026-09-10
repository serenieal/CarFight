// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoDaceTests.cpp
// Version: v1.1.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-04 Ammo DACE descriptor/production probe/bootstrap focused Automation입니다.
// Changelog:
// - v1.1.0: production fingerprint emitted token label exact coverage를 독립 manifest로 검증하고 immutable Ammo bootstrap exact1의 fixed field/signature regression을 추가했습니다.
// - v1.0.0: descriptor/bootstrap, production serialize→parse→fingerprint→materialize/extract, array/SoftObject negative drift와 migration exact0 regression을 추가.
// Migration:
// - 모든 fixture는 memory/transient-only입니다. Product Staging, persisted Ammo asset, Missile accepted history와 CFDAContractBase.cpp를 수정하지 않습니다.
// - fingerprint coverage는 production token emission을 관측하되 expected label manifest는 test-owned literal로 독립 유지합니다.

#include "CFDAAmmoDace.h"
#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDAContractGuard.h"
#include "CFDAMissileProvider.h"
#include "CFAmmoData.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAAmmoDaceTestsPrivate
{
	// Production probe에서 공통으로 사용할 exact Ammo payload를 생성합니다.
	FCFDAAmmoPayload BuildProbePayload(const bool bWithIcon = true)
	{
		// DACE production probe payload입니다.
		FCFDAAmmoPayload Payload;
		Payload.AmmoId = FName(TEXT("DaceProbeAmmo"));
		Payload.AmmoDisplayName.Text = TEXT("DACE Probe Ammo");
		Payload.AmmoFamilyId = FName(TEXT("ProbeFamily"));
		Payload.UnitMassKg = 3.25f;
		Payload.AmmoTags = {FName(TEXT("HE")), FName(TEXT("AP"))};
		Payload.AmmoIcon = bWithIcon
			? FSoftObjectPath(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"))
			: FSoftObjectPath();
		Payload.MaximumLoadableAmmoCount = 24;
		Payload.bCanBeResupplied = true;
		return Payload;
	}

	// Production Ammo fingerprint가 emit해야 하는 exact token label sequence를 독립 manifest로 반환합니다.
	TArray<FString> BuildExpectedFingerprintLabels(const bool bWithIcon)
	{
		// Schema/revision/class + Ammo exact8 semantic의 non-conditional token label sequence입니다.
		TArray<FString> Labels =
		{
			TEXT("SchemaId"),
			TEXT("SchemaRevision"),
			TEXT("AdapterContractRevision"),
			TEXT("DataAssetTypeClassPath"),
			TEXT("Payload.AmmoId"),
			TEXT("Payload.AmmoDisplayName"),
			TEXT("Payload.AmmoFamilyId"),
			TEXT("Payload.UnitMassKg"),
			TEXT("Payload.AmmoTags.Count"),
			TEXT("Payload.AmmoTags[]"),
			TEXT("Payload.AmmoTags[]"),
			TEXT("Payload.AmmoIcon.IsNull")
		};
		if (bWithIcon)
		{
			Labels.Add(TEXT("Payload.AmmoIcon.Path"));
		}
		Labels.Add(TEXT("Payload.MaximumLoadableAmmoCount"));
		Labels.Add(TEXT("Payload.bCanBeResupplied"));
		return Labels;
	}

	// Observed production token labels가 independent expected manifest와 순서/중복까지 exact 일치하는지 검증합니다.
	bool ValidateFingerprintLabels(
		FAutomationTestBase& Test,
		const TArray<FString>& ObservedLabels,
		const TArray<FString>& ExpectedLabels,
		const TCHAR* Context)
	{
		Test.TestEqual(*FString::Printf(TEXT("%s token label count must remain exact"), Context), ObservedLabels.Num(), ExpectedLabels.Num());
		if (ObservedLabels.Num() != ExpectedLabels.Num())
		{
			return false;
		}
		for (int32 LabelIndex = 0; LabelIndex < ExpectedLabels.Num(); ++LabelIndex)
		{
			Test.TestEqual(
				*FString::Printf(TEXT("%s token label[%d] must remain exact"), Context, LabelIndex),
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
		const bool bSerialized = FJsonSerializer::Serialize(RootObject, Writer);
		Writer->Close();
		return bSerialized;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoDaceDescriptorTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceDescriptorBootstrap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoDaceProductionProbeTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceProductionProbe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoDaceNegativeProbeTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceNegativeProbe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoDaceMigrationTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_04.AmmoDaceBootstrapMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Current four descriptors가 native Source와 bootstrap exact1에 동일하게 결속되는지 검증합니다.
bool FCFDAAmmoDaceDescriptorTest::RunTest(const FString& Parameters)
{
	// Current Ammo DACE aggregate contract 검증 결과입니다.
	const FCFDAContractGuardResult ContractResult = FCFDAAmmoDace::ValidateCurrentContract();
	TestTrue(TEXT("Current Ammo DACE descriptor/history/revision contract must pass"), ContractResult.bPassed);
	// Independent accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& History = FCFDAAmmoDace::GetAcceptedSnapshots();
	TestEqual(TEXT("Ammo accepted history must start at exact1"), History.Num(), 1);
	if (History.Num() != 1)
	{
		return false;
	}

	// Dedicated append-only owner가 보존해야 하는 immutable bootstrap exact1입니다.
	const FCFDAAcceptedContractSnapshot& Bootstrap = History[0];
	TestEqual(TEXT("Ammo bootstrap SnapshotId must remain immutable"), Bootstrap.SnapshotId, FString(TEXT("DACE-AmmoData-S1-A1-Bootstrap")));
	TestTrue(TEXT("Ammo bootstrap previous signature must remain empty"), Bootstrap.PreviousSnapshotSignature.IsEmpty());
	TestEqual(TEXT("Ammo bootstrap SchemaId must remain immutable"), Bootstrap.SchemaId, FString(TEXT("CarFight.DataAsset.AmmoData")));
	TestEqual(TEXT("Ammo bootstrap SchemaRevision must remain 1"), Bootstrap.SchemaRevision, 1);
	TestEqual(TEXT("Ammo bootstrap AdapterContractRevision must remain 1"), Bootstrap.AdapterContractRevision, 1);
	TestEqual(TEXT("Ammo bootstrap class path must remain immutable"), Bootstrap.DataAssetTypeClassPath, FString(TEXT("/Script/CarFight_Re.CFAmmoData")));
	TestEqual(TEXT("Ammo bootstrap Source signature must remain immutable"), Bootstrap.SourceShapeSignature, FString(TEXT("sha256:cd2aab5d0899a7c8c6def7e46bc90c69041801012b4fe874b25d2dd762d92339")));
	TestEqual(TEXT("Ammo bootstrap Adapter signature must remain immutable"), Bootstrap.AdapterShapeSignature, FString(TEXT("sha256:a0541028ddf1dd8b760e440f9af80456e7d1d8ae6d133c2727f67b9052d34073")));
	TestEqual(TEXT("Ammo bootstrap Mapping signature must remain immutable"), Bootstrap.SourceAdapterMappingSignature, FString(TEXT("sha256:9199d6eaaa37fb5a5493f85d230ce3cfff56543928d9b118902d188615097b4b")));
	TestEqual(TEXT("Ammo bootstrap Semantic signature must remain immutable"), Bootstrap.SemanticContractSignature, FString(TEXT("sha256:22cab6b7fc8951cd9d556fdfd32781570b17fb5c40f864a4df331a668e48ceff")));
	TestEqual(TEXT("Ammo bootstrap migration impact must remain NoMigration"), Bootstrap.MigrationImpact, ECFDAContractMigrationImpact::NoMigration);
	TestEqual(TEXT("Ammo bootstrap migration resolution must remain NotRequired"), Bootstrap.MigrationResolution, ECFDAContractMigrationResolution::NotRequired);
	TestTrue(TEXT("Ammo bootstrap migration evidence must remain empty"), Bootstrap.MigrationEvidenceId.IsEmpty());
	TestEqual(TEXT("Ammo bootstrap chain signature must remain immutable"), Bootstrap.SnapshotSignature, FString(TEXT("sha256:b6de5965fefe3ddf25c2318e9e426bb4f67072f3e5d8da4278cfcab15973cdd3")));

	// Current descriptor exact4 signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Signature failure diagnostic입니다.
	FString SignatureError;
	TestTrue(TEXT("Current Ammo signatures must build"), FCFDAAmmoDace::BuildCurrentSignatures(CurrentSignatures, SignatureError));
	if (!SignatureError.IsEmpty())
	{
		AddError(SignatureError);
		return false;
	}
	TestEqual(TEXT("Ammo SourceShape signature must equal accepted bootstrap"), CurrentSignatures.SourceShapeSignature, Bootstrap.SourceShapeSignature);
	TestEqual(TEXT("Ammo AdapterShape signature must equal accepted bootstrap"), CurrentSignatures.AdapterShapeSignature, Bootstrap.AdapterShapeSignature);
	TestEqual(TEXT("Ammo Mapping signature must equal accepted bootstrap"), CurrentSignatures.SourceAdapterMappingSignature, Bootstrap.SourceAdapterMappingSignature);
	TestEqual(TEXT("Ammo Semantic signature must equal accepted bootstrap"), CurrentSignatures.SemanticContractSignature, Bootstrap.SemanticContractSignature);

	// Bootstrap chain signature readback입니다.
	FString RebuiltSnapshotSignature;
	// Snapshot signature build failure diagnostic입니다.
	FString SnapshotError;
	TestTrue(TEXT("Ammo bootstrap snapshot signature must rebuild"), FCFDAContractGuard::BuildSnapshotSignature(Bootstrap, RebuiltSnapshotSignature, SnapshotError));
	TestEqual(TEXT("Ammo bootstrap stored chain signature must be exact"), RebuiltSnapshotSignature, Bootstrap.SnapshotSignature);
	return true;
}

// 실제 Ammo production serializer/parser/fingerprint/materializer-extractor가 같은 semantic contract를 왕복하는지 검증합니다.
bool FCFDAAmmoDaceProductionProbeTest::RunTest(const FString& Parameters)
{
	// Non-empty tags + non-null Texture2D SoftObject probe payload입니다.
	const FCFDAAmmoPayload ProbePayload = CFDAAmmoDaceTestsPrivate::BuildProbePayload(true);
	// Referenced Texture의 probe 전 현재 loaded object 상태입니다.
	UObject* const IconObjectBefore = ProbePayload.AmmoIcon.ResolveObject();
	// Production payload fingerprint입니다.
	FString SourceFingerprint;
	// 실제 production token append path에서 관측한 non-null icon token label sequence입니다.
	TArray<FString> ObservedFingerprintLabels;
	// Fingerprint failure diagnostic입니다.
	FString FingerprintError;
	{
		// Production common token framing의 thread-local observation sink입니다.
		CFDACommonPrimitives::FScopedSemanticTokenProbe TokenProbe(ObservedFingerprintLabels);
		TestTrue(TEXT("Ammo production fingerprint token probe must bind"), TokenProbe.IsBound());
		TestTrue(TEXT("Ammo production fingerprint must succeed"), CFDAAmmoProviderImpl::BuildSemanticFingerprint(ProbePayload, SourceFingerprint, FingerprintError));
	}
	// Test-owned independent non-null icon expected token manifest입니다.
	const TArray<FString> ExpectedFingerprintLabels = CFDAAmmoDaceTestsPrivate::BuildExpectedFingerprintLabels(true);
	CFDAAmmoDaceTestsPrivate::ValidateFingerprintLabels(*this, ObservedFingerprintLabels, ExpectedFingerprintLabels, TEXT("Ammo non-null icon fingerprint"));

	// Production serializer output입니다.
	FString SerializedJson;
	// Serializer failure diagnostic입니다.
	FString SerializerError;
	TestTrue(TEXT("Ammo production serializer must succeed"), CFDAAmmoProviderImpl::SerializeStagingJson(
		ProbePayload,
		TEXT("/Game/Test/CarFight/DAOAmmoDace/DA_DaceProbe.DA_DaceProbe"),
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
		FCFDAAmmoDace::GetAdapterShapeDescriptor());
	TestTrue(TEXT("Production Ammo serialized JSON must cover AdapterShape including AmmoTags[]"), AdapterCoverage.bPassed);

	// Production parser exact result입니다.
	const FCFDAAmmoParseResult ParseResult = CFDAAmmoProviderImpl::ParseJson(
		SerializedJson,
		TEXT("Authoring/DataAssetStaging/AmmoData/__AutomationDace__/DaceProbeAmmo.json"));
	TestTrue(TEXT("Production Ammo serializer output must parse through strict production parser"), ParseResult.bValid);
	if (!ParseResult.bValid)
	{
		return false;
	}
	TestEqual(TEXT("Serialize→parse fingerprint must remain exact"), ParseResult.Record.StagingSemanticFingerprint, SourceFingerprint);

	// Transient-only materializer probe target입니다.
	UCFAmmoData* const TransientAmmo = NewObject<UCFAmmoData>(GetTransientPackage());
	TestNotNull(TEXT("Transient Ammo probe object must be created"), TransientAmmo);
	if (TransientAmmo == nullptr)
	{
		return false;
	}
	CFDAAmmoProviderImpl::MaterializePayload(*TransientAmmo, ParseResult.Record.Payload);
	// Materialized transient UObject에서 다시 추출한 exact8 payload입니다.
	FCFDAAmmoPayload ExtractedPayload;
	// Extractor diagnostics입니다.
	TArray<FCFDAStagingIssue> ExtractIssues;
	TestTrue(TEXT("Materialized Ammo must extract through production extractor"), CFDAAmmoProviderImpl::ExtractPayload(*TransientAmmo, ExtractedPayload, ExtractIssues));
	// Extracted semantic fingerprint입니다.
	FString ExtractedFingerprint;
	// Extracted fingerprint failure diagnostic입니다.
	FString ExtractedFingerprintError;
	TestTrue(TEXT("Extracted Ammo fingerprint must succeed"), CFDAAmmoProviderImpl::BuildSemanticFingerprint(ExtractedPayload, ExtractedFingerprint, ExtractedFingerprintError));
	TestEqual(TEXT("Materialize→extract semantic fingerprint must remain exact"), ExtractedFingerprint, SourceFingerprint);

	// Referenced Texture의 probe 후 loaded object 상태입니다.
	UObject* const IconObjectAfter = ProbePayload.AmmoIcon.ResolveObject();
	TestEqual(TEXT("AmmoIcon production probe must not load referenced Texture asset"), IconObjectAfter, IconObjectBefore);

	// Null AmmoIcon round-trip fixture입니다.
	const FCFDAAmmoPayload NullIconPayload = CFDAAmmoDaceTestsPrivate::BuildProbePayload(false);
	// Null icon production fingerprint입니다.
	FString NullIconFingerprint;
	// 실제 production token append path에서 관측한 null icon token label sequence입니다.
	TArray<FString> ObservedNullIconLabels;
	// Null icon fingerprint failure diagnostic입니다.
	FString NullIconFingerprintError;
	{
		// Null AmmoIcon conditional token coverage observation sink입니다.
		CFDACommonPrimitives::FScopedSemanticTokenProbe NullIconTokenProbe(ObservedNullIconLabels);
		TestTrue(TEXT("Null AmmoIcon fingerprint token probe must bind"), NullIconTokenProbe.IsBound());
		TestTrue(TEXT("Null AmmoIcon production fingerprint must succeed"), CFDAAmmoProviderImpl::BuildSemanticFingerprint(NullIconPayload, NullIconFingerprint, NullIconFingerprintError));
	}
	// Test-owned independent null icon expected token manifest입니다.
	const TArray<FString> ExpectedNullIconLabels = CFDAAmmoDaceTestsPrivate::BuildExpectedFingerprintLabels(false);
	CFDAAmmoDaceTestsPrivate::ValidateFingerprintLabels(*this, ObservedNullIconLabels, ExpectedNullIconLabels, TEXT("Ammo null icon fingerprint"));
	TestFalse(TEXT("Null AmmoIcon fingerprint must omit Payload.AmmoIcon.Path"), ObservedNullIconLabels.Contains(TEXT("Payload.AmmoIcon.Path")));

	// Null icon serialized JSON입니다.
	FString NullIconJson;
	// Null icon serialization failure diagnostic입니다.
	FString NullIconError;
	TestTrue(TEXT("Null AmmoIcon production serializer must succeed"), CFDAAmmoProviderImpl::SerializeStagingJson(
		NullIconPayload,
		TEXT("/Game/Test/CarFight/DAOAmmoDace/DA_DaceNull.DA_DaceNull"),
		FString(),
		NullIconJson,
		NullIconError));
	// Null icon strict parse result입니다.
	const FCFDAAmmoParseResult NullIconParse = CFDAAmmoProviderImpl::ParseJson(
		NullIconJson,
		TEXT("Authoring/DataAssetStaging/AmmoData/__AutomationDace__/DaceNull.json"));
	TestTrue(TEXT("Null AmmoIcon serializer→parser round-trip must pass"), NullIconParse.bValid);
	return true;
}

// AmmoTags physical/semantic drift와 AmmoIcon target-class Source drift를 fail-closed 검증합니다.
bool FCFDAAmmoDaceNegativeProbeTest::RunTest(const FString& Parameters)
{
	// Valid production source fixture입니다.
	const FCFDAAmmoPayload ProbePayload = CFDAAmmoDaceTestsPrivate::BuildProbePayload(true);
	// Valid production JSON입니다.
	FString ValidJson;
	// Valid serialization failure diagnostic입니다.
	FString SerializeError;
	TestTrue(TEXT("Negative fixture base serializer must succeed"), CFDAAmmoProviderImpl::SerializeStagingJson(
		ProbePayload,
		TEXT("/Game/Test/CarFight/DAOAmmoDace/DA_DaceNeg.DA_DaceNeg"),
		FString(),
		ValidJson,
		SerializeError));

	// Mutable memory JSON root입니다.
	TSharedPtr<FJsonObject> WrongTypeRoot;
	TestTrue(TEXT("Negative fixture JSON must parse in memory"), CFDAAmmoDaceTestsPrivate::ParseRootJson(ValidJson, WrongTypeRoot));
	if (!WrongTypeRoot.IsValid())
	{
		return false;
	}
	// Mutable Ammo payload object입니다.
	TSharedPtr<FJsonObject> WrongTypePayload = WrongTypeRoot->GetObjectField(TEXT("Payload"));
	// Number element를 포함한 malformed AmmoTags physical array입니다.
	TArray<TSharedPtr<FJsonValue>> WrongTypeTags;
	WrongTypeTags.Add(MakeShared<FJsonValueString>(TEXT("ap")));
	WrongTypeTags.Add(MakeShared<FJsonValueNumber>(7.0));
	WrongTypePayload->SetArrayField(TEXT("AmmoTags"), WrongTypeTags);
	// Wrong element JSON text입니다.
	FString WrongTypeJson;
	TestTrue(TEXT("Wrong element fixture serialization must succeed"), CFDAAmmoDaceTestsPrivate::SerializeRootJson(WrongTypeRoot.ToSharedRef(), WrongTypeJson));
	// DACE physical observer result입니다.
	const FCFDAContractGuardResult WrongElementCoverage = FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(
		WrongTypeJson,
		FCFDAAmmoDace::GetAdapterShapeDescriptor());
	TestFalse(TEXT("AmmoTags wrong JSON element type must fail DACE AdapterShape"), WrongElementCoverage.bPassed);
	// Strict provider parse result입니다.
	const FCFDAAmmoParseResult WrongElementParse = CFDAAmmoProviderImpl::ParseJson(WrongTypeJson);
	TestFalse(TEXT("AmmoTags wrong JSON element type must fail production parser"), WrongElementParse.bValid);

	// Semantic duplicate tag fixture root입니다.
	TSharedPtr<FJsonObject> DuplicateRoot;
	TestTrue(TEXT("Duplicate fixture JSON must parse in memory"), CFDAAmmoDaceTestsPrivate::ParseRootJson(ValidJson, DuplicateRoot));
	// Duplicate fixture payload입니다.
	TSharedPtr<FJsonObject> DuplicatePayload = DuplicateRoot->GetObjectField(TEXT("Payload"));
	// FName case-insensitive semantic duplicate array입니다.
	TArray<TSharedPtr<FJsonValue>> DuplicateTags;
	DuplicateTags.Add(MakeShared<FJsonValueString>(TEXT("HE")));
	DuplicateTags.Add(MakeShared<FJsonValueString>(TEXT("he")));
	DuplicatePayload->SetArrayField(TEXT("AmmoTags"), DuplicateTags);
	// Duplicate fixture JSON text입니다.
	FString DuplicateJson;
	CFDAAmmoDaceTestsPrivate::SerializeRootJson(DuplicateRoot.ToSharedRef(), DuplicateJson);
	// Duplicate strict parse result입니다.
	const FCFDAAmmoParseResult DuplicateParse = CFDAAmmoProviderImpl::ParseJson(DuplicateJson);
	TestFalse(TEXT("AmmoTags FName semantic duplicate must fail production parser"), DuplicateParse.bValid);

	// Order/case permutation semantic-equivalence payload입니다.
	FCFDAAmmoPayload PermutedPayload = ProbePayload;
	PermutedPayload.AmmoTags = {FName(TEXT("ap")), FName(TEXT("he"))};
	// Original semantic fingerprint입니다.
	FString OriginalFingerprint;
	// Original fingerprint error입니다.
	FString OriginalError;
	CFDAAmmoProviderImpl::BuildSemanticFingerprint(ProbePayload, OriginalFingerprint, OriginalError);
	// Permuted semantic fingerprint입니다.
	FString PermutedFingerprint;
	// Permuted fingerprint error입니다.
	FString PermutedError;
	CFDAAmmoProviderImpl::BuildSemanticFingerprint(PermutedPayload, PermutedFingerprint, PermutedError);
	TestEqual(TEXT("AmmoTags order/case permutation must preserve semantic fingerprint"), PermutedFingerprint, OriginalFingerprint);

	// Current expected Source descriptor copy입니다.
	TArray<FCFDASourceFieldDescriptor> DriftedSource = FCFDAAmmoDace::GetSourceShapeDescriptor();
	// AmmoIcon target-class row입니다.
	FCFDASourceFieldDescriptor* DriftedIcon = DriftedSource.FindByPredicate([](const FCFDASourceFieldDescriptor& Descriptor)
	{
		return Descriptor.SourcePropertyPath.Equals(TEXT("AmmoIcon"), ESearchCase::CaseSensitive);
	});
	TestNotNull(TEXT("AmmoIcon DACE source descriptor must exist"), DriftedIcon);
	if (DriftedIcon != nullptr)
	{
		DriftedIcon->ReflectedTypePath = TEXT("/Script/Engine.Texture");
	}
	// Target-class drift coverage result입니다.
	const FCFDAContractGuardResult TargetClassDrift = FCFDAContractGuard::ValidateSourceShapeCoverage(
		FCFDAAmmoDace::GetSourceShapeDescriptor(),
		DriftedSource);
	TestFalse(TEXT("AmmoIcon UTexture2D target-class drift must fail SourceShape coverage"), TargetClassDrift.bPassed);
	return true;
}

// Independent Ammo bootstrap, exact0 canonical target와 no-delta migration gate가 다른 TypeKey와 분리되는지 검증합니다.
bool FCFDAAmmoDaceMigrationTest::RunTest(const FString& Parameters)
{
	// Current trusted Ammo provider입니다.
	const FCFDATypeProviderEntry& AmmoProvider = CFDAAmmoProvider::GetProvider();
	TestEqual(TEXT("Ammo DACE must be ContractReady"), AmmoProvider.Descriptor.DaceReadiness, ECFDADaceReadiness::ContractReady);
	TestEqual(TEXT("Ammo DACE owner must be CFDAAmmoDace"), AmmoProvider.Descriptor.DaceContractOwnerName, FName(TEXT("CFDAAmmoDace")));
	TestTrue(TEXT("Ammo canonical target set must be explicitly declared"), AmmoProvider.Descriptor.bDaceCanonicalStagingTargetSetDeclared);
	TestEqual(TEXT("Ammo Product canonical target set must remain exact0"), AmmoProvider.Descriptor.DaceCanonicalStagingRelativePaths.Num(), 0);

	// Current no-delta migration gate입니다.
	const FCFDAMigrationGateResult CurrentGate = FCFDAAmmoDace::EvaluateCurrentMigrationGate();
	TestTrue(TEXT("Accepted Ammo bootstrap + exact0 compatibility must pass migration validation"), CurrentGate.Validation.bPassed);
	TestFalse(TEXT("No-delta Ammo state must reject duplicate accepted append"), CurrentGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Accepted Ammo current contract may promote Current projection"), CurrentGate.bCurrentSystemPromotionAllowed);

	// Cross-TypeKey contamination result입니다.
	const FCFDAContractGuardResult CrossTypeHistory = FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
		AmmoProvider.Descriptor,
		FCFDAContractGuard::GetAcceptedSnapshots());
	TestFalse(TEXT("Missile accepted chain must never validate as DACE-AmmoData history"), CrossTypeHistory.bPassed);

	// Old SchemaRevision memory JSON fixture를 production serializer output에서 파생합니다.
	const FCFDAAmmoPayload Payload = CFDAAmmoDaceTestsPrivate::BuildProbePayload(false);
	// Current valid JSON입니다.
	FString CurrentJson;
	// Current serializer failure diagnostic입니다.
	FString SerializerError;
	CFDAAmmoProviderImpl::SerializeStagingJson(
		Payload,
		TEXT("/Game/Test/CarFight/DAOAmmoDace/DA_DaceOldRev.DA_DaceOldRev"),
		FString(),
		CurrentJson,
		SerializerError);
	// Old-revision mutable JSON root입니다.
	TSharedPtr<FJsonObject> OldRevisionRoot;
	CFDAAmmoDaceTestsPrivate::ParseRootJson(CurrentJson, OldRevisionRoot);
	if (!OldRevisionRoot.IsValid())
	{
		AddError(TEXT("Old revision fixture root parse failed."));
		return false;
	}
	OldRevisionRoot->SetNumberField(TEXT("SchemaRevision"), 0);
	// Old revision memory JSON입니다.
	FString OldRevisionJson;
	CFDAAmmoDaceTestsPrivate::SerializeRootJson(OldRevisionRoot.ToSharedRef(), OldRevisionJson);
	// Strict old-revision parse result입니다.
	const FCFDAAmmoParseResult OldRevisionParse = CFDAAmmoProviderImpl::ParseJson(OldRevisionJson);
	TestFalse(TEXT("Old Ammo SchemaRevision must fail strict current provider compatibility"), OldRevisionParse.bValid);
	return true;
}

#endif
